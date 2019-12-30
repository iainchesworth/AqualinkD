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
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <sys/time.h>
#include <syslog.h>
#include <assert.h>

#include "mongoose.h"

#include "config/config_helpers.h"
#include "json/json_messages.h"
#include "json/json_string_utils.h"
#include "serial/aq_serial_types.h"
#include "aqualink.h"
#include "aq_programmer.h"
#include "utils.h"
#include "net_services.h"
#include "domoticz.h"
#include "aq_mqtt.h"
#include "pda.h"
#include "aquapure.h"


static struct aqualinkdata* _aqualink_data;
static char* _web_root;

static int _mqtt_exit_flag = false;

#ifndef MG_DISABLE_MQTT
void start_mqtt(struct mg_mgr* mgr);
static struct aqualinkdata _last_mqtt_aqualinkdata;
void mqtt_broadcast_aqualinkstate(struct mg_connection* nc);
#endif


static sig_atomic_t s_signal_received = 0;
static struct mg_serve_http_opts s_http_server_opts;

static void signal_handler(int sig_num) 
{
	signal(sig_num, signal_handler);  // Reinstantiate signal handler
	s_signal_received = sig_num;
}


static int is_websocket(const struct mg_connection* nc) 
{
	return nc->flags & MG_F_IS_WEBSOCKET;
}

static int is_mqtt(const struct mg_connection* nc) 
{
	return nc->flags & MG_F_USER_1;
}

static void set_mqtt(struct mg_connection* nc) 
{
	nc->flags |= MG_F_USER_1;
}

static void ws_send(struct mg_connection* nc, char* msg)
{
	int size = strlen(msg);
	mg_send_websocket_frame(nc, WEBSOCKET_OP_TEXT, msg, size);
}

void broadcast_aqualinkstate_error(struct mg_connection* nc, char* msg)
{
	struct mg_connection* c;
	char data[JSON_STATUS_SIZE];
	build_aqualink_error_status_JSON(data, JSON_STATUS_SIZE, msg);

	for (c = mg_next(nc->mgr, NULL); c != NULL; c = mg_next(nc->mgr, c)) 
	{
		if (is_websocket(c))
		{
			ws_send(c, data);
		}
	}
	// Maybe enhacment in future to sent error messages to MQTT
}

void broadcast_aqualinkstate(struct mg_connection* nc)
{
	static int mqtt_count = 0;
	struct mg_connection* c;
	char data[JSON_STATUS_SIZE];
	build_aqualink_status_JSON(_aqualink_data, data, JSON_STATUS_SIZE);

#ifndef MG_DISABLE_MQTT  
	if (_mqtt_exit_flag == true) 
	{
		mqtt_count++;
		if (mqtt_count >= 10) 
		{
			start_mqtt(nc->mgr);
			mqtt_count = 0;
		}
	}
#endif

	for (c = mg_next(nc->mgr, NULL); c != NULL; c = mg_next(nc->mgr, c)) 
	{
		if (is_websocket(c)) 
		{
			ws_send(c, data);
		}

#ifndef MG_DISABLE_MQTT
		else if (is_mqtt(c)) 
		{
			mqtt_broadcast_aqualinkstate(c);
		}
#endif
	}
}

void send_mqtt(struct mg_connection* nc, const char* topic, char* message)
{
	static uint16_t msg_id = 0;

	if (topic == NULL)
	{
		return;
	}

	if (msg_id >= 65535) 
	{ 
		msg_id = 1; 
	}
	else 
	{ 
		msg_id++; 
	}

	mg_mqtt_publish(nc, topic, msg_id, MG_MQTT_RETAIN | MG_MQTT_QOS(1), message, strlen(message));
	logMessage(LOG_INFO, "MQTT: Published id=%d: %s %s\n", msg_id, topic, message);
}

void send_domoticz_mqtt_state_msg(struct mg_connection* nc, int idx, int value)
{
	assert(0 != nc);

	char mqtt_msg[JSON_MQTT_MSG_SIZE];
	const char* mqtt_dz_pub_topic = CFG_MqttDzPubTopic();

	if (idx <= 0)
	{
		// Do nothing;
	}
	else
	{
		build_mqtt_status_JSON(mqtt_msg, JSON_MQTT_MSG_SIZE, idx, value, TEMP_UNKNOWN);
		send_mqtt(nc, mqtt_dz_pub_topic, mqtt_msg);
	}
}

void send_domoticz_mqtt_temp_msg(struct mg_connection* nc, int idx, int value)
{
	assert(0 != nc);

	char mqtt_msg[JSON_MQTT_MSG_SIZE];

	const char* mqtt_dz_pub_topic = CFG_MqttDzPubTopic();
	const bool convert_dz_temp = CFG_ConvertDzTemp();

	if (idx <= 0) 
	{
		// Do nothing;
	}
	else
	{
		build_mqtt_status_JSON(mqtt_msg, JSON_MQTT_MSG_SIZE, idx, 0, ((FAHRENHEIT == _aqualink_data->temp_units) && (convert_dz_temp)) ? roundf(degFtoC(value)) : value);
		send_mqtt(nc, mqtt_dz_pub_topic, mqtt_msg);
	}
}
void send_domoticz_mqtt_numeric_msg(struct mg_connection* nc, int idx, int value)
{
	assert(0 != nc);

	char mqtt_msg[JSON_MQTT_MSG_SIZE];
	const char* mqtt_dz_pub_topic = CFG_MqttDzPubTopic();

	if (idx <= 0)
	{
		// Do nothing;
	}
	else
	{
		build_mqtt_status_JSON(mqtt_msg, JSON_MQTT_MSG_SIZE, idx, 0, value);
		send_mqtt(nc, mqtt_dz_pub_topic, mqtt_msg);
	}
}
void send_domoticz_mqtt_status_message(struct mg_connection* nc, int idx, int value, char* svalue)
{
	assert(0 != nc);

	char mqtt_msg[JSON_MQTT_MSG_SIZE];
	const char* mqtt_dz_pub_topic = CFG_MqttDzPubTopic();

	if (idx <= 0)
	{
		// Do nothing;
	}
	else
	{
		build_mqtt_status_message_JSON(mqtt_msg, JSON_MQTT_MSG_SIZE, idx, value, svalue);
		send_mqtt(nc, mqtt_dz_pub_topic, mqtt_msg);
	}
}

void send_mqtt_state_msg(struct mg_connection* nc, char* dev_name, AQ_LED_States state)
{
	assert(0 != nc);
	assert(0 != dev_name);

	const unsigned int MQTT_PUB_TOPIC_LEN = 250;
	static char mqtt_pub_topic[MQTT_PUB_TOPIC_LEN];

	const char* mqtt_aq_topic = CFG_MqttAqTopic();

	snprintf(mqtt_pub_topic, MQTT_PUB_TOPIC_LEN, "%s/%s/delay", mqtt_aq_topic, dev_name);
	send_mqtt(nc, mqtt_pub_topic, (state == FLASH ? MQTT_ON : MQTT_OFF));

	snprintf(mqtt_pub_topic, MQTT_PUB_TOPIC_LEN, "%s/%s", mqtt_aq_topic, dev_name);
	send_mqtt(nc, mqtt_pub_topic, (state == OFF ? MQTT_OFF : MQTT_ON));
}

void send_mqtt_aux_msg(struct mg_connection* nc, char* dev_name, char* dev_topic, int value)
{
	assert(0 != nc);
	assert(0 != dev_name);
	assert(0 != dev_topic);
	
	const unsigned int MQTT_PUB_TOPIC_LEN = 250;
	static char mqtt_pub_topic[MQTT_PUB_TOPIC_LEN];
	const unsigned int MSG_LEN = 10;
	static char msg[MSG_LEN];

	const char* mqtt_aq_topic = CFG_MqttAqTopic();

	snprintf(msg, MSG_LEN, "%d", value);
	snprintf(mqtt_pub_topic, MQTT_PUB_TOPIC_LEN, "%s/%s%s", mqtt_aq_topic, dev_name, dev_topic);

	send_mqtt(nc, mqtt_pub_topic, msg);
}

