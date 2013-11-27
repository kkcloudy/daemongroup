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
* dcli_captive.c
*
* MODIFY:
*		by <shaojw@autelan.com> on 2010-1-28 15:09:03 revision <0.1>
		by <chensheng@autelan.com> on 2010-3-15
*
* CREATOR:
*		qinhs@autelan.com
*
* DESCRIPTION:
*		CLI definition for captive portal module.
*
* DATE:
*		2010-1-28 15:09:11
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.17 $	
*******************************************************************************/

#ifdef __cplusplus
extern "C"
{
#endif
#include <zebra.h>
#include <dbus/dbus.h>
#include <stdlib.h>
#include <sysdef/npd_sysdef.h>
#include <dbus/npd/npd_dbus_def.h>
#include <string.h>
#include <stdio.h>
#include "dcli_main.h"
#include "vty.h"
#include "command.h"
#include "if.h"
#include "sysdef/returncode.h"
#include "dcli_captive.h"
#include "ws_public.h"

struct cmd_node captive_node = 
{
	CAPTIVE_NODE,
	"%s(captive-portal)# "
};

typedef struct cp_if_t *cp_if_link;

struct cp_if_t{
	char *if_name;
	cp_if_link next;
};

inline cp_if_link CP_IF_NODE(char *if_name, cp_if_link next)
{
	cp_if_link t = malloc(sizeof(*t));
	t->if_name = if_name;
	t->next = next;
	return t;
}

inline cp_if_link cp_if_list_init(void) 
{
	return CP_IF_NODE(NULL, NULL); 
}

inline void cp_if_list_destroy(cp_if_link head)
{
	cp_if_link t, x;
	for (t=head; t; t=t->next, free(x)) x = t;
}

inline void cp_if_list_insert_node(cp_if_link head, char *if_name)
{
	head->next = CP_IF_NODE(if_name, head->next);
}
	
inline int cp_if_list_find_node(cp_if_link head, char *if_name)
{
	cp_if_link t;
	for (t=head->next; t; t=t->next)
		if (strcmp(t->if_name, if_name) == 0)
			break;
	return t != NULL;
}

/* use macro rather than function, because the vty variable in DEFUN is needed. */
#define show_captive_portal_info(record) \
do { \
	int cur_id; \
	char ip_addr[16], if_info[256], strPort[10], *p=NULL, *token=NULL; \
	memset(ip_addr, 0, sizeof(ip_addr)); \
	memset(if_info, 0, sizeof(if_info)); \
	for (p = record; *p; p++) \
		if ('\r' == *p || '\n' == *p){ \
			*p = '\0'; \
			break; \
		} \
	sscanf(record, "%d	%s	%s %*s %*s %s", &cur_id, ip_addr, strPort, if_info); \
	vty_out(vty, "captive portal %d\n", cur_id); \
	vty_out(vty, "==================================================\n"); \
	vty_out(vty, "captive portal id : %d\n", cur_id); \
	vty_out(vty, "server ip         : %s\n", ip_addr); \
	vty_out(vty, "server port       : %s\n", strPort); \
	vty_out(vty, "interface         :"); \
	for (token = strtok(if_info, ","); token; token = strtok(NULL, ",")) \
		vty_out(vty, " %s", token); \
	vty_out(vty, "\n"); \
	vty_out(vty, "==================================================\n"); \
}while(0)

int parse_captive_portal_id(char *str, int *id) 
{
	*id = -1;
	if (NULL == str || '\0' == str[0]) return CAPTIVE_FAILURE;
	if ('\0' == str[1] && str[0] >= '0' && str[0] <= '7'){
		*id = str[0] - '0';
		return CAPTIVE_SUCCESS;
	}
	else
		return CAPTIVE_FAILURE;
}

int getRecordById( int id, char *record, int len )
{
	FILE *db;
	
	db= fopen( PORTAL_CONF_PATH,"r");
	
	if( NULL == record )
		return 0;
	
	if( id < 0 || id > 7 )
	{
		return 0;
	}
	
	if( NULL == db )
	{
		*record = 0;
		return 0;	
	}
	
	while( fgets( record, len, db ) )
	{
		if( id == record[0]-'0' )
		{
			fclose( db );
			return 1;
		}
	}
	
	*record = 0;
	fclose( db );
	return 0;
}

/* This function is copyed from WID_Check_IP_Format() in wid_wtp.h . */
int captive_check_ip_format(const char *str)
{
	char *endptr = NULL;
	char* endptr1 = NULL;
	char c;
	int IP,i;
	
	c = str[0];
	if (c>='0'&&c<='9'){
		IP= strtoul(str,&endptr,10);
		if(IP < 0||IP > 255)
			return CAPTIVE_IP_CHECK_FAILURE;
		else if(((IP < 10)&&((endptr - str) > 1))||((IP < 100)&&((endptr - str) > 2))||((IP < 256)&&((endptr - str) > 3)))
			return CAPTIVE_IP_CHECK_FAILURE;
		for(i = 0; i < 3; i++){
			if(endptr[0] == '\0'||endptr[0] != '.')
				return CAPTIVE_IP_CHECK_FAILURE;
			else{
				endptr1 = &endptr[1];
				IP= strtoul(&endptr[1],&endptr,10); 
				if((2 == i) && (0 == IP))//the last IP value can't be 0.
					return CAPTIVE_IP_CHECK_FAILURE;
				if(IP < 0||IP > 255)
					return CAPTIVE_IP_CHECK_FAILURE;				
				else if(((IP < 10)&&((endptr - endptr1) > 1))||((IP < 100)&&((endptr - endptr1) > 2))||((IP < 256)&&((endptr - endptr1) > 3)))
					return CAPTIVE_IP_CHECK_FAILURE;
			}
		}
		if(endptr[0] == '\0' && IP >= 0)
			return CAPTIVE_IP_CHECK_SUCCESS;
		else
			return CAPTIVE_IP_CHECK_FAILURE;
	}
	else
		return CAPTIVE_IP_CHECK_FAILURE;		
}

int captive_check_ipv6_format(const char *str)
{
	struct in6_addr ipv6;
	memset(&ipv6, 0, sizeof(ipv6));
	if(!inet_pton(AF_INET6, str, &ipv6)) {
		return CAPTIVE_IP_CHECK_FAILURE;
	}
    return CAPTIVE_IP_CHECK_SUCCESS;
}

int captive_check_interfaces_format(const char *str, char *err, int size)
{
	char if_tmp[256], record[256], buf_if[256];
	char *token_if_tmp = NULL, *save_if_tmp = NULL, *token_buf_if = NULL, *save_buf_if = NULL;
	FILE *fp = NULL;
	infi  interf;
	infi *q = NULL;
	int cp_id;
	cp_if_link head = NULL;

	memset(if_tmp, 0, sizeof(if_tmp));
	memset(record, 0, sizeof(record));
	memset(buf_if, 0, sizeof(buf_if));
	strncpy(if_tmp, str, sizeof(if_tmp)-1);

	const char *p = NULL;
	int fmt_ok = 1;
	if (NULL ==  str || '\0' == str[0] || ',' == str[0])
		fmt_ok = 0;
	for (p = str; *p; p++)
		if (',' == *p && ',' == *(p+1))
			fmt_ok = 0;
	if (',' == *(p-1))
		fmt_ok = 0;
	if (!fmt_ok){
		strncpy(err, "interfaces formated wrongly", size);
			return CAPTIVE_INTERFACES_CHECK_FAILURE;
	}
	
	head = cp_if_list_init();
	for (token_if_tmp = strtok_r(if_tmp, ",", &save_if_tmp); token_if_tmp; token_if_tmp = strtok_r(NULL, ",", &save_if_tmp)){
		if (strcmp(token_if_tmp, "lo") == 0){
			strncpy(err, "lo isn't a physical interface", size);
			return CAPTIVE_INTERFACES_CHECK_FAILURE;
		}

		interface_list_ioctl(0, &interf);
		for (q = interf.next; q; q = q->next)
			if (strcmp(q->if_name, token_if_tmp) == 0) break;
		if (NULL == q){
			snprintf(err, size, "interface %s not exist or no ip address", token_if_tmp);
			return CAPTIVE_INTERFACES_CHECK_FAILURE;
		}
		
		if ( (fp = fopen(PORTAL_CONF_PATH, "r")) != NULL){
			while (fgets(record, sizeof(record), fp) != NULL){
				sscanf(record ,"%d %*s %*s %*s %*s %s", &cp_id, buf_if);
				for (token_buf_if = strtok_r(buf_if, ",", &save_buf_if); token_buf_if; token_buf_if = strtok_r(NULL, ",", &save_buf_if))
					if (strcmp(token_buf_if, token_if_tmp) == 0){
						snprintf(err, size, "interface %s has been used by captive portal %d", token_if_tmp, cp_id);
						return CAPTIVE_INTERFACES_CHECK_FAILURE;
					}
			}
		}

		if (cp_if_list_find_node(head, token_if_tmp)){
			snprintf(err, size, "interface %s is duplicated", token_if_tmp);
			return CAPTIVE_INTERFACES_CHECK_FAILURE;
		}
		else
			cp_if_list_insert_node(head, token_if_tmp);
	}

	cp_if_list_destroy(head);

	return CAPTIVE_INTERFACES_CHECK_SUCCESS;
}

int captive_port_is_legal_input(const char *str)
{
	const char *p = NULL;
	int port;
	
	if (NULL == str || '\0' == str[0] || '0' == str[0])
		return 0;

	for (p = str; *p; p++)
		if (*p < '0' || *p > '9')
			return 0;
		
	port = atoi(str);
	if (port < 0 || port > 65535)
		return 0;
	
	return 1;
}

/* This function is copyed from mac_format_check in dcli_fdb.c . */

int captive_check_mac_format
(
	char* str,
	int len
) 
{
	int i = 0;
	unsigned int result = CAPTIVE_MAC_CHECK_SUCESS;
	char c = 0;
	
	if( 17 != len){
	   return CAPTIVE_MAC_CHECK_FAILURE;
	}
	for(;i<len;i++) {
		c = str[i];
		if((2 == i)||(5 == i)||(8 == i)||(11 == i)||(14 == i)){
			if((':'!=c)&&('-'!=c))
				return CAPTIVE_MAC_CHECK_FAILURE;
		}
		else if((c>='0'&&c<='9')||
			(c>='A'&&c<='F')||
			(c>='a'&&c<='f'))
			continue;
		else {
			result = CAPTIVE_MAC_CHECK_FAILURE;
			return result;
		}
    }
	if((str[2] != str[5])||(str[2] != str[8])||(str[2] != str[11])||(str[2] != str[14])||
		(str[5] != str[8])||(str[5] != str[11])||(str[5] != str[14])||
		(str[8] != str[11])||(str[8] != str[14])){
		
        result = CAPTIVE_MAC_CHECK_FAILURE;
		return result;
	}
	return result;
}



DEFUN(conf_captive_portal_func,
	conf_captive_portal_cmd,
	"config captive-portal <0-7>",
	CONFIG_STR
	"config which interface will use portal, and set the redir listen server ip and port\n" 
	"specified ID for you want to config\n"
)
{
	unsigned int cp_id = -1;
	
	cp_id = strtoul(argv[0],NULL,10);
	
	//vty_out(vty,"for test! cp_id = %d\n", cp_id );
	
	if(CONFIG_NODE == vty->node) {
		vty->node = CAPTIVE_NODE;
		vty->index = (void*)cp_id;
	}
	else
	{
		vty_out (vty, "Terminal mode change must under configure mode!\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
}


DEFUN(show_captive_portal_func,
	show_captive_portal_cmd,
	"show captive-portal [<0-7>]",
	SHOW_STR
	"config which interface will use portal, and set the redir listen server port\n" 
	"specified ID of captive portal\n"
)
{
	//vty_out(vty,"for test!!!!!  argc = %d\n", argc);
	int ret, id;
	char record[256];

	memset(record, 0, sizeof(record));
	if (0 == argc){
		int cur_id, i, num = 0;
		char ip_addr[16], if_info[256], strPort[10], *p=NULL, *token=NULL;

		memset(ip_addr, 0, sizeof(ip_addr));
		memset(if_info, 0, sizeof(if_info));
		for (i = 0; i <= 7; i++)
			if (getRecordById(i, record, sizeof(record)) == 1) num++;

		vty_out(vty, "captive portal list summary:\n");	
    	vty_out(vty, "%d captive portal exist\n", num);
    	vty_out(vty, "========================================================================\n");
		vty_out(vty, "%-16s %-16s %-16s %s\n", "ID", "ServIP", "ServPort", "Interface");

		for (i = 0; i <= 7; i++)
			if (getRecordById(i, record, sizeof(record)) == 1){
				for (p = record; *p; p++)
					if ('\r' == *p || '\n' == *p){
						*p = '\0';
						break;
					}
				sscanf(record, "%d	%s	%s %*s %*s %s", &cur_id, ip_addr, strPort, if_info);
				vty_out(vty,"%-16d %-16s %-16s", cur_id, ip_addr, strPort);
				for (token = strtok(if_info, ","); token; token = strtok(NULL, ","))
					vty_out(vty, " %s", token);
				vty_out(vty, "\n");
			}
		vty_out(vty, "========================================================================\n");
		
		return CMD_SUCCESS;
	} 
	else {
		ret = parse_captive_portal_id((char *)argv[0], &id);
		if (CAPTIVE_SUCCESS != ret){
			vty_out(vty, "captive portal id must be <0-7>!\n");
			return CMD_WARNING;
		}
		ret = getRecordById(id, record, sizeof(record));
		if (1 == ret)
			show_captive_portal_info(record);
		else
			vty_out(vty, "Captive portal %d not configured!\n", id);
	}
	
	return CMD_SUCCESS;
}


DEFUN(show_cur_captive_portal_func,
	show_cur_captive_portal_cmd,
	"show captive-portal-config",
	SHOW_STR
	"show config of current id\n" 
)
{
	//vty_out(vty,"for test!!!!!\n");
	int ret, id;
	char record[256];

	memset(record, 0, sizeof(record));
	id = (int)(vty->index);
	if (id < 0 || id > 7){
		vty_out(vty, "captive portal id must be <0-7>!\n");
		return CMD_WARNING;
	}
	ret = getRecordById(id, record, sizeof(record));
	if (1 == ret)
		show_captive_portal_info(record);
	else
		vty_out(vty, "Captive portal %d not configured!\n", id);

	return CMD_SUCCESS;
}


DEFUN(set_cur_captive_portal_param_func,
	set_cur_captive_portal_param_cmd,
	"set params A.B.C.D PORTNO INTERFACES",
	"set param of this captive_portal\n"
	"set param of this captive_portal\n"
	"set redir server ipaddress!\n"
	"set redir server port, it's TCP protocal port no.!\n"
	"set interfaces which will use portal, with format if_name[,if_name]...\n"
)
{
	//vty_out(vty,"for test!!!!!  argc=%d\n", argc);
	//vty_out(vty,"for test!!!!!  cp_id=%d\n", (unsigned int)(vty->index));
	int ret, rec, cur_id;
	char ip_addr[16], interfaces[256], strPort[10], command[256], usrname[20], err[100];

	memset(ip_addr, 0, sizeof(ip_addr));
	memset(interfaces, 0, sizeof(interfaces));
	memset(command, 0, sizeof(command));
	memset(usrname, 0, sizeof(usrname));
	memset(err, 0, sizeof(err));

	/* the ip address seem to have been checked with an error prompt "% Unknown command.", so the code segment below is like unuseful.  */
	ret = captive_check_ip_format((char*)(argv[0]));
	//vty_out(vty, "ret = %d\n", ret);
	if(ret != CAPTIVE_IP_CHECK_SUCCESS){
		vty_out(vty, "<error> unknown ip format\n");
		return CMD_WARNING;
	}

	if (strcmp(argv[1], "default") != 0 && !captive_port_is_legal_input(argv[1])){	
		vty_out(vty, "<error> port no. must range <0-65535>, default 3990\n");
		return CMD_WARNING; 	
	}
	
	ret = captive_check_interfaces_format((char *)argv[2], err, sizeof(err)-1);
	if (CAPTIVE_INTERFACES_CHECK_SUCCESS != ret){
		vty_out(vty, "%s\n", err);
		return CMD_WARNING;
	}

	cur_id = (int)(vty->index);
	if (cur_id < 0 || cur_id > 7){
		vty_out(vty, "captive portal id must range <0-7>!\n");
		return CMD_WARNING;
	}
	
	strncpy(ip_addr, argv[0], sizeof(ip_addr));
	strncpy(strPort, argv[1], sizeof(strPort));
	strncpy(interfaces, argv[2], sizeof(interfaces));
	snprintf(usrname, sizeof(usrname), "usr%d", cur_id);
	sprintf( command, "sudo cp_create_profile.sh %d %s %s %s %s >/dev/null 2>&1", 
							cur_id, ip_addr, strPort, usrname, "password" );
	rec = system(command);
 	ret = WEXITSTATUS(rec);

	switch( ret )
	{
		case 0:
			break;
		case 3:
			vty_out(vty, "The ID has been setted!\n");
			break;
		case 4:
			vty_out(vty, "IP address is an other portal server!\n");
			break;
		case 5:
			vty_out(vty, "The user has been used!\n");
			break;
		case 1:
		case 2:
		default:
			vty_out(vty, "Unknown Error!\n");
			break;
	}
	if( 0 != rec )
		return CMD_FAILURE;

	sprintf(command, "sudo cp_apply_if.sh %d %s >/dev/null 2>&1", cur_id, interfaces);
 	rec = system(command);
	ret = WEXITSTATUS(rec);
	if(rec == 0)
	{
		vty_out(vty, "setting sucessfull!\n");
		return CMD_SUCCESS;
	}
	else
		vty_out(vty, "setting failed!\n");
	
	return CMD_FAILURE;
}

DEFUN(clear_captive_portal_func,
	clear_captive_portal_cmd,
	"clear captive-portal <0-7>",
	CLEAR_STR
	"config which interface will use portal, and set the redir listen server ip and port!\n" 
	"special ID for you want to clear!\n"
)
{
	int cp_id, ret, status;
	char command[256], record[256];

	memset(command, 0, sizeof(command));
	memset(record, 0, sizeof(record));
	cp_id = strtoul(argv[0], NULL, 10);
	//vty_out(vty,"for test! cp_id = %d\n", cp_id );
	ret = getRecordById(cp_id, record, sizeof(record));
	if (0 == ret){
		vty_out(vty, "captive portal %d has't been configured!\n", cp_id);
		return CMD_FAILURE;
	}
	
	sprintf(command, "sudo cp_del_portal_id.sh %d > /dev/null 2>&1", cp_id);
	//vty_out(vty, "cmd = %s\n", command);
	status = system(command); 	 
	ret = WEXITSTATUS(status);	
	
	//return ret;
	vty_out(vty, "clear captive portal %d successed!\n", cp_id);
	return CMD_SUCCESS;
}


/****** white list ******/
int captive_check_portset_format(const char *str)
{
	const char *p = NULL;
	char *tmp = NULL, *token = NULL;
	
	if (NULL == str || '\0' == str[0] || ',' == str[0]) 
		return CAPTIVE_FAILURE;

	if (strlen(str) > PORTSET_LEN -1)
		return CAPTIVE_FAILURE;
	
	if (strcmp(str, "all") == 0)
		return CAPTIVE_SUCCESS;
	
	for (p = str; *p; p++){
		if ((*p < '0' || *p > '9') && *p != ',')
			return CAPTIVE_FAILURE;
		if (',' == *p && ',' == *(p+1))
			return CAPTIVE_FAILURE;
	}
	if (',' ==  *(p-1))
		return CAPTIVE_FAILURE;

	tmp = strdup(str);
	for (token = strtok(tmp, ","); token; token = strtok(NULL, ",")){
		int port = atoi(token);
		if (port <= 0 || port > 65535 || '0' == token[0]){
			free(tmp);
			return CAPTIVE_FAILURE;
		}
	}

	free(tmp);
	return CAPTIVE_SUCCESS;
	
}

int parse_iprange_portset(const char *str, iprange_portset_t *item)
{
	const char *p1 = NULL, *p2 = NULL;
	char ipaddr_begin[IP_ADDR_LEN], ipaddr_end[IP_ADDR_LEN], portset[PORTSET_LEN]; 

	memset(ipaddr_begin, 0, sizeof(ipaddr_begin));
	memset(ipaddr_end, 0, sizeof(ipaddr_end));
	memset(portset, 0, sizeof(portset));
	p1 = str;
	if ( (p2 = strchr(p1, '-')) != NULL){
		strncpy(ipaddr_begin, p1, p2-p1);
		if (captive_check_ip_format(ipaddr_begin) != CAPTIVE_IP_CHECK_SUCCESS)
			return CAPTIVE_FAILURE;
		
		p1 = p2+1;
		if ( (p2 = strchr( p1, ':' )) != NULL){
			strncpy(ipaddr_end, p1, p2-p1);
			if (captive_check_ip_format(ipaddr_end) != CAPTIVE_IP_CHECK_SUCCESS)
				return CAPTIVE_FAILURE;
			
			strncpy(portset, p2+1, PORTSET_LEN-1);
			if (captive_check_portset_format(portset) != CAPTIVE_SUCCESS)
				return CAPTIVE_FAILURE;

			snprintf(item->iprange, IPRANGE_LEN-1, "%s-%s", ipaddr_begin, ipaddr_end);
			strncpy(item->portset, portset, PORTSET_LEN-1);
		}
		else {
			strncpy(ipaddr_end, p1, IP_ADDR_LEN-1);
			if (captive_check_ip_format(ipaddr_end) != CAPTIVE_IP_CHECK_SUCCESS)
				return CAPTIVE_FAILURE;

			snprintf(item->iprange, IPRANGE_LEN-1, "%s-%s", ipaddr_begin, ipaddr_end);
			strncpy(item->portset, "all", PORTSET_LEN-1);
		}
	}
	else {
		if ( (p2 = strchr( p1, ':')) != NULL){
			strncpy(ipaddr_begin, p1, p2-p1);
			if (captive_check_ip_format(ipaddr_begin) != CAPTIVE_IP_CHECK_SUCCESS)
				return CAPTIVE_FAILURE;

			strncpy(portset, p2+1, PORTSET_LEN-1);
			if (captive_check_portset_format(portset) != CAPTIVE_SUCCESS)
				return CAPTIVE_FAILURE;

			strncpy(item->iprange, ipaddr_begin, IPRANGE_LEN-1);
			strncpy(item->portset, portset, PORTSET_LEN-1);
		}
		else {
			if (captive_check_ip_format(str) != CAPTIVE_IP_CHECK_SUCCESS)
				return CAPTIVE_FAILURE;
			strncpy(item->iprange, str, IPRANGE_LEN-1);
			strncpy(item->portset, "all", PORTSET_LEN-1);
		}
	}

	return CAPTIVE_SUCCESS;
}

int parse_ipv6range_portset(const char *str, ipv6range_portset_t *item)
{
	const char *p1 = NULL, *p2 = NULL, *p3 = NULL, *p4 = NULL;
	char ipv6addr_begin[IPV6_ADDR_LEN], ipv6addr_end[IPV6_ADDR_LEN], portset[PORTSET_LEN]; 

	memset(ipv6addr_begin, 0, sizeof(ipv6addr_begin));
	memset(ipv6addr_end, 0, sizeof(ipv6addr_end));
	memset(portset, 0, sizeof(portset));
	p1 = str;
	p2 = strchr(p1, '[');
	if (NULL == p2){
        p3 = strchr(p1, '-');
        if (NULL == p3) {
            if (captive_check_ipv6_format(str) != CAPTIVE_IP_CHECK_SUCCESS) {
                return CAPTIVE_FAILURE;
			}
            strncpy(item->ipv6range, str, IPV6RANGE_LEN-1);
            strncpy(item->portset, "all", PORTSET_LEN-1);
        } else {
			strncpy(ipv6addr_begin, p1, p3-p1);
			if (captive_check_ipv6_format(ipv6addr_begin) != CAPTIVE_IP_CHECK_SUCCESS) {
				return CAPTIVE_FAILURE;
			}
			p3++;
			strncpy(ipv6addr_end, p3, strlen(p3));
			if (captive_check_ipv6_format(ipv6addr_end) != CAPTIVE_IP_CHECK_SUCCESS) {
				return CAPTIVE_FAILURE;
			}
			snprintf(item->ipv6range, IPV6RANGE_LEN-1, "%s-%s", ipv6addr_begin, ipv6addr_end);
			strncpy(item->portset, "all", PORTSET_LEN-1);
        }
	} else {
		p4 = strchr(p1, ']');
		if (NULL == p4) {
            return CAPTIVE_FAILURE;
		}

		p1++;
        p3 = strchr(p1, '-');
        if (NULL == p3) {
			strncpy(ipv6addr_begin, p1, p4-p1);
            if (captive_check_ipv6_format(ipv6addr_begin) != CAPTIVE_IP_CHECK_SUCCESS) {
                return CAPTIVE_FAILURE;
			}
            strncpy(item->ipv6range, ipv6addr_begin, IPV6RANGE_LEN-1);
        } else {
			strncpy(ipv6addr_begin, p1, p3-p1);
			if (captive_check_ipv6_format(ipv6addr_begin) != CAPTIVE_IP_CHECK_SUCCESS) {
				return CAPTIVE_FAILURE;
			}
			p3++;
			strncpy(ipv6addr_end, p3, p4 - p3);
			if (captive_check_ipv6_format(ipv6addr_end) != CAPTIVE_IP_CHECK_SUCCESS) {
				return CAPTIVE_FAILURE;
			}
			snprintf(item->ipv6range, IPV6RANGE_LEN-1, "%s-%s", ipv6addr_begin, ipv6addr_end);
        }

        if (NULL == strchr(p4 + 1, ':')) {
            return CAPTIVE_FAILURE;
		}
        strncpy(portset, p4 + 2, PORTSET_LEN-1);
        if (captive_check_portset_format(portset) != CAPTIVE_SUCCESS) {
            return CAPTIVE_FAILURE;
		}
        strncpy(item->portset, portset, PORTSET_LEN-1);
	}

	return CAPTIVE_SUCCESS;
}

int get_white_list(int id, white_list_t *p_list)
{
	FILE *fp = NULL;
	char line[MAX_LINE_LEN], *p = NULL, *token = NULL;

	memset(line, 0, sizeof(line));
	
	if (id < 0 || id > 7)
		return CAPTIVE_FAILURE;
	if (NULL == p_list)
		return CAPTIVE_FAILURE;

	p_list->id = id;
	p_list->num = 0;
	
	if ( (fp = fopen(PORTAL_LIST_PATH, "r")) == NULL)
		return CAPTIVE_SUCCESS;

	while (fgets(line, sizeof(line)-1, fp) != NULL){
		for (p = line; *p; p++)
			if ('\r' == *p || '\n' == *p){
				*p = '\0';
				break;
			}
		if ( (token = strtok(line, " \t")) == NULL || atoi(token) != CP_WHITE_LIST_FLAG)
			continue;
		if ( (token = strtok(NULL, " \t")) == NULL || atoi(token) != id)
			continue;
		if ( (token = strtok(NULL, " \t")) == NULL)
			continue;
		memset(p_list->list[p_list->num].iprange, 0, IPRANGE_LEN);
		strncpy(p_list->list[p_list->num].iprange, token, IPRANGE_LEN-1);

		memset(p_list->list[p_list->num].portset, 0, PORTSET_LEN);
		if ( (token = strtok(NULL, " \t")) == NULL)
			strncpy(p_list->list[p_list->num].portset, "all", PORTSET_LEN-1);
		else
			strncpy(p_list->list[p_list->num].portset, token, PORTSET_LEN-1);
		p_list->num++;
	}

	fclose(fp);
	
	return CAPTIVE_SUCCESS;
}

int find_in_white_list(const white_list_t *p_list, const iprange_portset_t *p_item)
{
	int i;
	
	if (NULL == p_list || NULL == p_item)
		return 0;

	for (i = 0; i < p_list->num; i++)
		if (strcmp(p_list->list[i].iprange, p_item->iprange) == 0 
			&& strcmp(p_list->list[i].portset, p_item->portset) == 0)
			return 1;
		
	return 0;
}

int find_white_list_domain(int cp_id, const char *domain)
{
	FILE *fp = NULL;
	char line[MAX_LINE_LEN], *p = NULL, *token = NULL;
	
	memset(line, 0, sizeof(line));
		
	if (cp_id < 0 || cp_id > 7)
		return 0;
		
	if ( (fp = fopen(PORTAL_LIST_PATH, "r")) == NULL)
		return 0;
	
	while (fgets(line, sizeof(line)-1, fp) != NULL){
		for (p = line; *p; p++)
			if ('\r' == *p || '\n' == *p){
				*p = '\0';
				break;
			}
		if ( (token = strtok(line, " \t")) == NULL || atoi(token) != CP_WHITE_LIST_FLAG_DOMAIN)
			continue;
		if ( (token = strtok(NULL, " \t")) == NULL || atoi(token) != cp_id)
			continue;
		if ( (token = strtok(NULL, " \t")) != NULL && strcmp(token, domain) == 0)
			return 1;	
	}
	
	fclose(fp);
		
	return 0;
}

DEFUN(conf_cp_whitelist_with_ip_func,
	conf_cp_whitelist_with_ip_cmd,
	"(add|del) white-list ip IPRANGE[:PORTSET]",
	"add white list\n"
	"delete white list\n"
	"external network which users not authenticated can still access\n" 
	"specifys that white list is described with iprange:postset format\n"
	"ip range and port set to be applied to the white list, with format A.B.C.D[-A.B.C.D][:(all|PORT[,PORT]...)]\n"
)
{
	//vty_out(vty, "argc = %d\n", argc);
	//int i;
	//for (i = 0; i < argc; i++)
		//vty_out(vty, "argv[%d] = %s\n", i, argv[i]);
	int ret, status, operate, cur_id;
	char command[MAX_CMD_LEN];
	iprange_portset_t item;

	memset(command, 0, sizeof(command));
	
	cur_id = (int)(vty->index);
	if (cur_id < 0 || cur_id > 7){
		vty_out(vty, "captive portal id must be <0-7>\n");
		return CMD_WARNING;
	}
	
	if (strcmp(argv[0], "add") == 0)
		operate = WHITELIST_OPERATE_ADD;
	else if (strcmp(argv[0], "del") == 0)
		operate = WHITELIST_OPERATE_DEL;
	else{
		vty_out(vty, "% unknown command!\n");
		return CMD_FAILURE;
	}

	ret = parse_iprange_portset(argv[1], &item);
	if (CAPTIVE_SUCCESS != ret){
		vty_out(vty, "error iprange-portset format\n");
		return CMD_FAILURE;
	}
	
	if (WHITELIST_OPERATE_ADD == operate){
		white_list_t list;
		
		get_white_list(cur_id, &list);
		if (list.num >= MAX_WHITE_LIST_NUM){
			vty_out(vty, "the num of white list reached the max %d!\n", MAX_WHITE_LIST_NUM);
			return CMD_FAILURE;
		}

		if (find_in_white_list(&list, &item)){
			vty_out(vty, "the iprange and portset have already existed!\n");
			return CMD_FAILURE;
		}

		//char record_tmp[256];
		//if (getRecordById(cur_id, record_tmp, sizeof(record_tmp)) == 0){
		//	vty_out(vty, "the captive portal %d isn't in use!\n", cur_id);
		//	return CMD_FAILURE;
		//}
		
		snprintf(command, sizeof(command)-1, ADD_WHITE_LIST_CMD, cur_id, item.iprange, item.portset);
		status = system(command);
		ret = WEXITSTATUS(status);
		if (0 == ret){
			vty_out(vty, "add white list succeeded!\n");
			return CMD_SUCCESS;
		}
		else {
			vty_out(vty, "add white list failed!\n");
			return CMD_FAILURE;
		}
	}
	else if (WHITELIST_OPERATE_DEL == operate){
		char record_tmp[256];
		if (getRecordById(cur_id, record_tmp, sizeof(record_tmp)) == 0){
			vty_out(vty, "the captive portal %d isn't in use!\n", cur_id);
			return CMD_FAILURE;
		}

		white_list_t list;
		get_white_list(cur_id, &list);
		if (!find_in_white_list(&list, &item)){
			vty_out(vty, "the iprange and portset don't exist!\n");
			return CMD_FAILURE;
		}

		snprintf(command, sizeof(command)-1, DEL_WHITE_LIST_CMD, cur_id, item.iprange, item.portset);
		status = system(command);
		ret = WEXITSTATUS(status);
		if (0 == ret){
			vty_out(vty, "del white list succeeded!\n");
			return CMD_SUCCESS;
		}
		else {
			vty_out(vty, "del white list failed!\n");
			return CMD_FAILURE;
		}
	}
	
	return CMD_SUCCESS;
}

DEFUN(conf_cp_whitelist_with_domain_func,
	conf_cp_whitelist_with_domain_cmd,
	"(add|del) white-list domain DOMAIN-URL",
	"add white list\n"
	"delete white list\n"
	"external network which users not authenticated can still access\n" 
	"specifys that white list is described with domain-url format\n"
	"domain url to be applied to the white list\n"
)
{
	//vty_out(vty, "argc = %d\n", argc);
	//int i;
	//for (i = 0; i < argc; i++)
		//vty_out(vty, "argv[%d] = %s\n", i, argv[i]);
	int ret, status, operate, cur_id;
	char command[MAX_CMD_LEN];

	memset(command, 0, sizeof(command));
	
	cur_id = (int)(vty->index);
	if (cur_id < 0 || cur_id > 7){
		vty_out(vty, "captive portal id must be <0-7>\n");
		return CMD_WARNING;
	}
	
	if (strcmp(argv[0], "add") == 0)
		operate = WHITELIST_OPERATE_ADD;
	else if (strcmp(argv[0], "del") == 0)
		operate = WHITELIST_OPERATE_DEL;
	else{
		vty_out(vty, "% unknown command!\n");
		return CMD_FAILURE;
	}

	if (WHITELIST_OPERATE_ADD == operate){
		if (find_white_list_domain(cur_id, argv[1])){
			vty_out(vty, "white list domain %s has existed in this captive portal", argv[1]);
			return CMD_WARNING;
		}
		snprintf(command, sizeof(command)-1, ADD_WHITE_LIST_DOMAIN_CMD, cur_id, argv[1]);
		status = system(command);
		ret = WEXITSTATUS(status);
		if (0 == ret){
			vty_out(vty, "add white list succeeded!\n");
			return CMD_SUCCESS;
		}
		else if (3 == ret){
			vty_out(vty, "no dns server!\n");
			return CMD_FAILURE;
		}
		else {
			vty_out(vty, "add white list failed!\n");
			return CMD_FAILURE;
		}
	}
	else if (WHITELIST_OPERATE_DEL == operate){
		if (!find_white_list_domain(cur_id, argv[1])){
			vty_out(vty, "white list domain %s not exist in this captive portal", argv[1]);
			return CMD_WARNING;
		}
		snprintf(command, sizeof(command)-1, DEL_WHITE_LIST_DOMAIN_CMD, cur_id, argv[1]);
		status = system(command);
		ret = WEXITSTATUS(status);
		if (0 == ret){
			vty_out(vty, "del white list succeeded!\n");
			return CMD_SUCCESS;
		}
		else {
			vty_out(vty, "del white list failed!\n");
			return CMD_FAILURE;
		}
	}
	
	return CMD_SUCCESS;
}

/****** black list ******/
int get_black_list(int id, black_list_t *p_list)
{
	FILE *fp = NULL;
	char line[MAX_LINE_LEN], *p = NULL, *token = NULL;

	memset(line, 0, sizeof(line));
	
	if (id < 0 || id > 7)
		return CAPTIVE_FAILURE;
	if (NULL == p_list)
		return CAPTIVE_FAILURE;

	p_list->id = id;
	p_list->num = 0;
	
	if ( (fp = fopen(PORTAL_LIST_PATH, "r")) == NULL)
		return CAPTIVE_SUCCESS;

	while (fgets(line, sizeof(line)-1, fp) != NULL){
		for (p = line; *p; p++)
			if ('\r' == *p || '\n' == *p){
				*p = '\0';
				break;
			}
		if ( (token = strtok(line, " \t")) == NULL || atoi(token) != CP_BLACK_LIST_FLAG)
			continue;
		if ( (token = strtok(NULL, " \t")) == NULL || atoi(token) != id)
			continue;
		if ( (token = strtok(NULL, " \t")) == NULL)
			continue;
		memset(p_list->list[p_list->num].iprange, 0, IPRANGE_LEN);
		strncpy(p_list->list[p_list->num].iprange, token, IPRANGE_LEN-1);

		memset(p_list->list[p_list->num].portset, 0, PORTSET_LEN);
		if ( (token = strtok(NULL, " \t")) == NULL)
			strncpy(p_list->list[p_list->num].portset, "all", PORTSET_LEN-1);
		else
			strncpy(p_list->list[p_list->num].portset, token, PORTSET_LEN-1);
		p_list->num++;
	}

	fclose(fp);
	
	return CAPTIVE_SUCCESS;
}

int find_in_black_list(const black_list_t *p_list, const iprange_portset_t *p_item)
{
	int i;
	
	if (NULL == p_list || NULL == p_item)
		return 0;

	for (i = 0; i < p_list->num; i++)
		if (strcmp(p_list->list[i].iprange, p_item->iprange) == 0 
			&& strcmp(p_list->list[i].portset, p_item->portset) == 0)
			return 1;
		
	return 0;
}

int find_black_list_domain(int cp_id, const char *domain)
{
	FILE *fp = NULL;
	char line[MAX_LINE_LEN], *p = NULL, *token = NULL;
	
	memset(line, 0, sizeof(line));
		
	if (cp_id < 0 || cp_id > 7)
		return 0;
		
	if ( (fp = fopen(PORTAL_LIST_PATH, "r")) == NULL)
		return 0;
	
	while (fgets(line, sizeof(line)-1, fp) != NULL){
		for (p = line; *p; p++)
			if ('\r' == *p || '\n' == *p){
				*p = '\0';
				break;
			}
		if ( (token = strtok(line, " \t")) == NULL || atoi(token) != CP_BLACK_LIST_FLAG_DOMAIN)
			continue;
		if ( (token = strtok(NULL, " \t")) == NULL || atoi(token) != cp_id)
			continue;
		if ( (token = strtok(NULL, " \t")) != NULL && strcmp(token, domain) == 0)
			return 1;	
	}
	
	fclose(fp);
		
	return 0;
}

DEFUN(conf_cp_blacklist_with_ip_func,
	conf_cp_blacklist_with_ip_cmd,
	"(add|del) black-list ip IPRANGE[:PORTSET]",
	"add black list\n"
	"delete black list\n"
	"external network which users authenticated cann't yet access\n" 
	"specifys that black list is described with iprange:postset format\n"
	"ip range and port set to be applied to the black list, with format A.B.C.D[-A.B.C.D][:(all|PORT[,PORT]...)]\n"
)
{
	//vty_out(vty, "argc = %d\n", argc);
	//int i;
	//for (i = 0; i < argc; i++)
		//vty_out(vty, "argv[%d] = %s\n", i, argv[i]);
	int ret, status, operate, cur_id;
	char command[MAX_CMD_LEN];
	iprange_portset_t item;

	memset(command, 0, sizeof(command));
	
	cur_id = (int)(vty->index);
	if (cur_id < 0 || cur_id > 7){
		vty_out(vty, "captive portal id must range <0-7>!\n");
		return CMD_WARNING;
	}
	
	if (strcmp(argv[0], "add") == 0)
		operate = BLACKLIST_OPERATE_ADD;
	else if (strcmp(argv[0], "del") == 0)
		operate = BLACKLIST_OPERATE_DEL;
	else{
		vty_out(vty, "% unknown command!\n");
		return CMD_FAILURE;
	}

	ret = parse_iprange_portset(argv[1], &item);
	if (CAPTIVE_SUCCESS != ret){
		vty_out(vty, "error iprange-portset format\n");
		return CMD_FAILURE;
	}
	
	if (BLACKLIST_OPERATE_ADD == operate){
		black_list_t list;
		
		get_black_list(cur_id, &list);
		if (list.num >= MAX_BLACK_LIST_NUM){
			vty_out(vty, "the num of black list reached the max %d!\n", MAX_BLACK_LIST_NUM);
			return CMD_FAILURE;
		}

		if (find_in_black_list(&list, &item)){
			vty_out(vty, "the iprange and portset have already existed!\n");
			return CMD_FAILURE;
		}

		//char record_tmp[256];
		//if (getRecordById(cur_id, record_tmp, sizeof(record_tmp)) == 0){
		//	vty_out(vty, "the captive portal %d isn't in use!\n", cur_id);
		//	return CMD_FAILURE;
		//}
		
		snprintf(command, sizeof(command)-1, ADD_BLACK_LIST_CMD, cur_id, item.iprange, item.portset);
		status = system(command);
		ret = WEXITSTATUS(status);
		if (0 == ret){
			vty_out(vty, "add black list succeeded!\n");
			return CMD_SUCCESS;
		}
		else {
			vty_out(vty, "add black list failed!\n");
			return CMD_FAILURE;
		}
	}
	else if (BLACKLIST_OPERATE_DEL == operate){
		char record_tmp[256];
		if (getRecordById(cur_id, record_tmp, sizeof(record_tmp)) == 0){
			vty_out(vty, "the captive portal %d isn't in use!\n", cur_id);
			return CMD_FAILURE;
		}

		black_list_t list;
		get_black_list(cur_id, &list);
		if (!find_in_black_list(&list, &item)){
			vty_out(vty, "the iprange and portset don't exist!\n");
			return CMD_FAILURE;
		}

		snprintf(command, sizeof(command)-1, DEL_BLACK_LIST_CMD, cur_id, item.iprange, item.portset);
		status = system(command);
		ret = WEXITSTATUS(status);
		if (0 == ret){
			vty_out(vty, "del black list succeeded!\n");
			return CMD_SUCCESS;
		}
		else {
			vty_out(vty, "del black list failed!\n");
			return CMD_FAILURE;
		}
	}
	
	return CMD_SUCCESS;
}

DEFUN(conf_cp_blacklist_with_domain_func,
	conf_cp_blacklist_with_domain_cmd,
	"(add|del) black-list domain DOMAIN-URL",
	"add black list\n"
	"delete black list\n"
	"external network which users authenticated cann't yet access\n" 
	"specifys that black list is described with domain-url format\n"
	"domain url to be applied to the black list\n"
)
{
	//vty_out(vty, "argc = %d\n", argc);
	//int i;
	//for (i = 0; i < argc; i++)
		//vty_out(vty, "argv[%d] = %s\n", i, argv[i]);
	int ret, status, operate, cur_id;
	char command[MAX_CMD_LEN];
	iprange_portset_t item;

	memset(command, 0, sizeof(command));
	
	cur_id = (int)(vty->index);
	if (cur_id < 0 || cur_id > 7){
		vty_out(vty, "captive portal id must be <0-7>\n");
		return CMD_WARNING;
	}
	
	if (strcmp(argv[0], "add") == 0)
		operate = BLACKLIST_OPERATE_ADD;
	else if (strcmp(argv[0], "del") == 0)
		operate = BLACKLIST_OPERATE_DEL;
	else{
		vty_out(vty, "% unknown command!\n");
		return CMD_FAILURE;
	}

	if (BLACKLIST_OPERATE_ADD == operate){
		if (find_black_list_domain(cur_id, argv[1])){
			vty_out(vty, "black list domain %s has existed in this captive portal", argv[1]);
			return CMD_WARNING;
		}
		snprintf(command, sizeof(command)-1, ADD_BLACK_LIST_DOMAIN_CMD, cur_id, argv[1]);
		status = system(command);
		ret = WEXITSTATUS(status);
		if (0 == ret){
			vty_out(vty, "add black list succeeded!\n");
			return CMD_SUCCESS;
		}
		//else if (3 == ret){
		//	vty_out(vty, "no dns server!\n");
		//	return CMD_FAILURE;
		//}
		else {
			vty_out(vty, "add black list failed!\n");
			return CMD_FAILURE;
		}
	}
	else if (BLACKLIST_OPERATE_DEL == operate){
		if (!find_black_list_domain(cur_id, argv[1])){
			vty_out(vty, "black list domain %s not exist in this captive portal", argv[1]);
			return CMD_WARNING;
		}
		snprintf(command, sizeof(command)-1, DEL_BLACK_LIST_DOMAIN_CMD, cur_id, argv[1]);
		status = system(command);
		ret = WEXITSTATUS(status);
		if (0 == ret){
			vty_out(vty, "del black list succeeded!\n");
			return CMD_SUCCESS;
		}
		else {
			vty_out(vty, "del black list failed!\n");
			return CMD_FAILURE;
		}
	}
	
	return CMD_SUCCESS;
}

#define whitelist_match_format(format, flag)	((strcmp(format, "ip")==0 && CP_WHITE_LIST_FLAG==flag) \
		|| (strcmp(format, "domain")==0 && CP_WHITE_LIST_FLAG_DOMAIN==flag) \
		|| (strcmp(format, "all")==0 && (CP_WHITE_LIST_FLAG==flag || CP_WHITE_LIST_FLAG_DOMAIN==flag)))

#define whitelist_match_cpid(strcpid, id)		(strcmp(strcpid, "all") == 0 || atoi(strcpid) == id)

/* use macro rather than function, because the vty variable in DEFUN is needed. */
#define show_whitelist(strcpid, format) \
do { \
	FILE *fp = NULL; \
	char line[MAX_LINE_LEN], *p = NULL, *token = NULL; \
	char *fmt_tmp = NULL, *cpid_tmp = NULL, *ip_or_domain_tmp = NULL, *portset_tmp = NULL; \
	int count = 0; \
	memset(line, 0, sizeof(line)); \
	vty_out(vty, "white list summary:\n"); \
	vty_out(vty, "========================================================================\n"); \
	vty_out(vty, "%s\t %s\t %s\t %s\n", "cp-id", "ip/domain", "ip/domain", "portset"); \
	if ( (fp = fopen(PORTAL_LIST_PATH, "r")) == NULL){ \
		vty_out(vty, "========================================================================\n"); \
		vty_out(vty, "count = %d\n", count); \
		break; \
	} \
	while (fgets(line, sizeof(line)-1, fp) != NULL){ \
		for (p = line; *p; p++) \
			if ('\r' == *p || '\n' == *p){ \
				*p = '\0'; \
				break; \
			} \
		if ( (token = strtok(line, " \t")) == NULL || !whitelist_match_format(format, atoi(token))) \
			continue; \
		fmt_tmp = atoi(token) == CP_WHITE_LIST_FLAG?"ip":"domain"; \
		if ( (token = strtok(NULL, " \t")) == NULL || !whitelist_match_cpid(strcpid, atoi(token))) \
			continue; \
		cpid_tmp = token; \
		if ( (token = strtok(NULL, " \t")) == NULL) \
			continue; \
		ip_or_domain_tmp = token; \
		if ( (token = strtok(NULL, " \t")) != NULL) \
			portset_tmp = token; \
		else \
			portset_tmp = ""; \
		vty_out(vty, "%s\t %s\t %s\t %s\n", cpid_tmp, fmt_tmp, ip_or_domain_tmp, portset_tmp); \
		count++; \
	} \
	fclose(fp); \
	vty_out(vty, "========================================================================\n"); \
	vty_out(vty, "count = %d\n", count); \
}while(0)

#define whitelist_is_legal_id(strcpid)	(strcmp(strcpid, "all") == 0 \
									|| (strlen(strcpid) == 1 && strcpid[0] >= '0' && strcpid[0] <= '7'))
#define whitelist_is_legal_format(strfmt)	(strcmp(strfmt, "ip") == 0 || strcmp(strfmt, "domain") == 0 \
									|| strcmp(strfmt, "all") == 0)
