#include "hardware_device_registry.h"

#include <assert.h>
#include <stdbool.h>

bool does_device_exist_in_hardware_registry(DevicesRegistry* registry, HardwareDevice* device)
{
	assert(0 != registry);
	assert(0 != device);

	return false;
}

bool add_device_to_hardware_registry(DevicesRegistry* registry, HardwareDevice* device)
{
	assert(0 != registry);
	assert(0 != device);

	return false;
}

unsigned int count_of_devices_in_hardware_registry(DevicesRegistry* registry)
{
	assert(0 != registry);

	return registry->device_count;
}
