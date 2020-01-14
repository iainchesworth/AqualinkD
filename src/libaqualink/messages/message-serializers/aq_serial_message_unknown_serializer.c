#include "aq_serial_message_unknown_serializer.h"

#include <assert.h>
#include <string.h>

#include "serial/aq_serial_checksums.h"
#include "serial/aq_serial_types.h"

void serialize_unknown_packet(const AQ_Unknown_Packet* const packet, unsigned char outbound_buffer[], unsigned int buffer_length)
{
	assert(0 != packet);
	assert(0 != outbound_buffer);
	assert(AQ_UNKNOWN_PACKET_LENGTH <= buffer_length);

	memset(outbound_buffer, 0, buffer_length);

	///FIXME - Implement when required
}

void deserialize_unknown_packet(AQ_Unknown_Packet* packet, const unsigned char inbound_buffer[], unsigned int buffer_length)
{
	assert(0 != packet);
	assert(0 != inbound_buffer);
	assert(AQ_UNKNOWN_PACKET_LENGTH <= buffer_length);

	packet->Header.DLE = inbound_buffer[0];
	packet->Header.STX = inbound_buffer[1];

	packet->Destination.Type = (HardwareDeviceTypes)(inbound_buffer[2] & 0xFC);
	packet->Destination.Instance = (HardwareDeviceInstanceTypes)(inbound_buffer[2] & 0x03);
	packet->Command = (SerialData_Commands)inbound_buffer[3];

	// There may be a payload of 0..n bytes (length is indeterminate)

	packet->Terminator.Checksum = inbound_buffer[buffer_length - 3];
	packet->Terminator.DLE = inbound_buffer[buffer_length - 2];
	packet->Terminator.ETX = inbound_buffer[buffer_length - 1];
}
