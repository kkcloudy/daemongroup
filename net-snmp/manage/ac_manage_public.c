#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <dbus/dbus.h>
#include <sys/ioctl.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <net/if.h>
#include "board/board_define.h"
#include "hmd/hmdpub.h"
#include "ws_dbus_def.h"
#include "ws_dbus_list_interface.h"
#include "ac_manage_def.h"
#include "ac_manage_public.h"
#include "ac_manage_dbus.h"

#define PFM_DBUS_BUSNAME				"pfm.daemon"
#define PFM_DBUS_OBJPATH				"/pfm/daemon"
#define PFM_DBUS_INTERFACE				"pfm.daemon"
#define PFM_DBUS_METHOD_PFM_TABLE 	    "pfm_maintain_table"

DBusConnection *tipc_connection[SLOT_MAX_NUM + 1] = { 0 };

unsigned int local_slotID = 0; 
unsigned int active_master_slotID = 0;


DBusConnection *
manage_dbus_bus_get_tipc_connection(unsigned int slot_id) {  

	if(0 == slot_id || slot_id > SLOT_MAX_NUM) {
		syslog(LOG_WARNING, "manage_dbus_bus_get_tipc_connection fun error: the slot_id is %d\n",slot_id);
		return NULL;
	}
	
	DBusError dbus_error;
	DBusConnection *tempConnection = NULL;
	
	dbus_error_init (&dbus_error);
	
	tempConnection = dbus_bus_get_remote(DBUS_BUS_SYSTEM, slot_id, &dbus_error);
	if(NULL == tempConnection) {
		dbus_error_free(&dbus_error);
		addresses_shutdown_func();
		syslog(LOG_WARNING, "manage_dbus_bus_get_tipc_connection: The slot %d remote server is not ready.\n", slot_id);
		return NULL;
	}
	
	syslog(LOG_DEBUG, "The slot %d dbus through tipc connection is OK\n", slot_id);
	return tempConnection;
}

void 
manage_dbus_tipc_connection_close(DBusConnection **connection) {

	if(NULL == connection || NULL == *connection)
		return;
		
	dbus_connection_close(*connection);
	*connection = NULL;
	return;
}

void 
init_manage_tipc_dbus(void) {
    
    int i = 1;
    for(i = 1; i <= SLOT_MAX_NUM; i++) {
        if(tipc_connection[i]) {
            manage_dbus_tipc_connection_close(&tipc_connection[i]);
        }

        tipc_connection[i] = manage_dbus_bus_get_tipc_connection(i);
    }

    return ;
}

void 
free_manage_tipc_dbus(void){
    int i = 1;
    for(i = 1; i <= SLOT_MAX_NUM; i++) {
        if(tipc_connection[i]) {
            manage_dbus_tipc_connection_close(&tipc_connection[i]);
        }
    }
	return;
}

void
manage_free_master_instance_para(instance_parameter **paraHead) {
    if(NULL == paraHead)
        return ;

    while(*paraHead) {
        instance_parameter *paraNode = (*paraHead)->next;
        MANAGE_FREE(*paraHead);
        *paraHead = paraNode;
    }
    return ;
}

