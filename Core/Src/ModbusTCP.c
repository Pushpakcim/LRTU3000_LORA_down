/*
 * ModbusTCP.c
 *
 *  Created on: Jan 4, 2023
 *      Author: maulin
 */
/**************************************************************************//**
 * Includes
 *****************************************************************************/

#include "main.h"
#include "ModbusTCP.h"
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "lwip/err.h"
/**************************************************************************//**
 * Variable
 *****************************************************************************/

unsigned int count_ModbusTCP = 0;
osThreadId ModbusTCP_TaskHandle;

/**************************************************************************//**
 * Function name 	: ModbusTCP_start
 * arguments		: 1)
 * return 		 	: no return type
 * Note				: #
 *****************************************************************************/

void ModbusTCP_start()
{
	osThreadDef(ModbusTCPTask, StartModbusTCPTask, osPriorityNormal, 0, 256*4); //512
	ModbusTCP_TaskHandle = osThreadCreate(osThread(ModbusTCPTask), NULL);
}

/**************************************************************************//**
 * Function name 	: ModbusTCPTask
 * arguments		: 1)
 * return 		 	:
 * Note				:
 * 					:
 * 					:
 *****************************************************************************/

void StartModbusTCPTask(void const * argument)
{

	if(ModbusH[COM_MODBUSTCP_1].uModbusType == MB_SLAVE)
	{
		TCPinitserver(&ModbusH[COM_MODBUSTCP_1]); // start the Modbus server slave
	}
	else
	{

	}

	for(;;)
	{
		//count_ModbusTCP++;
		count_ModbusTCP = 0;
		if(ModbusH[COM_MODBUSTCP_1].uModbusType == MB_MASTER)
		{
			osDelay(1000);
		}
		else
		{
			if(TCPwaitConnData(&ModbusH[COM_MODBUSTCP_1]) == false) // wait for connection and receive data
			{
				continue; // TCP package was not validated
			}
			//ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
			osDelay(50);
			ProcessModbusSlave(&ModbusH[COM_MODBUSTCP_1]);
			osDelay(50);
		}
	}
}

