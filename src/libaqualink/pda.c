/*
 * Copyright (c) 2017 Shaun Feakes - All rights reserved
 *
 * You may use redistribute and/or modify this code under the terms of
 * the GNU General Public License version 2 as published by the
 * Free Software Foundation. For the terms of this license,
 * see <http://www.gnu.org/licenses/>.
 *
 * You are free to use this software under the terms of the GNU General
 * Public License, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 *  https://github.com/sfeakes/aqualinkd
 */

#define _GNU_SOURCE 1 // for strcasestr & strptime

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "aqualink.h"

#include "hardware/buttons/buttons.h"
#include "logging/logging.h"
#include "pda.h"
#include "pda_menu.h"
#include "utils.h"

static struct aqualinkdata* _aqualink_data;
static unsigned char _last_packet_type;
static unsigned long _pda_loop_cnt = 0;
static bool _initWithRS = false;

// Each RS message is around 0.25 seconds apart

#define PDA_SLEEP_FOR 120 // 30 seconds
#define PDA_WAKE_FOR 6 // ~1 seconds

#ifdef BETA_PDA_AUTOLABEL
static struct aqconfig* _aqualink_config;
void init_pda(struct aqualinkdata* aqdata, struct aqconfig* aqconfig)
{
	_aqualink_data = aqdata;
	_aqualink_config = aqconfig;
	set_pda_mode(true);
}
#else
void init_pda(struct aqualinkdata* aqdata)
{
	_aqualink_data = aqdata;
	set_pda_mode(true);
}
#endif


bool pda_shouldSleep() {
	if (_pda_loop_cnt++ < PDA_WAKE_FOR) 
	{
		return false;
	}

	if (_pda_loop_cnt > PDA_WAKE_FOR + PDA_SLEEP_FOR) 
	{
		_pda_loop_cnt = 0;
		return false;
	}

	// NSF NEED TO CHECK ACTIVE THREADS.
	if (_aqualink_data->active_thread.thread_id != 0) 
	{
		DEBUG("PDA can't sleep as thread %d,%p is active",
			_aqualink_data->active_thread.ptype,
			_aqualink_data->active_thread.thread_id);

		_pda_loop_cnt = 0;
		return false;
	}

	// Last see if there are any open websockets. (don't sleep if the web UI is open)
	if (_aqualink_data->open_websockets > 0) 
	{
		DEBUG("PDA can't sleep as websocket is active");
		return false;
	}

	return true;
}

void pda_wake() {
	pda_reset_sleep();
	// Add and specic code to run when wake is called. 
}

void pda_reset_sleep() {
	_pda_loop_cnt = 0;
}

unsigned char get_last_pda_packet_type()
{
	return _last_packet_type;
}

void set_pda_led(struct aqualinkled* led, char state)
{
	AQ_LED_States old_state = led->state;
	if (state == 'N')
	{
		led->state = ON;
	}
	else if (state == 'A')
	{
		led->state = ENABLE;
	}
	else if (state == '*')
	{
		led->state = FLASH;
	}
	else
	{
		led->state = OFF;
	}
	if (old_state != led->state)
	{
		DEBUG("set_pda_led from %d to %d", old_state, led->state);
	}
}

