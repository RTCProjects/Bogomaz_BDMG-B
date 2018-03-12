#ifndef _SETTINGS_H
#define _SETTINGS_H

#include "main.h"

#define RANGES_COUNT	3
#define FACTORS_COUNT	8


typedef struct
{
	uint16	pulsePwrPeriod;		//период импульсного питания (мкс)
	uint16	pulseDuration;		//длительность импульсного питания (мкс)
	uint8		modeValue;				//вариант работы в режиме измерения
	//uint8		rangeValue;				//номер текущего диапазона
	uint8		mSliderSize[RANGES_COUNT];	//размеры движков
	float		mDeadTime[RANGES_COUNT];		//мертвое время
	float		fSensCounters[RANGES_COUNT];	//чувствительность для каждого диапазона
	//значения переходов между диапазонами
	uint16	range1to2Value;
	uint16	range2to3Value;
	uint16	range3maxValue;
	uint16	range3to2Value;
	uint16	range2to1Value;
	//поправочные коэфициенты
	float		correctionFactors[FACTORS_COUNT];
	
	//uint16	timeSpan;
}tSettings;

extern tSettings	MainSettings;

void		SETTINGS_Init(void);
void		SETTINGS_Default(void);
void 		SETTINGS_Save(void);
uint8  	SETTINGS_Load(void);

#endif
