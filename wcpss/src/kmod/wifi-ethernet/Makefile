#wifi_INCLUDES := -I ${SUBDIRS}/config -I ${OCTEON_ROOT}/gpl-executive
wifi_INCLUDES := -I ${OCTEON_ROOT}/gpl-executive

#kernel_source :=/opt/CN/kernel2.6.16.26cn
kernel_source := ${OCT_KERNEL_ROOT}
#kernel_source :=../../../../kernel2.6.16.26cn

# Common flags to be passed for driver compilation
EXTRA_CFLAGS += -Winline -Wall ${wifi_INCLUDES} -I$(ACCAPI_DIR)

default: all
obj-m         :=  wifi-ethernet.o

wifi-ethernet-objs := wifi.o
wifi-ethernet-objs += wifi-ioctl.o
wifi-ethernet-objs += wifi-common.o
wifi-ethernet-objs += wifi-sm.o
wifi-ethernet-objs += wifi-rx.o
wifi-ethernet-objs += wifi-tx.o



wifi-ethernet.o: $(wifi-ethernet-objs)
	$(LD) -r $^ -o $@


all: 
	$(MAKE) -C $(kernel_source) SUBDIRS=`pwd`  modules;

clean:
	rm -rf .*.cmd *.mod.c *.o *.ko  .tmp_versions



