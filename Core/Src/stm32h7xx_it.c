/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32h7xx_it.c
  * @brief   Interrupt Service Routines.
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
#include "stm32h7xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern UART_HandleTypeDef huart8;
extern UART_HandleTypeDef huart2;
extern ETH_HandleTypeDef heth;
extern TIM_HandleTypeDef htim17;
extern UART_HandleTypeDef huart3;
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
  while (1)
  {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Pre-fetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/******************************************************************************/
/* STM32H7xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32h7xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles EXTI line0 interrupt.
  */
void EXTI0_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI0_IRQn 0 */

  /* USER CODE END EXTI0_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(DI1_Pulse_Pin);
  /* USER CODE BEGIN EXTI0_IRQn 1 */

  /* USER CODE END EXTI0_IRQn 1 */
}

/**
  * @brief This function handles EXTI line[9:5] interrupts.
  */
void EXTI9_5_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI9_5_IRQn 0 */
//BaseType_t xHigherPriorityTaskWoken = pdFALSE;
////#ifdef SL_WFX_USE_SPI
//  if (__HAL_GPIO_EXTI_GET_IT(SL_WFX_IRQ_GPIO_SPI) != RESET) {
//    xSemaphoreGiveFromISR(wfx_wakeup_sem, &xHigherPriorityTaskWoken);
//    vTaskNotifyGiveFromISR( busCommTaskHandle, &xHigherPriorityTaskWoken );
//  }
////#endif /* SL_WFX_USE_SPI */

  /* USER CODE END EXTI9_5_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(DI2_Pulse_Pin);
  /* USER CODE BEGIN EXTI9_5_IRQn 1 */
//portYIELD_FROM_ISR( xHigherPriorityTaskWoken );


  /* USER CODE END EXTI9_5_IRQn 1 */
}

uint16_t maulin_RxXferSize=0,maulin_RxXferCount=0,maulin_NbRxDataToProcess=0;
uint8_t maulin_halftx=0,maulin_fulltx=0,maulin_ideal;
/**
  * @brief This function handles USART2 global interrupt.
  */
void USART2_IRQHandler(void)
{
  /* USER CODE BEGIN USART2_IRQn 0 */
  //BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  /* USER CODE END USART2_IRQn 0 */
  HAL_UART_IRQHandler(&huart2);
  /* USER CODE BEGIN USART2_IRQn 1 */
  if(huart2.ErrorCode == HAL_UART_ERROR_RTO)
  {
	  //COM_RS485_1_RX = 1 ;
	  //vTaskNotifyGiveFromISR( COM_PORT_RS485_1_TaskHandle, &xHigherPriorityTaskWoken );
	  maulin_ideal++;
	  dataSendToRxRingFiller(&lora_rx_rb, Lora_RX_Buff, sizeof(Lora_RX_Buff), huart2.RxXferCount, 0);
  }
  HAL_UART_Receive_IT(&huart2, Lora_RX_Buff, sizeof(Lora_RX_Buff));
  //portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
  /* USER CODE END USART2_IRQn 1 */
}



void USART3_IRQHandler(void)
{
  /* USER CODE BEGIN USART3_IRQn 0 */
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  /* USER CODE END USART3_IRQn 0 */
  HAL_UART_IRQHandler(&huart3);
  /* USER CODE BEGIN USART3_IRQn 1 */
  if(huart3.ErrorCode == HAL_UART_ERROR_RTO)
  {
	  if(beforeRTOS==1)
	  {
		  // STNo:2
		if((ModbusH[COM_RS232_1].u8RxBuffer[0]=='S')&&(ModbusH[COM_RS232_1].u8RxBuffer[1]=='T')&&(ModbusH[COM_RS232_1].u8RxBuffer[2]=='N')&&(ModbusH[COM_RS232_1].u8RxBuffer[3]=='o')&&(ModbusH[COM_RS232_1].u8RxBuffer[4]==':'))
		{
			SlotNo_RS232_1=atoi_new((const char*)&ModbusH[COM_RS232_1].u8RxBuffer[5],strlen((const char*)ModbusH[COM_RS232_1].u8RxBuffer)-5);
		}
	  }
	  else
	  {
		  vTaskNotifyGiveFromISR( COM_PORT_RS232_1_TaskHandle, &xHigherPriorityTaskWoken );
	  }
  }
  HAL_UART_Receive_IT(&huart3, &ModbusH[COM_RS232_1].u8RxBuffer[0], sizeof(ModbusH[COM_RS232_1].u8RxBuffer));
  portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
  /* USER CODE END USART3_IRQn 1 */
}
/**
  * @brief This function handles UART4 global interrupt.
  */

