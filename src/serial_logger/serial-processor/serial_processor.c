#include "serial_processor.h"

#ifdef OLD



int rs_fd;
int packet_length;
unsigned char packet_buffer[AQ_MAXPKTLEN];
unsigned char lastID = 0x00;
int i = 0;
bool found;
serial_id_log slog[SLOG_MAX];
serial_id_log pent_slog[SLOG_MAX];
int sindex = 0;
int pent_sindex = 0;
int received_packets = 0;
int logPackets = PACKET_MAX;
int logLevel = LOG_NOTICE;
//bool playback_file = false;

//int logLevel; 
//char buffer[256];
//bool idMode = true;

#define SLOG_MAX 80
#define PACKET_MAX 600

typedef struct serial_id_log {
	unsigned char ID;
	bool inuse;
} serial_id_log;

bool _keepRunning = true;

unsigned char _goodID[] = { 0x0a, 0x0b, 0x08, 0x09 };
unsigned char _goodPDAID[] = { 0x60, 0x61, 0x62, 0x63 };
unsigned char _filter[10];

int _filters = 0;
bool _rawlog = false;
bool _playback_file = false;

void intHandler(int dummy) {
	_keepRunning = false;
	NOTICE("Stopping!");
	if (_playback_file)  // If we are reading file, loop is irevelent
		exit(0);
}

#define MASTER " <-- Master control panel"
#define SWG " <-- Salt Water Generator (Aquarite mode)"
#define KEYPAD " <-- RS Keypad"
#define SPA_R " <-- Spa remote"
#define AQUA " <-- Aqualink (iAqualink?)"
#define HEATER " <-- LX Heater"
#define ONE_T " <-- Onetouch device"
#define PC_DOCK " <-- PC Interface (RS485 to RS232)"
#define PDA " <-- PDA Remote"
#define EPUMP " <-- Jandy VSP ePump"
#define CHEM " <-- Chemlink"

#define UNKNOWN " <-- Unknown Device"

#define P_VSP " <-- Pentair VSP"
#define P_MASTER " <-- Pentair Master (Probably Jandy RS Control Panel)"
#define P_SWG " <-- Salt Water Generator (Jandy mode)"
#define P_BCAST " <-- Broadcast address"
#define P_RCTL " <-- Remote wired controller"
#define P_RWCTL " <-- Remote wireless controller (Screen Logic)"
#define P_CTL " <-- Pool controller (EasyTouch)"

const char* getDevice(unsigned char ID) {
	if (ID >= 0x00 && ID <= 0x03)
		return MASTER;
	if (ID >= 0x08 && ID <= 0x0B)
		return KEYPAD;
	if (ID >= 0x50 && ID <= 0x53)
		return SWG;
	if (ID >= 0x20 && ID <= 0x23)
		return SPA_R;
	if (ID >= 0x30 && ID <= 0x33)
		return AQUA;
	if (ID >= 0x38 && ID <= 0x3B)
		return HEATER;
	if (ID >= 0x40 && ID <= 0x43)
		return ONE_T;
	if (ID >= 0x58 && ID <= 0x5B)
		return PC_DOCK;
	if (ID >= 0x60 && ID <= 0x63)
		return PDA;
	//if (ID >= 0x70 && ID <= 0x73)
	if (ID >= 0x78 && ID <= 0x7B)
		return EPUMP;
	if (ID >= 0x80 && ID <= 0x83)
		return CHEM;
	//if (ID == 0x08)
	//  return KEYPAD;

	return UNKNOWN;
}

const char* getPentairDevice(unsigned char ID) {
	if (ID >= 0x60 && ID <= 0x6F)
		return P_VSP;
	if (ID == 0x02)
		return P_SWG;
	if (ID == 0x10)
		return P_MASTER;
	if (ID == 0x0F)
		return P_BCAST;
	if (ID == 0x10)
		return P_CTL;
	if (ID == 0x20)
		return P_RCTL;
	if (ID == 0x22)
		return P_RWCTL;

	return UNKNOWN;
}

void advance_cursor() {
	static int pos = 0;
	char cursor[4] = { '/','-','\\','|' };
	printf("%c\b", cursor[pos]);
	fflush(stdout);
	pos = (pos + 1) % 4;
}

bool canUse(unsigned char ID) {
	int i;
	for (i = 0; i < 4; i++) {
		if (ID == _goodID[i])
			return true;
	}
	for (i = 0; i < 4; i++) {
		if (ID == _goodPDAID[i])
			return true;
	}
	return false;
}
char* canUseExtended(unsigned char ID) {
	int i;
	for (i = 0; i < 4; i++) {
		if (ID == _goodID[i])
			return " <-- can use for Aqualinkd";
	}
	for (i = 0; i < 4; i++) {
		if (ID == _goodPDAID[i])
			return " <-- can use for Aqualinkd (PDA mode only)";
	}
	return "";
}


void printHex(char* pk, int length)
{
	int i = 0;
	for (i = 0; i < length; i++)
	{
		printf("0x%02hhx|", pk[i]);
	}
}


void printPacket(unsigned char ID, unsigned char* packet_buffer, int packet_length)
{
	//if (_filter != 0x00 && ID != _filter && packet_buffer[PKT_DEST] != _filter )
	//  return;
	if (_rawlog) {
		printHex((char*)packet_buffer, packet_length);
		printf("\n");
		return;
	}

	if (_filters != 0)
	{
		int i;
		bool dest_match = false;
		bool src_match = false;

		for (i = 0; i < _filters; i++) {
			if (packet_buffer[PKT_DEST] == _filter[i])
				dest_match = true;
			if (ID == _filter[i] && packet_buffer[PKT_DEST] == 0x00)
				src_match = true;
		}

		if (dest_match == false && src_match == false)
			return;
	}

	/*
	  if (_filter != 0x00) {
		if ( packet_buffer[PKT_DEST]==0x00 && ID != _filter )
		  return;
		if ( packet_buffer[PKT_DEST]!=0x00 && packet_buffer[PKT_DEST] != _filter )
		  return;
	  }
	*/
	if (getProtocolType(packet_buffer) == JANDY) {
		if (packet_buffer[PKT_DEST] != 0x00)
			printf("\n");
		printf("Jandy   %4.4s 0x%02hhx of type %8.8s", (packet_buffer[PKT_DEST] == 0x00 ? "From" : "To"), (packet_buffer[PKT_DEST] == 0x00 ? ID : packet_buffer[PKT_DEST]), get_packet_type(packet_buffer, packet_length));
	}
	else {
		printf("Pentair From 0x%02hhx To 0x%02hhx       ", packet_buffer[PEN_PKT_FROM], packet_buffer[PEN_PKT_DEST]);
	}
	printf(" | HEX: ");
	printHex((char*)packet_buffer, packet_length);

	if (packet_buffer[PKT_CMD] == CMD_MSG || packet_buffer[PKT_CMD] == CMD_MSG_LONG) {
		printf("  Message : ");
		//fwrite(packet_buffer + 4, 1, AQ_MSGLEN+1, stdout);
		fwrite(packet_buffer + 4, 1, packet_length - 7, stdout);
	}

	//if (packet_buffer[PKT_DEST]==0x00)
	//  printf("\n\n");
	//else
	printf("\n");
}

#endif // OLD