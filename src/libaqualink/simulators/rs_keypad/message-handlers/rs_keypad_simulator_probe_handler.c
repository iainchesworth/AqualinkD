#include "rs_keypad_simulator_probe_handler.h"

#include <stdbool.h>

#include "logging/logging.h"
#include "messages/aq_serial_message_probe.h"
#include "simulators/rs_keypad/rs_keypad_simulator_private.h"

bool rs_keypad_simulator_probemessagehandler(AQ_Probe_Packet* packet)
{
	UNREFERENCED_PARAMETER(packet);

	bool handled_probe_message = false;

	if ((!aqualink_rs_keypad_simulator.Config.IsInitialised) && (!rs_keypad_simulator_initmutex()))
	{
		ERROR("Failed to initialise Aqualink RS Keypad Simulator mutex");
	}
	else if (thrd_error == mtx_lock(&(aqualink_rs_keypad_simulator.Config.SimulatorAccessMutex)))
	{
		ERROR("Failed to lock Aqualink RS Keypad Simulator mutex");
	}
	else if (!aqualink_rs_keypad_simulator.IsEnabled)
	{
		WARN("Simulator is DISABLED but was asked to ACK a PROBE request");
	}
	else
	{
		TRACE("Transmitted ACK response to PROBE request");
		handled_probe_message = true;
	}

	if ((aqualink_rs_keypad_simulator.Config.IsInitialised) && (thrd_error == mtx_unlock(&(aqualink_rs_keypad_simulator.Config.SimulatorAccessMutex))))
	{
		ERROR("Failed to unlock Aqualink RS Keypad Simulator mutex");
	}

	return handled_probe_message;
}
