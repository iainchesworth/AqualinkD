#include "aq_serial_message_ack_handler.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "cross-platform/serial.h"
#include "hardware/controllers/rs_controller.h"
#include "hardware/devices/hardware_device.h"
#include "hardware/devices/hardware_device_registry.h"
#include "hardware/devices/hardware_device_registry_helpers.h"
#include "logging/logging.h"
#include "messages/message-serializers/aq_serial_message_ack_serializer.h"
#include "serial/aq_serial_writer_queue.h"
#include "utility/utils.h"

bool handle_ack_packet(AQ_Ack_Packet processedPacket)
{
	//
	// ACKs are in response to a message which means that we are only able to know more
	// by checking out the previous message.
	// 
	//     M --> ????? --> S
	//     M <--  ACK  <-- S
	//             .
	//             .
	//             .
	//     M --> ????? --> S
	//     M <--  ACK  <-- S
	//
	// RULES
	//  - If the ACK was sent to the master, the previous message's destination is the external device
	//  - If the ACK was sent by the master, the destination is the external device
	//

	HardwareDeviceId sender_of_ack_message = processedPacket.Destination;
	HardwareDeviceId external_device;

	if (Master == processedPacket.Destination.Type)
	{
		external_device = rs_controller_get_last_message_destination();
	}
	else
	{
		external_device = processedPacket.Destination;
	}

	//
	// Now that a "possible" device has been registered, process the packet.
	//

	if ((INVALID_DEVICE_ID.Type == external_device.Type) && (INVALID_DEVICE_ID.Instance == external_device.Instance))
	{
		TRACE("Received ACK...no preceding message was received (are we starting up?) so this ACK will be ignored");
	}
	else
	{
		switch (processedPacket.Destination.Type)
		{
		case Master:
			TRACE("Received ACK from Master Device --> id: 0x%02x, type: 0x%02x, ack'd command: 0x%02x", sender_of_ack_message, processedPacket.AckType, processedPacket.CommandBeingAcked);
			TRACE("    - ACK'ing message 0x%02x sent by device 0x%02x", external_device, rs_controller_get_last_message_destination());
			device_registry_add_generic_device(external_device);
			break;

		case Keypad:
			TRACE("Received ACK from Keypad --> id: 0x%02x, type: 0x%02x, ack'd command: 0x%02x", sender_of_ack_message, processedPacket.AckType, processedPacket.CommandBeingAcked);
			device_registry_add_generic_device(external_device);
			break;

		case DualSpaSideSwitch:
			TRACE("Received ACK from Dual Spa Side Switch interface board --> id: 0x%02x, type: 0x%02x, ack'd command: 0x%02x", sender_of_ack_message, processedPacket.AckType, processedPacket.CommandBeingAcked);
			device_registry_add_generic_device(external_device);
			break;

		case Unknown_0x1x:
			TRACE("Received ACK from unknown device (range 0x18) --> id: 0x%02x, type: 0x % 02x, ack'd command: 0x%02x", sender_of_ack_message, processedPacket.AckType, processedPacket.CommandBeingAcked);
			device_registry_add_generic_device(external_device);
			break;

		case SPA_Remote:
			TRACE("Received ACK from SPA Remote --> id: 0x%02x, type: 0x%02x, ack'd command: 0x%02x", sender_of_ack_message, processedPacket.AckType, processedPacket.CommandBeingAcked);
			device_registry_add_generic_device(external_device);
			break;
					   
		case Unknown_0x2x:
			TRACE("Received ACK from unknown device (range 0x28 -> 0x2B) --> id: 0x%02x, type: 0x%02x, ack'd command: 0x%02x", sender_of_ack_message, processedPacket.AckType, processedPacket.CommandBeingAcked);
			device_registry_add_generic_device(external_device);
			break;

		case Aqualink:
			TRACE("Received ACK from Aqualink --> id: 0x%02x, type: 0x%02x, ack'd command: 0x%02x", sender_of_ack_message, processedPacket.AckType, processedPacket.CommandBeingAcked);
			device_registry_add_generic_device(external_device);
			break;

		case LX_Heater:
			TRACE("Received ACK from LX Heater --> id: 0x%02x, type: 0x%02x, ack'd command: 0x%02x", sender_of_ack_message, processedPacket.AckType, processedPacket.CommandBeingAcked);
			device_registry_add_generic_device(external_device);
			break;

		case OneTouch:
			TRACE("Received ACK from OneTouch --> id: 0x%02x, type: 0x%02x, ack'd command: 0x%02x", sender_of_ack_message, processedPacket.AckType, processedPacket.CommandBeingAcked);
			device_registry_add_generic_device(external_device);
			break;

		case Unknown_0x4x:
			TRACE("Received ACK from unknown device (range 0x48) --> id: 0x%02x, type: 0x%02x, ack'd command: 0x%02x", sender_of_ack_message, processedPacket.AckType, processedPacket.CommandBeingAcked);
			device_registry_add_generic_device(external_device);
			break;

		case SWG:
			TRACE("Received ACK from SWG --> id: 0x%02x, type: 0x%02x, ack'd command: 0x%02x", sender_of_ack_message, processedPacket.AckType, processedPacket.CommandBeingAcked);
			device_registry_add_generic_device(external_device);
			break;

		case PC_Interface:
			TRACE("Received ACK from PC Interface --> id: 0x%02x, type: 0x%02x, ack'd command: 0x%02x", sender_of_ack_message, processedPacket.AckType, processedPacket.CommandBeingAcked);
			device_registry_add_generic_device(external_device);
			break;

		case PDA_Remote:
			TRACE("Received ACK from PDA Remote --> id: 0x%02x, type: 0x%02x, ack'd command: 0x%02x", sender_of_ack_message, processedPacket.AckType, processedPacket.CommandBeingAcked);
			device_registry_add_pda_remote(external_device);
			break;

		case Unknown_0x6x:
			TRACE("Received ACK from unknown device (range 0x68 -> 0x6B) --> id: 0x%02x, type: 0x%02x, ack'd command: 0x%02x", sender_of_ack_message, processedPacket.AckType, processedPacket.CommandBeingAcked);
			device_registry_add_generic_device(external_device);
			break;

		case Unknown_0x7x:
			TRACE("Received ACK for unknown device (range 0x70 -> 0x73) --> id: 0x%02x, type: 0x%02x, ack'd command: 0x%02x", sender_of_ack_message, processedPacket.AckType, processedPacket.CommandBeingAcked);
			device_registry_add_generic_device(external_device);
			break;

		case Jandy_VSP_ePump:
			TRACE("Received ACK from Jandy VSP ePump --> id: 0x%02x, type: 0x%02x, ack'd command: 0x%02x", sender_of_ack_message, processedPacket.AckType, processedPacket.CommandBeingAcked);
			device_registry_add_generic_device(external_device);
			break;

		case ChemLink:
			TRACE("Received ACK from ChemLink --> id: 0x%02x, type: 0x%02x, ack'd command: 0x%02x", sender_of_ack_message, processedPacket.AckType, processedPacket.CommandBeingAcked);
			device_registry_add_generic_device(external_device);
			break;

		case Unknown_0x8x:
			TRACE("Received ACK from unknown device (range 0x88 -> 0x89) --> id: 0x%02x, type: 0x%02x, ack'd command: 0x%02x", sender_of_ack_message, processedPacket.AckType, processedPacket.CommandBeingAcked);
			device_registry_add_generic_device(external_device);
			break;

		case iAqualink:
			TRACE("Received ACK from iAqualink --> id: 0x%02x, type: 0x%02x, ack'd command: 0x%02x", sender_of_ack_message, processedPacket.AckType, processedPacket.CommandBeingAcked);
			device_registry_add_generic_device(external_device);
			break;

		default:
			DEBUG(" Received ACK from unknown device --> id: 0x%02x, type: 0x%02x, ack'd command: 0x%02x", sender_of_ack_message, processedPacket.AckType, processedPacket.CommandBeingAcked);
			device_registry_add_generic_device(external_device);
			break;
		}
	}

	// Last thing to do is register this packet with the controller as the "previous packet".
	rs_controller_record_message_event(processedPacket.Command, processedPacket.Destination);

	return true;
}

