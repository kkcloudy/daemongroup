#include <string.h>
#include <dbus/dbus.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <zebra.h>
#include "command.h"
#include <dirent.h>
#include "../../dcli/src/lib/dcli_main.h"
#include "mapi_sigtran2udp.h"
#include "dcli_sigtran2udp.h"
#include "dbus/iu/SigtranDBusPath.h"

#define MAX_STR_LEN			(1024*1024)
#if 0
struct cmd_node sigtran_node = 
{
	SIGTRANUDP_NODE,
	"%s(config-sigtran)# ",
	1
};
struct cmd_node hansi_sigtran_node =
{
	HANSI_SIGTRANUDP_NODE,
	"%s(hansi-sigtran %d-%d)# ",
};

/**********************************************************************************
 *  iu_ip2ulong
 *
 *	DESCRIPTION:
 * 		convert IP (A.B.C.D) to IP (ABCD) pattern
 *
 *	INPUT:
 *		str - (A.B.C.D)
 *	
 *	OUTPUT:
 *		null
 *
 * 	RETURN:
 *		
 *		IP	-  ip (ABCD)
 *		
 **********************************************************************************/

static unsigned long iu_ip2ulong(char *str)
{
	char *sep=".";
	char *token = NULL;
	unsigned long ip_long[4]; 
	unsigned long ip = 0;
	int i = 1;

	token=strtok(str,sep);
	if(NULL != token){
		ip_long[0] = strtoul(token,NULL,10);
	}
	while((token!=NULL)&&(i<4))
	{
		token=strtok(NULL,sep);
		if(NULL != token){
			ip_long[i] = strtoul(token,NULL,10);
		}
		i++;
	}

	ip=(ip_long[0]<<24)+(ip_long[1]<<16)+(ip_long[2]<<8)+ip_long[3];

	return ip;
}

DEFUN(config_sigtran2udp_mode_cmd_func,
		config_sigtran2udp_mode_cmd,
		"config sigtran2udp",
		CONFIG_STR
		"sigtran udp proess\n"
     )
{
	if(vty->node == CONFIG_NODE)
	{
		vty->node = SIGTRANUDP_NODE;
	}
	else if(vty->node == HANSI_NODE)
	{
		vty->node = HANSI_SIGTRANUDP_NODE;
	}
	else if(vty->node == LOCAL_HANSI_NODE)
	{
		vty->node = LOCAL_HANSI_SIGTRANUDP_NODE;
	}

	return CMD_IU_SUCCESS;

}



