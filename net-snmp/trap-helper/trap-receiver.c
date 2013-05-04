/* trap-receiver.c */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include "trap-util.h"
#include "trap-list.h"
#include "trap-def.h"
#include "ws_dbus_def.h"
#include "ac_manage_def.h"
#include "ws_snmpd_engine.h"
#include "ws_snmpd_manual.h"
#include "ac_manage_interface.h"
#include "trap-receiver.h"

TrapList gReceiverList = {0};
TrapList gV3UserList = {0};

typedef struct netsnmp_udp_addr_pair_s {
    struct sockaddr_in remote_addr;
    struct in_addr local_addr;
} netsnmp_udp_addr_pair;

static void 
trap_receiver_free(TrapReceiver *tRcv){

	if (NULL != tRcv)
	{
		if( NULL != tRcv->ss )
		{
			snmp_close( tRcv->ss );
		}
		free(tRcv);
	}
}

static int
snmp_input(int operation,
           netsnmp_session * session,
           int reqid, netsnmp_pdu *pdu, void *magic)
{
    return 1;
}


int init_trap_session(TrapReceiver *tRcv, unsigned int type)
{
	netsnmp_session session, *ss=NULL;
	netsnmp_transport *transport=NULL;
	char local_name[136];
	char peer_name[136];
	memset(local_name,0,136);
	memset(peer_name,0,136);

	if (NULL == tRcv) {
		return -1;
	}

	snmp_sess_init(&session);
	 
	session.callback = snmp_input;
	session.callback_magic = NULL;
	session.version = tRcv->version;
	session.community = strdup(tRcv->community);
	session.community_len = strlen(tRcv->community);

    /*
        * init session perrname
        */
    char peername[136] = { 0 };
    snprintf(peername, sizeof(peername) - 1, "%s:%hd", tRcv->dest_ipAddr, tRcv->dest_port ? tRcv->dest_port : 162);
	
	session.peername = strdup(peername);
	session.remote_port = tRcv->dest_port;
    trap_syslog(LOG_INFO,"session.peername: %s\n", session.peername); 
    
	if( NULL != netsnmp_ds_get_string(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_CLIENT_ADDR)){
		netsnmp_ds_set_string(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_CLIENT_ADDR, NULL); //clear local bind ip and port
	}
    
    if(type) {
        if(tRcv->dest_ipAddr[0]) {
            char localname[136] = { 0 };
            snprintf(localname, sizeof(localname) - 1, "%s:%hd", tRcv->sour_ipAddr, tRcv->sour_port);
    	    
    		session.localname = strdup(localname);
    		session.local_port = tRcv->sour_port;
    		
    		trap_syslog(LOG_INFO,"session.localname: %s\n", session.localname); 

    		netsnmp_ds_set_string(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_CLIENT_ADDR, session.localname); //add local bind ip and port
            
            netsnmp_transport *transport = NULL;
            transport = netsnmp_tdomain_transport_full(APPLICATION_NAME, session.peername, 0, "udp", NULL );//"udp", session.peername  ) ; //transport for socket 

            if(NULL == netsnmp_ds_get_string(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_CLIENT_ADDR)) {
                trap_syslog(LOG_INFO, "client_socket is NULL!\n");

                MANAGE_FREE(session.community);
                MANAGE_FREE(session.peername);
                MANAGE_FREE(session.localname);

                return -1;
            }
            
            if ( NULL == transport ){
                trap_syslog(LOG_INFO, "transport is return NULL!\n");   //there should have some fault tolerance. example: the peername can't bind or the peerport has been used

                MANAGE_FREE(session.community);
                MANAGE_FREE(session.peername);
                MANAGE_FREE(session.localname);

                return -1;
            }
            
            ss = snmp_add(&session,
                            transport,
                            NULL, NULL);
            
    	}
    	else{
    		trap_syslog(LOG_INFO, "init_trap_session: This LocalAddr is not exist!\n");   //there should have some fault tolerance. example: the peername can't bind or the peerport has been used
            return -1;
    	}
    }
    else {
        trap_syslog(LOG_INFO, "init_trap_session: This session not bind local ip addr!\n");
        ss = snmp_add(&session, 
                       netsnmp_transport_open_client(APPLICATION_NAME, session.peername), 
                       NULL, NULL);
    }    
	
	if(NULL == ss) {
		trap_syslog(LOG_INFO, "Failed to open client : %s\n", session.peername);
    }
    else{
		if ( NULL != tRcv->ss){
    		snmp_close(tRcv->ss);
    		tRcv->ss = NULL;
	    }
    	trap_syslog(LOG_INFO,"init_trap_session tRcv(%p)->ss(%p)", tRcv, ss);
    	tRcv->ss = ss;
	}

    MANAGE_FREE(session.community);
    MANAGE_FREE(session.peername);
    MANAGE_FREE(session.localname);
	
	return 0;
}

