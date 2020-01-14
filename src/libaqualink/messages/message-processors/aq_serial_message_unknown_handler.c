#include "aq_serial_message_unknown_handler.h"

#include <assert.h>
#include <string.h>

#include "cross-platform/serial.h"
#include "hardware/controllers/rs_controller.h"
#include "hardware/devices/hardware_device_registry.h"
#include "hardware/devices/hardware_device.h"
#include "logging/logging.h"
#include "messages/message-serializers/aq_serial_message_unknown_serializer.h"
#include "utility/utils.h"

bool handle_unknown_packet(AQ_Unknown_Packet processedPacket)
{
	bool handled_unknown_packet = false;

	//
	// UNKNOWN messages are those messages that we cannot decode yet.  Just note that we received them and 
	// move on (as we can't really do anything else).
	//

	if (rs_controller_was_packet_to_or_from_rs6_simulator(processedPacket.Destination))
	{
		TRACE("Received UNKNOWN for the RS6 Keypad Simulator --> id: 0x%02x", processedPacket.Destination);
		handled_unknown_packet = rs_controller_rs6_simulator_handle_unknown_packet(&processedPacket);
	}
	else if (rs_controller_was_packet_to_or_from_pda_simulator(processedPacket.Destination))
	{
		TRACE("Received UNKNOWN for the PDA Simulator --> id: 0x%02x", processedPacket.Destination);
		handled_unknown_packet = rs_controller_pda_simulator_handle_unknown_packet(&processedPacket);
	}
	else
	{
		switch (processedPacket.Destination.Type)
		{
		case Master:
			TRACE("Received UNKNOWN packet for Master Device --> id: 0x%02x", processedPacket.Destination);
			break;

		case Keypad:
			TRACE("Received UNKNOWN for Keypad --> id: 0x%02x", processedPacket.Destination);
			break;

		case DualSpaSideSwitch:
			TRACE("Received UNKNOWN for Dual Spa Side Switch interface board --> id: 0x%02x", processedPacket.Destination);
			break;

		case Unknown_0x1x:
			TRACE("Received UNKNOWN for unknown device (range 0x18) --> id: 0x%02x", processedPacket.Destination);
			break;

		case SPA_Remote:
			TRACE("Received UNKNOWN for SPA Remote --> id: 0x%02x", processedPacket.Destination);
			break;

		case Unknown_0x2x:
			TRACE("Received UNKNOWN for unknown device (range 0x28 -> 0x2B) --> id: 0x%02x", processedPacket.Destination);
			break;

		case Aqualink:
			TRACE("Received UNKNOWN for Aqualink --> id: 0x%02x", processedPacket.Destination);
			break;

		case LX_Heater:
			TRACE("Received UNKNOWN for LX Heater --> id: 0x%02x", processedPacket.Destination);
			break;

		case OneTouch:
			TRACE("Received UNKNOWN for OneTouch --> id: 0x%02x", processedPacket.Destination);
			break;

		case Unknown_0x4x:
			TRACE("Received UNKNOWN for unknown device (range 0x48) --> id: 0x%02x", processedPacket.Destination);
			break;

		case SWG:
			TRACE("Received UNKNOWN for SWG --> id: 0x%02x", processedPacket.Destination);
			break;

		case PC_Interface:
			TRACE("Received UNKNOWN for PC Interface --> id: 0x%02x", processedPacket.Destination);
			break;

		case PDA_Remote:
			TRACE("Received UNKNOWN for PDA Remote --> id: 0x%02x", processedPacket.Destination);
			break;

		case Unknown_0x6x:
			TRACE("Received UNKNOWN for unknown device (range 0x68 -> 0x6B) --> id: 0x%02x", processedPacket.Destination);
			break;

		case Unknown_0x7x:
			TRACE("Received UNKNOWN for unknown device (range 0x70 -> 0x73) --> id: 0x%02x", processedPacket.Destination);
			break;

		case Jandy_VSP_ePump:
			TRACE("Received UNKNOWN for Jandy VSP ePump --> id: 0x%02x", processedPacket.Destination);
			break;

		case ChemLink:
			TRACE("Received UNKNOWN for ChemLink --> id: 0x%02x", processedPacket.Destination);
			break;

		case Unknown_0x8x:
			TRACE("Received UNKNOWN for unknown device (range 0x88 -> 0x89) --> id: 0x%02x", processedPacket.Destination);
			break;

		case iAqualink:
			TRACE("Received UNKNOWN for iAqualink --> id: 0x%02x", processedPacket.Destination);
			break;

		default:
			DEBUG(" Received unknown packet for unknown device --> id: 0x%02x", processedPacket.Destination);
			break;
		}

		handled_unknown_packet = true;
	}

	// Last thing to do is register this packet with the controller as the "previous packet".
	rs_controller_record_message_event(processedPacket.Command, processedPacket.Destination);

	return handled_unknown_packet;
}

bool process_unknown_packet(unsigned char* rawPacket, unsigned int length)
{
	assert(0 != rawPacket);
	assert(AQ_UNKNOWN_PACKET_LENGTH <= length);

	TRACE("UNKNOWN - received %d bytes ; expected %d bytes", length, AQ_UNKNOWN_PACKET_LENGTH);
	INFO_IF((AQ_UNKNOWN_PACKET_LENGTH != length), "UNKNOWN - packet length AS-READ is not the same as expected...expected %d, Actual %d", AQ_UNKNOWN_PACKET_LENGTH, length);
	WARN_IF((AQ_UNKNOWN_PACKET_LENGTH < length), "UNKNOWN - packet length AS-READ is longer than expected...expected %d, Actual %d", AQ_UNKNOWN_PACKET_LENGTH, length);
	
	AQ_Unknown_Packet processedPacket;

	deserialize_unknown_packet(&processedPacket, rawPacket, length);

	return handle_unknown_packet(processedPacket);
}
