#include "onetouch_simulator_private.h"
#include "onetouch_simulator_types.h"

#include <stdbool.h>

#include "cross-platform/threads.h"
#include "hardware/devices/hardware_device_types.h"
#include "logging/logging.h"
#include "messages/message-bus/aq_serial_message_bus.h"
#include "simulators/onetouch/message-handlers/onetouch_simulator_msg_long_handler.h"
#include "simulators/onetouch/message-handlers/onetouch_simulator_probe_handler.h"
#include "simulators/onetouch/message-handlers/onetouch_simulator_status_handler.h"
#include "simulators/onetouch_simulator.h"

Aqualink_OneTouchSimulator aqualink_onetouch_simulator =
{
	.Config =
	{
		.IsInitialised = false,
		// .SimulatorAccessMutex
	},

	.IsEnabled = false,
	.Id = {.Type = OneTouch, .Instance = Instance_1 },

	.Initialise = &onetouch_simulator_initialise,
	.AckMessageHandler = 0,
	.MsgLongMessageHandler = &onetouch_simulator_msglongmessagehandler,
	.ProbeMessageHandler = &onetouch_simulator_probemessagehandler,
	.StatusMessageHandler = &onetouch_simulator_statusmessagehandler,
	.UnknownMessageHandler = 0
};

unsigned char onetouch_simulator_messagetopic_buffer[TOPIC_MAX_MSG_LENGTH];
MessageBus_Topic onetouch_simulator_messagetopic;
thrd_t onetouch_simulator_thread;

bool onetouch_simulator_initmutex()
{
	TRACE("Initialising Aqualink OneTouch Simulator mutex");

	aqualink_onetouch_simulator.Config.IsInitialised = (thrd_success == mtx_init(&(aqualink_onetouch_simulator.Config.SimulatorAccessMutex), mtx_plain | mtx_recursive));

	return aqualink_onetouch_simulator.Config.IsInitialised;
}
