
#include <REG51.H>


#ifndef __LCD_H__
#define __LCD_H__


#define uint  unsigned short int
#define uchar unsigned char
sbit CLK=P2^6;
sbit DAT=P2^5;
sbit CS=P2^7;
#define  BIAS     0x52             	//0b1000 0101 0010  1/3duty 4com 
#define  SYSDIS   0X00             	//0b1000 0000 0000  关振系统荡器和LCD偏压发生器 
#define  SYSEN    0X02             	//0b1000 0000 0010 	打开系统振荡器 
#define  LCDOFF   0X04             	//0b1000 0000 0100  关LCD偏压 
#define  LCDON    0X06             	//0b1000 0000 0110  打开LCD偏压 
#define  XTAL     0x28            	//0b1000 0010 1000 	外部接时钟 
#define  RC256    0X30             	//0b1000 0011 0000  内部时钟 
#define  TONEON   0X12             	//0b1000 0001 0010  打开声音输出 
#define  TONEOFF  0X10             	//0b1000 0001 0000 	关闭声音输出 
#define  WDTDIS1  0X0A            	//0b1000 0000 1010  禁止看门狗

//fged_abcDP
#define LCD_CLR 	0x00		//0000_0000
#define LCD_MARK1 0x58		//0101_1000
#define LCD_MARK2 0x40		//0100_0000
#define LCD_MARK3 0xCC		//1100_1100
#define LCD_0 		0xBE		//1011_1110
#define LCD_1 		0x06		//0000_0110
#define LCD_2 		0x7C		//0111_1100
#define LCD_3 		0x5E		//0101_1110
#define LCD_4 		0xC6		//1100_0110
#define LCD_5 		0xDA		//1101_1010
#define LCD_6 		0xFA		//1111_1010
#define LCD_7 		0x0E		//0000_1110
#define LCD_8 		0xFE		//1111_1110
#define LCD_9 		0xDE		//1101_1110
#define LCD_A 		0xEE		//1110_1110
#define LCD_B 		0xF2		//1111_0010
#define LCD_C 		0xB8		//1011_1000
#define LCD_D 		0x76		//0111_0110
#define LCD_E 		0xF8		//1111_1000
#define LCD_F 		0xE8		//1110_1000
#define LCD_ERROR 	0x5E		//0101_1110
#define	LCD_c		0x70		//0111_0000
#define	LCD_DP		0x01

//P3_0: DATA
//P3_1: CLK
//p3_2: CS

extern void Ht1621Wr_Data(uchar Data,uchar cnt);
extern void Ht1621WrCmd(uchar Cmd);
extern void Ht1621WrOneData(uchar Addr,uchar Data);
extern void Ht1621_Init(void);
extern void Ht1621WrAllData(void);

extern void LCD_show(uint solar_time);

extern void LCD_show_solar_time(uint solar_time);
extern void LCD_show_temper(uint temp);
extern void LCD_show_bin(uchar data_bin);
extern void Ht1621OFF(void);
extern void Ht1621ON(void);

#endif
