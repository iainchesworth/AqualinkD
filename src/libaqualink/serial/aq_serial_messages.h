#ifndef AQ_SERIAL_MESSAGES_H_
#define AQ_SERIAL_MESSAGES_H_

#include <stdbool.h>

bool process_aqualink_packet(unsigned char* packet, unsigned int length);

#endif // AQ_SERIAL_MESSAGES_H_
