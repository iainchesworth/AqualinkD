#include "config_helpers.h"
#include "config_private.h"

#include <confuse.h>
#include <stdbool.h>

#include "hardware/devices/hardware_device_types.h"
#include "logging/logging_levels.h"
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

LoggingLevels CFG_LogLevel()
{
	return (LoggingLevels)cfg_getint(_config_parameters, CONFIG_INT_LOG_LEVEL);
}

int CFG_SocketPort()
{
	return cfg_getint(_config_parameters, CONFIG_INT_SOCKET_PORT);
}

int CFG_SslSocketPort()
{
	return cfg_getint(_config_parameters, CONFIG_INT_SSL_SOCKET_PORT);
}

const char* CFG_WebDirectory()
{
	return cfg_getstr(_config_parameters, CONFIG_STR_WEB_DIRECTORY);
}

bool CFG_Insecure()
{
	return (cfg_true == cfg_getbool(_config_parameters, CONFIG_BOOL_INSECURE));
}

HardwareDeviceId CFG_DeviceId()
{
	unsigned char calculated_device_id = (unsigned char)cfg_getint(_config_parameters, CONFIG_INT_DEVICE_ID);
	HardwareDeviceId device_id;

	device_id.Type = (HardwareDeviceTypes)(calculated_device_id & 0xFC);
	device_id.Instance = (HardwareDeviceInstanceTypes)(calculated_device_id & 0x03);

	return device_id;
}

