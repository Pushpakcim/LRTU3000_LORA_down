/*
 * COM_PORT_RS232_1.c
 *
 *  Created on: Nov 23, 2022
 *      Author: maulin
 */

/**************************************************************************//**
 * Includes
 *****************************************************************************/

#include "main.h"
#include"define.h"

/**************************************************************************//**
 * Macros
 *****************************************************************************/
#define AT_PORT_SEND_STR(str)       AT_Serial_Send_fn((const void*)(str), (size_t)strlen(str))
#define AT_PORT_SEND_CONST_STR(str) AT_Serial_Send_fn((const void*)(str), (size_t)(sizeof(str) - 1))
#define AT_PORT_SEND_CHR(ch)        AT_Serial_Send_fn((const void*)(ch), (size_t)1)
#define AT_PORT_SEND_FLUSH()        AT_Serial_Send_fn(NULL, 0)
#define AT_PORT_SEND(d, l)          AT_Serial_Send_fn((const void*)(d), (size_t)(l))

/* Send data over AT port */

#define AT_PORT_SEND_WITH_FLUSH(d, l)                                                                                  \
    do {                                                                                                               \
        AT_PORT_SEND((d), (l));                                                                                        \
        AT_PORT_SEND_FLUSH();                                                                                          \
    } while (0)

/* Beginning and end of every AT command */
#define AT_PORT_SEND_BEGIN_AT()                                                                                        \
    do {                                                                                                               \
        AT_PORT_SEND_CONST_STR("AT");                                                                                  \
    } while (0)
#define AT_PORT_SEND_END_AT()                                                                                          \
    do {                                                                                                               \
        AT_PORT_SEND(CRLF, CRLF_LEN);                                                                                  \
        AT_PORT_SEND(NULL, 0);                                                                                         \
    } while (0)

/* Send special characters over AT port with condition */
#define AT_PORT_SEND_QUOTE_COND(q)                                                                                     \
    do {                                                                                                               \
        if ((q)) {                                                                                                     \
            AT_PORT_SEND_CONST_STR("\"");                                                                              \
        }                                                                                                              \
    } while (0)
#define AT_PORT_SEND_COMMA_COND(c)                                                                                     \
    do {                                                                                                               \
        if ((c)) {                                                                                                     \
            AT_PORT_SEND_CONST_STR(",");                                                                               \
        }                                                                                                              \
    } while (0)
#define AT_PORT_SEND_EQUAL_COND(e)                                                                                     \
    do {                                                                                                               \
        if ((e)) {                                                                                                     \
            AT_PORT_SEND_CONST_STR("=");                                                                               \
        }                                                                                                              \
    } while (0)

/* Send special characters */
#define AT_PORT_SEND_CTRL_Z() AT_PORT_SEND_STR("\x1A")
#define AT_PORT_SEND_ESC()    AT_PORT_SEND_STR("\x1B")

/**************************************************************************//**
 * Variable
 *****************************************************************************/

unsigned int count_EC200U = 0 , modem_network_check_sec = 0 , network_reconnectCount=0;
char flag_modem_reboot = 0 ;
int16_t Modem_AT_Command = LWGSM_CMD_IDLE;
osThreadId EC200U_TaskHandle;
osSemaphoreId modem_PortBlockSemaphore;
osSemaphoreId modem_SequenceBlockSemaphore;
char Lora_Modem_Ascii_String_app_eui[50],Lora_Modem_Ascii_String_app_key[50],Lora_Modem_Ascii_String_dev_eui[50];

/**************************************************************************//**
 * Function name 	: EC200U_start
 * arguments		: 1)
 * return 		 	: no return type
 * Note				: #
 *****************************************************************************/

void EC200U_start()
{
	osThreadDef(EC200UTask, StartEC200UTask, osPriorityNormal, 0, 512); //512
	EC200U_TaskHandle = osThreadCreate(osThread(EC200UTask), NULL);
}

/**************************************************************************//**
 * Function name 	: StartCOM_PORT_RS232_1Task
 * arguments		: 1)
 * return 		 	:
 * Note				:
 * 					:
 * 					:
 *****************************************************************************/

void StartEC200UTask(void const * argument)
{
	HAL_UART_Receive_IT(&huart8, &EC200U_RX_Buff[0], sizeof(EC200U_RX_Buff));
	modem_PortBlockSemaphore = xSemaphoreCreateBinary();
	xSemaphoreGive(modem_PortBlockSemaphore);

	modem_SequenceBlockSemaphore = xSemaphoreCreateBinary();
	xSemaphoreGive(modem_SequenceBlockSemaphore);


	if(Pro_Application_flag == 0)
	{
		EC200U_modem_reboot();
	}

	EC200U_Modeminfo();

	for(;;)
	{

		//count_EC200U++;
		count_EC200U=0;
		modem_network_check_sec++;
		
		if((flag_modem_reboot == 1) && (Pro_Application_flag == 0))
		{
			if(EC200U_modem_reboot())
			{
				flag_modem_reboot=0;
				Modem_gsm_network_status = LWGSM_NETWORK_REG_STATUS_SIM_ERR; // it may be due to module not responded
				Modem_gsm_network_GACT_Status = 0;
				//mqtt[0].status = MQTT_STATE_DISCONNECTED_DUE_TO_NETWORK_DOWN;  // network down so mqtt also down.
				mqtt[0].connect.state = MQTT_CONNECTION_STATE_DEFAULT;
			}
		}

		if(Modem_gsm_network_GACT_Status == 0)
		{
			EC200U_make_internetConnection();
			modem_network_check_sec = 120;
			if(network_reconnectCount++ > 3600)
			{
				flag_modem_reboot = 1;
			}
		}
		else
		{
			network_reconnectCount=0;
		}
		if(modem_network_check_sec >= 120)
		{
			modem_network_check_sec = 0;
			EC200U_networkCheck();
		}

		{
			gFinalAnaValF[MODEM_PHY_STATUS_gFinalAnaValF]= Modem_PHY_Status;
			gFinalAnaValF[MODEM_GSM_SIM_STATUS_gFinalAnaValF]= Modem_gsm_sim_state;
			gFinalAnaValF[MODEM_GSM_NETWORK_STATUS_gFinalAnaValF]= Modem_gsm_network_status;
			gFinalAnaValF[MODEM_GSM_GACT_STATUS_gFinalAnaValF]= Modem_gsm_network_GACT_Status;
			gFinalAnaValF[MODEM_GSM_RSSI_gFinalAnaValF]= Modem_gsm_rssi;
		}
		osDelay(1000);
	}
}

/**************************************************************************//**
 * Function name 	: AT_Serial_Send_fn
 * arguments		: 1)
 * return 		 	:
 * Note				:
 * 					:
 * 					:
 *****************************************************************************/

uint16_t AT_Serial_Send_fn(const void* data, size_t len)
{
    const uint8_t* d = data;

    HAL_UART_Transmit(&huart8,d,len,5000);

    return len;
}

/**************************************************************************//**
 * Function name 	: EC200U_modem_reboot
 * arguments		: 1)
 * return 		 	:
 * Note				:
 * 					:
 * 					:
 *****************************************************************************/
unsigned char EC200U_modem_reboot()
{
	unsigned char res=0;

	if(xSemaphoreTake(modem_SequenceBlockSemaphore, 1200000))
	{
		modemCommandSequence = MODEM_CMD_MODEM_RESET;
		if(xSemaphoreTake(modem_PortBlockSemaphore, 30000))
		{
			Modem_AT_Command = LWGSM_CMD_RESET;
			lwgsmi_initiate_cmd(Modem_AT_Command,0);
			osDelay(15000);

			Modem_AT_Command = LWGSM_CMD_RESET_QRESET;
			lwgsmi_initiate_cmd(Modem_AT_Command,0);
			osDelay(15000);
			res =1;
			xSemaphoreGive(modem_PortBlockSemaphore);
		}
		else
		{
			xSemaphoreGive(modem_PortBlockSemaphore);
		}
		Modem_AT_Command = LWGSM_CMD_IDLE;
		modemCommandSequence = MODEM_CMD_SEQUEANCE_NONE;
		xSemaphoreGive(modem_SequenceBlockSemaphore);
	}

	return res;
}


/**************************************************************************//**
 * Function name 	: EC200U_Modeminfo
 * arguments		: 1)
 * return 		 	:
 * Note				:
 * 					:
 * 					:
 *****************************************************************************/
