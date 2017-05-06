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
 *  anti-tampering.h: Optional anti tampering
 *  ---------------------------------------------------------------------------
 */

#pragma once

#ifdef	__cplusplus
extern "C" {
#endif


#if defined(ANTI_TAMPERING_ENABLED)
#include "md5.h"


// This inline performs an MD5 check on file i
extern void md5( unsigned char *input, int ilen, unsigned char output[16] );

static __inline bool integrity_check(uint8_t* buffer, uint32_t lsize, const uint8_t referenceMD5Hash[16])
{
	int j;
	uint8_t md5hash[16];
	md5(buffer, lsize, md5hash);
	for (j=0; j<16; j++)
		if (md5hash[j] != referenceMD5Hash[j])
			return false;
//	printf("{ ");
//	for (j=0; j<16; j++)
//		printf("0x%02x, ", md5hash[j]);
//	printf("}, \\\n");
	return true;
}
#endif

#ifdef	__cplusplus
}
#endif
