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
* mibs_public.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
*
* DESCRIPTION:
* define subagent's public function.
*
*
*******************************************************************************/


#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <syslog.h>

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "board/board_define.h"
#include "ac_manage_def.h"
#include "ws_dbus_def.h"

#include "mibs_public.h"


static manage_session *snmp_session = NULL;


/**********************************************************************************
*  ip_long2str
*
*  DESCRIPTION:
*	 convert ip (ABCD) format to (A.B.C.D)
*
*  INPUT:
*	 ipAddress - string (ABCD)
*  
*  OUTPUT:
*	  null
*
*  RETURN:
*	  buff - ip (A.B.C.D)
*	  
**********************************************************************************/
/*
 unsigned int ip_long2str(unsigned long ipAddress,unsigned char **buff)
 {
	 unsigned long	 cnt;
	 unsigned char *tmpPtr = *buff;
 
	 cnt = sprintf((char*)tmpPtr,"%ld.%ld.%ld.%ld",(ipAddress>>24) & 0xFF, \
			 (ipAddress>>16) & 0xFF,(ipAddress>>8) & 0xFF,ipAddress & 0xFF);
	 
	 return cnt;
 }
*/
/**********************************************************************************
 *	dcli_ip2ulong
 *
 *	DESCRIPTION:
 *		convert IP (A.B.C.D) to IP (ABCD) pattern
 *
 *	INPUT:
 *		str - (A.B.C.D)
 *	
 *	OUTPUT:
 *		null
 *
 *	RETURN:
 *		
 *		IP	-  ip (ABCD)
 *		
 **********************************************************************************/



unsigned long dcli_ip2ulong(char *str)
{
	char *sep=".";
	char *token;
	unsigned long ip_long[4]; 
	unsigned long ip = 0;
	int i = 1;
	
	token=strtok(str,sep);
	if(token)
	{
		ip_long[0] = strtoul(token,NULL,10);
		while((token!=NULL)&&(i<4))
		{
			token=strtok(NULL,sep);
			ip_long[i] = strtoul(token,NULL,10);
			i++;
		}
		
		ip=(ip_long[0]<<24)+(ip_long[1]<<16)+(ip_long[2]<<8)+ip_long[3];
	}

	return ip;
}

/*
int mac_format_check
(
	char* str,
	int len
) 
{
	int i = 0;
	unsigned int result = NPD_SUCCESS;
	char c = 0;
	
    if( 17 != len){
	   return NPD_FAIL;
	}

	
	for(;i<len;i++) {
			c = str[i];
			if((2 == i)||(5 == i)||(8 == i)||(11 == i)||(14 == i)){
				if((':'!=c)&&('-'!=c))
					return NPD_FAIL;
			}
			else if((c>='0'&&c<='9')||
				(c>='A'&&c<='F')||
				(c>='a'&&c<='f'))
				continue;
			else {
				result = NPD_FAIL;
				return result;
			}

	}
	if((str[2] != str[5])||(str[2] != str[8])||(str[2] != str[11])||(str[2] != str[14])||
		(str[5] != str[8])||(str[5] != str[11])||(str[5] != str[14])||
		(str[8] != str[11])||(str[8] != str[14])){
		
        result = NPD_FAIL;
		return result;
	}
	

	return result;
}


int parse_mac_addr(char* input,ETHERADDR *  macAddr)
 {
 	
	int i = 0;
	char cur = 0,value = 0;
	
	if((NULL == input)||(NULL == macAddr)) {
		return NPD_FAIL;
	}
	if(NPD_FAIL == mac_format_check(input,strlen(input))) {
		return NPD_FAIL;
	}
	
	for(i = 0; i <6;i++) {
		cur = *(input++);
		if(cur == ':') {
			i--;
			continue;
		}
		if((cur >= '0') &&(cur <='9')) {
			value = cur - '0';
		}
		else if((cur >= 'A') &&(cur <='F')) {
			value = cur - 'A';
			value += 0xa;
		}
		else if((cur >= 'a') &&(cur <='f')) {
			value = cur - 'a';
			value += 0xa;
		}
		macAddr->arEther[i] = value;
		cur = *(input++);	
		if((cur >= '0') &&(cur <='9')) {
			value = cur - '0';
		}
		else if((cur >= 'A') &&(cur <='F')) {
			value = cur - 'A';
			value += 0xa;
		}
		else if((cur >= 'a') &&(cur <='f')) {
			value = cur - 'a';
			value += 0xa;
		}
		macAddr->arEther[i] = (macAddr->arEther[i]<< 4)|value;
	}
	
	return NPD_SUCCESS;
} 


int is_muti_brc_mac(ETHERADDR *mac)
{
  if(mac->arEther[0] & 0x1)
  	return 1;
  else{ return 0;}
}
*/

int maskstr2int( char *mask )
{
	unsigned int iMask, m0, m1, m2, m3;
	char binarystr[64]="";
	int i,iRet;
	
	sscanf(mask,"%u.%u.%u.%u", &m3,&m2,&m1,&m0);
	iMask = m3*256*256*256 + m2*256*256 + m1*256 + m0;
	
	iRet = 0;
	for( i=0; i < 32 ;i++ )
	{
		if((iMask&1) == 1 )
		{
			binarystr[31-i] = '1';
			iRet ++;
		}
		else
		{
			binarystr[31-i] = '0';	
		}
		iMask = iMask >> 1;
	}
	
	if( strstr( binarystr, "01" ) )
	{
		return -1;	
	}
	
	return iRet;
}

