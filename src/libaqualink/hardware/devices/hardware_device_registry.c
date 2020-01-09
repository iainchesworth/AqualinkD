#include "hardware_device_registry.h"
#include "hardware_device_utils.h"

#include <assert.h>
#include <stdbool.h>

#include "logging/logging.h"

bool does_device_exist_in_hardware_registry(DevicesRegistry* registry, HardwareDevice* device)
{
	assert(0 != registry);
	assert(0 != device);

	const DeviceId desired_device_id = extract_device_id_from_device_structure(device);

	bool device_exists = false;

	if (0 == registry->device_count)
	{
		TRACE("Desired device (%02x) was not found in device registry as there are no devices in the registry", desired_device_id);
	}
	else
	{
		DevicesRegistry_ListNode* device_node = registry->head;
		int device_index;

		for (device_index = 0; device_index < registry->device_count; ++device_index)
		{
			const DeviceId current_device_id = extract_device_id_from_device_structure(device_node->device);
			if (current_device_id == desired_device_id)
			{
				TRACE("Desired device (%02x) already exists in device registry", desired_device_id);
				device_exists = true;

				// Terminate the search loop.
				break;
			}
			else
			{
				// Still not yet found so increment to the next node
				device_node = device_node->next;
			}
		}

		TRACE("Desired device (%02x) was not found in device registry", desired_device_id);
	}

	return device_exists;
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
