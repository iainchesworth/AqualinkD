#ifndef AQ_SERIAL_MESSAGE_UNKNOWN_HANDLER_H_
#define AQ_SERIAL_MESSAGE_UNKNOWN_HANDLER_H_

#include <stdbool.h>

bool monitor_unknownmessageprocessor(unsigned char* rawPacket, unsigned int length);

#endif // AQ_SERIAL_MESSAGE_UNKNOWN_HANDLER_H_
