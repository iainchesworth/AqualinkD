#include "aqualink_master_controller.h"

#include "hardware/simulators/rs_keypadsimulator.h"

AqualinkRS_KeypadSimulator aqualink_keypad_simulator =
{
	.IsEnabled = false,
	.Id = 0xFF,

	.FunctionKeypad = { FilterPump, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 },
	.MenuKeypad = { Back, 1, 2, 3, 4, 5, 6 },

	.ProbeMessageHandler = &aqualinkrs_keypadsimulator_probemessagehandler
};

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

	.Simulator = &aqualink_keypad_simulator,	

	.Devices = { .head = 0, .tail = 0, .device_count = 0 },

	.ActiveProbe =
	{
		.Destination = 0xFF,
		.Timestamp = 0
	}
};
