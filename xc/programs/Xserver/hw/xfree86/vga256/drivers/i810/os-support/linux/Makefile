###############################################################################
#
# Makefile for the agpgart.o kernel module.  
#
# Keith Whitwell <keithw@precisioninsight.com>
#
# Based on Dev3dfx makefile by Carlo Wood
#
# $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/i810/os-support/linux/Makefile,v 1.1.2.2 1999/11/18 19:06:19 hohndel Exp $

ifeq ($(OPT_CFLAGS),)
OPT_CFLAGS = -O2 -fomit-frame-pointer -m486
endif

CFLAGS = $(OPT_CFLAGS) -DMODULE -D__KERNEL__ -I/usr/src/linux/include -pipe -fno-strength-reduce -malign-loops=2 -malign-jumps=2 -malign-functions=2
CC=gcc

###############################################################################
# You should never need to change anything below.

# Detect kernel version
KERNEL_VERSION=$(shell ./version-test.sh)
KERNEL_VERSION_MAJOR:=$(shell echo "$(KERNEL_VERSION)" | awk -F. '{ printf ("%s.%s\n", $$1, $$2) }')

# Detect if we have mtrr
HAVE_MTRR:=$(shell grep "flags.*:.* mtrr " /proc/cpuinfo)

ifneq ($(HAVE_MTRR),)
CFLAGS += -DHAVE_MTRR
endif

AUTOCONF=/usr/include/linux/autoconf.h

# Decide if we are SMP
HAVE_SMP := $(shell \
	if grep -q '^\#define.*CONFIG_SMP.*1' $(AUTOCONF); \
	then echo 1; else echo 0; fi)

# Decide if we should use modules
HAVE_MODVERSIONS := $(shell \
	if grep -q '^\#define.*CONFIG_MODVERSIONS.*1' $(AUTOCONF); \
	then echo 1; else echo 0; fi)

ifeq ($(HAVE_SMP),1)
CFLAGS += -D__SMP__
endif

#ifeq ($(HAVE_MODVERSIONS),1)
#CFLAGS += -DMODVERSIONS
#endif

all: sanity agpgart.o

# Sanity checks
sanity:
	@( \
	if [ "$(KERNEL_VERSION_MAJOR)" != "2.0" -a \
	     "$(KERNEL_VERSION_MAJOR)" != "2.1" -a \
	     "$(KERNEL_VERSION_MAJOR)" != "2.2" -a \
	     "$(KERNEL_VERSION_MAJOR)" != "2.3" ]; then \
		echo "KERNEL_VERSION=$(KERNEL_VERSION)"; \
		echo "KERNEL_VERSION_MAJOR=$(KERNEL_VERSION_MAJOR)"; \
		echo "Something wrong with detection of KERNEL_VERSION?"; \
		echo "Please check Makefile and report bug"; \
	fi; \
	if [ "$(KERNEL_VERSION_MAJOR)" != "2.2" ]; then \
		echo "KERNEL_VERSION_MAJOR=$(KERNEL_VERSION_MAJOR)"; \
		echo "agpgart.o not yet available for non-2.2 kernels" \
		exit -1; \
	fi; \
	if [ ! -e /usr/src/linux ]; then \
		echo "Expect kernel source at location /usr/src/linux"; \
		echo "Sym.link /usr/src/linux -> where you have your sources"; \
		exit -1; \
	fi; \
	if [ ! -r /usr/src/linux/include ]; then \
		echo "Expect readable headers in /usr/src/linux/include"; \
		exit -1; \
	fi; \
	if [ ! -r /usr/src/linux/include/linux/version.h ]; then \
		echo "Missing /usr/src/linux/include/linux/version.h"; \
		echo "Configure and install the kernel first"; \
		exit -1; \
	fi; \
	if [ ! -e /proc/cpuinfo ]; then \
		echo "You need the /proc file system"; \
		echo "Reconfigure kernel and say Yes to CONFIG_PROC_FS"; \
		exit -1; \
	fi; \
	)

###############################################################################
# kernel 2.2.x only

ifeq ($(KERNEL_VERSION_MAJOR),2.2)

agpgart.o: agpgart.c Makefile
	$(CC) $(CFLAGS) -c -o $@ agpgart.c

else

agpgart.o:
	@echo "agpgart.o not yet available for non-2.2 kernels"
	@exit -1

endif

###############################################################################

install:
	mkdir -p /lib/modules/$(KERNEL_VERSION)/misc
	cp agpgart.o /lib/modules/$(KERNEL_VERSION)/misc/agpgart.o
	mkdir -p /usr/local/include
	cp agpgart.h /usr/local/include/agpgart.h
	if [ -e /lib/modules/$(KERNEL_VERSION)/modules.dep ]; then \
		indep=`grep 'misc/agpgart.o:' /lib/modules/$(KERNEL_VERSION)/modules.dep`; \
		if [ -z "$$indep" ]; then \
			echo "/lib/modules/$(KERNEL_VERSION)/misc/agpgart.o:" >> /lib/modules/$(KERNEL_VERSION)/modules.dep; \
			echo "" >> /lib/modules/$(KERNEL_VERSION)/modules.dep; \
		fi; \
	fi; \
	if [ ! -c /dev/agpgart ]; then \
	        mknod /dev/agpgart c 10 175; \
		chmod go+w /dev/agpgart; \
	fi; \
		echo "/lib/modules/$(KERNEL_VERSION)/misc/agpgart.o"; \
		inconf=`grep 'alias char-major-10-175 agpgart' /etc/conf.modules`; \
		if [ -z "$$inconf" ]; then \
			echo "alias char-major-10-175 agpgart" >> /etc/conf.modules; \
		fi; \
	/sbin/depmod -a 


###############################################################################
# This is for debugging purposes by the developers:

clean:
	rm -f *.o *.s

tar:
	tar czf ../../SOURCES/I810Gtt.tar.gz agpgart.c agpgart.h testgart.c README Makefile

debug:
	make OPT_CFLAGS="-g -Wall -Wstrict-prototypes -DDEBUG"

testgart.o: testgart.c
	$(CC) -O0 -g -c testgart.c -o testgart.o
