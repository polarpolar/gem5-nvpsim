#include<reg51.h>

sfr P3CTRL=0xb1;
sbit TIP=P0^1;


void main()
{

EA = 0;		// Set CPU status: interrupt disable
P2 = 0xff; 	// flag  

//////			reset by wishbone    ////////
P1 = 0x20;	//00100000
P2 = 0xfe;		// flag
P1 = 0x00;

/////   		 I2C write						///////

//PRER set to 100kHz
P1 = 0xD0;		//1101000,PRERlo
P0 = 0x1F;		//PRERl0=1F
P1 = 0x54;		//Read status
P0 = 0x00;
P1 = 0xD1;		//1101000,PRERhi

//CTR, enable
P1 = 0x54;
P0 = 0x80;
P1 = 0xD2;		//11010010,CTR	
P3CTRL=0xC0;    //switch P3[7],P3[6] to i2c port

//TXR: transfer slave adr
P1 = 0x54;
P0 = 0x20;
P1 = 0xD3;		//11010011,TXR

//CR:start, write
P1 = 0x54;

P0 = 0x90;
P1 = 0xD4;		//11010100,CR

//SR: wait tranfer over
P1 = 0x54;
P2 = 0x00;		//must insert nop, P0 change after 2 cyc of P1
while(TIP);
P2 = 0x7e;		// flag



//TXR: tranfer slave mem adr
P1 = 0x54;
P0 = 0x01;
P1 = 0xD3;		

//CR: write
P1 = 0x54;
P0 = 0x10;
P1 = 0xD4;		

//SR: wait tranfer over
P1 = 0x54;
P2 = 0x00;
while(TIP);
P2 = 0xfe;			// flag



//TXR: tranfer slave data
P1 = 0x54;
P0 = 0x11;
P1 = 0xD3;		

//CR, write
P1 = 0x54;
P0 = 0x10;
P1 = 0xD4;		

//SR: wait tranfer over
P1 = 0x54;
P2 = 0x00;
while(TIP);
P2 = 0x7e;		// flag



//TXR:tranfer slave data(adr inc)
P1 = 0x54;
P0 = 0x22;
P1 = 0xD3;		

//CR: write
P1 = 0x54;
P0 = 0x10;

P1 = 0xD4;		

//SR: wait tranfer over
P1 = 0x54;
P2 = 0x00;
while(TIP);
P2 = 0xfe;			// flag



//TXR: transfer slave data
P1 = 0x54;
P0 = 0x33;
P1 = 0xD3;		

//CR: stop, write
P1 = 0x54;
P0 = 0x50;
P1 = 0xD4;		

//SR: wait tranfer over
P1 = 0x54;
P2 = 0x00;
while(TIP);
P2 = 0xfc;			// flag

while(1);
}