void send_mqtt_heater_state_msg(struct mg_connection* nc, char* dev_name, AQ_LED_States state)
{
	assert(0 != nc);
	assert(0 != dev_name);
	
	const unsigned int MQTT_PUB_TOPIC_LEN = 250;
	static char mqtt_pub_topic[MQTT_PUB_TOPIC_LEN];

	const char* mqtt_aq_topic = CFG_MqttAqTopic();

	snprintf(mqtt_pub_topic, MQTT_PUB_TOPIC_LEN, "%s/%s", mqtt_aq_topic, dev_name);

	if (state == ENABLE) 
	{
		send_mqtt(nc, mqtt_pub_topic, MQTT_OFF);
		snprintf(mqtt_pub_topic, MQTT_PUB_TOPIC_LEN, "%s/%s%s", mqtt_aq_topic, dev_name, ENABELED_SUBT);
		send_mqtt(nc, mqtt_pub_topic, MQTT_ON);
	}
	else 
	{
		send_mqtt(nc, mqtt_pub_topic, (state == OFF ? MQTT_OFF : MQTT_ON));
		snprintf(mqtt_pub_topic, MQTT_PUB_TOPIC_LEN, "%s/%s%s", mqtt_aq_topic, dev_name, ENABELED_SUBT);
		send_mqtt(nc, mqtt_pub_topic, (state == OFF ? MQTT_OFF : MQTT_ON));
	}
}

// NSF need to change this function to the _new once finished.
void send_mqtt_temp_msg(struct mg_connection* nc, char* dev_name, long value)
{
	assert(0 != nc);
	assert(0 != dev_name);

	const unsigned int MQTT_PUB_TOPIC_LEN = 250;
	static char mqtt_pub_topic[MQTT_PUB_TOPIC_LEN];
	const unsigned int DEGC_LEN = 10;
	static char degC[DEGC_LEN];

	const char* mqtt_aq_topic = CFG_MqttAqTopic();
	const bool convert_mqtt_temp = CFG_ConvertMqttTemp();
	
	snprintf(degC, DEGC_LEN, "%.2f", (_aqualink_data->temp_units == FAHRENHEIT && convert_mqtt_temp) ? degFtoC(value) : value);
	snprintf(mqtt_pub_topic, MQTT_PUB_TOPIC_LEN, "%s/%s", mqtt_aq_topic, dev_name);

	send_mqtt(nc, mqtt_pub_topic, degC);
}

void send_mqtt_setpoint_msg(struct mg_connection* nc, char* dev_name, long value)
{
	assert(0 != nc);
	assert(0 != dev_name);

	const unsigned int MQTT_PUB_TOPIC_LEN = 250;
	static char mqtt_pub_topic[MQTT_PUB_TOPIC_LEN];
	const unsigned int DEGC_LEN = 10;
	static char degC[DEGC_LEN];

	const char* mqtt_aq_topic = CFG_MqttAqTopic();
	const bool convert_mqtt_temp = CFG_ConvertMqttTemp();

	snprintf(degC, DEGC_LEN, "%.2f", (_aqualink_data->temp_units == FAHRENHEIT && convert_mqtt_temp) ? degFtoC(value) : value);
	snprintf(mqtt_pub_topic, MQTT_PUB_TOPIC_LEN, "%s/%s/setpoint", mqtt_aq_topic, dev_name);
	send_mqtt(nc, mqtt_pub_topic, degC);
}
void send_mqtt_numeric_msg(struct mg_connection* nc, char* dev_name, int value)
{
	assert(0 != nc);
	assert(0 != dev_name);

	const unsigned int MQTT_PUB_TOPIC_LEN = 250;
	static char mqtt_pub_topic[MQTT_PUB_TOPIC_LEN];
	const unsigned int MSG_LEN = 10;
	static char msg[MSG_LEN];

	const char* mqtt_aq_topic = CFG_MqttAqTopic();

	snprintf(msg, MSG_LEN, "%d", value);
	snprintf(mqtt_pub_topic, MQTT_PUB_TOPIC_LEN, "%s/%s", mqtt_aq_topic, dev_name);
	send_mqtt(nc, mqtt_pub_topic, msg);
}
void send_mqtt_float_msg(struct mg_connection* nc, char* dev_name, float value) 
{
	assert(0 != nc);
	assert(0 != dev_name);

	const unsigned int MQTT_PUB_TOPIC_LEN = 250;
	static char mqtt_pub_topic[MQTT_PUB_TOPIC_LEN];
	const unsigned int MSG_LEN = 10;
	static char msg[MSG_LEN];

	const char* mqtt_aq_topic = CFG_MqttAqTopic();

	snprintf(msg, MSG_LEN, "%.2f", value);
	snprintf(mqtt_pub_topic, MQTT_PUB_TOPIC_LEN, "%s/%s", mqtt_aq_topic, dev_name);
	send_mqtt(nc, mqtt_pub_topic, msg);
}

void send_mqtt_int_msg(struct mg_connection* nc, char* dev_name, int value) 
{
	send_mqtt_numeric_msg(nc, dev_name, value);
}

void send_mqtt_string_msg(struct mg_connection* nc, char* dev_name, char* msg) 
{
	assert(0 != nc);
	assert(0 != dev_name);

	const unsigned int MQTT_PUB_TOPIC_LEN = 250;
	static char mqtt_pub_topic[MQTT_PUB_TOPIC_LEN];

	const char* mqtt_aq_topic = CFG_MqttAqTopic();

	snprintf(mqtt_pub_topic, MQTT_PUB_TOPIC_LEN, "%s/%s", mqtt_aq_topic, dev_name);
	send_mqtt(nc, mqtt_pub_topic, msg);
}

