#ifndef AQ_RS_BUTTONS_H_
#define AQ_RS_BUTTONS_H_

typedef enum tagAqualinkRS_ButtonStates
{
	NotPresent,
	PresentButDisabled,

	ButtonIsOff,
	ButtonIsOn,
	ButtonIsFlashing,

	///FIXME OLD STUFF HERE:

	LedOn = 0,
	LedOff = 1,
	LedFlashing = 2,
	LedEnabled = 3,

	UnknownLedState = -1
	
}
AqualinkRS_ButtonStates;

typedef struct tagAqualinkRS_Button
{
	AqualinkRS_ButtonStates State;
	const char* Label;
	
	///FIXME OLD STUFF HERE:
	
	char* label;
	char* name;
	char* pda_label;
	unsigned char code;
	int dz_idx;
}
AqualinkRS_Button;

typedef enum tagAqualinkRS_ButtonTypes
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
	ButtonTypeCount = 12		// TOTAL_BUTTONS
}
AqualinkRS_ButtonTypes;

typedef enum AqualinkRS_KeypadMenuButtonTypes
{
	Menu,
	Cancel,
	Back,
	Forward,
	Enter,
	Hold,
	Override,
	KeypadMenuButtonTypeCount
}
AqualinkRS_KeypadMenuButtonTypes;

// Forward declarations.
struct aqualinkdata;

void init_buttons(struct aqualinkdata* aqdata);

#endif // AQ_RS_BUTTONS_H_
