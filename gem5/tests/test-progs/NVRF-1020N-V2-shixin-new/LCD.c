#include "LCD.h"
#include "Delay.h"
//-----------------------------------------------------------------------------------------
//�������ƣ�Ht1621Wr_Data() 
//��    �ܣ�д���ݺ���,cntΪ��������λ��,���ݴ���Ϊ��λ��ǰ 
//-----------------------------------------------------------------------------------------

volatile int ttt;

void Ht1621Wr_Data(uchar Data,uchar cnt){ 
	uchar i; 
  for (i=0;i<cnt;i++){  
  	CLK = 0;   	//CLK->L
		ttt=0;
		
  	if((Data & 0x80)==0x80){
  		DAT = 1;		//DATA->H
  	}
  	else{
  		DAT = 0;	//DATA->L
  	}
  	/////DelayUS(1);
		ttt=1;
		CLK = 1; 		//CLK->H
		ttt=2;
  	/////DelayUS(1);
		Data<<=1;  
		ttt=3;
  } 
} 
//-----------------------------------------------------------------------------------------
//�������ƣ�void Ht1621WrCmd(uchar Cmd) 
//��������: HT1621����д�뺯�� 
//����˵����CmdΪд����������  
//˵    ����д�������ʶλ100 
//-----------------------------------------------------------------------------------------
void Ht1621WrCmd(uchar Cmd){
	CS = 0; 
	Ht1621Wr_Data(0x80,4);          //д�������־100 
	Ht1621Wr_Data(Cmd,8);           //д���������� 
	CS = 1;
	DelayUS(1);
}
/* 
//-----------------------------------------------------------------------------------------
//�������ƣ�void Ht1621WrOneData(uchar Addr,uchar Data) 
//��������: HT1621��ָ����ַд�����ݺ��� 
//����˵����AddrΪд���ʼ��ַ��DataΪд������  
//˵    ������ΪHT1621������λ4λ������ʵ��д������Ϊ�����ĺ�4λ 
//-----------------------------------------------------------------------------------------
void Ht1621WrOneData(uchar Addr,uchar Data){ 
	CS = 0;
  Ht1621Wr_Data(0xa0,3);                 	//д�����ݱ�־101 
  Ht1621Wr_Data(Addr<<2,6);          			//д���ַ���� 
  Ht1621Wr_Data(Data<<4,4);          			//д������ 
  CS = 1;
  /////DelayUS(1);
}
*/ 
//-----------------------------------------------------------------------------------------
//�������ƣ�void Ht1621_Init(void) 
//��������: HT1621��ʼ�� 
//˵    ������ʼ����Һ���������ֶξ���ʾ 
//-----------------------------------------------------------------------------------------
void Ht1621_Init(void){ 
  // HT1621_CS=1; 
  // HT1621_WR=1; 
  // HT1621_DAT=1; 
  // DelayMS(20);                     //��ʱʹLCD������ѹ�ȶ� 
   Ht1621WrCmd(BIAS); 
   Ht1621WrCmd(RC256);              	//ʹ���ڲ����� 
   Ht1621WrCmd(SYSDIS); 
   Ht1621WrCmd(WDTDIS1); 
   Ht1621WrCmd(SYSEN); 
   Ht1621WrCmd(LCDON); 
}
/* 
//-----------------------------------------------------------------------------------------
//�������ƣ�void Ht1621WrAllData() 
//��������: HT1621����д�뷽ʽ���� 
//˵    ����HT1621������λ4λ���˴�ÿ������Ϊ8λ��д������������8λ���� 
//-----------------------------------------------------------------------------------------
void Ht1621WrAllData(void){ 
 	uchar i;  
 	CS = 0;
  Ht1621Wr_Data(0xa0,3);          	//д�����ݱ�־101 
  Ht1621Wr_Data(0x00<<2,6);         //д���ַ���� 
	for(i=0;i<4;i++){ 
		Ht1621Wr_Data(0x00,8);         	//д������ 
	} 
	CS = 1;
	/////DelayUS(1);
} 
*/

void LCD_show(uint solar_time){
	uchar i; 
	CS = 0;
	Ht1621Wr_Data(0xa0,3);          	//д�����ݱ�־101
	Ht1621Wr_Data(0x00<<2,6);         //д���ַ���� 
	for(i=0;i<4;i++){ 
		switch((solar_time&0xF000)>>12){
			case 0x00:Ht1621Wr_Data(LCD_0,8);break;
			case 0x01:Ht1621Wr_Data(LCD_1,8);break;
			case 0x02:Ht1621Wr_Data(LCD_2,8);break;
			case 0x03:Ht1621Wr_Data(LCD_3,8);break;
			case 0x04:Ht1621Wr_Data(LCD_4,8);break;
			case 0x05:Ht1621Wr_Data(LCD_5,8);break;
			case 0x06:Ht1621Wr_Data(LCD_6,8);break;
			case 0x07:Ht1621Wr_Data(LCD_7,8);break;
			case 0x08:Ht1621Wr_Data(LCD_8,8);break;
			case 0x09:Ht1621Wr_Data(LCD_9,8);break;
			case 0x0a:Ht1621Wr_Data(LCD_A,8);break;
			case 0x0b:Ht1621Wr_Data(LCD_B,8);break;
			case 0x0c:Ht1621Wr_Data(LCD_C,8);break;
			case 0x0d:Ht1621Wr_Data(LCD_D,8);break;
			case 0x0e:Ht1621Wr_Data(LCD_E,8);break;
			case 0x0f:Ht1621Wr_Data(LCD_F,8);break;
			default: Ht1621Wr_Data(LCD_ERROR,8);break;
		}
		solar_time = solar_time<<4;        
	} 
	CS = 1;
	/////DelayUS(1);
}
/*
void Ht1621OFF(void){
	CS = 1;
	/////DelayUS(1);
}
void Ht1621ON(void){
	CS = 0;
}
*/
