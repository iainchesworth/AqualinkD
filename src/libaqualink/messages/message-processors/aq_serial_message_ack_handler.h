#ifndef AQ_SERIAL_MESSAGE_ACK_HANDLER_H_
#define AQ_SERIAL_MESSAGE_ACK_HANDLER_H_

#include <stdbool.h>

#include "messages/aq_serial_message_ack.h"
#include "serial/aq_serial_types.h"

bool process_ack_packet(unsigned char* rawPacket, unsigned int length);

bool send_ack_packet(SerialData_AckTypes ack_type, unsigned char command_being_acked);

#endif // AQ_SERIAL_MESSAGE_ACK_HANDLER_H_
