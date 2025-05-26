/*
 * Copyright © 2022 Keith Packard <keithp@keithp.com>
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

#ifndef _AO_PINS_H_
#define _AO_PINS_H_

#define AO_XOSC			1
#define AO_XOSC_FREQ		16000000
#define AO_XOSC_DIV		256
#define AO_XOSC_MUL		768

#define AO_AHB_PRESCALER	1
#define AO_APBA_PRESCALER	1

#define HAS_SERIAL_1		1
#define USE_SERIAL_1_STDIN	0

#define AO_CONFIG_DEFAULT_FLIGHT_LOG_MAX	(512 * 1024)
#define AO_CONFIG_DEFAULT_ACCEL_PLUS_G		-20
#define AO_CONFIG_DEFAULT_ACCEL_MINUS_G		20
#define AO_CONFIG_MAX_SIZE			1024
#define LOG_ERASE_MARK				0x55
#define LOG_MAX_ERASE				128
#define AO_LOG_FORMAT				AO_LOG_FORMAT_TELEMETRUM

#define HAS_EEPROM		1
#define USE_INTERNAL_FLASH	0
#define USE_EEPROM_CONFIG	0
#define USE_STORAGE_CONFIG	1
#define HAS_USB			1
#define USE_USB_STDIO	1
#define HAS_BATTERY_REPORT	1
#define BEEPER_CHANNEL		4
#define BEEPER_TIMER		3
#define BEEPER_PORT		(&samd21_port_a)
#define BEEPER_PIN		16
#define HAS_RADIO		1
#define HAS_RADIO_10MW		1
#define HAS_TELEMETRY		1
#define HAS_APRS		1
#define HAS_COMPANION		1

#define HAS_SPI_0		1
#define HAS_SPI_3		1
#define HAS_SPI_5		1

#define PACKET_HAS_SLAVE	1
#define PACKET_HAS_MASTER	0

#define LOW_LEVEL_DEBUG		0

#define HAS_LED			1
#define LED_0_PORT		(&samd21_port_b)
#define LED_0_PIN		10
#define LED_1_PORT		(&samd21_port_b)
#define LED_1_PIN		11
#define AO_LED_RED		(1 << 0)
#define AO_LED_GREEN		(1 << 1)

#define HAS_GPS			1
#define HAS_FLIGHT		1
#define HAS_ADC			1
#define HAS_ADC_TEMP		1
#define HAS_LOG			1

/*
 * Beeper
 */

#define HAS_BEEP		1
/* Beep on PA16 function E TCC2.0 */

#define AO_BEEP_TCC		(&samd21_tcc2)
#define AO_BEEP_TCC_APBC_MASK	SAMD21_PM_APBCMASK_TCC2
#define AO_BEEP_PORT		(&samd21_port_a)
#define AO_BEEP_PIN		(16)
#define AO_BEEP_FUNC		SAMD21_PORT_PMUX_FUNC_E

/*
 * Igniter
 */

#define HAS_IGNITE		1
#define HAS_IGNITE_REPORT	1

#define AO_SENSE_DROGUE(p)	((p)->adc.sense_a)
#define AO_SENSE_MAIN(p)	((p)->adc.sense_m)
#define AO_IGNITER_CLOSED	400
#define AO_IGNITER_OPEN		60

/* Drogue */
#define AO_IGNITER_DROGUE_PORT	(&samd21_port_a)
#define AO_IGNITER_DROGUE_PIN	19

/* Main */
#define AO_IGNITER_MAIN_PORT	(&samd21_port_a)
#define AO_IGNITER_MAIN_PIN	18

/*
 * ADC
 */
#define AO_DATA_RING		32
#define AO_ADC_NUM_SENSE	2

struct ao_adc {
	int16_t			sense_a;
	int16_t			sense_m;
	int16_t			v_batt;
	int16_t			temp;
};

#define AO_ADC_DUMP(p) \
	printf("tick: %5lu drogue: %5d main: %5d batt: %5d\n", \
	       (p)->tick, \
	       (p)->adc.sense_a, (p)->adc.sense_m, \
	       (p)->adc.v_batt);

#define AO_ADC_SENSE_DROGUE		18
#define AO_ADC_SENSE_DROGUE_PORT	(&samd21_port_a)
#define AO_ADC_SENSE_DROGUE_PIN		10

#define AO_ADC_SENSE_MAIN		19
#define AO_ADC_SENSE_MAIN_PORT		(&samd21_port_a)
#define AO_ADC_SENSE_MAIN_PIN		11

#define AO_ADC_V_BATT			17
#define AO_ADC_V_BATT_PORT		(&samd21_port_a)
#define AO_ADC_V_BATT_PIN		9

#define AO_ADC_TEMP			SAMD21_ADC_INPUTCTRL_MUXPOS_TEMP

#define AO_NUM_ADC_PIN			3

