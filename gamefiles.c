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
*  gamefiles.c: Game files open/read
*  ---------------------------------------------------------------------------
*/

#include "gamefiles.h"

#include "traces.h"
#include "colditz.h"
#include "low-level.h"
#include "pack.h"
#include "anti-tampering.h"
#include "sfx.h"

uint8_t*   fbuffer[NB_FILES];
char*      fname[NB_FILES] = FNAMES;			// file name(s)
uint32_t   fsize[NB_FILES] = FSIZES;

#if defined(ANTI_TAMPERING_ENABLED)

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

const uint8_t fmd5hash[NB_FILES][16] = FMD5HASHES;
#endif

void depack_loadtune();


void gamefiles_init()
{
	uint16_t i;
	for (i = 0; i < NB_FILES; i++)
	{
		fbuffer[i] = NULL;
	}
}



// Initial file loader
bool load_all_files()
{
	bool result = true;
	size_t read;
	uint16_t i;
	int compressed_loader = 0;

	// We need a little padding of the loader to keep the offsets happy
	fsize[LOADER] += LOADER_PADDING;

	for (i = 0; i<NB_FILES; i++)
	{
		if ((fbuffer[i] = (uint8_t*)aligned_malloc(fsize[i], 16)) == NULL)
		{
			printf("Could not allocate buffers\n");
			result = false;
			return result;
		}

		if (i == LOADER)
		{
			fbuffer[LOADER] += LOADER_PADDING;
			fsize[LOADER] -= LOADER_PADDING;
		}

		if ((fd = fopen(fname[i], "rb")) == NULL)
		{
			printf("Couldn't find file '%s'\n", fname[i]);

			/* Take care of the compressed loader if present */
			if (i == LOADER)
			{
				// Uncompressed loader was not found
				// Maybe there's a compressed one?
				printf("  Trying to use compressed loader '%s' instead\n", ALT_LOADER);
				if ((fd = fopen(ALT_LOADER, "rb")) == NULL)
				{
					printf("  '%s' not found - Aborting.\n", ALT_LOADER);
					result = false;
					return result;
				}
				// OK, file was found - let's allocated the compressed data buffer
				if ((mbuffer = (uint8_t*)aligned_malloc(ALT_LOADER_SIZE, 16)) == NULL)
				{
					printf("  Could not allocate source buffer for loader decompression\n");
					result = false;
					return result;
				}

				read = fread(mbuffer, 1, ALT_LOADER_SIZE, fd);
				if ((read != ALT_LOADER_SIZE) && (read != ALT_LOADER_SIZE2))
				{
					printf("  '%s': Unexpected file size or read error\n", ALT_LOADER);
					result = false;
					return result;
				}
				compressed_loader = 1;

				printf("  Uncompressing...");
				{
					uint8_t* buffer = fbuffer[LOADER];
					uint32_t size = fsize[LOADER];
					if (uncompress(buffer, size))
					{
						printf("  Error!\n");
						result = false;
						return result;
					}
				}

				if (read == ALT_LOADER_SIZE2)   // SKR_COLD NTSC FIX, with one byte diff
					writebyte(fbuffer[LOADER], 0x1b36, 0x67);

				printf("  OK.\n  Now saving file as '%s'\n", fname[LOADER]);
				if ((fd = fopen(fname[LOADER], "wb")) == NULL)
				{
					printf("  Can't create file '%s'\n", fname[LOADER]);
					result = false;
					return result;
				}

				// Write file
				read = fwrite(fbuffer[LOADER], 1, fsize[LOADER], fd);
				if (read != fsize[LOADER])
				{
					printf("  '%s': Unexpected file size or write error\n", fname[LOADER]);
					result = false;
					return result;
				}
				printf("  DONE.\n\n");
			}
			else
			{
				result = false;
				return result;
			}
		}

		// Read file (except in the case of a compressed loader)
		if (!((i == LOADER) && (compressed_loader)))
		{
			printv("Reading file '%s'...\n", fname[i]);
			read = fread(fbuffer[i], 1, fsize[i], fd);
			if (read != fsize[i])
			{
				printf("'%s': Unexpected file size or read error\n", fname[i]);
				result = false;
				return result;
			}
		}

		fclose(fd);
		fd = NULL;
	}

	// OK, now we can reset our LOADER's start address
	fbuffer[LOADER] -= LOADER_PADDING;
	return result;
}

