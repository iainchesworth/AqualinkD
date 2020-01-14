#ifndef AQ_SERIAL_MESSAGE_ACK_SERIALIZER_H_
#define AQ_SERIAL_MESSAGE_ACK_SERIALIZER_H_

#include "messages/aq_serial_message_ack.h"

void serialize_ack_packet(const AQ_Ack_Packet * const packet, unsigned char outbound_buffer[], unsigned int buffer_length);
void deserialize_ack_packet(AQ_Ack_Packet* packet, const unsigned char inbound_buffer[], unsigned int buffer_length);

#endif // AQ_SERIAL_MESSAGE_ACK_SERIALIZER_H_
