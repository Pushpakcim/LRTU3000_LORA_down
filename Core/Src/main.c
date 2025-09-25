/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "lwip.h"
#include "stdio.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* ETH_CODE: add lwiperf, see comment in StartDefaultTask function */
#include "lwip/apps/lwiperf.h"
#include "lwip/udp.h"
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

I2C_HandleTypeDef hi2c1;
QSPI_HandleTypeDef hqspi;

RTC_HandleTypeDef hrtc;

IWDG_HandleTypeDef hiwdg1;

UART_HandleTypeDef huart8;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;
CRC_HandleTypeDef hcrc;
osThreadId defaultTaskHandle;
/* USER CODE BEGIN PV */

////*****QSPI****////
unsigned char Volatile_1_Non_VOL_0 = 0;
uint8_t Id_Reg_Arry[3] = {0};
uint8_t reg1 = 0,reg2 = 0, reg3 = 0;
HAL_StatusTypeDef SPI_Status;
unsigned char Tx_Buffer = 'A',Tx_Buffer1;
osSemaphoreId sendExternalFlashSemaphore;

//struct BLE_Config QSPI_Store_para;
//struct BLE_Config QSPI_Store_para1;

//****RTC****//
//struct tm t1;
//struct tm t1,*time_sunset,*time_sunrise;
uint8_t rtctime[20], rtcdate[20];
//uint32_t Get_UTCsunset,Get_UTCsunrise;

//****LORA****//
uint8_t Lora_RX_Buff[50];        	//LoRA buffer to fill from Rx interrupt
lwrb_t lora_rx_rb;  				//LoRA Ring buffer instance for RX data
uint8_t lora_rx_rb_data[1000];		//LoRA Ring buffer data array for RX DMA

uint32_t stm32deviceID[3],STM32_CRC32;
uint8_t stm8deviceID[12];
//uint8_t err=0;
char flag_ethernet_reboot,flag_day_night_reboot;

///////*****END*****////
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MPU_Config(void);
static void MX_GPIO_Init(void);
//static void MX_ADC1_Init(void);
//static void MX_RTC_Init(void);
static void MX_IWDG1_Init(void);
void MX_UART4_Init(void);
//static void MX_UART5_Init(void);
//static void MX_UART8_Init(void);
//static void MX_USART1_UART_Init(void);
//static void MX_USART2_UART_Init(void);
static void MX_QUADSPI_Init(void);
static void MX_CRC_Init(void);
void StartDefaultTask(void const * argument);
void Print_Flash(void);
void Print_Memory_RTOS_Stack(void);
void reboot_device_func(void);
void MX_I2C1_Init(void);
static void MX_USART3_UART_Init(void);
void WriteLog(uint8_t LogEnable,const char *pData,uint8_t logType);
/* USER CODE BEGIN PFP */
cJSON_Hooks rtos_mem_fn = { pvPortMalloc, vPortFree };
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


#ifdef SVC_DEBUG
//======================================================================
//Debug SWV (use for virtual COM Port Using ST Link)
//======================================================================
int __io_putchar(int ch)
{
	ITM_SendChar(ch); return(ch);
}
//======================================================================
//======================================================================
//Debug SWV (use for virtual COM Port Using ST Link)
//======================================================================
int _write(int file, char *ptr, int len)
{
	int DataIdx;
	for (DataIdx = 0; DataIdx < len; DataIdx++)
	{ __io_putchar(*ptr++); }
	return len;
}
//======================================================================
#endif
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */

unsigned int erase_push_counter=0,PushPressTime = 0,Fascia_Pin_status = 2;
int main(void)
{
  /* USER CODE BEGIN 1 */
//	char tDebug[100];
  /* USER CODE END 1 */

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();

  /* Enable I-Cache---------------------------------------------------------*/
  SCB_EnableICache();

  /* Enable D-Cache---------------------------------------------------------*/
  SCB_EnableDCache();

  /* MCU Configuration--------------------------------------------------------*/
  __enable_irq();
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();

  HAL_GPIO_WritePin(DISPLAY_BKLT_GPIO_Port, DISPLAY_BKLT_Pin , GPIO_PIN_SET);

  HAL_GPIO_WritePin(LED_LORA_GPIO_Port,LED_LORA_Pin , GPIO_PIN_SET);
  HAL_Delay(250);
  HAL_GPIO_WritePin(LED_LORA_GPIO_Port, LED_LORA_Pin, GPIO_PIN_RESET);
  HAL_Delay(250);
  HAL_GPIO_WritePin(LED_LORA_GPIO_Port, LED_LORA_Pin, GPIO_PIN_SET);
  HAL_Delay(250);
  HAL_GPIO_WritePin(LED_LORA_GPIO_Port, LED_LORA_Pin, GPIO_PIN_RESET);

  MX_ADC1_Init();
  MX_RTC_Init();
  MX_CRC_Init();
#ifdef WATCH_DOG_ENABLE //MAU_EXCEPTION
  MX_IWDG1_Init();
  HAL_IWDG_Refresh(&hiwdg1);
#endif
  //MX_UART4_Init();
  MX_I2C1_Init();
  MX_QUADSPI_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */

 // *********RTC***********//
 // set_time(t1);

  if(HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR1) != 0x32F2)
	set_time(t1);
  else
	get_time(t1);

  print_time();
  //**************************//

#ifdef WATCH_DOG_ENABLE //MAU_EXCEPTION
	HAL_IWDG_Refresh(&hiwdg1);
#endif

  lwrb_init(&EC200U_RX_rb, EC200U_RX_rb_data, sizeof(EC200U_RX_rb_data));
  lwrb_init(&lora_rx_rb, lora_rx_rb_data, sizeof(lora_rx_rb_data));

  cJSON_InitHooks(&rtos_mem_fn);
  /* USER CODE END 2 */

  /* USER CODE BEGIN RTOS_MUTEX */
  W25Q_Init();
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  stm32deviceID[0]=HAL_GetUIDw0();
  stm32deviceID[1]=HAL_GetUIDw1();
  stm32deviceID[2]=HAL_GetUIDw2();

  stm8deviceID[0] = (stm32deviceID[0]) & 0xff;
  stm8deviceID[1] = (stm32deviceID[0] >> 8) & 0xff;
  stm8deviceID[2] = (stm32deviceID[0] >> 16) & 0xff;
  stm8deviceID[3] = (stm32deviceID[0] >> 24);

  stm8deviceID[4] = stm32deviceID[1] & 0xff;
  stm8deviceID[5] = (stm32deviceID[1] >> 8) & 0xff;
  stm8deviceID[6] = (stm32deviceID[1] >> 16) & 0xff;
  stm8deviceID[7] = (stm32deviceID[1] >> 24);

  stm8deviceID[8] = stm32deviceID[2] & 0xff;
  stm8deviceID[9] = (stm32deviceID[2] >> 8) & 0xff;
  stm8deviceID[10] = (stm32deviceID[3] >> 16) & 0xff;
  stm8deviceID[11] = (stm32deviceID[4] >> 24);

  //STM32_CRC32 = HAL_CRC_Calculate(&hcrc, (uint32_t *)stm32deviceID,3);
  STM32_CRC32 = HAL_CRC_Calculate(&hcrc, (uint32_t *)stm8deviceID,12);

  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */

  /* USER CODE END RTOS_TIMERS */
//  ExtFlash_Read_RuntimePara(1);
//  ExtFlash_Read_EPROM_General(1);
//  ExtFlash_Read_EPROM_AI_Calibration(1);
//  ExtFlash_Read_EPROM_Schedule(1);
//  ExtFlash_Read_EPROM_Modbus_Quary_Detail(1);
//  ExtFlash_Read_EPROM_PCBPLC_GENERAL_REG(1);
//  ExtFlash_Read_gPlcRecFlash(1);
//  ExtFlash_Read_EPROM_Frequent(1);
  /* USER CODE BEGIN RTOS_QUEUES */

#ifdef WATCH_DOG_ENABLE //MAU_EXCEPTION
	HAL_IWDG_Refresh(&hiwdg1);
#endif

  ExtFlash_Read_RuntimePara(0);
  ExtFlash_Read_EPROM_General(0);
  ExtFlash_Read_EPROM_AI_Calibration(0);
  ExtFlash_Read_EPROM_Schedule(0);
  ExtFlash_Read_EPROM_LORA(0);
  ExtFlash_Read_EPROM_Modbus_Quary_Detail(0);
  ExtFlash_Read_EPROM_PCBPLC_GENERAL_REG(0);
  ExtFlash_Read_gPlcRecFlash(0);

//  ExtFlash_Read_RuntimePara(1);
//  ExtFlash_Read_EPROM_General(1);
//  ExtFlash_Read_EPROM_AI_Calibration(1);
//  ExtFlash_Read_EPROM_Schedule(1);
//  ExtFlash_Read_EPROM_LORA(1);
//  ExtFlash_Read_EPROM_Modbus_Quary_Detail(1);
//  ExtFlash_Read_EPROM_PCBPLC_GENERAL_REG(1);
//  ExtFlash_Read_gPlcRecFlash(1);

#ifdef WATCH_DOG_ENABLE
	HAL_IWDG_Refresh(&hiwdg1);
#endif

  ExtFlash_Read_EPROM_Frequent(0);

	for(unsigned char iii=0;iii<15;iii++)
	{
		Dual_DO_Pulse_Stage[iii] = 0;
		Dual_DO_actual_PulseWidth[iii] = 500;
	}

  syncExtFlashVariableWithPCBPLCVariable();
  ExtFlash_Read_OTA_Data();

#ifdef WATCH_DOG_ENABLE
	HAL_IWDG_Refresh(&hiwdg1);
