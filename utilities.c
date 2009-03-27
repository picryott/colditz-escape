/**
 **  Escape from Colditz
 **
 **  Utility functions
 **
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if !defined(PSP)
#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include <gl/glut.h>
#else
#include <stdarg.h>
#include <pspkernel.h>
#include <pspdebug.h>
#include <pspgu.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif

#include "colditz.h"
#include "utilities.h"
#include "low-level.h"

// Whatever you do, you don't want local textures variables
GLuint* cell_texid;
GLuint* sprite_texid;
GLuint panel_texid;
GLuint render_texid;

u8  obs_to_sprite[NB_OBS_TO_SPRITE];
u8	removable_tile_set[CMP_MAP_WIDTH][CMP_MAP_HEIGHT];


void load_all_files()
{
	size_t read;
	u32 i;
	int compressed_loader = 0;

	for (i=0; i<NB_FILES; i++)
	{
		if ( (fbuffer[i] = (u8*) aligned_malloc(fsize[i], 16)) == NULL)
		{
			perr("Could not allocate buffers\n");
			ERR_EXIT;
		}

		if ((fd = fopen (fname[i], "rb")) == NULL)
		{
			if (opt_verbose)
				perror ("fopen()");
			perr("Can't find file '%s'\n", fname[i]);

			/* Take care of the compressed loader if present */
			if (i == LOADER)
			{
				// Uncompressed loader was not found
				// Maybe there's a compressed one?
				perr("  Trying to use compressed loader '%s' instead\n",ALT_LOADER);
				if ((fd = fopen (ALT_LOADER, "rb")) == NULL)
				{
					print("  '%s' not found.\n", ALT_LOADER);
					ERR_EXIT;
				}
				// OK, file was found - let's allocated the compressed data buffer
				if ((mbuffer = (u8*) aligned_malloc(ALT_LOADER_SIZE, 16)) == NULL)
				{
					perr("Could not allocate source buffer for uncompress\n");
					ERR_EXIT;
				}
				if (opt_verbose)
					print("Reading file '%s'...\n", ALT_LOADER);
				read = fread (mbuffer, 1, ALT_LOADER_SIZE, fd);
				if (read != ALT_LOADER_SIZE)
				{
					if (opt_verbose)
						perror ("fread()");
					perr("'%s': Unexpected file size or read error\n", ALT_LOADER);
					ERR_EXIT;
				}
				compressed_loader = 1;

				perr("  Uncompressing...\n");
				if (uncompress(fsize[LOADER]))
				{
					perr("Decompression error\n");
					ERR_EXIT;
				}
				perr("  OK. Now saving file as '%s'\n",fname[LOADER]);
				if ((fd = fopen (fname[LOADER], "wb")) == NULL)
				{
					if (opt_verbose)
						perror ("fopen()");
					perr("Can't create file '%s'\n", fname[LOADER]);
					ERR_EXIT;
				}
				
				// Write file
				if (opt_verbose)
						print("Writing file '%s'...\n", fname[LOADER]);
				read = fwrite (fbuffer[LOADER], 1, fsize[LOADER], fd);
				if (read != fsize[LOADER])
				{
					if (opt_verbose)
						perror ("fwrite()");
					perr("'%s': Unexpected file size or write error\n", fname[LOADER]);
					ERR_EXIT;
				}				
			}
			else 
				ERR_EXIT;
		}
	
		// Read file (except in the case of a compressed loader)
		if (!((i == LOADER) && (compressed_loader)))
		{
			if (opt_verbose)
				print("Reading file '%s'...\n", fname[i]);
			read = fread (fbuffer[i], 1, fsize[i], fd);
			if (read != fsize[i])
			{
				if (opt_verbose)
					perror ("fread()");
				perr("'%s': Unexpected file size or read error\n", fname[i]);
				ERR_EXIT;
			}
		}

		fclose (fd);
		fd = NULL;
	}
}


// Get some properties (max/min/...) according to file data
void get_properties()
{
	u16 room_index;
	u32 ignore = 0;
	u32 offset;
	u8  i,j;

	// Get the number of rooms
	for (room_index=0; ;room_index++)
	{	
		// Read the offset
		offset = readlong((u8*)fbuffer[ROOMS], OFFSETS_START+4*room_index);
		if (offset == 0xFFFFFFFF)
		{	// For some reason there is a break in the middle
			ignore++;
			if (ignore > FFs_TO_IGNORE)
				break;
		}
	}
	nb_rooms = room_index;
	print("nb_rooms = %X\n", nb_rooms);

	// A backdrop cell is exactly 256 bytes (32*16*4bits)
	nb_cells = fsize[CELLS] / 0x100;
	cell_texid = malloc(sizeof(GLuint) * nb_cells);
	GLCHK(glGenTextures(nb_cells, cell_texid));
	print("nb_cells = %X\n", nb_cells);

	nb_sprites = readword(fbuffer[SPRITES],0) + 1;
	sprite_texid = malloc(sizeof(GLuint) * nb_sprites);
	GLCHK(glGenTextures(nb_sprites, sprite_texid));
	print("nb_sprites = %X\n", nb_sprites);

	nb_objects = readword(fbuffer[OBJECTS],0) + 1;
	print("nb_objects = %X\n", nb_objects);
	for (i=0; i<NB_OBS_TO_SPRITE; i++)
		obs_to_sprite[i] = readbyte(fbuffer[LOADER],OBS_TO_SPRITE_START+i);

	// This will be needed to hide the pickable objects on the outside map
	// if the removable walls are set
	for (i=0; i<CMP_MAP_WIDTH; i++)
		for (j=0; j<CMP_MAP_HEIGHT; j++)
			removable_tile_set[i][j] = 0;

	// Set our textures for panel and zoom
	glGenTextures( 1, &panel_texid );
	glGenTextures( 1, &render_texid );
	glBindTexture(GL_TEXTURE_2D, panel_texid);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 512, 32, 0, GL_RGB, GL_UNSIGNED_BYTE, fbuffer[PANEL]);
}


