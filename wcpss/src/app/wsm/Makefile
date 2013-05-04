CC = mips-linux-gnu-gcc
#CC = gcc

LDFLAGS += -L../pub -lwcpsspub -lpthread -L$(MIPS_ADDITION_LIB_ROOT_PATH)/usr/lib 
#-lpthread -ldl

CFLAGS += -DCW_NO_DTLS -I. -g -I../pub -DWRITE_STD_OUTPUT -DCW_DEBUGGING -D_GNU_SOURCE -D_DISTRIBUTION_
CFLAGS += -I$(ACCAPI_DIR)

RM = /bin/rm -f

WSM_OBJS = wsm_main.o wsm_main_tbl.o wsm_wifi_bind.o wsm_frame_transform.o wsm_shared_mem.o wsm_dbus.o

#WSM_SRCS = $(WID_OBJS:.o=.c)

wsm : $(WSM_OBJS)
	$(CC) $(WSM_OBJS) $(CFLAGS) $(DBUS_LIBS) $(LDFLAGS) -o wsm

%.o: %.c
	$(CC) -c $(CFLAGS) $(DBUS_INCLUDE) -o $*.o $<

.PHONY: clean

clean: 
	$(RM) wsm $(WSM_OBJS)
