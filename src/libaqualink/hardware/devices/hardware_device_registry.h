#ifndef AQ_HARDWARE_DEVICE_REGISTRY_H_
#define AQ_HARDWARE_DEVICE_REGISTRY_H_

#include <stdbool.h>

#include "hardware/devices/hardware_devices.h"

struct DevicesRegistry_ListNode
{
	HardwareDevice device;
	struct DevicesRegistry_ListNode* next;
};
typedef struct DevicesRegistry_ListNode DevicesRegistry_ListNode;

struct DevicesRegistry
{
	DevicesRegistry_ListNode* head;
	DevicesRegistry_ListNode* tail;
	unsigned int device_count;
};
typedef struct DevicesRegistry DevicesRegistry;

bool does_device_exist_in_hardware_registry(DevicesRegistry* registry, HardwareDevice* device);
bool add_device_to_hardware_registry(DevicesRegistry* registry, HardwareDevice* device);
unsigned int count_of_devices_in_hardware_registry(DevicesRegistry* registry);

#endif // AQ_HARDWARE_DEVICE_REGISTRY_H_
