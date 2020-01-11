#ifndef AQ_PDA_SIMULATOR_H_
#define AQ_PDA_SIMULATOR_H_

#include <stdbool.h>

#include "cross-platform/threads.h"
#include "hardware/devices/hardware_device_types.h"

#include "simulator_types.h"

typedef struct tagAqualink_PDASimulator
{
	struct
	{
		bool IsInitialised;
		mtx_t SimulatorAccessMutex;
	}
	Config;

	bool IsEnabled;

	DeviceId Id;
	
	Simulator_Initialise Initialise;
	Simulator_ProbeMessageHandler ProbeMessageHandler;
	Simulator_MsgLongMessageHandler MsgLongMessageHandler;
}
Aqualink_PDASimulator;

// Simulator functions

void pda_simulator_enable();
void pda_simulator_disable();

// Simulator message handlers

bool pda_simulator_initialise();
bool pda_simulator_msglongmessagehandler();

// Simulator message builders

#endif // AQ_PDA_SIMULATOR_H_
