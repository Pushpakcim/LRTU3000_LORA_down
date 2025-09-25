/*
 * DIDO.c
 *
 *  Created on: Dec 28, 2022
 *      Author: SanketP
 */

#include "main.h"
#include "DIDO.h"
osThreadId DIDO_TaskHandle;
unsigned char bit_status[16]={0};
float DO_Final_value[MAX_DO_CHANNEL];
float DI_Final_value[MAX_DI_CHANNEL];
unsigned char test_DI_Final_value[MAX_DI_CHANNEL];
unsigned char  DO_KEY_Array[32];
unsigned char Set_to_switch,BLE_led_status,BLE_led_status1,GPS_led_status,GPS_led_status1;
int count_DO;
unsigned int DI1_Pulse_Count=0,DI2_Pulse_Count=0,DI1_Pulse_Count_for_Frequency=0,DI2_Pulse_Count_for_Frequency=0;
float DI1_Freq=0.0,DI2_Freq=0.0;
float DI1_Freq_fromWaveLength=0.0000,DI2_Freq_fromWaveLength=0.0000;
uint64_t actualPulse_DI_frequency_time=0;
GPIO_TypeDef* DO_GPIO_Port_Array[] = {DO1_GPIO_Port, DO2_GPIO_Port, DO3_GPIO_Port, DO4_GPIO_Port, DO5_GPIO_Port,
		DO6_GPIO_Port, DO7_GPIO_Port, DO8_GPIO_Port, DO9_GPIO_Port, DO10_GPIO_Port,
		DO11_GPIO_Port, DO12_GPIO_Port, DO13_GPIO_Port, DO14_GPIO_Port, DO15_GPIO_Port,
		DO16_GPIO_Port, DO17_GPIO_Port, DO18_GPIO_Port, DO19_GPIO_Port, DO20_GPIO_Port,
		DO21_GPIO_Port, DO22_GPIO_Port, DO23_GPIO_Port, DO24_GPIO_Port, DO25_GPIO_Port, DO26_GPIO_Port};
uint16_t DO_Pin_Array[] = {DO1_Pin, DO2_Pin, DO3_Pin, DO4_Pin, DO5_Pin, DO6_Pin, DO7_Pin, DO8_Pin, DO9_Pin, DO10_Pin,
		DO11_Pin, DO12_Pin, DO13_Pin, DO14_Pin, DO15_Pin, DO16_Pin, DO17_Pin, DO18_Pin, DO19_Pin, DO20_Pin,
		DO21_Pin, DO22_Pin, DO23_Pin, DO24_Pin, DO25_Pin, DO26_Pin};

char Dual_DO_Pulse_Stage[15];
char DO_PulseIsSet_array[30];
unsigned short int DO_actual_PulseWidth[30];
unsigned short int DO_actual_PulseWidth_Count[30];
unsigned short int Dual_DO_actual_PulseWidth[30];
unsigned short int Dual_DO_actual_PulseWidth_Count[30];
/**************************************************************************//**
 * Function name 	: DO_DI_start
 * arguments		: 1)
 * return 		 	: no return type
 * Note				: #
 *****************************************************************************/

void DIDO_start()
{
	osThreadDef(DIDOTask, StartDIDOTask, osPriorityNormal, 0, 512*8); //512
	DIDO_TaskHandle = osThreadCreate(osThread(DIDOTask), NULL);
}

/**************************************************************************//**
 * Function name 	: StartDIDOTask
 * arguments		: 1)
 * return 		 	:
 * Note				:
 * 					:
 * 					:
 *****************************************************************************/
