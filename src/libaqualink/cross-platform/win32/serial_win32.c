#include "../serial.h"

#if defined (WIN32)

#include <Windows.h>

#include "config/config_helpers.h"
#include "logging/logging.h"

const SerialDevice SERIALDEVICE_INVALID = INVALID_HANDLE_VALUE;

static DCB original_serial_config, aqualink_serial_config;

LPVOID GetFormattedMessage(DWORD dwErrorCode)
{
	LPVOID lpMsgBuf;

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, dwErrorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL);

	return lpMsgBuf;
}

SerialDevice initialise_serial_device()
{
	const char* serial_device = CFG_SerialPort();
	HANDLE hSerialDevice;

	hSerialDevice = CreateFile(serial_device, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
	if (SERIALDEVICE_INVALID == hSerialDevice)
	{
		if (ERROR_FILE_NOT_FOUND == GetLastError())
		{
			ERROR("Failed to open serial port: %s", CFG_SerialPort());
			ERROR("    Specified serial port doesn't exist...check your configuration");
		}
		else
		{
			DWORD dwErrorCode = GetLastError();
			LPVOID lpMsgBuf = GetFormattedMessage(dwErrorCode);		
				
			ERROR("Failed to open serial port: %s", CFG_SerialPort());
			ERROR("    CreateFile() error: %d - %s", dwErrorCode, lpMsgBuf);

			LocalFree(lpMsgBuf);
		}
	}
	else
	{
		// Initialisation was successful.
		DEBUG("Successfully opened serial device: %s", CFG_SerialPort());
	}

	return hSerialDevice;
}

bool set_interface_attributes(SerialDevice serial_device)
{
	bool returnCode = false;

	// Initialise the serial device tty config structures!
	memset(&original_serial_config, 0, sizeof(DCB));
	memset(&aqualink_serial_config, 0, sizeof(DCB));

	original_serial_config.DCBlength = sizeof(DCB);

	if (!GetCommState(serial_device, &original_serial_config))
	{
		DWORD dwErrorCode = GetLastError();
		LPVOID lpMsgBuf = GetFormattedMessage(dwErrorCode);

		WARN("Failed to get serial port attributes");
		WARN("    GetCommState() error: %d - %s", dwErrorCode, lpMsgBuf);

		LocalFree(lpMsgBuf);
	}
	else
	{
		memcpy(&aqualink_serial_config, &original_serial_config, sizeof(DCB));

		aqualink_serial_config.BaudRate = CBR_9600;
		aqualink_serial_config.ByteSize = 8;
		aqualink_serial_config.StopBits = ONESTOPBIT;
		aqualink_serial_config.Parity = NOPARITY;

		if (!SetCommState(serial_device, &aqualink_serial_config))
		{
			DWORD dwErrorCode = GetLastError();
			LPVOID lpMsgBuf = GetFormattedMessage(dwErrorCode);

			ERROR("Failed to set serial port baud attributes");
			ERROR("    GetCommState() error: %d - %s", dwErrorCode, lpMsgBuf);

			LocalFree(lpMsgBuf);
		}
		else
		{
			DEBUG("Serial port configured successfully");
			returnCode = true;
		}
	}

	return returnCode;
}

int read_from_serial_device(SerialDevice serial_device, unsigned char buffer[], unsigned int buffer_length)
{
	DWORD dwBytesRead = 0;

	if (!ReadFile(serial_device, buffer, buffer_length, &dwBytesRead, NULL))
	{
		DWORD dwErrorCode = GetLastError();
		LPVOID lpMsgBuf = GetFormattedMessage(dwErrorCode);

		DEBUG("Failed to read from serial port");
		DEBUG("    ReadFile() error: %d - %s", dwErrorCode, lpMsgBuf);

		LocalFree(lpMsgBuf);

		return -1;
	}

	return dwBytesRead;
}

int write_to_serial_device(SerialDevice serial_device, const unsigned char buffer[], unsigned int buffer_length)
{
	DWORD dwBytesWritten = 0;

	if (!WriteFile(serial_device, buffer, buffer_length, &dwBytesWritten, NULL))
	{
		DWORD dwErrorCode = GetLastError();
		LPVOID lpMsgBuf = GetFormattedMessage(dwErrorCode);

		DEBUG("Failed to write to serial port");
		DEBUG("    WriteFile() error: %d - %s", dwErrorCode, lpMsgBuf);

		LocalFree(lpMsgBuf);

		return -1;
	}

	return dwBytesWritten;
}

void close_serial_device(SerialDevice serial_device)
{
	CloseHandle(serial_device);
}

#endif // defined (WIN32)