// Convert an Amiga 12 bit RGB colour palette to 16 bit GRAB
void to_16bit_palette(u8 palette_index)
{
	u32 i;
	u16 rgb, grab;

	int palette_start = palette_index * 0x20;

	// Read the palette
	if (opt_verbose)
		print("Using Amiga Palette index: %d\n", palette_index);


	for (i=0; i<16; i++)		// 16 colours
	{
		rgb = readword(fbuffer[PALETTES], palette_start + 2*i);
		if (opt_verbose)
		{
			print(" %03X", rgb); 
			if (i==7)
				print("\n");
		}
		// OK, we need to convert our rgb to grab
		// 1) Leave the R&B values as they are
		grab = rgb & 0x0F0F;
		// 2) Set Alpha to no transparency
		grab |= 0x00F0;
		// 3) Set Green
		grab |= (rgb << 8) & 0xF000;
		// 4) Write in the palette
		aPalette[i] = grab;
	}
	if (opt_verbose)
		print("\n\n");
}


// Convert a 4 bit line-interleaved source to 16 bit RGBA (GRAB) destination
void line_interleaved_to_wGRAB(u8* source, u8* dest, u16 w, u16 h)
{
	u8 colour_index;
	u32 i,j,l,pos;
	int k;
	u32 wb;
	u8 line_byte[4];

	// the width of interest to us is the one in bytes.
	wb = w/8;

	// We'll write sequentially to the destination
	pos = 0;
	for (i=0; i<h; i++)
	{	// h lines to process
		for (j=0; j<wb; j++)
		{	// wb bytes per line
			for (k=0; k<4; k++)
				// Read one byte from each of the 4 lines (starting from max y for openGL)
				line_byte[3-k] = readbyte(source, 4*(wb*i) + k*wb + j);
			// Write 8 RGBA values
			for (k=0; k<8; k++)
			{
				colour_index = 0;
				// Get the palette colour index and rotate the line bytes
				for (l=0; l<4; l++)
				{
					colour_index <<= 1;
					colour_index |= (line_byte[l]&0x80)?1:0;
					line_byte[l] <<= 1;
				}
				// Alpha is always set to 0
				writeword(dest, pos, aPalette[colour_index]);
				pos += 2;
			}
		}
	}
}


// Convert a 1+4 bits (mask+colour) bitplane source
// to 16 bit RGBA (GRAB) destination
void bitplane_to_wGRAB(u8* source, u8* dest, u16 w, u16 ext_w, u16 h)
{
	u16 bitplane_size;
	u8  colour_index;
	u16 i,j,k,wb,ext_wb;
	u8  bitplane_byte[5], mask_byte;
	u32 pos = 0;

	wb = w/8;	// width in bytes
	ext_wb = ext_w/8;
	bitplane_size = h*wb; 

	for (i=0; i<bitplane_size; i++)	
	{
		// Read one byte from each bitplane...
		for (j=0; j<5; j++)
			// bitplanes are in reverse order for colour
			// and so is openGL's coordinate system for y
			bitplane_byte[4-j] = readbyte(source, i + (j*bitplane_size) );

		// For clarity
		mask_byte = bitplane_byte[4];

		// Write 8 RGBA words 
		for (k=0; k<8; k++)
		{

			colour_index = 0;
			// Get the palette colour index and rotate the bitplane bytes
			for (j=0; j<4; j++)
			{
				colour_index <<= 1;
				colour_index |= (bitplane_byte[j]&0x80)?1:0;
				bitplane_byte[j] <<= 1;
			}
			// Alpha is in 3rd position, and needs to be cleared on empty mask
			writeword(dest, pos, aPalette[colour_index] & ((mask_byte&0x80)?0xFFFF:0xFF0F));
			pos += 2;
			// Takes care of padding in width
			while ((u16)(pos%(2*ext_w))>=(2*w))
				pos +=2;	// calloced to zero, so just skim
			mask_byte <<=1;
		}
	}
}


// Converts the room cells to RGB data we can handle
void cells_to_wGRAB(u8* source, u8* dest)
{
	u32 i;

	// Convert each 32x16x4bit (=256 bytes) cell to RGB
	for (i=0; i<nb_cells; i++)
	{
		line_interleaved_to_wGRAB(source + (256*i), dest+(2*RGBA_SIZE*256*i), 32, 16);
		GLCHK(glBindTexture(GL_TEXTURE_2D, cell_texid[i]));
		GLCHK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 32, 16, 0, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4_REV, 
			((u8*)rgbCells) + i*2*RGBA_SIZE*0x100));
	}

}


// Initialize the sprite array
void init_sprites()
{
	u32 index = 2;	// We need to ignore the first word (nb of sprites)
	u16 sprite_index = 0;
	u16 sprite_w;	// width, in words
	u32 sprite_address;

	// Allocate the sprites and overlay arrays
	sprite = aligned_malloc(nb_sprites * sizeof(s_sprite), 16);
	overlay = aligned_malloc(MAX_OVERLAY * sizeof(s_overlay), 16);

	// First thing we do is populate the sprite offsets at the beginning of the table
	sprite_address = index + 4* (readword(fbuffer[SPRITES],0) + 1);
	for (sprite_index=0; sprite_index<nb_sprites; sprite_index++)
	{
		sprite_address += readlong(fbuffer[SPRITES],index);
		writelong(fbuffer[SPRITES],index,sprite_address);
		index+=4;
	}
	// Each sprite is prefixed by 2 words (x size in words, y size in pixels)
	// and one longword (size of one bitplane, in bytes)
	// NB: MSb on x size will be set if sprite is animated
	for (sprite_index=0; sprite_index<nb_sprites; sprite_index++)
	{
		sprite_address = readlong(fbuffer[SPRITES],2+4*sprite_index);
//		print("sprite[%X] address = %08X\n", sprite_index, sprite_address);
		// x size is given in words
		sprite_w = readword(fbuffer[SPRITES],sprite_address);
		// w is fine as it's either 2^4 or 2^5
		sprite[sprite_index].w = 16*(sprite_w & 0x7FFF);
		sprite[sprite_index].corrected_w = powerize(sprite[sprite_index].w);
		// h will be problematic as pspgl wants a power of 2
		sprite[sprite_index].h = readword(fbuffer[SPRITES],sprite_address+2);
		sprite[sprite_index].corrected_h = powerize(sprite[sprite_index].h);
		
		// According to MSb of sprite_w (=no_mask), we'll need to use RGBA or RGB
//		sprite[sprite_index].type = (sprite_w & 0x8000)?GL_RGB:GL_RGBA;
		// There's an offset to position the sprite depending on the mask's presence
		sprite[sprite_index].x_offset = (sprite_w & 0x8000)?16:1;
		sprite[sprite_index].data = aligned_malloc( RGBA_SIZE * 
			sprite[sprite_index].corrected_w * sprite[sprite_index].corrected_h, 16);
//		print("  w,h = %0X, %0X\n", sprite[sprite_index].w , sprite[sprite_index].h);
	}
}


// Converts the sprites to 16 bit GRAB data we can handle
void sprites_to_wGRAB()
{
	u16 sprite_index;
	u16 bitplane_size;
	u32 sprite_address;
	u8* sbuffer;
	u16 w,h;
	int no_mask = 0;

	for (sprite_index=0; sprite_index<nb_sprites; sprite_index++)
	{
		// Get the base in the original Colditz sprite file
		sprite_address = readlong(fbuffer[SPRITES],2+4*sprite_index);

		// if MSb is set, we have 4 bitplanes instead of 5
		w = readword(fbuffer[SPRITES],sprite_address);
		no_mask = w & 0x8000;
		w *= 2;		// width in bytes
		h = sprite[sprite_index].h;

		bitplane_size = readword(fbuffer[SPRITES],sprite_address+6);
		if (bitplane_size != w*h)
			print("sprites_to_wGRAB: Integrity check failure on bitplane_size\n");

		// Source address
		sbuffer = fbuffer[SPRITES] + sprite_address + 8; 

		if (no_mask)
			// Bitplanes that have no mask are line-interleaved, like cells
			line_interleaved_to_wGRAB(sbuffer, sprite[sprite_index].data, sprite[sprite_index].w, h);
		else
			bitplane_to_wGRAB(sbuffer, sprite[sprite_index].data, sprite[sprite_index].w,
				sprite[sprite_index].corrected_w, h);

		// Now that we have data in a GL readable format, let's texturize it!
		GLCHK(glBindTexture(GL_TEXTURE_2D, sprite_texid[sprite_index]));
		GLCHK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sprite[sprite_index].corrected_w, 
			sprite[sprite_index].corrected_h, 0, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4_REV,
			sprite[sprite_index].data));

	}
}


