#include"I2C.h"
//#include "LCD.h"

void I2C_w(uchar sv_adr, uchar mem_adr, uchar dat[], uint cnt){ //slave adr 7 bit
	
	uint i;

	//////////////		TXR: transfer slave adr	//////////////////
	P1 = 0x54;
	P0 = (sv_adr<<1)&(~0x01);
	P1 = 0x73;		//01_1_10_011,TXR
	
	//CR:start, write
	P1 = 0x54;
	P0 = 0x90;
	P1 = 0x74;		//01_1_10_100,CR
	
	//SR: wait tranfer over
	P1 = 0x54;
	myNOP();		//must insert nop, P0 change after 2 cyc of P1
	while(TIP);
	
	
	///////////////   TXR: tranfer slave mem adr  ///////////////
	P1 = 0x54;
	P0 = mem_adr;
	P1 = 0x73;		//01_1_10_011,TXR
	
	//CR: write
	P1 = 0x54;
	P0 = 0x10;
	P1 = 0x74;		//01_1_10_100,CR
	
	//SR: wait tranfer over
	P1 = 0x54;
	myNOP();
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

void I2C_r(uchar sv_adr, uchar mem_adr, uchar dat[], uint cnt){
	uint i;
	/////////////   TXR: transfer slave adr   //////////////////
	P1 = 0x54;
	P0 = (sv_adr<<1)&(~0x01);
	P1 = 0x73;		//01_1_10_011,TXR
	
	//CR: start, write
	P1 = 0x54;
	P0 = 0x90;
	P1 = 0x74;		//01_1_10_100,CR
	
	//SR: wait tranfer over
	P1 = 0x54;
	myNOP();		
	while(TIP);
	

	////////////   TXR: slave mem adr   /////////////////////
	P1 = 0x54;
	P0 = mem_adr;
	P1 = 0x73;		//01_1_10_011,TXR
	
	//CR: write
	P1 = 0x54;
	P0 = 0x10;
	P1 = 0x74;		//01_1_10_100,CR
	
	//SR: wait tranfer over
	P1 = 0x54;
	myNOP();
	while(TIP);

	
	//////////////		TXR: slave adr restart		////////////////////
	P1 = 0x54;
	P0 = (sv_adr<<1)|(0x01);
	P1 = 0x73;		//01_1_10_011,TXR
	
	//CR: restart, read
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

void I2C_rst(){
	//////			reset by wishbone    ////////
	P1 = 0x50;	//01_0_11_000
}

void I2C_initial(){
	//PRER set to 100kHz
	P1 = 0x54;		//01_1_10_000,PRERlo
	P0 = 0x02;
	P1 = 0x70;
			
	P1 = 0x54;		
	P0 = 0x00;
	P1 = 0x71;		//01_1_10_001,PRERhi
	
	//CTR, enable
	P1 = 0x54;
	P0 = 0x80;
	P1 = 0x72;		//01_1_10_010,CTR	
}



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