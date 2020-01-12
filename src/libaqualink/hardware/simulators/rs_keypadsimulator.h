#ifndef AQ_RS_KEYPADSIMULATOR_H_
#define AQ_RS_KEYPADSIMULATOR_H_

#include <stdbool.h>

#include "cross-platform/threads.h"
#include "hardware/buttons/rs_buttons.h"
#include "hardware/devices/hardware_device_types.h"
#include "serial/serializers/aq_serial_message_probe_serializer.h"

#include "simulator_types.h"

typedef struct tagAqualinkRS_KeypadSimulator
{
	struct
	{
		bool IsInitialised;
		mtx_t SimulatorAccessMutex;
	}
	Config;

	bool IsEnabled;

	DeviceId Id;

	AqualinkRS_ButtonTypes FunctionKeypad[ButtonTypeCount];					///FIXME - this is NOT a structure suitable for the button functions.
	AqualinkRS_KeypadMenuButtonTypes MenuKeypad[KeypadMenuButtonTypeCount];	///FIXME - this is NOT a structure suitable for the button functions.

	Simulator_Initialise Initialise;
	Simulator_AckMessageHandler AckMessageHandler;
	Simulator_MsgLongMessageHandler MsgLongMessageHandler;
	Simulator_ProbeMessageHandler ProbeMessageHandler;
	Simulator_StatusMessageHandler StatusMessageHandler;
	Simulator_UnknownMessageHandler UnknownMessageHandler;
}
AqualinkRS_KeypadSimulator;

// Simulator functions

bool rs_keypadsimulator_enable();
bool rs_keypadsimulator_disable();

// Simulator message handlers

bool rs_keypadsimulator_initialise();
bool rs_keypadsimulator_probemessagehandler(AQ_Probe_Packet* packet);

// Simulator message builders

///FIXME - add message builders.

#endif // AQ_RS_KEYPADSIMULATOR_H_
