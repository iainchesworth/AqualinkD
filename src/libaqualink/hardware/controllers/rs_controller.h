#ifndef AQ_RS_CONTROLLER_H_
#define AQ_RS_CONTROLLER_H_

#include "cross-platform/time.h"
#include "hardware/buttons/rs_buttons.h"
#include "hardware/devices/hardware_device_registry.h"
#include "hardware/devices/hardware_device_types.h"
#include "hardware/heaters/heaters.h"
#include "messages/message-bus/aq_serial_message_bus.h"
#include "messages/message-serializers/aq_serial_message_ack_serializer.h"
#include "messages/message-serializers/aq_serial_message_msg_long_serializer.h"
#include "messages/message-serializers/aq_serial_message_probe_serializer.h"
#include "messages/message-serializers/aq_serial_message_status_serializer.h"
#include "messages/message-serializers/aq_serial_message_unknown_serializer.h"
#include "serial/aq_serial_types.h"
#include "simulators/onetouch/onetouch_simulator_types.h"
#include "simulators/onetouch_simulator.h"
#include "simulators/pda/pda_simulator_types.h"
#include "simulators/pda_simulator.h"
#include "simulators/rs_keypad/rs_keypad_simulator_types.h"
#include "simulators/rs_keypad_simulator.h"

typedef enum tagAqualinkRS_Variants
{
	RS4, RS6, RS8, RS12, RS16, RS24, RS32
}
AqualinkRS_Variants;

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

	// Simulators
	Aqualink_RS_Keypad_Simulator* RS_Keypad_Simulator;
	Aqualink_PDASimulator* PDA_Simulator;
	Aqualink_OneTouchSimulator* OneTouch_Simulator;

	// Simulator Message Bus.
	MessageBus Simulator_MessageBus;

	//==============================================
	//
	// Custom on a per-installation basis
	//
	//==============================================

	DevicesRegistry* Devices;

	//==============================================
	//
	// Extra stuff 'n' things necessary to make the 
	// controller function well.
	//
	//==============================================

	struct
	{
		SerialData_Commands Type;
		HardwareDeviceId Destination;
		time_t Timestamp;
	}
	LastMessage;

}
AqualinkRS;

void rs_controller_initialise(AqualinkRS_Variants variant);
void rs_controller_destroy(void);

void rs_controller_record_message_event(SerialData_Commands command, HardwareDeviceId destination);
SerialData_Commands rs_controller_get_last_message_type(void);
HardwareDeviceId rs_controller_get_last_message_destination(void);
MessageBus* rs_controller_get_simulator_message_bus(void);

void rs_controller_print_detected_devices(void);

// RS6 Keypad Simulator
bool rs_controller_enable_rs_keypad_simulator(void);
bool rs_controller_disable_rs_keypad_simulator(void);

// PDA Simulator
bool rs_controller_enable_pda_simulator(void); 
bool rs_controller_disable_pda_simulator(void);

// OneTouch Simulator
bool rs_controller_enable_onetouch_simulator(void);
bool rs_controller_disable_onetouch_simulator(void);

#endif // AQ_RS_CONTROLLER_H_
