#include <syslog.h>
#include "ws_dbus_list_interface.h"
#include "ws_dbus_list.h"
#include "dbus/wcpss/ACDbusDef1.h"
#include "ac_manage_def.h"
#include "ws_snmpd_engine.h"
#include "ws_snmpd_manual.h"
#include "ac_manage_interface.h"

#include "ws_public.h"


extern dbus_connection_list snmpd_dbus_connection_list;
extern unsigned int snmp_collection_mode;
extern DBusConnection *ccgi_dbus_connection;

void 
snmpd_collect_mode_init(void){
    snmpd_dbus_connection_init();

    STSNMPSysInfo snmp_info;
    int manage_ret = AC_MANAGE_SUCCESS;
    manage_ret = ac_manage_show_snmp_base_info(ccgi_dbus_connection, &snmp_info);
    if(AC_MANAGE_SUCCESS == manage_ret) {
        snmp_collection_mode = snmp_info.collection_mode;
        syslog(LOG_NOTICE, "The snmp collection mode is %s\n", snmp_collection_mode ? "decentral" : "concentre");
    }
    else {
        syslog(LOG_NOTICE, "snmpd_collect_mode_init:ac_manage_show_snmp_base_info get failed!\n");
    }

    return ;
}
void init_dbus_connection_list(void)
{
    init_distributed_flag();
    snmpd_dbus_connection_list_init();
    dl_dcli_init();
    snmpd_vrrp_state_init();
    
    return ;
}
void uninit_dbus_connection_list(void)
{
    return uninit_snmpd_dbus_connection_list();
}

DBusConnection*
init_slot_dbus_connection(unsigned int slot_id) {

    if(slot_id == snmpd_dbus_connection_list.local_slot_num) {
        syslog(LOG_DEBUG, "This connection is local , don`t need init it\n");
        return ccgi_dbus_connection;
    }

    struct list_head *pos = NULL;
    list_for_each(pos, &snmpd_dbus_list_head) {
        netsnmp_dbus_connection *snmpd_dbus_node = dbus_node(pos);
        if(slot_id == snmpd_dbus_node->slot_id) {

            if(snmpd_dbus_node->connection) {
                snmpd_close_dbus_connection(&(snmpd_dbus_node->connection));
            }

            snmpd_dbus_node->connection = dbus_get_tipc_connection(slot_id);

            return snmpd_dbus_node->connection;
        }
    }

    return NULL;
}

void *tipc_dbus_connection_pthread(void)
{
    snmp_pid_write(SNMP_THREAD_INFO_PATH, "snmp tipc dbus connection thread");
    
	while(1) {
		tipc_dbus_connection_maintenance();
		sleep(10);
	}
	return NULL;
}

void
had_hansi_message_handler(int state) {
    return snmpd_had_hansi_advertise(state);
}


static snmpd_dbus_message *init_new_message(dbus_parameter parameter, void *message)
{
    snmpd_dbus_message *temp_message = NULL;
    if(NULL == (temp_message = (snmpd_dbus_message *)malloc(sizeof(snmpd_dbus_message)) )) {  
        syslog(LOG_DEBUG, "new_dbus_message: malloc slot %d ,vrrp type %d, instance %d error\n", 
                            parameter.slot_id, parameter.local_id,  parameter.instance_id);
        return NULL;
    }
    
    memset(temp_message, 0, sizeof(snmpd_dbus_message));
    memcpy(&temp_message->parameter, &parameter, sizeof(dbus_parameter));
    temp_message->message = message;
    
    return temp_message;
    
}
static snmpd_dbus_message *new_dbus_message(dbus_parameter parameter, 
                                                    DBusConnection *connection, 
                                                    void *method, 
                                                    int flag, 
                                                    va_list var_args)
{
    if(NULL == connection) 
        return NULL;
    
    int ret = 0;
    void *message = NULL;
    switch(flag) {
        case SHOW_ALL_WTP_TABLE_METHOD: 
            ret = ((show_all_wtp_table *)method)(parameter, connection, &message);
            break;
		/*wangchao add*/	
		case SHOW_WTP_WIFI_LOCATE_PUBLIC_CONIFG_METHOD:
			ret = ((wifi_location*)method)(parameter, connection, &message, va_arg(var_args, long));
            break;
        default:
            syslog(LOG_DEBUG, "unknown method %d in new_dbus_message", flag);
            break;
    }

    syslog(LOG_DEBUG, "new_dbus_message: slot_id %d /local_id %d /instance_id %d get message %p, ret = %d\n",
                       parameter.slot_id, parameter.local_id, parameter.instance_id, message, ret);

    if(1 == ret && message) {
        snmpd_dbus_message *temp_message = init_new_message(parameter, message);
        return temp_message;
    }
    else if (SNMPD_CONNECTION_ERROR == ret) {
        close_slot_dbus_connection(parameter.slot_id);
		//syslog(LOG_INFO,"####close_slot_dbus_connection slot %d####\n",parameter.slot_id);
    } 
    
    return NULL;
        
}

