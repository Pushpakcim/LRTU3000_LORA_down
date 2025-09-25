/*
 * COM_PORT_RS485_2.c
 *
 *  Created on: Nov 23, 2022
 *      Author: maulin
 */

/**************************************************************************//**
 * Includes
 *****************************************************************************/

#include "main.h"
#include "COM_PORT_RS485_2.h"

/**************************************************************************//**
 * Variable
 *****************************************************************************/
unsigned int count_RS485_2=0;
unsigned char COM_RS485_2_RX=0;
osThreadId COM_PORT_RS485_2_TaskHandle;

///**************************************************************************//**
// * Function name 	: MX_UART5_Init
// * arguments		: 1)
// * return 		 	: no return type
// * Note				: UART5 Initialization Function
// *****************************************************************************/
//
///**
//  * @brief UART5 Initialization Function
//  * @param None
//  * @retval None
//  */
//void MX_UART5_Init(void)
//{
//
//  /* USER CODE BEGIN UART5_Init 0 */
//
//  /* USER CODE END UART5_Init 0 */
//
//  /* USER CODE BEGIN UART5_Init 1 */
//
//  /* USER CODE END UART5_Init 1 */
//  huart5.Instance = UART5;
//  huart5.Init.BaudRate = EPROM_General.S_Comm.Rs485_2_Info.S_Baudrate;//9600;
//  huart5.Init.WordLength = UART_WORDLENGTH_8B;
//  huart5.Init.StopBits = UART_STOPBITS_1;
//  huart5.Init.Parity = UART_PARITY_NONE;
//  huart5.Init.Mode = UART_MODE_TX_RX;
//  huart5.Init.HwFlowCtl = UART_HWCONTROL_NONE;
//  huart5.Init.OverSampling = UART_OVERSAMPLING_16;
//  huart5.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
//  huart5.Init.ClockPrescaler = UART_PRESCALER_DIV1;
//  huart5.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
//  if (HAL_RS485Ex_Init(&huart5, UART_DE_POLARITY_HIGH, 0, 0) != HAL_OK)
//  {
//    Error_Handler();
//  }
//  if (HAL_UARTEx_SetTxFifoThreshold(&huart5, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
//  {
//    Error_Handler();
//  }
//  if (HAL_UARTEx_SetRxFifoThreshold(&huart5, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
//  {
//    Error_Handler();
//  }
//  if (HAL_UARTEx_DisableFifoMode(&huart5) != HAL_OK)
//  {
//    Error_Handler();
//  }
//  /* USER CODE BEGIN UART5_Init 2 */
//  HAL_UART_ReceiverTimeout_Config(&huart5,100);
//  HAL_UART_EnableReceiverTimeout(&huart5);
//  /* USER CODE END UART5_Init 2 */
//
//}
//
///**************************************************************************//**
// * Function name 	: COM_PORT_RS485_2_start
// * arguments		: 1)
// * return 		 	: no return type
// * Note				: #
// *****************************************************************************/
//
//void COM_PORT_RS485_2_start()
//{
//	osThreadDef(COM_PORT_RS485_2Task, StartCOM_PORT_RS485_2Task, osPriorityNormal, 0, 512); //512
//	COM_PORT_RS485_2_TaskHandle = osThreadCreate(osThread(COM_PORT_RS485_2Task), NULL);
//}
//
///**************************************************************************//**
// * Function name 	: StartCOM_PORT_RS485_2Task
// * arguments		: 1)
// * return 		 	:
// * Note				:
// * 					:
// * 					:
// *****************************************************************************/
//
//void StartCOM_PORT_RS485_2Task(void const * argument)
//{
//	osDelay(5000);
//	HAL_UART_Receive_IT(&huart5, &ModbusH[COM_RS485_2].u8RxBuffer[0], sizeof(ModbusH[COM_RS485_2].u8RxBuffer));
//	for(;;)
//	{
//		#ifdef WATCH_DOG_ENABLE //MAU_EXCEPTION
//			HAL_IWDG_Refresh(&hiwdg1);
//		#endif
//		//count_RS485_2++;
//		count_RS485_2=0;
//		if(Pro_Application_flag == 1)
//		{
//			ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
//			osDelay(20);
//			if(FindSubstr((char *)&ModbusH[COM_RS485_2].u8RxBuffer[0],"TX_RS485_1")!=-1)
//			{
//				HAL_UART_Transmit(&huart5,(const uint8_t *)"TX_RS485_2",strlen("TX_RS485_2"),1000);
//			}
//			osDelay(20);
//		}
//		else
//		{
//			if(ModbusH[COM_RS485_2].uModbusType == MB_MASTER)
//			{
//				//for(unsigned int QueryNo=0;QueryNo<gNoofQueryStored;QueryNo++)
//				for(unsigned int QueryNo=0;QueryNo<EPROM_Modbus_Quary_Detail.TotalQuery;QueryNo++)
//				{
//					if(telegram[QueryNo].u8Validation == 0)
//					{
//						if(telegram[QueryNo].uPortNo==COM_RS485_2)
//						{
//							memset(ModbusH[COM_RS485_2].u8RxBuffer,0,sizeof(ModbusH[COM_RS485_2].u8RxBuffer));
//							Master_Send_Modbus_Query(&ModbusH[COM_RS485_2],&telegram[QueryNo]);
//							if(ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(2000)))  //  to increase response time || ticket : https://cimcondigital.atlassian.net/browse/IRTU6000PP-29
//							{
//								osDelay(100);
//								Master_Parse_Modbus_Responce(&ModbusH[COM_RS485_2],&telegram[QueryNo]);
//								osDelay(EPROM_General.S_Comm.Rs485_2_Info.S_Poll_Freq);
//
//								//sprintf((char *)print,"RS485_2 Take if(500)r\n");
//								//WriteLog(1, print, 1);
//							}
//							else
//							{
//								osDelay(300);
//								sprintf((char *)print,"RS485_2 port read query slave is not response in 2000 ms\r\n");
//								WriteLog(1, print, 1);
//							}
//						}
//					}
//					else
//					{
//						sprintf((char *)print,"RS485_2 Port not Validate for Query u8Validation:%d\r\n",telegram[QueryNo].u8Validation);
//						WriteLog(1, print, 1);
//					}
//				}
//
//				if(flagWriteQueryAvailabe == 1)
//				{
//					if(MODBUS_Write[0].mPortSelection_write == COM_RS485_2)
//					{
//						flagWriteQueryAvailabe = 0;//MODBUS_Write[i].mPortSelection_write
//						//telegram[127].uQueryNo = i;
//						telegram[124].uPortNo = MODBUS_Write[0].mPortSelection_write;//EPROM_Modbus_Quary_Detail.Mod_Quary[i].mPortSelection;
//						telegram[124].u8id = MODBUS_Write[0].mSlaveId_write;//EPROM_Modbus_Quary_Detail.Mod_Quary[i].mSlaveId;          /*!< Slave address between 1 and 247. 0 means broadcast */
//						telegram[124].u8fct = MODBUS_Write[0].mFunctionCode_write;         /*!< Function code: 1, 2, 3, 4, 5, 6, 15 or 16 */
//						telegram[124].u16RegAdd = MODBUS_Write[0].mRegStartAddr_write;    /*!< Address of the first register to access at slave/s */
//						telegram[124].u16CoilsNo = MODBUS_Write[0].mNoOfRegister_write;   /*!< Number of coils or registers to access */
//						telegram[124].u16reg[0] = MODBUS_Write[0].mValue_write;
//						telegram[124].uDataType = MODBUS_Write[0].mDataType_write;
//						telegram[124].uQueryNo = MODBUS_Write[0].mWriteQueryNumber;
//
//						memset(ModbusH[COM_RS485_2].u8RxBuffer,0,sizeof(ModbusH[COM_RS485_2].u8RxBuffer));
//						Master_Send_Modbus_Query(&ModbusH[COM_RS485_2],&telegram[124]);
//
//						if(ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(2000))) //  to increase response time || ticket : https://cimcondigital.atlassian.net/browse/IRTU6000PP-29
//						{
//							osDelay(100);
//							Master_Parse_Modbus_Responce(&ModbusH[COM_RS485_2],&telegram[124]);
//							osDelay(EPROM_General.S_Comm.Rs485_2_Info.S_Poll_Freq);
//						}
//						else
//						{
//							sprintf((char *)print,"RS485_2 port write query slave is not response in 2000 ms\r\n");
//							WriteLog(1, print, 1);
//						}
//					}
//					sprintf((char *)print,"RS485_2 Port Write Query\r\n");
//					WriteLog(1, print, 1);
//				}
//				osDelay (1000);
//			}
//			else
//			{
//				if(ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(2000)))
//				{
//					//ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
//					osDelay(50);
//					ProcessModbusSlave(&ModbusH[COM_RS485_2]);
//					osDelay(50);
//				}
//			}
//		}
//	}
//}