void mqtt_broadcast_aqualinkstate(struct mg_connection* nc)
{
	assert(0 != _aqualink_data);

	const int dzidx_air_temp = CFG_DzIdxAirTemp();
	const int dzidx_pool_water_temp = CFG_DzIdxPoolWaterTemp();
	const int dzidx_spa_water_temp = CFG_DzIdxSpaWaterTemp();
	const int dzidx_swg_percent = CFG_DzIdxSwgPercent();
	const int dzidx_swg_ppm = CFG_DzIdxSwgPpm();
	const int dzidx_swg_status = CFG_DzIdxSwgStatus();
	const bool convert_mqtt_temp = CFG_ConvertMqttTemp();
	const bool report_zero_pool_temp = CFG_ReportZeroPoolTemp();
	const bool report_zero_spa_temp = CFG_ReportZeroSpaTemp();

	static int cnt = 0;
	bool force_update = false;

	if (cnt > 300) 
	{  
		// 100 = about every 2 minutes.
		force_update = true;
		cnt = 0;
	}
	else 
	{
		cnt++;
		force_update = false;
	}

	int i;

	if (_aqualink_data->service_mode_state != _last_mqtt_aqualinkdata.service_mode_state) 
	{
		_last_mqtt_aqualinkdata.service_mode_state = _aqualink_data->service_mode_state;
		send_mqtt_string_msg(nc, SERVICE_MODE_TOPIC, _aqualink_data->service_mode_state == OFF ? MQTT_OFF : (_aqualink_data->service_mode_state == FLASH ? MQTT_FLASH : MQTT_ON));
	}

	if (_aqualink_data->air_temp != TEMP_UNKNOWN && _aqualink_data->air_temp != _last_mqtt_aqualinkdata.air_temp) 
	{
		_last_mqtt_aqualinkdata.air_temp = _aqualink_data->air_temp;
		send_mqtt_temp_msg(nc, AIR_TEMP_TOPIC, _aqualink_data->air_temp);
		send_domoticz_mqtt_temp_msg(nc, dzidx_air_temp, _aqualink_data->air_temp);
	}

	if (_aqualink_data->pool_temp != _last_mqtt_aqualinkdata.pool_temp) 
	{
		if (_aqualink_data->pool_temp == TEMP_UNKNOWN && report_zero_pool_temp) 
		{
			_last_mqtt_aqualinkdata.pool_temp = TEMP_UNKNOWN;
			send_mqtt_temp_msg(nc, POOL_TEMP_TOPIC, (convert_mqtt_temp ? -18 : 0));
		}
		else if (_aqualink_data->pool_temp != TEMP_UNKNOWN) 
		{
			_last_mqtt_aqualinkdata.pool_temp = _aqualink_data->pool_temp;
			send_mqtt_temp_msg(nc, POOL_TEMP_TOPIC, _aqualink_data->pool_temp);
			send_domoticz_mqtt_temp_msg(nc, dzidx_pool_water_temp, _aqualink_data->pool_temp);
			
			// IF spa is off, report pool water temp to Domoticz.
			if (_aqualink_data->spa_temp == TEMP_UNKNOWN) 
			{
				send_domoticz_mqtt_temp_msg(nc, dzidx_spa_water_temp, _aqualink_data->pool_temp);
			}
		}
	}

	if (_aqualink_data->spa_temp != _last_mqtt_aqualinkdata.spa_temp) 
	{
		if (_aqualink_data->spa_temp == TEMP_UNKNOWN && report_zero_spa_temp) 
		{
			_last_mqtt_aqualinkdata.spa_temp = TEMP_UNKNOWN;
			send_mqtt_temp_msg(nc, SPA_TEMP_TOPIC, (convert_mqtt_temp ? -18 : 0));
		}
		else if (_aqualink_data->spa_temp != TEMP_UNKNOWN) 
		{
			_last_mqtt_aqualinkdata.spa_temp = _aqualink_data->spa_temp;
			send_mqtt_temp_msg(nc, SPA_TEMP_TOPIC, _aqualink_data->spa_temp);
			send_domoticz_mqtt_temp_msg(nc, dzidx_spa_water_temp, _aqualink_data->spa_temp);
		}
	}

	if (_aqualink_data->pool_htr_set_point != TEMP_UNKNOWN && _aqualink_data->pool_htr_set_point != _last_mqtt_aqualinkdata.pool_htr_set_point) 
	{
		_last_mqtt_aqualinkdata.pool_htr_set_point = _aqualink_data->pool_htr_set_point;
		send_mqtt_setpoint_msg(nc, BTN_POOL_HTR, _aqualink_data->pool_htr_set_point);
	}

	if (_aqualink_data->spa_htr_set_point != TEMP_UNKNOWN && _aqualink_data->spa_htr_set_point != _last_mqtt_aqualinkdata.spa_htr_set_point) 
	{
		_last_mqtt_aqualinkdata.spa_htr_set_point = _aqualink_data->spa_htr_set_point;
		send_mqtt_setpoint_msg(nc, BTN_SPA_HTR, _aqualink_data->spa_htr_set_point);
	}

	if (_aqualink_data->frz_protect_set_point != TEMP_UNKNOWN && _aqualink_data->frz_protect_set_point != _last_mqtt_aqualinkdata.frz_protect_set_point) 
	{
		_last_mqtt_aqualinkdata.frz_protect_set_point = _aqualink_data->frz_protect_set_point;
		send_mqtt_setpoint_msg(nc, FREEZE_PROTECT, _aqualink_data->frz_protect_set_point);
		send_mqtt_string_msg(nc, FREEZE_PROTECT, _aqualink_data->frz_protect_state == ON ? MQTT_ON : MQTT_OFF);
		_last_mqtt_aqualinkdata.frz_protect_state = _aqualink_data->frz_protect_state;
		send_mqtt_string_msg(nc, FREEZE_PROTECT_ENABELED, MQTT_ON);
	}

	if (_aqualink_data->frz_protect_state != _last_mqtt_aqualinkdata.frz_protect_state) 
	{
		_last_mqtt_aqualinkdata.frz_protect_state = _aqualink_data->frz_protect_state;
		send_mqtt_string_msg(nc, FREEZE_PROTECT, _aqualink_data->frz_protect_state == ON ? MQTT_ON : MQTT_OFF);
	}

	if (_aqualink_data->battery != _last_mqtt_aqualinkdata.battery) 
	{
		_last_mqtt_aqualinkdata.battery = _aqualink_data->battery;
		send_mqtt_string_msg(nc, BATTERY_STATE, _aqualink_data->battery == OK ? MQTT_ON : MQTT_OFF);
	}

	if (_aqualink_data->ar_swg_status != _last_mqtt_aqualinkdata.ar_swg_status ||  _aqualink_data->swg_percent != _last_mqtt_aqualinkdata.swg_percent) 
	{  
		// Percent can also effect status
		char message[30];
		int status;
		int dzalert;

		get_swg_status_mqtt(_aqualink_data, message, &status, &dzalert);

		if (!_aqualink_data->simulate_panel) 
		{
			sprintf(_aqualink_data->last_display_message, "%s", message);
		}

		send_domoticz_mqtt_status_message(nc, dzidx_swg_status, dzalert, &message[9]);
		send_mqtt_int_msg(nc, SWG_TOPIC, status);

		if (_aqualink_data->ar_swg_status == SWG_STATUS_OFF)
		{
			send_mqtt_int_msg(nc, SWG_ENABELED_TOPIC, SWG_OFF);
		}
		else
		{
			send_mqtt_int_msg(nc, SWG_ENABELED_TOPIC, SWG_ON);
		}

		send_mqtt_int_msg(nc, SWG_EXTENDED_TOPIC, (int)_aqualink_data->ar_swg_status);

		_last_mqtt_aqualinkdata.ar_swg_status = _aqualink_data->ar_swg_status;
	}

	if (_aqualink_data->boost != _last_mqtt_aqualinkdata.boost) 
	{
		send_mqtt_int_msg(nc, SWG_BOOST_TOPIC, _aqualink_data->boost);
		_last_mqtt_aqualinkdata.boost = _aqualink_data->boost;
	}

	if (_aqualink_data->ar_swg_status != SWG_STATUS_OFF) 
	{ 
		// If the SWG is actually on
		if (_aqualink_data->swg_percent != TEMP_UNKNOWN && (force_update || _aqualink_data->swg_percent != _last_mqtt_aqualinkdata.swg_percent)) 
		{
			_last_mqtt_aqualinkdata.swg_percent = _aqualink_data->swg_percent;
			send_mqtt_numeric_msg(nc, SWG_PERCENT_TOPIC, _aqualink_data->swg_percent);
			send_mqtt_float_msg(nc, SWG_PERCENT_F_TOPIC, roundf(degFtoC(_aqualink_data->swg_percent)));
			send_mqtt_float_msg(nc, SWG_SETPOINT_TOPIC, roundf(degFtoC(_aqualink_data->swg_percent)));
			send_domoticz_mqtt_numeric_msg(nc, dzidx_swg_percent, _aqualink_data->swg_percent);
		}
		if (_aqualink_data->swg_ppm != TEMP_UNKNOWN && (force_update || _aqualink_data->swg_ppm != _last_mqtt_aqualinkdata.swg_ppm)) 
		{
			_last_mqtt_aqualinkdata.swg_ppm = _aqualink_data->swg_ppm;
			send_mqtt_numeric_msg(nc, SWG_PPM_TOPIC, _aqualink_data->swg_ppm);
			send_mqtt_float_msg(nc, SWG_PPM_F_TOPIC, roundf(degFtoC(_aqualink_data->swg_ppm)));
			send_domoticz_mqtt_numeric_msg(nc, dzidx_swg_ppm, _aqualink_data->swg_ppm);
		}
	}

	// Loop over LED's and send any changes.
	for (i = 0; i < TOTAL_BUTTONS; i++) 
	{
		if (_last_mqtt_aqualinkdata.aqualinkleds[i].state != _aqualink_data->aqbuttons[i].led->state) 
		{
			_last_mqtt_aqualinkdata.aqualinkleds[i].state = _aqualink_data->aqbuttons[i].led->state;

			if (_aqualink_data->aqbuttons[i].code == KEY_POOL_HTR || _aqualink_data->aqbuttons[i].code == KEY_SPA_HTR) 
			{
				send_mqtt_heater_state_msg(nc, _aqualink_data->aqbuttons[i].name, _aqualink_data->aqbuttons[i].led->state);
			}
			else 
			{
				send_mqtt_state_msg(nc, _aqualink_data->aqbuttons[i].name, _aqualink_data->aqbuttons[i].led->state);
			}

			if (_aqualink_data->aqbuttons[i].dz_idx != DZ_NULL_IDX) 
			{
				send_domoticz_mqtt_state_msg(nc, _aqualink_data->aqbuttons[i].dz_idx, (_aqualink_data->aqbuttons[i].led->state == OFF ? DZ_OFF : DZ_ON));
			}
		}
	}

	// Loop over Pumps
	for (i = 0; i < MAX_PUMPS; i++) 
	{
		if (_aqualink_data->pumps[i].rpm != TEMP_UNKNOWN && _aqualink_data->pumps[i].rpm != _last_mqtt_aqualinkdata.pumps[i].rpm) 
		{
			_last_mqtt_aqualinkdata.pumps[i].rpm = _aqualink_data->pumps[i].rpm;
			send_mqtt_aux_msg(nc, _aqualink_data->pumps[i].button->name, PUMP_RPM_TOPIC, _aqualink_data->pumps[i].rpm);
		}
		if (_aqualink_data->pumps[i].gph != TEMP_UNKNOWN && _aqualink_data->pumps[i].gph != _last_mqtt_aqualinkdata.pumps[i].gph) 
		{
			_last_mqtt_aqualinkdata.pumps[i].gph = _aqualink_data->pumps[i].gph;
			send_mqtt_aux_msg(nc, _aqualink_data->pumps[i].button->name, PUMP_GPH_TOPIC, _aqualink_data->pumps[i].gph);
		}
		if (_aqualink_data->pumps[i].watts != TEMP_UNKNOWN && _aqualink_data->pumps[i].watts != _last_mqtt_aqualinkdata.pumps[i].watts) 
		{
			_last_mqtt_aqualinkdata.pumps[i].watts = _aqualink_data->pumps[i].watts;
			send_mqtt_aux_msg(nc, _aqualink_data->pumps[i].button->name, PUMP_WATTS_TOPIC, _aqualink_data->pumps[i].watts);
		}
	}
} 

