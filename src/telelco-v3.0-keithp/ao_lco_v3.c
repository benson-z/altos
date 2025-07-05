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

#include <ao.h>
#include <ao_lco.h>
#include <ao_event.h>
#include <ao_quadrature.h>
#include <ao_radio_cmac.h>
#include <ao_st7565.h>
#include <ao_adc_single.h>
#include <ao_pwm.h>

#define WIDTH	AO_ST7565_WIDTH
#define HEIGHT	AO_ST7565_HEIGHT
#define STRIDE	AO_BITMAP_STRIDE(WIDTH)

static uint32_t	image[STRIDE * HEIGHT];

static struct ao_bitmap fb = {
	.base = image,
	.stride = STRIDE,
	.width = WIDTH,
	.height = HEIGHT,
	.damage = AO_BOX_INIT,
};

static const struct ao_transform logo_transform = {
	.x_scale = 40, .x_off = 8,
	.y_scale = 40, .y_off = 0,
};

static const struct ao_transform show_transform = {
	.x_scale = 36, .x_off = 100,
	.y_scale = 36, .y_off = 0,
};

#define BIG_FONT BitstreamVeraSans_Roman_58_font
#define VOLT_FONT BitstreamVeraSans_Roman_58_font
#define SMALL_FONT BitstreamVeraSans_Roman_12_font
#define TINY_FONT BitstreamVeraSans_Roman_10_font
#define LOGO_FONT BenguiatGothicStd_Bold_24_font

#define LABEL_Y		(int16_t) (SMALL_FONT.ascent)
#define VALUE_Y		(int16_t) (LABEL_Y + 5 + BIG_FONT.ascent)

#define SEP_X		82
#define SEP_WIDTH	2

#define BOX_X		(SEP_X / 2)
#define PAD_X		((WIDTH + SEP_X + SEP_WIDTH) / 2)

#define VALUE_LABEL_X	64
#define RSSI_LABEL_X	15

#define SCAN_X		(WIDTH - 100) / 2
#define SCAN_Y		50
#define SCAN_HEIGHT	3
#define SCANNING_X	(WIDTH / 2)
#define SCANNING_Y	(SCAN_Y - 2)
#define FOUND_Y		63
#define FOUND_X		3
#define FOUND_WIDTH	(WIDTH - 6)
#define CONTRAST_LABEL_X	37
#define CONTRAST_WIDTH	100
#define CONTRAST_X	(WIDTH - CONTRAST_WIDTH) / 2
#define CONTRAST_Y	20
#define CONTRAST_HEIGHT	20
#define CONTRAST_VALUE_X	64
#define CONTRAST_VALUE_Y	(CONTRAST_Y + CONTRAST_HEIGHT + SMALL_FONT.ascent + 3)
#define BACKLIGHT_LABEL_X	37
#define BACKLIGHT_WIDTH	100
#define BACKLIGHT_X	(WIDTH - BACKLIGHT_WIDTH) / 2
#define BACKLIGHT_Y	20
#define BACKLIGHT_HEIGHT	20
#define BACKLIGHT_VALUE_X	64
#define BACKLIGHT_VALUE_Y	(BACKLIGHT_Y + BACKLIGHT_HEIGHT + SMALL_FONT.ascent + 3)
#define INFO_FONT	TINY_FONT
#define INFO_START_Y	((int16_t) (INFO_FONT.ascent + 2))
#define INFO_STEP_Y	((int16_t) (INFO_FONT.ascent + 2))

#define AO_LCO_DRAG_RACE_START_TIME	AO_SEC_TO_TICKS(5)
#define AO_LCO_DRAG_RACE_STOP_TIME	AO_SEC_TO_TICKS(2)

/* UI values */
static uint8_t	ao_lco_select_mode;
static uint8_t	ao_lco_event_debug;

#define PRINTE(...) do { if (!ao_lco_debug && !ao_lco_event_debug) break; printf ("\r%5lu %s: ", (unsigned long) ao_tick_count, __func__); printf(__VA_ARGS__); flush(); } while(0)
#define AO_LCO_SELECT_BOX	0
#define AO_LCO_SELECT_PAD	1

static uint8_t	ao_lco_display_mutex;

static void
_ao_center_text(int16_t x, int16_t y, const struct ao_font *font, const char *str)
{
	int16_t width = ao_text_width(font, str);
	ao_text(&fb, font, x - width/2, y, str, AO_BLACK, AO_COPY);
}

static void
_ao_lco_show_pad(int8_t pad)
{
	char	str[5];

	_ao_center_text(PAD_X, LABEL_Y, &SMALL_FONT, "Pad");
	snprintf(str, sizeof(str), "%d", pad);
	_ao_center_text(PAD_X, VALUE_Y, &BIG_FONT, str);
}

