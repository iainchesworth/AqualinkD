#include "rs_keypad_simulator.h"

#include <assert.h>
#include <stdbool.h>

#include "cross-platform/threads.h"
#include "logging/logging.h"
#include "simulators/rs_keypad/rs_keypad_simulator_private.h"
#include "simulators/rs_keypad/rs_keypad_simulator_thread.h"
#include "simulators/rs_keypad/rs_keypad_simulator_types.h"

bool rs_keypad_simulator_initialise(MessageBus* simulator_message_bus)
{
	assert(0 != simulator_message_bus);

	TRACE("Initialising Aqualink RS Keypad Simulator");

	if ((!aqualink_rs_keypad_simulator.Config.IsInitialised) && (!rs_keypad_simulator_initmutex()))
	{
		ERROR("Failed to initialise Aqualink RS Keypad Simulator mutex");
	}
	else
	{
		messagebus_topic_init(&rs_keypad_simulator_messagetopic, rs_keypad_simulator_messagetopic_buffer, TOPIC_MAX_MSG_LENGTH);
		messagebus_advertise_topic_by_type(simulator_message_bus, &rs_keypad_simulator_messagetopic, aqualink_rs_keypad_simulator.Id.Type);


		if (thrd_error == thrd_create(&rs_keypad_simulator_thread, rs_keypad_simulator_processing_thread, 0))
		{
			ERROR("Failed to create Aqualink RS Keypad Simulator Thread");
		}
		else
		{
			DEBUG("PDA Simulator thread created successfully");
		}
	}

	return aqualink_rs_keypad_simulator.Config.IsInitialised;
}

bool rs_keypad_simulator_enable()
{
	if ((!aqualink_rs_keypad_simulator.Config.IsInitialised) && (!rs_keypad_simulator_initmutex()))
	{
		ERROR("Failed to initialise Aqualink RS Keypad Simulator mutex");
	}
	else if (thrd_error == mtx_lock(&(aqualink_rs_keypad_simulator.Config.SimulatorAccessMutex)))
	{
		ERROR("Failed to lock Aqualink RS Keypad Simulator mutex");
	}
	else
	{
		aqualink_rs_keypad_simulator.IsEnabled = true;
		INFO("Aqualink RS Keypad Simulator is now ENABLED");
	}

	if ((aqualink_rs_keypad_simulator.Config.IsInitialised) && (thrd_error == mtx_unlock(&(aqualink_rs_keypad_simulator.Config.SimulatorAccessMutex))))
	{
		ERROR("Failed to unlock Aqualink RS Keypad Simulator mutex");
	}

	return (true == aqualink_rs_keypad_simulator.IsEnabled);
}

bool rs_keypad_simulator_disable()
{
	if ((!aqualink_rs_keypad_simulator.Config.IsInitialised) && (!rs_keypad_simulator_initmutex()))
	{
		ERROR("Failed to initialise Aqualink RS Keypad Simulator mutex");
	}
	else if (thrd_error == mtx_lock(&(aqualink_rs_keypad_simulator.Config.SimulatorAccessMutex)))
	{
		ERROR("Failed to lock Aqualink RS Keypad Simulator mutex");
	}
	else
	{
		aqualink_rs_keypad_simulator.IsEnabled = false;
		INFO("Aqualink keypad simulator is now DISABLED");
	}

	if ((aqualink_rs_keypad_simulator.Config.IsInitialised) && (thrd_error == mtx_unlock(&(aqualink_rs_keypad_simulator.Config.SimulatorAccessMutex))))
	{
		ERROR("Failed to unlock Aqualink RS Keypad Simulator mutex");
	}

	return (false == aqualink_rs_keypad_simulator.IsEnabled);
}