void pass_pda_equiptment_status_item(char* msg)
{
	static char* index;
	int i;

	// EQUIPMENT STATUS
	//
	//  AquaPure 100%
	//  SALT 25500 PPM
	//   FILTER PUMP
	//    POOL HEAT
	//   SPA HEAT ENA

	// EQUIPMENT STATUS
	//
	//  FREEZE PROTECT
	//  AquaPure 100%
	//  SALT 25500 PPM
	//  CHECK AquaPure
	//  GENERAL FAULT
	//   FILTER PUMP
	//     CLEANER
	//
	// Equipment Status
	// 
	// Intelliflo VS 1 
	//      RPM: 1700  
	//     Watts: 367  
	// 
	// 
	// 
	// 
	// 

	// Check message for status of device
	// Loop through all buttons and match the PDA text.
	if ((index = strcasestr(msg, "CHECK AquaPure")) != NULL)
	{
		DEBUG("CHECK AquaPure");
	}
	else if ((index = strcasestr(msg, "FREEZE PROTECT")) != NULL)
	{
		_aqualink_data->frz_protect_state = ON;
	}
	else if ((index = strcasestr(msg, MSG_SWG_PCT)) != NULL)
	{
		_aqualink_data->swg_percent = atoi(index + strlen(MSG_SWG_PCT));
		if (_aqualink_data->ar_swg_status == SWG_STATUS_OFF) { _aqualink_data->ar_swg_status = SWG_STATUS_ON; }
		DEBUG("AquaPure = %d", _aqualink_data->swg_percent);
	}
	else if ((index = strcasestr(msg, MSG_SWG_PPM)) != NULL)
	{
		_aqualink_data->swg_ppm = atoi(index + strlen(MSG_SWG_PPM));
		if (_aqualink_data->ar_swg_status == SWG_STATUS_OFF) { _aqualink_data->ar_swg_status = SWG_STATUS_ON; }
		DEBUG("SALT = %d", _aqualink_data->swg_ppm);
	}
	else if ((index = strcasestr(msg, MSG_PMP_RPM)) != NULL)
	{ // Default to pump 0, should check for correct pump
		_aqualink_data->pumps[0].rpm = atoi(index + strlen(MSG_PMP_RPM));
		DEBUG("RPM = %d", _aqualink_data->pumps[0].rpm);
	}
	else if ((index = strcasestr(msg, MSG_PMP_WAT)) != NULL)
	{ // Default to pump 0, should check for correct pump
		_aqualink_data->pumps[0].watts = atoi(index + strlen(MSG_PMP_WAT));
		DEBUG("Watts = %d", _aqualink_data->pumps[0].watts);
	}
	else
	{
		char labelBuff[AQ_MSGLEN + 1];
		strncpy(labelBuff, msg, AQ_MSGLEN + 1);
		msg = stripwhitespace(labelBuff);

		if (strcasecmp(msg, "POOL HEAT ENA") == 0)
		{
			_aqualink_data->aqbuttons[PoolHeater].led->state = ENABLE;
		}
		else if (strcasecmp(msg, "SPA HEAT ENA") == 0)
		{
			_aqualink_data->aqbuttons[SpaHeater].led->state = ENABLE;
		}
		else
		{
			for (i = 0; i < AqualinkButtonCount; i++)
			{
				if (strcasecmp(msg, _aqualink_data->aqbuttons[i].pda_label) == 0)
				{
					DEBUG("*** Found Status for %s = '%.*s'", _aqualink_data->aqbuttons[i].pda_label, AQ_MSGLEN, msg);
					// It's on (or delayed) if it's listed here.
					if (_aqualink_data->aqbuttons[i].led->state != FLASH)
					{
						_aqualink_data->aqbuttons[i].led->state = ON;
					}
					break;
				}
			}
		}
	}
}

void process_pda_packet_msg_long_temp(const char* msg)
{
	//           'AIR         POOL'
	//           ' 86`     86`    '
	//           'AIR   SPA       '
	//           ' 86` 86`        '
	//           'AIR             '
	//           ' 86`            '
	//           'AIR        WATER'  // In case of single device.
	_aqualink_data->temp_units = FAHRENHEIT; // Force FAHRENHEIT
	if (stristr(pda_m_line(1), "AIR") != NULL)
	{
		_aqualink_data->air_temp = atoi(msg);
	}

	if (stristr(pda_m_line(1), "SPA") != NULL)
	{
		_aqualink_data->spa_temp = atoi(msg + 4);
		_aqualink_data->pool_temp = TEMP_UNKNOWN;
	}
	else if (stristr(pda_m_line(1), "POOL") != NULL)
	{
		_aqualink_data->pool_temp = atoi(msg + 7);
		_aqualink_data->spa_temp = TEMP_UNKNOWN;
	}
	else if (stristr(pda_m_line(1), "WATER") != NULL)
	{
		_aqualink_data->pool_temp = atoi(msg + 7);
		_aqualink_data->spa_temp = TEMP_UNKNOWN;
	}
	else
	{
		_aqualink_data->pool_temp = TEMP_UNKNOWN;
		_aqualink_data->spa_temp = TEMP_UNKNOWN;
	}
}