static void
_ao_lco_show_box(int16_t box)
{
	char	str[7];

	_ao_center_text(BOX_X, LABEL_Y, &SMALL_FONT, "Bank");
	snprintf(str, sizeof(str), "%d", box);
	_ao_center_text(BOX_X, VALUE_Y, &BIG_FONT, str);
}

static void
_ao_format_voltage(char *str, size_t size, uint16_t decivolts)
{
	snprintf(str, size, "%d.%d", decivolts / 10, decivolts % 10);
}

#if AO_LCO_HAS_CONTRAST
static void
_ao_lco_show_contrast(void)
{
	char buf[8];
	uint8_t	brightness = ao_st7565_get_brightness();
	int16_t contrast = (int16_t) (brightness * CONTRAST_WIDTH / AO_LCO_MAX_CONTRAST);

	_ao_center_text(WIDTH/2, LABEL_Y, &SMALL_FONT, "Contrast");
	ao_rect(&fb, CONTRAST_X, CONTRAST_Y, contrast, CONTRAST_HEIGHT, AO_BLACK, AO_COPY);
	snprintf(buf, sizeof(buf), "%d %%", brightness * 100 / AO_LCO_MAX_CONTRAST);
	_ao_center_text(WIDTH/2, CONTRAST_VALUE_Y, &SMALL_FONT, buf);
}
#endif

#if AO_LCO_HAS_BACKLIGHT
static void
_ao_lco_show_backlight(void)
{
	char buf[8];
	int32_t	backlight = ao_lco_get_backlight();
	int16_t value = (int16_t) (backlight * BACKLIGHT_WIDTH / AO_LCO_MAX_BACKLIGHT);

	_ao_center_text(WIDTH/2, LABEL_Y, &SMALL_FONT, "Backlight");
	ao_rect(&fb, BACKLIGHT_X, BACKLIGHT_Y, value, BACKLIGHT_HEIGHT, AO_BLACK, AO_COPY);
	snprintf(buf, sizeof(buf), "%ld %%", backlight * 100 / AO_LCO_MAX_BACKLIGHT);
	_ao_center_text(WIDTH/2, BACKLIGHT_VALUE_Y, &SMALL_FONT, buf);
}
#endif

static int16_t info_y;

static void
_ao_lco_info(const char *format, ...)
{
	va_list a;
	char	buf[20];
	va_start(a, format);
	vsnprintf(buf, sizeof(buf), format, a);
	va_end(a);
	ao_text(&fb, &INFO_FONT, 0, info_y, buf, AO_BLACK, AO_COPY);
	info_y += INFO_STEP_Y;
}

static void
_ao_lco_show_lco_info(void)
{
	char		battery[7];
	struct ao_adc	packet;
	int16_t		decivolt;

	ao_logo_poly(&fb, &show_transform, AO_BLACK, AO_COPY);

	ao_adc_single_get(&packet);
	decivolt = ao_battery_decivolt(packet.v_batt);
	_ao_format_voltage(battery, sizeof(battery), (uint16_t) decivolt);

	info_y = INFO_START_Y;
	_ao_lco_info("%s", ao_product);
	_ao_lco_info("Serial: %d", ao_serial_number);
	_ao_lco_info("Battery: %sV", battery);
	_ao_lco_info("Version: %s", ao_version);
	_ao_lco_info("Callsign: %s", ao_config.callsign);
	_ao_lco_info("Frequency: %ld.%03d",
		     ao_config.frequency / 1000,
		     (int) (ao_config.frequency % 1000));
}

static uint8_t
popcount(uint32_t value)
{
	uint8_t count = 0;
	while(value != 0) {
		count += value & 1;
		value >>= 1;
	}
	return count;
}

static void
_ao_lco_show_pad_info(void)
{
	char	pad_battery[7];

	ao_logo_poly(&fb, &show_transform, AO_BLACK, AO_COPY);
	info_y = INFO_START_Y;
	_ao_lco_info("Bank: %d", ao_lco_box);
	if (!(ao_lco_valid[ao_lco_box] & AO_LCO_VALID_LAST)) {
		_ao_lco_info("Contact lost");
		_ao_lco_info("Last RSSI: %ddBm", ao_radio_cmac_last_rssi);
	} else {
		_ao_lco_info("Total pads: %d", popcount(ao_pad_query.channels));
		_ao_lco_info("RSSI: %ddBm", ao_radio_cmac_rssi);
		_ao_format_voltage(pad_battery, sizeof(pad_battery), ao_pad_query.battery);
		_ao_lco_info("Battery: %sV", pad_battery);
		_ao_lco_info("Arming switch: %s", ao_pad_query.arm_status ? "On" : "Off");
	}
}

