/* dcli_pdc.c */

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
#include "pdc_interface.h"
#include "dcli_pdc.h"

#define SHOW_STR_LEN 	1024*5

struct cmd_node pdc_node = 
{
	PDC_NODE,
	"%s(config-pdc)# "
};

struct cmd_node hansi_pdc_node = 
{
	HANSI_PDC_NODE,
	"%s(hansi-pdc %d-%d)# ",
	1
};

struct cmd_node local_hansi_pdc_node = 
{
	LOCAL_HANSI_PDC_NODE,
	"%s(local-hansi-pdc %d-%d)# ",
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

static int
is_mask( unsigned long mask )
{
	int i;
	long temp = (long)mask;/*”–∑˚∫≈”““∆±£¡Ù∑˚∫≈Œª*/

	for( i=0;i<32;i++ ){
		//printf("temp = %08X\n",(unsigned int)temp);
		if( 1 == (temp&0x1) ){/*right shift until last bit to  1*/
			break;
		}
		
		temp >>= 1;
	}

	return ((~temp)==0);
}

unsigned int
mask2binary(unsigned int mask){
	unsigned int mask_ret = 0xffffffff;
	if (0 == mask)
	 {
	    return 0;
	}
	mask_ret = (mask_ret << (32 - mask));
	return mask_ret;
}

unsigned int
binary2mask(unsigned int mask){
	int i = 0;
	if (0xffffffff == mask){
	    return 32;
	}
	for (i = 0; i < 32; i++)
	{
		if ( 0 == mask << i){
			return i;
		}

	}
}

#define PDC_VALUE_IP_MASK_CHECK(num) 					\
if((num<1)||(num>32)) 		\
{													\
	vty_out(vty,"%% Illegal ip mask value!\n");    \
	return CMD_WARNING;							\
}


#define PDC_DCLI_INIT_HANSI_INFO	\
int hansitype = HANSI_LOCAL;   	\
int slot_id = HostSlotId;   \
int insid = 0;\
if(vty->node == PDC_NODE){\
	insid = 0;\
}else if(vty->node == HANSI_PDC_NODE){\
	insid = (int)vty->index; 	\
	hansitype = HANSI_REMOTE;\
	slot_id = vty->slotindex; \
}\
else if (vty->node == LOCAL_HANSI_PDC_NODE)\
{\
	insid = (int)vty->index;\
	hansitype = HANSI_LOCAL;\
	slot_id = vty->slotindex;\
}\
DBusConnection *dcli_dbus_connection_curr = NULL;\
ReInitDbusConnection(&dcli_dbus_connection_curr, slot_id, distributFag);

DEFUN(config_pdc_func,
	config_pdc_cmd,
	"config pdc",
	CONFIG_STR
	"Config pdc\n"
)
{
	if (CONFIG_NODE == vty->node)
	{
		vty->node = PDC_NODE;
	}
	else if (HANSI_NODE == vty->node)
	{
		vty->node = HANSI_PDC_NODE;
	}
	else if (LOCAL_HANSI_NODE == vty->node)
	{
		vty->node = LOCAL_HANSI_PDC_NODE;
	}
	else
	{
		vty_out (vty, "%% invalid config mode, node %d\n", vty->node);
		return CMD_WARNING;
	}
	
	return CMD_SUCCESS;
}

DEFUN(pdc_set_nasip_func,
	pdc_set_nasip_cmd,
	"set nasip A.B.C.D",
	SETT_STR
	"pdc nasip\n"
	"pdc nasip\n"
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

	PDC_DCLI_INIT_HANSI_INFO

	nasip = ntohl(addr.s_addr);
	ret = pdc_intf_set_nasip( dcli_dbus_connection_curr,
								hansitype, insid,
								nasip);
	if (EAG_RETURN_OK == ret) {
		return CMD_SUCCESS;
	}
	else if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_ERR_PDC_SERVICE_ALREADY_ENABLE == ret) {
		vty_out(vty, "%% service already started\n");
	}
	else {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}

