#include "aq_serial_message_senders.h"

#include <stdbool.h>

#include "messages/aq_serial_message_ack.h"
#include "serial/aq_serial_writer_queue.h"
#include "serial/aq_serial_types.h"

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

	if (!serial_writer_enqueue_ack_message(&ackPacket))
	{
		packet_can_be_sent_successfully = true;
	}

	return packet_can_be_sent_successfully;
}