void
ao_lco_show(void)
{
	ao_mutex_get(&ao_lco_display_mutex);
	ao_rect(&fb, 0, 0, WIDTH, HEIGHT, AO_WHITE, AO_COPY);
	switch (ao_lco_box) {
#if AO_LCO_HAS_CONTRAST
	case AO_LCO_CONTRAST:
		_ao_lco_show_contrast();
		break;
#endif
#if AO_LCO_HAS_BACKLIGHT
	case AO_LCO_BACKLIGHT:
		_ao_lco_show_backlight();
		break;
#endif
	case AO_LCO_LCO_INFO:
		_ao_lco_show_lco_info();
		break;
	default:
		switch (ao_lco_pad) {
		case AO_LCO_PAD_INFO:
			_ao_lco_show_pad_info();
			break;
		default:
			_ao_lco_show_pad(ao_lco_pad);
			_ao_lco_show_box(ao_lco_box);
			ao_rect(&fb, SEP_X, 0, SEP_WIDTH, HEIGHT, AO_BLACK, AO_COPY);
		}
		break;
	}
	ao_st7565_update(&fb);
	ao_mutex_put(&ao_lco_display_mutex);
}

static void
ao_lco_set_select(void)
{
	if (ao_lco_armed) {
		ao_led_off(AO_LED_PAD);
		ao_led_off(AO_LED_BOX);
	} else {
		switch (ao_lco_select_mode) {
		case AO_LCO_SELECT_PAD:
			ao_led_off(AO_LED_BOX);
			ao_led_on(AO_LED_PAD);
			break;
		case AO_LCO_SELECT_BOX:
			ao_led_off(AO_LED_PAD);
			ao_led_on(AO_LED_BOX);
			break;
		default:
			break;
		}
	}
}


#if AO_LCO_HAS_CONTRAST
void
ao_lco_set_contrast(int32_t contrast)
{
	ao_st7565_set_brightness((uint8_t) contrast);
}

int32_t
ao_lco_get_contrast(void)
{
	return (int32_t) ao_st7565_get_brightness();
}
#endif

#if AO_LCO_HAS_BACKLIGHT
static uint16_t ao_backlight;

void
ao_lco_set_backlight(int32_t backlight)
{
	ao_backlight = (uint16_t) backlight;
	ao_pwm_set(AO_LCD_BL_PWM_CHAN, ao_backlight);
}

int32_t
ao_lco_get_backlight(void)
{
	return (int32_t) ao_backlight;
}
#endif

static struct ao_task	ao_lco_drag_task;

static void
ao_lco_drag_monitor(void)
{
	AO_TICK_TYPE	delay = ~0UL;
	AO_TICK_TYPE	now;

	ao_beep_for(AO_BEEP_MID, AO_MS_TO_TICKS(200));
	for (;;) {
		PRINTD("Drag monitor count %d delay %lu\n", ao_lco_drag_beep_count, (unsigned long) delay);
		if (delay == (AO_TICK_TYPE) ~0)
			ao_sleep(&ao_lco_drag_beep_count);
		else
			ao_sleep_for(&ao_lco_drag_beep_count, delay);

		delay = ~0UL;
		now = ao_time();
		delay = ao_lco_drag_warn_check(now, delay);
		delay = ao_lco_drag_beep_check(now, delay);
	}
}

static void
ao_lco_input(void)
{
	static struct ao_event	event;

	for (;;) {
		ao_event_get(&event);
		PRINTE("event type %d unit %d value %ld\n",
		       event.type, event.unit, (long) event.value);
		switch (event.type) {
		case AO_EVENT_QUADRATURE:
			switch (event.unit) {
			case AO_QUADRATURE_SELECT:
				if (!ao_lco_armed) {
					switch (ao_lco_select_mode) {
					case AO_LCO_SELECT_PAD:
						ao_lco_step_pad((int8_t) event.value);
						break;
					case AO_LCO_SELECT_BOX:
						ao_lco_step_box((int8_t) event.value);
						break;
					default:
						break;
					}
				}
				break;
			}
			break;
		case AO_EVENT_BUTTON:
			switch (event.unit) {
			case AO_BUTTON_ARM:
				ao_lco_set_armed((uint8_t) event.value);
				ao_lco_set_select();
				break;
			case AO_BUTTON_FIRE:
				if (ao_lco_armed)
					ao_lco_set_firing((uint8_t) event.value);
				break;
			case AO_BUTTON_DRAG_SELECT:
				if (event.value)
					ao_lco_toggle_drag();
				break;
			case AO_BUTTON_DRAG_MODE:
				if (event.value)
					ao_lco_drag_enable();
				else
					ao_lco_drag_disable();
				break;
			case AO_BUTTON_ENCODER_SELECT:
				if (event.value) {
					if (!ao_lco_armed) {
						ao_lco_select_mode = 1 - ao_lco_select_mode;
						ao_lco_set_select();
					}
				}
				break;
			}
			break;
		}
	}
}

