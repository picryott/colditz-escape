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
*  event.c: events structures
*  ---------------------------------------------------------------------------
*/

#include "event.h"
#include "traces.h"

s_event		events[NB_EVENTS];


void events_resetCallbacks()
{
	uint16_t i;
	// clear the events array
	for (i = 0; i< NB_EVENTS; i++)
		events[i].function = NULL;
}
void events_apply()
{
	uint16_t i;
	for (i = 0; i<NB_EVENTS; i++)
	{
		if (events[i].function == NULL)
			continue;
		if (game_time > events[i].expiration_time)
		{	// Execute the timeout function
			events[i].function(events[i].parameter);
			// Make the event available again
			events[i].function = NULL;
		}
	}
}

// Simple event handler
void enqueue_event(void(*f)(uint32_t), uint32_t p, uint64_t delay)
{
	uint8_t i;

	// find an empty event to use
	for (i = 0; i< NB_EVENTS; i++)
		if (events[i].function == NULL)
			break;

	if (i == NB_EVENTS)
	{
		perr("Couldn't enqueue event!!!\n");
		return;
	}

	events[i].function = f;
	events[i].parameter = p;
	events[i].expiration_time = game_time + delay;
}