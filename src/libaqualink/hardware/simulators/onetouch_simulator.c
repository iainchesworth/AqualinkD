#include "onetouch_simulator.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

#include "cross-platform/threads.h"
#include "logging/logging.h"
#include "serial/message-processors/aq_serial_message_ack.h"
#include "serial/serializers/aq_serial_message_msg_long_serializer.h"
#include "serial/aq_serial_types.h"

Aqualink_OneTouchSimulator aqualink_onetouch_simulator =
{
	.Config =
	{
		.IsInitialised = false,
		// .SimulatorAccessMutex
	},

	.IsEnabled = false,
	.Id = 0xFF,

	.Initialise = &onetouch_simulator_initialise,
	.AckMessageHandler = 0,
	.MsgLongMessageHandler = &onetouch_simulator_msglongmessagehandler,
	.ProbeMessageHandler = &onetouch_simulator_probemessagehandler,
	.StatusMessageHandler = 0,
	.UnknownMessageHandler = 0
};

static bool onetouch_simulator_initmutex()
{
	TRACE("Initialising Aqualink OneTouch Simulator mutex");

	aqualink_onetouch_simulator.Config.IsInitialised = (thrd_success == mtx_init(&(aqualink_onetouch_simulator.Config.SimulatorAccessMutex), mtx_plain | mtx_recursive));

	return aqualink_onetouch_simulator.Config.IsInitialised;
}

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

bool onetouch_simulator_probemessagehandler(AQ_Probe_Packet* packet)
{
	bool handled_probe_message = false;

	if ((!aqualink_onetouch_simulator.Config.IsInitialised) && (!onetouch_simulator_initmutex()))
	{
		ERROR("Failed to initialise Aqualink OneTouch Simulator mutex");
	}
	else if (thrd_error == mtx_lock(&(aqualink_onetouch_simulator.Config.SimulatorAccessMutex)))
	{
		ERROR("Failed to lock Aqualink OneTouch Simulator mutex");
	}
	else if (!aqualink_onetouch_simulator.IsEnabled)
	{
		WARN("Aqualink OneTouch Simulator is DISABLED but was asked to ACK a PROBE request");
	}
	else if (!send_ack_packet(ACK_NORMAL, CMD_PROBE))
	{
		WARN("Aqualink OneTouch Simulator failed to send an ACK response to PROBE request");
	}
	else
	{
		TRACE("Aqualink OneTouch Simulator transmitted ACK response to PROBE request");
		handled_probe_message = true;
	}

	if ((aqualink_onetouch_simulator.Config.IsInitialised) && (thrd_error == mtx_unlock(&(aqualink_onetouch_simulator.Config.SimulatorAccessMutex))))
	{
		ERROR("Failed to unlock Aqualink OneTouch Simulator mutex");
	}

	return handled_probe_message;
}

bool onetouch_simulator_msglongmessagehandler(AQ_Msg_Long_Packet* packet)
{
	bool handled_msglong_message = false;

	if ((!aqualink_onetouch_simulator.Config.IsInitialised) && (!onetouch_simulator_initmutex()))
	{
		ERROR("Failed to initialise Aqualink OneTouch Simulator mutex");
	}
	else if (thrd_error == mtx_lock(&(aqualink_onetouch_simulator.Config.SimulatorAccessMutex)))
	{
		ERROR("Failed to lock Aqualink OneTouch Simulator mutex");
	}
	else if (!aqualink_onetouch_simulator.IsEnabled)
	{
		WARN("Aqualink OneTouch Simulator is DISABLED but was asked to handle a MSG LONG message");
	}
	else if (!send_ack_packet(ACK_NORMAL, CMD_MSG_LONG))
	{
		WARN("Aqualink OneTouch Simulator failed to send an ACK response to MSG LONG request");
	}
	else
	{
		TRACE("Simulator successfully handled MSG LONG message");
		handled_msglong_message = true;
	}

	if ((aqualink_onetouch_simulator.Config.IsInitialised) && (thrd_error == mtx_unlock(&(aqualink_onetouch_simulator.Config.SimulatorAccessMutex))))
	{
		ERROR("Failed to unlock Aqualink OneTouch Simulator mutex");
	}

	return handled_msglong_message;
}

bool onetouch_simulator_statusmessagehandler(AQ_Status_Packet* packet)
{
	bool handled_status_message = false;

	if ((!aqualink_onetouch_simulator.Config.IsInitialised) && (!onetouch_simulator_initmutex()))
	{
		ERROR("Failed to initialise Aqualink OneTouch Simulator mutex");
	}
	else if (thrd_error == mtx_lock(&(aqualink_onetouch_simulator.Config.SimulatorAccessMutex)))
	{
		ERROR("Failed to lock Aqualink OneTouch Simulator mutex");
	}
	else if (!aqualink_onetouch_simulator.IsEnabled)
	{
		WARN("Aqualink OneTouch Simulator is DISABLED but was asked to ACK a STATUS request");
	}
	else if (!send_ack_packet(ACK_NORMAL, CMD_STATUS))
	{
		WARN("Aqualink OneTouch Simulator failed to send an ACK response to STATUS request");
	}
	else
	{
		TRACE("Aqualink OneTouch Simulator transmitted ACK response to STATUS request");
		handled_status_message = true;
	}

	if ((aqualink_onetouch_simulator.Config.IsInitialised) && (thrd_error == mtx_unlock(&(aqualink_onetouch_simulator.Config.SimulatorAccessMutex))))
	{
		ERROR("Failed to unlock Aqualink OneTouch Simulator mutex");
	}

	return handled_status_message;
}