int getTempforMeteohub(char* buffer)
{
	assert(0 != _aqualink_data);

	int length = 0;

	if (_aqualink_data->air_temp != TEMP_UNKNOWN)
	{
		length += sprintf(buffer + length, "t0 %d\n", (int)degFtoC(_aqualink_data->air_temp) * 10);
	}
	else
	{
		length += sprintf(buffer + length, "t0 \n");
	}

	if (_aqualink_data->pool_temp != TEMP_UNKNOWN)
	{
		length += sprintf(buffer + length, "t1 %d\n", (int)degFtoC(_aqualink_data->pool_temp) * 10);
	}
	else
	{
		length += sprintf(buffer + length, "t1 \n");
	}

	return strlen(buffer);
}

void set_light_mode(char* value, int button)
{
	assert(0 != value);

	const bool pda_mode = CFG_PdaMode();
	const float light_programming_mode = CFG_LightProgrammingMode();
	const int light_programming_initial_on = CFG_LightProgrammingInitialOn();
	const int light_programming_initial_off = CFG_LightProgrammingInitialOff();
	const int light_programming_button_pool = CFG_LightProgrammingButtonPool();
	const int light_programming_button_spa = CFG_LightProgrammingButtonSpa();

	if (pda_mode == true) 
	{
		logMessage(LOG_ERR, "Light mode control not supported in PDA mode\n");
		return;
	}

	if (light_programming_button_pool != button && light_programming_button_spa != button) 
	{
		logMessage(LOG_ERR, "Light mode control not configured for button %d\n", button);
		return;
	}

	char buf[LIGHT_MODE_BUFER];
	
	// 5 below is light index, need to look this up so it's not hard coded.
	snprintf(buf, LIGHT_MODE_BUFER, "%-5s%-5d%-5d%-5d%.2f", value,
		button,
		light_programming_initial_on,
		light_programming_initial_off,
		light_programming_mode);

	aq_programmer(AQ_SET_COLORMODE, buf, _aqualink_data);
}

