/****************************************/
/* Configure Funcion for tmp100			*/
/* Temperature Sensor					*/
/* Author: Wu Tongda					*/
/* Tsinghua EE NICS						*/
/* Final Time: 2014/04/17				*/
/****************************************/
#include "tmp100.h"
#include "LCD.h"

/*****************************************/
/* Prepare of tmp100					 */
/*****************************************/
void tmpPre(){
	I2C_rst();
	I2C_initial();
	
	// Slave Address
	//ADDR0 = 0;
	//ADDR1 = 0;	
}

/*****************************************/
/* initialization of tmp100				 */
/* resolution: 12bits;					 */
/* range: -55~125;						 */
/* Continuous Mode.						 */
/*****************************************/
void tmpInit(){
	uchar cmd[2];	

	tmpPre();

	// Configuration
	cmd[0] = 0x60;	// 0110_0000 1ShutDown/0Continuous
	cmd[1] = 0x00;
	I2C_w(TMP100_SLA_ADDR, TMP100_CON_ADDR, cmd, 1);
	cmd[0] = 0xC9;
	cmd[1] = 0x00;
	I2C_w(TMP100_SLA_ADDR, TMP100_TML_ADDR, cmd, 2);
	cmd[0] = 0x7D;
	cmd[1] = 0x00;
	I2C_w(TMP100_SLA_ADDR, TMP100_TMH_ADDR, cmd, 2);
}

/****************************************/
/* Set resolution of tmp100				*/
/* resolution(4-steps): 				*/
/*	9bits(res=0),						*/
/*	10bits(res=1),						*/
/*	11bits(res=2),						*/
/*	12bits(res=3).						*/
/****************************************/
void tmpSetRes(uint res){
	uchar cmd;

	tmpPre();
 /*
	if(res>3||res<0){
		LCD_show(0xEAA0);
		DelayMS(1000); 
		return;
	}
 */
	I2C_r(TMP100_SLA_ADDR, TMP100_CON_ADDR, &cmd, 1);
	
	if(res==0)					// 9bits
		cmd = (cmd&0x9F)|0x00;
	else if(res==1)				// 10bits
		cmd = (cmd&0x9F)|0x20;
	else if(res==2)				// 11bits
		cmd = (cmd&0x9F)|0x40;
	else if(res==3)				// 12bits
		cmd = (cmd&0x9F)|0x60;

	I2C_w(TMP100_SLA_ADDR, TMP100_CON_ADDR, &cmd, 1);
}

/****************************************/
/* Set range of tmp100					*/
/* range(2-steps): 						*/
/*	ran=0: -25~85;						*/
/*	ran=1: -55~125.						*/
/****************************************/
/*
void tmpSetRange(uint ran){
	uchar cmd[2];

	tmpPre();
	
	// temperature low
	if(ran){
		cmd[0] = 0xC9;
		cmd[1] = 0x00;
	}else{
		cmd[0] = 0xE7;
		cmd[1] = 0x00;
	}
	I2C_w(TMP100_SLA_ADDR, TMP100_TML_ADDR, cmd, 2);
	
	// temperature high
	if(ran){
		cmd[0] = 0x7D;
		cmd[1] = 0x00;
	}else{
		cmd[0] = 0x55;
		cmd[1] = 0x00;
	}
	I2C_w(TMP100_SLA_ADDR, TMP100_TMH_ADDR, cmd, 2);	
}
*/

/****************************************/
/* Set Operation Mode				    */
/* Mode(2-steps): 						*/
/*	mode=0: Continuous Mode;			*/
/*	mode=1: ShutDown Mode.				*/
/****************************************/
/*
void tmpSetMode(uint mode){
	uchar cmd;

	tmpPre();

	I2C_r(TMP100_SLA_ADDR, TMP100_CON_ADDR, &cmd, 1);
	
	if(mode)  					// ShutDown Mode
		cmd = (cmd&0xFE)|0x01;
	else						// Continuous Mode
		cmd = (cmd&0xFE)|0x00;

	I2C_w(TMP100_SLA_ADDR, TMP100_CON_ADDR, &cmd, 1);		
}
*/

/****************************************/
/* Get temperature from tmp100			*/
/* Continuous Mode						*/
/****************************************/
void tmpGetTmpCont(int *tmp){
	uchar dataRead[2];
//	int *temp;

	// Read temperature
	//I2C_r(TMP100_SLA_ADDR, TMP100_TMP_ADDR, &dataRead, 2);
	I2C_r(TMP100_SLA_ADDR, TMP100_TMP_ADDR, &dataRead, 1);
	*tmp = dataRead[0];
	//LCD_show(*tmp);DelayMS(10000);
	//*temp = dataRead[1];
	//LCD_show(*temp>>4);DelayMS(10000);
	//*tmp = (*tmp<<4)|(*temp>>4);
	//*tmp=(*tmp<<8)>>8;
	*tmp=(*tmp<<8);
	//LCD_show(*tmp);DelayMS(1000);
}
/****************************************/
/* Get temperature from tmp100			*/
/* ShutDown Mode						*/
/****************************************/
/*
void tmpGetTmpShut(int *tmp){
	uchar cmd;
	uchar dataRead[2];
	int temp;

	tmpPre();

	// Set OS(Configuration^1) = 1
	I2C_r(TMP100_SLA_ADDR, TMP100_CON_ADDR, &cmd, 1);
	if(~(cmd&0x80) || ~(cmd&0x01))
		cmd |= 0x81;
	I2C_w(TMP100_SLA_ADDR, TMP100_CON_ADDR, &cmd, 1);
	wait(1,10);
	
	// Read temperature
	I2C_r(TMP100_SLA_ADDR, TMP100_TMP_ADDR, &dataRead, 2);
	*tmp = dataRead[0];
	//LCD_show(*tmp);DelayMS(1000);
	temp = dataRead[1];
	//LCD_show(*temp);DelayMS(1000);
	*tmp = (*tmp<<4)|(temp>>4);
	//LCD_show(*tmp);DelayMS(1000);
	
	// Set OS Back to 0
	cmd &= ~0x80;
	I2C_w(TMP100_SLA_ADDR, TMP100_CON_ADDR, &dataRead, 1);
}
*/
/****************************************/
/* Get temperature from tmp100			*/
/* Any Mode								*/
/****************************************/
/*
void tmpGetTmp(int *tmp){
	uchar mode;

	tmpPre();

	// Get Mode
	I2C_r(TMP100_SLA_ADDR, TMP100_CON_ADDR, &mode, 1);

	if(mode&(0x01))
		tmpGetTmpShut(tmp);
	else
		tmpGetTmpCont(tmp);	
}
*/