/*
 * Copyright © 2021 Keith Packard <keithp@keithp.com>
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
#include <ao_bmi088.h>
#include <ao_mmc5983.h>
#include <ao_adxl375.h>
#include <ao_log.h>
#include <ao_exti.h>
#include <ao_packet.h>
#include <ao_companion.h>
#include <ao_profile.h>
#include <ao_eeprom.h>
#include <ao_i2c_bit.h>
#if HAS_SAMPLE_PROFILE
#include <ao_sample_profile.h>
#endif
#include <ao_pyro.h>
#if HAS_STACK_GUARD
#include <ao_mpu.h>
#endif
#include <ao_pwm.h>

#define AO_FAIL_FLASH	1
#define AO_FAIL_ADC	2
#define AO_FAIL_GPS	3
#define AO_FAIL_RADIO	4
#define AO_FAIL_ACCEL	5
#define AO_FAIL_BARO	6
#define AO_FAIL_IMU	7
#define AO_FAIL_MAG	8
#define AO_FAIL_RANGE	9

static void
ao_validate(void)
{
	static struct ao_telemetry_location	gps_data;
	static struct ao_telemetry_satellite	gps_tracking_data;
	uint8_t new;
	uint8_t data;
	int16_t	decivolt;
	AO_TICK_TYPE	gps_start;

	ao_config_get();
	/* Check the flash part */
	ao_storage_setup();
	if (ao_storage_total != 8 * 1024 * 1024)
		ao_panic(AO_FAIL_FLASH);

	/* Check the battery voltage */
	data = ao_data_head;
	do {
		ao_sleep((void *) &ao_data_head);
	} while (ao_data_head == data);
	decivolt = ao_battery_decivolt(ao_data_ring[data].adc.v_batt);
	if (decivolt < 35 || 55 < decivolt)
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

	if (!ao_radio_post())
		ao_panic(AO_FAIL_RADIO);

	ao_sample_init();
	ao_flight_state = ao_flight_startup;
	for (;;) {

		/*
		 * Process ADC samples, just looping
		 * until the sensors are calibrated.
		 */
		if (ao_sample())
			break;
	}

	if (ao_sensor_errors & AO_DATA_MS5607)
		ao_panic(AO_FAIL_BARO);
	if (ao_sensor_errors & AO_DATA_BMI088)
		ao_panic(AO_FAIL_IMU);
	if (ao_sensor_errors & AO_DATA_MMC5983)
		ao_panic(AO_FAIL_MAG);
	if (ao_sensor_errors & AO_DATA_ADXL375)
		ao_panic(AO_FAIL_ACCEL);
	if (ao_sensor_errors)
		ao_panic(AO_FAIL_RANGE);

	/* Check ground accel value to make sure it's somewhat valid */
	if (ao_ground_accel < (accel_t) AO_CONFIG_DEFAULT_ACCEL_PLUS_G - ACCEL_NOSE_UP ||
	    ao_ground_accel > (accel_t) AO_CONFIG_DEFAULT_ACCEL_MINUS_G + ACCEL_NOSE_UP) {
		ao_panic(AO_FAIL_ACCEL);
	}
	if (ao_ground_height < -1000 || ao_ground_height > 7000)
		ao_panic(AO_FAIL_BARO);

	ao_led_on(AO_LED_GREEN);
	ao_beep_for(AO_BEEP_MID_DEFAULT, AO_MS_TO_TICKS(100));

	ao_exit();
}

struct ao_task ao_validate_task;

int
main(void)
{
	ao_clock_init();

#if HAS_STACK_GUARD
	ao_mpu_init();
#endif

	ao_task_init();
	ao_serial_init();
	ao_led_init();
	ao_led_off(LEDS_AVAILABLE);
	ao_timer_init();

	ao_spi_init();
#ifdef MMC5983_I2C
	ao_i2c_bit_init();
#endif
	ao_dma_init();
	ao_exti_init();

	ao_adc_init();
#if HAS_BEEP
	ao_beep_init();
#endif
	ao_cmd_init();

	ao_ms5607_init();
	ao_bmi088_init();
	ao_mmc5983_init();
	ao_adxl375_init();

	ao_eeprom_init();
	ao_storage_init();

	ao_usb_init();
	ao_gps_init();

	ao_radio_init();
	ao_igniter_init();
	ao_pyro_init();

	ao_config_init();
#if AO_PROFILE
	ao_profile_init();
#endif
#if HAS_SAMPLE_PROFILE
	ao_sample_profile_init();
#endif

	ao_pwm_init();

	ao_add_task(&ao_validate_task, ao_validate, "validate");

	ao_start_scheduler();
	return 0;
}
