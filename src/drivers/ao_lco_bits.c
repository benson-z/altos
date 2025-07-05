/*
 * Copyright © 2018 Keith Packard <keithp@keithp.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 */

#include <ao.h>
#include <ao_lco.h>
#include <ao_radio_cmac.h>

uint8_t		ao_lco_debug;

int8_t		ao_lco_pad;
int16_t		ao_lco_box;

uint8_t		ao_lco_armed;					/* arm active */
uint8_t		ao_lco_firing;					/* fire active */

int16_t		ao_lco_min_box, ao_lco_max_box;

uint8_t		ao_lco_pretending;

#if AO_LCO_DRAG
uint8_t		ao_lco_drag_race;
#endif

struct ao_pad_query	ao_pad_query;				/* latest query response */

static uint8_t		ao_lco_channels[AO_PAD_MAX_BOXES];	/* pad channels available on each box */
static uint16_t		ao_lco_tick_offset[AO_PAD_MAX_BOXES];	/* 16 bit offset from local to remote tick count */
static uint8_t		ao_lco_selected[AO_PAD_MAX_BOXES];	/* pads selected to fire */

static uint32_t		ao_lco_query_good, ao_lco_query_bad;

uint8_t		ao_lco_valid[AO_PAD_MAX_BOXES];		/* AO_LCO_VALID bits per box */

static const AO_LED_TYPE	continuity_led[AO_LED_CONTINUITY_NUM] = {
#ifdef AO_LED_CONTINUITY_0
	AO_LED_CONTINUITY_0,
#endif
#ifdef AO_LED_CONTINUITY_1
	AO_LED_CONTINUITY_1,
#endif
#ifdef AO_LED_CONTINUITY_2
	AO_LED_CONTINUITY_2,
#endif
#ifdef AO_LED_CONTINUITY_3
	AO_LED_CONTINUITY_3,
#endif
#ifdef AO_LED_CONTINUITY_4
	AO_LED_CONTINUITY_4,
#endif
#ifdef AO_LED_CONTINUITY_5
	AO_LED_CONTINUITY_5,
#endif
#ifdef AO_LED_CONTINUITY_6
	AO_LED_CONTINUITY_6,
#endif
#ifdef AO_LED_CONTINUITY_7
	AO_LED_CONTINUITY_7,
#endif
};

/* Set LEDs to match remote box status */
void
ao_lco_igniter_status(void)
{
	uint8_t		c;
	uint8_t		t = 0;

	for (;;) {
#if AO_LCO_DRAG
		if (ao_lco_drag_race)
			ao_sleep_for(&ao_pad_query, AO_MS_TO_TICKS(50));
		else
#endif
			ao_sleep(&ao_pad_query);
		if (ao_lco_box_pseudo(ao_lco_box)) {
			ao_led_off(AO_LED_GREEN|AO_LED_AMBER|AO_LED_RED);
			continue;
		}
		PRINTD("RSSI %d VALID %d\n", ao_radio_cmac_rssi, ao_lco_valid[ao_lco_box]);
		if (!(ao_lco_valid[ao_lco_box] & AO_LCO_VALID_LAST)) {
			ao_led_on(AO_LED_RED);
			ao_led_off(AO_LED_GREEN|AO_LED_AMBER);
			continue;
		}
		if (ao_radio_cmac_rssi < -90) {
			ao_led_on(AO_LED_AMBER);
			ao_led_off(AO_LED_RED|AO_LED_GREEN);
		} else {
			ao_led_on(AO_LED_GREEN);
			ao_led_off(AO_LED_RED|AO_LED_AMBER);
		}
		if (ao_pad_query.arm_status)
			ao_led_on(AO_LED_REMOTE_ARM);
		else
			ao_led_off(AO_LED_REMOTE_ARM);

		for (c = 0; c < AO_LED_CONTINUITY_NUM; c++) {
			uint8_t	status;

			if (ao_pad_query.channels & (1 << c))
				status = ao_pad_query.igniter_status[c];
			else
				status = AO_PAD_IGNITER_STATUS_NO_IGNITER_RELAY_OPEN;

#if AO_LCO_DRAG
			if (ao_lco_drag_race && (ao_lco_selected[ao_lco_box] & (1 << c))) {
				uint8_t	on = 0;
				if (status == AO_PAD_IGNITER_STATUS_GOOD_IGNITER_RELAY_OPEN) {
					if (t)
						on = 1;
				} else {
					if (t == 1)
						on = 1;
				}
				if (on)
					ao_led_on(continuity_led[c]);
				else
					ao_led_off(continuity_led[c]);
			} else
#endif
			{
				if (status == AO_PAD_IGNITER_STATUS_GOOD_IGNITER_RELAY_OPEN)
					ao_led_on(continuity_led[c]);
				else
					ao_led_off(continuity_led[c]);
			}
		}
		t = (t + 1) & 3;
	}
}

