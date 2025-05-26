#
# AltOS build
#
#

include ../samd21/Makefile.defs

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
        ao_adxl375.h \
        ao_cc1200_CC1200.h \
        ao_task.h \
        ao_whiten.h \
	samd21.h \
	Makefile

# SAMD21G17D

SAMD21_ROM=128
SAMD21_RAM=16

#PROFILE=ao_profile.c
#PROFILE_DEF=-DAO_PROFILE=1

#SAMPLE_PROFILE=ao_sample_profile.c \
#       ao_sample_profile_timer.c
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
        ao_led.c \
        ao_stdio.c \
        ao_panic.c \
        ao_timer.c \
        ao_mutex.c \
        ao_serial_samd21.c \
        ao_gps_ublox.c \
        ao_gps_show.c \
        ao_gps_report_metrum.c \
        ao_ignite.c \
        ao_freq.c \
        ao_dma_samd21.c \
        ao_spi_samd21.c \
        ao_cc1200.c \
        ao_data.c \
        ao_ms5607.c \
        ao_adxl375.c \
        ao_adc_samd21.c \
        ao_beep_samd21.c \
        ao_storage.c \
        ao_m25.c \
	ao_usb_samd21.c \
	ao_exti_samd21.c \
        ao_report.c \
        ao_convert_pa.c \
        ao_convert_volt.c \
        ao_log.c \
        ao_log_metrum.c \
        ao_sample.c \
        ao_kalman.c \
        ao_flight.c \
        ao_telemetry.c \
        ao_packet_slave.c \
        ao_packet.c \
        ao_companion.c \
        ao_aprs.c \
        $(PROFILE) \
        $(SAMPLE_PROFILE) \
        $(STACK_GUARD)

PRODUCT=TeleMetrum-v4.0
PRODUCT_DEF=-DTELEMETRUM_V_4_0
IDPRODUCT=0x000b

CFLAGS = $(PRODUCT_DEF) $(SAMD21_CFLAGS) $(PROFILE_DEF) $(SAMPLE_PROFILE_DEF) $(STACK_GUARD_DEF)

PROGNAME=telemetrum-v4.0
PROG=$(PROGNAME)-$(VERSION).elf
HEX=$(PROGNAME)-$(VERSION).ihx
FLASH_PROG=flash-loader/$(PROGNAME)-altos-flash-$(VERSION).elf
BOTH_HEX=$(PROGNAME)-combined-$(VERSION).ihx

SRC=$(ALTOS_SRC) ao_telemetrum_seeed.c
OBJ=$(SRC:.c=.o)

all: $(PROG) $(HEX) $(BOTH_HEX)

$(PROG): Makefile $(OBJ)
	$(call quiet,CC) $(LDFLAGS) -o $(PROG) $(OBJ) $(LIBS)

$(BOTH_HEX): $(PROG) $(FLASH_PROG)
	../../ao-tools/ao-elftohex/ao-elftohex -n --output=$@ $(FLASH_PROG) $(PROG)

$(FLASH_PROG): FRC
	+cd flash-loader && make

FRC:

$(OBJ): $(INC)

load: $(PROG)
	stm-load $(PROG)

distclean:	clean

clean:
	rm -f *.o $(PROGNAME)-*.elf $(PROGNAME)-*.ihx $(PROGNAME)-*.map
	rm -f ao_product.h

install:

uninstall:
