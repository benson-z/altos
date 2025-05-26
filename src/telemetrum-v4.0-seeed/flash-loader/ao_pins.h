/*
 * Copyright © 2022 Bdale Garbee <bdale@gag.com>
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

#include <ao_flash_samd21_pins.h>

/* cs_comp_0 (companion port pin 6) to gnd for boot loader mode */

#define AO_BOOT_PIN			1
#define AO_BOOT_APPLICATION_GPIO	(samd21_port_a)
#define AO_BOOT_APPLICATION_PIN		13
#define AO_BOOT_APPLICATION_VALUE	1
#define AO_BOOT_APPLICATION_MODE	AO_MODE_PULL_UP

/* USB */
#define HAS_USB			1

#define AO_XOSC			1
#define AO_XOSC_FREQ		16000000
#define AO_XOSC_DIV		256
#define AO_XOSC_MUL		768

#endif /* _AO_PINS_H_ */
