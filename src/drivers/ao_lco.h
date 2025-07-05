/*
 * Copyright © 2012 Keith Packard <keithp@keithp.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */

#ifndef _AO_LCO_H_
#define _AO_LCO_H_

#include <ao_lco_func.h>

#ifndef AO_LCO_DRAG
#define AO_LCO_DRAG	1
#endif

#define DEBUG	1

#if DEBUG
extern uint8_t	ao_lco_debug;
#define PRINTD(...) do { if (!ao_lco_debug) break; printf ("\r%5lu %s: ", (unsigned long) ao_tick_count, __func__); printf(__VA_ARGS__); flush(); } while(0)
#else
#define PRINTD(...) 
#endif

#if AO_LCO_DRAG
extern uint8_t	ao_lco_drag_race;	/* true when drag race mode enabled */
#endif

extern int8_t	ao_lco_pad;		/* Currently selected pad */
extern int16_t	ao_lco_box;		/* Currently selected box */

extern uint8_t	ao_lco_armed;		/* armed mode active */
extern uint8_t	ao_lco_firing;		/* fire button pressed */

extern struct ao_pad_query	ao_pad_query;	/* Last received QUERY from pad */

#ifdef AO_LCO_DRAG_RACE_BOX
# define AO_LCO_BOX_DRAG	0		/* Box number to enable drag race mode (old LCO bits) */
# define AO_LCO_BOX_FIRST	AO_LCO_BOX_DRAG
#else
# ifdef AO_LCO_HAS_CONTRAST
#  define AO_LCO_CONTRAST	-2
#  ifndef AO_LCO_BOX_FIRST
#   define AO_LCO_BOX_FIRST	AO_LCO_CONTRAST
#  endif
# endif
# ifdef AO_LCO_HAS_BACKLIGHT
#  define AO_LCO_BACKLIGHT 	-1
#  ifndef AO_LCO_BOX_FIRST
#   define AO_LCO_BOX_FIRST 	AO_LCO_BACKLIGHT
#  endif
# endif
# if AO_LCO_HAS_LCO_INFO
#  define AO_LCO_LCO_INFO	0		/* Box number to show LCO info */
#  ifndef AO_LCO_BOX_FIRST
#   define AO_LCO_BOX_FIRST	AO_LCO_LCO_INFO
#  endif
# else
#  define AO_LCO_LCO_VOLTAGE	-1
#  ifndef AO_LCO_BOX_FIRST
#   define AO_LCO_BOX_FIRST	AO_LCO_LCO_VOLTAGE
#  endif
# endif
# ifndef AO_LCO_BOX_FIRST
#  define AO_LCO_BOX_FIRST	1
# endif
#endif

#ifdef AO_LCO_HAS_PAD_INFO
# define AO_LCO_PAD_INFO	0		/* Pad number to show box info */
# define AO_LCO_PAD_FIRST	AO_LCO_PAD_INFO
#else
# define AO_LCO_PAD_VOLTAGE	0		/* Pad number to show box voltage */
# define AO_LCO_PAD_RSSI	-1		/* Pad number to show box RSSI */
# define AO_LCO_PAD_FIRST	AO_LCO_PAD_RSSI
#endif

static inline bool
ao_lco_box_pseudo(int16_t box)
{
	switch (box) {
#ifdef AO_LCO_LCO_VOLTAGE
	case AO_LCO_LCO_VOLTAGE:
		return true;
#endif
#ifdef AO_LCO_DRAG_RACE_BOX
	case AO_LCO_BOX_DRAG:
		return true;
#endif
#ifdef AO_LCO_CONTRAST
	case AO_LCO_CONTRAST:
		return true;
#endif
#ifdef AO_LCO_BACKLIGHT
	case AO_LCO_BACKLIGHT:
		return true;
#endif
#ifdef AO_LCO_LCO_INFO
	case AO_LCO_LCO_INFO:
		return true;
#endif
	default:
		return false;
	}
}

static inline bool
ao_lco_pad_pseudo(int8_t pad)
{
	switch (pad) {
#ifdef AO_LCO_PAD_VOLTAGE
	case AO_LCO_PAD_VOLTAGE:
		return true;
#endif
#ifdef AO_LCO_PAD_RSSI
	case AO_LCO_PAD_RSSI:
		return true;
#endif
#ifdef AO_LCO_PAD_INFO
	case AO_LCO_PAD_INFO:
		return true;
#endif
	default:
		return false;
	}
}

extern int16_t	ao_lco_min_box, ao_lco_max_box;

#define AO_LCO_MASK_SIZE(n)	(((n) + 7) >> 3)
#define AO_LCO_MASK_ID(n)	((n) >> 3)
#define AO_LCO_MASK_SHIFT(n)	((n) & 7)

extern uint8_t	ao_lco_box_mask[AO_LCO_MASK_SIZE(AO_PAD_MAX_BOXES)];

#define AO_LCO_VALID_LAST	1
#define AO_LCO_VALID_EVER	2

extern uint8_t	ao_lco_valid[AO_PAD_MAX_BOXES];		/* AO_LCO_VALID bits per box */

/*
 * Shared functions
 */

void
ao_lco_igniter_status(void);

void
ao_lco_update(void);

uint8_t
ao_lco_pad_present(int16_t box, int8_t pad);

int8_t
ao_lco_pad_first(int16_t box);

void
ao_lco_set_pad(int8_t new_pad);

void
ao_lco_step_pad(int8_t dir);

void
ao_lco_set_box(int16_t new_box);

void
ao_lco_step_box(int8_t dir);

void
ao_lco_set_armed(uint8_t armed);

void
ao_lco_set_firing(uint8_t firing);

void
ao_lco_pretend(void);

void
ao_lco_toggle_drag(void);

void
ao_lco_search(void);

void
ao_lco_monitor(void);

extern int8_t			ao_lco_drag_beep_count;

/* enable drag race mode */
void
ao_lco_drag_enable(void);

/* disable drag race mode */
void
ao_lco_drag_disable(void);

/* Handle drag beeps, return new delay */
AO_TICK_TYPE
ao_lco_drag_beep_check(AO_TICK_TYPE now, AO_TICK_TYPE delay);

/* Check if it's time to beep during drag race. Return new delay */
AO_TICK_TYPE
ao_lco_drag_warn_check(AO_TICK_TYPE now, AO_TICK_TYPE delay);

/* Request 'beeps' additional drag race beeps */
void
ao_lco_drag_add_beeps(int8_t beeps);

/* task function for beeping while arm is active */
void
ao_lco_arm_warn(void);

/*
 * Provided by the hw-specific driver code
 */

void
ao_lco_show_pad(int8_t pad);

void
ao_lco_show_box(int16_t box);

void
ao_lco_show(void);

void
ao_lco_init(void);

uint8_t
ao_lco_box_present(int16_t box);

#ifdef AO_LCO_HAS_CONTRAST
void
ao_lco_set_contrast(int32_t contrast);

int32_t
ao_lco_get_contrast(void);
#endif

#ifdef AO_LCO_HAS_BACKLIGHT
void
ao_lco_set_backlight(int32_t backlight);

int32_t
ao_lco_get_backlight(void);
#endif

#ifdef AO_LCO_SEARCH_API

void
ao_lco_search_start(void);

void
ao_lco_search_box_check(int16_t box);

void
ao_lco_search_box_present(int16_t box);

void
ao_lco_search_done(void);

#endif /* AO_LCO_SEARCH_API */

#endif /* _AO_LCO_H_ */
