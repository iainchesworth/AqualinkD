#include "../terminal.h"

#if defined (WIN32)

#include <windows.h>

#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING  0x0004
#endif

static bool terminal_colour_is_initialised = false;
static bool terminal_colour_enabled = false;

bool terminal_supports_colour()
{
	if (terminal_colour_is_initialised)
	{
		// We've previously set the console mode so it's all good...do nothing
	}
	else
	{
		// Need to work out if the terminal supports colour...

		HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
		if (INVALID_HANDLE_VALUE != hOut)
		{
			DWORD dwMode = 0;

			GetConsoleMode(hOut, &dwMode);
			dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;

			terminal_colour_enabled = (0 != SetConsoleMode(hOut, dwMode));
		}
		else
		{
			terminal_colour_enabled = false;
		}

		// Okay, now that the support is/isn't determined, never repeat the check.
		terminal_colour_is_initialised = true;
	}

	return terminal_colour_enabled;
}

#endif // defined WIN32
