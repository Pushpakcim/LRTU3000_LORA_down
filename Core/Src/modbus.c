
/*
 * mosbus.c
 *
 *  Created on: Nov 12, 2022
 *      Author: SanketP
 */

/**************************************************************************//**
 * Includes
 *****************************************************************************/

#include "main.h"
#include "modbus.h"
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "lwip/err.h"
#include "lwip/api.h"
#include "pcbplccomm.h"

/**************************************************************************//**
 * Variable
 *****************************************************************************/
extern UART_HandleTypeDef huart3;
uint8_t Digital_bit_Query_array[100];
uint8_t gNoofQueryStored;
uint16_t gTotalNumberOfReadDataUsingAllQuery;
modbus_t telegram[MODBUS_MASTER_MAX_TOTAL_QUERY];
modbusHandler_t ModbusH[6]; // [0 : COM_RS485_1] [1 : COM_RS485_2] 	[2 : COM_RS232_1]  [3 : COM_RS232_2]  [4 : COM_MODBUSTCP_1] [5 : COM_LORA]
uint16_t ModbusDATA[100]={11,22,33,44,55,66,77,88};
char RTCsync_flage=0;
const unsigned char fctsupported[] =
{
    MB_FC_READ_COILS,
    MB_FC_READ_DISCRETE_INPUT,
    MB_FC_READ_REGISTERS,
    MB_FC_READ_INPUT_REGISTER,
    MB_FC_WRITE_COIL,
    MB_FC_WRITE_REGISTER,
    MB_FC_WRITE_MULTIPLE_COILS,
    MB_FC_WRITE_MULTIPLE_REGISTERS
};

void WriteLog(uint8_t LogEnable,const char *pData,uint8_t logType);

/**************************************************************************//**
 * Function name 	: ModbusInit
 * arguments		: 1) modH : ModbusPort Handler
 * 		 		 	: 2)
 * return			:
 * Note				:
 * 					:
 *****************************************************************************/

void ModbusInit(modbusHandler_t * modH)
{

	modH->port = &huart2; //HAL Serial Port handler
	modH->u8id = 1; //!< 0=master, 1..247=slave number
	//modH->u8Buffer[0] = '\0'; //Modbus buffer for communication
	modH->u8BufferSize = 0;
	modH->u16regs = 0;
	modH-> u16regsize = 0;

}

/**************************************************************************//**
 * Function name 	: ModbusInitData
 * arguments		: 1) telegram : Modbus Query Structure
 * 		 		 	: 2)
 * return			:
 * Note				: Feel telegram Structure with Query Data
 * 					:
 *****************************************************************************/

void ModbusInitData(modbus_t *telegram)
 {
 	telegram->uQueryNo = 0;
 	telegram->uPortNo = 1;
 	telegram->u8id = 1;          /*!< Slave address between 1 and 247. 0 means broadcast */
 	telegram->u8fct = 6;         /*!< Function code: 1, 2, 3, 4, 5, 6, 15 or 16 */
 	telegram->u16RegAdd = 1;    /*!< Address of the first register to access at slave/s */
 	telegram->u16CoilsNo = 1;   /*!< Number of coils or registers to access */
 	telegram->u16reg[0] = 123;
  }

 /**************************************************************************//**
  * Function name 	: validateAnswer
  * arguments		: 1) modH : ModbusPort Handler
  * 		 		: 2)
  * return			: 0 if OK, EXCEPTION if anything fails
  * Note			: This method validates slave incoming messages
  * 				:
  *****************************************************************************/

uint8_t validateAnswer(modbusHandler_t *modH)
 {
	 uint8_t isSupported =0;
     // check message crc vs calculated crc
 	uint16_t u16MsgCRC =
         ((modH->u8RxBuffer[modH->u8BufferSize - 2] << 8)
          | modH->u8RxBuffer[modH->u8BufferSize - 1]); // combine the crc Low & High bytes
     if ( calcCRC(modH->u8RxBuffer,  modH->u8BufferSize-2) != u16MsgCRC )
     {
     	//modH->u16errCnt ++;
         return 2;
     }

     // check exception
     if ((modH->u8Buffer[ FUNC ] & 0x80) != 0)
     {
     	//modH->u16errCnt ++;
         return 3;
     }


     for (uint8_t i = 0; i< sizeof( fctsupported ); i++)
     {
         if (fctsupported[i] == modH->u8Buffer[FUNC])
         {
             isSupported = 1;
             break;
         }
     }
     if (!isSupported)
     {
     	//modH->u16errCnt ++;
         return EXC_FUNC_CODE;
     }

     return 0; // OK, no exception code thrown
 }

/**************************************************************************//**
 * Function name 	: validateAnswer (Not used)
 * arguments		: 1) modH : ModbusPort Handler
 * 		 			: 2)
 * return			: 0 if OK, EXCEPTION if anything fails
 * Note				: This method validates slave incoming messages
 * 					:
 *****************************************************************************/

//uint8_t validateRequest(modbusHandler_t *modH)
// {
// 	// check message crc vs calculated crc
//
//
// 	    uint16_t u16MsgCRC,isSupported;
// 	    u16MsgCRC= ((modH->u8Buffer[modH->u8BufferSize - 2] << 8)
// 	    		   	         | modH->u8Buffer[modH->u8BufferSize - 1]); // combine the crc Low & High bytes
//
//
// 	    if ( calcCRC( modH->u8Buffer,  modH->u8BufferSize-2 ) != u16MsgCRC )
// 	    {
// 	       		//modH->u16errCnt ++;
// 	       		return 2;
// 	    }
//
// 	    // check fct code
// 	    for (uint8_t i = 0; i< sizeof( fctsupported ); i++)
// 	    {
// 	        if (fctsupported[i] == modH->u8Buffer[FUNC])
// 	        {
// 	            isSupported = 1;
// 	            break;
// 	        }
// 	    }
// 	    if (!isSupported)
// 	    {
// 	    	//modH->u16errCnt ++;
// 	        return EXC_FUNC_CODE;
// 	    }
//
// 	    // check start address & nb range
// 	    uint16_t u16AdRegs = 0;
// 	    uint16_t u16NRegs = 0;
//
// 	    //uint8_t u8regs;
// 	    switch ( modH->u8Buffer[ FUNC ] )
// 	    {
// 	    case MB_FC_READ_COILS:
// 	    case MB_FC_READ_DISCRETE_INPUT:
// 	    case MB_FC_WRITE_MULTIPLE_COILS:
// 	    	u16AdRegs = word( modH->u8Buffer[ ADD_HI ], modH->u8Buffer[ ADD_LO ]) / 16;
// 	    	u16NRegs = word( modH->u8Buffer[ NB_HI ], modH->u8Buffer[ NB_LO ]) /16;
// 	    	if(word( modH->u8Buffer[ NB_HI ], modH->u8Buffer[ NB_LO ]) % 16) u16NRegs++; // check for incomplete words
// 	    	// verify address range
// 	    	if((u16AdRegs + u16NRegs) > modH->u16regsize) return EXC_ADDR_RANGE;
//
// 	    	//verify answer frame size in bytes
//
// 	    	u16NRegs = word( modH->u8Buffer[ NB_HI ], modH->u8Buffer[ NB_LO ]) / 8;
// 	    	if(word( modH->u8Buffer[ NB_HI ], modH->u8Buffer[ NB_LO ]) % 8) u16NRegs++;
// 	    	u16NRegs = u16NRegs + 5; // adding the header  and CRC ( Slave address + Function code  + number of data bytes to follow + 2-byte CRC )
// 	        if(u16NRegs > 256) return EXC_REGS_QUANT;
//
// 	        break;
// 	    case MB_FC_WRITE_COIL:
// 	    	u16AdRegs = word( modH->u8Buffer[ ADD_HI ], modH->u8Buffer[ ADD_LO ]) / 16;
// 	    	if(word( modH->u8Buffer[ ADD_HI ], modH->u8Buffer[ ADD_LO ]) % 16) u16AdRegs++;	// check for incomplete words
// 	        if (u16AdRegs > modH->u16regsize) return EXC_ADDR_RANGE;
// 	        break;
// 	    case MB_FC_WRITE_REGISTER :
// 	    	u16AdRegs = word( modH->u8Buffer[ ADD_HI ], modH->u8Buffer[ ADD_LO ]);
// 	        if (u16AdRegs > modH-> u16regsize) return EXC_ADDR_RANGE;
// 	        break;
// 	    case MB_FC_READ_REGISTERS :
// 	    case MB_FC_READ_INPUT_REGISTER :
// 	    case MB_FC_WRITE_MULTIPLE_REGISTERS :
// 	    	u16AdRegs = word( modH->u8Buffer[ ADD_HI ], modH->u8Buffer[ ADD_LO ]);
// 	        u16NRegs = word( modH->u8Buffer[ NB_HI ], modH->u8Buffer[ NB_LO ]);
// 	        if (( u16AdRegs + u16NRegs ) > modH->u16regsize) return EXC_ADDR_RANGE;
//
// 	        //verify answer frame size in bytes
// 	        u16NRegs = u16NRegs*2 + 5; // adding the header  and CRC
// 	        if ( u16NRegs > 256 ) return EXC_REGS_QUANT;
// 	        break;
// 	    }
// 	    return 0; // OK, no exception code thrown
//
// }

/**************************************************************************//**
 * Function name 	: Master_Parse_Modbus_Responce
 * arguments		: 1) argument : modH : ModbusPort Handler
 * 		 			: 2) argument2 : telegram
 * return			:
 * Note				:
 * 					:
 *****************************************************************************/
void Master_Parse_Modbus_Responce(void *argument,void *argument2)
{

	modbusHandler_t *modH =  (modbusHandler_t *)argument;
	modbus_t *telegram = (modbus_t *)argument2;

	// This is the case for implementations with only USART support
	// SendQuery(modH, telegram);
	//getRxBuffer(modH);

//	modH->u8BufferSize = modH->u8RxBuffer[2]+3;
//	telegram->u8rxdataValidation = validateAnswer(modH);
//	if (telegram->u8rxdataValidation != 0)
//	{
//
//	}
//	else
	{
		switch( modH->u8Buffer[ FUNC ] )
		{
			case MB_FC_READ_COILS:
			case MB_FC_READ_DISCRETE_INPUT:
			  //call get_FC1 to transfer the incoming message to u16regs buffer
			  get_FC1(modH,telegram);
			  break;
			case MB_FC_READ_INPUT_REGISTER:
			case MB_FC_READ_REGISTERS :
			  // call get_FC3 to transfer the incoming message to u16regs buffer
			  get_FC3(modH,telegram);
			  break;
			case MB_FC_WRITE_COIL:
			case MB_FC_WRITE_REGISTER :
			case MB_FC_WRITE_MULTIPLE_COILS:
			case MB_FC_WRITE_MULTIPLE_REGISTERS :
				get_FC5_FC6(modH,telegram);
			  // nothing to do
			  break;
			default:
			  break;
		}
	}

}

/**************************************************************************//**
 * Function name 	: Master_Send_Modbus_Query
 * arguments		: 1) argument : modH : ModbusPort Handler
 * 		 			: 2) telegram
 * return			:
 * Note				:
 * 					:
 *****************************************************************************/

void Master_Send_Modbus_Query(void *argument, modbus_t *telegram)
 {
	 modbusHandler_t *modH =  (modbusHandler_t *)argument;
	 //uint32_t notification;

	 //modH->i8lastError = 0;
	 // validate message: CRC, FCT, address and size
	 //uint8_t u8exception = validateRequest(modH);
	// if (u8exception > 0)
	// {}
	// else
	 {
		 // process message
		 switch(telegram->u8fct )
		 {
 			case MB_FC_READ_COILS:
 			case MB_FC_READ_DISCRETE_INPUT:
 				modH->i8state = SendQuery(modH, telegram);
 				break;
 			case MB_FC_READ_INPUT_REGISTER:
 			case MB_FC_READ_REGISTERS :
 				modH->i8state = SendQuery(modH, telegram);
 				break;
 			case MB_FC_WRITE_COIL:
 				modH->i8state = SendQuery(modH, telegram);
 				break;
 			case MB_FC_WRITE_REGISTER :
 				modH->i8state = SendQuery(modH, telegram);
 				break;
 			case MB_FC_WRITE_MULTIPLE_COILS:
 				modH->i8state = SendQuery(modH, telegram);
 				break;
 			case MB_FC_WRITE_MULTIPLE_REGISTERS :
 				modH->i8state = SendQuery(modH, telegram);
 				break;
 			default:
 				break;
		 }
	 }
 }

/**************************************************************************//**
 * Function name 	: SendQuery
 * arguments		: 1) modH : ModbusPort Handler
 * 		 			: 2) telegram
 * return			:
 * Note				:
 * 					:
 *****************************************************************************/