bool CFG_OverrideFreezeProtect()
{
	return (cfg_true == cfg_getbool(_config_parameters, CONFIG_BOOL_OVERRIDE_FREEZE_PROTECT));
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

int CFG_DzIdxAirTemp()
{
	return cfg_getint(_config_parameters, CONFIG_INT_DZIDX_AIR_TEMP);
}

int CFG_DzIdxPoolWaterTemp()
{
	return cfg_getint(_config_parameters, CONFIG_INT_DZIDX_POOL_WATER_TEMP);
}

int CFG_DzIdxSpaWaterTemp()
{
	return cfg_getint(_config_parameters, CONFIG_INT_DZIDX_SPA_WATER_TEMP);
}

int CFG_DzIdxSwgPercent()
{
	return cfg_getint(_config_parameters, CONFIG_INT_DZIDX_SWG_PERCENT);
}

int CFG_DzIdxSwgPpm()
{
	return cfg_getint(_config_parameters, CONFIG_INT_DZIDX_SWG_PPM);
}

int CFG_DzIdxSwgStatus()
{
	return cfg_getint(_config_parameters, CONFIG_INT_DZIDX_SWG_STATUS);
}

double CFG_LightProgrammingMode()
{
	return cfg_getfloat(_config_parameters, CONFIG_FLOAT_LIGHT_PROGRAMMING_MODE);
}

int CFG_LightProgrammingInitialOn()
{
	return cfg_getint(_config_parameters, CONFIG_INT_LIGHT_PROGRAMMING_INITIAL_ON);
}

int CFG_LightProgrammingInitialOff()
{
	return cfg_getint(_config_parameters, CONFIG_INT_LIGHT_PROGRAMMING_INITIAL_OFF);
}

int CFG_LightProgrammingButtonPool()
{
	return cfg_getint(_config_parameters, CONFIG_INT_LIGHT_PROGRAMMING_BUTTON_POOL);
}

int CFG_LightProgrammingButtonSpa()
{
	return cfg_getint(_config_parameters, CONFIG_INT_LIGHT_PROGRAMMING_BUTTON_SPA);
}

bool CFG_NoDaemonize()
{
	return (cfg_true == cfg_getbool(_config_parameters, CONFIG_BOOL_NO_DAEMONIZE));
}

const char* CFG_LogFile()
{
	return cfg_getstr(_config_parameters, CONFIG_STR_LOG_FILE);
}

bool CFG_PdaMode()
{
	return (cfg_true == cfg_getbool(_config_parameters, CONFIG_BOOL_PDA_MODE));
}

bool CFG_PdaSleepMode()
{
	return (cfg_true == cfg_getbool(_config_parameters, CONFIG_BOOL_PDA_SLEEP_MODE));
}

bool CFG_ConvertMqttTemp()
{
	return (cfg_true == cfg_getbool(_config_parameters, CONFIG_BOOL_CONVERT_MQTT_TEMP_TO_C));
}

bool CFG_ConvertDzTemp()
{
	return (cfg_true == cfg_getbool(_config_parameters, CONFIG_BOOL_CONVERT_DZ_TEMP_TO_C));
}

bool CFG_ReportZeroPoolTemp()
{
	return (cfg_true == cfg_getbool(_config_parameters, CONFIG_BOOL_REPORT_ZERO_POOL_TEMP));
}

bool CFG_ReportZeroSpaTemp()
{
	return (cfg_true == cfg_getbool(_config_parameters, CONFIG_BOOL_REPORT_ZERO_SPA_TEMP));
}

bool CFG_ReadAllDevices()
{
	return (cfg_true == cfg_getbool(_config_parameters, CONFIG_BOOL_READ_ALL_DEVICES));
}

bool CFG_UsePanelAuxLabels()
{
	return (cfg_true == cfg_getbool(_config_parameters, CONFIG_BOOL_USE_PANEL_AUX_LABELS));
}

bool CFG_ForceSwg()
{
	return (cfg_true == cfg_getbool(_config_parameters, CONFIG_BOOL_FORCE_SWG));
}

int CFG_SwgZeroIgnore()
{
	return cfg_getint(_config_parameters, CONFIG_INT_SWG_ZERO_IGNORE);
}

bool CFG_ReadPentairPackets()
{
	return (cfg_true == cfg_getbool(_config_parameters, CONFIG_BOOL_READ_PENTAIR_PACKETS));
}

bool CFG_DisplayWarningsWeb()
{
	return (cfg_true == cfg_getbool(_config_parameters, CONFIG_BOOL_DISPLAY_WARNINGS_IN_WEB));
}

bool CFG_PlaybackMode()
{
	return (cfg_true == cfg_getbool(_config_parameters, CONFIG_BOOL_PLAYBACK_MODE));
}

bool CFG_RecordMode()
{
	return (cfg_true == cfg_getbool(_config_parameters, CONFIG_BOOL_RECORD_MODE));
}

const char* CFG_RawSerial_LogFile()
{
	return cfg_getstr(_config_parameters, CONFIG_STR_RAW_SERIAL_LOG_FILE);
}

const char* CFG_ButtonFilterPumpLabel()
{
	return cfg_getstr(_config_parameters, CONFIG_STR_BUTTON_FILTERPUMP_LABEL);
}

const char* CFG_ButtonSpaModeLabel()
{
	return cfg_getstr(_config_parameters, CONFIG_STR_BUTTON_SPAMODE_LABEL);
}

const char* CFG_ButtonAux1Label()
{
	return cfg_getstr(_config_parameters, CONFIG_STR_BUTTON_AUX1_LABEL);
}

const char* CFG_ButtonAux2Label()
{
	return cfg_getstr(_config_parameters, CONFIG_STR_BUTTON_AUX2_LABEL);
}

const char* CFG_ButtonAux3Label()
{
	return cfg_getstr(_config_parameters, CONFIG_STR_BUTTON_AUX3_LABEL);
}

const char* CFG_ButtonAux4Label()
{
	return cfg_getstr(_config_parameters, CONFIG_STR_BUTTON_AUX4_LABEL);
}

const char* CFG_ButtonAux5Label()
{
	return cfg_getstr(_config_parameters, CONFIG_STR_BUTTON_AUX5_LABEL);
}

const char* CFG_ButtonAux6Label()
{
	return cfg_getstr(_config_parameters, CONFIG_STR_BUTTON_AUX6_LABEL);
}

const char* CFG_ButtonAux7Label()
{
	return cfg_getstr(_config_parameters, CONFIG_STR_BUTTON_AUX7_LABEL);
}

const char* CFG_ButtonPoolHeaterLabel()
{
	return cfg_getstr(_config_parameters, CONFIG_STR_BUTTON_POOLHEATER_LABEL);
}

const char* CFG_ButtonSpaHeaterLabel()
{
	return cfg_getstr(_config_parameters, CONFIG_STR_BUTTON_SPAHEATER_LABEL);
}

const char* CFG_ButtonSolarHeaterLabel()
{
	return cfg_getstr(_config_parameters, CONFIG_STR_BUTTON_SOLARHEATER_LABEL);
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

void CFG_Set_LogLevel(LoggingLevels logLevel)
{
	cfg_setint(_config_parameters, CONFIG_INT_LOG_LEVEL, (int)logLevel);
}

void CFG_Set_Insecure(bool insecure)
{
	cfg_setbool(_config_parameters, CONFIG_BOOL_INSECURE, (insecure) ? cfg_true : cfg_false);
}

void CFG_Set_DeviceId(HardwareDeviceId deviceId)
{
	unsigned char calculated_id = ((deviceId.Type & deviceId.Instance) & 0xFF);
	cfg_setint(_config_parameters, CONFIG_INT_DEVICE_ID, (int)calculated_id);
}

void CFG_Set_NoDaemonize(bool daemonize)
{
	cfg_setbool(_config_parameters, CONFIG_BOOL_NO_DAEMONIZE, (daemonize) ? cfg_true : cfg_false);
}

void CFG_Set_LogFile(const char* logFile)
{
	cfg_setstr(_config_parameters, CONFIG_STR_LOG_FILE, logFile);
}

void CFG_Set_PlaybackMode(bool modeIsEnabled)
{
	cfg_setbool(_config_parameters, CONFIG_BOOL_PLAYBACK_MODE, (modeIsEnabled) ? cfg_true : cfg_false);
}

void CFG_Set_RecordMode(bool modeIsEnabled)
{
	cfg_setbool(_config_parameters, CONFIG_BOOL_RECORD_MODE, (modeIsEnabled) ? cfg_true : cfg_false);
}

void CFG_Set_RawSerial_LogFile(const char* logfile)
{
	cfg_setstr(_config_parameters, CONFIG_STR_RAW_SERIAL_LOG_FILE, logfile);
}
