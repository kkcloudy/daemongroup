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
* portal_ha.c
*
*
* CREATOR:
* autelan.software.xxx. team
*
* DESCRIPTION:
* xxx module main routine
*
*
*******************************************************************************/

/* eag_captive.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <sys/wait.h>
#include "eag_errcode.h"
#include "nm_list.h"
#include "eag_log.h"

#include "eag_mem.h"
#include "eag_blkmem.h"
#include "session.h"
#include "eag_interface.h"
#include "eag_util.h"

#include "eag_captive.h"
#include <dbus/dbus.h>
#include "eag_dbus.h"
#include "eag_authorize.h"
#include "eag_dbus.h"
#include "eag_iptables.h"
#include "eag_ip6tables.h"
#include "appconn.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/if.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

#include "nmp_process.h"
extern nmp_mutex_t eag_iptables_lock;

/*captive portal shell call!!!!!!*/
#define CAP_SHELL_PATH	"/usr/bin/"
#define CAP_SHELL_CMD_LINE_LEN	256
#define CAP_FILENAME_MAX_LEN	64
#define EAG_IPTABLES_ADD	4
#define EAG_IPTABLES_DELTE	5
#define EAG_SHELL_OFF		1

struct eag_captive {
	unsigned int redir_srv_ip;
	struct in6_addr redir_srv_ipv6;
	unsigned short redir_srv_port;
	CAP_STATUS status;
	int capid;
	int instype;
	int isipset;
	int macauth_isipset;

	unsigned long curr_ifnum;
	char cpif[CP_MAX_INTERFACE_NUM][MAX_IF_NAME_LEN];

	unsigned long ipv6_curr_ifnum;
	char ipv6_cpif[CP_MAX_INTERFACE_NUM][MAX_IF_NAME_LEN];

	struct bw_rules white;
	struct bw_rules black;

	eag_ins_t *eagins;
	eag_redir_t *redir;
};

#define EAG_LOOPBACK(x)		(((x) & htonl(0xff000000)) == htonl(0x7f000000))
#define EAG_MULTICAST(x)	(((x) & htonl(0xf0000000)) == htonl(0xe0000000))
#define EAG_BADCLASS(x)		(((x) & htonl(0xf0000000)) == htonl(0xf0000000))
#define EAG_ZERONET(x)		(((x) & htonl(0xff000000)) == htonl(0x00000000))
#define EAG_LOCAL_MCAST(x)	(((x) & htonl(0xFFFFFF00)) == htonl(0xE0000000))

static int 
eag_u32ipaddr_check(unsigned int ipaddr)
{
	if (EAG_LOOPBACK(ipaddr)
		|| EAG_MULTICAST(ipaddr)
		|| EAG_BADCLASS(ipaddr)
		|| EAG_ZERONET(ipaddr)
		|| EAG_LOCAL_MCAST(ipaddr)) 
	{
		return -1;
	}

	return 0;
}

static int 
eag_check_interface(char *ifname) 
{
	struct ifreq tmp;
	int sock = -1;
	struct sockaddr_in *addr = NULL;

	if (NULL == ifname) {
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	
	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock < 0) {
		return EAG_ERR_SOCKET_FAILED;
	}

	memset(&tmp, 0, sizeof(tmp));
	strncpy(tmp.ifr_name, ifname, sizeof(tmp.ifr_name) - 1);
	if (ioctl(sock, SIOCGIFADDR, &tmp) < 0) {
		close(sock);
		sock = -1;
		return -1;
	}
	close(sock);
	sock = -1;

	addr = (struct sockaddr_in *)&tmp.ifr_addr;
	
	if (eag_u32ipaddr_check(htonl(addr->sin_addr.s_addr))) {
		return EAG_ERR_UNKNOWN;
	}

	return EAG_RETURN_OK;
}

static int
captive_shell_create( int insid, int instype, void *addr, unsigned short port, uint32_t family)
{
	int ret;
	char ipstr[32];
	char ipv6str[48];
	char cmd[CAP_SHELL_CMD_LINE_LEN];
	memset(cmd, 0, sizeof(cmd));
	if (EAG_IPV4 == family) {
		ip2str( *((unsigned long *)addr), ipstr, sizeof(ipstr)-1);
		snprintf( cmd, sizeof(cmd)-1, 
				"sudo "CAP_SHELL_PATH"cp_create_profile.sh %d %s %s %d %d",
				insid, (instype==HANSI_LOCAL)?"L":"R", ipstr, port, family );
	}
	if (EAG_IPV6 == family) {
		ipv6tostr( (struct in6_addr *)addr, ipv6str, sizeof(ipv6str)-1 );
		snprintf( cmd, sizeof(cmd)-1, 
				"sudo "CAP_SHELL_PATH"cp_create_profile.sh %d %s %s %d %d",
				insid, (instype==HANSI_LOCAL)?"L":"R", ipv6str, port, family );
	}
	ret = system(cmd);
	if (-1 == ret) {
		eag_log_err("captive_shell_create system cmd=%s error ret=%d", cmd, ret);
		return ret;
	}
    if (0 == WIFEXITED(ret)) {
		eag_log_err("captive_shell_create system cmd=%s error ret=%d", cmd, ret);
		return ret;
	}
	ret = WEXITSTATUS(ret);
	eag_log_info("captive_shell_create cmd=%s ret=%d", cmd, ret);
	if( 4 == ret ){/*4 is define in cp_create_profile.sh for insid and instype already exist in ip(6)tables rules!!*/
		eag_log_err("captive_shell_create this insid and instype is already exist!");
		ret = EAG_RETURN_OK;
	}

	return ret;
}

static int
captive_shell_destroy( int insid, int instype, uint32_t family )
{
	int ret;
	char cmd[CAP_SHELL_CMD_LINE_LEN];
	memset(cmd, 0, sizeof(cmd));
	snprintf( cmd, sizeof(cmd)-1, 
			"sudo "CAP_SHELL_PATH"cp_del_portal_id.sh %d %s %d",
			insid, (instype==HANSI_LOCAL)?"L":"R", family );
	ret = system(cmd);
	ret = WEXITSTATUS(ret);
	eag_log_info("captive_shell_destroy cmd=%s ret=%d", cmd, ret);	
	return ret;
}


#if EAG_SHELL_OFF
static int
captive_iptables_add_intf( int insid, int instype, char *intf )
{
	char cap_id_file[CAP_FILENAME_MAX_LEN] = {0};
	char cap_if_db_file[CAP_FILENAME_MAX_LEN] = {0};
	char ins_type = 0;
	FILE *fp = NULL;
	mode_t old_mask = 0;
	int ret = 0;

	ins_type = (instype == HANSI_LOCAL) ? 'L' : 'R'; 
	snprintf(cap_id_file, CAP_FILENAME_MAX_LEN-1, 
				"/var/run/cpp/CP_%c%d_IPV4", ins_type, insid);
	snprintf(cap_if_db_file, CAP_FILENAME_MAX_LEN-1, 
				"/var/run/cpp/CP_IF_INFO_%s_IPV4", intf);

	if (access(cap_id_file, F_OK) < 0) {
		eag_log_warning("captive_iptables_add_intf %s is not exist", cap_id_file);
		return EAG_ERR_CAPTIVE_ID_FILE_NOT_EXIST;
	}
	
	if (access(cap_if_db_file, F_OK) == 0) {
		eag_log_warning("captive_iptables_add_intf %s is already exist", cap_if_db_file);
		return EAG_ERR_CAPTIVE_IF_DB_FILE_ALREADY_EXIST;
	}

	ret = eag_iptable_add_interface(insid, ins_type, intf);
	if (EAG_RETURN_OK != ret) {
		eag_log_err("captive_iptables_add_intf eag_iptable_add_interface error");
		return ret;
	}
	
	old_mask = umask(022);
	fp = fopen(cap_if_db_file, "w");
	if (NULL == fp) {
		eag_log_warning("captive_iptables_add_intf open %s fail:%s",
						cap_if_db_file, safe_strerror(errno));
		return EAG_ERR_UNKNOWN;
	}
	fprintf(fp, "%c%d", ins_type, insid);
	fclose(fp);
	umask(old_mask);
	
	return 0;
}

static int
captive_iptables_del_intf( int insid, int instype, char *intf )
{
	char cap_if_db_file[CAP_FILENAME_MAX_LEN] = {0};
	char ins_type = 0;
	char cmpstr[16] = {0};
	char buf[16] = {0};
	FILE *fp = NULL;
	int ret = 0;

	ins_type = (instype == HANSI_LOCAL) ? 'L' : 'R'; 
	snprintf(cap_if_db_file, CAP_FILENAME_MAX_LEN-1, 
				"/var/run/cpp/CP_IF_INFO_%s_IPV4", intf);
	snprintf(cmpstr, 15, "%c%d", ins_type, insid);
	fp = fopen(cap_if_db_file, "r");
	if (NULL == fp) {
		eag_log_warning("captive_iptables_del_intf open %s fail:%s",
						cap_if_db_file, safe_strerror(errno));
		return EAG_ERR_UNKNOWN;
	}
	fgets(buf, 15, fp);
	if (strcmp(buf, cmpstr)) {
		eag_log_warning("%s not be used by %s but by %s", intf, cmpstr, buf);
		fclose(fp);
		return EAG_ERR_UNKNOWN;
	}
	fclose(fp);

	ret = eag_iptable_del_interface(insid, ins_type, intf);
	if (EAG_RETURN_OK != ret) {
		eag_log_err("captive_iptables_del_intf eag_iptable_del_interface error");
		return ret;
	}

	ret = remove(cap_if_db_file);
	if (0 != ret) {
		eag_log_err("captive_iptables_del_intf delete %s fail:%s",
						cap_if_db_file, safe_strerror(errno));
	}
	
	return ret;
}

static int
captive_iptables_add_white_ip(int insid, int instype, unsigned long ipbegin,
		unsigned long ipend, char *ipport, char *intf)
{
	struct white_black_iprange input_info;
	memset(&input_info, 0, sizeof(struct white_black_iprange));

	snprintf(input_info.chain_name, 32, "CP_%s%d_F_DEFAULT", 
		(instype==HANSI_LOCAL)?"L":"R", insid);
	snprintf(input_info.nat_chain_name, 32, "CP_%s%d_N_DEFAULT",
		(instype==HANSI_LOCAL)?"L":"R", insid);
	strcpy(input_info.target_name, "FW_FILTER");
	strcpy(input_info.nat_target_name, "FW_DNAT");
	input_info.ipbegin = ipbegin;
	input_info.ipend = ipend;
	strcpy(input_info.portstring, ipport);
	strncpy(input_info.iniface, intf, MAX_IF_NAME_LEN);
	input_info.flag = EAG_IPTABLES_ADD;

	eag_iptable_iprange(&input_info);

	return 0;
}

static int
captive_iptables_del_white_ip(int insid, int instype, unsigned long ipbegin,
		unsigned long ipend, char *ipport, char *intf)
{
	struct white_black_iprange input_info;
	memset(&input_info, 0, sizeof(struct white_black_iprange));

	snprintf(input_info.chain_name, 32, "CP_%s%d_F_DEFAULT", 
		(instype==HANSI_LOCAL)?"L":"R", insid);
	snprintf(input_info.nat_chain_name, 32, "CP_%s%d_N_DEFAULT",
		(instype==HANSI_LOCAL)?"L":"R", insid);
	strcpy(input_info.target_name, "FW_FILTER");
	strcpy(input_info.nat_target_name, "FW_DNAT");
	input_info.ipbegin = ipbegin;
	input_info.ipend = ipend;
	strcpy(input_info.portstring, ipport);
	strcpy(input_info.iniface, intf);
	input_info.flag = EAG_IPTABLES_DELTE;

	eag_iptable_iprange(&input_info);

	return 0;
}

static int
captive_iptables_add_black_ip(int insid, int instype, unsigned long ipbegin,
		unsigned long ipend, char *ipport, char *intf)
{
	struct white_black_iprange input_info;
	memset(&input_info, 0, sizeof(struct white_black_iprange));

	snprintf(input_info.chain_name, 32, "CP_%s%d_F_AUTH_DEFAULT", 
		(instype==HANSI_LOCAL)?"L":"R", insid);
	snprintf(input_info.nat_chain_name, 32, "CP_%s%d_N_AUTH_DEFAULT",
		(instype==HANSI_LOCAL)?"L":"R", insid);
	strcpy(input_info.target_name, "DROP");
	strcpy(input_info.nat_target_name, "FW_DNAT");
	input_info.ipbegin = ipbegin;
	input_info.ipend = ipend;
	strcpy(input_info.portstring, ipport);
	strcpy(input_info.iniface, intf);
	input_info.flag = EAG_IPTABLES_ADD;

	eag_iptable_iprange(&input_info);

	return 0;
}

static int
captive_iptables_del_black_ip(int insid, int instype, unsigned long ipbegin,
		unsigned long ipend, char *ipport, char *intf)
{
	struct white_black_iprange input_info;
	memset(&input_info, 0, sizeof(struct white_black_iprange));

	snprintf(input_info.chain_name, 32, "CP_%s%d_F_AUTH_DEFAULT", 
		(instype==HANSI_LOCAL)?"L":"R", insid);
	snprintf(input_info.nat_chain_name, 32, "CP_%s%d_N_AUTH_DEFAULT",
		(instype==HANSI_LOCAL)?"L":"R", insid);
	strcpy(input_info.target_name, "DROP");
	strcpy(input_info.nat_target_name, "FW_DNAT");
	input_info.ipbegin = ipbegin;
	input_info.ipend = ipend;
	strcpy(input_info.portstring, ipport);
	strcpy(input_info.iniface, intf);
	input_info.flag = EAG_IPTABLES_DELTE;

	eag_iptable_iprange(&input_info);

	return 0;
}

static int
captive_iptables_add_white_domain( int insid, int instype, 
			            struct bw_rule_t *rule )
{
	int i;
	struct white_black_iprange input_info;
	memset(&input_info, 0, sizeof(struct white_black_iprange));

	input_info.flag = EAG_IPTABLES_ADD;

	if(NULL == rule) {
		eag_log_err("captive_iptables_add_white_domain rule is NULL!");
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	
	if(rule->type != RULE_DOMAIN) {
		eag_log_err("captive_iptables_add_white_domain rule is not domain type!");
		return EAG_ERR_UNKNOWN;		
	}

	if(strlen(rule->key.domain.name) == 0) {
		eag_log_err("captive_iptables_add_white_domain  domain_name is empty!");
		return EAG_ERR_UNKNOWN;
    }
	#if 0
	if(0 == rule->key.domain.num) {
		eag_log_err("captive_iptables_add_white_domain  domain ip num is 0!");
		return EAG_ERR_UNKNOWN;
	}
	#endif
	snprintf(input_info.chain_name, 32, "CP_%s%d_F_DEFAULT", 
		(instype==HANSI_LOCAL)?"L":"R", insid);
	snprintf(input_info.nat_chain_name, 32, "CP_%s%d_N_DEFAULT",
		(instype==HANSI_LOCAL)?"L":"R", insid);
	strcpy(input_info.target_name, "FW_FILTER");
	strcpy(input_info.nat_target_name, "FW_DNAT");
	strcpy(input_info.comment_str, rule->key.domain.name);
	strcpy(input_info.iniface, rule->intf);
	strcpy(input_info.portstring, "all");

	for(i=0; i<rule->key.domain.num; i++) {
		input_info.ipbegin = rule->key.domain.ip[i];
		input_info.ipend = rule->key.domain.ip[i];	
		
		eag_iptable_white_domain(&input_info);
	}
	return 0;
}

static int
captive_iptables_del_white_domain( int insid, int instype, 
			            struct bw_rule_t *rule )
{
	int i;
	struct white_black_iprange input_info;
	memset(&input_info, 0, sizeof(struct white_black_iprange));

	input_info.flag = EAG_IPTABLES_DELTE;

	if(NULL == rule) {
		eag_log_err("captive_iptables_del_white_domain rule is NULL!");
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	
	if(rule->type != RULE_DOMAIN) {
		eag_log_err("captive_iptables_del_white_domain rule is not domain type!");
		return EAG_ERR_UNKNOWN;		
	}

	if(strlen(rule->key.domain.name) == 0) {
		eag_log_err("captive_iptables_add_white_domain  domain_name is empty!");
		return EAG_ERR_UNKNOWN;
    }
	#if 0
	if(0 == rule->key.domain.num) {
		eag_log_err("captive_iptables_add_white_domain  domain ip num is 0!");
		return EAG_ERR_UNKNOWN;
	}
	#endif
	
	snprintf(input_info.chain_name, 32, "CP_%s%d_F_DEFAULT", 
		(instype==HANSI_LOCAL)?"L":"R", insid);
	snprintf(input_info.nat_chain_name, 32, "CP_%s%d_N_DEFAULT",
		(instype==HANSI_LOCAL)?"L":"R", insid);
	strcpy(input_info.target_name, "FW_FILTER");
	strcpy(input_info.nat_target_name, "FW_DNAT");
	strcpy(input_info.comment_str, rule->key.domain.name);
	strcpy(input_info.iniface, rule->intf);
	strcpy(input_info.portstring, "all");
		

	for(i=0; i<rule->key.domain.num; i++) {
		input_info.ipbegin = rule->key.domain.ip[i];
		input_info.ipend = rule->key.domain.ip[i];
		
		eag_iptable_white_domain(&input_info);
	}
	return 0;
}

static int
captive_iptables_add_black_domain( int insid, int instype, 
			            struct bw_rule_t *rule )
{
	int i;
	struct white_black_iprange input_info;
	memset(&input_info, 0, sizeof(struct white_black_iprange));