/*
 * Light up everything for a second at power on to let the user
 * visually inspect the system for correct operation
 */
static void
ao_lco_display_test(void)
{
	ao_led_on(AO_LEDS_AVAILABLE);
	ao_rect(&fb, 0, 0, WIDTH, HEIGHT, AO_BLACK, AO_COPY);
	ao_st7565_update(&fb);
	ao_delay(AO_MS_TO_TICKS(1000));
	ao_led_off(AO_LEDS_AVAILABLE);
}

static struct ao_task ao_lco_input_task;
static struct ao_task ao_lco_monitor_task;
static struct ao_task ao_lco_arm_warn_task;
static struct ao_task ao_lco_igniter_status_task;

static int16_t	found_width;
#define MAX_FOUND	32
static int16_t	found_boxes[MAX_FOUND];
static uint8_t	nfound;

void
ao_lco_search_start(void)
{
	ao_rect(&fb, 0, 0, WIDTH, HEIGHT, AO_WHITE, AO_COPY);
	ao_logo(&fb, &logo_transform, &LOGO_FONT, AO_BLACK, AO_COPY);
	_ao_center_text(SCANNING_X, SCANNING_Y, &TINY_FONT, "Scanning...");
	found_width = 0;
	nfound = 0;
}

void
ao_lco_search_box_check(int16_t box)
{
	if (box > 0)
		ao_rect(&fb, SCAN_X, SCAN_Y, box, SCAN_HEIGHT, AO_BLACK, AO_COPY);
	ao_st7565_update(&fb);
}

void
ao_lco_search_box_present(int16_t box)
{
	char	str[8];
	int16_t	width;
	int16_t box_top = FOUND_Y - TINY_FONT.ascent;
	int16_t	x;
	uint8_t	n;

	snprintf(str, sizeof(str), "%s%u", nfound ? ", " : "", box);
	width = ao_text_width(&TINY_FONT, str);
	while (found_width + width > FOUND_WIDTH || nfound == MAX_FOUND)
	{
		snprintf(str, sizeof(str), "%u, ", found_boxes[0]);
		found_width -= ao_text_width(&TINY_FONT, str);
		memmove(&found_boxes[0], &found_boxes[1], (nfound - 1) * sizeof (int16_t));
		nfound--;
	}
	found_boxes[nfound++] = box;

	ao_rect(&fb, FOUND_X, FOUND_Y - TINY_FONT.ascent, FOUND_WIDTH, HEIGHT - box_top, AO_WHITE, AO_COPY);
	x = FOUND_X;
	for (n = 0; n < nfound; n++) {
		snprintf(str, sizeof(str), "%s%u", n ? ", " : "", found_boxes[n]);
		int16_t next_x = ao_text(&fb, &TINY_FONT, x, FOUND_Y, str, AO_BLACK, AO_COPY);
		x = next_x;
	}
	found_width = x - FOUND_X;
}

void
ao_lco_search_done(void)
{
	ao_st7565_update(&fb);
}

static void
ao_lco_main(void)
{
	ao_lco_display_test();
	ao_lco_search();
	ao_add_task(&ao_lco_input_task, ao_lco_input, "lco input");
	ao_add_task(&ao_lco_arm_warn_task, ao_lco_arm_warn, "lco arm warn");
	ao_add_task(&ao_lco_igniter_status_task, ao_lco_igniter_status, "lco igniter status");
	ao_add_task(&ao_lco_drag_task, ao_lco_drag_monitor, "drag race");
	ao_lco_monitor();
}

#if DEBUG
static void
ao_lco_set_debug(void)
{
	uint32_t r = ao_cmd_decimal();
	if (ao_cmd_status == ao_cmd_success){
		ao_lco_debug = r & 1;
		ao_lco_event_debug = (r & 2) >> 1;
	}
}

const struct ao_cmds ao_lco_cmds[] = {
	{ ao_lco_set_debug,	"D <0 off, 1 on>\0Debug" },
	{ ao_lco_search,	"s\0Search for pad boxes" },
	{ ao_lco_pretend,	"p\0Pretend there are lots of pad boxes" },
	{ 0, NULL }
};
#endif

void
ao_lco_init(void)
{
	ao_add_task(&ao_lco_monitor_task, ao_lco_main, "lco monitor");
#if DEBUG
	ao_cmd_register(&ao_lco_cmds[0]);
#endif
}
