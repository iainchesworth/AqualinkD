#ifndef AQ_ONETOUCH_SIMULATOR_STATUS_HANDLER_H_
#define AQ_ONETOUCH_SIMULATOR_STATUS_HANDLER_H_

#include <stdbool.h>

#include "messages/aq_serial_message_status.h"

bool onetouch_simulator_statusmessagehandler(AQ_Status_Packet* packet);

#endif // AQ_ONETOUCH_SIMULATOR_STATUS_HANDLER_H_