void process_pda_packet_msg_long_time(const char* msg)
{
	// message "     SAT 8:46AM "
	//         "     SAT 10:29AM"
	//         "     SAT 4:23PM "
	if (msg[AQ_MSGLEN - 1] == ' ')
	{
		strncpy(_aqualink_data->time, msg + 9, 6);
	}
	else
	{
		strncpy(_aqualink_data->time, msg + 9, 7);
	}
	strncpy(_aqualink_data->date, msg + 5, 3);
	// :TODO: NSF Come back and change the above to correctly check date and time in future
}

void process_pda_packet_msg_long_equipment_control(const char* msg)
{
	// These are listed as "FILTER PUMP  OFF"
	int i;
	char labelBuff[AQ_MSGLEN + 1];
	strncpy(labelBuff, msg, AQ_MSGLEN - 4);
	labelBuff[AQ_MSGLEN - 4] = 0;

	DEBUG("*** Checking Equiptment '%s'", labelBuff);

	for (i = 0; i < AqualinkButtonCount; i++)
	{
		if (strcasecmp(stripwhitespace(labelBuff), _aqualink_data->aqbuttons[i].pda_label) == 0)
		{
			DEBUG("*** Found EQ CTL Status for %s = '%.*s'", _aqualink_data->aqbuttons[i].pda_label, AQ_MSGLEN, msg);
			set_pda_led(_aqualink_data->aqbuttons[i].led, msg[AQ_MSGLEN - 1]);
		}
	}

	// Force SWG off if pump is off.
	if (_aqualink_data->aqbuttons[0].led->state == OFF)
	{
		_aqualink_data->ar_swg_status = SWG_STATUS_OFF;
	}

	// NSF I think we need to check TEMP1 and TEMP2 and set Pool HEater and Spa heater directly, to support single device.
	if (_aqualink_data->single_device) {
		if (strcasecmp(stripwhitespace(labelBuff), "TEMP1") == 0)
		{
			set_pda_led(_aqualink_data->aqbuttons[PoolHeater].led, msg[AQ_MSGLEN - 1]);
		}
		if (strcasecmp(stripwhitespace(labelBuff), "TEMP2") == 0)
		{
			set_pda_led(_aqualink_data->aqbuttons[SpaHeater].led, msg[AQ_MSGLEN - 1]);
		}
	}
}

void process_pda_packet_msg_long_home(const char* msg)
{
	if (stristr(msg, "POOL MODE") != NULL)
	{
		// If pool mode is on the filter pump is on but if it is off the filter pump might be on if spa mode is on.
		if (msg[AQ_MSGLEN - 1] == 'N')
		{
			_aqualink_data->aqbuttons[FilterPump].led->state = ON;
		}
		else if (msg[AQ_MSGLEN - 1] == '*')
		{
			_aqualink_data->aqbuttons[FilterPump].led->state = FLASH;
		}
	}
	else if (stristr(msg, "POOL HEATER") != NULL)
	{
		set_pda_led(_aqualink_data->aqbuttons[PoolHeater].led, msg[AQ_MSGLEN - 1]);
	}
	else if (stristr(msg, "SPA MODE") != NULL)
	{
		// when SPA mode is on the filter may be on or pending
		if (msg[AQ_MSGLEN - 1] == 'N')
		{
			_aqualink_data->aqbuttons[FilterPump].led->state = ON;
			_aqualink_data->aqbuttons[SpaMode].led->state = ON;
		}
		else if (msg[AQ_MSGLEN - 1] == '*')
		{
			_aqualink_data->aqbuttons[FilterPump].led->state = FLASH;
			_aqualink_data->aqbuttons[SpaMode].led->state = ON;
		}
		else
		{
			_aqualink_data->aqbuttons[SpaMode].led->state = OFF;
		}
	}
	else if (stristr(msg, "SPA HEATER") != NULL)
	{
		set_pda_led(_aqualink_data->aqbuttons[SpaHeater].led, msg[AQ_MSGLEN - 1]);
	}
}

void setSingleDeviceMode()
{
	if (_aqualink_data->single_device != true)
	{
		_aqualink_data->single_device = true;
		NOTICE("AqualinkD set to 'Pool OR Spa Only' mode");
	}
}

