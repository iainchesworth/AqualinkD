#ifndef AQ_SERIAL_MESSAGE_UNKNOWN_SERIALIZER_H_
#define AQ_SERIAL_MESSAGE_UNKNOWN_SERIALIZER_H_

#include "messages/aq_serial_message_unknown.h"

void serialize_unknown_packet(const AQ_Unknown_Packet* const packet, unsigned char outbound_buffer[], unsigned int buffer_length);
void deserialize_unknown_packet(AQ_Unknown_Packet* packet, const unsigned char inbound_buffer[], unsigned int buffer_length);

#endif // AQ_SERIAL_MESSAGE_UNKNOWN_SERIALIZER_H_
