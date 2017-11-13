
#include <REG51.H>
#include <stdio.h>
#include "I2C.h"


#ifndef __ADS1000_H__
#define __ADS1000_H__


#define TEMP_SENSOR_ADR  0x90 


#define One_Time_Conv_Cmd  0x90;
#define Continuous_Conv_Cmd 0x80;



extern void ADS1000_Init(void);
extern uint ADS1000_Read_Temp();



#endif 