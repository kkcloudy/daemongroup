/* dcli_rdc.c */

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <zebra.h>
#include <dbus/dbus.h>
#include <arpa/inet.h>

#include "command.h"
#include "dcli_main.h"
#include "eag_errcode.h"
#include "eag_conf.h"
#include "rdc_interface.h"
#include "dcli_rdc.h"

#define SHOW_STR_LEN 	1024*5

struct cmd_node rdc_node = 
{
	RDC_NODE,
	"%s(config-rdc)# "
};

struct cmd_node hansi_rdc_node = 
{
	HANSI_RDC_NODE,
	"%s(hansi-rdc %d-%d)# ",
	1
};

struct cmd_node local_hansi_rdc_node = 
{
	LOCAL_HANSI_RDC_NODE,
	"%s(local-hansi-rdc %d-%d)# ",
	1
};

static char * 
ip2str(uint32_t ip, char *str, size_t size)
{
	if (NULL != str)
	{
		snprintf(str, size, "%u.%u.%u.%u", 
			(ip>>24)&0xff, (ip>>16)&0xff, (ip>>8)&0xff, ip&0xff);
	}

	return str;
}

static char *
inet_ntoa_ex(unsigned long ip, char *ipstr, unsigned int buffsize)
{
	snprintf(ipstr, buffsize, "%lu.%lu.%lu.%lu", ip >> 24,
		 (ip & 0xff0000) >> 16, (ip & 0xff00) >> 8, (ip & 0xff));

	return ipstr;
}

#define RDC_DCLI_INIT_HANSI_INFO	\
int hansitype = HANSI_LOCAL;   	\
int slot_id = HostSlotId;   \
int insid = 0;\
if(vty->node == RDC_NODE){\
	insid = 0;\
}else if(vty->node == HANSI_RDC_NODE){\
	insid = (int)vty->index; 	\
	hansitype = HANSI_REMOTE;\
	slot_id = vty->slotindex; \
}\
else if (vty->node == LOCAL_HANSI_RDC_NODE)\
{\
	insid = (int)vty->index;\
	hansitype = HANSI_LOCAL;\
	slot_id = vty->slotindex;\
}\
DBusConnection *dcli_dbus_connection_curr = NULL;\
ReInitDbusConnection(&dcli_dbus_connection_curr, slot_id, distributFag);

DEFUN(config_rdc_func,
	config_rdc_cmd,
	"config rdc",
	CONFIG_STR
	"Config rdc\n"
)
{
	if (CONFIG_NODE == vty->node)
	{
		vty->node = RDC_NODE;
	}
	else if (HANSI_NODE == vty->node)
	{
		vty->node = HANSI_RDC_NODE;
	}
	else if (LOCAL_HANSI_NODE == vty->node)
	{
		vty->node = LOCAL_HANSI_RDC_NODE;
	}
	else
	{
		vty_out (vty, "%% invalid config mode, node %d\n", vty->node);
		return CMD_WARNING;
	}
	
	return CMD_SUCCESS;
}

DEFUN(rdc_set_nasip_func,
	rdc_set_nasip_cmd,
	"set nasip A.B.C.D",
	SETT_STR
	"rdc nasip\n"
	"rdc nasip\n"
)
{
	int ret = 0;
	uint32_t nasip = 0;
	struct in_addr addr = {0};

	ret = inet_aton(argv[0], &addr);
	if (!ret) {
		vty_out(vty, "%% invalid ip address\n");
		return CMD_WARNING;
	}

	RDC_DCLI_INIT_HANSI_INFO

	nasip = ntohl(addr.s_addr);
	ret = rdc_intf_set_nasip( dcli_dbus_connection_curr,
								hansitype, insid,
								nasip);
	if (EAG_RETURN_OK == ret) {
		return CMD_SUCCESS;
	}
	else if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_ERR_RDC_SERVICE_ALREADY_ENABLE == ret) {
		vty_out(vty, "%% service already started\n");
	}
	else {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}

	//return CMD_FAILURE;
	return CMD_SUCCESS;
}

