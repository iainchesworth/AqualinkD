#ifndef AQ_BUTTONS_H_
#define AQ_BUTTONS_H_

typedef enum tagAqualinkButtonLedStates
{
	LedOn = 0,
	LedOff = 1,
	LedFlashing = 2,
	LedEnabled = 3,

	UnknownLedState = -1
}
AqualinkButtonLedStates;

typedef struct tagAqualinkButtonLed
{
	AqualinkButtonLedStates state;
}
AqualinkButtonLed;

typedef struct tagAqualinkbutton
{
	AqualinkButtonLed* led;
	char* label;
	char* name;
	char* pda_label;
	unsigned char code;
	int dz_idx;
}
AqualinkButton;

typedef enum tagAqualinkButtons
{
	FilterPump = 0,				// PUMP_INDEX
	SpaMode = 1,				// SPA_INDEX
	Aux_1,
	Aux_2,
	Aux_3,
	Aux_4,
	Aux_5,
	Aux_6,
	Aux_7,
	PoolHeater = 9,				// POOL_HEAT_INDEX
	SpaHeater = 10,				// SPA_HEAT_INDEX
	SolarHeater,
	AqualinkButtonCount = 12	// TOTAL_BUTTONS
}
AqualinkButtons;

// Forward declarations.
struct aqualinkdata;

void init_buttons(struct aqualinkdata* aqdata);

#endif
