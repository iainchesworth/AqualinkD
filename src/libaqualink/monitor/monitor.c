#include "monitor.h"

#include <assert.h>
#include <stdbool.h>

#include "cross-platform/threads.h"
#include "logging/logging.h"
#include "messages/message-bus/aq_serial_message_bus.h"
#include "monitor/message-handlers/aq_serial_message_ack_handler.h"
#include "monitor/message-handlers/aq_serial_message_msg_long_handler.h"
#include "monitor/message-handlers/aq_serial_message_probe_handler.h"
#include "monitor/message-handlers/aq_serial_message_status_handler.h"
#include "monitor/message-handlers/aq_serial_message_unknown_handler.h"
#include "serial/aq_serial.h"

#include "aqualink.h"

MessageBus monitor_messagebus;
MessageBus_Topic monitor_messagetopic;

unsigned char monitor_messagetopic_buffer[TOPIC_MAX_MSG_LENGTH];
const char monitor_messagetopic_name[] = "Message Monitor Message Topic";

thrd_t monitor_thread;

bool monitor_initialise(void)
{
	messagebus_init(&monitor_messagebus);
	messagebus_topic_init(&monitor_messagetopic, monitor_messagetopic_buffer, TOPIC_MAX_MSG_LENGTH);
	messagebus_advertise_topic_by_name(&monitor_messagebus, &monitor_messagetopic, monitor_messagetopic_name);

	if (thrd_error == thrd_create(&monitor_thread, monitor_processing_thread, 0))
	{
		ERROR("Failed to create message monitor thread");
	}
	else
	{
		DEBUG("Successfully created monitor monitor thread");
	}

	return true;
}

bool monitor_enable(void)
{
	return true;
}

bool monitor_disable(void)
{
	return true;
}

MessageBus* monitor_get_monitor_message_bus()
{
	return &monitor_messagebus;
}

MessageBus_Topic* monitor_get_monitor_message_topic()
{
	return &monitor_messagetopic;
}

int monitor_processing_thread(void* argument)
{
	DEBUG("Starting Monitor Thread");

	UNREFERENCED_PARAMETER(argument);

	const unsigned int COMMAND_ID_BYTE_LOCATION = 3;
	unsigned char rawPacket[AQ_MAXPKTLEN];

	do
	{
		// Clearing the packet buffer to make sure that there's no data carry-over.
		memset(rawPacket, 0, AQ_MAXPKTLEN);

		// Wait for an incoming message.
		TRACE("Waiting for messages on the OneTouch Simulator message topic");
		messagebus_topic_wait(&monitor_messagetopic, rawPacket, AQ_MAXPKTLEN);

		TRACE("Monitor tracked message on serial bus...processing it");
		switch (rawPacket[COMMAND_ID_BYTE_LOCATION])
		{
		case CMD_ACK:
			DEBUG("Received CMD_ACK packet");
			{
				monitor_ackmessageprocessor(rawPacket, AQ_MAXPKTLEN);
			}
			break;

		case CMD_PROBE:
			DEBUG("Received CMD_PROBE packet");
			{
				monitor_probemessageprocessor(rawPacket, AQ_MAXPKTLEN);
			}
			break;

		case CMD_STATUS:
			DEBUG("Received CMD_STATUS packet");
			{
				monitor_statusmessageprocessor(rawPacket, AQ_MAXPKTLEN);
			}
			break;

		case CMD_MSG:
			DEBUG("Received CMD_MSG packet");
			break;

		case CMD_MSG_LONG:
			DEBUG("Received CMD_MSG_LONG packet");
			{
				monitor_msglongmessageprocessor(rawPacket, AQ_MAXPKTLEN);
			}
			break;

		// Aquarite Commands

		case CMD_GETID:
			DEBUG("Received CMD_GETID packet");
			break;

		case CMD_PERCENT:
			DEBUG("Received CMD_PERCENT packet");
			break;

		case CMD_PPM:
			DEBUG("Received CMD_PPM packet");
			break;

		// PDA / OneTouch Commands

		case CMD_PDA_0x05:
			DEBUG("Received CMD_PDA_0x05 packet");
			break;

		case CMD_PDA_0x1B:
			DEBUG("Received CMD_PDA_0x1B packet");
			break;

		case CMD_PDA_HIGHLIGHT:
			DEBUG("Received CMD_PDA_HIGHLIGHT packet");
			break;

		case CMD_PDA_CLEAR:
			DEBUG("Received CMD_PDA_CLEAR packet");
			break;

		case CMD_PDA_SHIFTLINES:
			DEBUG("Received CMD_PDA_SHIFTLINES packet");
			break;

		case CMD_PDA_HIGHLIGHTCHARS:
			DEBUG("Received CMD_PDA_HIGHLIGHTCHARS packet"); 
			break;

		// iAqualink Commands

		case CMD_IAQ_0x23:
			DEBUG("Received CMD_IAQ_0x23 packet");
			break;

		case CMD_IAQ_MENU_MSG:
			DEBUG("Received CMD_IAQ_MENU_MSG packet");
			break;

		case CMD_IAQ_MSG:
			DEBUG("Received CMD_IAQ_MSG packet");
			break;

		case CMD_IAQ_0x28:
			DEBUG("Received CMD_IAQ_0x28 packet");
			break;

		case CMD_IAQ_0x2d:
			DEBUG("Received CMD_IAQ_0x2d packet"); 
			break;

		// Everything else

		default:
			DEBUG("Received unknown packet type");
			{
				monitor_unknownmessageprocessor(rawPacket, AQ_MAXPKTLEN);
			}
			break;
		}
	}
	while(1);
}
