/*
 * Copyright © 2024 Keith Packard <keithp@keithp.com>
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
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */

#ifndef AO_GPS_TEST
#include "ao.h"
#endif

#if HAS_GPS_MOSAIC

#include "ao_gps_mosaic.h"
#include "ao_crc_ccitt.h"
#include <stdarg.h>
#include <stdio.h>
#include <math.h>

#define AO_MOSAIC_DEBUG	0

#if AO_MOSAIC_DEBUG

#define DBG_PROTO	1
#define DBG_CHAR	2
#define DBG_INIT	4

static uint8_t mosaic_dbg_enable = 0;

static void mosaic_dbg(int level, char *format, ...) {
	va_list a;

	if (level & mosaic_dbg_enable) {
		va_start(a, format);
		vprintf(format, a);
		va_end(a);
		flush();
	}
}

#else
#define mosaic_dbg(fmt, ...)
#endif

static inline uint8_t mosaic_byte(void) {
	uint8_t	c = (uint8_t) ao_mosaic_getchar();
#if AO_MOSAIC_DEBUG
	if (' ' <= c && c <= '~')
		mosaic_dbg(DBG_CHAR, "%c", c);
	else
		mosaic_dbg(DBG_CHAR, " %02x", c);
#endif
	return c;
}

static inline void mosaic_putc(char c) {
#if AO_MOSAIC_DEBUG
	if (' ' <= c && c <= '~')
		mosaic_dbg(DBG_CHAR, "%c", c);
	else
		mosaic_dbg(DBG_CHAR, " (%02x)", c);
#endif
	ao_mosaic_putchar(c);
}

static void mosaic_puts(const char *s) {
	char c;
	while ((c = *s++) != '\0')
		mosaic_putc(c);
}

extern uint8_t ao_gps_new;
extern uint8_t ao_gps_mutex;
extern AO_TICK_TYPE ao_gps_tick;
extern AO_TICK_TYPE ao_gps_utc_tick;
extern struct ao_telemetry_location	ao_gps_data;
extern struct ao_telemetry_satellite	ao_gps_tracking_data;

static void
mosaic_wait_idle(void)
{
	uint8_t	b;
	for (;;) {
		while(mosaic_byte() != '$')
			;
		if (mosaic_byte() != 'R')
			continue;
		for (;;) {
			b = mosaic_byte();
			if (b == '\n')
				return;
		}
	}
}

static void
mosaic_command(const char *cmd)
{
	mosaic_puts(cmd);
	mosaic_wait_idle();
}

static void
mosaic_setup(void)
{
	ao_mosaic_set_speed(AO_SERIAL_SPEED_115200);
	/* Send messages always */
	mosaic_command("smrf, OnlyRef \r");
	/* Set receiver dynamics mode */
	mosaic_command("srd, High, Unlimited \r");
	/* Report position once per second */
	/* Report time once per second */
	/* Report sat info once per second */
	mosaic_command("sso, Stream1, COM3, ReceiverTime+MeasEpoch+PVTGeodetic+DOP, sec1 \r");
}

#if AO_MOSAIC_DEBUG && !defined(AO_GPS_TEST)
static void mosaic_option(void)
{
	uint8_t r = (uint8_t) ao_cmd_hex();
	if (ao_cmd_status != ao_cmd_success) {
		ao_cmd_status = ao_cmd_success;
		ao_gps_show();
	} else {
		mosaic_dbg_enable = r;
		printf ("gps debug set to %d\n", mosaic_dbg_enable);
	}
}
#else
#define mosaic_option ao_gps_show
#endif

const struct ao_cmds ao_mosaic_cmds[] = {
	{ mosaic_option, 	"g\0Display Mosaic GPS" },
	{ 0, NULL },
};

static struct sbf sbf;

static void
mosaic_read(void *buf, size_t len)
{
	uint8_t *b = buf;
	while (len--)
		*b++ = mosaic_byte();
}

static uint16_t
mosaic_read_crc(void *buf, size_t len, uint16_t crc)
{
	uint8_t *b = buf;

	while (len--) {
		uint8_t byte = mosaic_byte();
		crc = ao_crc16_ccitt(crc, byte);
		*b++ = byte;
	}
	return crc;
}