#endif

  {
	  erase_push_counter = 0;
	  Fascia_Pin_status = HAL_GPIO_ReadPin(KEY_IN_GPIO_Port, KEY_IN_Pin);

	  if(HAL_GPIO_ReadPin(KEY_IN_GPIO_Port, KEY_IN_Pin)==GPIO_PIN_RESET)
	  {

		  while((HAL_GPIO_ReadPin(KEY_IN_GPIO_Port, KEY_IN_Pin)==GPIO_PIN_RESET)&&(erase_push_counter<500))
		  {
			  HAL_Delay(10);
			  erase_push_counter++;
		  }

		  if(erase_push_counter >= 500)
		  {
			  HAL_GPIO_WritePin(LED_LORA_GPIO_Port, LED_LORA_Pin, SET);
			  HAL_GPIO_WritePin(DISPLAY_BKLT_GPIO_Port, DISPLAY_BKLT_Pin, SET);

			  erase_push_counter = 0 ;
			  HAL_Delay(1000);
			  if(HAL_GPIO_ReadPin(KEY_IN_GPIO_Port, KEY_IN_Pin)==GPIO_PIN_SET)
			  {
				  while(erase_push_counter<500)
				  {
					  if(HAL_GPIO_ReadPin(KEY_IN_GPIO_Port, KEY_IN_Pin)==GPIO_PIN_RESET)
					  {
						  HAL_GPIO_WritePin(LED_LORA_GPIO_Port, LED_LORA_Pin, RESET);
						  HAL_GPIO_WritePin(DISPLAY_BKLT_GPIO_Port, DISPLAY_BKLT_Pin, RESET);
						  erase_push_counter = erase_push_counter+20;
						  PushPressTime++;
						  HAL_Delay(200);
						  HAL_GPIO_WritePin(LED_LORA_GPIO_Port, LED_LORA_Pin, SET);
						  HAL_GPIO_WritePin(DISPLAY_BKLT_GPIO_Port, DISPLAY_BKLT_Pin, SET);
					  }
					  erase_push_counter++;
					  HAL_Delay(10);
				  }
				  if(PushPressTime == 2)
				  {
					  PushPressTime = 0;
					  HAL_GPIO_WritePin(LED_LORA_GPIO_Port, LED_LORA_Pin, RESET);
					  HAL_GPIO_WritePin(DISPLAY_BKLT_GPIO_Port, DISPLAY_BKLT_Pin, RESET);
					  HAL_Delay(500);
					  ExtFlash_Read_RuntimePara(1);

					  HAL_GPIO_WritePin(LED_LORA_GPIO_Port, LED_LORA_Pin, SET);
					  HAL_GPIO_WritePin(DISPLAY_BKLT_GPIO_Port, DISPLAY_BKLT_Pin, SET);
					  HAL_Delay(500);
					  ExtFlash_Read_EPROM_General(1);
					  ExtFlash_Read_EPROM_AI_Calibration(1);

					  HAL_GPIO_WritePin(LED_LORA_GPIO_Port, LED_LORA_Pin, RESET);
					  HAL_GPIO_WritePin(DISPLAY_BKLT_GPIO_Port, DISPLAY_BKLT_Pin, RESET);
					  HAL_Delay(500);
					  ExtFlash_Read_EPROM_Schedule(1);

					  HAL_GPIO_WritePin(LED_LORA_GPIO_Port, LED_LORA_Pin, SET);
					  HAL_GPIO_WritePin(DISPLAY_BKLT_GPIO_Port, DISPLAY_BKLT_Pin, SET);
					  HAL_Delay(500);
					  ExtFlash_Read_EPROM_Modbus_Quary_Detail(1);

					  HAL_GPIO_WritePin(LED_LORA_GPIO_Port, LED_LORA_Pin, RESET);
					  HAL_GPIO_WritePin(DISPLAY_BKLT_GPIO_Port, DISPLAY_BKLT_Pin, RESET);
					  HAL_Delay(500);
					  ExtFlash_Read_EPROM_PCBPLC_GENERAL_REG(1);
					  ExtFlash_Read_EPROM_Frequent(1);
					  HAL_GPIO_WritePin(LED_LORA_GPIO_Port, LED_LORA_Pin, SET);
					  HAL_GPIO_WritePin(DISPLAY_BKLT_GPIO_Port, DISPLAY_BKLT_Pin, SET);
					  HAL_Delay(500);
					  ExtFlash_Read_gPlcRecFlash(1);
					  ExtFlash_Read_EPROM_LORA(1);

					  HAL_GPIO_WritePin(LED_LORA_GPIO_Port, LED_LORA_Pin, RESET);
					  HAL_GPIO_WritePin(DISPLAY_BKLT_GPIO_Port, DISPLAY_BKLT_Pin, RESET);
					  HAL_Delay(500);
					  W25Q_EraseSector(EPROM_REC_MODIFIED_START_ADDRESS);

					  HAL_GPIO_WritePin(LED_LORA_GPIO_Port, LED_LORA_Pin, SET);
					  HAL_GPIO_WritePin(DISPLAY_BKLT_GPIO_Port, DISPLAY_BKLT_Pin, SET);
					  HAL_Delay(500);
					  HAL_GPIO_WritePin(LED_LORA_GPIO_Port, LED_LORA_Pin, RESET);
					  HAL_GPIO_WritePin(DISPLAY_BKLT_GPIO_Port, DISPLAY_BKLT_Pin, RESET);
					  syncExtFlashVariableWithPCBPLCVariable();
				  }
			  }
		  }
		  HAL_GPIO_WritePin(LED_LORA_GPIO_Port, LED_LORA_Pin, RESET);
		  HAL_GPIO_WritePin(DISPLAY_BKLT_GPIO_Port, DISPLAY_BKLT_Pin, RESET);
	  }
  }



////  MX_UART5_Init();
  MX_UART8_Init();
////  MX_USART1_UART_Init();
  MX_USART2_UART_Init();

  checkProductionMode();
//  if(Pro_Application_flag)
//  {
//	  strcpy((char *)EPROM_LoRa_Modem.lora_app_eui_set,PRODUCTION_LORA_APPEID);
//	  strcpy((char *)EPROM_LoRa_Modem.lora_app_key_set,PRODUCTION_LORA_APPKEY);
//	 // ExtFlash_update_EPROM_General();
//	  ExtFlash_update_EPROM_LORA();
//  }
//  else
//  {
//	  strcpy((char *)EPROM_LoRa_Modem.lora_app_eui_set,DEFAULT_LORA_APPEID);
//	  strcpy((char *)EPROM_LoRa_Modem.lora_app_key_set,DEFAULT_LORA_APPKEY);
//	  ExtFlash_update_EPROM_General();
//  }

  if(Pro_Application_flag)
  {
	  EPROM_General.pro_CheckByte = 0xAA;
	  ExtFlash_update_EPROM_General();
	  EPROM_General.pro_CheckByte = 0;
	  ExtFlash_Read_EPROM_General(0);
	  if(EPROM_General.pro_CheckByte == 0xAA)
	  {
		  pro_Flash_State = 1;
	  }
	  else
	  {
		  pro_Flash_State = 0;
	  }
	  strcpy((char *)EPROM_LoRa_Modem.lora_app_eui_set,PRODUCTION_LORA_APPEID);
	  strcpy((char *)EPROM_LoRa_Modem.lora_app_key_set,PRODUCTION_LORA_APPKEY);
	  ExtFlash_update_EPROM_LORA();
  }

  lcd_initialize();
  HAL_Delay(10);
  lcd_set_cursor(1,0);
  lcd_display_string("CIMCON SOFTWARE");
  lcd_set_cursor(2,0);
  lcd_display_string("    LRTU3000    ");

  BuildModbusMasterQueryTelegrams();

  sprintf((char *)print, "\r\nDevice IP : %d.%d.%d.%d\r\n", EPROM_General.E_Comm.E_IP_Add[0],EPROM_General.E_Comm.E_IP_Add[1], EPROM_General.E_Comm.E_IP_Add[2],EPROM_General.E_Comm.E_IP_Add[3]);
  WriteLog(1, (char *)print, 1);
  sprintf((char *)print, "\r\nPROD IP : %d.%d.%d.%d\r\n", PROD_Ethernet_IP[0],PROD_Ethernet_IP[1], PROD_Ethernet_IP[2],PROD_Ethernet_IP[3]);
   WriteLog(1, (char *)print, 1);


//  sprintf((char *)print, "BLE MAC Add : %x:", EPROM_General.bleDetails.BLE_MAC_Add[0]);
//  WriteLog(1, (char *)print, 1);
//  sprintf((char *)print, "%x:%x:%x:%x:%x\r\n", EPROM_General.bleDetails.BLE_MAC_Add[1],EPROM_General.bleDetails.BLE_MAC_Add[2],EPROM_General.bleDetails.BLE_MAC_Add[3],
//  		EPROM_General.bleDetails.BLE_MAC_Add[4],EPROM_General.bleDetails.BLE_MAC_Add[5]);
//  WriteLog(1, (char *)print, 1);

//  gFinalAnaValF[GPS_NO_OF_SATALITE_gFinalAnaValF]= EPROM_General.rebootCount; // TODO : Remove this it

  //****** Get Astro Time ******//
  Get_Astro_time();
  //*********************//
  //W25Q_ReadRaw((u8_t*) &gPlcRecFlash, sizeof(gPlcRecFlash), PCB_REC_INFO_FILE_START_ADDRESS);
  /* USER CODE END RTOS_QUEUES */
 // Print_Flash();
  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityAboveNormal, 0, 512*8);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  EC200U_start();
  RxRingProcess_start();
  if(gFinalAnaValF[COMM_MODE_ETHER_GPRS_gFinalAnaValF] == 0)
  {
	  Modem_MQTT_start();
  }
