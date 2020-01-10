#ifndef AQ_SERIAL_MESSAGE_UNKNOWN_SERIALIZER_H_
#define AQ_SERIAL_MESSAGE_UNKNOWN_SERIALIZER_H_

#include "serial/aq_serial_types.h"

typedef struct tagAQ_Unknown_Packet
{
	unsigned char Header_DLE;
	unsigned char Header_STX;
	SerialData_Destinations Destination : 8;
	SerialData_Commands Command : 8;

	// There may be a payload of 0..n bytes (length is indeterminate)

	unsigned char Checksum;
	unsigned char Terminator_DLE;
	unsigned char Terminator_ETX;
}
AQ_Unknown_Packet;

extern const unsigned int AQ_UNKNOWN_PACKET_LENGTH;

void serialize_unknown_packet(const AQ_Unknown_Packet* const packet, unsigned char outbound_buffer[], unsigned int buffer_length);
void deserialize_unknown_packet(AQ_Unknown_Packet* packet, const unsigned char inbound_buffer[], unsigned int buffer_length);


#endif // AQ_SERIAL_MESSAGE_UNKNOWN_SERIALIZER_H_
