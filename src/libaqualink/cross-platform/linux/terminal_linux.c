#include "../terminal.h"

#if !defined (WIN32)

#include <stdlib.h>
#include <string.h>

bool terminal_supports_colour()
{
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
}

#endif // !defined WIN32
