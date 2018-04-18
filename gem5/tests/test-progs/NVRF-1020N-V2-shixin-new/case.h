#include <REG51.H>
#include <stdio.h>
#include "I2C.h"
#include "Delay.h"
#include "KXTJ9.h"
#include "tmp100.h"
#include "NVRF.h"


typedef unsigned char uint8;
typedef unsigned short uint16;
int send_prepare_counter = 0;
uint8 send_prepare[120];

static uint16 crc_byte(uint16 crc, uint8 b)
{
    uint8 i;
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

static uint16 crc_packet(uint8 *dat,  int len)
{
    uint16 crc = 0;

    while (len-- > 0)
        crc = crc_byte(crc, *dat++);

    return crc;
}

void send_message_prepare(uint8 am_type)
{
    send_prepare[0] = 68;
    send_prepare[1]++;
    send_prepare[2] = 0;
    send_prepare[3] = 0xff;
    send_prepare[4] = 0xff;
    send_prepare[5] = 0;
    send_prepare[6] = 0;
    send_prepare[7] = 0; 
    send_prepare[8] = 0; 
    send_prepare[9] = am_type;
    send_prepare_counter = 10;
}
int send_message_add(uint8 byte)
{

    if (send_prepare_counter > 110)
        {
            send_prepare_counter = 0;
            return -1;
        }
    else
        {
            send_prepare[send_prepare_counter++] = byte;
            return 1;
        }
}

int send_message_16_add(uint16 b2)
{
    if (send_prepare_counter > 110)
        {
            return -1;
        }
    send_message_add( (uint8)((b2 >> 8) & 0xff) );
    return send_message_add( (uint8)(b2 & 0xff) );
}

void write_serial_packet(uint8 *dat, int len){
		//需要替换成NVRF发射功耗时间

}

int send_message(uint8 a)
{
    if (send_prepare_counter > 9)
        {
            send_prepare[7] = (uint8)(send_prepare_counter - 10);

            write_serial_packet(send_prepare, send_prepare_counter);  //外设操作
            return 1;
        }
    else
        {
            return -1;
        }
}



