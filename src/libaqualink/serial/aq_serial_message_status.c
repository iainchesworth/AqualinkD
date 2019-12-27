#include "aq_serial_message_status.h"

#include <assert.h>
#include <string.h>
#include "aq_serial_types.h"
#include "utils.h"

static const unsigned int AQ_STATUS_PACKET_LENGTH = 9;
typedef union tagAQ_Status_Packet
{
	struct
	{
		unsigned char Header_DLE;
		unsigned char Header_STX;
		SerialData_Destinations Destination : 8;
		SerialData_Commands Command : 8;
		struct
		{
			AQ_LED_States LED_00 : 2;
			AQ_LED_States LED_01 : 2;
			AQ_LED_States LED_02 : 2;
			AQ_LED_States LED_03 : 2;
			AQ_LED_States LED_04 : 2;
			AQ_LED_States LED_05 : 2;
			AQ_LED_States LED_06 : 2;
			AQ_LED_States LED_07 : 2;
			AQ_LED_States LED_08 : 2;
			AQ_LED_States LED_09 : 2;
			AQ_LED_States LED_10 : 2;
			AQ_LED_States LED_11 : 2;
			AQ_LED_States LED_12 : 2;
			AQ_LED_States LED_13 : 2;
			AQ_LED_States LED_14 : 2;
			AQ_LED_States Pool_Header : 2;	// POOL_HTR_LED_INDEX
			AQ_LED_States LED_16 : 2;
			AQ_LED_States SPA_Heater : 2;		// SPA_HTR_LED_INDEX
			AQ_LED_States LED_18 : 2;
			AQ_LED_States Solar_Heater : 2;	// SOLAR_HTR_LED_INDEX
		}
		AQ_Status_Packet_LEDs;
	};
	unsigned char RawBytes[AQ_STATUS_PACKET_LENGTH];
}
AQ_Status_Packet;

bool handle_status_packet(AQ_Status_Packet processedPacket)
{
	const int myID = 0x0A;

	switch (processedPacket.Destination)
	{
	case myID:
		logMessage(LOG_DEBUG, "AQ_Serial_Message_Status.c | handle_status_packet() | Received STATUS for AqualinkD...but I didn't sent this!\n");
		break;

	case Master_0:
	case Master_1:
	case Master_2:
	case Master_3:
		logMessage(LOG_DEBUG, "AQ_Serial_Message_Status.c | handle_status_packet() | Received STATUS for Master Device --> id: 0x%02x\n", processedPacket.Destination);
		break;

	default:
		logMessage(LOG_DEBUG, "AQ_Serial_Message_Status.c | handle_status_packet() |  Received STATUS for unknown device --> id: 0x%02x\n", processedPacket.Destination);
		break;
	}

	return true;
}

bool process_status_packet(unsigned char* rawPacket, unsigned int length)
{
	assert(0 != rawPacket);
	assert(AQ_STATUS_PACKET_LENGTH <= length);

	AQ_Status_Packet processedPacket;
	memcpy(processedPacket.RawBytes, rawPacket, AQ_STATUS_PACKET_LENGTH);

	return handle_status_packet(processedPacket);
}