int
manage_get_master_instance_para(instance_parameter **paraHead) {
    syslog(LOG_DEBUG, "enter manage_get_master_instance_para\n");
    
    if(NULL == paraHead) {
        return AC_MANAGE_CONFIG_FAIL;
    }
    *paraHead = NULL;

    if(0 == local_slotID) {
		if(VALID_DBM_FLAG == get_dbm_effective_flag())
		{
			local_slotID = manage_get_product_info(PRODUCT_LOCAL_SLOTID);
		}
        if(0 == local_slotID) {
            syslog(LOG_WARNING, "manage_get_master_instance_para: get local slot id error\n");
            return AC_MANAGE_FILE_OPEN_FAIL;
        }
    }

    int ret = 0;
    struct Hmd_Board_Info_show *instance_head = NULL;
    syslog(LOG_DEBUG, "manage_get_master_instance_para: before show_broad_instance_info\n");
    ret = show_broad_instance_info(ac_manage_dbus_connection, &instance_head);
    syslog(LOG_DEBUG, "manage_get_master_instance_para: after show_broad_instance_info, ret = %d\n", ret);
    if(1 == ret && instance_head) {

        struct Hmd_Board_Info_show *instance_node = NULL;
        for(instance_node = instance_head->hmd_board_list; NULL != instance_node; instance_node = instance_node->next) {
            
            if(NULL == tipc_connection[instance_node->slot_no]) {
                tipc_connection[instance_node->slot_no] = manage_dbus_bus_get_tipc_connection(instance_node->slot_no);
                if(NULL == tipc_connection[instance_node->slot_no]) {
                    syslog(LOG_WARNING, "manage_get_master_instance_para: init slot %d connection error\n", instance_node->slot_no);
                    continue;        
                }
            }

            unsigned int instance_state = 0;
            int manual_ret = AC_MANAGE_SUCCESS;
            if(local_slotID == instance_node->slot_no) {
                manual_ret = snmp_show_manual_set_instance_master(&instance_state); 
            }
            else {
                manual_ret = ac_manage_show_snmp_manual_instance(tipc_connection[instance_node->slot_no], &instance_state);
            }
            syslog(LOG_DEBUG,"manage_get_master_instance_para: after show slot %d manual set instance state: manual_ret = %d, instance_state = %d\n",
                                instance_node->slot_no, manual_ret, instance_state);
            
            int i = 0;
            for(i = 0; i < instance_node->InstNum; i++) {
                if(0 >= instance_node->Hmd_Inst[i]->Inst_ID){
                    continue;
                }
                
                if((1 == instance_node->Hmd_Inst[i]->isActive) || 
                    (AC_MANAGE_SUCCESS == manual_ret && (instance_state & (0x1 << (instance_node->Hmd_Inst[i]->Inst_ID - 1))))){
                    instance_parameter *paraNode = (instance_parameter *)malloc(sizeof(instance_parameter));
                    if(NULL == paraNode) {
                        syslog(LOG_WARNING, "manage_get_master_instance_para: malloc remote instance %d-%d parameter fail\n",
                                             instance_node->slot_no, instance_node->Hmd_Inst[i]->Inst_ID);
                        continue;
                    }

                    paraNode->parameter.instance_id = instance_node->Hmd_Inst[i]->Inst_ID;
                    paraNode->parameter.local_id = 0;
                    paraNode->parameter.slot_id = instance_node->slot_no;
                    paraNode->connection = tipc_connection[instance_node->slot_no];
                    syslog(LOG_DEBUG, "manage_get_master_instance_para: remote hansi %d-%d is master\n", 
                                        instance_node->slot_no, instance_node->Hmd_Inst[i]->Inst_ID);
                    
                    paraNode->next = *paraHead;
                    *paraHead = paraNode;

                }
            }

            for(i = 0; i < instance_node->LocalInstNum; i++) { 
                if(0 >= instance_node->Hmd_Local_Inst[i]->Inst_ID){
                    continue;
                }
                
                if((1 == instance_node->Hmd_Local_Inst[i]->isActive) || 
                    (AC_MANAGE_SUCCESS == manual_ret && (instance_state & (0x1 << (instance_node->Hmd_Local_Inst[i]->Inst_ID + INSTANCE_NUM - 1))))) {
                    
                    if(NULL == tipc_connection[instance_node->slot_no]) {
                        tipc_connection[instance_node->slot_no] = manage_dbus_bus_get_tipc_connection(instance_node->slot_no);
                        if(NULL == tipc_connection[instance_node->slot_no]) {
                            syslog(LOG_WARNING, "manage_get_master_instance_para: init slot %d connection error\n", instance_node->slot_no);
                            break;        
                        }
                    }

                    instance_parameter *paraNode = (instance_parameter *)malloc(sizeof(instance_parameter));
                    if(NULL == paraNode) {
                        syslog(LOG_WARNING, "manage_get_master_instance_para: malloc local instance %d-%d parameter fail\n",
                                             instance_node->slot_no, instance_node->Hmd_Local_Inst[i]->Inst_ID);
                        continue;
                    }

                    paraNode->parameter.instance_id = instance_node->Hmd_Local_Inst[i]->Inst_ID;
                    paraNode->parameter.local_id = 1;
                    paraNode->parameter.slot_id = instance_node->slot_no;
                    paraNode->connection = tipc_connection[instance_node->slot_no];
                    syslog(LOG_DEBUG, "manage_get_master_instance_para: local hansi %d-%d is master\n", 
                                        instance_node->slot_no, instance_node->Hmd_Local_Inst[i]->Inst_ID);
                                        
                    paraNode->next = *paraHead;
                    *paraHead = paraNode;
                }
            }
        }
    }

	free_broad_instance_info(&instance_head);
	
	syslog(LOG_DEBUG, "exit manage_get_master_instance_para\n");
	return AC_MANAGE_SUCCESS;
}