void process_pda_packet_msg_long_set_temp(const char* msg)
{
	DEBUG("process_pda_packet_msg_long_set_temp");

	if (stristr(msg, "POOL HEAT") != NULL)
	{
		_aqualink_data->pool_htr_set_point = atoi(msg + 10);
		DEBUG("pool_htr_set_point = %d", _aqualink_data->pool_htr_set_point);
	}
	else if (stristr(msg, "SPA HEAT") != NULL)
	{
		_aqualink_data->spa_htr_set_point = atoi(msg + 10);
		DEBUG("spa_htr_set_point = %d", _aqualink_data->spa_htr_set_point);
	}
	else if (stristr(msg, "TEMP1") != NULL)
	{
		setSingleDeviceMode();
		_aqualink_data->pool_htr_set_point = atoi(msg + 10);
		DEBUG("pool_htr_set_point = %d", _aqualink_data->pool_htr_set_point);
	}
	else if (stristr(msg, "TEMP2") != NULL)
	{
		setSingleDeviceMode();
		_aqualink_data->spa_htr_set_point = atoi(msg + 10);
		DEBUG("spa_htr_set_point = %d", _aqualink_data->spa_htr_set_point);
	}


}

void process_pda_packet_msg_long_spa_heat(const char* msg)
{
	if (strncasecmp(msg, "    ENABLED     ", 16) == 0)
	{
		_aqualink_data->aqbuttons[SpaHeater].led->state = ENABLE;
	}
	else if (strncasecmp(msg, "  SET TO", 8) == 0)
	{
		_aqualink_data->spa_htr_set_point = atoi(msg + 8);
		DEBUG("spa_htr_set_point = %d", _aqualink_data->spa_htr_set_point);
	}
}

void process_pda_packet_msg_long_pool_heat(const char* msg)
{
	if (strncasecmp(msg, "    ENABLED     ", 16) == 0)
	{
		_aqualink_data->aqbuttons[PoolHeater].led->state = ENABLE;
	}
	else if (strncasecmp(msg, "  SET TO", 8) == 0)
	{
		_aqualink_data->pool_htr_set_point = atoi(msg + 8);
		DEBUG("pool_htr_set_point = %d", _aqualink_data->pool_htr_set_point);
	}
}

void process_pda_packet_msg_long_freeze_protect(const char* msg)
{
	if (strncasecmp(msg, "TEMP      ", 10) == 0)
	{
		_aqualink_data->frz_protect_set_point = atoi(msg + 10);
		DEBUG("frz_protect_set_point = %d", _aqualink_data->frz_protect_set_point);
	}
}

void process_pda_packet_msg_long_SWG(const char* msg)
{
	//PDA Line 0 =   SET AquaPure
	//PDA Line 1 =
	//PDA Line 2 =
	//PDA Line 3 = SET POOL TO: 45%
	//PDA Line 4 =  SET SPA TO:  0%

	// If spa is on, read SWG for spa, if not set SWG for pool
	if (_aqualink_data->aqbuttons[SpaMode].led->state != OFF) {
		if (strncasecmp(msg, "SET SPA TO:", 11) == 0)
		{
			_aqualink_data->swg_percent = atoi(msg + 13);
			DEBUG("SPA swg_percent = %d", _aqualink_data->swg_percent);
		}
	}
	else {
		if (strncasecmp(msg, "SET POOL TO:", 12) == 0)
		{
			_aqualink_data->swg_percent = atoi(msg + 13);
			DEBUG("POOL swg_percent = %d", _aqualink_data->swg_percent);
		}
	}
}

void process_pda_packet_msg_long_unknown(const char* msg)
{
	int i;
	// Lets make a guess here and just see if there is an ON/OFF/ENA/*** at the end of the line
	// When you turn on/off a piece of equiptment, a clear screen followed by single message is sent.
	// So we are not in any PDA menu, try to catch that message here so we catch new device state ASAP.
	if (msg[AQ_MSGLEN - 1] == 'N' || msg[AQ_MSGLEN - 1] == 'F' || msg[AQ_MSGLEN - 1] == 'A' || msg[AQ_MSGLEN - 1] == '*')
	{
		for (i = 0; i < AqualinkButtonCount; i++)
		{
			if (stristr(msg, _aqualink_data->aqbuttons[i].pda_label) != NULL)
			{
				printf("*** UNKNOWN Found Status for %s = '%.*s'", _aqualink_data->aqbuttons[i].pda_label, AQ_MSGLEN, msg);
				// set_pda_led(_aqualink_data->aqbuttons[i].led, msg[AQ_MSGLEN-1]);
			}
		}
	}
}

