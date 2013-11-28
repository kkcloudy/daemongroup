#include <syslog.h>
#include <fcntl.h>
#include "hmd/hmdpub.h"
#include "board/board_define.h"
#include "ws_local_hansi.h"
#include "ws_dbus_list.h"
#include "ac_manage_def.h"
#include "ws_snmpd_engine.h"
#include "ws_snmpd_manual.h"
#include "ac_manage_interface.h"

#define TRY_TO_CON_NUM 3	/*³¢ÊÔdbus_get_tipc_connection´ÎÊý*/

int distributed_flag = 0;
dbus_connection_list snmpd_dbus_connection_list = { 0 };
unsigned int snmp_collection_mode = 0;

void init_distributed_flag(void)
{
	FILE *fp = NULL;    	
	if(VALID_DBM_FLAG == get_dbm_effective_flag())
	{
		fp = fopen(DISTRIBUTED_FILE, "r");
	}
	if(NULL == fp) {
        syslog(LOG_DEBUG, "Open DISTRIBUTED_FILE error\n");
		return ;
	}
    
	if(fscanf(fp, "%d", &distributed_flag) <= 0) {
		syslog(LOG_DEBUG, "Fscanf DISTRIBUTED_FILE error\n");
		fclose(fp);
		return ;
	}
	

	if(distributed_flag) {
	    distributed_flag = 1;
        syslog(LOG_DEBUG, "The system is distributed\n");
    }
    else {
        syslog(LOG_DEBUG, "The system is unDistributed\n");
    }

	fclose(fp);
	return ;
}

int get_product_info(char *filename)
{
	int fd;
	char buff[16] = {0};
	unsigned int data;

	if(NULL == filename) {
		return -1;
	}

    fd = open(filename, O_RDONLY, 0);
    if(fd >= 0) {
        if(read(fd, buff, 16) < 0) {
            syslog(LOG_WARNING, "get_product_info: Read error : no value\n");
            close(fd);
            return 0;
        }    
    }
    else {        
        syslog(LOG_WARNING, "get_product_info: Open file:%s error!\n", filename);
        return 0;
    }
    
    data = strtoul(buff, NULL, 10);

    close(fd);
    
	return data;
}
struct board_info *get_board_info(unsigned int slot_id)
{
    if(0 == slot_id || slot_id > SLOT_MAX_NUM) {
		syslog(LOG_DEBUG, "dbus_get_tipc_connection fun error: the slot_id is %d\n",slot_id);
		return NULL;
	}

    /*
        *   get product slot num
        */
    FILE *fp = NULL;
    char temp_buf[64] = { 0 };
	unsigned int board_on_mask = 0;

	if(VALID_DBM_FLAG == get_dbm_effective_flag())
	{
		fp = fopen(BOARD_MASK_FILE, "r");
	}
	if(NULL == fp){
		syslog(LOG_DEBUG, "Open BOARD_MASK_FILE error\n");
		return NULL;
	}
	
	if(fscanf(fp, "%u", &board_on_mask) <= 0 ) {
	    fclose(fp);
		syslog(LOG_DEBUG, "Fscanf BOARD_MASK_FILE error\n");
		return NULL;
	}

    syslog(LOG_DEBUG, "The broad_on_mask is %d\n", board_on_mask);
	fclose(fp);


    
    /*
        *   init borad info
        */    
    struct board_info *temp = NULL;
    temp = (struct board_info *)malloc(sizeof(struct board_info));
    if(NULL ==  temp) {
        syslog(LOG_DEBUG, "get_broad_info: malloc error\n");
        return NULL;
    }
    memset(temp, 0, sizeof(struct board_info));

