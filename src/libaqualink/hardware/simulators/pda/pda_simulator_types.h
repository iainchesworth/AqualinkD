#ifndef PDA_SIMULATOR_H_
#define PDA_SIMULATOR_H_

#include <stdbool.h>

#include "cross-platform/threads.h"
#include "hardware/devices/hardware_device_types.h"
#include "hardware/simulators/simulator_types.h"

typedef struct tagAqualink_PDASimulator
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
Aqualink_PDASimulator;

#endif // PDA_SIMULATOR_H_
