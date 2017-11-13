#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdint.h>

const uint8_t VDEV_FINISH = 0x04;
const uint32_t N = 10;

int main()
{
	size_t s = sizeof(uint8_t);
	void * addr = (void*) 0x3e800000;
	uint8_t *p = (uint8_t*) mmap(addr, s, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
	for (uint32_t i = 0; i < N; i++) {
		*p = 0x01;
		while (!(*p & VDEV_FINISH));
	}
	munmap(addr, s); 
	return 0;
}