xTimerHandle PTHandle;
void TimerCallback (xTimerHandle xTimer);
void StartDIDOTask(void const * argument)
{
//	//HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
//	set_pulse_do_polarity();
//	osDelay(10);
//	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
//
//	PTHandle = xTimerCreate("timer1ms", pdMS_TO_TICKS(1), pdTRUE, (void *) 1, TimerCallback); //Periodic Timer Create for telemetry Send logic.
//	xTimerStart(PTHandle,0);
//
//	HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
//
//	lcd_clear();
//	lcd_set_cursor(1,0);
//	if(Pro_Application_flag == 1)
//	{
//		lcd_display_string("   PRODUCTION   ");
//		//lcd_display_string("FLOW 1:");
//		lcd_set_cursor(2,0);
//		//lcd_display_string("FLOW 2:");
//		lcd_display_string("      MODE      ");
//	}
//	else
//	{
//		//lcd_display_string("SET FL:");
//		lcd_display_string("FLOW 1:");
//		lcd_set_cursor(2,0);
//		lcd_display_string("FLOW 2:");
//		//lcd_display_string("ACT FL:");
//	}

osDelay(1000);
	for(;;)
	{
		if(Pro_Application_flag==0)
		{
		lcd_clear();
		lcd_set_cursor(1,0);
		lcd_display_string("FR1 L/S:");
		lcd_set_cursor(2,0);
		lcd_display_string("FR2 L/S:");
		lcd_set_cursor(1,8);
		lcd_float_print(EPROM_General.Pulse_DO_DI_Detail.Pulse_DI_Detail[0].Pulse_DI_Flow_Calculated,3);
		lcd_set_cursor(2,8);
		lcd_float_print(EPROM_General.Pulse_DO_DI_Detail.Pulse_DI_Detail[1].Pulse_DI_Flow_Calculated,3);
		osDelay(2000);
		lcd_clear();
		lcd_set_cursor(1,0);
		lcd_display_string("TF1 m3:");
		lcd_set_cursor(2,0);
		lcd_display_string("TF2 m3:");
		lcd_set_cursor(1,7);
		lcd_float_print(gFinalAnaValF[712],2);
		lcd_set_cursor(2,7);
		lcd_float_print(gFinalAnaValF[713],2);
		osDelay(2000);
		}

/*
		//count_DO++;
		count_DO = 0;
		flag_flashUpdateEPROM_PCBPLC_GENERAL_REG_WaitCounter++;

		#ifdef WATCH_DOG_ENABLE //MAU_EXCEPTION
			HAL_IWDG_Refresh(&hiwdg1);
		#endif

		//if(count_DO < 3600) // External Watch dog  //(ProTest_Request) & (0x1<<4)
		if((proTestRequest) & (0x1<<PRODUCTION_TEST_BIT_WATCHDOG))
		{

		}
		else
		{
			HAL_GPIO_TogglePin(Watchdog_GPIO_Port, Watchdog_Pin);
		}

		if(Pro_Application_flag == 1)
		{
			pro_checkDIDOState();
		}
		else
		{
			Status_led_BLE_GPS();
			lcd_set_cursor(1,8);
			lcd_float_print(EPROM_General.Pulse_DO_DI_Detail.Pulse_DI_Detail[0].Pulse_DI_Flow_Calculated,3);
			lcd_set_cursor(2,8);
			lcd_float_print(EPROM_General.Pulse_DO_DI_Detail.Pulse_DI_Detail[1].Pulse_DI_Flow_Calculated,3);
		}

		ScanDI();
		//set_do(); // Temp

		if(flag_flashUpdateEPROM_General == 1)
		{
			flag_flashUpdateEPROM_General_WaitCounter--;
			if(flag_flashUpdateEPROM_General_WaitCounter == 0)
			{
				flag_flashUpdateEPROM_General = 0;
				if(xSemaphoreTake(sendExternalFlashSemaphore, 1000) == pdTRUE )
				{
					ExtFlash_update_EPROM_General();
					xSemaphoreGive(sendExternalFlashSemaphore);
				}
				else
				{
					flag_flashUpdateEPROM_General = 1;
					flag_flashUpdateEPROM_General_WaitCounter = 1;
				}
			}

		}

		if(flag_flashUpdateEPROM_Frequent == 1)
		{
			flag_flashUpdateEPROM_Frequent_WaitCounter--;
			if(flag_flashUpdateEPROM_Frequent_WaitCounter == 0)
			{
				flag_flashUpdateEPROM_Frequent = 0;
				if(xSemaphoreTake(sendExternalFlashSemaphore, 1000) == pdTRUE )
				{
					ExtFlash_update_EPROM_Frequent();
					xSemaphoreGive(sendExternalFlashSemaphore);
				}
				else
				{
					flag_flashUpdateEPROM_Frequent = 1;
					flag_flashUpdateEPROM_Frequent_WaitCounter = 1;
				}
			}
		}

		if(flag_flashUpdateEPROM_AI_Calibration == 1)
		{
			flag_flashUpdateEPROM_AI_Calibration_WaitCounter--;
			if(flag_flashUpdateEPROM_AI_Calibration_WaitCounter == 0)
			{
				flag_flashUpdateEPROM_AI_Calibration = 0;
				if(xSemaphoreTake(sendExternalFlashSemaphore, 1000) == pdTRUE )
				{
					ExtFlash_update_EPROM_AI_Calibration();
					xSemaphoreGive(sendExternalFlashSemaphore);
				}
				else
				{
					flag_flashUpdateEPROM_AI_Calibration = 1;
					flag_flashUpdateEPROM_AI_Calibration_WaitCounter = 1;
				}
			}

		}

		if(flag_flashUpdateEPROM_Schedule == 1)
		{
			flag_flashUpdateEPROM_Schedule_WaitCounter--;
			if(flag_flashUpdateEPROM_Schedule_WaitCounter == 0)
			{
				flag_flashUpdateEPROM_Schedule = 0;
				if(xSemaphoreTake(sendExternalFlashSemaphore, 1000) == pdTRUE )
				{
					ExtFlash_update_EPROM_Schedule();
					xSemaphoreGive(sendExternalFlashSemaphore);
				}
				else
				{
					flag_flashUpdateEPROM_Schedule = 1;
					flag_flashUpdateEPROM_Schedule_WaitCounter = 1;
				}
			}

		}

		if(flag_flashUpdateEPROM_LORA == 1)
		{
			flag_flashUpdateEPROM_LORA_WaitCounter--;
			if(flag_flashUpdateEPROM_LORA_WaitCounter == 0)
			{
				flag_flashUpdateEPROM_LORA = 0;
				if(xSemaphoreTake(sendExternalFlashSemaphore, 1000) == pdTRUE )
				{
					ExtFlash_update_EPROM_LORA();
					xSemaphoreGive(sendExternalFlashSemaphore);
				}
				else
				{
					flag_flashUpdateEPROM_LORA = 1;
					flag_flashUpdateEPROM_LORA_WaitCounter = 1;
				}
			}

		}

		if(flag_flashUpdateEPROM_Modbus_Quary_Detail == 1)
		{
			flag_flashUpdateEPROM_Modbus_Quary_Detail_WaitCounter--;
			if(flag_flashUpdateEPROM_Modbus_Quary_Detail_WaitCounter == 0)
			{
				flag_flashUpdateEPROM_Modbus_Quary_Detail = 0;
				if(xSemaphoreTake(sendExternalFlashSemaphore, 1000) == pdTRUE )
				{
					ExtFlash_update_EPROM_Modbus_Quary_Detail();
					xSemaphoreGive(sendExternalFlashSemaphore);
				}
				else
				{
					flag_flashUpdateEPROM_Modbus_Quary_Detail = 1;
					flag_flashUpdateEPROM_Modbus_Quary_Detail_WaitCounter = 1;
				}
			}
		}

		if(flag_flashSaveRecipe == 1)
		{
			flag_flashSaveRecipe_WaitCounter--;
			if(flag_flashSaveRecipe_WaitCounter==0)
			{
				flag_flashSaveRecipe = 0;
		        if(xSemaphoreTake(sendExternalFlashSemaphore, 1000) == pdTRUE )
				{
		        	WriteModifiedRecipeFile(MODIFIED_RECIPE_FILE_PATH);
					xSemaphoreGive(sendExternalFlashSemaphore);
				}
		        else
		        {
		        	flag_flashSaveRecipe = 1;
		        	flag_flashSaveRecipe_WaitCounter = 1;
		        }
			}
		}

		if(flag_flashUpdateEPROM_PCBPLC_GENERAL_REG_WaitCounter == 60)
		{
			flag_flashUpdateEPROM_PCBPLC_GENERAL_REG_WaitCounter = 0;
			if(xSemaphoreTake(sendExternalFlashSemaphore, 1000) == pdTRUE )
			{
				for(int i=0;i<MAX_GENERAL_AI;i++)
				{
					EPROM_PCBPLC_General_Reg.General_Reg[i]=gFinalAnaValF[GENERAL_PURPOSE_AI_gFinalAnaValF+i];
				}
				ExtFlash_update_EPROM_PCBPLC_GENERAL_REG();

				EPROM_Frequent.DI1_Pulse = DI1_Pulse_Count;
				EPROM_Frequent.DI2_Pulse = DI2_Pulse_Count;
				ExtFlash_update_EPROM_Frequent();
				xSemaphoreGive(sendExternalFlashSemaphore);
			}
			else
			{
				flag_flashUpdateEPROM_PCBPLC_GENERAL_REG_WaitCounter = 299;
			}
		}
		osDelay(1000);
#if 0
		if(count_DO%60==0)
		{
			if(Set_to_switch)
			{
				for(int i=1;i<9;i++)
				{
					DO_On_Off(SET, i);
					DO_Final_value[i-1]=1;
				}
				Set_to_switch=0;
				HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, SET);
				HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, SET);
				HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, SET);
				HAL_GPIO_WritePin(LED4_GPIO_Port, LED4_Pin, SET);
			}
			else
			{
				for(int i=1;i<9;i++)
				{
					DO_On_Off(RESET, i);
					DO_Final_value[i-1]=0;
				}
				Set_to_switch=1;
				HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, RESET);
				HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, RESET);
				HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, RESET);
				HAL_GPIO_WritePin(LED4_GPIO_Port, LED4_Pin, RESET);
			}
		}
		osDelay(1000);
		ScanDI();
		count_DO++;

#endif */

	}

}


/* Timer Call back Function */
/*******************************************************************************/
volatile unsigned int DO_pulseIsSet=0;
volatile unsigned int Dual_DO_pulseIsSet=0;

char DO1_PulseIsSet = 0;
unsigned short int DO1_actual_PulseWidth = 0;
unsigned short int DO1_actual_PulseWidth_Count = 0;

