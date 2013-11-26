/*******************************************************************************
Copyright (C) Autelan Technology


This software file is owned and distributed by Autelan Technology 
********************************************************************************


THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR 
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************************
* dcli_eag.c
*
*
* CREATOR:
* liuyu@autelan.com
*
* DESCRIPTION:
* CLI definition for eag module.
*
*
*******************************************************************************/

#include <zebra.h>
#include <dbus/dbus.h>
#include <stdlib.h>
#include <sysdef/npd_sysdef.h>
#include <dbus/npd/npd_dbus_def.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpathInternals.h>
#include <ctype.h>

#include "dcli_system.h"
//#include "vty.h"
#include "command.h"
#include "if.h"
#include "sysdef/returncode.h"

//#include "ws_eag_conf.h"
#include "ws_user_manage.h"
#include "user_manage.h"
#include "dcli_eag.h"
#include "ws_conf_engine.h"
#include "dcli_captive.h" 
#include "dcli_domain.h"
#include "drp_def.h"
#include "drp_interface.h"

/*eag 2.0*/
#include "eag_conf.h"
#include "eag_interface.h"
#include "nm_list.h"
#include "eag_errcode.h"

#include "bsd_bsd.h"
#include "ws_dbus_list.h"
#include "ac_manage_def.h"
#include "ac_manage_extend_interface.h"
#include <string.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include "dcli_main.h"

#define SHOW_STR_LEN 1024*5
#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]

static char *
ip2str(uint32_t ip, char *str, size_t size)
{
	if (NULL == str) {
		return NULL;
	}

	memset(str, 0, size);
	snprintf(str, size-1, "%u.%u.%u.%u",
		(ip>>24)&0xff, (ip>>16)&0xff, (ip>>8)&0xff, ip&0xff);

	return str;
}

static char *
ipv6tostr(struct in6_addr *ipv6, char *str, size_t size)
{
	if (NULL == str || NULL == ipv6) {
		return NULL;
	}
	
	memset(str, 0, size);
	if(!inet_ntop(AF_INET6, (const void *)ipv6, str, size)) {
		return "[ipv6 error]";
	}

	return str;
}

static int
ipv6_compare_null(struct in6_addr *ipv6)
{
	unsigned char cmp[16] = "";
	memset(cmp, 0, sizeof(cmp));

	return memcmp(ipv6, cmp, sizeof(struct in6_addr));
}

static char *
mac2str(const uint8_t mac[6], char *str, size_t size, char separator)
{
	if (NULL == mac || NULL == str || size <= 0) {
		return NULL;
	}
	if (':' != separator && '-' != separator && '_' != separator) {
		separator = ':';
	}

	memset(str, 0, size);
	snprintf(str, size-1, "%02X%c%02X%c%02X%c%02X%c%02X%c%02X",
		mac[0], separator, mac[1], separator, mac[2], separator,
		mac[3], separator, mac[4], separator, mac[5]);

	return str;
}

static int
str2mac(const char *str, uint8_t mac[6])
{
	char separator = ':';
	int num = 0;
	
	if (NULL == str || NULL == mac || strlen(str) < 17) {
		return -1;
	}

	separator = str[2];
	switch (separator) {
	case ':':
		num = sscanf(str, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
			&mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
		break;
	case '-':
		num = sscanf(str, "%hhx-%hhx-%hhx-%hhx-%hhx-%hhx",
			&mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
		break;
	case '_':
		num = sscanf(str, "%hhx_%hhx_%hhx_%hhx_%hhx_%hhx",
			&mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
		break;
	default:
		break;
	}
	
	return (6 == num) ? 0 : -1;
}

static int
is_mask(uint32_t mask)
{
	int i = 0;
	
	for (i = 0; i < 32; i++) {
		if ((mask>>i) & 0X1) {
			break;
		}
	}
	for (; i < 32; i++) {
		if (!((mask>>i) & 0X1)) {
			return 0;
		}
	}

	return 1;
}

static int
is_number(const char *str)
{
	if (NULL == str) {
		return 0;
	}
	while ('\0' != *str) {
		if (*str < '0' || *str > '9') {
			return 0;
		}
		str++;
	}
	return 1;
}

static int
is_domain(const char *str)
{
	if (NULL == str) {
		return 0;
	}
	int i = 0;
	for (i = 0; str[i] != 0; i++) {
		if (0 == isalnum(str[i])
		&& '.' != str[i]
		&& '-' != str[i]) {
			return 0;
		}
	}
	if (str[0] == '-' || str[i-1] == '-') {
		return 0;
	}
	return 1;
}

struct cmd_node eag_node = 
{
	EAG_NODE,
	"%s(config-eag)# "
};

struct cmd_node eag_eagins_node = 
{
	EAG_INS_NODE,
	"%s(config-eag-eagins)# "
};

struct cmd_node eag_nasid_node = 
{
	EAG_NASID_NODE,
	"%s(config-eag-nasid)# "
};

struct cmd_node eag_radius_node = 
{
	EAG_RADIUS_NODE,
	"%s(config-eag-radius)# "
};

struct cmd_node eag_portal_node = 
{
	EAG_PORTAL_NODE,
	"%s(config-eag-portal)# "
};

struct cmd_node eag_vlanmap_node = 
{
	EAG_VLANMAP_NODE,
	"%s(config-eag-vlanmap)# "
};

#define SYSLOG_FILE_PATH	"/var/log/syslogservice.log"
#define SYSLOG_CLI_PATH	"/var/log/cli.log"

static int ip_input_is_legal(const char *ip_str)
{
	unsigned int ipaddr = 0;
	
	/* check ip address 0.X.X.X */
	if(1 == inet_pton(AF_INET, ip_str, &ipaddr)) {
		if (0 == (ipaddr & 0xff000000)) {
			//printf("Invalid ip address : %s(0.X.X.X)\n", ip_str);
			return -1;
		}
	} else {
		//printf("Invalid ip address %s\n", ip_str);
		return -1;
	}

	return 0;
}

#if 0
int ip_input_is_legal(const char *str)
{
	char *endptr = NULL;
	char* endptr1 = NULL;
	char c;
	int IP,i;
	
	c = str[0];
	if (c>='0'&&c<='9') {
		IP= strtoul(str,&endptr,10);
		if(IP < 0||IP > 255)
			return 0;
		else if(((IP < 10)&&((endptr - str) > 1))||((IP < 100)&&((endptr - str) > 2))||((IP < 256)&&((endptr - str) > 3)))
			return 0;

		for(i = 0; i < 3; i++) {
			if(endptr[0] == '\0'||endptr[0] != '.')
				return 0;
			else {
				endptr1 = &endptr[1];
				IP= strtoul(&endptr[1],&endptr,10); 			
				if(IP < 0||IP > 255)
					return 0;				
				else if(((IP < 10)&&((endptr - endptr1) > 1))||((IP < 100)&&((endptr - endptr1) > 2))||((IP < 256)&&((endptr - endptr1) > 3)))
					return 0;
			}
		}
		if(endptr[0] == '\0' && IP >= 0)
			return 1;
		else
			return 0;
	}
	else
	return 0;		
}
#endif


/*******************************************************************
 *	eag_show_syslog
 * 
 *	DESCRIPTION:
 *		This function will show syslog
 *
 *	INPUT:
 *		vty 				- for print log
 *		show_line_flag	- is show how many line in the syslog
 *		tail_num			- show the last line of syslog
 *		key_word			- show the syslog contain the keyword
 *	
 *	OUTPUT:		
 *		NULL
 *
 *	RETURN:
 *		CMD_FAILURE		- error
 *		CMD_SUCCESS	- successful
 *
 *********************************************************************/
int eag_show_syslog(struct vty* vty, int show_line_flag, char * tail_num, char * key_word)
{
	char file_path[] = SYSLOG_FILE_PATH;	
	char cmd[128] = "";
	char buff[1024] = "";
	
	FILE *p_file = NULL;
	int i = 0;
		
	memset(cmd, 0, sizeof(cmd));
	memset(buff, 0, sizeof(buff));

	if (TRUE_EAG_VTY == show_line_flag)
	{
		snprintf(cmd, sizeof(cmd), "cat %s|wc -l", file_path);		
	}
	else
	{
		if (NULL != tail_num)
		{
			snprintf(cmd, sizeof(cmd), "cat %s|tail -%s", file_path, tail_num);
		}
		else if(NULL != key_word)
		{
			snprintf(cmd, sizeof(cmd), "cat %s|grep %s", file_path, key_word);
		}
		else
		{
			snprintf(cmd, sizeof(cmd), "cat %s", file_path);
		}
	}
	
	p_file = popen(cmd,"r");

	if (NULL == p_file)
	{
		vty_out(vty, "Can not open file!\n");
		return CMD_FAILURE;
	}
	
	fgets(buff, sizeof(buff), p_file);
	
	while(buff[0] != '\0')
	{
		vty_out(vty,"%s",buff);
		
		memset(buff, 0, sizeof(buff));
		
		fgets(buff, sizeof(buff), p_file);		
	}

	pclose(p_file);
	
	return CMD_SUCCESS;
}


/*******************************************************************
 *	eag_show_syslog
 * 
 *	DESCRIPTION:
 *		This function will show syslog
 *
 *	INPUT:
 *	
 *	OUTPUT:		
 *		NULL
 *
 *	RETURN:
 *		CMD_FAILURE		- error
 *		CMD_SUCCESS	- successful
 *
 *********************************************************************/
int eag_show_syslog_cli(struct vty* vty)
{
	char file_path[] = SYSLOG_CLI_PATH;	
	char cmd[128] = "";
	char buff[1024] = "";
	
	FILE *p_file = NULL;
	int i = 0;
		
	memset(cmd, 0, sizeof(cmd));
	memset(buff, 0, sizeof(buff));

	snprintf(cmd, sizeof(cmd), "cat %s", file_path);
	
	p_file = popen(cmd,"r");

	if (NULL == p_file)
	{
		vty_out(vty, "Can not open file!\n");
		return CMD_FAILURE;
	}
	
	fgets(buff, sizeof(buff), p_file);
	
	while(buff[0] != '\0')
	{
		vty_out(vty,"%s",buff);
		
		memset(buff, 0, sizeof(buff));
		
		fgets(buff, sizeof(buff), p_file);		
	}

	pclose(p_file);
	
	return CMD_SUCCESS;
}

/*******************************************************************
 *	eag_show_syslog
 * 
 *	DESCRIPTION:
 *		This function will show syslog
 *
 *	INPUT:
 *		vty 				- for print log
 *	
 *	OUTPUT:		
 *		NULL
 *
 *	RETURN:
 *		CMD_FAILURE		- error
 *		CMD_SUCCESS	- successful
 *
 *********************************************************************/
int eag_show_syslog_time(struct vty* vty, int *time)
{
	char file_path[] = SYSLOG_FILE_PATH;	
	char cmd[128] = "";
	char buff[1024] = "";
	int get_time_part[3];
	int get_time;
	int start_time = 0;
	int end_time = 0;	
	FILE *p_file = NULL;
	int i = 0;

	if (NULL == time)
	{
		vty_out(vty, "error!no param!\n");
		return CMD_FAILURE;
	}
	
	start_time = time[0]*10000 + time[1]*100 + time[2];
	end_time = time[3]*10000 + time[4]*100 + time[5];	
		
	memset(cmd, 0, sizeof(cmd));
	memset(buff, 0, sizeof(buff));

	snprintf(cmd, sizeof(cmd), "cat %s", file_path);
	
	p_file = popen(cmd,"r");

	if (NULL == p_file)
	{
		vty_out(vty, "Can not open file!\n");
		return CMD_FAILURE;
	}
	
	fgets(buff, sizeof(buff), p_file);
	while(buff[0] != '\0')
	{
		memset(get_time_part, 0, sizeof(get_time_part));
		
		sscanf(buff,"%*[^ ] %*d %d:%d:%d", &(get_time_part[0]), &(get_time_part[1]), &(get_time_part[2]));

		get_time = get_time_part[0]*10000 + get_time_part[1]*100 + get_time_part[2];
		
		if (start_time > end_time)
		{
			if (start_time <= get_time || get_time <= end_time)
			{
				vty_out(vty,"%s",buff);
			}
		}
		else
		{
			if (start_time <= get_time && get_time <= end_time)
			{
				vty_out(vty,"%s",buff);		
			}
		}
		
		memset(buff, 0, sizeof(buff));
		
		fgets(buff, sizeof(buff), p_file);		
	}

	pclose(p_file);
	
	return CMD_SUCCESS;
}

#if 1
/* eag */
#endif

DEFUN(conf_eag_func,
	conf_eag_cmd,
	"config eag",
	CONFIG_STR
	"Config eag\n"
)
{
	if(CONFIG_NODE == vty->node)
	{
		vty->node = EAG_NODE;
		//vty->index = NULL;	
	}
	else if( HANSI_NODE == vty->node )
	{
		vty->node = HANSI_EAG_NODE;
	}
	else if( LOCAL_HANSI_NODE == vty->node )
	{
		vty->node = LOCAL_HANSI_EAG_NODE;
	}
	else
	{
		vty_out (vty, "Terminal mode change must under configure mode!\n");
		return CMD_WARNING;
	}

	//vty_out(vty,"for test! config eag\n");
	
	return CMD_SUCCESS;
}

#if 1
/* nas id */
#endif

int eag_nas_policy_is_legal_nasid(const char *strnasid)
{
	const char *p;

	if (NULL == strnasid || '\0' == strnasid[0] || strlen(strnasid) > MAX_NASID_LEN-1)
		return 0;
	
	for (p = strnasid; *p; p++)
		if (!IS_PRINT(*p))
			break;
	return '\0' == *p;
}

int eag_nas_policy_has_item(int policy_id, const char *attr)
{
	char tmpz[20];
	
	memset(tmpz, 0, 20);
	sprintf(tmpz, "%s%d", MTN_N, policy_id);
	
	int flag = 1;
	if (access(MULTI_NAS_F, 0) != 0)
		return 0;
	flag = if_design_node(MULTI_NAS_F, tmpz, ATT_Z, (char *)attr);

	return 0 == flag;
}

int eag_nas_policy_add_item(int policy_id, const char *attr, const char *nas_type, const char *begin_point, const char *end_point, const char *nasid, const char *syntaxis_point)
{
	char *tmpz;

	tmpz = (char *)malloc(20);
	memset(tmpz, 0, 20);
	sprintf(tmpz, "%s%d", MTN_N, policy_id);	
	
	add_eag_node_attr(MULTI_NAS_F, tmpz, ATT_Z, (char *)attr);
	mod_eag_node(MULTI_NAS_F, tmpz, ATT_Z, (char *)attr, NTYPE, (char *)nas_type);
	mod_eag_node(MULTI_NAS_F, tmpz, ATT_Z, (char *)attr, NSTART, (char *)begin_point);
	mod_eag_node(MULTI_NAS_F, tmpz, ATT_Z, (char *)attr, NEND, (char *)end_point);
	mod_eag_node(MULTI_NAS_F, tmpz, ATT_Z, (char *)attr, NNASID, (char *)nasid);
	mod_eag_node(MULTI_NAS_F, tmpz, ATT_Z, (char *)attr, NCOVER, (char *)syntaxis_point);

	free(tmpz);
	//write_status_file( MULTI_NAS_STATUS, "start" );
	return 0;
}

int eag_nas_policy_del_item(int policy_id, const char *nas_type)
{
	int flag;
	char nodez[32];

	memset(nodez, 0, 32);
	snprintf(nodez, 32, "%s%d", MTN_N, policy_id);

	flag = delete_eag_onelevel(MULTI_NAS_F, nodez, ATT_Z, (char *)nas_type);	

	return flag;
}

int eag_nas_policy_get_items(int policy_id, struct st_nasz *chead, int *num)
{
	char tempz[20];
	int flag = -1;
	
	memset(tempz, 0, 20);
	snprintf(tempz, 20, "%s%d", MTN_N, policy_id);
	memset(chead, 0, sizeof(*chead));
	*num = 0;

	if (access(MULTI_NAS_F, 0) != 0)
		return flag;
	flag = read_nas_xml(MULTI_NAS_F, chead, num, tempz);
	
	return flag;
}

void show_nas_policy(struct vty *vty, int policy_id)
{
	int flag, count;
	struct st_nasz chead, *cq;
	
	flag = eag_nas_policy_get_items(policy_id, &chead, &count);
	if (count > 0){
		vty_out(vty, "====================nas policy %d====================\n", policy_id);
		vty_out(vty, "%-20s %-20s %-20s %-20s %s\n", "type", "beginning-point", "end-point", "nasid", "syntaxis-point");
	}
	for (cq = chead.next; cq; cq = cq->next)
		vty_out(vty, "%-20s %-20s %-20s %-20s %s\n", cq->ntype, cq->nstart?cq->nstart:"", cq->nend?cq->nend:"", cq->nnasid, cq->ncover);
	if(flag==0 && count > 0)
		Free_nas_info(&chead);

	vty_out(vty, "nas policy %d, count = %d\n", policy_id, count);
}

int eag_nas_policy_is_ready(int policy_id, char *err, int err_size)
{
	if (policy_id < 1 || policy_id > 5){
		snprintf(err, err_size, "eag nas policy %d out of range", policy_id);
		return 0;
	}

	if (!eag_nas_policy_has_item(policy_id, "default")){
		snprintf(err, err_size, "eag nas policy %d configuration not completed", policy_id);
		return 0;
	}
	
	return 1;
}

int eag_nas_policy_being_used(int policy_id, int *idset)
{
	int i, flag = 0;
	struct st_eagz cq;

	*idset = 0;
	for (i = 1; i <= 5; i++)
		if (eag_ins_is_running(i)){
			eag_ins_get(i, &cq);
			if (eag_nas_policy_is_legal_strid(cq.nasid) && atoi(cq.nasid) == policy_id){
				flag = 1;
				*idset |= 1<<i;
			}
		}

		return flag;		
}

#define check_eag_nas_being_used(policy_id) \
	do { \
		int idset = 0; \
		if (eag_nas_policy_being_used(policy_id, &idset)){ \
			char err[256]; \
			int i; \
			snprintf(err, sizeof(err), "eag nas policy %d being used by eag instance ", policy_id); \
			for (i = 1; i<=5; i++) \
				if ((1<<i)&idset) \
					snprintf(err+strlen(err), sizeof(err)-strlen(err), "%d, ", i); \
			snprintf(err+strlen(err), sizeof(err)-strlen(err), "please stop eag instance first"); \
			vty_out(vty, "%s\n", err); \
			return CMD_FAILURE; \
		} \
	} while(0)

DEFUN(conf_eag_nas_policy_func,
	conf_eag_nas_policy_cmd,
	"config eag-nas-policy <1-5>",
	CONFIG_STR
	"Config the eag nas policy\n" 
	"ID of the eag nas policy\n"
)
{
	int policy_id = 0;

	do {	/* check config file */
		char *tmp=(char *)malloc(64);
		int ret;
		
		if(access(MULTI_NAS_F,0)!=0)
		{
			create_eag_xml(MULTI_NAS_F);
		}
		else
		{
			ret=if_xml_file_z(MULTI_NAS_F);
			if(ret!=0)
			{
				memset(tmp,0,64);
				sprintf(tmp,"sudo rm  %s > /dev/null",MULTI_NAS_F);
				system(tmp);
				create_eag_xml(MULTI_NAS_F);
			}
		}
		free(tmp);
	}while(0);
	
	policy_id = atoi(argv[0]);
	if (EAG_NODE == vty->node) 
	{
		vty->node = EAG_NASID_NODE;
		vty->index = (void *)policy_id;
	}
	else
	{
		vty_out(vty, "Terminal mode change must under eag mode!\n");
		return CMD_WARNING;
	}
	
	return CMD_SUCCESS;
}

DEFUN(add_eag_nas_policy_vlan_func,
	add_eag_nas_policy_vlan_cmd,
	"add vlan <1-4094> <1-4094> NASID <0-99> [default]",
	"add eag nas policy. if first, would be default; if exist, and not default, modify it\n"
	"nas policy, vlan type\n"
	"beginning point of vlan id\n"
	"end point of vlan id\n"
	"nas id\n"
	"syntaxis point\n"
	"make this item of policy default\n"
)
{
	//vty_out(vty, "argc = %d\n", argc);
	//int i;
	//for (i = 0; i < argc; i++)
	//	vty_out(vty, "argv[%d] = %s\n", i, argv[i]);
	int cur_id;
	const char *begin_point, *end_point, *nasid, *syntaxis_point, *nas_type;
	char attr[64];
	
	cur_id = (int)(vty->index);
	if (cur_id < 0 || cur_id > 5){
		vty_out(vty, "nas policy id must be <0-5>!\n");
		return CMD_WARNING;
	}

	nas_type = "vlan";
	begin_point = argv[0];
	end_point = argv[1];
	nasid = argv[2];
	if (!eag_nas_policy_is_legal_nasid(nasid)){
		vty_out(vty, "error nasid format\n");
		return CMD_WARNING;
	}
	syntaxis_point = argv[3];

	if (5 == argc && strcmp(argv[4], "default") != 0){
		vty_out(vty, "error parameter : %s\n", argv[4]);
		return CMD_WARNING;
	}
	
	if (!eag_nas_policy_has_item(cur_id, "default") || 5 == argc)
		strcpy(attr, "default");
	else
		snprintf(attr, sizeof(attr), "%s_%s_%s", nas_type, begin_point, end_point);
		
	
	check_eag_nas_being_used(cur_id);
	eag_nas_policy_add_item(cur_id, attr, nas_type, begin_point, end_point, nasid, syntaxis_point);
	
	return CMD_SUCCESS;
}

DEFUN(add_eag_nas_policy_subintf_both_func,
	add_eag_nas_policy_subintf_both_cmd,
	"add subintf both BEGIN_POINT END_POINT NASID <0-99> [default]",
	"add eag nas policy. if first, would be default; if exist, and not default, modify it\n"
	"nas policy, subintf type\n"
	"both begin and end point written\n"
	"beginning point\n"
	"end point\n"
	"nas id\n"
	"syntaxis point\n"
	"make this item of policy default\n"
)
{
	//vty_out(vty, "argc = %d\n", argc);
	//int i;
	//for (i = 0; i < argc; i++)
	//	vty_out(vty, "argv[%d] = %s\n", i, argv[i]);
	int cur_id;
	const char *begin_point, *end_point, *nasid, *syntaxis_point, *nas_type;
	char attr[64];

	nas_type = "subintf";
	cur_id = (int)(vty->index);
	if (cur_id < 0 || cur_id > 5){
		vty_out(vty, "nas policy id must be <0-5>!\n");
		return CMD_WARNING;
	}

	begin_point = argv[0];
	if (strlen(begin_point) > MAX_NAS_BEGIN_POINT_LEN-1){
		vty_out(vty, "error begin point format\n");
		return CMD_WARNING;
	}
	
	end_point = argv[1];
	if (strlen(end_point) > MAX_NAS_END_POINT_LEN-1){
		vty_out(vty, "error end point format\n");
		return CMD_WARNING;
	}
	
	nasid = argv[2];
	if (!eag_nas_policy_is_legal_nasid(nasid)){
		vty_out(vty, "error nasid format\n");
		return CMD_WARNING;
	}
	syntaxis_point = argv[3];

	if (5 == argc && strcmp(argv[4], "default") != 0){
		vty_out(vty, "error parameter : %s\n", argv[4]);
		return CMD_WARNING;
	}
	
	if (!eag_nas_policy_has_item(cur_id, "default") || 5 == argc)
		strcpy(attr, "default");
	else
		snprintf(attr, sizeof(attr), "%s_%s_%s", nas_type, begin_point, end_point);
		
	
	check_eag_nas_being_used(cur_id);
	eag_nas_policy_add_item(cur_id, attr, nas_type, begin_point, end_point, nasid, syntaxis_point);
	
	return CMD_SUCCESS;
}

DEFUN(add_eag_nas_policy_subintf_begin_func,
	add_eag_nas_policy_subintf_begin_cmd,
	"add subintf begin BEGIN_POINT NASID <0-99> [default]",
	"add eag nas policy. if first, would be default; if exist, and not default, modify it\n"
	"nas policy, subintf type\n"
	"only begin point written\n"
	"beginning point\n"
	"nas id\n"
	"syntaxis point\n"
	"make this item of policy default\n"
)
{
	//vty_out(vty, "argc = %d\n", argc);
	//int i;
	//for (i = 0; i < argc; i++)
	//	vty_out(vty, "argv[%d] = %s\n", i, argv[i]);
	int cur_id;
	const char *begin_point, *nasid, *syntaxis_point, *nas_type;
	char attr[64];

	nas_type = "subintf";
	cur_id = (int)(vty->index);
	if (cur_id < 0 || cur_id > 5){
		vty_out(vty, "nas policy id must be <0-5>!\n");
		return CMD_WARNING;
	}

	begin_point = argv[0];
	if (strlen(begin_point) > MAX_NAS_BEGIN_POINT_LEN-1){
		vty_out(vty, "error begin point format\n");
		return CMD_WARNING;
	}
	
	nasid = argv[1];
	if (!eag_nas_policy_is_legal_nasid(nasid)){
		vty_out(vty, "error nasid format\n");
		return CMD_WARNING;
	}
	syntaxis_point = argv[2];

	if (4 == argc && strcmp(argv[3], "default") != 0){
		vty_out(vty, "error parameter : %s\n", argv[3]);
		return CMD_WARNING;
	}
	
	if (!eag_nas_policy_has_item(cur_id, "default") || 4 == argc)
		strcpy(attr, "default");
	else
		snprintf(attr, sizeof(attr), "%s_%s_%s", nas_type, begin_point, "");
		
	
	check_eag_nas_being_used(cur_id);
	eag_nas_policy_add_item(cur_id, attr, nas_type, begin_point, "", nasid, syntaxis_point);
	
	return CMD_SUCCESS;
}

DEFUN(add_eag_nas_policy_subintf_end_func,
	add_eag_nas_policy_subintf_end_cmd,
	"add subintf end END_POINT NASID <0-99> [default]",
	"add eag nas policy. if first, would be default; if exist, and not default, modify it\n"
	"nas policy, subintf type\n"
	"only end point written\n"
	"end point\n"
	"nas id\n"
	"syntaxis point\n"
	"make this item of policy default\n"
)
{
	//vty_out(vty, "argc = %d\n", argc);
	//int i;
	//for (i = 0; i < argc; i++)
	//	vty_out(vty, "argv[%d] = %s\n", i, argv[i]);
	int cur_id;
	const char *end_point, *nasid, *syntaxis_point, *nas_type;
	char attr[64];

	nas_type = "subintf";
	cur_id = (int)(vty->index);
	if (cur_id < 0 || cur_id > 5){
		vty_out(vty, "nas policy id must be <0-5>!\n");
		return CMD_WARNING;
	}
	
	end_point = argv[0];
	if (strlen(end_point) > MAX_NAS_END_POINT_LEN-1){
		vty_out(vty, "error end point format\n");
		return CMD_WARNING;
	}
	
	nasid = argv[1];
	if (!eag_nas_policy_is_legal_nasid(nasid)){
		vty_out(vty, "error nasid format\n");
		return CMD_WARNING;
	}
	syntaxis_point = argv[2];

	if (4 == argc && strcmp(argv[3], "default") != 0){
		vty_out(vty, "error parameter : %s\n", argv[3]);
		return CMD_WARNING;
	}
	
	if (!eag_nas_policy_has_item(cur_id, "default") || 4 == argc)
		strcpy(attr, "default");
	else
		snprintf(attr, sizeof(attr), "%s_%s_%s", nas_type, "", end_point);
		
	
	check_eag_nas_being_used(cur_id);
	eag_nas_policy_add_item(cur_id, attr, nas_type, "", end_point, nasid, syntaxis_point);
	
	return CMD_SUCCESS;
}

DEFUN(add_eag_nas_policy_subintf_none_func,
	add_eag_nas_policy_subintf_none_cmd,
	"add subintf none NASID <0-99> [default]",
	"add eag nas policy. if first, would be default; if exist, and not default, modify it\n"
	"nas policy, subintf type\n"
	"neither begin nor end point written\n"
	"nas id\n"
	"syntaxis point\n"
	"make this item of policy default\n"
)
{
	//vty_out(vty, "argc = %d\n", argc);
	//int i;
	//for (i = 0; i < argc; i++)
	//	vty_out(vty, "argv[%d] = %s\n", i, argv[i]);
	int cur_id;
	const char *nasid, *syntaxis_point, *nas_type;
	char attr[64];

	nas_type = "subintf";
	cur_id = (int)(vty->index);
	if (cur_id < 0 || cur_id > 5){
		vty_out(vty, "nas policy id must be <0-5>!\n");
		return CMD_WARNING;
	}
	
	nasid = argv[0];
	if (!eag_nas_policy_is_legal_nasid(nasid)){
		vty_out(vty, "error nasid format\n");
		return CMD_WARNING;
	}
	syntaxis_point = argv[1];

	if (3 == argc && strcmp(argv[2], "default") != 0){
		vty_out(vty, "error parameter : %s\n", argv[2]);
		return CMD_WARNING;
	}
	
	if (!eag_nas_policy_has_item(cur_id, "default") || 3 == argc)
		strcpy(attr, "default");
	else
		snprintf(attr, sizeof(attr), "%s_%s_%s", nas_type, "", "");
		
	
	check_eag_nas_being_used(cur_id);
	eag_nas_policy_add_item(cur_id, attr, nas_type, "", "", nasid, syntaxis_point);
	
	return CMD_SUCCESS;
}

DEFUN(add_eag_nas_policy_wlan_func,
	add_eag_nas_policy_wlan_cmd,
	"add wlan <1-15> <1-15> NASID <0-99> [default]",
	"add eag nas policy. if first, would be default; if exist, and not default, modify it\n"
	"nas policy, wlan type\n"
	"beginning point of wlan id\n"
	"end point of wlan id\n"
	"nas id\n"
	"syntaxis point\n"
	"make this item of policy default\n"
)
{
	//vty_out(vty, "argc = %d\n", argc);
	//int i;
	//for (i = 0; i < argc; i++)
	//	vty_out(vty, "argv[%d] = %s\n", i, argv[i]);
	int cur_id;
	const char *begin_point, *end_point, *nasid, *syntaxis_point, *nas_type;
	char attr[64];
	
	cur_id = (int)(vty->index);
	if (cur_id < 0 || cur_id > 5){
		vty_out(vty, "nas policy id must be <0-5>!\n");
		return CMD_WARNING;
	}

	nas_type = "wlan";
	begin_point = argv[0];
	end_point = argv[1];
	nasid = argv[2];
	if (!eag_nas_policy_is_legal_nasid(nasid)){
		vty_out(vty, "error nasid format\n");
		return CMD_WARNING;
	}
	syntaxis_point = argv[3];

	if (5 == argc && strcmp(argv[4], "default") != 0){
		vty_out(vty, "error parameter : %s\n", argv[4]);
		return CMD_WARNING;
	}
	
	if (!eag_nas_policy_has_item(cur_id, "default") || 5 == argc)
		strcpy(attr, "default");
	else
		snprintf(attr, sizeof(attr), "%s_%s_%s", nas_type, begin_point, end_point);
		
	
	check_eag_nas_being_used(cur_id);
	eag_nas_policy_add_item(cur_id, attr, nas_type, begin_point, end_point, nasid, syntaxis_point);
	
	return CMD_SUCCESS;
}

DEFUN(add_eag_nas_policy_ipaddress_func,
	add_eag_nas_policy_ipaddress_cmd,
	"add ipaddress A.B.C.D A.B.C.D NASID <0-99> [default]",
	"add eag nas policy. if first, would be default; if exist, and not default, modify it\n"
	"nas policy, ipAddress type\n"
	"beginning point of ip address\n"
	"end point of ip address\n"
	"nas id\n"
	"syntaxis point\n"
	"make this item of policy default\n"
)
{
	//vty_out(vty, "argc = %d\n", argc);
	//int i;
	//for (i = 0; i < argc; i++)
	//	vty_out(vty, "argv[%d] = %s\n", i, argv[i]);
	int cur_id;
	const char *begin_point, *end_point, *nasid, *syntaxis_point, *nas_type;
	char attr[64];
	
	cur_id = (int)(vty->index);
	if (cur_id < 0 || cur_id > 5){
		vty_out(vty, "nas policy id must be <0-5>!\n");
		return CMD_WARNING;
	}

	nas_type = "ipAddress";
	begin_point = argv[0];
	if (eag_check_ip_format(begin_point) != EAG_SUCCESS){
		vty_out(vty, "error ip address input : %s\n", begin_point);
		return CMD_WARNING;
	}
	end_point = argv[1];
	if (eag_check_ip_format(end_point) != EAG_SUCCESS){
		vty_out(vty, "error ip address input : %s\n", end_point);
		return CMD_WARNING;
	}
	nasid = argv[2];
	if (!eag_nas_policy_is_legal_nasid(nasid)){
		vty_out(vty, "error nasid format\n");
		return CMD_WARNING;
	}
	syntaxis_point = argv[3];

	if (5 == argc && strcmp(argv[4], "default") != 0){
		vty_out(vty, "error parameter : %s\n", argv[4]);
		return CMD_WARNING;
	}
	
	if (!eag_nas_policy_has_item(cur_id, "default") || 5 == argc)
		strcpy(attr, "default");
	else
		snprintf(attr, sizeof(attr), "%s_%s_%s", nas_type, begin_point, end_point);
		
	
	check_eag_nas_being_used(cur_id);
	eag_nas_policy_add_item(cur_id, attr, nas_type, begin_point, end_point, nasid, syntaxis_point);
	
	return CMD_SUCCESS;
}

DEFUN(add_eag_nas_policy_wtp_func,
	add_eag_nas_policy_wtp_cmd,
	"add wtp <1-4094> <1-4094> NASID <0-99> [default]",
	"add eag nas policy. if first, would be default; if exist, and not default, modify it\n"
	"nas policy, wtp type\n"
	"beginning point of wtp id\n"
	"end point of wtp id\n"
	"nas id\n"
	"syntaxis point\n"
	"make this item of policy default\n"
)
{
	//vty_out(vty, "argc = %d\n", argc);
	//int i;
	//for (i = 0; i < argc; i++)
	//	vty_out(vty, "argv[%d] = %s\n", i, argv[i]);
	int cur_id;
	const char *begin_point, *end_point, *nasid, *syntaxis_point, *nas_type;
	char attr[64];
	
	cur_id = (int)(vty->index);
	if (cur_id < 0 || cur_id > 5){
		vty_out(vty, "nas policy id must be <0-5>!\n");
		return CMD_WARNING;
	}

	nas_type = "wtp";
	begin_point = argv[0];
	end_point = argv[1];
	nasid = argv[2];
	if (!eag_nas_policy_is_legal_nasid(nasid)){
		vty_out(vty, "error nasid format\n");
		return CMD_WARNING;
	}
	syntaxis_point = argv[3];

	if (5 == argc && strcmp(argv[4], "default") != 0){
		vty_out(vty, "error parameter : %s\n", argv[4]);
		return CMD_WARNING;
	}
	
	if (!eag_nas_policy_has_item(cur_id, "default") || 5 == argc)
		strcpy(attr, "default");
	else
		snprintf(attr, sizeof(attr), "%s_%s_%s", nas_type, begin_point, end_point);
		
	
	check_eag_nas_being_used(cur_id);
	eag_nas_policy_add_item(cur_id, attr, nas_type, begin_point, end_point, nasid, syntaxis_point);
	
	return CMD_SUCCESS;
}

DEFUN(del_eag_nas_policy_vlan_func,
	del_eag_nas_policy_vlan_cmd,
	"del vlan <1-4094> <1-4094>",
	"del eag nas policy, need specify the nas type\n"
	"vlan type\n"
	"beginning point of vlan id\n"
	"end point of vlan id\n"
)
{
	//vty_out(vty, "argc = %d\n", argc);
	//int i;
	//for (i = 0; i < argc; i++)
	//	vty_out(vty, "argv[%d] = %s\n", i, argv[i]);
	int cur_id, ret;
	const char *begin_point, *end_point;
	char attr[64];
	
	cur_id = (int)(vty->index);
	if (cur_id < 0 || cur_id > 5){
		vty_out(vty, "nas policy id must be <0-5>!\n");
		return CMD_WARNING;
	}

	begin_point = argv[0];
	end_point = argv[1];
	snprintf(attr, sizeof(attr), "%s_%s_%s", "vlan", begin_point, end_point);
	
	check_eag_nas_being_used(cur_id);
	ret = eag_nas_policy_del_item(cur_id, attr);
	if (ret != 0)
		vty_out(vty, "nas policy not exit, or default can't deleted\n");
	
	return CMD_SUCCESS;
}

DEFUN(del_eag_nas_policy_subintf_both_func,
	del_eag_nas_policy_subintf_both_cmd,
	"del subintf both BEGIN_POINT END_POINT",
	"del eag nas policy, need specify the nas type\n"
	"subintf type\n"
	"both begin and end point written\n"
	"beginning point\n"
	"end point\n"
)
{
	//vty_out(vty, "argc = %d\n", argc);
	//int i;
	//for (i = 0; i < argc; i++)
	//	vty_out(vty, "argv[%d] = %s\n", i, argv[i]);
	int cur_id, ret;
	const char *begin_point, *end_point;
	char attr[64];
	
	cur_id = (int)(vty->index);
	if (cur_id < 0 || cur_id > 5){
		vty_out(vty, "nas policy id must be <0-5>!\n");
		return CMD_WARNING;
	}

	begin_point = argv[0];
	if (strlen(begin_point) > MAX_NAS_BEGIN_POINT_LEN-1){
		vty_out(vty, "error begin point format\n");
		return CMD_WARNING;
	}
	
	end_point = argv[1];
	if (strlen(end_point) > MAX_NAS_END_POINT_LEN-1){
		vty_out(vty, "error end point format\n");
		return CMD_WARNING;
	}
	
	snprintf(attr, sizeof(attr), "%s_%s_%s", "subintf", begin_point, end_point);
	
	check_eag_nas_being_used(cur_id);
	ret = eag_nas_policy_del_item(cur_id, attr);
	if (ret != 0)
		vty_out(vty, "nas policy not exit, or default can't deleted\n");
	
	return CMD_SUCCESS;
}

DEFUN(del_eag_nas_policy_subintf_begin_func,
	del_eag_nas_policy_subintf_begin_cmd,
	"del subintf begin BEGIN_POINT",
	"del eag nas policy, need specify the nas type\n"
	"subintf type\n"
	"only begin point written\n"
	"beginning point\n"
)
{
	//vty_out(vty, "argc = %d\n", argc);
	//int i;
	//for (i = 0; i < argc; i++)
	//	vty_out(vty, "argv[%d] = %s\n", i, argv[i]);
	int cur_id, ret;
	const char *begin_point;
	char attr[64];
	
	cur_id = (int)(vty->index);
	if (cur_id < 0 || cur_id > 5){
		vty_out(vty, "nas policy id must be <0-5>!\n");
		return CMD_WARNING;
	}

	begin_point = argv[0];
	if (strlen(begin_point) > MAX_NAS_BEGIN_POINT_LEN-1){
		vty_out(vty, "error begin point format\n");
		return CMD_WARNING;
	}
	
	snprintf(attr, sizeof(attr), "%s_%s_%s", "subintf", begin_point, "");
	
	check_eag_nas_being_used(cur_id);
	ret = eag_nas_policy_del_item(cur_id, attr);
	if (ret != 0)
		vty_out(vty, "nas policy not exit, or default can't deleted\n");
	
	return CMD_SUCCESS;
}

DEFUN(del_eag_nas_policy_subintf_end_func,
	del_eag_nas_policy_subintf_end_cmd,
	"del subintf end END_POINT",
	"del eag nas policy, need specify the nas type\n"
	"subintf type\n"
	"only end point written\n"
	"end point\n"
)
{
	//vty_out(vty, "argc = %d\n", argc);
	//int i;
	//for (i = 0; i < argc; i++)
	//	vty_out(vty, "argv[%d] = %s\n", i, argv[i]);
	int cur_id, ret;
	const char *end_point;
	char attr[64];
	
	cur_id = (int)(vty->index);
	if (cur_id < 0 || cur_id > 5){
		vty_out(vty, "nas policy id must be <0-5>!\n");
		return CMD_WARNING;
	}

	end_point = argv[0];
	if (strlen(end_point) > MAX_NAS_BEGIN_POINT_LEN-1){
		vty_out(vty, "error end point format\n");
		return CMD_WARNING;
	}
	
	snprintf(attr, sizeof(attr), "%s_%s_%s", "subintf", "", end_point);
	
	check_eag_nas_being_used(cur_id);
	ret = eag_nas_policy_del_item(cur_id, attr);
	if (ret != 0)
		vty_out(vty, "nas policy not exit, or default can't deleted\n");
	
	return CMD_SUCCESS;
}

DEFUN(del_eag_nas_policy_subintf_none_func,
	del_eag_nas_policy_subintf_none_cmd,
	"del subintf none",
	"del eag nas policy, need specify the nas type\n"
	"subintf type\n"
	"neither begin nor end point written\n"
)
{
	//vty_out(vty, "argc = %d\n", argc);
	//int i;
	//for (i = 0; i < argc; i++)
	//	vty_out(vty, "argv[%d] = %s\n", i, argv[i]);
	int cur_id, ret;
	const char *begin_point, *end_point;
	char attr[64];
	
	cur_id = (int)(vty->index);
	if (cur_id < 0 || cur_id > 5){
		vty_out(vty, "nas policy id must be <0-5>!\n");
		return CMD_WARNING;
	}
	
	snprintf(attr, sizeof(attr), "%s_%s_%s", "subintf", "", "");
	
	check_eag_nas_being_used(cur_id);
	ret = eag_nas_policy_del_item(cur_id, attr);
	if (ret != 0)
		vty_out(vty, "nas policy not exit, or default can't deleted\n");
	
	return CMD_SUCCESS;
}

DEFUN(del_eag_nas_policy_wlan_func,
	del_eag_nas_policy_wlan_cmd,
	"del wlan <1-15> <1-15>",
	"del eag nas policy, need specify the nas type\n"
	"wlan type\n"
	"beginning point of wlan id\n"
	"end point of wlan id\n"
)
{
	//vty_out(vty, "argc = %d\n", argc);
	//int i;
	//for (i = 0; i < argc; i++)
	//	vty_out(vty, "argv[%d] = %s\n", i, argv[i]);
	int cur_id, ret;
	const char *begin_point, *end_point;
	char attr[64];
	
	cur_id = (int)(vty->index);
	if (cur_id < 0 || cur_id > 5){
		vty_out(vty, "nas policy id must be <0-5>!\n");
		return CMD_WARNING;
	}

	begin_point = argv[0];
	end_point = argv[1];
	snprintf(attr, sizeof(attr), "%s_%s_%s", "wlan", begin_point, end_point);
	
	check_eag_nas_being_used(cur_id);
	ret = eag_nas_policy_del_item(cur_id, attr);
	if (ret != 0)
		vty_out(vty, "nas policy not exit, or default can't deleted\n");
	
	return CMD_SUCCESS;
}

DEFUN(del_eag_nas_policy_ipaddress_func,
	del_eag_nas_policy_ipaddress_cmd,
	"del ipaddress A.B.C.D A.B.C.D",
	"del eag nas policy, need specify the nas type\n"
	"ipAddress type\n"
	"beginning point of ip address\n"
	"end point of ip address\n"
)
{
	//vty_out(vty, "argc = %d\n", argc);
	//int i;
	//for (i = 0; i < argc; i++)
	//	vty_out(vty, "argv[%d] = %s\n", i, argv[i]);
	int cur_id, ret;
	const char *begin_point, *end_point;
	char attr[64];
	
	cur_id = (int)(vty->index);
	if (cur_id < 0 || cur_id > 5){
		vty_out(vty, "nas policy id must be <0-5>!\n");
		return CMD_WARNING;
	}

	begin_point = argv[0];
	if (eag_check_ip_format(begin_point) != EAG_SUCCESS){
		vty_out(vty, "error ip address input : %s\n", begin_point);
		return CMD_WARNING;
	}
	end_point = argv[1];
	if (eag_check_ip_format(end_point) != EAG_SUCCESS){
		vty_out(vty, "error ip address input : %s\n", end_point);
		return CMD_WARNING;
	}
	snprintf(attr, sizeof(attr), "%s_%s_%s", "ipAddress", begin_point, end_point);
	
	check_eag_nas_being_used(cur_id);
	ret = eag_nas_policy_del_item(cur_id, attr);
	if (ret != 0)
		vty_out(vty, "nas policy not exit, or default can't deleted\n");
	
	return CMD_SUCCESS;
}

DEFUN(del_eag_nas_policy_wtp_func,
	del_eag_nas_policy_wtp_cmd,
	"del wtp <1-4094> <1-4094>",
	"del eag nas policy, need specify the nas type\n"
	"wtp type\n"
	"beginning point of wtp id\n"
	"end point of wtp id\n"
)
{
	//vty_out(vty, "argc = %d\n", argc);
	//int i;
	//for (i = 0; i < argc; i++)
	//	vty_out(vty, "argv[%d] = %s\n", i, argv[i]);
	int cur_id, ret;
	const char *begin_point, *end_point;
	char attr[64];
	
	cur_id = (int)(vty->index);
	if (cur_id < 0 || cur_id > 5){
		vty_out(vty, "nas policy id must be <0-5>!\n");
		return CMD_WARNING;
	}

	begin_point = argv[0];
	end_point = argv[1];
	snprintf(attr, sizeof(attr), "%s_%s_%s", "wtp", begin_point, end_point);
	
	check_eag_nas_being_used(cur_id);
	ret = eag_nas_policy_del_item(cur_id, attr);
	if (ret != 0)
		vty_out(vty, "nas policy not exit, or default can't deleted\n");
	
	return CMD_SUCCESS;
}

DEFUN(show_eag_nas_policy_func,
	show_eag_nas_policy_cmd,
	"show eag-nas-policy [<1-5>]",
	SHOW_STR
	"eag nas policy\n"
	"ID of the eag nas policy"
)
{
	if (1 == argc){
		const char *str_id;
		
		str_id = argv[0];
		if (!eag_nas_policy_is_legal_strid(str_id)){
			vty_out(vty, "eag nas policy id must be <1-5>\n");
			return CMD_WARNING;
		}
		show_nas_policy(vty, atoi(str_id));
	}
	else {
		int i;
		for (i = 1; i <= 5; i++)
			show_nas_policy(vty, i);
	}

	return CMD_SUCCESS;
}

#if 1
/* radius */
#endif

/* This function is copyed from WID_Check_IP_Format() in wid_wtp.h . */
int eag_check_ip_format(const char *str)
{
	char *endptr = NULL;
	char* endptr1 = NULL;
	char c;
	int IP,i;
	
	c = str[0];
	if (c>='0'&&c<='9'){
		IP= strtoul(str,&endptr,10);
		if(IP < 0||IP > 255)
			return EAG_FAILURE;
		else if(((IP < 10)&&((endptr - str) > 1))||((IP < 100)&&((endptr - str) > 2))||((IP < 256)&&((endptr - str) > 3)))
			return EAG_FAILURE;
		for(i = 0; i < 3; i++){
			if(endptr[0] == '\0'||endptr[0] != '.')
				return EAG_FAILURE;
			else{
				endptr1 = &endptr[1];
				IP= strtoul(&endptr[1],&endptr,10); 			
				if(IP < 0||IP > 255)
					return EAG_FAILURE;				
				else if(((IP < 10)&&((endptr - endptr1) > 1))||((IP < 100)&&((endptr - endptr1) > 2))||((IP < 256)&&((endptr - endptr1) > 3)))
					return EAG_FAILURE;
			}
		}
		if(endptr[0] == '\0' && IP >= 0)
			return EAG_SUCCESS;
		else
			return EAG_FAILURE;
	}
	else
		return EAG_FAILURE;		
}

int eag_check_port_format(const char *str)
{
	const char *p = NULL;
	int port;
	
	if (NULL == str || '\0' == str[0] || '0' == str[0])
		return EAG_FAILURE;

	for (p = str; *p; p++)
		if (*p < '0' || *p > '9')
			return EAG_FAILURE;
		
	port = atoi(str);
	if (port < 1 || port > 65535)
		return EAG_FAILURE;
	
	return EAG_SUCCESS;
}

int eag_radius_policy_has_item(int policy_id, const char *domain_name)
{
	char tmpz[20];
	
	memset(tmpz, 0, 20);
	sprintf(tmpz, "%s%d", MTR_N, policy_id);
	
	int flag = 1;
	if (access(MULTI_RADIUS_F, 0) != 0)
		return 0;
	flag = if_design_node(MULTI_RADIUS_F, tmpz, ATT_Z, (char *)domain_name);

	return 0 == flag;
}

int eag_radius_policy_modify_param(int policy_id, const char *domain_name, const char *key, const char *value)
{
	char *tmpz;
	
	tmpz = (char *)malloc(20);
	memset(tmpz, 0, 20);
	sprintf(tmpz, "%s%d", MTR_N, policy_id); 
	
	add_eag_node_attr(MULTI_RADIUS_F, tmpz, ATT_Z, (char *)domain_name);
	mod_eag_node(MULTI_RADIUS_F, tmpz, ATT_Z, (char *)domain_name, RDOMAIN, (char *)domain_name);
	mod_eag_node(MULTI_RADIUS_F, tmpz, ATT_Z, (char *)domain_name, (char *)key, (char *)value);
	
	free(tmpz);
	return 0;
}

int eag_radius_policy_del_item(int policy_id, const char *domain_name)
{
	int flag;
	char nodez[32];
	memset(nodez, 0, 32);
	snprintf(nodez, 32, "%s%d", MTR_N, policy_id);

	flag = delete_eag_onelevel(MULTI_RADIUS_F, nodez, ATT_Z, (char *)domain_name);

	return flag;
}

int eag_radius_policy_get_items(int policy_id, struct st_radiusz *chead, int *num)
{
	char tempz[20];
	int flag = -1;

	memset(tempz, 0, 20);
	snprintf(tempz, 20, "%s%d", MTR_N, policy_id);
	memset(chead, 0, sizeof(*chead));
	*num = 0;
	
	if (access(MULTI_RADIUS_F, 0) != 0)
		return flag;
	flag = read_radius_xml(MULTI_RADIUS_F, chead, num, tempz);

	return flag;
}

int eag_radius_policy_get_item_by_domain(int policy_id, const char *domain_name, struct st_radiusz *cq)
{	
	int flag = -1;
	char nodez[32];

	memset(nodez, 0, 32);
	snprintf(nodez, 32, "%s%d", MTR_N, policy_id);
	memset(cq, 0, sizeof(*cq));

	if (access(MULTI_RADIUS_F, 0) != 0)
		return flag;
	flag = get_radius_struct(MULTI_RADIUS_F, nodez, ATT_Z, (char *)domain_name, cq);

	return flag;
}

void show_radius_policy(struct vty *vty, int policy_id, const char *domain_name, const char *view)
{
	int i, count, flag;
	
	if (strcmp(view, "horizontal") == 0){
		if (strcmp(domain_name, "all") == 0){
			struct st_radiusz chead, *cq;
			flag = eag_radius_policy_get_items(policy_id, &chead, &count);
			if (count > 0){
				vty_out(vty, "====================radius policy %d====================\n", policy_id);
				vty_out(vty, "%-20s %-20s %-20s %-20s %s\n", "domain", "auth-serv-ip", "auth-serv-port", "acct-serv-ip", "acct-serv-port");
			}
			for (cq = chead.next; cq; cq = cq->next)
				vty_out(vty, "%-20s %-20s %-20s %-20s %s\n", cq->domain_name, cq->radius_server_ip, cq->radius_server_port, cq->charging_server_ip, cq->charging_server_port);
			if(flag==0 && count > 0)
				Free_radius_info(&chead);
		}
		else{
			if (eag_radius_policy_has_item(policy_id, domain_name)){
				vty_out(vty, "====================radius policy %d====================\n", policy_id);
				vty_out(vty, "%-20s %-20s %-20s %-20s %s\n", "domain", "auth-serv-ip", "auth-serv-port", "acct-serv-ip", "acct-serv-port");
				struct st_radiusz cq;
				eag_radius_policy_get_item_by_domain(policy_id, domain_name, &cq);
				count = 1;
				vty_out(vty, "%-20s %-20s %-20s %-20s %s\n", cq.domain_name, cq.radius_server_ip, cq.radius_server_port, cq.charging_server_ip, cq.charging_server_port);
			}
			else
				count = 0;
		}
	}
	else{
		if (strcmp(domain_name, "all") == 0){
			struct st_radiusz chead, *cq;
			flag = eag_radius_policy_get_items(policy_id, &chead, &count);
			if (count > 0)
				vty_out(vty, "====================radius policy %d====================\n", policy_id);
			for (cq = chead.next; cq; cq = cq->next){
				vty_out(vty, "----------domain %s----------\n", cq->domain_name);
				vty_out(vty, "%-25s : %s\n", "auth-serv-type", cq->radius_server_type);
				vty_out(vty, "%-25s : %s\n", "auth-serv-ip", cq->radius_server_ip);
				vty_out(vty, "%-25s : %s\n", "auth-serv-port", cq->radius_server_port);
				vty_out(vty, "%-25s : %s\n", "auth-serv-key", cq->radius_server_key);
				vty_out(vty, "%-25s : %s\n", "auth-serv-protocol", cq->radius_server_portal);
				vty_out(vty, "%-25s : %s\n", "acct-serv-ip", cq->charging_server_ip);
				vty_out(vty, "%-25s : %s\n", "acct-serv-port", cq->charging_server_port);
				vty_out(vty, "%-25s : %s\n", "acct-serv-key", cq->charging_server_key);
				vty_out(vty, "%-25s : %s\n", "bak-auth-serv-ip", cq->backup_radius_server_ip);
				vty_out(vty, "%-25s : %s\n", "bak-auth-serv-port", cq->backup_radius_server_port);
				vty_out(vty, "%-25s : %s\n", "bak-auth-serv-key", cq->backup_radius_server_key);
				vty_out(vty, "%-25s : %s\n", "bak-auth-serv-protocol", cq->backup_radius_server_portal);
				vty_out(vty, "%-25s : %s\n", "bak-acct-serv-ip", cq->backup_charging_server_ip);
				vty_out(vty, "%-25s : %s\n", "bak-acct-serv-port", cq->backup_charging_server_port);
				vty_out(vty, "%-25s : %s\n", "bak-acct-serv-key", cq->backup_charging_server_key);
				vty_out(vty, "%-25s : %s\n", "rev-updown-flow", strcmp(cq->swap_octets, "checked")==0?"enable":"disable");
			}
			if(flag==0 && count > 0)
				Free_radius_info(&chead);
		}
		else{
			if (eag_radius_policy_has_item(policy_id, domain_name)){
				vty_out(vty, "====================radius policy %d====================\n", policy_id);
				struct st_radiusz cq;
				eag_radius_policy_get_item_by_domain(policy_id, domain_name, &cq);
				count = 1;
				vty_out(vty, "----------domain %s----------\n", cq.domain_name);
				vty_out(vty, "%-25s : %s\n", "auth-serv-type", cq.radius_server_type);
				vty_out(vty, "%-25s : %s\n", "auth-serv-ip", cq.radius_server_ip);
				vty_out(vty, "%-25s : %s\n", "auth-serv-port", cq.radius_server_port);
				vty_out(vty, "%-25s : %s\n", "auth-serv-key", cq.radius_server_key);
				vty_out(vty, "%-25s : %s\n", "auth-serv-protocol", cq.radius_server_portal);
				vty_out(vty, "%-25s : %s\n", "acct-serv-ip", cq.charging_server_ip);
				vty_out(vty, "%-25s : %s\n", "acct-serv-port", cq.charging_server_port);
				vty_out(vty, "%-25s : %s\n", "acct-serv-key", cq.charging_server_key);
				vty_out(vty, "%-25s : %s\n", "bak-auth-serv-ip", cq.backup_radius_server_ip);
				vty_out(vty, "%-25s : %s\n", "bak-auth-serv-port", cq.backup_radius_server_port);
				vty_out(vty, "%-25s : %s\n", "bak-auth-serv-key", cq.backup_radius_server_key);
				vty_out(vty, "%-25s : %s\n", "bak-auth-serv-protocol", cq.backup_radius_server_portal);
				vty_out(vty, "%-25s : %s\n", "bak-acct-serv-ip", cq.backup_charging_server_ip);
				vty_out(vty, "%-25s : %s\n", "bak-acct-serv-port", cq.backup_charging_server_port);
				vty_out(vty, "%-25s : %s\n", "bak-acct-serv-key", cq.backup_charging_server_key);
				vty_out(vty, "%-25s : %s\n", "rev-updown-flow", strcmp(cq.swap_octets, "checked")==0?"enable":"disable");
			}
			else
				count = 0;
			}
	}
	vty_out(vty, "radius policy %d, count = %d\n", policy_id, count);
}

int eag_radius_policy_is_ready(int policy_id, char *err, int err_size)
{
	if (policy_id < 1 || policy_id > 5){
		snprintf(err, err_size, "eag radius policy %d out of range", policy_id);
		return 0;
	}
	
	if (!eag_radius_policy_has_item(policy_id, "default")){
		snprintf(err, err_size, "eag radius policy %d configuration not completed", policy_id);
		return 0;
	}
		
	return 1;
}

int eag_radius_policy_being_used(int policy_id, int *idset)
{
	int i, flag = 0;
	struct st_eagz cq;

	*idset = 0;
	for (i = 1; i <= 5; i++)
		if (eag_ins_is_running(i)){
			eag_ins_get(i, &cq);
			if (eag_radius_policy_is_legal_strid(cq.radiusid) && atoi(cq.radiusid) == policy_id){
				flag = 1;
				*idset |= 1<<i;
			}
		}

		return flag;		
}

#define check_eag_radius_being_used(policy_id) \
	do { \
		int idset = 0; \
		if (eag_radius_policy_being_used(policy_id, &idset)){ \
			char err[256]; \
			int i; \
			snprintf(err, sizeof(err), "eag radius policy %d being used by eag instance ", policy_id); \
			for (i = 1; i<=5; i++) \
				if ((1<<i)&idset) \
					snprintf(err+strlen(err), sizeof(err)-strlen(err), "%d, ", i); \
			snprintf(err+strlen(err), sizeof(err)-strlen(err), "please stop eag instance first"); \
			vty_out(vty, "%s\n", err); \
			return CMD_FAILURE; \
		} \
	} while(0)

DEFUN(conf_eag_radius_policy_func,
	conf_eag_radius_policy_cmd,
	"config eag-radius-policy <1-5>",
	CONFIG_STR
	"Config the eag radius policy\n" 
	"ID of the eag radius policy\n"
)
{
	int plcid = 0;

	do {	/* check config file */
		char *tmp=(char *)malloc(64);
		int ret;	
	
		if(access(MULTI_RADIUS_F,0)!=0)
		{
			create_eag_xml(MULTI_RADIUS_F);
			//write_status_file( MULTI_RADIUS_STATUS, "start" );
		}
		else
		{
			ret=if_xml_file_z(MULTI_RADIUS_F);
			if(ret!=0)
			{
				memset(tmp,0,64);
				sprintf(tmp,"sudo rm  %s > /dev/null",MULTI_RADIUS_F);
				system(tmp);
				create_eag_xml(MULTI_RADIUS_F);
				//write_status_file( MULTI_RADIUS_STATUS, "start" );
	  		}
		}
    	free(tmp);
	}while(0);
	
	plcid = atoi(argv[0]);
	if (EAG_NODE == vty->node) 
	{
		vty->node = EAG_RADIUS_NODE;
		vty->index = (void *)plcid;
	}
	else
	{
		vty_out(vty, "Terminal mode change must under eag mode!\n");
		return CMD_WARNING;
	}
	
	return CMD_SUCCESS;
}

DEFUN(set_eag_radius_policy_server_type_func,
	set_eag_radius_policy_server_type_cmd,
	"modify radius (default|DOMAIN) auth-serv-type (default|sam-rj|huawei)",
	"modify parameter of radius policy, if 'DOMAIN' not exist, create it, and 'default' must first created\n"
	"radius policy\n"
	"a default domain name, must first created\n"
	"domain name, specify a item of radius policy, and 'default' must first created\n"
	"authentication server type\n"
	"default of radius server type\n"
	"sam-rj of radius server type\n"
	"huawei of radius server type\n"
)
{
	//vty_out(vty, "argc = %d\n", argc);
	//int i;
	//for (i = 0; i < argc; i++)
	//	vty_out(vty, "argv[%d] = %s\n", i, argv[i]);
	int cur_id;
	const char *domain_name, *server_type;
	
	cur_id = (int)(vty->index);
	if (cur_id < 0 || cur_id > 5){
		vty_out(vty, "radius policy id must range <0-5>!\n");
		return CMD_WARNING;
	}

	domain_name = argv[0];
	if (strlen(domain_name) > MAX_RADIUS_DOMAIN_LEN-1){
		vty_out(vty, "error domain name format\n");
		return CMD_WARNING;
	}
	
	server_type = argv[1];
	if (strcmp(server_type, "default") != 0
		&& strcmp(server_type, "sam-rj") != 0
		&& strcmp(server_type, "huawei") != 0){
		vty_out(vty, "unknown server type: %s\n", server_type);
		return CMD_WARNING;
	}
	
	if (strcmp(domain_name, "default") != 0 && !eag_radius_policy_has_item(cur_id, "default")){
		vty_out(vty, "must first create default radius policy\n");
		return CMD_FAILURE;
	}

	check_eag_radius_being_used(cur_id);
	eag_radius_policy_modify_param(cur_id, domain_name, RRADST, server_type);
		
	return CMD_SUCCESS;
}

DEFUN(set_eag_radius_policy_server_ip_func,
	set_eag_radius_policy_server_ip_cmd,
	"modify radius (default|DOMAIN) (auth-serv-ip|acct-serv-ip|bak-auth-serv-ip|bak-acct-serv-ip) [A.B.C.D]",
	"modify parameter of radius policy, if 'DOMAIN' not exist, create it, and 'default' must first created\n"
	"radius policy\n"
	"a default domain name, must first created\n"
	"domain name, specify a item of radius policy, and 'default' must first created\n"
	"authentication server ip\n"
	"accounting server ip\n"
	"backup authentication server ip\n"
	"backup accounting server ip\n"
	"server ip, to be blank if not fill up\n"
)
{
	//vty_out(vty, "argc = %d\n", argc);
	//int i;
	//for (i = 0; i < argc; i++)
	//	vty_out(vty, "argv[%d] = %s\n", i, argv[i]);
	int cur_id;
	const char *domain_name, *server_ip_type, *server_ip;
	
	cur_id = (int)(vty->index);
	if (cur_id < 0 || cur_id > 5){
		vty_out(vty, "radius policy id must range <0-5>!\n");
		return CMD_WARNING;
	}

	domain_name = argv[0];
	if (strlen(domain_name) > MAX_RADIUS_DOMAIN_LEN-1){
		vty_out(vty, "error domain name format\n");
		return CMD_WARNING;
	}
	
	server_ip_type = argv[1];
	if (strcmp(server_ip_type, "auth-serv-ip") == 0)
		server_ip_type = RRIP;
	else if (strcmp(server_ip_type, "acct-serv-ip") == 0)
		server_ip_type = RCIP;
	else if (strcmp(server_ip_type, "bak-auth-serv-ip") == 0)
		server_ip_type = RBIP;
	else if (strcmp(server_ip_type, "bak-acct-serv-ip") == 0)
		server_ip_type = RBCIP;
	else {
		vty_out(vty, "error parameter : %s\n", server_ip_type);
		return CMD_WARNING;
	}
	
	if (3 == argc){
		server_ip = argv[2];
		if (eag_check_ip_format(server_ip) != EAG_SUCCESS){
			vty_out(vty, "error ip address format\n");
			return CMD_WARNING;
		}
	}
	else
		server_ip = "";

	if (strcmp(domain_name, "default") != 0 && !eag_radius_policy_has_item(cur_id, "default")){
		vty_out(vty, "must first create default radius policy\n");
		return CMD_FAILURE;
	}

	check_eag_radius_being_used(cur_id);
	eag_radius_policy_modify_param(cur_id, domain_name, server_ip_type, server_ip);
		
	return CMD_SUCCESS;
}

DEFUN(set_eag_radius_policy_server_port_func,
	set_eag_radius_policy_server_port_cmd,
	"modify radius (default|DOMAIN) (auth-serv-port|acct-serv-port|bak-auth-serv-port|bak-acct-serv-port) [<1-65535>]",
	"modify parameter of radius policy, if 'DOMAIN' not exist, create it, and 'default' must first created\n"
	"radius policy\n"
	"a default domain name, must first created\n"
	"domain name, specify a item of radius policy, and 'default' must first created\n"
	"authentication server port\n"
	"accounting server port\n"
	"backup authentication server port\n"
	"backup accounting server port\n"
	"server port, to be blank if not fill up\n"
)
{
	//vty_out(vty, "argc = %d\n", argc);
	//int i;
	//for (i = 0; i < argc; i++)
	//	vty_out(vty, "argv[%d] = %s\n", i, argv[i]);
	int cur_id;
	const char *domain_name, *server_port_type, *server_port;
	
	cur_id = (int)(vty->index);
	if (cur_id < 0 || cur_id > 5){
		vty_out(vty, "radius policy id must range <0-5>!\n");
		return CMD_WARNING;
	}

	domain_name = argv[0];
	if (strlen(domain_name) > MAX_RADIUS_DOMAIN_LEN-1){
		vty_out(vty, "error domain name format\n");
		return CMD_WARNING;
	}
	
	server_port_type = argv[1];
	if (strcmp(server_port_type, "auth-serv-port") == 0)
		server_port_type = RRPORT;
	else if (strcmp(server_port_type, "acct-serv-port") == 0)
		server_port_type = RCPORT;
	else if (strcmp(server_port_type, "bak-auth-serv-port") == 0)
		server_port_type = RBPORT;
	else if (strcmp(server_port_type, "bak-acct-serv-port") == 0)
		server_port_type = RBCPORT;
	else {
		vty_out(vty, "error parameter : %s\n", server_port_type);
		return CMD_WARNING;
	}	
		
	if (3 == argc){
		server_port = argv[2];
		if (eag_check_port_format(server_port) != EAG_SUCCESS){
			vty_out(vty, "error port format\n");
			return CMD_WARNING;
		}
	}
	else
		server_port = "";

	if (strcmp(domain_name, "default") != 0 && !eag_radius_policy_has_item(cur_id, "default")){
		vty_out(vty, "must first create default radius policy\n");
		return CMD_FAILURE;
	}

	check_eag_radius_being_used(cur_id);
	eag_radius_policy_modify_param(cur_id, domain_name, server_port_type, server_port);
		
	return CMD_SUCCESS;
}

DEFUN(set_eag_radius_policy_server_key_func,
	set_eag_radius_policy_server_key_cmd,
	"modify radius (default|DOMAIN) (auth-serv-key|acct-serv-key|bak-auth-serv-key|bak-acct-serv-key) [KEY]",
	"modify parameter of radius policy, if 'DOMAIN' not exist, create it, and 'default' must first created\n"
	"radius policy\n"
	"a default domain name, must first created\n"
	"domain name, specify a item of radius policy, and 'default' must first created\n"
	"authentication server key\n"
	"accounting server key\n"
	"backup authentication server key\n"
	"backup accounting server key\n"
	"server key, to be blank if not fill up\n"
)
{
	//vty_out(vty, "argc = %d\n", argc);
	//int i;
	//for (i = 0; i < argc; i++)
	//	vty_out(vty, "argv[%d] = %s\n", i, argv[i]);
	int cur_id;
	const char *domain_name, *server_key_type, *server_key;
	
	cur_id = (int)(vty->index);
	if (cur_id < 0 || cur_id > 5){
		vty_out(vty, "radius policy id must range <0-5>!\n");
		return CMD_WARNING;
	}

	domain_name = argv[0];
	if (strlen(domain_name) > MAX_RADIUS_DOMAIN_LEN-1){
		vty_out(vty, "error domain name format\n");
		return CMD_WARNING;
	}
	
	server_key_type = argv[1];
	if (strcmp(server_key_type, "auth-serv-key") == 0)
		server_key_type = RRKEY;
	else if (strcmp(server_key_type, "acct-serv-key") == 0)
		server_key_type = RCKEY;
	else if (strcmp(server_key_type, "bak-auth-serv-key") == 0)
		server_key_type = RBKEY;
	else if (strcmp(server_key_type, "bak-acct-serv-key") == 0)
		server_key_type = RBCKEY;
	else{
		vty_out(vty, "error parameter : %s\n", server_key_type);
		return CMD_WARNING;
	}
	
	if (3 == argc){
		server_key = argv[2];
		if (strlen(server_key) > MAX_RADIUS_KEY_LEN-1){
			vty_out(vty, "error key format\n");
			return CMD_WARNING;
		}
	}
	else
		server_key = "";

	if (strcmp(domain_name, "default") != 0 && !eag_radius_policy_has_item(cur_id, "default")){
		vty_out(vty, "must first create default radius policy\n");
		return CMD_FAILURE;
	}

	check_eag_radius_being_used(cur_id);
	eag_radius_policy_modify_param(cur_id, domain_name, server_key_type, server_key);
		
	return CMD_SUCCESS;
}

DEFUN(set_eag_radius_policy_server_protocol_func,
	set_eag_radius_policy_server_protocol_cmd,
	"modify radius (default|DOMAIN) (auth-serv-protocol|bak-auth-serv-protocol) (chap|pap)",
	"modify parameter of radius policy, if 'DOMAIN' not exist, create it, and 'default' must first created\n"
	"radius policy\n"
	"a default domain name, must first created\n"
	"domain name, specify a item of radius policy, and 'default' must first created\n"
	"authentication type on authentication server\n"
	"authentication type on backup authentication server\n"
	"chap auth protocol\n"
	"pap auth protocol\n"
)
{
	//vty_out(vty, "argc = %d\n", argc);
	//int i;
	//for (i = 0; i < argc; i++)
	//	vty_out(vty, "argv[%d] = %s\n", i, argv[i]);
	int cur_id;
	const char *domain_name, *server_type, *server_protocol;
	
	cur_id = (int)(vty->index);
	if (cur_id < 0 || cur_id > 5){
		vty_out(vty, "radius policy id must range <0-5>!\n");
		return CMD_WARNING;
	}

	domain_name = argv[0];
	if (strlen(domain_name) > MAX_RADIUS_DOMAIN_LEN-1){
		vty_out(vty, "error domain name format\n");
		return CMD_WARNING;
	}
	
	server_type = argv[1];	
	if (strcmp(server_type, "auth-serv-protocol") == 0)
		server_type = RRPORTAL;
	else if (strcmp(server_type, "bak-auth-serv-protocol") == 0)
		server_type = RBPORTAL;
	else {
		vty_out(vty, "error parameter : %s\n", server_type);
		return CMD_WARNING;
	}
	
	server_protocol = argv[2];
	if (strcmp(server_protocol, "chap") == 0)
		server_protocol = "CHAP";
	else if (strcmp(server_protocol, "pap") == 0)
		server_protocol = "PAP";
	else {
		vty_out(vty, "error parameter : %s\n", server_protocol);
		return CMD_WARNING;
	}

	if (strcmp(domain_name, "default") != 0 && !eag_radius_policy_has_item(cur_id, "default")){
		vty_out(vty, "must first create default radius policy\n");
		return CMD_FAILURE;
	}

	check_eag_radius_being_used(cur_id);
	eag_radius_policy_modify_param(cur_id, domain_name, server_type, server_protocol);
		
	return CMD_SUCCESS;
}

DEFUN(set_eag_radius_policy_rev_updown_flow_func,
	set_eag_radius_policy_rev_updown_flow_cmd,
	"modify radius (default|DOMAIN) rev-updown-flow (enable|disable)",
	"modify parameter of radius policy, if 'DOMAIN' not exist, create it, and 'default' must first created\n"
	"radius policy\n"
	"a default domain name, must first created\n"
	"domain name, specify a item of radius policy, and 'default' must first created\n"
	"reverse up/down flow\n"
	"enable recersing up/down flow\n"
	"disable recersing up/down flow\n"
)
{
	//vty_out(vty, "argc = %d\n", argc);
	//int i;
	//for (i = 0; i < argc; i++)
	//	vty_out(vty, "argv[%d] = %s\n", i, argv[i]);
	int cur_id;
	const char *domain_name, *select;
	
	cur_id = (int)(vty->index);
	if (cur_id < 0 || cur_id > 5){
		vty_out(vty, "radius policy id must range <0-5>!\n");
		return CMD_WARNING;
	}

	domain_name = argv[0];
	if (strlen(domain_name) > MAX_RADIUS_DOMAIN_LEN-1){
		vty_out(vty, "error domain name format\n");
		return CMD_WARNING;
	}
	
	select = argv[1];
	if (strcmp(select, "enable") == 0)
		select = "checked";
	else if (strcmp(select, "disable") == 0)
		select = "";
	else{
		vty_out(vty, "error parameter : %s\n", select);
		return CMD_WARNING;
	}

	if (strcmp(domain_name, "default") != 0 && !eag_radius_policy_has_item(cur_id, "default")){
		vty_out(vty, "must first create default radius policy\n");
		return CMD_FAILURE;
	}

	check_eag_radius_being_used(cur_id);
	eag_radius_policy_modify_param(cur_id, domain_name, R_SWAP_OCTETS, select);
		
	return CMD_SUCCESS;
}

DEFUN(del_eag_radius_policy_by_domain_func,
	del_eag_radius_policy_by_domain_cmd,
	"delete radius DOMAIN",
	"delete the radius policy specified by domain name, but 'default' can't be deleted\n"
	"radius policy\n"
	"domain name, specify a item of radius policy\n"
)
{
	//vty_out(vty, "argc = %d\n", argc);
	//int i;
	//for (i = 0; i < argc; i++)
	//	vty_out(vty, "argv[%d] = %s\n", i, argv[i]);
	int cur_id;
	const char *domain_name;
	
	cur_id = (int)(vty->index);
	if (cur_id < 0 || cur_id > 5){
		vty_out(vty, "radius policy id must range <0-5>!\n");
		return CMD_WARNING;
	}

	domain_name = argv[0];
	if (strlen(domain_name) > MAX_RADIUS_DOMAIN_LEN-1){
		vty_out(vty, "error domain name format\n");
		return CMD_WARNING;
	}
	
	if (strcmp(domain_name, "default") == 0){
		vty_out(vty, "default radius policy can't be deleted\n");
		return CMD_WARNING;
	}
	if (!eag_radius_policy_has_item(cur_id, domain_name)){
		vty_out(vty, "radius policy %s not exist\n", domain_name);
		return CMD_WARNING;
	}

	check_eag_radius_being_used(cur_id);
	eag_radius_policy_del_item(cur_id, domain_name);
		
	return CMD_SUCCESS;
}

DEFUN(show_eag_radius_policy_func,
	show_eag_radius_policy_cmd,
	"show eag-radius-policy [<1-5>]",
	SHOW_STR
	"radius policy\n" 
	"radius policy id\n"
)
{
	if (1 == argc){
		const char *str_id;
		
		str_id = argv[0];
		if (!eag_radius_policy_is_legal_strid(str_id)){
			vty_out(vty, "eag radius policy id must be <1-5>\n");
			return CMD_WARNING;
		}
		show_radius_policy(vty, atoi(str_id), "all", "vertical");
	}
	else {
		int i;
		for (i = 1; i <= 5; i++)
			show_radius_policy(vty, i, "all", "vertical");
	}

	return CMD_SUCCESS;
}

DEFUN(show_cur_eag_radius_policy_func,
	show_cur_eag_radius_policy_cmd,
	"show eag-radius-policy (all|default|DOMAIN) (horizontal|vertical)",
	SHOW_STR
	"radius policy\n" 
	"all domain\n"
	"default domain\n"
	"specified domain\n"
	"horizontal view\n"
	"vertical view\n"
)
{
	const char *domain_name, *view;
	int cur_id;
	
	cur_id = (int)(vty->index);
	if (cur_id < 0 || cur_id > 5){
		vty_out(vty, "radius policy id must range <0-5>!\n");
		return CMD_WARNING;
	}

	domain_name = argv[0];
	
	view = argv[1];
	if (strcmp(view, "horizontal") != 0 && strcmp(view, "vertical") != 0){
		vty_out(vty, "error parameter : %s\n", view);
		return CMD_WARNING;
	}
	
	show_radius_policy(vty, cur_id, domain_name, view);

	return CMD_SUCCESS;
}


#if 1
/* portal */
#endif

int eag_portal_policy_has_item(int portal_id, const char *key_word)
{
	char tmpz[20];
	
	memset(tmpz, 0, 20);
	sprintf(tmpz, "%s%d", MTP_N, portal_id);
	
	int flag = 1;
	if (access(MULTI_PORTAL_F, 0) != 0)
		return 0;
	flag = if_design_node(MULTI_PORTAL_F, tmpz, ATT_Z, (char *)key_word);

	return 0 == flag;
}

int eag_portal_policy_modify_param(int portal_id, const char *key_word, const char *key, const char *value)
{
	char *tmpz;
	
	tmpz = (char *)malloc(20);
	memset(tmpz, 0, 20);
	sprintf(tmpz, "%s%d", MTP_N, portal_id); 
	
	add_eag_node_attr(MULTI_PORTAL_F, tmpz, ATT_Z, (char *)key_word);
	mod_eag_node(MULTI_PORTAL_F, tmpz, ATT_Z, (char *)key_word, P_KEYWORD, (char *)key_word);
	mod_eag_node(MULTI_PORTAL_F, tmpz, ATT_Z, (char *)key_word, (char *)key, (char *)value);
	
	free(tmpz);
	return 0;
}

int eag_portal_policy_del_item(int portal_id, const char *key_word)
{
	int flag;
	char nodez[32];
	memset(nodez, 0, 32);
	snprintf(nodez, 32, "%s%d", MTP_N, portal_id);

	flag = delete_eag_onelevel(MULTI_PORTAL_F, nodez, ATT_Z, (char *)key_word);

	return flag;
}

int eag_portal_policy_get_items(int policy_id, struct st_portalz *chead, int *num)
{
	char tempz[20];
	int flag = -1;
	
	memset(tempz, 0, 20);
	snprintf(tempz, 20, "%s%d", MTP_N, policy_id);
	memset(chead, 0, sizeof(*chead));
	*num = 0;

	if (access(MULTI_PORTAL_F, 0) != 0)
		return flag;
	flag = read_portal_xml(MULTI_PORTAL_F, chead, num, tempz);
	
	return flag;
}

int eag_portal_policy_is_legal_ssid(const char *str_key_word)
{
	const char *p;

	if (NULL == str_key_word || '\0' == str_key_word[0] || strlen(str_key_word) > MAX_PORTAL_SSID_LEN-1)
		return 0;
	
	for (p = str_key_word; *p; p++)
		if (!(*p >= '0' && *p <= '9' || *p >= 'A' && *p <= 'Z' || *p >= 'a' && *p <= 'z'
			|| '-' == *p || '_' == *p || '.' == *p))
			break;
		
		return '\0' == *p;
}

static int get_portal_param_by_portal_url
(
char *portal_url, 
char *url_prefix,
int url_prefix_len,
int *ip_addr, 
int *port, 
char * web_page,
int web_page_len
);

int eag_portal_policy_is_legal_weburl(const char *str)
{
	const char *p;

	if (NULL == str || '\0' == str[0] || strlen(str) > MAX_PORTAL_WEBURL_LEN-1)
		return 0;

	do {
		int ret = -1;
		char temp_buff[1024], ipaddr[256];
		int temp_int;
		strncpy(ipaddr, str, 256);
		ret = get_portal_param_by_portal_url(	ipaddr,
												temp_buff,
												sizeof(temp_buff),
												&temp_int,
												&temp_int,
												temp_buff,
												sizeof(temp_buff));
		return 1 == ret;
	} while (0);

	return 0;
}

int eag_portal_policy_get_item_by_ssid(int policy_id, const char *key_word, struct st_portalz *cq)
{	
	int flag = -1;
	char nodez[32];

	memset(nodez, 0, 32);
	snprintf(nodez, 32, "%s%d", MTP_N, policy_id);
	memset(cq, 0, sizeof(*cq));

	if (access(MULTI_PORTAL_F, 0) != 0)
		return flag;
	flag = get_portal_struct(MULTI_PORTAL_F, nodez, ATT_Z, (char *)key_word, cq);

	return flag;
}

void show_portal_policy(struct vty *vty, int policy_id)
{
	int flag, count;
	struct st_portalz chead, *cq;
	
	flag = eag_portal_policy_get_items(policy_id, &chead, &count);
	if (count > 0){
		vty_out(vty, "====================portal policy %d====================\n", policy_id);
	}
	for (cq = chead.next; cq; cq = cq->next)
	{
		vty_out(vty, "----------keyword: %s----------\n", cq->p_keyword);
		if (0 == strcmp(cq->p_keyword,"default"))
		{
			vty_out(vty, "%-12s : %s\n", "type", cq->p_type);
		}
		else
		{
			//vty_out(vty, "%-12s : %s\n", "type", "(as-default)");
		}
		//vty_out(vty, "%-12s : %s\n", "ip", cq->pip);
		//vty_out(vty, "%-12s : %s\n", "port", cq->pport);
		vty_out(vty, "%-12s : %s\n", "web", cq->pip);
		vty_out(vty, "%-12s : %s\n", "domain", cq->pdomain);
		vty_out(vty, "%-12s : %s\n", "protocol", cq->pjom);
		vty_out(vty, "%-12s : %s\n", "acname", cq->pacname);
		vty_out(vty, "%-12s : %s\n", "ad-web", cq->advertise_url);
		if (strcmp(cq->pjom, "JSON") != 0)
			vty_out(vty, "%-12s : %s\n", "notice-port", cq->pnport);
	}
	if(flag==0 && count > 0)
		Free_portal_info(&chead);

	vty_out(vty, "portal policy %d, count = %d\n", policy_id, count);
}

int eag_portal_policy_is_ready(int policy_id, char *err, int err_size)
{
	if (policy_id < 1 || policy_id > 5){
		snprintf(err, err_size, "eag portal policy %d out of range", policy_id);
		return 0;
	}
	
	if (!eag_portal_policy_has_item(policy_id, "default")){
		snprintf(err, err_size, "eag portal policy %d configuration not completed", policy_id);
		return 0;
	}

	struct st_portalz cq;
	int flag = -1;
	flag = eag_portal_policy_get_item_by_ssid(policy_id, "default", &cq);
	if (NULL == cq.pip || strcmp(cq.pip, "") == 0){
		snprintf(err, err_size, "eag portal policy %d web not configed", policy_id);
		if (0 == flag) Free_get_portal_struct(&cq);
		return 0;
	}
	if (NULL == cq.pjom || strcmp(cq.pjom, "") == 0){
		snprintf(err, err_size, "eag portal policy %d protocol not configed", policy_id);
		if (0 == flag) Free_get_portal_struct(&cq);
		return 0;
	}
	if (0 == flag) Free_get_portal_struct(&cq);

	return 1;
}

int eag_portal_policy_being_used(int policy_id, int *idset)
{
	int i, flag = 0;
	struct st_eagz cq;

	*idset = 0;
	for (i = 1; i <= 5; i++)
		if (eag_ins_is_running(i)){
			eag_ins_get(i, &cq);
			if (eag_portal_policy_is_legal_strid(cq.portalid) && atoi(cq.portalid) == policy_id){
				flag = 1;
				*idset |= 1<<i;
			}
		}

		return flag;		
}

#define check_eag_portal_being_used(policy_id) \
	do { \
		int idset = 0; \
		if (eag_portal_policy_being_used(policy_id, &idset)){ \
			char err[256]; \
			int i; \
			snprintf(err, sizeof(err), "eag portal policy %d being used by eag instance ", policy_id); \
			for (i = 1; i<=5; i++) \
				if ((1<<i)&idset) \
					snprintf(err+strlen(err), sizeof(err)-strlen(err), "%d, ", i); \
			snprintf(err+strlen(err), sizeof(err)-strlen(err), "please stop eag instance first"); \
			vty_out(vty, "%s\n", err); \
			return CMD_FAILURE; \
		} \
	} while(0)

DEFUN(conf_eag_portal_policy_func,
	conf_eag_portal_policy_cmd,
	"config eag-portal-policy <1-5>",
	CONFIG_STR
	"Config the eag portal policy\n" 
	"ID of the eag portal policy\n"
)
{
	int portal_id = 0;

	do {	/* check config file */
		char *tmp=(char *)malloc(64);
		int ret;
		
		if(access(MULTI_PORTAL_F,0)!=0)
		{
			create_eag_xml(MULTI_PORTAL_F);
			//write_status_file( MULTI_PORTAL_STATUS, "start" );
		}
		else
		{
			ret=if_xml_file_z(MULTI_PORTAL_F);
			if(ret!=0)
			{
				memset(tmp,0,64);
				sprintf(tmp,"sudo rm  %s > /dev/null",MULTI_PORTAL_F);
				system(tmp);
				create_eag_xml(MULTI_PORTAL_F);
				//write_status_file( MULTI_PORTAL_STATUS, "start" );
			}
		}
    	free(tmp);
	}while(0);
	
	portal_id = atoi(argv[0]);
	if (EAG_NODE == vty->node) 
	{
		vty->node = EAG_PORTAL_NODE;
		vty->index = (void *)portal_id;
	}
	else
	{
		vty_out(vty, "Terminal mode change must under eag mode!\n");
		return CMD_WARNING;
	}
	
	return CMD_SUCCESS;
}
				
DEFUN(set_eag_portal_policy_type_func,
	set_eag_portal_policy_type_cmd,
	"modify portal default type (Essid|Wlanid|Wtpid|Vlanid|Interface)",
	"modify parameter of portal type\n"
	"default portal policy\n"
	"default\n"
	"type\n"
	"Essid\n"
	"Wlanid\n"
	"Wtpid\n"
	"Vlanid\n"
	"Interface\n"
)
{
	int cur_id;
	const char *key_word = "default";
	const char *type;
	
	cur_id = (int)(vty->index);
	if (cur_id < 0 || cur_id > 5){
		vty_out(vty, "portal policy id must range <0-5>!\n");
		return CMD_WARNING;
	}

	if (!eag_portal_policy_is_legal_ssid(key_word)){
		vty_out(vty, "error parameter format : %s\n", key_word);
		return CMD_WARNING;
	}
	
	type = argv[0];

	if (	0 != strcmp(type,"Essid") && 0 != strcmp(type,"Wlanid") &&
			0 != strcmp(type,"Wtpid") && 0 != strcmp(type,"Vlanid") && 
			0 != strcmp(type,"Interface"))
	{
		vty_out(vty, "unknown portal type : %s ! type must be one of (Essid|Wlanid|Wtpid|Vlanid|Interface)\n", type);
		return CMD_WARNING;
	}
		
	// note : here, check if all former portals are completed.

	check_eag_portal_being_used(cur_id);
	eag_portal_policy_modify_param(cur_id, key_word, P_TYPE, type);
		
	return CMD_SUCCESS;
}

DEFUN(set_eag_portal_policy_protocol_func,
	set_eag_portal_policy_protocol_cmd,
	"modify portal (default|KEY-WORD) protocol (json|mobile)",
	"modify parameter of portal policy, if not exist, create it, and 'default' must first created\n"
	"portal policy\n"
	"a default name, must first created\n"
	"key word name, specify a item of portal policy, and 'default' must first created\n"
	"portal type\n"
	"json, inside portal\n"
	"mobile, outside portal\n"
)
{
	//vty_out(vty, "argc = %d\n", argc);
	//int i;
	//for (i = 0; i < argc; i++)
	//	vty_out(vty, "argv[%d] = %s\n", i, argv[i]);
	int cur_id;
	const char *key_word, *protocol;
	
	cur_id = (int)(vty->index);
	if (cur_id < 0 || cur_id > 5){
		vty_out(vty, "portal policy id must range <0-5>!\n");
		return CMD_WARNING;
	}

	key_word = argv[0];
	if (!eag_portal_policy_is_legal_ssid(key_word)){
		vty_out(vty, "error parameter format : %s\n", key_word);
		return CMD_WARNING;
	}
	
	protocol = argv[1];
	if (strcmp(protocol, "json") == 0)
		protocol = "JSON";
	else if (strcmp(protocol, "mobile") == 0)
		protocol = "MOBILE";
	else {
		vty_out(vty, "unknown portal type : %s\n", protocol);
		return CMD_WARNING;
	}
	
	if (strcmp(key_word, "default") != 0 && !eag_portal_policy_has_item(cur_id, "default")){
		vty_out(vty, "must first create default portal policy\n");
		return CMD_FAILURE;
	}
	
	// note : here, check if all former portals are completed.

	check_eag_portal_being_used(cur_id);
	if (!eag_portal_policy_has_item(cur_id, "default"))
		eag_portal_policy_modify_param(cur_id, "default", P_TYPE, "Essid");
	eag_portal_policy_modify_param(cur_id, key_word, PJOM, protocol);
		
	return CMD_SUCCESS;
}

DEFUN(set_eag_portal_policy_ip_func,
	set_eag_portal_policy_ip_cmd,
	"modify portal (default|KEY-WORD) ip A.B.C.D",
	"modify parameter of portal policy, if not exist, create it, and 'default' must first created\n"
	"portal policy\n"
	"a default name, must first created\n"
	"key word name, specify a item of portal policy, and 'default' must first created\n"
	"ip address of portal\n"
	"ip address of portal\n"
)
{
	//vty_out(vty, "argc = %d\n", argc);
	//int i;
	//for (i = 0; i < argc; i++)
	//	vty_out(vty, "argv[%d] = %s\n", i, argv[i]);
	int cur_id;
	const char *key_word, *ipaddr;
	
	cur_id = (int)(vty->index);
	if (cur_id < 0 || cur_id > 5){
		vty_out(vty, "portal policy id must range <0-5>!\n");
		return CMD_WARNING;
	}

	key_word = argv[0];
	if (!eag_portal_policy_is_legal_ssid(key_word)){
		vty_out(vty, "error parameter format : %s\n", key_word);
		return CMD_WARNING;
	}
	
	ipaddr = argv[1];
	if (eag_check_ip_format(ipaddr) != EAG_SUCCESS){
		vty_out(vty, "error ip address format\n");
		return CMD_WARNING;
	}
	
	if (strcmp(key_word, "default") != 0 && !eag_portal_policy_has_item(cur_id, "default")){
		vty_out(vty, "must first create default portal policy\n");
		return CMD_FAILURE;
	}

	//note :  here, check if all former portals are completed.
	
	check_eag_portal_being_used(cur_id);
	if (!eag_portal_policy_has_item(cur_id, "default"))
		eag_portal_policy_modify_param(cur_id, "default", P_TYPE, "Essid");
	eag_portal_policy_modify_param(cur_id, key_word, PIP, ipaddr);
		
	return CMD_SUCCESS;
}

DEFUN(set_eag_portal_policy_port_func,
	set_eag_portal_policy_port_cmd,
	"modify portal (default|KEY-WORD) notice-port [<1-65535>]",
	"modify parameter of portal policy, if not exist, create it, and 'default' must first created\n"
	"portal policy\n"
	"a default name, must first created\n"
	"key word name, specify a item of portal policy, and 'default' must first created\n"
	"unexpectedly-offline-notice port\n"
	"port number, to be blank if not fill up\n"
)
{
	//vty_out(vty, "argc = %d\n", argc);
	//int i;
	//for (i = 0; i < argc; i++)
	//	vty_out(vty, "argv[%d] = %s\n", i, argv[i]);
	int cur_id;
	const char *key_word, *port_type, *port;
	
	cur_id = (int)(vty->index);
	if (cur_id < 0 || cur_id > 5){
		vty_out(vty, "portal policy id must range <0-5>!\n");
		return CMD_WARNING;
	}

	key_word = argv[0];
	if (!eag_portal_policy_is_legal_ssid(key_word)){
		vty_out(vty, "error parameter format : %s\n", key_word);
		return CMD_WARNING;
	}
	/*
	port_type = argv[1];
	if (strcmp(port_type, "port") == 0)
		port_type = PPORT;
	else if (strcmp(port_type, "notice-port") == 0)
		port_type = PNPORT;
	else {
		vty_out(vty, "error parameter : %s\n", port_type);
		return CMD_WARNING;
	}	
	*/	
	if (2 == argc){
		port = argv[1];
		if (eag_check_port_format(port) != EAG_SUCCESS){
			vty_out(vty, "error port format\n");
			return CMD_WARNING;
		}
	}
	else
		port = "";

	if (strcmp(key_word, "default") != 0 && !eag_portal_policy_has_item(cur_id, "default")){
		vty_out(vty, "must first create default portal policy\n");
		return CMD_FAILURE;
	}

	//note : here, check if all former portals are completed.

	check_eag_portal_being_used(cur_id);
	if (!eag_portal_policy_has_item(cur_id, "default"))
		eag_portal_policy_modify_param(cur_id, "default", P_TYPE, "Essid");
	eag_portal_policy_modify_param(cur_id, key_word, PNPORT, port);
		
	return CMD_SUCCESS;
}

DEFUN(set_eag_portal_policy_web_url_func,
	set_eag_portal_policy_web_url_cmd,
	"modify portal (default|KEY-WORD) web WEB-URL",
	"modify parameter of portal policy, if  not exist, create it, and 'default' must first created\n"
	"portal policy\n"
	"a default name, must first created\n"
	"key word name, specify a item of portal policy, and 'default' must first created\n"
	"portal web url\n"
	"portal web url, like http://192.168.7.245/portal/login.html\n"
)
{
	//vty_out(vty, "argc = %d\n", argc);
	//int i;
	//for (i = 0; i < argc; i++)
	//	vty_out(vty, "argv[%d] = %s\n", i, argv[i]);
	int cur_id;
	const char *key_word, *web_url;
	
	cur_id = (int)(vty->index);
	if (cur_id < 0 || cur_id > 5){
		vty_out(vty, "portal policy id must range <0-5>!\n");
		return CMD_WARNING;
	}

	key_word = argv[0];
	if (!eag_portal_policy_is_legal_ssid(key_word)){
		vty_out(vty, "error parameter format : %s\n", key_word);
		return CMD_WARNING;
	}
	
	web_url = argv[1];
	if (!eag_portal_policy_is_legal_weburl(web_url)){
		vty_out(vty, "error web url format\n");
		return CMD_WARNING;
	}

	if (strcmp(key_word, "default") != 0 && !eag_portal_policy_has_item(cur_id, "default")){
		vty_out(vty, "must first create default portal policy\n");
		return CMD_FAILURE;
	}

	//note :  here, check if all former portals are completed.

	check_eag_portal_being_used(cur_id);
	if (!eag_portal_policy_has_item(cur_id, "default"))
		eag_portal_policy_modify_param(cur_id, "default", P_TYPE, "Essid");
	eag_portal_policy_modify_param(cur_id, key_word, PIP, web_url);
		
	return CMD_SUCCESS;
}

DEFUN(set_eag_portal_policy_acname_func,
	set_eag_portal_policy_acname_cmd,
	"modify portal (default|KEY-WORD) acname [.ACNAME]",
	"modify parameter of portal policy, if not exist, create it, and 'default' must first created\n"
	"portal policy\n"
	"a default name, must first created\n"
	"key word name, specify a item of portal policy, and 'default' must first created\n"
	"ac name\n"
	"ac name, to be blank if not fill up\n"
)
{
	//vty_out(vty, "argc = %d\n", argc);
	//int i;
	//for (i = 0; i < argc; i++)
	//	vty_out(vty, "argv[%d] = %s\n", i, argv[i]);
	int cur_id;
	const char *key_word, *acname;
	
	cur_id = (int)(vty->index);
	if (cur_id < 0 || cur_id > 5){
		vty_out(vty, "portal policy id must range <0-5>!\n");
		return CMD_WARNING;
	}

	key_word = argv[0];
	if (!eag_portal_policy_is_legal_ssid(key_word)){
		vty_out(vty, "error parameter format : %s\n", key_word);
		return CMD_WARNING;
	}
	
	if (2 == argc){
		acname = argv[1];
		if (strlen(acname) > MAX_PORTAL_ACNAME_LEN-1){
			vty_out(vty, "error acname format\n");
			return CMD_WARNING;
		}
	}
	else
		acname = "";

	if (strcmp(key_word, "default") != 0 && !eag_portal_policy_has_item(cur_id, "default")){
		vty_out(vty, "must first create default portal policy\n");
		return CMD_FAILURE;
	}

	//note :  here, check if all former portals are completed.
	//vty_out(vty, "acname = %s\n", acname);
	check_eag_portal_being_used(cur_id);
	if (!eag_portal_policy_has_item(cur_id, "default"))
		eag_portal_policy_modify_param(cur_id, "default", P_TYPE, "Essid");
	eag_portal_policy_modify_param(cur_id, key_word, PACN, acname);
		
	return CMD_SUCCESS;
}

DEFUN(set_eag_portal_policy_domain_func,
	set_eag_portal_policy_domain_cmd,
	"modify portal (default|KEY-WORD) domain [.DOMAIN]",
	"modify parameter of portal policy, if not exist, create it, and 'default' must first created\n"
	"portal policy\n"
	"a default name, must first created\n"
	"key word name, specify a item of portal policy, and 'default' must first created\n"
	"domain\n"
	"domain, to be blank if not fill up\n"
)
{
	//vty_out(vty, "argc = %d\n", argc);
	//int i;
	//for (i = 0; i < argc; i++)
	//	vty_out(vty, "argv[%d] = %s\n", i, argv[i]);
	int cur_id;
	const char *key_word, *domian;
	
	cur_id = (int)(vty->index);
	if (cur_id < 0 || cur_id > 5){
		vty_out(vty, "portal policy id must range <0-5>!\n");
		return CMD_WARNING;
	}

	key_word = argv[0];
	if (!eag_portal_policy_is_legal_ssid(key_word)){
		vty_out(vty, "error parameter format : %s\n", key_word);
		return CMD_WARNING;
	}
	
	if (2 == argc){
		domian = argv[1];
		if (strlen(domian) > MAX_PORTAL_DOMAIN_LEN-1){
			vty_out(vty, "error domain format\n");
			return CMD_WARNING;
		}
	}
	else
		domian = "";

	if (strcmp(key_word, "default") != 0 && !eag_portal_policy_has_item(cur_id, "default")){
		vty_out(vty, "must first create default portal policy\n");
		return CMD_FAILURE;
	}

	// note : here, check if all former portals are completed.

	check_eag_portal_being_used(cur_id);
	if (!eag_portal_policy_has_item(cur_id, "default"))
		eag_portal_policy_modify_param(cur_id, "default", P_TYPE, "Essid");
	eag_portal_policy_modify_param(cur_id, key_word, PDOMAIN, domian);
		
	return CMD_SUCCESS;
}

DEFUN(set_eag_portal_policy_ad_web_url_func,
	set_eag_portal_policy_ad_web_url_cmd,
	"modify portal (default|KEY-WORD) ad-web [WEB-URL]",
	"modify parameter of portal policy, if  not exist, create it, and 'default' must first created\n"
	"portal policy\n"
	"a default name, must first created\n"
	"key word name, specify a item of portal policy, and 'default' must first created\n"
	"advertising web url\n"
	"advertising web url, to be blank if not fill up, like http://xxx.com/welcome.html\n"
)
{
	//vty_out(vty, "argc = %d\n", argc);
	//int i;
	//for (i = 0; i < argc; i++)
	//	vty_out(vty, "argv[%d] = %s\n", i, argv[i]);
	int cur_id;
	const char *key_word, *web_url;
	
	cur_id = (int)(vty->index);
	if (cur_id < 0 || cur_id > 5){
		vty_out(vty, "portal policy id must range <0-5>!\n");
		return CMD_WARNING;
	}

	key_word = argv[0];
	if (!eag_portal_policy_is_legal_ssid(key_word)){
		vty_out(vty, "error format\n");
		return CMD_WARNING;
	}

	if (2 == argc){
		web_url = argv[1];
		if (strlen(web_url) > MAX_PORTAL_WEBURL_LEN-1){
			vty_out(vty, "error ad web url format\n");
			return CMD_WARNING;
		}
	}
	else
		web_url = "";

	if (strcmp(key_word, "default") != 0 && !eag_portal_policy_has_item(cur_id, "default")){
		vty_out(vty, "must first create default portal policy\n");
		return CMD_FAILURE;
	}

	//note :  here, check if all former portals are completed.

	check_eag_portal_being_used(cur_id);
	if (!eag_portal_policy_has_item(cur_id, "default"))
		eag_portal_policy_modify_param(cur_id, "default", P_TYPE, "Essid");
	eag_portal_policy_modify_param(cur_id, key_word, P_ADVERTISE_URL, web_url);
		
	return CMD_SUCCESS;
}

DEFUN(del_eag_portal_policy_by_key_word_func,
	del_eag_portal_policy_by_key_word_cmd,
	"delete portal KEY-WORD",
	"delete the portal policy specified by key-word, but 'default' can't be deleted\n"
	"portal policy\n"
	"key word, specify a item of portal policy\n"
)
{
	//vty_out(vty, "argc = %d\n", argc);
	//int i;
	//for (i = 0; i < argc; i++)
	//	vty_out(vty, "argv[%d] = %s\n", i, argv[i]);
	int cur_id;
	const char *key_word;
	
	cur_id = (int)(vty->index);
	if (cur_id < 0 || cur_id > 5){
		vty_out(vty, "portal policy id must range <0-5>!\n");
		return CMD_WARNING;
	}

	key_word = argv[0];
	if (!eag_portal_policy_is_legal_ssid(key_word)){
		vty_out(vty, "error format\n");
		return CMD_WARNING;
	}
	
	if (strcmp(key_word, "default") == 0){
		vty_out(vty, "default portal policy can't be deleted\n");
		return CMD_WARNING;
	}
	if (!eag_portal_policy_has_item(cur_id, key_word)){
		vty_out(vty, "portal policy %s not exist\n", key_word);
		return CMD_WARNING;
	}

	check_eag_portal_being_used(cur_id);
	eag_portal_policy_del_item(cur_id, key_word);
		
	return CMD_SUCCESS;
}

DEFUN(show_eag_portal_policy_func,
	show_eag_portal_policy_cmd,
	"show eag-portal-policy [<1-5>]",
	SHOW_STR
	"eag portal policy\n"
	"ID of the eag portal policy"
)
{
	if (1 == argc){
		const char *str_id;
		
		str_id = argv[0];
		if (!eag_portal_policy_is_legal_strid(str_id)){
			vty_out(vty, "eag portal policy id must be <1-5>\n");
			return CMD_WARNING;
		}
		show_portal_policy(vty, atoi(str_id));
	}
	else {
		int i;
		for (i = 1; i <= 5; i++)
			show_portal_policy(vty, i);
	}

	return CMD_SUCCESS;
}

#if 1
/* vlan_map */
#endif

int eag_vlanmap_policy_add_item(int policy_id, const char *wlanid_begin, const char *wlanid_end, 
							const char *wtpid_begin, const char *wtpid_end, const char *vlanid)
{
	char wwvkey[50], *tmpz;
	
 	memset(wwvkey, 0, sizeof(wwvkey));
	sprintf(wwvkey, "%s.%s.%s.%s", wlanid_begin, wlanid_end, wtpid_begin, wtpid_end);
	
	tmpz = (char *)malloc(20);
	memset(tmpz, 0, 20);
	sprintf(tmpz, "%s%d", MTW_N, policy_id);	

	add_eag_node_attr(MULTI_WWV_F, tmpz, ATT_Z, wwvkey);
	mod_eag_node(MULTI_WWV_F, tmpz, ATT_Z, wwvkey, WLANSIDZ, (char *)wlanid_begin);
	mod_eag_node(MULTI_WWV_F, tmpz, ATT_Z, wwvkey, WLANEIDZ, (char *)wlanid_end);
	mod_eag_node(MULTI_WWV_F, tmpz, ATT_Z, wwvkey, WTPSIDZ, (char *)wtpid_begin);
	mod_eag_node(MULTI_WWV_F, tmpz, ATT_Z, wwvkey, WTPEIDZ, (char *)wtpid_end);
	mod_eag_node(MULTI_WWV_F, tmpz, ATT_Z, wwvkey, VLANIDZ, (char *)vlanid);
	
	free(tmpz);
	return 0;
}

int eag_vlanmap_policy_has_item(int policy_id, const char *wlanid_begin, const char *wlanid_end, const char *wtpid_begin, const char *wtpid_end)
{
	char wwvkey[50], *tmpz;

	memset(wwvkey, 0, sizeof(wwvkey));
	sprintf(wwvkey, "%s.%s.%s.%s", wlanid_begin, wlanid_end, wtpid_begin, wtpid_end);
	
	tmpz = (char *)malloc(20);
	memset(tmpz, 0, 20);
	sprintf(tmpz, "%s%d", MTW_N, policy_id);
	
	int flag = 1;
	if (access(MULTI_WWV_F, 0) != 0)
		return 0;
	flag = if_design_node(MULTI_WWV_F, tmpz, ATT_Z, wwvkey);
	free(tmpz);

	return 0 == flag;
}

int eag_vlanmap_policy_del_item(int policy_id, const char *wlanid_begin, const char *wlanid_end, const char *wtpid_begin, const char *wtpid_end)
{
	int flag;
	char nodez[32], attz[50];

	memset(nodez, 0, 32);
	snprintf(nodez, 32, "%s%d", MTW_N, policy_id);
	memset(attz, 0, sizeof(attz));
	sprintf(attz, "%s.%s.%s.%s", wlanid_begin, wlanid_end, wtpid_begin, wtpid_end);

	flag = delete_eag_onelevel(MULTI_WWV_F, nodez, ATT_Z, attz);	

	return flag;
}

int eag_vlanmap_policy_get_items(int policy_id, struct st_wwvz *chead, int *num)
{
	char tempz[20];
	int flag = -1;
	
	memset(tempz, 0, 20);
	snprintf(tempz, 20, "%s%d", MTW_N, policy_id);
	memset(chead, 0, sizeof(*chead));
	*num = 0;

	if (access(MULTI_WWV_F, 0) != 0)
		return flag;
	flag = read_wwvz_xml(MULTI_WWV_F, chead, num, tempz);
	
	return flag;
}

void show_vlanmap_policy(struct vty *vty, int policy_id)
{
	int flag, count;
	struct st_wwvz chead, *cq;
	
	flag = eag_vlanmap_policy_get_items(policy_id, &chead, &count);
	if (count > 0){
		vty_out(vty, "====================vlan map policy %d====================\n", policy_id);
		vty_out(vty, "%-20s %-20s %-20s %-20s %s\n", "beginning-wlanid", "end-wlanid", "beginning-wtpid", "end-wtpid", "vlanid");
	}
	for (cq = chead.next; cq; cq = cq->next)
		vty_out(vty, "%-20s %-20s %-20s %-20s %s\n", cq->wlansidz, cq->wlaneidz, cq->wtpsidz, cq->wtpeidz, cq->vlanidz);
	if(flag==0 && count > 0)
		Free_wwvz_info(&chead);

	vty_out(vty, "vlan map policy %d, count = %d\n", policy_id, count);
}

int eag_vlanmap_policy_being_used(int policy_id, int *idset)
{
	int i, flag = 0;
	struct st_eagz cq;

	*idset = 0;
	for (i = 1; i <= 5; i++)
		if (eag_ins_is_running(i)){
			eag_ins_get(i, &cq);
			if (eag_vlanmap_policy_is_legal_strid(cq.wwvid) && atoi(cq.wwvid) == policy_id){
				flag = 1;
				*idset |= 1<<i;
			}
		}

		return flag;		
}

#define check_eag_vlanmap_being_used(policy_id) \
	do { \
		int idset = 0; \
		if (eag_vlanmap_policy_being_used(policy_id, &idset)){ \
			char err[256]; \
			int i; \
			snprintf(err, sizeof(err), "eag vlanmap policy %d being used by eag instance ", policy_id); \
			for (i = 1; i<=5; i++) \
				if ((1<<i)&idset) \
					snprintf(err+strlen(err), sizeof(err)-strlen(err), "%d, ", i); \
			snprintf(err+strlen(err), sizeof(err)-strlen(err), "please stop eag instance first"); \
			vty_out(vty, "%s\n", err); \
			return CMD_FAILURE; \
		} \
	} while(0)

DEFUN(conf_eag_vlan_map_policy_func,
	conf_eag_vlan_map_policy_cmd,
	"config eag-vlanmap-policy <1-5>",
	CONFIG_STR
	"Config the eag vlan map policy\n" 
	"ID of the eag vlan map policy\n"
)
{
	int policy_id = 0;

	do {	/* check config file */
		char *tmp=(char *)malloc(64);
		int ret;
		
		if(access(MULTI_WWV_F,0)!=0)
		{
			create_eag_xml(MULTI_WWV_F);
			//write_status_file( MULTI_WWV_STATUS, "start" );
		}
		else
		{
			ret=if_xml_file_z(MULTI_WWV_F);
			if(ret!=0)
			{
				memset(tmp,0,64);
				sprintf(tmp,"sudo rm  %s > /dev/null",MULTI_WWV_F);
				system(tmp);
				create_eag_xml(MULTI_WWV_F);
				//write_status_file( MULTI_WWV_STATUS, "start" );
			}
		}
    	free(tmp);
	}while(0);
	
	policy_id = atoi(argv[0]);
	if (EAG_NODE == vty->node) 
	{
		vty->node = EAG_VLANMAP_NODE;
		vty->index = (void *)policy_id;
	}
	else
	{
		vty_out(vty, "Terminal mode change must under eag mode!\n");
		return CMD_WARNING;
	}
	
	return CMD_SUCCESS;
}

DEFUN(add_eag_vlan_map_policy_func,
	add_eag_vlan_map_policy_cmd,
	"add vlanmap <1-128> <1-128> <1-2048> <1-2048> <1-4096>",
	"add eag vlan map policy, if the range of both wlanid and wtpid exist, modify mapped vlanid\n"
	"eag vlan map policy, map wlan and wtp upon vlan\n"
	"the beginning wlan id\n"
	"the end wlan id\n"
	"the beginning wtp id\n"
	"the end wtp id\n"
	"the vlan id, upon which both wlan and wtp are mapped\n"
)
{
	//vty_out(vty, "argc = %d\n", argc);
	//int i;
	//for (i = 0; i < argc; i++)
	//	vty_out(vty, "argv[%d] = %s\n", i, argv[i]);
	int cur_id;
	const char *wlanid_begin, *wlanid_end, *wtpid_begin, *wtpid_end, *vlanid;
	
	cur_id = (int)(vty->index);
	if (cur_id < 0 || cur_id > 5){
		vty_out(vty, "vlan map policy id must range <0-5>!\n");
		return CMD_WARNING;
	}

	wlanid_begin = argv[0];
	wlanid_end = argv[1];
	wtpid_begin = argv[2];
	wtpid_end = argv[3];
	vlanid = argv[4];

	if (atoi(wlanid_begin) > atoi(wlanid_end) || atoi(wtpid_begin) > atoi(wtpid_end)){
		vty_out(vty, "error, begin id greater than end id\n");
		return CMD_WARNING;
	}
	
	check_eag_vlanmap_being_used(cur_id);
	eag_vlanmap_policy_add_item(cur_id, wlanid_begin, wlanid_end, wtpid_begin, wtpid_end, vlanid);
	
	return CMD_SUCCESS;
}

DEFUN(del_eag_vlan_map_policy_func,
	del_eag_vlan_map_policy_cmd,
	"del vlanmap <1-128> <1-128> <1-2048> <1-2048>",
	"del eag vlan map policy, need specify the range of both wlan and wtp\n"
	"eag vlan map policy, map wlan and wtp upon vlan\n"
	"the beginning wlan id\n"
	"the end wlan id\n"
	"the beginning wtp id\n"
	"the end wtp id\n"
)
{
	//vty_out(vty, "argc = %d\n", argc);
	//int i;
	//for (i = 0; i < argc; i++)
	//	vty_out(vty, "argv[%d] = %s\n", i, argv[i]);
	int cur_id;
	const char *wlanid_begin, *wlanid_end, *wtpid_begin, *wtpid_end;
	
	cur_id = (int)(vty->index);
	if (cur_id < 0 || cur_id > 5){
		vty_out(vty, "vlan map policy id must range <0-5>!\n");
		return CMD_WARNING;
	}

	wlanid_begin = argv[0];
	wlanid_end = argv[1];
	wtpid_begin = argv[2];
	wtpid_end = argv[3];

	if (!eag_vlanmap_policy_has_item(cur_id, wlanid_begin, wlanid_end, wtpid_begin, wtpid_end)){
		vty_out(vty, "vlan map policy wlan:%s-%s wtp:%s-%s not exist", wlanid_begin, wlanid_end, wtpid_begin, wtpid_end);
		return CMD_FAILURE;
	}

	check_eag_vlanmap_being_used(cur_id);
	eag_vlanmap_policy_del_item(cur_id, wlanid_begin, wlanid_end, wtpid_begin, wtpid_end);
	
	return CMD_SUCCESS;
}

DEFUN(show_eag_vlan_map_policy_func,
	show_eag_vlan_map_policy_cmd,
	"show eag-vlanmap-policy [<1-5>]",
	SHOW_STR
	"eag vlan map policy, map wlan and wtp upon vlan\n"
	"ID of the eag vlan map policy"
)
{
	if (1 == argc){
		const char *str_id;
		
		str_id = argv[0];
		if (!eag_vlanmap_policy_is_legal_strid(str_id)){
			vty_out(vty, "eag vlan map policy id must be <1-5>\n");
			return CMD_WARNING;
		}
		show_vlanmap_policy(vty, atoi(str_id));
	}
	else {
		int i;
		for (i = 1; i <= 5; i++)
			show_vlanmap_policy(vty, i);
	}

	return CMD_SUCCESS;
}

#if 1
/* eag instance */
#endif

t_conf_item all_conf_item[]={
	{HS_PLOT_ID, "1", HS_PLOT_ID, 1, NULL, NULL, NULL},
	{HS_STATUS, "stop", HS_STATUS, 1, NULL, NULL, NULL},
	{HS_STATUS_KICK, "stop", HS_STATUS_KICK, 1, NULL, NULL, NULL},
	{HS_DEBUG_LOG, "stop", HS_DEBUG_LOG, 1, NULL, NULL, NULL},
	{HS_WANIF, "eth0-1", HS_WANIF, 0, NULL, NULL, NULL},
	{HS_LANIF, "eth.p.0-4", HS_LANIF, 0, NULL, NULL, NULL},
	{HS_NETWORK, "10.1.1.0", HS_NETWORK, 0, NULL, NULL, NULL},
	{HS_NETMASK, "255.255.255.0", HS_NETMASK, 0, NULL, NULL, NULL},
	{HS_UAMLISTEN, "10.1.1.254", HS_UAMLISTEN, 1, NULL, NULL, NULL},
	{HS_UAMPORT, "3990", HS_UAMPORT, 1, NULL, NULL, NULL},
	//{HS_NAS_PT, "1", HS_NAS_PT, 1, NULL, NULL, NULL},
	//{HS_RADIUS_PT, "1", HS_RADIUS_PT, 1, NULL, NULL, NULL},
	//{HS_PORTAL_PT, "1", HS_PORTAL_PT, 1, NULL, NULL, NULL},
	//{HS_WWV_PT, "1", HS_WWV_PT, 1, NULL, NULL, NULL},
	{HS_NAS_PT, "", HS_NAS_PT, 1, NULL, NULL, NULL},
	{HS_RADIUS_PT, "", HS_RADIUS_PT, 1, NULL, NULL, NULL},
	{HS_PORTAL_PT, "", HS_PORTAL_PT, 1, NULL, NULL, NULL},
	{HS_WWV_PT, "", HS_WWV_PT, 1, NULL, NULL, NULL},
	{HS_DEFIDLETIMEOUT, "60", HS_DEFIDLETIMEOUT, 1, NULL, NULL, NULL},
	{HS_VRRPID, "1", HS_VRRPID, 1, NULL, NULL, NULL},
	{HS_PPI_PORT, "1", HS_PPI_PORT, 1, NULL, NULL, NULL},
	{HS_MAX_HTTPRSP, "35", HS_MAX_HTTPRSP, 1, NULL, NULL, NULL},
};

#define MAX_ITEM_NUM	sizeof(all_conf_item)/sizeof(all_conf_item[0])

int eag_ins_set_param(int ins_id, const char *conf_key, const char *conf_value)
{
	char tmp[10];
	memset(tmp, 0, sizeof(tmp));
	sprintf(tmp, "%d", ins_id);
	
	add_eag_node_attr(MULTI_EAG_F, MTC_N, ATT_Z, tmp);
	mod_eag_node(MULTI_EAG_F, MTC_N, ATT_Z, tmp, HS_PLOT_ID, tmp);
	mod_eag_node(MULTI_EAG_F, MTC_N, ATT_Z, tmp, (char *)conf_key, (char *)conf_value);

	do {     /* note :  is necessary?  */
		int i;
		for (i = 0; i < MAX_ITEM_NUM; i++)
			if (0 == all_conf_item[i].show_flag)
				mod_eag_node(MULTI_EAG_F, MTC_N, ATT_Z, tmp, all_conf_item[i].conf_name, all_conf_item[i].conf_value);
	} while(0);
	
	return 0;
}

int eag_ins_exist(int ins_id)
{
	char tmpz[10];
	int flag = 1;
	
	memset(tmpz, 0, 10);
	sprintf(tmpz, "%d", ins_id);

	if (access(MULTI_EAG_F, 0) != 0)
		return 0;
	flag = if_design_node(MULTI_EAG_F, MTC_N, ATT_Z, tmpz);

	return 0 == flag;
}

int eag_ins_del(int ins_id)
{
	int flag;
	char tmpz[10];
	
	memset(tmpz, 0, 10);
	sprintf(tmpz, "%d", ins_id);

	flag = delete_eag_onelevel(MULTI_EAG_F, MTC_N, ATT_Z, tmpz);	

	return flag;
}

int eag_ins_get(int ins_id, struct st_eagz *cq)
{
	int flag = -1;
	char tmpz[10];
	
	memset(tmpz, 0, 10);
	sprintf(tmpz, "%d", ins_id);
	memset(cq, 0, sizeof(*cq));

	if (access(MULTI_EAG_F, 0) != 0)
		return flag;
	flag = get_eag_struct(MULTI_EAG_F, MTC_N, ATT_Z, tmpz, cq);
	
	return flag;
}

void show_eag_instance(struct vty *vty, int ins_id)
{
	if (eag_ins_exist(ins_id)){
		vty_out(vty, "====================eag instance %d====================\n", ins_id);
		struct st_eagz cq;
		eag_ins_get(ins_id, &cq);
		vty_out(vty, "%-20s : %s\n", "service", strcmp(cq.eag_start, "start")==0?"enable":"disable");
		vty_out(vty, "%-20s : %s\n", "kick-state", cq.space_start);
		vty_out(vty, "%-20s : %s\n", "debug-log", cq.debug_log);
		vty_out(vty, "%-20s : %s\n", "uam-listen-ip", cq.db_listen);
		vty_out(vty, "%-20s : %s\n", "uam-port", cq.listen_port);
		vty_out(vty, "%-20s : %s\n", "nas-policy", cq.nasid);
		vty_out(vty, "%-20s : %s\n", "radius-policy", cq.radiusid);
		vty_out(vty, "%-20s : %s\n", "portal-policy", cq.portalid);
		vty_out(vty, "%-20s : %s\n", "vlanmap-policy", cq.wwvid);
		vty_out(vty, "%-20s : %s\n", "time-out", cq.timeout);
		vty_out(vty, "%-20s : %s\n", "vrrp-id", cq.vrrpid);
		vty_out(vty, "%-20s : %s\n", "ppi-port", cq.ppi_port);
		vty_out(vty, "%-20s : %s\n", "max-http-request", cq.max_httprsp);
	}
	else
		vty_out(vty, "eag instance %d not configed\n", ins_id);
}

int eag_ins_is_ready(int ins_id, char *err, int err_size)
{
	// note : only simply check, not perfectly
	struct st_eagz cq;
	memset(&cq, 0, sizeof(cq));

	if (ins_id < 1 || ins_id > 5){
		snprintf(err, err_size, "eag instance %d out of range", ins_id);
		return 0;
	}
	
	eag_ins_get(ins_id, &cq);
	if (strcmp(cq.db_listen, "") == 0){		/* A.B.C.D */
		snprintf(err, err_size, "uam-listen-ip not configed");
		return 0;
	}
	else if (strcmp(cq.listen_port, "") == 0){		/* <0-65535> */
		snprintf(err, err_size, "uam-port not configed");
		return 0;
	}
	else if (strcmp(cq.nasid, "") == 0){		/* <1-5> */
		snprintf(err, err_size, "nas-policy not configed");
		return 0;
	}
	else if (strcmp(cq.radiusid, "") == 0){		 /* <1-5> */
		snprintf(err, err_size, "radius-policy not configed");
		return 0;
	}
	else if (strcmp(cq.portalid, "") == 0){		 /* <1-5> */
		snprintf(err, err_size, "portal-policy not configed");
		return 0;
	}
	else if (strcmp(cq.timeout, "") == 0){		 /* <1-999999999> */
		snprintf(err, err_size, "time-out not configed");
		return 0;
	}
	else if (strcmp(cq.vrrpid, "") == 0){		/* <1-999999999> */
		snprintf(err, err_size, "vrrp-id not configed");
		return 0;
	}
	else if (strcmp(cq.ppi_port, "") == 0){		/* <1-65535> */
		snprintf(err, err_size, "ppi-port not configed");
		return 0;
	}
	else if (strcmp(cq.max_httprsp, "") == 0){		/* <1-999999999> */
		snprintf(err, err_size, "max-http-request not configed");
		return 0;
	}
	
	return 1;
}

int eag_ins_is_running(int ins_id)
{
	struct st_eagz cq;
	memset(&cq, 0, sizeof(cq));

	if (ins_id < 1 || ins_id > 5){
		return 0;
	}
	
	eag_ins_get(ins_id, &cq);
	if (strcmp(cq.eag_start, "start") == 0){		/* A.B.C.D */
		return 1;
	}
	
	return 0;
}

DEFUN(conf_eag_ins_func,
	conf_eag_ins_cmd,
	"config eag-instance <1-5>",
	CONFIG_STR
	"Config the eag instance\n" 
	"Eag instance ID\n"
)
{
	int ins_id = 0;

	do {	/* check config file */
		char *tmp=(char *)malloc(100);
		int ret;

		if(access(MULTI_EAG_F,0)!=0)
		{
			create_eag_xml(MULTI_EAG_F);
			//write_status_file( MULTI_EAG_STATUS, "start" );
		}
		else
		{
			ret=if_xml_file_z(MULTI_EAG_F);
			if(ret!=0)
			{
				memset(tmp,0,100);
				sprintf(tmp,"sudo rm  %s > /dev/null",MULTI_EAG_F);
				system(tmp);
				create_eag_xml(MULTI_EAG_F);
				//write_status_file( MULTI_EAG_STATUS, "start" );
		
			}
		}
		free(tmp);
	} while(0);
	
	ins_id = atoi(argv[0]);
	if (EAG_NODE == vty->node) 
	{
		vty->node = EAG_INS_NODE;
		vty->index = (void *)ins_id;
	}
	else
	{
		vty_out(vty, "Terminal mode change must under eag mode!\n");
		return CMD_WARNING;
	}

	if (!eag_ins_exist(ins_id)){        /*note :  is necessary?  */
		int i;
		char tmp[10];
		memset(tmp, 0, sizeof(tmp));
		sprintf(tmp, "%d", ins_id);

		add_eag_node_attr(MULTI_EAG_F, MTC_N, ATT_Z, tmp);
		for (i = 0; i < MAX_ITEM_NUM; i++)
				mod_eag_node(MULTI_EAG_F, MTC_N, ATT_Z, tmp, all_conf_item[i].conf_name, all_conf_item[i].conf_value);
	}
	
	return CMD_SUCCESS;
}

DEFUN(set_eag_ins_switch_options_func,
	set_eag_ins_switch_options_cmd,
	"set (kick-state|debug-log) (enable|disable)",
	SETT_STR
	"Set kick state, allow user to do nothing during a specified time\n"
	"Set debug log\n"
	"Enable\n"
	"Disable\n"
)
{
	//vty_out(vty, "argc = %d\n", argc);
	//int i;
	//for (i = 0; i < argc; i++)
	//	vty_out(vty, "argv[%d] = %s\n", i, argv[i]);
	int cur_id;
	const char *conf_key, *conf_value;
	
	cur_id = (int)(vty->index);
	if (cur_id < 0 || cur_id > 5){
		vty_out(vty, "eag instance id must be <0-5>!\n");
		return CMD_WARNING;
	}

	conf_key = argv[0];
	if (strcmp(conf_key, "kick-state") == 0)
		conf_key = HS_STATUS_KICK;
	else if (strcmp(conf_key, "debug-log") == 0)
		conf_key = HS_DEBUG_LOG;
	else {
		vty_out(vty, "error parameter : %s\n", conf_key);
		return CMD_WARNING;
	}
	
	conf_value = argv[1];
	if (strcmp(conf_value, "enable") == 0)
		conf_value = "start";
	else if (strcmp(conf_value, "disable") == 0)
		conf_value = "stop";
	else{
		vty_out(vty, "error parameter : %s\n", conf_value);
		return CMD_WARNING;
	}

	if (eag_ins_is_running(cur_id)){
		vty_out(vty, "eag instance running, please stop it first\n");
		return CMD_FAILURE;
	}
	eag_ins_set_param(cur_id, conf_key, conf_value);
		
	return CMD_SUCCESS;
}

DEFUN(set_eag_ins_service_options_func,
	set_eag_ins_service_options_cmd,
	"service (enable|disable)",
	"Start or stop the eag instance\n"
	"Enable, start\n"
	"Disable, stop\n"
)
{
	//vty_out(vty, "argc = %d\n", argc);
	//int i;
	//for (i = 0; i < argc; i++)
	//	vty_out(vty, "argv[%d] = %s\n", i, argv[i]);
	int cur_id;
	const char *conf_key, *conf_value;
	char err[256];
	struct st_eagz cq;

	memset(err, 0, sizeof(err));
	memset(&cq, 0, sizeof(cq));
		
	cur_id = (int)(vty->index);
	if (cur_id < 0 || cur_id > 5){
		vty_out(vty, "eag instance id must be <0-5>!\n");
		return CMD_WARNING;
	}

	conf_key = HS_STATUS;

	conf_value = argv[0];
	if (strcmp(conf_value, "enable") == 0)
		conf_value = "start";
	else if (strcmp(conf_value, "disable") == 0)
		conf_value = "stop";
	else{
		vty_out(vty, "error parameter : %s\n", conf_value);
		return CMD_WARNING;
	}
		
	if (strcmp(conf_value, "start") == 0){
		if (!eag_ins_is_ready(cur_id, err, sizeof(err))){
			vty_out(vty, "%s\n", err);
			return CMD_FAILURE;
		}
		eag_ins_get(cur_id, &cq);
		if (!eag_nas_policy_is_ready(atoi(cq.nasid), err, sizeof(err))){
			vty_out(vty, "%s\n", err);
			return CMD_FAILURE;
		}
		if (!eag_radius_policy_is_ready(atoi(cq.radiusid), err, sizeof(err))){
			vty_out(vty, "%s\n", err);
			return CMD_FAILURE;
		}
		if (!eag_portal_policy_is_ready(atoi(cq.portalid), err, sizeof(err))){
			vty_out(vty, "%s\n", err);
			return CMD_FAILURE;
		}
		eag_ins_set_param(cur_id, conf_key, conf_value);
		eag_services_restart();
	}
	else {
		eag_ins_set_param(cur_id, conf_key, conf_value);
		eag_services_restart();
	}
		
	return CMD_SUCCESS;
}

DEFUN(set_eag_ins_ip_options_func,
	set_eag_ins_ip_options_cmd,
	"set uam-listen-ip A.B.C.D",
	SETT_STR
	"Set uam listen ip, that is redirection-listening ip\n"
	"ip address\n"
)
{
	//vty_out(vty, "argc = %d\n", argc);
	//int i;
	//for (i = 0; i < argc; i++)
	//	vty_out(vty, "argv[%d] = %s\n", i, argv[i]);
	int cur_id;
	const char *conf_key, *conf_value;
	
	cur_id = (int)(vty->index);
	if (cur_id < 0 || cur_id > 5){
		vty_out(vty, "eag instance id must be <0-5>!\n");
		return CMD_WARNING;
	}

	conf_key = HS_UAMLISTEN;
	conf_value = argv[0];
	if (eag_check_ip_format(conf_value) != EAG_SUCCESS){
		vty_out(vty, "error ip address input : %s\n", conf_value);
		return CMD_WARNING;
	}	

	if (eag_ins_is_running(cur_id)){
		vty_out(vty, "eag instance running, please stop it first\n");
		return CMD_FAILURE;
	}
	eag_ins_set_param(cur_id, conf_key, conf_value);

	return CMD_SUCCESS;
}

DEFUN(set_eag_ins_port_num_options_func,
	set_eag_ins_port_num_options_cmd,
	"set (uam-port|ppi-port) <0-65535>",
	SETT_STR
	"Set uam listen port, that is redirection-listening port\n"
	"Set ppi port, that is portal-protocol interaction port\n"
	"the port number\n"
)
{
	//vty_out(vty, "argc = %d\n", argc);
	//int i;
	//for (i = 0; i < argc; i++)
	//	vty_out(vty, "argv[%d] = %s\n", i, argv[i]);
	int cur_id;
	const char *conf_key, *conf_value;
	
	cur_id = (int)(vty->index);
	if (cur_id < 0 || cur_id > 5){
		vty_out(vty, "eag instance id must be <0-5>!\n");
		return CMD_WARNING;
	}
	
	conf_key = argv[0];
	if (strcmp(conf_key, "uam-port") == 0)
		conf_key = HS_UAMPORT;
	else if (strcmp(conf_key, "ppi-port") == 0)
		conf_key = HS_PPI_PORT;
	else {
		vty_out(vty, "error parameter : %s\n", conf_key);
		return CMD_WARNING;
	}
		
	conf_value = argv[1];

	if (eag_ins_is_running(cur_id)){
		vty_out(vty, "eag instance running, please stop it first\n");
		return CMD_FAILURE;
	}
	eag_ins_set_param(cur_id, conf_key, conf_value);

	return CMD_SUCCESS;
}

DEFUN(set_eag_ins_policy_func,
	set_eag_ins_policy_cmd,
	"set (nas-policy|radius-policy|portal-policy|vlanmap-policy) <1-5>",
	SETT_STR
	"Set nas policy\n"
	"Set radius policy\n"
	"Set portal policy\n"
	"Set vlan map policy\n"
	"policy id\n"
)
{
	//vty_out(vty, "argc = %d\n", argc);
	//int i;
	//for (i = 0; i < argc; i++)
	//	vty_out(vty, "argv[%d] = %s\n", i, argv[i]);
	int cur_id;
	const char *conf_key, *conf_value;
		
	cur_id = (int)(vty->index);
	if (cur_id < 0 || cur_id > 5){
		vty_out(vty, "eag instance id must be <0-5>!\n");
		return CMD_WARNING;
	}
		
	conf_key = argv[0];
	if (strcmp(conf_key, "nas-policy") == 0)
		conf_key = HS_NAS_PT;
	else if (strcmp(conf_key, "radius-policy") == 0)
		conf_key = HS_RADIUS_PT;
	else if (strcmp(conf_key, "portal-policy") == 0)
		conf_key = HS_PORTAL_PT;
	else if (strcmp(conf_key, "vlanmap-policy") == 0)
		conf_key = HS_WWV_PT;
	else {
		vty_out(vty, "error parameter : %s\n", conf_key);
		return CMD_WARNING;
	}
		
	// note : here, check if policy has default and be ready
		
	conf_value = argv[1];
	if (eag_ins_is_running(cur_id)){
		vty_out(vty, "eag instance running, please stop it first\n");
		return CMD_FAILURE;
	}
	eag_ins_set_param(cur_id, conf_key, conf_value);
	
	return CMD_SUCCESS;	
}

DEFUN(set_eag_ins_timeout_option_func,
	set_eag_ins_timeout_option_cmd,
	"set time-out <0-86400>",
	SETT_STR
	"Set time-out time, by second\n"
	"the time-out time, max 86400 second, 24*60*60\n"
)
{
	//vty_out(vty, "argc = %d\n", argc);
	//int i;
	//for (i = 0; i < argc; i++)
	//	vty_out(vty, "argv[%d] = %s\n", i, argv[i]);
	int cur_id;
	const char *conf_key, *conf_value;
	
	cur_id = (int)(vty->index);
	if (cur_id < 0 || cur_id > 5){
		vty_out(vty, "eag instance id must be <0-5>!\n");
		return CMD_WARNING;
	}
		
	conf_key = HS_DEFIDLETIMEOUT;
		
	// note : here, check if policy has default and be ready
		
	conf_value = argv[0];

	if (eag_ins_is_running(cur_id)){
		vty_out(vty, "eag instance running, please stop it first\n");
		return CMD_FAILURE;
	}
	eag_ins_set_param(cur_id, conf_key, conf_value);
	
	return CMD_SUCCESS;	
}

DEFUN(set_eag_ins_vrrpid_option_func,
	set_eag_ins_vrrpid_option_cmd,
	"set vrrp-id <0-16>",
	SETT_STR
	"Set vrrp-id\n"
	"the vrrp-id number\n"
)
{
	//vty_out(vty, "argc = %d\n", argc);
	//int i;
	//for (i = 0; i < argc; i++)
	//	vty_out(vty, "argv[%d] = %s\n", i, argv[i]);
	int cur_id;
	const char *conf_key, *conf_value;
	
	cur_id = (int)(vty->index);
	if (cur_id < 0 || cur_id > 5){
		vty_out(vty, "eag instance id must be <0-5>!\n");
		return CMD_WARNING;
	}
		
	conf_key = HS_VRRPID;
		
	// note : here, check if policy has default and be ready
		
	conf_value = argv[0];

	if (eag_ins_is_running(cur_id)){
		vty_out(vty, "eag instance running, please stop it first\n");
		return CMD_FAILURE;
	}
	eag_ins_set_param(cur_id, conf_key, conf_value);
	
	return CMD_SUCCESS;	
}

DEFUN(set_eag_ins_maxhttprsp_option_func,
	set_eag_ins_maxhttprsp_option_cmd,
	"set max-http-request <0-100>",
	SETT_STR
	"Set max http request times in 5 seconds\n"
	"the max http request times\n"
)
{
	//vty_out(vty, "argc = %d\n", argc);
	//int i;
	//for (i = 0; i < argc; i++)
	//	vty_out(vty, "argv[%d] = %s\n", i, argv[i]);
	int cur_id;
	const char *conf_key, *conf_value;
	
	cur_id = (int)(vty->index);
	if (cur_id < 0 || cur_id > 5){
		vty_out(vty, "eag instance id must be <0-5>!\n");
		return CMD_WARNING;
	}
		
	conf_key = HS_MAX_HTTPRSP;
		
	// note : here, check if policy has default and be ready
		
	conf_value = argv[0];

	if (eag_ins_is_running(cur_id)){
		vty_out(vty, "eag instance running, please stop it first\n");
		return CMD_FAILURE;
	}
	eag_ins_set_param(cur_id, conf_key, conf_value);
	
	return CMD_SUCCESS;	
}

DEFUN(del_eag_ins_func,
	del_eag_ins_cmd,
	"del eag-instance <1-5>",
	"Delete"
	"Delete the eag instance config\n"
	"specified ID for you want to delete\n"
)
{
	int ins_id;

	ins_id = atoi(argv[0]);

	if (!eag_ins_exist(ins_id)){
		vty_out(vty, "eag instance %d not configed\n", ins_id);
		return CMD_FAILURE;
	}

	// note :  if eag ins running, can be deleted?

	if (eag_ins_is_running(ins_id)){
		vty_out(vty, "eag instance %d running, please stop it first\n", ins_id);
		return CMD_FAILURE;
	}
	eag_ins_del(ins_id);
	
	return CMD_SUCCESS;
}

DEFUN(show_eag_ins_all_option_func,
	show_eag_ins_all_option_cmd,
	"show eag-instance [<1-5>]",
	SHOW_STR
	"Show the eag instance\n" 
	"Specified ID for you want to show\n"
)
{
	if (1 == argc){
		const char *str_id;
			
		str_id = argv[0];
		if (!eag_ins_is_legal_strid(str_id)){
			vty_out(vty, "eag instance id must be <1-5>\n");
			return CMD_WARNING;
		}
		show_eag_instance(vty, atoi(str_id));
	}
	else {
		int i;
		for (i = 1; i <= 5; i++)
			show_eag_instance(vty, i);
	}
	
	return CMD_SUCCESS;
}

#if 1
/* show syslog */
#endif

DEFUN(show_syslog_cli_func,
	show_syslog_cli_cmd,
	"show syslog cli [SLOTID]",
	SHOW_STR
	"Syslog information\n"
	"Display all keyword\n"
)
{	
	if (0 == argc)
	{
		return eag_show_syslog_cli(vty);
	}
	int slotid = 0;
	int ret = 0;
	slotid = atoi(argv[0]);
	if((slotid < 1)||(slotid > 16))
	{
		vty_out(vty,"Slot ID is error\n");
		return CMD_SUCCESS;
	}
	int local_id = 0;
	char logfile[64] = {0};	
	snprintf(logfile,sizeof(logfile)-1,"/var/run/slotcli%d.log",slotid);
	char buff[256] = {0};
	FILE *fp = NULL;
	local_id = get_product_info(PRODUCT_LOCAL_SLOTID);
	if(slotid == local_id)
	{
		return eag_show_syslog_cli(vty);
	}
	char cmdstr[128] = {0};
	if(dbus_connection_dcli[slotid]->dcli_dbus_connection)
	{
		ret = dcli_bsd_copy_file_to_board_v2(dbus_connection_dcli[slotid]->dcli_dbus_connection,local_id,SYSLOG_CLI_PATH,logfile,1,0);
	}
	///////
	fp = fopen(logfile,"r");
	if(NULL == fp)
	{
		vty_out(vty,"This Slot has not syslogservice file\n");
		return CMD_SUCCESS;
	}
	while(fgets(buff,sizeof(buff),fp)!=NULL)
	{
		vty_out(vty,"%s",buff);
		memset(buff,0,sizeof(buff));
	}			   
	fclose(fp);
	snprintf(cmdstr,sizeof(cmdstr)-1,"sudo rm %s >/dev/null",logfile);
	system(cmdstr);
	return CMD_FAILURE;
}

DEFUN(show_syslog_all_func,
	show_syslog_all_cmd,
	"show syslog all [SLOTID]",
	SHOW_STR
	"Syslog information\n"
	"Display all keyword\n"
)
{	
	if (0 == argc)
	{
		return eag_show_syslog(vty, FALSE_EAG_VTY, NULL, NULL);
	}
	int slotid = 0;
	int ret = 0;
	slotid = atoi(argv[0]);
	if((slotid < 1)||(slotid > 16))
	{
		vty_out(vty,"Slot ID is error\n");
		return CMD_SUCCESS;
	}
	int local_id = 0;
	char logfile[64] = {0};	
	snprintf(logfile,sizeof(logfile)-1,"/var/run/slot%d.log",slotid);
	char buff[256] = {0};
	FILE *fp = NULL;
	local_id = get_product_info(PRODUCT_LOCAL_SLOTID);
	if(slotid == local_id)
	{
		return eag_show_syslog(vty, FALSE_EAG_VTY, NULL, NULL);
	}
	char cmdstr[128] = {0};
	if(dbus_connection_dcli[slotid]->dcli_dbus_connection)
	{
		ret = dcli_bsd_copy_file_to_board_v2(dbus_connection_dcli[slotid]->dcli_dbus_connection,local_id,SYSLOG_FILE_PATH,logfile,1,0);
	}
	///////
	fp = fopen(logfile,"r");
	if(NULL == fp)
	{
		vty_out(vty,"This Slot has not syslogservice file\n");
		return CMD_SUCCESS;
	}
	while(fgets(buff,sizeof(buff),fp)!=NULL)
	{
		vty_out(vty,"%s",buff);
		memset(buff,0,sizeof(buff));
	}			   
	fclose(fp);
	snprintf(cmdstr,sizeof(cmdstr)-1,"sudo rm %s >/dev/null",logfile);
	system(cmdstr);
	return CMD_FAILURE;
}

DEFUN(show_syslog_line_num_func,
	show_syslog_line_num_cmd,
	"show syslog line-num",
	SHOW_STR
	"Syslog information\n"
	"How many lines in syslog\n"
)
{	
	if (0 == argc)
	{
		return eag_show_syslog(vty, TRUE_EAG_VTY, NULL, NULL);
	}

	return CMD_FAILURE;
}

DEFUN(show_syslog_last_line_func,
	show_syslog_last_line_cmd,
	"show syslog last-line <1-999999>",
	SHOW_STR
	"Syslog information\n"
	"Show the last lines of syslog\n"
	"Line num\n"
)
{	
	if (1 == argc)
	{
		return eag_show_syslog(vty, FALSE_EAG_VTY, (char *)argv[0], NULL);
	}

	return CMD_FAILURE;
}

DEFUN(show_syslog_key_word_func,
	show_syslog_key_word_cmd,
	"show syslog by key-word KEYWORD",
	SHOW_STR
	"Syslog information\n"	
	"By some terms\n"
	"Display syslog include keyword\n"
	"Keyword\n"
)
{	
	if(1 == argc)
	{
		return eag_show_syslog(vty, FALSE_EAG_VTY, NULL, (char *)argv[0]);
	}
	return CMD_FAILURE;
}

DEFUN(show_syslog_time_func,
	show_syslog_time_cmd,
	"show syslog by time STARTTIME ENDTIME",
	SHOW_STR
	"Syslog information\n"
	"By some terms\n"
	"Time\n"
	"Start time.eg: 01:05:59 is 010559\n"
	"End time.eg: 01:05:59 is 010559\n"
)
{	
	int i = 0;
	int time[6];
	char argv_buff[13] = "";
	
	if (2 != argc)
	{
		vty_out(vty,"error, lack of param!");
		return CMD_FAILURE;
	}

	if (6 != strlen(argv[0]) ||6 != strlen(argv[1]))
	{
		vty_out(vty,"param format error. eg: 01:05:59 is 010559");
		return CMD_FAILURE;
	}
	
	memset(time, 0, sizeof(time));
	memset(argv_buff, 0, sizeof(argv_buff));

	snprintf(argv_buff, sizeof(argv_buff), "%s%s",argv[0],argv[1]);

	for (i=0; i<6; i++)
	{
		if ('0' > argv_buff[i*2] || '9' < argv_buff[i*2] 
			|| '0' > argv_buff[i*2+1] || '9' < argv_buff[i*2+1] )
		{
			vty_out(vty,"param format error. eg: 01:05:59 is 010559");
			return CMD_FAILURE;
		}
		
		time[i] = (argv_buff[i*2]-'0')*10 + (argv_buff[i*2+1]-'0');

		if (((0 == i || 3 == i) && time[i] >23)
			|| time[i] >59)
		{
			vty_out(vty,"param format error. eg: 01:05:59 is 010559");
			return CMD_FAILURE;
		}
	}

	return eag_show_syslog_time(vty, time);
	

	return CMD_FAILURE;
}


/*for 2.0  eag config!  */
struct cmd_node hansi_eag_node =
{
	HANSI_EAG_NODE,
	"%s(hansi-eag %d-%d)# ",
	1
};
/*shaojunwu add 201106020*/
struct cmd_node local_hansi_eag_node =
{
	LOCAL_HANSI_EAG_NODE,
	"%s(local-hansi-eag %d-%d)# ",
	1
};

extern int boot_flag;

/*if eag is running, return 1*/
int eag_ins_running_state(DBusConnection * conn, int hansitype, int insid)
{
	int ret = 0;
	struct eag_base_conf baseconf;		
	memset(&baseconf, 0, sizeof(baseconf));
	
	ret = eag_get_base_conf(conn, hansitype, insid, &baseconf);
	if ((EAG_RETURN_OK == ret) && (1 == baseconf.status))
		ret = 1;
	else
		ret = 0;
	return ret;
}

static int
eag_dcli_check_errno (struct vty* vty, int err_no, const char *format, ...)
{
	char buf[4096] = {0};
	va_list args;
	
	if (0 == err_no) {
		return 0;
	}
	va_start (args, format);
	vsnprintf(buf, sizeof(buf), format, args);
	vty_out(vty, "%s: ", buf);
	switch(err_no) {
	case EAG_ERR_DBUS_FAILED:
		vty_out(vty,"dbus wrong");
		break;
	case EAG_ERR_UNKNOWN:
		vty_out(vty,"unknown");
		break;
	case EAG_ERR_NULL_POINTER:
		vty_out(vty,"interval wrong input null param");
		break;
	case EAG_ERR_INPUT_PARAM_ERR:
		vty_out(vty,"function input param");
		break;
	case EAG_ERR_CONFIG_PARAM_OUT_OF_RANGE:
		vty_out(vty,"input param out of range");
		break;
	case EAG_ERR_CONFIG_PARAM_OVER_MAX_VALUE:
		vty_out(vty,"input param over max value");
		break;
	case EAG_ERR_CONFIG_SET_INTERVAL_ERR:
		vty_out(vty,"eag set param interval wrong");
		break;
	case EAG_ERR_CONFIG_ITEM_NOT_FOUND:
		vty_out(vty,"eag no such param");
		break;
	case EAG_ERR_CONFIG_ITEM_PARAM_CONFLICT:
		vty_out(vty,"eag input param conflict");
		break;
	default:
		vty_out(vty,"unknown errno %d",err_no);
		break;
	}

	va_end (args);
	return 1;
}

#if 1
/* eag show running */
#endif

int
eag_multi_portal_config_show_running(struct vty* vty)
{
	int ret = -1;
	int i = 0;
	char showStr[SHOW_STR_LEN] = "";
	char key_type[20] = "";
	char key[128] = "";
	struct portal_conf portalconf;
	char ip_str[32] = "";
	memset(key_type, 0, sizeof(key_type));
	memset(key, 0, sizeof(key));
	memset(showStr, 0, sizeof(showStr));		
	memset( &portalconf, 0, sizeof(struct portal_conf) );

	ret = eag_get_portal_conf( dcli_dbus_connection, 
								HANSI_LOCAL, 0,
								&portalconf );
	
	if (EAG_RETURN_OK == ret) {		
		for (i = 0; i < portalconf.current_num; i++) {
			switch (portalconf.portal_srv[i].key_type) {						
			case PORTAL_KEYTYPE_ESSID:
				strncpy(key_type, "essid", sizeof(key_type)-1);
				strncpy(key, portalconf.portal_srv[i].key.essid, sizeof(key)-1);
				break;
			case PORTAL_KEYTYPE_WLANID:
				strncpy(key_type, "wlanid", sizeof(key_type)-1);
				snprintf(key, sizeof(key)-1, "%lu", portalconf.portal_srv[i].key.wlanid);
				break;
			case PORTAL_KEYTYPE_VLANID:
				strncpy(key_type, "vlanid", sizeof(key_type)-1);
				snprintf(key, sizeof(key)-1, "%lu", portalconf.portal_srv[i].key.vlanid);
				break;
			case PORTAL_KEYTYPE_WTPID:
				strncpy(key_type, "wtpid", sizeof(key_type)-1);
				snprintf(key, sizeof(key)-1, "%lu", portalconf.portal_srv[i].key.wtpid);
				break;
			case PORTAL_KEYTYPE_INTF:
				strncpy(key_type, "interface", sizeof(key_type)-1);
				strncpy(key, portalconf.portal_srv[i].key.intf, sizeof(key)-1);
				break;
			default:
				break;
			}
			
			if (0 != strcmp(portalconf.portal_srv[i].domain, "")
				&& (0 != portalconf.portal_srv[i].mac_server_ip 
					|| 0 != portalconf.portal_srv[i].mac_server_port))
			{
				if (MACBIND_SERVER_IP == portalconf.portal_srv[i].ip_or_domain)
				{
					snprintf(showStr, sizeof(showStr)-1, 
						" add portal-server %s %s %s %u domain %s",
						key_type, key, portalconf.portal_srv[i].portal_url, 
						portalconf.portal_srv[i].ntf_port, portalconf.portal_srv[i].domain);
					vtysh_add_show_string(showStr);
					
					ip2str(portalconf.portal_srv[i].mac_server_ip, ip_str, sizeof(ip_str));
					snprintf(showStr, sizeof(showStr)-1, 
						" set macbind-server %s %s ip %s %u", 
						key_type, key, ip_str, 
						portalconf.portal_srv[i].mac_server_port);
					vtysh_add_show_string(showStr);
				}
				else if (MACBIND_SERVER_DOMAIN == portalconf.portal_srv[i].ip_or_domain)
				{
					snprintf(showStr, sizeof(showStr)-1, 
						" add portal-server %s %s %s %u domain %s",
						key_type, key, portalconf.portal_srv[i].portal_url, 
						portalconf.portal_srv[i].ntf_port, portalconf.portal_srv[i].domain);
					vtysh_add_show_string(showStr);
					
					snprintf(showStr, sizeof(showStr)-1, 
						" set macbind-server %s %s domain %s %u", 
						key_type, key, portalconf.portal_srv[i].macbind_server_domain, 
						portalconf.portal_srv[i].mac_server_port);
					vtysh_add_show_string(showStr);
				}
				else 
				{
                    ip2str(portalconf.portal_srv[i].mac_server_ip, ip_str, sizeof(ip_str));
                    snprintf(showStr, sizeof(showStr)-1, 
                        " add portal-server %s %s %s %u domain %s macauth %s %u", 
                        key_type, key, portalconf.portal_srv[i].portal_url,
                        portalconf.portal_srv[i].ntf_port, portalconf.portal_srv[i].domain,
                        ip_str, portalconf.portal_srv[i].mac_server_port);
                    vtysh_add_show_string(showStr);
				}
			} 
			else if (0 != strcmp(portalconf.portal_srv[i].domain, ""))
			{
				snprintf(showStr, sizeof(showStr)-1, 
					" add portal-server %s %s %s %u domain %s",
					key_type, key, portalconf.portal_srv[i].portal_url, 
					portalconf.portal_srv[i].ntf_port, portalconf.portal_srv[i].domain);
				vtysh_add_show_string(showStr);
			}
			else if (0 != portalconf.portal_srv[i].mac_server_ip 
					|| 0 != portalconf.portal_srv[i].mac_server_port)
			{
				if (MACBIND_SERVER_IP == portalconf.portal_srv[i].ip_or_domain)
				{
                    snprintf(showStr, sizeof(showStr)-1, 
                        " add portal-server %s %s %s %u", 
                        key_type, key, portalconf.portal_srv[i].portal_url,
                        portalconf.portal_srv[i].ntf_port);
                    vtysh_add_show_string(showStr);
					
					ip2str(portalconf.portal_srv[i].mac_server_ip, ip_str, sizeof(ip_str));
					snprintf(showStr, sizeof(showStr)-1, 
						" set macbind-server %s %s ip %s %u", 
						key_type, key, ip_str, 
						portalconf.portal_srv[i].mac_server_port);
					vtysh_add_show_string(showStr);
				}
				else if (MACBIND_SERVER_DOMAIN == portalconf.portal_srv[i].ip_or_domain)
				{
                    snprintf(showStr, sizeof(showStr)-1, 
                        " add portal-server %s %s %s %u", 
                        key_type, key, portalconf.portal_srv[i].portal_url,
                        portalconf.portal_srv[i].ntf_port);
                    vtysh_add_show_string(showStr);

					snprintf(showStr, sizeof(showStr)-1, 
						" set macbind-server %s %s domain %s %u", 
						key_type, key, portalconf.portal_srv[i].macbind_server_domain, 
						portalconf.portal_srv[i].mac_server_port);
					vtysh_add_show_string(showStr);
				} else {
					ip2str(portalconf.portal_srv[i].mac_server_ip, ip_str, sizeof(ip_str));
					snprintf(showStr, sizeof(showStr)-1, 
						" add portal-server %s %s %s %u macauth %s %u", 
						key_type, key, portalconf.portal_srv[i].portal_url,
						portalconf.portal_srv[i].ntf_port,
						ip_str, portalconf.portal_srv[i].mac_server_port);
					vtysh_add_show_string(showStr);
				}
			}
			else  {
				snprintf(showStr, sizeof(showStr)-1, 
					" add portal-server %s %s %s %u", 
					key_type, key, portalconf.portal_srv[i].portal_url,
					portalconf.portal_srv[i].ntf_port);
				vtysh_add_show_string(showStr);
			}

			if( 0 != strcmp(portalconf.portal_srv[i].acname, ""))
			{
				snprintf(showStr, sizeof(showStr)-1, " set portal-server %s %s ac-name %s",\
													key_type, key, portalconf.portal_srv[i].acname);
				vtysh_add_show_string(showStr);
			}

			if( 1 == portalconf.portal_srv[i].acip_to_url)
			{
				snprintf(showStr, sizeof(showStr)-1, " set portal-server %s %s acip-to-url enable",\
													key_type, key);
				vtysh_add_show_string(showStr);
			}
			if( 1 == portalconf.portal_srv[i].nasid_to_url)
			{
				snprintf(showStr, sizeof(showStr)-1, " set portal-server %s %s nasid-to-url enable",\
													key_type, key);
				vtysh_add_show_string(showStr);
			}
			if( 1 == portalconf.portal_srv[i].usermac_to_url)
			{
				snprintf(showStr, sizeof(showStr)-1, " set portal-server %s %s usermac-to-url enable",\
													key_type, key);
				vtysh_add_show_string(showStr);
			}		
			if( 1 == portalconf.portal_srv[i].clientmac_to_url)
			{
				snprintf(showStr, sizeof(showStr)-1, " set portal-server %s %s clientmac_to_url enable",\
													key_type, key);
				vtysh_add_show_string(showStr);
			}			
			if( 1 == portalconf.portal_srv[i].apmac_to_url)
			{
				snprintf(showStr, sizeof(showStr)-1, " set portal-server %s %s apmac_to_url enable",\
													key_type, key);
				vtysh_add_show_string(showStr);
			}			
			if( 1 == portalconf.portal_srv[i].wlan_to_url)
			{
				snprintf(showStr, sizeof(showStr)-1, " set portal-server %s %s wlan_to_url enable",\
													key_type, key);
				vtysh_add_show_string(showStr);
			}			
			if( 1 == portalconf.portal_srv[i].redirect_to_url)
			{
				snprintf(showStr, sizeof(showStr)-1, " set portal-server %s %s redirect_to_url enable",\
													key_type, key);
				vtysh_add_show_string(showStr);
			}
			if( 1 == portalconf.portal_srv[i].wlanparameter 
				&& strlen(portalconf.portal_srv[i].deskey)>0)
			{
				snprintf(showStr, sizeof(showStr)-1, 
							" set portal-server %s %s wlanparameter enable %s",
							key_type, key, portalconf.portal_srv[i].deskey );
				vtysh_add_show_string(showStr);
			}

			if( 1 == portalconf.portal_srv[i].wlanuserfirsturl )
			{
				snprintf(showStr, sizeof(showStr)-1, 
							" set portal-server %s %s wlanuserfirsturl enable",\
							key_type, key);
				vtysh_add_show_string(showStr);
			}
			if( 0 != strcmp(portalconf.portal_srv[i].url_suffix, ""))
			{
				snprintf(showStr, sizeof(showStr)-1, " set portal-server %s %s url-suffix %s",\
													key_type, key, portalconf.portal_srv[i].url_suffix);
				vtysh_add_show_string(showStr);
			}

			if( 0 != strcmp(portalconf.portal_srv[i].secret, ""))
			{
				snprintf(showStr, sizeof(showStr)-1, " set portal-server %s %s secret %s",\
													key_type, key, portalconf.portal_srv[i].secret);
				vtysh_add_show_string(showStr);
			}
			if( 1 == portalconf.portal_srv[i].wlanapmac)
			{
				snprintf(showStr, sizeof(showStr)-1, " set portal-server %s %s wlanapmac enable",\
													key_type, key);
				vtysh_add_show_string(showStr);
			}

			if( 1 == portalconf.portal_srv[i].wlanusermac
				&& strlen(portalconf.portal_srv[i].wlanusermac_deskey)>0)
			{
				snprintf(showStr, sizeof(showStr)-1, 
							" set portal-server %s %s wlanusermac enable %s",
							key_type, key, portalconf.portal_srv[i].wlanusermac_deskey );
				vtysh_add_show_string(showStr);
			}
			
			if( WISPR_URL_HTTP == portalconf.portal_srv[i].wisprlogin )
			{
				snprintf(showStr, sizeof(showStr)-1, " set portal-server %s %s wisprlogin enable http",\
													key_type, key);
				vtysh_add_show_string(showStr);
			}else if( WISPR_URL_HTTPS == portalconf.portal_srv[i].wisprlogin ){
				snprintf(showStr, sizeof(showStr)-1, " set portal-server %s %s wisprlogin enable https",\
													key_type, key);
				vtysh_add_show_string(showStr);
			}

			if( 1 == portalconf.portal_srv[i].mobile_urlparam ) {
				snprintf(showStr, sizeof(showStr)-1, 
							" set portal-server %s %s mobile-urlparam disable", key_type, key);
				vtysh_add_show_string(showStr);
			}
			
			if( 1 == portalconf.portal_srv[i].urlparam_add
				&& strlen(portalconf.portal_srv[i].save_urlparam_config)>0)
			{
				snprintf(showStr, sizeof(showStr)-1, 
							" set portal-server %s %s urlparam-add %s",
							key_type, key, portalconf.portal_srv[i].save_urlparam_config);
				vtysh_add_show_string(showStr);
			}
		}
	}

return CMD_SUCCESS;
}

char *
eag_multi_portal_config_show_running_2(int localid, int slot_id, int index)
{
	int ret = -1;
	int i = 0;
	char showStr[SHOW_STR_LEN*30];
	char key_type[20];
	char key[128];
	struct portal_conf portalconf;
	memset(key_type, 0, sizeof(key_type));
	memset(key, 0, sizeof(key));
	memset(showStr, 0, sizeof(showStr));		
	memset( &portalconf, 0, sizeof(struct portal_conf) );
	char *tmp = NULL;
	DBusConnection *dcli_dbus_connection_curr = NULL;
	char *cursor = NULL;
	int totalLen = 0;
	char ip_str[32] = "";
	cursor = showStr;
	
	ReInitDbusConnection(&dcli_dbus_connection_curr, slot_id, distributFag);

	ret = eag_get_portal_conf(dcli_dbus_connection_curr, 
								localid, index,
								&portalconf);
	
	if (EAG_RETURN_OK == ret) {
		for (i = 0; i < portalconf.current_num; i++) {
			switch(portalconf.portal_srv[i].key_type) {						
			case PORTAL_KEYTYPE_ESSID:
				strncpy(key_type, "essid", sizeof(key_type)-1);
				strncpy(key, portalconf.portal_srv[i].key.essid, sizeof(key)-1);
				break;
			case PORTAL_KEYTYPE_WLANID:
				strncpy(key_type, "wlanid", sizeof(key_type)-1);
				snprintf(key, sizeof(key)-1, "%lu", portalconf.portal_srv[i].key.wlanid);
				break;
			case PORTAL_KEYTYPE_VLANID:
				strncpy(key_type, "vlanid", sizeof(key_type)-1);
				snprintf(key, sizeof(key)-1, "%lu", portalconf.portal_srv[i].key.vlanid);
				break;
			case PORTAL_KEYTYPE_WTPID:
				strncpy(key_type, "wtpid", sizeof(key_type)-1);
				snprintf(key, sizeof(key)-1, "%lu", portalconf.portal_srv[i].key.wtpid);
				break;
			case PORTAL_KEYTYPE_INTF:
				strncpy(key_type, "interface", sizeof(key_type)-1);
				strncpy(key, portalconf.portal_srv[i].key.intf, sizeof(key)-1);
				break;
			default:
				break;
			}

			if (0 != strcmp(portalconf.portal_srv[i].domain, "")
				&& (0 != portalconf.portal_srv[i].mac_server_ip 
					|| 0 != portalconf.portal_srv[i].mac_server_port))
			{
				if (MACBIND_SERVER_IP == portalconf.portal_srv[i].ip_or_domain)
				{
					totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1," add portal-server %s %s %s %u domain %s\n",
									key_type, key, portalconf.portal_srv[i].portal_url, 
									portalconf.portal_srv[i].ntf_port, portalconf.portal_srv[i].domain);

					ip2str(portalconf.portal_srv[i].mac_server_ip, ip_str, sizeof(ip_str));
					totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, 
									" set macbind-server %s %s ip %s %u\n", 
									key_type, key, ip_str, 
									portalconf.portal_srv[i].mac_server_port);
				}
				else if (MACBIND_SERVER_DOMAIN == portalconf.portal_srv[i].ip_or_domain)
				{
					totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1," add portal-server %s %s %s %u domain %s\n",
									key_type, key, portalconf.portal_srv[i].portal_url, 
									portalconf.portal_srv[i].ntf_port, portalconf.portal_srv[i].domain);

					totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, 
									" set macbind-server %s %s domain %s %u\n", 
									key_type, key, portalconf.portal_srv[i].macbind_server_domain, 
									portalconf.portal_srv[i].mac_server_port);
				}
				else 
				{
					ip2str(portalconf.portal_srv[i].mac_server_ip, ip_str, sizeof(ip_str));
					totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1," add portal-server %s %s %s %u domain %s macauth %s %u\n",
									key_type, key, portalconf.portal_srv[i].portal_url, 
									portalconf.portal_srv[i].ntf_port, portalconf.portal_srv[i].domain,
									ip_str, portalconf.portal_srv[i].mac_server_port);
				}
			} 
			else if (0 != strcmp(portalconf.portal_srv[i].domain, ""))
			{
				totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1," add portal-server %s %s %s %u domain %s\n",
								key_type, key, portalconf.portal_srv[i].portal_url, 
								portalconf.portal_srv[i].ntf_port, portalconf.portal_srv[i].domain);
			}
			else if (0 != portalconf.portal_srv[i].mac_server_ip 
					|| 0 != portalconf.portal_srv[i].mac_server_port)
			{
				if (MACBIND_SERVER_IP == portalconf.portal_srv[i].ip_or_domain)
				{
                    totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " add portal-server %s %s %s %u\n",
                                    key_type, key, portalconf.portal_srv[i].portal_url, 
                                    portalconf.portal_srv[i].ntf_port);

					ip2str(portalconf.portal_srv[i].mac_server_ip, ip_str, sizeof(ip_str));
					totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, 
									" set macbind-server %s %s ip %s %u\n", 
									key_type, key, ip_str, 
									portalconf.portal_srv[i].mac_server_port);
				}
				else if (MACBIND_SERVER_DOMAIN == portalconf.portal_srv[i].ip_or_domain)
				{
                    totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " add portal-server %s %s %s %u\n",
                                    key_type, key, portalconf.portal_srv[i].portal_url, 
                                    portalconf.portal_srv[i].ntf_port);

					totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, 
									" set macbind-server %s %s domain %s %u\n", 
									key_type, key, portalconf.portal_srv[i].macbind_server_domain, 
									portalconf.portal_srv[i].mac_server_port);
				} else {
					ip2str(portalconf.portal_srv[i].mac_server_ip, ip_str, sizeof(ip_str));
					totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1," add portal-server %s %s %s %u macauth %s %u\n",
									key_type, key, portalconf.portal_srv[i].portal_url, 
									portalconf.portal_srv[i].ntf_port,
									ip_str, portalconf.portal_srv[i].mac_server_port);
				}
			}
			else {
				totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " add portal-server %s %s %s %u\n",
								key_type, key, portalconf.portal_srv[i].portal_url, 
								portalconf.portal_srv[i].ntf_port);
			}

			if( 0 != strcmp(portalconf.portal_srv[i].acname, ""))
			{
				totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " set portal-server %s %s ac-name %s\n", key_type, key, portalconf.portal_srv[i].acname);
			}
			
			if( 1 == portalconf.portal_srv[i].acip_to_url)
			{
				totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " set portal-server %s %s acip-to-url enable\n", key_type, key);
			}			
			if( 1 == portalconf.portal_srv[i].nasid_to_url)
			{
				totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " set portal-server %s %s nasid-to-url enable\n", key_type, key);
			}
			if( 1 == portalconf.portal_srv[i].wlanparameter 
				&& strlen(portalconf.portal_srv[i].deskey)>0)
			{
				totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, 
							" set portal-server %s %s wlanparameter enable %s\n",
							key_type, key, portalconf.portal_srv[i].deskey );
			}

			if( 1 == portalconf.portal_srv[i].wlanuserfirsturl )
			{
				totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, 
							" set portal-server %s %s wlanuserfirsturl enable\n",\
							key_type, key);
			}
			if( 0 != strcmp(portalconf.portal_srv[i].url_suffix, ""))
			{
				totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " set portal-server %s %s url-suffix %s\n", key_type, key, portalconf.portal_srv[i].url_suffix);
			}

			if( 0 != strcmp(portalconf.portal_srv[i].secret, ""))
			{
				totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " set portal-server %s %s secret %s\n", key_type, key, portalconf.portal_srv[i].secret);
			}
			if( 1 == portalconf.portal_srv[i].usermac_to_url)
			{
				totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " set portal-server %s %s usermac-to-url enable\n", key_type, key);
			}
			if( 1 == portalconf.portal_srv[i].clientmac_to_url)
			{
				totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " set portal-server %s %s clientmac-to-url enable\n", key_type, key);
			}
			if( 1 == portalconf.portal_srv[i].apmac_to_url)
			{
				totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " set portal-server %s %s apmac-to-url enable\n", key_type, key);
			}
			if( 1 == portalconf.portal_srv[i].wlan_to_url)
			{
				totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " set portal-server %s %s wlan-to-url enable\n", key_type, key);
			}
			if( 1 == portalconf.portal_srv[i].redirect_to_url)
			{
				totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " set portal-server %s %s redirect-to-url enable\n", key_type, key);
			}
			if( 1 == portalconf.portal_srv[i].wlanapmac)
			{
				totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " set portal-server %s %s wlanapmac enable\n", key_type, key);
			}

			if( 1 == portalconf.portal_srv[i].wlanusermac
				&& strlen(portalconf.portal_srv[i].wlanusermac_deskey)>0)
			{
				totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, 
							" set portal-server %s %s wlanusermac enable %s\n",
							key_type, key, portalconf.portal_srv[i].wlanusermac_deskey );
			}

			if( WISPR_URL_HTTP == portalconf.portal_srv[i].wisprlogin )
			{
				totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, 
										" set portal-server %s %s wisprlogin enable http\n",\
										key_type, key);
			}else if( WISPR_URL_HTTPS == portalconf.portal_srv[i].wisprlogin ){
				totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1,
										" set portal-server %s %s wisprlogin enable https\n",\
										key_type, key);
			}			

			if( 1 == portalconf.portal_srv[i].mobile_urlparam ) {
				totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, 
							" set portal-server %s %s mobile-urlparam disable\n", key_type, key);
			}
			
			if( 1 == portalconf.portal_srv[i].urlparam_add
				&& strlen(portalconf.portal_srv[i].save_urlparam_config)>0)
			{
				totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, 
							" set portal-server %s %s urlparam-add %s\n",
							key_type, key, portalconf.portal_srv[i].save_urlparam_config );
			}
		}
	}
#if 0	
	if (0 == strlen(showStr)){
		return "";// might case core dump!!
	}
#endif

	tmp = malloc(strlen(showStr) + 1);
	if (NULL == tmp) {
		return NULL;
	}
	memset(tmp, 0, strlen(showStr) + 1);
	memcpy(tmp, showStr, strlen(showStr));
	return tmp;
}

int
eag_radius_config_show_running(struct vty* vty)
{
	int ret = -1;
	int i = 0;
	char *domain = NULL;
	struct radius_conf radiusconf;
	char showStr[SHOW_STR_LEN];
	char auth_ip[32];
	char acct_ip[32];
	char backup_auth_ip[32];
	char backup_acct_ip[32];

	memset(&radiusconf, 0, sizeof(struct radius_conf));
	memset(showStr, 0, sizeof(showStr));
	memset(auth_ip, 0, sizeof(auth_ip));
	memset(acct_ip, 0, sizeof(acct_ip));
	memset(backup_auth_ip, 0, sizeof(backup_auth_ip));
	memset(backup_acct_ip, 0, sizeof(backup_acct_ip));	
	
	ret = eag_get_radius_conf(dcli_dbus_connection, 
								HANSI_LOCAL, 0, 
								domain,
								&radiusconf);
	if (EAG_RETURN_OK == ret) {
		for (i = 0; i < radiusconf.current_num; i++) {
			ip2str( radiusconf.radius_srv[i].auth_ip, auth_ip, sizeof(auth_ip));
			ip2str( radiusconf.radius_srv[i].acct_ip, acct_ip, sizeof(acct_ip));
			
			if( 0!=radiusconf.radius_srv[i].backup_auth_ip )				
			{
				ip2str( radiusconf.radius_srv[i].backup_auth_ip, backup_auth_ip, sizeof(backup_auth_ip));
				ip2str( radiusconf.radius_srv[i].backup_acct_ip, backup_acct_ip, sizeof(backup_acct_ip));
				snprintf(showStr, sizeof(showStr), " add radius-server %s auth %s %u %s acct %s %u %s backup-auth %s %u %s backup-acct %s %u %s",\
														radiusconf.radius_srv[i].domain,\
														auth_ip,\
														radiusconf.radius_srv[i].auth_port,\
														radiusconf.radius_srv[i].auth_secret,\
														acct_ip,\
														radiusconf.radius_srv[i].acct_port,\
														radiusconf.radius_srv[i].acct_secret,\
														backup_auth_ip,\
														radiusconf.radius_srv[i].backup_auth_port,\
														radiusconf.radius_srv[i].backup_auth_secret,\
														backup_acct_ip,\
														radiusconf.radius_srv[i].backup_acct_port,\
														radiusconf.radius_srv[i].backup_acct_secret);
				vtysh_add_show_string(showStr);
			}
			else {
				snprintf(showStr, sizeof(showStr), " add radius-server %s auth %s %u %s acct %s %u %s",\
														radiusconf.radius_srv[i].domain,\
														auth_ip,\
														radiusconf.radius_srv[i].auth_port,\
														radiusconf.radius_srv[i].auth_secret,\
														acct_ip,\
														radiusconf.radius_srv[i].acct_port,\
														radiusconf.radius_srv[i].acct_secret,\
														backup_auth_ip,\
														radiusconf.radius_srv[i].backup_auth_port,\
														radiusconf.radius_srv[i].backup_auth_secret,\
														backup_acct_ip,\
														radiusconf.radius_srv[i].backup_acct_port,\
														radiusconf.radius_srv[i].backup_acct_secret);
				vtysh_add_show_string(showStr);
			}
			
			if (0 != radiusconf.radius_srv[i].remove_domain_name) {
				snprintf(showStr, sizeof(showStr), " set radius %s remove-domain-name enable", radiusconf.radius_srv[i].domain);
				vtysh_add_show_string(showStr);
			}
			if (0 != radiusconf.radius_srv[i].class_to_bandwidth) {
				snprintf(showStr, sizeof(showStr), " set radius %s class-to-bandwidth enable", radiusconf.radius_srv[i].domain);
				vtysh_add_show_string(showStr);
			}
		}
	}
	return CMD_SUCCESS;
}

char *
eag_radius_config_show_running_2(int localid, int slot_id,int index)
{
	int ret = -1;
	int i = 0;
	char *domain = NULL;
	struct radius_conf radiusconf;
	char *tmp = NULL;
	char showStr[SHOW_STR_LEN*5];
	char auth_ip[32];
	char acct_ip[32];
	char backup_auth_ip[32];
	char backup_acct_ip[32];
	DBusConnection *dcli_dbus_connection_curr = NULL;
	char *cursor = NULL;
	int totalLen = 0;
	memset(showStr, 0, sizeof(showStr));
	cursor = showStr;
	totalLen = 0;
	
	ReInitDbusConnection(&dcli_dbus_connection_curr, slot_id, distributFag);
	memset(&radiusconf, 0, sizeof(struct radius_conf));
	memset(showStr, 0, sizeof(showStr));
	memset(auth_ip, 0, sizeof(auth_ip));
	memset(acct_ip, 0, sizeof(acct_ip));
	memset(backup_auth_ip, 0, sizeof(backup_auth_ip));
	memset(backup_acct_ip, 0, sizeof(backup_acct_ip));
	
	ret = eag_get_radius_conf(dcli_dbus_connection_curr, 
								localid, index, 
								domain,
								&radiusconf);
	if (EAG_RETURN_OK == ret) {
		for(i = 0; i < radiusconf.current_num; i++) {
			ip2str(radiusconf.radius_srv[i].auth_ip, auth_ip, sizeof(auth_ip));
			ip2str( radiusconf.radius_srv[i].acct_ip, acct_ip, sizeof(acct_ip));
			
			if( 0!=radiusconf.radius_srv[i].backup_auth_ip )				
			{
				ip2str( radiusconf.radius_srv[i].backup_auth_ip, backup_auth_ip, sizeof(backup_auth_ip));
				ip2str( radiusconf.radius_srv[i].backup_acct_ip, backup_acct_ip, sizeof(backup_acct_ip));
				totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " add radius-server %s auth %s %u %s acct %s %u %s backup-auth %s %u %s backup-acct %s %u %s\n",\
														radiusconf.radius_srv[i].domain,\
														auth_ip,\
														radiusconf.radius_srv[i].auth_port,\
														radiusconf.radius_srv[i].auth_secret,\
														acct_ip,\
														radiusconf.radius_srv[i].acct_port,\
														radiusconf.radius_srv[i].acct_secret,\
														backup_auth_ip,\
														radiusconf.radius_srv[i].backup_auth_port,\
														radiusconf.radius_srv[i].backup_auth_secret,\
														backup_acct_ip,\
														radiusconf.radius_srv[i].backup_acct_port,\
														radiusconf.radius_srv[i].backup_acct_secret);
			}
			else {
				totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " add radius-server %s auth %s %u %s acct %s %u %s\n",\
														radiusconf.radius_srv[i].domain,\
														auth_ip,\
														radiusconf.radius_srv[i].auth_port,\
														radiusconf.radius_srv[i].auth_secret,\
														acct_ip,\
														radiusconf.radius_srv[i].acct_port,\
														radiusconf.radius_srv[i].acct_secret,\
														backup_auth_ip,\
														radiusconf.radius_srv[i].backup_auth_port,\
														radiusconf.radius_srv[i].backup_auth_secret,\
														backup_acct_ip,\
														radiusconf.radius_srv[i].backup_acct_port,\
														radiusconf.radius_srv[i].backup_acct_secret);
			}
			if (0 != radiusconf.radius_srv[i].remove_domain_name) {
				totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " set radius %s remove-domain-name enable\n", radiusconf.radius_srv[i].domain);
			}
			if (0 != radiusconf.radius_srv[i].class_to_bandwidth) {
				totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " set radius %s class-to-bandwidth enable\n", radiusconf.radius_srv[i].domain);
			}
		}
	}

#if 0	
		if (0 == strlen(showStr)){
			return "";// might case core dump!!
		}
#endif

	tmp = malloc(strlen(showStr) + 1);
	if (NULL == tmp){
		return NULL;
	}
	memset(tmp, 0, strlen(showStr) + 1);
	memcpy(tmp, showStr, strlen(showStr));
	return tmp;
}

int
eag_captive_portal_config_show_running(struct vty* vty)
{
	/* add interface */
	int ret = -1;
	int i = 0;
	char showStr[SHOW_STR_LEN];
	eag_captive_intfs captive_intfs;
	memset(showStr, 0, sizeof(showStr));	
	memset(&captive_intfs, 0, sizeof(captive_intfs));	
	
	ret = eag_get_captive_intfs(dcli_dbus_connection,
						HANSI_LOCAL, 0, &captive_intfs);
	
	if (EAG_RETURN_OK == ret) {		
		if (captive_intfs.curr_ifnum > 0) {
			for (i = 0; i < captive_intfs.curr_ifnum; i++) {
				snprintf(showStr, sizeof(showStr), " add captive-interface %s", captive_intfs.cpif[i]);
				vtysh_add_show_string(showStr);
			}
		}		
	}

	/* add white list */
	int type_tmp = -1;
	char *type = NULL;
	struct bw_rules white;
	char ipbegin[32] = {0};
	char ipend[32] = {0};
	memset(&white, 0, sizeof(white));
	
	ret = eag_show_white_list(dcli_dbus_connection,
						HANSI_LOCAL, 0, &white);
	if (EAG_RETURN_OK == ret) {	
		if (white.curr_num > 0) {			
			for (i = 0; i < white.curr_num; i++) {
				if (white.rule[i].type == RULE_IPADDR)
					type = "IP";
				else if (white.rule[i].type == RULE_DOMAIN)
					type = "Domain";

				ip2str(white.rule[i].key.ip.ipbegin, ipbegin, sizeof(ipbegin));
				ip2str(white.rule[i].key.ip.ipend, ipend, sizeof(ipend));

				type_tmp = white.rule[i].type;
				if((RULE_TYPE)type_tmp == RULE_IPADDR)
				{
					if(strcmp(ipend, "0.0.0.0") == 0)
					{
						if(strcmp(white.rule[i].intf, "") == 0)
						{
							snprintf(showStr, sizeof(showStr), " add white-list ip %s:%s", ipbegin ,white.rule[i].key.ip.ports);
							vtysh_add_show_string(showStr);
						}
						else
						{
							snprintf(showStr, sizeof(showStr), " add white-list ip %s:%s %s",\
									ipbegin ,white.rule[i].key.ip.ports,white.rule[i].intf);
							vtysh_add_show_string(showStr);
						}
					}
					else
					{
						if(strcmp(white.rule[i].intf, "") == 0)
						{
							snprintf(showStr, sizeof(showStr), " add white-list ip %s-%s:%s", ipbegin ,ipend, white.rule[i].key.ip.ports);
							vtysh_add_show_string(showStr);
						}
						else
						{
							snprintf(showStr, sizeof(showStr), " add white-list ip %s-%s:%s %s",\
									ipbegin ,ipend, white.rule[i].key.ip.ports,white.rule[i].intf);
							vtysh_add_show_string(showStr);
						}
					}
					
				}
				else if((RULE_TYPE)type_tmp == RULE_DOMAIN )
				{
					if(strcmp(white.rule[i].intf, "") == 0)
					{
						snprintf(showStr, sizeof(showStr), " add white-list domain %s",\
									white.rule[i].key.domain.name);
						vtysh_add_show_string(showStr);
					}
					else
					{
						snprintf(showStr, sizeof(showStr), " add white-list domain %s %s",\
									white.rule[i].key.domain.name, white.rule[i].intf);
						vtysh_add_show_string(showStr);
					}					
				}
			}
		}
	}

	/* add black-list */
	struct bw_rules black;
	ret = eag_show_black_list(dcli_dbus_connection,
						HANSI_LOCAL, 0, &black);
	if (EAG_RETURN_OK == ret) {	
		if (black.curr_num > 0) {
			for (i = 0; i < black.curr_num; i++) {
				if (black.rule[i].type == RULE_IPADDR)
					type = "IP";
				else if (black.rule[i].type == RULE_DOMAIN)
					type = "Domain";

				ip2str(black.rule[i].key.ip.ipbegin, ipbegin, sizeof(ipbegin));
				ip2str(black.rule[i].key.ip.ipend, ipend, sizeof(ipend));

				type_tmp = black.rule[i].type;
				if((RULE_TYPE)type_tmp == RULE_IPADDR)
				{
					if(strcmp(ipend, "0.0.0.0") == 0)
					{
						if(strcmp(black.rule[i].intf, "") == 0)
						{
							snprintf(showStr, sizeof(showStr), " add black-list ip %s:%s", ipbegin ,black.rule[i].key.ip.ports);
							vtysh_add_show_string(showStr);
						}
						else
						{
							snprintf(showStr, sizeof(showStr), " add black-list ip %s:%s %s",\
									ipbegin ,black.rule[i].key.ip.ports,black.rule[i].intf);
							vtysh_add_show_string(showStr);
						}
					}
					else
					{
						if(strcmp(black.rule[i].intf, "") == 0)
						{
							snprintf(showStr, sizeof(showStr), " add black-list ip %s-%s:%s", ipbegin ,ipend, black.rule[i].key.ip.ports);
							vtysh_add_show_string(showStr);
						}
						else
						{
							snprintf(showStr, sizeof(showStr), "add black-list ip %s-%s:%s %s",\
									ipbegin ,ipend, black.rule[i].key.ip.ports,black.rule[i].intf);
							vtysh_add_show_string(showStr);
						}
					}
				}
				else if((RULE_TYPE)type_tmp == RULE_DOMAIN)
				{
					if(strcmp(black.rule[i].intf, "") == 0)
					{
						snprintf(showStr, sizeof(showStr), " add black-list domain %s",\
									black.rule[i].key.domain.name);
						vtysh_add_show_string(showStr);
					}
					else
					{
						snprintf(showStr, sizeof(showStr), "add black-list domain %s %s",\
									black.rule[i].key.domain.name, black.rule[i].intf);
						vtysh_add_show_string(showStr);
					}					
				}
			}
		}
	}
}

char *
eag_captive_portal_config_show_running_2(int localid, int slot_id,int index)
{
	/* add interface */
	int ret = -1;
	int i = 0;
	char showStr[SHOW_STR_LEN*30];
	eag_captive_intfs captive_intfs;
	memset(showStr, 0, sizeof(showStr));	
	memset(&captive_intfs, 0, sizeof(captive_intfs));
	char *tmp = NULL;
	DBusConnection *dcli_dbus_connection_curr = NULL;
	char *cursor = NULL;
	int totalLen = 0;
	
	memset(showStr, 0, sizeof(showStr));
	cursor = showStr;
	totalLen = 0;
	
	ReInitDbusConnection(&dcli_dbus_connection_curr, slot_id, distributFag);
	
	ret = eag_get_captive_intfs( dcli_dbus_connection_curr,
						localid, index, &captive_intfs );
	
	if (EAG_RETURN_OK == ret) {
		if (captive_intfs.curr_ifnum > 0) {
			for (i=0; i < captive_intfs.curr_ifnum; i++) {
				totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " add captive-interface %s\n", captive_intfs.cpif[i]);
			}
		}		
	}

	/* add white list */
	int type_tmp = -1;
	char *type = NULL;
	struct bw_rules white;
	char ipbegin[32] = {0};
	char ipend[32] = {0};
	memset(&white, 0, sizeof(white));
	
	ret = eag_show_white_list(dcli_dbus_connection_curr,
						localid, index, &white);
	if (EAG_RETURN_OK == ret) {	
		if (white.curr_num > 0) {			
			for (i=0; i < white.curr_num; i++) {
				if(white.rule[i].type == RULE_IPADDR)
					type = "IP";
				else if (white.rule[i].type == RULE_DOMAIN)
					type = "Domain";

				ip2str(white.rule[i].key.ip.ipbegin, ipbegin, sizeof(ipbegin));
				ip2str(white.rule[i].key.ip.ipend, ipend, sizeof(ipend));

				type_tmp = white.rule[i].type;
				if( (RULE_TYPE)type_tmp == RULE_IPADDR )
				{
					if(strcmp(ipend, "0.0.0.0") == 0)
					{
						if(strcmp(white.rule[i].intf, "") == 0)
						{
							totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " add white-list ip %s:%s\n", ipbegin ,white.rule[i].key.ip.ports);
						}
						else
						{
							totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " add white-list ip %s:%s %s\n",\
									ipbegin ,white.rule[i].key.ip.ports,white.rule[i].intf);
						}
					}
					else
					{
						if(strcmp(white.rule[i].intf, "") == 0)
						{
							totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " add white-list ip %s-%s:%s\n", ipbegin ,ipend, white.rule[i].key.ip.ports);
						}
						else
						{
							totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " add white-list ip %s-%s:%s %s\n",\
									ipbegin ,ipend, white.rule[i].key.ip.ports,white.rule[i].intf);
						}
					}
					
				}
				else if((RULE_TYPE)type_tmp == RULE_DOMAIN)
				{
					if(strcmp(white.rule[i].intf, "") == 0)
					{
						totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1," add white-list domain %s\n",\
									white.rule[i].key.domain.name);
					}
					else
					{
						totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1," add white-list domain %s %s\n",\
									white.rule[i].key.domain.name, white.rule[i].intf);
					}					
				}
			}
		}
	}

	//add black-list
	struct bw_rules black;
	ret = eag_show_black_list(dcli_dbus_connection_curr,
						localid, index, &black);
	if (EAG_RETURN_OK == ret) {	
		if (black.curr_num > 0) {			
			for (i=0; i < black.curr_num; i++) {
				if (black.rule[i].type == RULE_IPADDR)
					type = "IP";
				else if (black.rule[i].type == RULE_DOMAIN)
					type = "Domain";

				ip2str( black.rule[i].key.ip.ipbegin, ipbegin,sizeof(ipbegin));
				ip2str( black.rule[i].key.ip.ipend, ipend,sizeof(ipend));

				type_tmp = black.rule[i].type;
				if( (RULE_TYPE)type_tmp == RULE_IPADDR )
				{
					if(strcmp(ipend, "0.0.0.0") == 0)
					{
						if(strcmp(black.rule[i].intf, "") == 0)
						{
							totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1," add black-list ip %s:%s\n", ipbegin ,black.rule[i].key.ip.ports);
						}
						else
						{
							totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1," add black-list ip %s:%s %s\n",\
									ipbegin ,black.rule[i].key.ip.ports,black.rule[i].intf);
						}
					}
					else
					{
						if(strcmp(black.rule[i].intf, "") == 0)
						{
							totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " add black-list ip %s-%s:%s\n", ipbegin ,ipend, black.rule[i].key.ip.ports);
						}
						else
						{
							totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " add black-list ip %s-%s:%s %s\n",\
									ipbegin ,ipend, black.rule[i].key.ip.ports,black.rule[i].intf);
						}
					}
					
				}
				else if( (RULE_TYPE)type_tmp == RULE_DOMAIN )
				{
					if(strcmp(black.rule[i].intf, "") == 0)
					{
						totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " add black-list domain %s\n",\
									black.rule[i].key.domain.name);
					}
					else
					{
						totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " add black-list domain %s %s\n",\
									black.rule[i].key.domain.name, black.rule[i].intf);
					}					
				}
			}
		}
	}
	
#if 0	
	if (0 == strlen(showStr)){
		return "";// might case core dump!!
	}
#endif

	tmp = malloc(strlen(showStr) + 1);
	if (NULL == tmp){
		return NULL;
	}
	memset(tmp, 0, strlen(showStr) + 1);
	memcpy(tmp, showStr, strlen(showStr));
	return tmp;
}

int
eag_nasid_conf_show_running(struct vty* vty)
{
	int ret=-1;
	int i=0;
	struct api_nasid_conf nasidconf;
	memset(&nasidconf, 0, sizeof(nasidconf));
	char showStr[SHOW_STR_LEN]={0};
	memset(showStr, 0, sizeof(showStr));	

	ret = eag_get_nasid(dcli_dbus_connection, HANSI_LOCAL, 0, &nasidconf);
	if (EAG_RETURN_OK == ret) {
		if (0 == nasidconf.current_num) {
			return CMD_SUCCESS;
		}

		for (i = 0; i<nasidconf.current_num; i++) {
			switch(nasidconf.nasid_map[i].key_type) {						
			case NASID_KEYTYPE_WLANID:
				if (nasidconf.nasid_map[i].keywd_1
					==nasidconf.nasid_map[i].keywd_2){
					snprintf(showStr, sizeof(showStr)," add nasid wlanid %lu ",nasidconf.nasid_map[i].keywd_1);
				}else{
					snprintf(showStr, sizeof(showStr)," add nasid wlanid %lu-%lu ",\
								nasidconf.nasid_map[i].keywd_1,\
								nasidconf.nasid_map[i].keywd_2);
				}
				break;
			case NASID_KEYTYPE_VLANID:
				if (nasidconf.nasid_map[i].keywd_1
					==nasidconf.nasid_map[i].keywd_2){
					snprintf(showStr, sizeof(showStr)," add nasid vlanid %lu ",nasidconf.nasid_map[i].keywd_1);
				}else{
					snprintf(showStr, sizeof(showStr)," add nasid vlanid %lu-%lu ",\
								nasidconf.nasid_map[i].keywd_1,\
								nasidconf.nasid_map[i].keywd_2);
				}
				break;
			case NASID_KEYTYPE_WTPID:
				if (nasidconf.nasid_map[i].keywd_1
					==nasidconf.nasid_map[i].keywd_2){
					snprintf(showStr, sizeof(showStr)," add nasid wtpid %lu ",nasidconf.nasid_map[i].keywd_1);
				}else{
					snprintf(showStr, sizeof(showStr)," add nasid wtpid %lu-%lu ",\
								nasidconf.nasid_map[i].keywd_1,\
								nasidconf.nasid_map[i].keywd_2);
				}
				break;
			case NASID_KEYTYPE_IPRANGE:
				{
					char ip_1[24] = {0};
					char ip_2[24] = {0};
					if (nasidconf.nasid_map[i].keywd_1==nasidconf.nasid_map[i].keywd_2) {
						ip2str(nasidconf.nasid_map[i].keywd_1,ip_1,sizeof(ip_1));
						snprintf(showStr, sizeof(showStr), " add nasid iprange %s ",ip_1);
					} else {
						ip2str(nasidconf.nasid_map[i].keywd_1,ip_1,sizeof(ip_1));
						ip2str(nasidconf.nasid_map[i].keywd_2,ip_2,sizeof(ip_2));
						snprintf(showStr, sizeof(showStr)," add nasid iprange %s-%s ",ip_1,ip_2);
					}
				}
				break;
			case NASID_KEYTYPE_INTF:
				snprintf(showStr, sizeof(showStr), " add nasid interface %s ",nasidconf.nasid_map[i].keystr);
				break;
			default:
				break;
			}
			snprintf(showStr+strlen(showStr), sizeof(showStr)-strlen(showStr)-1, "nasid %s syntaxis-point %lu",\
										nasidconf.nasid_map[i].nasid,nasidconf.nasid_map[i].conid);
			vtysh_add_show_string(showStr);
		}
	}
	
	return CMD_SUCCESS;
}

char *
eag_nasid_conf_show_running_2(int localid, int slot_id,int index)
{
	int ret=-1;
	int i=0;
	int flag=0;
	struct api_nasid_conf nasidconf;
	memset(&nasidconf, 0, sizeof(nasidconf));
	char showStr[SHOW_STR_LEN*30] = {0};
	memset(showStr, 0, sizeof(showStr));	
	char *tmp = NULL;
	DBusConnection *dcli_dbus_connection_curr = NULL;
	char *cursor = NULL;
	int totalLen = 0;
	cursor = showStr;
	totalLen = 0;
	
	ReInitDbusConnection(&dcli_dbus_connection_curr, slot_id, distributFag);
	ret = eag_get_nasid(dcli_dbus_connection_curr, localid, index, &nasidconf);
	if (EAG_RETURN_OK == ret){
		if (0 == nasidconf.current_num) {
			return "";
		}
		for (i = 0; i < nasidconf.current_num; i++){
			switch (nasidconf.nasid_map[i].key_type) {						
			case NASID_KEYTYPE_WLANID:
				if (nasidconf.nasid_map[i].keywd_1
					==nasidconf.nasid_map[i].keywd_2){
					totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1," add nasid wlanid %lu ",nasidconf.nasid_map[i].keywd_1);
				}else{
					totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1," add nasid wlanid %lu-%lu ",\
								nasidconf.nasid_map[i].keywd_1,\
								nasidconf.nasid_map[i].keywd_2);
				}
				break;
			case NASID_KEYTYPE_VLANID:
				if (nasidconf.nasid_map[i].keywd_1
					==nasidconf.nasid_map[i].keywd_2){
					totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1," add nasid vlanid %lu ",nasidconf.nasid_map[i].keywd_1);
				}else{
					totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1," add nasid vlanid %lu-%lu ",\
								nasidconf.nasid_map[i].keywd_1,\
								nasidconf.nasid_map[i].keywd_2);
				}
				break;
			case NASID_KEYTYPE_WTPID:
				if (nasidconf.nasid_map[i].keywd_1
					==nasidconf.nasid_map[i].keywd_2){
					totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1," add nasid wtpid %lu ",nasidconf.nasid_map[i].keywd_1);
				}else{
					totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1," add nasid wtpid %lu-%lu ",\
								nasidconf.nasid_map[i].keywd_1,\
								nasidconf.nasid_map[i].keywd_2);
				}
				break;
			case NASID_KEYTYPE_IPRANGE:
				{
					char ip_1[24] = {0};
					char ip_2[24] = {0};
					if (nasidconf.nasid_map[i].keywd_1 == nasidconf.nasid_map[i].keywd_2){
						ip2str(nasidconf.nasid_map[i].keywd_1,ip_1,sizeof(ip_1));
						totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " add nasid iprange %s ", ip_1);
					} else {
						ip2str(nasidconf.nasid_map[i].keywd_1, ip_1, sizeof(ip_1));
						ip2str(nasidconf.nasid_map[i].keywd_2, ip_2, sizeof(ip_2));
						totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1," add nasid iprange %s-%s ", ip_1, ip_2);
					}
				}
				break;
			case NASID_KEYTYPE_INTF:
				totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1," add nasid interface %s ",nasidconf.nasid_map[i].keystr);
				break;
			default:
				flag = 1;
				break;
			}
			if (flag)
				continue;
			
			totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1,"nasid %s syntaxis-point %lu\n",\
										nasidconf.nasid_map[i].nasid,nasidconf.nasid_map[i].conid);
		}
	}

#if 0	
		if (0 == strlen(showStr)){
			return "";// might case core dump!!
		}
#endif

	tmp = malloc(strlen(showStr) + 1);
	if (NULL == tmp) {
		return NULL;
	}
	memset(tmp, 0, strlen(showStr) + 1);
	memcpy(tmp, showStr, strlen(showStr));
	return tmp;
}

int
eag_nasportid_conf_show_running(struct vty* vty)
{
	int ret = -1;
	int i = 0;
	char showStr[SHOW_STR_LEN*30] = {0};
	memset(showStr, 0, sizeof(showStr));	
	struct nasportid_conf nasportid;
	memset (&nasportid,0,sizeof(nasportid));
	
	ret = eag_get_nasportid(dcli_dbus_connection, HANSI_LOCAL, 0, &nasportid);

	if (EAG_RETURN_OK != ret) {
		return CMD_SUCCESS;
	}

	if (0 == nasportid.current_num) {
		return CMD_SUCCESS;
	}

	for (i = 0; i < nasportid.current_num; i++) {
		switch(nasportid.nasportid_map[i].key_type) {
		case NASPORTID_KEYTYPE_WLAN_WTP:
			if (nasportid.nasportid_map[i].key.wlan_wtp.wlanid_begin
				== nasportid.nasportid_map[i].key.wlan_wtp.wlanid_end) {
				snprintf(showStr, sizeof(showStr), " add nasportid wlanid %lu ", nasportid.nasportid_map[i].key.wlan_wtp.wlanid_begin);
			} else {
				snprintf(showStr, sizeof(showStr), " add nasportid wlanid %lu-%lu ", nasportid.nasportid_map[i].key.wlan_wtp.wlanid_begin,\
						nasportid.nasportid_map[i].key.wlan_wtp.wlanid_end);
			}
			if (nasportid.nasportid_map[i].key.wlan_wtp.wtpid_begin
				== nasportid.nasportid_map[i].key.wlan_wtp.wtpid_end) {
				snprintf(showStr+strlen(showStr), sizeof(showStr)-strlen(showStr)-1, "wtpid %lu ",nasportid.nasportid_map[i].key.wlan_wtp.wtpid_begin);
			} else {
				snprintf(showStr+strlen(showStr), sizeof(showStr)-strlen(showStr)-1, "wtpid %lu-%lu ",nasportid.nasportid_map[i].key.wlan_wtp.wtpid_begin,\
						nasportid.nasportid_map[i].key.wlan_wtp.wtpid_end);
			}
			break;
		case NASPORTID_KEYTYPE_VLAN:
			if (nasportid.nasportid_map[i].key.vlan.vlanid_begin
				== nasportid.nasportid_map[i].key.vlan.vlanid_end) {
				snprintf(showStr, sizeof(showStr), " add nasportid vlanid %lu ", nasportid.nasportid_map[i].key.vlan.vlanid_begin);
			} else {
				snprintf(showStr, sizeof(showStr), " add nasportid vlanid %lu-%lu ", nasportid.nasportid_map[i].key.vlan.vlanid_begin,\
						nasportid.nasportid_map[i].key.vlan.vlanid_end);
			}
			break;
		default:
			return CMD_SUCCESS;
		}
		snprintf(showStr+strlen(showStr), sizeof(showStr)-strlen(showStr)-1,"nasportid %lu",nasportid.nasportid_map[i].nasportid);
		vtysh_add_show_string(showStr);
	}

	return CMD_SUCCESS;
}

char *
eag_nasportid_conf_show_running_2(int localid, int slot_id,int index)
{
	int ret = -1;
	int i = 0;
	char showStr[SHOW_STR_LEN*30] = {0};
	memset(showStr, 0, sizeof(showStr));	
	struct nasportid_conf nasportid;
	memset(&nasportid, 0, sizeof(nasportid));
	char *tmp = NULL;
	DBusConnection *dcli_dbus_connection_curr = NULL;
	char *cursor = NULL;
	int totalLen = 0;
	memset(showStr, 0, sizeof(showStr));
	cursor = showStr;
	totalLen = 0;
	
	ReInitDbusConnection(&dcli_dbus_connection_curr, slot_id, distributFag);
	ret = eag_get_nasportid(dcli_dbus_connection_curr, localid, index, &nasportid);

	if (EAG_RETURN_OK != ret) {
		return "";
	}

	if (0 == nasportid.current_num) {
		return "";
	}

	for (i = 0; i < nasportid.current_num; i++) {
		switch(nasportid.nasportid_map[i].key_type) {
		case NASPORTID_KEYTYPE_WLAN_WTP:
			if (nasportid.nasportid_map[i].key.wlan_wtp.wlanid_begin
				== nasportid.nasportid_map[i].key.wlan_wtp.wlanid_end) {
				totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " add nasportid wlanid %lu ", nasportid.nasportid_map[i].key.wlan_wtp.wlanid_begin);
			} else {
				totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " add nasportid wlanid %lu-%lu ", nasportid.nasportid_map[i].key.wlan_wtp.wlanid_begin,\
						nasportid.nasportid_map[i].key.wlan_wtp.wlanid_end);
			}
			if (nasportid.nasportid_map[i].key.wlan_wtp.wtpid_begin
				== nasportid.nasportid_map[i].key.wlan_wtp.wtpid_end) {
				totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, "wtpid %lu ",nasportid.nasportid_map[i].key.wlan_wtp.wtpid_begin);
			} else {
				totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, "wtpid %lu-%lu ",nasportid.nasportid_map[i].key.wlan_wtp.wtpid_begin,\
						nasportid.nasportid_map[i].key.wlan_wtp.wtpid_end);
			}
			break;
		case NASPORTID_KEYTYPE_VLAN:
			if (nasportid.nasportid_map[i].key.vlan.vlanid_begin
				== nasportid.nasportid_map[i].key.vlan.vlanid_end) {
				totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " add nasportid vlanid %lu ", nasportid.nasportid_map[i].key.vlan.vlanid_begin);
			} else {
				totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " add nasportid vlanid %lu-%lu ", nasportid.nasportid_map[i].key.vlan.vlanid_begin,\
						nasportid.nasportid_map[i].key.vlan.vlanid_end);
			}
			break;
		default:
			return CMD_SUCCESS;
		}
		totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, "nasportid %lu\n",nasportid.nasportid_map[i].nasportid);
	}

#if 0	
	if (0 == strlen(showStr)){
		return "";// might case core dump!!
	}
#endif	
	tmp = malloc(strlen(showStr) + 1);
	if (NULL == tmp){
		return NULL;
	}
	memset(tmp, 0, strlen(showStr) + 1);
	memcpy(tmp, showStr, strlen(showStr));
	return tmp;
}

int
eag_base_config_show_running(struct vty* vty)
{
	int ret = -1;
	struct eag_base_conf baseconf = {0};
	char showStr[SHOW_STR_LEN] = "";
	char nasip_str[32] = "";
	char nasipv6_str[48] = "";
	memset(&baseconf, 0, sizeof(baseconf));

	ret = eag_get_base_conf(dcli_dbus_connection, HANSI_LOCAL, 0, &baseconf);
	if (EAG_RETURN_OK == ret) {
		if (0 != baseconf.nasip) {
			ip2str(baseconf.nasip, nasip_str, sizeof(nasip_str));
			snprintf(showStr, sizeof(showStr), " set nasip %s", nasip_str);
			vtysh_add_show_string(showStr);
		}
		if (ipv6_compare_null(&(baseconf.nasipv6))) {
			ipv6tostr(&(baseconf.nasipv6), nasipv6_str, sizeof(nasipv6_str));
			snprintf(showStr, sizeof(showStr), " set nasipv6 %s", nasipv6_str);
			vtysh_add_show_string(showStr);		
		}
		if (1 == baseconf.ipv6_switch) {
			snprintf(showStr, sizeof(showStr), " set ipv6 service enable");
			vtysh_add_show_string(showStr);		
		}
		if (DEFAULT_PORTAL_PORT != baseconf.portal_port) {
			snprintf(showStr, sizeof(showStr), " set portal-port %u", baseconf.portal_port);
			vtysh_add_show_string(showStr);
		}
		if (DEFAULT_PORTAL_RETRY_TIMES != baseconf.portal_retry_times
			|| DEFAULT_PORTAL_RETRY_INTERVAL != baseconf.portal_retry_interval) {
			snprintf(showStr, sizeof(showStr), " set portal-retry %d %d", 
				baseconf.portal_retry_interval, baseconf.portal_retry_times);
			vtysh_add_show_string(showStr);
		}
		if (0 == baseconf.auto_session) {
			snprintf(showStr, sizeof(showStr), " set auto-session disable");
			vtysh_add_show_string(showStr);	
		}
		if (DEFAULT_ACCT_INTERVAL != baseconf.radius_acct_interval) {
			snprintf(showStr, sizeof(showStr), " set acct-interval %d",
				baseconf.radius_acct_interval);
			vtysh_add_show_string(showStr);
		}
		if (DEFAULT_RADIUS_RETRY_INTERVAL != baseconf.radius_retry_interval
			|| DEFAULT_RADIUS_RETRY_TIMES != baseconf.radius_retry_times
			|| DEFAULT_VICE_RADIUS_RETRY_TIMES != baseconf.vice_radius_retry_times) {
			snprintf(showStr, sizeof(showStr), " set radius-retry %d %d %d",
				baseconf.radius_retry_interval, baseconf.radius_retry_times, baseconf.vice_radius_retry_times);
			vtysh_add_show_string(showStr);
		}
		if (DEFAULT_MAX_REDIR_TIMES != baseconf.max_redir_times) {
			snprintf(showStr, sizeof(showStr), " set max-redir-times %d", baseconf.max_redir_times);
			vtysh_add_show_string(showStr);		
		}
		if (0 != baseconf.force_dhcplease) {
			snprintf(showStr, sizeof(showStr), " set force-dhcplease enable");
			vtysh_add_show_string(showStr);		
		}
		if (0 != baseconf.check_errid) {
			snprintf(showStr, sizeof(showStr), " set check-errid enable");
			vtysh_add_show_string(showStr);		
		}
		if (DEFAULT_IDLE_TIMEOUT != baseconf.idle_timeout 
			|| DEFAULT_IDLE_FLOW != baseconf.idle_flow) {
			snprintf(showStr, sizeof(showStr), " set idle-timeout %u idle-flow %llu",
				baseconf.idle_timeout, baseconf.idle_flow); 
			vtysh_add_show_string(showStr);		
		}
		if (0 == baseconf.ipset_auth) {
			snprintf(showStr, sizeof(showStr), " set ipset-auth disable");
			vtysh_add_show_string(showStr);
		}
		if (0 == baseconf.force_wireless){
			snprintf(showStr, sizeof(showStr), " set force-wireless disable");
			vtysh_add_show_string(showStr);		
		}
		if (0 != baseconf.check_nasportid) {
			snprintf(showStr, sizeof(showStr), " set check-nasportid enable");
			vtysh_add_show_string(showStr);
		}

		if (FLUX_FROM_WIRELESS == baseconf.flux_from) {
			snprintf(showStr, sizeof(showStr), " set flux-from wireless");
			vtysh_add_show_string(showStr);
		}else if (FLUX_FROM_IPTABLES == baseconf.flux_from) {
			snprintf(showStr, sizeof(showStr), " set flux-from iptables");
			vtysh_add_show_string(showStr);
		}else if (FLUX_FROM_IPTABLES_L2 == baseconf.flux_from) {
			snprintf(showStr, sizeof(showStr), " set flux-from iptables_L2");
			vtysh_add_show_string(showStr);
		}else if (FLUX_FROM_FASTFWD_IPTABLES == baseconf.flux_from) {
			snprintf(showStr, sizeof(showStr), " set flux-from fastfwd_iptables");
			vtysh_add_show_string(showStr);
		}
		if (DEFAULT_FLUX_INTERVAL != baseconf.flux_interval) {
			snprintf(showStr, sizeof(showStr), " set flux-interval %d",
				baseconf.flux_interval);
			vtysh_add_show_string(showStr);
		}
		/* if (1 == baseconf.is_distributed) {
			snprintf(showStr, sizeof(showStr), " set distributed on");
			vtysh_add_show_string(showStr);	
		} */
		if (1 == baseconf.rdc_distributed) {
			snprintf(showStr, sizeof(showStr), " set rdc-distributed on");
			vtysh_add_show_string(showStr);	
		}
		if (1 == baseconf.pdc_distributed) {
			snprintf(showStr, sizeof(showStr), " set pdc-distributed on");
			vtysh_add_show_string(showStr);	
		}
		/* if (1 != baseconf.rdcpdc_slotid || 0 != baseconf.rdcpdc_insid) {
			snprintf(showStr, sizeof(showStr), " set rdcpdc-hansi %d-%d",
				baseconf.rdcpdc_slotid, baseconf.rdcpdc_insid);
			vtysh_add_show_string(showStr);		
		} */
		if (1 != baseconf.rdc_slotid || 0 != baseconf.rdc_insid) {
			snprintf(showStr, sizeof(showStr), " set rdc-hansi %d-%d",
				baseconf.rdc_slotid, baseconf.rdc_insid);
			vtysh_add_show_string(showStr);		
		}
		if (1 != baseconf.pdc_slotid || 0 != baseconf.pdc_insid) {
			snprintf(showStr, sizeof(showStr), " set pdc-hansi %d-%d",
				baseconf.pdc_slotid, baseconf.pdc_insid);
			vtysh_add_show_string(showStr);		
		}
		if (1000 != baseconf.input_correct_factor
			|| 1000 != baseconf.output_correct_factor) {
			snprintf(showStr, sizeof(showStr), " set octets-correct-factor %u %u",
				baseconf.input_correct_factor, baseconf.output_correct_factor);
			vtysh_add_show_string(showStr);	
		}
		if (0 != baseconf.trap_onlineusernum_switch){
			snprintf(showStr, sizeof(showStr), " set trap-switch online-user-num on");
			vtysh_add_show_string(showStr);	
		}
		if (1000 != baseconf.threshold_onlineusernum) {	
			snprintf(showStr, sizeof(showStr), " set threshold online-user-num %d",
													baseconf.threshold_onlineusernum);
			vtysh_add_show_string(showStr);		
		}
		if (0 != baseconf.trap_switch_abnormal_logoff) {
			snprintf(showStr, sizeof(showStr), " set trap-switch abnormal_logoff on");
			vtysh_add_show_string(showStr);		
		}
		if (PORTAL_PROTOCOL_TELECOM == baseconf.portal_protocol) {
			snprintf(showStr, sizeof(showStr), " set portal-protocol telecom");
			vtysh_add_show_string(showStr);		
		}
		
		if (1 == baseconf.macauth_switch) {
			snprintf(showStr, sizeof(showStr), " set mac-auth service enable");
			vtysh_add_show_string(showStr);		
		}
		if (0 == baseconf.macauth_ipset_auth) {
			snprintf(showStr, sizeof(showStr), " set mac-auth ipset-auth disable");
			vtysh_add_show_string(showStr);		
		}
		if (FLUX_FROM_WIRELESS == baseconf.macauth_flux_from) {
			snprintf(showStr, sizeof(showStr), " set mac-auth flux-from wireless");
			vtysh_add_show_string(showStr);
		}else if (FLUX_FROM_IPTABLES == baseconf.macauth_flux_from) {
			snprintf(showStr, sizeof(showStr), " set mac-auth flux-from iptables");
			vtysh_add_show_string(showStr);
		}else if (FLUX_FROM_IPTABLES_L2 == baseconf.macauth_flux_from) {
			snprintf(showStr, sizeof(showStr), " set mac-auth flux-from iptables_L2");
			vtysh_add_show_string(showStr);
		}else if (FLUX_FROM_FASTFWD_IPTABLES == baseconf.macauth_flux_from) {
			snprintf(showStr, sizeof(showStr), " set mac-auth flux-from fastfwd_iptables");
			vtysh_add_show_string(showStr);
		}
		if (DEFAULT_MACAUTH_FLUX_INTERVAL != baseconf.macauth_flux_interval) {
			snprintf(showStr, sizeof(showStr), " set mac-auth flux-interval %d",
				baseconf.macauth_flux_interval);
			vtysh_add_show_string(showStr);		
		}
		if (DEFAULT_MACAUTH_FLUX_THRESHOLD != baseconf.macauth_flux_threshold 
			|| DEFAULT_MACAUTH_CHECK_INTERVAL != baseconf.macauth_check_interval) {
			snprintf(showStr, sizeof(showStr), " set mac-auth flux-threshold %d check-interval %d",
				baseconf.macauth_flux_threshold, baseconf.macauth_check_interval);
			vtysh_add_show_string(showStr);		
		}
		if (0 == baseconf.macauth_notice_bindserver) {
			snprintf(showStr, sizeof(showStr), " set mac-auth notice-to-bindserver disable");
			vtysh_add_show_string(showStr);		
		}
		if (0 == baseconf.autelan_log) {
			snprintf(showStr, sizeof(showStr), " set log-format autelan off");
			vtysh_add_show_string(showStr);		
		}
		if (1 == baseconf.henan_log) {
			snprintf(showStr, sizeof(showStr), " set log-format henan on");
			vtysh_add_show_string(showStr);		
		}
		if (1 == baseconf.username_check) {
			snprintf(showStr, sizeof(showStr), " set username-check on");
			vtysh_add_show_string(showStr);		
		}
		if (1 == baseconf.l2super_vlan) {
			snprintf(showStr, sizeof(showStr), " set l2super-vlan enable");
			vtysh_add_show_string(showStr);		
		}
		if (1 == baseconf.telecom_idletime_valuecheck) {
			snprintf(showStr, sizeof(showStr), " set telecom idletime-valuecheck on");
			vtysh_add_show_string(showStr);		
		}
		if (1 == baseconf.status) {
			snprintf(showStr, sizeof(showStr), " service enable");
			vtysh_add_show_string(showStr);		
		}
	}
	
	return CMD_SUCCESS;
}

char *
eag_base_config_show_running_2(int localid, int slot_id,int index)
{
	int ret = -1;
	struct eag_base_conf baseconf = {0};
	char showStr[SHOW_STR_LEN*5] = "";
	char nasip_str[32] = "";
	char nasipv6_str[48] = "";	
	memset( &baseconf, 0, sizeof(baseconf));
	char *tmp = NULL;
	DBusConnection *dcli_dbus_connection_curr = NULL;
	char *cursor = NULL;
	int totalLen = 0;
	cursor = showStr;
	totalLen = 0;

	ReInitDbusConnection(&dcli_dbus_connection_curr, slot_id, distributFag);
	ret = eag_get_base_conf(dcli_dbus_connection_curr, localid, index, &baseconf);
	if (EAG_RETURN_OK == ret) {
		if (0 != baseconf.nasip) {
			ip2str(baseconf.nasip, nasip_str, sizeof(nasip_str));
			totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " set nasip %s\n", nasip_str);
		}
		if (ipv6_compare_null(&(baseconf.nasipv6))) {
			ipv6tostr(&(baseconf.nasipv6), nasipv6_str, sizeof(nasipv6_str));
			totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " set nasipv6 %s\n", nasipv6_str);
		}
		if (1 == baseconf.ipv6_switch) {
			totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " set ipv6 service enable\n");
		}
		if (DEFAULT_PORTAL_PORT != baseconf.portal_port) {
			totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " set portal-port %u\n", baseconf.portal_port);
		}
		if (DEFAULT_PORTAL_RETRY_TIMES != baseconf.portal_retry_times
			|| DEFAULT_PORTAL_RETRY_INTERVAL != baseconf.portal_retry_interval) {
			totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " set portal-retry %d %d\n", 
				baseconf.portal_retry_interval, baseconf.portal_retry_times);
		}
		if (0 == baseconf.auto_session) {
			totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " set auto-session disable\n");
		}
		if (DEFAULT_ACCT_INTERVAL != baseconf.radius_acct_interval) {
			totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " set acct-interval %d\n", baseconf.radius_acct_interval);
		}
		if (DEFAULT_RADIUS_RETRY_INTERVAL != baseconf.radius_retry_interval
			|| DEFAULT_RADIUS_RETRY_TIMES != baseconf.radius_retry_times
			|| DEFAULT_VICE_RADIUS_RETRY_TIMES != baseconf.vice_radius_retry_times) {
			totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " set radius-retry %d %d %d\n", 
				baseconf.radius_retry_interval, baseconf.radius_retry_times, baseconf.vice_radius_retry_times);
		}
		if (DEFAULT_MAX_REDIR_TIMES != baseconf.max_redir_times) {
			totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " set max-redir-times %d\n", baseconf.max_redir_times);
		}
		if (0 != baseconf.force_dhcplease){
			totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " set force-dhcplease enable\n");
		}
		if (0 != baseconf.check_errid){
			totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " set check-errid enable\n");
		}
		if (DEFAULT_IDLE_TIMEOUT != baseconf.idle_timeout
			|| DEFAULT_IDLE_FLOW != baseconf.idle_flow) {
			totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " set idle-timeout %u idle-flow %llu\n",
				baseconf.idle_timeout, baseconf.idle_flow);
		}
		if (0 == baseconf.ipset_auth){
			totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " set ipset-auth disable\n");
		}
		if (0 == baseconf.force_wireless){
			totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " set force-wireless disable\n");
		}
		if (0 != baseconf.check_nasportid){
			totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " set check-nasportid enable\n");
		}
		
		if (FLUX_FROM_WIRELESS == baseconf.flux_from) {
			totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " set flux-from wireless\n");
		}else if (FLUX_FROM_IPTABLES == baseconf.flux_from) {
			totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " set flux-from iptables\n");
		}else if (FLUX_FROM_IPTABLES_L2 == baseconf.flux_from) {
			totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " set flux-from iptables_L2\n");
		}else if (FLUX_FROM_FASTFWD_IPTABLES == baseconf.flux_from) {
			totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " set flux-from fastfwd_iptables\n");
		}
		
		if (DEFAULT_FLUX_INTERVAL != baseconf.flux_interval) {
			totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " set flux-interval %d\n", baseconf.flux_interval);
		}
		
		/* if (1 == baseconf.is_distributed) {
			totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " set distributed on\n");
		} */
		if (1 == baseconf.rdc_distributed) {
			totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " set rdc-distributed on\n");
		}
		if (1 == baseconf.pdc_distributed) {
			totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " set pdc-distributed on\n");
		}
		/* if (1 != baseconf.rdcpdc_slotid || 0 != baseconf.rdcpdc_insid) {
			totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " set rdcpdc-hansi %d-%d\n",
				baseconf.rdcpdc_slotid, baseconf.rdcpdc_insid );
		} */
		if (1 != baseconf.rdc_slotid || 0 != baseconf.rdc_insid) {
			totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " set rdc-hansi %d-%d\n",
				baseconf.rdc_slotid, baseconf.rdc_insid );
		}
		if (1 != baseconf.pdc_slotid || 0 != baseconf.pdc_insid) {
			totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " set pdc-hansi %d-%d\n",
				baseconf.pdc_slotid, baseconf.pdc_insid );
		}
		if (1000 != baseconf.input_correct_factor
			|| 1000 != baseconf.output_correct_factor) {
			totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " set octets-correct-factor %u %u\n",
				baseconf.input_correct_factor, baseconf.output_correct_factor);
		}
		if (0 != baseconf.trap_onlineusernum_switch){
			totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " set trap-switch online-user-num on\n");
		}
		if (1000 != baseconf.threshold_onlineusernum) {
			totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " set threshold online-user-num %d\n",
													baseconf.threshold_onlineusernum);
		}	
		if (0 != baseconf.trap_switch_abnormal_logoff){
			totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " set trap-switch abnormal_logoff on\n");
		}
		if (PORTAL_PROTOCOL_TELECOM == baseconf.portal_protocol) {
			totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " set portal-protocol telecom\n");
		}
		if (0 != baseconf.macauth_switch){
			totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " set mac-auth service enable\n");
		}
		if (0 == baseconf.macauth_ipset_auth){
			totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " set mac-auth ipset-auth disable\n");
		}
		if (FLUX_FROM_WIRELESS == baseconf.macauth_flux_from) {
			totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " set mac-auth flux-from wireless\n");
		}else if (FLUX_FROM_IPTABLES == baseconf.macauth_flux_from) {
			totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " set mac-auth flux-from iptables\n");
		}else if (FLUX_FROM_IPTABLES_L2 == baseconf.macauth_flux_from) {
			totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " set mac-auth flux-from iptables_L2\n");
		}else if (FLUX_FROM_FASTFWD_IPTABLES == baseconf.macauth_flux_from) {
			totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " set mac-auth flux-from fastfwd_iptables\n");
		}
		if (DEFAULT_MACAUTH_FLUX_INTERVAL != baseconf.macauth_flux_interval) {
			totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " set mac-auth flux-interval %d\n", baseconf.macauth_flux_interval);
		}
		if (DEFAULT_MACAUTH_FLUX_THRESHOLD != baseconf.macauth_flux_threshold 
			|| DEFAULT_MACAUTH_CHECK_INTERVAL != baseconf.macauth_check_interval) {
			totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " set mac-auth flux-threshold %d check-interval %d\n",
				baseconf.macauth_flux_threshold, baseconf.macauth_check_interval);
		}
		if (0 == baseconf.macauth_notice_bindserver) {
			totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " set mac-auth notice-to-bindserver disable\n");
		}
		if (0 == baseconf.autelan_log) {
			totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " set log-format autelan off\n");
		}
		if (1 == baseconf.henan_log) {
			totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " set log-format henan on\n");
		}
		if (1 == baseconf.username_check) {
			totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " set username-check on\n");
		}
		if (1 == baseconf.l2super_vlan) {
			totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " set l2super-vlan enable\n");
		}
		if (1 == baseconf.telecom_idletime_valuecheck) {
			totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " set telecom idletime-valuecheck on\n");
		}
		if (1 == baseconf.status) {
			totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " service enable\n");	
		}
	}
	
	tmp = malloc(strlen(showStr) + 1);
	if (NULL == tmp){
		return NULL;
	}
	memset(tmp, 0, strlen(showStr) + 1);
	memcpy(tmp, showStr, strlen(showStr));
	return tmp;
}

int
eag_has_config(void)
{
	int ret = 0;
	char *domain = NULL;
	struct portal_conf portalconf = {0};
	struct radius_conf radiusconf = {0};
	eag_captive_intfs captive_intfs = {0};
	struct api_nasid_conf nasidconf = {0};
	struct nasportid_conf nasportid = {0};
	struct eag_base_conf baseconf = {0};
	struct bw_rules white = {0};
	struct bw_rules black = {0};
	
	ret = eag_get_portal_conf(dcli_dbus_connection, HANSI_LOCAL,
					0, &portalconf);
	if (EAG_RETURN_OK == ret) {
		if (0 != portalconf.current_num) {
			return 1;
		}
	}

	ret = eag_get_radius_conf(dcli_dbus_connection, HANSI_LOCAL, 
					0, domain, &radiusconf );
	if (EAG_RETURN_OK == ret) {
		if (0 != radiusconf.current_num){
			return 1;
		}
	}

	ret = eag_get_captive_intfs(dcli_dbus_connection, HANSI_LOCAL, 
					0, &captive_intfs);
	if (EAG_RETURN_OK == ret) {
		if (0 != captive_intfs.curr_ifnum) {
			return 1;
		}
	}

	ret = eag_show_white_list(dcli_dbus_connection, HANSI_LOCAL, 0, &white);
	if (EAG_RETURN_OK == ret) {
		if (white.curr_num > 0) {
			return 1;
		}
	}

	ret = eag_show_black_list(dcli_dbus_connection, HANSI_LOCAL, 0, &black);
	if (EAG_RETURN_OK == ret) {
		if (black.curr_num > 0) {
			return 1;
		}
	}

	ret = eag_get_nasid(dcli_dbus_connection, HANSI_LOCAL, 0, &nasidconf);
	if (EAG_RETURN_OK == ret) {
		if (0 != nasidconf.current_num) {
			return 1;
		}
	}

	ret = eag_get_nasportid(dcli_dbus_connection, HANSI_LOCAL,
					0, &nasportid);
	if (EAG_RETURN_OK == ret) {
		if (0 != nasportid.current_num) {
			return 1;
		}
	}

	ret = eag_get_base_conf(dcli_dbus_connection, HANSI_LOCAL, 0, &baseconf);
	if (EAG_RETURN_OK == ret) {
		if (0 != baseconf.nasip) {
			return 1;
		}
		if (DEFAULT_PORTAL_PORT != baseconf.portal_port) {
			return 1;
		}
		if (DEFAULT_PORTAL_RETRY_TIMES != baseconf.portal_retry_times
			|| DEFAULT_PORTAL_RETRY_INTERVAL != baseconf.portal_retry_interval) {
			return 1;
		}
		if (0 == baseconf.auto_session) {
			return 1;
		}
		if (DEFAULT_ACCT_INTERVAL != baseconf.radius_acct_interval) {
			return 1;
		}
		if (DEFAULT_RADIUS_RETRY_INTERVAL != baseconf.radius_retry_interval
			|| DEFAULT_RADIUS_RETRY_TIMES != baseconf.radius_retry_times
			|| DEFAULT_VICE_RADIUS_RETRY_TIMES != baseconf.vice_radius_retry_times) {
			return 1;
		}
		if (DEFAULT_MAX_REDIR_TIMES != baseconf.max_redir_times) {
			return 1;		
		}
		if (0 != baseconf.force_dhcplease) {
			return 1;
		}
		if (0 != baseconf.check_errid) {
			return 1;
		}
		if (DEFAULT_IDLE_TIMEOUT != baseconf.idle_timeout 
			|| DEFAULT_IDLE_FLOW != baseconf.idle_flow) {
			return 1;	
		}
		if (0 == baseconf.force_wireless){
			return 1;	
		}
		if (0 == baseconf.ipset_auth){
			return 1;	
		}

		if (FLUX_FROM_FASTFWD != baseconf.flux_from) {
			return 1;
		}
		if (DEFAULT_FLUX_INTERVAL != baseconf.flux_interval) {
			return 1;
		}
		if (0 != baseconf.trap_switch_abnormal_logoff) {
			return 1;
		}
		if (0 != baseconf.trap_onlineusernum_switch) {
			return 1;
		}
		if (1000 != baseconf.threshold_onlineusernum) {
			return 1;
		}
		/* if (1 == baseconf.is_distributed) {
			return 1;
		} */
		if (1 == baseconf.rdc_distributed) {
			return 1;
		}
		if (1 == baseconf.pdc_distributed) {
			return 1;
		}
		/* if (1 != baseconf.rdcpdc_slotid || 0 != baseconf.rdcpdc_insid) {
			return 1;	
		} */
		if (1 != baseconf.rdc_slotid || 0 != baseconf.rdc_insid) {
			return 1;	
		}
		if (1 != baseconf.pdc_slotid || 0 != baseconf.pdc_insid) {
			return 1;	
		}
		if (1000 != baseconf.input_correct_factor
			|| 1000 != baseconf.output_correct_factor) {
			return 1;	
		}
		if (0 != baseconf.macauth_switch
			|| 0 == baseconf.macauth_ipset_auth
			|| FLUX_FROM_FASTFWD != baseconf.flux_from
			|| DEFAULT_MACAUTH_FLUX_INTERVAL != baseconf.macauth_flux_interval
			|| DEFAULT_MACAUTH_FLUX_THRESHOLD != baseconf.macauth_flux_threshold
			|| DEFAULT_MACAUTH_CHECK_INTERVAL != baseconf.macauth_check_interval)
		{
			return 1;
		}
		if (0 == baseconf.macauth_notice_bindserver)
		{
			return 1;
		}
		if (PORTAL_PROTOCOL_MOBILE != baseconf.portal_protocol) {
			return 1;		
		}
		if (0 == baseconf.autelan_log) {
			return 1;
		}
		if (1 == baseconf.henan_log) {
			return 1;
		}
		if (1 == baseconf.l2super_vlan) {
			return 1;
		}
		if (1 == baseconf.status) {
			return 1;		
		}
	}

	return 0;
}

int
dcli_eag_show_running_return(struct vty* vty)
{
	/* just return */
	return CMD_SUCCESS;
}

int
dcli_eag_show_running_config(struct vty* vty)
{
	/*eag 2.0*/
	char tmpstr[64];
	memset(tmpstr, 0, 64);
	sprintf(tmpstr, BUILDING_MOUDLE, "EAG");
	vtysh_add_show_string(tmpstr);

	if (!eag_has_config()) {
		return CMD_SUCCESS;
	}
	vtysh_add_show_string("config eag");
		
	eag_multi_portal_config_show_running(vty);
	eag_radius_config_show_running(vty);
	eag_captive_portal_config_show_running(vty);
	eag_nasid_conf_show_running(vty);
	eag_nasportid_conf_show_running(vty);
	eag_base_config_show_running(vty);
	
	vtysh_add_show_string(" exit");
	
	return CMD_SUCCESS;
}

char *
dcli_eag_show_running_config_2(int localid, int slot_id, int index)
{
	char *tmp = NULL;
	char showStr[SHOW_STR_LEN*100];
	char *cursor = NULL;
	int totalLen = 0;
	int tmplen = 0;
	memset (showStr, 0, sizeof(showStr));
	cursor = showStr;
	totalLen = 0;

	totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, "config eag\n");
	tmplen = totalLen;

	tmp = eag_multi_portal_config_show_running_2(localid, slot_id, index);
	if (NULL != tmp) {
		if ("" != tmp){
			totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, "%s\n",tmp);
			tmplen++;
			free(tmp);
		}
	}else {
		return NULL;
	}
		
	tmp = eag_radius_config_show_running_2(localid, slot_id, index);
	if (NULL != tmp) {
		if ("" != tmp){
			totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, "%s\n",tmp);
			tmplen++;
			free(tmp);
		}
	}else {
		return NULL;
	}
	
	tmp = eag_captive_portal_config_show_running_2(localid, slot_id, index);
	if (NULL != tmp) {
		if ("" != tmp) {
			totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, "%s\n",tmp);
			tmplen++;
			free(tmp);
		}
	}else {
		return NULL;
	}
	
	tmp = eag_nasid_conf_show_running_2(localid, slot_id, index);
	if (NULL != tmp){
		if ("" != tmp) {
			totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, "%s\n",tmp);
			tmplen++;
			free(tmp);
		}
	}else {
		return NULL;
	}
	
	tmp = eag_nasportid_conf_show_running_2(localid, slot_id, index);
	if (NULL != tmp){
		if ("" != tmp) {
			totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, "%s\n",tmp);
			tmplen++;
			free(tmp);
		}
	}else {
		return NULL;
	}
	
	tmp = eag_base_config_show_running_2(localid, slot_id, index);
	if (NULL != tmp){
		if ("" != tmp) {
			totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, "%s\n",tmp);
			tmplen++;
			free(tmp);
		}
	}else {
		return NULL;
	}

	/* eag has no config */
	if (totalLen == tmplen) {
		tmp = (char*)malloc(1);
		if (NULL == tmp) {
			return NULL;
		}
		memset(tmp, 0, 1);
		return tmp;
	}
	/* eag has config */
	totalLen += snprintf(cursor+totalLen, sizeof(showStr)-totalLen-1, " exit\n");
	tmp = (char *)malloc(strlen(showStr)+1);
	if (NULL == tmp){
		return NULL;
	}
	memset(tmp, 0, strlen(showStr)+1);
	memcpy(tmp,showStr,strlen(showStr));
	return tmp;
}

#define EAG_DCLI_INIT_HANSI_INFO	\
int hansitype = HANSI_LOCAL;   	\
int slot_id = HostSlotId;   \
int insid = 0;\
if(vty->node == EAG_NODE){\
	insid = 0;\
}else if(vty->node == HANSI_EAG_NODE){\
	insid = (int)vty->index; 	\
	hansitype = HANSI_REMOTE;\
	slot_id = vty->slotindex; \
}\
else if (vty->node == LOCAL_HANSI_EAG_NODE)\
{\
	insid = (int)vty->index;\
	hansitype = HANSI_LOCAL;\
	slot_id = vty->slotindex;\
}\
DBusConnection *dcli_dbus_connection_curr = NULL;\
ReInitDbusConnection(&dcli_dbus_connection_curr, slot_id, distributFag);

#if 0
/* base conf */
#endif

DEFUN(set_eag_nasip_func,
	set_eag_nasip_cmd,
	"set nasip A.B.C.D",
	SETT_STR
	"set nasip\n"
	"ip addr which used to identify nas!\n"
)
{
	int ret;
	unsigned long ipaddr;
	struct in_addr inaddr;
	
	int hansitype = HANSI_LOCAL;	
	int slot_id = HostSlotId;   
	int insid = 0;
	if (vty->node == EAG_NODE) {
		insid = 0;
	}
	else if (vty->node == HANSI_EAG_NODE) {
		insid = (int)vty->index; 	
		hansitype = HANSI_REMOTE;
		slot_id = vty->slotindex; 
	}
	else if (vty->node == LOCAL_HANSI_EAG_NODE) {
		insid = (int)vty->index;
		hansitype = HANSI_LOCAL;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection_curr = NULL;
	ReInitDbusConnection(&dcli_dbus_connection_curr, slot_id, distributFag);

	ret = inet_aton(argv[0], &inaddr);
	if (!ret) {
            vty_out(vty, "%% invalid ip address\n");
            return CMD_WARNING;
    }
	ipaddr = ntohl(inaddr.s_addr);

	if (eag_ins_running_state(dcli_dbus_connection_curr, hansitype, insid)) {
		vty_out(vty, "%% eag is running, please stop it first\n");
		return CMD_FAILURE;
	}
	
	ret = eag_set_nasip(dcli_dbus_connection_curr, hansitype, insid, ipaddr);
	if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_ERR_UNKNOWN == ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}

	return CMD_SUCCESS;
}

DEFUN(set_eag_nasipv6_func,
	set_eag_nasipv6_cmd,
	"set nasipv6 IPV6",
	SETT_STR
	"set nasipv6\n"
	"ipv6 addr like A::B.C.D.E or A::B:C which used to identify nas!\n"
)
{
	int ret;
	uint32_t ipv6[4];
	
	int hansitype = HANSI_LOCAL;	
	int slot_id = HostSlotId;   
	int insid = 0;
	if (vty->node == EAG_NODE) {
		insid = 0;
	}
	else if (vty->node == HANSI_EAG_NODE) {
		insid = (int)vty->index; 	
		hansitype = HANSI_REMOTE;
		slot_id = vty->slotindex; 
	}
	else if (vty->node == LOCAL_HANSI_EAG_NODE) {
		insid = (int)vty->index;
		hansitype = HANSI_LOCAL;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection_curr = NULL;
	ReInitDbusConnection(&dcli_dbus_connection_curr, slot_id, distributFag);

	memset(ipv6, 0, sizeof(ipv6));
	ret = inet_pton(AF_INET6, argv[0], (struct in6_addr *)ipv6);
	if (!ret) {
		vty_out(vty, "%% invalid ipv6 address, please like A::B.C.D.E or A::B:C\n");
		return CMD_WARNING;
    }
	if (0 == memcmp(argv[0], "::", 2)) {
		vty_out(vty, "%% invalid ipv6 address, please like A::B.C.D.E or A::B:C, A should not be NULL\n");
		return CMD_WARNING;
	}
	if (eag_ins_running_state(dcli_dbus_connection_curr, hansitype, insid)) {
		vty_out(vty, "%% eag is running, please stop it first\n");
		return CMD_FAILURE;
	}
	ret = eag_set_nasipv6(dcli_dbus_connection_curr, hansitype, insid, ipv6);
	if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_ERR_UNKNOWN == ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}

	return CMD_SUCCESS;
}

DEFUN(set_eag_ipv6_service_status_func,
	set_eag_ipv6_service_status_cmd,
	"set ipv6 service (enable|disable)",
	SETT_STR
	"ipv6 service\n"
	"ipv6 service\n"
	"Enable\n"
	"Disable\n"
)
{
	int ipv6_service = 0;
	int ret = -1;
	
	if (strncmp(argv[0], "enable", strlen(argv[0])) == 0) {
		ipv6_service = 1;
	} 
	else if (strncmp(argv[0], "disable", strlen(argv[0])) == 0) {
		ipv6_service = 0;
	}
	else {
		vty_out(vty,"%% bad command parameter\n");
		return CMD_WARNING;
	}
	
	EAG_DCLI_INIT_HANSI_INFO
#if 1		
	if(eag_ins_running_state(dcli_dbus_connection_curr, hansitype, insid)){
		vty_out(vty, "eag instance %d is running, please stop it first\n",insid);
		return CMD_FAILURE;
	}
#endif	
	ret = eag_set_ipv6_server(dcli_dbus_connection_curr, hansitype, insid, ipv6_service);
	if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_RETURN_OK != ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}
	return CMD_SUCCESS; 
}

DEFUN(set_eag_distributed_func,
	set_eag_distributed_cmd,
	"set distributed (on|off)",
	SETT_STR
	"set distributed on/off\n"
	"set distributed on!\n"
	"set distributed off!\n"	
)
{
	if (!boot_flag) {
		vty_out(vty, "You should use new command 'set rdc-distributed/pdc-distributed', this command only use in boot time\n");
		return CMD_SUCCESS;
	}

	int ret;
	int flag = 0;
	
	int hansitype = HANSI_LOCAL;	
	int slot_id = HostSlotId;   
	int insid = 0;
	if(vty->node == EAG_NODE){
		insid = 0;
	}
	else if (vty->node == HANSI_EAG_NODE) {
		insid = (int)vty->index; 	
		hansitype = HANSI_REMOTE;
		slot_id = vty->slotindex; 
	}
	else if (vty->node == LOCAL_HANSI_EAG_NODE) {
		insid = (int)vty->index;
		hansitype = HANSI_LOCAL;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection_curr = NULL;
	ReInitDbusConnection(&dcli_dbus_connection_curr,slot_id,distributFag);

	flag = (strncmp(argv[0], "on", 2) == 0) ? 1 : 0;
	
	if (eag_ins_running_state(dcli_dbus_connection_curr, hansitype, insid)) {
		vty_out(vty, "%% eag is running, please stop it first\n");
		return CMD_FAILURE;
	}
	
	//ret = eag_set_distributed(dcli_dbus_connection_curr, hansitype, insid, flag);
	ret = eag_set_rdc_distributed(dcli_dbus_connection_curr, hansitype, insid, flag);
	ret = eag_set_pdc_distributed(dcli_dbus_connection_curr, hansitype, insid, flag);
	if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_ERR_UNKNOWN == ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}

	return CMD_SUCCESS;
}

DEFUN(set_eag_rdc_distributed_func,
	set_eag_rdc_distributed_cmd,
	"set rdc-distributed (on|off)",
	SETT_STR
	"set rdc-distributed on/off\n"
	"set rdc-distributed on!\n"
	"set rdc-distributed off!\n"	
)
{
	int ret;
	int flag = 0;
	
	int hansitype = HANSI_LOCAL;	
	int slot_id = HostSlotId;   
	int insid = 0;
	if(vty->node == EAG_NODE){
		insid = 0;
	}
	else if (vty->node == HANSI_EAG_NODE) {
		insid = (int)vty->index; 	
		hansitype = HANSI_REMOTE;
		slot_id = vty->slotindex; 
	}
	else if (vty->node == LOCAL_HANSI_EAG_NODE) {
		insid = (int)vty->index;
		hansitype = HANSI_LOCAL;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection_curr = NULL;
	ReInitDbusConnection(&dcli_dbus_connection_curr,slot_id,distributFag);

	flag = (strncmp(argv[0], "on", 2) == 0) ? 1 : 0;
	
	if (eag_ins_running_state(dcli_dbus_connection_curr, hansitype, insid)) {
		vty_out(vty, "%% eag is running, please stop it first\n");
		return CMD_FAILURE;
	}
	
	ret = eag_set_rdc_distributed(dcli_dbus_connection_curr, hansitype, insid, flag);
	if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_ERR_UNKNOWN == ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}

	return CMD_SUCCESS;
}

DEFUN(set_eag_pdc_distributed_func,
	set_eag_pdc_distributed_cmd,
	"set pdc-distributed (on|off)",
	SETT_STR
	"set pdc-distributed on/off\n"
	"set pdc-distributed on!\n"
	"set pdc-distributed off!\n"	
)
{
	int ret;
	int flag = 0;
	
	int hansitype = HANSI_LOCAL;	
	int slot_id = HostSlotId;   
	int insid = 0;
	if(vty->node == EAG_NODE){
		insid = 0;
	}
	else if (vty->node == HANSI_EAG_NODE) {
		insid = (int)vty->index; 	
		hansitype = HANSI_REMOTE;
		slot_id = vty->slotindex; 
	}
	else if (vty->node == LOCAL_HANSI_EAG_NODE) {
		insid = (int)vty->index;
		hansitype = HANSI_LOCAL;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection_curr = NULL;
	ReInitDbusConnection(&dcli_dbus_connection_curr,slot_id,distributFag);

	flag = (strncmp(argv[0], "on", 2) == 0) ? 1 : 0;
	
	if (eag_ins_running_state(dcli_dbus_connection_curr, hansitype, insid)) {
		vty_out(vty, "%% eag is running, please stop it first\n");
		return CMD_FAILURE;
	}
	
	ret = eag_set_pdc_distributed(dcli_dbus_connection_curr, hansitype, insid, flag);
	if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_ERR_UNKNOWN == ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}

	return CMD_SUCCESS;
}

DEFUN(set_eag_rdcpdc_hansi_func,
	set_eag_rdcpdc_hansi_cmd,
	"set rdcpdc-hansi PARAM",
	SETT_STR
	"set eag rdcpdc-hansi slotid-insid\n"
	"set eag rdcpdc-hansi slotid-insid\n"
)
{
	if (!boot_flag) {
		vty_out(vty, "You should use new command 'set rdc-hansi/pdc-hansi', this command only use in boot time\n");
		return CMD_SUCCESS;
	}

	int ret;
	int flag=0;
	
	int hansitype = HANSI_LOCAL;	
	int slot_id = HostSlotId;   
	int insid = 0;
	int pdcrdc_slotid=0;
	int pdcrdc_insid=0;
	if(vty->node == EAG_NODE) {
		insid = 0;
	}
	else if (vty->node == HANSI_EAG_NODE) {
		insid = (int)vty->index; 	
		hansitype = HANSI_REMOTE;
		slot_id = vty->slotindex; 
	}
	else if (vty->node == LOCAL_HANSI_EAG_NODE) {
		insid = (int)vty->index;
		hansitype = HANSI_LOCAL;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection_curr = NULL;
	ReInitDbusConnection(&dcli_dbus_connection_curr, slot_id, distributFag);

	ret = sscanf(argv[0], "%d-%d", &pdcrdc_slotid, &pdcrdc_insid);
	if (ret != 2) {
		vty_out(vty, "%% the PARAM should format like 1-3(slotid-insid)!\n");
		return CMD_FAILURE;
	}

	if (pdcrdc_slotid > 10 || pdcrdc_insid > 16 ||
		pdcrdc_slotid < 0 || pdcrdc_insid < 0) {
		vty_out(vty, "%% Slot id should less than 10 and insid shold less than 16\n");		
		return CMD_FAILURE;
	}
#if 0	
	if(eag_ins_running_state(dcli_dbus_connection_curr, hansitype, insid)){
		vty_out(vty, "eag instance %d is running, please stop it first\n",insid);
		return CMD_FAILURE;
	}
#endif	
	//ret = eag_set_rdcpdc_ins(dcli_dbus_connection_curr, hansitype, insid, 
						//pdcrdc_slotid, pdcrdc_insid);
	ret = eag_set_rdc_ins(dcli_dbus_connection_curr, hansitype, insid, 
						pdcrdc_slotid, pdcrdc_insid);
	ret = eag_set_pdc_ins(dcli_dbus_connection_curr, hansitype, insid, 
						pdcrdc_slotid, pdcrdc_insid);
	if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_ERR_UNKNOWN == ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}

	return CMD_SUCCESS;
}

DEFUN(set_eag_rdc_hansi_func,
	set_eag_rdc_hansi_cmd,
	"set rdc-hansi PARAM",
	SETT_STR
	"set eag rdc-hansi slotid-insid\n"
	"set eag rdc-hansi slotid-insid\n"
)
{
	int ret;
	int flag=0;
	
	int hansitype = HANSI_LOCAL;	
	int slot_id = HostSlotId;   
	int insid = 0;
	int rdc_slotid=0;
	int rdc_insid=0;
	if(vty->node == EAG_NODE) {
		insid = 0;
	}
	else if (vty->node == HANSI_EAG_NODE) {
		insid = (int)vty->index; 	
		hansitype = HANSI_REMOTE;
		slot_id = vty->slotindex; 
	}
	else if (vty->node == LOCAL_HANSI_EAG_NODE) {
		insid = (int)vty->index;
		hansitype = HANSI_LOCAL;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection_curr = NULL;
	ReInitDbusConnection(&dcli_dbus_connection_curr, slot_id, distributFag);

	ret = sscanf(argv[0], "%d-%d", &rdc_slotid, &rdc_insid);
	if (ret != 2) {
		vty_out(vty, "%% the PARAM should format like 1-3(slotid-insid)!\n");
		return CMD_FAILURE;
	}

	if (rdc_slotid > 10 || rdc_insid > 16 ||
		rdc_slotid < 0 || rdc_insid < 0) {
		vty_out(vty, "%% Slot id should less than 10 and insid shold less than 16\n");		
		return CMD_FAILURE;
	}
#if 0	
	if(eag_ins_running_state(dcli_dbus_connection_curr, hansitype, insid)){
		vty_out(vty, "eag instance %d is running, please stop it first\n",insid);
		return CMD_FAILURE;
	}
#endif	
	ret = eag_set_rdc_ins(dcli_dbus_connection_curr, hansitype, insid, 
						rdc_slotid, rdc_insid);
	if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_ERR_UNKNOWN == ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}

	return CMD_SUCCESS;
}

DEFUN(set_eag_pdc_hansi_func,
	set_eag_pdc_hansi_cmd,
	"set pdc-hansi PARAM",
	SETT_STR
	"set eag pdc-hansi slotid-insid\n"
	"set eag pdc-hansi slotid-insid\n"
)
{
	int ret;
	int flag=0;
	
	int hansitype = HANSI_LOCAL;	
	int slot_id = HostSlotId;   
	int insid = 0;
	int pdc_slotid=0;
	int pdc_insid=0;
	if(vty->node == EAG_NODE) {
		insid = 0;
	}
	else if (vty->node == HANSI_EAG_NODE) {
		insid = (int)vty->index; 	
		hansitype = HANSI_REMOTE;
		slot_id = vty->slotindex; 
	}
	else if (vty->node == LOCAL_HANSI_EAG_NODE) {
		insid = (int)vty->index;
		hansitype = HANSI_LOCAL;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection_curr = NULL;
	ReInitDbusConnection(&dcli_dbus_connection_curr, slot_id, distributFag);

	ret = sscanf(argv[0], "%d-%d", &pdc_slotid, &pdc_insid);
	if (ret != 2) {
		vty_out(vty, "%% the PARAM should format like 1-3(slotid-insid)!\n");
		return CMD_FAILURE;
	}

	if (pdc_slotid > 10 || pdc_insid > 16 ||
		pdc_slotid < 0 || pdc_insid < 0) {
		vty_out(vty, "%% Slot id should less than 10 and insid shold less than 16\n");		
		return CMD_FAILURE;
	}
#if 0	
	if(eag_ins_running_state(dcli_dbus_connection_curr, hansitype, insid)){
		vty_out(vty, "eag instance %d is running, please stop it first\n",insid);
		return CMD_FAILURE;
	}
#endif	
	ret = eag_set_pdc_ins(dcli_dbus_connection_curr, hansitype, insid, 
						pdc_slotid, pdc_insid);
	if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_ERR_UNKNOWN == ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}

	return CMD_SUCCESS;
}

DEFUN(set_eag_portal_port_func,
	set_eag_portal_port_cmd,
	"set portal-port <1-65535>",
	SETT_STR
	"set portal-port\n"
	"port which used to connect to portal server! default is 2000.\n"
)
{
	int ret;
	unsigned short port;
	
	EAG_DCLI_INIT_HANSI_INFO
	if (eag_ins_running_state(dcli_dbus_connection_curr, hansitype, insid)) {
		vty_out(vty, "%% eag is running, please stop it first\n");
		return CMD_FAILURE;
	}
	
	port = (unsigned short)atoi(argv[0]);
	ret = eag_set_portal_port(dcli_dbus_connection_curr, hansitype, insid, port);
	if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_ERR_UNKNOWN == ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}
	
	return CMD_SUCCESS;
}

DEFUN(set_eag_portal_retry_params_func,
	set_eag_portal_retry_params_cmd,
	"set portal-retry <1-10> <0-10>",
	SETT_STR
	"set portal-retry params!\n"
	"set portal requirement timeout vlaue\n"
	"set portal retry times\n"
)
{
	int ret = -1;
	int resend_times = -1;
	unsigned long resend_interval = -1;
	
	EAG_DCLI_INIT_HANSI_INFO
#if 0
	if(eag_ins_running_state(dcli_dbus_connection_curr, hansitype, insid)){
		vty_out(vty, "eag instance %d is running, please stop it first\n",insid);
		return CMD_FAILURE;
	}
#endif	
	if ((NULL != argv[0]) && (NULL != argv[1])) {
		resend_interval = atoi(argv[0]);
		resend_times = atoi(argv[1]);
	}

	ret = eag_set_portal_retry_params(dcli_dbus_connection_curr, 
								hansitype, insid,
								resend_interval,
								resend_times);
	
	if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_ERR_UNKNOWN == ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}
	
	return CMD_SUCCESS;
}

DEFUN(set_eag_auto_session_func,
	set_eag_auto_session_cmd,
	"set auto-session (enable|disable)",
	SETT_STR
	"auto create user session\n"
	"Enable\n"
	"Disable\n"
)
{
	int auto_session = 0;
	int ret = -1;
	
	if (strncmp(argv[0], "enable", strlen(argv[0])) == 0) {		
		auto_session = 1;
	}
	else if (strncmp(argv[0], "disable", strlen(argv[0])) == 0) {
		auto_session = 0;
	}
#if 0	
	else {		
		vty_out(vty, "%% input param error\n");
	}
#endif	
	EAG_DCLI_INIT_HANSI_INFO
#if 0
	if(eag_ins_running_state(dcli_dbus_connection_curr, hansitype, insid)){
		vty_out(vty, "eag instance %d is running, please stop it first\n",insid);
		return CMD_FAILURE;
	}
#endif
	ret = eag_set_auto_session(dcli_dbus_connection_curr, 
											hansitype, insid, 
											auto_session);	
	if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_RETURN_OK != ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}
	return CMD_SUCCESS; 
}

DEFUN(set_eag_acct_interval_func,
	set_eag_acct_interval_cmd,
	"set acct-interval <60-3600>",
	SETT_STR
	"set acct_interval\n"
	"set acct_interval seconds\n"
)
{
	int ret;
	int interval;
	//int idletimeout;

	EAG_DCLI_INIT_HANSI_INFO
#if 0
	if(eag_ins_running_state(dcli_dbus_connection_curr, hansitype, insid)){
		vty_out(vty, "eag instance %d is running, please stop it first\n",insid);
		return CMD_FAILURE;
	}
#endif	
	interval = atoi(argv[0]);
	//idletimeout = atoi(argv[1]);
	ret = eag_set_acct_interval(dcli_dbus_connection_curr, 
								hansitype, insid,					
								interval);
	if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_ERR_UNKNOWN == ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}
	
	return CMD_SUCCESS;
}

DEFUN(set_eag_radius_retry_params_func,
	set_eag_radius_retry_params_cmd,
	"set radius-retry <1-10> <0-10> <0-10>",
	SETT_STR
	"set radius-retry params!\n"
	"set radius requirement timeout vlaue\n"
	"set radius master retry times\n"
	"set radius backup retry times\n"
)
{
	int ret = -1;
	int timeout = -1;
	int master_retry_times = -1;
	int backup_retry_times = -1;
	
	EAG_DCLI_INIT_HANSI_INFO
#if 0		
	if(eag_ins_running_state(dcli_dbus_connection_curr, hansitype, insid)){
		vty_out(vty, "eag instance %d is running, please stop it first\n",insid);
		return CMD_FAILURE;
	}
#endif	
	if ((NULL != argv[0]) && (NULL != argv[1]) && (NULL != argv[2])) {
		timeout = atoi(argv[0]);
		master_retry_times = atoi(argv[1]);
		backup_retry_times = atoi(argv[2]);		
	}

	ret = eag_set_radius_retry_params(dcli_dbus_connection_curr, 
								hansitype, insid,					
								timeout,
								master_retry_times,
								backup_retry_times);
	
	if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_ERR_UNKNOWN == ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}
	
	return CMD_SUCCESS;
}

DEFUN(set_eag_max_redir_times_func,
	set_eag_max_redir_times_cmd,
	"set max-redir-times <10-100>",
	SETT_STR
	"set max http request times in interval\n"
	"the request-times' value\n"
)
{
	int ret = -1;
	unsigned long request_times = 0;
	unsigned long interval = 0;
	char * input_value_0 = (char *)argv[0];

	request_times = strtoul(input_value_0, NULL, 10);

	EAG_DCLI_INIT_HANSI_INFO
#if 0		
	if(eag_ins_running_state(dcli_dbus_connection_curr, hansitype, insid)){
		vty_out(vty, "eag instance %d is running, please stop it first\n",insid);
		return CMD_FAILURE;
	}
#endif	
	ret = eag_set_max_redir_times(dcli_dbus_connection_curr, 
											hansitype, insid, 
											request_times);	

	if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty,"%% dbus error\n");
	}
	else if (EAG_ERR_INPUT_PARAM_ERR == ret) {
		vty_out(vty,"%% input prarm error\n");
	}
	else if (EAG_RETURN_OK != ret){
		vty_out(vty,"%% unknown error: %d\n", ret);
	}

	return CMD_SUCCESS;
}

DEFUN(set_eag_force_dhcplease_func,
	set_eag_force_dhcplease_cmd,
	"set force-dhcplease (enable|disable)",
	SETT_STR
	"Forbid authorize if not from dhcp-lease \n"
	"Enable, forbid authorize if no from dhcp-lease switch enable\n"
	"Disable, forbid authorize if no from dhcp-lease switch disable\n"
)
{
	int force_dhcplease = 0;
	int ret = -1;
	
	if (strncmp(argv[0], "enable", strlen(argv[0])) == 0) {
		force_dhcplease = 1;
	} 
	else if (strncmp(argv[0], "disable", strlen(argv[0])) == 0) {
		force_dhcplease = 0;
	}
#if 0	
	else {
		vty_out(vty,"%% bad command parameter\n");
		return CMD_WARNING;
	}
#endif	
	EAG_DCLI_INIT_HANSI_INFO
#if 0		
	if(eag_ins_running_state(dcli_dbus_connection_curr, hansitype, insid)){
		vty_out(vty, "eag instance %d is running, please stop it first\n",insid);
		return CMD_FAILURE;
	}
#endif	
#if 1
	ret = eag_set_force_dhcplease(dcli_dbus_connection_curr, hansitype, insid, force_dhcplease);
#endif
	if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_RETURN_OK != ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}
	return CMD_SUCCESS; 
}

DEFUN(set_eag_check_errid_func,
	set_eag_check_errid_cmd,
	"set check-errid (enable|disable)",
	SETT_STR
	"Check Errid when eag response to portal server \n"
	"Enable, add ErrID to package when response to portal server\n"
	"Disable, not add ErrID to package when response to portal server\n"
)
{
	int check_errid = 0;
	int ret = -1;
	
	if(strncmp(argv[0], "enable", strlen(argv[0])) == 0){
		check_errid = 1;
	}
	else if(strncmp(argv[0], "disable", strlen(argv[0])) == 0){
		check_errid = 0;
	}
	
	EAG_DCLI_INIT_HANSI_INFO
#if 0
	if(eag_ins_running_state(dcli_dbus_connection_curr,hansitype, insid)){
		vty_out(vty, "eag instance %d is running, please stop it first\n",insid);
		return CMD_FAILURE;
	}
#endif
	ret = eag_set_check_errid(dcli_dbus_connection_curr, hansitype, insid, check_errid);

	if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_RETURN_OK != ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}
	return CMD_SUCCESS; 
}

DEFUN(set_eag_idle_params_func,
	set_eag_idle_params_cmd,
	"set idle-timeout <60-86400> idle-flow <0-10485760>",
	SETT_STR
	"set idle_timeout\n"
	"set idle_timeout seconds\n"	
	"Set idle flow, by Bytes\n"
	"the idle flow value, max 10MB, 1024*1024*10 Bytes\n"
)
{
	int ret = -1;
	uint64_t idle_flow = 0;
	unsigned long idle_timeout =0 ;
	
	idle_timeout = atoi(argv[0]);
	idle_flow = strtoull(argv[1], NULL, 10);
	EAG_DCLI_INIT_HANSI_INFO
#if 1		
	if(eag_ins_running_state(dcli_dbus_connection_curr, hansitype, insid)){
		vty_out(vty, "eag instance %d is running, please stop it first\n",insid);
		return CMD_FAILURE;
	}
#endif	
	ret = eag_set_idle_params(dcli_dbus_connection_curr, 
											hansitype, insid, 
											idle_timeout,
											idle_flow);
											
	if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_RETURN_OK != ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}
	return CMD_SUCCESS;	
}

DEFUN(set_eag_force_wireless_func,
	set_eag_force_wireless_cmd,
	"set force-wireless (enable|disable)",
	SETT_STR
	"only for wireless users\n"
	"Enable\n"
	"Disable\n"
)
{
	int force_wireless = 0;
	int ret = -1;
	
	if (strncmp(argv[0], "enable", strlen(argv[0])) == 0) {
		force_wireless = 1;
	} 
	else if (strncmp(argv[0], "disable", strlen(argv[0])) == 0) {
		force_wireless = 0;
	}
	else {
		vty_out(vty,"%% bad command parameter\n");
		return CMD_WARNING;
	}
	
	EAG_DCLI_INIT_HANSI_INFO
#if 1		
	if(eag_ins_running_state(dcli_dbus_connection_curr, hansitype, insid)){
		vty_out(vty, "eag instance %d is running, please stop it first\n",insid);
		return CMD_FAILURE;
	}
#endif	
	ret = eag_set_force_wireless(dcli_dbus_connection_curr, 
											hansitype, insid, 
											force_wireless);	
	if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_RETURN_OK != ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}
	return CMD_SUCCESS; 
}

DEFUN(set_eag_flux_from_func,
	set_eag_flux_from_cmd,
	"set flux-from (iptables|iptables_L2|wireless|fastfwd|fastfwd_iptables)",
	SETT_STR
	"set flux from iptables, wireless or fastfwd \n"
	"set flux from iptables \n"
	"set flux from iptables_L2 \n"
	"set flux from wireless \n"
	"set flux from fastfwd \n"
	"set flux from fastfwd and iptables\n"
)
{	
	int flux_from = 0;
	int  ret = -1;

	if (strncmp(argv[0], "iptables", strlen(argv[0])) == 0) {
		flux_from = FLUX_FROM_IPTABLES;
	}
	else if (strncmp(argv[0], "iptables_L2", strlen(argv[0])) == 0) {
		flux_from = FLUX_FROM_IPTABLES_L2;
	}
	else if (strncmp(argv[0], "wireless", strlen(argv[0])) == 0) {
		flux_from = FLUX_FROM_WIRELESS;
	}
	else if (strncmp(argv[0], "fastfwd", strlen(argv[0])) == 0) {
		flux_from = FLUX_FROM_FASTFWD;
	}
	else if (strncmp(argv[0], "fastfwd_iptables", strlen(argv[0])) == 0) {
		flux_from = FLUX_FROM_FASTFWD_IPTABLES;
	}
	else {
		vty_out(vty, "input param error\n");
		return CMD_SUCCESS;
	}

	EAG_DCLI_INIT_HANSI_INFO
	if(eag_ins_running_state(dcli_dbus_connection_curr,hansitype, insid)){
		vty_out(vty, "eag instance %d is running, please stop it first\n",insid);
		return CMD_FAILURE;
	}
	ret = eag_set_flux_from(dcli_dbus_connection_curr, 
											hansitype, insid, 
											flux_from);
	if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}else if( EAG_RETURN_OK != ret ){
		vty_out(vty,"%% unknown error : %d\n", ret);
	}
	return CMD_SUCCESS;	
}

DEFUN(set_eag_flux_interval_func,
	set_eag_flux_interval_cmd,
	"set flux-interval <10-3600>",
	SETT_STR
	"set flux interval\n"
	"flux interval\n"
)
{
	int ret = -1;
	int flux_interval = 0;

	flux_interval = atoi(argv[0]);

	EAG_DCLI_INIT_HANSI_INFO
#if 0		
	if(eag_ins_running_state(dcli_dbus_connection_curr, hansitype, insid)){
		vty_out(vty, "eag instance %d is running, please stop it first\n",insid);
		return CMD_FAILURE;
	}
#endif	
	ret = eag_set_flux_interval(dcli_dbus_connection_curr, 
											hansitype, insid, 
											flux_interval);

	if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty,"%% dbus error\n");
	}
	else if (EAG_ERR_INPUT_PARAM_ERR == ret) {
		vty_out(vty,"%% input prarm error\n");
	}
	else if (EAG_RETURN_OK != ret){
		vty_out(vty,"%% unknown error: %d\n", ret);
	}

	return CMD_SUCCESS;
}

DEFUN(set_eag_ipset_auth_func,
	set_eag_ipset_auth_cmd,
	"set ipset-auth (enable|disable)",
	SETT_STR
	"Use ipset method to authorize\n"
	"Enable\n"
	"Disable\n"
)
{
	int ipset_auth = 0;
	int ret = -1;
	
	if (strncmp(argv[0], "enable", strlen(argv[0])) == 0) {
		ipset_auth = 1;
	} 
	else if (strncmp(argv[0], "disable", strlen(argv[0])) == 0) {
		ipset_auth = 0;
	}
	else {
		vty_out(vty,"%% bad command parameter\n");
		return CMD_WARNING;
	}
	
	EAG_DCLI_INIT_HANSI_INFO
	if(eag_ins_running_state(dcli_dbus_connection_curr, hansitype, insid)){
		vty_out(vty, "%% eag is running, please stop it first\n");
		return CMD_FAILURE;
	}
	ret = eag_set_ipset_auth(dcli_dbus_connection_curr,
					hansitype, insid, 
					ipset_auth);	

	if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_RETURN_OK != ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}
	return CMD_SUCCESS; 
}

DEFUN(set_eag_check_nasportid_func,
	set_eag_check_nasportid_cmd,
	"set check-nasportid (enable|disable)",
	SETT_STR
	"Check nasportid\n"
	"Enable\n"
	"Disable\n"
)
{
	int check_nasportid = 0;
	int ret = -1;
	
	if (strncmp(argv[0], "enable", strlen(argv[0])) == 0) {
		check_nasportid = 1;
	} 
	else if (strncmp(argv[0], "disable", strlen(argv[0])) == 0) {
		check_nasportid = 0;
	}
	else {
		vty_out(vty,"%% bad command parameter\n");
		return CMD_WARNING;
	}
	
	EAG_DCLI_INIT_HANSI_INFO
	if(eag_ins_running_state(dcli_dbus_connection_curr, hansitype, insid)){
		vty_out(vty, "%% eag is running, please stop it first\n");
		return CMD_FAILURE;
	}
	ret = eag_set_check_nasportid(dcli_dbus_connection_curr,
					hansitype, insid, 
					check_nasportid);	

	if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_RETURN_OK != ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}
	return CMD_SUCCESS; 
}

DEFUN(set_trap_online_user_num_switch_func,
	set_trap_online_user_num_switch_cmd,
	"set trap-switch online-user-num (on|off)",
	SETT_STR
	"set trap switch\n"
	"set trap online-user-num switch on/off\n"
	"set trap online-user-num switch on/off\n"
	"set trap online-user-num switch on!\n"
	"set trap online-user-num switch off!\n"
)
{
	int switch_t = 0;
	int ret = -1;
	char *conf_value = (char *)argv[0];
	
	if(strncmp(conf_value, "on",strlen(conf_value)) == 0){		
		switch_t = 1;
	}
	else if(strncmp(conf_value, "off",strlen(conf_value)) == 0){
		switch_t = 0;
	}else{		
		vty_out(vty, "%% input param error\n");
	}
	
	EAG_DCLI_INIT_HANSI_INFO
	ret = eag_set_trap_onlineusernum_switch(dcli_dbus_connection_curr, 
											hansitype, insid, 
											switch_t);
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

DEFUN(set_threshold_online_user_num_func,
	set_threshold_online_user_num_cmd,
	"set threshold online-user-num <1-10240>",
	SETT_STR
	"set threshold for trap\n"
	"set threshold online-user-num <1-10240>\n"
	"set threshold online-user-num <1-10240>\n"
)
{
	int ret = 0;
	uint16_t num = 1000;

	num = (uint16_t)strtoul(argv[0], NULL, 10);

	EAG_DCLI_INIT_HANSI_INFO
	ret = eag_set_threshold_onlineusernum(dcli_dbus_connection_curr,
											hansitype, insid,
											num);
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

DEFUN(set_octets_correct_factor_func,
	set_octets_correct_factor_cmd,
	"set octets-correct-factor <1-4294967295> <1-4294967295>",
	SETT_STR
	"for adjust octets value,the base value is 1000\n"
	"input octets adjusted value, if needn't adjust, set the value as 1000\n"
	"output octets adjusted value, if needn't adjust ,set the value as 1000\n"
)
{
	int ret = -1;
	char *input = (char *)argv[0];
	char *output = (char *)argv[1];
	unsigned long input_value = 0;
	unsigned long output_value = 0;
	
	input_value = strtoul(input, NULL, 10);
	output_value = strtoul(output, NULL, 10);
#if 0	
	if((0 == input_value) || (0 == output_value)){		
		vty_out(vty, "set adjusted-value error! error type: out of range\n");
		return CMD_FAILURE;
	}
#endif
	EAG_DCLI_INIT_HANSI_INFO
#if 0		
	if(eag_ins_running_state(dcli_dbus_connection_curr, hansitype, insid)){
		vty_out(vty, "eag instance %d is running, please stop it first\n",insid);
		return CMD_FAILURE;
	}
#endif	
	ret = eag_set_octets_correct_factor(dcli_dbus_connection_curr, 
											hansitype, insid, 
											input_value,
											output_value);
	if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty,"%% dbus error\n");
	}
	else if (EAG_ERR_INPUT_PARAM_ERR == ret) {
		vty_out(vty,"%% input prarm error\n");
	}
	else if (EAG_RETURN_OK != ret){
		vty_out(vty,"%% unknown error: %d\n", ret);
	}

	return CMD_SUCCESS; 
}	

DEFUN(set_eag_abnormal_logoff_trap_switch_func,
	set_eag_abnormal_logoff_trap_switch_cmd,
	"set trap-switch abnormal_logoff (on|off)",
	SETT_STR
	"trap switch\n"
	"abnormal logoff trap\n"
	"Switch On\n"
	"Switch Off\n"
)
{
	int trap_switch = 0;
	int ret = -1;
	if (strncmp(argv[0], "on", strlen(argv[0])) == 0) {
		trap_switch = 1;
	} 
	else if (strncmp(argv[0], "off", strlen(argv[0])) == 0) {
		trap_switch = 0;
	}
	else {
		vty_out(vty,"%% bad command parameter\n");
		return CMD_WARNING;
	}
	EAG_DCLI_INIT_HANSI_INFO
	ret = eag_set_trap_switch_abnormal_logoff(dcli_dbus_connection_curr, 
											hansitype, insid, 
											trap_switch);
	if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_RETURN_OK != ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}
	return CMD_SUCCESS; 
}

DEFUN(set_eag_portal_protocol_func,
	set_eag_portal_protocol_cmd,
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

	EAG_DCLI_INIT_HANSI_INFO
	if(eag_ins_running_state(dcli_dbus_connection_curr,hansitype, insid)){
		vty_out(vty, "eag instance %d is running, please stop it first\n",insid);
		return CMD_FAILURE;
	}
	ret = eag_set_portal_protocol(dcli_dbus_connection_curr, 
											hansitype, insid, 
											portal_protocol);
	if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}else if( EAG_RETURN_OK != ret ){
		vty_out(vty,"%% unknown error : %d\n", ret);
	}
	return CMD_SUCCESS;	
}

DEFUN(set_eag_telecom_idletime_valuecheck_func,
	set_eag_telecom_idletime_valuecheck_cmd,
	"set telecom idletime-valuecheck (on|off)",
	SETT_STR
	"set telecom\n"
	"set telecom idletime-valuecheck\n"
	"set idletime-valuecheck on\n"
	"set idletime-valuecheck off\n"
)
{	
	int value_check = 0;
	int  ret = -1;

	if (strncmp(argv[0], "on", strlen(argv[0])) == 0) {
		value_check = 1;
	}
	else if (strncmp(argv[0], "off", strlen(argv[0])) == 0) {
		value_check = 0;
	}
	else {
		vty_out(vty, "input param error\n");
		return CMD_SUCCESS;
	}

	EAG_DCLI_INIT_HANSI_INFO

	ret = eag_set_telecom_idletime_valuecheck(dcli_dbus_connection_curr, 
											hansitype, insid, 
											value_check);
	if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	} else if ( EAG_RETURN_OK != ret ) {
		vty_out(vty,"%% unknown error : %d\n", ret);
	}
	
	return CMD_SUCCESS;	
}

DEFUN(set_eag_l2super_vlan_status_func,
	set_eag_l2super_vlan_status_cmd,
	"set l2super-vlan (enable|disable)",
	SETT_STR
	"l2super-vlan\n"
	"Enable\n"
	"Disable\n"
)
{
	int l2super_vlan_switch = 0;
	int ret = -1;
	
	if (strncmp(argv[0], "enable", strlen(argv[0])) == 0) {
		l2super_vlan_switch = 1;
	} 
	else if (strncmp(argv[0], "disable", strlen(argv[0])) == 0) {
		l2super_vlan_switch = 0;
	}
	else {
		vty_out(vty,"%% bad command parameter\n");
		return CMD_WARNING;
	}
	
	EAG_DCLI_INIT_HANSI_INFO
#if 1		
	if(eag_ins_running_state(dcli_dbus_connection_curr, hansitype, insid)){
		vty_out(vty, "eag instance %d is running, please stop it first\n",insid);
		return CMD_FAILURE;
	}
#endif	
	ret = eag_set_l2super_vlan_switch(dcli_dbus_connection_curr, 
											hansitype, insid, 
											l2super_vlan_switch);
	if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_RETURN_OK != ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}
	return CMD_SUCCESS; 
}

DEFUN(set_eag_mac_auth_service_status_func,
	set_eag_mac_auth_service_status_cmd,
	"set mac-auth service (enable|disable)",
	SETT_STR
	"mac auth\n"
	"mac auth service\n"
	"Enable\n"
	"Disable\n"
)
{
	int macauth_switch = 0;
	int ret = -1;
	
	if (strncmp(argv[0], "enable", strlen(argv[0])) == 0) {
		macauth_switch = 1;
	} 
	else if (strncmp(argv[0], "disable", strlen(argv[0])) == 0) {
		macauth_switch = 0;
	}
	else {
		vty_out(vty,"%% bad command parameter\n");
		return CMD_WARNING;
	}
	
	EAG_DCLI_INIT_HANSI_INFO
#if 1		
	if(eag_ins_running_state(dcli_dbus_connection_curr, hansitype, insid)){
		vty_out(vty, "eag instance %d is running, please stop it first\n",insid);
		return CMD_FAILURE;
	}
#endif	
	ret = eag_set_macauth_switch(dcli_dbus_connection_curr, 
											hansitype, insid, 
											macauth_switch);
	if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_RETURN_OK != ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}
	return CMD_SUCCESS; 
}

DEFUN(set_eag_mac_auth_ipset_auth_func,
	set_eag_mac_auth_ipset_auth_cmd,
	"set mac-auth ipset-auth (enable|disable)",
	SETT_STR
	"mac auth\n"
	"Use ipset method to authorize for mac-auth\n"
	"Enable\n"
	"Disable\n"
)
{
	int ipset_auth = 0;
	int ret = -1;
	
	if (strncmp(argv[0], "enable", strlen(argv[0])) == 0) {
		ipset_auth = 1;
	} 
	else if (strncmp(argv[0], "disable", strlen(argv[0])) == 0) {
		ipset_auth = 0;
	}
	else {
		vty_out(vty,"%% bad command parameter\n");
		return CMD_WARNING;
	}
	
	EAG_DCLI_INIT_HANSI_INFO
	if(eag_ins_running_state(dcli_dbus_connection_curr, hansitype, insid)){
		vty_out(vty, "%% eag is running, please stop it first\n");
		return CMD_FAILURE;
	}
	ret = eag_set_macauth_ipset_auth(dcli_dbus_connection_curr,
					hansitype, insid, 
					ipset_auth);	

	if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_RETURN_OK != ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}
	return CMD_SUCCESS; 
}

DEFUN(set_eag_mac_auth_flux_from_func,
	set_eag_mac_auth_flux_from_cmd,
	"set mac-auth flux-from (iptables|iptables_L2|wireless|fastfwd|fastfwd_iptables)",
	SETT_STR
	"mac auth\n"
	"set flux from iptables, wireless or fastfwd \n"
	"set flux from iptables \n"
	"set flux from iptables_L2 \n"
	"set flux from wireless \n"
	"set flux from fastfwd \n"
	"set flux from fastfwd and iptables\n"
)
{	
	int flux_from = 0;
	int  ret = -1;

	if (strncmp(argv[0], "iptables", strlen(argv[0])) == 0) {
		flux_from = FLUX_FROM_IPTABLES;
	}
	else if (strncmp(argv[0], "iptables_L2", strlen(argv[0])) == 0) {
		flux_from = FLUX_FROM_IPTABLES_L2;
	}
	else if (strncmp(argv[0], "wireless", strlen(argv[0])) == 0) {
		flux_from = FLUX_FROM_WIRELESS;
	}
	else if (strncmp(argv[0], "fastfwd", strlen(argv[0])) == 0) {
		flux_from = FLUX_FROM_FASTFWD;
	}
	else if (strncmp(argv[0], "fastfwd_iptables", strlen(argv[0])) == 0) {
		flux_from = FLUX_FROM_FASTFWD_IPTABLES;
	}
	else {
		vty_out(vty, "input param error\n");
		return CMD_SUCCESS;
	}

	EAG_DCLI_INIT_HANSI_INFO
	if(eag_ins_running_state(dcli_dbus_connection_curr,hansitype, insid)){
		vty_out(vty, "eag instance %d is running, please stop it first\n",insid);
		return CMD_FAILURE;
	}
	ret = eag_set_macauth_flux_from(dcli_dbus_connection_curr, 
											hansitype, insid, 
											flux_from);
	if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}else if( EAG_RETURN_OK != ret ){
		vty_out(vty,"%% unknown error : %d\n", ret);
	}
	return CMD_SUCCESS;	
}

DEFUN(set_eag_mac_auth_flux_interval_func,
	set_eag_mac_auth_flux_interval_cmd,
	"set mac-auth flux-interval <1-3600>",
	SETT_STR
	"mac auth\n"
	"mac auth flux interval\n"
	"mac auth flux interval\n"
)
{
	int flux_interval = 0;
	int ret = -1;
	
	flux_interval = atoi(argv[0]);
	
	EAG_DCLI_INIT_HANSI_INFO
#if 0		
	if(eag_ins_running_state(dcli_dbus_connection_curr, hansitype, insid)){
		vty_out(vty, "eag instance %d is running, please stop it first\n",insid);
		return CMD_FAILURE;
	}
#endif	
	ret = eag_set_macauth_flux_interval(dcli_dbus_connection_curr, 
											hansitype, insid, 
											flux_interval);	
	if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_RETURN_OK != ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}
	return CMD_SUCCESS; 
}

DEFUN(set_eag_mac_auth_flux_threshold_func,
	set_eag_mac_auth_flux_threshold_cmd,
	"set mac-auth flux-threshold <0-104857600> check-interval <60-3600>",
	SETT_STR
	"mac auth\n"
	"mac auth flux threshold\n"
	"max 100M Bytes, 100*1024*1024.\n"
	"mac auth check interval\n"
	"check interval value, 60-3600seconds.\n"
)
{
	int flux_threshold = 0;
    int check_interval = 0;
	int ret = -1;
	
	flux_threshold = atoi(argv[0]);
    check_interval = atoi(argv[1]);
	
	EAG_DCLI_INIT_HANSI_INFO
#if 0		
	if(eag_ins_running_state(dcli_dbus_connection_curr, hansitype, insid)){
		vty_out(vty, "eag instance %d is running, please stop it first\n",insid);
		return CMD_FAILURE;
	}
#endif	
	ret = eag_set_macauth_flux_threshold(dcli_dbus_connection_curr, 
											hansitype, insid, 
											flux_threshold, check_interval);
	if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_RETURN_OK != ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}
	return CMD_SUCCESS; 
}

DEFUN(set_eag_mac_auth_notice_bindserver_func,
	set_eag_mac_auth_notice_bindserver_cmd,
	"set mac-auth notice-to-bindserver (enable|disable)",
	SETT_STR
	"mac auth\n"
	"mac auth notice to bindserver user logon or logoff\n"
	"Enable\n"
	"Disable\n"
)
{
	int notice_bindserver = 0;
	int ret = -1;
	
	if (strncmp(argv[0], "enable", strlen(argv[0])) == 0) {
		notice_bindserver = 1;
	} 
	else if (strncmp(argv[0], "disable", strlen(argv[0])) == 0) {
		notice_bindserver = 0;
	}
	else {
		vty_out(vty,"%% bad command parameter\n");
		return CMD_WARNING;
	}
	
	EAG_DCLI_INIT_HANSI_INFO
	if(eag_ins_running_state(dcli_dbus_connection_curr, hansitype, insid)){
		vty_out(vty, "%% eag is running, please stop it first\n");
		return CMD_FAILURE;
	}
	ret = eag_set_macauth_notice_bindserver(dcli_dbus_connection_curr,
					hansitype, insid, 
					notice_bindserver);	

	if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_RETURN_OK != ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}
	return CMD_SUCCESS; 
}

DEFUN(eag_service_status_func,
	eag_service_status_cmd,
	"service (enable|disable)",
	SER_STR
	"service eag enable\n"
	"service eag disable\n"
)
{
	int ret = 0;
	int status = 0;

	EAG_DCLI_INIT_HANSI_INFO

	if (strncmp(argv[0], "enable", strlen(argv[0])) == 0) {
		status = 1;
	}
	else if (strncmp(argv[0], "disable", strlen(argv[0])) == 0) {
		status = 0;
	}
	else{
		return CMD_SUCCESS;
	}

	ret = eag_set_services_status( dcli_dbus_connection_curr, hansitype, insid, status );

	if (EAG_RETURN_OK == ret) {
		return CMD_SUCCESS;
	}
	else if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_ERR_EAGINS_SERVICE_ALREADY_ENABLE == ret) {
		vty_out(vty, "%% service already enable\n" );
	}
	else if (EAG_ERR_EAGINS_SERVICE_IS_DISABLE == ret) {
		vty_out(vty, "%% service already disable\n" );
	}
	else if (EAG_ERR_REDIR_SOCKET_INIT_FAILED == ret) {
		vty_out(vty, "%% cannot create redir socket\n" );
	}
	else if (EAG_ERR_REDIR_SOCKET_BIND_FAILED == ret) {
		vty_out(vty, "%% cannot bind redir ip/port\n");
	}
	else if (EAG_ERR_REDIR_SOCKET_LISTEN_FAILED == ret) {
		vty_out(vty, "%% cannot listen redir socket\n");
	}
	else if (EAG_ERR_PORTAL_SOCKET_BIND_FAILED == ret) {
		vty_out(vty, "%% cannot bind portal ip/port\n");
	}
	else if (EAG_ERR_COA_SOCKET_BIND_FAILED == ret) {
		vty_out(vty, "%% cannot bind coa ip/port\n");
	}
	else if (EAG_ERR_CAPTIVE_REDIR_PARAM_NOT_SET == ret) {
		vty_out(vty, "%% captive portal start failed! You should check the nasip configure\n");
	}
	else if (EAG_ERR_EAGINS_NASIP_IS_EMPTY == ret) {
		vty_out(vty, "%% please config nasip before service enable\n");
	}
	else if (EAG_ERR_EAGINS_NASIPV6_IS_EMPTY == ret) {
		vty_out(vty, "%% please config nasipv6 before service enable\n");
	}
	else {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}
	
	return CMD_SUCCESS;
}

DEFUN(show_eag_base_conf_func,
	show_eag_base_conf_cmd,
	"show eag-base-config",
	SHOW_STR
	"eag base config\n"
)
{
	int ret = 0;
	struct eag_base_conf baseconf;
	
	EAG_DCLI_INIT_HANSI_INFO

	memset(&baseconf, 0, sizeof(baseconf));

	ret = eag_get_base_conf(dcli_dbus_connection_curr, hansitype, insid, &baseconf);
	
	if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n" );
	}
	else if (EAG_RETURN_OK == ret) {
		char nasip_str[32] = "";
		char nasipv6_str[48] = "";
		char *flux_from = "";
		char *macauth_flux_from = "";
		vty_out(vty, "service status               :%s\n", (1 == baseconf.status) ? "enable" : "disable");
		ip2str( baseconf.nasip, nasip_str, sizeof(nasip_str)-1);
		vty_out(vty, "nas ip                       :%s\n", nasip_str);
		vty_out(vty, "ipv6 service status          :%s\n", (1 == baseconf.ipv6_switch) ? "enable" : "disable");
        ipv6tostr( &(baseconf.nasipv6), nasipv6_str, sizeof(nasipv6_str));
		vty_out(vty, "nas ipv6                     :%s\n", nasipv6_str);
		//vty_out(vty, "distributed switch        :%s\n", (1 == baseconf.is_distributed) ? "on" : "off");	
		vty_out(vty, "rdc-distributed switch       :%s\n", (1 == baseconf.rdc_distributed) ? "on" : "off");
		vty_out(vty, "pdc-distributed switch       :%s\n", (1 == baseconf.pdc_distributed) ? "on" : "off");
		//vty_out(vty, "distributed rdc pdc param :%d-%d\n", baseconf.rdcpdc_slotid, baseconf.rdcpdc_insid);
		vty_out(vty, "distributed rdc param        :%d-%d\n", baseconf.rdc_slotid, baseconf.rdc_insid);
		vty_out(vty, "distributed pdc param        :%d-%d\n", baseconf.pdc_slotid, baseconf.pdc_insid);
		vty_out(vty, "portal port                  :%hu\n", baseconf.portal_port);
		vty_out(vty, "portal retry interval        :%d\n", baseconf.portal_retry_interval);
		vty_out(vty, "portal retry times           :%d\n", baseconf.portal_retry_times);
		vty_out(vty, "auto-session                 :%s\n", (1 == baseconf.auto_session) ? "enable" : "disable");
		vty_out(vty, "account interval             :%d\n", baseconf.radius_acct_interval);
		vty_out(vty, "radius retry interval        :%d\n", baseconf.radius_retry_interval);
		vty_out(vty, "radius master retry times    :%d\n", baseconf.radius_retry_times);
		vty_out(vty, "radius backup retry times    :%d\n", baseconf.vice_radius_retry_times);
		vty_out(vty, "max-http-request per 5s      :%lu\n", baseconf.max_redir_times);
		vty_out(vty, "force-dhcplease              :%s\n", (1 == baseconf.force_dhcplease) ? "enable" : "disable");
		vty_out(vty, "check-errid                  :%s\n", (1 == baseconf.check_errid)?"enable":"disable");
		vty_out(vty, "idle timeout                 :%d\n", baseconf.idle_timeout);
		vty_out(vty, "idle flow                    :%llu\n", baseconf.idle_flow);
		vty_out(vty, "force-wireless               :%s\n", (1 == baseconf.force_wireless)?"enable":"disable");
 		if (FLUX_FROM_IPTABLES_L2 == baseconf.flux_from) {
			flux_from = "iptables_L2";
		} else if (FLUX_FROM_WIRELESS == baseconf.flux_from) {
			flux_from = "wireless";
		} else if (FLUX_FROM_FASTFWD == baseconf.flux_from) {
			flux_from = "fastfwd";
		} else if (FLUX_FROM_FASTFWD_IPTABLES == baseconf.flux_from) {
			flux_from = "fastfwd_iptables";
		} else {
			flux_from = "iptables";
		}
		vty_out(vty, "flux-from                    :%s\n", flux_from);
		vty_out(vty, "flux-interval                :%d\n", baseconf.flux_interval);
		vty_out(vty, "ipset-auth                   :%s\n", (1 == baseconf.ipset_auth)?"enable":"disable");
		vty_out(vty, "check-nasportid              :%s\n", (1 == baseconf.check_nasportid)?"enable":"disable");
		vty_out(vty, "trap-switch abnormal logoff  :%s\n", (1 == baseconf.trap_switch_abnormal_logoff)?"on":"off");
		vty_out(vty, "trap-switch online-user-num  :%s\n", (1==baseconf.trap_onlineusernum_switch)?"on":"off");
		vty_out(vty, "threshold online-user-num    :%d\n", baseconf.threshold_onlineusernum);
		vty_out(vty, "portal protocol              :%s\n", (PORTAL_PROTOCOL_MOBILE == baseconf.portal_protocol)?"mobile":"telecom");
 		if (FLUX_FROM_IPTABLES_L2 == baseconf.macauth_flux_from) {
			macauth_flux_from = "iptables_L2";
		} else if (FLUX_FROM_WIRELESS == baseconf.macauth_flux_from) {
			macauth_flux_from = "wireless";
		} else if (FLUX_FROM_FASTFWD == baseconf.macauth_flux_from) {
			macauth_flux_from = "fastfwd";
		} else if (FLUX_FROM_FASTFWD_IPTABLES == baseconf.macauth_flux_from) {
			macauth_flux_from = "fastfwd_iptables";
		} else {
			macauth_flux_from = "iptables";
		}
		vty_out(vty, "mac-auth server              :%s\n", (1 == baseconf.macauth_switch)?"enable":"disable");
		vty_out(vty, "mac-auth ipset-auth          :%s\n", (1 == baseconf.macauth_ipset_auth)?"enable":"disable");
		vty_out(vty, "mac-auth flux-from           :%s\n", macauth_flux_from);
		vty_out(vty, "mac-auth flux-interval       :%d\n", baseconf.macauth_flux_interval);
		vty_out(vty, "mac-auth flux-threshold      :%d\n", baseconf.macauth_flux_threshold);
		vty_out(vty, "mac-auth check-interval      :%d\n", baseconf.macauth_check_interval);
		vty_out(vty, "mac-auth notice-to-bindserver:%s\n", (1 == baseconf.macauth_notice_bindserver)?"enable":"disable");
		vty_out(vty, "log-format autelan           :%s\n", (1 == baseconf.autelan_log)?"on":"off");
		vty_out(vty, "log-format henan             :%s\n", (1 == baseconf.henan_log)?"on":"off");
		vty_out(vty, "l2super-vlan                 :%s\n", (1 == baseconf.l2super_vlan)?"enable":"disable");
		vty_out(vty, "username check               :%s\n", (1 == baseconf.username_check)?"on":"off");
		vty_out(vty, "telecom idletime-valuecheck  :%s\n", (1 == baseconf.telecom_idletime_valuecheck)?"on":"off");
	}
	else {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}
	
	return CMD_SUCCESS;
}

#if 0
/* nasid */
#endif

#if 0
DEFUN(add_eag_nasid_func,
	add_eag_nasid_cmd,
	"add nasid (wlanid|vlanid|wtpid|iprange|interface) VALUE nasid VALUE syntaxis-point <0-99>",
	"add\n"
	"add nasid\n"
	"wlanid <1-"DEFUN_MAX_WLANID_INPUT">\n"
	"vlanid <1-"DEFUN_MAX_MAPED_VLANID">\n"
	"wtpid <1-"DEFUN_MAX_WTPID_INPUT">\n"
	"ip range A.B.C.D[-A.B.C.D]\n"
	"interface\n"
	"nasid value\n"
	"syntaxis-point\n"
	"syntaxis-point <0-99>"
)
{
	int ret = 0;
	unsigned long key_type = 0;
	unsigned long keywd_1 = 0;
	unsigned long keywd_2 = 0;
	char *tmp_ip = NULL;
	char *ip = NULL;
	char ip_1[24] = {0};
	char ip_2[24] = {0};

	EAG_DCLI_INIT_HANSI_INFO
		
	struct nasid_map_t nasidmap;
	memset(&nasidmap, 0, sizeof(struct nasid_map_t));

	if (0 == strncmp(argv[0], "wlanid", strlen(argv[0]))) {
		keywd_1 = strtoul(argv[1], NULL, 10);
		nasidmap.key_type = NASID_KEYTYPE_WLANID;
		nasidmap.key.wlanid = keywd_1;
	}
	else if (0 == strncmp(argv[0], "vlanid", strlen(argv[0]))) {
		keywd_1 = strtoul(argv[1], NULL, 10);
		nasidmap.key_type = NASID_KEYTYPE_VLANID;
		nasidmap.key.vlanid = keywd_1;
	}
	else if (0 == strncmp(argv[0], "wtpid", strlen(argv[0]))) {
		keywd_1 = strtoul(argv[1], NULL, 10);
		nasidmap.key_type = NASID_KEYTYPE_WTPID;
		nasidmap.key.wtpid = keywd_1;
	}
	else if (0==strncmp(argv[0], "iprange", strlen(argv[0]))) {
		memset(ip_1, 0, sizeof(ip_1));
		memset(ip_2, 0, sizeof(ip_2));
		nasidmap.key_type = NASID_KEYTYPE_IPRANGE;
		tmp_ip = (char *)argv[1];
		if (NULL == (ip = strchr(tmp_ip, '-'))) {
			strncpy(ip_1, tmp_ip, sizeof(ip_1) - 1);
			if (eag_check_ip_format(ip_1) != EAG_SUCCESS) {
				vty_out(vty, "%% error ip address input : %s\n", ip_1);
				return CMD_WARNING;
			}
			inet_atoi(ip_1, &nasidmap.key.iprange.ip_begin);
			nasidmap.key.iprange.ip_end = nasidmap.key.iprange.ip_begin;
		}
		else {
			strncpy(ip_1, tmp_ip, ip - tmp_ip);
			if (eag_check_ip_format(ip_1) != EAG_SUCCESS) {
				vty_out(vty, "%% error ip address input : %s\n", ip_1);
				return CMD_WARNING;
			}
			tmp_ip = ip + 1;
			strncpy(ip_2, tmp_ip, sizeof(ip_1) - 1);
			if (eag_check_ip_format(ip_2) != EAG_SUCCESS){
				vty_out(vty, "%% error ip address input : %s\n", ip_2);
				return CMD_WARNING;
			}
			inet_atoi(ip_1, &(nasidmap.key.iprange.ip_begin));
			inet_atoi(ip_2, &(nasidmap.key.iprange.ip_end));
		}
	}
	else if (0 == strncmp(argv[0], "interface", strlen(argv[0]))){
		nasidmap.key_type = NASID_KEYTYPE_INTF;
		strncpy(nasidmap.key.intf,(char *)argv[1],MAX_NASID_KEY_BUFF_LEN-1);
	}
	else {
		vty_out(vty, "%% unknown error\n");
		return CMD_WARNING;
	}

	strncpy(nasidmap.nasid, argv[2], MAX_NASID_LEN-1);
	nasidmap.conid = strtoul(argv[3], NULL, 10);
	
	ret = eag_add_nasid(dcli_dbus_connection_curr, hansitype, insid, &nasidmap);
	if (eag_dcli_check_errno(vty, ret, "add nasid error")){
		return CMD_FAILURE;
	}

	return CMD_SUCCESS;
}

DEFUN(del_eag_nasid_func,
	del_eag_nasid_cmd,
	"delete nasid (wlanid|vlanid|wtpid|iprange|interface) VALUE",
	"delete\n"
	"nasid\n"
	"wlanid <1-"DEFUN_MAX_WLANID_INPUT">\n"
	"vlanid <1-"DEFUN_MAX_MAPED_VLANID">\n"
	"wtpid <1-"DEFUN_MAX_WTPID_INPUT">\n"
	"ip range A.B.C.D[-A.B.C.D]\n"
	"interface\n"
	"value\n"
)
{
	int ret = 0;
	unsigned long key_type = 0;
	unsigned long keywd_1 = 0;
	unsigned long keywd_2 = 0;
	char *tmp_ip = NULL;
	char *ip = NULL;
	char ip_1[24];
	char ip_2[24];

	EAG_DCLI_INIT_HANSI_INFO
		
	struct nasid_map_t nasidmap;
	memset (&nasidmap, 0, sizeof(struct nasid_map_t));
	
	if (0 == strncmp(argv[0], "wlanid", strlen(argv[0]))) {
		keywd_1 = strtoul(argv[1], NULL, 10);
		nasidmap.key_type = NASID_KEYTYPE_WLANID;
		nasidmap.key.wlanid = keywd_1;
	}
	else if (0 == strncmp(argv[0], "vlanid", strlen(argv[0]))) {
		keywd_1 = strtoul(argv[1], NULL, 10);
		nasidmap.key_type = NASID_KEYTYPE_VLANID;
		nasidmap.key.vlanid = keywd_1;
	}
	else if (0 == strncmp(argv[0], "wtpid", strlen(argv[0]))) {
		keywd_1 = strtoul(argv[1], NULL, 10);
		nasidmap.key_type = NASID_KEYTYPE_WTPID;
		nasidmap.key.wtpid = keywd_1;
	}
	else if (0 == strncmp(argv[0], "iprange", strlen(argv[0]))) {
		memset(ip_1, 0, sizeof(ip_1));
		memset(ip_2, 0, sizeof(ip_2));
		nasidmap.key_type = NASID_KEYTYPE_IPRANGE;
		tmp_ip = (char *)argv[1];
		if (NULL == (ip = strchr(tmp_ip, '-'))) {
			strncpy(ip_1, tmp_ip, sizeof(ip_1) - 1);
			if (eag_check_ip_format(ip_1) != EAG_SUCCESS) {
				vty_out(vty, "%% error ip address input : %s\n", ip_1);
				return CMD_WARNING;
			}
			inet_atoi(ip_1, &nasidmap.key.iprange.ip_begin);
			nasidmap.key.iprange.ip_end = nasidmap.key.iprange.ip_begin;
		}
		else {
			strncpy(ip_1, tmp_ip, ip-tmp_ip);
			if (eag_check_ip_format(ip_1) != EAG_SUCCESS){
				vty_out(vty, "%% error ip address input : %s\n", ip_1);
				return CMD_WARNING;
			}
			tmp_ip = ip + 1;
			strncpy(ip_2, tmp_ip, 24-1);
			if (eag_check_ip_format(ip_2) != EAG_SUCCESS){
				vty_out(vty, "%% error ip address input : %s\n", ip_2);
				return CMD_WARNING;
			}
			inet_atoi(ip_1, &nasidmap.key.iprange.ip_begin);
			inet_atoi(ip_2, &nasidmap.key.iprange.ip_end);
		}
	}
	else if (0 == strncmp(argv[0], "interface", strlen(argv[0]))) {
		nasidmap.key_type = NASID_KEYTYPE_INTF;
		strncpy(nasidmap.key.intf,argv[1],MAX_NASID_KEY_BUFF_LEN-1);
	}
	else {
		vty_out(vty, "%% unknown error\n");
		return CMD_WARNING;
	}
	
	ret = eag_del_nasid(dcli_dbus_connection_curr, hansitype, insid, &nasidmap);
	if (eag_dcli_check_errno(vty, ret, "del nasid error")){
		return CMD_FAILURE;
	}

	return CMD_SUCCESS;
}

DEFUN(show_eag_nasid_func,
	show_eag_nasid_cmd,
	"show eag-nasid-config",
	SHOW_STR
	"eag base config\n"
)
{
	int ret = 0;
	int i = 0;

	EAG_DCLI_INIT_HANSI_INFO
		
	struct api_nasid_conf nasidconf;
	memset(&nasidconf, 0, sizeof(nasidconf));

	ret = eag_get_nasid(dcli_dbus_connection_curr, hansitype, insid, &nasidconf);
	if (eag_dcli_check_errno(vty, ret, "show nasid error")) {
		return CMD_FAILURE;
	}

	if (0 == nasidconf.current_num){
		vty_out(vty,"no configuration!");
		return CMD_SUCCESS;
	}

	for (i = 0; i < nasidconf.current_num; i++) {
		switch(nasidconf.nasid_map[i].key_type){						
		case NASID_KEYTYPE_WLANID:
			vty_out(vty, "%d.wlanid %lu ", i + 1, nasidconf.nasid_map[i].keywd_1);
			break;
		case NASID_KEYTYPE_VLANID:
			vty_out(vty, "%d.vlanid %lu ", i + 1, nasidconf.nasid_map[i].keywd_1);
			break;
		case NASID_KEYTYPE_WTPID:
			vty_out(vty, "%d.wtpid %lu ", i + 1, nasidconf.nasid_map[i].keywd_1);
			break;
		case NASID_KEYTYPE_IPRANGE:
			{
				char ip_1[24] = {0};
				char ip_2[24] = {0};
				if (nasidconf.nasid_map[i].keywd_1 == nasidconf.nasid_map[i].keywd_2){
					ip2str(nasidconf.nasid_map[i].keywd_1, ip_1, sizeof(ip_1));
					vty_out(vty,"%d.iprange %s ",i+1,ip_1);
				}
				else {
					ip2str(nasidconf.nasid_map[i].keywd_1, ip_1, sizeof(ip_1));
					ip2str(nasidconf.nasid_map[i].keywd_2, ip_2, sizeof(ip_2));
					vty_out(vty,"%d.iprange %s-%s ", i + 1, ip_1, ip_2);
				}
			}
			break;
		case NASID_KEYTYPE_INTF:
			vty_out(vty, "%d.interface %s ", i + 1, nasidconf.nasid_map[i].keystr);
			break;
		default:
			vty_out(vty, "%% nasid_type error\n");
			return EAG_ERR_UNKNOWN;
			break;
		}
		vty_out(vty, "nasid %s syntaxis-point %lu\n", nasidconf.nasid_map[i].nasid, nasidconf.nasid_map[i].conid);
	}
	
	return CMD_SUCCESS;
}
#endif 

 /*nasid config support wlan range */
DEFUN(add_eag_nasid_func,
	add_eag_nasid_cmd,
	"add nasid (wlanid|vlanid|wtpid|iprange|interface) VALUE nasid VALUE syntaxis-point <0-99>",
	"add\n"
	"add nasid\n"
	"wlanid <1-"DEFUN_MAX_WLANID_INPUT">[-<1-"DEFUN_MAX_WLANID_INPUT">]\n"
	"vlanid <1-"DEFUN_MAX_MAPED_VLANID">[-<1-"DEFUN_MAX_MAPED_VLANID">]\n"
	"wtpid <1-"DEFUN_MAX_WTPID_INPUT">[-<1-"DEFUN_MAX_WTPID_INPUT">]\n"
	"ip range A.B.C.D[-A.B.C.D]\n"
	"interface\n"
	"value\n"
	"nasid\n"
	"nasid value\n"
	"syntaxis-point\n"
	"syntaxis-point <0-99>"
)
{
	int ret = 0;
	unsigned long key_type = 0;
	unsigned long keywd_1 = 0;
	unsigned long keywd_2 = 0;
	char *tmp_ip = NULL;
	char *ip = NULL;
	char ip_1[24] = {0};
	char ip_2[24] = {0};
	char *tmp = NULL;
	char *tmp_1 = NULL;

	EAG_DCLI_INIT_HANSI_INFO
		
	struct nasid_map_t nasidmap;
	memset (&nasidmap,0,sizeof(struct nasid_map_t));

	if (0==strncmp(argv[0],"wlanid",strlen(argv[0]))) {
		tmp=(char *)argv[1];
		nasidmap.key_type = NASID_KEYTYPE_WLANID;
		if (NULL != (tmp_1 = strchr(tmp, '-'))) {
			keywd_1 = strtoul(tmp,NULL,10);
			tmp=tmp_1+1;
			keywd_2 = strtoul(tmp,NULL,10);
		} else {
			keywd_1 = strtoul(tmp,NULL,10);
			keywd_2 = keywd_1;
		}
		nasidmap.key.wlanidrange.id_begin = keywd_1;
		nasidmap.key.wlanidrange.id_end = keywd_2;
	} else if (0==strncmp(argv[0],"vlanid",strlen(argv[0]))) {
		tmp=(char *)argv[1];
		nasidmap.key_type = NASID_KEYTYPE_VLANID;
		if (NULL != (tmp_1 = strchr(tmp, '-'))) {
			keywd_1 = strtoul(tmp,NULL,10);
			tmp=tmp_1+1;
			keywd_2 = strtoul(tmp,NULL,10);
		} else {
			keywd_1 = strtoul(tmp,NULL,10);
			keywd_2 = keywd_1;
		}
		nasidmap.key.vlanidrange.id_begin = keywd_1;
		nasidmap.key.vlanidrange.id_end = keywd_2;
	} else if (0==strncmp(argv[0],"wtpid",strlen(argv[0]))) {
		tmp=(char *)argv[1];
		nasidmap.key_type = NASID_KEYTYPE_WTPID;
		if (NULL != (tmp_1 = strchr(tmp, '-'))) {
			keywd_1 = strtoul(tmp,NULL,10);
			tmp=tmp_1+1;
			keywd_2 = strtoul(tmp,NULL,10);
		} else {
			keywd_1 = strtoul(tmp,NULL,10);
			keywd_2 = keywd_1;
		}
		nasidmap.key.wtpidrange.id_begin = keywd_1;
		nasidmap.key.wtpidrange.id_end = keywd_2;
	} else if (0==strncmp(argv[0],"iprange",strlen(argv[0]))) {
		memset (ip_1,0,sizeof(ip_1));
		memset (ip_2,0,sizeof(ip_2));
		nasidmap.key_type = NASID_KEYTYPE_IPRANGE;
		tmp_ip=(char *)argv[1];
		if(NULL==(ip = strchr(tmp_ip, '-'))){
			strncpy(ip_1,tmp_ip,sizeof(ip_1)-1);
			if (eag_check_ip_format(ip_1) != EAG_SUCCESS){
				vty_out(vty, "error ip address input : %s\n", ip_1);
				return CMD_WARNING;
			}
			inet_atoi(ip_1,&nasidmap.key.iprange.ip_begin);
			nasidmap.key.iprange.ip_end=nasidmap.key.iprange.ip_begin;
		} else {
			strncpy(ip_1,tmp_ip,ip-tmp_ip);
			if (eag_check_ip_format(ip_1) != EAG_SUCCESS){
				vty_out(vty, "error ip address input : %s\n", ip_1);
				return CMD_WARNING;
			}
			tmp_ip=ip+1;
			strncpy(ip_2,tmp_ip,sizeof(ip_1)-1);
			if (eag_check_ip_format(ip_2) != EAG_SUCCESS){
				vty_out(vty, "error ip address input : %s\n", ip_2);
				return CMD_WARNING;
			}
			inet_atoi(ip_1,&(nasidmap.key.iprange.ip_begin));
			inet_atoi(ip_2,&(nasidmap.key.iprange.ip_end));
		}
	}else if (0==strncmp(argv[0],"interface",strlen(argv[0]))) { 
		nasidmap.key_type = NASID_KEYTYPE_INTF;
		strncpy(nasidmap.key.intf,(char *)argv[1],MAX_NASID_KEY_BUFF_LEN-1);
	}else {
		vty_out(vty,"add nasid error: type unknown!\n");
		return CMD_WARNING;
	}

	strncpy(nasidmap.nasid,argv[2],MAX_NASID_LEN-1);
	nasidmap.conid = strtoul(argv[3],NULL,10);
	
	ret = eag_add_nasid( dcli_dbus_connection_curr, hansitype, insid, &nasidmap);
	if (eag_dcli_check_errno(vty, ret, "add nasid error")){
		return CMD_FAILURE;
	}

	return CMD_SUCCESS;
}

DEFUN(modify_eag_nasid_func,
	modify_eag_nasid_cmd,
	"modify nasid (wlanid|vlanid|wtpid|iprange|interface) VALUE nasid VALUE syntaxis-point <0-99>",
	"modify\n"
	"modify nasid\n"
	"wlanid <1-"DEFUN_MAX_WLANID_INPUT">[-<1-"DEFUN_MAX_WLANID_INPUT">]\n"
	"vlanid <1-"DEFUN_MAX_MAPED_VLANID">[-<1-"DEFUN_MAX_MAPED_VLANID">]\n"
	"wtpid <1-"DEFUN_MAX_WTPID_INPUT">[-<1-"DEFUN_MAX_WTPID_INPUT">]\n"
	"ip range A.B.C.D[-A.B.C.D]\n"
	"interface\n"
	"value\n"
	"nasid\n"
	"nasid value\n"
	"syntaxis-point\n"
	"syntaxis-point <0-99>"
)
{
	int ret = 0;
	unsigned long key_type = 0;
	unsigned long keywd_1 = 0;
	unsigned long keywd_2 = 0;
	char *tmp_ip = NULL;
	char *ip = NULL;
	char ip_1[24] = {0};
	char ip_2[24] = {0};
	char *tmp = NULL;
	char *tmp_1 = NULL;

	EAG_DCLI_INIT_HANSI_INFO
		
	struct nasid_map_t nasidmap;
	memset (&nasidmap,0,sizeof(struct nasid_map_t));

	if (0==strncmp(argv[0],"wlanid",strlen(argv[0]))) {
		tmp=(char *)argv[1];
		nasidmap.key_type = NASID_KEYTYPE_WLANID;
		if (NULL != (tmp_1 = strchr(tmp, '-'))) {
			keywd_1 = strtoul(tmp,NULL,10);
			tmp=tmp_1+1;
			keywd_2 = strtoul(tmp,NULL,10);
		} else {
			keywd_1 = strtoul(tmp,NULL,10);
			keywd_2 = keywd_1;
		}
		nasidmap.key.wlanidrange.id_begin = keywd_1;
		nasidmap.key.wlanidrange.id_end = keywd_2;
	} else if (0==strncmp(argv[0],"vlanid",strlen(argv[0]))) {
		tmp=(char *)argv[1];
		nasidmap.key_type = NASID_KEYTYPE_VLANID;
		if (NULL != (tmp_1 = strchr(tmp, '-'))) {
			keywd_1 = strtoul(tmp,NULL,10);
			tmp=tmp_1+1;
			keywd_2 = strtoul(tmp,NULL,10);
		} else {
			keywd_1 = strtoul(tmp,NULL,10);
			keywd_2 = keywd_1;
		}
		nasidmap.key.vlanidrange.id_begin = keywd_1;
		nasidmap.key.vlanidrange.id_end = keywd_2;
	} else if (0==strncmp(argv[0],"wtpid",strlen(argv[0]))) {
		tmp=(char *)argv[1];
		nasidmap.key_type = NASID_KEYTYPE_WTPID;
		if (NULL != (tmp_1 = strchr(tmp, '-'))) {
			keywd_1 = strtoul(tmp,NULL,10);
			tmp=tmp_1+1;
			keywd_2 = strtoul(tmp,NULL,10);
		} else {
			keywd_1 = strtoul(tmp,NULL,10);
			keywd_2 = keywd_1;
		}
		nasidmap.key.wtpidrange.id_begin = keywd_1;
		nasidmap.key.wtpidrange.id_end = keywd_2;
	} else if (0==strncmp(argv[0],"iprange",strlen(argv[0]))) {
		memset (ip_1,0,sizeof(ip_1));
		memset (ip_2,0,sizeof(ip_2));
		nasidmap.key_type = NASID_KEYTYPE_IPRANGE;
		tmp_ip=(char *)argv[1];
		if(NULL==(ip = strchr(tmp_ip, '-'))){
			strncpy(ip_1,tmp_ip,sizeof(ip_1)-1);
			if (eag_check_ip_format(ip_1) != EAG_SUCCESS){
				vty_out(vty, "error ip address input : %s\n", ip_1);
				return CMD_WARNING;
			}
			inet_atoi(ip_1,&nasidmap.key.iprange.ip_begin);
			nasidmap.key.iprange.ip_end=nasidmap.key.iprange.ip_begin;
		} else {
			strncpy(ip_1,tmp_ip,ip-tmp_ip);
			if (eag_check_ip_format(ip_1) != EAG_SUCCESS){
				vty_out(vty, "error ip address input : %s\n", ip_1);
				return CMD_WARNING;
			}
			tmp_ip=ip+1;
			strncpy(ip_2,tmp_ip,sizeof(ip_1)-1);
			if (eag_check_ip_format(ip_2) != EAG_SUCCESS){
				vty_out(vty, "error ip address input : %s\n", ip_2);
				return CMD_WARNING;
			}
			inet_atoi(ip_1,&(nasidmap.key.iprange.ip_begin));
			inet_atoi(ip_2,&(nasidmap.key.iprange.ip_end));
		}
	}else if (0==strncmp(argv[0],"interface",strlen(argv[0]))) { 
		nasidmap.key_type = NASID_KEYTYPE_INTF;
		strncpy(nasidmap.key.intf,(char *)argv[1],MAX_NASID_KEY_BUFF_LEN-1);
	}else {
		vty_out(vty,"add nasid error: type unknown!\n");
		return CMD_WARNING;
	}

	strncpy(nasidmap.nasid,argv[2],MAX_NASID_LEN-1);
	nasidmap.conid = strtoul(argv[3],NULL,10);
	
	ret = eag_modify_nasid( dcli_dbus_connection_curr, hansitype, insid, &nasidmap);
	if (eag_dcli_check_errno(vty, ret, "modify nasid error")){
		return CMD_FAILURE;
	}

	return CMD_SUCCESS;
}

DEFUN(del_eag_nasid_func,
	del_eag_nasid_cmd,
	"delete nasid (wlanid|vlanid|wtpid|iprange|interface) VALUE",
	"delete\n"
	"nasid\n"
	"wlanid <1-"DEFUN_MAX_WLANID_INPUT">[-<1-"DEFUN_MAX_WLANID_INPUT">]\n"
	"vlanid <1-"DEFUN_MAX_MAPED_VLANID">[-<1-"DEFUN_MAX_MAPED_VLANID">]\n"
	"wtpid <1-"DEFUN_MAX_WTPID_INPUT">[-<1-"DEFUN_MAX_WTPID_INPUT">]\n"
	"ip range A.B.C.D[-A.B.C.D]\n"
	"interface\n"
	"value\n"
)
{
	int ret=0;
	unsigned long key_type=0;
	unsigned long keywd_1=0;
	unsigned long keywd_2=0;
	char *tmp_ip=NULL;
	char *ip=NULL;
	char ip_1[24];
	char ip_2[24];
	char *tmp = NULL;
	char *tmp_1 = NULL;

	EAG_DCLI_INIT_HANSI_INFO
		
	struct nasid_map_t nasidmap;
	memset (&nasidmap,0,sizeof(struct nasid_map_t));
	
	if (0==strncmp(argv[0],"wlanid",strlen(argv[0]))){
		tmp=(char *)argv[1];
		nasidmap.key_type = NASID_KEYTYPE_WLANID;
		if (NULL != (tmp_1 = strchr(tmp, '-'))) {
			keywd_1 = strtoul(tmp,NULL,10);
			tmp=tmp_1+1;
			keywd_2 = strtoul(tmp,NULL,10);
		} else {
			keywd_1 = strtoul(tmp,NULL,10);
			keywd_2 = keywd_1;
		}
		nasidmap.key.wlanidrange.id_begin = keywd_1;
		nasidmap.key.wlanidrange.id_end = keywd_2;
	}else if(0==strncmp(argv[0],"vlanid",strlen(argv[0]))){
		tmp=(char *)argv[1];
		nasidmap.key_type = NASID_KEYTYPE_VLANID;
		if (NULL != (tmp_1 = strchr(tmp, '-'))) {
			keywd_1 = strtoul(tmp,NULL,10);
			tmp=tmp_1+1;
			keywd_2 = strtoul(tmp,NULL,10);
		} else {
			keywd_1 = strtoul(tmp,NULL,10);
			keywd_2 = keywd_1;
		}
		nasidmap.key.vlanidrange.id_begin = keywd_1;
		nasidmap.key.vlanidrange.id_end = keywd_2;
	}else if(0==strncmp(argv[0],"wtpid",strlen(argv[0]))){
		tmp=(char *)argv[1];
		nasidmap.key_type = NASID_KEYTYPE_WTPID;
		if (NULL != (tmp_1 = strchr(tmp, '-'))) {
			keywd_1 = strtoul(tmp,NULL,10);
			tmp=tmp_1+1;
			keywd_2 = strtoul(tmp,NULL,10);
		} else {
			keywd_1 = strtoul(tmp,NULL,10);
			keywd_2 = keywd_1;
		}
		nasidmap.key.wtpidrange.id_begin = keywd_1;
		nasidmap.key.wtpidrange.id_end = keywd_2;
	}else if(0==strncmp(argv[0],"iprange",strlen(argv[0]))){
		memset (ip_1,0,sizeof(ip_1));
		memset (ip_2,0,sizeof(ip_2));
		nasidmap.key_type = NASID_KEYTYPE_IPRANGE;
		tmp_ip=(char *)argv[1];
		if(NULL==(ip = strchr(tmp_ip, '-'))){
			strncpy(ip_1,tmp_ip,sizeof(ip_1)-1);
			if (eag_check_ip_format(ip_1) != EAG_SUCCESS){
				vty_out(vty, "error ip address input : %s\n", ip_1);
				return CMD_WARNING;
			}
			inet_atoi(ip_1,&nasidmap.key.iprange.ip_begin);
			nasidmap.key.iprange.ip_end=nasidmap.key.iprange.ip_begin;
		}else{
			strncpy(ip_1,tmp_ip,ip-tmp_ip);
			if (eag_check_ip_format(ip_1) != EAG_SUCCESS){
				vty_out(vty, "error ip address input : %s\n", ip_1);
				return CMD_WARNING;
			}
			tmp_ip=ip+1;
			strncpy(ip_2,tmp_ip,24-1);
			if (eag_check_ip_format(ip_2) != EAG_SUCCESS){
				vty_out(vty, "error ip address input : %s\n", ip_2);
				return CMD_WARNING;
			}
			inet_atoi(ip_1,&nasidmap.key.iprange.ip_begin);
			inet_atoi(ip_2,&nasidmap.key.iprange.ip_end);
		}
	}else if(0==strncmp(argv[0],"interface",strlen(argv[0]))){
		nasidmap.key_type = NASID_KEYTYPE_INTF;
		strncpy(nasidmap.key.intf,argv[1],MAX_NASID_KEY_BUFF_LEN-1);
	}else {
		vty_out(vty,"set nasid error: type unknown!\n");
		return CMD_WARNING;
	}
	
	ret = eag_del_nasid( dcli_dbus_connection_curr, hansitype, insid, &nasidmap);
	if (eag_dcli_check_errno(vty,ret, "del nasid error")){
		return CMD_FAILURE;
	}

	return CMD_SUCCESS;
}

DEFUN(show_eag_nasid_func,
	show_eag_nasid_cmd,
	"show eag-nasid-config",
	SHOW_STR
	"eag base config\n"
)
	{
		int ret = 0;
		int i = 0;
	
		EAG_DCLI_INIT_HANSI_INFO
			
		struct api_nasid_conf nasidconf;
		memset(&nasidconf, 0, sizeof(nasidconf));
	
		ret = eag_get_nasid(dcli_dbus_connection_curr, hansitype, insid, &nasidconf);
		if (eag_dcli_check_errno(vty, ret, "show nasid error")) {
			return CMD_FAILURE;
		}
	
		if (0 == nasidconf.current_num){
			vty_out(vty,"no configuration!");
			return CMD_SUCCESS;
		}
	
		for (i = 0; i < nasidconf.current_num; i++) {
			switch(nasidconf.nasid_map[i].key_type){						
			case NASID_KEYTYPE_WLANID:
				if (nasidconf.nasid_map[i].keywd_1 == nasidconf.nasid_map[i].keywd_2) {
					vty_out(vty,"%d.wlanid %lu ", i+1, nasidconf.nasid_map[i].keywd_1);
				} else {
					vty_out(vty,"%d.wlanid %lu-%lu ",i+1,
									nasidconf.nasid_map[i].keywd_1,
									nasidconf.nasid_map[i].keywd_2);
				}
				break;
			case NASID_KEYTYPE_VLANID:
				if (nasidconf.nasid_map[i].keywd_1 == nasidconf.nasid_map[i].keywd_2) {
					vty_out(vty,"%d.vlanid %lu ", i+1, nasidconf.nasid_map[i].keywd_1);
				} else {
					vty_out(vty,"%d.vlanid %lu-%lu ",i+1,
									nasidconf.nasid_map[i].keywd_1,
									nasidconf.nasid_map[i].keywd_2);
				}
				break;
			case NASID_KEYTYPE_WTPID:
				if (nasidconf.nasid_map[i].keywd_1 == nasidconf.nasid_map[i].keywd_2) {
					vty_out(vty,"%d.wtpid %lu ", i+1, nasidconf.nasid_map[i].keywd_1);
				} else {
					vty_out(vty,"%d.wtpid %lu-%lu ", i+1,
									nasidconf.nasid_map[i].keywd_1,
									nasidconf.nasid_map[i].keywd_2);
				}
				break;
			case NASID_KEYTYPE_IPRANGE:
				{
					char ip_1[24]={0};
					char ip_2[24]={0};
					if (nasidconf.nasid_map[i].keywd_1==nasidconf.nasid_map[i].keywd_2){
						ip2str(nasidconf.nasid_map[i].keywd_1,ip_1,sizeof(ip_1));
						vty_out(vty,"%d.iprange %s ",i+1,ip_1);
					} else {
						ip2str(nasidconf.nasid_map[i].keywd_1,ip_1,sizeof(ip_1));
						ip2str(nasidconf.nasid_map[i].keywd_2,ip_2,sizeof(ip_2));
						vty_out(vty,"%d.iprange %s-%s ",i+1,ip_1,ip_2);
					}
				}
				break;
			case NASID_KEYTYPE_INTF:
				vty_out(vty,"%d.interface %s ", i+1, nasidconf.nasid_map[i].keystr);
				break;
			default:
				vty_out(vty, "%% nasid_type error\n");
				return EAG_ERR_UNKNOWN;
				break;
			}
			vty_out(vty, "nasid %s syntaxis-point %lu\n", nasidconf.nasid_map[i].nasid, nasidconf.nasid_map[i].conid);
		}
		
		return CMD_SUCCESS;
	}




#if 0
/* nasportid */
#endif
DEFUN(add_eag_nasportid_func,
	add_eag_nasportid_cmd,
	"add nasportid wlanid VALUE wtpid VALUE vlanid VALUE",
	"add\n"
	"add nasportid\n"
	"wlanid\n"
	"wlanid <1-"DEFUN_MAX_WLANID_INPUT">[-<1-"DEFUN_MAX_WLANID_INPUT">]\n"
	"wtpid\n"
	"wtpid <1-"DEFUN_MAX_WTPID_INPUT">[-<1-"DEFUN_MAX_WTPID_INPUT">]"
	"vlanid\n"
	"vlanid <1-"DEFUN_MAX_MAPED_VLANID">\n"
)
{
	if (!boot_flag) {
		vty_out(vty, "You should use add nasportid wlanid VALUE wtpid VALUE nasportid VALUE\n");
		return CMD_SUCCESS;
	}

	int ret = 0;
	struct eag_id_range_t wlanid = {0};
	struct eag_id_range_t wtpid = {0};
	struct eag_id_range_t vlanid = {0};
	unsigned long nasportid = 0 ;
	char *tmp = NULL;
	char *tmp_1 = NULL;

 	EAG_DCLI_INIT_HANSI_INFO
	
	tmp = (char *)argv[0];
	if (NULL != (tmp_1 = strchr(tmp, '-'))) {
		wlanid.begin = strtoul(tmp, NULL, 10);
		tmp = tmp_1 + 1;
		wlanid.end = strtoul(tmp, NULL, 10);
	}
	else {
		wlanid.begin = strtoul(tmp, NULL, 10);
		wlanid.end = wlanid.begin;
	}
	
	tmp = (char *)argv[1];
	if (NULL != (tmp_1 = strchr(tmp, '-'))) {
		wtpid.begin = strtoul(tmp, NULL, 10);
		tmp = tmp_1 + 1;
		wtpid.end = strtoul(tmp, NULL, 10);
	}
	else{
		wtpid.begin = strtoul(tmp, NULL, 10);
		wtpid.end = wtpid.begin;
	}
	
	tmp = (char *)argv[2];
	nasportid = strtoul(tmp, NULL, 10);
	
	ret = eag_add_nasportid(dcli_dbus_connection_curr, hansitype, insid, wlanid, wtpid, vlanid, NASPORTID_KEYTYPE_WLAN_WTP, nasportid);
	if (eag_dcli_check_errno(vty, ret, "add nasportid error")) {
			return CMD_FAILURE;
	}
	return CMD_SUCCESS;
}

DEFUN(del_eag_nasportid_func,
	del_eag_nasportid_cmd,
	"delete nasportid wlanid VALUE wtpid VALUE vlanid VALUE",
	"delete\n"
	"delete nasportid\n"
	"wlanid\n"
	"wlanid <1-"DEFUN_MAX_WLANID_INPUT">[-<1-"DEFUN_MAX_WLANID_INPUT">]\n"
	"wtpid\n"
	"wtpid <1-"DEFUN_MAX_WTPID_INPUT">[-<1-"DEFUN_MAX_WTPID_INPUT">]"
	"vlanid\n"
	"vlanid <1-"DEFUN_MAX_MAPED_VLANID">\n"
)
{
	if (!boot_flag) {
		vty_out(vty, "You should use del nasportid wlanid VALUE wtpid VALUE nasportid VALUE\n");
		return CMD_SUCCESS;
	}

	int ret = 0;
	struct eag_id_range_t wlanid = {0};
	struct eag_id_range_t wtpid = {0};
	struct eag_id_range_t vlanid = {0};
	unsigned long nasportid = 0;
	char *tmp = NULL;
	char *tmp_1 = NULL;

	EAG_DCLI_INIT_HANSI_INFO

	tmp = (char *)argv[0];
	if (NULL != (tmp_1 = strchr(tmp, '-'))) {
		wlanid.begin = strtoul(tmp, NULL, 10);
		tmp = tmp_1 + 1;
		wlanid.end = strtoul(tmp, NULL, 10);
	}
	else {
		wlanid.begin = strtoul(tmp, NULL, 10);
		wlanid.end = wlanid.begin;
	}
	
	tmp = (char *)argv[1];
	if (NULL != (tmp_1 = strchr(tmp, '-'))){
		wtpid.begin = strtoul(tmp, NULL, 10);
		tmp = tmp_1 + 1;
		wtpid.end = strtoul(tmp, NULL, 10);
	}
	else {
		wtpid.begin = strtoul(tmp, NULL, 10);
		wtpid.end = wtpid.begin;
	}
	
	tmp = (char *)argv[2];
	nasportid= strtoul(tmp, NULL, 10);
	
	ret = eag_del_nasportid(dcli_dbus_connection_curr, hansitype, insid, wlanid, wtpid, vlanid, NASPORTID_KEYTYPE_WLAN_WTP, nasportid);
	if (eag_dcli_check_errno(vty, ret, "del nasportid error")){
			return CMD_FAILURE;
	}
	return CMD_SUCCESS;
}

DEFUN(add_eag_nasportid_wlan_wtp_func,
	add_eag_nasportid_wlan_wtp_cmd,
	"add nasportid wlanid VALUE wtpid VALUE nasportid VALUE",
	"add\n"
	"add nasportid\n"
	"wlanid\n"
	"wlanid <1-"DEFUN_MAX_WLANID_INPUT">[-<1-"DEFUN_MAX_WLANID_INPUT">]\n"
	"wtpid\n"
	"wtpid <1-"DEFUN_MAX_WTPID_INPUT">[-<1-"DEFUN_MAX_WTPID_INPUT">]"
	"nasportid\n"
	"nasportid <1-"DEFUN_MAX_MAPED_NASPORTID">\n"
)
{
	int ret = 0;
	struct eag_id_range_t wlanid = {0};
	struct eag_id_range_t wtpid = {0};
	struct eag_id_range_t vlanid = {0};
	unsigned long nasportid = 0 ;
	char *tmp = NULL;
	char *tmp_1 = NULL;

 	EAG_DCLI_INIT_HANSI_INFO
	
	tmp = (char *)argv[0];
	if (NULL != (tmp_1 = strchr(tmp, '-'))) {
		wlanid.begin = strtoul(tmp, NULL, 10);
		tmp = tmp_1 + 1;
		wlanid.end = strtoul(tmp, NULL, 10);
	}
	else {
		wlanid.begin = strtoul(tmp, NULL, 10);
		wlanid.end = wlanid.begin;
	}
	
	tmp = (char *)argv[1];
	if (NULL != (tmp_1 = strchr(tmp, '-'))) {
		wtpid.begin = strtoul(tmp, NULL, 10);
		tmp = tmp_1 + 1;
		wtpid.end = strtoul(tmp, NULL, 10);
	}
	else{
		wtpid.begin = strtoul(tmp, NULL, 10);
		wtpid.end = wtpid.begin;
	}
	
	tmp = (char *)argv[2];
	nasportid = strtoul(tmp, NULL, 10);
	
	ret = eag_add_nasportid(dcli_dbus_connection_curr, hansitype, insid, wlanid, wtpid, vlanid, NASPORTID_KEYTYPE_WLAN_WTP, nasportid);
	if (eag_dcli_check_errno(vty, ret, "add nasportid error")) {
			return CMD_FAILURE;
	}
	return CMD_SUCCESS;
}

DEFUN(del_eag_nasportid_wlan_wtp_func,
	del_eag_nasportid_wlan_wtp_cmd,
	"delete nasportid wlanid VALUE wtpid VALUE nasportid VALUE",
	"delete\n"
	"delete nasportid\n"
	"wlanid\n"
	"wlanid <1-"DEFUN_MAX_WLANID_INPUT">[-<1-"DEFUN_MAX_WLANID_INPUT">]\n"
	"wtpid\n"
	"wtpid <1-"DEFUN_MAX_WTPID_INPUT">[-<1-"DEFUN_MAX_WTPID_INPUT">]"
	"nasportid\n"
	"nasportid <1-"DEFUN_MAX_MAPED_NASPORTID">\n"
)
{
	int ret = 0;
	struct eag_id_range_t wlanid = {0};
	struct eag_id_range_t wtpid = {0};
	struct eag_id_range_t vlanid = {0};
	unsigned long nasportid = 0;
	char *tmp = NULL;
	char *tmp_1 = NULL;

	EAG_DCLI_INIT_HANSI_INFO

	tmp = (char *)argv[0];
	if (NULL != (tmp_1 = strchr(tmp, '-'))) {
		wlanid.begin = strtoul(tmp, NULL, 10);
		tmp = tmp_1 + 1;
		wlanid.end = strtoul(tmp, NULL, 10);
	}
	else {
		wlanid.begin = strtoul(tmp, NULL, 10);
		wlanid.end = wlanid.begin;
	}
	
	tmp = (char *)argv[1];
	if (NULL != (tmp_1 = strchr(tmp, '-'))){
		wtpid.begin = strtoul(tmp, NULL, 10);
		tmp = tmp_1 + 1;
		wtpid.end = strtoul(tmp, NULL, 10);
	}
	else {
		wtpid.begin = strtoul(tmp, NULL, 10);
		wtpid.end = wtpid.begin;
	}
	
	tmp = (char *)argv[2];
	nasportid= strtoul(tmp, NULL, 10);
	
	ret = eag_del_nasportid(dcli_dbus_connection_curr, hansitype, insid, wlanid, wtpid, vlanid, NASPORTID_KEYTYPE_WLAN_WTP, nasportid);
	if (eag_dcli_check_errno(vty, ret, "del nasportid error")){
			return CMD_FAILURE;
	}
	return CMD_SUCCESS;
}


DEFUN(add_eag_nasportid_vlan_func,
	add_eag_nasportid_vlan_cmd,
	"add nasportid vlanid VALUE nasportid VALUE",
	"add\n"
	"add nasportid\n"
	"vlanid\n"
	"vlanid <1-"DEFUN_MAX_VLANID_INPUT">[-<1-"DEFUN_MAX_VLANID_INPUT">]\n"
	"nasportid\n"
	"nasportid <1-"DEFUN_MAX_MAPED_NASPORTID">\n"
)
{
	int ret = 0;
	struct eag_id_range_t wlanid = {0};
	struct eag_id_range_t wtpid = {0};
	struct eag_id_range_t vlanid = {0};
	unsigned long nasportid = 0 ;
	char *tmp = NULL;
	char *tmp_1 = NULL;

 	EAG_DCLI_INIT_HANSI_INFO
	
	tmp = (char *)argv[0];
	if (NULL != (tmp_1 = strchr(tmp, '-'))) {
		vlanid.begin = strtoul(tmp, NULL, 10);
		tmp = tmp_1 + 1;
		vlanid.end = strtoul(tmp, NULL, 10);
	}
	else {
		vlanid.begin = strtoul(tmp, NULL, 10);
		vlanid.end = vlanid.begin;
	}

	tmp = (char *)argv[1];
	nasportid = strtoul(tmp, NULL, 10);
	
	ret = eag_add_nasportid(dcli_dbus_connection_curr, hansitype, insid, wlanid, wtpid, vlanid, NASPORTID_KEYTYPE_VLAN, nasportid);
	if (eag_dcli_check_errno(vty, ret, "add nasportid error")) {
			return CMD_FAILURE;
	}
	return CMD_SUCCESS;
}

DEFUN(del_eag_nasportid_vlan_func,
	del_eag_nasportid_vlan_cmd,
	"delete nasportid vlanid VALUE nasportid VALUE",
	"delete\n"
	"delete nasportid\n"
	"vlanid\n"
	"vlanid <1-"DEFUN_MAX_VLANID_INPUT">[-<1-"DEFUN_MAX_VLANID_INPUT">]\n"
	"nasportid\n"
	"nasportid <1-"DEFUN_MAX_MAPED_NASPORTID">\n"
)
{
	int ret = 0;
	struct eag_id_range_t wlanid = {0};
	struct eag_id_range_t wtpid = {0};
	struct eag_id_range_t vlanid = {0};
	unsigned long nasportid = 0;
	char *tmp = NULL;
	char *tmp_1 = NULL;

	EAG_DCLI_INIT_HANSI_INFO

	tmp = (char *)argv[0];
	if (NULL != (tmp_1 = strchr(tmp, '-'))) {
		vlanid.begin = strtoul(tmp, NULL, 10);
		tmp = tmp_1 + 1;
		vlanid.end = strtoul(tmp, NULL, 10);
	}
	else {
		vlanid.begin = strtoul(tmp, NULL, 10);
		vlanid.end = vlanid.begin;
	}
	
	tmp = (char *)argv[1];
	nasportid = strtoul(tmp, NULL, 10);
	
	ret = eag_del_nasportid(dcli_dbus_connection_curr, hansitype, insid, wlanid, wtpid, vlanid, NASPORTID_KEYTYPE_VLAN, nasportid);
	if (eag_dcli_check_errno(vty, ret, "del nasportid error")){
			return CMD_FAILURE;
	}
	return CMD_SUCCESS;
}



DEFUN(show_eag_nasportid_func,
	show_eag_nasportid_cmd,
	"show eag-nasportid-config",
	SHOW_STR
	"eag base config\n"
)
{
	int ret=0;
	int i=0;

	EAG_DCLI_INIT_HANSI_INFO
		
	struct nasportid_conf nasportid;
	memset(&nasportid, 0, sizeof(nasportid));
	
	ret = eag_get_nasportid(dcli_dbus_connection_curr, hansitype, insid, &nasportid);
	if (eag_dcli_check_errno(vty, ret, "del nasportid error")) {
			return CMD_FAILURE;
	}

	if (0 == nasportid.current_num){
		vty_out(vty,"no configuration!");
		return CMD_SUCCESS;
	}

	for (i = 0; i < nasportid.current_num; i++) {
		switch(nasportid.nasportid_map[i].key_type) {
		case NASPORTID_KEYTYPE_WLAN_WTP:
			if (nasportid.nasportid_map[i].key.wlan_wtp.wlanid_begin
				== nasportid.nasportid_map[i].key.wlan_wtp.wlanid_end) {
				vty_out(vty,"%d.wlanid %lu ", i+1, nasportid.nasportid_map[i].key.wlan_wtp.wlanid_begin);
			} else {
				vty_out(vty,"%d.wlanid %lu-%lu ", i+1, nasportid.nasportid_map[i].key.wlan_wtp.wlanid_begin,\
						nasportid.nasportid_map[i].key.wlan_wtp.wlanid_end);
			}
			if (nasportid.nasportid_map[i].key.wlan_wtp.wtpid_begin
				== nasportid.nasportid_map[i].key.wlan_wtp.wtpid_end) {
				vty_out(vty,"wtpid %lu ",nasportid.nasportid_map[i].key.wlan_wtp.wtpid_begin);
			} else {
				vty_out(vty,"wtpid %lu-%lu ",nasportid.nasportid_map[i].key.wlan_wtp.wtpid_begin,\
						nasportid.nasportid_map[i].key.wlan_wtp.wtpid_end);
			}
			break;
		case NASPORTID_KEYTYPE_VLAN:
			if (nasportid.nasportid_map[i].key.vlan.vlanid_begin
				== nasportid.nasportid_map[i].key.vlan.vlanid_end) {
				vty_out(vty,"%d.vlanid %lu ", i+1, nasportid.nasportid_map[i].key.vlan.vlanid_begin);
			} else {
				vty_out(vty,"%d.vlanid %lu-%lu ", i+1, nasportid.nasportid_map[i].key.vlan.vlanid_begin,\
						nasportid.nasportid_map[i].key.vlan.vlanid_end);
			}
			break;
		default:
			vty_out(vty, "%% unknown nasportid type\n");
			return CMD_SUCCESS;
		}
		vty_out(vty,"nasportid %lu\n",nasportid.nasportid_map[i].nasportid);
	}

	return CMD_SUCCESS;
}

#if 0
/* statistics */
#endif
#if 0
DEFUN(show_eag_statistics_info_func,
	show_eag_statistics_info_cmd,
	"show eag-statistics-info",
	SHOW_STR
	"show eag statistics info \n"
)
{
	int ret = -1;

	struct list_head ap_stat;
	EAG_DCLI_INIT_HANSI_INFO
	ret = eag_get_ap_statistics(dcli_dbus_connection_curr, hansitype, insid, &ap_stat);
	if(0 == ret){
		struct eag_ap_stat *pos = NULL;
		struct eag_ap_stat *n = NULL;
		struct eag_all_stat eag_stat = {0};
		list_for_each_entry_safe(pos, n, &ap_stat, node) {
			eag_stat.ap_num++;
			eag_stat.online_user_num+=pos->online_user_num;
			eag_stat.user_connect_total_time+=pos->user_connect_total_time;
			
			eag_stat.http_redir_request_count+=pos->http_redir_request_count;			
			eag_stat.http_redir_success_count+=pos->http_redir_success_count;			
														  
			eag_stat.challenge_req_count+=pos->challenge_req_count;			
			eag_stat.challenge_ack_0_count+=pos->challenge_ack_0_count; 		
			eag_stat.challenge_ack_1_count+=pos->challenge_ack_1_count; 		
			eag_stat.challenge_ack_2_count+=pos->challenge_ack_2_count;
			eag_stat.challenge_ack_3_count+=pos->challenge_ack_3_count; 		
			eag_stat.challenge_ack_4_count+=pos->challenge_ack_4_count;
														  
			eag_stat.challenge_timeout_count+=pos->challenge_timeout_count;
			eag_stat.challenge_busy_count+=pos->challenge_busy_count;
														  
			eag_stat.auth_req_count+=pos->auth_req_count;
			eag_stat.auth_ack_0_count+=pos->auth_ack_0_count;				
			eag_stat.auth_ack_1_count+=pos->auth_ack_1_count;	
			eag_stat.auth_ack_2_count+=pos->auth_ack_2_count;						
			eag_stat.auth_ack_3_count+=pos->auth_ack_3_count;			
			eag_stat.auth_ack_4_count+=pos->auth_ack_4_count;
			
			eag_stat.req_auth_unknown_type_count+=pos->req_auth_unknown_type_count;
			eag_stat.req_auth_password_missing_count+=pos->req_auth_password_missing_count;
			eag_stat.ack_auth_busy_count+=pos->ack_auth_busy_count;
			eag_stat.auth_disorder_count+=pos->auth_disorder_count;
																 
			eag_stat.normal_logoff_count+=pos->normal_logoff_count;
			eag_stat.abnormal_logoff_count+=pos->abnormal_logoff_count; 		
			
			eag_stat.access_request_count+=pos->access_request_count;			
			eag_stat.access_request_retry_count+=pos->access_request_retry_count;			
			eag_stat.access_request_timeout_count+=pos->access_request_timeout_count;			
			eag_stat.access_accept_count+=pos->access_accept_count;
			eag_stat.access_reject_count+=pos->access_reject_count;
			
			eag_stat.acct_request_start_count+=pos->acct_request_start_count;
			eag_stat.acct_request_start_retry_count+=pos->acct_request_start_retry_count;			
			eag_stat.acct_response_start_count+=pos->acct_response_start_count; 		
			
			eag_stat.acct_request_update_count+=pos->acct_request_update_count;
			eag_stat.acct_request_update_retry_count+=pos->acct_request_update_retry_count;
			eag_stat.acct_response_update_count+=pos->acct_response_update_count;			
			
			eag_stat.acct_request_stop_count+=pos->acct_request_stop_count;			
			eag_stat.acct_request_stop_retry_count+=pos->acct_request_stop_retry_count; 		
			eag_stat.acct_response_stop_count+=pos->acct_response_stop_count;			
		}
		eag_free_ap_statistics(&ap_stat);
		vty_out(vty, "ap num                          :%lu\n", eag_stat.ap_num);	
		vty_out(vty, "online user num                 :%lu\n", eag_stat.online_user_num);
		vty_out(vty, "user connect total time         :%lu\n", eag_stat.user_connect_total_time);
			
		vty_out(vty, "http redir req count            :%lu\n", eag_stat.http_redir_request_count);			
		vty_out(vty, "http redir req success count    :%lu\n", eag_stat.http_redir_success_count);			
			                                              
		vty_out(vty, "challenge req count             :%lu\n", eag_stat.challenge_req_count);			
		vty_out(vty, "challenge ack success count     :%lu\n", eag_stat.challenge_ack_0_count);			
		vty_out(vty, "challenge ack errcode 1 count   :%lu\n", eag_stat.challenge_ack_1_count);			
		vty_out(vty, "challenge ack errcode 2 count   :%lu\n", eag_stat.challenge_ack_2_count);
		vty_out(vty, "challenge ack errcode 3 count   :%lu\n", eag_stat.challenge_ack_3_count);			
		vty_out(vty, "challenge ack errcode 4 count   :%lu\n", eag_stat.challenge_ack_4_count);
			                                              
		vty_out(vty, "challenge timeout count         :%lu\n", eag_stat.challenge_timeout_count);
		vty_out(vty, "challenge busy count            :%lu\n", eag_stat.challenge_busy_count);
			                                              
		vty_out(vty, "auth req count                  :%lu\n", eag_stat.auth_req_count);
		vty_out(vty, "auth ack success count          :%lu\n", eag_stat.auth_ack_0_count);				
		vty_out(vty, "auth ack errcode 1 count        :%lu\n", eag_stat.auth_ack_1_count);	
		vty_out(vty, "auth ack errcode 2 count        :%lu\n", eag_stat.auth_ack_2_count);						
		vty_out(vty, "auth ack errcode 3 count        :%lu\n", eag_stat.auth_ack_3_count);			
		vty_out(vty, "auth ack errcode 4 count        :%lu\n", eag_stat.auth_ack_4_count);
			
		vty_out(vty, "req auth unknown type count     :%lu\n", eag_stat.req_auth_unknown_type_count);
		vty_out(vty, "req auth password missing count :%lu\n", eag_stat.req_auth_password_missing_count);
		vty_out(vty, "ack auth busy count             :%lu\n", eag_stat.ack_auth_busy_count);
		vty_out(vty, "auth disorder count             :%lu\n", eag_stat.auth_disorder_count);
                                                                 
		vty_out(vty, "normal log out count            :%lu\n", eag_stat.normal_logoff_count);
		vty_out(vty, "abnormal log out count          :%lu\n", eag_stat.abnormal_logoff_count);			
			
		vty_out(vty, "access req count                :%lu\n", eag_stat.access_request_count);			
		vty_out(vty, "access req retry count          :%lu\n", eag_stat.access_request_retry_count);			
		vty_out(vty, "access req timeout count        :%lu\n", eag_stat.access_request_timeout_count);			
		vty_out(vty, "access ack access count         :%lu\n", eag_stat.access_accept_count);
		vty_out(vty, "access ack reject count         :%lu\n", eag_stat.access_reject_count);
			
		vty_out(vty, "acct request start count        :%lu\n", eag_stat.acct_request_start_count);
		vty_out(vty, "acct request start retry count  :%lu\n", eag_stat.acct_request_start_retry_count);			
		vty_out(vty, "acct response start count       :%lu\n", eag_stat.acct_response_start_count);			
			
		vty_out(vty, "acct request update count       :%lu\n", eag_stat.acct_request_update_count);
		vty_out(vty, "acct request update retry count :%lu\n", eag_stat.acct_request_update_retry_count);
		vty_out(vty, "acct response update count      :%lu\n", eag_stat.acct_response_update_count);			
			
		vty_out(vty, "acct request stop count         :%lu\n", eag_stat.acct_request_stop_count);			
		vty_out(vty, "acct request stop retry count   :%lu\n", eag_stat.acct_request_stop_retry_count);			
		vty_out(vty, "acct response stop count        :%lu\n", eag_stat.acct_response_stop_count);	
	}
	else if (EAG_ERR_INPUT_PARAM_ERR == ret) {
		vty_out(vty, "%% input prarm error\n");
	}
	else if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");	
	}
	else if (EAG_ERR_MALLOC_FAILED == ret) {
		vty_out(vty, "%% eag malloc failed error\n" );	
	}
	else {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}

	return CMD_SUCCESS;
	
}
#endif

DEFUN(show_eag_statistics_info_func,
	show_eag_statistics_info_cmd,
	"show eag-statistics-info",
	SHOW_STR
	"show eag statistics info \n"
)
{
	int ret = -1;
	struct eag_all_stat eag_stat = {0};

	EAG_DCLI_INIT_HANSI_INFO
	ret = eag_get_eag_statistics(dcli_dbus_connection_curr, hansitype, insid, &eag_stat);
	if (0 == ret) {
		vty_out(vty, "ap num                          :%lu\n", eag_stat.ap_num);	
		vty_out(vty, "online user num                 :%lu\n", eag_stat.online_user_num);
		vty_out(vty, "user connect total time         :%lu\n", eag_stat.user_connect_total_time);

		vty_out(vty, "macauth online user num         :%lu\n", eag_stat.macauth_online_user_num);
		vty_out(vty, "macauth user connect total time :%lu\n", eag_stat.macauth_user_connect_total_time);
			
		vty_out(vty, "http redir req count            :%lu\n", eag_stat.http_redir_request_count);			
		vty_out(vty, "http redir req success count    :%lu\n", eag_stat.http_redir_success_count);			
			                                              
		vty_out(vty, "challenge req count             :%lu\n", eag_stat.challenge_req_count);			
		vty_out(vty, "challenge ack success count     :%lu\n", eag_stat.challenge_ack_0_count);			
		vty_out(vty, "challenge ack errcode 1 count   :%lu\n", eag_stat.challenge_ack_1_count);			
		vty_out(vty, "challenge ack errcode 2 count   :%lu\n", eag_stat.challenge_ack_2_count);
		vty_out(vty, "challenge ack errcode 3 count   :%lu\n", eag_stat.challenge_ack_3_count);			
		vty_out(vty, "challenge ack errcode 4 count   :%lu\n", eag_stat.challenge_ack_4_count);
			                                              
		vty_out(vty, "challenge timeout count         :%lu\n", eag_stat.challenge_timeout_count);
		vty_out(vty, "challenge busy count            :%lu\n", eag_stat.challenge_busy_count);
			                                              
		vty_out(vty, "auth req count                  :%lu\n", eag_stat.auth_req_count);
		vty_out(vty, "auth ack success count          :%lu\n", eag_stat.auth_ack_0_count);				
		vty_out(vty, "auth ack errcode 1 count        :%lu\n", eag_stat.auth_ack_1_count);	
		vty_out(vty, "auth ack errcode 2 count        :%lu\n", eag_stat.auth_ack_2_count);						
		vty_out(vty, "auth ack errcode 3 count        :%lu\n", eag_stat.auth_ack_3_count);			
		vty_out(vty, "auth ack errcode 4 count        :%lu\n", eag_stat.auth_ack_4_count);

		vty_out(vty, "macauth req count               :%lu\n", eag_stat.macauth_req_count);
		vty_out(vty, "macauth ack success count       :%lu\n", eag_stat.macauth_ack_0_count);				
		vty_out(vty, "macauth ack errcode 1 count     :%lu\n", eag_stat.macauth_ack_1_count);	
		vty_out(vty, "macauth ack errcode 2 count     :%lu\n", eag_stat.macauth_ack_2_count);						
		vty_out(vty, "macauth ack errcode 3 count     :%lu\n", eag_stat.macauth_ack_3_count);			
		vty_out(vty, "macauth ack errcode 4 count     :%lu\n", eag_stat.macauth_ack_4_count);	

		vty_out(vty, "req auth unknown type count     :%lu\n", eag_stat.req_auth_unknown_type_count);
		vty_out(vty, "req auth password missing count :%lu\n", eag_stat.req_auth_password_missing_count);
		vty_out(vty, "ack auth busy count             :%lu\n", eag_stat.ack_auth_busy_count);
		vty_out(vty, "auth disorder count             :%lu\n", eag_stat.auth_disorder_count);
                                                                 
		vty_out(vty, "normal log out count            :%lu\n", eag_stat.normal_logoff_count);
		vty_out(vty, "abnormal log out count          :%lu\n", eag_stat.abnormal_logoff_count);			
		vty_out(vty, "macauth abnormal log out count  :%lu\n", eag_stat.macauth_abnormal_logoff_count);
			
		vty_out(vty, "access req count                :%lu\n", eag_stat.access_request_count);			
		vty_out(vty, "access req retry count          :%lu\n", eag_stat.access_request_retry_count);			
		vty_out(vty, "access req timeout count        :%lu\n", eag_stat.access_request_timeout_count);			
		vty_out(vty, "access ack access count         :%lu\n", eag_stat.access_accept_count);
		vty_out(vty, "access ack reject count         :%lu\n", eag_stat.access_reject_count);
			
		vty_out(vty, "acct request start count        :%lu\n", eag_stat.acct_request_start_count);
		vty_out(vty, "acct request start retry count  :%lu\n", eag_stat.acct_request_start_retry_count);			
		vty_out(vty, "acct response start count       :%lu\n", eag_stat.acct_response_start_count);			
			
		vty_out(vty, "acct request update count       :%lu\n", eag_stat.acct_request_update_count);
		vty_out(vty, "acct request update retry count :%lu\n", eag_stat.acct_request_update_retry_count);
		vty_out(vty, "acct response update count      :%lu\n", eag_stat.acct_response_update_count);			
			
		vty_out(vty, "acct request stop count         :%lu\n", eag_stat.acct_request_stop_count);			
		vty_out(vty, "acct request stop retry count   :%lu\n", eag_stat.acct_request_stop_retry_count);			
		vty_out(vty, "acct response stop count        :%lu\n", eag_stat.acct_response_stop_count);	
	}
	else if (EAG_ERR_INPUT_PARAM_ERR == ret) {
		vty_out(vty, "%% input prarm error\n");
	}
	else if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");	
	}
	else if (EAG_ERR_MALLOC_FAILED == ret) {
		vty_out(vty, "%% eag malloc failed error\n" );	
	}
	else {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}

	return CMD_SUCCESS;
	
}

DEFUN(show_ap_statistics_info_func,
	show_ap_statistics_info_cmd,
	"show ap-statistics-info",
	SHOW_STR
	"show the ap statistics info\n"
)
{
	int ret = -1;

	struct list_head ap_stat;
	char user_mac_str[32] = {0};
	EAG_DCLI_INIT_HANSI_INFO
	ret = eag_get_ap_statistics(dcli_dbus_connection_curr, hansitype, insid, &ap_stat);
	if(0 == ret){
		struct eag_ap_stat *pos = NULL;
		struct eag_ap_stat *n = NULL;
		list_for_each_entry_safe(pos, n, &ap_stat, node) {
			memset(user_mac_str, 0, sizeof(user_mac_str));
			snprintf(user_mac_str, sizeof(user_mac_str)-1,"%02X:%02X:%02X:%02X:%02X:%02X",\
				pos->ap_mac[0], pos->ap_mac[1], pos->ap_mac[2],	pos->ap_mac[3], pos->ap_mac[4], pos->ap_mac[5]);
			vty_out(vty, "apmac                           :%s\n", user_mac_str);	
			vty_out(vty, "online user num                 :%lu\n", pos->online_user_num);
			vty_out(vty, "user connect total time         :%lu\n", pos->user_connect_total_time);

            vty_out(vty, "macauth online user num         :%lu\n", pos->macauth_online_user_num);
            vty_out(vty, "macauth user connect total time :%lu\n", pos->macauth_user_connect_total_time);
			
			vty_out(vty, "http redir req count            :%lu\n", pos->http_redir_request_count);			
			vty_out(vty, "http redir req success count    :%lu\n", pos->http_redir_success_count);			
			                                              
			vty_out(vty, "challenge req count             :%lu\n", pos->challenge_req_count);			
			vty_out(vty, "challenge ack success count     :%lu\n", pos->challenge_ack_0_count);			
			vty_out(vty, "challenge ack errcode 1 count   :%lu\n", pos->challenge_ack_1_count);			
			vty_out(vty, "challenge ack errcode 2 count   :%lu\n", pos->challenge_ack_2_count);
			vty_out(vty, "challenge ack errcode 3 count   :%lu\n", pos->challenge_ack_3_count);			
			vty_out(vty, "challenge ack errcode 4 count   :%lu\n", pos->challenge_ack_4_count);
			                                              
			vty_out(vty, "challenge timeout count         :%lu\n", pos->challenge_timeout_count);
			vty_out(vty, "challenge busy count            :%lu\n", pos->challenge_busy_count);
			                                              
			vty_out(vty, "auth req count                  :%lu\n", pos->auth_req_count);
			vty_out(vty, "auth ack success count          :%lu\n", pos->auth_ack_0_count);				
			vty_out(vty, "auth ack errcode 1 count        :%lu\n", pos->auth_ack_1_count);	
			vty_out(vty, "auth ack errcode 2 count        :%lu\n", pos->auth_ack_2_count);						
			vty_out(vty, "auth ack errcode 3 count        :%lu\n", pos->auth_ack_3_count);			
			vty_out(vty, "auth ack errcode 4 count        :%lu\n", pos->auth_ack_4_count);

			vty_out(vty, "macauth req count               :%lu\n", pos->macauth_req_count);
			vty_out(vty, "macauth ack success count       :%lu\n", pos->macauth_ack_0_count);				
			vty_out(vty, "macauth ack errcode 1 count     :%lu\n", pos->macauth_ack_1_count);	
			vty_out(vty, "macauth ack errcode 2 count     :%lu\n", pos->macauth_ack_2_count);						
			vty_out(vty, "macauth ack errcode 3 count     :%lu\n", pos->macauth_ack_3_count);			
			vty_out(vty, "macauth ack errcode 4 count     :%lu\n", pos->macauth_ack_4_count);

			vty_out(vty, "req auth unknown type count     :%lu\n", pos->req_auth_unknown_type_count);
			vty_out(vty, "req auth password missing count :%lu\n", pos->req_auth_password_missing_count);
			vty_out(vty, "ack auth busy count             :%lu\n", pos->ack_auth_busy_count);
			vty_out(vty, "auth disorder count             :%lu\n", pos->auth_disorder_count);
                                                                 
			vty_out(vty, "normal log out count            :%lu\n", pos->normal_logoff_count);
			vty_out(vty, "abnormal log out count          :%lu\n", pos->abnormal_logoff_count);			
			vty_out(vty, "macauth abnormal log out count  :%lu\n", pos->macauth_abnormal_logoff_count);

			vty_out(vty, "access req count                :%lu\n", pos->access_request_count);			
			vty_out(vty, "access req retry count          :%lu\n", pos->access_request_retry_count);			
			vty_out(vty, "access req timeout count        :%lu\n", pos->access_request_timeout_count);			
			vty_out(vty, "access ack access count         :%lu\n", pos->access_accept_count);
			vty_out(vty, "access ack reject count         :%lu\n", pos->access_reject_count);
			
			vty_out(vty, "acct request start count        :%lu\n", pos->acct_request_start_count);
			vty_out(vty, "acct request start retry count  :%lu\n", pos->acct_request_start_retry_count);			
			vty_out(vty, "acct response start count       :%lu\n", pos->acct_response_start_count);			
			
			vty_out(vty, "acct request update count       :%lu\n", pos->acct_request_update_count);
			vty_out(vty, "acct request update retry count :%lu\n", pos->acct_request_update_retry_count);
			vty_out(vty, "acct response update count      :%lu\n", pos->acct_response_update_count);			
			
			vty_out(vty, "acct request stop count         :%lu\n", pos->acct_request_stop_count);			
			vty_out(vty, "acct request stop retry count   :%lu\n", pos->acct_request_stop_retry_count);			
			vty_out(vty, "acct response stop count        :%lu\n", pos->acct_response_stop_count);			
					
			vty_out(vty, "------------------------------------------------\n");
			
		}
		eag_free_ap_statistics(&ap_stat);
	}
	else if (EAG_ERR_INPUT_PARAM_ERR == ret) {
		vty_out(vty, "%% input prarm error\n");
	}
	else if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");	
	}
	else if (EAG_ERR_MALLOC_FAILED == ret) {
		vty_out(vty, "%% eag malloc failed error\n" );	
	}
	else {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}

	return CMD_SUCCESS;
}



DEFUN(show_bss_statistics_info_func,
	show_bss_statistics_info_cmd,
	"show bss-statistics-info",
	SHOW_STR
	"show the bss statistics info\n"
)
{
	int ret = -1;

	struct list_head bss_stat;
	char user_mac_str[32] = {0};
	EAG_DCLI_INIT_HANSI_INFO
	
	ret = eag_get_bss_statistics(dcli_dbus_connection_curr, hansitype, insid, &bss_stat);
	if (EAG_RETURN_OK == ret){
		struct eag_bss_stat *pos = NULL;
		struct eag_bss_stat *n = NULL;
		list_for_each_entry_safe (pos, n, &bss_stat, node) {
			memset(user_mac_str, 0, sizeof(user_mac_str));
			snprintf(user_mac_str, sizeof(user_mac_str)-1,"%02X:%02X:%02X:%02X:%02X:%02X",\
				pos->ap_mac[0], pos->ap_mac[1], pos->ap_mac[2],	pos->ap_mac[3], pos->ap_mac[4], pos->ap_mac[5]);
			vty_out(vty, "apmac                           :%s\n", user_mac_str);
			vty_out(vty, "wlanid                          :%u\n", pos->wlanid);
			vty_out(vty, "radioid                         :%u\n", pos->radioid);		
			vty_out(vty, "online user num                 :%lu\n", pos->online_user_num);
			vty_out(vty, "user connect total time         :%lu\n", pos->user_connect_total_time);

            vty_out(vty, "macauth online user num         :%lu\n", pos->macauth_online_user_num);
            vty_out(vty, "macauth user connect total time :%lu\n", pos->macauth_user_connect_total_time);
			
			vty_out(vty, "http redir req count            :%lu\n", pos->http_redir_request_count);			
			vty_out(vty, "http redir req success count    :%lu\n", pos->http_redir_success_count);			
			                                              
			vty_out(vty, "challenge req count             :%lu\n", pos->challenge_req_count);			
			vty_out(vty, "challenge ack success count     :%lu\n", pos->challenge_ack_0_count);			
			vty_out(vty, "challenge ack errcode 1 count   :%lu\n", pos->challenge_ack_1_count);			
			vty_out(vty, "challenge ack errcode 2 count   :%lu\n", pos->challenge_ack_2_count);
			vty_out(vty, "challenge ack errcode 3 count   :%lu\n", pos->challenge_ack_3_count);			
			vty_out(vty, "challenge ack errcode 4 count   :%lu\n", pos->challenge_ack_4_count);
			                                              
			vty_out(vty, "challenge timeout count         :%lu\n", pos->challenge_timeout_count);
			vty_out(vty, "challenge busy count            :%lu\n", pos->challenge_busy_count);
			                                              
			vty_out(vty, "auth req count                  :%lu\n", pos->auth_req_count);
			vty_out(vty, "auth ack success count          :%lu\n", pos->auth_ack_0_count);				
			vty_out(vty, "auth ack errcode 1 count        :%lu\n", pos->auth_ack_1_count);	
			vty_out(vty, "auth ack errcode 2 count        :%lu\n", pos->auth_ack_2_count);						
			vty_out(vty, "auth ack errcode 3 count        :%lu\n", pos->auth_ack_3_count);			
			vty_out(vty, "auth ack errcode 4 count        :%lu\n", pos->auth_ack_4_count);

			vty_out(vty, "macauth req count               :%lu\n", pos->macauth_req_count);
			vty_out(vty, "macauth ack success count       :%lu\n", pos->macauth_ack_0_count);				
			vty_out(vty, "macauth ack errcode 1 count     :%lu\n", pos->macauth_ack_1_count);	
			vty_out(vty, "macauth ack errcode 2 count     :%lu\n", pos->macauth_ack_2_count);						
			vty_out(vty, "macauth ack errcode 3 count     :%lu\n", pos->macauth_ack_3_count);			
			vty_out(vty, "macauth ack errcode 4 count     :%lu\n", pos->macauth_ack_4_count);
			
			vty_out(vty, "req auth unknown type count     :%lu\n", pos->req_auth_unknown_type_count);
			vty_out(vty, "req auth password missing count :%lu\n", pos->req_auth_password_missing_count);
			vty_out(vty, "ack auth busy count             :%lu\n", pos->ack_auth_busy_count);
			vty_out(vty, "auth disorder count             :%lu\n", pos->auth_disorder_count);
                                                                 
			vty_out(vty, "normal log out count            :%lu\n", pos->normal_logoff_count);
			vty_out(vty, "abnormal log out count          :%lu\n", pos->abnormal_logoff_count);			
			vty_out(vty, "macauth abnormal log out count  :%lu\n", pos->macauth_abnormal_logoff_count);
		
			vty_out(vty, "access req count                :%lu\n", pos->access_request_count);			
			vty_out(vty, "access req retry count          :%lu\n", pos->access_request_retry_count);			
			vty_out(vty, "access req timeout count        :%lu\n", pos->access_request_timeout_count);			
			vty_out(vty, "access ack access count         :%lu\n", pos->access_accept_count);
			vty_out(vty, "access ack reject count         :%lu\n", pos->access_reject_count);
			
			vty_out(vty, "acct request start count        :%lu\n", pos->acct_request_start_count);
			vty_out(vty, "acct request start retry count  :%lu\n", pos->acct_request_start_retry_count);			
			vty_out(vty, "acct response start count       :%lu\n", pos->acct_response_start_count);			
			
			vty_out(vty, "acct request update count       :%lu\n", pos->acct_request_update_count);
			vty_out(vty, "acct request update retry count :%lu\n", pos->acct_request_update_retry_count);
			vty_out(vty, "acct response update count      :%lu\n", pos->acct_response_update_count);			
			
			vty_out(vty, "acct request stop count         :%lu\n", pos->acct_request_stop_count);			
			vty_out(vty, "acct request stop retry count   :%lu\n", pos->acct_request_stop_retry_count);			
			vty_out(vty, "acct response stop count        :%lu\n", pos->acct_response_stop_count);			
					
			vty_out(vty, "------------------------------------------------\n");
			
		}
		eag_free_bss_statistics(&bss_stat);
	}
	else if (EAG_ERR_INPUT_PARAM_ERR == ret) {
		vty_out(vty, "%% input prarm error\n");
	}
	else if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");	
	}
	else if (EAG_ERR_MALLOC_FAILED == ret) {
		vty_out(vty, "%% eag malloc failed error\n" );	
	}
	else {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}

	return CMD_SUCCESS;
}

#if 0
/* portal server */
#endif

#if 0
"add portal-server (ssid|wlanid|vlanid|wtpid|interface) KEY URL <1-65535> ",
"add portal-server (ssid|wlanid|vlanid|wtpid|interface) KEY URL <1-65535> domain DOMAIN",
"add portal-server (ssid|wlanid|vlanid|wtpid|interface) KEY URL <1-65535> macauth A.B.C.D <1-65535>",
"add portal-server (ssid|wlanid|vlanid|wtpid|interface) KEY URL <1-65535> domain DOMAIN macauth A.B.C.D <1-65535>",
#endif
DEFUN(eag_add_portal_server_func,
	eag_add_portal_server_cmd,
	"add portal-server (wlanid|vlanid|wtpid|interface) KEY URL <1-65535>",
	"add\n"
	"add portal-server url\n"
	"portal server index type wlanid\n"	
	"portal server index type vlanid\n"		
	"portal server index type wtpid\n"	
	"portal server index type l3interface name\n"
	"portal server index key\n"	
	"portal server url(e.g.:http://A.B.C.D:PORT/page.html)\n"
	"portal server ntf_logout port\n"
)
{
	int ret = 0;
	PORTAL_KEY_TYPE key_type;
	unsigned long keyid = 0;
	const char *keystr = "";
	const char *portal_url = argv[2];
	uint16_t ntf_port = atoi(argv[3]);
	const char *domain = "";
	uint32_t mac_server_ip = 0;
	uint16_t mac_server_port = 0;
	struct in_addr addr = {0};

	EAG_DCLI_INIT_HANSI_INFO
#if 0
	if(eag_ins_running_state(dcli_dbus_connection_curr, hansitype, insid)){
		vty_out(vty, "eag instance %d is running, please stop it first\n",insid);
		return CMD_FAILURE;
	}
#endif	
	if (5 == argc) {
		domain = argv[4];
	} else if (6 == argc) {
		ret = inet_aton(argv[4], &addr);
		if (!ret) {
			vty_out(vty, "%% invalid ip address\n");
			return CMD_WARNING;
		}
		mac_server_ip = ntohl(addr.s_addr);
		mac_server_port = atoi(argv[5]);
	} else if (7 == argc) {
		domain = argv[4];
		ret = inet_aton(argv[5], &addr);
		if (!ret) {
			vty_out(vty, "%% invalid ip address\n");
			return CMD_WARNING;
		}
		mac_server_ip = ntohl(addr.s_addr);
		mac_server_port = atoi(argv[6]);
	}

	if (strncmp(argv[0], "wlanid", strlen(argv[0])) == 0) {
		key_type = PORTAL_KEYTYPE_WLANID;
		keyid = atoi(argv[1]);
		if (keyid == 0 || keyid > 128){
			vty_out(vty, "%% wlan id is out of range 1~128\n");
			return CMD_SUCCESS;
		}
	}
	else if (strncmp(argv[0], "vlanid", strlen(argv[0])) == 0) {
		key_type = PORTAL_KEYTYPE_VLANID;
		keyid = atoi(argv[1]);
		if(keyid == 0 || keyid > 4096) {
			vty_out(vty, "%% vlan id is out of range 1~4096\n");
			return CMD_SUCCESS;
		}		
	}
	else if (strncmp(argv[0], "wtpid", strlen(argv[0])) == 0){
		key_type = PORTAL_KEYTYPE_WTPID;
		keyid = atoi(argv[1]);
		if (keyid == 0 || keyid > 2048){
			vty_out(vty, "%% wtp id is out of range 1~2048\n");
			return CMD_SUCCESS;
		}
	}
	else if (strncmp(argv[0], "interface", strlen(argv[0])) == 0) {
		key_type = PORTAL_KEYTYPE_INTF;
		keystr = (char *)argv[1];
	}
	else {
		vty_out(vty, "%% unknown index type %s\n", argv[0]);
		return CMD_SUCCESS;
	}
	
	ret = eag_add_portal_server( dcli_dbus_connection_curr,
										hansitype,insid,					
										key_type,
										keyid,
										keystr,
										portal_url, 
										ntf_port,
										domain,
										mac_server_ip,
										mac_server_port);
	if(EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_ERR_UNKNOWN == ret) {
		vty_out(vty, "%% unknown error\n");
	}
	else if (EAG_ERR_PORTAL_ADD_SRV_KEY_EXIST == ret) {
		vty_out(vty, "%% the add key is exist\n");
	}
	else if (EAG_ERR_PORTAL_ADD_SRV_MAX_NUM == ret) {
		vty_out(vty, "%% MAX number limite:%d\n", MAX_PORTAL_NUM);
	}
	else if (EAG_ERR_PORTAL_ADD_SRV_URL_LEN_LIMITE == ret) {
		vty_out(vty, "%% length of url is over limite:%d\n", MAX_PORTAL_URL_LEN);
	}
	else if (EAG_ERR_PORTAL_ADD_SRV_ERR_TYPE == ret) {
		vty_out(vty, "%% error index type\n");
	}
	else if (EAG_ERR_PORTAL_ADD_SRV_INTF_LEN_LIMITE == ret) {
		vty_out(vty, "%% length of intf is over limite:%d\n", MAX_PORTAL_KEY_BUFF_LEN);
	}
	else if (EAG_RETURN_OK != ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}
	
	return CMD_SUCCESS;
}

ALIAS(eag_add_portal_server_func,
	eag_add_portal_server_cmd_domain,
	"add portal-server (wlanid|vlanid|wtpid|interface) KEY URL <1-65535> domain DOMAIN",
	"add\n"
	"add portal-server url\n"
	"portal server index type wlanid\n"	
	"portal server index type vlanid\n"		
	"portal server index type wtpid\n"	
	"portal server index type l3interface name\n"
	"portal server index key\n"	
	"portal server url(e.g.:http://A.B.C.D:PORT/page.html)\n"
	"portal server ntf_logout port\n"
	"portal server bind to radius domain\n"
	"portal server bind to radius domain\n"
)

ALIAS(eag_add_portal_server_func,
	eag_add_portal_server_cmd_macauth,
	"add portal-server (wlanid|vlanid|wtpid|interface) KEY URL <1-65535> macauth A.B.C.D <1-65535>",
	"add\n"
	"add portal-server url\n"
	"portal server index type wlanid\n"	
	"portal server index type vlanid\n"		
	"portal server index type wtpid\n"	
	"portal server index type l3interface name\n"
	"portal server index key\n"	
	"portal server url(e.g.:http://A.B.C.D:PORT/page.html)\n"
	"portal server ntf_logout port\n"
	"mac auth server\n"
	"mac auth server ip\n"
	"mac auth server port\n"
)

ALIAS(eag_add_portal_server_func,
	eag_add_portal_server_cmd_domain_macauth,
	"add portal-server (wlanid|vlanid|wtpid|interface) KEY URL <1-65535> domain DOMAIN macauth A.B.C.D <1-65535>",
	"add\n"
	"add portal-server url\n"
	"portal server index type wlanid\n"	
	"portal server index type vlanid\n"		
	"portal server index type wtpid\n"	
	"portal server index type l3interface name\n"
	"portal server index key\n"	
	"portal server url(e.g.:http://A.B.C.D:PORT/page.html)\n"
	"portal server ntf_logout port\n"
	"portal server bind to radius domain\n"
	"portal server bind to radius domain\n"
	"mac auth server\n"
	"mac auth server ip\n"
	"mac auth server port\n"
)

DEFUN(eag_add_portal_server_essid_func,
	eag_add_portal_server_essid_cmd,
	"add portal-server essid .ARGUMENTS",
	"add\n"
	"add portal-server url\n"
	"portal server index type essid\n"	
	"eg:add portal-server essid KEY_WORD URL PORT [domain DOMAIN] [macauth A.B.C.D <1-65535>]\n"

)
{
	int ret = -1;
	int i = 0;
	PORTAL_KEY_TYPE key_type = PORTAL_KEYTYPE_ESSID;
	unsigned long keyid = 0;
	const char *keystr = "";
	const char *portal_url = "";
	int ntf_port = 0;
	const char *domain = "";
	char essid[MAX_PORTAL_SSID_LEN] = "";
	char http_str[10] = "http://";
	char https_str[10] = "https://";
	uint32_t mac_server_ip = 0;
	uint16_t mac_server_port = 0;
	struct in_addr addr = {0};
	const char *tmp_port = NULL;

	for (i = 0; i < argc && 0 != strncmp(argv[i], http_str, strlen(http_str))
		&& 0 != strncmp(argv[i], https_str, strlen(https_str)); i++) 
	{
		if (0 != i) {
			strncat(essid, " ", 1);
		}
		if((strlen(essid) + strlen(argv[i])) > MAX_PORTAL_SSID_LEN-1) {
			vty_out(vty, "essid length is too long\n");
			return CMD_SUCCESS;
		}
		strncat(essid, argv[i], strlen(argv[i]));
		keystr = essid;
	}
	if (strlen(keystr) == 0) {
		vty_out(vty, "essid is null\n");
		return CMD_SUCCESS;
	}
	
	if (i + 2 == argc) {	;
	} else if (i + 4 == argc) {
		if (0 != strncmp(argv[i+2], "domain", strlen(argv[i+2]))) {
			vty_out(vty, "Used as: add portal-server essid KEY_WORD URL PORT [domain DOMAIN] [macauth A.B.C.D <1-65535>]\n");
			return CMD_SUCCESS;
		}
		domain = argv[i+3];
	} else if (i + 5 == argc) {
		if (0 != strncmp(argv[i+2], "macauth", strlen(argv[i+2]))) {
			vty_out(vty, "Used as: add portal-server essid KEY_WORD URL PORT [domain DOMAIN] [macauth A.B.C.D <1-65535>]\n");
			return CMD_SUCCESS;
		}
		ret = inet_aton(argv[i+3], &addr);
		if (!ret) {
			vty_out(vty, "%% invalid ip address\n");
			return CMD_WARNING;
		}
		mac_server_ip = ntohl(addr.s_addr);
		mac_server_port = atoi(argv[i+4]);
	} else if (i + 7 == argc) {
		if (0 != strncmp(argv[i+2], "domain", strlen(argv[i+2]))) {
			vty_out(vty, "Used as: add portal-server essid KEY_WORD URL PORT [domain DOMAIN] [macauth A.B.C.D <1-65535>]\n");
			return CMD_SUCCESS;
		}
		domain = argv[i+3];
		if (0 != strncmp(argv[i+4], "macauth", strlen(argv[i+4]))) {
			vty_out(vty, "Used as: add portal-server essid KEY_WORD URL PORT [domain DOMAIN] [macauth A.B.C.D <1-65535>]\n");
			return CMD_SUCCESS;
		}
		ret = inet_aton(argv[i+5], &addr);
		if (!ret) {
			vty_out(vty, "%% invalid ip address\n");
			return CMD_WARNING;
		}
		mac_server_ip = ntohl(addr.s_addr);
		mac_server_port = atoi(argv[i+6]);
	} else if (i + 7 < argc) {
		vty_out(vty, "%% too many argument\n");
		return CMD_SUCCESS;
	} else if (i + 2 > argc) {
		vty_out(vty, "%% too few argument\n");
		return CMD_SUCCESS;
	} else {
		vty_out(vty, "Used as: add portal-server essid KEY_WORD URL PORT [domain DOMAIN] [macauth A.B.C.D <1-65535>]\n");
		return CMD_SUCCESS;
	}
	portal_url = argv[i];
	tmp_port = argv[++i];
	if (0 == is_number(tmp_port)) {
		vty_out(vty, "%% ntf port has illegal character\n");
		return CMD_SUCCESS;
	}
	ntf_port = atoi(tmp_port);
	if (ntf_port <= 0 || ntf_port > 65535) {
		vty_out(vty, "%% ntf port must be in range 1~65535 \n");
		return CMD_SUCCESS;
	}

	EAG_DCLI_INIT_HANSI_INFO
#if 0		
	if(eag_ins_running_state(dcli_dbus_connection_curr,hansitype, insid)){
		vty_out(vty, "eag instance %d is running, please stop it first\n",insid);
		return CMD_FAILURE;
	}
#endif	

		
	ret = eag_add_portal_server( dcli_dbus_connection_curr,
										hansitype,insid,					
										key_type,
										keyid,
										keystr,
										portal_url, 
										ntf_port,
										domain,
										mac_server_ip,
										mac_server_port);
	if(EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_ERR_UNKNOWN == ret) {
		vty_out(vty, "%% unknown error\n");
	}
	else if (EAG_ERR_PORTAL_ADD_SRV_KEY_EXIST == ret) {
		vty_out(vty, "%% the add key is exist\n");
	}
	else if (EAG_ERR_PORTAL_ADD_SRV_MAX_NUM == ret) {
		vty_out(vty, "%% MAX number limite:%d\n", MAX_PORTAL_NUM);
	}
	else if (EAG_ERR_PORTAL_ADD_SRV_URL_LEN_LIMITE == ret) {
		vty_out(vty, "%% length of url is over limite:%d\n", MAX_PORTAL_URL_LEN);
	}
	else if (EAG_ERR_PORTAL_ADD_SRV_ERR_TYPE == ret) {
		vty_out(vty, "%% error index type\n");
	}
	else if (EAG_RETURN_OK != ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}
	
	return CMD_SUCCESS;
}

DEFUN(eag_set_macbind_server_func,
	eag_set_macbind_server_cmd,
	"set macbind-server (wlanid|vlanid|wtpid|interface) KEY (ip|domain) ADDRESS <1-65535>",
	"set\n"
	"set macbind-server\n"
	"macbind server index type wlanid\n"
	"macbind server index type vlanid\n"
	"macbind server index type wtpid\n"
	"macbind server index type l3interface name\n"
	"macbind server index key\n"
	"macbind server ip\n"
	"macbind server domain\n"
	"ip or domain\n"
	"macbind server port\n"
)
{
	int ret = -1;
	PORTAL_KEY_TYPE key_type = PORTAL_KEYTYPE_ESSID;
	unsigned long keyid = 0;
	const char *keystr = "";
	uint32_t macbind_server_ip = 0;
	uint16_t macbind_server_port = 0;
	struct in_addr addr = {0};
	const char *tmp_port = NULL;
    char domain[MAX_MACBIND_SERVER_DOMAIN_LEN] = "";
    domain_pt domain_conf;
    domain_ct domain_ctr;
    int slotid = HostSlotId;
    int ip_or_domain = 0;
    int i = 0;

	if (strncmp(argv[0], "wlanid", strlen(argv[0])) == 0) {
		key_type = PORTAL_KEYTYPE_WLANID;
		keyid = atoi(argv[1]);
		if (keyid == 0 || keyid > 128){
			vty_out(vty, "%% wlan id is out of range 1~128\n");
			return CMD_SUCCESS;
		}
	}
	else if (strncmp(argv[0], "vlanid", strlen(argv[0])) == 0) {
		key_type = PORTAL_KEYTYPE_VLANID;
		keyid = atoi(argv[1]);
		if(keyid == 0 || keyid > 4096) {
			vty_out(vty, "%% vlan id is out of range 1~4096\n");
			return CMD_SUCCESS;
		}		
	}
	else if (0 == strncmp(argv[0], "wtpid", strlen(argv[0]))){
		keyid = atoi(argv[1]);
		key_type = PORTAL_KEYTYPE_WTPID;
	}
	else if (0 == strncmp(argv[0], "interface", strlen(argv[0]))) {
		key_type = PORTAL_KEYTYPE_INTF;
		keystr = (char *)argv[1];
	}
	else {
		vty_out(vty, "%% unknown index type %s\n", argv[0]);
		return CMD_SUCCESS;
	}

	EAG_DCLI_INIT_HANSI_INFO
	
#if 0		
    if(eag_ins_running_state(dcli_dbus_connection_curr,hansitype, insid)){
        vty_out(vty, "eag instance %d is running, please stop it first\n",insid);
        return CMD_FAILURE;
    }
#endif

	if (0 == strcmp(argv[2], "ip")) {
        memset(&addr, 0, sizeof(addr));
		ret = inet_aton(argv[3], &addr);
		if (!ret) {
			vty_out(vty, "%% invalid ip address\n");
			return CMD_WARNING;
		}
		macbind_server_ip = ntohl(addr.s_addr);
		macbind_server_port = atoi(argv[4]);
		ip_or_domain = MACBIND_SERVER_IP;
        //vty_out(vty, "%% ip:%x port:%x\n", macbind_server_ip, macbind_server_port);
	} else if (0 == strcmp(argv[2], "domain")) {
        if (MAX_MACBIND_SERVER_DOMAIN_LEN < strlen(argv[3]) + 1) {
            vty_out(vty, "%% this domain is too long, out of limite:%d\n", MAX_MACBIND_SERVER_DOMAIN_LEN - 1);
            return CMD_SUCCESS;
        }
        if (0 == is_domain(argv[3])) {
            vty_out(vty, "%% this domain format error, please check it\n");
            return CMD_SUCCESS;
        }
        memset(domain, 0, sizeof(domain));
        strncpy(domain, argv[3], sizeof(domain) - 1);
        
        ret = conf_drp_get_dbus_connect_params(&slotid);
        if (ret < 0){
            vty_out(vty, "%% eag get drp connection config error:%d\n",ret);
            return CMD_FAILURE;
        }
        memset(&domain_ctr, 0, sizeof(domain_ctr));
        memset(&domain_conf, 0, sizeof(domain_conf));
        strncpy((domain_conf.domain_name), domain, sizeof(domain_conf.domain_name) - 1);
        
        ReInitDbusConnection(&dcli_dbus_connection_curr, slotid, distributFag);
        ret = conf_drp_get_domain_ip(dcli_dbus_connection_curr, &domain_conf, &domain_ctr);
        _drp_return_if_fail(0 == ret,ret,CMD_WARNING);
        
        ReInitDbusConnection(&dcli_dbus_connection_curr, slot_id, distributFag);
        
        if (0 == domain_ctr.num) {
            vty_out(vty, "%% this domain can not parse, please check it\n");
            return CMD_SUCCESS;
        }
        //vty_out(vty, "conf_drp_get_domain_ip ret = %d\n",ret);
        if ( 0 == ret){
            //vty_out(vty, "domain %s ip num %d\n", domain_ctr.domain_name, domain_ctr.num);
            if (0 < domain_ctr.num){
                macbind_server_ip = domain_ctr.domain_ip[0].ipaddr;
            }
        }
		macbind_server_port = atoi(argv[4]);
		ip_or_domain = MACBIND_SERVER_DOMAIN;
        //vty_out(vty, "%% ip:%x port:%x\n", macbind_server_ip, macbind_server_port);
	} else {
		vty_out(vty, "%% invalid input format!\n");
		return CMD_WARNING;
	}
	
	if (0 == macbind_server_ip 
		|| 0 == macbind_server_port) {
		vty_out(vty, "%% invalid ip or port\n");
        return CMD_SUCCESS;
	}
	
	ret = eag_set_macbind_server( dcli_dbus_connection_curr,
										hansitype,insid,					
										key_type,
										keyid,
										keystr,
										ip_or_domain,
										domain,
										macbind_server_ip,
										macbind_server_port);
	if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_ERR_INPUT_PARAM_ERR == ret) {
		vty_out(vty, "%% input prarm error\n");
	}
	else if (EAG_ERR_PORTAL_ADD_SRV_ERR_TYPE == ret) {
		vty_out(vty, "%% keyword type error\n");
	}
	else if (EAG_ERR_PORTAL_MODIFY_SRV_NOT_EXIST == ret) {
		vty_out(vty, "%% the add key is not exist\n");
	}
	else if (EAG_ERR_PORTAL_MODIFY_SRV_ERR_TYPE == ret) {
		vty_out(vty, "%% error index type\n");
	}
	else if (EAG_RETURN_OK != ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}
    return CMD_SUCCESS;
}

DEFUN(eag_set_macbind_server_essid_func,
	eag_set_macbind_server_essid_cmd,
	"set macbind-server essid .ARGUMENTS",
	"set\n"
	"set macbind-server\n"
	"macbind server index type essid\n"
	"eg:set macbind-server essid KEY (ip|domain) ADDRESS <1-65535>"
)
{
	int ret = -1;
	PORTAL_KEY_TYPE key_type = PORTAL_KEYTYPE_ESSID;
	unsigned long keyid = 0;
	const char *keystr = "";
	char essid[MAX_PORTAL_SSID_LEN] = "";
	uint32_t macbind_server_ip = 0;
	uint16_t macbind_server_port = 0;
	struct in_addr addr = {0};
    char domain[MAX_DOMAIN_NAME_LEN];
    domain_pt domain_conf;
    domain_ct domain_ctr;
    int slotid = HostSlotId;
    int ip_or_domain = 0;
    int i = 0;
    
    memset(essid, 0, sizeof(essid));
	for (i = 0;i < argc; i++) {
		if (0 != i) {
			if (0 == strcmp(argv[i], "ip")) {
                ip_or_domain = MACBIND_SERVER_IP;
				break;
			} else if (0 == strcmp(argv[i], "domain")) {
                ip_or_domain = MACBIND_SERVER_DOMAIN;
				break;
			}
			strncat(essid, " ", 1);
		}
		if ((strlen(essid) + strlen(argv[i])) > MAX_PORTAL_SSID_LEN-1) {
			vty_out(vty, "%% essid length is too long\n");
			return CMD_SUCCESS;
		}
		strncat(essid, argv[i], strlen(argv[i]));
		keystr = essid;
	}
	
	if (strlen(keystr) == 0) {
		vty_out(vty, "essid is null\n");
		return CMD_SUCCESS;
	}

	EAG_DCLI_INIT_HANSI_INFO
	
#if 0		
	if(eag_ins_running_state(dcli_dbus_connection_curr,hansitype, insid)){
		vty_out(vty, "eag instance %d is running, please stop it first\n",insid);
		return CMD_FAILURE;
	}
#endif

	switch(ip_or_domain) {
	case MACBIND_SERVER_IP:
		if ((i+3) == argc) {
            memset(&addr, 0, sizeof(addr));
            ret = inet_aton((char *)argv[i+1], &addr);
            if (!ret) {
                vty_out(vty, "%% invalid ip address\n");
                return CMD_WARNING;
            }
            macbind_server_ip = ntohl(addr.s_addr);
            macbind_server_port = atoi(argv[i+2]);
            //vty_out(vty, "%% ip:%x port:%x\n", macbind_server_ip, macbind_server_port);
		} else {
			vty_out(vty, "%% argument is wrong\n");
		}
		break;
	case MACBIND_SERVER_DOMAIN:
		if ((i+3) == argc){
            if (MAX_MACBIND_SERVER_DOMAIN_LEN < strlen(argv[i+1]) + 1) {
                vty_out(vty, "%% this domain is too long, out of limite:%d\n", MAX_MACBIND_SERVER_DOMAIN_LEN - 1);
                return CMD_SUCCESS;
            }
            if (0 == is_domain(argv[i+1])) {
                vty_out(vty, "%% this domain format error, please check it\n");
                return CMD_SUCCESS;
            }
            memset(domain, 0, sizeof(domain));
            strncpy(domain, argv[i+1], sizeof(domain) - 1);
            
            ret = conf_drp_get_dbus_connect_params(&slotid);
            if (ret < 0){
                vty_out(vty, "%% eag get drp connection config error:%d\n",ret);
                return CMD_FAILURE;
            }
            memset(&domain_ctr, 0, sizeof(domain_ctr));
            memset(&domain_conf, 0, sizeof(domain_conf));
            strncpy((domain_conf.domain_name), domain, sizeof(domain_conf.domain_name) - 1);
            
            ReInitDbusConnection(&dcli_dbus_connection_curr, slotid, distributFag);
            ret = conf_drp_get_domain_ip(dcli_dbus_connection_curr, &domain_conf, &domain_ctr);
            _drp_return_if_fail(0 == ret,ret,CMD_WARNING);
            
            ReInitDbusConnection(&dcli_dbus_connection_curr, slot_id, distributFag);
            
            if (0 == domain_ctr.num) {
                vty_out(vty, "%% this domain can not parse, please check it\n");
                return CMD_SUCCESS;
            }
           // vty_out(vty, "conf_drp_get_domain_ip ret = %d\n",ret);
            if ( 0 == ret){
                //vty_out(vty, "domain %s ip num %d\n", domain_ctr.domain_name, domain_ctr.num);
                if (0 < domain_ctr.num){
                    macbind_server_ip = domain_ctr.domain_ip[0].ipaddr;
                }
            }
            macbind_server_port = atoi(argv[i+2]);
            //vty_out(vty, "%% ip:%x port:%x\n", macbind_server_ip, macbind_server_port);
        } else {
            vty_out(vty, "%% invalid input format!\n");
            return CMD_WARNING;
        }
	}
    if (0 == macbind_server_ip 
        || 0 == macbind_server_port) {
        vty_out(vty, "%% invalid ip or port\n");
        return CMD_SUCCESS;
    }
    
    ret = eag_set_macbind_server( dcli_dbus_connection_curr,
                                        hansitype,insid,                    
                                        key_type,
                                        keyid,
                                        keystr,
                                        ip_or_domain,
                                        domain,
                                        macbind_server_ip,
                                        macbind_server_port);
	if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_ERR_INPUT_PARAM_ERR == ret) {
		vty_out(vty, "%% input prarm error\n");
	}
	else if (EAG_ERR_PORTAL_ADD_SRV_ERR_TYPE == ret) {
		vty_out(vty, "%% keyword type error\n");
	}
	else if (EAG_ERR_PORTAL_MODIFY_SRV_NOT_EXIST == ret) {
		vty_out(vty, "%% the add key is not exist\n");
	}
	else if (EAG_ERR_PORTAL_MODIFY_SRV_ERR_TYPE == ret) {
		vty_out(vty, "%% error index type\n");
	}
	else if (EAG_RETURN_OK != ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}
    return CMD_SUCCESS;
}

DEFUN(set_eag_portal_server_acname_func,
	set_eag_portal_server_acname_cmd,
	"set portal-server (wlanid|vlanid|wtpid|interface) KEY ac-name [ACNAME]",
	"set parameter of portal policy, if not exist, create it\n"
	"portal policy\n"
	"portal server index type wlanid\n"	
	"portal server index type vlanid\n"		
	"portal server index type wtpid\n"	
	"portal server index type l3interface name\n"
	"key word name, specify a item of portal policy\n"
	"AC's name\n"
	"AC's, to be blank if not fill up\n"
)
{
	char *key_word = (char *)argv[0], *acname = NULL;
	unsigned long keyid = 0;
	PORTAL_KEY_TYPE key_type;
	int ret = -1;
	if (3 == argc){
		acname = (char *)argv[2];
		if (strlen(acname) > MAX_PORTAL_ACNAME_LEN-1){
			vty_out(vty, "error acname format\n");
			return CMD_WARNING;
		}
	}
	else
		acname = "";

	if (strncmp(argv[0], "wlanid", strlen(argv[0])) == 0) {
		key_type = PORTAL_KEYTYPE_WLANID;
		keyid = atoi(argv[1]);
		if (keyid == 0 || keyid > 128){
			vty_out(vty, "%% wlan id is out of range 1~128\n");
			return CMD_SUCCESS;
		}
	}
	else if (strncmp(argv[0], "vlanid", strlen(argv[0])) == 0) {
		key_type = PORTAL_KEYTYPE_VLANID;
		keyid = atoi(argv[1]);
		if(keyid == 0 || keyid > 4096) {
			vty_out(vty, "%% vlan id is out of range 1~4096\n");
			return CMD_SUCCESS;
		}		
	}
	else if (strncmp(argv[0], "wtpid", strlen(argv[0])) == 0){
		keyid = atoi(argv[1]);
		key_type = PORTAL_KEYTYPE_WTPID;
	}
	else if (strncmp(argv[0], "interface", strlen(argv[0])) == 0) {
		key_type = PORTAL_KEYTYPE_INTF;
		key_word = (char *)argv[1];
	}
	else {
		vty_out(vty, "%% unknown index type %s\n", argv[0]);
		return CMD_SUCCESS;
	}
	
	EAG_DCLI_INIT_HANSI_INFO
#if 0
	if(eag_ins_running_state(dcli_dbus_connection_curr, hansitype, insid)){
		vty_out(vty, "eag instance %d is running, please stop it first\n",insid);
		return CMD_FAILURE;
	}
#endif
	ret = eag_set_portal_server_acname( dcli_dbus_connection_curr,
										hansitype,insid,
										key_type,
										keyid,
										key_word,
										acname);
	
	if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_ERR_INPUT_PARAM_ERR == ret) {
		vty_out(vty, "%% input prarm error\n");
	}
	else if (EAG_ERR_PORTAL_ADD_SRV_ERR_TYPE == ret) {
		vty_out(vty, "%% keyword type error\n");
	}
	else if (EAG_ERR_PORTAL_MODIFY_SRV_NOT_EXIST == ret) {
		vty_out(vty, "%% the add key is not exist\n");
	}
	else if (EAG_ERR_PORTAL_MODIFY_SRV_ERR_TYPE == ret) {
		vty_out(vty, "%% error index type\n");
	}
	else if (EAG_RETURN_OK != ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}
}

DEFUN(set_eag_portal_server_acip_to_url_func,
	set_eag_portal_server_acip_to_url_cmd,
	"set portal-server (wlanid|vlanid|wtpid|interface) KEY acip-to-url (enable|disable)",
	"set parameter of portal policy, if not exist, create it\n"
	"portal policy\n"
	"portal server index type wlanid\n" 
	"portal server index type vlanid\n" 	
	"portal server index type wtpid\n"	
	"portal server index type l3interface name\n"
	"key word name, specify a item of portal policy\n"
	"add acip infornation to url\n"
	"add acip infornation to url enable\n"
	"add acip infornation to url disable\n"
)
{
	int ret = -1;
	PORTAL_KEY_TYPE key_type;
	unsigned long keyid = 0;
	char *keystr = "";
	int acip_to_url = 0;

	if (strncmp(argv[0], "wlanid", strlen(argv[0])) == 0) {
		key_type = PORTAL_KEYTYPE_WLANID;
		keyid = atoi(argv[1]);
		if (keyid == 0 || keyid > 128){
			vty_out(vty, "%% wlan id is out of range 1~128\n");
			return CMD_SUCCESS;
		}
	}
	else if (strncmp(argv[0], "vlanid", strlen(argv[0])) == 0) {
		key_type = PORTAL_KEYTYPE_VLANID;
		keyid = atoi(argv[1]);
		if(keyid == 0 || keyid > 4096) {
			vty_out(vty, "%% vlan id is out of range 1~4096\n");
			return CMD_SUCCESS;
		}		
	}
	else if (0 == strncmp(argv[0], "wtpid", strlen(argv[0]))){
		keyid = atoi(argv[1]);
		key_type = PORTAL_KEYTYPE_WTPID;
	}
	else if (0 == strncmp(argv[0], "interface", strlen(argv[0]))) {
		key_type = PORTAL_KEYTYPE_INTF;
		keystr = (char *)argv[1];
	}
	else {
		vty_out(vty, "%% unknown index type %s\n", argv[0]);
		return CMD_SUCCESS;
	}

	if (0 == strncmp(argv[2],"enable",strlen(argv[2])))
	{
		acip_to_url = 1;
	}
	else if (0 == strncmp(argv[2],"disable",strlen(argv[2])))
	{
		acip_to_url = 0;
	}
	else
	{
		vty_out(vty, "%% bad command parameter\n");
		return CMD_SUCCESS;
	}
	EAG_DCLI_INIT_HANSI_INFO

	ret = eag_set_portal_server_acip_to_url( dcli_dbus_connection_curr,
										hansitype,insid,
										key_type,
										keyid,
										keystr,
										acip_to_url);
	if(EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_ERR_UNKNOWN == ret) {
		vty_out(vty, "%% unknown error\n");
	}
	else if (EAG_ERR_PORTAL_MODIFY_SRV_NOT_EXIST == ret) {
		vty_out(vty, "%% the add key is not exist\n");
	}
	else if (EAG_ERR_PORTAL_MODIFY_SRV_ERR_TYPE == ret) {
		vty_out(vty, "%% error index type\n");
	}
	else if (EAG_RETURN_OK != ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}
	
	return CMD_SUCCESS;
}

DEFUN(set_eag_portal_server_nasid_to_url_func,
	set_eag_portal_server_nasid_to_url_cmd,
	"set portal-server (wlanid|vlanid|wtpid|interface) KEY nasid-to-url (enable|disable)",
	"set parameter of portal policy, if not exist, create it\n"
	"portal policy\n"
	"portal server index type wlanid\n" 
	"portal server index type vlanid\n" 	
	"portal server index type wtpid\n"	
	"portal server index type l3interface name\n"
	"key word name, specify a item of portal policy\n"
	"add nasid infornation to url\n"
	"add nasid infornation to url enable\n"
	"add nasid infornation to url disable\n"
)
{
	int ret = -1;
	PORTAL_KEY_TYPE key_type;
	unsigned long keyid = 0;
	char *keystr = "";
	int nasid_to_url = 0;

	if (strncmp(argv[0], "wlanid", strlen(argv[0])) == 0) {
		key_type = PORTAL_KEYTYPE_WLANID;
		keyid = atoi(argv[1]);
		if (keyid == 0 || keyid > 128){
			vty_out(vty, "%% wlan id is out of range 1~128\n");
			return CMD_SUCCESS;
		}
	}
	else if (strncmp(argv[0], "vlanid", strlen(argv[0])) == 0) {
		key_type = PORTAL_KEYTYPE_VLANID;
		keyid = atoi(argv[1]);
		if(keyid == 0 || keyid > 4096) {
			vty_out(vty, "%% vlan id is out of range 1~4096\n");
			return CMD_SUCCESS;
		}		
	}
	else if (0 == strncmp(argv[0], "wtpid", strlen(argv[0]))){
		keyid = atoi(argv[1]);
		key_type = PORTAL_KEYTYPE_WTPID;
	}
	else if (0 == strncmp(argv[0], "interface", strlen(argv[0]))) {
		key_type = PORTAL_KEYTYPE_INTF;
		keystr = (char *)argv[1];
	}
	else {
		vty_out(vty, "%% unknown index type %s\n", argv[0]);
		return CMD_SUCCESS;
	}

	if (0 == strncmp(argv[2],"enable",strlen(argv[2])))
	{
		nasid_to_url = 1;
	}
	else if (0 == strncmp(argv[2],"disable",strlen(argv[2])))
	{
		nasid_to_url = 0;
	}
	else
	{
		vty_out(vty, "%% bad command parameter\n");
		return CMD_SUCCESS;
	}
	EAG_DCLI_INIT_HANSI_INFO

	ret = eag_set_portal_server_nasid_to_url( dcli_dbus_connection_curr,
										hansitype,insid,
										key_type,
										keyid,
										keystr,
										nasid_to_url);
	if(EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_ERR_UNKNOWN == ret) {
		vty_out(vty, "%% unknown error\n");
	}
	else if (EAG_ERR_PORTAL_MODIFY_SRV_NOT_EXIST == ret) {
		vty_out(vty, "%% the add key is not exist\n");
	}
	else if (EAG_ERR_PORTAL_MODIFY_SRV_ERR_TYPE == ret) {
		vty_out(vty, "%% error index type\n");
	}
	else if (EAG_RETURN_OK != ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}
	
	return CMD_SUCCESS;
}

DEFUN(set_eag_portal_server_wlanparameter_func,
	set_eag_portal_server_wlanparameter_cmd,
	"set portal-server (wlanid|vlanid|wtpid|interface) KEY wlanparameter (enable|disable) [DESKEY]",
	"set parameter of portal policy, if not exist, create it\n"
	"portal policy\n"
	"portal server index type wlanid\n" 
	"portal server index type vlanid\n" 	
	"portal server index type wtpid\n"	
	"portal server index type l3interface name\n"
	"key word name, specify a item of portal policy\n"
	"add wlanparameter infornation to url\n"
	"add wlanparameter infornation to url enable\n"
	"add wlanparameter infornation to url disable\n"
	"set wlanparameter des encrypt key\n"
)
{
	int ret = -1;
	PORTAL_KEY_TYPE key_type;
	unsigned long keyid = 0;
	char *keystr = "";
	int wlanparameter = 0;
	char *deskey=NULL;

	if (strncmp(argv[0], "wlanid", strlen(argv[0])) == 0) {
		key_type = PORTAL_KEYTYPE_WLANID;
		keyid = atoi(argv[1]);
		if (keyid == 0 || keyid > 128){
			vty_out(vty, "%% wlan id is out of range 1~128\n");
			return CMD_SUCCESS;
		}
	}
	else if (strncmp(argv[0], "vlanid", strlen(argv[0])) == 0) {
		key_type = PORTAL_KEYTYPE_VLANID;
		keyid = atoi(argv[1]);
		if(keyid == 0 || keyid > 4096) {
			vty_out(vty, "%% vlan id is out of range 1~4096\n");
			return CMD_SUCCESS;
		}		
	}
	else if (0 == strncmp(argv[0], "wtpid", strlen(argv[0]))){
		keyid = atoi(argv[1]);
		key_type = PORTAL_KEYTYPE_WTPID;
	}
	else if (0 == strncmp(argv[0], "interface", strlen(argv[0]))) {
		key_type = PORTAL_KEYTYPE_INTF;
		keystr = (char *)argv[1];
	}
	else {
		vty_out(vty, "%% unknown index type %s\n", argv[0]);
		return CMD_SUCCESS;
	}

	if (0 == strncmp(argv[2],"enable",strlen(argv[2])))
	{
		wlanparameter = 1;
		if (4!=argc) {
			vty_out(vty, "%% you should set DESkey\n");
			return CMD_SUCCESS;
		}
		deskey = (char *)argv[3];
		if ( strlen(deskey)>MAX_DES_KEY_LEN){
			vty_out(vty, "%% deskey length should not larger than %d\n",MAX_DES_KEY_LEN);
			return CMD_SUCCESS;
		}
	}
	else if (0 == strncmp(argv[2],"disable",strlen(argv[2])))
	{
		wlanparameter = 0;
		if (3!=argc) {
			vty_out(vty, "%% bad command parameter:should not set DESkey\n");
			return CMD_SUCCESS;
		}
	}
	else
	{
		vty_out(vty, "%% bad command parameter\n");
		return CMD_SUCCESS;
	}

	EAG_DCLI_INIT_HANSI_INFO

	ret = eag_set_portal_server_wlanparameter( dcli_dbus_connection_curr,
										hansitype,insid,
										key_type,
										keyid,
										keystr,
										wlanparameter,
										deskey);
	if(EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_ERR_UNKNOWN == ret) {
		vty_out(vty, "%% unknown error\n");
	}
	else if (EAG_ERR_PORTAL_MODIFY_SRV_NOT_EXIST == ret) {
		vty_out(vty, "%% the add key is not exist\n");
	}
	else if (EAG_ERR_PORTAL_MODIFY_SRV_ERR_TYPE == ret) {
		vty_out(vty, "%% error index type\n");
	}
	else if (EAG_ERR_PORTAL_WLANPARAMTER_DESKEY_LEN_LIMITE == ret) {
		vty_out(vty, "%% deskey length should not larger than %d\n",MAX_DES_KEY_LEN);
	}
	else if (EAG_ERR_PORTAL_WLANPARAMTER_NOT_SET_DESKEY == ret) {
		vty_out(vty, "%% you should set deskey when enable wlanparameter!\n");
	}
	else if (EAG_RETURN_OK != ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}
	
	return CMD_SUCCESS;
}


DEFUN(set_eag_portal_server_wlanuserfirsturl_func,
	set_eag_portal_server_wlanuserfirsturl_cmd,
	"set portal-server (wlanid|vlanid|wtpid|interface) KEY wlanuserfirsturl (enable|disable)",
	"set parameter of portal policy, if not exist, create it\n"
	"portal policy\n"
	"portal server index type wlanid\n" 
	"portal server index type vlanid\n" 	
	"portal server index type wtpid\n"	
	"portal server index type l3interface name\n"
	"key word name, specify a item of portal policy\n"
	"add wlanuserfirsturl infornation to url\n"
	"add wlanuserfirsturl infornation to url enable\n"
	"add wlanuserfirsturl infornation to url disable\n"
)
{
	int ret = -1;
	PORTAL_KEY_TYPE key_type;
	unsigned long keyid = 0;
	char *keystr = "";
	int wlanuserfirsturl = 0;

	if (strncmp(argv[0], "wlanid", strlen(argv[0])) == 0) {
		key_type = PORTAL_KEYTYPE_WLANID;
		keyid = atoi(argv[1]);
		if (keyid == 0 || keyid > 128){
			vty_out(vty, "%% wlan id is out of range 1~128\n");
			return CMD_SUCCESS;
		}
	}
	else if (strncmp(argv[0], "vlanid", strlen(argv[0])) == 0) {
		key_type = PORTAL_KEYTYPE_VLANID;
		keyid = atoi(argv[1]);
		if (keyid == 0 || keyid > 4096) {
			vty_out(vty, "%% vlan id is out of range 1~4096\n");
			return CMD_SUCCESS;
		}		
	}
	else if (0 == strncmp(argv[0], "wtpid", strlen(argv[0]))){
		keyid = atoi(argv[1]);
		key_type = PORTAL_KEYTYPE_WTPID;
	}
	else if (0 == strncmp(argv[0], "interface", strlen(argv[0]))) {
		key_type = PORTAL_KEYTYPE_INTF;
		keystr = (char *)argv[1];
	}
	else {
		vty_out(vty, "%% unknown index type %s\n", argv[0]);
		return CMD_SUCCESS;
	}

	if (0 == strncmp(argv[2],"enable",strlen(argv[2])))
	{
		wlanuserfirsturl = 1;
	}
	else if (0 == strncmp(argv[2],"disable",strlen(argv[2])))
	{
		wlanuserfirsturl = 0;
	}
	else
	{
		vty_out(vty, "%% bad command parameter\n");
		return CMD_SUCCESS;
	}
	EAG_DCLI_INIT_HANSI_INFO

	ret = eag_set_portal_server_wlanuserfirsturl( dcli_dbus_connection_curr,
										hansitype,insid,
										key_type,
										keyid,
										keystr,
										wlanuserfirsturl);
	if(EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_ERR_UNKNOWN == ret) {
		vty_out(vty, "%% unknown error\n");
	}
	else if (EAG_ERR_PORTAL_MODIFY_SRV_NOT_EXIST == ret) {
		vty_out(vty, "%% the add key is not exist\n");
	}
	else if (EAG_ERR_PORTAL_MODIFY_SRV_ERR_TYPE == ret) {
		vty_out(vty, "%% error index type\n");
	}
	else if (EAG_RETURN_OK != ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}
	
	return CMD_SUCCESS;
}


DEFUN(set_eag_portal_server_url_suffix_func,
	set_eag_portal_server_url_suffix_cmd,
	"set portal-server (wlanid|vlanid|wtpid|interface) KEY url-suffix [SUFFIX]",
	"set parameter of portal policy, if not exist, create it\n"
	"portal policy\n"
	"portal server index type wlanid\n"	
	"portal server index type vlanid\n"		
	"portal server index type wtpid\n"	
	"portal server index type l3interface name\n"
	"key word name, specify a item of portal policy\n"
	"URL SUFFIX\n"
	"portal url suffix, like 'region=GD', to be blank if not fill up\n"
)
{
	char *key_word = (char *)argv[0], *url_suffix = NULL;
	unsigned long keyid = 0;
	PORTAL_KEY_TYPE key_type;
	int ret = -1;
	if (3 == argc){
		url_suffix = (char *)argv[2];
		if (strlen(url_suffix) > MAX_PORTAL_URL_SUFFIX_LEN-1){
			vty_out(vty, "%% error url-suffix format\n");
			return CMD_WARNING;
		}
		if (NULL == strchr(url_suffix,'=')) {
			vty_out(vty,"%% error url-suffix format,please print like 'suffix_name=value'\n");
			return CMD_WARNING;
		}
	}
	else
		url_suffix = "";

	if (strncmp(argv[0], "wlanid", strlen(argv[0])) == 0) {
		key_type = PORTAL_KEYTYPE_WLANID;
		keyid = atoi(argv[1]);
		if (keyid == 0 || keyid > 128){
			vty_out(vty, "%% wlan id is out of range 1~128\n");
			return CMD_SUCCESS;
		}
	}
	else if (strncmp(argv[0], "vlanid", strlen(argv[0])) == 0) {
		key_type = PORTAL_KEYTYPE_VLANID;
		keyid = atoi(argv[1]);
		if(keyid == 0 || keyid > 4096) {
			vty_out(vty, "%% vlan id is out of range 1~4096\n");
			return CMD_SUCCESS;
		}		
	}
	else if (strncmp(argv[0], "wtpid", strlen(argv[0])) == 0){
		keyid = atoi(argv[1]);
		key_type = PORTAL_KEYTYPE_WTPID;
	}
	else if (strncmp(argv[0], "interface", strlen(argv[0])) == 0) {
		key_type = PORTAL_KEYTYPE_INTF;
		key_word = (char *)argv[1];
	}
	else {
		vty_out(vty, "%% unknown index type %s\n", argv[0]);
		return CMD_SUCCESS;
	}
	
	EAG_DCLI_INIT_HANSI_INFO

	ret = eag_set_portal_server_url_suffix( dcli_dbus_connection_curr,
										hansitype,insid,
										key_type,
										keyid,
										key_word,
										url_suffix);
	
	if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_ERR_INPUT_PARAM_ERR == ret) {
		vty_out(vty, "%% input prarm error\n");
	}
	else if (EAG_ERR_PORTAL_ADD_SRV_ERR_TYPE == ret) {
		vty_out(vty, "%% keyword type error\n");
	}
	else if (EAG_ERR_PORTAL_MODIFY_SRV_NOT_EXIST == ret) {
		vty_out(vty, "%% the add key is not exist\n");
	}
	else if (EAG_ERR_PORTAL_MODIFY_SRV_ERR_TYPE == ret) {
		vty_out(vty, "%% error index type\n");
	}
	else if (EAG_RETURN_OK != ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}
}


DEFUN(set_eag_portal_server_wlanapmac_func,
	set_eag_portal_server_wlanapmac_cmd,
	"set portal-server (wlanid|vlanid|wtpid|interface) KEY wlanapmac (enable|disable)",
	"set parameter of portal policy\n"
	"portal policy\n"
	"portal server index type wlanid\n" 
	"portal server index type vlanid\n" 	
	"portal server index type wtpid\n"	
	"portal server index type l3interface name\n"
	"key word name, specify a item of portal policy\n"
	"add wlanapmac infornation to url\n"
	"add wlanapmac infornation to url enable\n"
	"add wlanapmac infornation to url disable\n"
)
{
	int ret = -1;
	PORTAL_KEY_TYPE key_type;
	unsigned long keyid = 0;
	char *keystr = "";
	int wlanapmac = 0;

	if (strncmp(argv[0], "wlanid", strlen(argv[0])) == 0) {
		key_type = PORTAL_KEYTYPE_WLANID;
		keyid = atoi(argv[1]);
		if (keyid == 0 || keyid > 128){
			vty_out(vty, "%% wlan id is out of range 1~128\n");
			return CMD_SUCCESS;
		}
	}
	else if (strncmp(argv[0], "vlanid", strlen(argv[0])) == 0) {
		key_type = PORTAL_KEYTYPE_VLANID;
		keyid = atoi(argv[1]);
		if (keyid == 0 || keyid > 4096) {
			vty_out(vty, "%% vlan id is out of range 1~4096\n");
			return CMD_SUCCESS;
		}		
	}
	else if (0 == strncmp(argv[0], "wtpid", strlen(argv[0]))){
		keyid = atoi(argv[1]);
		key_type = PORTAL_KEYTYPE_WTPID;
	}
	else if (0 == strncmp(argv[0], "interface", strlen(argv[0]))) {
		key_type = PORTAL_KEYTYPE_INTF;
		keystr = (char *)argv[1];
	}
	else {
		vty_out(vty, "%% unknown index type %s\n", argv[0]);
		return CMD_SUCCESS;
	}

	if (0 == strncmp(argv[2],"enable",strlen(argv[2])))
	{
		wlanapmac = 1;
	}
	else if (0 == strncmp(argv[2],"disable",strlen(argv[2])))
	{
		wlanapmac = 0;
	}
	else
	{
		vty_out(vty, "%% bad command parameter\n");
		return CMD_SUCCESS;
	}
	EAG_DCLI_INIT_HANSI_INFO

	ret = eag_set_portal_server_wlanapmac( dcli_dbus_connection_curr,
										hansitype,insid,
										key_type,
										keyid,
										keystr,
										wlanapmac);
	if(EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_ERR_UNKNOWN == ret) {
		vty_out(vty, "%% unknown error\n");
	}
	else if (EAG_ERR_PORTAL_MODIFY_SRV_NOT_EXIST == ret) {
		vty_out(vty, "%% the add key is not exist\n");
	}
	else if (EAG_ERR_PORTAL_MODIFY_SRV_ERR_TYPE == ret) {
		vty_out(vty, "%% error index type\n");
	}
	else if (EAG_RETURN_OK != ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}
	
	return CMD_SUCCESS;
}

DEFUN(set_eag_portal_server_usermac_to_url_func,
	set_eag_portal_server_usermac_to_url_cmd,
	"set portal-server (wlanid|vlanid|wtpid|interface) KEY usermac-to-url (enable|disable)",
	"set parameter of portal policy, if not exist, create it\n"
	"portal policy\n"
	"portal server index type wlanid\n" 
	"portal server index type vlanid\n" 	
	"portal server index type wtpid\n"	
	"portal server index type l3interface name\n"
	"key word name, specify a item of portal policy\n"
	"add usermac infornation to url\n"
	"add usermac infornation to url enable\n"
	"add usermac infornation to url disable\n"
)
{
	int ret = -1;
	PORTAL_KEY_TYPE key_type;
	unsigned long keyid = 0;
	char *keystr = "";
	int usermac_to_url = 0;

	if (strncmp(argv[0], "wlanid", strlen(argv[0])) == 0) {
		key_type = PORTAL_KEYTYPE_WLANID;
		keyid = atoi(argv[1]);
		if (keyid == 0 || keyid > 128){
			vty_out(vty, "%% wlan id is out of range 1~128\n");
			return CMD_SUCCESS;
		}
	}
	else if (strncmp(argv[0], "vlanid", strlen(argv[0])) == 0) {
		key_type = PORTAL_KEYTYPE_VLANID;
		keyid = atoi(argv[1]);
		if(keyid == 0 || keyid > 4096) {
			vty_out(vty, "%% vlan id is out of range 1~4096\n");
			return CMD_SUCCESS;
		}		
	}
	else if (0 == strncmp(argv[0], "wtpid", strlen(argv[0]))){
		keyid = atoi(argv[1]);
		key_type = PORTAL_KEYTYPE_WTPID;
	}
	else if (0 == strncmp(argv[0], "interface", strlen(argv[0]))) {
		key_type = PORTAL_KEYTYPE_INTF;
		keystr = (char *)argv[1];
	}
	else {
		vty_out(vty, "%% unknown index type %s\n", argv[0]);
		return CMD_SUCCESS;
	}

	if (0 == strncmp(argv[2], "enable", strlen(argv[2])))
	{
		usermac_to_url = 1;
	}
	else if (0 == strncmp(argv[2], "disable", strlen(argv[2])))
	{
		usermac_to_url = 0;
	}
	else
	{
		vty_out(vty, "%% bad command parameter\n");
		return CMD_SUCCESS;
	}
	EAG_DCLI_INIT_HANSI_INFO

	ret = eag_set_portal_server_usermac_to_url( dcli_dbus_connection_curr,
										hansitype,insid,
										key_type,
										keyid,
										keystr,
										usermac_to_url);
	if(EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_ERR_UNKNOWN == ret) {
		vty_out(vty, "%% unknown error\n");
	}
	else if (EAG_ERR_PORTAL_MODIFY_SRV_NOT_EXIST == ret) {
		vty_out(vty, "%% the add key is not exist\n");
	}
	else if (EAG_ERR_PORTAL_MODIFY_SRV_ERR_TYPE == ret) {
		vty_out(vty, "%% error index type\n");
	}
	else if (EAG_RETURN_OK != ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}
	
	return CMD_SUCCESS;
}

DEFUN(set_eag_portal_server_clientmac_to_url_func,
	set_eag_portal_server_clientmac_to_url_cmd,
	"set portal-server (wlanid|vlanid|wtpid|interface) KEY clientmac-to-url (enable|disable)",
	"set parameter of portal policy, if not exist, create it\n"
	"portal policy\n"
	"portal server index type wlanid\n" 
	"portal server index type vlanid\n" 	
	"portal server index type wtpid\n"	
	"portal server index type l3interface name\n"
	"key word name, specify a item of portal policy\n"
	"add clientmac infornation to url\n"
	"add clientmac infornation to url enable\n"
	"add clientmac infornation to url disable\n"
)
{
	int ret = -1;
	PORTAL_KEY_TYPE key_type;
	unsigned long keyid = 0;
	char *keystr = "";
	int clientmac_to_url = 0;

	if (strncmp(argv[0], "wlanid", strlen(argv[0])) == 0) {
		key_type = PORTAL_KEYTYPE_WLANID;
		keyid = atoi(argv[1]);
		if (keyid == 0 || keyid > 128){
			vty_out(vty, "%% wlan id is out of range 1~128\n");
			return CMD_SUCCESS;
		}
	}
	else if (strncmp(argv[0], "vlanid", strlen(argv[0])) == 0) {
		key_type = PORTAL_KEYTYPE_VLANID;
		keyid = atoi(argv[1]);
		if(keyid == 0 || keyid > 4096) {
			vty_out(vty, "%% vlan id is out of range 1~4096\n");
			return CMD_SUCCESS;
		}		
	}
	else if (0 == strncmp(argv[0], "wtpid", strlen(argv[0]))){
		keyid = atoi(argv[1]);
		key_type = PORTAL_KEYTYPE_WTPID;
	}
	else if (0 == strncmp(argv[0], "interface", strlen(argv[0]))) {
		key_type = PORTAL_KEYTYPE_INTF;
		keystr = (char *)argv[1];
	}
	else {
		vty_out(vty, "%% unknown index type %s\n", argv[0]);
		return CMD_SUCCESS;
	}

	if (0 == strncmp(argv[2], "enable", strlen(argv[2])))
	{
		clientmac_to_url = 1;
	}
	else if (0 == strncmp(argv[2], "disable", strlen(argv[2])))
	{
		clientmac_to_url = 0;
	}
	else
	{
		vty_out(vty, "%% bad command parameter\n");
		return CMD_SUCCESS;
	}
	EAG_DCLI_INIT_HANSI_INFO

	ret = eag_set_portal_server_clientmac_to_url( dcli_dbus_connection_curr,
										hansitype,insid,
										key_type,
										keyid,
										keystr,
										clientmac_to_url);
	if(EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_ERR_UNKNOWN == ret) {
		vty_out(vty, "%% unknown error\n");
	}
	else if (EAG_ERR_PORTAL_MODIFY_SRV_NOT_EXIST == ret) {
		vty_out(vty, "%% the add key is not exist\n");
	}
	else if (EAG_ERR_PORTAL_MODIFY_SRV_ERR_TYPE == ret) {
		vty_out(vty, "%% error index type\n");
	}
	else if (EAG_RETURN_OK != ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}
	
	return CMD_SUCCESS;
}

DEFUN(set_eag_portal_server_apmac_to_url_func,
	set_eag_portal_server_apmac_to_url_cmd,
	"set portal-server (wlanid|vlanid|wtpid|interface) KEY apmac-to-url (enable|disable)",
	"set parameter of portal policy, if not exist, create it\n"
	"portal policy\n"
	"portal server index type wlanid\n" 
	"portal server index type vlanid\n" 	
	"portal server index type wtpid\n"	
	"portal server index type l3interface name\n"
	"key word name, specify a item of portal policy\n"
	"add apmac infornation to url\n"
	"add apmac infornation to url enable\n"
	"add apmac infornation to url disable\n"
)
{
	int ret = -1;
	PORTAL_KEY_TYPE key_type;
	unsigned long keyid = 0;
	char *keystr = "";
	int apmac_to_url = 0;

	if (strncmp(argv[0], "wlanid", strlen(argv[0])) == 0) {
		key_type = PORTAL_KEYTYPE_WLANID;
		keyid = atoi(argv[1]);
		if (keyid == 0 || keyid > 128){
			vty_out(vty, "%% wlan id is out of range 1~128\n");
			return CMD_SUCCESS;
		}
	}
	else if (strncmp(argv[0], "vlanid", strlen(argv[0])) == 0) {
		key_type = PORTAL_KEYTYPE_VLANID;
		keyid = atoi(argv[1]);
		if(keyid == 0 || keyid > 4096) {
			vty_out(vty, "%% vlan id is out of range 1~4096\n");
			return CMD_SUCCESS;
		}		
	}
	else if (0 == strncmp(argv[0], "wtpid", strlen(argv[0]))){
		keyid = atoi(argv[1]);
		key_type = PORTAL_KEYTYPE_WTPID;
	}
	else if (0 == strncmp(argv[0], "interface", strlen(argv[0]))) {
		key_type = PORTAL_KEYTYPE_INTF;
		keystr = (char *)argv[1];
	}
	else {
		vty_out(vty, "%% unknown index type %s\n", argv[0]);
		return CMD_SUCCESS;
	}

	if (0 == strncmp(argv[2], "enable", strlen(argv[2])))
	{
		apmac_to_url = 1;
	}
	else if (0 == strncmp(argv[2], "disable", strlen(argv[2])))
	{
		apmac_to_url = 0;
	}
	else
	{
		vty_out(vty, "%% bad command parameter\n");
		return CMD_SUCCESS;
	}
	EAG_DCLI_INIT_HANSI_INFO

	ret = eag_set_portal_server_apmac_to_url( dcli_dbus_connection_curr,
										hansitype,insid,
										key_type,
										keyid,
										keystr,
										apmac_to_url);
	if(EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_ERR_UNKNOWN == ret) {
		vty_out(vty, "%% unknown error\n");
	}
	else if (EAG_ERR_PORTAL_MODIFY_SRV_NOT_EXIST == ret) {
		vty_out(vty, "%% the add key is not exist\n");
	}
	else if (EAG_ERR_PORTAL_MODIFY_SRV_ERR_TYPE == ret) {
		vty_out(vty, "%% error index type\n");
	}
	else if (EAG_RETURN_OK != ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}
	
	return CMD_SUCCESS;
}

DEFUN(set_eag_portal_server_wlan_to_url_func,
	set_eag_portal_server_wlan_to_url_cmd,
	"set portal-server (wlanid|vlanid|wtpid|interface) KEY wlan-to-url (enable|disable)",
	"set parameter of portal policy, if not exist, create it\n"
	"portal policy\n"
	"portal server index type wlanid\n" 
	"portal server index type vlanid\n" 	
	"portal server index type wtpid\n"	
	"portal server index type l3interface name\n"
	"key word name, specify a item of portal policy\n"
	"add wlan infornation to url\n"
	"add wlan infornation to url enable\n"
	"add wlan infornation to url disable\n"
)
{
	int ret = -1;
	PORTAL_KEY_TYPE key_type;
	unsigned long keyid = 0;
	char *keystr = "";
	int wlan_to_url = 0;

	if (strncmp(argv[0], "wlanid", strlen(argv[0])) == 0) {
		key_type = PORTAL_KEYTYPE_WLANID;
		keyid = atoi(argv[1]);
		if (keyid == 0 || keyid > 128){
			vty_out(vty, "%% wlan id is out of range 1~128\n");
			return CMD_SUCCESS;
		}
	}
	else if (strncmp(argv[0], "vlanid", strlen(argv[0])) == 0) {
		key_type = PORTAL_KEYTYPE_VLANID;
		keyid = atoi(argv[1]);
		if(keyid == 0 || keyid > 4096) {
			vty_out(vty, "%% vlan id is out of range 1~4096\n");
			return CMD_SUCCESS;
		}		
	}
	else if (0 == strncmp(argv[0], "wtpid", strlen(argv[0]))){
		keyid = atoi(argv[1]);
		key_type = PORTAL_KEYTYPE_WTPID;
	}
	else if (0 == strncmp(argv[0], "interface", strlen(argv[0]))) {
		key_type = PORTAL_KEYTYPE_INTF;
		keystr = (char *)argv[1];
	}
	else {
		vty_out(vty, "%% unknown index type %s\n", argv[0]);
		return CMD_SUCCESS;
	}

	if (0 == strncmp(argv[2], "enable", strlen(argv[2])))
	{
		wlan_to_url = 1;
	}
	else if (0 == strncmp(argv[2], "disable", strlen(argv[2])))
	{
		wlan_to_url = 0;
	}
	else
	{
		vty_out(vty, "%% bad command parameter\n");
		return CMD_SUCCESS;
	}
	EAG_DCLI_INIT_HANSI_INFO

	ret = eag_set_portal_server_wlan_to_url( dcli_dbus_connection_curr,
										hansitype,insid,
										key_type,
										keyid,
										keystr,
										wlan_to_url);
	if(EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_ERR_UNKNOWN == ret) {
		vty_out(vty, "%% unknown error\n");
	}
	else if (EAG_ERR_PORTAL_MODIFY_SRV_NOT_EXIST == ret) {
		vty_out(vty, "%% the add key is not exist\n");
	}
	else if (EAG_ERR_PORTAL_MODIFY_SRV_ERR_TYPE == ret) {
		vty_out(vty, "%% error index type\n");
	}
	else if (EAG_RETURN_OK != ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}
	
	return CMD_SUCCESS;
}

DEFUN(set_eag_portal_server_redirect_to_url_func,
	set_eag_portal_server_redirect_to_url_cmd,
	"set portal-server (wlanid|vlanid|wtpid|interface) KEY redirect-to-url (enable|disable)",
	"set parameter of portal policy, if not exist, create it\n"
	"portal policy\n"
	"portal server index type wlanid\n" 
	"portal server index type vlanid\n" 	
	"portal server index type wtpid\n"	
	"portal server index type l3interface name\n"
	"key word name, specify a item of portal policy\n"
	"add redirect infornation to url\n"
	"add redirect infornation to url enable\n"
	"add redirect infornation to url disable\n"
)
{
	int ret = -1;
	PORTAL_KEY_TYPE key_type;
	unsigned long keyid = 0;
	char *keystr = "";
	int redirect_to_url = 0;

	if (strncmp(argv[0], "wlanid", strlen(argv[0])) == 0) {
		key_type = PORTAL_KEYTYPE_WLANID;
		keyid = atoi(argv[1]);
		if (keyid == 0 || keyid > 128){
			vty_out(vty, "%% wlan id is out of range 1~128\n");
			return CMD_SUCCESS;
		}
	}
	else if (strncmp(argv[0], "vlanid", strlen(argv[0])) == 0) {
		key_type = PORTAL_KEYTYPE_VLANID;
		keyid = atoi(argv[1]);
		if(keyid == 0 || keyid > 4096) {
			vty_out(vty, "%% vlan id is out of range 1~4096\n");
			return CMD_SUCCESS;
		}		
	}
	else if (0 == strncmp(argv[0], "wtpid", strlen(argv[0]))){
		keyid = atoi(argv[1]);
		key_type = PORTAL_KEYTYPE_WTPID;
	}
	else if (0 == strncmp(argv[0], "interface", strlen(argv[0]))) {
		key_type = PORTAL_KEYTYPE_INTF;
		keystr = (char *)argv[1];
	}
	else {
		vty_out(vty, "%% unknown index type %s\n", argv[0]);
		return CMD_SUCCESS;
	}

	if (0 == strncmp(argv[2], "enable", strlen(argv[2])))
	{
		redirect_to_url = 1;
	}
	else if (0 == strncmp(argv[2], "disable", strlen(argv[2])))
	{
		redirect_to_url = 0;
	}
	else
	{
		vty_out(vty, "%% bad command parameter\n");
		return CMD_SUCCESS;
	}
	EAG_DCLI_INIT_HANSI_INFO

	ret = eag_set_portal_server_redirect_to_url( dcli_dbus_connection_curr,
										hansitype,insid,
										key_type,
										keyid,
										keystr,
										redirect_to_url);
	if(EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_ERR_UNKNOWN == ret) {
		vty_out(vty, "%% unknown error\n");
	}
	else if (EAG_ERR_PORTAL_MODIFY_SRV_NOT_EXIST == ret) {
		vty_out(vty, "%% the add key is not exist\n");
	}
	else if (EAG_ERR_PORTAL_MODIFY_SRV_ERR_TYPE == ret) {
		vty_out(vty, "%% error index type\n");
	}
	else if (EAG_RETURN_OK != ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}
	
	return CMD_SUCCESS;
}

DEFUN(set_eag_portal_server_wlanusermac_func,
	set_eag_portal_server_wlanusermac_cmd,
	"set portal-server (wlanid|vlanid|wtpid|interface) KEY wlanusermac (enable|disable) [DESKEY]",
	"set parameter of portal policy\n"
	"portal policy\n"
	"portal server index type wlanid\n" 
	"portal server index type vlanid\n" 	
	"portal server index type wtpid\n"	
	"portal server index type l3interface name\n"
	"key word name, specify a item of portal policy\n"
	"add wlanusermac infornation to url\n"
	"add wlanusermac infornation to url enable\n"
	"add wlanusermac infornation to url disable\n"
	"set wlanparameter des encrypt key\n"
)
{
	int ret = -1;
	PORTAL_KEY_TYPE key_type;
	unsigned long keyid = 0;
	char *keystr = "";
	int wlanusermac = 0;
	char *deskey=NULL;

	if (strncmp(argv[0], "wlanid", strlen(argv[0])) == 0) {
		key_type = PORTAL_KEYTYPE_WLANID;
		keyid = atoi(argv[1]);
		if (keyid == 0 || keyid > 128){
			vty_out(vty, "%% wlan id is out of range 1~128\n");
			return CMD_SUCCESS;
		}
	}
	else if (strncmp(argv[0], "vlanid", strlen(argv[0])) == 0) {
		key_type = PORTAL_KEYTYPE_VLANID;
		keyid = atoi(argv[1]);
		if (keyid == 0 || keyid > 4096) {
			vty_out(vty, "%% vlan id is out of range 1~4096\n");
			return CMD_SUCCESS;
		}		
	}
	else if (0 == strncmp(argv[0], "wtpid", strlen(argv[0]))){
		keyid = atoi(argv[1]);
		key_type = PORTAL_KEYTYPE_WTPID;
	}
	else if (0 == strncmp(argv[0], "interface", strlen(argv[0]))) {
		key_type = PORTAL_KEYTYPE_INTF;
		keystr = (char *)argv[1];
	}
	else {
		vty_out(vty, "%% unknown index type %s\n", argv[0]);
		return CMD_SUCCESS;
	}

	if (0 == strncmp(argv[2],"enable",strlen(argv[2])))
	{
		wlanusermac = 1;
		if (4!=argc) {
			vty_out(vty, "%% you should set DESkey\n");
			return CMD_SUCCESS;
		}
		deskey = (char *)argv[3];
		if ( strlen(deskey)>MAX_DES_KEY_LEN){
			vty_out(vty, "%% deskey length should not larger than %d\n",MAX_DES_KEY_LEN);
			return CMD_SUCCESS;
		}
	}
	else if (0 == strncmp(argv[2],"disable",strlen(argv[2])))
	{
		wlanusermac = 0;
		if (3!=argc) {
			vty_out(vty, "%% bad command parameter:should not set DESkey\n");
			return CMD_SUCCESS;
		}
	}
	else
	{
		vty_out(vty, "%% bad command parameter\n");
		return CMD_SUCCESS;
	}
	EAG_DCLI_INIT_HANSI_INFO

	ret = eag_set_portal_server_wlanusermac( dcli_dbus_connection_curr,
										hansitype,insid,
										key_type,
										keyid,
										keystr,
										wlanusermac,
										deskey);
	if(EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_ERR_UNKNOWN == ret) {
		vty_out(vty, "%% unknown error\n");
	}
	else if (EAG_ERR_PORTAL_MODIFY_SRV_NOT_EXIST == ret) {
		vty_out(vty, "%% the add key is not exist\n");
	}
	else if (EAG_ERR_PORTAL_MODIFY_SRV_ERR_TYPE == ret) {
		vty_out(vty, "%% error index type\n");
	}
	else if (EAG_RETURN_OK != ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}
	
	return CMD_SUCCESS;
}

DEFUN(set_eag_portal_server_secret_func,
	set_eag_portal_server_secret_cmd,
	"set portal-server (wlanid|vlanid|wtpid|interface) KEY secret [SECRET]",
	SETT_STR
	"portal server\n"
	"portal server index type wlanid\n"	
	"portal server index type vlanid\n"		
	"portal server index type wtpid\n"	
	"portal server index type l3interface name\n"
	"key word name, specify a item of portal server\n"
	"shared secret\n"
	"shared secret\n"
)
{
	const char *key_word = argv[0];
	const char *secret = "";
	unsigned long keyid = 0;
	PORTAL_KEY_TYPE key_type;
	int ret = -1;
	if (3 == argc) {
		secret = argv[2];
		if (strlen(secret) > PORTAL_SECRETSIZE-1) {
			vty_out(vty, "error secret format\n");
			return CMD_WARNING;
		}
	}
	else
		secret = "";

	if (strncmp(argv[0], "wlanid", strlen(argv[0])) == 0) {
		key_type = PORTAL_KEYTYPE_WLANID;
		keyid = atoi(argv[1]);
		if (keyid == 0 || keyid > 128){
			vty_out(vty, "%% wlan id is out of range 1~128\n");
			return CMD_SUCCESS;
		}
	}
	else if (strncmp(argv[0], "vlanid", strlen(argv[0])) == 0) {
		key_type = PORTAL_KEYTYPE_VLANID;
		keyid = atoi(argv[1]);
		if(keyid == 0 || keyid > 4096) {
			vty_out(vty, "%% vlan id is out of range 1~4096\n");
			return CMD_SUCCESS;
		}		
	}
	else if (strncmp(argv[0], "wtpid", strlen(argv[0])) == 0){
		keyid = atoi(argv[1]);
		key_type = PORTAL_KEYTYPE_WTPID;
	}
	else if (strncmp(argv[0], "interface", strlen(argv[0])) == 0) {
		key_type = PORTAL_KEYTYPE_INTF;
		key_word = argv[1];
	}
	else {
		vty_out(vty, "%% unknown index type %s\n", argv[0]);
		return CMD_SUCCESS;
	}
	
	EAG_DCLI_INIT_HANSI_INFO
#if 0
	if(eag_ins_running_state(dcli_dbus_connection_curr, hansitype, insid)){
		vty_out(vty, "eag instance %d is running, please stop it first\n",insid);
		return CMD_FAILURE;
	}
#endif
	ret = eag_set_portal_server_secret( dcli_dbus_connection_curr,
										hansitype,insid,
										key_type,
										keyid,
										key_word,
										secret);
	
	if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_ERR_INPUT_PARAM_ERR == ret) {
		vty_out(vty, "%% input prarm error\n");
	}
	else if (EAG_ERR_PORTAL_ADD_SRV_ERR_TYPE == ret) {
		vty_out(vty, "%% keyword type error\n");
	}
	else if (EAG_ERR_PORTAL_MODIFY_SRV_NOT_EXIST == ret) {
		vty_out(vty, "%% the add key is not exist\n");
	}
	else if (EAG_ERR_PORTAL_MODIFY_SRV_ERR_TYPE == ret) {
		vty_out(vty, "%% error index type\n");
	}
	else if (EAG_RETURN_OK != ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}
}

DEFUN(set_eag_portal_server_wisprlogin_func,
	set_eag_portal_server_wisprlogin_cmd,
	"set portal-server (wlanid|vlanid|wtpid|interface) KEY wisprlogin (enable|disable) [TYPE]",
	"set parameter of portal policy\n"
	"portal policy\n"
	"portal server index type wlanid\n" 
	"portal server index type vlanid\n" 	
	"portal server index type wtpid\n"	
	"portal server index type l3interface name\n"
	"key word name, specify a item of portal policy\n"
	"add wisprlogin url infornation to url\n"
	"add wisprlogin url infornation to url enable\n"
	"add wisprlogin url infornation to url disable\n"
	"set wisprlogin url for http|https\n"
)
{
	int ret = -1;
	PORTAL_KEY_TYPE key_type;
	unsigned long keyid = 0;
	char *keystr = "";
	int status = 0;
	char *type=NULL;

	if (strncmp(argv[0], "wlanid", strlen(argv[0])) == 0) {
		key_type = PORTAL_KEYTYPE_WLANID;
		keyid = atoi(argv[1]);
		if (keyid == 0 || keyid > 128){
			vty_out(vty, "%% wlan id is out of range 1~128\n");
			return CMD_SUCCESS;
		}
	}
	else if (strncmp(argv[0], "vlanid", strlen(argv[0])) == 0) {
		key_type = PORTAL_KEYTYPE_VLANID;
		keyid = atoi(argv[1]);
		if (keyid == 0 || keyid > 4096) {
			vty_out(vty, "%% vlan id is out of range 1~4096\n");
			return CMD_SUCCESS;
		}		
	}
	else if (0 == strncmp(argv[0], "wtpid", strlen(argv[0]))){
		keyid = atoi(argv[1]);
		key_type = PORTAL_KEYTYPE_WTPID;
	}
	else if (0 == strncmp(argv[0], "interface", strlen(argv[0]))) {
		key_type = PORTAL_KEYTYPE_INTF;
		keystr = (char *)argv[1];
	}
	else {
		vty_out(vty, "%% unknown index type %s\n", argv[0]);
		return CMD_SUCCESS;
	}

	if (0 == strncmp(argv[2],"enable",strlen(argv[2])))
	{
		status = 1;
		if (4!=argc) {
			vty_out(vty, "%% you should set type for http or https\n");
			return CMD_SUCCESS;
		}
		type = (char *)argv[3];
	}
	else if (0 == strncmp(argv[2],"disable",strlen(argv[2])))
	{
		status = 0;
		if (3!=argc) {
			vty_out(vty, "%% bad command parameter:should not set type\n");
			return CMD_SUCCESS;
		}
	}
	else
	{
		vty_out(vty, "%% bad command parameter\n");
		return CMD_SUCCESS;
	}
	EAG_DCLI_INIT_HANSI_INFO

	ret = eag_set_portal_server_wisprlogin( dcli_dbus_connection_curr,
										hansitype,insid,
										key_type,
										keyid,
										keystr,
										status,
										type );
	if(EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_ERR_UNKNOWN == ret) {
		vty_out(vty, "%% unknown error\n");
	}
	else if (EAG_ERR_PORTAL_MODIFY_SRV_NOT_EXIST == ret) {
		vty_out(vty, "%% the add key is not exist\n");
	}
	else if (EAG_ERR_PORTAL_MODIFY_SRV_ERR_TYPE == ret) {
		vty_out(vty, "%% error index type\n");
	}
	else if (EAG_ERR_PORTAL_SET_WISPR_URL_TYPE_ERR == ret) {
		vty_out(vty,"%% error url type error,should be http or https\n");
	}
	else if (EAG_RETURN_OK != ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}
	
	return CMD_SUCCESS;
}

DEFUN(set_eag_portal_server_mobile_urlparam_func,
	set_eag_portal_server_mobile_urlparam_cmd,
	"set portal-server (wlanid|vlanid|wtpid|interface) KEY mobile-urlparam (enable|disable)",
	"set parameter of portal policy, if not exist, create it\n"
	"portal policy\n"
	"portal server index type wlanid\n" 
	"portal server index type vlanid\n" 	
	"portal server index type wtpid\n"	
	"portal server index type l3interface name\n"
	"key word name, specify a item of portal policy\n"
	"set mobile-urlparam such as:wlanuserip wlanacname ssid\n"
	"set mobile-urlparam to url enable\n"
	"set mobile-urlparam to url disable\n"
)
{
	int ret = -1;
	PORTAL_KEY_TYPE key_type;
	unsigned long keyid = 0;
	char *keystr = "";
	int mobile_urlparam = 0;

	if (strncmp(argv[0], "wlanid", strlen(argv[0])) == 0) {
		key_type = PORTAL_KEYTYPE_WLANID;
		keyid = atoi(argv[1]);
		if (keyid == 0 || keyid > 128){
			vty_out(vty, "%% wlan id is out of range 1~128\n");
			return CMD_SUCCESS;
		}
	}
	else if (strncmp(argv[0], "vlanid", strlen(argv[0])) == 0) {
		key_type = PORTAL_KEYTYPE_VLANID;
		keyid = atoi(argv[1]);
		if(keyid == 0 || keyid > 4096) {
			vty_out(vty, "%% vlan id is out of range 1~4096\n");
			return CMD_SUCCESS;
		}		
	}
	else if (0 == strncmp(argv[0], "wtpid", strlen(argv[0]))){
		keyid = atoi(argv[1]);
		key_type = PORTAL_KEYTYPE_WTPID;
	}
	else if (0 == strncmp(argv[0], "interface", strlen(argv[0]))) {
		key_type = PORTAL_KEYTYPE_INTF;
		keystr = (char *)argv[1];
	}
	else {
		vty_out(vty, "%% unknown index type %s\n", argv[0]);
		return CMD_SUCCESS;
	}

	if (0 == strncmp(argv[2], "enable", strlen(argv[2])))
	{
		mobile_urlparam = 0;/*default 0 is enable*/
	}
	else if (0 == strncmp(argv[2], "disable", strlen(argv[2])))
	{
		mobile_urlparam = 1;
	}
	else
	{
		vty_out(vty, "%% bad command parameter\n");
		return CMD_SUCCESS;
	}
	EAG_DCLI_INIT_HANSI_INFO

	ret = eag_set_portal_server_mobile_urlparam( dcli_dbus_connection_curr,
										hansitype,insid,
										key_type,
										keyid,
										keystr,
										mobile_urlparam );
	if(EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_ERR_UNKNOWN == ret) {
		vty_out(vty, "%% unknown error\n");
	}
	else if (EAG_ERR_PORTAL_MODIFY_SRV_NOT_EXIST == ret) {
		vty_out(vty, "%% the add key is not exist\n");
	}
	else if (EAG_ERR_PORTAL_MODIFY_SRV_ERR_TYPE == ret) {
		vty_out(vty, "%% error index type\n");
	}
	else if (EAG_RETURN_OK != ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}
	
	return CMD_SUCCESS;
}

DEFUN(set_eag_portal_server_urlparam_func,
	set_eag_portal_server_urlparam_cmd,
	"set portal-server (wlanid|vlanid|wtpid|interface) KEY (urlparam-add|urlparam-del) PARAMSTR",
	"set parameter of portal policy, if not exist, create it\n"
	"portal policy\n"
	"portal server index type wlanid\n" 
	"portal server index type vlanid\n" 	
	"portal server index type wtpid\n"	
	"portal server index type l3interface name\n"
	"key word name, specify a item of portal policy\n"
	"add portal server url param\n"
	"del portal server url param\n"
	"1.urlparam-add: "\
	"PARAM=${NEWNAME;deskey=...(len:0-8);format=-(or default :);letter=lower(or default upper);"\
	"type=https(or default http);encode=off(or default on);value=...(len:0-127)};PARAM=${NEWNAME;...;PARAM=${NEWNEME;...}}     "\
	"PS: You can choose PARAM from [nasip,userip,usermac,apmac,apname,essid,nasid,acname,firsturl,wisprurl], "\
	"wisrpurl can take PARAM except wisprurl.   and deskey(for mac encrypt),format(for mac delimiter),letter(for mac upper or lower), "\
	"so usermac and apmac need.   and type(for url type),encode(for url coding), so firsturl and wisprurl need.   "\
	"and value for acname(also for other param you assign value).     "\
	"For example: nasip=${acip};userip=${userip};"\
	"usermac=${clientmac;deskey=12345678;format=-;letter=lower};apmac=${apmac;letter=lower};apname=${apname};essid=${wlan};"\
	"nasid=${nasid};acname=${acname;value=Autelan};firsturl=${redirect;encode=on};wisprurl=${wisprlogin;type=https;encode=on;"\
	"nasip=${...};userip=${...};usermac=${...};...}          "\
	"2.urlparam-del: PARAM1;PARAM2;PARAM3{PARAM1;PARAM2...};PARAM4.   "\
	"For example: nasip;userip;usermac;apmac;apname;essid;nasid;acname;firsturl;wisprurl{nasip;userip;usermac;...}.   "\
	"or nasip;wisprurl.   you can delete one or more.\n"
)
{
	PORTAL_KEY_TYPE key_type;
	unsigned long keyid = 0;
	char *key_word = "";
	int add_or_del = 0;
	char *url_param = NULL;
	int ret = -1;

	if (strncmp(argv[2], "urlparam-add", strlen(argv[2])) == 0) {
		add_or_del = 1;
	} else if (strncmp(argv[2], "urlparam-del", strlen(argv[2])) == 0) {
		add_or_del = 0;
	}

	if (strncmp(argv[0], "wlanid", strlen(argv[0])) == 0) {
		key_type = PORTAL_KEYTYPE_WLANID;
		keyid = atoi(argv[1]);
		if (keyid == 0 || keyid > 128){
			vty_out(vty, "%% wlan id is out of range 1~128\n");
			return CMD_SUCCESS;
		}
	}
	else if (strncmp(argv[0], "vlanid", strlen(argv[0])) == 0) {
		key_type = PORTAL_KEYTYPE_VLANID;
		keyid = atoi(argv[1]);
		if(keyid == 0 || keyid > 4096) {
			vty_out(vty, "%% vlan id is out of range 1~4096\n");
			return CMD_SUCCESS;
		}		
	}
	else if (strncmp(argv[0], "wtpid", strlen(argv[0])) == 0){
		keyid = atoi(argv[1]);
		key_type = PORTAL_KEYTYPE_WTPID;
	}
	else if (strncmp(argv[0], "interface", strlen(argv[0])) == 0) {
		key_type = PORTAL_KEYTYPE_INTF;
		key_word = (char *)argv[1];
	}
	else {
		vty_out(vty, "%% unknown index type %s\n", argv[0]);
		return CMD_SUCCESS;
	}
	
	url_param = (char *)argv[3];
	if (strlen(url_param) > URL_PARAM_QUERY_STR_LEN-1){
		vty_out(vty, "%% url-param length out of limite:%d\n", URL_PARAM_QUERY_STR_LEN-1);
		return CMD_WARNING;
	}
	
	EAG_DCLI_INIT_HANSI_INFO

	ret = eag_set_portal_server_urlparam( dcli_dbus_connection_curr,
										hansitype,insid,
										key_type,
										keyid,
										key_word,
										add_or_del,
										url_param);
	
	if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_ERR_INPUT_PARAM_ERR == ret) {
		vty_out(vty, "%% input prarm error\n");
	}
	else if (EAG_ERR_PORTAL_ADD_SRV_ERR_TYPE == ret) {
		vty_out(vty, "%% keyword type error\n");
	}
	else if (EAG_ERR_PORTAL_MODIFY_SRV_NOT_EXIST == ret) {
		vty_out(vty, "%% the add key is not exist\n");
	}
	else if (EAG_ERR_PORTAL_MODIFY_SRV_ERR_TYPE == ret) {
		vty_out(vty, "%% error index type\n");
	}
	else if (EAG_ERR_PORTAL_SET_URLPARAM_INPUT_ERR == ret) {
		vty_out(vty, "%% url param str input error\n");
	}
	else if (EAG_ERR_PORTAL_SET_URLPARAM_LEN_LINITE == ret) {
		vty_out(vty, "%% url param str input length out of limite\n");
	}
	else if (EAG_RETURN_OK != ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}
}

static int eag_urlparam_show_v2(struct vty* vty, struct url_param_t *param)
{
	if (vty == NULL || param == NULL) {
		return -1;
	}
	switch (param->param_type) {
    case URL_PARAM_NASIP:
        vty_out(vty, "[nasip]:\n   name:%s\n", param->param_name);
        if (strlen(param->param_value) > 0) {
			vty_out(vty, "   value:%s\n", param->param_value);
        }
        break;
    case URL_PARAM_USERIP:
        vty_out(vty, "[userip]:\n   name:%s\n", param->param_name);
		if (strlen(param->param_value) > 0) {
			vty_out(vty, "   value:%s\n", param->param_value);
        }
        break;
    case URL_PARAM_USERMAC:
        vty_out(vty, "[usermac]:\n   name:%s\n   deskey:%s\n   format:%s\n   letter:%s\n", 
                param->param_name, param->mac_deskey, 
                strlen(param->mac_format)>0?(param->mac_format):":", 
				(UP_LETTER_LOWER == param->letter_type)?"lower":"upper");
		if (strlen(param->param_value) > 0) {
			vty_out(vty, "   value:%s\n", param->param_value);
        }
        break;
    case URL_PARAM_APMAC:
        vty_out(vty, "[apmac]:\n   name:%s\n   deskey:%s\n   format:%s\n   letter:%s\n", 
                param->param_name, param->mac_deskey, 
                strlen(param->mac_format)>0?(param->mac_format):":", 
            	(UP_LETTER_LOWER == param->letter_type)?"lower":"upper");
		if (strlen(param->param_value) > 0) {
			vty_out(vty, "   value:%s\n", param->param_value);
        }
        break;
    case URL_PARAM_APNAME:
        vty_out(vty, "[apname]:\n   name:%s\n", param->param_name);
		if (strlen(param->param_value) > 0) {
			vty_out(vty, "   value:%s\n", param->param_value);
        }
        break;
    case URL_PARAM_ESSID:
        vty_out(vty, "[essid]:\n   name:%s\n", param->param_name);
		if (strlen(param->param_value) > 0) {
			vty_out(vty, "   value:%s\n", param->param_value);
        }
        break;
    case URL_PARAM_NASID:
        vty_out(vty, "[nasid]:\n   name:%s\n", param->param_name);
		if (strlen(param->param_value) > 0) {
			vty_out(vty, "   value:%s\n", param->param_value);
        }
        break;
    case URL_PARAM_ACNAME:
        vty_out(vty, "[acname]:\n   name:%s\n   value:%s\n", param->param_name, param->param_value);
		break;
    case URL_PARAM_FIRSTURL:
        vty_out(vty, "[firsturl]:\n   name:%s\n   type:%s\n   encode:%s\n", 
                param->param_name, (UP_HTTPS == param->url_type)?"https":"http", 
                (UP_ENCODE_OFF == param->url_encode)?"off":"on");
		if (strlen(param->param_value) > 0) {
			vty_out(vty, "   value:%s\n", param->param_value);
        }
        break;
    default:
        break;
	}

	return 0;
}

static int eag_urlparam_show_v1( struct vty* vty, 
							struct urlparam_query_str_t *urlparam, 
							URLPARAM com_or_wis )
{
    if (urlparam == NULL || vty == NULL) {
		return -1;
    }
    
    struct url_param_t *param = NULL;
	int i = 0;

	if (UP_COMMON == com_or_wis) {
		for(i = 0; i < urlparam->common_param_num; i++) {
			param = &(urlparam->common_param[i]);
			eag_urlparam_show_v2(vty, param);
		}
	} else if (UP_WISPR == com_or_wis) {
		for(i = 0; i < urlparam->wispr_param_num; i++) {
			param = &(urlparam->wispr_param[i]);
			eag_urlparam_show_v2(vty, param);
		}
	}
	
	return 0;
}

static int eag_urlparam_show(struct vty* vty, struct urlparam_query_str_t *urlparam)
{
	if (urlparam == NULL || vty == NULL) {
		return -1;
	}
	
    vty_out(vty, "========================= url param =========================\n");
    if (urlparam->common_param_num > 0) {
        eag_urlparam_show_v1(vty, urlparam, UP_COMMON);
    }
    if (UP_STATUS_ON == urlparam->wispr_status) {
        vty_out(vty, "[wisprurl]:\n   name:%s\n   type:%s\n   encode:%s\n", 
                urlparam->wispr_name, (UP_HTTPS == urlparam->wispr_type)?"https":"http", 
                (UP_ENCODE_OFF == urlparam->wispr_encode)?"off":"on");
        if (strlen(urlparam->wispr_value) > 0) {
            vty_out(vty, "   value:%s\n", urlparam->wispr_value);
        }
        if (urlparam->wispr_param_num > 0) {
            vty_out(vty, "=================== wispr param ===================\n");
            eag_urlparam_show_v1(vty, urlparam, UP_WISPR);
            vty_out(vty, "==================== wispr END ====================\n");
        }
    }
    vty_out(vty, "============================ END ============================\n");

	return 0;
}

DEFUN(show_eag_portal_server_urlparam_func,
	show_eag_portal_server_urlparam_cmd,
	"set portal-server (wlanid|vlanid|wtpid|interface) KEY urlparam-show",
	"set parameter of portal policy, if not exist, create it\n"
	"portal policy\n"
	"portal server index type wlanid\n" 
	"portal server index type vlanid\n" 	
	"portal server index type wtpid\n"	
	"portal server index type l3interface name\n"
	"key word name, specify a item of portal policy\n"
	"urlparam-show\n"
)
{
	PORTAL_KEY_TYPE key_type;
	unsigned long keyid = 0;
	char *key_word = "";
	struct portal_srv_t portal_srv = {0};
	int ret = -1;

	if (strncmp(argv[0], "wlanid", strlen(argv[0])) == 0) {
		key_type = PORTAL_KEYTYPE_WLANID;
		keyid = atoi(argv[1]);
		if (keyid == 0 || keyid > 128){
			vty_out(vty, "%% wlan id is out of range 1~128\n");
			return CMD_SUCCESS;
		}
	}
	else if (strncmp(argv[0], "vlanid", strlen(argv[0])) == 0) {
		key_type = PORTAL_KEYTYPE_VLANID;
		keyid = atoi(argv[1]);
		if(keyid == 0 || keyid > 4096) {
			vty_out(vty, "%% vlan id is out of range 1~4096\n");
			return CMD_SUCCESS;
		}		
	}
	else if (strncmp(argv[0], "wtpid", strlen(argv[0])) == 0){
		keyid = atoi(argv[1]);
		key_type = PORTAL_KEYTYPE_WTPID;
	}
	else if (strncmp(argv[0], "interface", strlen(argv[0])) == 0) {
		key_type = PORTAL_KEYTYPE_INTF;
		key_word = (char *)argv[1];
	}
	else {
		vty_out(vty, "%% unknown index type %s\n", argv[0]);
		return CMD_SUCCESS;
	}
	
	EAG_DCLI_INIT_HANSI_INFO

	ret = eag_show_portal_server_urlparam( dcli_dbus_connection_curr,
										hansitype,insid,
										key_type,
										keyid,
										key_word, 
										&portal_srv );
	if (EAG_RETURN_OK == ret) {
		eag_urlparam_show(vty, &(portal_srv.urlparam_query_str));
	}
	
	if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_ERR_INPUT_PARAM_ERR == ret) {
		vty_out(vty, "%% input prarm error\n");
	}
	else if (EAG_ERR_PORTAL_ADD_SRV_ERR_TYPE == ret) {
		vty_out(vty, "%% keyword type error\n");
	}
	else if (EAG_ERR_PORTAL_MODIFY_SRV_NOT_EXIST == ret) {
		vty_out(vty, "%% the add key is not exist\n");
	}
	else if (EAG_ERR_PORTAL_MODIFY_SRV_ERR_TYPE == ret) {
		vty_out(vty, "%% error index type\n");
	}
	else if (EAG_RETURN_OK != ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}
}

DEFUN(set_eag_portal_server_essid_func,
	set_eag_portal_server_essid_cmd,
	"set portal-server essid .ARGUMENTS",
	"set parameter of portal policy, if not exist, create it\n"
	"portal policy\n"
	"portal server index type essid\n"	
	"eg:set portal-server essid KEY (ac-name|acip-to-url|nasid-to-url|url-suffix|wlanparameter|wlanuserfirsturl|secret|wlanapmac|wlanusermac|wisprlogin|usermac-to-url|clientmac-to-url|apmac-to-url|wlan-to-url|redirect-to-url|urlparam-add|urlparam-del|urlparam-show) params,the format like other keytypes.\n"		
)
{
	char *key_word = (char *)argv[0];
	unsigned long keyid = 0;
	PORTAL_KEY_TYPE key_type = PORTAL_KEYTYPE_ESSID;
	int ret = -1;
	int i = 0;
	char essid[MAX_PORTAL_SSID_LEN] = {0};
	int func_type = 0;
	int if_enable = 0;
	char *acname = NULL;
	char *DESkey = NULL;
	char *url_suffix = NULL;
	char *secret = NULL;
	int add_or_del = 0;
	char *url_param = NULL;
	struct portal_srv_t portal_srv = {0};
	
	for (i = 0;i < argc; i++) {
		if (0 != i) {
			if (0 == strcmp(argv[i], "ac-name")) {
				func_type = 1;
				break;
			} else if (0 == strcmp(argv[i], "acip-to-url")) {
				func_type = 2;
				break;
			} else if (0 == strcmp(argv[i], "nasid-to-url")) {
				func_type = 3;
				break;
			} else if (0 == strcmp(argv[i], "url-suffix")) {
				func_type = 4;
				break;
			} else if (0 == strcmp(argv[i], "wlanparameter")) {
				func_type = 5;
				break;
			} else if (0 == strcmp(argv[i], "wlanuserfirsturl")) {
				func_type = 6;
				break;
			} else if (0 == strcmp(argv[i], "secret")) {
				func_type = 9;
				break;
			} else if (0 == strcmp(argv[i], "wlanapmac")) {
				func_type = 10;
				break;
			} else if (0 == strcmp(argv[i], "wlanusermac")) {
				func_type = 11;
				break;
			} else if (0 == strcmp(argv[i], "wisprlogin")) {
				func_type = 12;
				break;
			} else if (0 == strcmp(argv[i], "usermac-to-url")) {
				func_type = 13;
				break;
			} else if (0 == strcmp(argv[i], "clientmac-to-url")) {
				func_type = 14;
				break;
			} else if (0 == strcmp(argv[i], "apmac-to-url")) {
				func_type = 15;
				break;
			} else if (0 == strcmp(argv[i], "wlan-to-url")) {
				func_type = 16;
				break;
			} else if (0 == strcmp(argv[i], "redirect-to-url")) {
				func_type = 17;
				break;
			} else if (0 == strcmp(argv[i], "urlparam-add")) {
				func_type = 18;
				break;
			} else if (0 == strcmp(argv[i], "urlparam-del")) {
				func_type = 19;
				break;
			} else if (0 == strcmp(argv[i], "urlparam-show")) {
				func_type = 20;
				break;
			}
			strncat(essid, " ", 1);
		}
		if ((strlen(essid) + strlen(argv[i])) > MAX_PORTAL_SSID_LEN-1) {
			vty_out(vty, "%% essid length is too long\n");
			return CMD_SUCCESS;
		}
		strncat(essid, argv[i], strlen(argv[i]));
		key_word = essid;
	}

	EAG_DCLI_INIT_HANSI_INFO
	switch(func_type) {
	case 1:
		if ((i+2) == argc) {
			acname = (char *)argv[i+1];
			if (strlen(acname) > MAX_PORTAL_ACNAME_LEN-1){
				vty_out(vty, "%% error acname format\n");
				return CMD_WARNING;
			}
		} else if ((i+1) == argc) { 
			acname = "";
		} else {
			vty_out(vty, "%% argument is wrong\n");
		}		
		ret = eag_set_portal_server_acname(dcli_dbus_connection_curr, hansitype, insid,
										key_type, keyid, key_word, acname);
		break;
	case 2:
		if ((i+2) == argc){
			if (0 == strncmp(argv[i+1],"enable",strlen(argv[i+1]))) {
				if_enable = 1;
			} else if (0 == strncmp(argv[i+1],"disable",strlen(argv[i+1]))) {
				if_enable = 0;
			} else {
				vty_out(vty, "%% argument is wrong\n");
				return CMD_WARNING;
			}
		} else {
			vty_out(vty, "%% argument is wrong\n");
			return CMD_WARNING;
		}
		ret = eag_set_portal_server_acip_to_url(dcli_dbus_connection_curr, hansitype, insid,
										key_type, keyid, key_word, if_enable);
		break;
	case 3:
		if ((i+2) == argc){
			if (0 == strncmp(argv[i+1],"enable",strlen(argv[i+1]))) {
				if_enable = 1;
			} else if (0 == strncmp(argv[i+1],"disable",strlen(argv[i+1]))) {
				if_enable = 0;
			} else {
				vty_out(vty, "%% argument is wrong\n");
				return CMD_WARNING;
			}
		} else {
			vty_out(vty, "%% argument is wrong\n");
			return CMD_WARNING;
		}
		ret = eag_set_portal_server_nasid_to_url(dcli_dbus_connection_curr, hansitype, insid,
										key_type, keyid, key_word, if_enable);
		break;
	case 4:
		if ((i+2) == argc) {
			url_suffix = (char *)argv[i+1];
			if (strlen(url_suffix) > MAX_PORTAL_URL_SUFFIX_LEN-1){
				vty_out(vty, "%% error url-suffix format\n");
				return CMD_WARNING;
			}
			if (NULL == strchr(url_suffix,'=')) {
				vty_out(vty,"%% error url-suffix format,please print like 'suffix_name=value'\n");
				return CMD_WARNING;
			}
		} else if ((i+1) == argc) { 
			url_suffix = "";
		} else {
			vty_out(vty, "%% argument is wrong\n");
		}

		ret = eag_set_portal_server_url_suffix(dcli_dbus_connection_curr, hansitype, insid,
										key_type, keyid, key_word, url_suffix);
		break;
	case 5:
		if ((i+2) == argc) {
			if (0 == strncmp(argv[i+1],"enable",strlen(argv[i+1]))) {
				//if_enable = 1;
				vty_out(vty, "%% you should set DESKey! eg:set portal-server essid KEY wlanparameter enable 12312312\n");
				return CMD_WARNING;
			} else if (0 == strncmp(argv[i+1],"disable",strlen(argv[i+1]))) {
				if_enable = 0;
				DESkey = "";
			} else {
				vty_out(vty, "%% argument is wrong\n");
				return CMD_WARNING;
			}
		} else if ((i+3) == argc) {
			if (0 == strncmp(argv[i+1],"enable",strlen(argv[i+1]))) {
				if_enable = 1;
				DESkey = (char *)argv[i+2];
			} else {
				vty_out(vty, "%% argument is wrong\n");
				return CMD_WARNING;
			}
		}
		else {
			vty_out(vty, "%% argument is wrong\n");
			return CMD_WARNING;
		}
		ret = eag_set_portal_server_wlanparameter(dcli_dbus_connection_curr, hansitype, insid,
										key_type, keyid, key_word, if_enable, DESkey);		
		break;
	case 6:
		if ((i+2) == argc){
			if (0 == strncmp(argv[i+1],"enable",strlen(argv[i+1]))) {
				if_enable = 1;
			} else if (0 == strncmp(argv[i+1],"disable",strlen(argv[i+1]))) {
				if_enable = 0;
			} else {
				vty_out(vty, "%% argument is wrong\n");
				return CMD_WARNING;
			}
		} else {
			vty_out(vty, "%% argument is wrong\n");
			return CMD_WARNING;
		}
		ret = eag_set_portal_server_wlanuserfirsturl(dcli_dbus_connection_curr, hansitype, insid,
										key_type, keyid, key_word, if_enable);
		break;
	case 9:
		if ((i+2) == argc) {
			secret = (char *)argv[i+1];
			if (strlen(secret) > PORTAL_SECRETSIZE-1){
				vty_out(vty, "%% error secret format\n");
				return CMD_WARNING;
			}
		} else if ((i+1) == argc) { 
			secret = "";
		} else {
			vty_out(vty, "%% argument is wrong\n");
		}		
		ret = eag_set_portal_server_secret(dcli_dbus_connection_curr, hansitype, insid,
										key_type, keyid, key_word, secret);
		break;
	case 10:
		if ((i+2) == argc){
			if (0 == strncmp(argv[i+1],"enable",strlen(argv[i+1]))) {
				if_enable = 1;
			} else if (0 == strncmp(argv[i+1],"disable",strlen(argv[i+1]))) {
				if_enable = 0;
			} else {
				vty_out(vty, "%% argument is wrong\n");
				return CMD_WARNING;
			}
		} else {
			vty_out(vty, "%% argument is wrong\n");
			return CMD_WARNING;
		}
		ret = eag_set_portal_server_wlanapmac(dcli_dbus_connection_curr, hansitype, insid,
										key_type, keyid, key_word, if_enable);
		break;
	case 11:
		if ((i+2) == argc) {
			if (0 == strncmp(argv[i+1],"enable",strlen(argv[i+1]))) {
				vty_out(vty, "%% you should set DESKey! eg:set portal-server essid KEY wlanusermac enable 12312312\n");
				return CMD_WARNING;
			} else if (0 == strncmp(argv[i+1],"disable",strlen(argv[i+1]))) {
				if_enable = 0;
				DESkey = "";
			} else {
				vty_out(vty, "%% argument is wrong\n");
				return CMD_WARNING;
			}
		} else if ((i+3) == argc) {
			if (0 == strncmp(argv[i+1],"enable",strlen(argv[i+1]))) {
				if_enable = 1;
				DESkey = (char *)argv[i+2];
			} else {
				vty_out(vty, "%% argument is wrong\n");
				return CMD_WARNING;
			}
		}
		else {
			vty_out(vty, "%% argument is wrong\n");
			return CMD_WARNING;
		}
		ret = eag_set_portal_server_wlanusermac(dcli_dbus_connection_curr, hansitype, insid,
										key_type, keyid, key_word, if_enable, DESkey);	
		break;
	case 12:{
		char *type=NULL;
		if ((i+2) == argc) {
			if (0 == strncmp(argv[i+1],"enable",strlen(argv[i+1]))) {
				vty_out(vty, "%% you should set url type! eg:set portal-server essid KEY wlanusermac enable (http|https)\n");
				return CMD_WARNING;
			} else if (0 == strncmp(argv[i+1],"disable",strlen(argv[i+1]))) {
				if_enable = 0;
				type = "";
			} else {
				vty_out(vty, "%% argument is wrong\n");
				return CMD_WARNING;
			}
		} else if ((i+3) == argc) {
			if (0 == strncmp(argv[i+1],"enable",strlen(argv[i+1]))) {
				if_enable = 1;
				type = (char *)argv[i+2];
			} else {
				vty_out(vty, "%% argument is wrong\n");
				return CMD_WARNING;
			}
		}
		else {
			vty_out(vty, "%% argument is wrong\n");
			return CMD_WARNING;
		}
		ret = eag_set_portal_server_wisprlogin(dcli_dbus_connection_curr, hansitype, insid,
										key_type, keyid, key_word, if_enable, type);	
		}
		break;
		case 13:
		if ((i+2) == argc){
			if (0 == strncmp(argv[i+1],"enable",strlen(argv[i+1]))) {
				if_enable = 1;
			} else if (0 == strncmp(argv[i+1],"disable",strlen(argv[i+1]))) {
				if_enable = 0;
			} else {
				vty_out(vty, "%% argument is wrong\n");
				return CMD_WARNING;
			}
		} else {
			vty_out(vty, "%% argument is wrong\n");
			return CMD_WARNING;
		}
		ret = eag_set_portal_server_usermac_to_url(dcli_dbus_connection_curr, hansitype, insid,
										key_type, keyid, key_word, if_enable);
		break;
		case 14:
		if ((i+2) == argc){
			if (0 == strncmp(argv[i+1],"enable",strlen(argv[i+1]))) {
				if_enable = 1;
			} else if (0 == strncmp(argv[i+1],"disable",strlen(argv[i+1]))) {
				if_enable = 0;
			} else {
				vty_out(vty, "%% argument is wrong\n");
				return CMD_WARNING;
			}
		} else {
			vty_out(vty, "%% argument is wrong\n");
			return CMD_WARNING;
		}
		ret = eag_set_portal_server_clientmac_to_url(dcli_dbus_connection_curr, hansitype, insid,
										key_type, keyid, key_word, if_enable);
		break;
		case 15:
		if ((i+2) == argc){
			if (0 == strncmp(argv[i+1],"enable",strlen(argv[i+1]))) {
				if_enable = 1;
			} else if (0 == strncmp(argv[i+1],"disable",strlen(argv[i+1]))) {
				if_enable = 0;
			} else {
				vty_out(vty, "%% argument is wrong\n");
				return CMD_WARNING;
			}
		} else {
			vty_out(vty, "%% argument is wrong\n");
			return CMD_WARNING;
		}
		ret = eag_set_portal_server_apmac_to_url(dcli_dbus_connection_curr, hansitype, insid,
										key_type, keyid, key_word, if_enable);
		break;
		case 16:
		if ((i+2) == argc){
			if (0 == strncmp(argv[i+1],"enable",strlen(argv[i+1]))) {
				if_enable = 1;
			} else if (0 == strncmp(argv[i+1],"disable",strlen(argv[i+1]))) {
				if_enable = 0;
			} else {
				vty_out(vty, "%% argument is wrong\n");
				return CMD_WARNING;
			}
		} else {
			vty_out(vty, "%% argument is wrong\n");
			return CMD_WARNING;
		}
		ret = eag_set_portal_server_wlan_to_url(dcli_dbus_connection_curr, hansitype, insid,
										key_type, keyid, key_word, if_enable);
		break;	
		case 17:
		if ((i+2) == argc){
			if (0 == strncmp(argv[i+1],"enable",strlen(argv[i+1]))) {
				if_enable = 1;
			} else if (0 == strncmp(argv[i+1],"disable",strlen(argv[i+1]))) {
				if_enable = 0;
			} else {
				vty_out(vty, "%% argument is wrong\n");
				return CMD_WARNING;
			}
		} else {
			vty_out(vty, "%% argument is wrong\n");
			return CMD_WARNING;
		}
		ret = eag_set_portal_server_redirect_to_url(dcli_dbus_connection_curr, hansitype, insid,
										key_type, keyid, key_word, if_enable);
		break;
		case 18:
        case 19:
   		if (strncmp(argv[i], "urlparam-add", strlen(argv[i])) == 0) {
			add_or_del = 1;
		} else if (strncmp(argv[i], "urlparam-del", strlen(argv[i])) == 0) {
			add_or_del = 0;
		}
        if ((i+2) == argc) {
            url_param = (char *)argv[i+1];
            if (strlen(url_param) > URL_PARAM_QUERY_STR_LEN-1){
				vty_out(vty, "%% url-param length out of limite:%d\n", URL_PARAM_QUERY_STR_LEN-1);
				return CMD_WARNING;
			}
        } else {
            vty_out(vty, "%% argument is wrong\n");
			return CMD_WARNING;
        }
        
		ret = eag_set_portal_server_urlparam( dcli_dbus_connection_curr, hansitype,insid, 
										key_type, keyid, key_word, add_or_del, url_param);
		break;
		case 20:
   		if (strncmp(argv[i], "urlparam-show", strlen(argv[i])) == 0) {
            ret = eag_show_portal_server_urlparam( dcli_dbus_connection_curr,
                                                hansitype,insid,
                                                key_type,
                                                keyid,
                                                key_word, 
                                                &portal_srv );
            if (EAG_RETURN_OK == ret) {
                eag_urlparam_show(vty, &(portal_srv.urlparam_query_str));
            }
        } else {
            vty_out(vty, "%% argument is wrong\n");
			return CMD_WARNING;
        }
		break;
	default:
		break;
	}
	
	if( EAG_ERR_DBUS_FAILED == ret ){
		vty_out(vty,"%% dbus error\n");
	}else if( EAG_ERR_INPUT_PARAM_ERR == ret ){
		vty_out(vty,"%% input prarm error\n");
	}else if( EAG_ERR_PORTAL_ADD_SRV_ERR_TYPE == ret ){
		vty_out(vty,"%% keyword type error\n");
	}else if( EAG_ERR_PORTAL_MODIFY_SRV_NOT_EXIST == ret ){
		vty_out(vty,"%% the add key is not exist\n" );
	}else if( EAG_ERR_PORTAL_MODIFY_SRV_ERR_TYPE == ret ){
		vty_out(vty,"%% error index type\n" );
	}else if( EAG_ERR_PORTAL_WLANPARAMTER_DESKEY_LEN_LIMITE == ret ){
		vty_out(vty,"%% DESKey length should not larger than %d\n",MAX_DES_KEY_LEN);
	}else if( EAG_ERR_PORTAL_SET_WISPR_URL_TYPE_ERR == ret ){
		vty_out(vty,"%% error url type error,should be http or https\n");
	}
	else if( EAG_RETURN_OK != ret ){
		vty_out(vty,"%% unknown error\n" );
	}
}

DEFUN(eag_modify_portal_server_func,
	eag_modify_portal_server_cmd,
	"modify portal-server (wlanid|vlanid|wtpid|interface) KEY URL <1-65535>",
	SHOW_STR
	"modify portal-server url\n"
	"portal server index type wlanid\n"	
	"portal server index type vlanid\n"		
	"portal server index type wtpid\n"	
	"portal server index type l3interface name\n"
	"portal server index key\n"	
	"portal server url(e.g.:http://A.B.C.D:PORT/page.html)\n"
	"portal server ntf_logout port\n"
)
{
	int ret = -1;
	PORTAL_KEY_TYPE key_type;
	unsigned long keyid = 0;
	const char *keystr = "";
	const char *portal_url = argv[2];
	uint16_t ntf_port = atoi(argv[3]);
	const char *domain = "";
	uint32_t mac_server_ip = 0;
	uint16_t mac_server_port = 0;
	struct in_addr addr = {0};

	if (5 == argc) {
		domain = argv[4];
	} else if (6 == argc) {
		ret = inet_aton(argv[4], &addr);
		if (!ret) {
			vty_out(vty, "%% invalid ip address\n");
			return CMD_WARNING;
		}
		mac_server_ip = ntohl(addr.s_addr);
		mac_server_port = atoi(argv[5]);
	} else if (7 == argc) {
		domain = argv[4];
		ret = inet_aton(argv[5], &addr);
		if (!ret) {
			vty_out(vty, "%% invalid ip address\n");
			return CMD_WARNING;
		}
		mac_server_ip = ntohl(addr.s_addr);
		mac_server_port = atoi(argv[6]);
	}

	if (strncmp(argv[0], "wlanid", strlen(argv[0])) == 0) {
		key_type = PORTAL_KEYTYPE_WLANID;
		keyid = atoi(argv[1]);
		if (keyid == 0 || keyid > 128){
			vty_out(vty, "%% wlan id is out of range 1~128\n");
			return CMD_SUCCESS;
		}
	}
	else if (strncmp(argv[0], "vlanid", strlen(argv[0])) == 0) {
		key_type = PORTAL_KEYTYPE_VLANID;
		keyid = atoi(argv[1]);
		if(keyid == 0 || keyid > 4096) {
			vty_out(vty, "%% vlan id is out of range 1~4096\n");
			return CMD_SUCCESS;
		}		
	}
	else if (strncmp(argv[0], "wtpid", strlen(argv[0])) == 0){
		keyid = atoi(argv[1]);
		key_type = PORTAL_KEYTYPE_WTPID;
	}
	else if (strncmp(argv[0], "interface", strlen(argv[0])) == 0) {
		key_type = PORTAL_KEYTYPE_INTF;
		keystr = (char *)argv[1];
	}
	else {
		vty_out(vty, "%% unknown index type %s\n", argv[0]);
		return CMD_SUCCESS;
	}

	EAG_DCLI_INIT_HANSI_INFO
#if 0		
	if(eag_ins_running_state(dcli_dbus_connection_curr, hansitype, insid)){
		vty_out(vty, "eag instance %d is running, please stop it first\n",insid);
		return CMD_FAILURE;
	}
#endif	
	ret = eag_modify_portal_server( dcli_dbus_connection_curr,
										hansitype,insid,					
										key_type,
										keyid,
										keystr,
										portal_url, 
										ntf_port,
										domain ,
										mac_server_ip,
										mac_server_port);
	if(EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_ERR_UNKNOWN == ret) {
		vty_out(vty, "%% unknown error\n");
	}
	else if (EAG_ERR_PORTAL_MODIFY_SRV_NOT_EXIST == ret) {
		vty_out(vty, "%% the modify key is not exist\n");
	}
	else if (EAG_ERR_PORTAL_MODIFY_SRV_ERR_TYPE == ret) {
		vty_out(vty, "%% error index type\n");
	}
	else if (EAG_RETURN_OK != ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}
	
	return CMD_SUCCESS;
}


ALIAS(eag_modify_portal_server_func,
	eag_modify_portal_server_cmd_domain,
	"modify portal-server (wlanid|vlanid|wtpid|interface) KEY URL <1-65535> domain DOMAIN",
	SHOW_STR
	"modify portal-server url\n"
	"portal server index type wlanid\n"	
	"portal server index type vlanid\n"		
	"portal server index type wtpid\n"	
	"portal server index type l3interface name\n"
	"portal server index key\n"	
	"portal server url(e.g.:http://A.B.C.D:PORT/page.html)\n"
	"portal server ntf_logout port\n"
	"portal server bind to radius domain\n"
	"portal server bind to radius domain\n"
)

ALIAS(eag_modify_portal_server_func,
	eag_modify_portal_server_cmd_macauth,
	"modify portal-server (wlanid|vlanid|wtpid|interface) KEY URL <1-65535> macauth A.B.C.D <1-65535>",
	SHOW_STR
	"modify portal-server url\n"
	"portal server index type wlanid\n"	
	"portal server index type vlanid\n"		
	"portal server index type wtpid\n"	
	"portal server index type l3interface name\n"
	"portal server index key\n"	
	"portal server url(e.g.:http://A.B.C.D:PORT/page.html)\n"
	"portal server ntf_logout port\n"
	"mac auth server\n"
	"mac auth server ip\n"
	"mac auth server port\n"
)

ALIAS(eag_modify_portal_server_func,
	eag_modify_portal_server_cmd_domain_macauth,
	"modify portal-server (wlanid|vlanid|wtpid|interface) KEY URL <1-65535> domain DOMAIN macauth A.B.C.D <1-65535>",
	SHOW_STR
	"modify portal-server url\n"
	"portal server index type wlanid\n"	
	"portal server index type vlanid\n"		
	"portal server index type wtpid\n"	
	"portal server index type l3interface name\n"
	"portal server index key\n"	
	"portal server url(e.g.:http://A.B.C.D:PORT/page.html)\n"
	"portal server ntf_logout port\n"
	"portal server bind to radius domain\n"
	"portal server bind to radius domain\n"
	"mac auth server\n"
	"mac auth server ip\n"
	"mac auth server port\n"
)

DEFUN(eag_modify_portal_server_essid_func,
	eag_modify_portal_server_essid_cmd,
	"modify portal-server essid .ARGUMENTS",
	SHOW_STR
	"modify portal-server url\n"
	"portal server index type essid\n"	
	"eg:modify portal-server essid KEY_WORD URL PORT [domain DOMAIN] [macauth A.B.C.D <1-65535>]\n"		
)
{
	int ret = -1;
	int i = 0;
	PORTAL_KEY_TYPE key_type = PORTAL_KEYTYPE_ESSID;
	unsigned long keyid = 0;
	const char *keystr = "";
	const char *portal_url = "";
	int ntf_port = 0;
	const char *domain = "";
	char essid[MAX_PORTAL_SSID_LEN] = {0};
	char http_str[10] = "http://";
	char https_str[10] = "https://";
	uint32_t mac_server_ip = 0;
	uint16_t mac_server_port = 0;
	struct in_addr addr = {0};
	const char *tmp_port = NULL;

	for (i = 0; i < argc && 0 != strncmp(argv[i], http_str, strlen(http_str))
		&& 0 != strncmp(argv[i], https_str, strlen(https_str)); i++) 
	{
		if (0 != i) {
			strncat(essid, " ", 1);
		}
		if ((strlen(essid) + strlen(argv[i])) > MAX_PORTAL_SSID_LEN-1) {
			vty_out(vty, "essid length is too long\n");
			return CMD_SUCCESS;
		}
		strncat(essid, argv[i], strlen(argv[i]));
		keystr = essid;
	}
	if (strlen(keystr) == 0) {
		vty_out(vty, "essid is null\n");
		return CMD_SUCCESS;
	}
	if (i + 2 == argc) {	;
	} else if (i + 4 == argc) {
		if (0 != strncmp(argv[i+2], "domain", strlen(argv[i+2]))) {
			vty_out(vty, "Used as: modify portal-server essid KEY_WORD URL PORT [domain DOMAIN] [macauth A.B.C.D <1-65535>]\n");
			return CMD_SUCCESS;
		}
		domain = argv[i+3];
	} else if (i + 5 == argc) {
		if (0 != strncmp(argv[i+2], "macauth", strlen(argv[i+2]))) {
			vty_out(vty, "Used as: modify portal-server essid KEY_WORD URL PORT [domain DOMAIN] [macauth A.B.C.D <1-65535>]\n");
			return CMD_SUCCESS;
		}
		ret = inet_aton(argv[i+3], &addr);
		if (!ret) {
			vty_out(vty, "%% invalid ip address\n");
			return CMD_WARNING;
		}
		mac_server_ip = ntohl(addr.s_addr);
		mac_server_port = atoi(argv[i+4]);
	} else if (i + 7 == argc) {
		if (0 != strncmp(argv[i+2], "domain", strlen(argv[i+2]))) {
			vty_out(vty, "Used as: modify portal-server essid KEY_WORD URL PORT [domain DOMAIN] [macauth A.B.C.D <1-65535>]\n");
			return CMD_SUCCESS;
		}
		domain = argv[i+3];
		if (0 != strncmp(argv[i+4], "macauth", strlen(argv[i+4]))) {
			vty_out(vty, "Used as: modify portal-server essid KEY_WORD URL PORT [domain DOMAIN] [macauth A.B.C.D <1-65535>]\n");
			return CMD_SUCCESS;
		}
		ret = inet_aton(argv[i+5], &addr);
		if (!ret) {
			vty_out(vty, "%% invalid ip address\n");
			return CMD_WARNING;
		}
		mac_server_ip = ntohl(addr.s_addr);
		mac_server_port = atoi(argv[i+6]);
	} else if (i + 7 < argc) {
		vty_out(vty, "%% too many argument\n");
		return CMD_SUCCESS;
	} else if (i + 2 > argc) {
		vty_out(vty, "%% too few argument\n");
		return CMD_SUCCESS;
	} else {
		vty_out(vty, "Used as: modify portal-server essid KEY_WORD URL PORT [domain DOMAIN] [macauth A.B.C.D <1-65535>]\n");
		return CMD_SUCCESS;
	}
	portal_url = argv[i];
	tmp_port = argv[++i];
	if (0 == is_number(tmp_port)) {
		vty_out(vty, "%% ntf port has illegal character\n");
		return CMD_SUCCESS;
	}
	ntf_port = atoi(tmp_port);
	if (ntf_port <= 0 || ntf_port > 65535) {
		vty_out(vty, "%% ntf port must be in range 1~65535 \n");
		return CMD_SUCCESS;
	}
	

	EAG_DCLI_INIT_HANSI_INFO
#if 0
	if(eag_ins_running_state(dcli_dbus_connection_curr,hansitype, insid)){
		vty_out(vty, "eag instance %d is running, please stop it first\n",insid);
		return CMD_FAILURE;
	}
#endif	
	ret = eag_modify_portal_server( dcli_dbus_connection_curr,
										hansitype,insid,					
										key_type,
										keyid,
										keystr,
										portal_url, 
										ntf_port,
										domain ,
										mac_server_ip,
										mac_server_port);
	if(EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_ERR_UNKNOWN == ret) {
		vty_out(vty, "%% unknown error\n");
	}
	else if (EAG_ERR_PORTAL_MODIFY_SRV_NOT_EXIST == ret) {
		vty_out(vty, "%% the modify key is not exist\n");
	}
	else if (EAG_ERR_PORTAL_MODIFY_SRV_ERR_TYPE == ret) {
		vty_out(vty, "%% error index type\n");
	}
	else if (EAG_RETURN_OK != ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}
	
	return CMD_SUCCESS;
}


DEFUN(eag_show_portal_conf_func,
	eag_show_portal_conf_cmd,
	"show portal-server",
	SHOW_STR
	"show portal-server configuration\n"
)
{
	int ret = -1;
	int i = 0;
	struct portal_conf portalconf;
	memset( &portalconf, 0, sizeof(struct portal_conf) );
	char ip_str[32] = "";
	
	EAG_DCLI_INIT_HANSI_INFO
		
	ret = eag_get_portal_conf( dcli_dbus_connection_curr, 
								hansitype,insid,
								&portalconf );

	if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error!\n");
	}
	else if (EAG_ERR_INPUT_PARAM_ERR == ret) {
		vty_out(vty, "%% input prarm error!\n");
	}
	else if (EAG_RETURN_OK != ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	} 
	else {		
		vty_out( vty, "Portal-server num = %d\n", portalconf.current_num);
		for( i=0; i<portalconf.current_num; i++ ) {
			vty_out( vty, "===========================================================\n" );
			switch(portalconf.portal_srv[i].key_type) {
			case PORTAL_KEYTYPE_ESSID:
				vty_out(vty, "Portal key type           :Essid\n");
				vty_out(vty, "portal key word           :%s\n", portalconf.portal_srv[i].key.essid);
				break;
			case PORTAL_KEYTYPE_WLANID:
				vty_out(vty, "Portal key type           :Wlanid\n");
				vty_out(vty, "portal key word           :%lu\n", portalconf.portal_srv[i].key.wlanid);
				break;
			case PORTAL_KEYTYPE_VLANID:
				vty_out(vty, "Portal key type           :Vlanid\n");
				vty_out(vty, "portal key word           :%lu\n", portalconf.portal_srv[i].key.vlanid);
				break;
			case PORTAL_KEYTYPE_WTPID:
				vty_out(vty, "Portal key type           :Wtpid\n");
				vty_out(vty, "portal key word           :%lu\n", portalconf.portal_srv[i].key.wtpid);
				break;
			case PORTAL_KEYTYPE_INTF:
				vty_out(vty, "Portal key type           :Intf\n");
				vty_out(vty, "portal key word           :%s\n", portalconf.portal_srv[i].key.intf);
				break;
			default:
				vty_out(vty, "Portal key type           :\n");
				break;
			}				
			vty_out(vty, "portal url                :%s\n", portalconf.portal_srv[i].portal_url);				
			vty_out(vty, "portal ntfport            :%u\n", portalconf.portal_srv[i].ntf_port);
			if(0 != strcmp(portalconf.portal_srv[i].domain, ""))
				vty_out(vty, "portal domain             :%s\n", portalconf.portal_srv[i].domain);
			if(0 != strcmp(portalconf.portal_srv[i].acname, ""))
				vty_out(vty, "portal acname             :%s\n", portalconf.portal_srv[i].acname);
			if (MACBIND_SERVER_DOMAIN == portalconf.portal_srv[i].ip_or_domain) {
				vty_out(vty, "macbind server domain     :%s\n", portalconf.portal_srv[i].macbind_server_domain);
            }
			ip2str(portalconf.portal_srv[i].mac_server_ip, ip_str, sizeof(ip_str));
			vty_out(vty, "macbind server ip         :%s\n", ip_str);
			vty_out(vty, "macbind server port       :%u\n", portalconf.portal_srv[i].mac_server_port);
			
			vty_out(vty, "portal acip-to-url        :%s\n", (1 == portalconf.portal_srv[i].acip_to_url)?"enable":"disable");
			vty_out(vty, "portal usermac-to-url     :%s\n", (1 == portalconf.portal_srv[i].usermac_to_url)?"enable":"disable");
			vty_out(vty, "portal clientmac-to-url   :%s\n", (1 == portalconf.portal_srv[i].clientmac_to_url)?"enable":"disable");	
			vty_out(vty, "portal apmac-to-url       :%s\n", (1 == portalconf.portal_srv[i].apmac_to_url)?"enable":"disable");	
			vty_out(vty, "portal wlan-to-url        :%s\n", (1 == portalconf.portal_srv[i].wlan_to_url)?"enable":"disable");	
			vty_out(vty, "portal redirect-to-url    :%s\n", (1 == portalconf.portal_srv[i].redirect_to_url)?"enable":"disable");			
			vty_out(vty, "portal nasid-to-url       :%s\n", (1 == portalconf.portal_srv[i].nasid_to_url)?"enable":"disable");
			vty_out(vty, "portal wlanparameter      :%s\n", (1==portalconf.portal_srv[i].wlanparameter)?"enable":"disable");
			if (1 == portalconf.portal_srv[i].wlanparameter) {
				vty_out(vty, "portal deskey             :%s\n", portalconf.portal_srv[i].deskey );
			}
			vty_out(vty, "portal wlanuserfirsturl   :%s\n", 
						(1==portalconf.portal_srv[i].wlanuserfirsturl)?"enable":"disable");
			if(0 != strcmp(portalconf.portal_srv[i].url_suffix, ""))
				vty_out(vty, "portal url-suffix         :%s\n", portalconf.portal_srv[i].url_suffix);
			if(0 != strcmp(portalconf.portal_srv[i].secret, ""))
				vty_out(vty, "portal secret             :%s\n", portalconf.portal_srv[i].secret);
			vty_out(vty, "portal wlanapmac          :%s\n", (1 == portalconf.portal_srv[i].wlanapmac)?"enable":"disable");
			vty_out(vty, "portal wlanusermac        :%s\n", (1==portalconf.portal_srv[i].wlanusermac)?"enable":"disable");
			if (1 == portalconf.portal_srv[i].wlanusermac) {
				vty_out(vty, "portal wlanusermac deskey :%s\n", portalconf.portal_srv[i].wlanusermac_deskey);
			}
			if (WISPR_URL_HTTP==portalconf.portal_srv[i].wisprlogin){
				vty_out(vty, "portal wisprurl type      :%s\n","http");
			}else if(WISPR_URL_HTTPS==portalconf.portal_srv[i].wisprlogin){
				vty_out(vty, "portal wisprurl type      :%s\n","https");
			}else{
				vty_out(vty, "portal wisprurl type      :%s\n","disable");
			}
			if( 1 == portalconf.portal_srv[i].urlparam_add
				&& strlen(portalconf.portal_srv[i].save_urlparam_config) > 0) {
				vty_out(vty, "portal urlparam-add str   :%s\n", portalconf.portal_srv[i].save_urlparam_config);
			}
		}
	}
	return CMD_SUCCESS;
}

DEFUN(eag_del_portal_server_func,
	eag_del_portal_server_cmd,
	"delete portal-server (wlanid|vlanid|wtpid|interface) KEY-WORD",
	"delete\n"
	"delete portal-server configuration\n"
	"portal server index type wlanid\n"	
	"portal server index type vlanid\n"		
	"portal server index type wtpid\n"	
	"portal server index type l3interface name\n"
	"portal server index key-word\n"
)
{
	int ret = -1;
	PORTAL_KEY_TYPE key_type;
	unsigned long keyid = 0;
	char *keystr = "";

	if (strncmp(argv[0], "wlanid", strlen(argv[0])) == 0) {
		key_type = PORTAL_KEYTYPE_WLANID;
		keyid = atoi(argv[1]);
		if (keyid == 0 || keyid > 128){
			vty_out(vty, "%% wlan id is out of range 1~128\n");
			return CMD_SUCCESS;
		}
	}
	else if (strncmp(argv[0], "vlanid", strlen(argv[0])) == 0) {
		key_type = PORTAL_KEYTYPE_VLANID;
		keyid = atoi(argv[1]);
		if(keyid == 0 || keyid > 4096) {
			vty_out(vty, "%% vlan id is out of range 1~4096\n");
			return CMD_SUCCESS;
		}		
	}
	else if (strncmp(argv[0], "wtpid", strlen(argv[0])) == 0){
		keyid = atoi(argv[1]);
		key_type = PORTAL_KEYTYPE_WTPID;
	}
	else if (strncmp(argv[0], "interface", strlen(argv[0])) == 0) {
		key_type = PORTAL_KEYTYPE_INTF;
		keystr = (char *)argv[1];
	}
	else {
		vty_out(vty, "%% unknown index type %s\n", argv[0]);
		return CMD_SUCCESS;
	}

	EAG_DCLI_INIT_HANSI_INFO
#if 0		
	if(eag_ins_running_state(dcli_dbus_connection_curr, hansitype, insid)){
		vty_out(vty, "eag instance %d is running, please stop it first\n",insid);
		return CMD_FAILURE;
	}
#endif	
	ret = eag_del_portal_server( dcli_dbus_connection_curr,
										hansitype,insid,				
										key_type,
										keyid,
										keystr);

	if(EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_ERR_UNKNOWN == ret) {
		vty_out(vty, "%% unknown error\n");
	}
	else if (EAG_ERR_PORTAL_DEL_SRV_ERR_TYPE == ret) {
		vty_out(vty, "%% error index type\n");
	}
	else if (EAG_ERR_PORTAL_DEL_SRV_NOT_EXIST == ret) {
		vty_out(vty, "%% the del key is not exist\n");
	}
	else if (EAG_RETURN_OK != ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}
	

	return CMD_SUCCESS;
}

DEFUN(eag_del_portal_conf_essid_func,
	eag_del_portal_conf_essid_cmd,
	"delete portal-server essid .KEY-WORD",
	"delete\n"
	"delete portal-server configuration\n"
	"portal server index type essid\n"	
	"eg:delete portal-server essid test\n"
)
{
	int ret = -1;
	int i = 0;
	PORTAL_KEY_TYPE key_type = PORTAL_KEYTYPE_ESSID;
	unsigned long keyid = 0;
	char *keystr = "";
	char essid[MAX_PORTAL_SSID_LEN] = {0};

	for (i = 0; i < argc; i++) {
		if (0 != i) {
			strncat(essid, " ", 1);
		}
		if((strlen(essid) + strlen(argv[i])) > MAX_PORTAL_SSID_LEN - 1) {
			vty_out(vty, "%% essid length is too long\n");
			return CMD_SUCCESS;
		}
		strncat(essid, argv[i], strlen(argv[i]));
		keystr = essid;
	}

	EAG_DCLI_INIT_HANSI_INFO
#if 0		
	if(eag_ins_running_state(dcli_dbus_connection_curr,hansitype, insid)){
		vty_out(vty, "eag instance %d is running, please stop it first\n",insid);
		return CMD_FAILURE;
	}
#endif	
	ret = eag_del_portal_server(dcli_dbus_connection_curr,
										hansitype,insid,				
										key_type,
										keyid,
										keystr);

	if(EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_ERR_UNKNOWN == ret) {
		vty_out(vty, "%% unknown error\n");
	}
	else if (EAG_ERR_PORTAL_DEL_SRV_ERR_TYPE == ret) {
		vty_out(vty, "%% error index type\n");
	}
	else if (EAG_ERR_PORTAL_DEL_SRV_NOT_EXIST == ret) {
		vty_out(vty, "%% the del key is not exist\n");
	}
	else if (EAG_RETURN_OK != ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}	

	return CMD_SUCCESS;
}

#if 0
/* radius server */
#endif
DEFUN(eag_add_radius_server_func,
	eag_add_radius_server_cmd,
	"add radius-server DOMAIN auth A.B.C.D <1-65535> SECRET acct A.B.C.D <1-65535> SECRET",
	"add config\n"
	"add radius-server\n"
	"radius server domain\n"
	"radius server author\n"	
	"radius server author ip\n"		
	"radius server author port\n"	
	"radius server author secret\n"
	"radius server account\n"	
	"radius server account ip\n"		
	"radius server account port\n"	
	"radius server account secret\n"
)
{
	int ret;
	char *domain;
	unsigned long auth_ip=0;
	unsigned short auth_port=0;
	char *auth_secret="";
	unsigned long acct_ip=0;
	unsigned short acct_port=0;
	char *acct_secret="";
	
	unsigned long backup_auth_ip=0;
	unsigned short backup_auth_port=0;
	char *backup_auth_secret="";
	unsigned long backup_acct_ip=0;
	unsigned short backup_acct_port=0;
	char *backup_acct_secret="";	
	struct in_addr inaddr;
	

#if 0	
	if( 7 != argc && 13 != argc ){
		vty_out(vty,"error param!");
		return CMD_SUCCESS;
	}
#endif	
	domain = (char *)argv[0];
	ret = inet_aton(argv[1], &inaddr);
	if (!ret){
    	vty_out(vty, "%% invalid ip address\n");
		return CMD_WARNING;
    }
	auth_ip = ntohl(inaddr.s_addr);
	auth_port = atoi(argv[2]);
	auth_secret = (char *)argv[3];
	
	ret = inet_aton( argv[4],&inaddr);
	if (!ret){
    	vty_out(vty, "%% invalid ip address\n");
		return CMD_WARNING;
    }
	acct_ip = ntohl(inaddr.s_addr);
	acct_port = atoi(argv[5]);
	acct_secret = (char *)argv[6];
	
	if(13 == argc) {
		ret = inet_aton( argv[7],&inaddr);
		if (!ret) {
            vty_out(vty, "%% invalid ip address\n");
            return CMD_WARNING;
    	}
		backup_auth_ip = ntohl(inaddr.s_addr);
		backup_auth_port = atoi(argv[8]);
		backup_auth_secret = (char *)argv[9];
		
		ret = inet_aton( argv[10],&inaddr);
		if (!ret) {
            vty_out(vty, "%% invalid ip address\n");
            return CMD_WARNING;
    	}
		backup_acct_ip = ntohl(inaddr.s_addr);
		backup_acct_port = atoi(argv[11]);
		backup_acct_secret = (char *)argv[12];		
	}
	
	EAG_DCLI_INIT_HANSI_INFO
#if 0		
	if(eag_ins_running_state(dcli_dbus_connection_curr, hansitype, insid)){
		vty_out(vty, "eag instance %d is running, please stop it first\n",insid);
		return CMD_FAILURE;
	}	
#endif	
	ret = eag_add_radius( dcli_dbus_connection_curr, 
								hansitype,insid,
								domain,
								auth_ip,
								auth_port,
								auth_secret,
								acct_ip,
								acct_port,
								acct_secret,
								backup_auth_ip,
								backup_auth_port,
								backup_auth_secret,
								backup_acct_ip,
								backup_acct_port,
								backup_acct_secret );
	
	if( EAG_ERR_RADIUS_DOMAIN_LEN_ERR == ret ){
		vty_out(vty, "%% domain len error. should be %d!\n", 
						MAX_RADIUS_DOMAIN_LEN-1);
	}
	else if( EAG_ERR_RADIUS_PARAM_ERR == ret ){
		vty_out(vty, "%% ip or port error!\n") ;
	}
	else if( EAG_ERR_RADIUS_SECRET_LENTH_OUTOFF_SIZE == ret ){
		vty_out(vty, "%% secret len out size. should be %d!\n", 
						RADIUS_SECRETSIZE-1);
	}
	else if( EAG_ERR_RADIUS_DOAMIN_AREADY_EXIST == ret ){
		vty_out(vty, "%% domain already exist!\n") ;
	}
	else if( EAG_ERR_RADIUS_MAX_NUM_LIMITE == ret ){
		vty_out(vty, "%% max radius num limite:%d\n", MAX_RADIUS_SRV_NUM) ;
	}
	else if( EAG_ERR_DBUS_FAILED == ret ){
		vty_out(vty, "%% dbus error!\n");
	}
	else if( EAG_RETURN_OK != ret ){
		vty_out(vty, "%% unknown error: %d\n", ret);
	}

	return CMD_SUCCESS;
}

ALIAS(eag_add_radius_server_func,
	eag_add_radius_server_cmd_hasbackup,
	"add radius-server DOMAIN auth A.B.C.D <1-65535> SECRET acct A.B.C.D <1-65535> SECRET"\
		" backup-auth A.B.C.D <1-65535> SECRET backup-acct A.B.C.D <1-65535> SECRET",
	"add config\n"
	"add radius-server\n"
	"radius server domain\n"
	"radius server author\n"	
	"radius server author ip\n"		
	"radius server author port\n"	
	"radius server author secret\n"
	"radius server account\n"	
	"radius server account ip\n"		
	"radius server account port\n"	
	"radius server account secret\n"
	"radius server backup author\n"	
	"radius server backup author ip\n"		
	"radius server backup author port\n"	
	"radius server backup author secret\n"
	"radius server backup account\n"	
	"radius server backup account ip\n"		
	"radius server backup account port\n"	
	"radius server backup account secret\n"	
)


DEFUN(eag_modify_radius_server_func,
	eag_modify_radius_server_cmd,
	"modify radius-server DOMAIN auth A.B.C.D <1-65535> SECRET acct A.B.C.D <1-65535> SECRET",
	"modify config\n"
	"modify radius-server\n"
	"radius server domain\n"
	"radius server author\n"	
	"radius server author ip\n"		
	"radius server author port\n"	
	"radius server author secret\n"
	"radius server account\n"	
	"radius server account ip\n"		
	"radius server account port\n"	
	"radius server account secret\n"
)
{
	int ret;
	char *domain;
	unsigned long auth_ip=0;
	unsigned short auth_port=0;
	char *auth_secret="";
	unsigned long acct_ip=0;
	unsigned short acct_port=0;
	char *acct_secret="";
	
	unsigned long backup_auth_ip=0;
	unsigned short backup_auth_port=0;
	char *backup_auth_secret="";
	unsigned long backup_acct_ip=0;
	unsigned short backup_acct_port=0;
	char *backup_acct_secret="";	
	struct in_addr inaddr;
	

#if 0	
	if( 7 != argc && 13 != argc ){
		vty_out(vty,"error param!");
		return CMD_SUCCESS;
	}
#endif	
	domain = (char *)argv[0];
	ret = inet_aton( argv[1],&inaddr);
	if (!ret) {
            vty_out(vty, "%% invalid ip address\n");
            return CMD_WARNING;
    }
	auth_ip = ntohl(inaddr.s_addr);
	auth_port = atoi(argv[2]);
	auth_secret = (char *)argv[3];
	
	ret = inet_aton( argv[4],&inaddr);
	if (!ret) {
            vty_out(vty, "%% invalid ip address\n");
            return CMD_WARNING;
    }
	acct_ip = ntohl(inaddr.s_addr);
	acct_port = atoi(argv[5]);
	acct_secret = (char *)argv[6];
	
	if( 13 == argc ){
		ret = inet_aton( argv[7],&inaddr);
		if (!ret) {
            vty_out(vty, "%% invalid ip address\n");
            return CMD_WARNING;
    	}
		backup_auth_ip = ntohl(inaddr.s_addr);
		backup_auth_port = atoi(argv[8]);
		backup_auth_secret = (char *)argv[9];
		
		ret = inet_aton( argv[10],&inaddr);
		if (!ret) {
            vty_out(vty, "%% invalid ip address\n");
            return CMD_WARNING;
    	}
		backup_acct_ip = ntohl(inaddr.s_addr);
		backup_acct_port = atoi(argv[11]);
		backup_acct_secret = (char *)argv[12];		
	}
	
	EAG_DCLI_INIT_HANSI_INFO
#if 0		
	if(eag_ins_running_state(dcli_dbus_connection_curr, hansitype, insid)){
		vty_out(vty, "eag instance %d is running, please stop it first\n",insid);
		return CMD_FAILURE;
	}
#endif	
	ret = eag_modify_radius( dcli_dbus_connection_curr, 
								hansitype,insid,				
								domain,
								auth_ip,
								auth_port,
								auth_secret,
								acct_ip,
								acct_port,
								acct_secret,
								backup_auth_ip,
								backup_auth_port,
								backup_auth_secret,
								backup_acct_ip,
								backup_acct_port,
								backup_acct_secret );
	
	if(EAG_ERR_RADIUS_DOAMIN_NOT_EXIST == ret) {
		vty_out(vty, "%% domain not find\n") ;
	}
	else if( EAG_ERR_RADIUS_DOMAIN_LEN_ERR == ret ){
		vty_out(vty, "%% domain len error. should be %d!\n", 
						MAX_RADIUS_DOMAIN_LEN-1);
	}
	else if( EAG_ERR_RADIUS_PARAM_ERR == ret ){
		vty_out(vty, "%% ip or port error!\n") ;
	}
	else if( EAG_ERR_RADIUS_SECRET_LENTH_OUTOFF_SIZE == ret ){
		vty_out(vty, "%% secret len out size. should be %d!\n", 
						RADIUS_SECRETSIZE-1);
	}
	else if(EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if(EAG_RETURN_OK != ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}

	return CMD_SUCCESS;
}

ALIAS(eag_modify_radius_server_func,
	eag_modify_radius_server_cmd_hasbackup,
	"modify radius-server DOMAIN auth A.B.C.D <1-65535> SECRET acct A.B.C.D <1-65535> SECRET"\
		" backup-auth A.B.C.D <1-65535> SECRET backup-acct A.B.C.D <1-65535> SECRET",
	"modify config\n"
	"modify radius-server\n"
	"radius server domain\n"
	"radius server author\n"	
	"radius server author ip\n"		
	"radius server author port\n"	
	"radius server author secret\n"
	"radius server account\n"	
	"radius server account ip\n"		
	"radius server account port\n"	
	"radius server account secret\n"
	"radius server backup author\n"	
	"radius server backup author ip\n"		
	"radius server backup author port\n"	
	"radius server backup author secret\n"
	"radius server backup account\n"	
	"radius server backup account ip\n"		
	"radius server backup account port\n"	
	"radius server backup account secret\n"	
)

DEFUN(eag_del_radius_server_func,
	eag_del_radius_server_cmd,
	"delete radius-server DOMAIN",
	"delete\n"
	"del radius-server\n"
	"del radius-serve of domain\n"
)
{
	int ret;
	char *domain=NULL;
	struct radius_conf radiusconf;
	int i;


	domain = (char *)argv[0];

	memset( &radiusconf, 0, sizeof(struct radius_conf) );
	
	EAG_DCLI_INIT_HANSI_INFO
#if 0		
	if(eag_ins_running_state(dcli_dbus_connection_curr, hansitype, insid)){
		vty_out(vty, "eag instance %d is running, please stop it first\n",insid);
		return CMD_FAILURE;
	}
#endif	
	ret = eag_del_radius( dcli_dbus_connection_curr, 
								hansitype,insid, 
								domain );

	if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if( EAG_ERR_RADIUS_DOMAIN_LEN_ERR == ret ){
		vty_out(vty, "%% domain len error. should be %d!\n", 
						MAX_RADIUS_DOMAIN_LEN-1);
	}
	else if (EAG_ERR_RADIUS_DOAMIN_NOT_EXIST == ret) {
		vty_out(vty, "%% the del domain is not exist\n");
	}
	else if (EAG_RETURN_OK != ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}

	return CMD_SUCCESS;
}

DEFUN(eag_set_radius_remove_domainname_func,
	eag_set_radius_remove_domainname_cmd,
	"set radius DOMAIN remove-domain-name  (enable|disable)",
	"set parameter of radius policy, if 'DOMAIN' not exist, create it\n"
	"Radius policy\n"
	"Domain name, specify a item of radius policy\n"
	"Strip the user domain name when send EAG send package  RADIUS server\n"
	"Enable strip the user domain name when EAG send package RADIUS server\n"
	"Disable strip the user domain name when EAG send package RADIUS server\n"
)
{
	char domain_name[MAX_RADIUS_DOMAIN_LEN - 1] = {0};
	int remove_domain_switch= 0;
	int ret = -1;

	if (strlen(argv[0]) > MAX_RADIUS_DOMAIN_LEN-1){
		vty_out(vty, "error domain name format\n");
		return CMD_WARNING;
	}
	strcpy(domain_name, argv[0]);
	
	if (strncmp(argv[1], "enable", strlen(argv[1])) == 0) {
		remove_domain_switch = 1;
	} 
	else if (strncmp(argv[1], "disable", strlen(argv[1])) == 0) {
		remove_domain_switch = 0;
	}
	else {
		vty_out(vty,"%% bad command parameter\n");
		return CMD_WARNING;
	}

	EAG_DCLI_INIT_HANSI_INFO
	
	ret = eag_set_remove_domain_switch(dcli_dbus_connection_curr,
					hansitype, insid, 
					domain_name, remove_domain_switch);	

	if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if( EAG_ERR_RADIUS_DOMAIN_LEN_ERR == ret ){
		vty_out(vty, "%% domain len error. should be %d!\n", 
						MAX_RADIUS_DOMAIN_LEN-1);
	}
	else if (EAG_ERR_RADIUS_DOAMIN_NOT_EXIST == ret) {
		vty_out(vty, "%% radius domain %s is not exit\n", domain_name);
	}
	else if (EAG_RETURN_OK != ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}
	return CMD_SUCCESS; 
}

DEFUN(eag_set_radius_class_to_bandwidth_func,
	eag_set_radius_class_to_bandwidth_cmd,
	"set radius DOMAIN class-to-bandwidth  (enable|disable)",
	"set parameter of radius policy, if 'DOMAIN' not exist, create it\n"
	"Radius policy\n"
	"Domain name, specify a item of radius policy\n"
	"class to bandwidth\n"
	"Enable\n"
	"Disable\n"
)
{
	char domain_name[MAX_RADIUS_DOMAIN_LEN - 1] = {0};
	int class_to_bandwidth = 0;
	int ret = -1;

	if (strlen(argv[0]) > MAX_RADIUS_DOMAIN_LEN-1){
		vty_out(vty, "error domain name format\n");
		return CMD_WARNING;
	}
	strncpy(domain_name, argv[0], sizeof(domain_name)-1);
	
	if (strncmp(argv[1], "enable", strlen(argv[1])) == 0) {
		class_to_bandwidth = 1;
	} 
	else if (strncmp(argv[1], "disable", strlen(argv[1])) == 0) {
		class_to_bandwidth = 0;
	}
	else {
		vty_out(vty,"%% bad command parameter\n");
		return CMD_WARNING;
	}

	EAG_DCLI_INIT_HANSI_INFO
	
	ret = eag_set_class_to_bandwidth_switch(dcli_dbus_connection_curr,
					hansitype, insid, 
					domain_name, class_to_bandwidth);	

	if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if( EAG_ERR_RADIUS_DOMAIN_LEN_ERR == ret ){
		vty_out(vty, "%% domain len error. should be %d!\n", 
						MAX_RADIUS_DOMAIN_LEN-1);
	}
	else if (EAG_ERR_RADIUS_DOAMIN_NOT_EXIST == ret) {
		vty_out(vty, "%% radius domain %s is not exit\n", domain_name);
	}
	else if (EAG_RETURN_OK != ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}
	return CMD_SUCCESS; 
}

DEFUN(eag_show_radius_conf_func,
	eag_show_radius_conf_cmd,
	"show radius-server [DOMAIN]",
	"show config\n"
	"show radius-server \n"
	"show radius-server by domain\n"
)
{
	int ret;
	char *domain=NULL;
	struct radius_conf radiusconf;
	int i;

	if (1 == argc) {
		domain = (char *)argv[0];
	} else {
		domain = "";
	}

	memset( &radiusconf, 0, sizeof(struct radius_conf) );
	
	EAG_DCLI_INIT_HANSI_INFO
		
	ret = eag_get_radius_conf( dcli_dbus_connection_curr, 
								hansitype,insid, 
								domain,
								&radiusconf );

	if( EAG_ERR_DBUS_FAILED == ret ){
		vty_out(vty, "%% dbus error\n");
	}
	else if( EAG_ERR_RADIUS_DOMAIN_LEN_ERR == ret ){
		vty_out(vty, "%% domain len error. should be %d!\n", 
						MAX_RADIUS_DOMAIN_LEN-1);
	}
	else if( EAG_ERR_RADIUS_DOAMIN_NOT_EXIST == ret ){
		vty_out(vty, "%% domain %s not find\n", domain);
	}
	else if( EAG_RETURN_OK != ret ){
		vty_out(vty, "%% unknown error: %d\n", ret);
	}
	else{
		for( i=0; i<radiusconf.current_num; i++ ){
			char ipstr[32];
			vty_out( vty, "=====================================\n" );
			vty_out( vty, "radius domain            :%s\n", radiusconf.radius_srv[i].domain);
			vty_out( vty, "radius remove domain     :%s\n", 
					1 == radiusconf.radius_srv[i].remove_domain_name?"enable":"disable");
			vty_out( vty, "radius class-to-bandwidth:%s\n", 
					1 == radiusconf.radius_srv[i].class_to_bandwidth?"enable":"disable");
			ip2str( radiusconf.radius_srv[i].auth_ip, ipstr,sizeof(ipstr));
			vty_out( vty, "radius auth ip           :%s\n", ipstr);
			vty_out( vty, "radius auth port         :%u\n",  radiusconf.radius_srv[i].auth_port);
			vty_out( vty, "radius auth sercet       :%s\n", radiusconf.radius_srv[i].auth_secret);
			ip2str( radiusconf.radius_srv[i].acct_ip, ipstr,sizeof(ipstr));
			vty_out( vty, "radius acct ip           :%s\n", ipstr );
			vty_out( vty, "radius acct port         :%u\n", radiusconf.radius_srv[i].acct_port );
			vty_out( vty, "radius acct sercet       :%s\n", radiusconf.radius_srv[i].acct_secret );
			ip2str( radiusconf.radius_srv[i].backup_auth_ip, ipstr,sizeof(ipstr));
			vty_out( vty, "radius backup auth ip    :%s\n", ipstr );
			vty_out( vty, "radius backup auth port  :%u\n", radiusconf.radius_srv[i].backup_auth_port );
			vty_out( vty, "radius backup auth sercet:%s\n", radiusconf.radius_srv[i].backup_auth_secret );
			ip2str( radiusconf.radius_srv[i].backup_acct_ip, ipstr,sizeof(ipstr));
			vty_out( vty, "radius backup acct ip    :%s\n", ipstr );
			vty_out( vty, "radius backup acct port  :%u\n", radiusconf.radius_srv[i].backup_acct_port );
			vty_out( vty, "radius backup acct sercet:%s\n", radiusconf.radius_srv[i].backup_acct_secret );		
		}
	}

	return CMD_SUCCESS;
}

#if 0
/* log, debug */
#endif

DEFUN(show_eag_relative_time_func,
	show_eag_relative_time_cmd,
	"show eag-relative-time",
	SHOW_STR
	"show eag relative time\n"
)
{
	int ret = -1;
	long timenow = 0;
	EAG_DCLI_INIT_HANSI_INFO
	
	ret = eag_get_relative_time(dcli_dbus_connection_curr, hansitype, insid, &timenow);
	if (EAG_RETURN_OK == ret) {
		vty_out(vty,"eag relative time :%lu\n", timenow);			
	}
	else if( EAG_ERR_INPUT_PARAM_ERR == ret ){
		vty_out(vty, "%% input prarm error\n");
	}
	else if( EAG_ERR_DBUS_FAILED == ret ){
		vty_out(vty, "%% dbus error\n" );	
	}
	else{
		vty_out(vty, "%% unknown error: %d\n", ret);
	}

	return CMD_SUCCESS;
}

DEFUN(eag_add_debug_filter_func,
	eag_add_debug_filter_cmd,
	"add debug-filter FILTER",
	"add config\n"
	"add debug-filter string\n"
	"add debug-filter string\n"
)
{
	int ret;

	EAG_DCLI_INIT_HANSI_INFO
		
	ret = eag_add_debug_filter(dcli_dbus_connection_curr, hansitype, insid, (char *)argv[0]);

	if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_RETURN_OK != ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}	

	return CMD_SUCCESS;
}

DEFUN(eag_del_debug_filter_func,
	eag_del_debug_filter_cmd,
	"del debug-filter FILTER",
	"del config\n"
	"del debug-filter string\n"
	"del debug-filter string\n"
)
{
	int ret;

	EAG_DCLI_INIT_HANSI_INFO
		
	ret = eag_del_debug_filter(dcli_dbus_connection_curr,hansitype,insid, (char *)argv[0]);

	if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_RETURN_OK != ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}	

	return CMD_SUCCESS;
}

DEFUN(eag_log_all_appconn_func,
	eag_log_all_appconn_cmd,
	"log all appconn",
	"do log\n"
	"log all session\n"
	"log all session\n"
)
{
	int ret;
	
	EAG_DCLI_INIT_HANSI_INFO
		
	ret = eag_log_all_appconn(dcli_dbus_connection_curr,hansitype,insid);

	if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_RETURN_OK != ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}	

	return CMD_SUCCESS;
}

DEFUN(eag_log_all_portalsess_func,
	eag_log_all_portalsess_cmd,
	"log all portalsess",
	"do log\n"
	"log all-portalsess\n"
	"log all-portalsess\n"
)
{
	#if 1
	int ret = -1;
	
	EAG_DCLI_INIT_HANSI_INFO
		
	ret = eag_log_all_portalsess(dcli_dbus_connection_curr, hansitype, insid);

	if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_RETURN_OK != ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}	

	return CMD_SUCCESS;
	#endif
}

DEFUN(eag_log_all_sockradius_func,
	eag_log_all_sockradius_cmd,
	"log all sockradius",
	"do log\n"
	"log all radius\n"
)
{
	#if 1
	int ret = -1;
	
	EAG_DCLI_INIT_HANSI_INFO
		
	ret = eag_log_all_sockradius( dcli_dbus_connection_curr, hansitype, insid);

	if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_RETURN_OK != ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}	

	return CMD_SUCCESS;
	#endif
}

DEFUN(eag_log_all_redirconn_func,
	eag_log_all_redirconn_cmd,
	"log all redirconn",
	"do log\n"
	"log all redir\n"
)
{
	#if 1
	int ret;
	
	EAG_DCLI_INIT_HANSI_INFO
		
	ret = eag_log_all_redirconn(dcli_dbus_connection_curr, hansitype, insid);

	if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_RETURN_OK != ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}	

	return CMD_SUCCESS;
	#endif
}

DEFUN(eag_log_all_thread_func,
	eag_log_all_thread_cmd,
	"log all thread",
	"do log\n"
	"log all thread\n"
)
{
	int ret;
	
	EAG_DCLI_INIT_HANSI_INFO
		
	ret = eag_log_all_thread(dcli_dbus_connection_curr, hansitype, insid);

	if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_RETURN_OK != ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}	

	return CMD_SUCCESS;
}

DEFUN(eag_log_all_blkmem_func,
	eag_log_all_blkmem_cmd,
	"log all blkmem",
	"do log\n"
	"log all blkmem\n"
)
{
	int ret;
	
	EAG_DCLI_INIT_HANSI_INFO
		
	ret = eag_log_all_blkmem( dcli_dbus_connection_curr,hansitype,insid);

	if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_RETURN_OK != ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}	

	return CMD_SUCCESS;
}

DEFUN(eag_log_all_mac_preauth_func,
	eag_log_all_mac_preauth_cmd,
	"log all macauth-preauth",
	"do log\n"
	"log all macauth-preauth\n"
)
{
	int ret = 0;
	
	EAG_DCLI_INIT_HANSI_INFO
		
	ret = eag_log_all_mac_preauth(dcli_dbus_connection_curr, hansitype, insid);

	if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_RETURN_OK != ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}	

	return CMD_SUCCESS;
}

DEFUN(eag_set_rdc_client_log_func,
	eag_set_rdc_client_log_cmd,
	"log eag-rdc-client (on|off)",
	"do log\n"
	"log eag rdc client packet\n"
	"log on\n"
	"log off\n"
)
{
	int ret = 0;
	int status = 0;
		
	if (strncmp(argv[0], "on", strlen(argv[0])) == 0) {
		status = 1;
	} 
	else if (strncmp(argv[0], "off", strlen(argv[0])) == 0) {
		status = 0;
	}
#if 0	
	else {
		vty_out(vty,"%% bad command parameter\n");
		return CMD_WARNING;
	}
#endif
	EAG_DCLI_INIT_HANSI_INFO

	ret = eag_set_rdc_client_log( dcli_dbus_connection_curr,
									hansitype, insid,
									status);

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

DEFUN(eag_set_pdc_client_log_func,
	eag_set_pdc_client_log_cmd,
	"log eag-pdc-client (on|off)",
	"do log\n"
	"log eag pdc client packet\n"
	"log on\n"
	"log off\n"
)
{
	int ret = 0;
	int status = 0;
		
	if (strncmp(argv[0], "on", strlen(argv[0])) == 0) {
		status = 1;
	} 
	else if (strncmp(argv[0], "off", strlen(argv[0])) == 0) {
		status = 0;
	}
#if 0	
	else {
		vty_out(vty,"%% bad command parameter\n");
		return CMD_WARNING;
	}
#endif
	EAG_DCLI_INIT_HANSI_INFO

	ret = eag_set_pdc_client_log( dcli_dbus_connection_curr,
									hansitype, insid,
									status);

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

#if 0
/*captive-interface,white-list, black-list*/
#endif
DEFUN(add_captive_portal_intfs_func,
	add_captive_portal_intfs_cmd,	
	"add captive-interface INTERFACE",
	SHOW_STR
	"add captive-portal interface\n"
	"add captive-portal interface\n"
	"the interface name\n"
)
{
	int ret = -1;
	char *intfs = (char *)argv[0];

#if 0
	if (!if_nametoindex(intfs)) {
		vty_out(vty, "%% No such interface %s\n", intfs);
		return CMD_WARNING;
	}
	if (eag_check_interface_addr(intfs)) {
		vty_out(vty, "%% Interface without setting the IP address,"\
				" please set IP address first.\n");
		return CMD_WARNING;
	}
#endif
	EAG_DCLI_INIT_HANSI_INFO
#if 0
	if(eag_ins_running_state(dcli_dbus_connection_curr, hansitype, insid)){
		vty_out(vty, "eag instance %d is running, please stop it first\n",insid);
		return CMD_FAILURE;
	}
#endif
	ret = eag_add_captive_intf( dcli_dbus_connection_curr,
						hansitype,insid, intfs );
	
	if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty,"%% dbus error\n");
	}
	else if (EAG_ERR_INPUT_PARAM_ERR == ret) {
		vty_out(vty,"%% input prarm error\n");
	}
	else if (EAG_ERR_CAPTIVE_INTERFACE_NUM_LIMIT == ret) {
		vty_out(vty,"%% max captive num limite:%d\n", CP_MAX_INTERFACE_NUM);
	}
	else if (EAG_ERR_CAPTIVE_INTERFACE_AREADY_USED == ret) {
		vty_out(vty, "%% captive interface is already exist\n");
	}
	else if (EAG_ERR_CAPTIVE_INTERFACE_NOT_EXIST == ret) {
		vty_out(vty, "%% captive interface without setting the IP address\n");
	}
	else if (EAG_RETURN_OK != ret){
		vty_out(vty,"%% unknown error: %d\n", ret);
	}
	
	return CMD_SUCCESS;
}

DEFUN(del_captive_portal_intfs_func,
	del_captive_portal_intfs_cmd,	
	"del captive-interface INTERFACE",
	"del params!\n"
	"del captive-portal interface\n"
	"del captive-portal interface\n"
	"the interface name\n"
)
{
	int ret = -1;
	char *intfs = (char *)argv[0];
	
	EAG_DCLI_INIT_HANSI_INFO
#if 0
	if(eag_ins_running_state(dcli_dbus_connection_curr, hansitype, insid)){
		vty_out(vty, "eag instance %d is running, please stop it first\n",insid);
		return CMD_FAILURE;
	}
#endif
	ret = eag_del_captive_intf( dcli_dbus_connection_curr,
						hansitype,insid, intfs );
	
	if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty,"%% dbus error\n");
	}
	else if (EAG_ERR_INPUT_PARAM_ERR == ret) {
		vty_out(vty,"%% input prarm error\n");
	}
	else if (EAG_ERR_CAPTIVE_INTERFACE_NOT_EXIST == ret) {
		vty_out(vty, "%% captive interface is not exist\n");
	}
	else if (EAG_RETURN_OK != ret){
		vty_out(vty,"%% unknown error: %d\n", ret);
	}
	
	return CMD_SUCCESS;
}


DEFUN(show_captive_portal_intfs_func,
	show_captive_portal_intfs_cmd,
	"show captive-interface",
	"show config\n"
	"show captive-interface\n"
)
{
	int ret = -1, i = 0;
	eag_captive_intfs captive_intfs;

	EAG_DCLI_INIT_HANSI_INFO
		
	memset( &captive_intfs, 0, sizeof(captive_intfs) );	
	
	ret = eag_get_captive_intfs( dcli_dbus_connection_curr,
						hansitype,insid, &captive_intfs );
	
	if( EAG_RETURN_OK == ret ){		
		if( captive_intfs.curr_ifnum > 0 ){
			vty_out(vty, "================================================\n");
			vty_out(vty, "%-10s %s\n","Index","Interface Name");	
			for( i=0; i < captive_intfs.curr_ifnum; i++ ){						
				vty_out(vty, "%2d %14s\n",i+1, captive_intfs.cpif[i]);
			}
			vty_out(vty, "================================================\n");	
		}
		
	}
	else if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}
	return CMD_SUCCESS;
}

DEFUN(show_captive_portal_white_list_func,
	show_captive_portal_white_list_cmd,
	"show white-list (ip|domain|all)",
	SHOW_STR
	"show white list\n" 
	"only the white-list with the format of iprange:portset would be listed\n"
	"only the white-list with the format of domain-url would be listed\n\n"
	"all the white-list would be listed, both iprange-portset and domain-url\n"
)
{
	int ret = -1;
	int i = 0;
	int type_tmp = -1;

	struct bw_rules white;
	EAG_DCLI_INIT_HANSI_INFO
	
	memset( &white, 0, sizeof(white) );
	char *type = NULL;
	ret = eag_show_white_list( dcli_dbus_connection_curr,
						hansitype,insid, &white);
	if( EAG_RETURN_OK == ret ){	

		if( white.curr_num > 0 ) {
			char ipbegin[32] = {0};
			char ipend[32] = {0};
			char ipv6begin[48] = {0};
			char ipv6end[48] = {0};
			char eagins_ip[32] = {0};
			vty_out(vty, "================================================\n");
			vty_out(vty, "%-7s %-20s %-10s\n","Mode","IPx-range/Domain","Interface");
			for( i=0; i < white.curr_num; i++ ){
				if(white.rule[i].type == RULE_IPADDR) {
					type = "IP";
					ip2str( white.rule[i].key.ip.ipbegin, ipbegin,sizeof(ipbegin));
					ip2str( white.rule[i].key.ip.ipend, ipend,sizeof(ipend));
				} else if (white.rule[i].type == RULE_DOMAIN 
					|| white.rule[i].type == RULE_IPV6DOMAIN) {
					type = "Domain";
				} else if(white.rule[i].type == RULE_IPV6ADDR) {
					type = "IPV6";
					ipv6tostr( &(white.rule[i].key.ipv6.ipv6begin), ipv6begin,sizeof(ipv6begin));
					ipv6tostr( &(white.rule[i].key.ipv6.ipv6end), ipv6end,sizeof(ipv6end));
				}

				type_tmp = white.rule[i].type;
				if( ((RULE_TYPE)type_tmp == RULE_IPADDR) && ((strcmp(argv[0],"all") == 0)||((strcmp(argv[0],"ip") == 0))) )
				{
					if(strcmp(ipend, "0.0.0.0") == 0)
					{
						vty_out(vty, "%-7s %s:%-12s %s\n",type,ipbegin,white.rule[i].key.ip.ports, 
								white.rule[i].intf);
					}
					else
					{
						vty_out(vty, "%-7s %s-%s:%-3s %s\n",type,ipbegin,ipend,white.rule[i].key.ip.ports, 
									white.rule[i].intf);
					}
					
				} 
				else if( ((RULE_TYPE)type_tmp == RULE_IPV6ADDR) && ((strcmp(argv[0],"all") == 0)||((strcmp(argv[0],"ip") == 0))) )
				{
					if(strcmp(ipv6end, "::") == 0)
					{
						vty_out(vty, "%-7s %s:%-12s %s\n",type,ipv6begin,white.rule[i].key.ipv6.ports, 
								white.rule[i].intf);
					}
					else
					{
						vty_out(vty, "%-7s %s-%s:%-3s %s\n",type,ipv6begin,ipv6end,white.rule[i].key.ipv6.ports, 
									white.rule[i].intf);
					}
					
				}
				else if( ((RULE_TYPE)type_tmp == RULE_DOMAIN) && ((strcmp(argv[0],"all") == 0)||((strcmp(argv[0],"domain") == 0))) )
				{
					vty_out(vty, "%-7s %-20s %s\n",type, white.rule[i].key.domain.name, 	white.rule[i].intf);
				}
				
			}
			vty_out(vty, "================================================\n");
		}
		
	}
	else if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}

	return CMD_SUCCESS;
}

DEFUN(show_captive_portal_black_list_func,
	show_captive_portal_black_list_cmd,
	"show black-list (ip|domain|all)",
	SHOW_STR
	"show black list\n" 
	"only the black-list with the format of iprange:portset would be listed\n"
	"only the black-list with the format of domain-url would be listed\n\n"
	"all theblack-list would be listed, both iprange-portset and domain-url\n"
)
{
	int ret = -1;
	int i = 0, type_tmp = -1;

	struct bw_rules black;
	EAG_DCLI_INIT_HANSI_INFO
	
	memset( &black, 0, sizeof(black) );
	char *type = (char *)argv[0];
	ret = eag_show_black_list( dcli_dbus_connection_curr,
						hansitype,insid, &black);
	if( EAG_RETURN_OK == ret ){
		
		if( black.curr_num > 0 ){
			char ipbegin[32] = {0};
			char ipend[32] = {0};
			char ipv6begin[48] = {0};
			char ipv6end[48] = {0};
			char eagins_ip[32] = {0};
			vty_out(vty, "================================================\n");
			vty_out(vty, "%-7s %-20s %-10s\n","Mode","IP-range/Domain","Interface");
			for( i=0; i < black.curr_num; i++ ){
				if(black.rule[i].type == RULE_IPADDR) {
					type = "IP";
					ip2str( black.rule[i].key.ip.ipbegin, ipbegin,sizeof(ipbegin));
					ip2str( black.rule[i].key.ip.ipend, ipend,sizeof(ipend));
				} else if (black.rule[i].type == RULE_DOMAIN 
					|| black.rule[i].type == RULE_IPV6DOMAIN) {
					type = "Domain";
				} else if(black.rule[i].type == RULE_IPV6ADDR) {
					type = "IPV6";
					ipv6tostr( &(black.rule[i].key.ipv6.ipv6begin), ipv6begin,sizeof(ipv6begin));
					ipv6tostr( &(black.rule[i].key.ipv6.ipv6end), ipv6end,sizeof(ipv6end));
				}

				type_tmp = black.rule[i].type;
				if(((RULE_TYPE)type_tmp == RULE_IPADDR) && ((strcmp(argv[0],"all") == 0)||((strcmp(argv[0],"ip") == 0))) )
				{
					if(strcmp(ipend, "0.0.0.0") == 0)
					{
						vty_out(vty, "%-7s %s:%-12s %s\n", type, ipbegin, black.rule[i].key.ip.ports, black.rule[i].intf);
					}
					else
					{
						vty_out(vty, "%-7s %s-%s:%-3s %s\n", type, ipbegin, ipend, black.rule[i].key.ip.ports, black.rule[i].intf);
					}
				}
				else if( ((RULE_TYPE)type_tmp == RULE_IPV6ADDR) && ((strcmp(argv[0],"all") == 0)||((strcmp(argv[0],"ip") == 0))) )
				{
					if(strcmp(ipv6end, "::") == 0)
					{
						vty_out(vty, "%-7s %s:%-12s %s\n",type,ipv6begin,black.rule[i].key.ipv6.ports, 
								black.rule[i].intf);
					}
					else
					{
						vty_out(vty, "%-7s %s-%s:%-3s %s\n",type,ipv6begin,ipv6end,black.rule[i].key.ipv6.ports, 
									black.rule[i].intf);
					}
					
				}
				else if(((RULE_TYPE)type_tmp == RULE_DOMAIN) && ((strcmp(argv[0],"all") == 0)||((strcmp(argv[0],"domain") == 0))) )
				{
					vty_out(vty, "%-7s %-20s %s\n",type, black.rule[i].key.domain.name, black.rule[i].intf);
				}
				
			}
			vty_out(vty, "================================================\n");
		}
	}
	else if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}
	
	return CMD_SUCCESS;
}

DEFUN(conf_captive_portal_white_list_with_ip_func,
	conf_captive_portal_white_list_with_ip_cmd,	
	"(add|del) white-list ip IPRANGE[:PORTSET] [INTFS]",
	"add\n"
	"delete\n"
	"add or delete white list\n"
	"add or delete white list by ip format\n"
	"specifys that white list is described with iprange:postset format\n"
	"ip range and port set to be applied to the white list, with format A.B.C.D[-A.B.C.D][:(all|PORT[,PORT]...)]\n"
)
{
	
	int ret = -1;
	RULE_TYPE type = RULE_IPADDR;
	iprange_portset_t item;
	char intfs[50];
	EAG_DCLI_INIT_HANSI_INFO
#if 0		
	if(eag_ins_running_state(dcli_dbus_connection_curr, hansitype, insid)){
		vty_out(vty, "eag instance %d is running, please stop it first\n",insid);
		return CMD_FAILURE;
	}
#endif	
	memset(&intfs, 0, sizeof(intfs));
	memset(&item, 0, sizeof(item));
	if(3 == argc) {
		strncpy(intfs, argv[2], sizeof(intfs)-1);
		if (!if_nametoindex(intfs)) {
			vty_out(vty, "%% No such interface %s\n", intfs);
			return CMD_WARNING;
		}	
	}
	
	ret = parse_iprange_portset(argv[1], &item);//test_L
	if (0 != ret){
		vty_out(vty, "%% error iprange-portset format\n");
		return CMD_FAILURE;
	}
	
	if(0 == strcmp(argv[0], "add")) {
		ret = eag_conf_captive_list( dcli_dbus_connection_curr,
							hansitype,insid, type, item.iprange, item.portset, "", intfs, CP_ADD_LIST, CP_WHITE_LIST);
	}
	else if(0 == strcmp(argv[0], "del")) {
		ret = eag_conf_captive_list( dcli_dbus_connection_curr,
							hansitype,insid, type, item.iprange, item.portset, "", intfs, CP_DEL_LIST, CP_WHITE_LIST);
	}
	
	if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if(EAG_ERR_CAPTIVE_RULE_AREADY_IN_WHITE == ret) {
		vty_out(vty, "%% This rule aready in white-list\n");
	}
	else if(EAG_ERR_CAPTIVE_RULE_NOT_IN_WHITE == ret) {
		vty_out(vty, "%% This rule not in white-list\n");
	}
	else if(EAG_ERR_CAPTIVE_WHITE_LIST_NUM_LIMITE == ret) {
		vty_out(vty, "%% max white-list num limite:%d\n", MAX_BW_RULES_NUM);
	}
	else if (EAG_RETURN_OK != ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}
	
	return CMD_SUCCESS;
}

DEFUN(conf_captive_portal_black_list_with_ip_func,
	conf_captive_portal_black_list_with_ip_cmd,	
	"(add|del) black-list ip IPRANGE[:PORTSET] [INTFS]",
	"add\n"
	"delete\n"
	"add or delete black list\n"
	"add or delete black list by ip format\n"
	"specifys that black list is described with iprange:postset format\n"
	"ip range and port set to be applied to the black list, with format A.B.C.D[-A.B.C.D][:(all|PORT[,PORT]...)]\n"
)
{
	int ret = -1;
	RULE_TYPE type = RULE_IPADDR;
	iprange_portset_t item;
	char intfs[50];
	EAG_DCLI_INIT_HANSI_INFO
#if 0
	if(eag_ins_running_state(dcli_dbus_connection_curr, hansitype, insid)){
		vty_out(vty, "eag instance %d is running, please stop it first\n",insid);
		return CMD_FAILURE;
	}
#endif	
	memset(&intfs, 0, sizeof(intfs));
	memset(&item, 0, sizeof(item));
	if(3 == argc) {
		strncpy(intfs, argv[2], sizeof(intfs)-1);
		if (!if_nametoindex(intfs)) {
			vty_out(vty, "%% No such interface %s\n", intfs);
			return CMD_WARNING;
		}
	}
	ret = parse_iprange_portset(argv[1], &item);//test_L
	if (0 != ret){
		vty_out(vty, "%% error iprange-portset format\n");
		return CMD_FAILURE;
	}
	
	if(0 == strcmp(argv[0], "add")) {
		ret = eag_conf_captive_list( dcli_dbus_connection_curr,	hansitype,insid,
											type, item.iprange, item.portset, "", intfs, CP_ADD_LIST, CP_BLACK_LIST);
	}
	else if(0 == strcmp(argv[0], "del")) {
		ret = eag_conf_captive_list( dcli_dbus_connection_curr,	hansitype,insid,
											type, item.iprange, item.portset, "", intfs, CP_DEL_LIST, CP_BLACK_LIST);
	}
	
	if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if ( EAG_ERR_CAPTIVE_RULE_AREADY_IN_BLACK == ret ){
		vty_out(vty, "%% This rule aready in black-list\n");
	}
	else if( EAG_ERR_CAPTIVE_RULE_NOT_IN_BLACK == ret ){
		vty_out(vty, "%% This rule not in black-list\n");
	}
	else if(EAG_ERR_CAPTIVE_BLACK_LIST_NUM_LIMITE == ret) {
		vty_out(vty, "%% max black-list num limite:%d\n", MAX_BW_RULES_NUM);
	}
	else if( EAG_RETURN_OK != ret ){
		vty_out(vty, "%% unknown error: %d\n", ret);
	}
	
	return CMD_SUCCESS;
}

DEFUN(conf_captive_portal_white_list_with_ipv6_func,
	conf_captive_portal_white_list_with_ipv6_cmd,	
	"(add|del) white-list ipv6 IPRANGE[:PORTSET] [INTFS]",
	"add\n"
	"delete\n"
	"add or delete white list\n"
	"add or delete white list by ipv6 format\n"
	"specifys that white list is described with ipv6range:postset format\n"
	"ipv6 range and port set to be applied to the white list, with format A::B[-A::B][:(all|PORT[,PORT]...)]\n"
)
{
	
	int ret = -1;
	RULE_TYPE type = RULE_IPV6ADDR;
	ipv6range_portset_t item;
	char intfs[50];
	EAG_DCLI_INIT_HANSI_INFO
#if 0		
	if(eag_ins_running_state(dcli_dbus_connection_curr, hansitype, insid)){
		vty_out(vty, "eag instance %d is running, please stop it first\n",insid);
		return CMD_FAILURE;
	}
#endif	
	memset(&intfs, 0, sizeof(intfs));
	memset(&item, 0, sizeof(item));
	if(3 == argc) {
		strncpy(intfs, argv[2], sizeof(intfs)-1);
		if (!if_nametoindex(intfs)) {
			vty_out(vty, "%% No such interface %s\n", intfs);
			return CMD_WARNING;
		}	
	}
	
	ret = parse_ipv6range_portset(argv[1], &item);//test_L
	if (0 != ret){
		vty_out(vty, "%% error iprange-portset format\n");
		return CMD_FAILURE;
	}
	
	if(0 == strcmp(argv[0], "add")) {
		ret = eag_conf_captive_ipv6_list( dcli_dbus_connection_curr,
							hansitype,insid, type, item.ipv6range, item.portset, "", intfs, CP_ADD_LIST, CP_WHITE_LIST);
	}
	else if(0 == strcmp(argv[0], "del")) {
		ret = eag_conf_captive_ipv6_list( dcli_dbus_connection_curr,
							hansitype,insid, type, item.ipv6range, item.portset, "", intfs, CP_DEL_LIST, CP_WHITE_LIST);
	}
	
	if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if(EAG_ERR_CAPTIVE_RULE_AREADY_IN_WHITE == ret) {
		vty_out(vty, "%% This rule aready in white-list\n");
	}
	else if(EAG_ERR_CAPTIVE_RULE_NOT_IN_WHITE == ret) {
		vty_out(vty, "%% This rule not in white-list\n");
	}
	else if(EAG_ERR_CAPTIVE_WHITE_LIST_NUM_LIMITE == ret) {
		vty_out(vty, "%% max white-list num limite:%d\n", MAX_BW_RULES_NUM);
	}
	else if (EAG_RETURN_OK != ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}
	
	return CMD_SUCCESS;
}

DEFUN(conf_captive_portal_black_list_with_ipv6_func,
	conf_captive_portal_black_list_with_ipv6_cmd,	
	"(add|del) black-list ipv6 IPRANGE[:PORTSET] [INTFS]",
	"add\n"
	"delete\n"
	"add or delete black list\n"
	"add or delete black list by ipv6 format\n"
	"specifys that black list is described with iprange:postset format\n"
	"ipv6 range and port set to be applied to the black list, with format A::B[-A::B][:(all|PORT[,PORT]...)]\n"
)
{
	int ret = -1;
	RULE_TYPE type = RULE_IPV6ADDR;
	ipv6range_portset_t item;
	char intfs[50];
	EAG_DCLI_INIT_HANSI_INFO
#if 0
	if(eag_ins_running_state(dcli_dbus_connection_curr, hansitype, insid)){
		vty_out(vty, "eag instance %d is running, please stop it first\n",insid);
		return CMD_FAILURE;
	}
#endif	
	memset(&intfs, 0, sizeof(intfs));
	memset(&item, 0, sizeof(item));
	if(3 == argc) {
		strncpy(intfs, argv[2], sizeof(intfs)-1);
		if (!if_nametoindex(intfs)) {
			vty_out(vty, "%% No such interface %s\n", intfs);
			return CMD_WARNING;
		}
	}
	ret = parse_ipv6range_portset(argv[1], &item);//test_L
	if (0 != ret){
		vty_out(vty, "%% error iprange-portset format\n");
		return CMD_FAILURE;
	}
	
	if(0 == strcmp(argv[0], "add")) {
		ret = eag_conf_captive_ipv6_list( dcli_dbus_connection_curr,	hansitype,insid,
											type, item.ipv6range, item.portset, "", intfs, CP_ADD_LIST, CP_BLACK_LIST);
	}
	else if(0 == strcmp(argv[0], "del")) {
		ret = eag_conf_captive_ipv6_list( dcli_dbus_connection_curr,	hansitype,insid,
											type, item.ipv6range, item.portset, "", intfs, CP_DEL_LIST, CP_BLACK_LIST);
	}
	
	if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if ( EAG_ERR_CAPTIVE_RULE_AREADY_IN_BLACK == ret ){
		vty_out(vty, "%% This rule aready in black-list\n");
	}
	else if( EAG_ERR_CAPTIVE_RULE_NOT_IN_BLACK == ret ){
		vty_out(vty, "%% This rule not in black-list\n");
	}
	else if(EAG_ERR_CAPTIVE_BLACK_LIST_NUM_LIMITE == ret) {
		vty_out(vty, "%% max black-list num limite:%d\n", MAX_BW_RULES_NUM);
	}
	else if( EAG_RETURN_OK != ret ){
		vty_out(vty, "%% unknown error: %d\n", ret);
	}
	
	return CMD_SUCCESS;
}

DEFUN(conf_captive_portal_white_list_with_domain_func,
	conf_captive_portal_white_list_with_domain_cmd,
	"(add|del) white-list domain DOMAIN-URL [INTFS]",
	"add\n"
	"delete\n"
	"add or delete white list\n"
	"add or delete white list by domain format\n"
	"specifys that white list is described with domain-url format\n"
	"domain url to be applied to the white list\n"
)
{
	int ret = -1;
	RULE_TYPE type = RULE_DOMAIN;
	char intfs[50];
	char domain[256];
	domain_pt domain_conf;
	domain_ct domain_ctr;
	int i = 0;
	int slotid = HostSlotId;
	
	EAG_DCLI_INIT_HANSI_INFO
#if 0		
	if(eag_ins_running_state(dcli_dbus_connection_curr, hansitype, insid)){
		vty_out(vty, "eag instance %d is running, please stop it first\n",insid);
		return CMD_FAILURE;
	}
#endif	
	memset(&intfs, 0, sizeof(intfs));
	memset(&domain, 0, sizeof(domain));
	if(3 == argc) {
		strncpy(intfs, argv[2], sizeof(intfs)-1);
		if (!if_nametoindex(intfs)) {
			vty_out(vty, "%% No such interface %s\n", intfs);
			return CMD_WARNING;
		}
	}
	if (0 == is_domain(argv[1])) {
		vty_out(vty, "%% this domain can not parse, please check it\n");
		return CMD_SUCCESS;
	}
	
	strncpy(domain, argv[1], sizeof(domain)-1);
	
	if(0 == strcmp(argv[0], "add"))
	{
		ret = conf_drp_get_dbus_connect_params(&slotid);
		if (ret < 0){
			vty_out(vty, "eag get drp connection config error:%d\n",ret);
			return CMD_FAILURE;
		}
		memset(&domain_conf,0,sizeof(domain_conf));
		strncpy((domain_conf.domain_name),domain,sizeof(domain_conf.domain_name)-1);
		memset(&domain_ctr,0,sizeof(domain_ctr));
		ReInitDbusConnection(&dcli_dbus_connection_curr,slotid,distributFag);
		ret = conf_drp_get_domain_ip(dcli_dbus_connection_curr,	&domain_conf, &domain_ctr);
		ReInitDbusConnection(&dcli_dbus_connection_curr,slot_id,distributFag);
		_drp_return_if_fail(0 == ret,ret,CMD_WARNING);
		if (0 == domain_ctr.num) {
			vty_out(vty, "%% this domain can not parse, please check it\n");
			return CMD_SUCCESS;
		}
		//vty_out(vty, "conf_drp_get_domain_ip ret = %d \n",ret);
		if ( 0 == ret){
			int nbyte = 0;
			//vty_out(vty, "domain %s ip num %d \n",domain_ctr.domain_name,domain_ctr.num);
			for (i = 0; i < domain_ctr.num ; i++){
				nbyte = snprintf (domain,sizeof (domain) -1,"%s",(char *)argv[1]);
				for (i = 0; i<domain_ctr.num; i++){
					nbyte += snprintf(domain+nbyte,sizeof(domain)-nbyte -1,";%lu",domain_ctr.domain_ip[i].ipaddr);
				}
			}
		}

		ret = eag_conf_captive_list( dcli_dbus_connection_curr,
							hansitype,insid, type, "", "", domain, intfs, CP_ADD_LIST, CP_WHITE_LIST);
	}
	else if(0 == strcmp(argv[0], "del"))
	{
		ret = eag_conf_captive_list( dcli_dbus_connection_curr,
							hansitype,insid, type, "", "", domain, intfs, CP_DEL_LIST, CP_WHITE_LIST);
	}
	
	if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if(EAG_ERR_CAPTIVE_RULE_AREADY_IN_WHITE == ret) {
		vty_out(vty, "%% This rule aready in white-list\n");
	}
	else if(EAG_ERR_CAPTIVE_RULE_NOT_IN_WHITE == ret) {
		vty_out(vty, "%% This rule not in white-list\n");
	}
	else if(EAG_ERR_CAPTIVE_WHITE_LIST_NUM_LIMITE == ret) {
		vty_out(vty, "%% max white-list num limite:%d\n", MAX_BW_RULES_NUM);
	}
	else if (EAG_RETURN_OK != ret) {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}

	return CMD_SUCCESS;
}

DEFUN(conf_captive_portal_black_list_with_domain_func,
	conf_captive_portal_black_list_with_domain_cmd,
	"(add|del) black-list domain DOMAIN-URL [INTFS]",
	"add\n"
	"delete\n"
	"add or delete black list\n"
	"add or delete black list by domain format\n"
	"specifys that black list is described with domain-url format\n"
	"domain url to be applied to the black list\n"
)
{
	int ret = -1;
	RULE_TYPE type = RULE_DOMAIN;
	char intfs[50];
	char domain[256];
	domain_pt domain_conf;
	domain_ct domain_ctr;
	int i = 0;
	int slotid = HostSlotId;
	
	EAG_DCLI_INIT_HANSI_INFO
#if 0
	if(eag_ins_running_state(dcli_dbus_connection_curr, hansitype, insid)){
		vty_out(vty, "eag instance %d is running, please stop it first\n",insid);
		return CMD_FAILURE;
	}
#endif	
	memset(&intfs, 0, sizeof(intfs));
	memset(&domain, 0, sizeof(domain));
	if(3 == argc) {
		strncpy(intfs, argv[2], sizeof(intfs)-1);
		if (!if_nametoindex(intfs)) {
			vty_out(vty, "%% No such interface %s\n", intfs);
			return CMD_WARNING;
		}
	}
	if (0 == is_domain(argv[1])) {
		vty_out(vty, "%% error this domain can not parse, please check it\n");
		return CMD_SUCCESS;
	}
	strncpy(domain, argv[1], sizeof(domain)-1);
	
	if(0 == strcmp(argv[0], "add"))
	{
		ret = conf_drp_get_dbus_connect_params(&slotid);
		if (ret < 0){
			vty_out(vty, "eag get drp connection config error:%d\n",ret);
			return CMD_FAILURE;
		}
		memset(&domain_conf,0,sizeof(domain_conf));
		strncpy((domain_conf.domain_name),domain,sizeof(domain_conf.domain_name)-1);
		memset(&domain_ctr,0,sizeof(domain_ctr));
		ReInitDbusConnection(&dcli_dbus_connection_curr,slotid,distributFag);
		ret = conf_drp_get_domain_ip(dcli_dbus_connection_curr,	&domain_conf, &domain_ctr);
		ReInitDbusConnection(&dcli_dbus_connection_curr,slot_id,distributFag);
		_drp_return_if_fail(0 == ret,ret,CMD_WARNING);
		if (0 == domain_ctr.num) {
			vty_out(vty, "%% this domain can not parse, please check it\n");
			return CMD_SUCCESS;
		}
		//vty_out(vty, "conf_drp_get_domain_ip ret = %d \n",ret);
		if ( 0 == ret){
			int nbyte = 0;
			//vty_out(vty, "domain %s ip num %d \n",domain_ctr.domain_name,domain_ctr.num);
			for (i = 0; i < domain_ctr.num ; i++){
				nbyte = snprintf (domain,sizeof (domain) -1,"%s",(char *)argv[1]);
				for (i = 0; i<domain_ctr.num; i++){
					nbyte += snprintf(domain+nbyte,sizeof(domain)-nbyte -1,";%lu",domain_ctr.domain_ip[i].ipaddr);
				}
			}
		}
	
		ret = eag_conf_captive_list( dcli_dbus_connection_curr,
							hansitype,insid, type, "", "", domain, intfs, CP_ADD_LIST, CP_BLACK_LIST);
	}
	else if(0 == strcmp(argv[0], "del"))
	{
		ret = eag_conf_captive_list( dcli_dbus_connection_curr,
							hansitype, insid, type, "", "", domain, intfs, CP_DEL_LIST, CP_BLACK_LIST);
	}
	
	if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if ( EAG_ERR_CAPTIVE_RULE_AREADY_IN_BLACK == ret ){
		vty_out(vty, "%% This rule aready in black-list\n");
	}
	else if( EAG_ERR_CAPTIVE_RULE_NOT_IN_BLACK == ret ){
		vty_out(vty, "%% This rule not in black-list\n");
	}
	else if(EAG_ERR_CAPTIVE_BLACK_LIST_NUM_LIMITE == ret) {
		vty_out(vty, "%% max black-list num limite:%d\n", MAX_BW_RULES_NUM);
	}
	else if( EAG_RETURN_OK != ret ){
		vty_out(vty, "%% unknown error: %d\n", ret);
	}

	return CMD_SUCCESS;
}
#if 0
/* show/kick user */
#endif

static int mac_input_is_legal(char *value)
{

	//char mac[6] = "";
	int num = 0;
	int mac[6];
	
	if (NULL == value || NULL == mac || strlen(value) != 17) {
		return -1;
	}

	if (':' == value[1] || ':' == value[2]) {
		num = sscanf(value, "%02x:%02x:%02x:%02x:%02x:%02x",
			&mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
		if (6 == num) {
			sprintf(value,"%02X-%02X-%02X-%02X-%02X-%02X",
			mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		}
	}
	if ('-' == value[1] || '-' == value[2]) {
		num = sscanf(value, "%02x-%02x-%02x-%02x-%02x-%02x",
			&mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
		if (6 == num) {
			sprintf(value,"%02X-%02X-%02X-%02X-%02X-%02X",
			mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		}
	}

	return (6==num);//success---only argc num is 6
}

static int username_input_is_legal(char *value)
{
	return 1;
}

static int eag_show_user_log(struct vty* vty, DBusConnection *connection, int slot_id, int hansi_type, int hansi_id, char * key_word, int last_line_num)
{
	char src_path[] = "/var/log";
	char des_path[] = "/tmp";
	char src_file_path[64] = "";
	char des_file_path[64] = "";
	char log_name[64] = "";
	char packet_name[64] = "";
	char operate_cmd_str[128] = "";
	void *cmd_ret = (void *)0;
	int status = 0;
	char cmd[128] = "";
	char buff[1024] = "";
	char hansi_str[6] = "";
	char tail_str[32] = "";
	FILE *p_file = NULL;
	int i = 0;
	int local_id = 0;
	int ret = 0;

	local_id = get_product_info(PRODUCT_LOCAL_SLOTID);
	if (slot_id == local_id) {
		memset(des_file_path, 0, sizeof(des_file_path));
		snprintf(des_file_path, sizeof(des_file_path)-1, "%s/system.log", src_path);
		goto readlog;
	}
	
	memset(log_name, 0, sizeof(log_name));
	snprintf(log_name, sizeof(log_name)-1, "slot%d_ul", slot_id);
	memset(packet_name, 0, sizeof(packet_name));
	snprintf(packet_name, sizeof(packet_name)-1, "slot%d_ul.tar.bz2", slot_id);

	/*compress slot user log*/
	memset(operate_cmd_str, 0, sizeof(operate_cmd_str));
	snprintf(operate_cmd_str, sizeof(operate_cmd_str)-1,
			"cd %s;cat system.log|grep USERLOG > %s;tar -jcf %s %s >/dev/null 2>/dev/null", 
			src_path, log_name, packet_name, log_name);
	if (sizeof(operate_cmd_str) - 1 < strlen(operate_cmd_str)) {
		vty_out(vty, "%% Cmd length out range\n");
		return CMD_FAILURE;
	}

	if (0 != ac_manage_exec_extend_command(connection, 1, operate_cmd_str, &cmd_ret)) {
		vty_out(vty, "%% Operate slot cmd fail\n");
		return CMD_FAILURE;
	}

	/*copy slot user log to local*/
	memset(src_file_path, 0, sizeof(src_file_path));
	snprintf(src_file_path, sizeof(src_file_path)-1, "%s/%s", src_path, packet_name);
	memset(des_file_path, 0, sizeof(des_file_path));
	snprintf(des_file_path, sizeof(des_file_path)-1, "%s/%s", des_path, packet_name);

	if(connection)
	{
		ret = dcli_bsd_copy_file_to_board_v2(connection, local_id, src_file_path, des_file_path, 1, 0);
	}

	/*decompress user log*/
	memset(cmd, 0, sizeof(cmd));
	snprintf(cmd, sizeof(cmd)-1, "cd %s ; tar -jxf %s >/dev/null 2>/dev/null", des_path, packet_name);
	status = system(cmd);
	if (-1 == status || 0 == WIFEXITED(status)|| 0 != WEXITSTATUS(status)) {
		vty_out(vty, "%% Invalid cmd\n");
		return CMD_FAILURE;
	}

	memset(des_file_path, 0, sizeof(des_file_path));
	snprintf(des_file_path, sizeof(des_file_path)-1, "%s/%s", des_path, log_name);

readlog:
	/*print user log*/
	if (access(des_file_path, 0) != 0)
	{
		vty_out(vty, "%% Can not have required syslog!\n");
		return CMD_FAILURE;
	}

	if(NULL != key_word)
	{
		memset(cmd, 0, sizeof(cmd));
		snprintf(hansi_str, sizeof(hansi_str), "%d-%d:", hansi_type, hansi_id); //':' is used for distrabute 0-1 or 0-1x
		snprintf(cmd, sizeof(cmd), "cat %s|grep USERLOG|grep %s|grep ' %s:'", 
				des_file_path, hansi_str, key_word);
		if (last_line_num > 0) {
			snprintf(tail_str, sizeof(tail_str), "|tail -%d", last_line_num);
			strncat(cmd, tail_str, sizeof(cmd)-strlen(cmd)-1);
		}
	}
	p_file = popen(cmd, "r");

	if (NULL == p_file)
	{
		vty_out(vty, "%% Can not open file!\n");
		return CMD_FAILURE;
	}

	fgets(buff, sizeof(buff), p_file);
	while(buff[0] != '\0')
	{
		vty_out(vty, "%s", buff);
		memset(buff, 0, sizeof(buff));
		fgets(buff, sizeof(buff), p_file);		
	}

	pclose(p_file);

	if (slot_id == local_id) {
		return CMD_SUCCESS;
	}

	/*del slot user log*/
	memset(operate_cmd_str, 0, sizeof(operate_cmd_str));
	snprintf(operate_cmd_str, sizeof(operate_cmd_str)-1,
			"rm -rf %s/%s* >/dev/null 2>/dev/null", src_path, log_name);
	if (sizeof(operate_cmd_str) - 1 < strlen(operate_cmd_str)) {
		vty_out(vty, "%% Cmd length out range!\n");
		return CMD_FAILURE;
	}

	cmd_ret = (void *)0;
	if (0 != ac_manage_exec_extend_command(connection, 1, operate_cmd_str, &cmd_ret)) {
		vty_out(vty, "%% Operate slot cmd fail!\n");
		return CMD_FAILURE;
	}

	/*del user log*/
	memset(cmd, 0, sizeof(cmd));
	snprintf(cmd, sizeof(cmd)-1, "rm -rf %s/%s* >/dev/null 2>/dev/null", des_path, log_name);
	status = system(cmd);
	if (-1 == status || 0 == WIFEXITED(status)|| 0 != WEXITSTATUS(status)) {
		vty_out(vty, "%% Invalid cmd!\n");
		return CMD_FAILURE;
	}

	return CMD_SUCCESS;
}

DEFUN(eag_show_user_list_func,
	eag_show_user_list_cmd,
	"show user (list|all)",
	SHOW_STR
	"eag user\n"
	"list of eag users\n"
	"all of eag users\n"
)
{
	int ret = 0;
	int i = 0;
	struct eag_userdb userdb = {0};
	struct eag_user *user = NULL;
	char ipstr[32] = "";
	char ipv6str[48] = "";
	char macstr[36] = "";
	char ap_macstr[36] = "";
	uint32_t hour = 0;
	uint32_t minute = 0;
	uint32_t second = 0;
	char timestr[32] = "";
			
	EAG_DCLI_INIT_HANSI_INFO

	eag_userdb_init(&userdb);
	ret = eag_show_user_all(dcli_dbus_connection_curr,
							hansitype, insid,
							&userdb);
	if (EAG_RETURN_OK == ret) {
		vty_out(vty, "user num : %d\n", userdb.num);
		vty_out(vty, "%-7s %-18s %-16s %-40s %-18s %-12s %-18s %-18s %-18s %-12s\n",
				"ID", "UserName", "UserIP", "UserIPV6", "UserMAC", 
				"SessionTime", "OutputFlow", "InputFlow", "ApMAC","VLANID");

		list_for_each_entry(user, &(userdb.head), node) {
			i++;
			ip2str(user->user_ip, ipstr, sizeof(ipstr));
			ipv6tostr((struct in6_addr *)(user->user_ipv6), ipv6str, sizeof(ipv6str));
			mac2str(user->usermac, macstr, sizeof(macstr), ':');
			mac2str(user->apmac, ap_macstr, sizeof(ap_macstr), ':');
			hour = user->session_time/3600;
			minute = (user->session_time%3600)/60;
			second = user->session_time%60;
			snprintf(timestr, sizeof(timestr), "%u:%02u:%02u",
				hour, minute, second);
			
			vty_out(vty, "%-7d %-18s %-16s %-40s %-18s %-12s %-18llu %-18llu %-18s %-12lu\n",
				i, user->username, ipstr, ipv6str, macstr,
				timestr, user->output_octets, user->input_octets,ap_macstr,user->vlanid);
		}
		vty_out(vty, "user num : %d\n", userdb.num);
	}
	else if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}
	eag_userdb_destroy(&userdb);

	return CMD_SUCCESS;
}

DEFUN(eag_show_user_by_username_func,
	eag_show_user_by_username_cmd,
	"show user name USERNAME",
	SHOW_STR
	"eag user\n"
	"username\n"
	"username\n"
)
{
	int ret = 0;
	int i = 0;
	struct eag_userdb userdb = {0};
	struct eag_user *user = NULL;
	char ipstr[32] = "";
	char ipv6str[48] = "";
	char macstr[36] = "";
	uint32_t hour = 0;
	uint32_t minute = 0;
	uint32_t second = 0;
	char timestr[32] = "";
			
	EAG_DCLI_INIT_HANSI_INFO

	eag_userdb_init(&userdb);
	ret = eag_show_user_by_username(dcli_dbus_connection_curr,
							hansitype, insid,
							&userdb,
							argv[0]);
	if (EAG_RETURN_OK == ret) {
		vty_out(vty, "user num : %d\n", userdb.num);
		vty_out(vty, "%-7s %-18s %-16s %-40s %-18s %-12s %-18s %-18s\n",
				"ID", "UserName", "UserIP", "UserIPV6", "UserIPV6", "UserMAC", 
				"SessionTime", "OutputFlow", "InputFlow");

		list_for_each_entry(user, &(userdb.head), node) {
			i++;
			ip2str(user->user_ip, ipstr, sizeof(ipstr));
			ipv6tostr((struct in6_addr *)(user->user_ipv6), ipv6str, sizeof(ipv6str));
			mac2str(user->usermac, macstr, sizeof(macstr), ':');
			hour = user->session_time/3600;
			minute = (user->session_time%3600)/60;
			second = user->session_time%60;
			snprintf(timestr, sizeof(timestr), "%u:%02u:%02u",
				hour, minute, second);
			
			vty_out(vty, "%-7d %-18s %-16s %-40s %-18s %-12s %-18llu %-18llu\n",
				i, user->username, ipstr, ipv6str, macstr,
				timestr, user->output_octets, user->input_octets);
		}
		vty_out(vty, "user num : %d\n", userdb.num);
	}
	else if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}
	eag_userdb_destroy(&userdb);

	return CMD_SUCCESS;
}

DEFUN(eag_show_user_by_userip_func,
	eag_show_user_by_userip_cmd,
	"show user ip A.B.C.D",
	SHOW_STR
	"eag user\n"
	"user ip\n"
	"user ip\n"
)
{
	int ret = 0;
	int i = 0;
	struct eag_userdb userdb = {0};
	struct eag_user *user = NULL;
	char ipstr[32] = "";
	char ipv6str[48] = "";
	char macstr[36] = "";
	uint32_t hour = 0;
	uint32_t minute = 0;
	uint32_t second = 0;
	char timestr[32] = "";
	uint32_t userip = 0;
	struct in_addr addr = {0};

	ret = inet_aton(argv[0], &addr);
	if (!ret) {
		vty_out(vty, "%% invalid ip address\n");
		return CMD_WARNING;
	}

	EAG_DCLI_INIT_HANSI_INFO

	userip = ntohl(addr.s_addr);
	eag_userdb_init(&userdb);
	ret = eag_show_user_by_userip(dcli_dbus_connection_curr,
							hansitype, insid,
							&userdb,
							userip);
	if (EAG_RETURN_OK == ret) {
		vty_out(vty, "user num : %d\n", userdb.num);
		vty_out(vty, "%-7s %-18s %-16s %-40s %-18s %-12s %-18s %-18s\n",
				"ID", "UserName", "UserIP", "UserIPV6", "UserMAC", 
				"SessionTime", "OutputFlow", "InputFlow");

		list_for_each_entry(user, &(userdb.head), node) {
			i++;
			ip2str(user->user_ip, ipstr, sizeof(ipstr));
			ipv6tostr((struct in6_addr *)(user->user_ipv6), ipv6str, sizeof(ipv6str));
			mac2str(user->usermac, macstr, sizeof(macstr), ':');
			hour = user->session_time/3600;
			minute = (user->session_time%3600)/60;
			second = user->session_time%60;
			snprintf(timestr, sizeof(timestr), "%u:%02u:%02u",
				hour, minute, second);
			
			vty_out(vty, "%-7d %-18s %-16s %-40s %-18s %-12s %-18llu %-18llu\n",
				i, user->username, ipstr, ipv6str, macstr,
				timestr, user->output_octets, user->input_octets);
		}
		vty_out(vty, "user num : %d\n", userdb.num);
	}
	else if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}
	eag_userdb_destroy(&userdb);

	return CMD_SUCCESS;
}

DEFUN(eag_show_user_by_usermac_func,
	eag_show_user_by_usermac_cmd,
	"show user mac MAC",
	SHOW_STR
	"eag user\n"
	"user mac\n"
	"user mac\n"
)
{
	int ret = 0;
	int i = 0;
	struct eag_userdb userdb = {0};
	struct eag_user *user = NULL;
	char ipstr[32] = "";
	char ipv6str[48] = "";
	char macstr[36] = "";
	uint32_t hour = 0;
	uint32_t minute = 0;
	uint32_t second = 0;
	char timestr[32] = "";
	uint8_t usermac[6] = {0};

	ret = str2mac(argv[0], usermac);
	if (0 != ret) {
		vty_out(vty, "%% invalid mac\n");
		return CMD_WARNING;
	}

	EAG_DCLI_INIT_HANSI_INFO

	eag_userdb_init(&userdb);
	ret = eag_show_user_by_usermac(dcli_dbus_connection_curr,
							hansitype, insid,
							&userdb,
							usermac);
	if (EAG_RETURN_OK == ret) {
		vty_out(vty, "user num : %d\n", userdb.num);
		vty_out(vty, "%-7s %-18s %-16s %-40s %-18s %-12s %-18s %-18s\n",
				"ID", "UserName", "UserIP", "UserIPV6", "UserMAC", 
				"SessionTime", "OutputFlow", "InputFlow");

		list_for_each_entry(user, &(userdb.head), node) {
			i++;
			ip2str(user->user_ip, ipstr, sizeof(ipstr));
			ipv6tostr((struct in6_addr *)(user->user_ipv6), ipv6str, sizeof(ipv6str));
			mac2str(user->usermac, macstr, sizeof(macstr), ':');
			hour = user->session_time/3600;
			minute = (user->session_time%3600)/60;
			second = user->session_time%60;
			snprintf(timestr, sizeof(timestr), "%u:%02u:%02u",
				hour, minute, second);
			
			vty_out(vty, "%-7d %-18s %-16s %-40s %-18s %-12s %-18llu %-18llu\n",
				i, user->username, ipstr, ipv6str, macstr,
				timestr, user->output_octets, user->input_octets);
		}
		vty_out(vty, "user num : %d\n", userdb.num);
	}
	else if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}
	eag_userdb_destroy(&userdb);

	return CMD_SUCCESS;
}

DEFUN(eag_show_user_by_index_func,
	eag_show_user_by_index_cmd,
	"show user index <1-10240>",
	SHOW_STR
	"eag user\n"
	"user index\n"
	"user index\n"
)
{
	int ret = 0;
	int i = 0;
	struct eag_userdb userdb = {0};
	struct eag_user *user = NULL;
	char ipstr[32] = "";
	char ipv6str[48] = "";
	char macstr[36] = "";
	uint32_t hour = 0;
	uint32_t minute = 0;
	uint32_t second = 0;
	char timestr[32] = "";
	uint32_t index = 0;

	index = atoi(argv[0]);

	EAG_DCLI_INIT_HANSI_INFO

	eag_userdb_init(&userdb);
	ret = eag_show_user_by_index(dcli_dbus_connection_curr,
							hansitype, insid,
							&userdb,
							index);
	if (EAG_RETURN_OK == ret) {
		vty_out(vty, "user num : %d\n", userdb.num);
		vty_out(vty, "%-7s %-18s %-16s %-40s %-18s %-12s %-18s %-18s\n",
				"ID", "UserName", "UserIP", "UserIPV6", "UserMAC", 
				"SessionTime", "OutputFlow", "InputFlow");

		list_for_each_entry(user, &(userdb.head), node) {
			i++;
			ip2str(user->user_ip, ipstr, sizeof(ipstr));
			ipv6tostr((struct in6_addr *)(user->user_ipv6), ipv6str, sizeof(ipv6str));
			mac2str(user->usermac, macstr, sizeof(macstr), ':');
			hour = user->session_time/3600;
			minute = (user->session_time%3600)/60;
			second = user->session_time%60;
			snprintf(timestr, sizeof(timestr), "%u:%02u:%02u",
				hour, minute, second);
			
			vty_out(vty, "%-7d %-18s %-16s %-40s %-18s %-12s %-18llu %-18llu\n",
				i, user->username, ipstr, ipv6str, macstr,
				timestr, user->output_octets, user->input_octets);
		}
		vty_out(vty, "user num : %d\n", userdb.num);
	}
	else if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}
	eag_userdb_destroy(&userdb);

	return CMD_SUCCESS;
}

DEFUN(eag_show_user_log_func,
	eag_show_user_log_cmd,
	"show user-log (ip|mac|username) KEYWORD [LINE_NUM]",
	SHOW_STR
	"show user log\n"
	"show user log by ip A.B.C.D\n"
	"show user log by mac MAC\n"
	"show user log by username NAME\n"
	"A.B.C.D, MAC or USERNAME\n"
	"show last few lines of the user log\n"
)
{
	int ret;
	int key = 0;
	char *key_word = NULL;
	int last_line_num = 0;
	
	
	if (argc == 3) {
		last_line_num = atoi(argv[2]);
		if (last_line_num <= 0) {
			vty_out(vty,"%% line num input error!\n");
			return CMD_WARNING;
		}
	} else if (argc == 2){
		last_line_num = 0;
	} else {
		vty_out(vty,"%% Bad command parameter\n");
		return CMD_WARNING;
	}
	key_word = (char *)argv[1];
	if (0 == strncmp(argv[0], "ip",strlen(argv[0]))) {
		key = SHOW_LOG_BY_IP;
		if (0 != ip_input_is_legal(key_word)) {
			vty_out(vty, "%% IP format error!\n");
			return CMD_WARNING;
		}
	} else if (0 == strncmp(argv[0], "mac",strlen(argv[0]))) {
		key = SHOW_LOG_BY_MAC;
		if (1 != mac_input_is_legal(key_word)) {
			vty_out(vty,"%% MAC format error!\n");
			return CMD_WARNING;
		}
	} else if (0 == strncmp(argv[0], "username",strlen(argv[0]))) {
		key = SHOW_LOG_BY_UNAME;
		if (!username_input_is_legal((char *)argv[1])) {
			vty_out(vty,"%% USERNAME has ilegal words!\n");
			return CMD_WARNING;
		}
	} else {
		vty_out(vty,"%% Bad command parameter\n");
		return CMD_WARNING;
	}
	
	EAG_DCLI_INIT_HANSI_INFO
		
	ret = eag_show_user_log(vty, dcli_dbus_connection_curr, slot_id, hansitype, insid, key_word, last_line_num);

	if (CMD_SUCCESS != ret) {
		vty_out(vty, "%% cmd fail\n");
	}

	return CMD_SUCCESS;
}

DEFUN(eag_kick_user_by_username_func,
	eag_kick_user_by_username_cmd,
	"kick user name USERNAME",
	"kick user offline\n"
	"eag user\n"
	"username\n"
	"username\n"
)
{
	int ret = 0;
		
	EAG_DCLI_INIT_HANSI_INFO

	ret = eag_kick_user_by_username(dcli_dbus_connection_curr,
							hansitype, insid,
							argv[0]);
	if (EAG_RETURN_OK == ret) {
		return CMD_SUCCESS;
	}
	else if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_ERR_APPCONN_DELAPP_NOT_INDB == ret) {
		vty_out(vty, "%% user not exist\n");
	}
	else if (EAG_ERR_HANSI_IS_BACKUP == ret) {
		vty_out(vty, "%% device is backup\n");
	}
	else {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}

	return CMD_SUCCESS;
}

DEFUN(eag_kick_user_by_userip_func,
	eag_kick_user_by_userip_cmd,
	"kick user ip A.B.C.D",
	"kick user offline\n"
	"eag user\n"
	"user ip\n"
	"user ip\n"
)
{
	int ret = 0;
	uint32_t userip = 0;
	struct in_addr addr = {0};
	
	ret = inet_aton(argv[0], &addr);
	if (!ret) {
		vty_out(vty, "%% invalid ip address\n");
		return CMD_WARNING;
	}

	EAG_DCLI_INIT_HANSI_INFO

	userip = ntohl(addr.s_addr);
	ret = eag_kick_user_by_userip(dcli_dbus_connection_curr,
							hansitype, insid,
							userip);
	if (EAG_RETURN_OK == ret) {
		return CMD_SUCCESS;
	}
	else if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_ERR_APPCONN_DELAPP_NOT_INDB == ret) {
		vty_out(vty, "%% user not exist\n");
	}
	else if (EAG_ERR_HANSI_IS_BACKUP == ret) {
		vty_out(vty, "%% device is backup\n");
	}
	else {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}

	return CMD_SUCCESS;
}

DEFUN(eag_kick_user_by_usermac_func,
	eag_kick_user_by_usermac_cmd,
	"kick user mac MAC",
	"kick user offline\n"
	"eag user\n"
	"user mac\n"
	"user mac\n"
)
{
	int ret = 0;
	uint8_t usermac[6] = {0};
	
	ret = str2mac(argv[0], usermac);
	if (0 != ret) {
		vty_out(vty, "%% invalid mac\n");
		return CMD_WARNING;
	}

	EAG_DCLI_INIT_HANSI_INFO

	ret = eag_kick_user_by_usermac(dcli_dbus_connection_curr,
							hansitype, insid,
							usermac);
	if (EAG_RETURN_OK == ret) {
		return CMD_SUCCESS;
	}
	else if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_ERR_APPCONN_DELAPP_NOT_INDB == ret) {
		vty_out(vty, "%% user not exist\n");
	}
	else if (EAG_ERR_HANSI_IS_BACKUP == ret) {
		vty_out(vty, "%% device is backup\n");
	}
	else {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}

	return CMD_SUCCESS;
}

DEFUN(eag_kick_user_by_index_func,
	eag_kick_user_by_index_cmd,
	"kick user index <1-10240>",
	"kick user offline\n"
	"eag user\n"
	"user index\n"
	"user index\n"
)
{
	int ret = 0;
	uint32_t index = 0;

	index = atoi(argv[0]);
	
	EAG_DCLI_INIT_HANSI_INFO

	ret = eag_kick_user_by_index(dcli_dbus_connection_curr,
							hansitype, insid,
							index);
	if (EAG_RETURN_OK == ret) {
		return CMD_SUCCESS;
	}
	else if (EAG_ERR_DBUS_FAILED == ret) {
		vty_out(vty, "%% dbus error\n");
	}
	else if (EAG_ERR_APPCONN_DELAPP_NOT_INDB == ret) {
		vty_out(vty, "%% user not exist\n");
	}
	else if (EAG_ERR_HANSI_IS_BACKUP == ret) {
		vty_out(vty, "%% device is backup\n");
	}
	else {
		vty_out(vty, "%% unknown error: %d\n", ret);
	}

	return CMD_SUCCESS;
}
#if 1
/* add methods of debug for eag */

DEFUN(eag_set_user_log_status_func,
	eag_set_user_log_status_cmd,
	"(open|close) user-log",
	"open\n"
	"close\n"
	"user-log-filter\n"
)
{
	int ret = 0;
	int status = 0;
	int key = 0;

	EAG_DCLI_INIT_HANSI_INFO

	if (strncmp(argv[0], "open", strlen(argv[0])) == 0) {
		status = 1;
	}
	else if (strncmp(argv[0], "close", strlen(argv[0])) == 0) {
		status = 0;
	}
	else{
		vty_out(vty, "%% unknown error\n");
		return CMD_WARNING;
	}

	ret = eag_set_user_log_status( dcli_dbus_connection_curr, hansitype, insid, status);

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

DEFUN(eag_set_log_format_status_func,
	eag_set_log_format_status_cmd,
	"set log-format (autelan|henan) (on|off)",
	SETT_STR
	"log-format\n"
	"autelan log\n"
	"henan log\n"
	"log on\n"
	"log off\n"
)
{
	int ret = 0;
	int key = 0;
	int status = 0;

	EAG_DCLI_INIT_HANSI_INFO

	if (strncmp(argv[0], "autelan", strlen(argv[0])) == 0) {
		key = AUTELAN_LOG;
	}
	else if (strncmp(argv[0], "henan", strlen(argv[0])) == 0) {
		key = HENAN_LOG;
	}
	else{
		vty_out(vty, "%% unknown error\n");
		return CMD_WARNING;
	}
	
	if (strncmp(argv[1], "on", strlen(argv[1])) == 0) {
		status = 1;
	}
	else if (strncmp(argv[1], "off", strlen(argv[1])) == 0) {
		status = 0;
	}
	else{
		vty_out(vty, "%% unknown error\n");
		return CMD_WARNING;
	}
	
	ret = eag_set_log_format_status( dcli_dbus_connection_curr, hansitype, insid, key, status);

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

DEFUN(eag_set_username_check_status_func,
	eag_set_username_check_status_cmd,
	"set username-check (on|off)",
	SETT_STR
	"username-check\n"
	"on\n"
	"off\n"
)
{
	int ret = 0;
	int status = 0;

	EAG_DCLI_INIT_HANSI_INFO

	if (strncmp(argv[0], "on", strlen(argv[1])) == 0) {
		status = 1;
	}
	else if (strncmp(argv[0], "off", strlen(argv[1])) == 0) {
		status = 0;
	}
	else{
		vty_out(vty, "%% unknown error\n");
		return CMD_WARNING;
	}
	
	ret = eag_set_username_check_status( dcli_dbus_connection_curr, hansitype, insid, status);

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

DEFUN(config_eag_debug_cmd_func,
	config_eag_debug_cmd,
	"debug eag (all|error|warning|notice|info|debug) <0-1> <0-16>",
	"debug\n"
	MODULE_DEBUG_STR(eag)
	MODULE_DEBUG_LEVEL_STR(eag, all)
	MODULE_DEBUG_LEVEL_STR(eag, error)
	MODULE_DEBUG_LEVEL_STR(eag, warning)
	MODULE_DEBUG_LEVEL_STR(eag, notice)
	MODULE_DEBUG_LEVEL_STR(eag, info)
	MODULE_DEBUG_LEVEL_STR(eag, debug)
	"hansi type, 0: local; 1: remote\n"
	"hansi id\n"
)
{
	int ret = 0;
	unsigned int flag = 0;
	int hansi_type = atoi(argv[1]);
	int hansi_id = atoi(argv[2]);
	
	if(argc > 3) {
		vty_out(vty, "%% Command parameters number error!\n");
		return CMD_WARNING;
	}

	if(0 == strncmp(argv[0], "all", strlen(argv[0]))) {
		flag = LOG_AT_LEAST_DEBUG;
	}		
	else if(0 == strncmp(argv[0],"error", strlen(argv[0]))) {
		flag = LOG_ERR_MASK;
	}
	else if (0 == strncmp(argv[0],"warning", strlen(argv[0]))) {
		flag = LOG_WARNING_MASK;
	}
	else if (0 == strncmp(argv[0],"notice", strlen(argv[0]))) {
		flag = LOG_NOTICE_MASK;
	}
	else if (0 == strncmp(argv[0],"info", strlen(argv[0]))) {
		flag = LOG_INFO_MASK;
	}
	else if (0 == strncmp(argv[0],"debug", strlen(argv[0]))) {
		flag = LOG_DEBUG_MASK;
	}
	else {
		vty_out(vty, "%% Command parameter %s error!\n", argv[0]);
		return CMD_WARNING;
	}
	vty_out(vty, "%%mask = %#x\n", flag);
	ret = eag_dbus_eag_debug_on(dcli_dbus_connection, hansi_type, hansi_id, flag);

	return CMD_SUCCESS;
}

DEFUN(config_eag_no_debug_cmd_func,
	config_eag_no_debug_cmd,
	"no debug eag (all|error|warning|notice|info|debug) <0-1> <0-16>",
	"Disable specific function \n"
	NODEBUG_STR
	MODULE_DEBUG_STR(eag)
	MODULE_DEBUG_LEVEL_STR(eag, all)
	MODULE_DEBUG_LEVEL_STR(eag, error)
	MODULE_DEBUG_LEVEL_STR(eag, warning)
	MODULE_DEBUG_LEVEL_STR(eag, notice)
	MODULE_DEBUG_LEVEL_STR(eag, info)
	MODULE_DEBUG_LEVEL_STR(eag, debug)
	"hansi type, 0: local, 1:remote\n"
	"hansi id\n"
)
{
	int ret = 0;
	unsigned int flag = 0;
	int hansi_type = atoi(argv[1]);
	int hansi_id = atoi(argv[2]);
	
	if (argc > 3) {
		vty_out(vty, "%% Command parameters number error!\n");
		return CMD_WARNING;
	}

	if(0 == strncmp(argv[0], "all", strlen(argv[0]))) {
		flag = LOG_AT_LEAST_DEBUG;
	}		
	else if(0 == strncmp(argv[0],"error", strlen(argv[0]))) {
		flag = LOG_ERR_MASK;
	}
	else if (0 == strncmp(argv[0],"warning", strlen(argv[0]))) {
		flag = LOG_WARNING_MASK;
	}
	else if (0 == strncmp(argv[0],"notice", strlen(argv[0]))) {
		flag = LOG_NOTICE_MASK;
	}
	else if (0 == strncmp(argv[0],"info", strlen(argv[0]))) {
		flag = LOG_INFO_MASK;
	}
	else if (0 == strncmp(argv[0],"debug", strlen(argv[0]))) {
		flag = LOG_DEBUG_MASK;
	}
	else {
		vty_out(vty, "%% Command parameter %s error!\n", argv[0]);
		return CMD_WARNING;
	}

	ret = eag_dbus_eag_debug_off(dcli_dbus_connection, hansi_type, hansi_id, flag);
	
	return CMD_SUCCESS;
}

DEFUN(debug_eag_pkt_info,
	config_eag_debug_pkt_info_cmd,
	"debug eag packet (all|receive|send) <0-1> <0-16>",
	"debug\n"
	MODULE_DEBUG_STR(eag)
	MODULE_DEBUG_STR(packet)
	MODULE_DEBUG_LEVEL_STR(packet, all)
	MODULE_DEBUG_LEVEL_STR(packet, receive)
	MODULE_DEBUG_LEVEL_STR(packet, send)
	"hansi type, 0: local, 1:remote\n"
	"hansi id\n"
)
{
	int ret = 0;
	unsigned int flag = 0;
	int hansi_type = atoi(argv[1]);
	int hansi_id = atoi(argv[2]);
	
	if (argc > 3) {
		vty_out(vty, "%% Command parameters number error!\n");
		return CMD_WARNING;
	}
	
	if (0 == strncmp(argv[0], "all", strlen(argv[0])))	{
		flag = DCLI_DEBUG_FLAG_PKT_ALL;
	}else if (0 == strncmp(argv[0], "receive", strlen(argv[0]))) {
		flag = DCLI_DEBUG_FLAG_PKT_REV;
	}else if (0 == strncmp(argv[0], "send", strlen(argv[0]))) {
		flag = DCLI_DEBUG_FLAG_PKT_SED;
	}else {
		vty_out(vty,"%% Command parameter %s error!\n", argv[0]);
		return CMD_WARNING;
	}

	ret = eag_dbus_eag_debug_on(dcli_dbus_connection, hansi_type, hansi_id, flag);
	
	return CMD_SUCCESS;
}

DEFUN(no_debug_eag_pkt_info,
	config_eag_no_debug_pkt_info_cmd,
	"no debug eag packet (all|receive|send) <0-1> <0-16>",
	NODEBUG_STR
	"debug\n"
	MODULE_DEBUG_STR(eag)
	MODULE_DEBUG_STR(packet)
	MODULE_DEBUG_LEVEL_STR(packet, all)
	MODULE_DEBUG_LEVEL_STR(packet, receive)
	MODULE_DEBUG_LEVEL_STR(packet, send)
	"hansi type, 0: local, 1:remote\n"
	"hansi id\n"
)
{	
	int ret = 0;
	unsigned int flag = 0;
	int hansi_type = atoi(argv[1]);
	int hansi_id = atoi(argv[2]);
	
	if (argc > 3) {
		vty_out(vty,"%% Command parameters number error!\n");
		return CMD_WARNING;
	}

	if (0 == strncmp(argv[0], "all", strlen(argv[0]))) {
		flag = DCLI_DEBUG_FLAG_PKT_ALL;
	}else if (0 == strncmp(argv[0], "receive", strlen(argv[0]))) {
		flag = DCLI_DEBUG_FLAG_PKT_REV;
	}else if (0 == strncmp(argv[0], "send", strlen(argv[0]))) {
		flag = DCLI_DEBUG_FLAG_PKT_SED;
	}else {
		vty_out(vty,"%% Command parameter %s error!\n", argv[0]);
		return CMD_WARNING;
	}

	ret = eag_dbus_eag_debug_off(dcli_dbus_connection, hansi_type, hansi_id, flag);
	
	return CMD_SUCCESS;
}
#endif

#if 1
/* test */
#endif
DEFUN(set_eag_ins_test_func,
	set_eag_ins_test_cmd,
	"test",
	SETT_STR
)
{	
	//eag_show_syslog(vty, ,"100");
	return CMD_SUCCESS;
}

void 
dcli_eag_init(void)
{
	/* eag node */
	install_node(&eag_node, dcli_eag_show_running_config,"eag");
	install_default(EAG_NODE);	
	install_element(CONFIG_NODE, &conf_eag_cmd);

	/* nasid */
	install_node(&eag_nasid_node, dcli_eag_show_running_return,"eag-nasid");
	install_default(EAG_NASID_NODE);
	install_element(EAG_NODE, &conf_eag_nas_policy_cmd);
	install_element(EAG_NASID_NODE, &add_eag_nas_policy_vlan_cmd);
	//install_element(EAG_NASID_NODE, &add_eag_nas_policy_subintf_cmd);
	install_element(EAG_NASID_NODE, &add_eag_nas_policy_subintf_both_cmd);
	install_element(EAG_NASID_NODE, &add_eag_nas_policy_subintf_begin_cmd);
	install_element(EAG_NASID_NODE, &add_eag_nas_policy_subintf_end_cmd);
	install_element(EAG_NASID_NODE, &add_eag_nas_policy_subintf_none_cmd);
	install_element(EAG_NASID_NODE, &add_eag_nas_policy_wlan_cmd);
	install_element(EAG_NASID_NODE, &add_eag_nas_policy_ipaddress_cmd);
	install_element(EAG_NASID_NODE, &add_eag_nas_policy_wtp_cmd);
	//install_element(EAG_NASID_NODE, &del_eag_nas_policy_cmd);
	install_element(EAG_NASID_NODE, &del_eag_nas_policy_vlan_cmd);
	//install_element(EAG_NASID_NODE, &del_eag_nas_policy_subintf_cmd);
	install_element(EAG_NASID_NODE, &del_eag_nas_policy_subintf_both_cmd);
	install_element(EAG_NASID_NODE, &del_eag_nas_policy_subintf_begin_cmd);
	install_element(EAG_NASID_NODE, &del_eag_nas_policy_subintf_end_cmd);
	install_element(EAG_NASID_NODE, &del_eag_nas_policy_subintf_none_cmd);
	install_element(EAG_NASID_NODE, &del_eag_nas_policy_wlan_cmd);
	install_element(EAG_NASID_NODE, &del_eag_nas_policy_ipaddress_cmd);
	install_element(EAG_NASID_NODE, &del_eag_nas_policy_wtp_cmd);
	install_element(CONFIG_NODE, &show_eag_nas_policy_cmd);
	install_element(VIEW_NODE, &show_eag_nas_policy_cmd);
	install_element(EAG_NODE, &show_eag_nas_policy_cmd);
	install_element(EAG_NASID_NODE, &show_eag_nas_policy_cmd);
	//install_element(EAG_NODE, &del_eag_nasid_policy_cmd);
	//install_element(CONFIG_NODE, &show_eag_nasid_policy_cmd);
	//install_element(VIEW_NODE, &show_eag_nasid_policy_cmd);
	//install_element(EAG_NODE, &show_eag_nasid_policy_cmd);
	//install_element(EAG_NASID_NODE, &show_eag_nasid_policy_cmd);
	//install_element(EAG_NASID_NODE, &set_eag_nasid_policy_default_entry_cmd);
	//install_element(EAG_NASID_NODE, &add_eag_nasid_entry_cmd);
	//install_element(EAG_NASID_NODE, &del_eag_nasid_entry_cmd);
	
	/* radius */
	install_node(&eag_radius_node, dcli_eag_show_running_return,"eag-radius");
	install_default(EAG_RADIUS_NODE);	
	install_element(EAG_NODE, &conf_eag_radius_policy_cmd);
	install_element(EAG_RADIUS_NODE, &set_eag_radius_policy_server_type_cmd);
	install_element(EAG_RADIUS_NODE, &set_eag_radius_policy_server_ip_cmd);
	install_element(EAG_RADIUS_NODE, &set_eag_radius_policy_server_port_cmd);
	install_element(EAG_RADIUS_NODE, &set_eag_radius_policy_server_key_cmd);
	install_element(EAG_RADIUS_NODE, &set_eag_radius_policy_server_protocol_cmd);
	install_element(EAG_RADIUS_NODE, &set_eag_radius_policy_rev_updown_flow_cmd);
	install_element(EAG_RADIUS_NODE, &del_eag_radius_policy_by_domain_cmd);
	install_element(CONFIG_NODE, &show_eag_radius_policy_cmd);
	install_element(VIEW_NODE, &show_eag_radius_policy_cmd);
	install_element(EAG_NODE, &show_eag_radius_policy_cmd);
	install_element(EAG_RADIUS_NODE, &show_eag_radius_policy_cmd);
	//install_element(VIEW_NODE, &show_cur_eag_radius_policy_cmd);
	//install_element(EAG_RADIUS_NODE, &show_cur_eag_radius_policy_cmd);

	/* portal */
	install_node(&eag_portal_node, dcli_eag_show_running_return,"eag-portal");
	install_default(EAG_PORTAL_NODE);
	install_element(EAG_NODE, &conf_eag_portal_policy_cmd);
	install_element(EAG_PORTAL_NODE, &set_eag_portal_policy_protocol_cmd);
	//install_element(EAG_PORTAL_NODE, &set_eag_portal_policy_ip_cmd);
	install_element(EAG_PORTAL_NODE, &set_eag_portal_policy_port_cmd);
	install_element(EAG_PORTAL_NODE, &set_eag_portal_policy_web_url_cmd);
	install_element(EAG_PORTAL_NODE, &set_eag_portal_policy_acname_cmd);
	install_element(EAG_PORTAL_NODE, &set_eag_portal_policy_type_cmd);	
	install_element(EAG_PORTAL_NODE, &set_eag_portal_policy_domain_cmd);
	install_element(EAG_PORTAL_NODE, &set_eag_portal_policy_ad_web_url_cmd);
	install_element(EAG_PORTAL_NODE, &del_eag_portal_policy_by_key_word_cmd);
	install_element(CONFIG_NODE, &show_eag_portal_policy_cmd);
	install_element(VIEW_NODE, &show_eag_portal_policy_cmd);
	install_element(EAG_NODE, &show_eag_portal_policy_cmd);
	install_element(EAG_PORTAL_NODE, &show_eag_portal_policy_cmd);
	
	/* vlanmap */
	install_node(&eag_vlanmap_node, dcli_eag_show_running_return,"eag-vlanmap");
	install_default(EAG_VLANMAP_NODE);
	install_element(EAG_NODE, &conf_eag_vlan_map_policy_cmd);
	install_element(EAG_VLANMAP_NODE, &add_eag_vlan_map_policy_cmd);
	install_element(EAG_VLANMAP_NODE, &del_eag_vlan_map_policy_cmd);
	install_element(CONFIG_NODE, &show_eag_vlan_map_policy_cmd);
	install_element(VIEW_NODE, &show_eag_vlan_map_policy_cmd);
	install_element(EAG_NODE, &show_eag_vlan_map_policy_cmd);
	install_element(EAG_VLANMAP_NODE, &show_eag_vlan_map_policy_cmd);

	/* eag_ins node */
	install_node(&eag_eagins_node, dcli_eag_show_running_return,"eag-ins");
	install_default(EAG_INS_NODE);	
	install_element(EAG_NODE, &conf_eag_ins_cmd);
	install_element(EAG_INS_NODE, &set_eag_ins_switch_options_cmd);
	install_element(EAG_INS_NODE, &set_eag_ins_ip_options_cmd);
	install_element(EAG_INS_NODE, &set_eag_ins_port_num_options_cmd);
	install_element(EAG_INS_NODE, &set_eag_ins_policy_cmd);
	install_element(EAG_INS_NODE, &set_eag_ins_timeout_option_cmd);
	install_element(EAG_INS_NODE, &set_eag_ins_vrrpid_option_cmd);
	install_element(EAG_INS_NODE, &set_eag_ins_maxhttprsp_option_cmd);
	install_element(EAG_INS_NODE, &set_eag_ins_service_options_cmd);
	install_element(EAG_NODE, &del_eag_ins_cmd);
	install_element(CONFIG_NODE, &show_eag_ins_all_option_cmd);
	install_element(VIEW_NODE, &show_eag_ins_all_option_cmd);
	install_element(EAG_NODE, &show_eag_ins_all_option_cmd);
	install_element(EAG_INS_NODE, &show_eag_ins_all_option_cmd);
	//install_element(EAG_INS_NODE, &show_single_eag_ins_single_option_cmd);
	//install_element(EAG_INS_NODE, &set_eag_ins_string_options_cmd);

	/* syslog */
	install_element(VIEW_NODE,&show_syslog_all_cmd);
	install_element(ENABLE_NODE,&show_syslog_all_cmd);
	install_element(CONFIG_NODE,&show_syslog_all_cmd);

	install_element(VIEW_NODE,&show_syslog_cli_cmd);
	install_element(ENABLE_NODE,&show_syslog_cli_cmd);
	install_element(CONFIG_NODE,&show_syslog_cli_cmd);
	
	install_element(VIEW_NODE,&show_syslog_line_num_cmd);
	install_element(ENABLE_NODE,&show_syslog_line_num_cmd);
	install_element(CONFIG_NODE,&show_syslog_line_num_cmd);
	
	install_element(VIEW_NODE,&show_syslog_last_line_cmd);
	install_element(ENABLE_NODE,&show_syslog_last_line_cmd);
	install_element(CONFIG_NODE,&show_syslog_last_line_cmd);
		
	install_element(VIEW_NODE,&show_syslog_key_word_cmd);
	install_element(ENABLE_NODE,&show_syslog_key_word_cmd);
	install_element(CONFIG_NODE,&show_syslog_key_word_cmd);

	install_element(VIEW_NODE,&show_syslog_time_cmd);
	install_element(ENABLE_NODE,&show_syslog_time_cmd);
	install_element(CONFIG_NODE,&show_syslog_time_cmd);
	
	/*eag 2.0*/
	/*base config*/	
	install_node(&hansi_eag_node, dcli_eag_show_running_return,"eag-hansi");
	install_default(HANSI_EAG_NODE);	
	install_node(&local_hansi_eag_node, dcli_eag_show_running_return,"eag-local-hansi");
	install_default(LOCAL_HANSI_EAG_NODE);

	install_element(HANSI_NODE, &conf_eag_cmd);
	install_element(LOCAL_HANSI_NODE, &conf_eag_cmd);

	install_element(EAG_NODE, &set_eag_nasip_cmd);
	install_element(HANSI_EAG_NODE, &set_eag_nasip_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &set_eag_nasip_cmd);
	
	install_element(EAG_NODE, &set_eag_nasipv6_cmd);
	install_element(HANSI_EAG_NODE, &set_eag_nasipv6_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &set_eag_nasipv6_cmd);

    install_element(EAG_NODE, &set_eag_ipv6_service_status_cmd);
    install_element(HANSI_EAG_NODE, &set_eag_ipv6_service_status_cmd);
    install_element(LOCAL_HANSI_EAG_NODE, &set_eag_ipv6_service_status_cmd);

	install_element(EAG_NODE, &set_eag_distributed_cmd);
	install_element(HANSI_EAG_NODE, &set_eag_distributed_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &set_eag_distributed_cmd);

	install_element(EAG_NODE, &set_eag_rdc_distributed_cmd);
	install_element(HANSI_EAG_NODE, &set_eag_rdc_distributed_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &set_eag_rdc_distributed_cmd);
	
	install_element(EAG_NODE, &set_eag_pdc_distributed_cmd);
	install_element(HANSI_EAG_NODE, &set_eag_pdc_distributed_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &set_eag_pdc_distributed_cmd);
	
	install_element(EAG_NODE, &set_eag_l2super_vlan_status_cmd);
	install_element(HANSI_EAG_NODE, &set_eag_l2super_vlan_status_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &set_eag_l2super_vlan_status_cmd);
	
	install_element(EAG_NODE, &set_eag_rdcpdc_hansi_cmd);
	install_element(HANSI_EAG_NODE, &set_eag_rdcpdc_hansi_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &set_eag_rdcpdc_hansi_cmd);

	install_element(EAG_NODE, &set_eag_rdc_hansi_cmd);
	install_element(HANSI_EAG_NODE, &set_eag_rdc_hansi_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &set_eag_rdc_hansi_cmd);
	
	install_element(EAG_NODE, &set_eag_pdc_hansi_cmd);
	install_element(HANSI_EAG_NODE, &set_eag_pdc_hansi_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &set_eag_pdc_hansi_cmd);

	install_element(EAG_NODE, &set_eag_portal_port_cmd);
	install_element(HANSI_EAG_NODE, &set_eag_portal_port_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &set_eag_portal_port_cmd);
	
	install_element(EAG_NODE, &set_eag_portal_retry_params_cmd);
	install_element(HANSI_EAG_NODE, &set_eag_portal_retry_params_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &set_eag_portal_retry_params_cmd);	
	
	install_element(EAG_NODE, &set_eag_auto_session_cmd);
	install_element(HANSI_EAG_NODE, &set_eag_auto_session_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &set_eag_auto_session_cmd);

	install_element(EAG_NODE, &set_eag_acct_interval_cmd);
	install_element(HANSI_EAG_NODE, &set_eag_acct_interval_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &set_eag_acct_interval_cmd);	

	install_element(EAG_NODE, &set_eag_radius_retry_params_cmd);
	install_element(HANSI_EAG_NODE, &set_eag_radius_retry_params_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &set_eag_radius_retry_params_cmd);

	install_element(EAG_NODE, &set_eag_max_redir_times_cmd);
	install_element(HANSI_EAG_NODE, &set_eag_max_redir_times_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &set_eag_max_redir_times_cmd);

	install_element(EAG_NODE, &set_eag_force_dhcplease_cmd);
	install_element(HANSI_EAG_NODE, &set_eag_force_dhcplease_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &set_eag_force_dhcplease_cmd);

	install_element(EAG_NODE, &set_eag_check_errid_cmd);
	install_element(HANSI_EAG_NODE, &set_eag_check_errid_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &set_eag_check_errid_cmd);

	install_element(EAG_NODE, &set_eag_idle_params_cmd);
	install_element(HANSI_EAG_NODE, &set_eag_idle_params_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &set_eag_idle_params_cmd);

	install_element(EAG_NODE, &set_eag_force_wireless_cmd);
	install_element(HANSI_EAG_NODE, &set_eag_force_wireless_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &set_eag_force_wireless_cmd);

	install_element(EAG_NODE, &set_eag_flux_from_cmd);
	install_element(HANSI_EAG_NODE, &set_eag_flux_from_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &set_eag_flux_from_cmd);

	install_element(EAG_NODE, &set_eag_flux_interval_cmd);
	install_element(HANSI_EAG_NODE, &set_eag_flux_interval_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &set_eag_flux_interval_cmd);

	install_element(EAG_NODE, &set_eag_ipset_auth_cmd);
	install_element(HANSI_EAG_NODE, &set_eag_ipset_auth_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &set_eag_ipset_auth_cmd);	

	install_element(EAG_NODE, &set_eag_check_nasportid_cmd);
	install_element(HANSI_EAG_NODE, &set_eag_check_nasportid_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &set_eag_check_nasportid_cmd);	

	install_element(EAG_NODE, &set_trap_online_user_num_switch_cmd);
	install_element(HANSI_EAG_NODE, &set_trap_online_user_num_switch_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &set_trap_online_user_num_switch_cmd);

	install_element(EAG_NODE, &set_threshold_online_user_num_cmd);
	install_element(HANSI_EAG_NODE, &set_threshold_online_user_num_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &set_threshold_online_user_num_cmd);

	install_element(EAG_NODE, &set_octets_correct_factor_cmd);
	install_element(HANSI_EAG_NODE, &set_octets_correct_factor_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &set_octets_correct_factor_cmd);

	install_element(EAG_NODE, &set_eag_abnormal_logoff_trap_switch_cmd);
	install_element(HANSI_EAG_NODE, &set_eag_abnormal_logoff_trap_switch_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &set_eag_abnormal_logoff_trap_switch_cmd);

	install_element(EAG_NODE, &set_eag_portal_protocol_cmd);
	install_element(HANSI_EAG_NODE, &set_eag_portal_protocol_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &set_eag_portal_protocol_cmd);

	install_element(EAG_NODE, &set_eag_telecom_idletime_valuecheck_cmd);
	install_element(HANSI_EAG_NODE, &set_eag_telecom_idletime_valuecheck_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &set_eag_telecom_idletime_valuecheck_cmd);
	
	install_element(EAG_NODE, &set_eag_mac_auth_service_status_cmd);
	install_element(HANSI_EAG_NODE, &set_eag_mac_auth_service_status_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &set_eag_mac_auth_service_status_cmd);
	
	install_element(EAG_NODE, &set_eag_mac_auth_ipset_auth_cmd);
	install_element(HANSI_EAG_NODE, &set_eag_mac_auth_ipset_auth_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &set_eag_mac_auth_ipset_auth_cmd);
	
	install_element(EAG_NODE, &set_eag_mac_auth_flux_from_cmd);
	install_element(HANSI_EAG_NODE, &set_eag_mac_auth_flux_from_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &set_eag_mac_auth_flux_from_cmd);

	install_element(EAG_NODE, &set_eag_mac_auth_flux_interval_cmd);
	install_element(HANSI_EAG_NODE, &set_eag_mac_auth_flux_interval_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &set_eag_mac_auth_flux_interval_cmd);

	install_element(EAG_NODE, &set_eag_mac_auth_flux_threshold_cmd);
	install_element(HANSI_EAG_NODE, &set_eag_mac_auth_flux_threshold_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &set_eag_mac_auth_flux_threshold_cmd);

	install_element(EAG_NODE, &set_eag_mac_auth_notice_bindserver_cmd);
	install_element(HANSI_EAG_NODE, &set_eag_mac_auth_notice_bindserver_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &set_eag_mac_auth_notice_bindserver_cmd);

	install_element(EAG_NODE, &eag_service_status_cmd);
	install_element(HANSI_EAG_NODE, &eag_service_status_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &eag_service_status_cmd);

	install_element(EAG_NODE, &show_eag_base_conf_cmd);
	install_element(HANSI_EAG_NODE, &show_eag_base_conf_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &show_eag_base_conf_cmd);

	/* nasid */
	install_element(EAG_NODE, &add_eag_nasid_cmd);
	install_element(HANSI_EAG_NODE, &add_eag_nasid_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &add_eag_nasid_cmd);
	
	install_element(EAG_NODE, &modify_eag_nasid_cmd);
	install_element(HANSI_EAG_NODE, &modify_eag_nasid_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &modify_eag_nasid_cmd);

	install_element(EAG_NODE, &del_eag_nasid_cmd);
	install_element(HANSI_EAG_NODE, &del_eag_nasid_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &del_eag_nasid_cmd);
	
	install_element(EAG_NODE, &show_eag_nasid_cmd);
	install_element(HANSI_EAG_NODE, &show_eag_nasid_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &show_eag_nasid_cmd);

	/* nasportid */
	install_element(EAG_NODE, &add_eag_nasportid_cmd);
	install_element(HANSI_EAG_NODE, &add_eag_nasportid_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &add_eag_nasportid_cmd);
	
	install_element(EAG_NODE, &del_eag_nasportid_cmd);
	install_element(HANSI_EAG_NODE, &del_eag_nasportid_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &del_eag_nasportid_cmd);

	install_element(EAG_NODE, &add_eag_nasportid_wlan_wtp_cmd);
	install_element(HANSI_EAG_NODE, &add_eag_nasportid_wlan_wtp_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &add_eag_nasportid_wlan_wtp_cmd);
	
	install_element(EAG_NODE, &del_eag_nasportid_wlan_wtp_cmd);
	install_element(HANSI_EAG_NODE, &del_eag_nasportid_wlan_wtp_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &del_eag_nasportid_wlan_wtp_cmd);

	install_element(EAG_NODE, &add_eag_nasportid_vlan_cmd);
	install_element(HANSI_EAG_NODE, &add_eag_nasportid_vlan_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &add_eag_nasportid_vlan_cmd);
	
	install_element(EAG_NODE, &del_eag_nasportid_vlan_cmd);
	install_element(HANSI_EAG_NODE, &del_eag_nasportid_vlan_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &del_eag_nasportid_vlan_cmd);
	
	install_element(EAG_NODE, &show_eag_nasportid_cmd);
	install_element(HANSI_EAG_NODE, &show_eag_nasportid_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &show_eag_nasportid_cmd);

	/* statistics */
	install_element(EAG_NODE, &show_bss_statistics_info_cmd);
	install_element(HANSI_EAG_NODE, &show_bss_statistics_info_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &show_bss_statistics_info_cmd);

	install_element(EAG_NODE, &show_ap_statistics_info_cmd);
	install_element(HANSI_EAG_NODE, &show_ap_statistics_info_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &show_ap_statistics_info_cmd);

	install_element(EAG_NODE, &show_eag_statistics_info_cmd);
	install_element(HANSI_EAG_NODE, &show_eag_statistics_info_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &show_ap_statistics_info_cmd);
	
	/*multi portal*/
	install_element(EAG_NODE, &eag_add_portal_server_cmd);
	install_element(HANSI_EAG_NODE, &eag_add_portal_server_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &eag_add_portal_server_cmd);
	
	install_element(EAG_NODE, &eag_add_portal_server_cmd_domain);
	install_element(HANSI_EAG_NODE, &eag_add_portal_server_cmd_domain);
	install_element(LOCAL_HANSI_EAG_NODE, &eag_add_portal_server_cmd_domain);

	install_element(EAG_NODE, &eag_add_portal_server_cmd_macauth);
	install_element(HANSI_EAG_NODE, &eag_add_portal_server_cmd_macauth);
	install_element(LOCAL_HANSI_EAG_NODE, &eag_add_portal_server_cmd_macauth);

	install_element(EAG_NODE, &eag_set_macbind_server_cmd);
	install_element(HANSI_EAG_NODE, &eag_set_macbind_server_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &eag_set_macbind_server_cmd);

	install_element(EAG_NODE, &eag_set_macbind_server_essid_cmd);
	install_element(HANSI_EAG_NODE, &eag_set_macbind_server_essid_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &eag_set_macbind_server_essid_cmd);

	install_element(EAG_NODE, &eag_add_portal_server_cmd_domain_macauth);
	install_element(HANSI_EAG_NODE, &eag_add_portal_server_cmd_domain_macauth);
	install_element(LOCAL_HANSI_EAG_NODE, &eag_add_portal_server_cmd_domain_macauth);

	install_element(EAG_NODE, &eag_add_portal_server_essid_cmd);
	install_element(HANSI_EAG_NODE, &eag_add_portal_server_essid_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &eag_add_portal_server_essid_cmd);
	
	install_element(EAG_NODE, &eag_modify_portal_server_cmd);
	install_element(HANSI_EAG_NODE, &eag_modify_portal_server_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &eag_modify_portal_server_cmd);

	install_element(EAG_NODE, &eag_modify_portal_server_cmd_domain);
	install_element(HANSI_EAG_NODE, &eag_modify_portal_server_cmd_domain);
	install_element(LOCAL_HANSI_EAG_NODE, &eag_modify_portal_server_cmd_domain);
	
	install_element(EAG_NODE, &eag_modify_portal_server_cmd_macauth);
	install_element(HANSI_EAG_NODE, &eag_modify_portal_server_cmd_macauth);
	install_element(LOCAL_HANSI_EAG_NODE, &eag_modify_portal_server_cmd_macauth);
	
	install_element(EAG_NODE, &eag_modify_portal_server_cmd_domain_macauth);
	install_element(HANSI_EAG_NODE, &eag_modify_portal_server_cmd_domain_macauth);
	install_element(LOCAL_HANSI_EAG_NODE, &eag_modify_portal_server_cmd_domain_macauth);

	install_element(EAG_NODE, &eag_modify_portal_server_essid_cmd);
	install_element(HANSI_EAG_NODE, &eag_modify_portal_server_essid_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &eag_modify_portal_server_essid_cmd);

	install_element(EAG_NODE, &eag_del_portal_server_cmd);
	install_element(HANSI_EAG_NODE, &eag_del_portal_server_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &eag_del_portal_server_cmd);

	install_element(EAG_NODE, &eag_del_portal_conf_essid_cmd);
	install_element(HANSI_EAG_NODE, &eag_del_portal_conf_essid_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &eag_del_portal_conf_essid_cmd);
	
	install_element(EAG_NODE, &eag_show_portal_conf_cmd);
	install_element(HANSI_EAG_NODE, &eag_show_portal_conf_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &eag_show_portal_conf_cmd);

	install_element(EAG_NODE, &set_eag_portal_server_essid_cmd);
	install_element(HANSI_EAG_NODE, &set_eag_portal_server_essid_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &set_eag_portal_server_essid_cmd);	

	install_element(EAG_NODE, &set_eag_portal_server_acname_cmd);
	install_element(HANSI_EAG_NODE, &set_eag_portal_server_acname_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &set_eag_portal_server_acname_cmd);	
	
	install_element(EAG_NODE, &set_eag_portal_server_acip_to_url_cmd);
	install_element(HANSI_EAG_NODE, &set_eag_portal_server_acip_to_url_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &set_eag_portal_server_acip_to_url_cmd);	
	
	install_element(EAG_NODE, &set_eag_portal_server_nasid_to_url_cmd);
	install_element(HANSI_EAG_NODE, &set_eag_portal_server_nasid_to_url_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &set_eag_portal_server_nasid_to_url_cmd);	

	install_element(EAG_NODE, &set_eag_portal_server_wlanparameter_cmd );
	install_element(HANSI_EAG_NODE, &set_eag_portal_server_wlanparameter_cmd );
	install_element(LOCAL_HANSI_EAG_NODE, &set_eag_portal_server_wlanparameter_cmd );

	install_element(EAG_NODE, &set_eag_portal_server_wlanuserfirsturl_cmd );
	install_element(HANSI_EAG_NODE, &set_eag_portal_server_wlanuserfirsturl_cmd );
	install_element(LOCAL_HANSI_EAG_NODE, &set_eag_portal_server_wlanuserfirsturl_cmd );

	install_element(EAG_NODE, &set_eag_portal_server_url_suffix_cmd);
	install_element(HANSI_EAG_NODE, &set_eag_portal_server_url_suffix_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &set_eag_portal_server_url_suffix_cmd);

	install_element(EAG_NODE, &set_eag_portal_server_secret_cmd);
	install_element(HANSI_EAG_NODE, &set_eag_portal_server_secret_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &set_eag_portal_server_secret_cmd);

	install_element(EAG_NODE, &set_eag_portal_server_wlanapmac_cmd );
	install_element(HANSI_EAG_NODE, &set_eag_portal_server_wlanapmac_cmd );
	install_element(LOCAL_HANSI_EAG_NODE, &set_eag_portal_server_wlanapmac_cmd );

	install_element(EAG_NODE, &set_eag_portal_server_usermac_to_url_cmd);
	install_element(HANSI_EAG_NODE, &set_eag_portal_server_usermac_to_url_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &set_eag_portal_server_usermac_to_url_cmd );

	install_element(EAG_NODE, &set_eag_portal_server_clientmac_to_url_cmd);
	install_element(HANSI_EAG_NODE, &set_eag_portal_server_clientmac_to_url_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &set_eag_portal_server_clientmac_to_url_cmd );
	
	install_element(EAG_NODE, &set_eag_portal_server_apmac_to_url_cmd);
	install_element(HANSI_EAG_NODE, &set_eag_portal_server_apmac_to_url_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &set_eag_portal_server_apmac_to_url_cmd );
	
	install_element(EAG_NODE, &set_eag_portal_server_wlan_to_url_cmd);
	install_element(HANSI_EAG_NODE, &set_eag_portal_server_wlan_to_url_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &set_eag_portal_server_wlan_to_url_cmd );
	
	install_element(EAG_NODE, &set_eag_portal_server_redirect_to_url_cmd);
	install_element(HANSI_EAG_NODE, &set_eag_portal_server_redirect_to_url_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &set_eag_portal_server_redirect_to_url_cmd );
	
	install_element(EAG_NODE, &set_eag_portal_server_wlanusermac_cmd);
	install_element(HANSI_EAG_NODE, &set_eag_portal_server_wlanusermac_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &set_eag_portal_server_wlanusermac_cmd);
	
	install_element(EAG_NODE, &set_eag_portal_server_wisprlogin_cmd);
	install_element(HANSI_EAG_NODE, &set_eag_portal_server_wisprlogin_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &set_eag_portal_server_wisprlogin_cmd);

	install_element(EAG_NODE, &set_eag_portal_server_mobile_urlparam_cmd);
	install_element(HANSI_EAG_NODE, &set_eag_portal_server_mobile_urlparam_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &set_eag_portal_server_mobile_urlparam_cmd);

	//install_element(EAG_NODE, &show_eag_portal_server_urlparam_cmd);
	//install_element(HANSI_EAG_NODE, &show_eag_portal_server_urlparam_cmd);
	//install_element(LOCAL_HANSI_EAG_NODE, &show_eag_portal_server_urlparam_cmd);
	
	install_element(EAG_NODE, &set_eag_portal_server_urlparam_cmd);
	install_element(HANSI_EAG_NODE, &set_eag_portal_server_urlparam_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &set_eag_portal_server_urlparam_cmd);
	/*multi radius*/
	install_element(EAG_NODE, &eag_add_radius_server_cmd);
	install_element(HANSI_EAG_NODE, &eag_add_radius_server_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &eag_add_radius_server_cmd);	
	
	install_element(EAG_NODE, &eag_add_radius_server_cmd_hasbackup);
	install_element(HANSI_EAG_NODE, &eag_add_radius_server_cmd_hasbackup);
	install_element(LOCAL_HANSI_EAG_NODE, &eag_add_radius_server_cmd_hasbackup);	
	
	install_element(EAG_NODE, &eag_modify_radius_server_cmd);
	install_element(HANSI_EAG_NODE, &eag_modify_radius_server_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &eag_modify_radius_server_cmd);	
	
	install_element(EAG_NODE, &eag_modify_radius_server_cmd_hasbackup);
	install_element(HANSI_EAG_NODE, &eag_modify_radius_server_cmd_hasbackup);
	install_element(LOCAL_HANSI_EAG_NODE, &eag_modify_radius_server_cmd_hasbackup);	
	
	install_element(EAG_NODE, &eag_del_radius_server_cmd);	
	install_element(HANSI_EAG_NODE, &eag_del_radius_server_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &eag_del_radius_server_cmd);

	install_element(EAG_NODE, &eag_set_radius_remove_domainname_cmd);	
	install_element(HANSI_EAG_NODE, &eag_set_radius_remove_domainname_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &eag_set_radius_remove_domainname_cmd);
	
	install_element(EAG_NODE, &eag_set_radius_class_to_bandwidth_cmd);	
	install_element(HANSI_EAG_NODE, &eag_set_radius_class_to_bandwidth_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &eag_set_radius_class_to_bandwidth_cmd);
	
	install_element(EAG_NODE, &eag_show_radius_conf_cmd);
	install_element(HANSI_EAG_NODE, &eag_show_radius_conf_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &eag_show_radius_conf_cmd);	
	
	/*captive-portal*/
	install_element(EAG_NODE, &add_captive_portal_intfs_cmd);
	install_element(HANSI_EAG_NODE, &add_captive_portal_intfs_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &add_captive_portal_intfs_cmd);
	
	install_element(EAG_NODE, &del_captive_portal_intfs_cmd);
	install_element(HANSI_EAG_NODE, &del_captive_portal_intfs_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &del_captive_portal_intfs_cmd);	
	
	install_element(EAG_NODE, &show_captive_portal_intfs_cmd);
	install_element(HANSI_EAG_NODE, &show_captive_portal_intfs_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &show_captive_portal_intfs_cmd);	
	
	install_element(EAG_NODE, &conf_captive_portal_white_list_with_ip_cmd);
	install_element(HANSI_EAG_NODE, &conf_captive_portal_white_list_with_ip_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &conf_captive_portal_white_list_with_ip_cmd);	
	
	install_element(EAG_NODE, &conf_captive_portal_black_list_with_ip_cmd);
	install_element(HANSI_EAG_NODE, &conf_captive_portal_black_list_with_ip_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &conf_captive_portal_black_list_with_ip_cmd);	

	install_element(EAG_NODE, &conf_captive_portal_white_list_with_domain_cmd);
	install_element(HANSI_EAG_NODE, &conf_captive_portal_white_list_with_domain_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &conf_captive_portal_white_list_with_domain_cmd);	
	
	install_element(EAG_NODE, &conf_captive_portal_black_list_with_domain_cmd);
	install_element(HANSI_EAG_NODE, &conf_captive_portal_black_list_with_domain_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &conf_captive_portal_black_list_with_domain_cmd);	

	install_element(EAG_NODE, &conf_captive_portal_white_list_with_ipv6_cmd);
	install_element(HANSI_EAG_NODE, &conf_captive_portal_white_list_with_ipv6_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &conf_captive_portal_white_list_with_ipv6_cmd);	
	
	install_element(EAG_NODE, &conf_captive_portal_black_list_with_ipv6_cmd);
	install_element(HANSI_EAG_NODE, &conf_captive_portal_black_list_with_ipv6_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &conf_captive_portal_black_list_with_ipv6_cmd);	

	install_element(EAG_NODE, &show_captive_portal_white_list_cmd);
	install_element(HANSI_EAG_NODE, &show_captive_portal_white_list_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &show_captive_portal_white_list_cmd);	
	
	install_element(EAG_NODE, &show_captive_portal_black_list_cmd);	
	install_element(HANSI_EAG_NODE, &show_captive_portal_black_list_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &show_captive_portal_black_list_cmd);	
	
	/*for debug*/
	#if 0
	install_element(EAG_NODE, &show_eag_relative_time_cmd);
	install_element(HANSI_EAG_NODE, &show_eag_relative_time_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &show_eag_relative_time_cmd);	
	#endif

	install_element(EAG_NODE, &eag_set_user_log_status_cmd);
	install_element(HANSI_EAG_NODE, &eag_set_user_log_status_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &eag_set_user_log_status_cmd);

	install_element(EAG_NODE, &eag_set_log_format_status_cmd);
	install_element(HANSI_EAG_NODE, &eag_set_log_format_status_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &eag_set_log_format_status_cmd);
	
	install_element(EAG_NODE, &eag_set_username_check_status_cmd);
	install_element(HANSI_EAG_NODE, &eag_set_username_check_status_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &eag_set_username_check_status_cmd);

	install_element(EAG_NODE, &eag_add_debug_filter_cmd);
	install_element(HANSI_EAG_NODE, &eag_add_debug_filter_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &eag_add_debug_filter_cmd);	
	
	install_element(EAG_NODE, &eag_del_debug_filter_cmd);
	install_element(HANSI_EAG_NODE, &eag_del_debug_filter_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &eag_del_debug_filter_cmd);	
	
	install_element(EAG_NODE, &eag_log_all_appconn_cmd);
	install_element(HANSI_EAG_NODE, &eag_log_all_appconn_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &eag_log_all_appconn_cmd);		

	install_element(EAG_NODE, &eag_log_all_portalsess_cmd);
	install_element(HANSI_EAG_NODE, &eag_log_all_portalsess_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &eag_log_all_portalsess_cmd);

	install_element(EAG_NODE, &eag_log_all_sockradius_cmd);
	install_element(HANSI_EAG_NODE, &eag_log_all_sockradius_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &eag_log_all_sockradius_cmd);

	install_element(EAG_NODE, &eag_log_all_redirconn_cmd);
	install_element(HANSI_EAG_NODE, &eag_log_all_redirconn_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &eag_log_all_redirconn_cmd);

	install_element(EAG_NODE, &eag_log_all_thread_cmd);
	install_element(HANSI_EAG_NODE, &eag_log_all_thread_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &eag_log_all_thread_cmd);

	install_element(EAG_NODE, &eag_log_all_blkmem_cmd);
	install_element(HANSI_EAG_NODE, &eag_log_all_blkmem_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &eag_log_all_blkmem_cmd);

	install_element(EAG_NODE, &eag_log_all_mac_preauth_cmd);
	install_element(HANSI_EAG_NODE, &eag_log_all_mac_preauth_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &eag_log_all_mac_preauth_cmd);

	install_element(EAG_NODE, &eag_set_rdc_client_log_cmd);
	install_element(HANSI_EAG_NODE, &eag_set_rdc_client_log_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &eag_set_rdc_client_log_cmd);

	install_element(EAG_NODE, &eag_set_pdc_client_log_cmd);
	install_element(HANSI_EAG_NODE, &eag_set_pdc_client_log_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &eag_set_pdc_client_log_cmd);
	
	/*for eag station */
	install_element(EAG_NODE, &eag_show_user_log_cmd);
	install_element(HANSI_EAG_NODE, &eag_show_user_log_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &eag_show_user_log_cmd);
	
	install_element(EAG_NODE, &eag_show_user_list_cmd);
	install_element(HANSI_EAG_NODE, &eag_show_user_list_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &eag_show_user_list_cmd);

	install_element(EAG_NODE, &eag_show_user_by_username_cmd);
	install_element(HANSI_EAG_NODE, &eag_show_user_by_username_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &eag_show_user_by_username_cmd);

	install_element(EAG_NODE, &eag_show_user_by_userip_cmd);
	install_element(HANSI_EAG_NODE, &eag_show_user_by_userip_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &eag_show_user_by_userip_cmd);

	install_element(EAG_NODE, &eag_show_user_by_usermac_cmd);
	install_element(HANSI_EAG_NODE, &eag_show_user_by_usermac_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &eag_show_user_by_usermac_cmd);

	install_element(EAG_NODE, &eag_show_user_by_index_cmd);
	install_element(HANSI_EAG_NODE, &eag_show_user_by_index_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &eag_show_user_by_index_cmd);

	install_element(EAG_NODE, &eag_kick_user_by_username_cmd);
	install_element(HANSI_EAG_NODE, &eag_kick_user_by_username_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &eag_kick_user_by_username_cmd);

	install_element(EAG_NODE, &eag_kick_user_by_userip_cmd);
	install_element(HANSI_EAG_NODE, &eag_kick_user_by_userip_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &eag_kick_user_by_userip_cmd);

	install_element(EAG_NODE, &eag_kick_user_by_usermac_cmd);
	install_element(HANSI_EAG_NODE, &eag_kick_user_by_usermac_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &eag_kick_user_by_usermac_cmd);

	install_element(EAG_NODE, &eag_kick_user_by_index_cmd);
	install_element(HANSI_EAG_NODE, &eag_kick_user_by_index_cmd);
	install_element(LOCAL_HANSI_EAG_NODE, &eag_kick_user_by_index_cmd);

#if 1
	/* add methods of debug for eag */
	install_element(HIDDENDEBUG_NODE,&config_eag_debug_cmd);
	install_element(HIDDENDEBUG_NODE,&config_eag_no_debug_cmd);
	install_element(HIDDENDEBUG_NODE,&config_eag_debug_pkt_info_cmd);
	install_element(HIDDENDEBUG_NODE,&config_eag_no_debug_pkt_info_cmd);
#endif	
}

static int get_portal_param_by_portal_url
(
char *portal_url, 
char *url_prefix,
int url_prefix_len,
int *ip_addr, 
int *port, 
char * web_page,
int web_page_len
)
{
	if (	NULL == portal_url	|| NULL == ip_addr 
		||	NULL == port 		|| NULL == web_page
		||	NULL == url_prefix	|| 0 >= web_page_len
		||	0 >= url_prefix_len)
	{
		//log_err("param err!\n");
		return -1;
	}
	
	unsigned int ip_part_value = -1;
	int ip_part_num = -1;
	int i = 0;
	
	char *p_url = NULL;	
	p_url = portal_url;
	
	if ( 0 == strncmp(p_url,"http://",strlen("http://")))
	{
		strncpy(url_prefix,"http://", url_prefix_len);
		p_url += strlen("http://");
	}
	else if( 0 == strncmp(p_url,"https://",strlen("https://")))
	{
		strncpy(url_prefix,"https://", url_prefix_len);
		p_url += strlen("https://");
	}
	else
	{
		//log_err("url err! url must start as http:// or https:// \n");
		return -1;
	}
	
	/* get ip */
	ip_part_value = -1;
	ip_part_num = 1;
	*ip_addr = 0;
	
	for(i=0; i<17; i++)
	{
		if('\0' == *p_url)
		{
			//log_err("url ip err!\n");
			return -1;
		}
		else if ('/' == *p_url || ':' == *p_url)
		{
			if (0 <= ip_part_value && 255 >= ip_part_value && 4 == ip_part_num)
			{
				/* success */
				*ip_addr += ip_part_value;
				break;
			}
			else
			{
				//log_err("url ip err!\n");
				return -1;
			}
		}
		else if ('.' == *p_url)
		{
			if (		((1 == ip_part_num && 0 < ip_part_value) || (1 < ip_part_num && 0 <= ip_part_value))
					&&	255 >= ip_part_value
					&&	4 > ip_part_num)
			{
				/* legal */
				*ip_addr += ip_part_value << ((4-ip_part_num)*8);
				ip_part_value = -1;
				ip_part_num++;
			}
			else
			{
				//log_err("url ip err!\n");
				return -1;
			}
		}
		else if ('0' <= *p_url || '9' >= *p_url)
		{
			if (-1 == ip_part_value)
			{
				ip_part_value = 0;
			}
			else
			{
				ip_part_value *= 10;
			}
			
			ip_part_value += (*p_url - '0');
		}
		else
		{
			//log_err("url ip err!\n");
			return -1;
		}
		p_url++;
	}
	
	
	/* get port */
	*port = 0;
	if (':' == *p_url)
	{
		p_url++;
		while (NULL != p_url && '0' <= *p_url && '9' >= *p_url)
		{
			*port *= 10;
			*port += (*p_url - '0');			
			p_url++;
		}

		if (65535 < *port || 0 >= *port)
		{
				//log_err("url port err!\n");
				return -1;
		}		
	}
	else if ('/' == *p_url)
	{
		*port = 80;		
	}
	else
	{
		//log_err("url port err!\n");
		return -1;
	}
	
	/* get web page */
	if ('/' != *p_url)	
	{
		//log_err("web page err!\n");
		return -1;
	}
	else
	{		
		strncpy(web_page,p_url,web_page_len);
	}
	
	return 1;
}