//  //Modem_GPS_start();
//  Modem_BLE_start();

//  if(Pro_Application_flag == 0)
//  {
//	  pcbplc_start();
//  }
  Lora_start();
  DIDO_start();
  //ADC_start();
  ///////////////////////////////////////////AN////////////////////////////
//   HAL_GPIO_WritePin(DO1_GPIO_Port, DO1_Pin, RESET);
// 	  	  HAL_GPIO_WritePin(DO7_GPIO_Port, DO7_Pin, SET);
// 	  		  HAL_GPIO_WritePin(DO8_GPIO_Port, DO8_Pin, RESET);
// 	  	  osDelay(2000);
////////////////////////////////////////////////////AN//////////////////////////

//  COM_PORT_RS485_1_start();
//  COM_PORT_RS485_2_start();
  COM_PORT_RS232_1_start();
//  COM_PORT_RS232_2_start();


  /* USER CODE END RTOS_THREADS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 35;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

///**
//  * @brief ADC1 Initialization Function
//  * @param None
//  * @retval None
//  */
//static void MX_ADC1_Init(void)
//{
//
//  /* USER CODE BEGIN ADC1_Init 0 */
//
//  /* USER CODE END ADC1_Init 0 */
//
//  ADC_MultiModeTypeDef multimode = {0};
//  ADC_ChannelConfTypeDef sConfig = {0};
//
//  /* USER CODE BEGIN ADC1_Init 1 */
//
//  /* USER CODE END ADC1_Init 1 */
//
//  /** Common config
//  */
//  hadc1.Instance = ADC1;
//  hadc1.Init.Resolution = ADC_RESOLUTION_16B;
//  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
//  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
//  hadc1.Init.LowPowerAutoWait = DISABLE;
//  hadc1.Init.ContinuousConvMode = DISABLE;
//  hadc1.Init.NbrOfConversion = 1;
//  hadc1.Init.DiscontinuousConvMode = ENABLE;
//  hadc1.Init.NbrOfDiscConversion = 1;
//  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
//  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
//  hadc1.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DR;
//  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
//  hadc1.Init.LeftBitShift = ADC_LEFTBITSHIFT_NONE;
//  hadc1.Init.OversamplingMode = ENABLE;
//  hadc1.Init.Oversampling.Ratio = 256;
//  hadc1.Init.Oversampling.RightBitShift = ADC_RIGHTBITSHIFT_8;
//  hadc1.Init.Oversampling.TriggeredMode = ADC_TRIGGEREDMODE_SINGLE_TRIGGER;
//  hadc1.Init.Oversampling.OversamplingStopReset = ADC_REGOVERSAMPLING_CONTINUED_MODE;
//  if (HAL_ADC_Init(&hadc1) != HAL_OK)
//  {
//    Error_Handler();
//  }
//
//  /** Configure the ADC multi-mode
//  */
//  multimode.Mode = ADC_MODE_INDEPENDENT;
//  if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK)
//  {
//    Error_Handler();
//  }
//
//  /** Configure Regular Channel
//  */
//  sConfig.Channel = ADC_CHANNEL_15;
//  sConfig.Rank = ADC_REGULAR_RANK_1;
//  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
//  sConfig.SingleDiff = ADC_SINGLE_ENDED;
//  sConfig.OffsetNumber = ADC_OFFSET_NONE;
//  sConfig.Offset = 0;
//  sConfig.OffsetSignedSaturation = DISABLE;
//  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
//  {
//    Error_Handler();
//  }
//  /* USER CODE BEGIN ADC1_Init 2 */
//
//  /* USER CODE END ADC1_Init 2 */
//
//}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x00C0EAFF;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief CRC Initialization Function
  * @param None
  * @retval None
  */
