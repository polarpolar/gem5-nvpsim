

#include <REG51.H>
#include <stdio.h>
#include "Delay.h"


#ifndef __I2C_H__
#define __I2C_H__


//sbit TIP = P0^1;
//sbit RXACK = P0^7;
//sbit TIP = P3^6;
//sbit RXACK = P3^7;
sfr P3CTRL=0xb1;
sbit TIP=P0^1;

#define uint  unsigned short int
#define uchar unsigned char

extern void I2C_w(uchar sv_adr, uchar mem_adr, uchar dat[], uint cnt);
extern void I2C_r(uchar sv_adr, uchar mem_adr, uchar dat[], uint cnt);

//extern void I2C_write(uchar sv_adr, uchar dat[],uint cnt);
//extern void I2C_read(uchar sv_adr, uchar dat[],uint cnt);

extern void I2C_rst();
extern void I2C_initial();


#endif