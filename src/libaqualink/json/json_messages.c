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
#include "json_messages.h"
#include "json_string_utils.h"
#include "json_serializer.h"

#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <json-c/json_object.h>

#include "config/config.h"
#include "hardware/buttons/rs_buttons.h"
#include "logging/logging.h"
#include "utility/utils.h"
#include "version/version.h"
#include "aqualink.h"

/*
const char* getStatus(struct aqualinkdata* aqdata)
{
	if (aqdata->active_thread.thread_id != 0 && !aqdata->simulate_panel) 
	{
		return JSON_PROGRAMMING;
	}

	//if (aqdata->last_message != NULL && aq_stristr(aqdata->last_message, "SERVICE") != NULL ) {
	if (aqdata->service_mode_state == ON) 
	{
		return JSON_SERVICE;
	}

	if (aqdata->service_mode_state == FLASH) 
	{
		return JSON_TIMEOUT;
	}

	if (aqdata->last_display_message[0] != '\0') 
	{
		int i;
		for (i = 0; i < strlen(aqdata->last_display_message); i++) 
		{
			if (aqdata->last_display_message[i] <= 31 || aqdata->last_display_message[i] >= 127) 
			{
				aqdata->last_display_message[i] = ' ';
			}
			else 
			{
				switch (aqdata->last_display_message[i]) 
				{
				case '"':
				case '/':
				case '\n':
				case '\t':
				case '\f':
				case '\r':
				case '\b':
					aqdata->last_display_message[i] = ' ';
					break;
				}
			}
		}

		return aqdata->last_display_message;
	}

	return JSON_READY;
}

static int serialise_json_object(json_object* jobj, char* buffer, const int buffer_length)
{
	assert(0 != jobj);
	assert(0 != buffer);

	const char* serialised_json = json_object_to_json_string_ext(jobj, JSON_C_TO_STRING_PRETTY);
	int copy_length = 0;

	if (0 != serialised_json)
	{
		copy_length = (strlen(serialised_json) < buffer_length) ? strlen(serialised_json) : buffer_length;

		memset(buffer, 0, buffer_length);
		memcpy(buffer, serialised_json, copy_length);

		buffer[copy_length] = 0;
	}

	return copy_length;
}

int build_mqtt_status_JSON(char* buffer, int buffer_length, int idx, int nvalue, double tvalue)
{
	assert(0 != buffer);

	int json_length = 0;

	json_object* jobj = json_object_new_object();

	json_object_object_add(jobj, "idx", json_object_new_int(idx));
	json_object_object_add(jobj, "nvalue", json_object_new_int(nvalue));

	if (TEMP_UNKNOWN != tvalue)
	{
		json_object_object_add(jobj, "stype", json_object_new_string("SetPoint"));

		json_object* svalue = json_object_new_double(tvalue);
		json_object_set_serializer(svalue, double_to_json_string, "%.01f", NULL);
		json_object_object_add(jobj, "svalue", svalue);
	}
	
	json_length = serialise_json_object(jobj, buffer, buffer_length);

	json_object_put(jobj);

	return json_length;
}

int build_mqtt_status_message_JSON(char* buffer, int buffer_length, int idx, int nvalue, char* svalue)
{
	assert(0 != buffer);
	assert(0 != svalue);

	int json_length = 0;

	json_object* jobj = json_object_new_object();

	json_object_object_add(jobj, "idx", json_object_new_int(idx));
	json_object_object_add(jobj, "nvalue", json_object_new_int(nvalue));
	json_object_object_add(jobj, "svalue", json_object_new_string(svalue));

	json_length = serialise_json_object(jobj, buffer, buffer_length);

	json_object_put(jobj);

	return json_length;
}

int build_aqualink_status_JSON(struct aqualinkdata* aqdata, char* buffer, int buffer_length)
{
	assert(0 != aqdata);
	assert(0 != buffer);

	int json_length = 0;

	json_object* jobj = json_object_new_object();

	json_object_object_add(jobj, "type", json_object_new_string("status"));
	json_object_object_add(jobj, "status", json_object_new_string(getStatus(aqdata)));
	json_object_object_add(jobj, "version", json_object_new_string(aqdata->version));
	json_object_object_add(jobj, "aqualinkd_version", json_object_new_string(AQUALINKD_VERSION));
	json_object_object_add(jobj, "date", json_object_new_string(aqdata->date));
	json_object_object_add(jobj, "time", json_object_new_string(aqdata->time));

	json_object_object_add(jobj, "pool_htr_set_pnt", json_object_new_int(aqdata->pool_htr_set_point));
	json_object_object_add(jobj, "spa_htr_set_pnt", json_object_new_int(aqdata->spa_htr_set_point));
	json_object_object_add(jobj, "frz_protect_set_pnt", json_object_new_int(aqdata->frz_protect_set_point));
	   

	if (TEMP_UNKNOWN != aqdata->air_temp) { json_object_object_add(jobj, "air_temp", json_object_new_int(aqdata->air_temp)); }
	if (TEMP_UNKNOWN != aqdata->pool_temp) { json_object_object_add(jobj, "pool_temp", json_object_new_int(aqdata->pool_temp)); }
	if (TEMP_UNKNOWN != aqdata->spa_temp) { json_object_object_add(jobj, "spa_temp", json_object_new_int(aqdata->spa_temp)); }

	if (TEMP_UNKNOWN != aqdata->swg_percent) { json_object_object_add(jobj, "swg_percent", json_object_new_int(aqdata->swg_percent)); }
	if (TEMP_UNKNOWN != aqdata->swg_ppm) { json_object_object_add(jobj, "swg_ppm", json_object_new_int(aqdata->swg_ppm)); }

	if (aqdata->temp_units == FAHRENHEIT) 
	{
		json_object_object_add(jobj, "temp_units", json_object_new_string(JSON_FAHRENHEIT));
	}
	else if (aqdata->temp_units == CELSIUS)
	{
		json_object_object_add(jobj, "temp_units", json_object_new_string(JSON_CELSIUS));
	}
	else
	{
		json_object_object_add(jobj, "temp_units", json_object_new_string(JSON_UNKNOWN));
	}

	json_object_object_add(jobj, "battery", json_object_new_string((OK == aqdata->battery) ? JSON_OK : JSON_LOW));

	if (aqdata->swg_percent == 101) { json_object_object_add(jobj, "swg_boost_msg", json_object_new_string(aqdata->boost_msg)); }

	{
		json_object* jleds = json_object_new_object();
		int button_index;

		for (button_index = FilterPump; button_index < ButtonTypeCount; ++button_index)
		{
			assert(0 != aqdata->aqbuttons[button_index].name);
			assert(0 != aqdata->aqbuttons[button_index].label);
			assert(0 != aqdata->aqbuttons[button_index].pda_label);
			assert(0 != aqdata->aqbuttons[button_index].led);

			json_object_object_add(jleds, aqdata->aqbuttons[button_index].name, json_object_new_string(LED2text(aqdata->aqbuttons[button_index].led->state)));
		}

		if (TEMP_UNKNOWN != aqdata->swg_percent)
		{
			json_object_object_add(jleds, SWG_TOPIC, json_object_new_string(LED2text(get_swg_led_state(aqdata))));
			json_object_object_add(jleds, SWG_BOOST_TOPIC, json_object_new_string((aqdata->boost) ? JSON_ON : JSON_OFF));
		}
	   
		if (TEMP_UNKNOWN != aqdata->frz_protect_set_point) { json_object_object_add(jleds, FREEZE_PROTECT, json_object_new_string((ON == aqdata->frz_protect_state) ? JSON_ON : JSON_ENABLED)); }

		json_object_object_add(jobj, "leds", jleds);
	}

	{
		int pump_index;

		for (pump_index = 0; pump_index < MAX_PUMPS; ++pump_index)
		{
			if ((TEMP_UNKNOWN != aqdata->pumps[pump_index].rpm) || (TEMP_UNKNOWN != aqdata->pumps[pump_index].gph) || (TEMP_UNKNOWN != aqdata->pumps[pump_index].watts))
			{
				assert(0 != aqdata->pumps[pump_index].button->label);
				assert(0 != aqdata->pumps[pump_index].button->name);

				json_object* jpumps = json_object_new_object();
				json_object_object_add(jpumps, "name", json_object_new_string(aqdata->pumps[pump_index].button->label));
				json_object_object_add(jpumps, "id", json_object_new_string(aqdata->pumps[pump_index].button->name));
				json_object_object_add(jpumps, "RPM", json_object_new_int(aqdata->pumps[pump_index].rpm));
				json_object_object_add(jpumps, "GPH", json_object_new_int(aqdata->pumps[pump_index].gph));
				json_object_object_add(jpumps, "Watts", json_object_new_int(aqdata->pumps[pump_index].watts));

				const int MAX_PUMP_OBJECT_NAME_LEN = 9; // i.e. supports "Pump_" + ### + NULL.
				char pump_object_name[MAX_PUMP_OBJECT_NAME_LEN];

				snprintf(pump_object_name, MAX_PUMP_OBJECT_NAME_LEN, "Pump_%d", pump_index + 1);

				json_object_object_add(jobj, pump_object_name, jpumps);
			}
		}
	}

	json_length = serialise_json_object(jobj, buffer, buffer_length);

	json_object_put(jobj);

	return json_length;
}

int build_aqualink_error_status_JSON(char* buffer, int buffer_length, char* msg)
{
	assert(0 != buffer);
	assert(0 != msg);

	int json_length = 0;

	json_object* jobj = json_object_new_object();

	json_object_object_add(jobj, "type", json_object_new_string("status"));
	json_object_object_add(jobj, "status", json_object_new_string(msg));

	json_length = serialise_json_object(jobj, buffer, buffer_length);

	json_object_put(jobj);

	return json_length;
}

char* get_aux_information(aqkey* button, struct aqualinkdata* aqdata, char* buffer)
{
	int i;
	int length = 0;
	buffer[0] = '\0';

	for (i = 0; i < MAX_PUMPS; i++) {
		if (button == aqdata->pumps[i].button) {
			if (aqdata->pumps[i].rpm != TEMP_UNKNOWN || aqdata->pumps[i].gph != TEMP_UNKNOWN || aqdata->pumps[i].watts != TEMP_UNKNOWN) {
				length += sprintf(buffer, ",\"Pump_RPM\":\"%d\",\"Pump_GPH\":\"%d\",\"Pump_Watts\":\"%d\"", aqdata->pumps[i].rpm, aqdata->pumps[i].gph, aqdata->pumps[i].watts);
				break;
			}
		}
	}

	return buffer;
}

int build_device_JSON(struct aqualinkdata* aqdata, int programable_switch1, int programable_switch2, char* buffer, int size, bool homekit)
{
	char aux_info[AUX_BUFFER_SIZE];
	memset(&buffer[0], 0, size);
	int length = 0;
	int i;

	// IF temp units are F assume homekit is using F
	bool homekit_f = (homekit && aqdata->temp_units == FAHRENHEIT);

	length += sprintf(buffer + length, "{\"type\": \"devices\"");
	length += sprintf(buffer + length, ",\"date\":\"%s\"", aqdata->date);//"09/01/16 THU",
	length += sprintf(buffer + length, ",\"time\":\"%s\"", aqdata->time);//"1:16 PM",
	if (aqdata->temp_units == FAHRENHEIT)
	{
		length += sprintf(buffer + length, ",\"temp_units\":\"%s\"", JSON_FAHRENHEIT);
	}
	else if (aqdata->temp_units == CELSIUS)
	{
		length += sprintf(buffer + length, ",\"temp_units\":\"%s\"", JSON_CELSIUS);
	}
	else
	{
		length += sprintf(buffer + length, ",\"temp_units\":\"%s\"", JSON_UNKNOWN);
	}

	length += sprintf(buffer + length, ", \"devices\": [");

	for (i = FilterPump; i < ButtonTypeCount; i++)
	{
		if (strcmp(BTN_POOL_HTR, aqdata->aqbuttons[i].name) == 0 && aqdata->pool_htr_set_point != TEMP_UNKNOWN) {
			length += sprintf(buffer + length, "{\"type\": \"setpoint_thermo\", \"id\": \"%s\", \"name\": \"%s\", \"state\": \"%s\", \"status\": \"%s\", \"spvalue\": \"%.*f\", \"value\": \"%.*f\", \"int_status\": \"%d\" },",
				aqdata->aqbuttons[i].name,
				aqdata->aqbuttons[i].label,
				aqdata->aqbuttons[i].led->state == ON ? JSON_ON : JSON_OFF,
				LED2text(aqdata->aqbuttons[i].led->state),
				((homekit) ? 2 : 0),
				((homekit_f) ? degFtoC(aqdata->pool_htr_set_point) : aqdata->pool_htr_set_point),
				((homekit) ? 2 : 0),
				((homekit_f) ? degFtoC(aqdata->pool_temp) : aqdata->pool_temp),
				LED2int(aqdata->aqbuttons[i].led->state));
		}
		else if (strcmp(BTN_SPA_HTR, aqdata->aqbuttons[i].name) == 0 && aqdata->spa_htr_set_point != TEMP_UNKNOWN) {
			length += sprintf(buffer + length, "{\"type\": \"setpoint_thermo\", \"id\": \"%s\", \"name\": \"%s\", \"state\": \"%s\", \"status\": \"%s\", \"spvalue\": \"%.*f\", \"value\": \"%.*f\", \"int_status\": \"%d\" },",
				aqdata->aqbuttons[i].name,
				aqdata->aqbuttons[i].label,
				aqdata->aqbuttons[i].led->state == ON ? JSON_ON : JSON_OFF,
				LED2text(aqdata->aqbuttons[i].led->state),
				((homekit) ? 2 : 0),
				((homekit_f) ? degFtoC(aqdata->spa_htr_set_point) : aqdata->spa_htr_set_point),
				((homekit) ? 2 : 0),
				((homekit_f) ? degFtoC(aqdata->spa_temp) : aqdata->spa_temp),
				LED2int(aqdata->aqbuttons[i].led->state));
		}
		else if ((programable_switch1 > 0 && programable_switch1 == i) ||
			(programable_switch2 > 0 && programable_switch2 == i)) {
			length += sprintf(buffer + length, "{\"type\": \"switch_program\", \"id\": \"%s\", \"name\": \"%s\", \"state\": \"%s\", \"status\": \"%s\", \"int_status\": \"%d\" %s},",
				aqdata->aqbuttons[i].name,
				aqdata->aqbuttons[i].label,
				aqdata->aqbuttons[i].led->state == ON ? JSON_ON : JSON_OFF,
				LED2text(aqdata->aqbuttons[i].led->state),
				LED2int(aqdata->aqbuttons[i].led->state),
				get_aux_information(&aqdata->aqbuttons[i], aqdata, aux_info));
		}
		else {
			length += sprintf(buffer + length, "{\"type\": \"switch\", \"id\": \"%s\", \"name\": \"%s\", \"state\": \"%s\", \"status\": \"%s\", \"int_status\": \"%d\" %s},",
				aqdata->aqbuttons[i].name,
				aqdata->aqbuttons[i].label,
				aqdata->aqbuttons[i].led->state == ON ? JSON_ON : JSON_OFF,
				LED2text(aqdata->aqbuttons[i].led->state),
				LED2int(aqdata->aqbuttons[i].led->state),
				get_aux_information(&aqdata->aqbuttons[i], aqdata, aux_info));
		}
	}

	if (aqdata->frz_protect_set_point != TEMP_UNKNOWN && aqdata->air_temp != TEMP_UNKNOWN) {
		length += sprintf(buffer + length, "{\"type\": \"setpoint_freeze\", \"id\": \"%s\", \"name\": \"%s\", \"state\": \"%s\", \"status\": \"%s\", \"spvalue\": \"%.*f\", \"value\": \"%.*f\", \"int_status\": \"%d\" },",
			FREEZE_PROTECT,
			"Freeze Protection",
			//JSON_OFF,
			aqdata->frz_protect_state == ON ? JSON_ON : JSON_OFF,
			//JSON_ENABLED,
			aqdata->frz_protect_state == ON ? LED2text(ON) : LED2text(ENABLE),
			((homekit) ? 2 : 0),
			((homekit_f) ? degFtoC(aqdata->frz_protect_set_point) : aqdata->frz_protect_set_point),
			((homekit) ? 2 : 0),
			((homekit_f) ? degFtoC(aqdata->air_temp) : aqdata->air_temp),
			aqdata->frz_protect_state == ON ? 1 : 0);
	}

	if (aqdata->swg_percent != TEMP_UNKNOWN) {
		length += sprintf(buffer + length, "{\"type\": \"setpoint_swg\", \"id\": \"%s\", \"name\": \"%s\", \"state\": \"%s\", \"status\": \"%s\", \"spvalue\": \"%.*f\", \"value\": \"%.*f\", \"int_status\": \"%d\" },",
			SWG_TOPIC,
			"Salt Water Generator",
			aqdata->ar_swg_status == SWG_STATUS_OFF ? JSON_OFF : JSON_ON,
			//aqdata->ar_swg_status == SWG_STATUS_OFF?JSON_OFF:JSON_ON,
			LED2text(get_swg_led_state(aqdata)),
			((homekit) ? 2 : 0),
			((homekit_f) ? degFtoC(aqdata->swg_percent) : aqdata->swg_percent),
			((homekit) ? 2 : 0),
			((homekit_f) ? degFtoC(aqdata->swg_percent) : aqdata->swg_percent),
			aqdata->ar_swg_status == SWG_STATUS_OFF ? LED2int(OFF) : LED2int(ON));

		//length += sprintf(buffer+length, "{\"type\": \"value\", \"id\": \"%s\", \"name\": \"%s\", \"state\": \"%s\", \"value\": \"%d\" },",
		length += sprintf(buffer + length, "{\"type\": \"value\", \"id\": \"%s\", \"name\": \"%s\", \"state\": \"%s\", \"value\": \"%.*f\" },",
			((homekit_f) ? SWG_PERCENT_F_TOPIC : SWG_PERCENT_TOPIC),
			"Salt Water Generator Percent",
			"on",
			((homekit_f) ? 2 : 0),
			((homekit_f) ? degFtoC(aqdata->swg_percent) : aqdata->swg_percent));
		if (!homekit) { // For the moment keep boost off homekit   
			length += sprintf(buffer + length, "{\"type\": \"switch\", \"id\": \"%s\", \"name\": \"%s\", \"state\": \"%s\", \"status\": \"%s\", \"int_status\": \"%d\"},",
				SWG_BOOST_TOPIC,
				"SWG Boost",
				aqdata->boost ? JSON_ON : JSON_OFF,
				aqdata->boost ? JSON_ON : JSON_OFF,
				aqdata->boost ? LED2int(ON) : LED2int(OFF));
		}
	}

	if (aqdata->swg_ppm != TEMP_UNKNOWN) {

		length += sprintf(buffer + length, "{\"type\": \"value\", \"id\": \"%s\", \"name\": \"%s\", \"state\": \"%s\", \"value\": \"%.*f\" },",
			((homekit_f) ? SWG_PPM_F_TOPIC : SWG_PPM_TOPIC),
			"Salt Level PPM",
			"on",
			((homekit) ? 2 : 0),
			((homekit_f) ? roundf(degFtoC(aqdata->swg_ppm)) : aqdata->swg_ppm));

	//	
	  //length += sprintf(buffer+length, "{\"type\": \"value\", \"id\": \"%s\", \"name\": \"%s\", \"state\": \"%s\", \"value\": \"%d\" },",
		//							  SWG_PPM_TOPIC,
			//						  "Salt Level PPM",
				//					  "on",
					//				  aqdata->swg_ppm);
	  //

	}

	length += sprintf(buffer + length, "{\"type\": \"temperature\", \"id\": \"%s\", \"name\": \"%s\", \"state\": \"%s\", \"value\": \"%.*f\" },",
		AIR_TEMP_TOPIC,
		//AIR_TEMPERATURE,
		"Pool Air Temperature",
		"on",
		((homekit) ? 2 : 0),
		((homekit_f) ? degFtoC(aqdata->air_temp) : aqdata->air_temp));
	length += sprintf(buffer + length, "{\"type\": \"temperature\", \"id\": \"%s\", \"name\": \"%s\", \"state\": \"%s\", \"value\": \"%.*f\" },",
		POOL_TEMP_TOPIC,
		//POOL_TEMPERATURE,
		"Pool Water Temperature",
		"on",
		((homekit) ? 2 : 0),
		((homekit_f) ? degFtoC(aqdata->air_temp) : aqdata->pool_temp));
	length += sprintf(buffer + length, "{\"type\": \"temperature\", \"id\": \"%s\", \"name\": \"%s\", \"state\": \"%s\", \"value\": \"%.*f\" }",
		SPA_TEMP_TOPIC,
		//SPA_TEMPERATURE,
		"Spa Water Temperature",
		"on",
		((homekit) ? 2 : 0),
		((homekit_f) ? degFtoC(aqdata->air_temp) : aqdata->spa_temp));

	//
	//  length += sprintf(buffer+length,  "], \"aux_device_detail\": [");
	//  for (i=0; i < MAX_PUMPS; i++) {
	//  }
	
	length += sprintf(buffer + length, "]}");

	DEBUG("WEB: homebridge used %d of %d", length, size);

	buffer[length] = '\0';

	return strlen(buffer);

	//return length;
}

int build_aux_labels_JSON(struct aqualinkdata* aqdata, char* buffer, int size)
{
	memset(&buffer[0], 0, size);
	int length = 0;
	int i;

	length += sprintf(buffer + length, "{\"type\": \"aux_labels\"");

	for (i = FilterPump; i < ButtonTypeCount; i++)
	{
		length += sprintf(buffer + length, ",\"%s\": \"%s\"", aqdata->aqbuttons[i].name, aqdata->aqbuttons[i].label);
	}

	length += sprintf(buffer + length, "}");

	return length;
}

// WS Received '{"parameter":"SPA_HTR","value":99}'
// WS Received '{"command":"KEY_HTR_POOL"}'
// WS Received '{"command":"GET_AUX_LABELS"}'

bool parseJSONwebrequest(char* buffer, struct JSONwebrequest* request)
{
	int i = 0;
	int found = 0;
	bool reading = false;

	request->first.key = NULL;
	request->first.value = NULL;
	request->second.key = NULL;
	request->second.value = NULL;
	request->third.key = NULL;
	request->third.value = NULL;

	int length = strlen(buffer);

	while (i < length)
	{
		switch (buffer[i]) {
		case '{':
		case '"':
		case '}':
		case ':':
		case ',':
		case ' ':
			// Ignore space , : if reading a string
			if (reading == true && buffer[i] != ' ' && buffer[i] != ',' && buffer[i] != ':') {
				reading = false;
				buffer[i] = '\0';
				found++;
			}
			break;

		default:
			if (reading == false) {
				reading = true;
				switch (found) {
				case 0:
					request->first.key = &buffer[i];
					break;
				case 1:
					request->first.value = &buffer[i];
					break;
				case 2:
					request->second.key = &buffer[i];
					break;
				case 3:
					request->second.value = &buffer[i];
					break;
				case 4:
					request->third.key = &buffer[i];
					break;
				case 5:
					request->third.value = &buffer[i];
					break;
				}
			}
			break;
		}

		if (found >= 6) {
			break;
		}

		i++;
	}

	return true;
}

bool parseJSONmqttrequest(const char* str, size_t len, int* idx, int* nvalue, char* svalue) {
	unsigned int i = 0;
	int found = 0;

	svalue[0] = '\0';

	for (i = 0; i < len && str[i] != '\0'; i++) {
		if (str[i] == '"') {
			if (strncmp("\"idx\"", (char*)&str[i], 5) == 0) {
				i = i + 5;
				for (; str[i] != ',' && str[i] != '\0'; i++) {
					if (str[i] == ':') {
						*idx = atoi(&str[i + 1]);
						found++;
					}
				}
			}
			else if (strncmp("\"nvalue\"", (char*)&str[i], 8) == 0) {
				i = i + 8;
				for (; str[i] != ',' && str[i] != '\0'; i++) {
					if (str[i] == ':') {
						*nvalue = atoi(&str[i + 1]);
						found++;
					}
				}
			}
			else if (strncmp("\"svalue1\"", (char*)&str[i], 9) == 0) {
				i = i + 9;
				for (; str[i] != ',' && str[i] != '\0'; i++) {
					if (str[i] == ':') {
						while (str[i] == ':' || str[i] == ' ' || str[i] == '"' || str[i] == '\'') { i++; }
						int j = i + 1;
						while (str[j] != '"' && str[j] != '\'' && str[j] != ',' && str[j] != '}') { j++; }
						strncpy(svalue, &str[i], ((j - i) > DZ_SVALUE_LEN ? DZ_SVALUE_LEN : (j - i)));
						svalue[((j - i) > DZ_SVALUE_LEN ? DZ_SVALUE_LEN : (j - i))] = '\0'; // Simply force the last termination
						found++;
					}
				}
			}
			if (found >= 4) {
				return true;
			}
		}
	}
	// Just incase svalue is not found, we really don;t care for most devices.
	if (found >= 2) {
		return true;
	}
	return false;
}
*/