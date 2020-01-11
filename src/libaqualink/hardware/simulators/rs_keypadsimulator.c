#include "rs_keypadsimulator.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

#include "cross-platform/threads.h"
#include "logging/logging.h"
#include "serial/message-processors/aq_serial_message_ack.h"
#include "serial/serializers/aq_serial_message_probe_serializer.h"
#include "serial/aq_serial_types.h"

AqualinkRS_KeypadSimulator aqualink_keypad_simulator =
{
	.Config =
	{
		.IsInitialised = false,
		// .SimulatorAccessMutex
	},

	.IsEnabled = false,
	.Id = 0xFF,

	.FunctionKeypad = { FilterPump, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 },
	.MenuKeypad = { Back, 1, 2, 3, 4, 5, 6 },

	.Initialise = &rs_keypadsimulator_initialise,
	.AckMessageHandler = 0,
	.MsgLongMessageHandler = 0,
	.ProbeMessageHandler = &rs_keypadsimulator_probemessagehandler,
	.StatusMessageHandler = 0,
	.UnknownMessageHandler = 0
};

static bool rs_keypadsimulator_initmutex()
{
	TRACE("Initialising Aqualink RS Keypad Simulator mutex");

	aqualink_keypad_simulator.Config.IsInitialised = (thrd_success == mtx_init(&(aqualink_keypad_simulator.Config.SimulatorAccessMutex), mtx_plain | mtx_recursive));
	
	return aqualink_keypad_simulator.Config.IsInitialised;
}

void rs_keypadsimulator_enable()
{
	if ((!aqualink_keypad_simulator.Config.IsInitialised) && (!rs_keypadsimulator_initmutex()))
	{
		ERROR("Failed to initialise Aqualink RS Keypad Simulator mutex");
	}
	else if (thrd_error == mtx_lock(&(aqualink_keypad_simulator.Config.SimulatorAccessMutex)))
	{
		ERROR("Failed to lock Aqualink RS Keypad Simulator mutex");
	}
	else
	{
		aqualink_keypad_simulator.IsEnabled = true;
		INFO("Aqualink RS Keypad Simulator is now ENABLED");
	}

	if ((aqualink_keypad_simulator.Config.IsInitialised) && (thrd_error == mtx_unlock(&(aqualink_keypad_simulator.Config.SimulatorAccessMutex))))
	{
		ERROR("Failed to unlock Aqualink RS Keypad Simulator mutex");
	}
}

void rs_keypadsimulator_disable()
{
	if ((!aqualink_keypad_simulator.Config.IsInitialised) && (!rs_keypadsimulator_initmutex()))
	{
		ERROR("Failed to initialise Aqualink RS Keypad Simulator mutex");
	}
	else if (thrd_error == mtx_lock(&(aqualink_keypad_simulator.Config.SimulatorAccessMutex)))
	{
		ERROR("Failed to lock Aqualink RS Keypad Simulator mutex");
	}
	else
	{
		aqualink_keypad_simulator.IsEnabled = false;
		INFO("Aqualink keypad simulator is now DISABLED");
	}

	if ((aqualink_keypad_simulator.Config.IsInitialised) && (thrd_error == mtx_unlock(&(aqualink_keypad_simulator.Config.SimulatorAccessMutex))))
	{
		ERROR("Failed to unlock Aqualink RS Keypad Simulator mutex");
	}
}

bool rs_keypadsimulator_initialise()
{
	TRACE("Initialising Aqualink RS Keypad Simulator");

	if ((!aqualink_keypad_simulator.Config.IsInitialised) && (!rs_keypadsimulator_initmutex()))
	{
		ERROR("Failed to initialise Aqualink RS Keypad Simulator mutex");
	}

	return aqualink_keypad_simulator.Config.IsInitialised;
}

bool rs_keypadsimulator_probemessagehandler(AQ_Probe_Packet* packet)
{
	bool handled_probe_message = false;

	if ((!aqualink_keypad_simulator.Config.IsInitialised) && (!rs_keypadsimulator_initmutex()))
	{
		ERROR("Failed to initialise Aqualink RS Keypad Simulator mutex");
	}
	else if (thrd_error == mtx_lock(&(aqualink_keypad_simulator.Config.SimulatorAccessMutex)))
	{
		ERROR("Failed to lock Aqualink RS Keypad Simulator mutex");
	}
	else if (!aqualink_keypad_simulator.IsEnabled)
	{
		WARN("Simulator is DISABLED but was asked to ACK a PROBE request");
	}
	else if (!send_ack_packet(ACK_NORMAL, CMD_PROBE))
	{
		WARN("Failed to send an ACK response to PROBE request");
	}
	else
	{
		TRACE("Transmitted ACK response to PROBE request");
		handled_probe_message = true;
	}
	
	if ((aqualink_keypad_simulator.Config.IsInitialised) && (thrd_error == mtx_unlock(&(aqualink_keypad_simulator.Config.SimulatorAccessMutex))))
	{
		ERROR("Failed to unlock Aqualink RS Keypad Simulator mutex");
	}

	return handled_probe_message;
}
