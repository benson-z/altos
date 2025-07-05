/*
 * Copyright © 2018 Bdale Garbee <bdale@gag.com>
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

#include <ao_flash_stm_pins.h>

/* Companion port cs_companion0 PC10 */

#define AO_BOOT_PIN		1
#define AO_BOOT_APPLICATION_GPIO	stm_gpioc
#define AO_BOOT_APPLICATION_PIN		10
#define AO_BOOT_APPLICATION_VALUE	1
#define AO_BOOT_APPLICATION_MODE	AO_EXTI_MODE_PULL_UP

#define HAS_USB_PULLUP	1
#define AO_USB_PULLUP_PORT	(&stm_gpioa)
#define AO_USB_PULLUP_PIN	10

#endif /* _AO_PINS_H_ */
