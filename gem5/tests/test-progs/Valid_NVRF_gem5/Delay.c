#include "Delay.h"


void wait(int m, int n){
	int temp1, temp2;
	
	for(temp1=0;temp1!=m;++temp1){
			for(temp2=0;temp2!=n;++temp2){}
	}	
}

void DelayUS(uchar ten_us){ 
	ten_us = ten_us<<2;
  while(--ten_us);  
} 

void DelayMS(uint iMs){ 
  uint i,j; 
  for(i=0;i<iMs;i++) 
     for(j=0;j<3;j++){
   		DelayUS(1); 
  }
} 

void myNOP(){
	//#pragma asm
	//NOP
	//#pragma endasm
}