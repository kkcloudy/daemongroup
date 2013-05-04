OCTEON_ROOT = /opt/CN/OCTEON-SDK
CAVIUM_INCLUDES := -I ${OCTEON_ROOT}/target/include -I ${OCTEON_ROOT}/gpl-executive -I ../../cavium-ethernet -I ../../cavium-ethernet

kernel_source := ${KERNEL_ROOT}

default: all
obj-m	:=  kpppoe.o

all:
	@echo "Make pppoe_kernel module"
	$(MAKE) -C $(kernel_source) SUBDIRS=`pwd` modules;
	
clean:
	@echo "Make pppoe_kernel clean"
	rm -rf .*.cmd *.mod.c *.o *.ko config/cvmx-config.h .tmp_versions
