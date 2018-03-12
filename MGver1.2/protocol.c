#include "protocol.h"
#include "can.h"
#include "settings.h"
#include "process.h"
#include "system.h"

uint16	timeSpan;			//время измерения (в десятых долях сек)
uint8		rangeValue;		//номер текущего диапазона (0 - авто)
uint8		autoMode;			//автоматический режим
	
const uint8 mResetResponse[] = {0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00};
const uint8	mWDTRestartResponse[] = {0xEE};

static uint8	dataMsgCounter = 0;



void Protocol_Initialization()
{
	rangeValue = 0;
	timeSpan = 10;
	
	autoMode = 0;
}

void Protocol_SetRangeValue(uint8	Value)
{
	rangeValue = Value;
}

uint8 Protocol_GetRangeValue()
{
	return rangeValue;
}
uint8	Protocol_GetAutoModeState()
{
	return autoMode;
}

void Protocol_TransmitMsgCallback(uint16	canID)
{
	/*if(canID == TX_DATA)
	{
		dataMsgCounter++;
		
			if(dataMsgCounter == 1)
				Protocol_CounterMsg();
			else if(dataMsgCounter == 2)
				Protocol_DoseRateMsg();
			else 
				dataMsgCounter = 0;
	}*/
}
//отправка сообщения с параметром
void Protocol_SendDataMsg(uint8*	pData,uint8	*pValue,uint8 dataSize)
{
	uint8	pIndex = pData[1];
	
		memcpy(pData + 4,(uint32*)(pValue),dataSize);
	
	pData[1] = 0;
	pData[2] = pIndex;
	
	CAN_SendMessage(TX_CMD,pData,8);
}
//получение параметра от ПО
void Protocol_GetDataMsg(uint8*	pValue,	uint8	*pData,uint8	dataSize,eSaveSate saveState)
{
	memcpy(pValue,pData + 4,dataSize);
		if(saveState == E_SAVE_STATE)SETTINGS_Save();
	
	Protocol_SendDataMsg(pData,(uint8*)pValue,dataSize);
}
//отправка синхронизационного сообщения
void Protocol_ResponseDataPackaje(eResponseState responseState)
{
	uint8	pData[2];
				pData[0] = 0x40;
				pData[1] = (uint8)responseState;
	
		CAN_SendMessage(TX_DATA,pData,sizeof(pData));
}
//подтверждение остановки приема данных
void Protocol_ConfirmStopData()
{
	uint8	pData[2];
		pData[0] = 0x41;
		pData[1] = 0;
	
	CAN_SendMessage(TX_DATA,pData,sizeof(pData));
}

//Заголовочное сообщение данных
void Protocol_HeaderDataMsg(uint8	*pData,uint16	counterRate)
{	
	//uint8	pData[8];
	
	dataMsgCounter = 0;
	memset(pData,0,sizeof(uint8) * 8);
	
		pData[0] = MainSettings.modeValue;
		pData[1] = rangeValue;
		pData[2] = (uint8)timeSpan & 0x00FF;
		pData[3] = (uint8)(timeSpan >> 8);
		pData[4] = 2;	//кол-во байт на счётчик
	//высчитываем кол-во счётчиков
		if(rangeValue == 1)
			pData[5] = 4;
		else
			pData[5] = 2;
		
		pData[6] = (uint8)counterRate & 0x00FF;
		pData[7] = (uint8)(counterRate >> 8);
		
		
	//CAN_SendMessage(TX_DATA,pData,sizeof(pData));
}
void Protocol_CounterMsg(uint8	*pCanData,uint16	*pData)
{
	//uint16 *pCanData[8];// = (uint16*)pData;
		memset(pCanData,0,sizeof(uint16) * 8);

	if(	rangeValue == 1){
		memcpy(pCanData,pData,sizeof(uint16) * 4);
	}		
	else if(rangeValue == 2){
		memcpy(pCanData,pData + 4,sizeof(uint16) * 2);
	}
	else{
		memcpy(pCanData,pData + 6,sizeof(uint16) * 2);
	}
	//CAN_SendMessage(TX_DATA,(uint8*)pCanData,8);
	
}
void Protocol_DoseRateMsg(uint8	*pData,float doseValue)
{
	//uint8	pData[8];
		
		memcpy(pData,&doseValue,sizeof(float));
		memset(pData + 4,(uint8)0xFF,sizeof(float));
	
	//CAN_SendMessage(TX_DATA,pData,sizeof(pData));
}
void Protocol_StartMsg()
{
	CAN_SendMessage(TX_CMD,mResetResponse,8);
}
void Protocol_WDTRestartMsg()
{
	CAN_SendMessage(TX_CMD,mWDTRestartResponse,1);
}

