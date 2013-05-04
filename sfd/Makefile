NPD_DBUS_INCLUDE := -I ../accapi/dbus/npd
kernel_source := ${KERNEL_ROOT}

deamon_file := sfd.c

# Common flags to be passed for driver compilation
EXTRA_CFLAGS += -Winline -Wall ${CAVIUM_INCLUDES} -DOCTEON_MODEL=${OCTEON_MODEL}

kern: all
obj-m := sfd_kern.o

all:
	@echo "Make sfd_kern module"
	$(MAKE) -C $(kernel_source) SUBDIRS=`pwd` modules;

daemon:
	@echo "Make sfd_daemon"
	$(CC) -Werror -g ${DBUS_INCLUDE} ${DBUS_LIBS} ${NPD_DBUS_INCLUDE} -I../dcli/src/lib -L${BUILDROOTDIR}/usr/lib -lpthread ${deamon_file} -o sfd_daemon

clean:
	@echo "Make sfd clean"
	rm -rf .*.cmd *.mod.c *.o *.ko sfd_daemon .tmp_versions