int 
manage_get_product_info(const char *filename) {

    int fd;
    char buff[16] = {0};
    unsigned int data;

    if(NULL == filename) {
        return 0;
    }

    fd = open(filename, O_RDONLY, 0);
    if(fd >= 0) {
        if(read(fd, buff, 16) < 0) {
            syslog(LOG_WARNING, "manage_get_product_info: Read error : no value\n");
            close(fd);
            return 0;
        }    
    }
    else {        
        syslog(LOG_WARNING, "manage_get_product_info: Open file:%s error!\n", filename);
        return 0;
    }
    
    data = strtoul(buff, NULL, 10);

    close(fd);

    return data;
}

int
manage_ifname_is_legal_input(const char *ifname){
	struct ifreq ifr;
	int sk;

	if(!strcmp(ifname, "any")) {
		return AC_MANAGE_SUCCESS;
	}
	
	sk = socket(AF_INET, SOCK_DGRAM, 0);
	if (sk <= 0) {
		goto error;
	}

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name) - 1); 		

	if (ioctl(sk, SIOCGIFINDEX, &ifr)) {
		goto error1;
	}

	close(sk);
	return AC_MANAGE_SUCCESS;

error1:
	close(sk);
error:
	return AC_MANAGE_INPUT_TYPE_ERROR;
}

int 
manage_get_slot_id_by_ifname(const char *ifname) {

	int slotnum = -1;
	int i = 0;
	int count = 0;
	char tmp[32];

	memset(tmp, 0, sizeof(tmp));
	memcpy(tmp, ifname, strlen(ifname));

	/* eth : cpu */
	if (0 == strncmp(ifname, "eth", 3)) {
		sscanf(ifname, "eth%d-%*d", &slotnum);
	}

	/* ve */
	else if (0 == strncmp(ifname, "ve", 2)) {
		//sscanf(ifname, "ve%d.%*d", &slotnum);
		sscanf(ifname, "ve%2d", &slotnum);
	} 

	/* radio */
	else if (0 == strncmp(ifname, "r", 1)) {
		for (i = 0; i < strlen(ifname); i++) {
			/*use '-' to make sure this radio is local board or remote board */
			if (tmp[i] == '-') {
				count++;
			}			
		}
		
		if (2 == count) {	/*local board*/
			if(VALID_DBM_FLAG == get_dbm_effective_flag())
			{
				slotnum = manage_get_product_info(PRODUCT_LOCAL_SLOTID);
			}
		} else if(3 == count) {	/*remote board*/
			sscanf(ifname, "r%d-%*d-%*d-%d.%*d", &slotnum);
		}
	}

	/* wlan */
	else if (0 == strncmp(ifname, "wlan", 4)) {
		for (i = 0; i < strlen(ifname); i++) {
			if(tmp[i] == '-') {
				count++;
			}
		}
		
		if (1 == count) {	/*local board*/
			if(VALID_DBM_FLAG == get_dbm_effective_flag())
			{
				slotnum = manage_get_product_info(PRODUCT_LOCAL_SLOTID);
			}
		} else if (2 == count) {	/*remote board*/
			sscanf(ifname, "wlan%d-%*d-%*d", &slotnum);
		}
	}

	/* ebr */
	else if (0 == strncmp(ifname, "ebr", 3)) {
		for (i = 0; i < strlen(ifname); i++) {
			if (tmp[i] == '-') {
				count++;
			}
		}
		if (1 == count) {	/*local board*/
			if(VALID_DBM_FLAG == get_dbm_effective_flag())
			{
				slotnum = manage_get_product_info(PRODUCT_LOCAL_SLOTID);
			}
		} else if (2 == count) {	/*remote board*/
			sscanf(ifname, "ebr%d-%*d-%*d", &slotnum);
		}
	}

	/* slot all */
	else if (0 == strncmp(ifname, "slot", 4)) {
        sscanf(ifname, "slot%d", &slotnum);
	}
	
	return slotnum;
}



