#include "rs_controller.h"

#include <assert.h>

#include "config/config_helpers.h"
#include "hardware/buttons/rs_buttons.h"
#include "hardware/devices/hardware_device_registry.h"
#include "hardware/devices/hardware_device_registry_private.h"
#include "logging/logging.h"
#include "messages/message-serializers/aq_serial_message_probe_serializer.h"
#include "monitor/monitor.h"
#include "serial/aq_serial_types.h"
#include "simulators/onetouch/onetouch_simulator_private.h"
#include "simulators/onetouch_simulator.h"
#include "simulators/pda/pda_simulator_private.h"
#include "simulators/pda_simulator.h"
#include "simulators/rs_keypad/rs_keypad_simulator_private.h"
#include "simulators/rs_keypad_simulator.h"

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

	.RS_Keypad_Simulator = &aqualink_rs_keypad_simulator,

	.PDA_Simulator = &aqualink_pda_simulator,

	.OneTouch_Simulator = &aqualink_onetouch_simulator,

	// .Simulator_MessageBus

	.Devices = &aqualink_master_controller_device_registry,

	.LastMessage =
	{
		.Type = CMD_UNKNOWN,
		.Destination = { .Type = UnknownDevice, .Instance = Instance_0}, // INVALID_DEVICE_ID
		.Timestamp = 0
	}
};

static void rs_controller_configure_buttons()
{
	int button_index;

	for (button_index = FilterPump; button_index < ButtonTypeCount; ++button_index)
	{
		aqualink_master_controller.Buttons[button_index].State = NotPresent;
	}

	//
	// NOTE NOTE NOTE:
	//
	//    The below works by allowing each variant to configure the buttons that
	//    are unique to *that* controller i.e. we drop through cases until all
	//    buttons are configured (but only start where required).
	//

	switch (aqualink_master_controller.Variant)
	{
	case RS32:
		// NO BREAK HERE

	case RS24:
		// NO BREAK HERE

	case RS12:
		// NO BREAK HERE

	case RS8:
		aqualink_master_controller.Buttons[Aux_7].State = ButtonIsOff;
		aqualink_master_controller.Buttons[Aux_7].Label = CFG_ButtonAux7Label();
		aqualink_master_controller.Buttons[Aux_6].State = ButtonIsOff;
		aqualink_master_controller.Buttons[Aux_6].Label = CFG_ButtonAux6Label();
		// NO BREAK HERE

	case RS6:
		aqualink_master_controller.Buttons[Aux_5].State = ButtonIsOff;
		aqualink_master_controller.Buttons[Aux_5].Label = CFG_ButtonAux5Label();
		aqualink_master_controller.Buttons[Aux_4].State = ButtonIsOff;
		aqualink_master_controller.Buttons[Aux_4].Label = CFG_ButtonAux4Label();
		// NO BREAK HERE

	case RS4:
		aqualink_master_controller.Buttons[SolarHeater].State = ButtonIsOff;
		aqualink_master_controller.Buttons[SolarHeater].Label = CFG_ButtonSolarHeaterLabel();
		aqualink_master_controller.Buttons[SpaHeater].State = ButtonIsOff;
		aqualink_master_controller.Buttons[SpaHeater].Label = CFG_ButtonSpaHeaterLabel();
		aqualink_master_controller.Buttons[PoolHeater].State = ButtonIsOff;
		aqualink_master_controller.Buttons[PoolHeater].Label = CFG_ButtonPoolHeaterLabel();
		aqualink_master_controller.Buttons[Aux_3].State = ButtonIsOff;
		aqualink_master_controller.Buttons[Aux_3].Label = CFG_ButtonAux3Label();
		aqualink_master_controller.Buttons[Aux_2].State = ButtonIsOff;
		aqualink_master_controller.Buttons[Aux_2].Label = CFG_ButtonAux2Label();
		aqualink_master_controller.Buttons[Aux_1].State = ButtonIsOff;
		aqualink_master_controller.Buttons[Aux_1].Label = CFG_ButtonAux1Label();
		aqualink_master_controller.Buttons[FilterPump].State = ButtonIsOff;
		aqualink_master_controller.Buttons[FilterPump].Label = CFG_ButtonFilterPumpLabel();
		break;

	default:
		break;
	}
}

