#include "rs_keypadsimulator.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

#include "cross-platform/threads.h"
#include "logging/logging.h"
#include "serial/message-processors/aq_serial_message_ack.h"
#include "serial/aq_serial_types.h"

static mtx_t* aqualinkrs_keypadsimulator_mutex = 0;

void enable_aqualinkrs_keypadsimulator(AqualinkRS_KeypadSimulator* simulator)
{
	assert(0 != simulator);

	if (0 == aqualinkrs_keypadsimulator_mutex)
	{
		TRACE("Initialising Aqualink RS Keypad Simulator mutex");

		if (0 == (aqualinkrs_keypadsimulator_mutex = (mtx_t*)malloc(sizeof(mtx_t))))
		{
			ERROR("Failed to allocate memory for Aqualink RS Keypad Simulator mutex");
		}
		if (thrd_error == mtx_init(aqualinkrs_keypadsimulator_mutex, mtx_plain | mtx_recursive))
		{
			ERROR("Failed to initialise Aqualink RS Keypad Simulator mutex");
		}
		else
		{
			DEBUG("Aqualink RS Keypad Simulator mutex successfully initialised");
		}
	}

	if (thrd_error == mtx_lock(aqualinkrs_keypadsimulator_mutex))
	{
		ERROR("Failed to lock Aqualink RS Keypad Simulator mutex");
	}
	else
	{
		simulator->IsEnabled = true;
		INFO("Aqualink keypad simulator is now ENABLED");
	}

	if ((0 != aqualinkrs_keypadsimulator_mutex) && (thrd_error == mtx_unlock(aqualinkrs_keypadsimulator_mutex)))
	{
		ERROR("Failed to unlock Aqualink RS Keypad Simulator mutex");
	}
}

void disable_aqualinkrs_keypadsimulator(AqualinkRS_KeypadSimulator* simulator)
{
	assert(0 != aqualinkrs_keypadsimulator_mutex);
	assert(0 != simulator);

	if (thrd_error == mtx_lock(aqualinkrs_keypadsimulator_mutex))
	{
		ERROR("Failed to lock Aqualink RS Keypad Simulator mutex");
	}
	else
	{
		simulator->IsEnabled = false;
		INFO("Aqualink keypad simulator is now DISABLED");
	}

	if ((0 != aqualinkrs_keypadsimulator_mutex) && (thrd_error == mtx_unlock(aqualinkrs_keypadsimulator_mutex)))
	{
		ERROR("Failed to unlock Aqualink RS Keypad Simulator mutex");
	}
}

bool aqualinkrs_keypadsimulator_probemessagehandler(AqualinkRS_KeypadSimulator* simulator)
{
	assert(0 != aqualinkrs_keypadsimulator_mutex);
	assert(0 != simulator);

	bool handled_probe_message = false;

	if (thrd_error == mtx_lock(aqualinkrs_keypadsimulator_mutex))
	{
		ERROR("Failed to lock Aqualink RS Keypad Simulator mutex");
	}
	else
	{
		if (simulator->IsEnabled)
		{
			WARN("Simulator is DISABLED but was asked to ACK a PROBE request");
		}
		else if (!send_ack_packet(ACK_NORMAL, CMD_PROBE))
		{
			WARN("Failed to send an ACK response to PROBE request");
		}
		else
		{
			TRACE("Transmitted ACK response to PROBE request");
			handled_probe_message = true;
		}
	}

	if ((0 != aqualinkrs_keypadsimulator_mutex) && (thrd_error == mtx_unlock(aqualinkrs_keypadsimulator_mutex)))
	{
		ERROR("Failed to unlock Aqualink RS Keypad Simulator mutex");
	}

	return handled_probe_message;
}
