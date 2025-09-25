/*
 * common.h
 *
 *  Created on: Nov 21, 2022
 *      Author: maulin
 */

#ifndef INC_COMMON_H_
#define INC_COMMON_H_

/**************************************************************************//**
 * Constant
 *****************************************************************************/

//RegisterMap Address
#define REG_MAP_START_ADDRESS_TO_STORE_ANALOG_DATA 				45
#define REG_MAP_START_ADDRESS_TO_STORE_READ_MODBUS_DATA_DIGITAL 16 // 0 to 15 physical Di  and 16 to ... Modbus DI
#define REG_MAP_START_ADDRESS_TO_STORE_READ_MODBUS_DATA 		300
#define REG_MAP_START_ADDRESS_TO_STORE_AI_TAG					300

#define PRODUCTION_TEST_BIT_DO			0
#define PRODUCTION_TEST_BIT_DI			1
#define PRODUCTION_TEST_BIT_RTC			2
#define PRODUCTION_TEST_BIT_GPS			3
#define PRODUCTION_TEST_BIT_BLE			4
#define PRODUCTION_TEST_BIT_4G			5
#define PRODUCTION_TEST_BIT_FLASH		6
#define PRODUCTION_TEST_BIT_RS485_1		7
#define PRODUCTION_TEST_BIT_RS485_2		8
#define PRODUCTION_TEST_BIT_RS232_1		9
#define PRODUCTION_TEST_BIT_RS232_2		10
#define PRODUCTION_TEST_BIT_SD_CARD		11
#define PRODUCTION_TEST_BIT_KEY_LED		12
#define PRODUCTION_TEST_BIT_WATCHDOG	13

/**************************************************************************//**
 * TYPE Def
 *****************************************************************************/
typedef union
{
	unsigned char ch[4];
	signed char s_ch[4];
	unsigned short sh;
	unsigned int uint;
	float fl;
	int int_32;
	short sh_16;
} union_Datatypes ;

typedef union {
	uint8_t  u8[4];
	uint16_t u16[2];
	uint32_t u32;

} bytesFields ;


// A structure with forced alignment
typedef struct
{
	uint8_t variable0 	: 1;
	uint8_t variable1 	: 1;
	uint8_t variable2 	: 1;
	uint8_t variable3 	: 1;
	uint8_t variable4 	: 1;
	uint8_t variable5 	: 1;
	uint8_t variable6 	: 1;
	uint8_t variable7 	: 1;
}flag_ChangeRequired;

/**************************************************************************//**
 * Macro
 *****************************************************************************/
/**
 * \brief           Calculate length of statically allocated array
 */
#define ARRAY_LEN(x)            (sizeof(x) / sizeof((x)[0]))

///**
// * \brief           Assert an input parameter if in valid range
// * \note            Since this is a macro, it may only be used on a functions where return status is of type \ref lwgsmr_t enumeration
// * \param[in]       c: Condition to test
// */
/*
#define LWGSM_ASSERT(c)                                                                                           		\
    do {                                                                                                               \
        if (!(c)) {                                                                                                    \
            LWGSM_DEBUGF(LWGSM_CFG_DBG_ASSERT, "Assert failed in file %s on line %d: %s\r\n", __FILE__, (int)__LINE__, \
                         #c);                                                                                          \
            return lwgsmERRPAR;                                                                                        \
        }                                                                                                             \
    } while (0)
*/
/**
 * \brief           Align `x` value to specific number of bytes, provided by \ref LWGSM_CFG_MEM_ALIGNMENT configuration
 * \param[in]       x: Input value to align
 * \return          Input value aligned to specific number of bytes
 * \hideinitializer
 */
#define LWGSM_MEM_ALIGN(x)                ((x + (LWGSM_CFG_MEM_ALIGNMENT - 1)) & ~(LWGSM_CFG_MEM_ALIGNMENT - 1))

/**
 * \brief           Get minimal value between `x` and `y` inputs
 * \param[in]       x: First input to test
 * \param[in]       y: Second input to test
 * \return          Minimal value between `x` and `y` parameters
 * \hideinitializer
 */
