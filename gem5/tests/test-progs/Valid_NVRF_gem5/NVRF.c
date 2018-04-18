#include <NVRF.h>

char xdata payload[PNum+2] _at_ 0x000000;//={0x56,0x86,0x4a,0x73,0x3d,0x45,0x01,0x30,0x90,0xf5};
char code macCoorExtAddr[8]={0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA};
char code macCoorShortAddr[2]={0xEF,0xBE};
char code macPANId[2]={0xAD,0xDE};
char code Char_Channel=0x1A;
char code macDevShortAddr[2]={0xAA,0xAA};

void wait1(int i, int j)
{
	int m,n;
	for(m=0;m<i;m++)
		for(n=0;n<j;n++)
			{}
}

/*
int delay1000()
{
	int i;
	for(i=4000;i>0;i--){}
	return 0;
}	
*/

void nvrf_wr(unsigned char adr, unsigned char dat)
{
	P0=dat;
	P1=0x80|adr;
	P1=0x00;
}

//Write the configuration data to RF_CONF
void nvrf_config()
{
	int i=0;
	unsigned char adr=0x00;
	while(i<8)
	{
		nvrf_wr(adr,macCoorExtAddr[i]);
		adr=adr+0x01;
		i=i+1;
	}
	i=0;
	while(i<2)
	{
		nvrf_wr(adr,macCoorShortAddr[i]);
		adr=adr+0x01;
		i=i+1;
	}
	i=0;
	while(i<2)
	{
		nvrf_wr(adr,macPANId[i]);
		adr=adr+0x01;
		i=i+1;
	}	
	nvrf_wr(adr,Char_Channel);
	adr=adr+0x01;

	i=0;
	while(i<2)
	{
		nvrf_wr(adr,macDevShortAddr[i]);
		adr=adr+0x01;
		i=i+1;
	}
	//read the configuration data in RF_CONF
	/*
	i=0;
	adr=0x00;
	while(i<16)
	{
		P1=0x00|adr;
		wait1(1,1);
		//P3=P0;
		adr=adr+0x01;
		i=i+1;
	}
	P1=0x00;
	*/
	return;
}


//Fill dara for trasmission to RF_DATA.
void drill_buffer()
{
	int i=0;
	unsigned char adr=0x10;
	//fill payload[] into RF_DATA
	while(i<PNum)
	{
		 P0=payload[i];
		 P1=0x80|adr;
		 P1=0x00;
		 adr=adr+0x01;
		 i=i+1;
	}

	
	//read the data in RF_DATA
	/*
	i=0;
	adr=0x10;
	while(i<10)
	{
		 P1=0x00|adr;
		 wait1(1,1);
		 //P3=P0;
		 adr=adr+0x01;
		 i=i+1;
	}
	P1=0x00;
	*/
	return;
}

//nvrf transmission function. length: number of transimission bytes   nvrf_choice: 0: ML7266 1:ML7396 
void nvrf_start(char length, char nvrf_choice) 
{
	P1=0x0F;
	wait1(1,1);
	if(P0^6==1)
	{
		P0=0x40; // clear "DONE"
		P1=0x80|0x0F;
		P1=0x00;
	} 
	P0=(length<<2)+(nvrf_choice<<1)+0x01;	   //configure NVRF_CFG register
	P1=0x80|0x0F;
	P1=0x00;   
}


//Power management function	
int PMU_conf(char a, char b, char c, char d)
{
	PMHSB=b;
	PMMSB=c;
	PMLSB=d;
	PMINST=a;
	PMINST=0;
	return 0;
}