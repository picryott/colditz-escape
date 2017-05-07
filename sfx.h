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
*  sfx.h: SFXs structures
*  ---------------------------------------------------------------------------
*/

#ifndef CE_SFX_H_
#define CE_SFX_H_

#include <stdint.h>
#include <stdbool.h>

// Sound FXs
typedef struct
{
	uint32_t				address;
	uint16_t				length;
	uint16_t				psp_length;
	uint16_t				frequency;
	uint8_t				volume;
	short*			upconverted_address;
	unsigned long	upconverted_length;
} s_sfx;

// SFX data

#define NB_SFXS					5
#define SFX_DOOR				0
#define SFX_WTF					1
#define SFX_SAW					2
#define SFX_FOOTSTEPS			3
#define SFX_SHOOT				4


#define SFX_TABLE_START			0x0000CA3E
#define SFX_ADDRESS_START		0x0000CA6A

extern s_sfx sfx[NB_SFXS];

void sfxs_initialize(uint8_t* buffer);

void play_sfx(int sfx_id, uint8_t* buffer, bool thrillerdance);


void play_cluck(bool thrillerdance);
void thriller_toggle();


#endif