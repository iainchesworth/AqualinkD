#include "config_helpers.h"
#include "config_private.h"

#include <confuse.h>
#include <stdbool.h>
#include "config.h"

// This is the "global" configuration parameter store (initialised in config.c).
extern cfg_t* _config_parameters;

const char* CFG_ConfigFile()
{
	return cfg_getstr(_config_parameters, CONFIG_STR_CONFIG_FILE);
}

const char* CFG_PidFile()
{
	return cfg_getstr(_config_parameters, CONFIG_STR_PID_FILE);
}

const char* CFG_SerialPort()
{
	return cfg_getstr(_config_parameters, CONFIG_STR_SERIAL_PORT);
}

const int CFG_LogLevel()
{
	return cfg_getint(_config_parameters, CONFIG_INT_LOG_LEVEL);
}

const int CFG_SocketPort()
{
	return cfg_getint(_config_parameters, CONFIG_INT_SOCKET_PORT);
}

const int CFG_SslSocketPort()
{
	return cfg_getint(_config_parameters, CONFIG_INT_SSL_SOCKET_PORT);
}

const char* CFG_WebDirectory()
{
	return cfg_getstr(_config_parameters, CONFIG_STR_WEB_DIRECTORY);
}

const bool CFG_Insecure()
{
	return cfg_getbool(_config_parameters, CONFIG_BOOL_INSECURE);
}

const int CFG_DeviceId()
{
	return cfg_getint(_config_parameters, CONFIG_INT_DEVICE_ID);
}

const bool CFG_OverrideFreezeProtect()
{
	return cfg_getbool(_config_parameters, CONFIG_BOOL_OVERRIDE_FREEZE_PROTECT);
}

const char* CFG_MqttDzSubTopic()
{
	return cfg_getstr(_config_parameters, CONFIG_STR_MQTT_DZ_SUB_TOPIC);
}

const char* CFG_MqttDzPubTopic()
{
	return cfg_getstr(_config_parameters, CONFIG_STR_MQTT_DZ_PUB_TOPIC);
}

const char* CFG_MqttAqTopic()
{
	return cfg_getstr(_config_parameters, CONFIG_STR_MQTT_AQ_TOPIC);
}

const char* CFG_MqttServer()
{
	return cfg_getstr(_config_parameters, CONFIG_STR_MQTT_SERVER);
}

const char* CFG_MqttUser()
{
	return cfg_getstr(_config_parameters, CONFIG_STR_MQTT_USER);
}

const char* CFG_MqttPassword()
{
	return cfg_getstr(_config_parameters, CONFIG_STR_MQTT_PASSWD);
}

const char* CFG_MqttId()
{
	return cfg_getstr(_config_parameters, CONFIG_STR_MQTT_ID);
}

const int CFG_DzIdxAirTemp()
{
	return cfg_getint(_config_parameters, CONFIG_INT_DZIDX_AIR_TEMP);
}

const int CFG_DzIdxPoolWaterTemp()
{
	return cfg_getint(_config_parameters, CONFIG_INT_DZIDX_POOL_WATER_TEMP);
}

const int CFG_DzIdxSpaWaterTemp()
{
	return cfg_getint(_config_parameters, CONFIG_INT_DZIDX_SPA_WATER_TEMP);
}

const int CFG_DzIdxSwgPercent()
{
	return cfg_getint(_config_parameters, CONFIG_INT_DZIDX_SWG_PERCENT);
}

const int CFG_DzIdxSwgPpm()
{
	return cfg_getint(_config_parameters, CONFIG_INT_DZIDX_SWG_PPM);
}

const int CFG_DzIdxSwgStatus()
{
	return cfg_getint(_config_parameters, CONFIG_INT_DZIDX_SWG_STATUS);
}

const float CFG_LightProgrammingMode()
{
	return cfg_getfloat(_config_parameters, CONFIG_FLOAT_LIGHT_PROGRAMMING_MODE);
}

const int CFG_LightProgrammingInitialOn()
{
	return cfg_getint(_config_parameters, CONFIG_INT_LIGHT_PROGRAMMING_INITIAL_ON);
}

const int CFG_LightProgrammingInitialOff()
{
	return cfg_getint(_config_parameters, CONFIG_INT_LIGHT_PROGRAMMING_INITIAL_OFF);
}

const int CFG_LightProgrammingButtonPool()
{
	return cfg_getint(_config_parameters, CONFIG_INT_LIGHT_PROGRAMMING_BUTTON_POOL);
}

const int CFG_LightProgrammingButtonSpa()
{
	return cfg_getint(_config_parameters, CONFIG_INT_LIGHT_PROGRAMMING_BUTTON_SPA);
}

const bool CFG_NoDaemonize()
{
	return cfg_getbool(_config_parameters, CONFIG_BOOL_NO_DAEMONIZE);
}

