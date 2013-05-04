#OCTEON_ROOT = /opt/CN/OCTEON-SDK
#kernel_source := ../kernel2.6.16.26cn

OCTNIC_DIR = ../oct-nic/components/driver/host/driver
OCTNIC_INCLUDES := -I ${OCTNIC_DIR}/linux/octnic/ -I $(OCTNIC_DIR)/osi/ -I $(OCTNIC_DIR)/linux/
CAVIUM_INCLUDES := -I ./config -I ${OCTEON_ROOT}/target/include -I ${OCTEON_ROOT}/executive -I $(KERNEL_ROOT)/include/linux
#CAVIUM_INCLUDES := -I ${OCTEON_ROOT}/target/include -I ${OCTEON_ROOT}/gpl-executive -I ../cavium-ethernet
#kernel_source := ${OCTEON_ROOT}/linux/kernel_2.6/linux/

# Common flags to be passed for driver compilation
EXTRA_CFLAGS += -Winline -Wall ${CAVIUM_INCLUDES} ${OCTNIC_INCLUDES} -DOCTEON_MODEL=${OCTEON_MODEL} -D__SMP__ -DBUILDNO_VERSION2_1
EXTRA_CFLAGS += -Werror

ifdef IPFWD_LEARN_MODE_STANDALONE
EXTRA_CFLAGS += -DIPFWD_LEARN_MODE_STANDALONE
PREFIX := _standalone
else
PREFIX := _coexist
endif


default: all
obj-m         :=  ipfwd_learn$(PREFIX).o

ipfwd_learn$(PREFIX)-objs := ipfwd_learn.o
ipfwd_learn$(PREFIX)-objs += ipfwd_learn_localip.o
ipfwd_learn$(PREFIX)-objs += ipfwd_log_proc.o

ipfwd_learn.o: $(ipfwd_learn_std-objs)
	$(LD) -r $^ -o $@


all:
	$(MAKE) -C $(KERNEL_ROOT) SUBDIRS=`pwd`   modules;

clean:
	rm -rf .*.cmd *.mod.c *.o *.ko  .tmp_versions  Module.symvers modules.order

