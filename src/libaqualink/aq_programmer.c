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

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <threads.h>
#include <unistd.h>
#include <string.h>


#include "aqualink.h"
#include "utils.h"
#include "aq_programmer.h"
#include "serial/aq_serial.h"
#include "serial/aq_serial_types.h"
#include "pda.h"
#include "pda_menu.h"
#include "init_buttons.h"
#include "pda_aq_programmer.h"

#ifdef AQ_DEBUG
#include <time.h>
#include "timespec_subtract.h"
#endif

bool select_sub_menu_item(struct aqualinkdata* aq_data, char* item_string);
bool select_menu_item(struct aqualinkdata* aq_data, char* item_string);
void cancel_menu();


int set_aqualink_pool_heater_temps(void* ptr);
int set_aqualink_spa_heater_temps(void* ptr);
int set_aqualink_freeze_heater_temps(void* ptr);
int set_aqualink_time(void* ptr);
int get_aqualink_pool_spa_heater_temps(void* ptr);
int get_aqualink_programs(void* ptr);
int get_freeze_protect_temp(void* ptr);
int get_aqualink_diag_model(void* ptr);
int get_aqualink_aux_labels(void* ptr);
int set_aqualink_light_colormode(void* ptr);
int set_aqualink_PDA_init(void* ptr);
int set_aqualink_SWG(void* ptr);
int set_aqualink_boost(void* ptr);

bool waitForButtonState(struct aqualinkdata* aq_data, aqkey* button, AQ_LED_States state, int numMessageReceived);
bool waitForEitherMessage(struct aqualinkdata* aq_data, char* message1, char* message2, int numMessageReceived);

void waitfor_queue2empty();

#define MAX_STACK 20
int _stack_place = 0;
unsigned char _commands[MAX_STACK];
unsigned char _pgm_command = NUL;

bool _last_sent_was_cmd = false;

// External view of adding to queue
void aq_send_cmd(unsigned char cmd) {
	push_aq_cmd(cmd);
}

bool push_aq_cmd(unsigned char cmd) {

	//logMessage(LOG_DEBUG, "push_aq_cmd '0x%02hhx'\n", cmd);

	if (_stack_place < MAX_STACK) {
		_commands[_stack_place] = cmd;
		_stack_place++;
	}
	else {
		logMessage(LOG_ERR, "Command queue overflow, too many unsent commands to RS control panel\n");
		return false;
	}

	return true;
}

int get_aq_cmd_length()
{
	return _stack_place;
}

unsigned char pop_aq_cmd(struct aqualinkdata* aq_data)
{
	unsigned char cmd = NUL;
	// Only send commands on status messages 
	// Are we in programming mode
	if (aq_data->active_thread.thread_id != 0) {
		/*if ( (_pgm_command == KEY_MENU && aq_data->last_packet_type == CMD_STATUS) ||
		// Need to not the key_menu below
			 ( _pgm_command != NUL && (aq_data->last_packet_type == CMD_STATUS || aq_data->last_packet_type == CMD_MSG_LONG) )) {*/
		if ((_pgm_command != NUL && (aq_data->last_packet_type == CMD_STATUS)) ||
			// Boost pool has to send commands to msg long
			(aq_data->active_thread.ptype == AQ_SET_BOOST && (aq_data->last_packet_type == CMD_STATUS || aq_data->last_packet_type == CMD_MSG_LONG))) {
			cmd = _pgm_command;
			_pgm_command = NUL;
			logMessage(LOG_DEBUG, "RS SEND cmd '0x%02hhx' (programming)\n", cmd);
		}
		else if (_pgm_command != NUL) {
			logMessage(LOG_DEBUG, "RS Waiting to send cmd '0x%02hhx' (programming)\n", _pgm_command);
		}
		else {
			logMessage(LOG_DEBUG, "RS SEND cmd '0x%02hhx' empty queue (programming)\n", cmd);
		}
	}
	else if (_stack_place > 0 && aq_data->last_packet_type == CMD_STATUS) {
		cmd = _commands[0];
		_stack_place--;
		logMessage(LOG_DEBUG, "RS SEND cmd '0x%02hhx'\n", cmd);
		memmove(&_commands[0], &_commands[1], sizeof(unsigned char) * _stack_place);
	}
	else {
		logMessage(LOG_DEBUG, "RS SEND cmd '0x%02hhx'\n", cmd);
	}

	//printf("RSM sending cmd '0x%02hhx' in reply to '0x%02hhx'\n",cmd,aq_data->last_packet_type);

	return cmd;
}


//unsigned char pop_aq_cmd_old(struct aqualinkdata *aq_data);

unsigned char pop_aq_cmd_XXXXXX(struct aqualinkdata* aq_data)
{
	unsigned char cmd = NUL;
	static bool last_sent_was_cmd = false;

	// USE BELOW IF PDA HAS ISSUES WITH NEW COMMAND LOGIC

	//if ( pda_mode() == true ) {
	//  return pop_aq_cmd_old(aq_data);
	//}

	// Only press menu to a status command
	// Only send commands on status messages when programming date
	// Otherwise send every other command.

	// Are we in programming mode
	if (aq_data->active_thread.thread_id != 0) {
		if (((_pgm_command == KEY_MENU || aq_data->active_thread.ptype == AQ_SET_TIME) && aq_data->last_packet_type == CMD_STATUS) ||
			(pda_mode() == false && aq_data->active_thread.ptype != AQ_SET_TIME && last_sent_was_cmd == false) ||
			(pda_mode() == true && aq_data->last_packet_type == CMD_STATUS)
			//(pda_mode() == true && last_sent_was_cmd == false)
			) {
			cmd = _pgm_command;
			_pgm_command = NUL;
			logMessage(LOG_DEBUG, "RS SEND cmd '0x%02hhx' (programming)\n", cmd);
			/*} else if (aq_data->active_thread.ptype != AQ_SET_TIME && last_sent_was_cmd == false) {
			  cmd = _pgm_command;
			  _pgm_command = NUL;
			  logMessage(LOG_DEBUG, "RS SEND cmd '0x%02hhx' (programming)\n", cmd);*/
		}
		else if (_pgm_command != NUL) {
			logMessage(LOG_DEBUG, "RS Waiting to send cmd '0x%02hhx' (programming)\n", _pgm_command);
		}
		else {
			logMessage(LOG_DEBUG, "RS SEND cmd '0x%02hhx' empty queue (programming)\n", cmd);
		}
	}
	else if (_stack_place > 0 && aq_data->last_packet_type == CMD_STATUS) {
		cmd = _commands[0];
		_stack_place--;
		logMessage(LOG_DEBUG, "RS SEND cmd '0x%02hhx'\n", cmd);
		memmove(&_commands[0], &_commands[1], sizeof(unsigned char) * _stack_place);
	}
	else {
		logMessage(LOG_DEBUG, "RS SEND cmd '0x%02hhx'\n", cmd);
	}

	if (cmd == NUL)
	{
		last_sent_was_cmd = false;
	}
	else
	{
		last_sent_was_cmd = true;
	}

	return cmd;
}

unsigned char pop_aq_cmd_old(struct aqualinkdata* aq_data)
{
	unsigned char cmd = NUL;

	// can only send a command every other ack.

	if (_last_sent_was_cmd == true) {
		_last_sent_was_cmd = false;
	}
	else if (aq_data->active_thread.thread_id != 0) {
		cmd = _pgm_command;
		_pgm_command = NUL;
	}
	else if (_stack_place > 0) {
		cmd = _commands[0];
		_stack_place--;
		memmove(&_commands[0], &_commands[1], sizeof(unsigned char) * _stack_place);
	}

	if (cmd == NUL)
	{
		_last_sent_was_cmd = false;
	}
	else
	{
		_last_sent_was_cmd = true;
	}

	return cmd;
}

int roundTo(int num, int denominator)
{
	return ((num + (denominator / 2)) / denominator) * denominator;
}

int setpoint_check(int type, int value, struct aqualinkdata* aqdata)
{
	int rtn = value;
	int max = 0;
	int min = 0;
	char* type_msg;

	switch (type) {
	case POOL_HTR_SETOINT:
		type_msg = (aqdata->single_device ? "Temp1" : "Pool");
		if (aqdata->temp_units == CELSIUS) {
			max = HEATER_MAX_C;
			min = (aqdata->single_device ? HEATER_MIN_C : HEATER_MIN_C - 1);
		}
		else {
			max = HEATER_MAX_F;
			min = (aqdata->single_device ? HEATER_MIN_F : HEATER_MIN_F - 1);
		}
		// if single device then TEMP1 & 2 (not pool & spa), TEMP1 must be set higher than TEMP2
		if (aqdata->single_device &&
			aqdata->spa_htr_set_point != TEMP_UNKNOWN &&
			min <= aqdata->spa_htr_set_point)
		{
			min = aqdata->spa_htr_set_point + 1;
		}
		break;
	case SPA_HTR_SETOINT:
		type_msg = (aqdata->single_device ? "Temp2" : "Spa");
		if (aqdata->temp_units == CELSIUS) {
			max = (aqdata->single_device ? HEATER_MAX_C : HEATER_MAX_C - 1);
			min = HEATER_MIN_C;
		}
		else {
			max = (aqdata->single_device ? HEATER_MAX_F : HEATER_MAX_F - 1);
			min = HEATER_MIN_F;
		}
		// if single device then TEMP1 & 2 (not pool & spa), TEMP2 must be set lower than TEMP1
		if (aqdata->single_device &&
			aqdata->pool_htr_set_point != TEMP_UNKNOWN &&
			max >= aqdata->pool_htr_set_point)
		{
			max = aqdata->pool_htr_set_point - 1;
		}
		break;
	case FREEZE_SETPOINT:
		type_msg = "Freeze protect";
		if (aqdata->temp_units == CELSIUS) {
			max = FREEZE_PT_MAX_C;
			min = FREEZE_PT_MIN_C;
		}
		else {
			max = FREEZE_PT_MAX_F;
			min = FREEZE_PT_MIN_F;
		}
		break;
	case SWG_SETPOINT:
		type_msg = "Salt Water Generator";
		max = SWG_PERCENT_MAX;
		min = SWG_PERCENT_MIN;
		break;
	default:
		type_msg = "Unknown";
		break;
	}

	if (rtn > max)
	{
		rtn = max;
	}
	else if (rtn < min)
	{
		rtn = min;
	}

	// If SWG make sure it's 0,5,10,15,20......
	if (type == SWG_SETPOINT) {
		rtn = roundTo(rtn, 5);
	}

	if (rtn != value)
	{
		logMessage(LOG_WARNING, "Setpoint of %d for %s is outside range, using %d\n", value, type_msg, rtn);
	}
	else
	{
		logMessage(LOG_NOTICE, "Setting setpoint of %s to %d\n", type_msg, rtn);
	}

	return rtn;
}

void kick_aq_program_thread(struct aqualinkdata* aq_data)
{
	if (aq_data->active_thread.thread_id != 0) 
	{
		logMessage(LOG_DEBUG, "Kicking thread %d,%p message '%s'\n", aq_data->active_thread.ptype, aq_data->active_thread.thread_id, aq_data->last_message);
		cnd_broadcast(&aq_data->active_thread.thread_cond);
	}
}

void aq_programmer(program_type type, char* args, struct aqualinkdata* aq_data)
{
	struct programmingThreadCtrl* programmingthread = malloc(sizeof(struct programmingThreadCtrl));

	if (pda_mode() == true) 
	{
		pda_reset_sleep();
		if (type != AQ_PDA_INIT &&
			type != AQ_PDA_WAKE_INIT &&
			type != AQ_PDA_DEVICE_STATUS &&
			type != AQ_SET_POOL_HEATER_TEMP &&
			type != AQ_SET_SPA_HEATER_TEMP &&
			type != AQ_SET_SWG_PERCENT &&
			type != AQ_PDA_DEVICE_ON_OFF &&
#ifdef BETA_PDA_AUTOLABEL
			type != AQ_GET_AUX_LABELS &&
#endif
			type != AQ_GET_POOL_SPA_HEATER_TEMPS &&
			type != AQ_SET_FRZ_PROTECTION_TEMP &&
			type != AQ_SET_BOOST) {
			logMessage(LOG_ERR, "Selected Programming mode '%d' not supported with PDA mode control panel\n", type);
			return;
		}
	}

	programmingthread->aq_data = aq_data;
	programmingthread->thread_id = 0;
	if (args != NULL)
	{
		strncpy(programmingthread->thread_args, args, sizeof(programmingthread->thread_args) - 1);
	}

	switch (type) {
	case AQ_GET_POOL_SPA_HEATER_TEMPS:
		if (thrd_success != thrd_create(&programmingthread->thread_id, get_aqualink_pool_spa_heater_temps, (void*)programmingthread)) {
			logMessage(LOG_ERR, "could not create thread\n");
			return;
		}
		break;
	case AQ_GET_FREEZE_PROTECT_TEMP:
		if (thrd_success != thrd_create(&programmingthread->thread_id, get_freeze_protect_temp, (void*)programmingthread)) {
			logMessage(LOG_ERR, "could not create thread\n");
			return;
		}
		break;
	case AQ_SET_TIME:
		if (thrd_success != thrd_create(&programmingthread->thread_id, set_aqualink_time, (void*)programmingthread)) {
			logMessage(LOG_ERR, "could not create thread\n");
			return;
		}
		break;
	case  AQ_SET_POOL_HEATER_TEMP:
		if (thrd_success != thrd_create(&programmingthread->thread_id, set_aqualink_pool_heater_temps, (void*)programmingthread)) {
			logMessage(LOG_ERR, "could not create thread\n");
			return;
		}
		break;
	case  AQ_SET_SPA_HEATER_TEMP:
		if (thrd_success != thrd_create(&programmingthread->thread_id, set_aqualink_spa_heater_temps, (void*)programmingthread)) {
			logMessage(LOG_ERR, "could not create thread\n");
			return;
		}
		break;
	case  AQ_SET_FRZ_PROTECTION_TEMP:
		if (thrd_success != thrd_create(&programmingthread->thread_id, set_aqualink_freeze_heater_temps, (void*)programmingthread)) {
			logMessage(LOG_ERR, "could not create thread\n");
			return;
		}
		break;
	case AQ_GET_DIAGNOSTICS_MODEL:
		if (thrd_success != thrd_create(&programmingthread->thread_id, get_aqualink_diag_model, (void*)programmingthread)) {
			logMessage(LOG_ERR, "could not create thread\n");
			return;
		}
		break;
	case AQ_GET_PROGRAMS:
		if (thrd_success != thrd_create(&programmingthread->thread_id, get_aqualink_programs, (void*)programmingthread)) {
			logMessage(LOG_ERR, "could not create thread\n");
			return;
		}
		break;
	case AQ_SET_COLORMODE:
		if (thrd_success != thrd_create(&programmingthread->thread_id, set_aqualink_light_colormode, (void*)programmingthread)) {
			logMessage(LOG_ERR, "could not create thread\n");
			return;
		}
		break;
	case AQ_PDA_INIT:
		if (thrd_success != thrd_create(&programmingthread->thread_id, set_aqualink_PDA_init, (void*)programmingthread)) {
			logMessage(LOG_ERR, "could not create thread\n");
			return;
		}
		break;
	case AQ_PDA_WAKE_INIT:
		if (thrd_success != thrd_create(&programmingthread->thread_id, set_aqualink_PDA_wakeinit, (void*)programmingthread)) {
			logMessage(LOG_ERR, "could not create thread\n");
			return;
		}
		break;
	case AQ_SET_SWG_PERCENT:
		if (thrd_success != thrd_create(&programmingthread->thread_id, set_aqualink_SWG, (void*)programmingthread)) {
			logMessage(LOG_ERR, "could not create thread\n");
			return;
		}
		break;
	case AQ_PDA_DEVICE_STATUS:
		if (thrd_success != thrd_create(&programmingthread->thread_id, get_aqualink_PDA_device_status, (void*)programmingthread)) {
			logMessage(LOG_ERR, "could not create thread\n");
			return;
		}
		break;
	case AQ_PDA_DEVICE_ON_OFF:
		if (thrd_success != thrd_create(&programmingthread->thread_id, set_aqualink_PDA_device_on_off, (void*)programmingthread)) {
			logMessage(LOG_ERR, "could not create thread\n");
			return;
		}
		break;
	case AQ_GET_AUX_LABELS:
		if (thrd_success != thrd_create(&programmingthread->thread_id, get_aqualink_aux_labels, (void*)programmingthread)) {
			logMessage(LOG_ERR, "could not create thread\n");
			return;
		}
		break;
	case AQ_SET_BOOST:
		if (thrd_success != thrd_create(&programmingthread->thread_id, set_aqualink_boost, (void*)programmingthread)) {
			logMessage(LOG_ERR, "could not create thread\n");
			return;
		}
		break;
	default:
		logMessage(LOG_ERR, "Don't understand thread type\n");
		break;
	}

	if (programmingthread->thread_id != 0) {
		thrd_detach(programmingthread->thread_id);
	}
	else {

	}
}


