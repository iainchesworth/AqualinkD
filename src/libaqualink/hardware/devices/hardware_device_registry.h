#ifndef AQ_HARDWARE_DEVICE_REGISTRY_H_
#define AQ_HARDWARE_DEVICE_REGISTRY_H_

#include <stdbool.h>

#include "hardware/devices/hardware_device.h"

struct DevicesRegistry_ListNode
{
	HardwareDevice* device;
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

bool hardware_registry_does_device_exist(HardwareDevice* device);
bool hardware_registry_add_device(HardwareDevice* device);
unsigned int hardware_registry_get_device_count();
void hardware_registry_destroy();

#endif // AQ_HARDWARE_DEVICE_REGISTRY_H_
