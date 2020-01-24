#ifndef AQ_SERIAL_MESSAGE_SENDERS_H_
#define AQ_SERIAL_MESSAGE_SENDERS_H_

#include <stdbool.h>

#include "serial/aq_serial_types.h"

bool send_ack_packet(SerialData_AckTypes ackType, unsigned char commandBeingAcked);

#endif // AQ_SERIAL_MESSAGE_SENDERS_H_
