#ifndef _PPPOE_DISCOVER_H
#define _PPPOE_DISCOVER_H

int pppoe_discover_start(discover_struct_t *discover);
int pppoe_discover_stop(discover_struct_t *discover);

discover_struct_t *pppoe_discover_init(thread_master_t *master,
										pppoe_manage_t *manage,
										discover_config_t *config);
void pppoe_discover_destroy(discover_struct_t **discover);

int pppoe_discover_config_virtual_mac(discover_config_t *config, uint8 *virtualMac);
int pppoe_discover_show_running_config(discover_config_t *config, char *cmd, uint32 len);
void pppoe_discover_config_setup(discover_config_t *config, char *base_ifname, char *sname);
int pppoe_discover_config_init(discover_config_t **config);
void pppoe_discover_config_exit(discover_config_t **config);

#endif
