#ifndef _PPPOE_CONTROL_H
#define _PPPOE_CONTROL_H


int pppoe_control_start(control_struct_t *control);
int pppoe_control_stop(control_struct_t *control);

control_struct_t * pppoe_control_init(thread_master_t *master,
								pppoe_manage_t *manage,
								control_config_t *config);
void pppoe_control_destroy(control_struct_t **control);

int pppoe_control_show_running_config(control_config_t *ctlConf, char *cmd, uint32 len);
void pppoe_control_config_setup(control_config_t *config, char *ifname);
int pppoe_control_config_init(control_config_t **config);
void pppoe_control_config_exit(control_config_t **config);

#endif
