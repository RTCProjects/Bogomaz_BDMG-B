#include "can.h"
#include "settings.h"
#include "protocol.h"
#include "system.h"

static uint8	txEnable = 0;

void CAN_Interrupt0() interrupt 0x54	
{
	txEnable = 1;
	
//	uint16	currentID = ((CAN_Message_16[0].MOARH >> 2) & 0x07FF);
//	Protocol_TransmitMsgCallback(currentID);
		
	
}

void CAN_Interrupt1() interrupt 0x55		//индекс 0x00X
{
	uint16 regID = ((CAN_Message_16[3].MOARH >> 2) & 0x000F);
	uint8 *msgData = (uint8*)malloc(sizeof(uint8) * 8);
	if(msgData)
	{
		*((uint16*)&msgData[0]) = CAN_Message_16[3].MODATALL;
		*((uint16*)&msgData[2]) = CAN_Message_16[3].MODATALH;
		*((uint16*)&msgData[4]) = CAN_Message_16[3].MODATAHL;
		*((uint16*)&msgData[6]) = CAN_Message_16[3].MODATAHH;

	Protocol_ReceiveMsgCallback(regID,msgData);

	free(msgData);
	}
	
}

void CAN_Interrupt2() interrupt 0x56			//индекс 0x20X
{
/*
	uint16 regID = ((CAN_Message_16[4].MOARH >> 2) & 0x07FF);
	uint8 msgData[8];

		*((uint16*)&msgData[0]) = CAN_Message_16[4].MODATALL;
		*((uint16*)&msgData[2]) = CAN_Message_16[4].MODATALH;
		*((uint16*)&msgData[4]) = CAN_Message_16[4].MODATAHL;
		*((uint16*)&msgData[6]) = CAN_Message_16[4].MODATAHH;
*/

}

 
void CAN_Init()
{
	txEnable = 1;

	DP9 |= 0x000B;
	ALTSEL0P9 |= 0x0008;
	ALTSEL1P9 |= 0x0008;
	
	
	CAN_IC0 = 0x005C;	//72Xh
	CAN_IC1 = 0x0060;	//00Xh
	CAN_IC2 = 0x0074;	//20Xh
	
	NCR0  = 0x0041;		//разрешаем вносить изменения в конфигурацию
	NPCR0 = 0x0003;		//Выбрали P9.2 для приема в CAN узел 0
	NBTR0 = 0x2344;		//скорость узла 500kb/sec при тактировании 16Мгц
	NCR0  = 0x0000;		//Запрет после реконфигурации
	
	/*
		Список областей сообщений
	
		На приём
			3. 00Хh - аварийное сообщение для CAN
	    2. 20Хh - сообщение с данными от БДГП-С
		На передачу
		  1. 000 - CAN сообщение с настраиваемым ID
			2. 722 - данные (прерывание)
			3. 280 - передача старт/стоп
	*/
	//передача
	//000 - универсальный вар.
	CAN_Message_16[0].MOCTRH = 0x0E08;
	CAN_Message_16[0].MOCTRL = 0x0000;
	CAN_Message_16[0].MOIPRL = 0x0000;	/*выбор линии прерываний для передачи - 0*/
  CAN_Message_16[0].MOFCRH = 0x0802; // DLC = 8, разрешить прерывание на передачу 
	 
	
	//72Xh
	/*CAN_Message_16[1].MOCTRH = 0x0080;
  CAN_Message_16[1].MOCTRL = 0x0000;
  CAN_Message_16[1].MOIPRL = 0x0000;	// выбор линии прерываний по приему - 0
	CAN_Message_16[1].MOFCRH = 0x0801;	// разрешить прерывание на прием
	

	CAN_Message_16[2].MOCTRH = 0x0E08;
	CAN_Message_16[2].MOCTRL = 0x0000;
	CAN_Message_16[2].MOIPRL = 0x0000;	//выбор линии прерываний для передачи - 0
  CAN_Message_16[2].MOFCRH = 0x0800; // DLC = 8, запретить прерывание на передачу */
	

	//32X
	CAN_Message_16[3].MOCTRH = 0x0080;
  CAN_Message_16[3].MOCTRL = 0x0000;
  CAN_Message_16[3].MOIPRL = 0x0001;	//выбор линии прерываний по приему - 1
	CAN_Message_16[3].MOFCRH = 0x0801;	//разрешить прерываение на прием
	
	//20X
	CAN_Message_16[4].MOCTRH = 0x0080;
  CAN_Message_16[4].MOCTRL = 0x0000;
  CAN_Message_16[4].MOIPRL = 0x0002;	// выбор линии прерываний по приему - 2
	CAN_Message_16[4].MOFCRH = 0x0801;	//разрешить прерываение на прием

	System_Delay(10);
	
	CAN_Message_16[0].MOARH = 0x8000;	 // идентификатор 000(XXX)h - динамически ИД
  CAN_Message_16[0].MOAMRH = 0x3FFF; //	маска идентификатора	
		
	CAN_Message_16[1].MOARH = 0x9C80;	 // идентификатор 72Xh (X - 2,3,4)
  CAN_Message_16[1].MOAMRH = 0x3FE3; //	маска идентификатора		
	
	CAN_Message_16[2].MOARH = 0x8A00;	 // идентификатор 280h
  CAN_Message_16[2].MOAMRH = 0x3FFF; //	маска идентификатора		
	
	CAN_Message_16[3].MOARH = 0x8C80;	 // идентификатор 32Xh
  CAN_Message_16[3].MOAMRH = 0x3FE3; //	маска идентификатора	(00X, где X - 0,1,2,3,4)
	
	CAN_Message_16[4].MOARH = 0x8800;	 // идентификатор 20Xh	(00X, где X - 0,1,2,3,4)
  CAN_Message_16[4].MOAMRH = 0x3FE3; //	маска идентификатора	

	System_Delay(10);
	
	PANCTR_H = 0x0100;				 // message object 0 -> List 1
  PANCTR = 0x0002;				   
   _nop_(); _nop_(); _nop_(); _nop_(); _nop_(); _nop_();
	 
	 
	PANCTR_H = 0x0101;				 //	message object 1 -> List 1
  PANCTR = 0x0002;			    	 
   _nop_(); _nop_(); _nop_(); _nop_(); _nop_(); _nop_(); 
	 
	 
	PANCTR_H = 0x0102;				 //	message object 2 -> List 1
  PANCTR = 0x0002;			    	 
   _nop_(); _nop_(); _nop_(); _nop_(); _nop_(); _nop_(); 	
	 
	 
	PANCTR_H = 0x0103;				 //	message object 3 -> List 1
  PANCTR = 0x0002;			    	 
   _nop_(); _nop_(); _nop_(); _nop_(); _nop_(); _nop_(); 	 


	PANCTR_H = 0x0104;				 //	message object 4 -> List 1
  PANCTR = 0x0002;			    	 
   _nop_(); _nop_(); _nop_(); _nop_(); _nop_(); _nop_(); 	 


	CAN_Message_16[0].MOCTRH = 0x0020; 
	CAN_Message_16[0].MOCTRL = 0x0000;
	
	CAN_Message_16[1].MOCTRH = 0x0020; 
	CAN_Message_16[1].MOCTRL = 0x0000;
	
	CAN_Message_16[2].MOCTRH = 0x0020; 
	CAN_Message_16[2].MOCTRL = 0x0000;
	
	CAN_Message_16[3].MOCTRH = 0x0020; 
	CAN_Message_16[3].MOCTRL = 0x0000;
	
	CAN_Message_16[4].MOCTRH = 0x0020; 
	CAN_Message_16[4].MOCTRL = 0x0000;	
	
}


