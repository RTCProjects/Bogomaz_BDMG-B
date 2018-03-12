#include "main.h"
#include "can.h"
#include "flash.h"
#include "settings.h"
#include "protocol.h"
#include "process.h"
#include "system.h"
#include "math.h"

uint8 	malloc_mempool[0x1000];	//32Б динамической памяти 

sbit P9_0 = P9^0;
void main(void)
{

  WDTIN = 1;
	WDTPRE = 1;

	CAN_Init();


		init_mempool(&malloc_mempool, sizeof(malloc_mempool));
		
		SETTINGS_Init();		

		
		Process_Initializetion();
		Protocol_Initialization();
		
	P2 = 0x0000;
	DP2 = 0x0000;
	
	P7 |= 0x0000;
	DP7 |= 0x00F0;
	
	P9 |= 0x0010;//drv off
	DP9 |= 0x0033;
	

		
	T01CON = 0x0106;
	T0REL = 0x6769;
	T1REL = 0x0000;
	
	T0IC = 0x006A;
	T1IC = 0x0069;
	
	T78CON = 0x0101;
	T7IC = 0x006B;
	T8IC = 0x006C;
	

	CC28IC = 0x007F;	//75
	CC30IC = 0x007B;	//76
	CC4IC = 0x0077;		//77
	
	
	T7R = 0;//PWM1
	T8R = 0;//PWM2
	T1R = 0;//PWM3
	
	T0R = 0;//MainProc 1 sec

	

	//P7.4 T7
	/*
		Параметр влияет на длительность импульса выключения HV питания. 
		
		Увеличение параметра уевеличивает длительность импульса
		Уменьшение параметра уменьшает длительность импульса
		
			Цена деления - прибл. 0.8us
	*/
	CC28 = 0xFB88;
	CCM7 |= 0x0006;
	//P7.6 T8
	CC30 = 0xFB88;
	CCM7 |= 0x0F00; 
	
	//P9.0 T1
	CC4 = 0xFB88;
	CCM1 |= 0x000F;
	
	
	//настройка внешних входов прерываний
	EXICON = 0x5555; // все входы по положительному фронту
	EXISEL0 = 0;	//стандартный вход P2.X
	EXISEL1 = 0;
	
	IEN = 1;
	
	//проверка на перезагрузку WDT
	if(WDTR)Protocol_WDTRestartMsg(); 
	
	//проверка на перезапуск
	if(SWR)Protocol_StartMsg();
	
		while(1)
		{
			_srvwdt_();
		}
}


