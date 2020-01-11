#ifndef AQ_SERIAL_READER_H_
#define AQ_SERIAL_READER_H_

#include "cross-platform/serial.h"

typedef enum tagSerialThread_ReadStates
{
	ST_WAITFOR_PACKETSTART		= 0,
	ST_RECEIVE_PACKETPAYLOAD	= 1,
	ST_RETRY_RECEIVEPAYLOAD		= 2,
	ST_VALIDATE_PACKETPAYLOAD   = 3,
	ST_READERROR_OCCURRED		= 4,
	ST_TERMINATE_READPACKET		= 5
}
SerialThread_ReadStates;

int serial_getnextpacket(SerialDevice serial_device, unsigned char* packet);

#endif // AQ_SERIAL_READER_H_