#define AO_ADC_PIN0_PORT	AO_ADC_SENSE_DROGUE_PORT
#define AO_ADC_PIN0_PIN		AO_ADC_SENSE_DROGUE_PIN
#define AO_ADC_PIN1_PORT	AO_ADC_SENSE_MAIN_PORT
#define AO_ADC_PIN1_PIN		AO_ADC_SENSE_MAIN_PIN
#define AO_ADC_PIN2_PORT	AO_ADC_V_BATT_PORT
#define AO_ADC_PIN2_PIN		AO_ADC_V_BATT_PIN

#define AO_NUM_ADC	       	(AO_NUM_ADC_PIN + 1)

#define AO_ADC_SQ0		AO_ADC_SENSE_DROGUE
#define AO_ADC_SQ1		AO_ADC_SENSE_MAIN
#define AO_ADC_SQ2		AO_ADC_V_BATT
#define AO_ADC_SQ3		AO_ADC_TEMP

/*
 * Voltage divider on ADC battery sampler
 */
#define AO_BATTERY_DIV_PLUS	56	/* 5.6k */
#define AO_BATTERY_DIV_MINUS	100	/* 10k */

/*
 * Voltage divider on ADC igniter samplers
 */
#define AO_IGNITE_DIV_PLUS	100	/* 100k */
#define AO_IGNITE_DIV_MINUS	27	/* 27k */

/*
 * ADC reference in decivolts
 */
#define AO_ADC_REFERENCE_DV	33

/*
 * GPS
 */

#define AO_SERIAL_SPEED_UBLOX	AO_SERIAL_SPEED_9600
#define AO_UBLOX_VERSION	10

#define HAS_SERIAL_1		1
#define USE_SERIAL_1_STDIN	0
#define SERIAL_1_PA00_PA01	1

#define ao_gps_getchar		ao_serial1_getchar
#define ao_gps_putchar		ao_serial1_putchar
#define ao_gps_set_speed	ao_serial1_set_speed
#define ao_gps_fifo		(ao_samd21_usart1.rx_fifo)

/*
 * Pressure sensor settings
 */
#define HAS_MS5607		1
#define HAS_MS5611		0
#define AO_MS5607_PRIVATE_PINS	0
#define AO_MS5607_CS_PORT	(&samd21_port_a)
#define AO_MS5607_CS_PIN	21
#define AO_MS5607_MISO_PORT	(&samd21_port_a)
#define AO_MS5607_MISO_PIN	20
#define AO_MS5607_SPI_INDEX	AO_SPI_3_PA22_PA23_PA20

/*
 * SPI Flash memory
 */

#define M25_MAX_CHIPS		1
#define AO_M25_SPI_CS_PORT	(&samd21_port_a)
#define AO_M25_SPI_CS_MASK	(1 << 27)
#define AO_M25_SPI_BUS		AO_SPI_5_PB22_PB23_PB03


/*
 * Radio (cc1200)
 */

/* gets pretty close to 434.550 */

#define AO_RADIO_CAL_DEFAULT 	5695733

#define AO_CC1200_SPI_CS_PORT	(&samd21_port_a)
#define AO_CC1200_SPI_CS_PIN	7
#define AO_CC1200_SPI_BUS	AO_SPI_5_PB22_PB23_PB03

#define AO_CC1200_INT_PORT		(&samd21_port_b)
#define AO_CC1200_INT_PIN		(8)
#define AO_CC1200_MCU_WAKEUP_PORT	(&samd21_port_b)
#define AO_CC1200_MCU_WAKEUP_PIN	(9)

#define AO_CC1200_INT_GPIO	2
#define AO_CC1200_INT_GPIO_IOCFG	CC1200_IOCFG2

#define AO_CC1200_MARC_GPIO	3
#define AO_CC1200_MARC_GPIO_IOCFG	CC1200_IOCFG3

#define HAS_BOOT_RADIO		0

#define HAS_HIGHG_ACCEL		1

/* ADXL375 */

#define HAS_ADXL375		1
#define AO_ADXL375_CS_PORT	(&samd21_port_a)
#define AO_ADXL375_CS_PIN	8
#define AO_ADXL375_SPI_INDEX	(AO_SPI_0_PA04_PA05_PA06 | AO_SPI_MODE_3)

#define AO_ADXL375_AXIS		x
#define AO_ADXL375_INVERT	1

#define NUM_CMDS		16

/*
 * Companion
 */

#define AO_COMPANION_CS_PORT	(&samd21_port_a)
#define AO_COMPANION_CS_PIN	(13)
#define AO_COMPANION_SPI_BUS	AO_SPI_5_PB22_PB23_PB03

/*
 * Monitor
 */

#define HAS_MONITOR		0
#define LEGACY_MONITOR		0
#define HAS_MONITOR_PUT		1
#define AO_MONITOR_LED		0
#define HAS_RSSI		0

/*
 * Profiling Viterbi decoding
 */

#ifndef AO_PROFILE
#define AO_PROFILE	       	0
#endif

#endif /* _AO_PINS_H_ */
