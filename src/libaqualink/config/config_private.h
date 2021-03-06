#ifndef AQ_CONFIG_PRIVATE_H_
#define AQ_CONFIG_PRIVATE_H_

#include <confuse.h>

// Set this high, as people are confused about SWG bouncing to zero on some panels, just stop the questions
#define DEFAULT_SWG_ZERO_IGNORE_COUNT 20

#define MQTT_ID_LEN 18 // 20 seems to kill mosquitto 1.6

extern const char CONFIG_STR_CONFIG_FILE[];
extern const char CONFIG_STR_PID_FILE[];
extern const char CONFIG_STR_SERIAL_PORT[];
extern const char CONFIG_INT_LOG_LEVEL[];
extern const char CONFIG_INT_SOCKET_PORT[];
extern const char CONFIG_INT_SSL_SOCKET_PORT[];
extern const char CONFIG_STR_WEB_DIRECTORY[];
extern const char CONFIG_BOOL_INSECURE[];
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

extern const char CONFIG_BOOL_NO_DAEMONIZE[];
extern const char CONFIG_STR_LOG_FILE[];

extern const char CONFIG_BOOL_PDA_MODE[];
extern const char CONFIG_BOOL_PDA_SLEEP_MODE[];

extern const char CONFIG_BOOL_CONVERT_MQTT_TEMP_TO_C[];
extern const char CONFIG_BOOL_CONVERT_DZ_TEMP_TO_C[];
extern const char CONFIG_BOOL_REPORT_ZERO_POOL_TEMP[];
extern const char CONFIG_BOOL_REPORT_ZERO_SPA_TEMP[];
extern const char CONFIG_BOOL_READ_ALL_DEVICES[];
extern const char CONFIG_BOOL_USE_PANEL_AUX_LABELS[];
extern const char CONFIG_BOOL_FORCE_SWG[];
extern const char CONFIG_INT_SWG_ZERO_IGNORE[];
extern const char CONFIG_BOOL_READ_PENTAIR_PACKETS[];
extern const char CONFIG_BOOL_DISPLAY_WARNINGS_IN_WEB[];

extern const char CONFIG_BOOL_PLAYBACK_MODE[];
extern const char CONFIG_BOOL_RECORD_MODE[];
extern const char CONFIG_STR_RAW_SERIAL_LOG_FILE[];
 
extern const char CONFIG_STR_BUTTON_FILTERPUMP_LABEL[];
extern const char CONFIG_STR_BUTTON_SPAMODE_LABEL[];
extern const char CONFIG_STR_BUTTON_AUX1_LABEL[];
extern const char CONFIG_STR_BUTTON_AUX2_LABEL[];
extern const char CONFIG_STR_BUTTON_AUX3_LABEL[];
extern const char CONFIG_STR_BUTTON_AUX4_LABEL[];
extern const char CONFIG_STR_BUTTON_AUX5_LABEL[];
extern const char CONFIG_STR_BUTTON_AUX6_LABEL[];
extern const char CONFIG_STR_BUTTON_AUX7_LABEL[];
extern const char CONFIG_STR_BUTTON_POOLHEATER_LABEL[];
extern const char CONFIG_STR_BUTTON_SPAHEATER_LABEL[];
extern const char CONFIG_STR_BUTTON_SOLARHEATER_LABEL[];

#endif // AQ_CONFIG_PRIVATE_H_
