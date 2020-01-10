#include "aq_serial_messages.h"

#include <assert.h>
#include "logging/logging.h"
#include "serial/message-processors/aq_serial_message_ack.h"
#include "serial/message-processors/aq_serial_message_msg_long.h"
#include "serial/message-processors/aq_serial_message_probe.h"
#include "serial/message-processors/aq_serial_message_status.h"
#include "serial/message-processors/aq_serial_message_unknown.h"
#include "aq_serial.h"
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
		DEBUG("Received CMD_ACK packet");
		returnCode = process_ack_packet(rawPacket, length);
		break;

	case CMD_STATUS:
		DEBUG("Received CMD_STATUS packet");
		returnCode = process_status_packet(rawPacket, length);
		break;

	case CMD_MSG:
		DEBUG("Received CMD_MSG packet");
		break;

	case CMD_MSG_LONG:
		DEBUG("Received CMD_MSG_LONG packet");
		returnCode = process_msg_long_packet(rawPacket, length);
		break;

	case CMD_PROBE:
		DEBUG("Received CMD_PROBE packet");
		returnCode = process_probe_packet(rawPacket, length);
		break;

	case CMD_GETID:
		DEBUG("Received CMD_GETID packet");
		break;

	case CMD_PERCENT:
		DEBUG("Received CMD_PERCENT packet");
		break;

	case CMD_PPM:
		DEBUG("Received CMD_PPM packet");
		break;

	case CMD_PDA_0x05:	// PDA UNKNOWN
		DEBUG("Received CMD_PDA_0x05 packet");
		break;

	case CMD_PDA_0x1B:	// PDA INIT (guess)
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

	case CMD_IAQ_MSG:
		DEBUG("Received CMD_IAQ_MSG packet");
		break;

	case CMD_IAQ_MENU_MSG:
		DEBUG("Received CMD_IAQ_MENU_MSG packet");
		break;

	default:
		DEBUG("Received unknown packet type");
		returnCode = process_unknown_packet(rawPacket, length);
		break;
	}

	return true;
}
