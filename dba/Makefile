kernel_source := ../kernel2.6.16.26cn


# Common flags to be passed for driver compilation
EXTRA_CFLAGS += -Winline -Wall #${CAVIUM_INCLUDES} -DOCTEON_MODEL=${OCTEON_MODEL}
DEFPATH = $(ACCAPI_DIR)
#CFLAGS_LOCAL += -I$(DEFPATH)
EXTRA_CFLAGS += -I$(DEFPATH)


default: all
obj-m    :=  dba.o
dba-objs := main.o
dba-objs += packet_snooping.o
dba-objs += pppoe_snooping.o
dba-objs += netlink.o
dba-objs += dhcp_broadcast_agent.o
dba-objs += log.o

#dba.o: $(dba-objs)
#	$(LD) -r $^ -o $@

all: 
	$(MAKE) -C $(KERNEL_ROOT) SUBDIRS=`pwd`  modules;

clean:
	rm -rf .*.cmd *.mod.c *.o *.ko config/cvmx-config.h .tmp_versions

