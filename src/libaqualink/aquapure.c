
#include <stdio.h>

#include "aquapure.h"
#include "logging/logging.h"
#include "serial/aq_serial.h"
#include "aqualink.h"
#include "utils.h"
#include "aq_mqtt.h"

bool processPacketToSWG(unsigned char* packet, int packet_length, struct aqualinkdata* aqdata, int swg_zero_ignore) {
	static int swg_zero_cnt = 0;
	bool changedAnything = false;

	// Only read message from controller to SWG to set SWG Percent if we are not programming, as we might be changing this
	if (packet[3] == CMD_PERCENT && aqdata->active_thread.thread_id == 0 && packet[4] != 0xFF) {
		// In service or timeout mode SWG set % message is very strange. AR %% | HEX: 0x10|0x02|0x50|0x11|0xff|0x72|0x10|0x03|
		// Not really sure what to do with this, just ignore 0xff / 255 for the moment. (if statment above)

		// SWG can get ~10 messages to set to 0 then go back again for some reason, so don't go to 0 until 10 messages are received
		if (swg_zero_cnt <= swg_zero_ignore && packet[4] == 0x00 && packet[5] == 0x73) {
			DEBUG("Ignoring SWG set to %d due to packet packet count %d <= %d from control panel to SWG 0x%02hhx 0x%02hhx", (int)packet[4],
				swg_zero_cnt, swg_zero_ignore, packet[4], packet[5]);
			swg_zero_cnt++;
		}
		else if (swg_zero_cnt > swg_zero_ignore&& packet[4] == 0x00 && packet[5] == 0x73) {
			aqdata->swg_percent = (int)packet[4];
			changedAnything = true;
		}
		else {
			swg_zero_cnt = 0;
			aqdata->swg_percent = (int)packet[4];
			changedAnything = true;
		}

		if (aqdata->swg_percent > 100) {
			aqdata->boost = true;
		}
		else {
			aqdata->boost = false;
		}
	}
	return changedAnything;
}

bool processPacketFromSWG(const unsigned char* packet, int packet_length, struct aqualinkdata* aqdata) {
	bool changedAnything = false;

	if (packet[PKT_CMD] == CMD_PPM) {
		aqdata->ar_swg_status = packet[5];
		if (aqdata->swg_delayed_percent != TEMP_UNKNOWN && aqdata->ar_swg_status == SWG_STATUS_ON) { // We have a delayed % to set.
			char sval[10];
			snprintf(sval, 9, "%d", aqdata->swg_delayed_percent);
			aq_programmer(AQ_SET_SWG_PERCENT, sval, aqdata);
			NOTICE("Setting SWG %% to %d, from delayed message", aqdata->swg_delayed_percent);
			aqdata->swg_delayed_percent = TEMP_UNKNOWN;
		}
		aqdata->swg_ppm = packet[4] * 100;
		changedAnything = true;
		// DEBUG("Read SWG PPM %d from ID 0x%02hhx", aqdata.swg_ppm, SWG_DEV_ID);
	}

	return changedAnything;
}


AQ_LED_States get_swg_led_state(struct aqualinkdata* aqdata)
{
	switch (aqdata->ar_swg_status) {
		// Level = (0=gray, 1=green, 2=yellow, 3=orange, 4=red)
	case SWG_STATUS_ON:
		return (aqdata->swg_percent > 0 ? ON : ENABLE);
		break;
	case SWG_STATUS_NO_FLOW:
		return ENABLE;
		break;
	case SWG_STATUS_LOW_SALT:
		return (aqdata->swg_percent > 0 ? ON : ENABLE);
		break;
	case SWG_STATUS_HI_SALT:
		return (aqdata->swg_percent > 0 ? ON : ENABLE);
		break;
	case SWG_STATUS_HIGH_CURRENT:
		return (aqdata->swg_percent > 0 ? ON : ENABLE);
		break;
	case SWG_STATUS_TURNING_OFF:
		return OFF;
		break;
	case SWG_STATUS_CLEAN_CELL:
		return (aqdata->swg_percent > 0 ? ON : ENABLE);
		return ENABLE;
		break;
	case SWG_STATUS_LOW_VOLTS:
		return ENABLE;
		break;
	case SWG_STATUS_LOW_TEMP:
		return ENABLE;
		break;
	case SWG_STATUS_CHECK_PCB:
		return ENABLE;
		break;
	case SWG_STATUS_OFF: // THIS IS OUR OFF STATUS, NOT AQUAPURE
		return OFF;
		break;
	default:
		return (aqdata->swg_percent > 0 ? ON : ENABLE);
		break;
	}
}

void get_swg_status_mqtt(struct aqualinkdata* aqdata, char* message, int* status, int* dzalert)
{
	switch (aqdata->ar_swg_status) {
		// Level = (0=gray, 1=green, 2=yellow, 3=orange, 4=red)
	case SWG_STATUS_ON:
		*status = (aqdata->swg_percent > 0 ? SWG_ON : SWG_OFF);
		sprintf(message, "AQUAPURE GENERATING CHLORINE");
		*dzalert = 1;
		break;
	case SWG_STATUS_NO_FLOW:
		*status = SWG_OFF;
		sprintf(message, "AQUAPURE NO FLOW");
		*dzalert = 2;
		break;
	case SWG_STATUS_LOW_SALT:
		*status = (aqdata->swg_percent > 0 ? SWG_ON : SWG_OFF);
		sprintf(message, "AQUAPURE LOW SALT");
		*dzalert = 2;
		break;
	case SWG_STATUS_HI_SALT:
		*status = (aqdata->swg_percent > 0 ? SWG_ON : SWG_OFF);
		sprintf(message, "AQUAPURE HIGH SALT");
		*dzalert = 3;
		break;
	case SWG_STATUS_HIGH_CURRENT:
		*status = (aqdata->swg_percent > 0 ? SWG_ON : SWG_OFF);
		sprintf(message, "AQUAPURE HIGH CURRENT");
		*dzalert = 4;
		break;
	case SWG_STATUS_TURNING_OFF:
		*status = SWG_OFF;
		sprintf(message, "AQUAPURE TURNING OFF");
		*dzalert = 0;
		break;
	case SWG_STATUS_CLEAN_CELL:
		*status = (aqdata->swg_percent > 0 ? SWG_ON : SWG_OFF);
		sprintf(message, "AQUAPURE CLEAN CELL");
		*dzalert = 2;
		break;
	case SWG_STATUS_LOW_VOLTS:
		*status = (aqdata->swg_percent > 0 ? SWG_ON : SWG_OFF);
		sprintf(message, "AQUAPURE LOW VOLTAGE");
		*dzalert = 3;
		break;
	case SWG_STATUS_LOW_TEMP:
		*status = SWG_OFF;
		sprintf(message, "AQUAPURE WATER TEMP LOW");
		*dzalert = 2;
		break;
	case SWG_STATUS_CHECK_PCB:
		*status = SWG_OFF;
		sprintf(message, "AQUAPURE CHECK PCB");
		*dzalert = 4;
		break;
	case SWG_STATUS_OFF: // THIS IS OUR OFF STATUS, NOT AQUAPURE
		*status = SWG_OFF;
		sprintf(message, "AQUAPURE OFF");
		*dzalert = 0;
		break;
	default:
		*status = (aqdata->swg_percent > 0 ? SWG_ON : SWG_OFF);
		sprintf(message, "AQUAPURE UNKNOWN STATUS");
		*dzalert = 4;
		break;
	}
}