char DO2_PulseIsSet = 0;
unsigned short int DO2_actual_PulseWidth = 0;
unsigned short int DO2_actual_PulseWidth_Count = 0;

char DO3_PulseIsSet = 0;
unsigned short int DO3_actual_PulseWidth = 0;
unsigned short int DO3_actual_PulseWidth_Count = 0;

char DO4_PulseIsSet = 0;
unsigned short int DO4_actual_PulseWidth = 0;
unsigned short int DO4_actual_PulseWidth_Count = 0;

char DO5_PulseIsSet = 0;
unsigned short int DO5_actual_PulseWidth = 0;
unsigned short int DO5_actual_PulseWidth_Count = 0;

char DO6_PulseIsSet = 0;
unsigned short int DO6_actual_PulseWidth = 0;
unsigned short int DO6_actual_PulseWidth_Count = 0;

char DO7_PulseIsSet = 0;
unsigned short int DO7_actual_PulseWidth = 0;
unsigned short int DO7_actual_PulseWidth_Count = 0;

char DO8_PulseIsSet = 0;
unsigned short int DO8_actual_PulseWidth = 0;
unsigned short int DO8_actual_PulseWidth_Count = 0;

char DO9_PulseIsSet = 0;
unsigned short int DO9_actual_PulseWidth = 0;
unsigned short int DO9_actual_PulseWidth_Count = 0;

char DO10_PulseIsSet = 0;
unsigned short int DO10_actual_PulseWidth = 0;
unsigned short int DO10_actual_PulseWidth_Count = 0;

char DO11_PulseIsSet = 0;
unsigned short int DO11_actual_PulseWidth = 0;
unsigned short int DO11_actual_PulseWidth_Count = 0;

char DO12_PulseIsSet = 0;
unsigned short int DO12_actual_PulseWidth = 0;
unsigned short int DO12_actual_PulseWidth_Count = 0;

char DO13_PulseIsSet = 0;
unsigned short int DO13_actual_PulseWidth = 0;
unsigned short int DO13_actual_PulseWidth_Count = 0;

char DO14_PulseIsSet = 0;
unsigned short int DO14_actual_PulseWidth = 0;
unsigned short int DO14_actual_PulseWidth_Count = 0;

char DO15_PulseIsSet = 0;
unsigned short int DO15_actual_PulseWidth = 0;
unsigned short int DO15_actual_PulseWidth_Count = 0;

char DO16_PulseIsSet = 0;
unsigned short int DO16_actual_PulseWidth = 0;
unsigned short int DO16_actual_PulseWidth_Count = 0;

char DO17_PulseIsSet = 0;
unsigned short int DO17_actual_PulseWidth = 0;
unsigned short int DO17_actual_PulseWidth_Count = 0;

char DO18_PulseIsSet = 0;
unsigned short int DO18_actual_PulseWidth = 0;
unsigned short int DO18_actual_PulseWidth_Count = 0;

char DO19_PulseIsSet = 0;
unsigned short int DO19_actual_PulseWidth = 0;
unsigned short int DO19_actual_PulseWidth_Count = 0;

char DO20_PulseIsSet = 0;
unsigned short int DO20_actual_PulseWidth = 0;
unsigned short int DO20_actual_PulseWidth_Count = 0;

char DO21_PulseIsSet = 0;
unsigned short int DO21_actual_PulseWidth = 0;
unsigned short int DO21_actual_PulseWidth_Count = 0;

char DO22_PulseIsSet = 0;
unsigned short int DO22_actual_PulseWidth = 0;
unsigned short int DO22_actual_PulseWidth_Count = 0;

char DO23_PulseIsSet = 0;
unsigned short int DO23_actual_PulseWidth = 0;
unsigned short int DO23_actual_PulseWidth_Count = 0;

char DO24_PulseIsSet = 0;
unsigned short int DO24_actual_PulseWidth = 0;
unsigned short int DO24_actual_PulseWidth_Count = 0;

char DO25_PulseIsSet = 0;
unsigned short int DO25_actual_PulseWidth = 0;
unsigned short int DO25_actual_PulseWidth_Count = 0;

char DO26_PulseIsSet = 0;
unsigned short int DO26_actual_PulseWidth = 0;
unsigned short int DO26_actual_PulseWidth_Count = 0;

unsigned short int DI_Frequency_Counter=0;

