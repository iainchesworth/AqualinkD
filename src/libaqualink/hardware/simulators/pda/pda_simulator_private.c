#include "pda_simulator_private.h"
#include "pda_simulator_msg_long_handler.h"
#include "pda_simulator_probe_handler.h"
#include "pda_simulator_status_handler.h"
#include "pda_simulator_types.h"

#include <stdbool.h>

#include "cross-platform/threads.h"
#include "hardware/devices/hardware_device_types.h"
#include "hardware/simulators/pda_simulator.h"
#include "logging/logging.h"

Aqualink_PDASimulator aqualink_pda_simulator =
{
	.Config =
	{
		.IsInitialised = false,
		// .SimulatorAccessMutex
	},

	.IsEnabled = false,
	.Id = {.Type = PDA_Remote, .Instance = Instance_0 },

	.Initialise = &pda_simulator_initialise,
	.AckMessageHandler = 0,
	.MsgLongMessageHandler = &pda_simulator_msglongmessagehandler,
	.ProbeMessageHandler = &pda_simulator_probemessagehandler,
	.StatusMessageHandler = &pda_simulator_statusmessagehandler,
	.UnknownMessageHandler = 0
};

bool pda_simulator_initmutex()
{
	TRACE("Initialising Aqualink PDA Simulator mutex");

	aqualink_pda_simulator.Config.IsInitialised = (thrd_success == mtx_init(&(aqualink_pda_simulator.Config.SimulatorAccessMutex), mtx_plain | mtx_recursive));

	return aqualink_pda_simulator.Config.IsInitialised;
}
