#ifndef AQ_PDA_REMOTE_H_
#define AQ_PDA_REMOTE_H_

#include "hardware/devices/hardware_device_types.h"

typedef struct tagPdaRemote_DeviceInfo
{
	HardwareDeviceId Id;
	const char* Name;
}
PdaRemote_DeviceInfo;

#endif // AQ_PDA_REMOTE_H_
