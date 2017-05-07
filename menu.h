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
*  menu.h: menu functions
*  ---------------------------------------------------------------------------
*/

#ifndef CE_MENU_H_
#define CE_MENU_H_


#include <stdbool.h>
/*
*	Menu stuff
*/
#define MIN_MENU_FADE			0.4f
#define MENU_MARKER				0x20
#define NB_MENUS				4
#define NB_MENU_ITEMS			10
#define FIRST_MENU_ITEM			3
#define MAIN_MENU				0
#define OPTIONS_MENU			1
#define SAVE_MENU				2
#define LOAD_MENU				3
#define ABOUT_MENU				4
#define NB_SAVEGAMES			6
// Items for the main menu
#define MENU_RETURN				3
#define MENU_RESTART			4
#define MENU_SAVE				5
#define MENU_LOAD				6
#define MENU_OPTIONS			7
#define MENU_EXIT				9
// Items for the options menu
#define MENU_BACK_TO_MAIN		3
#define MENU_SKIP_INTRO			4
#define MENU_PICTURE_CORNERS	5
#define MENU_SMOOTHING			6
#define MENU_FULLSCREEN			7
#define MENU_ENHANCEMENTS		8
#define MENU_ORIGINAL_MODE		9


extern int			selected_menu_item, selected_menu;
extern char*		menus[NB_MENUS][NB_MENU_ITEMS];
extern bool			enabled_menus[NB_MENUS][NB_MENU_ITEMS];

extern char save_list[NB_SAVEGAMES][20];
extern char* on_off[3];
extern char* smoothing[4];

#endif