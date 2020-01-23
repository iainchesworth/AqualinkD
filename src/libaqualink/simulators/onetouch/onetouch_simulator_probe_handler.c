#include "onetouch_simulator_probe_handler.h"
#include "onetouch_simulator_private.h"

#include <stdbool.h>

#include "logging/logging.h"
#include "messages/message-processors/aq_serial_message_ack_handler.h"
#include "messages/aq_serial_message_probe.h"

#include "aqualink.h"

bool onetouch_simulator_probemessagehandler(AQ_Probe_Packet* packet)
{
	UNREFERENCED_PARAMETER(packet);

	bool handled_probe_message = false;

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
		WARN("Aqualink OneTouch Simulator is DISABLED but was asked to ACK a PROBE request");
	}
	else
	{
		DEBUG("Simulator successfully handled PROBE message");
		handled_probe_message = true;
	}

	if ((aqualink_onetouch_simulator.Config.IsInitialised) && (thrd_error == mtx_unlock(&(aqualink_onetouch_simulator.Config.SimulatorAccessMutex))))
	{
		ERROR("Failed to unlock Aqualink OneTouch Simulator mutex");
	}

	return handled_probe_message;
}
