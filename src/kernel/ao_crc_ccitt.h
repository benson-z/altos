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

#ifndef _AO_CRC_CCITT_H_
#define _AO_CRC_CCITT_H_

#include <stdint.h>

extern const uint16_t ao_crc16_ccitt_table[256];

static inline uint16_t ao_crc16_ccitt(uint16_t crc, uint8_t byte)
{
	uint16_t high = crc << 8;
	uint8_t low = (uint8_t) (crc >> 8) ^ byte;
	return high ^ ao_crc16_ccitt_table[low];
}

#endif /* _AO_CRC_CCITT_H_ */
