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
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

#include <libgen.h>

#include <sys/ioctl.h>
//#include <sys/socket.h>
//#include <sys/time.h>
//#include <syslog.h>
//#include <unistd.h>
#include <netdb.h>
//#include <linux/if.h>
//#include <sys/types.h>
#include <unistd.h>
#include <net/if.h>
#include <confuse.h>

#include "config.h"
#include "utils.h"
#include "aq_serial.h" 
#include "aqualink.h"  // Included for TEMP_UNKNOWN

static cfg_opt_t _config_parameters_defaults[] =
{
    CFG_STR(CONFIG_STR_CONFIG_FILE, DEFAULT_CONFIG_FILE, CFGF_NONE),
    CFG_STR(CONFIG_STR_SERIAL_PORT, DEFAULT_SERIALPORT, CFGF_NONE),
    CFG_INT(CONFIG_INT_LOG_LEVEL, DEFAULT_LOG_LEVEL, CFGF_NONE),
    CFG_STR(CONFIG_STR_SOCKET_PORT, DEFAULT_WEBPORT, CFGF_NONE),
    CFG_STR(CONFIG_STR_WEB_DIRECTORY, DEFAULT_WEBROOT, CFGF_NONE),
    CFG_INT(CONFIG_INT_DEVICE_ID, 0x0A, CFGF_NONE), // strtoul(DEFAULT_DEVICE_ID, NULL, 16)
    CFG_BOOL("override_freeze_protect", FALSE, CFGF_NONE),

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
	CFG_BOOL(CONFIG_BOOL_DAEMONIZE, true, CFGF_NONE),
	CFG_STR("log_file", "\0", CFGF_NONE),
    CFG_BOOL("pda_mode", false, CFGF_NONE),
    CFG_BOOL("pda_sleep_mode", false, CFGF_NONE),
    CFG_BOOL(CONFIG_BOOL_CONVERT_MQTT_TEMP, true, CFGF_NONE),
    CFG_BOOL(CONFIG_BOOL_CONVERT_DZ_TEMP, true, CFGF_NONE),
    CFG_BOOL(CONFIG_BOOL_REPORT_ZERO_POOL_TEMP, false, CFGF_NONE),
    CFG_BOOL(CONFIG_BOOL_REPORT_ZERO_SPA_TEMP, false, CFGF_NONE),
    CFG_BOOL(CONFIG_BOOL_READ_ALL_DEVICES, true, CFGF_NONE),
    CFG_BOOL("use_panel_aux_labels", false, CFGF_NONE),
    CFG_BOOL("debug_RSProtocol_packets", false, CFGF_NONE),
    CFG_BOOL(CONFIG_BOOL_FORCE_SWG, false, CFGF_NONE),
    CFG_INT(CONFIG_INT_SWG_ZERO_IGNORE, DEFAILT_SWG_ZERO_IGNORE_COUNT, CFGF_NONE),
    CFG_BOOL(CONFIG_BOOL_READ_PENTAIR_PACKETS, false, CFGF_NONE),
    CFG_BOOL("display_warnings_web", false, CFGF_NONE),
    CFG_BOOL(CONFIG_BOOL_DEBUG_RSPROTOCOL_PACKETS, false, CFGF_NONE),
    CFG_BOOL(CONFIG_BOOL_LOG_RAW_RS_BYTES, false, CFGF_NONE),

    CFG_END()
};

void initParameters()
{
    _config_parameters = cfg_init(_config_parameters_defaults, CFGF_NONE);
}

void handleConfigurationFileOptions()
{
    const char* configuration_file = cfg_getstr(_config_parameters, CONFIG_STR_CONFIG_FILE);

    if (0 == configuration_file)
    {
        logMessage(LOG_DEBUG, "No configuration file specified...using defaults\n");
    }
    else if (!cfg_parse(_config_parameters, configuration_file))
    {
        logMessage(LOG_DEBUG, "Failed to load specified configuration file...using defaults\n");
    }
    else
    {
        logMessage(LOG_DEBUG, "Loaded configuration from %s\n", configuration_file);
    }
}

#define MAXCFGLINE 256

char *generate_mqtt_id(char *buf, int len);

#ifdef _OLD_CFG_REMOVE_ME

