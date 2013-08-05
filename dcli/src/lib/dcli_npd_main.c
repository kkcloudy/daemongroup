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
* dcli_vlan.c
*
*
* CREATOR:
*		qinhs@autelan.com
*
* DESCRIPTION:
*		CLI definition for VLAN module.
*
* DATE:
*		02/21/2008	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.112 $	
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

#include <zebra.h>
#include <stdlib.h>
#include <dbus/dbus.h>
#include "command.h"

#include "sysdef/npd_sysdef.h"
#include "dbus/npd/npd_dbus_def.h"
#include "util/npd_list.h"
#include "npd/nam/npd_amapi.h"
#include "if.h"
#include "dcli_vlan.h"
#include "dcli_trunk.h"
#include "sysdef/returncode.h"
#include "npd/nbm/npd_bmapi.h"

#include "dcli_main.h"
#include "dcli_sem.h"
#include "dcli_intf.h"
#include "board/board_define.h"  /* AC_BOARD */

extern int is_distributed;



#if 1/*****from dcli_routesyn.c*****/


/* Convert IP address's netmask such as 255.255.255.0 ,into integer. */

unsigned char get_ip_masklen (const char *cp)
{
  unsigned int netmask;
  unsigned char len;
  unsigned char *pnt;
  unsigned char *end;
  unsigned char val;

  if(1 != inet_atoi(cp,&netmask))
  {
		return 0;
  }

  len = 0;
  pnt = (unsigned char *) &netmask;
  end = pnt + 4;

  while ((pnt < end) && (*pnt == 0xff))
    {
      len+= 8;
      pnt++;
    } 

  if (pnt < end)
    {
      val = *pnt;
      while (val)
	{
	  len++;
	  val <<= 1;
	}
    }
  return len;
}



int
inet_atoi (const char *cp, unsigned int  *inaddr)
{
  int dots = 0;
  register u_long addr = 0;
  register u_long val = 0, base = 10;

  do
    {
      register char c = *cp;

      switch (c)
	{
	case '0': case '1': case '2': case '3': case '4': case '5':
	case '6': case '7': case '8': case '9':
	  val = (val * base) + (c - '0');
	  break;
	case '.':
	  if (++dots > 3)
	    return 0;
	case '\0':
	  if (val > 255)
	    return 0;
	  addr = addr << 8 | val;
	  val = 0;
	  break;
	default:
	  return 0;
	}
    } while (*cp++) ;

  if (dots < 3)
    addr <<= 8 * (3 - dots);
  if (inaddr)
    *inaddr = htonl (addr);
  return 1;
}

#endif



#if 1 /***wangchao moved from dcli_acl.c****/

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



#define DCLI_IPMASK_STRING_MAXLEN	(strlen("255.255.255.255/32"))
#define DCLI_IPMASK_STRING_MINLEN	(strlen("0.0.0.0/0"))
#define DCLI_IP_STRING_MAXLEN	(strlen("255.255.255.255"))
#define DCLI_IP_STRING_MINLEN   (strlen("0.0.0.0"))

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


/*****ip_long2str: wangchao moved from dcli_acl.c**********/

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



struct cmd_node vlan_node = 
{
	VLAN_NODE,
	"%s(config-vlan)# ",
	1
};
struct cmd_node vlan_egress_node = 
{
	VLAN_EGRESS_NODE,
	" ",
	1
};

#endif /*end of dcli_acl.c*/





#if 1 /*moved from dcli_vlan.c*/

/*********wangchao: moved from dcli_vlan.c************

int parse_vlan_no(char* str,unsigned short* vlanId)
int parse_single_param_no(char* str,unsigned short* sig_param)
int parse_slotno_localport(char* str,unsigned int *slotno,unsigned int *portno) 
int parse_slotno_localport(char* str,unsigned int *slotno,unsigned int *portno) 
int parse_slotno_localport_include_slot0(char* str,unsigned int *slotno,unsigned int *portno) 
int param_first_char_check(char* str,unsigned int cmdtip)


******************************************************/
extern DBusConnection *dcli_dbus_connection_igmp;


int parse_vlan_no(char* str,unsigned short* vlanId) 
{
	char *endptr = NULL;
	char c;
	if (NULL == str) return NPD_FAIL;
	c = str[0];
	if (c>'0'&&c<='9'){
		*vlanId= strtoul(str,&endptr,10);
		if('\0' != endptr[0]){
			return NPD_FAIL;
		}
		return NPD_SUCCESS;	
	}
	else {
		return NPD_FAIL; /*not Vlan ID. for Example ,enter '.',and so on ,some special char.*/
	}
}

int parse_single_param_no(char* str,unsigned short* sig_param) 
{
	char *endptr = NULL;

	if (NULL == str) return NPD_FAIL;
	*sig_param= strtoul(str,&endptr,10);
	return NPD_SUCCESS;	
}


int parse_slotno_localport(char* str,unsigned int *slotno,unsigned int *portno) 
{
	char *endptr = NULL;
	char c = 0;
	if (NULL == str) return -1;
	/* add for AX7605i-alpha cscd port by qinhs@autelan.com 2009-11-18 */
	if(!strncmp(tolower(str), "cscd", 4)) {
		*slotno = AX7i_XG_CONNECT_SLOT_NUM;
		if(strlen(str) > strlen("cscd*")) {
			return NPD_FAIL ;
		}
		else if('0' == str[4]) {
			*portno = 1;
		}		
		else if('1' == str[4]) {
			*portno = 2;
		}
		else {
			return NPD_FAIL;
		}
		return NPD_SUCCESS;
	}
	c = str[0];
	if (c>='0' && c<='9'){
		*slotno= strtoul(str,&endptr,10);
		if(SLOT_PORT_SPLIT_SLASH != endptr[0] &&
            SLOT_PORT_SPLIT_DASH != endptr[0]){
		    return -1;
		}
		else {
             *portno = strtoul(&endptr[1],&endptr,10);
             if( 52 < *portno ||'\0' != endptr[0])/*for au5000 :24; for ax7000 : 6*/
			 	return 1;
        }
		return 0;	
	}
	else {
		return -1; /*not Vlan ID. for Example ,enter '.',and so on ,some special char.*/
	}
}

