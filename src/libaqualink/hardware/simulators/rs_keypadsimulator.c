#include "rs_keypadsimulator.h"

#include <assert.h>
#include <stdbool.h>

#include "logging/logging.h"
#include "serial/aq_serial_message_ack.h"

void enable_aqualinkrs_keypadsimulator(AqualinkRS_KeypadSimulator* simulator)
{
	assert(0 != simulator);

	simulator->IsEnabled = true;
}

void disable_aqualinkrs_keypadsimulator(AqualinkRS_KeypadSimulator* simulator)
{
	assert(0 != simulator);

	simulator->IsEnabled = false;
}

bool aqualinkrs_keypadsimulator_probemessagehandler(AqualinkRS_KeypadSimulator* simulator)
{
	assert(0 != simulator);

	bool handled_probe_message = false;

	if (!simulator->IsEnabled)
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

	return handled_probe_message;
}