/*
* initialize data to default values
*/
void initParameters (struct aqconfig * parms)
{
  //char *p;
  parms->serial_port = DEFAULT_SERIALPORT;
  parms->log_level = DEFAULT_LOG_LEVEL;
  parms->socket_port = DEFAULT_WEBPORT;
  parms->web_directory = DEFAULT_WEBROOT;
  //parms->device_id = strtoul(DEFAULT_DEVICE_ID, &p, 16);
  parms->device_id = strtoul(DEFAULT_DEVICE_ID, NULL, 16);
  //sscanf(DEFAULT_DEVICE_ID, "0x%x", &parms->device_id);
  parms->override_freeze_protect = FALSE;

  parms->mqtt_dz_sub_topic = DEFAULT_MQTT_DZ_OUT;
  parms->mqtt_dz_pub_topic = DEFAULT_MQTT_DZ_IN;
  parms->mqtt_aq_topic = DEFAULT_MQTT_AQ_TP;
  parms->mqtt_server = DEFAULT_MQTT_SERVER;
  parms->mqtt_user = DEFAULT_MQTT_USER;
  parms->mqtt_passwd = DEFAULT_MQTT_PASSWD;

  parms->dzidx_air_temp = TEMP_UNKNOWN;
  parms->dzidx_pool_water_temp = TEMP_UNKNOWN;
  parms->dzidx_spa_water_temp = TEMP_UNKNOWN;
  //parms->dzidx_pool_thermostat = TEMP_UNKNOWN; // removed until domoticz has a better virtual thermostat
  //parms->dzidx_spa_thermostat = TEMP_UNKNOWN; // removed until domoticz has a better virtual thermostat
  parms->light_programming_mode = 0;
  parms->light_programming_initial_on = 15;
  parms->light_programming_initial_off = 12;
  parms->light_programming_button_pool = TEMP_UNKNOWN;
  parms->light_programming_button_spa = TEMP_UNKNOWN;
  parms->deamonize = true;
  parms->log_file = '\0';
  parms->pda_mode = false;
  parms->pda_sleep_mode = false;
  parms->convert_mqtt_temp = true;
  parms->convert_dz_temp = true;
  parms->report_zero_pool_temp = false;
  parms->report_zero_spa_temp = false;
  parms->read_all_devices = true;
  parms->use_panel_aux_labels = false;
  parms->debug_RSProtocol_packets = false;
  parms->force_swg = false;
  //parms->swg_pool_and_spa = false;
  parms->read_pentair_packets = false;
  parms->swg_zero_ignore = DEFAILT_SWG_ZERO_IGNORE_COUNT;
  parms->display_warnings_web = false;
  parms->log_raw_RS_bytes = false;
 
  generate_mqtt_id(parms->mqtt_ID, MQTT_ID_LEN);
}

#endif // _OLD_CFG_REMOVE_ME

char *cleanalloc(char*str)
{
  char *result;
  str = cleanwhitespace(str);
  
  result = (char*)malloc(strlen(str)+1);
  strcpy ( result, str );
  //printf("Result=%s\n",result);
  return result;
}

// Find the first network interface with valid MAC and put mac address into buffer upto length
bool mac(char *buf, int len)
{
  struct ifreq s;
  int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
  struct if_nameindex *if_nidxs, *intf;

  if_nidxs = if_nameindex();
  if (if_nidxs != NULL)
  {
    for (intf = if_nidxs; intf->if_index != 0 || intf->if_name != NULL; intf++)
    {
      strcpy(s.ifr_name, intf->if_name);
      if (0 == ioctl(fd, SIOCGIFHWADDR, &s))
      {
        int i;
        if ( s.ifr_addr.sa_data[0] == 0 &&
             s.ifr_addr.sa_data[1] == 0 &&
             s.ifr_addr.sa_data[2] == 0 &&
             s.ifr_addr.sa_data[3] == 0 &&
             s.ifr_addr.sa_data[4] == 0 &&
             s.ifr_addr.sa_data[5] == 0 ) {
          continue;
        }
        for (i = 0; i < 6 && i * 2 < len; ++i)
        {
          sprintf(&buf[i * 2], "%02x", (unsigned char)s.ifr_addr.sa_data[i]);
        }
        return true;
      }
    }
  }

  return false;
}

char *generate_mqtt_id(char *buf, int len) {
  extern char *__progname; // glibc populates this
  int i;
  strncpy(buf, basename(__progname), len);
  i = strlen(buf);

  if (i < len) {
    buf[i++] = '_';
    // If we can't get MAC to pad mqtt id then use PID
    if (!mac(&buf[i], len - i)) {
      sprintf(&buf[i], "%.*d", (len-i), getpid());
    }
  }

  buf[len] = '\0';

  return buf;
}

