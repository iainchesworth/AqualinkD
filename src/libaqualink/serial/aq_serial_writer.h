#ifndef AQ_SERIAL_WRITER
#define AQ_SERIAL_WRITER

#include "cross-platform/serial.h"

typedef enum tagSerialThread_WriteStates
{
	ST_SEND_PACKETPAYLOAD = 0,
	ST_RETRY_SENDPAYLOAD = 1,
	ST_WRITEERROR_OCCURRED = 2,
	ST_TERMINATE_WRITEPACKET = 3
}
SerialThread_WriteStates;

int serial_sendnextpacket(SerialDevice serial_device);

#endif // AQ_SERIAL_WRITER
