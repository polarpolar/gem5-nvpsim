#include "ML7266.h"

static unsigned char callback_length;
static unsigned char callback_PC;
static unsigned char callback_payload;

void ML_TX(unsigned char *payload){
	do{
	//unsigned char appDataReq[] = {0x0F,0x40,SrcAdrMode,DstAdrMode,macPANId,DevshortAddr,0x05,0x00,0x00,0x00,0x00,0x00,0x01,0x01};
		unsigned char len=payload[0]+10+3;
		int j; 
		SCEN=0;
		send_char(0x02);
		//cmd length
		send_char(len);
		send_char(0x40);
		send_char(0x00);
		send_char(0x02);
		send_char(0xAD);													   
		send_char(0xDE);
		send_char(0xAA);
		send_char(0xAA);
		//msdulength
		send_char((payload[0]+3));
		//data
		send_char(0x88);
		send_char(0x88);
		send_char(0x88);
		for(j=1;j<=payload[0];j++)
		{
			send_char(payload[j]);
		}
		//send_char(0x99);
		//tail
		send_char(0x01);
		send_char(0x01);
		SCEN=1;wait(1,2000);
		if(SINTN==0)
		{
			SCEN=0;
			send_char(0x01);
			callback_length=send_char(0x00);
			callback_PC=send_char(0x00);
			send_char(0x00);
			callback_payload=send_char(0x00);
			SCEN=1;wait(1,500);
			//LCD_show(callback_PC);wait(1,10000);
			//LCD_show(callback_payload);wait(1,10000);
			callback_payload=0x00;
		}
	}while(callback_payload!=0x00);
}

void ML_Reset(){
	do{
	//MLME-RESET.request
	//unsigned char appCmd_1[] = {0x02,0x54,0x01};
		SCEN=0;
		send_char(0x02);
		send_char(0x02);
		send_char(0x54);
		send_char(0x01);
		SCEN=1;wait(1,500);
		if(SINTN==0)
		{
			SCEN=0;
			send_char(0x01);
			callback_length=send_char(0x00);
			callback_PC=send_char(0x00);
			callback_payload=send_char(0x00);
			SCEN=1;wait(1,500);
			//LCD_show(callback_PC);wait(1,10000);
			//LCD_show(callback_payload);wait(1,10000);
		}
	}while(callback_payload!=0x00);
}

void ML_SetExAdr(){
	do{
	//MLME-SET.request( aExtendedAddress )
	// unsigned char appCmd_2[] = {0x0A,0x5B,0x6F,macCoorExtAddr};	
		SCEN=0;
		send_char(0x02);
		send_char(0x0A);
		send_char(0x5B);
		send_char(0x6F);
		send_char(0xAA);
		send_char(0xAA);
		send_char(0xAA);
		send_char(0xAA);
		send_char(0xAA);
		send_char(0xAA);
		send_char(0xAA);
		send_char(0xAA);
		SCEN=1;wait(1,500);
		if(SINTN==0)
		{
			SCEN=0;
			send_char(0x01);
			callback_length=send_char(0x00);
			send_char(0x00);
			callback_payload=send_char(0x00);
			callback_PC=send_char(0x00);
			SCEN=1;wait(1,500);
			//LCD_show(callback_PC);wait(1,10000);
			//LCD_show(callback_payload);wait(1,10000);
		}
	}while(callback_payload!=0x00);
}

void ML_SetAssPmt(){
	do{
	//MLME-SET.request( macAssociationPermit )
	// unsigned char appCmd_3[] = {0x03,0x5B,0x41,0x01};	
		SCEN=0;
		send_char(0x02);
		send_char(0x03);
		send_char(0x5B);
		send_char(0x41);
		send_char(0x01);
		SCEN=1;wait(1,500);
		if(SINTN==0)
		{
			SCEN=0;
			send_char(0x01);
			callback_length=send_char(0x00);
			send_char(0x00);
			callback_payload=send_char(0x00);
			callback_PC=send_char(0x00);
			SCEN=1;wait(1,500);
			//LCD_show(callback_PC);wait(1,10000);
			//LCD_show(callback_payload);wait(1,10000);
		}
	}while(callback_payload!=0x00);	
}

void ML_SetShoAdr(){
	do{
	//MLME-SET.request( macShortAddress )
	// unsigned char appCmd_4[] = {0x04,0x5B,0x53,macCoorShortAddr};
		SCEN=0;
		send_char(0x02);
		send_char(0x04);
		send_char(0x5B);
		send_char(0x53);
		send_char(0xEF);
		send_char(0xBE);
		SCEN=1;wait(1,500);
		if(SINTN==0)
		{
			SCEN=0;
			send_char(0x01);
			callback_length=send_char(0x00);
			send_char(0x00);
			callback_payload=send_char(0x00);
			callback_PC=send_char(0x00);
			SCEN=1;wait(1,500);
			//LCD_show(callback_PC);wait(1,10000);
			//LCD_show(callback_payload);wait(1,10000);
		}
	}while(callback_payload!=0x00);	
}

