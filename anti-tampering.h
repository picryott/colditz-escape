/*
 *  Colditz Escape - Rewritten Engine for "Escape From Colditz"
 *  copyright (C) 2008-2017 Aperture Software
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

#include <stdbool.h>
#include <stdint.h>

#if defined(ANTI_TAMPERING_ENABLED)
#include "md5.h"

// MD5 hashes of the game data files

#define FMD5HASHES	{																					\
	{ 0x0c, 0x4f, 0xeb, 0x19, 0xfc, 0x53, 0xaf, 0xa9, 0x03, 0x83, 0x24, 0xc1, 0xad, 0xa2, 0x1c, 0xe9 }, \
	{ 0xd8, 0x23, 0x9a, 0x3e, 0x68, 0xe4, 0x6f, 0x36, 0x5f, 0xf2, 0x4d, 0xca, 0x5d, 0x12, 0xfb, 0x52 }, \
	{ 0x15, 0xdc, 0x6b, 0xa1, 0x39, 0x2c, 0x9a, 0x31, 0x66, 0x1a, 0xd3, 0x78, 0xee, 0x98, 0x11, 0x62 }, \
	{ 0x24, 0x15, 0x8a, 0xe9, 0x52, 0x7d, 0x92, 0x15, 0xab, 0x4e, 0x00, 0x00, 0x32, 0x1c, 0x53, 0x75 }, \
	{ 0x8c, 0x3f, 0x01, 0xde, 0x56, 0xf9, 0x9d, 0x1c, 0x3c, 0x09, 0x05, 0x84, 0x8e, 0x96, 0x66, 0xa8 }, \
	{ 0xb7, 0x8d, 0xbf, 0x3c, 0xdd, 0xa7, 0xfc, 0x92, 0x9a, 0x55, 0x56, 0xd2, 0x4f, 0x8f, 0x82, 0xb3 }, \
	{ 0x10, 0xd9, 0x97, 0xad, 0x03, 0x5a, 0x4c, 0xde, 0x46, 0x5a, 0x82, 0xd9, 0x99, 0x46, 0xbe, 0x81 }, \
	{ 0x0a, 0x57, 0x16, 0x00, 0x7c, 0x53, 0x2f, 0x59, 0xf4, 0x1f, 0x1c, 0xd9, 0xf3, 0x5b, 0x79, 0xd1 }, \
	{ 0x5c, 0xd4, 0xa6, 0x75, 0x8b, 0xe9, 0xf9, 0xc2, 0xff, 0xee, 0xa6, 0x72, 0xbc, 0xd6, 0x05, 0x61 }, \
	{ 0x35, 0x22, 0x3d, 0x00, 0x68, 0x2f, 0x2d, 0x3a, 0x8f, 0x8a, 0x77, 0xa7, 0xa1, 0xa9, 0x71, 0x06 }, \
	{ 0xcb, 0xe0, 0x09, 0xbe, 0x17, 0x15, 0xae, 0x03, 0xbf, 0xd6, 0x03, 0x91, 0x7f, 0x78, 0xe5, 0x67 }	}


// This inline performs an MD5 check on file i
extern void md5(unsigned char *input, int ilen, unsigned char output[16]);
extern const uint8_t  fmd5hash[NB_FILES][16];
static __inline bool integrity_check(uint16_t i)
{
	int j;
	uint8_t md5hash[16];
	md5(fbuffer[i]+((i==LOADER)?LOADER_PADDING:0), fsize[i], md5hash);
	for (j=0; j<16; j++)
		if (md5hash[j] != fmd5hash[i][j])
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
