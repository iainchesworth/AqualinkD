#include "logging_sink_console_terminal.h"

#include "cross-platform/terminal.h"

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
