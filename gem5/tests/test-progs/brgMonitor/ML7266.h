#include <stdio.h>
#include "reg51.h"
#include "LCD.h"
#include "SCI.h"


#ifndef __ML7266_H__
#define __ML7266_H__

extern void ML_TX(unsigned char *payload);
extern void ML_TX_init();
//internal function
extern void ML_Reset();
extern void ML_SetExAdr();
extern void ML_SetAssPmt();
extern void ML_SetShoAdr();
extern void ML_SetPanID();
extern void ML_SetRxOn();
extern void ML_SetPowMode();
extern void ML_Start();
#endif