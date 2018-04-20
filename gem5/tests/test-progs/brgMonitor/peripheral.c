//
//
#include "peripheral.h"

void 
periRegister(int peri_id, uint8_t *reg_file){
	reg_file = (uint8_t*) mmap(PERI_ADDR[peri_id], sizeof(uint8_t), PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
}

void 
periLogout(int peri_id){ 
	munmap(PERI_ADDR[peri_id], sizeof(uint8_t));
}

void 
periInit(uint8_t *cmd_reg){
	*cmd_reg = VDEV_INIT;
	while(!(*cmd_reg & VDEV_READY));
}

void 
tmpSense(int *tmp, uint8_t *cmd_reg){
	*cmd_reg = VDEV_EXEC;
	while(!(*cmd_reg & VDEV_FINISH));
	*tmp = 12;
}

void 
accSense(int *x, int *y, int *z, uint8_t *cmd_reg){
	*cmd_reg = VDEV_EXEC;
	while(!(*cmd_reg & VDEV_FINISH));
	*x = 1;
	*y = 2;
	*z = 3;
}

void	
rfTrans(uint8_t *cmd_reg, uint8_t *payload){
	*cmd_reg = VDEV_EXEC;
	while(!(*cmd_reg & VDEV_FINISH));
};

void	
generalVdevActive(uint8_t *cmd_reg){
	*cmd_reg = VDEV_EXEC;
	while(!(*cmd_reg & VDEV_FINISH));
}


