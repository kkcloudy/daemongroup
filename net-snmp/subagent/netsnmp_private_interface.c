
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include <syslog.h>

#include "ac_manage_def.h"

#include "netsnmp_private_interface.h"

static netsnmp_log_handler *private_log_handler = NULL;


void 
netsnmp_register_item(netsnmp_item_register **item_register, void *item) {
    snmp_log(LOG_DEBUG, "enter netsnmp_register_item\n");
    
    if(NULL == item_register || NULL == item) {
        snmp_log(LOG_ERR, "netsnmp_register_item: Input para is error\n");
        return ;
    }

    netsnmp_item_register *temp_register = (netsnmp_item_register *)malloc(sizeof(netsnmp_item_register));
    if(NULL == temp_register) {
        snmp_log(LOG_ERR, "netsnmp_register_item: malloc temp_register fail!\n");
        return ;
    }
    memset(temp_register, 0, sizeof(*temp_register));

    temp_register->item = item;
    temp_register->next = *item_register;
    *item_register = temp_register;
    
    snmp_log(LOG_DEBUG, "exit netsnmp_register_item\n");
    return ;
}

void 
netsnmp_container_load_item_register(netsnmp_container *container, 
                                                netsnmp_item_register **item_register, 
                                                netsnmp_item_release_func *release_func) {
    snmp_log(LOG_DEBUG, "enter netsnmp_container_load_item_register\n");
    
    if(NULL == container || NULL == item_register || NULL == release_func) {
        snmp_log(LOG_ERR, "netsnmp_container_load_item_register: Input para is error\n");
        return ;
    }

    while(*item_register) {
        if(CONTAINER_INSERT(container, (*item_register)->item)) {
            snmp_log(LOG_INFO, "netsnmp_container_load_item_register: container insert fail!\n");
            release_func((*item_register)->item);
        }
        
        netsnmp_item_register *temp = (*item_register)->next;
        SNMP_FREE(*item_register);
        *item_register = temp;
    }
    
    snmp_log(LOG_DEBUG, "exit netsnmp_container_load_item_register\n");
    return ;
}

void
netsnmp_config_log_debug(unsigned int debugLevel) {
    
    switch(debugLevel) {
        case 0:
            setlogmask(LOG_ON_INFO); 
            netsnmp_remove_loghandler(private_log_handler);
            private_log_handler = netsnmp_register_loghandler(NETSNMP_LOGHANDLER_SYSLOG, LOG_ERR);;
            syslog(LOG_INFO, "close snmp log debug......\n");
            break;
            
        case 1:
            setlogmask(LOG_ON_DEBUG);         
            netsnmp_remove_loghandler(private_log_handler);
            private_log_handler = netsnmp_register_loghandler(NETSNMP_LOGHANDLER_SYSLOG, LOG_ERR);;
            syslog(LOG_DEBUG, "open snmp dbus list debug......\n");
            break;
            
        case 2:                
            setlogmask(LOG_ON_DEBUG); 
            netsnmp_remove_loghandler(private_log_handler);
            private_log_handler = netsnmp_register_loghandler(NETSNMP_LOGHANDLER_SYSLOG, LOG_DEBUG);
            syslog(LOG_DEBUG, "open snmp dbus list and source code debug......\n");
            break;
            
        default :
            setlogmask(LOG_ON_INFO);    
            netsnmp_remove_loghandler(private_log_handler);
            private_log_handler = netsnmp_register_loghandler(NETSNMP_LOGHANDLER_SYSLOG, LOG_ERR);;
            syslog(LOG_INFO, "ac_manage_show_snmp_log_debug: return unkown debugLevel %d, so close snmp log debug\n", debugLevel);
            break;
    }

    return ;
}

