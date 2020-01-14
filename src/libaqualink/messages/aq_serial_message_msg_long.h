#ifndef AQ_SERIAL_MSG_LONG_H_
#define AQ_SERIAL_MSG_LONG_H_

#include <stdint.h>

#include "hardware/devices/hardware_device_types.h"
#include "serial/aq_serial_types.h"

#define AQ_MSG_LONG_PACKET_MESSAGE_LENGTH 16

typedef struct tagAQ_Msg_Long_Packet
{
	struct
	{
		uint8_t DLE;
		uint8_t STX;
	}
	Header;

	HardwareDeviceId Destination;
	SerialData_Commands Command;

	unsigned char LineNumber;
	unsigned char Message[AQ_MSG_LONG_PACKET_MESSAGE_LENGTH];

	struct
	{
		uint8_t Checksum;
		uint8_t DLE;
		uint8_t ETX;
	}
	Terminator;
}
AQ_Msg_Long_Packet;

extern const unsigned int AQ_MSG_LONG_PACKET_LENGTH;

#endif // AQ_SERIAL_MSG_LONG_H_
