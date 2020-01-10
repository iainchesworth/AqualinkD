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
#include "aqualink.h"

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "config/config.h"
#include "config/config_helpers.h"
#include "cross-platform/daemon.h"
#include "cross-platform/time.h"
#include "logging/logging.h"
#include "logging/logging_sink_basic_file.h"
#include "logging/logging_utils.h"
#include "hardware/buttons/rs_buttons.h"
#include "hardware/aqualink_master_controller.h"
#include "hardware/controllers/rs_controller.h"
#include "serial/aq_serial.h"
#include "string/string_utils.h"
#include "version/version.h"
#include "web/aq_web.h"

#include "aquapure.h"
#include "aq_programmer.h"
#include "net_services.h"
#include "options.h"
#include "pda.h"
#include "pda_aq_programmer.h"
#include "pda_menu.h"
#include "pentair_messages.h"
#include "utils.h"

static volatile bool _keepRunning = true;
static struct aqualinkdata _aqualink_data;

void intHandler(int dummy)
{
	UNREFERENCED_PARAMETER(dummy);

	_keepRunning = false;
	NOTICE("Stopping!");
}

void processLEDstate()
{
	int i = 0;
	int byte;
	int bit;

	for (byte = 0; byte < 5; byte++)
	{
		for (bit = 0; bit < 8; bit += 2)
		{
			if (((_aqualink_data.raw_status[byte] >> (bit + 1)) & 1) == 1)
			{
				_aqualink_data.aqualinkleds[i].state = FLASH;
			}
			else if (((_aqualink_data.raw_status[byte] >> bit) & 1) == 1)
			{
				_aqualink_data.aqualinkleds[i].state = ON;
			}
			else
			{
				_aqualink_data.aqualinkleds[i].state = OFF;
			}

			++i;
		}
	}

	// Reset enabled state for heaters, as they take 2 led states
	if (_aqualink_data.aqualinkleds[POOL_HTR_LED_INDEX - 1].state == OFF && _aqualink_data.aqualinkleds[POOL_HTR_LED_INDEX].state == ON)
	{
		_aqualink_data.aqualinkleds[POOL_HTR_LED_INDEX - 1].state = ENABLE;
	}

	if (_aqualink_data.aqualinkleds[SPA_HTR_LED_INDEX - 1].state == OFF && _aqualink_data.aqualinkleds[SPA_HTR_LED_INDEX].state == ON)
	{
		_aqualink_data.aqualinkleds[SPA_HTR_LED_INDEX - 1].state = ENABLE;
	}

	if (_aqualink_data.aqualinkleds[SOLAR_HTR_LED_INDEX - 1].state == OFF && _aqualink_data.aqualinkleds[SOLAR_HTR_LED_INDEX].state == ON)
	{
		_aqualink_data.aqualinkleds[SOLAR_HTR_LED_INDEX - 1].state = ENABLE;
	}
}

bool checkAqualinkTime()
{
	static time_t last_checked;
	time_t now = time(0); // get time now
	int time_difference;
	struct tm aq_tm;
	time_t aqualink_time;

	time_difference = (int)difftime(now, last_checked);
	if (time_difference < TIME_CHECK_INTERVAL)
	{
		DEBUG("time not checked, will check in %d seconds", TIME_CHECK_INTERVAL - time_difference);
		return true;
	}

	last_checked = now;

	char datestr[DATE_STRING_LEN];
	strcpy(&datestr[0], _aqualink_data.date);
	strcpy(&datestr[12], " ");
	strcpy(&datestr[13], _aqualink_data.time);

	if (0 == strptime(datestr, "%m/%d/%y %a %I:%M %p", &aq_tm))
	{
		ERROR("Could not convert RS time string '%s'", datestr);
		last_checked = (time_t)NULL;
		return true;
	}

	aq_tm.tm_isdst = -1; // Force mktime to use local timezone
	aqualink_time = mktime(&aq_tm);
	time_difference = (int)difftime(now, aqualink_time);

	INFO("Aqualink time is off by %d seconds...", time_difference);

	if (abs(time_difference) <= ACCEPTABLE_TIME_DIFF)
	{
		// Time difference is less than or equal to 90 seconds (1 1/2 minutes).
		// Set the return value to true.
		return true;
	}

	return false;
}

void queueGetProgramData()
{
	// Init string good time to get setpoints
	aq_send_cmd(NUL);
	aq_programmer(AQ_GET_POOL_SPA_HEATER_TEMPS, NULL, &_aqualink_data);
	aq_programmer(AQ_GET_FREEZE_PROTECT_TEMP, NULL, &_aqualink_data);

	if (CFG_UsePanelAuxLabels() == true)
	{
		aq_programmer(AQ_GET_AUX_LABELS, NULL, &_aqualink_data);
	}
}

void setUnits(char* msg)
{
	DEBUG("Getting temp units from message %s, looking at %c", msg, msg[strlen(msg) - 1]);

	if (msg[strlen(msg) - 1] == 'F') {
		_aqualink_data.temp_units = FAHRENHEIT;
	}
	else if (msg[strlen(msg) - 1] == 'C') {
		_aqualink_data.temp_units = CELSIUS;
	}
	else {
		_aqualink_data.temp_units = UNKNOWN;
	}

	INFO("Temp Units set to %d (F=0, C=1, Unknown=3)", _aqualink_data.temp_units);
}

