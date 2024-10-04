/*
 * Copyright © 2013 Keith Packard <keithp@keithp.com>
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
#include <ao_log.h>
#include <ao_exti.h>
#include <ao_tracker.h>

#define AO_FAIL_FLASH	1
#define AO_FAIL_ADC	2
#define AO_FAIL_GPS	3

static void
ao_validate(void)
{
	static struct ao_telemetry_location	gps_data;
	static struct ao_telemetry_satellite	gps_tracking_data;
	uint8_t new;
	uint8_t data;
	int16_t	decivolt;
	AO_TICK_TYPE	gps_start;

	/* Check the flash part */
	ao_storage_setup();
	if (ao_storage_total != 2 * 1024 * 1024)
		ao_panic(AO_FAIL_FLASH);

	/* Check the battery voltage */
	data = ao_data_head;
	do {
		ao_sleep((void *) &ao_data_head);
	} while (ao_data_head == data);
	decivolt = ao_battery_decivolt(ao_data_ring[data].adc.v_batt);
	if (decivolt < 35 && 55 < decivolt)
		ao_panic(AO_FAIL_ADC);

	/* Check to make sure GPS data is being received */
	gps_start = ao_time();
	for (;;) {
		while ((new = ao_gps_new) == 0)
			ao_sleep_for(&ao_gps_new, AO_SEC_TO_TICKS(1));
		ao_mutex_get(&ao_gps_mutex);
		if (new & AO_GPS_NEW_DATA)
			memcpy(&gps_data, &ao_gps_data, sizeof (ao_gps_data));
		if (new & AO_GPS_NEW_TRACKING)
			memcpy(&gps_tracking_data, &ao_gps_tracking_data, sizeof (ao_gps_tracking_data));
		ao_gps_new = 0;
		ao_mutex_put(&ao_gps_mutex);

		if (new & AO_GPS_NEW_DATA) {
			if (gps_data.flags & AO_GPS_RUNNING)
				break;
		}
		if ((AO_TICK_SIGNED) (ao_time() - gps_start) > (AO_TICK_SIGNED) AO_SEC_TO_TICKS(10))
			ao_panic(AO_FAIL_GPS);
	}
	ao_led_on(LEDS_AVAILABLE);
	ao_exit();
}

struct ao_task ao_validate_task;

int
main(void)
{
	ao_clock_init();
	ao_task_init();
	ao_cmd_init();
	ao_config_init();

	ao_led_init();
	ao_led_off(LEDS_AVAILABLE);

	/* internal systems */
	ao_timer_init();
	ao_dma_init();
	ao_exti_init();

	/* SoC hardware */
	ao_adc_init();
	ao_serial_init();
	ao_spi_init();
	ao_usb_init();

	/* External hardware */
	ao_storage_init();
	ao_radio_init();
	ao_gps_init();


	ao_add_task(&ao_validate_task, ao_validate, "validate");

	ao_start_scheduler();
}
