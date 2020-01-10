#include "aq_serial_message_status_serializer.h"

#include <assert.h>
#include <string.h>

#include "serial/aq_serial_checksums.h"
#include "serial/aq_serial_types.h"

const unsigned int AQ_STATUS_PACKET_LENGTH = 12;

void serialize_status_packet(const AQ_Status_Packet* const packet, unsigned char outbound_buffer[], unsigned int buffer_length)
{
	assert(0 != packet);
	assert(0 != outbound_buffer);
	assert(AQ_STATUS_PACKET_LENGTH <= buffer_length);

	memset(outbound_buffer, 0, buffer_length);

	///FIXME - Implement when required
}

void deserialize_status_packet(AQ_Status_Packet* packet, const unsigned char inbound_buffer[], unsigned int buffer_length)
{
	assert(0 != packet);
	assert(0 != inbound_buffer);
	assert(AQ_STATUS_PACKET_LENGTH <= buffer_length);

	packet->Header_DLE = inbound_buffer[0];
	packet->Header_STX = inbound_buffer[1];
	packet->Destination = (SerialData_Destinations)(inbound_buffer[2] & 0xff);
	packet->Command = (SerialData_Commands)(inbound_buffer[3] & 0xff);
	
	packet->LED_00 = (AQ_LED_States)((inbound_buffer[4] & 0xff000000) >> 24);
	packet->LED_01 = (AQ_LED_States)((inbound_buffer[4] & 0x00ff0000) >> 16);
	packet->LED_02 = (AQ_LED_States)((inbound_buffer[4] & 0x0000ff00) >> 8);
	packet->LED_03 = (AQ_LED_States)((inbound_buffer[4] & 0x000000ff) >> 0);
	packet->LED_04 = (AQ_LED_States)((inbound_buffer[5] & 0xff000000) >> 24);
	packet->LED_05 = (AQ_LED_States)((inbound_buffer[5] & 0x00ff0000) >> 16);
	packet->LED_06 = (AQ_LED_States)((inbound_buffer[5] & 0x0000ff00) >> 8);
	packet->LED_07 = (AQ_LED_States)((inbound_buffer[5] & 0x000000ff) >> 0);
	packet->LED_08 = (AQ_LED_States)((inbound_buffer[6] & 0xff000000) >> 24);
	packet->LED_09 = (AQ_LED_States)((inbound_buffer[6] & 0x00ff0000) >> 16);
	packet->LED_10 = (AQ_LED_States)((inbound_buffer[6] & 0x0000ff00) >> 8);
	packet->LED_11 = (AQ_LED_States)((inbound_buffer[6] & 0x000000ff) >> 0);
	packet->LED_12 = (AQ_LED_States)((inbound_buffer[7] & 0xff000000) >> 24);
	packet->LED_13 = (AQ_LED_States)((inbound_buffer[7] & 0x00ff0000) >> 16);
	packet->LED_14 = (AQ_LED_States)((inbound_buffer[7] & 0x0000ff00) >> 8);
	packet->Pool_Heater = (AQ_LED_States)((inbound_buffer[7] & 0x000000ff) >> 0);
	packet->LED_16 = (AQ_LED_States)((inbound_buffer[8] & 0xff000000) >> 24);
	packet->SPA_Heater = (AQ_LED_States)((inbound_buffer[8] & 0x00ff0000) >> 16);
	packet->LED_18 = (AQ_LED_States)((inbound_buffer[8] & 0x0000ff00) >> 8);
	packet->Solar_Heater = (AQ_LED_States)((inbound_buffer[8] & 0x000000ff) >> 0);
	
	packet->Checksum = inbound_buffer[9];
	packet->Terminator_DLE = inbound_buffer[10];
	packet->Terminator_ETX = inbound_buffer[11];
}