void processMessage(char* message)
{
	char* msg;
	static bool _initWithRS = false;
	static bool _gotREV = false;
	static int freeze_msg_count = 0;
	static int service_msg_count = 0;
	static int swg_msg_count = 0;
	static int boost_msg_count = 0;
	// NSF replace message with msg
	msg = stripwhitespace(message);
	strcpy(_aqualink_data.last_message, msg);

	INFO("RS Message :- '%s'", msg);

	// Check long messages in this if/elseif block first, as some messages are similar.
	// ie "POOL TEMP" and "POOL TEMP IS SET TO"  so want correct match first.
	//

	if (aq_stristr(msg, "JANDY AquaLinkRS") != NULL)
	{
		_aqualink_data.last_display_message[0] = '\0';
	}

	// Don't do any message counts if we are programming
	if (_aqualink_data.active_thread.thread_id == 0) 
	{
		// If we have more than 10 messages without "Service Mode is active" assume it's off.
		if (_aqualink_data.service_mode_state != OFF && service_msg_count++ > 10)
		{
			_aqualink_data.service_mode_state = OFF;
			service_msg_count = 0;
		}

		// If we have more than 40 messages without "SALT or AQUAPURE" assume SWG is off.
		if (CFG_ReadAllDevices() == false)
		{
			if (_aqualink_data.ar_swg_status == SWG_STATUS_ON && swg_msg_count++ > 40)
			{
				_aqualink_data.ar_swg_status = SWG_STATUS_OFF;
				swg_msg_count = 0;
			}
		}

		// If we have more than 10 messages without "FREE PROTECT ACTIVATED" assume it's off.
		if (_aqualink_data.frz_protect_state == ON && freeze_msg_count++ > 10)
		{
			_aqualink_data.frz_protect_state = ENABLE;
			freeze_msg_count = 0;
		}

		if (_aqualink_data.boost == true && boost_msg_count++ > 10)
		{
			_aqualink_data.boost = false;
			_aqualink_data.boost_msg[0] = '\0';
			boost_msg_count = 0;
		}
	}

	if (aq_stristr(msg, LNG_MSG_BATTERY_LOW) != NULL)
	{
		_aqualink_data.battery = LOW;
		strcpy(_aqualink_data.last_display_message, msg); // Also display the message on web UI
	}
	else if (aq_stristr(msg, LNG_MSG_POOL_TEMP_SET) != NULL)
	{
		_aqualink_data.pool_htr_set_point = atoi(message + 20);

		if (_aqualink_data.temp_units == UNKNOWN) {
			setUnits(msg);
		}
	}
	else if (aq_stristr(msg, LNG_MSG_SPA_TEMP_SET) != NULL)
	{
		_aqualink_data.spa_htr_set_point = atoi(message + 19);

		if (_aqualink_data.temp_units == UNKNOWN) {
			setUnits(msg);
		}
	}
	else if (aq_stristr(msg, LNG_MSG_FREEZE_PROTECTION_SET) != NULL)
	{
		_aqualink_data.frz_protect_set_point = atoi(message + 28);
		_aqualink_data.frz_protect_state = ENABLE;

		if (_aqualink_data.temp_units == UNKNOWN) {
			setUnits(msg);
		}
	}
	else if (aq_strnicmp(msg, MSG_AIR_TEMP, MSG_AIR_TEMP_LEN) == 0)
	{
		_aqualink_data.air_temp = atoi(msg + MSG_AIR_TEMP_LEN);

		if (_aqualink_data.temp_units == UNKNOWN) {
			setUnits(msg);
		}
	}
	else if (aq_strnicmp(msg, MSG_POOL_TEMP, MSG_POOL_TEMP_LEN) == 0)
	{
		_aqualink_data.pool_temp = atoi(msg + MSG_POOL_TEMP_LEN);

		if (_aqualink_data.temp_units == UNKNOWN) {
			setUnits(msg);
		}
	}
	else if (aq_strnicmp(msg, MSG_SPA_TEMP, MSG_SPA_TEMP_LEN) == 0)
	{
		_aqualink_data.spa_temp = atoi(msg + MSG_SPA_TEMP_LEN);

		if (_aqualink_data.temp_units == UNKNOWN) {
			setUnits(msg);
		}
	}
	// NSF If get water temp rather than pool or spa in some cases, then we are in Pool OR Spa ONLY mode
	else if (aq_strnicmp(msg, MSG_WATER_TEMP, MSG_WATER_TEMP_LEN) == 0)
	{
		_aqualink_data.pool_temp = atoi(msg + MSG_WATER_TEMP_LEN);
		_aqualink_data.spa_temp = atoi(msg + MSG_WATER_TEMP_LEN);
		if (_aqualink_data.temp_units == UNKNOWN) {
			setUnits(msg);
		}

		if (_aqualink_data.single_device != true)
		{
			_aqualink_data.single_device = true;
			NOTICE("AqualinkD set to 'Pool OR Spa Only' mode");
		}
	}
	else if (aq_stristr(msg, LNG_MSG_WATER_TEMP1_SET) != NULL)
	{
		_aqualink_data.pool_htr_set_point = atoi(message + 28);

		if (_aqualink_data.temp_units == UNKNOWN) {
			setUnits(msg);
		}

		if (_aqualink_data.single_device != true)
		{
			_aqualink_data.single_device = true;
			NOTICE("AqualinkD set to 'Pool OR Spa Only' mode");
		}
	}
	else if (aq_stristr(msg, LNG_MSG_WATER_TEMP2_SET) != NULL)
	{
		_aqualink_data.spa_htr_set_point = atoi(message + 27);

		if (_aqualink_data.temp_units == UNKNOWN) {
			setUnits(msg);
		}

		if (_aqualink_data.single_device != true)
		{
			_aqualink_data.single_device = true;
			NOTICE("AqualinkD set to 'Pool OR Spa Only' mode");
		}
	}
	else if (aq_stristr(msg, LNG_MSG_SERVICE_ACTIVE) != NULL)
	{
		if (_aqualink_data.service_mode_state == OFF) {
			NOTICE("AqualinkD set to Service Mode");
		}
		_aqualink_data.service_mode_state = ON;
		service_msg_count = 0;
	}
	else if (aq_stristr(msg, LNG_MSG_TIMEOUT_ACTIVE) != NULL)
	{
		if (_aqualink_data.service_mode_state == OFF) {
			NOTICE("AqualinkD set to Timeout Mode");
		}
		_aqualink_data.service_mode_state = FLASH;
		service_msg_count = 0;
	}
	else if (aq_stristr(msg, LNG_MSG_FREEZE_PROTECTION_ACTIVATED) != NULL)
	{
		_aqualink_data.frz_protect_state = ON;
		freeze_msg_count = 0;
		strcpy(_aqualink_data.last_display_message, msg); // Also display the message on web UI
	}
	else if (msg[2] == '/' && msg[5] == '/' && msg[8] == ' ')
	{
		// date in format '08/29/16 MON'
		strcpy(_aqualink_data.date, msg);
	}
	else if (aq_strnicmp(msg, MSG_SWG_PCT, MSG_SWG_PCT_LEN) == 0)
	{
		_aqualink_data.swg_percent = atoi(msg + MSG_SWG_PCT_LEN);
		if (_aqualink_data.ar_swg_status == SWG_STATUS_OFF)
		{
			_aqualink_data.ar_swg_status = SWG_STATUS_ON;
		}

		swg_msg_count = 0;
	}
	else if (aq_strnicmp(msg, MSG_SWG_PPM, MSG_SWG_PPM_LEN) == 0)
	{
		_aqualink_data.swg_ppm = atoi(msg + MSG_SWG_PPM_LEN);
		if (_aqualink_data.ar_swg_status == SWG_STATUS_OFF)
		{
			_aqualink_data.ar_swg_status = SWG_STATUS_ON;
		}

		swg_msg_count = 0;
	}
	else if ((msg[1] == ':' || msg[2] == ':') && msg[strlen(msg) - 1] == 'M')
	{
		// time in format '9:45 AM'
		strcpy(_aqualink_data.time, msg);

		// Setting time takes a long time, so don't try until we have all other programmed data.
		if ((_initWithRS == true) && strlen(_aqualink_data.date) > 1 && checkAqualinkTime() != true)
		{
			NOTICE("RS time is NOT accurate '%s %s', re-setting on controller!", _aqualink_data.time, _aqualink_data.date);
			aq_programmer(AQ_SET_TIME, NULL, &_aqualink_data);
		}
		else
		{
			DEBUG("RS time is accurate '%s %s'", _aqualink_data.time, _aqualink_data.date);
		}
		// If we get a time message before REV, the controller didn't see us as we started too quickly.
		if (_gotREV == false)
		{
			NOTICE("Getting control panel information", msg);
			aq_programmer(AQ_GET_DIAGNOSTICS_MODEL, NULL, &_aqualink_data);
			_gotREV = true; // Force it to true just incase we don't understand the model#
		}
	}
	else if (strstr(msg, " REV ") != NULL)
	{
		// '8157 REV MMM'
		// A master firmware revision message.
		strcpy(_aqualink_data.version, msg);
		_gotREV = true;
		NOTICE("Control Panel %s", msg);
		if (_initWithRS == false)
		{
			queueGetProgramData();
			_initWithRS = true;
		}
	}
	else if (aq_stristr(msg, " TURNS ON") != NULL)
	{
		NOTICE("Program data '%s'", msg);
	}
	else if (CFG_OverrideFreezeProtect() == TRUE && aq_strnicmp(msg, "Press Enter* to override Freeze Protection with", 47) == 0)
	{
		aq_send_cmd(KEY_ENTER);
	}
	else if ((msg[4] == ':') && (aq_strnicmp(msg, "AUX", 3) == 0))
	{ // AUX label "AUX1:"
		int labelid = atoi(msg + 3);
		if (labelid > 0 && CFG_UsePanelAuxLabels() == true)
		{
			// Aux1: on panel = Button 3 in aqualinkd  (button 2 in array)
			NOTICE("AUX LABEL %d '%s'", labelid + 1, msg);
			_aqualink_data.aqbuttons[labelid + 1].label = prettyString(cleanalloc(msg + 5));
		}
	}
	// BOOST POOL 23:59 REMAINING
	else if ((aq_strnicmp(msg, "BOOST POOL", 10) == 0) && (strstr(msg, "REMAINING") != NULL)) {
		// Ignore messages if in programming mode.  We get one of these turning off for some strange reason.
		if (_aqualink_data.active_thread.thread_id == 0) {
			memcpy(_aqualink_data.boost_msg, &msg[11], 6);
			_aqualink_data.boost = true;
			boost_msg_count = 0;
			strcpy(_aqualink_data.last_display_message, msg); // Also display the message on web UI if not in programming mode
		}
	}
	else
	{
		TRACE("Ignoring '%s'", msg);

		if (_aqualink_data.active_thread.thread_id == 0 &&
			aq_stristr(msg, "JANDY AquaLinkRS") == NULL &&
			aq_stristr(msg, "PUMP O") == NULL &&// Catch 'PUMP ON' and 'PUMP OFF' but not 'PUMP WILL TURN ON'
			aq_stristr(msg, "MAINTAIN") == NULL /* && // Catch 'MAINTAIN TEMP IS OFF'
			aq_stristr(msg, "CLEANER O") == NULL &&
			aq_stristr(msg, "SPA O") == NULL &&
			aq_stristr(msg, "AUX") == NULL*/
			)
		{ // Catch all AUX1 AUX5 messages
			strcpy(_aqualink_data.last_display_message, msg);
		}
	}

	// Send every message if we are in simulate panel mode
	if (_aqualink_data.simulate_panel)
	{
		ascii(_aqualink_data.last_display_message, msg);
	}

	// We processed the next message, kick any threads waiting on the message.
	kick_aq_program_thread(&_aqualink_data);
}