int8_t SendQuery(modbusHandler_t *modH ,  modbus_t *telegram )
{

	uint8_t u8regsno, u8bytesno;

	modH->u16regs = telegram->u16reg;

	// telegram header
	modH->u8Buffer[ ID ]         = telegram->u8id;
	modH->u8Buffer[ FUNC ]       = telegram->u8fct;
	modH->u8Buffer[ ADD_HI ]     = highByte(telegram->u16RegAdd );
	modH->u8Buffer[ ADD_LO ]     = lowByte( telegram->u16RegAdd );

	switch( telegram->u8fct )
	{
	case MB_FC_READ_COILS:
	case MB_FC_READ_DISCRETE_INPUT:
	case MB_FC_READ_REGISTERS:
	case MB_FC_READ_INPUT_REGISTER:
		modH->u8Buffer[ NB_HI ]      = highByte(telegram->u16CoilsNo );
		modH->u8Buffer[ NB_LO ]      = lowByte( telegram->u16CoilsNo );
		modH->u8BufferSize = 6;
		break;
	case MB_FC_WRITE_COIL:
		modH->u8Buffer[ NB_HI ]      = (( telegram->u16reg[0]> 0) ? 0xff : 0);
		modH->u8Buffer[ NB_LO ]      = 0;
		modH->u8BufferSize = 6;
		break;
	case MB_FC_WRITE_REGISTER:
		modH->u8Buffer[ NB_HI ]      = highByte( telegram->u16reg[0]);
		modH->u8Buffer[ NB_LO ]      = lowByte( telegram->u16reg[0]);
		modH->u8BufferSize = 6;
		break;
	case MB_FC_WRITE_MULTIPLE_COILS: // TODO: implement "sending coils"
		u8regsno = telegram->u16CoilsNo / 16;
		u8bytesno = u8regsno * 2;
		if ((telegram->u16CoilsNo % 16) != 0)
		{
			u8bytesno++;
			u8regsno++;
		}

		modH->u8Buffer[ NB_HI ]      = highByte(telegram->u16CoilsNo );
		modH->u8Buffer[ NB_LO ]      = lowByte( telegram->u16CoilsNo );
		modH->u8Buffer[ BYTE_CNT ]    = u8bytesno;
		modH->u8BufferSize = 7;

		for (uint16_t i = 0; i < u8bytesno; i++)
		{
			if(i%2)
			{
				modH->u8Buffer[ modH->u8BufferSize ] = lowByte( telegram->u16reg[ i/2 ] );
			}
			else
			{
				modH->u8Buffer[  modH->u8BufferSize ] = highByte( telegram->u16reg[ i/2 ] );

			}
			modH->u8BufferSize++;
		}
		break;

	case MB_FC_WRITE_MULTIPLE_REGISTERS:
		modH->u8Buffer[ NB_HI ]      = highByte(telegram->u16CoilsNo );
		modH->u8Buffer[ NB_LO ]      = lowByte( telegram->u16CoilsNo );
		modH->u8Buffer[ BYTE_CNT ]    = (uint8_t) ( telegram->u16CoilsNo * 2 );
		modH->u8BufferSize = 7;

		for (uint16_t i=0; i< telegram->u16CoilsNo; i++)
		{

			modH->u8Buffer[  modH->u8BufferSize ] = highByte(  telegram->u16reg[ i ] );
			modH->u8BufferSize++;
			modH->u8Buffer[  modH->u8BufferSize ] = lowByte( telegram->u16reg[ i ] );
			modH->u8BufferSize++;
		}
		break;
	}
	sendTxBuffer(modH);
	return 0;
}

/**************************************************************************//**
 * Function name 	: calcCRC
 * arguments		: 1) *Buffer
 * 		 			: 2) u8length
 * return			: CRC
 * Note				:
 * 					:
 *****************************************************************************/

uint16_t calcCRC(uint8_t *Buffer, uint8_t u8length)
{
    unsigned int temp, temp2, flag;
    temp = 0xFFFF;
    for (unsigned char i = 0; i < u8length; i++)
    {
        temp = temp ^ Buffer[i];
        for (unsigned char j = 1; j <= 8; j++)
        {
            flag = temp & 0x0001;
            temp >>=1;
            if (flag)
                temp ^= 0xA001;
        }
    }
    // Reverse byte order.
    temp2 = temp >> 8;
    temp = (temp << 8) | temp2;
    temp &= 0xFFFF;
    // the returned value is already swapped
    // crcLo byte is first & crcHi byte is last
    return temp;

}

/**************************************************************************//**
 * Function name 	: sendTxBuffer
 * arguments		: 1) modH : ModbusPort Handler
 * 		 			:
 * return			:
 * Note				: send Query over selected port
 * 					:
 *****************************************************************************/

void sendTxBuffer(modbusHandler_t *modH)
{
    // append CRC to message

    if(modH->u8Port != COM_MODBUSTCP_1)
	{
		uint16_t u16crc = calcCRC(modH->u8Buffer, modH->u8BufferSize);
		modH->u8Buffer[ modH->u8BufferSize ] = u16crc >> 8;
		modH->u8BufferSize++;
		modH->u8Buffer[ modH->u8BufferSize ] = u16crc & 0x00ff;
		modH->u8BufferSize++;
	}

   // transfer buffer to serial line IT
    //HAL_UART_Transmit(modH->port, modH->u8Buffer,  modH->u8BufferSize,300);


//    if(modH->u8Port == COM_RS485_1)
//    {
//    	HAL_UART_Transmit(modH->port, modH->u8Buffer,  modH->u8BufferSize,1000);
//    }
//    else if(modH->u8Port == COM_RS485_2)
//    {
//        //HAL_UART_Transmit(modH->port, modH->u8Buffer,  modH->u8BufferSize,1000);
//    }
//    else if(modH->u8Port == COM_RS232_1)
//	{
//        HAL_UART_Transmit(&huart4, modH->u8Buffer,  modH->u8BufferSize,1000);
//	}
//    else if(modH->u8Port == COM_RS232_2)
//	{
//        //HAL_UART_Transmit(modH->port, modH->u8Buffer,  modH->u8BufferSize,1000);
//	}

    if(modH->u8Port == COM_MODBUSTCP_1)
	{
  	  struct netvector  xNetVectors[2];
  	  uint8_t u8MBAPheader[6];
  	  size_t uBytesWritten;


  	  u8MBAPheader[0] = highByte(modH->u16TransactionID); // this might need improvement the transaction ID could be validated
  	  u8MBAPheader[1] = lowByte(modH->u16TransactionID);
  	  u8MBAPheader[2] = 0; //protocol ID
  	  u8MBAPheader[3] = 0; //protocol ID
  	  u8MBAPheader[4] = 0; //highbyte data length always 0
  	  u8MBAPheader[5] = modH->u8BufferSize; //highbyte data length

  	  xNetVectors[0].len = 6;
  	  xNetVectors[0].ptr = (void *) u8MBAPheader;

  	  xNetVectors[1].len = modH->u8BufferSize;
  	  xNetVectors[1].ptr = (void *) modH->u8Buffer;


  	  netconn_set_sendtimeout(modH->newconns[modH->newconnIndex].conn, modH->u16timeOut);
  	  err_enum_t err;

  	  err = netconn_write_vectors_partly(modH->newconns[modH->newconnIndex].conn, xNetVectors, 2, NETCONN_COPY, &uBytesWritten);
  	  if (err != ERR_OK )
  	  {

  		 // ModbusCloseConn(modH->newconns[modH->newconnIndex].conn);
  		 ModbusCloseConnNull(modH);

  	  }


  	  if(modH->uModbusType == MB_MASTER )
  	  {
  	    //xTimerReset(modH->xTimerTimeout,0);
  	  }
	}
    else if(modH->u8Port == COM_LORA)
    {
    	memset(lora_tx_buf,0,sizeof(lora_tx_buf));
    	asciiStringToHexString((char *)modH->u8Buffer,lora_tx_buf, modH->u8BufferSize);
    }
    else
	{
		HAL_UART_Transmit(modH->port, modH->u8Buffer,  modH->u8BufferSize,1000);
	}

    modH->u8BufferSize = 0;
    // increase message counter
}

/**************************************************************************//**
 * Function name 	: process_FC1
 * arguments		: 1) modH : ModbusPort Handler
 * 		 			:
 * return			: u8BufferSize Response to master length
 * Note				: This method processes functions 1
 * 					:
 *****************************************************************************/

int8_t process_FC1(modbusHandler_t *modH )
{
    uint16_t u16currentRegister;
    uint8_t u8currentBit;
	uint8_t u8bytesno, u8bitsno;
    uint16_t u8CopyBufferSize;
    uint16_t u16currentCoil, u16coil;

    // get the first and last coil from the message
	
    uint16_t u16StartCoil = word( modH->u8RxBuffer[ ADD_HI ], modH->u8RxBuffer[ ADD_LO ] );
    uint16_t u16Coilno = word( modH->u8RxBuffer[ NB_HI ], modH->u8RxBuffer[ NB_LO ] );

    modH->u8Buffer[ ID ]       = modH->u8RxBuffer[ ID ];
    modH->u8Buffer[ FUNC ]     = modH->u8RxBuffer[ FUNC ];
	
    // put the number of bytes in the outcoming message

    u8bytesno = (uint8_t) (u16Coilno / 8);
    if (u16Coilno % 8 != 0)
	{
    	u8bytesno ++;
	}

    modH->u8Buffer[ ADD_HI ]  = u8bytesno;
    modH->u8BufferSize         = ADD_LO;
    modH->u8Buffer[modH->u8BufferSize + u8bytesno - 1 ] = 0;

    // read each coil from the register map and put its value inside the outcoming message

    u8bitsno = 0;


	for (u16currentCoil = 0; u16currentCoil < u16Coilno; u16currentCoil++)
	{
		u16coil = u16StartCoil + u16currentCoil;
		u16currentRegister =  (u16coil / 16);
		u8currentBit = (uint8_t) (u16coil % 16);

		if ((u16currentCoil + u16StartCoil + 1 >= 1)
				&& (u16currentCoil + u16StartCoil + 1 <= 960))  // for DI status
		{
			bitWrite(
				modH->u8Buffer[ modH->u8BufferSize ],
				u8bitsno,
				//bitRead( modH->u16regs[ u16currentRegister ], u8currentBit ) );
				dig_bit_array[u16currentCoil + u16StartCoil]);
		}
//		else if((u16currentCoil + u16StartCoil+1 >= 67) && (u16currentCoil + u16StartCoil+1 <= 74)) //TODO DO for status
//		{
//			bitWrite(
//				modH->u8Buffer[ modH->u8BufferSize ],
//				u8bitsno,(1-gFinalAnaValF[PHYSICAL_DO_gFinalAnaValF+u16currentCoil]));
//		}
		else
		{
			bitWrite(modH->u8Buffer[ modH->u8BufferSize ],u8bitsno,0);
		}

		u8bitsno ++;

		if (u8bitsno > 7)
		{
			u8bitsno = 0;
			modH->u8BufferSize++;
		}
	}

    // send outcoming message

    if (u16Coilno % 8 != 0)
	{
    	modH->u8BufferSize ++;
	}

    u8CopyBufferSize = modH->u8BufferSize +2;
    sendTxBuffer(modH);
    //stop :
    return u8CopyBufferSize;
}

/**************************************************************************//**
 * Function name 	: process_FC3
 * arguments		: 1) modH : ModbusPort Handler
 * 		 			:
 * return			: u8BufferSize Response to master length
 * Note				: This method processes functions 3 & 4
 * 					: This method reads a word array and transfers it to the master
 *****************************************************************************/