unsigned char EC200U_Modeminfo()
{
	unsigned char res=0;

	if(xSemaphoreTake(modem_SequenceBlockSemaphore, 1200000))
	{
		modemCommandSequence = MODEM_CMD_MODEM_INFO;
		if(xSemaphoreTake(modem_PortBlockSemaphore, 1000))
		{
			osDelay(2000);
			Modem_AT_Command = LWGSM_CMD_CGMR_GET;
			lwgsmi_initiate_cmd(Modem_AT_Command,0);
			osDelay(1000);

			Modem_AT_Command = LWGSM_CMD_CGSN_GET;
			lwgsmi_initiate_cmd(Modem_AT_Command,0);
			osDelay(500);

			Modem_AT_Command = LWGSM_CMD_CGMI_GET;
			lwgsmi_initiate_cmd(Modem_AT_Command,0);
			osDelay(500);

			Modem_AT_Command = LWGSM_CMD_CGMM_GET;
			lwgsmi_initiate_cmd(Modem_AT_Command,0);
			osDelay(500);

			Modem_AT_Command = LWGSM_CMD_QPWRBACKOFF;  // Add command as per Quectel suggestion for M2M card || ticket : https://cimcondigital.atlassian.net/browse/IRTU6000PP-19
			lwgsmi_initiate_cmd(Modem_AT_Command,0);
			osDelay(2000);

			if(Flag_QPWRBACKOFF == 0)
			{
				Modem_AT_Command = LWGSM_CMD_QPWRBACKOFF;  // Add command as per Quectel suggestion for M2M card || ticket : https://cimcondigital.atlassian.net/browse/IRTU6000PP-19
				lwgsmi_initiate_cmd(Modem_AT_Command,0);
				osDelay(2000);

				if(Flag_QPWRBACKOFF == 0)
				{
					Modem_AT_Command = LWGSM_CMD_QPWRBACKOFF;  // Add command as per Quectel suggestion for M2M card || ticket : https://cimcondigital.atlassian.net/browse/IRTU6000PP-19
					lwgsmi_initiate_cmd(Modem_AT_Command,0);
					osDelay(2000);
				}
			}
			//if(recv_buff)



//			memset(ModemInfo.model_serial_number,0,sizeof(ModemInfo.model_serial_number));
//			memcpy(ModemInfo.model_serial_number,"0123456789",10);


			xSemaphoreGive(modem_PortBlockSemaphore);
		}
//		else
//		{
//			xSemaphoreGive(modem_PortBlockSemaphore);
//		}
		Modem_AT_Command = LWGSM_CMD_IDLE;
		modemCommandSequence = MODEM_CMD_SEQUEANCE_NONE;
		xSemaphoreGive(modem_SequenceBlockSemaphore);
	}

	return res;
}
/**************************************************************************//**
 * Function name 	: EC200U_make_internetConnection
 * arguments		: 1)
 * return 		 	:
 * Note				:
 * 					:
 * 					:
 *****************************************************************************/

lwgsmr_t EC200U_make_internetConnection()
{
	lwgsmr_t res=0;
	unsigned int Delay_50=0;

	if(xSemaphoreTake(modem_SequenceBlockSemaphore, 1200000))
	{
		modemCommandSequence = MODEM_CMD_SEQUEANCE_NETWORK_INIT;
		if(xSemaphoreTake(modem_PortBlockSemaphore, 1000))
		{
			Modem_PHY_Status_t = lwgsmERRNODEVICE;

			//Modem_AT_Command = LWGSM_CMD_RESET_DEVICE_FIRST_CMD;
			//lwgsmi_initiate_cmd(LWGSM_CMD_RESET_DEVICE_FIRST_CMD,0);


			Modem_AT_Command = LWGSM_CMD_ATE0;
			lwgsmi_initiate_cmd(LWGSM_CMD_ATE0,0);




			if(xSemaphoreTake(modem_PortBlockSemaphore, 5000))
			{
				if(Modem_PHY_Status_t != Modem_PHY_Status)
				{
					Modem_PHY_Status = Modem_PHY_Status_t;
				}

				if(Modem_PHY_Status == lwgsmOK)
				{

					Modem_gsm_sim_state_t = LWGSM_SIM_STATE_NOT_INSERTED;
					Modem_AT_Command = LWGSM_CMD_CPIN_GET;
					lwgsmi_initiate_cmd(LWGSM_CMD_CPIN_GET,0);
					if(xSemaphoreTake(modem_PortBlockSemaphore, 1000))
					{
						osDelay(5000);
						Delay_50 = (unsigned int)(1000/50);
						while(Delay_50)
						{
							if(Modem_gsm_sim_state_t != LWGSM_SIM_STATE_NOT_INSERTED)
							{
								break;
							}
							else
							{
								Delay_50--;
								osDelay(50);
							}
						}

						if(Modem_gsm_sim_state_t != Modem_gsm_sim_state)
						{
							Modem_gsm_sim_state = Modem_gsm_sim_state_t;
						}

						if(Modem_gsm_sim_state == LWGSM_SIM_STATE_READY)
						{
							Modem_AT_Command = LWGSM_CMD_CSQ_GET;
							lwgsmi_initiate_cmd(LWGSM_CMD_CSQ_GET,0);
							if(xSemaphoreTake(modem_PortBlockSemaphore, 1000))
							{
								Modem_gsm_network_status_t = LWGSM_NETWORK_REG_STATUS_SIM_ERR;
								Modem_AT_Command = LWGSM_CMD_CGREG_GET;
								lwgsmi_initiate_cmd(LWGSM_CMD_CGREG_GET,0);

								if(xSemaphoreTake(modem_PortBlockSemaphore, 3000))
								{
									Delay_50 = (unsigned int)(1000/50);
									while(Delay_50)
									{
										if(Modem_gsm_network_status_t != LWGSM_NETWORK_REG_STATUS_SIM_ERR)
										{
											break;
										}
										else
										{
											Delay_50--;
											osDelay(50);
										}
									}
									if(Modem_gsm_network_status_t != Modem_gsm_network_status)
									{
										Modem_gsm_network_status = Modem_gsm_network_status_t;
									}

									//if((Modem_gsm_network_status == LWGSM_NETWORK_REG_STATUS_CONNECTED)
									//		||(Modem_gsm_network_status == LWGSM_NETWORK_REG_STATUS_CONNECTED_ROAMING))
									{
										Modem_AT_Command = LWGSM_CMD_CGATT_SET_0;
										lwgsmi_initiate_cmd(LWGSM_CMD_CGATT_SET_0,0);
										if(xSemaphoreTake(modem_PortBlockSemaphore, 1000))
										{
											Modem_AT_Command = LWGSM_CMD_CGATT_SET_1;
											lwgsmi_initiate_cmd(LWGSM_CMD_CGATT_SET_1,0);
											if(xSemaphoreTake(modem_PortBlockSemaphore, 10000))
											{
												Modem_gsm_network_GATT_Status_t = 0;
												Modem_AT_Command = LWGSM_CMD_CGATT_GET;
												lwgsmi_initiate_cmd(LWGSM_CMD_CGATT_GET,0);
												if(xSemaphoreTake(modem_PortBlockSemaphore, 1000))
												{
													Delay_50 = (unsigned int)(1000/50);
													while(Delay_50)
													{
														if((Modem_gsm_network_GATT_Status_t == 1))
														{
															break;
														}
														else
														{
															Delay_50--;
															osDelay(50);
														}
													}

													if(Modem_gsm_network_GATT_Status_t != Modem_gsm_network_GATT_Status)
													{
														Modem_gsm_network_GATT_Status = Modem_gsm_network_GATT_Status_t;
													}
													if(Modem_gsm_network_GATT_Status ==1)
													{
														Modem_AT_Command = LWGSM_CMD_CGACT_SET_0;
														lwgsmi_initiate_cmd(LWGSM_CMD_CGACT_SET_0,0);
														if(xSemaphoreTake(modem_PortBlockSemaphore, 10000))
														{
															////////////
															osDelay(1000);
															Modem_AT_Command = LWGSM_CMD_CGDCONT;
															lwgsmi_initiate_cmd(LWGSM_CMD_CGDCONT,0);
															if(xSemaphoreTake(modem_PortBlockSemaphore, 10000))
															{
																osDelay(2000);
																Modem_AT_Command = LWGSM_CMD_CGACT_SET_1;
																lwgsmi_initiate_cmd(LWGSM_CMD_CGACT_SET_1,0);
																if(xSemaphoreTake(modem_PortBlockSemaphore, 10000))
																{
																	osDelay(1000);
																	Modem_gsm_network_GACT_Status_t = 0;
																	Modem_AT_Command = LWGSM_CMD_CGACT_GET;
																	lwgsmi_initiate_cmd(LWGSM_CMD_CGACT_GET,0);
																	if(xSemaphoreTake(modem_PortBlockSemaphore, 1000))
																	{
																		Delay_50 = (unsigned int)(1000/50);
																		while(Delay_50)
																		{
																			if((Modem_gsm_network_GACT_Status_t == 1))
																			{
																				break;
																			}
																			else
																			{
																				Delay_50--;
																				osDelay(50);
																			}
																		}
																		if(Modem_gsm_network_GACT_Status_t != Modem_gsm_network_GACT_Status)
																		{
																			Modem_gsm_network_GACT_Status = Modem_gsm_network_GACT_Status_t;
																		}
																		if(Modem_gsm_network_GACT_Status == 1)
																		{
																			Modem_AT_Command = LWGSM_CMD_CGPADDR_GET;
																			lwgsmi_initiate_cmd(LWGSM_CMD_CGPADDR_GET,0);
																			if(xSemaphoreTake(modem_PortBlockSemaphore, 1000))
																			{
																				goto End_5;// Finish
																			}
																			else
																			{
																				goto End_5;// Semaphore not get after LWGSM_CMD_CGACT_GET
																			}
																		}
																		else
																		{
																			goto End_5; // Modem_gsm_network_GACT_Status = 0
																		}
																	}
																	else
																	{
																		goto End_4;// Semaphore not get after LWGSM_CMD_CGACT_GET
																	}
																}
																else
																{
																	goto End_4;// Semaphore not get after LWGSM_CMD_CGACT_SET_1
																}

															}
															else
															{
																goto End_4;// Semaphore not get after LWGSM_CMD_CGDCONT
															}
														/////////////////////////////////////////////////////
														}
														else
														{
															goto End_4;// Semaphore not get after LWGSM_CMD_CGACT_SET_0
														}
													}
													else
													{
														goto End_4; // Modem_gsm_network_GATT_Status = 0
													}
												}
												else
												{
													goto End_3; // Semaphore not get after LWGSM_CMD_CGATT_GET
												}
											}
											else
											{
												goto End_3; // Semaphore not get after LWGSM_CMD_CGATT_SET_1
											}
										}
										else
										{
											goto End_3; // Semaphore not get after LWGSM_CMD_CGATT_SET_0
										}
									}
									//else
									//{
									//	goto End_3; // != LWGSM_NETWORK_REG_STATUS_CONNECTED !=LWGSM_NETWORK_REG_STATUS_CONNECTED_ROAMING
									//}
								}
								else
								{
									goto End_2; // Semaphore not get after LWGSM_CMD_CGREG_GET
								}
							}
							else
							{
								goto End_2; // Semaphore not get after LWGSM_CMD_CSQ_GET
							}
						}
						else
						{
							goto End_1; // LWGSM_CMD_RESET_DEVICE_FIRST_CMD  != LWGSM_SIM_STATE_READY
						}
					}
					else
					{
						goto End_1; // Semaphore not get after LWGSM_CMD_CPIN_GET
					}
				}
				else
				{
					goto End; // LWGSM_CMD_RESET_DEVICE_FIRST_CMD  = lwgsmERRNODEVICE
				}

			}
			else
			{
				goto End; // Semaphore not get after LWGSM_CMD_RESET_DEVICE_FIRST_CMD
			}

			End:
			Modem_PHY_Status = lwgsmERRNODEVICE;
			End_1:
			Modem_gsm_sim_state = LWGSM_SIM_STATE_NOT_INSERTED;
			End_2:
			Modem_gsm_network_status = LWGSM_NETWORK_REG_STATUS_SIM_ERR; // it may be due to module not responded
			End_3:
			Modem_gsm_network_GATT_Status = 0;
			End_4:
			Modem_gsm_network_GACT_Status = 0;
			mqtt[0].status = MQTT_STATE_DISCONNECTED_DUE_TO_NETWORK_DOWN;  // network down so mqtt also down.
			mqtt[0].connect.state = MQTT_CONNECTION_STATE_DEFAULT;
			End_5:
			Modem_AT_Command = LWGSM_CMD_IDLE;
			xSemaphoreGive(modem_PortBlockSemaphore);

		}

		modemCommandSequence = MODEM_CMD_SEQUEANCE_NONE;
		xSemaphoreGive(modem_SequenceBlockSemaphore);
	}


	return res;
}


