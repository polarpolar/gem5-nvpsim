#include "KXTJ9_1005.h"
#include "I2C.h"
#include "LCD.h"

//uint KXTJ9_SLAVE_ADDR = 0x0F;

void KXTJ9_prep(){
	I2C_rst();
	I2C_initial();
	//KXTJ9_ADDR = 0;

}

void KXTJ9_Init(){
	
	uchar cmd[2];
	
	KXTJ9_prep();
	
  //Initialize CTRL_REG1								  //  ctrl-reg1 0000-0000
	//cmd[0] = 0x00;
	//I2C_w(KXTJ9_SLAVE_ADDR, KXTJ9_CTRL_REG1, cmd, 1);
	
	cmd[0] = 0x80;
	I2C_w(KXTJ9_SLAVE_ADDR, KXTJ9_CTRL_REG2, cmd, 1);  	// ctrl-reg2 1000-0000
  //DelayMS(100);
	do{	
		I2C_r(KXTJ9_SLAVE_ADDR, KXTJ9_CTRL_REG2, cmd, 1);
		//DelayMS(50);
		//LCD_show(cmd[0]);	
	  //DelayMS(25);
	}while(cmd[0]&0x80) ;								  //??
	

	//Initialize CTRL_REG1								  //  ctrl-reg1 0000-0000
	cmd[0] = 0x00;
	I2C_w(KXTJ9_SLAVE_ADDR, KXTJ9_CTRL_REG1, cmd, 1);
		
	//Initialize CTRL_REG2								  //  ctrl-reg2 0000-0000
	cmd[0] = 0x00;
	I2C_w(KXTJ9_SLAVE_ADDR, KXTJ9_CTRL_REG2, cmd, 1);

	//Initialize INT_CTRL1				  //   int-ctrl1,2 0001-0000,0011-1111
	cmd[0]=0x38;
	I2C_w(KXTJ9_SLAVE_ADDR, KXTJ9_INT_CTRL_REG1, cmd, 1);
	
	//Initialize INT_CTRL1				  //   int-ctrl1,2 0001-0000,0011-1111
	cmd[0]=0x06;
	I2C_w(KXTJ9_SLAVE_ADDR, KXTJ9_DATA_CTRL_REG1, cmd, 1);
	
  I2C_r(KXTJ9_SLAVE_ADDR, KXTJ9_DATA_CTRL_REG1, cmd, 2);
	LCD_show(0x3001);DelayMS(5000);
	LCD_show(cmd[0]);DelayMS(5000);
	
	//cmd[0]=0x22;
	//I2C_r(KXTJ9_SLAVE_ADDR, KXTJ9_INT_CTRL_REG1, cmd, 1);
  //DelayMS(50);
	//LCD_show(cmd[0]);	
	//DelayMS(25);
	/*
	do{	
		I2C_r(KXTJ9_SLAVE_ADDR, KXTJ9_CTRL_REG1, cmd, 2);
		DelayMS(25);
	}while((cmd[0]&0x80)&(cmd[1]&0x3f)) ;
		*/
		
	//cmd[0] = 0x3f;
	//I2C_w(KXTJ9_SLAVE_ADDR, KXTJ9_INT_CTRL_REG2, cmd, 1);
}

void KXTJ9_Set_Resolution(uint high){
	
	uchar cmd;
	
	KXTJ9_prep();
	 
	//Load CTRL_REG1 to cmd									  
	I2C_r(KXTJ9_SLAVE_ADDR, KXTJ9_CTRL_REG1, &cmd, 1);

	if(cmd & 0x80){			//check whether PC1==1
		cmd &= (~0x80);		//Set PC1 = 0					  //cmd = 0xxx-xxxx
		I2C_w(KXTJ9_SLAVE_ADDR, KXTJ9_CTRL_REG1, &cmd, 1);
	}
	
	if(high)
		cmd |= 0x40;
	else
		cmd &= (~0x40);

	I2C_w(KXTJ9_SLAVE_ADDR, KXTJ9_CTRL_REG1, &cmd, 1);

}