DEFUN(show_whitelist_func,
	show_whitelist_cmd,
	"show white-list (<0-7>|all) (ip|domain|all)",
	SHOW_STR
	"external network which users not authenticated can still access\n" 
	"specify captive portal id of which white-list would be listed\n"
	"indicate that white-list of all captive portals would be listed\n"
	"only the white-list with the format of iprange:portset would be listed\n"
	"only the white-list with the format of domain-url would be listed\n\n"
	"all the white-list would be listed, both iprange-portset and domain-url\n"
)
{
	//vty_out(vty, "argc = %d\n", argc);
	//int i;
	//for (i = 0; i < argc; i++)
	//	vty_out(vty, "argv[%d] = %s\n", i, argv[i]);

	if (!whitelist_is_legal_id(argv[0])){
		vty_out(vty, "captive portal id must be all or <0-7>!\n");
		return CMD_WARNING;
	}

	if (!whitelist_is_legal_format(argv[1])){
		vty_out(vty, "only 'ip', 'domain', 'all' are optional!\n");
		return CMD_WARNING;
	}
	
	show_whitelist(argv[0], argv[1]);
	
	return CMD_SUCCESS;
}

DEFUN(show_whitelist_by_curid_func,
	show_whitelist_by_curid_cmd,
	"show white-list (ip|domain|all)",
	SHOW_STR
	"external network which users not authenticated can still access\n" 
	"only the white-list with the format of iprange:portset would be listed\n"
	"only the white-list with the format of domain-url would be listed\n\n"
	"all the white-list would be listed, both iprange-portset and domain-url\n"
)
{
	//vty_out(vty, "argc = %d\n", argc);
	//int i;
	//for (i = 0; i < argc; i++)
	//	vty_out(vty, "argv[%d] = %s\n", i, argv[i]);
	int cur_id;
	char strcpid[10];
	
	cur_id = (int)(vty->index);
	if (cur_id < 0 || cur_id > 7){
		vty_out(vty, "captive portal id must range <0-7>!\n");
		return CMD_WARNING;
	}
	snprintf(strcpid, sizeof(strcpid), "%d", cur_id);
	
	if (!whitelist_is_legal_format(argv[0])){
		vty_out(vty, "only 'ip', 'domain', 'all' are optional!\n");
		return CMD_WARNING;
	}
	
	show_whitelist(strcpid, argv[0]);
	
	return CMD_SUCCESS;
}