int8_t process_FC3(modbusHandler_t *modH)
{

    uint16_t u16StartAdd = word( modH->u8RxBuffer[ ADD_HI ], modH->u8RxBuffer[ ADD_LO ] );
    uint8_t u8regsno = word( modH->u8RxBuffer[ NB_HI ], modH->u8RxBuffer[ NB_LO ] );
    uint16_t u8CopyBufferSize = 0;
    uint16_t i,index;
    union_Datatypes D1;

    modH->u8Buffer[ ID ]       = modH->u8RxBuffer[ ID ];
    modH->u8Buffer[ FUNC ]     = modH->u8RxBuffer[ FUNC ];

    if(u8regsno % 2) // 4
    {
    	modH->u8Buffer[ 2 ]        = (u8regsno-1) * 2;
    }
    else
    {
    	modH->u8Buffer[ 2 ]        = u8regsno * 2;
    }
    modH->u8BufferSize         = 3;

    //for (i = u16StartAdd, index = 0; i < u16StartAdd + (u8regsno/2); ++i, ++index)
    for (i = u16StartAdd+1, index = 0; i < (u16StartAdd + u8regsno); i += 2, ++index)
    {
    	/* Physical DI*/
    	if((i >= 1) && (i < (1 + (16*2))))
		{
			D1.fl = gFinalAnaValF[1 + (i/2)];
		}
    	else if((i >= 67) && (i < (67 + (8*2))))
		{
			D1.fl = gFinalAnaValF[i/2];
		}
    	else if((i >= 103) && (i <= 113))
		{
			D1.fl = gFinalAnaValF[i/2];
		}
    	else if((i >= 115) && (i <= 129))
		{
			D1.fl = getFloatValueAddress(i);
		}
    	else if((i >= 91) && (i < (91 + (6*2))))
    	{
    		D1.fl = gFinalAnaValF[i/2];
    	}
    	/* General purpose array */
    	else if((i >= START_IDX_MODBUS_ANA_PARA) && (i < (START_IDX_MODBUS_ANA_PARA + (MAX_GEN_ANA_PARA*2))))
    	{
    		D1.fl = gFinalAnaValF[START_IDX_GEN_ANA_PARA_TAG+((i-START_IDX_MODBUS_ANA_PARA)/2)];
    	}
    	/* Modbus Slave parameters */
    	else if((i >= 761) && (i < (761 + (300*2))))
		{
			D1.fl = gFinalAnaValF[(i-160)/2];
		}
    	/* MISC parameters */
		else if((i >= 1361) && (i < (1361 + (98*2))))
		{
			D1.fl = gFinalAnaValF[(i-160)/2];
		}
    	/* Schedule */
    	else if((i >= 2971) && (i < (2971 + (420*2))))
		{
			D1.fl = gFinalAnaValF[i/2];
		}
    	/* Modbus Read*/
    	else if((i >= 3891) && (i < (3891 + (300*2))))
		{
			D1.fl = gFinalAnaValF[i/2];
		}
    	/* Modbus Write and Ack*/
    	else if((i >= 4491) && (i < (4491 + (40*2))))
		{
			D1.fl = gFinalAnaValF[i/2];
		}
    	/* Scalling parameters */
    	else if((i >= 4571) && (i < (4571 + (150*2))))
		{
			D1.fl = gFinalAnaValF[i/2];
		}
    	/* Recipe parameters */
		else if((i >= RECIPE_VAR_START_MODBUS_INDEX) && (i <= RECIPE_VAR_END_MODBUS_INDEX))
		{
			D1.fl = gFinalAnaValF[i/2];
		}
    	/* pulse DI parameters */
    	else if((i >=8701) && (i < (8701 + (80*2))))
		{
			D1.fl = gFinalAnaValF[i/2];
		}
    	/* pulse DO parameters */
    	else if((i >= 8861) && (i < (8861 + (130*2))))
		{
			D1.fl = gFinalAnaValF[i/2];
		}
		/* pulse Dual DO parameters */
    	else if((i >= 9201) && (i < (9201 + (26*2))))
		{
			D1.fl = gFinalAnaValF[i/2];
		}
    	else if((i >= 5901) && (i < (5901 + (4*4))))
    	{
    		switch(i)
			{
				case 5901:
				case 5902:
				{
					D1.s_ch[0] = Lora_Modem_Ascii_String_app_eui[1];
					D1.s_ch[1] = Lora_Modem_Ascii_String_app_eui[0];
					D1.s_ch[2] = Lora_Modem_Ascii_String_app_eui[3];
					D1.s_ch[3] = Lora_Modem_Ascii_String_app_eui[2];
					break;
				}
				case 5903:
				case 5904:
				{
					D1.s_ch[0] = Lora_Modem_Ascii_String_app_eui[5];
					D1.s_ch[1] = Lora_Modem_Ascii_String_app_eui[4];
					D1.s_ch[2] = Lora_Modem_Ascii_String_app_eui[7];
					D1.s_ch[3] = Lora_Modem_Ascii_String_app_eui[6];
					break;
				}
				case 5905:
				case 5906:
				{
					D1.s_ch[0] = Lora_Modem_Ascii_String_app_key[1];
					D1.s_ch[1] = Lora_Modem_Ascii_String_app_key[0];
					D1.s_ch[2] = Lora_Modem_Ascii_String_app_key[3];
					D1.s_ch[3] = Lora_Modem_Ascii_String_app_key[2];
					break;
				}
				case 5907:
				case 5908:
				{
					D1.s_ch[0] = Lora_Modem_Ascii_String_app_key[5];
					D1.s_ch[1] = Lora_Modem_Ascii_String_app_key[4];
					D1.s_ch[2] = Lora_Modem_Ascii_String_app_key[7];
					D1.s_ch[3] = Lora_Modem_Ascii_String_app_key[6];
					break;
				}
				case 5909:
				case 5910:
				{
					D1.s_ch[0] =Lora_Modem_Ascii_String_app_key[9];
					D1.s_ch[1] =Lora_Modem_Ascii_String_app_key[8];
					D1.s_ch[2] =Lora_Modem_Ascii_String_app_key[11];
					D1.s_ch[3] =Lora_Modem_Ascii_String_app_key[10];
					break;
				}
				case 5911:
				case 5912:
				{
					D1.s_ch[0] =Lora_Modem_Ascii_String_app_key[13];
					D1.s_ch[1] =Lora_Modem_Ascii_String_app_key[12];
					D1.s_ch[2] =Lora_Modem_Ascii_String_app_key[15];
					D1.s_ch[3] =Lora_Modem_Ascii_String_app_key[14];
					break;
				}
				case 5913:
				case 5914:
				{
					D1.s_ch[0] =Lora_Modem_Ascii_String_dev_eui[1];
					D1.s_ch[1] =Lora_Modem_Ascii_String_dev_eui[0];
					D1.s_ch[2] =Lora_Modem_Ascii_String_dev_eui[3];
					D1.s_ch[3] =Lora_Modem_Ascii_String_dev_eui[2];
					break;
				}
				case 5915:
				case 5916:
				{
					D1.s_ch[0] = Lora_Modem_Ascii_String_dev_eui[5];
					D1.s_ch[1] = Lora_Modem_Ascii_String_dev_eui[4];
					D1.s_ch[2] = Lora_Modem_Ascii_String_dev_eui[7];
					D1.s_ch[3] = Lora_Modem_Ascii_String_dev_eui[6];
					break;
				}
			}
		}
    	else
    	{
    		//sprintf((char *)print,"Modbus_task:Address=%d is out of range\r\n",i);
    		//WriteLog(1, print, 1);
    		D1.fl = 0;
    	}

    	modH->u8Buffer[ modH->u8BufferSize ] = D1.s_ch[1];
		modH->u8BufferSize++;
		modH->u8Buffer[ modH->u8BufferSize ] = D1.s_ch[0];
		modH->u8BufferSize++;
		modH->u8Buffer[ modH->u8BufferSize ] = D1.s_ch[3];
		modH->u8BufferSize++;
		modH->u8Buffer[ modH->u8BufferSize ] = D1.s_ch[2];
		modH->u8BufferSize++;
    }
    u8CopyBufferSize = modH->u8BufferSize +2;
    sendTxBuffer(modH);

    //DECLINE:
    return u8CopyBufferSize;
}
/**************************************************************************//**
 * Function name 	: getFloatValueAddress
 * arguments		: 1) registerAddress : uint16_t registerAddress
 * 		 		 	: 2)
 * return			: float value
 * Note				: This method is used to read the float values from the Modbus address
 * 					: This method returns the float value from the Modbus address
 *****************************************************************************/
float getFloatValueAddress(uint16_t registerAddress)
{
	unsigned char channelIndex = 0;
	switch (registerAddress)
	{
	case MODBUS_FLOAT_Lora_Frequency:return EPROM_General.Lora_Frequency;
	case MODBUS_FLOAT_Lora_Spreading_Factor:return EPROM_General.Lora_Spreading_Factor;
	case MODBUS_FLOAT_Lora_Bandwidth:return EPROM_General.Lora_Bandwidth;
	case MODBUS_FLOAT_Lora_Code_Rate  :return EPROM_General.Lora_Code_Rate;
	case MODBUS_FLOAT_Lora_Preamble_Length:return EPROM_General.Lora_Preamble_Length;
	case MODBUS_FLOAT_Lora_TX_Power :return EPROM_General.Lora_TX_Power;
	case MODBUS_FLOAT_Lora_P2P :return EPROM_General.Lora_p2p;
	case MODBUS_FLOAT_Modem_EC200_presence :return EPROM_General.Modem_EC200_presence;
		 default:
			 return 0;
	}
	return 0;
}
/**************************************************************************//**
 * Function name 	: process_FC5
 * arguments		: 1) modH : ModbusPort Handler
 * 		 			:
 * return			: u8BufferSize Response to master length
 * Note				: This method processes functions 5
 * 					: This method writes a value assigned by the master to a single bit
 *****************************************************************************/

int8_t process_FC5( modbusHandler_t *modH )
{
    //uint8_t u8currentBit;
    uint16_t u16currentRegister;
    uint16_t u8CopyBufferSize;
    uint16_t u16coil = word( modH->u8RxBuffer[ ADD_HI ], modH->u8RxBuffer[ ADD_LO ] );
    uint16_t u16val = word( modH->u8RxBuffer[ NB_HI ], modH->u8RxBuffer[ NB_LO ] );

    modH->u8Buffer[ ID ]       = modH->u8RxBuffer[ ID ];
    modH->u8Buffer[ FUNC ]     = modH->u8RxBuffer[ FUNC ];
    // point to the register and its bit
    //u16currentRegister = (u16coil / 16);
    //u8currentBit = (uint8_t) (u16coil % 16);
    u16currentRegister = u16coil;

    // write to coil
    //bitWrite(
    //	modH->u16regs[ u16currentRegister ],
    //    u8currentBit,
	//	modH->u8Buffer[ NB_HI ] == 0xff );

    u16currentRegister++;

    if( (u16currentRegister >= 67) && (u16currentRegister <= (u16currentRegister + gpcbplcCnfg.mMaxDoEnabled)) )
	{
    	if(u16val == 0xFF00)
		{
			//outputDOflag[u16currentRegister-67] = 1;
			EPROM_General.DoModeDetails.DO_Value[u16currentRegister-67] = 1;
		}
		else
		{
			//outputDOflag[u16currentRegister-67] = 0;
			EPROM_General.DoModeDetails.DO_Value[u16currentRegister-67] = 0;
		}
	    flag_flashUpdateEPROM_General = 1;
	    flag_flashUpdateEPROM_General_WaitCounter=5;
	}

    // send answer to master
    modH->u8BufferSize = 6;
    u8CopyBufferSize =  modH->u8BufferSize +2;
    sendTxBuffer(modH);

    return u8CopyBufferSize;
}

/**************************************************************************//**
 * Function name 	: process_FC6
 * arguments		: 1) modH : ModbusPort Handler
 * 		 			:
 * return			: u8BufferSize Response to master length
 * Note				: This method processes functions 6
 * 					: This method writes a value assigned by the master to a single word
 *****************************************************************************/

