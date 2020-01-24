#include "onetouch_simulator_msg_long_handler.h"

#include <stdbool.h>

#include "logging/logging.h"
#include "messages/message-processors/aq_serial_message_senders.h"
#include "messages/aq_serial_message_msg_long.h"
#include "simulators/onetouch/onetouch_simulator_private.h"
#include "simulators/onetouch/onetouch_simulator_screen.h"

#include "aqualink.h"

bool onetouch_simulator_msglongmessagehandler(AQ_Msg_Long_Packet* packet)
{
	UNREFERENCED_PARAMETER(packet);

	bool handled_msglong_message = false;

	if ((!aqualink_onetouch_simulator.Config.IsInitialised) && (!onetouch_simulator_initmutex()))
	{
		ERROR("Failed to initialise Aqualink OneTouch Simulator mutex");
	}
	else if (thrd_error == mtx_lock(&(aqualink_onetouch_simulator.Config.SimulatorAccessMutex)))
	{
		ERROR("Failed to lock Aqualink OneTouch Simulator mutex");
	}
	else if (!aqualink_onetouch_simulator.IsEnabled)
	{
		WARN("Aqualink OneTouch Simulator is DISABLED but was asked to handle a MSG LONG message");
	}
	else
	{
		onetouch_screen_write_page_line(OneTouch_Screen_Page1, ((OneTouch_ScreenLines)packet->LineNumber), packet->Message);
		
		DEBUG("Simulator successfully handled MSG LONG message");
		handled_msglong_message = true;
	}

	if ((aqualink_onetouch_simulator.Config.IsInitialised) && (thrd_error == mtx_unlock(&(aqualink_onetouch_simulator.Config.SimulatorAccessMutex))))
	{
		ERROR("Failed to unlock Aqualink OneTouch Simulator mutex");
	}

	return handled_msglong_message;
}
