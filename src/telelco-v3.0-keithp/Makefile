#
# AltOS build for TeleLCO v3.0
#
#

STM32F1_LINKER_SCRIPT=altos-128.ld

include ../stm32f1/Makefile.defs

INC = \
	ao.h \
	ao_arch.h \
	ao_arch_funcs.h \
	ao_boot.h \
	ao_companion.h \
	ao_data.h \
	ao_sample.h \
	ao_pins.h \
	ao_product.h \
	ao_lco.h \
	ao_lco_cmd.h \
	ao_lco_func.h \
	ao_radio_spi.h \
	ao_radio_cmac.h \
	ao_cc1200_CC1200.h \
	ao_cc1200.h \
	ao_st7565.h \
	ao_font.h \
	ao_logo.h \
	stm32f1.h

#
# Common AltOS sources
#

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
	ao_clock.c \
	ao_timer.c \
	ao_mutex.c \
	ao_freq.c \
	ao_adc_single_stm.c \
	ao_dma_stm.c \
	ao_spi_stm.c \
	ao_beep_stm.c \
	ao_convert_volt.c \
	ao_fast_timer.c \
	ao_pwm_stm.c \
	ao_eeprom.c \
	ao_flash_stm.c \
	ao_usb_stm.c \
	ao_exti_stm.c \
	ao_cc1200.c \
	ao_radio_cmac.c \
	ao_aes.c \
	ao_aes_tables.c \
	ao_st7565.c \
	ao_rect.c \
	ao_text.c \
	ao_text_width.c \
	ao_box.c \
	ao_copy.c \
	ao_blt.c \
	ao_line.c \
	ao_logo.c \
	ao_poly.c \
	BitstreamVeraSans-Roman-58.c \
	BitstreamVeraSans-Roman-24.c \
	BitstreamVeraSans-Roman-10.c \
	BitstreamVeraSans-Roman-12.c \
	BenguiatGothicStd-Bold-24.c \
	ao_quadrature.c \
	ao_button.c \
	ao_event.c \
	ao_lco_bits.c \
	ao_lco_v3.c \
	ao_lco_cmd.c \
	ao_lco_func.c \
	ao_radio_cmac_cmd.c

PRODUCT=TeleLCO-v3.0
PRODUCT_DEF=-DTELELCO
IDPRODUCT=0x0023

CFLAGS = $(PRODUCT_DEF) $(STM32F1_CFLAGS)

PROGNAME=telelco-v3.0
PROG=$(PROGNAME)-$(VERSION).elf
HEX=$(PROGNAME)-$(VERSION).ihx

SRC=$(ALTOS_SRC) ao_telelco.c
OBJ=$(SRC:.c=.o)

all: $(PROG) $(HEX)

$(PROG): Makefile $(OBJ) altos-128.ld
	$(call quiet,CC) $(LDFLAGS) -o $(PROG) $(OBJ) $(LIBS)

$(OBJ): $(INC)

distclean:	clean

clean:
	rm -f *.o $(PROGNAME)-*.elf $(PROGNAME)-*.ihx $(PROGNAME)-*.map
	rm -f ao_product.h

ao_font.h:
	cd ../draw && make ao_font.h ao_logo.h

ao_logo.h: ao_font.h

BitstreamVeraSans-Roman-58.c: ao_font.h
BitstreamVeraSans-Roman-24.c: ao_font.h
BitstreamVeraSans-Roman-10.c: ao_font.h
BitstreamVeraSans-Roman-12.c: ao_font.h
BenguiatGothicStd-Bold-24.c: ao_font.h

install:

uninstall:
