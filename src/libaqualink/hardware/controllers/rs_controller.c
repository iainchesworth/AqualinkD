#include "rs_controller.h"

#include <assert.h>

#include "config/config_helpers.h"
#include "hardware/buttons/rs_buttons.h"

static void configure_aqualinkrs_controller_buttons(AqualinkRS* controller)
{
	assert(0 != controller);

	int button_index;

	for (button_index = FilterPump; button_index < ButtonTypeCount; ++button_index)
	{
		controller->Buttons[button_index].State = NotPresent;
	}

	//
	// NOTE NOTE NOTE:
	//
	//    The below works by allowing each variant to configure the buttons that
	//    are unique to *that* controller i.e. we drop through cases until all
	//    buttons are configured (but only start where required).
	//

	switch (controller->Variant)
	{
	case RS32:
		// NO BREAK HERE

	case RS24:
		// NO BREAK HERE

	case RS12:
		// NO BREAK HERE

	case RS8:
		controller->Buttons[Aux_7].State = ButtonIsOff;
		controller->Buttons[Aux_7].Label = CFG_ButtonAux7Label();
		controller->Buttons[Aux_6].State = ButtonIsOff;
		controller->Buttons[Aux_6].Label = CFG_ButtonAux6Label();
		// NO BREAK HERE

	case RS6:
		controller->Buttons[Aux_5].State = ButtonIsOff;
		controller->Buttons[Aux_5].Label = CFG_ButtonAux5Label();
		controller->Buttons[Aux_4].State = ButtonIsOff;
		controller->Buttons[Aux_4].Label = CFG_ButtonAux4Label();
		// NO BREAK HERE

	case RS4:
		controller->Buttons[SolarHeater].State = ButtonIsOff;
		controller->Buttons[SolarHeater].Label = CFG_ButtonSolarHeaterLabel();
		controller->Buttons[SpaHeater].State = ButtonIsOff;
		controller->Buttons[SpaHeater].Label = CFG_ButtonSpaHeaterLabel();
		controller->Buttons[PoolHeater].State = ButtonIsOff;
		controller->Buttons[PoolHeater].Label = CFG_ButtonPoolHeaterLabel();
		controller->Buttons[Aux_3].State = ButtonIsOff;
		controller->Buttons[Aux_3].Label = CFG_ButtonAux3Label();
		controller->Buttons[Aux_2].State = ButtonIsOff;
		controller->Buttons[Aux_2].Label = CFG_ButtonAux2Label();
		controller->Buttons[Aux_1].State = ButtonIsOff;
		controller->Buttons[Aux_1].Label = CFG_ButtonAux1Label();
		controller->Buttons[FilterPump].State = ButtonIsOff;
		controller->Buttons[FilterPump].Label = CFG_ButtonFilterPumpLabel();
		break;

	default:
		break;
	}
}

void initialise_aqualinkrs_controller(AqualinkRS* controller, AqualinkRS_Variants variant)
{
	assert(0 != controller);

	controller->Variant = variant;
	controller->State = Auto;
	controller->Mode = Pool;

	controller->Simulator.Id = CFG_DeviceId();

	configure_aqualinkrs_controller_buttons(controller);
}

void record_probe_event(AqualinkRS* controller, unsigned int destination)
{
	assert(0 != controller);

	controller->ActiveProbe.Destination = destination;
	controller->ActiveProbe.Timestamp = time(0);
}
