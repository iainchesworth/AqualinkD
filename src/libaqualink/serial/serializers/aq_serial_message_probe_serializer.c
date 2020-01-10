#include "aq_serial_message_probe_serializer.h"

#include <assert.h>
#include <string.h>

#include "serial/aq_serial_checksums.h"
#include "serial/aq_serial_types.h"

const unsigned int AQ_PROBE_PACKET_LENGTH = 7;

void serialize_probe_packet(const AQ_Probe_Packet* const packet, unsigned char outbound_buffer[], unsigned int buffer_length)
{
	assert(0 != packet);
	assert(0 != outbound_buffer);
	assert(AQ_PROBE_PACKET_LENGTH <= buffer_length);

	memset(outbound_buffer, 0, buffer_length);

	///FIXME - Implement when required
}

void deserialize_probe_packet(AQ_Probe_Packet* packet, const unsigned char inbound_buffer[], unsigned int buffer_length)
{
	assert(0 != packet);
	assert(0 != inbound_buffer);
	assert(AQ_PROBE_PACKET_LENGTH <= buffer_length);

	packet->Header_DLE = inbound_buffer[0];
	packet->Header_STX = inbound_buffer[1];
	packet->Destination = (SerialData_Destinations)(inbound_buffer[2] & 0xff);
	packet->Command = (SerialData_Commands)(inbound_buffer[3] & 0xff);

	///FIXME - Is there a payload?  Since this is a work-in-progress
	// don't hard code the bytes for the terminator/checksum yet.

	packet->Checksum = inbound_buffer[buffer_length - 3];
	packet->Terminator_DLE = inbound_buffer[buffer_length - 2];
	packet->Terminator_ETX = inbound_buffer[buffer_length - 1];
}