    if((board_on_mask >> (slot_id - 1)) & 0x1) {

        int board_state = 0;        
        int board_active = 0;
        int board_type = 0;

        /*
              * get slot board state:BOARD_ABNORMAL/BOARD_ENABLE
              */
        fp = NULL;
		if(VALID_DBM_FLAG == get_dbm_effective_flag())
		{
			memset(temp_buf, 0, sizeof(temp_buf));
			sprintf(temp_buf, "/dbm/product/slot/slot%d/board_state", slot_id);
			fp = fopen(temp_buf, "r");
		}
        if(fp == NULL) {            
            temp->board_state = BOARD_ABNORMAL;
            
            syslog(LOG_DEBUG, "get_broad_info: The slot %d is work abnormal\n", slot_id);
            return temp;
        }
        
		fscanf(fp, "%d", &board_state);
	    fclose(fp);

		if (board_state <= 1) {
			temp->board_state = BOARD_ABNORMAL;
            syslog(LOG_DEBUG, "get_broad_info: The slot %d is work abnormal\n", slot_id);
			return temp;
		}
		else {
            temp->board_state = BOARD_ENABLE;
		}

        /*
              *     get slot borad_type.
              */
        fp = NULL;
		if(VALID_DBM_FLAG == get_dbm_effective_flag())
		{
			memset(temp_buf, 0, sizeof(temp_buf));
			sprintf(temp_buf, "/dbm/product/slot/slot%d/is_master", slot_id);
			fp = fopen(temp_buf, "r");
		}
        if(fp == NULL) {
            temp->board_type = 0;
            return temp;
        }
        
		fscanf(fp, "%d", &board_type);
	    fclose(fp);

        temp->board_type = (1 == board_type) ? 1 : 0;

                
        /*
              *     get slot borad active state.
              */
        fp = NULL;
		if(VALID_DBM_FLAG == get_dbm_effective_flag())
		{
			memset(temp_buf, 0, sizeof(temp_buf));
			sprintf(temp_buf, "/dbm/product/slot/slot%d/is_active_master", slot_id);
			fp = fopen(temp_buf, "r");
		}
        if(NULL == fp) {
            temp->board_active = 0;
            return temp;
        }
        
        fscanf(fp, "%d", &board_active);
        fclose(fp);
        
        temp->board_active = (1 == board_active) ? 1 : 0;
        
    }
    else {        
        syslog(LOG_DEBUG, "get_broad_info: The slot %d is empty\n", slot_id);
        temp->board_state = BOARD_NULL;
    }

    return temp;
    
}

DBusConnection *
dbus_get_tipc_connection(unsigned int slot_id)
{  
	if(0 == slot_id || slot_id > SLOT_MAX_NUM)
	{
		syslog(LOG_DEBUG, "dbus_get_tipc_connection fun error: the slot_id is %d\n",slot_id);
		return NULL;
	}
	
	DBusError dbus_error;
	DBusConnection *tempConnection = NULL;
	
	dbus_threads_init_default();

	dbus_error_init (&dbus_error);
	
	tempConnection = dbus_bus_get_remote(DBUS_BUS_SYSTEM, slot_id, &dbus_error);
	if(NULL == tempConnection) {	    
        if (dbus_error_is_set(&dbus_error)) {
            syslog(LOG_DEBUG, "The slot %d: dbus_get_tipc_connection: %s\n", dbus_error.message);
		    dbus_error_free(&dbus_error);
		}
		addresses_shutdown_func();
		syslog(LOG_DEBUG, "The slot %d remote server is not ready.\n", slot_id);
		return NULL;
	}
	
	syslog(LOG_DEBUG, "The slot %d tipc connection is OK\n", slot_id);
	return tempConnection;
		
}

void snmpd_close_dbus_connection(DBusConnection **connection)
{
	if(NULL == connection || NULL == *connection)
		return;

    if(*connection == ccgi_dbus_connection) {
	     dbus_connection_close(*connection);
         *connection = NULL;
         ccgi_dbus_connection = NULL; 

         /*
                * liutao edit at 2013-08-30
                * some interface not check (ccgi_dbus_connection != NULL), so need reinit dbus connection 
                */
        /* 
        snmpd_dbus_connection_init();
         *connection = ccgi_dbus_connection;
         */
    }
    else {
	    dbus_connection_close(*connection);	    
        *connection = NULL;
	}    
	
	return;
}

/*
*   add for 2.0 test
*/
static void 
manual_set_vrrp_state(netsnmp_dbus_connection *temp){

    if(temp->connection) {
        unsigned int manual_instance = 0;
        int ret = ac_manage_show_snmp_manual_instance(temp->connection, &manual_instance);
        syslog(LOG_INFO, "manual_instance = %d\n", manual_instance);
        if(AC_MANAGE_SUCCESS == ret && manual_instance) {
            int i = 0;
            for(i = 0; i < (INSTANCE_NUM * 2); i++) {
                if(manual_instance & (0x1 << i)) {
                    unsigned int local_id = i / INSTANCE_NUM;
                    unsigned int instance_id = i - (local_id * INSTANCE_NUM) + 1;
                    
                    temp->instance_state[local_id][instance_id] = SNMPD_VRRP_MASTER;
                    temp->master_instance[local_id][temp->master_instance_count[local_id]++] = instance_id;
                    
                    syslog(LOG_INFO, "Snmp manual set %s %d-%d Master\n", 
                                        local_id ? "Local-hansi" : "Remote-hansi", temp->slot_id, instance_id);
                }
            }
        }
    }    
    
	return ;    
}

