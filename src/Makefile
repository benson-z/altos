#
# AltOS build
#

vpath make-altitude util
vpath make-altitude-pa util
vpath make-kalman util
vpath make-whiten util
vpath kalman.5c kalman
vpath kalman_filter.5c kalman
vpath kalman_micro.5c kalman
vpath load_csv.5c kalman
vpath matrix.5c kalman

TOPDIR=.

include Makedefs

ARMM3DIRS=\
	easymega-v1.0 easymega-v1.0/flash-loader \
	easymega-v2.0 easymega-v2.0/flash-loader \
	easymotor-v3 easymotor-v3/flash-loader \
	easytimer-v1 easytimer-v1/flash-loader \
	easytimer-v2 easytimer-v2/flash-loader \
	telemega-v0.1 telemega-v0.1/flash-loader \
	telemega-v1.0 telemega-v1.0/flash-loader \
	telemega-v2.0 telemega-v2.0/flash-loader \
	telemega-v3.0 telemega-v3.0/flash-loader \
	telemega-v4.0 telemega-v4.0/flash-loader \
	telemega-v5.0 telemega-v5.0/flash-loader \
	telemega-v6.0 telemega-v6.0/flash-loader \
	telemega-v7.0 telemega-v7.0/flash-loader \
	telemetrum-v2.0 telemetrum-v2.0/flash-loader \
	telemetrum-v3.0 telemetrum-v3.0/flash-loader \
	telegps-v0.3 telegps-v0.3/flash-loader \
	telegps-v1.0 telegps-v1.0/flash-loader \
	telegps-v2.0 telegps-v2.0/flash-loader \
	telegps-v3.0 telegps-v3.0/flash-loader \
	telegps-v4.0 telegps-v4.0/flash-loader \
	telelco-v0.2 telelco-v0.2/flash-loader \
	telelco-v0.2-cc1200 telelco-v0.2-cc1200/flash-loader \
	telelco-v0.3 telelco-v0.3/flash-loader \
	telelco-v2.0 telelco-v2.0/flash-loader \
	teledongle-v3.0 teledongle-v3.0/flash-loader \
	teleballoon-v2.0 \
	telebt-v3.0 telebt-v3.0/flash-loader \
	telebt-v4.0 telebt-v4.0/flash-loader \
	telelcotwo-v0.1 telelcotwo-v0.1/flash-loader \
	telefireone-v1.0 telefireone-v1.0/flash-loader \
	telefiretwo-v0.1 telefiretwo-v0.1/flash-loader \
	telefireeight-v1.0 telefireeight-v1.0/flash-loader \
	telefireeight-v2.0 telefireeight-v2.0/flash-loader 

ARMM0DIRS=\
	easymini-v1.0 easymini-v1.0/flash-loader \
	chaoskey-v1.0 chaoskey-v1.0/flash-loader \
	telemetrum-v4.0 telemetrum-v4.0/flash-loader \
	telemini-v3.0 telemini-v3.0/flash-loader \
	easymini-v2.0 easymini-v2.0/flash-loader \
	easymini-v3.0 easymini-v3.0/flash-loader \
	telelco-v3.0 telelco-v3.0/flash-loader \
	easymega-v3.0 easymega-v3.0/flash-loader \
	micropeak-v2.0

AVRDIRS=\
	micropeak microkite microsplash

SUBDIRS=draw test

ifeq ($(strip $(HAVE_ARM_M3_CC)),yes)
SUBDIRS+=$(ARMM3DIRS)
foo=bar
endif

ifeq ($(strip $(HAVE_ARM_M0_CC)),yes)
SUBDIRS+=$(ARMM0DIRS)
baz=bletch
endif

ifeq ($(strip $(HAVE_AVR_CC)),yes)
SUBDIRS += $(AVRDIRS)
endif

ALLDIRS=$(ARMM3DIRS) $(ARMM0DIRS) $(AVRDIRS)

all: all-local all-recursive

RECURSIVE_TARGETS = all-recursive install-recursive

$(RECURSIVE_TARGETS):
	@target=`echo $@ | sed 's/-recursive//'`; \
	for subdir in $(SUBDIRS); do \
		echo "Making $$target in $$subdir"; \
		(cd $$subdir && $(MAKE) $$target) || exit 1; \
	done

ALL_RECURSIVE_TARGETS = clean-recursive

$(ALL_RECURSIVE_TARGETS):
	@target=`echo $@ | sed 's/-recursive//'`; \
	for subdir in $(ALLDIRS); do \
		echo "Making $$target in $$subdir"; \
		(cd $$subdir && $(MAKE) $$target) || exit 1; \
	done

distclean:	clean

clean: clean-local clean-recursive

install: install-recursive

uninstall:

all-recursive: all-local

all-local: altitude.h altitude-pa.h altitude-pa-small.h ao_kalman.h ao_whiten.h $(PDCLIB)

altitude.h: make-altitude
	nickle $< > $@

altitude-pa.h: make-altitude-pa
	nickle $< > $@

altitude-pa-small.h: make-altitude-pa
	nickle $< --sample 3 > $@

ao_kalman.h: make-kalman kalman.5c kalman_micro.5c kalman_filter.5c load_csv.5c matrix.5c
	bash $< kalman > $@

ao_whiten.h: make-whiten
	nickle $< > $@

clean-local: $(CLEAN_PDCLIB)
	rm -f altitude.h ao_kalman.h