DEFUN(rdc_set_timeout_func,
	rdc_set_timeout_cmd,
	"set timeout <1-200>",
	SETT_STR
	"rdc packet timeout\n"
	"rdc packet timeout\n"
)
{
	int ret = 0;
	uint32_t timeout = RDC_TIMEOUT_DEFAULT;

	RDC_DCLI_INIT_HANSI_INFO

	timeout = strtoul(argv[0], NULL, 10);
	ret = rdc_intf_set_timeout( dcli_dbus_connection_curr,
								hansitype, insid,
								timeout);
	if (EAG_RETURN_OK == ret) {
		return CMD_SUCCESS;
	}
	else if (EAG_ERR_DBUS_FAILED == ret) {
			vty_out(vty, "%% dbus error\n");
	}
	else {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}

	//return CMD_FAILURE;
	return CMD_SUCCESS;
}

DEFUN(rdc_set_radius_server_func,
	rdc_set_radius_server_cmd,
	"(add|delete)  radius-server  A.B.C.D  SECRET ",
	"set rdc radius config\n"
	"set radius-server\n"
	"radius server  (for authorize)\n"	
	"radius server ip address  (for authorize) \n"		
	"radius server secret(for authorize)\n"
)
{
	int ret=0;
	uint32_t auth_ip=0;
	unsigned short auth_port=0;
	char *auth_secret="";
	unsigned long acct_ip=0;
	unsigned short acct_port=0;
	char *acct_secret="";
		
	struct in_addr inaddr={0};
	
	ret=inet_aton( argv[1],&inaddr);
	if (!ret) {
		vty_out(vty, "%% invalid ip address\n");
		return CMD_WARNING;
	}
	auth_ip = ntohl(inaddr.s_addr);
	/*auth_port is  useless,set a const value*/
	auth_port = 1;
	auth_secret = (char *)argv[2];

	RDC_DCLI_INIT_HANSI_INFO	
	if(0 == strncmp(argv[0],"add",strlen(argv[0]))){
		/*vty_out(vty,"add radius hansitype=%d,insid=%d,auth_ip=%ld,auth_port=%d,auth_sec=%s\n",
					hansitype,insid,auth_ip,auth_port,auth_secret);*/
		ret = rdc_add_radius( dcli_dbus_connection_curr, 
									hansitype,insid,
									auth_ip,
									auth_port,
									auth_secret );
		
		if( EAG_ERR_RADIUS_DOMAIN_LEN_ERR == ret ){
			vty_out(vty,"add radius-server error! error type: domain len error. should be %d!\n", 
							MAX_RADIUS_DOMAIN_LEN-1 );
		}
		else if( EAG_ERR_RADIUS_PARAM_ERR == ret ){
			vty_out(vty,"add radius-server error! error type: ip or port error!\n");
		}
		else if( EAG_ERR_RADIUS_SECRET_LENTH_OUTOFF_SIZE == ret ){
			vty_out(vty,"add radius-server error! error type: secret len out size. should be %d!\n", 
							RADIUS_SECRETSIZE-1);
		}
		else if( EAG_ERR_RADIUS_DOAMIN_AREADY_EXIST == ret ){
			vty_out(vty,"add radius-server error! error type: radius already exist!\n") ;
		}
		else if( EAG_ERR_RADIUS_MAX_NUM_LIMITE == ret ){
			vty_out(vty,"add radius-server error! error type: radius num limite:%d\n", MAX_RADIUS_SRV_NUM ) ;
		}
		else if( EAG_ERR_DBUS_FAILED == ret ){
			vty_out(vty,"add radius-server error! error type: dbus error!\n");
		}
		else if( EAG_RETURN_OK != ret ){
			vty_out(vty,"add radius-server error! error type: unknown!\n");
		}
		else{
			vty_out(vty, "add rdc radius server successfully\n", ret);
		}
	}
	else{		
		/*vty_out(vty,"delete radius hansitype=%d,insid=%d,auth_ip=%ld,auth_port=%d,auth_sec=%s\n",
					hansitype,insid,auth_ip,auth_port,auth_secret);*/
		ret = rdc_del_radius( dcli_dbus_connection_curr, 
									hansitype,insid,
									auth_ip,
									auth_port,
									auth_secret );
		
		if( EAG_ERR_RADIUS_DOMAIN_LEN_ERR == ret ){
			vty_out(vty,"delete radius-server error! error type: domain len error. should be %d!\n", 
							MAX_RADIUS_DOMAIN_LEN-1 );
		}
		else if( EAG_ERR_RADIUS_PARAM_ERR == ret ){
			vty_out(vty,"delete radius-server error! error type: ip or port error!\n");
		}
		else if( EAG_ERR_RADIUS_SECRET_LENTH_OUTOFF_SIZE == ret ){
			vty_out(vty,"delete radius-server error! error type: secret len out size. should be %d!\n", 
							RADIUS_SECRETSIZE-1);
		}
		else if( EAG_ERR_RADIUS_DOAMIN_NOT_EXIST == ret ){
			vty_out(vty,"delete radius-server error! error type: radius not exist!\n") ;
		}
		else if( EAG_ERR_RADIUS_MAX_NUM_LIMITE == ret ){
			vty_out(vty,"delete radius-server error! error type: radius num limite:%d\n", MAX_RADIUS_SRV_NUM ) ;
		}
		else if( EAG_ERR_DBUS_FAILED == ret ){
			vty_out(vty,"delete radius-server error! error type: dbus error!\n");
		}
		else if( EAG_RETURN_OK != ret ){
			vty_out(vty,"delete radius-server error! error type: unknown!\n");
		}
		else{
			vty_out(vty,"delete rdc radius server successfully!\n");
		}
	}
	return CMD_SUCCESS;
}

