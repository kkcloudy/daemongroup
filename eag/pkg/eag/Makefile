#CFLAGS=-O3 -Wall -ffunction-sections
CFLAGS= -Wall -g -ffunction-sections
#CFLAGS+= -Werror
CC=mips-linux-gnu-gcc
AR=mips-linux-gnu-ar
OBJCOPY=mips-linux-gnu-objcopy
RANLIB=ranlib
LIBS= -L.. -ldbusapi -lm

LD_PATH = -L$(MIPS_ADDITION_LIB_ROOT_PATH)/usr/lib

LDFLAGS += -Wl,--gc-sections

INCLUDE += -I ../../../iptables/debsrc/iptables-1.4.6/include
INCLUDE += -I ./
INCLUDE += -I $(MIPS_ADDITION_LIB_ROOT_PATH)/usr/include/libxml2
INCLUDE += -I $(MIPS_ADDITION_LIB_ROOT_PATH)/usr/include
INCLUDE += -I ../../../accapi
INCLUDE += -I ../../../accapi/nm/app/eag
INCLUDE += -I ../../../had/src/lib
INCLUDE += -I /opt/mipslibs/usr/include/
INCLUDE += -I ../../../accapi/nm/public
INCLUDE += $(DBUS_INCLUDE)
INCLUDE += -I ../
#for openssl 
INCLUDE += -I ../../../asd/src/app/wapi/include/

XML_LIB = $(LD_PATH) -lz
#DBUS_LIB = -L$(MIPS_ADDITION_LIB_ROOT_PATH)/lib -ldbus-1
LDFLAGS += $(XML_LIB)
LDFLAGS += $(DBUS_LIBS)
LDFLAGS += -L$(MIPS_ADDITION_LIB_ROOT_PATH)/lib
LDFLAGS += -lpthread
LOCAL_LIB = -L ./
#for openssl
LDFLAGS += -L ../../../asd/src/app/wapi -lwcrypto

#soruces := $(shell ls *.c|grep -v main|grep -v radius|grep -v portal|grep -v redir|grep -v eag_ins|grep -v eag_thread)
iptcsource = ../../../iptables/debsrc/iptables-1.4.6/libiptc/.libs/lib*.a

publicsources = eag_log.o eag_blkmem.o  eag_thread.o eag_errcode.o eag_hansi.o eag_time.o hashtable.o eag_dbus.o eag_util.o eag_ifipaddr.o \
		nmp_process.o md5.o portal_packet.o radius_packet.o

eagsources =  eag_main.o eag_radius.o eag_redir.o eag_portal.o eag_stamsg.o eag_fastfwd.o\
		eag_captive.o appconn.o  eag_license.o eag_wireless.o  \
		eag_authorize.o eag_conf.o \
		eag_ins.o  eag_ipinfo.o eag_statistics.o eag_iptables.o \
		rdc_handle.o pdc_handle.o eag_coa.o eag_ipset.o eag_trap.o eag_macauth.o eag_arp.o

rdcsources = rdc_main.o rdc_ins.o rdc_pktconn.o rdc_server.o rdc_client.o rdc_userconn.o rdc_coa.o rdc_coaconn.o

pdcsources = pdc_main.o pdc_ins.o pdc_server.o pdc_client.o pdc_userconn.o pdc_usermap.o

libeagintfsources=eag_conf.o eag_interface.o rdc_interface.o pdc_interface.o


all:libeagintf.so pdc rdc eag
	[ -d ../../../../auteware/files/opt/bin/ ] || mkdir -p ../../../../auteware/files/opt/bin/
	[ -d ../../../../auteware/files/opt/lib/ ] || mkdir -p ../../../../auteware/files/opt/lib/
	cp eag ../../../../auteware/files/opt/bin/
	cp pdc ../../../../auteware/files/opt/bin/
	cp rdc ../../../../auteware/files/opt/bin/
	cp libeagintf.so $(LIB_EXPORT_DIR)
	-ln -s $(shell pwd)/eag_conf.h $(shell pwd)/../../../accapi/nm/app/eag/eag_conf.h
	-ln -s $(shell pwd)/eag_interface.h $(shell pwd)/../../../accapi/nm/app/eag/eag_interface.h
	-ln -s $(shell pwd)/rdc_interface.h $(shell pwd)/../../../accapi/nm/app/eag/rdc_interface.h
	-ln -s $(shell pwd)/pdc_interface.h $(shell pwd)/../../../accapi/nm/app/eag/pdc_interface.h
	-ln -s $(shell pwd)/eag_errcode.h $(shell pwd)/../../../accapi/nm/app/eag/eag_errcode.h
	-ln -s $(shell pwd)/eag_dbus.h $(shell pwd)/../../../accapi/nm/app/eag/eag_dbus.h
	-ln -s $(shell pwd)/eag_trap.h $(shell pwd)/../../../accapi/nm/app/eag/eag_trap.h

pdc:$(publicsources) $(pdcsources)
	$(CC) $^ $(LDFLAGS) -ldl -o $@

rdc:$(publicsources) $(rdcsources)
	$(CC) $^ $(LDFLAGS) -ldl -o $@

eag:$(publicsources) $(eagsources)
	$(CC) $^ $(iptcsource) $(LDFLAGS) -ldl -o $@

libeagintf.so:$(libeagintfsources)
	$(CC) $(LDFLAGS) -shared -Wl,-soname,libeagintf.so -o $@ $^

eag_interface:eag_interface.o
	$(CC) $< $(LDFLAGS) -o $@

.c.o:
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@ 


space:
	df|grep sdb2 > aaa
	awk '{printf $$4}' aaa > space
	
codeformat:
	space=$(shell df|grep sdb2|awk '{print $$4}');
	echo $(space);
	if [ $$(space) -gt 10000 ];then \
		ls *.h | xargs indent -kr -i8 -ts8 -sob -l80 -ss -bs -psl; \
		echo "yyy";\
		ls *.c | xargs indent -kr -i8 -ts8 -sob -l80 -ss -bs -psl; \
	fi)

%.test:%.c
	$(CC) -D$(<:.c=_test)=1  $(CFLAGS) $(INCLUDE) $< -o $@ 
#	./$@; \
#	if [ $$? -eq 0 ];	then \
#		echo $(<:.c=_test_ok);\
#	else \
#		echo $(<:.c=_test_failed_failed_failed);\
#	fi;\
	rm $@;\
	

%.d:%.c
	set -e; rm -f $@;\
	$(CC) -MM $(CFLAGS) $(INCLUDE) $< > $@.$$$$;\
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@;\
	rm -f $@.$$$$;



clean:
	rm -f *.o *.d *.test $(TARGET) eag_interface eag libeag.so libeagintf.so pdc rdc
