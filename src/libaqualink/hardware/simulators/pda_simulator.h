#ifndef AQ_PDA_SIMULATOR_H_
#define AQ_PDA_SIMULATOR_H_

#include <stdbool.h>

#include "cross-platform/threads.h"
#include "hardware/devices/hardware_device_types.h"
#include "serial/serializers/aq_serial_message_msg_long_serializer.h"

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
	Simulator_AckMessageHandler AckMessageHandler;
	Simulator_MsgLongMessageHandler MsgLongMessageHandler;
	Simulator_ProbeMessageHandler ProbeMessageHandler;
	Simulator_StatusMessageHandler StatusMessageHandler;
	Simulator_UnknownMessageHandler UnknownMessageHandler;
}
Aqualink_PDASimulator;

// Simulator functions

bool pda_simulator_enable();
bool pda_simulator_disable();

// Simulator message handlers

bool pda_simulator_initialise();
bool pda_simulator_probemessagehandler(AQ_Probe_Packet* packet);
bool pda_simulator_msglongmessagehandler(AQ_Msg_Long_Packet* packet);

// Simulator message builders

#endif // AQ_PDA_SIMULATOR_H_
