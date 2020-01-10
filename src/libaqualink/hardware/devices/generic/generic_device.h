#ifndef AQ_GENERIC_DEVICE_H_
#define AQ_GENERIC_DEVICE_H_

#include "hardware/devices/hardware_device_types.h"

typedef struct tagGenericDeviceInfo
{
	DeviceId Id;
}
GenericDeviceInfo;

void add_generic_device_to_hardware_registry(const DeviceId device_id);

#endif // AQ_GENERIC_DEVICE_H_