int parse_slotno_localport_include_slot0(char* str,unsigned int *slotno,unsigned int *portno) 
{
	char *endptr = NULL;
	char c = 0;
	if (NULL == str) return -1;
	
	/* add for AX7605i-alpha cscd port by qinhs@autelan.com 2009-11-18 */
	if(!strncmp(tolower(str), "cscd", 4)) {
		*slotno = AX7i_XG_CONNECT_SLOT_NUM;
		if(strlen(str) > strlen("cscd*")) {
			return NPD_FAIL ;
		}
		else if('0' == str[4]) {
			*portno = 1;
		}		
		else if('1' == str[4]) {
			*portno = 2;
		}
		else {
			return NPD_FAIL;
		}
		return NPD_SUCCESS;
	}

	c = str[0];
	if (c>='0' && c<='9'){
		*slotno= strtoul(str,&endptr,10);
		if(SLOT_PORT_SPLIT_SLASH != endptr[0] &&
            SLOT_PORT_SPLIT_DASH != endptr[0]){
		    return -1;
		}
		else {
             *portno = strtoul(&endptr[1],&endptr,10);
             if( 52 < *portno ||'\0' != endptr[0])/*for au5000 :24; for ax7000 : 6*/
			 	return 1;
        }
		return 0;	
	}
	else {
		return -1; /*not Vlan ID. for Example ,enter '.',and so on ,some special char.*/
	}
}

/****************************************************
*FUN:param_first_char_check 
*Params: 
*	IN	str		:String field user entered from vtysh. 
*		cmdtip	:Type of string field::0--add/delete
*									   1--tag/untag
*	OUT NPD_FAIL :Bad Command String Field.
*******************************************/
int param_first_char_check(char* str,unsigned int cmdtip)
{
	int i;
	int ret = NPD_FAIL;
	char c = 0;
	if(NULL == str){
		return ret;
	}
	c = str[0];
	switch (cmdtip){
		case 0:/*add/delete*/
			if('a' == c){ret = 1;}
			else if('d' == c){ret = 0;}
			else {ret = NPD_FAIL;}
			break;
		case 1:/*untag/tag*/
			if(c =='t'){ret = 1;}
			else if('u' == c){ret = 0;}
			else {ret = NPD_FAIL;}
			break;
		default:
			break;
	}
	return ret;
	
}

 


int dcli_vlan_show_running_config(struct vty* vty) 
{	
	int ret = 1;
	char *showStr = NULL,*cursor = NULL,ch = 0,tmpBuf[SHOWRUN_PERLINE_SIZE] = {0};
	DBusMessage *query, *reply;
	DBusError err;
	
	query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,	\
							NPD_DBUS_VLAN_OBJPATH ,	\
							NPD_DBUS_VLAN_INTERFACE ,	\
							NPD_DBUS_VLAN_METHOD_SHOW_RUNNING_CONFIG);

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("show vlan running config failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return ret;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_STRING, &showStr,
					DBUS_TYPE_INVALID)) 
	{
	
		char _tmpstr[64];
		memset(_tmpstr,0,64);
		sprintf(_tmpstr,BUILDING_MOUDLE,"VLAN");
		vtysh_add_show_string(_tmpstr);
		vtysh_add_show_string(showStr);
		ret = 0;
	} 
	else 
	{
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	dbus_message_unref(reply);
	return ret;	
}

/*********************end of dcli_vlan.c********************************************/

#endif /*end of dcli_vlan.c*/




/******moved from dcli_qos.c*********/
int dcli_qos_policer_show_running_config()
{	 
	 char *showStr = NULL,*cursor = NULL,ch = 0,tmpBuf[SHOWRUN_PERLINE_SIZE] = {0};
	 int ret = 1;
	 DBusMessage		*query, *reply;
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
            										 	NPD_DBUS_QOS_OBJPATH, \
            										 	NPD_DBUS_QOS_INTERFACE, \
            										 	NPD_DBUS_METHOD_SHOW_POLICER_RUNNIG_CONFIG);
             
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
            	 if (NULL == reply) {
            		 printf("show qos_policer running config failed get reply.\n");
            		 if (dbus_error_is_set(&err)) {
            			 dbus_error_free_for_dcli(&err);
            		 }
            		 return 1;
            	 }
             
            	 if (dbus_message_get_args ( reply, &err,
            					 DBUS_TYPE_STRING, &showStr,
            					 DBUS_TYPE_INVALID)) 
            	 {
            	 
            			char _tmpstr[64];
            			memset(_tmpstr,0,64);
            			sprintf(_tmpstr,BUILDING_MOUDLE,"QOS POLICER");
            			vtysh_add_show_string(_tmpstr);
            			vtysh_add_show_string(showStr);
            			ret = 0;
            	 } 
            	 else 
            	 {
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




void dcli_npd_init() 
{
	/***********dcli_vlan.c********/
	install_node (&vlan_node, dcli_vlan_show_running_config, "VLAN_NODE");
	install_default(VLAN_NODE);
	
	
}


