#ifndef AQ_SERIAL_MESSAGE_MSG_LONG_SERIALISER_H_
#define AQ_SERIAL_MESSAGE_MSG_LONG_SERIALISER_H_

#include "serial/aq_serial_types.h"

#define AQ_MSG_LONG_PACKET_MESSAGE_LENGTH 16

typedef struct tagAQ_Msg_Long_Packet
{
	unsigned char Header_DLE;
	unsigned char Header_STX;
	SerialData_Destinations Destination;
	SerialData_Commands Command;

	unsigned char LineNumber;
	unsigned char Message[AQ_MSG_LONG_PACKET_MESSAGE_LENGTH];

	unsigned char Checksum;
	unsigned char Terminator_DLE;
	unsigned char Terminator_ETX;
}
AQ_Msg_Long_Packet;

extern const unsigned int AQ_MSG_LONG_PACKET_LENGTH;

void serialize_msg_long_packet(const AQ_Msg_Long_Packet* const packet, unsigned char outbound_buffer[], unsigned int buffer_length);
void deserialize_msg_long_packet(AQ_Msg_Long_Packet* packet, const unsigned char inbound_buffer[], unsigned int buffer_length);

#endif // AQ_SERIAL_MESSAGE_MSG_LONG_SERIALISER_H_