static void MX_CRC_Init(void)
{

  /* USER CODE BEGIN CRC_Init 0 */

  /* USER CODE END CRC_Init 0 */

  /* USER CODE BEGIN CRC_Init 1 */

  /* USER CODE END CRC_Init 1 */
  hcrc.Instance = CRC;
  hcrc.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_ENABLE;
  hcrc.Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_ENABLE;
  hcrc.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_NONE;
  hcrc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE;
  hcrc.InputDataFormat = CRC_INPUTDATA_FORMAT_BYTES;
  if (HAL_CRC_Init(&hcrc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CRC_Init 2 */

  /* USER CODE END CRC_Init 2 */

}
/**
  * @brief QUADSPI Initialization Function
  * @param None
  * @retval None
  */
static void MX_QUADSPI_Init(void)
{

  /* USER CODE BEGIN QUADSPI_Init 0 */

  /* USER CODE END QUADSPI_Init 0 */

  /* USER CODE BEGIN QUADSPI_Init 1 */

  /* USER CODE END QUADSPI_Init 1 */
  /* QUADSPI parameter configuration*/
  hqspi.Instance = QUADSPI;
  hqspi.Init.ClockPrescaler = 14;
  hqspi.Init.FifoThreshold = 1;
  hqspi.Init.SampleShifting = QSPI_SAMPLE_SHIFTING_HALFCYCLE;
  hqspi.Init.FlashSize = 25;
  hqspi.Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_2_CYCLE;
  hqspi.Init.ClockMode = QSPI_CLOCK_MODE_3;
  hqspi.Init.FlashID = QSPI_FLASH_ID_1;
  hqspi.Init.DualFlash = QSPI_DUALFLASH_DISABLE;
  if (HAL_QSPI_Init(&hqspi) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN QUADSPI_Init 2 */

  /* USER CODE END QUADSPI_Init 2 */

}
/**
  * @brief IWDG1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_IWDG1_Init(void)
{

  /* USER CODE BEGIN IWDG1_Init 0 */

  /* USER CODE END IWDG1_Init 0 */

  /* USER CODE BEGIN IWDG1_Init 1 */

  /* USER CODE END IWDG1_Init 1 */
  hiwdg1.Instance = IWDG1;
  hiwdg1.Init.Prescaler = IWDG_PRESCALER_256;
  hiwdg1.Init.Window = 4095;
  hiwdg1.Init.Reload = 4095;
  if (HAL_IWDG_Init(&hiwdg1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN IWDG1_Init 2 */

  /* USER CODE END IWDG1_Init 2 */

}



/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, DO1_Pin|DO2_Pin|DO3_Pin|DO4_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, DO5_Pin|DO6_Pin|USB_RST_Pin|DO16_Pin
                          |DO17_Pin|DO18_Pin/*|DO24_Pin|DO25_Pin*/, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, DO9_Pin|DO10_Pin|DO_EN_Pin|DISPLAY_BKLT_Pin
                          |LED_LORA_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, DO11_Pin|DO12_Pin|DO13_Pin|DO14_Pin
                          |DO15_Pin|DO26_Pin|LORA_RST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, DO19_Pin|DO20_Pin|DO21_Pin|DO22_Pin
                          |DO23_Pin, GPIO_PIN_RESET);

  HAL_GPIO_WritePin(GPIOB, DO_EN_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOC, USB_RST_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOD, LORA_RST_Pin, GPIO_PIN_SET);

  /*Configure GPIO pins : DO1_Pin DO2_Pin DO3_Pin DO4_Pin */
  GPIO_InitStruct.Pin = DO1_Pin|DO2_Pin|DO3_Pin|DO4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : DO5_Pin DO6_Pin USB_RST_Pin DO16_Pin
                           DO17_Pin DO18_Pin DO24_Pin DO25_Pin */
  GPIO_InitStruct.Pin = Watchdog_Pin|DO5_Pin|DO6_Pin|USB_RST_Pin|DO16_Pin
                          |DO17_Pin|DO18_Pin/*|DO24_Pin|DO25_Pin*/;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : DI1_Pulse_Pin */
  GPIO_InitStruct.Pin = DI1_Pulse_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(DI1_Pulse_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : DI2_Pulse_Pin */
  GPIO_InitStruct.Pin = DI2_Pulse_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(DI2_Pulse_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : DI3_Pin DI4_Pin DI5_Pin DI6_Pin
                           DI7_Pin DI8_Pin DO7_Pin DO8_Pin */
  GPIO_InitStruct.Pin = DI3_Pin|DI4_Pin|DI5_Pin|DI6_Pin
                          |DI7_Pin|DI8_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins :  DO7_Pin DO8_Pin */
  GPIO_InitStruct.Pin = DO7_Pin|DO8_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : DO9_Pin DO10_Pin DO_EN_Pin DISPLAY_BKLT_Pin
                           LED_LORA_Pin */
  GPIO_InitStruct.Pin = DO9_Pin|DO10_Pin|DO_EN_Pin|DISPLAY_BKLT_Pin
                          |LED_LORA_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : DO11_Pin DO12_Pin DO13_Pin DO14_Pin
                           DO15_Pin DO26_Pin LORA_RST_Pin */
  GPIO_InitStruct.Pin = DO11_Pin|DO12_Pin|DO13_Pin|DO14_Pin
                          |DO15_Pin|DO26_Pin|LORA_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : DO19_Pin DO20_Pin DO21_Pin DO22_Pin
                           DO23_Pin */
  GPIO_InitStruct.Pin = DO19_Pin|DO20_Pin|DO21_Pin|DO22_Pin
                          |DO23_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : KEY_IN_Pin */
  GPIO_InitStruct.Pin = KEY_IN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(KEY_IN_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

}

/* USER CODE BEGIN 4 */




/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* init code for LWIP */
	sendExternalFlashSemaphore= xSemaphoreCreateBinary();
	xSemaphoreGive(sendExternalFlashSemaphore);

	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7,GPIO_PIN_RESET);

	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_3,GPIO_PIN_SET);
	osDelay(100);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_3,GPIO_PIN_RESET);
	osDelay(100);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_3,GPIO_PIN_SET);

  MX_LWIP_Init();

  LOCK_TCPIP_CORE();

//  udpserver_init();

  udpserver_1_init();
  //tcpserver_1_init();
  tcpserver_init();
//  ModbusTCP_start();

//  if(gFinalAnaValF[COMM_MODE_ETHER_GPRS_gFinalAnaValF] == 1)
//  {
//	  Ethernet_MQTT_start();
//  }

  UNLOCK_TCPIP_CORE();

  /* USER CODE BEGIN 5 */
    /* ETH_CODE: Adding lwiperf to measure TCP/IP performance.
   * iperf 2.0.6 (or older?) is required for the tests. Newer iperf2 versions
   * might work without data check, but they send different headers.
   * iperf3 is not compatible at all.
   * Adding lwiperf.c file to the project is necessary.
   * The default include path should already contain
   * 'lwip/apps/lwiperf.h'
   */
  LOCK_TCPIP_CORE();
  lwiperf_start_tcp_server_default(NULL, NULL);

//  ip4_addr_t remote_addr;
//  IP4_ADDR(&remote_addr, 192, 168, 1, 1);
//  lwiperf_start_tcp_client_default(&remote_addr, NULL, NULL);
  UNLOCK_TCPIP_CORE();


//  const char* message = "Hello UDP message!\n\r";

//  osDelay(1000);

// ip_addr_t PC_IPADDR;
//  IP_ADDR4(&PC_IPADDR, 199, 199, 50, 123);

//  struct udp_pcb* my_udp = udp_new();
//  udp_connect(my_udp, &PC_IPADDR, 55151);
//  struct pbuf* udp_buffer = NULL;

  /* Infinite loop */
  for (;;)
  {
	  //Print_Memory_RTOS_Stack();
	  if(Pro_Application_flag == 0)
	  {
		  service_start();
		  service_stop();
	  }
//	  if(Pro_Application_flag)
//	  {
//
//	  }
//	  else
//	  {
//		  if(count_ModbusTCP++ >= 60)
//		  {
//				sprintf((char *)print,"count_ModbusTCP:%d limit cross so do Soft Reboot\r\n",count_ModbusTCP);
//				WriteLog(1, (const char *)print, 1);
//				reboot_device_func();
//				osDelay(1000);
//				//HAL_NVIC_SystemReset();
//		  }
//		  if(count_EC200U++ >= 60)
//		  {
//				sprintf((char *)print,"count_EC200U:%d limit cross so do Soft Reboot\r\n",count_EC200U);
//				WriteLog(1, (const char *)print, 1);
//				reboot_device_func();
//				osDelay(1000);
//				//HAL_NVIC_SystemReset();
//		  }
//		  if(count_EC200U_RxRingProcess++ >= 60)
//		  {
//				sprintf((char *)print,"count_EC200U_RxRingProcess:%d limit cross so do Soft Reboot\r\n",count_EC200U_RxRingProcess);
//				WriteLog(1, (const char *)print, 1);
//				reboot_device_func();
//				osDelay(1000);;
//				//HAL_NVIC_SystemReset();
//		  }
//		  if(gFinalAnaValF[COMM_MODE_ETHER_GPRS_gFinalAnaValF] == 0)
//		  {
////			  if(count_Modem_MQTT++ >= 60)
////			  {
////					sprintf((char *)print,"count_Modem_MQTT:%d\r\n limit cross so do Soft Reboot",count_Modem_MQTT);
////					WriteLog(1, (const char *)print, 1);
////					reboot_device_func();
////					osDelay(1000);
////					//HAL_NVIC_SystemReset();
////			  }
////			  if(Modem_gsm_network_GACT_Status == 1)
////			  {
////				  if(modem_MQTT_publish_success_timer++ > 720) //  every min 2 time (60minx2)x6(hour)=720
////				  {
////					  sprintf((char *)print,"modem_MQTT_publish_success_timer:%d limit cross so modem Reboot\r\n",modem_MQTT_publish_success_timer);
////					  WriteLog(1, (const char *)print, 1);
////					  flag_modem_reboot = 1;
////					  modem_MQTT_publish_success_timer = 0;
////				  }
////			  }
//		  }
//		  else
//		  {
//			  if(count_Ethernet_MQTT++ >= 60)
//			  {
//					sprintf((char *)print,"count_Ethernet_MQTT:%d\r\n limit cross so do Soft Reboot",count_Ethernet_MQTT);
//					WriteLog(1, (const char *)print, 1);
//					reboot_device_func();
//					osDelay(1000);
//					//HAL_NVIC_SystemReset();
//			  }
//			  if(ethernet_MQTT_publish_success_timer++ > 720) //  every min 2 time (60minx2)x6(hour)=720
//			  {
//				  //sprintf((char *)print,"ethernet_MQTT_publish_success_timer:%d limit cross so modem Reboot\r\n",ethernet_MQTT_publish_success_timer);
//				 // WriteLog(1, (const char *)print, 1);
//				  flag_ethernet_reboot = 1;
//				  ethernet_MQTT_publish_success_timer = 0;
//				  //reboot_device_func();
//			  }
//		  }
////		  if(count_Modem_BLE++ >= 60)
////		  {
////				sprintf((char *)print,"count_Modem_BLE:%d limit cross so do Soft Reboot\r\n",count_Modem_BLE);
////				WriteLog(1, (const char *)print, 1);
////				reboot_device_func();
////				osDelay(1000);
////				//HAL_NVIC_SystemReset();
////		  }
//		  if(count_DO++ >= 60)
//		  {
//				sprintf((char *)print,"count_DO:%d limit cross so do Soft Reboot\r\n",count_DO);
//				WriteLog(1, (const char *)print, 1);
//				reboot_device_func();
//				osDelay(1000);
//				//HAL_NVIC_SystemReset();
//		  }
//		  if(count_ADC++ >= 60)
//		  {
//				sprintf((char *)print,"count_ADC:%ld limit cross so do Soft Reboot\r\n",count_ADC);
//				WriteLog(1, (const char *)print, 1);
//				reboot_device_func();
//				osDelay(1000);
//				//HAL_NVIC_SystemReset();
//		  }
////		  if(count_RS485_1++ >= 60)
////		  {
////				sprintf((char *)print,"count_RS485_1:%d limit cross so do Soft Reboot\r\n",count_RS485_1);
////				WriteLog(1, (const char *)print, 1);
////				reboot_device_func();
////				osDelay(1000);
////				//HAL_NVIC_SystemReset();
////		  }
////		  if(count_RS485_2++ >= 60)
////		  {
////				sprintf((char *)print,"count_RS485_2:%d limit cross so do Soft Reboot\r\n",count_RS485_1);
////				WriteLog(1, (const char *)print, 1);
////				reboot_device_func();
////				osDelay(1000);
////				//HAL_NVIC_SystemReset();
////		  }
////		  if(count_RS232_1++ >= 60)
////		  {
////				sprintf((char *)print,"count_RS322_1:%d limit cross so do Soft Reboot\r\n",count_RS485_1);
////				WriteLog(1, (const char *)print, 1);
////				reboot_device_func();
////				osDelay(1000);
////				//HAL_NVIC_SystemReset();
////		  }
////		  if(count_RS232_2++ >= 60)
////		  {
////				sprintf((char *)print,"count_RS232_2:%d limit cross so do Soft Reboot\r\n",count_RS485_1);
////				WriteLog(1, (const char *)print, 1);
////				reboot_device_func();
////				osDelay(1000);
////				//HAL_NVIC_SystemReset();
////		  }
//
//		  if(flag_ethernet_reboot == 1)
//		  {
//			  if((((gFinalAnaValF[SUNRISE_HOUR_gFinalAnaValF]*60) + gFinalAnaValF[SUNRISE_MIN_gFinalAnaValF]) < (gTime.Hours*60 + gTime.Minutes)) &&
//					  (((gFinalAnaValF[SUNSET_HOUR_gFinalAnaValF]*60) + gFinalAnaValF[SUNSET_HOUR_gFinalAnaValF]) > (gTime.Hours*60 + gTime.Minutes)))
//			  {
//				  flag_day_night_reboot = 1;
//			  }
//			  else
//			  {
//				  flag_day_night_reboot = 2;
//			  }
//
//			  if(flag_ethernet_reboot == 1)
//			  {
//				  if(EPROM_General.Cust_Detail.reboot_day_night == flag_day_night_reboot)
//				  {
//					  flag_ethernet_reboot = 0;
//					  sprintf((char *)print,"Ethernet MQTT not connect day/night:%d limit cross so device Reboot\r\n",flag_day_night_reboot);
//					  WriteLog(1, (const char *)print, 1);
//					  reboot_device_func();
//				  }
//				  else if(EPROM_General.Cust_Detail.reboot_day_night == 0)
//				  {
//					  flag_ethernet_reboot = 0;
//					  sprintf((char *)print,"Ethernet MQTT not connect day/night:%d limit cross so device Reboot\r\n",flag_day_night_reboot);
//					  WriteLog(1, (const char *)print, 1);
//					  reboot_device_func();
//				  }
//			  }
//		  }
//	  }
	  osDelay(30000);
//    /* !! PBUF_RAM is critical for correct operation !! */
//    udp_buffer = pbuf_alloc(PBUF_TRANSPORT, strlen(message), PBUF_RAM);
//
//    if (udp_buffer != NULL) {
//      memcpy(udp_buffer->payload, message, strlen(message));
//      udp_send(my_udp, udp_buffer);
//      pbuf_free(udp_buffer);
//    }
  }
  /* USER CODE END 5 */


}
void reboot_device_func(void)
{
	flag_flashSaveRecipe = 1;
	flag_flashUpdateEPROM_General = 1;
	flag_flashUpdateEPROM_AI_Calibration = 1;
	flag_flashUpdateEPROM_Schedule = 1;
	flag_flashUpdateEPROM_Modbus_Quary_Detail = 1;

	flag_flashSaveRecipe_WaitCounter=1;
	flag_flashUpdateEPROM_General_WaitCounter=1;
	flag_flashUpdateEPROM_AI_Calibration_WaitCounter=1;
	flag_flashUpdateEPROM_Schedule_WaitCounter=1;
	flag_flashUpdateEPROM_Modbus_Quary_Detail_WaitCounter=1;
	flag_flashUpdateEPROM_PCBPLC_GENERAL_REG_WaitCounter=299;
	osDelay(6000);
	HAL_NVIC_SystemReset();
}

