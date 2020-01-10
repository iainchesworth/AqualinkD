#include "aq_serial_message_msg_long.h"

#include <assert.h>
#include <string.h>

#include "cross-platform/serial.h"
#include "hardware/aqualink_master_controller.h"
#include "hardware/devices/hardware_device_registry.h"
#include "hardware/devices/hardware_device.h"
#include "logging/logging.h"
#include "serial/serializers/aq_serial_message_msg_long_serializer.h"
#include "serial/aq_serial_types.h"
#include "utils.h"

bool handle_msg_long_packet(AQ_Msg_Long_Packet processedPacket)
{
	return false;
}

bool process_msg_long_packet(unsigned char* rawPacket, unsigned int length)
{
	assert(0 != rawPacket);
	assert(AQ_MSG_LONG_PACKET_LENGTH <= length);

	TRACE("MSG LONG - received %d bytes ; expected %d bytes", length, AQ_MSG_LONG_PACKET_LENGTH);
	WARN_IF((AQ_MSG_LONG_PACKET_LENGTH != length), "MSG LONG - packet length AS-READ is not the same as expected...expected %d, Actual %d", AQ_MSG_LONG_PACKET_LENGTH, length);
	WARN_IF((AQ_MSG_LONG_PACKET_LENGTH < length), "MSG LONG - packet length AS-READ is longer than expected...expected %d, Actual %d", AQ_MSG_LONG_PACKET_LENGTH, length);

	AQ_Msg_Long_Packet processedPacket;

	deserialize_msg_long_packet(&processedPacket, rawPacket, length);

	INFO("Line Number: 0x%02x - %.16s", processedPacket.LineNumber, processedPacket.Message);

	return handle_msg_long_packet(processedPacket);
}
