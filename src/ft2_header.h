#pragma once

#include <SDL2/SDL.h>
#include <stdint.h>
#include <stdbool.h>
#ifdef _DEBUG
#include <assert.h>
#endif
#ifdef _WIN32
#define WIN32_MEAN_AND_LEAN
#include <windows.h>
#else
#include <limits.h> // also has PATH_MAX
#endif
#include "ft2_replayer.h"

#define PROG_VER_STR "2.23"

// do NOT change these! It will only mess things up...

#define FT2_VBLANK_HZ 70.086302895323 /* VGA 640x400 @ 70Hz */
#define SCREEN_W 632
#define SCREEN_H 400
#define VBLANK_HZ 60

// 70Hz (FT2 vblank) delta -> 60Hz vblank delta (rounded)
#define SCALE_VBLANK_DELTA(x) (int32_t)(((x) * (FT2_VBLANK_HZ / (double)VBLANK_HZ)) + 0.5)
#define SCALE_VBLANK_DELTA_REV(x) (int32_t)(((x) * (VBLANK_HZ / (double)FT2_VBLANK_HZ)) + 0.5)

#ifndef ASSERT
#ifdef _DEBUG
#define ASSERT(x) assert(x)
#else
#define ASSERT(x)
#endif
#endif

/* Amount of extra bytes to allocate for every instrument sample,
** this is used for a hack for resampling interpolation to be
** branchless in the inner channel mixer loop.
** Warning: Do not change this!
*/
#define SMP_DAT_OFFSET ((MAX_LEFT_TAPS*2)+1)
#define SAMPLE_PAD_LENGTH (SMP_DAT_OFFSET+(MAX_RIGHT_TAPS*2))

#ifndef _WIN32
#define _stricmp strcasecmp
#define _strnicmp strncasecmp
#define DIR_DELIMITER '/'
#else
#define DIR_DELIMITER '\\'
#define PATH_MAX MAX_PATH
#endif

#ifndef PI
#define PI 3.14159265358979323846264338327950288
#endif

#define SGN(x) (((x) >= 0) ? 1 : -1)
#define ABS(a) (((a) < 0) ? -(a) : (a))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define CLAMP(x, low, high) (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

/* These convert between big-endian on-disk formats (AIFF, IFF/8SVX, MOD/DIGI/STK
** sample headers) and host order: a no-op on a big-endian host, a byte swap on a
** little-endian host. Works for both loading and saving.
*/
#define SWAP16(x) SDL_SwapBE16(x)
#define SWAP32(x) SDL_SwapBE32(x)
#define SWAP64(x) SDL_SwapBE64(x)

/* Little-endian on-disk formats (XM, and other PC/DOS-native formats) have no swapping
** at all today, since every existing supported platform is little-endian. These helpers
** are only compiled in for a big-endian host; calling the same helper twice (once before
** a write, once after) is safe since a byte swap is its own inverse.
*/
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
static inline void swapSampleData16(int8_t *data, int32_t numSamples)
{
	uint16_t *p = (uint16_t *)data;
	for (int32_t i = 0; i < numSamples; i++)
		p[i] = SDL_SwapLE16(p[i]);
}

static inline void swapXmHdr(xmHdr_t *h)
{
	h->version = SDL_SwapLE16(h->version);
	h->headerSize = (int32_t)SDL_SwapLE32((uint32_t)h->headerSize);
	h->numOrders = SDL_SwapLE16(h->numOrders);
	h->songLoopStart = SDL_SwapLE16(h->songLoopStart);
	h->numChannels = SDL_SwapLE16(h->numChannels);
	h->numPatterns = SDL_SwapLE16(h->numPatterns);
	h->numInstr = SDL_SwapLE16(h->numInstr);
	h->flags = SDL_SwapLE16(h->flags);
	h->speed = SDL_SwapLE16(h->speed);
	h->BPM = SDL_SwapLE16(h->BPM);
}

// used when saving (struct is written in one shot); on load these fields are read
// piecemeal (layout differs by XM version), so they're swapped individually instead
static inline void swapXmPatHdr(xmPatHdr_t *ph)
{
	ph->headerSize = (int32_t)SDL_SwapLE32((uint32_t)ph->headerSize);
	ph->numRows = (int16_t)SDL_SwapLE16((uint16_t)ph->numRows);
	ph->dataSize = SDL_SwapLE16(ph->dataSize);
}

static inline void swapXmSmpHdr(xmSmpHdr_t *sh)
{
	sh->length = SDL_SwapLE32(sh->length);
	sh->loopStart = SDL_SwapLE32(sh->loopStart);
	sh->loopLength = SDL_SwapLE32(sh->loopLength);
}

// swaps the instrument header fields only, NOT the embedded smp[] array (swap those via swapXmSmpHdr)
static inline void swapXmInsHdr(xmInsHdr_t *ih)
{
	ih->instrSize = SDL_SwapLE32(ih->instrSize);
	ih->numSamples = (int16_t)SDL_SwapLE16((uint16_t)ih->numSamples);
	ih->sampleSize = (int32_t)SDL_SwapLE32((uint32_t)ih->sampleSize);

	for (int32_t i = 0; i < 12; i++)
	{
		ih->volEnvPoints[i][0] = (int16_t)SDL_SwapLE16((uint16_t)ih->volEnvPoints[i][0]);
		ih->volEnvPoints[i][1] = (int16_t)SDL_SwapLE16((uint16_t)ih->volEnvPoints[i][1]);
		ih->panEnvPoints[i][0] = (int16_t)SDL_SwapLE16((uint16_t)ih->panEnvPoints[i][0]);
		ih->panEnvPoints[i][1] = (int16_t)SDL_SwapLE16((uint16_t)ih->panEnvPoints[i][1]);
	}

	ih->fadeout = SDL_SwapLE16(ih->fadeout);
	ih->midiProgram = (int16_t)SDL_SwapLE16((uint16_t)ih->midiProgram);
	ih->midiBend = (int16_t)SDL_SwapLE16((uint16_t)ih->midiBend);
}
#endif

typedef struct smpPtr_t
{
	int8_t *origPtr, *ptr;
} smpPtr_t;