/* MPU Configuration */

void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x0;
  MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
  MPU_InitStruct.SubRegionDisable = 0x87;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Number = MPU_REGION_NUMBER1;
  MPU_InitStruct.BaseAddress = 0x30020000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_128KB;
  MPU_InitStruct.SubRegionDisable = 0x0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL1;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  MPU_InitStruct.Number = MPU_REGION_NUMBER2;
  MPU_InitStruct.BaseAddress = 0x30040000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_1KB;//MPU_REGION_SIZE_512B;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM17 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM17) {
    HAL_IncTick();
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
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
	//  err=1;
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

void Print_Flash(void)
{
#if 0
	char tDebug[256] = {0, };
	int i;

	//******* External flash *******//
#ifdef WATCH_DOG_ENABLE //MAU_EXCEPTION
	HAL_IWDG_Refresh(&hiwdg1);
#endif
    sprintf(tDebug, "\r\n\r\n");
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "//******************************************START************************************//\r\n");
    WriteLog(1, tDebug, 1);

	sprintf((char*)tDebug, "Date:%02d-%02d-%02d\r\n",gDate.Date, gDate.Month, 2000 + gDate.Year);
	WriteLog(1, tDebug, 1);
	sprintf((char*)tDebug, "Time:%02d:%02d:%02d\r\n",gTime.Hours, gTime.Minutes, gTime.Seconds);
	WriteLog(1, tDebug, 1);

    sprintf(tDebug, "g_flashRunTimeParaSturct.s_ExtDataFlash_CheckByte=%d\r\n", g_flashRunTimeParaSturct.s_ExtDataFlash_CheckByte);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "g_flashRunTimeParaSturct.s_ExtDataFlash_IsDataLogOverwritten=%d\r\n", g_flashRunTimeParaSturct.s_ExtDataFlash_IsDataLogOverwritten);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "g_flashRunTimeParaSturct.s_ExtDataFlash_PageCounter=%d\r\n", g_flashRunTimeParaSturct.s_ExtDataFlash_PageCounter);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "g_flashRunTimeParaSturct.s_temp_Counter=%d\r\n", g_flashRunTimeParaSturct.s_temp_Counter);
    WriteLog(1, tDebug, 1);
//    sprintf(tDebug, "g_flashRunTimeParaSturct.unused=%s\r\n", g_flashRunTimeParaSturct.unused);
//    WriteLog(1, tDebug, 1);

    sprintf(tDebug, "\r\n\r\n");
    WriteLog(1, tDebug, 1);
	//******** General EPROM *********//
    sprintf(tDebug, "EPROM_General.checkbyte=%d\r\n", EPROM_General.checkbyte);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_General.ChecksumOfStuct=%d\r\n", EPROM_General.ChecksumOfStuct);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_General.SizeOfStuct=%d\r\n", EPROM_General.SizeOfStuct);
    WriteLog(1, tDebug, 1);
   // sprintf(tDebug, "EPROM_General.forFutureUse1=%s\r\n", EPROM_General.forFutureUse1);
  //  WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_General.rebootCount=%d\r\n", EPROM_General.rebootCount);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_General.LogRate=%d\r\n", EPROM_General.LogRate);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_General.maxLograteTimeSliceDelayS=%d\r\n", EPROM_General.maxLograteTimeSliceDelayS);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_General.History_En_Di=%d\r\n", EPROM_General.History_En_Di);
    WriteLog(1, tDebug, 1);

    sprintf(tDebug, "\r\n\r\n");
    WriteLog(1, tDebug, 1);

#ifdef WATCH_DOG_ENABLE //MAU_EXCEPTION
	HAL_IWDG_Refresh(&hiwdg1);
#endif

    sprintf(tDebug, "EPROM_General.Rtu_Detail.RTUId=%d\r\n", EPROM_General.Rtu_Detail.RTUId);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_General.Rtu_Detail.HW_Version=%s\r\n", EPROM_General.Rtu_Detail.HW_Version);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_General.Rtu_Detail.PLC_Version=%s\r\n", EPROM_General.Rtu_Detail.PLC_Version);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_General.Rtu_Detail.REC_Version=%s\r\n", EPROM_General.Rtu_Detail.REC_Version);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_General.Rtu_Detail.Hex_Version=%s\r\n", DEFAULT_FV_VERSION);//EPROM_General.Rtu_Detail.Hex_Version);
    WriteLog(1, tDebug, 1);
   // sprintf(tDebug, "EPROM_General.Rtu_Detail.forFutureUse=%s\r\n", EPROM_General.Rtu_Detail.forFutureUse);
   // WriteLog(1, tDebug, 1);

    sprintf(tDebug, "\r\n\r\n");
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_General.Cust_Detail.Proj_Code=%s\r\n", EPROM_General.Cust_Detail.Proj_Code);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_General.Cust_Detail.Site_Name=%s\r\n", EPROM_General.Cust_Detail.Site_Name);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_General.Cust_Detail.Time_zone=%s\r\n", EPROM_General.Cust_Detail.Time_zone);
    WriteLog(1, tDebug, 1);
   // sprintf(tDebug, "EPROM_General.Cust_Detail.forFutureUse=%s\r\n", EPROM_General.Cust_Detail.forFutureUse);
   // WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_General.Cust_Detail.Client_Id=%d\r\n", EPROM_General.Cust_Detail.Client_Id);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_General.Cust_Detail.Reader_Id=%d\r\n", EPROM_General.Cust_Detail.Reader_Id);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_General.Cust_Detail.Lattitude=%lf\r\n", EPROM_General.Cust_Detail.Lattitude);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_General.Cust_Detail.Longitude=%lf\r\n", EPROM_General.Cust_Detail.Longitude);
    WriteLog(1, tDebug, 1);

    sprintf(tDebug, "\r\n\r\n");
    WriteLog(1, tDebug, 1);

#ifdef WATCH_DOG_ENABLE //MAU_EXCEPTION
	HAL_IWDG_Refresh(&hiwdg1);
