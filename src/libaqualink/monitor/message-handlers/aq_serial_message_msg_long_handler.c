#include "aq_serial_message_msg_long_handler.h"

#include <assert.h>
#include <string.h>

#include "cross-platform/serial.h"
#include "hardware/controllers/rs_controller.h"
#include "hardware/devices/hardware_device_registry.h"
#include "hardware/devices/hardware_device.h"
#include "logging/logging.h"
#include "messages/message-serializers/aq_serial_message_msg_long_serializer.h"
#include "utility/utils.h"

bool monitor_msglongmessagehandler(AQ_Msg_Long_Packet processedPacket)
{
	bool handled_msg_long_packet = false;

	handled_msg_long_packet = false;
	
	// Last thing to do is register this packet with the controller as the "previous packet".
	rs_controller_record_message_event(processedPacket.Command, processedPacket.Destination);

	return handled_msg_long_packet;
}

bool monitor_msglongmessageprocessor(unsigned char* rawPacket, unsigned int length)
{
	assert(0 != rawPacket);
	assert(AQ_MSG_LONG_PACKET_LENGTH <= length);

	TRACE("MSG LONG - received %d bytes ; expected %d bytes", length, AQ_MSG_LONG_PACKET_LENGTH);
	WARN_IF((AQ_MSG_LONG_PACKET_LENGTH != length), "MSG LONG - packet length AS-READ is not the same as expected...expected %d, Actual %d", AQ_MSG_LONG_PACKET_LENGTH, length);
	WARN_IF((AQ_MSG_LONG_PACKET_LENGTH < length), "MSG LONG - packet length AS-READ is longer than expected...expected %d, Actual %d", AQ_MSG_LONG_PACKET_LENGTH, length);

	AQ_Msg_Long_Packet processedPacket;

	deserialize_msg_long_packet(&processedPacket, rawPacket, length);

	INFO("Line Number: 0x%02x - %.16s", processedPacket.LineNumber, processedPacket.Message);

	return monitor_msglongmessagehandler(processedPacket);
}