// Populates the overlay table
void set_overlays(int x, int y, u32 current_tile, u16 room_x)
{
	u16 tile1_data, tile2_data;
	u16 i;
	short sx, sy;
	u16 sid;	// sprite index

	// read current tile
	tile1_data = readword(fbuffer[ROOMS], current_tile) & 0xFF80;
	for (i=0; i<(12*NB_SPECIAL_TILES); i+=12)
	{
		if (readword(fbuffer[LOADER], SPECIAL_TILES_START+i) != tile1_data)
			continue;
		sx = readword(fbuffer[LOADER], SPECIAL_TILES_START+i+8);
		if (opt_debug)
			print("  match: %04X, direction: %04X\n", tile1_data, sx);
		if (i >= (12*(NB_SPECIAL_TILES-4)))
		// The four last special tiles are exits. We need to check is they are open
		{
			// Get the exit data (same tile if tunnel, 2 rows down in door)
			tile2_data = readword(fbuffer[ROOMS], current_tile + 
				(i==(12*(NB_SPECIAL_TILES-1)))?0:(4*room_x));
//			print("got exit: %04X\n", tile2_data);
			// Validity check
			if (!(tile2_data & 0x000F))
				print("set_overlays: Integrity check failure on exit tile\n");
			// if the tile is an exit and the exit is open
			if (tile2_data & 0x0010)
			{
				if (opt_debug)
					print("    exit open: ignoring overlay\n");
				// The second check on exits is always an FA00, thus we can safely
				break;
			}
		}
			 
		if (sx < 0)
			tile2_data = readword(fbuffer[ROOMS], current_tile-2) & 0xFF80;
		else
			tile2_data = readword(fbuffer[ROOMS], current_tile+2) & 0xFF80;
		// ignore if special tile that follows is matched
		if (readword(fbuffer[LOADER], SPECIAL_TILES_START+i+2) == tile2_data)
		{
			if (opt_debug)
				print("    ignored as %04X matches\n", tile2_data);
			continue;
		}
		sid = readword(fbuffer[LOADER], SPECIAL_TILES_START+i+4);
		overlay[overlay_index].sid = sid;
		if (opt_debug)
			print("    overlay as %04X != %04X => %X\n", tile2_data, 
				readword(fbuffer[LOADER], SPECIAL_TILES_START+i+2), sid);
		sy = readword(fbuffer[LOADER], SPECIAL_TILES_START+i+6);
		if (opt_debug)
			print("    sx: %04X, sy: %04X\n", sx, sy);
		overlay[overlay_index].x = x + (int)sx - (int)sprite[sid].w + (int)(sprite[sid].x_offset);
		overlay[overlay_index].y = y + (int)sy - (int)sprite[sid].h + 1;
		overlay_index++;
		// No point in looking for overlays any further if we met our match 
		// UNLESS this is a double bed overlay, in which case the same tile
		// needs to be checked for a double match (in both in +x and -x)
		if (tile1_data != 0xEF00)
			break;
	}
}


// Read the props (pickable objects) from obs.bin
void set_objects()
{
	u16 i;
	u16 x, y;

	for (i=0; i<(8*nb_objects); i+=8)
	{
		if (readword(fbuffer[OBJECTS],i+2) != current_room_index)
			continue;
		overlay[overlay_index].sid = obs_to_sprite[readword(fbuffer[OBJECTS],i+2+6)];
		x = readword(fbuffer[OBJECTS],i+2+4) - 15;
		y = readword(fbuffer[OBJECTS],i+2+2) - 3;
		overlay[overlay_index].x = gl_off_x + x;
		overlay[overlay_index].y = gl_off_y + y;

		// Because of the removable walls we have a special case for the CMP_MAP
		if ((current_room_index == ROOM_OUTSIDE) && (removable_tile_set[x/32][y/16]))
				overlay_index--;

		if (opt_debug)
			print("  pickup object match: sid=%X\n", overlay[overlay_index].sid);
		overlay_index++;
	}
	
}


void display_sprite(float x, float y, float w, float h, GLuint texid) 
{
	float x1, x2, y1, y2;

	x1 = origin_x + x;
	x2 = origin_x + x + w;
	y1 = origin_y + y;
	y2 = origin_y + y + h;

	glBindTexture(GL_TEXTURE_2D, texid);

	// Don't modify pixel colour ever
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	// Do linear interpolation. Looks better, but if you zoom, you have to zoom
	// the whole colour buffer, else the sprite seams will show
//	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// If we don't set clamp, our tiling will show
#if defined(PSP)
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
#else
	// For some reason GL_CLAMP_TO_EDGE on Win achieves the same as GL_CLAMP on PSP
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#endif 

	// pspGL does not implement QUADS
	glBegin(GL_TRIANGLE_FAN);

	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(x1, y1, 0.0f);

	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(x1, y2, 0.0f);

	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(x2, y2, 0.0f);

	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(x2, y1, 0.0f);

	glEnd();
}

// Display all our overlays
void display_overlays()
{
	u8 i;

	for (i=0; i<overlay_index; i++)
		display_sprite(overlay[i].x, overlay[i].y, sprite[overlay[i].sid].corrected_w, 
			sprite[overlay[i].sid].corrected_h, sprite_texid[overlay[i].sid]);
}