#endif
    //**************** AI_DI_DO ***************//
    sprintf(tDebug, "EPROM_General.AI_DI_DO_Detail.Total_Di=%d\r\n", EPROM_General.AI_DI_DO_Detail.Total_Di);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_General.AI_DI_DO_Detail.Total_Do=%d\r\n", EPROM_General.AI_DI_DO_Detail.Total_Do);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_General.AI_DI_DO_Detail.Total_Ai=%d\r\n", EPROM_General.AI_DI_DO_Detail.Total_Ai);
    WriteLog(1, tDebug, 1);

    sprintf(tDebug, "\r\n\r\n");
    WriteLog(1, tDebug, 1);
    for(i=0;i<MAX_AI_CHANNEL;i++)
    {
    	sprintf(tDebug, "EPROM_General.AI_DI_DO_Detail.AI_Detail[%d].Id=%d\r\n", i,EPROM_General.AI_DI_DO_Detail.AI_Detail[i].Id);
    	WriteLog(1, tDebug, 1);
    	sprintf(tDebug, "EPROM_General.AI_DI_DO_Detail.AI_Detail[%d].AI_ch_Type=%d\r\n", i,EPROM_General.AI_DI_DO_Detail.AI_Detail[i].AI_ch_Type);
    	WriteLog(1, tDebug, 1);
    	sprintf(tDebug, "EPROM_General.AI_DI_DO_Detail.AI_Detail[%d].scaleLo=%f\r\n", i,EPROM_General.AI_DI_DO_Detail.AI_Detail[i].scaleLo);
    	WriteLog(1, tDebug, 1);
    	sprintf(tDebug, "EPROM_General.AI_DI_DO_Detail.AI_Detail[%d].scaleHi=%f\r\n", i,EPROM_General.AI_DI_DO_Detail.AI_Detail[i].scaleHi);
    	WriteLog(1, tDebug, 1);
    	sprintf(tDebug, "EPROM_General.AI_DI_DO_Detail.AI_Detail[%d].calZ=%f\r\n", i,EPROM_General.AI_DI_DO_Detail.AI_Detail[i].calZ);
    	WriteLog(1, tDebug, 1);
    	sprintf(tDebug, "EPROM_General.AI_DI_DO_Detail.AI_Detail[%d].calS=%f\r\n", i,EPROM_General.AI_DI_DO_Detail.AI_Detail[i].calS);
    	WriteLog(1, tDebug, 1);
    	sprintf(tDebug, "EPROM_General.AI_DI_DO_Detail.AI_Detail[%d].mAiLowCal_mA=%d\r\n", i,EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiLowCal_mA);
    	WriteLog(1, tDebug, 1);
    	sprintf(tDebug, "EPROM_General.AI_DI_DO_Detail.AI_Detail[%d].mAiLowMidCal_mA=%d\r\n", i,EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiLowMidCal_mA);
    	WriteLog(1, tDebug, 1);
    	sprintf(tDebug, "EPROM_General.AI_DI_DO_Detail.AI_Detail[%d].mAiMidCal_mA=%d\r\n", i,EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiMidCal_mA);
    	WriteLog(1, tDebug, 1);
    	sprintf(tDebug, "EPROM_General.AI_DI_DO_Detail.AI_Detail[%d].mAiHighCal_mA=%d\r\n", i,EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiHighCal_mA);
    	WriteLog(1, tDebug, 1);
    	sprintf(tDebug, "EPROM_General.AI_DI_DO_Detail.AI_Detail[%d].mAiLowCal_V=%d\r\n", i,EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiLowCal_V);
    	WriteLog(1, tDebug, 1);
    	sprintf(tDebug, "EPROM_General.AI_DI_DO_Detail.AI_Detail[%d].mAiLowMidCal_V=%d\r\n", i,EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiLowMidCal_V);
    	WriteLog(1, tDebug, 1);
    	sprintf(tDebug, "EPROM_General.AI_DI_DO_Detail.AI_Detail[%d].mAiMidCal_V=%d\r\n", i,EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiMidCal_V);
    	WriteLog(1, tDebug, 1);
    	sprintf(tDebug, "EPROM_General.AI_DI_DO_Detail.AI_Detail[%d].mAiHighCal_V=%d\r\n", i,EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiHighCal_V);
    	WriteLog(1, tDebug, 1);
    	sprintf(tDebug, "EPROM_General.AI_DI_DO_Detail.AI_Detail[%d].mAiLowCal_mA_Point=%f\r\n", i,EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiLowCal_mA_Point);
    	WriteLog(1, tDebug, 1);
    	sprintf(tDebug, "EPROM_General.AI_DI_DO_Detail.AI_Detail[%d].mAiLowMidCal_mA_Point=%f\r\n", i,EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiLowMidCal_mA_Point);
    	WriteLog(1, tDebug, 1);
    	sprintf(tDebug, "EPROM_General.AI_DI_DO_Detail.AI_Detail[%d].mAiMidCal_mA_Point=%f\r\n", i,EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiMidCal_mA_Point);
    	WriteLog(1, tDebug, 1);
    	sprintf(tDebug, "EPROM_General.AI_DI_DO_Detail.AI_Detail[%d].mAiHighCal_mA_Point=%f\r\n", i,EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiHighCal_mA_Point);
    	WriteLog(1, tDebug, 1);
    	sprintf(tDebug, "EPROM_General.AI_DI_DO_Detail.AI_Detail[%d].mAiLowCal_V_Point=%f\r\n", i,EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiLowCal_V_Point);
    	WriteLog(1, tDebug, 1);
    	sprintf(tDebug, "EPROM_General.AI_DI_DO_Detail.AI_Detail[%d].mAiLowMidCal_V_Point=%f\r\n", i,EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiLowMidCal_V_Point);
    	WriteLog(1, tDebug, 1);
    	sprintf(tDebug, "EPROM_General.AI_DI_DO_Detail.AI_Detail[%d].mAiMidCal_V_Point=%f\r\n", i,EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiMidCal_V_Point);
    	WriteLog(1, tDebug, 1);
    	sprintf(tDebug, "EPROM_General.AI_DI_DO_Detail.AI_Detail[%d].mAiHighCal_V_Point=%f\r\n", i,EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiHighCal_V_Point);
    	WriteLog(1, tDebug, 1);
    }

    sprintf(tDebug, "\r\n\r\n");
    WriteLog(1, tDebug, 1);

#ifdef WATCH_DOG_ENABLE //MAU_EXCEPTION
	HAL_IWDG_Refresh(&hiwdg1);
#endif

    sprintf(tDebug, "EPROM_General.S_Comm.Rs232_1_Info.S_Co_En_Di=%d\r\n", EPROM_General.S_Comm.Rs232_1_Info.S_Co_En_Di);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_General.S_Comm.Rs232_1_Info.S_Protocol=%d\r\n", EPROM_General.S_Comm.Rs232_1_Info.S_Protocol);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_General.S_Comm.Rs232_1_Info.S_Ma_sl_Cu=%d\r\n", EPROM_General.S_Comm.Rs232_1_Info.S_Ma_sl_Cu);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_General.S_Comm.Rs232_1_Info.S_Baudrate=%d\r\n", EPROM_General.S_Comm.Rs232_1_Info.S_Baudrate);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_General.S_Comm.Rs232_1_Info.S_Port_Id=%d\r\n", EPROM_General.S_Comm.Rs232_1_Info.S_Port_Id);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_General.S_Comm.Rs232_1_Info.S_Poll_Freq=%d\r\n", EPROM_General.S_Comm.Rs232_1_Info.S_Poll_Freq);
    WriteLog(1, tDebug, 1);

    sprintf(tDebug, "EPROM_General.S_Comm.Rs232_2_Info.S_Co_En_Di=%d\r\n", EPROM_General.S_Comm.Rs232_2_Info.S_Co_En_Di);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_General.S_Comm.Rs232_2_Info.S_Protocol=%d\r\n", EPROM_General.S_Comm.Rs232_2_Info.S_Protocol);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_General.S_Comm.Rs232_2_Info.S_Ma_sl_Cu=%d\r\n", EPROM_General.S_Comm.Rs232_2_Info.S_Ma_sl_Cu);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_General.S_Comm.Rs232_2_Info.S_Baudrate=%d\r\n", EPROM_General.S_Comm.Rs232_2_Info.S_Baudrate);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_General.S_Comm.Rs232_2_Info.S_Port_Id=%d\r\n", EPROM_General.S_Comm.Rs232_2_Info.S_Port_Id);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_General.S_Comm.Rs232_2_Info.S_Poll_Freq=%d\r\n", EPROM_General.S_Comm.Rs232_2_Info.S_Poll_Freq);
    WriteLog(1, tDebug, 1);


    sprintf(tDebug, "EPROM_General.S_Comm.Rs485_1_Info.S_Co_En_Di=%d\r\n", EPROM_General.S_Comm.Rs485_1_Info.S_Co_En_Di);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_General.S_Comm.Rs485_1_Info.S_Protocol=%d\r\n", EPROM_General.S_Comm.Rs485_1_Info.S_Protocol);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_General.S_Comm.Rs485_1_Info.S_Ma_sl_Cu=%d\r\n", EPROM_General.S_Comm.Rs485_1_Info.S_Ma_sl_Cu);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_General.S_Comm.Rs485_1_Info.S_Baudrate=%d\r\n", EPROM_General.S_Comm.Rs485_1_Info.S_Baudrate);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_General.S_Comm.Rs485_1_Info.S_Port_Id=%d\r\n", EPROM_General.S_Comm.Rs485_1_Info.S_Port_Id);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_General.S_Comm.Rs485_1_Info.S_Poll_Freq=%d\r\n", EPROM_General.S_Comm.Rs485_1_Info.S_Poll_Freq);
    WriteLog(1, tDebug, 1);

    sprintf(tDebug, "EPROM_General.S_Comm.Rs485_2_Info.S_Co_En_Di=%d\r\n", EPROM_General.S_Comm.Rs485_2_Info.S_Co_En_Di);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_General.S_Comm.Rs485_2_Info.S_Protocol=%d\r\n", EPROM_General.S_Comm.Rs485_2_Info.S_Protocol);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_General.S_Comm.Rs485_2_Info.S_Ma_sl_Cu=%d\r\n", EPROM_General.S_Comm.Rs485_2_Info.S_Ma_sl_Cu);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_General.S_Comm.Rs485_2_Info.S_Baudrate=%d\r\n", EPROM_General.S_Comm.Rs485_2_Info.S_Baudrate);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_General.S_Comm.Rs485_2_Info.S_Port_Id=%d\r\n", EPROM_General.S_Comm.Rs485_2_Info.S_Port_Id);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_General.S_Comm.Rs485_2_Info.S_Poll_Freq=%d\r\n", EPROM_General.S_Comm.Rs485_2_Info.S_Poll_Freq);
    WriteLog(1, tDebug, 1);

    sprintf(tDebug, "\r\n\r\n");
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_General.E_Comm.E_Co_En_Di=%d\r\n", EPROM_General.E_Comm.E_Co_En_Di);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_General.E_Comm.E_Mode=%d\r\n", EPROM_General.E_Comm.E_Mode);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_General.E_Comm.E_Mod_TCP=%d\r\n", EPROM_General.E_Comm.E_Mod_TCP);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_General.E_Comm.E_Ser_cli=%d\r\n", EPROM_General.E_Comm.E_Ser_cli);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_General.E_Comm.E_IP_Add=%d.%d.%d.%d\r\n", EPROM_General.E_Comm.E_IP_Add[0],EPROM_General.E_Comm.E_IP_Add[1], EPROM_General.E_Comm.E_IP_Add[2],EPROM_General.E_Comm.E_IP_Add[3]);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_General.E_Comm.E_Subnet_Add=%d.%d.%d.%d\r\n", EPROM_General.E_Comm.E_Subnet_Add[0], EPROM_General.E_Comm.E_Subnet_Add[1], EPROM_General.E_Comm.E_Subnet_Add[2], EPROM_General.E_Comm.E_Subnet_Add[3]);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_General.E_Comm.E_Gateway_Add=%d.%d.%d.%d\r\n", EPROM_General.E_Comm.E_Gateway_Add[0],EPROM_General.E_Comm.E_Gateway_Add[1], EPROM_General.E_Comm.E_Gateway_Add[2],EPROM_General.E_Comm.E_Gateway_Add[3]);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_General.E_Comm.E_Preferred_DNS=%d.%d.%d.%d\r\n", EPROM_General.E_Comm.E_Preferred_DNS[0],EPROM_General.E_Comm.E_Preferred_DNS[1], EPROM_General.E_Comm.E_Preferred_DNS[2],EPROM_General.E_Comm.E_Preferred_DNS[3]);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_General.E_Comm.E_Alternate_DNS=%d.%d.%d.%d\r\n", EPROM_General.E_Comm.E_Alternate_DNS[0],EPROM_General.E_Comm.E_Alternate_DNS[1], EPROM_General.E_Comm.E_Alternate_DNS[2],EPROM_General.E_Comm.E_Alternate_DNS[3]);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_General.E_Comm.E_TCP_Port=%d\r\n", EPROM_General.E_Comm.E_TCP_Port);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_General.E_Comm.E_Poll_Freq=%d\r\n", EPROM_General.E_Comm.E_Poll_Freq);
    WriteLog(1, tDebug, 1);


    sprintf(tDebug, "\r\n\r\n");