static void init_dbus_parameter(dbus_parameter *parameter, 
                                     netsnmp_dbus_connection *dbus_node, 
                                     int vrrp_state, 
                                     int local_id , 
                                     int instance_id)
{
    memset(parameter, 0, sizeof(dbus_parameter));

    parameter->slot_id = dbus_node->slot_id;
    parameter->vrrp_state = vrrp_state;
    parameter->local_id = local_id;
    parameter->instance_id = instance_id;
    
    return; 
}

static void init_instance_parameter(instance_parameter **para,
                                          netsnmp_dbus_connection *dbus_node, 
                                          int vrrp_state, 
                                          unsigned int local_id , 
                                          unsigned int instance_id)
{
    if(NULL == para || NULL == dbus_node)
        return;

    *para = NULL;
    
    instance_parameter *temp = (instance_parameter *)malloc(sizeof(instance_parameter));
    if(temp) {
        memset(temp, 0, sizeof(instance_parameter));
        
        init_dbus_parameter(&temp->parameter,
                            dbus_node,
                            vrrp_state,
                            local_id,
                            instance_id);
                            
        temp->connection = dbus_node->connection;
    }
    
    *para = temp;
    return;
}
static void new_slot_instance_message(snmpd_dbus_message ** head_message, 
                                                netsnmp_dbus_connection *dbus_node, 
                                                void *method, 
                                                int flag, 
                                                va_list var_args)
{
    if(NULL == dbus_node || NULL == dbus_node->connection)
        return;
        
    snmpd_dbus_message *temp_head = *head_message;
    
    int i ;
    for(i = 0; i < VRRP_TYPE_NUM; i++) {
        int j ;
        for(j = 0; j < dbus_node->master_instance_count[i]; j++) {

            dbus_parameter parameter;
            init_dbus_parameter(&parameter, 
                                dbus_node, 
                                SNMPD_VRRP_MASTER, 
                                i, 
                                dbus_node->master_instance[i][j]);

            
            snmpd_dbus_message *new_message = new_dbus_message(parameter, 
                                                               dbus_node->connection, 
                                                               method, 
                                                               flag, 
                                                               var_args);
                                                               
            if(NULL != new_message) {
                new_message->next = temp_head;
                temp_head = new_message;
            }
        }
    }
    
    *head_message = temp_head;
}

