#include "aq_serial_message_probe.h"
#include "aq_serial_message_ack.h"

#include <assert.h>
#include <string.h>

#include "config/config_helpers.h"
#include "cross-platform/serial.h"
#include "hardware/aqualink_master_controller.h"
#include "logging/logging.h"
#include "serial/serializers/aq_serial_message_probe_serializer.h"
#include "serial/aq_serial_types.h"
#include "utils.h"

bool handle_probe_packet(AQ_Probe_Packet processedPacket)
{
	bool handled_probe_packet = false;

	// The controller has issued a probe request, record it (along with the time it was sent).
	record_probe_event(&aqualink_master_controller, processedPacket.Destination);

	// Do anything custom that is required for each probe type (e.g. handle simulator responses).
	const DeviceId simulator_id = aqualink_master_controller.Simulator->Id;
	if ((aqualink_master_controller.Simulator->IsEnabled) && (simulator_id == processedPacket.Destination))
	{
		TRACE("Received PROBE for the Simulator --> id: 0x%02x", processedPacket.Destination);
		handled_probe_packet = aqualink_master_controller.Simulator->ProbeMessageHandler(&aqualink_master_controller);
	}
	else
	{
		switch (processedPacket.Destination)
		{
		case Master_0:
		case Master_1:
		case Master_2:
		case Master_3:
			TRACE("Received PROBE for Master Device --> id: 0x%02x", processedPacket.Destination);
			break;

		case Keypad_0:
		case Keypad_1:
		case Keypad_2:
		case Keypad_3:
			TRACE("Received PROBE for Keypad --> id: 0x%02x", processedPacket.Destination);
			break;

		case DualSpaSideSwitch_InterfaceBoard:
			TRACE("Received PROBE for Dual Spa Side Switch interface board --> id: 0x%02x", processedPacket.Destination);
			break;

		case Unknown_0x18:
			TRACE("Received PROBE for unknown device (range 0x18) --> id: 0x%02x", processedPacket.Destination);
			break;

		case SPA_Remote_0:
		case SPA_Remote_1:
		case SPA_Remote_2:
		case SPA_Remote_3:
			TRACE("Received PROBE for SPA Remote --> id: 0x%02x", processedPacket.Destination);
			break;

		case Unknown_1_0x28:
		case Unknown_2_0x29:
		case Unknown_3_0x2A:
		case Unknown_4_0x2B:
			TRACE("Received PROBE for unknown device (range 0x28 -> 0x2B) --> id: 0x%02x", processedPacket.Destination);
			break;

		case Aqualink_0:
		case Aqualink_1:
		case Aqualink_2:
		case Aqualink_3:
			TRACE("Received PROBE for Aqualink --> id: 0x%02x", processedPacket.Destination);
			break;

		case LX_Heater_0:
		case LX_Heater_1:
		case LX_Heater_2:
		case LX_Heater_3:
			TRACE("Received PROBE for LX Heater --> id: 0x%02x", processedPacket.Destination);
			break;

		case OneTouch_0:
		case OneTouch_1:
		case OneTouch_2:
		case OneTouch_3:
			TRACE("Received PROBE for OneTouch --> id: 0x%02x", processedPacket.Destination);
			break;

		case Unknown_0x48:
			TRACE("Received PROBE for unknown device (range 0x48) --> id: 0x%02x", processedPacket.Destination);
			break;

		case SWG_0:
		case SWG_1:
		case SWG_2:
		case SWG_3:
			TRACE("Received PROBE for SWG --> id: 0x%02x", processedPacket.Destination);
			break;

		case PC_Interface_0:
		case PC_Interface_1:
		case PC_Interface_2:
		case PC_Interface_3:
			TRACE("Received PROBE for PC Interface --> id: 0x%02x", processedPacket.Destination);
			break;

		case PDA_Remote_0:
		case PDA_Remote_1:
		case PDA_Remote_2:
		case PDA_Remote_3:
			TRACE("Received PROBE for PDA Remote --> id: 0x%02x", processedPacket.Destination);
			break;

		case Unknown_1_0x68:
		case Unknown_2_0x69:
		case Unknown_3_0x6A:
		case Unknown_4_0x6B:
			TRACE("Received PROBE for unknown device (range 0x68 -> 0x6B) --> id: 0x%02x", processedPacket.Destination);
			break;

		case Unknown_1_0x70:
		case Unknown_2_0x71:
		case Unknown_3_0x72:
		case Unknown_4_0x73:
			TRACE("Received PROBE for unknown device (range 0x70 -> 0x73) --> id: 0x%02x", processedPacket.Destination);
			break;

		case Jandy_VSP_ePump_0:
		case Jandy_VSP_ePump_1:
		case Jandy_VSP_ePump_2:
		case Jandy_VSP_ePump_3:
			TRACE("Received PROBE for Jandy VSP ePump --> id: 0x%02x", processedPacket.Destination);
			break;

		case ChemLink_0:
		case ChemLink_1:
		case ChemLink_2:
		case ChemLink_3:
			TRACE("Received PROBE for ChemLink --> id: 0x%02x", processedPacket.Destination);
			break;

		case Unknown_0_0x88:
		case Unknown_1_0x89:
			TRACE("Received PROBE for unknown device (range 0x88 -> 0x89) --> id: 0x%02x", processedPacket.Destination);
			break;

		case iAqualink_0:
		case iAqualink_1:
		case iAqualink_2:
		case iAqualink_3:
			TRACE("Received PROBE for iAqualink --> id: 0x%02x", processedPacket.Destination);
			break;

		default:
			INFO("Received PROBE for unknown device --> id: 0x%02x", processedPacket.Destination);
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

	TRACE("PROBE - received %d bytes ; expected %d bytes", length, AQ_PROBE_PACKET_LENGTH);
	WARN_IF((AQ_PROBE_PACKET_LENGTH != length), "PROBE - packet length AS-READ is not the same as expected...expected %d, Actual %d", AQ_PROBE_PACKET_LENGTH, length);
	WARN_IF((AQ_PROBE_PACKET_LENGTH < length), "PROBE - packet length AS-READ is longer than expected...expected %d, Actual %d", AQ_PROBE_PACKET_LENGTH, length);

	AQ_Probe_Packet processedPacket;

	deserialize_probe_packet(&processedPacket, rawPacket, length);

	return handle_probe_packet(processedPacket);
}