#ifdef WATCH_DOG_ENABLE //MAU_EXCEPTION
	HAL_IWDG_Refresh(&hiwdg1);
#endif

    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_General.Mo_Comm.Mo_Co_En_Di=%d\r\n", EPROM_General.Mo_Comm.Mo_Co_En_Di);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_General.Mo_Comm.Mo_Com_Int=%d\r\n", EPROM_General.Mo_Comm.Mo_Com_Int);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_General.Mo_Comm.Mo_Proto=%d\r\n", EPROM_General.Mo_Comm.Mo_Proto);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Bro_IP=%s\r\n", EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Bro_IP);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Bro_Port=%d\r\n", EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Bro_Port);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Us_Name=%s\r\n", EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Us_Name);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Us_Pass=%s\r\n", EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Us_Pass);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Cli_Id=%s\r\n", EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Cli_Id);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_General.Mo_Comm.MQTT_Conn.MQTT_PUB_Topic=%s\r\n", EPROM_General.Mo_Comm.MQTT_Conn.MQTT_PUB_Topic);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Sub_Topic=%s\r\n", EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Sub_Topic);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_General.Mo_Comm.Mo_APN=%s\r\n", EPROM_General.Mo_Comm.Mo_APN);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_General.Mo_Comm.MQTT_LiveFreq=%d\r\n", EPROM_General.Mo_Comm.MQTT_LiveFreq);
    WriteLog(1, tDebug, 1);

    sprintf(tDebug, "\r\n\r\n");
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_General.bleDetails.BLE_Co_En_Di=%d\r\n", EPROM_General.bleDetails.BLE_Co_En_Di);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_General.bleDetails.BLE_MAC_Add=%x:", EPROM_General.bleDetails.BLE_MAC_Add[0]);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "%x:%x:%x:%x:%x\r\n", EPROM_General.bleDetails.BLE_MAC_Add[1],EPROM_General.bleDetails.BLE_MAC_Add[2],EPROM_General.bleDetails.BLE_MAC_Add[3],
    		EPROM_General.bleDetails.BLE_MAC_Add[4],EPROM_General.bleDetails.BLE_MAC_Add[5]);
    WriteLog(1, tDebug, 1);
//    sprintf(tDebug, "EPROM_General.bleDetails.forFutureUse=%s\r\n", EPROM_General.bleDetails.forFutureUse);
//    WriteLog(1, tDebug, 1);

    sprintf(tDebug, "EPROM_General.gpsDetails.GPS_Co_En_Di=%d\r\n", EPROM_General.gpsDetails.GPS_Co_En_Di);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_General.gpsDetails.GPS_Poll_Freq=%d\r\n", EPROM_General.gpsDetails.GPS_Poll_Freq);
    WriteLog(1, tDebug, 1);
//    sprintf(tDebug, "EPROM_General.gpsDetails.forFutureUse=%s\r\n", EPROM_General.gpsDetails.forFutureUse);
//    WriteLog(1, tDebug, 1);.

    sprintf(tDebug, "EPROM_General.sdCardDetails.SD_Card_Co_En_Di=%d\r\n", EPROM_General.sdCardDetails.SD_Card_Co_En_Di);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_General.sdCardDetails.SD_Card_Size=%d\r\n", EPROM_General.sdCardDetails.SD_Card_Size);
    WriteLog(1, tDebug, 1);
//    sprintf(tDebug, "EPROM_General.sdCardDetails.SD_Card_Size=%d\r\n", EPROM_General.sdCardDetails.SD_Card_Size);
//    WriteLog(1, tDebug, 1);

    sprintf(tDebug, "EPROM_General.DoModeDetails.Do_Mode=%d\r\n", EPROM_General.DoModeDetails.Do_Mode);
    WriteLog(1, tDebug, 1);

	for(i=0;i<35;i++)
	{
	    sprintf(tDebug, "EPROM_General.DoModeDetails.Do_Mode[%d]=%d\r\n", i, EPROM_General.DoModeDetails.DO_Value[i]);
	    WriteLog(1, tDebug, 1);
	}
//    sprintf(tDebug, "EPROM_General.DoModeDetails.forFutureUse=%d\r\n", EPROM_General.DoModeDetails.forFutureUse);
//    WriteLog(1, tDebug, 1);

    sprintf(tDebug, "\r\n\r\n");
    WriteLog(1, tDebug, 1);

#ifdef WATCH_DOG_ENABLE //MAU_EXCEPTION
	HAL_IWDG_Refresh(&hiwdg1);
#endif

#if 1
    //*************** Schedule ******************//
    sprintf(tDebug, "EPROM_Schedule.checkbyte=%d\r\n", EPROM_Schedule.checkbyte);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_Schedule.ChecksumOfStuct=%d\r\n", EPROM_Schedule.ChecksumOfStuct);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_Schedule.SizeOfStuct=%d\r\n", EPROM_Schedule.SizeOfStuct);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_Schedule.forFutureUse1=%s\r\n", EPROM_Schedule.forFutureUse1);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_Schedule.Total_No_Schedule=%d\r\n", EPROM_Schedule.Total_No_Schedule);
    WriteLog(1, tDebug, 1);

	for(i=0;i<EPROM_Schedule.Total_No_Schedule;i++)
	{
	    sprintf(tDebug, "EPROM_Schedule.Schedule[%d].Sch_Id =%d\r\n", i,EPROM_Schedule.Schedule[i].Sch_Id );
	    WriteLog(1, tDebug, 1);
	    sprintf(tDebug, "EPROM_Schedule.Schedule[%d].Sch_En_Di =%d\r\n", i,EPROM_Schedule.Schedule[i].Sch_En_Di );
	    WriteLog(1, tDebug, 1);
	    sprintf(tDebug, "EPROM_Schedule.Schedule[%d].Start_HH =%d\r\n", i,EPROM_Schedule.Schedule[i].Start_HH );
	    WriteLog(1, tDebug, 1);
	    sprintf(tDebug, "EPROM_Schedule.Schedule[%d].Start_Min =%d\r\n", i,EPROM_Schedule.Schedule[i].Start_Min );
	    WriteLog(1, tDebug, 1);
	    sprintf(tDebug, "EPROM_Schedule.Schedule[%d].Stop_HH =%d\r\n", i,EPROM_Schedule.Schedule[i].Stop_HH );
	    WriteLog(1, tDebug, 1);
	    sprintf(tDebug, "EPROM_Schedule.Schedule[%d].Stop_Min =%d\r\n", i,EPROM_Schedule.Schedule[i].Stop_Min );
	    WriteLog(1, tDebug, 1);
	    sprintf(tDebug, "EPROM_Schedule.Schedule[%d].forFutureUse =%s\r\n", i,EPROM_Schedule.Schedule[i].forFutureUse );
	    WriteLog(1, tDebug, 1);
	}
#endif

#ifdef WATCH_DOG_ENABLE //MAU_EXCEPTION
	HAL_IWDG_Refresh(&hiwdg1);
#endif

	//************Modbus Query ***************//

    sprintf(tDebug, "EPROM_Modbus_Quary_Detail.checkbyte=%d\r\n", EPROM_Modbus_Quary_Detail.checkbyte);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_Modbus_Quary_Detail.ChecksumOfStuct=%d\r\n", EPROM_Modbus_Quary_Detail.ChecksumOfStuct);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_Modbus_Quary_Detail.SizeOfStuct=%d\r\n", EPROM_Modbus_Quary_Detail.SizeOfStuct);
    WriteLog(1, tDebug, 1);
//    sprintf(tDebug, "EPROM_Modbus_Quary_Detail.forFutureUse1=%s\r\n", EPROM_Modbus_Quary_Detail.forFutureUse1);
//    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_Modbus_Quary_Detail.TotalQuery=%d\r\n", EPROM_Modbus_Quary_Detail.TotalQuery);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_Modbus_Quary_Detail.RetryCount=%d\r\n", EPROM_Modbus_Quary_Detail.RetryCount);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_Modbus_Quary_Detail.TotalPara=%d\r\n", EPROM_Modbus_Quary_Detail.TotalPara);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "EPROM_Modbus_Quary_Detail.mMaxDataTagEnabled=%d\r\n", EPROM_Modbus_Quary_Detail.mMaxDataTagEnabled);
    WriteLog(1, tDebug, 1);