#define blacklist_match_format(format, flag)	((strcmp(format, "ip")==0 && CP_BLACK_LIST_FLAG==flag) \
		|| (strcmp(format, "domain")==0 && CP_BLACK_LIST_FLAG_DOMAIN==flag) \
		|| (strcmp(format, "all")==0 && (CP_BLACK_LIST_FLAG==flag || CP_BLACK_LIST_FLAG_DOMAIN==flag)))

#define blacklist_match_cpid(strcpid, id)		(strcmp(strcpid, "all") == 0 || atoi(strcpid) == id)

/* use macro rather than function, because the vty variable in DEFUN is needed. */
#define show_blacklist(strcpid, format) \
do { \
	FILE *fp = NULL; \
	char line[MAX_LINE_LEN], *p = NULL, *token = NULL; \
	char *fmt_tmp = NULL, *cpid_tmp = NULL, *ip_or_domain_tmp = NULL, *portset_tmp = NULL; \
	int count = 0; \
	memset(line, 0, sizeof(line)); \
	vty_out(vty, "black list summary:\n"); \
	vty_out(vty, "========================================================================\n"); \
	vty_out(vty, "%s\t %s\t %s\t %s\n", "cp-id", "ip/domain", "ip/domain", "portset"); \
	if ( (fp = fopen(PORTAL_LIST_PATH, "r")) == NULL){ \
		vty_out(vty, "========================================================================\n"); \
		vty_out(vty, "count = %d\n", count); \
		break; \
	} \
	while (fgets(line, sizeof(line)-1, fp) != NULL){ \
		for (p = line; *p; p++) \
			if ('\r' == *p || '\n' == *p){ \
				*p = '\0'; \
				break; \
			} \
		if ( (token = strtok(line, " \t")) == NULL || !blacklist_match_format(format, atoi(token))) \
			continue; \
		fmt_tmp = atoi(token) == CP_BLACK_LIST_FLAG?"ip":"domain"; \
		if ( (token = strtok(NULL, " \t")) == NULL || !blacklist_match_cpid(strcpid, atoi(token))) \
			continue; \
		cpid_tmp = token; \
		if ( (token = strtok(NULL, " \t")) == NULL) \
			continue; \
		ip_or_domain_tmp = token; \
		if ( (token = strtok(NULL, " \t")) != NULL) \
			portset_tmp = token; \
		else \
			portset_tmp = ""; \
		vty_out(vty, "%s\t %s\t %s\t %s\n", cpid_tmp, fmt_tmp, ip_or_domain_tmp, portset_tmp); \
		count++; \
	} \
	fclose(fp); \
	vty_out(vty, "========================================================================\n"); \
	vty_out(vty, "count = %d\n", count); \
}while(0)