unsigned int DI1_waveLength = 0;
unsigned int DI2_waveLength = 0;
void TimerCallback (xTimerHandle xTimer)
{

	DI1_waveLength++;//++;
	DI2_waveLength++;//++;
	//DI_Frequency_Counter+=2;//++;
	if(++DI_Frequency_Counter>EPROM_Frequent.Pulse_DI_frequency_time)  //3000
	{
		DI1_Freq = DI1_Pulse_Count_for_Frequency;///5.0;//((DI1_Pulse_Count-preDI1_Pulse_Count)/3.0);
		DI2_Freq = DI2_Pulse_Count_for_Frequency;///5.0;//((DI2_Pulse_Count-preDI2_Pulse_Count)/3.0);

		DI1_Pulse_Count_for_Frequency=0;
		DI2_Pulse_Count_for_Frequency=0;

		DI_Frequency_Counter=0;
	}

//	DI1_waveLength+=2;//++;
//	DI2_waveLength+=2;//++;
//	DI_Frequency_Counter+=2;//++;
//	if(DI_Frequency_Counter>EPROM_Frequent.Pulse_DI_frequency_time)  //3000
//	{
//		DI1_Freq = DI1_Pulse_Count_for_Frequency;///5.0;//((DI1_Pulse_Count-preDI1_Pulse_Count)/3.0);
//		DI2_Freq = DI2_Pulse_Count_for_Frequency;///5.0;//((DI2_Pulse_Count-preDI2_Pulse_Count)/3.0);
//
//		DI1_Pulse_Count_for_Frequency=0;
//		DI2_Pulse_Count_for_Frequency=0;
//
//		DI_Frequency_Counter=0;
//	}


//	if(DO_pulseIsSet)
//	{
//		int temp_tDoIdx = 0;
//		for(int tDoIdx = 0 ; tDoIdx < EPROM_General.Pulse_DO_DI_Detail.Total_Pulse_Do ; ++tDoIdx)
//		{
//			temp_tDoIdx = tDoIdx/2;
//			if(DO_PulseIsSet_array[tDoIdx] == 1)
//			{
//				DO_actual_PulseWidth_Count[tDoIdx]+=2;//++
//				if(DO_actual_PulseWidth_Count[tDoIdx]>DO_actual_PulseWidth[tDoIdx])
//				{
//					//HAL_GPIO_TogglePin(DO_GPIO_Port_Array[tDoIdx], DO_Pin_Array[tDoIdx]);
//					HAL_GPIO_WritePin(DO_GPIO_Port_Array[tDoIdx], DO_Pin_Array[tDoIdx], EPROM_General.Pulse_DO_DI_Detail.Pulse_DO_Detail[tDoIdx].Pulse_DO_polarity?GPIO_PIN_SET:GPIO_PIN_RESET);
//					DO_PulseIsSet_array[tDoIdx] = 0;
//					DO_pulseIsSet &= ~(1 << tDoIdx);
//					if(EPROM_General.Pulse_DO_DI_Detail.Pulse_DO_Detail[tDoIdx].Pulse_DO_type==3)
//					{
//						//DO_pulseIsSet &= ~(1 << tDoIdx);
//						if(Dual_DO_Pulse_Stage[temp_tDoIdx] == 2)
//						{
//							Dual_DO_actual_PulseWidth_Count[temp_tDoIdx] = 0;
//
//							Dual_DO_Pulse_Stage[temp_tDoIdx] = 3;
//							gFinalAnaValF[CONFIG_PARA_PULSE_DUAL_DO_gFinalAnaValF + (tDoIdx)] = 3; //((temp_tDoIdx)*2)
//							Dual_DO_pulseIsSet |= (1<<(temp_tDoIdx));
//							//EPROM_General.Pulse_DO_DI_Detail.Pulse_DO_Detail[tDoIdx+1].Pulse_DO_Count = 1;
//							//Dual_DO_actual_PulseWidth_Count[tDoIdx/2] = 0;
//						}
//						else if(Dual_DO_Pulse_Stage[(tDoIdx-1)/2] == 4)
//						{
//							Dual_DO_Pulse_Stage[(tDoIdx-1)/2] = 0;
//							Dual_DO_pulseIsSet &= ~(1 << ((tDoIdx-1)/2));
//
//							gFinalAnaValF[CONFIG_PARA_PULSE_DUAL_DO_gFinalAnaValF + ((tDoIdx-1))] = 0;
//						}
//					}
//				}
//			}
//		}
//	}
//
//	if(Dual_DO_pulseIsSet)
//	{
//		int temp_tDoIdx = 0;
//		for(int tDoIdx = 0 ; tDoIdx < EPROM_General.Pulse_DO_DI_Detail.Total_Pulse_Do ; tDoIdx+=2)
//		{
//			temp_tDoIdx = tDoIdx/2;
//			if(Dual_DO_Pulse_Stage[temp_tDoIdx] == 3)  // Dual_DO : First Do operation done
//			{
//				Dual_DO_actual_PulseWidth_Count[temp_tDoIdx]+=2;//++
//				if(Dual_DO_actual_PulseWidth_Count[temp_tDoIdx]>Dual_DO_actual_PulseWidth[temp_tDoIdx])
//				{
//					Dual_DO_Pulse_Stage[temp_tDoIdx] = 4;
//					gFinalAnaValF[CONFIG_PARA_PULSE_DUAL_DO_gFinalAnaValF + (tDoIdx)] = 4; // ((tDoIdx/2)*2)
//
//					Dual_DO_actual_PulseWidth_Count[temp_tDoIdx] = 0;
//
//					//if((EPROM_General.Pulse_DO_DI_Detail.Pulse_DO_Detail[tDoIdx+1].Pulse_DO_Count) > 0)
//					//{
//					//	EPROM_General.Pulse_DO_DI_Detail.Pulse_DO_Detail[tDoIdx+1].Pulse_DO_Count--;
//					//	gFinalAnaValF[CONFIG_PARA_PULSE_DO_gFinalAnaValF + ((tDoIdx+1)*5) + 3 ] = EPROM_General.Pulse_DO_DI_Detail.Pulse_DO_Detail[tDoIdx+1].Pulse_DO_Count;
//						DO_actual_PulseWidth[tDoIdx+1] = EPROM_General.Pulse_DO_DI_Detail.Pulse_DO_Detail[tDoIdx+1].Pulse_DO_Width*EPROM_General.Pulse_DO_DI_Detail.Pulse_DO_Detail[tDoIdx+1].Pulse_DO_Width_scale;
//						DO_PulseIsSet_array[tDoIdx+1] = 1;
//						DO_actual_PulseWidth_Count[tDoIdx+1] = 0;
//						DO_pulseIsSet |= (1 << (tDoIdx+1));
//						HAL_GPIO_WritePin(DO_GPIO_Port_Array[tDoIdx+1], DO_Pin_Array[tDoIdx+1], EPROM_General.Pulse_DO_DI_Detail.Pulse_DO_Detail[tDoIdx+1].Pulse_DO_polarity?GPIO_PIN_RESET:GPIO_PIN_SET);
//					//}
//				}
//			}
//		}
//	}
}

