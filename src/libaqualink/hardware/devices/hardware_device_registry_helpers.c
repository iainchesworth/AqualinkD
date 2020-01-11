#include "hardware_device_registry_helpers.h"

#include <stdlib.h>

#include "logging/logging.h"
#include "hardware/devices/generic/generic_device.h"
#include "hardware/devices/pda-remote/pda_remote.h"
#include "hardware/devices/hardware_device.h"
#include "hardware/devices/hardware_device_registry.h"
#include "hardware/devices/hardware_device_registry_private.h"
#include "hardware/devices/hardware_device_types.h"

void device_registry_add_generic_device(const DeviceId device_id)
{
	HardwareDevice* this_device = (HardwareDevice*)malloc(sizeof(HardwareDevice));

	this_device->Type = Generic;
	this_device->Info.gdi.Id = device_id;

	if (does_device_exist_in_hardware_registry(&aqualink_master_controller_device_registry, this_device))
	{
		TRACE("Device 0x%02x is already present in the registry", this_device->Info.gdi.Id);
	}
	else if (!add_device_to_hardware_registry(&aqualink_master_controller_device_registry, this_device))
	{
		WARN("Discovered device 0x%02x but could not register it in the local registry", this_device->Info.gdi.Id);
	}
	else
	{
		DEBUG("Device 0x%02x has been added to the registry", this_device->Info.gdi.Id);
	}
}

static const char PDA_REMOTE_DEVICE_NAME[] = "PDA Remote";

void device_registry_add_pda_remote(const DeviceId device_id)
{
	HardwareDevice* this_device = (HardwareDevice*)malloc(sizeof(HardwareDevice));

	this_device->Type = Generic;

	this_device->Info.prdi.Id = device_id;
	this_device->Info.prdi.Name = PDA_REMOTE_DEVICE_NAME;

	if (does_device_exist_in_hardware_registry(&aqualink_master_controller_device_registry, this_device))
	{
		TRACE("Device 0x%02x is already present in the registry", this_device->Info.gdi.Id);
	}
	else if (!add_device_to_hardware_registry(&aqualink_master_controller_device_registry, this_device))
	{
		WARN("Discovered device 0x%02x but could not register it in the local registry", this_device->Info.gdi.Id);
	}
	else
	{
		DEBUG("Device 0x%02x has been added to the registry", this_device->Info.gdi.Id);
	}
}