#define blacklist_is_legal_id(strcpid)	(strcmp(strcpid, "all") == 0 \
									|| (strlen(strcpid) == 1 && strcpid[0] >= '0' && strcpid[0] <= '7'))
#define blacklist_is_legal_format(strfmt)	(strcmp(strfmt, "ip") == 0 || strcmp(strfmt, "domain") == 0 \
									|| strcmp(strfmt, "all") == 0)

DEFUN(show_blacklist_func,
	show_blacklist_cmd,
	"show black-list (<0-7>|all) (ip|domain|all)",
	SHOW_STR
	"external network which users authenticated cann't yet access\n" 
	"specify captive portal id of which black-list would be listed\n"
	"indicate that black-list of all captive portals would be listed\n"
	"only the black-list with the format of iprange:portset would be listed\n"
	"only the black-list with the format of domain-url would be listed\n\n"
	"all the black-list would be listed, both iprange-portset and domain-url\n"
)
{
	//vty_out(vty, "argc = %d\n", argc);
	//int i;
	//for (i = 0; i < argc; i++)
	//	vty_out(vty, "argv[%d] = %s\n", i, argv[i]);

	if (!blacklist_is_legal_id(argv[0])){
		vty_out(vty, "captive portal id must be all or <0-7>!\n");
		return CMD_WARNING;
	}

	if (!blacklist_is_legal_format(argv[1])){
		vty_out(vty, "only 'ip', 'domain', 'all' are optional!\n");
		return CMD_WARNING;
	}
	
	show_blacklist(argv[0], argv[1]);
	
	return CMD_SUCCESS;
}

