/****************************************/
/* Configure Funcion for tmp100(head)	*/
/* Temperature Sensor					*/
/* Author: Wu Tongda					*/
/* Tsinghua EE NICS						*/
/* Final Time: 2014/04/17				*/
/****************************************/
#include "reg51.h"
#include "I2C.h"
#include "delay.h"
#include "stdio.h"

#ifndef __TMP100_H__
#define __TMP100_H__

#define TMP100_SLA_ADDR 0x48 	// ADDR0=0,ADDR1=0; addr=1001000
#define TMP100_TMP_ADDR 0x00	// Temperature Value
#define TMP100_CON_ADDR 0x80	// ConfigurationR
#define TMP100_TML_ADDR 0x02	// Temperature Low
#define TMP100_TMH_ADDR 0x03	// Temperature High


void tmpPre();
void tmpInit();
void tmpSetRes(uint res);
//void tmpSetRange(uint ran);
void tmpSetMode(uint mode);
//void tmpGetTmpShut(int *tmp);
void tmpGetTmpCont(int *tmp);
//void tmpGetTmp(int *tmp);

#endif