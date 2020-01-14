#include "aq_serial_message_msg_long_serializer.h"

#include <assert.h>
#include <string.h>

#include "serial/aq_serial_checksums.h"

void serialize_msg_long_packet(const AQ_Msg_Long_Packet* const packet, unsigned char outbound_buffer[], unsigned int buffer_length)
{
	assert(0 != packet);
	assert(0 != outbound_buffer);
	assert(AQ_MSG_LONG_PACKET_LENGTH <= buffer_length);

	memset(outbound_buffer, 0, buffer_length);

	///FIXME - Implement when required
}

void deserialize_msg_long_packet(AQ_Msg_Long_Packet* packet, const unsigned char inbound_buffer[], unsigned int buffer_length)
{
	assert(0 != packet);
	assert(0 != inbound_buffer);
	assert(AQ_MSG_LONG_PACKET_LENGTH <= buffer_length);

	packet->Header.DLE = inbound_buffer[0];
	packet->Header.STX = inbound_buffer[1];

	packet->Destination.Type = (HardwareDeviceTypes)(inbound_buffer[2] & 0xFC);
	packet->Destination.Instance = (HardwareDeviceInstanceTypes)(inbound_buffer[2] & 0x03);
	packet->Command = (SerialData_Commands)inbound_buffer[3];

	packet->LineNumber = inbound_buffer[4];
	memcpy(packet->Message, &inbound_buffer[5], AQ_MSG_LONG_PACKET_MESSAGE_LENGTH);

	///FIXME Add payload processing.
	
	packet->Terminator.Checksum = inbound_buffer[buffer_length - 3];
	packet->Terminator.DLE = inbound_buffer[buffer_length - 2];
	packet->Terminator.ETX = inbound_buffer[buffer_length - 1];
}
