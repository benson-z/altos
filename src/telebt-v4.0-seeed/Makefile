#
# AltOS build
#
#

include ../stmf0/Makefile.defs

INC = \
	ao.h \
	ao_arch.h \
	ao_arch_funcs.h \
	ao_boot.h \
	ao_pins.h \
	ao_product.h \
	ao_cc1200_CC1200.h \
	ao_task.h \
	ao_rn4678.h \
	stm32f0.h \
	Makefile

ALTOS_SRC = \
	ao_boot_chain.c \
	ao_interrupt.c \
	ao_product.c \
	ao_romconfig.c \
	ao_cmd.c \
	ao_config.c \
	ao_data.c \
	ao_task.c \
	ao_led_stmf0.c \
	ao_stdio.c \
	ao_panic.c \
	ao_timer.c \
	ao_mutex.c \
	ao_serial_stm.c \
	ao_rn4678.c \
	ao_freq.c \
	ao_dma_stm.c \
	ao_spi_stm.c \
	ao_cc1200.c \
	ao_adc_stm.c \
	ao_usb_stm.c \
	ao_exti_stm.c \
	ao_convert_volt.c \
	ao_packet_master.c \
	ao_packet.c \
	ao_monitor.c \
	ao_send_packet.c

PRODUCT=TeleBT-v4.0
PRODUCT_DEF=-DTELEBT_V_4_0
IDPRODUCT=0x000e

CFLAGS = $(PRODUCT_DEF) $(STMF0_CFLAGS)

PROGNAME=telebt-v4.0-seeed
PROG=$(PROGNAME)-$(VERSION).elf
HEX=$(PROGNAME)-$(VERSION).ihx
FLASH_PROG=flash-loader/$(PROGNAME)-altos-flash-$(VERSION).elf
BOTH_HEX=$(PROGNAME)-combined-$(VERSION).ihx

SRC=$(ALTOS_SRC) ao_telebt_seeed.c
OBJ=$(SRC:.c=.o)

all: $(PROG) $(HEX) $(BOTH_HEX)

$(PROG): Makefile $(OBJ) altos.ld
	$(call quiet,CC) $(LDFLAGS) -o $(PROG) $(OBJ) $(LIBS)

$(BOTH_HEX): $(PROG) $(FLASH_PROG)
	../../ao-tools/ao-elftohex/ao-elftohex -n --output=$@ $(FLASH_PROG) $(PROG)

$(FLASH_PROG): FRC
	+cd flash-loader && make

$(OBJ): $(INC)

distclean:	clean

clean:
	rm -f *.o $(PROGNAME)-*.elf $(PROGNAME)-*.ihx $(PROGNAME)-*.map
	rm -f ao_product.h

install:

uninstall:

FRC:
