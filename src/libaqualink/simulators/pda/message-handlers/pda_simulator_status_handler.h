#ifndef PDA_SIMULATOR_STATUS_HANDLER_H_
#define PDA_SIMULATOR_STATUS_HANDLER_H_

#include <stdbool.h>

#include "messages/aq_serial_message_status.h"

bool pda_simulator_statusmessagehandler(AQ_Status_Packet* packet);

#endif // PDA_SIMULATOR_STATUS_HANDLER_H_