void waitForSingleThreadOrTerminate(struct programmingThreadCtrl* threadCtrl, program_type type)
{
	int tries = 120;
	static int waitTime = 1;
	int i = 0;

	i = 0;
	while (get_aq_cmd_length() > 0 && (i++ <= tries)) {
		logMessage(LOG_DEBUG, "Thread %p (%s) sleeping, waiting command queue to empty\n", &threadCtrl->thread_id, ptypeName(type));
		sleep(waitTime);
	}
	if (i >= tries) {
		logMessage(LOG_ERR, "Thread %p (%s) timeout waiting, ending\n", &threadCtrl->thread_id, ptypeName(type));
		free(threadCtrl);
		thrd_exit(0);
	}

	while ((threadCtrl->aq_data->active_thread.thread_id != 0) && (i++ <= tries)) 
	{
		//logMessage (LOG_DEBUG, "Thread %d sleeping, waiting for thread %d to finish\n", threadCtrl->thread_id, threadCtrl->aq_data->active_thread.thread_id);
		logMessage(LOG_DEBUG, "Thread %p (%s) sleeping, waiting for thread %p (%s) to finish\n",
			&threadCtrl->thread_id, ptypeName(type),
			threadCtrl->aq_data->active_thread.thread_id, ptypeName(threadCtrl->aq_data->active_thread.ptype));
		sleep(waitTime);
	}

	if (i >= tries) 
	{
		//logMessage (LOG_ERR, "Thread %d timeout waiting, ending\n",threadCtrl->thread_id);
		logMessage(LOG_ERR, "Thread %d,%p timeout waiting for thread %d,%p to finish\n",
			type, &threadCtrl->thread_id, threadCtrl->aq_data->active_thread.ptype,
			threadCtrl->aq_data->active_thread.thread_id);
		free(threadCtrl);
		thrd_exit(0);
	}

	// Clear out any messages to the UI.
	threadCtrl->aq_data->last_display_message[0] = '\0';
	threadCtrl->aq_data->active_thread.thread_id = &threadCtrl->thread_id;
	threadCtrl->aq_data->active_thread.ptype = type;

#ifdef AQ_DEBUG
	clock_gettime(CLOCK_REALTIME, &threadCtrl->aq_data->start_active_time);
#endif

	//logMessage (LOG_DEBUG, "Thread %d is active\n", threadCtrl->aq_data->active_thread.thread_id);
	logMessage(LOG_DEBUG, "Thread %d,%p is active (%s)\n",
		threadCtrl->aq_data->active_thread.ptype,
		threadCtrl->aq_data->active_thread.thread_id,
		ptypeName(threadCtrl->aq_data->active_thread.ptype));
}

void cleanAndTerminateThread(struct programmingThreadCtrl* threadCtrl)
{
#ifndef AQ_DEBUG
	logMessage(LOG_DEBUG, "Thread %d,%p (%s) finished\n", threadCtrl->aq_data->active_thread.ptype, threadCtrl->thread_id, ptypeName(threadCtrl->aq_data->active_thread.ptype));
#else
	struct timespec elapsed;
	clock_gettime(CLOCK_REALTIME, &threadCtrl->aq_data->last_active_time);
	timespec_subtract(&elapsed, &threadCtrl->aq_data->last_active_time, &threadCtrl->aq_data->start_active_time);
	logMessage(LOG_DEBUG, "Thread %d,%p (%s) finished in %d.%03ld sec\n",
		threadCtrl->aq_data->active_thread.ptype,
		threadCtrl->aq_data->active_thread.thread_id,
		ptypeName(threadCtrl->aq_data->active_thread.ptype),
		elapsed.tv_sec, elapsed.tv_nsec / 1000000L);
#endif

	// Quick delay to allow for last message to be sent.
	delayMicroseconds(500);
	threadCtrl->aq_data->active_thread.thread_id = 0;
	threadCtrl->aq_data->active_thread.ptype = AQP_NULL;
	threadCtrl->thread_id = 0;
	free(threadCtrl);
	thrd_exit(0);
}

bool setAqualinkNumericField_new(struct aqualinkdata* aq_data, char* value_label, int value, int increment);

bool setAqualinkNumericField(struct aqualinkdata* aq_data, char* value_label, int value)
{
	return setAqualinkNumericField_new(aq_data, value_label, value, 1);
}

bool setAqualinkNumericField_new(struct aqualinkdata* aq_data, char* value_label, int value, int increment)
{
	logMessage(LOG_DEBUG, "Setting menu item '%s' to %d\n", value_label, value);
	//char leading[10];  // description of the field (POOL, SPA, FRZ)
	int current_val = -1;        // integer value of the current set point
	//char trailing[10];      // the degrees and scale
	char searchBuf[20];

	sprintf(searchBuf, "^%s", value_label);
	int val_len = strlen(value_label);
	int i = 0;
	do
	{
		if (waitForMessage(aq_data, searchBuf, 4) != true) {
			logMessage(LOG_WARNING, "AQ_Programmer Could not set numeric input '%s', not found\n", value_label);
			cancel_menu();
			return false;
		}
		sscanf(&aq_data->last_message[val_len], "%*[^0123456789]%d", &current_val);
		logMessage(LOG_DEBUG, "%s set to %d, looking for %d\n", value_label, current_val, value);

		if (value > current_val) {
			// Increment the field.
			sprintf(searchBuf, "%s %d", value_label, current_val + increment);
			send_cmd(KEY_RIGHT);
		}
		else if (value < current_val) {
			// Decrement the field.
			sprintf(searchBuf, "%s %d", value_label, current_val - increment);
			send_cmd(KEY_LEFT);
		}
		else {
			// Just send ENTER. We are at the right value.
			sprintf(searchBuf, "%s %d", value_label, current_val);
			send_cmd(KEY_ENTER);
		}

		if (i++ >= 100) {
			logMessage(LOG_WARNING, "AQ_Programmer Could not set numeric input '%s', to '%d'\n", value_label, value);
			break;
		}
	} while (value != current_val);

	return true;
}

