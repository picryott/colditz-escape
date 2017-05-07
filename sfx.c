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
*  sfx.c: SFXs structures
*  ---------------------------------------------------------------------------
*/

#include "sfx.h"
#include "low-level.h"
#include "soundplayer.h"
#include "cluck.h"

s_sfx sfx[NB_SFXS];

// Additional SFX
short*			upcluck;
unsigned long	upcluck_len;
short*			upthrill;
unsigned long	upthrill_len;


// Initalize the SFXs
void sfxs_initialize(uint8_t* buffer)
{
	uint16_t	i, j;

	// Initialize SFXs
	for (i = 0; i<NB_SFXS; i++)
	{
		sfx[i].address = SFX_ADDRESS_START + readword(buffer, SFX_TABLE_START + 8 * i);
		sfx[i].volume = readbyte(buffer, SFX_TABLE_START + 8 * i + 3);
		sfx[i].frequency = (uint16_t)(Period2Freq((float)readword(buffer, SFX_TABLE_START + 8 * i + 4)) / 1.0);
		sfx[i].length = readword(buffer, SFX_TABLE_START + 8 * i + 6);
#if defined(WIN32)
		// Why, of course Microsoft had to use UNSIGNED for 8 bit WAV data but SIGNED for 16!
		// (Whereas Commodore et al. did the LOGICAL thing of using signed ALWAYS)
		// We need to sign convert our 8 bit mono samples on Windows
		for (j = 0; j<sfx[i].length; j++)
			writebyte(buffer, sfx[i].address + j,
			(uint8_t)(readbyte(buffer, sfx[i].address + j) + 0x80));
#elif defined(PSP)
		// On the PSP on the other hand, we must upconvert our 8 bit mono samples @ whatever frequency
		// to 16bit/44.1 kHz. The psp_upsample() routine from soundplayer is there for that.
		psp_upsample(&(sfx[i].upconverted_address), &(sfx[i].upconverted_length),
			(char*)(buffer + sfx[i].address), sfx[i].length, sfx[i].frequency);
#else
#error No SFX playout for this platform
#endif
	}

	// The footstep's SFX volume is a bit too high for my taste
	sfx[SFX_FOOTSTEPS].volume /= 3;

#if defined(PSP)
	// Let's upconvert us some chicken
	for (j = 0; j<sizeof(cluck_sfx); j++)
		cluck_sfx[j] += 0x80;
	psp_upsample(&upcluck, &upcluck_len, (char*)cluck_sfx, sizeof(cluck_sfx), 8000);
	// "'coz this is thriller!..."
	for (j = 0; j<sizeof(thriller_sfx); j++)
		thriller_sfx[j] += 0x80;
	psp_upsample(&upthrill, &upthrill_len, (char*)thriller_sfx, sizeof(thriller_sfx), 8000);
#endif
}

// Play one of the 5 game SFXs
void play_sfx(int sfx_id, uint8_t* buffer, bool thrillerdance)
{
	if (thrillerdance)
		return;
#if defined(WIN32)
	play_sample(-1, sfx[sfx_id].volume, buffer + sfx[sfx_id].address,
		sfx[sfx_id].length, sfx[sfx_id].frequency, 8, false);
#elif defined(PSP)
	play_sample(-1, sfx[sfx_id].volume, sfx[sfx_id].upconverted_address,
		sfx[sfx_id].upconverted_length, PLAYBACK_FREQ, 16, false);
#else
#error No SFX playout for this platform
#endif
}


// Play one of the non game SFX
void play_cluck(bool thrillerdance)
{
	if (thrillerdance)
		return;
	msleep(120);
#if defined(WIN32)
	play_sample(-1, 60, cluck_sfx, sizeof(cluck_sfx), 8000, 8, false);
#elif defined(PSP)
	play_sample(-1, 60, upcluck, upcluck_len, PLAYBACK_FREQ, 16, false);
#else
#error No SFX playout for this platform
#endif
}

// Play one of the non game SFX
void thriller_toggle()
{
	static bool is_playing = false;

	if (is_playing)
		stop_loop();
	else
	{
#if defined(WIN32)
		play_sample(-1, 60, thriller_sfx, sizeof(thriller_sfx), 8000, 8, true);
#elif defined(PSP)
		play_sample(-1, 60, upthrill, upthrill_len, PLAYBACK_FREQ, 16, true);
#else
#error No SFX playout for this platform
#endif
	}
	is_playing = !is_playing;
}