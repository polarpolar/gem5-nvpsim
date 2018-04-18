#include"I2C.h"
//#include "LCD.h"

void I2C_w(uchar sv_adr, uchar mem_adr, uchar dat[], uint cnt){ //slave adr 7 bit
	
	uint i;

	//////////////		TXR: transfer slave adr	//////////////////
	P1 = 0x54;
	P0 = (sv_adr<<1)&(~0x01);
	P1 = 0xD3;		//11010011,TXR
	
	//CR:start, write
	P1 = 0x54;
	P0 = 0x90;
	P1 = 0xD4;		//11010100,CR
	
	//SR: wait tranfer over
	P1 = 0x54;
  //P2 = 0x00;		//must insert nop, P0 change after 2 cyc of P1
	myNOP();		//must insert nop, P0 change after 2 cyc of P1
  while(TIP);
  //P2 = 0x7e;		// flag
	
	
	///////////////   TXR: tranfer slave mem adr  ///////////////
	P1 = 0x54;
	P0 = mem_adr;
	P1 = 0xD3;		//11010011,TXR
	
	//CR: write
	P1 = 0x54;
	P0 = 0x10;
	P1 = 0xD4;		//11010100,CR
	
	//SR: wait tranfer over
	P1 = 0x54;
	//P2 = 0x00;
	myNOP();		//must insert nop, P0 change after 2 cyc of P1
	while(TIP);
	//P2 = 0xfe;			// flag
	
	
	//////////////		TXR: tranfer slave data		/////////////////
	for(i=0;i<cnt-1;i++){
		//TXR:tranfer slave data(adr inc)
		P1 = 0x54;
		P0 = dat[i];
		P1 = 0xD3;		//11010011,TXR
	
		//CR: write
		P1 = 0x54;
		P0 = 0x10;
		P1 = 0xD4;		//11010100,CR
	

		//SR: wait tranfer over
		P1 = 0x54;
		//P2 = 0x00;
		myNOP();		//must insert nop, P0 change after 2 cyc of P1
		while(TIP);
		//P2 = 0xfe;		// flag
	}
	
	//TXR: transfer slave data
	P1 = 0x54;
	P0 = dat[cnt-1];
	P1 = 0xD3;		//11010011,TXR
	
	//CR: stop, write
	P1 = 0x54;
	P0 = 0x50;
	P1 = 0xD4;		//11010100,CR
	
	//SR: wait tranfer over
	P1 = 0x54;
	//P2 = 0x00;
	myNOP();		//must insert nop, P0 change after 2 cyc of P1
	while(TIP);
	//P2 = 0xfc;			// flag
}

void I2C_r(uchar sv_adr, uchar mem_adr, uchar dat[], uint cnt){
	uint i;
	
  //PRER set to 400kHz
	//P1 = 0xD0;		//11010000,PRERlo
	//P0 = 0x07;		
	//P1 = 0x54;
	//P0 = 0x00;
	//P1 = 0xD1;		//11010001,PRERhi
	
	/////////////   TXR: transfer slave adr   //////////////////
	P1 = 0x54;
	P0 = (sv_adr<<1)&(~0x01);
	P1 = 0xD3;		//11010011,TXR
	
	//CR: start, write
	P1 = 0x54;
	P0 = 0x90;
	P1 = 0xD4;		//11010100,CR
	
	//SR: wait tranfer over
	P1 = 0x54;
	//P2 = 0x00;		
	myNOP();		//must insert nop, P0 change after 2 cyc of P1
	while(TIP);
	//P2 = 0xDc;		// flag
	

	////////////   TXR: slave mem adr   /////////////////////
	P1 = 0x54;
	P0 = mem_adr;
	P1 = 0xD3;		//11010011,TXR
	
	//CR: write
	P1 = 0x54;
	P0 = 0x10;
	P1 = 0xD4;		//11010100,CR
	
	//SR: wait tranfer over
	P1 = 0x54;
	//P2 = 0x00;
	myNOP();		//must insert nop, P0 change after 2 cyc of P1
	while(TIP);
	//P2 = 0xfc;		// flag

	
	//////////////		TXR: slave adr restart		////////////////////
	P1 = 0x54;
	P0 = (sv_adr<<1)|(0x01);
	P1 = 0xD3;		//11010011,TXR
	
	//CR: restart, read
	P1 = 0x54;
	P0 = 0x90;
	P1 = 0xD4;		//11010100,CR
	
	//SR: wait tranfer over
	P1 = 0x54;
	//P2 = 0x00;
	myNOP();		//must insert nop, P0 change after 2 cyc of P1
	while(TIP);
	//P2 = 0xDc;		// flag

	//////////////		Reading		/////////////////////
	for(i=0;i<cnt-1;i++){
//		//CR:read,ack
//		P1 = 0x54;
//		P0 = 0x20;
//		P1 = 0xD4;		
//	
//		//SR: wait tranfer over
//		P1 = 0x54;
//		P2 = 0x00;
//		while(TIP);
//		P2 = 0xfc;		// flag
//	
//		//RXR
//		P1 = 0x53;
//		P2 = 0x00;		// insert nop
//		if(P0!=0x11)	P2 = 0xee;		// flag
		
		//CR:read,ack
	P1 = 0x54;
	P0 = 0x20;
	P1 = 0xD4;		
	
	//SR: wait tranfer over
	P1 = 0x54;
	//P2 = 0x00;
	myNOP();		//must insert nop, P0 change after 2 cyc of P1
	while(TIP);
	//P2 = 0xfc;		// flag
	
	//RXR
	P1 = 0x53;
	myNOP();		//must insert nop, P0 change after 2 cyc of P1
	//P2 = 0x00;
	//if(P0!=0x11)	P2 = 0xee;		// flag

		dat[i] = P0;
	}
	
//	//CR:read,nack,stop
//	P1 = 0x54;
//	P0 = 0x68;
//	P1 = 0xD4;		
//	
//	//SR: wait tranfer over
//	P1 = 0x54;
//	P2 = 0x00;
//	while(TIP);
//	P2 = 0xfc;		// flag
//	
//	//RXR
//	P1 = 0x53;
//	P2 = 0x00;
//	if(P0!=0x33)	P2 = 0xee;			// flag
//	P2 = 0xf8;		// flag
	
		//CR:read,nack,stop
	P1 = 0x54;
	P0 = 0x68;
	P1 = 0xD4;		
	
	//SR: wait tranfer over
	P1 = 0x54;
	//P2 = 0x00;
	myNOP();		//must insert nop, P0 change after 2 cyc of P1
	while(TIP);
	//P2 = 0xfc;		// flag
	
	//RXR
	P1 = 0x53;
	myNOP();		//must insert nop, P0 change after 2 cyc of P1
	//P2 = 0x00;
	//if(P0!=0x33)	P2 = 0xee;			// flag
	//P2 = 0xf8;		// flag
	
	dat[cnt-1] = P0;

}