void removable_walls()
{	// Get the current removable walls mask to apply

	int tile_x, tile_y;
	u16 tile_data;
	u32 cmp_data;
	u32 tmp_bitmask;
	u8  bit_index;
	u32 dir_offset;
	int rdx, rdy;
	u8  cmp_x, cmp_y;

	rdx = prisoner_x - last_p_x;
	rdy = (prisoner_2y/2) - last_p_y; 
	// If no motion, exit
	if ((rdx == 0) && (rdy == 0))
		return;

	// Compute the tile on which we stand
	tile_y = prisoner_2y / 32;

	// Sanity checks
	if (tile_y < 0)
		tile_y = 0;
	if (tile_y >= CMP_MAP_HEIGHT)
		tile_y = CMP_MAP_HEIGHT-1;

	tile_x = prisoner_x / 32;
	if (tile_x < 0)
		tile_x = 0;
	if (tile_x >= CMP_MAP_WIDTH)
		tile_x = CMP_MAP_WIDTH-1;

	// Read a longword in the first part of the compressed map
	// The compressed map elements are of the form
	// OOOO OOOO OOOO OOOT TTTT TTTT IIII II DD
	// where:
	// OOO OOOO OOOO OOOO is the index for overlay tile (or 0 for tiles without cmp map overlays)
	// T TTTT TTTT        is the base tile index (tile to display with all overlays removed)
	// II IIII            is the removable_mask index to use when positionned on this tile
	//                    (REMOVABLES_MASKS_LENGTH possible values)
	// DD                 is the index for the direction subroutine to pick
	cmp_data = readlong((u8*)fbuffer[COMPRESSED_MAP], (tile_y*CMP_MAP_WIDTH+tile_x)*4);

	tile_data = (cmp_data & 0x1FF00) >> 1;
	
	bit_index = ((u8)cmp_data) & 0xFC;
	if (bit_index == 0)
		return;

	// direction "subroutine" to use (diagonal, horizontal...)
	dir_offset = ((u8)cmp_data) & 0x03;

	// read the mask with relevant removable turned on, associated to the tile
	tmp_bitmask = readlong((u8*)fbuffer[LOADER],  REMOVABLES_MASKS_START + bit_index);
	
	if (tile_data <= 0x480)
	// ignore if blank or exit
	// nb: if it is an exit, lower 5 bits are the exit number
		return;
	// TO_DO: ignore tunnels


	// direction "subroutines":
	if ((dir_offset == 0) && (rdy > 0))
	{	// moving down and having crossed the horizontal
		// boundary (set at the tile top)
		// => turn the relevant removable visible
		rem_bitmask = tmp_bitmask;
	}
	if ((dir_offset == 1) && (rdy < 0))
	{	// moving up and having crossed the horizontal
		// boundary (set at tile bottom)
		// => turn the relevant removable invisible
		tmp_bitmask &= ~(1 << (bit_index >> 2));	// don't forget to rotate!
		rem_bitmask = tmp_bitmask;
	}
	if (dir_offset == 2)
	{	// check the crossing of a bottom-left to top-right diagonal
		cmp_x = 0xF - ((prisoner_x/2) & 0xF);	// need to invert x
		cmp_y = (prisoner_2y/2) & 0xF;
		if ( ((rdx > 0) && (rdy == 0)) || (rdy > 0) )
		{	// into the bottom "quadrant"
			if (cmp_x <= cmp_y)
			{	// turn removable on
				rem_bitmask = tmp_bitmask;
			}
		}
		else
		{	// into the top "quadrant"
			if (cmp_x >= cmp_y)
			{	// turn removable off
				tmp_bitmask &= ~(1 << (bit_index >> 2));
				rem_bitmask = tmp_bitmask;
			}
		}
	}
	if (dir_offset == 3)
	{	// check the crossing of a top-left to bottom-right diagonal
		cmp_x = (prisoner_x/2) & 0xF;
		cmp_y = (prisoner_2y/2) & 0xF;
		if ( ((rdx < 0) && (rdy == 0)) || (rdy > 0) )
		{	// into the bottom "quadrant"
			if (cmp_x <= cmp_y)
			{	// turn removable on
				rem_bitmask = tmp_bitmask;
			}
		}
		else
		{	// into the top "quadrant"
			if (cmp_x >= cmp_y)
			{	// turn removable off
				tmp_bitmask &= ~(1 << (bit_index >> 2));
				rem_bitmask = tmp_bitmask;
			}
		}
	}
}

