#ifndef AQ_RS_CONTROLLER_H_
#define AQ_RS_CONTROLLER_H_

#include "cross-platform/time.h"
#include "hardware/buttons/rs_buttons.h"
#include "hardware/devices/hardware_device_registry.h"
#include "hardware/devices/hardware_device_types.h"
#include "hardware/heaters/heaters.h"
#include "hardware/simulators/pda_simulator.h"
#include "hardware/simulators/rs_keypadsimulator.h"
#include "serial/serializers/aq_serial_message_ack_serializer.h"
#include "serial/serializers/aq_serial_message_msg_long_serializer.h"
#include "serial/serializers/aq_serial_message_probe_serializer.h"
#include "serial/serializers/aq_serial_message_status_serializer.h"
#include "serial/serializers/aq_serial_message_unknown_serializer.h"
#include "serial/aq_serial_types.h"

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
	AqualinkRS_KeypadSimulator* RS6_KeypadSimulator;
	Aqualink_PDASimulator* PDA_Simulator;

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
		DeviceId Destination;
		time_t Timestamp;
	}
	LastMessage;

}
AqualinkRS;

void rs_controller_initialise(AqualinkRS_Variants variant);

void rs_controller_record_message_event(SerialData_Commands command, DeviceId destination);
SerialData_Commands rs_controller_get_last_message_type();
DeviceId rs_controller_get_last_message_destination();

void rs_controller_print_detected_devices();

// RS6 Keypad Simulator
bool rs_controller_enable_rs6_simulator();
bool rs_controller_was_packet_to_or_from_rs6_simulator(DeviceId device_id);
bool rs_controller_rs6_simulator_handle_ack_packet(AQ_Ack_Packet* probePacketforSimulator);
bool rs_controller_rs6_simulator_handle_msg_long_packet(AQ_Msg_Long_Packet* probePacketforSimulator);
bool rs_controller_rs6_simulator_handle_status_packet(AQ_Status_Packet* probePacketforSimulator);
bool rs_controller_rs6_simulator_handle_probe_packet(AQ_Probe_Packet* probePacketforSimulator);
bool rs_controller_rs6_simulator_handle_unknown_packet(AQ_Unknown_Packet* probePacketforSimulator);

// PDA Simulator
bool rs_controller_enable_pda_simulator(); 
bool rs_controller_was_packet_to_or_from_pda_simulator(DeviceId device_id);
bool rs_controller_pda_simulator_handle_ack_packet(AQ_Ack_Packet* probePacketforSimulator);
bool rs_controller_pda_simulator_handle_msg_long_packet(AQ_Msg_Long_Packet* probePacketforSimulator);
bool rs_controller_pda_simulator_handle_status_packet(AQ_Status_Packet* probePacketforSimulator);
bool rs_controller_pda_simulator_handle_probe_packet(AQ_Probe_Packet* probePacketforSimulator);
bool rs_controller_pda_simulator_handle_unknown_packet(AQ_Unknown_Packet* probePacketforSimulator);

#endif // AQ_RS_CONTROLLER_H_
