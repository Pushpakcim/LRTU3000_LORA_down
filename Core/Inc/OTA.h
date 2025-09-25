/*
 * OTA.h
 *
 *  Created on: Jan 5, 2023
 *      Author: Shreyanss
 */

#ifndef INC_OTA_H_
#define INC_OTA_H_

//#include "libs.h"
#include "w25q_mem.h"
#include "json_parser.h"

typedef enum {
    OTA_FAIL = 0,
	OTA_SUCCESS,
} OTA_FILE_ACK;

typedef enum {
    PLC = 0,        //.PLC File
	REC, 			//.REC File
	HEX,			//.bin File
} OTA_FILE_TYPE;

typedef enum {
    START = 0,			//Start OTA
	IN_PROGRESS,		//In Progress
	VERIFICATION,		//Verification
	FINISH,				//Finish
} OTA_STATE;

struct OTA
{
	int CMDState; //1
	int FileType; //1
	int Chunk_no; //1
	int Chunk_Size; //512
	int crc;
	int FileSize;
	//char Chunk_Data[8192]; // hex string 2k or 4k data
};


struct OTA_ACK
{
	int cmd;
	int CMDState;
	int Chunk_no;
	int FileType;
	char OtaACK;
};

typedef struct OTA_hex_Data
{
	unsigned char checkbyte;							//1
	unsigned char ChecksumOfStuct;						//1+1=2
	uint16_t SizeOfStuct;								//2+2=4
	char OTA_State;
	char HEX_Crc;
	long int File_Size;
	unsigned char bootloaderVersion[100];
	char extrs_reserved[100];
}OTA_hex_Data_t;

extern struct OTA_ACK OTA_ACK_Data;

char OTA_HexChartoHexByte(unsigned char a);
//void Ota_File_write_ack(COM_TYPE com_mode,int fileType,int chunk_number,int chunk_lenth,char *data ,char * ACK_Response);
unsigned char varify_OTA_CRC(unsigned char crc,unsigned long int length,unsigned int start_address);
unsigned char convert_OTA_HextoAsciiString(char* i_HexString, char* O_AsciiString);

#endif /* INC_OTA_H_ */