void pro_checkDIDOState()
{
	unsigned char i,pro_Do_State_off[26],pro_Do_State_on[26],pro_Do_State_off1[26];

	if(pro_DO_DI_TestFinish == 0 && EPROM_General.slot_id !=0)
	{
		for(unsigned char i=1;i<=10;i++)
		{
			DoTurnOn(i); // it will make pin low
		}

		osDelay(2000);
		ScanDI();
		for(i=0;i<6;i++)
		{
			pro_Do_State_off[i]=test_DI_Final_value[i];
		}

		pro_Do_State_off[6]=test_DI_Final_value[6];
		pro_Do_State_off[7]=test_DI_Final_value[6];

		pro_Do_State_off[8]=test_DI_Final_value[7];
		pro_Do_State_off[9]=test_DI_Final_value[7];

		for(i=1;i<=10;i++)
		{
			DoTurnOff(i); // it will make pin high
		}
		osDelay(2000);
		ScanDI();
		for(i=0;i<6;i++)
		{
			pro_Do_State_on[i]=test_DI_Final_value[i];
		}
		pro_Do_State_on[6]=test_DI_Final_value[6];
		pro_Do_State_on[7]=test_DI_Final_value[6];

		pro_Do_State_on[8]=test_DI_Final_value[7];
		pro_Do_State_on[9]=test_DI_Final_value[7];

		for(i=7;i<=10;i++)
		{
			Do_RESET_SET(i);
		}
		osDelay(2000);
		ScanDI();
		pro_Do_State_off1[6]=test_DI_Final_value[6];
		pro_Do_State_off1[7]=test_DI_Final_value[6];

		pro_Do_State_off1[8]=test_DI_Final_value[7];
		pro_Do_State_off1[9]=test_DI_Final_value[7];
		for(i=7;i<=10;i++)
		{
			DoTurnOff(i);
		}
		osDelay(1000);
		for(unsigned char i=11;i<=26;i++)
		{
			DoTurnOn(i); // it will make pin low
		}
		osDelay(2000);
		ScanDI();
//		for(i=0;i<8;i++)
//		{
//			pro_Do_State_off[11+i]=test_DI_Final_value[i];
//		}
			pro_Do_State_off[10]=test_DI_Final_value[0];
			pro_Do_State_off[11]=test_DI_Final_value[0];

			pro_Do_State_off[12]=test_DI_Final_value[1];
			pro_Do_State_off[13]=test_DI_Final_value[1];

			pro_Do_State_off[14]=test_DI_Final_value[2];
			pro_Do_State_off[15]=test_DI_Final_value[2];

			pro_Do_State_off[16]=test_DI_Final_value[3];
			pro_Do_State_off[17]=test_DI_Final_value[3];

			pro_Do_State_off[18]=test_DI_Final_value[4];
			pro_Do_State_off[19]=test_DI_Final_value[4];

			pro_Do_State_off[20]=test_DI_Final_value[5];
			pro_Do_State_off[21]=test_DI_Final_value[5];

			pro_Do_State_off[22]=test_DI_Final_value[6];
			pro_Do_State_off[23]=test_DI_Final_value[6];

			pro_Do_State_off[24]=test_DI_Final_value[7];
			pro_Do_State_off[25]=test_DI_Final_value[7];


		for(i=11;i<=26;i++)
		{
			DoTurnOff(i); // it will make pin high
		}
		osDelay(2000);
		ScanDI();
//		for(i=0;i<8;i++)
//		{
//			pro_Do_State_on[8+i]=test_DI_Final_value[i];
//		}

			pro_Do_State_on[10]=test_DI_Final_value[0];
			pro_Do_State_on[11]=test_DI_Final_value[0];

			pro_Do_State_on[12]=test_DI_Final_value[1];
			pro_Do_State_on[13]=test_DI_Final_value[1];

			pro_Do_State_on[14]=test_DI_Final_value[2];
			pro_Do_State_on[15]=test_DI_Final_value[2];

			pro_Do_State_on[16]=test_DI_Final_value[3];
			pro_Do_State_on[17]=test_DI_Final_value[3];

			pro_Do_State_on[18]=test_DI_Final_value[4];
			pro_Do_State_on[19]=test_DI_Final_value[4];

			pro_Do_State_on[20]=test_DI_Final_value[5];
			pro_Do_State_on[21]=test_DI_Final_value[5];

			pro_Do_State_on[22]=test_DI_Final_value[6];
			pro_Do_State_on[23]=test_DI_Final_value[6];

			pro_Do_State_on[24]=test_DI_Final_value[7];
			pro_Do_State_on[25]=test_DI_Final_value[7];

		for(i=11;i<=26;i++)
		{
			Do_RESET_SET(i);
		}
		osDelay(2000);
		ScanDI();
			pro_Do_State_off1[10]=test_DI_Final_value[0];
			pro_Do_State_off1[11]=test_DI_Final_value[0];

			pro_Do_State_off1[12]=test_DI_Final_value[1];
			pro_Do_State_off1[13]=test_DI_Final_value[1];

			pro_Do_State_off1[14]=test_DI_Final_value[2];
			pro_Do_State_off1[15]=test_DI_Final_value[2];

			pro_Do_State_off1[16]=test_DI_Final_value[3];
			pro_Do_State_off1[17]=test_DI_Final_value[3];

			pro_Do_State_off1[18]=test_DI_Final_value[4];
			pro_Do_State_off1[19]=test_DI_Final_value[4];

			pro_Do_State_off1[20]=test_DI_Final_value[5];
			pro_Do_State_off1[21]=test_DI_Final_value[5];

			pro_Do_State_off1[22]=test_DI_Final_value[6];
			pro_Do_State_off1[23]=test_DI_Final_value[6];

			pro_Do_State_off1[24]=test_DI_Final_value[7];
			pro_Do_State_off1[25]=test_DI_Final_value[7];

		for(i=11;i<=26;i++)
		{
			DoTurnOff(i);
		}
		osDelay(1000);
/*
		for(unsigned char i=17;i<=24;i++)
		{
			DoTurnOn(i); // it will make pin low
		}
		osDelay(3000);
		ScanDI();
		for(i=0;i<8;i++)
		{
			pro_Do_State_off[16+i]=test_DI_Final_value[i];
		}
		for(i=17;i<=24;i++)
		{
			DoTurnOff(i); // it will make pin high
		}
		osDelay(3000);
		ScanDI();
		for(i=0;i<8;i++)
		{
			pro_Do_State_on[16+i]=test_DI_Final_value[i];
		}

		for(unsigned char i=25;i<=26;i++)
		{
			DoTurnOn(i); // it will make pin low
		}
		osDelay(3000);
		ScanDI();
		for(i=0;i<2;i++)
		{
			pro_Do_State_off[24+i]=test_DI_Final_value[i];
		}
		for(i=25;i<=26;i++)
		{
			DoTurnOff(i); // it will make pin high
		}
		osDelay(3000);
		ScanDI();
		for(i=0;i<2;i++)
		{
			pro_Do_State_on[24+i]=test_DI_Final_value[i];
		}
*/
		for(i=0;i<8;i++)
		{
			pro_DI_State[i]=1;
		}

		for(i=0;i<=5;i++)
		{
			if(((pro_Do_State_off[i] == 0)&&(pro_Do_State_on[i] == 1)))
			{
				pro_DO_State[i] = 1;
			}
			else
			{
				pro_DO_State[i] = 0;
				if(i<6)
				{
					pro_DI_State[i] = 0;
				}
			}
		}

		for(i=6;i<=25;i++)
		{
			if(((pro_Do_State_off[i] == 0)&&(pro_Do_State_on[i] == 1)&&(pro_Do_State_off1[i] == 0)))
			{
				pro_DO_State[i] = 1;
			}
			else
			{
				pro_DO_State[i] = 0;

				if(i==6)
				{
					pro_DI_State[6] = 0;
				}
				if(i==8)
				{
					pro_DI_State[7] = 0;
				}
//				if(i<8)
//				{
//					pro_DI_State[i] = 0;
//				}
//				else if((i>=8)&&(i<16))
//				{
//					pro_DI_State[i-8] = 0;
//				}
//				else if((i>=16)&&(i<24))
//				{
//					pro_DI_State[i-16] = 0;
//				}
//				else if((i>=24)&&(i<26))
//				{
//					pro_DI_State[i-24] = 0;
//				}
			}
//			if(i >= 7 && pro_DO_State[i] == 1 && i % 2 == 1)
//			{
//				pro_DO_State[i + 1] = 1;
//			}
			pro_DO_DI_TestFinish = 1;
			flagLORAPubLogData=1;
//			flagLORAPubLogData_fail = 255;
		}
	}

	if(EPROM_General.slot_id==0 || key_LED_flag == 1)
	{

		key_LED_flag=0;
		for(;;)
		{
			HAL_GPIO_TogglePin(Watchdog_GPIO_Port, Watchdog_Pin);  // AN added for watchdog refresh

			if(HAL_GPIO_ReadPin(KEY_IN_GPIO_Port, KEY_IN_Pin))
			{
				HAL_GPIO_WritePin(DISPLAY_BKLT_GPIO_Port, DISPLAY_BKLT_Pin, RESET);
				HAL_GPIO_WritePin(LED_LORA_GPIO_Port, LED_LORA_Pin, RESET);
				lcd_clear();
				lcd_set_cursor(1,0);
				lcd_display_string("   PRODUCTION   ");
				lcd_set_cursor(2,0);
				lcd_display_string("  KEY NOT PRESS  ");
				osDelay(1000);

			}
			else
			{
				HAL_GPIO_WritePin(DISPLAY_BKLT_GPIO_Port, DISPLAY_BKLT_Pin, SET);
				HAL_GPIO_WritePin(LED_LORA_GPIO_Port, LED_LORA_Pin, SET);
				lcd_clear();
				lcd_set_cursor(1,0);
				lcd_display_string("   PRODUCTION   ");
				lcd_set_cursor(2,0);
				lcd_display_string("  KEY PRESS ");
				osDelay(1000);

			}
		}
	}
}