#if 1
	for(i=0;i<EPROM_Modbus_Quary_Detail.TotalQuery;i++)
	{
	    sprintf(tDebug, "EPROM_Modbus_Quary_Detail.Mod_Quary[%d].Mod_Quary_ID=%d\r\n", i, EPROM_Modbus_Quary_Detail.Mod_Quary[i].Mod_Quary_ID);
	    WriteLog(1, tDebug, 1);
	    sprintf(tDebug, "EPROM_Modbus_Quary_Detail.Mod_Quary[%d].mFunctionCode=%d\r\n", i, EPROM_Modbus_Quary_Detail.Mod_Quary[i].mFunctionCode);
	    WriteLog(1, tDebug, 1);
	    sprintf(tDebug, "EPROM_Modbus_Quary_Detail.Mod_Quary[%d].mDataType=%d\r\n", i, EPROM_Modbus_Quary_Detail.Mod_Quary[i].mDataType);
	    WriteLog(1, tDebug, 1);
	    sprintf(tDebug, "EPROM_Modbus_Quary_Detail.Mod_Quary[%d].mPortSelection=%d\r\n", i, EPROM_Modbus_Quary_Detail.Mod_Quary[i].mPortSelection);
	    WriteLog(1, tDebug, 1);
	    sprintf(tDebug, "EPROM_Modbus_Quary_Detail.Mod_Quary[%d].mSlaveId=%d\r\n", i, EPROM_Modbus_Quary_Detail.Mod_Quary[i].mSlaveId);
	    WriteLog(1, tDebug, 1);
	    sprintf(tDebug, "EPROM_Modbus_Quary_Detail.Mod_Quary[%d].mRegStartAddr=%d\r\n", i, EPROM_Modbus_Quary_Detail.Mod_Quary[i].mRegStartAddr);
	    WriteLog(1, tDebug, 1);
	    sprintf(tDebug, "EPROM_Modbus_Quary_Detail.Mod_Quary[%d].mRegStartAddr=%d\r\n", i, EPROM_Modbus_Quary_Detail.Mod_Quary[i].mRegStartAddr);
	    WriteLog(1, tDebug, 1);
//	    sprintf(tDebug, "EPROM_Modbus_Quary_Detail.Mod_Quary[%d].forFutureUse=%d\r\n", i, EPROM_Modbus_Quary_Detail.Mod_Quary[i].forFutureUse);
//	    WriteLog(1, tDebug, 1);
	}
#endif
    sprintf(tDebug, "\r\n\r\n");
    WriteLog(1, tDebug, 1);

#ifdef WATCH_DOG_ENABLE //MAU_EXCEPTION
	HAL_IWDG_Refresh(&hiwdg1);
#endif

	//********* PLC REC Flash *****//
    sprintf(tDebug, "gPlcRecFlash.checkbyte=%d\r\n", gPlcRecFlash.checkbyte);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "gPlcRecFlash.extract_receipe=%d\r\n", gPlcRecFlash.extract_receipe);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "gPlcRecFlash.mPlcFileLength=%ld\r\n", gPlcRecFlash.mPlcFileLength);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "gPlcRecFlash.mRecFileLength=%ld\r\n", gPlcRecFlash.mRecFileLength);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "gPlcRecFlash.mPlcFileCRC=%ld\r\n", gPlcRecFlash.mPlcFileCRC);
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "gPlcRecFlash.mRecFileCRC=%ld\r\n", gPlcRecFlash.mRecFileCRC);
    WriteLog(1, tDebug, 1);

    sprintf(tDebug, "//******************************************END************************************//");
    WriteLog(1, tDebug, 1);
    sprintf(tDebug, "\r\n\r\n");
    WriteLog(1, tDebug, 1);

#ifdef WATCH_DOG_ENABLE //MAU_EXCEPTION
	HAL_IWDG_Refresh(&hiwdg1);
#endif

#if 0
	for(i=0;i<MAX_PLCVAR;i++)
	{
	    sprintf(tDebug, "gPlcRecFlash.plcVarArr[%d]=%s\r\n", i, gPlcRecFlash.plcVarArr[i]);
	    WriteLog(1, tDebug, 1);
	    sprintf(tDebug, "gPlcRecFlash.mPlcVarTypeArr[%d]=%s\r\n", i, gPlcRecFlash.mPlcVarTypeArr[i]);
	    WriteLog(1, tDebug, 1);
	}
#endif

#ifdef WATCH_DOG_ENABLE //MAU_EXCEPTION
	HAL_IWDG_Refresh(&hiwdg1);
#endif
#endif
}

void Print_Memory_RTOS_Stack(void)
{
#if 1
	unsigned long int uxHighWaterMark[20]={0,};

	uxHighWaterMark[0] = uxTaskGetStackHighWaterMark( defaultTaskHandle );
	uxHighWaterMark[1] = uxTaskGetStackHighWaterMark( EC200U_TaskHandle );
	uxHighWaterMark[2] = uxTaskGetStackHighWaterMark( EC200U_RxRingProcess_TaskHandle );
	uxHighWaterMark[3] = uxTaskGetStackHighWaterMark( Modem_MQTT_TaskHandle );
	uxHighWaterMark[4] = uxTaskGetStackHighWaterMark( Modem_BLE_TaskHandle );
	uxHighWaterMark[5] = uxTaskGetStackHighWaterMark( pcbplc_TaskHandle );
	uxHighWaterMark[6] = uxTaskGetStackHighWaterMark( COM_PORT_RS232_1_TaskHandle );
	uxHighWaterMark[7] = uxTaskGetStackHighWaterMark( COM_PORT_RS232_2_TaskHandle );
	uxHighWaterMark[8] = uxTaskGetStackHighWaterMark( DIDO_TaskHandle );
	uxHighWaterMark[9] = uxTaskGetStackHighWaterMark( ADC_TaskHandle );
	uxHighWaterMark[10] = uxTaskGetStackHighWaterMark( COM_PORT_RS485_1_TaskHandle );
	uxHighWaterMark[11] = uxTaskGetStackHighWaterMark( COM_PORT_RS485_2_TaskHandle );
	uxHighWaterMark[12] = xPortGetFreeHeapSize();

	sprintf((char*)print, "\r\n==================Start=============\r\nDate:%02d-%02d-%02d\r\n",gDate.Date, gDate.Month, 2000 + gDate.Year);
	WriteLog(1, print, 1);
	sprintf((char*)print, "Time:%02d:%02d:%02d\r\n\r\n",gTime.Hours, gTime.Minutes, gTime.Seconds);
	WriteLog(1, print, 1);

	sprintf((char *)print,"TCP:%ld\r\nEC200U:%ld\r\nEC200U_Ring:%ld\r\n",uxHighWaterMark[0],uxHighWaterMark[1],uxHighWaterMark[2]);
	WriteLog(1, print, 1);

	sprintf((char *)print,"MQTT:%ld\r\nBLE:%ld\r\nPCBPLC:%ld\r\n",uxHighWaterMark[3],uxHighWaterMark[4],uxHighWaterMark[5]);
	WriteLog(1, print, 1);

	sprintf((char *)print,"RS232_1:%ld\r\nRS232_2:%ld\r\nDIDO:%ld\r\n",uxHighWaterMark[6],uxHighWaterMark[7],uxHighWaterMark[8]);
	WriteLog(1, print, 1);

	sprintf((char *)print,"ADC:%ld\r\nRS485_1:%ld\r\nRS485_2:%ld\r\n",uxHighWaterMark[9],uxHighWaterMark[10],uxHighWaterMark[11]);
	WriteLog(1, print, 1);

	sprintf((char *)print,"HEAP:%ld\r\n\r\n===============END============\r\n",uxHighWaterMark[12]);
	WriteLog(1, print, 1);


	sprintf((char *)print,"\r\nCount:***************START*******************\r\n");
	WriteLog(1, print, 1);

	sprintf((char *)print,"count_ModbusTCP:%d\r\n",count_ModbusTCP);
	WriteLog(1, print, 1);

	sprintf((char *)print,"count_EC200U:%d\r\n",count_EC200U);
	WriteLog(1, print, 1);

	sprintf((char *)print,"count_EC200U_RxRingProcess:%d\r\n",count_EC200U_RxRingProcess);
	WriteLog(1, print, 1);

	sprintf((char *)print,"count_Modem_MQTT:%d\r\n",count_Modem_MQTT);
	WriteLog(1, print, 1);

	sprintf((char *)print,"count_Modem_BLE:%d\r\n",count_Modem_BLE);
	WriteLog(1, print, 1);

	sprintf((char *)print,"count_DO:%d\r\n",count_DO);
	WriteLog(1, print, 1);

	sprintf((char *)print,"count_ADC:%d\r\n",count_ADC);
	WriteLog(1, print, 1);

	sprintf((char *)print,"count_RS485_1:%d\r\n",count_RS485_1);
	WriteLog(1, print, 1);

	sprintf((char *)print,"count_RS485_2:%d\r\n",count_RS485_2);
	WriteLog(1, print, 1);

	sprintf((char *)print,"count_RS232_1:%d\r\n",count_RS232_1);
	WriteLog(1, print, 1);

	sprintf((char *)print,"count_RS232_2:%d\r\n",count_RS232_2);
	WriteLog(1, print, 1);

	sprintf((char *)print,"modem_MQTT_publish_success_timer:%d\r\n",modem_MQTT_publish_success_timer);
	WriteLog(1, print, 1);

	sprintf((char *)print,"Count:***************END*******************\r\n\r\n");
	WriteLog(1, print, 1);
#endif

}
/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart3.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart3, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart3, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */
  HAL_UART_ReceiverTimeout_Config(&huart3,10); // maulin change from 100 to 10
  HAL_UART_EnableReceiverTimeout(&huart3);

  /* USER CODE END USART3_Init 2 */

}