bool OLD_setAqualinkNumericField_OLD(struct aqualinkdata* aq_data, char* value_label, int value)
{ // Works for everything but not SWG
	logMessage(LOG_DEBUG, "Setting menu item '%s' to %d\n", value_label, value);
	char leading[10];  // description of the field (POOL, SPA, FRZ)
	int current_val;        // integer value of the current set point
	char trailing[10];      // the degrees and scale
	char searchBuf[20];

	sprintf(searchBuf, "^%s", value_label);

	do
	{
		if (waitForMessage(aq_data, searchBuf, 3) != true) {
			logMessage(LOG_WARNING, "AQ_Programmer Could not set numeric input '%s', not found\n", value_label);
			cancel_menu();
			return false;
		}
		//logMessage(LOG_DEBUG,"WAITING for kick value=%d\n",current_val);     
		sscanf(aq_data->last_message, "%s %d%s", leading, &current_val, trailing);
		logMessage(LOG_DEBUG, "%s set to %d, looking for %d\n", value_label, current_val, value);

		if (value > current_val) {
			// Increment the field.
			sprintf(searchBuf, "%s %d", value_label, current_val + 1);
			send_cmd(KEY_RIGHT);
		}
		else if (value < current_val) {
			// Decrement the field.
			sprintf(searchBuf, "%s %d", value_label, current_val - 1);
			send_cmd(KEY_LEFT);
		}
		else {
			// Just send ENTER. We are at the right value.
			sprintf(searchBuf, "%s %d", value_label, current_val);
			send_cmd(KEY_ENTER);
		}
	} while (value != current_val);

	return true;
}

int set_aqualink_boost(void* ptr)
{
	struct programmingThreadCtrl* threadCtrl;
	threadCtrl = (struct programmingThreadCtrl*) ptr;
	struct aqualinkdata* aq_data = threadCtrl->aq_data;

	waitForSingleThreadOrTerminate(threadCtrl, AQ_SET_BOOST);
	/*
	menu
  <find menu>
  BOOST POOL
  <wait 2 messages>
  PRESS ENTER* TO START BOOST POOL
  <press enter>


  <Menu when in boost>
  BOOST POOL 23:59 REMAINING


  menu
  <find menu>
  BOOST POOL
  <find menu>
  STOP BOOST POOL
  <press enter>
   */
	int val = atoi((char*)threadCtrl->thread_args);

	if (pda_mode() == true) {
		set_PDA_aqualink_boost(aq_data, val);
		cleanAndTerminateThread(threadCtrl);
		return 0;
	}

	logMessage(LOG_DEBUG, "programming BOOST to %s\n", val == true ? "On" : "Off");

	if (select_menu_item(aq_data, "BOOST POOL") != true) {
		logMessage(LOG_WARNING, "Could not select BOOST POOL menu\n");
		cancel_menu();
		cleanAndTerminateThread(threadCtrl);
		return 0;
	}

	if (val == true) {
		waitForMessage(threadCtrl->aq_data, "TO START BOOST POOL", 5);
		send_cmd(KEY_ENTER);
		waitfor_queue2empty();
	}
	else {
		int wait_messages = 5;
		int i = 0;
		while (i++ < wait_messages)
		{
			waitForMessage(aq_data, "STOP BOOST POOL", 1);
			if (stristr(aq_data->last_message, "STOP BOOST POOL") != NULL) {
				// This is a really bad hack, message sequence is out for boost for some reason, so as soon as we see stop message, force enter key.
				_pgm_command = KEY_ENTER;
				logMessage(LOG_DEBUG, "**** FOUND STOP BOOST POOL ****\n");
				break;
			}
			else {
				logMessage(LOG_DEBUG, "Find item in Menu: loop %d of %d looking for 'STOP BOOST POOL' received message '%s'\n", i, wait_messages, aq_data->last_message);
				delayMicroseconds(200);
				if (stristr(aq_data->last_message, "STOP BOOST POOL") != NULL) {
					_pgm_command = KEY_ENTER;
					logMessage(LOG_DEBUG, "**** FOUND STOP BOOST POOL ****\n");
					break;
				}
				send_cmd(KEY_RIGHT);
				waitfor_queue2empty();
			}
		}

		waitForMessage(aq_data, "STOP BOOST POOL", 1);
		if (stristr(aq_data->last_message, "STOP BOOST POOL") != NULL) {
			send_cmd(KEY_ENTER);
		}
		else {
			logMessage(LOG_DEBUG, "**** GIVING UP ****\n");
		}
	}
	waitForMessage(aq_data, NULL, 1);

	cleanAndTerminateThread(threadCtrl);
	return 0;
}


int set_aqualink_SWG(void* ptr)
{
	struct programmingThreadCtrl* threadCtrl;
	threadCtrl = (struct programmingThreadCtrl*) ptr;
	struct aqualinkdata* aq_data = threadCtrl->aq_data;

	waitForSingleThreadOrTerminate(threadCtrl, AQ_SET_SWG_PERCENT);

	int val = atoi((char*)threadCtrl->thread_args);
	val = setpoint_check(SWG_SETPOINT, val, aq_data);

	if (pda_mode() == true) {
		set_PDA_aqualink_SWG_setpoint(aq_data, val);
		cleanAndTerminateThread(threadCtrl);
		return 0;
	}

	logMessage(LOG_DEBUG, "programming SWG percent to %d\n", val);

	if (select_menu_item(aq_data, "SET AQUAPURE") != true) {
		logMessage(LOG_WARNING, "Could not select SET AQUAPURE menu\n");
		cancel_menu();
		cleanAndTerminateThread(threadCtrl);
		return 0;
	}

	// If spa is on, set SWG for spa, if not set SWG for pool
	if (aq_data->aqbuttons[SPA_INDEX].led->state != OFF) {
		if (select_sub_menu_item(aq_data, "SET SPA SP") != true) {
			logMessage(LOG_WARNING, "Could not select SWG setpoint menu for SPA\n");
			cancel_menu();
			cleanAndTerminateThread(threadCtrl);
			return 0;
		}
		setAqualinkNumericField_new(aq_data, "SPA SP", val, 5);
	}
	else {
		if (select_sub_menu_item(aq_data, "SET POOL SP") != true) {
			logMessage(LOG_WARNING, "Could not select SWG setpoint menu\n");
			cancel_menu();
			cleanAndTerminateThread(threadCtrl);
			return 0;
		}
		setAqualinkNumericField_new(aq_data, "POOL SP", val, 5);
	}

	// usually miss this message, not sure why, but wait anyway to make sure programming has ended
	// NSF have see the below message RS Message :-
	// 'Pool set to 20%'
	// 'POOL SP IS SET TO 20%'
	waitForMessage(threadCtrl->aq_data, "SET TO", 1);
	//waitForMessage(threadCtrl->aq_data, "POOL SP IS SET TO", 1);

	// Since we read % directly from RS message, wait for another few messages that way
	// We won't registed a SWG bounce, since we already told clients SWG was at new % before programming started
	waitForMessage(threadCtrl->aq_data, NULL, 1);

	cleanAndTerminateThread(threadCtrl);
	return 0;
}



int get_aqualink_aux_labels(void* ptr)
{
	struct programmingThreadCtrl* threadCtrl;
	threadCtrl = (struct programmingThreadCtrl*) ptr;
	struct aqualinkdata* aq_data = threadCtrl->aq_data;

	waitForSingleThreadOrTerminate(threadCtrl, AQ_GET_AUX_LABELS);

	if (pda_mode() == true) {
		get_PDA_aqualink_aux_labels(aq_data);
		cleanAndTerminateThread(threadCtrl);
		return 0;
	}

	if (select_menu_item(aq_data, "REVIEW") != true) {
		logMessage(LOG_WARNING, "Could not select REVIEW menu\n");
		cancel_menu();
		cleanAndTerminateThread(threadCtrl);
		return 0;
	}

	if (select_sub_menu_item(aq_data, "AUX LABELS") != true) {
		logMessage(LOG_WARNING, "Could not select AUX LABELS menu\n");
		cancel_menu();
		cleanAndTerminateThread(threadCtrl);
		return 0;
	}

	waitForMessage(aq_data, NULL, 5); // Receive 5 messages

	cleanAndTerminateThread(threadCtrl);
	return 0;
}

int set_aqualink_light_colormode(void* ptr)
{
	int i;
	struct programmingThreadCtrl* threadCtrl;
	threadCtrl = (struct programmingThreadCtrl*) ptr;
	struct aqualinkdata* aq_data = threadCtrl->aq_data;

	waitForSingleThreadOrTerminate(threadCtrl, AQ_SET_COLORMODE);

	char* buf = (char*)threadCtrl->thread_args;
	int val = atoi(&buf[0]);
	int btn = atoi(&buf[5]);
	int iOn = atoi(&buf[10]);
	int iOff = atoi(&buf[15]);
	float pmode = atof(&buf[20]);

	if (btn < 0 || btn >= TOTAL_BUTTONS) {
		logMessage(LOG_ERR, "Can't program light mode on button %d\n", btn);
		cleanAndTerminateThread(threadCtrl);
		return 0;
	}

	aqkey* button = &aq_data->aqbuttons[btn];
	unsigned char code = button->code;

	logMessage(LOG_NOTICE, "Light Programming #: %d, on button: %s, with pause mode: %f (initial on=%d, initial off=%d)\n", val, button->label, pmode, iOn, iOff);

	// Simply turn the light off if value is 0
	if (val <= 0) {
		if (button->led->state == ON) {
			send_cmd(code);
		}
		cleanAndTerminateThread(threadCtrl);
		return 0;
	}

	int seconds = 1000;
	// Needs to start programming sequence with light on, if off we need to turn on for 15 seconds
	// before we can send the next off.
	if (button->led->state != ON) {
		logMessage(LOG_INFO, "Light Programming Initial state off, turning on for %d seconds\n", iOn);
		send_cmd(code);
		delayMicroseconds(iOn * seconds);
	}

	logMessage(LOG_INFO, "Light Programming turn off for %d seconds\n", iOff);
	// Now need to turn off for between 11 and 14 seconds to reset light.
	send_cmd(code);
	delayMicroseconds(iOff * seconds);

	// Now light is reset, pulse the appropiate number of times to advance program.
	logMessage(LOG_INFO, "Light Programming button pulsing on/off %d times\n", val);

	// Program light in safe mode (slowley), or quick mode
	if (pmode > 0) {
		for (i = 1; i < (val * 2); i++) {
			logMessage(LOG_INFO, "Light Programming button press number %d - %s of %d\n", i, i % 2 == 0 ? "Off" : "On", val);
			send_cmd(code);
			delayMicroseconds(pmode * seconds); // 0.3 works, but using 0.4 to be safe
		}
	}
	else {
		for (i = 1; i < val; i++) {
			logMessage(LOG_INFO, "Light Programming button press number %d - %s of %d\n", i, "ON", val);
			send_cmd(code);
			waitForButtonState(aq_data, button, ON, 2);
			logMessage(LOG_INFO, "Light Programming button press number %d - %s of %d\n", i, "OFF", val);
			send_cmd(code);
			waitForButtonState(aq_data, button, OFF, 2);
		}

		logMessage(LOG_INFO, "Finished - Light Programming button press number %d - %s of %d\n", i, "ON", val);
		send_cmd(code);
	}

	cleanAndTerminateThread(threadCtrl);
	return 0;
}
int set_aqualink_pool_heater_temps(void* ptr)
{
	struct programmingThreadCtrl* threadCtrl;
	threadCtrl = (struct programmingThreadCtrl*) ptr;
	struct aqualinkdata* aq_data = threadCtrl->aq_data;
	char* name;
	char* menu_name;
	waitForSingleThreadOrTerminate(threadCtrl, AQ_SET_POOL_HEATER_TEMP);

	int val = atoi((char*)threadCtrl->thread_args);
	/*
	if (val > HEATER_MAX) {
	  val = HEATER_MAX;
	} else if ( val < MEATER_MIN) {
	  val = MEATER_MIN;
	}
	*/
	val = setpoint_check(POOL_HTR_SETOINT, val, aq_data);

	if (pda_mode() == true) {
		set_PDA_aqualink_heater_setpoint(aq_data, val, true);
		cleanAndTerminateThread(threadCtrl);
		return 0;
	}

	// NSF IF in TEMP1 / TEMP2 mode, we need C range of 1 to 40 is 2 to 40 for TEMP1, 1 to 39 TEMP2
	if (aq_data->single_device == true) {
		name = "TEMP1";
		menu_name = "SET TEMP1";
	}
	else {
		name = "POOL";
		menu_name = "SET POOL TEMP";
	}

	logMessage(LOG_DEBUG, "Setting pool heater setpoint to %d\n", val);

	if (select_menu_item(aq_data, "SET TEMP") != true) {
		logMessage(LOG_WARNING, "Could not select SET TEMP menu\n");
		cancel_menu();
		cleanAndTerminateThread(threadCtrl);
		return 0;
	}

	//if (select_sub_menu_item(aq_data, "SET POOL TEMP") != true) {
	if (select_sub_menu_item(aq_data, menu_name) != true) {
		logMessage(LOG_WARNING, "Could not select SET POOL TEMP menu\n");
		cancel_menu();
		cleanAndTerminateThread(threadCtrl);
		return 0;
	}

	if (aq_data->single_device == true) {
		// Need to get pass this message 'TEMP1 MUST BE SET HIGHER THAN TEMP2'
		// and get this message 'TEMP1 20�C ~*'
		// Before going to numeric field.
		waitForMessage(threadCtrl->aq_data, "MUST BE SET", 5);
		send_cmd(KEY_LEFT);
		while (stristr(aq_data->last_message, "MUST BE SET") != NULL) {
			delayMicroseconds(500);
		}
	}

	//setAqualinkNumericField(aq_data, "POOL", val);
	setAqualinkNumericField(aq_data, name, val);

	// usually miss this message, not sure why, but wait anyway to make sure programming has ended
	waitForMessage(threadCtrl->aq_data, "POOL TEMP IS SET TO", 1);

	cleanAndTerminateThread(threadCtrl);
	return 0;
}
int set_aqualink_spa_heater_temps(void* ptr)
{
	struct programmingThreadCtrl* threadCtrl;
	threadCtrl = (struct programmingThreadCtrl*) ptr;
	struct aqualinkdata* aq_data = threadCtrl->aq_data;

	waitForSingleThreadOrTerminate(threadCtrl, AQ_SET_SPA_HEATER_TEMP);

	int val = atoi((char*)threadCtrl->thread_args);
	char* name;
	char* menu_name;
	
	val = setpoint_check(SPA_HTR_SETOINT, val, aq_data);

	if (pda_mode() == true) {
		set_PDA_aqualink_heater_setpoint(aq_data, val, false);
		cleanAndTerminateThread(threadCtrl);
		return 0;
	}

	// NSF IF in TEMP1 / TEMP2 mode, we need C range of 1 to 40 is 2 to 40 for TEMP1, 1 to 39 TEMP2

	if (aq_data->single_device == true) {
		name = "TEMP2";
		menu_name = "SET TEMP2";
	}
	else {
		name = "SPA";
		menu_name = "SET SPA TEMP";
	}

	logMessage(LOG_DEBUG, "Setting spa heater setpoint to %d\n", val);

	if (select_menu_item(aq_data, "SET TEMP") != true) {
		logMessage(LOG_WARNING, "Could not select SET TEMP menu\n");
		cancel_menu();
		cleanAndTerminateThread(threadCtrl);
		return 0;
	}

	if (select_sub_menu_item(aq_data, menu_name) != true) {
		logMessage(LOG_WARNING, "Could not select SET SPA TEMP menu\n");
		cancel_menu();
		cleanAndTerminateThread(threadCtrl);
		return 0;
	}

	if (aq_data->single_device == true) {
		// Need to get pass this message 'TEMP2 MUST BE SET LOWER THAN TEMP1'
		// and get this message 'TEMP2 20�C ~*'
		// Before going to numeric field.
		waitForMessage(threadCtrl->aq_data, "MUST BE SET", 5);
		send_cmd(KEY_LEFT);
		while (stristr(aq_data->last_message, "MUST BE SET") != NULL) {
			delayMicroseconds(500);
		}
	}

	setAqualinkNumericField(aq_data, name, val);

	// usually miss this message, not sure why, but wait anyway to make sure programming has ended
	waitForMessage(threadCtrl->aq_data, "SPA TEMP IS SET TO", 1);

	cleanAndTerminateThread(threadCtrl);
	return 0;
}