uint8_t
ao_lco_pad_present(int16_t box, int8_t pad)
{
	/* voltage measurement is always valid */
	if (ao_lco_pad_pseudo(pad))
		return 1;
	if (!ao_lco_channels[box])
		return 0;
	if (pad > AO_PAD_MAX_CHANNELS)
		return 0;
	return (ao_lco_channels[box] >> (pad - 1)) & 1;
}

int8_t
ao_lco_pad_first(int16_t box)
{
	int8_t	pad;

	for (pad = 1; pad <= AO_PAD_MAX_CHANNELS; pad++)
		if (ao_lco_pad_present(box, pad))
			return pad;
	return 0;
}

static uint8_t
ao_lco_get_channels(int16_t box, struct ao_pad_query *query)
{
	int8_t			r;

	r = ao_lco_query((uint16_t) box, query, &ao_lco_tick_offset[box]);
	if (r == AO_RADIO_CMAC_OK) {
		ao_lco_channels[box] = query->channels;
		ao_lco_valid[box] = AO_LCO_VALID_LAST | AO_LCO_VALID_EVER;
		++ao_lco_query_good;
	} else {
		ao_lco_valid[box] &= (uint8_t) ~AO_LCO_VALID_LAST;
		++ao_lco_query_bad;
	}
	PRINTD("ao_lco_get_channels(%d) rssi %d valid %d ret %d offset %d good %"PRIu32" bad %"PRIu32"\n",
	       box, ao_radio_cmac_rssi, ao_lco_valid[box], r, ao_lco_tick_offset[box],
	       ao_lco_query_good, ao_lco_query_bad);
	ao_wakeup(&ao_pad_query);
	return ao_lco_valid[box];
}

void
ao_lco_update(void)
{
	if (ao_lco_box_pseudo(ao_lco_box)) {
		ao_lco_show();
		return;
	}

	uint8_t	previous_valid = ao_lco_valid[ao_lco_box];

	if (ao_lco_get_channels(ao_lco_box, &ao_pad_query) & AO_LCO_VALID_LAST) {
		if (!(previous_valid & AO_LCO_VALID_EVER)) {
			if (!ao_lco_pad_pseudo(ao_lco_pad))
				ao_lco_set_pad(ao_lco_pad_first(ao_lco_box));
		}
	}
	if (ao_lco_pad_pseudo(ao_lco_pad))
		ao_lco_show();
}

uint8_t	ao_lco_box_mask[AO_LCO_MASK_SIZE(AO_PAD_MAX_BOXES)];

static void
ao_lco_box_reset_present(void)
{
	ao_lco_min_box = 0xff;
	ao_lco_max_box = 0x00;
	ao_lco_pretending = 0;
	memset(ao_lco_box_mask, 0, sizeof (ao_lco_box_mask));
}

static void
ao_lco_box_set_present(int16_t box)
{
	if (box < ao_lco_min_box)
		ao_lco_min_box = box;
	if (box > ao_lco_max_box)
		ao_lco_max_box = box;
	if (box >= AO_PAD_MAX_BOXES)
		return;
	ao_lco_box_mask[AO_LCO_MASK_ID(box)] |= (uint8_t) (1 << AO_LCO_MASK_SHIFT(box));
}

void
ao_lco_set_pad(int8_t new_pad)
{
	ao_lco_pad = new_pad;
	ao_lco_show();
}

void
ao_lco_set_box(int16_t new_box)
{
	ao_lco_box = new_box;
	if (!ao_lco_box_pseudo(ao_lco_box)) {
		if (ao_lco_box < AO_PAD_MAX_BOXES) {
			if (ao_lco_pretending)
				ao_lco_channels[ao_lco_box] = 0xff;
			else
				ao_lco_channels[ao_lco_box] = 0;
		}
	}
	ao_lco_pad = 1;
	ao_lco_show();
}

