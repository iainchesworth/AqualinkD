#include "json_string_utils.h"

#include "serial/aq_serial_types.h"

const char* LED2text(AQ_LED_States state)
{
	char* returnText = 0;

	switch (state) {
	case ON:
		returnText = JSON_ON;
		break;

	case OFF:
		returnText = JSON_OFF;
		break;

	case FLASH:
		returnText = JSON_FLASH;
		break;

	case ENABLE:
		returnText = JSON_ENABLED;
		break;

	case LED_S_UNKNOWN:
	default:
		returnText = "unknown";
		break;
	}

	return returnText;
}

int LED2int(AQ_LED_States state)
{
	return state;
}