int set_aqualink_freeze_heater_temps(void* ptr)
{
	struct programmingThreadCtrl* threadCtrl;
	threadCtrl = (struct programmingThreadCtrl*) ptr;
	struct aqualinkdata* aq_data = threadCtrl->aq_data;

	waitForSingleThreadOrTerminate(threadCtrl, AQ_SET_FRZ_PROTECTION_TEMP);

	int val = atoi((char*)threadCtrl->thread_args);

	val = setpoint_check(FREEZE_SETPOINT, val, aq_data);

	logMessage(LOG_DEBUG, "Setting sfreeze protection to %d\n", val);

	if (pda_mode() == true) {
		set_PDA_aqualink_freezeprotect_setpoint(aq_data, val);
		cleanAndTerminateThread(threadCtrl);
		return 0;
	}

	//setAqualinkTemp(aq_data, "SYSTEM SETUP", "FRZ PROTECT", "TEMP SETTING", "FRZ", val);
	if (select_menu_item(aq_data, "SYSTEM SETUP") != true) {
		logMessage(LOG_WARNING, "Could not select SYSTEM SETUP menu\n");
		cancel_menu();
		cleanAndTerminateThread(threadCtrl);
		return 0;
	}

	if (select_sub_menu_item(aq_data, "FRZ PROTECT") != true) {
		logMessage(LOG_WARNING, "Could not select FRZ PROTECT menu\n");
		cancel_menu();
		cleanAndTerminateThread(threadCtrl);
		return 0;
	}

	if (select_sub_menu_item(aq_data, "TEMP SETTING") != true) {
		logMessage(LOG_WARNING, "Could not select TEMP SETTING menu\n");
		cancel_menu();
		cleanAndTerminateThread(threadCtrl);
		return 0;
	}

	setAqualinkNumericField(aq_data, "FRZ", val);
	 
	waitForMessage(threadCtrl->aq_data, "FREEZE PROTECTION IS SET TO", 3);

	cleanAndTerminateThread(threadCtrl);
	return 0;
}

int set_aqualink_time(void* ptr)
{
	struct programmingThreadCtrl* threadCtrl;
	threadCtrl = (struct programmingThreadCtrl*) ptr;
	struct aqualinkdata* aq_data = threadCtrl->aq_data;

	waitForSingleThreadOrTerminate(threadCtrl, AQ_SET_TIME);
	logMessage(LOG_NOTICE, "Setting time on aqualink\n");

	time_t now = time(0);   // get time now
	struct tm* result = localtime(&now);
	char hour[11];

	if (result->tm_hour == 0)
	{
		sprintf(hour, "HOUR 12 AM");
	}
	else if (result->tm_hour < 11)
	{
		sprintf(hour, "HOUR %d AM", result->tm_hour);
	}
	else if (result->tm_hour == 12)
	{
		sprintf(hour, "HOUR 12 PM");
	}
	else // Must be 13 or more
	{
		sprintf(hour, "HOUR %d PM", result->tm_hour - 12);
	}

	logMessage(LOG_DEBUG, "Setting time to %d/%d/%d %d:%d\n", result->tm_mon + 1, result->tm_mday, result->tm_year + 1900, result->tm_hour + 1, result->tm_min);

	if (select_menu_item(aq_data, "SET TIME") != true) {
		logMessage(LOG_WARNING, "Could not select SET TIME menu\n");
		cancel_menu();
		cleanAndTerminateThread(threadCtrl);
		return 0;
	}

	setAqualinkNumericField(aq_data, "YEAR", result->tm_year + 1900);
	setAqualinkNumericField(aq_data, "MONTH", result->tm_mon + 1);
	setAqualinkNumericField(aq_data, "DAY", result->tm_mday);
	select_sub_menu_item(aq_data, hour);
	setAqualinkNumericField(aq_data, "MINUTE", result->tm_min);

	send_cmd(KEY_ENTER);

	cleanAndTerminateThread(threadCtrl);
	return 0;
}

int get_aqualink_diag_model(void* ptr)
{
	struct programmingThreadCtrl* threadCtrl;
	threadCtrl = (struct programmingThreadCtrl*) ptr;
	struct aqualinkdata* aq_data = threadCtrl->aq_data;

	waitForSingleThreadOrTerminate(threadCtrl, AQ_GET_DIAGNOSTICS_MODEL);

	if (select_menu_item(aq_data, "SYSTEM SETUP") != true) {
		logMessage(LOG_WARNING, "Could not select HELP menu\n");
		cancel_menu();
		cleanAndTerminateThread(threadCtrl);
		return 0;
	}

	if (select_sub_menu_item(aq_data, "DIAGNOSTICS") != true) {
		logMessage(LOG_WARNING, "Could not select DIAGNOSTICS menu\n");
		cancel_menu();
		cleanAndTerminateThread(threadCtrl);
		return 0;
	}

	waitForMessage(aq_data, NULL, 8); // Receive 8 messages
	//8157 REV MMM | BATTERY OK | Cal:  -27  0  6 | CONTROL PANEL #1 | CONTROL PANEL #3 | WATER SENSOR OK | AIR SENSOR OK | SOLAR SENSOR OPENED
	cleanAndTerminateThread(threadCtrl);
	return 0;
}