static netsnmp_dbus_connection * new_netsnmp_dbus_connection(unsigned int slot_id, int flag)
{
    netsnmp_dbus_connection *temp = NULL;
    temp = (netsnmp_dbus_connection *)malloc(sizeof(netsnmp_dbus_connection));
    if(NULL == temp) {
        syslog(LOG_DEBUG, "error create slot %d new netsnmp dbus connection\n", slot_id);
        return NULL;
    }
    memset(temp, 0, sizeof(netsnmp_dbus_connection));

    temp->slot_id = slot_id;
    if(LOCAL_CONNECTION == flag)
    {
        if(NULL == ccgi_dbus_connection)
        {
            syslog(LOG_DEBUG, "The ccgi_dbus_connection is NULL, need init it\n");
            if(0 != snmpd_dbus_connection_init())
            {
                syslog(LOG_DEBUG, "The ccgi_dbus_connection is request failed, need connection tipc connection\n");
                temp->connection = dbus_get_tipc_connection(slot_id);
            }
            else
            {
               temp->connection = ccgi_dbus_connection;
               syslog(LOG_DEBUG, "The slot %d conncetion is ccgi_dbus_connection\n", slot_id);
            }
        }
        else
        {
            temp->connection = ccgi_dbus_connection;
            syslog(LOG_DEBUG, "The slot %d conncetion is ccgi_dbus_connection\n", slot_id);
        }
        snmpd_dbus_connection_list.local_dbus_node = temp;
    }
    else
        temp->connection = dbus_get_tipc_connection(slot_id);


    /*
        *   add for 2.0 test
        */
    manual_set_vrrp_state(temp);
    
    syslog(LOG_DEBUG, "create slot %d new netsnmp dbus connection\n", slot_id);
    return temp;
}

int snmpd_dbus_connection_list_init(void)
{
    if(NULL != snmpd_dbus_list_head.next) {
        syslog(LOG_DEBUG, "The dbus connection list is already init\n");
        return SNMPD_DBUS_SUCCESS;
    }
    syslog(LOG_DEBUG, "enter snmpd_dbus_connection_list_init\n");
    
    int ret = SNMPD_DBUS_SUCCESS;
    int slot_count = 0;

	if(VALID_DBM_FLAG == get_dbm_effective_flag())
	{
		slot_count = get_product_info(SLOT_COUNT_FILE);
	}
    
	memset(&snmpd_dbus_connection_list, 0, sizeof(snmpd_dbus_connection_list));
	if(VALID_DBM_FLAG == get_dbm_effective_flag())
	{
		snmpd_dbus_connection_list.local_slot_num = get_product_info(LOCAL_SLOTID_FILE);
	}
	if(SNMPD_DBUS_ERROR == snmpd_dbus_connection_list.local_slot_num) {
        snmpd_dbus_connection_list.local_slot_num = 1;
	}
	
	INIT_LIST_HEAD(&snmpd_dbus_list_head);

    if(distributed_flag && slot_count > 1 && !snmp_collection_mode) {
    
    	int i ;
    	for(i = 0 ; i < slot_count ; i++) {
    	
    	    unsigned int slot_id = i + 1;
    	    struct board_info *info = get_board_info(slot_id);
    	    if (NULL == info)
    	        continue;
    	    else if (BOARD_ENABLE != info->board_state) {
                free(info);
    	        continue;
    	    }    
    	    
    	    int flag = 0;
    	    if(slot_id == snmpd_dbus_connection_list.local_slot_num)
                flag = LOCAL_CONNECTION;
            else
                flag = REMOTE_CONNECTION;

            netsnmp_dbus_connection *temp_dbus_connection = new_netsnmp_dbus_connection(slot_id, flag);
    		if(NULL == temp_dbus_connection) {
                syslog(LOG_DEBUG, "Get new %d dbus connection is error\n", slot_id);
                ret = SNMPD_DBUS_ERROR;
				free(info);
				continue;
            }

            memcpy(&temp_dbus_connection->bInfo, info, sizeof(struct board_info));

			temp_dbus_connection->connet_times = 0;

            list_add(list_pos(temp_dbus_connection), &snmpd_dbus_list_head);            
            free(info);
    	}
        syslog(LOG_DEBUG, "Distributed(Snmp Concentre Collection):exit snmpd_dbus_connection_list_init\n");
	}
	else {

        netsnmp_dbus_connection *temp_dbus_connection = NULL;
        if(NULL == (temp_dbus_connection = new_netsnmp_dbus_connection(snmpd_dbus_connection_list.local_slot_num, LOCAL_CONNECTION)))
        {
            syslog(LOG_DEBUG, "Get local slot %d dbus connection is error\n", snmpd_dbus_connection_list.local_slot_num);
            return SNMPD_DBUS_ERROR;
        }
        list_add(list_pos(temp_dbus_connection), &snmpd_dbus_list_head);
    	if(snmp_collection_mode) {
            syslog(LOG_DEBUG, "Distributed(Snmp Decentral Collection):exit snmpd_dbus_connection_list_init\n");
    	}
    	else {
            syslog(LOG_DEBUG, "UNDistributed:exit snmpd_dbus_connection_list_init\n");
        }
    }
    
    return ret;
}