//uint16_t maulin_RxXferSize=0,maulin_RxXferCount=0,maulin_NbRxDataToProcess=0;
//uint8_t maulin_halftx=0,maulin_fulltx=0,maulin_ideal;
//
//void UART4_IRQHandler(void)
//{
//  /* USER CODE BEGIN UART4_IRQn 0 */
//	//BaseType_t xHigherPriorityTaskWoken = pdFALSE;
//  /* USER CODE END UART4_IRQn 0 */
//	HAL_UART_IRQHandler(&huart4);
//  /* USER CODE BEGIN UART4_IRQn 1 */
//    if(huart4.ErrorCode == HAL_UART_ERROR_RTO)
//    {
//    	//maulin_RxXferSize=huart4.RxXferSize;               /*!< UART Rx Transfer size              */
//    	//maulin_RxXferCount=huart4.RxXferCount;              /*!< UART Rx Transfer Counter           */
//    	//maulin_NbRxDataToProcess=huart4.NbRxDataToProcess;
//    	maulin_ideal++;
//    	dataSendToRxRingFiller(0);
//    	//COM_RS232_1_RX = 1 ;
//    	//vTaskNotifyGiveFromISR( EC200U_RxRingFiller_TaskHandle, &xHigherPriorityTaskWoken );
//    }
//
//    HAL_UART_Receive_IT(&huart4, &EC200U_RX_Buff[0], sizeof(EC200U_RX_Buff));
//    //portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
//    /* USER CODE END UART4_IRQn 1 */
//}
/**
  * @brief This function handles UART8 global interrupt.
  */
void UART8_IRQHandler(void)
{
  /* USER CODE BEGIN UART8_IRQn 0 */
	//BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  /* USER CODE END UART8_IRQn 0 */
	HAL_UART_IRQHandler(&huart8);
  /* USER CODE BEGIN UART8_IRQn 1 */
    if(huart8.ErrorCode == HAL_UART_ERROR_RTO)
    {
    	//maulin_RxXferSize=huart8.RxXferSize;               /*!< UART Rx Transfer size              */
    	//maulin_RxXferCount=huart8.RxXferCount;              /*!< UART Rx Transfer Counter           */
    	//maulin_NbRxDataToProcess=huart8.NbRxDataToProcess;
    	maulin_ideal++;
    	dataSendToRxRingFiller(&EC200U_RX_rb, EC200U_RX_Buff, sizeof(EC200U_RX_Buff), huart8.RxXferCount, 0);
    	//COM_RS232_1_RX = 1 ;
    	//vTaskNotifyGiveFromISR( EC200U_RxRingFiller_TaskHandle, &xHigherPriorityTaskWoken );
    }

    HAL_UART_Receive_IT(&huart8, &EC200U_RX_Buff[0], sizeof(EC200U_RX_Buff));
    //portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
    /* USER CODE END UART8_IRQn 1 */
}


/**
  * @brief This function handles Ethernet global interrupt.
  */
void ETH_IRQHandler(void)
{
  /* USER CODE BEGIN ETH_IRQn 0 */

  /* USER CODE END ETH_IRQn 0 */
  HAL_ETH_IRQHandler(&heth);
  /* USER CODE BEGIN ETH_IRQn 1 */

  /* USER CODE END ETH_IRQn 1 */
}

/**
  * @brief This function handles TIM17 global interrupt.
  */
void TIM17_IRQHandler(void)
{
  /* USER CODE BEGIN TIM17_IRQn 0 */

  /* USER CODE END TIM17_IRQn 0 */
  HAL_TIM_IRQHandler(&htim17);
  /* USER CODE BEGIN TIM17_IRQn 1 */

  /* USER CODE END TIM17_IRQn 1 */
}

/* USER CODE BEGIN 1 */

void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance==USART2)
	{
		maulin_halftx++;
		dataSendToRxRingFiller(&lora_rx_rb, Lora_RX_Buff, sizeof(Lora_RX_Buff), huart->RxXferCount, 1);
	}
	if(huart->Instance==UART8)
	{
		maulin_halftx++;
		dataSendToRxRingFiller(&EC200U_RX_rb, EC200U_RX_Buff, sizeof(EC200U_RX_Buff), huart->RxXferCount, 1);
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance==USART2)
	{
		maulin_fulltx++;
		dataSendToRxRingFiller(&lora_rx_rb, Lora_RX_Buff, sizeof(Lora_RX_Buff), huart->RxXferCount, 2);
	}
	if(huart->Instance==UART8)
	{
		maulin_fulltx++;
		dataSendToRxRingFiller(&EC200U_RX_rb, EC200U_RX_Buff, sizeof(EC200U_RX_Buff), huart->RxXferCount, 2);
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	switch(GPIO_Pin)
	{
		case DI1_Pulse_Pin:
		{
			DI1_Pulse_Count++;
			DI1_Pulse_Count_for_Frequency++;
			DI1_Freq_fromWaveLength=1000.000/DI1_waveLength;
			DI1_waveLength = 0;
			break;
		}
		case DI2_Pulse_Pin:
		{
			DI2_Pulse_Count++;
			DI2_Pulse_Count_for_Frequency++;
			DI2_Freq_fromWaveLength=1000.000/DI2_waveLength;
			DI2_waveLength = 0;
			break;
		}
		default:
			break;
	}

}
/* USER CODE END 1 */
