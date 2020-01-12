#ifndef AQ_ONETOUCH_SIMULATOR_H_
#define AQ_ONETOUCH_SIMULATOR_H_

#include <stdbool.h>

#include "cross-platform/threads.h"
#include "hardware/devices/hardware_device_types.h"
#include "serial/serializers/aq_serial_message_probe_serializer.h"
#include "serial/serializers/aq_serial_message_msg_long_serializer.h"

#include "simulator_types.h"

typedef struct tagAqualink_OneTouchSimulator
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
Aqualink_OneTouchSimulator;

// Simulator functions

bool onetouch_simulator_enable();
bool onetouch_simulator_disable();

// Simulator message handlers

bool onetouch_simulator_initialise();
bool onetouch_simulator_probemessagehandler(AQ_Probe_Packet* packet);
bool onetouch_simulator_msglongmessagehandler(AQ_Msg_Long_Packet* packet);

// Simulator message builders


#endif // AQ_ONETOUCH_SIMULATOR_H_
