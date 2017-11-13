#include "ADS1000.h"
//#include "LCD.h"


void ADS1000_Init(void){

	uchar tmp[2];
	I2C_rst();
	I2C_initial();

	tmp[0] = Continuous_Conv_Cmd;
	
	I2C_write(TEMP_SENSOR_ADR, tmp, 1); //one-shot
}

uint ADS1000_Read_Temp(){

	uchar tmp[3];
	uint temp_result;
	double cal;

	I2C_rst();
	I2C_initial();
	
	I2C_read(TEMP_SENSOR_ADR, tmp, 3);
	
	temp_result = ((tmp[0] & 0x00ff)<< 8) | (tmp[1] & 0x00ff);		  // 8bit & 16bit?
	cal = temp_result;
	temp_result = 300 - (cal - 1074.9 ) * 10000 / 6433; 
	 
	return temp_result;

}