	input_info.flag = EAG_IPTABLES_ADD;

	if(NULL == rule) {
		eag_log_err("captive_iptables_add_black_domain rule is NULL!");
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	
	if(rule->type != RULE_DOMAIN) {
		eag_log_err("captive_iptables_add_black_domain rule is not domain type!");
		return EAG_ERR_UNKNOWN;		
	}

	if(strlen(rule->key.domain.name) == 0) {
		eag_log_err("captive_iptables_add_black_domain  domain_name is empty!");
		return EAG_ERR_UNKNOWN;
    }
	#if 0
	if(0 == rule->key.domain.num) {
		eag_log_err("captive_iptables_add_black_domain  domain ip num is 0!");
		return EAG_ERR_UNKNOWN;
	}
	#endif

	snprintf(input_info.chain_name, 32, "CP_%s%d_F_AUTH_DEFAULT", 
		(instype==HANSI_LOCAL)?"L":"R", insid);
	strcpy(input_info.target_name, "DROP");
	strcpy(input_info.nat_chain_name, "");
	strcpy(input_info.nat_target_name, "");
	strcpy(input_info.comment_str, rule->key.domain.name);
	strcpy(input_info.iniface, rule->intf);
	strcpy(input_info.portstring, "all");

	for(i=0; i<rule->key.domain.num; i++) {
		input_info.ipbegin = rule->key.domain.ip[i];
		input_info.ipend = rule->key.domain.ip[i];	
		
		eag_iptable_black_domain(&input_info);
	}
		
	/*eag_iptable_black_domain(chain_name, rule->intf, rule->key.domain.name, flag);*/
	return 0;
}

static int
captive_iptables_del_black_domain( int insid, int instype, 
			            struct bw_rule_t *rule )
{
	int i;
	struct white_black_iprange input_info;
	memset(&input_info, 0, sizeof(struct white_black_iprange));

	input_info.flag = EAG_IPTABLES_DELTE;

	if(NULL == rule) {
		eag_log_err("captive_iptables_add_black_domain rule is NULL!");
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	
	if(rule->type != RULE_DOMAIN) {
		eag_log_err("captive_iptables_add_black_domain rule is not domain type!");
		return EAG_ERR_UNKNOWN;		
	}

	if(strlen(rule->key.domain.name) == 0) {
		eag_log_err("captive_iptables_add_black_domain  domain_name is empty!");
		return EAG_ERR_UNKNOWN;
    }
	#if 0
	if(0 == rule->key.domain.num) {
		eag_log_err("captive_iptables_add_black_domain  domain ip num is 0!");
		return EAG_ERR_UNKNOWN;
	}
	#endif 

	snprintf(input_info.chain_name, 32, "CP_%s%d_F_AUTH_DEFAULT", 
		(instype==HANSI_LOCAL)?"L":"R", insid);
	strcpy(input_info.target_name, "DROP");
	strcpy(input_info.nat_chain_name, "");
	strcpy(input_info.nat_target_name, "");
	strcpy(input_info.comment_str, rule->key.domain.name);
	strcpy(input_info.iniface, rule->intf);
	strcpy(input_info.portstring, "all");

	for(i=0; i<rule->key.domain.num; i++) {
		input_info.ipbegin = rule->key.domain.ip[i];
		input_info.ipend = rule->key.domain.ip[i];	
		
		eag_iptable_black_domain(&input_info);
	}
		
	/*eag_iptable_black_domain(chain_name, rule->intf, rule->key.domain.name, flag);*/
	return 0;
}

static int
captive_ip6tables_add_intf( int insid, int instype, char *intf )
{
	char cap_id_file[CAP_FILENAME_MAX_LEN] = {0};
	char cap_if_db_file[CAP_FILENAME_MAX_LEN] = {0};
	char ins_type = 0;
	FILE *fp = NULL;
	mode_t old_mask = 0;
	int ret = 0;

	ins_type = (instype == HANSI_LOCAL) ? 'L' : 'R'; 
	snprintf(cap_id_file, CAP_FILENAME_MAX_LEN-1, 
				"/var/run/cpp/CP_%c%d_IPV6", ins_type, insid);
	snprintf(cap_if_db_file, CAP_FILENAME_MAX_LEN-1, 
				"/var/run/cpp/CP_IF_INFO_%s_IPV6", intf);

	if (access(cap_id_file, F_OK) < 0) {
		eag_log_warning("captive_ip6tables_add_intf %s is not exist", cap_id_file);
		return EAG_ERR_CAPTIVE_ID_FILE_NOT_EXIST;
	}
	
	if (access(cap_if_db_file, F_OK) == 0) {
		eag_log_warning("captive_ip6tables_add_intf %s is already exist", cap_if_db_file);
		return EAG_ERR_CAPTIVE_IF_DB_FILE_ALREADY_EXIST;
	}

	ret = eag_ip6table_add_interface(insid, ins_type, intf);
	if (EAG_RETURN_OK != ret) {
		eag_log_err("captive_ip6tables_add_intf eag_ip6table_add_interface error");
		return ret;
	}
	
	old_mask = umask(022);
	fp = fopen(cap_if_db_file, "w");
	if (NULL == fp) {
		eag_log_warning("captive_ip6tables_add_intf open %s fail:%s",
						cap_if_db_file, safe_strerror(errno));
		return EAG_ERR_UNKNOWN;
	}
	fprintf(fp, "%c%d", ins_type, insid);
	fclose(fp);
	umask(old_mask);
	
	return 0;
}

static int
captive_ip6tables_del_intf( int insid, int instype, char *intf )
{
	char cap_if_db_file[CAP_FILENAME_MAX_LEN] = {0};
	char ins_type = 0;
	char cmpstr[16] = {0};
	char buf[16] = {0};
	FILE *fp = NULL;
	int ret = 0;

	ins_type = (instype == HANSI_LOCAL) ? 'L' : 'R'; 
	snprintf(cap_if_db_file, CAP_FILENAME_MAX_LEN-1, 
				"/var/run/cpp/CP_IF_INFO_%s_IPV6", intf);
	snprintf(cmpstr, 15, "%c%d", ins_type, insid);
	fp = fopen(cap_if_db_file, "r");
	if (NULL == fp) {
		eag_log_warning("captive_ip6tables_del_intf open %s fail:%s",
						cap_if_db_file, safe_strerror(errno));
		return EAG_ERR_UNKNOWN;
	}
	fgets(buf, 15, fp);
	if (strcmp(buf, cmpstr)) {
		eag_log_warning("%s not be used by %s but by %s", intf, cmpstr, buf);
		fclose(fp);
		return EAG_ERR_UNKNOWN;
	}
	fclose(fp);

	ret = eag_ip6table_del_interface(insid, ins_type, intf);
	if (EAG_RETURN_OK != ret) {
		eag_log_err("captive_ip6tables_del_intf eag_iptable_del_interface error");
		return ret;
	}

	ret = remove(cap_if_db_file);
	if (0 != ret) {
		eag_log_err("captive_ip6tables_del_intf delete %s fail:%s",
						cap_if_db_file, safe_strerror(errno));
	}
	
	return ret;
}
#if 0
static int
captive_ip6tables_add_white_ip(int insid, int instype, unsigned long ipbegin,
		unsigned long ipend, char *ipport, char *intf)
{
	struct ipv6_white_black_iprange input_info;
	memset(&input_info, 0, sizeof(struct ipv6_white_black_iprange));

	snprintf(input_info.chain_name, 32, "CP_%s%d_F_DEFAULT", 
		(instype==HANSI_LOCAL)?"L":"R", insid);
	snprintf(input_info.nat_chain_name, 32, "CP_%s%d_N_DEFAULT",
		(instype==HANSI_LOCAL)?"L":"R", insid);
	strcpy(input_info.target_name, "FW_FILTER");
	strcpy(input_info.nat_target_name, "FW_DNAT");
	input_info.ipbegin = ipbegin;
	input_info.ipend = ipend;
	strcpy(input_info.portstring, ipport);
	strncpy(input_info.iniface, intf, MAX_IF_NAME_LEN);
	input_info.flag = EAG_IP6TABLES_ADD;

	eag_ip6table_iprange(&input_info);

	return 0;
}

static int
captive_ip6tables_del_white_ip(int insid, int instype, unsigned long ipbegin,
		unsigned long ipend, char *ipport, char *intf)
{
	struct ipv6_white_black_iprange input_info;
	memset(&input_info, 0, sizeof(struct ipv6_white_black_iprange));

	snprintf(input_info.chain_name, 32, "CP_%s%d_F_DEFAULT", 
		(instype==HANSI_LOCAL)?"L":"R", insid);
	snprintf(input_info.nat_chain_name, 32, "CP_%s%d_N_DEFAULT",
		(instype==HANSI_LOCAL)?"L":"R", insid);
	strcpy(input_info.target_name, "FW_FILTER");
	strcpy(input_info.nat_target_name, "FW_DNAT");
	input_info.ipbegin = ipbegin;
	input_info.ipend = ipend;
	strcpy(input_info.portstring, ipport);
	strcpy(input_info.iniface, intf);
	input_info.flag = EAG_IP6TABLES_DELTE;

	eag_ip6table_iprange(&input_info);

	return 0;
}

static int
captive_ip6tables_add_black_ip(int insid, int instype, unsigned long ipbegin,
		unsigned long ipend, char *ipport, char *intf)
{
	struct ipv6_white_black_iprange input_info;
	memset(&input_info, 0, sizeof(struct ipv6_white_black_iprange));

	snprintf(input_info.chain_name, 32, "CP_%s%d_F_AUTH_DEFAULT", 
		(instype==HANSI_LOCAL)?"L":"R", insid);
	snprintf(input_info.nat_chain_name, 32, "CP_%s%d_N_AUTH_DEFAULT",
		(instype==HANSI_LOCAL)?"L":"R", insid);
	strcpy(input_info.target_name, "DROP");
	strcpy(input_info.nat_target_name, "FW_DNAT");
	input_info.ipbegin = ipbegin;
	input_info.ipend = ipend;
	strcpy(input_info.portstring, ipport);
	strcpy(input_info.iniface, intf);
	input_info.flag = EAG_IP6TABLES_ADD;

	eag_ip6table_iprange(&input_info);

	return 0;
}

static int
captive_ip6tables_del_black_ip(int insid, int instype, unsigned long ipbegin,
		unsigned long ipend, char *ipport, char *intf)
{
	struct white_black_iprange input_info;
	memset(&input_info, 0, sizeof(struct white_black_iprange));

	snprintf(input_info.chain_name, 32, "CP_%s%d_F_AUTH_DEFAULT", 
		(instype==HANSI_LOCAL)?"L":"R", insid);
	snprintf(input_info.nat_chain_name, 32, "CP_%s%d_N_AUTH_DEFAULT",
		(instype==HANSI_LOCAL)?"L":"R", insid);
	strcpy(input_info.target_name, "DROP");
	strcpy(input_info.nat_target_name, "FW_DNAT");
	input_info.ipbegin = ipbegin;
	input_info.ipend = ipend;
	strcpy(input_info.portstring, ipport);
	strcpy(input_info.iniface, intf);
	input_info.flag = EAG_IP6TABLES_DELTE;

	eag_ip6table_iprange(&input_info);

	return 0;
}

static int
captive_ip6tables_add_white_domain( int insid, int instype, 
			            struct bw_rule_t *rule )
{
	int i;
	struct ipv6_white_black_iprange input_info;
	memset(&input_info, 0, sizeof(struct ipv6_white_black_iprange));

	input_info.flag = EAG_IP6TABLES_ADD;

	if(NULL == rule) {
		eag_log_err("captive_ip6tables_add_white_domain rule is NULL!");
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	
	if(rule->type != RULE_DOMAIN) {
		eag_log_err("captive_ip6tables_add_white_domain rule is not domain type!");
		return EAG_ERR_UNKNOWN;		
	}

	if(strlen(rule->key.domain.name) == 0) {
		eag_log_err("captive_ip6tables_add_white_domain  domain_name is empty!");
		return EAG_ERR_UNKNOWN;
    }
	#if 0
	if(0 == rule->key.domain.num) {
		eag_log_err("captive_iptables_add_white_domain  domain ip num is 0!");
		return EAG_ERR_UNKNOWN;
	}
	#endif
	snprintf(input_info.chain_name, 32, "CP_%s%d_F_DEFAULT", 
		(instype==HANSI_LOCAL)?"L":"R", insid);
	snprintf(input_info.nat_chain_name, 32, "CP_%s%d_N_DEFAULT",
		(instype==HANSI_LOCAL)?"L":"R", insid);
	strcpy(input_info.target_name, "FW_FILTER");
	strcpy(input_info.nat_target_name, "FW_DNAT");
	strcpy(input_info.comment_str, rule->key.domain.name);
	strcpy(input_info.iniface, rule->intf);
	strcpy(input_info.portstring, "all");

	for(i=0; i<rule->key.domain.num; i++) {
		input_info.ipbegin = rule->key.domain.ip[i];
		input_info.ipend = rule->key.domain.ip[i];	
		
		eag_ip6table_white_domain(&input_info);
	}
	return 0;
}

static int
captive_ip6tables_del_white_domain( int insid, int instype, 
			            struct bw_rule_t *rule )
{
	int i;
	struct white_black_iprange input_info;
	memset(&input_info, 0, sizeof(struct white_black_iprange));

	input_info.flag = EAG_IP6TABLES_DELTE;

	if(NULL == rule) {
		eag_log_err("captive_iptables_del_white_domain rule is NULL!");
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	
	if(rule->type != RULE_DOMAIN) {
		eag_log_err("captive_iptables_del_white_domain rule is not domain type!");
		return EAG_ERR_UNKNOWN;		
	}

	if(strlen(rule->key.domain.name) == 0) {
		eag_log_err("captive_iptables_add_white_domain  domain_name is empty!");
		return EAG_ERR_UNKNOWN;
    }
	#if 0
	if(0 == rule->key.domain.num) {
		eag_log_err("captive_iptables_add_white_domain  domain ip num is 0!");
		return EAG_ERR_UNKNOWN;
	}
	#endif
	
	snprintf(input_info.chain_name, 32, "CP_%s%d_F_DEFAULT", 
		(instype==HANSI_LOCAL)?"L":"R", insid);
	snprintf(input_info.nat_chain_name, 32, "CP_%s%d_N_DEFAULT",
		(instype==HANSI_LOCAL)?"L":"R", insid);
	strcpy(input_info.target_name, "FW_FILTER");
	strcpy(input_info.nat_target_name, "FW_DNAT");
	strcpy(input_info.comment_str, rule->key.domain.name);
	strcpy(input_info.iniface, rule->intf);
	strcpy(input_info.portstring, "all");
		

	for(i=0; i<rule->key.domain.num; i++) {
		input_info.ipbegin = rule->key.domain.ip[i];
		input_info.ipend = rule->key.domain.ip[i];
		
		eag_ip6table_white_domain(&input_info);
	}
	return 0;
}

static int
captive_ip6tables_add_black_domain( int insid, int instype, 
			            struct bw_rule_t *rule )
{
	int i;
	struct white_black_iprange input_info;
	memset(&input_info, 0, sizeof(struct white_black_iprange));

	input_info.flag = EAG_IP6TABLES_ADD;

	if(NULL == rule) {
		eag_log_err("captive_iptables_add_black_domain rule is NULL!");
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	
	if(rule->type != RULE_DOMAIN) {
		eag_log_err("captive_iptables_add_black_domain rule is not domain type!");
		return EAG_ERR_UNKNOWN;		
	}

	if(strlen(rule->key.domain.name) == 0) {
		eag_log_err("captive_iptables_add_black_domain  domain_name is empty!");
		return EAG_ERR_UNKNOWN;
    }
	#if 0
	if(0 == rule->key.domain.num) {
		eag_log_err("captive_iptables_add_black_domain  domain ip num is 0!");
		return EAG_ERR_UNKNOWN;
	}
	#endif

	snprintf(input_info.chain_name, 32, "CP_%s%d_F_AUTH_DEFAULT", 
		(instype==HANSI_LOCAL)?"L":"R", insid);
	strcpy(input_info.target_name, "DROP");
	strcpy(input_info.nat_chain_name, "");
	strcpy(input_info.nat_target_name, "");
	strcpy(input_info.comment_str, rule->key.domain.name);
	strcpy(input_info.iniface, rule->intf);
	strcpy(input_info.portstring, "all");

	for(i=0; i<rule->key.domain.num; i++) {
		input_info.ipbegin = rule->key.domain.ip[i];
		input_info.ipend = rule->key.domain.ip[i];	
		
		eag_ip6table_black_domain(&input_info);
	}
		
	/*eag_iptable_black_domain(chain_name, rule->intf, rule->key.domain.name, flag);*/
	return 0;
}

static int
captive_ip6tables_del_black_domain( int insid, int instype, 
			            struct bw_rule_t *rule )
{
	int i;
	struct white_black_iprange input_info;
	memset(&input_info, 0, sizeof(struct white_black_iprange));

	input_info.flag = EAG_IP6TABLES_DELTE;