static void
init_trap_receiver_session(TrapReceiver *tRcv) {
    
    if(-1 == init_trap_session(tRcv, 1)) { // try to bind sour IPAddr
        init_trap_session(tRcv, 0);
    }    

    return ;
}

void
init_trap_receverList_session(TrapList *receivelist) {

    if(NULL == receivelist) {
        return ;
    }
    
	TrapNode *tNode = NULL;
	TrapReceiver *tRcv = NULL;

	for(tNode = receivelist->first; tNode; tNode = tNode->next) {
		tRcv = tNode->data;
		if (NULL != tRcv) {
			init_trap_receiver_session(tRcv);
		}
	}
	trap_syslog(LOG_DEBUG, "init_trap_receverList_session(%p)\n", receivelist);
	return 0;
	
}

int close_trap_receiver_session(TrapReceiver *tRcv ) //remove session remove transport and so on
{
	if (NULL == tRcv) {
		return -1;
	}
	
	if ( NULL != tRcv->ss) {
		snmp_close(tRcv->ss);
		tRcv->ss = NULL;
	}
	
	trap_syslog(LOG_INFO, "close_trap_receiver_session tRcv(%p)", tRcv);
	return 0;
}

int close_trap_receiverList_session(TrapList *receivelist)
{
	TrapNode *tNode = NULL;
	TrapReceiver *tRcv = NULL;

	if(NULL == receivelist)
		return -1;

	for(tNode = receivelist->first; tNode; tNode = tNode->next) {
		tRcv = tNode->data;
		if (NULL != tRcv) {
			close_trap_receiver_session(tRcv);
		}
	}
	trap_syslog(LOG_DEBUG,"close_trap_receiverList_session(%p)\n",receivelist);
	return 0;
}



void init_trap_instance_receiver_list(TrapInsVrrpState *ins_vrrp_state)
{
    
	STSNMPTrapReceiver *receiver_array = NULL;
	unsigned int receiver_num = 0;
    int ret = ac_manage_show_trap_receiver(ccgi_dbus_connection, &receiver_array, &receiver_num);
	if(AC_MANAGE_SUCCESS == ret && receiver_array) {
        int i = 0;
        for(i = 0; i < receiver_num; i++) {
        
            if(0 == receiver_array[i].status)       //receiver disable
                continue;

            unsigned int local_id = receiver_array[i].local_id;
            unsigned int instance_id = receiver_array[i].instance_id;
            if(local_id >= VRRP_TYPE_NUM || 0 == instance_id || instance_id > INSTANCE_NUM) {
                trap_syslog(LOG_INFO,"trap_instance_receiver_list_init: get instance or local_id error!\n");
                continue;
            }
            
            if(NULL == ins_vrrp_state->instance[local_id][instance_id].receivelist) {
                TrapList *receivelist = (TrapList *)malloc(sizeof(TrapList));
                if(NULL == receivelist){
                    trap_syslog(LOG_INFO,"trap_instance_receiver_list_init: malloc receiver list error!\n");
                    continue;
                }
                memset(receivelist, 0, sizeof(TrapList));
                ins_vrrp_state->instance[local_id][instance_id].receivelist = receivelist; 
            }
                
            TrapReceiver *tRcv = NULL;
            tRcv = (TrapReceiver *)malloc(sizeof(TrapReceiver));
            if(NULL == tRcv) {
                trap_syslog(LOG_INFO,"trap_instance_receiver_list_init: malloc temp receiver error!\n");
                continue;
            }
            memset(tRcv, 0, sizeof(*tRcv));

            tRcv->local_id = local_id;
            tRcv->instance_id = instance_id;
                
            if(3 == receiver_array[i].version) {
                tRcv->version = SNMP_VERSION_3;
            }
            else if(2 == receiver_array[i].version) {
                tRcv->version = SNMP_VERSION_2c;
                strncpy(tRcv->community, receiver_array[i].trapcom, sizeof(tRcv->community) - 1);
            }
            else {
                tRcv->version = SNMP_VERSION_1;
                strncpy(tRcv->community, receiver_array[i].trapcom, sizeof(tRcv->community) - 1);
            }
            
            strncpy(tRcv->dest_ipAddr, receiver_array[i].dest_ipAddr, sizeof(tRcv->dest_ipAddr) - 1);
            tRcv->dest_port = (unsigned short)receiver_array[i].dest_port;
            
            strncpy(tRcv->sour_ipAddr, receiver_array[i].sour_ipAddr, sizeof(tRcv->sour_ipAddr) - 1);
            tRcv->sour_port = (unsigned short)receiver_array[i].sour_port;

            if(1 == ins_vrrp_state->instance[local_id][instance_id].vrrp_state) {
                init_trap_receiver_session(tRcv);
            }
            
            trap_list_append(ins_vrrp_state->instance[local_id][instance_id].receivelist, tRcv);
        }
	}

    MANAGE_FREE(receiver_array);
	return ;
}