/***************************************************************
Function Name : DO 											   *
Inputs:			OnBoard , TurnOn/Off , BitNumber)	   *
OutPuts:		It Alter a Bit on specified DO				   *
****************************************************************/
void DO_On_Off(short State,short BitNumber)
{
	if(State==SET)
	{
		DoTurnOn(BitNumber);
	}
	else if(State==RESET)
	{
		DoTurnOff(BitNumber);
	}
	else
	{
	}
}

/***************************************************************
This Function SET a bit on  DO(8-Bit) for a                *
															   *						   *															   *
Input:	Bit no												   *
****************************************************************/
void DoTurnOn(unsigned short bit_no)
{
	switch(bit_no)
	{
		case 1:
				HAL_GPIO_WritePin(DO1_GPIO_Port, DO1_Pin, SET);
				break;
		case 2:
				HAL_GPIO_WritePin(DO2_GPIO_Port, DO2_Pin, SET);
				break;
		case 3:
				HAL_GPIO_WritePin(DO3_GPIO_Port, DO3_Pin, SET);
				break;
		case 4:
				HAL_GPIO_WritePin(DO4_GPIO_Port, DO4_Pin, SET);
				break;
		case 5:
				HAL_GPIO_WritePin(DO5_GPIO_Port, DO5_Pin, SET);
				break;
		case 6:
				HAL_GPIO_WritePin(DO6_GPIO_Port, DO6_Pin, SET);
				break;
		case 7:
				HAL_GPIO_WritePin(DO7_GPIO_Port, DO7_Pin, SET);
				break;
		case 8:
				HAL_GPIO_WritePin(DO8_GPIO_Port, DO8_Pin, RESET);
				break;
		case 9:
				HAL_GPIO_WritePin(DO9_GPIO_Port, DO9_Pin, SET);
				break;
		case 10:
				HAL_GPIO_WritePin(DO10_GPIO_Port, DO10_Pin, RESET);
				break;
		case 11:
				HAL_GPIO_WritePin(DO11_GPIO_Port, DO11_Pin, SET);
				break;
		case 12:
				HAL_GPIO_WritePin(DO12_GPIO_Port, DO12_Pin, RESET);
				break;
		case 13:
				HAL_GPIO_WritePin(DO13_GPIO_Port, DO13_Pin, SET);
				break;
		case 14:
				HAL_GPIO_WritePin(DO14_GPIO_Port, DO14_Pin, RESET);
				break;
		case 15:
				HAL_GPIO_WritePin(DO15_GPIO_Port, DO15_Pin, SET);
				break;
		case 16:
				HAL_GPIO_WritePin(DO16_GPIO_Port, DO16_Pin, RESET);
				break;
		case 17:
				HAL_GPIO_WritePin(DO17_GPIO_Port, DO17_Pin, SET);
				break;
		case 18:
				HAL_GPIO_WritePin(DO18_GPIO_Port, DO18_Pin, RESET);
				break;
		case 19:
				HAL_GPIO_WritePin(DO19_GPIO_Port, DO19_Pin, SET);
				break;
		case 20:
				HAL_GPIO_WritePin(DO20_GPIO_Port, DO20_Pin, RESET);
				break;
		case 21:
				HAL_GPIO_WritePin(DO21_GPIO_Port, DO21_Pin, SET);
				break;
		case 22:
				HAL_GPIO_WritePin(DO22_GPIO_Port, DO22_Pin, RESET);
				break;
		case 23:
				HAL_GPIO_WritePin(DO23_GPIO_Port, DO23_Pin, SET);
				break;
		case 24:
				HAL_GPIO_WritePin(DO24_GPIO_Port, DO24_Pin, RESET);
				break;
		case 25:
				HAL_GPIO_WritePin(DO25_GPIO_Port, DO25_Pin, SET);
				break;
		case 26:
				HAL_GPIO_WritePin(DO26_GPIO_Port, DO26_Pin, RESET);
				break;
	}
}

/***************************************************************
This Function Re-SET a bit on EXT DO(8-Bit) for a             *
															   *
Input:	Bit no												   *
****************************************************************/
void DoTurnOff(unsigned short bit_no)
{
	switch(bit_no)
	{
		case 1:
				HAL_GPIO_WritePin(DO1_GPIO_Port, DO1_Pin, RESET);
				break;
		case 2:
				HAL_GPIO_WritePin(DO2_GPIO_Port, DO2_Pin, RESET);
				break;
		case 3:
				HAL_GPIO_WritePin(DO3_GPIO_Port, DO3_Pin, RESET);
				break;
		case 4:
				HAL_GPIO_WritePin(DO4_GPIO_Port, DO4_Pin, RESET);
				break;
		case 5:
				HAL_GPIO_WritePin(DO5_GPIO_Port, DO5_Pin, RESET);
				break;
		case 6:
				HAL_GPIO_WritePin(DO6_GPIO_Port, DO6_Pin, RESET);
				break;
		case 7:
				HAL_GPIO_WritePin(DO7_GPIO_Port, DO7_Pin, RESET);
				break;
		case 8:
				HAL_GPIO_WritePin(DO8_GPIO_Port, DO8_Pin, RESET);
				break;
		case 9:
				HAL_GPIO_WritePin(DO9_GPIO_Port, DO9_Pin, RESET);
				break;
		case 10:
				HAL_GPIO_WritePin(DO10_GPIO_Port, DO10_Pin, RESET);
				break;
		case 11:
				HAL_GPIO_WritePin(DO11_GPIO_Port, DO11_Pin, RESET);
				break;
		case 12:
				HAL_GPIO_WritePin(DO12_GPIO_Port, DO12_Pin, RESET);
				break;
		case 13:
				HAL_GPIO_WritePin(DO13_GPIO_Port, DO13_Pin, RESET);
				break;
		case 14:
				HAL_GPIO_WritePin(DO14_GPIO_Port, DO14_Pin, RESET);
				break;
		case 15:
				HAL_GPIO_WritePin(DO15_GPIO_Port, DO15_Pin, RESET);
				break;
		case 16:
				HAL_GPIO_WritePin(DO16_GPIO_Port, DO16_Pin, RESET);
				break;
		case 17:
				HAL_GPIO_WritePin(DO17_GPIO_Port, DO17_Pin, RESET);
				break;
		case 18:
				HAL_GPIO_WritePin(DO18_GPIO_Port, DO18_Pin, RESET);
				break;
		case 19:
				HAL_GPIO_WritePin(DO19_GPIO_Port, DO19_Pin, RESET);
				break;
		case 20:
				HAL_GPIO_WritePin(DO20_GPIO_Port, DO20_Pin, RESET);
				break;
		case 21:
				HAL_GPIO_WritePin(DO21_GPIO_Port, DO21_Pin, RESET);
				break;
		case 22:
				HAL_GPIO_WritePin(DO22_GPIO_Port, DO22_Pin, RESET);
				break;
		case 23:
				HAL_GPIO_WritePin(DO23_GPIO_Port, DO23_Pin, RESET);
				break;
		case 24:
				HAL_GPIO_WritePin(DO24_GPIO_Port, DO24_Pin, RESET);
				break;
		case 25:
				HAL_GPIO_WritePin(DO25_GPIO_Port, DO25_Pin, RESET);
				break;
		case 26:
				HAL_GPIO_WritePin(DO26_GPIO_Port, DO26_Pin, RESET);
				break;
	}
}