int8_t process_FC6(modbusHandler_t *modH )
{

    uint16_t u16add = word( modH->u8RxBuffer[ ADD_HI ], modH->u8RxBuffer[ ADD_LO ] );
    uint16_t u8CopyBufferSize;
    uint16_t k;
    union_Datatypes D;
    unsigned char Lora_Modem_Ascii_modbus_reveived_data[10];
    uint16_t u16val = word( modH->u8RxBuffer[ NB_HI ], modH->u8RxBuffer[ NB_LO ] );
    //uint16_t u16crc = word( modH->u8RxBuffer[ CRC_HI ], modH->u8RxBuffer[ CRC_LO ] );

    modH->u8Buffer[ID]     = modH->u8RxBuffer[ ID ];
    modH->u8Buffer[FUNC]   = modH->u8RxBuffer[ FUNC ];
    modH->u8Buffer[ADD_HI] = (u16add >> 8) & 0x00FF;
    modH->u8Buffer[ADD_HI] = (u16add & 0x00FF);
    modH->u8Buffer[NB_HI]  = (u16val >> 8) & 0x00FF;
    modH->u8Buffer[NB_LO]  = (u16val & 0x00FF);
    modH->u8Buffer[CRC_HI] = (u16val >> 8) & 0x00FF; //@todo: calculate CRC
    modH->u8Buffer[CRC_LO] = (u16val & 0x00FF); //@todo: calculate CRC

    // keep the same header
    modH->u8BufferSize = RESPONSE_SIZE;

	D.ch[0] = modH->u8RxBuffer[4];
	D.ch[1] = modH->u8RxBuffer[4+1];

	k = u16add;
	switch(k+1)
	{
		case 5901:
		{
			Lora_Modem_Ascii_String_app_eui[0]=D.ch[0];
			Lora_Modem_Ascii_String_app_eui[1]=D.ch[1];

			asciiStringToHexString(Lora_Modem_Ascii_String_app_eui, Lora_Modem_Ascii_modbus_reveived_data, 2);
			memcpy(&(EPROM_LoRa_Modem.lora_app_eui_set[0]), Lora_Modem_Ascii_modbus_reveived_data, 4);
			flag_flashUpdateEPROM_LORA_WaitCounter = 5;
			flag_flashUpdateEPROM_LORA = 1;
			break;
		}
		case 5902:
		{
			Lora_Modem_Ascii_String_app_eui[2]=D.ch[0];
			Lora_Modem_Ascii_String_app_eui[3]=D.ch[1];

			asciiStringToHexString(&(Lora_Modem_Ascii_String_app_eui[2]), Lora_Modem_Ascii_modbus_reveived_data, 2);
			memcpy( &(EPROM_LoRa_Modem.lora_app_eui_set[4]), Lora_Modem_Ascii_modbus_reveived_data, 4);
			flag_flashUpdateEPROM_LORA_WaitCounter = 5;
			flag_flashUpdateEPROM_LORA = 1;
			break;
		}
		case 5903:
		{
			Lora_Modem_Ascii_String_app_eui[4]=D.ch[0];
			Lora_Modem_Ascii_String_app_eui[5]=D.ch[1];

			asciiStringToHexString(&(Lora_Modem_Ascii_String_app_eui[4]), Lora_Modem_Ascii_modbus_reveived_data, 2);
			memcpy(&(EPROM_LoRa_Modem.lora_app_eui_set[8]), Lora_Modem_Ascii_modbus_reveived_data, 4);
			flag_flashUpdateEPROM_LORA_WaitCounter = 5;
			flag_flashUpdateEPROM_LORA = 1;
			break;
		}
		case 5904:
		{
			Lora_Modem_Ascii_String_app_eui[6]=D.ch[0];
			Lora_Modem_Ascii_String_app_eui[7]=D.ch[1];

			asciiStringToHexString(&(Lora_Modem_Ascii_String_app_eui[6]), Lora_Modem_Ascii_modbus_reveived_data, 2);
			memcpy(&(EPROM_LoRa_Modem.lora_app_eui_set[12]), Lora_Modem_Ascii_modbus_reveived_data, 4);
			EPROM_LoRa_Modem.lora_app_eui_set[16] = '\0';
			flag_flashUpdateEPROM_LORA_WaitCounter = 5;
			flag_flashUpdateEPROM_LORA = 1;
			break;
		}
		case 5905:
		{
			Lora_Modem_Ascii_String_app_key[0]=D.ch[0];
			Lora_Modem_Ascii_String_app_key[1]=D.ch[1];

			asciiStringToHexString(&(Lora_Modem_Ascii_String_app_key[0]), Lora_Modem_Ascii_modbus_reveived_data, 2);
			memcpy(&(EPROM_LoRa_Modem.lora_app_key_set[0]), Lora_Modem_Ascii_modbus_reveived_data, 4);
			flag_flashUpdateEPROM_LORA_WaitCounter = 5;
			flag_flashUpdateEPROM_LORA = 1;
			break;
		}
		case 5906:
		{
			Lora_Modem_Ascii_String_app_key[2]=D.ch[0];
			Lora_Modem_Ascii_String_app_key[3]=D.ch[1];

			asciiStringToHexString(&(Lora_Modem_Ascii_String_app_key[2]), Lora_Modem_Ascii_modbus_reveived_data, 2);
			memcpy(&(EPROM_LoRa_Modem.lora_app_key_set[4]), Lora_Modem_Ascii_modbus_reveived_data, 4);
			flag_flashUpdateEPROM_LORA_WaitCounter = 5;
			flag_flashUpdateEPROM_LORA = 1;
			break;
		}
		case 5907:
		{
			Lora_Modem_Ascii_String_app_key[4]=D.ch[0];
			Lora_Modem_Ascii_String_app_key[5]=D.ch[1];

			asciiStringToHexString(&(Lora_Modem_Ascii_String_app_key[4]), Lora_Modem_Ascii_modbus_reveived_data, 2);
			memcpy(&(EPROM_LoRa_Modem.lora_app_key_set[8]), Lora_Modem_Ascii_modbus_reveived_data, 4);
			flag_flashUpdateEPROM_LORA_WaitCounter = 5;
			flag_flashUpdateEPROM_LORA = 1;
			break;
		}
		case 5908:
		{
			Lora_Modem_Ascii_String_app_key[6]=D.ch[0];
			Lora_Modem_Ascii_String_app_key[7]=D.ch[1];

			asciiStringToHexString(&(Lora_Modem_Ascii_String_app_key[6]), Lora_Modem_Ascii_modbus_reveived_data, 2);
			memcpy(&(EPROM_LoRa_Modem.lora_app_key_set[12]), Lora_Modem_Ascii_modbus_reveived_data, 4);
			flag_flashUpdateEPROM_LORA_WaitCounter = 5;
			flag_flashUpdateEPROM_LORA = 1;
			break;
		}
		case 5909:
		{
			Lora_Modem_Ascii_String_app_key[8]=D.ch[0];
			Lora_Modem_Ascii_String_app_key[9]=D.ch[1];

			asciiStringToHexString(&(Lora_Modem_Ascii_String_app_key[8]), Lora_Modem_Ascii_modbus_reveived_data, 2);
			memcpy(&(EPROM_LoRa_Modem.lora_app_key_set[16]), Lora_Modem_Ascii_modbus_reveived_data, 4);
			flag_flashUpdateEPROM_LORA_WaitCounter = 5;
			flag_flashUpdateEPROM_LORA = 1;
			break;
		}
		case 5910:
		{
			Lora_Modem_Ascii_String_app_key[10]=D.ch[0];
			Lora_Modem_Ascii_String_app_key[11]=D.ch[1];

			asciiStringToHexString(&(Lora_Modem_Ascii_String_app_key[10]), Lora_Modem_Ascii_modbus_reveived_data, 2);
			memcpy(&(EPROM_LoRa_Modem.lora_app_key_set[20]), Lora_Modem_Ascii_modbus_reveived_data, 4);
			flag_flashUpdateEPROM_LORA_WaitCounter = 5;
			flag_flashUpdateEPROM_LORA = 1;
			break;
		}
		case 5911:
		{
			Lora_Modem_Ascii_String_app_key[12]=D.ch[0];
			Lora_Modem_Ascii_String_app_key[13]=D.ch[1];

			asciiStringToHexString(&(Lora_Modem_Ascii_String_app_key[12]), Lora_Modem_Ascii_modbus_reveived_data, 2);
			memcpy(&(EPROM_LoRa_Modem.lora_app_key_set[24]), Lora_Modem_Ascii_modbus_reveived_data, 4);
			flag_flashUpdateEPROM_LORA_WaitCounter = 5;
			flag_flashUpdateEPROM_LORA = 1;
			break;
		}
		case 5912:
		{
			Lora_Modem_Ascii_String_app_key[14]=D.ch[0];
			Lora_Modem_Ascii_String_app_key[15]=D.ch[1];

			asciiStringToHexString(&(Lora_Modem_Ascii_String_app_key[14]), Lora_Modem_Ascii_modbus_reveived_data, 2);
			memcpy(&(EPROM_LoRa_Modem.lora_app_key_set[28]), Lora_Modem_Ascii_modbus_reveived_data, 4);
			EPROM_LoRa_Modem.lora_app_key_set[32] = '\0';
			flag_flashUpdateEPROM_LORA_WaitCounter = 5;
			flag_flashUpdateEPROM_LORA = 1;
			break;
		}
		default:
			break;
	}

    u8CopyBufferSize = modH->u8BufferSize + 2;
    sendTxBuffer(modH);

#if 0
    int i;
    //int ValidString=0;
    //unsigned char FrameValidFlag;
    //short int RegAddr,CRC,ByteCount;
    int RegOffSet,NoOfRegs,j,k;
    long timestamp,timestamp1;



    /** @todo: CRC validation is pending */
    //FrameValidFlag = CheckFrameForCRC(Msg,NoOfBytesInFeame-2,CRC);


   // {
    	if(EPROM.MODBUS_Disp==MODBUS_LONG)
    	{
    		D.ch[0] = Msg[4+i];
			D.ch[1] = Msg[4+i+1];
			D.ch[2] = Msg[4+i+2];
			D.ch[3] = Msg[4+i+3];
    	}
    	else if(EPROM.MODBUS_Disp==MODBUS_FLOAT)
		{
    		D.ch[1] = Msg[7+i];
			D.ch[0] = Msg[7+i+1];
			D.ch[3] = Msg[7+i+2];
			D.ch[2] = Msg[7+i+3];
		}
    	else
		{
    		D.ch[3] = Msg[7+i];
			D.ch[2] = Msg[7+i+1];
			D.ch[1] = Msg[7+i+2];
			D.ch[0] = Msg[7+i+3];
		}

		FinalAnaValF[561] = 6;
		FinalAnaValF[562] = 1;
		FinalAnaValF[571] = RegOffSet;
		FinalAnaValF[581] =	D.fl;

		Val[j] = D.fl;

		k = u16add;
		switch(k)
		{
			#if 0
			case 84:
			{
				FinalAnaValF[k/2] = Val[j];
				timestamp=(long)FinalAnaValF[k/2];
				break;
			}
			case 86:
			{
				FinalAnaValF[k/2] = Val[j];
				timestamp1=(long)FinalAnaValF[k/2];
				if(timestamp==12345 && timestamp1==67890)
				{
				   EPROM.RTU_BootFlag=LANBOOT;
				   SaveTOEprom=1;
				   SaveToDisk=1;Restart_Device=1;
				}
				break;
			}
			case 80:
			{
				FinalAnaValF[k/2] = Val[j];
				timestamp=(long)FinalAnaValF[k/2];
				sprintf(dispStr,"\r\nSDATEUpdate %ul %f ",timestamp,FinalAnaValF[k/2]);
				fnDebugMsg(dispStr);
				RDate.Date = (unsigned char)(timestamp/10000);	//day of month
				RDate.Month =(unsigned char)((timestamp%10000)/100);	//month
				RDate.Year = (unsigned char)((timestamp%10000)%100);		// year
				if((RDate.Year>0 && RDate.Year<=99)  && (RDate.Month>=1 && RDate.Month<=12) && (RDate.Date>=1  && RDate.Date<=31) )
				{
					updateTCPIPTime=1;
				}
				break;
			}
			case 82:
			{
				FinalAnaValF[k/2] = Val[j];
				NoofCnt=(int)FinalAnaValF[k/2];
				break;
			}
			case 70:
			{
				FinalAnaValF[k/2] = Val[j];
				EPROM.AOValue[0]=Val[j];
				Analog_output(Val[j],1);
				SaveTOEprom=1;
				break;
			}
			case 72:
			{
				FinalAnaValF[k/2] = Val[j];
				EPROM.AOValue[1]=Val[j];
				Analog_output(Val[j],2);
				SaveTOEprom=1;
				break;
			}
			case 74:
			{
				FinalAnaValF[k/2] = Val[j];
				EPROM.AOValue[2]=Val[j];
				Analog_output(Val[j],3);
				SaveTOEprom=1;
				break;
			}
			case 76:
			{
				FinalAnaValF[k/2] = Val[j];
				EPROM.AOValue[3]=Val[j];
				Analog_output(Val[j],4);
				SaveTOEprom=1;
				break;
			}
			#endif
			default:
			{
				if((k >= 1800) && (k <= 6000) && (k%2) == 0)
				{
					ControlLogic(k/2,Val[j]);
				}
				else if((k >= 803) && (k <= 1001))
				{
					ControlLogic(k/2,Val[j]);
				}
				else
				{
					//RTU_para_WriteMod(k/2,Val[j]);
				}
				break;
			}
		}

		//if(k/2>=(785+220+(MAXSCH*3)) && (k/2)<=(785+220+(MAXSCH*3)+MAX3PARA))
    	//{
    	//}
		//else
		//{
		//	InsertIntoFlashBuff(__LINE__);;
    	//	SaveTOEprom=1;
		//}

		RS[0] = EPROM.Address;
		RS[1] = Msg[1];
		RS[2] =	Msg[2];
		RS[3] = Msg[3];
		RS[4] = Msg[4];
		RS[5] = Msg[5];
		D.sh = CRC1(RS,6);
		RS[6] = D.ch[1];
		RS[7] = D.ch[0];
		RS[8] = '\0';
		WriteMODstring(RS,8);
#endif
		return u8CopyBufferSize;
}

/**************************************************************************//**
 * Function name 	: process_FC15
 * arguments		: 1) modH : ModbusPort Handler
 * 		 			:
 * return			: u8BufferSize Response to master length
 * Note				: This method processes functions 15
 * 					: This method writes a bit array assigned by the master
 *****************************************************************************/

int8_t process_FC15( modbusHandler_t *modH )
{
    uint8_t u8currentBit, u8frameByte, u8bitsno;
    uint16_t u16currentRegister;
    uint16_t u8CopyBufferSize;
    uint16_t u16currentCoil, u16coil;
    uint8_t bTemp;

    // get the first and last coil from the message
    uint16_t u16StartCoil = word( modH->u8RxBuffer[ ADD_HI ], modH->u8RxBuffer[ ADD_LO ] );
    uint16_t u16Coilno = word( modH->u8RxBuffer[ NB_HI ], modH->u8RxBuffer[ NB_LO ] );


    modH->u8Buffer[ ID ]       = modH->u8RxBuffer[ ID ];
    modH->u8Buffer[ FUNC ]     = modH->u8RxBuffer[ FUNC ];
    // read each coil from the register map and put its value inside the outcoming message
    u8bitsno = 0;
    u8frameByte = 7;
    for (u16currentCoil = 0; u16currentCoil < u16Coilno; u16currentCoil++)
    {

        u16coil = u16StartCoil + u16currentCoil;
        u16currentRegister = (u16coil / 16);
        u8currentBit = (uint8_t) (u16coil % 16);

        bTemp = bitRead(
        			modH->u8Buffer[ u8frameByte ],
                    u8bitsno );

        bitWrite(
            modH->u16regs[ u16currentRegister ],
            u8currentBit,
            bTemp );

        u8bitsno ++;

        if (u8bitsno > 7)
        {
            u8bitsno = 0;
            u8frameByte++;
        }
    }

    // send outcoming message
    // it's just a copy of the incomping frame until 6th byte
    modH->u8BufferSize         = 6;
    u8CopyBufferSize = modH->u8BufferSize +2;
    sendTxBuffer(modH);
    return u8CopyBufferSize;
}

/**************************************************************************//**
 * Function name 	: process_FC16
 * arguments		: 1) modH : ModbusPort Handler
 * 		 			:
 * return			: u8BufferSize Response to master length
 * Note				: This method processes functions 16
 * 					: This method writes a word array assigned by the master
 *****************************************************************************/

