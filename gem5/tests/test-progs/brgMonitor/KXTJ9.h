
#include "reg51.h"
#include <stdio.h>
#include "Delay.h"

#ifndef __KXTJ9_H__
#define __KXTJ9_H__


sbit KXTJ9_ADDR = P3^7; 

#define KXTJ9_SLAVE_ADDR 0x0F

#define KXTJ9_XOUT_L			0x06
#define KXTJ9_XOUT_H			0x07
#define KXTJ9_YOUT_L			0x08
#define KXTJ9_YOUT_H			0x09
#define KXTJ9_ZOUT_L			0x0A
#define KXTJ9_ZOUT_H			0x0B
#define KXTJ9_DCST_RESP			0x0C
#define KXTJ9_WHO_AM_I			0x0F
#define KXTJ9_INT_SOURCE1		0x16
#define KXTJ9_INT_SOURCE2		0x17
#define KXTJ9_STATUS_REG		0x18
#define KXTJ9_INT_REL			0x1A
#define KXTJ9_CTRL_REG1			0x1B
#define KXTJ9_CTRL_REG2			0x1D
#define KXTJ9_INT_CTRL_REG1		0x1E
#define KXTJ9_INT_CTRL_REG2		0x1F
#define KXTJ9_DATA_CTRL_REG1	0x21



extern void KXTJ9_Init();

extern void KXTJ9_Set_Resolution(uint high);

extern void KXTJ9_Set_Range(uint range);

extern void KXTJ9_Get_Acceleration(int* a_x,int* a_y, int* a_z);


#endif