DEFUN(rdc_show_radius_conf_func,
	rdc_show_radius_conf_cmd,
	"show radius-server",
	"show config\n"
	"show radius-server \n"
	"show radius-server by domain\n"
)
{
	int ret=0,i=0;
	struct rdc_coa_radius_conf radiusconf;
	char *domain="";
	memset( &radiusconf, 0, sizeof(struct rdc_coa_radius_conf) );
	/*vty_out(vty,"domain=%s,radiusconf=%p\n",domain,&radiusconf);*/
	
	RDC_DCLI_INIT_HANSI_INFO
	ret = rdc_get_radius_conf( dcli_dbus_connection_curr, 
								hansitype,insid, domain,
								&radiusconf );

	if( EAG_ERR_DBUS_FAILED == ret ){
		vty_out(vty,"show radius-server error! error type: dbus error!\n");
	}else if( EAG_ERR_RADIUS_DOAMIN_NOT_EXIST == ret ){
		vty_out(vty,"show radius-server error! error type: radius server not find!\n");
	}else if( EAG_RETURN_OK != ret ){
		vty_out(vty,"show radius-server error! error type: unknown!\n");
	}else{
		for( i=0; i<radiusconf.current_num; i++ ){
			char ipstr[32];
			vty_out( vty, "=====================================\n" );
			inet_ntoa_ex( radiusconf.radius_srv[i].auth_ip, ipstr,sizeof(ipstr));
			vty_out( vty, "radius ip :%s\n", ipstr);
			/*vty_out( vty, "radius  port       :%u\n",  radiusconf.radius_srv[i].auth_port);*/
			vty_out( vty, "radius  sercet :%s\n", radiusconf.radius_srv[i].auth_secret);	
		}
	}
	vty_out( vty, "=====================================\n" );

	return CMD_SUCCESS;
}

