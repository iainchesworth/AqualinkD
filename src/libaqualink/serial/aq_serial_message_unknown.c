#include "aq_serial_message_unknown.h"

#include <assert.h>
#include <string.h>

#include "cross-platform/serial.h"
#include "logging/logging.h"
#include "aq_serial_types.h"
#include "utils.h"

static const unsigned int AQ_UNKNOWN_PACKET_LENGTH = 4;
typedef union tagAQ_Unknown_Packet
{
	struct PACKED_SERIAL_STRUCT
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
	switch (processedPacket.Destination)
	{
	case Master_0:
	case Master_1:
	case Master_2:
	case Master_3:
		TRACE("Received unknown packet for Master Device --> id: 0x%02x", processedPacket.Destination);
		break;

	case Keypad_0:
	case Keypad_1:
	case Keypad_2:
	case Keypad_3:
		TRACE("Received unknown for Keypad --> id: 0x%02x", processedPacket.Destination);
		break;

	case DualSpaSideSwitch_InterfaceBoard:
		TRACE("Received unknown for Dual Spa Side Switch interface board --> id: 0x%02x", processedPacket.Destination);
		break;

	case SPA_Remote_0:
	case SPA_Remote_1:
	case SPA_Remote_2:
	case SPA_Remote_3:
		TRACE("Received unknown for SPA Remote --> id: 0x%02x", processedPacket.Destination);
		break;

	case Aqualink_0:
	case Aqualink_1:
	case Aqualink_2:
	case Aqualink_3:
		TRACE("Received unknown for Aqualink --> id: 0x%02x", processedPacket.Destination);
		break;

	case LX_Heater_0:
	case LX_Heater_1:
	case LX_Heater_2:
	case LX_Heater_3:
		TRACE("Received unknown for LX Heater --> id: 0x%02x", processedPacket.Destination);
		break;

	case OneTouch_0:
	case OneTouch_1:
	case OneTouch_2:
	case OneTouch_3:
		TRACE("Received unknown for OneTouch --> id: 0x%02x", processedPacket.Destination);
		break;

	case SWG_0:
	case SWG_1:
	case SWG_2:
	case SWG_3:
		TRACE("Received unknown for SWG --> id: 0x%02x", processedPacket.Destination);
		break;

	case PC_Interface_0:
	case PC_Interface_1:
	case PC_Interface_2:
	case PC_Interface_3:
		TRACE("Received unknown for PC Interface --> id: 0x%02x", processedPacket.Destination);
		break;

	case PDA_Remote_0:
	case PDA_Remote_1:
	case PDA_Remote_2:
	case PDA_Remote_3:
		TRACE("Received unknown for PDA Remote --> id: 0x%02x", processedPacket.Destination);
		break;

	case Jandy_VSP_ePump_0:
	case Jandy_VSP_ePump_1:
	case Jandy_VSP_ePump_2:
	case Jandy_VSP_ePump_3:
		TRACE("Received unknown for Jandy VSP ePump --> id: 0x%02x", processedPacket.Destination);
		break;

	case ChemLink_0:
	case ChemLink_1:
	case ChemLink_2:
	case ChemLink_3:
		TRACE("Received unknown for ChemLink --> id: 0x%02x", processedPacket.Destination);
		break;

	case iAqualink_0:
	case iAqualink_1:
	case iAqualink_2:
	case iAqualink_3:
		TRACE("Received unknown for iAqualink --> id: 0x%02x", processedPacket.Destination);
		break;

	default:
		TRACE(" Received unknown packet for unknown device --> id: 0x%02x", processedPacket.Destination);
		break;
	}

	return true;
}

bool process_unknown_packet(unsigned char* rawPacket, unsigned int length)
{
	assert(0 != rawPacket);
	assert(AQ_UNKNOWN_PACKET_LENGTH <= length);

	TRACE("UNKNOWN - received %d bytes ; expected %d bytes", length, AQ_UNKNOWN_PACKET_LENGTH);
	WARN_IF((AQ_UNKNOWN_PACKET_LENGTH < length), "UNKNOWN - packet length AS-READ is longer than expected...");
	
	AQ_Unknown_Packet processedPacket;
	memcpy(processedPacket.RawBytes, rawPacket, AQ_UNKNOWN_PACKET_LENGTH);

	return handle_unknown_packet(processedPacket);
}
