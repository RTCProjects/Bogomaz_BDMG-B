#include "process.h"
#include "protocol.h"
#include "settings.h"
#include "can.h"

uint8		sliderCounter = 0;
uint8		dynamicMode = 0;

uint16	sensorCnt[8];

float		sliderQuery[SLIDER_SIZE];


void	Process_Initializetion(void)
{
	memset(sensorCnt,0,sizeof(sensorCnt));
	memset(sliderQuery,0,sizeof(float) * SLIDER_SIZE);
	
	CC8IC = 0x007C;
	CC9IC = 0x0078;
	CC10IC = 0x0074;
	CC11IC = 0x0070;
	
	CC12IC = 0x0074;
	CC13IC = 0x0070;
	
	CC14IC = 0x0074;
	CC15IC = 0x0070;
		
}

/*
	Обработка импульса счётчика
	1. Инкрементируем счётчик текущего канала
	2. Снимаем питание  на счётчиках на 100us
*/
void Process_Group1IntOff()
{
	CC8IE = 0;
	CC9IE = 0;
	CC10IE = 0;
	CC11IE = 0;
}
void Process_Group2IntOff()
{
	
}
void Process_Group3IntOff()
{
	
}


void Process_EXIN1() 	interrupt CC8IC_VEC
{
	//T7IR = 1;
	T7 = 0xFFFF;
	sensorCnt[0]++;
	
}
void Process_EXIN2()	interrupt CC9IC_VEC
{
	T7 = 0xFFFF;
	sensorCnt[1]++;
}
void Process_EXIN3()	interrupt CC10IC_VEC
{	
	T7 = 0xFFFF;
	sensorCnt[2]++;
}
void Process_EXIN4()	interrupt CC11IC_VEC
{
	T7 = 0xFFFF;
	sensorCnt[3]++;
}
void Process_EXIN5()	interrupt CC12IC_VEC
{
	T8 = 0xFFFF;
	sensorCnt[4]++;
}
void Process_EXIN6()	interrupt CC13IC_VEC
{
	T8 = 0xFFFF;
	sensorCnt[5]++;
}
void Process_EXIN7()	interrupt CC14IC_VEC
{
	T1 = 0xFFFF;
	sensorCnt[6]++;
	
}
void Process_EXIN8()	interrupt CC15IC_VEC
{
	T1 = 0xFFFF;
	sensorCnt[7]++;
	
}

/*
Процедура подсчёта данных
*/

void	Process_MainProc()	interrupt T0IC_VEC
{
	uint8	*pSendPackage;
	uint8	i = 0;
	uint8	sliderMax = 2;	//параметр динамического заполнения движка
	uint8	currentRange = Protocol_GetRangeValue();	//текущий диапазон
	uint8	autoMode = Protocol_GetAutoModeState();

	float currentDose = 0;	//текущая МД
	float averageDose = 0;	//средняя МД
	float counterRate = 0;	//текущая скорость счёта
	
	//получаем МД и скорость счёта
		currentDose = Process_CalculateCurrentDoseRate(currentRange);//МД текущего диапазона
		counterRate = Process_CalculateCountingRate(currentRange);//Скорость счёта текущего диапазона
	//проверяем ДУ для переключения диапазона
	
	if(autoMode){
		switch(currentRange)
		{
			case 1:
			{
				if(counterRate >= MainSettings.range1to2Value){
					Process_Stop();
					Protocol_SetRangeValue(2);
					Process_Start();
				}
			}break;
			
			case 2:
			{
				if(counterRate >= MainSettings.range2to3Value){
					Process_Stop();
					Protocol_SetRangeValue(3);
					Process_Start();
				}
				if(counterRate < MainSettings.range2to1Value)
				{
					Process_Stop();
					Protocol_SetRangeValue(1);
					Process_Start();
				}
			}break;
			
			case 3:
			{
				if(counterRate >= MainSettings.range3maxValue)
					Process_Stop();
				if(counterRate < MainSettings.range3to2Value)
				{
					Process_Stop();
					Protocol_SetRangeValue(2);
					Process_Start();
				}
			}break;
		}
	}

	//динамическая размерность движка справедлива только для 1 диапазона
	if(currentRange == 1)
	{
		if(counterRate < 3){
			sliderMax = 30;
			
			if(dynamicMode == 2){
				sliderCounter = 30;
				//memset(sliderQuery,0,sizeof(float) * SLIDER_SIZE);
			}
			
			dynamicMode = 1;
		}
		else if(counterRate >= 3 && counterRate < 10){
			sliderMax = 10;
			
			if(dynamicMode == 3){
					sliderCounter = 10;
					//memset(sliderQuery,0,sizeof(float) * SLIDER_SIZE);
			}
			
			dynamicMode = 2;
		}
		else if(counterRate >= 10) {
			sliderMax = 2;
			dynamicMode = 3;
		}
	}
	
	if(sliderCounter<sliderMax){
		sliderQuery[sliderCounter] = currentDose;
		sliderCounter++;
	}
	
	else{
		memcpy(&sliderQuery[0],&sliderQuery[1],sizeof(float) * (sliderMax - 1));
		sliderQuery[sliderMax - 1] = currentDose;
	}
	
	for(i = 0;i<sliderMax;i++)
		averageDose += sliderQuery[i];
	
	
	sliderCounter ? averageDose = averageDose / sliderMax : currentDose;
	
	/*if(sliderCounter == 0)
		averageDose = currentDose;
	else
		averageDose = averageDose / sliderCounter;
	*/
	
	//формирование пакета данных для отправки
	pSendPackage = (uint8*)malloc(sizeof(uint8) * 32);
	
		memset(pSendPackage,0,sizeof(uint8) * 32);
	
	if(pSendPackage)
	{
		pSendPackage[0] = 0x40;
		pSendPackage[1] = E_NEWDATA_RESPONSE;
		
	
		Protocol_HeaderDataMsg(pSendPackage + 8,counterRate);
		Protocol_CounterMsg(pSendPackage + 16,sensorCnt);
		Protocol_DoseRateMsg(pSendPackage + 24,averageDose);	

			CAN_SendDataPackage(TX_DATA,pSendPackage,sizeof(uint8) * 32);
	}
	free(pSendPackage);
	
	
	/*
	Protocol_ResponseDataPackaje(E_NEWDATA_RESPONSE);
	Protocol_HeaderDataMsg(counterRate);
	Protocol_CounterMsg(sensorCnt);
	Protocol_DoseRateMsg(averageDose);
	*/
	memset(sensorCnt,0,sizeof(sensorCnt));//обнуляем массив счётчиков
	

		
}