#define LWGSM_MIN(x, y)                   ((x) < (y) ? (x) : (y))

/**
 * \brief           Get maximal value between `x` and `y` inputs
 * \param[in]       x: First input to test
 * \param[in]       y: Second input to test
 * \return          Maximal value between `x` and `y` parameters
 * \hideinitializer
 */
#define LWGSM_MAX(x, y)                   ((x) > (y) ? (x) : (y))

/**
 * \brief           Get size of statically declared array
 * \param[in]       x: Input array
 * \return          Number of array elements
 * \hideinitializer
 */
#define LWGSM_ARRAYSIZE(x)                (sizeof(x) / sizeof((x)[0]))

/**
 * \brief           Unused argument in a function call
 * \note            Use this on all parameters in a function which are not used to prevent
 *                  compiler warnings complaining about "unused variables"
 * \param[in]       x: Variable which is not used
 * \hideinitializer
 */
#define LWGSM_UNUSED(x)                   ((void)(x))

/**
 * \brief           Get input value casted to `unsigned 32-bit` value
 * \param[in]       x: Input value
 * \hideinitializer
 */
#define LWGSM_U32(x)                      ((uint32_t)(x))

/**
 * \brief           Get input value casted to `unsigned 16-bit` value
 * \param[in]       x: Input value
 * \hideinitializer
 */
#define LWGSM_U16(x)                      ((uint16_t)(x))

/**
 * \brief           Get input value casted to `unsigned 8-bit` value
 * \param[in]       x: Input value
 * \hideinitializer
 */
#define LWGSM_U8(x)                       ((uint8_t)(x))

/**
 * \brief           Get input value casted to `signed 32-bit` value
 * \param[in]       x: Input value
 * \hideinitializer
 */
#define LWGSM_I32(x)                      ((int32_t)(x))

/**
 * \brief           Get input value casted to `signed 16-bit` value
 * \param[in]       x: Input value
 * \hideinitializer
 */
#define LWGSM_I16(x)                      ((int16_t)(x))

/**
 * \brief           Get input value casted to `signed 8-bit` value
 * \param[in]       x: Input value
 * \hideinitializer
 */
#define LWGSM_I8(x)                       ((int8_t)(x))

/**
 * \brief           Get input value casted to `size_t` value
 * \param[in]       x: Input value
 * \hideinitializer
 */
#define LWGSM_SZ(x)                       ((size_t)(x))

/**
 * \brief           Convert `unsigned 32-bit` number to string
 * \param[in]       num: Number to convert
 * \param[out]      out: Output variable to save string
 * \return          Pointer to output variable
 * \hideinitializer
 */
#define lwgsm_u32_to_str(num, out)        lwgsm_u32_to_gen_str(LWGSM_U32(num), (out), 0, 0)

/**
 * \brief           Convert `unsigned 32-bit` number to HEX string
 * \param[in]       num: Number to convert
 * \param[out]      out: Output variable to save string
 * \param[in]       w: Width of output string.
 *                      When number is shorter than width, leading `0` characters will apply.
 * \return          Pointer to output variable
 * \hideinitializer
 */
#define lwgsm_u32_to_hex_str(num, out, w) lwgsm_u32_to_gen_str(LWGSM_U32(num), (out), 1, (w))

/**
 * \brief           Convert `signed 32-bit` number to string
 * \param[in]       num: Number to convert
 * \param[out]      out: Output variable to save string
 * \return          Pointer to output variable
 * \hideinitializer
 */
#define lwgsm_i32_to_str(num, out)        lwgsm_i32_to_gen_str(LWGSM_I32(num), (out))

/**
 * \brief           Convert `unsigned 16-bit` number to string
 * \param[in]       num: Number to convert
 * \param[out]      out: Output variable to save string
 * \return          Pointer to output variable
 * \hideinitializer
 */
#define lwgsm_u16_to_str(num, out)        lwgsm_u32_to_gen_str(LWGSM_U32(LWGSM_U16(num)), (out), 0, 0)

