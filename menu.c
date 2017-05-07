/*
*  Colditz Escape! - Rewritten Engine for "Escape From Colditz"
*  copyright (C) 2008-2009 Aperture Software
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 3 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License along
*  with this program; if not, write to the Free Software Foundation, Inc.,
*  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*
*  ---------------------------------------------------------------------------
*  menu.c: menu functions
*  ---------------------------------------------------------------------------
*/
#include "menu.h"

#define NULL 0
/*
*	Menu variables & consts
*/
int	  selected_menu_item, selected_menu;
char save_list[NB_SAVEGAMES][20];
char* menus[NB_MENUS][NB_MENU_ITEMS] = {
	{ " MAIN MENU", "", "", "BACK TO GAME", "RESET GAME", "SAVE", "LOAD", "OPTIONS", "", "EXIT GAME" } ,
	{ " OPTIONS MENU", "", "", "BACK TO MAIN MENU", "SKIP INTRO     ", "PICTURE CORNERS",
	"GFX SMOOTHING  ", "FULLSCREEN     ", "ENHANCEMENTS   ", "ORIGINAL MODE  " },
	// The save slots of the following menus will be filled at runtime
	{ " SAVE MENU", "", "", "BACK TO MAIN MENU", NULL, NULL, NULL, NULL, NULL, NULL },
	{ " LOAD MENU", "", "", "BACK TO MAIN MENU", NULL, NULL, NULL, NULL, NULL, NULL } };
char* on_off[3] = { "", ":ON", ":OFF" };
char* smoothing[4] = { ":NONE", ":LINEAR", ":HQ2X" };

bool  enabled_menus[NB_MENUS][NB_MENU_ITEMS] = {
	{ 0, 0, 0, 1, 1, 1, 1, 1, 0, 1 },
#if defined(PSP)
	// Options like linear interprolation and fullscreen don't make sense on PSP
	{ 0, 0, 0, 1, 1, 1, 0, 0, 1, 1 },
#else
	{ 0, 0, 0, 1, 1, 1, 1, 1, 1, 1 },
#endif
	{ 0, 0, 0, 1, 1, 1, 1, 1, 1, 1 },	// Save: all slots enabled by default
	{ 0, 0, 0, 1, 0, 0, 0, 0, 0, 0 }	// Load: no slots available by default
};
