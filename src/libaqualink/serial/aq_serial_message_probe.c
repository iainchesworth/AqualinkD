#include "aq_serial_message_probe.h"

#include <assert.h>
#include <string.h>
#include "aq_serial_types.h"
#include "utils.h"

static const unsigned int AQ_PROBE_PACKET_LENGTH = 11;
typedef union tagAQ_Probe_Packet
{
	struct
	{
		unsigned char Header_DLE;
		unsigned char Header_STX;
		SerialData_Destinations Destination : 8;
		SerialData_Commands Command : 8;
		unsigned char UNKNOWN_DATA[4];
		unsigned char Checksum;
		unsigned char Terminator_DLE;
		unsigned char Terminator_ETX;
	};
	unsigned char RawBytes[AQ_PROBE_PACKET_LENGTH];
}
AQ_Probe_Packet;

bool handle_probe_packet(AQ_Probe_Packet processedPacket)
{
	const int myID = 0x0A;

	switch (processedPacket.Destination)
	{
	case myID:
		logMessage(LOG_DEBUG, "AQ_Serial_Message_Probe.c | handle_probe_packet() | Received probe for AqualinkD...responding!\n");
		break;

	case Master_0:
	case Master_1:
	case Master_2:
	case Master_3:
		logMessage(LOG_DEBUG, "AQ_Serial_Message_Probe.c | handle_probe_packet() | Received probe for Master Device --> id: 0x%02x\n", processedPacket.Destination);
		break;

	default:
		logMessage(LOG_DEBUG, "AQ_Serial_Message_Probe.c | handle_probe_packet() | Received probe for unknown device --> id: 0x%02x\n", processedPacket.Destination);
		break;
	}

	return true;
}

bool process_probe_packet(unsigned char* rawPacket, unsigned int length)
{
	assert(0 != rawPacket);
	assert(AQ_PROBE_PACKET_LENGTH <= length);

	AQ_Probe_Packet processedPacket;
	memcpy(processedPacket.RawBytes, rawPacket, AQ_PROBE_PACKET_LENGTH);

	return handle_probe_packet(processedPacket);
}