	if(NULL == rule) {
		eag_log_err("captive_iptables_add_black_domain rule is NULL!");
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	
	if(rule->type != RULE_DOMAIN) {
		eag_log_err("captive_iptables_add_black_domain rule is not domain type!");
		return EAG_ERR_UNKNOWN;		
	}

	if(strlen(rule->key.domain.name) == 0) {
		eag_log_err("captive_iptables_add_black_domain  domain_name is empty!");
		return EAG_ERR_UNKNOWN;
    }
	#if 0
	if(0 == rule->key.domain.num) {
		eag_log_err("captive_iptables_add_black_domain  domain ip num is 0!");
		return EAG_ERR_UNKNOWN;
	}
	#endif 

	snprintf(input_info.chain_name, 32, "CP_%s%d_F_AUTH_DEFAULT", 
		(instype==HANSI_LOCAL)?"L":"R", insid);
	strcpy(input_info.target_name, "DROP");
	strcpy(input_info.nat_chain_name, "");
	strcpy(input_info.nat_target_name, "");
	strcpy(input_info.comment_str, rule->key.domain.name);
	strcpy(input_info.iniface, rule->intf);
	strcpy(input_info.portstring, "all");

	for(i=0; i<rule->key.domain.num; i++) {
		input_info.ipbegin = rule->key.domain.ip[i];
		input_info.ipend = rule->key.domain.ip[i];	
		
		eag_ip6table_black_domain(&input_info);
	}
		
	/*eag_iptable_black_domain(chain_name, rule->intf, rule->key.domain.name, flag);*/
	return 0;
}
#endif
#else

static int
captive_shell_add_intf( int insid, int instype, uint32_t family, char *intf )
{
	int ret;
	char cmd[CAP_SHELL_CMD_LINE_LEN];
	#if 0
	if( EAG_RETURN_OK != eag_captive_set_nat_flag(intf, IFF_EAG_DNAT_PREVENT, 1) ){
		eag_log_err("eag_captive_set_nat_flag error!");
		return EAG_ERR_UNKNOWN;
	}
	#endif
	snprintf( cmd, sizeof(cmd)-1, 
			"sudo "CAP_SHELL_PATH"cp_apply_if.sh %d %s %s %d ",
			insid, (instype==HANSI_LOCAL)?"L":"R", intf, family);
	
	ret = system(cmd);
	ret = WEXITSTATUS(ret);
	eag_log_info("captive_shell_add_intf cmd=%s ret=%d", cmd, ret);
	return ret;
}


static int
captive_shell_del_intf( int insid, int instype, uint32_t family, char *intf )
{
	int ret;
	char cmd[CAP_SHELL_CMD_LINE_LEN];

	snprintf( cmd, sizeof(cmd)-1, 
			"sudo "CAP_SHELL_PATH"cp_del_portal_interface.sh %d %s %s %d ",
			insid, (instype==HANSI_LOCAL)?"L":"R", intf, family );
	ret = system(cmd);
	ret = WEXITSTATUS(ret);
	eag_log_info("captive_shell_del_intf cmd=%s ret=%d", cmd, ret);
	return ret;
}

static int
captive_shell_add_white_ip( int insid, int instype, 
			unsigned long ipbegin, unsigned long ipend, char *ipport, char *intf )
{
	int ret;
	char ipbeginstr[32];
	char ipendstr[32];
	char cmd[CAP_SHELL_CMD_LINE_LEN];

	if( strcmp(ipport,"all") == 0 ){
		ipport = "0";
	}

	ip2str( ipbegin, ipbeginstr, sizeof(ipbeginstr)-1);
	ip2str( ipend, ipendstr, sizeof(ipendstr)-1);
	snprintf( cmd, sizeof(cmd)-1, 
			"sudo "CAP_SHELL_PATH"cp_add_white_list.sh %d %s %s-%s %s %s",
			insid, (instype==HANSI_LOCAL)?"L":"R", ipbeginstr, ipendstr, ipport, 
			(intf==NULL)?"":intf);
	ret = system(cmd);
	ret = WEXITSTATUS(ret);
	eag_log_info("captive_shell_add_white_ip cmd=%s ret=%d", cmd, ret);
	return ret;
}

static int
captive_shell_del_white_ip( int insid, int instype,
		unsigned long ipbegin, unsigned long ipend, char *ipport, char *intf )
{
	int ret;
	char ipbeginstr[32];
	char ipendstr[32];
	char cmd[CAP_SHELL_CMD_LINE_LEN];

	if( strcmp(ipport,"all") == 0 ){
		ipport = "0";
	}


	ip2str( ipbegin, ipbeginstr, sizeof(ipbeginstr)-1);
	ip2str( ipend, ipendstr, sizeof(ipendstr)-1);
	snprintf( cmd, sizeof(cmd)-1, 
			"sudo "CAP_SHELL_PATH"cp_del_white_list.sh %d %s %s-%s %s %s",
			insid, (instype==HANSI_LOCAL)?"L":"R", ipbeginstr, ipendstr, ipport, 
			(intf==NULL)?"":intf);
	ret = system(cmd);
	ret = WEXITSTATUS(ret);
	eag_log_info("captive_shell_del_white_ip cmd=%s ret=%d", cmd, ret);
	return ret;
}

static int
captive_shell_add_black_ip( int insid, int instype,
			unsigned long ipbegin, unsigned long ipend, char *ipport, char *intf )
{
	int ret;
	char ipbeginstr[32];
	char ipendstr[32];
	char cmd[CAP_SHELL_CMD_LINE_LEN];

	if( strcmp(ipport,"all") == 0 ){
		ipport = "0";
	}
	

	ip2str( ipbegin, ipbeginstr, sizeof(ipbeginstr)-1);
	ip2str( ipend, ipendstr, sizeof(ipendstr)-1);
	snprintf( cmd, sizeof(cmd)-1, 
			"sudo "CAP_SHELL_PATH"cp_add_black_list.sh %d %s %s-%s %s %s",
			insid, (instype==HANSI_LOCAL)?"L":"R", ipbeginstr, ipendstr, ipport, 
			(intf==NULL)?"":intf);
	ret = system(cmd);
	ret = WEXITSTATUS(ret);
	eag_log_info("captive_shell_add_black_ip cmd=%s ret=%d", cmd, ret);
	return ret;
}

static int
captive_shell_del_black_ip( int insid, int instype,
			unsigned long ipbegin, unsigned long ipend, char *ipport, char *intf )
{
	int ret;
	char ipbeginstr[32];
	char ipendstr[32];
	char cmd[CAP_SHELL_CMD_LINE_LEN];

	if( strcmp(ipport,"all") == 0 ){
		ipport = "0";
	}
	

	ip2str( ipbegin, ipbeginstr, sizeof(ipbeginstr)-1);
	ip2str( ipend, ipendstr, sizeof(ipendstr)-1);
	snprintf( cmd, sizeof(cmd)-1, 
			"sudo "CAP_SHELL_PATH"cp_del_black_list.sh %d %s %s-%s %s %s",
			insid, (instype==HANSI_LOCAL)?"L":"R", ipbeginstr, ipendstr, ipport, 
			(intf==NULL)?"":intf);
	ret = system(cmd);
	ret = WEXITSTATUS(ret);
	eag_log_info("captive_shell_del_black_ip cmd=%s ret=%d", cmd, ret);
	return ret;
}

static int
captive_shell_add_white_domain( int insid, int instype, 
			            struct bw_rule_t *rule )
{
    int ret;
	int i;
	char ipstr[32];
    char cmd[CAP_SHELL_CMD_LINE_LEN];

	if( NULL == rule ){
		eag_log_err("captive_shell_add_white_domain rule is NULL!");
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	
	if( rule->type != RULE_DOMAIN ){
		eag_log_err("captive_shell_add_white_domain rule is not domain type!");
		return EAG_ERR_UNKNOWN;		
	}
	
    if( strlen(rule->key.domain.name) == 0 ){
		eag_log_err("captive_shell_add_white_domain  domain_name is empty!");
		return EAG_ERR_UNKNOWN;
    }

	if( 0 == rule->key.domain.num ){
		eag_log_err("captive_shell_add_white_domain  domain ip num is 0!");
		return EAG_ERR_UNKNOWN;
	}

	for( i=0; i<rule->key.domain.num; i++ ){
		ip2str( rule->key.domain.ip[i], ipstr, sizeof(ipstr)-1);
	    snprintf( cmd, sizeof(cmd)-1, 
		        "sudo "CAP_SHELL_PATH"cp_add_white_list_domain.sh %d %s %s-%s %s %s",
		        insid, (instype==HANSI_LOCAL)?"L":"R", ipstr, ipstr, 
		        rule->key.domain.name, rule->intf);
	    ret = system(cmd);
	    ret = WEXITSTATUS(ret);			
		eag_log_info("captive_shell_add_white_domain cmd=%s ret=%d", cmd, ret);
		printf("captive_shell_add_white_domain cmd=%s ret=%d", cmd, ret);
		
	}
    
    return ret;
}

static int
captive_shell_del_white_domain( int insid, int instype,
		    struct bw_rule_t *rule )
{
	int ret;
	int i;
	char ipstr[32];
	char cmd[CAP_SHELL_CMD_LINE_LEN];

	if( NULL == rule ){
		eag_log_err("captive_shell_del_white_domain rule is NULL!");
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	if( rule->type != RULE_DOMAIN ){
		eag_log_err("captive_shell_del_white_domain rule is not domain type!");
		return EAG_ERR_UNKNOWN;		
	}
	
	if( strlen(rule->key.domain.name) == 0 ){
		eag_log_err("captive_shell_del_white_domain  domain_name is empty!");
		return EAG_ERR_UNKNOWN;
	}
#if 0
	if( 0 == rule->key.domain.num ){
		eag_log_err("captive_shell_del_white_domain  domain ip num is 0!");
		return EAG_ERR_UNKNOWN;
	}
#endif

	for( i=0; i<rule->key.domain.num; i++ ){
		ip2str( rule->key.domain.ip[i], ipstr, sizeof(ipstr)-1);
		snprintf( cmd, sizeof(cmd)-1, 
				"sudo "CAP_SHELL_PATH"cp_del_white_list_domain.sh %d %s %s-%s %s %s",
				insid, (instype==HANSI_LOCAL)?"L":"R", ipstr, ipstr, 
				rule->key.domain.name, rule->intf);
		ret = system(cmd);
		ret = WEXITSTATUS(ret); 		
		eag_log_info("captive_shell_del_white_domain cmd=%s ret=%d", cmd, ret);
		printf("captive_shell_del_white_domain cmd=%s ret=%d", cmd, ret);
		
	}
	
	return ret;
}


static int
captive_shell_add_black_domain( int insid, int instype,
			        struct bw_rule_t *rule )
{
	int ret;
	char cmd[CAP_SHELL_CMD_LINE_LEN];

	if( NULL == rule ){
		eag_log_err("captive_shell_add_black_domain rule is NULL!");
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	
	if( rule->type != RULE_DOMAIN ){
		eag_log_err("captive_shell_add_black_domain rule is not domain type!");
		return EAG_ERR_UNKNOWN;		
	}
	
	if( strlen(rule->key.domain.name) == 0 ){
		eag_log_err("captive_shell_add_black_domain  domain_name is empty!");
		return EAG_ERR_UNKNOWN;
	}

#if 0
	if( 0 == rule->key.domain.num ){
		eag_log_err("captive_shell_add_black_domain  domain ip num is 0!");
		return EAG_ERR_UNKNOWN;
	}
#endif

	snprintf( cmd, sizeof(cmd)-1, 
			"sudo "CAP_SHELL_PATH"cp_add_black_list_domain.sh %d %s  %s %s",
			insid, (instype==HANSI_LOCAL)?"L":"R",  
			rule->key.domain.name, rule->intf);
	ret = system(cmd);
	ret = WEXITSTATUS(ret); 		
	eag_log_info("captive_shell_add_black_domain cmd=%s ret=%d", cmd, ret);
	
	printf("captive_shell_add_black_domain cmd=%s ret=%d", cmd, ret);
	
	return ret;
}



static int
captive_shell_del_black_domain( int insid, int instype,
			 		struct bw_rule_t *rule )
{
	int ret;
	char cmd[CAP_SHELL_CMD_LINE_LEN];

