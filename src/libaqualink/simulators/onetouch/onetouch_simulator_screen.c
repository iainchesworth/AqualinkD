#include "onetouch_simulator_screen.h"

#include <memory.h>

unsigned char onetouch_simulator_screen[OneTouch_Screen_PageCount][OneTouch_Screen_LineCount][SCREEN_WIDTH_IN_CHARACTERS];

void onetouch_screen_clear_all_pages()
{
	unsigned int page_index, line_index;

	for (page_index = 0; page_index < OneTouch_Screen_PageCount; ++page_index)
	{
		for (line_index = 0; line_index < OneTouch_Screen_LineCount; ++line_index)
		{
			memset(onetouch_simulator_screen[page_index][line_index], 0, SCREEN_WIDTH_IN_CHARACTERS);
		}
	}
}

void onetouch_screen_clear_page(OneTouch_ScreenPages page_to_clear)
{
	unsigned line_index;

	for (line_index = 0; line_index < OneTouch_Screen_LineCount; ++line_index)
	{
		memset(onetouch_simulator_screen[page_to_clear][line_index], 0, SCREEN_WIDTH_IN_CHARACTERS);
	}
}

void onetouch_screen_write_page_line(OneTouch_ScreenPages page, OneTouch_ScreenLines line, const unsigned char content[])
{
	memcpy(onetouch_simulator_screen[page][line], content, SCREEN_WIDTH_IN_CHARACTERS);
}
