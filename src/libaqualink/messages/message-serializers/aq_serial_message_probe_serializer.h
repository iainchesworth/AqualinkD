#ifndef AQ_SERIAL_MESSAGE_PROBE_SERIALIZER_H_
#define AQ_SERIAL_MESSAGE_PROBE_SERIALIZER_H_

#include "messages/aq_serial_message_probe.h"

void serialize_probe_packet(const AQ_Probe_Packet* const packet, unsigned char outbound_buffer[], unsigned int buffer_length);
void deserialize_probe_packet(AQ_Probe_Packet* packet, const unsigned char inbound_buffer[], unsigned int buffer_length);

#endif // AQ_SERIAL_MESSAGE_PROBE_SERIALIZER_H_
