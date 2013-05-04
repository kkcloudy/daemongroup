ifneq ($(KERNELRELEASE),)
obj-m := ipt_ipp2p.o

else
KERNEL_SRC = $(OCT_KERNEL_ROOT)
#KERNEL_SRC ?= $(firstword $(wildcard /lib/modules/$(shell uname -r)/build /usr/src/linux))
ifeq ($(KERNEL_SRC),)
$(error You need to define KERNEL_SRC)
endif

ifneq ($wildcard $(KERNEL_SRC)/include/linux/modversions.h),)
MODVERSIONS = -DMODVERSIONS
endif

_KVER = $(strip $(shell cat $(KERNEL_SRC)/Makefile | grep -e '^VERSION' | cut -d"=" -f2))
_KPL = $(strip $(shell cat $(KERNEL_SRC)/Makefile | grep -e '^PATCHLEVEL' | cut -d"=" -f2))
_KSUB = $(strip $(shell cat $(KERNEL_SRC)/Makefile | grep -e '^SUBLEVEL' | cut -d"=" -f2))
KERNEL_SERIES=$(_KVER).$(_KPL)

ifeq ($(KERNEL_SERIES), 2.6)
	TARGET=ipt_ipp2p.ko
else
	TARGET=ipt_ipp2p.o
endif

SED = sed
IPTABLES_BIN = iptables
IPTABLES_ROOT = ../../
IPTABLES_SRC = ${IPTABLES_ROOT}debsrc/iptables-1.3.6.0debian1/iptables
$(info IPTABLES_SRC is $(IPTABLES_SRC) )
IPTVER = "1.3.6"

ifndef IPTABLES_SRC
$(warning IPTABLE_SRC not defined, try to search one.)
IPTVER = \
	$(shell $(IPTABLES_BIN) --version | $(SED) -e 's/^iptables v//')
IPTABLES_SRC = $(wildcard /usr/src/iptables-$(IPTVER))
#IPTABLES_SRC = /var/tmp/portage/net-firewall/iptables-1.4.0-r1/work/iptables-1.4.0
endif

ifeq ($(IPTABLES_SRC),)
$(warning You need to install iptables sources and maybe set IPTABLES_SRC)
endif

IPTABLES_INCLUDE = -I$(IPTABLES_SRC)/include

$(warning IPTABLES_INCLUDE is $(IPTABLES_INCLUDE) )

ifneq ($(IPTVER),)
	IPTABLES_VERSION = $(IPTVER)
else
	IPTABLES_VERSION = $(shell cat $(IPTABLES_SRC)/Makefile | grep -e '^IPTABLES_VERSION:=' | cut -d"=" -f2)
endif

IPTABLES_OPTION = -DIPTABLES_VERSION=\"$(IPTABLES_VERSION)\"
CC = mips-linux-gnu-gcc
CFLAGS = -O3 -Wall



$(warning Target is $(TARGET) and kernel src is $(KERNEL_SRC))
all: modules libipt_ipp2p.so

modules: $(TARGET)

ipt_ipp2p.o: ipt_ipp2p.h ipt_ipp2p.c
	$(CC) $(CFLAGS) -I$(KERNEL_SRC)/include -c ipt_ipp2p.c -D__KERNEL__ -DMODULE $(MODVERSIONS)

ipt_ipp2p.ko: ipt_ipp2p.h ipt_ipp2p.c
	$(MAKE) -C $(KERNEL_SRC) SUBDIRS=`pwd` modules


libipt_ipp2p.so: libipt_ipp2p.c ipt_ipp2p.h
	$(CC) $(CFLAGS) $(IPTABLES_OPTION) $(IPTABLES_INCLUDE) -fPIC -c libipt_ipp2p.c
	$(CC) -shared -o libipt_ipp2p.so libipt_ipp2p.o
install:
	cp ipt_ipp2p.ko /lib/modules/$(shell uname -r)/kernel/net/ipv4/netfilter/
	cp libipt_ipp2p.so /lib/iptables/
	depmod -a
clean:
	-rm -rf *.o *.so *.ko .*.cmd *.mod.c .tmp_versions *~
endif