int8_t process_FC16(modbusHandler_t *modH )
{
    uint16_t u16StartAdd = modH->u8RxBuffer[ ADD_HI ] << 8 | modH->u8RxBuffer[ ADD_LO ];
    uint16_t u16regsno = modH->u8RxBuffer[ NB_HI ] << 8 | modH->u8RxBuffer[ NB_LO ];
    uint16_t u8CopyBufferSize = 0;
    uint16_t i, j, k;
    //uint16_t temp;
    union_Datatypes D;
    float tValue = 0;
   if(ModbusH[5].u8RxBuffer[4]==0 && ModbusH[5].u8RxBuffer[5]==12)
   {
	   RTCsync_flage=1;
   }
    // build header
    modH->u8Buffer[ID]      = modH->u8RxBuffer[ ID ];
    modH->u8Buffer[FUNC]    = modH->u8RxBuffer[ FUNC ];
    modH->u8Buffer[ADD_HI]  = (u16StartAdd >> 8) & 0x00FF;
    modH->u8Buffer[ADD_LO]  = (u16StartAdd) & 0x00FF;
    modH->u8Buffer[NB_HI]   = (u16regsno >> 8) & 0x00FF;
    modH->u8Buffer[NB_LO]   = (u16regsno) & 0x00FF;
    //modH->u8Buffer[CRC_HI]  = 0;
    //modH->u8Buffer[CRC_LO]  = (uint8_t) u16regsno; // answer is always 256 or less bytes
    modH->u8BufferSize         = RESPONSE_SIZE;

    // write registers
//    for (i = 0; i < u16regsno; i++)
//    {
//    	temp = word(
//        		modH->u8Buffer[ (BYTE_CNT + 1) + i * 2 ],
//				modH->u8Buffer[ (BYTE_CNT + 2) + i * 2 ]);
//
//        modH->u16regs[u16StartAdd + i ] = temp;
//    }

    if(u16regsno <= 120)
    {
    	//FinalAnaValF[561]=16;
    	//FinalAnaValF[562]=u16regsno/2;
    	//FinalAnaValF[571]=u16StartAdd;
    	//k++;
		u16StartAdd++;
    	for(i = 0,j = 0, k = (u16StartAdd) ; j < (u16regsno/2) ; i = i+4,j++,k = k+2)
    	{
//    		if(EPROM.MODBUS_Disp==MODBUS_LONG)
//    		{
//    			D.ch[0] = modH->u8RxBuffer[7+i];
//    			D.ch[1] = modH->u8RxBuffer[7+i+1];
//    			D.ch[2] = modH->u8RxBuffer[7+i+2];
//    			D.ch[3] = modH->u8RxBuffer[7+i+3];
//    		}
//    		else if(EPROM.MODBUS_Disp==MODBUS_FLOAT)
//    		{
    			D.ch[1] = modH->u8RxBuffer[7+i];
    			D.ch[0] = modH->u8RxBuffer[7+i+1];
    			D.ch[3] = modH->u8RxBuffer[7+i+2];
    			D.ch[2] = modH->u8RxBuffer[7+i+3];
//    		}
//    		else
//    		{
//				D.ch[3] = modH->u8RxBuffer[7+i];
//    			D.ch[2] = modH->u8RxBuffer[7+i+1];
//    			D.ch[1] = modH->u8RxBuffer[7+i+2];
//    			D.ch[0] = modH->u8RxBuffer[7+i+3];
//    		}

    		//FinalAnaValF[581+j] = D.fl;
    		tValue = D.fl;

    		//k++;
    		switch(k)
    		{
				#if 0
    			case 84:
    			{
    				FinalAnaValF[k/2] = tValue;
    				timestamp=(long)FinalAnaValF[k/2];
    				break;
    			}
    			case 86:
    			{
    				FinalAnaValF[k/2] = tValue;
    				timestamp1=(long)FinalAnaValF[k/2];

    				if(timestamp==12345 && timestamp1==67890)
    				{
					   EPROM.RTU_BootFlag=LANBOOT;
					   SaveTOEprom=1;
					   SaveToDisk=1;Restart_Device=1;
    				}
    				break;
    			}
    			case 80:
    			{
					FinalAnaValF[k/2] = tValue;
					timestamp=(long)FinalAnaValF[k/2];
					RDate.Date = (unsigned char)(timestamp/10000);	//day of month
					RDate.Month =(unsigned char)((timestamp%10000)/100);	//month
					RDate.Year = (unsigned char)((timestamp%10000)%100);		// year
					if((RDate.Year>0 && RDate.Year<=99)  && (RDate.Month>=1 && RDate.Month<=12) && (RDate.Date>=1  && RDate.Date<=31) )
					{
						updateTCPIPTime=1;
					}
					break;
    			}
    			case 82:
    			{
					FinalAnaValF[k/2] = tValue;
					timestamp=(long)FinalAnaValF[k/2];
					RTime.Hour = (unsigned char)(timestamp/10000);	//RTime.Hour
					RTime.Min = (unsigned char)((timestamp%10000)/100);	//Minute 	   84600
					RTime.Sec = (unsigned char)((timestamp%10000)%100);	// Second
					if((RTime.Hour>=0 && RTime.Hour<=23)  && (RTime.Min>=0 && RTime.Min<=59) && (RTime.Sec>=0  && RTime.Sec<=59) )
					{
						if(updateTCPIPTime==1)
						{
							updateTCPIPTime=3;
						}
						else
						{
							updateTCPIPTime=2;
						}
					}
					break;
    			}
    			case 70:
    			{
					FinalAnaValF[k/2] = tValue;
					EPROM.AOValue[0]=tValue;
					Analog_output(tValue,1);SaveTOEprom=1;
					break;
    			}
    			case 72:
    			{
					FinalAnaValF[k/2] = tValue;
					EPROM.AOValue[1]=tValue;
					Analog_output(tValue,2);SaveTOEprom=1;
					break;
    			}
				case 74:
				{
					FinalAnaValF[k/2] = tValue;
					EPROM.AOValue[2]=tValue;
					Analog_output(tValue,3);SaveTOEprom=1;
					break;
				}
    			case 76:
    			{
					FinalAnaValF[k/2] = tValue;
					EPROM.AOValue[3]=tValue;
					Analog_output(tValue,4);SaveTOEprom=1;
					break;
    			}
				#endif
    			case 103: //Date
    			{
    				struct tm t, t1;
    				gFinalAnaValF[k/2] = tValue;
//    				if(RTCsync_flage==1)
//    				{
//							if((tValue>0)&&(tValue<31))
//							{
//								t.tm_mday = tValue;//bcdToDec(tValue);
//							}
//							else
//							{
//								t.tm_mday = gTimeInfo.mDate;
//							}
//    				}
//    				else
//    				{
							t.tm_hour = gTimeInfo.mHour;//bcdToDec(gTimeInfo.mHour);
							t.tm_min = gTimeInfo.mMinute;//bcdToDec(gTimeInfo.mMinute);
							t.tm_sec = gTimeInfo.mSecond;//bcdToDec(gTimeInfo.mSecond);
							//t.tm_wday = (gTimeInfo.WeekDay) - 1;
							if((tValue>0)&&(tValue<=31))
							{
								t.tm_mday = tValue;//bcdToDec(tValue);
							}
							else
							{
								t.tm_mday = gTimeInfo.mDate;
							}
							t.tm_mon = gTimeInfo.mMonth-1;//bcdToDec(gTimeInfo.mMonth) - 1;
							t.tm_year = gTimeInfo.mYear + 2000 - 1900;//bcdToDec(gTimeInfo.mYear) + 2000 - 1900;

							UTC = (uint64_t)mktime_new(&t);
    		//		}
    				set_time(t1);
    				break;
    			}
    			case 105: //month
				{
					struct tm t, t1;
					gFinalAnaValF[k/2] = tValue;
					if(RTCsync_flage==1)
					{
//						if((tValue>1)&&(tValue<14))
						if((tValue>0)&&(tValue<13))     //  An add for 1 to 12 month set
						{
							t.tm_mon = tValue-1;
						}
						else
						{
							t.tm_mon = gTimeInfo.mMonth-1;
						}
					}
					else
					{
						t.tm_hour = gTimeInfo.mHour;//bcdToDec(gTimeInfo.mHour);
						t.tm_min = gTimeInfo.mMinute;//bcdToDec(gTimeInfo.mMinute);
						t.tm_sec = gTimeInfo.mSecond;//bcdToDec(gTimeInfo.mSecond);
						//t.tm_wday = (gTimeInfo.WeekDay) - 1;
						if((tValue>0)&&(tValue<13))
						{
							t.tm_mon = tValue-1;
						}
						else
						{
							t.tm_mon = gTimeInfo.mMonth-1;
						}
						t.tm_mday = gTimeInfo.mDate;//bcdToDec(gTimeInfo.mDate);
						t.tm_year = gTimeInfo.mYear + 2000 - 1900;//bcdToDec(gTimeInfo.mYear) + 2000 - 1900;
					}
 						UTC = (uint64_t)mktime_new(&t);

					set_time(t1);
					break;
				}
    			case 107://year
				{
					struct tm t, t1;
					gFinalAnaValF[k/2] = tValue;
					if(RTCsync_flage==1)
					{
						if((tValue>2000)&&(tValue<2100))
						{
							t.tm_year = tValue - 1900;
						}
						else
						{
							t.tm_year = (gTimeInfo.mYear) + 2000 - 1900;
						}
					}
					else
					{
						t.tm_hour = gTimeInfo.mHour;//bcdToDec(gTimeInfo.mHour);
						t.tm_min = gTimeInfo.mMinute;//bcdToDec(gTimeInfo.mMinute);
						t.tm_sec = gTimeInfo.mSecond;//bcdToDec(gTimeInfo.mSecond);
						//t.tm_wday = (gTimeInfo.WeekDay) - 1;
						t.tm_mon = gTimeInfo.mMonth-1;//bcdToDec(gTimeInfo.mMonth) - 1;
						t.tm_mday = gTimeInfo.mDate;//bcdToDec(gTimeInfo.mDate);
						if((tValue>2000)&&(tValue<2100))
						{
							t.tm_year = tValue - 1900;
						}
						else
						{
							t.tm_year = (gTimeInfo.mYear) + 2000 - 1900;
						}
					}
						UTC = (uint64_t)mktime_new(&t);

					set_time(t1);
					break;
				}
    			case 109://hour
				{
					struct tm t, t1;
					gFinalAnaValF[k/2] = tValue;
					if(RTCsync_flage==1)
					{
						if((tValue>=0)&&(tValue<24))
						{
						t.tm_hour = tValue;//bcdToDec(tValue);
						}
						else
						{
						t.tm_hour = gTimeInfo.mHour;
						}

					}
					else
					{
						if((tValue>=0)&&(tValue<24))
						{
							t.tm_hour = tValue;//bcdToDec(tValue);
						}
						else
						{
							t.tm_hour = gTimeInfo.mHour;
						}

						t.tm_min = gTimeInfo.mMinute;//bcdToDec(gTimeInfo.mMinute);
						t.tm_sec = gTimeInfo.mSecond;//bcdToDec(gTimeInfo.mSecond);
						//t.tm_wday = (gTimeInfo.WeekDay) - 1;
						t.tm_mon = gTimeInfo.mMonth-1;//bcdToDec(gTimeInfo.mMonth) - 1;
						t.tm_mday = (gTimeInfo.mDate);
						t.tm_year = (gTimeInfo.mYear) + 2000 - 1900;
					}
						UTC = (uint64_t)mktime_new(&t);

					set_time(t1);
					break;
				}
    			case 111://minute
				{
					struct tm t, t1;
					gFinalAnaValF[k/2] = tValue;
					if(RTCsync_flage==1)
					{
						if((tValue>=0)&&(tValue<60))
						{
						t.tm_min = (tValue);
						}
						else
						{
						t.tm_min = gTimeInfo.mMinute;
						}
					}
					else
					{
						t.tm_hour = (gTimeInfo.mHour);
						if((tValue>=0)&&(tValue<60))
						{
							t.tm_min = (tValue);
						}
						else
						{
							t.tm_min = gTimeInfo.mMinute;
						}

						t.tm_sec = (gTimeInfo.mSecond);
						//t.tm_wday = (gTimeInfo.WeekDay) - 1;
						t.tm_mon = (gTimeInfo.mMonth) - 1;
						t.tm_mday = (gTimeInfo.mDate);
						t.tm_year = (gTimeInfo.mYear) + 2000 - 1900;
					}
						UTC = (uint64_t)mktime_new(&t);

					set_time(t1);
					break;
				}
    			case 113://second
				{
					struct tm t, t1;
					gFinalAnaValF[k/2] = tValue;
					if(RTCsync_flage==1)
					{
						RTCsync_flage=0;
						memset(ModbusH[5].u8RxBuffer,0,300);
						if((tValue>=0)&&(tValue<60))
						{
						t.tm_sec = (tValue);
						}
						else
						{
						t.tm_sec = (gTimeInfo.mSecond);
						}
					}
					else
					{
						t.tm_hour = (gTimeInfo.mHour);
						t.tm_min = (gTimeInfo.mMinute);
						if((tValue>=0)&&(tValue<60))
						{
							t.tm_sec = (tValue);
						}
						else
						{
							t.tm_sec = (gTimeInfo.mSecond);
						}
						t.tm_sec = (tValue);
						//t.tm_wday = (gTimeInfo.WeekDay) - 1;
						t.tm_mon = (gTimeInfo.mMonth) - 1;
						t.tm_mday = (gTimeInfo.mDate);
						t.tm_year = (gTimeInfo.mYear) + 2000 - 1900;
					}
						UTC = (uint64_t)mktime_new(&t);

					set_time(t1);
					break;
				}
    			case MODBUS_FLOAT_Lora_Frequency:
				if ((tValue >= 0) && (tValue < 4294967294) )
				{
					EPROM_General.Lora_Frequency = (uint32_t)tValue;
					  ExtFlash_update_EPROM_General();
				}
				break;
    			case MODBUS_FLOAT_Lora_Spreading_Factor:
    				if ((tValue >= 0) && (tValue < 255) )
    				{
    					EPROM_General.Lora_Spreading_Factor = (uint32_t)tValue;
    					  ExtFlash_update_EPROM_General();
    				}
    				break;
    			case MODBUS_FLOAT_Lora_Bandwidth:
    				if ((tValue >= 0) && (tValue < 255) )
    				{
    					EPROM_General.Lora_Bandwidth = (uint32_t)tValue;
    					  ExtFlash_update_EPROM_General();
    				}
    				break;
    			case MODBUS_FLOAT_Lora_Code_Rate:
    				if ((tValue >= 0) && (tValue < 255) )
    				{
    					EPROM_General.Lora_Code_Rate = (uint32_t)tValue;
    					  ExtFlash_update_EPROM_General();
    				}
    				break;
    			case MODBUS_FLOAT_Lora_Preamble_Length:
    				if ((tValue >= 0) && (tValue < 65535) )
    				{
    					EPROM_General.Lora_Preamble_Length = (uint32_t)tValue;
    					  ExtFlash_update_EPROM_General();
    				}
    				break;
    			case MODBUS_FLOAT_Lora_TX_Power:
    				if ((tValue >= 0) && (tValue < 255) )
    				{
    					EPROM_General.Lora_TX_Power = (uint32_t)tValue;
    					  ExtFlash_update_EPROM_General();
    				}
    				break;
    			case MODBUS_FLOAT_Lora_P2P:
				if ((tValue >= 0) && (tValue < 255) )
				{
					EPROM_General.Lora_p2p = (uint32_t)tValue;
					  ExtFlash_update_EPROM_General();
				}
    			case MODBUS_FLOAT_Modem_EC200_presence:
				if ((tValue >= 0) && (tValue < 255) )
				{
					EPROM_General.Modem_EC200_presence = (uint32_t)tValue;
					  ExtFlash_update_EPROM_General();
				}
    				break;
    			case 5901:
    			case 5902:
				{
					Lora_Modem_Ascii_String_app_eui[0]=D.ch[0];
					Lora_Modem_Ascii_String_app_eui[1]=D.ch[1];
					Lora_Modem_Ascii_String_app_eui[2]=D.ch[2];
					Lora_Modem_Ascii_String_app_eui[3]=D.ch[3];
					asciiStringToHexString(Lora_Modem_Ascii_String_app_key, EPROM_LoRa_Modem.lora_app_eui_set, 4);
					flag_flashUpdateEPROM_LORA_WaitCounter = 5;
					flag_flashUpdateEPROM_LORA = 1;
					break;
				}
    			case 5903:
    			case 5904:
				{
					Lora_Modem_Ascii_String_app_eui[4]=D.ch[0];
					Lora_Modem_Ascii_String_app_eui[5]=D.ch[1];
					Lora_Modem_Ascii_String_app_eui[6]=D.ch[2];
					Lora_Modem_Ascii_String_app_eui[7]=D.ch[3];
					asciiStringToHexString(Lora_Modem_Ascii_String_app_key, EPROM_LoRa_Modem.lora_app_eui_set, 4);
					flag_flashUpdateEPROM_LORA_WaitCounter = 5;
					flag_flashUpdateEPROM_LORA = 1;
					break;
				}
    			case 5905:
    			case 5906:
				{
					Lora_Modem_Ascii_String_app_key[0]=D.ch[0];
					Lora_Modem_Ascii_String_app_key[1]=D.ch[1];
					Lora_Modem_Ascii_String_app_key[3]=D.ch[2];
					Lora_Modem_Ascii_String_app_key[4]=D.ch[3];
					asciiStringToHexString(Lora_Modem_Ascii_String_app_key, EPROM_LoRa_Modem.lora_app_key_set, 4);
					flag_flashUpdateEPROM_LORA_WaitCounter = 5;
					flag_flashUpdateEPROM_LORA = 1;
					break;
				}
    			case 5907:
    			case 5908:
				{
					Lora_Modem_Ascii_String_app_key[5]=D.ch[0];
					Lora_Modem_Ascii_String_app_key[6]=D.ch[1];
					Lora_Modem_Ascii_String_app_key[7]=D.ch[2];
					Lora_Modem_Ascii_String_app_key[8]=D.ch[3];
					asciiStringToHexString(Lora_Modem_Ascii_String_app_key, EPROM_LoRa_Modem.lora_app_key_set, 4);
					flag_flashUpdateEPROM_LORA_WaitCounter = 5;
					flag_flashUpdateEPROM_LORA = 1;
					break;
				}
    			case 5909:
    			case 5910:
				{
					Lora_Modem_Ascii_String_app_key[9]=D.ch[0];
					Lora_Modem_Ascii_String_app_key[10]=D.ch[1];
					Lora_Modem_Ascii_String_app_key[11]=D.ch[2];
					Lora_Modem_Ascii_String_app_key[12]=D.ch[3];
					asciiStringToHexString(Lora_Modem_Ascii_String_app_key, EPROM_LoRa_Modem.lora_app_key_set, 4);
					flag_flashUpdateEPROM_LORA_WaitCounter = 5;
					flag_flashUpdateEPROM_LORA = 1;
					break;
				}
    			case 5911:
    			case 5912:
				{
					Lora_Modem_Ascii_String_app_key[13]=D.ch[0];
					Lora_Modem_Ascii_String_app_key[14]=D.ch[1];
					Lora_Modem_Ascii_String_app_key[15]=D.ch[2];
					Lora_Modem_Ascii_String_app_key[16]=D.ch[3];
					asciiStringToHexString(Lora_Modem_Ascii_String_app_key, EPROM_LoRa_Modem.lora_app_key_set, 4);
					flag_flashUpdateEPROM_LORA_WaitCounter = 5;
					flag_flashUpdateEPROM_LORA = 1;
					break;
				}
    			default:
    			{
    				if(k >= 1800 && k <= 9301 && (k%2) == 0)//if(k >= 1800 && k <= 9201 && (k%2) == 0)
					{
						ControlLogic (k/2, tValue);
					}
    				else if((k >= 1800) && (k <= 9301) && ((k%2) == 1))//else if((k >= 1800) && (k <= 9201) && ((k%2) == 1))
					{
						ControlLogic (k/2, tValue);
					}
    				else if((k >= 1201) && (k <= 1243) && ((k%2) == 1))
					{
						ControlLogic (k/2, tValue);
					}
    				else if((k >= 1361) && (k <= 1555) && ((k%2) == 1))
					{
						ControlLogic ((k-160)/2, tValue);
					}
    				else if(k >= 161 && k <= 759 && (k%2) == 1)
    				{
    					ControlLogic (START_IDX_GEN_ANA_PARA_TAG+((k-START_IDX_MODBUS_ANA_PARA)/2), tValue);
    				}
    				else if((k >= RECIPE_VAR_START_MODBUS_INDEX) &&
    						(k <= RECIPE_VAR_END_MODBUS_INDEX) &&
							((RECIPE_VAR_START_MODBUS_INDEX % 2) == 1))
					{
						ControlLogic(k/2, tValue);
					}
					else
					{
						//RTU_para_WriteMod(k/2,tValue);
					}
    				break;
    			}
    		}
    	}

//    	if((k/2)>=(785+220+(MAXSCH*3)) && (k/2)<=(785+220+(MAXSCH*3)+MAX3PARA))
//		{}
//		else
//		{	InsertIntoFlashBuff(__LINE__);;
//			SaveTOEprom=1;
//
//		}

    	u8CopyBufferSize = 8;
    	sendTxBuffer(modH);
    }


    return u8CopyBufferSize;
}

