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
*  animation.h: animations structures
*  ---------------------------------------------------------------------------
*/

#ifndef CE_ANIMATION_H_
#define CE_ANIMATION_H_

#include <stdint.h>

// Animated sprites data
typedef struct
{
	uint32_t	index;	// index for the ani in the LOADER table
	int32_t	framecount;
	uint32_t end_of_ani_parameter;
	void(*end_of_ani_function)(uint32_t);
} s_animation;

#endif