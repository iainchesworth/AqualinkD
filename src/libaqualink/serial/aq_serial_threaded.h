#ifndef AQ_SERIAL_THREADED_H
#define AQ_SERIAL_THREADED_H

typedef enum tagSerialThread_States
{
	ST_INIT = 0,
	ST_READPACKET = 1,
	ST_WRITEPACKET = 2,
	ST_RECOVERY = 3,
	ST_TERMINATE = 4
}
SerialThread_States;

int serial_thread(void* termination_handler_ptr);

#endif // AQ_SERIAL_THREADED_H
