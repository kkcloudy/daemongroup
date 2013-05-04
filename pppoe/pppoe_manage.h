#ifndef _PPPOE_MANAGE_H
#define _PPPOE_MANAGE_H

/* this func is not thread safe. if master thread want call it, need use tbus */
int manage_sess_kick_by_mac(pppoe_manage_t *manage, unsigned char *mac);
int manage_sess_kick_by_sid(pppoe_manage_t *manage, uint32 sid);

/* this func is not thread safe, only use by device thread */
session_struct_t *__manage_sess_get_by_mac(pppoe_manage_t *manage, unsigned char *mac);
session_struct_t *__manage_sess_get_by_sid(pppoe_manage_t *manage, uint32 sid);
struct list_head *__manage_sess_get_list(pppoe_manage_t *manage);

/* this func is thread safe */
session_struct_t *manage_sess_get_by_mac(pppoe_manage_t *manage, unsigned char *mac);
session_struct_t *manage_sess_get_by_sid(pppoe_manage_t *manage, uint32 sid);

int _manage_sess_offline(session_struct_t *sess);
void _manage_sess_exit(session_struct_t *sess);
void manage_sess_exit(session_struct_t *sess);
session_struct_t *manage_sess_init(pppoe_manage_t *manage, uint8 *mac, uint8 *serverMac);

int manage_sessions_opt_register(pppoe_manage_t *manage, sessionOptType optType, 
										void *arg, sessOptFunc func);
int manage_sessions_opt_unregister(pppoe_manage_t *manage, sessionOptType optType);

int manage_sess_online_sync(pppoe_manage_t *manage, struct pppoe_buf *pbuf);
int manage_sess_offline_sync(pppoe_manage_t *manage, struct pppoe_buf *pbuf);
int manage_sess_update_sync(pppoe_manage_t *manage, struct pppoe_buf *pbuf);


/* this func is not thread safe. if master thread want call it, need use tbus */
uint32 manage_sync_flag(pppoe_manage_t *manage);
void manage_sessions_sync_finished(pppoe_manage_t *manage);
void manage_sessions_sync(pppoe_manage_t *manage);
void manage_sessions_clear(pppoe_manage_t *manage);
void manage_sessions_clear_v2(pppoe_manage_t *manage);

int pppoe_manage_start(pppoe_manage_t *manage);
int pppoe_manage_stop(struct pppoe_manage *manage);
int pppoe_manage_restart(pppoe_manage_t *manage);


pppoe_manage_t *pppoe_manage_init(thread_master_t *master, 
									backup_struct_t *backup,
									tbus_connection_t *connection,
									manage_config_t *config);
void pppoe_manage_destroy(pppoe_manage_t **manage);

int pppoe_manage_config_sessions_max_sid(manage_config_t *config, uint32 max_sid);
int pppoe_manage_config_sessions_ipaddr(manage_config_t *config, uint32 minIP, uint32 maxIP);
int pppoe_manage_config_sessions_dns(manage_config_t *config, uint32 dns1, uint32 dns2);
int pppoe_manage_show_online_user(pppoe_manage_t *manage, struct pppoeUserInfo **userList, uint32 *userNum);
int pppoe_manage_show_running_config(manage_config_t *config, char *cmd, uint32 len);

void pppoe_manage_config_setup(manage_config_t *config, 
						uint32 slot_id, uint32 local_id, uint32 instance_id,
						int ifindex, uint32 dev_id, uint32 ipaddr, char *ifname);
int pppoe_manage_config_init(manage_config_t **config);
void pppoe_manage_config_exit(manage_config_t **config);

#endif
