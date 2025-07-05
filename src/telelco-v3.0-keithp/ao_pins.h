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

#ifndef _AO_PINS_H_
#define _AO_PINS_H_

/* 16MHz crystal */

#define AO_HSE		1
#define AO_HSE_BYPASS	0

#define AO_SYSCLK	72000000
#define AO_HCLK		72000000
#define AO_APB1CLK	36000000
#define AO_APB2CLK	72000000
#define AO_ADCCLK	12000000

/* PLLMUL is 9, PLLXTPRE (pre divider) is 2, so the
 * overall PLLCLK is 16 * 9/2 = 72MHz (used as SYSCLK)
 *
 * HCLK is SYSCLK / 1 (HPRE_DIV) = 72MHz (72MHz max)
 * USB is PLLCLK / 1.5 (USBPRE)= 48MHz (must be 48MHz)
 * APB2 is HCLK / 1 (PPRE2_DIV) = 72MHz (72MHz max)
 * APB1 is HCLK / 2 (PPRE1_DIV) = 36MHz (36MHz max)
 * ADC is APB2 / 6 (ADCPRE) = 12MHz (14MHz max)
 */

#define AO_RCC_CFGR_USBPRE	STM_RCC_CFGR_USBPRE_1_5
#define AO_RCC_CFGR_PLLMUL	STM_RCC_CFGR_PLLMUL_9
#define AO_RCC_CFGR_PLLXTPRE	STM_RCC_CFGR_PLLXTPRE_2
#define AO_RCC_CFGR_PPRE2_DIV	STM_RCC_CFGR_PPRE2_DIV_1
#define AO_RCC_CFGR_PPRE1_DIV	STM_RCC_CFGR_PPRE1_DIV_2
#define AO_RCC_CFGR_HPRE_DIV	STM_RCC_CFGR_HPRE_DIV_1
#define AO_RCC_CFGR_ADCPRE	STM_RCC_CFGR_ADCPRE_6


#define AO_CC1200_FOSC		40000000

#define HAS_EEPROM		1
#define USE_INTERNAL_FLASH	1
#define USE_EEPROM_CONFIG	1
#define USE_STORAGE_CONFIG	0
#define HAS_USB			1
#define HAS_BEEP		1
#define BEEPER_TIMER		3
#define BEEPER_CHANNEL		1
#define BEEPER_PORT		(&stm_gpioc)
#define BEEPER_PIN		6
#define AO_BEEP_MID_DEFAULT	179	/* 2100 Hz */
#define HAS_RADIO		1
#define HAS_RADIO_RATE		1
#define HAS_TELEMETRY		0
#define HAS_AES			1
#define HAS_STATIC_TEST		0

#define HAS_USB_PULLUP		1
#define AO_USB_PULLUP_PORT	(&stm_gpioa)
#define AO_USB_PULLUP_PIN	10

#define HAS_SPI_1		1	/* NHD-C12864LZ LCD Module */
#define SPI_1_PA5_PA6_PA7	1
#define SPI_1_PA6_DISABLE	1
#define SPI_1_MODE_OUTPUT	STM_GPIO_CR_MODE_OUTPUT_50MHZ
#define SPI_1_PB3_PB4_PB5	0
#define SPI_1_PE13_PE14_PE15	0

#define HAS_SPI_2		1	/* CC1200 */
#define SPI_2_PB13_PB14_PB15	1
#define SPI_2_PD1_PD3_PD4	0
#define SPI_2_GPIO		(&stm_gpiod)
#define SPI_2_SCK		1
#define SPI_2_MISO		3
#define SPI_2_MOSI		4
#define SPI_2_MODE_OUTPUT	STM_GPIO_CR_MODE_OUTPUT_10MHZ

#define HAS_I2C_1		0

#define HAS_I2C_2		0

#define PACKET_HAS_SLAVE	0
#define PACKET_HAS_MASTER	0

#define AO_FAST_TIMER		4
#define FAST_TIMER_FREQ		10000	/* .1ms for debouncing */

/* LCD module */
#define AO_ST7565_CS_PORT	(&stm_gpioc)	/* pin 1 */
#define AO_ST7565_CS_PIN	4
#define AO_ST7565_RESET_PORT	(&stm_gpioc)	/* pin 2 */
#define AO_ST7565_RESET_PIN	5
#define AO_ST7565_A0_PORT	(&stm_gpioa)	/* pin 3 */
#define AO_ST7565_A0_PIN	3
#define AO_ST7565_SPI_BUS	(AO_SPI_1_PA5_PA6_PA7 | AO_SPI_MODE_3)
#define AO_ST7565_WIDTH		128
#define AO_ST7565_HEIGHT	64
#define AO_ST7565_BIAS		ST7565_LCD_BIAS_1_9

