/* pdc_userconn.h */

#ifndef _RDC_USERCONN_H
#define _RDC_USERCONN_H

#include "stdint.h"
#include "pdc_def.h"

pdc_userconn_db_t * 
pdc_userconn_db_create();

int
pdc_userconn_db_destroy(pdc_userconn_db_t *db);

int
pdc_userconn_db_add(pdc_userconn_db_t *db,
						pdc_userconn_t *userconn);
int 
pdc_userconn_db_del(pdc_userconn_db_t *db,
						pdc_userconn_t *userconn);

pdc_userconn_t *
pdc_userconn_db_find_user(pdc_userconn_db_t *db,
							uint32_t ip );

pdc_userconn_t *
pdc_userconn_new(pdc_userconn_db_t * db, uint32_t userip );
int
pdc_userconn_set_hansi( pdc_userconn_t *userconn, uint8_t slot_id,
							uint8_t hansi_type, uint8_t hansi_id );

int
pdc_userconn_get_hansi( pdc_userconn_t *userconn, uint8_t *slot_id,
							uint8_t *hansi_id );
int
pdc_userconn_get_eaginfo( pdc_userconn_t *userconn, 
								uint32_t *eagip, uint16_t *eagport );

int
pdc_userconn_free(pdc_userconn_db_t * db, pdc_userconn_t *userconn);

int
pdc_userconn_set_last_time( pdc_userconn_t *userconn, uint32_t last_time );

int
pdc_userconn_set_start_time( pdc_userconn_t *userconn, uint32_t start_time );

int
pdc_userconn_set_usermac( pdc_userconn_t *userconn, uint8_t *mac );

int
pdc_userconn_get_usermac( pdc_userconn_t *userconn, uint8_t *mac );


int
pdc_userconn_db_check_timeout( pdc_userconn_db_t *userdb, 
							 uint32_t stop_timeout );

int
pdc_userconn_db_clear(pdc_userconn_db_t *userdb);


DBusMessage *
pdc_dbus_method_set_userconn(
				DBusConnection *conn,
				DBusMessage *msg,
				void *user_data );

DBusMessage *
pdc_dbus_method_log_userconn(
				DBusConnection *conn,
				DBusMessage *msg,
				void *user_data );


#endif