bool process_packet(unsigned char* packet, int length)
{
	bool rtn = false;
	static unsigned char last_packet[AQ_MAXPKTLEN];
	static char message[AQ_MSGLONGLEN + 1];
	static int processing_long_msg = 0;

	// Check packet against last check if different.
	if (memcmp(packet, last_packet, length) == 0)
	{
		TRACE("RS Received duplicate, ignoring.", length);
		return rtn;
	}

	memcpy(last_packet, packet, length);
	_aqualink_data.last_packet_type = packet[PKT_CMD];
	rtn = true;

	if (CFG_PdaMode() == true)
	{
		return process_pda_packet(packet, length);
	}

	// If we are in the middle of processing a message, and get another, we end the message
	if (processing_long_msg > 0 && packet[PKT_CMD] != CMD_MSG_LONG)
	{
		processing_long_msg = 0;
		processMessage(message);
	}

	switch (packet[PKT_CMD])
	{
	case CMD_ACK:
		break;

	case CMD_STATUS:
		memcpy(_aqualink_data.raw_status, packet + 4, AQ_PSTLEN);
		processLEDstate();

		if (_aqualink_data.aqbuttons[FilterPump].led->state == OFF)
		{
			_aqualink_data.pool_temp = TEMP_UNKNOWN;
			_aqualink_data.spa_temp = TEMP_UNKNOWN;
		}
		else if (_aqualink_data.aqbuttons[SpaMode].led->state == OFF && _aqualink_data.single_device != true)
		{
			_aqualink_data.spa_temp = TEMP_UNKNOWN;
		}
		else if (_aqualink_data.aqbuttons[SpaMode].led->state == ON && _aqualink_data.single_device != true)
		{
			_aqualink_data.pool_temp = TEMP_UNKNOWN;
		}

		// COLOR MODE programming relies on state changes, so let any threads know
		if (_aqualink_data.active_thread.ptype == AQ_SET_COLORMODE) {
			kick_aq_program_thread(&_aqualink_data);
		}
		break;

	case CMD_MSG:
	case CMD_MSG_LONG:
	{
		int index = packet[PKT_DATA]; // Will get 0x00 for complete message, 0x01 for start on long message 0x05 last of long message
		if (index <= 1) {
			memset(message, 0, AQ_MSGLONGLEN + 1);
			strncpy(message, (char*)packet + PKT_DATA + 1, AQ_MSGLEN);
			processing_long_msg = index;
		}
		else
		{
			strncpy(&message[(processing_long_msg * AQ_MSGLEN)], (char*)packet + PKT_DATA + 1, AQ_MSGLEN);
			if (++processing_long_msg != index) {
				ERROR("Long message index %d doesn't match buffer %d", index, processing_long_msg);
			}
#ifdef  PROCESS_INCOMPLETE_MESSAGES
			kick_aq_program_thread(&_aqualink_data);
#endif
		}

		if (index == 0 || index == 5) {
			processMessage(message); // This will kick thread
		}

	}
	break;

	case CMD_PROBE:
		DEBUG("RS Received PROBE length %d.", length);
		rtn = false;
		break;

	default:
		INFO("RS Received unknown packet, 0x%02hhx", packet[PKT_CMD]);
		rtn = false;
		break;
	}

	return rtn;
}

