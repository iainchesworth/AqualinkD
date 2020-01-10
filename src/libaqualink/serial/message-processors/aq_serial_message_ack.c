#include "aq_serial_message_ack.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "cross-platform/serial.h"
#include "hardware/aqualink_master_controller.h"
#include "hardware/devices/hardware_device_registry.h"
#include "hardware/devices/hardware_device.h"
#include "logging/logging.h"
#include "serial/serializers/aq_serial_message_ack_serializer.h"
#include "serial/aq_serial_types.h"
#include "serial/aq_serial_writer_queue.h"
#include "utils.h"

bool handle_ack_packet(AQ_Ack_Packet processedPacket)
{
	//
	// ACKs are in response to a PROBE which means that we are only able to know the source
	// by checking out the last probe destination.
	// 
	//     M --> PROBE --> S
	//     M <--  ACK  <-- S
	//             .
	//             .
	//             .
	//     M --> PROBE --> S
	//     M <--  ACK  <-- S
	//

	const DeviceId source_of_ack = aqualink_master_controller.ActiveProbe.Destination;
	if (INVALID_DEVICE_ID == source_of_ack)
	{
		TRACE("Received ACK...no preceding PROBE was received (are we starting up?) so this ACK will be ignored");
	}
	else
	{
		switch (source_of_ack)
		{
		case Master_0:
		case Master_1:
		case Master_2:
		case Master_3:
			TRACE("Received ACK from Master Device --> id: 0x%02x, type: 0x%02x, ack'd command: 0x%02x", source_of_ack, processedPacket.AckType, processedPacket.CommandBeingAcked);
			add_generic_device_to_hardware_registry(source_of_ack);
			break;

		case Keypad_0:
		case Keypad_1:
		case Keypad_2:
		case Keypad_3:
			TRACE("Received ACK from Keypad --> id: 0x%02x, type: 0x%02x, ack'd command: 0x%02x", source_of_ack, processedPacket.AckType, processedPacket.CommandBeingAcked);
			add_generic_device_to_hardware_registry(source_of_ack);
			break;

		case DualSpaSideSwitch_InterfaceBoard:
			TRACE("Received ACK from Dual Spa Side Switch interface board --> id: 0x%02x, type: 0x%02x, ack'd command: 0x%02x", source_of_ack, processedPacket.AckType, processedPacket.CommandBeingAcked);
			add_generic_device_to_hardware_registry(source_of_ack);
			break;

		case Unknown_0x18:
			TRACE("Received ACK from unknown device (range 0x18) --> id: 0x%02x, type: 0x % 02x, ack'd command: 0x%02x", source_of_ack, processedPacket.AckType, processedPacket.CommandBeingAcked);
			add_generic_device_to_hardware_registry(source_of_ack);
			break;

		case SPA_Remote_0:
		case SPA_Remote_1:
		case SPA_Remote_2:
		case SPA_Remote_3:
			TRACE("Received ACK from SPA Remote --> id: 0x%02x, type: 0x%02x, ack'd command: 0x%02x", source_of_ack, processedPacket.AckType, processedPacket.CommandBeingAcked);
			add_generic_device_to_hardware_registry(source_of_ack);
			break;
					   
		case Unknown_1_0x28:
		case Unknown_2_0x29:
		case Unknown_3_0x2A:
		case Unknown_4_0x2B:
			TRACE("Received ACK from unknown device (range 0x28 -> 0x2B) --> id: 0x%02x, type: 0x%02x, ack'd command: 0x%02x", source_of_ack, processedPacket.AckType, processedPacket.CommandBeingAcked);
			add_generic_device_to_hardware_registry(source_of_ack);
			break;

		case Aqualink_0:
		case Aqualink_1:
		case Aqualink_2:
		case Aqualink_3:
			TRACE("Received ACK from Aqualink --> id: 0x%02x, type: 0x%02x, ack'd command: 0x%02x", source_of_ack, processedPacket.AckType, processedPacket.CommandBeingAcked);
			add_generic_device_to_hardware_registry(source_of_ack);
			break;

		case LX_Heater_0:
		case LX_Heater_1:
		case LX_Heater_2:
		case LX_Heater_3:
			TRACE("Received ACK from LX Heater --> id: 0x%02x, type: 0x%02x, ack'd command: 0x%02x", source_of_ack, processedPacket.AckType, processedPacket.CommandBeingAcked);
			add_generic_device_to_hardware_registry(source_of_ack);
			break;

		case OneTouch_0:
		case OneTouch_1:
		case OneTouch_2:
		case OneTouch_3:
			TRACE("Received ACK from OneTouch --> id: 0x%02x, type: 0x%02x, ack'd command: 0x%02x", source_of_ack, processedPacket.AckType, processedPacket.CommandBeingAcked);
			add_generic_device_to_hardware_registry(source_of_ack);
			break;

		case Unknown_0x48:
			TRACE("Received ACK from unknown device (range 0x48) --> id: 0x%02x, type: 0x%02x, ack'd command: 0x%02x", source_of_ack, processedPacket.AckType, processedPacket.CommandBeingAcked);
			add_generic_device_to_hardware_registry(source_of_ack);
			break;

		case SWG_0:
		case SWG_1:
		case SWG_2:
		case SWG_3:
			TRACE("Received ACK from SWG --> id: 0x%02x, type: 0x%02x, ack'd command: 0x%02x", source_of_ack, processedPacket.AckType, processedPacket.CommandBeingAcked);
			add_generic_device_to_hardware_registry(source_of_ack);
			break;

		case PC_Interface_0:
		case PC_Interface_1:
		case PC_Interface_2:
		case PC_Interface_3:
			TRACE("Received ACK from PC Interface --> id: 0x%02x, type: 0x%02x, ack'd command: 0x%02x", source_of_ack, processedPacket.AckType, processedPacket.CommandBeingAcked);
			add_generic_device_to_hardware_registry(source_of_ack);
			break;

		case PDA_Remote_0:
		case PDA_Remote_1:
		case PDA_Remote_2:
		case PDA_Remote_3:
			TRACE("Received ACK from PDA Remote --> id: 0x%02x, type: 0x%02x, ack'd command: 0x%02x", source_of_ack, processedPacket.AckType, processedPacket.CommandBeingAcked);
			add_generic_device_to_hardware_registry(source_of_ack);
			break;

		case Unknown_1_0x68:
		case Unknown_2_0x69:
		case Unknown_3_0x6A:
		case Unknown_4_0x6B:
			TRACE("Received ACK from unknown device (range 0x68 -> 0x6B) --> id: 0x%02x, type: 0x%02x, ack'd command: 0x%02x", source_of_ack, processedPacket.AckType, processedPacket.CommandBeingAcked);
			add_generic_device_to_hardware_registry(source_of_ack);
			break;

		case Unknown_1_0x70:
		case Unknown_2_0x71:
		case Unknown_3_0x72:
		case Unknown_4_0x73:
			TRACE("Received ACK for unknown device (range 0x70 -> 0x73) --> id: 0x%02x, type: 0x%02x, ack'd command: 0x%02x", source_of_ack, processedPacket.AckType, processedPacket.CommandBeingAcked);
			add_generic_device_to_hardware_registry(source_of_ack);
			break;

		case Jandy_VSP_ePump_0:
		case Jandy_VSP_ePump_1:
		case Jandy_VSP_ePump_2:
		case Jandy_VSP_ePump_3:
			TRACE("Received ACK from Jandy VSP ePump --> id: 0x%02x, type: 0x%02x, ack'd command: 0x%02x", source_of_ack, processedPacket.AckType, processedPacket.CommandBeingAcked);
			add_generic_device_to_hardware_registry(source_of_ack);
			break;

		case ChemLink_0:
		case ChemLink_1:
		case ChemLink_2:
		case ChemLink_3:
			TRACE("Received ACK from ChemLink --> id: 0x%02x, type: 0x%02x, ack'd command: 0x%02x", source_of_ack, processedPacket.AckType, processedPacket.CommandBeingAcked);
			add_generic_device_to_hardware_registry(source_of_ack);
			break;

		case Unknown_0_0x88:
		case Unknown_1_0x89:
			TRACE("Received ACK from unknown device (range 0x88 -> 0x89) --> id: 0x%02x, type: 0x%02x, ack'd command: 0x%02x", source_of_ack, processedPacket.AckType, processedPacket.CommandBeingAcked);
			add_generic_device_to_hardware_registry(source_of_ack);
			break;

		case iAqualink_0:
		case iAqualink_1:
		case iAqualink_2:
		case iAqualink_3:
			TRACE("Received ACK from iAqualink --> id: 0x%02x, type: 0x%02x, ack'd command: 0x%02x", source_of_ack, processedPacket.AckType, processedPacket.CommandBeingAcked);
			add_generic_device_to_hardware_registry(source_of_ack);
			break;

		default:
			DEBUG(" Received ACK from unknown device --> id: 0x%02x, type: 0x%02x, ack'd command: 0x%02x", source_of_ack, processedPacket.AckType, processedPacket.CommandBeingAcked);
			add_generic_device_to_hardware_registry(source_of_ack);
			break;
		}
	}

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

	ackPacket.Header_DLE = DLE;
	ackPacket.Header_STX = STX;

	ackPacket.Destination = Master_0;
	ackPacket.Command = CMD_ACK;
	ackPacket.AckType = ackType;
	ackPacket.CommandBeingAcked = commandBeingAcked;

	// Ignore the checksum...that'll be added when the packet is serialized.

	ackPacket.Terminator_DLE = DLE;
	ackPacket.Terminator_ETX = ETX;

	if(!serial_writer_enqueue_ack_message(&ackPacket))
	{
		packet_can_be_sent_successfully = true;
	}

	return packet_can_be_sent_successfully;
}
