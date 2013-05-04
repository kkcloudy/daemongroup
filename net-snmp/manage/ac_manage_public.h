#ifndef _AC_MANAGE_PUBLIC_H_
#define _AC_MANAGE_PUBLIC_H_



struct pfmOptParameter{
    int pfm_opt;
    int pfm_opt_para;
    unsigned short pfm_protocol;
    
    char *ifName;
    
    char *src_ipaddr;
    unsigned int src_port;
    
    char *dest_ipaddr;
    unsigned int dest_port;
    
    int slot_id;    
};

struct running_config {
    char showStr[256];

    struct running_config *next;
};


extern DBusConnection *tipc_connection[SLOT_MAX_NUM + 1];
extern unsigned int local_slotID; 
extern unsigned int active_master_slotID;


DBusConnection *manage_dbus_bus_get_tipc_connection(unsigned int slot_id);

void manage_dbus_tipc_connection_close(DBusConnection **connection);

void init_manage_tipc_dbus(void);

void manage_free_master_instance_para(instance_parameter **paraHead);

int manage_get_master_instance_para(instance_parameter **paraHead);

int manage_get_product_info(const char *filename);

int manage_ifname_is_legal_input(const char *ifname);

int manage_get_slot_id_by_ifname(const char *ifname);

int manage_config_pfm_table_entry(struct pfmOptParameter *pfmParameter);

struct running_config *manage_new_running_config(void);

void manage_insert_running_config(struct running_config **head,
									struct running_config **end,
									struct running_config *temp);

void manage_free_running_config(struct running_config **configHead);


#endif
