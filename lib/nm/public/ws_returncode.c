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
* ws_returncode.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* function for web
*
*
***************************************************************************/
#include "ws_returncode.h"
#include <stdio.h>
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include "npd/nbm/npd_bmapi.h"

unsigned int parse_param_ifName
(
    char * ifName,
    unsigned char * port_no,
    unsigned char * slot_no,
    unsigned int  * vid
)
{
    char * endptr = NULL;
	unsigned int flag = 2;
	unsigned int vid2;
	if(0 == strncmp(ifName,"vlan",4)){
		*vid = (unsigned int)strtoul(ifName+4,&endptr,10);
		if((*vid) != 0){
		    flag = 0;
		}
	}
	else if(0 == strncmp(ifName,"eth",3)){
		if(NPD_SUCCESS == parse_slotport_tag_no(ifName+3,port_no,slot_no,vid,&vid2)){
		    flag = 1;
		}
	}
	return flag;
}
int parse_slotno_localport_include_slot0(char* str,unsigned int *slotno,unsigned int *portno) 
{
	char *endptr = NULL;
	char c = 0;
	if (NULL == str) return -1;
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
int parse_slotport_no(char *str,unsigned char *slotno,unsigned char *portno) 
{
	char *endptr = NULL;
	char *endptr2 = NULL;

	if (NULL == str) return NPD_FAIL;
	*portno = strtoul(str,&endptr,10);
	if (endptr) {
		if ((SLOT_PORT_SPLIT_DASH == endptr[0])||(SLOT_PORT_SPLIT_SLASH == endptr[0])) {
            *slotno = *portno;
			*portno = strtoul((char *)&(endptr[1]),&endptr2,10);
			if('\0' == endptr2[0]) {
				return NPD_SUCCESS;
			}
			else {
				return NPD_FAIL;
			}
		}
		if ('\0' == endptr[0]) {
			*slotno = 0;
			return NPD_SUCCESS;
		}
	}
	return NPD_FAIL;	
}

int parse_slotport_tag_no(char *str,unsigned char *slotno,unsigned char *portno,unsigned int * tag1,unsigned int *tag2)
{
	char *endptr = NULL;
	char *endptr2 = NULL;
	char *endptr3 = NULL;
	char *endptr4 = NULL;
	char * tmpstr = str;
	
	if(NULL == str){
		return NPD_FAIL;
	}
	if((NULL == slotno)||(NULL == portno)||(NULL == tag1)){
		return NPD_FAIL;
	}
	*slotno = 0;
	*portno = 0;
	*tag1 = 0;
	if(NULL == tag2){
		*tag2 = 0;
	}
	if(0 == strncmp(str,"eth",3)){
		tmpstr = str+3;
	}
	if (NULL == tmpstr) {return NPD_FAIL;}
	if(((tmpstr[0] == '0')&&(tmpstr[1] != SLOT_PORT_SPLIT_DASH))|| \
		(tmpstr[0] > '9')||(tmpstr[0] < '0')){
         return NPD_FAIL;
	}
	*portno = (char)strtoul(tmpstr,&endptr,10);
	if (endptr) {
		if ((SLOT_PORT_SPLIT_DASH == endptr[0])&& \
			(('0' < endptr[1])&&('9' >= endptr[1]))){
            *slotno = *portno;
			*portno = (char)strtoul((char *)&(endptr[1]),&endptr2,10);			
			if(endptr2){	
				if('\0' == endptr2[0]){
					*tag1 = 0;
					return NPD_SUCCESS;
				}
				else if(('.' == endptr2[0])&&\
					(('0' < endptr2[1])&&('9' >= endptr2[1]))){
					*tag1 = strtoul((char *)&(endptr2[1]),&endptr3,10);
					if((NULL == endptr3)||('\0' == endptr3[0])){
						if(tag2) *tag2 = 0;
						return NPD_SUCCESS;
					}
					if(!tag2) return NPD_FAIL;
					if((endptr3 != NULL)&&(endptr3[0] == '.')){
						if(('0' >= endptr3[1])||('9' < endptr3[1])){
							return NPD_FAIL;
						}
						if(tag2){
							*tag2 = strtoul((char *)&(endptr3[1]),&endptr4,10);
							if((endptr4 != NULL)&&(endptr4[0] != '\0')){
								return NPD_FAIL;
							}
						}
						return NPD_SUCCESS;
					}
					return NPD_FAIL;
					
				}
				else{
					*tag1 = 0;
					return NPD_FAIL;
				}
			}
			
			*tag1 = 0;
			if(tag2) *tag2 = 0;
			return NPD_SUCCESS;
		}
	}
	*slotno = 0;
	*tag1 = 0;
	if(tag2) *tag2 = 0;
	return NPD_FAIL;	
}
int parse_param_no(char* str,unsigned int* param)
{
	char* endptr = NULL;

	if(NULL == str)return COMMON_ERROR;
	*param = strtoul(str,&endptr,10);

	return COMMON_SUCCESS;
	
}
int parse_intf_name(char *str,char* name)
{

char *endptr = NULL;

	if (NULL == str) return COMMON_ERROR;
	printf("before parsing,the str :: %s\n",str);
	strncpy(name,str,16);
	name[15] = '\0';

	return COMMON_SUCCESS;
}
int parse_ip_address(char *str,unsigned int* addr)
{
	char* endptr = NULL;

	if(NULL == str)return COMMON_ERROR;
	printf("before parsing the str :: %s\n",str);

	*addr = dcli_ip2ulong(str);
	printf("IP %d\n",*addr);
	return COMMON_SUCCESS;
	
}

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

 int parse_mac_addr(char* input,ETHERADDR* macAddr) 
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
		if((cur == ':') ||(cur == '-')){
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
        return COMMON_ERROR;
	}	
	str = *buf;
	length = strlen(str);
	if( length > DCLI_IPMASK_STRING_MAXLEN ||  \
		length < DCLI_IP_STRING_MINLEN ){
		return COMMON_ERROR;
	}
	if((str[0] > '9')||(str[0] < '1')){
		return COMMON_ERROR;
	}
	for(i = 0; i < length; i++){
		if('/' == str[i]){
            splitCount++;
			if((i == length - 1)||('0' > str[i+1])||(str[i+1] > '9')){
                return COMMON_ERROR;
			}
		}
		if((str[i] > '9'||str[i]<'0') &&  \
			str[i] != '.' &&  \
			str[i] != '/' &&  \
			str[i] != '\0'
		){
			return COMMON_ERROR;
		}
	}
	if(1 != splitCount){
        return COMMON_ERROR;
	}
	
	strcp=(char *)malloc(50*sizeof(char));
	if (NULL == strcp) {
		return COMMON_ERROR;
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
			return COMMON_ERROR;
		}
		if((ipMask<0)||(ipMask>32))
		{
			free(strcp);
			strcp = NULL;
			return COMMON_ERROR;	
		}
		else
		{
			*mask = ipMask;
		}
	}
	
	if(COMMON_SUCCESS != parse_ip_check(token2)){
		free(strcp);
		strcp = NULL;
		return COMMON_ERROR;
	}
    *ipAddress = dcli_ip2ulong(token2);	
	free(strcp);
	strcp = NULL;
	return COMMON_SUCCESS;
}
int parse_short_parse(char* str,unsigned short* shot){
	char *endptr = NULL;

	if (NULL == str) return NPD_FAIL;
	*shot= strtoul(str,&endptr,10);
	return NPD_SUCCESS;	
}
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
			return COMMON_ERROR;
		}
		if((str[0] > '9')||(str[0] < '1')){
			return COMMON_ERROR;
		}
		for(i=0;i<strlen(str);i++){
			ipAddr[i]=str[i];
			if('.' == str[i]){
                pointCount++;
				if((i == strlen(str)-1)||('0' > str[i+1])||(str[i+1] > '9')){
					return COMMON_ERROR;
				}
			}
			if((str[i]>'9'||str[i]<'0')&&str[i]!='.'&&str[i]!='\0'){
				return COMMON_ERROR;
			}
		}
		if(3 != pointCount){
            return COMMON_ERROR;
		}
		token=strtok(ipAddr,sep);
		if((NULL == token)||("" == token)||(strlen(token) < 1)||\
			((strlen(token) > 1) && ('0' == token[0]))){
			return COMMON_ERROR;
		}
		if(NULL != token){
		    ip_long[0] = strtoul(token,NULL,10);
		}
		else {
			return COMMON_ERROR;
		}
		i=1;
		while((token!=NULL)&&(i<4))
		{
			token=strtok(NULL,sep);
			if((NULL == token)||("" == token)||(strlen(token) < 1)|| \
				((strlen(token) > 1) && ('0' == token[0]))){
				return COMMON_ERROR;
			}
			if(NULL != token){
			    ip_long[i] = strtoul(token,NULL,10);
			}
			else {
				return COMMON_ERROR;
			}
			i++;
		}
		for(i=0;i<4;i++){
            if(ip_long[i]>255){
                return COMMON_ERROR;
			}
		}
		return COMMON_SUCCESS;
		
}

