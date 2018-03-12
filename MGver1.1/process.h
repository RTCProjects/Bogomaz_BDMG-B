#ifndef _PROCESS_H
#define _PROCESS_H

#include "main.h"

/*
#define SENS_RANGE_1	1405
#define SENS_RANGE_2	383
#define SENS_RANGE_3	1.26f
*/

#define DEADTIME_KF_1		0.0004f
#define DEADTIME_KF_2		0.0002f
#define DEADTIME_KF_3		0.0002f

#define SLIDER_SIZE		30

#define RNG_1_MAX 	1780

#define RNG_2_MIN		850
#define RNG_2_MAX 	4350

#define RNG_3_MIN		45
#define RNG_3_MAX 	3600


sbit KEY1_L	= P7^4;
sbit KEY1_H	= P7^5;

sbit KEY2_L	= P7^6;
sbit KEY2_H	= P7^7;

sbit KEY3_L	= P9^0;
sbit KEY3_H	= P9^1;


typedef enum
{
	IMDB_INITIALIZATION 						= 0x00,
	IMDB_OPERATION_CONTROL					= 0x01,
	IMDB_FULLTIME_WORK							= 0x02,
	IMDB_SETTINGS_MODE		 					= 0x03,
	IMDB_SPECTR_ACCUMULATION	 			= 0x04,
	IMDB_CALIBRATION_I		 					= 0x05,
	IMDB_CALIBRATION_II		 					= 0x06,
	IMDB_CALIBRATION_III		 				= 0x07,
	IMDB_CALIBRATION_IV				 			= 0x08,
	IMDB_BACKGROUND_ACCUMULATION	 	= 0x09
}IMDB_Status;

typedef enum
{
	DEVICE_READY = 1,
	DEVICE_NOTREADY = 0
}Device_Status;




void	Process_Initializetion(void);
void 	Process_Start(void);
void 	Process_Stop(void);

float		Process_CalculateCurrentDoseRate(uint8	rangeValue);
float		Process_CalculateCountingRate(uint8 rangeValue);
#endif