void CAN_SendMessage(uint16 id,uint8 *pData,uint8 Len)
{ 
	uint8 *moData = 0;
	uint8	index = 0;

		/*while(txEnable == 0)
		{
			if( (NSR0 & 0x7) != 0){	//проверка acknolegment 
				//IMDB_SendCanTransmitError(0);
				break;
			}
		}*/
	
	CAN_Message_16[0].MOFCRH &= 0xF0FF;
	CAN_Message_16[0].MOFCRH |= (uint16) (Len<<8);
	
	CAN_Message_16[0].MOARH &= 0x8000;
	CAN_Message_16[0].MOARH |= (uint16)(id<<2);
	
	
	moData = (uint8*)&CAN_Message_16[0].MODATALL;
	
	for(index = 0;index<Len;index++)
	{
		*(moData + (index)) = *(pData + index);
	}
	
		txEnable = 0;
	
	CAN_Message_16[0].MOCTRH = 0x0100;	
	
	System_Delay(1000);
}
/*
void CAN_SendProtocolData(uint8 moNubmer,uint8	*pData, int packSize)
{
	int msgCnt = packSize / 8;
	int lastMsgSize = packSize % 8;
	int i,j = 0;
	
	uint8	arrData[8];
	
	for(i = 0;i < msgCnt;i++)
	{
		for(j = 0;j<8;j++)
			arrData[j] = *(pData + j + (i*8));
		
		CAN_SendMessage(moNubmer,arrData,8);
	}
}
*/
