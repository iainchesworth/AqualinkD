#include "aq_serial_messages.h"

#include <assert.h>
#include "aq_serial.h"
#include "aq_serial_message_ack.h"
#include "aq_serial_message_probe.h"
#include "aq_serial_message_status.h"
#include "aq_serial_message_unknown.h"
#include "aq_serial_types.h"
#include "utils.h"

bool process_aqualink_packet(unsigned char* rawPacket, unsigned int length)
{
	static const int COMMAND_ID_BYTE_LOCATION = 3;

	assert(0 != rawPacket);
	assert(COMMAND_ID_BYTE_LOCATION <= length);

	bool returnCode = false;

	switch (rawPacket[COMMAND_ID_BYTE_LOCATION])
	{
	case CMD_ACK:
		logMessage(LOG_DEBUG, "AQ_Serial_Messages.c | process_aqualink_packet | Received CMD_ACK packet\n");
		returnCode = process_ack_packet(rawPacket, length);
		break;

	case CMD_STATUS:
		logMessage(LOG_DEBUG, "AQ_Serial_Messages.c | process_aqualink_packet | Received CMD_STATUS packet\n");
		returnCode = process_status_packet(rawPacket, length);
		break;

	case CMD_MSG:
		logMessage(LOG_DEBUG, "AQ_Serial_Messages.c | process_aqualink_packet | Received CMD_MSG packet\n");
		break;

	case CMD_MSG_LONG:
		logMessage(LOG_DEBUG, "AQ_Serial_Messages.c | process_aqualink_packet | Received CMD_MSG_LONG packet\n");
		break;

	case CMD_PROBE:
		logMessage(LOG_DEBUG, "AQ_Serial_Messages.c | process_aqualink_packet | Received CMD_PROBE packet\n");
		returnCode = process_probe_packet(rawPacket, length);
		break;

	case CMD_GETID:
		logMessage(LOG_DEBUG, "AQ_Serial_Messages.c | process_aqualink_packet | Received CMD_GETID packet\n");
		break;

	case CMD_PERCENT:
		logMessage(LOG_DEBUG, "AQ_Serial_Messages.c | process_aqualink_packet | Received CMD_PERCENT packet\n");
		break;

	case CMD_PPM:
		logMessage(LOG_DEBUG, "AQ_Serial_Messages.c | process_aqualink_packet | Received CMD_PPM packet\n");
		break;

	case CMD_PDA_0x05:	// PDA UNKNOWN
		logMessage(LOG_DEBUG, "AQ_Serial_Messages.c | process_aqualink_packet | Received CMD_PDA_0x05 packet\n");
		break;

	case CMD_PDA_0x1B:	// PDA INIT (guess)
		logMessage(LOG_DEBUG, "AQ_Serial_Messages.c | process_aqualink_packet | Received CMD_PDA_0x1B packet\n");
		break;

	case CMD_PDA_HIGHLIGHT:
		logMessage(LOG_DEBUG, "AQ_Serial_Messages.c | process_aqualink_packet | Received CMD_PDA_HIGHLIGHT packet\n");
		break;

	case CMD_PDA_CLEAR:
		logMessage(LOG_DEBUG, "AQ_Serial_Messages.c | process_aqualink_packet | Received CMD_PDA_CLEAR packet\n");
		break;

	case CMD_PDA_SHIFTLINES:
		logMessage(LOG_DEBUG, "AQ_Serial_Messages.c | process_aqualink_packet | Received CMD_PDA_SHIFTLINES packet\n");
		break;

	case CMD_PDA_HIGHLIGHTCHARS:
		logMessage(LOG_DEBUG, "AQ_Serial_Messages.c | process_aqualink_packet | Received CMD_PDA_HIGHLIGHTCHARS packet\n");
		break;

	case CMD_IAQ_MSG:
		logMessage(LOG_DEBUG, "AQ_Serial_Messages.c | process_aqualink_packet | Received CMD_IAQ_MSG packet\n");
		break;

	case CMD_IAQ_MENU_MSG:
		logMessage(LOG_DEBUG, "AQ_Serial_Messages.c | process_aqualink_packet | Received CMD_IAQ_MENU_MSG packet\n");
		break;

	default:
		logMessage(LOG_DEBUG, "AQ_Serial_Messages.c | process_aqualink_packet | Received unknown packet type\n");
		returnCode = process_unknown_packet(rawPacket, length);
		break;
	}

	return true;
}
