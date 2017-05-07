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
*  event.h: events structures
*  ---------------------------------------------------------------------------
*/

#ifndef CE_EVENT_H_
#define CE_EVENT_H_

#include <stdint.h>

// Timed events
typedef struct
{
	uint64_t	expiration_time;
	uint32_t parameter;
	void(*function)(uint32_t);
} s_event;

// Stack for time delayed events. Doubt we'll need more than that
#define NB_EVENTS				32

extern s_event	events[NB_EVENTS];

void events_resetCallbacks();

extern uint64_t game_time;
void events_apply();
void enqueue_event(void(*f)(uint32_t), uint32_t p, uint64_t delay);


#endif