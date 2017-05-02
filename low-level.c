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
 *  low-level.c: Helper functions for byte/bit/char manipulation & compression
 *  Aligned malloc code from Satya Kiran Popuri:
 *    http://www.cs.uic.edu/~spopuri/amalloc.html
 *  PowerPacker decrunch code from 'amigadepacker' by Heikki Orsila:
 *    http://zakalwe.fi/~shd/foss/amigadepacker/
 *  ---------------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(PSP)
#include <psptypes.h>
#include <psp/psp-printf.h>
#endif
#include <stdint.h>

#include "low-level.h"

// dammit %b should have been made a C standard by now!
// <sigh> converts a 32 bit number to binary string then...
const char *to_binary(uint32_t x)
{
    static char b[33];
    uint8_t i;
    uint32_t m;
    for (i=0,m=0x80000000; m!=0; i++,m>>=1)
        b[i] = (x&m)?'1':'0';
    b[i] = 0;
    return b;
}


// Power-of-two... err... ize!
// We need this to change a dimension to the closest greater power of two
// as pspgl can only deal with power of two dimensionned textures
uint16_t powerize(uint16_t n)
{
    uint16_t retval;
    int i, first_one, last_one;

    retval = n;	// left unchanged if already power of two
                // also works if n == 0
    first_one = -1;
    last_one = -1;

    for (i=0; i<16; i++)
    {
        if (n & 0x0001)
        {
            if (first_one == -1)
                first_one = i;
            last_one = i;
        }
        n >>= 1;
    }
    if (first_one != last_one)
        retval = 1<<(last_one+1);

    return retval;
}

/* Returns a piece of memory aligned to the given
 * alignment parameter. Alignment must be a power of
 * 2.
 * This function returns memory of length 'bytes' or more
 */
void *aligned_malloc(size_t bytes, size_t alignment)
{
    size_t size;
    size_t delta;
    void *malloc_ptr;
    void *new_ptr;
    void *aligned_ptr;

    /* Check if alignment is a power of 2
     * as promised by the caller.
     */
    if ( alignment & (alignment-1)) /* If not a power of 2 */
        return NULL;

    /* Determine how much more to allocate
     * to make room for the alignment:
     *
     * We need (alignment - 1) extra locations
     * in the worst case - i.e., malloc returns an
     * address off by 1 byte from an aligned
     * address.
     */
    size = bytes + alignment - 1;

    /* Additional storage space for storing a delta. */
    size += sizeof(size_t);

    /* Allocate memory using malloc() */
    malloc_ptr = calloc(size, 1);

    if (NULL == malloc_ptr)
        return NULL;

    /* Move pointer to account for storage of delta */
    new_ptr = (void *) ((char *)malloc_ptr + sizeof(size_t));

    /* Make ptr a multiple of alignment,
     * using the standard trick. This is
     * used everywhere in the Linux kernel
     * for example.
     */
    aligned_ptr = (void *) (((size_t)new_ptr + alignment - 1) & ~(alignment -1));

    delta = (size_t)aligned_ptr - (size_t)malloc_ptr;

    /* write the delta just before the place we return to user */
    *((size_t *)aligned_ptr - 1) = delta;

    return aligned_ptr;
}


/* Frees a chunk of memory returned by aligned_malloc() */
void aligned_free(void *ptr)
{
    size_t delta;
    void *malloc_ptr;

    if (NULL == ptr)
        return;

    /* Retrieve delta */
    delta = *( (size_t *)ptr - 1);

    /* Calculate the original ptr returned by malloc() */
    malloc_ptr = (void *) ( (size_t)ptr - delta);

    free(malloc_ptr);
}

