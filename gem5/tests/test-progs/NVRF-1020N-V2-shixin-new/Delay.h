
#include <stdio.h>
#include <REG51.H>



#ifndef __DELAY_H__
#define __DELAY_H__


#define uint  unsigned short int
#define uchar unsigned char



extern void wait(int m,int n);
extern void DelayUS(uchar ten_us);
extern void DelayMS(uint iMs);
extern void myNOP();


#endif