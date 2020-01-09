#include "aq_serial_message_probe.h"

#include <assert.h>
#include <string.h>
#include "config/config_helpers.h"
#include "hardware/aqualink_master_controller.h"
#include "logging/logging.h"
#include "aq_serial_types.h"
#include "aq_serial_message_ack.h"
#include "utils.h"

static const unsigned int AQ_PROBE_PACKET_LENGTH = 7; //  11;
typedef union tagAQ_Probe_Packet
{
	struct
	{
		unsigned char Header_DLE;
		unsigned char Header_STX;
		SerialData_Destinations Destination : 8;
		SerialData_Commands Command : 8;
		// unsigned char UNKNOWN_DATA[4];
		unsigned char Checksum;
		unsigned char Terminator_DLE;
		unsigned char Terminator_ETX;
	};
	unsigned char RawBytes[AQ_PROBE_PACKET_LENGTH];
}
AQ_Probe_Packet;

bool handle_probe_packet(AQ_Probe_Packet processedPacket)
{
	bool handled_probe_packet = false;

	// The controller has issued a probe request, record it (along with the time it was sent).
	record_probe_event(&aqualink_master_controller, processedPacket.Destination);

	// Do anything custom that is required for each probe type (e.g. handle simulator responses).
	const DeviceId simulator_id = aqualink_master_controller.Simulator.Id;
	if ((aqualink_master_controller.Simulator.IsEnabled) && (simulator_id == processedPacket.Destination))
	{
		TRACE("Received probe for the Simulator --> id: 0x%02x", processedPacket.Destination);
		handled_probe_packet = aqualink_master_controller.Simulator.ProbeMessageHandler(&aqualink_master_controller);
	}
	else
	{
		switch (processedPacket.Destination)
		{
		case Master_0:
		case Master_1:
		case Master_2:
		case Master_3:
			TRACE("Received probe for Master Device --> id: 0x%02x", processedPacket.Destination);
			break;

		case Keypad_0:
		case Keypad_1:
		case Keypad_2:
		case Keypad_3:
			TRACE("Received probe for Keypad --> id: 0x%02x", processedPacket.Destination);
			break;

		case DualSpaSideSwitch_InterfaceBoard:
			TRACE("Received probe for Dual Spa Side Switch interface board --> id: 0x%02x", processedPacket.Destination);
			break;

		case SPA_Remote_0:
		case SPA_Remote_1:
		case SPA_Remote_2:
		case SPA_Remote_3:
			TRACE("Received probe for SPA Remote --> id: 0x%02x", processedPacket.Destination);
			break;

		case Aqualink_0:
		case Aqualink_1:
		case Aqualink_2:
		case Aqualink_3:
			TRACE("Received probe for Aqualink --> id: 0x%02x", processedPacket.Destination);
			break;

		case LX_Header_0:
		case LX_Header_1:
		case LX_Header_2:
		case LX_Header_3:
			TRACE("Received probe for LX Header --> id: 0x%02x", processedPacket.Destination);
			break;

		case OneTouch_0:
		case OneTouch_1:
		case OneTouch_2:
		case OneTouch_3:
			TRACE("Received probe for OneTouch --> id: 0x%02x", processedPacket.Destination);
			break;

		case SWG_0:
		case SWG_1:
		case SWG_2:
		case SWG_3:
			TRACE("Received probe for SWG --> id: 0x%02x", processedPacket.Destination);
			break;

		case PC_Interface_0:
		case PC_Interface_1:
		case PC_Interface_2:
		case PC_Interface_3:
			TRACE("Received probe for PC Interface --> id: 0x%02x", processedPacket.Destination);
			break;

		case PDA_Remote_0:
		case PDA_Remote_1:
		case PDA_Remote_2:
		case PDA_Remote_3:
			TRACE("Received probe for PDA Remote --> id: 0x%02x", processedPacket.Destination);
			break;

		case Jandy_VSP_ePump_0:
		case Jandy_VSP_ePump_1:
		case Jandy_VSP_ePump_2:
		case Jandy_VSP_ePump_3:
			TRACE("Received probe for Jandy VSP ePump --> id: 0x%02x", processedPacket.Destination);
			break;

		case ChemLink_0:
		case ChemLink_1:
		case ChemLink_2:
		case ChemLink_3:
			TRACE("Received probe for ChemLink --> id: 0x%02x", processedPacket.Destination);
			break;

		case iAqualink_0:
		case iAqualink_1:
		case iAqualink_2:
		case iAqualink_3:
			TRACE("Received probe for iAqualink --> id: 0x%02x", processedPacket.Destination);
			break;

		default:
			TRACE("Received probe for unknown device --> id: 0x%02x", processedPacket.Destination);
			break;
		}

		handled_probe_packet = true;
	}

	return handled_probe_packet;
}

bool process_probe_packet(unsigned char* rawPacket, unsigned int length)
{
	assert(0 != rawPacket);
	assert(AQ_PROBE_PACKET_LENGTH <= length);

	AQ_Probe_Packet processedPacket;
	memcpy(processedPacket.RawBytes, rawPacket, AQ_PROBE_PACKET_LENGTH);

	return handle_probe_packet(processedPacket);
}
