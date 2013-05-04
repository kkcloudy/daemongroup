/* rdc_coa.h */

#ifndef _RDC_COA_H
#define _RDC_COA_H

#include <stdint.h>
#include "rdc_def.h"

rdc_coa_t *
rdc_coa_new(void);

int
rdc_coa_free(rdc_coa_t *coa);

int
rdc_coa_start(rdc_coa_t *coa);

int
rdc_coa_stop(rdc_coa_t *coa);

int
rdc_coa_send_response(rdc_coa_t *coa,
						rdc_coaconn_t *coaconn,
						struct rdc_packet_t *rdcpkt);

int 
rdc_coa_set_thread_master(rdc_coa_t *coa,
								eag_thread_master_t *master);

int 
rdc_coa_set_rdcins(rdc_coa_t *coa,
						rdc_ins_t *rdcins);

int 
rdc_coa_set_nasip(rdc_coa_t *coa,
						uint32_t ip);

int 
rdc_coa_set_port(rdc_coa_t *coa,
						uint16_t port);

int
rdc_coa_set_userdb( rdc_coa_t *coa, rdc_userconn_db_t *userdb );
int
rdc_coa_set_server( rdc_coa_t *coa,
					rdc_server_t *server );
int
rdc_coa_set_coadb( rdc_coa_t *coa,
					rdc_coaconn_db_t *coadb );

struct  radius_srv_coa  *
rdc_coa_check_radius_srv( struct  rdc_coa_radius_conf *radiusconf, const unsigned long ip,const unsigned short  port,char * secret);

int
rdc_coa_set_radius_srv(struct radius_srv_coa *radius_srv,
		    unsigned long auth_ip,
		    unsigned short auth_port,
		    char *auth_secret, unsigned int auth_secretlen);
int
rdc_coa_del_radius_srv( struct rdc_coa_radius_conf *radiusconf,
		    unsigned long auth_ip, unsigned short auth_port,char *auth_secret );

int 
rdc_coa_set_radiusconf( rdc_coa_t *this, struct rdc_coa_radius_conf *radiusconf );

struct radius_srv_coa *
rdc_coa_conf_radius_srv(struct rdc_coa_radius_conf *radiusconf);


#endif        /* _RDC_SERVER_H */