static int 
manage_config_pfm_by_dbus(DBusConnection *connection, struct pfmOptParameter *pfmParameter) {
    syslog(LOG_DEBUG, "enter manage_config_pfm_by_dbus\n");
    
    if(NULL == connection || NULL == pfmParameter || NULL == pfmParameter->ifName) {
        syslog(LOG_WARNING, "manage_config_pfm_by_dbus: input para error!\n");
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }

    if(0 == strncmp(pfmParameter->ifName, "slot", 4)) {
        pfmParameter->ifName = "all";
    }
       
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int ret = AC_MANAGE_CONFIG_FAIL;
	
	query = dbus_message_new_method_call(
        								PFM_DBUS_BUSNAME,		
        								PFM_DBUS_OBJPATH,	
        								PFM_DBUS_INTERFACE,
        								PFM_DBUS_METHOD_PFM_TABLE);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_INT32,  &pfmParameter->pfm_opt,
							DBUS_TYPE_INT32,  &pfmParameter->pfm_opt_para,
							DBUS_TYPE_UINT16, &pfmParameter->pfm_protocol,
							DBUS_TYPE_STRING, &pfmParameter->ifName,
							DBUS_TYPE_UINT32, &pfmParameter->src_port,
							DBUS_TYPE_UINT32, &pfmParameter->dest_port,
							DBUS_TYPE_STRING, &pfmParameter->src_ipaddr,
							DBUS_TYPE_STRING, &pfmParameter->dest_ipaddr,
							DBUS_TYPE_INT32,  &pfmParameter->slot_id,
							DBUS_TYPE_INVALID);


	

    	
	reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);
	if (NULL == reply){
		if (dbus_error_is_set(&err)){
			syslog(LOG_WARNING, "%s raised: %s\n", err.name, err.message);
			dbus_message_unref(query);
			dbus_error_free(&err);
		}
		
        return AC_MANAGE_DBUS_ERROR;
	}
	
	dbus_message_unref(query);
		
	if(dbus_message_get_args(reply, &err,
								DBUS_TYPE_UINT32,&ret,
								DBUS_TYPE_INVALID))  {
								
		syslog(LOG_DEBUG, "recv reply is :%s\n", ret == 0 ? "OK" : "ERROR");
	} 
	else {
		if (dbus_error_is_set(&err)) {
			syslog(LOG_WARNING, "%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}		
	}
	
	dbus_message_unref(reply);
	
    syslog(LOG_DEBUG, "exit manage_config_pfm_by_dbus, ret = %d\n", ret);

    if(0 == ret) {
        return AC_MANAGE_SUCCESS;
    }

	return AC_MANAGE_CONFIG_FAIL;
}

static int 
manage_proxy_pfm_config(DBusConnection *connection, struct pfmOptParameter *pfmParameter) {
    syslog(LOG_DEBUG, "enter manage_proxy_pfm_config\n");
    
    if(NULL == connection || NULL == pfmParameter || NULL == pfmParameter->ifName) {
        syslog(LOG_WARNING, "manage_proxy_pfm_config: input para error!\n");
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }
    
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;

	int ret = AC_MANAGE_DBUS_ERROR;

	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
										AC_MANAGE_DBUS_OBJPATH,
										AC_MANAGE_DBUS_INTERFACE,
										AC_MANAGE_DBUS_PROXY_PFM_CONFIG);

    dbus_error_init(&err);
    
    dbus_message_append_args(query,
                                DBUS_TYPE_INT32,  &pfmParameter->pfm_opt,    /*0:add , 1:delete*/
                                DBUS_TYPE_INT32,  &pfmParameter->pfm_opt_para,
                                DBUS_TYPE_UINT16, &pfmParameter->pfm_protocol,
                                DBUS_TYPE_STRING, &pfmParameter->ifName,
                                DBUS_TYPE_UINT32, &pfmParameter->src_port,
                                DBUS_TYPE_UINT32, &pfmParameter->dest_port,
                                DBUS_TYPE_STRING, &pfmParameter->src_ipaddr,
                                DBUS_TYPE_STRING, &pfmParameter->dest_ipaddr,
                                DBUS_TYPE_INT32,  &pfmParameter->slot_id,
                                DBUS_TYPE_INVALID);

    reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);

    dbus_message_unref(query);

    if(NULL == reply) {
        if(dbus_error_is_set(&err)) {
            dbus_error_free(&err);
        }
        return AC_MANAGE_DBUS_ERROR;
    }
    
	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);

    dbus_message_unref(reply);
    
    return ret;
}

