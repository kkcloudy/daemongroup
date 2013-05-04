#ifndef _THREAD_BUS_H
#define _THREAD_BUS_H

/* thread bus flag */
enum {
	TBUS_FLAG_REPLY		= 0x1,
	TBUS_FLAG_ERRFREE 	= 0x2,
	TBUS_FLAG_UNHANDLE	= 0x4,
};


int tbus_send_signal(tbus_connection_t *connection, uint32 dest,
						uint32 method_id, void *data, void *para, 
						tbusDataFree dfree, uint32 flag);
int tbus_send_method_call_with_reply(tbus_connection_t *connection, uint32 dest,
											uint32 method_id, void *data, void *para,
											tbusDataFree dfree, uint32 flag, uint32 timeout);

int tbus_connection_dispatch(tbus_connection_t *connection, uint32 len);
tbus_connection_t *tbus_connection_create(thread_bus_t *tbus, uint32 connect_id);
void tbus_connection_destroy(tbus_connection_t **connection);

thread_bus_t *thread_bus_init(uint32 connection_num);
void thread_bus_exit(thread_bus_t **tbus);


#endif
