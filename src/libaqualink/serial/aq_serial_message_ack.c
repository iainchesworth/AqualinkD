#include "aq_serial_message_ack.h"

#include <assert.h>
#include <string.h>
#include "aq_serial_types.h"
#include "utils.h"

static const unsigned int AQ_ACK_PACKET_LENGTH = 7;
typedef union tagAQ_Ack_Packet
{
	struct
	{
		unsigned char Header_DLE;
		unsigned char Header_STX;
		SerialData_Destinations Destination : 8;
		SerialData_Commands Command : 8;
		unsigned char Checksum;
		unsigned char Terminator_DLE;
		unsigned char Terminator_ETX;
	};
	unsigned char RawBytes[AQ_ACK_PACKET_LENGTH];
}
AQ_Ack_Packet;

bool handle_ack_packet(AQ_Ack_Packet processedPacket)
{
	const int myID = 0x0A;

	switch (processedPacket.Destination)
	{
	case myID:
		logMessage(LOG_DEBUG, "AQ_Serial_Message_Ack.c | handle_ack_packet() | Received ACK for AqualinkD...but I didn't sent this!\n");
		break;

	case Master_0:
	case Master_1:
	case Master_2:
	case Master_3:
		logMessage(LOG_DEBUG, "AQ_Serial_Message_Ack.c | handle_ack_packet() | Received ACK for Master Device --> id: 0x%02x\n", processedPacket.Destination);
		break;

	default:
		logMessage(LOG_DEBUG, "AQ_Serial_Message_Ack.c | handle_ack_packet() |  Received ACK for unknown device --> id: 0x%02x\n", processedPacket.Destination);
		break;
	}

	return true;
}

bool process_ack_packet(unsigned char* rawPacket, unsigned int length)
{
	assert(0 != rawPacket);
	assert(AQ_ACK_PACKET_LENGTH == length);

	AQ_Ack_Packet processedPacket;
	memcpy(processedPacket.RawBytes, rawPacket, AQ_ACK_PACKET_LENGTH);

	return handle_ack_packet(processedPacket);
}