/**************************************************************************//**
 * Function name 	: get_FC1
 * arguments		: 1) modH : ModbusPort Handler
 * 		 			: modbus_t *telegram
 * return			:
 * Note				: This method processes functions 1 & 2 (for master)
 * 					: This method puts the slave answer into master data buffer
 *****************************************************************************/

void get_FC1(modbusHandler_t *modH,modbus_t *telegram )
{
    uint8_t  i,j,k;
     /*for (i=0; i< modH->u8Buffer[2]; i++) {

        if(i%2)
        {
        	modH->u16regs[i/2]= word(modH->u8Buffer[i+u8byte], lowByte(modH->u16regs[i/2]));
        }
        else
        {

        	modH->u16regs[i/2]= word(highByte(modH->u16regs[i/2]), modH->u8Buffer[i+u8byte]);
        }

     }*/

	modH->u8BufferSize = modH->u8RxBuffer[2]+3+2;
	telegram->u8rxdataValidation = validateAnswer(modH);
	if (telegram->u8rxdataValidation != 0)
	{

	}
	else
	{
		if(telegram->uDataType==DIGITAL_TYPE)
		{
			for(j = 3,i = 0 ; j < (modH->u8RxBuffer[2]+3) ; j++)
			{
				for(k = 0 ; k < 8 ; k++)
				{
					Digital_bit_Query_array[k] = ((modH->u8RxBuffer[j]>>k) & 0x01);
					//gFinalAnaValF[telegram->u16addressRegisterMap + i] = Digital_bit_Query_array[k];
					//FinalAnaValF[k]=Digital_bit_Query_array[k];
					dig_bit_array[telegram->u16addressRegisterMap + i] = Digital_bit_Query_array[k];
					i++;
					if(i >= telegram->u8noOfData)//Rx_data[3])
					{
						break;
					}
				}
			}
		}
	}
}

/**************************************************************************//**
 * Function name 	: get_FC3
 * arguments		: 1) modH : ModbusPort Handler
 * 		 			: 2) modbus_t *telegram
 * return			:
 * Note				: This method processes functions 3 & 4 (for master)
 * 					: This method puts the slave answer into master data buffer
 *****************************************************************************/

void get_FC3(modbusHandler_t *modH,modbus_t *telegram)
{
    uint16_t  i,j;
    float LocalFloat=0.0;
    union_Datatypes D1;
   /* for (i=0; i< modH->u8Buffer[ 2 ] /2; i++)
    {
    	modH->u16regs[ i ] = word(modH->u8Buffer[ u8byte ], modH->u8Buffer[ u8byte +1 ]);
        u8byte += 2;
    }*/

    	modH->u8BufferSize = modH->u8RxBuffer[2]+3+2;
    	telegram->u8rxdataValidation = validateAnswer(modH);
    	if (telegram->u8rxdataValidation != 0)
    	{

    	}
    	else
    	{
    	    for(j = 3,i = 0 ; j < (modH->u8RxBuffer[2]+3) ; i++)
    		{
    			if(telegram->uDataType==FLOAT_TYPE)				// Float - Flow Computer
    			{
    				D1.ch[1] =  modH->u8RxBuffer[j+0];
    				D1.ch[0] =  modH->u8RxBuffer[j+1];
    				D1.ch[3] =  modH->u8RxBuffer[j+2];
    				D1.ch[2] =  modH->u8RxBuffer[j+3];
    				LocalFloat = D1.fl;
    				j += 4;
    			}
    			else if(telegram->uDataType==SWFLOAT_TYPE)					//Swapped Float - Flow Computer
    			{
    				D1.ch[3] =  modH->u8RxBuffer[j+0];
    				D1.ch[2] =  modH->u8RxBuffer[j+1];
    				D1.ch[1] =  modH->u8RxBuffer[j+2];
    				D1.ch[0] =  modH->u8RxBuffer[j+3];
    				LocalFloat = D1.fl;
    				j += 4;
    			}
    			else if(telegram->uDataType==INTEGER_TYPE || telegram->uDataType == UNSIGNED32_TYPE)					//Interger-32 bits - Flow Computer
    			{
    				D1.ch[0] =  modH->u8RxBuffer[j+3];
    				D1.ch[1] =  modH->u8RxBuffer[j+2];
    				D1.ch[2] =  modH->u8RxBuffer[j+1];
    				D1.ch[3] =  modH->u8RxBuffer[j+0];
    				LocalFloat = D1.uint;
    				j += 4;
    			}
    			else if(telegram->uDataType == SIGNED32_TYPE)					//Interger-32 bits - Flow Computer
    			{
    				D1.ch[0] =  modH->u8RxBuffer[j+3];
    				D1.ch[1] =  modH->u8RxBuffer[j+2];
    				D1.ch[2] =  modH->u8RxBuffer[j+1];
    				D1.ch[3] =  modH->u8RxBuffer[j+0];
    				LocalFloat = D1.int_32;
    				j += 4;
    			}
    			else if(telegram->uDataType == UNSIGNED16_TYPE)			//Interger-16 bits - Flow Computer
    			{
    				D1.ch[0] =  modH->u8RxBuffer[j+1];
    				D1.ch[1] =  modH->u8RxBuffer[j+0];
    				LocalFloat = D1.sh;
    				j += 2;
    			}
    			else if(telegram->uDataType == SIGNED16_TYPE)			//Interger-16 bits - Flow Computer
    			{
    				D1.ch[0] =  modH->u8RxBuffer[j+1];
    				D1.ch[1] =  modH->u8RxBuffer[j+0];
    				LocalFloat = D1.sh_16;
    				j += 2;
    			}
    			else if(telegram->uDataType == UNSIGN_CHAR_STRING_TYPE)			//Interger-16 bits - Flow Computer
    			{
    						D1.ch[0] =  modH->u8RxBuffer[j];
    						//D1.ch[1] =  modH->u8RxBuffer[j+0];
    						LocalFloat = D1.ch[0];
    						j += 1;
    			}
    			else if(telegram->uDataType == CHAR_STRING_TYPE)			//Interger-16 bits - Flow Computer
    			{
    						D1.s_ch[0] =  modH->u8RxBuffer[j];
    						//D1.ch[1] =  modH->u8RxBuffer[j+0];
    						LocalFloat = D1.s_ch[0];
    						j += 1;
    			}
    			else
    			{
    				break;
    			}
    			if(i < telegram->u8noOfData)  // RS port hand if above 255 value ticket : https://cimcondigital.atlassian.net/browse/IRTU6000PP-20
    			{
        			gFinalAnaValF[telegram->u16addressRegisterMap+i] = LocalFloat;
    			}
    			else
    			{
    				break;
    			}
    		}
    	}
}
/**************************************************************************//**
 * Function name 	: get_FC5_FC6
 * arguments		: 1) modH : ModbusPort Handler
 * 		 			: 2) modbus_t *telegram
 * return			:
 * Note				: This method processes functions 5 & 6 (for master)
 * 					: This method puts the slave answer into master data buffer
 *****************************************************************************/
