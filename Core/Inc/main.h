/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include "cJSON.h"
#include "lwrb.h"
#include "modbus.h"
#include "cmsis_os.h"
#include "ADC.h"
#include "common.h"
#include "COM_PORT_RS485_1.h"
#include "COM_PORT_RS485_2.h"
#include "COM_PORT_RS232_1.h"
#include "COM_PORT_RS232_2.h"
#include "EC200U.h"
#include <RxRingProcess.h>
#include "Modem_MQTT.h"
#include "modem_GPS.h"
#include "modem_BLE.h"
#include "ATmodemTypes.h"
#include "json_parser.h"
#include "udpserver.h"
#include "tcpserver.h"
#include "ModbusTCP.h"
#include "OTA.h"
#include "DIDO.h"
#include "w25q_mem.h"
#include "libs.h"
#include "RTC_Time.h"
#include "time.h"
#include "pcbplcTimerTask.h"
#include "pcbplcService.h"
#include "define.h"
#include "Configuration.h"
#include "pcbplc.h"
#include "pcbplcTask.h"
#include "Ethernet_MQTT.h"
#include "Lcd_16x2.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
extern int count_DO;
extern uint32_t count_ADC;
extern void reboot_device_func(void);

extern uint32_t stm32deviceID[3],STM32_CRC32;

extern ADC_HandleTypeDef hadc1;

extern QSPI_HandleTypeDef hqspi;
extern I2C_HandleTypeDef hi2c1;
extern RTC_HandleTypeDef hrtc;

extern IWDG_HandleTypeDef hiwdg1;

extern UART_HandleTypeDef huart4;
extern UART_HandleTypeDef huart5;
extern UART_HandleTypeDef huart8;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern osSemaphoreId sendExternalFlashSemaphore;

extern uint8_t Lora_RX_Buff[50];        	//LoRA buffer to fill from Rx interrupt
extern lwrb_t lora_rx_rb;  					//LoRA Ring buffer instance for RX data
extern uint8_t lora_rx_rb_data[1000];		//LoRA Ring buffer data array for RX DMA
/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
//#define WATCH_DOG_ENABLE
#define SVC_DEBUG

#define DO1_Pin GPIO_PIN_3
#define DO1_GPIO_Port GPIOE
#define DO2_Pin GPIO_PIN_4
#define DO2_GPIO_Port GPIOE
#define DO3_Pin GPIO_PIN_5
#define DO3_GPIO_Port GPIOE
#define DO4_Pin GPIO_PIN_6
#define DO4_GPIO_Port GPIOE
#define DO5_Pin GPIO_PIN_13
#define DO5_GPIO_Port GPIOC
#define Watchdog_GPIO_Port GPIOC
#define Watchdog_Pin GPIO_PIN_0
#define DO6_Pin GPIO_PIN_2
#define DO6_GPIO_Port GPIOC
#define USB_RST_Pin GPIO_PIN_3
#define USB_RST_GPIO_Port GPIOC
#define DI1_Pulse_Pin GPIO_PIN_0
#define DI1_Pulse_GPIO_Port GPIOA
#define DI1_Pulse_EXTI_IRQn EXTI0_IRQn
#define AI1_Pin GPIO_PIN_3
#define AI1_GPIO_Port GPIOA
#define AI2_Pin GPIO_PIN_4
#define AI2_GPIO_Port GPIOA
#define AI3_Pin GPIO_PIN_5
#define AI3_GPIO_Port GPIOA
#define AI4_Pin GPIO_PIN_6
#define AI4_GPIO_Port GPIOA
#define AI5_Pin GPIO_PIN_0
#define AI5_GPIO_Port GPIOB
#define AI6_Pin GPIO_PIN_1
#define AI6_GPIO_Port GPIOB
#define DI2_Pulse_Pin GPIO_PIN_7
#define DI2_Pulse_GPIO_Port GPIOE
#define DI2_Pulse_EXTI_IRQn EXTI9_5_IRQn
#define DI3_Pin GPIO_PIN_8
#define DI3_GPIO_Port GPIOE
#define DI4_Pin GPIO_PIN_9
#define DI4_GPIO_Port GPIOE
#define DI5_Pin GPIO_PIN_10
#define DI5_GPIO_Port GPIOE
#define DI6_Pin GPIO_PIN_11
#define DI6_GPIO_Port GPIOE
#define DI7_Pin GPIO_PIN_12
#define DI7_GPIO_Port GPIOE
#define DI8_Pin GPIO_PIN_13
#define DI8_GPIO_Port GPIOE
#define DO7_Pin GPIO_PIN_14
#define DO7_GPIO_Port GPIOE
#define DO8_Pin GPIO_PIN_15
#define DO8_GPIO_Port GPIOE
#define DO9_Pin GPIO_PIN_14
#define DO9_GPIO_Port GPIOB
#define DO10_Pin GPIO_PIN_15
#define DO10_GPIO_Port GPIOB
#define DO11_Pin GPIO_PIN_8
#define DO11_GPIO_Port GPIOD
#define DO12_Pin GPIO_PIN_9
#define DO12_GPIO_Port GPIOD
#define DO13_Pin GPIO_PIN_10
#define DO13_GPIO_Port GPIOD
#define DO14_Pin GPIO_PIN_14
#define DO14_GPIO_Port GPIOD
#define DO15_Pin GPIO_PIN_15
#define DO15_GPIO_Port GPIOD
#define DO16_Pin GPIO_PIN_6
#define DO16_GPIO_Port GPIOC
#define DO17_Pin GPIO_PIN_7
#define DO17_GPIO_Port GPIOC
#define DO18_Pin GPIO_PIN_9
#define DO18_GPIO_Port GPIOC
#define DO19_Pin GPIO_PIN_8
#define DO19_GPIO_Port GPIOA
#define DO20_Pin GPIO_PIN_9
#define DO20_GPIO_Port GPIOA
#define DO21_Pin GPIO_PIN_10
#define DO21_GPIO_Port GPIOA
#define DO22_Pin GPIO_PIN_11
#define DO22_GPIO_Port GPIOA
#define DO23_Pin GPIO_PIN_12
#define DO23_GPIO_Port GPIOA

#define DO24_Pin GPIO_PIN_10
#define DO24_GPIO_Port GPIOA
#define DO25_Pin GPIO_PIN_11
#define DO25_GPIO_Port GPIOA

#define DO26_Pin GPIO_PIN_0
#define DO26_GPIO_Port GPIOD
#define LORA_RST_Pin GPIO_PIN_4
#define LORA_RST_GPIO_Port GPIOD
#define KEY_IN_Pin GPIO_PIN_7
#define KEY_IN_GPIO_Port GPIOD
#define DO_EN_Pin GPIO_PIN_5
#define DO_EN_GPIO_Port GPIOB
#define DISPLAY_BKLT_Pin GPIO_PIN_8
#define DISPLAY_BKLT_GPIO_Port GPIOB
#define LED_LORA_Pin GPIO_PIN_9
#define LED_LORA_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/**************************************************************************//**
 * Ennam / uninon / Structure
 *****************************************************************************/

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
