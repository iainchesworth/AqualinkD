#include "pda_simulator.h"

#include <assert.h>
#include <stdbool.h>

#include "cross-platform/threads.h"
#include "hardware/simulators/pda/pda_simulator_private.h"
#include "logging/logging.h"

bool pda_simulator_enable()
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

	return (true == aqualink_pda_simulator.IsEnabled);
}

bool pda_simulator_disable()
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

	return (false == aqualink_pda_simulator.IsEnabled);
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
