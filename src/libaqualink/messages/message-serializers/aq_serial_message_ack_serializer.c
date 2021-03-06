#include "aq_serial_message_ack_serializer.h"

#include <assert.h>
#include <string.h>

#include "serial/aq_serial_checksums.h"

void serialize_ack_packet(const AQ_Ack_Packet * const packet, unsigned char outbound_buffer[], unsigned int buffer_length)
{
	assert(0 != packet);
	assert(0 != outbound_buffer);
	assert(AQ_ACK_PACKET_LENGTH <= buffer_length);

	memset(outbound_buffer, 0, buffer_length);

	outbound_buffer[0] = packet->Header.DLE;
	outbound_buffer[1] = packet->Header.STX;
	outbound_buffer[2] = (unsigned char)((packet->Destination.Type & packet->Destination.Instance) & 0xFF);
	outbound_buffer[3] = packet->Command;
	outbound_buffer[4] = packet->AckType;
	outbound_buffer[5] = packet->CommandBeingAcked;
	outbound_buffer[6] = generate_jandy_checksum(outbound_buffer, AQ_ACK_PACKET_LENGTH);
	outbound_buffer[7] = packet->Terminator.DLE;
	outbound_buffer[8] = packet->Terminator.ETX;	
}

void deserialize_ack_packet(AQ_Ack_Packet* packet, const unsigned char inbound_buffer[], unsigned int buffer_length)
{
	assert(0 != packet);
	assert(0 != inbound_buffer);
	assert(AQ_ACK_PACKET_LENGTH <= buffer_length);

	packet->Header.DLE = inbound_buffer[0];
	packet->Header.STX = inbound_buffer[1];
	
	packet->Destination.Type = (HardwareDeviceTypes)(inbound_buffer[2] & 0xFC);
	packet->Destination.Instance = (HardwareDeviceInstanceTypes)(inbound_buffer[2] & 0x03);
	packet->Command = (SerialData_Commands)inbound_buffer[3];

	packet->AckType = (SerialData_AckTypes)inbound_buffer[4];
	packet->CommandBeingAcked = inbound_buffer[5];

	packet->Terminator.Checksum = inbound_buffer[6];
	packet->Terminator.DLE = inbound_buffer[7];
	packet->Terminator.ETX = inbound_buffer[8];
}