void Protocol_ReceiveMsgCallback(uint8	devID, uint8 *pData)
{
	__IO uint8	x = 0;
	if(devID == 0)	//пришло сообщение для всех БД
	{
		if(pData[0] == 0x01)
		{	
			
			System_Reset();
		}
	}
	else if(devID == DEVICE_NUMBER)	//получили месседж для конкретного блока
	{
		switch(pData[0])//проверяем индекс команды
		{
			case 0x04:	//выдать параметр
			{
				switch(pData[1])
				{
					case TIMESPAN:
					{
						Protocol_SendDataMsg(pData,(uint8*)&timeSpan,sizeof(timeSpan));	break; 
					}
					break;
					
					case PERIOD_PULSE_POWER: 		Protocol_SendDataMsg(pData,(uint8*)&MainSettings.pulsePwrPeriod,sizeof(MainSettings.pulsePwrPeriod));	break; 
					case PULSE_DURATION_POWER: 	Protocol_SendDataMsg(pData,(uint8*)&MainSettings.pulseDuration,sizeof(MainSettings.pulseDuration)); break;
					case WORKING_MODE:	Protocol_SendDataMsg(pData,(uint8*)&MainSettings.modeValue,sizeof(MainSettings.modeValue)); break;
					case CURRENT_RANGE: Protocol_SendDataMsg(pData,(uint8*)&rangeValue,sizeof(rangeValue)); break;
					
					case DEADTIME_1:	Protocol_SendDataMsg(pData,(uint8*)&MainSettings.mDeadTime[0],sizeof(MainSettings.mDeadTime[0]));	break;				
					case DEADTIME_2:	Protocol_SendDataMsg(pData,(uint8*)&MainSettings.mDeadTime[1],sizeof(MainSettings.mDeadTime[1]));	break;
					case DEADTIME_3:	Protocol_SendDataMsg(pData,(uint8*)&MainSettings.mDeadTime[2],sizeof(MainSettings.mDeadTime[2]));	break;			
					//DEBUG SECTION
					case RANGE_1_TO_2:	Protocol_SendDataMsg(pData,(uint8*)&MainSettings.range1to2Value,sizeof(MainSettings.range1to2Value));	break;
					case RANGE_2_TO_3:	Protocol_SendDataMsg(pData,(uint8*)&MainSettings.range2to3Value,sizeof(MainSettings.range2to3Value));	break;
					case RANGE_3_MAX:		Protocol_SendDataMsg(pData,(uint8*)&MainSettings.range3maxValue,sizeof(MainSettings.range3maxValue));	break;
					case RANGE_3_TO_2:	Protocol_SendDataMsg(pData,(uint8*)&MainSettings.range3to2Value,sizeof(MainSettings.range3to2Value));	break;
					case RANGE_2_TO_1:	Protocol_SendDataMsg(pData,(uint8*)&MainSettings.range2to1Value,sizeof(MainSettings.range2to1Value));	break;
					
					case SENS_RANGE_1:	Protocol_SendDataMsg(pData,(uint8*)&MainSettings.fSensCounters[0],sizeof(MainSettings.fSensCounters[0]));	break;				
					case SENS_RANGE_2:	Protocol_SendDataMsg(pData,(uint8*)&MainSettings.fSensCounters[1],sizeof(MainSettings.fSensCounters[1]));	break;
					case SENS_RANGE_3:	Protocol_SendDataMsg(pData,(uint8*)&MainSettings.fSensCounters[2],sizeof(MainSettings.fSensCounters[2]));	break;	
					//END DEBUG SECTION
					
					//Correction factors
					case CORR_FACTOR_1_1:		Protocol_SendDataMsg(pData,(uint8*)&MainSettings.correctionFactors[0],sizeof(float));	break;
					case CORR_FACTOR_1_2:		Protocol_SendDataMsg(pData,(uint8*)&MainSettings.correctionFactors[1],sizeof(float));	break;
					case CORR_FACTOR_1_3:		Protocol_SendDataMsg(pData,(uint8*)&MainSettings.correctionFactors[2],sizeof(float));	break;
					case CORR_FACTOR_1_4:		Protocol_SendDataMsg(pData,(uint8*)&MainSettings.correctionFactors[3],sizeof(float));	break;
					case CORR_FACTOR_2_1:		Protocol_SendDataMsg(pData,(uint8*)&MainSettings.correctionFactors[4],sizeof(float));	break;
					case CORR_FACTOR_2_2:		Protocol_SendDataMsg(pData,(uint8*)&MainSettings.correctionFactors[5],sizeof(float));	break;
					case CORR_FACTOR_3_1:		Protocol_SendDataMsg(pData,(uint8*)&MainSettings.correctionFactors[6],sizeof(float));	break;
					case CORR_FACTOR_3_2:		Protocol_SendDataMsg(pData,(uint8*)&MainSettings.correctionFactors[7],sizeof(float));	break;
					//end correction factors
					default: 
					{
						pData[0] = 0xEE;
						pData[2] = pData[1];
						pData[1] = 0x04;
						CAN_SendMessage(TX_ERROR,pData,3);
					}break;
					
				}
			}break;
			
			case 0x05:	//задать параметр
			{
				switch(pData[1])
				{
					case TIMESPAN:	Protocol_GetDataMsg((uint8*)&timeSpan,pData,sizeof(timeSpan),E_NO_SAVE_STATE);	break;
					
					case PERIOD_PULSE_POWER:		Protocol_GetDataMsg((uint8*)&MainSettings.pulsePwrPeriod,pData,sizeof(MainSettings.pulsePwrPeriod),E_SAVE_STATE);	break;
					case PULSE_DURATION_POWER:	Protocol_GetDataMsg((uint8*)&MainSettings.pulseDuration,pData,sizeof(MainSettings.pulseDuration),E_SAVE_STATE);	break;
					case WORKING_MODE:					Protocol_GetDataMsg((uint8*)&MainSettings.modeValue,pData,sizeof(MainSettings.modeValue),E_SAVE_STATE);	break;
					case CURRENT_RANGE:					
					{
						Protocol_GetDataMsg((uint8*)&rangeValue,pData,sizeof(rangeValue),E_NO_SAVE_STATE);
						if(rangeValue == 0)
							autoMode = 1;
						else
							autoMode = 0;
					}break;
					
					case RANGE_1_TO_2:	Protocol_GetDataMsg((uint8*)&MainSettings.range1to2Value,pData,sizeof(uint16),E_SAVE_STATE);	break;
					case RANGE_2_TO_3:	Protocol_GetDataMsg((uint8*)&MainSettings.range2to3Value,pData,sizeof(uint16),E_SAVE_STATE);	break;
					case RANGE_3_MAX:		Protocol_GetDataMsg((uint8*)&MainSettings.range3maxValue,pData,sizeof(uint16),E_SAVE_STATE);	break;
					case RANGE_3_TO_2:	Protocol_GetDataMsg((uint8*)&MainSettings.range3to2Value,pData,sizeof(uint16),E_SAVE_STATE);	break;
					case RANGE_2_TO_1:	Protocol_GetDataMsg((uint8*)&MainSettings.range2to1Value,pData,sizeof(uint16),E_SAVE_STATE);	break;
					
					case SENS_RANGE_1:	Protocol_GetDataMsg((uint8*)&MainSettings.fSensCounters[0],pData,sizeof(float),E_SAVE_STATE);	break;
					case SENS_RANGE_2:	Protocol_GetDataMsg((uint8*)&MainSettings.fSensCounters[1],pData,sizeof(float),E_SAVE_STATE);	break;
					case SENS_RANGE_3:	Protocol_GetDataMsg((uint8*)&MainSettings.fSensCounters[2],pData,sizeof(float),E_SAVE_STATE);	break;
					
					case CORR_FACTOR_1_1:	Protocol_GetDataMsg((uint8*)&MainSettings.correctionFactors[0],pData,sizeof(float),E_SAVE_STATE);	break;
					case CORR_FACTOR_1_2:	Protocol_GetDataMsg((uint8*)&MainSettings.correctionFactors[1],pData,sizeof(float),E_SAVE_STATE);	break;
					case CORR_FACTOR_1_3:	Protocol_GetDataMsg((uint8*)&MainSettings.correctionFactors[2],pData,sizeof(float),E_SAVE_STATE);	break;
					case CORR_FACTOR_1_4:	Protocol_GetDataMsg((uint8*)&MainSettings.correctionFactors[3],pData,sizeof(float),E_SAVE_STATE);	break;
					case CORR_FACTOR_2_1:	Protocol_GetDataMsg((uint8*)&MainSettings.correctionFactors[4],pData,sizeof(float),E_SAVE_STATE);	break;
					case CORR_FACTOR_2_2:	Protocol_GetDataMsg((uint8*)&MainSettings.correctionFactors[5],pData,sizeof(float),E_SAVE_STATE);	break;
					case CORR_FACTOR_3_1:	Protocol_GetDataMsg((uint8*)&MainSettings.correctionFactors[6],pData,sizeof(float),E_SAVE_STATE);	break;
					case CORR_FACTOR_3_2:	Protocol_GetDataMsg((uint8*)&MainSettings.correctionFactors[7],pData,sizeof(float),E_SAVE_STATE);	break;
					
					case DEADTIME_1:	Protocol_GetDataMsg((uint8*)&MainSettings.mDeadTime[0],pData,sizeof(float),E_SAVE_STATE);	break;
					case DEADTIME_2:	Protocol_GetDataMsg((uint8*)&MainSettings.mDeadTime[1],pData,sizeof(float),E_SAVE_STATE);	break;
					case DEADTIME_3:	Protocol_GetDataMsg((uint8*)&MainSettings.mDeadTime[2],pData,sizeof(float),E_SAVE_STATE);	break;
					
					default: 
					{
						pData[0] = 0xEE;
						pData[2] = pData[1];
						pData[1] = 0x05;
							CAN_SendMessage(TX_ERROR,pData,3);
					}break;	
				}
				
				
			}break;
			
			case 0x40:	//старт измерений
			{
				Protocol_ResponseDataPackaje(E_ACCEPT_RESPONSE);
				Process_Start();
				
				
			}break;
			
			case 0x41:	//стоп измерений
			{
				
				Protocol_ConfirmStopData();
				Process_Stop();
				//Protocol_ResponseDataPackaje(E_COMPLETED_RESPONSE);
				
				
			}break;
		}
	}
	else
	{
		
	}

}