void
ao_lco_step_pad(int8_t dir)
{
	int16_t	new_pad;

	switch (ao_lco_box) {
#ifdef AO_LCO_HAS_CONTRAST
	case AO_LCO_CONTRAST: {
		int32_t contrast = ao_lco_get_contrast();

		contrast = (contrast + AO_LCO_CONTRAST_STEP - 1) / AO_LCO_CONTRAST_STEP;
		contrast += dir;
		contrast *= AO_LCO_CONTRAST_STEP;
		if (contrast < AO_LCO_MIN_CONTRAST)
			contrast = AO_LCO_MIN_CONTRAST;
		if (contrast > AO_LCO_MAX_CONTRAST)
			contrast = AO_LCO_MAX_CONTRAST;
		ao_lco_set_contrast(contrast);
		ao_lco_show();
		break;
	}
#endif
#ifdef AO_LCO_HAS_BACKLIGHT
	case AO_LCO_BACKLIGHT: {
		int32_t backlight = ao_lco_get_backlight();

		backlight = (backlight + AO_LCO_BACKLIGHT_STEP - 1) / AO_LCO_BACKLIGHT_STEP;
		backlight += dir;
		backlight *= AO_LCO_BACKLIGHT_STEP;
		if (backlight < AO_LCO_MIN_BACKLIGHT)
			backlight = AO_LCO_MIN_BACKLIGHT;
		if (backlight > AO_LCO_MAX_BACKLIGHT)
			backlight = AO_LCO_MAX_BACKLIGHT;
		ao_lco_set_backlight(backlight);
		ao_lco_show();
		break;
	}
#endif
#if AO_LCO_HAS_LCO_INFO
	case AO_LCO_LCO_INFO: {
#if AO_LCO_MIN_INFO_PAGE < AO_LCO_MAX_INFO_PAGE
		int32_t info_page = ao_lco_get_info_page();

		info += dir;
		if (info_page < AO_LCO_MIN_INFO_PAGE)
			info_page = AO_LCO_MIN_INFO_PAGE;
		if (info_page > AO_LCO_MAX_INFO_PAGE)
			info_page = AO_LCO_MAX_INFO_PAGE;
		ao_lco_set_info_page();
#endif
		break;
	}
#endif
	default:
		new_pad = (int16_t) ao_lco_pad;
		do {
			new_pad += dir;
			if (new_pad > AO_PAD_MAX_CHANNELS)
				new_pad = AO_LCO_PAD_FIRST;
			if (new_pad < AO_LCO_PAD_FIRST)
				new_pad = AO_PAD_MAX_CHANNELS;
			if (new_pad == ao_lco_pad)
				break;
		} while (!ao_lco_pad_present(ao_lco_box, (int8_t) new_pad));
		PRINTD("New pad %d\n", new_pad);
		ao_lco_set_pad((int8_t) new_pad);
		break;
	}
}

uint8_t
ao_lco_box_present(int16_t box)
{
	if (ao_lco_box_pseudo(box))
		return 1;
	if (box >= AO_PAD_MAX_BOXES)
		return 0;
	return (ao_lco_box_mask[AO_LCO_MASK_ID(box)] >> AO_LCO_MASK_SHIFT(box)) & 1;
}

void
ao_lco_step_box(int8_t dir)
{
	int16_t new_box = ao_lco_box;

	do {
		new_box += dir;
		if (new_box > ao_lco_max_box)
			new_box = AO_LCO_BOX_FIRST;
		else if (new_box < AO_LCO_BOX_FIRST)
			new_box = ao_lco_max_box;
		if (new_box == ao_lco_box)
			break;
	} while (!ao_lco_box_present(new_box));
	PRINTD("New box %d\n", new_box);
	ao_lco_set_box(new_box);
}

void
ao_lco_set_armed(uint8_t armed)
{
	if (ao_lco_box_pseudo(ao_lco_box))
		return;

	ao_lco_armed = armed;
	PRINTD("Armed %d\n", ao_lco_armed);
	if (ao_lco_armed) {
#if AO_LCO_DRAG
		if (ao_lco_drag_race) {
			int16_t	box;

			for (box = ao_lco_min_box; box <= ao_lco_max_box; box++)
				if (ao_lco_selected[box])
					break;
			if (box > ao_lco_max_box)
				ao_lco_armed = 0;
		} else
#endif
		{
			memset(ao_lco_selected, 0, sizeof (ao_lco_selected));
			if (ao_lco_pad != 0)
				ao_lco_selected[ao_lco_box] = (1 << (ao_lco_pad - 1));
			else
				ao_lco_armed = 0;
		}
	}
	ao_wakeup(&ao_lco_armed);
}

void
ao_lco_set_firing(uint8_t firing)
{
	ao_lco_firing = firing;
	PRINTD("Firing %d\n", ao_lco_firing);
	ao_wakeup(&ao_lco_armed);
}

#if 0
static int16_t fake_boxes[] = {
	1, 2, 3, 5, 8, 11, 13, 17, 19, 23, 29, 31, 37, 62, 97
};
#define ARRAYSIZE(a)	(sizeof(a) / sizeof((a)[0]))
#define NFAKE ARRAYSIZE(fake_boxes)

