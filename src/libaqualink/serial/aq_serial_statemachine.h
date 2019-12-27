#ifndef AQ_SERIAL_STATEMACHINE_H_
#define AQ_SERIAL_STATEMACHINE_H_

typedef enum tagSerialThread_States
{
	ST_INIT = 0,
	ST_READPACKET = 1,
	ST_WRITEPACKET = 2,
	ST_RECOVERY = 3,
	ST_TERMINATE = 4
}
SerialThread_States;

typedef enum tagSerialThread_ReadStates
{
	ST_WAITFOR_PACKETSTART		= 0,
	ST_RECEIVE_PACKETPAYLOAD	= 1,
	ST_RETRY_RECEIVEPAYLOAD		= 2,
	ST_VALIDATE_PACKETPAYLOAD   = 3,
	ST_ERROR_OCCURRED			= 4,
	ST_TERMINATE_READPACKET		= 5
}
SerialThread_ReadStates;

int serial_getnextpacket(int fd, unsigned char* packet);

#endif // AQ_SERIAL_STATEMACHINE_H_