DEFUN(show_blacklist_by_curid_func,
	show_blacklist_by_curid_cmd,
	"show black-list (ip|domain|all)",
	SHOW_STR
	"external network which users authenticated cann't yet access\n" 
	"only the black-list with the format of iprange:portset would be listed\n"
	"only the black-list with the format of domain-url would be listed\n\n"
	"all the black-list would be listed, both iprange-portset and domain-url\n"
)
{
	//vty_out(vty, "argc = %d\n", argc);
	//int i;
	//for (i = 0; i < argc; i++)
	//	vty_out(vty, "argv[%d] = %s\n", i, argv[i]);
	int cur_id;
	char strcpid[10];
	
	cur_id = (int)(vty->index);
	if (cur_id < 0 || cur_id > 7){
		vty_out(vty, "captive portal id must range <0-7>!\n");
		return CMD_WARNING;
	}
	snprintf(strcpid, sizeof(strcpid), "%d", cur_id);
	
	if (!blacklist_is_legal_format(argv[0])){
		vty_out(vty, "only 'ip', 'domain', 'all' are optional!\n");
		return CMD_WARNING;
	}
	
	show_blacklist(strcpid, argv[0]);
	
	return CMD_SUCCESS;
}

int dcli_cap_portal_show_running_config(struct vty* vty)
{
	//vty_out(vty,"!captive-portal session\n" );
	// !captive-portal session
	do {
		char _tmpstr[64];
		memset(_tmpstr, 0, 64);
		sprintf(_tmpstr, BUILDING_MOUDLE, "captive-portal");
		vtysh_add_show_string(_tmpstr);
	} while(0);
	
#if 1   /*  不要删除，只是暂时注销*/
	FILE *fp = NULL;
	int i, cur_id;
	char record[MAX_LINE_LEN], showStr[256], ip_addr[IP_ADDR_LEN], strPort[10], if_info[256], line[MAX_LINE_LEN];
	char *p=NULL, *token=NULL, *list_type=NULL, *fmt_type=NULL, *ip_or_domain=NULL, *portset=NULL; 
		
	memset(record, 0, sizeof(record));
	memset(showStr, 0, sizeof(showStr));
	memset(ip_addr, 0, sizeof(ip_addr));
	memset(if_info, 0, sizeof(if_info));
	memset(line, 0, sizeof(line));
	
	for (i = 0; i <= 7; i++)
		if (getRecordById(i, record, sizeof(record)) == 1){
			// config captive-portal <0-7>
			snprintf(showStr, sizeof(showStr), "config captive-portal %d", i);
			vtysh_add_show_string(showStr);

			// set params A.B.C.D PORTNO INTERFACES
			for (p = record; *p; p++)
				if ('\r' == *p || '\n' == *p){
					*p = '\0';
					break;
				}
			sscanf(record, "%d %s %s %*s %*s %s", &cur_id, ip_addr, strPort, if_info);
			snprintf(showStr, sizeof(showStr), "set params %s %s %s", ip_addr, strPort, if_info);
			vtysh_add_show_string(showStr);

			// add white-list ip	 IPRANGE[:PORTSET]
			// add white-list domain DOMAIN-URL
			// add black-list ip IPRANGE[:PORTSET]
			// add black-list domain DOMAIN-URL
			if ( (fp = fopen(PORTAL_LIST_PATH, "r")) == NULL){
				vtysh_add_show_string("exit");
				break;
			}
			while (fgets(line, sizeof(line)-1, fp) != NULL){
				for (p = line; *p; p++)
					if ('\r' == *p || '\n' == *p){
						*p = '\0';
						break;
					}
				if ( (token = strtok(line, " \t")) == NULL)
					continue;
				if (atoi(token) == CP_WHITE_LIST_FLAG){
					list_type = "white-list";
					fmt_type = "ip";
				}
				else if (atoi(token) == CP_WHITE_LIST_FLAG_DOMAIN){
					list_type = "white-list";
					fmt_type = "domain";
				}
				else if (atoi(token) == CP_BLACK_LIST_FLAG){
					list_type = "black-list";
					fmt_type = "ip";
				}
				else {
					list_type = "black-list";
					fmt_type = "domain";
				}
					
				if ( (token = strtok(NULL, " \t")) == NULL || atoi(token) != i)
					continue;
		
				if ( (token = strtok(NULL, " \t")) == NULL)
					continue;
				ip_or_domain = token;
				if ( (token = strtok(NULL, " \t")) != NULL)
					portset = token;
				else
					portset = "all";

				if (strcmp(fmt_type, "ip") == 0)
					snprintf(showStr, sizeof(showStr), "add %s %s %s:%s", list_type, fmt_type, ip_or_domain, portset);
				else
					snprintf(showStr, sizeof(showStr), "add %s %s %s", list_type, fmt_type, ip_or_domain);
				vtysh_add_show_string(showStr);
			}
			
			fclose(fp);
			vtysh_add_show_string("exit");
		}
#endif

	return 0;
}