void process_pda_packet_msg_long_level_aux_device(const char* msg)
{
#ifdef BETA_PDA_AUTOLABEL
	int li = -1;
	char* str, * label;

	if (!_aqualink_config->use_panel_aux_labels)
		return;
	// NSF  Need to check config for use_panel_aux_labels value and ignore if not set

	// Only care once we have the full menu, so check line 
	//  PDA Line 0 =    LABEL AUX1   
	//  PDA Line 1 =     
	//  PDA Line 2 =   CURRENT LABEL 
	//  PDA Line 3 =       AUX1          


	if ((strlen(pda_m_line(3)) > 0) &&
		(strncasecmp(pda_m_line(2), "  CURRENT LABEL ", 16) == 0) &&
		(strncasecmp(pda_m_line(0), "   LABEL AUX", 12) == 0)) {
		str = pda_m_line(0);
		li = atoi(&str[12]);  // 12 is a guess, need to check on real system
		if (li > 0) {
			str = cleanwhitespace(pda_m_line(3));
			label = (char*)malloc(strlen(str) + 1);
			strcpy(label, str);
			_aqualink_data->aqbuttons[li - 1].pda_label = label;
		}
		else {
			ERROR("PDA couldn't get AUX? number\n", pda_m_line(0));
		}
	}
#endif
}

void process_pda_freeze_protect_devices()
{
	//  PDA Line 0 =  FREEZE PROTECT
	//  PDA Line 1 =     DEVICES
	//  PDA Line 2 =
	//  PDA Line 3 = FILTER PUMP    X
	//  PDA Line 4 = SPA
	//  PDA Line 5 = CLEANER        X
	//  PDA Line 6 = POOL LIGHT
	//  PDA Line 7 = SPA LIGHT
	//  PDA Line 8 = EXTRA AUX
	//  PDA Line 9 =
	int i;
	DEBUG("process_pda_freeze_protect_devices");
	for (i = 1; i < PDA_LINES; i++)
	{
		if (pda_m_line(i)[AQ_MSGLEN - 1] == 'X')
		{
			DEBUG("PDA freeze protect enabled by %s", pda_m_line(i));
			if (_aqualink_data->frz_protect_state == OFF)
			{
				_aqualink_data->frz_protect_state = ENABLE;
				break;
			}
		}
	}
}

