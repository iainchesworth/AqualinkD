#include "rs_keypad_simulator_private.h"
#include "rs_keypad_simulator_types.h"

#include <stdbool.h>

#include "cross-platform/threads.h"
#include "hardware/buttons/rs_buttons.h"
#include "hardware/devices/hardware_device_types.h"
#include "logging/logging.h"
#include "simulators/rs_keypad/message-handlers/rs_keypad_simulator_probe_handler.h"
#include "simulators/rs_keypad_simulator.h"

Aqualink_RS_Keypad_Simulator aqualink_rs_keypad_simulator =
{
	.Config =
	{
		.IsInitialised = false,
		// .SimulatorAccessMutex
	},

	.IsEnabled = false,
	.Id = {.Type = Keypad, .Instance = Instance_0 },

	.FunctionKeypad = { FilterPump, SpaMode, Aux_1, Aux_2, Aux_3, Aux_4, Aux_5, Aux_6, Aux_7, PoolHeater, SpaHeater, SolarHeater },
	.MenuKeypad = { Menu, Cancel, Back, Forward, Enter, Hold, Override },

	.Initialise = &rs_keypad_simulator_initialise,
	.AckMessageHandler = 0,
	.MsgLongMessageHandler = 0,
	.ProbeMessageHandler = &rs_keypad_simulator_probemessagehandler,
	.StatusMessageHandler = 0,
	.UnknownMessageHandler = 0
};

unsigned char rs_keypad_simulator_messagetopic_buffer[TOPIC_MAX_MSG_LENGTH];
MessageBus_Topic rs_keypad_simulator_messagetopic;
thrd_t rs_keypad_simulator_thread;

bool rs_keypad_simulator_initmutex()
{
	TRACE("Initialising Aqualink RS Keypad Simulator mutex");

	aqualink_rs_keypad_simulator.Config.IsInitialised = (thrd_success == mtx_init(&(aqualink_rs_keypad_simulator.Config.SimulatorAccessMutex), mtx_plain | mtx_recursive));

	return aqualink_rs_keypad_simulator.Config.IsInitialised;
}
