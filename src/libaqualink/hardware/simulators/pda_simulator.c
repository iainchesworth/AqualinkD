#include "pda_simulator.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

#include "cross-platform/threads.h"
#include "logging/logging.h"

Aqualink_PDASimulator aqualink_pda_simulator =
{
	.Config =
	{
		.IsInitialised = false,
		// .SimulatorAccessMutex
	},

	.IsEnabled = false,
	.Id = 0xFF,

	.Initialise = &pda_simulator_initialise,
	.ProbeMessageHandler = 0,
	.MsgLongMessageHandler = &pda_simulator_msglongmessagehandler
};

static bool pda_simulator_initmutex()
{
	TRACE("Initialising Aqualink PDA Simulator mutex");

	aqualink_pda_simulator.Config.IsInitialised = (thrd_success == mtx_init(&(aqualink_pda_simulator.Config.SimulatorAccessMutex), mtx_plain | mtx_recursive));

	return aqualink_pda_simulator.Config.IsInitialised;
}

void pda_simulator_enable()
{
	if ((!aqualink_pda_simulator.Config.IsInitialised) && (!pda_simulator_initmutex()))
	{
		ERROR("Failed to initialise Aqualink PDA Simulator mutex");
	}
	else if (thrd_error == mtx_lock(&(aqualink_pda_simulator.Config.SimulatorAccessMutex)))
	{
		ERROR("Failed to lock Aqualink PDA Simulator mutex");
	}
	else
	{
		aqualink_pda_simulator.IsEnabled = true;
		INFO("Aqualink PDA Simulator is now ENABLED");
	}

	if ((aqualink_pda_simulator.Config.IsInitialised) && (thrd_error == mtx_unlock(&(aqualink_pda_simulator.Config.SimulatorAccessMutex))))
	{
		ERROR("Failed to unlock Aqualink PDA Simulator mutex");
	}
}

void pda_simulator_disable()
{
	if ((!aqualink_pda_simulator.Config.IsInitialised) && (!pda_simulator_initmutex()))
	{
		ERROR("Failed to initialise Aqualink PDA Simulator mutex");
	}
	else if (thrd_error == mtx_lock(&(aqualink_pda_simulator.Config.SimulatorAccessMutex)))
	{
		ERROR("Failed to lock Aqualink PDA Simulator mutex");
	}
	else
	{
		aqualink_pda_simulator.IsEnabled = false;
		INFO("Aqualink PDA Simulator is now DISABLED");
	}

	if ((aqualink_pda_simulator.Config.IsInitialised) && (thrd_error == mtx_unlock(&(aqualink_pda_simulator.Config.SimulatorAccessMutex))))
	{
		ERROR("Failed to unlock Aqualink PDA Simulator mutex");
	}
}

bool pda_simulator_initialise()
{
	TRACE("Initialising Aqualink PDA Simulator");

	if ((!aqualink_pda_simulator.Config.IsInitialised) && (!pda_simulator_initmutex()))
	{
		ERROR("Failed to initialise Aqualink PDA Simulator mutex");
	}

	return aqualink_pda_simulator.Config.IsInitialised;
}

bool pda_simulator_msglongmessagehandler()
{
	bool handled_msglong_message = false;

	if ((!aqualink_pda_simulator.Config.IsInitialised) && (!pda_simulator_initmutex()))
	{
		ERROR("Failed to initialise Aqualink RS Keypad Simulator mutex");
	}
	else if (thrd_error == mtx_lock(&(aqualink_pda_simulator.Config.SimulatorAccessMutex)))
	{
		ERROR("Failed to lock Aqualink RS Keypad Simulator mutex");
	}
	else if (!aqualink_pda_simulator.IsEnabled)
	{
		WARN("Simulator is DISABLED but was asked to handle a MSG LONG message");
	}
	else
	{
		TRACE("Simulator successfully handled MSG LONG message");
		handled_msglong_message = true;
	}

	if ((aqualink_pda_simulator.Config.IsInitialised) && (thrd_error == mtx_unlock(&(aqualink_pda_simulator.Config.SimulatorAccessMutex))))
	{
		ERROR("Failed to unlock Aqualink PDA Simulator mutex");
	}

	return handled_msglong_message;
}