bool loadGameFiles()
{
	bool result = true;
	uint16_t i = 0;
	// Load the data. If it's the first time the game is ran, we might have
	// to uncompress LOADTUNE.MUS (PowerPack) and SKR_COLD (custom compression)
	if (load_all_files())
	{
#if defined(ANTI_TAMPERING_ENABLED)
		for (i = 0; i < NB_FILES; i++)
		{
			if ( !integrity_check(fbuffer[i] + ((i == LOADER) ? LOADER_PADDING : 0), fsize[i], fmd5hash[i] ) )
			{
				perr("Integrity check failure on file '%s'\n", fname[i]);
				result = false;
			}
		}
#endif
		if (result)
		{
			depack_loadtune();
		}
		// Some of the files need patching (this was done too in the original game!)
		if (result)
		{
			fix_files(false);
		}
	}
	else
	{
		result = false; //critical error
	}
	return result;
}

// Uncompress the PowerPacked LOADTUNE.MUS if needed
void depack_loadtune()
{
	size_t read;
	uint32_t length;
	uint8_t *ppbuffer, *buffer;

	// Don't bother if we already have an uncompressed LOADTUNE
	if ((fd = fopen(mod_name[MOD_LOADTUNE], "rb")) != NULL)
	{
		fclose(fd);
		return;
	}

	// No uncompressed LOADTUNE? Look for the PowerPacked one
	printf("Couldn't find file '%s'\n  Trying to use PowerPacked '%s' instead\n", mod_name[0], PP_LOADTUNE_NAME);

	if ((fd = fopen(PP_LOADTUNE_NAME, "rb")) == NULL)
	{
		printf("  Can't find '%s' - Aborting.\n", PP_LOADTUNE_NAME);
		return;
	}

	if ((ppbuffer = (uint8_t*)calloc(PP_LOADTUNE_SIZE, 1)) == NULL)
	{
		printf("  Could not allocate source buffer for ppunpack\n");
		fclose(fd);
		return;
	}

	// So far so good
	read = fread(ppbuffer, 1, PP_LOADTUNE_SIZE, fd);
	fclose(fd);

	// Is it the file we are looking for?
	if (read != PP_LOADTUNE_SIZE)
	{
		printf("  '%s': Unexpected file size or read error\n", PP_LOADTUNE_NAME);
		free(ppbuffer); return;
	}

	if (ppbuffer[0] != 'P' || ppbuffer[1] != 'P' ||
		ppbuffer[2] != '2' || ppbuffer[3] != '0')
	{
		printf("  '%s': Not a PowerPacked file\n", PP_LOADTUNE_NAME);
		free(ppbuffer); return;
	}

	// The uncompressed length is given at the end of the file
	length = read24(ppbuffer, PP_LOADTUNE_SIZE - 4);

	if ((buffer = (uint8_t*)calloc(length, 1)) == NULL)
	{
		printf("  Could not allocate destination buffer for ppunpack\n");
		free(ppbuffer); return;
	}

	printf("  Uncompressing...");
	// Call the PowerPacker unpack subroutine
	ppDecrunch(&ppbuffer[8], buffer, &ppbuffer[4], PP_LOADTUNE_SIZE - 12, length, ppbuffer[PP_LOADTUNE_SIZE - 1]);
	free(ppbuffer);

	// We'll play MOD directly from files, so write it
	printf("  OK.\n  Now saving file as '%s'\n", mod_name[0]);
	if ((fd = fopen(mod_name[0], "wb")) == NULL)
	{
		printf("  Couldn't create file '%s'\n", mod_name[0]);
		free(buffer); return;
	}

	read = fwrite(buffer, 1, length, fd);
	if (read != length)
		printf("  '%s': write error.\n", mod_name[0]);

	printf("  DONE.\n\n");

	fclose(fd);
	free(buffer);
}

