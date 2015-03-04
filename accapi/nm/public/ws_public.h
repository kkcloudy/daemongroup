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
* ws_public.h
*
*
* CREATOR:
* autelan.software.Network Dep. team
*
*
* DESCRIPTION:
* function for web
*
*
***************************************************************************/
#ifndef _WS_PUBLIC_H
#define _WS_PUBLIC_H

#include <sys/types.h>
#include <sys/socket.h>


#define IFI_NAME 16
#define	IFI_HADDR 8

#define LONG_SORT 600
#define SHORT_SORT 120

#ifndef CMD_SUCCESS
#define CMD_SUCCESS 0
#endif
#ifndef CMD_FAILURE
#define CMD_FAILURE -1
#endif

#if 0
#define TRAP_CONF_PATH "/opt/www/htdocs/trap/trap_conf.xml"
#else
#define TRAP_CONF_PATH "/opt/services/option/trapconf_option"
#endif

#define SNMP_THREAD_INFO_PATH   "/var/run/snmp_thread_info"

#define MAX_IF_IFNAME_LEN		32
#define WEB_IPMASK_STRING_MAXLEN	(strlen("255.255.255.255"))
#define WEB_IPMASK_STRING_MINLEN	(strlen("0.0.0.0"))


typedef struct ifi_info
{
  char ifi_name[IFI_NAME];
  u_char ifi_haddr[IFI_HADDR];
  u_short ifi_hlen;
  short ifi_flags;
  short ifi_myflags;
  struct sockaddr *ifi_addr;
  struct sockaddr *ifi_brdaddr;
  struct sockaddr *ifi_dstaddr;
  struct sockaddr *ifi_mask;
  struct ifi_info *ifi_next;
}ifi_info;

typedef struct {
    unsigned int trunkMbr[4];
}TRUNK_MEMBER_BMP;

typedef struct {
    unsigned int portMbr[2];
}PORT_MEMBER_BMP;

struct radius_req_rate {
    unsigned int local_id;
    unsigned int instance_id;

    unsigned int access_accept_rate;
};


#define GET_CMD_STDOUT(buff,buff_size,cmd)\
	{\
		FILE *fp;\
		fp = popen( cmd,"r" );\
		if( NULL != fp ){\
			memset(buff,0,sizeof(buff));\
			fgets( buff, buff_size, fp );\
			pclose(fp);\
		}\
	}


typedef struct inf
{
	char if_addr[256];
	char if_name[32];
	char if_stat[32];
	char if_mask[32];
	int  upflag;
	struct inf *next;
}infi;


typedef struct if_t {
	char ifname[MAX_IF_IFNAME_LEN];
	char ipaddr[32];
	char ipaddr_ipv6[50];
	unsigned int prefix;
	unsigned int mask;
	struct if_t *next;
} if3;

typedef struct if_list_t{
	int if_num;
	if3 *if_head;
} if_list_p;

#define IFI_ALIAS 1
struct ifi_info *get_ifi_info(int, int);
struct ifi_info *get_ifi_info_v6(int, int);
extern void free_ifi_info(ifi_info *ifihead);

extern ifi_info *get_ifi_info(int family, int doaliases);
extern ifi_info *get_ifi_info_v6(int family, int doaliases);
extern char *sock_ntop(const struct sockaddr *sa, socklen_t salen);

extern int interface_list_ioctl (int af,struct inf * interface);

extern void free_inf(infi * infter);
extern unsigned long dcli_ip2ulong(char *str);

extern void check_trap_conf_file(void);

extern void Free_get_all_if_info(if_list_p *iflist_t);
/*返回0时，调用Free_get_all_if_info释放空间*/
extern int get_all_if_info( if_list_p *iflist_t );

extern int get_int_from_file(char *filename);
extern int get_str_from_file(char *filename, char *buff);
extern int ifname2ifindex_by_ioctl(const char *dev);

extern int checkIpFormatValid(char *ipAddress);

extern int trap_name_is_legal_input(const char *str) ;

extern int snmp_ipaddr_is_legal_input(const char *str);

extern int ve_interface_parse(const char *str, char *ifname, unsigned int size);

extern int ac_trap_get_flag(char *fpath);

extern void ac_trap_set_flag(char *fpath,int debug_em_switch);

extern int ac_preempt_switch_trap_has_sent(void);

extern int ac_preempt_interval_trap_has_sent(void);
#endif 
