#include "hardware_device_registry_helpers.h"

#include <stdlib.h>

#include "logging/logging.h"
#include "hardware/devices/generic/generic_device.h"
#include "hardware/devices/pda-remote/pda_remote.h"
#include "hardware/devices/hardware_device.h"
#include "hardware/devices/hardware_device_registry.h"
#include "hardware/devices/hardware_device_registry_private.h"
#include "hardware/devices/hardware_device_types.h"

void device_registry_add_generic_device(const HardwareDeviceId device_id)
{
	HardwareDevice* this_device = (HardwareDevice*)malloc(sizeof(HardwareDevice));

	this_device->Type = Generic;
	this_device->Info.gdi.Id = device_id;

	if (hardware_registry_does_device_exist(this_device))
	{
		TRACE("Device 0x%02x is already present in the registry", this_device->Info.gdi.Id);
	}
	else if (!hardware_registry_add_device(this_device))
	{
		WARN("Discovered device 0x%02x but could not register it in the local registry", this_device->Info.gdi.Id);
	}
	else
	{
		DEBUG("Device 0x%02x has been added to the registry", this_device->Info.gdi.Id);
	}
}

static const char PDA_REMOTE_DEVICE_NAME[] = "PDA Remote";

void device_registry_add_pda_remote(const HardwareDeviceId device_id)
{
	HardwareDevice* this_device = (HardwareDevice*)malloc(sizeof(HardwareDevice));

	this_device->Type = Pda_Remote;

	this_device->Info.prdi.Id = device_id;
	this_device->Info.prdi.Name = PDA_REMOTE_DEVICE_NAME;

	if (hardware_registry_does_device_exist(this_device))
	{
		TRACE("Device 0x%02x is already present in the registry", this_device->Info.gdi.Id);
	}
	else if (!hardware_registry_add_device(this_device))
	{
		WARN("Discovered device 0x%02x but could not register it in the local registry", this_device->Info.gdi.Id);
	}
	else
	{
		DEBUG("Device 0x%02x has been added to the registry", this_device->Info.gdi.Id);
	}
}

void device_registry_destroy_entry(HardwareDevice* device)
{
	if (0 == device)
	{
		ERROR("Cannot destroy device entry in registry...already destroyed (or doesn't exist)");
	}
	else if (Generic == device->Type)
	{
		TRACE("De-allocating memory assigned to a generic hardware device entry");

		free(device);
		device = 0;
	}
	else if (Pda_Remote == device->Type)
	{
		TRACE("De-allocating memory assigned to a PDA Remote hardware device entry");

		free(device);
		device = 0;
	}
	else
	{
		// Don't know what type of entry this is...don't free it as we don't know
		// whether it was malloc'd to start with.
	}
}