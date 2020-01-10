#include "aq_serial_message_status.h"

#include <assert.h>
#include <string.h>

#include "cross-platform/serial.h"
#include "hardware/aqualink_master_controller.h"
#include "hardware/devices/hardware_device_registry.h"
#include "hardware/devices/hardware_device.h"
#include "logging/logging.h"
#include "serial/serializers/aq_serial_message_status_serializer.h"
#include "serial/aq_serial_types.h"
#include "utils.h"

bool handle_status_packet(AQ_Status_Packet processedPacket)
{
	//
	// STATUSes are in response to a PROBE which means that we are only able to know the source
	// by checking out the last probe destination.  Note that they typically precede the ACK
	// message from the PROBE'd device.
	// 
	//     M --> PROBE --> S
	//     M <-- S'TUS <-- S
	//     M <--  ACK  <-- S
	//             .
	//             .
	//             .
	//     M --> PROBE --> S
	//     M <-- S'TUS <-- S
	//     M <--  ACK  <-- S
	//

	const DeviceId source_of_status = aqualink_master_controller.ActiveProbe.Destination;
	if (INVALID_DEVICE_ID == source_of_status)
	{
		TRACE("Received STATUS...no preceding PROBE was received (are we starting up?) so this STATUS will be ignored");
	}
	else
	{
		switch (source_of_status)
		{
		case Master_0:
		case Master_1:
		case Master_2:
		case Master_3:
			TRACE("Received STATUS from Master Device --> id: 0x%02x", source_of_status);
			break;

		case Keypad_0:
		case Keypad_1:
		case Keypad_2:
		case Keypad_3:
			TRACE("Received STATUS from Keypad --> id: 0x%02x", source_of_status);
			break;

		case DualSpaSideSwitch_InterfaceBoard:
			TRACE("Received STATUS from Dual Spa Side Switch interface board --> id: 0x%02x", source_of_status);
			break;

		case Unknown_0x18:
			TRACE("Received STATUS from unknown device (range 0x18) --> id: 0x%02x", source_of_status);
			break;

		case SPA_Remote_0:
		case SPA_Remote_1:
		case SPA_Remote_2:
		case SPA_Remote_3:
			TRACE("Received STATUS from SPA Remote --> id: 0x%02x", source_of_status);
			break;

		case Unknown_1_0x28:
		case Unknown_2_0x29:
		case Unknown_3_0x2A:
		case Unknown_4_0x2B:
			TRACE("Received STATUS from unknown device (range 0x28 -> 0x2B) --> id: 0x%02x", source_of_status);
			break;

		case Aqualink_0:
		case Aqualink_1:
		case Aqualink_2:
		case Aqualink_3:
			TRACE("Received STATUS from Aqualink --> id: 0x%02x", source_of_status);
			break;

		case LX_Heater_0:
		case LX_Heater_1:
		case LX_Heater_2:
		case LX_Heater_3:
			TRACE("Received STATUS from LX Heater --> id: 0x%02x", source_of_status);
			break;

		case OneTouch_0:
		case OneTouch_1:
		case OneTouch_2:
		case OneTouch_3:
			TRACE("Received STATUS from OneTouch --> id: 0x%02x", source_of_status);
			break;

		case Unknown_0x48:
			TRACE("Received STATUS from unknown device (range 0x48) --> id: 0x%02x", source_of_status);
			break;

		case SWG_0:
		case SWG_1:
		case SWG_2:
		case SWG_3:
			TRACE("Received STATUS from SWG --> id: 0x%02x", source_of_status);
			break;

		case PC_Interface_0:
		case PC_Interface_1:
		case PC_Interface_2:
		case PC_Interface_3:
			TRACE("Received STATUS from PC Interface --> id: 0x%02x", source_of_status);
			break;

		case PDA_Remote_0:
		case PDA_Remote_1:
		case PDA_Remote_2:
		case PDA_Remote_3:
			TRACE("Received STATUS from PDA Remote --> id: 0x%02x", source_of_status);
			break;

		case Unknown_1_0x68:
		case Unknown_2_0x69:
		case Unknown_3_0x6A:
		case Unknown_4_0x6B:
			TRACE("Received STATUS from unknown device (range 0x68 -> 0x6B) --> id: 0x%02x", source_of_status);
			break;

		case Unknown_1_0x70:
		case Unknown_2_0x71:
		case Unknown_3_0x72:
		case Unknown_4_0x73:
			TRACE("Received STATUS from unknown device (range 0x70 -> 0x73) --> id: 0x%02x", source_of_status);
			break;

		case Jandy_VSP_ePump_0:
		case Jandy_VSP_ePump_1:
		case Jandy_VSP_ePump_2:
		case Jandy_VSP_ePump_3:
			TRACE("Received STATUS from Jandy VSP ePump --> id: 0x%02x", source_of_status);
			break;

		case ChemLink_0:
		case ChemLink_1:
		case ChemLink_2:
		case ChemLink_3:
			TRACE("Received STATUS from ChemLink --> id: 0x%02x", source_of_status);
			break;

		case Unknown_0_0x88:
		case Unknown_1_0x89:
			TRACE("Received STATUS from unknown device (range 0x88 -> 0x89) --> id: 0x%02x", source_of_status);
			break;

		case iAqualink_0:
		case iAqualink_1:
		case iAqualink_2:
		case iAqualink_3:
			TRACE("Received STATUS from iAqualink --> id: 0x%02x", source_of_status);
			break;

		default:
			DEBUG(" Received STATUS from unknown device --> id: 0x%02x", source_of_status);
			break;
		}
	}

	return true;
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
