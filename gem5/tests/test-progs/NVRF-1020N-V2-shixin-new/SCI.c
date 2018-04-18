#include "SCI.h"

void ResetWishbone() {
	P1=0x38;					//复位总线
	wait(10,100);				//几毫秒
	P1=0x35;					//设置分频为最大
	wait(1,100);
	P0=0x00;								   
	P1=0x34;					//设置CRTL寄存器
	wait(1,100);
	P0=0x90;
	wait(1,100);
}

void SCI_Init(){
												//ML7266初始化方法
    /**************************/
	REGPD=0;
	SINTN=1;
	wait(1,100);								//等待近百毫秒
	RESET=0;									//异步复位
	wait(1,100);								//等待毫秒
	RESET=1;
	wait(1,100);								//芯片进入掉电状态，此时可以写读configuration registers和SXOSCON Command Strobe

	/**************************/
}

unsigned char send_char(unsigned char c){
	unsigned char receive;

	P1 = 0xC0; //11000000	// send P0's value to SPI Core 's Add : SPI_TX_0  and wait to be sent
 	P0 = c;  //load data tp P0

	P1 = 0x00; 
	P0 = 0x88;  //10001000	// load instruction to P0

	P1 = 0xC4;  //1100100	// send the instruction to SPI Core's Add : SPI_CTRL
	P0 = 0x8c;

	do{
		P1 = 0x44; //01000100	// load the CTRL register
	}
	while(busy);	// wait until finish
	
	P1=0x40;
	wait(1,1);
	receive=P0;
	return(receive);
}

/*unsigned char send_char(unsigned char c){
	unsigned char receive;

	P1 = 0x30;
 	P0 = c;  

	P1 = 0x00;
	P0 = 0x10;

	P1 = 0x34;
	P0 = 0x34;

	do{
		P1 = 0x14;
	}
	while(busy);	
	
	P1=0x10;
	wait(1,1);
	receive=P0;
	return(receive);
}*/
/*
void ReadRegister(unsigned char add) {
	
	add=0x40 | add;
	ResetWishbone();
	CSn=0;
	wait(1,100);
	send_char(add);//读寄存器，此后SI脚为无效状态
	wait(1,100);
	RSB=ByteRead;
	wait(1,100);
	send_char(0x00);
	wait(1,100);
	MSB=ByteRead;
	send_char(0x00);
	wait(1,100);
	CSn=1;
	wait(1,100);
	LSB=ByteRead;
	wait(1,100);
}

void WriteRegister(unsigned char add) {
	ResetWishbone();
	CSn=0;
	wait(1,100);
	send_char(add);//写寄存器
	wait(1,100);
	RSB=ByteRead;	
	wait(1,100);
	send_char(MSB);
	wait(1,100);
	send_char(LSB);
	wait(1,100);
	CSn=1;	
	wait(1,100);
}

void Oscillator(){//开启晶振
	while( (ByteRead&0x40)!=0x40){
		CSn=0;
		send_char(0x01);
		wait(1,100);	
		CSn=1;	
		wait(1,100);	
	}
}

void SET_Freq() {//设置发送频率
	ReadRegister(FSCTRL);
	MSB |= 0x01;
	LSB = 0x65+5*(CHANNEL-11);			
	WriteRegister(FSCTRL);
}

void SET_CCA_THR(unsigned char CCA_thr) {//设置CCA阈值
	
	CCA_thr=CCA_thr | 0x80;
	ReadRegister(RSSI);
	MSB=CCA_thr; //lsb 保持不变
	WriteRegister(MDMCTRL0);
	wait(1,100);
	ReadRegister(MDMCTRL0);
	wait(1,100);
}

void SET_CRC(unsigned char CRC) {
	CRC=CRC & 0x20;
	ReadRegister(MDMCTRL0);
	LSB=LSB | CRC;
	WriteRegister(MDMCTRL0);
	wait(1,100);
}

void OFF_AddressDecode() {//关闭地址译码
		//设置无地址认证
	ReadRegister(MDMCTRL0);
	MSB = MSB & 0xF7;
	WriteRegister(MDMCTRL0);
	wait(1,100);
}

void Command(unsigned char command){
	ResetWishbone();
	CSn=0;
	wait(1,100);
	send_char(command);
	wait(1,100);
	RSB=ByteRead;
	wait(1,100);
	CSn=1;
	wait(1,100);
}

void WriteTXFIFO(unsigned char length,unsigned int d) {

	ResetWishbone();
	CSn=0;
	wait(1,10);
	send_char(0x3E);//写FIFO
	wait(1,100);
	send_char(length);
	wait(1,100);
	send_char(d>>8);
	wait(1,100);
	send_char(d);
	wait(1,100);
	send_char(0x00);
	wait(1,100);
	send_char(0x00);
	wait(1,100);
	CSn=1;	
	wait(1,10);
}


void ReadRXFIFO(void) {
	ResetWishbone();
	CSn=0;
	wait(1,100);
	send_char(0x3F);//读FIFO
	RSB=ByteRead;
	wait(1,100);	
	CSn=1;	
	wait(1,100);
}

void ReadRam(unsigned char ramAdd,unsigned char bank) {
	ResetWishbone();
	ramAdd=ramAdd | 0x80;
	bank=(bank<<6);
	bank=bank | 0x20;
	CSn=0;
	wait(1,100);
	send_char(ramAdd);
	wait(1,100);
	RSB=ByteRead;
	wait(1,100);
	send_char(bank);
	wait(1,100);
	send_char(0x00);
	wait(1,100);
	CSn=1;	
	MSB=ByteRead;
	wait(1,100);
}
*/