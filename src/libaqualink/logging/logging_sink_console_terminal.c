#include "logging_sink_console_terminal.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

static bool terminal_supports_colour()
{
#ifdef _WIN32

	#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
	#define ENABLE_VIRTUAL_TERMINAL_PROCESSING  0x0004
	#endif

	HANDLE hOut = ::GetStdHandle(STD_OUTPUT_HANDLE);
	if (hOut != INVALID_HANDLE_VALUE) 
	{
		DWORD dwMode = 0;
		::GetConsoleMode(hOut, &dwMode);
		dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
		return ::SetConsoleMode(hOut, dwMode) != 0;
	}
	return false;

#else // _WIN32
	
	const char* term;

	if (0 == (term = getenv("TERM")))
	{
		return 0 == strcmp(term, "cygwin")
			|| 0 == strcmp(term, "linux")
			|| 0 == strcmp(term, "rxvt-unicode-256color")
			|| 0 == strcmp(term, "screen")
			|| 0 == strcmp(term, "screen-256color")
			|| 0 == strcmp(term, "screen.xterm-256color")
			|| 0 == strcmp(term, "tmux-256color")
			|| 0 == strcmp(term, "xterm")
			|| 0 == strcmp(term, "xterm-256color")
			|| 0 == strcmp(term, "xterm-termite")
			|| 0 == strcmp(term, "xterm-color");
	}

	return false;

#endif
}

#ifdef _WIN32
#define VTSEQ(ID) ("\x1b[1;" #ID "m")
#else // _WIN32
#define VTSEQ(ID) ("\x1b[" #ID "m")
#endif

const char* terminal_black()
{
	return terminal_supports_colour() ? VTSEQ(30) : "";
}

const char* terminal_red()
{
	return terminal_supports_colour() ? VTSEQ(31) : "";
}

const char* terminal_green()
{
	return terminal_supports_colour() ? VTSEQ(32) : "";
}

const char* terminal_yellow()
{
	return terminal_supports_colour() ? VTSEQ(33) : "";
}

const char* terminal_blue()
{
	return terminal_supports_colour() ? VTSEQ(34) : "";
}

const char* terminal_purple()
{
	return terminal_supports_colour() ? VTSEQ(35) : "";
}

const char* terminal_cyan()
{
	return terminal_supports_colour() ? VTSEQ(36) : "";
}

const char* terminal_light_gray()
{
	return terminal_supports_colour() ? VTSEQ(37) : "";
}

const char* terminal_white()
{
	return terminal_supports_colour() ? VTSEQ(37) : "";
}

const char* terminal_light_red()
{
	return terminal_supports_colour() ? VTSEQ(91) : "";
}

const char* terminal_dim()
{
	return terminal_supports_colour() ? VTSEQ(2) : "";
}

// Formatting
const char* terminal_bold()
{
	return terminal_supports_colour() ? VTSEQ(1) : "";
}

const char* terminal_underline()
{
	return terminal_supports_colour() ? VTSEQ(4) : "";
}

// You should end each line with this!
const char* terminal_reset()
{
	return terminal_supports_colour() ? VTSEQ(0) : "";
}