static struct ao_telemetry_location new_location;
static struct ao_telemetry_satellite new_satellite;

static
int32_t
clip_value(float value, float low, float high)
{
	if (value < low)
		value = low;
	else if (value > high)
		value = high;
	return (int32_t) roundf(value);
}

void
ao_gps_mosaic(void)
{
	AO_TICK_TYPE		packet_start_tick;
	AO_TICK_TYPE		solution_tick = 0;

#ifndef AO_GPS_TEST
	ao_cmd_register(&ao_mosaic_cmds[0]);
#endif
	mosaic_setup();

	for (;;) {
		uint16_t	crc_computed;

		while(mosaic_byte() != '$')
			;
		if (mosaic_byte() != '@')
			continue;
		packet_start_tick = ao_tick_count;

		mosaic_read(&sbf.header.crc, sizeof(sbf.header.crc));
		crc_computed = mosaic_read_crc(&sbf.header.h, sizeof(sbf.header.h), 0);
		if (sbf.header.h.length > sizeof(sbf) + 2) {
			mosaic_dbg(DBG_PROTO, "too long %d > %ld\n",
				   sbf.header.h.length, sizeof(sbf) + 2);
			continue;
		}
		crc_computed = mosaic_read_crc(sbf.data, sbf.header.h.length - 8, crc_computed);
		if (crc_computed != sbf.header.crc) {
			mosaic_dbg(DBG_PROTO, "crc error (computed 0x%04x)\n", crc_computed);
			continue;
		}

		bool gps_ready = false;

		switch (SBF_BLOCK_NUMBER(sbf.header.h.id)) {
		case SBF_RECEIVER_TIME:
			solution_tick = packet_start_tick;
			memset(&new_location, 0, sizeof(new_location));
			memset(&new_satellite, 0, sizeof(new_satellite));
			new_location.year = (uint8_t) sbf.receiver_time.utcyear;
			new_location.month = (uint8_t) sbf.receiver_time.utcmonth;
			new_location.day = (uint8_t) sbf.receiver_time.utcday;
			new_location.hour = (uint8_t) sbf.receiver_time.utchour;
			new_location.minute = (uint8_t) sbf.receiver_time.utcmin;
			new_location.second = (uint8_t) sbf.receiver_time.utcsec;

			if (sbf.receiver_time.utcyear != -128 &&
			    sbf.receiver_time.utcmonth != -128 &&
			    sbf.receiver_time.utcday != -128 &&
			    sbf.receiver_time.utchour != -128 &&
			    sbf.receiver_time.utcmin != -128 &&
			    sbf.receiver_time.utcsec != -128)
			{
				new_location.flags |= AO_GPS_DATE_VALID;
			}

			mosaic_dbg(DBG_PROTO, "ReceiverTime year %d month %d day %d hour %d min %d sec %d\n",
				   sbf.receiver_time.utcyear,
				   sbf.receiver_time.utcmonth,
				   sbf.receiver_time.utcday,
				   sbf.receiver_time.utchour,
				   sbf.receiver_time.utcmin,
				   sbf.receiver_time.utcsec);
			break;
		case SBF_MEAS_EPOCH:
			mosaic_dbg(DBG_PROTO, "MeasEpoch sb1len %d sb2len %d\n",
				   sbf.meas_epoch.sb1length,
				   sbf.meas_epoch.sb2length);
			{
				uint8_t	i1, i2;
				uint8_t	nsat = 0, nsol = 0;
				struct sbf_meas_epoch_channel_type1 *type1;
				struct sbf_meas_epoch_channel_type2 *type2;

				type1 = (void *) (&sbf.meas_epoch + 1);
				for (i1 = 0; i1 < sbf.meas_epoch.n1; i1++) {
					uint8_t signal_type = type1->type & 0x1f;
					uint8_t cn0;

					if (signal_type == 1 || signal_type == 2)
						cn0 = type1->cn0 / 4;
					else
						cn0 = type1->cn0 / 4 + 10;
					if (nsat < AO_TELEMETRY_SATELLITE_MAX_SAT) {
						new_satellite.sats[nsat].svid = type1->svid;
						new_satellite.sats[nsat].c_n_1 = cn0;
						new_satellite.channels = nsat + 1;
					}
					nsat++;
					if (type1->locktime != 65535)
						nsol++;
					mosaic_dbg(DBG_PROTO, "  Type1 type 0x%x channel %d svid %d cn0 %d locktime %u\n",
						   type1->type, type1->rxchannel, type1->svid, cn0, type1->locktime);
					type2 = (void *) ((uint8_t *) type1 + sbf.meas_epoch.sb1length);
					for (i2 = 0; i2 < type1->n2; i2++) {
						mosaic_dbg(DBG_PROTO, "    Type2 type %d cn0 %d\n",
							   type2->type, type2->cn0);
						type2 = (void *) ((uint8_t *) type2 + sbf.meas_epoch.sb1length);
					}
					type1 = (void *) type2;
				}
				if (nsol > 15)
					nsol = 15;
				new_location.flags |= nsol;
			}
			break;
		case SBF_PVT_GEODETIC:
			mosaic_dbg(DBG_PROTO, "PVTGeodetic mode 0x%02x error %d lat %f lon %f height %f\n",
				   sbf.geodetic.mode,
				   sbf.geodetic.error,
				   sbf.geodetic.latitude * 180.0/M_PI,
				   sbf.geodetic.longitude * 180.0/M_PI,
				   sbf.geodetic.height);

			/* Need to use double here to preserve all of the available precision */
			new_location.latitude = (int32_t) round(sbf.geodetic.latitude * (1e7 * 180.0/M_PI));
			new_location.longitude = (int32_t) round(sbf.geodetic.longitude * (1e7 * 180.0f/M_PI));
			gps_alt_t altitude = (gps_alt_t) round(sbf.geodetic.height);
			AO_TELEMETRY_LOCATION_SET_ALTITUDE(&new_location, altitude);
			if (sbf.geodetic.latitude != -2e10 &&
			    sbf.geodetic.longitude != -2e10 &&
			    sbf.geodetic.height != -2e10)
			{
				new_location.flags |= AO_GPS_VALID;
			}
			if (sbf.geodetic.vn != -2e10 &&
			    sbf.geodetic.ve != -2e10 &&
			    sbf.geodetic.vu != -2e10)
			{
				new_location.flags |= AO_GPS_COURSE_VALID;
			}
			float ground_speed = hypotf((float) sbf.geodetic.vn, (float) sbf.geodetic.ve);
			float course = atan2f((float) sbf.geodetic.ve, (float) sbf.geodetic.vn);
			new_location.ground_speed = (uint16_t) clip_value(ground_speed, 0, 65535.0f);
			new_location.climb_rate = (int16_t) clip_value(sbf.geodetic.vu * 100, -32768.0f, 32767.0f);
			new_location.course = (uint8_t) clip_value(course * (90.0f/(float)M_PI), 0.0f, 255.0f);
			break;
		case SBF_DOP:
			mosaic_dbg(DBG_PROTO, "DOP pdop%d tdop %d hdop %d vdop %d\n",
				   sbf.dop.pdop,
				   sbf.dop.tdop,
				   sbf.dop.hdop,
				   sbf.dop.vdop);
			new_location.pdop = (uint8_t) (sbf.dop.pdop / 10);
			new_location.hdop = (uint8_t) (sbf.dop.hdop / 10);
			new_location.vdop = (uint8_t) (sbf.dop.vdop / 10);
			gps_ready = true;
			break;
		default:
			mosaic_dbg(DBG_PROTO, "block %d revision %d length %d avail %ld\n",
				   SBF_BLOCK_NUMBER(sbf.header.h.id),
				   SBF_BLOCK_REVISION(sbf.header.h.id),
				   sbf.header.h.length,
				   sizeof(sbf));
			break;
		}

		if (!gps_ready)
			continue;

		new_location.flags |= AO_GPS_RUNNING;

		ao_mutex_get(&ao_gps_mutex);
		ao_gps_data = new_location;
		ao_gps_tracking_data = new_satellite;
		ao_gps_tick = solution_tick;

		ao_mutex_put(&ao_gps_mutex);
		ao_gps_new = AO_GPS_NEW_DATA | AO_GPS_NEW_TRACKING;
		ao_wakeup(&ao_gps_new);
	}
}

#endif