	if( NULL == rule ){
		eag_log_err("captive_shell_del_black_domain rule is NULL!");
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	
	if( rule->type != RULE_DOMAIN ){
		eag_log_err("captive_shell_del_black_domain rule is not domain type!");
		return EAG_ERR_UNKNOWN;		
	}

	if( strlen(rule->key.domain.name) == 0 ){
		eag_log_err("captive_shell_del_black_domain  domain_name is empty!");
		return EAG_ERR_UNKNOWN;
	}

#if 0
	if( 0 == rule->key.domain.num ){
		eag_log_err("captive_shell_del_black_domain  domain ip num is 0!");
		return EAG_ERR_UNKNOWN;
	}
#endif

	snprintf( cmd, sizeof(cmd)-1, 
			"sudo "CAP_SHELL_PATH"cp_del_black_list_domain.sh %d %s  %s %s",
			insid, (instype==HANSI_LOCAL)?"L":"R",	
			rule->key.domain.name, rule->intf);
	ret = system(cmd);
	ret = WEXITSTATUS(ret); 		
	eag_log_info("captive_shell_del_black_domain cmd=%s ret=%d", cmd, ret);
	printf("captive_shell_del_black_domain cmd=%s ret=%d", cmd, ret);
	return ret;
}
#endif

static int
captive_shell_add_white_ipv6( int insid, int instype, 
            				struct in6_addr *ipv6begin, 
            				struct in6_addr *ipv6end, 
            				char *ipport, char *intf )
{
	int ret;
	char ipv6beginstr[48];
	char ipv6endstr[48];
	char cmd[CAP_SHELL_CMD_LINE_LEN];

	if( strcmp(ipport,"all") == 0 ){
		ipport = "0";
	}

	ipv6tostr( ipv6begin, ipv6beginstr, sizeof(ipv6beginstr)-1);
	ipv6tostr( ipv6end, ipv6endstr, sizeof(ipv6endstr)-1);
	snprintf( cmd, sizeof(cmd)-1, 
			"sudo "CAP_SHELL_PATH"cp_add_white_list.sh %d %s %s-%s %s 6 %s",
			insid, (instype==HANSI_LOCAL)?"L":"R", ipv6beginstr, ipv6endstr, ipport, 
			(intf==NULL)?"":intf);
	ret = system(cmd);
	ret = WEXITSTATUS(ret);
	eag_log_info("captive_shell_add_white_ip cmd=%s ret=%d", cmd, ret);
	return ret;
}

static int
captive_shell_del_white_ipv6( int insid, int instype, 
            				struct in6_addr *ipv6begin, 
            				struct in6_addr *ipv6end, 
            				char *ipport, char *intf )
{
	int ret;
	char ipv6beginstr[48];
	char ipv6endstr[48];
	char cmd[CAP_SHELL_CMD_LINE_LEN];

	if( strcmp(ipport,"all") == 0 ){
		ipport = "0";
	}

	ipv6tostr( ipv6begin, ipv6beginstr, sizeof(ipv6beginstr)-1);
	ipv6tostr( ipv6end, ipv6endstr, sizeof(ipv6endstr)-1);
	snprintf( cmd, sizeof(cmd)-1, 
			"sudo "CAP_SHELL_PATH"cp_del_white_list.sh %d %s %s-%s %s 6 %s",
			insid, (instype==HANSI_LOCAL)?"L":"R", ipv6beginstr, ipv6endstr, ipport, 
			(intf==NULL)?"":intf);
	ret = system(cmd);
	ret = WEXITSTATUS(ret);
	eag_log_info("captive_shell_del_white_ip cmd=%s ret=%d", cmd, ret);
	return ret;
}

static int
captive_shell_add_black_ipv6( int insid, int instype, 
            				struct in6_addr *ipv6begin, 
            				struct in6_addr *ipv6end, 
            				char *ipport, char *intf )
{
	int ret;
	char ipv6beginstr[48];
	char ipv6endstr[48];
	char cmd[CAP_SHELL_CMD_LINE_LEN];

	if( strcmp(ipport,"all") == 0 ){
		ipport = "0";
	}
	
	ipv6tostr( ipv6begin, ipv6beginstr, sizeof(ipv6beginstr)-1);
	ipv6tostr( ipv6end, ipv6endstr, sizeof(ipv6endstr)-1);
	snprintf( cmd, sizeof(cmd)-1, 
			"sudo "CAP_SHELL_PATH"cp_add_black_list.sh %d %s %s-%s %s 6 %s",
			insid, (instype==HANSI_LOCAL)?"L":"R", ipv6beginstr, ipv6endstr, ipport, 
			(intf==NULL)?"":intf);
	ret = system(cmd);
	ret = WEXITSTATUS(ret);
	eag_log_info("captive_shell_add_black_ip cmd=%s ret=%d", cmd, ret);
	return ret;
}

static int
captive_shell_del_black_ipv6( int insid, int instype, 
            				struct in6_addr *ipv6begin, 
            				struct in6_addr *ipv6end, 
            				char *ipport, char *intf )
{
	int ret;
	char ipv6beginstr[48];
	char ipv6endstr[48];
	char cmd[CAP_SHELL_CMD_LINE_LEN];

	if( strcmp(ipport,"all") == 0 ){
		ipport = "0";
	}
	
	ipv6tostr( ipv6begin, ipv6beginstr, sizeof(ipv6beginstr)-1);
	ipv6tostr( ipv6end, ipv6endstr, sizeof(ipv6endstr)-1);
	snprintf( cmd, sizeof(cmd)-1, 
			"sudo "CAP_SHELL_PATH"cp_del_black_list.sh %d %s %s-%s %s 6 %s",
			insid, (instype==HANSI_LOCAL)?"L":"R", ipv6beginstr, ipv6endstr, ipport, 
			(intf==NULL)?"":intf);
	ret = system(cmd);
	ret = WEXITSTATUS(ret);
	eag_log_info("captive_shell_del_black_ip cmd=%s ret=%d", cmd, ret);
	return ret;
}

int eag_captive_get_capid( eag_captive_t * cap )
{
	if( NULL == cap ) return 0;

	return cap->capid;
}
int eag_captive_get_hansitype( eag_captive_t * cap )
{
	if( NULL == cap ) return 0;
	return cap->instype;
}

int eag_captive_set_ipset( eag_captive_t * cap, int switch_t)
{
	if( NULL == cap) {
		eag_log_err("eag_captive_set_ipset cap = NULL!");
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	cap->isipset = switch_t;

	return EAG_RETURN_OK;
}

int eag_captive_get_ipset( eag_captive_t *cap )
{
	if( NULL == cap ) return -1;
	return cap->isipset;
}

int eag_captive_set_macauth_ipset( eag_captive_t * cap, int switch_t)
{
	if( NULL == cap) {
		eag_log_err("eag_captive_set_macauth_ipset cap = NULL!");
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	cap->macauth_isipset = switch_t;

	return EAG_RETURN_OK;
}

int eag_captive_get_macauth_ipset( eag_captive_t *cap )
{
	if( NULL == cap ) return -1;
	return cap->macauth_isipset;
}

eag_captive_t *
eag_captive_new( int capid, int type )
{
	eag_captive_t *cap = NULL;

	cap = (eag_captive_t *) eag_malloc(sizeof (eag_captive_t));
	if (NULL == cap) {
		eag_log_err("eag_captive_new eag_malloc failed!");
		return NULL;
	}
	memset(cap, 0, sizeof (eag_captive_t));

	cap->capid = capid;
	cap->instype = type;
	cap->isipset = 1;
	cap->macauth_isipset = 1;
	eag_log_debug("eag_captive", "eag_captive_new sucess! cap=%p", cap);
	return cap;
}

int
eag_captive_free(eag_captive_t * cap)
{
	if (CAP_START == cap->status) {
		eag_captive_stop(cap);
	}

	eag_free(cap);

	eag_log_debug("eag_captive", "eag_captive_free sucess!");
	return EAG_RETURN_OK;
}

int
eag_captive_set_redir_srv(eag_captive_t * cap,
			  unsigned long srv_ip, unsigned short srv_port)
{
	if (NULL == cap) {
		eag_log_err("eag_captive_set_redir_srv cap = NULL!");
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	eag_log_debug("eag_captive", "eag_captive_set_redir_srv success!");
	cap->redir_srv_ip = srv_ip;
	cap->redir_srv_port = srv_port;
	return EAG_RETURN_OK;
}

int
eag_captive_set_ipv6_redir_srv(eag_captive_t * cap,
			  struct in6_addr *srv_ipv6, unsigned short srv_port)
{
	if (NULL == cap || NULL == srv_ipv6) {
		eag_log_err("eag_captive_set_ipv6_redir_srv cap = NULL!");
		return EAG_ERR_INPUT_PARAM_ERR;
	}
		
	eag_log_debug("eag_captive", "eag_captive_set_ipv6_redir_srv success!");
	cap->redir_srv_ipv6 = *srv_ipv6;
	return EAG_RETURN_OK;
}

static int
is_interface_valid(char *intf)
{
	/*check if the system has this interface */
	/*eag_log_warning("TODO: you should complete is_interface_valid!");*/
	return EAG_TRUE;
}

#if 0
struct cap_rule_t *
eag_captive_get_rule(eag_captive_t * cap, char *intf)
{
	struct cap_rule_t *rule;

	list_for_each_entry(rule, &(cap->rule), node) {
		if (0 == strcmp(rule->intf, intf)) {
			return rule;
		}
	}

	return NULL;
}
#endif

int
eag_captive_is_intf_in_list(eag_captive_t * cap, uint32_t family, char *intf)
{
	int i;
	if (EAG_IPV4 == family || EAG_MIX == family) {
		for( i=0; i<cap->curr_ifnum; i++ ){
			if( strcmp( intf, cap->cpif[i] ) == 0 ){
				return EAG_TRUE;
			}
		}
	}
	if (EAG_IPV6 == family || EAG_MIX == family) {
		for( i=0; i<cap->ipv6_curr_ifnum; i++ ){
			if( strcmp( intf, cap->ipv6_cpif[i] ) == 0 ){
				return EAG_TRUE;
			}
		}
	}
	return EAG_FALSE;
}

int
eag_captive_add_interface(eag_captive_t * cap, uint32_t family, char *intf)
{
	int ret = EAG_ERR_UNKNOWN;

	if (NULL == cap 
		|| NULL == intf
		|| strlen(intf) == 0 
		|| strlen(intf)>MAX_IF_NAME_LEN-1) {
		eag_log_err("eag_captive_add_interface cap=%p  intfs=%p:%s",
			    cap, intf, (NULL == intf) ? "" : intf);
		return EAG_ERR_INPUT_PARAM_ERR;
	}
#if 0
    if (eag_check_interface(intf)) {
		eag_log_err("eag_captive_add_interface add interface without setting the IP address!");
		return EAG_ERR_CAPTIVE_INTERFACE_NOT_EXIST;
	}
#endif
	if (!if_nametoindex(intf)) {
		eag_log_err("eag_captive_add_interface no such interface %s\n", intf);
		return EAG_ERR_CAPTIVE_INTERFACE_NOT_EXIST;
	}
	eag_log_info("family:%d, intf:%s", family, intf);
	if (EAG_IPV6 == family || EAG_MIX == family) {
		if( cap->ipv6_curr_ifnum >= CP_MAX_INTERFACE_NUM ){
			eag_log_err("eag_captive_add_interface add interface num to limit!");
			return EAG_ERR_CAPTIVE_INTERFACE_NUM_LIMIT;
		}
	}
	if (EAG_IPV4 == family || EAG_MIX == family) {
		if( cap->curr_ifnum >= CP_MAX_INTERFACE_NUM ){
			eag_log_err("eag_captive_add_interface add interface num to limit!");
			return EAG_ERR_CAPTIVE_INTERFACE_NUM_LIMIT;
		}
	}
	if( EAG_TRUE == eag_captive_is_intf_in_list( cap, family, intf) ){
		eag_log_err("eag_captive_add_interface add interface aready be used!");
		return EAG_ERR_CAPTIVE_INTERFACE_AREADY_USED;
	}
	
	if ( EAG_TRUE == is_interface_valid(intf)) {
        if (EAG_IPV4 == family || EAG_MIX == family) {
			strncpy(cap->cpif[cap->curr_ifnum], intf, MAX_IF_NAME_LEN - 1);
			cap->curr_ifnum++;
			if( CAP_START == cap->status ){
				#if EAG_SHELL_OFF
				captive_iptables_add_intf( cap->capid, cap->instype, intf );
				#else
				captive_shell_add_intf( cap->capid, cap->instype, family, intf );
				#endif
			}
			ret = EAG_RETURN_OK;
		}
		if (EAG_IPV6 == family || EAG_MIX == family) {
			strncpy(cap->ipv6_cpif[cap->ipv6_curr_ifnum], intf, MAX_IF_NAME_LEN - 1);
			cap->ipv6_curr_ifnum++;
			if( CAP_START == cap->status ){
    			#if EAG_SHELL_OFF
				captive_ip6tables_add_intf( cap->capid, cap->instype, intf );
    			#else
				captive_shell_add_intf( cap->capid, cap->instype, family, intf );
    			#endif
			}
			ret = EAG_RETURN_OK;
		}
	} 

	return ret;
}

int
eag_captive_del_interface(eag_captive_t * cap, uint32_t family, char *intf)
{
	int ret = EAG_ERR_UNKNOWN;
	int i;
	
	if (NULL == cap 
		|| NULL == intf
		|| strlen(intf) == 0 
		|| strlen(intf)>MAX_IF_NAME_LEN-1) {
		eag_log_err("eag_captive_del_interface cap=%p  intfs=%p:%s",
				cap, intf, (NULL == intf) ? "" : intf);
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	
    if (EAG_IPV4 == family || EAG_MIX == family) {
		for( i=0; i<cap->curr_ifnum; i++ ){
			if( strcmp( intf, cap->cpif[i] ) == 0 ){
				break;
			}
		}
		if( i >= cap->curr_ifnum ){
			eag_log_err("eag_captive_del_interface del ipv4 interface not exist!");
			return EAG_ERR_CAPTIVE_INTERFACE_NOT_EXIST;
		}
		for(;i<cap->curr_ifnum;i++ ){
			strncpy( cap->cpif[i], cap->cpif[i+1], MAX_IF_NAME_LEN-1 );
		}
		cap->curr_ifnum--;
	}
	if (EAG_IPV6 == family || EAG_MIX == family) {
		for( i=0; i<cap->ipv6_curr_ifnum; i++ ){
			if( strcmp( intf, cap->ipv6_cpif[i] ) == 0 ){
				break;
			}
		}
		if( i >= cap->ipv6_curr_ifnum ){
			eag_log_err("eag_captive_del_interface del ipv6 interface not exist!");
			return EAG_ERR_CAPTIVE_INTERFACE_NOT_EXIST;
		}
		for(;i<cap->ipv6_curr_ifnum;i++ ){
			strncpy( cap->ipv6_cpif[i], cap->ipv6_cpif[i+1], MAX_IF_NAME_LEN-1 );
		}
		cap->ipv6_curr_ifnum--;
	}

	if( CAP_START == cap->status ){
        if (EAG_IPV4 == family  || EAG_MIX == family) {
			#if EAG_SHELL_OFF
			captive_iptables_del_intf( cap->capid, cap->instype, intf );
			#else
			captive_shell_del_intf( cap->capid, cap->instype, family, intf );
			#endif
		}
		if (EAG_IPV6 == family  || EAG_MIX == family) {
			#if EAG_SHELL_OFF
			captive_ip6tables_del_intf( cap->capid, cap->instype, intf );
			#else
			captive_shell_del_intf( cap->capid, cap->instype, family, intf );
			#endif
		}
	}

	ret = EAG_RETURN_OK;
	return ret;
}




int
eag_captive_start(eag_captive_t *cap)
{
	int i;
	int ret;
	if( CAP_START == cap->status  ){
		eag_log_err("eag_captive_start failed: server already start" );
		//return EAG_ERR_CAPTIVE_SERVICE_ALREADY_START;
		return EAG_RETURN_OK;
	}
	
	if( 0 == cap->redir_srv_ip 
		|| 0 == cap->redir_srv_port ){
		eag_log_err("eag_captive_start failed redir ip=%x port=%u", 
					cap->redir_srv_ip, cap->redir_srv_port );
		return EAG_ERR_CAPTIVE_REDIR_PARAM_NOT_SET;
	}

	ret = captive_shell_create( cap->capid, cap->instype,
						&(cap->redir_srv_ip), cap->redir_srv_port, EAG_IPV4 );
	if( EAG_RETURN_OK != ret ){
		eag_log_err("eag_captive_start captive_shell_create ipv4 failed!");
		//return EAG_ERR_CAPTIVE_CALL_SHELL_FAILED;
	}
	/*shell add intf*/
	for( i=0; i<cap->curr_ifnum; i++ ){
		#if EAG_SHELL_OFF
		ret = captive_iptables_add_intf( cap->capid, cap->instype, cap->cpif[i]);
		#else
		ret = captive_shell_add_intf( cap->capid, cap->instype, EAG_IPV4, cap->cpif[i]);
		#endif
		if( EAG_RETURN_OK != ret ){
			eag_log_err("eag_captive_start add ipv4 intf %s failed:%d!", cap->cpif[i],ret);
		}
	}
	
    if (eag_ins_get_ipv6_switch(cap->eagins)) {
		ret = captive_shell_create( cap->capid, cap->instype,
							&(cap->redir_srv_ipv6), cap->redir_srv_port + 1, EAG_IPV6 );
		if( EAG_RETURN_OK != ret ){
			eag_log_err("eag_captive_start captive_shell_create ipv6 failed!");
			//return EAG_ERR_CAPTIVE_CALL_SHELL_FAILED;
		}
		for( i=0; i<cap->ipv6_curr_ifnum; i++ ){
			#if EAG_SHELL_OFF
			ret = captive_ip6tables_add_intf( cap->capid, cap->instype, cap->ipv6_cpif[i]);
			#else
			ret = captive_shell_add_intf( cap->capid, cap->instype, EAG_IPV6, cap->ipv6_cpif[i]);
			#endif
			if( EAG_RETURN_OK != ret ){
				eag_log_err("eag_captive_start add ipv6 intf %s failed:%d!", cap->ipv6_cpif[i],ret);
			}
		}
	}
	#if 0
	/*shell add white list*/
	for( i=0; i<cap->white.curr_num; i++ ){
		if( RULE_IPADDR == cap->white.rule[i].type ){
			#if EAG_SHELL_OFF
			ret = captive_iptables_add_white_ip( cap->capid, cap->instype,
						cap->white.rule[i].key.ip.ipbegin, cap->white.rule[i].key.ip.ipend,
						cap->white.rule[i].key.ip.ports, cap->white.rule[i].intf );
			#else
			ret = captive_shell_add_white_ip( cap->capid, cap->instype,
						cap->white.rule[i].key.ip.ipbegin, cap->white.rule[i].key.ip.ipend,
						cap->white.rule[i].key.ip.ports, cap->white.rule[i].intf );	
			#endif
			if( EAG_RETURN_OK != ret ){
				eag_log_err("eag_captive_start shell add white list failed:%d!", ret);
			}
		}else{
			#if EAG_SHELL_OFF
			ret = captive_iptables_add_white_domain( cap->capid, cap->instype, &(cap->white.rule[i])); 
			#else
			ret = captive_shell_add_white_domain( cap->capid, cap->instype, &(cap->white.rule[i])); 
			#endif
			if( EAG_RETURN_OK != ret ){
				eag_log_err("eag_captive_add_white_list add white domain %s failed:%d", cap->white.rule[i].key.domain.name, ret );
			}
		}
	}
	/*shell add black list*/
	for( i=0; i<cap->black.curr_num; i++ ){
		if( RULE_IPADDR == cap->black.rule[i].type ){
			#if EAG_SHELL_OFF
			ret = captive_iptables_add_black_ip( cap->capid, cap->instype,
						cap->black.rule[i].key.ip.ipbegin, cap->black.rule[i].key.ip.ipend,
						cap->black.rule[i].key.ip.ports, cap->black.rule[i].intf );
			#else
			ret = captive_shell_add_black_ip( cap->capid, cap->instype,
						cap->black.rule[i].key.ip.ipbegin, cap->black.rule[i].key.ip.ipend,
						cap->black.rule[i].key.ip.ports, cap->black.rule[i].intf );
			#endif
			if( EAG_RETURN_OK != ret ){
				eag_log_err("eag_captive_start shell add black list failed:%d!", ret);
			}
		}else{
			#if EAG_SHELL_OFF
			ret = captive_iptables_add_black_domain( cap->capid, cap->instype, &(cap->black.rule[i])); 
			#else
			ret = captive_shell_add_black_domain( cap->capid, cap->instype, &(cap->black.rule[i])); 
			#endif
			if( EAG_RETURN_OK != ret ){
				eag_log_err("eag_captive_add_black_list add black domain %s failed:%d", cap->black.rule[i].key.domain.name, ret );
			}
		}
	}
	#endif
	cap->status = CAP_START;
	return EAG_RETURN_OK;
}

int
eag_captive_stop(eag_captive_t *cap)
{
	int i;
	int ret;
	
	if( CAP_STOP == cap->status  ){
		eag_log_err("eag_captive_stop failed: server not start" );
		return EAG_ERR_CAPTIVE_SERVICE_NOT_START;
	}

	#if 0
	/*shell del white list*/
	for( i=0; i<cap->white.curr_num; i++ ){
		if( RULE_IPADDR == cap->white.rule[i].type ){
			#if EAG_SHELL_OFF
			ret = captive_iptables_del_white_ip( cap->capid, cap->instype,
						cap->white.rule[i].key.ip.ipbegin, cap->white.rule[i].key.ip.ipend,
						cap->white.rule[i].key.ip.ports, cap->white.rule[i].intf );
			#else
			ret = captive_shell_del_white_ip( cap->capid, cap->instype,
						cap->white.rule[i].key.ip.ipbegin, cap->white.rule[i].key.ip.ipend,
						cap->white.rule[i].key.ip.ports, cap->white.rule[i].intf );
			#endif
			if( EAG_RETURN_OK != ret ){
				eag_log_err("eag_captive_stop shell del white list failed:%d!", ret);
			}
		}else{
			#if EAG_SHELL_OFF
			ret = captive_iptables_del_white_domain(cap->capid, cap->instype, &(cap->white.rule[i])); 	
			#endif
			//eag_log_warning("TODO: you should proc domain white list at here!");
		}
	}

	/*shell del black list*/
	for( i=0; i<cap->black.curr_num; i++ ){
		if( RULE_IPADDR == cap->black.rule[i].type ){
			#if EAG_SHELL_OFF
			ret = captive_iptables_del_black_ip( cap->capid, cap->instype,
						cap->black.rule[i].key.ip.ipbegin, cap->black.rule[i].key.ip.ipend,
						cap->black.rule[i].key.ip.ports, cap->black.rule[i].intf );
			#else
			ret = captive_shell_del_black_ip( cap->capid, cap->instype,
						cap->black.rule[i].key.ip.ipbegin, cap->black.rule[i].key.ip.ipend,
						cap->black.rule[i].key.ip.ports, cap->black.rule[i].intf );
			#endif
			if( EAG_RETURN_OK != ret ){
				eag_log_err("eag_captive_stop shell del black list failed:%d!", ret);
			}
		}else{
			#if EAG_SHELL_OFF
			ret = captive_iptables_del_black_domain( cap->capid, cap->instype, &(cap->black.rule[i])); 
			#endif
			//eag_log_warning("TODO: you should proc domain black list at here!");
		}
	}
	#endif
	/*shell del intf*/
	for( i=0; i<cap->curr_ifnum; i++ ){
		#if EAG_SHELL_OFF		
		ret = captive_iptables_del_intf( cap->capid, cap->instype, cap->cpif[i]);
		#else
		ret = captive_shell_del_intf( cap->capid, cap->instype, EAG_IPV4, cap->cpif[i]);
		#endif
		if( EAG_RETURN_OK != ret ){
			eag_log_err("eag_captive_stop del intf %s failed:%d!", cap->cpif[i],ret);
		}
	}
	if( EAG_RETURN_OK != captive_shell_destroy( cap->capid, cap->instype, EAG_IPV4 ) ){
		eag_log_err("eag_captive_stop captive_shell_destroy ipv4 failed!");
		//return EAG_ERR_CAPTIVE_CALL_SHELL_FAILED;
	}
	
    if (eag_ins_get_ipv6_switch(cap->eagins)) {
		for( i=0; i<cap->ipv6_curr_ifnum; i++ ){
	    	#if EAG_SHELL_OFF       
			ret = captive_ip6tables_del_intf( cap->capid, cap->instype, cap->ipv6_cpif[i]);
	    	#else
			ret = captive_shell_del_intf( cap->capid, cap->instype, EAG_IPV6, cap->ipv6_cpif[i]);
	    	#endif
			if( EAG_RETURN_OK != ret ){
				eag_log_err("eag_captive_stop del intf %s failed:%d!", cap->ipv6_cpif[i],ret);
			}
		}
		if( EAG_RETURN_OK != captive_shell_destroy( cap->capid, cap->instype, EAG_IPV6 ) ){
			eag_log_err("eag_captive_stop captive_shell_destroy ipv6 failed!");
			//return EAG_ERR_CAPTIVE_CALL_SHELL_FAILED;
		}
	}
	cap->status = CAP_STOP;

	#if 1 // test AC endian
	char ipv4_str[32] = "100.1.2.48";
	char ipv6_str[48] = "2013::100.1.2.48";
	struct in_addr ipv4;
	struct in6_addr ipv6;
	uint32_t cmp[4];
	memset(ipv6_str, 0, sizeof(ipv6_str));
	memset(&ipv4, 0, sizeof(ipv4));
	memset(&ipv6, 0, sizeof(ipv6));
	memset(cmp, 0, sizeof(cmp));
    inet_pton(AF_INET, ipv4_str, &ipv4);
    eag_log_info("TEST n ipv4=%#x", ipv4);
    eag_log_info("TEST h ipv4=%#x", ntohl(ipv4.s_addr));
    inet_pton(AF_INET6, ipv6_str, &ipv6);
    memcpy(&ipv6, cmp, sizeof(ipv6));
    eag_log_info("TEST n ipv6=%#x %x %x %x", cmp[0], cmp[1], cmp[2], cmp[3]);
	#endif
	return EAG_RETURN_OK;
}



int
eag_captive_is_disable(eag_captive_t * cap)
{
	if(CAP_STOP == cap->status){
		return EAG_RETURN_OK;
	}else{
		return EAG_ERR_UNKNOWN;
	}
}	

int
eag_captive_authorize(eag_captive_t * cap, struct appsession *appsession)
{
	eag_authorize_t *eag_auth = NULL;

	if (NULL == cap || NULL == appsession) {
		eag_log_err("eag_captive_authorize input params err!"
			    " cap = %p  appsession=%p", cap, appsession);
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	if (CAP_STOP == cap->status) {
		eag_log_err("eag_captive_authorize service not start!"
			    " can't authorize to capid=%u", cap->capid);
		return EAG_ERR_CAPTIVE_SERVICE_NOT_START;
	}

	if (0 == cap->isipset) {
		eag_auth = eag_authorieze_get_iptables_auth();
	} else {
		eag_auth = eag_authorize_get_ipset_auth();
	}

	if( NULL != eag_auth ){
		eag_authorize_do_authorize( eag_auth, appsession);
	}
	return EAG_RETURN_OK;
}

int
eag_captive_deauthorize(eag_captive_t * cap, struct appsession *appsession)
{
	eag_authorize_t *eag_auth = NULL;

	if (NULL == cap || NULL == appsession) {
		eag_log_err("eag_captive_deauthorize input params err!"
			    "cap=%p  appsession=%p", cap, appsession);
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	if (CAP_STOP == cap->status) {
		eag_log_err("eag_captive_deauthorize service not start!");
		return EAG_ERR_CAPTIVE_SERVICE_NOT_START;
	}

	if (0 == cap->isipset) {
		eag_auth = eag_authorieze_get_iptables_auth();
	} else {
		eag_auth = eag_authorize_get_ipset_auth();
	}

	if( NULL != eag_auth ){
		eag_authorize_de_authorize( eag_auth, appsession);
	}
	return EAG_RETURN_OK;
}
#if 0
int
eag_captive_eap_authorize(eag_captive_t * cap, unsigned int user_ip)
{
	eag_authorize_t *eag_auth = NULL;

	if (NULL == cap || 0 == user_ip) {
		eag_log_err("eag_captive_eap_authorize input params err!"
			    " cap = %p  user_ip=%d", cap, user_ip);
		return EAG_ERR_INPUT_PARAM_ERR;
	}
#if 0
	if (CAP_STOP == cap->status) {
		eag_log_err("eag_captive_authorize service not start!"
			    " can't authorize to capid=%u", cap->capid);
		return EAG_ERR_CAPTIVE_SERVICE_NOT_START;
	}
#endif
	eag_auth = eag_authorieze_get_iptables_auth();
	if( NULL != eag_auth ){
		eag_authorize_do_eap_authorize( eag_auth, user_ip);
	}
	return EAG_RETURN_OK;
}

int
eag_captive_del_eap_authorize(eag_captive_t * cap, unsigned int user_ip)
{
	eag_authorize_t *eag_auth = NULL;

	if (NULL == cap || 0 == user_ip) {
		eag_log_err("eag_captive_del_eap_authorize input params err!"
			    "cap=%p  user_ip=%d", cap, user_ip);
		return EAG_ERR_INPUT_PARAM_ERR;
	}
#if 0
	if (CAP_STOP == cap->status) {
		eag_log_err("eag_captive_deauthorize service not start!");
		return EAG_ERR_CAPTIVE_SERVICE_NOT_START;
	}
#endif
	eag_auth = eag_authorieze_get_iptables_auth();
	if( NULL != eag_auth ){
		eag_authorize_del_eap_authorize( eag_auth, user_ip);
	}
	return EAG_RETURN_OK;
}
#endif
int
eag_captive_macpre_authorize(eag_captive_t * cap, user_addr_t *user_addr)
{
	eag_authorize_t *eag_auth = NULL;

	if (NULL == cap || NULL == user_addr ) {
		eag_log_err("eag_captive_macpre_authorize input params err!"
            "cap=%p  user_addr=%p", cap, user_addr);
		return EAG_ERR_INPUT_PARAM_ERR;
	}
#if 0
	if (CAP_STOP == cap->status) {
		eag_log_err("eag_captive_authorize service not start!"
			    " can't authorize to capid=%u", cap->capid);
		return EAG_ERR_CAPTIVE_SERVICE_NOT_START;
	}
#endif
	if (0 == cap->macauth_isipset) {
		eag_auth = eag_authorieze_get_iptables_auth();
	} else {
		eag_auth = eag_authorize_get_ipset_auth();
	}
	//eag_auth = eag_authorieze_get_iptables_auth();
	if( NULL != eag_auth ){
		eag_authorize_do_macpre_authorize( eag_auth, user_addr);
	}
	return EAG_RETURN_OK;
}

int
eag_captive_del_macpre_authorize(eag_captive_t * cap, user_addr_t *user_addr)
{
	eag_authorize_t *eag_auth = NULL;

	if (NULL == cap || NULL == user_addr) {
		eag_log_err("eag_captive_del_macpre_authorize input params err!"
			    "cap=%p  user_addr=%p", cap, user_addr);
		return EAG_ERR_INPUT_PARAM_ERR;
	}
#if 0
	if (CAP_STOP == cap->status) {
		eag_log_err("eag_captive_deauthorize service not start!");
		return EAG_ERR_CAPTIVE_SERVICE_NOT_START;
	}
#endif
	if (0 == cap->macauth_isipset) {
		eag_auth = eag_authorieze_get_iptables_auth();
	} else {
		eag_auth = eag_authorize_get_ipset_auth();
	}
	//eag_auth = eag_authorieze_get_iptables_auth();
	if( NULL != eag_auth ){
		eag_authorize_del_macpre_authorize( eag_auth, user_addr);
	}
	return EAG_RETURN_OK;
}

int
eag_captive_update_session(eag_captive_t * cap,struct appsession *appsession)
{
/*TODO!!*/
	eag_log_warning
	    ("TODO:eag_captive_update_session function not completed!");
	return EAG_RETURN_OK;
}

int
eag_captive_check_flux(eag_captive_t * cap, unsigned int check_interval)
{
/*TODO!!*/
	eag_log_warning("TODO:eag_captive_check_flux function not completed!");
	return EAG_RETURN_OK;
}

struct bw_rule_t *
get_bw_rule_exist(struct bw_rules *bwrules,
				  RULE_TYPE type,
				  unsigned long ipbegin, unsigned long ipend,
				  char *ports,
				  char *domain, char *intf)
{
	struct bw_rule_t *rule;
	int i;

	for( i=0; i<bwrules->curr_num;i++){
		rule = &(bwrules->rule[i]);
		if (type == rule->type) {
			switch (type) {
			case RULE_IPADDR:
				if (rule->key.ip.ipbegin == ipbegin
				    && rule->key.ip.ipend == ipend
				    && 0 == strcmp(rule->key.ip.ports,ports)
				    && 0 == strcmp(rule->intf, intf)) {
					return rule;
				}
				break;
			case RULE_DOMAIN:
				if (NULL != domain
				    && 0 == strcmp(rule->key.domain.name, domain)
				    && 0 == strcmp(rule->intf, intf)) {
					return rule;
				}
				break;
			default:
				break;
			}
		}
	}

	return NULL;
}

int
eag_captive_add_white_list(eag_captive_t * cap,
			   RULE_TYPE type,
			   unsigned long ipbegin, unsigned long ipend,
			   /*unsigned short portbegin, unsigned short portend,*/
			   char *ports,
			   char *domain, char *intf)
{
	int ret;
	struct bw_rule_t *wrule;
	char *domain_name = NULL;
	char *ip_addr_str = NULL;
	unsigned long ip_addr = 0;
	int ip_num = 0;

	if( 0 == ipend ){
		ipend = ipbegin;
	}

	if( ipend < ipbegin ){
		eag_log_err("eag_captive_add_white_list ipend < ipbegin not permit!");
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	if (NULL == cap || (RULE_IPADDR != type && RULE_DOMAIN != type)) {
		eag_log_err("eag_captive_add_white_list input err! "\
		    	"cap=%p type=%d intf=%p intf=%s",
			     cap, type, intf, (NULL == intf) ? "" : intf);
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	if( cap->white.curr_num >= MAX_BW_RULES_NUM ){
		eag_log_err("eag_captive_add_white_list ");
		return EAG_ERR_CAPTIVE_WHITE_LIST_NUM_LIMITE;
	}

	if (NULL != domain){
		domain_name = strtok(domain,";");
	}
	if (NULL != get_bw_rule_exist(&(cap->white),
				      type,
				      ipbegin, ipend,
				      ports, domain_name, intf)) {
		eag_log_warning("eag_captive_add_white_list already exist!"\
						"type=%d ipbegin=%lu ipend=%lu ports=%s "\
						"domain=%s intf=%p intf=%s",
			     		type, ipbegin, ipend, ports, domain_name,
			     		intf, (NULL == intf) ? "" : intf);
		return EAG_ERR_CAPTIVE_RULE_AREADY_IN_WHITE;
	}

	wrule = &(cap->white.rule[cap->white.curr_num]);
	memset(wrule, 0, sizeof (struct bw_rule_t));
	wrule->type = type;

	if (NULL != intf && EAG_TRUE == is_interface_valid(intf)) {
		strncpy(wrule->intf, intf, sizeof (wrule->intf) - 1);
	}

	if (RULE_IPADDR == type) {
		eag_log_debug("eag_captive",
			      "eag_captive_add_white_list type=ipaddr");
		eag_log_debug("eag_captive",
			      "ipbegin=%lx ipend=%lx ports=%s",
			      ipbegin, ipend, ports );
		wrule->key.ip.ipbegin = ipbegin;
		wrule->key.ip.ipend = ipend;
		strncpy( wrule->key.ip.ports, ports, sizeof(wrule->key.ip.ports)-1 );
		if( CAP_START == cap->status ){
			#if EAG_SHELL_OFF
			ret = captive_iptables_add_white_ip( cap->capid, cap->instype,
					ipbegin, ipend, ports, intf );
			#else
			ret = captive_shell_add_white_ip( cap->capid, cap->instype,
					ipbegin, ipend, ports, intf );
			#endif
			if( EAG_RETURN_OK != ret ){
				eag_log_err("eag_captive_add_white_list add white ip failed:%d", ret );
				//return EAG_ERR_UNKNOWN;
			}
		}
	} else {
		if (NULL == domain || strlen(domain) == 0) {
			eag_log_err
			    ("eag_captive_add_white_list input err! type=domain,but domain=%p:%s",
			     domain, (NULL == domain) ? "" : domain);
			return EAG_ERR_INPUT_PARAM_ERR;
		}
		eag_log_debug("eag_captive",
			      "eag_captive_add_white_list type=domain  domain=%s",
			      domain);
		
		while((ip_addr_str=strtok(NULL,";"))){
			if (ip_num >= MAX_DOMAIN_IP_NUM){
				eag_log_err("eag_captive_add_white_list domain %s "
						"ip addr num %d over MAX_DOMAIN_IP_NUM = %d",domain_name,ip_num,MAX_DOMAIN_IP_NUM);
				break;
			}
			ip_addr = strtoul(ip_addr_str,NULL,10);
			eag_log_debug("eag_captive","eag_captive_add_white_list ip is %s ip_addr=%lu\n", ip_addr_str, ip_addr);
			wrule->key.domain.ip[ip_num] = ip_addr;
			ip_num += 1;			
		}
		wrule->key.domain.num = ip_num;
		strncpy(wrule->key.domain.name, domain_name, sizeof(wrule->key.domain.name) - 1);
		if( CAP_START == cap->status ){
			#if EAG_SHELL_OFF
			ret = captive_iptables_add_white_domain( cap->capid, cap->instype, wrule);		
			#else
			ret = captive_shell_add_white_domain( cap->capid, cap->instype, wrule);	
			#endif
			if( EAG_RETURN_OK != ret ){
				eag_log_err("eag_captive_add_white_list add white domain %s failed:%d", wrule->key.domain.name, ret );
				//return EAG_ERR_UNKNOWN;
			}
		}
	}

	cap->white.curr_num++;
	return EAG_RETURN_OK;
}

int
eag_captive_del_white_list(eag_captive_t *cap,
			   RULE_TYPE type,
			   unsigned long ipbegin, unsigned long ipend,
			   char *ports, char *domain, char *intf )
{
	struct bw_rule_t *wrule;
	int ret;
	unsigned long cpsize;
	
	if (NULL == cap || (RULE_IPADDR != type && RULE_DOMAIN != type)) {
		eag_log_err("eag_captive_del_white_list input err! "\
					"cap=%p type=%d intf=%p intf=%s",
				     cap, type, intf, (NULL == intf) ? "" : intf);
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	if( 0 == ipend ){
			ipend = ipbegin;
	}
	wrule = get_bw_rule_exist(&(cap->white),
				  type,
				  ipbegin, ipend,
				  ports, domain, intf);
	if (NULL == wrule) {
		eag_log_err("eag_captive_del_white_list "\
					"this whitelist rule not in captive!");
		return EAG_ERR_CAPTIVE_RULE_NOT_IN_WHITE;
	}

	/*TODO!!! del iptables */
	if (RULE_IPADDR == type) {
		eag_log_debug("eag_captive",
			      "eag_captive_del_white_list type=ipaddr");
		eag_log_debug("eag_captive",
			      "ipbegin=%lx ipend=%lx ports=%s",
			      ipbegin, ipend, ports );
		if( CAP_START == cap->status ){
			#if EAG_SHELL_OFF
			ret = captive_iptables_del_white_ip( cap->capid, cap->instype,
					ipbegin, ipend, ports, intf );
			#else
			ret = captive_shell_del_white_ip( cap->capid, cap->instype,
					ipbegin, ipend, ports, intf );
			#endif
			if( EAG_RETURN_OK != ret ){
				eag_log_err("eag_captive_del_white_list add white ip failed:%d", ret );
			}
		}

	} else {
		if( CAP_START == cap->status ){
			#if EAG_SHELL_OFF
			ret = captive_iptables_del_white_domain( cap->capid, cap->instype, wrule); 
			#else
			ret = captive_shell_del_white_domain( cap->capid, cap->instype, wrule); 
			#endif
			if( EAG_RETURN_OK != ret ){
				eag_log_err("eag_captive_del_white_list del white domain %s failed:%d", wrule->key.domain.name, ret );
			}
		}
	}

	cap->white.curr_num--;	
	if( cap->white.curr_num > 0 ){
		cpsize = (cap->white.curr_num)*sizeof(struct bw_rule_t) - 
				((unsigned long)wrule-(unsigned long)(cap->white.rule));		
		memcpy( wrule, wrule+1, cpsize );
	}
	return EAG_RETURN_OK;
}

int
eag_captive_add_black_list(eag_captive_t *cap,
			   RULE_TYPE type,
			   unsigned long ipbegin, unsigned long ipend,
			   char *ports, char *domain, char *intf)
{
	int ret;
	struct bw_rule_t *wrule;
	char *domain_name = NULL;
	char *ip_addr_str = NULL;
	unsigned long ip_addr = 0;
	int ip_num = 0;

	if( 0 == ipend ){
		ipend = ipbegin;
	}

	if( ipend < ipbegin ){
		eag_log_err("eag_captive_add_black_list ipend < ipbegin not permit!");
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	if (NULL == cap || (RULE_IPADDR != type && RULE_DOMAIN != type)) {
		eag_log_err("eag_captive_add_black_list input err! "\
				"cap=%p type=%d intf=%p intf=%s",
				 cap, type, intf, (NULL == intf) ? "" : intf);
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	if( cap->black.curr_num >= MAX_BW_RULES_NUM ){
		eag_log_err("eag_captive_add_black_list black list num limite");
		return EAG_ERR_CAPTIVE_BLACK_LIST_NUM_LIMITE;
	}

	
	if (NULL != domain){
		domain_name = strtok(domain,";");
	}

	if (NULL != get_bw_rule_exist(&(cap->black),
					  type,
					  ipbegin, ipend,
					  ports, domain, intf)) {
		eag_log_warning("eag_captive_add_black_list already exist!"\
						"type=%d ipbegin=%lu ipend=%lu ports=%s "\
						"domain=%s intf=%p intf=%s",
						type, ipbegin, ipend, ports, domain,
						intf, (NULL == intf) ? "" : intf);
		return EAG_ERR_CAPTIVE_RULE_AREADY_IN_BLACK;
	}

	wrule = &(cap->black.rule[cap->black.curr_num]);
	memset(wrule, 0, sizeof (struct bw_rule_t));
	wrule->type = type;

	if (NULL != intf && EAG_TRUE == is_interface_valid(intf)) {
		strncpy(wrule->intf, intf, sizeof (wrule->intf) - 1);
	}

	if (RULE_IPADDR == type) {
		eag_log_debug("eag_captive",
				  "eag_captive_add_black_list type=ipaddr");
		eag_log_debug("eag_captive",
				  "ipbegin=%lx ipend=%lx ports=%s",
				  ipbegin, ipend, ports );
		wrule->key.ip.ipbegin = ipbegin;
		wrule->key.ip.ipend = ipend;
		strncpy( wrule->key.ip.ports, ports, sizeof(wrule->key.ip.ports)-1 );
		if( CAP_START == cap->status ){
			#if EAG_SHELL_OFF
			ret = captive_iptables_add_black_ip( cap->capid, cap->instype,
					ipbegin, ipend, ports, intf );
			#else
			ret = captive_shell_add_black_ip( cap->capid, cap->instype,
					ipbegin, ipend, ports, intf );
			#endif
			if( EAG_RETURN_OK != ret ){
				eag_log_err("eag_captive_add_black_list add black ip failed:%d", ret );
			}
		}
	} else {
		if (NULL == domain || strlen(domain) == 0) {
			eag_log_err("eag_captive_add_black_list input err! type=domain,but domain=%p:%s",
				 domain, (NULL == domain) ? "" : domain);
			return EAG_ERR_INPUT_PARAM_ERR;
		}
		eag_log_debug("eag_captive", "eag_captive_add_black_list type=domain  domain=%s", domain);

		while((ip_addr_str=strtok(NULL,";"))){
			if (ip_num >= MAX_DOMAIN_IP_NUM){
				eag_log_err("eag_captive_add_white_list domain %s "
						"ip addr num %d over MAX_DOMAIN_IP_NUM = %d",domain_name,ip_num,MAX_DOMAIN_IP_NUM);
				break;
			}
			ip_addr = strtoul(ip_addr_str,NULL,10);
			eag_log_debug("eag_captive","eag_captive_add_white_list ip is %s ip_addr=%lu\n", ip_addr_str, ip_addr);
			wrule->key.domain.ip[ip_num] = ip_addr;
			ip_num += 1;			
		}
		wrule->key.domain.num = ip_num;
		strncpy(wrule->key.domain.name, domain_name, sizeof(wrule->key.domain.name) - 1);

		if( CAP_START == cap->status ){
			#if EAG_SHELL_OFF
			ret = captive_iptables_add_black_domain( cap->capid, cap->instype, wrule);
			#else
			ret = captive_shell_add_black_domain( cap->capid, cap->instype, wrule);
			#endif
			if( EAG_RETURN_OK != ret ){
				eag_log_err("eag_captive_add_black_list add black domain %s failed:%d", wrule->key.domain.name, ret );
			}
		}
	}

	cap->black.curr_num++;
	return EAG_RETURN_OK;
}

int
eag_captive_del_black_list(eag_captive_t *cap,
			   RULE_TYPE type,
			   unsigned long ipbegin, unsigned long ipend,
			   char *ports, char *domain, char *intf)
{
	struct bw_rule_t *wrule;
	int ret;
	unsigned long cpsize;

	if (NULL == cap || (RULE_IPADDR != type && RULE_DOMAIN != type)) {
		eag_log_err("eag_captive_del_black_list input err! "\
					"cap=%p type=%d intf=%p intf=%s",
					 cap, type, intf, (NULL == intf) ? "" : intf);
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	if( 0 == ipend ){
		ipend = ipbegin;
	}
	wrule = get_bw_rule_exist(&(cap->black),
							  type, ipbegin, ipend,
							  ports, domain, intf);
	if (NULL == wrule) {
		eag_log_err("eag_captive_del_black_list "\
					"this blacklist rule not in captive!");
		return EAG_ERR_CAPTIVE_RULE_NOT_IN_BLACK;
	}

	/*TODO!!! del iptables */
	if (RULE_IPADDR == type) {
		eag_log_debug("eag_captive",
				  "eag_captive_del_black_list type=ipaddr");
		eag_log_debug("eag_captive",
				  "ipbegin=%lx ipend=%lx ports=%s",
				  ipbegin, ipend, ports );
		if( CAP_START == cap->status ){
			#if EAG_SHELL_OFF
			ret = captive_iptables_del_black_ip( cap->capid, cap->instype,
					ipbegin, ipend, ports, intf );
			#else
			ret = captive_shell_del_black_ip( cap->capid, cap->instype,
					ipbegin, ipend, ports, intf );
			#endif
			if( EAG_RETURN_OK != ret ){
				eag_log_err("eag_captive_del_black_list add black ip failed:%d", ret );
			}
		}

	} else {
		if( CAP_START == cap->status ){
			#if EAG_SHELL_OFF
			ret = captive_iptables_del_black_domain( cap->capid, cap->instype, wrule);
			#else
			ret = captive_shell_del_black_domain( cap->capid, cap->instype, wrule);
			#endif
			if( EAG_RETURN_OK != ret ){
				eag_log_err("eag_captive_del_black_list del black domain %s failed:%d", wrule->key.domain.name, ret );
			}
		}
	}

	cap->black.curr_num--;
	if( cap->black.curr_num > 0 ){
		cpsize = cap->black.curr_num*sizeof(struct bw_rule_t) - 
				((unsigned long)wrule-(unsigned long)(cap->black.rule));
		memcpy( wrule, wrule+1, cpsize );
	}

	return EAG_RETURN_OK;
}

DBusMessage *
eag_dbus_method_conf_captive_list(
				DBusConnection *conn, 
				DBusMessage *msg, 
				void *user_data )
{
	eag_captive_t *captive = NULL;
	DBusMessage* reply = NULL;
	DBusMessageIter iter = {0};
	DBusError		err = {0};
	char *port = NULL;
	char *iprange = NULL;
	char *add_or_del = NULL;
	char *intfs = NULL;
	char *domain = NULL;
	char *white_or_black = NULL;
	int ret = -1, type = 0;
	char ipbegin[128] = {0};
	char ipend[128] = {0};
	char *ip_tmp = NULL;
	struct in_addr ipaddr_begin;
	struct in_addr ipaddr_end;
	memset(&ipaddr_begin, 0, sizeof(ipaddr_begin));
	memset(&ipaddr_end, 0, sizeof(ipaddr_end));
	
	eag_log_info("eag_dbus_method_conf_captive_list");

	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
	eag_log_err("eag_dbus_method_conf_captive_list "\
		"DBUS new reply message error!\n");
	return NULL;
	}

	captive = (eag_captive_t *)user_data;
	if( NULL == captive){
	eag_log_err("eag_dbus_method_conf_captive_list user_data error!");

	ret = EAG_ERR_UNKNOWN;
	goto replyx;
	}

	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg ,&err,
					DBUS_TYPE_UINT32, &type,
					DBUS_TYPE_STRING, &iprange,
					DBUS_TYPE_STRING, &port,
					DBUS_TYPE_STRING, &domain,
					DBUS_TYPE_STRING, &intfs,
					DBUS_TYPE_STRING, &add_or_del,
					DBUS_TYPE_STRING, &white_or_black,
					DBUS_TYPE_INVALID))){
	eag_log_err("eag_dbus_method_conf_captive_list "\
		"unable to get input args\n");
	if (dbus_error_is_set(&err)) {
		eag_log_err("eag_dbus_method_conf_captive_list %s raised:%s\n",
					err.name, err.message);
		dbus_error_free(&err);
	}
		ret = EAG_ERR_DBUS_FAILED;
		goto replyx;
	}	
	
	if((RULE_IPADDR == (RULE_TYPE)type) && (NULL != iprange) && (0 != strcmp(iprange,"")))
	{
		ip_tmp = strtok(iprange, "-");
		if(ip_tmp!=NULL)
		{
			strncpy(ipbegin, ip_tmp, sizeof(ipbegin)-1);
		}
		ip_tmp = strtok(NULL, "-");
		if(ip_tmp!=NULL)
		{
			strncpy(ipend, ip_tmp, sizeof(ipend)-1);
		}
	}

	if(strcmp(white_or_black, CP_WHITE_LIST) == 0)
	{
		if( (add_or_del!=NULL) && (0 == strcmp(add_or_del, CP_ADD_LIST)))
		{		
			inet_aton(ipbegin, &ipaddr_begin);
			inet_aton(ipend, &ipaddr_end);
			//ret = eag_captive_add_white_list(captive, (RULE_TYPE)type, ipaddr_begin.s_addr,ipaddr_end.s_addr,port,domain,intfs);
		}
		else if( (add_or_del!=NULL) && (0 == strcmp(add_or_del, CP_DEL_LIST)))
		{
			inet_aton(ipbegin, &ipaddr_begin);
			inet_aton(ipend, &ipaddr_end);
			//ret = eag_captive_del_white_list(captive, type, ipaddr_begin.s_addr,ipaddr_end.s_addr,port,domain,intfs);
		}
	}
	else if(strcmp(white_or_black, CP_BLACK_LIST) == 0)
	{
		if( (add_or_del!=NULL) && (0 == strcmp(add_or_del, CP_ADD_LIST)))
		{
			inet_aton(ipbegin, &ipaddr_begin);
			inet_aton(ipend, &ipaddr_end);
			//ret = eag_captive_add_black_list(captive, (RULE_TYPE)type, ipaddr_begin.s_addr,ipaddr_end.s_addr,port,domain,intfs);
		}
		else if( (add_or_del!=NULL) && (0 == strcmp(add_or_del, CP_DEL_LIST)))
		{		
			inet_aton(ipbegin, &ipaddr_begin);
			inet_aton(ipend, &ipaddr_end);
			//ret = eag_captive_del_black_list(captive, type, ipaddr_begin.s_addr,ipaddr_end.s_addr,port,domain,intfs);
		}
	}
	replyx:
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
						DBUS_TYPE_INT32, &ret);
	return reply;
}

struct bw_rule_t *
get_ipv6_bw_rule_exist(struct bw_rules *bwrules,
					RULE_TYPE type,
					struct in6_addr *ipv6begin, struct in6_addr *ipv6end,
					char *ports,
					char *domain, char *intf)
{
	struct bw_rule_t *rule;
	int i;

	for( i=0; i<bwrules->curr_num;i++){
		rule = &(bwrules->rule[i]);
		if (type == rule->type) {
			switch (type) {
			case RULE_IPV6ADDR:
				if (0 == memcmp(&(rule->key.ipv6.ipv6begin), ipv6begin, sizeof(struct in6_addr))
				    && 0 == memcmp(&(rule->key.ipv6.ipv6end), ipv6end, sizeof(struct in6_addr))
				    && 0 == strcmp(rule->key.ipv6.ports, ports)
				    && 0 == strcmp(rule->intf, intf)) {
					return rule;
				}
				break;
			case RULE_IPV6DOMAIN:
				if (NULL != domain
				    && 0 == strcmp(rule->key.ipv6_domain.name, domain)
				    && 0 == strcmp(rule->intf, intf)) {
					return rule;
				}
				break;
			default:
				break;
			}
		}
	}

	return NULL;
}

int
eag_captive_add_white_ipv6_list(eag_captive_t * cap,
			   RULE_TYPE type,
			   struct in6_addr *ipv6begin, struct in6_addr *ipv6end,
			   /*unsigned short portbegin, unsigned short portend,*/
			   char *ports,
			   char *domain, char *intf)
{
	int ret;
	struct bw_rule_t *wrule;
	char *domain_name = NULL;
	char *ip_addr_str = NULL;
	unsigned long ip_addr = 0;
	int ip_num = 0;
	char ipv6begin_str[48] = "";
	char ipv6end_str[48] = "";
	unsigned char ipv6_null[16];

	memset(ipv6_null, 0, sizeof(ipv6_null));
	if( 0 == memcmp(ipv6end, ipv6_null, sizeof(ipv6_null)) ){
		*ipv6end = *ipv6begin;
	}
	ipv6tostr(ipv6begin, ipv6begin_str, sizeof(ipv6begin_str));
	ipv6tostr(ipv6end, ipv6end_str, sizeof(ipv6end_str));
	
#if 0
	if( ipv6end < ipv6begin ){
		eag_log_err("eag_captive_add_white_list ipv6end < ipv6begin not permit!");
		return EAG_ERR_INPUT_PARAM_ERR;
	}
#endif
	if (NULL == cap || (RULE_IPADDR != type && RULE_DOMAIN != type)) {
		eag_log_err("eag_captive_add_white_ipv6_list input err! "\
		    	"cap=%p type=%d intf=%p intf=%s",
			     cap, type, intf, (NULL == intf) ? "" : intf);
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	if( cap->white.curr_num >= MAX_BW_RULES_NUM ){
		eag_log_err("eag_captive_add_white_list ");
		return EAG_ERR_CAPTIVE_WHITE_LIST_NUM_LIMITE;
	}

	if (NULL != domain){
		domain_name = strtok(domain,";");
	}
	if (NULL != get_ipv6_bw_rule_exist(&(cap->white),
				      type,
				      ipv6begin, ipv6end,
				      ports, domain_name, intf)) {
		eag_log_warning("eag_captive_add_white_ipv6_list already exist!"\
						"type=%d ipv6begin=%s ipv6end=%s ports=%s "\
						"domain=%s intf=%p intf=%s",
			     		type, ipv6begin_str, ipv6end_str, ports, domain_name,
			     		intf, (NULL == intf) ? "" : intf);
		return EAG_ERR_CAPTIVE_RULE_AREADY_IN_WHITE;
	}

	wrule = &(cap->white.rule[cap->white.curr_num]);
	memset(wrule, 0, sizeof (struct bw_rule_t));
	wrule->type = type;

	if (NULL != intf && EAG_TRUE == is_interface_valid(intf)) {
		strncpy(wrule->intf, intf, sizeof (wrule->intf) - 1);
	}

	if (RULE_IPADDR == type) {
		eag_log_debug("eag_captive",
			      "eag_captive_add_white_ipv6_list type=ipaddr");
		eag_log_debug("eag_captive",
			      "ipv6begin=%s ipv6end=%s ports=%s",
			      ipv6begin_str, ipv6end_str, ports );
		wrule->key.ipv6.ipv6begin = *ipv6begin;
		wrule->key.ipv6.ipv6end = *ipv6end;
		strncpy( wrule->key.ip.ports, ports, sizeof(wrule->key.ip.ports)-1 );
		if( CAP_START == cap->status ){
			ret = captive_shell_add_white_ipv6( cap->capid, cap->instype,
					ipv6begin, ipv6end, ports, intf );
			/* Not completed currently
			#if EAG_SHELL_OFF
			ret = captive_ip6tables_add_white_ip( cap->capid, cap->instype,
					ipv6begin, ipv6end, ports, intf );
			#else
			ret = captive_shell_add_white_ipv6( cap->capid, cap->instype,
					ipv6begin, ipv6end, ports, intf );
			#endif
			*/
			if( EAG_RETURN_OK != ret ){
				eag_log_err("eag_captive_add_white_ipv6_list add white ip failed:%d", ret );
				//return EAG_ERR_UNKNOWN;
			}
		}
	} else {
		/* Not completed currently
		if (NULL == domain || strlen(domain) == 0) {
			eag_log_err
			    ("eag_captive_add_white_ipv6_list input err! type=domain,but domain=%p:%s",
			     domain, (NULL == domain) ? "" : domain);
			return EAG_ERR_INPUT_PARAM_ERR;
		}
		eag_log_debug("eag_captive",
			      "eag_captive_add_white_ipv6_list type=domain  domain=%s",
			      domain);
		
		while((ip_addr_str=strtok(NULL,";"))){
			if (ip_num >= MAX_DOMAIN_IP_NUM){
				eag_log_err("eag_captive_add_white_ipv6_list domain %s "
						"ip addr num %d over MAX_DOMAIN_IP_NUM = %d",domain_name,ip_num,MAX_DOMAIN_IP_NUM);
				break;
			}
			ip_addr = strtoul(ip_addr_str,NULL,10);
			eag_log_debug("eag_captive","eag_captive_add_white_ipv6_list ip is %s ip_addr=%lu\n", ip_addr_str, ip_addr);
			wrule->key.domain.ip[ip_num] = ip_addr;
			ip_num += 1;			
		}
		wrule->key.domain.num = ip_num;
		strncpy(wrule->key.domain.name, domain_name, sizeof(wrule->key.domain.name) - 1);
		if( CAP_START == cap->status ){
			#if EAG_SHELL_OFF
			ret = captive_iptables_add_white_domain( cap->capid, cap->instype, wrule);		
			#else
			ret = captive_shell_add_white_domain( cap->capid, cap->instype, wrule);	
			#endif
			if( EAG_RETURN_OK != ret ){
				eag_log_err("eag_captive_add_white_ipv6_list add white domain %s failed:%d", wrule->key.domain.name, ret );
				//return EAG_ERR_UNKNOWN;
			}
		}
    */
	}
	cap->white.curr_num++;
	return EAG_RETURN_OK;
}

int
eag_captive_del_white_ipv6_list(eag_captive_t *cap,
				RULE_TYPE type,
				struct in6_addr *ipv6begin, struct in6_addr *ipv6end,
				char *ports, char *domain, char *intf )
{
	struct bw_rule_t *wrule;
	int ret;
	unsigned long cpsize;
	char ipv6begin_str[48] = "";
	char ipv6end_str[48] = "";
	unsigned char ipv6_null[16];
	
	if (NULL == cap || (RULE_IPADDR != type && RULE_DOMAIN != type)) {
		eag_log_err("eag_captive_del_white_ipv6_list input err! "\
					"cap=%p type=%d intf=%p intf=%s",
				     cap, type, intf, (NULL == intf) ? "" : intf);
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	memset(ipv6_null, 0, sizeof(ipv6_null));
	if( 0 == memcmp(ipv6end, ipv6_null, sizeof(ipv6_null)) ){
		*ipv6end = *ipv6begin;
	}
	ipv6tostr(ipv6begin, ipv6begin_str, sizeof(ipv6begin_str));
	ipv6tostr(ipv6end, ipv6end_str, sizeof(ipv6end_str));

	wrule = get_ipv6_bw_rule_exist(&(cap->white),
				      type,
				      ipv6begin, ipv6end,
				      ports, domain, intf);
	if (NULL == wrule) {
		eag_log_err("eag_captive_del_white_ipv6_list "\
					"this whitelist rule not in captive!");
		return EAG_ERR_CAPTIVE_RULE_NOT_IN_WHITE;
	}

	/*TODO!!! del iptables */
	if (RULE_IPADDR == type) {
		eag_log_debug("eag_captive",
			      "eag_captive_del_white_ipv6_list type=ipaddr");
		eag_log_debug("eag_captive",
			      "ipv6begin=%s ipv6end=%s ports=%s",
			      ipv6begin_str, ipv6end_str, ports );
		if( CAP_START == cap->status ){
			ret = captive_shell_del_white_ipv6( cap->capid, cap->instype,
					ipv6begin, ipv6end, ports, intf );
			/* Not completed currently
			#if EAG_SHELL_OFF
			ret = captive_iptables_del_white_ip( cap->capid, cap->instype,
					ipv6begin, ipv6end, ports, intf );
			#else
			ret = captive_shell_del_white_ip( cap->capid, cap->instype,
					ipv6begin, ipv6end, ports, intf );
			#endif
			*/
			if( EAG_RETURN_OK != ret ){
				eag_log_err("eag_captive_del_white_ipv6_list add white ip failed:%d", ret );
			}
		}

	} else {
		/* Not completed currently
		if( CAP_START == cap->status ){
			#if EAG_SHELL_OFF
			ret = captive_ip6tables_del_white_domain( cap->capid, cap->instype, wrule); 
			#else
			ret = captive_shell_del_white_domain( cap->capid, cap->instype, wrule); 
			#endif
			if( EAG_RETURN_OK != ret ){
				eag_log_err("eag_captive_del_white_ipv6_list del white domain %s failed:%d", wrule->key.domain.name, ret );
			}
		}
		*/
	}

	cap->white.curr_num--;	
	if( cap->white.curr_num > 0 ){
		cpsize = (cap->white.curr_num)*sizeof(struct bw_rule_t) - 
				((unsigned long)wrule-(unsigned long)(cap->white.rule));		
		memcpy( wrule, wrule+1, cpsize );
	}
	return EAG_RETURN_OK;
}

int
eag_captive_add_black_ipv6_list(eag_captive_t *cap,
				RULE_TYPE type,
				struct in6_addr *ipv6begin, struct in6_addr *ipv6end,
				char *ports, char *domain, char *intf)
{
	int ret;
	struct bw_rule_t *wrule;
	char *domain_name = NULL;
	char *ip_addr_str = NULL;
	unsigned long ip_addr = 0;
	int ip_num = 0;
	char ipv6begin_str[48] = "";
	char ipv6end_str[48] = "";
	unsigned char ipv6_null[16];

	memset(ipv6_null, 0, sizeof(ipv6_null));
	if( 0 == memcmp(ipv6end, ipv6_null, sizeof(ipv6_null)) ){
		*ipv6end = *ipv6begin;
	}
	ipv6tostr(ipv6begin, ipv6begin_str, sizeof(ipv6begin_str));
	ipv6tostr(ipv6end, ipv6end_str, sizeof(ipv6end_str));
	
#if 0
	if( ipv6end < ipv6begin ){
		eag_log_err("eag_captive_add_black_list ipv6end < ipv6begin not permit!");
		return EAG_ERR_INPUT_PARAM_ERR;
	}
#endif
	if (NULL == cap || (RULE_IPADDR != type && RULE_DOMAIN != type)) {
		eag_log_err("eag_captive_add_black_ipv6_list input err! "\
				"cap=%p type=%d intf=%p intf=%s",
				 cap, type, intf, (NULL == intf) ? "" : intf);
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	if( cap->black.curr_num >= MAX_BW_RULES_NUM ){
		eag_log_err("eag_captive_add_black_ipv6_list black list num limite");
		return EAG_ERR_CAPTIVE_BLACK_LIST_NUM_LIMITE;
	}

	
	if (NULL != domain){
		domain_name = strtok(domain,";");
	}

	if (NULL != get_ipv6_bw_rule_exist(&(cap->black),
					  type,
					  ipv6begin, ipv6end,
					  ports, domain, intf)) {
		eag_log_warning("eag_captive_add_black_ipv6_list already exist!"\
						"type=%d ipv6begin=%s ipv6end=%s ports=%s "\
						"domain=%s intf=%p intf=%s",
						type, ipv6begin_str, ipv6end_str, ports, domain,
						intf, (NULL == intf) ? "" : intf);
		return EAG_ERR_CAPTIVE_RULE_AREADY_IN_BLACK;
	}

	wrule = &(cap->black.rule[cap->black.curr_num]);
	memset(wrule, 0, sizeof (struct bw_rule_t));
	wrule->type = type;

	if (NULL != intf && EAG_TRUE == is_interface_valid(intf)) {
		strncpy(wrule->intf, intf, sizeof (wrule->intf) - 1);
	}

	if (RULE_IPADDR == type) {
		eag_log_debug("eag_captive",
				  "eag_captive_add_black_ipv6_list type=ipaddr");
		eag_log_debug("eag_captive",
				  "ipv6begin=%s ipv6end=%s ports=%s",
				  ipv6begin, ipv6end, ports );
		wrule->key.ipv6.ipv6begin = *ipv6begin;
		wrule->key.ipv6.ipv6end = *ipv6end;
		strncpy( wrule->key.ip.ports, ports, sizeof(wrule->key.ip.ports)-1 );
		if( CAP_START == cap->status ){
			ret = captive_shell_add_black_ipv6( cap->capid, cap->instype,
					ipv6begin, ipv6end, ports, intf );
			/* Not completed currently
			#if EAG_SHELL_OFF
			ret = captive_iptables_add_black_ip( cap->capid, cap->instype,
					ipbegin, ipend, ports, intf );
			#else
			ret = captive_shell_add_black_ip( cap->capid, cap->instype,
					ipbegin, ipend, ports, intf );
			#endif
			*/
			if( EAG_RETURN_OK != ret ){
				eag_log_err("eag_captive_add_black_ipv6_list add black ip failed:%d", ret );
			}
		}
	} else {
		/* Not completed currently
		if (NULL == domain || strlen(domain) == 0) {
			eag_log_err("eag_captive_add_black_list input err! type=domain,but domain=%p:%s",
				 domain, (NULL == domain) ? "" : domain);
			return EAG_ERR_INPUT_PARAM_ERR;
		}
		eag_log_debug("eag_captive", "eag_captive_add_black_list type=domain  domain=%s", domain);

		while((ip_addr_str=strtok(NULL,";"))){
			if (ip_num >= MAX_DOMAIN_IP_NUM){
				eag_log_err("eag_captive_add_white_list domain %s "
						"ip addr num %d over MAX_DOMAIN_IP_NUM = %d",domain_name,ip_num,MAX_DOMAIN_IP_NUM);
				break;
			}
			ip_addr = strtoul(ip_addr_str,NULL,10);
			eag_log_debug("eag_captive","eag_captive_add_white_list ip is %s ip_addr=%lu\n", ip_addr_str, ip_addr);
			wrule->key.domain.ip[ip_num] = ip_addr;
			ip_num += 1;			
		}
		wrule->key.domain.num = ip_num;
		strncpy(wrule->key.domain.name, domain_name, sizeof(wrule->key.domain.name) - 1);

		if( CAP_START == cap->status ){
			#if EAG_SHELL_OFF
			ret = captive_iptables_add_black_domain( cap->capid, cap->instype, wrule);
			#else
			ret = captive_shell_add_black_domain( cap->capid, cap->instype, wrule);
			#endif
			if( EAG_RETURN_OK != ret ){
				eag_log_err("eag_captive_add_black_list add black domain %s failed:%d", wrule->key.domain.name, ret );
			}
		}
		*/
	}

	cap->black.curr_num++;
	return EAG_RETURN_OK;
}

int
eag_captive_del_black_ipv6_list(eag_captive_t *cap,
				RULE_TYPE type,
				struct in6_addr *ipv6begin, struct in6_addr *ipv6end,
				char *ports, char *domain, char *intf)
{
	struct bw_rule_t *wrule;
	int ret;
	unsigned long cpsize;
	char ipv6begin_str[48] = "";
	char ipv6end_str[48] = "";
	unsigned char ipv6_null[16];

	if (NULL == cap || (RULE_IPADDR != type && RULE_DOMAIN != type)) {
		eag_log_err("eag_captive_del_black_list input err! "\
					"cap=%p type=%d intf=%p intf=%s",
					 cap, type, intf, (NULL == intf) ? "" : intf);
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	
	memset(ipv6_null, 0, sizeof(ipv6_null));
	if( 0 == memcmp(ipv6end, ipv6_null, sizeof(ipv6_null)) ){
		*ipv6end = *ipv6begin;
	}
	ipv6tostr(ipv6begin, ipv6begin_str, sizeof(ipv6begin_str));
	ipv6tostr(ipv6end, ipv6end_str, sizeof(ipv6end_str));
	
	wrule = get_ipv6_bw_rule_exist(&(cap->black),
							  type, ipv6begin, ipv6end,
							  ports, domain, intf);
	if (NULL == wrule) {
		eag_log_err("eag_captive_del_black_ipv6_list "\
					"this blacklist rule not in captive!");
		return EAG_ERR_CAPTIVE_RULE_NOT_IN_BLACK;
	}

	/*TODO!!! del iptables */
	if (RULE_IPADDR == type) {
		eag_log_debug("eag_captive",
				  "eag_captive_del_black_ipv6_list type=ipaddr");
		eag_log_debug("eag_captive",
				  "ipv6begin=%s ipv6end=%s ports=%s",
				  ipv6begin_str, ipv6end_str, ports );
		if( CAP_START == cap->status ){
			ret = captive_shell_del_black_ipv6( cap->capid, cap->instype,
					ipv6begin, ipv6end, ports, intf );
            /* Not completed currently
			#if EAG_SHELL_OFF
			ret = captive_iptables_del_black_ip( cap->capid, cap->instype,
					ipbegin, ipend, ports, intf );
			#else
			ret = captive_shell_del_black_ip( cap->capid, cap->instype,
					ipbegin, ipend, ports, intf );
			#endif
			*/
			if( EAG_RETURN_OK != ret ){
				eag_log_err("eag_captive_del_black_ipv6_list add black ip failed:%d", ret );
			}
		}

	} else {
		/* Not completed currently
		if( CAP_START == cap->status ){
			#if EAG_SHELL_OFF
			ret = captive_iptables_del_black_domain( cap->capid, cap->instype, wrule);
			#else
			ret = captive_shell_del_black_domain( cap->capid, cap->instype, wrule);
			#endif
			if( EAG_RETURN_OK != ret ){
				eag_log_err("eag_captive_del_black_ipv6_list del black domain %s failed:%d", wrule->key.domain.name, ret );
			}
		}
		*/
	}

	cap->black.curr_num--;
	if( cap->black.curr_num > 0 ){
		cpsize = cap->black.curr_num*sizeof(struct bw_rule_t) - 
				((unsigned long)wrule-(unsigned long)(cap->black.rule));
		memcpy( wrule, wrule+1, cpsize );
	}

	return EAG_RETURN_OK;
}

DBusMessage *
eag_dbus_method_conf_captive_ipv6_list(
				DBusConnection *conn, 
				DBusMessage *msg, 
				void *user_data )
{
	eag_captive_t *captive = NULL;
	DBusMessage* reply = NULL;
	DBusMessageIter iter = {0};
	DBusError		err = {0};
	char *port = NULL;
	char *ipv6range = NULL;
	char *add_or_del = NULL;
	char *intfs = NULL;
	char *domain = NULL;
	char *white_or_black = NULL;
	int ret = -1, type = 0;
	char ipv6begin[128] = {0};
	char ipv6end[128] = {0};
	char *ipv6_tmp = NULL;
	struct in6_addr ipv6addr_begin;
	struct in6_addr ipv6addr_end;
	memset(&ipv6addr_begin, 0, sizeof(ipv6addr_begin));
	memset(&ipv6addr_end, 0, sizeof(ipv6addr_end));
	
	eag_log_info("eag_dbus_method_conf_captive_list");

	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
	eag_log_err("eag_dbus_method_conf_captive_list "\
		"DBUS new reply message error!\n");
	return NULL;
	}

	captive = (eag_captive_t *)user_data;
	if( NULL == captive){
	eag_log_err("eag_dbus_method_conf_captive_list user_data error!");

	ret = EAG_ERR_UNKNOWN;
	goto replyx;
	}

	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg ,&err,
					DBUS_TYPE_UINT32, &type,
					DBUS_TYPE_STRING, &ipv6range,
					DBUS_TYPE_STRING, &port,
					DBUS_TYPE_STRING, &domain,
					DBUS_TYPE_STRING, &intfs,
					DBUS_TYPE_STRING, &add_or_del,
					DBUS_TYPE_STRING, &white_or_black,
					DBUS_TYPE_INVALID))){
	eag_log_err("eag_dbus_method_conf_captive_list "\
		"unable to get input args\n");
	if (dbus_error_is_set(&err)) {
		eag_log_err("eag_dbus_method_conf_captive_list %s raised:%s\n",
					err.name, err.message);
		dbus_error_free(&err);
	}
		ret = EAG_ERR_DBUS_FAILED;
		goto replyx;
	}	
	
	if((RULE_IPADDR == (RULE_TYPE)type) && (NULL != ipv6range) && (0 != strcmp(ipv6range,"")))
	{
		ipv6_tmp = strtok(ipv6range, "-");
		if(ipv6_tmp!=NULL)
		{
			strncpy(ipv6begin, ipv6_tmp, sizeof(ipv6begin)-1);
		}
		ipv6_tmp = strtok(NULL, "-");
		if(ipv6_tmp!=NULL)
		{
			strncpy(ipv6end, ipv6_tmp, sizeof(ipv6end)-1);
		}
	}

	if(strcmp(white_or_black, CP_WHITE_LIST) == 0)
	{
		if( (add_or_del!=NULL) && (0 == strcmp(add_or_del, CP_ADD_LIST)))
		{		
			inet_pton(AF_INET6, ipv6begin, &ipv6addr_begin);
			inet_pton(AF_INET6, ipv6end, &ipv6addr_end);
			ret = eag_captive_add_white_ipv6_list(captive, (RULE_TYPE)type, &ipv6addr_begin, &ipv6addr_end, port, domain, intfs);
		}
		else if( (add_or_del!=NULL) && (0 == strcmp(add_or_del, CP_DEL_LIST)))
		{
			inet_pton(AF_INET6, ipv6begin, &ipv6addr_begin);
			inet_pton(AF_INET6, ipv6end, &ipv6addr_end);
			ret = eag_captive_del_white_ipv6_list(captive, type, &ipv6addr_begin, &ipv6addr_end, port, domain, intfs);
		}
	}
	else if(strcmp(white_or_black, CP_BLACK_LIST) == 0)
	{
		if( (add_or_del!=NULL) && (0 == strcmp(add_or_del, CP_ADD_LIST)))
		{
			inet_pton(AF_INET6, ipv6begin, &ipv6addr_begin);
			inet_pton(AF_INET6, ipv6end, &ipv6addr_end);
			ret = eag_captive_add_black_ipv6_list(captive, (RULE_TYPE)type, &ipv6addr_begin, &ipv6addr_end.s6_addr, port, domain, intfs);
		}
		else if( (add_or_del!=NULL) && (0 == strcmp(add_or_del, CP_DEL_LIST)))
		{		
			inet_pton(AF_INET6, ipv6begin, &ipv6addr_begin);
			inet_pton(AF_INET6, ipv6end, &ipv6addr_end);
			ret = eag_captive_del_black_ipv6_list(captive, type, &ipv6addr_begin, &ipv6addr_end, port, domain, intfs);
		}
	}
	replyx:
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
						DBUS_TYPE_INT32, &ret);
	return reply;
}

DBusMessage *
eag_dbus_method_show_captive_intfs(
				DBusConnection *conn, 
				DBusMessage *msg, 
				void *user_data )
{
	eag_captive_t *captive = NULL;
	DBusMessage* reply = NULL;
	DBusMessageIter iter = {0};
	DBusError		err = {0};
	char *intfs = NULL;
	int ret = 0, i =0;
	eag_log_info("eag_dbus_method_show_captive_intfs");

	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		eag_log_err("eag_dbus_method_show_captive_intfs "\
					"DBUS new reply message error!\n");
		return NULL;
	}

	captive = (eag_captive_t *)user_data;
	if( NULL == captive){
		eag_log_err("eag_dbus_method_show_captive_intfs user_data error!");
		ret = EAG_ERR_UNKNOWN;
		goto replyx;
	}

	dbus_error_init(&err);

replyx:
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_INT32, &ret);	
	if( EAG_RETURN_OK == ret ){
		dbus_message_iter_init_append(reply, &iter);
		dbus_message_iter_append_basic(&iter,
										DBUS_TYPE_UINT32, &(captive->curr_ifnum));
		
		for(i=0; i<captive->curr_ifnum; i++){
			intfs = captive->cpif[i];
			dbus_message_iter_append_basic(&iter,
											DBUS_TYPE_STRING, &intfs);
		}
		dbus_message_iter_init_append(reply, &iter);
		dbus_message_iter_append_basic(&iter,
										DBUS_TYPE_UINT32, &(captive->ipv6_curr_ifnum));
		
		for(i=0; i<captive->ipv6_curr_ifnum; i++){
			intfs = captive->ipv6_cpif[i];
			dbus_message_iter_append_basic(&iter,
											DBUS_TYPE_STRING, &intfs);
		}
	}
	return reply;
}

DBusMessage *
eag_dbus_method_show_white_list(
			    DBusConnection *conn, 
			    DBusMessage *msg, 
			    void *user_data )
{
	eag_captive_t *captive = NULL;
	struct bw_rule_t *rule= NULL;
	DBusMessage* reply = NULL;
	DBusMessageIter iter = {0};
	DBusError		err = {0};
	unsigned long num = 0;
	int type = 0,zero = 0;;
	unsigned long ipv6_begin[4];
	unsigned long ipv6_end[4];
	char *ports = NULL;
	char *intf=NULL;
	char *domain=NULL;
	
	int ret = -1;

	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		eag_log_err("eag_dbus_method_show_white_list "\
					"DBUS new reply message error!\n");
		return NULL;
	}

	captive = (eag_captive_t *)user_data;
	if( NULL == captive ){
		eag_log_err("eag_dbus_method_show_white_list user_data error!");
		ret = EAG_ERR_UNKNOWN;
		goto replyx;
	}
	
	dbus_error_init(&err);		
	num = captive->white.curr_num;
	rule = &(captive->white.rule[0]);
	ret = EAG_RETURN_OK;
replyx:
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_INT32, &ret);

	if( EAG_RETURN_OK == ret ){
		dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &num);
	}

	if( EAG_RETURN_OK == ret && num > 0 ){
		int i;
		DBusMessageIter  iter_array;
		dbus_message_iter_open_container (&iter,
									DBUS_TYPE_ARRAY,
										DBUS_STRUCT_BEGIN_CHAR_AS_STRING
											DBUS_TYPE_UINT32_AS_STRING //type
											DBUS_TYPE_UINT32_AS_STRING //ipxbegin[1]
											DBUS_TYPE_UINT32_AS_STRING //ipxbegin[2]
											DBUS_TYPE_UINT32_AS_STRING //ipxbegin[3]
											DBUS_TYPE_UINT32_AS_STRING //ipxbegin[4]
											DBUS_TYPE_UINT32_AS_STRING //ipxend[1]
											DBUS_TYPE_UINT32_AS_STRING //ipxend[2]
											DBUS_TYPE_UINT32_AS_STRING //ipxend[3]
											DBUS_TYPE_UINT32_AS_STRING //ipxend[4]
											DBUS_TYPE_STRING_AS_STRING //ports
											DBUS_TYPE_STRING_AS_STRING //domain
											DBUS_TYPE_STRING_AS_STRING	//intf																			
										DBUS_STRUCT_END_CHAR_AS_STRING,
									&iter_array);

		for( i=0; i<num; i++ ){
			DBusMessageIter iter_struct;
			dbus_message_iter_open_container (&iter_array,
										DBUS_TYPE_STRUCT,
										NULL,
										&iter_struct);
			type = (int)rule[i].type;
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32, &type);
			switch(rule[i].type){						
			case RULE_IPADDR:					
				domain = "";
				ports = rule[i].key.ip.ports;
				dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &(rule[i].key.ip.ipbegin));
				dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &zero);
				dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &zero);
				dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &zero);
				dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &(rule[i].key.ip.ipend));
				dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &zero);
				dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &zero);
				dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &zero);
				dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_STRING, &ports);
				dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_STRING, &domain);
				break;
			case RULE_DOMAIN:
				domain = rule[i].key.domain.name;
				ports = "";
				dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &zero);
				dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &zero);
				dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &zero);
				dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &zero);
				
				dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &zero);
				dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &zero);
				dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &zero);
				dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &zero);
				
				dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_STRING, &ports);
				dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_STRING, &domain);
				break;
			case RULE_IPV6ADDR:					
				domain = "";
				ports = rule[i].key.ipv6.ports;
				memcpy(ipv6_begin, &(rule[i].key.ipv6.ipv6begin), sizeof(ipv6_begin));
				memcpy(ipv6_end, &(rule[i].key.ipv6.ipv6end), sizeof(ipv6_end));
				dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &ipv6_begin[0]);
				dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &ipv6_begin[1]);
				dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &ipv6_begin[2]);
				dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &ipv6_begin[3]);
				dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &ipv6_end[0]);
				dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &ipv6_end[1]);
				dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &ipv6_end[2]);
				dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &ipv6_end[3]);
				dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_STRING, &ports);
				dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_STRING, &domain);
				break;
			default:
				ports = "";
				domain = "";					
				dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &zero);
				dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &zero);
				dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &zero);
				dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &zero);
				
				dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &zero);
				dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &zero);
				dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &zero);
				dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &zero);
				
				dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_STRING, &ports);
				dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_STRING, &domain);					
				break;
			}

			intf = rule[i].intf;
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_STRING, &intf);

			dbus_message_iter_close_container (&iter_array, &iter_struct);

		}
		dbus_message_iter_close_container (&iter, &iter_array);
	}
	return reply;
}