#ifdef _OLD_CFG_REMOVE_ME

bool setConfigValue(struct aqconfig *config_parameters, struct aqualinkdata *aqdata, char *param, char *value) {
  bool rtn = false;
  static int pi=0;

  if (strncasecmp(param, "socket_port", 11) == 0) {
    config_parameters->socket_port = cleanalloc(value);
    rtn=true;
  } else if (strncasecmp(param, "serial_port", 11) == 0) {
    config_parameters->serial_port = cleanalloc(value);
    rtn=true;
  } else if (strncasecmp(param, "log_level", 9) == 0) {
    config_parameters->log_level = text2elevel(cleanalloc(value));
    rtn=true;
  } else if (strncasecmp(param, "device_id", 9) == 0) {
    config_parameters->device_id = strtoul(cleanalloc(value), NULL, 16);
    rtn=true;
  } else if (strncasecmp(param, "web_directory", 13) == 0) {
    config_parameters->web_directory = cleanalloc(value);
    rtn=true;
  } else if (strncasecmp(param, "log_file", 8) == 0) {
    config_parameters->log_file = cleanalloc(value);
    rtn=true;
  } else if (strncasecmp(param, "mqtt_address", 12) == 0) {
    config_parameters->mqtt_server = cleanalloc(value);
    rtn=true;
  } else if (strncasecmp(param, "mqtt_dz_sub_topic", 17) == 0) {
    config_parameters->mqtt_dz_sub_topic = cleanalloc(value);
    rtn=true;
  } else if (strncasecmp(param, "mqtt_dz_pub_topic", 17) == 0) {
    config_parameters->mqtt_dz_pub_topic = cleanalloc(value);
    rtn=true;
  } else if (strncasecmp(param, "mqtt_aq_topic", 13) == 0) {
    config_parameters->mqtt_aq_topic = cleanalloc(value);
    rtn=true;
  } else if (strncasecmp(param, "mqtt_user", 9) == 0) {
    config_parameters->mqtt_user = cleanalloc(value);
    rtn=true;
  } else if (strncasecmp(param, "mqtt_passwd", 11) == 0) {
    config_parameters->mqtt_passwd = cleanalloc(value);
    rtn=true;
  } else if (strncasecmp(param, "air_temp_dzidx", 14) == 0) {
    config_parameters->dzidx_air_temp = strtoul(value, NULL, 10);
    rtn=true;
  } else if (strncasecmp(param, "pool_water_temp_dzidx", 21) == 0) {
    config_parameters->dzidx_pool_water_temp = strtoul(value, NULL, 10);
    rtn=true;
  } else if (strncasecmp(param, "spa_water_temp_dzidx", 20) == 0) {
    config_parameters->dzidx_spa_water_temp = strtoul(value, NULL, 10);
    rtn=true;
  } else if (strncasecmp(param, "light_programming_mode", 21) == 0) {
    config_parameters->light_programming_mode = atof(cleanalloc(value)); // should free this
    rtn=true;
  } else if (strncasecmp(param, "light_programming_initial_on", 28) == 0) {
    config_parameters->light_programming_initial_on = strtoul(value, NULL, 10);
    rtn=true;
  } else if (strncasecmp(param, "light_programming_initial_off", 29) == 0) {
    config_parameters->light_programming_initial_off = strtoul(value, NULL, 10);
    rtn=true;
  } else if (strncasecmp(param, "light_programming_button_spa", 28) == 0) {
    config_parameters->light_programming_button_spa = strtoul(value, NULL, 10) - 1;
    rtn=true;
  } else if (strncasecmp(param, "light_programming_button", 24) == 0 ||
             strncasecmp(param, "light_programming_button_pool", 29) == 0) {
    config_parameters->light_programming_button_pool = strtoul(value, NULL, 10) - 1;
    rtn=true;
  } else if (strncasecmp(param, "SWG_percent_dzidx", 17) == 0) {
    config_parameters->dzidx_swg_percent = strtoul(value, NULL, 10);
    rtn=true;
  } else if (strncasecmp(param, "SWG_PPM_dzidx", 13) == 0) {
    config_parameters->dzidx_swg_ppm = strtoul(value, NULL, 10);
    rtn=true;
  } else if (strncasecmp(param, "SWG_Status_dzidx", 14) == 0) {
    config_parameters->dzidx_swg_status = strtoul(value, NULL, 10);
    rtn=true;
  } else if (strncasecmp(param, "override_freeze_protect", 23) == 0) {
    config_parameters->override_freeze_protect = text2bool(value);
    rtn=true;
  } else if (strncasecmp(param, "pda_mode", 8) == 0) {
    config_parameters->pda_mode = text2bool(value);
    set_pda_mode(config_parameters->pda_mode);
    //config_parameters->use_PDA_auxiliary = false;
    rtn=true;
  } else if (strncasecmp(param, "pda_sleep_mode", 8) == 0) {
    config_parameters->pda_sleep_mode = text2bool(value);
    //set_pda_mode(config_parameters->pda_mode);
    rtn=true;
  } else if (strncasecmp(param, "convert_mqtt_temp_to_c", 22) == 0) {
    config_parameters->convert_mqtt_temp = text2bool(value);
    rtn=true;
  } else if (strncasecmp(param, "convert_dz_temp_to_c", 20) == 0) {
    config_parameters->convert_dz_temp = text2bool(value);
    rtn=true;
    /*
  } else if (strncasecmp(param, "flash_mqtt_buttons", 18) == 0) {
    config_parameters->flash_mqtt_buttons = text2bool(value);
    rtn=true;*/
  } else if (strncasecmp(param, "report_zero_spa_temp", 20) == 0) {
    config_parameters->report_zero_spa_temp = text2bool(value);
    rtn=true;
  } else if (strncasecmp (param, "report_zero_pool_temp", 21) == 0) {
    config_parameters->report_zero_pool_temp = text2bool(value);
    rtn=true;
  } else if (strncasecmp (param, "read_all_devices", 16) == 0) {
    config_parameters->read_all_devices = text2bool(value);
    rtn=true;
  } else if (strncasecmp (param, "use_panel_aux_labels", 20) == 0) {
    config_parameters->use_panel_aux_labels = text2bool(value);
    rtn=true;
    } else if (strncasecmp (param, "force_SWG", 9) == 0) {
    config_parameters->force_swg = text2bool(value);
    rtn=true;
  } else if (strncasecmp (param, "debug_RSProtocol_packets", 24) == 0) {
    config_parameters->debug_RSProtocol_packets = text2bool(value);
    rtn=true;
  } else if (strncasecmp (param, "read_pentair_packets", 17) == 0) {
    config_parameters->read_pentair_packets = text2bool(value);
    config_parameters->read_all_devices = true;
    rtn=true;
  } else if (strncasecmp (param, "swg_zero_ignore_count", 21) == 0) {
    config_parameters->swg_zero_ignore = strtoul(value, NULL, 10);
    rtn=true;
  } else if (strncasecmp (param, "display_warnings_in_web", 23) == 0) {
    config_parameters->display_warnings_web = text2bool(value);
    rtn=true;
  }
  /* 
  else if (strncasecmp (param, "use_PDA_auxiliary", 17) == 0) {
    config_parameters->use_PDA_auxiliary = text2bool(value);
    if ( pda_mode() ) {
      logMessage(LOG_ERR, "ERROR Can't use `use_PDA_auxiliary` in PDA mode, ignoring'\n");
      config_parameters->use_PDA_auxiliary = false;
    }
    rtn=true;
  } */
  // removed until domoticz has a better virtual thermostat
  /*else if (strncasecmp (param, "pool_thermostat_dzidx", 21) == 0) {      
              config_parameters->dzidx_pool_thermostat = strtoul(value, NULL, 10);
              rtn=true;
            } else if (strncasecmp (param, "spa_thermostat_dzidx", 20) == 0) {
              config_parameters->dzidx_spa_thermostat = strtoul(value, NULL, 10);
              rtn=true;
            } */
  else if (strncasecmp(param, "button_", 7) == 0) {
    int num = strtoul(param + 7, NULL, 10) - 1;
    if (strncasecmp(param + 9, "_label", 6) == 0) {
      aqdata->aqbuttons[num].label = cleanalloc(value);
      rtn=true;
    } else if (strncasecmp(param + 9, "_dzidx", 6) == 0) {
      aqdata->aqbuttons[num].dz_idx = strtoul(value, NULL, 10);
      rtn=true;
    } else if (strncasecmp(param + 9, "_PDA_label", 10) == 0) {
      aqdata->aqbuttons[num].pda_label = cleanalloc(value);
      rtn=true;
    } else if (strncasecmp(param + 9, "_pumpID", 7) == 0) {
      //aqdata->aqbuttons[num].pda_label = cleanalloc(value);
      //96 to 111 = Pentair, 120 to 123 = Jandy
      if (pi < MAX_PUMPS) {
        aqdata->pumps[pi].button = &aqdata->aqbuttons[num];
        aqdata->pumps[pi].pumpID = strtoul(cleanalloc(value), NULL, 16);
        //aqdata->pumps[pi].buttonID = num;
        if (aqdata->pumps[pi].pumpID < 119)
          aqdata->pumps[pi].ptype = PENTAIR;
        else
          aqdata->pumps[pi].ptype = JANDY;
        pi++;
      } else {
        logMessage(LOG_ERR, "Config error, VSP Pumps limited to %d, ignoring %s'\n",MAX_PUMPS,param);
      }
      rtn=true;
    }
  }

  return rtn;
}


void readCfg (struct aqconfig *config_parameters, struct aqualinkdata *aqdata, char *cfgFile)
{
  FILE * fp ;
  char bufr[MAXCFGLINE];
  char *b_ptr;

  config_parameters->config_file = cleanalloc(cfgFile);

  if( (fp = fopen(cfgFile, "r")) != NULL){
    while(! feof(fp)){
      if (fgets(bufr, MAXCFGLINE, fp) != NULL)
      {
        b_ptr = &bufr[0];
        char *indx;
        // Eat leading whitespace
        while(isspace(*b_ptr)) b_ptr++;
        if ( b_ptr[0] != '\0' && b_ptr[0] != '#')
        {
          indx = strchr(b_ptr, '=');  
          if ( indx != NULL) 
          {
            if ( ! setConfigValue(config_parameters, aqdata, b_ptr, indx+1))
              logMessage(LOG_ERR, "Unknown config parameter '%.*s'\n",strlen(b_ptr)-1, b_ptr);
          } 
        }
      }
    }
    fclose(fp);
  } else {
    /* error processing, couldn't open file */
    logMessage(LOG_ERR, "Error reading config file '%s'\n",cfgFile);
    displayLastSystemError(cfgFile);
    exit (EXIT_FAILURE);
  }
}


#endif // _OLD_CFG_REMOVE_ME



