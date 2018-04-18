#include <REG51.H>
#include <stdio.h>
#include "I2C.h"
#include "Delay.h"
#include "LCD.h"
#include "KXTJ9.h"
#include "tmp100.h"
#include "NVRF.h"

#define ANum 10	// Acc data in one packet
#define PeriNum 3	// Peri. number

void main()
{
	int 		acc_x,acc_y,acc_z;
	int 		tmp;
	int 		PacketCount = 0;	// added
	uint8_t * 	payload;		// added
	unsigned char 	i;

	printf("Program Start.\n");
	
	while(PacketCount < 1)
	{	
		// initialize payloads
		for(i=0;i<PNum;i++)
			payload[i]=0xAAAA;
		PacketCount++;
		
		// initialization of NVRF module
		nvrf_config();		// NVRF module
		
		// initialization of Temp Sensor
		tmpInit();		//initialization of tmp100 		
		tmpSetRes(3);		//Set resolution of tmp100: 12bits
		
		// data read from Temp Sensor
		tmpGetTmpCont(&tmp);
		payload[0]=(uchar)((tmp&0xFF00)>>8);
		payload[1]=(uchar)(tmp&0x00FF);
		
		// initialization of ACM Sensor		
		KXTJ9_Init();			// ACM Sensor	
		KXTJ9_Set_Resolution(1);	// high res
		KXTJ9_Set_Range(2);		// -2g -- +2g	
		
		// data read from ACM Sensor
		for(i=1;i<=ANum;i++)
		{
			KXTJ9_Get_Acceleration(&acc_x, &acc_y, &acc_z);
			payload[6*i-4]=(uchar)((acc_x&0xFF00)>>8);
			payload[6*i-3]=(uchar)(acc_x&0x00FF);
			payload[6*i-2]=(uchar)((acc_y&0xFF00)>>8);
			payload[6*i-1]=(uchar)(acc_y&0x00FF);
			payload[6*i]=(uchar)((acc_z&0xFF00)>>8);
			payload[6*i+1]=(uchar)(acc_z&0x00FF);
		}

		// data transmission via RF module			
		drill_buffer();
		DelayMS(100);
		nvrf_start(PNum,0);
		DelayMS(200);
	}	
}	