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
* dcli_acl.c
*
*
* CREATOR:
*		qinhs@autelan.com
*
* DESCRIPTION:
*		CLI definition for ACL module.
*
* DATE:
*		02/21/2008	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.110 $	
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif
#include <zebra.h>
#include <dbus/dbus.h>
#include <stdlib.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sysdef/npd_sysdef.h>
#include <dbus/npd/npd_dbus_def.h>
#include "vty.h"
#include "dcli_vlan.h"
#include "dcli_acl.h"
#include "command.h"
#include "if.h"
#include "sysdef/returncode.h"
/* for distributed acl */
#include "dcli_main.h"  /* it must be here */
#include "dcli_sem.h"
#include "board/board_define.h"

typedef char boolean;

#define DCLI_IPMASK_STRING_MAXLEN	(strlen("255.255.255.255/32"))
#define DCLI_IPMASK_STRING_MINLEN	(strlen("0.0.0.0/0"))
#define DCLI_IP_STRING_MAXLEN	(strlen("255.255.255.255"))
#define DCLI_IP_STRING_MINLEN   (strlen("0.0.0.0"))


struct cmd_node acl_node_distributed = 
{
	ACL_NODE_DISTRIBUTED,
	"%s(config-acl-sw-board)# "
};
struct cmd_node acl_group_node = 
{
	ACL_GROUP_NODE,
	"%s(config-ingress-acl-group)# "
};
struct cmd_node egress_acl_group_node = 
{
	ACL_EGRESS_GROUP_NODE,
	"%s(config-egress-acl-group)# "
};
struct cmd_node time_range_node =
{
	TIME_RANGE_NODE,
	"%s(config-time-range)# "
};

extern DBusConnection *dcli_dbus_connection;
extern int is_distributed;
static DBusConnection *dcli_dbus_connection_acl;
static DBusConnection *dcli_dbus_connection_acl_port;
extern int mac_format_check(char * str,int len);
extern int parse_mac_addr(char * input,ETHERADDR * macAddr);

int
str2_ipv6_addr
( 
	char *str,
	struct ipv6addr *p
)
{
	int ret;

	ret = inet_pton(AF_INET6, str, p->ipbuf);
	if (ret != 1) {
		return 0;
	}

	return ret;
}

int INDEX_LENTH_CHECK(unsigned char *str,unsigned int num)
{
	char c=0;
	
	c = str[0];
	if(!(c<='9'&&c>'0')){
		return CMD_WARNING;
	}
	else{
		if(num<(strlen(str)))
		{
		  return CMD_WARNING;
		}
	}
}

#define VALUE_IP_MASK_CHECK(num) 					\
if((num<1)||(num>32)) 		\
 {													\
		vty_out(vty,"%% Illegal ip mask value!\n");    \
		return CMD_WARNING;							\
 }

int dcli_checkPoint(char *ptr)
{
	int ret = 0;
	while(*ptr != '\0') {
		if(((*ptr) < '0')||((*ptr) > '9')){
			ret = 1;
	 		break;
		}
		*ptr++;
	}
	return ret;
}

/**********************************************************************************
 *  dcli_str2ulong
 *
 *	DESCRIPTION:
 * 		convert string to long interger
 *
 *	INPUT:
 *		str - string
 *	
 *	OUTPUT:
 *		val - value
 *
 * 	RETURN:
 *		
 *		 ACL_RETURN_CODE_ERROR -
 *       ACL_RETURN_CODE_SUCCESS -
 *		
 **********************************************************************************/
int dcli_str2ulong(char *str,unsigned int *Value)
{
	char *endptr = NULL;
	char c = 0;
	int ret = 0;
	if (NULL == str) return ACL_RETURN_CODE_ERROR;

	ret = dcli_checkPoint(str);
	if(ret == 1){
		return ACL_RETURN_CODE_ERROR;
	}

	c = str[0];
	if((strlen(str) > 1)&&('0' == c)){
		/* string(not single "0") should not start with '0'*/
		return ACL_RETURN_CODE_ERROR;
	}		
	*Value= strtoul(str,&endptr,10);
	if('\0' != endptr[0]){
		return ACL_RETURN_CODE_ERROR;
	}
	return ACL_RETURN_CODE_SUCCESS;	
}

/**********************************************************************************
 *  dcli_ip2ulong
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

unsigned long dcli_ip2ulong(char *str)
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
/**********************************************************************************
 *  dcli_ip2ulong
 *
 *	DESCRIPTION:
 * 		Get IP And IP mask Lenth from input string (A.B.C.D/M) 
 *
 *	INPUT:
 *		str - (A.B.C.D/M) 
 *	
 *	OUTPUT:
 *		null
 *
 * 	RETURN:
 *		
 *		IP	-  ip
 *	    mask - ip mask
 *		
 **********************************************************************************/

int	ip_address_format2ulong(char ** buf,unsigned long *ipAddress,unsigned int *mask)
{
	char *strcp = NULL;
	char *split = "/";
	char *token1 = NULL,*token2 = NULL;
	int Val = 0,ret = 0;
	unsigned int ipMask = 0;
	int i = 0, length = 0;
	int splitCount=0;
	char * str = NULL;
	
	if(NULL == buf||NULL == *buf){
        return CMD_WARNING;
	}	
	str = *buf;
	length = strlen(str);
	if( length > DCLI_IPMASK_STRING_MAXLEN ||  \
		length < DCLI_IP_STRING_MINLEN ){
		return CMD_WARNING;
	}
	if((str[0] > '9')||(str[0] < '1')){
		return CMD_WARNING;
	}
	for(i = 0; i < length; i++){
		if('/' == str[i]){
            splitCount++;
			if((i == length - 1)||('0' > str[i+1])||(str[i+1] > '9')){
                return CMD_WARNING;
			}
		}
		if((str[i] > '9'||str[i]<'0') &&  \
			str[i] != '.' &&  \
			str[i] != '/' &&  \
			str[i] != '\0'
		){
			return CMD_WARNING;
		}
	}
	if(1 != splitCount){
        return CMD_WARNING;
	}
	
	strcp=(char *)malloc(50*sizeof(char));
	if (NULL == strcp) {
		return CMD_WARNING;
	}
	memset(strcp,'\0',50);
	strcpy(strcp,str);	

	token1 = strtok(strcp,split);
	token2 = token1;/*token2 is ip string*/
	if(NULL != token1) {
		token1 = strtok(NULL,split);/*token1 is mask string*/
		ret = dcli_str2ulong(token1,&ipMask);
		if (ACL_RETURN_CODE_ERROR == ret) {
			free(strcp);
			strcp = NULL;
			return CMD_WARNING;
		}
		if((ipMask<0)||(ipMask>32))
		{
			free(strcp);
			strcp = NULL;
			return CMD_WARNING;	
		}
		else
		{
			*mask = ipMask;
		}
	}
	
	if(CMD_SUCCESS != parse_ip_check(token2)){
		free(strcp);
		strcp = NULL;
		return CMD_WARNING;
	}
    *ipAddress = dcli_ip2ulong(token2);	
	free(strcp);
	strcp = NULL;
	return CMD_SUCCESS;
}
/*
 *parse_ip_check
 */
int parse_ip_check(char * str)
	{
	
		char *sep=".";
		char *token = NULL;
		unsigned long ip_long[4] = {0}; 
		int i = 0;
		int pointCount=0;
		char ipAddr[DCLI_IP_STRING_MAXLEN+1]={0};
		if(str==NULL||strlen(str)>DCLI_IP_STRING_MAXLEN || \
			strlen(str) < DCLI_IP_STRING_MINLEN ){
			return CMD_WARNING;
		}
		if((str[0] > '9')||(str[0] < '1')){
			return CMD_WARNING;
		}
		for(i=0;i<strlen(str);i++){
			ipAddr[i]=str[i];
			if('.' == str[i]){
                pointCount++;
				if((i == strlen(str)-1)||('0' > str[i+1])||(str[i+1] > '9')){
					return CMD_WARNING;
				}
			}
			if((str[i]>'9'||str[i]<'0')&&str[i]!='.'&&str[i]!='\0'){
				return CMD_WARNING;
			}
		}
		if(3 != pointCount){
            return CMD_WARNING;
		}
		token=strtok(ipAddr,sep);
		if((NULL == token)||("" == token)||(strlen(token) < 1)||\
			((strlen(token) > 1) && ('0' == token[0]))){
			return CMD_WARNING;
		}
		if(NULL != token){
		    ip_long[0] = strtoul(token,NULL,10);
		}
		else {
			return CMD_WARNING;
		}
		i=1;
		while((token!=NULL)&&(i<4))
		{
			token=strtok(NULL,sep);
			if((NULL == token)||("" == token)||(strlen(token) < 1)|| \
				((strlen(token) > 1) && ('0' == token[0]))){
				return CMD_WARNING;
			}
			if(NULL != token){
			    ip_long[i] = strtoul(token,NULL,10);
			}
			else {
				return CMD_WARNING;
			}
			i++;
		}
		for(i=0;i<4;i++){
            if(ip_long[i]>255){
                return CMD_WARNING;
			}
		}
		return CMD_SUCCESS;
		
}



#define DCLI_IP_STRING_MAXLEN	(strlen("255.255.255.255"))

int is_muti_brc_ip(char *str)
{
		char *sep=".";
		char *token = NULL;
		char ipAddr[DCLI_IP_STRING_MAXLEN+1]={0};
		int i;
		int multi = 0;
		for(i=0;i<strlen(str);i++){
			ipAddr[i]=str[i];
		}	
		token = strtok(ipAddr,sep);
		multi  = atoi(token);
		if ((239 >=  multi) && (multi >= 224)) {
			return 1;
		}
		for (i = 3;i > 0;i--) {
			token=strtok(NULL,sep);
		}
		multi  = atoi(token);
		if (multi == 255) {
			return 1;
		}
		return 0;
}


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
 unsigned int ip_long2str(unsigned long ipAddress,unsigned char **buff)
 {
	 unsigned long	 cnt = 0;
	 unsigned char *tmpPtr = *buff;
 
	 cnt = sprintf((char*)tmpPtr,"%ld.%ld.%ld.%ld",(ipAddress>>24) & 0xFF, \
			 (ipAddress>>16) & 0xFF,(ipAddress>>8) & 0xFF,ipAddress & 0xFF);
	 
	 return cnt;
 }
 /****************************************************************
 *FUN:timeRange_name_legal_check
 *Params :
 *	
 *
 *	 OUT ALIAS_NAME_LEN_ERROR--time name too long
 *		 ALIAS_NAME_HEAD_ERROR--illegal char on head of time name 
 *		 ALIAS_NAME_BODY_ERROR--unsupported char in time name
 ****************************************************************/
 int timeRange_name_legal_check(char* str,unsigned int len)
 {
	 int i = 0;
	 int ret = ACL_RETURN_CODE_ERROR;
	 char c = 0;
	 if((NULL == str)||(len==0)){
		 return ret;
	 }
	 if(len >20){
		 ret = ALIAS_NAME_LEN_ERROR;
		 return ret;
	 }
 
	 c = str[0];
	 if( (c=='_')||
		 (c<='z'&&c>='a')||
		 (c<='Z'&&c>='A')
	   ){
		 ret = ACL_RETURN_CODE_SUCCESS;
	 }
	 else {
		 return ALIAS_NAME_HEAD_ERROR;
	 }
	 for (i=1;i<=len-1;i++){
		 c = str[i];
		 if( (c>='0' && c<='9')||
			 (c<='z'&&c>='a')||
			 (c<='Z'&&c>='A')||
			 (c=='_')
			 ){
			 continue;
		 }
		 else {
			 ret =ALIAS_NAME_BODY_ERROR;
			 break;
		 }
	 }
	 return ret;
 }
 int timeRange_absolute_deal
 (
 	char *str,
 	unsigned int *startyear,
 	unsigned int *startmonth,
 	unsigned int *startday,
 	unsigned int *starthour,
 	unsigned int *startminute
 )
 {
	 
	 int   len=0,ret = ACL_RETURN_CODE_ERROR;
	 char *endptr1 = NULL,*endptr2 = NULL,*endptr3 = NULL;
	 char *endptr4 = NULL,*endptr5 = NULL;
	 

	 len=strlen(str);
	 if((NULL == str)||(len==0)){
		 return ret;
	 }
	 if(len >= (ALIAS_NAME_SIZE+1)){
		 ret = ALIAS_NAME_LEN_ERROR;
		 return ret;
	 }
	 *startyear=strtoul(str,&endptr1,10);
	
	 if(endptr1){
		if(TIME_SPLIT_DASH==endptr1[0]){			
			*startmonth=strtoul((char *)&endptr1[1],&endptr2,10);			
			if(TIME_SPLIT_DASH==endptr2[0]){
				*startday=strtoul((char *)&endptr2[1],&endptr3,10);				
				if(TIME_SPLIT_SLASH==endptr3[0]){
					*starthour=strtoul((char *)&endptr3[1],&endptr4,10);					
					if(TIME_SPLIT_SLASH==endptr4[0]){
						*startminute=strtoul((char *)&endptr4[1],&endptr5,10);						
						if('\0'==endptr5[0]){
							 return ACL_RETURN_CODE_SUCCESS;
						 }
						else{
							return ACL_RETURN_CODE_ERROR;
						}
					}
					
				}
				
			}
			
		}
		
	 }
	 return ACL_RETURN_CODE_ERROR;
 }
 
int timeRange_time_deal(char *str,unsigned int *hour,unsigned int *minute)
{

	int   len=0,ret = ACL_RETURN_CODE_ERROR;
	char *endptr1 = NULL,*endptr2 = NULL;
	

	len=strlen(str);
	
	if((NULL == str)||(len==0)){
		return ret;
	}
	if(len > 5){
		ret = ALIAS_NAME_LEN_ERROR;
		return ret;
	}
	*hour=strtoul(str,&endptr1,10);
	if(endptr1){ 
	   if(TIME_SPLIT_SLASH==endptr1[0]){
		   *minute=strtoul((char *)&endptr1[1],&endptr2,10);
		   if('\0'==endptr2[0]){
				return ACL_RETURN_CODE_SUCCESS;
			}
		   else{
				return ACL_RETURN_CODE_ERROR;
		   }
	   	}
	}
	return ACL_RETURN_CODE_ERROR;
}
int timeRange_time_check_illegal
(	 
	unsigned int sm,
	unsigned int sd,
	unsigned int sh,
	unsigned int smt	
)
{
	unsigned int ret =ACL_TRUE;
	
	if((sm>12)||(sm<1))
		return ACL_FALSE;
	if((sd>31)||(sd<1))
		return ACL_FALSE;
	if((sh>24)||(sh<0))
		return ACL_FALSE;
	if((smt>60)||(smt<0))
		return ACL_FALSE;

	return ret;

}
 int timeRange_time_hour_check_illegal
 (	  
	 unsigned int sh,
	 unsigned int sm 
 )
 {
	 unsigned int ret =ACL_TRUE;
	 
	 if((sh>24)||(sh<0))
		 return ACL_FALSE;
	 if((sm>60)||(sm<0))
		 return ACL_FALSE;
 
	 return ret;
 
 }

int dcli_acl_rule_show_running_config()
{
	
    char *showStr = NULL,*cursor = NULL,ch = 0,tmpBuf[SHOWRUN_PERLINE_SIZE] = {0};
    int ret = 0;
    DBusMessage *query = NULL, *reply = NULL;
    DBusError err;

	int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
    int local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);	
	int function_type = -1;
	int i = 0;

	char file_path[64] = {0};

    if(is_distributed == DISTRIBUTED_SYSTEM)
    {
		for(i = 1; i <= slotNum; i++)
		{
			sprintf(file_path,"/dbm/product/slot/slot%d/function_type", i);
			function_type = get_product_info(file_path);
			
			if (function_type == SWITCH_BOARD)
			{
	 
                query = dbus_message_new_method_call(  NPD_DBUS_BUSNAME,\
                							 	NPD_DBUS_ACL_OBJPATH, \
                							 	NPD_DBUS_ACL_INTERFACE, \
                							 	NPD_DBUS_METHOD_SHOW_ACL_RULE_RUNNIG_CONFIG);
                						 
                dbus_error_init(&err);

                if(NULL == dbus_connection_dcli[i]->dcli_dbus_connection) 				
                {
                	if(i == local_slot_id) 
                	{
                	    reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
                	}
                	else 
                	{	
                		printf("Can not connect to slot %d .\n", i);
                		continue;
                	}
                }
				else 
                {
                	reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[i]->dcli_dbus_connection, query, -1, &err);
                }

   
                dbus_message_unref(query);

                if (NULL == reply)
                {
                    printf("failed get reply, Plese check slot %d\n",i);
                    if (dbus_error_is_set(&err))
                    {
                        dbus_error_free_for_dcli(&err);
                    }
                    return CMD_WARNING;
                }

                if (dbus_message_get_args ( reply, &err,
                		 DBUS_TYPE_STRING, &showStr,
                		 DBUS_TYPE_INVALID)) 
                {
                    char _tmpstr[64];
                    memset(_tmpstr,0,64);
                    sprintf(_tmpstr,"\n!ACL section slot %d.\n",i);
                    vtysh_add_show_string(_tmpstr);
                    vtysh_add_show_string(showStr);
                    ret = CMD_SUCCESS;
                } 
                else 
                {
                    printf("Failed get args of slot %d.\n",i);				
                    if (dbus_error_is_set(&err)) 
                    {
                        dbus_error_free_for_dcli(&err);
                    }
                }
                dbus_message_unref(reply);
			}
		}
    }
	 return ret; 
}

int dcli_acl_group_show_running_config(struct vty *vty)
{	 

    char *showStr = NULL,*cursor = NULL,ch = 0,tmpBuf[SHOWRUN_PERLINE_SIZE] = {0};
    int ret = CMD_SUCCESS;
    DBusMessage *query = NULL, *reply = NULL;
    DBusError err;

	int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
    int local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);	
	int product_type = get_product_info(SEM_PRODUCT_TYPE_PATH);
	int function_type = -1;
	int i = 0;

	char file_path[64] = {0};

    if(is_distributed == DISTRIBUTED_SYSTEM)
    {
		for(i = 1; i <= slotNum; i++)
		{
			sprintf(file_path,"/dbm/product/slot/slot%d/function_type", i);
			function_type = get_product_info(file_path);
			
			if (function_type == SWITCH_BOARD)
			{
                	 
                query = dbus_message_new_method_call(  NPD_DBUS_BUSNAME,\
                								 	NPD_DBUS_ACL_OBJPATH, \
                								 	NPD_DBUS_ACL_INTERFACE, \
                								 	NPD_DBUS_METHOD_SHOW_ACL_GROUP_RUNNIG_CONFIG);

                dbus_error_init(&err);

                if(NULL == dbus_connection_dcli[i]->dcli_dbus_connection) 				
                {
                	if(i == local_slot_id) 
                	{
                	    reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
                	}
                	else 
                	{	
                		printf("Can not connect to slot %d .\n", i);
                		continue;
                	}
                }
				else 
                {
                	reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[i]->dcli_dbus_connection, query, -1, &err);
                }

   
                dbus_message_unref(query);

                if (NULL == reply)
                {
                    printf("failed get reply, Plese check slot %d\n",i);
                    if (dbus_error_is_set(&err))
                    {
                        dbus_error_free_for_dcli(&err);
                    }
                    return CMD_WARNING;
                }
				
                if (dbus_message_get_args ( reply, &err,
                			 DBUS_TYPE_STRING, &showStr,
                			 DBUS_TYPE_INVALID)) 
                {
                    char _tmpstr[64];
                    memset(_tmpstr,0,64);
                    sprintf(_tmpstr,"\n!ACL GROUP section slot %d.\n",i);
                    vtysh_add_show_string(_tmpstr);
                    vtysh_add_show_string(showStr);
                    ret = CMD_SUCCESS;
                } 
                else 
                {
                    printf("Failed get args of slot %d.\n",i);					
                    if (dbus_error_is_set(&err)) 
                    {
                     dbus_error_free_for_dcli(&err);
                    }
                }
    			dbus_message_unref(reply);				
			}
		}
    }	 
	return ret; 
}
int dcli_acl_qos_show_running_config(struct vty* vty)
{	
	char *showStr = NULL,*cursor = NULL,ch = 0,tmpBuf[SHOWRUN_PERLINE_SIZE] = {0};
	int ret = 0;
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;


	int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
    int local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);	
	int product_type = get_product_info(SEM_PRODUCT_TYPE_PATH);
	int function_type = -1;
	int i = 0;

	char file_path[64] = {0};

    if(is_distributed == DISTRIBUTED_SYSTEM)
    {
		for(i = 1; i <= slotNum; i++)
		{
			sprintf(file_path,"/dbm/product/slot/slot%d/function_type", i);
			function_type = get_product_info(file_path);
			
			if (function_type == SWITCH_BOARD)
			{

            	query = dbus_message_new_method_call(  NPD_DBUS_BUSNAME,\
            										   NPD_DBUS_ACL_OBJPATH, \
            										   NPD_DBUS_ACL_INTERFACE, \
            										   NPD_DBUS_METHOD_SHOW_ACL_QOS_RUNNIG_CONFIG);

            	dbus_error_init(&err);

                if(NULL == dbus_connection_dcli[i]->dcli_dbus_connection) 				
                {
                	if(i == local_slot_id) 
                	{
                	    reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
                	}
                	else 
                	{	
                		printf("Can not connect to slot %d .\n", i);
                		continue;
                	}
                }
				else 
                {
                	reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[i]->dcli_dbus_connection, query, -1, &err);
                }

                dbus_message_unref(query);

                if (NULL == reply)
                {
                    printf("failed get reply, Plese check slot %d\n",i);
                    if (dbus_error_is_set(&err))
                    {
                        dbus_error_free_for_dcli(&err);
                    }
                    return CMD_WARNING;
                }
				
            	if (dbus_message_get_args ( reply, &err,
            					DBUS_TYPE_STRING, &showStr,
            					DBUS_TYPE_INVALID)) 
            	{
            	
            		char _tmpstr[64];
            		memset(_tmpstr,0,64);
            		sprintf(_tmpstr,"\n!ACL QOS section slot %d.\n",i);
            		vtysh_add_show_string(_tmpstr);
            		vtysh_add_show_string(showStr);
            		ret = 0;
            	} 
            	else 
            	{
                    printf("Failed get args of slot %d.\n",i);					
            		if (dbus_error_is_set(&err)) 
            		{
            			dbus_error_free_for_dcli(&err);
            		}
            	}

            	dbus_message_unref(reply);
			}
		}
    }

	/* show Qos polier rule info, zhangdi@autelan.com 2013-03-11 */
	dcli_qos_policer_show_running_config();
	
	/* show ACL rule info */	
	dcli_acl_rule_show_running_config();
	
	return ret; 
}


/* add for distributed acl */
DEFUN(config_acl_on_board_cmd_func,
	  config_acl_on_board_cmd,
	  "config acl switch-board <1-10>",
	  CONFIG_STR
	  "Configure acl of Switch-board\n"
	  "Configure swtich-board on slot N\n"
	  "Slot id of swtich-board \n"
)
{
	unsigned int dist_slot = 0; 
	int ret = 0;
   	unsigned int nodeSave = 0;
   	int local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
	int function_type = -1;
	char file_path[64] = {0};

	
	ret = dcli_str2ulong((char*)argv[0],&dist_slot);
	if (ACL_RETURN_CODE_ERROR == ret) {
		vty_out(vty,"%% Illegal slot number!\n");
		return CMD_WARNING;
	}

	if((dist_slot < 1)||(dist_slot > 10))
	{
		vty_out(vty, "%% Slot number out range!\n");
		return CMD_WARNING;
	}

	if(is_distributed == DISTRIBUTED_SYSTEM)
    {

		/* check if the right board */
    	sprintf(file_path,"/dbm/product/slot/slot%d/function_type", dist_slot);
    	function_type = get_product_info(file_path);
    	
    	if (function_type != SWITCH_BOARD)
    	{
    		vty_out(vty, "Slot %d is not Switch-board, Please select another !\n", dist_slot);	
    		return CMD_WARNING;
    	}		

		/* send CMD */
    	if(NULL == dbus_connection_dcli[dist_slot]->dcli_dbus_connection) 				
    	{
			vty_out(vty, "Can not connect to slot %d.\n", dist_slot);
			return CMD_WARNING;
    	}
		else 
    	{
			vty_out(vty, "<<========== Config acl switch-board slot: %d =========>>\n",dist_slot);
            dcli_dbus_connection_acl = dbus_connection_dcli[dist_slot]->dcli_dbus_connection;			

			/*vty_out(vty,"Enter inner CMD node...\n");*/
			if(CONFIG_NODE == vty->node)
			{
				vty->node = ACL_NODE_DISTRIBUTED;
				nodeSave = dist_slot;
				vty->index = (void*)nodeSave;/*when not add & before vlanId, the Vty enter <config-line> CMD Node.*/
			}
			else{
				vty_out (vty, "Terminal mode change must under configure mode!\n", VTY_NEWLINE);
				return CMD_WARNING;
			}
		}
    }
	return CMD_SUCCESS;
}

 
DEFUN(config_acl_rule_trap_ip_cmd_func,
	  config_acl_rule_trap_ip_cmd,
	  "acl (standard|extended) <1-1000> trap ip dip ( A.B.C.D/M |any) sip ( A.B.C.D/M |any)",
	  ACL_STR
	  "Standard acl rule\n"
	  "Extended acl rule\n"
	  "Standard rule index range in 1-1000, extended rule index range in 1-500\n"
	  "Acl rule action Trap-to-Cpu\n"
	  "Acl rule deal with IP packet\n"
	  "Destination IP address for IP packet\n"
	  "Specify IP address in <A.B.C.D/M> format\n"
	  "Any legal destination IP address\n"
	  "Source IP address for IP packet\n"
	  "Specify IP address in <A.B.C.D/M> format\n"
	  "Any legal source IP address\n"
)
{
	DBusMessage 	*query = NULL, *reply = NULL;
	DBusError 		err;
	unsigned int    ruleIndex = 0,sipmaskLen = 0,dipmaskLen = 0;
	unsigned long 	dipno = 0, sipno = 0;
	unsigned  int	ruleType = 0,i = 0;
	unsigned long	op_ret = 0;
	int Val=0,ret = 0;
	
	if((argc < 2)||(argc > 4))
	{
		vty_out(vty, "%% bad parameters number!\n");
		return CMD_WARNING;
	}
	if(strncmp("standard",argv[0],strlen(argv[0]))==0)
	{
		ruleType = STANDARD_ACL_RULE;
	}
	else if (strncmp("extended",argv[0],strlen(argv[0]))==0)
	{
		ruleType = EXTENDED_ACL_RULE;
	}
	else
	{
		vty_out(vty,"bad command parameter!\n");
		return CMD_WARNING;
	}
	
	ret = dcli_str2ulong((char*)argv[1],&ruleIndex);
	if (ACL_RETURN_CODE_ERROR == ret) {
		vty_out(vty,"%% Illegal rule index!\n");
		return CMD_WARNING;
	}

	if ((ruleType == EXTENDED_ACL_RULE)&&(ruleIndex > MAX_EXT_RULE_NUM)) {
		vty_out(vty,"extended rule must less than 500!\n");
		return CMD_WARNING;
	}	

	ruleIndex = ruleIndex-1;
	
	if(strncmp("any",argv[2],strlen(argv[2]))==0){
		   dipmaskLen=0;
		   dipno=0;
	}
	else{
		   Val=ip_address_format2ulong((char**)&argv[2],&dipno,&dipmaskLen);
		   if(CMD_WARNING == Val) {
				vty_out(vty, "%% Bad parameter %s\n", argv[2]);
				return CMD_WARNING;
		   }
		   VALUE_IP_MASK_CHECK(dipmaskLen);	
		  	   
	}
	
	if(strncmp("any",argv[3],strlen(argv[3]))==0){ 
			sipmaskLen=0;
		  	sipno=0;
		}
	else{
        Val=ip_address_format2ulong((char**)&argv[3],&sipno,&sipmaskLen);
		if(CMD_WARNING == Val) {
			 vty_out(vty, "%% Bad parameter %s\n", argv[3]);
			 return CMD_WARNING;
		}
		VALUE_IP_MASK_CHECK(sipmaskLen);
	}
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_ACL_OBJPATH,	\
					NPD_DBUS_ACL_INTERFACE, NPD_DBUS_METHOD_CONFIG_ACL_RULE_TRAP_IP);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&ruleIndex,				  
							 DBUS_TYPE_UINT32,&ruleType,
							 DBUS_TYPE_UINT32,&sipmaskLen,
							 DBUS_TYPE_UINT32,&dipmaskLen,
							 DBUS_TYPE_UINT32,&dipno,
							 DBUS_TYPE_UINT32,&sipno,
							 DBUS_TYPE_INVALID);
		   
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_acl,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{
			if (ACL_RETURN_CODE_SUCCESS == op_ret ) 
			{
				/*vty_out(vty,"Access-list %d is successfully set!\n",(ruleIndex+1));*/
			}	
			else if(ACL_RETURN_CODE_GLOBAL_EXISTED == op_ret)
				vty_out(vty,"%% Access-list %d existed!\n",(ruleIndex+1));
			else if((ACL_RETURN_CODE_EXT_NO_SPACE==op_ret)&&(ruleType == STANDARD_ACL_RULE))
		         	vty_out(vty,"%% because the extended should take up twice spaces than stdard acl,but %d has been set extended!\n",(ruleIndex-511));
			else if((ACL_RETURN_CODE_EXT_NO_SPACE==op_ret)&&(ruleType == EXTENDED_ACL_RULE))
				vty_out(vty,"%% because the extended rule should take up twice spaces than standard acl,\
				 			  but %d has been set standard rule! ~~set fail\n",(ruleIndex+513));
			else if(ACL_RETURN_CODE_SAME_FIELD == op_ret)
				vty_out(vty,"%% identical fields of packet can not set again\n");
			else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret)
				vty_out(vty,"%% Product not support this function!\n");
			else 
				vty_out(vty,"%% set fail!\n");/*maybe acl index supass range,or malloc fail*/
	} 
	else 
	{
		/*vty_out(vty,"Failed get args.\n");*/
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}		
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(config_acl_rule_trap_udp_cmd_func,
	  config_acl_rule_trap_udp_cmd,
	  "acl standard <1-1000> trap udp dip (A.B.C.D/M |any) dst-port (<0-65535>|any) sip (A.B.C.D/M |any) src-port (<0-65535>|any)",
	  ACL_STR
	  "Standard acl rule\n"
	  "Standard rule index range in 1-1000\n"
	  "Acl rule action Trap-to-Cpu\n"
	  "Acl rule deal with UDP packet\n"
	  "Destination IP address for UDP packet\n"
	  "Specify IP address in <A.B.C.D/M> format\n"
	  "Any legal destination IP address\n"
	  "Destination port with UDP packet\n"
	  "Specify Port number in range 0-65535\n"
	  "Any legal destination port number\n"
      "Source IP address for UDP packet\n"
	  "Specify IP address in <A.B.C.D/M> format\n"
	  "Any legal source IP address\n"
	  "Source port with UDP packet\n"
	  "Specify Port number in range 0-65535\n"
	  "Any legal source port number\n"
)
{
	DBusMessage 	*query = NULL, *reply = NULL;
	DBusError 		err;
	unsigned int 	ruleIndex = 0,sipmaskLen = 0,dipmaskLen = 0;
	unsigned long 	dipno = 0, sipno = 0;
	unsigned int   srcport = 0,dstport = 0;
	unsigned int	ruleType = 0,ret = 0,i = 0;
	unsigned long	op_ret = 0;
	int Val=0;
	
	if((argc < 2)||(argc > 5))
	{
		vty_out(vty, "bad parameters number!\n");
		return CMD_FAILURE;
	}
	
	ret = dcli_str2ulong((char*)argv[0],&ruleIndex);
	if (ACL_RETURN_CODE_ERROR == ret) {
		vty_out(vty,"%% Illegal rule index!\n");
		return CMD_FAILURE;
	}
	ruleIndex = ruleIndex-1;
	
	ruleType = STANDARD_ACL_RULE;

	 if(strncmp("any",argv[1],strlen(argv[1]))==0){
		dipmaskLen=0;
	  	dipno=0;
       }
	else{
		Val=ip_address_format2ulong((char**)&argv[1],&dipno,&dipmaskLen);	
		if(Val==CMD_FAILURE)  return CMD_FAILURE;
		VALUE_IP_MASK_CHECK(dipmaskLen);
	}

	if(strncmp("any",argv[2],strlen(argv[2]))==0) {             
        dstport = ACL_ANY_PORT;   
	}
      else{
        ret= dcli_str2ulong((char*)argv[2],&dstport);
		if (ACL_RETURN_CODE_ERROR == ret) {
			vty_out(vty,"%% Illegal destination port!\n");
			return CMD_SUCCESS;
		}
	}
	if(strncmp("any",argv[3],strlen(argv[3]))==0){ 
			sipmaskLen=0;
		  	sipno=0;
	}
	else{					
		 Val=ip_address_format2ulong((char**)&argv[3],&sipno,&sipmaskLen);	
		 if(Val==CMD_FAILURE)  return CMD_FAILURE;
		 VALUE_IP_MASK_CHECK(sipmaskLen);	
	}
		
    if(strncmp("any",argv[4],strlen(argv[4]))==0) {           
        srcport = ACL_ANY_PORT; 
    }
    else{
        ret = dcli_str2ulong((char*)argv[4],&srcport);
		if (ACL_RETURN_CODE_ERROR == ret) {
			vty_out(vty,"%% Illegal source port!\n");
			return CMD_SUCCESS;
		}
	}
     
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_ACL_OBJPATH,NPD_DBUS_ACL_INTERFACE, NPD_DBUS_METHOD_CONFIG_ACL_RULE_TRAP_UDP);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&ruleIndex,
							 DBUS_TYPE_UINT32,&ruleType,
							 DBUS_TYPE_UINT32,&sipmaskLen,
							 DBUS_TYPE_UINT32,&dipmaskLen,
							 DBUS_TYPE_UINT32,&dipno,
							 DBUS_TYPE_UINT32,&sipno,
						     DBUS_TYPE_UINT32,&dstport,
					         DBUS_TYPE_UINT32,&srcport,
							 DBUS_TYPE_INVALID);
   
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_acl,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{
			if (ACL_RETURN_CODE_SUCCESS == op_ret ) 
			{
				/*vty_out(vty,"Access-list %d is successfully set!\n",(ruleIndex+1) );*/
			}
			else if(ACL_RETURN_CODE_GLOBAL_EXISTED == op_ret)
				vty_out(vty,"%% Access-list %d existed!\n",(ruleIndex+1));
			else if(ACL_RETURN_CODE_EXT_NO_SPACE==op_ret)
		          vty_out(vty,"%% because the extended should take up twice spaces than stdard acl,but %d has been set extended!\n",(ruleIndex-511));
	
			else if(ACL_RETURN_CODE_SAME_FIELD == op_ret)
				vty_out(vty,"%% identical fields of packet can not set again\n");
			else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret)
				vty_out(vty,"%% Product not support this function!\n");
			else 
				vty_out(vty,"%% set fail!\n");
		
	} 
	else 
	{
		/*vty_out(vty,"Failed get args.\n");*/
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
	
}
	
DEFUN(config_acl_rule_trap_tcp_cmd_func,
	  config_acl_rule_trap_tcp_cmd,
	  "acl standard <1-1000> trap tcp dip (A.B.C.D/M|any) dst-port (<0-65535>|any) sip (A.B.C.D/M|any) src-port (<0-65535>|any)",
	  ACL_STR
	  "Standard acl rule\n"
	  "Standard rule index range in 1-1000\n"
	  "Acl rule action Trap-to-Cpu\n"
	  "Acl rule deal with TCP packet\n"
	  "Destination IP address for TCP packet\n"
	  "Specify IP address in <A.B.C.D/M> format\n"
	  "Any legal destination IP address\n"
	  "Destination port with TCP packet\n"
	  "Specify Port number in range 0-65535\n"
	  "Any legal destination port number\n"
	  "Source IP address for TCP packet\n"
	  "Specify IP address in <A.B.C.D/M> format\n"
	  "Any legal source IP address\n"
	  "Source port with TCP packet\n"
	  "Specify Port number in range 0-65535\n"
	  "Any legal source port number\n"
)
{
	DBusMessage 	*query = NULL, *reply = NULL;
	DBusError		err;
	unsigned int	ruleIndex = 0,sipmaskLen = 0,dipmaskLen = 0,srcport = 0,dstport = 0;
	unsigned long	dipno = 0, sipno = 0;
	unsigned int 	ruleType = 0,ret = 0,i = 0;
	unsigned long	op_ret = 0;
	int Val=0;
	
	if((argc < 2)||(argc > 5))
	{
		vty_out(vty, "bad parameters number!\n");
		return CMD_FAILURE;
	}
	
	ret = dcli_str2ulong((char*)argv[0],&ruleIndex);
	if (ACL_RETURN_CODE_ERROR == ret) {
		vty_out(vty,"%% Illegal rule index!\n");
		return CMD_FAILURE;
	}
	ruleIndex = ruleIndex-1;

	ruleType = STANDARD_ACL_RULE;

	if(strncmp("any",argv[1],strlen(argv[1]))==0){
			dipmaskLen=0;
			dipno=0;
	}
	else{	
			Val=ip_address_format2ulong((char**)&argv[1],&dipno,&dipmaskLen);			
			if(Val==CMD_FAILURE) return CMD_FAILURE;
			VALUE_IP_MASK_CHECK(dipmaskLen);
	}
	
	
	if(strncmp("any",argv[2],strlen(argv[2]))==0) {	   
		dstport = ACL_ANY_PORT;
	}
	else{
		ret= dcli_str2ulong((char*)argv[2],&dstport);
		if(ret==ACL_RETURN_CODE_ERROR)
		{	
			vty_out(vty,"%% Illegal destination port!\n");
			return CMD_FAILURE; 	
		}

	}
	if(strncmp("any",argv[3],strlen(argv[3]))==0)
		{ 
			sipmaskLen=0;
			sipno=0;
		}
	else
		{						
		  Val=ip_address_format2ulong((char**)&argv[3],&sipno,&sipmaskLen); 	
		  if(Val==CMD_FAILURE)  return CMD_FAILURE;
		  VALUE_IP_MASK_CHECK(sipmaskLen);
	}
	
	if(strncmp("any",argv[4],strlen(argv[4]))==0) {	   
		srcport = ACL_ANY_PORT;
	}
	else{
		ret= dcli_str2ulong((char*)argv[4],&srcport);
		if(ret==ACL_RETURN_CODE_ERROR)
		{	
			vty_out(vty,"%% Illegal source port!\n");
			return CMD_FAILURE;		
		}
	}
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_ACL_OBJPATH,NPD_DBUS_ACL_INTERFACE, NPD_DBUS_METHOD_CONFIG_ACL_RULE_TRAP_TCP);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&ruleIndex,
							 DBUS_TYPE_UINT32,&ruleType,
							 DBUS_TYPE_UINT32,&sipmaskLen,
							 DBUS_TYPE_UINT32,&dipmaskLen,
							 DBUS_TYPE_UINT32,&dipno,
							 DBUS_TYPE_UINT32,&sipno,
							 DBUS_TYPE_UINT32,&dstport,
							 DBUS_TYPE_UINT32,&srcport,
							 DBUS_TYPE_INVALID);
  
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_acl,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{
			if (ACL_RETURN_CODE_SUCCESS == op_ret ) 
			{
				/*vty_out(vty,"Access-list %d is successfully set!\n",(ruleIndex+1) );*/
			}
			else if(ACL_RETURN_CODE_GLOBAL_EXISTED == op_ret)
				vty_out(vty,"%% Access-list %d existed!\n",(ruleIndex+1));
			else if((ACL_RETURN_CODE_EXT_NO_SPACE==op_ret)&&(ruleType == STANDARD_ACL_RULE))
		         	vty_out(vty,"%% because the extended should take up twice spaces than stdard acl,but %d has been set extended!\n",(ruleIndex-511));
			else if((ACL_RETURN_CODE_EXT_NO_SPACE==op_ret)&&(ruleType == EXTENDED_ACL_RULE))
				vty_out(vty,"%% because the extended rule should take up twice spaces than standard acl,\
				 			  but %d has been set standard rule! ~~set fail\n",(ruleIndex+513));
			else if(ACL_RETURN_CODE_SAME_FIELD == op_ret)
				vty_out(vty,"%% identical fields of packet can not set again\n");
			else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret)
				vty_out(vty,"%% Product not support this function!\n");
			else 
				vty_out(vty,"%% set fail!\n");		
	} 
	else 
	{
		/*vty_out(vty,"Failed get args.\n");*/
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
	
}
	
DEFUN(config_acl_rule_trap_icmp_cmd_func,
	  config_acl_rule_trap_icmp_cmd,
	  "acl (standard|extended) <1-1000> trap icmp dip (A.B.C.D/M |any) sip (A.B.C.D/M |any) type (<0-255>|any) code (<0-255>|any)",
	  ACL_STR
	  "Standard acl rule\n"
	  "Extended acl rule\n"
	  "Standard rule index range in 1-1000, extended rule index range in 1-500\n"
	  "Acl rule action Trap-to-Cpu\n" 
	  "Acl rule deal with ICMP packet\n"
	  "Destination IP address for ICMP packet\n"
	  "Specify IP address in <A.B.C.D/M> format\n"
	  "Any legal destination IP address\n"
	  "Source IP address for ICMP packet\n"
	  "Specify IP address in <A.B.C.D/M> format\n"
	  "Any legal source IP address\n"
	  "Icmp type with ICMP packet\n"
	  "Specify type number in range 0-255\n"
	  "Any legal type number\n"
	  "Icmp code with ICMP packet\n"
	  "Specifycode number in range 0-255\n"
	  "Any legal code number\n"
)
{
	DBusMessage 	*query = NULL, *reply = NULL;
	DBusError		err;
	unsigned int    ruleIndex = 0,sipmaskLen = 0,dipmaskLen = 0;
	unsigned long	 dipno = 0, sipno = 0;
	unsigned int 	ruleType = 0,ret = 0,i = 0;
	unsigned long	op_ret = 0;
	unsigned char   typeno = 0, codeno = 0, acl_any = 0;;
	unsigned int    tmp1=0,tmp2=0;
	int Val=0;

	if((argc < 2)||(argc >6))
	{
		vty_out(vty, "bad parameters number!\n");
		return CMD_WARNING;
	}
	if(strncmp("standard",argv[0],strlen(argv[0]))==0)
	{
		ruleType = STANDARD_ACL_RULE;
	}
	else if (strncmp("extended",argv[0],strlen(argv[0]))==0)
	{
		ruleType = EXTENDED_ACL_RULE;
	}
	else
	{
		vty_out(vty,"bad command parameter!\n");
		return CMD_WARNING;
	}	
	ret = dcli_str2ulong((char*)argv[1],&ruleIndex);
	if (ACL_RETURN_CODE_ERROR == ret) {
		vty_out(vty,"%% Illegal rule index!\n");
		return CMD_WARNING;
	}
	if ((ruleType == EXTENDED_ACL_RULE)&&(ruleIndex > MAX_EXT_RULE_NUM)) {
		vty_out(vty,"extended rule must less than 500!\n");
		return CMD_WARNING;
	}
	ruleIndex = ruleIndex-1;
	
	if(strncmp("any",argv[2],strlen(argv[2]))==0)
		{
			dipmaskLen=0;
			dipno=0;
		}
	else{	
			Val=ip_address_format2ulong((char**)&argv[2],&dipno,&dipmaskLen);			
			if(CMD_WARNING == Val) {
				 vty_out(vty, "%% Bad parameter %s\n", argv[2]);
				 return CMD_WARNING;
			}
			VALUE_IP_MASK_CHECK(dipmaskLen);
	}
	if(strncmp("any",argv[3],strlen(argv[3]))==0)
		{ 
			sipmaskLen=0;
			sipno=0;
		}
	else
		{						
		  Val=ip_address_format2ulong((char**)&argv[3],&sipno,&sipmaskLen); 	
		  if(CMD_WARNING == Val) {
			   vty_out(vty, "%% Bad parameter %s\n", argv[3]);
			   return CMD_WARNING;
		  }
		  VALUE_IP_MASK_CHECK(sipmaskLen); 
	}
	
	if(strncmp("any",argv[4],strlen(argv[4]))==0) {	   
		typeno=0;
		acl_any |= 1;
	}
	else{
		ret= dcli_str2ulong((char*)argv[4],&tmp1);
		if (ACL_RETURN_CODE_ERROR == ret) {
			vty_out(vty,"%% Illegal icmp type!\n");
			return CMD_WARNING;
		}
		typeno = tmp1;
	}
	if(strncmp("any",argv[5],strlen(argv[5]))==0) {	   
		codeno=0;
		acl_any |= 2;
	}	
	else{
		ret= dcli_str2ulong((char*)argv[5],&tmp2);
		if (ACL_RETURN_CODE_ERROR == ret) {
			vty_out(vty,"%% Illegal icmp code!\n");
			return CMD_WARNING;
		}
		codeno = tmp2;
	}
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_ACL_OBJPATH,NPD_DBUS_ACL_INTERFACE, NPD_DBUS_METHOD_CONFIG_ACL_RULE_TRAP_ICMP);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&ruleIndex,	
							 DBUS_TYPE_UINT32,&ruleType,
							 DBUS_TYPE_UINT32,&sipmaskLen,
							 DBUS_TYPE_UINT32,&dipmaskLen,
							 DBUS_TYPE_UINT32,&dipno,
							 DBUS_TYPE_UINT32,&sipno,
							 DBUS_TYPE_BYTE,&typeno,
							 DBUS_TYPE_BYTE,&codeno,
							 DBUS_TYPE_BYTE,&acl_any,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_acl,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{
			if (ACL_RETURN_CODE_SUCCESS == op_ret ) 
			{
				/*vty_out(vty,"Access-list %d is successfully set!\n",(ruleIndex+1) );*/
			}	
			else if(ACL_RETURN_CODE_GLOBAL_EXISTED == op_ret)
				vty_out(vty,"%% Access-list %d existed!\n",(ruleIndex+1));
			else if((ACL_RETURN_CODE_EXT_NO_SPACE==op_ret)&&(ruleType == STANDARD_ACL_RULE))
		         	vty_out(vty,"%% because the extended should take up twice spaces than stdard acl,but %d has been set extended!\n",(ruleIndex-511));
			else if((ACL_RETURN_CODE_EXT_NO_SPACE==op_ret)&&(ruleType == EXTENDED_ACL_RULE))
				vty_out(vty,"%% because the extended rule should take up twice spaces than standard acl,\
				 			  but %d has been set standard rule! ~~set fail\n",(ruleIndex+513));	
			else if(ACL_RETURN_CODE_SAME_FIELD == op_ret)
				vty_out(vty,"%% identical fields of packet can not set again\n");
			else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret)
				vty_out(vty,"%% Product not support this function!\n");
			else 
				vty_out(vty,"%% set fail!\n");
	} 
	else 
	{
		/*vty_out(vty,"Failed get args.\n");*/
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;	
}
DEFUN(config_acl_rule_trap_to_cpu_ethernet_func,
	  config_acl_rule_trap_to_cpu_ethernet_cmd,
	  "acl extended <1-1000> trap ethernet dmac (H:H:H:H:H:H|any) smac (H:H:H:H:H:H|any)",
	  ACL_STR
	  "Extended acl rule\n"
	  "Extended rule index range in 1-500\n"
	  "Acl rule action Trap-to-Cpu\n" 
	  "Match with ethernet packets\n"
	  "Care destination mac address\n"
	  "Specify DMAC address in IPv6 fomat\n"
	  "Any legal MAC address\n"
	  "care source mac address \n"
	  "Specify DMAC address in IPv6 fomat\n"
	  "Any legal MAC address\n"
)
{
	DBusMessage 	*query = NULL, *reply = NULL;
	DBusError		err;
	ETHERADDR		dmacAddr,smacAddr;
	unsigned int	ruleIndex = 0,packetType = 0; 
	unsigned int	ruleType = 0,op_ret = 0;
	unsigned int	ethertype = 0xffff;
	char *			str[20]={"ff:ff:ff:ff:ff:ff"};
	char *			strcp = NULL;
	int Val=0, ret = 0;
	
	if((argc < 2)||(argc > 3))
	{
		vty_out(vty, "bad parameters number!\n");
		return CMD_WARNING;
	}	
	ret = dcli_str2ulong((char*)argv[0],&ruleIndex);
	if (ACL_RETURN_CODE_ERROR == ret) {
		vty_out(vty,"%% Illegal rule index!\n");
		return CMD_WARNING;
	}
	ruleType = EXTENDED_ACL_RULE; 
	if ((ruleType == EXTENDED_ACL_RULE)&&(ruleIndex > MAX_EXT_RULE_NUM)) {
		vty_out(vty,"extended rule must less than 500!\n");
		return CMD_WARNING;
	}
	ruleIndex = ruleIndex-1; 
	memset(&smacAddr,0,sizeof(ETHERADDR));
	memset(&dmacAddr,0,sizeof(ETHERADDR));
	/*memset(&maskMac,0,sizeof(ETHERADDR));*/
	/*fetch the 1st param : DMAC addr*/

	if(strncmp("any",argv[1],strlen(argv[1]))!=0){
		op_ret = parse_mac_addr((char *)argv[1],&dmacAddr);
		if (ACL_RETURN_CODE_SUCCESS != op_ret) {
	    	vty_out(vty," %% Unknow mac addr format.\n");
			return CMD_SUCCESS;
		}
	}
	/*fetch the 1st param : SMAC addr*/
	if(strncmp("any",argv[2],strlen(argv[2]))!=0){
		op_ret = parse_mac_addr((char *)argv[2],&smacAddr);
		if (ACL_RETURN_CODE_SUCCESS != op_ret) {
	    	vty_out(vty,"%% Unknow mac addr format.\n");
			return CMD_SUCCESS;
		}
	}
		
	/*query*/
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_ACL_OBJPATH,NPD_DBUS_ACL_INTERFACE, NPD_DBUS_METHOD_CONFIG_ACL_RULE_TRAP_MAC);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&ruleIndex,	
							 DBUS_TYPE_UINT32,&ruleType,					 
						     DBUS_TYPE_UINT32,&ethertype,
							 DBUS_TYPE_BYTE,  &dmacAddr.arEther[0],
							 DBUS_TYPE_BYTE,  &dmacAddr.arEther[1],
							 DBUS_TYPE_BYTE,  &dmacAddr.arEther[2],
							 DBUS_TYPE_BYTE,  &dmacAddr.arEther[3],
							 DBUS_TYPE_BYTE,  &dmacAddr.arEther[4],
							 DBUS_TYPE_BYTE,  &dmacAddr.arEther[5],
						        DBUS_TYPE_BYTE,  &smacAddr.arEther[0],
							 DBUS_TYPE_BYTE,  &smacAddr.arEther[1],
							 DBUS_TYPE_BYTE,  &smacAddr.arEther[2],
							 DBUS_TYPE_BYTE,  &smacAddr.arEther[3],
							 DBUS_TYPE_BYTE,  &smacAddr.arEther[4],
							 DBUS_TYPE_BYTE,  &smacAddr.arEther[5],						
							 DBUS_TYPE_INVALID);
      
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_acl,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{
			if (ACL_RETURN_CODE_SUCCESS == op_ret ) 
			{
				/*vty_out(vty,"Access-list %d is successfully set!\n",(ruleIndex+1));*/
			}
			else if(ACL_RETURN_CODE_GLOBAL_EXISTED == op_ret)
				vty_out(vty,"%% Access-list %d existed!\n",(ruleIndex+1));
			else if((ACL_RETURN_CODE_EXT_NO_SPACE==op_ret)&&(ruleType == STANDARD_ACL_RULE))
		         	vty_out(vty,"%% because the extended should take up twice spaces than stdard acl,but %d has been set extended!\n",(ruleIndex-511));
			else if((ACL_RETURN_CODE_EXT_NO_SPACE==op_ret)&&(ruleType == EXTENDED_ACL_RULE))
				vty_out(vty,"%% because the extended rule should take up twice spaces than standard acl,\
				 			  but %d has been set standard rule! ~~set fail\n",(ruleIndex+513));
			else if(ACL_RETURN_CODE_SAME_FIELD == op_ret)
				vty_out(vty,"%% identical fields of packet can not set again\n");
			else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret)
				vty_out(vty,"%% Product not support this function!\n");
			else 
				vty_out(vty,"%% set fail!\n");
		
	} 
	else 
	{
		/*vty_out(vty,"Failed get args.\n");*/
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(config_acl_rule_trap_arp_func,
	  config_acl_rule_trap_arp_cmd,
	  "acl extended <1-1000> trap arp smac (H:H:H:H:H:H|any) vid (<1-4095>|any) sourceport (PORTNO|any)",
	  ACL_STR
	  "extended acl rule\n"
	  "extended rule index range in 1-500\n"
	  "acl rule action Trap-to-Cpu\n" 
	  "match with arp packets\n"
	  "care source mac address\n"
	  "Specify SMAC address in Hexa.Eg:00:00:11:11:aa:aa\n"
	  "any source mac\n"
	  "care vlan id \n"
	  "specify vlan id in 1~4095\n"
	  "any vlan id\n"
	  "care Interface port which packets come from \n"
	  CONFIG_ETHPORT_STR 
	  "any ports that packets come from\n"
)	
{
	DBusMessage 	*query = NULL, *reply = NULL;
  	DBusError		err;
	ETHERADDR		smacAddr,maskMac;
	unsigned char   slot_no=ACL_ANY_PORT_CHAR,port_no=ACL_ANY_PORT_CHAR;
	unsigned int	ruleIndex = 0; 
	unsigned int	ruleType = 0,op_ret = 0,vlanId=0,ifIndex = 0,ret = 0,eth_ret = 0,port_ret = 0;
	char *			str[20]={"ff:ff:ff:ff:ff:ff"};
	char *			strcp = NULL;
	
	if((argc < 2)||(argc > 4))
	{
		vty_out(vty, "bad parameters number!\n");
		return CMD_WARNING;
	}
	
	ret = dcli_str2ulong((char*)argv[0],&ruleIndex);
	if (ACL_RETURN_CODE_ERROR == ret) {
		vty_out(vty,"%% Illegal rule index!\n");
		return CMD_WARNING;
	}
	ruleType = EXTENDED_ACL_RULE; 
	if ((ruleType == EXTENDED_ACL_RULE)&&(ruleIndex > MAX_EXT_RULE_NUM)) {
		vty_out(vty,"extended rule must less than 500!\n");
		return CMD_WARNING;
	}
	ruleIndex = ruleIndex-1; 	
	
	memset(&smacAddr,0,sizeof(ETHERADDR));
	memset(&maskMac,0,sizeof(ETHERADDR));

	/*fetch the 1st param : SMAC addr*/
	if(strncmp("any",argv[1],strlen(argv[1]))!=0){
		op_ret = parse_mac_addr((char *)argv[1],&smacAddr);
		if (ACL_RETURN_CODE_SUCCESS != op_ret) {
	    	vty_out(vty,"%% Unknow mac addr format.\n");
			return CMD_SUCCESS;
		}
	
		/*convert mac_mask to standard format*/
		strcp=(char *)malloc(50*sizeof(char));
		if (NULL == strcp) {
			return CMD_WARNING;
		}
		memset(strcp,'\0',50);
		strcpy(strcp,str[0]);
		
		op_ret = parse_mac_addr(strcp,&maskMac);
		if (ACL_RETURN_CODE_SUCCESS != op_ret) {
	    	vty_out(vty,"%% Unknow mac addr format.\n");
			return CMD_SUCCESS;
		}
	}
    /*vlan id*/
	if(strncmp("any",argv[2],strlen(argv[2]))!=0){
		ret = dcli_str2ulong((char*)argv[2],&vlanId);		
		if(ret==ACL_RETURN_CODE_ERROR)
		{
			vty_out(vty,"%% Illegal vlan Id!\n");
			free(strcp);
			return CMD_WARNING;
		}
	}
	/*slot/port*/
	if(strncmp("any",argv[3],strlen(argv[3]))!=0){
		ret = parse_slotport_no((char *)argv[3],&slot_no,&port_no);

		if (ACL_RETURN_CODE_SUCCESS != ret) {
	    	vty_out(vty,"%% Unknown portno format.\n");
			free(strcp);
			return CMD_SUCCESS;
		}
	}
	
	/*query*/
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_ACL_OBJPATH,NPD_DBUS_ACL_INTERFACE, NPD_DBUS_METHOD_CONFIG_ACL_RULE_TRAP_ARP);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&ruleIndex,	
							 DBUS_TYPE_UINT32,&ruleType,						
						        DBUS_TYPE_BYTE,  &smacAddr.arEther[0],
							 DBUS_TYPE_BYTE,  &smacAddr.arEther[1],
							 DBUS_TYPE_BYTE,  &smacAddr.arEther[2],
							 DBUS_TYPE_BYTE,  &smacAddr.arEther[3],
							 DBUS_TYPE_BYTE,  &smacAddr.arEther[4],
							 DBUS_TYPE_BYTE,  &smacAddr.arEther[5],
						        DBUS_TYPE_BYTE,  &maskMac.arEther[0],
							 DBUS_TYPE_BYTE,  &maskMac.arEther[1],
							 DBUS_TYPE_BYTE,  &maskMac.arEther[2],
							 DBUS_TYPE_BYTE,  &maskMac.arEther[3],
							 DBUS_TYPE_BYTE,  &maskMac.arEther[4],
							 DBUS_TYPE_BYTE,  &maskMac.arEther[5],	
							 DBUS_TYPE_UINT32,&vlanId,
							 DBUS_TYPE_BYTE,  &slot_no,
							 DBUS_TYPE_BYTE,  &port_no,
							 DBUS_TYPE_INVALID);
   	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_acl,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			vty_out(vty,"%%s raised: %s",err.name,err.message);
			free(strcp);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_UINT32,&eth_ret,		
		DBUS_TYPE_INVALID))
	{

			if(ACL_RETURN_CODE_SUCCESS != eth_ret){
				vty_out(vty,"%% Error! illegal port index!\n");
			}
			else{
				if (ACL_RETURN_CODE_SUCCESS == op_ret ) 
				{
					/*vty_out(vty,"Access-list %d is successfully set!\n",(ruleIndex+1));*/
				}
				else if(ACL_RETURN_CODE_GLOBAL_EXISTED == op_ret)
					vty_out(vty,"%% Access-list %d existed!\n",(ruleIndex+1));
				else if((ACL_RETURN_CODE_EXT_NO_SPACE==op_ret)&&(ruleType == STANDARD_ACL_RULE))
		         		vty_out(vty,"%% because the extended should take up twice spaces than stdard acl,but %d has been set extended!\n",(ruleIndex-511));
				else if((ACL_RETURN_CODE_EXT_NO_SPACE==op_ret)&&(ruleType == EXTENDED_ACL_RULE))
					vty_out(vty,"%% because the extended rule should take up twice spaces than standard acl,\
				 			  but %d has been set standard rule! ~~set fail\n",(ruleIndex+513));	
				else if(ACL_RETURN_CODE_SAME_FIELD == op_ret)
					vty_out(vty,"%% identical fields of packet can not set again\n");	
				else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret)
					vty_out(vty,"%% Product not support this function!\n");
				else 
					vty_out(vty,"%% set fail!\n");
			}			
	} 
	else 
	{
		/*vty_out(vty,"Failed get args.\n");*/
		if (dbus_error_is_set(&err)) 
		{
			free(strcp);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}	

DEFUN(config_acl_rule_permit_deny_arp_func,
	  config_acl_rule_permit_deny_arp_cmd,
	  "acl extended <1-1000> (permit|deny) arp smac (H:H:H:H:H:H|any) vid (<1-4095>|any) sourceport (PORTNO|any) [policer] [<1-255>]",
	  ACL_STR
	  "extended acl rule\n"
	  "extended rule index range in 1-500\n"
	  "permit packets matched\n" 
	  "deny packets matched\n"
	  "match with arp packets\n"
	  "care source mac address\n"
	  "Specify SMAC address in Hexa.Eg:00:00:11:11:aa:aa\n"
	  "any source mac\n"
	  "care vlan id \n"
	  "specify vlan id in 1~4095\n"
	  "any vlan id\n"
	  "care Interface port which packets come from \n"
	  CONFIG_ETHPORT_STR 
	  "any ports for packets coming from\n"
	  "Allow policing on ingress port\n"
	  "Policer Id range in 1-255\n"
)	
{
	DBusMessage 	*query = NULL, *reply = NULL;
  	DBusError		err;
	ETHERADDR		smacAddr,maskMac;
	unsigned char   slot_no=ACL_ANY_PORT_CHAR,port_no=ACL_ANY_PORT_CHAR;
	unsigned int	ruleIndex = 0; 
	unsigned int	ruleType = 0,actionType = 0,op_ret = 0,vlanId=0,ifIndex = 0,ret = 0,eth_ret = 0,port_ret = 0;
	char *			str[20]={"ff:ff:ff:ff:ff:ff"};
	char *			strcp = NULL;
	unsigned int    policer=0,policerId=0;
	
	ret = dcli_str2ulong((char*)argv[0],&ruleIndex);
	if (ACL_RETURN_CODE_ERROR == ret) {
		vty_out(vty,"%% Illegal rule index!\n");
		return CMD_WARNING;
	}
	ruleType = EXTENDED_ACL_RULE; 
	if ((ruleType == EXTENDED_ACL_RULE)&&(ruleIndex > MAX_EXT_RULE_NUM)) {
		vty_out(vty,"extended rule must less than 500!\n");
		return CMD_WARNING;
	}
	ruleIndex = ruleIndex-1; 	

	if(strncmp("permit",argv[1],strlen(argv[1]))==0) { actionType=0;}
    else if(strncmp("deny",argv[1],strlen(argv[1]))==0) {actionType=1;}      
   
    		
	memset(&smacAddr,0,sizeof(ETHERADDR));
	memset(&maskMac,0,sizeof(ETHERADDR));
	
	/*fetch the 1st param : SMAC addr*/
	if(strncmp("any",argv[2],strlen(argv[2]))!=0){
		op_ret = parse_mac_addr((char *)argv[2],&smacAddr);
		if (ACL_RETURN_CODE_SUCCESS != op_ret) {
	    		vty_out(vty,"%% Unknow mac addr format.\n");
			return CMD_WARNING;
		}
	
		/*convert mac_mask to standard format*/
		strcp=(char *)malloc(50*sizeof(char));
		memset(strcp,'\0',50);
	       strcpy(strcp,str[0]);
		
		op_ret = parse_mac_addr(strcp,&maskMac);
		if (ACL_RETURN_CODE_SUCCESS != op_ret) {
	    		vty_out(vty,"%% Unknow mac addr format.\n");
			return CMD_WARNING;
		}
	}
    /*vlan id*/
	if(strncmp("any",argv[3],strlen(argv[3]))!=0){
		 ret= dcli_str2ulong((char*)argv[3],&vlanId);		
 		if(ret==ACL_RETURN_CODE_ERROR)
 		{
 			vty_out(vty,"%% Illegal vlan id!\n");
 			return CMD_WARNING;
 		}
	}
	/*slot/port*/
	if(strncmp("any",argv[4],strlen(argv[4]))!=0){
		ret = parse_slotport_no((char *)argv[4],&slot_no,&port_no);

		if (ACL_RETURN_CODE_SUCCESS != ret) {
	    		vty_out(vty,"%% Unknown portno format.\n");
			return CMD_WARNING;
		}
	}
	/*policer*/
	if((argc>5)&&(argc<8)){
		if(actionType==0){	
			if(strncmp("policer",argv[5],strlen(argv[5]))==0){
				policer = 1;
				ret = dcli_str2ulong((char *)argv[6],&policerId);
				if(ret==ACL_RETURN_CODE_ERROR)
				{
					vty_out(vty,"%% Illegal policer ID!\n");
					return CMD_WARNING;
				}
			}
			else{
				vty_out(vty,"%% unknown command\n");
				return CMD_WARNING;
			}			
		}
		else if(actionType==1){
			 vty_out(vty,"%% Policer not support deny operation!\n");
			 return CMD_WARNING;
		}
	}
	else if(argc>7){
		vty_out(vty,"%% too many param\n");
		return CMD_WARNING;
	}
	/*query*/
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_ACL_OBJPATH,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_METHOD_CONFIG_ACL_RULE_PERMIT_DENY_ARP);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&ruleIndex,	
							 DBUS_TYPE_UINT32,&ruleType,
							 DBUS_TYPE_UINT32,&actionType,
						     DBUS_TYPE_BYTE,  &smacAddr.arEther[0],
							 DBUS_TYPE_BYTE,  &smacAddr.arEther[1],
							 DBUS_TYPE_BYTE,  &smacAddr.arEther[2],
							 DBUS_TYPE_BYTE,  &smacAddr.arEther[3],
							 DBUS_TYPE_BYTE,  &smacAddr.arEther[4],
							 DBUS_TYPE_BYTE,  &smacAddr.arEther[5],
						     DBUS_TYPE_BYTE,  &maskMac.arEther[0],
							 DBUS_TYPE_BYTE,  &maskMac.arEther[1],
							 DBUS_TYPE_BYTE,  &maskMac.arEther[2],
							 DBUS_TYPE_BYTE,  &maskMac.arEther[3],
							 DBUS_TYPE_BYTE,  &maskMac.arEther[4],
							 DBUS_TYPE_BYTE,  &maskMac.arEther[5],	
							 DBUS_TYPE_UINT32,&vlanId,
							 DBUS_TYPE_BYTE,  &slot_no,
							 DBUS_TYPE_BYTE,  &port_no,
							 DBUS_TYPE_UINT32,&policer,
							 DBUS_TYPE_UINT32, &policerId,
							 DBUS_TYPE_INVALID);
  
   
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_acl,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_UINT32,&eth_ret,		
		DBUS_TYPE_INVALID))
	{

			if(ACL_RETURN_CODE_SUCCESS != eth_ret){
				vty_out(vty,"%% Error!illegal port index!\n");
			}
			else{
				if (ACL_RETURN_CODE_SUCCESS == op_ret ) 
				{
					/*vty_out(vty,"Access-list %d is successfully set!\n",(ruleIndex+1));*/
				}
				else if(ACL_RETURN_CODE_GLOBAL_EXISTED == op_ret)
					vty_out(vty,"%% Access-list %d existed!\n",(ruleIndex+1));
				
				else if((ACL_RETURN_CODE_EXT_NO_SPACE==op_ret)&&(ruleType == STANDARD_ACL_RULE))
		         		vty_out(vty,"%% because the extended should take up twice spaces than stdard acl,but %d has been set extended!\n",(ruleIndex-511));
				else if((ACL_RETURN_CODE_EXT_NO_SPACE==op_ret)&&(ruleType == EXTENDED_ACL_RULE))
					vty_out(vty,"%% because the extended rule should take up twice spaces than standard acl,\
				 			  but %d has been set standard rule! ~~set fail\n",(ruleIndex+513));
				else if(ACL_RETURN_CODE_SAME_FIELD == op_ret)
					vty_out(vty,"%% identical fields of packet can not set again\n");	
				else if(ACL_RETURN_CODE_POLICER_ID_NOT_SET == op_ret)
					vty_out(vty,"%% Policer %d not existed!\n",policerId);
				else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret)
					vty_out(vty,"%% Product not support this function!\n");
				else 
					vty_out(vty,"%% set fail!\n");
			}			
	} 
	else 
	{
		/*vty_out(vty,"Failed get args.\n");*/
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}	

DEFUN(config_acl_rule_deny_mac_func,
	  config_acl_rule_deny_mac_cmd,
	  "acl extended <1-1000> (permit|deny) ethernet dmac (H:H:H:H:H:H|any) smac (H:H:H:H:H:H|any) [policer] [<1-255>]",
	  ACL_STR
	  "extended acl rule\n"
	  "extended rule index range in 1-500\n"
	  "permit packets matched\n" 
	  "deny packets matched\n"
	  "match with ethernet packets\n"
	  "care destination mac address\n"
	  "Specify DMAC address in Hexa.Eg:00:00:11:11:aa:aa\n"
	  "any destination mac\n"
	  "care source mac address \n"
	  "Specify SMAC address in Hexa.Eg:00:00:11:11:aa:aa\n"
	  "any source mac\n"
	  "Allow policing on ingress port\n"
	  "Policer Id range in 1-255\n"
)
{
	DBusMessage 	*query = NULL, *reply = NULL;
	DBusError		err;
	ETHERADDR		dmacAddr,smacAddr;
	unsigned int	ruleIndex = 0,packetType = 0,actionType = 0; 
	unsigned int	ruleType = 0,op_ret = 0,ethertype = 0xffff;
	char *			str[20]={"ff:ff:ff:ff:ff:ff"};
	char *			strcp = NULL;
	unsigned int 	policer=0,policerId=0;
	int ret = 0;
	
	#if 0
	if((argc < 2)||(argc > 4))
	{
		vty_out(vty, "bad parameters number!\n");
		return CMD_WARNING;
	}
	#endif
	
	ret = dcli_str2ulong((char*)argv[0],&ruleIndex);
	if (ACL_RETURN_CODE_ERROR == ret) {
		vty_out(vty,"%% Illegal rule index!\n");
		return CMD_WARNING;
	}
	ruleType = EXTENDED_ACL_RULE; 
	if ((ruleType == EXTENDED_ACL_RULE)&&(ruleIndex > MAX_EXT_RULE_NUM)) {
		vty_out(vty,"extended rule must less than 500!\n");
		return CMD_WARNING;
	}
	ruleIndex = ruleIndex-1; 

	if(strncmp("permit",argv[1],strlen(argv[1]))==0)
        actionType=0;
    else if(strncmp("deny",argv[1],strlen(argv[1]))==0)
        actionType=1;
   
	memset(&smacAddr,0,sizeof(ETHERADDR));
	memset(&dmacAddr,0,sizeof(ETHERADDR));
	/*memset(&maskMac,0,sizeof(ETHERADDR));*/
	/*fetch the 1st param : DMAC addr*/
	if(strncmp("any",argv[2],strlen(argv[2]))!=0){
		op_ret = parse_mac_addr((char *)argv[2],&dmacAddr);
		if (ACL_RETURN_CODE_SUCCESS != op_ret) {
			vty_out(vty,"%% Unknow mac addr format.\n");
			return CMD_SUCCESS;
		}
	}
	/*fetch the 1st param : SMAC addr*/
	if(strncmp("any",argv[3],strlen(argv[3]))!=0){
		op_ret = parse_mac_addr((char *)argv[3],&smacAddr);
		if (ACL_RETURN_CODE_SUCCESS != op_ret) {
			vty_out(vty,"%% Unknow mac addr format.\n");
			return CMD_SUCCESS;
		}
	}
	 /*policer*/
	if((argc>4)&&(argc<7)){
		if(actionType==0){	
			if(strncmp("policer",argv[4],strlen(argv[4]))==0){
				policer = 1;
				ret = dcli_str2ulong((char *)argv[5],&policerId);
				if(ret==ACL_RETURN_CODE_ERROR)
				{
					vty_out(vty,"%% Illegal policer ID!\n");
					return CMD_WARNING;
				}
			}
			else{
				vty_out(vty,"%% unknown command\n");
				return CMD_WARNING;
			}			
		}
		else if(actionType==1){
			vty_out(vty,"%% Policer not support deny operation!\n");
			 return CMD_WARNING;
		}
	}
	else if(argc>6){
		vty_out(vty,"%% too many param\n");
		return CMD_WARNING;
	}
	/*query*/
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_ACL_OBJPATH,NPD_DBUS_ACL_INTERFACE, NPD_DBUS_METHOD_CONFIG_ACL_RULE_DENY_MAC);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&ruleIndex,	
							 DBUS_TYPE_UINT32,&ruleType,
							 DBUS_TYPE_UINT32,&actionType,
							 DBUS_TYPE_UINT32,&ethertype,
							 DBUS_TYPE_BYTE,  &dmacAddr.arEther[0],
							 DBUS_TYPE_BYTE,  &dmacAddr.arEther[1],
							 DBUS_TYPE_BYTE,  &dmacAddr.arEther[2],
							 DBUS_TYPE_BYTE,  &dmacAddr.arEther[3],
							 DBUS_TYPE_BYTE,  &dmacAddr.arEther[4],
							 DBUS_TYPE_BYTE,  &dmacAddr.arEther[5],
							 DBUS_TYPE_BYTE,  &smacAddr.arEther[0],
							 DBUS_TYPE_BYTE,  &smacAddr.arEther[1],
							 DBUS_TYPE_BYTE,  &smacAddr.arEther[2],
							 DBUS_TYPE_BYTE,  &smacAddr.arEther[3],
							 DBUS_TYPE_BYTE,  &smacAddr.arEther[4],
							 DBUS_TYPE_BYTE,  &smacAddr.arEther[5],	
							 DBUS_TYPE_UINT32,&policer,
							 DBUS_TYPE_UINT32,&policerId,
							 DBUS_TYPE_INVALID);
 
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_acl,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{
			if (ACL_RETURN_CODE_SUCCESS == op_ret ) 
			{
				/*vty_out(vty,"Access-list %d is successfully set!\n",(ruleIndex+1));*/
			}
			else if(ACL_RETURN_CODE_GLOBAL_EXISTED == op_ret)
				vty_out(vty,"%% Access-list %d existed!\n",(ruleIndex+1));
			
			else if((ACL_RETURN_CODE_EXT_NO_SPACE==op_ret)&&(ruleType == STANDARD_ACL_RULE))
		         	vty_out(vty,"%% because the extended should take up twice spaces than stdard acl,but %d has been set extended!\n",(ruleIndex-511));
			else if((ACL_RETURN_CODE_EXT_NO_SPACE==op_ret)&&(ruleType == EXTENDED_ACL_RULE))
				vty_out(vty,"%% because the extended rule should take up twice spaces than standard acl,\
				 			  but %d has been set standard rule! ~~set fail\n",(ruleIndex+513));
			else if(ACL_RETURN_CODE_SAME_FIELD == op_ret)
				vty_out(vty,"%% identical fields of packet can not set again\n");	
			else if(ACL_RETURN_CODE_POLICER_ID_NOT_SET == op_ret)
					vty_out(vty,"%% Policer %d not existed!\n",policerId);
			else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret)
				vty_out(vty,"%% Product not support this function!\n");
			else 
				vty_out(vty,"%% set fail!\n");
		
	} 
	else 
	{
		/*vty_out(vty,"Failed get args.\n");*/
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(config_acl_rule_deny_ethertype_func,
	  config_acl_rule_deny_ethertype_cmd,
	  "acl extended <1-1000> (permit|deny) ethertype ETHERTYPE dmac (H:H:H:H:H:H|any) smac (H:H:H:H:H:H|any) [policer] [<1-255>]",
	  ACL_STR
	  "extended acl rule\n"
	  "extended rule index range in 1-500\n"
	  "permit packets matched\n" 
	  "deny packets matched\n"
	  "ethertype range\n"
	  "range in 0-65534 or any\n"
	  "care destination mac address\n"
	  "Specify DMAC address in Hexa.Eg:00:00:11:11:aa:aa\n"
	  "any destination mac\n"
	  "care source mac address \n"
	  "Specify SMAC address in Hexa.Eg:00:00:11:11:aa:aa\n"
	  "any source mac\n"
	  "Allow policing on ingress port\n"
	  "Policer Id range in 1-255\n"
)
{
	DBusMessage 	*query = NULL, *reply = NULL;
	DBusError		err;
	ETHERADDR		dmacAddr,smacAddr;
	unsigned int	ruleIndex = 0,packetType = 0,actionType = 0;
	unsigned int	ethertype = 0xffff;
	unsigned int	ruleType = 0,op_ret = 0;
	char *			str[20]={"ff:ff:ff:ff:ff:ff"};
	char *			strcp = NULL;
	unsigned int 	policer=0,policerId=0;
	int ret = 0;
	
	#if 0
	if((argc < 2)||(argc > 4))
	{
		vty_out(vty, "bad parameters number!\n");
		return CMD_WARNING;
	}
	#endif
	
	ret = dcli_str2ulong((char*)argv[0],&ruleIndex);
	if (ACL_RETURN_CODE_ERROR == ret) {
		vty_out(vty,"%% Illegal rule index!\n");
		return CMD_WARNING;
	}
	ruleType = EXTENDED_ACL_RULE; 
	if ((ruleType == EXTENDED_ACL_RULE)&&(ruleIndex > MAX_EXT_RULE_NUM)) {
		vty_out(vty,"extended rule must less than 500!\n");
		return CMD_WARNING;
	}
	ruleIndex = ruleIndex-1; 

	if(strncmp("permit",argv[1],strlen(argv[1]))==0)
        actionType=0;
    else if(strncmp("deny",argv[1],strlen(argv[1]))==0)
        actionType=1;
   
	memset(&smacAddr,0,sizeof(ETHERADDR));
	memset(&dmacAddr,0,sizeof(ETHERADDR));
	/*memset(&maskMac,0,sizeof(ETHERADDR));*/
	/*fetch the 1st param : DMAC addr*/

	if(strncmp("any",argv[2],strlen(argv[2]))==0)	{   
		ethertype = 0xffff;
	}
	else{
		/*ret= dcli_str2ulong((char*)argv[2],&ethertype);*/
		/*change for hex max ethertype is 0xfffe*/
		if (strlen((char*)argv[2]) > 6) {		
			vty_out(vty,"%% Illegal ethertype number!\n");
			return CMD_WARNING;
		}
		ethertype = strtoul((char*)argv[2], NULL, 0);
		if(ret==ACL_RETURN_CODE_ERROR || ethertype > 0xfffe) {
			vty_out(vty,"%% Illegal ethertype number!\n");
			return CMD_WARNING;
		}
	}
	
	if(strncmp("any",argv[3],strlen(argv[3]))!=0){
		op_ret = parse_mac_addr((char *)argv[3],&dmacAddr);
		if (ACL_RETURN_CODE_SUCCESS != op_ret) {
			vty_out(vty,"%% Unknow mac addr format.\n");
			return CMD_SUCCESS;
		}
	}
	/*fetch the 1st param : SMAC addr*/
	if(strncmp("any",argv[4],strlen(argv[4]))!=0){
		op_ret = parse_mac_addr((char *)argv[4],&smacAddr);
		if (ACL_RETURN_CODE_SUCCESS != op_ret) {
			vty_out(vty,"%% Unknow mac addr format.\n");
			return CMD_SUCCESS;
		}
	}
	 /*policer*/
	if((argc>5)&&(argc<8)){
		if(actionType==0){	
			if(strncmp("policer",argv[5],strlen(argv[5]))==0){
				policer = 1;
				ret = dcli_str2ulong((char *)argv[6],&policerId);
				if(ret==ACL_RETURN_CODE_ERROR)
				{
					vty_out(vty,"%% Illegal policer ID!\n");
					return CMD_WARNING;
				}
			}
			else{
				vty_out(vty,"%% unknown command\n");
				return CMD_WARNING;
			}			
		}
		else if(actionType==1){
			vty_out(vty,"%% Policer not support deny operation!\n");
			 return CMD_WARNING;
		}
	}
	else if(argc>7){
		vty_out(vty,"%% too many param\n");
		return CMD_WARNING;
	}
	/*query*/
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_ACL_OBJPATH,NPD_DBUS_ACL_INTERFACE, NPD_DBUS_METHOD_CONFIG_ACL_RULE_DENY_MAC);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&ruleIndex,	
							 DBUS_TYPE_UINT32,&ruleType,
							 DBUS_TYPE_UINT32,&actionType,
							 DBUS_TYPE_UINT32,&ethertype,
							 DBUS_TYPE_BYTE,  &dmacAddr.arEther[0],
							 DBUS_TYPE_BYTE,  &dmacAddr.arEther[1],
							 DBUS_TYPE_BYTE,  &dmacAddr.arEther[2],
							 DBUS_TYPE_BYTE,  &dmacAddr.arEther[3],
							 DBUS_TYPE_BYTE,  &dmacAddr.arEther[4],
							 DBUS_TYPE_BYTE,  &dmacAddr.arEther[5],
							 DBUS_TYPE_BYTE,  &smacAddr.arEther[0],
							 DBUS_TYPE_BYTE,  &smacAddr.arEther[1],
							 DBUS_TYPE_BYTE,  &smacAddr.arEther[2],
							 DBUS_TYPE_BYTE,  &smacAddr.arEther[3],
							 DBUS_TYPE_BYTE,  &smacAddr.arEther[4],
							 DBUS_TYPE_BYTE,  &smacAddr.arEther[5],	
							 DBUS_TYPE_UINT32,&policer,
							 DBUS_TYPE_UINT32,&policerId,
							 DBUS_TYPE_INVALID);
 
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_acl,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{
			if (ACL_RETURN_CODE_SUCCESS == op_ret ) 
			{
				/*vty_out(vty,"Access-list %d is successfully set!\n",(ruleIndex+1));*/
			}
			else if(ACL_RETURN_CODE_GLOBAL_EXISTED == op_ret)
				vty_out(vty,"%% Access-list %d existed!\n",(ruleIndex+1));
			
			else if((ACL_RETURN_CODE_EXT_NO_SPACE==op_ret)&&(ruleType == STANDARD_ACL_RULE))
		         	vty_out(vty,"%% because the extended should take up twice spaces than stdard acl,but %d has been set extended!\n",(ruleIndex-511));
			else if((ACL_RETURN_CODE_EXT_NO_SPACE==op_ret)&&(ruleType == EXTENDED_ACL_RULE))
				vty_out(vty,"%% because the extended rule should take up twice spaces than standard acl,\
				 			  but %d has been set standard rule! ~~set fail\n",(ruleIndex+513));
			else if(ACL_RETURN_CODE_SAME_FIELD == op_ret)
				vty_out(vty,"%% identical fields of packet can not set again\n");	
			else if(ACL_RETURN_CODE_POLICER_ID_NOT_SET == op_ret)
					vty_out(vty,"%% Policer %d not existed!\n",policerId);
			else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret)
				vty_out(vty,"%% Product not support this function!\n");
			else 
				vty_out(vty,"%% set fail!\n");
		
	} 
	else 
	{
		/*vty_out(vty,"Failed get args.\n");*/
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(config_acl_rule_trap_to_cpu_ethertype_func,
	  config_acl_rule_trap_to_cpu_ethertype_cmd,
	  "acl extended <1-1000> trap ethertype ETHERTYPE dmac (H:H:H:H:H:H|any) smac (H:H:H:H:H:H|any)",
	  ACL_STR
	  "extended acl rule\n"
	  "extended rule index range in 1-500\n"
	  "acl rule action Trap-to-Cpu\n" 
	  "deny packets matched\n"
	  "ethertype range\n"
	  "range in 0-65534 or any\n"
	  "care destination mac address\n"
	  "Specify DMAC address in Hex. fomat e.g.:00:00:11:11:aa:aa\n"
	  "Any destination mac address\n"
	  "care source mac address \n"
	  "Specify SMAC address in Hexa.Eg:00:00:11:11:aa:aa\n"
	  "Any source mac address\n"
)
{
	DBusMessage 	*query = NULL, *reply = NULL;
	DBusError		err;
	ETHERADDR		dmacAddr,smacAddr;
	unsigned int	ruleIndex = 0,packetType = 0; 
	unsigned int	ruleType = 0,op_ret = 0;
	unsigned int	ethertype = 0xffff;
	char *			str[20]={"ff:ff:ff:ff:ff:ff"};
	char *			strcp = NULL;
	int Val=0, ret = 0;
	
	if((argc < 2)||(argc > 3))
	{
		vty_out(vty, "bad parameters number!\n");
		return CMD_WARNING;
	}	
	ret = dcli_str2ulong((char*)argv[0],&ruleIndex);
	if (ACL_RETURN_CODE_ERROR == ret) {
		vty_out(vty,"%% Illegal rule index!\n");
		return CMD_WARNING;
	}
	ruleType = EXTENDED_ACL_RULE; 
	if ((ruleType == EXTENDED_ACL_RULE)&&(ruleIndex > MAX_EXT_RULE_NUM)) {
		vty_out(vty,"extended rule must less than 500!\n");
		return CMD_WARNING;
	}
	ruleIndex = ruleIndex-1; 
	memset(&smacAddr,0,sizeof(ETHERADDR));
	memset(&dmacAddr,0,sizeof(ETHERADDR));
	/*memset(&maskMac,0,sizeof(ETHERADDR));*/
	/*fetch the 1st param : DMAC addr*/

	if(strncmp("any",argv[1],strlen(argv[1]))==0)	{   
		ethertype = 0xffff;
	}
	else{
		/*ret= dcli_str2ulong((char*)argv[1],&ethertype);*/
		/*change for hex max ethertype is 0xfffe*/
		if (strlen((char*)argv[1]) > 6) {		
			vty_out(vty,"%% Illegal ethertype number!\n");
			return CMD_WARNING;
		}
		ethertype = strtoul((char*)argv[1], NULL, 0);
		if(ret==ACL_RETURN_CODE_ERROR || ethertype > 0xfffe) {
			vty_out(vty,"%% Illegal ethertype number!\n");
			return CMD_WARNING;
		}
	}

	if(strncmp("any",argv[2],strlen(argv[2]))!=0){
		op_ret = parse_mac_addr((char *)argv[2],&dmacAddr);
		if (ACL_RETURN_CODE_SUCCESS != op_ret) {
	    	vty_out(vty," %% Unknow mac addr format.\n");
			return CMD_SUCCESS;
		}
	}
	/*fetch the 1st param : SMAC addr*/
	if(strncmp("any",argv[3],strlen(argv[3]))!=0){
		op_ret = parse_mac_addr((char *)argv[3],&smacAddr);
		if (ACL_RETURN_CODE_SUCCESS != op_ret) {
	    	vty_out(vty,"%% Unknow mac addr format.\n");
			return CMD_SUCCESS;
		}
	}
		
	/*query*/
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_ACL_OBJPATH,NPD_DBUS_ACL_INTERFACE, NPD_DBUS_METHOD_CONFIG_ACL_RULE_TRAP_MAC);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&ruleIndex,	
							 DBUS_TYPE_UINT32,&ruleType,					 
							 DBUS_TYPE_UINT32,&ethertype,
							 DBUS_TYPE_BYTE,  &dmacAddr.arEther[0],
							 DBUS_TYPE_BYTE,  &dmacAddr.arEther[1],
							 DBUS_TYPE_BYTE,  &dmacAddr.arEther[2],
							 DBUS_TYPE_BYTE,  &dmacAddr.arEther[3],
							 DBUS_TYPE_BYTE,  &dmacAddr.arEther[4],
							 DBUS_TYPE_BYTE,  &dmacAddr.arEther[5],
						        DBUS_TYPE_BYTE,  &smacAddr.arEther[0],
							 DBUS_TYPE_BYTE,  &smacAddr.arEther[1],
							 DBUS_TYPE_BYTE,  &smacAddr.arEther[2],
							 DBUS_TYPE_BYTE,  &smacAddr.arEther[3],
							 DBUS_TYPE_BYTE,  &smacAddr.arEther[4],
							 DBUS_TYPE_BYTE,  &smacAddr.arEther[5],						
							 DBUS_TYPE_INVALID);
      
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_acl,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{
			if (ACL_RETURN_CODE_SUCCESS == op_ret ) 
			{
				/*vty_out(vty,"Access-list %d is successfully set!\n",(ruleIndex+1));*/
			}
			else if(ACL_RETURN_CODE_GLOBAL_EXISTED == op_ret)
				vty_out(vty,"%% Access-list %d existed!\n",(ruleIndex+1));
			else if((ACL_RETURN_CODE_EXT_NO_SPACE==op_ret)&&(ruleType == STANDARD_ACL_RULE))
		         	vty_out(vty,"%% because the extended should take up twice spaces than stdard acl,but %d has been set extended!\n",(ruleIndex-511));
			else if((ACL_RETURN_CODE_EXT_NO_SPACE==op_ret)&&(ruleType == EXTENDED_ACL_RULE))
				vty_out(vty,"%% because the extended rule should take up twice spaces than standard acl,\
				 			  but %d has been set standard rule! ~~set fail\n",(ruleIndex+513));
			else if(ACL_RETURN_CODE_SAME_FIELD == op_ret)
				vty_out(vty,"%% identical fields of packet can not set again\n");
			else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret)
				vty_out(vty,"%% Product not support this function!\n");
			else 
				vty_out(vty,"%% set fail!\n");
		
	} 
	else 
	{
		/*vty_out(vty,"Failed get args.\n");*/
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(config_acl_rule_mirror_redirect_ethertype_func,
	  config_acl_rule_mirror_redirect_ethertype_cmd,
	  "acl extended <1-1000> redirect PORTNO ethertype ETHERTYPE dmac (H:H:H:H:H:H|any) smac (H:H:H:H:H:H|any)",
	  ACL_STR
	  "extended acl rule\n"
	  "extended rule index range in 1-500\n"	 
	  "acl rule action redirect\n"
	  CONFIG_ETHPORT_STR 
	  "ethertype range\n"
	  "range in 0-65534 or any\n"
	  "care destination mac address\n"
	  "Specify DMAC address in Hexa.Eg:00:00:11:11:aa:aa\n"
	  "Any destination mac address\n"
	  "care source mac address \n"
	  "Specify SMAC address in Hexa.Eg:00:00:11:11:aa:aa\n"
	  "Any source mac address\n"
	  
)
{
	DBusMessage 	*query = NULL, *reply = NULL;
	DBusError		err;
	ETHERADDR		dmacAddr,smacAddr;
	unsigned int	ruleIndex = 0,packetType = 0,actionType = 0; 
	unsigned int	ruleType = 0,op_ret = 0,temp = 0;
	char *			str[20]={"ff:ff:ff:ff:ff:ff"};
	char *			strcp = NULL;
	unsigned char   slot_no=0,port_no=0;
	unsigned int 	policer=0,policerId=0,ethertype = 0xffff;
	int ret = 0;
	
	ret = dcli_str2ulong((char*)argv[0],&ruleIndex);
	if (ACL_RETURN_CODE_ERROR == ret) {
		vty_out(vty,"%% Illegal rule index!\n");
		return CMD_WARNING;
	}
	ruleType = EXTENDED_ACL_RULE; 
	if ((ruleType == EXTENDED_ACL_RULE)&&(ruleIndex > MAX_EXT_RULE_NUM)) {
		vty_out(vty,"extended rule must less than 500!\n");
		return CMD_WARNING;
	}
	ruleIndex = ruleIndex-1; 

	actionType=4;

	temp = parse_slotport_no((char*)argv[1],&slot_no,&port_no);

	if(strncmp("any",argv[2],strlen(argv[2]))==0)	{   
		ethertype = 0xffff;
	}
	else{
		/*ret= dcli_str2ulong((char*)argv[2],&ethertype);*/
		/*change for hex max ethertype is 0xfffe*/
		if (strlen((char*)argv[2]) > 6) {		
			vty_out(vty,"%% Illegal ethertype number!\n");
			return CMD_WARNING;
		}
		ethertype = strtoul((char*)argv[2], NULL, 0);
		if(ret==ACL_RETURN_CODE_ERROR || ethertype > 0xfffe) {
			vty_out(vty,"%% Illegal ethertype number!\n");
			return CMD_WARNING;
		}
	}

	if (ACL_RETURN_CODE_SUCCESS != ret) {
    	vty_out(vty,"%%  Illegal format with slot/port!\n");
		return CMD_WARNING;
	}
	memset(&smacAddr,0,sizeof(ETHERADDR));
	memset(&dmacAddr,0,sizeof(ETHERADDR));
	/*memset(&maskMac,0,sizeof(ETHERADDR));*/

	/*fetch the 1st param : DMAC addr*/
	if(strncmp("any",argv[3],strlen(argv[3]))!=0){
		op_ret = parse_mac_addr((char *)argv[3],&dmacAddr);
		if (ACL_RETURN_CODE_SUCCESS != op_ret) {
			vty_out(vty,"%% Unknown mac addr format.\n");
			return CMD_SUCCESS;
		}
	}
	/*fetch the 1st param : SMAC addr*/
	if(strncmp("any",argv[4],strlen(argv[4]))!=0){
		op_ret = parse_mac_addr((char *)argv[4],&smacAddr);
		if (ACL_RETURN_CODE_SUCCESS != op_ret) {
			vty_out(vty,"%% Unknown mac addr format.\n");
			return CMD_SUCCESS;
		}
	}
	/*query*/
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_ACL_OBJPATH,NPD_DBUS_ACL_INTERFACE, NPD_DBUS_METHOD_CONFIG_ACL_RULE_MIRROR_OR_REDIRECT_MAC);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&ruleIndex,	
							 DBUS_TYPE_UINT32,&ruleType,
							 DBUS_TYPE_UINT32,&actionType,
							 DBUS_TYPE_UINT32,&ethertype,
							 DBUS_TYPE_BYTE,  &dmacAddr.arEther[0],
							 DBUS_TYPE_BYTE,  &dmacAddr.arEther[1],
							 DBUS_TYPE_BYTE,  &dmacAddr.arEther[2],
							 DBUS_TYPE_BYTE,  &dmacAddr.arEther[3],
							 DBUS_TYPE_BYTE,  &dmacAddr.arEther[4],
							 DBUS_TYPE_BYTE,  &dmacAddr.arEther[5],
							 DBUS_TYPE_BYTE,  &smacAddr.arEther[0],
							 DBUS_TYPE_BYTE,  &smacAddr.arEther[1],
							 DBUS_TYPE_BYTE,  &smacAddr.arEther[2],
							 DBUS_TYPE_BYTE,  &smacAddr.arEther[3],
							 DBUS_TYPE_BYTE,  &smacAddr.arEther[4],
							 DBUS_TYPE_BYTE,  &smacAddr.arEther[5],
							 DBUS_TYPE_BYTE,  &slot_no,
							 DBUS_TYPE_BYTE,  &port_no,
							 DBUS_TYPE_UINT32,&policer,							
 							 DBUS_TYPE_UINT32,&policerId,
							 DBUS_TYPE_INVALID);
 
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_acl,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{
			if (ACL_RETURN_CODE_SUCCESS == op_ret ) 
			{
				/*vty_out(vty,"Access-list %d is successfully set!\n",(ruleIndex+1));*/
			}
			else if(NPD_DBUS_ERROR_NO_SUCH_PORT==op_ret)
				vty_out(vty,"%%  illegal port index\n");
			else if(ACL_RETURN_CODE_GLOBAL_EXISTED == op_ret)
				vty_out(vty,"%%  Access-list %d existed!\n",(ruleIndex+1));
			else if((ACL_RETURN_CODE_EXT_NO_SPACE==op_ret)&&(ruleType == STANDARD_ACL_RULE))
		         	vty_out(vty,"%% because the extended should take up twice spaces than stdard acl,but %d has been set extended!\n",(ruleIndex-511));
			else if((ACL_RETURN_CODE_EXT_NO_SPACE==op_ret)&&(ruleType == EXTENDED_ACL_RULE))
				vty_out(vty,"%% because the extended rule should take up twice spaces than standard acl,\
				 			  but %d has been set standard rule! ~~set fail\n",(ruleIndex+513));
			else if(ACL_RETURN_CODE_SAME_FIELD == op_ret)
				vty_out(vty,"%%  identical fields of packet can not set again\n");	
			else if(ACL_RETURN_CODE_POLICER_ID_NOT_SET == op_ret)
				vty_out(vty,"%% Policer %d not existed!\n",policerId);
			else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret)
				vty_out(vty,"%% Product not support this function!\n");
			else 
				vty_out(vty,"%%  set fail!\n");
	} 
	else 
	{
		/*vty_out(vty,"Failed get args.\n");*/
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}


DEFUN(config_acl_rule_deny_icmp_func,
	  config_acl_rule_deny_icmp_cmd,
	  "acl (standard|extended) <1-1000> (permit|deny) icmp dip (A.B.C.D/M |any) sip (A.B.C.D/M |any) type (<0-255>|any) code (<0-255>|any) [policer] [<1-255>]",
	  ACL_STR
	  "standard acl rule\n"
	  "extended acl rule\n"
	  "standard rule index range in 1-1000, extended rule index range in 1-500\n"
	  "acl rule action permit\n" 
	  "acl rule action deny\n"
	  "Source IP address for ICMP packet\n"
	  "Specify IP address in <A.B.C.D/M> format\n"
	  "Any legal source IP address\n"
	  "Icmp type with ICMP packet\n"
	  "Specify type number in range 0-255\n"
	  "Any legal type number\n"
	  "Icmp code with ICMP packet\n"
	  "Specifycode number in range 0-255\n"
	  "Any legal code number\n"
	  "Allow policing on ingress port\n"
	  "Policer Id range in 1-255\n"
)
{
	DBusMessage 	*query = NULL, *reply = NULL;
	DBusError		err;
	unsigned int	ruleIndex = 0,sipmaskLen = 0,dipmaskLen = 0;
	unsigned long	 dipno = 0, sipno = 0;
	unsigned  int	ruleType = 0,actionType = 0,ret = 0,i = 0;
	unsigned long	op_ret = 0;
	unsigned char   typeno=0,codeno=0,acl_any=0;;
	unsigned int     tmp1=0,tmp2=0;
	unsigned int	policer=0,policerId=0;
	int Val=0;
	
	#if 0
	if((argc < 2)||(argc > 6))
	{
		vty_out(vty, "bad parameters number!\n");
		return CMD_FAILURE;
	}
	#endif
	if(strncmp("standard",argv[0],strlen(argv[0]))==0)
	{
		ruleType = STANDARD_ACL_RULE;
	}
	else if (strncmp("extended",argv[0],strlen(argv[0]))==0)
	{
		ruleType = EXTENDED_ACL_RULE;
	}
	else
	{
		vty_out(vty,"bad command parameter!\n");
		return CMD_WARNING;
	}	
	ret = dcli_str2ulong((char*)argv[1],&ruleIndex);
	if (ACL_RETURN_CODE_ERROR == ret) {
		vty_out(vty,"%% Illegal rule index!\n");
		return CMD_WARNING;
	}

	if ((ruleType == EXTENDED_ACL_RULE)&&(ruleIndex > MAX_EXT_RULE_NUM)) {
		vty_out(vty,"extended rule must less than 500!\n");
		return CMD_WARNING;
	}
	ruleIndex = ruleIndex-1;

	if(strncmp("permit",argv[2],strlen(argv[2]))==0) { actionType=0;}
    else if(strncmp("deny",argv[2],strlen(argv[2]))==0) {actionType=1;}      
   
	
	if(strncmp("any",argv[3],strlen(argv[3]))==0)
		{
			dipmaskLen=0;
			dipno=0;
		}
	else
		{	
			Val=ip_address_format2ulong((char**)&argv[3],&dipno,&dipmaskLen);			
			if(CMD_WARNING == Val) {
				 vty_out(vty, "%% Bad parameter %s\n", argv[3]);
				 return CMD_WARNING;
			}
			VALUE_IP_MASK_CHECK(dipmaskLen);
	}

	if(strncmp("any",argv[4],strlen(argv[4]))==0)
		{ 
			sipmaskLen=0;
			sipno=0;
		}
	else
		{						
		  Val=ip_address_format2ulong((char**)&argv[4],&sipno,&sipmaskLen); 	
		  if(CMD_WARNING == Val) {
			   vty_out(vty, "%% Bad parameter %s\n", argv[4]);
			   return CMD_WARNING;
		  }
		  VALUE_IP_MASK_CHECK(sipmaskLen);
	}
		
	if(strncmp("any",argv[5],strlen(argv[5]))==0) {	   
		typeno=0;
		acl_any |= 1;
	}
	else{
		ret= dcli_str2ulong((char*)argv[5],&tmp1);
		if(ret==ACL_RETURN_CODE_ERROR)
	   	{
			vty_out(vty,"%% Illegal icmp type!\n");
			return CMD_WARNING;
	     }
	      typeno = tmp1;
	} 
	if(strncmp("any",argv[6],strlen(argv[6]))==0) {	   
		codeno=0;
		acl_any |= 2;
	}
	else{
		ret= dcli_str2ulong((char*)argv[6],&tmp2);
		if(ret==ACL_RETURN_CODE_ERROR)
	   	{
			vty_out(vty,"%% Illegal icmp code!\n");
			return CMD_WARNING;
	      }
	      codeno =tmp2;
	}
	
	/*policer*/
	if((argc>7)&&(argc<10)){
		if(actionType==0){	
			if(strncmp("policer",argv[7],strlen(argv[7]))==0){
				policer = 1;
				
				  ret= dcli_str2ulong((char *)argv[8],&policerId);
				   if(ret==ACL_RETURN_CODE_ERROR)
				   	{
						vty_out(vty,"%% Illegal policer ID!\n");
						return CMD_WARNING;
				   }
			}
			else{
				vty_out(vty,"%% unknown command\n");
				return CMD_WARNING;
			}			
		}
		else if(actionType==1){
			vty_out(vty,"%% Policer not support deny operation!\n");
			return CMD_WARNING;
		}
	}
	else if(argc>9){
		vty_out(vty,"too many param\n");
		return CMD_WARNING;
	}	
		
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_ACL_OBJPATH,NPD_DBUS_ACL_INTERFACE, NPD_DBUS_METHOD_CONFIG_ACL_RULE_DENY_ICMP);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&ruleIndex,	
							 DBUS_TYPE_UINT32,&ruleType,
							 DBUS_TYPE_UINT32,&actionType,
							 DBUS_TYPE_UINT32,&sipmaskLen,
							 DBUS_TYPE_UINT32,&dipmaskLen,
							 DBUS_TYPE_UINT32,&dipno,
							 DBUS_TYPE_UINT32,&sipno,
							 DBUS_TYPE_BYTE,&typeno,
							 DBUS_TYPE_BYTE,&codeno,
							 DBUS_TYPE_BYTE,&acl_any,
							 DBUS_TYPE_UINT32,&policer,
							 DBUS_TYPE_UINT32,&policerId,
							 DBUS_TYPE_INVALID);

		   
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_acl,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{
			if (ACL_RETURN_CODE_SUCCESS == op_ret ) {
				/*vty_out(vty,"Access-list %d is successfully set!\n",(ruleIndex+1));*/
			}
			else if(ACL_RETURN_CODE_GLOBAL_EXISTED == op_ret)
				vty_out(vty,"%% acl %d existed!\n",(ruleIndex+1));
			else if((ACL_RETURN_CODE_EXT_NO_SPACE==op_ret)&&(ruleType == STANDARD_ACL_RULE))
		         	vty_out(vty,"%% because the extended should take up twice spaces than stdard acl,but %d has been set extended!\n",(ruleIndex-511));
			else if((ACL_RETURN_CODE_EXT_NO_SPACE==op_ret)&&(ruleType == EXTENDED_ACL_RULE))
				vty_out(vty,"%% because the extended rule should take up twice spaces than standard acl,\
				 			  but %d has been set standard rule! ~~set fail\n",(ruleIndex+513));
			else if(ACL_RETURN_CODE_SAME_FIELD == op_ret)
				vty_out(vty,"%% identical fields of packet can not set again\n");	
			else if(ACL_RETURN_CODE_POLICER_ID_NOT_SET == op_ret)
				vty_out(vty,"%% Policer %d not existed!\n",policerId);
			else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret)
				vty_out(vty,"%% Product not support this function!\n");
			else 
				vty_out(vty,"%% set fail!\n");
		
	} 
	else 
	{
		/*vty_out(vty,"Failed get args.\n");*/
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
	
}

DEFUN(config_acl_rule_deny_ip_func,
	  config_acl_rule_deny_ip_cmd,
	  "acl (standard|extended) <1-1000> (permit|deny) ip dip (A.B.C.D/M |any) sip (A.B.C.D/M |any) [policer] [<1-255>]",
	  ACL_STR
	  "standard acl rule\n"
	  "extended acl rule\n"
      "standard rule index range in 1-1000, extended rule index range in 1-500\n"
	  "acl rule action permit\n" 
	  "acl rule action deny\n"
	  "acl rule deal with IP packet\n"
	  "Destination IP address for IP packet\n"
	  "Specify IP address in <A.B.C.D/M> format\n"
	  "Any legal destination IP address\n"
	  "Source IP address for TCP OR UDP packet\n"
	  "Specify IP address in <A.B.C.D/M> format\n"
	  "Any legal source IP address\n"
	  "Allow policing on ingress port\n"
	  "Policer Id range in 1-255\n"
)
{
	DBusMessage 	*query = NULL, *reply = NULL;
	DBusError		err;
	unsigned int	ruleIndex = 0;
	unsigned long	dipno = 0, sipno = 0;
	unsigned int	sipmaskLen = 0,dipmaskLen = 0;
	unsigned  int	ruleType = 0,actionType = 0,ret = 0,i = 0;
	unsigned long	op_ret = 0;
	unsigned int	policer=0,policerId=0;
	int Val=0;
	
	#if 0
	if((argc < 2)||(argc > 4))
	{
		vty_out(vty, "bad parameters number!\n");
		return CMD_WARNING;
	}
	#endif
	if(strncmp("standard",argv[0],strlen(argv[0]))==0)
	{
		ruleType = STANDARD_ACL_RULE;
	}
	else if (strncmp("extended",argv[0],strlen(argv[0]))==0)
	{
		ruleType = EXTENDED_ACL_RULE;
	}
	else
	{
		vty_out(vty,"bad command parameter!\n");
		return CMD_WARNING;
	}
	ret = dcli_str2ulong((char*)argv[1],&ruleIndex);
	if (ACL_RETURN_CODE_ERROR == ret) {
		vty_out(vty,"%% Illegal rule index!\n");
		return CMD_WARNING;
	}
	if ((ruleType == EXTENDED_ACL_RULE)&&(ruleIndex > MAX_EXT_RULE_NUM)) {
		vty_out(vty,"extended rule must less than 500!\n");
		return CMD_WARNING;
	}
	ruleIndex = ruleIndex-1;

	if(strncmp("permit",argv[2],strlen(argv[2]))==0) { actionType=0;}
    else if(strncmp("deny",argv[2],strlen(argv[2]))==0) {actionType=1;}      
   

	if(strncmp("any",argv[3],strlen(argv[3]))==0)
		{
			dipmaskLen=0;
			dipno=0;
		}
	else
		{	
			Val=ip_address_format2ulong((char**)&argv[3],&dipno,&dipmaskLen);			
			if(CMD_WARNING == Val) {
				 vty_out(vty, "%% Bad parameter %s\n", argv[3]);
				 return CMD_WARNING;
			}
			VALUE_IP_MASK_CHECK(dipmaskLen);
	  }
	
	if(strncmp("any",argv[4],strlen(argv[4]))==0)
		{ 
			sipmaskLen=0;
			sipno=0;
		}
	else
		{						
		  Val=ip_address_format2ulong((char**)&argv[4],&sipno,&sipmaskLen); 	
		  if(CMD_WARNING == Val) {
			   vty_out(vty, "%% Bad parameter %s\n", argv[4]);
			   return CMD_WARNING;
		  }
		  VALUE_IP_MASK_CHECK(sipmaskLen);
	}
	
	/*policer*/
	   if((argc>5)&&(argc<8)){
		   if(actionType==0){  
			   if(strncmp("policer",argv[5],strlen(argv[5]))==0){
				   policer = 1;
				    ret= dcli_str2ulong((char *)argv[6],&policerId);
				   if(ret==ACL_RETURN_CODE_ERROR)
				   	{
						vty_out(vty,"%% Illegal policer ID!\n");
						return CMD_WARNING;
				   }
			   }
			   else{
				   vty_out(vty,"%% unknown command\n");
				   return CMD_WARNING;
			   }		   
		   }
		   else if(actionType==1){
		   		vty_out(vty,"%% Policer not support deny operation!\n");
			    return CMD_WARNING;
		   }
	   }
	   else if(argc>7){
		   vty_out(vty,"%% too many param\n");
		   return CMD_WARNING;
	   }

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_ACL_OBJPATH, \
					NPD_DBUS_ACL_INTERFACE, NPD_DBUS_METHOD_CONFIG_ACL_RULE_DENY_IP);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&ruleIndex,				  
							 DBUS_TYPE_UINT32,&ruleType,
							 DBUS_TYPE_UINT32,&actionType,
							 DBUS_TYPE_UINT32,&sipmaskLen,
							 DBUS_TYPE_UINT32,&dipmaskLen,
							 DBUS_TYPE_UINT32,&dipno,
							 DBUS_TYPE_UINT32,&sipno,
							  DBUS_TYPE_UINT32,&policer,							
 							 DBUS_TYPE_UINT32,&policerId,
							 DBUS_TYPE_INVALID);
	   
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_acl,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{
			if (ACL_RETURN_CODE_SUCCESS == op_ret ) {
				/*vty_out(vty,"Access-list %d is successfully set!\n",(ruleIndex+1));*/
			}		
			else if(ACL_RETURN_CODE_GLOBAL_EXISTED == op_ret)
				vty_out(vty,"%% Access-list %d existed!\n",(ruleIndex+1));
			
			else if((ACL_RETURN_CODE_EXT_NO_SPACE==op_ret)&&(ruleType == STANDARD_ACL_RULE))
		         	vty_out(vty,"%% because the extended should take up twice spaces than stdard acl,but %d has been set extended!\n",(ruleIndex-511));
			else if((ACL_RETURN_CODE_EXT_NO_SPACE==op_ret)&&(ruleType == EXTENDED_ACL_RULE))
				vty_out(vty,"%% because the extended rule should take up twice spaces than standard acl,\
				 			  but %d has been set standard rule! ~~set fail\n",(ruleIndex+513));

			else if(ACL_RETURN_CODE_SAME_FIELD == op_ret)
				vty_out(vty,"%% identical fields of packet can not set again\n");	
            		else if(ACL_RETURN_CODE_POLICER_ID_NOT_SET == op_ret)
				vty_out(vty,"%% Policer %d not existed!\n",policerId);
			else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret)
				vty_out(vty,"%% Product not support this function!\n");
			else 
				vty_out(vty,"%% set fail!\n");
		
	} 
	else 
	{
		/*vty_out(vty,"Failed get args.\n");*/
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
	
}

DEFUN(config_acl_rule_deny_ipv6_func,
	  config_acl_rule_deny_ipv6_cmd,
	  "acl extended <1-1000> (permit|deny|trap) next-header <0-255> dipv6 (IPV6 |any) sipv6 (IPV6 |any) [policer] [<1-255>]",
	  ACL_STR
	  "extended acl rule\n"
      "extended rule index range in 1-1000\n"
	  "acl rule action permit\n" 
	  "acl rule action deny\n"
	  "acl rule action trap-to-cpu\n"
	  "identity of IPv6 next header\n"
	  "identity range in 1-255\n"
	  "Destination IP address for IP packet\n"
	  "Specify IP address in <X:X:X:X:X:X:X:X> format\n"
	  "Any legal destination IP address\n"
	  "Source IP address for TCP OR UDP packet\n"
	  "Specify IPv6 address\n"
	  "Any legal source IP address\n"
	  "Allow policing on ingress port\n"
	  "Policer Id range in 1-255\n"
)
{
	DBusMessage 	*query = NULL, *reply = NULL;
	DBusError		err;
	unsigned int	ruleIndex = 0, nextheader = 0;
	unsigned int	sipmaskLen = 1,dipmaskLen = 1;
	unsigned  int	ruleType = 0,actionType = 0,ret = 0,i = 0;
	unsigned long	op_ret = 0;
	unsigned int	policer=0,policerId=0;
	int Val=0;
	struct ipv6addr dip, sip;
	
	memset(&dip, 0, sizeof(struct ipv6addr));
	memset(&sip, 0, sizeof(struct ipv6addr));
	
	ret = dcli_str2ulong((char*)argv[0],&ruleIndex);
	if (ACL_RETURN_CODE_ERROR == ret) {
		vty_out(vty,"%% Illegal rule index!\n");
		return CMD_WARNING;
	}
	if ((ruleType == EXTENDED_ACL_RULE)&&(ruleIndex > MAX_EXT_RULE_NUM)) {
		vty_out(vty,"extended rule must less than 500!\n");
		return CMD_WARNING;
	}
	ruleIndex = ruleIndex-1;
	
	ruleType = EXTENDED_ACL_RULE;
	if ((ruleType == EXTENDED_ACL_RULE)&&(ruleIndex > MAX_EXT_RULE_NUM)) {
		vty_out(vty,"extended rule must less than 500!\n");
		return CMD_WARNING;
	}

	if(strncmp("permit",argv[1],strlen(argv[1]))==0) {
		actionType=0;
	}
       else if(strncmp("deny",argv[1],strlen(argv[1]))==0) {
	   	actionType=1;
	} 
	else if(strncmp("trap",argv[1],strlen(argv[1]))==0) {
	   	actionType=2;
	}   
   
  	ret = dcli_str2ulong((char*)argv[2],&nextheader);
	if (ACL_RETURN_CODE_ERROR == ret) {
		vty_out(vty,"%% Illegal rule index!\n");
		return CMD_WARNING;
	}

	if(strncmp("any",argv[3],strlen(argv[3]))==0) {
		dipmaskLen=0;
	}
	else {	
		ret = str2_ipv6_addr((char*)argv[3], &dip);
		if (!ret) {
			return CMD_WARNING;
		}
		dipmaskLen=1;
	}
	
	if(strncmp("any",argv[4],strlen(argv[4]))==0) { 
		sipmaskLen=0;
	}
	else {						
		ret = str2_ipv6_addr((char*)argv[4], &sip);
		if (!ret) {
			return CMD_WARNING;
		}
		sipmaskLen=1;
	}
	
	/*policer*/
	   if((argc>5)&&(argc<8)){
		   if(actionType==0){  
			   if(strncmp("policer",argv[5],strlen(argv[5]))==0){
				   policer = 1;
				    ret= dcli_str2ulong((char *)argv[6],&policerId);
				   if(ret==ACL_RETURN_CODE_ERROR)
				   	{
						vty_out(vty,"%% Illegal policer ID!\n");
						return CMD_WARNING;
				   }
			   }
			   else{
				   vty_out(vty,"%% unknown command\n");
				   return CMD_WARNING;
			   }		   
		   }
		   else if(actionType==1){
		   		vty_out(vty,"%% Policer not support deny operation!\n");
			    return CMD_WARNING;
		   }
		    else if(actionType==2){
		   		vty_out(vty,"%% Policer not support trap operation!\n");
			    return CMD_WARNING;
		   }
	   }
	   else if(argc>7){
		   vty_out(vty,"%% too many param\n");
		   return CMD_WARNING;
	   }


	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										NPD_DBUS_ACL_OBJPATH,
										NPD_DBUS_ACL_INTERFACE,
										NPD_DBUS_METHOD_CONFIG_ACL_RULE_DENY_IPV6);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &ruleIndex,				  
							DBUS_TYPE_UINT32, &ruleType,
							DBUS_TYPE_UINT32, &actionType,
							DBUS_TYPE_UINT32, &nextheader,
							DBUS_TYPE_UINT32, &sipmaskLen,
							DBUS_TYPE_UINT32, &dipmaskLen,
							DBUS_TYPE_BYTE, &(dip.ipbuf[0]),
							DBUS_TYPE_BYTE, &(dip.ipbuf[1]),
							DBUS_TYPE_BYTE, &(dip.ipbuf[2]),
							DBUS_TYPE_BYTE, &(dip.ipbuf[3]),
							DBUS_TYPE_BYTE, &(dip.ipbuf[4]),
							DBUS_TYPE_BYTE, &(dip.ipbuf[5]),
							DBUS_TYPE_BYTE, &(dip.ipbuf[6]),
							DBUS_TYPE_BYTE, &(dip.ipbuf[7]),
							DBUS_TYPE_BYTE, &(dip.ipbuf[8]),
							DBUS_TYPE_BYTE, &(dip.ipbuf[9]),
							DBUS_TYPE_BYTE, &(dip.ipbuf[10]),
							DBUS_TYPE_BYTE, &(dip.ipbuf[11]),
							DBUS_TYPE_BYTE, &(dip.ipbuf[12]),
							DBUS_TYPE_BYTE, &(dip.ipbuf[13]),
							DBUS_TYPE_BYTE, &(dip.ipbuf[14]),							
							DBUS_TYPE_BYTE, &(dip.ipbuf[15]),
							DBUS_TYPE_BYTE, &(sip.ipbuf[0]),
							DBUS_TYPE_BYTE, &(sip.ipbuf[1]),
							DBUS_TYPE_BYTE, &(sip.ipbuf[2]),
							DBUS_TYPE_BYTE, &(sip.ipbuf[3]),
							DBUS_TYPE_BYTE, &(sip.ipbuf[4]),
							DBUS_TYPE_BYTE, &(sip.ipbuf[5]),
							DBUS_TYPE_BYTE, &(sip.ipbuf[6]),
							DBUS_TYPE_BYTE, &(sip.ipbuf[7]),
							DBUS_TYPE_BYTE, &(sip.ipbuf[8]),
							DBUS_TYPE_BYTE, &(sip.ipbuf[9]),
							DBUS_TYPE_BYTE, &(sip.ipbuf[10]),
							DBUS_TYPE_BYTE, &(sip.ipbuf[11]),
							DBUS_TYPE_BYTE, &(sip.ipbuf[12]),
							DBUS_TYPE_BYTE, &(sip.ipbuf[13]),
							DBUS_TYPE_BYTE, &(sip.ipbuf[14]),							
							DBUS_TYPE_BYTE, &(sip.ipbuf[15]),	
							DBUS_TYPE_UINT32, &policer,							
							DBUS_TYPE_UINT32, &policerId,
							DBUS_TYPE_INVALID);
	   
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_acl,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{
			if (ACL_RETURN_CODE_SUCCESS == op_ret ) {
				/*vty_out(vty,"Access-list %d is successfully set!\n",(ruleIndex+1));*/
			}		
			else if(ACL_RETURN_CODE_GLOBAL_EXISTED == op_ret)
				vty_out(vty,"%% Access-list %d existed!\n",(ruleIndex+1));
			
			else if((ACL_RETURN_CODE_EXT_NO_SPACE==op_ret)&&(ruleType == STANDARD_ACL_RULE))
		         	vty_out(vty,"%% because the extended should take up twice spaces than stdard acl,but %d has been set extended!\n",(ruleIndex-511));
			else if((ACL_RETURN_CODE_EXT_NO_SPACE==op_ret)&&(ruleType == EXTENDED_ACL_RULE))
				vty_out(vty,"%% because the extended rule should take up twice spaces than standard acl,\
				 			  but %d has been set standard rule! ~~set fail\n",(ruleIndex+513));

			else if(ACL_RETURN_CODE_SAME_FIELD == op_ret)
				vty_out(vty,"%% identical fields of packet can not set again\n");	
            		else if(ACL_RETURN_CODE_POLICER_ID_NOT_SET == op_ret)
				vty_out(vty,"%% Policer %d not existed!\n",policerId);
			else 
				vty_out(vty,"%% set fail!\n");
		
	} 
	else 
	{
		/*vty_out(vty,"Failed get args.\n");*/
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
	
}


DEFUN(config_acl_rule_redirect_ipv6_func,
	  config_acl_rule_redirect_ipv6_cmd,
	  "acl extended <1-1000> redirect PORTNO next-header <0-255> dipv6 (IPV6 |any) sipv6 (IPV6 |any)",
	  ACL_STR	
	  "extended acl rule\n"
         "extended rule index range in 1-500\n"
	  "acl rule action redirect\n"
	  CONFIG_ETHPORT_STR 
	  "identity of the first extension header\n"
	  "identity range in 1-255\n"
	  "Destination IP address for IPV6 packet\n"
	  "IPV6 address format \n"
	  "Any legal destination IPV6 address\n"
	  "Source IP address IPV6 packet\n"
	  "IPV6 address format\n"
	  "Any legal source IPV6 address\n"
)
{
	DBusMessage 	*query = NULL, *reply = NULL;
	DBusError		err;
	unsigned int	ruleIndex = 0, nextheader = 0;
	unsigned int	sipmaskLen = 1,dipmaskLen = 1;
	unsigned  int	ruleType = 0,actionType = 0,ret = 0,i = 0;
	unsigned long	op_ret = 0;	
	unsigned char   mslot=0,mport=0;
	int Val=0;
	struct ipv6addr dip, sip;

	
	if(argc>5){
		  vty_out(vty,"%% too many param\n");
		  return CMD_WARNING;
	}
	memset(&dip, 0, sizeof(struct ipv6addr));
	memset(&sip, 0, sizeof(struct ipv6addr));
	
	ret = dcli_str2ulong((char*)argv[0],&ruleIndex);
	if (ACL_RETURN_CODE_ERROR == ret) {
		vty_out(vty,"%% Illegal rule index!\n");
		return CMD_WARNING;
	}
	if ((ruleType == EXTENDED_ACL_RULE)&&(ruleIndex > MAX_EXT_RULE_NUM)) {
		vty_out(vty,"extended rule must less than 500!\n");
		return CMD_WARNING;
	}
	ruleIndex = ruleIndex-1;
	
	ruleType = EXTENDED_ACL_RULE;
	if ((ruleType == EXTENDED_ACL_RULE)&&(ruleIndex > MAX_EXT_RULE_NUM)) {
		vty_out(vty,"extended rule must less than 500!\n");
		return CMD_WARNING;
	}

	actionType=4;
   
  	/*mirror or redirect port*/
   	ret = parse_slotport_no((char *)argv[1],&mslot,&mport);
	if (ACL_RETURN_CODE_SUCCESS != ret) {
    		vty_out(vty,"%% Unknow portno format.\n");
		return CMD_SUCCESS;
	}
	ret = dcli_str2ulong((char*)argv[2],&nextheader);
	if (ACL_RETURN_CODE_ERROR == ret) {
		vty_out(vty,"%% Illegal rule index!\n");
		return CMD_WARNING;
	}

	
	if(strncmp("any",argv[3],strlen(argv[3]))==0) {
		dipmaskLen=0;
	}
	else {	
		ret = str2_ipv6_addr((char*)argv[3], &dip);
		if (!ret) {
			vty_out(vty,"%% Illegal ipv6 format.\n");
			return CMD_WARNING;
		}
		dipmaskLen=1;
	}
	
	if(strncmp("any",argv[4],strlen(argv[4]))==0) { 
		sipmaskLen=0;
	}
	else {						
		ret = str2_ipv6_addr((char*)argv[4], &sip);
		if (!ret) {
			vty_out(vty,"%% Illegal ipv6 format.\n");
			return CMD_WARNING;
		}
		sipmaskLen=1;
	}	
	
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										NPD_DBUS_ACL_OBJPATH,
										NPD_DBUS_ACL_INTERFACE,
										NPD_DBUS_METHOD_CONFIG_ACL_REDIRECT_IPV6);
	dbus_error_init(&err);	
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &ruleIndex,				  
							DBUS_TYPE_UINT32, &ruleType,
							DBUS_TYPE_UINT32, &actionType,
							DBUS_TYPE_UINT32, &nextheader,
							DBUS_TYPE_BYTE,  &mslot,
							DBUS_TYPE_BYTE,  &mport,
							DBUS_TYPE_UINT32, &sipmaskLen,
							DBUS_TYPE_UINT32, &dipmaskLen,
							DBUS_TYPE_BYTE, &(dip.ipbuf[0]),
							DBUS_TYPE_BYTE, &(dip.ipbuf[1]),
							DBUS_TYPE_BYTE, &(dip.ipbuf[2]),
							DBUS_TYPE_BYTE, &(dip.ipbuf[3]),
							DBUS_TYPE_BYTE, &(dip.ipbuf[4]),
							DBUS_TYPE_BYTE, &(dip.ipbuf[5]),
							DBUS_TYPE_BYTE, &(dip.ipbuf[6]),
							DBUS_TYPE_BYTE, &(dip.ipbuf[7]),
							DBUS_TYPE_BYTE, &(dip.ipbuf[8]),
							DBUS_TYPE_BYTE, &(dip.ipbuf[9]),
							DBUS_TYPE_BYTE, &(dip.ipbuf[10]),
							DBUS_TYPE_BYTE, &(dip.ipbuf[11]),
							DBUS_TYPE_BYTE, &(dip.ipbuf[12]),
							DBUS_TYPE_BYTE, &(dip.ipbuf[13]),
							DBUS_TYPE_BYTE, &(dip.ipbuf[14]),							
							DBUS_TYPE_BYTE, &(dip.ipbuf[15]),
							DBUS_TYPE_BYTE, &(sip.ipbuf[0]),
							DBUS_TYPE_BYTE, &(sip.ipbuf[1]),
							DBUS_TYPE_BYTE, &(sip.ipbuf[2]),
							DBUS_TYPE_BYTE, &(sip.ipbuf[3]),
							DBUS_TYPE_BYTE, &(sip.ipbuf[4]),
							DBUS_TYPE_BYTE, &(sip.ipbuf[5]),
							DBUS_TYPE_BYTE, &(sip.ipbuf[6]),
							DBUS_TYPE_BYTE, &(sip.ipbuf[7]),
							DBUS_TYPE_BYTE, &(sip.ipbuf[8]),
							DBUS_TYPE_BYTE, &(sip.ipbuf[9]),
							DBUS_TYPE_BYTE, &(sip.ipbuf[10]),
							DBUS_TYPE_BYTE, &(sip.ipbuf[11]),
							DBUS_TYPE_BYTE, &(sip.ipbuf[12]),
							DBUS_TYPE_BYTE, &(sip.ipbuf[13]),
							DBUS_TYPE_BYTE, &(sip.ipbuf[14]),							
							DBUS_TYPE_BYTE, &(sip.ipbuf[15]),							
							DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_acl,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) 
	{
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{
			if (ACL_RETURN_CODE_SUCCESS == op_ret ) {
				/*vty_out(vty,"Access-list %d is successfully set!\n",(ruleIndex+1));*/
			}		
			else if(ACL_RETURN_CODE_GLOBAL_EXISTED == op_ret)
				vty_out(vty,"%% Access-list %d existed!\n",(ruleIndex+1));
			
			else if((ACL_RETURN_CODE_EXT_NO_SPACE==op_ret)&&(ruleType == STANDARD_ACL_RULE))
		         	vty_out(vty,"%% because the extended should take up twice spaces than stdard acl,but %d has been set extended!\n",(ruleIndex-511));
			else if((ACL_RETURN_CODE_EXT_NO_SPACE==op_ret)&&(ruleType == EXTENDED_ACL_RULE))
				vty_out(vty,"%% because the extended rule should take up twice spaces than standard acl,\
				 			  but %d has been set standard rule! ~~set fail\n",(ruleIndex+513));
			else if(ACL_RETURN_CODE_SAME_FIELD == op_ret)
				vty_out(vty,"%% identical fields of packet can not set again\n");            		
			else 
				vty_out(vty,"%% set fail!\n");
		
	} 
	else 
	{
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
	
}

DEFUN(config_extend_acl_rule_redirect_tcp_udp_ipv6_cmd_func,
	  config_extend_acl_rule_redirect_tcp_udp_ipv6_cmd,
	  "acl extended <1-1000> redirect PORTNO (tcp|udp) \
	  dipv6 (IPV6|any) dst-port (<0-65535>|any) sipv6 (IPV6|any ) \
	  src-port (<0-65535>|any )",
	  ACL_STR
	  "extended acl rule\n"
	  "extended acl rule index range in 1-500\n"
	  "acl rule action redirect\n"
	  CONFIG_ETHPORT_STR 
	  "match tcp packets\n"
	  "match udp packets\n"
	  "care destination ip\n"
	  "destination ip format <X:X:X:X:X:X:X:X>\n"
	  "any destination ip\n"
	  "care destination port\n"
	  "destination port range in 0-65535\n"
	  "any destination port\n"
	  "care source ip\n"
	  "source ip format <X:X:X:X:X:X:X:X>\n"
	  "any source ip\n"
	  "care source port\n"
	  "source port range in 0-65535\n"
	  "any source port\n"
)
{
	DBusMessage 	*query = NULL, *reply = NULL;
	DBusError		err;
	unsigned int	ruleIndex = 0, nextheader = 0;
	unsigned int	sipmaskLen = 1,dipmaskLen = 1, packetType = 0, srcport=0,dstport=0;
	unsigned  int	ruleType = 0,actionType = 0,ret = 0,i = 0;
	unsigned long	op_ret = 0; 
	unsigned char	mslot=0,mport=0;
	int Val=0;
	struct ipv6addr dip, sip;
	
	memset(&dip, 0, sizeof(struct ipv6addr));
	memset(&sip, 0, sizeof(struct ipv6addr));
	
	ret = dcli_str2ulong((char*)argv[0],&ruleIndex);
	if (ACL_RETURN_CODE_ERROR == ret) {
		vty_out(vty,"%% Illegal rule index!\n");
		return CMD_WARNING;
	}
	if ((ruleType == EXTENDED_ACL_RULE)&&(ruleIndex > MAX_EXT_RULE_NUM)) {
		vty_out(vty,"extended rule must less than 500!\n");
		return CMD_WARNING;
	}
	ruleIndex = ruleIndex-1;
	
	ruleType = EXTENDED_ACL_RULE;
	if ((ruleType == EXTENDED_ACL_RULE)&&(ruleIndex > MAX_EXT_RULE_NUM)) {
		vty_out(vty,"extended rule must less than 500!\n");
		return CMD_WARNING;
	}

	actionType=4;
   
	/*mirror or redirect port*/
	ret = parse_slotport_no((char *)argv[1],&mslot,&mport);
	if (ACL_RETURN_CODE_SUCCESS != ret) {
			vty_out(vty,"%% Unknow portno format.\n");
		return CMD_SUCCESS;
	}

	if(strncmp("udp",argv[2],strlen(argv[2]))==0)
		packetType=1;
	else if(strncmp("tcp",argv[2],strlen(argv[2]))==0)
		packetType=2;

	
	if(strncmp("any",argv[3],strlen(argv[3]))==0) {
		dipmaskLen=0;
	}
	else {	
		ret = str2_ipv6_addr((char*)argv[3], &dip);
		if (!ret) {
			vty_out(vty,"%% Illegal ipv6 format.\n");
			return CMD_WARNING;
		}
		dipmaskLen=1;
	}
	if(strncmp("any",argv[4],strlen(argv[4]))==0)	{   
		dstport = ACL_ANY_PORT;
	}
	else{
		ret= dcli_str2ulong((char*)argv[4],&dstport);
		if(ret==ACL_RETURN_CODE_ERROR)
		{
			vty_out(vty,"%% Illegal destination port!\n");
			return CMD_WARNING;
		}

	}
	
	if(strncmp("any",argv[5],strlen(argv[5]))==0) { 
		sipmaskLen=0;
	}
	else {						
		ret = str2_ipv6_addr((char*)argv[5], &sip);
		if (!ret) {
			vty_out(vty,"%% Illegal ipv6 format.\n");
			return CMD_WARNING;
		}
		sipmaskLen=1;
	}	
	if(strncmp("any",argv[6],strlen(argv[6]))==0) {	   
		srcport = ACL_ANY_PORT;
	}
	else{
		ret= dcli_str2ulong((char*)argv[6],&srcport);
		if(ret==ACL_RETURN_CODE_ERROR)
		{
			vty_out(vty,"%% Illegal source port!\n");
			return CMD_WARNING;
		}
	}
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										NPD_DBUS_ACL_OBJPATH,
										NPD_DBUS_ACL_INTERFACE,
										NPD_DBUS_METHOD_CONFIG_ACL_REDIRECT_TCP_UDP_IPV6);
	dbus_error_init(&err);	
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &ruleIndex,				  
							DBUS_TYPE_UINT32, &ruleType,
							DBUS_TYPE_UINT32, &actionType,
							DBUS_TYPE_UINT32, &packetType,
							DBUS_TYPE_BYTE,  &mslot,
							DBUS_TYPE_BYTE,  &mport,
							DBUS_TYPE_UINT32, &sipmaskLen,
							DBUS_TYPE_UINT32, &dipmaskLen,
							DBUS_TYPE_UINT32, &dstport,
							DBUS_TYPE_UINT32, &srcport,
							DBUS_TYPE_BYTE, &(dip.ipbuf[0]),
							DBUS_TYPE_BYTE, &(dip.ipbuf[1]),
							DBUS_TYPE_BYTE, &(dip.ipbuf[2]),
							DBUS_TYPE_BYTE, &(dip.ipbuf[3]),
							DBUS_TYPE_BYTE, &(dip.ipbuf[4]),
							DBUS_TYPE_BYTE, &(dip.ipbuf[5]),
							DBUS_TYPE_BYTE, &(dip.ipbuf[6]),
							DBUS_TYPE_BYTE, &(dip.ipbuf[7]),
							DBUS_TYPE_BYTE, &(dip.ipbuf[8]),
							DBUS_TYPE_BYTE, &(dip.ipbuf[9]),
							DBUS_TYPE_BYTE, &(dip.ipbuf[10]),
							DBUS_TYPE_BYTE, &(dip.ipbuf[11]),
							DBUS_TYPE_BYTE, &(dip.ipbuf[12]),
							DBUS_TYPE_BYTE, &(dip.ipbuf[13]),
							DBUS_TYPE_BYTE, &(dip.ipbuf[14]),							
							DBUS_TYPE_BYTE, &(dip.ipbuf[15]),
							DBUS_TYPE_BYTE, &(sip.ipbuf[0]),
							DBUS_TYPE_BYTE, &(sip.ipbuf[1]),
							DBUS_TYPE_BYTE, &(sip.ipbuf[2]),
							DBUS_TYPE_BYTE, &(sip.ipbuf[3]),
							DBUS_TYPE_BYTE, &(sip.ipbuf[4]),
							DBUS_TYPE_BYTE, &(sip.ipbuf[5]),
							DBUS_TYPE_BYTE, &(sip.ipbuf[6]),
							DBUS_TYPE_BYTE, &(sip.ipbuf[7]),
							DBUS_TYPE_BYTE, &(sip.ipbuf[8]),
							DBUS_TYPE_BYTE, &(sip.ipbuf[9]),
							DBUS_TYPE_BYTE, &(sip.ipbuf[10]),
							DBUS_TYPE_BYTE, &(sip.ipbuf[11]),
							DBUS_TYPE_BYTE, &(sip.ipbuf[12]),
							DBUS_TYPE_BYTE, &(sip.ipbuf[13]),
							DBUS_TYPE_BYTE, &(sip.ipbuf[14]),							
							DBUS_TYPE_BYTE, &(sip.ipbuf[15]),							
							DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_acl,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) 
	{
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{
			if (ACL_RETURN_CODE_SUCCESS == op_ret ) {
				/*vty_out(vty,"Access-list %d is successfully set!\n",(ruleIndex+1));*/
			}		
			else if(ACL_RETURN_CODE_GLOBAL_EXISTED == op_ret)
				vty_out(vty,"%% Access-list %d existed!\n",(ruleIndex+1));
			
			else if((ACL_RETURN_CODE_EXT_NO_SPACE==op_ret)&&(ruleType == STANDARD_ACL_RULE))
					vty_out(vty,"%% because the extended should take up twice spaces than stdard acl,but %d has been set extended!\n",(ruleIndex-511));
			else if((ACL_RETURN_CODE_EXT_NO_SPACE==op_ret)&&(ruleType == EXTENDED_ACL_RULE))
				vty_out(vty,"%% because the extended rule should take up twice spaces than standard acl,\
							  but %d has been set standard rule! ~~set fail\n",(ruleIndex+513));
			else if(ACL_RETURN_CODE_SAME_FIELD == op_ret)
				vty_out(vty,"%% identical fields of packet can not set again\n");						
			else 
				vty_out(vty,"%% set fail!\n");
		
	} 
	else 
	{
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
	
}



DEFUN(config_extend_acl_rule_permit_deny_ipv6_cmd_func,
	  config_extend_acl_rule_permit_deny_ipv6_cmd,
	  "acl extended <1-1000> (permit|deny|trap) (tcp|udp) \
	  dipv6 (IPV6|any) dst-port (<0-65535>|any) sipv6 (IPV6|any ) \
	  src-port (<0-65535>|any ) [policer] [<1-255>]",
	  ACL_STR
	  "extended acl rule\n"
	  "extended acl rule index range in 1-1000\n"
	  "permit matched packets\n"
	  "deny matched packets\n"
	  "trap to cpu matched packets\n"
	  "match tcp packets\n"
	  "match udp packets\n"
	  "care destination ip\n"
	  "destination ip format <X:X:X:X:X:X:X:X>\n"
	  "any destination ip\n"
	  "care destination port\n"
	  "destination port range in 0-65535\n"
	  "any destination port\n"
	  "care source ip\n"
	  "source ip format <X:X:X:X:X:X:X:X>\n"
	  "any source ip\n"
	  "care source port\n"
	  "source port range in 0-65535\n"
	  "any source port\n"
	  "Allow policing on ingress port\n"
	  "Policer Id range in 1-255\n"
	  
)
{	  
	DBusMessage 	*query = NULL, *reply = NULL;
	DBusError		err;
	unsigned int	ruleIndex = 0,dipmaskLen=1,sipmaskLen=1;
	unsigned int	actionType = 0,packetType = 0,srcport=0,dstport=0,vlanId=0;
	unsigned int     ruleType = 0,op_ret = 0,ret = 0, Val=0, policer = 0, policerId = 0;
	struct ipv6addr dip, sip;
	
	memset(&dip, 0, sizeof(struct ipv6addr));
	memset(&sip, 0, sizeof(struct ipv6addr));

	ret = dcli_str2ulong((char*)argv[0],&ruleIndex);
	if (ACL_RETURN_CODE_ERROR == ret) {
		vty_out(vty,"%% Illegal rule index!\n");
		return CMD_WARNING;
	}
	ruleIndex = ruleIndex-1;
	
	ruleType = EXTENDED_ACL_RULE; 
	if ((ruleType == EXTENDED_ACL_RULE)&&(ruleIndex > MAX_EXT_RULE_NUM)) {
		vty_out(vty,"extended rule must less than 500!\n");
		return CMD_WARNING;
	}
	
	if(strncmp("permit",argv[1],strlen(argv[1]))==0)
		actionType=0;
	else if(strncmp("deny",argv[1],strlen(argv[1]))==0)
		actionType=1;
	else if(strncmp("trap",argv[1],strlen(argv[1]))==0)
		actionType=2;
	
	if(strncmp("udp",argv[2],strlen(argv[2]))==0)
		packetType=1;
	else if(strncmp("tcp",argv[2],strlen(argv[2]))==0)
		packetType=2;
	
	if(strncmp("any",argv[3],strlen(argv[3]))==0) {
		dipmaskLen=0;
	}
	else {	
		ret = str2_ipv6_addr((char*)argv[3], &dip);
		if (!ret) {
			return CMD_WARNING;
		}
		dipmaskLen=1;
	}

	if(strncmp("any",argv[4],strlen(argv[4]))==0)	{   
		dstport = ACL_ANY_PORT;
	}
	else{
		ret= dcli_str2ulong((char*)argv[4],&dstport);
		if(ret==ACL_RETURN_CODE_ERROR)
		{
			vty_out(vty,"%% Illegal destination port!\n");
			return CMD_WARNING;
		}

	}
	
	if(strncmp("any",argv[5],strlen(argv[5]))==0) { 
		sipmaskLen=0;
	}
	else {						
		ret = str2_ipv6_addr((char*)argv[5], &sip);
		if (!ret) {
			return CMD_WARNING;
		}
		sipmaskLen=1;
	}
	
	if(strncmp("any",argv[6],strlen(argv[6]))==0) {	   
		srcport = ACL_ANY_PORT;
	}
	else{
		ret= dcli_str2ulong((char*)argv[6],&srcport);
		if(ret==ACL_RETURN_CODE_ERROR)
		{
			vty_out(vty,"%% Illegal source port!\n");
			return CMD_WARNING;
		}
	}

	/*policer*/
	if((argc>7)&&(argc<10)){
		if(actionType==0){	
			if(strncmp("policer",argv[7],strlen(argv[7]))==0){
				policer = 1;
				ret = dcli_str2ulong((char *)argv[8],&policerId);
				if(ret==ACL_RETURN_CODE_ERROR)
				{
					vty_out(vty,"%% Illegal policer ID!\n");
					return CMD_WARNING;
				}
			}
			else{
				vty_out(vty,"unknown command\n");
				return CMD_WARNING;
			}			
		}
		else if(actionType==1){
			vty_out(vty,"%% Policer not support deny operation!\n");
			 return CMD_WARNING;
		}
		else if(actionType==2){
			vty_out(vty,"%% Policer not support trap operation!\n");
			 return CMD_WARNING;
		}
	}
	else if(argc>9){
		vty_out(vty,"too many param\n");
		return CMD_WARNING;
	}	
		
	/*query*/
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										NPD_DBUS_ACL_OBJPATH,
										NPD_DBUS_ACL_INTERFACE,
										NPD_DBUS_METHOD_CONFIG_ACL_RULE_EXTENDED_PERMIT_DENY_TCP_UDP_IPV6);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &ruleIndex,	
							DBUS_TYPE_UINT32, &ruleType,
							DBUS_TYPE_UINT32, &actionType,
							DBUS_TYPE_UINT32, &packetType,
							DBUS_TYPE_BYTE, &(dip.ipbuf[0]),
							DBUS_TYPE_BYTE, &(dip.ipbuf[1]),
							DBUS_TYPE_BYTE, &(dip.ipbuf[2]),
							DBUS_TYPE_BYTE, &(dip.ipbuf[3]),
							DBUS_TYPE_BYTE, &(dip.ipbuf[4]),
							DBUS_TYPE_BYTE, &(dip.ipbuf[5]),
							DBUS_TYPE_BYTE, &(dip.ipbuf[6]),
							DBUS_TYPE_BYTE, &(dip.ipbuf[7]),
							DBUS_TYPE_BYTE, &(dip.ipbuf[8]),
							DBUS_TYPE_BYTE, &(dip.ipbuf[9]),
							DBUS_TYPE_BYTE, &(dip.ipbuf[10]),
							DBUS_TYPE_BYTE, &(dip.ipbuf[11]),
							DBUS_TYPE_BYTE, &(dip.ipbuf[12]),
							DBUS_TYPE_BYTE, &(dip.ipbuf[13]),
							DBUS_TYPE_BYTE, &(dip.ipbuf[14]),							
							DBUS_TYPE_BYTE, &(dip.ipbuf[15]),
							DBUS_TYPE_BYTE, &(sip.ipbuf[0]),
							DBUS_TYPE_BYTE, &(sip.ipbuf[1]),
							DBUS_TYPE_BYTE, &(sip.ipbuf[2]),
							DBUS_TYPE_BYTE, &(sip.ipbuf[3]),
							DBUS_TYPE_BYTE, &(sip.ipbuf[4]),
							DBUS_TYPE_BYTE, &(sip.ipbuf[5]),
							DBUS_TYPE_BYTE, &(sip.ipbuf[6]),
							DBUS_TYPE_BYTE, &(sip.ipbuf[7]),
							DBUS_TYPE_BYTE, &(sip.ipbuf[8]),
							DBUS_TYPE_BYTE, &(sip.ipbuf[9]),
							DBUS_TYPE_BYTE, &(sip.ipbuf[10]),
							DBUS_TYPE_BYTE, &(sip.ipbuf[11]),
							DBUS_TYPE_BYTE, &(sip.ipbuf[12]),
							DBUS_TYPE_BYTE, &(sip.ipbuf[13]),
							DBUS_TYPE_BYTE, &(sip.ipbuf[14]),							
							DBUS_TYPE_BYTE, &(sip.ipbuf[15]),
							DBUS_TYPE_UINT32, &dipmaskLen,
							DBUS_TYPE_UINT32, &sipmaskLen,
							DBUS_TYPE_UINT32, &dstport,
							DBUS_TYPE_UINT32, &srcport,
							DBUS_TYPE_UINT32, &policer,							
							DBUS_TYPE_UINT32, &policerId,
							DBUS_TYPE_INVALID);
  
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_acl,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{
			if (ACL_RETURN_CODE_SUCCESS == op_ret ) 
			{
				/*vty_out(vty,"Access-list %d is successfully set!\n",(ruleIndex+1));*/
			}
			else if(NPD_DBUS_ERROR_NO_SUCH_PORT ==op_ret){
				vty_out(vty,"%% Error! illegal port index\n");
			}
			else if(ACL_RETURN_CODE_GLOBAL_EXISTED == op_ret)
				vty_out(vty,"%% Access-list %d existed!\n",(ruleIndex+1));
			else if(ACL_RETURN_CODE_EXT_NO_SPACE==op_ret)
			     vty_out(vty,"%% because the extended rule should take up twice spaces than standard acl,\
				 			  but %d has been set standard rule! ~~set fail\n",(ruleIndex+513));
			else if(ACL_RETURN_CODE_SAME_FIELD == op_ret)
				vty_out(vty,"%% identical fields of packet can not set again\n");	
			else if(ACL_RETURN_CODE_POLICER_ID_NOT_SET == op_ret)
				vty_out(vty,"%% Policer %d not existed!\n",policerId);
			else 
				vty_out(vty,"%% set fail!\n");
		
	} 
	else 
	{
		/*vty_out(vty,"Failed get args.\n");*/
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS; 
}

DEFUN(config_acl_rule_deny_tcp_or_udp_func,
	  config_acl_rule_deny_tcp_or_udp_cmd,
	  "acl standard <1-1000> (permit|deny) (tcp|udp) dip (A.B.C.D/M |any) dst-port (<0-65535>|any) sip (A.B.C.D/M |any) src-port (<0-65535>|any) [policer] [<1-255>]",
	  ACL_STR
	  "standard acl rule\n"
	  "standard rule index range in 1-1000\n"
	  "ACL rule action permit\n"
	  "deny matched packets\n"
	  "match tcp packets\n"
	  "match udp packets\n"
	  "care destination ip\n"
	  "destination ip format <X:X:X:X:X:X:X:X>\n"
	  "any destination ip\n"
	  "care destination port\n"
	  "destination port range in 0-65535\n"
	  "any destination port\n"
	  "Source IP address\n"
	  "source ip format <X:X:X:X:X:X:X:X>\n"
	  "any source ip\n"
	  "Transport layer source port\n"
	  "source port range in 0-65535\n"
	  "any source port\n"
	  "Allow policing on ingress port\n"
	  "Policer Id range in 1-255\n"
)
{
	DBusMessage 	*query = NULL, *reply = NULL;
	DBusError 		err;
	unsigned int 	ruleIndex = 0;
	unsigned long 	dipno = 0, sipno = 0;
	unsigned int    sipmaskLen = 0,dipmaskLen = 0;
	unsigned int   srcport = 0,dstport = 0;
	unsigned long    actionType = 0,packetType=0;
	unsigned int     ruleType = 0,ret = 0,i = 0;
	unsigned long	op_ret = 0;
	unsigned int 	policer=0,policerId=0;
	int Val = 0;
	
	#if 0
	if((argc < 2)||(argc > 7))
	{
		vty_out(vty, "bad parameters number!\n");
		return CMD_FAILURE;
	}
	#endif
		
	ret = dcli_str2ulong((char*)argv[0],&ruleIndex);
	if (ACL_RETURN_CODE_ERROR == ret) {
		vty_out(vty,"%% Illegal rule index!\n");
		return CMD_SUCCESS;
	}
	ruleIndex = ruleIndex-1;

	ruleType = STANDARD_ACL_RULE;
	
   if(strncmp("permit",argv[1],strlen(argv[1]))==0) { actionType=0;}
    else if(strncmp("deny",argv[1],strlen(argv[1]))==0) {actionType=1;}      
   

	if(strncmp("udp",argv[2],strlen(argv[2]))==0)
        packetType=1;
    else if(strncmp("tcp",argv[2],strlen(argv[2]))==0)
        packetType=2;
    
    if(strncmp("any",argv[3],strlen(argv[3]))==0)
    	{
			dipmaskLen=0;
		  	dipno=0;
        }
	else{
		Val=ip_address_format2ulong((char**)&argv[3],&dipno,&dipmaskLen);			
		if(Val==CMD_FAILURE)  return CMD_FAILURE;
		VALUE_IP_MASK_CHECK(dipmaskLen);
	}
	

	if(strncmp("any",argv[4],strlen(argv[4]))==0) {       
        dstport = ACL_ANY_PORT;
	}
      else{
        ret= dcli_str2ulong((char*)argv[4],&dstport);
		if(ret==ACL_RETURN_CODE_ERROR)
		{
			vty_out(vty,"%% Illegal destination port!\n");
			return CMD_FAILURE;
		}

	}
	if(strncmp("any",argv[5],strlen(argv[5]))==0)
		{ 
			sipmaskLen=0;
		  	sipno=0;
		}
	else{						
		 Val=ip_address_format2ulong((char**)&argv[5],&sipno,&sipmaskLen);		
		 if(Val==CMD_FAILURE)  return CMD_FAILURE;
		 VALUE_IP_MASK_CHECK(sipmaskLen);
	}
	
    if(strncmp("any",argv[6],strlen(argv[6]))==0) {      
        srcport = ACL_ANY_PORT;
    }
    else{
        ret= dcli_str2ulong((char*)argv[6],&srcport);
		if(ret==ACL_RETURN_CODE_ERROR)
		{
			vty_out(vty,"%% Illegal source port!\n");
			return CMD_FAILURE;
		}
	}
		/*policer*/
	if((argc>7)&&(argc<10)){
		if(actionType==0){	
			if(strncmp("policer",argv[7],strlen(argv[7]))==0){
				policer = 1;
				 ret= dcli_str2ulong((char *)argv[8],&policerId);
				if(ret==ACL_RETURN_CODE_ERROR)
				{
					vty_out(vty,"%% Illegal policer ID!\n");
					return CMD_FAILURE;
				}
			}
			else{
				vty_out(vty,"%% unknown command\n");
				return CMD_FAILURE;
			}			
		}
		else if(actionType==1){
			vty_out(vty,"%% Policer not support deny operation!\n");
			return CMD_FAILURE;
		}
		
	}
	else if(argc>9){
		vty_out(vty,"%% too many param\n");
		return CMD_FAILURE;
	}
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_ACL_OBJPATH,NPD_DBUS_ACL_INTERFACE, NPD_DBUS_METHOD_CONFIG_ACL_RULE_DENY_TCP_OR_UDP);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&ruleIndex,
							DBUS_TYPE_UINT32,&ruleType,
							DBUS_TYPE_UINT32,&sipmaskLen,
							DBUS_TYPE_UINT32,&dipmaskLen,
							DBUS_TYPE_UINT32,&dipno,
							DBUS_TYPE_UINT32,&sipno,
							DBUS_TYPE_UINT32,&dstport,
							DBUS_TYPE_UINT32,&srcport,
							DBUS_TYPE_UINT32,&actionType,
							DBUS_TYPE_UINT32,&packetType,
							DBUS_TYPE_UINT32,&policer,							
							DBUS_TYPE_UINT32,&policerId,
							DBUS_TYPE_INVALID);
  
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_acl,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{
			if (ACL_RETURN_CODE_SUCCESS == op_ret ) 
			{
				/*vty_out(vty,"Access-list %d is successfully set!\n",(ruleIndex+1));*/
			}
			else if(ACL_RETURN_CODE_GLOBAL_EXISTED == op_ret)
				vty_out(vty,"%% Access-list %d existed!\n",(ruleIndex+1));
			
			else if((ACL_RETURN_CODE_EXT_NO_SPACE==op_ret)&&(ruleType == STANDARD_ACL_RULE))
		         	vty_out(vty,"%% because the extended should take up twice spaces than stdard acl,but %d has been set extended!\n",(ruleIndex-511));
			else if((ACL_RETURN_CODE_EXT_NO_SPACE==op_ret)&&(ruleType == EXTENDED_ACL_RULE))
				vty_out(vty,"%% because the extended rule should take up twice spaces than standard acl,\
				 			  but %d has been set standard rule! ~~set fail\n",(ruleIndex+513));
			else if(ACL_RETURN_CODE_SAME_FIELD == op_ret)
				vty_out(vty,"%% identical fields of packet can not set again\n");	
			else if(ACL_RETURN_CODE_POLICER_ID_NOT_SET == op_ret)
					vty_out(vty,"%% Policer %d not existed!\n",policerId);
			else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret)
				vty_out(vty,"%% Product not support this function!\n");
			else 
				vty_out(vty,"%% set fail!\n");
		
	} 
	else 
	{
		/*vty_out(vty,"Failed get args.\n");*/
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
	
}

DEFUN(config_acl_rule_mirror_redirect_arp_cmd_func,
	  config_acl_rule_mirror_redirect_arp_cmd,
	  "acl extended <1-1000> redirect PORTNO arp smac (H:H:H:H:H:H|any) vid (<1-4095>|any) sourceport (PORTNO|any) [policer] [<1-255>]",
	  ACL_STR
	  "extended acl rule\n"
	  "extended rule index range in 1-1000\n"	
	  "acl rule action redirect\n"
	  CONFIG_ETHPORT_STR 
	  "match with arp packets\n"
	  "care source mac address\n"
	  "Specify SMAC address in Hexa.Eg:00:00:11:11:aa:aa\n"
	  "any source mac\n"
	  "care vlan id \n"
	  "specify vlan id in 1~4095\n"
	  "any vlan id\n"
	  "care Interface port which packets come from \n"
	  CONFIG_ETHPORT_STR 
	  "any ports that packets coming from\n"
	  "Allow policing on ingress port\n"
	  "Policer Id range in 1-255\n"
)	
{
	DBusMessage 	*query = NULL, *reply = NULL;
  	DBusError		err;
	ETHERADDR		smacAddr,maskMac;
	unsigned char   slot_no=ACL_ANY_PORT_CHAR,port_no=ACL_ANY_PORT_CHAR;
	unsigned char   mslot=0,mport=0;
	unsigned int	ruleIndex = 0; 
	unsigned int	ruleType = 0,actionType = 0,op_ret = 0,vlanId=0,ifIndex = 0,ret = 0,eth_ret = 0,port_ret = 0;
	char *			str[20]={"ff:ff:ff:ff:ff:ff"};
	unsigned int 	policer=0,policerId=0;
	char 		strcp[50];
	
	#if 0
	if((argc < 2)||(argc > 6))
	{
		vty_out(vty, "bad parameters number!\n");
		return CMD_WARNING;
	}
	#endif

	ret = dcli_str2ulong((char*)argv[0],&ruleIndex);
	if (ACL_RETURN_CODE_ERROR == ret) {
		vty_out(vty,"%% Illegal rule index!\n");
		return CMD_WARNING;
	}
	ruleType = EXTENDED_ACL_RULE; 
	if ((ruleType == EXTENDED_ACL_RULE)&&(ruleIndex > MAX_EXT_RULE_NUM)) {
		vty_out(vty,"extended rule must less than 500!\n");
		return CMD_WARNING;
	}
	ruleIndex = ruleIndex-1; 	

	memset(&smacAddr,0,sizeof(ETHERADDR));
	memset(&maskMac,0,sizeof(ETHERADDR));

    actionType=4;
    
   /*mirror or redirect port*/
   ret = parse_slotport_no((char *)argv[1],&mslot,&mport);

	if (ACL_RETURN_CODE_SUCCESS != ret) {
    	vty_out(vty,"%% Unknow portno format.\n");
		return CMD_SUCCESS;
	}
	
	/*fetch the 1st param : SMAC addr*/
	if(strncmp("any",argv[2],strlen(argv[2]))!=0){
		op_ret = parse_mac_addr((char *)argv[2],&smacAddr);
		if (ACL_RETURN_CODE_SUCCESS != op_ret) {
	    	vty_out(vty,"%% Unknow mac addr format.\n");
			return CMD_SUCCESS;
		}
	
		/*convert mac_mask to standard format*/
		/*strcp=(char *)malloc(50*sizeof(char));*/
		memset(strcp,'\0',50);
		strcpy(strcp,str[0]);
		
		op_ret = parse_mac_addr(strcp,&maskMac);
		if (ACL_RETURN_CODE_SUCCESS != op_ret) {
	    	vty_out(vty,"%% Unknow mac addr format.\n");
			return CMD_SUCCESS;
		}
	}
    /*vlan id*/
	if(strncmp("any",argv[3],strlen(argv[3]))!=0){
		ret = dcli_str2ulong((char*)argv[3],&vlanId);
		if (ACL_RETURN_CODE_ERROR == ret) {
			vty_out(vty,"%% Illegal vlan Id!\n");
			return CMD_WARNING;
		}
	}
	/*slot/port*/
	if(strncmp("any",argv[4],strlen(argv[4]))!=0){
		ret = parse_slotport_no((char *)argv[4],&slot_no,&port_no);

		if (ACL_RETURN_CODE_SUCCESS != ret) {
	    	vty_out(vty,"%% Unknow portno format.\n");
			return CMD_SUCCESS;
		}
	}
	/*policer*/
	if((argc>5)&&(argc<8)){		
			if(strncmp("policer",argv[5],strlen(argv[5]))==0){
				policer = 1;
				ret = dcli_str2ulong((char *)argv[6],&policerId);
				if (ACL_RETURN_CODE_ERROR == ret) {
					vty_out(vty,"%% Illegal policer ID!\n");
					return CMD_WARNING;
				}
			}
			else{
				vty_out(vty,"%% Unknown command\n");
				return CMD_WARNING;
			}						
	}
	else if(argc>7){
		vty_out(vty,"%% too many param\n");
		return CMD_WARNING;
	}
	/*query*/
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_ACL_OBJPATH,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_METHOD_CONFIG_ACL_RULE_MIRROR_REDIRECT_ARP);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&ruleIndex,	
							 DBUS_TYPE_UINT32,&ruleType,
							 DBUS_TYPE_UINT32,&actionType,
							 DBUS_TYPE_BYTE,  &mslot,
							 DBUS_TYPE_BYTE,  &mport,
						     DBUS_TYPE_BYTE,  &smacAddr.arEther[0],
							 DBUS_TYPE_BYTE,  &smacAddr.arEther[1],
							 DBUS_TYPE_BYTE,  &smacAddr.arEther[2],
							 DBUS_TYPE_BYTE,  &smacAddr.arEther[3],
							 DBUS_TYPE_BYTE,  &smacAddr.arEther[4],
							 DBUS_TYPE_BYTE,  &smacAddr.arEther[5],
						     DBUS_TYPE_BYTE,  &maskMac.arEther[0],
							 DBUS_TYPE_BYTE,  &maskMac.arEther[1],
							 DBUS_TYPE_BYTE,  &maskMac.arEther[2],
							 DBUS_TYPE_BYTE,  &maskMac.arEther[3],
							 DBUS_TYPE_BYTE,  &maskMac.arEther[4],
							 DBUS_TYPE_BYTE,  &maskMac.arEther[5],	
							 DBUS_TYPE_UINT32,&vlanId,
							 DBUS_TYPE_BYTE,  &slot_no,
							 DBUS_TYPE_BYTE,  &port_no,
							 DBUS_TYPE_UINT32,&policer,							
 							 DBUS_TYPE_UINT32,&policerId,
							 DBUS_TYPE_INVALID);
  
   
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_acl,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_UINT32,&eth_ret,		
		DBUS_TYPE_INVALID))
	{

			if(ACL_RETURN_CODE_SUCCESS != eth_ret){
				vty_out(vty,"%% Error! illegal port index!\n");
			}
			else{
				if (ACL_RETURN_CODE_SUCCESS == op_ret ) 
				{
					/*vty_out(vty,"Access-list %d is successfully set!\n",(ruleIndex+1));*/
				}
				else if(ACL_RETURN_CODE_GLOBAL_EXISTED == op_ret)
					vty_out(vty,"%% Access-list %d existed!\n",(ruleIndex+1));
				
				else if((ACL_RETURN_CODE_EXT_NO_SPACE==op_ret)&&(ruleType == STANDARD_ACL_RULE))
		         		vty_out(vty,"%% because the extended should take up twice spaces than stdard acl,but %d has been set extended!\n",(ruleIndex-511));
				else if((ACL_RETURN_CODE_EXT_NO_SPACE==op_ret)&&(ruleType == EXTENDED_ACL_RULE))
					vty_out(vty,"%% because the extended rule should take up twice spaces than standard acl,\
				 			  but %d has been set standard rule! ~~set fail\n",(ruleIndex+513));
				else if(ACL_RETURN_CODE_SAME_FIELD == op_ret)
					vty_out(vty,"%% identical fields of packet can not set again\n");	
                else if(ACL_RETURN_CODE_POLICER_ID_NOT_SET == op_ret)
					vty_out(vty,"%% Policer %d not existed!\n",policerId);
				else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret)
					vty_out(vty,"%% Product not support this function!\n");
				else 
					vty_out(vty,"%% set fail!\n");
			}			
	} 
	else 
	{
		/*vty_out(vty,"Failed get args.\n");*/
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}	

DEFUN(config_acl_rule_mirror_redirect_ip_func,
	  config_acl_rule_mirror_redirect_ip_cmd,
	  "acl (standard|extended) <1-1000> redirect PORTNO ip dip (A.B.C.D/M |any) sip (A.B.C.D/M |any) [policer] [<1-255>]",
	  ACL_STR
	  "standard acl rule\n"
	  "extended acl rule\n"
	  "standard rule index range in 1-1000, extended rule index range in 1-500\n"  
	  "acl rule action redirect\n"
	  CONFIG_ETHPORT_STR 
	  "acl rule deal with IP packet\n"
	  "Destination IP address for IP packet\n"
	  "Specify IP address in <A.B.C.D/M> format\n"
	  "Any legal destination IP address\n"
	  "Source IP address for TCP OR UDP packet\n"
	  "Specify IP address in <A.B.C.D/M> format\n"
	  "Any legal source IP address\n"
	  "Allow policing on ingress port\n"
	  "Policer Id range in 1-255\n"
)
{
	DBusMessage 	*query = NULL, *reply = NULL;
	DBusError		err;
	unsigned int	ruleIndex = 0;
	unsigned long	dipno = 0, sipno = 0;
	unsigned int    sipmaskLen = 0,dipmaskLen = 0;
	unsigned  int	ruleType = 0,actionType = 0,ret = 0,i = 0,temp = 0;
	unsigned long	op_ret = 0;
	unsigned char   slot_no = 0,port_no = 0;
	unsigned int	policer=0,policerId=0;
	int Val=0;

	
	#if 0
	if((argc < 2)||(argc > 5))
	{
		vty_out(vty, "bad parameters number!\n");
		return CMD_WARNING;
	}
    #endif
	if(strncmp("standard",argv[0],strlen(argv[0]))==0)
	{
		ruleType = STANDARD_ACL_RULE;
	}
	else if (strncmp("extended",argv[0],strlen(argv[0]))==0)
	{
		ruleType = EXTENDED_ACL_RULE;
	}
	else
	{
		vty_out(vty,"bad command parameter!\n");
		return CMD_WARNING;
	}	
	ret = dcli_str2ulong((char*)argv[1],&ruleIndex);
	if (ACL_RETURN_CODE_ERROR == ret) {
		vty_out(vty,"%% Illegal rule index!\n");
		return CMD_WARNING;
	}
	if ((ruleType == EXTENDED_ACL_RULE)&&(ruleIndex > MAX_EXT_RULE_NUM)) {
		vty_out(vty,"extended rule must less than 500!\n");
		return CMD_WARNING;
	}
	ruleIndex = ruleIndex-1;		
	actionType=4;

	temp = parse_slotport_no((char*)argv[2],&slot_no,&port_no);
	
	if (ACL_RETURN_CODE_SUCCESS != ret) {
    		vty_out(vty,"%% illegal format with slot/port!\n");
		return CMD_WARNING;
	}
	

	if(strncmp("any",argv[3],strlen(argv[3]))==0)
		{
			dipmaskLen=0;
			dipno=0;
		}
	else
		{	
			Val=ip_address_format2ulong((char**)&argv[3],&dipno,&dipmaskLen);			
			if(CMD_WARNING == Val) {
				 vty_out(vty, "%% Bad parameter %s\n", argv[3]);
				 return CMD_WARNING;
			}
			VALUE_IP_MASK_CHECK(dipmaskLen);
		}
	
	
	if(strncmp("any",argv[4],strlen(argv[4]))==0)
		{ 
			sipmaskLen=0;
			sipno=0;
		}
	else {						
		 Val = ip_address_format2ulong((char**)&argv[4],&sipno,&sipmaskLen); 	
		 if(CMD_WARNING == Val) {
			  vty_out(vty, "%% Bad parameter %s\n", argv[4]);
			  return CMD_WARNING;
		 }
		 VALUE_IP_MASK_CHECK(sipmaskLen);
	}
	
	
	/*policer*/
	if((argc>5)&&(argc<8)){ 	
			if(strncmp("policer",argv[5],strlen(argv[5]))==0){
				policer = 1;
				ret = dcli_str2ulong((char *)argv[6],&policerId);
				if (ACL_RETURN_CODE_ERROR == ret) {
					vty_out(vty,"%% Illegal policer ID!\n");
					return CMD_WARNING;
				}
			}
			else{
				vty_out(vty,"%% unknown command\n");
				return CMD_WARNING;
			}						
	}
	else if(argc>7){
		vty_out(vty,"%% too many param\n");
		return CMD_WARNING;
	}

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_ACL_OBJPATH, \
					NPD_DBUS_ACL_INTERFACE, NPD_DBUS_METHOD_CONFIG_ACL_RULE_MIRROR_OR_REDIRECT_IP);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&ruleIndex,				  
							 DBUS_TYPE_UINT32,&ruleType,
							 DBUS_TYPE_UINT32,&actionType,
							 DBUS_TYPE_UINT32,&sipmaskLen,
							 DBUS_TYPE_UINT32,&dipmaskLen,
							 DBUS_TYPE_UINT32,&dipno,
							 DBUS_TYPE_UINT32,&sipno,
							 DBUS_TYPE_BYTE  ,&slot_no,
							 DBUS_TYPE_BYTE  ,&port_no,
							 DBUS_TYPE_UINT32,&policer,							
 							 DBUS_TYPE_UINT32,&policerId,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_acl,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{
			if (ACL_RETURN_CODE_SUCCESS == op_ret ) 
			{
				/*vty_out(vty,"Access-list %d is successfully set!\n",(ruleIndex+1));*/
			}	
			else if(NPD_DBUS_ERROR_NO_SUCH_PORT==op_ret){
				vty_out(vty,"%% Error ! illegal port index\n");
			}
			else if(ACL_RETURN_CODE_GLOBAL_EXISTED == op_ret)
				vty_out(vty,"%% Access-list %d existed!\n",(ruleIndex+1));
			
			else if((ACL_RETURN_CODE_EXT_NO_SPACE==op_ret)&&(ruleType == STANDARD_ACL_RULE))
		         	vty_out(vty,"%% because the extended should take up twice spaces than stdard acl,but %d has been set extended!\n",(ruleIndex-511));
			else if((ACL_RETURN_CODE_EXT_NO_SPACE==op_ret)&&(ruleType == EXTENDED_ACL_RULE))
				vty_out(vty,"%% because the extended rule should take up twice spaces than standard acl,\
				 			  but %d has been set standard rule! ~~set fail\n",(ruleIndex+513));

			else if(ACL_RETURN_CODE_SAME_FIELD == op_ret)
				vty_out(vty,"%% identical fields of packet can not set again\n");	
			else if(ACL_RETURN_CODE_POLICER_ID_NOT_SET == op_ret)
					vty_out(vty,"%% Policer %d not existed!\n",policerId);
			else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret)
					vty_out(vty,"%% Product not support this function!\n");
			else 
				vty_out(vty,"%% set fail!\n");
		
	} 
	else 
	{
		/*vty_out(vty,"Failed get args.\n");*/
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
	
}

DEFUN(config_acl_rule_mirror_redirect_tcp_or_udp_func,
	  config_acl_rule_mirror_redirect_tcp_or_udp_cmd,
	  "acl standard <1-1000> redirect PORTNO (tcp|udp) dip (A.B.C.D/M |any) dst-port (<0-65535>|any) sip (A.B.C.D/M |any) src-port (<0-65535>|any) [policer] [<1-255>]",
	  ACL_STR
	  "standard acl rule\n"
	  "standard rule index range in 1-1000\n"
	  "acl rule action redirect\n"
	  CONFIG_ETHPORT_STR 
	  "acl rule deal with TCP packet\n"
	  "acl rule deal with UDP packet\n"
      "Destination IP address for TCP OR UDP packet\n"
	  "Specify IP address in <A.B.C.D/M> format\n"
	  "Any legal destination IP address\n"
	  "Destination port with TCP OR UDP packet\n"
	  "Specify Port number in range 0-65535\n"
	  "Any legal destination port number\n"
      "Source IP address for TCP OR UDP packet\n"
	  "Specify IP address in <A.B.C.D/M> format\n"
	  "Any legal source IP address\n"
	  "Source port with TCP OR UDP packet\n"
	  "Specify Port number in range 0-65535\n"
	  "Any legal source port number\n"	
	  "Allow policing on ingress port\n"
	  "Policer Id range in 1-255\n"
)
{
	DBusMessage 	*query = NULL, *reply = NULL;
	DBusError 		err;
	unsigned int 	ruleIndex = 0;
	unsigned long 	dipno = 0, sipno = 0;
	unsigned int   srcport = 0,dstport = 0,sipmaskLen = 0,dipmaskLen = 0;
	unsigned long    actionType = 0,packetType=0;
	unsigned int     ruleType = 0,ret = 0,i = 0,temp = 0;
	unsigned long	op_ret = 0;
	unsigned char   slot_no = 0,port_no = 0;	
	unsigned int	policer=0,policerId=0;
	int Val =0;

	ret = dcli_str2ulong((char*)argv[0],&ruleIndex);
	if (ACL_RETURN_CODE_ERROR == ret) {
		vty_out(vty,"%% Illegal rule index!\n");
		return CMD_FAILURE;
	}
	ruleIndex = ruleIndex-1;
				
	ruleType = STANDARD_ACL_RULE;
	actionType=4;

	temp = parse_slotport_no((char*)argv[1],&slot_no,&port_no);
	
	if (ACL_RETURN_CODE_SUCCESS != ret) {
    	vty_out(vty,"%% Illegal format with slot/port!\n");
		return CMD_FAILURE;
	}
	
	if(strncmp("udp",argv[2],strlen(argv[2]))==0)
        packetType=1;
    else if(strncmp("tcp",argv[2],strlen(argv[2]))==0)
        packetType=2;

	if(strncmp("any",argv[3],strlen(argv[3]))==0)
    	{
			dipmaskLen=0;
		  	dipno=0;
    }
	else{	
			Val=ip_address_format2ulong((char**)&argv[3],&dipno,&dipmaskLen);			
			if(Val==CMD_FAILURE)  return CMD_FAILURE;
			VALUE_IP_MASK_CHECK(dipmaskLen);

	}
		
    if(strncmp("any",argv[4],strlen(argv[4]))==0) {     
        dstport = ACL_ANY_PORT;
    }
    else{
       ret = dcli_str2ulong((char*)argv[4],&dstport);
	   if (ACL_RETURN_CODE_ERROR == ret) {
			  vty_out(vty,"%% Illegal destination port!\n");
			  return CMD_FAILURE;
		  }

	}
	if(strncmp("any",argv[5],strlen(argv[5]))==0)
		{ 
			sipmaskLen=0;
		  	sipno=0;
		}
	else
		{						
		  Val=ip_address_format2ulong((char**)&argv[5],&sipno,&sipmaskLen);		
			if(Val==CMD_FAILURE)  return CMD_FAILURE;
			VALUE_IP_MASK_CHECK(sipmaskLen);
	}
   
    
    if(strncmp("any",argv[6],strlen(argv[6]))==0) {       
        srcport = ACL_ANY_PORT;
    }
    else{
       ret = dcli_str2ulong((char*)argv[6],&srcport);
	   if (ACL_RETURN_CODE_ERROR == ret) {
		   vty_out(vty,"%% Illegal source port!\n");
		   return CMD_FAILURE;
	   }

	}
	/*policer*/
	if((argc>7)&&(argc<10)){		
		if(strncmp("policer",argv[7],strlen(argv[7]))==0){
			policer = 1;
			ret = dcli_str2ulong((char *)argv[8],&policerId);
			if (ACL_RETURN_CODE_ERROR == ret) {
				vty_out(vty,"%% Illegal policer ID!\n");
				return CMD_FAILURE;
			}
		}
		else{
			vty_out(vty,"%% unknown command\n");
			return CMD_FAILURE;
		}						
	}
	else if(argc>9){
		vty_out(vty,"%% too many param\n");
		return CMD_FAILURE;
	}
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_ACL_OBJPATH,NPD_DBUS_ACL_INTERFACE, NPD_DBUS_METHOD_CONFIG_ACL_RULE_MIRROR_OR_REDIRECT_TCP_UDP);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&ruleIndex,
							DBUS_TYPE_UINT32,&ruleType,
							DBUS_TYPE_UINT32,&sipmaskLen,
							DBUS_TYPE_UINT32,&dipmaskLen,
							DBUS_TYPE_UINT32,&dipno,
							DBUS_TYPE_UINT32,&sipno,
							DBUS_TYPE_UINT32,&dstport,
							DBUS_TYPE_UINT32,&srcport,
							DBUS_TYPE_UINT32,&actionType,
							DBUS_TYPE_UINT32,&packetType,
							DBUS_TYPE_BYTE  ,&slot_no,
							DBUS_TYPE_BYTE  ,&port_no,	
							DBUS_TYPE_UINT32,&policer,							
							DBUS_TYPE_UINT32,&policerId,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_acl,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			vty_out(vty,"%%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{
			if (ACL_RETURN_CODE_SUCCESS == op_ret ) 
			{
				/*vty_out(vty,"Access-list %d is successfully set!\n",(ruleIndex+1));*/
			}	
			else if(NPD_DBUS_ERROR_NO_SUCH_PORT==op_ret){
				vty_out(vty,"%% Error ! illegal port index\n");
			}
			else if(ACL_RETURN_CODE_GLOBAL_EXISTED == op_ret)
				vty_out(vty,"%% Access-list %d existed!\n",(ruleIndex+1));
			
			else if((ACL_RETURN_CODE_EXT_NO_SPACE==op_ret)&&(ruleType == STANDARD_ACL_RULE))
		         	vty_out(vty,"%% because the extended should take up twice spaces than stdard acl,but %d has been set extended!\n",(ruleIndex-511));
			else if((ACL_RETURN_CODE_EXT_NO_SPACE==op_ret)&&(ruleType == EXTENDED_ACL_RULE))
				vty_out(vty,"%% because the extended rule should take up twice spaces than standard acl,\
				 			  but %d has been set standard rule! ~~set fail\n",(ruleIndex+513));

			else if(ACL_RETURN_CODE_SAME_FIELD == op_ret)
				vty_out(vty,"%% identical fields of packet can not set again\n");	
			else if(ACL_RETURN_CODE_POLICER_ID_NOT_SET == op_ret)
					vty_out(vty,"%% Policer %d not existed!\n",policerId);
			else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret)
				vty_out(vty,"%% Product not support this function!\n");
			else 
				vty_out(vty,"%% set fail!\n");	
		
	} 
	else 
	{
		/*vty_out(vty,"Failed get args.\n");*/
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
	
}
	
DEFUN(config_acl_rule_mirror_redirect_icmp_cmd_func,
	  config_acl_rule_mirror_redirect_icmp_cmd,
	  "acl (standard|extended) <1-1000> redirect PORTNO icmp dip (A.B.C.D/M |any) sip (A.B.C.D/M |any) type (<0-255>|any) code (<0-255>|any) [policer] [<1-255>]",
	  ACL_STR
	  "standard acl rule\n"
	  "extended acl rule\n"
	  "standard rule index range in 1-1000, extended rule index range in 1-500\n" 
	  "acl rule action redirect\n"
	  CONFIG_ETHPORT_STR 
	  "acl rule deal with ICMP packet\n"
	  "Destination IP address for ICMP packet\n"
	  "Specify IP address in <A.B.C.D/M> format\n"
	  "Any legal destination IP address\n"
	  "Source IP address for ICMP packet\n"
	  "Specify IP address in <A.B.C.D/M> format\n"
	  "Any legal source IP address\n"
	  "Icmp type with ICMP packet\n"
	  "Specify type number in range 0-255\n"
	  "Any legal type number\n"
	  "Icmp code with ICMP packet\n"
	  "Specifycode number in range 0-255\n"
	  "Any legal code number\n"	
	  "Allow policing on ingress port\n"
	  "Policer Id range in 1-255\n"
)
{
	DBusMessage 	*query = NULL, *reply = NULL;
	DBusError		err;
	unsigned int	ruleIndex = 0,sipmaskLen = 0,dipmaskLen = 0;
	unsigned long	 dipno = 0, sipno = 0;
	unsigned int 	ruleType = 0,ret = 0,i = 0,temp = 0,actionType = 0;
	unsigned long	op_ret = 0;
	unsigned char   slot_no = 0,port_no = 0;
	unsigned char    typeno=0,codeno=0,acl_any=0;
	unsigned int     tmp1=0,tmp2=0;
	unsigned int	policer=0,policerId=0;
	int Val=0;
	
	if(strncmp("standard",argv[0],strlen(argv[0]))==0)
	{
		ruleType = STANDARD_ACL_RULE;
	}
	else if (strncmp("extended",argv[0],strlen(argv[0]))==0)
	{
		ruleType = EXTENDED_ACL_RULE;
	}
	else
	{
		vty_out(vty,"bad command parameter!\n");
		return CMD_WARNING;
	}
	ret = dcli_str2ulong((char*)argv[1],&ruleIndex);
	if (ACL_RETURN_CODE_ERROR == ret) {
		vty_out(vty,"%% Illegal rule index!\n");
		return CMD_WARNING;
	}
	if ((ruleType == EXTENDED_ACL_RULE)&&(ruleIndex > MAX_EXT_RULE_NUM)) {
		vty_out(vty,"extended rule must less than 500!\n");
		return CMD_WARNING;
	}
	ruleIndex = ruleIndex-1;

	actionType=4;

	temp = parse_slotport_no((char*)argv[2],&slot_no,&port_no);
	
	if (ACL_RETURN_CODE_SUCCESS != ret) {
    	vty_out(vty,"%%  illegal format with slot/port!\n");
		return CMD_WARNING;
	}

	if(strncmp("any",argv[3],strlen(argv[3]))==0)
	{
		dipmaskLen=0;
		dipno=0;
	}
	else
	{	
		Val=ip_address_format2ulong((char**)&argv[3],&dipno,&dipmaskLen);
		if(CMD_WARNING == Val) {
			 vty_out(vty, "%% Bad parameter %s\n", argv[3]);
			 return CMD_WARNING;
		}
		VALUE_IP_MASK_CHECK(dipmaskLen);
	}
	
	
	if(strncmp("any",argv[4],strlen(argv[4]))==0)
	{ 
		sipmaskLen=0;
		sipno=0;
	}
	else
	{						
	  Val=ip_address_format2ulong((char**)&argv[4],&sipno,&sipmaskLen); 	
	  if(CMD_WARNING == Val) {
		   vty_out(vty, "%% Bad parameter %s\n", argv[4]);
		   return CMD_WARNING;
	  }
	   VALUE_IP_MASK_CHECK(sipmaskLen); 
	}
	
	
	if(strncmp("any",argv[5],strlen(argv[5]))==0) {	   
		typeno=0;
		acl_any |= 1;
	}
	else{
		ret= dcli_str2ulong((char*)argv[5],&tmp1);
		if (ACL_RETURN_CODE_ERROR == ret) {
			vty_out(vty,"%% Illegal icmp type!\n");
			return CMD_WARNING;
		}
		typeno = tmp1;
	}
	if(strncmp("any",argv[6],strlen(argv[6]))==0) {	   
		codeno=0;
		acl_any |= 2;
	}
	else{
		ret= dcli_str2ulong((char*)argv[6],&tmp2);
		if (ACL_RETURN_CODE_ERROR == ret) {
			vty_out(vty,"%% Illegal icmp code!\n");
			return CMD_WARNING;
		}
		codeno =tmp2;
	}

	/*policer*/
	if((argc>7)&&(argc<10)){		
			if(strncmp("policer",argv[7],strlen(argv[7]))==0){
				policer = 1;
				
				ret= dcli_str2ulong((char *)argv[8],&policerId);
				 if (ACL_RETURN_CODE_ERROR == ret) {
					vty_out(vty,"%% Illegal policer ID!\n");
					return CMD_WARNING;
				}
			}
			else{
				vty_out(vty,"unknown command\n");
				return CMD_WARNING;
			}						
	}
	else if(argc>9){
		vty_out(vty,"too many param\n");
		return CMD_WARNING;
	}
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_ACL_OBJPATH,NPD_DBUS_ACL_INTERFACE, NPD_DBUS_METHOD_CONFIG_ACL_RULE_MIRROR_OR_REDIRECT_ICMP);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&ruleIndex,	
							DBUS_TYPE_UINT32,&ruleType,
							DBUS_TYPE_UINT32,&actionType,
							DBUS_TYPE_UINT32,&sipmaskLen,
							DBUS_TYPE_UINT32,&dipmaskLen,
							DBUS_TYPE_UINT32,&dipno,
							DBUS_TYPE_UINT32,&sipno,
							DBUS_TYPE_BYTE,&typeno,
							DBUS_TYPE_BYTE,&codeno,
							DBUS_TYPE_BYTE,&acl_any,
							DBUS_TYPE_BYTE  ,&slot_no,
							DBUS_TYPE_BYTE  ,&port_no,
							DBUS_TYPE_UINT32,&policer,							
							DBUS_TYPE_UINT32,&policerId,
							DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_acl,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{
			if (ACL_RETURN_CODE_SUCCESS == op_ret ) 
			{
				/*vty_out(vty,"Access-list %d is successfully set!\n",(ruleIndex+1));*/
			}	
			else if(NPD_DBUS_ERROR_NO_SUCH_GROUP==op_ret){
				vty_out(vty,"%%  Error, illegal port index\n");
			}
			else if(ACL_RETURN_CODE_GLOBAL_EXISTED == op_ret)
				vty_out(vty,"%%  Access-list %d existed!\n",(ruleIndex+1));
			
			else if((ACL_RETURN_CODE_EXT_NO_SPACE==op_ret)&&(ruleType == STANDARD_ACL_RULE))
		         	vty_out(vty,"%% because the extended should take up twice spaces than stdard acl,but %d has been set extended!\n",(ruleIndex-511));
			else if((ACL_RETURN_CODE_EXT_NO_SPACE==op_ret)&&(ruleType == EXTENDED_ACL_RULE))
				vty_out(vty,"%% because the extended rule should take up twice spaces than standard acl,\
				 			  but %d has been set standard rule! ~~set fail\n",(ruleIndex+513));
			else if(ACL_RETURN_CODE_SAME_FIELD == op_ret)
				vty_out(vty,"%%  identical fields of packet can not set again\n");	
			else if(ACL_RETURN_CODE_POLICER_ID_NOT_SET == op_ret)
				vty_out(vty,"%% Policer %d not existed!\n",policerId);
			else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret)
				vty_out(vty,"%% Product not support this function!\n");
			else 
				vty_out(vty,"%%  set fail!\n");
	} 
	else 
	{
		/*vty_out(vty,"Failed get args.\n");*/
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;	
}

DEFUN(config_acl_rule_mirror_redirect_mac_func,
	  config_acl_rule_mirror_redirect_mac_cmd,
	  "acl extended <1-1000> redirect PORTNO ethernet dmac (H:H:H:H:H:H|any) smac (H:H:H:H:H:H|any) [policer] [<1-255>]",
	  ACL_STR
	  "extended acl rule\n"
	  "extended rule index range in 1-1000\n"	 
	  "acl rule action redirect\n"
	  CONFIG_ETHPORT_STR 
	  "match with ethernet packets\n"
	  "care destination mac address\n"
	  "Specify DMAC address in Hexa.Eg:00:00:11:11:aa:aa\n"
	  "care source mac address \n"
	  "Specify SMAC address in Hexa.Eg:00:00:11:11:aa:aa\n"
	  "Allow policing on ingress port\n"
	  "Policer Id range in 1-255\n"
)
{
	DBusMessage 	*query = NULL, *reply = NULL;
	DBusError		err;
	ETHERADDR		dmacAddr,smacAddr;
	unsigned int	ruleIndex = 0,packetType = 0,actionType = 0; 
	unsigned int	ruleType = 0,op_ret = 0,temp = 0;
	char *			str[20]={"ff:ff:ff:ff:ff:ff"};
	char *			strcp = NULL;
	unsigned char   slot_no=0,port_no=0;
	unsigned int 	policer=0,policerId=0,ethertype = 0xffff;
	int ret = 0;
	
	ret = dcli_str2ulong((char*)argv[0],&ruleIndex);
	if (ACL_RETURN_CODE_ERROR == ret) {
		vty_out(vty,"%% Illegal rule index!\n");
		return CMD_WARNING;
	}
	ruleType = EXTENDED_ACL_RULE; 
	if ((ruleType == EXTENDED_ACL_RULE)&&(ruleIndex > MAX_EXT_RULE_NUM)) {
		vty_out(vty,"extended rule must less than 500!\n");
		return CMD_WARNING;
	}
	ruleIndex = ruleIndex-1; 

	actionType=4;

	temp = parse_slotport_no((char*)argv[1],&slot_no,&port_no);
	
	if (ACL_RETURN_CODE_SUCCESS != ret) {
    	vty_out(vty,"%%  Illegal format with slot/port!\n");
		return CMD_WARNING;
	}
	memset(&smacAddr,0,sizeof(ETHERADDR));
	memset(&dmacAddr,0,sizeof(ETHERADDR));
	/*memset(&maskMac,0,sizeof(ETHERADDR));*/

	/*fetch the 1st param : DMAC addr*/
	if(strncmp("any",argv[2],strlen(argv[2]))!=0){
		op_ret = parse_mac_addr((char *)argv[2],&dmacAddr);
		if (ACL_RETURN_CODE_SUCCESS != op_ret) {
			vty_out(vty,"%% Unknown mac addr format.\n");
			return CMD_SUCCESS;
		}
	}
	/*fetch the 1st param : SMAC addr*/
	if(strncmp("any",argv[3],strlen(argv[3]))!=0){
		op_ret = parse_mac_addr((char *)argv[3],&smacAddr);
		if (ACL_RETURN_CODE_SUCCESS != op_ret) {
			vty_out(vty,"%% Unknown mac addr format.\n");
			return CMD_SUCCESS;
		}
	}
	/*policer*/
	if((argc>4)&&(argc<7)){
			if(strncmp("policer",argv[4],strlen(argv[4]))==0){
				policer = 1;
				 ret= dcli_str2ulong((char *)argv[5],&policerId);
				 if (ACL_RETURN_CODE_ERROR == ret) {
					vty_out(vty,"%% Illegal policer ID!\n");
					return CMD_WARNING;
				}
			}
			else{
				vty_out(vty,"%% Unknown command\n");
				return CMD_WARNING;
		}						
	}
	else if(argc>6){
		vty_out(vty,"%% Too many param\n");
		return CMD_WARNING;
	}
	/*query*/
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_ACL_OBJPATH,NPD_DBUS_ACL_INTERFACE, NPD_DBUS_METHOD_CONFIG_ACL_RULE_MIRROR_OR_REDIRECT_MAC);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&ruleIndex,	
							 DBUS_TYPE_UINT32,&ruleType,
							 DBUS_TYPE_UINT32,&actionType,
							 DBUS_TYPE_UINT32,&ethertype,
							 DBUS_TYPE_BYTE,  &dmacAddr.arEther[0],
							 DBUS_TYPE_BYTE,  &dmacAddr.arEther[1],
							 DBUS_TYPE_BYTE,  &dmacAddr.arEther[2],
							 DBUS_TYPE_BYTE,  &dmacAddr.arEther[3],
							 DBUS_TYPE_BYTE,  &dmacAddr.arEther[4],
							 DBUS_TYPE_BYTE,  &dmacAddr.arEther[5],
							 DBUS_TYPE_BYTE,  &smacAddr.arEther[0],
							 DBUS_TYPE_BYTE,  &smacAddr.arEther[1],
							 DBUS_TYPE_BYTE,  &smacAddr.arEther[2],
							 DBUS_TYPE_BYTE,  &smacAddr.arEther[3],
							 DBUS_TYPE_BYTE,  &smacAddr.arEther[4],
							 DBUS_TYPE_BYTE,  &smacAddr.arEther[5],
							 DBUS_TYPE_BYTE,  &slot_no,
							 DBUS_TYPE_BYTE,  &port_no,
							 DBUS_TYPE_UINT32,&policer,							
 							 DBUS_TYPE_UINT32,&policerId,
							 DBUS_TYPE_INVALID);
 
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_acl,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{
			if (ACL_RETURN_CODE_SUCCESS == op_ret ) 
			{
				/*vty_out(vty,"Access-list %d is successfully set!\n",(ruleIndex+1));*/
			}
			else if(NPD_DBUS_ERROR_NO_SUCH_PORT==op_ret)
				vty_out(vty,"%%  illegal port index\n");
			else if(ACL_RETURN_CODE_GLOBAL_EXISTED == op_ret)
				vty_out(vty,"%%  Access-list %d existed!\n",(ruleIndex+1));
			else if((ACL_RETURN_CODE_EXT_NO_SPACE==op_ret)&&(ruleType == STANDARD_ACL_RULE))
		         	vty_out(vty,"%% because the extended should take up twice spaces than stdard acl,but %d has been set extended!\n",(ruleIndex-511));
			else if((ACL_RETURN_CODE_EXT_NO_SPACE==op_ret)&&(ruleType == EXTENDED_ACL_RULE))
				vty_out(vty,"%% because the extended rule should take up twice spaces than standard acl,\
				 			  but %d has been set standard rule! ~~set fail\n",(ruleIndex+513));
			else if(ACL_RETURN_CODE_SAME_FIELD == op_ret)
				vty_out(vty,"%%  identical fields of packet can not set again\n");	
			else if(ACL_RETURN_CODE_POLICER_ID_NOT_SET == op_ret)
				vty_out(vty,"%% Policer %d not existed!\n",policerId);
			else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret)
				vty_out(vty,"%% Product not support this function!\n");
			else 
				vty_out(vty,"%%  set fail!\n");
	} 
	else 
	{
		/*vty_out(vty,"Failed get args.\n");*/
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
DEFUN(config_extend_acl_rule_trap_permit_deny_cmd_func,
	  config_extend_acl_rule_trap_permit_deny_cmd,
	  "acl extended <1-1000> (permit|deny|trap) (tcp|udp) \
	  dip (A.B.C.D/M|any) dst-port (<0-65535>|any) sip (A.B.C.D/M|any ) \
	  src-port (<0-65535>|any ) dmac (H:H:H:H:H:H|any) smac (H:H:H:H:H:H|any)\
	  vid (<1-4095>|any) sourceport (PORTNO|any) [policer] [<1-255>]",
	  ACL_STR
	  "extended acl rule\n"
	  "extended acl rule index range in 1-500\n"
	  "permit matched packets\n"
	  "deny matched packets\n"
	  "trap to cpu matched packets\n"
	  "match tcp packets\n"
	  "match udp packets\n"
	  "care destination ip\n"
	  "destination ip format A.B.C.D/M\n"
	  "any destination ip\n"
	  "care destination port\n"
	  "destination port range in 0-65535\n"
	  "any destination port\n"
	  "care source ip\n"
	  "source ip format A.B.C.d/M\n"
	  "any source ip\n"
	  "care source port\n"
	  "source port range in 0-65535\n"
	  "any source port\n"
	  "care destination mac\n"
	  "destination mac format H:H:H:H:H:H\n"
	  "any destination mac\n"
	  "care source mac\n"
	  "source mac format H:H:H:H:H:H\n"
	  "any source mac \n"
	  "care vlan id\n"
	  "vlan id range in 1-4095\n"
	  "any vlan\n"
	  "care source port which packets comes from\n"
	  CONFIG_ETHPORT_STR 
	  "any port that packets come from\n"
	  "Allow policing on ingress port\n"
	  "Policer Id range in 1-255\n"
	  
)
{	  
	DBusMessage 	*query = NULL, *reply = NULL;
	DBusError		err;
	ETHERADDR		dmacAddr,smacAddr;
	unsigned int	ruleIndex = 0,dipmaskLen=0,sipmaskLen=0;
	unsigned int	actionType = 0,packetType = 0,srcport=0,dstport=0,vlanId=0;
	unsigned char   dataslot = ACL_ANY_PORT_CHAR,dataport = ACL_ANY_PORT_CHAR;
	unsigned int     ruleType = 0,op_ret = 0,ret = 0;
	unsigned long   dipno=0,sipno=0;
	char *			strcp = NULL;
	char *			str[20]={"ff:ff:ff:ff:ff:ff"};
	unsigned int	policer=0,policerId=0;
	int Val=0;
	#if 0
	if((argc < 2)||(argc > 11))
	{
		vty_out(vty, "bad parameters number!\n");
		return CMD_WARNING;
	}
	#endif
		
	ret = dcli_str2ulong((char*)argv[0],&ruleIndex);
	if (ACL_RETURN_CODE_ERROR == ret) {
		vty_out(vty,"%% Illegal rule index!\n");
		return CMD_WARNING;
	}
	ruleType = EXTENDED_ACL_RULE; 
	if ((ruleType == EXTENDED_ACL_RULE)&&(ruleIndex > MAX_EXT_RULE_NUM)) {
		vty_out(vty,"extended rule must less than 500!\n");
		return CMD_WARNING;
	}
	ruleIndex = ruleIndex-1;
	
	if(strncmp("permit",argv[1],strlen(argv[1]))==0)
		actionType=0;
	else if(strncmp("deny",argv[1],strlen(argv[1]))==0)
		actionType=1;
	else if(strncmp("trap",argv[1],strlen(argv[1]))==0)
		actionType=2;
	
	if(strncmp("udp",argv[2],strlen(argv[2]))==0)
		packetType=1;
	else if(strncmp("tcp",argv[2],strlen(argv[2]))==0)
		packetType=2;
	
	if(strncmp("any",argv[3],strlen(argv[3]))==0)
	{
		dipmaskLen=0;
		dipno=0;
	}
	else{
		Val=ip_address_format2ulong((char**)&argv[3],&dipno,&dipmaskLen);			
		if(CMD_WARNING == Val) {
			 vty_out(vty, "%% Bad parameter %s\n", argv[3]);
			 return CMD_WARNING;
		}
		VALUE_IP_MASK_CHECK(dipmaskLen);
	}
	

	if(strncmp("any",argv[4],strlen(argv[4]))==0)	{   
		dstport = ACL_ANY_PORT;
	}
	else{
		ret= dcli_str2ulong((char*)argv[4],&dstport);
		if(ret==ACL_RETURN_CODE_ERROR)
		{
			vty_out(vty,"%% Illegal destination port!\n");
			return CMD_WARNING;
		}

	}
	if(strncmp("any",argv[5],strlen(argv[5]))==0)
	{ 
		sipmaskLen=0;
		sipno=0;
	}
	else{						
		Val=ip_address_format2ulong((char**)&argv[5],&sipno,&sipmaskLen); 	
		if(CMD_WARNING == Val) {
			 vty_out(vty, "%% Bad parameter %s\n", argv[5]);
			 return CMD_WARNING;
		}
		VALUE_IP_MASK_CHECK(sipmaskLen);
	}
	
	
	if(strncmp("any",argv[6],strlen(argv[6]))==0) {	   
		srcport = ACL_ANY_PORT;
	}
	else{
		ret= dcli_str2ulong((char*)argv[6],&srcport);
		if(ret==ACL_RETURN_CODE_ERROR)
		{
			vty_out(vty,"%% Illegal source port!\n");
			return CMD_WARNING;
		}
	}
	memset(&smacAddr,0,sizeof(ETHERADDR));
	memset(&dmacAddr,0,sizeof(ETHERADDR));
	/*memset(&maskMac,0,sizeof(ETHERADDR));*/
	/*dmac*/
	if(strncmp("any",argv[7],strlen(argv[7]))!=0){
		op_ret = parse_mac_addr((char *)argv[7],&dmacAddr);
		if (ACL_RETURN_CODE_SUCCESS != op_ret) {
			vty_out(vty,"%% Unknown mac addr format.\n");
			return CMD_WARNING;
		}
	}
	/*smac*/
	if(strncmp("any",argv[8],strlen(argv[8]))!=0){
		op_ret = parse_mac_addr((char *)argv[8],&smacAddr);
		if (ACL_RETURN_CODE_SUCCESS != op_ret) {
			vty_out(vty,"%% Unknown mac addr format.\n");
			return CMD_WARNING;
		}
	}
		
	if(strncmp("any",argv[9],strlen(argv[9]))!=0)
		ret=dcli_str2ulong((char*)argv[9],&vlanId);
		if(ret==ACL_RETURN_CODE_ERROR)
		{
			vty_out(vty,"%% Illegal vlan Id!\n");
			return CMD_WARNING;
		}
	if(strncmp("any",argv[10],strlen(argv[10]))!=0){
		/*dataslot/dataport*/
		ret = parse_slotport_no((char *)argv[10],&dataslot,&dataport);

		if (ACL_RETURN_CODE_SUCCESS != ret) {
			vty_out(vty,"%% Unknown portno format.\n");
			return CMD_WARNING;
		}
	}
	/*policer*/
	if((argc>11)&&(argc<14)){
		if(actionType==0){	
			if(strncmp("policer",argv[11],strlen(argv[11]))==0){
				policer = 1;
				ret = dcli_str2ulong((char *)argv[12],&policerId);
				if(ret==ACL_RETURN_CODE_ERROR)
				{
					vty_out(vty,"%% Illegal policer ID!\n");
					return CMD_WARNING;
				}
			}
			else{
				vty_out(vty,"unknown command\n");
				return CMD_WARNING;
			}			
		}
		else if(actionType==1){
			vty_out(vty,"%% Policer not support deny operation!\n");
			 return CMD_WARNING;
		}
		else if(actionType==2){
			vty_out(vty,"%% Policer not support trap operation!\n");
			 return CMD_WARNING;
		}
	}
	else if(argc>13){
		vty_out(vty,"too many param\n");
		return CMD_WARNING;
	}	
		
	/*query*/
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_ACL_OBJPATH,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_METHOD_CONFIG_ACL_RULE_EXTENDED_PERMIT_DENY_TRAP_TCP_UDP);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&ruleIndex,	
							 DBUS_TYPE_UINT32,&ruleType,
							 DBUS_TYPE_UINT32,&actionType,
							 DBUS_TYPE_UINT32,&packetType,
							 DBUS_TYPE_UINT32,&dipno,
							 DBUS_TYPE_UINT32,&sipno,
							 DBUS_TYPE_UINT32,&dipmaskLen,
							 DBUS_TYPE_UINT32,&sipmaskLen,
							 DBUS_TYPE_UINT32,&dstport,
							 DBUS_TYPE_UINT32,&srcport,
							 DBUS_TYPE_BYTE,  &dmacAddr.arEther[0],
							 DBUS_TYPE_BYTE,  &dmacAddr.arEther[1],
							 DBUS_TYPE_BYTE,  &dmacAddr.arEther[2],
							 DBUS_TYPE_BYTE,  &dmacAddr.arEther[3],
							 DBUS_TYPE_BYTE,  &dmacAddr.arEther[4],
							 DBUS_TYPE_BYTE,  &dmacAddr.arEther[5],
							 DBUS_TYPE_BYTE,  &smacAddr.arEther[0],
							 DBUS_TYPE_BYTE,  &smacAddr.arEther[1],
							 DBUS_TYPE_BYTE,  &smacAddr.arEther[2],
							 DBUS_TYPE_BYTE,  &smacAddr.arEther[3],
							 DBUS_TYPE_BYTE,  &smacAddr.arEther[4],
							 DBUS_TYPE_BYTE,  &smacAddr.arEther[5],	
							 DBUS_TYPE_UINT32,&vlanId,
							 DBUS_TYPE_BYTE,  &dataslot,
							 DBUS_TYPE_BYTE,  &dataport,
							 DBUS_TYPE_UINT32,&policer,							
 							 DBUS_TYPE_UINT32,&policerId,
							 DBUS_TYPE_INVALID);
  
   
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_acl,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{
			if (ACL_RETURN_CODE_SUCCESS == op_ret ) 
			{
				/*vty_out(vty,"Access-list %d is successfully set!\n",(ruleIndex+1));*/
			}
			else if(NPD_DBUS_ERROR_NO_SUCH_PORT ==op_ret){
				vty_out(vty,"%% Error! illegal port index\n");
			}
			else if(ACL_RETURN_CODE_GLOBAL_EXISTED == op_ret)
				vty_out(vty,"%% Access-list %d existed!\n",(ruleIndex+1));
			else if(ACL_RETURN_CODE_EXT_NO_SPACE==op_ret)
			     vty_out(vty,"%% because the extended rule should take up twice spaces than standard acl,\
				 			  but %d has been set standard rule! ~~set fail\n",(ruleIndex+513));
			else if(ACL_RETURN_CODE_SAME_FIELD == op_ret)
				vty_out(vty,"%% identical fields of packet can not set again\n");	
			else if(ACL_RETURN_CODE_POLICER_ID_NOT_SET == op_ret)
				vty_out(vty,"%% Policer %d not existed!\n",policerId);
			else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret)
				vty_out(vty,"%% Product not support this function!\n");
			else 
				vty_out(vty,"%% set fail!\n");
		
	} 
	else 
	{
		/*vty_out(vty,"Failed get args.\n");*/
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS; 
}
	


DEFUN(config_extend_acl_rule_mirror_redirect_cmd_func,
	  config_extend_acl_rule_mirror_redirect_cmd,
	  "acl extended <1-1000> redirect PORTNO (tcp|udp) \
	  dip (A.B.C.D/M|any) dst-port (<0-65535>|any) sip (A.B.C.D/M|any ) \
	  src-port (<0-65535>|any) dmac (H:H:H:H:H:H|any) smac (H:H:H:H:H:H|any) \
	  vid (<1-4095>|any) sourceport (PORTNO|any)",
	  ACL_STR
	  "extended acl rule\n"
	  "extended acl rule index range in 1-500\n"
	  "redirect matched packets to another port\n"
	  CONFIG_ETHPORT_STR 
	  "match tcp packets\n"
	  "match udp packets\n"
	  "care destination ip\n"
	  "destination ip format A.B.C.D/M\n"
	  "any destination ip\n"
	  "care destination port\n"
	  "destination port range in 0-65535\n"
	  "any destination port\n"
	  "care source ip\n"
	  "source ip format A.B.C.D/M\n"
	  "any source ip\n"
	  "care source port\n"
	  "source port range in 0-65535\n"
	  "any source port\n"
	  "care destination mac\n"
	  "destination mac format H:H:H:H:H:H\n"
	  "any destination mac\n"
	  "care source mac\n"
	  "source mac format H:H:H:H:H:H\n"
	  "any source mac \n"
	  "care vlan id\n"
	  "vlan id range in 1-4095\n"
	  "any vlan\n"
	  "care source port which packets comes from\n"
	  CONFIG_ETHPORT_STR 
	  "any port that packets come from\n"
)
{	  
	DBusMessage 	*query = NULL, *reply = NULL;
	DBusError		err = { 0 };
	ETHERADDR		dmacAddr,smacAddr;
	unsigned int	ruleIndex=0,dipmaskLen=0,sipmaskLen=0;
	unsigned int	actionType = 0,packetType = 0,srcport=0,dstport=0,vlanId=0;
	unsigned int	 ruleType = 0,op_ret = 0,ret = 0,temp = 0;
	unsigned char    slot_no=0,port_no=0,dataslot=ACL_ANY_PORT_CHAR,dataport=ACL_ANY_PORT_CHAR;
	char *			strcp = NULL;
	char *			str[20]={"ff:ff:ff:ff:ff:ff"};
	unsigned int	policer=0,policerId=0;
	int Val=0;
	unsigned long	dipno=0,sipno=0;
	
	#if 0
	if((argc < 2)||(argc > 12))
	{
		vty_out(vty, "bad parameters number!\n");
		return CMD_WARNING;
	}
	#endif
	
	ret = dcli_str2ulong((char*)argv[0],&ruleIndex);
	if (ACL_RETURN_CODE_ERROR == ret) {
		vty_out(vty,"%% Illegal rule index!\n");
		return CMD_WARNING;
	}
	ruleType = EXTENDED_ACL_RULE; 
	if ((ruleType == EXTENDED_ACL_RULE)&&(ruleIndex > MAX_EXT_RULE_NUM)) {
		vty_out(vty,"extended rule must less than 500!\n");
		return CMD_WARNING;
	}
	ruleIndex = ruleIndex-1;

	actionType=4;
	
	temp = parse_slotport_no((char*)argv[1],&slot_no,&port_no);
	
	if (ACL_RETURN_CODE_SUCCESS != ret) {
    	vty_out(vty,"%% Illegal format with slot/port!\n");
		return CMD_WARNING;
	}
	
	if(0==strncmp("udp",argv[2],strlen(argv[2])))
		packetType=1;
	else if(0==strncmp("tcp",argv[2],strlen(argv[2])))
		packetType=2;
	
	if(strncmp("any",argv[3],strlen(argv[3]))==0)
	{
		dipmaskLen=0;
		dipno=0;
	}
	else{
		Val=ip_address_format2ulong((char**)&argv[3],&dipno,&dipmaskLen);			
		if(CMD_WARNING == Val) {
			 vty_out(vty, "%% Bad parameter %s\n", argv[3]);
			 return CMD_WARNING;
		}
		VALUE_IP_MASK_CHECK(dipmaskLen);
	}
	
	if(strncmp("any",argv[4],strlen(argv[4]))==0) {	   
		dstport = ACL_ANY_PORT;
	}
	else{
		ret= dcli_str2ulong((char*)argv[4],&dstport);
		if(ret==ACL_RETURN_CODE_ERROR)
		{
			vty_out(vty,"%% Illegal destination port index!\n");
			return CMD_WARNING;
		}

	}
	if(strncmp("any",argv[5],strlen(argv[5]))==0)
	{ 
		sipmaskLen=0;
		sipno=0;
	}
	else{						
		 Val=ip_address_format2ulong((char**)&argv[5],&sipno,&sipmaskLen); 	
		 if(CMD_WARNING == Val) {
			  vty_out(vty, "%% Bad parameter %s\n", argv[5]);
			  return CMD_WARNING;
		 }
		 VALUE_IP_MASK_CHECK(sipmaskLen);
	}
	
	if(strncmp("any",argv[6],strlen(argv[6]))==0) {	   
		srcport = ACL_ANY_PORT;
	}
	else{
		ret	= dcli_str2ulong((char*)argv[6],&srcport);
		if(ret==ACL_RETURN_CODE_ERROR)
		{
			vty_out(vty,"%% Illegal source port index!\n");
			return CMD_WARNING;
		}
	}
	memset(&smacAddr,0,sizeof(ETHERADDR));
	memset(&dmacAddr,0,sizeof(ETHERADDR));

	/*dmac*/
	if(strncmp("any",argv[7],strlen(argv[7]))!=0){
		op_ret = parse_mac_addr((char *)argv[7],&dmacAddr);
		if (ACL_RETURN_CODE_SUCCESS != op_ret) {
			vty_out(vty,"%% Unknown mac addr format.\n");
			return CMD_SUCCESS;
		}
	}
	/*smac*/
	if(strncmp("any",argv[8],strlen(argv[8]))!=0){
		op_ret = parse_mac_addr((char *)argv[8],&smacAddr);
		if (ACL_RETURN_CODE_SUCCESS != op_ret) {
			vty_out(vty,"%% Unknown mac addr format.\n");
			return CMD_SUCCESS;
		}
	}
		
	if(strncmp("any",argv[9],strlen(argv[9]))!=0)
		{
			ret=dcli_str2ulong((char*)argv[9],&vlanId);
			if(ret==ACL_RETURN_CODE_ERROR)
				{
					vty_out(vty,"%% Illegal vlan Id!\n");
					return CMD_WARNING;
				}
		}
	/*dataslot/dataport*/
	if(strncmp("any",argv[10],strlen(argv[10]))!=0){
		ret = parse_slotport_no((char *)argv[10],&dataslot,&dataport);

		if (ACL_RETURN_CODE_SUCCESS != ret) {
			vty_out(vty,"%% Unknown portno format.\n");
			return CMD_SUCCESS;
		}
	}
	/*policer*/
	if((argc>11)&&(argc<14)){		
			if(strncmp("policer",argv[11],strlen(argv[11]))==0){
				policer = 1;
				ret = dcli_str2ulong((char *)argv[12],&policerId);
				if(ret==ACL_RETURN_CODE_ERROR)
				{
					vty_out(vty,"%% Illegal policer ID!\n");
					return CMD_WARNING;
				}
			}
			else{
				vty_out(vty,"%% Unknown command\n");
				return CMD_WARNING;
			}						
	}
	else if(argc>13){
		vty_out(vty,"%% Too many param\n");
		return CMD_WARNING;
	}
	/*query*/
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_ACL_OBJPATH,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_METHOD_CONFIG_ACL_RULE_EXTENDED_MIRROR_REDIRECT_TCP_UDP);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&ruleIndex,	
							 DBUS_TYPE_UINT32,&ruleType,
							 DBUS_TYPE_UINT32,&actionType,
							 DBUS_TYPE_BYTE,  &slot_no,
							 DBUS_TYPE_BYTE,  &port_no,
							 DBUS_TYPE_UINT32,&packetType,
							 DBUS_TYPE_UINT32,&dipno,
							 DBUS_TYPE_UINT32,&sipno,
							 DBUS_TYPE_UINT32,&dipmaskLen,
							 DBUS_TYPE_UINT32,&sipmaskLen,
							 DBUS_TYPE_UINT32,&dstport,
							 DBUS_TYPE_UINT32,&srcport,
							 DBUS_TYPE_BYTE,  &dmacAddr.arEther[0],
							 DBUS_TYPE_BYTE,  &dmacAddr.arEther[1],
							 DBUS_TYPE_BYTE,  &dmacAddr.arEther[2],
							 DBUS_TYPE_BYTE,  &dmacAddr.arEther[3],
							 DBUS_TYPE_BYTE,  &dmacAddr.arEther[4],
							 DBUS_TYPE_BYTE,  &dmacAddr.arEther[5],
							 DBUS_TYPE_BYTE,  &smacAddr.arEther[0],
							 DBUS_TYPE_BYTE,  &smacAddr.arEther[1],
							 DBUS_TYPE_BYTE,  &smacAddr.arEther[2],
							 DBUS_TYPE_BYTE,  &smacAddr.arEther[3],
							 DBUS_TYPE_BYTE,  &smacAddr.arEther[4],
							 DBUS_TYPE_BYTE,  &smacAddr.arEther[5],
							 DBUS_TYPE_UINT32,&vlanId,
							 DBUS_TYPE_BYTE,  &dataslot,
							 DBUS_TYPE_BYTE,  &dataport,							 
						     DBUS_TYPE_UINT32,&policer,							
 							 DBUS_TYPE_UINT32,&policerId,
							 DBUS_TYPE_INVALID);
  
   
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_acl,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{
			if (ACL_RETURN_CODE_SUCCESS == op_ret ) 
			{
				/*vty_out(vty,"Access-list %d is successfully set!\n",(ruleIndex+1));*/
			}
			else if(NPD_DBUS_ERROR_NO_SUCH_PORT ==op_ret){
				vty_out(vty,"%% Error! illegal port index\n");
			}
			else if(ACL_RETURN_CODE_GLOBAL_EXISTED == op_ret)
				vty_out(vty,"%% Access-list %d existed!\n",(ruleIndex+1));
			else if(ACL_RETURN_CODE_EXT_NO_SPACE==op_ret)
				 vty_out(vty,"%% because the extended rule should take up twice spaces than standard acl,\
				 			  but %d has been set standard rule! ~~set fail\n",(ruleIndex+513));
			else if(ACL_RETURN_CODE_SAME_FIELD == op_ret)
				vty_out(vty,"%% identical fields of packet can not set again\n");	
			else if(ACL_RETURN_CODE_POLICER_ID_NOT_SET == op_ret)
				vty_out(vty,"%% Policer %d not existed!\n",policerId);
			else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret)
				vty_out(vty,"%% Product not support this function!\n");
			else 
				vty_out(vty,"%% set fail!\n");
		
	} 
	else 
	{
		/*vty_out(vty,"Failed get args.\n");*/
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS; 
}

DEFUN(config_acl_service_enable_cmd_func,
	  config_acl_service_enable_cmd,
	  "config acl service (enable|disable)",
	  CONFIG_STR
	  "Configure acl service\n"
	  "Configure acl service enable or disable\n"
	  "Enable acl service \n"
	  "Disable acl service\n"	  
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	boolean isEnable = FALSE;
	unsigned int op_ret = 0;
	if(strncmp("enable",argv[0],strlen(argv[0]))==0)
	{
		isEnable = 1;
	}
	else if (strncmp("disable",argv[0],strlen(argv[0]))==0)
	{
		isEnable = 0;
	}
	else
	{
		vty_out(vty,"bad command parameter!\n");
		return CMD_WARNING;
	}		
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_ACL_OBJPATH,NPD_DBUS_ACL_INTERFACE, NPD_DBUS_METHOD_CONFIG_ACL);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&isEnable,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_acl,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{
			if (ACL_RETURN_CODE_SUCCESS == op_ret ) 
			{
				/*vty_out(vty,"Access-list Service is %d\n",isEnable);*/;
			}		
			else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret)
				vty_out(vty,"%% Product not support this function!\n");
		
	} 
	else 
	{
		/*vty_out(vty,"Failed get args.\n");*/
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(show_acl_service_cmd_func,
	show_acl_service_cmd,
	"show acl service",
	SHOW_STR
	"Show acl service\n"
	"Show acl service information\n"
)
{
	
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int Isable = 0;
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_ACL_OBJPATH,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_METHOD_SHOW_ACL_SERVICE);
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_acl,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&Isable,
		DBUS_TYPE_INVALID))
	{						
		if(ACL_TRUE==Isable)
			vty_out(vty,"Global acl service is enabled!\n");
		if(ACL_FALSE==Isable)
			vty_out(vty,"Global acl service is disabled!\n");			
	} 
	else 
	{
		/*vty_out(vty,"Failed get args.\n");*/
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

 DEFUN(show_acl_lists_cmd_func,
	 show_acl_lists_cmd,
	 "show acl list",
	 SHOW_STR
	 "Show acl rule\n" 
	 "Show all acl rules\n"
 )
 {	 
	 DBusMessage *query = NULL, *reply = NULL;
	 DBusError err;
	 DBusMessageIter  iter;
	 DBusMessageIter  iter_array;
	 char			 actype[50],protype[10];

	 unsigned int acl_Cont = 0;
	 unsigned int ruleIndex=0, startIndex = 0, endIndex = 0; 
	 unsigned int ruleType=0;
	 unsigned long dip=0,sip=0;
	 unsigned long  maskdip=0, masksip=0;
	 unsigned long srcport=0,dstport=0;
	 unsigned char icmp_code=0,icmp_type=0,code_mask=0,type_mask=0;
	 unsigned long packetType=0;
	 unsigned long actionType=0; 
	 unsigned char dmac[6]={0},smac[6]={0};
	 unsigned int vlanid=0;
	 unsigned char sourceslot=0,sourceport=0,redirectslot=0,redirectport=0,mirrorslot=0,mirrorport=0;
	 unsigned int i=0,j=0,ret,acl_count=0,type_flag;
	 unsigned char sipBuf[MAX_IP_STRLEN] = {0};
	 unsigned char dipBuf[MAX_IP_STRLEN] = {0};  
	 unsigned char *sipPtr = sipBuf,*dipPtr = dipBuf;
	 unsigned int policer = 0,policerId = 0;
	 unsigned int  modifyUP = 0,modifyDSCP = 0;
	 unsigned int  up = 0,dscp = 0,egrUP = 0, egrDSCP = 0;
	 unsigned int qosprofileindex = 0;
	 unsigned char upmm[10],dscpmm[10];
	 unsigned int precedence=0, nextheader= 0;
	 unsigned char appendIndex=0;
	 struct ipv6addr dipv6, sipv6;

	 memset(&dipv6, 0, sizeof(struct ipv6addr));
	 memset(&sipv6, 0, sizeof(struct ipv6addr));
	 
	 query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_ACL_OBJPATH,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_METHOD_SHOW_ACL);
	 dbus_error_init(&err);
	 reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_acl,query,-1, &err);
	 dbus_message_unref(query);
	 if (NULL == reply) 
	 {
		 /*vty_out(vty,"failed get reply.\n");*/
		 if (dbus_error_is_set(&err)) 
		 {
			 dbus_error_free_for_dcli(&err);
		 }
		 return CMD_SUCCESS;
	 }
	 dbus_message_iter_init(reply,&iter);
 
	 dbus_message_iter_get_basic(&iter,&ret);
 
	 if(0 == ret){
		 dbus_message_iter_next(&iter);  
		 dbus_message_iter_get_basic(&iter,&acl_count);
		 dbus_message_iter_next(&iter); 
		/* dbus_message_iter_get_basic(&iter,&type_flag); 
		// dbus_message_iter_next(&iter);  */
		 dbus_message_iter_recurse(&iter,&iter_array);
				   
		 for (j = 0; j < acl_count; j++) {
			DBusMessageIter iter_struct;
			dbus_message_iter_recurse(&iter_array,&iter_struct);			 
			dbus_message_iter_get_basic(&iter_struct,&ruleIndex);
			dbus_message_iter_next(&iter_struct); 
			dbus_message_iter_get_basic(&iter_struct,&startIndex);
			dbus_message_iter_next(&iter_struct); 
			dbus_message_iter_get_basic(&iter_struct,&endIndex);
			dbus_message_iter_next(&iter_struct); 
			dbus_message_iter_get_basic(&iter_struct,&ruleType);
			dbus_message_iter_next(&iter_struct); 
			dbus_message_iter_get_basic(&iter_struct,&actionType);
			dbus_message_iter_next(&iter_struct);			 
			dbus_message_iter_get_basic(&iter_struct,&packetType);
			dbus_message_iter_next(&iter_struct);			 
			dbus_message_iter_get_basic(&iter_struct,&dip);
			dbus_message_iter_next(&iter_struct);			 
			dbus_message_iter_get_basic(&iter_struct,&maskdip);
			dbus_message_iter_next(&iter_struct);			 
			dbus_message_iter_get_basic(&iter_struct,&sip);
			dbus_message_iter_next(&iter_struct);			 
			dbus_message_iter_get_basic(&iter_struct,&masksip);
			dbus_message_iter_next(&iter_struct);
			/*recv dst ipv6 address*/
			dbus_message_iter_get_basic(&iter_struct,&dipv6.ipbuf[0]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dipv6.ipbuf[1]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dipv6.ipbuf[2]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dipv6.ipbuf[3]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dipv6.ipbuf[4]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dipv6.ipbuf[5]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dipv6.ipbuf[6]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dipv6.ipbuf[7]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dipv6.ipbuf[8]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dipv6.ipbuf[9]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dipv6.ipbuf[10]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dipv6.ipbuf[11]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dipv6.ipbuf[12]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dipv6.ipbuf[13]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dipv6.ipbuf[14]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dipv6.ipbuf[15]);
			dbus_message_iter_next(&iter_struct);
			/*recv sorce ipv6 address*/
			dbus_message_iter_get_basic(&iter_struct,&sipv6.ipbuf[0]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&sipv6.ipbuf[1]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&sipv6.ipbuf[2]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&sipv6.ipbuf[3]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&sipv6.ipbuf[4]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&sipv6.ipbuf[5]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&sipv6.ipbuf[6]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&sipv6.ipbuf[7]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&sipv6.ipbuf[8]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&sipv6.ipbuf[9]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&sipv6.ipbuf[10]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&sipv6.ipbuf[11]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&sipv6.ipbuf[12]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&sipv6.ipbuf[13]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&sipv6.ipbuf[14]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&sipv6.ipbuf[15]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&nextheader);
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&dstport);
			dbus_message_iter_next(&iter_struct);			 
			dbus_message_iter_get_basic(&iter_struct,&srcport);
			dbus_message_iter_next(&iter_struct);			 
			dbus_message_iter_get_basic(&iter_struct,&icmp_code);
			dbus_message_iter_next(&iter_struct);			 
			dbus_message_iter_get_basic(&iter_struct,&icmp_type);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&code_mask);
			dbus_message_iter_next(&iter_struct);			 
			dbus_message_iter_get_basic(&iter_struct,&type_mask);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dmac[0]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dmac[1]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dmac[2]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dmac[3]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dmac[4]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dmac[5]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&smac[0]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&smac[1]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&smac[2]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&smac[3]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&smac[4]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&smac[5]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&vlanid);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&sourceslot);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&sourceport);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&redirectslot);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&redirectport);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&mirrorslot);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&mirrorport);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&policer);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&policerId);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&modifyUP);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&modifyDSCP);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&up);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dscp);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&egrUP);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&egrDSCP);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&qosprofileindex);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&precedence);	
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&appendIndex);
			dbus_message_iter_next(&iter_array); 
			 
		 switch(actionType)
		 {
			 case 0:strcpy(actype,"Permit");			 break;
			 case 1:strcpy(actype,"Deny");				 break;
			 case 2:strcpy(actype,"Trap"); 		 break;
			 case 3:strcpy(actype,"Permit");	 break;
			 case 4:strcpy(actype,"Redirect");			 break;
			 case 5:strcpy(actype,"Ingress QoS Mark");	 break;
			 case 6:strcpy(actype,"Egress QoS Remark");  break;
			 default: 
					 break; 	 
		 }
		
		switch(packetType)
		 {
			 case 0:strcpy(protype,"IP");		 break;
			 case 1:strcpy(protype,"UDP");		 break;
			 case 2:strcpy(protype,"TCP");		 break;
			 case 3:strcpy(protype,"ICMP"); 	 	break;  
			 case 4:strcpy(protype,"Ethernet");  break;
			 case 5:strcpy(protype,"ARP");	     break; 
			 case 6:strcpy(protype,"All");	     break; 
			 default:
					   break;									 
		 }	 
		 ip_long2str(dip,&dipPtr);
		 ip_long2str(sip,&sipPtr);

		 
		 vty_out(vty,"===============================================\n");
		 vty_out(vty,"%-40s: %ld\n","acl index",ruleIndex);

		 if(ruleType ==STANDARD_ACL_RULE){
		 	 vty_out(vty,"%-40s: %s\n","acl type","standard");
		 }	 
		 else {
		 	 vty_out(vty,"%-40s: %s\n","acl type","extended");
		 }	
		if((packetType==0 )||(packetType==3)){
		 	 vty_out(vty,"%-40s: %s\n","action",actype);
			 vty_out(vty,"%-40s: %s\n","ip protocal",protype);
			 vty_out(vty,"%-40s: %s/%ld\n","destination IP address",dipPtr,maskdip);				 
			 vty_out(vty,"%-40s: %s/%ld\n","source IP address",sipPtr,masksip);
			 
			if((strcmp(protype,"TCP")==0)||(strcmp(protype,"UDP")==0)){
				if (ACL_ANY_PORT == dstport) {
					vty_out(vty,"%-40s: %s\n","destination port","any");
				}
				else {
					vty_out(vty,"%-40s: %ld\n","destination port",dstport);
				}
				if (ACL_ANY_PORT == srcport) {
					vty_out(vty,"%-40s: %s\n","source port","any");
				}
				else {
					vty_out(vty,"%-40s: %ld\n","source port",srcport);
				}
			}
			if(strcmp(protype,"ICMP")==0){
				if (0 == code_mask) {
					 vty_out(vty,"%-40s: %s\n","icmp code","any");
				}
				else {
				 	vty_out(vty,"%-40s: %ld\n","icmp code",icmp_code);
				}
				if (0 == type_mask) {
				 	vty_out(vty,"%-40s: %s\n","icmp type","any");
				}
				else {
					vty_out(vty,"%-40s: %ld\n","icmp type",icmp_type);
				}
			}
			if (strcmp(actype,"Redirect")==0)
		 		 vty_out(vty,"%-40s: %d/%d\n","redirect port",redirectslot,redirectport);
		 	
		   
			if(1==policer){
				vty_out(vty,"%-40s: %s\n","policer","enable");
				vty_out(vty,"%-40s: %d\n","policer ID",policerId);
			}
			if(32==appendIndex){
				vty_out(vty,"%-40s: %d\n","ingress qos",qosprofileindex);
		
			}
				vty_out(vty,"===============================================\n");
	    }
	  			
		else if((packetType==4)||(packetType==5)){
			 vty_out(vty,"%-40s: %s\n","action",actype);
		 	 vty_out(vty,"%-40s: %s\n","ip protocal",protype);
			 vty_out(vty,"%-40s: %02x:%02x:%02x:%02x:%02x:%02x \n","source MAC",smac[0],smac[1],smac[2],smac[3],smac[4],smac[5]); 				 
				
			 if(strcmp(protype,"Ethernet")==0){
				 vty_out(vty,"%-40s: %02x:%02x:%02x:%02x:%02x:%02x \n","destination MAC",dmac[0],dmac[1],dmac[2],dmac[3],dmac[4],dmac[5]);
			 }
		
			if(strcmp(protype,"ARP")==0){
				 vty_out(vty,"%-40s: %s \n","destination MAC","FF:FF:FF:FF:FF:FF");
				 if (0 == vlanid) {
			 	 	vty_out(vty,"%-40s: %s\n","vlan id","any");
				 }
				 else {
					vty_out(vty,"%-40s: %d\n","vlan id", vlanid);
				 }
				 if((ACL_ANY_PORT_CHAR == sourceslot)&&(ACL_ANY_PORT_CHAR == sourceport)){
			       	vty_out(vty,"%-40s: %s\n","source port", "any");
				 }
				 else {
					vty_out(vty,"%-40s: %d/%d\n","source port",sourceslot,sourceport);
				 }
			}

			 if (strcmp(actype,"Redirect")==0)
			 	vty_out(vty,"%-40s: %d/%d\n","redirect port",redirectslot,redirectport);

			
			 if(1==policer){
				vty_out(vty,"%-40s: %s\n","policer","enable");
				vty_out(vty,"%-40s: %d\n","policer ID",policerId);
		 	 }
			if(32==appendIndex){
				vty_out(vty,"%-40s: %d\n","ingress qos",qosprofileindex);
		
			}
		     vty_out(vty,"===============================================\n");
		}
		else if((packetType==6)){
			
			switch(modifyUP){
				case 0:strcpy(upmm,"Keep"); break;
				case 1:
				case 2:strcpy(upmm,"Enable");break;
				default :break;
			}
			switch(modifyDSCP){
				case 0:strcpy(dscpmm,"Keep"); break;
				case 1:
				case 2:strcpy(dscpmm,"Enable");break;
				default :break;
			}	
							
			vty_out(vty,"%-40s: %s\n","action",actype);
			
			if(0==modifyUP)
				vty_out(vty,"%-40s: %s\n","source UP","none");
			else if(2==modifyUP)
				vty_out(vty,"%-40s: %d\n","source UP",up);
			if(0==modifyDSCP)
				vty_out(vty,"%-40s: %s\n","source Dscp","none");
			else if(2==modifyDSCP)
				vty_out(vty,"%-40s: %d\n","source Dscp",dscp);
			
			if(strcmp(actype,"Ingress QoS Mark")==0){	
				
				vty_out(vty,"%-40s: %d\n","Mark QoS profile Table",qosprofileindex);
				if(precedence==0)
					vty_out(vty,"%-40s: %s\n","SubQosMarkers","enable");
				else if(precedence==1)
					vty_out(vty,"%-40s: %s\n","SubQosMarkers","disable");
			}
			if(strcmp(actype,"Egress QoS Remark")==0){																
				vty_out(vty,"%-40s: %d\n","egress UP",egrUP);
			    vty_out(vty,"%-40s: %d\n","egress DSCP",egrDSCP);
			}
			
			vty_out(vty,"%-40s: %s\n","Modify UP",upmm);
			vty_out(vty,"%-40s: %s\n","Modify DSCP",dscpmm);		
			
			if(1==policer){
				vty_out(vty,"%-40s: %s\n","policer","enable");
				vty_out(vty,"%-40s: %d\n","policer ID",policerId);
		 	 }
			
			vty_out(vty,"===============================================\n");
	  }
	 else if((packetType==1)||(packetType==2)){ 
	 	 if(ruleType == STANDARD_ACL_RULE) {
		 	 vty_out(vty,"%-40s: %s\n","action",actype);
			 vty_out(vty,"%-40s: %s\n","ip protocal",protype);
			 vty_out(vty,"%-40s: %s/%ld\n","destination IP address",dipPtr,maskdip);				 
			 vty_out(vty,"%-40s: %s/%ld\n","source IP address",sipPtr,masksip);
			 
			if((strcmp(protype,"TCP")==0)||(strcmp(protype,"UDP")==0)){
				if (ACL_ANY_PORT == dstport) {
					vty_out(vty,"%-40s: %s\n","destination port","any");
				}
				else {
					vty_out(vty,"%-40s: %ld\n","destination port",dstport);
				}
				if (ACL_ANY_PORT == srcport) {
					vty_out(vty,"%-40s: %s\n","source port","any");
				}
				else {
					vty_out(vty,"%-40s: %ld\n","source port",srcport);
				}
			}
			if(strcmp(protype,"ICMP")==0){
				 vty_out(vty,"%-40s: %ld\n","icmp code",icmp_code);
				 vty_out(vty,"%-40s: %ld\n","icmp type",icmp_type);
			}
			if (strcmp(actype,"Redirect")==0)
		 		 vty_out(vty,"%-40s: %d/%d\n","redirect port",redirectslot,redirectport);
		 	
		   
			if(1==policer){
				vty_out(vty,"%-40s: %s\n","policer","enable");
				vty_out(vty,"%-40s: %d\n","policer ID",policerId);
			}
			if(32==appendIndex){
				vty_out(vty,"%-40s: %d\n","ingress qos",qosprofileindex);
		
			}
				vty_out(vty,"===============================================\n");
	    	  }
		 else if(ruleType==EXTENDED_ACL_RULE){
		 	 /*vty_out(vty,"%-40s: %s\n","acl type","extended");*/
			 vty_out(vty,"%-40s: %s\n","action",actype);
			 vty_out(vty,"%-40s: %s\n","ip protocal",protype);
			 vty_out(vty,"%-40s: %s/%ld\n","destination IP address",dipPtr,maskdip);
		 	if (ACL_ANY_PORT == dstport) {
				vty_out(vty,"%-40s: %s\n","destination port","any");
			}
			else {
				vty_out(vty,"%-40s: %ld\n","destination port",dstport);
			}
			 vty_out(vty,"%-40s: %s/%ld\n","source IP address",sipPtr,masksip);
			 if (ACL_ANY_PORT == srcport) {
				vty_out(vty,"%-40s: %s\n","source port","any");
			}
			else {
				vty_out(vty,"%-40s: %ld\n","source port",srcport);
			}
			 vty_out(vty,"%-40s: %02x:%02x:%02x:%02x:%02x:%02x \n","destination MAC",dmac[0],dmac[1],dmac[2],dmac[3],dmac[4],dmac[5]);
			 vty_out(vty,"%-40s: %02x:%02x:%02x:%02x:%02x:%02x \n","source MAC",smac[0],smac[1],smac[2],smac[3],smac[4],smac[5]); 
		 	 if (0 == vlanid) {
		 	 	vty_out(vty,"%-40s: %s\n","vlan id","any");
			 }
			 else {
				vty_out(vty,"%-40s: %d\n","vlan id", vlanid);
			 }
		 	if((ACL_ANY_PORT_CHAR == sourceslot)&&(ACL_ANY_PORT_CHAR == sourceport)){
		       	vty_out(vty,"%-40s: %s\n","source port", "any");
			 }
			 else {
				vty_out(vty,"%-40s: %d/%d\n","source port",sourceslot,sourceport);
			 }

			 if (strcmp(actype,"Redirect")==0)
		 		 vty_out(vty,"%-40s: %d/%d\n","redirect port",redirectslot,redirectport);
		 	
		     if (strcmp(actype,"MirrorToAnalyzer")==0)
		 		vty_out(vty,"%-40s: %d/%d\n","analyzer port",mirrorslot,mirrorport);
	        
			 if(1==policer){
				vty_out(vty,"%-40s: %s\n","policer","enable");
				vty_out(vty,"%-40s: %d\n","policer ID",policerId);
			  }
			 if(32==appendIndex){
					vty_out(vty,"%-40s: %d\n","ingress qos",qosprofileindex);
			
				}
				vty_out(vty,"===============================================\n");
		    }
	 
	      }	
	 	else {
		 	 /*vty_out(vty,"%-40s: %s\n","acl type","extended");*/
			 vty_out(vty,"%-40s: %s\n","action",actype);

			 if (strcmp(actype,"Redirect")==0){
		 		 vty_out(vty,"%-40s: %d/%d\n","redirect port",redirectslot,redirectport);
				 vty_out(vty,"%-40s: %u\n","ipv6 next-header", nextheader);
			 }
			 else{
			 	vty_out(vty,"%-40s: %u\n","ipv6 next-header", nextheader);
			 }			 
			 char dbuf[sizeof("ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255")];
			  vty_out(vty,"%-40s: %s \n","destination IP address",inet_ntop(AF_INET6, dipv6.ipbuf, dbuf, sizeof(dbuf)));
			  if (8 == packetType) {
				 if (ACL_ANY_PORT == dstport) {
					 vty_out(vty,"%-40s: %s\n","destination port","any");
				 }
				 else {
					 vty_out(vty,"%-40s: %ld\n","destination port",dstport);
				 }
			  }
			  
			  char sbuf[sizeof("ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255")];
			  vty_out(vty,"%-40s: %s \n","source IP address",inet_ntop(AF_INET6, sipv6.ipbuf, sbuf, sizeof(sbuf)));
			  if (8 == packetType) {
				  if (ACL_ANY_PORT == srcport) {
					 vty_out(vty,"%-40s: %s\n","source port","any");
				 }
				 else {
					 vty_out(vty,"%-40s: %ld\n","source port",srcport);
				 }
			 }
			    if (strcmp(actype,"Redirect")!=0){
			     	if(1==policer){
			    		vty_out(vty,"%-40s: %s\n","policer","enable");
			    		vty_out(vty,"%-40s: %d\n","policer ID",policerId);
			    		}
			    	if(32==appendIndex){
			    		vty_out(vty,"%-40s: %d\n","ingress qos",qosprofileindex);		
			    	}
			     }
   			
			vty_out(vty,"===============================================\n");
	 		}	 
		}
     }  /*rule not null*/
	
	 else if(ACL_RETURN_CODE_ERROR == ret) {
		 vty_out(vty,"%% Error: No ACL rule exists.\n");
	 }
 	else if(NPD_DBUS_ERROR_NO_SUCH_PORT ==ret){
		vty_out(vty,"%% Error: illegal port index\n");
	}
	else if(NPD_DBUS_ERROR_NO_SUCH_VLAN +1 ==ret){
		vty_out(vty,"%% Error:illegal vlan index\n");
	}
	
	dbus_message_unref(reply);
	 return CMD_SUCCESS;
 }
show_ip_acl_lists_count
(
	unsigned int *count
)
 {	  
	  DBusMessage *query = NULL, *reply = NULL;
	  DBusError err;
	  DBusMessageIter  iter;
	  DBusMessageIter  iter_array;
	  char			  actype[50],protype[10];
 
	  unsigned int acl_Cont = 0;
	  unsigned int ruleIndex=0, startIndex = 0, endIndex = 0; 
	  unsigned int ruleType=0;
	  unsigned long dip=0,sip=0, tmpsip = 0;
	  unsigned long  maskdip=0, masksip=0;
	  unsigned long srcport=0,dstport=0;
	  unsigned char icmp_code=0,icmp_type=0,code_mask=0,type_mask=0;
	  unsigned long packetType=0;
	  unsigned long actionType=0; 
	  unsigned char dmac[6]={0},smac[6]={0};
	  unsigned int vlanid=0;
	  unsigned char sourceslot=0,sourceport=0,redirectslot=0,redirectport=0,mirrorslot=0,mirrorport=0;
	  unsigned int i=0,j=0,k=0,ret,acl_count=0,type_flag;
	  unsigned char sipBuf[MAX_IP_STRLEN] = {0};
	  unsigned char dipBuf[MAX_IP_STRLEN] = {0};  
	  unsigned char *sipPtr = sipBuf,*dipPtr = dipBuf;
	  unsigned int policer = 0,policerId = 0;
	  unsigned int	modifyUP = 0,modifyDSCP = 0;
	  unsigned int	up = 0,dscp = 0,egrUP = 0, egrDSCP = 0;
	  unsigned int qosprofileindex = 0;
	  unsigned char upmm[10],dscpmm[10];
	  unsigned int precedence=0, nextheader= 0;
	  unsigned char appendIndex=0;
	  struct ipv6addr dipv6, sipv6;
 
	  memset(&dipv6, 0, sizeof(struct ipv6addr));
	  memset(&sipv6, 0, sizeof(struct ipv6addr));
	  
	  query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_ACL_OBJPATH,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_METHOD_SHOW_ACL);
	  dbus_error_init(&err);
	  reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_acl,query,-1, &err);
	  dbus_message_unref(query);
	  if (NULL == reply) 
	  {
		  /*vty_out(vty,"failed get reply.\n");*/
		  if (dbus_error_is_set(&err)) 
		  {
			  dbus_error_free_for_dcli(&err);
		  }
		  return CMD_SUCCESS;
	  }
	  dbus_message_iter_init(reply,&iter);
  
	  dbus_message_iter_get_basic(&iter,&ret);
  
	  if(0 == ret){
		  dbus_message_iter_next(&iter);  
		  dbus_message_iter_get_basic(&iter,&acl_count);
		  dbus_message_iter_next(&iter); 
		 /* dbus_message_iter_get_basic(&iter,&type_flag); 
		 // dbus_message_iter_next(&iter);	*/
		  dbus_message_iter_recurse(&iter,&iter_array);
					
		  for (j = 0; j < acl_count; j++) {
			 DBusMessageIter iter_struct;
			 dbus_message_iter_recurse(&iter_array,&iter_struct);			  
			 dbus_message_iter_get_basic(&iter_struct,&ruleIndex);
			 dbus_message_iter_next(&iter_struct); 
			 dbus_message_iter_get_basic(&iter_struct,&startIndex);
			 dbus_message_iter_next(&iter_struct); 
			 dbus_message_iter_get_basic(&iter_struct,&endIndex);			 
			 dbus_message_iter_next(&iter_struct); 
			 dbus_message_iter_get_basic(&iter_struct,&ruleType);
			 dbus_message_iter_next(&iter_struct); 
			 dbus_message_iter_get_basic(&iter_struct,&actionType);
			 dbus_message_iter_next(&iter_struct);			  
			 dbus_message_iter_get_basic(&iter_struct,&packetType);
			 dbus_message_iter_next(&iter_struct);			  
			 dbus_message_iter_get_basic(&iter_struct,&dip);
			 dbus_message_iter_next(&iter_struct);			  
			 dbus_message_iter_get_basic(&iter_struct,&maskdip);
			 dbus_message_iter_next(&iter_struct);			  
			 dbus_message_iter_get_basic(&iter_struct,&sip);
			 dbus_message_iter_next(&iter_struct);			  
			 dbus_message_iter_get_basic(&iter_struct,&masksip);
			 dbus_message_iter_next(&iter_struct);
			 /*recv dst ipv6 address*/
			 dbus_message_iter_get_basic(&iter_struct,&dipv6.ipbuf[0]);
			 dbus_message_iter_next(&iter_struct);
			 dbus_message_iter_get_basic(&iter_struct,&dipv6.ipbuf[1]);
			 dbus_message_iter_next(&iter_struct);
			 dbus_message_iter_get_basic(&iter_struct,&dipv6.ipbuf[2]);
			 dbus_message_iter_next(&iter_struct);
			 dbus_message_iter_get_basic(&iter_struct,&dipv6.ipbuf[3]);
			 dbus_message_iter_next(&iter_struct);
			 dbus_message_iter_get_basic(&iter_struct,&dipv6.ipbuf[4]);
			 dbus_message_iter_next(&iter_struct);
			 dbus_message_iter_get_basic(&iter_struct,&dipv6.ipbuf[5]);
			 dbus_message_iter_next(&iter_struct);
			 dbus_message_iter_get_basic(&iter_struct,&dipv6.ipbuf[6]);
			 dbus_message_iter_next(&iter_struct);
			 dbus_message_iter_get_basic(&iter_struct,&dipv6.ipbuf[7]);
			 dbus_message_iter_next(&iter_struct);
			 dbus_message_iter_get_basic(&iter_struct,&dipv6.ipbuf[8]);
			 dbus_message_iter_next(&iter_struct);
			 dbus_message_iter_get_basic(&iter_struct,&dipv6.ipbuf[9]);
			 dbus_message_iter_next(&iter_struct);
			 dbus_message_iter_get_basic(&iter_struct,&dipv6.ipbuf[10]);
			 dbus_message_iter_next(&iter_struct);
			 dbus_message_iter_get_basic(&iter_struct,&dipv6.ipbuf[11]);
			 dbus_message_iter_next(&iter_struct);
			 dbus_message_iter_get_basic(&iter_struct,&dipv6.ipbuf[12]);
			 dbus_message_iter_next(&iter_struct);
			 dbus_message_iter_get_basic(&iter_struct,&dipv6.ipbuf[13]);
			 dbus_message_iter_next(&iter_struct);
			 dbus_message_iter_get_basic(&iter_struct,&dipv6.ipbuf[14]);
			 dbus_message_iter_next(&iter_struct);
			 dbus_message_iter_get_basic(&iter_struct,&dipv6.ipbuf[15]);
			 dbus_message_iter_next(&iter_struct);
			 /*recv sorce ipv6 address*/
			 dbus_message_iter_get_basic(&iter_struct,&sipv6.ipbuf[0]);
			 dbus_message_iter_next(&iter_struct);
			 dbus_message_iter_get_basic(&iter_struct,&sipv6.ipbuf[1]);
			 dbus_message_iter_next(&iter_struct);
			 dbus_message_iter_get_basic(&iter_struct,&sipv6.ipbuf[2]);
			 dbus_message_iter_next(&iter_struct);
			 dbus_message_iter_get_basic(&iter_struct,&sipv6.ipbuf[3]);
			 dbus_message_iter_next(&iter_struct);
			 dbus_message_iter_get_basic(&iter_struct,&sipv6.ipbuf[4]);
			 dbus_message_iter_next(&iter_struct);
			 dbus_message_iter_get_basic(&iter_struct,&sipv6.ipbuf[5]);
			 dbus_message_iter_next(&iter_struct);
			 dbus_message_iter_get_basic(&iter_struct,&sipv6.ipbuf[6]);
			 dbus_message_iter_next(&iter_struct);
			 dbus_message_iter_get_basic(&iter_struct,&sipv6.ipbuf[7]);
			 dbus_message_iter_next(&iter_struct);
			 dbus_message_iter_get_basic(&iter_struct,&sipv6.ipbuf[8]);
			 dbus_message_iter_next(&iter_struct);
			 dbus_message_iter_get_basic(&iter_struct,&sipv6.ipbuf[9]);
			 dbus_message_iter_next(&iter_struct);
			 dbus_message_iter_get_basic(&iter_struct,&sipv6.ipbuf[10]);
			 dbus_message_iter_next(&iter_struct);
			 dbus_message_iter_get_basic(&iter_struct,&sipv6.ipbuf[11]);
			 dbus_message_iter_next(&iter_struct);
			 dbus_message_iter_get_basic(&iter_struct,&sipv6.ipbuf[12]);
			 dbus_message_iter_next(&iter_struct);
			 dbus_message_iter_get_basic(&iter_struct,&sipv6.ipbuf[13]);
			 dbus_message_iter_next(&iter_struct);
			 dbus_message_iter_get_basic(&iter_struct,&sipv6.ipbuf[14]);
			 dbus_message_iter_next(&iter_struct);
			 dbus_message_iter_get_basic(&iter_struct,&sipv6.ipbuf[15]);
			 dbus_message_iter_next(&iter_struct);
			 dbus_message_iter_get_basic(&iter_struct,&nextheader);
			 dbus_message_iter_next(&iter_struct);
 
			 dbus_message_iter_get_basic(&iter_struct,&dstport);
			 dbus_message_iter_next(&iter_struct);			  
			 dbus_message_iter_get_basic(&iter_struct,&srcport);
			 dbus_message_iter_next(&iter_struct);			  
			 dbus_message_iter_get_basic(&iter_struct,&icmp_code);
			 dbus_message_iter_next(&iter_struct);			  
			 dbus_message_iter_get_basic(&iter_struct,&icmp_type);
			 dbus_message_iter_next(&iter_struct);
			 dbus_message_iter_get_basic(&iter_struct,&code_mask);
			 dbus_message_iter_next(&iter_struct);			  
			 dbus_message_iter_get_basic(&iter_struct,&type_mask);
			 dbus_message_iter_next(&iter_struct);
			 dbus_message_iter_get_basic(&iter_struct,&dmac[0]);
			 dbus_message_iter_next(&iter_struct);
			 dbus_message_iter_get_basic(&iter_struct,&dmac[1]);
			 dbus_message_iter_next(&iter_struct);
			 dbus_message_iter_get_basic(&iter_struct,&dmac[2]);
			 dbus_message_iter_next(&iter_struct);
			 dbus_message_iter_get_basic(&iter_struct,&dmac[3]);
			 dbus_message_iter_next(&iter_struct);
			 dbus_message_iter_get_basic(&iter_struct,&dmac[4]);
			 dbus_message_iter_next(&iter_struct);
			 dbus_message_iter_get_basic(&iter_struct,&dmac[5]);
			 dbus_message_iter_next(&iter_struct);
			 dbus_message_iter_get_basic(&iter_struct,&smac[0]);
			 dbus_message_iter_next(&iter_struct);
			 dbus_message_iter_get_basic(&iter_struct,&smac[1]);
			 dbus_message_iter_next(&iter_struct);
			 dbus_message_iter_get_basic(&iter_struct,&smac[2]);
			 dbus_message_iter_next(&iter_struct);
			 dbus_message_iter_get_basic(&iter_struct,&smac[3]);
			 dbus_message_iter_next(&iter_struct);
			 dbus_message_iter_get_basic(&iter_struct,&smac[4]);
			 dbus_message_iter_next(&iter_struct);
			 dbus_message_iter_get_basic(&iter_struct,&smac[5]);
			 dbus_message_iter_next(&iter_struct);
			 dbus_message_iter_get_basic(&iter_struct,&vlanid);
			 dbus_message_iter_next(&iter_struct);
			 dbus_message_iter_get_basic(&iter_struct,&sourceslot);
			 dbus_message_iter_next(&iter_struct);
			 dbus_message_iter_get_basic(&iter_struct,&sourceport);
			 dbus_message_iter_next(&iter_struct);
			 dbus_message_iter_get_basic(&iter_struct,&redirectslot);
			 dbus_message_iter_next(&iter_struct);
			 dbus_message_iter_get_basic(&iter_struct,&redirectport);
			 dbus_message_iter_next(&iter_struct);
			 dbus_message_iter_get_basic(&iter_struct,&mirrorslot);
			 dbus_message_iter_next(&iter_struct);
			 dbus_message_iter_get_basic(&iter_struct,&mirrorport);
			 dbus_message_iter_next(&iter_struct);
			 dbus_message_iter_get_basic(&iter_struct,&policer);
			 dbus_message_iter_next(&iter_struct);
			 dbus_message_iter_get_basic(&iter_struct,&policerId);
			 dbus_message_iter_next(&iter_struct);
			 dbus_message_iter_get_basic(&iter_struct,&modifyUP);
			 dbus_message_iter_next(&iter_struct);
			 dbus_message_iter_get_basic(&iter_struct,&modifyDSCP);
			 dbus_message_iter_next(&iter_struct);
			 dbus_message_iter_get_basic(&iter_struct,&up);
			 dbus_message_iter_next(&iter_struct);
			 dbus_message_iter_get_basic(&iter_struct,&dscp);
			 dbus_message_iter_next(&iter_struct);
			 dbus_message_iter_get_basic(&iter_struct,&egrUP);
			 dbus_message_iter_next(&iter_struct);
			 dbus_message_iter_get_basic(&iter_struct,&egrDSCP);
			 dbus_message_iter_next(&iter_struct);
			 dbus_message_iter_get_basic(&iter_struct,&qosprofileindex);
			 dbus_message_iter_next(&iter_struct);
			 dbus_message_iter_get_basic(&iter_struct,&precedence);  
			 dbus_message_iter_next(&iter_struct);
			 dbus_message_iter_get_basic(&iter_struct,&appendIndex);
			 dbus_message_iter_next(&iter_array); 
			  
		  switch(actionType)
		  {
			  case 0:strcpy(actype,"Permit");			  break;
			  case 1:strcpy(actype,"Deny"); 			  break;
			  case 2:strcpy(actype,"Trap"); 	  break;
			  case 3:strcpy(actype,"Permit");	  break;
			  case 4:strcpy(actype,"Redirect"); 		  break;
			  case 5:strcpy(actype,"Ingress QoS Mark");   break;
			  case 6:strcpy(actype,"Egress QoS Remark");  break;
			  default: 
					  break;	  
		  }
		 
		 switch(packetType)
		  {
			  case 0:strcpy(protype,"IP");		  break;
			  case 1:strcpy(protype,"UDP"); 	  break;
			  case 2:strcpy(protype,"TCP"); 	  break;
			  case 3:strcpy(protype,"ICMP");		 break;  
			  case 4:strcpy(protype,"Ethernet");  break;
			  case 5:strcpy(protype,"ARP"); 	  break; 
			  case 6:strcpy(protype,"All"); 	  break; 
			  default:
						break;									  
		  }   
		  ip_long2str(dip,&dipPtr);
		  ip_long2str(sip,&sipPtr);
 
		  if ((packetType==0) && (startIndex && endIndex)) {
				*count += (endIndex - startIndex + 1); 
		  }
		  	  
		 }
	  }  /*rule not null*/
	 
	  else if(ACL_RETURN_CODE_ERROR == ret) {
		  return 1;
	  }
	 else if(NPD_DBUS_ERROR_NO_SUCH_PORT ==ret){
		 return 1;
	 }
	 else if(NPD_DBUS_ERROR_NO_SUCH_VLAN +1 ==ret){
		return 1;
	 }
	 
	 dbus_message_unref(reply);
	 
	  return CMD_SUCCESS;
  }

DEFUN(show_ip_acl_lists_cmd_func,
	 show_ip_acl_lists_cmd,
	 "show ip-acl list",
	 SHOW_STR
	 "Show ip-acl rule\n" 
	 "Show all ip-acl rules\n"
 )
 {	 
	 DBusMessage *query = NULL, *reply = NULL;
	 DBusError err;
	 DBusMessageIter  iter;
	 DBusMessageIter  iter_array;
	 char			 actype[50],protype[10];

	 unsigned int acl_Cont = 0;
	 unsigned int ruleIndex=0, startIndex = 0, endIndex = 0; 
	 unsigned int ruleType=0;
	 unsigned long dip=0,sip=0, tmpsip = 0, count = 0, len= 0;
	 unsigned long  maskdip=0, masksip=0;
	 unsigned long srcport=0,dstport=0;
	 unsigned char icmp_code=0,icmp_type=0,code_mask=0,type_mask=0;
	 unsigned long packetType=0;
	 unsigned long actionType=0; 
	 unsigned char dmac[6]={0},smac[6]={0};
	 unsigned int vlanid=0;
	 unsigned char sourceslot=0,sourceport=0,redirectslot=0,redirectport=0,mirrorslot=0,mirrorport=0;
	 unsigned int i=0,j=0,k=0,ret = 0,acl_count=0,type_flag;
	 unsigned char sipBuf[20] = {0};
	 unsigned char dipBuf[20] = {0};  
	 unsigned char *sipPtr = sipBuf,*dipPtr = dipBuf;
	 unsigned int policer = 0,policerId = 0;
	 unsigned int  modifyUP = 0,modifyDSCP = 0;
	 unsigned int  up = 0,dscp = 0,egrUP = 0, egrDSCP = 0;
	 unsigned int qosprofileindex = 0;
	 unsigned char upmm[10],dscpmm[10];
	 unsigned int precedence=0, nextheader= 0;
	 unsigned char appendIndex=0;
	 struct ipv6addr dipv6, sipv6;

	 memset(&dipv6, 0, sizeof(struct ipv6addr));
	 memset(&sipv6, 0, sizeof(struct ipv6addr));
	 
	 ret = show_ip_acl_lists_count(&count);
	 if (CMD_SUCCESS != ret ) {
		 return CMD_SUCCESS;
	 }
	 vty_out(vty, "===========================ACL COUNT %-5d=========================\n", count);
	 vty_out(vty,"%-12s%-18s%-18s%-13s%-10s\n","acl-id","destination-ip" ,"source-ip", "action", "policer-id");
	 
	 query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_ACL_OBJPATH,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_METHOD_SHOW_ACL);
	 dbus_error_init(&err);
	 reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_acl,query,-1, &err);
	 dbus_message_unref(query);
	 if (NULL == reply) 
	 {
		 /*vty_out(vty,"failed get reply.\n");*/
		 if (dbus_error_is_set(&err)) 
		 {
			 dbus_error_free_for_dcli(&err);
		 }
		 return CMD_SUCCESS;
	 }
	 dbus_message_iter_init(reply,&iter);
 
	 dbus_message_iter_get_basic(&iter,&ret);
 
	 if(0 == ret){
		 dbus_message_iter_next(&iter);  
		 dbus_message_iter_get_basic(&iter,&acl_count);
		 dbus_message_iter_next(&iter); 
		/* dbus_message_iter_get_basic(&iter,&type_flag); 
		// dbus_message_iter_next(&iter);  */
		 dbus_message_iter_recurse(&iter,&iter_array);
				   
		 for (j = 0; j < acl_count; j++) {
			DBusMessageIter iter_struct;
			dbus_message_iter_recurse(&iter_array,&iter_struct);			 
			dbus_message_iter_get_basic(&iter_struct,&ruleIndex);
			dbus_message_iter_next(&iter_struct); 
			dbus_message_iter_get_basic(&iter_struct,&startIndex);
			dbus_message_iter_next(&iter_struct); 
			dbus_message_iter_get_basic(&iter_struct,&endIndex);
			dbus_message_iter_next(&iter_struct); 
			dbus_message_iter_get_basic(&iter_struct,&ruleType);
			dbus_message_iter_next(&iter_struct); 
			dbus_message_iter_get_basic(&iter_struct,&actionType);
			dbus_message_iter_next(&iter_struct);			 
			dbus_message_iter_get_basic(&iter_struct,&packetType);
			dbus_message_iter_next(&iter_struct);			 
			dbus_message_iter_get_basic(&iter_struct,&dip);
			dbus_message_iter_next(&iter_struct);			 
			dbus_message_iter_get_basic(&iter_struct,&maskdip);
			dbus_message_iter_next(&iter_struct);			 
			dbus_message_iter_get_basic(&iter_struct,&sip);
			dbus_message_iter_next(&iter_struct);			 
			dbus_message_iter_get_basic(&iter_struct,&masksip);
			dbus_message_iter_next(&iter_struct);
			/*recv dst ipv6 address*/
			dbus_message_iter_get_basic(&iter_struct,&dipv6.ipbuf[0]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dipv6.ipbuf[1]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dipv6.ipbuf[2]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dipv6.ipbuf[3]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dipv6.ipbuf[4]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dipv6.ipbuf[5]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dipv6.ipbuf[6]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dipv6.ipbuf[7]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dipv6.ipbuf[8]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dipv6.ipbuf[9]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dipv6.ipbuf[10]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dipv6.ipbuf[11]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dipv6.ipbuf[12]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dipv6.ipbuf[13]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dipv6.ipbuf[14]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dipv6.ipbuf[15]);
			dbus_message_iter_next(&iter_struct);
			/*recv sorce ipv6 address*/
			dbus_message_iter_get_basic(&iter_struct,&sipv6.ipbuf[0]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&sipv6.ipbuf[1]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&sipv6.ipbuf[2]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&sipv6.ipbuf[3]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&sipv6.ipbuf[4]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&sipv6.ipbuf[5]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&sipv6.ipbuf[6]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&sipv6.ipbuf[7]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&sipv6.ipbuf[8]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&sipv6.ipbuf[9]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&sipv6.ipbuf[10]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&sipv6.ipbuf[11]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&sipv6.ipbuf[12]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&sipv6.ipbuf[13]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&sipv6.ipbuf[14]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&sipv6.ipbuf[15]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&nextheader);
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&dstport);
			dbus_message_iter_next(&iter_struct);			 
			dbus_message_iter_get_basic(&iter_struct,&srcport);
			dbus_message_iter_next(&iter_struct);			 
			dbus_message_iter_get_basic(&iter_struct,&icmp_code);
			dbus_message_iter_next(&iter_struct);			 
			dbus_message_iter_get_basic(&iter_struct,&icmp_type);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&code_mask);
			dbus_message_iter_next(&iter_struct);			 
			dbus_message_iter_get_basic(&iter_struct,&type_mask);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dmac[0]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dmac[1]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dmac[2]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dmac[3]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dmac[4]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dmac[5]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&smac[0]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&smac[1]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&smac[2]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&smac[3]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&smac[4]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&smac[5]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&vlanid);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&sourceslot);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&sourceport);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&redirectslot);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&redirectport);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&mirrorslot);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&mirrorport);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&policer);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&policerId);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&modifyUP);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&modifyDSCP);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&up);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dscp);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&egrUP);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&egrDSCP);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&qosprofileindex);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&precedence);	
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&appendIndex);
			dbus_message_iter_next(&iter_array); 
			 
		 switch(actionType)
		 {
			 case 0:strcpy(actype,"Permit");			 break;
			 case 1:strcpy(actype,"Deny");				 break;
			 case 2:strcpy(actype,"Trap"); 		 break;
			 case 3:strcpy(actype,"Permit");	 break;
			 case 4:strcpy(actype,"Redirect");			 break;
			 case 5:strcpy(actype,"Ingress QoS Mark");	 break;
			 case 6:strcpy(actype,"Egress QoS Remark");  break;
			 default: 
					 break; 	 
		 }
		
		switch(packetType)
		 {
			 case 0:strcpy(protype,"IP");		 break;
			 case 1:strcpy(protype,"UDP");		 break;
			 case 2:strcpy(protype,"TCP");		 break;
			 case 3:strcpy(protype,"ICMP"); 	 	break;  
			 case 4:strcpy(protype,"Ethernet");  break;
			 case 5:strcpy(protype,"ARP");	     break; 
			 case 6:strcpy(protype,"All");	     break; 
			 default:
					   break;									 
		 }	 
		
		 if ((packetType==0) && (startIndex && endIndex)) {		 	
				len = ip_long2str(dip,&dipPtr); 
				sprintf(dipPtr+len, "/%d", maskdip);
				for (k = 0; k < (endIndex - startIndex + 1); k ++) {	
					vty_out(vty,"%-12d", startIndex + k);
					
					vty_out(vty,"%-18s", maskdip ? dipPtr:"any");
					
					tmpsip = sip + k;
					len = ip_long2str(tmpsip, &sipPtr);
					sprintf(sipPtr+len, "/%d", 32);
					vty_out(vty,"%-18s", sipPtr);
					
					vty_out(vty,"%-13s", actype);
					memset(sipPtr, 0, 20);

					if(!actionType && policer){
						vty_out(vty,"%-10d \n", policerId);
					}
					else {
						vty_out(vty,"-\n");
					}
				}
				memset(dipPtr, 0, 20);
				continue;
		 }
	 
		}
     }  /*rule not null*/
	 else if(ACL_RETURN_CODE_ERROR == ret) {
		 vty_out(vty,"%% Error: No ACL rule exists.\n");
	 }
 	else if(NPD_DBUS_ERROR_NO_SUCH_PORT ==ret){
		vty_out(vty,"%% Error: illegal port index\n");
	}
	else if(NPD_DBUS_ERROR_NO_SUCH_VLAN +1 ==ret){
		vty_out(vty,"%% Error:illegal vlan index\n");
	}
	vty_out(vty, "===========================ACL COUNT %-5d=========================\n", count);
	dbus_message_unref(reply);
	 return CMD_SUCCESS;
 }

DEFUN(show_ip_acl_index_cmd_func,
	 show_ip_acl_index_cmd,
	 "show ip-acl <1-40000>",
	 SHOW_STR
	 "Show ip-acl rule\n" 
	 "Show all ip-acl rules\n"
 )
 {	 
	 DBusMessage *query = NULL, *reply = NULL;
	 DBusError err;
	 DBusMessageIter  iter;
	 DBusMessageIter  iter_array;
	 char			 actype[50],protype[10];

	 unsigned int acl_Cont = 0;
	 unsigned int ruleIndex=0, showIndex = 0, startIndex = 0, endIndex = 0; 
	 unsigned int ruleType=0;
	 unsigned long dip=0,sip=0, tmpsip = 0, len = 0;
	 unsigned long  maskdip=0, masksip=0;
	 unsigned long srcport=0,dstport=0;
	 unsigned char icmp_code=0,icmp_type=0,code_mask=0,type_mask=0;
	 unsigned long packetType=0;
	 unsigned long actionType=0; 
	 unsigned char dmac[6]={0},smac[6]={0};
	 unsigned int vlanid=0;
	 unsigned char sourceslot=0,sourceport=0,redirectslot=0,redirectport=0,mirrorslot=0,mirrorport=0;
	 unsigned int i=0,j=0,k=0,ret = 0,acl_count=0,type_flag = 0;
	 unsigned char sipBuf[20] = {0};
	 unsigned char dipBuf[20] = {0};  
	 unsigned char *sipPtr = sipBuf,*dipPtr = dipBuf;
	 unsigned int policer = 0,policerId = 0;
	 unsigned int  modifyUP = 0,modifyDSCP = 0;
	 unsigned int  up = 0,dscp = 0,egrUP = 0, egrDSCP = 0;
	 unsigned int qosprofileindex = 0;
	 unsigned char upmm[10] = {0},dscpmm[10] = {0};
	 unsigned int precedence=0, nextheader= 0;
	 unsigned char appendIndex=0;
	 struct ipv6addr dipv6, sipv6;

	 memset(&dipv6, 0, sizeof(struct ipv6addr));
	 memset(&sipv6, 0, sizeof(struct ipv6addr));
	 
	ret = dcli_str2ulong((char*)argv[0],&showIndex);
	if (ACL_RETURN_CODE_ERROR == ret) {
		vty_out(vty,"%% Illegal rule index!\n");
		return CMD_WARNING;
	}	
	 
	 query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_ACL_OBJPATH,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_METHOD_SHOW_ACL);
	 dbus_error_init(&err);
	 reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_acl,query,-1, &err);
	 dbus_message_unref(query);
	 if (NULL == reply) 
	 {
		 /*vty_out(vty,"failed get reply.\n");*/
		 if (dbus_error_is_set(&err)) 
		 {
			 dbus_error_free_for_dcli(&err);
		 }
		 return CMD_SUCCESS;
	 }
	 dbus_message_iter_init(reply,&iter);
 
	 dbus_message_iter_get_basic(&iter,&ret);
 
	 if(0 == ret){
		 dbus_message_iter_next(&iter);  
		 dbus_message_iter_get_basic(&iter,&acl_count);
		 dbus_message_iter_next(&iter); 
		/* dbus_message_iter_get_basic(&iter,&type_flag); 
		// dbus_message_iter_next(&iter);  */
		 dbus_message_iter_recurse(&iter,&iter_array);
				   
		 for (j = 0; j < acl_count; j++) {
			DBusMessageIter iter_struct;
			dbus_message_iter_recurse(&iter_array,&iter_struct);			 
			dbus_message_iter_get_basic(&iter_struct,&ruleIndex);
			dbus_message_iter_next(&iter_struct); 
			dbus_message_iter_get_basic(&iter_struct,&startIndex);
			dbus_message_iter_next(&iter_struct); 
			dbus_message_iter_get_basic(&iter_struct,&endIndex);
			dbus_message_iter_next(&iter_struct); 
			dbus_message_iter_get_basic(&iter_struct,&ruleType);
			dbus_message_iter_next(&iter_struct); 
			dbus_message_iter_get_basic(&iter_struct,&actionType);
			dbus_message_iter_next(&iter_struct);			 
			dbus_message_iter_get_basic(&iter_struct,&packetType);
			dbus_message_iter_next(&iter_struct);			 
			dbus_message_iter_get_basic(&iter_struct,&dip);
			dbus_message_iter_next(&iter_struct);			 
			dbus_message_iter_get_basic(&iter_struct,&maskdip);
			dbus_message_iter_next(&iter_struct);			 
			dbus_message_iter_get_basic(&iter_struct,&sip);
			dbus_message_iter_next(&iter_struct);			 
			dbus_message_iter_get_basic(&iter_struct,&masksip);
			dbus_message_iter_next(&iter_struct);
			/*recv dst ipv6 address*/
			dbus_message_iter_get_basic(&iter_struct,&dipv6.ipbuf[0]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dipv6.ipbuf[1]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dipv6.ipbuf[2]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dipv6.ipbuf[3]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dipv6.ipbuf[4]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dipv6.ipbuf[5]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dipv6.ipbuf[6]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dipv6.ipbuf[7]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dipv6.ipbuf[8]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dipv6.ipbuf[9]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dipv6.ipbuf[10]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dipv6.ipbuf[11]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dipv6.ipbuf[12]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dipv6.ipbuf[13]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dipv6.ipbuf[14]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dipv6.ipbuf[15]);
			dbus_message_iter_next(&iter_struct);
			/*recv sorce ipv6 address*/
			dbus_message_iter_get_basic(&iter_struct,&sipv6.ipbuf[0]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&sipv6.ipbuf[1]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&sipv6.ipbuf[2]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&sipv6.ipbuf[3]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&sipv6.ipbuf[4]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&sipv6.ipbuf[5]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&sipv6.ipbuf[6]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&sipv6.ipbuf[7]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&sipv6.ipbuf[8]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&sipv6.ipbuf[9]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&sipv6.ipbuf[10]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&sipv6.ipbuf[11]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&sipv6.ipbuf[12]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&sipv6.ipbuf[13]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&sipv6.ipbuf[14]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&sipv6.ipbuf[15]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&nextheader);
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&dstport);
			dbus_message_iter_next(&iter_struct);			 
			dbus_message_iter_get_basic(&iter_struct,&srcport);
			dbus_message_iter_next(&iter_struct);			 
			dbus_message_iter_get_basic(&iter_struct,&icmp_code);
			dbus_message_iter_next(&iter_struct);			 
			dbus_message_iter_get_basic(&iter_struct,&icmp_type);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&code_mask);
			dbus_message_iter_next(&iter_struct);			 
			dbus_message_iter_get_basic(&iter_struct,&type_mask);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dmac[0]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dmac[1]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dmac[2]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dmac[3]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dmac[4]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dmac[5]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&smac[0]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&smac[1]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&smac[2]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&smac[3]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&smac[4]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&smac[5]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&vlanid);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&sourceslot);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&sourceport);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&redirectslot);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&redirectport);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&mirrorslot);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&mirrorport);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&policer);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&policerId);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&modifyUP);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&modifyDSCP);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&up);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dscp);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&egrUP);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&egrDSCP);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&qosprofileindex);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&precedence);	
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&appendIndex);
			dbus_message_iter_next(&iter_array); 
			 
		 switch(actionType)
		 {
			 case 0:strcpy(actype,"Permit");			 break;
			 case 1:strcpy(actype,"Deny");				 break;
			 case 2:strcpy(actype,"Trap"); 		 break;
			 case 3:strcpy(actype,"Permit");	 break;
			 case 4:strcpy(actype,"Redirect");			 break;
			 case 5:strcpy(actype,"Ingress QoS Mark");	 break;
			 case 6:strcpy(actype,"Egress QoS Remark");  break;
			 default: 
					 break; 	 
		 }
		
		switch(packetType)
		 {
			 case 0:strcpy(protype,"IP");		 break;
			 case 1:strcpy(protype,"UDP");		 break;
			 case 2:strcpy(protype,"TCP");		 break;
			 case 3:strcpy(protype,"ICMP"); 	 	break;  
			 case 4:strcpy(protype,"Ethernet");  break;
			 case 5:strcpy(protype,"ARP");	     break; 
			 case 6:strcpy(protype,"All");	     break; 
			 default:
					   break;									 
		 }	 

		 if ((packetType==0) && (startIndex && endIndex)) {
				for (k = 0; k < (endIndex - startIndex + 1); k ++) {	
					if (showIndex == (startIndex+k)) {
						vty_out(vty,"===================================================================\n");
						vty_out(vty,"%-12s%-18s%-18s%-13s%-10s\n","acl-id","destination-ip" ,"source-ip", "action", "policer-id");
						vty_out(vty,"%-12d", startIndex + k);
						
						vty_out(vty,"%-18s", maskdip ? dipPtr:"any");
						
						tmpsip = sip + k;
						len = ip_long2str(tmpsip, &sipPtr);
						sprintf(sipPtr+len, "/%d", 32);
						vty_out(vty,"%-18s", sipPtr);
						
						vty_out(vty,"%-13s", actype);
						memset(sipPtr, 0, 20);
						
						if(!actionType && policer){
							vty_out(vty,"%-10d \n", policerId);
						}
						else {
							vty_out(vty,"-\n");
						}
						vty_out(vty,"===================================================================\n");
					}
				}
				continue;
		 }
	 
		}
     }  /*rule not null*/
	
	 else if(ACL_RETURN_CODE_ERROR == ret) {
		 vty_out(vty,"%% Error: No ACL rule exists.\n");
	 }
 	else if(NPD_DBUS_ERROR_NO_SUCH_PORT ==ret){
		vty_out(vty,"%% Error: illegal port index\n");
	}
	else if(NPD_DBUS_ERROR_NO_SUCH_VLAN +1 ==ret){
		vty_out(vty,"%% Error:illegal vlan index\n");
	}
	
	dbus_message_unref(reply);
	 return CMD_SUCCESS;
 }

DEFUN(delete_acl_cmd_func,
      delete_acl_cmd,
      "delete acl INDEX",
      "Delete configuration \n"
      "Delete acl \n"
      "Standard acl index range in 1-1000,extended acl index range in 1-500\n"
      )
{
      DBusMessage 	*query = NULL, *reply = NULL;
	  DBusError 	 err;
	  DBusMessageIter  iter;
	  DBusMessageIter  iter_array;
	  unsigned int  ruleIndex = 0,op_ret = 0;
	  unsigned int   group_num = 0,j=0,count=0;
	  unsigned char   slot_no = 0,local_port_no = 0;
	  int ret = 0;
	  
	  ret = dcli_str2ulong((char*)argv[0],&ruleIndex);
	  if (ACL_RETURN_CODE_ERROR == ret) {
			vty_out(vty,"%% Illegal rule index!\n");
			return CMD_WARNING;
		}
	  ruleIndex = ruleIndex-1;

      query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_ACL_OBJPATH,NPD_DBUS_ACL_INTERFACE, NPD_DBUS_METHOD_DELETE_ACL);
	  dbus_error_init(&err);
	  dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&ruleIndex,
							 DBUS_TYPE_INVALID);
      reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_acl,query,-1, &err);
	  dbus_message_unref(query);
	  if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	 }
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_UINT32,&group_num,
		DBUS_TYPE_INVALID))
	{						
		if (ACL_RETURN_CODE_ERROR == op_ret ) {
			vty_out(vty,"%% Range range error\n");
		}
		else if(op_ret==ACL_RETURN_CODE_GLOBAL_NOT_EXISTED)	{
			vty_out(vty,"%% Rule %d not existed!\n",(ruleIndex+1));
		}
		else if(op_ret == ACL_RETURN_CODE_GROUP_RULE_EXISTED){
			vty_out(vty,"%% Can't delete this acl since it is bound to ingress group %d\n",group_num);
		}	
		else if(op_ret ==ACL_RETURN_CODE_EGRESS_GROUP_RULE_EXISTED){
			vty_out(vty,"%% Can't delete this acl since it is bound to egress group %d\n",group_num);
		}
		else if(ACL_RETURN_CODE_RULE_INDEX_ERROR==op_ret){
			vty_out(vty,"%% Acl rule range should be 1-1000!\n");
		}
		else if(ACL_RETURN_CODEL_MIRROR_USE==op_ret){
			vty_out(vty,"%% Can't delete this acl since it used by mirror function!\n");
		}
	}
	else 
	{
		/*vty_out(vty,"Failed get args.\n");*/
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;	
}

DEFUN(config_ethport_acl_enable_func,
	  config_ethport_acl_enable_cmd,
	  "(ingress|egress) acl (enable|disable)",
	  "Configure ingress acl service\n"
	  "Configure egress acl service\n"
	  "Interface acl service\n"
	  "Enable acl service in the interface\n"
	  "Disable acl service in the interface\n"
)
{	
	DBusMessage *query, *reply;
	DBusError err;
	boolean isEnable = FALSE;
	unsigned int g_index = 0,EnableInfo = 0,temp_info = 0,op_ret = 0;
	unsigned int node_flag = 0,dir_info = 0;
	unsigned int slot_no = 0;

	if(ETH_PORT_NODE==vty->node){
		node_flag = 0;
		g_index = (unsigned int)(vty->index);
	}
	else if(VLAN_NODE==vty->node){
		node_flag = 1;
		g_index = (unsigned int)(vty->index);
	}
	
	if(strncmp("ingress",argv[0],strlen(argv[0]))==0){
		dir_info = 0;
	}
	else if (strncmp("egress",argv[0],strlen(argv[0]))==0){
		dir_info = 1;
	}	
	else{
		vty_out(vty,"bad command parameter!\n");
		return CMD_WARNING;
	}
	
	if(strncmp("enable",argv[1],strlen(argv[1]))==0){
		isEnable = TRUE;
	}
	else if (strncmp("disable",argv[1],strlen(argv[1]))==0){
		isEnable = FALSE;
	}
	else{
		vty_out(vty,"bad command parameter!\n");
		return CMD_WARNING;
	}	
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_ETHPORTS_OBJPATH,NPD_DBUS_ETHPORTS_INTERFACE,NPD_DBUS_ETHPORTS_METHOD_CONFIG_ACL);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
						     DBUS_TYPE_UINT32,&dir_info,
							 DBUS_TYPE_UINT32,&node_flag,
							 DBUS_TYPE_UINT32,&g_index,
							 DBUS_TYPE_BYTE,&isEnable,
							 DBUS_TYPE_INVALID);
	/*vty_out(vty,"node_flag %d\n",node_flag);
	//vty_out(vty,"g_index %d\n",g_index);
	//vty_out(vty,"isEnable %d\n",isEnable);*/
	if(is_distributed == DISTRIBUTED_SYSTEM)
    {
		SLOT_PORT_ANALYSIS_SLOT(g_index, slot_no);

    	if(NULL == dbus_connection_dcli[slot_no]->dcli_dbus_connection) 				
    	{
			vty_out(vty, "Connection to slot%d is not exist.\n", slot_no);
			return CMD_WARNING;
    	}
		else 
    	{
			dcli_dbus_connection_acl_port = dbus_connection_dcli[slot_no]->dcli_dbus_connection;
			vty_out(vty, "%s %s acl of port on slot %d.\n",(isEnable == TRUE)?"Enable":"Disable",  \
				(dir_info == 0)?"ingress":"egress",slot_no);
		}
    }
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_acl_port,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&EnableInfo,
		DBUS_TYPE_UINT32, &op_ret,
		DBUS_TYPE_INVALID)) {
			if(node_flag==1){
				if(op_ret==NPD_DBUS_ERROR_NO_SUCH_VLAN +1)
					vty_out(vty,"%% Error!vlan illegal!\n");
				else if(op_ret==NPD_VLAN_NOTEXISTS)
					vty_out(vty,"%% Error!vlan %d not existed!\n",g_index);
				else if(op_ret == ACL_RETURN_CODE_GROUP_VLAN_BINDED)
					vty_out(vty,"%% Error!please unbind acl group first!\n");
				else if(op_ret==ACL_RETURN_CODE_BCM_DEVICE_NOT_SUPPORT)
					vty_out(vty,"%% device not support egress and ingress acl group binded by vlan\n");			
				else if(op_ret==ACL_RETURN_CODE_UDP_VLAN_RULE_ENABLE)
					vty_out(vty,"%% vlan have udp rules for dhcpsnooping so can not disable acl service\n");
				else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret)
					vty_out(vty,"%% Product not support this function!\n");
				else if(EnableInfo==NPD_DBUS_ERROR_NO_SUCH_PORT)
					vty_out(vty,"%% Error!vlan %d exists some illegal port!\n",g_index);
				else if(EnableInfo!=ACL_RETURN_CODE_SUCCESS)
					vty_out(vty,"%% Error!Vlan enable fail!\n");				
			}
			else if(node_flag==0){
				if(op_ret==NPD_DBUS_ERROR_NO_SUCH_PORT)
					vty_out(vty,"%% Error! No such port\n");
				else if (ACL_PORT_NOT_SUPPORT_BINDED == op_ret)
					vty_out(vty, "%% Port not support bind acl group.\n");
				else if(op_ret == ACL_RETURN_CODE_GROUP_PORT_BINDED)
					vty_out(vty,"%% Error!please unbind acl group first!\n");
				else if (ACL_PORT_NOT_SUPPORT_BINDED == op_ret)
					vty_out(vty, "%% Port not support bind acl group.\n");
				else if(op_ret==ACL_RETURN_CODE_BCM_DEVICE_NOT_SUPPORT)
					vty_out(vty,"%% device not support egress acl group binded by port\n");
				else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret)
					vty_out(vty,"%% Product not support this function!\n");
				else if(EnableInfo!=ACL_RETURN_CODE_SUCCESS)
					vty_out(vty,"%% Error!Port enable fail!\n");
			}
		
	} else {
		/*vty_out(vty,"Failed get args.\n");*/
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
	
}

DEFUN(add_delete_acl_rule_to_acl_group_func,
		add_delete_acl_rule_to_acl_group_cmd,
		"(add|delete) acl <1-1000>",
		"Add acl rule to acl group\n"
		"Delete acl rule from acl group\n"
		"Add or delete standard or extended access list\n"
		"Specify standard rule range in 1-1000,extended rule range in 1-500\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int     ruleIndex = 0 ;
	unsigned int      ifIndex = 0;
	unsigned int      op_ret = 0,op_ret1 = 0,op_ret2 = 0;
	unsigned int	  acl_group_num = 0;
	unsigned int      group_inf = 0,ret,num = 0;
	unsigned int      op_flag = 0,op_info = 0,dir_info = 0,exist_dir = 0;

	if(0==strncmp("add",argv[0],strlen(argv[0])))
	   op_flag = 0;
	else if(0==strncmp("delete",argv[0],strlen(argv[0])))
	   op_flag = 1;
	
	ret = dcli_str2ulong((char*)argv[1],&ruleIndex);
	if (ACL_RETURN_CODE_ERROR == ret) {
		vty_out(vty,"%% Illegal rule index!\n");
		return CMD_WARNING;
	}
	ruleIndex = ruleIndex-1;

	if(ACL_GROUP_NODE==(vty->node)){
		acl_group_num = (unsigned int)(vty->index);
		dir_info = 0; /*ingress*/
	}
	else if(ACL_EGRESS_GROUP_NODE==(vty->node)){
		acl_group_num = (unsigned int)(vty->index);
		dir_info = 1; /*egress*/
	}
	else{
		vty_out(vty,"%% unknown config mode\n");
		return CMD_WARNING;
	}
	/*vty_out(vty,"config acl_group %d\n",acl_group_num);*/
		
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_ACL_OBJPATH,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_METHOD_ADD_ACL_TO_GROUP);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&dir_info,
						     DBUS_TYPE_UINT32,&op_flag,
							 DBUS_TYPE_UINT32,&acl_group_num,
							 DBUS_TYPE_UINT32,&ruleIndex,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_acl,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&group_inf,
		DBUS_TYPE_UINT32,&ret,
		DBUS_TYPE_UINT32,&num,
		DBUS_TYPE_UINT32,&op_info,
		DBUS_TYPE_INVALID)) {
		
			if(group_inf==ACL_RETURN_CODE_ERROR)	
		 		vty_out(vty,"%% sorry,you should creat a acl group firstly!\n");
			else if(op_flag==0) {
				if((ACL_RETURN_CODE_GROUP_RULE_EXISTED==ret)&&(0!=num))					
					vty_out(vty,"%% rule %d has been added in ingress group %d!\n",(ruleIndex+1),num);
				else if((ACL_RETURN_CODE_EGRESS_GROUP_RULE_EXISTED==ret)&&(0!=num))
					vty_out(vty,"%% rule %d has been added in egress group %d!\n",(ruleIndex+1),num);									
				else if(ACL_RETURN_CODE_GLOBAL_NOT_EXISTED==ret)
					vty_out(vty,"%% acl %d has not been set!\n",(ruleIndex+1));
				else if(ACL_RETURN_CODE_ADD_EQUAL_RULE == op_info)
					vty_out(vty,"%% you add the equal rule in group !\n");
				else if (ACL_RETURN_CODE_ALREADY_PORT == op_info)
					vty_out(vty, "%% Qos mode is not flow, please change mode \n");
				else if (op_ret == ACL_RETURN_CODE_NO_QOS_MODE)
					vty_out(vty, "%% There is no qos mode, please config qos mode \n");
				else if(ACL_RETURN_CODE_GROUP_EGRESS_ERROR==op_info)
					vty_out(vty,"%% ingress port not support egress qos rule !\n");
				else if(ACL_RETURN_CODE_GROUP_EGRESS_NOT_SUPPORT==op_info)
					vty_out(vty,"%% egress port not support such action,including trap ,mirror,redirect,ingress qos!\n");
				else if(ACL_RETURN_CODE_STD_RULE == op_info)
					vty_out(vty,"%% added in the right group !\n");
				else if(ACL_RETURN_CODE_SUCCESS!=op_info)
					vty_out(vty,"%% add fail!\n");
			}
			else if(op_flag==1) {
				if(ACL_RETURN_CODE_GLOBAL_NOT_EXISTED==ret)
					vty_out(vty,"%% acl %d has not been set!\n",(ruleIndex+1));
				else if(ACL_RETURN_CODE_GROUP_RULE_EXISTED==ret){
					if(num!=acl_group_num){						
						if(dir_info==ACL_DIRECTION_INGRESS)
							vty_out(vty,"%% ingress group %d has no rule %d\n",acl_group_num,(ruleIndex+1));
						else if(dir_info==ACL_DIRECTION_EGRESS)
							vty_out(vty,"%% egress group %d has no rule %d\n",acl_group_num,(ruleIndex+1));
			
					}
					else if(num==acl_group_num) {
						if(ACL_RETURN_CODE_SUCCESS != op_info)
							vty_out(vty,"%% ingress delete fail!\n");
					}	
				}
				else if(ACL_RETURN_CODE_EGRESS_GROUP_RULE_EXISTED==ret){
					if(num!=acl_group_num)	{					
						if(dir_info==ACL_DIRECTION_INGRESS)
							vty_out(vty,"%% ingress group %d has no rule %d\n",acl_group_num,(ruleIndex+1));
						else if(dir_info==ACL_DIRECTION_EGRESS)
							vty_out(vty,"%% egress group %d has no rule %d\n",acl_group_num,(ruleIndex+1));			
					}
					else if(num==acl_group_num){
						if(ACL_RETURN_CODE_SUCCESS!=op_info)
							vty_out(vty,"%% egress delete fail!\n");
					}	
				}
				else if(ACL_RETURN_CODE_SUCCESS==ret){
					if(dir_info==ACL_DIRECTION_INGRESS)
						vty_out(vty,"%% ingress group %d has no this rule %d\n",acl_group_num,(ruleIndex+1));
					else if(dir_info==ACL_DIRECTION_EGRESS)
						vty_out(vty,"%% egress group %d has no this rule %d\n",acl_group_num,(ruleIndex+1));						
				}			
			}
			else if(ACL_RETURN_CODE_RULE_INDEX_ERROR==ret){
				vty_out(vty,"%% Acl rule range should be 1-1000!\n");
		}
	 }

	else {
		/*vty_out(vty,"Failed get args.\n");*/
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;

}

DEFUN(show_bind_ethport_acl_Info_func,
	  show_bind_ethport_acl_Info_cmd,
	  "show (ingress|egress) acl",
	  SHOW_STR
	  "Ingress acl rule\n"
	  "Egress acl rule\n"
	  "Show bind acl rule info\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	unsigned int g_index = 0,port_info = 0,rule_info = 0;
	unsigned int slot_no=0,local_port_no=0;
	unsigned int j=0,count=0,group_num=0;
	unsigned int ruleIndex=0,ret = 0;
	unsigned int node_flag = 0,dir_info = 0;
	unsigned int portState=ACL_RETURN_CODE_SUCCESS;
	unsigned int slot_id=0;

	if(ETH_PORT_NODE==vty->node){
		node_flag = 0;
		g_index = (unsigned int)(vty->index);
	}
	else if(VLAN_NODE==vty->node){
		node_flag = 1;
		g_index = (unsigned int)(vty->index);
	}
	if(strncmp("ingress",argv[0],strlen(argv[0]))==0){
		dir_info = 0;
	}
	else if (strncmp("egress",argv[0],strlen(argv[0]))==0){
		dir_info = 1;
	}
	else{
		vty_out(vty,"bad command parameter!\n");
		return CMD_WARNING;
	}	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
		NPD_DBUS_ETHPORTS_OBJPATH,NPD_DBUS_ETHPORTS_INTERFACE,NPD_DBUS_ETHPORTS_METHOD_SHOW_BIND_ACL);
	dbus_error_init(&err);

	dbus_message_append_args(query,
						     DBUS_TYPE_UINT32,&dir_info,
							 DBUS_TYPE_UINT32,&node_flag,
							 DBUS_TYPE_UINT32,&g_index,
							 DBUS_TYPE_INVALID);
	if(is_distributed == DISTRIBUTED_SYSTEM)
    {
		SLOT_PORT_ANALYSIS_SLOT(g_index, slot_id);

    	if(NULL == dbus_connection_dcli[slot_id]->dcli_dbus_connection) 				
    	{
			vty_out(vty, "Connection to slot%d is not exist.\n", slot_id);
			return CMD_WARNING;
    	}
		else 
    	{
			dcli_dbus_connection_acl_port = dbus_connection_dcli[slot_id]->dcli_dbus_connection;
			vty_out(vty, "Show %s acl of port on slot %d.\n",(dir_info == 0)?"ingress":"egress",slot_id);
		}
    }	

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_acl_port,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		/*vty_out(vty,"failed get reply.\n");*/
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	/*recept*/
	if(node_flag==0){
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&ret);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&portState);	

		if(NPD_DBUS_ERROR_NO_SUCH_PORT ==ret){
			vty_out(vty,"Error:illegal port index!\n");
		}
		else if(ACL_RETURN_CODE_ON_PORT_DISABLE==portState){
				vty_out(vty,"%% Since port state is disabled,cannot see the details\n");
		}
		else if(ACL_RETURN_CODE_GROUP_PORT_NOTFOUND==ret) {
			if(dir_info ==0)
			     vty_out(vty,"%% Error: No ingress acl group infomation on ethport!\n");
			else if(dir_info ==1)
			     vty_out(vty,"%% Error: No egress acl group infomation on ethport!\n");
		}

		else if(ACL_RETURN_CODE_SUCCESS == ret){
					
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&slot_no);
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&local_port_no);
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&group_num);
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&count);
			dbus_message_iter_next(&iter);			
			dbus_message_iter_recurse(&iter,&iter_array);
			
			vty_out(vty,"===============================================\n");
			vty_out(vty,"%-40s: %d/%d\n","Interface-Ethernet",slot_no,local_port_no);
		    if(dir_info ==0){
				vty_out(vty,"%-40s: %d\n","ingress acl group",group_num);}
			else if(dir_info ==1){
				vty_out(vty,"%-40s: %d\n","egress acl group",group_num);}				

			vty_out(vty,"%-40s: %d\n","acl count",count);
			
		    
			for (j = 0; j < count; j++) {
				DBusMessageIter iter_struct;
				dbus_message_iter_recurse(&iter_array,&iter_struct);			
				dbus_message_iter_get_basic(&iter_struct,&ruleIndex);
				dbus_message_iter_next(&iter_array); 
				
				vty_out(vty,"%-40s: %d\n","acl index",ruleIndex);
				}
			vty_out(vty,"===============================================\n");
		
					
		}
		else if(ACL_RETURN_CODE_GROUP_RULE_NOTEXISTED == ret)
		{			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&slot_no);
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&local_port_no);
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&group_num);
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&count);
			dbus_message_iter_next(&iter);
			dbus_message_iter_recurse(&iter,&iter_array);
			
			vty_out(vty,"===============================================\n");
			vty_out(vty,"%-40s: %d/%d\n","Interface-Ethernet",slot_no,local_port_no);
			if(dir_info ==0){
				vty_out(vty,"%-40s: %d\n","ingress acl group",group_num);}
			else if(dir_info ==1){
				vty_out(vty,"%-40s: %d\n","egress acl group",group_num);}	
			vty_out(vty,"%-40s: %s\n","acl index","null");
			vty_out(vty,"===============================================\n");
		
		
		}
	} /*port*/
	else if(node_flag==1){
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&ret);
		dbus_message_iter_next(&iter);			
		dbus_message_iter_get_basic(&iter,&portState);		
		if(NPD_DBUS_ERROR_NO_SUCH_VLAN +1 == ret) {
			vty_out(vty,"Error: vlan illegal!\n");
		}
		else if(NPD_VLAN_NOTEXISTS==ret){
			vty_out(vty,"Error: vlan not existed\n");
		}
		else if(ACL_RETURN_CODE_ON_PORT_DISABLE==portState){
				vty_out(vty,"%% Since vlan state is disabled,cannot see the details\n");
		}
		else if(ACL_RETURN_CODE_ERROR == ret) {
			if(dir_info ==0)
			     vty_out(vty,"%% Error: No ingress acl group infomation in vlan!\n");
			if(dir_info ==1)
			     vty_out(vty,"%% Error: No egress acl group infomation in vlan!\n");	
		}
		else if(ACL_RETURN_CODE_SUCCESS == ret){
			dbus_message_iter_next(&iter);			
			dbus_message_iter_get_basic(&iter,&group_num);
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&count);
			dbus_message_iter_next(&iter);			
			dbus_message_iter_recurse(&iter,&iter_array);
		
			vty_out(vty,"===============================================\n");
			vty_out(vty,"%-40s: %d\n","Vlan Id",g_index);
			vty_out(vty,"%-40s: %d\n","bind acl group",group_num);
			vty_out(vty,"%-40s: %d\n","acl count",count);
							
			for (j = 0; j < count; j++) {
				DBusMessageIter iter_struct;
				dbus_message_iter_recurse(&iter_array,&iter_struct);			
				dbus_message_iter_get_basic(&iter_struct,&ruleIndex);
				dbus_message_iter_next(&iter_array); 
				
				vty_out(vty,"%-40s: %d\n","acl index",ruleIndex);
				}
			vty_out(vty,"===============================================\n");			
		}
		else if(ACL_RETURN_CODE_GROUP_RULE_NOTEXISTED == ret) {
			dbus_message_iter_next(&iter);			
			dbus_message_iter_get_basic(&iter,&group_num);
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&count);
			dbus_message_iter_next(&iter);			
			dbus_message_iter_recurse(&iter,&iter_array);
			
			vty_out(vty,"===============================================\n");
			vty_out(vty,"%-40s: %d\n","Vlan Id",g_index);
			vty_out(vty,"%-40s: %d\n","bind acl group",group_num);
			vty_out(vty,"%-40s: %s\n","acl index","null");
			vty_out(vty,"===============================================\n");			
		}
	}
		dbus_message_unref(reply);
		return CMD_SUCCESS;
}

DEFUN(bind_acl_group_to_ethport_func,
	  bind_acl_group_to_ethport_cmd,
	  "bind (ingress|egress) acl-group <1-1023>",
	  "Bind configuration\n"
	  "Ingress acl group\n"
	  "Egress acl group\n"
	  "Bind acl-group to port\n"
	  "Acl-group num range in 1-1023\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;

	unsigned int g_index = 0,sw_info=0,cfg_info=0;
	unsigned int group_num = 0,group_info = 0;
	unsigned int IsEnable=0;  
	unsigned int node_flag = 0,dir_info=0;
	unsigned int slot_no = 0;
	int ret = 0;

	if(ETH_PORT_NODE==vty->node){
		node_flag = 0;
		g_index = (unsigned int)(vty->index);
	}
	else if(VLAN_NODE==vty->node){
		node_flag = 1;
		g_index = (unsigned int)(vty->index);
	}
	if(strncmp("ingress",argv[0],strlen(argv[0]))==0){
		dir_info = 0;
	}
	else if (strncmp("egress",argv[0],strlen(argv[0]))==0){
		dir_info = 1;
	}	
	else{
		vty_out(vty,"%% bad command parameter!\n");
		return CMD_WARNING;
	}
	
	ret = dcli_str2ulong((char*)argv[1],&group_num);
	if (ACL_RETURN_CODE_ERROR == ret) {
		vty_out(vty,"%% Illegal group number!\n");
		return CMD_WARNING;
	}	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_ETHPORTS_OBJPATH,NPD_DBUS_ETHPORTS_INTERFACE,NPD_DBUS_ETHPORTS_METHOD_BIND_ACL_GROUP);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&dir_info,
							 DBUS_TYPE_UINT32,&node_flag,
							 DBUS_TYPE_UINT32,&g_index,
							 DBUS_TYPE_UINT32,&group_num,						 
							 DBUS_TYPE_INVALID);
	if(is_distributed == DISTRIBUTED_SYSTEM)
    {
		SLOT_PORT_ANALYSIS_SLOT(g_index, slot_no);

    	if(NULL == dbus_connection_dcli[slot_no]->dcli_dbus_connection) 				
    	{
			vty_out(vty, "Connection to slot%d is not exist.\n", slot_no);
			return CMD_WARNING;
    	}
		else 
    	{
			dcli_dbus_connection_acl_port = dbus_connection_dcli[slot_no]->dcli_dbus_connection;
			vty_out(vty, "Bind %s acl-group of port on slot %d.\n",(dir_info == 0)?"ingress":"egress",slot_no);
		}
    }

	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_acl_port,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	
	if (dbus_message_get_args ( reply, &err,			
			DBUS_TYPE_UINT32, &sw_info,
		       DBUS_TYPE_UINT32, &cfg_info,
		       DBUS_TYPE_UINT32, &IsEnable,
			DBUS_TYPE_INVALID)) {
			/*vty_out(vty,"node flag %d\n",node_flag);
			//vty_out(vty,"sw_info %d\n",sw_info);
			//vty_out(vty,"cfg_info %d\n",cfg_info);
			//vty_out(vty,"IsEnable %d\n",IsEnable);*/
			  if(node_flag==0){
			  	
			  	 	if(ACL_RETURN_CODE_ERROR==IsEnable){
						vty_out(vty,"%% port should be enabled acl service firstly!\n");
						/*
						if(dir_info==ACL_DIRECTION_INGRESS)
							vty_out(vty,"%% port should be enabled ingress acl service firstly!\n");
						else if(dir_info==ACL_DIRECTION_EGRESS)
							vty_out(vty,"%% port should be enabled egress acl service firstly!\n");
						*/
					}

					else if (NPD_DBUS_ERROR_NO_SUCH_PORT == sw_info) {
						vty_out(vty,"%% Error: No such port.\n");
					}				
					else if (ACL_PORT_NOT_SUPPORT_BINDED == sw_info) {
						vty_out(vty, "%% Port not support bind acl group.\n");
					}
					else if(ACL_RETURN_CODE_GROUP_NOT_EXISTED == sw_info){
						vty_out(vty,"%% Acl group %d not existed on slot %d!\n",group_num,slot_no);
					}
					else if (ACL_PORT_NOT_SUPPORT_BINDED == sw_info) {
						vty_out(vty, "%% Port not support bind acl group.\n");
					}
					else if (ACL_RETURN_CODE_BCM_DEVICE_NOT_SUPPORT == sw_info) {
						vty_out(vty, "%% device not support acl group and policy-map binded in the same port \n");
					}
					else if(ACL_RETURN_CODE_BCM_DEVICE_NOT_SUPPORT == sw_info){
						vty_out(vty,"%% device not support egress acl group and ingress acl group binded by vlan\n");
					}
					else if(ACL_RETURN_CODE_ERROR == sw_info){
						vty_out(vty,"%% bind fail!\n");
					}
						
					else if (ACL_RETURN_CODE_GROUP_PORT_BINDED ==sw_info) {
						vty_out(vty,"%% Port has binded acl group yet!\n");		
					}
					else if (ACL_RETURN_CODE_SUCCESS==cfg_info){
						/*vty_out(vty,"successfully bind\n");*/
					}
										
					else if(ACL_RETURN_CODE_NPD_DBUS_ERR_GENERAL==cfg_info){
						vty_out(vty,"%% setting port's configure table fail\n");
					}
					
					
			  	}
			  if(node_flag==1){

					if(ACL_RETURN_CODE_ERROR==IsEnable){
						vty_out(vty,"%% vlan should be enabled acl service firstly!\n");
					}
					else if (NPD_DBUS_ERROR_NO_SUCH_VLAN +1  == sw_info) {
							vty_out(vty,"%% ERROR: vlan illegal\n");
					}
					else if(NPD_VLAN_NOTEXISTS==sw_info){
							vty_out(vty,"%% Error:the vlan not existed\n");
					}
				   else if(ACL_RETURN_CODE_GROUP_NOT_EXISTED == sw_info){
						vty_out(vty,"%% Acl group %d not existed!\n",group_num);
					}
					else if(ACL_RETURN_CODE_ERROR == sw_info){
						vty_out(vty,"%% Bind fail!\n");
					}
						
					else if (ACL_RETURN_CODE_GROUP_VLAN_BINDED ==sw_info) {
						vty_out(vty,"%% Vlan has binded acl group yet!\n");		
					}
					else if (ACL_RETURN_CODE_SUCCESS==cfg_info){
						/*vty_out(vty,"successfully bind\n");*/
					}
										
					else if(ACL_RETURN_CODE_NPD_DBUS_ERR_GENERAL==cfg_info){
						vty_out(vty,"%% setting ports in vlan's configure table fail\n");
					}
					
			  }	
			   if(ACL_RETURN_CODE_GROUP_INDEX_ERROR==sw_info)
			     vty_out(vty,"%% Acl group range should be 1-1023!\n");
			  
		} else {
			/*vty_out(vty,"Failed get args.\n");*/
			if (dbus_error_is_set(&err)) {
				dbus_error_free_for_dcli(&err);
			}
		}
		dbus_message_unref(reply);
		return CMD_SUCCESS;
		
}
DEFUN(no_acl_group_on_ethport_func,
	  no_acl_group_on_ethport_cmd,
	  "unbind (ingress|egress) acl-group <1-1023>",
	  "Unbind configuration \n"
	  "Ingress acl group\n"
	  "Egress acl group\n"
	  "Cancel acl group on the ethport\n"
	  "Acl group index <1-1023>\n"
)
{
	
	DBusMessage *query, *reply;
	DBusError err;

	unsigned int g_index = 0,port_info = 0;
	unsigned int group_num = 0,sw_info = 0,cfg_info = 0;
	unsigned int node_flag = 0,dir_info=0;
	unsigned int slot_no = 0;
	int ret = 0;
	
	if(ETH_PORT_NODE==vty->node){
		node_flag = 0;
		g_index = (unsigned int)(vty->index);
	}
	else if(VLAN_NODE==vty->node){
		node_flag = 1;
		g_index = (unsigned int)(vty->index);
	}

	if(strncmp("ingress",argv[0],strlen(argv[0]))==0){
		dir_info = 0;
	}
	else if (strncmp("egress",argv[0],strlen(argv[0]))==0){
		dir_info = 1;
	}
	else{
		vty_out(vty,"bad command parameter!\n");
		return CMD_WARNING;
	}
	ret = dcli_str2ulong((char*)argv[1],&group_num);
	if (ACL_RETURN_CODE_ERROR == ret) {
		vty_out(vty,"%% Illegal group number!\n");
		return CMD_WARNING;
	}	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_ETHPORTS_OBJPATH,NPD_DBUS_ETHPORTS_INTERFACE,NPD_DBUS_ETHPORTS_METHOD_DELETE_ACL_GROUP);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&dir_info,
							 DBUS_TYPE_UINT32,&node_flag,
 						     DBUS_TYPE_UINT32,&g_index,
							 DBUS_TYPE_UINT32,&group_num,
							 DBUS_TYPE_INVALID);

	if(is_distributed == DISTRIBUTED_SYSTEM)
    {
		SLOT_PORT_ANALYSIS_SLOT(g_index, slot_no);

    	if(NULL == dbus_connection_dcli[slot_no]->dcli_dbus_connection) 				
    	{
			vty_out(vty, "Connection to slot%d is not exist.\n", slot_no);
			return CMD_WARNING;
    	}
		else 
    	{
			dcli_dbus_connection_acl_port = dbus_connection_dcli[slot_no]->dcli_dbus_connection;
			vty_out(vty, "Unbind %s acl-group of port on slot %d.\n",(dir_info == 0)?"ingress":"egress", slot_no);
		}
    }
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_acl_port,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	
	if (dbus_message_get_args ( reply, &err,
		    DBUS_TYPE_UINT32, &sw_info,
			DBUS_TYPE_UINT32, &cfg_info,
			DBUS_TYPE_INVALID)) {
			  if(node_flag==0){
					if (NPD_DBUS_ERROR_NO_SUCH_PORT == sw_info) {
						vty_out(vty,"%% Error: No such port.\n");
					}

					else if(ACL_RETURN_CODE_GROUP_PORT_NOTFOUND  == sw_info){
						vty_out(vty,"%% port has no acl group info\n");
					}	
					else if(ACL_RETURN_CODE_GROUP_NOT_EXISTED ==sw_info){
						vty_out(vty,"%% wrong group index on port!\n");
					}
					else if(ACL_RETURN_CODE_ENABLE_FIRST ==sw_info){
						vty_out(vty,"%% port should be enabled acl service firstly!\n");
					}
					
					else if(ACL_RETURN_CODE_SUCCESS != cfg_info){
						vty_out(vty,"%%  unbind cfg table fail!\n");
					}
					else if((ACL_RETURN_CODE_SUCCESS == cfg_info)&&(sw_info==ACL_RETURN_CODE_SUCCESS)){
						/*vty_out(vty,"unbind success!\n");*/
					}
									
			  	}
			  else if(node_flag==1){
				   if (NPD_DBUS_ERROR_NO_SUCH_VLAN +1 == sw_info) {
						vty_out(vty,"%% Error!illegal vlan\n");
					}
					else if (NPD_VLAN_NOTEXISTS == sw_info) {
						vty_out(vty,"%% Error! vlan %d not existed\n",g_index);
					}
					else if(ACL_RETURN_CODE_ENABLE_FIRST ==sw_info){
						vty_out(vty,"%% vlan should be enabled acl service firstly!\n");
					}
					else if(ACL_RETURN_CODE_GROUP_NOT_BINDED == sw_info){
						vty_out(vty,"%% vlan has no acl group info\n");
					}	
					else if(ACL_RETURN_CODE_GROUP_WRONG_INDEX ==sw_info){
						vty_out(vty,"%% wrong group index on vlan!\n");
					}
					else if(ACL_RETURN_CODE_SUCCESS != cfg_info){
						vty_out(vty,"%% unbind hardware information fail!\n");
					}			
					else if((ACL_RETURN_CODE_SUCCESS == cfg_info)&&(sw_info==ACL_RETURN_CODE_SUCCESS)){
						/*vty_out(vty,"unbind success!\n");*/
					}
			  }
			  if(ACL_RETURN_CODE_GROUP_INDEX_ERROR==sw_info)
			     vty_out(vty,"%% Acl group range should be 1-1023!\n");
			
		} else {
			/*vty_out(vty,"Failed get args.\n");*/
			if (dbus_error_is_set(&err)) {
				dbus_error_free_for_dcli(&err);
			}
		}
		dbus_message_unref(reply);
		return CMD_SUCCESS;
}
DEFUN(create_acl_group_cmd_func,
	  create_acl_group_cmd,
	  "create (ingress|egress) acl-group <1-1023>",
	  "Create configuration\n"
	  "Create ingress group\n"
	  "Create egress group\n"
	  "Create acl-group\n"
	  "Acl-group range in 1-1023\n"
)
{
	
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int groupNum = 0,op_ret = 0;	
	unsigned int dir_info=0;
	int ret = 0;
	
	if(strncmp("ingress",argv[0],strlen(argv[0]))==0){
		dir_info = 0;
	}
	else if (strncmp("egress",argv[0],strlen(argv[0]))==0){
		dir_info = 1;
	}	
	else{
		vty_out(vty,"bad command parameter!\n");
		return CMD_WARNING;
	}
	ret = dcli_str2ulong((char*)argv[1],&groupNum);
	if (ACL_RETURN_CODE_ERROR == ret) {
		vty_out(vty,"%% Illegal group number!\n");
		return CMD_WARNING;
	}	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_ACL_OBJPATH,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_METHOD_CREATE_ACL_GROUP);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,		
						     DBUS_TYPE_UINT32,&dir_info,
							 DBUS_TYPE_UINT32,&groupNum,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_acl,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)) {
			if (ACL_RETURN_CODE_SUCCESS == op_ret ) {
				/*vty_out(vty,"creat acl group %d successfully!\n",groupNum);*/
				
			}
			else if(ACL_RETURN_CODE_ERROR == op_ret ){
				if(dir_info==0)
					vty_out(vty,"%% Ingress acl group %d has been created!\n",groupNum);
				else if(dir_info==1)
					vty_out(vty,"%% Egress acl group %d has been created!\n",groupNum);
			}
			else if (ACL_RETURN_CODE_BCM_DEVICE_NOT_SUPPORT == op_ret) {
				vty_out(vty,"%% device not support egress acl group !\n");
			}
			else if(ACL_RETURN_CODE_GROUP_SAME_ID == op_ret){
				if(dir_info==0)
					vty_out(vty,"%% %d has been used for egress group!\n",groupNum);
				else if(dir_info==1)
					vty_out(vty,"%% %d has been used for ingress group!\n",groupNum);		
			}
			else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret)
				vty_out(vty,"%% Product not support this function!\n");
			 if(ACL_RETURN_CODE_GROUP_INDEX_ERROR==op_ret)
			     vty_out(vty,"%% Acl group range should be 1-1023!\n");
	}			
	else {
		/*vty_out(vty,"Failed get args.\n");*/
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(config_acl_group_cmd_func,
	  config_acl_group_cmd,
	  "config (ingress|egress) acl-group <1-1023>",
	  CONFIG_STR
	  "Config ingress group\n"
	  "Config egress group\n"
	  "Config acl-group\n"
	  "Acl-group range in 1-1023\n"
)
{
	
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int groupNum = 0,op_ret = 0,dir_info = 0;	
	int ret = 0;
	
	if(strncmp("ingress",argv[0],strlen(argv[0]))==0){
		dir_info = 0;
	}
	else if (strncmp("egress",argv[0],strlen(argv[0]))==0){
		dir_info = 1;
	}	
	else{
		vty_out(vty,"%% bad command parameter!\n");
		return CMD_WARNING;
	}
	ret = dcli_str2ulong((char*)argv[1],&groupNum);
	if (ACL_RETURN_CODE_ERROR == ret) {
		vty_out(vty,"%% Illegal group number!\n");
		return CMD_WARNING;
	}				
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_ACL_OBJPATH,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_METHOD_CONFIG_ACL_GROUP);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,		
							 DBUS_TYPE_UINT32,&dir_info,
							 DBUS_TYPE_UINT32,&groupNum,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_acl,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)) {
		 /* vty_out(vty," op_ret %d\n",op_ret);*/
			if (ACL_RETURN_CODE_GROUP_SUCCESS == op_ret ) {
				/*vty_out(vty,"creat acl group %d successfully!\n",groupNum);*/
				#if 0
				if(CONFIG_NODE == vty->node) {
	            #else
				if(ACL_NODE_DISTRIBUTED == vty->node){ 
				#endif
					if(0==dir_info){
						vty->node = ACL_GROUP_NODE;
					    	vty->index = (void*)groupNum;
					}
					else if(1==dir_info) {
						vty->node = ACL_EGRESS_GROUP_NODE;
						vty->index = (void*)groupNum;
					}
					dbus_message_unref(reply);
					return CMD_SUCCESS;
				}
				else {
				  	/*vty_out (vty, "Terminal mode change remain under config mode!\n", VTY_NEWLINE);*/
					dbus_message_unref(reply);
					return CMD_SUCCESS;
				}
			}
			else if(ACL_RETURN_CODE_GROUP_NOT_EXISTED == op_ret) {
				vty_out(vty,"%% acl group %d not existed!\n",groupNum);
				dbus_message_unref(reply);
				return CMD_SUCCESS;
			}
			else if(ACL_RETURN_CODE_UNBIND_FRIST == op_ret) {
				vty_out(vty,"%% this group binded by port, please unbind it first \n");
				dbus_message_unref(reply);
				return CMD_SUCCESS;
			}
			
			if(ACL_RETURN_CODE_GROUP_INDEX_ERROR==op_ret)
			     vty_out(vty,"%% Acl group range should be 1-1023!\n");
			
	} 
	else {
		/*vty_out(vty,"Failed get args.\n");*/
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
DEFUN(delete_acl_group_func,
	  delete_acl_group_cmd,
	  "delete (ingress|egress) acl-group <1-1023>",
	  "Delete configuration \n"
	  "Delete ingress acl-group\n"
	  "Delete egress acl-group\n"
	  "Delete acl group\n"
	  "delete acl group index 1~1023\n"
)
{	
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int groupNum = 0,op_ret = 0,groupInfo = 0;
	unsigned int temp=0,dir_info=0;
	int ret = 0;
	
	if(strncmp("ingress",argv[0],strlen(argv[0]))==0){
		dir_info = 0;
	}
	else if (strncmp("egress",argv[0],strlen(argv[0]))==0){
		dir_info = 1;
	}	
	else{
		vty_out(vty,"%% bad command parameter!\n");
		return CMD_WARNING;
	}
	ret = dcli_str2ulong((char*)argv[1],&groupNum);
	if (ACL_RETURN_CODE_ERROR == ret) {
		vty_out(vty,"%% Illegal group number!\n");
		return CMD_WARNING;
	}			
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_ACL_OBJPATH,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_METHOD_DELETE_ACL_GROUP);
	
	dbus_error_init(&err);

	dbus_message_append_args(query, 
						     DBUS_TYPE_UINT32,&dir_info,
							 DBUS_TYPE_UINT32,&groupNum,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_acl,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&groupInfo,
		DBUS_TYPE_UINT32,&op_ret,		
		DBUS_TYPE_INVALID)) {

			if(ACL_RETURN_CODE_ERROR==groupInfo)
			{
				if(dir_info==0)
					vty_out(vty,"%% Ingress acl group %d not existed!\n",groupNum);
				else if(dir_info==1)
					vty_out(vty,"%% Egress acl group %d not existed!\n",groupNum);	
			}
			else if (ACL_RETURN_CODE_BCM_DEVICE_NOT_SUPPORT == op_ret) {
				vty_out(vty,"%% device not support egress acl group !\n");
			}
			else if(ACL_RETURN_CODE_GROUP_PORT_BINDED==op_ret)
		    {
				vty_out(vty,"%% this acl group %d has been binded on port or vlan,",groupNum);
				vty_out(vty,"please unbind the acl group firstly!\n");
			}
			else if(ACL_RETURN_CODE_ERROR==op_ret)
			{
				if(dir_info==0)
					vty_out(vty,"%% Delete Ingress acl group fail!\n");
				else if(dir_info==1)
					vty_out(vty,"%% Delete Egress acl group fail\n");						
			}
			else if(ACL_RETURN_CODE_SUCCESS==op_ret)
			{
				/*vty_out(vty,"delete acl group successfully\n");*/
			}
			 if(ACL_RETURN_CODE_GROUP_INDEX_ERROR==op_ret)
			     vty_out(vty,"%% Acl group range should be 1-1023!\n");
	} 
	else {
		/*vty_out(vty,"Failed get args.\n");*/
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;

}

DEFUN(show_acl_group_func,
	  show_acl_group_cmd,
	  "show (ingress|egress) acl-group",
	  "Show running system information\n"
	  "Ingress acl group\n"
	  "Egress acl group\n"
	  "Show acl group information\n"
)
{	
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	unsigned int     k = 0,j = 0,ret = 0,group_count=0,dir_info=0;
	unsigned int     group_num=0,count=0,slot_no=0,port_no=0,portnum = 0, vid_count = 0, index = 0, vidindex=0;

	if(strncmp("ingress",argv[0],strlen(argv[0]))==0){
		dir_info = 0;
	}
	else if (strncmp("egress",argv[0],strlen(argv[0]))==0){
		dir_info = 1;
	}	
	else{
		vty_out(vty,"bad command parameter!\n");
		return CMD_WARNING;
	}
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_ACL_OBJPATH,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_METHOD_SHOW_ACL_GROUP);
	dbus_error_init(&err);
	dbus_message_append_args(query, 
						     DBUS_TYPE_UINT32,&dir_info,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_acl,query,-1, &err);
	dbus_message_unref(query);
	
	if (NULL == reply) 
	 {
		 vty_out(vty,"failed get reply.\n");
		 if (dbus_error_is_set(&err)) 
		 {
			 dbus_error_free_for_dcli(&err);
		 }
		 return CMD_SUCCESS;
	 }
	 dbus_message_iter_init(reply,&iter);
 
	 dbus_message_iter_get_basic(&iter,&ret);

	 if(ACL_RETURN_CODE_ERROR == ret) {
	 	if(dir_info==0)
		  vty_out(vty,"%% Error! No any ingress acl group existed!\n");
		else if (dir_info==1)
		  vty_out(vty,"%% Error! No any egress acl group existed!\n");
	 }
 
	 else if(ACL_RETURN_CODE_SUCCESS == ret){

		 dbus_message_iter_next(&iter);  
		 dbus_message_iter_get_basic(&iter,&group_count); 

		 dbus_message_iter_next(&iter);  
 	     dbus_message_iter_recurse(&iter,&iter_array);
 				
 	     for (j = 0; j < group_count; j++) {

			  DBusMessageIter iter_struct;
			  DBusMessageIter iter_sub_array;
			  
	 		  dbus_message_iter_recurse(&iter_array,&iter_struct);			  

			  dbus_message_iter_get_basic(&iter_struct,&group_num);

			  dbus_message_iter_next(&iter_struct); 
	 		  dbus_message_iter_get_basic(&iter_struct,&count);

			  dbus_message_iter_next(&iter_struct); 
	 		  dbus_message_iter_get_basic(&iter_struct,&portnum);

			  dbus_message_iter_next(&iter_struct); 
	 		  dbus_message_iter_get_basic(&iter_struct,&vid_count);

			
			 vty_out(vty,"===============================================\n");
			 if(dir_info==0)			 	
			 	vty_out(vty,"%-40s: %d\n","Ingress acl group",group_num);
			 else if(dir_info==1)
			 	vty_out(vty,"%-40s: %d\n","Egress acl group",group_num);
			 
				vty_out(vty,"%-40s: %d\n","binded by port count ",portnum);
			  dbus_message_iter_next(&iter_struct); 		  
			  dbus_message_iter_recurse(&iter_struct,&iter_sub_array);

			  for (k = 0; k < portnum; k++){
				  DBusMessageIter iter_sub_struct;
				  dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
							  
				  dbus_message_iter_get_basic(&iter_sub_struct,&slot_no);
				  dbus_message_iter_next(&iter_sub_struct);

				  dbus_message_iter_get_basic(&iter_sub_struct,&port_no);
			         dbus_message_iter_next(&iter_sub_struct);
			         vty_out(vty,"%-40s: %d/%d\n","binded by port",slot_no, port_no);

			         dbus_message_iter_next(&iter_sub_array);
			  }	

			  dbus_message_iter_next(&iter_struct); 	

			  vty_out(vty,"%-40s: %d\n","binded by vlan count ",vid_count);
			  dbus_message_iter_recurse(&iter_struct,&iter_sub_array);

			  for (k = 0; k < vid_count; k++){
				  DBusMessageIter iter_sub_struct;
				  dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
							  
				  dbus_message_iter_get_basic(&iter_sub_struct,&vidindex);
			         dbus_message_iter_next(&iter_sub_struct);
			         vty_out(vty,"%-40s: %d \n","binded by vlan",vidindex);

			         dbus_message_iter_next(&iter_sub_array);
			  }
			  
			 	vty_out(vty,"%-40s: %d\n","acl count",count);
			  dbus_message_iter_next(&iter_struct); 		  
			  dbus_message_iter_recurse(&iter_struct,&iter_sub_array);

			  for (k = 0; k < count; k++){
				  DBusMessageIter iter_sub_struct;
				  dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
							  
				  dbus_message_iter_get_basic(&iter_sub_struct,&index);
				  dbus_message_iter_next(&iter_sub_struct);
			  
		          vty_out(vty,"%-40s: %d\n","acl",index);

		          dbus_message_iter_next(&iter_sub_array);
			  }			  	  
		 	 vty_out(vty,"===============================================\n");
		 	 dbus_message_iter_next(&iter_array);

 	     }/*for*/
	 }  /*if*/
	
	
 
	dbus_message_unref(reply);
	 return CMD_SUCCESS;		 
}
DEFUN(show_acl_index_cmd_func,
	  show_acl_index_cmd,
	  "show acl <1-1000>",
	  SHOW_STR
	  "Show acl rule\n"
	  "Access-list range in 1-1000\n"
)
{	 
	 DBusMessage *query = NULL, *reply = NULL;
	 DBusError err;
	 DBusMessageIter  iter;	
	 char			 actype[50],protype[10];

	 unsigned int acl_Cont = 0;
	 unsigned int ruleIndex=0; 
	 unsigned int ruleType=0;
	 unsigned long	dip=0,sip=0,maskdip=0,masksip=0,srcport=0,dstport=0;
	 unsigned char icmp_code=0,icmp_type=0,code_mask=0,type_mask=0;	
	 unsigned long packetType=0,actionType=0;
	 unsigned char dmac[6]={0},smac[6]={0};
	 unsigned int vlanid=0;
	 unsigned char sourceslot=0,sourceport=0,redirectslot=0,redirectport=0,mirrorslot=0,mirrorport=0;
	 unsigned int i=0,j=0,ret,acl_count=0,type_flag;
	 unsigned char sipBuf[MAX_IP_STRLEN] = {0},dipBuf[MAX_IP_STRLEN] = {0};
	 unsigned char *sipPtr = sipBuf,*dipPtr = dipBuf;
	 unsigned int  policer=0,policerId=0,modifyUP = 0,modifyDSCP = 0,up=0,dscp=0,egrUP=0,egrDSCP=0,qosprofileindex=0;
	 unsigned char upmm[10]={0},dscpmm[10]={0};
	 unsigned int precedence=0, nextheader = 0;
	 unsigned char appendIndex=0;
	 struct ipv6addr dipv6, sipv6;

	 memset(&dipv6, 0, sizeof(struct ipv6addr));
	 memset(&sipv6, 0, sizeof(struct ipv6addr));
	 
	ret = dcli_str2ulong((char*)argv[0],&ruleIndex);
	if (ACL_RETURN_CODE_ERROR == ret) {
		vty_out(vty,"%% Illegal rule index!\n");
		return CMD_WARNING;
	}	
	 ruleIndex -=1;
	
	 query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_ACL_OBJPATH,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_METHOD_SHOW_ACL_INDEX);
	 dbus_error_init(&err);
	 dbus_message_append_args(query, 
						     DBUS_TYPE_UINT32,&ruleIndex,
							 DBUS_TYPE_INVALID);
	 reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_acl,query,-1, &err);
	 dbus_message_unref(query);
	 if (NULL == reply) 
	 {
		 vty_out(vty,"failed get reply.\n");
		 if (dbus_error_is_set(&err)) 
		 {
			 dbus_error_free_for_dcli(&err);
		 }
		 return CMD_SUCCESS;
	 }
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&ret,
		DBUS_TYPE_UINT32,&ruleType,
		DBUS_TYPE_UINT32,&actionType,
		DBUS_TYPE_UINT32,&packetType,
		DBUS_TYPE_UINT32,&dip,
		DBUS_TYPE_UINT32,&maskdip,
		DBUS_TYPE_UINT32,&sip,
		DBUS_TYPE_UINT32,&masksip,
		DBUS_TYPE_BYTE,&(dipv6.ipbuf[0]),
		DBUS_TYPE_BYTE,&(dipv6.ipbuf[1]),
		DBUS_TYPE_BYTE,&(dipv6.ipbuf[2]),
		DBUS_TYPE_BYTE,&(dipv6.ipbuf[3]),
		DBUS_TYPE_BYTE,&(dipv6.ipbuf[4]),
		DBUS_TYPE_BYTE,&(dipv6.ipbuf[5]),
		DBUS_TYPE_BYTE,&(dipv6.ipbuf[6]),
		DBUS_TYPE_BYTE,&(dipv6.ipbuf[7]),
		DBUS_TYPE_BYTE,&(dipv6.ipbuf[8]),
		DBUS_TYPE_BYTE,&(dipv6.ipbuf[9]),
		DBUS_TYPE_BYTE,&(dipv6.ipbuf[10]),
		DBUS_TYPE_BYTE,&(dipv6.ipbuf[11]),
		DBUS_TYPE_BYTE,&(dipv6.ipbuf[12]),
		DBUS_TYPE_BYTE,&(dipv6.ipbuf[13]),
		DBUS_TYPE_BYTE,&(dipv6.ipbuf[14]),
		DBUS_TYPE_BYTE,&(dipv6.ipbuf[15]),
		DBUS_TYPE_BYTE,&(sipv6.ipbuf[0]),
		DBUS_TYPE_BYTE,&(sipv6.ipbuf[1]),
		DBUS_TYPE_BYTE,&(sipv6.ipbuf[2]),
		DBUS_TYPE_BYTE,&(sipv6.ipbuf[3]),
		DBUS_TYPE_BYTE,&(sipv6.ipbuf[4]),
		DBUS_TYPE_BYTE,&(sipv6.ipbuf[5]),
		DBUS_TYPE_BYTE,&(sipv6.ipbuf[6]),
		DBUS_TYPE_BYTE,&(sipv6.ipbuf[7]),
		DBUS_TYPE_BYTE,&(sipv6.ipbuf[8]),
		DBUS_TYPE_BYTE,&(sipv6.ipbuf[9]),
		DBUS_TYPE_BYTE,&(sipv6.ipbuf[10]),
		DBUS_TYPE_BYTE,&(sipv6.ipbuf[11]),
		DBUS_TYPE_BYTE,&(sipv6.ipbuf[12]),
		DBUS_TYPE_BYTE,&(sipv6.ipbuf[13]),
		DBUS_TYPE_BYTE,&(sipv6.ipbuf[14]),
		DBUS_TYPE_BYTE,&(sipv6.ipbuf[15]),
		DBUS_TYPE_UINT32,&nextheader,
		DBUS_TYPE_UINT32,&dstport,
		DBUS_TYPE_UINT32,&srcport,
		DBUS_TYPE_BYTE,&icmp_code,
		DBUS_TYPE_BYTE,&icmp_type,
		DBUS_TYPE_BYTE,&code_mask,
		DBUS_TYPE_BYTE,&type_mask,
		DBUS_TYPE_BYTE,&dmac[0],
		DBUS_TYPE_BYTE,&dmac[1],
		DBUS_TYPE_BYTE,&dmac[2],
		DBUS_TYPE_BYTE,&dmac[3],
		DBUS_TYPE_BYTE,&dmac[4],
		DBUS_TYPE_BYTE,&dmac[5],
		DBUS_TYPE_BYTE,&smac[0],
		DBUS_TYPE_BYTE,&smac[1],
		DBUS_TYPE_BYTE,&smac[2],
		DBUS_TYPE_BYTE,&smac[3],
		DBUS_TYPE_BYTE,&smac[4],
		DBUS_TYPE_BYTE,&smac[5],
		DBUS_TYPE_UINT32,&vlanid,
		DBUS_TYPE_UINT32,&sourceslot,
		DBUS_TYPE_UINT32,&sourceport,
		DBUS_TYPE_UINT32,&redirectslot,
		DBUS_TYPE_UINT32,&redirectport,
		DBUS_TYPE_UINT32,&mirrorslot,
		DBUS_TYPE_UINT32,&mirrorport,
		DBUS_TYPE_UINT32,&policer,
		DBUS_TYPE_UINT32,&policerId,
		DBUS_TYPE_UINT32,&modifyUP,
		DBUS_TYPE_UINT32,&modifyDSCP,
		DBUS_TYPE_UINT32,&up,
		DBUS_TYPE_UINT32,&dscp,
		DBUS_TYPE_UINT32,&egrUP,
		DBUS_TYPE_UINT32,&egrDSCP,
		DBUS_TYPE_UINT32,&qosprofileindex,
		DBUS_TYPE_UINT32,&precedence,
		DBUS_TYPE_BYTE, &appendIndex,
		DBUS_TYPE_INVALID)){
		if(ACL_RETURN_CODE_SUCCESS == ret){
			
		switch(actionType)
		 {
			 case 0:strcpy(actype,"Permit");			 break;
			 case 1:strcpy(actype,"Deny");				 break;
			 case 2:strcpy(actype,"Trap"); 		 break;
			 case 3:strcpy(actype,"Permit");	 break;
			 case 4:strcpy(actype,"Redirect");			 break;
			 case 5:strcpy(actype,"Ingress QoS Mark");	 break;
			 case 6:strcpy(actype,"Egress QoS Remark");  break;
			 default: 
					 break; 	 
		 }
		
		switch(packetType)
		 {
			 case 0:strcpy(protype,"IP");		 break;
			 case 1:strcpy(protype,"UDP");		 break;
			 case 2:strcpy(protype,"TCP");		 break;
			 case 3:strcpy(protype,"ICMP"); 	 break;  
			 case 4:strcpy(protype,"Ethernet");  break;
			 case 5:strcpy(protype,"ARP");		 break; 
			 case 6:strcpy(protype,"All");		 break; 
			 default:
					   break;									 
		 }	 
		 ip_long2str(dip,&dipPtr);
		 ip_long2str(sip,&sipPtr);

		 
		 vty_out(vty,"===============================================\n");
		 vty_out(vty,"%-40s: %ld\n","acl index",1+ruleIndex);

		 if(ruleType ==STANDARD_ACL_RULE) {
			 vty_out(vty,"%-40s: %s\n","acl type","standard");
		 }
		else {
			 vty_out(vty,"%-40s: %s\n","acl type","extended");
		}	 
		
			if((packetType==0 )||(packetType==3)){
				 vty_out(vty,"%-40s: %s\n","action",actype);
				 vty_out(vty,"%-40s: %s\n","ip protocal",protype);
				 vty_out(vty,"%-40s: %s/%ld\n","destination IP address",dipPtr,maskdip);				 
				 vty_out(vty,"%-40s: %s/%ld\n","source IP address",sipPtr,masksip);
				 
				if((strcmp(protype,"TCP")==0)||(strcmp(protype,"UDP")==0)){
					if (ACL_ANY_PORT == dstport) {
						vty_out(vty,"%-40s: %s\n","destination port","any");
					}
					else {
						vty_out(vty,"%-40s: %ld\n","destination port",dstport);
					}
					if (ACL_ANY_PORT == srcport) {
						vty_out(vty,"%-40s: %s\n","source port","any");
					}
					else {
						vty_out(vty,"%-40s: %ld\n","source port",srcport);
					}

				}
				if(strcmp(protype,"ICMP")==0){
					if (0 == code_mask) {
						 vty_out(vty,"%-40s: %s\n","icmp code","any");
					}
					else {
						vty_out(vty,"%-40s: %ld\n","icmp code",icmp_code);
					}
					if (0 == type_mask) {
						vty_out(vty,"%-40s: %s\n","icmp type","any");
					}
					else {
						vty_out(vty,"%-40s: %ld\n","icmp type",icmp_type);
					}

				}
				if (strcmp(actype,"Redirect")==0)
					 vty_out(vty,"%-40s: %d/%d\n","redirect port",redirectslot,redirectport);
						
				if(1==policer){
					vty_out(vty,"%-40s:%s\n","policer","enable");
					vty_out(vty,"%-40s:%d\n","policer ID",policerId);
				}
				if(32==appendIndex){
					vty_out(vty,"%-40s:%d\n","ingress qos",qosprofileindex);
				}
				
					vty_out(vty,"===============================================\n");
			}
					
			else if((packetType==4)||(packetType==5)){
				 vty_out(vty,"%-40s: %s\n","action",actype);
				 vty_out(vty,"%-40s: %s\n","ip protocal",protype);
				 vty_out(vty,"%-40s: %02x:%02x:%02x:%02x:%02x:%02x \n","source MAC",smac[0],smac[1],smac[2],smac[3],smac[4],smac[5]);				 
					
				 if(strcmp(protype,"Ethernet")==0){
					 vty_out(vty,"%-40s: %02x:%02x:%02x:%02x:%02x:%02x \n","destination MAC",dmac[0],dmac[1],dmac[2],dmac[3],dmac[4],dmac[5]);
				 }
			
				if(strcmp(protype,"ARP")==0){
					 vty_out(vty,"%-40s: %s \n","destination MAC","FF:FF:FF:FF:FF:FF");
					  if (0 == vlanid) {
			 	 		vty_out(vty,"%-40s: %s\n","vlan id","any");
					 }
					 else {
						vty_out(vty,"%-40s: %d\n","vlan id", vlanid);
					 }
					 if((ACL_ANY_PORT_CHAR == sourceslot)&&(ACL_ANY_PORT_CHAR == sourceport)){
				       	vty_out(vty,"%-40s: %s\n","source port", "any");
					 }
					 else {
						vty_out(vty,"%-40s: %d/%d\n","source port",sourceslot,sourceport);
					 }
				}

				 if (strcmp(actype,"Redirect")==0)
					vty_out(vty,"%-40s: %d/%d\n","redirect port",redirectslot,redirectport);
		
				 if(1==policer){
					vty_out(vty,"%-40s:%s\n","policer","enable");
					vty_out(vty,"%-40s:%d\n","policer ID",policerId);
				 }
				if(32==appendIndex){
					vty_out(vty,"%-40s:%d\n","ingress qos",qosprofileindex);
				}
				 vty_out(vty,"===============================================\n");
			}
			else if((packetType==6)){
				
				switch(modifyUP){
					case 0:strcpy(upmm,"Keep"); break;
					case 1:
					case 2:strcpy(upmm,"Enable");break;
					default :break;
				}
				switch(modifyDSCP){
					case 0:strcpy(dscpmm,"Keep"); break;
					case 1:
					case 2:strcpy(dscpmm,"Enable");break;
					default :break;
				}	
								
				vty_out(vty,"%-40s: %s\n","action",actype);
				/*vty_out(vty,"%-40s: %s\n","ip protocal",protype);*/
				if(strcmp(actype,"Ingress QoS Mark")==0){	
					if(0==modifyUP)
						vty_out(vty,"%-40s: %s\n","source UP","none");
					else if(2==modifyUP)
						vty_out(vty,"%-40s: %d\n","source UP",up);
					if(0==modifyDSCP)
						vty_out(vty,"%-40s: %s\n","source Dscp","none");
					else if(2==modifyDSCP)
						vty_out(vty,"%-40s: %d\n","source Dscp",dscp);
					
					vty_out(vty,"%-40s: %d\n","Mark QoS profile Table",qosprofileindex);
					if(precedence==0)
						vty_out(vty,"%-40s: %s\n","SubQosMarkers","enable");
					else if(precedence==1)
						vty_out(vty,"%-40s: %s\n","SubQosMarkers","disable");
				}
				if(strcmp(actype,"Egress QoS Remark")==0){											
					vty_out(vty,"%-40s: %d\n","Source UP",up);
					vty_out(vty,"%-40s: %d\n","Source DSCP",dscp);
					vty_out(vty,"%-40s: %d\n","egress UP",egrUP);
					vty_out(vty,"%-40s: %d\n","egress DSCP",egrDSCP);
				}
				
				vty_out(vty,"%-40s: %s\n","Modify UP",upmm);
				vty_out(vty,"%-40s: %s\n","Modify DSCP",dscpmm);		
				
				if(1==policer){
					vty_out(vty,"%-40s: %s\n","policer","enable");
					vty_out(vty,"%-40s: %d\n","policer ID",policerId);
				 }
				
				vty_out(vty,"===============================================\n");
			}
/*		 }*/
		if ((packetType==1)||(packetType==2)) {
			if(ruleType==STANDARD_ACL_RULE){
			 	 vty_out(vty,"%-40s: %s\n","action",actype);
				 vty_out(vty,"%-40s: %s\n","ip protocal",protype);
				 vty_out(vty,"%-40s: %s/%ld\n","destination IP address",dipPtr,maskdip);				 
				 vty_out(vty,"%-40s: %s/%ld\n","source IP address",sipPtr,masksip);
				 
				if((strcmp(protype,"TCP")==0)||(strcmp(protype,"UDP")==0)){
					if (ACL_ANY_PORT == dstport) {
						vty_out(vty,"%-40s: %s\n","destination port","any");
					}
					else {
						vty_out(vty,"%-40s: %ld\n","destination port",dstport);
					}
					if (ACL_ANY_PORT == srcport) {
						vty_out(vty,"%-40s: %s\n","source port","any");
					}
					else {
						vty_out(vty,"%-40s: %ld\n","source port",srcport);
					}

				}
			   	if (strcmp(actype,"Redirect")==0)
					 vty_out(vty,"%-40s: %d/%d\n","redirect port",redirectslot,redirectport);
				
				if(1==policer){
					vty_out(vty,"%-40s: %s\n","policer","enable");
					vty_out(vty,"%-40s: %d\n","policer ID",policerId);
				}
				if(32==appendIndex){
					vty_out(vty,"%-40s: %d\n","ingress qos",qosprofileindex);
				}
				vty_out(vty,"===============================================\n");
		    	  }
			 else if(ruleType==EXTENDED_ACL_RULE){
				/* vty_out(vty,"%-40s: %s\n","acl type","extended");*/
				 vty_out(vty,"%-40s: %s\n","action",actype);
				 vty_out(vty,"%-40s: %s\n","ip protocal",protype);
				 vty_out(vty,"%-40s: %s/%ld\n","destination IP address",dipPtr,maskdip);
				 if (ACL_ANY_PORT == dstport) {
					 vty_out(vty,"%-40s: %s\n","destination port","any");
				 }
				 else {
					 vty_out(vty,"%-40s: %ld\n","destination port",dstport);
				 }
				 vty_out(vty,"%-40s: %s/%ld\n","source IP address",sipPtr,masksip);
				 if (ACL_ANY_PORT == srcport) {
					 vty_out(vty,"%-40s: %s\n","source port","any");
				 }
				 else {
					 vty_out(vty,"%-40s: %ld\n","source port",srcport);
				 }
				 vty_out(vty,"%-40s: %02x:%02x:%02x:%02x:%02x:%02x \n","destination MAC",dmac[0],dmac[1],dmac[2],dmac[3],dmac[4],dmac[5]);
				 vty_out(vty,"%-40s: %02x:%02x:%02x:%02x:%02x:%02x \n","source MAC",smac[0],smac[1],smac[2],smac[3],smac[4],smac[5]); 
				 if (0 == vlanid) {
			 	 	vty_out(vty,"%-40s: %s\n","vlan id","any");
				 }
				 else {
					vty_out(vty,"%-40s: %d\n","vlan id", vlanid);
				 }
				  if((ACL_ANY_PORT_CHAR == sourceslot)&&(ACL_ANY_PORT_CHAR == sourceport)){
			       	vty_out(vty,"%-40s: %s\n","source port", "any");
				 }
				 else {
					vty_out(vty,"%-40s: %d/%d\n","source port",sourceslot,sourceport);
				 }

				 if (strcmp(actype,"Redirect")==0)
					 vty_out(vty,"%-40s: %d/%d\n","redirect port",redirectslot,redirectport);
				
				 if (strcmp(actype,"MirrorToAnalyzer")==0)
					vty_out(vty,"%-40s: %d/%d\n","analyzer port",mirrorslot,mirrorport);
				
				 if(1==policer){
					vty_out(vty,"%-40s:%s\n","policer","enable");
					vty_out(vty,"%-40s:%d\n","policer ID",policerId);
				  }
				 if(32==appendIndex){
					vty_out(vty,"%-40s:%d\n","ingress qos",qosprofileindex);
				}
				vty_out(vty,"===============================================\n");
				}
			}
			else {
				 vty_out(vty,"%-40s: %s\n","action",actype);
				 if (strcmp(actype,"Redirect")==0){
			 		 vty_out(vty,"%-40s: %d/%d\n","redirect port",redirectslot,redirectport);
					 vty_out(vty,"%-40s: %u\n","ipv6 next-header", nextheader);
				 }
				 else{
				 	vty_out(vty,"%-40s: %u\n","ipv6 next-header", nextheader);
				 }
				 char dbuf[sizeof("ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255")];
				  vty_out(vty,"%-40s: %s \n","destination IP address",inet_ntop(AF_INET6, dipv6.ipbuf, dbuf, sizeof(dbuf)));
				  if (8 == packetType) {
					 if (ACL_ANY_PORT == dstport) {
						 vty_out(vty,"%-40s: %s\n","destination port","any");
					 }
					 else {
						 vty_out(vty,"%-40s: %ld\n","destination port",dstport);
					 }
				 }
				 char sbuf[sizeof("ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255")];
				 vty_out(vty,"%-40s: %s \n","source IP address",inet_ntop(AF_INET6, sipv6.ipbuf, sbuf, sizeof(sbuf)));
				if (8 == packetType) {
					if (ACL_ANY_PORT == srcport) {
							 vty_out(vty,"%-40s: %s\n","source port","any");
					 }
					else {
							 vty_out(vty,"%-40s: %ld\n","source port",srcport);
					}
				 }
				 if (strcmp(actype,"Redirect")!=0){
				 	if(1==policer){
	   				vty_out(vty,"%-40s: %s\n","policer","enable");
	   				vty_out(vty,"%-40s: %d\n","policer ID",policerId);
		   			}
					if(32==appendIndex){
						vty_out(vty,"%-40s: %d\n","ingress qos",qosprofileindex);		
					}
				 }
				
				vty_out(vty,"===============================================\n");
			}

		 }
		 else if(ACL_RETURN_CODE_ERROR == ret) {
			 vty_out(vty,"%% Error: ACL rule %d not exists!\n",1+ruleIndex);
		 }
		else if(NPD_DBUS_ERROR_NO_SUCH_PORT ==ret){
			vty_out(vty,"%% Error: illegal port index\n");
		}
		else if(ACL_RETURN_CODE_RULE_INDEX_ERROR==ret)
			 vty_out(vty,"%% Acl rule range should be 1-1000!\n");
		 else
			vty_out(vty,"%% display error!\n");

	}

	dbus_message_unref(reply);
	 return CMD_SUCCESS;
 }

DEFUN(show_acl_group_index_cmd_func,
	  show_acl_group_index_cmd,
	  "show (ingress|egress) acl-group <1-1023>",
	  SHOW_STR
	  "Ingress acl group\n"
	  "Egress acl group\n"
	  "Show acl group information\n"
	  "Acl group range in 1-1023\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	unsigned int	 k = 0,j = 0,ret = 0,group_count=0,dir_info=0;
	unsigned int	 group_num=0,count=0,index=0;
	unsigned int     groupIndex=0,portcount=0;
	unsigned int    		slot_no=0,local_port_no=0;
	unsigned int	vid_count = 0, vid = 0;
	
	if(strncmp("ingress",argv[0],strlen(argv[0]))==0){
		dir_info = 0;
	}
	else if (strncmp("egress",argv[0],strlen(argv[0]))==0){
		dir_info = 1;
	}	
	else{
		vty_out(vty,"bad command parameter!\n");
		return CMD_WARNING;
	}
	ret = dcli_str2ulong((char*)argv[1],&groupIndex);
	if (ACL_RETURN_CODE_ERROR == ret) {
		vty_out(vty,"%% Illegal group number!\n");
		return CMD_WARNING;
	}
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_ACL_OBJPATH,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_METHOD_SHOW_ACL_GROUP_INDEX);
	dbus_error_init(&err);
	dbus_message_append_args(query, 
							 DBUS_TYPE_UINT32,&dir_info,
							 DBUS_TYPE_UINT32,&groupIndex,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_acl,query,-1, &err);
	dbus_message_unref(query);
	
	if (NULL == reply) 
	 {
		 vty_out(vty,"failed get reply.\n");
		 if (dbus_error_is_set(&err)) 
		 {
			 dbus_error_free_for_dcli(&err);
		 }
		 return CMD_SUCCESS;
	 }

		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&ret);	
		if(ACL_RETURN_CODE_SUCCESS == ret){
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&count);
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&portcount);
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&vid_count);
			dbus_message_iter_next(&iter);
			dbus_message_iter_recurse(&iter,&iter_array);
			
			vty_out(vty,"===============================================\n");			
		      if(dir_info == 0) {
				vty_out(vty,"%-40s: %d\n","ingress acl group",groupIndex);
			}
			else if(dir_info ==1) {
				vty_out(vty,"%-40s: %d\n","egress acl group",groupIndex);
			}				

			vty_out(vty,"%-40s: %d\n","binded by port count",portcount);
					    
			for (j = 0; j < portcount; j++) {
				DBusMessageIter iter_struct;
				dbus_message_iter_recurse(&iter_array,&iter_struct);			
				dbus_message_iter_get_basic(&iter_struct,&slot_no);
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&local_port_no);
				dbus_message_iter_next(&iter_array); 
				
				vty_out(vty,"%-40s: %d/%d\n","binded by port", slot_no, local_port_no);
			}
			dbus_message_iter_next(&iter);	

			dbus_message_iter_recurse(&iter,&iter_array);
			vty_out(vty,"%-40s: %d\n","binded by vlan count",vid_count);
			for (j = 0; j < vid_count; j++) {
				DBusMessageIter iter_struct;
				dbus_message_iter_recurse(&iter_array,&iter_struct);			
				dbus_message_iter_get_basic(&iter_struct,&vid);
				dbus_message_iter_next(&iter_array); 
				
				vty_out(vty,"%-40s: %d \n","binded by vlan", vid);
			}
			dbus_message_iter_next(&iter);

			vty_out(vty,"%-40s: %d\n","acl count",count);
			dbus_message_iter_recurse(&iter,&iter_array);					    
			for (j = 0; j < count; j++) {
				DBusMessageIter iter_struct;
				dbus_message_iter_recurse(&iter_array,&iter_struct);			
				dbus_message_iter_get_basic(&iter_struct,&index);
				dbus_message_iter_next(&iter_array); 
				
				vty_out(vty,"%-40s: %d\n","acl index",index);
				}
			vty_out(vty,"===============================================\n");
		
					
		}
		else if(ACL_RETURN_CODE_GROUP_NOT_EXISTED == ret)
		{			
		   if(dir_info ==0){
				vty_out(vty,"%% ingress acl group %d not existed!\n",groupIndex);}
			else if(dir_info ==1){
				vty_out(vty,"%% egress acl group %d not existed!\n",groupIndex);}		
		
		}
		if(ACL_RETURN_CODE_GROUP_INDEX_ERROR==ret)
			 vty_out(vty,"%% Acl group range should be 1-1023!\n");

	dbus_message_unref(reply);
	 return CMD_SUCCESS;
}

/* STANDARD ACL RULE RANGE */
/* standard acl rule range with dip range sip range policer range */
/* standard acl rule range with dip range sip range policer unique */
DEFUN(config_acl_std_dsip_range_policer_func,
	  config_acl_std_dsip_range_cmd,
	  "acl-range standard <1-1000> <1-1000> ip-range destination A.B.C.D A.B.C.D source A.B.C.D A.B.C.D",
	  ACL_STR
	  "standard acl rule\n"
      "start index, standard rule index range in 1-1000\n"
      "end index, standard rule index range in 1-1000\n"
	  "acl rule deal with IP packet\n"
	  "Destination IP address for IP packet\n"
	  "Specify start IP address in <A.B.C.D> format\n"
	  "Specify end IP address in <A.B.C.D> format\n"
	  "Source IP address for IP packet\n"
	  "Specify start IP address in <A.B.C.D> format\n"
	  "Specify end IP address in <A.B.C.D> format\n"
)
{
	DBusMessage 	*query = NULL, *reply = NULL;
	DBusError		err;
	unsigned int	startIndex = 0, endIndex = 0;
	unsigned long	startDip = 0, endDip = 0, startSip = 0, endSip = 0, maskLen = 0;
	unsigned  int	ruleType = 0, ret = 0, i = 0;
	unsigned long	op_ret = 0;
	unsigned int	policer = 0, startPid = 0, endPid = 0;
	int Val = 0;
	
	#if 0
	if((argc < 2)||(argc > 4))
	{
		vty_out(vty, "bad parameters number!\n");
		return CMD_WARNING;
	}
	#endif
	ret = dcli_str2ulong((char*)argv[0],&startIndex);
	if (ACL_RETURN_CODE_ERROR == ret) {
		vty_out(vty,"%% Illegal rule index!\n");
		return CMD_WARNING;
	}
	if ((ruleType == EXTENDED_ACL_RULE)&&(startIndex > MAX_EXT_RULE_NUM)) {
		vty_out(vty,"extended rule must less than 500!\n");
		return CMD_WARNING;
	}
	startIndex = startIndex-1;

	ret = dcli_str2ulong((char*)argv[1],&endIndex);
	if (ACL_RETURN_CODE_ERROR == ret) {
		vty_out(vty,"%% Illegal rule index!\n");
		return CMD_WARNING;
	}
	if ((ruleType == EXTENDED_ACL_RULE)&&(endIndex > MAX_EXT_RULE_NUM)) {
		vty_out(vty,"extended rule must less than 500!\n");
		return CMD_WARNING;
	}
	endIndex = endIndex-1;

	startDip = dcli_ip2ulong((char*)argv[2]);			
	endDip = dcli_ip2ulong((char*)argv[3]);
	startSip = dcli_ip2ulong((char*)argv[4]);
	endSip = dcli_ip2ulong((char*)argv[5]);
	
	if (((endSip - startSip) != (endIndex - startIndex)) || ((endDip - startDip) != (endIndex - startIndex)) ){
		vty_out(vty,"%% Illegal rule index!\n");
		return CMD_WARNING;
	}

	/*policer*/
	if (7 == argc) {
		policer = 1;
		ret = dcli_str2ulong((char *)argv[6], &startPid);
		if(ACL_RETURN_CODE_ERROR == ret) {
			vty_out(vty,"%% Illegal policer index!\n");
			return CMD_WARNING;
		}
	}
	else if (8 == argc) {
		policer = 2;
		ret = dcli_str2ulong((char *)argv[6], &startPid);
		if(ACL_RETURN_CODE_ERROR == ret) {
			vty_out(vty,"%% Illegal policer index!\n");
			return CMD_WARNING;
		}
		ret = dcli_str2ulong((char *)argv[7], &endPid);
		if(ACL_RETURN_CODE_ERROR == ret) {
			vty_out(vty,"%% Illegal policer index!\n");
			return CMD_WARNING;
		}
		if ((endPid - startPid) != (endIndex - startIndex)) {
			vty_out(vty,"%% Illegal policer index!\n");
			return CMD_WARNING;
		}	
	}

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_ACL_OBJPATH, \
					NPD_DBUS_ACL_INTERFACE, NPD_DBUS_METHOD_CONFIG_ACL_RULE_IP_RANGE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &startIndex,
							 DBUS_TYPE_UINT32, &endIndex,
							 DBUS_TYPE_UINT32, &ruleType,
							 DBUS_TYPE_UINT32, &startDip,
							 DBUS_TYPE_UINT32, &endDip,
							 DBUS_TYPE_UINT32, &startSip,
							 DBUS_TYPE_UINT32, &endSip,
							 DBUS_TYPE_UINT32, &policer,
							 DBUS_TYPE_UINT32, &startPid,							
 							 DBUS_TYPE_UINT32, &endPid,
							 DBUS_TYPE_INVALID);
	   
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_acl,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{
			if (ACL_RETURN_CODE_SUCCESS == op_ret ) {
				/*vty_out(vty,"%% set fail!\n");*/
			}
			else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret)
				vty_out(vty,"%% Product not support this function!\n");
			/*
			else if(ACL_RETURN_CODE_GLOBAL_EXISTED == op_ret)
				vty_out(vty,"%% Access-list %d existed!\n",(startPid+1));
			
			else if((ACL_RETURN_CODE_EXT_NO_SPACE==op_ret)&&(ruleType == STANDARD_ACL_RULE))
		         	vty_out(vty,"%% because the extended should take up twice spaces than stdard acl,but %d has been set extended!\n",(startPid-511));
			else if((ACL_RETURN_CODE_EXT_NO_SPACE==op_ret)&&(ruleType == EXTENDED_ACL_RULE))
				vty_out(vty,"%% because the extended rule should take up twice spaces than standard acl,\
				 			  but %d has been set standard rule! ~~set fail\n",(startPid+513));

			else if(ACL_RETURN_CODE_SAME_FIELD == op_ret)
				vty_out(vty,"%% identical fields of packet can not set again\n");	
            		else if(ACL_RETURN_CODE_POLICER_ID_NOT_SET == op_ret)
				vty_out(vty,"%% Policer %d not existed!\n", startPid);
				*/
			else 
				vty_out(vty,"%% set fail!\n");
		
	} 
	else 
	{
		/*vty_out(vty,"Failed get args.\n");*/
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
	
}

ALIAS(config_acl_std_dsip_range_policer_func,
		config_acl_std_dsip_range_policer_cmd,
		"acl-range standard <1-1000> <1-1000> ip-range destination A.B.C.D A.B.C.D source A.B.C.D A.B.C.D policer <1-255>",
		ACL_STR
);

ALIAS(config_acl_std_dsip_range_policer_func,
		config_acl_std_dsip_range_policer_range_cmd,
		"acl-range standard <1-1000> <1-1000> ip-range destination A.B.C.D A.B.C.D source A.B.C.D A.B.C.D policer-range <1-255> <1-255>",
		ACL_STR
);

/* standard acl rule range with dip range sip any policer range */
/* standard acl rule range with dip range sip any policer unique */
DEFUN(config_acl_std_dip_range_policer_func,
		config_acl_std_dip_range_cmd,
		"acl-range standard <1-1000> <1-1000> ip-range destination A.B.C.D A.B.C.D source any",
		ACL_STR
		"standard acl rule\n"
		"start index, standard rule index range in 1-1000\n"
		"end index, standard rule index range in 1-1000\n"
		"acl rule deal with IP packet\n"
		"Destination IP address for IP packet\n"
		"Specify start IP address in <A.B.C.D> format\n"
	    "Specify end IP address in <A.B.C.D> format\n"
	    "Source IP address for IP packet\n"
		"Source IP address for TCP OR UDP packet\n"		
		"Any legal source IP address\n"
)
{
	DBusMessage 	*query = NULL, *reply = NULL;
	DBusError		err;
	unsigned int	startIndex = 0, endIndex = 0;
	unsigned long	startDip = 0, endDip = 0, startSip = 0, endSip = 0, maskLen = 0;
	unsigned  int	ruleType = 0, ret = 0, i = 0;
	unsigned long	op_ret = 0;
	unsigned int	policer = 0, startPid = 0, endPid = 0;
	int Val = 0;
	
#if 0
	if((argc < 2)||(argc > 4))
	{
		vty_out(vty, "bad parameters number!\n");
		return CMD_WARNING;
	}
#endif
	ret = dcli_str2ulong((char*)argv[0],&startIndex);
	if (ACL_RETURN_CODE_ERROR == ret) {
		vty_out(vty,"%% Illegal rule index!\n");
		return CMD_WARNING;
	}
	if ((ruleType == EXTENDED_ACL_RULE)&&(startIndex > MAX_EXT_RULE_NUM)) {
		vty_out(vty,"extended rule must less than 500!\n");
		return CMD_WARNING;
	}
	startIndex = startIndex-1;

	ret = dcli_str2ulong((char*)argv[1],&endIndex);
	if (ACL_RETURN_CODE_ERROR == ret) {
		vty_out(vty,"%% Illegal rule index!\n");
		return CMD_WARNING;
	}
	if ((ruleType == EXTENDED_ACL_RULE)&&(endIndex > MAX_EXT_RULE_NUM)) {
		vty_out(vty,"extended rule must less than 500!\n");
		return CMD_WARNING;
	}
	endIndex = endIndex-1;


	startDip = dcli_ip2ulong((char*)argv[2]);			
	endDip = dcli_ip2ulong((char*)argv[3]);
	startSip = 0;
	endSip = 0;
	
	if ((endDip - startDip) != (endIndex - startIndex)) {
		vty_out(vty,"%% Illegal rule index!\n");
		return CMD_WARNING;
	}
	
	/*policer*/
	if (5 == argc) {
		policer = 1;
		ret = dcli_str2ulong((char *)argv[4], &startPid);
		if(ACL_RETURN_CODE_ERROR == ret) {
			vty_out(vty,"%% Illegal policer index!\n");
			return CMD_WARNING;
		}
	}
	else if (6 == argc) {
		policer = 2;
		ret = dcli_str2ulong((char *)argv[4], &startPid);
		if(ACL_RETURN_CODE_ERROR == ret) {
			vty_out(vty,"%% Illegal policer index!\n");
			return CMD_WARNING;
		}
		ret = dcli_str2ulong((char *)argv[5], &endPid);
		if(ACL_RETURN_CODE_ERROR == ret) {
			vty_out(vty,"%% Illegal policer index!\n");
			return CMD_WARNING;
		}
		if ((endPid - startPid) != (endIndex - startIndex)) {
			vty_out(vty,"%% Illegal policer index!\n");
			return CMD_WARNING;
		}	
	}

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_ACL_OBJPATH, \
					NPD_DBUS_ACL_INTERFACE, NPD_DBUS_METHOD_CONFIG_ACL_RULE_IP_RANGE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &startIndex,
							 DBUS_TYPE_UINT32, &endIndex,
							 DBUS_TYPE_UINT32, &ruleType,
							 DBUS_TYPE_UINT32, &startDip,
							 DBUS_TYPE_UINT32, &endDip,
							 DBUS_TYPE_UINT32, &startSip,
							 DBUS_TYPE_UINT32, &endSip,
							 DBUS_TYPE_UINT32, &policer,
							 DBUS_TYPE_UINT32, &startPid,							
							 DBUS_TYPE_UINT32, &endPid,
							 DBUS_TYPE_INVALID);
	   
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_acl,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{
			if (ACL_RETURN_CODE_SUCCESS == op_ret ) {
				/*vty_out(vty,"%% set fail!\n");*/
			}	
			else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret)
				vty_out(vty,"%% Product not support this function!\n");
			/*
			else if(ACL_RETURN_CODE_GLOBAL_EXISTED == op_ret)
				vty_out(vty,"%% Access-list %d existed!\n",(startPid+1));
			
			else if((ACL_RETURN_CODE_EXT_NO_SPACE==op_ret)&&(ruleType == STANDARD_ACL_RULE))
					vty_out(vty,"%% because the extended should take up twice spaces than stdard acl,but %d has been set extended!\n",(startPid-511));
			else if((ACL_RETURN_CODE_EXT_NO_SPACE==op_ret)&&(ruleType == EXTENDED_ACL_RULE))
				vty_out(vty,"%% because the extended rule should take up twice spaces than standard acl,\
							  but %d has been set standard rule! ~~set fail\n",(startPid+513));

			else if(ACL_RETURN_CODE_SAME_FIELD == op_ret)
				vty_out(vty,"%% identical fields of packet can not set again\n");	
					else if(ACL_RETURN_CODE_POLICER_ID_NOT_SET == op_ret)
				vty_out(vty,"%% Policer %d not existed!\n", startPid);
				*/
			else 
				vty_out(vty,"%% set fail!\n");
		
	} 
	else 
	{
		/*vty_out(vty,"Failed get args.\n");*/
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;

}
ALIAS(config_acl_std_dip_range_policer_func,
		config_acl_std_dip_range_policer_cmd,
		"acl-range standard <1-1000> <1-1000> ip-range destination A.B.C.D A.B.C.D source any policer <1-255>",
		ACL_STR
);

ALIAS(config_acl_std_dip_range_policer_func,
		config_acl_std_dip_range_policer_range_cmd,
		"acl-range standard <1-1000> <1-1000> ip-range destination A.B.C.D A.B.C.D source any policer-range <1-255> <1-255>",
		ACL_STR
);

/* standard acl rule range with dip any sip range policer range */
/* standard acl rule range with dip any sip range policer unique */
DEFUN(config_acl_std_sip_range_policer_func,
		config_acl_std_sip_range_cmd,
		"acl-range standard <1-1000> <1-1000> ip-range destination any source A.B.C.D A.B.C.D",
		ACL_STR
		"standard acl rule\n"
		"start index, standard rule index range in 1-1000\n"
		"end index, standard rule index range in 1-1000\n"
		"acl rule deal with IP packet\n"
		"Destination IP address for IP packet\n"
		"Any legal destination IP address\n"
		"Source IP address for TCP OR UDP packet\n"
		"Specify IP address in <A.B.C.D> format\n"
		"Specify IP address in <A.B.C.D> format\n"
)
{
	DBusMessage 	*query = NULL, *reply = NULL;
	DBusError		err;
	unsigned int	startIndex = 0, endIndex = 0;
	unsigned long	startDip = 0, endDip = 0, startSip = 0, endSip = 0, maskLen = 0;
	unsigned  int	ruleType = 0, ret = 0, i = 0;
	unsigned long	op_ret = 0;
	unsigned int	policer = 0, startPid = 0, endPid = 0;
	int Val = 0;
	
#if 0
	if((argc < 2)||(argc > 4))
	{
		vty_out(vty, "bad parameters number!\n");
		return CMD_WARNING;
	}
#endif
	ret = dcli_str2ulong((char*)argv[0],&startIndex);
	if (ACL_RETURN_CODE_ERROR == ret) {
		vty_out(vty,"%% Illegal rule index!\n");
		return CMD_WARNING;
	}
	if ((ruleType == EXTENDED_ACL_RULE)&&(startIndex > MAX_EXT_RULE_NUM)) {
		vty_out(vty,"extended rule must less than 500!\n");
		return CMD_WARNING;
	}
	startIndex = startIndex-1;

	ret = dcli_str2ulong((char*)argv[1],&endIndex);
	if (ACL_RETURN_CODE_ERROR == ret) {
		vty_out(vty,"%% Illegal rule index!\n");
		return CMD_WARNING;
	}
	if ((ruleType == EXTENDED_ACL_RULE)&&(endIndex > MAX_EXT_RULE_NUM)) {
		vty_out(vty,"extended rule must less than 500!\n");
		return CMD_WARNING;
	}
	endIndex = endIndex-1;

	startDip = 0;
	endDip = 0;
	startSip = dcli_ip2ulong((char*)argv[2]);
	endSip = dcli_ip2ulong((char*)argv[3]);
	
	if ((endSip - startSip) != (endIndex - startIndex)) {
		vty_out(vty,"%% Illegal rule index!\n");
		return CMD_WARNING;
	}
	
	/*policer*/
	if (5 == argc) {
		policer = 1;
		ret = dcli_str2ulong((char *)argv[4], &startPid);
		if(ACL_RETURN_CODE_ERROR == ret) {
			vty_out(vty,"%% Illegal policer index!\n");
			return CMD_WARNING;
		}
	}
	else if (6 == argc) {
		policer = 2;
		ret = dcli_str2ulong((char *)argv[4], &startPid);
		if(ACL_RETURN_CODE_ERROR == ret) {
			vty_out(vty,"%% Illegal policer index!\n");
			return CMD_WARNING;
		}
		ret = dcli_str2ulong((char *)argv[5], &endPid);
		if(ACL_RETURN_CODE_ERROR == ret) {
			vty_out(vty,"%% Illegal policer index!\n");
			return CMD_WARNING;
		}
		if ((endPid - startPid) != (endIndex - startIndex)) {
			vty_out(vty,"%% Illegal policer index!\n");
			return CMD_WARNING;
		}	
	}	
	/*policer*/
	/*
	if(argc > 4){
		if(strcmp("policer", argv[4])){
			policer = 1;
			ret= dcli_str2ulong((char *)argv[5],&startPid);
			if(ret==ACL_RETURN_CODE_ERROR) {
				vty_out(vty,"%% Illegal policer ID!\n");
				return CMD_WARNING;
			}
		}
		else if(strcmp("policer-range", argv[5])){
			policer = 2;
			ret= dcli_str2ulong((char *)argv[6],&startPid);
			if(ret==ACL_RETURN_CODE_ERROR) {
				vty_out(vty,"%% Illegal policer ID!\n");
				return CMD_WARNING;
			}
			ret= dcli_str2ulong((char *)argv[7],&endPid);
			if(ret==ACL_RETURN_CODE_ERROR) {
				vty_out(vty,"%% Illegal policer ID!\n");
				return CMD_WARNING;
			}
			if ((endPid - startPid) != (endIndex - startIndex)) {
				vty_out(vty,"%% Illegal policer index!\n");
				return CMD_WARNING;
			}
		}
		else{
			vty_out(vty,"%% unknown command\n");
			return CMD_WARNING;
		}		   
	}
	*/
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_ACL_OBJPATH, \
					NPD_DBUS_ACL_INTERFACE, NPD_DBUS_METHOD_CONFIG_ACL_RULE_IP_RANGE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &startIndex,
							 DBUS_TYPE_UINT32, &endIndex,
							 DBUS_TYPE_UINT32, &ruleType,
							 DBUS_TYPE_UINT32, &startDip,
							 DBUS_TYPE_UINT32, &endDip,
							 DBUS_TYPE_UINT32, &startSip,
							 DBUS_TYPE_UINT32, &endSip,
							 DBUS_TYPE_UINT32, &policer,
							 DBUS_TYPE_UINT32, &startPid,							
							 DBUS_TYPE_UINT32, &endPid,
							 DBUS_TYPE_INVALID);
	   
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_acl,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{
			if (ACL_RETURN_CODE_SUCCESS == op_ret ) {
				/*vty_out(vty,"%% set fail!\n");*/
			}	
			else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret)
				vty_out(vty,"%% Product not support this function!\n");
			/*
			else if(ACL_RETURN_CODE_GLOBAL_EXISTED == op_ret)
				vty_out(vty,"%% Access-list %d existed!\n",(startPid+1));
			
			else if((ACL_RETURN_CODE_EXT_NO_SPACE==op_ret)&&(ruleType == STANDARD_ACL_RULE))
					vty_out(vty,"%% because the extended should take up twice spaces than stdard acl,but %d has been set extended!\n",(startPid-511));
			else if((ACL_RETURN_CODE_EXT_NO_SPACE==op_ret)&&(ruleType == EXTENDED_ACL_RULE))
				vty_out(vty,"%% because the extended rule should take up twice spaces than standard acl,\
							  but %d has been set standard rule! ~~set fail\n",(startPid+513));

			else if(ACL_RETURN_CODE_SAME_FIELD == op_ret)
				vty_out(vty,"%% identical fields of packet can not set again\n");	
					else if(ACL_RETURN_CODE_POLICER_ID_NOT_SET == op_ret)
				vty_out(vty,"%% Policer %d not existed!\n", startPid);
				*/
			else 
				vty_out(vty,"%% set fail!\n");
		
	} 
	else 
	{
		/*vty_out(vty,"Failed get args.\n");*/
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;

}

ALIAS(config_acl_std_sip_range_policer_func,
		config_acl_std_sip_range_policer_cmd,
		"acl-range standard <1-1000> <1-1000> ip-range destination any source A.B.C.D A.B.C.D policer <1-255>",
		ACL_STR
);

ALIAS(config_acl_std_sip_range_policer_func,
		config_acl_std_sip_range_policer_range_cmd,
		"acl-range standard <1-1000> <1-1000> ip-range destination any source A.B.C.D A.B.C.D policer-range <1-255> <1-255>",
		ACL_STR
);

/* EXTENDED ACL RULE RANGE */
/* extended acl rule range with dip range sip range policer range */
/* extended acl rule range with dip range sip range policer unique */
/* extended acl rule range with dip range sip any policer range */
/* extended acl rule range with dip range sip any policer unique */
/* extended acl rule range with dip any sip range policer range */
/* extended acl rule range with dip any sip range policer unique */

DEFUN(config_acl_std_permit_sip_range_func,
		config_acl_std_permit_sip_range_cmd,
		"acl-range standard <1-1000> (permit|deny) index-range <1-40000> <1-40000> ip-range destination any source A.B.C.D A.B.C.D",
		ACL_STR
		"standard acl rule\n"
		"index, standard rule index range in 1-1000\n"
		"start index, standard rule index range in 1-40000\n"
		"end index, standard rule index range in 1-40000\n"
		"acl rule deal with IP packet\n"
		"Destination IP address for IP packet\n"
		"Any legal destination IP address\n"
		"Source IP address for TCP OR UDP packet\n"
		"Specify IP address in <A.B.C.D format\n"
		"Specify IP address in <A.B.C.D format\n"
		"Allow policing on ingress port\n"
		"Policer Id range in 1-255\n"
)
{
	DBusMessage 	*query = NULL, *reply = NULL;
	DBusError		err;
	unsigned int	index = 0, startIndex = 0, endIndex = 0, actionType = 0;
	unsigned long	startDip = 0, endDip = 0, startSip = 0, endSip = 0, maskLen = 0;
	unsigned  int	ruleType = 0, ret = 0, i = 0;
	unsigned long	op_ret = 0;
	unsigned int	policer = 0, startPid = 0, endPid = 0;
	int Val = 0;
	
#if 0
	if((argc < 2)||(argc > 4))
	{
		vty_out(vty, "bad parameters number!\n");
		return CMD_WARNING;
	}
#endif
	ret = dcli_str2ulong((char*)argv[0],&index);
	if (ACL_RETURN_CODE_ERROR == ret) {
		vty_out(vty,"%% Illegal rule index!\n");
		return CMD_WARNING;
	}
	if ((ruleType == EXTENDED_ACL_RULE)&&(index > MAX_EXT_RULE_NUM)) {
		vty_out(vty,"extended rule must less than 500!\n");
		return CMD_WARNING;
	}
	index = index - 1;
	
	if(strncmp("permit",argv[1],strlen(argv[1]))==0) { 
		actionType=0;
	}
    else if(strncmp("deny",argv[1],strlen(argv[1]))==0) 
	{
		actionType=1;
	}      

	ret = dcli_str2ulong((char*)argv[2],&startIndex);
	if (ACL_RETURN_CODE_ERROR == ret) {
		vty_out(vty,"%% Illegal rule index!\n");
		return CMD_WARNING;
	}


	ret = dcli_str2ulong((char*)argv[3],&endIndex);
	if (ACL_RETURN_CODE_ERROR == ret) {
		vty_out(vty,"%% Illegal rule index!\n");
		return CMD_WARNING;
	}

	startDip = 0;
	endDip = 0;
	startSip = dcli_ip2ulong((char*)argv[4]);
	endSip = dcli_ip2ulong((char*)argv[5]);
	
	if ((endSip - startSip) != (endIndex - startIndex)) {
		vty_out(vty,"%% Illegal rule index!\n");
		return CMD_WARNING;
	}
	
	/*policer*/
	if (7 == argc) {
		if (actionType) {	
			vty_out(vty,"%% policer must action permit!\n");
			return CMD_WARNING;
		}
		policer = 1;
		ret = dcli_str2ulong((char *)argv[6], &startPid);
		if(ACL_RETURN_CODE_ERROR == ret) {
			vty_out(vty,"%% Illegal policer index!\n");
			return CMD_WARNING;
		}
	}

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_ACL_OBJPATH, \
					NPD_DBUS_ACL_INTERFACE, NPD_DBUS_METHOD_CONFIG_ACL_PERMIT_RULE_IP_RANGE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &index,
							 DBUS_TYPE_UINT32, &startIndex,
							 DBUS_TYPE_UINT32, &endIndex,
							 DBUS_TYPE_UINT32, &ruleType,
							 DBUS_TYPE_UINT32, &actionType,
							 DBUS_TYPE_UINT32, &startDip,
							 DBUS_TYPE_UINT32, &endDip,
							 DBUS_TYPE_UINT32, &startSip,
							 DBUS_TYPE_UINT32, &endSip,
							 DBUS_TYPE_UINT32, &policer,
							 DBUS_TYPE_UINT32, &startPid,							
							 DBUS_TYPE_UINT32, &endPid,
							 DBUS_TYPE_INVALID);
	   
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_acl,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{
			if (ACL_RETURN_CODE_SUCCESS == op_ret ) {
				/*vty_out(vty,"%% set fail!\n");*/
			}	
			/*
			else if(ACL_RETURN_CODE_GLOBAL_EXISTED == op_ret)
				vty_out(vty,"%% Access-list %d existed!\n",(startPid+1));
			
			else if((ACL_RETURN_CODE_EXT_NO_SPACE==op_ret)&&(ruleType == STANDARD_ACL_RULE))
					vty_out(vty,"%% because the extended should take up twice spaces than stdard acl,but %d has been set extended!\n",(startPid-511));
			else if((ACL_RETURN_CODE_EXT_NO_SPACE==op_ret)&&(ruleType == EXTENDED_ACL_RULE))
				vty_out(vty,"%% because the extended rule should take up twice spaces than standard acl,\
							  but %d has been set standard rule! ~~set fail\n",(startPid+513));

			else if(ACL_RETURN_CODE_SAME_FIELD == op_ret)
				vty_out(vty,"%% identical fields of packet can not set again\n");	
					else if(ACL_RETURN_CODE_POLICER_ID_NOT_SET == op_ret)
				vty_out(vty,"%% Policer %d not existed!\n", startPid);
				*/
			else 
				vty_out(vty,"%% set fail!\n");
		
	} 
	else 
	{
		/*vty_out(vty,"Failed get args.\n");*/
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;

}

ALIAS(config_acl_std_permit_sip_range_func,
		config_acl_std_permit_sip_policer_range_cmd,
		"acl-range standard <1-1000> (permit|deny) index-range <1-40000> <1-40000> ip-range destination any source A.B.C.D A.B.C.D policer <1-255>",
		ACL_STR
);

DEFUN(add_delete_acl_rule_to_acl_range_group_func,
		add_delete_acl_rule_to_acl_range_group_cmd,
		"(add|delete) acl-range <1-1000> <1-1000>",
		"Add acl rule to acl group\n"
		"Delete acl rule from acl group\n"
		"Add or delete standard or extended access list\n"
		"Specify standard rule range in 1-1000,extended rule range in 1-500\n"
		"Specify standard rule range in 1-1000,extended rule range in 1-500\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int      ruleIndex = 0, startIndex = 0, endIndex = 0;
	unsigned int      ifIndex = 0, i = 0;
	unsigned int      op_ret = 0,op_ret1 = 0,op_ret2 = 0;
	unsigned int	  acl_group_num = 0;
	unsigned int      group_inf = 0,ret = 0,num = 0;
	unsigned int      op_flag = 0,op_info = 0,dir_info = 0,exist_dir = 0;

	if(0==strncmp("add",argv[0],strlen(argv[0])))
	   op_flag = 0;
	else if(0==strncmp("delete",argv[0],strlen(argv[0])))
	   op_flag = 1;
	
	ret = dcli_str2ulong((char*)argv[1],&startIndex);
	if (ACL_RETURN_CODE_ERROR == ret) {
		vty_out(vty,"%% Illegal rule index!\n");
		return CMD_WARNING;
	}
	startIndex = startIndex-1;

	ret = dcli_str2ulong((char*)argv[2],&endIndex);
	if (ACL_RETURN_CODE_ERROR == ret) {
		vty_out(vty,"%% Illegal rule index!\n");
		return CMD_WARNING;
	}
	endIndex = endIndex-1;

	if(ACL_GROUP_NODE==(vty->node)){
		acl_group_num = (unsigned int)(vty->index);
		dir_info = 0; /*ingress*/
	}
	else if(ACL_EGRESS_GROUP_NODE==(vty->node)){
		acl_group_num = (unsigned int)(vty->index);
		dir_info = 1; /*egress*/
	}
	else{
		vty_out(vty,"%% unknown config mode\n");
		return CMD_WARNING;
	}
	/*vty_out(vty,"config acl_group %d\n",acl_group_num);*/
	for (i = 0; i < (endIndex - startIndex + 1); i++) {
		ruleIndex = startIndex + i;
		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
											NPD_DBUS_ACL_OBJPATH,
											NPD_DBUS_ACL_INTERFACE,
											NPD_DBUS_METHOD_ADD_ACL_TO_GROUP);
		
		dbus_error_init(&err);

		dbus_message_append_args(query,
								DBUS_TYPE_UINT32,&dir_info,
								DBUS_TYPE_UINT32,&op_flag,
								DBUS_TYPE_UINT32,&acl_group_num,
								DBUS_TYPE_UINT32,&ruleIndex,
								DBUS_TYPE_INVALID);
		
		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_acl,query,-1, &err);
		
		dbus_message_unref(query);
		if (NULL == reply) {
			vty_out(vty,"failed get reply.\n");
			if (dbus_error_is_set(&err)) {
				dbus_error_free_for_dcli(&err);
			}
			return CMD_SUCCESS;
		}

		if (dbus_message_get_args ( reply, &err,
			DBUS_TYPE_UINT32,&group_inf,
			DBUS_TYPE_UINT32,&ret,
			DBUS_TYPE_UINT32,&num,
			DBUS_TYPE_UINT32,&op_info,
			DBUS_TYPE_INVALID)) {
			
				if(group_inf==ACL_RETURN_CODE_ERROR)	
			 		vty_out(vty,"%% sorry,you should creat a acl group firstly!\n");
				else if(op_flag==0) {
					if((ACL_RETURN_CODE_GROUP_RULE_EXISTED==ret)&&(0!=num))					
						vty_out(vty,"%% rule %d has been added in ingress group %d!\n",(ruleIndex+1),num);
					else if((ACL_RETURN_CODE_EGRESS_GROUP_RULE_EXISTED==ret)&&(0!=num))
						vty_out(vty,"%% rule %d has been added in egress group %d!\n",(ruleIndex+1),num);									
					else if(ACL_RETURN_CODE_GLOBAL_NOT_EXISTED==ret)
						vty_out(vty,"%% acl %d has not been set!\n",(ruleIndex+1));
					else if(ACL_RETURN_CODE_ADD_EQUAL_RULE == op_info)
						vty_out(vty,"%% you add the equal rule in group !\n");
					else if (ACL_RETURN_CODE_ALREADY_PORT == op_info)
						vty_out(vty, "%% Qos mode is not flow, please change mode \n");
					else if (op_ret == ACL_RETURN_CODE_NO_QOS_MODE)
						vty_out(vty, "%% There is no qos mode, please config qos mode \n");
					else if(ACL_RETURN_CODE_GROUP_EGRESS_ERROR==op_info)
						vty_out(vty,"%% ingress port not support egress qos rule !\n");
					else if(ACL_RETURN_CODE_GROUP_EGRESS_NOT_SUPPORT==op_info)
						vty_out(vty,"%% egress port not support such action,including trap ,mirror,redirect,ingress qos!\n");
					else if(ACL_RETURN_CODE_STD_RULE == op_info)
						vty_out(vty,"%% added in the right group !\n");
					else if(ACL_RETURN_CODE_SUCCESS!=op_info)
						vty_out(vty,"%% add fail!\n");
				}
				else if(op_flag==1) {
					if(ACL_RETURN_CODE_GLOBAL_NOT_EXISTED==ret)
						vty_out(vty,"%% acl %d has not been set!\n",(ruleIndex+1));
					else if(ACL_RETURN_CODE_GROUP_RULE_EXISTED==ret){
						if(num!=acl_group_num){						
							if(dir_info==ACL_DIRECTION_INGRESS)
								vty_out(vty,"%% ingress group %d has no rule %d\n",acl_group_num,(ruleIndex+1));
							else if(dir_info==ACL_DIRECTION_EGRESS)
								vty_out(vty,"%% egress group %d has no rule %d\n",acl_group_num,(ruleIndex+1));
				
						}
						else if(num==acl_group_num) {
							if(ACL_RETURN_CODE_SUCCESS != op_info)
								vty_out(vty,"%% ingress delete fail!\n");
						}	
					}
					else if(ACL_RETURN_CODE_EGRESS_GROUP_RULE_EXISTED==ret){
						if(num!=acl_group_num)	{					
							if(dir_info==ACL_DIRECTION_INGRESS)
								vty_out(vty,"%% ingress group %d has no rule %d\n",acl_group_num,(ruleIndex+1));
							else if(dir_info==ACL_DIRECTION_EGRESS)
								vty_out(vty,"%% egress group %d has no rule %d\n",acl_group_num,(ruleIndex+1));			
						}
						else if(num==acl_group_num){
							if(ACL_RETURN_CODE_SUCCESS!=op_info)
								vty_out(vty,"%% egress delete fail!\n");
						}	
					}
					else if(ACL_RETURN_CODE_SUCCESS==ret){
						if(dir_info==ACL_DIRECTION_INGRESS)
							vty_out(vty,"%% ingress group %d has no this rule %d\n",acl_group_num,(ruleIndex+1));
						else if(dir_info==ACL_DIRECTION_EGRESS)
							vty_out(vty,"%% egress group %d has no this rule %d\n",acl_group_num,(ruleIndex+1));						
					}			
				}
				else if(ACL_RETURN_CODE_RULE_INDEX_ERROR==ret){
					vty_out(vty,"%% Acl rule range should be 1-1000!\n");
			}
		 }

		else {
			/*vty_out(vty,"Failed get args.\n");*/
			if (dbus_error_is_set(&err)) {
				dbus_error_free_for_dcli(&err);
			}
		}
		dbus_message_unref(reply);
	}
	return CMD_SUCCESS;

}

DEFUN(delete_acl_range_cmd_func,
      delete_acl_range_cmd,
      "delete acl-range <1-1000> <1-1000>",
      "Delete configuration \n"
      "Delete acl-range \n"
      "Standard acl index range in 1-1000,extended acl index range in 1-500\n"
      )
{
      DBusMessage 	*query = NULL, *reply = NULL;
	DBusError 	 err;
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	unsigned int  ruleIndex = 0, startIndex = 0, endIndex = 0,op_ret = 0;
	unsigned int   group_num = 0,i = 0,j=0,count=0;
	unsigned char   slot_no = 0,local_port_no = 0;
	int ret = 0;

	ret = dcli_str2ulong((char*)argv[0],&startIndex);
	if (ACL_RETURN_CODE_ERROR == ret) {
		vty_out(vty,"%% Illegal rule index!\n");
		return CMD_WARNING;
	}
	startIndex = startIndex-1;

	ret = dcli_str2ulong((char*)argv[1],&endIndex);
	if (ACL_RETURN_CODE_ERROR == ret) {
		vty_out(vty,"%% Illegal rule index!\n");
		return CMD_WARNING;
	}
	endIndex = endIndex-1;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_ACL_OBJPATH,NPD_DBUS_ACL_INTERFACE, NPD_DBUS_METHOD_DELETE_ACL_RANGE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
				 DBUS_TYPE_UINT32,&startIndex,
				 DBUS_TYPE_UINT32,&endIndex,
				 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_acl,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_UINT32,&group_num,
		DBUS_TYPE_INVALID))
	{						
		if (ACL_RETURN_CODE_ERROR == op_ret ) {
			vty_out(vty,"%% Range range error\n");
		}
		else if(op_ret==ACL_RETURN_CODE_GLOBAL_NOT_EXISTED)	{
			vty_out(vty,"%% Rule %d not existed!\n",(ruleIndex+1));
		}
		else if(op_ret == ACL_RETURN_CODE_GROUP_RULE_EXISTED){
			vty_out(vty,"%% Can't delete this acl since it is bound to ingress group %d\n",group_num);
		}	
		else if(op_ret ==ACL_RETURN_CODE_EGRESS_GROUP_RULE_EXISTED){
			vty_out(vty,"%% Can't delete this acl since it is bound to egress group %d\n",group_num);
		}
		else if(ACL_RETURN_CODE_RULE_INDEX_ERROR==op_ret){
			vty_out(vty,"%% Acl rule range should be 1-1000!\n");
		}
		else if(ACL_RETURN_CODEL_MIRROR_USE==op_ret){
			vty_out(vty,"%% Can't delete this acl since it used by mirror function!\n");
		}
		else if(ACL_RETURN_CODEL_RANGE_NOT_EXIST==op_ret){
			vty_out(vty,"%% This acl-range is not exist !\n");
		}
	}
	else 
	{
		/*vty_out(vty,"Failed get args.\n");*/
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);

	return CMD_SUCCESS;	
}

/*
DEFUN(config_acl_time_range_func,
	  config_acl_time_range,
	  "time-range TIME_RANGE_NAME",
	  "Configure time-range\n"
	  "Time range name,begins with char or'_', and name length no more than 20 characters\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;

	char*			timeName;
	unsigned int  timeId = 0;
	unsigned int    nameSize = 0;
	int ret;
	unsigned int op_ret;

	timeName = (char*)malloc(30);
	memset(timeName,'\0',30);

	ret = timeRange_name_legal_check((char*)argv[0],strlen(argv[0]));
	if(ALIAS_NAME_LEN_ERROR == ret) {
		vty_out(vty,"%% Bad parameter,timeRange name too long!\n");
		return CMD_WARNING;
	}
	else if(ALIAS_NAME_HEAD_ERROR == ret) {
		vty_out(vty,"%% Bad parameter,timeRange name begins with an illegal char!\n");
		return CMD_WARNING;
	}
	else if(ALIAS_NAME_BODY_ERROR == ret) {
		vty_out(vty,"%% Bad parameter,timeRange name contains illegal char!\n");
		return CMD_WARNING;
	}
	else{
		nameSize = strlen(argv[0]);
		memcpy(timeName,argv[0],nameSize);
		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
											NPD_DBUS_ACL_OBJPATH , \
											NPD_DBUS_ACL_INTERFACE ,	\
											NPD_DBUS_ACL_METHOD_SET_TIME_RANGE );
		
		dbus_error_init(&err);
		dbus_message_append_args(query,
								 DBUS_TYPE_STRING,&timeName, 
								 DBUS_TYPE_INVALID);

		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_acl,query,-1, &err);
	}
	
			
	dbus_message_unref(query);
	
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_UINT32, &timeId,
					DBUS_TYPE_INVALID)) 
	{
		
	   if(ACL_RETURN_CODE_ERROR == op_ret)   
	   	{
		   vty_out (vty, "%% Config time-range fail!\n");
	    }
		else
		{		
			if(CONFIG_NODE == vty->node) {
				vty->node = TIME_RANGE_NODE;
				vty->index = (void*)timeId;
			}
			else{
				vty_out (vty, "Terminal mode change must under configure mode!\n", VTY_NEWLINE);
				return CMD_WARNING;
			}
			
		}
		
	} 
	else 
	{
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}	
	}
	dbus_message_unref(reply);
	free(timeName);
	return CMD_SUCCESS;
}

DEFUN(set_absolute_time_cmd_func,
	  set_absolute_time_cmd,
	  "absolute start STARTIME end ENDTIME",
	  "Set absolute time\n"
	  "Absolute start time,format as (yyyy/mm/dd:hh:mm)\n"
	  "Absolute end time,format as (yyyy/mm/dd:hh:mm)\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;

	unsigned int	timeId = 0;
	int ret,tmp;
	unsigned int op_ret;
	unsigned int startyear=0,startmonth=0,startday=0,starthour=0,startminute=0;
	unsigned int endyear=0,endmonth=0,endday=0,endhour=0,endminute=0;

	if(TIME_RANGE_NODE==vty->node)
		timeId=(unsigned int)vty->index;
	
	ret = timeRange_absolute_deal((char*)argv[0],&startyear,&startmonth,&startday,&starthour,&startminute);	
	if(ret==ALIAS_NAME_LEN_ERROR){
		vty_out(vty,"%% Bad parameter,start time format too long!\n");
		return CMD_WARNING;
	}
	else if(ret==ACL_RETURN_CODE_ERROR){
		vty_out(vty,"%% Bad parameter,start time format illegal!\n");
		return CMD_WARNING;
	}	
	tmp=timeRange_time_check_illegal(startmonth,startday,starthour,startminute);
	if(tmp==ACL_FALSE){
		vty_out(vty,"%% Bad time parameter!\n");
		return CMD_WARNING;
	}
	ret = timeRange_absolute_deal((char*)argv[1],&endyear,&endmonth,&endday,&endhour,&endminute); 
	if(ret==ALIAS_NAME_LEN_ERROR){
		vty_out(vty,"%% Bad parameter,end time format too long!\n");
		return CMD_WARNING;
	}
	else if(ret==ACL_RETURN_CODE_ERROR){
		vty_out(vty,"%% Bad parameter,end time format illegal!\n");
		return CMD_WARNING;
	}
	tmp=timeRange_time_check_illegal(endmonth,endday,endhour,endminute);
	if(tmp==ACL_FALSE){
		vty_out(vty,"%% Bad time parameter!\n");
		return CMD_WARNING;
	}
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_ACL_OBJPATH , \
										NPD_DBUS_ACL_INTERFACE ,	\
										NPD_DBUS_ACL_METHOD_SET_ABSOLUTE);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&timeId,
							 DBUS_TYPE_UINT32,&startyear,
							 DBUS_TYPE_UINT32,&startmonth,
							 DBUS_TYPE_UINT32,&startday,
							 DBUS_TYPE_UINT32,&starthour,
							 DBUS_TYPE_UINT32,&startminute,
							 DBUS_TYPE_UINT32,&endyear,
							 DBUS_TYPE_UINT32,&endmonth,
							 DBUS_TYPE_UINT32,&endday,
							 DBUS_TYPE_UINT32,&endhour,
							 DBUS_TYPE_UINT32,&endminute,						 
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_acl,query,-1, &err);

	dbus_message_unref(query);
	
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) 
	{
		if (ACL_RETURN_CODE_SUCCESS != op_ret ) 
		{
			vty_out(vty,"%% set absolute time fail!\n"); 
		}
		
	} 
	else 
	{
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(set_periodic_time_cmd_func,
	  set_periodic_time_cmd,
	  "periodic <1-64> weekly VALUE STARTTIME to ENDTIME",
	  "Set periodic time\n"
	  "Support 64 periodic time\n"
	  "Execution weekly\n"
	  "Periodic value eg:(Monday|Tuesday|Wednesday|Thursday|Friday|Saturday|Sunday|weekdays|weekend|daily)\n"
	  "Start time,format in (hh:mm)\n"
	  "End time,format in (hh:mm)\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;

	unsigned int	timeId = 0;
	int ret,tmp;
	unsigned int op_ret;
	unsigned int period=0;
	unsigned int flag=0,starthour=0,startmin=0,endhour=0,endmin=0;
	
	if(TIME_RANGE_NODE==vty->node)
		timeId=(unsigned int)vty->index;

	ret = dcli_str2ulong((char *)argv[0],&period);
	if(ret==ACL_RETURN_CODE_ERROR)
	{
		vty_out(vty,"%% Illegal periodic value!\n");
		return CMD_WARNING ;
	}
	if(strncmp(argv[1],"Monday",strlen(argv[1]))==0)
		flag=1;
	else if(strncmp(argv[1],"Tuesday",strlen(argv[1]))==0)
		flag=2;
    else if(strncmp(argv[1],"Wednesday",strlen(argv[1]))==0)
		flag=3;
    else if(strncmp(argv[1],"Thursday",strlen(argv[1]))==0)
		flag=4;
	else if(strncmp(argv[1],"Friday",strlen(argv[1]))==0)
		flag=5;
	else if(strncmp(argv[1],"Saturday",strlen(argv[1]))==0)
		flag=6;
	else if(strncmp(argv[1],"Sunday",strlen(argv[1]))==0)
		flag=7;
	else if(strncmp(argv[1],"weekdays",strlen(argv[1]))==0)
		flag=8;
	else if(strncmp(argv[1],"weekend",strlen(argv[1]))==0)
		flag=9;
	else if(strncmp(argv[1],"daily",strlen(argv[1]))==0)
		flag=0;
	else{
		vty_out(vty,"%% unknown command!\n");
		return CMD_WARNING;
	}		
	ret = timeRange_time_deal((char*)argv[2],&starthour,&startmin); 

	if(ret==ALIAS_NAME_LEN_ERROR){
		vty_out(vty,"%% Bad parameter,start time format too long!\n");
		return CMD_WARNING;
	}
	else if(ret==ACL_RETURN_CODE_ERROR){
		vty_out(vty,"%% Bad parameter,start time format illegal!\n");
		return CMD_WARNING;
	}	
	tmp=timeRange_time_hour_check_illegal(starthour,startmin);
	if(tmp==ACL_FALSE){
		vty_out(vty,"%% Bad time parameter!\n");
		return CMD_WARNING;
	}
	ret = timeRange_time_deal((char*)argv[3],&endhour,&endmin); 
	if(ret==ALIAS_NAME_LEN_ERROR){
		vty_out(vty,"%% Bad parameter,end time format too long!\n");
		return CMD_WARNING;
	}
	else if(ret==ACL_RETURN_CODE_ERROR){
		vty_out(vty,"%% Bad parameter,end time format illegal!\n");
		return CMD_WARNING;
	}	
	tmp=timeRange_time_hour_check_illegal(endhour,endmin);
	if(tmp==ACL_FALSE){
		vty_out(vty,"%% Bad time parameter!\n");
		return CMD_WARNING;
	}
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_ACL_OBJPATH , \
										NPD_DBUS_ACL_INTERFACE ,	\
										NPD_DBUS_ACL_METHOD_SET_PERIODIC);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,	
							 DBUS_TYPE_UINT32,&timeId,
							 DBUS_TYPE_UINT32,&period,
							 DBUS_TYPE_UINT32,&flag,
							 DBUS_TYPE_UINT32,&starthour,
							 DBUS_TYPE_UINT32,&startmin,						 
							 DBUS_TYPE_UINT32,&endhour,
							 DBUS_TYPE_UINT32,&endmin,							 
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_acl,query,-1, &err);

	dbus_message_unref(query);
	
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) 
	{
		if(ACL_TIME_PERIOD_EXISTED==op_ret){
			vty_out(vty,"%% Error! period %d has existed!\n",period);
		}
		else if (ACL_RETURN_CODE_SUCCESS != op_ret ) 
		{
			vty_out(vty,"%% set periodic time fail!\n"); 
		}
		
	} 
	else 
	{
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(show_acl_time_range_func,
	  show_acl_time_range,
	  "show time-range",
	  SHOW_STR
	  "Show time range configuration\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;

	int ret;
	unsigned int op_ret;
	char *timeName=NULL;
	unsigned int startyear=0,startmonth=0,startday=0,starthour=0,startminute=0;
	unsigned int endyear=0,endmonth=0,endday=0,endhour=0,endminute=0;
	unsigned int period=0, flag=0,starth=0,startm=0,endh=0,endm=0;
	unsigned int time_count=0,perid_count=0;
	unsigned int i=0,j=0,k=0,index=0;
	unsigned char flagstr[15];
	
	memset(flagstr,'0',15);
		
	timeName=(char*)malloc(30);
	memset(timeName,0,30);

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_ACL_OBJPATH , \
										NPD_DBUS_ACL_INTERFACE ,	\
										NPD_DBUS_ACL_METHOD_SHOW_TIME_RANGE);
	
	dbus_error_init(&err);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_acl,query,-1, &err);

	dbus_message_unref(query);
	
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	 dbus_message_iter_init(reply,&iter); 
	 dbus_message_iter_get_basic(&iter,&ret);
	 
	 if(ACL_TIME_NAME_NOTEXIST ==ret){
		vty_out(vty,"%% No time range information existed!\n");		
	 }
	 else if((ACL_TIME_PERIOD_NOT_EXISTED==ret)||(ACL_RETURN_CODE_SUCCESS==ret))
	 {			
		dbus_message_iter_next(&iter);	
	 	dbus_message_iter_get_basic(&iter,&time_count); 

		//vty_out(vty,"ret %d,time_count %d\n",ret,time_count);
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);

		for(i=0;i<time_count;i++){
			 DBusMessageIter iter_struct;
			 DBusMessageIter iter_sub_array;
			 
	 		 dbus_message_iter_recurse(&iter_array,&iter_struct);	
			// dbus_message_iter_next(&iter_struct);  
			 dbus_message_iter_get_basic(&iter_struct,&timeName); 
			 dbus_message_iter_next(&iter_struct);  
			 dbus_message_iter_get_basic(&iter_struct,&startyear); 
			 dbus_message_iter_next(&iter_struct);  
			 dbus_message_iter_get_basic(&iter_struct,&startmonth); 
			 dbus_message_iter_next(&iter_struct);  
			 dbus_message_iter_get_basic(&iter_struct,&startday); 
			 dbus_message_iter_next(&iter_struct);  
			 dbus_message_iter_get_basic(&iter_struct,&starthour); 
			 dbus_message_iter_next(&iter_struct);  
			 dbus_message_iter_get_basic(&iter_struct,&startminute); 
			 dbus_message_iter_next(&iter_struct);  
			 dbus_message_iter_get_basic(&iter_struct,&endyear); 
			 dbus_message_iter_next(&iter_struct);  
			 dbus_message_iter_get_basic(&iter_struct,&endmonth); 
			 dbus_message_iter_next(&iter_struct);  
			 dbus_message_iter_get_basic(&iter_struct,&endday); 
			 dbus_message_iter_next(&iter_struct);  
			 dbus_message_iter_get_basic(&iter_struct,&endhour); 
			 dbus_message_iter_next(&iter_struct);  
			 dbus_message_iter_get_basic(&iter_struct,&endminute); 
			 dbus_message_iter_next(&iter_struct);  
			 dbus_message_iter_get_basic(&iter_struct,&perid_count);
			 vty_out(vty,"=============================================\n");
			 vty_out(vty,"%-20s: %s\n","time-range name",timeName);
			 vty_out(vty,"%-20s: %d/%d/%d:%d:%d\n","absolute start",startyear,startmonth,startday,starthour,startminute);
			 vty_out(vty,"%-20s: %d/%d/%d:%d:%d\n","absolute end",endyear,endmonth,endday,endhour,endminute);
			

			 dbus_message_iter_next(&iter_struct);		  
			 dbus_message_iter_recurse(&iter_struct,&iter_sub_array);
			
				for (j = 0; j < perid_count; j++){
				  DBusMessageIter iter_sub_struct;
				  dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
				  
				  dbus_message_iter_get_basic(&iter_sub_struct,&index);
				  dbus_message_iter_next(&iter_sub_struct);			  
				  dbus_message_iter_get_basic(&iter_sub_struct,&flag);
				  dbus_message_iter_next(&iter_sub_struct);
			  	  dbus_message_iter_get_basic(&iter_sub_struct,&starth);
				  dbus_message_iter_next(&iter_sub_struct);
				  dbus_message_iter_get_basic(&iter_sub_struct,&startm);
				  dbus_message_iter_next(&iter_sub_struct);
			  	  dbus_message_iter_get_basic(&iter_sub_struct,&endh);
				  dbus_message_iter_next(&iter_sub_struct);
				  dbus_message_iter_get_basic(&iter_sub_struct,&endm);
				  dbus_message_iter_next(&iter_sub_struct);

				  switch(flag)
				  {
					case 0:	strcpy(flagstr,"daily");break;
					case 1: strcpy(flagstr,"Monday");break;
					case 2: strcpy(flagstr,"Tuesday");break;
					case 3: strcpy(flagstr,"Wednesday");break;
					case 4: strcpy(flagstr,"Thursday");break;
					case 5: strcpy(flagstr,"Friday");break;
					case 6: strcpy(flagstr,"Saturday");break;
					case 7: strcpy(flagstr,"Sunday");break;
					case 8: strcpy(flagstr,"weekdays");break;
					case 9: strcpy(flagstr,"weekend");break;
					default :
						break;
				  }
			      vty_out(vty,"%-20s: %d %s %s %d:%d %s %d:%d\n","periodic",index,"weekly",flagstr,starth,startm,"to",endh,endm);
					 
				 dbus_message_iter_next(&iter_sub_array);
			  } 
			
			vty_out(vty,"===========================================\n");
			dbus_message_iter_next(&iter_array);
		}	
	 }
	
	dbus_message_unref(reply);
	return CMD_SUCCESS;
	
}
DEFUN(acl_rule_bind_time_range_func,
	  acl_rule_bind_time_range,
	  "append acl INDEX time-range TIMENAME",
	  "Append acl configuration\n"
	  ACL_STR
	  "Specify standard rule range in 1-1024,extended rule range in 1-512\n"	  	 
	  "Attach acl with time range attributes\n"
	  "Time range name\n"
)
{
	DBusMessage 	*query = NULL, *reply = NULL;
	DBusError 		err = { 0 };
	unsigned int 	ruleIndex;
	unsigned int	ret,tmp,op_ret;
	char	*timeName=NULL;
	
	timeName = (char*)malloc(30);
	memset(timeName,'\0',30);
	
	ret = dcli_str2ulong((char*)argv[0],&ruleIndex);
	if (ACL_RETURN_CODE_ERROR == ret) {
		vty_out(vty,"%% Illegal rule index!\n");
		return CMD_WARNING;
	}
	ruleIndex = ruleIndex-1;
	
	tmp = timeRange_name_legal_check((char*)argv[1],strlen(argv[1]));
	if(ALIAS_NAME_LEN_ERROR == tmp) {
		vty_out(vty,"%% Bad parameter,timeRange name too long!\n");
		return CMD_WARNING;
	}
	else if(ALIAS_NAME_HEAD_ERROR == tmp) {
		vty_out(vty,"%% Bad parameter,timeRange name begins with an illegal char!\n");
		return CMD_WARNING;
	}
	else if(ALIAS_NAME_BODY_ERROR == tmp) {
		vty_out(vty,"%% Bad parameter,timeRange name contains illegal char!\n");
		return CMD_WARNING;
	}
	else{
		
		memcpy(timeName,argv[1],strlen(argv[1]));
		
		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
											NPD_DBUS_ACL_OBJPATH , \
											NPD_DBUS_ACL_INTERFACE ,	\
											NPD_DBUS_ACL_METHOD_ACL_TIME_RANGE );
		
		dbus_error_init(&err);
		dbus_message_append_args(query,
								 DBUS_TYPE_UINT32,ruleIndex,
								 DBUS_TYPE_STRING,&timeName, 
								 DBUS_TYPE_INVALID);

		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_acl,query,-1, &err);
	}			
	dbus_message_unref(query);
	
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) 
	{
		if (ACL_RETURN_CODE_GLOBAL_NOT_EXISTED == op_ret ) 
		{
			vty_out(vty,"%% Error! Acl rule %d not existed!\n",ruleIndex); 
		}
		else  if(ACL_TIME_NAME_NOTEXIST == op_ret)   
		{
			vty_out(vty,"%% Error! Time range %s not existed!\n",timeName); 
			
		}
		else if(ACL_RETURN_CODE_RULE_TIME_NOT_SUPPORT==op_ret){
			vty_out(vty,"%% Error! Time range just support acl action with permit or deny!\n");
		}
	} 
	else 
	{
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}	
	}
	dbus_message_unref(reply);
	free(timeName);
	return CMD_SUCCESS;		

}
*/
#if 0
void dcli_acl_init(void)  
{
  /* init the dbus connect to the local board */
  dcli_dbus_connection_acl = dcli_dbus_connection;
  dcli_dbus_connection_acl_port	= dcli_dbus_connection;
  install_node (&acl_group_node, dcli_acl_group_show_running_config);
  install_default(ACL_GROUP_NODE);
  install_node (&egress_acl_group_node, dcli_acl_qos_show_running_config);
  install_default(ACL_EGRESS_GROUP_NODE);
  install_node (&time_range_node, NULL);
  install_default(TIME_RANGE_NODE);


  install_element(VIEW_NODE,&show_acl_group_cmd);
  install_element(VIEW_NODE,&show_acl_group_index_cmd);
  install_element(ENABLE_NODE,&show_acl_group_cmd);
  install_element(ENABLE_NODE,&show_acl_group_index_cmd);  
  install_element(VIEW_NODE,&show_acl_service_cmd);
  install_element(VIEW_NODE,&show_acl_lists_cmd);
  install_element(VIEW_NODE,&show_acl_index_cmd);
  install_element(VIEW_NODE,&show_ip_acl_lists_cmd);
  install_element(VIEW_NODE,&show_ip_acl_index_cmd);
  install_element(ENABLE_NODE,&show_acl_service_cmd);
  install_element(ENABLE_NODE,&show_acl_lists_cmd);
  install_element(ENABLE_NODE,&show_acl_index_cmd); 
  install_element(ENABLE_NODE,&show_ip_acl_lists_cmd);
  install_element(ENABLE_NODE,&show_ip_acl_index_cmd);
  install_element(CONFIG_NODE,&config_acl_service_enable_cmd);
  install_element(CONFIG_NODE,&show_acl_service_cmd);
  install_element(CONFIG_NODE,&show_acl_lists_cmd);
  install_element(CONFIG_NODE,&show_ip_acl_lists_cmd);
  install_element(CONFIG_NODE,&show_ip_acl_index_cmd);
  install_element(CONFIG_NODE,&show_acl_index_cmd);
  install_element(CONFIG_NODE,&delete_acl_cmd);
  install_element(CONFIG_NODE,&config_acl_on_board_cmd);
 
  /*standard rule*/
  /*trap*/
  install_element(CONFIG_NODE,&config_acl_rule_trap_ip_cmd);
  install_element(CONFIG_NODE,&config_acl_rule_trap_tcp_cmd);
  install_element(CONFIG_NODE,&config_acl_rule_trap_udp_cmd);
  install_element(CONFIG_NODE,&config_acl_rule_trap_icmp_cmd);
  install_element(CONFIG_NODE,&config_acl_rule_trap_to_cpu_ethernet_cmd);
  install_element(CONFIG_NODE,&config_acl_rule_trap_to_cpu_ethertype_cmd);
  install_element(CONFIG_NODE,&config_acl_rule_trap_arp_cmd);
  /*permit deny*/
  install_element(CONFIG_NODE,&config_acl_rule_deny_tcp_or_udp_cmd);
  install_element(CONFIG_NODE,&config_acl_rule_deny_mac_cmd);
  install_element(CONFIG_NODE,&config_acl_rule_deny_ethertype_cmd);
  install_element(CONFIG_NODE,&config_acl_rule_deny_ip_cmd);
  install_element(CONFIG_NODE,&config_acl_rule_deny_ipv6_cmd);
  install_element(CONFIG_NODE,&config_extend_acl_rule_permit_deny_ipv6_cmd);
  install_element(CONFIG_NODE,&config_acl_rule_deny_icmp_cmd);
  install_element(CONFIG_NODE,&config_acl_rule_permit_deny_arp_cmd);
  /*mirror redirect*/
  install_element(CONFIG_NODE,&config_acl_rule_mirror_redirect_ip_cmd);
  install_element(CONFIG_NODE,&config_acl_rule_mirror_redirect_icmp_cmd);
  install_element(CONFIG_NODE,&config_acl_rule_mirror_redirect_tcp_or_udp_cmd);
  install_element(CONFIG_NODE,&config_acl_rule_mirror_redirect_mac_cmd);
  install_element(CONFIG_NODE,&config_acl_rule_mirror_redirect_arp_cmd);
  /*extend rule*/
  install_element(CONFIG_NODE,&config_extend_acl_rule_trap_permit_deny_cmd);
  install_element(CONFIG_NODE,&config_extend_acl_rule_mirror_redirect_cmd);

 
  /*acl group*/ 
  install_element(CONFIG_NODE,&config_acl_group_cmd);
  install_element(CONFIG_NODE,&create_acl_group_cmd);
  install_element(CONFIG_NODE,&delete_acl_group_cmd);
  install_element(CONFIG_NODE,&show_acl_group_cmd);
  install_element(CONFIG_NODE,&show_acl_group_index_cmd);
  install_element(ACL_GROUP_NODE,&add_delete_acl_rule_to_acl_group_cmd);
  install_element(ACL_EGRESS_GROUP_NODE,&add_delete_acl_rule_to_acl_group_cmd);
  /*interface */
  install_element(ETH_PORT_NODE,&config_ethport_acl_enable_cmd);
  install_element(ETH_PORT_NODE,&show_bind_ethport_acl_Info_cmd);
  install_element(ETH_PORT_NODE,&bind_acl_group_to_ethport_cmd);
  install_element(ETH_PORT_NODE,&no_acl_group_on_ethport_cmd);

  /*vlan*/
  install_element(VLAN_NODE,&config_ethport_acl_enable_cmd);
  install_element(VLAN_NODE,&show_bind_ethport_acl_Info_cmd);
  install_element(VLAN_NODE,&bind_acl_group_to_ethport_cmd);
  install_element(VLAN_NODE,&no_acl_group_on_ethport_cmd);

  /*ip-range*/
  install_element(CONFIG_NODE,&config_acl_std_dsip_range_cmd);
  install_element(CONFIG_NODE,&config_acl_std_dsip_range_policer_cmd);
  install_element(CONFIG_NODE,&config_acl_std_dsip_range_policer_range_cmd);
  
  install_element(CONFIG_NODE,&config_acl_std_dip_range_cmd);
  install_element(CONFIG_NODE,&config_acl_std_dip_range_policer_cmd);
  install_element(CONFIG_NODE,&config_acl_std_dip_range_policer_range_cmd);
 
  install_element(CONFIG_NODE,&config_acl_std_sip_range_cmd);
  install_element(CONFIG_NODE, &config_acl_std_sip_range_policer_cmd);
  install_element(CONFIG_NODE,&config_acl_std_sip_range_policer_range_cmd);
  install_element(CONFIG_NODE,&config_acl_std_permit_sip_range_cmd);
  install_element(CONFIG_NODE,&config_acl_std_permit_sip_policer_range_cmd);

  install_element(CONFIG_NODE,&delete_acl_range_cmd);
  install_element(ACL_GROUP_NODE,&add_delete_acl_rule_to_acl_range_group_cmd);
  install_element(ACL_EGRESS_GROUP_NODE,&add_delete_acl_rule_to_acl_range_group_cmd);
  install_element(CONFIG_NODE,&config_acl_rule_redirect_ipv6_cmd);
  install_element(CONFIG_NODE,&config_extend_acl_rule_redirect_tcp_udp_ipv6_cmd);
  
  /*time-range*/
  /*
  install_element(CONFIG_NODE,		&config_acl_time_range);
  install_element(TIME_RANGE_NODE,	&set_absolute_time_cmd);
  install_element(TIME_RANGE_NODE,	&set_periodic_time_cmd);
  install_element(CONFIG_NODE,		&show_acl_time_range);
  install_element(CONFIG_NODE,		&delete_acl_time_range);
  install_element(CONFIG_NODE,		&acl_rule_bind_time_range);
  */
}
#else
void dcli_acl_init(void)  
{
  /* init the dbus connect to the local board, config acl */
  dcli_dbus_connection_acl = dcli_dbus_connection;
  /* bind acl group to eth-port */
  dcli_dbus_connection_acl_port	= dcli_dbus_connection;


  install_node (&acl_node_distributed, NULL,"ACL_NODE_DISTRIBUTED");
  install_default(ACL_NODE_DISTRIBUTED);

  
  install_node (&acl_group_node, dcli_acl_group_show_running_config, "ACL_GROUP_NODE");
  install_default(ACL_GROUP_NODE);
  install_node (&egress_acl_group_node, dcli_acl_qos_show_running_config, "ACL_EGRESS_GROUP_NODE");
  install_default(ACL_EGRESS_GROUP_NODE);
  install_node (&time_range_node, NULL, "TIME_RANGE_NODE");
  install_default(TIME_RANGE_NODE);


  install_element(CONFIG_NODE,&config_acl_on_board_cmd);    /* config node */
  /*
  install_element(VIEW_NODE,&show_acl_group_cmd);
  install_element(VIEW_NODE,&show_acl_group_index_cmd);
  install_element(ENABLE_NODE,&show_acl_group_cmd);
  install_element(ENABLE_NODE,&show_acl_group_index_cmd);  
  install_element(VIEW_NODE,&show_acl_service_cmd);
  install_element(VIEW_NODE,&show_acl_lists_cmd);
  install_element(VIEW_NODE,&show_acl_index_cmd);
  install_element(VIEW_NODE,&show_ip_acl_lists_cmd);
  install_element(VIEW_NODE,&show_ip_acl_index_cmd);
  install_element(ENABLE_NODE,&show_acl_service_cmd);
  install_element(ENABLE_NODE,&show_acl_lists_cmd);
  install_element(ENABLE_NODE,&show_acl_index_cmd); 
  install_element(ENABLE_NODE,&show_ip_acl_lists_cmd);
  install_element(ENABLE_NODE,&show_ip_acl_index_cmd);
  */
  install_element(ACL_NODE_DISTRIBUTED,&config_acl_service_enable_cmd);
  install_element(ACL_NODE_DISTRIBUTED,&show_acl_service_cmd);
  install_element(ACL_NODE_DISTRIBUTED,&show_acl_lists_cmd);
  install_element(ACL_NODE_DISTRIBUTED,&show_ip_acl_lists_cmd);
  install_element(ACL_NODE_DISTRIBUTED,&show_ip_acl_index_cmd);
  install_element(ACL_NODE_DISTRIBUTED,&show_acl_index_cmd);
  install_element(ACL_NODE_DISTRIBUTED,&delete_acl_cmd);
 
  /*standard rule*/
  /*trap*/
  install_element(ACL_NODE_DISTRIBUTED,&config_acl_rule_trap_ip_cmd);
  install_element(ACL_NODE_DISTRIBUTED,&config_acl_rule_trap_tcp_cmd);
  install_element(ACL_NODE_DISTRIBUTED,&config_acl_rule_trap_udp_cmd);
  install_element(ACL_NODE_DISTRIBUTED,&config_acl_rule_trap_icmp_cmd);
  install_element(ACL_NODE_DISTRIBUTED,&config_acl_rule_trap_to_cpu_ethernet_cmd);
  install_element(ACL_NODE_DISTRIBUTED,&config_acl_rule_trap_to_cpu_ethertype_cmd);
  install_element(ACL_NODE_DISTRIBUTED,&config_acl_rule_trap_arp_cmd);
  /*permit deny*/
  install_element(ACL_NODE_DISTRIBUTED,&config_acl_rule_deny_tcp_or_udp_cmd);
  install_element(ACL_NODE_DISTRIBUTED,&config_acl_rule_deny_mac_cmd);
  install_element(ACL_NODE_DISTRIBUTED,&config_acl_rule_deny_ethertype_cmd);
  install_element(ACL_NODE_DISTRIBUTED,&config_acl_rule_deny_ip_cmd);
  install_element(ACL_NODE_DISTRIBUTED,&config_acl_rule_deny_ipv6_cmd);
  install_element(ACL_NODE_DISTRIBUTED,&config_extend_acl_rule_permit_deny_ipv6_cmd);
  install_element(ACL_NODE_DISTRIBUTED,&config_acl_rule_deny_icmp_cmd);
  install_element(ACL_NODE_DISTRIBUTED,&config_acl_rule_permit_deny_arp_cmd);
  /*mirror redirect*/
  install_element(ACL_NODE_DISTRIBUTED,&config_acl_rule_mirror_redirect_ip_cmd);
  install_element(ACL_NODE_DISTRIBUTED,&config_acl_rule_mirror_redirect_icmp_cmd);
  install_element(ACL_NODE_DISTRIBUTED,&config_acl_rule_mirror_redirect_tcp_or_udp_cmd);
  install_element(ACL_NODE_DISTRIBUTED,&config_acl_rule_mirror_redirect_mac_cmd);
  install_element(ACL_NODE_DISTRIBUTED,&config_acl_rule_mirror_redirect_arp_cmd);
  /*extend rule*/
  install_element(ACL_NODE_DISTRIBUTED,&config_extend_acl_rule_trap_permit_deny_cmd);
  install_element(ACL_NODE_DISTRIBUTED,&config_extend_acl_rule_mirror_redirect_cmd);

 
  /*acl group*/ 
  install_element(ACL_NODE_DISTRIBUTED,&config_acl_group_cmd);
  install_element(ACL_NODE_DISTRIBUTED,&create_acl_group_cmd);
  install_element(ACL_NODE_DISTRIBUTED,&delete_acl_group_cmd);
  install_element(ACL_NODE_DISTRIBUTED,&show_acl_group_cmd);
  install_element(ACL_NODE_DISTRIBUTED,&show_acl_group_index_cmd);
  
  install_element(ACL_GROUP_NODE,&add_delete_acl_rule_to_acl_group_cmd);
  install_element(ACL_EGRESS_GROUP_NODE,&add_delete_acl_rule_to_acl_group_cmd);
  /*interface */
  install_element(ETH_PORT_NODE,&config_ethport_acl_enable_cmd);
  install_element(ETH_PORT_NODE,&show_bind_ethport_acl_Info_cmd);
  install_element(ETH_PORT_NODE,&bind_acl_group_to_ethport_cmd);
  install_element(ETH_PORT_NODE,&no_acl_group_on_ethport_cmd);

  /*vlan*/
  install_element(VLAN_NODE,&config_ethport_acl_enable_cmd);
  install_element(VLAN_NODE,&show_bind_ethport_acl_Info_cmd);
  install_element(VLAN_NODE,&bind_acl_group_to_ethport_cmd);
  install_element(VLAN_NODE,&no_acl_group_on_ethport_cmd);

  /*ip-range*/
  install_element(ACL_NODE_DISTRIBUTED,&config_acl_std_dsip_range_cmd);
  install_element(ACL_NODE_DISTRIBUTED,&config_acl_std_dsip_range_policer_cmd);
  install_element(ACL_NODE_DISTRIBUTED,&config_acl_std_dsip_range_policer_range_cmd);
  
  install_element(ACL_NODE_DISTRIBUTED,&config_acl_std_dip_range_cmd);
  install_element(ACL_NODE_DISTRIBUTED,&config_acl_std_dip_range_policer_cmd);
  install_element(ACL_NODE_DISTRIBUTED,&config_acl_std_dip_range_policer_range_cmd);
 
  install_element(ACL_NODE_DISTRIBUTED,&config_acl_std_sip_range_cmd);
  install_element(ACL_NODE_DISTRIBUTED, &config_acl_std_sip_range_policer_cmd);
  install_element(ACL_NODE_DISTRIBUTED,&config_acl_std_sip_range_policer_range_cmd);
  install_element(ACL_NODE_DISTRIBUTED,&config_acl_std_permit_sip_range_cmd);
  install_element(ACL_NODE_DISTRIBUTED,&config_acl_std_permit_sip_policer_range_cmd);

  install_element(ACL_NODE_DISTRIBUTED,&delete_acl_range_cmd);
  install_element(ACL_GROUP_NODE,&add_delete_acl_rule_to_acl_range_group_cmd);
  install_element(ACL_EGRESS_GROUP_NODE,&add_delete_acl_rule_to_acl_range_group_cmd);
  install_element(ACL_NODE_DISTRIBUTED,&config_acl_rule_redirect_ipv6_cmd);
  install_element(ACL_NODE_DISTRIBUTED,&config_extend_acl_rule_redirect_tcp_udp_ipv6_cmd);
  
}
#endif


#ifdef __cplusplus
}
#endif
