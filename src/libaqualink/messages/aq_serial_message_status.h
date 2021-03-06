#ifndef AQ_SERIAL_MESSAGE_STATUS_H_
#define AQ_SERIAL_MESSAGE_STATUS_H_

#include <stdint.h>

#include "hardware/devices/hardware_device_types.h"
#include "serial/aq_serial_types.h"

typedef struct tagAQ_Status_Packet
{
	struct
	{
		uint8_t DLE;
		uint8_t STX;
	}
	Header;

	HardwareDeviceId Destination;
	SerialData_Commands Command;

	AQ_LED_States LED_00;
	AQ_LED_States LED_01;
	AQ_LED_States LED_02;
	AQ_LED_States LED_03;
	AQ_LED_States LED_04;
	AQ_LED_States LED_05;
	AQ_LED_States LED_06;
	AQ_LED_States LED_07;
	AQ_LED_States LED_08;
	AQ_LED_States LED_09;
	AQ_LED_States LED_10;
	AQ_LED_States LED_11;
	AQ_LED_States LED_12;
	AQ_LED_States LED_13;
	AQ_LED_States LED_14;
	AQ_LED_States Pool_Heater;		// POOL_HTR_LED_INDEX
	AQ_LED_States LED_16;
	AQ_LED_States SPA_Heater;		// SPA_HTR_LED_INDEX
	AQ_LED_States LED_18;
	AQ_LED_States Solar_Heater;		// SOLAR_HTR_LED_INDEX

	struct
	{
		uint8_t Checksum;
		uint8_t DLE;
		uint8_t ETX;
	}
	Terminator;
}
AQ_Status_Packet;

extern const unsigned int AQ_STATUS_PACKET_LENGTH;

#endif // AQ_SERIAL_MESSAGE_STATUS_H_
