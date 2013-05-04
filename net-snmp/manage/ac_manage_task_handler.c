
/** include glibc **/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <errno.h>

#include <time.h>
#include <sys/time.h>
#include <sys/types.h>

/** include manage lib **/
#include "board/netlink.h"
#include "sem/product.h"
#include "board/board_define.h"

#include "ws_dbus_def.h"
#include "ws_dbus_list_interface.h"
#include "ws_intf.h"

#include "ac_manage_def.h"
#include "ac_manage_public.h"
#include "manage_log.h"
#include "manage_type.h"







#define MAX_IF_NUM	250

int
manage_task_test(void *para, size_t para_len,void **data, u_long *data_len) {
	if(NULL == data || NULL == data_len) {
		return AC_MANAGE_INPUT_TYPE_ERROR;
	}

	manage_log(LOG_INFO, "call manage_task_test\n");

	return AC_MANAGE_SUCCESS;
}

int
manage_task_interface_info(void *para, size_t para_len,void **data, u_long *data_len) {
	if(NULL == data || NULL == data_len) {
		return AC_MANAGE_INPUT_TYPE_ERROR;
	}

	*data = NULL;
	*data_len = 0;

	struct if_stats_list *temp_data = NULL;
	u_long if_num = 0, i = 0;
	
	
	FILE *fp = NULL;

	struct if_stats temp_stats;
	char buf[1024], buftemp[50], ifname[30];
	
	fp = fopen ("/proc/net/dev", "r");
	if(NULL == fp) {
		return AC_MANAGE_FILE_OPEN_FAIL;
	}

	fgets(buf, 1024, fp);
	fgets(buf, 1024, fp);
	
	while(fgets(buf, 1024, fp)) {
		memset(buftemp, 0, sizeof(buftemp));
		memset(ifname, 0 , sizeof(ifname));
		
		sscanf(buf, "%[^:]", buftemp);
		sscanf(buftemp, "%s", ifname);

		if(0 == strncmp(ifname, "r", 1) || 0 == strncmp(ifname, "pimreg", 6) 
		    || 0 == strncmp(ifname, "sit0", 4)) {
			continue;
		}
		if_num++;	
	}	

	if(0 == if_num) {
		fclose(fp);
		return AC_MANAGE_SUCCESS;
	}

	fseek(fp, 0, SEEK_SET);

	if(if_num > MAX_IF_NUM)/*limit interface num,avoid exceeding the send size*/
	{
		if_num = MAX_IF_NUM;
	}
	
	temp_data = (struct if_stats_list *)calloc(if_num, sizeof(struct if_stats_list));
	if(NULL == temp_data) {
		fclose(fp);
		return AC_MANAGE_MALLOC_ERROR;
	}
	
	fgets(buf, 1024, fp);
	fgets(buf, 1024, fp);

	while(fgets(buf, 1024, fp)) {
		if(i >= if_num) {
			break;
		}
		
		memset(buftemp, 0, sizeof(buftemp));
		memset(ifname, 0 , sizeof(ifname));
		
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
		
		manage_log(LOG_DEBUG, "manage_task_interface_info: ifname = %s\n", ifname);
		if(0 == strncmp(ifname, "r", 1) 
			|| 0 == strncmp(ifname, "ppp", 3)
			|| 0 == strncmp(ifname, "pimreg", 6) 
			|| 0 == strncmp(ifname, "sit0", 4)) {
			continue;
		} 

		memcpy(&(temp_data[i].stats), &temp_stats, sizeof(struct if_stats));
		strncpy(temp_data[i].ifname, ifname, sizeof(temp_data[i].ifname) - 1);

		i++;
	}

	*data = temp_data;
	*data_len = i * sizeof(struct if_stats_list);

	fclose(fp);
	
	return AC_MANAGE_SUCCESS;
}

int manage_task_I_O_board_Time(int slotid)
{
	char buff[255] = {0};
	FILE *fp = NULL;
	char city[31]={ 0 };
	char area[31]={ 0 };


	//set time to new slot
	
	manage_log(LOG_DEBUG,"manage_task_I_O_board_Time: set time to slotid:%d\n",slotid);	
	fp=popen("date -u","r");  
	if( fp != NULL )
	{
		memset(buff,0,sizeof(buff));
		fgets( buff, sizeof(buff), fp );
		
        if(NULL == tipc_connection[slotid]) {
            tipc_connection[slotid] = manage_dbus_bus_get_tipc_connection(slotid);
            if(NULL == tipc_connection[slotid]) {
                manage_log(LOG_WARNING, "mib_accumulate_acif_stats: get slot %d tipc connection error\n", slotid);
                return AC_MANAGE_INIT_DBUS_ERROR;
            }
        }
		ac_manage_set_time(tipc_connection[slotid],buff);
		pclose(fp);
	}
	//set timezone to new slot
	manage_log(LOG_DEBUG,"manage_task_I_O_board_Time: set timezone to slotid:%d\n",slotid);	
	
	memset(buff,0,sizeof(buff));
	fp = popen( "sudo date +%Z", "r" );
	if(NULL != fp)
	{
		fgets(buff, sizeof(buff), fp );
		buff[strlen(buff)-1] = '\0';
		manage_log(LOG_DEBUG,"manage_task_I_O_board_Time:timezone %s\n",buff);	
		if( 0 == strcmp("CST",buff)){
			strcpy(area,"Asia");
			strcpy(city,"Shanghai");
			ac_manage_set_timezone(tipc_connection[slotid],area,city);

		}else if( 0 == strcmp("UTC",buff)){
			strcpy(area,"Etc");
			strcpy(city,"UTC");
			ac_manage_set_timezone(tipc_connection[slotid],area,city);			
		}else{
			manage_log(LOG_DEBUG,"manage_task_I_O_board_Time:unknow the timezone %s\n",buff);	
			return AC_MANAGE_INPUT_TYPE_ERROR;
		}
	}
	pclose(fp);
	return 0;
}