/*
 * Radio is a cc1200 connected via SPI
 */

#define AO_RADIO_CAL_DEFAULT 	5695733

#define AO_CC1200_SPI_CS_PORT	(&stm_gpiob)
#define AO_CC1200_SPI_CS_PIN	8
#define AO_CC1200_SPI_BUS	AO_SPI_2_PB13_PB14_PB15
#define AO_CC1200_SPI		stm_spi2

#define AO_CC1200_INT_PORT	(&stm_gpiob)
#define AO_CC1200_INT_PIN	(9)

#define AO_CC1200_INT_GPIO	2
#define AO_CC1200_INT_GPIO_IOCFG	CC1200_IOCFG2

#define LOW_LEVEL_DEBUG		0

#define HAS_LED			1

#define AO_LED_RED		AO_LED_0	/* PC7 */
#define LED_0_PORT		(&stm_gpioc)
#define LED_0_PIN		7

#define AO_LED_AMBER		AO_LED_1	/* PC8 */
#define LED_1_PORT		(&stm_gpioc)
#define LED_1_PIN		8

#define AO_LED_GREEN		AO_LED_2	/* PC9 */
#define LED_2_PORT		(&stm_gpioc)
#define LED_2_PIN		9

#define AO_LED_BOX		AO_LED_3	/* PA9 */
#define LED_3_PORT		(&stm_gpioa)
#define LED_3_PIN		9

#define AO_LED_PAD		AO_LED_4	/* PA15 */
#define LED_4_PORT		(&stm_gpioa)
#define LED_4_PIN		15

#define AO_LED_DRAG		AO_LED_5	/* PC12 */
#define LED_5_PORT		(&stm_gpioc)
#define LED_5_PIN		12

#define AO_LED_CONTINUITY_7	AO_LED_6	/* PC13 */
#define LED_6_PORT		(&stm_gpioc)
#define LED_6_PIN		13

#define AO_LED_CONTINUITY_6	AO_LED_7	/* PC14 */
#define LED_7_PORT		(&stm_gpioc)
#define LED_7_PIN		14

#define AO_LED_CONTINUITY_5	AO_LED_8	/* PC15 */
#define LED_8_PORT		(&stm_gpioc)
#define LED_8_PIN		15

#define AO_LED_CONTINUITY_4	AO_LED_9	/* PC2 */
#define LED_9_PORT		(&stm_gpioc)
#define LED_9_PIN		2

#define AO_LED_CONTINUITY_3	AO_LED_10	/* PC3 */
#define LED_10_PORT		(&stm_gpioc)
#define LED_10_PIN		3

#define AO_LED_CONTINUITY_2	AO_LED_11	/* PA0 */
#define LED_11_PORT		(&stm_gpioa)
#define LED_11_PIN		0

#define AO_LED_CONTINUITY_1	AO_LED_12	/* PA6 */
#define LED_12_PORT		(&stm_gpioa)
#define LED_12_PIN		6

#define AO_LED_CONTINUITY_0	AO_LED_13	/* PB1 */
#define LED_13_PORT		(&stm_gpiob)
#define LED_13_PIN		1

#define AO_LED_CONTINUITY_NUM	8

#define AO_LED_REMOTE_ARM	AO_LED_14	/* PB3 */
#define LED_14_PORT		(&stm_gpiob)
#define LED_14_PIN		3

#define AO_LED_FIRE		AO_LED_15	/* PB0 */
#define LED_15_PORT		(&stm_gpiob)
#define LED_15_PIN		0

/*
 * Use event queue for input devices
 */

#define AO_EVENT		1

/*
 * Knobs
 */

#define AO_QUADRATURE_COUNT	1
#define AO_QUADRATURE_DEBOUNCE	0
#define AO_QUADRATURE_SINGLE_CODE	1

#define AO_QUADRATURE_0_PORT	&stm_gpiob
#define AO_QUADRATURE_0_A	12
#define AO_QUADRATURE_0_B	11

#define AO_QUADRATURE_SELECT	0

/*
 * Buttons
 */

