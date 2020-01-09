#ifndef AQ_SERIAL_TYPES_H_
#define AQ_SERIAL_TYPES_H_

/*
// PACKET DEFINES Jandy
#define NUL  0x00
#define DLE  0x10
#define STX  0x02
#define ETX  0x03
*/
typedef enum tagSerialData_JandyKeyBytes
{
	NUL = 0x00,
	DLE = 0x10,
	STX = 0x02,
	ETX = 0x03
}
SerialData_JandyKeyBytes;

/* DESTINATIONS */
typedef enum tagSerialData_Destinations
{
	Master_0 = 0x00,
	Master_1 = 0x01,
	Master_2 = 0x02,
	Master_3 = 0x03,

	Keypad_0 = 0x08,
	Keypad_1 = 0x09,
	Keypad_2 = 0x0A,
	Keypad_3 = 0x0B,

	DualSpaSideSwitch_InterfaceBoard = 0x10,

	SPA_Remote_0 = 0x20,
	SPA_Remote_1 = 0x21,
	SPA_Remote_2 = 0x22,
	SPA_Remote_3 = 0x23,

	Aqualink_0 = 0x30,
	Aqualink_1 = 0x31,
	Aqualink_2 = 0x32,
	Aqualink_3 = 0x33,

	LX_Heater_0 = 0x38,
	LX_Heater_1 = 0x39,
	LX_Heater_2 = 0x3A,
	LX_Heater_3 = 0x3B,

	OneTouch_0 = 0x40,
	OneTouch_1 = 0x41,
	OneTouch_2 = 0x42,
	OneTouch_3 = 0x43,

	SWG_0 = 0x50,
	SWG_1 = 0x51,
	SWG_2 = 0x52,
	SWG_3 = 0x53,

	PC_Interface_0 = 0x58,
	PC_Interface_1 = 0x59,
	PC_Interface_2 = 0x5A,
	PC_Interface_3 = 0x5B,

	PDA_Remote_0 = 0x60,
	PDA_Remote_1 = 0x61,
	PDA_Remote_2 = 0x62,
	PDA_Remote_3 = 0x63,

	Jandy_VSP_ePump_0 = 0x78,
	Jandy_VSP_ePump_1 = 0x79,
	Jandy_VSP_ePump_2 = 0x7A,
	Jandy_VSP_ePump_3 = 0x7B,

	ChemLink_0 = 0x80,
	ChemLink_1 = 0x81,
	ChemLink_2 = 0x82,
	ChemLink_3 = 0x83,

	iAqualink_0 = 0xA0,
	iAqualink_1 = 0xA1,
	iAqualink_2 = 0xA2,
	iAqualink_3 = 0xA3,

	Unknown_Device
}
SerialData_Destinations;

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
	CMD_PDA_0x04 = 0x04, // No idea, might be building menu
	CMD_PDA_0x05 = 0x05, // No idea
	CMD_PDA_0x1B = 0x1b,
	CMD_PDA_HIGHLIGHT = 0x08,
	CMD_PDA_CLEAR = 0x09,
	CMD_PDA_SHIFTLINES = 0x0F,
	CMD_PDA_HIGHLIGHTCHARS = 0x10,

	/* iAqualink */
	CMD_IAQ_MSG = 0x25,
	CMD_IAQ_MENU_MSG = 0x24
}
SerialData_Commands;

/* ACK RETURN COMMANDS */
typedef enum tagSerialData_AckTypes
{
	ACK_NORMAL = 0x00,
	ACK_SCREEN_BUSY = 0x01,			// Seems to be busy but can cache a message,
	ACK_SCREEN_BUSY_BLOCK = 0x03,	// Seems to be don't send me shit.
	ACK_PDA = 0x40,
	ACK_UNKNOWN_TYPE_1 = 0x80		// Unknown ACK type - found in serial traffic from a OneTouch
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