int
manage_task_I_O_board(void *data, size_t data_len,void **data_addr, size_t *data_addr_len)
{
		
		int i = 0;
		nl_msg_head_t* head = (nl_msg_head_t*)NLMSG_DATA((struct nlmsghdr *)data);

		manage_log(LOG_DEBUG,"manage_task_I_O_board: object(%d) recvfrom sem pid:%d\n", head->object,head->pid);

		if(head->object == NPD_MODULE || head->object == COMMON_MODULE)
		{
			char *chTemp = NLMSG_DATA((struct nlmsghdr *)data) + sizeof(nl_msg_head_t);			 
			
			for(i = 0; i < head->count; i++)
			{
				if(chTemp)
				{
					netlink_msg_t *nl_msg= (netlink_msg_t*)chTemp;	
					manage_log(LOG_DEBUG,"manage_task_I_O_board netlink msgType(%d)\n", nl_msg->msgType);	
					switch(nl_msg->msgType)
					{
						case SYSTEM_STATE_NOTIFIER_EVENT:
							manage_log(LOG_DEBUG,"#####manage_task_I_O_board from sem : SYSTEM_STATE_NOTIFIER_EVENT \n");									
							if(nl_msg->msgData.productInfo.action_type == SYSTEM_READY)
							{
								manage_log(LOG_DEBUG,"#####manage_task_I_O_board System ready.\n");
								
							}
							else if(nl_msg->msgData.productInfo.action_type == SYSTEM_RUNNING)
							{
								manage_log(LOG_DEBUG,"#####manage_task_I_O_board System running.\n");
												
								manage_log(LOG_DEBUG,"Enable asic after system-running OK.\n");											
							}									
							break;
						case BOARD_STATE_NOTIFIER_EVENT:
							manage_log(LOG_DEBUG,"#####manage_task_I_O_board from sem : BOARD_STATE_NOTIFIER_EVENT \n");									
							if(nl_msg->msgData.boardInfo.action_type == BOARD_INSERTED)
							{
								
								manage_log(LOG_DEBUG,"manage_task_I_O_board Board inserted slot %d\n",nl_msg->msgData.boardInfo.slotid);
								manage_task_I_O_board_Time(nl_msg->msgData.boardInfo.slotid);
								
							}
							else if(nl_msg->msgData.boardInfo.action_type == BOARD_REMOVED)
							{
								/* all the other board will receive the msg */
								manage_log(LOG_DEBUG,"manage_task_I_O_board Board remove slot %d\n",nl_msg->msgData.boardInfo.slotid);
							
							}
							else if(nl_msg->msgData.boardInfo.action_type == BOARD_INSERTED_AND_REMOVED)
							{
								/* who receive?? */
								manage_log(LOG_DEBUG,"manage_task_I_O_board Board remove & insert slot %d\n",nl_msg->msgData.boardInfo.slotid);
								
							}
							else
							{
								/* other msg, hot remove-insert */
							}
							break;		
						case ASIC_VLAN_NOTIFIER_ENVET:
							manage_log(LOG_DEBUG,"######manage_task_I_O_board from sem : ASIC_VLAN_NOTIFIER_ENVET vid = %d \n",nl_msg->msgData.vlanInfo.vlan_id);						
							break;
			
						case ACTIVE_STDBY_SWITCH_OCCUR_EVENT:	/* switch start */
							manage_log(LOG_DEBUG,"######manage_task_I_O_board from sem : ACTIVE_STDBY_SWITCH_OCCUR_EVENT \n");
															
							break;								 
						case ACTIVE_STDBY_SWITCH_COMPLETE_EVENT:	/* switch OK, update the master info */
							manage_log(LOG_DEBUG,"#####manage_task_I_O_board from sem : ACTIVE_STDBY_SWITCH_COMPLETE_EVENT \n");
							break;
						case ASIC_ETHPORT_UPDATE_EVENT:
							manage_log(LOG_DEBUG,"#####manage_task_I_O_board from sem : ASIC_ETHPORT_UPDATE_EVENT \n");									
							break;
						default:
							manage_log(LOG_DEBUG,"manage_task_I_O_board Error:npd recv an error message type\n");
							break;
					}

					chTemp = chTemp + sizeof(netlink_msg_t);
				}
			}
		}

}
int
manage_task_register_slot(void *para, size_t para_len,void **data, u_long *data_len)
{
	int ret = -1;
	int slot_id = 0;
	if(NULL == para){
		manage_log(LOG_DEBUG,"manage_task_register_slot:para is NULL\n");
		return -1;
	}
	manage_log(LOG_DEBUG,"manage_task_register_slot:slotid is %s\n",(char*)para);
	slot_id = atoi((char*)para);
	manage_log(LOG_DEBUG,"manage_task_register_slot:slotid is %d\n",slot_id);
	ret = manage_task_I_O_board_Time(slot_id);
	if(ret == 0){
		manage_log(LOG_DEBUG,"manage_task_register_slot:manage_task_I_O_board_Time successful\n");
	}else{
		manage_log(LOG_DEBUG,"manage_task_register_slot:manage_task_I_O_board_Time failed\n");
	}
	return 0;
}