void ML_SetPanID(){
	do{
	//MLME-SET.request( macPANId )
	// unsigned char appCmd_5[] = {0x04,0x5B,0x50,macPADId};
		SCEN=0;
		send_char(0x02);
		send_char(0x04);
		send_char(0x5B);
		send_char(0x50);
		send_char(0xAD);
		send_char(0xDE);
		SCEN=1;wait(1,500);
		if(SINTN==0)
		{
			SCEN=0;
			send_char(0x01);
			callback_length=send_char(0x00);
			send_char(0x00);
			callback_payload=send_char(0x00);
			callback_PC=send_char(0x00);
			SCEN=1;wait(1,500);
			//LCD_show(callback_PC);wait(1,10000);
			//LCD_show(callback_payload);wait(1,10000);
		}
	}while(callback_payload!=0x00);	
}

void ML_SetRxOn(){
	do{
	//MLME-SET.request( macRxOnWhenIdle = TRUE )
	// unsigned char appCmd_6[] = {0x03,0x5B,0x52,0x01};
		SCEN=0;
		send_char(0x02);
		send_char(0x03);
		send_char(0x5B);
		send_char(0x52);
		send_char(0x01);
		SCEN=1;wait(1,500);
		if(SINTN==0)
		{
			SCEN=0;
			send_char(0x01);
			callback_length=send_char(0x00);
			send_char(0x00);
			callback_payload=send_char(0x00);
			callback_PC=send_char(0x00);
			SCEN=1;wait(1,500);
			//LCD_show(callback_PC);wait(1,10000);
			//LCD_show(callback_payload);wait(1,10000);
		}
	}while(callback_payload!=0x00);	
}

void ML_SetPowMode(){
	do{
	//MLME-SET.request( Powersavemode )
	// unsigned char appCmd_7[] = {0x06,0x5D,macPANId,Char_Channel,0xFF01};
		SCEN=0;
		send_char(0x02);
		send_char(0x03);
		send_char(0x5B);
		send_char(0x80);
		send_char(0x00);
		SCEN=1;wait(1,500);
		if(SINTN==0)
		{
			SCEN=0;
			send_char(0x01);
			callback_length=send_char(0x00);
			send_char(0x00);
			callback_payload=send_char(0x00);
			callback_PC=send_char(0x00);
			SCEN=1;wait(1,500);
			//LCD_show(callback_PC);wait(1,10000);
			//LCD_show(callback_payload);wait(1,10000);
		}
	}while(callback_payload!=0x00);	
}

void ML_Start(){
	do{
	//MLME-START.request( non-beacon )
	// unsigned char appCmd_7[] = {0x06,0x5D,macPANId,Char_Channel,0xFF01};
		SCEN=0;
		send_char(0x02);
		send_char(0x06);
		send_char(0x5D);
		send_char(0xAD);
		send_char(0xDE);
		send_char(0x1A);
		send_char(0xFF);
		send_char(0x01);
		SCEN=1;wait(1,500);
		if(SINTN==0)
		{
			SCEN=0;
			send_char(0x01);
			callback_length=send_char(0x00);
			callback_PC=send_char(0x00);
			callback_payload=send_char(0x00);
			SCEN=1;wait(1,500);
			//LCD_show(callback_PC);wait(1,10000);
			//LCD_show(callback_payload);wait(1,10000);
		}
	}while(callback_payload!=0x00);	
}
void ML_TX_init(){
	ML_Reset();
				  LCD_show(0xBBB0);
			    DelayMS(100);
	ML_SetExAdr();
				  LCD_show(0xBBB1);
			    DelayMS(100);	
	ML_SetAssPmt();
				  LCD_show(0xBBB2);
			    DelayMS(100);	
	ML_SetShoAdr();
				  LCD_show(0xBBB3);
			    DelayMS(100);	
	ML_SetPanID();
				  LCD_show(0xBBB4);
			    DelayMS(100);	
	ML_SetRxOn();
				  LCD_show(0xBBB5);
			    DelayMS(100);	
	ML_SetPowMode();
				  LCD_show(0xBBB6);
			    DelayMS(100);	
	ML_Start();
				  LCD_show(0xBBB7);
			    DelayMS(100);	
}
