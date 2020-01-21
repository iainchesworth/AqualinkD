#ifndef AQ_CONFIG_HELPERS_H_
#define AQ_CONFIG_HELPERS_H_

#include <stdbool.h>

#include "hardware/devices/hardware_device_types.h"
#include "logging/logging_levels.h"

const char* CFG_ConfigFile(void);
const char* CFG_PidFile(void);
const char* CFG_SerialPort(void);
LoggingLevels CFG_LogLevel(void);
int CFG_SocketPort(void);
int CFG_SslSocketPort(void);
const char* CFG_WebDirectory(void);
bool CFG_Insecure(void);
HardwareDeviceId CFG_DeviceId(void);
bool CFG_OverrideFreezeProtect(void);

const char* CFG_MqttDzSubTopic(void);
const char* CFG_MqttDzPubTopic(void);
const char* CFG_MqttAqTopic(void);
const char* CFG_MqttServer(void);
const char* CFG_MqttUser(void);
const char* CFG_MqttPassword(void);
const char* CFG_MqttId(void);

int CFG_DzIdxAirTemp(void);
int CFG_DzIdxPoolWaterTemp(void);
int CFG_DzIdxSpaWaterTemp(void);
int CFG_DzIdxSwgPercent(void);
int CFG_DzIdxSwgPpm(void);
int CFG_DzIdxSwgStatus(void);

double CFG_LightProgrammingMode(void);
int CFG_LightProgrammingInitialOn(void);
int CFG_LightProgrammingInitialOff(void);
int CFG_LightProgrammingButtonPool(void);
int CFG_LightProgrammingButtonSpa(void);

bool CFG_NoDaemonize(void);
const char* CFG_LogFile(void);

bool CFG_PdaMode(void);
bool CFG_PdaSleepMode(void);

bool CFG_ConvertMqttTemp(void);
bool CFG_ConvertDzTemp(void);
bool CFG_ReportZeroPoolTemp(void);
bool CFG_ReportZeroSpaTemp(void);
bool CFG_ReadAllDevices(void);
bool CFG_UsePanelAuxLabels(void);
bool CFG_ForceSwg(void);
int CFG_SwgZeroIgnore(void);
bool CFG_ReadPentairPackets(void);
bool CFG_DisplayWarningsWeb(void);

bool CFG_PlaybackMode(void);
bool CFG_RecordMode(void);
const char* CFG_RawSerial_LogFile(void);

const char* CFG_ButtonFilterPumpLabel(void);
const char* CFG_ButtonSpaModeLabel(void);
const char* CFG_ButtonAux1Label(void);
const char* CFG_ButtonAux2Label(void);
const char* CFG_ButtonAux3Label(void);
const char* CFG_ButtonAux4Label(void);
const char* CFG_ButtonAux5Label(void);
const char* CFG_ButtonAux6Label(void);
const char* CFG_ButtonAux7Label(void);
const char* CFG_ButtonPoolHeaterLabel(void);
const char* CFG_ButtonSpaHeaterLabel(void);
const char* CFG_ButtonSolarHeaterLabel(void);

// Setters

void CFG_Set_ConfigFile(const char* configFile);
void CFG_Set_PidFile(const char* pidFile);
void CFG_Set_SerialPort(const char* serialPort);
void CFG_Set_LogLevel(LoggingLevels logLevel);
void CFG_Set_WebDirectory(const char* webRoot);
void CFG_Set_Insecure(bool insecure);
void CFG_Set_DeviceId(HardwareDeviceId deviceId);
void CFG_Set_NoDaemonize(bool daemonize);
void CFG_Set_LogFile(const char* logFile);
void CFG_Set_PlaybackMode(bool modeIsEnabled);
void CFG_Set_RecordMode(bool modeIsEnabled);
void CFG_Set_RawSerial_LogFile(const char* logfile);

#endif // AQ_CONFIG_HELPERS_H_
