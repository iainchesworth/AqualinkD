#ifndef AQUAPURE_H_
#define AQUAPURE_H_

#include <stdbool.h>

#include "aqualink.h"

bool processPacketToSWG(unsigned char *packet, int packet_length, struct aqualinkdata *aqdata, int swg_zero_ignore);
bool processPacketFromSWG(const unsigned char *packet, int packet_length, struct aqualinkdata *aqdata);

void get_swg_status_mqtt(struct aqualinkdata *aqdata, char *message, int *status, int *dzalert);
aqledstate get_swg_led_state(struct aqualinkdata *aqdata);

#endif // AQUAPURE_H_