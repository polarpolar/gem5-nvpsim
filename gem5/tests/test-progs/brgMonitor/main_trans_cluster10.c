#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/mman.h>
#include <stdint.h>
#include "reg51.h"
#include "delay.h"
#include "fourier.h"
#include "peripheral.h"
#include "encode.h"

#define ANum 60	// acceleration collection number in one packet 
#define PeriNum 3

int main()
{
	int acc_x, acc_y, acc_z, tmp, payload_size, i, j, count = 0;
	uint8_t collection[1+2+6*ANum];
	uint8_t * payload;
	uint8_t * tmp_reg, * acc_reg, * rf_reg, * gvdev1_reg, * gvdev2_reg, * gvdev3_reg, * gvdev4_reg, * gvdev5_reg;
	uint8_t ImagIn[16]={0};
	uint8_t ImagOut[16] = {0};
	uint8_t RealOut[16] = {0};

	printf("Program Start.\n");

	// peripheral register
	tmp_reg = (uint8_t*) mmap(PERI_ADDR[TMP_SENSOR_ID], sizeof(uint8_t), PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
	acc_reg = (uint8_t*) mmap(PERI_ADDR[ACC_SENSOR_ID], sizeof(uint8_t), PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
	rf_reg = (uint8_t*) mmap(PERI_ADDR[RF_ID], sizeof(uint8_t), PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
	//gvdev1_reg = (uint8_t*) mmap(PERI_ADDR[3], sizeof(uint8_t), PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
	//gvdev2_reg = (uint8_t*) mmap(PERI_ADDR[4], sizeof(uint8_t), PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
	//gvdev3_reg = (uint8_t*) mmap(PERI_ADDR[5], sizeof(uint8_t), PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
	//gvdev4_reg = (uint8_t*) mmap(PERI_ADDR[6], sizeof(uint8_t), PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
	//gvdev5_reg = (uint8_t*) mmap(PERI_ADDR[7], sizeof(uint8_t), PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);

	printf("Peripherals Registered.\n");


	printf("Start Execution.\n"); 
	for (j=0; j<PeriNum/2; j++)
	{
		// initialization of TMP Sensor
		periInit(tmp_reg);           // TMP sensor init
		printf("TMP Inited.\n"); 

	
		// data read from TMP Sensor
		tmpSense(&tmp, tmp_reg);
		collection[0]=(unsigned char)((tmp&0xFF00)>>8);
		collection[1]=(unsigned char)(tmp&0x00FF);
		printf("--Collecting the temperature data (%d).\n", tmp);
	
		// initialization of ACC Sensor
		periInit(acc_reg); 	
		printf("ACC Inited.\n");

		// initialization of general vdev-s
		//periInit(gvdev1_reg); 
		//periInit(gvdev2_reg); 
		//periInit(gvdev3_reg); 
		//periInit(gvdev4_reg); 
		//periInit(gvdev5_reg);

		// data read from ACC Sensor
		for(i=1; i<=10; i++)
		{
			accSense(&acc_x, &acc_y, &acc_z, acc_reg);
			accSense(&acc_x, &acc_y, &acc_z, acc_reg);
			accSense(&acc_x, &acc_y, &acc_z, acc_reg);
			collection[6*i-4]=(unsigned char)((acc_x&0xFF00)>>8);
			collection[6*i-3]=(unsigned char)(acc_x&0x00FF);
			collection[6*i-2]=(unsigned char)((acc_y&0xFF00)>>8);
			collection[6*i-1]=(unsigned char)(acc_y&0x00FF);
			collection[6*i]=(unsigned char)((acc_z&0xFF00)>>8);
			collection[6*i+1]=(unsigned char)(acc_z&0x00FF);
			printf("--Collecting the %d-th acceleration data.\n", i);
			collection[6*i-4]=(unsigned char)((acc_x&0xFF00)>>8);
			collection[6*i-3]=(unsigned char)(acc_x&0x00FF);
			collection[6*i-2]=(unsigned char)((acc_y&0xFF00)>>8);
			collection[6*i-1]=(unsigned char)(acc_y&0x00FF);
			collection[6*i]=(unsigned char)((acc_z&0xFF00)>>8);
			collection[6*i+1]=(unsigned char)(acc_z&0x00FF);
			printf("--Collecting the %d-th acceleration data.\n", i);
			collection[6*i-4]=(unsigned char)((acc_x&0xFF00)>>8);
			collection[6*i-3]=(unsigned char)(acc_x&0x00FF);
			collection[6*i-2]=(unsigned char)((acc_y&0xFF00)>>8);
			collection[6*i-1]=(unsigned char)(acc_y&0x00FF);
			collection[6*i]=(unsigned char)((acc_z&0xFF00)>>8);
			collection[6*i+1]=(unsigned char)(acc_z&0x00FF);
			printf("--Collecting the %d-th acceleration data.\n", i);	
		}
	}
	// Data processing: FFT
	fft(32, 0, collection, ImagIn, RealOut, payload);
	printf("FFT completed.\n");

	// Data processing: Encoding
	getEncodedPacket(payload, &payload_size);
	printf("The size of this package is %d Bytes.\n", payload_size);

	
	// transmit data from Zigbee
	//periInit(rf_reg);		// Zigbee init
	//rfTrans(rf_reg, payload);	// Zigbee transmit
	
	printf("RF transmission completed.\n");
	
	printf("Complete the %d-th packet.\n", ++count);


	// peripheral logout
	periLogout(TMP_SENSOR_ID);
	periLogout(ACC_SENSOR_ID);
	//periLogout(RF_ID);
	//periLogout(3);	periLogout(4);	periLogout(5);	//periLogout(6);	periLogout(7);

	return 0;
}
