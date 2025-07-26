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

#ifndef _AO_GPS_MOSAIC_H_
#define _AO_GPS_MOSAIC_H_

void
ao_gps_mosaic(void);

struct sbf_header {
	uint16_t	crc;
	struct {
		uint16_t	id;
		uint16_t	length;
	} h;
};

#define SBF_BLOCK_NUMBER(id)	((id) & 0x1fff)
#define SBF_BLOCK_REVISION(id)	((id) >> 13)

#define SBF_MEAS_EPOCH		4027

struct sbf_meas_epoch {
	uint32_t	tow;
	uint16_t	wnc;
	uint8_t		n1;
	uint8_t		sb1length;
	uint8_t		sb2length;
	uint8_t		commonflags;
	uint8_t		cumclkjumps;
	uint8_t		reserved;
};

struct sbf_meas_epoch_channel_type1 {
	uint8_t		rxchannel;
	uint8_t		type;
	uint8_t		svid;
	uint8_t		misc;
	uint32_t	codelsb;
	int32_t		doppler;
	uint16_t	carrierlsb;
	int8_t		carriermsb;
	uint8_t		cn0;
	uint16_t	locktime;
	uint8_t		obsinfo;
	uint8_t		n2;
};

struct sbf_meas_epoch_channel_type2 {
	uint8_t		type;
	uint8_t		locktime;
	uint8_t		cn0;
	uint8_t		offsetmsb;
	int8_t		carriermsb;
	uint8_t		obsinfo;
	uint16_t	codeoffsetlsb;
	uint16_t	carrierlsb;
	uint16_t	doppleroffsetlsb;
};

#define SBF_DOP			4001

struct sbf_dop {
	uint32_t	tow;
	uint16_t	wnc;
	uint8_t		nrsv;
	uint8_t		reserved;
	uint16_t	pdop;
	uint16_t	tdop;
	uint16_t	hdop;
	uint16_t	vdop;
	float		hpl;
	float		vpl;
};

#define SBF_PVT_GEODETIC	4007

struct sbf_pvt_geodetic {
	uint32_t	tow;
	uint16_t	wnc;
	uint8_t		mode;
	uint8_t		error;
	double		latitude;
	double		longitude;
	double		height;
	float		undulation;
	float		vn;
	float		ve;
	float		vu;
	float		cog;
	double		rxclkbias;
	float		rxclkdrift;
	uint8_t		timesystem;
	uint8_t		datum;
	uint8_t		nrsv;
	uint8_t		wacorrinfo;
	uint16_t	referenceid;
	uint16_t	meancorrage;
	uint32_t	signalinfo;
	uint8_t		alertflag;
	uint8_t		nrbases;
	uint16_t	pppinfo;
	uint16_t	latency;
	uint16_t	haccuracy;
	uint16_t	vaccuracy;
	uint8_t		misc;
};

#define SBF_RECEIVER_TIME	5914

struct sbf_receiver_time {
	uint32_t	tow;
	uint16_t	wnc;
	int8_t		utcyear;
	int8_t		utcmonth;
	int8_t		utcday;
	int8_t		utchour;
	int8_t		utcmin;
	int8_t		utcsec;
	int8_t		deltals;
	uint8_t		synclevel;
};

#define MAX_SBF_DATA	1024

struct sbf {
	struct sbf_header	header;
	union {
		struct sbf_receiver_time	receiver_time;
		struct sbf_meas_epoch		meas_epoch;
		struct sbf_pvt_geodetic		geodetic;
		struct sbf_dop			dop;
		uint8_t				data[MAX_SBF_DATA];
	};
};

#endif /* _AO_GPS_MOSAIC_H_ */