//DEBUG_DERIAL, DEBUG, INFO, NOTICE, WARNING, ERROR
char *errorlevel2text(int level) 
{
  switch(level) {
    case LOG_DEBUG_SERIAL:
      return "DEBUG_SERIAL";
      break;
    case LOG_DEBUG:
      return "DEBUG";
      break;
    case LOG_INFO:
      return "INFO";
      break;
    case LOG_NOTICE:
      return "NOTICE";
      break;
    case LOG_WARNING:
      return "WARNING";
      break;
    case LOG_ERR:
     default:
      return "ERROR";
      break;
  }
  
  return "";
}

bool remount_root_ro(bool readonly) 
{
    UNREFERENCED_PARAMETER(readonly);
	return true;
}

void writeCharValue (FILE *fp, char *msg, char *value)
{
    if (value == NULL) 
    {
        fprintf(fp, "#%s = \n", msg);
    }
    else
    {
        fprintf(fp, "%s = %s\n", msg, value);
    }
}
void writeIntValue (FILE *fp, char *msg, int value)
{
    if (value <= 0) 
    {
        fprintf(fp, "#%s = \n", msg);
    }
    else
    {
        fprintf(fp, "%s = %d\n", msg, value);
    }
}

#ifdef _OLD_CFG_REMOVE_ME