void action_delayed_request()
{
	char sval[10];
	snprintf(sval, 9, "%d", _aqualink_data.unactioned.value);

	// If we don't know the units yet, we can't action, so wait until we do.
	if (_aqualink_data.temp_units == UNKNOWN && _aqualink_data.unactioned.type != SWG_SETPOINT)
	{
		return;
	}

	if (_aqualink_data.unactioned.type == POOL_HTR_SETOINT)
	{
		_aqualink_data.unactioned.value = setpoint_check(POOL_HTR_SETOINT, _aqualink_data.unactioned.value, &_aqualink_data);
		if (_aqualink_data.pool_htr_set_point != _aqualink_data.unactioned.value)
		{
			aq_programmer(AQ_SET_POOL_HEATER_TEMP, sval, &_aqualink_data);
			NOTICE("Setting pool heater setpoint to %d", _aqualink_data.unactioned.value);
		}
		else
		{
			NOTICE("Pool heater setpoint is already %d, not changing", _aqualink_data.unactioned.value);
		}
	}
	else if (_aqualink_data.unactioned.type == SPA_HTR_SETOINT)
	{
		_aqualink_data.unactioned.value = setpoint_check(SPA_HTR_SETOINT, _aqualink_data.unactioned.value, &_aqualink_data);
		if (_aqualink_data.spa_htr_set_point != _aqualink_data.unactioned.value)
		{
			aq_programmer(AQ_SET_SPA_HEATER_TEMP, sval, &_aqualink_data);
			NOTICE("Setting spa heater setpoint to %d", _aqualink_data.unactioned.value);
		}
		else
		{
			NOTICE("Spa heater setpoint is already %d, not changing", _aqualink_data.unactioned.value);
		}
	}
	else if (_aqualink_data.unactioned.type == FREEZE_SETPOINT)
	{
		_aqualink_data.unactioned.value = setpoint_check(FREEZE_SETPOINT, _aqualink_data.unactioned.value, &_aqualink_data);
		if (_aqualink_data.frz_protect_set_point != _aqualink_data.unactioned.value)
		{
			aq_programmer(AQ_SET_FRZ_PROTECTION_TEMP, sval, &_aqualink_data);
			NOTICE("Setting freeze protect to %d", _aqualink_data.unactioned.value);
		}
		else
		{
			NOTICE("Freeze setpoint is already %d, not changing", _aqualink_data.unactioned.value);
		}
	}
	else if (_aqualink_data.unactioned.type == SWG_SETPOINT)
	{
		_aqualink_data.unactioned.value = setpoint_check(SWG_SETPOINT, _aqualink_data.unactioned.value, &_aqualink_data);
		if (_aqualink_data.ar_swg_status == SWG_STATUS_OFF)
		{
			// SWG is off, can't set %, so delay the set until it's on.
			_aqualink_data.swg_delayed_percent = _aqualink_data.unactioned.value;
		}
		else
		{
			if (_aqualink_data.swg_percent != _aqualink_data.unactioned.value)
			{
				aq_programmer(AQ_SET_SWG_PERCENT, sval, &_aqualink_data);
				NOTICE("Setting SWG %% to %d", _aqualink_data.unactioned.value);
			}
			else
			{
				NOTICE("SWG % is already %d, not changing", _aqualink_data.unactioned.value);
			}
		}
		// Let's just tell everyone we set it, before we actually did.  Makes homekit happy, and it will re-correct on error.
		_aqualink_data.swg_percent = _aqualink_data.unactioned.value;
	}
	else if (_aqualink_data.unactioned.type == SWG_BOOST)
	{
		if (_aqualink_data.ar_swg_status == SWG_STATUS_OFF) {
			ERROR("SWG is off, can't Boost pool");
		}
		else if (_aqualink_data.unactioned.value == _aqualink_data.boost) {
			ERROR("Request to turn Boost %s ignored, Boost is already %s", _aqualink_data.unactioned.value ? "On" : "Off", _aqualink_data.boost ? "On" : "Off");
		}
		else {
			aq_programmer(AQ_SET_BOOST, sval, &_aqualink_data);
		}

		// Let's just tell everyone we set it, before we actually did.  Makes homekit happy, and it will re-correct on error.
		_aqualink_data.boost = _aqualink_data.unactioned.value;
	}

	_aqualink_data.unactioned.type = NO_ACTION;
	_aqualink_data.unactioned.value = -1;
	_aqualink_data.unactioned.requested = 0;
}

