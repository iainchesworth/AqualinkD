#include "pda_simulator_probe_handler.h"

#include <stdbool.h>

#include "logging/logging.h"
#include "messages/message-processors/aq_serial_message_senders.h"
#include "messages/aq_serial_message_probe.h"
#include "simulators/pda/pda_simulator_private.h"

#include "aqualink.h"

bool pda_simulator_probemessagehandler(AQ_Probe_Packet* packet)
{
	UNREFERENCED_PARAMETER(packet);

	bool handled_probe_message = false;

	if ((!aqualink_pda_simulator.Config.IsInitialised) && (!pda_simulator_initmutex()))
	{
		ERROR("Failed to initialise Aqualink PDA Simulator mutex");
	}
	else if (thrd_error == mtx_lock(&(aqualink_pda_simulator.Config.SimulatorAccessMutex)))
	{
		ERROR("Failed to lock Aqualink PDA Simulator mutex");
	}
	else if (!aqualink_pda_simulator.IsEnabled)
	{
		WARN("Aqualink PDA Simulator is DISABLED but was asked to ACK a PROBE request");
	}
	else
	{
		DEBUG("Aqualink PDA Simulator transmitted ACK response to PROBE request");
		handled_probe_message = true;
	}

	if ((aqualink_pda_simulator.Config.IsInitialised) && (thrd_error == mtx_unlock(&(aqualink_pda_simulator.Config.SimulatorAccessMutex))))
	{
		ERROR("Failed to unlock Aqualink PDA Simulator mutex");
	}

	return handled_probe_message;
}
