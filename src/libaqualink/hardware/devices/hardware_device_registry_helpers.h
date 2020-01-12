#ifndef AQ_HARDWARE_DEVICE_REGISTRY_HELPERS_H_
#define AQ_HARDWARE_DEVICE_REGISTRY_HELPERS_H_

#include "hardware/devices/hardware_device.h"
#include "hardware/devices/hardware_device_types.h"

void device_registry_add_generic_device(const DeviceId device_id);
void device_registry_add_pda_remote(const DeviceId device_id);
void device_registry_destroy_entry(HardwareDevice* device);

#endif // AQ_HARDWARE_DEVICE_REGISTRY_HELPERS_H_