DBusMessage *
eag_dbus_method_show_black_list(
			    DBusConnection *conn, 
			    DBusMessage *msg, 
			    void *user_data )
{
	eag_captive_t *captive = NULL;
	struct bw_rule_t *rule= NULL;
	DBusMessage* reply = NULL;
	DBusMessageIter iter = {0};
	DBusError		err = {0};
	unsigned long num = 0;
	int type = 0,zero = 0;;
	unsigned long ipv6_begin[4];
	unsigned long ipv6_end[4];
	char *ports = NULL;
	char *intf=NULL;
	char *domain=NULL;
	
	int ret = 0;

	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		eag_log_err("eag_dbus_method_show_black_list "\
					"DBUS new reply message error!\n");
		return NULL;
	}

	captive = (eag_captive_t *)user_data;
	if( NULL == captive ){
		eag_log_err("eag_dbus_method_show_black_list user_data error!");
		ret = EAG_ERR_UNKNOWN;
		goto replyx;
	}
	
	dbus_error_init(&err);		
	num = captive->black.curr_num;
	rule = captive->black.rule;

replyx:
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_INT32, &ret);

	if( EAG_RETURN_OK == ret ){
		dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &num);
	}

	if( EAG_RETURN_OK == ret && num > 0 ){
		int i;
		DBusMessageIter  iter_array;
		dbus_message_iter_open_container (&iter,
									DBUS_TYPE_ARRAY,
										DBUS_STRUCT_BEGIN_CHAR_AS_STRING
											DBUS_TYPE_UINT32_AS_STRING //type
											DBUS_TYPE_UINT32_AS_STRING //ipxbegin[1]
											DBUS_TYPE_UINT32_AS_STRING //ipxbegin[2]
											DBUS_TYPE_UINT32_AS_STRING //ipxbegin[3]
											DBUS_TYPE_UINT32_AS_STRING //ipxbegin[4]
											DBUS_TYPE_UINT32_AS_STRING //ipxend[1]
											DBUS_TYPE_UINT32_AS_STRING //ipxend[2]
											DBUS_TYPE_UINT32_AS_STRING //ipxend[3]
											DBUS_TYPE_UINT32_AS_STRING //ipxend[4]
											DBUS_TYPE_STRING_AS_STRING //ports
											DBUS_TYPE_STRING_AS_STRING //domain
											DBUS_TYPE_STRING_AS_STRING	//intf																		
										DBUS_STRUCT_END_CHAR_AS_STRING,
									&iter_array);

		for( i=0; i<num; i++ ){
			DBusMessageIter iter_struct;
			dbus_message_iter_open_container (&iter_array,
										DBUS_TYPE_STRUCT,
										NULL,
										&iter_struct);
			type = (int)rule[i].type;
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_UINT32, &type);
			switch(rule[i].type){						
            case RULE_IPADDR:                   
                domain = "";
                ports = rule[i].key.ip.ports;
                dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &(rule[i].key.ip.ipbegin));
                dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &zero);
                dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &zero);
                dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &zero);
                dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &(rule[i].key.ip.ipend));
                dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &zero);
                dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &zero);
                dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &zero);
                dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_STRING, &ports);
                dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_STRING, &domain);
                break;
            case RULE_DOMAIN:
                domain = rule[i].key.domain.name;
                ports = "";
                dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &zero);
                dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &zero);
                dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &zero);
                dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &zero);
                
                dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &zero);
                dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &zero);
                dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &zero);
                dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &zero);
                
                dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_STRING, &ports);
                dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_STRING, &domain);
                break;
            case RULE_IPV6ADDR:                 
                domain = "";
                ports = rule[i].key.ipv6.ports;
                memcpy(ipv6_begin, &(rule[i].key.ipv6.ipv6begin), sizeof(ipv6_begin));
                memcpy(ipv6_end, &(rule[i].key.ipv6.ipv6end), sizeof(ipv6_end));
                dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &ipv6_begin[0]);
                dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &ipv6_begin[1]);
                dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &ipv6_begin[2]);
                dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &ipv6_begin[3]);
                dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &ipv6_end[0]);
                dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &ipv6_end[1]);
                dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &ipv6_end[2]);
                dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &ipv6_end[3]);
                dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_STRING, &ports);
                dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_STRING, &domain);
                break;
            default:
                ports = "";
                domain = "";                    
                dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &zero);
                dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &zero);
                dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &zero);
                dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &zero);
                
                dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &zero);
                dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &zero);
                dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &zero);
                dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &zero);
                
                dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_STRING, &ports);
                dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_STRING, &domain);                    
                break;
            }

			intf = rule[i].intf;
			dbus_message_iter_append_basic(&iter_struct,
											DBUS_TYPE_STRING, &intf);

			dbus_message_iter_close_container (&iter_array, &iter_struct);

		}
		dbus_message_iter_close_container (&iter, &iter_array);
	}
	return reply;
}