DEFUN(rdc_set_status_func,
	rdc_set_status_cmd,
	"service rdc (enable|disable)",
	SER_STR
	"start/stop rdc\n"
	"start rdc\n"
	"stop rdc\n"
)
{
	int ret = 0;
	int status = 0;
		
	if (strncmp(argv[0], "enable", strlen(argv[0])) == 0) {
		status = 1;
	} 
	else if (strncmp(argv[0], "disable", strlen(argv[0])) == 0) {
		status = 0;
	}
	else {
		vty_out(vty,"%% bad command parameter\n");
		return CMD_WARNING;
	}		

	RDC_DCLI_INIT_HANSI_INFO

	ret = rdc_intf_set_status( dcli_dbus_connection_curr,
								hansitype, insid,
								status);
	if (EAG_RETURN_OK == ret) {
		return CMD_SUCCESS;
	}
	else if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_ERR_RDC_SERVICE_ALREADY_ENABLE == ret) {
		vty_out(vty, "%% service already enable\n");
	}
	else if (EAG_ERR_RDC_SERVICE_ALREADY_DISABLE == ret) {
		vty_out(vty, "%% service already disable\n");
	}
	else if (EAG_ERR_RDC_PRODUCT_NOT_DISTRIBUTED == ret) {
		vty_out(vty, "%% product not distributed\n");
	}
	else if (EAG_ERR_SOCKET_BIND_FAILED == ret) {
		vty_out(vty, "%% cannot bind ip/port\n");
	}
	else {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}

	//return CMD_FAILURE;
	return CMD_SUCCESS;
}

DEFUN(rdc_show_base_conf_func,
	rdc_show_base_conf_cmd,
	"show rdc-base-config",
	SHOW_STR
	"show rdc base config\n"
)
{
	int ret = 0;
	char ipstr[32] = "";
	struct rdc_base_conf baseconf = {0};

	RDC_DCLI_INIT_HANSI_INFO

	ret = rdc_intf_get_base_conf( dcli_dbus_connection_curr,
						hansitype, insid,
						&baseconf);
	if (EAG_RETURN_OK == ret) {
		vty_out(vty, "%-15s : %s\n", "service", 
					baseconf.status?"enable":"disable");
		ip2str(baseconf.nasip, ipstr, sizeof(ipstr));
		vty_out(vty, "%-15s : %s\n", "nasip", ipstr);
		vty_out(vty, "%-15s : %u\n", "timeout", baseconf.timeout);
		return CMD_SUCCESS;
	}
	else if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}

	return CMD_FAILURE;
}

DEFUN(rdc_add_debug_filter_func,
	rdc_add_debug_filter_cmd,
	"add debug-filter FILTER",
	"add\n"
	"rdc debug filter string\n"
	"rdc debug filter string\n"
)
{
	int ret = 0;

	RDC_DCLI_INIT_HANSI_INFO

	ret = rdc_add_debug_filter( dcli_dbus_connection_curr,
						hansitype, insid,
						argv[0]);

	if (EAG_RETURN_OK == ret) {
		return CMD_SUCCESS;
	}
	else if (EAG_ERR_DBUS_FAILED == ret) {
			vty_out(vty, "%% dbus error\n");
	}
	else {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}

	return CMD_FAILURE;
}

DEFUN(rdc_del_debug_filter_func,
	rdc_del_debug_filter_cmd,
	"delete debug-filter FILTER",
	"delete\n"
	"rdc debug filter string\n"
	"rdc debug filter string\n"
)
{
	int ret = 0;

	RDC_DCLI_INIT_HANSI_INFO

	ret = rdc_del_debug_filter( dcli_dbus_connection_curr,
						hansitype, insid,
						argv[0]);

	if (EAG_RETURN_OK == ret) {
		return CMD_SUCCESS;
	}
	else if (EAG_ERR_DBUS_FAILED == ret) {
			vty_out(vty, "%% dbus error\n");
	}
	else {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}

	return CMD_FAILURE;
}

DEFUN(rdc_log_all_packetconn_func,
	rdc_log_all_packetconn_cmd,
	"log all packetconn",
	"do log\n"
	"log all rdc packetconn\n"
	"log all rdc packetconn\n"
)
{
	int ret = -1;

	RDC_DCLI_INIT_HANSI_INFO

	ret = rdc_log_all_packetconn( dcli_dbus_connection_curr,
									hansitype, insid);

	if (EAG_RETURN_OK == ret) {
		return CMD_SUCCESS;
	}
	else if (EAG_ERR_DBUS_FAILED == ret) {
			vty_out(vty, "%% dbus error\n");
	}
	else {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}

	return CMD_FAILURE;
}