void Do_RESET_SET(unsigned short bit_no)
{
	switch(bit_no)
	{
		case 1:
				HAL_GPIO_WritePin(DO1_GPIO_Port, DO1_Pin, RESET);
				break;
		case 2:
				HAL_GPIO_WritePin(DO2_GPIO_Port, DO2_Pin, RESET);
				break;
		case 3:
				HAL_GPIO_WritePin(DO3_GPIO_Port, DO3_Pin, RESET);
				break;
		case 4:
				HAL_GPIO_WritePin(DO4_GPIO_Port, DO4_Pin, RESET);
				break;
		case 5:
				HAL_GPIO_WritePin(DO5_GPIO_Port, DO5_Pin, RESET);
				break;
		case 6:
				HAL_GPIO_WritePin(DO6_GPIO_Port, DO6_Pin, RESET);
				break;
		case 7:
				HAL_GPIO_WritePin(DO7_GPIO_Port, DO7_Pin, RESET);
				break;
		case 8:
				HAL_GPIO_WritePin(DO8_GPIO_Port, DO8_Pin, SET);
				break;
		case 9:
				HAL_GPIO_WritePin(DO9_GPIO_Port, DO9_Pin, RESET);
				break;
		case 10:
				HAL_GPIO_WritePin(DO10_GPIO_Port, DO10_Pin, SET);
				break;
		case 11:
				HAL_GPIO_WritePin(DO11_GPIO_Port, DO11_Pin, RESET);
				break;
		case 12:
				HAL_GPIO_WritePin(DO12_GPIO_Port, DO12_Pin, SET);
				break;
		case 13:
				HAL_GPIO_WritePin(DO13_GPIO_Port, DO13_Pin, RESET);
				break;
		case 14:
				HAL_GPIO_WritePin(DO14_GPIO_Port, DO14_Pin, SET);
				break;
		case 15:
				HAL_GPIO_WritePin(DO15_GPIO_Port, DO15_Pin, RESET);
				break;
		case 16:
				HAL_GPIO_WritePin(DO16_GPIO_Port, DO16_Pin, SET);
				break;
		case 17:
				HAL_GPIO_WritePin(DO17_GPIO_Port, DO17_Pin, RESET);
				break;
		case 18:
				HAL_GPIO_WritePin(DO18_GPIO_Port, DO18_Pin, SET);
				break;
		case 19:
				HAL_GPIO_WritePin(DO19_GPIO_Port, DO19_Pin, RESET);
				break;
		case 20:
				HAL_GPIO_WritePin(DO20_GPIO_Port, DO20_Pin, SET);
				break;
		case 21:
				HAL_GPIO_WritePin(DO21_GPIO_Port, DO21_Pin, RESET);
				break;
		case 22:
				HAL_GPIO_WritePin(DO22_GPIO_Port, DO22_Pin, SET);
				break;
		case 23:
				HAL_GPIO_WritePin(DO23_GPIO_Port, DO23_Pin, RESET);
				break;
		case 24:
				HAL_GPIO_WritePin(DO24_GPIO_Port, DO24_Pin, SET);
				break;
		case 25:
				HAL_GPIO_WritePin(DO25_GPIO_Port, DO25_Pin, RESET);
				break;
		case 26:
				HAL_GPIO_WritePin(DO26_GPIO_Port, DO26_Pin, SET);
				break;
	}
}
/***************************************************************
This Function Scans(Read) On Board DI for all DI   *
									   *
															   *
Input:	void											   *
Return	void									   *
****************************************************************/


