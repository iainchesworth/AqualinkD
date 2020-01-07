

/*
 * Copyright (c) 2017 Shaun Feakes - All rights reserved
 *
 * You may use redistribute and/or modify this code under the terms of
 * the GNU General Public License version 2 as published by the
 * Free Software Foundation. For the terms of this license,
 * see <http://www.gnu.org/licenses/>.
 *
 * You are free to use this software under the terms of the GNU General
 * Public License, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 *  https://github.com/sfeakes/aqualinkd
 */


#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "pda_menu.h"
#include "config/config_helpers.h"
#include "logging/logging.h"
#include "serial/aq_serial.h"
#include "string/string_utils.h"
#include "utils.h"

int _hlightindex = -1;
char _menu[PDA_LINES][AQ_MSGLEN + 1];

void print_menu()
{
	int i;
	for (i = 0; i < PDA_LINES; i++) {
		DEBUG("PDA Menu Line %d = %s", i, _menu[i]);
	}

	if (_hlightindex > -1) {
		DEBUG("PDA Menu highlighted line = %d = %s", _hlightindex, _menu[_hlightindex]);
	}
}

int pda_m_hlightindex()
{
	return _hlightindex;
}

char* pda_m_hlight()
{
	return pda_m_line(_hlightindex);
}

char* pda_m_line(int index)
{
	if (index >= 0 && index < PDA_LINES)
	{
		return _menu[index];
	}


	return "-"; // Just return something bad so I can use string comparison with no null check
}

// Find exact menu item
int pda_find_m_index(char* text)
{
	int i;

	for (i = 0; i < PDA_LINES; i++)
	{
		if (strncmp(pda_m_line(i), text, strlen(text)) == 0)
		{
			return i;
		}
	}

	return -1;
}

// Fine menu item case insensative
int pda_find_m_index_case(char* text, int limit)
{
	int i;

	for (i = 0; i < PDA_LINES; i++)
	{
		if (aq_strnicmp(pda_m_line(i), text, limit) == 0)
		{
			return i;
		}
	}

	return -1;
}

// Find menu item very loose
int pda_find_m_index_loose(char* text)
{
	int i;

	for (i = 0; i < PDA_LINES; i++)
	{
		if (strstr(pda_m_line(i), text) != NULL)
		{
			return i;
		}
	}

	return -1;
}

pda_menu_type pda_m_type()
{
	pda_menu_type retValue = PM_UNKNOWN;


	if (aq_strnicmp(_menu[1], "AIR  ", 5) == 0)
	{
		retValue = PM_HOME;
	}
	else if (aq_strnicmp(_menu[0], "EQUIPMENT STATUS", 16) == 0)
	{
		retValue = PM_EQUIPTMENT_STATUS;
	}
	else if (aq_strnicmp(_menu[0], "   EQUIPMENT    ", 16) == 0)
	{
		retValue = PM_EQUIPTMENT_CONTROL;
	}
	else if (aq_strnicmp(_menu[0], "   MAIN MENU    ", 16) == 0)
	{
		retValue = PM_MAIN;
	}
	else if (aq_strnicmp(_menu[4], "POOL MODE", 9) == 0 || aq_strnicmp(_menu[9], "EQUIPMENT ON/OFF", 16) == 0)
	{
		// Will not see POOL MODE if single device config (pool vs pool&spa)

		if (pda_m_hlightindex() == -1)
		{
			retValue = PM_BUILDING_HOME;
		}
		else
		{
			retValue = PM_HOME;
		}
	}
	else if (aq_strnicmp(_menu[0], "    SET TEMP    ", 16) == 0)
	{
		retValue = PM_SET_TEMP;
	}
	else if (aq_strnicmp(_menu[0], "    SET TIME    ", 16) == 0)
	{
		retValue = PM_SET_TIME;
	}
	else if (aq_strnicmp(_menu[0], "  SET AquaPure  ", 16) == 0)
	{
		retValue = PM_AQUAPURE;
	}
	else if (aq_strnicmp(_menu[0], "    SPA HEAT    ", 16) == 0)
	{
		retValue = PM_SPA_HEAT;
	}
	else if (aq_strnicmp(_menu[0], "   POOL HEAT    ", 16) == 0)
	{
		retValue = PM_POOL_HEAT;
	}
	else if (aq_strnicmp(_menu[0], "  SYSTEM SETUP  ", 16) == 0)
	{
		retValue = PM_SYSTEM_SETUP;
	}
	else if (aq_strnicmp(_menu[6], "Use ARROW KEYS  ", 16) == 0 && aq_strnicmp(_menu[0], " FREEZE PROTECT ", 16) == 0)
	{
		retValue = PM_FREEZE_PROTECT;
	}
	else if (aq_strnicmp(_menu[1], "    DEVICES     ", 16) == 0 && aq_strnicmp(_menu[0], " FREEZE PROTECT ", 16) == 0)
	{
		retValue = PM_FREEZE_PROTECT_DEVICES;
	}
	else if (aq_strnicmp(_menu[3], "Firmware Version", 16) == 0 || aq_strnicmp(_menu[1], "    AquaPalm", 12) == 0 || aq_strnicmp(_menu[1], " PDA-P", 6) == 0)
	{
		// PDA-P4 Only -or- PDA-PS4 Combo
		retValue = PM_FW_VERSION;
	}
	else if (aq_strnicmp(_menu[0], "   LABEL AUX    ", 16) == 0) \
	{
		// Catch AUX and not AUX4
		retValue = PM_AUX_LABEL;
	}
	else if (aq_strnicmp(_menu[0], "   LABEL AUX", 12) == 0 && aq_strnicmp(_menu[2], "  CURRENT LABEL ", 16) == 0)
	{
		// Will have number ie AUX4
		retValue = PM_AUX_LABEL_DEVICE;
	}
	else if (strstr(_menu[0], "BOOST"))
	{
		// This is bad check, but PDA menus are BOOST | BOOST POOL | BOOST SPA, need to do better.
		retValue = PM_BOOST;
	}

	return retValue;
}

