#ifndef AQ_SERIAL_H_
#define AQ_SERIAL_H_

#include <stdbool.h>

#if defined (WIN32)

#include <Windows.h>
typedef HANDLE SerialDevice;

#else // defined (WIN32)

typedef int SerialDevice;

#endif // defined (WIN32)

extern const SerialDevice SERIALDEVICE_INVALID;

SerialDevice initialise_serial_device();
bool set_interface_attributes(SerialDevice serial_device);
int read_from_serial_device(SerialDevice serial_device, unsigned char buffer[], unsigned int buffer_length);
int write_to_serial_device(SerialDevice serial_device, const unsigned char buffer[], unsigned int buffer_length);
void close_serial_device(SerialDevice serial_device);

#endif // AQ_SERIAL_H_