// Display room
void display_room(u16 room_index)
{
// OK, I'll spare you the suspense: this is NOT optimized like hell!
// We are redrawing ALL the tiles, for EACH FRAME!
// Yup, no scrolling or anything: just plain unoptimized brute force...
// But hey, the PSP can handle it, and so should a decent PC, so why bother?

	u32 offset;					// Offsets to each rooms are given at
								// the beginning of the Rooms Map file
	u16 room_x, room_y, tile_data;
	int min_x, max_x, min_y, max_y;
	u16 tile_tmp, nb_tiles;
	u8  bit_index;
	int tile_x, tile_y;
	int pixel_x, pixel_y;
	int u;

//	printf("prisoner (x,y) = (%d,%d)\n", prisoner_x, prisoner_2y/2);

	if (room_index != ROOM_OUTSIDE)
	{	// Standard room (inside)
		// Read the offset
		offset = readlong((u8*)fbuffer[ROOMS], OFFSETS_START+4*room_index);
		if (opt_verbose)
			print("\noffset[%03X] = %08X ", room_index, (uint)offset);
		if (offset == 0xFFFFFFFF)
		{
			// For some reason there is a gap in the middle
			if (opt_verbose)
				print("\n  IGNORED");
			return;
		}

		// Now that we have the offset, let's look at the room

		// The 2 first words are the room Y and X dimension (in tiles),
		// in that order
		room_y = readword((u8*)fbuffer[ROOMS], ROOMS_START+offset);
		offset +=2;
		room_x = readword((u8*)fbuffer[ROOMS], ROOMS_START+offset);
		offset +=2;
		if (opt_verbose)
			print("(room_x=%X,room_y=%X)\n", room_x, room_y);

		gl_off_x = PSP_SCR_WIDTH/2 - prisoner_x;
		gl_off_y = PSP_SCR_HEIGHT/2 - (prisoner_2y/2);

		// reset room overlays
		overlay_index = 0;

		// Before we do anything, let's set the pickable objects in
		// our overlay table (so that room overlays go on top of 'em)
		set_objects();

		// Read the tiles data
		for (tile_y=0; tile_y<room_y; tile_y++)
		{
			if (opt_verbose)
				print("    ");	// Start of a line
			for(tile_x=0; tile_x<room_x; tile_x++)
			{
				pixel_x = gl_off_x+tile_x*32;
				pixel_y = gl_off_y+tile_y*16;


				// A tile is 32(x)*16(y)*4(bits) = 256 bytes
				// A specific room tile is identified by a word

				/*
				 * tile_data  = tttt tttt tggo xxxx 
				 * t: tile #
				 * g: lock grade (01 = lockpick, 10 = key 2, 11 = key 1)
				 * o: door open flag
				 * x: exit lookup number (in exit map [1-8])
				*/
				tile_data = readword((u8*)fbuffer[ROOMS], ROOMS_START+offset);

				display_sprite(pixel_x,pixel_y,32,16, 
					cell_texid[(tile_data>>7) + ((room_index>0x202)?0x1E0:0)]);

				// Display sprite overlay
				set_overlays(pixel_x, pixel_y, ROOMS_START+offset, room_x);

				offset +=2;		// Read next tile

				if (opt_verbose)
					print("%04X ", tile_data);
			}
			if (opt_verbose)
				print("\n");
		}

		if (opt_debug)
			print("\n");
	}
	else
	{	// on compressed map (outside)
		room_x = CMP_MAP_WIDTH;
		room_y = CMP_MAP_HEIGHT;

		// Compute GL offsets (position of 0,0 corner of the room wrt center of the screen) 
		gl_off_x = PSP_SCR_WIDTH/2 - prisoner_x;
		gl_off_y = PSP_SCR_HEIGHT/2 - (prisoner_2y/2);

		// reset room overlays
		overlay_index = 0;

		// Before we do anything, let's set the pickable objects in
		// our overlay table (so that room overlays go on top of 'em)
		set_objects();

		// Since we're outside, take care of removable sections
		removable_walls();

		// These are the min/max tile boundary computation for PSP screen
		// according to our cropped section
		min_y = prisoner_2y/32 - 7;
		if (min_y < 0)
			min_y = 0;

		max_y = prisoner_2y/32 + 9;
		if (max_y > room_y)
			max_y = room_y;

		min_x = prisoner_x/32 - 8;
		if (min_x < 0)
			min_x = 0;

		max_x = prisoner_x/32 + 9;
		if (max_x > room_x)
			max_x = room_x;

		// Read the tiles data
		for (tile_y=min_y; tile_y<max_y; tile_y++)
		{
			if (opt_verbose)
				print("    ");	// Start of a line
			for(tile_x=min_x; tile_x<max_x; tile_x++)
			{
				pixel_x = gl_off_x+tile_x*32;
				pixel_y = gl_off_y+tile_y*16;

				/* Read a longword in the first part of the compressed map
				 * The compressed map elements are of the form
				 * OOOO OOOO OOOO OOOT TTTT TTTT IIII IIDD
				 * where:
				 * OOO OOOO OOOO OOOO is the index for overlay tile (or 0 for tiles without cmp map overlays)
				 * T TTTT TTTT is the base tile index (tile to display with all overlays removed)
				 * II IIII is the removable_mask index to use when positionned on this tile
				 *         (REMOVABLES_MASKS_LENGTH possible values)
				 * DD is the index for the direction subroutine to pick
				 *
				 * NB: in the case of an exit (T TTTT TTTT < 0x900), IIII IIDD is the exit index
				 */

				tile_data = (readlong((u8*)fbuffer[COMPRESSED_MAP], (tile_y*room_x+tile_x)*4) & 0x1FF00);
				tile_data >>= 1 ;	// get a tile offset we can actually use

				// For the time being, we'll reset the removable boolean for props
				removable_tile_set[tile_x][tile_y] = 0;

				// If the first 15 bits of this longword are zero, then we have a simple tile, 
				// with remainder 17 being the tile data 
				offset = readword((u8*)fbuffer[COMPRESSED_MAP], (tile_y*room_x+tile_x)*4) & 0xFFFE;
				// First word (with mask 0xFFFE) indicates if we have a simple tile or not

				if (offset != 0)
				// If the first 15 bits are not null, we have a complex sequence, 
				// which we must read in second part of the compressed map, 
				// the 15 bits being the offset from start of second part
				{
					// The first word read is the number of overlapping tiles
					// overlapping tiles occur when there might be a wall hiding a walkable section
					nb_tiles = readword((u8*)fbuffer[COMPRESSED_MAP], CM_TILES_START+offset);
					// The rest of the data is a tile index (FF80), a bit index (1F), and 2 bits unused.
					// the later being used to check bits of an overlay bitmap longword
					for (u=nb_tiles; u!=0; u--)
					{
						tile_tmp = readword((u8*)fbuffer[COMPRESSED_MAP], CM_TILES_START+offset + 2*u);
						bit_index = tile_tmp & 0x1F;
						if ( (1<<bit_index) & rem_bitmask )
						{
							tile_data = tile_tmp;
							// If we're using a removable tile, we'll need to hide the props beneath
							removable_tile_set[tile_x][tile_y] = 1;
							break;
						}
					}
				}

				// At last, we have a tile we can display
				display_sprite(pixel_x,pixel_y,32,16, 
					cell_texid[(tile_data>>7)]);

				if (opt_verbose)
					printf("%04X ", tile_data & 0x1FF80);
			}
			if (opt_verbose)
				print("\n");
		}

		if (opt_debug)
			print("\n");
	}

	// Let's add our guy
	overlay[overlay_index].sid = prisoner_sid;	
//	overlay[overlay_index].sid = 0xC1;	

	overlay[overlay_index].x = PSP_SCR_WIDTH/2; // - 6; 
	overlay[overlay_index++].y = PSP_SCR_HEIGHT/2 - 28; 

	// Now that the background is done, and that we have the overlays, display the overlay sprites
	display_overlays();

	// We'll need that for next run
	last_p_x = prisoner_x;
	last_p_y = prisoner_2y/2; 
}