int
manage_config_pfm_table_entry(struct pfmOptParameter *pfmParameter) {
    syslog(LOG_DEBUG, "enter manage_config_pfm_table_entry\n");
    
    if(NULL == pfmParameter || NULL == pfmParameter->ifName) {
        syslog(LOG_WARNING, "manage_config_pfm_table_entry: input para error!\n");
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }

    int send_to_slot = manage_get_slot_id_by_ifname(pfmParameter->ifName);
    if(send_to_slot < 1 || send_to_slot > SLOT_MAX_NUM) {
        syslog(LOG_WARNING, "manage_config_pfm_table_entry: get send_to_slot %d error\n", send_to_slot);
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }

    if(send_to_slot == pfmParameter->slot_id) {
        return AC_MANAGE_SUCCESS;
    }


    if(0 == active_master_slotID) {
		
		if(VALID_DBM_FLAG == get_dbm_effective_flag())
		{
			active_master_slotID = manage_get_product_info(PRODUCT_ACTIVE_MASTER);
		}
        if(0 == active_master_slotID) {
            syslog(LOG_WARNING, "manage_config_pfm_table_entry: get active master slot id error\n");
            return AC_MANAGE_FILE_OPEN_FAIL;
        }
    }

    if(0 == local_slotID) {
		if(VALID_DBM_FLAG == get_dbm_effective_flag())
		{
			local_slotID = manage_get_product_info(PRODUCT_LOCAL_SLOTID);
		}
        if(0 == local_slotID) {
            syslog(LOG_WARNING, "manage_config_pfm_table_entry: get local slot id error\n");
            return AC_MANAGE_FILE_OPEN_FAIL;
        }
    }


    int pfm_ret = AC_MANAGE_SUCCESS;
    
    if(active_master_slotID == local_slotID) {
        if(NULL == tipc_connection[send_to_slot]) {
            tipc_connection[send_to_slot] = manage_dbus_bus_get_tipc_connection(send_to_slot);
            if(NULL == tipc_connection[send_to_slot]) {
                syslog(LOG_WARNING, "manage_config_pfm_table_entry: init slot pfm send to slot %d tipc connection error\n", send_to_slot);
                return AC_MANAGE_INIT_DBUS_ERROR;
            }
        }
        
        pfm_ret = manage_config_pfm_by_dbus(tipc_connection[send_to_slot], pfmParameter);
        if(AC_MANAGE_DBUS_ERROR == pfm_ret) {
            manage_dbus_tipc_connection_close(&tipc_connection[send_to_slot]);     
        }        
    }
    else {
        if(NULL == tipc_connection[active_master_slotID]) {
            tipc_connection[active_master_slotID] = manage_dbus_bus_get_tipc_connection(active_master_slotID);
            if(NULL == tipc_connection[active_master_slotID]) {
                syslog(LOG_WARNING, "manage_config_pfm_table_entry: init active master %d tipc connection error\n", active_master_slotID);
                return AC_MANAGE_INIT_DBUS_ERROR;
            }
        }

        pfm_ret = manage_proxy_pfm_config(tipc_connection[active_master_slotID], pfmParameter);
        if(AC_MANAGE_DBUS_ERROR == pfm_ret) {
            manage_dbus_tipc_connection_close(&tipc_connection[active_master_slotID]);        
        }
    }

    return pfm_ret;
}

struct running_config *
manage_new_running_config(void) {
    struct running_config *temp_config = (struct running_config *)malloc(sizeof(struct running_config));
    if(NULL == temp_config) {
        return NULL;
    }
    memset(temp_config, 0, sizeof(struct running_config));
    return temp_config;
}


void 
manage_insert_running_config(struct running_config **head,
                                        struct running_config **end,
                                        struct running_config *temp) {
    if(NULL == *head) {
        *head = temp;
        *end = temp;
    }
    else {
        (*end)->next = temp;
        *end = temp;
    }
    return ;
}

void
manage_free_running_config(struct running_config **configHead) {
    if(NULL == configHead || NULL == *configHead)
        return ;

    struct running_config *tempHead = *configHead;
    while(tempHead) {
        struct running_config *tempNode = tempHead->next;
        MANAGE_FREE(tempHead);
        tempHead = tempNode;
    }
    *configHead = NULL;
    
    return ;
}

