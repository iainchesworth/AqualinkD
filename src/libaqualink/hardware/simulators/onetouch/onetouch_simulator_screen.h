#ifndef AQ_ONETOUCH_SIMULATOR_SCREEN_H_
#define AQ_ONETOUCH_SIMULATOR_SCREEN_H_

typedef enum OneTouch_ScreenPages
{
	OneTouch_Screen_Page1 = 0,
	OneTouch_Screen_Page2 = 1,
	OneTouch_Screen_PageCount
}
OneTouch_ScreenPages;

typedef enum OneTouch_ScreenLines
{
	OneTouch_Screen_Line1 = 0,
	OneTouch_Screen_Line2 = 1,
	OneTouch_Screen_Line3 = 2,
	OneTouch_Screen_Line4 = 3,
	OneTouch_Screen_Line5 = 4,
	OneTouch_Screen_Line6 = 5,
	OneTouch_Screen_Line7 = 6,
	OneTouch_Screen_Line8 = 7,
	OneTouch_Screen_Line9 = 8,
	OneTouch_Screen_Line10 = 9,
	OneTouch_Screen_Line11 = 10,
	OneTouch_Screen_Line12 = 11,
	OneTouch_Screen_LineCount
}
OneTouch_ScreenLines;

#define SCREEN_WIDTH_IN_CHARACTERS 16
extern unsigned char onetouch_simulator_screen[OneTouch_Screen_PageCount][OneTouch_Screen_LineCount][SCREEN_WIDTH_IN_CHARACTERS];

void onetouch_screen_clear_all_pages();
void onetouch_screen_clear_page(OneTouch_ScreenPages page_to_clear);
void onetouch_screen_write_page_line(OneTouch_ScreenPages page, OneTouch_ScreenLines line, const unsigned char content[]);

#endif // AQ_ONETOUCH_SIMULATOR_SCREEN_H_
