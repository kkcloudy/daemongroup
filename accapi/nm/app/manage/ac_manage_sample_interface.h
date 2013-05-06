#ifndef _AC_MANAGE_SAMPLE_INTERFACE_H_
#define _AC_MANAGE_SAMPLE_INTERFACE_H_

struct instance_radius_config {
    unsigned int slot_id;
    unsigned int local_id;
    unsigned int instance_id;

    struct radius_conf radiusconf;

    struct instance_radius_config *next;
};

struct instance_portal_config {
    unsigned int slot_id;
    unsigned int local_id;
    unsigned int instance_id;

    struct portal_conf portalconf;

    struct instance_portal_config *next;
};

int ac_manage_show_radius_config(DBusConnection *connection, struct instance_radius_config **config_array, unsigned int *config_num);

int ac_manage_show_portal_config(DBusConnection *connection, struct instance_portal_config **config_array, unsigned int *config_num);



#endif