#define AO_BUTTON_COUNT		9
#define AO_BUTTON_MODE		AO_EXTI_MODE_PULL_UP

#define AO_BUTTON_DRAG_MODE	0
#define AO_BUTTON_0_PORT	&stm_gpioc
#define AO_BUTTON_0		1

#define AO_BUTTON_DRAG_SELECT	1
#define AO_BUTTON_1_PORT	&stm_gpioc
#define AO_BUTTON_1		0

#define AO_BUTTON_SPARE1       	2
#define AO_BUTTON_2_PORT	&stm_gpiob
#define AO_BUTTON_2		4

#define AO_BUTTON_SPARE2      	3
#define AO_BUTTON_3_PORT	&stm_gpiob
#define AO_BUTTON_3		5

#define AO_BUTTON_SPARE3       	4
#define AO_BUTTON_4_PORT	&stm_gpiob
#define AO_BUTTON_4		6

#define AO_BUTTON_ARM		5
#define AO_BUTTON_5_PORT	&stm_gpioa
#define AO_BUTTON_5		8

#define AO_BUTTON_FIRE		6
#define AO_BUTTON_6_PORT	&stm_gpioa
#define AO_BUTTON_6		4

#define AO_BUTTON_SPARE4	7
#define AO_BUTTON_7_PORT	&stm_gpiob
#define AO_BUTTON_7		7

#define AO_BUTTON_ENCODER_SELECT	8
#define AO_BUTTON_8_PORT	&stm_gpiob
#define AO_BUTTON_8		10

/* ADC */

struct ao_adc {
	int16_t		v_batt;
};

#define AO_ADC_DUMP(p) \
	printf("batt: %5d\n", (p)->v_batt)

#define HAS_ADC_SINGLE		1
#define HAS_ADC_TEMP		0
#define HAS_BATTERY_REPORT	1

#define AO_ADC_V_BATT		2
#define AO_ADC_V_BATT_PORT	(&stm_gpioa)
#define AO_ADC_V_BATT_PIN	2

#define AO_ADC_PIN0_PORT	AO_ADC_V_BATT_PORT
#define AO_ADC_PIN0_PIN		AO_ADC_V_BATT_PIN

#define AO_ADC_SQ1		AO_ADC_V_BATT

#define AO_NUM_ADC		1

/*
 * Voltage divider on ADC battery sampler
 */
#define AO_BATTERY_DIV_PLUS	15	/* 15k */
#define AO_BATTERY_DIV_MINUS	27	/* 27k */

/*
 * ADC reference in decivolts
 */
#define AO_ADC_REFERENCE_DV	33

#define AO_LCO_SEARCH_API

//#define AO_LCO_HAS_CONTRAST	1
//#define AO_LCO_MIN_CONTRAST	0
//#define AO_LCO_MAX_CONTRAST	63
//#define AO_LCO_CONTRAST_STEP	1

#define AO_LCO_HAS_BACKLIGHT_UI	1
#define AO_LCO_HAS_BACKLIGHT	1
#define AO_LCO_MIN_BACKLIGHT	0
#define AO_LCO_MAX_BACKLIGHT	65535
#define AO_LCO_BACKLIGHT_STEP	771

#define AO_LCO_HAS_LCO_INFO	1
#define AO_LCO_MIN_INFO_PAGE	0
#define AO_LCO_MAX_INFO_PAGE	0

#define AO_LCO_HAS_PAD_INFO	1

/*
 * LCD Backlight via PWM.
 *
 * Pin PA1, TIM2_CH2
 */

#define NUM_PWM			1
#define PWM_MAX			65535
#define AO_PWM_TIMER		stm_tim2
#define AO_LCD_BL_PWM_CHAN	1
#define AO_PWM_0_GPIO		(&stm_gpioa)
#define AO_PWM_0_PIN		1
#define AO_PWM_TIMER_ENABLE	STM_RCC_APB1ENR_TIM2EN
#define AO_PWM_TIMER_SCALE	1

#define AO_AFIO_PWM_REMAP	STM_AFIO_MAPR_TIM2_REMAP
#define AO_AFIO_PWM_REMAP_VAL	STM_AFIO_MAPR_TIM2_REMAP_PA0_PA1_PA2_PA3
#define AO_AFIO_PWM_REMAP_MASK	STM_AFIO_MAPR_TIM2_REMAP_MASK


#endif /* _AO_PINS_H_ */