bool process_pda_packet(unsigned char* packet, int length)
{
	bool rtn = true;
	int i;
	char* msg;
	//static bool init = false;

	process_pda_menu_packet(packet, length);

	// NSF.

	//_aqualink_data->last_msg_was_status = false;

	// debugPacketPrint(0x00, packet, length);

	switch (packet[PKT_CMD])
	{

	case CMD_ACK:
		DEBUG("RS Received ACK length %d.", length);
		//if (init == false)
		if (_initWithRS == false)
		{
			DEBUG("Running PDA_INIT");
			aq_programmer(AQ_PDA_INIT, NULL, _aqualink_data);
			//init = true;
		}
		break;

	case CMD_STATUS:
		_aqualink_data->last_display_message[0] = '\0';

		// If we get a status packet, and we are on the status menu, this is a list of what's on
		// or pending so unless flash turn everything off, and just turn on items that are listed.
		// This is the only way to update a device that's been turned off by a real PDA / keypad.
		// Note: if the last line of the status menu is present it may be cut off
		if (pda_m_type() == PM_EQUIPTMENT_STATUS)
		{
			if (_aqualink_data->frz_protect_state == ON)
			{
				_aqualink_data->frz_protect_state = ENABLE;
			}

			if (_aqualink_data->ar_swg_status == SWG_STATUS_ON)
			{
				_aqualink_data->ar_swg_status = SWG_STATUS_OFF;
			}

			if (pda_m_line(PDA_LINES - 1)[0] == '\0')
			{
				for (i = 0; i < AqualinkButtonCount; i++)
				{
					if (_aqualink_data->aqbuttons[i].led->state != FLASH)
					{
						_aqualink_data->aqbuttons[i].led->state = OFF;
					}
				}
			}
			else
			{
				DEBUG("PDA Equipment status may be truncated.");
			}
			for (i = 1; i < PDA_LINES; i++)
			{
				pass_pda_equiptment_status_item(pda_m_line(i));
			}

		}
		if (pda_m_type() == PM_FREEZE_PROTECT_DEVICES)
		{
			process_pda_freeze_protect_devices();
		}
		break;
	case CMD_MSG_LONG:
	{
		msg = (char*)packet + PKT_DATA + 1;

		if (packet[PKT_DATA] == 0x82)
		{ // Air & Water temp is always this ID
			process_pda_packet_msg_long_temp(msg);
		}
		else if (packet[PKT_DATA] == 0x40)
		{ // Time is always on this ID
			process_pda_packet_msg_long_time(msg);
			// If it wasn't a specific msg, (above) then run through and see what kind
			// of message it is depending on the PDA menu.  Note don't process EQUIPTMENT
			// STATUS menu here, wait until a CMD_STATUS is received.
		}
		else {
			switch (pda_m_type()) {
			case PM_EQUIPTMENT_CONTROL:
				process_pda_packet_msg_long_equipment_control(msg);
				break;
			case PM_HOME:
			case PM_BUILDING_HOME:
				process_pda_packet_msg_long_home(msg);
				break;
			case PM_SET_TEMP:
				process_pda_packet_msg_long_set_temp(msg);
				break;
			case PM_SPA_HEAT:
				process_pda_packet_msg_long_spa_heat(msg);
				break;
			case PM_POOL_HEAT:
				process_pda_packet_msg_long_pool_heat(msg);
				break;
			case PM_FREEZE_PROTECT:
				process_pda_packet_msg_long_freeze_protect(msg);
				break;
			case PM_AQUAPURE:
				process_pda_packet_msg_long_SWG(msg);
				break;
			case PM_AUX_LABEL_DEVICE:
				process_pda_packet_msg_long_level_aux_device(msg);
				break;
				//case PM_FW_VERSION:
				//  process_pda_packet_msg_long_FW_version(msg);
				//break;
			case PM_UNKNOWN:
			default:
				process_pda_packet_msg_long_unknown(msg);
				break;
			}
		}

		INFO("PDA Menu '%d' Selectedline '%s', Last line received '%.*s'", pda_m_type(), pda_m_hlight(), AQ_MSGLEN, msg);
		break;
	}
	case CMD_PDA_0x1B:
	{
		// We get two of these on startup, one with 0x00 another with 0x01 at index 4.  Just act on one.
		// Think this is PDA finishd showing startup screen
		if (packet[4] == 0x00) {
			if (_initWithRS == false)
			{
				_initWithRS = true;
				DEBUG("**** PDA INIT ****");
				aq_programmer(AQ_PDA_INIT, NULL, _aqualink_data);
				delayMicroseconds(50);  // Make sure this one runs first.
#ifdef BETA_PDA_AUTOLABEL
				if (_aqualink_config->use_panel_aux_labels)
					aq_programmer(AQ_GET_AUX_LABELS, NULL, _aqualink_data);
#endif
				aq_programmer(AQ_PDA_WAKE_INIT, NULL, _aqualink_data);
			}
			else {
				DEBUG("**** PDA WAKE INIT ****");
				aq_programmer(AQ_PDA_WAKE_INIT, NULL, _aqualink_data);
			}
		}
	}
	break;
}

	if (packet[PKT_CMD] == CMD_MSG_LONG || packet[PKT_CMD] == CMD_PDA_HIGHLIGHT ||
		packet[PKT_CMD] == CMD_PDA_SHIFTLINES || packet[PKT_CMD] == CMD_PDA_CLEAR ||
		packet[PKT_CMD] == CMD_PDA_HIGHLIGHTCHARS)
	{
		// We processed the next message, kick any threads waiting on the message.
		kick_aq_program_thread(_aqualink_data);
	}
	return rtn;
}
