#ifndef AQ_HARDWARE_DEVICE_TYPES_H_
#define AQ_HARDWARE_DEVICE_TYPES_H_

typedef enum HardwareDeviceTypes
{
	Master = 0x00,
	Keypad = 0x08,
	DualSpaSideSwitch = 0x10,
	Unknown_0x1x = 0x18,
	SPA_Remote = 0x20,
	Unknown_0x2x = 0x28,
	Aqualink = 0x30,
	LX_Heater = 0x38,
	OneTouch = 0x40,
	Unknown_0x4x = 0x48,
	SWG = 0x50,
	PC_Interface = 0x58,
	PDA_Remote = 0x60,
	Unknown_0x6x = 0x68,
	Unknown_0x7x = 0x70,
	Jandy_VSP_ePump = 0x78,
	ChemLink = 0x80,
	Unknown_0x8x = 0x88,
	iAqualink = 0xA0,
	UnknownDevice = 0xFC
}
HardwareDeviceTypes;

typedef enum HardwareDeviceInstanceTypes
{
	Instance_0 = 0x00,
	Instance_1 = 0x01,
	Instance_2 = 0x02,
	Instance_3 = 0x03,
}
HardwareDeviceInstanceTypes;

typedef struct HardwareDeviceId
{
	HardwareDeviceTypes Type;
	HardwareDeviceInstanceTypes Instance;
}
HardwareDeviceId;

extern const HardwareDeviceId INVALID_DEVICE_ID;

const char* device_id_to_string(HardwareDeviceId device_id);

#endif // AQ_HARDWARE_DEVICE_TYPES_H_