/*
--- Main Menu ---
Line 0 =
Line 1 = AIR
(Line 4 first, Line 2 last, Highligh when finished)
--- Equiptment Status  ---
Line 0 = EQUIPMENT STATUS
(Line 0 is first. No Highlight, everything in list is on)
--- Equiptment on/off menu --
Line 0 =    EQUIPMENT
(Line 0 is first. Highlight when complete)
*/

bool process_pda_menu_packet(unsigned char* packet, int length)
{
	bool rtn = true;
	signed char first_line;
	signed char last_line;
	signed char line_shift;
	signed char i;

	switch (packet[PKT_CMD]) {
	case CMD_PDA_CLEAR:
		_hlightindex = -1;
		memset(_menu, 0, PDA_LINES * (AQ_MSGLEN + 1));
		break;
	case CMD_MSG_LONG:
		if (packet[PKT_DATA] < 10) {
			memset(_menu[packet[PKT_DATA]], 0, AQ_MSGLEN);
			strncpy(_menu[packet[PKT_DATA]], (char*)packet + PKT_DATA + 1, AQ_MSGLEN);
			_menu[packet[PKT_DATA]][AQ_MSGLEN] = '\0';
		}
		if (CFG_LogLevel() >= Debug) { print_menu(); }
		break;
	case CMD_PDA_HIGHLIGHT:
		// when switching from hlight to hlightchars index 255 is sent to turn off hlight
		if (packet[4] <= PDA_LINES) {
			_hlightindex = packet[4];
		}
		else {
			_hlightindex = -1;
		}
		if (CFG_LogLevel() >= Debug) { print_menu(); }
		break;
	case CMD_PDA_HIGHLIGHTCHARS:
		if (packet[4] <= PDA_LINES) {
			_hlightindex = packet[4];
		}
		else {
			_hlightindex = -1;
		}
		if (CFG_LogLevel() >= Debug) { print_menu(); }
		break;
	case CMD_PDA_SHIFTLINES:
		/// press up from top - shift menu down by 1
		 //   PDA Shif | HEX: 0x10|0x02|0x62|0x0f|0x01|0x08|0x01|0x8d|0x10|0x03|
		 // press down from bottom - shift menu up by 1
		 //   PDA Shif | HEX: 0x10|0x02|0x62|0x0f|0x01|0x08|0xff|0x8b|0x10|0x03|
		first_line = (signed char)(packet[4]);
		last_line = (signed char)(packet[5]);
		line_shift = (signed char)(packet[6]);
		DEBUG("\n");
		if (line_shift < 0) {
			for (i = first_line - line_shift; i <= last_line; i++) {
				memcpy(_menu[i + line_shift], _menu[i], AQ_MSGLEN + 1);
			}
		}
		else {
			for (i = last_line; i >= first_line + line_shift; i--) {
				memcpy(_menu[i], _menu[i - line_shift], AQ_MSGLEN + 1);
			}
		}
		if (CFG_LogLevel() >= Debug) { print_menu(); }
		break;
	}

	return rtn;
}
