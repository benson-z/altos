#
# AltOS build
#
#

include ../stm/Makefile.defs

INC = \
	ao.h \
	ao_arch.h \
	ao_arch_funcs.h \
	ao_boot.h \
	ao_companion.h \
	ao_data.h \
	ao_sample.h \
	ao_pins.h \
	altitude-pa.h \
	ao_kalman.h \
	ao_product.h \
	ao_ms5607.h \
	ao_mpu9250.h \
	ao_mma655x.h \
	ao_cc1200_CC1200.h \
	ao_profile.h \
	ao_task.h \
	ao_whiten.h \
	ao_sample_profile.h \
	ao_quaternion.h \
	ao_mpu.h \
	stm32l.h \
	ao_ms5607_convert.c \
	Makefile

#
# Common AltOS sources
#

#PROFILE=ao_profile.c
#PROFILE_DEF=-DAO_PROFILE=1

#SAMPLE_PROFILE=ao_sample_profile.c \
#	ao_sample_profile_timer.c
#SAMPLE_PROFILE_DEF=-DHAS_SAMPLE_PROFILE=1

#STACK_GUARD=ao_mpu_stm.c
#STACK_GUARD_DEF=-DHAS_STACK_GUARD=1

ALTOS_SRC = \
	ao_boot_chain.c \
	ao_interrupt.c \
	ao_product.c \
	ao_romconfig.c \
	ao_cmd.c \
	ao_config.c \
	ao_task.c \
	ao_led_stm.c \
	ao_stdio.c \
	ao_panic.c \
	ao_timer.c \
	ao_mutex.c \
	ao_serial_stm.c \
	ao_gps_mosaic.c \
	ao_crc_ccitt.c \
	ao_gps_ublox.c \
	ao_gps_show.c \
	ao_gps_report_mega.c \
	ao_ignite.c \
	ao_freq.c \
	ao_dma_stm.c \
	ao_spi_stm.c \
	ao_cc1200.c \
	ao_data.c \
	ao_ms5607.c \
	ao_mma655x.c \
	ao_adc_stm.c \
	ao_beep_stm.c \
	ao_eeprom_stm.c \
	ao_storage.c \
	ao_m25.c \
	ao_usb_stm.c \
	ao_exti_stm.c \
	ao_report.c \
	ao_i2c_stm.c \
	ao_mpu9250.c \
	ao_convert_pa.c \
	ao_convert_volt.c \
	ao_log.c \
	ao_log_mega.c \
	ao_sample.c \
	ao_kalman.c \
	ao_flight.c \
	ao_telemetry.c \
	ao_packet_slave.c \
	ao_packet.c \
	ao_companion.c \
	ao_pyro.c \
	ao_aprs.c \
	ao_pwm_stm.c \
	$(PROFILE) \
	$(SAMPLE_PROFILE) \
	$(STACK_GUARD)

PRODUCT=TeleMega-v3.0
PRODUCT_DEF=-DTELEMEGA
IDPRODUCT=0x0023

CFLAGS = $(PRODUCT_DEF) $(STM_CFLAGS) $(PROFILE_DEF) $(SAMPLE_PROFILE_DEF) $(STACK_GUARD_DEF)

PROGNAME=telemega-v3.0
PROG=$(PROGNAME)-$(VERSION).elf
HEX=$(PROGNAME)-$(VERSION).ihx

SRC=$(ALTOS_SRC) ao_telemega.c
OBJ=$(SRC:.c=.o)

all: $(PROG) $(HEX)

$(PROG): Makefile $(OBJ) altos.ld
	$(call quiet,CC) $(LDFLAGS) -o $(PROG) $(OBJ) $(LIBS)

$(OBJ): $(INC)

distclean:	clean

clean:
	rm -f *.o $(PROGNAME)-*.elf $(PROGNAME)-*.ihx $(PROGNAME)-*.map
	rm -f ao_product.h

install:

uninstall:
