#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <dbus/dbus.h>
#include "board/board_define.h"
#include "ac_manage_def.h"
#include "ws_snmpd_manual.h"
#include "ws_intf.h"
#include "ac_manage_mib_manual.h"
#include "ws_dbus_def.h"

#include "ws_dbus_list_interface.h"
#include "ac_manage_public.h"

struct mib_manual_stats_s mib_manual_stats = { 0 }; 

int
mib_manual_set_acif_stats(struct mib_acif_stats *acif_node) {
    if(NULL == acif_node) {
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }

    int i = 0;
    struct mib_acif_stats *temp_node = NULL;
    for(i = 0, temp_node = mib_manual_stats.acifstats_head; 
        i < mib_manual_stats.acifstats_num && NULL != temp_node; 
        i++, temp_node = temp_node->next) {
        syslog(LOG_DEBUG, "temp_node->ifname = %s, acif_node->ifname = %s\n", temp_node->ifname, acif_node->ifname);
        if(strcmp(temp_node->ifname, acif_node->ifname)) {
            continue;
        }
        else {
            temp_node->acIfInNUcastPkts = acif_node->acIfInNUcastPkts;
            temp_node->acIfInDiscardPkts = acif_node->acIfInDiscardPkts;
            temp_node->acIfInErrors = acif_node->acIfInErrors;
            temp_node->acIfInMulticastPkts = acif_node->acIfInMulticastPkts;

            temp_node->acIfOutDiscardPkts = acif_node->acIfOutDiscardPkts;
            temp_node->acIfOutErrors = acif_node->acIfOutErrors;
            temp_node->acIfOutNUcastPkts = acif_node->acIfOutNUcastPkts;
            temp_node->acIfOutMulticastPkts = acif_node->acIfOutMulticastPkts;

            return AC_MANAGE_SUCCESS;
        }
    }

    struct mib_acif_stats *new_node = (struct mib_acif_stats *)malloc(sizeof(struct mib_acif_stats));
    if(NULL == new_node) {
        syslog(LOG_WARNING, "mib_manual_set_acif_stats: malloc error!\n");
        return AC_MANAGE_MALLOC_ERROR;
    }

    memset(new_node, 0, sizeof(struct mib_acif_stats));
    memcpy(new_node, acif_node, sizeof(struct mib_acif_stats));

    new_node->next = mib_manual_stats.acifstats_head;
    mib_manual_stats.acifstats_head = new_node;
    mib_manual_stats.acifstats_num++;

    return AC_MANAGE_SUCCESS;
}


int
mib_show_manual_acif_stats(struct mib_acif_stats **acif_array, unsigned int *acif_num) {
    if(NULL == acif_array || NULL == acif_num) {

    }

    *acif_array = NULL;
    *acif_num = 0;
    
    if(0 == mib_manual_stats.acifstats_num) {
        return AC_MANAGE_CONFIG_NONEXIST;
    }
    
    struct mib_acif_stats *temp_array = (struct mib_acif_stats *)calloc(mib_manual_stats.acifstats_num, sizeof(struct mib_acif_stats));
    if(NULL == temp_array) {
        syslog(LOG_WARNING, "mib_show_manual_acif_stats: malloc error!\n");
        return AC_MANAGE_MALLOC_ERROR;
    }
    
    int i = 0;
    struct mib_acif_stats *temp_node = NULL;
    for(i = 0, temp_node = mib_manual_stats.acifstats_head; 
        i < mib_manual_stats.acifstats_num && NULL != temp_node; 
        i++, temp_node = temp_node->next) {

        memcpy(&temp_array[i], temp_node, sizeof(struct mib_acif_stats));
    }
  

    *acif_array = temp_array;
    *acif_num = mib_manual_stats.acifstats_num;
    return AC_MANAGE_SUCCESS;
}

int
mib_get_localslot_acif_stats(struct if_stats_list **ifHead, unsigned int *if_num) {

    if(NULL == ifHead || NULL == if_num) {
        return AC_MANAGE_INPUT_TYPE_ERROR;
    }
    *ifHead = NULL;
    *if_num = 0;

	struct if_stats_list *tempHead = NULL;
	unsigned int temp_num = 0;
    
	FILE *fp = NULL;

	struct if_stats temp_stats = { 0 };
	char buf[1024] = {0};
	char buftemp[50] = {0};
	char ifname[30] = {0};
	
	fp = fopen ("/proc/net/dev", "r");
	if(NULL == fp) {
		return AC_MANAGE_FILE_OPEN_FAIL;
	}

	fgets (buf, 1024, fp);
	fgets (buf, 1024, fp);
	
	while(fgets(buf, 1024, fp)) {
	
		memset(ifname, 0 , sizeof(ifname));
		memset(buftemp, 0, sizeof(buftemp));
		
		sscanf(buf,
    		   "%[^:]:%llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
    		   buftemp,
    		   &temp_stats.rx_bytes,
    		   &temp_stats.rx_packets,
    		   &temp_stats.rx_errors,
    		   &temp_stats.rx_dropped,
    		   &temp_stats.rx_fifo_errors,
    		   &temp_stats.rx_frame_errors,
    		   &temp_stats.rx_compressed,
    		   &temp_stats.rx_multicast,
    		
    		   &temp_stats.tx_bytes,
    		   &temp_stats.tx_packets,
    		   &temp_stats.tx_errors,
    		   &temp_stats.tx_dropped,
    		   &temp_stats.tx_fifo_errors,
    		   &temp_stats.collisions,
    		   &temp_stats.tx_carrier_errors,
    		   &temp_stats.tx_compressed,
    		   &temp_stats.tx_multicast);
		   
		sscanf(buftemp, "%s", ifname);

		if(0 == strncmp(ifname, "r", 1) || 0 == strncmp(ifname, "pimreg", 6) 
		    || 0 == strncmp(ifname, "sit0", 4)) {
		    
			continue;
		} 

        struct if_stats_list *tempNode = (struct if_stats_list *)malloc(sizeof(struct if_stats_list));
        if(NULL == tempNode) {
            syslog(LOG_WARNING, "mib_get_localslot_acif_stats: malloc %s tempNode error\n", ifname);
            continue;
        }

        memset(tempNode, 0, sizeof(*tempNode));
        memcpy(&(tempNode->stats), &temp_stats, sizeof(tempNode->stats));
        strncpy(tempNode->ifname, ifname, sizeof(tempNode->ifname) - 1);

        tempNode->next = tempHead;
        tempHead = tempNode;
        temp_num++;
    }
    
	fclose(fp);
    
    *ifHead = tempHead;
    *if_num = temp_num;

    return AC_MANAGE_SUCCESS;
}

void
free_mib_acif_stats_list(struct if_stats_list **ifHead) {

    if(NULL == ifHead)
        return ;

    struct if_stats_list *tempHead = *ifHead;
    while(tempHead) {
        
        struct if_stats_list *tempNode = tempHead->next;
        MANAGE_FREE(tempHead);
        tempHead = tempNode;
    }
    *ifHead = NULL;
    
    return ;
}

    
static int 
show_slot_mib_acif_stats(DBusConnection *connection, struct if_stats_list **ifHead, unsigned int *if_num) {
                                            
    if(NULL == connection || NULL == ifHead || NULL == if_num)
        return AC_MANAGE_INPUT_TYPE_ERROR;
        
    DBusMessage *query, *reply;
    DBusError err;
    DBusMessageIter  iter;
    DBusMessageIter  iter_array;
    DBusMessageIter  iter_struct;

    *ifHead = NULL;
    *if_num = 0;
    
    int ret = AC_MANAGE_SUCCESS;
    unsigned int temp_num = 0;    
    
    query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME,
			                                         AC_MANAGE_DBUS_OBJPATH,
			                                         AC_MANAGE_DBUS_INTERFACE,
			                                         AC_MANAGE_DBUS_GET_MIB_LOCALSLOT_ACIF_STATS);

    dbus_error_init(&err);

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
    
    dbus_message_iter_next(&iter);  
    dbus_message_iter_get_basic(&iter, &temp_num);
    
    dbus_message_iter_next(&iter);  
    dbus_message_iter_recurse(&iter,&iter_array);   
    
    if(AC_MANAGE_SUCCESS == ret && temp_num){

        int i = 0;
        for(i = 0; i < temp_num; i++) {

            struct if_stats_list if_stats_node = { 0 };
            
            char *ifname = NULL;
            
            dbus_message_iter_recurse(&iter_array, &iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &ifname);

            
            
            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(if_stats_node.stats.rx_packets));

            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(if_stats_node.stats.tx_packets));
            
            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(if_stats_node.stats.rx_bytes));

            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(if_stats_node.stats.tx_bytes));
            
            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(if_stats_node.stats.rx_errors));

            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(if_stats_node.stats.tx_errors));
            
            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(if_stats_node.stats.rx_dropped));

            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(if_stats_node.stats.tx_dropped));

            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(if_stats_node.stats.rx_multicast));

            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(if_stats_node.stats.tx_multicast));
            
            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(if_stats_node.stats.rx_compressed));

            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(if_stats_node.stats.tx_compressed));
            
            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(if_stats_node.stats.collisions));



            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(if_stats_node.stats.rx_length_errors));
            
            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(if_stats_node.stats.rx_over_errors));

            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(if_stats_node.stats.rx_crc_errors));

            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(if_stats_node.stats.rx_frame_errors));

            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(if_stats_node.stats.rx_fifo_errors));
            
            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(if_stats_node.stats.rx_missed_errors));



            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(if_stats_node.stats.tx_aborted_errors));
            
            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(if_stats_node.stats.tx_carrier_errors));

            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(if_stats_node.stats.tx_fifo_errors));
            
            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(if_stats_node.stats.tx_heartbeat_errors));

            dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct, &(if_stats_node.stats.tx_window_errors));


            struct if_stats_list *tempNode = (struct if_stats_list *)malloc(sizeof(struct if_stats_list));
            if(tempNode) {
                memset(tempNode, 0, sizeof(*tempNode));
                memcpy(&(tempNode->stats), &(if_stats_node.stats), sizeof(tempNode->stats));
                strncpy(tempNode->ifname, ifname, sizeof(tempNode->ifname) - 1);
                
                tempNode->next = *ifHead; 
                *ifHead = tempNode;
                (*if_num)++;
            }
            dbus_message_iter_next(&iter_array);
        }
    }
    
    dbus_message_unref(reply);
    
    return ret;    
}

static struct if_stats_list *
search_interface_stats(struct if_stats_list *ifHead, const char *ifname) {
    if(NULL == ifHead || NULL == ifname)
        return NULL;
    
    while(ifHead){
    
        if(0 == strcmp(ifHead->ifname, ifname)) {
            return ifHead;
        }

        ifHead = ifHead->next;
    }

    return NULL;
}

static void
accumulate_interface_stats_node(struct if_stats_list *destNode, struct if_stats_list *sourNode) {
    if(NULL == destNode || NULL == sourNode) {
        return ;
    }

    destNode->stats.rx_packets           += sourNode->stats.rx_packets;
    destNode->stats.tx_packets           += sourNode->stats.tx_packets;
    destNode->stats.rx_bytes             += sourNode->stats.rx_bytes;
    destNode->stats.tx_bytes             += sourNode->stats.tx_bytes;
    destNode->stats.rx_errors            += sourNode->stats.rx_errors;
    destNode->stats.tx_errors            += sourNode->stats.tx_errors;
    destNode->stats.rx_dropped           += sourNode->stats.rx_dropped;
    destNode->stats.tx_dropped           += sourNode->stats.tx_dropped;
    destNode->stats.rx_multicast         += sourNode->stats.rx_multicast;
    destNode->stats.tx_multicast         += sourNode->stats.tx_multicast;
    destNode->stats.rx_compressed        += sourNode->stats.rx_compressed;
    destNode->stats.tx_compressed        += sourNode->stats.tx_compressed;
    destNode->stats.collisions           += sourNode->stats.collisions;

    
    /* detailed rx_errors: */
    destNode->stats.rx_length_errors     += sourNode->stats.rx_length_errors;
    destNode->stats.rx_over_errors       += sourNode->stats.rx_over_errors;
    destNode->stats.rx_crc_errors        += sourNode->stats.rx_crc_errors;
    destNode->stats.rx_frame_errors      += sourNode->stats.rx_frame_errors;
    destNode->stats.rx_fifo_errors       += sourNode->stats.rx_fifo_errors;
    destNode->stats.rx_missed_errors     += sourNode->stats.rx_missed_errors;

    
    /* detailed tx_errors */
    destNode->stats.tx_aborted_errors    += sourNode->stats.tx_aborted_errors;
    destNode->stats.tx_carrier_errors    += sourNode->stats.tx_carrier_errors;
    destNode->stats.tx_fifo_errors       += sourNode->stats.tx_fifo_errors;
    destNode->stats.tx_heartbeat_errors  += sourNode->stats.tx_heartbeat_errors;
    destNode->stats.tx_window_errors     += sourNode->stats.tx_window_errors;
    
    return ;
}


static void
accumulate_interface_stats_list(struct if_stats_list *destStats, struct if_stats_list *sourStats) {

    if(NULL == destStats || NULL == sourStats) {
        return ;
    }

    struct if_stats_list *tempSour = NULL;
    for(tempSour = sourStats; tempSour; tempSour = tempSour->next) {

        struct if_stats_list *tempDest = search_interface_stats(destStats, tempSour->ifname);
        if(NULL == tempDest) {
            continue;
        }

        accumulate_interface_stats_node(tempDest, tempSour);
    }

    return ;
}


int
mib_accumulate_acif_stats(struct if_stats_list **ifHead, unsigned int *if_num, unsigned int slot_id) {

    if(NULL == ifHead || NULL == if_num || 0 == slot_id || slot_id > SLOT_MAX_NUM)
        return AC_MANAGE_INPUT_TYPE_ERROR;


    *ifHead = NULL;
    *if_num = 0;
    
    if(0 == local_slotID) {
		if(VALID_DBM_FLAG == get_dbm_effective_flag())
		{
			local_slotID = manage_get_product_info(PRODUCT_LOCAL_SLOTID);
		}
		if(0 == local_slotID) {
			syslog(LOG_WARNING, "mib_accumulate_acif_stats: get loal slot id error\n");
			return AC_MANAGE_FILE_OPEN_FAIL;
		}
    }

    if(local_slotID > SLOT_MAX_NUM) {
        local_slotID = 0;
        syslog(LOG_WARNING, "mib_accumulate_acif_stats: the local id %d is error\n", local_slotID);
        return AC_MANAGE_FILE_OPEN_FAIL;
    }

    unsigned int temp_if_num = 0;
    struct if_stats_list *tempHead = NULL;
    int ret = AC_MANAGE_SUCCESS;
    if(local_slotID == slot_id) {
        ret = mib_get_localslot_acif_stats(&tempHead, &temp_if_num);
    }
    else {
        if(NULL == tipc_connection[slot_id]) {
            tipc_connection[slot_id] = manage_dbus_bus_get_tipc_connection(slot_id);
            if(NULL == tipc_connection[slot_id]) {
                syslog(LOG_WARNING, "mib_accumulate_acif_stats: get slot %d tipc connection error\n", slot_id);
                return AC_MANAGE_INIT_DBUS_ERROR;
            }
        }
        ret = show_slot_mib_acif_stats(tipc_connection[slot_id], &tempHead, &temp_if_num);
    }
    syslog(LOG_DEBUG, "mib_accumulate_acif_stats: mib_get_localslot_acif_stats, slot_id = %d, ret = %d\n", slot_id, ret);
    if(AC_MANAGE_SUCCESS == ret && tempHead && temp_if_num) {
                
        int i = 1;
        for(i = 1; i <= SLOT_MAX_NUM; i++) {

            if(i == slot_id) {
                syslog(LOG_DEBUG, "mib_accumulate_acif_stats: this slot %d ac interface stats is alreay get\n", i);
                continue;
            }
            
            int temp_ret = AC_MANAGE_SUCCESS;
            unsigned int temp_num = 0;
            struct if_stats_list *slotHead = NULL;

            if(i == local_slotID){
                temp_ret = mib_get_localslot_acif_stats(&slotHead, &temp_num);
                syslog(LOG_DEBUG, "mib_accumulate_acif_stats: mib_get_localslot_acif_stats, slot = %d, temp_ret = %d\n", i, ret);
            }
            else if(tipc_connection[i]) {
                temp_ret = show_slot_mib_acif_stats(tipc_connection[i], &slotHead, &temp_num);
                syslog(LOG_DEBUG, "mib_accumulate_acif_stats: show_slot_mib_acif_stats, slot = %d, temp_ret = %d\n", i, ret);
            }
            else {
                continue;
            }
            
            if(AC_MANAGE_SUCCESS == temp_ret && slotHead && temp_num) {
                accumulate_interface_stats_list(tempHead, slotHead);
            }
            free_mib_acif_stats_list(&slotHead);
        }
        
        *ifHead = tempHead;
        *if_num = temp_if_num;
    }
    else {
        free_mib_acif_stats_list(&tempHead);
    }
    
    return ret;
}

