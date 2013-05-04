/** include glibc **/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

#include <sys/types.h>

#include "ws_dbus_list_interface.h"
#include "board/board_define.h"

/** include manage lib **/
#include "manage_log.h"
#include "manage_type.h"
#include "manage_api.h"

#include "ac_manage_def.h"
#include "ac_manage_public.h"

#include "ac_manage_task_handler.h"

#include "ac_manage_session.h"
#include "board/netlink.h"
#include "sem/product.h"


#define _MANAGE_TIPC_RELAY_

static manage_method method_array[] = {
	{AC_MANAGE_TASK_METHOD_TEST, manage_task_test},
	{AC_MANAGE_TASK_METHOD_INTERFACE_INFO, manage_task_interface_info},
	{AC_MANAGE_TASK_METHOD_REGISTER_SLOT,manage_task_register_slot}
};

static u_long	method_array_size = sizeof(method_array)/sizeof(method_array[0]);
static manage_session *sp = NULL;


static int
_session_tipc_init() {
	manage_session session;
	manage_tipc_addr local_tipc;
	
	memset(&session, 0, sizeof(manage_session));
	memset(&local_tipc, 0, sizeof(manage_tipc_addr));

	session.flags |= MANAGE_FLAGS_TIPC_SOCKET;	

	if(0 == local_slotID) {
		if(VALID_DBM_FLAG == get_dbm_effective_flag())
		{
			local_slotID = manage_get_product_info(PRODUCT_LOCAL_SLOTID);
		}
		if(0 == local_slotID) {
			manage_log(LOG_WARNING, "_session_tipc_init: get local slot id error\n");
			return AC_MANAGE_FILE_OPEN_FAIL;
		}
	}

	if(0 == active_master_slotID) {
		if(VALID_DBM_FLAG == get_dbm_effective_flag())
		{
			active_master_slotID = manage_get_product_info(PRODUCT_ACTIVE_MASTER);
		}
		if(0 == active_master_slotID) {
			manage_log(LOG_WARNING, "_session_tipc_init: get active master slot id error\n");
			return AC_MANAGE_FILE_OPEN_FAIL;
		}
	}

	if(active_master_slotID == local_slotID) {
#ifdef _MANAGE_TIPC_RELAY_
		session.flags |= MANAGE_FLAGS_RELAY_SOCKET;
#endif	
	}

	local_tipc.type = MANAGE_TIPC_TYPE;
	local_tipc.instance = (0x1000 + local_slotID);

	manage_log(LOG_INFO, "_session_tipc_init: tipc type = 0x%x, instance = 0x%x\n", 
					local_tipc.type, local_tipc.instance);

	session.local = (void *)&local_tipc;
	session.local_len = sizeof(manage_tipc_addr);
	session.Method_s = NULL;

	if(NULL == (sp = manage_open(&session))) {
		manage_log(LOG_ERR, "_session_tipc_init: s_manage_errno = %d, s_errno = %d\n",
						session.s_manage_errno, session.s_errno);
		return MANAGEERR_OPEN_FAIL;
	}

	manage_log(LOG_INFO, "_session_tipc_init: tipc session is %p\n", sp);
	
	return AC_MANAGE_SUCCESS;
}

int
_session_netlink_sem_init()
{
		manage_session sess_netlink,*sp_net;
		manage_netlink_addr_group local_netlink;
		unsigned int is_active_master = 0;
		
		memset(&sess_netlink, 0, sizeof(manage_session));
		memset(&local_netlink, 0, sizeof(manage_tipc_addr));
	
		sess_netlink.flags |= MANAGE_FLAGS_NETLINK_SEM_SOCKET;	

		if(VALID_DBM_FLAG == get_dbm_effective_flag())
		{
			is_active_master = manage_get_product_info(PRODUCT_LOCAL_IS_ACTIVE_MASTER);
		}
		
		if(0 == is_active_master) {
				manage_log(LOG_WARNING, "_session_netlink_init: get active master slot id error\n");
				manage_log(LOG_INFO, "_session_netlink_init: there is stop\n");
				return AC_MANAGE_FILE_OPEN_FAIL;
		}

	
		local_netlink.src.protocol = NETLINK_DISTRIBUTED;
		local_netlink.src.nl_pid = getpid();
		local_netlink.src.nl_groups = 1;


		local_netlink.dest.protocol = NETLINK_DISTRIBUTED;
		local_netlink.dest.nl_pid = 0;
		local_netlink.dest.nl_groups = 1;

		sess_netlink.local = (void *)&local_netlink;
		sess_netlink.local_len = sizeof(manage_netlink_addr_group);
		sess_netlink.Method_s = manage_task_I_O_board;
		if(NULL == (sp_net = manage_open(&sess_netlink))) {
			manage_log(LOG_ERR, "_session_tipc_init: s_manage_errno = %d, s_errno = %d\n",
							sess_netlink.s_manage_errno, sess_netlink.s_errno);
			return MANAGEERR_OPEN_FAIL;
		}
	
		manage_log(LOG_INFO, "_session_tipc_init: tipc session is %p\n", sp_net);
		
		return AC_MANAGE_SUCCESS;


}
void
manage_register(){
	struct session_list *slp;
	manage_session * session_reg;
	int ret = 0;
	char  data[10] = { 0 };
		
	manage_message *message = NULL;
	manage_tipc_addr_group addr_group;	
	memset(&addr_group, 0, sizeof(manage_tipc_addr_group));

	if(0 == local_slotID) {
		if(VALID_DBM_FLAG == get_dbm_effective_flag())
		{
			local_slotID = manage_get_product_info(PRODUCT_LOCAL_SLOTID);
		}
		if(0 == local_slotID) {
			manage_log(LOG_WARNING, "manage_register: get local slot id error\n");
			return ;
		}
	}
	if(0 == active_master_slotID) {
		if(VALID_DBM_FLAG == get_dbm_effective_flag())
		{
			active_master_slotID = manage_get_product_info(PRODUCT_ACTIVE_MASTER);
		}
		if(0 == active_master_slotID) {
			manage_log(LOG_WARNING, "manage_register: get active master slot id error\n");
			return ;
		}
	}
	if(active_master_slotID == local_slotID) {
		manage_log(LOG_WARNING, "manage_register:It is master slot,so do not need to register!\n");	
		return ;
	}else{
		//do register
		memset(data,0,sizeof(data));
		sprintf(data,"%d",local_slotID);
		manage_log(LOG_DEBUG, "manage_register:data:%s\n",data);	
		
		message = manage_message_new(AC_MANAGE_TASK_METHOD_REGISTER_SLOT, 
									data, strlen(data));
		if( !message || !sp ){
			manage_log(LOG_ERR, "manage_register:message create failed!\n"); 
			return;
		}else{
			addr_group.dest.type = MANAGE_TIPC_TYPE;	/*tipc*/
			addr_group.dest.instance = 0x1000 + active_master_slotID;		/*slot id*/
			
			memcpy(&(addr_group.sour), sp->local, sizeof(manage_tipc_addr));
			if((ret = manage_message_send(sp, message,&addr_group,sizeof(manage_tipc_addr_group)))< 0){
				manage_log(LOG_WARNING, "manage_register_send_no_reply: send message fail\n");
				return -2;
			}
		}
	}
	MANAGE_FREE(message);
	return;
}



void
manage_session_init(void) {
	_session_tipc_init();
	//_session_netlink_sem_init();	
}

void
manage_session_destroy(void) {
	manage_close_all();
}

void
manage_task_init(void) {	
	manage_method_regist(method_array, method_array_size);
}

