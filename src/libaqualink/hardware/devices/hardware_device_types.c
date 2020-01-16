#include "hardware_device_types.h"

const HardwareDeviceId INVALID_DEVICE_ID = { .Type = UnknownDevice, .Instance = Instance_0 };

const char* const MASTER = " <-- Master control panel";
const char* const DUALSPASIDESWITCH = " <-- Dual spa side switch";
const char* const SW_GEN = " <-- Salt Water Generator (Aquarite mode)";
const char* const KEYPAD = " <-- RS Keypad";
const char* const SPA_R = " <-- Spa remote";
const char* const AQUA = " <-- Aqualink (iAqualink?)";
const char* const HEATER = " <-- LX Heater";
const char* const ONE_T = " <-- Onetouch device";
const char* const PC_DOCK = " <-- PC Interface (RS485 to RS232)";
const char* const PDA = " <-- PDA Remote";
const char* const EPUMP = " <-- Jandy VSP ePump";
const char* const CHEM = " <-- Chemlink";

const char* const P_VSP = " <-- Pentair VSP";
const char* const P_MASTER = " <-- Pentair Master (Probably Jandy RS Control Panel)";
const char* const P_SWG = " <-- Salt Water Generator (Jandy mode)";
const char* const P_BCAST = " <-- Broadcast address";
const char* const P_RCTL = " <-- Remote wired controller";
const char* const P_RWCTL = " <-- Remote wireless controller (Screen Logic)";
const char* const P_CTL = " <-- Pool controller (EasyTouch)";

const char* const UNKNOWN = " <-- Unknown Device";

const char* device_id_to_string(HardwareDeviceId device_id)
{
    switch (device_id.Type)
    {
    case Master: 
        return MASTER;

    case Keypad: 
        return KEYPAD;

    case DualSpaSideSwitch: 
        return DUALSPASIDESWITCH;

    case SPA_Remote:
        return SPA_R;

    case Aqualink:
    case iAqualink:
        return AQUA;

    case LX_Heater:
        return HEATER;

    case OneTouch:
        return ONE_T;

    case SWG:
        return SW_GEN;

    case PC_Interface:
        return PC_DOCK;

    case PDA_Remote:
        return PDA;

    case Jandy_VSP_ePump:
        return EPUMP;

    case ChemLink:
        return CHEM;

    case Unknown_0x1x:
    case Unknown_0x2x:
    case Unknown_0x4x:
    case Unknown_0x6x:
    case Unknown_0x7x:
    case Unknown_0x8x:
    case UnknownDevice:
    default:
        return UNKNOWN;
    }
}