#define MAX_BLOCK_ACK 12
#define MAX_BUSY_ACK  (50 + MAX_BLOCK_ACK)

void caculate_ack_packet(SerialDevice rs_serialDevice, unsigned char* packet_buffer)
{
	static int delayAckCnt = 0;

	// if PDA mode, should we sleep? if not Can only send command to status message on PDA.
	if (CFG_PdaMode() == true) {
		if (CFG_PdaSleepMode() && pda_shouldSleep()) {
			DEBUG("PDA Aqualink daemon in sleep mode");
			return;
		}

		send_extended_ack(rs_serialDevice, ACK_PDA, pop_aq_cmd(&_aqualink_data));


	}
	else if (_aqualink_data.simulate_panel && _aqualink_data.active_thread.thread_id == 0) {
		// We are in simlator mode, ack get's complicated now.
		// If have a command to send, send a normal ack.
		// If we last message is waiting for an input "SELECT xxxxx", then sent a pause ack
		// pause ack strarts with around 12 ACK_SCREEN_BUSY_DISPLAY acks, then 50  ACK_SCREEN_BUSY acks
		// if we send a command (ie keypress), the whole count needs to end and go back to sending normal ack.
		// In code below, it jumps to sending ACK_SCREEN_BUSY, which still seems to work ok.
		if (aq_strnicmp(_aqualink_data.last_display_message, "SELECT", 6) != 0) 
		{ 
			// Nothing to wait for, send normal ack.
			send_ack(rs_serialDevice, pop_aq_cmd(&_aqualink_data));
			delayAckCnt = 0;
		}
		else if (get_aq_cmd_length() > 0) 
		{
			// Send command and jump directly "busy but can receive message"
			send_ack(rs_serialDevice, pop_aq_cmd(&_aqualink_data));
			delayAckCnt = MAX_BUSY_ACK; // need to test jumping to MAX_BUSY_ACK here
		}
		else 
		{
			NOTICE("Sending display busy due to Simulator mode ");

			if (delayAckCnt < MAX_BLOCK_ACK) 
			{
				// block all incomming messages
				send_extended_ack(rs_serialDevice, ACK_SCREEN_BUSY_BLOCK, pop_aq_cmd(&_aqualink_data));
			}
			else if (delayAckCnt < MAX_BUSY_ACK) 
			{
				// say we are pausing
				send_extended_ack(rs_serialDevice, ACK_SCREEN_BUSY, pop_aq_cmd(&_aqualink_data));
			}
			else 
			{
				// We timed out pause, send normal ack (This should also reset the display message on next message received)
				send_ack(rs_serialDevice, pop_aq_cmd(&_aqualink_data));
			}

			delayAckCnt++;
		}
	}
	else {
		// We are in simulate panel mode, but a thread is active, so ignore simulate panel
		send_ack(rs_serialDevice, pop_aq_cmd(&_aqualink_data));
	}
}

