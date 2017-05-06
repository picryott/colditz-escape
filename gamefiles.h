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
*  gamefiles.h: Game files open/read
*  ---------------------------------------------------------------------------
*/

#ifndef _GAMEFILES_H_
#define _GAMEFILES_H_

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

	// # data files from the original game
#define NB_FILES				11
	// # files that need reload on a new game
#define NB_FILES_TO_RELOAD		4
	// # file buffers that go into a savegame
#define NB_FILES_TO_SAVE		5
	// Some handy identifier for files to make code reader friendly
#define ROOMS					0
#define COMPRESSED_MAP			1
#define OBJECTS					2
#define TUNNEL_IO				3
#define GUARDS					4
#define ROUTES					5
#define SPRITES_PANEL			6
#define CELLS					7
#define PALETTES				8
#define LOADER					9
#define SPRITES					10

	// Files definitions
#define FNAMES					{ "COLDITZ_ROOM_MAPS",	\
								  "COMPRESSED_MAP",		\
								  "OBS.BIN",			\
								  "TUNNELIODOORS.BIN",	\
								  "MENDAT.BIN",			\
								  "ROUTES.BIN",			\
								  "PANEL.BIN",			\
								  "COLDITZ_CELLS",		\
								  "PALS.BIN",			\
								  "COLDITZ-LOADER",		\
								  "SPRITES.SPR"			}
#define FSIZES					{ 58828,				\
								  33508,				\
								  2056,					\
								  120,					\
								  1288,					\
								  13364,				\
								  11720,				\
								  135944,				\
								  232,					\
								  56080,				\
								  71056					}
	// Most of the archive versions from the net use the Skid Row loader
#define ALT_LOADER				"SKR_COLD"
#define ALT_LOADER_SIZE			28820
#define ALT_LOADER_SIZE2        27940
	// If we place our loader at address 0x80, we won't have to convert the pointers from the disassembly ;)
#define LOADER_PADDING			0x00000080

extern uint8_t		*fbuffer[NB_FILES];
extern char			*fname[NB_FILES];
extern uint32_t		fsize[NB_FILES];

void gamefiles_init();

bool loadGameFiles();
bool reload_files();
void fix_files(bool reload);

/*
*	SAVE AND LOAD FUNCTIONS
*/
#define SAVE_SINGLE(el)  if (fwrite(&el, sizeof(el), 1, fd) != 1) return false
#define SAVE_ARRAY(ar)   if (fwrite(ar, sizeof(ar[0]), SIZE_A(ar), fd) != SIZE_A(ar)) return false;
#define SAVE_BUFFER(buf) if (fwrite(fbuffer[buf], 1, fsize[buf], fd) !=  fsize[buf]) return false;
#define LOAD_SINGLE(el)  if (fread(&el, sizeof(el), 1 , fd) != 1) return false
#define LOAD_ARRAY(ar)   if (fread(ar, sizeof(ar[0]), SIZE_A(ar), fd) != SIZE_A(ar)) return false;
#define LOAD_BUFFER(buf) if (fread(fbuffer[buf], 1, fsize[buf], fd) !=  fsize[buf]) return false;



// MD5 hashes of the game data files


#ifdef	__cplusplus
}
#endif

#endif