/*
void action_web_request(struct mg_connection* nc, struct http_message* http_msg) 
{
	assert(0 != nc);
	assert(0 != http_msg);

	const int light_programming_button_pool = cfg_getint(_config_parameters, CONFIG_INT_LIGHT_PROGRAMMING_BUTTON_POOL);
	const int light_programming_button_spa = cfg_getint(_config_parameters, CONFIG_INT_LIGHT_PROGRAMMING_BUTTON_SPA);
	const bool pda_mode = cfg_getbool(_config_parameters, CONFIG_BOOL_PDA_MODE);

	if (getLogLevel() >= LOG_INFO) 
	{ 
		// Simply for log message, check we are at this log level before running all this junk

		char* uri = (char*)malloc(http_msg->uri.len + http_msg->query_string.len + 2);
		strncpy(uri, http_msg->uri.p, http_msg->uri.len + http_msg->query_string.len + 1);
		uri[http_msg->uri.len + http_msg->query_string.len + 1] = '\0';
		logMessage(LOG_INFO, "URI request: '%s'\n", uri);
		free(uri);
	}

	// If we have a get request, pass it
	if (strstr(http_msg->method.p, "GET") && http_msg->query_string.len > 0) 
	{
		char command[20];

		mg_get_http_var(&http_msg->query_string, "command", command, sizeof(command));
		logMessage(LOG_INFO, "WEB: Message command='%s'\n", command);

		if (strcmp(command, "status") == 0) 
		{
			char data[JSON_STATUS_SIZE];
			int size = build_aqualink_status_JSON(_aqualink_data, data, JSON_STATUS_SIZE);
			mg_send_head(nc, 200, size, "Content-Type: application/json");
			mg_send(nc, data, size);
		}
		else if (strcmp(command, "mhstatus") == 0) 
		{
			char data[20];
			int size = getTempforMeteohub(data);
			mg_send_head(nc, 200, size, "Content-Type: text/plain");
			mg_send(nc, data, size);
		}
		else if (strcmp(command, "poollightmode") == 0) 
		{
			char value[20];
			mg_get_http_var(&http_msg->query_string, "value", value, sizeof(value));
			set_light_mode(value, light_programming_button_pool);
			mg_send_head(nc, 200, strlen(GET_RTN_OK), "Content-Type: text/plain");
			mg_send(nc, GET_RTN_OK, strlen(GET_RTN_OK));
		}
		else if (strcmp(command, "spalightmode") == 0) 
		{
			char value[20];
			mg_get_http_var(&http_msg->query_string, "value", value, sizeof(value));
			set_light_mode(value, light_programming_button_spa);
			mg_send_head(nc, 200, strlen(GET_RTN_OK), "Content-Type: text/plain");
			mg_send(nc, GET_RTN_OK, strlen(GET_RTN_OK));
		}
		else if (strcmp(command, "diag") == 0) 
		{
			aq_programmer(AQ_GET_DIAGNOSTICS_MODEL, NULL, _aqualink_data);
			mg_send_head(nc, 200, strlen(GET_RTN_OK), "Content-Type: text/plain");
			mg_send(nc, GET_RTN_OK, strlen(GET_RTN_OK));
		}
		else if (strcmp(command, "settime") == 0) 
		{
			aq_programmer(AQ_SET_TIME, NULL, _aqualink_data);
			mg_send_head(nc, 200, strlen(GET_RTN_OK), "Content-Type: text/plain");
			mg_send(nc, GET_RTN_OK, strlen(GET_RTN_OK));
		}
		else if (strcmp(command, "swg_percent") == 0) 
		{
			char value[20];
			mg_get_http_var(&http_msg->query_string, "value", value, sizeof(value));
			aq_programmer(AQ_SET_SWG_PERCENT, value, _aqualink_data);
			logMessage(LOG_INFO, "Web: request to set SWG to %s\n", value);
			mg_send_head(nc, 200, strlen(GET_RTN_OK), "Content-Type: text/plain");
			mg_send(nc, GET_RTN_OK, strlen(GET_RTN_OK));
		}
		else if (strcmp(command, "pool_htr_set_pnt") == 0) 
		{
			char value[20];
			mg_get_http_var(&http_msg->query_string, "value", value, sizeof(value));
			aq_programmer(AQ_SET_POOL_HEATER_TEMP, value, _aqualink_data);
			logMessage(LOG_INFO, "Web: request to set Pool heater to %s\n", value);
			mg_send_head(nc, 200, strlen(GET_RTN_OK), "Content-Type: text/plain");
			mg_send(nc, GET_RTN_OK, strlen(GET_RTN_OK));
		}
		else if (strcmp(command, "spa_htr_set_pnt") == 0) 
		{
			char value[20];
			mg_get_http_var(&http_msg->query_string, "value", value, sizeof(value));
			aq_programmer(AQ_SET_SPA_HEATER_TEMP, value, _aqualink_data);
			logMessage(LOG_INFO, "Web: request to set Spa heater to %s\n", value);
			mg_send_head(nc, 200, strlen(GET_RTN_OK), "Content-Type: text/plain");
			mg_send(nc, GET_RTN_OK, strlen(GET_RTN_OK));
		}
		else if (strcmp(command, "frz_protect_set_pnt") == 0) 
		{
			char value[20];
			mg_get_http_var(&http_msg->query_string, "value", value, sizeof(value));
			aq_programmer(AQ_SET_FRZ_PROTECTION_TEMP, value, _aqualink_data);
			logMessage(LOG_INFO, "Web: request to set Freeze protect to %s\n", value);
			mg_send_head(nc, 200, strlen(GET_RTN_OK), "Content-Type: text/plain");
			mg_send(nc, GET_RTN_OK, strlen(GET_RTN_OK));
		}
		else if (strcmp(command, "devices") == 0) 
		{
			char message[JSON_LABEL_SIZE * 10];
			int size = build_device_JSON(_aqualink_data, light_programming_button_pool, light_programming_button_spa, message, JSON_LABEL_SIZE * 10, false);
			mg_send_head(nc, 200, size, "Content-Type: application/json");
			mg_send(nc, message, size);
		}
		else if (strcmp(command, "homebridge") == 0) 
		{
			char message[JSON_LABEL_SIZE * 10];
			int size = build_device_JSON(_aqualink_data, light_programming_button_pool, light_programming_button_spa, message, JSON_LABEL_SIZE * 10, true);
			mg_send_head(nc, 200, size, "Content-Type: application/json");
			mg_send(nc, message, size);
		}
		else if (strcmp(command, "setconfigprm") == 0) 
		{
			char value[20];
			char param[30];
			char* webrtn;
			mg_get_http_var(&http_msg->query_string, "param", param, sizeof(param));
			mg_get_http_var(&http_msg->query_string, "value", value, sizeof(value));

			///FIXME FIXME FIXME

			if (setConfigValue(_aqualink_config, _aqualink_data, param, value)) 
			{
				webrtn = GET_RTN_OK;
				logMessage(LOG_INFO, "Web: request to config %s to %s\n", param, value);
			}
			else 
			{
				webrtn = GET_RTN_ERROR;
				logMessage(LOG_ERR, "Web: request to config %s to %s failed\n", param, value);
			}
			mg_send_head(nc, 200, strlen(webrtn), "Content-Type: text/plain");
			mg_send(nc, webrtn, strlen(webrtn));
		}
		else if (strcmp(command, "writeconfig") == 0) 
{
			if (writeCfg(_aqualink_config, _aqualink_data)) 
			{
				mg_send_head(nc, 200, strlen(GET_RTN_OK), "Content-Type: text/plain");
				mg_send(nc, GET_RTN_OK, strlen(GET_RTN_OK));
			}
			else 
			{
				mg_send_head(nc, 200, strlen(GET_RTN_ERROR), "Content-Type: text/plain");
				mg_send(nc, GET_RTN_ERROR, strlen(GET_RTN_ERROR));
			}
		}
		else if (strcmp(command, "debug") == 0) 
{
			char value[80];
			char* rtn;
			mg_get_http_var(&http_msg->query_string, "value", value, sizeof(value));
			if (strcmp(value, "start") == 0) 
			{
				startInlineDebug();
				rtn = GET_RTN_OK;
				logMessage(LOG_DEBUG, "WEB: Started inline debug mode\n");
			}
			else if (strcmp(value, "stop") == 0) 
			{
				logMessage(LOG_DEBUG, "WEB: Stoped inline debug mode\n");
				stopInlineDebug();
				rtn = GET_RTN_OK;
			}
			else if (strcmp(value, "serialstart") == 0) 
			{
				rtn = GET_RTN_OK;
			}
			else if (strcmp(value, "serialstop") == 0) 
			{
				rtn = GET_RTN_OK;
			}
			else if (strcmp(value, "status") == 0) 
			{
				snprintf(value, 80, "{\"sLevel\":\"%s\", \"iLevel\":%d, \"logReady\":\"%s\"}\n", elevel2text(getLogLevel()), getLogLevel(), islogFileReady() ? "true" : "false");
				mg_send_head(nc, 200, strlen(value), "Content-Type: text/json");
				mg_send(nc, value, strlen(value));
				return;
			}
			else if (strcmp(value, "clean") == 0) 
			{
				cleanInlineDebug();
				rtn = GET_RTN_OK;
			}
			else if (strcmp(value, "download") == 0) 
			{
				mg_http_serve_file(nc, http_msg, getInlineLogFName(), mg_mk_str("text/plain"), mg_mk_str(""));
				return;
			}
			else 
			{
				rtn = GET_RTN_UNKNOWN;
			}

			mg_send_head(nc, 200, strlen(rtn), "Content-Type: text/plain");
			mg_send(nc, rtn, strlen(rtn));
		}
		else 
{
			int i;
			for (i = 0; i < TOTAL_BUTTONS; i++) 
			{
				if (strcmp(command, _aqualink_data->aqbuttons[i].name) == 0) 
				{
					char value[20];
					char* rtn;
					mg_get_http_var(&http_msg->query_string, "value", value, sizeof(value));
					logMessage(LOG_DEBUG, "WEB: Message request '%s' change state to '%s'\n", command, value);

					if (pda_mode == true) 
					{
						char msg[PTHREAD_ARG];
						sprintf(msg, "%-5d%-5d", i, (strcmp(value, "on") == 0) ? ON : OFF);
						aq_programmer(AQ_PDA_DEVICE_ON_OFF, msg, _aqualink_data);
						rtn = GET_RTN_OK;
					}
					else if (strcmp(value, "on") == 0) 
					{
						if (_aqualink_data->aqbuttons[i].led->state == OFF || _aqualink_data->aqbuttons[i].led->state == FLASH) 
						{
							aq_send_cmd(_aqualink_data->aqbuttons[i].code);
							rtn = GET_RTN_OK;
							logMessage(LOG_INFO, "WEB: turn ON '%s' changed state to '%s'\n", command, value);
						}
						else 
						{
							rtn = GET_RTN_NOT_CHANGED;
							logMessage(LOG_INFO, "WEB: '%s' is already on '%s', current state %d\n", command, value, _aqualink_data->aqbuttons[i].led->state);
						}
					}
					else if (strcmp(value, "off") == 0) 
					{
						if (_aqualink_data->aqbuttons[i].led->state == ON ||
							_aqualink_data->aqbuttons[i].led->state == ENABLE ||
							_aqualink_data->aqbuttons[i].led->state == FLASH) 
						{
							aq_send_cmd(_aqualink_data->aqbuttons[i].code);
							rtn = GET_RTN_OK;
							logMessage(LOG_INFO, "WEB: turn Off '%s' changed state to '%s'\n", command, value);
						}
						else 
						{
							rtn = GET_RTN_NOT_CHANGED;
							logMessage(LOG_INFO, "WEB: '%s' is already off '%s', current state %d\n", command, value, _aqualink_data->aqbuttons[i].led->state);
						}
					}
					else 
					{ 
						// Blind switch
						aq_send_cmd(_aqualink_data->aqbuttons[i].code);
						rtn = GET_RTN_OK;
						logMessage(LOG_INFO, "WEB: '%s' blindly changed state\n", command, value);
					}

					logMessage(LOG_DEBUG, "WEB: On=%d, Off=%d, Enable=%d, Flash=%d\n", ON, OFF, ENABLE, FLASH);

					// NSF change OK and 2 below to a constant
					mg_send_head(nc, 200, strlen(rtn), "Content-Type: text/plain");
					mg_send(nc, rtn, strlen(rtn));

					// NSF place check we found command here
				}
			}
		}

		// If we get here, got a bad query
		mg_send_head(nc, 200, strlen(GET_RTN_UNKNOWN), "Content-Type: text/plain");
		mg_send(nc, GET_RTN_UNKNOWN, strlen(GET_RTN_UNKNOWN));

	}
	else 
	{
		struct mg_serve_http_opts opts;
		memset(&opts, 0, sizeof(opts)); // Reset all options to defaults
		opts.document_root = _web_root; // Serve files from the current directory
		mg_serve_http(nc, http_msg, s_http_server_opts);
	}
}
*/

