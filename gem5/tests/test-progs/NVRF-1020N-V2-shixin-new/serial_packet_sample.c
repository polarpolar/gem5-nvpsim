#include "serial_packet.h"
#include "uart.h"

INT8U receive_buffer[MTU] = {1,3,5};
INT8U send_buffer[MTU];
INT8U send_prepare[120];
int send_prepare_counter;
INT8U ack_buffer[3] = {P_ACK, 0, 0};
int send_counter;
int send_busy = 0;

static INT16U crc_byte(INT16U crc, INT8U b)
{
    INT8U i;
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
static INT16U crc_packet(INT8U *data, int len)
{
    INT16U crc = 0;

    while (len-- > 0)
        crc = crc_byte(crc, *data++);

    return crc;
}
INT8U *read_serial_packet(int *len)
{
    BOOLEAN in_sync; //记录是否已经同步记录
    BOOLEAN escaped; //记录上次是否是 ESCAPED码
    int receive_count;     //
    INT8U byte;
    
    in_sync = 0;// false
    escaped = 0;// false
    receive_count = 0;

    for (;;)
        {
            byte = USART3_GetCharBlock(0);//读取一个byte

            /* 
             * 如果不是同步状态，收到同步头就进入同步状态
             */
            if(!in_sync)
                {
                    if (byte == SYNC_BYTE)
                        {
                            in_sync = 1;
                            receive_count = 0;
                            escaped = 0;
                        }
                    continue;
                }
            /* 
             * 数据长度越界，放弃同步，重新寻找同步数据头
             */
            if (receive_count >= MTU)
                {
                    in_sync = 0;
                    continue;
                }
            /*
             * 同步后内部数据处理
             */
            if (escaped) //如果上一个是替代byte
                {
                    if (byte == SYNC_BYTE)/* sync byte following escape is an error, resync */
                        {
                            in_sync = 1;
                            receive_count = 0;
                            escaped = 0;
                            continue;
                        }
                    byte ^= 0x20;
                    escaped = 0;
                }
            else if (byte == ESCAPE_BYTE)
                {
                    escaped = 1;
                    continue;
                }
            else if (byte == SYNC_BYTE)//收到最后一个数据包
                {
                    int packet_len = receive_count;
                    INT16U read_crc, computed_crc;

                    receive_count = 0; /* ready for next packet */
                    if (packet_len < 4)
                        /* frames that are too small are ignored */
                        /* 重新开始同步 */
                        {
                            in_sync = 1;
                            receive_count = 0;
                            escaped = 0;
                            continue;
                        }
                    read_crc = receive_buffer[packet_len - 2] | receive_buffer[packet_len - 1] << 8;
                    computed_crc = crc_packet(receive_buffer, packet_len - 2);
                    
                    if (read_crc == computed_crc) 
                        {
                            *len = packet_len;
                            return &receive_buffer[0];
                        }
                    else
                        {
                            in_sync = 1;
                            receive_count = 0;
                            escaped = 0;
                            /* We don't lose sync here. If we did, garbage on the line
                               at startup will cause loss of the first packet. */
                            continue;
                        }
                }
            receive_buffer[receive_count++] = byte;
        }
}
void add_send_buffer(INT8U byte)
{
    if ((SYNC_BYTE == byte)||(ESCAPE_BYTE == byte))
        {
            send_buffer[send_counter++] = ESCAPE_BYTE;
            send_buffer[send_counter++] = byte ^ 0x20;
        }
    else
        {
            send_buffer[send_counter++] = byte;
        }
}
int write_serial_packet(INT8U *data, int len)
{
    INT16U send_crc;
    INT8U b;
    int i;
    if (len > 110){ return -1;}
    if (send_busy){ return -2;}
    //最好使用信号量来控制，暂时先用土方法
    send_busy = 1;

    send_crc = crc_packet(data, len);
    send_counter = 0;
    send_buffer[send_counter++] = SYNC_BYTE;

    for(i=0;i<len;i++)
        {
            b = *(data + i);
            add_send_buffer(b);
        }
    b = (INT8U)(send_crc & 0x00ff);
    add_send_buffer(b); 
		b = (INT8U)((send_crc >> 8) & 0x00ff);
    add_send_buffer(b);
    send_buffer[send_counter++] = SYNC_BYTE;
    
    USART3_PutData((INT8U *)send_buffer, send_counter);
    send_busy = 0;
    return 1;
}
INT8U *get_active_message(int *len)
{
    INT8U *get_data;
    int sf_len;
    
    get_data = read_serial_packet(&sf_len);
    if (P_PACKET_ACK == *get_data)
        {
            ack_buffer[1] = *(get_data + 1);
            write_serial_packet(ack_buffer, 3);
        }
    *len = sf_len - 3;
    return (get_data + 3);
}
int write_active_message(INT8U *data, int len)
{
    return 1;
}
void send_message_prepare(INT8U am_type)
{
    send_prepare[0] = P_PACKET_NO_ACK;
    send_prepare[1]++;
    send_prepare[2] = 0;
    send_prepare[3] = 0xff;
    send_prepare[4] = 0xff;
    send_prepare[5] = 0;
    send_prepare[6] = 0;
    send_prepare[7] = 0; //这个字节用于规定数据包长度
    send_prepare[8] = 0; //group id
    send_prepare[9] = am_type;
    send_prepare_counter = 10;
}
int send_message_16_add(INT16U b2)
{
    if (send_prepare_counter > 110)
        {
            return -1;
        }
    send_message_add( (INT8U)((b2 >> 8) & 0xff) );
    return send_message_add( (INT8U)(b2 & 0xff) );
}
int send_message_add(INT8U byte)
{
    /*
     * 这里进行AM message方式组包
     */ 
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
int send_message(INT8U a)
{
    if (send_prepare_counter > 9)
        {
            send_prepare[7] = (INT8U)(send_prepare_counter - 10);
            write_serial_packet(send_prepare, send_prepare_counter);
            return 1;
        }
    else
        {
            return -1;
        }
}