// Display Panel
void display_panel()
{

	glColor3f(0.0f, 0.0f, 0.0f);	// Set the colour to black

	glDisable(GL_BLEND);	// Needed for black objects to show

	// Because the original game wasn't designed for widescreen
	// we have to diagonally crop the area to keep some elements hidden
	// TO_DO: add some texture, to make it look like an old photograph or something
	glBegin(GL_TRIANGLES);

	glVertex2d(0, 0);
	glVertex2d(72, 0);
	glVertex2d(0, 36);

	glVertex2d(PSP_SCR_WIDTH, 0);
	glVertex2d(PSP_SCR_WIDTH-72, 0);
	glVertex2d(PSP_SCR_WIDTH, 36);

	glVertex2d(PSP_SCR_WIDTH, PSP_SCR_HEIGHT-32);
	glVertex2d(PSP_SCR_WIDTH-72, PSP_SCR_HEIGHT-32);
	glVertex2d(PSP_SCR_WIDTH, PSP_SCR_HEIGHT-32-36);

	glVertex2d(0, PSP_SCR_HEIGHT-32);
	glVertex2d(72, PSP_SCR_HEIGHT-32);
	glVertex2d(0, PSP_SCR_HEIGHT-32-36);

	glEnd();

	// Draw our base panel
	glBegin(GL_TRIANGLE_FAN);

	glVertex2d(0, PSP_SCR_HEIGHT-32);
	glVertex2d(PSP_SCR_WIDTH, PSP_SCR_HEIGHT-32);
	glVertex2d(PSP_SCR_WIDTH, PSP_SCR_HEIGHT);
	glVertex2d(0, PSP_SCR_HEIGHT);

	glEnd();

	// Restore colour
	glColor3f(1.0f, 1.0f, 1.0f);

	// 
//	glGenTextures( 1, &panel_texid );
	glBindTexture(GL_TEXTURE_2D, panel_texid);

//	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 512, 32, 0, GL_RGB, GL_UNSIGNED_BYTE, fbuffer[PANEL]);

	// pspGL does not implement QUADS
	glBegin(GL_TRIANGLE_FAN);
#define OFF_X 79
#define PAN_W 512
#define PAN_H 32

	glTexCoord2f(0.0f, 0.0f);
	glVertex2d(OFF_X, PSP_SCR_HEIGHT-32);

	glTexCoord2f(1.0f, 0.0f);
	glVertex2d(OFF_X+PAN_W, PSP_SCR_HEIGHT-32);

	glTexCoord2f(1.0, 1.0);
	glVertex2d(OFF_X+PAN_W, PSP_SCR_HEIGHT);

	glTexCoord2f(0.0, 1.0);
	glVertex2d(OFF_X, PSP_SCR_HEIGHT);

	glEnd();

	glEnable(GL_BLEND);	// We'll need blending for the sprites, etc.

}


// Here is the long sought after "zooming the ****ing 2D colour buffer" function.
// What a �$%^&*&^ing bore!!! And all this crap still doesn't work on PSP anyway!
void rescale_buffer()
{
// using the buffer as a texture, is the ONLY WAY I COULD FIND TO GET A ZOOM
// THAT WORKS PROPERLY IN OPENGL!!! (i.e. without showing artefacts around overlay sprites)
// Seriously guys, if you're doing 2D with sprites, you'll waste DAYS trying
// to figure out a bloody solution to zoom the lousy colour buffer, because 
// if you think, with all the GPU acceleration, there should be an easy way to
// achieve that crap, you couldn't be more wrong!
	float old_x, old_y;

	if ((gl_width != PSP_SCR_WIDTH) && (gl_height != PSP_SCR_HEIGHT))
	{	
		glDisable(GL_BLEND);	// Else we'll need a glClear

		// First, we copy the whole buffer into a texture
		glBindTexture(GL_TEXTURE_2D,render_texid);

		glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, PSP_SCR_WIDTH, PSP_SCR_HEIGHT, 0);

		// Then we clear our buffer (or disable blend)
//		glClear(GL_COLOR_BUFFER_BIT);

		// Then we change our viewport to the actual screen size
		glViewport(0, 0, gl_width, gl_height);

		// Now we change the projection, to the new dimensions
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
	    glOrtho(0, gl_width, gl_height, 0, -1, 1);

		// OK, now we can display the whole texture
		// But first, we need to reset the origin
		old_x = origin_x; origin_x = 0;
		old_y = origin_y; origin_y = 0;
		display_sprite(0,gl_height,gl_width,-gl_height,render_texid);
		origin_x = old_x; origin_y = old_y;

		// Finally, we restore the parameters
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, PSP_SCR_WIDTH, PSP_SCR_HEIGHT, 0, -1, 1);
		glViewport(0, 0, PSP_SCR_WIDTH, PSP_SCR_HEIGHT);

		glEnable(GL_BLEND);	// We'll need blending for the sprites, etc.

	}
}

