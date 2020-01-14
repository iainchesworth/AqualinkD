#include "aq_serial_message_status_handler.h"

#include <assert.h>
#include <string.h>

#include "cross-platform/serial.h"
#include "hardware/controllers/rs_controller.h"
#include "hardware/devices/hardware_device_registry.h"
#include "hardware/devices/hardware_device.h"
#include "logging/logging.h"
#include "messages/message-serializers/aq_serial_message_status_serializer.h"
#include "utility/utils.h"

bool handle_status_packet(AQ_Status_Packet processedPacket)
{
	bool handled_status_packet = false;

	if (rs_controller_was_packet_to_or_from_rs6_simulator(processedPacket.Destination))
	{
		TRACE("Received STATUS for the RS6 Keypad Simulator --> id: 0x%02x", processedPacket.Destination);
		handled_status_packet = rs_controller_rs6_simulator_handle_status_packet(&processedPacket);

		TRACE("Received STATUS message for the Aqualink RS6 Keypad Simulator so this STATUS will be ignored");
	}
	else if (rs_controller_was_packet_to_or_from_pda_simulator(processedPacket.Destination))
	{
		TRACE("Received STATUS for the PDA Simulator --> id: 0x%02x", processedPacket.Destination);
		handled_status_packet = rs_controller_pda_simulator_handle_status_packet(&processedPacket);

		TRACE("Received STATUS message for the Aqualink PDA Simulator so this STATUS will be ignored");
	}
	else
	{
		switch (processedPacket.Destination.Type)
		{
		case Master:
			TRACE("Received STATUS packet for Master Device --> id: 0x%02x", processedPacket.Destination);
			break;

		case Keypad:
			TRACE("Received STATUS for Keypad --> id: 0x%02x", processedPacket.Destination);
			break;

		case DualSpaSideSwitch:
			TRACE("Received STATUS for Dual Spa Side Switch interface board --> id: 0x%02x", processedPacket.Destination);
			break;

		case Unknown_0x1x:
			TRACE("Received STATUS for unknown device (range 0x18) --> id: 0x%02x", processedPacket.Destination);
			break;

		case SPA_Remote:
			TRACE("Received STATUS for SPA Remote --> id: 0x%02x", processedPacket.Destination);
			break;

		case Unknown_0x2x:
			TRACE("Received STATUS for unknown device (range 0x28 -> 0x2B) --> id: 0x%02x", processedPacket.Destination);
			break;

		case Aqualink:
			TRACE("Received STATUS for Aqualink --> id: 0x%02x", processedPacket.Destination);
			break;

		case LX_Heater:
			TRACE("Received STATUS for LX Heater --> id: 0x%02x", processedPacket.Destination);
			break;

		case OneTouch:
			TRACE("Received STATUS for OneTouch --> id: 0x%02x", processedPacket.Destination);
			break;

		case Unknown_0x4x:
			TRACE("Received STATUS for unknown device (range 0x48) --> id: 0x%02x", processedPacket.Destination);
			break;

		case SWG:
			TRACE("Received STATUS for SWG --> id: 0x%02x", processedPacket.Destination);
			break;

		case PC_Interface:
			TRACE("Received STATUS for PC Interface --> id: 0x%02x", processedPacket.Destination);
			break;

		case PDA_Remote:
			TRACE("Received STATUS for PDA Remote --> id: 0x%02x", processedPacket.Destination);
			break;

		case Unknown_0x6x:
			TRACE("Received STATUS for unknown device (range 0x68 -> 0x6B) --> id: 0x%02x", processedPacket.Destination);
			break;

		case Unknown_0x7x:
			TRACE("Received STATUS for unknown device (range 0x70 -> 0x73) --> id: 0x%02x", processedPacket.Destination);
			break;

		case Jandy_VSP_ePump:
			TRACE("Received STATUS for Jandy VSP ePump --> id: 0x%02x", processedPacket.Destination);
			break;

		case ChemLink:
			TRACE("Received STATUS for ChemLink --> id: 0x%02x", processedPacket.Destination);
			break;

		case Unknown_0x8x:
			TRACE("Received STATUS for unknown device (range 0x88 -> 0x89) --> id: 0x%02x", processedPacket.Destination);
			break;

		case iAqualink:
			TRACE("Received STATUS for iAqualink --> id: 0x%02x", processedPacket.Destination);
			break;

		default:
			DEBUG(" Received STATUS packet for unknown device --> id: 0x%02x", processedPacket.Destination);
			break;
		}

		handled_status_packet = true;
	}

	// Last thing to do is register this packet with the controller as the "previous packet".
	rs_controller_record_message_event(processedPacket.Command, processedPacket.Destination);

	return handled_status_packet;
}

bool process_status_packet(unsigned char* rawPacket, unsigned int length)
{
	assert(0 != rawPacket);
	assert(AQ_STATUS_PACKET_LENGTH <= length);

	TRACE("STATUS - received %d bytes ; expected %d bytes", length, AQ_STATUS_PACKET_LENGTH);
	WARN_IF((AQ_STATUS_PACKET_LENGTH != length), "STATUS - packet length AS-READ is not the same as expected...expected %d, Actual %d", AQ_STATUS_PACKET_LENGTH, length);
	WARN_IF((AQ_STATUS_PACKET_LENGTH < length), "STATUS - packet length AS-READ is longer than expected...expected %d, Actual %d", AQ_STATUS_PACKET_LENGTH, length);

	AQ_Status_Packet processedPacket;

	deserialize_status_packet(&processedPacket, rawPacket, length);

	return handle_status_packet(processedPacket);
}