static void new_slot_instance_parameter(instance_parameter **head_para, netsnmp_dbus_connection *dbus_node, int flag)
{
    if(NULL == dbus_node || NULL == dbus_node->connection)
        return ;
        
    instance_parameter *temp_head = *head_para, *temp_last = NULL;
  
    int i , j;
    switch(flag) {        
        case SNMPD_INSTANCE_ALL:
            for(i = 0; i < VRRP_TYPE_NUM; i++) {
                for(j = 0; j <= INSTANCE_NUM; j++) {
                    if(SNMPD_NO_VRRP == dbus_node->instance_state[i][j])
                        continue;
                        
                    instance_parameter *para = NULL;
                    init_instance_parameter(&para,
                                            dbus_node,
                                            dbus_node->instance_state[i][j], 
                                            i, 
                                            j);
                                            
                    if(NULL == para) {
                        syslog(LOG_DEBUG, "new_slot_instance_parameter: malloc (instance_parameter) error\n");
                        continue;
                    }
                    
                    
                    if(NULL == temp_last) {
                        para->next = temp_head;
                        temp_head = para;
                        
                    }
                    else {
                        para->next = temp_last->next;
                        temp_last->next = para;
                    }
                    temp_last = para;
                }
            } 
            break;
    
        case SNMPD_INSTANCE_MASTER:
            for(i = 0; i < VRRP_TYPE_NUM; i++) {
                for(j = 0; j < dbus_node->master_instance_count[i]; j++) {

                    instance_parameter *para = NULL;
                    init_instance_parameter(&para,
                                            dbus_node,
                                            SNMPD_VRRP_MASTER, 
                                            i, 
                                            dbus_node->master_instance[i][j]);
                                            
                    if(NULL == para) {
                        syslog(LOG_DEBUG, "new_slot_instance_parameter: malloc (instance_parameter) error\n");
                        continue;
                    }
                    

                    if(NULL == temp_last) {
                        para->next = temp_head;
                        temp_head = para;
                        
                    }
                    else {
                        para->next = temp_last->next;
                        temp_last->next = para;
                    }
                    temp_last = para;
                }
            } 
            break;
        
        case SNMPD_INSTANCE_BACK:
            for(i = 0; i < VRRP_TYPE_NUM; i++) {
                for(j = 0; j <= INSTANCE_NUM; j++) {
                    if(SNMPD_VRRP_BACK != dbus_node->instance_state[i][j])
                        continue;

                    instance_parameter *para = NULL;
                    init_instance_parameter(&para,
                                            dbus_node,
                                            dbus_node->instance_state[i][j], 
                                            i, 
                                            j);
                                            
                    if(NULL == para) {
                        syslog(LOG_DEBUG, "new_slot_instance_parameter: malloc (instance_parameter) error\n");
                        continue;
                    }
                    
                    if(NULL == temp_last) {
                        para->next = temp_head;
                        temp_head = para;
                    }
                    else {
                        para->next = temp_last->next;
                        temp_last->next = para;
                    }
                    temp_last = para;
                }
            } 
            break;
          
        case SNMPD_INSTANCE_MASTER_V2:
            for(i = 0; i < VRRP_TYPE_NUM; i++) {
                for(j = 0; j < dbus_node->master_instance_count[i]; j++) {

                    instance_parameter *para = NULL;
                    init_instance_parameter(&para,
                                            dbus_node,
                                            SNMPD_VRRP_MASTER, 
                                            i, 
                                            dbus_node->master_instance[i][j]);
                                            
                    if(NULL == para) {
                        syslog(LOG_DEBUG, "new_slot_instance_parameter: malloc (instance_parameter) error\n");
                        return ;
                    }
                     
                    para->next = temp_head;
                    *head_para = para;
                    return ;
                }
            } 
            break;

        case SNMPD_SLOT_CONNECT:
            {
                instance_parameter *para = NULL;
                init_instance_parameter(&para,
                                        dbus_node,
                                        SNMPD_VRRP_MASTER, 
                                        0, 
                                        0);
                                        
                if(NULL == para) {
                    syslog(LOG_DEBUG, "new_slot_instance_parameter: malloc (instance_parameter) error\n");
                    return ;
                }
                
                para->next = temp_head;
                temp_head = para;   
            }
            break;

        case SNMPD_SLOT_MASTER_CONNECT:
            {
                if(BOARD_ENABLE == dbus_node->bInfo.board_type && 
                    1 == dbus_node->bInfo.board_type &&
                    1 == dbus_node->bInfo.board_active) {
                    
                    instance_parameter *para = NULL;
                    init_instance_parameter(&para,
                                            dbus_node,
                                            SNMPD_VRRP_MASTER, 
                                            0, 
                                            0);
                                            
                    if(NULL == para) {
                        syslog(LOG_DEBUG, "new_slot_instance_parameter: malloc (instance_parameter) error\n");
                        return ;
                    }
                    
                    para->next = temp_head;
                    temp_head = para;   
                }
                else {
                    syslog(LOG_DEBUG, "This slot %d is not active master\n", dbus_node->slot_id);
                }
            }
            break;
                               
        default:
            syslog(LOG_DEBUG, "new_slot_instance_parameter: The flag %d is not find\n", flag);
            break;
    }

    *head_para = temp_head;

    return ;
}


