#ifndef _PPPOE_RADIUS_H
#define _PPPOE_RADIUS_H

int pppoe_radius_start(radius_struct_t *radius);
void pppoe_radius_stop(radius_struct_t *radius);

radius_struct_t *pppoe_radius_init(thread_master_t *master, 
					pppoe_manage_t *manage, radius_config_t *config);
void pppoe_radius_destroy(radius_struct_t **radius);

int pppoe_radius_config_nas_ipaddr(radius_config_t *config, uint32 nasip);
int pppoe_radius_config_rdc(radius_config_t *config, uint32 state, 
									uint32 slot_id, uint32 instance_id);
int pppoe_radius_config_auth_and_acct_server(radius_config_t *config, struct radius_srv *srv);
int pppoe_radius_show_running_config(radius_config_t *config, char *cmd, uint32 len);

void pppoe_radius_config_setup(radius_config_t *config, 
				uint32 slot_id, uint32 local_id, uint32 instance_id);
int pppoe_radius_config_init(radius_config_t **config);
void pppoe_radius_config_exit(radius_config_t **config);

#endif