void action_websocket_request(struct mg_connection* nc, struct websocket_message* wm) 
{
	assert(0 != nc);
	assert(0 != wm);

	const int light_programming_button_pool = CFG_LightProgrammingButtonPool();
	const int light_programming_button_spa = CFG_LightProgrammingButtonSpa();
	const bool pda_mode = CFG_PdaMode();

	char buffer[50];
	struct JSONwebrequest request;

	// Any websocket request means UI is active, so don't let AqualinkD go to sleep if in PDA mode
	if (pda_mode)
	{
		pda_reset_sleep();
	}

	strncpy(buffer, (char*)wm->data, wm->size);
	buffer[wm->size] = '\0';

	parseJSONwebrequest(buffer, &request);
	logMessage(LOG_INFO, "WS: Message - Key '%s' Value '%s' | Key2 '%s' Value2 '%s'\n", request.first.key, request.first.value, request.second.key, request.second.value);

	if (strcmp(request.first.key, "raw") == 0) 
	{
		_aqualink_data->simulate_panel = true;
		logMessage(LOG_NOTICE, "WS: Send raw command to controller %s\n", request.first.value);
		unsigned int n;
		sscanf(request.first.value, "0x%2x", &n);
		aq_send_cmd((unsigned char)n);
	}
	else if (strcmp(request.first.key, "command") == 0) 
	{
		_aqualink_data->simulate_panel = false;
		if (strcmp(request.first.value, "GET_AUX_LABELS") == 0) 
		{
			char labels[JSON_LABEL_SIZE];
			build_aux_labels_JSON(_aqualink_data, labels, JSON_LABEL_SIZE);
			ws_send(nc, labels);
		}
		else if (strcmp(request.first.value, "GET_DEVICES") == 0) 
		{
			char message[JSON_LABEL_SIZE * 10];
			build_device_JSON(_aqualink_data, light_programming_button_pool, light_programming_button_spa, message, JSON_LABEL_SIZE * 10, false);
			ws_send(nc, message);
		}
		else if (strcmp(request.first.value, "simulator") == 0) 
		{
			_aqualink_data->simulate_panel = true;
			logMessage(LOG_INFO, "WS: Set Simulator mode");
			char labels[JSON_LABEL_SIZE];
			build_aux_labels_JSON(_aqualink_data, labels, JSON_LABEL_SIZE);
			ws_send(nc, labels);
		}
		else if (strcmp(request.first.value, SWG_BOOST_TOPIC) == 0) 
		{
			if (request.second.value != NULL)
			{
				_aqualink_data->unactioned.value = request2bool(request.second.value);
			}
			else
			{
				_aqualink_data->unactioned.value = !_aqualink_data->boost;
			}

			_aqualink_data->unactioned.type = SWG_BOOST;
		}
		else 
		{ 
			// Search for value in command list
			int i;

			for (i = 0; i < TOTAL_BUTTONS; i++) 
			{
				if (strcmp(request.first.value, _aqualink_data->aqbuttons[i].name) == 0) 
				{
					logMessage(LOG_INFO, "WS: button '%s' pressed\n", _aqualink_data->aqbuttons[i].name);
					if (pda_mode == false) 
					{
						aq_send_cmd(_aqualink_data->aqbuttons[i].code);
					}
					else 
					{
						char msg[THREAD_ARG];
						sprintf(msg, "%-5d%-5d", i, (_aqualink_data->aqbuttons[i].led->state != OFF) ? OFF : ON);
						aq_programmer(AQ_PDA_DEVICE_ON_OFF, msg, _aqualink_data);
					}
					break;
					// NSF place check we found command here
				}
			}
		}
	}
	else if (strcmp(request.first.key, "parameter") == 0) 
	{
		_aqualink_data->simulate_panel = false;
		if (strcmp(request.first.value, "FRZ_PROTECT") == 0 || strcmp(request.first.value, FREEZE_PROTECT) == 0) 
		{
			aq_programmer(AQ_SET_FRZ_PROTECTION_TEMP, request.second.value, _aqualink_data);
		}
		else if (strcmp(request.first.value, "POOL_HTR") == 0 || strcmp(request.first.value, BTN_POOL_HTR) == 0) 
		{
			aq_programmer(AQ_SET_POOL_HEATER_TEMP, request.second.value, _aqualink_data);
		}
		else if (strcmp(request.first.value, "SPA_HTR") == 0 || strcmp(request.first.value, BTN_SPA_HTR) == 0) 
		{
			aq_programmer(AQ_SET_SPA_HEATER_TEMP, request.second.value, _aqualink_data);
		}
		else if (strcmp(request.first.value, SWG_TOPIC) == 0) 
		{
			int value = setpoint_check(SWG_SETPOINT, atoi(request.second.value), _aqualink_data);
			if (_aqualink_data->ar_swg_status == SWG_STATUS_OFF) 
			{
				// SWG is off, can't set %, so delay the set until it's on.
				_aqualink_data->swg_delayed_percent = value;
			}
			else 
			{
				aq_programmer(AQ_SET_SWG_PERCENT, request.second.value, _aqualink_data);
				_aqualink_data->swg_percent = value; // Set the value as if it's already been set, just incase it's 0 as we won't get that message, or will update next time
			}
		}
		else if (strcmp(request.first.value, "POOL_LIGHT_MODE") == 0) 
		{
			set_light_mode(request.second.value, light_programming_button_pool);
		}
		else if (strcmp(request.first.value, "LIGHT_MODE") == 0) 
		{
			int i;
			for (i = 0; i < TOTAL_BUTTONS; i++) 
			{
				if (strcmp(request.third.value, _aqualink_data->aqbuttons[i].name) == 0) 
				{
					set_light_mode(request.second.value, i);
					break;
				}
			}
		}
		else 
		{
			logMessage(LOG_DEBUG, "WS: Unknown parameter %s\n", request.first.value);
		}
	}
}

