#ifndef AQ_RS_KEYPAD_SIMULATOR_PROBE_HANDLER_H_
#define AQ_RS_KEYPAD_SIMULATOR_PROBE_HANDLER_H_

#include <stdbool.h>

#include "messages/aq_serial_message_probe.h"

bool rs_keypad_simulator_probemessagehandler(AQ_Probe_Packet* packet);

#endif // AQ_RS_KEYPAD_SIMULATOR_PROBE_HANDLER_H_
