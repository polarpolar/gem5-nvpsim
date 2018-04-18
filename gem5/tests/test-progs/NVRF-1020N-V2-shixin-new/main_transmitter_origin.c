#include <REG51.H>
#include <stdio.h>
#include "SCI.h"
#include "Delay.h"
#include "LCD.h"
#include "KXTJ9.h"
#include "ML7266.h"
#include "ADS1000.h"

#define ANum 10
	//unsigned char appCmd_1[] = {0x02,0x54,0x01};
	// unsigned char appCmd_2[] = {0x0A,0x5B,0x6F,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA};
	// unsigned char appCmd_3[] = {0x03,0x5B,0x41,0x01};
	// unsigned char appCmd_4[] = {0x04,0x5B,0x53,0xEF,0xBE};
	// unsigned char appCmd_5[] = {0x04,0x5B,0x50,0xAD,0xDE};
	// unsigned char appCmd_6[] = {0x03,0x5B,0x52,0x01};
	// unsigned char appCmd_7[] = {0x06,0x5D,0xAD,0xDE,0x1A,0xFF,0x01};
	// unsigned char appDataReq[] = {0x0F,0x40,0x00,0x02,0xAD,0xDE,0xAA,0xAA,0x05,0x00,0x00,0x00,0x00,0x00,0x01,0x01};
void main(){   
	unsigned char payload[1+2+6*ANum];
	int	 acc_x,acc_y,acc_z;
	unsigned char i;	
	unsigned char ini_cnt;
	int counter;


	
	Ht1621_Init();
	Ht1621WrAllData();
	LCD_show(0xFFFF);wait(1,1000);
	//ResetWishbone();//LCD_show(1);
	
	SCI_Init();	LCD_show(0x9999);wait(1,1000);
	ML_TX_init();LCD_show(0x8888);wait(1,1000);
	ADS1000_Init();LCD_show(0x7777);wait(1,1000);
				
	payload[0]=2+6*ANum;
	ini_cnt=50;
	counter=0;	
	while(1)
	{
		//initialization
		if(ini_cnt==50)
		{		
		Ht1621_Init();
		SCI_Init();
		ML_TX_init();

		KXTJ9_Init();				
		KXTJ9_Set_Resolution(1);	//high res
		KXTJ9_Set_Range(2);			//-2g -- +2g
		
		ADS1000_Init();

		ini_cnt=1;
		}
		payload[1]=(uchar)((counter&0xFF00)>>8);
		payload[2]=(uchar)(counter&0x00FF);
		
		for(i=1;i<=ANum;i++)
		{
				KXTJ9_Get_Acceleration(&acc_x, &acc_y, &acc_z);wait(1,100);
				payload[6*i-3]=(uchar)((acc_x&0xFF00)>>8);
				payload[6*i-2]=(uchar)(acc_x&0x00FF);
				payload[6*i-1]=(uchar)((acc_y&0xFF00)>>8);
				payload[6*i]=(uchar)(acc_y&0x00FF);
				payload[6*i+1]=(uchar)((acc_z&0xFF00)>>8);
				payload[6*i+2]=(uchar)(acc_z&0x00FF);
		}
		LCD_show(ADS1000_Read_Temp());
		//LCD_show(acc_x);								
		ML_TX(payload);
		ini_cnt++;
		counter++;				 		
	}	
}	