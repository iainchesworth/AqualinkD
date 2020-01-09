#include "aqualink_master_controller.h"

#include "hardware/simulators/rs_keypadsimulator.h"

AqualinkRS aqualink_master_controller =
{
	.Variant = RS8,
	.State = Auto,
	.Mode = Pool,

	.Buttons =
	{
		{ NotPresent, 0, 0, 0, 0, 0, 0 },
		{ NotPresent, 0, 0, 0, 0, 0, 0 },
		{ NotPresent, 0, 0, 0, 0, 0, 0 },
		{ NotPresent, 0, 0, 0, 0, 0, 0 },
		{ NotPresent, 0, 0, 0, 0, 0, 0 },
		{ NotPresent, 0, 0, 0, 0, 0, 0 },
		{ NotPresent, 0, 0, 0, 0, 0, 0 },
		{ NotPresent, 0, 0, 0, 0, 0, 0 },
		{ NotPresent, 0, 0, 0, 0, 0, 0 },
		{ NotPresent, 0, 0, 0, 0, 0, 0 },
		{ NotPresent, 0, 0, 0, 0, 0, 0 },
		{ NotPresent, 0, 0, 0, 0, 0, 0 }
	},

	.PoolHeater = {.Mode = HeaterIsOff },
	.SpaHeater = {.Mode = HeaterIsOff },
	.SolarHeater = {.Mode = HeaterIsOff },

	.Simulator =
	{
		.IsEnabled = false,
		.Id = 0xFF,

		.FunctionKeypad = { FilterPump },
		.MenuKeypad = { Back },

		.ProbeMessageHandler = &aqualinkrs_keypadsimulator_probemessagehandler
	},

	.Devices = { .head = 0, .tail = 0, .device_count = 0 },

	.ActiveProbe =
	{
		.Destination = 0xFF,
		.Timestamp = 0
	}
};
