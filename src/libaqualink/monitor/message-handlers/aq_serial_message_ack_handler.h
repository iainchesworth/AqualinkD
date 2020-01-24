#ifndef AQ_SERIAL_MESSAGE_ACK_HANDLER_H_
#define AQ_SERIAL_MESSAGE_ACK_HANDLER_H_

#include <stdbool.h>

bool monitor_ackmessageprocessor(unsigned char* rawPacket, unsigned int length);

#endif // AQ_SERIAL_MESSAGE_ACK_HANDLER_H_
