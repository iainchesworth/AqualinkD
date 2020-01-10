#ifndef AQ_SERIAL_MESSAGE_ACK_SERIALIZER_H_
#define AQ_SERIAL_MESSAGE_ACK_SERIALIZER_H_

#include "serial/aq_serial_types.h"

typedef struct tagAQ_Ack_Packet
{
	unsigned char Header_DLE;
	unsigned char Header_STX;
	SerialData_Destinations Destination;
	SerialData_Commands Command;
	SerialData_AckTypes AckType;
	unsigned char CommandBeingAcked;
	unsigned char Checksum;
	unsigned char Terminator_DLE;
	unsigned char Terminator_ETX;
}
AQ_Ack_Packet;

extern const unsigned int AQ_ACK_PACKET_LENGTH;

void serialize_ack_packet(const AQ_Ack_Packet * const packet, unsigned char outbound_buffer[], unsigned int buffer_length);
void deserialize_ack_packet(AQ_Ack_Packet* packet, const unsigned char inbound_buffer[], unsigned int buffer_length);

#endif // AQ_SERIAL_MESSAGE_ACK_SERIALIZER_H_