const char* CFG_LogFile()
{
	return cfg_getstr(_config_parameters, CONFIG_STR_LOG_FILE);
}

const bool CFG_PdaMode()
{
	return cfg_getbool(_config_parameters, CONFIG_BOOL_PDA_MODE);
}

const bool CFG_PdaSleepMode()
{
	return cfg_getbool(_config_parameters, CONFIG_BOOL_PDA_SLEEP_MODE);
}

const bool CFG_ConvertMqttTemp()
{
	return cfg_getbool(_config_parameters, CONFIG_BOOL_CONVERT_MQTT_TEMP_TO_C);
}

const bool CFG_ConvertDzTemp()
{
	return cfg_getbool(_config_parameters, CONFIG_BOOL_CONVERT_DZ_TEMP_TO_C);
}

const bool CFG_ReportZeroPoolTemp()
{
	return cfg_getbool(_config_parameters, CONFIG_BOOL_REPORT_ZERO_POOL_TEMP);
}

const bool CFG_ReportZeroSpaTemp()
{
	return cfg_getbool(_config_parameters, CONFIG_BOOL_REPORT_ZERO_SPA_TEMP);
}

const bool CFG_ReadAllDevices()
{
	return cfg_getbool(_config_parameters, CONFIG_BOOL_READ_ALL_DEVICES);
}

const bool CFG_UsePanelAuxLabels()
{
	return cfg_getbool(_config_parameters, CONFIG_BOOL_USE_PANEL_AUX_LABELS);
}

const bool CFG_ForceSwg()
{
	return cfg_getbool(_config_parameters, CONFIG_BOOL_FORCE_SWG);
}

const int CFG_SwgZeroIgnore()
{
	return cfg_getint(_config_parameters, CONFIG_INT_SWG_ZERO_IGNORE);
}

const bool CFG_ReadPentairPackets()
{
	return cfg_getbool(_config_parameters, CONFIG_BOOL_READ_PENTAIR_PACKETS);
}

const bool CFG_DisplayWarningsWeb()
{
	return cfg_getbool(_config_parameters, CONFIG_BOOL_DISPLAY_WARNINGS_IN_WEB);
}

const bool CFG_DebugRsProtocolPackets()
{
	return cfg_getbool(_config_parameters, CONFIG_BOOL_DEBUG_RSPROTOCOL_PACKETS);
}

const bool CFG_LogRawRsBytes()
{
	return cfg_getbool(_config_parameters, CONFIG_BOOL_LOG_RAW_RS_BYTES);
}

const char* CFG_LogRawRsBytes_LogFile()
{
	return cfg_getstr(_config_parameters, CONFIG_STR_LOG_RAW_RS_BYTES_LOGFILE);
}

//-----------------------------------------------------------------------
//
// SETTERS
//
//-----------------------------------------------------------------------

void CFG_Set_ConfigFile(const char* configFile)
{
	cfg_setstr(_config_parameters, CONFIG_STR_CONFIG_FILE, configFile);
}

void CFG_Set_PidFile(const char* pidFile)
{
	cfg_setstr(_config_parameters, CONFIG_STR_PID_FILE, pidFile);
}

void CFG_Set_SerialPort(const char* serialPort)
{
	cfg_setstr(_config_parameters, CONFIG_STR_SERIAL_PORT, serialPort);
}

void CFG_Set_LogLevel(int logLevel)
{
	cfg_setint(_config_parameters, CONFIG_INT_LOG_LEVEL, logLevel);
}

void CFG_Set_Insecure(bool insecure)
{
	cfg_setbool(_config_parameters, CONFIG_BOOL_INSECURE, insecure);
}

void CFG_Set_DeviceId(int deviceId)
{
	cfg_setint(_config_parameters, CONFIG_INT_DEVICE_ID, deviceId);
}

void CFG_Set_NoDaemonize(bool daemonize)
{
	cfg_setbool(_config_parameters, CONFIG_BOOL_NO_DAEMONIZE, daemonize);
}

void CFG_Set_LogFile(const char* logFile)
{
	cfg_setstr(_config_parameters, CONFIG_STR_LOG_FILE, logFile);
}

void CFG_Set_DebugRsProtocolPackets(bool debugRsProtocolPackets)
{
	cfg_setbool(_config_parameters, CONFIG_BOOL_DEBUG_RSPROTOCOL_PACKETS, debugRsProtocolPackets);
}

void CFG_Set_LogRawRsBytes(bool logRawRsBytes)
{
	cfg_setbool(_config_parameters, CONFIG_BOOL_LOG_RAW_RS_BYTES, logRawRsBytes);
}

void CFG_Set_LogRawRsBytes_LogFile(const char* logRawRsBytes_LogFile)
{
	cfg_setstr(_config_parameters, CONFIG_STR_LOG_RAW_RS_BYTES_LOGFILE, logRawRsBytes_LogFile);
}
