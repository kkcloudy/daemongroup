/* pdc_server.h */

#ifndef _PDC_SERVER_H
#define _PDC_SERVER_H

#include "stdint.h"
#include "pdc_def.h"
#include "pdc_packet.h"

pdc_server_t *
pdc_server_new(void);

int
pdc_server_free(pdc_server_t *server);

int 
pdc_server_set_thread_master(pdc_server_t *server,
								eag_thread_master_t *master);

int 
pdc_server_set_pdcins(pdc_server_t *server,
						pdc_ins_t *pdcins);

int 
pdc_server_set_ip(pdc_server_t *server,
						uint32_t ip);

int 
pdc_server_set_port(pdc_server_t *server,
						uint16_t port);

int
pdc_server_start(pdc_server_t *server);

int
pdc_server_stop(pdc_server_t *server);

int
pdc_server_send_packet(pdc_server_t *server,
								struct pdc_packet_t *pdc_packet);

DBusMessage *
pdc_dbus_method_add_map(
				DBusConnection *conn,
				DBusMessage *msg,
				void *user_data );

DBusMessage *
pdc_dbus_method_modify_map(
				DBusConnection *conn,
				DBusMessage *msg,
				void *user_data );

DBusMessage *
pdc_dbus_method_del_map(
				DBusConnection *conn,
				DBusMessage *msg,
				void *user_data );

DBusMessage *
pdc_dbus_method_show_maps(
				DBusConnection *conn,
				DBusMessage *msg,
				void *user_data );

DBusMessage *
pdc_dbus_method_add_ipv6_map(
				DBusConnection *conn,
				DBusMessage *msg,
				void *user_data );

DBusMessage *
pdc_dbus_method_del_ipv6_map(
				DBusConnection *conn,
				DBusMessage *msg,
				void *user_data );

DBusMessage *
pdc_dbus_method_modify_ipv6_map(
				DBusConnection *conn,
				DBusMessage *msg,
				void *user_data );

DBusMessage *
pdc_dbus_method_show_ipv6_maps(
				DBusConnection *conn,
				DBusMessage *msg,
				void *user_data );

#endif        /* _RDC_SERVER_H */
