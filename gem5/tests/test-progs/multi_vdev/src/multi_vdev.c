#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdint.h>

const uint8_t VDEV_FINISH = 0x04;
const uint32_t N = 10;

int main()
{
	size_t s = sizeof(uint8_t);
	void * addr1 = (void*) 0x3e800000;
	void * addr2 = (void*) 0x3e900000;
	uint8_t *p1 = (uint8_t*) mmap(addr1, s, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
	uint8_t *p2 = (uint8_t*) mmap(addr2, s, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);

	for (uint32_t i = 0; i < 1; i++) {
		*p1 = 0x01;
		while (!(*p1 & VDEV_FINISH));
		*p2 = 0x01;
		while (!(*p2 & VDEV_FINISH));
	}
	munmap(addr2, s);
	munmap(addr1, s); 
	return 0;
}