unsigned char find_unused_address(const unsigned char* packet)
{
	static int ID[4] = { 0,0,0,0 };  // 0=0x08, 1=0x09, 2=0x0A, 3=0x0B
	static unsigned char lastID = 0x00;

	if (packet[PKT_DEST] >= 0x08 && packet[PKT_DEST] <= 0x0B && packet[PKT_CMD] == CMD_PROBE) {
		lastID = packet[PKT_DEST];
	}
	else if (packet[PKT_DEST] == DEV_MASTER && lastID != 0x00) {
		lastID = 0x00;
	}
	else if (lastID != 0x00) {
		ID[lastID - 8]++;
		if (ID[lastID - 8] >= 3) {
			NOTICE("Found valid unused ID 0x%02hhx", lastID);
			return lastID;
		}
		lastID = 0x00;
	}
	else {
		lastID = 0x00;
	}

	return 0x00;
}

#include "cross-platform/signals.h"
#include "cross-platform/threads.h"
#include "serial/aq_serial_threaded.h"
#include "threads/thread_utils.h"
#include "web/aq_web.h"

bool main_loop()
{
	thrd_t serial_worker_thread, webserver_worker_thread;
	bool ran_successfully = false;

	// 1. Initialise configuration parameters and global data sets.
	if (!initialise_termination_handler())
	{
		ERROR("Failed to create worker synchronisation primitives");
	}

	// 2. Create various "server" threads.
	else if (thrd_success != thrd_create(&serial_worker_thread, &serial_thread, (void*)&wait_for_termination))
	{
		ERROR("Failed to start serial worker thread");
	}
	else if (thrd_success != thrd_create(&webserver_worker_thread, &webserver_thread, (void*)&wait_for_termination))
	{
		ERROR("Failed to start web worker thread");
	}
	else
	{
		INFO("Serial worker thread is running");

		// 3. Go!
		configure_termination_signals();

		INFO("Running AqualinkD...");
		if (!wait_for_termination())
		{
			ERROR("Failed when attempting to block-wait for termination handler");
		}
		else
		{
			// 4. Clean up shop and terminate worker threads.
			INFO("Cleaning up and closing down");
			cleanup_termination_handler();

			ran_successfully = true;
		}
	}

	return ran_successfully;
}

int main(int argc, char* argv[])
{
	int i, j;

	// Log only NOTICE messages and above. Debug and info messages
	// will not be logged to syslog.
	initialize_logging(&aqualink_default_logger);
	set_verbosity(&aqualink_default_logger, Notice);

	// Add the default console sink
	register_logging_sink(&aqualink_default_logger.Sinks, &aqualink_default_logging_sink_console);

	// Initialize the daemon's parameters.
	initialise_config_parameters();
	init_buttons(&_aqualink_data);

	// Process any options on the command line.
	handleOptions(argc, argv);

	// Finally, process any settings from the configuration file.
	handle_configuration_file_options();

	// Do all the configuration goodness...
	{
		set_verbosity(&aqualink_default_logger, CFG_LogLevel());

		if (0 == CFG_LogFile())
		{
			// Configure the output file logger by configuring the userdata and registering the sink.  Note that this
			// will implicitly initialise the sink.

			LoggingSinkBasicFileUserData* aqualink_basic_file_user_data = (LoggingSinkBasicFileUserData*)malloc(sizeof(LoggingSinkBasicFileUserData));
			aqualink_basic_file_user_data->Filename = CFG_ConfigFile();

			aqualink_default_logger_sink_file.UserData = aqualink_basic_file_user_data;
			register_logging_sink(&aqualink_default_logger.Sinks, &aqualink_default_logger_sink_file);
		}
	}

	// Initialise the master controller (and turn on the simulator).
	initialise_aqualinkrs_controller(&aqualink_master_controller, RS8);
	enable_aqualinkrs_keypadsimulator(aqualink_master_controller.Simulator);

	INFO("%s %s", AQUALINKD_NAME, AQUALINKD_VERSION);

	NOTICE("Config level             = %s", logging_level_to_string(CFG_LogLevel()));
	NOTICE("Config socket_port       = %d", CFG_SocketPort());
	NOTICE("Config serial_port       = %s", CFG_SerialPort());
	NOTICE("Config web_directory     = %s", CFG_WebDirectory());
	NOTICE("Config insecure          = %s", bool2text(CFG_Insecure()));
	NOTICE("Config device_id         = 0x%02hhx", CFG_DeviceId());
	NOTICE("Config read_all_devices  = %s", bool2text(CFG_ReadAllDevices()));
	NOTICE("Config use_aux_labels    = %s", bool2text(CFG_UsePanelAuxLabels()));
	NOTICE("Config override frz prot = %s", bool2text(CFG_OverrideFreezeProtect()));

#ifndef MG_DISABLE_MQTT
	NOTICE("Config mqtt_server       = %s", CFG_MqttServer());
	NOTICE("Config mqtt_dz_sub_topic = %s", CFG_MqttDzSubTopic());
	NOTICE("Config mqtt_dz_pub_topic = %s", CFG_MqttDzPubTopic());
	NOTICE("Config mqtt_aq_topic     = %s", CFG_MqttAqTopic());
	NOTICE("Config mqtt_user         = %s", CFG_MqttUser());
	NOTICE("Config mqtt_passwd       = %s", CFG_MqttPassword());
	NOTICE("Config mqtt_ID           = %s", CFG_MqttId());
	NOTICE("Config idx water temp    = %d", CFG_DzIdxAirTemp());
	NOTICE("Config idx pool temp     = %d", CFG_DzIdxPoolWaterTemp());
	NOTICE("Config idx spa temp      = %d", CFG_DzIdxSpaWaterTemp());
	NOTICE("Config idx SWG Percent   = %d", CFG_DzIdxSwgPercent());
	NOTICE("Config idx SWG PPM       = %d", CFG_DzIdxSwgPpm());
	NOTICE("Config PDA Mode          = %s", bool2text(CFG_PdaMode()));
	NOTICE("Config PDA Sleep Mode    = %s", bool2text(CFG_PdaSleepMode()));
	NOTICE("Config force SWG         = %s", bool2text(CFG_ForceSwg()));
#endif // MG_DISABLE_MQTT

	NOTICE("Config no_daemonize      = %s", bool2text(CFG_NoDaemonize()));
	NOTICE("Config file              = %s", CFG_ConfigFile());
	NOTICE("Config light_pgm_mode    = %.2f", CFG_LightProgrammingMode());
	NOTICE("Log file                 = %s", CFG_LogFile());
	NOTICE("Debug RS485 protocol     = %s", bool2text(CFG_DebugRsProtocolPackets()));
	NOTICE("Read Pentair Packets     = %s", bool2text(CFG_ReadPentairPackets()));
	NOTICE("Display warnings in web  = %s", bool2text(CFG_DisplayWarningsWeb()));

	if (CFG_SwgZeroIgnore() > 0)
	{
		NOTICE("Ignore SWG 0 msg count   = %d", CFG_SwgZeroIgnore());
	}

	for (i = FilterPump; i < ButtonTypeCount; i++)
	{
		char vsp[] = "None";

		for (j = 0; j < MAX_PUMPS; j++)
		{
			if (_aqualink_data.pumps[j].button == &_aqualink_data.aqbuttons[i])
			{
				sprintf(vsp, "0x%02hhx", _aqualink_data.pumps[j].pumpID);
			}
		}

		if (!CFG_PdaMode())
		{
			NOTICE("Config BTN %-13s = label %-15s | VSP ID %-4s  | dzidx %d | %s", _aqualink_data.aqbuttons[i].name, _aqualink_data.aqbuttons[i].label, vsp, _aqualink_data.aqbuttons[i].dz_idx, (i > 0 && (i == CFG_LightProgrammingButtonPool() || i == CFG_LightProgrammingButtonSpa()) ? "Programable" : ""));
		}
		else
		{
			NOTICE("Config BTN %-13s = label %-15s | VSP ID %-4s  | PDAlabel %-15s | dzidx %d", _aqualink_data.aqbuttons[i].name, _aqualink_data.aqbuttons[i].label, vsp, _aqualink_data.aqbuttons[i].pda_label, _aqualink_data.aqbuttons[i].dz_idx);
		}
	}

	if (CFG_NoDaemonize() == true)
	{
		NOTICE("Running %s interactively...", AQUALINKD_NAME);

		if (!main_loop())
		{
			ERROR("Failed to start %s interactively...exiting", AQUALINKD_NAME);
			return EXIT_FAILURE;
		}
	}
	else
	{
		NOTICE("Running %s as a daemon...", AQUALINKD_NAME);

		if (!daemonize(main_loop))
		{
			ERROR("Failed to start %s as a daemon...exiting", AQUALINKD_NAME);
			return EXIT_FAILURE;
		}
	}

	shutdown_logging(&aqualink_default_logger);

	return EXIT_SUCCESS;
}