// Thif function returns true if the prisoner is allowed at (px,p2y)
int check_footprint(u16 room_index, int px, int p2y)
{
	u32 tile, mask, offset;
	u32 footprint = SPRITE_FOOTPRINT;
	u16 room_x, room_y;
	u16 mask_y;
	// maks offsets for upper-left, upper-right, lower-left, lower_right tiles
	u16 mask_offset[4];
	u16 exit[4];
	int tile_x, tile_y;
	u8 i;

// Reads the tile index at (x,y) from compressed map and rotate
#define comp_readtile(x,y,r)		\
	((u32)(readlong((u8*)fbuffer[COMPRESSED_MAP], ((y)*room_x+(x))*4) & 0x1FF00) >> (r))
#define room_readtile(x,y,r)		\
	((u32)(readword((u8*)(fbuffer[ROOMS]+ROOMS_START+offset),((y)*room_x+(x))*2) & 0xFF80) >> (r))
// Converts a tile index to a longword offset
#define lo_readtile(x,y)			\
	((room_index == ROOM_OUTSIDE)?comp_readtile(x,y,6):room_readtile(x,y,5))
// check if a sprite is overflowing on a mask
// the condtion below is true is a bit of s (the sprite) is set over a clear bit of m (the mask)
#define sprite_overflow(s,m)				\
	( (((~m)^s)&(~m)) != (~m) )

#define comp_readexit(x,y,r)		\
	((u32)(readlong((u8*)fbuffer[COMPRESSED_MAP], ((y)*room_x+(x))*4) & 0x1F) << (r))
#define room_readexit(x,y,r)		\
	((u32)(readword((u8*)(fbuffer[ROOMS]+ROOMS_START+offset),((y)*room_x+(x))*2) & 0x1F) << (r))
// Converts a tile index to a longword offset
#define lo_readexit(x,y)		\
	((room_index == ROOM_OUTSIDE)?comp_readexit(x,y,2):room_readexit(x,y,2))



//	printf("prisoner (x,y) = (%d,%d)\n", prisoner_x, prisoner_2y/2);
	/*
	 * To check the footprint, we need to set 4 quadrants of masks
	 * in case our rectangular footprint spans more than a single tile
	 */

//	if (TO_DO: CHECK TUNNEL)
//		sprite_footprint = TUNNEL_FOOTPRINT;

	if (room_index == ROOM_OUTSIDE)
	{	// on compressed map (outside)
		room_x = CMP_MAP_WIDTH;
		room_y = CMP_MAP_HEIGHT;
	}
	else
	{	// in a room (inside)
		offset = readlong((u8*)fbuffer[ROOMS], OFFSETS_START+4*room_index);
		if (offset == 0xFFFFFFFF)
			return 0;
		room_y = readword((u8*)fbuffer[ROOMS], ROOMS_START+offset);
		offset +=2;
		room_x = readword((u8*)fbuffer[ROOMS], ROOMS_START+offset);
		offset +=2;
//		print("(room_x=%X,room_y=%X)\n", room_x, room_y);
	}

	// Compute the tile on which we try to stand
	tile_y = p2y / 32;
	tile_x = px / 32;

	// check if we are trying to overflow our room left or up
	if ((px<0) || (p2y<0))
		return 0;

	for (i=0; i<2; i++)
	{
		// Set the left mask offset (tile_x, tile_y(+1)) index, converted to a long offset
		tile = lo_readtile(tile_x, tile_y);
//		if ((tile != 0) && (tile <
//		printf("tile = %08X\n", tile << 5);
		mask_offset[2*i] = readlong((u8*)fbuffer[LOADER], TILE_MASKS_OFFSETS+tile);

		// Set the upper right mask offset
		if ((px&0x1F) < 16)
			mask_offset[2*i+1] = mask_offset[2*i] + 2;	// Just shift 16 bits on the same tile
		else
		{	// Need to lookup the adjacent (tile_x+1, tile_y(+1)) mask
			mask_offset[2*i] += 2;	// first, we need to offset our first quadrant
			if ((tile_x+1) < room_x)
			{	// only read adjacent if it exists (i.e. < room_x)
				tile = lo_readtile(tile_x+1, tile_y);
				mask_offset[2*i+1] = readlong((u8*)fbuffer[LOADER], TILE_MASKS_OFFSETS+tile);
			}
			else	// offset 0 is mask_empty
				mask_offset[2*i+1] = 0;
		}
		tile_y++;	// process lower tiles
	}

	// OK, now we have our 4 mask offsets
	mask_y = (p2y & 0x1E)<<1;	// one mask line is 4 bytes (and p2y is already 2*py)
	mask_offset[0] += mask_y;	// start at the right line
	mask_offset[1] += mask_y;
	footprint >>= (px & 0x0F);	// rotate our footprint according to our x pos
//		printf("foot = %08x [%d]\n", footprint, (px&0x1f));
	for (i=0; i<FOOTPRINT_HEIGHT; i++)
	{
		mask = readword((u8*)fbuffer[LOADER], TILE_MASKS_START+mask_offset[0]);
		mask <<= 16;
		mask |= readword((u8*)fbuffer[LOADER], TILE_MASKS_START+mask_offset[1]);
//			printf("mask[%d] = %08x\n", i, mask);
		//
		// this condition is true when a footprint mask bit 
		// is set over a cleared tile mask bit (overflow)
		if sprite_overflow(footprint,mask)
		{
			return 0;
		}
		mask_y+=4;
		// Do we need to change tile in y?
		if (mask_y == 0x40)
		{	// went over the tile boundary
			// => replace upper mask offsets with lower
			mask_offset[0] = mask_offset[2];
			mask_offset[1] = mask_offset[3];
		}
		else
		{
			mask_offset[0] +=4;
			mask_offset[1] +=4;
		}
	}
	return -1;
}