/////modify v1.116 新增加的函数
int ccgi_eth_port_interface_mode_config
( 
    unsigned char slot_no,
    unsigned char port_no   
)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int op_ret = NPD_DBUS_ERROR;
	unsigned int tmpIfIndex = ~0UI;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,		\
								NPD_DBUS_ETHPORTS_OBJPATH,		\
								NPD_DBUS_ETHPORTS_INTERFACE,		\
								NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_ETHPORT_INTERFACE);
	dbus_error_init(&err);

	dbus_message_append_args(query,
								DBUS_TYPE_BYTE,&slot_no,
								DBUS_TYPE_BYTE,&port_no,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return COMMON_FAIL;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_UINT32,&tmpIfIndex,
		DBUS_TYPE_INVALID)) {
			if(NPD_DBUS_SUCCESS == op_ret){
	            dbus_message_unref(reply);
				//sleep(1);
				return COMMON_SUCCESS;
			}
			else if(NPD_DBUS_ERROR_NO_SUCH_PORT == op_ret){
				//vty_out(vty,"%% NO SUCH PORT %d/%d!\n",slot_no,port_no);
			}
			else if((DCLI_ETH_PORT_ALREADY_IN_L3_VLAN == op_ret) || \
				(DCLI_DEFAULT_VLAN_IS_L3_VLAN == op_ret)){
				//vty_out(vty,"%% Port mode change fail,conflict with vlan interface\n");
			}
			else if(NPD_DBUS_ERROR_UNSUPPORT == op_ret)
				//vty_out(vty,"%% Unsupport this command\n");
				return COMMON_FAIL;
			else if(DCLI_INTF_CHECK_MAC_ERR == op_ret)
				//vty_out(vty, "%% Check interface's mac address FAILED!\n");
				return COMMON_FAIL;
			else
				//vty_out(vty,"%% Execute command failed\n");
				return COMMON_FAIL;
			
	}
	else {
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return COMMON_FAIL;

}

