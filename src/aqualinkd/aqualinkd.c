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
#define __USE_XOPEN 1

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <libgen.h>
#include <termios.h>
#include <signal.h>
#include <getopt.h>

#include <time.h> // Need GNU_SOURCE & XOPEN defined for strptime

#include "mongoose.h"
#include "aqualink.h"
#include "utils.h"
#include "config.h"
#include "aq_serial.h"
#include "init_buttons.h"
#include "aq_programmer.h"
#include "net_services.h"
#include "pda_menu.h"
#include "pda.h"
#include "pentair_messages.h"
#include "pda_aq_programmer.h"
#include "packetLogger.h"
#include "aquapure.h"
#include "version.h"

static volatile bool _keepRunning = true;
static struct aqconfig _config_parameters;
static struct aqualinkdata _aqualink_data;

void main_loop();

void intHandler(int dummy)
{
	UNREFERENCED_PARAMETER(dummy);

	_keepRunning = false;
	logMessage(LOG_NOTICE, "Stopping!");

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
		logMessage(LOG_DEBUG, "time not checked, will check in %d seconds", TIME_CHECK_INTERVAL - time_difference);
		return true;
	}

	last_checked = now;

	char datestr[DATE_STRING_LEN];
	strcpy(&datestr[0], _aqualink_data.date);
	strcpy(&datestr[12], " ");
	strcpy(&datestr[13], _aqualink_data.time);

	if (strptime(datestr, "%m/%d/%y %a %I:%M %p", &aq_tm) == NULL)
	{
		logMessage(LOG_ERR, "Could not convert RS time string '%s'", datestr);
		last_checked = (time_t)NULL;
		return true;
	}

	aq_tm.tm_isdst = -1; // Force mktime to use local timezone
	aqualink_time = mktime(&aq_tm);
	time_difference = (int)difftime(now, aqualink_time);

	logMessage(LOG_INFO, "Aqualink time is off by %d seconds...\n", time_difference);

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

	if (_config_parameters.use_panel_aux_labels == true)
	{
		aq_programmer(AQ_GET_AUX_LABELS, NULL, &_aqualink_data);
	}
}

