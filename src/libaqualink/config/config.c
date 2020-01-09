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

#include "config.h"
#include "config_helpers.h"
#include "config_parsers.h"
#include "config_private.h"
#include "config_validators.h"

#include <stdlib.h>
#include <string.h>
#include <confuse.h>

#include "logging/logging.h"
#include "serial/aq_serial.h"
#include "aqualink.h"  // Included for TEMP_UNKNOWN
#include "utils.h"

static cfg_opt_t _config_parameters_defaults[] =
{
	CFG_STR(CONFIG_STR_CONFIG_FILE, DEFAULT_CONFIG_FILE, CFGF_NONE),
	CFG_STR(CONFIG_STR_PID_FILE, DEFAULT_PID_FILE, CFGF_NONE),
	CFG_STR(CONFIG_STR_SERIAL_PORT, DEFAULT_SERIALPORT, CFGF_NONE),
	CFG_INT_CB(CONFIG_INT_LOG_LEVEL, DEFAULT_LOG_LEVEL, CFGF_NONE, parse_log_level),
	CFG_INT(CONFIG_INT_SOCKET_PORT, DEFAULT_WEBPORT, CFGF_NONE),
	CFG_INT(CONFIG_INT_SSL_SOCKET_PORT, DEFAULT_SSL_WEBPORT, CFGF_NONE),
	CFG_STR(CONFIG_STR_WEB_DIRECTORY, DEFAULT_WEBROOT, CFGF_NONE),
	CFG_BOOL(CONFIG_BOOL_INSECURE, DEFAULT_INSECURE, CFGF_NONE),
	CFG_INT(CONFIG_INT_DEVICE_ID, 0x0A, CFGF_NONE), // strtoul(DEFAULT_DEVICE_ID, NULL, 16)
	CFG_BOOL(CONFIG_BOOL_OVERRIDE_FREEZE_PROTECT, FALSE, CFGF_NONE),

	CFG_STR(CONFIG_STR_MQTT_DZ_SUB_TOPIC, DEFAULT_MQTT_DZ_OUT, CFGF_NONE),
	CFG_STR(CONFIG_STR_MQTT_DZ_PUB_TOPIC, DEFAULT_MQTT_DZ_IN, CFGF_NONE),
	CFG_STR(CONFIG_STR_MQTT_AQ_TOPIC, DEFAULT_MQTT_AQ_TP, CFGF_NONE),
	CFG_STR(CONFIG_STR_MQTT_SERVER, DEFAULT_MQTT_SERVER, CFGF_NONE),
	CFG_STR(CONFIG_STR_MQTT_USER, DEFAULT_MQTT_USER, CFGF_NONE),
	CFG_STR(CONFIG_STR_MQTT_PASSWD, DEFAULT_MQTT_PASSWD, CFGF_NONE),
	CFG_STR(CONFIG_STR_MQTT_ID, "", CFGF_NONE),

	CFG_INT(CONFIG_INT_DZIDX_AIR_TEMP, TEMP_UNKNOWN, CFGF_NONE),
	CFG_INT(CONFIG_INT_DZIDX_POOL_WATER_TEMP, TEMP_UNKNOWN, CFGF_NONE),
	CFG_INT(CONFIG_INT_DZIDX_SPA_WATER_TEMP, TEMP_UNKNOWN, CFGF_NONE),
	CFG_INT(CONFIG_INT_DZIDX_SWG_PERCENT, 0, CFGF_NONE),
	CFG_INT(CONFIG_INT_DZIDX_SWG_PPM, 0, CFGF_NONE),
	CFG_INT(CONFIG_INT_DZIDX_SWG_STATUS, 0, CFGF_NONE),

	CFG_FLOAT(CONFIG_FLOAT_LIGHT_PROGRAMMING_MODE, 0.0f, CFGF_NONE),
	CFG_INT(CONFIG_INT_LIGHT_PROGRAMMING_INITIAL_ON, 15, CFGF_NONE),
	CFG_INT(CONFIG_INT_LIGHT_PROGRAMMING_INITIAL_OFF, 12, CFGF_NONE),
	CFG_INT(CONFIG_INT_LIGHT_PROGRAMMING_BUTTON_POOL, TEMP_UNKNOWN, CFGF_NONE),
	CFG_INT(CONFIG_INT_LIGHT_PROGRAMMING_BUTTON_SPA, TEMP_UNKNOWN, CFGF_NONE),

	CFG_BOOL(CONFIG_BOOL_NO_DAEMONIZE, false, CFGF_NONE),
	CFG_STR(CONFIG_STR_LOG_FILE, DEFAULT_LOG_FILE, CFGF_NONE),

	CFG_BOOL(CONFIG_BOOL_PDA_MODE, false, CFGF_NONE),
	CFG_BOOL(CONFIG_BOOL_PDA_SLEEP_MODE, false, CFGF_NONE),
	CFG_BOOL(CONFIG_BOOL_CONVERT_MQTT_TEMP_TO_C, true, CFGF_NONE),
	CFG_BOOL(CONFIG_BOOL_CONVERT_DZ_TEMP_TO_C, true, CFGF_NONE),
	CFG_BOOL(CONFIG_BOOL_REPORT_ZERO_POOL_TEMP, false, CFGF_NONE),
	CFG_BOOL(CONFIG_BOOL_REPORT_ZERO_SPA_TEMP, false, CFGF_NONE),
	CFG_BOOL(CONFIG_BOOL_READ_ALL_DEVICES, true, CFGF_NONE),
	CFG_BOOL(CONFIG_BOOL_USE_PANEL_AUX_LABELS, false, CFGF_NONE),
	CFG_BOOL(CONFIG_BOOL_FORCE_SWG, false, CFGF_NONE),
	CFG_INT(CONFIG_INT_SWG_ZERO_IGNORE, DEFAILT_SWG_ZERO_IGNORE_COUNT, CFGF_NONE),
	CFG_BOOL(CONFIG_BOOL_READ_PENTAIR_PACKETS, false, CFGF_NONE),
	CFG_BOOL(CONFIG_BOOL_DISPLAY_WARNINGS_IN_WEB, false, CFGF_NONE),
	CFG_BOOL(CONFIG_BOOL_DEBUG_RSPROTOCOL_PACKETS, false, CFGF_NONE),
	CFG_BOOL(CONFIG_BOOL_LOG_RAW_RS_BYTES, false, CFGF_NONE),
	CFG_STR(CONFIG_STR_LOG_RAW_RS_BYTES_LOGFILE, DEFAULT_RS485_LOG_FILE, CFGF_NONE),

	CFG_STR(CONFIG_STR_BUTTON_FILTERPUMP_LABEL, "Filter Pump", CFGF_NONE),
	CFG_STR(CONFIG_STR_BUTTON_SPAMODE_LABEL, "SPA Mode", CFGF_NONE),
	CFG_STR(CONFIG_STR_BUTTON_AUX1_LABEL, "Aux 1", CFGF_NONE),
	CFG_STR(CONFIG_STR_BUTTON_AUX2_LABEL, "Aux 2", CFGF_NONE),
	CFG_STR(CONFIG_STR_BUTTON_AUX3_LABEL, "Aux 3", CFGF_NONE),
	CFG_STR(CONFIG_STR_BUTTON_AUX4_LABEL, "Aux 4", CFGF_NONE),
	CFG_STR(CONFIG_STR_BUTTON_AUX5_LABEL, "Aux 5", CFGF_NONE),
	CFG_STR(CONFIG_STR_BUTTON_AUX6_LABEL, "Aux 6", CFGF_NONE),
	CFG_STR(CONFIG_STR_BUTTON_AUX7_LABEL, "Aux 7", CFGF_NONE),
	CFG_STR(CONFIG_STR_BUTTON_POOLHEATER_LABEL, "Pool Heater", CFGF_NONE),
	CFG_STR(CONFIG_STR_BUTTON_SPAHEATER_LABEL, "Spa Heater", CFGF_NONE),
	CFG_STR(CONFIG_STR_BUTTON_SOLARHEATER_LABEL, "Solar Heater", CFGF_NONE),

	CFG_END()
};

// This is the "global" configuration parameter store (initialised in config.c).
cfg_t* _config_parameters = 0;

void initialise_config_parameters()
{
	_config_parameters = cfg_init(_config_parameters_defaults, CFGF_NONE);

	// Install any required validators (e.g. logging levels)
	cfg_set_validate_func(_config_parameters, CONFIG_INT_LOG_LEVEL, validate_loglevel);
}

void handle_configuration_file_options()
{
	DEBUG("Attempting to load configuration file: %s", CFG_ConfigFile());

	const char* configuration_file = CFG_ConfigFile();

	if (0 == configuration_file)
	{
		DEBUG("No configuration file specified...using defaults");
	}
	else if (CFG_PARSE_ERROR == cfg_parse(_config_parameters, configuration_file))
	{
		DEBUG("Failed to load specified configuration file...using defaults");
		exit(EXIT_FAILURE);
	}
	else
	{
		DEBUG("Loaded configuration from %s", configuration_file);
	}
}

bool write_configuration_options_to_file()
{
	FILE* dest_config_file = fopen(CFG_ConfigFile(), "w");
	bool write_successful = false;

	if (0 == dest_config_file)
	{
		WARN("Failed to open configuration file (%s) to save current configuration", CFG_ConfigFile());
	}
	else if (0 != cfg_print(_config_parameters, dest_config_file))
	{
		WARN("Failed to write to configuration file (%s) while saving current configuration", CFG_ConfigFile());
	}
	else
	{
		DEBUG("Wrote current configuration out to configuration file", CFG_ConfigFile());
		write_successful = true;
	}

	fclose(dest_config_file);
	return write_successful;
}

char* cleanalloc(char* str)
{
	char* result;
	str = cleanwhitespace(str);

	result = (char*)malloc(strlen(str) + 1);
	strcpy(result, str);
	return result;
}

#ifdef _OLD_CFG_REMOVE_ME

bool setConfigValue(struct aqconfig* config_parameters, struct aqualinkdata* aqdata, char* param, char* value) {

	///SNIP SNIP SNIP

  else if (aq_strnicmp(param, "button_", 7) == 0) {
  int num = strtoul(param + 7, NULL, 10) - 1;
  if (aq_strnicmp(param + 9, "_label", 6) == 0) {
	  aqdata->aqbuttons[num].label = cleanalloc(value);
	  rtn = true;
  }
  else if (aq_strnicmp(param + 9, "_dzidx", 6) == 0) {
	  aqdata->aqbuttons[num].dz_idx = strtoul(value, NULL, 10);
	  rtn = true;
  }
  else if (aq_strnicmp(param + 9, "_PDA_label", 10) == 0) {
	  aqdata->aqbuttons[num].pda_label = cleanalloc(value);
	  rtn = true;
  }
  else if (aq_strnicmp(param + 9, "_pumpID", 7) == 0) {
	  //96 to 111 = Pentair, 120 to 123 = Jandy
	  if (pi < MAX_PUMPS) {
		  aqdata->pumps[pi].button = &aqdata->aqbuttons[num];
		  aqdata->pumps[pi].pumpID = strtoul(cleanalloc(value), NULL, 16);
		  if (aqdata->pumps[pi].pumpID < 119)
			  aqdata->pumps[pi].ptype = PENTAIR;
		  else
			  aqdata->pumps[pi].ptype = JANDY;
		  pi++;
	  }
	  else {
		  ERROR("Config error, VSP Pumps limited to %d, ignoring %s'", MAX_PUMPS, param);
	  }
	  rtn = true;
  }
  }

  return rtn;
}

#endif // _OLD_CFG_REMOVE_ME