float	Process_CalculateCurrentDoseRate(uint8	rangeValue)
{
	uint16	i = 0;
	float	numerator = 0;
	float	denominator = 0;
	float	dose = 0;
	
	float leftPart = 0;
	float rightPart = 0;
	
	switch(rangeValue)
	{
		case 1:
		{
			/*sensorCnt[0] = 3;
			sensorCnt[1] = 3;
			sensorCnt[2] = 3;
			sensorCnt[3] = 3;*/
			
			
			leftPart = 1.0f / (MainSettings.fSensCounters[0] * 4 * 1);
			
			for(i = 0;i<4;i++)
			{
				numerator = sensorCnt[i] * MainSettings.correctionFactors[i];
				denominator = 1.0f - (sensorCnt[i] * MainSettings.mDeadTime[0]);
				
				rightPart = rightPart + (numerator / denominator);
			}	
		}break;
		
		case 2:
		{
			leftPart = 1.0f / (MainSettings.fSensCounters[1] * 2 * 1);
			
			for(i = 4;i<6;i++)
			{
				numerator = sensorCnt[i] * MainSettings.correctionFactors[i];
				denominator = 1.0f - (sensorCnt[i] * MainSettings.mDeadTime[1]);
				
				rightPart = rightPart + (numerator / denominator);
			}
		}break;
		
		case 3:
		{
			leftPart = 1.0f / (MainSettings.fSensCounters[2] * 2 * 1);
			
			for(i = 6;i<8;i++)
			{
				numerator = sensorCnt[i] * MainSettings.correctionFactors[i];
				denominator = 1.0f - (sensorCnt[i] * MainSettings.mDeadTime[2]);
				
				rightPart = rightPart + (numerator / denominator);
			}	
		}break;
	}
	
	dose = leftPart * rightPart;
	
	return dose;
}
float	Process_CalculateCountingRate(uint8 rangeValue)
{
	float	cntRate = 0;
	uint16	i = 0;
	
	switch(rangeValue)
	{
		case 1:
		{
			for(i = 0;i<4;i++)
				cntRate += sensorCnt[i];
			cntRate = cntRate / 4.0f;
		}break;
		
		case 2:
		{
			for(i = 4;i<6;i++)
				cntRate += sensorCnt[i];
			cntRate = cntRate / 2.0f;
		}break;
		
		case 3:
		{
			for(i = 6;i<8;i++)
				cntRate += sensorCnt[i];
			cntRate = cntRate / 2.0f;
		}break;
		
		default:break;
	}
	
	return cntRate;
}


void Process_Start()
{
	//устанавливаем стартовые параметры
	
	uint8	range = Protocol_GetRangeValue();

	sliderCounter = 0;
	
	memset(sensorCnt,0,sizeof(sensorCnt));
	memset(sliderQuery,0,sizeof(float) * SLIDER_SIZE);
	/*
	Алгоритм запуска измерений
	
		- проверяем текущий заданных диапазон. Если это 1,2,3 - то запускаем измерение в соответствующем диапазоне
		-
	*/

	
	switch(range)
	{
		case 0:
		{			
			Protocol_SetRangeValue(3);
			Process_Start();
		}break;
		
		case 1:	
		{
			//Выключаем ШИМ 2 3
			T8R = 0;
			T1R = 0;
			//Выключаем ключи 2 3
			KEY2_L = 0; KEY2_H = 0;
			KEY3_L = 0; KEY3_H = 0;
			//Включаем ШИМ 1
			T7R = 1;
			
			
		}break;
		
		case 2:	
		{
			//Выключаем ШИМ 1 3
			T7R = 0;
			T1R = 0;
			//Выключаем ключи 2 3
			KEY1_L = 0; KEY1_H = 0;
			KEY3_L = 0; KEY3_H = 0;
			//Включаем ШИМ 1
			T8R = 1;		

			
		}break;
		
		case 3:
		{
			//Выключаем ШИМ 1 2
			T7R = 0;
			T8R = 0;
			//Выключаем ключи 1 2
			KEY1_L = 0; KEY1_H = 0;
			KEY2_L = 0; KEY2_H = 0;
			//Включаем ШИМ 3
			T1R = 1;	
			
		}break;
	}
	
	T0 = 0x0000;
	
		
	//TO_DO ожидаем нарастание высокого напряжения
	
	T0R = 1;
}

void Process_Stop()
{
	T7R = 0;
	T8R = 0;
	T1R = 0;
	
		KEY1_L = 0; KEY1_H = 0;
		KEY2_L = 0; KEY2_H = 0;
		KEY3_L = 0; KEY3_H = 0;
	
	T0R = 0;
}