void setUnits(char* msg)
{
	logMessage(LOG_DEBUG, "Getting temp units from message %s, looking at %c", msg, msg[strlen(msg) - 1]);

	if (msg[strlen(msg) - 1] == 'F') {
		_aqualink_data.temp_units = FAHRENHEIT;
	}
	else if (msg[strlen(msg) - 1] == 'C') {
		_aqualink_data.temp_units = CELSIUS;
	}
	else {
		_aqualink_data.temp_units = UNKNOWN;
	}

	logMessage(LOG_INFO, "Temp Units set to %d (F=0, C=1, Unknown=3)", _aqualink_data.temp_units);
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

	logMessage(LOG_INFO, "RS Message :- '%s'\n", msg);

	// Check long messages in this if/elseif block first, as some messages are similar.
	// ie "POOL TEMP" and "POOL TEMP IS SET TO"  so want correct match first.
	//

	if (stristr(msg, "JANDY AquaLinkRS") != NULL)
	{
		_aqualink_data.last_display_message[0] = '\0';
	}

	// Don't do any message counts if we are programming
	if (_aqualink_data.active_thread.thread_id == 0) {
		// If we have more than 10 messages without "Service Mode is active" assume it's off.
		if (_aqualink_data.service_mode_state != OFF && service_msg_count++ > 10)
		{
			_aqualink_data.service_mode_state = OFF;
			service_msg_count = 0;
		}

		// If we have more than 40 messages without "SALT or AQUAPURE" assume SWG is off.
		if (_config_parameters.read_all_devices == false)
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

	if (stristr(msg, LNG_MSG_BATTERY_LOW) != NULL)
	{
		_aqualink_data.battery = LOW;
		strcpy(_aqualink_data.last_display_message, msg); // Also display the message on web UI
	}
	else if (stristr(msg, LNG_MSG_POOL_TEMP_SET) != NULL)
	{
		_aqualink_data.pool_htr_set_point = atoi(message + 20);

		if (_aqualink_data.temp_units == UNKNOWN) {
			setUnits(msg);
		}
	}
	else if (stristr(msg, LNG_MSG_SPA_TEMP_SET) != NULL)
	{
		_aqualink_data.spa_htr_set_point = atoi(message + 19);

		if (_aqualink_data.temp_units == UNKNOWN) {
			setUnits(msg);
		}
	}
	else if (stristr(msg, LNG_MSG_FREEZE_PROTECTION_SET) != NULL)
	{
		_aqualink_data.frz_protect_set_point = atoi(message + 28);
		_aqualink_data.frz_protect_state = ENABLE;

		if (_aqualink_data.temp_units == UNKNOWN) {
			setUnits(msg);
		}
	}
	else if (strncasecmp(msg, MSG_AIR_TEMP, MSG_AIR_TEMP_LEN) == 0)
	{
		_aqualink_data.air_temp = atoi(msg + MSG_AIR_TEMP_LEN);

		if (_aqualink_data.temp_units == UNKNOWN) {
			setUnits(msg);
		}
	}
	else if (strncasecmp(msg, MSG_POOL_TEMP, MSG_POOL_TEMP_LEN) == 0)
	{
		_aqualink_data.pool_temp = atoi(msg + MSG_POOL_TEMP_LEN);

		if (_aqualink_data.temp_units == UNKNOWN) {
			setUnits(msg);
		}
	}
	else if (strncasecmp(msg, MSG_SPA_TEMP, MSG_SPA_TEMP_LEN) == 0)
	{
		_aqualink_data.spa_temp = atoi(msg + MSG_SPA_TEMP_LEN);

		if (_aqualink_data.temp_units == UNKNOWN) {
			setUnits(msg);
		}
	}
	// NSF If get water temp rather than pool or spa in some cases, then we are in Pool OR Spa ONLY mode
	else if (strncasecmp(msg, MSG_WATER_TEMP, MSG_WATER_TEMP_LEN) == 0)
	{
		_aqualink_data.pool_temp = atoi(msg + MSG_WATER_TEMP_LEN);
		_aqualink_data.spa_temp = atoi(msg + MSG_WATER_TEMP_LEN);
		if (_aqualink_data.temp_units == UNKNOWN) {
			setUnits(msg);
		}

		if (_aqualink_data.single_device != true)
		{
			_aqualink_data.single_device = true;
			logMessage(LOG_NOTICE, "AqualinkD set to 'Pool OR Spa Only' mode\n");
		}
	}
	else if (stristr(msg, LNG_MSG_WATER_TEMP1_SET) != NULL)
	{
		_aqualink_data.pool_htr_set_point = atoi(message + 28);

		if (_aqualink_data.temp_units == UNKNOWN) {
			setUnits(msg);
		}

		if (_aqualink_data.single_device != true)
		{
			_aqualink_data.single_device = true;
			logMessage(LOG_NOTICE, "AqualinkD set to 'Pool OR Spa Only' mode\n");
		}
	}
	else if (stristr(msg, LNG_MSG_WATER_TEMP2_SET) != NULL)
	{
		_aqualink_data.spa_htr_set_point = atoi(message + 27);

		if (_aqualink_data.temp_units == UNKNOWN) {
			setUnits(msg);
		}

		if (_aqualink_data.single_device != true)
		{
			_aqualink_data.single_device = true;
			logMessage(LOG_NOTICE, "AqualinkD set to 'Pool OR Spa Only' mode\n");
		}
	}
	else if (stristr(msg, LNG_MSG_SERVICE_ACTIVE) != NULL)
	{
		if (_aqualink_data.service_mode_state == OFF) {
			logMessage(LOG_NOTICE, "AqualinkD set to Service Mode\n");
		}
		_aqualink_data.service_mode_state = ON;
		service_msg_count = 0;
	}
	else if (stristr(msg, LNG_MSG_TIMEOUT_ACTIVE) != NULL)
	{
		if (_aqualink_data.service_mode_state == OFF) {
			logMessage(LOG_NOTICE, "AqualinkD set to Timeout Mode\n");
		}
		_aqualink_data.service_mode_state = FLASH;
		service_msg_count = 0;
	}
	else if (stristr(msg, LNG_MSG_FREEZE_PROTECTION_ACTIVATED) != NULL)
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
	else if (strncasecmp(msg, MSG_SWG_PCT, MSG_SWG_PCT_LEN) == 0)
	{
		_aqualink_data.swg_percent = atoi(msg + MSG_SWG_PCT_LEN);
		if (_aqualink_data.ar_swg_status == SWG_STATUS_OFF)
		{
			_aqualink_data.ar_swg_status = SWG_STATUS_ON;
		}

		swg_msg_count = 0;
	}
	else if (strncasecmp(msg, MSG_SWG_PPM, MSG_SWG_PPM_LEN) == 0)
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
			logMessage(LOG_NOTICE, "RS time is NOT accurate '%s %s', re-setting on controller!\n", _aqualink_data.time, _aqualink_data.date);
			aq_programmer(AQ_SET_TIME, NULL, &_aqualink_data);
		}
		else
		{
			logMessage(LOG_DEBUG, "RS time is accurate '%s %s'\n", _aqualink_data.time, _aqualink_data.date);
		}
		// If we get a time message before REV, the controller didn't see us as we started too quickly.
		if (_gotREV == false)
		{
			logMessage(LOG_NOTICE, "Getting control panel information\n", msg);
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
		logMessage(LOG_NOTICE, "Control Panel %s\n", msg);
		if (_initWithRS == false)
		{
			queueGetProgramData();
			_initWithRS = true;
		}
	}
	else if (stristr(msg, " TURNS ON") != NULL)
	{
		logMessage(LOG_NOTICE, "Program data '%s'\n", msg);
	}
	else if (_config_parameters.override_freeze_protect == TRUE && strncasecmp(msg, "Press Enter* to override Freeze Protection with", 47) == 0)
	{
		aq_send_cmd(KEY_ENTER);
	}
	else if ((msg[4] == ':') && (strncasecmp(msg, "AUX", 3) == 0))
	{ // AUX label "AUX1:"
		int labelid = atoi(msg + 3);
		if (labelid > 0 && _config_parameters.use_panel_aux_labels == true)
		{
			// Aux1: on panel = Button 3 in aqualinkd  (button 2 in array)
			logMessage(LOG_NOTICE, "AUX LABEL %d '%s'\n", labelid + 1, msg);
			_aqualink_data.aqbuttons[labelid + 1].label = prettyString(cleanalloc(msg + 5));
		}
	}
	// BOOST POOL 23:59 REMAINING
	else if ((strncasecmp(msg, "BOOST POOL", 10) == 0) && (strstr(msg, "REMAINING") != NULL)) {
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
		logMessage(LOG_DEBUG_SERIAL, "Ignoring '%s'\n", msg);

		if (_aqualink_data.active_thread.thread_id == 0 &&
			stristr(msg, "JANDY AquaLinkRS") == NULL &&
			stristr(msg, "PUMP O") == NULL &&// Catch 'PUMP ON' and 'PUMP OFF' but not 'PUMP WILL TURN ON'
			stristr(msg, "MAINTAIN") == NULL /* && // Catch 'MAINTAIN TEMP IS OFF'
			stristr(msg, "CLEANER O") == NULL &&
			stristr(msg, "SPA O") == NULL &&
			stristr(msg, "AUX") == NULL*/
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
		logMessage(LOG_DEBUG_SERIAL, "RS Received duplicate, ignoring.\n", length);
		return rtn;
	}

	memcpy(last_packet, packet, length);
	_aqualink_data.last_packet_type = packet[PKT_CMD];
	rtn = true;

	if (_config_parameters.pda_mode == true)
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

		if (_aqualink_data.aqbuttons[PUMP_INDEX].led->state == OFF)
		{
			_aqualink_data.pool_temp = TEMP_UNKNOWN;
			_aqualink_data.spa_temp = TEMP_UNKNOWN;
		}
		else if (_aqualink_data.aqbuttons[SPA_INDEX].led->state == OFF && _aqualink_data.single_device != true)
		{
			_aqualink_data.spa_temp = TEMP_UNKNOWN;
		}
		else if (_aqualink_data.aqbuttons[SPA_INDEX].led->state == ON && _aqualink_data.single_device != true)
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
				logMessage(LOG_ERR, "Long message index %d doesn't match buffer %d\n", index, processing_long_msg);
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
		logMessage(LOG_DEBUG, "RS Received PROBE length %d.\n", length);
		rtn = false;
		break;

	default:
		logMessage(LOG_INFO, "RS Received unknown packet, 0x%02hhx\n", packet[PKT_CMD]);
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
			logMessage(LOG_NOTICE, "Setting pool heater setpoint to %d\n", _aqualink_data.unactioned.value);
		}
		else
		{
			logMessage(LOG_NOTICE, "Pool heater setpoint is already %d, not changing\n", _aqualink_data.unactioned.value);
		}
	}
	else if (_aqualink_data.unactioned.type == SPA_HTR_SETOINT)
	{
		_aqualink_data.unactioned.value = setpoint_check(SPA_HTR_SETOINT, _aqualink_data.unactioned.value, &_aqualink_data);
		if (_aqualink_data.spa_htr_set_point != _aqualink_data.unactioned.value)
		{
			aq_programmer(AQ_SET_SPA_HEATER_TEMP, sval, &_aqualink_data);
			logMessage(LOG_NOTICE, "Setting spa heater setpoint to %d\n", _aqualink_data.unactioned.value);
		}
		else
		{
			logMessage(LOG_NOTICE, "Spa heater setpoint is already %d, not changing\n", _aqualink_data.unactioned.value);
		}
	}
	else if (_aqualink_data.unactioned.type == FREEZE_SETPOINT)
	{
		_aqualink_data.unactioned.value = setpoint_check(FREEZE_SETPOINT, _aqualink_data.unactioned.value, &_aqualink_data);
		if (_aqualink_data.frz_protect_set_point != _aqualink_data.unactioned.value)
		{
			aq_programmer(AQ_SET_FRZ_PROTECTION_TEMP, sval, &_aqualink_data);
			logMessage(LOG_NOTICE, "Setting freeze protect to %d\n", _aqualink_data.unactioned.value);
		}
		else
		{
			logMessage(LOG_NOTICE, "Freeze setpoint is already %d, not changing\n", _aqualink_data.unactioned.value);
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
				logMessage(LOG_NOTICE, "Setting SWG %% to %d\n", _aqualink_data.unactioned.value);
			}
			else
			{
				logMessage(LOG_NOTICE, "SWG % is already %d, not changing\n", _aqualink_data.unactioned.value);
			}
		}
		// Let's just tell everyone we set it, before we actually did.  Makes homekit happy, and it will re-correct on error.
		_aqualink_data.swg_percent = _aqualink_data.unactioned.value;
	}
	else if (_aqualink_data.unactioned.type == SWG_BOOST)
	{
		if (_aqualink_data.ar_swg_status == SWG_STATUS_OFF) {
			logMessage(LOG_ERR, "SWG is off, can't Boost pool\n");
		}
		else if (_aqualink_data.unactioned.value == _aqualink_data.boost) {
			logMessage(LOG_ERR, "Request to turn Boost %s ignored, Boost is already %s\n", _aqualink_data.unactioned.value ? "On" : "Off", _aqualink_data.boost ? "On" : "Off");
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

void printHelp()
{
	printf("%s %s\n", AQUALINKD_NAME, AQUALINKD_VERSION);
	printf("\t-h         (this message)\n");
	printf("\t-d         (do not deamonize)\n");
	printf("\t-c <file>  (Configuration file)\n");
	printf("\t-v         (Debug logging)\n");
	printf("\t-vv        (Serial Debug logging)\n");
	printf("\t-rsd       (RS485 debug)\n");
	printf("\t-rsrd      (RS485 raw debug)\n");
}

static struct option aqualink_long_options[] =
{
	{ "help",         no_argument,       NULL, 'h'},
	{ "no-daemonize", no_argument,       NULL, 'd'},
	{ "config-file",  required_argument, NULL, 'c'},
	{ "debug",        no_argument,       NULL, 'v'},
	{ "vv",           no_argument,       NULL, '0'},
	{ "rsd",          no_argument,       NULL, '1'},
	{ "rsrd",         no_argument,       NULL, '2'},
	{0, 0, 0, 0}
};

static char* aqualink_short_options = "hdc:v0123";

int main(int argc, char* argv[])
{
	int i, j;
	char defaultCfg[] = "./aqualinkd.conf";
	char* cfgFile;

	int cmdln_loglevel = -1;
	bool cmdln_debugRS485 = false;
	bool cmdln_lograwRS485 = false;

	// Log only NOTICE messages and above. Debug and info messages
	// will not be logged to syslog.
	setlogmask(LOG_UPTO(LOG_NOTICE));

	// Initialize the daemon's parameters.
	init_parameters(&_config_parameters);
	cfgFile = defaultCfg;

	int ch = 0;

	while ((ch = getopt_long(argc, argv, aqualink_short_options, aqualink_long_options, NULL)) != -1)
	{
		// check to see if a single character or long option came through
		switch (ch)
		{
		case 'd': // short option 'd' / long option "no-daemonize"
			_config_parameters.deamonize = false;
			break;

		case 'c': // short option 'c' / long option "config-file"
			cfgFile = optarg;
			break;

		case 'v': // short option 'v' / long option "debug"
			cmdln_loglevel = LOG_DEBUG;
			break;

		case '0': // short option '0' / long option "vv"
			cmdln_loglevel = LOG_DEBUG_SERIAL;
			break;

		case '1': // short option '1' / long option "rsd"
			cmdln_debugRS485 = true;
			break;

		case '2': // short option '2' / long option "rsrd"
			cmdln_lograwRS485 = true;
			break;

		case 'h': // short option 'h' / long option "help"
		default:  // any unknown options
			printHelp();
			return EXIT_SUCCESS;;
		}
	}

	if (getuid() != 0)
	{
		fprintf(stderr, "ERROR %s Can only be run as root\n", argv[0]);
		return EXIT_FAILURE;
	}

	initButtons(&_aqualink_data);

	readCfg(&_config_parameters, &_aqualink_data, cfgFile);

	if (cmdln_loglevel != -1)
	{
		_config_parameters.log_level = cmdln_loglevel;
	}

	if (cmdln_debugRS485)
	{
		_config_parameters.debug_RSProtocol_packets = true;
	}

	if (cmdln_lograwRS485)
	{
		_config_parameters.log_raw_RS_bytes = true;
	}

	if (_config_parameters.display_warnings_web == true)
	{
		setLoggingPrms(_config_parameters.log_level, _config_parameters.deamonize, _config_parameters.log_file, _aqualink_data.last_display_message);
	}
	else
	{
		setLoggingPrms(_config_parameters.log_level, _config_parameters.deamonize, _config_parameters.log_file, NULL);
	}

	logMessage(LOG_NOTICE, "%s v%s\n", AQUALINKD_NAME, AQUALINKD_VERSION);

	logMessage(LOG_NOTICE, "Config log_level         = %d\n", _config_parameters.log_level);
	logMessage(LOG_NOTICE, "Config socket_port       = %s\n", _config_parameters.socket_port);
	logMessage(LOG_NOTICE, "Config serial_port       = %s\n", _config_parameters.serial_port);
	logMessage(LOG_NOTICE, "Config web_directory     = %s\n", _config_parameters.web_directory);
	logMessage(LOG_NOTICE, "Config device_id         = 0x%02hhx\n", _config_parameters.device_id);
	logMessage(LOG_NOTICE, "Config read_all_devices  = %s\n", bool2text(_config_parameters.read_all_devices));
	logMessage(LOG_NOTICE, "Config use_aux_labels    = %s\n", bool2text(_config_parameters.use_panel_aux_labels));
	logMessage(LOG_NOTICE, "Config override frz prot = %s\n", bool2text(_config_parameters.override_freeze_protect));

#ifndef MG_DISABLE_MQTT
	logMessage(LOG_NOTICE, "Config mqtt_server       = %s\n", _config_parameters.mqtt_server);
	logMessage(LOG_NOTICE, "Config mqtt_dz_sub_topic = %s\n", _config_parameters.mqtt_dz_sub_topic);
	logMessage(LOG_NOTICE, "Config mqtt_dz_pub_topic = %s\n", _config_parameters.mqtt_dz_pub_topic);
	logMessage(LOG_NOTICE, "Config mqtt_aq_topic     = %s\n", _config_parameters.mqtt_aq_topic);
	logMessage(LOG_NOTICE, "Config mqtt_user         = %s\n", _config_parameters.mqtt_user);
	logMessage(LOG_NOTICE, "Config mqtt_passwd       = %s\n", _config_parameters.mqtt_passwd);
	logMessage(LOG_NOTICE, "Config mqtt_ID           = %s\n", _config_parameters.mqtt_ID);
	logMessage(LOG_NOTICE, "Config idx water temp    = %d\n", _config_parameters.dzidx_air_temp);
	logMessage(LOG_NOTICE, "Config idx pool temp     = %d\n", _config_parameters.dzidx_pool_water_temp);
	logMessage(LOG_NOTICE, "Config idx spa temp      = %d\n", _config_parameters.dzidx_spa_water_temp);
	logMessage(LOG_NOTICE, "Config idx SWG Percent   = %d\n", _config_parameters.dzidx_swg_percent);
	logMessage(LOG_NOTICE, "Config idx SWG PPM       = %d\n", _config_parameters.dzidx_swg_ppm);
	logMessage(LOG_NOTICE, "Config PDA Mode          = %s\n", bool2text(_config_parameters.pda_mode));
	logMessage(LOG_NOTICE, "Config PDA Sleep Mode    = %s\n", bool2text(_config_parameters.pda_sleep_mode));
	logMessage(LOG_NOTICE, "Config force SWG         = %s\n", bool2text(_config_parameters.force_swg));
#endif // MG_DISABLE_MQTT

	logMessage(LOG_NOTICE, "Config deamonize         = %s\n", bool2text(_config_parameters.deamonize));
	logMessage(LOG_NOTICE, "Config log_file          = %s\n", _config_parameters.log_file);
	logMessage(LOG_NOTICE, "Config light_pgm_mode    = %.2f\n", _config_parameters.light_programming_mode);
	logMessage(LOG_NOTICE, "Debug RS485 protocol     = %s\n", bool2text(_config_parameters.debug_RSProtocol_packets));
	logMessage(LOG_NOTICE, "Read Pentair Packets     = %s\n", bool2text(_config_parameters.read_pentair_packets));
	logMessage(LOG_NOTICE, "Display warnings in web  = %s\n", bool2text(_config_parameters.display_warnings_web));

	if (_config_parameters.swg_zero_ignore > 0) {
		logMessage(LOG_NOTICE, "Ignore SWG 0 msg count   = %d\n", _config_parameters.swg_zero_ignore);
	}

	for (i = 0; i < TOTAL_BUTONS; i++)
	{
		char vsp[] = "None";
		for (j = 0; j < MAX_PUMPS; j++) {
			if (_aqualink_data.pumps[j].button == &_aqualink_data.aqbuttons[i]) {
				sprintf(vsp, "0x%02hhx", _aqualink_data.pumps[j].pumpID);
			}
		}
		if (!_config_parameters.pda_mode) {
			logMessage(LOG_NOTICE, "Config BTN %-13s = label %-15s | VSP ID %-4s  | dzidx %d | %s\n",
				_aqualink_data.aqbuttons[i].name, _aqualink_data.aqbuttons[i].label, vsp, _aqualink_data.aqbuttons[i].dz_idx,
				(i > 0 && (i == _config_parameters.light_programming_button_pool || i == _config_parameters.light_programming_button_spa) ? "Programable" : ""));
		}
		else {
			logMessage(LOG_NOTICE, "Config BTN %-13s = label %-15s | VSP ID %-4s  | PDAlabel %-15s | dzidx %d\n",
				_aqualink_data.aqbuttons[i].name, _aqualink_data.aqbuttons[i].label, vsp,
				_aqualink_data.aqbuttons[i].pda_label, _aqualink_data.aqbuttons[i].dz_idx);
		}
	}

	if (_config_parameters.deamonize == true)
	{
		char pidfile[256];
		sprintf(pidfile, "%s/%s.pid", "/run", basename(argv[0]));
		daemonise(pidfile, main_loop);
	}
	else
	{
		main_loop();
	}

	return EXIT_SUCCESS;
}

#define MAX_BLOCK_ACK 12
#define MAX_BUSY_ACK  (50 + MAX_BLOCK_ACK)

void caculate_ack_packet(int rs_fd, unsigned char* packet_buffer)
{
	static int delayAckCnt = 0;

	// if PDA mode, should we sleep? if not Can only send command to status message on PDA.
	if (_config_parameters.pda_mode == true) {
		if (_config_parameters.pda_sleep_mode && pda_shouldSleep()) {
			logMessage(LOG_DEBUG, "PDA Aqualink daemon in sleep mode\n");
			return;
		}

		send_extended_ack(rs_fd, ACK_PDA, pop_aq_cmd(&_aqualink_data));


	}
	else if (_aqualink_data.simulate_panel && _aqualink_data.active_thread.thread_id == 0) {
		// We are in simlator mode, ack get's complicated now.
		// If have a command to send, send a normal ack.
		// If we last message is waiting for an input "SELECT xxxxx", then sent a pause ack
		// pause ack strarts with around 12 ACK_SCREEN_BUSY_DISPLAY acks, then 50  ACK_SCREEN_BUSY acks
		// if we send a command (ie keypress), the whole count needs to end and go back to sending normal ack.
		// In code below, it jumps to sending ACK_SCREEN_BUSY, which still seems to work ok.
		if (strncasecmp(_aqualink_data.last_display_message, "SELECT", 6) != 0) 
		{ 
			// Nothing to wait for, send normal ack.
			send_ack(rs_fd, pop_aq_cmd(&_aqualink_data));
			delayAckCnt = 0;
		}
		else if (get_aq_cmd_length() > 0) 
		{
			// Send command and jump directly "busy but can receive message"
			send_ack(rs_fd, pop_aq_cmd(&_aqualink_data));
			delayAckCnt = MAX_BUSY_ACK; // need to test jumping to MAX_BUSY_ACK here
		}
		else 
		{
			logMessage(LOG_NOTICE, "Sending display busy due to Simulator mode \n");

			if (delayAckCnt < MAX_BLOCK_ACK) 
			{
				// block all incomming messages
				send_extended_ack(rs_fd, ACK_SCREEN_BUSY_BLOCK, pop_aq_cmd(&_aqualink_data));
			}
			else if (delayAckCnt < MAX_BUSY_ACK) 
			{
				// say we are pausing
				send_extended_ack(rs_fd, ACK_SCREEN_BUSY, pop_aq_cmd(&_aqualink_data));
			}
			else 
			{
				// We timed out pause, send normal ack (This should also reset the display message on next message received)
				send_ack(rs_fd, pop_aq_cmd(&_aqualink_data));
			}

			delayAckCnt++;
		}
	}
	else {
		// We are in simulate panel mode, but a thread is active, so ignore simulate panel
		send_ack(rs_fd, pop_aq_cmd(&_aqualink_data));
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
			logMessage(LOG_NOTICE, "Found valid unused ID 0x%02hhx\n", lastID);
			return lastID;
		}
		lastID = 0x00;
	}
	else {
		lastID = 0x00;
	}

	return 0x00;
}

void main_loop()
{
	struct mg_mgr mgr;
	int rs_fd;
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

	if (_config_parameters.force_swg == true) {
		_aqualink_data.swg_percent = 0;
		_aqualink_data.swg_ppm = 0;
	}

	if (!start_net_services(&mgr, &_aqualink_data, &_config_parameters))
	{
		logMessage(LOG_ERR, "Can not start webserver on port %s.\n", _config_parameters.socket_port);
		exit(EXIT_FAILURE);
	}

	startPacketLogger(_config_parameters.debug_RSProtocol_packets, _config_parameters.read_pentair_packets);

	signal(SIGINT, intHandler);
	signal(SIGTERM, intHandler);

	int blank_read = 0;
	rs_fd = init_serial_port(_config_parameters.serial_port);
	logMessage(LOG_NOTICE, "Listening to Aqualink RS8 on serial port: %s\n", _config_parameters.serial_port);

	if (_config_parameters.pda_mode == true)
	{

#ifdef BETA_PDA_AUTOLABEL
		init_pda(&_aqualink_data, &_config_parameters);
#else
		init_pda(&_aqualink_data);
#endif

	}

	if (_config_parameters.device_id == 0x00) {
		logMessage(LOG_NOTICE, "Searching for valid ID, please configure one for faster startup\n");
	}

	while (_keepRunning == true)
	{
		while ((rs_fd < 0 || blank_read >= MAX_ZERO_READ_BEFORE_RECONNECT) && _keepRunning == true)
		{
			if (rs_fd < 0)
			{
				sprintf(_aqualink_data.last_display_message, CONNECTION_ERROR);
				logMessage(LOG_ERR, "Aqualink daemon attempting to connect to master device...\n");
				broadcast_aqualinkstate_error(mgr.active_connections, CONNECTION_ERROR);
				mg_mgr_poll(&mgr, 1000); // Sevice messages
				mg_mgr_poll(&mgr, 3000); // should donothing for 3 seconds.
			}
			else
			{
				logMessage(LOG_ERR, "Aqualink daemon looks like serial error, resetting.\n");
				close_serial_port(rs_fd);
			}
			rs_fd = init_serial_port(_config_parameters.serial_port);
			blank_read = 0;
		}

		if (_config_parameters.log_raw_RS_bytes) {
			packet_length = get_packet_lograw(rs_fd, packet_buffer);
		}
		else {
			packet_length = get_packet(rs_fd, packet_buffer);
		}

		if (packet_length == -1)
		{
			// Unrecoverable read error. Force an attempt to reconnect.
			logMessage(LOG_ERR, "Bad packet length, reconnecting\n");
			blank_read = MAX_ZERO_READ_BEFORE_RECONNECT;
		}
		else if (packet_length == 0)
		{
			blank_read++;
		}
		else if (_config_parameters.device_id == 0x00) {
			blank_read = 0;
			_config_parameters.device_id = find_unused_address(packet_buffer);
			continue;
		}
		else if (packet_length > 0)
		{
			blank_read = 0;
			changed = false;

			if (packet_length > 0 && packet_buffer[PKT_DEST] == _config_parameters.device_id)
			{

				if (getLogLevel() >= LOG_DEBUG) {
					logMessage(LOG_DEBUG, "RS received packet of type %s length %d\n", get_packet_type(packet_buffer, packet_length), packet_length);
				}

				// Process the packet. This includes deriving general status, and identifying
				// warnings and errors.  If something changed, notify any listeners

				if (process_packet(packet_buffer, packet_length) != false)
				{
					changed = true;
				}

				// If we are not in PDA or Simulator mode, just sent ACK & any CMD, else caculate the ACK.
				if (!_aqualink_data.simulate_panel && !_config_parameters.pda_mode) {
					send_ack(rs_fd, pop_aq_cmd(&_aqualink_data));
				}
				else {
					caculate_ack_packet(rs_fd, packet_buffer);
				}
			}
			else if (packet_length > 0 && _config_parameters.read_all_devices == true)
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
					changed = processPacketToSWG(packet_buffer, packet_length, &_aqualink_data, _config_parameters.swg_zero_ignore);
				}
				else
				{
					interestedInNextAck = false;
				}

				if (_config_parameters.read_pentair_packets && getProtocolType(packet_buffer) == PENTAIR) {
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
				logMessage(LOG_DEBUG, "Actioning delayed request\n");
				action_delayed_request();
			}
		}

#ifdef BLOCKING_MODE
		// NOTHING HERE
#else
		tcdrain(rs_fd); // Make sure buffer has been sent.
		delayMicroseconds(10);
#endif

	}

	stopPacketLogger();
	// Reset and close the port.
	close_serial_port(rs_fd);
	// Clear webbrowser
	mg_mgr_free(&mgr);

	// NSF need to run through config memory and clean up.

	logMessage(LOG_NOTICE, "Exit!\n");
	exit(EXIT_FAILURE);
}
