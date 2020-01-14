#include "onetouch_simulator.h"

#include <assert.h>
#include <stdbool.h>

#include "cross-platform/threads.h"
#include "hardware/simulators/onetouch/onetouch_simulator_private.h"
#include "logging/logging.h"

bool onetouch_simulator_enable()
{
	if ((!aqualink_onetouch_simulator.Config.IsInitialised) && (!onetouch_simulator_initmutex()))
	{
		ERROR("Failed to initialise Aqualink OneTouch Simulator mutex");
	}
	else if (thrd_error == mtx_lock(&(aqualink_onetouch_simulator.Config.SimulatorAccessMutex)))
	{
		ERROR("Failed to lock Aqualink OneTouch Simulator mutex");
	}
	else
	{
		aqualink_onetouch_simulator.IsEnabled = true;
		INFO("Aqualink OneTouch Simulator is now ENABLED");
	}

	if ((aqualink_onetouch_simulator.Config.IsInitialised) && (thrd_error == mtx_unlock(&(aqualink_onetouch_simulator.Config.SimulatorAccessMutex))))
	{
		ERROR("Failed to unlock Aqualink OneTouch Simulator mutex");
	}

	return (true == aqualink_onetouch_simulator.IsEnabled);
}

bool onetouch_simulator_disable()
{
	if ((!aqualink_onetouch_simulator.Config.IsInitialised) && (!onetouch_simulator_initmutex()))
	{
		ERROR("Failed to initialise Aqualink OneTouch Simulator mutex");
	}
	else if (thrd_error == mtx_lock(&(aqualink_onetouch_simulator.Config.SimulatorAccessMutex)))
	{
		ERROR("Failed to lock Aqualink OneTouch Simulator mutex");
	}
	else
	{
		aqualink_onetouch_simulator.IsEnabled = false;
		INFO("Aqualink OneTouch Simulator is now DISABLED");
	}

	if ((aqualink_onetouch_simulator.Config.IsInitialised) && (thrd_error == mtx_unlock(&(aqualink_onetouch_simulator.Config.SimulatorAccessMutex))))
	{
		ERROR("Failed to unlock Aqualink OneTouch Simulator mutex");
	}

	return (false == aqualink_onetouch_simulator.IsEnabled);
}

bool onetouch_simulator_initialise()
{
	TRACE("Initialising Aqualink OneTouch Simulator");

	if ((!aqualink_onetouch_simulator.Config.IsInitialised) && (!onetouch_simulator_initmutex()))
	{
		ERROR("Failed to initialise Aqualink OneTouch Simulator mutex");
	}

	return aqualink_onetouch_simulator.Config.IsInitialised;
}
