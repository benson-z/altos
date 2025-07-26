vpath %.o .
vpath %.c ..:../kernel:../drivers:../util:../micropeak:../aes:../product
vpath %.h ..:../kernel:../drivers:../util:../micropeak:../aes:../product
vpath make-kalman ..:../kernel:../drivers:../util:../micropeak:../aes:../product

PROGS=ao_flight_test_mm \
	ao_flight_test_metrum ao_flight_test_mini \
	ao_gps_test ao_gps_test_skytraq ao_gps_test_ublox ao_convert_test ao_convert_pa_test ao_fec_test \
	ao_aprs_test ao_micropeak_test ao_fat_test ao_aes_test ao_int64_test \
	ao_ms5607_convert_test ao_quaternion_test \
	ao_flight_test_tmega4

INCS=ao_kalman.h ao_ms5607.h ao_log.h ao_data.h altitude-pa.h altitude.h ao_quaternion.h ao_eeprom_read.h
TEST_SRC=ao_flight_test.c
TEST_SRC_ALL=ao_flight_test.c ao_eeprom_read.c ao_eeprom_read_old.c ao_data.c
TEST_LIB=-ljson-c

KALMAN=make-kalman 

CFLAGS=-I.. -I. -I../kernel -I../drivers -I../product -I../lisp -O0 -g -Wall -DAO_LISP_TEST -no-pie

#all: $(PROGS) ao_aprs_data.wav
all: ao_gps_test_mosaic

clean::
	rm -f $(PROGS) ao_aprs_data.wav run-out.baro run-out.full

install:

ao_flight_test: $(TEST_SRC_ALL) ao_host.h ao_flight.c ao_sample.c ao_kalman.c altitude.h $(INCS)
	cc $(CFLAGS) -o $@ $(TEST_SRC) $(TEST_LIB)

ao_flight_test_noisy_accel: $(TEST_SRC_ALL) ao_host.h ao_flight.c ao_sample.c ao_kalman.c $(INCS)
	cc -DNOISY_ACCEL=1 $(CFLAGS) -o $@ $(TEST_SRC) $(TEST_LIB)

ao_flight_test_baro: $(TEST_SRC_ALL) ao_host.h ao_flight.c  ao_sample.c ao_kalman.c $(INCS)
	cc $(CFLAGS) -o $@ -DHAS_ACCEL=0 $(TEST_SRC) $(TEST_LIB)

ao_flight_test_accel: $(TEST_SRC_ALL) ao_host.h ao_flight.c  ao_sample.c ao_kalman.c $(INCS)
	cc $(CFLAGS) -o $@ -DFORCE_ACCEL=1 $(TEST_SRC) $(TEST_LIB)

ao_flight_test_mm: $(TEST_SRC_ALL) ao_host.h ao_flight.c ao_sample.c ao_kalman.c ao_pyro.c ao_pyro.h $(INCS)
	cc -DTELEMEGA=1 $(CFLAGS) -o $@ $(TEST_SRC)  $(TEST_LIB) -lm

ao_flight_test_tmega4: $(TEST_SRC_ALL) ao_host.h ao_flight.c ao_sample.c ao_kalman.c ao_pyro.c ao_pyro.h $(INCS)
	cc -DTELEMEGA_V4=1 $(CFLAGS) -o $@ $(TEST_SRC)  $(TEST_LIB) -lm

ao_flight_test_metrum: $(TEST_SRC_ALL) ao_host.h ao_flight.c ao_sample.c ao_kalman.c ao_pyro.c ao_pyro.h $(INCS)
	cc -DTELEMETRUM_V2=1 $(CFLAGS) -o $@ $(TEST_SRC)  $(TEST_LIB) -lm

ao_flight_test_mini: $(TEST_SRC_ALL) ao_host.h ao_flight.c ao_sample.c ao_kalman.c ao_pyro.c ao_pyro.h $(INCS)
	cc -DEASYMINI=1 -DHAS_ACCEL=0 $(CFLAGS) -o $@ $(TEST_SRC)  $(TEST_LIB) -lm

ao_flight_test_motor: $(TEST_SRC_ALL) ao_host.h ao_flight.c ao_sample.c ao_kalman.c ao_pyro.c ao_pyro.h $(INCS)
	cc -DEASYMOTOR_V_2=1 $(CFLAGS) -o $@ $(TEST_SRC) $(TEST_LIB) -lm

ao_gps_test: ao_gps_test.c ao_gps_sirf.c ao_gps_print.c ao_host.h
	cc $(CFLAGS) -o $@ $<

ao_gps_test_skytraq: ao_gps_test_skytraq.c ao_gps_skytraq.c ao_gps_print.c ao_gps_show.c ao_host.h
	cc $(CFLAGS) -o $@ $<

ao_gps_test_ublox: ao_gps_test_ublox.c ao_gps_ublox.c ao_gps_print.c ao_gps_show.c ao_host.h ao_gps_ublox.h
	cc $(CFLAGS) -o $@ $<

ao_gps_test_mosaic: ao_gps_test_mosaic.c ao_gps_mosaic.c ao_gps_print.c ao_gps_show.c ao_host.h ao_gps_mosaic.h
	cc $(CFLAGS) -o $@ $< -lm

ao_convert_test: ao_convert_test.c ao_convert.c altitude.h
	cc $(CFLAGS) -o $@ $<

ao_convert_pa_test: ao_convert_pa_test.c ao_convert_pa.c altitude-pa.h
	cc $(CFLAGS) -o $@ $<

ao_kalman.h: $(KALMAN)
	(cd .. && make ao_kalman.h)

ao_fec_test: ao_fec_test.c ao_fec_tx.c ao_fec_rx.c
	cc $(CFLAGS) -DAO_FEC_DEBUG=1 -o $@ ao_fec_test.c ../kernel/ao_fec_tx.c ../kernel/ao_fec_rx.c -lm

ao_aprs_test: ao_aprs_test.c ao_aprs.c
	cc $(CFLAGS) -o $@ ao_aprs_test.c -lm

SOX_INPUT_ARGS=--type raw --encoding unsigned-integer -b 8 -c 1 -r 9600
SOX_OUTPUT_ARGS=--type wav

ao_aprs_data.wav: ao_aprs_test
	./ao_aprs_test | sox $(SOX_INPUT_ARGS) - $(SOX_OUTPUT_ARGS) $@

check: ao_fec_test ao_flight_test ao_flight_test_baro run-tests
	./ao_fec_test && ./run-tests

ao_micropeak_test: ao_micropeak_test.c ao_microflight.c ao_kalman.h
	cc $(CFLAGS) -o $@ ao_micropeak_test.c -lm

ao_fat_test: ao_fat_test.c ao_fat.c ao_bufio.c
	cc $(CFLAGS) -o $@ ao_fat_test.c -Wno-deprecated-declarations -lssl -lcrypto

ao_aes_test: ao_aes_test.c ao_aes.c ao_aes_tables.c
	cc $(CFLAGS) -o $@ ao_aes_test.c

ao_int64_test: ao_int64_test.c ao_int64.c ao_int64.h
	cc $(CFLAGS) -o $@ ao_int64_test.c

ao_ms5607_convert_test: ao_ms5607_convert_test.c ao_ms5607_convert_8051.c ao_int64.c ao_int64.h
	cc $(CFLAGS) -o $@ ao_ms5607_convert_test.c

ao_quaternion_test: ao_quaternion_test.c ao_quaternion.h
	cc $(CFLAGS) -o $@ ao_quaternion_test.c -lm