bool writeCfg (struct aqconfig *config_parameters, struct aqualinkdata *aqdata)
{
  FILE *fp;
  int i;
  bool fs = remount_root_ro(false);

  fp = fopen(config_parameters->config_file, "w");
  if (fp == NULL) {
    logMessage(LOG_ERR, "Open config file failed '%s'\n", config_parameters->config_file);
    remount_root_ro(true);
    //fprintf(stdout, "Open file failed 'sprinkler.cron'\n");
    return false;
  }
  fprintf(fp, "#***** AqualinkD configuration *****\n");

  fprintf(fp, "socket_port = %s\n", config_parameters->socket_port);
  fprintf(fp, "serial_port = %s\n", config_parameters->serial_port);
  fprintf(fp, "device_id = 0x%02hhx\n", config_parameters->device_id);
  fprintf(fp, "read_all_devices = %s", bool2text(config_parameters->read_all_devices));
  writeCharValue(fp, "log_level", errorlevel2text(config_parameters->log_level));
  writeCharValue(fp, "web_directory", config_parameters->web_directory);
  writeCharValue(fp, "log_file", config_parameters->log_file);
  fprintf(fp, "pda_mode = %s\n", bool2text(config_parameters->pda_mode)); 

  fprintf(fp, "\n#** MQTT Configuration **\n");
  writeCharValue(fp, "mqtt_address", config_parameters->mqtt_server);
  writeCharValue(fp, "mqtt_dz_sub_topic", config_parameters->mqtt_dz_sub_topic);
  writeCharValue(fp, "mqtt_dz_pub_topic", config_parameters->mqtt_dz_pub_topic);
  writeCharValue(fp, "mqtt_aq_topic", config_parameters->mqtt_aq_topic);
  writeCharValue(fp, "mqtt_user", config_parameters->mqtt_user);
  writeCharValue(fp, "mqtt_passwd", config_parameters->mqtt_passwd);

  fprintf(fp, "\n#** General **\n");
  fprintf(fp, "convert_mqtt_temp_to_c = %s\n", bool2text(config_parameters->convert_mqtt_temp));
  fprintf(fp, "override_freeze_protect = %s\n", bool2text(config_parameters->override_freeze_protect));        
  //fprintf(fp, "flash_mqtt_buttons = %s\n", bool2text(config_parameters->flash_mqtt_buttons)); 
  fprintf(fp, "report_zero_spa_temp = %s\n", bool2text(config_parameters->report_zero_spa_temp));
  fprintf(fp, "report_zero_pool_temp = %s\n", bool2text(config_parameters->report_zero_pool_temp));

  fprintf(fp, "\n#** Programmable light **\n");
  //if (config_parameters->light_programming_button_pool <= 0) {
  //  fprintf(fp, "#light_programming_button_pool = %d\n", config_parameters->light_programming_button_pool); 
  //  fprintf(fp, "#light_programming_mode = %f\n", config_parameters->light_programming_mode);  
   // fprintf(fp, "#light_programming_initial_on = %d\n", config_parameters->light_programming_initial_on);         
  //  fprintf(fp, "#light_programming_initial_off = %d\n", config_parameters->light_programming_initial_off);
  //} else {
    fprintf(fp, "light_programming_button_pool = %d\n", config_parameters->light_programming_button_pool); 
    fprintf(fp, "light_programming_button_spa = %d\n", config_parameters->light_programming_button_spa); 
    fprintf(fp, "light_programming_mode = %f\n", config_parameters->light_programming_mode);  
    fprintf(fp, "light_programming_initial_on = %d\n", config_parameters->light_programming_initial_on);         
    fprintf(fp, "light_programming_initial_off = %d\n", config_parameters->light_programming_initial_off);
  //}

  fprintf(fp, "\n#** Domoticz **\n");
  fprintf(fp, "convert_dz_temp_to_c = %s\n", bool2text(config_parameters->convert_dz_temp));
  writeIntValue(fp, "air_temp_dzidx", config_parameters->dzidx_air_temp); 
  writeIntValue(fp, "pool_water_temp_dzidx", config_parameters->dzidx_pool_water_temp);     
  writeIntValue(fp, "spa_water_temp_dzidx", config_parameters->dzidx_spa_water_temp); 
  writeIntValue(fp, "SWG_percent_dzidx", config_parameters->dzidx_swg_percent); 
  writeIntValue(fp, "SWG_PPM_dzidx", config_parameters->dzidx_swg_ppm); 
  writeIntValue(fp, "SWG_Status_dzidx", config_parameters->dzidx_swg_status);    

  fprintf(fp, "\n#** Buttons **\n");
  for (i=0; i < TOTAL_BUTTONS; i++) 
  {
    fprintf(fp, "button_%.2d_label = %s\n", i+1, aqdata->aqbuttons[i].label);
    if (aqdata->aqbuttons[i].dz_idx > 0)
      fprintf(fp, "button_%.2d_dzidx = %d\n", i+1, aqdata->aqbuttons[i].dz_idx);
    if (aqdata->aqbuttons[i].pda_label != NULL)
      fprintf(fp, "button_%.2d_PDA_label = %s\n", i+1, aqdata->aqbuttons[i].pda_label);
  }
  fclose(fp);
  remount_root_ro(fs);

  return true;
}

#endif // _OLD_CFG_REMOVE_ME