/*void main_loop()
{
	struct mg_mgr mgr;
	SerialDevice rs_serialDevice;
	int packet_length;
	unsigned char packet_buffer[AQ_MAXPKTLEN + 1];
	bool interestedInNextAck = false;
	bool changed = false;
	int swg_noreply_cnt = 0;
	int i;

	// NSF need to find a better place to init this.
	_aqualink_data.simulate_panel = false;
	_aqualink_data.active_thread.thread_id = 0;
	_aqualink_data.air_temp = TEMP_UNKNOWN;
	_aqualink_data.pool_temp = TEMP_UNKNOWN;
	_aqualink_data.spa_temp = TEMP_UNKNOWN;
	_aqualink_data.frz_protect_set_point = TEMP_UNKNOWN;
	_aqualink_data.pool_htr_set_point = TEMP_UNKNOWN;
	_aqualink_data.spa_htr_set_point = TEMP_UNKNOWN;
	_aqualink_data.unactioned.type = NO_ACTION;
	_aqualink_data.swg_percent = TEMP_UNKNOWN;
	_aqualink_data.swg_ppm = TEMP_UNKNOWN;
	_aqualink_data.ar_swg_status = SWG_STATUS_OFF;
	_aqualink_data.swg_delayed_percent = TEMP_UNKNOWN;
	_aqualink_data.temp_units = UNKNOWN;
	_aqualink_data.single_device = false;
	_aqualink_data.service_mode_state = OFF;
	_aqualink_data.frz_protect_state = OFF;
	_aqualink_data.battery = OK;
	_aqualink_data.open_websockets = 0;

	pthread_mutex_init(&_aqualink_data.active_thread.thread_mutex, NULL);
	pthread_cond_init(&_aqualink_data.active_thread.thread_cond, NULL);

	for (i = 0; i < MAX_PUMPS; i++) {
		_aqualink_data.pumps[i].rpm = TEMP_UNKNOWN;
		_aqualink_data.pumps[i].gph = TEMP_UNKNOWN;
		_aqualink_data.pumps[i].watts = TEMP_UNKNOWN;
	}

	if (force_swg == true) {
		_aqualink_data.swg_percent = 0;
		_aqualink_data.swg_ppm = 0;
	}

	if (!start_net_services(&mgr, &_aqualink_data))
	{
		ERROR("Can not start webserver on port %s.", socket_port);
		exit(EXIT_FAILURE);
	}

	startPacketLogger(debug_RSProtocol_packets, read_pentair_packets);

	signal(SIGINT, intHandler);
	signal(SIGTERM, intHandler);

	int blank_read = 0;
	rs_serialDevice = init_serial_port(serial_port);
	NOTICE("Listening to Aqualink RS8 on serial port: %s", serial_port);

	if (pda_mode == true)
	{

#ifdef BETA_PDA_AUTOLABEL
		init_pda(&_aqualink_data, &_config_parameters);
#else
		init_pda(&_aqualink_data);
#endif

	}

	if (device_id == 0x00) {
		NOTICE("Searching for valid ID, please configure one for faster startup");
	}

	while (_keepRunning == true)
	{
		while ((rs_serialDevice < 0 || blank_read >= MAX_ZERO_READ_BEFORE_RECONNECT) && _keepRunning == true)
		{
			if (rs_serialDevice < 0)
			{
				sprintf(_aqualink_data.last_display_message, CONNECTION_ERROR);
				ERROR("Aqualink daemon attempting to connect to master device...");
				broadcast_aqualinkstate_error(mgr.active_connections, CONNECTION_ERROR);
				mg_mgr_poll(&mgr, 1000); // Sevice messages
				mg_mgr_poll(&mgr, 3000); // should donothing for 3 seconds.
			}
			else
			{
				ERROR("Aqualink daemon looks like serial error, resetting.");
				close_serial_port(rs_serialDevice);
			}
			rs_serialDevice = init_serial_port(serial_port);
			blank_read = 0;
		}

		if (raw_RS_bytes) {
			packet_length = get_packet_lograw(rs_serialDevice, packet_buffer);
		}
		else {
			packet_length = get_packet(rs_serialDevice, packet_buffer);
		}

		if (packet_length == -1)
		{
			// Unrecoverable read error. Force an attempt to reconnect.
			ERROR("Bad packet length, reconnecting");
			blank_read = MAX_ZERO_READ_BEFORE_RECONNECT;
		}
		else if (packet_length == 0)
		{
			DEBUG("Blank packed received");
			blank_read++;
		}
		else if (_config_parameters.device_id == 0x00) {
			DEBUG("Finding an unused address");
			blank_read = 0;
			device_id = find_unused_address(packet_buffer);
			continue;
		}
		else if (packet_length > 0)
		{
			DEBUG("Processing packet");

			///TEST TEST TEST
			process_aqualink_packet(packet_buffer, packet_length);

			blank_read = 0;
			changed = false;

			if (packet_length > 0 && packet_buffer[PKT_DEST] == device_id)
			{

				if (getLogLevel() >= DEBUG) 
				{
					DEBUG("RS received packet of type %s length %d", get_packet_type(packet_buffer, packet_length), packet_length);
				}

				// Process the packet. This includes deriving general status, and identifying
				// warnings and errors.  If something changed, notify any listeners
				
				if (process_packet(packet_buffer, packet_length) != false)
				{
					changed = true;
				}

				// If we are not in PDA or Simulator mode, just sent ACK & any CMD, else caculate the ACK.
				if (!_aqualink_data.simulate_panel && !pda_mode) 
				{
					send_ack(rs_serialDevice, pop_aq_cmd(&_aqualink_data));
				}
				else {
					caculate_ack_packet(rs_serialDevice, packet_buffer);
				}
			}
			else if (packet_length > 0 && read_all_devices == true)
			{
				if (packet_buffer[PKT_DEST] == DEV_MASTER && interestedInNextAck == true)
				{
					swg_noreply_cnt = 0;
					changed = processPacketFromSWG(packet_buffer, packet_length, &_aqualink_data);
					interestedInNextAck = false;
				}
				else if (interestedInNextAck == true && packet_buffer[PKT_DEST] != DEV_MASTER && _aqualink_data.ar_swg_status != SWG_STATUS_OFF)
				{
					if (++swg_noreply_cnt < 3) {
						_aqualink_data.ar_swg_status = SWG_STATUS_OFF;
						changed = true;
					}
					interestedInNextAck = false;
				}
				else if (packet_buffer[PKT_DEST] == SWG_DEV_ID)
				{
					interestedInNextAck = true;
					changed = processPacketToSWG(packet_buffer, packet_length, &_aqualink_data, swg_zero_ignore);
				}
				else
				{
					interestedInNextAck = false;
				}

				if (read_pentair_packets && getProtocolType(packet_buffer) == PENTAIR) {
					if (processPentairPacket(packet_buffer, packet_length, &_aqualink_data)) {
						changed = true;
					}
				}
			}

			if (changed) {
				broadcast_aqualinkstate(mgr.active_connections);
			}
		}
		mg_mgr_poll(&mgr, 0);

		// Any unactioned commands
		if (_aqualink_data.unactioned.type != NO_ACTION)
		{
			time_t now;
			time(&now);
			if (difftime(now, _aqualink_data.unactioned.requested) > 2)
			{
				DEBUG("Actioning delayed request");
				action_delayed_request();
			}
		}

#ifdef BLOCKING_MODE
		// NOTHING HERE
#else
		tcdrain(rs_serialDevice); // Make sure buffer has been sent.
		delayMicroseconds(10);
#endif

	}

	stopPacketLogger();
	// Reset and close the port.
	close_serial_port(rs_serialDevice);
	// Clear webbrowser
	mg_mgr_free(&mgr);

	// NSF need to run through config memory and clean up.

	NOTICE("Exit!");
	exit(EXIT_FAILURE);
}
*/
