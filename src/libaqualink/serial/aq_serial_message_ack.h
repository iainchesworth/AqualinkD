#ifndef AQ_SERIAL_MESSAGE_ACK_H_
#define AQ_SERIAL_MESSAGE_ACK_H_

#include <stdbool.h>

bool process_ack_packet(unsigned char* rawPacket, unsigned int length);

#endif // AQ_SERIAL_MESSAGE_ACK_H_
