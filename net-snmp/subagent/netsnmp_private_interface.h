#ifndef _NETSNMP_PRIVATE_INTERFACE_H_
#define _NETSNMP_PRIVATE_INTERFACE_H_


typedef void (netsnmp_item_release_func)(void*);


typedef struct netsnmp_item_register_s {
    void *item;
    struct netsnmp_item_register_s *next;
    
}netsnmp_item_register;



void netsnmp_register_item(netsnmp_item_register **item_register, void *item);

void netsnmp_container_load_item_register(netsnmp_container *container, 
                                                    netsnmp_item_register **item_register, 
                                                    netsnmp_item_release_func *release_func);

void netsnmp_config_log_debug(unsigned int debugLevel);

#endif