DEFUN(set_sigtran2udp_point_code_func,
		set_sigtran2udp_point_code_cmd,
		"set sigtran address A.B.C.D port <1-65535> point_code <1-4294967295>",
		"iu configrion\n"
		"homegateway\n"
		"ipv4 address\n"
		"sctp port\n"		  
		"point_code\n"	  
     )
{
	int ret = 0;	
	unsigned int my_ip = 0, self_pd = 0;
	unsigned short my_port = 0 ;
	ret = inet_pton (AF_INET, (char*)argv[0], &my_ip);
	if (ret != 1) {
		vty_out (vty, "malformed ip address : %s\n", argv[0]);
		return CMD_IU_FAILURE;
	}
	
	my_port = atoi(argv[1]);		
	self_pd = atoi(argv[2]);
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	if(vty->node == SIGTRANUDP_NODE){
		index = 0;
	}else if(vty->node == HANSI_SIGTRANUDP_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if(vty->node == LOCAL_HANSI_SIGTRANUDP_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dbus_connection = NULL;
	ReInitFemtoDbusConnection(&dbus_connection, slot_id, distributFag);
	ret = sigtran_set_self_point_code(index, localid, my_ip, my_port , self_pd, dbus_connection, UDP_IU_SET_SELF_POINT_CODE);
	if(ret)
	{
		vty_out(vty,"set self point_code fail\n");
	}	
	else	
		return CMD_IU_SUCCESS;		

	return CMD_IU_FAILURE;
}


DEFUN(set_sigtran2udp_func,
		set_sigtran2udp_cmd,
		"set homegateway address A.B.C.D port <1-65535> point-code <1-4294967295> connection-mode (client|server)",
		"ipv4 address\n"
		"sctp port\n"	
		"ss7 point_code\n"
		"connection mode\n"
		"client mode\n"
		"server mode\n"
     )
{
	int ret = 0;
	unsigned int msc_pd = 0;
	int msc_mode = 0;
	unsigned int msc_ip = NULL;
	unsigned short sctp_port = 0;
	ret = inet_pton (AF_INET, (char*)argv[0], &msc_ip);
	if (ret != 1) {
		vty_out (vty, "malformed ip address : %s\n", argv[0]);
		return CMD_IU_FAILURE;
	}
	
	sctp_port = atoi(argv[1]);	
	msc_pd = atoi(argv[2]);

	if(!strcmp("server", argv[3])){
		msc_mode = 1;		
	}else{
		msc_mode = 0;
	}	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	if(vty->node == SIGTRANUDP_NODE){
		index = 0;
	}else if(vty->node == HANSI_SIGTRANUDP_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if(vty->node == LOCAL_HANSI_SIGTRANUDP_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dbus_connection = NULL;
	ReInitFemtoDbusConnection(&dbus_connection, slot_id, distributFag);
	ret = sigtran_set_msc(index, localid, msc_ip, sctp_port, msc_pd, msc_mode, dbus_connection, UDP_IU_SET_MSC);
	if(ret)
	{
		vty_out(vty,"set self point_code fail\n");
	}	
	else

		return CMD_IU_SUCCESS;	

	return CMD_IU_FAILURE;
}


DEFUN(set_cn_func,
		set_cn_cmd,
		"set cn address A.B.C.D port <1-65535>",
		"ipv4 address\n"
		"sctp port\n"	
		"ss7 point_code\n"
		"connection mode\n"
		"client mode\n"
		"server mode\n"
     )
{
	int ret = 0;
	unsigned int msc_pd = 0;
	int msc_mode = 0;
	unsigned int msc_ip = NULL;
	unsigned short sctp_port = 0;	
	ret = inet_pton (AF_INET, (char*)argv[0], &msc_ip);
	if (ret != 1) {
		vty_out (vty, "malformed ip address : %s\n", argv[0]);
		return CMD_IU_FAILURE;
	}
	
	sctp_port = atoi(argv[1]);	
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	if(vty->node == SIGTRANUDP_NODE){
		index = 0;
	}else if(vty->node == HANSI_SIGTRANUDP_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if(vty->node == LOCAL_HANSI_SIGTRANUDP_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dbus_connection = NULL;
	ReInitFemtoDbusConnection(&dbus_connection, slot_id, distributFag);
	ret = sigtran_set_cn(index, localid, msc_ip, sctp_port, dbus_connection, UDP_IU_SET_SGSN);
	if(ret)
	{
		vty_out(vty,"set self point_code fail\n");
	}	
	else

		return CMD_IU_SUCCESS;	

	return CMD_IU_FAILURE;
}



DEFUN(sigtran2udp_debug_enable_cmd_func,
		sigtran2udp_debug_enable_cmd,
		"debug sigtran2udp (all|info|error|debug)",
		"Add debug iu Information\n"
		"Open iu debug level all\n"	
		"Open iu debug level info\n"
		"Open iu debug level error\n"
		"Open iu debug level debug\n"
     )
{
	unsigned int ret = 0, debug_type = 0, debug_enable = 1;
	unsigned int maxParaLen = 5;

	if(strncmp("all",argv[0],strnlen(argv[0],maxParaLen))==0) {
		debug_type = DEBUG_TYPE_ALL;
	}
	else if (strncmp("info",argv[0],strnlen(argv[0],maxParaLen))==0) {
		debug_type = DEBUG_TYPE_INFO;
	}
	else if (strncmp("error",argv[0],strnlen(argv[0],maxParaLen))==0) {
		debug_type = DEBUG_TYPE_ERROR;
	}
	else if (strncmp("debug",argv[0],strnlen(argv[0],maxParaLen))==0) {
		debug_type = DEBUG_TYPE_DEBUG;
	}
	else {
		vty_out(vty,"bad command parameter %s\n", argv[0]);
		return CMD_IU_FAILURE;
	}
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	if(vty->node == SIGTRANUDP_NODE){
		index = 0;
	}else if(vty->node == HANSI_SIGTRANUDP_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if(vty->node == LOCAL_HANSI_SIGTRANUDP_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dbus_connection = NULL;
	ReInitFemtoDbusConnection(&dbus_connection, slot_id, distributFag);
	ret = sigtran_set_debug_state(index, localid, debug_type, debug_enable, dbus_connection, UDP_IU_DBUS_METHOD_SET_DEBUG_STATE);

	if (!ret) {
		return CMD_IU_SUCCESS;
	}
	else {
		return CMD_IU_FAILURE;
	}
}

DEFUN(sigtran2udp_debug_disable_cmd_func,
		sigtran2udp_debug_disable_cmd,
		"no debug sigtran2udp (all|info|error|debug)",
		"Delete old Configuration\n"
		"Config iu debugging close\n"
		"Dhcp server\n"
		"Close iu debug level all\n"	
		"Close iu debug level info\n"
		"Close iu debug level error\n"
		"Close iu debug level debug\n"

     )
{
	unsigned int ret = 0, debug_type = 0, isEnable = 0;
	unsigned int maxParaLen = 5;

	if(strncmp("all",argv[0],strnlen(argv[0],maxParaLen))==0) {
		debug_type = DEBUG_TYPE_ALL;
	}
	else if (strncmp("info",argv[0],strnlen(argv[0],maxParaLen))==0) {
		debug_type = DEBUG_TYPE_INFO;
	}
	else if (strncmp("error",argv[0],strnlen(argv[0],maxParaLen))==0) {
		debug_type = DEBUG_TYPE_ERROR;
	}
	else if (strncmp("debug",argv[0],strnlen(argv[0],maxParaLen))==0) {
		debug_type = DEBUG_TYPE_DEBUG;
	}
	else {
		vty_out(vty,"bad command parameter %s\n", argv[0]);
		return CMD_IU_FAILURE;
	}
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	if(vty->node == SIGTRANUDP_NODE){
		index = 0;
	}else if(vty->node == HANSI_SIGTRANUDP_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if(vty->node == LOCAL_HANSI_SIGTRANUDP_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dbus_connection = NULL;
	ReInitFemtoDbusConnection(&dbus_connection, slot_id, distributFag);
	ret = sigtran_set_debug_state(index, localid, debug_type, isEnable, dbus_connection, UDP_IU_DBUS_METHOD_SET_DEBUG_STATE);

	if (!ret) {
		return CMD_IU_SUCCESS;
	}
	else {
		return CMD_IU_FAILURE;
	}
}


DEFUN(show_sigtran2udp_running_config_cmd,
		show_sigtran2udp_running_config,
		"show sigtran2udp running config",	
		"Config iu debugging close\n"
		"Dynamic Host Configuration Protocol\n"
		"running config\n"	
		"iu Configuration\n"
     )
{
	char showStr[MAX_STR_LEN] = {0};
	int ret = 1;
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	if(vty->node == SIGTRANUDP_NODE){
		index = 0;
	}else if(vty->node == HANSI_SIGTRANUDP_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if(vty->node == LOCAL_HANSI_SIGTRANUDP_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dbus_connection = NULL;
	ReInitFemtoDbusConnection(&dbus_connection, slot_id, distributFag);
	if(!sigtran_show_running_cfg_lib(index, localid, showStr, dbus_connection, UDP_IU_DBUS_METHOD_SHOW_RUNNING_CFG)){
		ret = 0;
		vty_out(vty,"%s", showStr);
	}

	return ret;	
}

DEFUN(sigtran2udp_enable_cmd_func,
	sigtran2udp_enable_cmd,
	"sigtran2udp service (enable|disable)",
	CONFIG_STR
	"Ip dhcp server (enable|disable) entity\n"
	"Specify pool name begins with char, and name length no more than 20 characters\n"
)
{
	unsigned int ret = 0, op_ret = 0, isEnable = 0;

	if(strncmp("enable",argv[0],strnlen(argv[0],7))==0) {
		isEnable = 1;
	}
	else if (strncmp("disable",argv[0],strnlen(argv[0],7))==0) {
		isEnable = 0;
	}
	else {
		vty_out(vty,"bad command parameter!\n");
		return CMD_IU_FAILURE;
	}
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	if(vty->node == SIGTRANUDP_NODE){
		index = 0;
	}else if(vty->node == HANSI_SIGTRANUDP_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if(vty->node == LOCAL_HANSI_SIGTRANUDP_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dbus_connection = NULL;
	ReInitFemtoDbusConnection(&dbus_connection, slot_id, distributFag);
	ret = set_sigtran2udp_enable(index, localid, isEnable, dbus_connection, UDP_IU_DBUS_METHOD_SET_SIGTRAN_ENABLE);
	
	if (!ret) {
		return CMD_IU_SUCCESS;
	}
	else {
		return CMD_IU_FAILURE;
	}
}

void dcli_sigtran2udp_init(void) {
	install_node (&hansi_sigtran_node,NULL,"HANSI_SIGTRANUDP_NODE");
	install_default(HANSI_SIGTRANUDP_NODE);
	install_element(HANSI_NODE,&config_sigtran2udp_mode_cmd);
	/****************HANSI SIGTRANUDP NODE**********************************/
	install_element(HANSI_SIGTRANUDP_NODE,&set_sigtran2udp_point_code_cmd);
	install_element(HANSI_SIGTRANUDP_NODE,&set_sigtran2udp_cmd);
	install_element(HANSI_SIGTRANUDP_NODE,&sigtran2udp_debug_enable_cmd);
	install_element(HANSI_SIGTRANUDP_NODE,&sigtran2udp_debug_disable_cmd);
	install_element(HANSI_SIGTRANUDP_NODE,&show_sigtran2udp_running_config);
	install_element(HANSI_SIGTRANUDP_NODE,&sigtran2udp_enable_cmd);
	install_element(HANSI_SIGTRANUDP_NODE,&set_cn_cmd);

	/****************LOCAL HANSI SIGTRANUDP NODE****************************/

	return;
}
#endif
