#ifndef AQ_RS_KEYPAD_SIMULATOR_TYPES_H_
#define AQ_RS_KEYPAD_SIMULATOR_TYPES_H_

#include <stdbool.h>

#include "cross-platform/threads.h"
#include "hardware/buttons/rs_buttons.h"
#include "hardware/devices/hardware_device_types.h"
#include "hardware/simulators/simulator_types.h"

typedef struct tagAqualink_RS_Keypad_Simulator
{
	struct
	{
		bool IsInitialised;
		mtx_t SimulatorAccessMutex;
	}
	Config;

	bool IsEnabled;

	HardwareDeviceId Id;

	AqualinkRS_ButtonTypes FunctionKeypad[ButtonTypeCount];					///FIXME - this is NOT a structure suitable for the button functions.
	AqualinkRS_KeypadMenuButtonTypes MenuKeypad[KeypadMenuButtonTypeCount];	///FIXME - this is NOT a structure suitable for the button functions.

	Simulator_Initialise Initialise;
	Simulator_AckMessageHandler AckMessageHandler;
	Simulator_MsgLongMessageHandler MsgLongMessageHandler;
	Simulator_ProbeMessageHandler ProbeMessageHandler;
	Simulator_StatusMessageHandler StatusMessageHandler;
	Simulator_UnknownMessageHandler UnknownMessageHandler;
}
Aqualink_RS_Keypad_Simulator;

#endif // AQ_RS6_KEYPAD_SIMULATOR_TYPES_H_