void I2C_rst(){
	EA = 0;		// Set CPU status: interrupt disable
  //P2 = 0xff; 	// flag  
	
	//////			reset by wishbone    ////////
	P1 = 0x20;	//00100000
	//P2 = 0xfe;		// flag
	P1 = 0x00;
}

void I2C_initial(){
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
}


/*
void I2C_write(uchar sv_adr, uchar dat[], uint cnt){ //slave adr 7 bit
	
	uint i;

	//////////////		TXR: transfer slave adr	//////////////////
	P1 = 0x54;
	P0 = (sv_adr) &(~0x01);
	P1 = 0x73;		//01_1_10_011,TXR
	
	//CR:start, write
	P1 = 0x54;
	P0 = 0x90;
	P1 = 0x74;		//01_1_10_100,CR
	
	//SR: wait tranfer over
	P1 = 0x54;
	myNOP();		//must insert nop, P0 change after 2 cyc of P1
	while(TIP);
	
	
	
	//////////////		TXR: tranfer slave data		/////////////////
	for(i=0;i<cnt-1;i++){
		//TXR:tranfer slave data(adr inc)
		P1 = 0x54;
		P0 = dat[i];
		P1 = 0x73;		//01_1_10_011,TXR
	
		//CR: write
		P1 = 0x54;
		P0 = 0x10;
		P1 = 0x74;		//01_1_10_100,CR
	
		//SR: wait tranfer over
		P1 = 0x54;
		myNOP();
		while(TIP);
	}
	
	//TXR: transfer slave data
	P1 = 0x54;
	P0 = dat[cnt-1];
	P1 = 0x73;		//01_1_10_011,TXR
	
	//CR: stop, write
	P1 = 0x54;
	P0 = 0x50;
	P1 = 0x74;		//01_1_10_100,CR
	
	//SR: wait tranfer over
	P1 = 0x54;
	myNOP();
	while(TIP);
}

void I2C_read(uchar sv_adr, uchar dat[], uint cnt){
	uint i;
	/////////////   TXR: transfer slave adr   //////////////////
	P1 = 0x54;
	P0 = (sv_adr)|(0x01);
	P1 = 0x73;		//01_1_10_011,TXR
	
	//CR: start, write
	P1 = 0x54;
	P0 = 0x90;
	P1 = 0x74;		//01_1_10_100,CR
	
	//SR: wait tranfer over
	P1 = 0x54;
	myNOP();		
	while(TIP);
	

	//////////////		Reading		/////////////////////
	for(i=0;i<cnt-1;i++){
		//CR:read,ack
		P1 = 0x54;
		P0 = 0x20;
		P1 = 0x74;		//01_1_10_100,CR
	
		//SR: wait tranfer over
		P1 = 0x54;
		myNOP();
		while(TIP);
	
		//RXR
		P1 = 0x53;
		myNOP();

		dat[i] = P0;
	}
	
	//CR:read,nack,stop
	P1 = 0x54;
	P0 = 0x68;
	P1 = 0x74;		//01_1_10_100,CR
	
	//SR: wait tranfer over
	P1 = 0x54;
	myNOP();
	while(TIP);
	
	//RXR
	P1 = 0x53;
	myNOP();
	
	dat[cnt-1] = P0;

} 
*/