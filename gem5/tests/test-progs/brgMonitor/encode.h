//
//	Defining the encoding algorithm.
//
#ifndef     __ENCODE_H_
#define     __ENCODE_H_

typedef unsigned char uint8_t;
typedef unsigned short uint16;

static uint16 crcByte(uint16 crc, uint8_t b);

static uint16 crcSegment(uint8_t *dat,  int len);

void packetPrepare(uint8_t am_type);

int packetAdd(uint8_t byte);

int packet16Add(uint16 b2);

int packetAddPayload(uint8_t* payload);

int packetConstruct();

void getEncodedPacket(uint8_t *packet, int *len);

#endif