int get_aqualink_pool_spa_heater_temps(void* ptr)
{
	//logMessage(LOG_DEBUG, "Could not select TEMP SET menu\n");
	struct programmingThreadCtrl* threadCtrl;
	threadCtrl = (struct programmingThreadCtrl*) ptr;
	struct aqualinkdata* aq_data = threadCtrl->aq_data;

	waitForSingleThreadOrTerminate(threadCtrl, AQ_GET_POOL_SPA_HEATER_TEMPS);
	logMessage(LOG_NOTICE, "Getting pool & spa heat setpoints from aqualink\n");

	if (pda_mode() == true) {
		if (!get_PDA_aqualink_pool_spa_heater_temps(aq_data)) {
			logMessage(LOG_ERR, "Error Getting PDA pool & spa heat protection setpoints\n");
		}
		cleanAndTerminateThread(threadCtrl);
		return 0;
	}

	if (select_menu_item(aq_data, "REVIEW") != true) {
		logMessage(LOG_WARNING, "Could not select REVIEW menu\n");
		cancel_menu();
		cleanAndTerminateThread(threadCtrl);
		return 0;
	}

	if (select_sub_menu_item(aq_data, "TEMP SET") != true) {
		logMessage(LOG_WARNING, "Could not select TEMP SET menu\n");
		cancel_menu();
		cleanAndTerminateThread(threadCtrl);
		return 0;
	}

	// Should receive 'POOL TEMP IS SET TO xx' then 'SPA TEMP IS SET TO xx' then 'MAINTAIN TEMP IS (OFF|ON)'
	// wait for the last message
	waitForMessage(threadCtrl->aq_data, "MAINTAIN TEMP IS", 5);

	//cancel_menu(threadCtrl->aq_data);
	cleanAndTerminateThread(threadCtrl);
	return 0;
}

int get_freeze_protect_temp(void* ptr)
{
	struct programmingThreadCtrl* threadCtrl;
	threadCtrl = (struct programmingThreadCtrl*) ptr;
	struct aqualinkdata* aq_data = threadCtrl->aq_data;

	waitForSingleThreadOrTerminate(threadCtrl, AQ_GET_FREEZE_PROTECT_TEMP);
	logMessage(LOG_NOTICE, "Getting freeze protection setpoints\n");


	if (pda_mode() == true) {
		if (!get_PDA_freeze_protect_temp(aq_data)) {
			logMessage(LOG_ERR, "Error Getting PDA freeze protection setpoints\n");
		}
		cleanAndTerminateThread(threadCtrl);
		return 0;
	}

	if (select_menu_item(aq_data, "REVIEW") != true) {
		logMessage(LOG_WARNING, "Could not select REVIEW menu\n");
		cancel_menu();
		cleanAndTerminateThread(threadCtrl);
		return 0;
	}

	if (select_sub_menu_item(aq_data, "FRZ PROTECT") != true) {
		logMessage(LOG_WARNING, "Could not select FRZ PROTECT menu\n");
		cancel_menu();
		cleanAndTerminateThread(threadCtrl);
		return 0;
	}

	waitForMessage(aq_data, "FREEZE PROTECTION IS SET TO", 6); // Changed from 3 to wait for multiple items to be listed

	cleanAndTerminateThread(threadCtrl);
	return 0;
}

bool get_aqualink_program_for_button(struct aqualinkdata* aq_data, unsigned char cmd)
{
	int rtnStringsWait = 7;

	if (!waitForMessage(aq_data, "SELECT DEVICE TO REVIEW or PRESS ENTER TO END", rtnStringsWait))
		return false;

	logMessage(LOG_DEBUG, "Send key '%d'\n", cmd);
	send_cmd(cmd);
	return waitForEitherMessage(aq_data, "NOT SET", "TURNS ON", rtnStringsWait);
}

int get_aqualink_programs(void* ptr)
{
	struct programmingThreadCtrl* threadCtrl;
	threadCtrl = (struct programmingThreadCtrl*) ptr;
	struct aqualinkdata* aq_data = threadCtrl->aq_data;
	char keys[10] = { KEY_PUMP, KEY_SPA, KEY_AUX1, KEY_AUX2, KEY_AUX3, KEY_AUX4, KEY_AUX5 }; // KEY_AUX6 & KEY_AUX7 kill programming mode

	waitForSingleThreadOrTerminate(threadCtrl, AQ_GET_PROGRAMS);

	if (select_menu_item(aq_data, "REVIEW") != true) {
		cancel_menu();
		cleanAndTerminateThread(threadCtrl);
		return 0;
	}

	if (select_sub_menu_item(aq_data, "PROGRAMS") != true) {
		cancel_menu();
		cleanAndTerminateThread(threadCtrl);
		return 0;
	}

	unsigned int i;

	for (i = 0; i < strlen(keys); i++) {
		if (!get_aqualink_program_for_button(aq_data, keys[i])) {
			cleanAndTerminateThread(threadCtrl);
			return 0;
		}
	}


	if (waitForMessage(aq_data, "SELECT DEVICE TO REVIEW or PRESS ENTER TO END", 6)) {
		send_cmd(KEY_ENTER);
	}
	else {
		cancel_menu();
	}

	cleanAndTerminateThread(threadCtrl);
	return 0;
}

void waitfor_queue2empty()
{
	int i = 0;

	while ((_pgm_command != NUL) && (i++ < 20)) {
		//sleep(1); // NSF Change to smaller time.
		//logMessage(LOG_DEBUG, "********  QUEUE IS FULL ********  delay\n");
		delayMicroseconds(50);
	}

	if (_pgm_command != NUL) {
		if (pda_mode()) {
			// Wait for longer in PDA mode since it's slower.
			while ((_pgm_command != NUL) && (i++ < 100)) {
				delayMicroseconds(100);
			}
		}
		logMessage(LOG_WARNING, "Send command Queue did not empty, timeout\n");
	}

}

void send_cmd(unsigned char cmd)
{
	waitfor_queue2empty();

	_pgm_command = cmd;

	logMessage(LOG_INFO, "Queue send '0x%02hhx' to controller (programming)\n", _pgm_command);
}

void cancel_menu()
{
	send_cmd(KEY_CANCEL);
}

/*
* added functionality, if start of string is ^ use that as must start with in comparison
*/

bool waitForEitherMessage(struct aqualinkdata* aq_data, char* message1, char* message2, int numMessageReceived)
{
	//logMessage(LOG_DEBUG, "waitForMessage %s %d %d\n",message,numMessageReceived,cmd);
	waitfor_queue2empty();  // MAke sure the last command was sent
	int i = 0;
	mtx_lock(&aq_data->active_thread.thread_mutex);
	char* msgS1 = "";
	char* msgS2 = "";
	char* ptr = NULL;


	if (message1 != NULL) {
		if (message1[0] == '^')
		{
			msgS1 = &message1[1];
		}
		else
		{
			msgS1 = message1;
		}
	}
	if (message2 != NULL) {
		if (message2[0] == '^')
		{
			msgS2 = &message2[1];
		}
		else
		{
			msgS2 = message2;
		}
	}

	while (++i <= numMessageReceived)
	{
		logMessage(LOG_DEBUG, "Programming mode: loop %d of %d looking for '%s' OR '%s' received message1 '%s'\n", i, numMessageReceived, message1, message2, aq_data->last_message);

		if (message1 != NULL) {
			ptr = stristr(aq_data->last_message, msgS1);
			if (ptr != NULL) { // match
				logMessage(LOG_DEBUG, "Programming mode: String MATCH '%s'\n", msgS1);
				if ((msgS1 == message1) || (ptr == aq_data->last_message))
				{
					// (a) match & don't care if first char
					// (b) // match & do care if first char
					break;
				}
			}
		}
		if (message2 != NULL) {
			ptr = stristr(aq_data->last_message, msgS2);
			if (ptr != NULL) { // match
				logMessage(LOG_DEBUG, "Programming mode: String MATCH '%s'\n", msgS2);
				if (msgS2 == message2) // match & don't care if first char
				{
					break;
				}
				else if (ptr == aq_data->last_message) // match & do care if first char
				{
					break;
				}
			}
		}

		cnd_wait(&aq_data->active_thread.thread_cond, &aq_data->active_thread.thread_mutex);
	}

	mtx_unlock(&aq_data->active_thread.thread_mutex);

	if (message1 != NULL && message2 != NULL && ptr == NULL) {
		//logmessage1(LOG_ERR, "Could not select MENU of Aqualink control panel\n");
		logMessage(LOG_DEBUG, "Programming mode: did not find '%s'\n", message1);
		return false;
	}
	logMessage(LOG_DEBUG, "Programming mode: found message1 '%s' or '%s' in '%s'\n", message1, message2, aq_data->last_message);

	return true;
}



