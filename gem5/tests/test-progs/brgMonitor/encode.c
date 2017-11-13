#include "encode.h"

int packet_prepare_counter = 0;
uint8_t packet_prepare[120];

static uint16 crcByte(uint16 crc, uint8_t b)
{
    uint8_t i;
    crc = crc ^ b << 8;
    i = 8;
    do
        if (crc & 0x8000)
            crc = crc << 1 ^ 0x1021;
        else
            crc = crc << 1;
    while (--i);

    return crc;
}

static uint16 crcSegment(uint8_t *dat,  int len)
{
    uint16 crc = 0;

    while (len-- > 0)
        crc = crcByte(crc, *dat++);

    return crc;
}

void packetPrepare(uint8_t am_type)
{
    packet_prepare[0] = 68;
    packet_prepare[1]++;
    packet_prepare[2] = 0;
    packet_prepare[3] = 0xff;
    packet_prepare[4] = 0xff;
    packet_prepare[5] = 0;
    packet_prepare[6] = 0;
    packet_prepare[7] = 0; 
    packet_prepare[8] = 0; 
    packet_prepare[9] = am_type;
    packet_prepare_counter = 10;
}

int packetAdd(uint8_t byte)
{

    if (packet_prepare_counter > 110)
        {
            packet_prepare_counter = 0;
            return -1;
        }
    else
        {
            packet_prepare[packet_prepare_counter++] = byte;
            return 1;
        }
}

int packet16Add(uint16 b2)
{
    if (packet_prepare_counter > 110)
        {
            return -1;
        }
    packetAdd( (uint8_t)((b2 >> 8) & 0xff) );
    return packetAdd( (uint8_t)(b2 & 0xff) );
}

int packetAddPayload(uint8_t* payload)
{
    uint8_t unit;
    int i = 0;
    int payload_len = sizeof(*payload)/sizeof(float);

    for (i = 0; i < payload_len; i++){
        if ( ! packet16Add((uint16)payload[i]) )
            return -1;
    }

    return payload_len*2;
}

int packetConstruct()
{
    if (packet_prepare_counter > 9)
        {
            packet_prepare[7] = (uint8_t)(packet_prepare_counter - 10);
            return 1;
        }
    else
        {
            return -1;
        }
}

void getEncodedPacket(uint8_t *packet, int *len){
    // 
    packetPrepare(0x00);
    packetAddPayload(packet);
    packetConstruct();

    packet = packet_prepare;
    *len = packet_prepare_counter;
}


