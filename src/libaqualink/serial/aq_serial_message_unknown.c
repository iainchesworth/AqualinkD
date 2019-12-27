#include "aq_serial_message_unknown.h"

#include <assert.h>
#include <string.h>
#include "aq_serial_types.h"
#include "utils.h"

static const unsigned int AQ_UNKNOWN_PACKET_LENGTH = 4;
typedef union tagAQ_Unknown_Packet
{
	struct
	{
		unsigned char Header_DLE;
		unsigned char Header_STX;
		SerialData_Destinations Destination : 8;
		SerialData_Commands Command : 8;
	};
	unsigned char RawBytes[AQ_UNKNOWN_PACKET_LENGTH];
}
AQ_Unknown_Packet;

bool handle_unknown_packet(AQ_Unknown_Packet processedPacket)
{
	const int myID = 0x0A;

	switch (processedPacket.Destination)
	{
	case myID:
		logMessage(LOG_DEBUG, "AQ_Serial_Message_Status.c | handle_unknown_packet() | Received unknown packet for AqualinkD...but I didn't sent this!\n");
		break;

	case Master_0:
	case Master_1:
	case Master_2:
	case Master_3:
		logMessage(LOG_DEBUG, "AQ_Serial_Message_Status.c | handle_unknown_packet() | Received unknown packet for Master Device --> id: 0x%02x\n", processedPacket.Destination);
		break;

	default:
		logMessage(LOG_DEBUG, "AQ_Serial_Message_Status.c | handle_unknown_packet() |  Received unknown packet for unknown device --> id: 0x%02x\n", processedPacket.Destination);
		break;
	}

	return true;
}

bool process_unknown_packet(unsigned char* rawPacket, unsigned int length)
{
	assert(0 != rawPacket);
	assert(AQ_UNKNOWN_PACKET_LENGTH <= length);
	
	AQ_Unknown_Packet processedPacket;
	memcpy(processedPacket.RawBytes, rawPacket, AQ_UNKNOWN_PACKET_LENGTH);

	return handle_unknown_packet(processedPacket);
}