void trap_receiver_list_destroy(TrapList *tRcvList)
{
	trap_list_destroy(tRcvList, (TrapFreeFunc)trap_receiver_free);

	snmp_shutdown(APPLICATION_NAME);
}

void trap_v3user_free(TrapV3User *tV3User)
{
	if (NULL != tV3User)
		free(tV3User);
}

void trap_v3user_list_parse(TrapList *tV3UserList)
{
	FILE *fp = NULL;
	char buf[1024];
	TrapV3User *tV3User;

	trap_list_init(tV3UserList);
	if ( (fp = fopen("/opt/services/conf/snmpd_conf.conf", "r")) == NULL)
		return;

	while (fgets(buf, sizeof(buf), fp) != NULL) {
		if (strncmp(buf, "createUser", strlen("createUser")) == 0) {
			tV3User = malloc(sizeof(*tV3User));
			memset(tV3User, 0, sizeof(*tV3User));
			sscanf(buf, "createUser %32[a-zA-Z0-9.] %8[a-zA-Z0-9.] %21[a-zA-Z0-9.] %8[a-zA-Z0-9.] %21[a-zA-Z0-9.]",
						tV3User->username, tV3User->auth_type, tV3User->auth_passwd,
						tV3User->priv_type, tV3User->priv_passwd);
			trap_list_append(tV3UserList, tV3User);
		}
	}
	fclose(fp);
}

void trap_v3user_list_destroy(TrapList *tV3UserList)
{
	trap_list_destroy(tV3UserList, (TrapFreeFunc)trap_v3user_free);
}

void trap_receiver_list_show(TrapList *tRcvList)
{
	TrapNode *tNode;
	TrapReceiver *tRcv;
	
	for (tNode = tRcvList->first; tNode; tNode = tNode->next) {
		tRcv = tNode->data;
		if (SNMP_VERSION_1 == tRcv->version || SNMP_VERSION_2c == tRcv->version)
			printf("\tversion = %d, ip = %s, community = %s\n", tRcv->version, tRcv->dest_ipAddr, tRcv->community);
		else
			printf("\tversion = %d, ip = %s\n", tRcv->version, tRcv->dest_ipAddr);
	}
}

void trap_v3user_list_show(TrapList *tRcvList)
{
	TrapNode *tNode;
	TrapV3User *tV3User;
	
	for (tNode = tRcvList->first; tNode; tNode = tNode->next) {
		tV3User = tNode->data;
		printf("\tusername = %s, auth_type = %s, auth_pass = %s, priv_type = %s, priv_pass = %s\n",
			tV3User->username, tV3User->auth_type, tV3User->auth_passwd, tV3User->priv_type, tV3User->priv_passwd);
	}
}

void trap_receiver_test(void)
{
	printf("TrapReceiver Test:\n");

	printf("trap receiver list parse\n");
	trap_receiver_list_parse(&gReceiverList);
	trap_receiver_list_show(&gReceiverList);

	printf("trap receiver list destroy\n");
	trap_receiver_list_destroy(&gReceiverList);
	trap_receiver_list_show(&gReceiverList);
	
	printf("trap v3user list parse\n");
	trap_v3user_list_parse(&gV3UserList);
	trap_v3user_list_show(&gV3UserList);

	printf("trap v3user list destroy\n");
	trap_v3user_list_destroy(&gV3UserList);
	trap_v3user_list_show(&gV3UserList);
}

