#include "aq_serial_message_ack.h"

#include <assert.h>
#include <string.h>
#include "logging/logging.h"
#include "aq_serial_types.h"
#include "aq_serial_checksums.h"
#include "utils.h"

static const unsigned int AQ_ACK_PACKET_LENGTH = 9;
typedef union tagAQ_Ack_Packet
{
	struct
	{
		unsigned char Header_DLE;
		unsigned char Header_STX;
		SerialData_Destinations Destination : 8;
		SerialData_Commands Command : 8;
		SerialData_AckTypes AckType : 8;
		unsigned char CommandBeingAcked;
		unsigned char Checksum;
		unsigned char Terminator_DLE;
		unsigned char Terminator_ETX;
	};
	unsigned char RawBytes[AQ_ACK_PACKET_LENGTH];
}
AQ_Ack_Packet;

bool handle_ack_packet(AQ_Ack_Packet processedPacket)
{
	switch (processedPacket.Destination)
	{
	case Master_0:
	case Master_1:
	case Master_2:
	case Master_3:
		TRACE("Received ACK for Master Device --> id: 0x%02x, type: 0x%02x, ack'd command: 0x%02x", processedPacket.Destination, processedPacket.AckType, processedPacket.CommandBeingAcked);
		break;

	case Keypad_0:
	case Keypad_1:
	case Keypad_2:
	case Keypad_3:
		TRACE("Received ACK for Keypad --> id: 0x%02x", processedPacket.Destination);
		break;

	case DualSpaSideSwitch_InterfaceBoard:
		TRACE("Received ACK for Dual Spa Side Switch interface board --> id: 0x%02x", processedPacket.Destination);
		break;

	case SPA_Remote_0:
	case SPA_Remote_1:
	case SPA_Remote_2:
	case SPA_Remote_3:
		TRACE("Received ACK for SPA Remote --> id: 0x%02x", processedPacket.Destination);
		break;

	case Aqualink_0:
	case Aqualink_1:
	case Aqualink_2:
	case Aqualink_3:
		TRACE("Received ACK for Aqualink --> id: 0x%02x", processedPacket.Destination);
		break;

	case LX_Header_0:
	case LX_Header_1:
	case LX_Header_2:
	case LX_Header_3:
		TRACE("Received ACK for LX Header --> id: 0x%02x", processedPacket.Destination);
		break;

	case OneTouch_0:
	case OneTouch_1:
	case OneTouch_2:
	case OneTouch_3:
		TRACE("Received ACK for OneTouch --> id: 0x%02x", processedPacket.Destination);
		break;

	case SWG_0:
	case SWG_1:
	case SWG_2:
	case SWG_3:
		TRACE("Received ACK for SWG --> id: 0x%02x", processedPacket.Destination);
		break;

	case PC_Interface_0:
	case PC_Interface_1:
	case PC_Interface_2:
	case PC_Interface_3:
		TRACE("Received ACK for PC Interface --> id: 0x%02x", processedPacket.Destination);
		break;

	case PDA_Remote_0:
	case PDA_Remote_1:
	case PDA_Remote_2:
	case PDA_Remote_3:
		TRACE("Received ACK for PDA Remote --> id: 0x%02x", processedPacket.Destination);
		break;

	case Jandy_VSP_ePump_0:
	case Jandy_VSP_ePump_1:
	case Jandy_VSP_ePump_2:
	case Jandy_VSP_ePump_3:
		TRACE("Received ACK for Jandy VSP ePump --> id: 0x%02x", processedPacket.Destination);
		break;

	case ChemLink_0:
	case ChemLink_1:
	case ChemLink_2:
	case ChemLink_3:
		TRACE("Received ACK for ChemLink --> id: 0x%02x", processedPacket.Destination);
		break;

	case iAqualink_0:
	case iAqualink_1:
	case iAqualink_2:
	case iAqualink_3:
		TRACE("Received ACK for iAqualink --> id: 0x%02x", processedPacket.Destination);
		break;

	default:
		TRACE(" Received ACK for unknown device --> id: 0x%02x", processedPacket.Destination);
		break;
	}

	return true;
}

bool process_ack_packet(unsigned char* rawPacket, unsigned int length)
{
	assert(0 != rawPacket);
	assert(AQ_ACK_PACKET_LENGTH == length);

	TRACE("AQ_Serial_Message_Ack.c | process_ack_packet() |  ACK - received %d bytes ; expected %d bytes", length, AQ_ACK_PACKET_LENGTH);

	AQ_Ack_Packet processedPacket;
	memcpy(processedPacket.RawBytes, rawPacket, AQ_ACK_PACKET_LENGTH);

	return handle_ack_packet(processedPacket);
}

bool send_ack_packet(SerialData_AckTypes ackType, unsigned char commandBeingAcked)
{
	AQ_Ack_Packet ackPacket;

	ackPacket.Header_DLE = DLE;
	ackPacket.Header_STX = STX;

	ackPacket.Destination = Master_0;
	ackPacket.Command = CMD_ACK;
	ackPacket.AckType = ackType;
	ackPacket.CommandBeingAcked = commandBeingAcked;

	ackPacket.Checksum = generate_jandy_checksum(ackPacket.RawBytes, AQ_ACK_PACKET_LENGTH);

	ackPacket.Terminator_DLE = DLE;
	ackPacket.Terminator_ETX = ETX;

	return false;	
}
