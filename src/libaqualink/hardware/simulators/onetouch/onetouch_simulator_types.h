#ifndef AQ_ONETOUCH_SIMULATOR_TYPES_H_
#define AQ_ONETOUCH_SIMULATOR_TYPES_H_

#include <stdbool.h>

#include "cross-platform/threads.h"
#include "hardware/devices/hardware_device_types.h"
#include "hardware/simulators/simulator_types.h"
#include "messages/message-bus/aq_serial_message_bus.h"

typedef struct tagAqualink_OneTouchSimulator
{
	struct
	{
		bool IsInitialised;
		mtx_t SimulatorAccessMutex;
	}
	Config;

	bool IsEnabled;

	HardwareDeviceId Id;

	Simulator_Initialise Initialise;
	Simulator_AckMessageHandler AckMessageHandler;
	Simulator_MsgLongMessageHandler MsgLongMessageHandler;
	Simulator_ProbeMessageHandler ProbeMessageHandler;
	Simulator_StatusMessageHandler StatusMessageHandler;
	Simulator_UnknownMessageHandler UnknownMessageHandler;
}
Aqualink_OneTouchSimulator;

#endif // AQ_ONETOUCH_SIMULATOR_TYPES_H_
