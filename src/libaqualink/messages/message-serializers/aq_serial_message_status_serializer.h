#ifndef AQ_SERIAL_MESSAGE_STATUS_SERIALISER_H_
#define AQ_SERIAL_MESSAGE_STATUS_SERIALISER_H_

#include "messages/aq_serial_message_status.h"

void serialize_status_packet(const AQ_Status_Packet* const packet, unsigned char outbound_buffer[], unsigned int buffer_length);
void deserialize_status_packet(AQ_Status_Packet* packet, const unsigned char inbound_buffer[], unsigned int buffer_length);

#endif // AQ_SERIAL_MESSAGE_STATUS_SERIALISER_H_
