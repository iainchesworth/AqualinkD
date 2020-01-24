#ifndef AQ_SERIAL_TYPES_H_
#define AQ_SERIAL_TYPES_H_

/* PACKET DEFINES - Jandy */
typedef enum tagSerialData_JandyKeyBytes
{
	NUL = 0x00,
	DLE = 0x10,
	STX = 0x02,
	ETX = 0x03
}
SerialData_JandyKeyBytes;

/* COMMANDS */
typedef enum tagSerialData_Commands
{
	CMD_PROBE = 0x00,
	CMD_ACK = 0x01,
	CMD_STATUS = 0x02,
	CMD_MSG = 0x03,
	CMD_MSG_LONG = 0x04,

	/* AquaRite commands */
	CMD_GETID = 0x14,  // May be remote control control
	CMD_PERCENT = 0x11,  // Set Percent
	CMD_PPM = 0x16, // Received PPM

	/* PDA commands */
	CMD_PDA_0x05 = 0x05, // No idea
	CMD_PDA_0x1B = 0x1b,
	CMD_PDA_HIGHLIGHT = 0x08,
	CMD_PDA_CLEAR = 0x09,
	CMD_PDA_SHIFTLINES = 0x0F,
	CMD_PDA_HIGHLIGHTCHARS = 0x10,

	/* iAqualink */
	CMD_IAQ_0x23 = 0x23,		// 8 bytes
	CMD_IAQ_MENU_MSG = 0x24,	// ????
	CMD_IAQ_MSG = 0x25,			// ????
	CMD_IAQ_0x28 = 0x28,		// 12 bytes
	CMD_IAQ_0x2d = 0x2d,		// 22 bytes

	CMD_UNKNOWN = 0xFF
}
SerialData_Commands;

/* ACK RETURN COMMANDS */
typedef enum tagSerialData_AckTypes
{
	ACK_NORMAL			  = 0x00,
	ACK_SCREEN_BUSY		  = 0x01,	// Seems to be busy but can cache a message,
	ACK_SCREEN_BUSY_BLOCK = 0x03,	// Seems to be don't send me shit.
	ACK_PDA				  = 0x40,
	ACK_UNKNOWN_TYPE_1	  = 0x80	// Unknown ACK type - found in serial traffic from a OneTouch
}
SerialData_AckTypes;

typedef enum tagAQ_LED_States
{
	ON = 0,
	OFF = 1,
	FLASH = 2,
	ENABLE = 3,
	LED_S_UNKNOWN = -1
}
AQ_LED_States;

#endif  // AQ_SERIAL_TYPES_H_
