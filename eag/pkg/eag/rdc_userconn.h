/* rdc_userconn.h */

#ifndef _RDC_USERCONN_H
#define _RDC_USERCONN_H

#include <stdint.h>
#include "rdc_def.h"

rdc_userconn_db_t * 
rdc_userconn_db_create();

int
rdc_userconn_db_destroy(rdc_userconn_db_t *db);

int
rdc_userconn_db_add(rdc_userconn_db_t *db,
						rdc_userconn_t *userconn);
int 
rdc_userconn_db_del(rdc_userconn_db_t *db,
						rdc_userconn_t *userconn);
rdc_userconn_t *
rdc_userconn_db_find_sessionid(rdc_userconn_db_t *db,
							char *sessionid );
rdc_userconn_t *
rdc_userconn_db_find_user(rdc_userconn_db_t *db,
							char *username, char *sessionid,
							uint32_t ip );
rdc_userconn_t *
rdc_userconn_db_find_username(rdc_userconn_db_t *db,
							char *username );

int
rdc_userconn_db_log_all( rdc_userconn_db_t *db );

rdc_userconn_t *
rdc_userconn_new(rdc_userconn_db_t * db, 
		char *username, char *sessionid, uint32_t userip );
int
rdc_userconn_set_client( rdc_userconn_t *userconn, uint32_t client_ip,
							uint16_t client_coa_port );

int
rdc_userconn_get_client( rdc_userconn_t *userconn, uint32_t *client_ip,
							uint16_t *client_coa_port );
char *
rdc_userconn_get_sessionid( rdc_userconn_t *userconn );
char *
rdc_userconn_get_username( rdc_userconn_t *userconn );


int
rdc_userconn_free(rdc_userconn_db_t * db, rdc_userconn_t *userconn);

int
rdc_userconn_set_last_time( rdc_userconn_t *userconn, uint32_t last_time );

int
rdc_userconn_set_start_time( rdc_userconn_t *userconn, uint32_t start_time );
int
rdc_userconn_set_last_acct_status( rdc_userconn_t *userconn, int acct_status );

int
rdc_userconn_db_check_timeout( rdc_userconn_db_t *userdb, 
			uint32_t timenow, uint32_t interval_timeout, uint32_t stop_timeout );


int
rdc_userconn_db_clear(rdc_userconn_db_t *userdb);


#endif