void action_mqtt_message(struct mg_connection* nc, struct mg_mqtt_message* msg) 
{
	assert(0 != nc);
	assert(0 != msg);

	const char* mqtt_aq_topic = CFG_MqttAqTopic();
	const bool convert_mqtt_temp = CFG_ConvertMqttTemp();

	unsigned int i;

	// If message doesn't end in set we don't care about it.
	if (strncmp(&msg->topic.p[msg->topic.len - 4], "/set", 4) != 0) 
	{
		logMessage(LOG_DEBUG, "MQTT: Ignore %.*s %.*s\n", msg->topic.len, msg->topic.p, msg->payload.len, msg->payload.p);
		return;
	}

	logMessage(LOG_DEBUG, "MQTT: topic %.*s %.*s\n", msg->topic.len, msg->topic.p, msg->payload.len, msg->payload.p);

	//Need to do this in a better manor, but for present it's ok.
	static char tmp[20];
	strncpy(tmp, msg->payload.p, msg->payload.len);
	tmp[msg->payload.len] = '\0';

	float value = atof(tmp);

	// get the parts from the topic
	char* pt1 = (char*)&msg->topic.p[strlen(mqtt_aq_topic) + 1];
	char* pt2 = NULL;
	char* pt3 = NULL;

	for (i = strlen(mqtt_aq_topic) + 1; i < msg->topic.len; i++) 
	{
		if (msg->topic.p[i] == '/') 
		{
			if (pt2 == NULL) 
			{
				pt2 = (char*)&msg->topic.p[++i];
			}
			else if (pt3 == NULL) 
			{
				pt3 = (char*)&msg->topic.p[++i];
				break;
			}
		}
	}

	//only care about topics with set at the end.
	//aqualinkd/Freeze/setpoint/set
	//aqualinkd/Filter_Pump/set
	//aqualinkd/Pool_Heater/setpoint/set
	//aqualinkd/Pool_Heater/set
	//aqualinkd/SWG/Percent_f/set

	if (pt3 != NULL && (strncmp(pt2, "setpoint", 8) == 0) && (strncmp(pt3, "set", 3) == 0)) 
	{
		int val = _aqualink_data->unactioned.value = (_aqualink_data->temp_units != CELSIUS && convert_mqtt_temp) ? round(degCtoF(value)) : round(value);
		if (strncmp(pt1, BTN_POOL_HTR, strlen(BTN_POOL_HTR)) == 0) 
		{
			_aqualink_data->unactioned.value = setpoint_check(POOL_HTR_SETOINT, val, _aqualink_data);
			_aqualink_data->unactioned.type = POOL_HTR_SETOINT;
		}
		else if (strncmp(pt1, BTN_SPA_HTR, strlen(BTN_SPA_HTR)) == 0) 
		{
			_aqualink_data->unactioned.value = setpoint_check(SPA_HTR_SETOINT, val, _aqualink_data);
			_aqualink_data->unactioned.type = SPA_HTR_SETOINT;
		}
		else if (strncmp(pt1, FREEZE_PROTECT, strlen(FREEZE_PROTECT)) == 0) 
		{
			_aqualink_data->unactioned.value = setpoint_check(FREEZE_SETPOINT, val, _aqualink_data);
			_aqualink_data->unactioned.type = FREEZE_SETPOINT;
		}
		else if (strncmp(pt1, "SWG", 3) == 0) 
		{  
			// If we get SWG percent as setpoint message.
			int val = round(degCtoF(value));
			_aqualink_data->unactioned.value = setpoint_check(SWG_SETPOINT, val, _aqualink_data);
			_aqualink_data->unactioned.type = SWG_SETPOINT;
		}
		else 
		{
			// Not sure what the setpoint is, ignore.
			logMessage(LOG_DEBUG, "MQTT: ignoring %.*s don't recognise button setpoint\n", msg->topic.len, msg->topic.p);
			return;
		}

		time(&_aqualink_data->unactioned.requested);
	}
	else if ((pt3 != NULL && (strncmp(pt1, "SWG", 3) == 0) && (strncmp(pt2, "Percent", 7) == 0) && (strncmp(pt3, "set", 3) == 0))) 
	{
		int val;

		if ((strncmp(pt2, "Percent_f", 9) == 0)) 
		{
			val = _aqualink_data->unactioned.value = round(degCtoF(value));
		}
		else 
		{
			val = _aqualink_data->unactioned.value = round(value);
		}

		_aqualink_data->unactioned.value = setpoint_check(SWG_SETPOINT, val, _aqualink_data);
		_aqualink_data->unactioned.type = SWG_SETPOINT;
	}
	else if ((pt3 != NULL && (strncmp(pt1, "SWG", 3) == 0) && (strncmp(pt2, "Boost", 5) == 0) && (strncmp(pt3, "set", 3) == 0))) 
	{
		_aqualink_data->unactioned.value = round(value);
		_aqualink_data->unactioned.type = SWG_BOOST;
	}
	else if (pt2 != NULL && (strncmp(pt2, "set", 3) == 0) && (strncmp(pt2, "setpoint", 8) != 0)) 
	{
		// Must be a switch on / off
		for (i = 0; i < TOTAL_BUTTONS; i++) 
		{
			if (strncmp(pt1, _aqualink_data->aqbuttons[i].name, strlen(_aqualink_data->aqbuttons[i].name)) == 0) 
			{
				logMessage(LOG_INFO, "MQTT: MATCH %s to topic %.*s\n", _aqualink_data->aqbuttons[i].name, msg->topic.len, msg->topic.p);
				// Message is either a 1 or 0 for on or off
				if (value > 1 || value < 0) 
				{
					logMessage(LOG_ERR, "MQTT: topic %.*s %.2f\n", msg->topic.len, msg->topic.p, value);
					logMessage(LOG_ERR, "MQTT: received unknown status of '%.*s' for '%s', Ignoring!\n", msg->payload.len, msg->payload.p, _aqualink_data->aqbuttons[i].name);
				}
				else if ((_aqualink_data->aqbuttons[i].led->state == OFF && value == 0) || (value == 1 && (_aqualink_data->aqbuttons[i].led->state == ON || _aqualink_data->aqbuttons[i].led->state == FLASH || _aqualink_data->aqbuttons[i].led->state == ENABLE))) 
				{
					logMessage(LOG_INFO, "MQTT: received '%s' for '%s', already '%s', Ignoring\n", (value == 0 ? "OFF" : "ON"), _aqualink_data->aqbuttons[i].name, (value == 0 ? "OFF" : "ON"));
				}
				else 
				{
					logMessage(LOG_INFO, "MQTT: received '%s' for '%s', turning '%s'\n", (value == 0 ? "OFF" : "ON"), _aqualink_data->aqbuttons[i].name, (value == 0 ? "OFF" : "ON"));
					if (CFG_PdaMode() == false) 
					{
						aq_send_cmd(_aqualink_data->aqbuttons[i].code);
					}
					else 
					{
						char msg[THREAD_ARG];
						sprintf(msg, "%-5d%-5d", i, (value == 0 ? OFF : ON));
						aq_programmer(AQ_PDA_DEVICE_ON_OFF, msg, _aqualink_data);
					}
					// Some circumstances these are ignored by control panel, so furce the MQTT state to be sent again on next poll just ncase it doesn't change
					// This causes a good MQTT button to flash on/off once before getting the latest update.  Need to find a better way
					//_last_mqtt_aqualinkdata.aqualinkleds[i].state = LED_S_UNKNOWN;
					// NSF   Update above to cater for ignored buttons.
				}
				break;
			}
		}
	}
	else 
	{
		// We don's care
		logMessage(LOG_DEBUG, "MQTT: ignoring topic %.*s\n", msg->topic.len, msg->topic.p);
	}
}

void action_domoticz_mqtt_message(struct mg_connection* nc, struct mg_mqtt_message* msg) 
{
	int idx = -1;
	int nvalue = -1;
	int i;
	char svalue[DZ_SVALUE_LEN + 1];

	if (parseJSONmqttrequest(msg->payload.p, msg->payload.len, &idx, &nvalue, svalue)) 
	{
		logMessage(LOG_DEBUG, "MQTT: DZ: Received message IDX=%d nValue=%d sValue=%s\n", idx, nvalue, svalue);
		for (i = 0; i < TOTAL_BUTTONS; i++) 
		{
			if (_aqualink_data->aqbuttons[i].dz_idx == idx) 
			{
				//NSF, should try to simplify this if statment, but not easy since AQ ON and DZ ON are different, and AQ has other states.
				if ((_aqualink_data->aqbuttons[i].led->state == OFF && nvalue == DZ_OFF) || (nvalue == DZ_ON && (_aqualink_data->aqbuttons[i].led->state == ON || _aqualink_data->aqbuttons[i].led->state == FLASH || _aqualink_data->aqbuttons[i].led->state == ENABLE))) 
				{
					logMessage(LOG_INFO, "MQTT: DZ: received '%s' for '%s', already '%s', Ignoring\n", (nvalue == DZ_OFF ? "OFF" : "ON"), _aqualink_data->aqbuttons[i].name, (nvalue == DZ_OFF ? "OFF" : "ON"));
				}
				else 
				{
					// NSF Below if needs to check that the button pressed is actually a light. Add this later
					if (_aqualink_data->active_thread.ptype == AQ_SET_COLORMODE) 
					{
						logMessage(LOG_NOTICE, "MQTT: DZ: received '%s' for '%s', IGNORING as we are programming light mode\n", (nvalue == DZ_OFF ? "OFF" : "ON"), _aqualink_data->aqbuttons[i].name);
					}
					else 
					{
						logMessage(LOG_INFO, "MQTT: DZ: received '%s' for '%s', turning '%s'\n", (nvalue == DZ_OFF ? "OFF" : "ON"), _aqualink_data->aqbuttons[i].name, (nvalue == DZ_OFF ? "OFF" : "ON"));
						if (CFG_PdaMode() == false) 
						{
							aq_send_cmd(_aqualink_data->aqbuttons[i].code);
						}
						else 
						{
							char msg[THREAD_ARG];
							sprintf(msg, "%-5d%-5d", i, (nvalue == DZ_OFF ? OFF : ON));
							aq_programmer(AQ_PDA_DEVICE_ON_OFF, msg, _aqualink_data);
						}
					}
				}
				break; // no need to continue in for loop, we found button.
			}
		}
	}

	// NSF Need to check idx against ours and decide if we are interested in it.
}