static void new_vrrp_instance_parameter(void **head_para, netsnmp_dbus_connection *dbus_node, dbus_parameter parameter,int flag)
{
    if(NULL == dbus_node || NULL == dbus_node->connection)
        return;
        
    instance_parameter *temp_head = *head_para;
    
    switch(flag) {
        case SNMPD_INSTANCE_ALL:
            if(SNMPD_VRRP_MASTER == dbus_node->instance_state[parameter.local_id][parameter.instance_id] ||
               SNMPD_VRRP_BACK == dbus_node->instance_state[parameter.local_id][parameter.instance_id]) {
                break;
            }
            return ;
    
        case SNMPD_INSTANCE_MASTER:
            if(SNMPD_VRRP_MASTER == dbus_node->instance_state[parameter.local_id][parameter.instance_id]) {
               break;
            }
            return ;
                            
        case SNMPD_INSTANCE_BACK:
            if(SNMPD_VRRP_BACK == dbus_node->instance_state[parameter.local_id][parameter.instance_id]){
                break;
            }    
            return ;  
            
        case SNMPD_INSTANCE_MASTER_V3:
            if(SNMPD_VRRP_MASTER == dbus_node->instance_state[parameter.local_id][parameter.instance_id]) {
               *head_para = dbus_node->connection;
               syslog(LOG_DEBUG, "SNMPD_INSTANCE_MASTER_V3: find the local %d instance %d in slot %d \n", parameter.local_id, parameter.instance_id, dbus_node->slot_id);
            }
            return;
            
        default:
            syslog(LOG_DEBUG, "new_vrrp_instance_parameter: The flag() is can`t find \n", flag);
            return ;
            
    }
    
    instance_parameter *para = NULL;
    init_instance_parameter(&para,
                            dbus_node,
                            dbus_node->instance_state[parameter.local_id][parameter.instance_id], 
                            parameter.local_id, 
                            parameter.instance_id);
                            
    if(NULL == para) {
        syslog(LOG_DEBUG, "new_vrrp_instance_parameter: malloc (instance_parameter) error\n");
        return;
    }
    
    syslog(LOG_DEBUG, "new_vrrp_instance_parameter: find the local %d instance %d in slot %d \n", parameter.local_id, parameter.instance_id, dbus_node->slot_id);
    para->connection = dbus_node->connection;                
    para->next = temp_head;
    temp_head = para;
    
    *head_para = temp_head;
    
    return;
}


snmpd_dbus_message *list_connection_call_dbus_method(void *method, int flag, ...)
{
    if(NULL == method)
        return NULL;
        
    snmpd_dbus_message *messageHead = NULL;
    struct list_head *pos = NULL;
    list_for_each(pos, &snmpd_dbus_list_head) {
        netsnmp_dbus_connection *snmpd_dbus_node = dbus_node(pos);
            
        va_list var_args;
        va_start(var_args, flag);
        new_slot_instance_message(&messageHead, 
                                  snmpd_dbus_node, 
                                  method, 
                                  flag, 
                                  var_args);
        va_end(var_args);
    }
    
    return messageHead;
}

void free_dbus_message_list(snmpd_dbus_message **messageHead, void *method)
{
    if(NULL == method || NULL == messageHead)
        return ;
        
    snmpd_dbus_message *tempHead = *messageHead;   
    while(tempHead) {
        snmpd_dbus_message *tempNode = tempHead->next;

        if(tempHead->message)
            ((free_table_message *)method)(tempHead->message);
        free(tempHead);
        
        tempHead = tempNode;
    }
    
    *messageHead = NULL;
    return ;
}

