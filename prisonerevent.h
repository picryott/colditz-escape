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
*  prisonerevent.h: Event related states
*  ---------------------------------------------------------------------------
*/

#ifndef CE_PRISONER_EVENT_H_
#define CE_PRISONER_EVENT_H_

#include <stdint.h>
#include <stdbool.h>

// Event related states (applies to prisoners only)
typedef struct
{
	bool require_pass;
	bool require_papers;
	bool to_solitary;
	bool unauthorized;
	bool display_shot;
	bool killed;
	bool escaped;
	bool thrown_stone;
	uint64_t	 pass_grace_period_expires;
	uint32_t  fatigue;
	uint64_t  solitary_release;
} s_prisoner_event;

/*TODO : move this define elsewhere*/
#define NB_NATIONS				4

extern s_prisoner_event p_event[NB_NATIONS];

#endif