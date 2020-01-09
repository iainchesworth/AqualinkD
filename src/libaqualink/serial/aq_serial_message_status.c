#include "aq_serial_message_status.h"

#include <assert.h>
#include <string.h>
#include <stdint.h>

#include "cross-platform/serial.h"
#include "hardware/aqualink_master_controller.h"
#include "hardware/devices/hardware_device_registry.h"
#include "hardware/devices/hardware_device.h"
#include "logging/logging.h"
#include "aq_serial_types.h"
#include "utils.h"

typedef struct PACKED_SERIAL_STRUCT tagAQ_Status_Packet
{
	uint8_t Header_DLE;
	uint8_t Header_STX;
	SerialData_Destinations Destination;
	SerialData_Commands Command;

	AQ_LED_States LED_00;
	AQ_LED_States LED_01;
	AQ_LED_States LED_02;
	AQ_LED_States LED_03;

	AQ_LED_States LED_04;
	AQ_LED_States LED_05;
	AQ_LED_States LED_06;
	AQ_LED_States LED_07;

	AQ_LED_States LED_08;
	AQ_LED_States LED_09;
	AQ_LED_States LED_10;
	AQ_LED_States LED_11;

	AQ_LED_States LED_12;
	AQ_LED_States LED_13;
	AQ_LED_States LED_14;
	AQ_LED_States Pool_Heater;		// POOL_HTR_LED_INDEX

	AQ_LED_States LED_16;
	AQ_LED_States SPA_Heater;		// SPA_HTR_LED_INDEX
	AQ_LED_States LED_18;
	AQ_LED_States Solar_Heater;		// SOLAR_HTR_LED_INDEX

	// padding byte here?

	uint8_t Checksum;
	uint8_t Terminator_DLE;
	uint8_t Terminator_ETX;
}
AQ_Status_Packet;

static const unsigned int AQ_STATUS_PACKET_LENGTH = 12;
/*typedef union tagAQ_Status_Packet
{
	AQ_Status_Processed_Packet Processed;
	uint8_t RawBytes[AQ_STATUS_PACKET_LENGTH];
}
AQ_Status_Packet;*/

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

	case LX_Heater_0:
	case LX_Heater_1:
	case LX_Heater_2:
	case LX_Heater_3:
		TRACE("Received STATUS for LX Heater --> id: 0x%02x", processedPacket.Destination);
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

	TRACE("STATUS - received %d bytes ; expected %d bytes", length, AQ_STATUS_PACKET_LENGTH);
	WARN_IF((AQ_STATUS_PACKET_LENGTH < length), "STATUS - packet length AS-READ is longer than expected...");

	AQ_Status_Packet processedPacket;

	processedPacket.Header_DLE = rawPacket[0];
	processedPacket.Header_STX = rawPacket[1];
	processedPacket.Destination = rawPacket[2];
	processedPacket.Command = rawPacket[3];
	processedPacket.LED_00 = ((rawPacket[4] & 0xff000000) >> 24);
	processedPacket.LED_01 = ((rawPacket[4] & 0x00ff0000) >> 16);
	processedPacket.LED_02 = ((rawPacket[4] & 0x0000ff00) >> 8);
	processedPacket.LED_03 = ((rawPacket[4] & 0x000000ff) >> 0);
	processedPacket.LED_04 = ((rawPacket[5] & 0xff000000) >> 24);
	processedPacket.LED_05 = ((rawPacket[5] & 0x00ff0000) >> 16);
	processedPacket.LED_06 = ((rawPacket[5] & 0x0000ff00) >> 8);
	processedPacket.LED_07 = ((rawPacket[5] & 0x000000ff) >> 0);
	processedPacket.LED_08 = ((rawPacket[6] & 0xff000000) >> 24);
	processedPacket.LED_09 = ((rawPacket[6] & 0x00ff0000) >> 16);
	processedPacket.LED_10 = ((rawPacket[6] & 0x0000ff00) >> 8);
	processedPacket.LED_11 = ((rawPacket[6] & 0x000000ff) >> 0);
	processedPacket.LED_12 = ((rawPacket[7] & 0xff000000) >> 24);
	processedPacket.LED_13 = ((rawPacket[7] & 0x00ff0000) >> 16);
	processedPacket.LED_14 = ((rawPacket[7] & 0x0000ff00) >> 8);
	processedPacket.Pool_Heater = ((rawPacket[7] & 0x000000ff) >> 0);
	processedPacket.LED_16 = ((rawPacket[8] & 0xff000000) >> 24);
	processedPacket.SPA_Heater = ((rawPacket[8] & 0x00ff0000) >> 16);
	processedPacket.LED_18 = ((rawPacket[8] & 0x0000ff00) >> 8);
	processedPacket.Solar_Heater = ((rawPacket[8] & 0x000000ff) >> 0);
	processedPacket.Checksum = rawPacket[9];
	processedPacket.Terminator_DLE = rawPacket[10];
	processedPacket.Terminator_ETX = rawPacket[11];

	return handle_status_packet(processedPacket);
}
