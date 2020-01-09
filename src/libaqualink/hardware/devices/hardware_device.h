#ifndef AQ_HARDWARE_DEVICE_H_
#define AQ_HARDWARE_DEVICE_H_

#include "hardware/devices/generic/generic_device.h"

enum DeviceTypes
{
	Generic
};

union DeviceInfoTypes
{
	GenericDeviceInfo gdi;
};

typedef struct tagHardwareDevice
{
	enum DeviceTypes Type;
	union DeviceInfoTypes Info;
}
HardwareDevice;

#endif // AQ_HARDWARE_DEVICE_H_