void dcli_captive_init
(
	void
)  
{
	install_node( &captive_node, dcli_cap_portal_show_running_config, "CAPTIVE_NODE");
	install_default(CAPTIVE_NODE);
	
	install_element(CONFIG_NODE, &conf_captive_portal_cmd);
	install_element(CONFIG_NODE, &show_captive_portal_cmd);
	install_element(VIEW_NODE, &show_captive_portal_cmd);
	install_element(CAPTIVE_NODE, &show_captive_portal_cmd);
	//install_element(CAPTIVE_NODE, &show_cur_captive_portal_cmd);
	install_element(CAPTIVE_NODE, &set_cur_captive_portal_param_cmd);
	//install_element(CONFIG_NODE, &clear_captive_portal_cmd);
	install_element(CAPTIVE_NODE, &clear_captive_portal_cmd);
	install_element(CAPTIVE_NODE, &conf_cp_whitelist_with_ip_cmd);
	install_element(CAPTIVE_NODE, &conf_cp_whitelist_with_domain_cmd);
	install_element(CAPTIVE_NODE, &conf_cp_blacklist_with_ip_cmd);
	install_element(CAPTIVE_NODE, &conf_cp_blacklist_with_domain_cmd);
	install_element(CONFIG_NODE, &show_whitelist_cmd);
	install_element(VIEW_NODE, &show_whitelist_cmd);
	install_element(CAPTIVE_NODE, &show_whitelist_cmd);
	//install_element(CAPTIVE_NODE, &show_whitelist_by_curid_cmd);
	install_element(CONFIG_NODE, &show_blacklist_cmd);
	install_element(VIEW_NODE, &show_blacklist_cmd);
	install_element(CAPTIVE_NODE, &show_blacklist_cmd);
	//install_element(CAPTIVE_NODE, &show_blacklist_by_curid_cmd);
}
#ifdef __cplusplus
}
#endif

