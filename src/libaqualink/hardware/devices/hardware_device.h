#ifndef AQ_HARDWARE_DEVICE_H_
#define AQ_HARDWARE_DEVICE_H_

#include "hardware/devices/generic/generic_device.h"
#include "hardware/devices/pda-remote/pda_remote.h"

enum DeviceTypes
{
	Generic,
	Pda_Remote
};

union DeviceInfoTypes
{
	GenericDeviceInfo gdi;
	PdaRemote_DeviceInfo prdi;
};

typedef struct tagHardwareDevice
{
	enum DeviceTypes Type;
	union DeviceInfoTypes Info;
}
HardwareDevice;

#endif // AQ_HARDWARE_DEVICE_H_
