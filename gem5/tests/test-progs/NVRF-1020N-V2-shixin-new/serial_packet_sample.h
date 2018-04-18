#ifndef SERIAL_PACKET_H
#define SERIAL_PACKET_H
#include "UCOSII\\os_cpu.h"

enum {
    SERIAL_HDLC_CTLESC_BYTE = 125,
    SERIAL_TOS_SERIAL_802_15_4_ID = 2,
    SERIAL_SERIAL_PROTO_ACK = 67,
    SERIAL_TOS_SERIAL_CC1000_ID = 1,
    SERIAL_SERIAL_PROTO_PACKET_NOACK = 69,
    SERIAL_SERIAL_PROTO_PACKET_UNKNOWN = 255,
    SERIAL_HDLC_FLAG_BYTE = 126,
    SERIAL_TOS_SERIAL_ACTIVE_MESSAGE_ID = 0,
    SERIAL_TOS_SERIAL_UNKNOWN_ID = 255,
    SERIAL_SERIAL_PROTO_PACKET_ACK = 68
};

enum {
  BUFSIZE = 256,
  MTU = 256,
  ACK_TIMEOUT = 100000, /* in us */
  SYNC_BYTE = SERIAL_HDLC_FLAG_BYTE,
  ESCAPE_BYTE = SERIAL_HDLC_CTLESC_BYTE,

  P_ACK = SERIAL_SERIAL_PROTO_ACK,
  P_PACKET_ACK = SERIAL_SERIAL_PROTO_PACKET_ACK,
  P_PACKET_NO_ACK = SERIAL_SERIAL_PROTO_PACKET_NOACK,
  P_UNKNOWN = SERIAL_SERIAL_PROTO_PACKET_UNKNOWN
};

typedef enum {
  msg_unknown_packet_type,	/* packet of unknown type received */
  msg_ack_timeout, 		/* ack not received within timeout */
  msg_sync,			/* sync achieved */
  msg_too_long,			/* greater than MTU (256 bytes) */
  msg_too_short,		/* less than 4 bytes */
  msg_bad_sync,			/* unexpected sync byte received */
  msg_bad_crc,			/* received packet has bad crc */
  msg_closed,			/* serial port closed itself */
  msg_no_memory,		/* malloc failed */
  msg_unix_error		/* check errno for details */
} serial_source_msg;

INT8U *read_serial_packet(int *len);
/* Effects: Read the serial source src. If a packet is available, return it.
     If in blocking mode and no packet is available, wait for one.
*/

int write_serial_packet(INT8U *data, int len);
/* Effects: writes len byte packet to serial source src
   Returns: 0 if packet successfully written, 1 if successfully written
    but not acknowledged, -1 otherwise
*/
INT8U *get_active_message(int *len);
int write_active_message(INT8U *data, int len);
void send_message_prepare(INT8U am_type);
int send_message_16_add(INT16U b2);
int send_message_add(INT8U byte);
int send_message(INT8U byte);
#endif //SERIAL_PACKET_H
