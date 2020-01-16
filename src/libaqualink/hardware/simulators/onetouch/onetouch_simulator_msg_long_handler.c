#include "onetouch_simulator_msg_long_handler.h"
#include "onetouch_simulator_private.h"

#include <stdbool.h>

#include "logging/logging.h"
#include "messages/message-processors/aq_serial_message_ack_handler.h"
#include "messages/aq_serial_message_msg_long.h"

#include "aqualink.h"

bool onetouch_simulator_msglongmessagehandler(AQ_Msg_Long_Packet* packet)
{
	UNREFERENCED_PARAMETER(packet);

	bool handled_msglong_message = false;

	if ((!aqualink_onetouch_simulator.Config.IsInitialised) && (!onetouch_simulator_initmutex()))
	{
		ERROR("Failed to initialise Aqualink OneTouch Simulator mutex");
	}
	else if (thrd_error == mtx_lock(&(aqualink_onetouch_simulator.Config.SimulatorAccessMutex)))
	{
		ERROR("Failed to lock Aqualink OneTouch Simulator mutex");
	}
	else if (!aqualink_onetouch_simulator.IsEnabled)
	{
		WARN("Aqualink OneTouch Simulator is DISABLED but was asked to handle a MSG LONG message");
	}
	else if (!send_ack_packet(ACK_NORMAL, CMD_MSG_LONG))
	{
		WARN("Aqualink OneTouch Simulator failed to send an ACK response to MSG LONG request");
	}
	else
	{
		DEBUG("Simulator successfully handled MSG LONG message");
		handled_msglong_message = true;
	}

	if ((aqualink_onetouch_simulator.Config.IsInitialised) && (thrd_error == mtx_unlock(&(aqualink_onetouch_simulator.Config.SimulatorAccessMutex))))
	{
		ERROR("Failed to unlock Aqualink OneTouch Simulator mutex");
	}

	return handled_msglong_message;
}