void ScanDI(void)
{

	if((Pro_Application_flag == 1)&&((pro_DO_DI_TestFinish == 0)))
	{
		if(HAL_GPIO_ReadPin(DI1_Pulse_GPIO_Port, DI1_Pulse_Pin))
		{
			test_DI_Final_value[0] = 1;
		}
		else
		{
			test_DI_Final_value[0] = 0;
		}

		if(HAL_GPIO_ReadPin(DI2_Pulse_GPIO_Port, DI2_Pulse_Pin))
		{
			test_DI_Final_value[1] = 1;
		}
		else
		{
			test_DI_Final_value[1] = 0;
		}

		if(HAL_GPIO_ReadPin(DI3_GPIO_Port, DI3_Pin))
		{
			test_DI_Final_value[2] = 1;
		}
		else
		{
			test_DI_Final_value[2] = 0;
		}

		if(HAL_GPIO_ReadPin(DI4_GPIO_Port, DI4_Pin))
		{
			test_DI_Final_value[3] = 1;
		}
		else
		{
			test_DI_Final_value[3] = 0;
		}
		if(HAL_GPIO_ReadPin(DI5_GPIO_Port, DI5_Pin))
		{
			test_DI_Final_value[4] = 1;
		}
		else
		{
			test_DI_Final_value[4] = 0;
		}

		if(HAL_GPIO_ReadPin(DI6_GPIO_Port, DI6_Pin))
		{
			test_DI_Final_value[5] = 1;
		}
		else
		{
			test_DI_Final_value[5] = 0;
		}

		if(HAL_GPIO_ReadPin(DI7_GPIO_Port, DI7_Pin))
		{
			test_DI_Final_value[6] = 1;
		}
		else
		{
			test_DI_Final_value[6] = 0;
		}
		if(HAL_GPIO_ReadPin(DI8_GPIO_Port, DI8_Pin))
		{
			test_DI_Final_value[7] = 1;
		}
		else
		{
			test_DI_Final_value[7] = 0;
		}
	}
	else
	{
	//	if(HAL_GPIO_ReadPin(DI1_GPIO_Port, DI1_Pin))
	//	{
	//		DI_Final_value[0] = 1;
	//	}
	//	else
	//	{
	//		DI_Final_value[0] = 0;
	//	}
	//
	//	if(HAL_GPIO_ReadPin(DI2_GPIO_Port, DI2_Pin))
	//	{
	//		DI_Final_value[1] = 1;
	//	}
	//	else
	//	{
	//		DI_Final_value[1] = 0;
	//	}

	if(HAL_GPIO_ReadPin(DI3_GPIO_Port, DI3_Pin))
	{
		DI_Final_value[0] = 1;
	}
	else
	{
		DI_Final_value[0] = 0;
	}

	if(HAL_GPIO_ReadPin(DI4_GPIO_Port, DI4_Pin))
	{
		DI_Final_value[1] = 1;
	}
	else
	{
		DI_Final_value[1] = 0;
	}
	if(HAL_GPIO_ReadPin(DI5_GPIO_Port, DI5_Pin))
	{
		DI_Final_value[2] = 1;
	}
	else
	{
		DI_Final_value[2] = 0;
	}

	if(HAL_GPIO_ReadPin(DI6_GPIO_Port, DI6_Pin))
	{
		DI_Final_value[3] = 1;
	}
	else
	{
		DI_Final_value[3] = 0;
	}

	if(HAL_GPIO_ReadPin(DI7_GPIO_Port, DI7_Pin))
	{
		DI_Final_value[4] = 1;
	}
	else
	{
		DI_Final_value[4] = 0;
	}

	if(HAL_GPIO_ReadPin(DI8_GPIO_Port, DI8_Pin))
	{
		DI_Final_value[5] = 1;
	}
	else
	{
		DI_Final_value[5] = 0;
	}

	//	if(HAL_GPIO_ReadPin(DI9_GPIO_Port, DI9_Pin))
	//	{
	//		DI_Final_value[8] = 1;
	//	}
	//	else
	//	{
	//		DI_Final_value[8] = 0;
	//	}
	//
	//	if(HAL_GPIO_ReadPin(DI10_GPIO_Port, DI10_Pin))
	//	{
	//		DI_Final_value[9] = 1;
	//	}
	//	else
	//	{
	//		DI_Final_value[9] = 0;
	//	}
	//
	//	if(HAL_GPIO_ReadPin(DI11_GPIO_Port, DI11_Pin))
	//	{
	//		DI_Final_value[10] = 1;
	//	}
	//	else
	//	{
	//		DI_Final_value[10] = 0;
	//	}
	//
	//	if(HAL_GPIO_ReadPin(DI12_GPIO_Port, DI12_Pin))
	//	{
	//		DI_Final_value[11] = 1;
	//	}
	//	else
	//	{
	//		DI_Final_value[11] = 0;
	//	}
	//
	//	if(HAL_GPIO_ReadPin(DI13_GPIO_Port, DI13_Pin))
	//	{
	//		DI_Final_value[12] = 1;
	//	}
	//	else
	//	{
	//		DI_Final_value[12] = 0;
	//	}
	//
	//	if(HAL_GPIO_ReadPin(DI14_GPIO_Port, DI14_Pin))
	//	{
	//		DI_Final_value[13] = 1;
	//	}
	//	else
	//	{
	//		DI_Final_value[13] = 0;
	//	}
	//
	//	if(HAL_GPIO_ReadPin(DI15_GPIO_Port, DI15_Pin))
	//	{
	//		DI_Final_value[14] = 1;
	//	}
	//	else
	//	{
	//		DI_Final_value[14] = 0;
	//	}
	//
	//	if(HAL_GPIO_ReadPin(DI16_GPIO_Port, DI16_Pin))
	//	{
	//		DI_Final_value[15] = 1;
	//	}
	//	else
	//	{
	//		DI_Final_value[15] = 0;
	//	}
	}
//	switch(bit_no)
//	{
//		case 1:
//			bit_status = HAL_GPIO_ReadPin(DI1_GPIO_Port, DI1_Pin);
//			break;
//		case 2:
//			bit_status = HAL_GPIO_ReadPin(DI2_GPIO_Port, DI2_Pin);
//			break;
//		case 3:
//			bit_status = HAL_GPIO_ReadPin(DI3_GPIO_Port, DI3_Pin);
//			break;
//		case 4:
//			bit_status = HAL_GPIO_ReadPin(DI4_GPIO_Port, DI4_Pin);
//			break;
//		case 5:
//			bit_status = HAL_GPIO_ReadPin(DI5_GPIO_Port, DI5_Pin);
//			break;
//		case 6:
//			bit_status = HAL_GPIO_ReadPin(DI6_GPIO_Port, DI6_Pin);
//			break;
//		case 7:
//			bit_status = HAL_GPIO_ReadPin(DI7_GPIO_Port, DI7_Pin);
//			break;
//		case 8:
//			bit_status = HAL_GPIO_ReadPin(DI8_GPIO_Port, DI8_Pin);
//			break;
//		case 9:
//			bit_status = HAL_GPIO_ReadPin(DI9_GPIO_Port, DI9_Pin);
//			break;
//		case 10:
//			bit_status = HAL_GPIO_ReadPin(DI10_GPIO_Port, DI10_Pin);
//			break;
//		case 11:
//			bit_status = HAL_GPIO_ReadPin(DI11_GPIO_Port, DI11_Pin);
//			break;
//		case 12:
//			bit_status = HAL_GPIO_ReadPin(DI12_GPIO_Port, DI12_Pin);
//			break;
//		case 13:
//			bit_status = HAL_GPIO_ReadPin(DI13_GPIO_Port, DI13_Pin);
//			break;
//		case 14:
//			bit_status = HAL_GPIO_ReadPin(DI14_GPIO_Port, DI14_Pin);
//			break;
//		case 15:
//			bit_status = HAL_GPIO_ReadPin(DI15_GPIO_Port, DI15_Pin);
//			break;
//		case 16:
//			bit_status = HAL_GPIO_ReadPin(DI16_GPIO_Port, DI16_Pin);
//			break;
//	}
//	return bit_status;
}


void Status_led_BLE_GPS(void)
{
//	if(ble.powerStauts)
//	{
//		if(ble.connectionStatus)
//		{
//			if(BLE_led_status1 < 5)
//			{
//				HAL_GPIO_WritePin(BLE_LED2_GPIO_Port, BLE_LED2_Pin, GPIO_PIN_RESET);
//			}
//			else if(BLE_led_status1 == 5)
//			{
//				HAL_GPIO_WritePin(BLE_LED2_GPIO_Port, BLE_LED2_Pin, GPIO_PIN_SET);
//				BLE_led_status1=0;
//			}
//			BLE_led_status1++;
//		}
//		else
//		{
//
//			if(BLE_led_status)
//			{
//				HAL_GPIO_WritePin(BLE_LED2_GPIO_Port, BLE_LED2_Pin, GPIO_PIN_RESET);
//				BLE_led_status=0;
//			}
//			else
//			{
//				HAL_GPIO_WritePin(BLE_LED2_GPIO_Port, BLE_LED2_Pin, GPIO_PIN_SET);
//				BLE_led_status=1;
//			}
//		}
//	}
//	else
//	{
//		HAL_GPIO_WritePin(BLE_LED2_GPIO_Port, BLE_LED2_Pin, GPIO_PIN_RESET);
//	}
//
//	if(BLE_led_status1 > 5)
//	{
//		BLE_led_status1 = 0;
//	}
//
//	if(gps.GNSS_state)
//	{
//		if(gps.fix)
//		{
//			if(GPS_led_status)
//			{
//				HAL_GPIO_WritePin(GPS_LED1_GPIO_Port, GPS_LED1_Pin, GPIO_PIN_RESET);
//				GPS_led_status=0;
//			}
//			else
//			{
//				HAL_GPIO_WritePin(GPS_LED1_GPIO_Port, GPS_LED1_Pin, GPIO_PIN_SET);
//				GPS_led_status=1;
//			}
//		}
//		else
//		{
//			if(GPS_led_status1 < 5)
//			{
//				HAL_GPIO_WritePin(GPS_LED1_GPIO_Port, GPS_LED1_Pin, GPIO_PIN_RESET);
//			}
//			else if(GPS_led_status1 == 5)
//			{
//				HAL_GPIO_WritePin(GPS_LED1_GPIO_Port, GPS_LED1_Pin, GPIO_PIN_SET);
//				GPS_led_status1=0;
//			}
//			GPS_led_status1++;
//		}
//	}
//	else
//	{
//		HAL_GPIO_WritePin(GPS_LED1_GPIO_Port, GPS_LED1_Pin, GPIO_PIN_RESET);
//	}
//
//	if(GPS_led_status1 > 5)
//	{
//		GPS_led_status1 = 0;
//	}
}