static bool
is_fake(int16_t box)
{
	unsigned i;
	for (i = 0; i < NFAKE; i++)
		if (fake_boxes[i] == box)
			return true;
	return false;
}
#else
#define is_fake(b)	false
#endif

void
ao_lco_search(void)
{
	int8_t		r;
	int8_t		try;
	int16_t		box;
	uint16_t	boxes = 0;

	ao_lco_box_reset_present();
#ifdef AO_LCO_SEARCH_API
	ao_lco_search_start();
#else
	ao_lco_show_box(0);
	ao_lco_show_pad(0);
#endif
	for (box = 0; box < AO_PAD_MAX_BOXES; box++) {
#ifdef AO_LCO_SEARCH_API
		ao_lco_search_box_check(box);
#else
		if ((box % 10) == 0)
			ao_lco_show_box(box);
#endif
		for (try = 0; try < 3; try++) {
			ao_lco_tick_offset[box] = 0;
			r = ao_lco_query((uint16_t) box, &ao_pad_query, &ao_lco_tick_offset[box]);
			PRINTD("box %d result %d offset %d\n", box, r, ao_lco_tick_offset[box]);
			if (r == AO_RADIO_CMAC_OK || is_fake(box)) {
				++boxes;
				ao_lco_box_set_present(box);
#ifdef AO_LCO_SEARCH_API
				ao_lco_search_box_present(box);
#else
				ao_lco_show_pad((int8_t) (boxes % 10));
#endif
				ao_delay(AO_MS_TO_TICKS(30));
				break;
			}
		}
	}
	if (ao_lco_min_box <= ao_lco_max_box)
		ao_lco_box = ao_lco_min_box;
	else
		ao_lco_min_box = ao_lco_max_box = ao_lco_box = 0;
	memset(ao_lco_valid, 0, sizeof (ao_lco_valid));
	memset(ao_lco_channels, 0, sizeof (ao_lco_channels));
#ifdef AO_LCO_SEARCH_API
	ao_lco_search_done();
#endif
	ao_lco_set_box(ao_lco_min_box);
}

void
ao_lco_pretend(void)
{
	int16_t box;

	ao_lco_pretending = 1;
	ao_lco_min_box = 1;
	ao_lco_max_box = AO_PAD_MAX_BOXES - 1;
	for (box = ao_lco_min_box; box <= ao_lco_max_box; box++)
		ao_lco_box_set_present(box);
	ao_lco_box = ao_lco_min_box;
	memset(ao_lco_valid, 0, sizeof (ao_lco_valid));
	memset(ao_lco_channels, 0, sizeof (ao_lco_channels));
	ao_lco_set_box(ao_lco_min_box);
}

void
ao_lco_monitor(void)
{
	AO_TICK_TYPE	delay;
	int16_t		box;

	for (;;) {
		PRINTD("monitor armed %d firing %d\n",
		       ao_lco_armed, ao_lco_firing);

		if (ao_lco_armed && ao_lco_firing) {
			ao_lco_ignite(AO_PAD_FIRE);
		} else {
			ao_lco_update();
			if (ao_lco_armed) {
				for (box = ao_lco_min_box; box <= ao_lco_max_box; box++) {
					if (ao_lco_selected[box]) {
						PRINTD("Arming box %d pads %x\n",
						       box, ao_lco_selected[box]);
						if (ao_lco_valid[box] & AO_LCO_VALID_EVER) {
							ao_lco_arm((uint16_t) box, ao_lco_selected[box], ao_lco_tick_offset[box]);
							ao_delay(AO_MS_TO_TICKS(10));
						}
					}
				}
			}
		}
		if (ao_lco_armed && ao_lco_firing)
			delay = AO_MS_TO_TICKS(100);
		else
			delay = AO_SEC_TO_TICKS(1);
		ao_sleep_for(&ao_lco_armed, delay);
	}
}

#if AO_LCO_DRAG

int8_t			ao_lco_drag_beep_count;
static uint8_t		ao_lco_drag_beep_on;
static AO_TICK_TYPE	ao_lco_drag_beep_time;
static AO_TICK_TYPE	ao_lco_drag_warn_time;

#define AO_LCO_DRAG_BEEP_TIME	AO_MS_TO_TICKS(50)
#define AO_LCO_DRAG_WARN_TIME	AO_SEC_TO_TICKS(5)

