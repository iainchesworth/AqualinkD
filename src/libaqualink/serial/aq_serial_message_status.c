#include "aq_serial_message_status.h"

#include <assert.h>
#include <string.h>
#include "logging/logging.h"
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
	switch (processedPacket.Destination)
	{
	case Master_0:
	case Master_1:
	case Master_2:
	case Master_3:
		TRACE("Received STATUS for Master Device --> id: 0x%02x", processedPacket.Destination);
		break;

	case Keypad_0:
	case Keypad_1:
	case Keypad_2:
	case Keypad_3:
		TRACE("Received STATUS for Keypad --> id: 0x%02x", processedPacket.Destination);
		break;

	case DualSpaSideSwitch_InterfaceBoard:
		TRACE("Received STATUS for Dual Spa Side Switch interface board --> id: 0x%02x", processedPacket.Destination);
		break;

	case SPA_Remote_0:
	case SPA_Remote_1:
	case SPA_Remote_2:
	case SPA_Remote_3:
		TRACE("Received STATUS for SPA Remote --> id: 0x%02x", processedPacket.Destination);
		break;

	case Aqualink_0:
	case Aqualink_1:
	case Aqualink_2:
	case Aqualink_3:
		TRACE("Received STATUS for Aqualink --> id: 0x%02x", processedPacket.Destination);
		break;

	case LX_Header_0:
	case LX_Header_1:
	case LX_Header_2:
	case LX_Header_3:
		TRACE("Received STATUS for LX Header --> id: 0x%02x", processedPacket.Destination);
		break;

	case OneTouch_0:
	case OneTouch_1:
	case OneTouch_2:
	case OneTouch_3:
		TRACE("Received STATUS for OneTouch --> id: 0x%02x", processedPacket.Destination);
		break;

	case SWG_0:
	case SWG_1:
	case SWG_2:
	case SWG_3:
		TRACE("Received STATUS for SWG --> id: 0x%02x", processedPacket.Destination);
		break;

	case PC_Interface_0:
	case PC_Interface_1:
	case PC_Interface_2:
	case PC_Interface_3:
		TRACE("Received STATUS for PC Interface --> id: 0x%02x", processedPacket.Destination);
		break;

	case PDA_Remote_0:
	case PDA_Remote_1:
	case PDA_Remote_2:
	case PDA_Remote_3:
		TRACE("Received STATUS for PDA Remote --> id: 0x%02x", processedPacket.Destination);
		break;

	case Jandy_VSP_ePump_0:
	case Jandy_VSP_ePump_1:
	case Jandy_VSP_ePump_2:
	case Jandy_VSP_ePump_3:
		TRACE("Received STATUS for Jandy VSP ePump --> id: 0x%02x", processedPacket.Destination);
		break;

	case ChemLink_0:
	case ChemLink_1:
	case ChemLink_2:
	case ChemLink_3:
		TRACE("Received STATUS for ChemLink --> id: 0x%02x", processedPacket.Destination);
		break;

	case iAqualink_0:
	case iAqualink_1:
	case iAqualink_2:
	case iAqualink_3:
		TRACE("Received STATUS for iAqualink --> id: 0x%02x", processedPacket.Destination);
		break;

	default:
		TRACE(" Received STATUS for unknown device --> id: 0x%02x", processedPacket.Destination);
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
