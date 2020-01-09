#ifndef AQ_SERIAL_H_
#define AQ_SERIAL_H_

#include <stdbool.h>

#if defined (WIN32)

#include <Windows.h>
typedef HANDLE SerialDevice;

#else // defined (WIN32)

typedef int SerialDevice;

#endif // defined (WIN32)

#if defined (_MSC_VER)
///FIXME -> MSVC uses #pragma packed and whatnot...grrr.
// #define PACKED_SERIAL_STRUCT __attribute__((__packed__))
#endif // defined (_MSC_VER)

#if defined (__clang__)
#define PACKED_SERIAL_STRUCT __attribute__((__packed__))
#endif // defined (CLANG)

#if defined (__GNUC__)
#define PACKED_SERIAL_STRUCT __attribute__((__packed__))
#endif // defined (CLANG)

extern const SerialDevice SERIALDEVICE_INVALID;

SerialDevice initialise_serial_device();
bool set_interface_attributes(SerialDevice serial_device);
int read_from_serial_device(SerialDevice serial_device, unsigned char buffer[], unsigned int buffer_length);
int write_to_serial_device(SerialDevice serial_device, const unsigned char buffer[], unsigned int buffer_length);
void close_serial_device(SerialDevice serial_device);

#endif // AQ_SERIAL_H_
