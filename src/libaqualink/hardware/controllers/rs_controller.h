#ifndef AQ_RS_CONTROLLER_H_
#define AQ_RS_CONTROLLER_H_

#include "cross-platform/time.h"
#include "hardware/buttons/rs_buttons.h"
#include "hardware/devices/hardware_device_registry.h"
#include "hardware/heaters/heaters.h"
#include "hardware/simulators/rs_keypadsimulator.h"

typedef enum tagAqualinkRS_Variants
{
	RS4, RS6, RS8, RS12, RS16, RS24, RS32
}
AqualinkRS_Variants;

/* 
	PDA4, PDA6, PDA8,
	ZQ4,
	UnknownControllerVariant
*/

typedef enum tagAqualinkRS_States
{
	Auto,
	Service,
	TimeOut,
}
AqualinkRS_States;

typedef enum tagAqualinkRS_Modes
{
	Pool,
	Spa,
	SpaFill,
	SpaDrain
}
AqualinkRS_Modes;

typedef struct tagAqualinkRS
{
	AqualinkRS_Variants Variant;
	AqualinkRS_States State;
	AqualinkRS_Modes Mode;

	//==============================================
	//
	// Common to all Aqualink RS controllers
	//
	//==============================================
	
	// Buttons
	AqualinkRS_Button Buttons[ButtonTypeCount];

	// Heaters
	Heater PoolHeater;
	Heater SpaHeater;
	Heater SolarHeater;

	// Keypad Simulator
	AqualinkRS_KeypadSimulator Simulator;

	//==============================================
	//
	// Custom on a per-installation basis
	//
	//==============================================

	DevicesRegistry Devices;

	//==============================================
	//
	// Extra stuff 'n' things necessary to make the 
	// controller function well.
	//
	//==============================================

	struct 
	{
		unsigned int Destination;
		time_t Timestamp;
	}
	ActiveProbe;

}
AqualinkRS;

// Aqualink RS controller functions.

void initialise_aqualinkrs_controller(AqualinkRS* controller, AqualinkRS_Variants variant);

// Messaging functions

void record_probe_event(AqualinkRS* controller, unsigned int destination);

#endif // AQ_RS_CONTROLLER_H_
