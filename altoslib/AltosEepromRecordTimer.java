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
 */

package org.altusmetrum.altoslib_14;

public class AltosEepromRecordTimer extends AltosEepromRecord {
	public static final int	record_length = 32;

	private int log_format;

	private int imu_type() {
		switch (log_format) {
		case AltosLib.AO_LOG_FORMAT_EASYTIMER_2:
			return AltosIMU.imu_type_easytimer_v2;
		default:
			return AltosLib.MISSING;
		}
	}

	private int imu_model() {
		switch (log_format) {
		case AltosLib.AO_LOG_FORMAT_EASYTIMER_2:
			return AltosLib.model_bmi088;
		}
		return AltosLib.MISSING;
	}

	private boolean sensor_normalized() {
		switch (log_format) {
		case AltosLib.AO_LOG_FORMAT_EASYTIMER_2:
			return true;
		}
		return false;
	}

	private int mag_model() {
		switch (log_format) {
		case AltosLib.AO_LOG_FORMAT_EASYTIMER_2:
			return AltosLib.model_mmc5983;
		}
		return AltosLib.MISSING;
	}

	/* AO_LOG_FLIGHT elements */
	private int flight() { return data16(0); }
	private int ground_accel() { return data16(2); }
	private int ground_pres() { return AltosLib.MISSING; }
	private int ground_accel_along() { return data16(4); }
	private int ground_accel_across() { return data16(6); }
	private int ground_accel_through() { return data16(8); }
	private int ground_roll() { return data32(12); }
	private int ground_pitch() { return data32(16); }
	private int ground_yaw() { return data32(20); }

	/* AO_LOG_STATE elements */
	private int state() { return data16(0); }
	private int reason() { return data16(2); }

	/* AO_LOG_SENSOR elements */

	private int accel_along() { return data16(0); }
	private int accel_across() { return data16(2); }
	private int accel_through() { return data16(4); }
	private int gyro_roll() { return data16(6); }
	private int gyro_pitch() { return data16(8); }
	private int gyro_yaw() { return data16(10); }
	private int mag_along() { return data16(12); }
	private int mag_across() { return data16(14); }
	private int mag_through() { return data16(16); }

	private int accel() { return -accel_along(); }

	private int v_batt() { return data16(18); }
	private int v_pbatt() { return data16(20); }
	private int nsense() { return 2; }
	private int sense(int i) { return data16(22 + i * 2); }
	private int pyro() { return data16(26); }

	public void provide_data(AltosDataListener listener, AltosCalData cal_data) {
		super.provide_data(listener, cal_data);

		cal_data.set_imu_type(imu_type());
		cal_data.set_imu_model(imu_model());
		cal_data.set_mag_model(mag_model());

		switch (cmd()) {
		case AltosLib.AO_LOG_FLIGHT:
			cal_data.set_flight(flight());
			cal_data.set_ground_accel(ground_accel());
			listener.set_accel_ground(cal_data.accel_along(ground_accel_along()),
						  cal_data.accel_across(ground_accel_across()),
						  cal_data.accel_through(ground_accel_through()));
			cal_data.set_gyro_zero(ground_roll() / 512.0,
					       ground_pitch() / 512.0,
					       ground_yaw() / 512.0);
			break;
		case AltosLib.AO_LOG_STATE:
			listener.set_state(state());
			break;
		case AltosLib.AO_LOG_SENSOR:
			AltosConfigData config_data = eeprom.config_data();

			int	accel_along = accel_along();
			int	accel_across = accel_across();
			int	accel_through = accel_through();
			int	gyro_roll = gyro_roll();
			int	gyro_pitch = gyro_pitch();
			int	gyro_yaw = gyro_yaw();

			int	mag_along = mag_along();
			int	mag_across = mag_across();
			int	mag_through = mag_through();

			listener.set_accel(cal_data.accel_along(accel_along),
					   cal_data.accel_across(accel_across),
					   cal_data.accel_through(accel_through));
			listener.set_gyro(cal_data.gyro_roll(gyro_roll),
					  cal_data.gyro_pitch(gyro_pitch),
					  cal_data.gyro_yaw(gyro_yaw));

			listener.set_mag(cal_data.mag_along(mag_along),
					 cal_data.mag_across(mag_across),
					 cal_data.mag_through(mag_through));

			listener.set_acceleration(cal_data.acceleration(accel()));

			listener.set_battery_voltage(AltosConvert.easy_timer_battery_voltage(v_batt()));
			listener.set_pyro_voltage(AltosConvert.easy_timer_pyro_voltage_15v(v_pbatt()));

			int nsense = nsense();

			listener.set_apogee_voltage(AltosConvert.easy_timer_pyro_voltage_15v(sense(nsense-2)));
			listener.set_main_voltage(AltosConvert.easy_timer_pyro_voltage_15v(sense(nsense-1)));

			double voltages[] = new double[nsense-2];
			for (int i = 0; i < nsense-2; i++)
				voltages[i] = AltosConvert.easy_timer_pyro_voltage_15v(sense(i));

			listener.set_igniter_voltage(voltages);
			listener.set_pyro_fired(pyro());
			break;
		}
	}

	public AltosEepromRecord next() {
		int	s = next_start();
		if (s < 0)
			return null;
		return new AltosEepromRecordTimer(eeprom, s);
	}

	public AltosEepromRecordTimer(AltosEeprom eeprom, int start) {
		super(eeprom, start, record_length);
		log_format = eeprom.config_data().log_format;
	}

	public AltosEepromRecordTimer(AltosEeprom eeprom) {
		this(eeprom, 0);
	}
}