int
eag_captive_set_eagins(eag_captive_t *captive,
		eag_ins_t *eagins)
{
	if (NULL == captive || NULL == eagins) {
		eag_log_err("eag_captive_set_eagins input error");
		return -1;
	}

	captive->eagins = eagins;

	return EAG_RETURN_OK;
}

int
eag_captive_set_redir(eag_captive_t *captive,
		eag_redir_t *redir)
{
	if (NULL == captive || NULL == redir) {
		eag_log_err("eag_captive_set_redir input error");
		return -1;
	}

	captive->redir = redir;

	return EAG_RETURN_OK;
}

#if 0
struct list_head *
eag_captive_get_black_list(struct eag_captive_t *cap)
{
	return &(cap->black);
}
#endif
#ifdef eag_captive_test

#include "eag_errcode.c"
#include "eag_log.c"

#include "eag_mem.c"
#include "eag_blkmem.c"
#include "eag_util.c"
#include "eag_iptables.c"

int
main()
{
	struct eag_captive_t *cap;
	eag_log_init("captive");

	cap = eag_captive_new(1,1);
	printf("11111\n");
	if (NULL == cap) {
		eag_log_err("create eag captive failed!");
		return -1;
	}

	eag_captive_set_redir_srv(cap, 1000, 20, NULL);
	printf("2222\n");

	eag_captive_add_interface(cap, "vlan1");
	printf("3333\n");

#if 0
	eag_captive_add_white_list(cap, RULE_IPADDR, 1, 3, "1,3", NULL,
				   "eth0-1");
	printf("444\n");
	eag_captive_add_white_list(cap, RULE_IPADDR, 1, 3, "1,3", NULL,
				   "eth0-2");
	printf("5555\n");
	eag_captive_add_white_list(cap, RULE_IPADDR, 1, 3, "1,3", NULL, "eth0-2");	/*should failed */
	printf("6666\n");
	eag_captive_add_white_list(cap, RULE_IPADDR, 1, 3, "1,3", NULL, "eth0-3");	/*should failed  eth0-3 not in captive */
	printf("7777\n");

	eag_captive_del_white_list(cap, RULE_IPADDR, 1, 3, "1,3", NULL,
				   "eth0-2");
printf("888\n");	
	eag_captive_del_white_list(cap, RULE_IPADDR, 1, 3, "1,3", NULL, "eth0-2");	/*should failed eth0-2 not in captive */
	printf("999\n");
	eag_captive_add_black_list(cap, RULE_IPADDR, 1, 3, "1,3", NULL,
				   "eth0-1");
	printf("aaaa\n");
	eag_captive_add_black_list(cap, RULE_IPADDR, 1, 3, "1,3", NULL,
				   "eth0-2");
	printf("bbbb\n");

	eag_captive_add_black_list(cap, RULE_IPADDR, 1, 3, "1,3", NULL, "eth0-2");	/*should failed */
	printf("cccc\n");

	eag_captive_add_black_list(cap, RULE_IPADDR, 1, 3, "1,3", NULL, "eth0-3");	/*should failed  eth0-3 not in captive */
	printf("ddd\n");

	//eag_captive_del_black_list(cap, RULE_IPADDR, 1, 3, "1,3", NULL, "eth0-2");
		printf("eee\n");
	//eag_captive_del_black_list(cap, RULE_IPADDR, 1, 3, "1,3", NULL, "eth0-2");	/*should failed */

	printf("fff\n");
	sleep(3);
#endif	
	eag_captive_start(cap);
	printf("ggg\n");

	sleep(5);
	printf("before connect up!\n");
	connect_up( 1, 1, "vlan1", 0xf0010203);
	connect_up( 1, 1, "vlan1", 0x10010203);
	printf("after connect up!\n");
	sleep(10);
	printf("before connect down!\n");
	connect_down( 1, 1, "vlan1", 0xf0010203);
	connect_down( 1, 1, "vlan1", 0x10010203);
	printf("after connect down!\n");

	sleep(10);

	eag_captive_stop(cap);

	
	printf("oooo\n");
	eag_captive_del_interface(cap, 4, "eth0");
	printf("ppp\n");
	eag_captive_del_interface(cap, 4, "eth1");
		printf("qqq\n");
	eag_captive_free(cap);
		printf("rrrr\n");
	eag_log_uninit();
	return 0;
}

#endif