// Looks like the original programmer found that some of the data files had issues,
// but rather than fixing the files, they patched them in the loader... go figure!
void fix_files(bool reload)
{
	uint8_t i;
	uint32_t mask;

	//
	// Original game patch
	//
	//////////////////////////////////////////////////////////////////
	//00001C10                 move.b  #$30,(fixed_crm_vector).l ; '0'
	writebyte(fbuffer[ROOMS], FIXED_CRM_VECTOR, 0x30);
	//00001C18                 move.w  #$73,(exits_base).l ; 's' ; fix room #0's exits
	writeword(fbuffer[ROOMS], ROOMS_EXITS_BASE, 0x0073);
	//00001C20                 move.w  #1,(exits_base+2).l
	writeword(fbuffer[ROOMS], ROOMS_EXITS_BASE + 2, 0x0001);
	//00001C28                 move.w  #$114,(r116_exits+$E).l ; fix room #116's last exit (0 -> $114)
	writeword(fbuffer[ROOMS], ROOMS_EXITS_BASE + (0x116 << 4) + 0xE, 0x0114);

	if (reload)
		// On a reload, no need to fix the other files again
		return;

	// DEBUG: I've always wanted to have the stethoscope!
	// (replaces the stone in the courtyard)
	//	writeword(fbuffer[OBJECTS],32,0x000E);

	// OK, because we're not exactly following the exact exit detection routines from the original game
	// (we took some shortcuts to make things more optimized) upper stairs landings are a major pain in
	// the ass to handle, so we might as well take this opportunity to do a little patching of our own...
	for (i = 9; i<16; i++)
	{	// Offset 0x280 is the intermediate right stairs landing
		mask = readlong((uint8_t*)fbuffer[LOADER], EXIT_MASKS_START + 0x280 + 4 * i);
		// eliminating the lower right section of the exit mask seems to do the job
		mask &= 0xFFFF8000;
		writelong((uint8_t*)fbuffer[LOADER], EXIT_MASKS_START + 0x280 + 4 * i, mask);
	}

	// The 3rd tunnel removable masks for overlays, on the compressed map, were not designed
	// for widescreen, and as such the tunnel will show open if we don't patch it.
	writebyte(fbuffer[LOADER], OUTSIDE_OVL_BASE + 0x38 + 1, 0x05);

	// In the original, they reset the tile index for tunnels, instead of starting at
	// 0x1E0 (TUNNEL_TILE_ADDON) as they should have done. Because we use the latter
	// more logical approach for efficiency, we'll patch a couple of words
	writeword(fbuffer[LOADER], TUNNEL_EXIT_TILES_LIST + 2 * IN_TUNNEL_EXITS_START,
		readword(fbuffer[LOADER], TUNNEL_EXIT_TILES_LIST + 2 * IN_TUNNEL_EXITS_START) + TUNNEL_TILE_ADDON);
	writeword(fbuffer[LOADER], TUNNEL_EXIT_TILES_LIST + 2 * IN_TUNNEL_EXITS_START + 2,
		readword(fbuffer[LOADER], TUNNEL_EXIT_TILES_LIST + 2 * IN_TUNNEL_EXITS_START + 2) + TUNNEL_TILE_ADDON);

	// Don't want to be picky, but when you include an IFF Sample to be played as an SFX, you might want
	// to make sure that either you remove the IFF header, or you start playing the actual BODY. The SFX
	// for the door SFX was screwed up in the original game because of the 0x68 bytes header. We fix it:
	writeword(fbuffer[LOADER], SFX_TABLE_START, readword(fbuffer[LOADER], SFX_TABLE_START) + 0x68);
	writeword(fbuffer[LOADER], SFX_TABLE_START + 6, readword(fbuffer[LOADER], SFX_TABLE_START + 6) - 0x68);
}

// Reload the files for a game restart
bool reload_files()
{
	bool result = true;
	
	uint32_t i;
	for (i = 0; i<NB_FILES_TO_RELOAD; i++)
	{
		size_t read;
		if ((fd = fopen(fname[i], "rb")) == NULL)
		{
			perrv("fopen()");
			printf("Can't find file '%s'\n", fname[i]);
		}
		// Read file
		printv("Reloading file '%s'...\n", fname[i]);
		read = fread(fbuffer[i], 1, fsize[i], fd);
		if (read != fsize[i])
		{
			perrv("fread()");
			printf("'%s': Unexpected file size or read error\n", fname[i]);
			result = false;
			return result;
		}

		fclose(fd);
		fd = NULL;
	}
	return result;
}