DEFUN(rdc_log_all_userconn_func,
	rdc_log_all_userconn_cmd,
	"log all userconn",
	"do log\n"
	"log all rdc userconn\n"
	"log all rdc userconn\n"
)
{
	int ret = -1;

	RDC_DCLI_INIT_HANSI_INFO

	ret = rdc_log_all_userconn( dcli_dbus_connection_curr,
									hansitype, insid);

	if (EAG_RETURN_OK == ret) {
		return CMD_SUCCESS;
	}
	else if (EAG_ERR_DBUS_FAILED == ret) {
			vty_out(vty, "%% dbus error\n");
	}
	else {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}

	return CMD_FAILURE;
}


DEFUN(rdc_log_all_sockclient_func,
	rdc_log_all_sockclient_cmd,
	"log all sockclient",
	"do log\n"
	"log all rdc sockclient\n"
	"log all rdc sockclient\n"
)
{
	int ret = -1;

	RDC_DCLI_INIT_HANSI_INFO

	ret = rdc_log_all_sockclient( dcli_dbus_connection_curr,
									hansitype, insid);

	if (EAG_RETURN_OK == ret) {
		return CMD_SUCCESS;
	}
	else if (EAG_ERR_DBUS_FAILED == ret) {
			vty_out(vty, "%% dbus error\n");
	}
	else {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}

	return CMD_FAILURE;
}

DEFUN(rdc_log_all_thread_func,
	rdc_log_all_thread_cmd,
	"log all-thread",
	"do log\n"
	"log all thread\n"

)
{
	int ret = -1;

	RDC_DCLI_INIT_HANSI_INFO

	ret = rdc_log_all_thread( dcli_dbus_connection_curr,
									hansitype, insid);

	if (EAG_RETURN_OK == ret) {
		return CMD_SUCCESS;
	}
	else if (EAG_ERR_DBUS_FAILED == ret) {
			vty_out(vty, "%% dbus error\n");
	}
	else {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}
	return CMD_FAILURE;
}

DEFUN(rdc_log_all_blkmem_func,
	rdc_log_all_blkmem_cmd,
	"log all-blkmem",
	"do log\n"
	"log all blkmem\n"
)
{
	int ret = -1;

	RDC_DCLI_INIT_HANSI_INFO

	ret = rdc_log_all_blkmem( dcli_dbus_connection_curr,
									hansitype, insid);

	if (EAG_RETURN_OK == ret) {
		return CMD_SUCCESS;
	}
	else if (EAG_ERR_DBUS_FAILED == ret) {
			vty_out(vty, "%% dbus error\n");
	}
	else {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}
	return CMD_FAILURE;
}

int rdc_has_config(void)
{
	int ret = 0;
	char *domain="";
	struct rdc_base_conf baseconf = {0};
	struct rdc_coa_radius_conf radiusconf = {0};

	ret = rdc_intf_get_base_conf(dcli_dbus_connection, HANSI_LOCAL, 0,
						&baseconf);
	if (EAG_RETURN_OK == ret) {
		if (0 != baseconf.nasip) {
			return 1;
		}
		if (RDC_TIMEOUT_DEFAULT != baseconf.timeout) {
			return 1;
		}
		if (0 != baseconf.status) {
			return 1;
		}
	}
	
	ret = rdc_get_radius_conf( dcli_dbus_connection, HANSI_LOCAL, 0, 
						domain,	&radiusconf );
	if( EAG_RETURN_OK == ret ){
		if (radiusconf.current_num > 0) {
			return 1;
		}	
	}

	return 0;
}

