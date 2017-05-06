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
*  trace.h: traces functions definitions
*  ---------------------------------------------------------------------------
*/
#pragma once

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdbool.h>

extern bool trace_verbose;

/* initialize trace engine*/
void trace_init();
/* set the verbosity level for traces */
void trace_setVerbose(bool flag);


#if defined(PSP_ONSCREEN_STDOUT)
#define perr(...)		printf(__VA_ARGS__)
#else
#define perr(...)		fprintf(stderr, __VA_ARGS__)
#endif
#define print(...)		printf(__VA_ARGS__)
#define printv(...)		if(trace_verbose) print(__VA_ARGS__)
#define perrv(...)		if(trace_verbose) perr(__VA_ARGS__)
#define printb(...)		if(trace_verbose) print(__VA_ARGS__)
#define perrb(...)		if(trace_verbose) perr(__VA_ARGS__)

#ifdef	__cplusplus
}
#endif