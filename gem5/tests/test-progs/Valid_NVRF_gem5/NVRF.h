#include <stdio.h>
#include <REG51.H>


#ifndef __NVRF_H__
#define __NVRF_H__

#define STANDBY1   0x33
#define STANDBY2   0x55
#define FLYWHEEL   0xCC
#define DEEP_SLEEP 0xAA

#define ANum 2	
#define PNum 2+6*ANum
extern char xdata payload[PNum+2]; //={0x56,0x86,0x4a,0x73,0x3d,0x45,0x01,0x30,0x90,0xf5};

sfr PMINST=0x91;
sfr PMHSB=0x92;
sfr PMMSB=0x93;
sfr PMLSB=0x94;

extern void nvrf_config();
extern void drill_buffer();
extern void nvrf_start(char length, char nvrf_choice);
//extern int delay1000();
extern void wait1(int i, int j);
extern int PMU_conf(char a, char b, char c, char d);

//unsigned char code temp[]={26,27,27,26,25,25,24,26,27};

#endif