void get_dev_oid(char *enterprise_pvivate_oid)
{
	char *buf = (char*)malloc(256);
	char * enterprise_snmp = (char*)malloc(256);
	char *snmp_sys_snmp = (char*)malloc(256);
	char *enter_tmp = (char*)malloc(20);
     FILE *enterprise_snmp_oid;
    FILE *snmp_sys_oid;
	int n = 0;
	memset(buf,0,256);
	memset(enterprise_snmp,0,256);
	memset(snmp_sys_snmp,0,256);
	memset(enter_tmp,0,20);
	enterprise_snmp_oid = fopen(ENTERPRISE_SNMP_OID,"r");
	if(enterprise_snmp_oid != NULL)
	{
		fgets(buf,256,enterprise_snmp_oid);
	}
	strcpy(enterprise_snmp,"1.3.6.1.4.1.");
	n = strlen(buf);
	if(n < 2)
	{
		strcpy(enter_tmp,"0");
	}
	else
	{
		strncpy(enter_tmp,buf,n-1);
	}
	if(strcmp(enter_tmp,"0")==0)
		{
		 strcat(enterprise_snmp,"31656");
		}
	else
		{
			strcat(enterprise_snmp,enter_tmp);

		}
	strcat(enterprise_pvivate_oid,enterprise_snmp);
	strcat(enterprise_pvivate_oid,".");
	memset(buf,0,256);
	snmp_sys_oid = fopen(SNMP_SYS_OID,"r");
	if(snmp_sys_oid != NULL)
	{
		fgets(buf,256,snmp_sys_oid);
	}
	n = 0;
	n = strlen(buf);
	if(n < 2)
	{
		strcpy(snmp_sys_snmp,"0");
	}
	else
	{
		strncpy(snmp_sys_snmp,buf,n-1);
	}
    if(strcmp(snmp_sys_snmp,"0")!=0)
    	{
			strcat(enterprise_pvivate_oid,snmp_sys_snmp);
			strcat(enterprise_pvivate_oid,".");
		}

	free(buf);
	free(enter_tmp);
	free(enterprise_snmp);
	free(snmp_sys_snmp);
	if(enterprise_snmp_oid != NULL)
	{
		fclose(enterprise_snmp_oid);
	}
	if(snmp_sys_oid != NULL)
	{
		fclose(snmp_sys_oid);
	}
	
}


void get_number(char * input_char,char *number_char )
 {   
  	char   a[20]; 
	strcpy(a,input_char);
  	char   b[12];   
  	int   i=0;   
  	int   j=0;   
    
  	for(i=0;i<=strlen(a);i++)   
  	{   
        if   ((int)a[i]>=48&&(int)a[i]<=57)   
    	{
			b[j]=a[i];   
			j++;
		}   
    }
	memcpy(number_char,b,4);
}   


void mad_dev_oid(oid *argv_oid, char * table_oid, size_t *len,char * oid_private)
	{
	char *p = NULL;
	char * private_oid = (char*)malloc(128);
	memset(private_oid,0,128);
	strcpy(private_oid,oid_private);
	strcat(private_oid,".");
	strcat(private_oid,table_oid);
	//memset(p,0,10);
	//int i = 0;
	p = strtok(private_oid,".");
	int count = 0;
	while( NULL != p)
		{
			 argv_oid[count++] = (oid)atoi(p);
			 p = strtok(NULL,".");
		}
*len = count;
//free(p);
free(private_oid);
}

/*É¾³ý×Ö·û´®½áÎ²µÄ»»ÐÐ*/
void delete_enter(char * string)
{
	if(string == NULL)
		return;
	
	int len = 0;
	len = strlen(string);
    int len_l = 0;
	char * tmp = string;
	while(*tmp != '\n')
	{
		len_l++;
		if(len_l >= len)
			return;
		tmp++;
	}
	*tmp = '\0';	
}

void
snmp_tipc_session_init(void) {
	manage_session session;
	manage_tipc_addr local_tipc;
	memset(&session, 0, sizeof(manage_session));
	memset(&local_tipc, 0, sizeof(manage_tipc_addr));

	session.flags |= MANAGE_FLAGS_TIPC_SOCKET;

	unsigned int local_slot_id = 0;
	if(VALID_DBM_FLAG == get_dbm_effective_flag())
	{
		local_slot_id = get_product_info(PRODUCT_LOCAL_SLOTID);
	}
	if(0 == local_slot_id || local_slot_id > SLOT_MAX_NUM) {
		syslog(LOG_WARNING, "snmp_tipc_session_init: get active local slot id failed!\n");
		return ;
	}

	local_tipc.type = SNMP_TIPC_TYPE;	/*ac sample tipc type*/
	local_tipc.instance = (0x1000 + local_slot_id);

	session.local = (void *)&local_tipc;
	session.local_len = sizeof(manage_tipc_addr);

	snmp_session = manage_open(&session);
	if(NULL == snmp_session) {
		syslog(LOG_ERR, "snmp_tipc_session_init: s_manage_errno = %d, s_errno = %d\n",
						session.s_manage_errno, session.s_errno);
		return ;
	}
}

manage_session *
snmp_get_tipc_session(void) {
	return snmp_session;
}