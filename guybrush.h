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
*  guybrush.h: guybrush structures
*  ---------------------------------------------------------------------------
*/

#ifndef CE_GUYBRUSH_H_
#define CE_GUYBRUSH_H_

#include <stdint.h>
#include "animation.h"

/*TODO: move the define somewhere else */
#define NB_NATIONS 4

// Guybrushes (prisoners or guards)
typedef struct
{
	uint16_t				room;					// Room index
	int16_t				px;
	int16_t				p2y;
	int16_t				speed;					// Walk = 1, Run = 2
												/* For animated overlays, direction is one of:
												*    3  2  4
												*    0  8  1
												*    6  5  7   */
	int16_t				direction;
	uint16_t				state;					// Motion related state (see above)
	uint32_t				ext_bitmask;			// Removable walls bitmask
	s_animation		animation;
	bool			reset_animation;
	bool			is_dressed_as_guard;
	bool			is_onscreen;
	// Guard activity variables
	bool			reinstantiate;
	bool			resume_motion;
	bool			blocked_by_prisoner;
	uint32_t				go_on;
	uint32_t				spent_in_room;
	uint16_t				wait;
	int16_t				target;
	int16_t				resume_px;
	int16_t				resume_p2y;
	int16_t				resume_direction;
	bool			fooled_by[NB_NATIONS];
} s_guybrush;

#endif