#ifndef AQ_SERIAL_MESSAGE_STATUS_HANDLER_H_
#define AQ_SERIAL_MESSAGE_STATUS_HANDLER_H_

#include <stdbool.h>

bool process_status_packet(unsigned char* rawPacket, unsigned int length);

#endif // AQ_SERIAL_MESSAGE_STATUS_HANDLER_H_
