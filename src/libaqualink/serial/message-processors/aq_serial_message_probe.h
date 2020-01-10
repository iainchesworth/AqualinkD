#ifndef AQ_SERIAL_MESSAGE_PROBE_H_
#define AQ_SERIAL_MESSAGE_PROBE_H_

#include <stdbool.h>

bool process_probe_packet(unsigned char* rawPacket, unsigned int length);

#endif // AQ_SERIAL_MESSAGE_PROBE_H_
