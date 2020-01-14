#ifndef AQ_SERIAL_MESSAGE_MSG_LONG_SERIALISER_H_
#define AQ_SERIAL_MESSAGE_MSG_LONG_SERIALISER_H_

#include "messages/aq_serial_message_msg_long.h"

void serialize_msg_long_packet(const AQ_Msg_Long_Packet* const packet, unsigned char outbound_buffer[], unsigned int buffer_length);
void deserialize_msg_long_packet(AQ_Msg_Long_Packet* packet, const unsigned char inbound_buffer[], unsigned int buffer_length);

#endif // AQ_SERIAL_MESSAGE_MSG_LONG_SERIALISER_H_
