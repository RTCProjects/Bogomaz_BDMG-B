#include "settings.h"
#include "flash.h"

tSettings	MainSettings;

void	SETTINGS_Init()
{

	if(!SETTINGS_Load())
	{
		SETTINGS_Default();
		SETTINGS_Save();

	}
}

void	SETTINGS_Default()
{
	int i = 0;

	MainSettings.pulsePwrPeriod = 100;	
	MainSettings.pulseDuration = 100;
	MainSettings.modeValue = 1;
	//MainSettings.rangeValue = 0;
	
	for(i = 0;i<RANGES_COUNT;i++){
		MainSettings.mSliderSize[i] = 10;
		
	}
	MainSettings.mDeadTime[0] = 0.0004f;
	MainSettings.mDeadTime[1] = 0.0002f;
	MainSettings.mDeadTime[2] = 0.0002f;
	
	MainSettings.fSensCounters[0] = 1405;
	MainSettings.fSensCounters[1] = 383;
	MainSettings.fSensCounters[2] = 1.26f;	
	
	MainSettings.range1to2Value = 1780;
	MainSettings.range2to3Value = 4350;
	MainSettings.range3maxValue = 3600;
	MainSettings.range3to2Value = 45;
	MainSettings.range2to1Value = 850;
	
	for(i = 0;i<FACTORS_COUNT;i++)
		MainSettings.correctionFactors[i] = 1.5f;
	
	//MainSettings.timeSpan = 10;
}

void 	SETTINGS_Save()
{
	int i = 0,addr_counter = 0;
	
	 uint16  *pSettings = (tSettings*)&MainSettings;
	 uint16 sizeSettings = sizeof(MainSettings);
	
	 uint16	chkSum = 0;
	
	 for(i = 0;i<sizeSettings >> 1;i++)
			chkSum += *(pSettings + i);
	
	FLASH_EraseSector(0x22000);
	FLASH_FSRControl();
	
	FLASH_WriteWord(0x22000,chkSum);
	FLASH_FSRControl();
	
	for(i = 0;i<sizeSettings;i+=2)
	{
		FLASH_WriteWord(0x22002 + i,*(pSettings + addr_counter));
		FLASH_FSRControl();
		

		addr_counter++;
	}
}
uint8 SETTINGS_Load()
{
	int i = 0;
	unsigned int far *addressChkSum = (unsigned int far *) 0x022000;
	unsigned int far *addressData = (unsigned int far *) 0x022002;
	
	uint16	*pSettings = (tSettings*)&MainSettings;
	uint16	dataSize = sizeof(MainSettings);
	uint16  chkSum = 0;
	
	for(i = 0;i<dataSize >> 1;i++){
		chkSum += *(addressData + i);

	}

	if(chkSum != *addressChkSum)
		return 0;
	
	
	for(i = 0;i<dataSize >> 1;i++){
		*(pSettings + i) = *(addressData + i);
		
	}
	return 1;
}
