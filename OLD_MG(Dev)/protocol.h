#ifndef _PROTOCOL_H
#define _PROTOCOL_H

#include "main.h"

#define TX_ERR_ID		0x120
#define TX_CMD_ID		0x520
#define TX_DATA_ID	0x720
//#define RX_ID				0x320

#define	DEVICE_NUMBER 1

#define TX_CMD 		TX_CMD_ID + DEVICE_NUMBER
#define TX_DATA 	TX_DATA_ID + DEVICE_NUMBER
#define TX_ERROR	TX_ERR_ID + DEVICE_NUMBER

#define TIMESPAN							0x40	//время накопления
#define PERIOD_PULSE_POWER		0x80
#define PULSE_DURATION_POWER	0x81
#define WORKING_MODE					0x83
#define CURRENT_RANGE					0x90
#define SLIDER_SIZE_1					0x91
#define SLIDER_SIZE_2					0x92
#define SLIDER_SIZE_3					0x93
#define DEADTIME_1						0x94
#define DEADTIME_2						0x95
#define DEADTIME_3						0x96
#define RANGE_1_TO_2					0x97
#define RANGE_2_TO_3					0x98
#define RANGE_3_MAX						0x99
#define RANGE_3_TO_2					0x9A
#define RANGE_2_TO_1					0x9B
#define SENS_RANGE_1					0x9C
#define SENS_RANGE_2					0x9D
#define SENS_RANGE_3					0x9E


#define CORR_FACTOR_1_1				0xA1
#define CORR_FACTOR_1_2				0xA2
#define CORR_FACTOR_1_3				0xA3
#define CORR_FACTOR_1_4				0xA4

#define CORR_FACTOR_2_1				0xB1
#define CORR_FACTOR_2_2				0xB2

#define CORR_FACTOR_3_1				0xC1
#define CORR_FACTOR_3_2				0xC2

typedef enum
{
	E_SAVE_STATE,
	E_NO_SAVE_STATE
}eSaveSate;

typedef enum
{
	E_COMPLETED_RESPONSE = 0x00,	//завершаем процесс передачи данных
	E_ACCEPT_RESPONSE = 0x01,			//получиил подтверждение о приему запроса на передачу
	E_NEWDATA_RESPONSE = 0x03			//новые данные
}eResponseState;

typedef  struct
{
	uint8 		frmVer;			//X.X - 4bits
	uint8 		prVer;			//X.X	-	4bits
	uint32		serialNum;	//HEX
}sServiceMsg;

void Protocol_Initialization(void);
void Protocol_ReceiveMsgCallback(uint8	devID, uint8 *pData);
void Protocol_TransmitMsgCallback(uint16 canID);

void Protocol_HeaderDataMsg(uint8	*pData,uint16	counterRate);
void Protocol_CounterMsg(uint8	*pCanData,uint16	*pData);
void Protocol_DoseRateMsg(uint8	*pData,float doseValue);
void Protocol_StartMsg(void);
void Protocol_WDTRestartMsg(void);
void Protocol_SetRangeValue(uint8);

void Protocol_ResponseDataPackaje(eResponseState responseState);

uint8 Protocol_GetRangeValue(void);
uint8	Protocol_GetAutoModeState(void);
#endif