/* Request 'beeps' additional drag race beeps */
void
ao_lco_drag_add_beeps(int8_t beeps)
{
	PRINTD("beep %d\n", beeps);
	if (ao_lco_drag_beep_count == 0)
		ao_lco_drag_beep_time = ao_time();
	ao_lco_drag_beep_count += beeps;
	ao_wakeup(&ao_lco_drag_beep_count);
}

/* Toggle current pad in drag set */
void
ao_lco_toggle_drag(void)
{
	if (ao_lco_drag_race && !ao_lco_pad_pseudo(ao_lco_pad) && !ao_lco_box_pseudo(ao_lco_box)) {
		ao_lco_selected[ao_lco_box] ^= (uint8_t) (1 << (ao_lco_pad - 1));
		PRINTD("Toggle box %d pad %d (pads now %x) to drag race\n",
		       ao_lco_pad, ao_lco_box, ao_lco_selected[ao_lco_box]);
		ao_lco_drag_add_beeps(ao_lco_pad);
	}
}

/* Check whether it's time to change the beeper status, then either
 * turn it on or off as necessary and bump the remaining beep counts
 */

AO_TICK_TYPE
ao_lco_drag_beep_check(AO_TICK_TYPE now, AO_TICK_TYPE delay)
{
	PRINTD("beep check count %d delta %ld\n",
	       ao_lco_drag_beep_count,
	       (long) (AO_TICK_SIGNED) (now - ao_lco_drag_beep_time));
	if (ao_lco_drag_beep_count) {
		if ((AO_TICK_SIGNED) (now - ao_lco_drag_beep_time) >= 0) {
			if (ao_lco_drag_beep_on) {
				ao_beep(0);
				PRINTD("beep stop\n");
				ao_lco_drag_beep_on = 0;
				if (ao_lco_drag_beep_count) {
					--ao_lco_drag_beep_count;
					if (ao_lco_drag_beep_count)
						ao_lco_drag_beep_time = now + AO_LCO_DRAG_BEEP_TIME;
				}
			} else {
				ao_beep(AO_BEEP_HIGH);
				PRINTD("beep start\n");
				ao_lco_drag_beep_on = 1;
				ao_lco_drag_beep_time = now + AO_LCO_DRAG_BEEP_TIME;
			}
		}
	}

	if (ao_lco_drag_beep_count) {
		AO_TICK_TYPE beep_delay = 0;

		if (ao_lco_drag_beep_time > now)
			beep_delay = ao_lco_drag_beep_time - now;

		if (delay > beep_delay)
			delay = beep_delay;
	}
	return delay;
}

void
ao_lco_drag_enable(void)
{
	if (!ao_lco_drag_race) {
		PRINTD("Drag enable\n");
		ao_lco_drag_race = 1;
		memset(ao_lco_selected, 0, sizeof (ao_lco_selected));
#ifdef AO_LED_DRAG
		ao_led_on(AO_LED_DRAG);
#endif
		ao_lco_drag_add_beeps(5);
		ao_lco_show();
	}
}

void
ao_lco_drag_disable(void)
{
	if (ao_lco_drag_race) {
		PRINTD("Drag disable\n");
		ao_lco_drag_race = 0;
#ifdef AO_LED_DRAG
		ao_led_off(AO_LED_DRAG);
#endif
		memset(ao_lco_selected, 0, sizeof (ao_lco_selected));
		ao_lco_drag_add_beeps(2);
		ao_lco_show();
	}
}

/* add a beep if it's time to warn the user that drag race mode is
 * active
 */

AO_TICK_TYPE
ao_lco_drag_warn_check(AO_TICK_TYPE now, AO_TICK_TYPE delay)
{
	if (ao_lco_drag_race) {
		AO_TICK_TYPE	warn_delay;

		if ((AO_TICK_SIGNED) (now - ao_lco_drag_warn_time) >= 0) {
			ao_lco_drag_add_beeps(1);
			ao_lco_drag_warn_time = now + AO_LCO_DRAG_WARN_TIME;
		}
		warn_delay = ao_lco_drag_warn_time - now;
		if (delay > warn_delay)
			delay = warn_delay;
	}
	return delay;
}
#endif /* AO_LCO_DRAG */

/* task function for beeping while arm is active */
void
ao_lco_arm_warn(void)
{
	for (;;) {
		while (!ao_lco_armed) {
#ifdef AO_LED_FIRE
			ao_led_off(AO_LED_FIRE);
#endif
			ao_sleep(&ao_lco_armed);
		}
#ifdef AO_LED_FIRE
		ao_led_on(AO_LED_FIRE);
#endif
		ao_beep_for(AO_BEEP_MID, AO_MS_TO_TICKS(200));
		ao_delay(AO_MS_TO_TICKS(200));
	}
}
