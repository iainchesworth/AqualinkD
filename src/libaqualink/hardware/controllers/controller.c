#include "controller.h"

#include <assert.h>

void init_system_board(AqualinkController* controller, AqualinkControllerVariants variant)
{
	assert(0 != controller);

	controller->Variant = variant;

	switch (variant)
	{
	case RS4:
	case RS6:
	case RS8:
		break;

	case RS12:
	case RS16:
	case RS24:
	case RS32:
	case PDA4:
	case PDA6:
	case PDA8:
	case ZQ4:
	case UnknownControllerVariant:
	default:
		break;
	}
}