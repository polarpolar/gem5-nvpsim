#include "Delay.h"
#include "LCD.h"

#ifndef __SCI_H__
#define __SCI_H__

#define SNOP 0x00
#define SXOSCON 0x01
#define SRXON 0x03
#define STXON 0x04
#define SRFOFF 0x06
#define SXOSCOFF 0x07
#define SFLUSHRX 0x08
#define SFLUSHTX 0x09
#define STXONCCA 0x05
#define MDMCTRL0 0x11
#define FSCTRL 0x18
#define IOCFG0 0x1C
#define TXFIFO_RAM 0x00
#define TXFIFO_BANK 0x00
#define TXFIFO 0x3E
#define RXFIFO 0x3F
#define MDMCTRL0 0x11
#define RSSI 0x13
#define SECCTRL0 0x19
#define RXCTRL1 0x17
#define CHANNEL 0x1A

			
sbit REGPD= P2^3; 
sbit SINTN= P2^2;	
sbit SCEN=	P2^1;
sbit RESET=	P2^0;
sbit busy= P0^2;

extern void ResetWishbone();
extern void SCI_Init();
extern unsigned char send_char(unsigned char c);

#endif