/**
 * \brief           Convert `unsigned 16-bit` number to HEX string
 * \param[in]       num: Number to convert
 * \param[out]      out: Output variable to save string
 * \param[in]       w: Width of output string.
 *                      When number is shorter than width, leading `0` characters will apply.
 * \return          Pointer to output variable
 * \hideinitializer
 */
#define lwgsm_u16_to_hex_str(num, out, w) lwgsm_u32_to_gen_str(LWGSM_U32(LWGSM_U16(num)), (out), 1, (w))

/**
 * \brief           Convert `signed 16-bit` number to string
 * \param[in]       num: Number to convert
 * \param[out]      out: Output variable to save string
 * \return          Pointer to output variable
 * \hideinitializer
 */
#define lwgsm_i16_to_str(num, out)        lwgsm_i32_to_gen_str(LWGSM_I32(LWGSM_I16(num)), (out))

/**
 * \brief           Convert `unsigned 8-bit` number to string
 * \param[in]       num: Number to convert
 * \param[out]      out: Output variable to save string
 * \return          Pointer to output variable
 * \hideinitializer
 */
#define lwgsm_u8_to_str(num, out)         lwgsm_u32_to_gen_str(LWGSM_U32(LWGSM_U8(num)), (out), 0, 0)

/**
 * \brief           Convert `unsigned 16-bit` number to HEX string
 * \param[in]       num: Number to convert
 * \param[out]      out: Output variable to save string
 * \param[in]       w: Width of output string.
 *                      When number is shorter than width, leading `0` characters will apply.
 * \return          Pointer to output variable
 * \hideinitializer
 */
#define lwgsm_u8_to_hex_str(num, out, w)  lwgsm_u32_to_gen_str(LWGSM_U32(LWGSM_U8(num)), (out), 1, (w))

/**
 * \brief           Convert `signed 8-bit` number to string
 * \param[in]       num: Number to convert
 * \param[out]      out: Output variable to save string
 * \return          Pointer to output variable
 * \hideinitializer
 */
#define lwgsm_i8_to_str(num, out)         lwgsm_i32_to_gen_str(LWGSM_I32(LWGSM_I8(num)), (out))


/**************************************************************************//**
 * Extern Variable
 *****************************************************************************/

extern unsigned char beforeRTOS,proCheck,checkAgain;
extern unsigned int SlotNo_RS232_1,SlotNo_RS232_2,SlotNo;
extern unsigned char PROD_Ethernet_IP[4];
extern unsigned char pro_MQTT_Broker_IP[30];
extern unsigned int pro_MQTT_Broker_Port;
extern unsigned char pro_MQTT_Client_ID[30];
extern unsigned char pro_LORA_AppEUI[30];
extern unsigned char pro_LORA_AppKey[30];
extern unsigned short int proTestRequest;
extern unsigned char ethernetMac[6];
extern unsigned char Pro_Application_flag,pro_DO_DI_TestFinish,UART_OTAflag,UART_OTA_ACKflag;

extern unsigned char pro_RS232_1_state,pro_RS232_2_state,pro_RS485_1_state,pro_RS485_2_state;
extern unsigned char pro_DO_State[26],pro_DI_State[8];
extern unsigned char pro_key1_status,pro_key2_status,pro_Flash_State,pro_I2C_State;

/**************************************************************************//**
 * Function Proto type
 *****************************************************************************/
uint16_t word(uint8_t H, uint8_t L);
char* lwgsm_u32_to_gen_str(uint32_t num, char* out, uint8_t is_hex, uint8_t padding);
char* lwgsm_i32_to_gen_str(int32_t num, char* out);
void lwgsm_str_to_asciiStr(const char * _in_Str,unsigned int _in_Str_len,const char * _out_asciiStr,unsigned int _in_Str_out);
char HexChartoHexByte(unsigned char a);
unsigned char convertHextoAsciiString(char* i_HexString,char* O_AsciiString,unsigned int _length);
int FindSubstr(char *listPointer, char *itemPointer);
void checkProductionMode();
void setProductionModePara();
int atoi_new(const char* str, int len);
void asciiStringToHexString(char* str, char* hexStr, int len);
void calculateLograteTimeSliceDelayS();
#endif /* INC_COMMON_H_ */
