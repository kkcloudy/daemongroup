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
* dcli_captive.h
*
* MODIFY:
*		by <chensheng@autelan.com> on ...
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
*  		$Revision: 1.4 $	
*******************************************************************************/
#ifndef __DCLI_CAPTIVE_H__
#define __DCLI_CAPTIVE_H__

#define CAPTIVE_IP_CHECK_SUCCESS		0
#define CAPTIVE_IP_CHECK_FAILURE		1

#define CAPTIVE_MAC_CHECK_SUCESS		0
#define CAPTIVE_MAC_CHECK_FAILURE		1

#define CAPTIVE_INTERFACES_CHECK_SUCCESS		0
#define CAPTIVE_INTERFACES_CHECK_FAILURE		1

#define CAPTIVE_SUCCESS		0
#define CAPTIVE_FAILURE		1

#define PORTAL_CONF_PATH	"/opt/services/conf/portal_conf.conf"
#define PORTAL_LIST_PATH	"/opt/services/option/portal_option"

#define SCRIPT_PATH					"sudo /usr/bin/"
#define ADD_WHITE_LIST_CMD			SCRIPT_PATH"cp_add_white_list.sh %d %s %s > /dev/null 2>&1"
#define DEL_WHITE_LIST_CMD			SCRIPT_PATH"cp_del_white_list.sh %d %s %s > /dev/null 2>&1"
#define ADD_WHITE_LIST_DOMAIN_CMD	SCRIPT_PATH"cp_add_white_list_domain.sh %d %s > /dev/null 2>&1"
#define DEL_WHITE_LIST_DOMAIN_CMD	SCRIPT_PATH"cp_del_white_list_domain.sh %d %s > /dev/null 2>&1"
#define ADD_BLACK_LIST_CMD			SCRIPT_PATH"cp_add_black_list.sh %d %s %s > /dev/null 2>&1"
#define DEL_BLACK_LIST_CMD			SCRIPT_PATH"cp_del_black_list.sh %d %s %s > /dev/null 2>&1"
#define ADD_BLACK_LIST_DOMAIN_CMD	SCRIPT_PATH"cp_add_black_list_domain.sh %d %s > /dev/null 2>&1"
#define DEL_BLACK_LIST_DOMAIN_CMD	SCRIPT_PATH"cp_del_black_list_domain.sh %d %s > /dev/null 2>&1"

#define CP_WHITE_LIST_FLAG				0
#define CP_BLACK_LIST_FLAG				1
#define CP_WHITE_LIST_FLAG_DOMAIN		3
#define CP_BLACK_LIST_FLAG_DOMAIN		4

#define MAX_CMD_LEN					256
#define MAX_LINE_LEN				256

#define IP_ADDR_LEN					24
#define IPRANGE_LEN					40
#define PORTSET_LEN					40
#define IPV6_ADDR_LEN				48
#define IPV6RANGE_LEN				80

#define MAX_ID_NUM					8
#define MAX_ID_NUM_DOMAIN			8

#define MAX_WHITE_LIST_NUM			50
#define MAX_WHITE_LIST_NUM_DOMAIN	512
#define MAX_BLACK_LIST_NUM			50
#define MAX_BLACK_LIST_NUM_DOMAIN	512

typedef struct {
	char iprange[IPRANGE_LEN];
	char portset[PORTSET_LEN];
} iprange_portset_t;

typedef struct {
	char ipv6range[IPV6RANGE_LEN];
	char portset[PORTSET_LEN];
} ipv6range_portset_t;

typedef struct {
	int id;
	int num;
	iprange_portset_t list[MAX_WHITE_LIST_NUM];
}white_list_t;

typedef struct {
	int id;
	int num;
	iprange_portset_t list[MAX_BLACK_LIST_NUM];
}black_list_t;

enum WHITELIST_OPERATE_TYPE
{
	WHITELIST_OPERATE_ADD,
	WHITELIST_OPERATE_DEL
};

enum BLACKLIST_OPERATE_TYPE
{
	BLACKLIST_OPERATE_ADD,
	BLACKLIST_OPERATE_DEL
};

int parse_captive_portal_id(char *str, int *id);
int getRecordById( int id, char *record, int len );
int captive_check_ip_format(const char *str);
int captive_check_ipv6_format(const char *str);
int captive_check_interfaces_format(const char *str, char *err, int size);
int captive_check_mac_format(char * mac,int len);

int captive_check_portset_format(const char *str);
int parse_iprange_portset(const char *str, iprange_portset_t *item);
int parse_ipv6range_portset(const char *str, ipv6range_portset_t *item);
int get_white_list(int id, white_list_t *p_list);
int find_in_white_list(const white_list_t *p_list, const iprange_portset_t *p_item);
int get_black_list(int id, black_list_t *p_list);
int find_in_black_list(const black_list_t *p_list, const iprange_portset_t *p_item);

#endif
