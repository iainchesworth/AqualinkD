#ifndef AQ_CONTROLLER_H_
#define AQ_CONTROLLER_H_

#include "hardware/buttons/buttons.h"

typedef enum tagAqualinkControllerVariants
{
	RS4, RS6, RS8, RS12, RS16, RS24, RS32,
	PDA4, PDA6, PDA8,
	ZQ4,
	UnknownControllerVariant
}
AqualinkControllerVariants;

typedef struct tagAqualinkController
{
	AqualinkControllerVariants Variant;

	// Buttons
	AqualinkButton Buttons[AqualinkButtonCount];

	// Pumps

	// Heaters

	// Interfaces

}
AqualinkController;

void init_system_board(AqualinkController* controller, AqualinkControllerVariants variant);

#endif // AQ_CONTROLLER_H_
