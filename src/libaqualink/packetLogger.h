#ifndef PACKETLOGGER_H_
#define PACKETLOGGER_H_

#include <stdbool.h>

#define RS485LOGFILE "/tmp/RS485.log"
#define RS485BYTELOGFILE "/tmp/RS485raw.log"

void startPacketLogger(bool debug_RSProtocol_packets, bool read_pentair_packets);
void stopPacketLogger();
//void logPacket(unsigned char *packet_buffer, int packet_length, bool checksumerror);
void logPacket(unsigned char *packet_buffer, int packet_length);
void logPacketError(unsigned char *packet_buffer, int packet_length);
void logPacketByte(unsigned char *byte);


#endif //PACKETLOGGER_H_