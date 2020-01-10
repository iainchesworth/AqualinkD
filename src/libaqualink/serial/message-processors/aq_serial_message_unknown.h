#ifndef AQ_SERIAL_MESSAGE_UNKNOWN_H_
#define AQ_SERIAL_MESSAGE_UNKNOWN_H_

#include <stdbool.h>

bool process_unknown_packet(unsigned char* rawPacket, unsigned int length);

#endif // AQ_SERIAL_MESSAGE_UNKNOWN_H_
