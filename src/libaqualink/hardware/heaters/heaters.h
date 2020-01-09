#ifndef AQ_HEATERS_H_
#define AQ_HEATERS_H_

typedef enum tagHeaterModes
{
	HeaterIsOff,
	HeaterIsEnabled,
	HeaterIsOn
}
HeaterModes;

typedef struct tagHeater
{
	HeaterModes Mode;
}
Heater;

#endif // AQ_HEATERS_H_