void list_instance_parameter(instance_parameter **paraHead, int flag)
{
    if(NULL == paraHead)
        return SNMPD_DBUS_ERROR;
    else
        *paraHead = NULL;
        
    instance_parameter *tempHead = NULL;
    struct list_head *pos = NULL;
    list_for_each(pos, &snmpd_dbus_list_head) {
        netsnmp_dbus_connection *snmpd_dbus_node = dbus_node(pos);
        new_slot_instance_parameter(&tempHead, snmpd_dbus_node, flag);
    }
    *paraHead = tempHead;
}


void free_instance_parameter_list(instance_parameter **paraHead)
{
    if(NULL == paraHead || NULL == *paraHead)
        return ;

    instance_parameter *tempHead = *paraHead;
    while(tempHead){
        instance_parameter *temp = tempHead->next;
        free(tempHead);
        tempHead = temp;
    }
    *paraHead = NULL;
    return;
}    

void close_slot_dbus_connection(unsigned int slot_id)
{
	syslog(LOG_ERR,"####close_slot_dbus_connection slot %d####\n",slot_id);
    struct list_head *pos = NULL;
    list_for_each(pos, &snmpd_dbus_list_head) {
        netsnmp_dbus_connection *snmpd_dbus_node = dbus_node(pos);
        if(slot_id == snmpd_dbus_node->slot_id && snmpd_dbus_node->connection) {
            snmpd_close_dbus_connection(&(snmpd_dbus_node->connection));
            return ;
        }
    }
}

int get_slot_dbus_connection(unsigned int slot_id, void **paraHead, int flag)
{
    if(NULL == paraHead)
        return SNMPD_DBUS_ERROR;
    else
        *paraHead = NULL;

#if 0
    if(LOCAL_SLOT_NUM == slot_id) {
        slot_id = snmpd_dbus_connection_list.local_slot_num;
    }
#endif    
    syslog(LOG_DEBUG, "enter find the slot %d instance parameter or dbus connection\n", slot_id);
     
    instance_parameter *tempHead = NULL;
    struct list_head *pos = NULL;
    list_for_each(pos, &snmpd_dbus_list_head) {
        netsnmp_dbus_connection *snmpd_dbus_node = dbus_node(pos);
        
        if (slot_id == snmpd_dbus_node->slot_id || LOCAL_SLOT_NUM == slot_id) {
            if(SNMPD_INSTANCE_MASTER_V3 == flag){

                if(NULL == snmpd_dbus_node->connection) {
                    syslog(LOG_DEBUG, "Can`t find the slot %d connection\n", slot_id);
                    return SNMPD_DBUS_ERROR;
                }  
                else {
                    *paraHead = snmpd_dbus_node->connection;
                    syslog(LOG_DEBUG, "exit find the slot %d connection\n", slot_id);
                    return SNMPD_DBUS_SUCCESS;
                }
            }
            else {
                new_slot_instance_parameter(&tempHead, snmpd_dbus_node, flag);
                if(LOCAL_SLOT_NUM == slot_id && NULL == tempHead) {
                    syslog(LOG_DEBUG, "This slot %d can not find master instance!\n", snmpd_dbus_node->slot_id);       
                    continue;
                }
                
                break;    
            }    
        }
    } 
    
    if(NULL == tempHead) {
        syslog(LOG_DEBUG, "Can`t fine slot %d connection\n", slot_id);
        return SNMPD_DBUS_ERROR;
    }
    else {
        *paraHead = tempHead;
        syslog(LOG_DEBUG, "exit find the slot %d instance parameter\n", slot_id);
        return SNMPD_DBUS_SUCCESS;
    }
}


