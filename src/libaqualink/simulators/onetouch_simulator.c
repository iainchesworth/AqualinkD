#include "onetouch_simulator.h"

#include <assert.h>
#include <stdbool.h>

#include "cross-platform/threads.h"
#include "logging/logging.h"
#include "simulators/onetouch/onetouch_simulator_private.h"
#include "simulators/onetouch/onetouch_simulator_screen.h"
#include "simulators/onetouch/onetouch_simulator_thread.h"
#include "simulators/onetouch/onetouch_simulator_types.h"

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

bool onetouch_simulator_initialise(MessageBus* simulator_message_bus)
{
	assert(0 != simulator_message_bus);

	TRACE("Initialising Aqualink OneTouch Simulator");

	if ((!aqualink_onetouch_simulator.Config.IsInitialised) && (!onetouch_simulator_initmutex()))
	{
		ERROR("Failed to initialise Aqualink OneTouch Simulator mutex");
	}
	else
	{
		// Perform any required initialisation.
		onetouch_screen_clear_all_pages();

		messagebus_topic_init(&onetouch_simulator_messagetopic, onetouch_simulator_messagetopic_buffer, TOPIC_MAX_MSG_LENGTH);
		messagebus_advertise_topic_by_type(simulator_message_bus, &onetouch_simulator_messagetopic, aqualink_onetouch_simulator.Id.Type);

		if (thrd_error == thrd_create(&onetouch_simulator_thread, onetouch_simulator_processing_thread, 0))
		{
			ERROR("Failed to create Aqualink OneTouch Simulator Thread");
		}
		else
		{
			DEBUG("OneTouch Simulator thread crated successfully");
		}
	}

	return aqualink_onetouch_simulator.Config.IsInitialised;
}
