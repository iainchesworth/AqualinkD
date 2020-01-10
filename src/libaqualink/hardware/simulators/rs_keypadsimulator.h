#ifndef AQ_RS_KEYPADSIMULATOR_H_
#define AQ_RS_KEYPADSIMULATOR_H_

#include <stdbool.h>

#include "hardware/buttons/rs_buttons.h"
#include "hardware/devices/hardware_device_types.h"

typedef bool (*KeypadSimulator_ProbeMessageHandler)();

typedef struct tagAqualinkRS_KeypadSimulator
{
	volatile bool IsEnabled;

	DeviceId Id;

	AqualinkRS_ButtonTypes FunctionKeypad[ButtonTypeCount];					///FIXME - this is NOT a structure suitable for the button functions.
	AqualinkRS_KeypadMenuButtonTypes MenuKeypad[KeypadMenuButtonTypeCount];	///FIXME - this is NOT a structure suitable for the button functions.

	KeypadSimulator_ProbeMessageHandler ProbeMessageHandler;
}
AqualinkRS_KeypadSimulator;

// Simulator functions

void enable_aqualinkrs_keypadsimulator(AqualinkRS_KeypadSimulator* simulator);
void disable_aqualinkrs_keypadsimulator(AqualinkRS_KeypadSimulator* simulator);

// Simulator message handlers

bool aqualinkrs_keypadsimulator_probemessagehandler(AqualinkRS_KeypadSimulator* simulator);

// Simulator message builers

///FIXME - add message builders.

#endif // AQ_RS_KEYPADSIMULATOR_H_