void rs_controller_initialise(AqualinkRS_Variants variant)
{
	assert(0 != aqualink_master_controller.RS_Keypad_Simulator);

	TRACE("Initialising Aqualink RS Controller...");

	monitor_initialise();

	aqualink_master_controller.Variant = variant;
	aqualink_master_controller.State = Auto;
	aqualink_master_controller.Mode = Pool;

	aqualink_master_controller.RS_Keypad_Simulator->Id.Type		= Keypad;		///FIXME
	aqualink_master_controller.RS_Keypad_Simulator->Id.Instance = Instance_0;	///FIXME
	
	aqualink_master_controller.PDA_Simulator->Id.Type			= PDA_Remote;	///FIXME
	aqualink_master_controller.PDA_Simulator->Id.Instance		= Instance_0;	///FIXME

	aqualink_master_controller.OneTouch_Simulator->Id.Type		= OneTouch;		///FIXME
	aqualink_master_controller.OneTouch_Simulator->Id.Instance	= Instance_1;	///FIXME

	messagebus_init(&(aqualink_master_controller.Simulator_MessageBus));

	rs_controller_configure_buttons();

	if (0 != aqualink_master_controller.RS_Keypad_Simulator->Initialise)
	{
		aqualink_master_controller.RS_Keypad_Simulator->Initialise(&(aqualink_master_controller.Simulator_MessageBus));
	}

	if (0 != aqualink_master_controller.PDA_Simulator->Initialise)
	{
		aqualink_master_controller.PDA_Simulator->Initialise(&(aqualink_master_controller.Simulator_MessageBus));
	}

	if (0 != aqualink_master_controller.OneTouch_Simulator->Initialise)
	{
		aqualink_master_controller.OneTouch_Simulator->Initialise(&(aqualink_master_controller.Simulator_MessageBus));
	}
}

void rs_controller_destroy()
{
	TRACE("Destroying Aqualink RS Controller...");

	rs_controller_disable_onetouch_simulator();
	rs_controller_disable_pda_simulator();
	rs_controller_disable_rs_keypad_simulator();

	hardware_registry_destroy();
}

void rs_controller_record_message_event(SerialData_Commands command, HardwareDeviceId destination)
{
	const time_t now = time(0);

	aqualink_master_controller.LastMessage.Type = command;
	aqualink_master_controller.LastMessage.Destination = destination;
	aqualink_master_controller.LastMessage.Timestamp = time(0);

	TRACE("Recording command 0x%02x packet for destination 0x%02x at offset %d", command, destination, now);
}

SerialData_Commands rs_controller_get_last_message_type()
{
	return aqualink_master_controller.LastMessage.Type;
}

HardwareDeviceId rs_controller_get_last_message_destination()
{
	return aqualink_master_controller.LastMessage.Destination;
}

MessageBus* rs_controller_get_simulator_message_bus()
{
	return &(aqualink_master_controller.Simulator_MessageBus);
}

void rs_controller_print_detected_devices()
{
	assert(0 != aqualink_master_controller.Devices);

	DevicesRegistry_ListNode* device_node = aqualink_master_controller.Devices->head;
	unsigned int device_index;

	for (device_index = 0; device_index < aqualink_master_controller.Devices->device_count; ++device_index)
	{
		HardwareDevice* device = device_node->device;

		if (0 == device)
		{
			ERROR("Invalid device pointer while trying to enumerate devices (for printing)");
		}
		else
		{
			switch (device->Type)
			{
			case Generic:
				NOTICE("Generic Device - Id 0x%02x - %s", device->Info.gdi.Id, device_id_to_string(device->Info.gdi.Id));
				break;

			case Pda_Remote:
				NOTICE("PDA Remote     - Id 0x%02x - %s", device->Info.prdi.Id, device_id_to_string(device->Info.prdi.Id));
				break;

			default:
				NOTICE("Unknown Device");
				break;
			}
		}

		device_node = device_node->next;
	}
}

//=============================================================================
//
// RS6 Simulator functions
//
//
//=============================================================================

bool rs_controller_enable_rs_keypad_simulator()
{
	if (0 == aqualink_master_controller.RS_Keypad_Simulator)
	{
		return false;
	}

	return rs_keypad_simulator_enable();
}

bool rs_controller_disable_rs_keypad_simulator()
{
	if (0 == aqualink_master_controller.RS_Keypad_Simulator)
	{
		return false;
	}

	return rs_keypad_simulator_disable();
}

//=============================================================================
//
// PDA Simulator functions
//
//
//=============================================================================

bool rs_controller_enable_pda_simulator()
{
	if (0 == aqualink_master_controller.PDA_Simulator)
	{
		return false;
	}

	return pda_simulator_enable();
}

bool rs_controller_disable_pda_simulator()
{
	if (0 == aqualink_master_controller.PDA_Simulator)
	{
		return false;
	}

	return pda_simulator_disable();
}

//=============================================================================
//
// OneTouch Simulator functions
//
//
//=============================================================================

bool rs_controller_enable_onetouch_simulator()
{
	if (0 == aqualink_master_controller.OneTouch_Simulator)
	{
		return false;
	}

	return onetouch_simulator_enable();
}

bool rs_controller_disable_onetouch_simulator()
{
	if (0 == aqualink_master_controller.OneTouch_Simulator)
	{
		return false;
	}

	return onetouch_simulator_disable();
}

bool rs_controller_was_packet_to_or_from_onetouch_simulator(HardwareDeviceId device_id)
{
	if (0 == aqualink_master_controller.OneTouch_Simulator)
	{
		return false;
	}

	return ((aqualink_master_controller.OneTouch_Simulator->Id.Type == device_id.Type) && (aqualink_master_controller.OneTouch_Simulator->Id.Instance == device_id.Instance));
}
