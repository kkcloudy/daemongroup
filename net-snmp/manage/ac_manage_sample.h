#ifndef _AC_MANAGE_SAMPLE_H_
#define _AC_MANAGE_SAMPLE_H_

void manage_free_radius_config(struct instance_radius_config **configHead);

int manage_get_master_radius_config(struct instance_radius_config **configHead, unsigned int *config_num);

void manage_free_portal_config(struct instance_portal_config **configHead);

int manage_get_master_portal_config(struct instance_portal_config **configHead, unsigned int *config_num);

#endif
