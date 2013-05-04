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
* mibs_public.h
*
*
* CREATOR:
* autelan.software.Network Dep. team
*
* DESCRIPTION:
* Corresponding mibs_public.c
*
*
*******************************************************************************/

#ifndef _MIBS_PUBLIC_H
#define _MIBS_PUBLIC_H

#include "manage_type.h"
#include "manage_api.h"

#define ENTERPRISE_SNMP_OID "/devinfo/enterprise_snmp_oid"
#define SNMP_SYS_OID		"/devinfo/snmp_sys_oid"

#define MIN(a,b) ( (a)<(b) ? (a):(b) )

//将192.169.1.1格式的地址转化成int型的地址
#define INET_ATON(ipaddr,addr_str)	\
		{\
			unsigned int a1,a2,a3,a4;\
			int ret;\
			ipaddr = 0;\
			if( NULL != addr_str )\
			{\
				ret = sscanf(addr_str,"%u.%u.%u.%u",&a1,&a2,&a3,&a4);\
				if( ret == 4 ){\
					ipaddr = a1*256*256*256+a2*256*256+a3*256+a4;\
				}\
			}\
		}
//将int 32的值转化成ip地址字符串
#define INET_NTOA(ip_int,addr_str)\
		{\
			if( NULL != addr_str )\
			{\
				unsigned int a1,a2,a3,a4;\
				unsigned int ip_uint = (unsigned int)ip_int;\
				a1 = (ip_uint&0xff000000)>>24;\
				a2 = (ip_uint&0x00ff0000)>>16;\
				a3 = (ip_uint&0x0000ff00)>>8;\
				a4 = (ip_uint&0x000000ff);\
				sprintf( addr_str, "%d.%d.%d.%d", a1,a2,a3,a4 );\
			}\
		}
		
//读取命令行的stdout,只读取第一行
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

/////////////////

struct ver_wtp{
	char *id;
	char *faddr;
	char *usrname;
	char *passwd;
	char *fname;
	char *apmode;
	struct ver_wtp *next;
};

#define MAX_ESSID_LEN 	32
#define MAX_IP_LEN 		32
#define MAX_PORT_LEN 	8
#define MAX_WEB_LEN 	1024
#define MAX_DOMAIN_LEN 	128

#define MIB_ERROR 	-1
#define MIB_OK 		0

#define SNMP_TIPC_TYPE	(0x7001)

int maskstr2int(char *mask);
void get_number(char * input_char, char *number_char);
void get_dev_oid(char *enter_oid);
void mad_dev_oid(oid *argv_oid, char * table_oid, size_t *len, char * oid_private);
void delete_enter(char * string);

void snmp_tipc_session_init(void);

manage_session *snmp_get_tipc_session(void);

#endif