/**************************************************************************//**
 * Function name 	: modem_isConnected
 * arguments		: 1)
 * return 		 	:
 * Note				:
 * 					:
 * 					:
 *****************************************************************************/

lwgsmr_t EC200U_networkCheck()
{
	lwgsmr_t res=0;
	unsigned int Delay_50=0;

	if(xSemaphoreTake(modem_SequenceBlockSemaphore, 1200000))
	{
		modemCommandSequence = MODEM_CMD_SEQUEANCE_NETWORK_CHECK;

		//==========================================================

		if(xSemaphoreTake(modem_PortBlockSemaphore, 1000))
		{
			Modem_PHY_Status_t = lwgsmERRNODEVICE;
			Modem_AT_Command = LWGSM_CMD_RESET_DEVICE_FIRST_CMD;
			lwgsmi_initiate_cmd(LWGSM_CMD_RESET_DEVICE_FIRST_CMD,0);
		}

		//==========================================================

		if(xSemaphoreTake(modem_PortBlockSemaphore, 1000))
		{
			if(Modem_PHY_Status_t != Modem_PHY_Status)
			{
				Modem_PHY_Status = Modem_PHY_Status_t;
			}

			if(Modem_PHY_Status == lwgsmOK)
			{
				Modem_gsm_sim_state_t = LWGSM_SIM_STATE_NOT_INSERTED;
				Modem_AT_Command = LWGSM_CMD_CPIN_GET;
				lwgsmi_initiate_cmd(LWGSM_CMD_CPIN_GET,0);

				if(xSemaphoreTake(modem_PortBlockSemaphore, 1000))
				{
					Delay_50 = (unsigned int)(1000/50);
					while(Delay_50)
					{
						if(Modem_gsm_sim_state_t != LWGSM_SIM_STATE_NOT_INSERTED)
						{
							break;
						}
						else
						{
							Delay_50--;
							osDelay(50);
						}
					}

					if(Modem_gsm_sim_state_t != Modem_gsm_sim_state)
					{
						Modem_gsm_sim_state = Modem_gsm_sim_state_t;
					}

					if(Modem_gsm_sim_state == LWGSM_SIM_STATE_READY)
					{
						Modem_AT_Command = LWGSM_CMD_CSQ_GET;
						lwgsmi_initiate_cmd(LWGSM_CMD_CSQ_GET,0);

						if(xSemaphoreTake(modem_PortBlockSemaphore, 1000))
						{
							Modem_gsm_network_status_t = LWGSM_NETWORK_REG_STATUS_SIM_ERR;
							Modem_AT_Command = LWGSM_CMD_CGREG_GET;
							lwgsmi_initiate_cmd(LWGSM_CMD_CGREG_GET,0);

							if(xSemaphoreTake(modem_PortBlockSemaphore, 1000))
							{
								Delay_50 = (unsigned int)(1000/50);
								while(Delay_50)
								{
									if(Modem_gsm_network_status_t != LWGSM_NETWORK_REG_STATUS_SIM_ERR)
									{
										break;
									}
									else
									{
										Delay_50--;
										osDelay(50);
									}
								}
								if(Modem_gsm_network_status_t != Modem_gsm_network_status)
								{
									Modem_gsm_network_status = Modem_gsm_network_status_t;
								}

								if((Modem_gsm_network_status == LWGSM_NETWORK_REG_STATUS_CONNECTED)
										||(Modem_gsm_network_status == LWGSM_NETWORK_REG_STATUS_CONNECTED_ROAMING))
								{
									Modem_gsm_network_GATT_Status_t = 0;
									Modem_AT_Command = LWGSM_CMD_CGATT_GET;
									lwgsmi_initiate_cmd(LWGSM_CMD_CGATT_GET,0);
									if(xSemaphoreTake(modem_PortBlockSemaphore, 1000))
									{
										Delay_50 = (unsigned int)(1000/50);
										while(Delay_50)
										{
											if((Modem_gsm_network_GATT_Status_t == 1))
											{
												break;
											}
											else
											{
												Delay_50--;
												osDelay(50);
											}
										}

										if(Modem_gsm_network_GATT_Status_t != Modem_gsm_network_GATT_Status)
										{
											Modem_gsm_network_GATT_Status = Modem_gsm_network_GATT_Status_t;
										}
										if(Modem_gsm_network_GATT_Status ==1)
										{
											Modem_gsm_network_GACT_Status_t = 0;
											Modem_AT_Command = LWGSM_CMD_CGACT_GET;
											lwgsmi_initiate_cmd(LWGSM_CMD_CGACT_GET,0);
											if(xSemaphoreTake(modem_PortBlockSemaphore, 1000))
											{
												Delay_50 = (unsigned int)(1000/50);
												while(Delay_50)
												{
													if((Modem_gsm_network_GACT_Status_t == 1))
													{
														break;
													}
													else
													{
														Delay_50--;
														osDelay(50);
													}
												}
												if(Modem_gsm_network_GACT_Status_t != Modem_gsm_network_GACT_Status)
												{
													Modem_gsm_network_GACT_Status = Modem_gsm_network_GACT_Status_t;
												}
												if(Modem_gsm_network_GACT_Status == 1)
												{
													Modem_AT_Command = LWGSM_CMD_CGPADDR_GET;
													lwgsmi_initiate_cmd(LWGSM_CMD_CGPADDR_GET,0);
													if(xSemaphoreTake(modem_PortBlockSemaphore, 1000))
													{
														goto End4;
//														Modem_AT_Command = LWGSM_CMD_IDLE;
//														xSemaphoreGive(modem_PortBlockSemaphore);
													}
													else
													{
														goto End4;
//														Modem_AT_Command = LWGSM_CMD_IDLE;
//														xSemaphoreGive(modem_PortBlockSemaphore);
													}
												}
												else
												{
													goto End4;
//													Modem_AT_Command = LWGSM_CMD_IDLE;
//													xSemaphoreGive(modem_PortBlockSemaphore);
												}
											}
											else
											{
												goto End4;
//												Modem_AT_Command = LWGSM_CMD_IDLE;
//												xSemaphoreGive(modem_PortBlockSemaphore);
											}
										}
										else
										{
											goto End3;
//											Modem_AT_Command = LWGSM_CMD_IDLE;
//											xSemaphoreGive(modem_PortBlockSemaphore);
//											Modem_gsm_network_GACT_Status = 0;
										}
									}
									else
									{
										goto End2;
//										Modem_AT_Command = LWGSM_CMD_IDLE;
//										xSemaphoreGive(modem_PortBlockSemaphore);
//										Modem_gsm_network_GATT_Status = 0;
//										Modem_gsm_network_GACT_Status = 0;
									}
								}
								else
								{
									goto End2;
//									Modem_AT_Command = LWGSM_CMD_IDLE;
//									xSemaphoreGive(modem_PortBlockSemaphore);
//									Modem_gsm_network_GATT_Status = 0;
//									Modem_gsm_network_GACT_Status = 0;
								}
							}
							else
							{
								goto End2;
//								Modem_AT_Command = LWGSM_CMD_IDLE;
//								xSemaphoreGive(modem_PortBlockSemaphore);
//								Modem_gsm_network_GATT_Status = 0;
//								Modem_gsm_network_GACT_Status = 0;
							}
						}
						else
						{
							goto End1;// it may be due to module not responded
//							Modem_AT_Command = LWGSM_CMD_IDLE;
//							xSemaphoreGive(modem_PortBlockSemaphore);
//							Modem_gsm_network_status = LWGSM_NETWORK_REG_STATUS_SIM_ERR;
//							Modem_gsm_network_GATT_Status = 0;
//							Modem_gsm_network_GACT_Status = 0;
						}
					}
					else
					{
						goto End1; // it may be due to module not responded
//						Modem_AT_Command = LWGSM_CMD_IDLE;
//						xSemaphoreGive(modem_PortBlockSemaphore);
//						Modem_gsm_network_status = LWGSM_NETWORK_REG_STATUS_SIM_ERR;
//						Modem_gsm_network_GATT_Status = 0;
//						Modem_gsm_network_GACT_Status = 0;
					}

				}
				else
				{
					goto End1; // it may be due to module not responded
//					Modem_AT_Command = LWGSM_CMD_IDLE;
//					xSemaphoreGive(modem_PortBlockSemaphore);
//					Modem_gsm_network_status = LWGSM_NETWORK_REG_STATUS_SIM_ERR;
//					Modem_gsm_network_GATT_Status = 0;
//					Modem_gsm_network_GACT_Status = 0;
				}
			}
			else
			{
				goto End0; // it may be due to module not responded
//				Modem_AT_Command = LWGSM_CMD_IDLE;
//				xSemaphoreGive(modem_PortBlockSemaphore);
//				Modem_PHY_Status = lwgsmERRNODEVICE;
//				Modem_gsm_sim_state = LWGSM_SIM_STATE_NOT_INSERTED;
//				Modem_gsm_network_status = LWGSM_NETWORK_REG_STATUS_SIM_ERR;
//				Modem_gsm_network_GATT_Status = 0;
//				Modem_gsm_network_GACT_Status = 0;
			}
		}
		else
		{
			goto End0; // it may be due to module not responded
//			Modem_AT_Command = LWGSM_CMD_IDLE;
//			xSemaphoreGive(modem_PortBlockSemaphore);
//			Modem_PHY_Status = lwgsmERRNODEVICE;
//			Modem_gsm_sim_state = LWGSM_SIM_STATE_NOT_INSERTED;
//			Modem_gsm_network_status = LWGSM_NETWORK_REG_STATUS_SIM_ERR;
//			Modem_gsm_network_GATT_Status = 0;
//			Modem_gsm_network_GACT_Status = 0;
		}

		//==================================================

		End0:
			Modem_PHY_Status = lwgsmERRNODEVICE;
			Modem_gsm_sim_state = LWGSM_SIM_STATE_NOT_INSERTED;
		End1:
			Modem_gsm_network_status = LWGSM_NETWORK_REG_STATUS_SIM_ERR; // it may be due to module not responded
		End2:
			Modem_gsm_network_GATT_Status = 0;
		End3:
			Modem_gsm_network_GACT_Status = 0;
			mqtt[0].status = MQTT_STATE_DISCONNECTED_DUE_TO_NETWORK_DOWN;  // network down so mqtt also down.
			mqtt[0].connect.state = MQTT_CONNECTION_STATE_DEFAULT;
		End4:
			Modem_AT_Command = LWGSM_CMD_IDLE;
			xSemaphoreGive(modem_PortBlockSemaphore);

		modemCommandSequence = MODEM_CMD_SEQUEANCE_NONE;
		xSemaphoreGive(modem_SequenceBlockSemaphore);
	}

	return res;
}
/**************************************************************************//**
 * Function name 	: modem_isConnected
 * arguments		: 1) lwgsm_cmd_t cmd
 * return 		 	: Member of \ref lwgsmr_t enumeration
 * Note				: Function to initialize every AT command
 * 					:
 * 					:
 *****************************************************************************/