bool process_ack_packet(unsigned char* rawPacket, unsigned int length)
{
	assert(0 != rawPacket);
	assert(AQ_ACK_PACKET_LENGTH == length);

	TRACE("ACK - received %d bytes ; expected %d bytes", length, AQ_ACK_PACKET_LENGTH);
	WARN_IF((AQ_ACK_PACKET_LENGTH != length), "ACK - packet length AS-READ is not the same as expected...expected %d, Actual %d", AQ_ACK_PACKET_LENGTH, length);
	WARN_IF((AQ_ACK_PACKET_LENGTH < length), "ACK - packet length AS-READ is longer than expected...expected %d, Actual %d", AQ_ACK_PACKET_LENGTH, length);

	AQ_Ack_Packet processedPacket;
	
	deserialize_ack_packet(&processedPacket, rawPacket, length);

	return handle_ack_packet(processedPacket);
}

bool send_ack_packet(SerialData_AckTypes ackType, unsigned char commandBeingAcked)
{
	bool packet_can_be_sent_successfully = false;
	AQ_Ack_Packet ackPacket;

	ackPacket.Header.DLE = DLE;
	ackPacket.Header.STX = STX;

	ackPacket.Destination.Type = Master;
	ackPacket.Destination.Instance = Instance_0;
	ackPacket.Command = CMD_ACK;
	ackPacket.AckType = ackType;
	ackPacket.CommandBeingAcked = commandBeingAcked;

	// Ignore the checksum...that'll be added when the packet is serialized.

	ackPacket.Terminator.DLE = DLE;
	ackPacket.Terminator.ETX = ETX;

	if(!serial_writer_enqueue_ack_message(&ackPacket))
	{
		packet_can_be_sent_successfully = true;
	}

	return packet_can_be_sent_successfully;
}
