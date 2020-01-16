#include "config_private_defaults.h"

#include <confuse.h>

#include "logging/logging_levels.h"

const char DEFAULT_LOG_FILE[]			= "/var/log/aqualink.log";
const char DEFAULT_WEBROOT[]			= "./";
const char DEFAULT_CONFIG_FILE[]		= "/etc/aqualinkd.conf";
const char DEFAULT_PID_FILE[]			= "/var/run/aqualinkd.pid";
const char DEFAULT_SERIALPORT[]			= "/dev/ttyS8";
const char DEFAULT_MQTT_DZ_IN[]			= "";
const char DEFAULT_MQTT_DZ_OUT[]		= "";
const char DEFAULT_MQTT_AQ_TP[]			= "";
const char DEFAULT_MQTT_SERVER[]		= "";
const char DEFAULT_MQTT_USER[]			= "";
const char DEFAULT_MQTT_PASSWD[]		= "";
const char DEFAULT_RAWSERIAL_LOGFILE[] = "/tmp/RS485.log";
