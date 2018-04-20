//
//
//
#ifndef __PERIPHERAL_H__
#define __PERIPHERAL_H__

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdint.h>


#define TMP_SENSOR_ID	0
#define ACC_SENSOR_ID	1
#define RF_ID			2

#define VDEV_INIT		0x80
#define VDEV_EXEC		0x40
#define VDEV_READY	0x04
#define VDEV_FINISH 	0x01

static int VDEV_REG_NUM[8] = {
	3,	// TMP_SENSOR
	7,	// ACC_SENSOR
	100,	// RF_TRANS
	7,	// General VdeV
	7,	// General VdeV
	7,	// General VdeV
	7,	// General VdeV
	7	// General VdeV
};

static void * PERI_ADDR[8] = {
	(void *) 0x3e800000,
	(void *) 0x3e900000,
	(void *) 0x3ea00000,
	(void *) 0x3eb00000,
	(void *) 0x3ec00000,
	(void *) 0x3ed00000,
	(void *) 0x3ee00000,
	(void *) 0x3ef00000
};

void	periRegister(int peri_id, uint8_t *reg_file);
void	periLogout(int peri_id);
void	periInit(uint8_t *cmd_reg);

void	tmpSense(int *tmp, uint8_t *cmd_reg);
void	accSense(int *x, int *y, int *z, uint8_t *cmd_reg);
void	rfTrans(uint8_t *cmd_reg, uint8_t *payload);
void	generalVdevActive(uint8_t *cmd_reg);




#endif

