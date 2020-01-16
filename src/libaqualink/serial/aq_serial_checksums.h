#ifndef AQ_SERIAL_CHECKSUMS_H_
#define AQ_SERIAL_CHECKSUMS_H_

#include <stdbool.h>

unsigned char generate_jandy_checksum(const unsigned char* packet, const int length);
void generate_pentair_checksum(unsigned char* packet, const int length);

bool validate_jandy_checksum(const unsigned char* packet, const int length);
bool validate_pentair_checksum(const unsigned char* packet, const int length);

#endif // AQ_SERIAL_CHECKSUMS_H_
