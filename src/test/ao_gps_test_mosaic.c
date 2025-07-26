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

#define AO_GPS_TEST
#define HAS_GPS	1
#define HAS_GPS_MOSAIC 1
#include "ao_host.h"
#include <termios.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define AO_GPS_NUM_SAT_MASK	(0xf << 0)
#define AO_GPS_NUM_SAT_SHIFT	(0)

#define AO_GPS_NEW_DATA		1
#define AO_GPS_NEW_TRACKING	2

#define AO_GPS_VALID		(1 << 4)
#define AO_GPS_RUNNING		(1 << 5)
#define AO_GPS_DATE_VALID	(1 << 6)
#define AO_GPS_COURSE_VALID	(1 << 7)

struct ao_telemetry_location {
	uint8_t			year;
	uint8_t			month;
	uint8_t			day;
	uint8_t			hour;
	uint8_t			minute;
	uint8_t			second;
	uint8_t			flags;
	int32_t			latitude;	/* degrees * 10⁷ */
	int32_t			longitude;	/* degrees * 10⁷ */
	int16_t			altitude_low;	/* m */
	uint16_t		ground_speed;	/* cm/s */
	uint8_t			course;		/* degrees / 2 */
	uint8_t			pdop;		/* * 5 */
	uint8_t			hdop;		/* * 5 */
	uint8_t			vdop;		/* * 5 */
	int16_t			climb_rate;	/* cm/s */
	uint16_t		h_error;	/* m */
	uint16_t		v_error;	/* m */
	int16_t			altitude_high;	/* m */
};

typedef int32_t		gps_alt_t;
#define AO_TELEMETRY_LOCATION_ALTITUDE(l) 	(((gps_alt_t) (l)->altitude_high << 16) | ((l)->altitude_low))
#define AO_GPS_ORIG_ALTITUDE(l)			AO_TELEMETRY_LOCATION_ALTITUDE(l)
#define AO_TELEMETRY_LOCATION_SET_ALTITUDE(l,a) (((l)->altitude_high = (a) >> 16), \
						 ((l)->altitude_low = (a)))

#define UBLOX_SAT_STATE_ACQUIRED		(1 << 0)
#define UBLOX_SAT_STATE_CARRIER_PHASE_VALID	(1 << 1)
#define UBLOX_SAT_BIT_SYNC_COMPLETE		(1 << 2)
#define UBLOX_SAT_SUBFRAME_SYNC_COMPLETE	(1 << 3)
#define UBLOX_SAT_CARRIER_PULLIN_COMPLETE	(1 << 4)
#define UBLOX_SAT_CODE_LOCKED			(1 << 5)
#define UBLOX_SAT_ACQUISITION_FAILED		(1 << 6)
#define UBLOX_SAT_EPHEMERIS_AVAILABLE		(1 << 7)

struct ao_telemetry_satellite_info {
	uint8_t		svid;
	uint8_t		c_n_1;
};

#define AO_TELEMETRY_SATELLITE_MAX_SAT	12

struct ao_telemetry_satellite {
	uint8_t					channels;
	struct ao_telemetry_satellite_info	sats[AO_TELEMETRY_SATELLITE_MAX_SAT];
};

#define ao_gps_orig ao_telemetry_location
#define ao_gps_tracking_orig ao_telemetry_satellite
#define ao_gps_sat_orig ao_telemetry_satellite_info

uint8_t ao_gps_new;
uint8_t ao_gps_mutex;
AO_TICK_TYPE ao_gps_tick;
AO_TICK_TYPE ao_gps_utc_tick;
struct ao_telemetry_location	ao_gps_data;
struct ao_telemetry_satellite	ao_gps_tracking_data;

void
ao_mutex_get(uint8_t *mutex)
{
}

void
ao_mutex_put(uint8_t *mutex)
{
}

static int ao_gps_fd;
static FILE *ao_gps_file;

#if 0
static void
ao_dbg_char(char c)
{
	char	line[128];
	line[0] = '\0';
	if (c < ' ') {
		if (c == '\n')
			sprintf (line, "\n");
		else
			sprintf (line, "\\%02x", ((int) c) & 0xff);
	} else {
		sprintf (line, "%c", c);
	}
	write(1, line, strlen(line));
}
#endif

#include <sys/time.h>

int
get_millis(void)
{
	struct timeval	tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

char
ao_mosaic_getchar(void)
{
	char	c;
	int	i;

	i = getc(ao_gps_file);
	if (i == EOF) {
		perror("getchar");
		exit(1);
	}
	c = i;
	return c;
}

void
ao_mosaic_putchar(char c)
{
	int	i;

	for (;;) {
		i = write(ao_gps_fd, &c, 1);
		if (i == 1)
			break;
		if (i < 0 && (errno == EINTR || errno == EAGAIN))
			continue;
		perror("putchar");
		exit(1);
	}
}

#define AO_SERIAL_SPEED_4800	0
#define AO_SERIAL_SPEED_9600	1
#define AO_SERIAL_SPEED_57600	2
#define AO_SERIAL_SPEED_115200	3

static void
ao_gps_set_speed(uint8_t speed)
{
	int	fd = ao_gps_fd;
	struct termios	termios;

	printf ("\t\tset speed %d\n", speed);
	tcdrain(fd);
	tcgetattr(fd, &termios);
	switch (speed) {
	case AO_SERIAL_SPEED_4800:
		cfsetspeed(&termios, B4800);
		break;
	case AO_SERIAL_SPEED_9600:
		cfsetspeed(&termios, B9600);
		break;
	case AO_SERIAL_SPEED_57600:
		cfsetspeed(&termios, B57600);
		break;
	case AO_SERIAL_SPEED_115200:
		cfsetspeed(&termios, B115200);
		break;
	}
	tcsetattr(fd, TCSAFLUSH, &termios);
	tcflush(fd, TCIFLUSH);
}

#define ao_time() 0

uint8_t	ao_task_minimize_latency;

#define ao_usb_getchar()	0

#include "ao_gps_print.c"
#include "ao_gps_show.c"
#include "ao_gps_mosaic.c"
#include "ao_crc_ccitt.c"

void
ao_dump_state(void *wchan)
{
	if (wchan == &ao_gps_new)
		ao_gps_show();
	return;
}

int
ao_gps_open(const char *tty)
{
	struct termios	termios;
	int fd;

	fd = open (tty, O_RDWR);
	if (fd < 0)
		return -1;

	tcgetattr(fd, &termios);
	cfmakeraw(&termios);
	cfsetspeed(&termios, B4800);
	tcsetattr(fd, TCSAFLUSH, &termios);

	tcdrain(fd);
	tcflush(fd, TCIFLUSH);
	return fd;
}

#include <getopt.h>

static const struct option options[] = {
	{ .name = "tty", .has_arg = 1, .val = 'T' },
	{ 0, 0, 0, 0},
};

static void usage(char *program)
{
	fprintf(stderr, "usage: %s [--tty <tty-name>]\n", program);
	exit(1);
}

int
main (int argc, char **argv)
{
	char	*tty = "/dev/ttyUSB0";
	int	c;

	while ((c = getopt_long(argc, argv, "T:", options, NULL)) != -1) {
		switch (c) {
		case 'T':
			tty = optarg;
			break;
		default:
			usage(argv[0]);
			break;
		}
	}
	ao_gps_fd = ao_gps_open(tty);
	if (ao_gps_fd < 0) {
		perror (tty);
		exit (1);
	}
	ao_gps_file = fdopen(ao_gps_fd, "r");
	mosaic();
	return 0;
}