int dcli_rdc_show_running_config(struct vty* vty)
{
	int ret = 0;
	char showStr[SHOW_STR_LEN] = "";
	char ipstr[32] = "";
	struct rdc_base_conf baseconf = {0};
	int i=0;
	struct rdc_coa_radius_conf radiusconf;
	char *domain="";
	memset( &radiusconf, 0, sizeof(struct rdc_coa_radius_conf) );

	snprintf(showStr, sizeof(showStr), BUILDING_MOUDLE, "RDC");
	vtysh_add_show_string(showStr);

	if (!rdc_has_config()) {
		return CMD_SUCCESS;
	}
	
	vtysh_add_show_string("config rdc");
	
	ret = rdc_intf_get_base_conf( dcli_dbus_connection,
						HANSI_LOCAL, 0,
						&baseconf);
	if (EAG_RETURN_OK == ret) {
		if (0 != baseconf.nasip) {
			ip2str(baseconf.nasip, ipstr, sizeof(ipstr));
			snprintf(showStr, sizeof(showStr), " set nasip %s",
						ipstr);
			vtysh_add_show_string(showStr);
		}
		if (RDC_TIMEOUT_DEFAULT != baseconf.timeout) {
			snprintf(showStr, sizeof(showStr), " set timeout %u",
						baseconf.timeout);
			vtysh_add_show_string(showStr);
		}
	}
	
	ret = rdc_get_radius_conf( dcli_dbus_connection, 
								HANSI_LOCAL, 0, domain,
								&radiusconf );

	if( EAG_RETURN_OK == ret ){
		for( i=0; i<radiusconf.current_num; i++ ){
			char ipstr[32];
			memset(showStr, 0, sizeof(showStr));
			inet_ntoa_ex( radiusconf.radius_srv[i].auth_ip, ipstr,sizeof(ipstr));			
			snprintf(showStr, sizeof(showStr), " add radius-server %s %s",ipstr,radiusconf.radius_srv[i].auth_secret);
			vtysh_add_show_string(showStr);
		}	
	}

	if (0 != baseconf.status) {
		snprintf(showStr, sizeof(showStr), " service rdc enable");
		vtysh_add_show_string(showStr);
	}

	vtysh_add_show_string(" exit");
	
	return CMD_SUCCESS;
}

char * 
dcli_rdc_show_running_config_2(int localid, int slot_id, int index)
{
	char *tmp = NULL;
	int ret = -1;
	char showStr[SHOW_STR_LEN] = "";
	char *cursor = NULL;
	int totalLen = 0;
	int tmplen = 0;
	DBusConnection *dcli_dbus_connection_curr = NULL;
	char ipstr[32] = "";
	struct rdc_base_conf baseconf = {0};

	int i=0;
	struct rdc_coa_radius_conf radiusconf;
	char *domain="";
	memset( &radiusconf, 0, sizeof(struct rdc_coa_radius_conf) );
	
	memset (showStr, 0, sizeof(showStr));
	cursor = showStr;
	totalLen = 0;

	totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, 
					"config rdc\n");
	tmplen = totalLen;
	
	ReInitDbusConnection(&dcli_dbus_connection_curr,slot_id,distributFag);
	
	ret = rdc_intf_get_base_conf( dcli_dbus_connection_curr,
						localid, index, &baseconf);
	if (EAG_RETURN_OK == ret) {
		if (0 != baseconf.nasip) {
			ip2str(baseconf.nasip, ipstr, sizeof(ipstr));
			totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, 
								" set nasip %s\n", ipstr);
		}
		if (RDC_TIMEOUT_DEFAULT != baseconf.timeout) {
			totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, 
						" set timeout %u\n", baseconf.timeout);
		}
	}

	ret = rdc_get_radius_conf( dcli_dbus_connection_curr, 
								localid, index, domain,
								&radiusconf );

	if( EAG_RETURN_OK == ret ){
		for( i=0; i<radiusconf.current_num; i++ ){
			char ipstr[32];
			inet_ntoa_ex( radiusconf.radius_srv[i].auth_ip, ipstr,sizeof(ipstr));
			totalLen +=snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1,
						" add radius-server %s %s\n",ipstr,radiusconf.radius_srv[i].auth_secret);			
		}
	}
	
	if (0 != baseconf.status) {
		totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, 
					" service rdc enable\n");
	}

	/* rdc has no config */
	if (totalLen == tmplen) {
		tmp = (char*)malloc(1);
		if (NULL == tmp){
			return NULL;
		}
		memset(tmp, 0 , 1);
		return tmp;
	}

	/* rdc has config */
	totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " exit\n");
	
	tmp = malloc(strlen(showStr)+1);
	if (NULL == tmp){
		return NULL;
	}
	memset (tmp, 0, strlen(showStr)+1);
	memcpy(tmp, showStr, strlen(showStr));
	return tmp;
}