	//return CMD_FAILURE;
	return CMD_SUCCESS;
}

DEFUN(pdc_set_portal_port_func,
	pdc_set_portal_port_cmd,
	"set portal-port <1-65535>",
	SETT_STR
	"pdc port\n"
	"pdc port\n"
)
{
	int ret = 0;
	uint16_t port = PDC_DEFAULT_PORTAL_PORT;

	PDC_DCLI_INIT_HANSI_INFO

	port = (uint16_t)strtoul(argv[0], NULL, 10);
	ret = pdc_intf_set_port( dcli_dbus_connection_curr,
								hansitype, insid,
								port);
	if (EAG_RETURN_OK == ret) {
		return CMD_SUCCESS;
	}
	else if (EAG_ERR_DBUS_FAILED == ret) {
			vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_ERR_PDC_SERVICE_ALREADY_ENABLE == ret) {
		vty_out(vty, "%% service already started\n");
	}
	else {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}

	//return CMD_FAILURE;
	return CMD_SUCCESS;
}

DEFUN(pdc_set_status_func,
	pdc_set_status_cmd,
	"service pdc (enable|disable)",
	SER_STR
	"start/stop pdc\n"
	"start pdc\n"
	"stop pdc\n"
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

	PDC_DCLI_INIT_HANSI_INFO

	ret = pdc_intf_set_status( dcli_dbus_connection_curr,
								hansitype, insid,
								status);
	if (EAG_RETURN_OK == ret) {
		return CMD_SUCCESS;
	}
	else if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_ERR_PDC_SERVICE_ALREADY_ENABLE == ret) {
		vty_out(vty, "%% service already enable\n");
	}
	else if (EAG_ERR_PDC_SERVICE_ALREADY_DISABLE == ret) {
		vty_out(vty, "%% service already disable\n");
	}
	else if (EAG_ERR_PDC_PRODUCT_NOT_DISTRIBUTED == ret) {
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

DEFUN(pdc_show_base_conf_func,
	pdc_show_base_conf_cmd,
	"show pdc-base-config",
	SHOW_STR
	"show pdc base config\n"
)
{
	int ret = 0;
	char ipstr[32] = "";
	struct pdc_base_conf baseconf = {0};

	PDC_DCLI_INIT_HANSI_INFO

	ret = pdc_intf_get_base_conf( dcli_dbus_connection_curr,
						hansitype, insid,
						&baseconf);
	if (EAG_RETURN_OK == ret) {
		vty_out(vty, "%-15s : %s\n", "service", 
					baseconf.status?"enable":"disable");
		ip2str(baseconf.nasip, ipstr, sizeof(ipstr));
		vty_out(vty, "%-15s : %s\n", "nasip", ipstr);
		vty_out(vty, "%-15s : %u\n", "port", baseconf.port);
		vty_out(vty, "%-15s : %s\n", "portal protocol", (PORTAL_PROTOCOL_MOBILE == baseconf.portal_protocol)?"mobile":"telecom");
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

DEFUN(pdc_add_map_func,
	pdc_add_map_cmd,
	"add pdc-map user-subnet A.B.C.D/MASK hansi PRARM",
	"add\n"
	"add pdc-map\n"
	"user subnet A.B.C.D/MASK\n"
	"user subnet A.B.C.D/MASK\n"
	"hansi slotid-insid\n"
	"hansi slotid-insid\n"
)
{
	int ret = 0;
	uint32_t userip = 0;
	uint32_t usermask = 0;
	int  eag_slotid = 0;
	int eag_insid = 0;
	struct in_addr inaddr;
	char ipstr[32] = "";
	char maskstr[32] = "";

	ret = ip_address_format2ulong((char**)&argv[0],&userip,&usermask);	
	if(CMD_WARNING == ret){
		vty_out(vty, "%% invalid ip address\n");
		return CMD_FAILURE;
	}
	PDC_VALUE_IP_MASK_CHECK(usermask);
	usermask = mask2binary(usermask);

	ret = sscanf(argv[1],"%d-%d", &eag_slotid, &eag_insid );
	if( ret != 2 ){
		vty_out(vty, "the PARAM should format like  1-3(slotid-insid)!\n");
		return CMD_FAILURE;
	}

	if( eag_slotid > 10 || eag_insid > 16 ||
		eag_slotid < 0 || eag_insid < 0){
		vty_out(vty, "Slot id should less than 10 and insid shold less than 16\n");		
		return CMD_FAILURE;
	}

	PDC_DCLI_INIT_HANSI_INFO

	ret = pdc_intf_add_map( dcli_dbus_connection_curr,
						hansitype, insid,
						userip, usermask, eag_slotid, eag_insid);
	
	if (EAG_RETURN_OK == ret) {
		return CMD_SUCCESS;
	}
	else if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_ERR_PDC_MAP_CONFLICT == ret){
		vty_out(vty,"%% pdc map conflict\n");
	}
	else if (EAG_ERR_PDC_MAP_NOT_FOUND == ret){
		vty_out(vty,"%% pdc map not found\n");
	}
	else if (EAG_ERR_PDC_MAP_NUM_OVERSTEP == ret){
		vty_out(vty,"%% pdc map over limit\n");
	}
	else {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}

	return CMD_FAILURE;
	
}

DEFUN(pdc_del_map_func,
	pdc_del_map_cmd,
	"delete pdc-map user-subnet A.B.C.D/MASK",
	"delete\n"
	"delete pdc-map\n"
	"user-subnet A.B.C.D/MASK\n"
	"user-subnet A.B.C.D/MASK\n"
)
{
	int ret = 0;
	uint32_t userip = 0;
	uint32_t usermask = 0;
	struct in_addr inaddr;
	char ipstr[32] = "";
	char maskstr[32] = "";

	ret = ip_address_format2ulong((char**)&argv[0],&userip,&usermask);	
	if(CMD_WARNING == ret){
		vty_out(vty, "%% invalid ip address\n");
		return CMD_FAILURE;
	}
	PDC_VALUE_IP_MASK_CHECK(usermask);
	usermask = mask2binary(usermask);

	PDC_DCLI_INIT_HANSI_INFO

	ret = pdc_intf_del_map( dcli_dbus_connection_curr,
						hansitype, insid,
						userip, usermask);
	
	if (EAG_RETURN_OK == ret) {
		return CMD_SUCCESS;
	}
	else if (EAG_ERR_DBUS_FAILED == ret) {
			vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_ERR_PDC_MAP_CONFLICT == ret){
		vty_out(vty,"%% pdc map conflict\n");
	}
	else if (EAG_ERR_PDC_MAP_NOT_FOUND == ret){
		vty_out(vty,"%% pdc map not found\n");
	}
	else if (EAG_ERR_PDC_MAP_NUM_OVERSTEP == ret){
		vty_out(vty,"%% pdc map over limit\n");
	}
	else {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}

	return CMD_FAILURE;
	
}

DEFUN(pdc_modify_map_func,
	pdc_modify_map_cmd,
	"set pdc-map user-subnet A.B.C.D/MASK hansi PRARM",
	"set\n"
	"set pdc-map\n"
	"user-subnet A.B.C.D/MASK\n"
	"user-subnet A.B.C.D/MASK\n"
	"hansi slotid-insid\n"
	"hansi slotid-insid\n"
)
{
	int ret = 0;
	uint32_t userip = 0;
	uint32_t usermask = 0;
	int  eag_slotid = 0;
	int eag_insid = 0;
	struct in_addr inaddr;
	char ipstr[32] = "";
	char maskstr[32] = "";
	
	ret = ip_address_format2ulong((char**)&argv[0],&userip,&usermask);	
	if(CMD_WARNING == ret){
		vty_out(vty, "%% invalid ip address\n");
		return CMD_FAILURE;
	}
	PDC_VALUE_IP_MASK_CHECK(usermask);
	usermask = mask2binary(usermask);

	ret = sscanf(argv[1],"%d-%d", &eag_slotid, &eag_insid );
	if( ret != 2 ){
		vty_out(vty, "the PARAM should format like  1-3(slotid-insid)!\n");
		return CMD_FAILURE;
	}

	if( eag_slotid > 10 || eag_insid > 16 ||
		eag_slotid < 0 || eag_insid < 0){
		vty_out(vty, "Slot id should less than 10 and insid shold less than 16\n");		
		return CMD_FAILURE;
	}

	PDC_DCLI_INIT_HANSI_INFO
	
	ret = pdc_intf_modify_map( dcli_dbus_connection_curr,
						hansitype, insid,
						userip, usermask, eag_slotid, eag_insid);
	
	if (EAG_RETURN_OK == ret) {
		return CMD_SUCCESS;
	}
	else if (EAG_ERR_PDC_MAP_CONFLICT == ret){
		vty_out(vty,"%% pdc map conflict\n");
	}
	else if (EAG_ERR_PDC_MAP_NOT_FOUND == ret){
		vty_out(vty,"%% pdc map not found\n");
	}
	else if (EAG_ERR_PDC_MAP_NUM_OVERSTEP == ret){
		vty_out(vty,"%% pdc map over limit\n");
	}
	else if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}

	return CMD_FAILURE;
	
}


DEFUN(pdc_show_map_func,
	pdc_show_map_cmd,
	"show pdc-map-config",
	SHOW_STR
	"show pdc map config\n"
)
{
	int ret = 0;
	int i = 0;
	char ipstr[32] = "";
	struct pdc_map_conf map_conf = {0};

	PDC_DCLI_INIT_HANSI_INFO

	ret = pdc_intf_show_maps( dcli_dbus_connection_curr,
						hansitype, insid,
						&map_conf);
	if (EAG_RETURN_OK == ret) {
		
		if (0 == map_conf.num){
			vty_out(vty, "no configuration!\n");
			return CMD_SUCCESS;
		}
		vty_out(vty, "==============================\n");
		for (i=0; i<map_conf.num; i++){
			ip2str(map_conf.map[i].userip, ipstr, sizeof(ipstr));
			vty_out(vty, "%-15s : %s\n", "user ip", ipstr);
			ip2str(map_conf.map[i].usermask, ipstr, sizeof(ipstr));
			vty_out(vty, "%-15s : %s\n", "user mask", ipstr);
			vty_out(vty, "%-15s : %d\n", "hansi slotid", map_conf.map[i].eag_slotid);
			vty_out(vty, "%-15s : %s\n", "hansi hansitype", (map_conf.map[i].eag_hansitype)?"local":"remote");
			vty_out(vty, "%-15s : %d\n", "hansi hansiid", map_conf.map[i].eag_hansiid);
			ip2str(map_conf.map[i].eag_ip, ipstr, sizeof(ipstr));
			vty_out(vty, "%-15s : %s\n", "eag ip", ipstr);
			vty_out(vty, "%-15s : %u\n", "eag portal port", map_conf.map[i].eag_port);
			vty_out(vty, "==============================\n");
		}
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

DEFUN(set_pdc_portal_protocol_func,
	set_pdc_portal_protocol_cmd,
	"set portal-protocol (mobile|telecom)",
	SETT_STR
	"set portal-protocol type\n"
	"set portal-protocol ChinaMobile\n"
	"set portal-protocol ChinaTelecom\n"
)
{	
	int portal_protocol = PORTAL_PROTOCOL_MOBILE;
	int  ret = -1;

	if (strncmp(argv[0], "mobile", strlen(argv[0])) == 0) {
		portal_protocol = PORTAL_PROTOCOL_MOBILE;
	}
	else if (strncmp(argv[0], "telecom", strlen(argv[0])) == 0) {
		portal_protocol = PORTAL_PROTOCOL_TELECOM;
	}
	else {
		vty_out(vty, "input param error\n");
		return CMD_SUCCESS;
	}

	PDC_DCLI_INIT_HANSI_INFO
	
	ret = pdc_intf_set_portal_protocol( dcli_dbus_connection_curr,
								hansitype, insid,
								portal_protocol);
	if (EAG_RETURN_OK == ret) {
		return CMD_SUCCESS;
	}
	else if (EAG_ERR_DBUS_FAILED == ret) {
			vty_out(vty, "%% dbus error\n");
	}
	else {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}

	return CMD_SUCCESS;
}

DEFUN(pdc_add_debug_filter_func,
	pdc_add_debug_filter_cmd,
	"add debug-filter FILTER",
	"add\n"
	"pdc debug filter string\n"
	"pdc debug filter string\n"
)
{
	int ret = 0;

	PDC_DCLI_INIT_HANSI_INFO

	ret = pdc_add_debug_filter( dcli_dbus_connection_curr,
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

DEFUN(pdc_log_all_blkmem_func,
	pdc_log_all_blkmem_cmd,
	"log all-blkmem",
	"do log\n"
	"log all blkmem\n"
)
{
	vty_out(vty, "111 pdc_log_all_blkmem_func log all-blkmem\n");
	#if 1
	int ret = 0;

	PDC_DCLI_INIT_HANSI_INFO

	ret = pdc_log_all_blkmem( dcli_dbus_connection_curr,
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
	#endif
	return CMD_FAILURE;
}

DEFUN(pdc_log_all_thread_func,
	pdc_log_all_thread_cmd,
	"log all-thread",
	"do log\n"
	"log all thread\n"
)
{
	#if 1
	int ret = 0;

	PDC_DCLI_INIT_HANSI_INFO

	#if 1
	ret = pdc_log_all_thread( dcli_dbus_connection_curr,
						hansitype, insid);
    #endif
	if (EAG_RETURN_OK == ret) {
		return CMD_SUCCESS;
	}
	else if (EAG_ERR_DBUS_FAILED == ret) {
			vty_out(vty, "%% dbus error\n");
	}
	else {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}
	#endif
	return CMD_FAILURE;
}


DEFUN(pdc_del_debug_filter_func,
	pdc_del_debug_filter_cmd,
	"delete debug-filter FILTER",
	"delete\n"
	"pdc debug filter string\n"
	"pdc debug filter string\n"
)
{
	int ret = 0;

	PDC_DCLI_INIT_HANSI_INFO

	ret = pdc_del_debug_filter( dcli_dbus_connection_curr,
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



DEFUN(pdc_set_userconn_func,
	pdc_set_userconn_cmd,
	"set user-hansi A.B.C.D hansi PARAM",
	"set user-hansi \n"
	"set user-hansi \n"
	"user ip addr\n"
	"user slot id\n"
	"hansi id\n"	
)
{
	int ret = 0;
	uint32_t userip=0;
	uint32_t eag_slot_id=0;
	uint32_t eag_hansi_type=0;
	uint32_t eag_hansi_id=0;
	struct in_addr addr = {0};

	PDC_DCLI_INIT_HANSI_INFO
	
	ret = inet_aton(argv[0], &addr);
	if (!ret) {
		vty_out(vty, "%% invalid ip address\n");
		return CMD_WARNING;
	}
	userip = ntohl(addr.s_addr);
	
	ret = sscanf(argv[1],"%d-%d", &eag_slot_id, &eag_hansi_id );
	if( ret != 2 ){
		vty_out(vty, "the PARAM should format like  1-3(slotid-insid)!\n");
		return CMD_FAILURE;
	}
	
	eag_hansi_type = hansitype;
	ret = pdc_set_userconn( dcli_dbus_connection_curr,
						hansitype, insid,
						userip,
						(uint8_t)eag_slot_id,
						(uint8_t)eag_hansi_type,
						(uint8_t)eag_hansi_id );

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


DEFUN(pdc_log_all_userconn_func,
	pdc_log_all_userconn_cmd,
	"log all-userconn",
	"log\n"
	"log all-userconn\n"
)
{
	int ret = 0;

	PDC_DCLI_INIT_HANSI_INFO

	ret = pdc_log_userconn( dcli_dbus_connection_curr,
						hansitype, insid );

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


int pdc_has_config(void)
{
	int ret = 0;
	struct pdc_base_conf baseconf = {0};
	struct pdc_map_conf map_conf = {0};

	ret = pdc_intf_get_base_conf(dcli_dbus_connection, HANSI_LOCAL, 
					0, &baseconf);
	if (EAG_RETURN_OK == ret) {
		if (0 != baseconf.nasip) {
			return 1;
		}
		if (PDC_DEFAULT_PORTAL_PORT != baseconf.port) {
			return 1;
		}
		if (PORTAL_PROTOCOL_MOBILE != baseconf.portal_protocol) {
			return 1;
		}
		if (0 != baseconf.status) {
			return 1;
		}
	}

	ret = pdc_intf_show_maps(dcli_dbus_connection,	HANSI_LOCAL, 0,
						&map_conf);
	if (EAG_RETURN_OK == ret) {
		if (map_conf.num > 0) {
			return 1;
		}
	}

	return 0;
}

int dcli_pdc_show_running_config(struct vty* vty)
{
	int ret = 0;
	char showStr[256] = "";
	char ipstr[32] = "";
	char userip[32] = "";
	unsigned int usermask = 0;
	char eag_ip[32] = "";
	struct pdc_base_conf baseconf = {0};
	struct pdc_map_conf map_conf = {0};
	int i = 0;

	snprintf(showStr, sizeof(showStr), BUILDING_MOUDLE, "PDC");
	vtysh_add_show_string(showStr);

	if (!pdc_has_config()) {
		return CMD_SUCCESS;
	}
	
	vtysh_add_show_string("config pdc");
	
	ret = pdc_intf_get_base_conf( dcli_dbus_connection,
						HANSI_LOCAL, 0,
						&baseconf);
	if (EAG_RETURN_OK == ret) {
		if (0 != baseconf.nasip) {
			ip2str(baseconf.nasip, ipstr, sizeof(ipstr));
			snprintf(showStr, sizeof(showStr), " set nasip %s",ipstr);
			vtysh_add_show_string(showStr);
		}
		if (PDC_DEFAULT_PORTAL_PORT != baseconf.port) {
			snprintf(showStr, sizeof(showStr), " set portal-port %u",
						baseconf.port);
			vtysh_add_show_string(showStr);
		}
		if (PORTAL_PROTOCOL_TELECOM == baseconf.portal_protocol) {
			snprintf(showStr, sizeof(showStr), " set portal-protocol telecom");
			vtysh_add_show_string(showStr);
		}
	}

	ret = pdc_intf_show_maps( dcli_dbus_connection,
						HANSI_LOCAL, 0,
						&map_conf);
	
	if (EAG_RETURN_OK == ret) {
		for (i=0; i<map_conf.num; i++){
			ip2str(map_conf.map[i].userip, userip, sizeof(userip));
			//ip2str(map_conf.map[i].usermask, usermask, sizeof(usermask));
			ip2str(map_conf.map[i].eag_ip, eag_ip, sizeof(eag_ip));	
			usermask = binary2mask(map_conf.map[i].usermask);
			snprintf(showStr, sizeof(showStr), " add pdc-map user-subnet %s/%d hansi %d-%d\n",
							userip, usermask, map_conf.map[i].eag_slotid,\
							map_conf.map[i].eag_hansiid);
			vtysh_add_show_string(showStr);
		}
	}

	if (0 != baseconf.status) {
		snprintf(showStr, sizeof(showStr), " service pdc enable");
		vtysh_add_show_string(showStr);
	}

	vtysh_add_show_string(" exit");
		
	return CMD_SUCCESS;
}

char *dcli_pdc_show_running_config_2(int localid, int slot_id,int index)
{
	char *tmp = NULL;
	int ret = -1;
	int i =0;
	char showStr[SHOW_STR_LEN] = "";
	char *cursor = NULL;
	int totalLen = 0;
	DBusConnection *dcli_dbus_connection_curr = NULL;
	char ipstr[32] = "";
	char userip[32] = "";
	unsigned int usermask = 0;
	char eag_ip[32] = "";
	int tmplen = 0;
	struct pdc_base_conf baseconf = {0};
	struct pdc_map_conf map_conf = {0};
	
	memset (showStr, 0, sizeof(showStr));
	cursor = showStr;
	totalLen = 0;

	totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, 
					"config pdc\n");
	tmplen = totalLen;

	ReInitDbusConnection(&dcli_dbus_connection_curr,slot_id,distributFag);
	ret = pdc_intf_get_base_conf( dcli_dbus_connection_curr,
						localid, index, &baseconf);
	if (EAG_RETURN_OK == ret) {
		if (0 != baseconf.nasip) {
			ip2str(baseconf.nasip, ipstr, sizeof(ipstr));
			totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, 
						" set nasip %s\n",ipstr);
		}
		if (PDC_DEFAULT_PORTAL_PORT != baseconf.port) {
			totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, 
							" set portal-port %u\n", baseconf.port);
		}
		if (PORTAL_PROTOCOL_TELECOM == baseconf.portal_protocol) {
			totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, 
							" set portal-protocol telecom\n");
		}
	}

	ret = pdc_intf_show_maps( dcli_dbus_connection_curr,
						localid, index, &map_conf);
	
	if (EAG_RETURN_OK == ret) {
		for (i=0; i<map_conf.num; i++){
			ip2str(map_conf.map[i].userip, userip, sizeof(userip));
			//ip2str(map_conf.map[i].usermask, usermask, sizeof(usermask));	
			usermask = binary2mask(map_conf.map[i].usermask);
			totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, 
							" add pdc-map user-subnet %s/%d hansi %d-%d\n",
							userip, usermask, map_conf.map[i].eag_slotid,\
							map_conf.map[i].eag_hansiid);
		}
	}

	if (0 != baseconf.status) {
		totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, 
					" service pdc enable\n");
	}

	/* pdc has no config */
	if (totalLen == tmplen) {
		tmp = (char*)malloc(1);
		if (NULL == tmp){
			return NULL;
		}
		memset(tmp, 0 , 1);
		return tmp;
	}
	/* pdc has config */
	totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " exit\n");
	
	tmp = malloc(strlen(showStr)+1);
	if (NULL == tmp){
		return NULL;
	}
	memset (tmp, 0, strlen(showStr)+1);
	memcpy(tmp, showStr, strlen(showStr));
	return tmp;	
}

void dcli_pdc_init(void)
{
	install_node(&pdc_node, dcli_pdc_show_running_config,"pdc");
	install_default(PDC_NODE);
	install_element(CONFIG_NODE, &config_pdc_cmd);

	install_node(&hansi_pdc_node, NULL,"pdc-hansi");
	install_default(HANSI_PDC_NODE);
	install_element(HANSI_NODE, &config_pdc_cmd);
		
	install_node(&local_hansi_pdc_node, NULL,"pdc-local-hansi");
	install_default(LOCAL_HANSI_PDC_NODE);
	install_element(LOCAL_HANSI_NODE, &config_pdc_cmd);

	/* pdc */
	install_element(PDC_NODE, &pdc_set_nasip_cmd);
	install_element(HANSI_PDC_NODE, &pdc_set_nasip_cmd);
	install_element(LOCAL_HANSI_PDC_NODE, &pdc_set_nasip_cmd);
	
	install_element(PDC_NODE, &pdc_set_portal_port_cmd);
	install_element(HANSI_PDC_NODE, &pdc_set_portal_port_cmd);
	install_element(LOCAL_HANSI_PDC_NODE, &pdc_set_portal_port_cmd);
	
	install_element(PDC_NODE, &pdc_set_status_cmd);
	install_element(HANSI_PDC_NODE, &pdc_set_status_cmd);
	install_element(LOCAL_HANSI_PDC_NODE, &pdc_set_status_cmd);
	
	install_element(PDC_NODE, &pdc_show_base_conf_cmd);
	install_element(HANSI_PDC_NODE, &pdc_show_base_conf_cmd);
	install_element(LOCAL_HANSI_PDC_NODE, &pdc_show_base_conf_cmd);
	
	install_element(PDC_NODE, &pdc_add_map_cmd);
	install_element(HANSI_PDC_NODE, &pdc_add_map_cmd);
	install_element(LOCAL_HANSI_PDC_NODE, &pdc_add_map_cmd);
	
	install_element(PDC_NODE, &pdc_del_map_cmd);
	install_element(HANSI_PDC_NODE, &pdc_del_map_cmd);
	install_element(LOCAL_HANSI_PDC_NODE, &pdc_del_map_cmd);
	
	install_element(PDC_NODE, &pdc_modify_map_cmd);
	install_element(HANSI_PDC_NODE, &pdc_modify_map_cmd);
	install_element(LOCAL_HANSI_PDC_NODE, &pdc_modify_map_cmd);
	
	install_element(PDC_NODE, &pdc_show_map_cmd);
	install_element(HANSI_PDC_NODE, &pdc_show_map_cmd);
	install_element(LOCAL_HANSI_PDC_NODE, &pdc_show_map_cmd);

	install_element(PDC_NODE, &set_pdc_portal_protocol_cmd);
	install_element(HANSI_PDC_NODE, &set_pdc_portal_protocol_cmd);
	install_element(LOCAL_HANSI_PDC_NODE, &set_pdc_portal_protocol_cmd);
	
	install_element(PDC_NODE, &pdc_add_debug_filter_cmd);
	install_element(HANSI_PDC_NODE, &pdc_add_debug_filter_cmd);
	install_element(LOCAL_HANSI_PDC_NODE, &pdc_add_debug_filter_cmd);
	
	install_element(PDC_NODE, &pdc_del_debug_filter_cmd);
	install_element(HANSI_PDC_NODE, &pdc_del_debug_filter_cmd);
	install_element(LOCAL_HANSI_PDC_NODE, &pdc_del_debug_filter_cmd);

	install_element(PDC_NODE, &pdc_log_all_blkmem_cmd);
	install_element(HANSI_PDC_NODE, &pdc_log_all_blkmem_cmd);
	install_element(LOCAL_HANSI_PDC_NODE, &pdc_log_all_blkmem_cmd);
	
	install_element(PDC_NODE, &pdc_log_all_thread_cmd);
	install_element(HANSI_PDC_NODE, &pdc_log_all_thread_cmd);
	install_element(LOCAL_HANSI_PDC_NODE, &pdc_log_all_thread_cmd);

	install_element(PDC_NODE, &pdc_set_userconn_cmd);
	install_element(HANSI_PDC_NODE, &pdc_set_userconn_cmd);
	install_element(LOCAL_HANSI_PDC_NODE, &pdc_set_userconn_cmd);

	install_element(PDC_NODE, &pdc_log_all_userconn_cmd);
	install_element(HANSI_PDC_NODE, &pdc_log_all_userconn_cmd);
	install_element(LOCAL_HANSI_PDC_NODE, &pdc_log_all_userconn_cmd);
}

