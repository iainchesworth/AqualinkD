
#ifndef CONFIG_H_
#define CONFIG_H_

#include <confuse.h>

// #include "utils.h"
// #include "aq_serial.h"
// #include "aqualink.h"

#define DEFAULT_LOG_LEVEL    5 
#define DEFAULT_WEBPORT      "6580"
#define DEFAULT_WEBROOT      "./"
#define DEFAULT_CONFIG_FILE  "./aqualinkd.conf"
#define DEFAULT_SERIALPORT   "/dev/ttyUSB0"
#define DEFAULT_DEVICE_ID    "0x0a"
#define DEFAULT_MQTT_DZ_IN   NULL
#define DEFAULT_MQTT_DZ_OUT  NULL
#define DEFAULT_MQTT_AQ_TP   NULL
#define DEFAULT_MQTT_SERVER  NULL
#define DEFAULT_MQTT_USER    NULL
#define DEFAULT_MQTT_PASSWD  NULL
// Set this high, as people are confused about SWG bouncing to zero on some panels, just stop the questions
#define DEFAILT_SWG_ZERO_IGNORE_COUNT 20

#define MQTT_ID_LEN 18 // 20 seems to kill mosquitto 1.6

static const char CONFIG_STR_CONFIG_FILE[] = "config_file";
static const char CONFIG_STR_SERIAL_PORT[] = "serial_port";
static const char CONFIG_INT_LOG_LEVEL[] = "log_level";
static const char CONFIG_STR_SOCKET_PORT[] = "socket_port";
static const char CONFIG_STR_WEB_DIRECTORY[] = "web_directory";
static const char CONFIG_INT_DEVICE_ID[] = "device_id";

static const char CONFIG_STR_MQTT_DZ_SUB_TOPIC[] = "mqtt_dz_sub_topic";
static const char CONFIG_STR_MQTT_DZ_PUB_TOPIC[] = "mqtt_dz_pub_topic";
static const char CONFIG_STR_MQTT_AQ_TOPIC[] = "mqtt_aq_topic";
static const char CONFIG_STR_MQTT_SERVER[] = "mqtt_server";
static const char CONFIG_STR_MQTT_USER[] = "mqtt_user";
static const char CONFIG_STR_MQTT_PASSWD[] = "mqtt_passwd";
static const char CONFIG_STR_MQTT_ID[] = "mqtt_id";

static const char CONFIG_INT_DZIDX_AIR_TEMP[] = "dzidx_air_temp";
static const char CONFIG_INT_DZIDX_POOL_WATER_TEMP[] = "dzidx_pool_water_temp";
static const char CONFIG_INT_DZIDX_SPA_WATER_TEMP[] = "dzidx_spa_water_temp";
static const char CONFIG_INT_DZIDX_SWG_PERCENT[] = "dzidx_swg_percent";
static const char CONFIG_INT_DZIDX_SWG_PPM[] = "dzidx_swg_ppm";
static const char CONFIG_INT_DZIDX_SWG_STATUS[] = "dzidx_swg_status";

static const char CONFIG_FLOAT_LIGHT_PROGRAMMING_MODE[] = "light_programming_mode";
static const char CONFIG_INT_LIGHT_PROGRAMMING_INITIAL_ON[] = "light_programming_initial_on";
static const char CONFIG_INT_LIGHT_PROGRAMMING_INITIAL_OFF[] = "light_programming_initial_off";
static const char CONFIG_INT_LIGHT_PROGRAMMING_BUTTON_POOL[] = "light_programming_button_pool";
static const char CONFIG_INT_LIGHT_PROGRAMMING_BUTTON_SPA[] = "light_programming_button_spa";

static const char CONFIG_BOOL_DAEMONIZE[] = "deamonize";

static const char CONFIG_BOOL_PDA_MODE[] = "pda_mode";

static const char CONFIG_BOOL_CONVERT_MQTT_TEMP[] = "convert_mqtt_temp";
static const char CONFIG_BOOL_CONVERT_DZ_TEMP[] = "convert_dz_temp";
static const char CONFIG_BOOL_REPORT_ZERO_POOL_TEMP[] = "report_zero_pool_temp";
static const char CONFIG_BOOL_REPORT_ZERO_SPA_TEMP[] = "report_zero_spa_temp";
static const char CONFIG_BOOL_READ_ALL_DEVICES[] = "read_all_devices";
static const char CONFIG_BOOL_FORCE_SWG[] = "force_swg";
static const char CONFIG_INT_SWG_ZERO_IGNORE[] = "swg_zero_ignore";
static const char CONFIG_BOOL_READ_PENTAIR_PACKETS[] = "read_pentair_packets";
static const char CONFIG_BOOL_DEBUG_RSPROTOCOL_PACKETS[] = "debug_RSProtocol_packets";
static const char CONFIG_BOOL_LOG_RAW_RS_BYTES[] = "log_raw_RS_bytes";

// This is the "global" configuration parameter store (initialised in config.c).
static cfg_t* _config_parameters = 0;

#ifdef _OLD_CFG_REMOVE_ME

struct aqconfig
{
  char *config_file;
  char *serial_port;
  unsigned int log_level;
  char *socket_port;
  char *web_directory;
  unsigned char device_id;
  bool deamonize;
  char *log_file;
  char *mqtt_dz_sub_topic;
  char *mqtt_dz_pub_topic;
  char *mqtt_aq_topic;
  char *mqtt_server;
  char *mqtt_user;
  char *mqtt_passwd;
  char mqtt_ID[MQTT_ID_LEN];
  int dzidx_air_temp;
  int dzidx_pool_water_temp;
  int dzidx_spa_water_temp;
  int dzidx_swg_percent;
  int dzidx_swg_ppm;
  int dzidx_swg_status;
  float light_programming_mode;
  int light_programming_initial_on;
  int light_programming_initial_off;
  int light_programming_button_pool;
  int light_programming_button_spa;
  bool override_freeze_protect;
  bool pda_mode;
  bool pda_sleep_mode;
  bool convert_mqtt_temp;
  bool convert_dz_temp;
  //bool flash_mqtt_buttons;
  bool report_zero_spa_temp;
  bool report_zero_pool_temp;
  bool read_all_devices;
  bool use_panel_aux_labels;
  bool force_swg;
  int swg_zero_ignore;
  bool display_warnings_web;
  //bool swg_pool_and_spa;
  //bool use_PDA_auxiliary;
  bool read_pentair_packets;
  bool debug_RSProtocol_packets;
  bool log_raw_RS_bytes;
  //int dzidx_pool_thermostat; // Domoticz virtual thermostats are crap removed until better
  //int dzidx_spa_thermostat;  // Domoticz virtual thermostats are crap removed until better
  //char mqtt_pub_topic[250];
  //char *mqtt_pub_tp_ptr = mqtt_pub_topic[];
};




void initParameters (struct aqconfig * parms);
//bool parse_config (struct aqconfig * parms, char *cfgfile);
//void readCfg (struct aqconfig *config_parameters, char *cfgFile);
void readCfg (struct aqconfig *config_parameters, struct aqualinkdata *aqdata, char *cfgFile);
bool writeCfg (struct aqconfig *config_parameters, struct aqualinkdata *aqdata);
bool setConfigValue(struct aqconfig *config_parameters, struct aqualinkdata *aqdata, char *param, char *value);

#endif // _OLD_CFG_REMOVE_ME

char *cleanalloc(char*str);

#endif // CONFIG_H_