void get_FC5_FC6(modbusHandler_t *modH,modbus_t *telegram )
{
	//uint8_t  i,j,k;
	//uint8_t  tSlave_id = modH->u8Buffer[0];
	uint8_t  tFunction_code = modH->u8Buffer[1];

	if((tFunction_code == 0x05) || (tFunction_code == 0x6) || (tFunction_code == 0xF) || (tFunction_code == 0x10))
	{
		gFinalAnaValF[MODBUS_WRITE_QUERY_ACK_gFinalAnaValF + ((telegram->uQueryNo)-1)] = 1;
	}
	else
	{
		gFinalAnaValF[MODBUS_WRITE_QUERY_ACK_gFinalAnaValF + ((telegram->uQueryNo)-1)] = 0;
	}
}
/**************************************************************************//**
 * Function name 	: BuildModbusMasterQueryTelegrams
 * arguments		: 1)
 * 		 			:
 * return			:
 * Note				: Make Basic Telegram From Flash Data / RegisterMap
 * 					: Fill Address of Register Map in each Query Telegram using previousQuery + PreviousQuerylength
 * 					: Validation check for Query
 *****************************************************************************/

void changeRTUIDinSlaveLogic()
{
	ModbusH[COM_RS485_1].u8id = EPROM_General.Rtu_Detail.RTUId; //slave ID
	ModbusH[COM_RS485_2].u8id = EPROM_General.Rtu_Detail.RTUId; //slave ID
	ModbusH[COM_RS232_1].u8id = EPROM_General.Rtu_Detail.RTUId; //slave ID
	ModbusH[COM_RS232_2].u8id = EPROM_General.Rtu_Detail.RTUId; //slave ID
	ModbusH[COM_MODBUSTCP_1].u8id = EPROM_General.Rtu_Detail.RTUId; 			//slave ID
	ModbusH[COM_LORA].u8id = EPROM_General.Rtu_Detail.RTUId; 			//slave ID
}

void BuildModbusMasterQueryTelegrams()
{
//	unsigned char i ;
//	unsigned int degital_Query_store_Location = REG_MAP_START_ADDRESS_TO_STORE_READ_MODBUS_DATA_DIGITAL;
//	unsigned int analog_Query_store_Location = REG_MAP_START_ADDRESS_TO_STORE_READ_MODBUS_DATA;
	//Make Basic Telegram From Flash Data / RegisterMap

//	ModbusH[COM_RS485_1].uModbusType = EPROM_General.S_Comm.Rs485_1_Info.S_Ma_sl_Cu;//MB_MASTER;
//	ModbusH[COM_RS485_1].port =  &huart2; // This is the UART port connected to RS485
//	ModbusH[COM_RS485_1].u8id = EPROM_General.Rtu_Detail.RTUId; //slave ID
//	ModbusH[COM_RS485_1].u16regs = ModbusDATA;
//	ModbusH[COM_RS485_1].u16regsize= sizeof(ModbusDATA)/sizeof(ModbusDATA[0]);
//	ModbusH[COM_RS485_1].u8Port = COM_RS485_1;

	ModbusH[COM_RS232_1].uModbusType = EPROM_General.S_Comm.Rs232_1_Info.S_Ma_sl_Cu;//MB_DEBUG;
	ModbusH[COM_RS232_1].port =  &huart3; // This is the UART port connected to RS232
	ModbusH[COM_RS232_1].u8id = EPROM_General.Rtu_Detail.RTUId; //slave ID
	ModbusH[COM_RS232_1].u16regs = ModbusDATA;
	ModbusH[COM_RS232_1].u16regsize= sizeof(ModbusDATA)/sizeof(ModbusDATA[0]);
	ModbusH[COM_RS232_1].u8Port = COM_RS232_1;

//	ModbusH[COM_RS485_2].uModbusType = EPROM_General.S_Comm.Rs485_2_Info.S_Ma_sl_Cu;//MB_SLAVE;
//	ModbusH[COM_RS485_2].port =  &huart5; // This is the UART port connected to RS232
//	ModbusH[COM_RS485_2].u8id = EPROM_General.Rtu_Detail.RTUId; //slave ID
//	ModbusH[COM_RS485_2].u16regs = ModbusDATA;
//	ModbusH[COM_RS485_2].u16regsize= sizeof(ModbusDATA)/sizeof(ModbusDATA[0]);
//	ModbusH[COM_RS485_2].u8Port = COM_RS485_2;
//
//	ModbusH[COM_RS232_2].uModbusType = EPROM_General.S_Comm.Rs232_2_Info.S_Ma_sl_Cu;//MB_SLAVE;
//	ModbusH[COM_RS232_2].port =  &huart8; // This is the UART port connected to RS232
//	ModbusH[COM_RS232_2].u8id = EPROM_General.Rtu_Detail.RTUId; //slave ID
//	ModbusH[COM_RS232_2].u16regs = ModbusDATA;
//	ModbusH[COM_RS232_2].u16regsize= sizeof(ModbusDATA)/sizeof(ModbusDATA[0]);
//	ModbusH[COM_RS232_2].u8Port = COM_RS232_2;

	ModbusH[COM_MODBUSTCP_1].uModbusType = MB_SLAVE;
	ModbusH[COM_MODBUSTCP_1].u8id = EPROM_General.Rtu_Detail.RTUId; 			//slave ID
	ModbusH[COM_MODBUSTCP_1].u16regs = ModbusDATA;
	ModbusH[COM_MODBUSTCP_1].u16regsize= sizeof(ModbusDATA)/sizeof(ModbusDATA[0]);
	ModbusH[COM_MODBUSTCP_1].u8Port = COM_MODBUSTCP_1;
	ModbusH[COM_MODBUSTCP_1].u16timeOut = 100; // TODO : Check impect
	ModbusH[COM_MODBUSTCP_1].uTcpPort = 502;

	ModbusH[COM_LORA].uModbusType = MB_SLAVE;//MB_SLAVE;
	ModbusH[COM_LORA].port = COM_LORA; // This is the UART port connected to RS232
	ModbusH[COM_LORA].u8id = EPROM_General.Rtu_Detail.RTUId; //slave ID
	ModbusH[COM_LORA].u16regs = ModbusDATA;
	ModbusH[COM_LORA].u16regsize= sizeof(ModbusDATA)/sizeof(ModbusDATA[0]);
	ModbusH[COM_LORA].u8Port = COM_LORA; // maulin needs to change for lora

	if(Pro_Application_flag)
	{
		ModbusH[COM_RS232_1].uModbusType = MB_SLAVE;
		ModbusH[COM_RS232_2].uModbusType = MB_SLAVE;
		ModbusH[COM_RS485_1].uModbusType = MB_SLAVE;
		ModbusH[COM_RS485_2].uModbusType = MB_SLAVE;
	}


//	for(i=0;i<EPROM_Modbus_Quary_Detail.TotalQuery;i++)
//	{
//		telegram[i].uQueryNo = i;
//		telegram[i].uPortNo = EPROM_Modbus_Quary_Detail.Mod_Quary[i].mPortSelection;
//		telegram[i].u8id = EPROM_Modbus_Quary_Detail.Mod_Quary[i].mSlaveId;          /*!< Slave address between 1 and 247. 0 means broadcast */
//		telegram[i].u8fct = EPROM_Modbus_Quary_Detail.Mod_Quary[i].mFunctionCode;         /*!< Function code: 1, 2, 3, 4, 5, 6, 15 or 16 */
//		telegram[i].u16RegAdd = EPROM_Modbus_Quary_Detail.Mod_Quary[i].mRegStartAddr;    /*!< Address of the first register to access at slave/s */
//		telegram[i].u16CoilsNo = EPROM_Modbus_Quary_Detail.Mod_Quary[i].mNoOfRegister;   /*!< Number of coils or registers to access */
//		telegram[i].u16reg[0] = 0;
//		telegram[i].uDataType = EPROM_Modbus_Quary_Detail.Mod_Quary[i].mDataType;
//	}
//
//
//	//Count No of Slave on Each port
//
//	//Count No of Query in Each Port
//
//	//Count No of Data Points to read (it is less than location allocated for read qurey storage in Register map)
//
//	degital_Query_store_Location = REG_MAP_START_ADDRESS_TO_STORE_READ_MODBUS_DATA_DIGITAL;
//	analog_Query_store_Location = REG_MAP_START_ADDRESS_TO_STORE_READ_MODBUS_DATA;
//	gTotalNumberOfReadDataUsingAllQuery = 0;
//
//	for(unsigned int QueryNo=0;QueryNo<EPROM_Modbus_Quary_Detail.TotalQuery;QueryNo++)
//	{
//		// FUNCTION Code Validator Only Read Query AlloW
//		telegram[QueryNo].u8Validation = 0;
//		if(!((telegram[QueryNo].u8fct ==MB_FC_READ_COILS) || (telegram[QueryNo].u8fct == MB_FC_READ_DISCRETE_INPUT) ||
//				(telegram[QueryNo].u8fct ==MB_FC_READ_REGISTERS) || (telegram[QueryNo].u8fct==MB_FC_READ_INPUT_REGISTER)))
//		{
//			telegram[QueryNo].u8Validation = telegram[QueryNo].u8Validation|MODBUS_QUERY_VALIDATION_WRONG_FUNCTION_CODE;
//
//			sprintf((char *)print,"MODBUS_QUERY_VALIDATION_WRONG_FUNCTION_CODE:%d\r\n",telegram[QueryNo].u8Validation);
//			WriteLog(1, print, 1);
//		}
//
//		// DATA Type Validator
//
//		if(!(telegram[QueryNo].uDataType == DIGITAL_TYPE || telegram[QueryNo].uDataType == UNSIGNED16_TYPE ||
//				telegram[QueryNo].uDataType == INTEGER_TYPE || telegram[QueryNo].uDataType == SIGNED32_TYPE ||
//				telegram[QueryNo].uDataType == FLOAT_TYPE || telegram[QueryNo].uDataType == SWFLOAT_TYPE ||
//				telegram[QueryNo].uDataType == UNSIGNED32_TYPE || telegram[QueryNo].uDataType == SIGNED16_TYPE ||
//				telegram[QueryNo].uDataType == CHAR_STRING_TYPE))
//		{
//			telegram[QueryNo].u8Validation = telegram[QueryNo].u8Validation|MODBUS_QUERY_VALIDATION_DATA_TYPE_NOT_VALID;
//
//			sprintf((char *)print,"MODBUS_QUERY_VALIDATION_DATA_TYPE_NOT_VALID:%d\r\n",telegram[QueryNo].u8Validation);
//			WriteLog(1, print, 1);
//		}
//
//		// Find no of data in Query based on Data Type and also validate Datatype mismatch with no of coil
//
//		if(telegram[QueryNo].uDataType == FLOAT_TYPE || telegram[QueryNo].uDataType == SWFLOAT_TYPE || telegram[QueryNo].uDataType == SIGNED32_TYPE || telegram[QueryNo].uDataType == UNSIGNED32_TYPE || telegram[QueryNo].uDataType == INTEGER_TYPE)
//		{
//			if((telegram[QueryNo].u16CoilsNo)%2 == 0)
//			{
//				telegram[QueryNo].u8noOfData = (telegram[QueryNo].u16CoilsNo)/2;
//			}
//			else
//			{
//				telegram[QueryNo].u8Validation = telegram[QueryNo].u8Validation;
//				telegram[QueryNo].u8noOfData = ((telegram[QueryNo].u16CoilsNo+1)/2);
//			}
//		}
//		else
//		{
//			telegram[QueryNo].u8noOfData = telegram[QueryNo].u16CoilsNo;
//		}
//
//		// max Data in One Query Validator
//
//		if(telegram[QueryNo].u16CoilsNo > MODBUS_MASTER_MAX_DATA_IN_ONE_QUERY)
//		{
//			telegram[QueryNo].u8Validation = telegram[QueryNo].u8Validation|MODBUS_QUERY_VALIDATION_MAX_DATA_IN_ONE_QUERY_LIMIT_EXCEED;
//
//			sprintf((char *)print,"MODBUS_QUERY_VALIDATION_MAX_DATA_IN_ONE_QUERY_LIMIT_EXCEED:%d\r\n",telegram[QueryNo].u8Validation);
//			WriteLog(1, print, 1);
//		}
//
//		// Salve ID Range Validator
//
//		if((0 > telegram[QueryNo].u8id)||(telegram[QueryNo].u8id > 247))
//		{
//			telegram[QueryNo].u8Validation = telegram[QueryNo].u8Validation|MODBUS_QUERY_VALIDATION_SLAVE_ID_NOT_IN_RANGE;
//		}
//
//		// Calculate Data Store address in Register Map
//
//		if(QueryNo==0)
//		{
//			if(telegram[QueryNo].uDataType == DIGITAL_TYPE)
//			{
//				telegram[QueryNo].u16addressRegisterMap=REG_MAP_START_ADDRESS_TO_STORE_READ_MODBUS_DATA_DIGITAL;
//				degital_Query_store_Location += telegram[QueryNo].u8noOfData;
//			}
//			else
//			{
//				telegram[QueryNo].u16addressRegisterMap=REG_MAP_START_ADDRESS_TO_STORE_READ_MODBUS_DATA;
//				analog_Query_store_Location += telegram[QueryNo].u8noOfData;
//			}
//		}
//		else
//		{
//			if(telegram[QueryNo].uDataType == DIGITAL_TYPE)
//			{
//				telegram[QueryNo].u16addressRegisterMap = degital_Query_store_Location;//telegram[QueryNo-1].u16addressRegisterMap+telegram[QueryNo-1].u8noOfData;
//				degital_Query_store_Location += telegram[QueryNo].u16CoilsNo;
//			}
//			else
//			{
//				telegram[QueryNo].u16addressRegisterMap = analog_Query_store_Location;//telegram[QueryNo-1].u16addressRegisterMap+telegram[QueryNo-1].u8noOfData;
//				analog_Query_store_Location += telegram[QueryNo].u8noOfData;
//			}
//			//telegram[QueryNo].u16addressRegisterMap = telegram[QueryNo-1].u16addressRegisterMap+telegram[QueryNo-1].u8noOfData;
//		}
//
//		// Total Number Of Read Data limit Validator
//
//		if((telegram[QueryNo].uDataType == DIGITAL_TYPE)&&(degital_Query_store_Location-REG_MAP_START_ADDRESS_TO_STORE_READ_MODBUS_DATA_DIGITAL) > MODBUS_MASTER_MAX_MAX_TOTAL_DATA_DIGITAL)
//		{
//			telegram[QueryNo].u8Validation = telegram[QueryNo].u8Validation|MODBUS_QUERY_VALIDATION_MAX_TOTAL_DATA_LIMIT_EXCEED;
//
//			sprintf((char *)print,"MODBUS_QUERY_VALIDATION_MAX_TOTAL_DATA_LIMIT_EXCEED:%d\r\n",telegram[QueryNo].u8Validation);
//			WriteLog(1, print, 1);
//		}
//		else if((analog_Query_store_Location-REG_MAP_START_ADDRESS_TO_STORE_READ_MODBUS_DATA) > MODBUS_MASTER_MAX_MAX_TOTAL_DATA_ANALOG)
//		{
//			telegram[QueryNo].u8Validation = telegram[QueryNo].u8Validation|MODBUS_QUERY_VALIDATION_MAX_TOTAL_DATA_LIMIT_EXCEED;
//
//			sprintf((char *)print,"MODBUS_QUERY_VALIDATION_MAX_TOTAL_DATA_LIMIT_EXCEED:%d\r\n",telegram[QueryNo].u8Validation);
//			WriteLog(1, print, 1);
//		}
//
////		gTotalNumberOfReadDataUsingAllQuery=gTotalNumberOfReadDataUsingAllQuery+telegram[QueryNo].u8noOfData;
////		if(gTotalNumberOfReadDataUsingAllQuery>MODBUS_MASTER_MAX_MAX_TOTAL_DATA)
////		{
////			telegram[QueryNo].u8Validation = telegram[QueryNo].u8Validation|MODBUS_QUERY_VALIDATION_MAX_TOTAL_DATA_LIMIT_EXCEED;
////		}
//
//	}

}
/**************************************************************************//**
 * Function name 	: ProcessModbusSlave
 * arguments		: 1) modbusHandler_t *modH
 * 		 			:
 * return			:
 * Note				: In Slave Mode Receive Modbus Query and Proccess Accordingly
 * 					: 
 *****************************************************************************/
