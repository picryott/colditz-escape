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
*  sprite.h: sprite structures
*  ---------------------------------------------------------------------------
*/

#ifndef CE_SPRITE_H_
#define CE_SPRITE_H_

#include <stdint.h>

// Structure to hold the standard RGBA sprites
typedef struct
{
	uint16_t w;
	uint16_t h;
	// Politicaly correct w & h (power of twos, to keep the PSP happy)
	uint16_t corrected_w;
	uint16_t corrected_h;
	int16_t x_offset;
	int16_t y_offset;
	int16_t z_offset;
	uint8_t* data;
} s_sprite;

// for nonstandard sprites (panel, etc)
typedef struct
{
	uint16_t w;
	uint16_t base;
	uint32_t offset;
} s_panel_sprite;

#endif