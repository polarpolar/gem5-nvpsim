#include "LCD.h"
#include "Delay.h"
//-----------------------------------------------------------------------------------------
//函数名称：Ht1621Wr_Data() 
//功    能：写数据函数,cnt为传送数据位数,数据传送为低位在前 
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
//函数名称：void Ht1621WrCmd(uchar Cmd) 
//功能描述: HT1621命令写入函数 
//参数说明：Cmd为写入命令数据  
//说    明：写入命令标识位100 
//-----------------------------------------------------------------------------------------
void Ht1621WrCmd(uchar Cmd){
	CS = 0; 
	Ht1621Wr_Data(0x80,4);          //写入命令标志100 
	Ht1621Wr_Data(Cmd,8);           //写入命令数据 
	CS = 1;
	DelayUS(1);
}
/* 
//-----------------------------------------------------------------------------------------
//函数名称：void Ht1621WrOneData(uchar Addr,uchar Data) 
//功能描述: HT1621在指定地址写入数据函数 
//参数说明：Addr为写入初始地址，Data为写入数据  
//说    明：因为HT1621的数据位4位，所以实际写入数据为参数的后4位 
//-----------------------------------------------------------------------------------------
void Ht1621WrOneData(uchar Addr,uchar Data){ 
	CS = 0;
  Ht1621Wr_Data(0xa0,3);                 	//写入数据标志101 
  Ht1621Wr_Data(Addr<<2,6);          			//写入地址数据 
  Ht1621Wr_Data(Data<<4,4);          			//写入数据 
  CS = 1;
  /////DelayUS(1);
}
*/ 
//-----------------------------------------------------------------------------------------
//函数名称：void Ht1621_Init(void) 
//功能描述: HT1621初始化 
//说    明：初始化后，液晶屏所有字段均显示 
//-----------------------------------------------------------------------------------------
void Ht1621_Init(void){ 
  // HT1621_CS=1; 
  // HT1621_WR=1; 
  // HT1621_DAT=1; 
  // DelayMS(20);                     //延时使LCD工作电压稳定 
   Ht1621WrCmd(BIAS); 
   Ht1621WrCmd(RC256);              	//使用内部振荡器 
   Ht1621WrCmd(SYSDIS); 
   Ht1621WrCmd(WDTDIS1); 
   Ht1621WrCmd(SYSEN); 
   Ht1621WrCmd(LCDON); 
}
/* 
//-----------------------------------------------------------------------------------------
//函数名称：void Ht1621WrAllData() 
//功能描述: HT1621连续写入方式函数 
//说    明：HT1621的数据位4位，此处每次数据为8位，写入数据总数按8位计算 
//-----------------------------------------------------------------------------------------
void Ht1621WrAllData(void){ 
 	uchar i;  
 	CS = 0;
  Ht1621Wr_Data(0xa0,3);          	//写入数据标志101 
  Ht1621Wr_Data(0x00<<2,6);         //写入地址数据 
	for(i=0;i<4;i++){ 
		Ht1621Wr_Data(0x00,8);         	//写入数据 
	} 
	CS = 1;
	/////DelayUS(1);
} 
*/

void LCD_show(uint solar_time){
	uchar i; 
	CS = 0;
	Ht1621Wr_Data(0xa0,3);          	//写入数据标志101
	Ht1621Wr_Data(0x00<<2,6);         //写入地址数据 
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
