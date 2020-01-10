#ifndef AQ_PDA_REMOTE_H_
#define AQ_PDA_REMOTE_H_

#include "hardware/devices/hardware_device_types.h"

typedef struct tagPdaRemote_DeviceInfo
{
	DeviceId Id;
	const char* Name;
}
PdaRemote_DeviceInfo;

void add_pda_remote_to_hardware_registry(const DeviceId device_id);

#endif // AQ_PDA_REMOTE_H_
