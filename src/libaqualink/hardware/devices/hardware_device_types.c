#include "hardware_device_types.h"

const DeviceId INVALID_DEVICE_ID = 0xFF;

const char* const MASTER = " <-- Master control panel";
const char* const SWG = " <-- Salt Water Generator (Aquarite mode)";
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

const char* const device_id_to_string(DeviceId device_id)
{
    if (device_id >= 0x00 && device_id <= 0x03)
    {
        return MASTER;
    }
    if (device_id >= 0x08 && device_id <= 0x0B)
    {
        return KEYPAD;
    }
    if (device_id >= 0x50 && device_id <= 0x53)
    {
        return SWG;
    }
    if (device_id >= 0x20 && device_id <= 0x23)
    {
        return SPA_R;
    }
    if (device_id >= 0x30 && device_id <= 0x33)
    {
        return AQUA;
    }
    if (device_id >= 0x38 && device_id <= 0x3B)
    {
        return HEATER;
    }
    if (device_id >= 0x40 && device_id <= 0x43)
    {
        return ONE_T;
    }
    if (device_id >= 0x58 && device_id <= 0x5B)
    {
        return PC_DOCK;
    }
    if (device_id >= 0x60 && device_id <= 0x63)
    {
        return PDA;
    }
    if (device_id >= 0x78 && device_id <= 0x7B)
    {
        return EPUMP;
    }
    if (device_id >= 0x80 && device_id <= 0x83)
    {
        return CHEM;
    }

    return UNKNOWN;
}
