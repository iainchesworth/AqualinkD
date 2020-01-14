#include "aq_serial_checksums.h"

#include <assert.h>
#include <stdbool.h>

#include "logging/logging.h"
#include "utility/utils.h"

int generate_jandy_checksum(const unsigned char* packet, const int length)
{
	assert(0 != packet);

	// From the documentation of the protocol:
	//
	//    DLE STX <data> <checksum> DLE ETX
	//
	//    <data>     := <dest><command><args>
	//    <checksum> : = the 7-bit sum of bytes DLE, STX and <data>

	const int bytes_to_process = length - 3;
	int current_byte, checksum = 0;

	for (current_byte = 0; current_byte < bytes_to_process; ++current_byte)
	{
		checksum += (unsigned int) packet[current_byte];
	}

	return(checksum & 0xff);
}

void generate_pentair_checksum(unsigned char* packet, const int length)
{
	assert(0 != packet);

	int i, sum, n;
	n = packet[8] + 9;
	sum = 0;

	for (i = 3; i < n; i++)
	{
		sum += (int)packet[i];
	}

	packet[n + 1] = (unsigned char)(sum & 0xFF);        // Low Byte
	packet[n] = (unsigned char)((sum >> 8) & 0xFF);		// High Byte

}

bool validate_jandy_checksum(const unsigned char* packet, const int length)
{
	assert(0 != packet);

	return (packet[length - 3] == generate_jandy_checksum(packet, length)) ? true : false;
}

bool validate_pentair_checksum(const unsigned char* packet, const int length)
{
	assert(0 != packet);

	int i, sum, n;
	n = packet[8] + 9;
	sum = 0;
	for (i = 3; i < n; i++) 
	{
		sum += (int)packet[i];
	}

	// Check against calculated length
	if (sum == (packet[length - 2] * 256 + packet[length - 1]))
	{
		return true;
	}

	// Check against actual # length
	if (sum == (packet[n] * 256 + packet[n + 1])) 
	{
		ERROR("Pentair checksum is accurate but length is not");
		return true;
	}

	return false;
}
