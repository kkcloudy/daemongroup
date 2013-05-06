#ifndef _WS_DBUS_LIST_INTERFACE_H_
#define _WS_DBUS_LIST_INTERFACE_H_

#include <dbus/dbus.h>
#include "ws_dbus_def.h"


#define SHOW_ALL_WTP_TABLE_METHOD       1


typedef struct {
    unsigned int slot_id;
    unsigned int vrrp_state;
    unsigned int local_id;
    unsigned int instance_id;
} dbus_parameter;

typedef struct snmpd_dbus_message_s{
    void *message;
    dbus_parameter parameter;
    struct snmpd_dbus_message_s *next;
} snmpd_dbus_message;

typedef struct instance_parameter_s {
    dbus_parameter parameter;
    DBusConnection *connection;
    struct instance_parameter_s *next;
} instance_parameter;


typedef int (show_all_wtp_table)(dbus_parameter, 
                                 DBusConnection *, 
                                 void **);
                                     
typedef void (free_table_message)(void *);


extern int distributed_flag;
extern int get_product_info(char * filename);

void init_dbus_connection_list(void);
void uninit_dbus_connection_list(void);
DBusConnection *init_slot_dbus_connection(unsigned int slot_id);
void *tipc_dbus_connection_pthread(void);
snmpd_dbus_message *list_connection_call_dbus_method(void *method, int flag, ...);
void free_dbus_message_list(snmpd_dbus_message **messageHead, void *method);
void list_instance_parameter(instance_parameter **paraHead,int flag); //SNMPD_INSTANCE_ALL
                                                                            //SNMPD_INSTANCE_MASTER
                                                                            //SNMPD_INSTANCE_BACK
void free_instance_parameter_list(instance_parameter **paraHead);
void close_slot_dbus_connection(unsigned int slot_id);
int get_slot_dbus_connection(unsigned int slot_id, void **paraHead, int flag); //SNMPD_INSTANCE_ALL
                                                                              //SNMPD_INSTANCE_MASTER
                                                                              //SNMPD_INSTANCE_BACK
                                                                              //SNMPD_INSTANCE_MASTER_V2
                                                                              //SNMPD_INSTANCE_MASTER_V3 :snmpd use it , paraHead is connection
int get_instance_dbus_connection(dbus_parameter parameter, void **paraHead, int flag); //SNMPD_INSTANCE_ALL,
                                                                                                //SNMPD_INSTANCE_MASTER   
                                                                                                //SNMPD_INSTANCE_BACK
                                                                                                //SNMPD_INSTANCE_MASTER_V3 :snmpd use it , paraHead is connection
unsigned long local_to_global_ID(dbus_parameter parameter, unsigned long id, unsigned long max_num);
unsigned long global_to_local_ID(dbus_parameter *parameter, unsigned long id, unsigned long max_num);
void get_slotID_localID_instanceID(char *plotid,dbus_parameter *parameter);/*返回0表示失败，返回1表示成功*/

int get_local_slot_id(void);

#endif