void dcli_rdc_init(void)
{
	install_node(&rdc_node, dcli_rdc_show_running_config,"rdc");
	install_default(RDC_NODE);
	install_element(CONFIG_NODE, &config_rdc_cmd);

	install_node(&hansi_rdc_node, NULL,"rdc-hansi");
	install_default(HANSI_RDC_NODE);
	install_element(HANSI_NODE, &config_rdc_cmd);
		
	install_node(&local_hansi_rdc_node, NULL,"rdc-local-hansi");
	install_default(LOCAL_HANSI_RDC_NODE);
	install_element(LOCAL_HANSI_NODE, &config_rdc_cmd);

	/* rdc */
	install_element(RDC_NODE, &rdc_set_nasip_cmd);
	install_element(HANSI_RDC_NODE, &rdc_set_nasip_cmd);
	install_element(LOCAL_HANSI_RDC_NODE, &rdc_set_nasip_cmd);
	
	install_element(RDC_NODE, &rdc_set_radius_server_cmd);
	install_element(HANSI_RDC_NODE, &rdc_set_radius_server_cmd);
	install_element(LOCAL_HANSI_RDC_NODE, &rdc_set_radius_server_cmd);

	install_element(RDC_NODE, &rdc_show_radius_conf_cmd);
	install_element(HANSI_RDC_NODE, &rdc_show_radius_conf_cmd);
	install_element(LOCAL_HANSI_RDC_NODE, &rdc_show_radius_conf_cmd);

	install_element(RDC_NODE, &rdc_set_timeout_cmd);
	install_element(HANSI_RDC_NODE, &rdc_set_timeout_cmd);
	install_element(LOCAL_HANSI_RDC_NODE, &rdc_set_timeout_cmd);

	install_element(RDC_NODE, &rdc_set_status_cmd);
	install_element(HANSI_RDC_NODE, &rdc_set_status_cmd);
	install_element(LOCAL_HANSI_RDC_NODE, &rdc_set_status_cmd);

	install_element(RDC_NODE, &rdc_show_base_conf_cmd);
	install_element(HANSI_RDC_NODE, &rdc_show_base_conf_cmd);
	install_element(LOCAL_HANSI_RDC_NODE, &rdc_show_base_conf_cmd);

	install_element(RDC_NODE, &rdc_add_debug_filter_cmd);
	install_element(HANSI_RDC_NODE, &rdc_add_debug_filter_cmd);
	install_element(LOCAL_HANSI_RDC_NODE, &rdc_add_debug_filter_cmd);

	install_element(RDC_NODE, &rdc_del_debug_filter_cmd);
	install_element(HANSI_RDC_NODE, &rdc_del_debug_filter_cmd);
	install_element(LOCAL_HANSI_RDC_NODE, &rdc_del_debug_filter_cmd);

	install_element(RDC_NODE, &rdc_log_all_packetconn_cmd);
	install_element(HANSI_RDC_NODE, &rdc_log_all_packetconn_cmd);
	install_element(LOCAL_HANSI_RDC_NODE, &rdc_log_all_packetconn_cmd);

    install_element(RDC_NODE, &rdc_log_all_userconn_cmd);
    install_element(HANSI_RDC_NODE, &rdc_log_all_userconn_cmd);
    install_element(LOCAL_HANSI_RDC_NODE, &rdc_log_all_userconn_cmd);

	install_element(RDC_NODE, &rdc_log_all_sockclient_cmd);
	install_element(HANSI_RDC_NODE, &rdc_log_all_sockclient_cmd);
	install_element(LOCAL_HANSI_RDC_NODE, &rdc_log_all_sockclient_cmd);

	install_element(RDC_NODE, &rdc_log_all_blkmem_cmd);
	install_element(HANSI_RDC_NODE, &rdc_log_all_blkmem_cmd);
	install_element(LOCAL_HANSI_RDC_NODE, &rdc_log_all_blkmem_cmd);

	install_element(RDC_NODE, &rdc_log_all_thread_cmd);
	install_element(HANSI_RDC_NODE, &rdc_log_all_thread_cmd);
	install_element(LOCAL_HANSI_RDC_NODE, &rdc_log_all_thread_cmd);
}

