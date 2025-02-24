/*
 * Copyright © 2015 Keith Packard <keithp@keithp.com>
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
#include <ao_exti.h>
#include <ao_packet.h>
#include <ao_send_packet.h>
#include <ao_usb.h>
#include <ao_rn4678.h>

static void
ao_validate(void)
{
	uint8_t data;
	int16_t	decivolt;

	ao_config_get();

	/* Check the battery voltage */
	data = ao_data_head;
	do {
		ao_sleep((void *) &ao_data_head);
	} while (ao_data_head == data);
	decivolt = ao_battery_decivolt(ao_data_ring[data].adc.v_batt);
	if (decivolt < 35 || 55 < decivolt)
		ao_panic(AO_FAIL_ADC);

	if (!ao_radio_post())
		ao_panic(AO_FAIL_RADIO);

	/*
	 * Wait for BT module to come online. That will panic if it
	 * doesn't work
	 */
	for (;;) {
		if (ao_num_stdios == 2)
			break;
		ao_delay(AO_SEC_TO_TICKS(1));
	}

	ao_led_on(AO_LED_BLUE);

	ao_exit();
}

struct ao_task ao_validate_task;

int
main(void)
{
	ao_clock_init();

	ao_task_init();
	ao_led_init();
	ao_led_off(LEDS_AVAILABLE);
	ao_timer_init();

	ao_spi_init();
	ao_dma_init();
	ao_exti_init();

	ao_adc_init();
	ao_cmd_init();
//	ao_send_packet_init();

	ao_usb_init();
	ao_serial_init();

	ao_radio_init();
//	ao_packet_master_init();
//	ao_monitor_init();
	ao_rn4678_init();

	ao_config_init();

	ao_add_task(&ao_validate_task, ao_validate, "validate");

	ao_start_scheduler();
	return 0;
}
