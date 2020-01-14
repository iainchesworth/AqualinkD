#include "hardware_device_registry.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

#include "hardware/devices/hardware_device_registry_helpers.h"
#include "hardware/devices/hardware_device_registry_private.h"
#include "hardware/devices/hardware_device_utils.h"
#include "logging/logging.h"

bool hardware_registry_does_device_exist(HardwareDevice* device)
{
	assert(0 != device);

	const HardwareDeviceId desired_device_id = extract_device_id_from_device_structure(device);

	bool device_exists = false;

	if (0 == aqualink_master_controller_device_registry.device_count)
	{
		TRACE("Desired device (0x%02x) was not found in device registry as there are no devices in the registry", desired_device_id);
	}
	else
	{
		DevicesRegistry_ListNode* device_node = aqualink_master_controller_device_registry.head;
		int device_index;

		for (device_index = 0; device_index < aqualink_master_controller_device_registry.device_count; ++device_index)
		{
			const HardwareDeviceId current_device_id = extract_device_id_from_device_structure(device_node->device);
			if ((current_device_id.Type == desired_device_id.Type) && ((current_device_id.Instance == desired_device_id.Instance)))
			{
				TRACE("Desired device (0x%02x) already exists in device registry", desired_device_id);
				device_exists = true;

				// Terminate the search loop.
				break;
			}
			
			// Still not yet found so increment to the next node
			device_node = device_node->next;
		}

		TRACE("Desired device (0x%02x) was %s in the device registry", desired_device_id, (device_exists) ? "found" : "NOT found");
	}

	return device_exists;
}

static bool add_device_to_tail_of_list(HardwareDevice* device)
{
	assert(0 == aqualink_master_controller_device_registry.tail->next);
	assert(0 != device);

	if (0 == (aqualink_master_controller_device_registry.tail->next = (DevicesRegistry_ListNode*)malloc(sizeof(DevicesRegistry_ListNode))))
	{
		// Failed to allocate a new node.
		return false;
	}

	// Add the device to the tail (by adding a new node).
	aqualink_master_controller_device_registry.tail->next->device = device;
	aqualink_master_controller_device_registry.tail->next->next = 0;
	aqualink_master_controller_device_registry.device_count++;

	// Move the tail to point at the last node.
	aqualink_master_controller_device_registry.tail = aqualink_master_controller_device_registry.tail->next;

	return true;
}

bool hardware_registry_add_device(HardwareDevice* device)
{
	assert(0 != device);
	
	bool retSuccess;

	if (0 != aqualink_master_controller_device_registry.head)
	{
		// The list already exists, add the device to the end of the list
		retSuccess = add_device_to_tail_of_list(device);
	}
	else if (0 == (aqualink_master_controller_device_registry.head = (DevicesRegistry_ListNode*)malloc(sizeof(DevicesRegistry_ListNode))))
	{
		// Failed to allocate a new node to create a new list.
		retSuccess = false;
	}
	else
	{
		// Add the device as the first node of the list.
		aqualink_master_controller_device_registry.head->device = device;
		aqualink_master_controller_device_registry.head->next = 0;
		aqualink_master_controller_device_registry.device_count++;

		// Make the tail point to the "last" node of the list.
		aqualink_master_controller_device_registry.tail = aqualink_master_controller_device_registry.head;

		retSuccess = true;
	}

	return retSuccess;
}

unsigned int hardware_registry_get_device_count()
{
	return aqualink_master_controller_device_registry.device_count;
}

void hardware_registry_destroy()
{
	if (0 == aqualink_master_controller_device_registry.device_count)
	{
		TRACE("Not iterating through device registry to destroy it as there are no devices registered");
	}
	else
	{
		DevicesRegistry_ListNode* device_node = aqualink_master_controller_device_registry.head, * prev_device_node;
		int device_index;

		for (device_index = 0; device_index < aqualink_master_controller_device_registry.device_count; ++device_index)
		{
			if (0 == device_node)
			{
				// An invalid node means something went wrong...there's nothing more we can do here as there's no next pointer.
				DEBUG("Came across an invalid device at index %d while attempting to destroy the registry", device_index);
				break;
			}
			
			if (0 == device_node->device)
			{
				DEBUG("Device information structure for device index %d was invalid; will ignore it", device_index);
			}
			else
			{
				TRACE("Destroying device information structure for device index %d", device_index);
				device_registry_destroy_entry(device_node->device);
			}

			// Now de-allocate the structural node component but remember
			// to store the pointer to the next node before de-allocating 
			// the node otherwise things will go wrong.

			prev_device_node = device_node;
			device_node = device_node->next;

			free(prev_device_node);
			prev_device_node = 0;
		}

		TRACE("Device registry has been destroyed; %d devices were deallocated", device_index);
	}
}