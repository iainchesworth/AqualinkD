#include "pda_simulator.h"

#include <assert.h>
#include <stdbool.h>

#include "cross-platform/threads.h"
#include "logging/logging.h"
#include "simulators/pda/pda_simulator_private.h"
#include "simulators/pda/pda_simulator_thread.h"
#include "simulators/pda/pda_simulator_types.h"

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

bool pda_simulator_initialise(MessageBus* simulator_message_bus)
{
	assert(0 != simulator_message_bus);

	TRACE("Initialising Aqualink PDA Simulator");

	if ((!aqualink_pda_simulator.Config.IsInitialised) && (!pda_simulator_initmutex()))
	{
		ERROR("Failed to initialise Aqualink PDA Simulator mutex");
	}
	else
	{
		messagebus_topic_init(&pda_simulator_messagetopic, pda_simulator_messagetopic_buffer, TOPIC_MAX_MSG_LENGTH);
		messagebus_advertise_topic_by_type(simulator_message_bus, &pda_simulator_messagetopic, aqualink_pda_simulator.Id.Type);

		if (thrd_error == thrd_create(&pda_simulator_thread, pda_simulator_processing_thread, 0))
		{
			ERROR("Failed to create Aqualink PDA Simulator Thread");
		}
		else
		{
			DEBUG("PDA Simulator thread crated successfully");
		}
	}

	return aqualink_pda_simulator.Config.IsInitialised;
}