int dcli_str2ulong(char *str,unsigned int *Value)
{
	char *endptr = NULL;	
	char c = 0;	int ret = 0;	
	if (NULL == str) 
		return ACL_RETURN_CODE_ERROR;	
	ret = dcli_checkPoint(str);	
	if(ret == 1)
		{		
			return ACL_RETURN_CODE_ERROR;	
		}	
	c = str[0];	
	if((strlen(str) > 1)&&('0' == c))
		{	
			/* string(not single "0") should not start with '0'*/	
			return ACL_RETURN_CODE_ERROR;	
		}		
	*Value= strtoul(str,&endptr,10);
	if('\0' != endptr[0])
		{	
			return ACL_RETURN_CODE_ERROR;	
		}	
	return ACL_RETURN_CODE_SUCCESS;	
}

int dcli_checkPoint(char *ptr)
{
	int ret = 0;
	while(*ptr != '\0')
		{
			if(((*ptr) < '0')||((*ptr) > '9'))
				{
					ret = 1;
					break;
				}
			*ptr++;
		}
	return ret;
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
	 unsigned long	 cnt;
	 unsigned char *tmpPtr = *buff;
 
	 cnt = sprintf((char*)tmpPtr,"%ld.%ld.%ld.%ld",(ipAddress>>24) & 0xFF, \
			 (ipAddress>>16) & 0xFF,(ipAddress>>8) & 0xFF,ipAddress & 0xFF);
	 
	 return cnt;
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
unsigned long ccgi_ip2ulong(char *str)
{
	char *sep=".";
	char *token = NULL;
	unsigned long ip_long[4]; 
	unsigned long ip = 0;
	int i = 0;
	char *ipstr=(char *)malloc(50);
	memset(ipstr,0,50);
	strcpy(ipstr,str);
	
	token=strtok(ipstr,".");
	i=0;
	while(token!=NULL)
	{		
		ip_long[i] = strtoul(token,NULL,10);
		token = strtok(NULL,".");	
		i++;
		if(i==4)
			break;
	}

	ip=(ip_long[0]<<24)+(ip_long[1]<<16)+(ip_long[2]<<8)+ip_long[3];
	free(ipstr);
	return ip;
}
/*删除字符串结尾的换行*/
void delete_line_blank(char * string)
{
	int len = 0;
	len = strlen(string);
    int len_l = 0;
	if(string == NULL)
		return;
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

