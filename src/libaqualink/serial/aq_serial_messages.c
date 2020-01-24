#include "aq_serial_messages.h"

#include <assert.h>

#include "hardware/controllers/rs_controller.h"
#include "logging/logging.h"
#include "messages/message-bus/aq_serial_message_bus.h"
#include "monitor/monitor.h"
#include "utility/utils.h"
#include "aq_serial.h"
#include "aq_serial_types.h"

bool process_aqualink_packet(unsigned char* rawPacket, unsigned int length)
{
	const unsigned int DESTINATION_BYTE_LOCATION = 2;

	assert(0 != rawPacket);

	MessageBus_Topic *simulator_topic, *monitor_topic;
	bool returnCode = false;

	if (0 == (simulator_topic = messagebus_find_topic_by_type(rs_controller_get_simulator_message_bus(), ((HardwareDeviceTypes)(rawPacket[DESTINATION_BYTE_LOCATION] & 0xFC)))))
	{
		WARN("Failed to retrieve simulator topic from message bus; cannot post message to simulator");
	}
	else if (!messagebus_topic_publish(simulator_topic, rawPacket, length))
	{
		WARN("Failed to publish message to simulator topic");
	}
	else if (0 == (monitor_topic = monitor_get_monitor_message_topic()))
	{
		WARN("Failed to retrieve monitor topic from message bus; cannot post message to monitor.");
	}
	else if (!messagebus_topic_publish(monitor_topic, rawPacket, length))
	{
		WARN("Failed to publish message to monitor topic");
	}
	else
	{
		returnCode = true;
	}

	return returnCode;
}
