#ifndef AQ_SERIAL_MESSAGE_PROBE_SERIALIZER_H_
#define AQ_SERIAL_MESSAGE_PROBE_SERIALIZER_H_

#include "serial/aq_serial_types.h"

typedef struct tagAQ_Probe_Packet
{
	unsigned char Header_DLE;
	unsigned char Header_STX;
	SerialData_Destinations Destination : 8;
	SerialData_Commands Command : 8;
	// unsigned char UNKNOWN_DATA[4];
	unsigned char Checksum;
	unsigned char Terminator_DLE;
	unsigned char Terminator_ETX;
}
AQ_Probe_Packet;

extern const unsigned int AQ_PROBE_PACKET_LENGTH;

void serialize_probe_packet(const AQ_Probe_Packet* const packet, unsigned char outbound_buffer[], unsigned int buffer_length);
void deserialize_probe_packet(AQ_Probe_Packet* packet, const unsigned char inbound_buffer[], unsigned int buffer_length);

#endif // AQ_SERIAL_MESSAGE_PROBE_SERIALIZER_H_