void uninit_snmpd_dbus_connection_list(void)
{
    if(NULL == snmpd_dbus_list_head.next) 
        return;
        
    struct list_head *pos = NULL;
    while(!list_empty(&snmpd_dbus_list_head))
    {   
        pos = snmpd_dbus_list_head.next;
        netsnmp_dbus_connection *snmpd_dbus_node = dbus_node(pos);
        list_del(pos); 

        if(snmpd_dbus_node->connection && snmpd_dbus_node->connection != ccgi_dbus_connection) {
            snmpd_close_dbus_connection(&(snmpd_dbus_node->connection));
        }
        free(snmpd_dbus_node);
    } 
    memset(&snmpd_dbus_connection_list, 0, sizeof(snmpd_dbus_connection_list));
    return;
}

void snmpd_vrrp_state_init(void)
{
    syslog(LOG_DEBUG, "enter snmpd_vrrp_state_init\n");
    
    int inst_ret = 0;
    struct Hmd_Board_Info_show *instance_head = NULL;
    syslog(LOG_DEBUG, "before show_broad_instance_info\n");
    inst_ret = show_broad_instance_info(ccgi_dbus_connection, &instance_head);
    syslog(LOG_DEBUG, "after show_broad_instance_info, inst_ret = %d\n", inst_ret);

    int vrrp_flag = 0;
    struct list_head *pos = NULL;
    list_for_each(pos, &snmpd_dbus_list_head)
    {
        int instance_state = 0;
        netsnmp_dbus_connection *snmpd_dbus_node = dbus_node(pos);
    /*   
        *
            memset(snmpd_dbus_node->master_instance_count, 0, sizeof(snmpd_dbus_node->master_instance_count));
            memset(snmpd_dbus_node->master_instance, 0, sizeof(snmpd_dbus_node->master_instance));
    */
    
        if(1 == inst_ret && instance_head) {
        
            struct Hmd_Board_Info_show *instance_node = NULL;
            for(instance_node = instance_head->hmd_board_list; NULL != instance_node; instance_node = instance_node->next)
            {
                if(snmpd_dbus_node->slot_id != instance_node->slot_no) {
                    continue;
                }

                syslog(LOG_DEBUG, "find hansi instance in slot %d\n", snmpd_dbus_node->slot_id);
                
                int i = 0;
                unsigned int remote_master_index = snmpd_dbus_node->master_instance_count[SNMPD_REMOTE_INSTANCE];
                for(i = 0; i < instance_node->InstNum; i++) {
                    if(0 == instance_node->Hmd_Inst[i]->Inst_ID) {
                        syslog(LOG_INFO, "The remote instance 0 is not exist!\n");
                        continue;
                    }
                    
                    syslog(LOG_DEBUG, "This borad remote hansi instance %d is %s\n", 
                                        instance_node->Hmd_Inst[i]->Inst_ID, 
                                        instance_node->Hmd_Inst[i]->isActive? "master" : "back");
                                        
                    if(SNMPD_NO_VRRP == snmpd_dbus_node->instance_state[SNMPD_REMOTE_INSTANCE][instance_node->Hmd_Inst[i]->Inst_ID]) {
                        if(1 == instance_node->Hmd_Inst[i]->isActive) {
                            snmpd_dbus_node->instance_state[SNMPD_REMOTE_INSTANCE][instance_node->Hmd_Inst[i]->Inst_ID] = SNMPD_VRRP_MASTER;
                            snmpd_dbus_node->master_instance[SNMPD_REMOTE_INSTANCE][remote_master_index++] = instance_node->Hmd_Inst[i]->Inst_ID;
                        }
                        else {
                            snmpd_dbus_node->instance_state[SNMPD_REMOTE_INSTANCE][instance_node->Hmd_Inst[i]->Inst_ID] = SNMPD_VRRP_BACK;
                        }
                    }        
                }

                unsigned int local_master_index = snmpd_dbus_node->master_instance_count[SNMPD_LOCAL_INSTANCE];
                for(i = 0; i < instance_node->LocalInstNum; i++) {      
                    if(0 == instance_node->Hmd_Local_Inst[i]->Inst_ID) {
                        syslog(LOG_INFO, "The local instance 0 is not exist!\n");
                        continue;
                    }
                    syslog(LOG_DEBUG, "This borad local hansi instance %d is %s\n", 
                                       instance_node->Hmd_Local_Inst[i]->Inst_ID, 
                                       instance_node->Hmd_Local_Inst[i]->isActive? "master" : "back");
                                       
                    if(SNMPD_NO_VRRP == snmpd_dbus_node->instance_state[SNMPD_LOCAL_INSTANCE][instance_node->Hmd_Local_Inst[i]->Inst_ID]) {                        
                        if(1 == instance_node->Hmd_Local_Inst[i]->isActive) {
                            snmpd_dbus_node->instance_state[SNMPD_LOCAL_INSTANCE][instance_node->Hmd_Local_Inst[i]->Inst_ID] = SNMPD_VRRP_MASTER;
                            snmpd_dbus_node->master_instance[SNMPD_LOCAL_INSTANCE][local_master_index++] = instance_node->Hmd_Local_Inst[i]->Inst_ID;
                        }
                        else {
                            snmpd_dbus_node->instance_state[SNMPD_LOCAL_INSTANCE][instance_node->Hmd_Local_Inst[i]->Inst_ID] = SNMPD_VRRP_BACK;
                        }
                    }        
                }
                
                snmpd_dbus_node->master_instance_count[SNMPD_REMOTE_INSTANCE] = remote_master_index;
                snmpd_dbus_node->master_instance_count[SNMPD_LOCAL_INSTANCE] = local_master_index;

                if(remote_master_index || local_master_index)
                    vrrp_flag = 1;
                    
                break;
            }
            
            syslog(LOG_DEBUG, "Init slot %d vrrp state success\n", snmpd_dbus_node->slot_id);
        }    
    } 
    
 #if 0   
    if(0 == vrrp_flag) {
        syslog(LOG_DEBUG, "There is no hansi instance in system\n");
        snmpd_local_dbus_node->instance_state[SNMPD_LOCAL_INSTANCE][0] = SNMPD_VRRP_MASTER;
        snmpd_local_dbus_node->master_instance[SNMPD_LOCAL_INSTANCE][0] = 0;
        snmpd_local_dbus_node->master_instance_count[SNMPD_LOCAL_INSTANCE] = 1;
    }
#endif

    free_broad_instance_info(&instance_head);
    syslog(LOG_DEBUG, "exit snmpd_vrrp_state_init\n");
    return;
}

static void refresh_snmpd_master_instance(netsnmp_dbus_connection *snmpd_dbus_node, unsigned int local_id)
{
    if(NULL == snmpd_dbus_node)
        return;
    
    snmpd_dbus_node->master_instance_count[local_id] = 0;        
    memset(snmpd_dbus_node->master_instance[local_id], 0, sizeof(snmpd_dbus_node->master_instance[local_id]));
    
    int i = 0, index = 0; 
    for(i ; i <= INSTANCE_NUM; i++)
    {
        if(SNMPD_VRRP_MASTER == snmpd_dbus_node->instance_state[local_id][i])
        {
            snmpd_dbus_node->master_instance[local_id][index++] = i;
        }
    }
    snmpd_dbus_node->master_instance_count[local_id] = index;
    
    return;
}

void
snmpd_had_hansi_advertise(int state) {
    
    struct list_head *pos = NULL;
    list_for_each(pos, &snmpd_dbus_list_head) {
        netsnmp_dbus_connection *snmpd_dbus_node = dbus_node(pos);
        
        syslog(LOG_INFO, "Enter slot %d handle had message\n", snmpd_dbus_node->slot_id);
        
        int i = 1;
        for(i = 1; i <= INSTANCE_NUM; i++ ) {
            if(SNMPD_NO_VRRP != snmpd_dbus_node->instance_state[SNMPD_REMOTE_INSTANCE][i]) {
                if(3 == state) {
                    snmpd_dbus_node->instance_state[SNMPD_REMOTE_INSTANCE][i] = SNMPD_INSTANCE_MASTER;
                }
                else {
                    snmpd_dbus_node->instance_state[SNMPD_REMOTE_INSTANCE][i] = SNMPD_INSTANCE_BACK;
                }
                
                syslog(LOG_INFO, "The slot %d remote hansi %d is %s\n", snmpd_dbus_node->slot_id, i, 3 == state ? "Master" : "Backup");
            }
        }
        refresh_snmpd_master_instance(snmpd_dbus_node, SNMPD_REMOTE_INSTANCE);
    }
    
    return ;
}

void snmpd_vrrp_state_advertise(unsigned int slot_id, unsigned int local_id, unsigned int instance_id, int state)
{
    if(slot_id > SLOT_MAX_NUM || 0 == slot_id) {
        syslog(LOG_DEBUG, "The slot_id %d is error\n", slot_id);
        return;
    }
    else if(instance_id > INSTANCE_NUM || 0 == instance_id) {
        syslog(LOG_DEBUG, "The instance_id %d is error\n", instance_id);
        return;
    }
    else if(local_id >= VRRP_TYPE_NUM) {
        syslog(LOG_DEBUG, "The local_id %d is error\n", local_id);
        return;
    }
    
    struct list_head *pos = NULL;
    list_for_each(pos, &snmpd_dbus_list_head) {
        netsnmp_dbus_connection *snmpd_dbus_node = dbus_node(pos);
        if(slot_id != snmpd_dbus_node->slot_id) {
            syslog(LOG_DEBUG, "The snmpd_dbus_node slot_id is not slot %d\n", slot_id);
            continue;
        }
        else {
            syslog(LOG_DEBUG, "The %d type instance %d vrrp type of slot %d is change and the state is %d\n", local_id, instance_id, slot_id, state);            
            if(3 == state){
                syslog(LOG_DEBUG, "The %d type instance %d vrrp type of slot %d is goto master\n", local_id, instance_id,slot_id);            
                snmpd_dbus_node->instance_state[local_id][instance_id] = SNMPD_INSTANCE_MASTER;
            }
            else {
                syslog(LOG_DEBUG, "The %d type instance %d vrrp type of slot %d is goto back\n", local_id, instance_id,slot_id);
                snmpd_dbus_node->instance_state[local_id][instance_id] = SNMPD_INSTANCE_BACK;
            }
            refresh_snmpd_master_instance(&snmpd_dbus_node, local_id);
            break;
        }  
    }
    return ;
}

void tipc_dbus_connection_maintenance(void)
{
	if(list_empty(&snmpd_dbus_list_head))
	{
		syslog(LOG_DEBUG, "The snmpd_dbus_connection_list is empty!\n");
	}
	else
	{
	    #if 1
	    struct list_head *pos = NULL;
	    list_for_each(pos, &snmpd_dbus_list_head)
        {
            netsnmp_dbus_connection *snmpd_dbus_node = dbus_node(pos);
            if((NULL == snmpd_dbus_node->connection)&&(snmpd_dbus_node->connet_times < TRY_TO_CON_NUM))
            {
            	/*
				syslog(LOG_INFO, "The snmpd_dbus_node->slot_id is %d!\n",snmpd_dbus_node->slot_id);
				*/
            	if(snmpd_dbus_connection_list.local_slot_num == snmpd_dbus_node->slot_id){
					 if(snmpd_dbus_connection_init()==0)
							snmpd_dbus_node->connection = ccgi_dbus_connection;
					 snmpd_dbus_node->connet_times += 1;
            	}else{
                	snmpd_dbus_node->connection = dbus_get_tipc_connection(snmpd_dbus_node->slot_id);
					snmpd_dbus_node->connet_times += 1;				
		syslog(LOG_ERR,"####dbus_get_tipc_connection slot=%d connet_times=%d####\n",snmpd_dbus_node->slot_id,snmpd_dbus_node->connet_times);
            	}
            }
        }
        #endif
	}
}