int get_instance_dbus_connection(dbus_parameter parameter, void **paraHead, int flag)
{
    if(NULL == paraHead)
        return SNMPD_DBUS_ERROR;
    else
        *paraHead = NULL;
    
    if(0 == parameter.instance_id || parameter.instance_id > INSTANCE_NUM) {
        syslog(LOG_DEBUG, "instance_dbus_parameter_init:The parameter.instance_id is error(%d)\n", parameter.instance_id);
        return SNMPD_DBUS_ERROR;
    }
    else if(parameter.local_id >= VRRP_TYPE_NUM) {    
        syslog(LOG_DEBUG, "instance_dbus_parameter_init:The parameter.local_id is error(%d)\n", parameter.local_id);
        return SNMPD_DBUS_ERROR;
    }
    
    syslog(LOG_DEBUG, "enter find local_id %d instance_id %d instance parameter or dbus connection\n", 
                       parameter.local_id, parameter.instance_id);

    instance_parameter *tempHead = NULL;
    struct list_head *pos = NULL;
    list_for_each(pos, &snmpd_dbus_list_head) {
    
        netsnmp_dbus_connection *snmpd_dbus_node = dbus_node(pos);

        if(SNMPD_REMOTE_INSTANCE == parameter.local_id && snmpd_dbus_node->slot_id != parameter.slot_id) {
            continue ;
        }
        
        if(SNMPD_INSTANCE_MASTER_V3 == flag) {
            void *connection = NULL;
            new_vrrp_instance_parameter(&connection,snmpd_dbus_node, parameter, flag);
            if(connection) {
                *paraHead = connection;
                
                syslog(LOG_DEBUG, "exit find local_id %d instance_id %d dbus connection\n", 
                                    parameter.local_id, parameter.instance_id);                    
                return SNMPD_DBUS_SUCCESS;
            }
        }
        else {            
            new_vrrp_instance_parameter(&tempHead, snmpd_dbus_node, parameter, flag);
        }
    }

    if(NULL == tempHead) {
        syslog(LOG_DEBUG, "Can`t find local_id %d instance_id %d instance parameter or dbus connection\n", 
                           parameter.local_id, parameter.instance_id);
        return SNMPD_DBUS_ERROR; 
    }
    else {
        syslog(LOG_DEBUG, "exit find local_id %d instance_id %d instance parameter\n", 
                            parameter.local_id, parameter.instance_id);
        *paraHead = tempHead;
        return SNMPD_DBUS_SUCCESS;
    }
}

unsigned long local_to_global_ID(dbus_parameter parameter, unsigned long id, unsigned long max_num)
{
    
    if(0 == parameter.instance_id || parameter.instance_id > INSTANCE_NUM || 
        parameter.local_id >= VRRP_TYPE_NUM || parameter.slot_id > SLOT_MAX_NUM) {
        return 0;
    }
    
    return ((!parameter.local_id * parameter.slot_id * INSTANCE_NUM + parameter.instance_id - 1) * max_num + id);
}

unsigned long global_to_local_ID(dbus_parameter *parameter, unsigned long id, unsigned long max_num)
{
    memset(parameter, 0, sizeof(dbus_parameter));
    
    unsigned int global_instanceid = id / max_num;
    
    parameter->local_id = (global_instanceid / INSTANCE_NUM) ? SNMPD_REMOTE_INSTANCE : SNMPD_LOCAL_INSTANCE;
    parameter->slot_id = global_instanceid / INSTANCE_NUM;
    parameter->instance_id = global_instanceid - !parameter->local_id * parameter->slot_id * INSTANCE_NUM + 1;
    
    return (id % max_num);
}

void get_slotID_localID_instanceID(char *plotid,dbus_parameter *parameter)/*返回0表示失败，返回1表示成功*/
{
	char *bake;
	char *temp = NULL;
	int i = 0;	
	if(NULL != plotid)
	{
		bake = (char *)malloc(strlen(plotid)+1);
		memset(bake,0,strlen(plotid)+1);
		strcpy(bake,plotid);
		
		temp = strtok(bake,"-");
		while(temp != NULL)
		{
			i++;
			if(i==1)
			{	
				parameter->slot_id= strtoul(temp,0,10);
			}
			else if(i ==2 )
			{	
				parameter->local_id = strtoul(temp,0,10);
			}
			else if(i==3)
			{	
				parameter->instance_id= strtoul(temp,0,10);
			}
			temp = strtok(NULL,"-");	  
		}
		if(bake)
		{
			free(bake);
		}
	}
}

int
get_local_slot_id(void){

    return snmpd_dbus_connection_list.local_slot_num;
}