void ProcessModbusSlave(modbusHandler_t *modH )
{
//	modbusHandler_t *modH =  (modbusHandler_t *)argument;

//   if(modH->xTypeHW == USART_HW || modH->xTypeHW == USART_HW_DMA)
//   {
//
//	  ulTaskNotifyTake(pdTRUE, portMAX_DELAY); /* Block until a Modbus Frame arrives */
//
//	  if (getRxBuffer(modH) == ERR_BUFF_OVERFLOW)
//	  {
//	      modH->i8lastError = ERR_BUFF_OVERFLOW;
//	   	  modH->u16errCnt++;
//		  continue;
//	  }
//
//   }

//   ulTaskNotifyTake(pdTRUE, portMAX_DELAY); /* Block until a Modbus Frame arrives */

//   if (modH->u8BufferSize < 7)
//   {
//      //The size of the frame is invalid
//      modH->i8lastError = ERR_BAD_SIZE;
//      modH->u16errCnt++;
//
//	  continue;
//    }


   // check slave id
	if(modH->u8Port != COM_MODBUSTCP_1)
	{
		if ( modH->u8RxBuffer[ID] !=  modH->u8id)
		{
	    	return; // return from function if Slave ID is not matched
		}

	    modH->u8BufferSize = (word(modH->u8RxBuffer[NB_HI],modH->u8RxBuffer[NB_LO])*2)+1+1+2+2+2;
	}


    // validate message: CRC, FCT, address and size
    //uint8_t u8exception = validateRequest(modH);
	//if (u8exception > 0)
	//{
	//	return; // return from function if CRC,size, address ID is not matched
	//}

	 //xSemaphoreTake(modH->ModBusSphrHandle , portMAX_DELAY); //before processing the message get the semaphore

	 // process message
	 switch(modH->u8RxBuffer[ FUNC ] )
	 {
			case MB_FC_READ_COILS:
			case MB_FC_READ_DISCRETE_INPUT:
				modH->i8state = process_FC1(modH);
				break;
			case MB_FC_READ_INPUT_REGISTER:
				modH->i8state = process_FC1(modH);//@todo
				break;
			case MB_FC_READ_REGISTERS :
				modH->i8state = process_FC3(modH);
				break;
			case MB_FC_WRITE_COIL:
				modH->i8state = process_FC5(modH);
				break;
			case MB_FC_WRITE_REGISTER :
				modH->i8state = process_FC6(modH);
				break;
			case MB_FC_WRITE_MULTIPLE_COILS:
				modH->i8state = process_FC15(modH);
				break;
			case MB_FC_WRITE_MULTIPLE_REGISTERS :
				modH->i8state = process_FC16(modH);
				break;
			default:
				break;
	 }
	// xSemaphoreGive(modH->ModBusSphrHandle); //Release the semaphore
}

/**************************************************************************//**
 * Function name 	: ProcessModbusSlave
 * arguments		: 1) modbusHandler_t *modH
 * 		 			:
 * return			:
 * Note				: In Slave Mode Receive Modbus Query and Proccess Accordingly
 * 					:
 *****************************************************************************/

bool TCPwaitConnData(modbusHandler_t *modH)
{
  struct netbuf *inbuf;
  err_t recv_err, accept_err;
  char* buf;
  uint16_t buflen;
  uint16_t uLength;
  bool xTCPvalid = false;
  tcpclients_t *clientconn;
  char tBuffer[50];

  //select the next connection slot to work with using round-robin
  modH->newconnIndex++;
  if(modH->newconnIndex > NUMBERTCPCONN)
  {
	  modH->newconnIndex = 0;
  }

  clientconn = &modH->newconns[modH->newconnIndex];

  //NULL means there is a free connection slot, so we can accept an incoming client connection
  if (clientconn->conn == NULL)
  {
      /* accept any incoming connection */
	  accept_err = netconn_accept(modH->conn, &clientconn->conn);
	  if(accept_err != ERR_OK)
	  {
		  if(accept_err == ERR_TIMEOUT)
		  {
			  //ModbusCloseConnNull(modH); //Mukesh
			  return xTCPvalid;
		  }
		  else
		  {
			  //ModbusCloseConnNull(modH); //Mukesh
			  sprintf(tBuffer,"TIME OUT ELSE TCP_IP accept_err=%d\r\n",accept_err);
			  WriteLog(1, tBuffer, 1);
			  return xTCPvalid;
		  }
		  // not valid incoming connection at this time
		  //ModbusCloseConn(clientconn->conn);


		  //ModbusCloseConnNull(modH);
		  //return xTCPvalid;
      }
	  else
	  {
		  clientconn->aging=0;
	  }
  }

  netconn_set_recvtimeout(clientconn->conn ,  modH->u16timeOut);

  recv_err = netconn_recv(clientconn->conn, &inbuf);
  if (recv_err == ERR_CLSD) //the connection was closed
  {
	  //Close and clean the connection
	  //ModbusCloseConn(clientconn->conn);
	  sprintf(tBuffer,"TIME OUT ERR_CLSD TCP_IP accept_err=%d\r\n",recv_err);
	  WriteLog(1, tBuffer, 1);
	  ModbusCloseConnNull(modH); //Mukesh
	  clientconn->aging = 0;

	  return xTCPvalid;
  }
  else if (recv_err == ERR_TIMEOUT) //No new data
  {
	  //continue the aging process
	  modH->newconns[modH->newconnIndex].aging++;

	  // if the connection is old enough and inactive close and clean it up
	  if (modH->newconns[modH->newconnIndex].aging >= TCPAGINGCYCLES)
	  {
		  sprintf(tBuffer,"TIME OUT TCPAGINGCYCLES accept_err=%d\r\n",recv_err);
		  WriteLog(1, tBuffer, 1);
		  //ModbusCloseConn(clientconn->conn);
		  ModbusCloseConnNull(modH); //Mukesh
		  clientconn->aging = 0;
	  }

 	  return xTCPvalid;
  }
  else if (recv_err == ERR_OK)
  {
      if (netconn_err(clientconn->conn) == ERR_OK)
      {
    	  /* Read the data from the port, blocking if nothing yet there.
    	  We assume the request (the part we care about) is in one netbuf */
   	      netbuf_data(inbuf, (void**)&buf, &buflen);
		  if (buflen>11) // minimum frame size for modbus TCP
		  {
			  if(buf[2] == 0 || buf[3] == 0 ) //validate protocol ID
			  {
			  	  uLength = (buf[4]<<8 & 0xff00) | buf[5];
			  	  if(uLength< (MAX_BUFFER-2)  && (uLength + 6) <= buflen)
			   	  {
			          for(int i = 0; i < uLength; i++)
			          {
			        	  modH->u8RxBuffer[i] = buf[i+6];
			          }
			          modH->u16TransactionID = (buf[0]<<8 & 0xff00) | buf[1];
			          modH->u8BufferSize = uLength + 2; //add 2 dummy bytes for CRC
			          xTCPvalid = true; // we have data for the modbus slave
			      }
			  }
		  }

		  netbuf_delete(inbuf); // delete the buffer always
		  clientconn->aging = 0; //reset the aging counter
	   }
   }

  return xTCPvalid;

}
/**************************************************************************//**
 * Function name 	: ProcessModbusSlave
 * arguments		: 1) modbusHandler_t *modH
 * 		 			:
 * return			:
 * Note				: In Slave Mode Receive Modbus Query and Proccess Accordingly
 * 					:
 *****************************************************************************/
void  TCPinitserver(modbusHandler_t *modH)
{
      err_t err;

	  /* Create a new TCP connection handle */
	  //if(modH-> xTypeHW == TCP_HW)
	  //{
		  modH->conn = netconn_new(NETCONN_TCP);
		  if (modH->conn!= NULL)
		  {
		     /* Bind to port (502) Modbus with default IP address */
			 if(modH->uTcpPort == 0) modH->uTcpPort = 502; //if port not defined
		     //err = netconn_bind(modH->conn, NULL, modH->uTcpPort);
			 err = netconn_bind(modH->conn, IP_ADDR_ANY, 502);
			 if (err == ERR_OK)
		     {
		    	 /* Put the connection into LISTEN state */
		    	 netconn_listen(modH->conn);
		    	 netconn_set_recvtimeout(modH->conn, 1); // this is necessary to make it non blocking
		    	 //@todo: suspect list
		     }
		     else
		     {
		    	 while(1)
				 {
					  // error binding the TCP Modbus port check your configuration
				 }
			 }
		  }
		  else
		  {
			  while(1)
			  {
				  // error creating new connection check your configuration,
				  // this function must be called after the scheduler is started
			  }
		  }
	  //}
}
/**************************************************************************//**
 * Function name 	: ProcessModbusSlave
 * arguments		: 1) modbusHandler_t *modH
 * 		 			:
 * return			:
 * Note				: In Slave Mode Receive Modbus Query and Proccess Accordingly
 * 					:
 *****************************************************************************/
void ModbusCloseConn(struct netconn *conn)
{

	if(conn != NULL)
	{
		netconn_close(conn);
		netconn_delete(conn);
	}

}
/**************************************************************************//**
 * Function name 	: ProcessModbusSlave
 * arguments		: 1) modbusHandler_t *modH
 * 		 			:
 * return			:
 * Note				: In Slave Mode Receive Modbus Query and Proccess Accordingly
 * 					:
 *****************************************************************************/
void ModbusCloseConnNull(modbusHandler_t * modH)
{

	if(modH->newconns[modH->newconnIndex].conn  != NULL)
	{

		netconn_close(modH->newconns[modH->newconnIndex].conn);
		netconn_delete(modH->newconns[modH->newconnIndex].conn);
		modH->newconns[modH->newconnIndex].conn = NULL;
	}

}
