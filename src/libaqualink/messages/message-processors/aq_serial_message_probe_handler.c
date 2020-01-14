#include "aq_serial_message_probe_handler.h"

#include <assert.h>
#include <string.h>

#include "config/config_helpers.h"
#include "cross-platform/serial.h"
#include "hardware/controllers/rs_controller.h"
#include "logging/logging.h"
#include "messages/aq_serial_message_ack.h"
#include "messages/message-serializers/aq_serial_message_probe_serializer.h"
#include "utility/utils.h"

bool handle_probe_packet(AQ_Probe_Packet processedPacket)
{
	bool handled_probe_packet = false;

	//
	// PROBE messages are only ever sent by the master so we don't need to do any of the normal
	// checks that we would do for other message types.
	//

	if (rs_controller_was_packet_to_or_from_rs6_simulator(processedPacket.Destination))
	{
		TRACE("Received PROBE for the RS6 Keypad Simulator --> id: 0x%02x", processedPacket.Destination.Type);
		handled_probe_packet = rs_controller_rs6_simulator_handle_probe_packet(&processedPacket);
	}
	else if (rs_controller_was_packet_to_or_from_pda_simulator(processedPacket.Destination))
	{
		TRACE("Received PROBE for the PDA Simulator --> id: 0x%02x", processedPacket.Destination.Type);
		handled_probe_packet = rs_controller_pda_simulator_handle_probe_packet(&processedPacket);
	}
	else
	{
		switch (processedPacket.Destination.Type)
		{
		case Master:
			TRACE("Received PROBE for Master Device --> id: 0x%02x", processedPacket.Destination.Type);
			break;

		case Keypad:
			TRACE("Received PROBE for Keypad --> id: 0x%02x", processedPacket.Destination.Type);
			break;

		case DualSpaSideSwitch:
			TRACE("Received PROBE for Dual Spa Side Switch interface board --> id: 0x%02x", processedPacket.Destination.Type);
			break;

		case Unknown_0x1x:
			TRACE("Received PROBE for unknown device (range 0x18) --> id: 0x%02x", processedPacket.Destination.Type);
			break;

		case SPA_Remote:
			TRACE("Received PROBE for SPA Remote --> id: 0x%02x", processedPacket.Destination.Type);
			break;

		case Unknown_0x2x:
			TRACE("Received PROBE for unknown device (range 0x28 -> 0x2B) --> id: 0x%02x", processedPacket.Destination.Type);
			break;

		case Aqualink:
			TRACE("Received PROBE for Aqualink --> id: 0x%02x", processedPacket.Destination.Type);
			break;

		case LX_Heater:
			TRACE("Received PROBE for LX Heater --> id: 0x%02x", processedPacket.Destination.Type);
			break;

		case OneTouch:
			TRACE("Received PROBE for OneTouch --> id: 0x%02x", processedPacket.Destination.Type);
			break;

		case Unknown_0x4x:
			TRACE("Received PROBE for unknown device (range 0x48) --> id: 0x%02x", processedPacket.Destination.Type);
			break;

		case SWG:
			TRACE("Received PROBE for SWG --> id: 0x%02x", processedPacket.Destination.Type);
			break;

		case PC_Interface:
			TRACE("Received PROBE for PC Interface --> id: 0x%02x", processedPacket.Destination.Type);
			break;

		case PDA_Remote:
			TRACE("Received PROBE for PDA Remote --> id: 0x%02x", processedPacket.Destination.Type);
			break;

		case Unknown_0x6x:
			TRACE("Received PROBE for unknown device (range 0x68 -> 0x6B) --> id: 0x%02x", processedPacket.Destination.Type);
			break;

		case Unknown_0x7x:
			TRACE("Received PROBE for unknown device (range 0x70 -> 0x73) --> id: 0x%02x", processedPacket.Destination.Type);
			break;

		case Jandy_VSP_ePump:
			TRACE("Received PROBE for Jandy VSP ePump --> id: 0x%02x", processedPacket.Destination.Type);
			break;

		case ChemLink:
			TRACE("Received PROBE for ChemLink --> id: 0x%02x", processedPacket.Destination.Type);
			break;

		case Unknown_0x8x:
			TRACE("Received PROBE for unknown device (range 0x88 -> 0x89) --> id: 0x%02x", processedPacket.Destination.Type);
			break;

		case iAqualink:
			TRACE("Received PROBE for iAqualink --> id: 0x%02x", processedPacket.Destination.Type);
			break;

		default:
			INFO("Received PROBE for unknown device --> id: 0x%02x", processedPacket.Destination.Type);
			break;
		}

		handled_probe_packet = true;
	}

	// Last thing to do is register this packet with the controller as the "previous packet".
	rs_controller_record_message_event(processedPacket.Command, processedPacket.Destination);

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