static void ev_handler(struct mg_connection* nc, int ev, void* ev_data) 
{
	assert(0 != nc);
	assert(0 != ev_data);

	const char* mqtt_dz_sub_topic = CFG_MqttDzSubTopic();
	const char* mqtt_aq_topic = CFG_MqttAqTopic();
	const char* mqtt_user = CFG_MqttUser();
	const char* mqtt_passwd = CFG_MqttPassword();
	const char* mqtt_id = CFG_MqttId();

	struct mg_mqtt_message* mqtt_msg;
	struct http_message* http_msg;
	struct websocket_message* ws_msg;
	char aq_topic[30];

	switch (ev) {
	case MG_EV_HTTP_REQUEST:
		http_msg = (struct http_message*)ev_data;
		
		///FIXME
		//action_web_request(nc, http_msg);
		
		logMessage(LOG_DEBUG, "Served WEB request\n");
		break;

	case MG_EV_WEBSOCKET_HANDSHAKE_DONE:
		_aqualink_data->open_websockets++;
		logMessage(LOG_DEBUG, "++ Websocket joined\n");
		break;

	case MG_EV_WEBSOCKET_FRAME:
		ws_msg = (struct websocket_message*)ev_data;
		action_websocket_request(nc, ws_msg);
		break;

	case MG_EV_CLOSE:
		if (is_websocket(nc)) 
		{
			_aqualink_data->open_websockets--;
			logMessage(LOG_DEBUG, "-- Websocket left\n");
		}
		else if (is_mqtt(nc)) 
		{
			logMessage(LOG_WARNING, "MQTT Connection closed\n");
			_mqtt_exit_flag = true;
		}
		break;

	case MG_EV_CONNECT: 
	{
		set_mqtt(nc);
		_mqtt_exit_flag = false;
		struct mg_send_mqtt_handshake_opts opts;
		memset(&opts, 0, sizeof(opts));
		opts.user_name = mqtt_user;
		opts.password = mqtt_passwd;
		opts.keep_alive = 5;
		opts.flags |= MG_MQTT_CLEAN_SESSION; // NFS Need to readup on this

		snprintf(aq_topic, 24, "%s/%s", mqtt_aq_topic, MQTT_LWM_TOPIC);
		opts.will_topic = aq_topic;
		opts.will_message = MQTT_OFF;

		mg_set_protocol_mqtt(nc);
		mg_send_mqtt_handshake_opt(nc, mqtt_id, opts);
		logMessage(LOG_INFO, "MQTT: Subscribing mqtt with id of: %s\n", mqtt_id);
	} 
	break;

	case MG_EV_MQTT_CONNACK:
	{
		struct mg_mqtt_topic_expression topics[2];

		int qos = 0;// can't be bothered with ack, so set to 0

		logMessage(LOG_INFO, "MQTT: Connection acknowledged\n");
		mqtt_msg = (struct mg_mqtt_message*)ev_data;
		if (mqtt_msg->connack_ret_code != MG_EV_MQTT_CONNACK_ACCEPTED) 
		{
			logMessage(LOG_WARNING, "Got mqtt connection error: %d\n", mqtt_msg->connack_ret_code);
			_mqtt_exit_flag = true;
		}

		snprintf(aq_topic, 29, "%s/#", mqtt_aq_topic);
		if (mqtt_aq_topic != NULL && mqtt_dz_sub_topic != NULL) 
		{
			topics[0].topic = aq_topic;
			topics[0].qos = qos;
			topics[1].topic = mqtt_dz_sub_topic;
			topics[1].qos = qos;
			mg_mqtt_subscribe(nc, topics, 2, 42);
			logMessage(LOG_INFO, "MQTT: Subscribing to '%s'\n", aq_topic);
			logMessage(LOG_INFO, "MQTT: Subscribing to '%s'\n", mqtt_dz_sub_topic);
		}
		else if (mqtt_aq_topic != NULL) 
		{
			topics[0].topic = aq_topic;
			topics[0].qos = qos;
			mg_mqtt_subscribe(nc, topics, 1, 42);
			logMessage(LOG_INFO, "MQTT: Subscribing to '%s'\n", aq_topic);
		}
		else if (mqtt_dz_sub_topic != NULL) 
		{
			topics[0].topic = mqtt_dz_sub_topic;;
			topics[0].qos = qos;
			mg_mqtt_subscribe(nc, topics, 1, 42);
			logMessage(LOG_INFO, "MQTT: Subscribing to '%s'\n", mqtt_dz_sub_topic);
		}
	}
	break;

	case MG_EV_MQTT_PUBACK:
		mqtt_msg = (struct mg_mqtt_message*)ev_data;
		logMessage(LOG_INFO, "MQTT: Message publishing acknowledged (msg_id: %d)\n", mqtt_msg->message_id);
		break;

	case MG_EV_MQTT_SUBACK:
		logMessage(LOG_INFO, "MQTT: Subscription(s) acknowledged\n");
		snprintf(aq_topic, 24, "%s/%s", mqtt_aq_topic, MQTT_LWM_TOPIC);
		send_mqtt(nc, aq_topic, MQTT_ON);
		break;

	case MG_EV_MQTT_PUBLISH:
		mqtt_msg = (struct mg_mqtt_message*)ev_data;

		if (mqtt_msg->message_id != 0) 
		{
			logMessage(LOG_INFO, "MQTT: received (msg_id: %d), looks like my own message, ignoring\n", mqtt_msg->message_id);
		}
		// NSF Need to change strlen to a global so it's not executed every time we check a topic
		if (mqtt_aq_topic != NULL && strncmp(mqtt_msg->topic.p, mqtt_aq_topic, strlen(mqtt_aq_topic)) == 0) 
		{
			action_mqtt_message(nc, mqtt_msg);
		}
		if (mqtt_dz_sub_topic != NULL && strncmp(mqtt_msg->topic.p, mqtt_dz_sub_topic, strlen(mqtt_dz_sub_topic)) == 0) 
		{
			action_domoticz_mqtt_message(nc, mqtt_msg);
		}
		break;
	}
}

void start_mqtt(struct mg_mgr* mgr) 
{
	assert(0 != mgr);

	const char* mqtt_dz_pub_topic = CFG_MqttDzPubTopic();
	const char* mqtt_dz_sub_topic = CFG_MqttDzSubTopic();
	const char* mqtt_aq_topic = CFG_MqttAqTopic();
	const char* mqtt_server = CFG_MqttServer();

	logMessage(LOG_NOTICE, "Starting MQTT client to %s\n", mqtt_server);
	
	if (mqtt_server == NULL || (mqtt_aq_topic == NULL && mqtt_dz_pub_topic == NULL && mqtt_dz_sub_topic == NULL)) {
		return;
	}

	if (mg_connect(mgr, mqtt_server, ev_handler) == NULL) 
	{
		logMessage(LOG_ERR, "Failed to create MQTT listener to %s\n", mqtt_server);
	}
	else 
	{
		int i;
	
		for (i = 0; i < TOTAL_BUTTONS; i++) 
		{
			_last_mqtt_aqualinkdata.aqualinkleds[i].state = LED_S_UNKNOWN;
		}
		
		_last_mqtt_aqualinkdata.ar_swg_status = SWG_STATUS_UNKNOWN;
		_last_mqtt_aqualinkdata.battery = -1;
		_last_mqtt_aqualinkdata.frz_protect_state = -1;
		_last_mqtt_aqualinkdata.boost = -1;
		_last_mqtt_aqualinkdata.service_mode_state = -1;
		_mqtt_exit_flag = false; // set here to stop multiple connects, if it fails truley fails it will get set to false.
	}
}

bool start_net_services(struct mg_mgr* mgr, struct aqualinkdata* aqdata) 
{
	assert(0 != mgr);
	assert(0 != aqdata);

	struct mg_connection* nc;
	_aqualink_data = aqdata;

	const char* socket_port = CFG_SocketPort();
	const char* web_directory = CFG_WebDirectory();

	signal(SIGTERM, signal_handler);
	signal(SIGINT, signal_handler);
	setvbuf(stdout, NULL, _IOLBF, 0);
	setvbuf(stderr, NULL, _IOLBF, 0);

	mg_mgr_init(mgr, NULL);
	logMessage(LOG_NOTICE, "Starting web server on port %s\n", socket_port);
	nc = mg_bind(mgr, socket_port, ev_handler);
	if (nc == NULL) 
	{
		logMessage(LOG_ERR, "Failed to create listener\n");
		return false;
	}

	// Set up HTTP server parameters
	mg_set_protocol_http_websocket(nc);
	s_http_server_opts.document_root = web_directory;  // Serve current directory
	s_http_server_opts.enable_directory_listing = "yes";

#ifndef MG_DISABLE_MQTT
	// Start MQTT
	start_mqtt(mgr);
#endif

	return true;
}
