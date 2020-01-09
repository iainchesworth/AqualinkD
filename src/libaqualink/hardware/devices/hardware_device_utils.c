#include "hardware_device_utils.h"
#include "hardware_device.h"

#include "logging/logging.h"

DeviceId extract_device_id_from_device_structure(HardwareDevice* device)
{
	DeviceId device_id = 0xFFFF;

	switch (device->Type)
	{
	case Generic:
		device_id = device->Info.gdi.Id;
		break;

	default:
		WARN("Invalid/unknown type of hardware device...cannot extract device id for hardware.");
		break;
	}

	return device_id;
}
