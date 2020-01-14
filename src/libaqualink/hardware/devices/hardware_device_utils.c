#include "hardware_device_utils.h"
#include "hardware_device.h"

#include "logging/logging.h"

HardwareDeviceId extract_device_id_from_device_structure(HardwareDevice* device)
{
	HardwareDeviceId device_id = INVALID_DEVICE_ID;

	switch (device->Type)
	{
	case Generic:
		device_id = device->Info.gdi.Id;
		break;

	case Pda_Remote:
		device_id = device->Info.prdi.Id;
		break;

	default:
		WARN("Invalid/unknown type of hardware device...cannot extract device id for hardware.");
		break;
	}

	return device_id;
}