void KXTJ9_Set_Range(uint range){

	uchar cmd;
	
	//Load CTRL_REG1 to cmd
	I2C_r(KXTJ9_SLAVE_ADDR, KXTJ9_CTRL_REG1, &cmd, 1);

	if(cmd & 0x80){			//if PC1==1, Set PC1 = 0
		cmd &= (~0x80);		
		I2C_w(KXTJ9_SLAVE_ADDR, KXTJ9_CTRL_REG1, &cmd, 1);
	}

	cmd &= 0xE7; 	//0x11100111
	if(range==4)
		cmd |= 0x08;
	else if(range == 8)
		cmd |= 0x10;


	cmd |= 0x20;
	I2C_w(KXTJ9_SLAVE_ADDR, KXTJ9_CTRL_REG1, &cmd, 1);

}  
void KXTJ9_Get_Acceleration(int* a_x,int* a_y, int* a_z){
	
	uchar cmd;

	uchar data_out[2];

	//Load CTRL_REG1 to cmd
	I2C_r(KXTJ9_SLAVE_ADDR, KXTJ9_CTRL_REG1, &cmd, 1);
	LCD_show(0x2000);DelayMS(5000);
	LCD_show(cmd);DelayMS(5000);

	if((cmd & 0x80) == 0){			//if PC1==0; Set PC1 = 1
		cmd |= 0x80;		
		I2C_w(KXTJ9_SLAVE_ADDR, KXTJ9_CTRL_REG1, &cmd, 1);
	}

	//----------------------TEST---------------
	I2C_r(KXTJ9_SLAVE_ADDR, KXTJ9_CTRL_REG1, &cmd, 1);
	LCD_show(0x1000);DelayMS(5000);
	LCD_show(cmd);DelayMS(5000);
	
	I2C_r(KXTJ9_SLAVE_ADDR, KXTJ9_STATUS_REG, &cmd, 1);
	LCD_show(0x1001);DelayMS(5000);
	LCD_show(cmd);DelayMS(5000);
	
	I2C_r(KXTJ9_SLAVE_ADDR, KXTJ9_DATA_CTRL_REG1, &cmd, 1);
	LCD_show(0x1002);DelayMS(5000);
	LCD_show(cmd);DelayMS(5000);
	
	/*I2C_r(KXTJ9_SLAVE_ADDR, KXTJ9_DCST_RESP, &cmd, 1);
	LCD_show(0x1000);DelayMS(5000);
	LCD_show(cmd);DelayMS(5000);
	
	I2C_r(KXTJ9_SLAVE_ADDR, KXTJ9_WHO_AM_I, &cmd, 1);
	LCD_show(0x1001);DelayMS(5000);
	LCD_show(cmd);DelayMS(5000);
	
	I2C_r(KXTJ9_SLAVE_ADDR, KXTJ9_DATA_CTRL_REG1, &cmd, 1);
	LCD_show(0x1002);DelayMS(5000);
	LCD_show(cmd);DelayMS(5000);
	
	I2C_r(KXTJ9_SLAVE_ADDR, KXTJ9_STATUS_REG, &cmd, 1);
	LCD_show(0x1003);DelayMS(5000);
	LCD_show(cmd);DelayMS(5000);
	
	I2C_r(KXTJ9_SLAVE_ADDR, KXTJ9_INT_CTRL_REG1, &cmd, 1);
	LCD_show(0x1004);DelayMS(5000);
	LCD_show(cmd);DelayMS(5000);
	
	I2C_r(KXTJ9_SLAVE_ADDR, KXTJ9_CTRL_REG2, &cmd, 1);
	LCD_show(0x1005);DelayMS(5000);
	LCD_show(cmd);DelayMS(5000);*/
	
	//read acceleration of x-axis
	I2C_r(KXTJ9_SLAVE_ADDR, KXTJ9_XOUT_L, data_out, 2);
	LCD_show(0x2001);DelayMS(5000);
	LCD_show(data_out[1]);DelayMS(5000);
	LCD_show(data_out[0]);DelayMS(5000);
	*a_x = (data_out[1] << 4) | (data_out[0] >> 4)	;
	
	//read acceleration of y-axis
	I2C_r(KXTJ9_SLAVE_ADDR, KXTJ9_YOUT_L, data_out, 2);
	LCD_show(0x2002);DelayMS(5000);
	LCD_show(data_out[1]);DelayMS(5000);
	LCD_show(data_out[0]);DelayMS(5000);
	*a_y = (data_out[1] << 4) | (data_out[0] >> 4)	;

	//read acceleration of z-axis
	I2C_r(KXTJ9_SLAVE_ADDR, KXTJ9_ZOUT_L, data_out, 2);
	LCD_show(0x2003);DelayMS(5000);
	LCD_show(data_out[1]);DelayMS(5000);
	LCD_show(data_out[0]);DelayMS(5000);
	*a_z = (data_out[1] << 4) | (data_out[0] >> 4)	;
	
	
	/*
		//read acceleration of x-axis
	I2C_r(KXTJ9_SLAVE_ADDR, 0x07, a_x, 1);
	//*a_x = data_out;//(data_out[1] << 4) | (data_out[0] >> 4)	;
	LCD_show(*a_x);DelayMS(5000);
	
	//read acceleration of y-axis
	I2C_r(KXTJ9_SLAVE_ADDR, 0x09, a_y, 1);
	//*a_y = data_out;//(data_out[1] << 4) | (data_out[0] >> 4)	;
  LCD_show(*a_y);DelayMS(5000);
	
	//read acceleration of z-axis
	I2C_r(KXTJ9_SLAVE_ADDR, 0x0B, a_z, 1);
	//*a_z = data_out;//(data_out[1] << 4) | (data_out[0] >> 4)	;
  LCD_show(*a_z);DelayMS(5000);*/

}	
