#ifndef AQ_CONFIG_HELPERS_H_
#define AQ_CONFIG_HELPERS_H_

#include <stdbool.h>

const char* CFG_ConfigFile();
const char* CFG_PidFile();
const char* CFG_SerialPort();
const int CFG_LogLevel();
const int CFG_SocketPort();
const int CFG_SslSocketPort();
const char* CFG_WebDirectory();
const bool CFG_Insecure();
const int CFG_DeviceId();
const bool CFG_OverrideFreezeProtect();

const char* CFG_MqttDzSubTopic();
const char* CFG_MqttDzPubTopic();
const char* CFG_MqttAqTopic();
const char* CFG_MqttServer();
const char* CFG_MqttUser();
const char* CFG_MqttPassword();
const char* CFG_MqttId();

const int CFG_DzIdxAirTemp();
const int CFG_DzIdxPoolWaterTemp();
const int CFG_DzIdxSpaWaterTemp();
const int CFG_DzIdxSwgPercent();
const int CFG_DzIdxSwgPpm();
const int CFG_DzIdxSwgStatus();

const float CFG_LightProgrammingMode();
const int CFG_LightProgrammingInitialOn();
const int CFG_LightProgrammingInitialOff();
const int CFG_LightProgrammingButtonPool();
const int CFG_LightProgrammingButtonSpa();

const bool CFG_NoDaemonize();
const char* CFG_LogFile();

const bool CFG_PdaMode();
const bool CFG_PdaSleepMode();

const bool CFG_ConvertMqttTemp();
const bool CFG_ConvertDzTemp();
const bool CFG_ReportZeroPoolTemp();
const bool CFG_ReportZeroSpaTemp();
const bool CFG_ReadAllDevices();
const bool CFG_UsePanelAuxLabels();
const bool CFG_ForceSwg();
const int CFG_SwgZeroIgnore();
const bool CFG_ReadPentairPackets();
const bool CFG_DisplayWarningsWeb();
const bool CFG_DebugRsProtocolPackets();
const bool CFG_LogRawRsBytes();
const char* CFG_LogRawRsBytes_LogFile();

// Setters

void CFG_Set_ConfigFile(const char* configFile);
void CFG_Set_PidFile(const char* pidFile);
void CFG_Set_SerialPort(const char* serialPort);
void CFG_Set_LogLevel(int logLevel);
void CFG_Set_Insecure(bool insecure);
void CFG_Set_NoDaemonize(bool daemonize);
void CFG_Set_LogFile(const char* logFile);
void CFG_Set_DebugRsProtocolPackets(bool debugRsProtocolPackets);
void CFG_Set_LogRawRsBytes(bool logRawRsBytes);
void CFG_Set_LogRawRsBytes_LogFile(const char* logRawRsBytes_LogFile);

#endif // AQ_CONFIG_HELPERS_H_
