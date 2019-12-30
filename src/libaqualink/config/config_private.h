#ifndef AQ_CONFIG_PRIVATE_H_
#define AQ_CONFIG_PRIVATE_H_

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

extern const char CONFIG_STR_CONFIG_FILE[];
extern const char CONFIG_STR_SERIAL_PORT[];
extern const char CONFIG_INT_LOG_LEVEL[];
extern const char CONFIG_STR_SOCKET_PORT[];
extern const char CONFIG_STR_WEB_DIRECTORY[];
extern const char CONFIG_INT_DEVICE_ID[];

extern const char CONFIG_BOOL_OVERRIDE_FREEZE_PROTECT[];

extern const char CONFIG_STR_MQTT_DZ_SUB_TOPIC[];
extern const char CONFIG_STR_MQTT_DZ_PUB_TOPIC[];
extern const char CONFIG_STR_MQTT_AQ_TOPIC[];
extern const char CONFIG_STR_MQTT_SERVER[];
extern const char CONFIG_STR_MQTT_USER[];
extern const char CONFIG_STR_MQTT_PASSWD[];
extern const char CONFIG_STR_MQTT_ID[];

extern const char CONFIG_INT_DZIDX_AIR_TEMP[];
extern const char CONFIG_INT_DZIDX_POOL_WATER_TEMP[];
extern const char CONFIG_INT_DZIDX_SPA_WATER_TEMP[];
extern const char CONFIG_INT_DZIDX_SWG_PERCENT[];
extern const char CONFIG_INT_DZIDX_SWG_PPM[];
extern const char CONFIG_INT_DZIDX_SWG_STATUS[];

extern const char CONFIG_FLOAT_LIGHT_PROGRAMMING_MODE[];
extern const char CONFIG_INT_LIGHT_PROGRAMMING_INITIAL_ON[];
extern const char CONFIG_INT_LIGHT_PROGRAMMING_INITIAL_OFF[];
extern const char CONFIG_INT_LIGHT_PROGRAMMING_BUTTON_POOL[];
extern const char CONFIG_INT_LIGHT_PROGRAMMING_BUTTON_SPA[];

extern const char CONFIG_BOOL_DAEMONIZE[];
extern const char CONFIG_STR_LOG_FILE[];

extern const char CONFIG_BOOL_PDA_MODE[];
extern const char CONFIG_BOOL_PDA_SLEEP_MODE[];

extern const char CONFIG_BOOL_CONVERT_MQTT_TEMP[];
extern const char CONFIG_BOOL_CONVERT_DZ_TEMP[];
extern const char CONFIG_BOOL_REPORT_ZERO_POOL_TEMP[];
extern const char CONFIG_BOOL_REPORT_ZERO_SPA_TEMP[];
extern const char CONFIG_BOOL_READ_ALL_DEVICES[];
extern const char CONFIG_BOOL_USE_PANEL_AUX_LABELS[];
extern const char CONFIG_BOOL_FORCE_SWG[];
extern const char CONFIG_INT_SWG_ZERO_IGNORE[];
extern const char CONFIG_BOOL_READ_PENTAIR_PACKETS[];
extern const char CONFIG_BOOL_DISPLAY_WARNINGS_WEB[];
extern const char CONFIG_BOOL_DEBUG_RSPROTOCOL_PACKETS[];
extern const char CONFIG_BOOL_LOG_RAW_RS_BYTES[];

#endif // AQ_CONFIG_PRIVATE_H_
