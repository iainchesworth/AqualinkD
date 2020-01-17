#include "onetouch_simulator_status_handler.h"
#include "onetouch_simulator_private.h"

#include <stdbool.h>

#include "logging/logging.h"
#include "messages/message-processors/aq_serial_message_ack_handler.h"
#include "messages/aq_serial_message_status.h"

#include "aqualink.h"

bool onetouch_simulator_statusmessagehandler(AQ_Status_Packet* packet)
{
	UNREFERENCED_PARAMETER(packet);

	bool handled_status_message = false;

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
		WARN("Aqualink OneTouch Simulator is DISABLED but was asked to ACK a STATUS request");
	}
	else if (!send_ack_packet(ACK_NORMAL, CMD_STATUS))
	{
		WARN("Aqualink OneTouch Simulator failed to send an ACK response to STATUS request");
	}
	else
	{
		DEBUG("Aqualink OneTouch Simulator transmitted ACK response to STATUS request");
		handled_status_message = true;
	}

	if ((aqualink_onetouch_simulator.Config.IsInitialised) && (thrd_error == mtx_unlock(&(aqualink_onetouch_simulator.Config.SimulatorAccessMutex))))
	{
		ERROR("Failed to unlock Aqualink OneTouch Simulator mutex");
	}

	return handled_status_message;
}