bool waitForMessage(struct aqualinkdata* aq_data, char* message, int numMessageReceived)
{
	logMessage(LOG_DEBUG, "waitForMessage %s %d\n", message, numMessageReceived);
	waitfor_queue2empty();  // MAke sure the last command was sent

	int i = 0;
	mtx_lock(&aq_data->active_thread.thread_mutex);
	char* msgS;
	char* ptr = NULL;

	if (message != NULL) {
		if (message[0] == '^')
		{
			msgS = &message[1];
		}
		else
		{
			msgS = message;
		}
	}

	while (++i <= numMessageReceived)
	{
		if (message != NULL)
		{
			logMessage(LOG_DEBUG, "Programming mode: loop %d of %d looking for '%s', last message received '%s'\n", i, numMessageReceived, message, aq_data->last_message);
		}
		else
		{
			logMessage(LOG_DEBUG, "Programming mode: loop %d of %d waiting for next message, last message received '%s'\n", i, numMessageReceived, aq_data->last_message);
		}

		if (message != NULL) {
			ptr = stristr(aq_data->last_message, msgS);
			if (ptr != NULL) { // match
				logMessage(LOG_DEBUG, "Programming mode: String MATCH\n");
				if (msgS == message) // match & don't care if first char
				{
					break;
				}
				else if (ptr == aq_data->last_message) // match & do care if first char
				{
					break;
				}
			}
		}

		cnd_wait(&aq_data->active_thread.thread_cond, &aq_data->active_thread.thread_mutex);
	}

	mtx_unlock(&aq_data->active_thread.thread_mutex);

	if (message != NULL && ptr == NULL) {
		logMessage(LOG_DEBUG, "Programming mode: did not find '%s'\n", message);
		return false;
	}
	else if (message != NULL)
	{
		logMessage(LOG_DEBUG, "Programming mode: found message '%s' in '%s'\n", message, aq_data->last_message);
	}
	else
	{
		logMessage(LOG_DEBUG, "Programming mode: waited for %d message(s)\n", numMessageReceived);
	}

	return true;
}

bool select_menu_item(struct aqualinkdata* aq_data, char* item_string)
{
	char* expectedMsg = "PRESS ENTER* TO SELECT";
	//char* expectedMsg = "PROGRAM";
	int wait_messages = 5;
	bool found = false;
	int tries = 0;
	// Select the MENU and wait to get the RS8 respond.

	while (found == false && tries <= 3) {
		send_cmd(KEY_MENU);
		found = waitForMessage(aq_data, expectedMsg, wait_messages);
		tries++;
	}

	if (found == false)
	{
		return false;
	}

	// NSF  This isn't needed and seems to cause issue on some controllers.
	return select_sub_menu_item(aq_data, item_string);
}

bool select_sub_menu_item(struct aqualinkdata* aq_data, char* item_string)
{
	int wait_messages = 25;
	int i = 0;

	while ((stristr(aq_data->last_message, item_string) == NULL) && (i++ < wait_messages))
	{
		logMessage(LOG_DEBUG, "Find item in Menu: loop %d of %d looking for '%s' received message '%s'\n", i, wait_messages, item_string, aq_data->last_message);
		send_cmd(KEY_RIGHT);
		waitForMessage(aq_data, NULL, 1);
	}

	if (stristr(aq_data->last_message, item_string) == NULL) {
		return false;
	}

	logMessage(LOG_DEBUG, "Find item in Menu: loop %d of %d FOUND menu item '%s', sending ENTER command\n", i, wait_messages, item_string);
	// Enter the mode specified by the argument.

	send_cmd(KEY_ENTER);

	waitForMessage(aq_data, NULL, 1);

	return true;

}

// NSF Need to test this, then use it for the color change mode. 

bool waitForButtonState(struct aqualinkdata* aq_data, aqkey* button, AQ_LED_States state, int numMessageReceived)
{
	int i = 0;
	mtx_lock(&aq_data->active_thread.thread_mutex);

	while (++i <= numMessageReceived)
	{
		logMessage(LOG_DEBUG, "Programming mode: loop %d of %d looking for state change to '%d' for '%s' \n", i, numMessageReceived, button->led->state, button->name);

		if (button->led->state == state) {
			logMessage(LOG_INFO, "Programming mode: Button State =%d for %s\n", state, button->name);
			break;
		}

		cnd_wait(&aq_data->active_thread.thread_cond, &aq_data->active_thread.thread_mutex);
	}

	mtx_unlock(&aq_data->active_thread.thread_mutex);

	if (numMessageReceived >= i) {
		logMessage(LOG_DEBUG, "Programming mode: did not find state '%d' for '%s'\n", button->led->state, button->name);
		return false;
	}
	logMessage(LOG_DEBUG, "Programming mode: found state '%d' for '%s'\n", button->led->state, button->name);

	return true;
}

const char* ptypeName(program_type type)
{
	switch (type) {
	case AQ_GET_POOL_SPA_HEATER_TEMPS:
		return "Get Heater setpoints";
		break;
	case AQ_GET_FREEZE_PROTECT_TEMP:
		return "Get Freeze proctect";
		break;
	case AQ_SET_TIME:
		return "Set time";
		break;
	case AQ_SET_POOL_HEATER_TEMP:
		return "Set Pool heater setpoint";
		break;
	case AQ_SET_SPA_HEATER_TEMP:
		return "Set Spa heater setpoint";
		break;
	case AQ_SET_FRZ_PROTECTION_TEMP:
		return "Set Freeze protect setpoint";
		break;
	case AQ_GET_DIAGNOSTICS_MODEL:
		return "Get diagnostics";
		break;
	case AQ_GET_PROGRAMS:
		return "Get programs";
		break;
	case AQ_SET_COLORMODE:
		return "Set light color";
		break;
	case AQ_PDA_INIT:
		return "Init PDA";
		break;
	case AQ_SET_SWG_PERCENT:
		return "Set SWG percent";
		break;
	case AQ_PDA_DEVICE_STATUS:
		return "Get PDA Device status";
		break;
	case AQ_PDA_DEVICE_ON_OFF:
		return "Switch PDA device on/off";
		break;
	case AQ_GET_AUX_LABELS:
		return "Get AUX labels";
		break;
	case AQ_PDA_WAKE_INIT:
		return "PDA init after wake";
		break;
	case AQ_SET_BOOST:
		return "SWG Boost";
		break;
	case AQP_NULL:
	default:
		return "Unknown";
		break;
	}
}
