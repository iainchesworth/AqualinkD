#include "onetouch_simulator_thread.h"
#include "onetouch_simulator_private.h"
#include "onetouch_simulator_screen.h"

#include "logging/logging.h"
#include "messages/message-bus/aq_serial_message_bus.h"
#include "messages/message-serializers/aq_serial_message_status_serializer.h"
#include "messages/aq_serial_message_status.h"
#include "serial/aq_serial.h"

#include "aqualink.h"

int onetouch_simulator_processing_thread(void* argument)
{
	DEBUG("Starting OneTouch Simulator Thread");

	UNREFERENCED_PARAMETER(argument);

	const unsigned int COMMAND_ID_BYTE_LOCATION = 3;
	unsigned char rawPacket[AQ_MAXPKTLEN];

	do
	{
		// Clearing the packet buffer to make sure that there's no data carry-over.
		memset(rawPacket, 0, AQ_MAXPKTLEN);

		// Wait for an incoming message.
		TRACE("Waiting for messages on the OneTouch Simulator message topic");
		messagebus_topic_wait(&onetouch_simulator_messagetopic, rawPacket, AQ_MAXPKTLEN);

		if (!aqualink_onetouch_simulator.IsEnabled)
		{
			TRACE("Simulator is DISABLED; ignoring message");
		}
		else
		{
			TRACE("Received message for OneTouch Simulator...processing it");
			switch (rawPacket[COMMAND_ID_BYTE_LOCATION])
			{
			case CMD_ACK:
				break;

			case CMD_STATUS:
				TRACE("OneTouch Simulator - STATUS message received");
				{
					AQ_Status_Packet processedPacket;
					deserialize_status_packet(&processedPacket, rawPacket, AQ_MAXPKTLEN);

					if (0 == aqualink_onetouch_simulator.StatusMessageHandler)
					{
						TRACE("OneTouch Simulator - No STATUS message handler installed!");
					}
					else if (aqualink_onetouch_simulator.StatusMessageHandler(&processedPacket))
					{
						TRACE("OneTouch Simulator - STATUS message handler failed to process message");
					}
					else
					{
						TRACE("OneTouch Simulator - STATUS message handler processed message successfully");
					}
				}
				break;

			case CMD_MSG_LONG:
				TRACE("OneTouch Simulator - MSG LONG message received");
				{
					AQ_Msg_Long_Packet processedPacket;
					deserialize_msg_long_packet(&processedPacket, rawPacket, AQ_MAXPKTLEN);

					if (0 == aqualink_onetouch_simulator.MsgLongMessageHandler)
					{
						TRACE("OneTouch Simulator - No MSG LONG message handler installed!");
					}
					else if (aqualink_onetouch_simulator.MsgLongMessageHandler(&processedPacket))
					{
						TRACE("OneTouch Simulator - MSG LONG message handler failed to process message");
					}
					else
					{
						TRACE("OneTouch Simulator - MSG LONG message handler processed message successfully");
					}
				}
				break;

			case CMD_PDA_CLEAR:
				TRACE("OneTouch Simulator - PDA CLEAR message received");
				{
					onetouch_screen_clear_all_pages();
					DEBUG("Simulator successfully handled PDA CLEAR message");
				}
				break;

			case CMD_PDA_HIGHLIGHT:
				TRACE("OneTouch Simulator - PDA HIGHLIGHT message received");
				{
					DEBUG("Simulator successfully handled PDA HIGHLIGHT message");
				}
				break;

			default:
				DEBUG("UNEXPECTED MESSAGE - OneTouch Simulator - Message Id: 0x%02x", rawPacket[COMMAND_ID_BYTE_LOCATION]);
				log_serial_packet(rawPacket, 64, false);
				break;
			}
		}

	}
	while (1);

	return 0;
}