lwgsmr_t lwgsmi_initiate_cmd(lwgsm_cmd_t cmd,void *argument)
{

    switch (cmd)
    {
    	/* Check current message we want to send over AT */
        case LWGSM_CMD_RESET:
        {
        	/* Reset modem with AT commands */
            /* Try with hardware reset */
//            if (lwgsm.ll.reset_fn != NULL && lwgsm.ll.reset_fn(1))
//            {
//                lwgsm_delay(2);
//                lwgsm.ll.reset_fn(0);
//                lwgsm_delay(500);
//            }

            /* Send manual AT command */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CFUN=1,1");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_RESET_QRESET:
        {
        	/* Reset modem with AT commands */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+QRESET");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_BLE_QSCLK:
         {
         	/* Reset modem with AT commands */
             AT_PORT_SEND_BEGIN_AT();
             AT_PORT_SEND_CONST_STR("+QSCLK=1");
             AT_PORT_SEND_END_AT();
             break;
         }
        case LWGSM_CMD_RESET_DEVICE_FIRST_CMD:
        {
        	/* First command for device driver specific reset */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_ATE0:
        {
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("E0");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_ATE1:
        {
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("E1");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CMEE_SET:
        {
        	/* Enable detailed error messages */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CMEE=1");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CLCC_SET:
        {
        	/* Enable detailed call info */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CLCC=1");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CGMI_GET:
        {
        	/* Get manufacturer */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CGMI");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CGMM_GET:
        {
        	/* Get model */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CGMM");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CGSN_GET:
        {
        	/* Get serial number */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CGSN");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CGMR_GET:
        {
        	/* Get revision */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CGMR");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CREG_SET:
        {
        	/* Enable +CREG message */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CREG=1");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CREG_GET:
        {
        	/* Get network registration status */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CREG?");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CGREG_GET:
        {
        	/* Get network registration status */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CGREG?");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CFUN_SET:
        {
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CFUN=");
            /**
             * \todo: If CFUN command forced, check value
             */
           // if (CMD_IS_DEF(LWGSM_CMD_RESET) || (CMD_IS_DEF(LWGSM_CMD_CFUN_SET) && msg->msg.cfun.mode))
            {
                AT_PORT_SEND_CONST_STR("1");
            }
//            else
//            {
//                AT_PORT_SEND_CONST_STR("0");
//            }
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CPIN_GET:
        {
        	/* Read current SIM status */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CPIN?");
            AT_PORT_SEND_END_AT();
            break;
        }
//        case LWGSM_CMD_CPIN_SET:
//        {
//        	/* Set SIM pin code */
//            AT_PORT_SEND_BEGIN_AT();
//            AT_PORT_SEND_CONST_STR("+CPIN=");
//            lwgsmi_send_string(msg->msg.cpin_enter.pin, 0, 1, 0);
//            AT_PORT_SEND_END_AT();
//            break;
//        }
//        case LWGSM_CMD_CPIN_ADD:
//        {
//        	/* Add new pin code */
//            AT_PORT_SEND_BEGIN_AT();
//            AT_PORT_SEND_CONST_STR("+CLCK=\"SC\",1,");
//            lwgsmi_send_string(msg->msg.cpin_add.pin, 0, 1, 0);
//            AT_PORT_SEND_END_AT();
//            break;
//        }
//        case LWGSM_CMD_CPIN_CHANGE:
//        {
//        	/* Change already active SIM */
//            AT_PORT_SEND_BEGIN_AT();
//            AT_PORT_SEND_CONST_STR("+CPWD=\"SC\"");
//            lwgsmi_send_string(msg->msg.cpin_change.current_pin, 0, 1, 1);
//            lwgsmi_send_string(msg->msg.cpin_change.new_pin, 0, 1, 1);
//            AT_PORT_SEND_END_AT();
//            break;
//        }
//        case LWGSM_CMD_CPIN_REMOVE:
//        {
//        	/* Remove current PIN */
//            AT_PORT_SEND_BEGIN_AT();
//            AT_PORT_SEND_CONST_STR("+CLCK=\"SC\",0,");
//            lwgsmi_send_string(msg->msg.cpin_remove.pin, 0, 1, 0);
//            AT_PORT_SEND_END_AT();
//            break;
//        }
//        case LWGSM_CMD_CPUK_SET:
//        {
//        	/* Enter PUK and set new PIN */
//            AT_PORT_SEND_BEGIN_AT();
//            AT_PORT_SEND_CONST_STR("+CPIN=");
//            lwgsmi_send_string(msg->msg.cpuk_enter.puk, 0, 1, 0);
//            lwgsmi_send_string(msg->msg.cpuk_enter.pin, 0, 1, 1);
//            AT_PORT_SEND_END_AT();
//            break;
//        }
//        case LWGSM_CMD_COPS_SET:
//        {
//        	/* Set current operator */
//            AT_PORT_SEND_BEGIN_AT();
//            AT_PORT_SEND_CONST_STR("+COPS=");
//            lwgsmi_send_number(LWGSM_U32(msg->msg.cops_set.mode), 0, 0);
//            if (msg->msg.cops_set.mode != LWGSM_OPERATOR_MODE_AUTO)
//            {
//                lwgsmi_send_number(LWGSM_U32(msg->msg.cops_set.format), 0, 1);
//                switch (msg->msg.cops_set.format)
//                {
//                    case LWGSM_OPERATOR_FORMAT_LONG_NAME:
//                    case LWGSM_OPERATOR_FORMAT_SHORT_NAME:
//                        lwgsmi_send_string(msg->msg.cops_set.name, 1, 1, 1);
//                        break;
//                    default:
//                        lwgsmi_send_number(LWGSM_U32(msg->msg.cops_set.num), 0, 1);
//                }
//            }
//            AT_PORT_SEND_END_AT();
//            break;
//        }
//        case LWGSM_CMD_COPS_GET:
//        {
//        	/* Get current operator */
//            AT_PORT_SEND_BEGIN_AT();
//            AT_PORT_SEND_CONST_STR("+COPS?");
//            AT_PORT_SEND_END_AT();
//            break;
//        }
//        case LWGSM_CMD_COPS_GET_OPT:
//        {
//        	/* Get list of available operators */
//            AT_PORT_SEND_BEGIN_AT();
//            AT_PORT_SEND_CONST_STR("+COPS=?");
//            AT_PORT_SEND_END_AT();
//            break;
//        }
        case LWGSM_CMD_CSQ_GET:
        {
        	/* Get signal strength */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CSQ");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CNUM:
        {
        	/* Get SIM number */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CNUM");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CIPSHUT:
        {
        	/* Shut down network connection and put to reset state */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CIPSHUT");
            AT_PORT_SEND_END_AT();
            break;
        }
#if LWGSM_CFG_CONN
        case LWGSM_CMD_CIPMUX:
        {
        	/* Enable multiple connections */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CIPMUX=1");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CIPHEAD:
        {
        	/* Enable information on receive data about connection and length */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CIPHEAD=1");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CIPSRIP:
        {
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CIPSRIP=1");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CIPSSL:
        {
        	/* Set SSL configuration */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CIPSSL=");
            lwgsmi_send_number((msg->msg.conn_start.type == LWGSM_CONN_TYPE_SSL) ? 1 : 0, 0, 0);
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CIPSTART:
        {
        	/* Start a new connection */
            lwgsm_conn_t* c = NULL;

            /* Do we have network connection? */
            /* Check if we are connected to network */

            msg->msg.conn_start.num = 0;                             /* Start with max value = invalidated */
            for (int16_t i = LWGSM_CFG_MAX_CONNS - 1; i >= 0; --i)
            {
            	/* Find available connection */
                if (!lwgsm.m.conns[i].status.f.active) {
                    c = &lwgsm.m.conns[i];
                    c->num = LWGSM_U8(i);
                    msg->msg.conn_start.num = LWGSM_U8(i); /* Set connection number for message structure */
                    break;
                }
            }
            if (c == NULL)
            {
                lwgsmi_send_conn_error_cb(msg, lwgsmERRNOFREECONN);
                return lwgsmERRNOFREECONN; /* We don't have available connection */
            }

            if (msg->msg.conn_start.conn != NULL)
            {
            	/* Is user interested about connection info? */
                *msg->msg.conn_start.conn = c;      /* Save connection for user */
            }

            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CIPSTART=");
            lwgsmi_send_number(LWGSM_U32(c->num), 0, 0);
            if (msg->msg.conn_start.type == LWGSM_CONN_TYPE_UDP)
            {
                lwgsmi_send_string("UDP", 0, 1, 1);
            }
            else
            {
                lwgsmi_send_string("TCP", 0, 1, 1);
            }
            lwgsmi_send_string(msg->msg.conn_start.host, 0, 1, 1);
            lwgsmi_send_port(msg->msg.conn_start.port, 0, 1);
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CIPCLOSE:
        {
        	/* Close the connection */
            lwgsm_conn_p c = msg->msg.conn_close.conn;
            if (c != NULL &&
                /* Is connection already closed or command for this connection is not valid anymore? */
                (!lwgsm_conn_is_active(c) || c->val_id != msg->msg.conn_close.val_id))
            {
                return lwgsmERR;
            }
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CIPCLOSE=");
            lwgsmi_send_number(
                LWGSM_U32(msg->msg.conn_close.conn ? msg->msg.conn_close.conn->num : LWGSM_CFG_MAX_CONNS), 0, 0);
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CIPSEND:
        {
        	/* Send data to connection */
            return lwgsmi_tcpip_process_send_data(); /* Process send data */
        }
        case LWGSM_CMD_CIPSTATUS:
        {
        	/* Get status of device and all connections */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CIPSTATUS");
            AT_PORT_SEND_END_AT();
            break;
        }
#endif /* LWGSM_CFG_CONN */
#if LWGSM_CFG_SMS
        case LWGSM_CMD_CMGF: { /* Select SMS message format */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CMGF=");
            if (CMD_IS_DEF(LWGSM_CMD_CMGS)) {
                lwgsmi_send_number(LWGSM_U32(!!msg->msg.sms_send.format), 0, 0);
            } else if (CMD_IS_DEF(LWGSM_CMD_CMGR)) {
                lwgsmi_send_number(LWGSM_U32(!!msg->msg.sms_read.format), 0, 0);
            } else if (CMD_IS_DEF(LWGSM_CMD_CMGL)) {
                lwgsmi_send_number(LWGSM_U32(!!msg->msg.sms_list.format), 0, 0);
            } else {
                /* Used for all other operations like delete all messages, etc */
                AT_PORT_SEND_CONST_STR("1");
            }
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CMGS: { /* Send SMS */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CMGS=");
            lwgsmi_send_string(msg->msg.sms_send.num, 0, 1, 0);
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CMGR: { /* Read message */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CMGR=");
            lwgsmi_send_number(LWGSM_U32(msg->msg.sms_read.pos), 0, 0);
            lwgsmi_send_number(LWGSM_U32(!msg->msg.sms_read.update), 0, 1);
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CMGD: { /* Delete SMS message */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CMGD=");
            lwgsmi_send_number(LWGSM_U32(msg->msg.sms_delete.pos), 0, 0);
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CMGDA: { /* Mass delete SMS messages */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CMGDA=");
            switch (msg->msg.sms_delete_all.status) {
                case LWGSM_SMS_STATUS_READ:
                    lwgsmi_send_string("DEL READ", 0, 1, 0);
                    break;
                case LWGSM_SMS_STATUS_UNREAD:
                    lwgsmi_send_string("DEL UNREAD", 0, 1, 0);
                    break;
                case LWGSM_SMS_STATUS_SENT:
                    lwgsmi_send_string("DEL SENT", 0, 1, 0);
                    break;
                case LWGSM_SMS_STATUS_UNSENT:
                    lwgsmi_send_string("DEL UNSENT", 0, 1, 0);
                    break;
                case LWGSM_SMS_STATUS_INBOX:
                    lwgsmi_send_string("DEL INBOX", 0, 1, 0);
                    break;
                case LWGSM_SMS_STATUS_ALL:
                    lwgsmi_send_string("DEL ALL", 0, 1, 0);
                    break;
                default:
                    break;
            }
            AT_PORT_SEND_END_AT();
            break;
        }

        case LWGSM_CMD_CMGL: { /* Delete SMS message */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CMGL=");
            lwgsmi_send_sms_stat(msg->msg.sms_list.status, 1, 0);
            lwgsmi_send_number(LWGSM_U32(!msg->msg.sms_list.update), 0, 1);
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CPMS_GET_OPT: { /* Get available SMS storages */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CPMS=?");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CPMS_GET: { /* Get current SMS storage info */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CPMS?");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CPMS_SET: { /* Set active SMS storage(s) */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CPMS=");
            if (CMD_IS_DEF(LWGSM_CMD_CMGR)) { /* Read SMS original command? */
                lwgsmi_send_dev_memory(msg->msg.sms_read.mem == LWGSM_MEM_CURRENT ? lwgsm.m.sms.mem[0].current
                                                                                  : msg->msg.sms_read.mem,
                                       1, 0);
            } else if (CMD_IS_DEF(LWGSM_CMD_CMGD)) { /* Delete SMS original command? */
                lwgsmi_send_dev_memory(msg->msg.sms_delete.mem == LWGSM_MEM_CURRENT ? lwgsm.m.sms.mem[0].current
                                                                                    : msg->msg.sms_delete.mem,
                                       1, 0);
            } else if (CMD_IS_DEF(LWGSM_CMD_CMGL)) { /* List SMS original command? */
                lwgsmi_send_dev_memory(msg->msg.sms_list.mem == LWGSM_MEM_CURRENT ? lwgsm.m.sms.mem[0].current
                                                                                  : msg->msg.sms_list.mem,
                                       1, 0);
            } else if (CMD_IS_DEF(
                           LWGSM_CMD_CPMS_SET)) { /* Do we want to set memory for read/delete,sent/write,receive? */
                for (size_t i = 0; i < 3; ++i) {  /* Write 3 memories */
                    lwgsmi_send_dev_memory(msg->msg.sms_memory.mem[i] == LWGSM_MEM_CURRENT ? lwgsm.m.sms.mem[i].current
                                                                                           : msg->msg.sms_memory.mem[i],
                                           1, !!i);
                }
            }
            AT_PORT_SEND_END_AT();
            break;
        }
#endif /* LWGSM_CFG_SMS */
#if LWGSM_CFG_CALL
        case LWGSM_CMD_ATD: { /* Start new call */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("D");
            lwgsmi_send_string(msg->msg.call_start.number, 0, 0, 0);
            AT_PORT_SEND_CONST_STR(";");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_ATA: { /* Answer phone call */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("A");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_ATH: { /* Disconnect existing connection (hang-up phone call) */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("H");
            AT_PORT_SEND_END_AT();
            break;
        }
#endif /* LWGSM_CFG_CALL */
#if LWGSM_CFG_PHONEBOOK
        case LWGSM_CMD_CPBS_GET_OPT: { /* Get available phonebook storages */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CPBS=?");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CPBS_GET: { /* Get current memory info */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CPBS?");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CPBS_SET: { /* Get current memory info */
            lwgsm_mem_t mem;
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CPBS=");
            switch (CMD_GET_DEF()) {
                case LWGSM_CMD_CPBW_SET:
                    mem = msg->msg.pb_write.mem;
                    break;
                case LWGSM_CMD_CPBR:
                    mem = msg->msg.pb_list.mem;
                    break;
                case LWGSM_CMD_CPBF:
                    mem = msg->msg.pb_search.mem;
                    break;
                default:
                    break;
            }
            lwgsmi_send_dev_memory(mem == LWGSM_MEM_CURRENT ? lwgsm.m.pb.mem.current : mem, 1, 0);
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CPBW_SET: { /* Write/Delete new/old entry */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CPBW=");
            if (msg->msg.pb_write.pos > 0) { /* Write number if more than 0 */
                lwgsmi_send_number(LWGSM_U32(msg->msg.pb_write.pos), 0, 0);
            }
            if (!msg->msg.pb_write.del) {
                lwgsmi_send_string(msg->msg.pb_write.num, 0, 1, 1);
                lwgsmi_send_number(LWGSM_U32(msg->msg.pb_write.type), 0, 1);
                lwgsmi_send_string(msg->msg.pb_write.name, 0, 1, 1);
            }
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CPBR: { /* Read entires */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CPBR=");
            lwgsmi_send_number(LWGSM_U32(msg->msg.pb_list.start_index), 0, 0);
            lwgsmi_send_number(LWGSM_U32(msg->msg.pb_list.etr), 0, 1);
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CPBF: { /* Find entires */
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CPBF=");
            lwgsmi_send_string(msg->msg.pb_search.search, 1, 1, 0);
            AT_PORT_SEND_END_AT();
            break;
        }
#endif /* LWGSM_CFG_PHONEBOOK */
//#if LWGSM_CFG_NETWORK
        case LWGSM_CMD_NETWORK_ATTACH:
        case LWGSM_CMD_CGACT_SET_0: {
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CGACT=0,1");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CGACT_SET_1: {
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CGACT=1,1");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CGACT_GET: {
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CGACT?");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_NETWORK_DETACH:
        case LWGSM_CMD_CGATT_SET_0: {
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CGATT=0");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CGATT_SET_1: {
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CGATT=1");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CGATT_GET: {
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CGATT?");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CGDCONT: {
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CGDCONT="); //“AT+CGDCONT=1,"IP","jionet"
            lwgsmi_send_number(LWGSM_U32(PDP_Context_ID), 0, 0);//PDP_Context_ID
            AT_PORT_SEND_CONST_STR(",\"IP\"");
            lwgsmi_send_string(PDP_Context_APN, 1, 1, 1);//PDP_Context_APN
//            lwgsmi_send_string(msg->msg.network_attach.user, 1, 1, 1);//
//            lwgsmi_send_string(msg->msg.network_attach.pass, 1, 1, 1);
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CGPADDR_GET: {
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CGPADDR=1");
            AT_PORT_SEND_END_AT();
            break;
        }

        case LWGSM_CMD_QPWRBACKOFF: {
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+QPWRBACKOFF=\"GSM900\",,17");
            AT_PORT_SEND_END_AT();
            break;
        }

//        case LWGSM_CMD_CIPMUX_SET: {
//            AT_PORT_SEND_BEGIN_AT();
//            AT_PORT_SEND_CONST_STR("+CIPMUX=1");
//            AT_PORT_SEND_END_AT();
//            break;
//        }
//        case LWGSM_CMD_CIPRXGET_SET: {
//            AT_PORT_SEND_BEGIN_AT();
//            AT_PORT_SEND_CONST_STR("+CIPRXGET=0");
//            AT_PORT_SEND_END_AT();
//            break;
//        }
//        case LWGSM_CMD_CSTT_SET: {
//            AT_PORT_SEND_BEGIN_AT();
//            AT_PORT_SEND_CONST_STR("+CSTT=");
//            lwgsmi_send_string(msg->msg.network_attach.apn, 1, 1, 0);
//            lwgsmi_send_string(msg->msg.network_attach.user, 1, 1, 1);
//            lwgsmi_send_string(msg->msg.network_attach.pass, 1, 1, 1);
//            AT_PORT_SEND_END_AT();
//            break;
//        }
//        case LWGSM_CMD_CIICR: {
//            AT_PORT_SEND_BEGIN_AT();
//            AT_PORT_SEND_CONST_STR("+CIICR");
//            AT_PORT_SEND_END_AT();
//            break;
//        }
//        case LWGSM_CMD_CIFSR: {
//            AT_PORT_SEND_BEGIN_AT();
//            AT_PORT_SEND_CONST_STR("+CIFSR");
//            AT_PORT_SEND_END_AT();
//            break;
//        }
//#endif /* LWGSM_CFG_NETWORK */

//    	LWGSM_CMD_MQTT_QMTCFG_RECV_MODE_SET, 	/*!< set MQTT Receive mode */
//    	LWGSM_CMD_MQTT_QMTOPEN, 				/*!< set MQTT OPEN Link */
//    	LWGSM_CMD_MQTT_QMTCONN_SET, 			/*!< set MQTT make Connection with broker */
//    	LWGSM_CMD_MQTT_QMTSUB_SET, 				/*!< set MQTT subscribe */
//    	LWGSM_CMD_MQTT_QMTPUBEX, 				/*!< MQTT Publish */
//    	LWGSM_CMD_MQTT_QMTUNS, 					/*!< set MQTT unsubscribe */
//    	LWGSM_CMD_MQTT_QMTDISC, 				/*!< Disconnect a Client from MQTT Server*/
//    	LWGSM_CMD_MQTT_QMTCLOSE, 				/*!< Close a Network for MQTT Client*/
        case LWGSM_CMD_MQTT_QMTCFG_RECV_MODE_SET:
        {
        	modem_mqtt_client_t *_mqtt =  (modem_mqtt_client_t *)argument;

            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+QMTCFG=\"recv/mode\"");
            //lwgsmi_send_string(_mqtt->cfg.recv_mode.msg_recv_mode, 0, 1, 1);
            lwgsmi_send_number(LWGSM_U32(_mqtt->client_idx), 0, 1);
            lwgsmi_send_number(LWGSM_U32(_mqtt->cfg.recv_mode.msg_recv_mode), 0, 1);
            lwgsmi_send_number(LWGSM_U32(_mqtt->cfg.recv_mode.msg_len_enable), 0, 1);
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_MQTT_QMTOPEN_SET:
        {
        	modem_mqtt_client_t *_mqtt =  (modem_mqtt_client_t *)argument;
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+QMTOPEN=");
            lwgsmi_send_number(LWGSM_U32(_mqtt->client_idx), 0, 0);
            lwgsmi_send_string(_mqtt->open.Broker, 0, 1, 1);
            lwgsmi_send_number(LWGSM_U32(_mqtt->open.Broker_Port), 0, 1);
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_MQTT_QMTOPEN_GET:
        {
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+QMTOPEN?");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_MQTT_QMTCLOSE:
        {
        	modem_mqtt_client_t *_mqtt =  (modem_mqtt_client_t *)argument;
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+QMTCLOSE=");
            lwgsmi_send_number(LWGSM_U32(_mqtt->client_idx), 0, 0);
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_MQTT_QMTCONN_SET:
        {
        	modem_mqtt_client_t *_mqtt =  (modem_mqtt_client_t *)argument;
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+QMTCONN=");
            lwgsmi_send_number(LWGSM_U32(_mqtt->client_idx), 0, 0);
            lwgsmi_send_string(_mqtt->connect.client_id, 0, 1, 1);
            lwgsmi_send_string(_mqtt->connect.client_pass, 0, 1, 1);
            lwgsmi_send_string(_mqtt->connect.client_user, 0, 1, 1);
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_MQTT_QMTCONN_GET:
        {
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+QMTCONN?");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_MQTT_QMTDISC:
        {
        	modem_mqtt_client_t *_mqtt =  (modem_mqtt_client_t *)argument;
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+QMTDISC=");
            lwgsmi_send_number(LWGSM_U32(_mqtt->client_idx), 0, 0);
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_MQTT_QMTSUB_SET:
        {
        	modem_mqtt_client_t *_mqtt =  (modem_mqtt_client_t *)argument;
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+QMTSUB=");
            lwgsmi_send_number(LWGSM_U32(_mqtt->client_idx), 0, 0);
            lwgsmi_send_number(LWGSM_U32(_mqtt->msgId), 0, 1);
            lwgsmi_send_string(_mqtt->sub.topic, 0, 1, 1);
            lwgsmi_send_number(LWGSM_U32(_mqtt->sub.QOS), 0, 1);
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_MQTT_QMTPUBEX:
        {
        	modem_mqtt_client_t *_mqtt =  (modem_mqtt_client_t *)argument;
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+QMTPUBEX=");
            lwgsmi_send_number(LWGSM_U32(_mqtt->client_idx), 0, 0);
            lwgsmi_send_number(LWGSM_U32(_mqtt->msgId), 0, 1);
            lwgsmi_send_number(LWGSM_U32(_mqtt->pub.QOS), 0, 1);
            lwgsmi_send_number(LWGSM_U32(_mqtt->pub.retain), 0, 1);
            lwgsmi_send_string(_mqtt->pub.topic, 0, 1, 1);
            lwgsmi_send_number(LWGSM_U32(_mqtt->pub.msg_len), 0, 1);
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_MQTT_PUB_MSG:
        {
        	count_2++;
        	modem_mqtt_client_t *_mqtt =  (modem_mqtt_client_t *)argument;
        	count_3++;
        	AT_PORT_SEND(_mqtt->pub.msg,_mqtt->pub.msg_len);
        	count_4++;
        	AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_GPS_QGPS_GET:
        {
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+QGPS?");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_GPS_QGPS_SET:
        {
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+QGPS=1");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_GPS_QGPSLOC:
        {
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+QGPSLOC=2");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_GPS_QGPSEND:
        {
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+QGPSEND");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_BLE_QBTPWR_GET:
        {
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+QBTPWR?");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_BLE_QBTPWR_ON:
        {
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+QBTPWR=1");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_BLE_QBTPWR_OFF:
        {
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+QBTPWR=0");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_BLE_QBTLEADDR_GET:
        {
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+QBTLEADDR?");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_BLE_QBTNAME_SET:
        {
        	modem_BLE_t *_ble =  (modem_BLE_t *)argument;
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+QBTNAME=0,\"iRTU-");  //AT+QBTNAME=0,"iRTU-6250ac9a97e8"
            lwgsmi_send_ip_mac(&_ble->macAddess,0,0,0,0);
            AT_PORT_SEND_QUOTE_COND(1);
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_BLE_QBTGATADV:
        {
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+QBTGATADV=1,128,160,0,0,7,0");  //AT+QBTGATADV=1,128,160,0,0,7,0 AT+QBTGATADV=<op>,<min_interval>,<max_interval>,<adv_type>,<own_addrtype>,<channel_map>,<filter>[[,<remote_addrtype>][,<remote_addr>]]
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_BLE_QBTADVDATA:
        {
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+QBTADVDATA=9,\"020106050938393130\"");  //AT+QBTADVDATA=9,"020106050938393130"
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_BLE_QBTADVSTR:
        {
        	modem_BLE_t *_ble =  (modem_BLE_t *)argument;
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+QBTADVSTR=3,1,\"iRTU-");  //AT+QBTADVSTR=3,1,"iRTU-6250ac9a97e8",0,"06"
            lwgsmi_send_ip_mac(&_ble->macAddess,0,0,0,0);
            AT_PORT_SEND_QUOTE_COND(1);
            AT_PORT_SEND_CONST_STR(",0,\"06\"");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_BLE_QBTGATSS: //AT+QBTGATSS=0,0,"f5899b5f800000800010000000000100",1
        {
        	modem_BLE_t *_ble =  (modem_BLE_t *)argument;
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+QBTGATSS=");
            lwgsmi_send_number(LWGSM_U32(_ble->Service->servID), 0, 0);
            lwgsmi_send_number(LWGSM_U32(_ble->Service->UUID_type), 0, 1);
            if(_ble->Service->UUID_type)
            {
            	lwgsmi_send_number(LWGSM_U32(_ble->Service->serv_UUID_s), 0, 1);
            }
            else
            {
            	lwgsmi_send_string((const char*)_ble->Service->serv_UUID_l, 0, 1, 1);
            }
            lwgsmi_send_number(LWGSM_U32(_ble->Service->serviceType), 0, 1);
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_BLE_QBTGATSC:  //AT+QBTGATSC=0,0,58,0,"f5899b5f800000800010000001000100"
        {
        	modem_BLE_t *_ble =  (modem_BLE_t *)argument;
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+QBTGATSC=");
            lwgsmi_send_number(LWGSM_U32(_ble->Characteristic->servID), 0, 0);
            lwgsmi_send_number(LWGSM_U32(_ble->Characteristic->charaID), 0, 1);
            lwgsmi_send_number(LWGSM_U32(_ble->Characteristic->property), 0, 1);
            lwgsmi_send_number(LWGSM_U32(0), 0, 1);   // Characteristic.UUID_type is fix 128bit
            lwgsmi_send_string((const char*)_ble->Characteristic->chara_UUID_l, 0, 1, 1);
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_BLE_QBTGATSCV: //AT+QBTGATSCV=0,0,3,0,"f5899b5f800000800010000001000100",20,"39383736353433323130"
        {
        	modem_BLE_t *_ble =  (modem_BLE_t *)argument;
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+QBTGATSCV=");
            lwgsmi_send_number(LWGSM_U32(_ble->Characteristic->servID), 0, 0);
            lwgsmi_send_number(LWGSM_U32(_ble->Characteristic->charaID), 0, 1);
            lwgsmi_send_number(LWGSM_U32(_ble->Characteristic->permission), 0, 1);
            lwgsmi_send_number(LWGSM_U32(0), 0, 1);   // Characteristic.UUID_type is fix 128bit
            lwgsmi_send_string((const char*)_ble->Characteristic->chara_UUID_l, 0, 1, 1);
            lwgsmi_send_number(LWGSM_U32(50), 0, 1);
            lwgsmi_send_string((const char * )_ble->msg, 0, 1, 1);
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_BLE_QBTGATSCD:  //AT+QBTGATSCD=0,0,3,1,10498,2,"0300"
        {
        	modem_BLE_t *_ble =  (modem_BLE_t *)argument;
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+QBTGATSCD=");
            lwgsmi_send_number(LWGSM_U32(_ble->Characteristic->servID), 0, 0);
            lwgsmi_send_number(LWGSM_U32(_ble->Characteristic->charaID), 0, 1);
            lwgsmi_send_number(LWGSM_U32(_ble->Characteristic->permission), 0, 1);
            lwgsmi_send_number(LWGSM_U32(1), 0, 1);   // Characteristic.UUID_type is fix 128bit
            lwgsmi_send_number(LWGSM_U32(10498), 0, 1);
            lwgsmi_send_number(LWGSM_U32(2), 0, 1);
            AT_PORT_SEND_CONST_STR(",\"0300\"");
            //lwgsmi_send_string((const char*)_ble->msg, 0, 1, 1);
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_BLE_QBTGATCHSCV:
        {
        	modem_BLE_t *_ble =  (modem_BLE_t *)argument;
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+QBTGATCHSCV=");
            lwgsmi_send_number(LWGSM_U32(_ble->Characteristic->servID), 0, 0);
            lwgsmi_send_number(LWGSM_U32(_ble->Characteristic->charaID), 0, 1);
            //lwgsmi_send_number(LWGSM_U32(_ble->msgLen), 0, 1);
            lwgsmi_send_number(LWGSM_U32(50), 0, 1);
            lwgsmi_send_string((const char*)_ble->msg, 0, 1, 1);
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_BLE_QBTGATSSC:
        {
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+QBTGATSSC=1,1");  //AT+QBTGATSSC=1,1
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_BLE_QBTADV:
        {
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+QBTADV=1");  //AT+QBTADV=1
            AT_PORT_SEND_END_AT();
            break;
        }

#if LWGSM_CFG_USSD
        case LWGSM_CMD_CUSD_GET: {
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CUSD?");
            AT_PORT_SEND_END_AT();
            break;
        }
        case LWGSM_CMD_CUSD: {
            AT_PORT_SEND_BEGIN_AT();
            AT_PORT_SEND_CONST_STR("+CUSD=1,");
            lwgsmi_send_string(msg->msg.ussd.code, 1, 1, 0);
            AT_PORT_SEND_END_AT();
            break;
        }
#endif /* LWGSM_CFG_USSD */
        default:
            return lwgsmERR; /* Invalid command */
    }
    return lwgsmOK; /* Valid command */
}

/**
 * \brief           Send IP or MAC address to AT port
 * \param[in]       d: Pointer to IP or MAC address
 * \param[in]       is_ip: Set to `1` when sending IP, `0` when MAC
 * \param[in]       withWithout: Set to `1` when with . or : , `0` when without . or :
 * \param[in]       q: Set to `1` to include start and ending quotes
 * \param[in]       c: Set to `1` to include comma before string
 */


void lwgsmi_send_ip_mac(const void* d, uint8_t is_ip,uint8_t withWithout, uint8_t q, uint8_t c)
{
    uint8_t ch;
    char str[4];
    const lwgsm_mac_t* mac = d;
    const lwgsm_ip_t* ip = d;

    AT_PORT_SEND_COMMA_COND(c); /* Send comma */
    if (d == NULL) {
        return;
    }
    AT_PORT_SEND_QUOTE_COND(q);                       	/* Send quote */

    ch = is_ip ? '.' : ':';                           	/* Get delimiter character */

    for (uint8_t i = 0; i < (is_ip ? 4 : 6); ++i)     	/* Process byte by byte */
    {
        if (is_ip) 										/* In case of IP ... */
        {
            lwgsm_u8_to_str(ip->ip[i], str);          	/* ... go to decimal format ... */
        }
        else 											/* ... in case of MAC ... */
        {
            lwgsm_u8_to_hex_str(mac->mac[i], str, 2); 	/* ... go to HEX format */
        }
        AT_PORT_SEND_STR(str);         /* Send str */

        if(withWithout)
        {
			if (i < (is_ip ? 4 : 6) - 1) 					/* Check end if characters */
			{
				AT_PORT_SEND_CHR(&ch);     /* Send character */
			}
        }
    }
    AT_PORT_SEND_QUOTE_COND(q); /* Send quote */
}

/**
 * \brief           Send string to AT port, either plain or escaped
 * \param[in]       str: Pointer to input string to string
 * \param[in]       e: Value to indicate string send format, escaped (`1`) or plain (`0`)
 * \param[in]       q: Value to indicate starting and ending quotes, enabled (`1`) or disabled (`0`)
 * \param[in]       c: Set to `1` to include comma before string
 */

void lwgsmi_send_string(const char* str, uint8_t e, uint8_t q, uint8_t c)
{
    char special = '\\';

    AT_PORT_SEND_COMMA_COND(c); /* Send comma */
    AT_PORT_SEND_QUOTE_COND(q); /* Send quote */
    if (str != NULL) {
        if (e) {                                                  /* Do we have to escape string? */
            while (*str) {                                        /* Go through string */
                if (*str == ',' || *str == '"' || *str == '\\') { /* Check for special character */
                    AT_PORT_SEND_CHR(&special);                   /* Send special character */
                }
                AT_PORT_SEND_CHR(str); /* Send character */
                ++str;
            }
        } else {
            AT_PORT_SEND_STR(str); /* Send plain string */
        }
    }
    AT_PORT_SEND_QUOTE_COND(q); /* Send quote */
}

/**
 * \brief           Send number (decimal) to AT port
 * \param[in]       num: Number to send to AT port
 * \param[in]       q: Value to indicate starting and ending quotes, enabled (`1`) or disabled (`0`)
 * \param[in]       c: Set to `1` to include comma before string
 */

void lwgsmi_send_number(uint32_t num, uint8_t q, uint8_t c)
{
    char str[11];

    lwgsm_u32_to_str(num, str); /* Convert digit to decimal string */

    AT_PORT_SEND_COMMA_COND(c); /* Send comma */
    AT_PORT_SEND_QUOTE_COND(q); /* Send quote */
    AT_PORT_SEND_STR(str);      /* Send string with number */
    AT_PORT_SEND_QUOTE_COND(q); /* Send quote */
}

/**
 * \brief           Send port number to AT port
 * \param[in]       port: Port number to send
 * \param[in]       q: Value to indicate starting and ending quotes, enabled (`1`) or disabled (`0`)
 * \param[in]       c: Set to `1` to include comma before string
 */

//void lwgsmi_send_port(lwgsm_port_t port, uint8_t q, uint8_t c)
//{
//    char str[6];
//
//    lwgsm_u16_to_str(LWGSM_PORT2NUM(port), str); /* Convert digit to decimal string */
//
//    AT_PORT_SEND_COMMA_COND(c); /* Send comma */
//    AT_PORT_SEND_QUOTE_COND(q); /* Send quote */
//    AT_PORT_SEND_STR(str);      /* Send string with number */
//    AT_PORT_SEND_QUOTE_COND(q); /* Send quote */
//}

/**
 * \brief           Send signed number to AT port
 * \param[in]       num: Number to send to AT port
 * \param[in]       q: Value to indicate starting and ending quotes, enabled (`1`) or disabled (`0`)
 * \param[in]       c: Set to `1` to include comma before string
 */
void lwgsmi_send_signed_number(int32_t num, uint8_t q, uint8_t c)
{
    char str[11];

    lwgsm_i32_to_str(num, str); /* Convert digit to decimal string */

    AT_PORT_SEND_COMMA_COND(c); /* Send comma */
    AT_PORT_SEND_QUOTE_COND(q); /* Send quote */
    AT_PORT_SEND_STR(str);      /* Send string with number */
    AT_PORT_SEND_QUOTE_COND(q); /* Send quote */
}
