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
* ws_dcli_acl.c
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
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <dbus/dbus.h>
#include "ws_dcli_acl.h"
/*#include "cgic.h"*/
#include <unistd.h>
#include "ws_returncode.h"

///////////////////////////////////////////////////////////////////
/* dcli_acl.c  version :v1.59  wangpeng 2008-12-02*/
//////////////////////////////////////////////////////////////////

#if 0
struct cmd_node time_range_node =
{
	TIME_RANGE_NODE,
	"%s(config-time-range)# "
};

#endif








int INDEX_LENTH_CHECK_ACL(char *str,unsigned int num)
{
	char c=0;
	
	c = str[0];
	if(!(c<='9'&&c>'0')){
		return -3;
	}
	else{
		if(num<(strlen(str)))
		{
		  return -3;
		}
	}
	return 1;
}



#define VALUE_IP_MASK_CHECK(num) 					\
if((num<1)||(num>32)) 		\
 {													\
		return CMD_FAILURE;							\
 }




 /**********************************************************************************
  *  dcli_str2ulong
  *
  *  DESCRIPTION:
  * 	 convert string to long interger
  *
  *  INPUT:
  * 	 str - string
  *  
  *  OUTPUT:
  * 	 null
  *
  *  RETURN:
  * 	 
  * 	 val - long value
  * 	 
  **********************************************************************************/
/*
 unsigned long dcli_str2ulong(char *str)
 {
	 unsigned long val = 0;
	 char * endChr;
	 int base = 10;
	 
	 if(NULL == str)
		 return 0;
	 else
		 val = strtoul(str, &endChr, base);
 
	 return val;
 }
 */
 int dcli_str2ulongWS(char *str,unsigned int *Value)
 {
	 char *endptr = NULL;
	 char c;
	 int ret = 0;
	 if (NULL == str) return NPD_FAIL;
 	
	 ret = dcli_checkPoint(str);
	 if(ret == 1)
	 {
		 return NPD_FAIL;
	 }

	  c = str[0];
	  if((strlen(str) > 1)&&('0' == c)){
		// string(not single "0") should not start with '0'
		return NPD_FAIL;
	}
	
	 *Value= strtoul(str,&endptr,10);
	
	 if('\0' != endptr[0]){
		 return NPD_FAIL;
	 }
	 return NPD_SUCCESS; 
 }

 /**********************************************************************************
  *  dcli_ip2ulong
  *
  *  DESCRIPTION:
  * 	 convert IP (A.B.C.D) to IP (ABCD) pattern
  *
  *  INPUT:
  * 	 str - (A.B.C.D)
  *  
  *  OUTPUT:
  * 	 null
  *
  *  RETURN:
  * 	 
  * 	 IP  -	ip (ABCD)
  * 	 
  **********************************************************************************/
 
 unsigned long dcli_ip2ulong(char *str)
 {
	 char *sep=".";
	 char *token;
	 unsigned long ip_long[4]; 
	 unsigned long ip;
	 int i = 1;
	 
	 token=strtok(str,sep);
	 ip_long[0] = strtoul(token,NULL,10);
	 while((token!=NULL)&&(i<4))
	 {
		 token=strtok(NULL,sep);
		 ip_long[i] = strtoul(token,NULL,10);
		 i++;
	 }
 
	 ip=(ip_long[0]<<24)+(ip_long[1]<<16)+(ip_long[2]<<8)+ip_long[3];
 
	 return ip;
 }
 

 
  /****************************************************************
  *FUN:timeRange_name_legal_check
  *Params :
  *  
  *
  *   OUT ALIAS_NAME_LEN_ERROR--time name too long
  * 	  ALIAS_NAME_HEAD_ERROR--illegal char on head of time name 
  * 	  ALIAS_NAME_BODY_ERROR--unsupported char in time name
  ****************************************************************/
  int timeRange_name_legal_check(char* str,unsigned int len)
  {
	  int i;
	  int ret = NPD_FAIL;
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
		  ret =NPD_SUCCESS;
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
	  
	  int	len=0,ret = NPD_FAIL;
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
							  return NPD_SUCCESS;
						  }
						 else{
							 return NPD_FAIL;
						 }
					 }
					 
				 }
				 
			 }
			 
		 }
		 
	  }
	  return NPD_FAIL;
  }
  
 int timeRange_time_deal(char *str,unsigned int *hour,unsigned int *minute)
 {
 
	 int   len=0,ret = NPD_FAIL;
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
				 return NPD_SUCCESS;
			 }
			else{
				 return NPD_FAIL;
			}
		 }
	 }
	 return NPD_FAIL;
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
 
 ///////////////////////////////////////////////////////////////////////////////////////////////////
int get_one_port_index(char * slotport,unsigned int* port_index)
{
	DBusMessage *query, *reply;
	DBusError err;
	int op_ret = 0,ret=0;
	unsigned int eth_g_index = 0;
	unsigned char slot_no = 0,port_no=0;
	
	ret = parse_slotport_no((char *)slotport,&slot_no,&port_no);
	if(ret==NPD_FAIL)
		{
			return 5;
		}
	dbus_error_init(&err);
	query = dbus_message_new_method_call(				\
											NPD_DBUS_BUSNAME, 	\
											NPD_DBUS_ETHPORTS_OBJPATH,	\
											NPD_DBUS_ETHPORTS_INTERFACE, \
											STP_DBUS_METHOD_GET_PORT_INDEX
											);
	
	
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

		return CMD_FAILURE;
	}
	
	dbus_message_get_args ( reply, &err,
								DBUS_TYPE_UINT32,&op_ret,
								DBUS_TYPE_UINT32,&eth_g_index,
								DBUS_TYPE_INVALID);
	*port_index=eth_g_index;
	dbus_message_unref(reply);
	return 0;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int show_acl_allinfo(struct acl_info  acl_all[],int *aNum)
{
	 DBusMessage *query = NULL, *reply = NULL;
	 DBusError err = { 0 };
	 DBusMessageIter  iter;
	 DBusMessageIter  iter_array;
	 char actype[50],protype[10];
	 unsigned int nextheader= 0,step = 0;
	 unsigned int ruleIndex=0,startIndex = 0,endIndex = 0; 
	 unsigned int ruleType=0;
	 unsigned long dip=0;
	 unsigned long sip=0;
	 unsigned long  maskdip=0;
	 unsigned long  masksip=0;
	 unsigned long srcport=0;
	 unsigned long dstport=0;
	 unsigned char icmp_code=0;
	 unsigned char icmp_type=0;
	 unsigned char code_mask=0;
	 unsigned char type_mask=0;
	 unsigned long packetType=0;
	 unsigned long actionType=0; 
	 unsigned char dmac[6]={0},smac[6]={0};
	 unsigned int vlanid=0;
	 unsigned char sourceslot=0,sourceport=0,redirectslot=0,redirectport=0,mirrorslot=0,mirrorport=0;
	 unsigned int j=0,ret,acl_count=0;
	 unsigned char sipBuf[MAX_IP_STRLEN] = {0};
	 unsigned char dipBuf[MAX_IP_STRLEN] = {0};  
	  unsigned char *sipPtr = sipBuf,*dipPtr = dipBuf;
	  unsigned int policer;
	  unsigned int  policerId;  
	  unsigned int  modifyUP;
	  unsigned int  modifyDSCP;
	  unsigned int  up;
	  unsigned int dscp;
	  unsigned int egrUP;
	  unsigned int egrDSCP;
	  unsigned int qosprofileindex;
	  char * upmm=(char *)malloc(10);
	  char * dscpmm=(char *)malloc(10);
	  memset(upmm,0,10);
	  memset(dscpmm,0,10);
	  
	  unsigned int precedence=0;
	  unsigned char appendIndex=0;
	 
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_ACL_OBJPATH,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_METHOD_SHOW_ACL);
	 dbus_error_init(&err);
	 reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	 dbus_message_unref(query);
	 if (NULL == reply) 
	 {
		 if (dbus_error_is_set(&err)) 
		 {
			 dbus_error_free(&err);
		 }
		 return CMD_SUCCESS;
	 }
	 dbus_message_iter_init(reply,&iter);
 
	 dbus_message_iter_get_basic(&iter,&ret);
	 	 if(0 == ret)
	 	 {
    		 dbus_message_iter_next(&iter);  
    		 dbus_message_iter_get_basic(&iter,&acl_count);
    		 dbus_message_iter_next(&iter); 
    		 dbus_message_iter_recurse(&iter,&iter_array);
				
    		 *aNum=acl_count;
			 for (j = 0; j < acl_count; j++) {
						  DBusMessageIter iter_struct;
						  dbus_message_iter_recurse(&iter_array,&iter_struct);			  
						  dbus_message_iter_get_basic(&iter_struct,&ruleIndex);
							dbus_message_iter_next(&iter_struct); 
							dbus_message_iter_get_basic(&iter_struct,&startIndex);
							dbus_message_iter_next(&iter_struct); 
							dbus_message_iter_get_basic(&iter_struct,&endIndex);
							dbus_message_iter_next(&iter_struct); 
							dbus_message_iter_get_basic(&iter_struct,&step);
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
							dbus_message_iter_get_basic(&iter_struct,&acl_all[j].dipv6[0]);
							dbus_message_iter_next(&iter_struct);
							dbus_message_iter_get_basic(&iter_struct,&acl_all[j].dipv6[1]);
							dbus_message_iter_next(&iter_struct);
							dbus_message_iter_get_basic(&iter_struct,&acl_all[j].dipv6[2]);
							dbus_message_iter_next(&iter_struct);
							dbus_message_iter_get_basic(&iter_struct,&acl_all[j].dipv6[3]);
							dbus_message_iter_next(&iter_struct);
							dbus_message_iter_get_basic(&iter_struct,&acl_all[j].dipv6[4]);
							dbus_message_iter_next(&iter_struct);
							dbus_message_iter_get_basic(&iter_struct,&acl_all[j].dipv6[5]);
							dbus_message_iter_next(&iter_struct);
							dbus_message_iter_get_basic(&iter_struct,&acl_all[j].dipv6[6]);
							dbus_message_iter_next(&iter_struct);
							dbus_message_iter_get_basic(&iter_struct,&acl_all[j].dipv6[7]);
							dbus_message_iter_next(&iter_struct);
							dbus_message_iter_get_basic(&iter_struct,&acl_all[j].dipv6[8]);
							dbus_message_iter_next(&iter_struct);
							dbus_message_iter_get_basic(&iter_struct,&acl_all[j].dipv6[9]);
							dbus_message_iter_next(&iter_struct);
							dbus_message_iter_get_basic(&iter_struct,&acl_all[j].dipv6[10]);
							dbus_message_iter_next(&iter_struct);
							dbus_message_iter_get_basic(&iter_struct,&acl_all[j].dipv6[11]);
							dbus_message_iter_next(&iter_struct);
							dbus_message_iter_get_basic(&iter_struct,&acl_all[j].dipv6[12]);
							dbus_message_iter_next(&iter_struct);
							dbus_message_iter_get_basic(&iter_struct,&acl_all[j].dipv6[13]);
							dbus_message_iter_next(&iter_struct);
							dbus_message_iter_get_basic(&iter_struct,&acl_all[j].dipv6[14]);
							dbus_message_iter_next(&iter_struct);
							dbus_message_iter_get_basic(&iter_struct,&acl_all[j].dipv6[15]);
							dbus_message_iter_next(&iter_struct);
							/*recv sorce ipv6 address*/
							dbus_message_iter_get_basic(&iter_struct,&acl_all[j].sipv6[0]);
							dbus_message_iter_next(&iter_struct);
							dbus_message_iter_get_basic(&iter_struct,&acl_all[j].sipv6[1]);
							dbus_message_iter_next(&iter_struct);
							dbus_message_iter_get_basic(&iter_struct,&acl_all[j].sipv6[2]);
							dbus_message_iter_next(&iter_struct);
							dbus_message_iter_get_basic(&iter_struct,&acl_all[j].sipv6[3]);
							dbus_message_iter_next(&iter_struct);
							dbus_message_iter_get_basic(&iter_struct,&acl_all[j].sipv6[4]);
							dbus_message_iter_next(&iter_struct);
							dbus_message_iter_get_basic(&iter_struct,&acl_all[j].sipv6[5]);
							dbus_message_iter_next(&iter_struct);
							dbus_message_iter_get_basic(&iter_struct,&acl_all[j].sipv6[6]);
							dbus_message_iter_next(&iter_struct);
							dbus_message_iter_get_basic(&iter_struct,&acl_all[j].sipv6[7]);
							dbus_message_iter_next(&iter_struct);
							dbus_message_iter_get_basic(&iter_struct,&acl_all[j].sipv6[8]);
							dbus_message_iter_next(&iter_struct);
							dbus_message_iter_get_basic(&iter_struct,&acl_all[j].sipv6[9]);
							dbus_message_iter_next(&iter_struct);
							dbus_message_iter_get_basic(&iter_struct,&acl_all[j].sipv6[10]);
							dbus_message_iter_next(&iter_struct);
							dbus_message_iter_get_basic(&iter_struct,&acl_all[j].sipv6[11]);
							dbus_message_iter_next(&iter_struct);
							dbus_message_iter_get_basic(&iter_struct,&acl_all[j].sipv6[12]);
							dbus_message_iter_next(&iter_struct);
							dbus_message_iter_get_basic(&iter_struct,&acl_all[j].sipv6[13]);
							dbus_message_iter_next(&iter_struct);
							dbus_message_iter_get_basic(&iter_struct,&acl_all[j].sipv6[14]);
							dbus_message_iter_next(&iter_struct);
							dbus_message_iter_get_basic(&iter_struct,&acl_all[j].sipv6[15]);
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
						  case 3:strcpy(actype,"Permit");	  break;	/*原来是mirror的，先单独做了个mirror模块*/					  
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
						  case 3:strcpy(protype,"ICMP");	  break;  
						  case 4:strcpy(protype,"Ethernet");  break;
						  case 5:strcpy(protype,"ARP"); 	  break; 
						  case 6:strcpy(protype,"All"); 	  break; 
						  case 7:strcpy(protype,"IPv6"); 	  break; 
						  default:
									break;									  
					  }   
					  ip_long2str(dip,&dipPtr);
					  ip_long2str(sip,&sipPtr);
					  
					  acl_all[j].ruleIndex=ruleIndex;
					  if(ruleType ==STANDARD_ACL_RULE){
					  	 	strcpy(acl_all[j].ruleType,"standard");
						 if((packetType==0 )||(packetType==1)||(packetType==2)||(packetType==3)){
						 		strcpy(acl_all[j].actype,actype);
						 		strcpy(acl_all[j].protype,protype);
						 		sprintf(acl_all[j].dip,"%s/%ld",dipPtr,maskdip);
						 		sprintf(acl_all[j].sip,"%s/%ld",sipPtr,masksip);
							 if((strcmp(protype,"TCP")==0)||(strcmp(protype,"UDP")==0))
							 {
							 	if(ACL_ANY_PORT == dstport)
							 	{
							 		acl_all[j].dstport=ACL_ANY_PORT;
							 	}
								else
								{
    							 		acl_all[j].dstport=dstport;
								}

								if(ACL_ANY_PORT == srcport)
								{
									acl_all[j].srcport=ACL_ANY_PORT;
								}
								else
								{
    							 		acl_all[j].srcport=srcport;
								}
							 }
							 if(strcmp(protype,"ICMP")==0)
							 {
							 	if(0 == code_mask)
							 	{
							 		acl_all[j].icmp_code=0;
							 	}
								else
								{
									acl_all[j].icmp_code=icmp_code;
								}

								if(0 == type_mask)
								{
							 		acl_all[j].icmp_type=0;
								}
								else
								{
									acl_all[j].icmp_type=icmp_type;
								}
							 }
							 if (strcmp(actype,"Redirect")==0)
							 		sprintf(acl_all[j].redirect_port,"%d/%d",redirectslot,redirectport);
							 
							
							 if(1==policer){
								acl_all[j].policerId=policerId;
								// vty_out(vty,"%-40s:%s\n","policer","enable");
							 }

							 
						 }
								 
						 else if((packetType==4)||(packetType==5)){
						 		strcpy(acl_all[j].actype,actype);
						 		strcpy(acl_all[j].protype,protype);
						 		sprintf(acl_all[j].smac,"%02x:%02x:%02x:%02x:%02x:%02x",smac[0],smac[1],smac[2],smac[3],smac[4],smac[5]);

							  if(strcmp(protype,"Ethernet")==0){
								  sprintf(acl_all[j].dmac,"%02x:%02x:%02x:%02x:%02x:%02x",dmac[0],dmac[1],dmac[2],dmac[3],dmac[4],dmac[5]);
							  }
						 
							 if(strcmp(protype,"ARP")==0){
							 		strcpy(acl_all[j].dmac,"FF:FF:FF:FF:FF:FF");
							 		acl_all[j].vlanid=vlanid;
							 		sprintf(acl_all[j].source_port,"%d/%d",sourceslot,sourceport);
							 }
			 
							  if (strcmp(actype,"Redirect")==0)
							  		sprintf(acl_all[j].redirect_port,"%d/%d",redirectslot,redirectport);
			 
							  if (strcmp(actype,"MirrorToAnalyzer")==0)
							  		sprintf(acl_all[j].analyzer_port,"%d/%d",mirrorslot,mirrorport);
							 
							  if(1==policer){
								 acl_all[j].policerId=policerId;
							  }
							 
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
											 
							strcpy(acl_all[j].actype,actype);
							 if(strcmp(actype,"Ingress QoS Mark")==0){
							 	acl_all[j].up=up;
							 	acl_all[j].dscp=dscp;
								 acl_all[j].qosprofileindex=qosprofileindex+1;
								 //vty_out(vty,"%-40s: %d\n","Mark QoS profile Table",1+qosprofileindex);
			 
							 }
							 if(strcmp(actype,"Egress QoS Remark")==0){ 										 
								acl_all[j].up=up;
							 	acl_all[j].dscp=dscp;
							 	acl_all[j].egrUP=egrUP;
							 	acl_all[j].egrDSCP=egrDSCP;
							 }
							 strcpy(acl_all[j].upmm,upmm);
							 strcpy(acl_all[j].dscpmm,dscpmm);
							 
							 if(1==policer){
								 acl_all[j].policerId=policerId;;
							  }
							 
						 }
					  }
					  else if(ruleType==EXTENDED_ACL_RULE){
					  		strcpy(acl_all[j].ruleType,"extended");
				  			strcpy(acl_all[j].actype,actype);
					 		strcpy(acl_all[j].protype,protype);
					 		sprintf(acl_all[j].dip,"%s/%ld",dipPtr,maskdip);
					 		acl_all[j].dstport=dstport;
					 		sprintf(acl_all[j].sip,"%s/%ld",sipPtr,masksip);
					 		acl_all[j].srcport=srcport;
					 		sprintf(acl_all[j].dmac,"%02x:%02x:%02x:%02x:%02x:%02x",dmac[0],dmac[1],dmac[2],dmac[3],dmac[4],dmac[5]);
						 	sprintf(acl_all[j].smac,"%02x:%02x:%02x:%02x:%02x:%02x",smac[0],smac[1],smac[2],smac[3],smac[4],smac[5]);
						 	acl_all[j].vlanid=vlanid;
						 	sprintf(acl_all[j].source_port,"%d/%d",sourceslot,sourceport);
			 
						  if (strcmp(actype,"Redirect")==0)
							  	sprintf(acl_all[j].redirect_port,"%d/%d",redirectslot,redirectport);
						 
						  if (strcmp(actype,"MirrorToAnalyzer")==0)
							 	sprintf(acl_all[j].analyzer_port,"%d/%d",mirrorslot,mirrorport);
						 
						  if(1==policer){
							 acl_all[j].policerId=policerId;;
						   }
						  
						 }
					  
					  }  
		 }
		
	dbus_message_unref(reply);
	free(upmm);
	free(dscpmm);
	 return CMD_SUCCESS;
}


int show_group_ByRuleIndex(unsigned int ruleindex,unsigned int dir,unsigned int * returnGpIndex,unsigned int * returnGpType)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = { 0 };
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	unsigned int     k,j,ret,group_count=0,dir_info=0;
	unsigned int     group_num=0,count=0,index=0,portnum = 0,vid_count = 0,slot_no=0,port_no=0, vidindex=0;

	dir_info=dir;
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_ACL_OBJPATH,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_METHOD_SHOW_ACL_GROUP);
	dbus_error_init(&err);
	dbus_message_append_args(query, 
						     DBUS_TYPE_UINT32,&dir_info,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	
	if (NULL == reply) 
	 {
		 if (dbus_error_is_set(&err)) 
		 {
		 	dbus_error_free(&err);
		 }
		 return 0;
	 }
	 dbus_message_iter_init(reply,&iter);
 
	 dbus_message_iter_get_basic(&iter,&ret);
	 
	 if(NPD_DBUS_ERROR == ret) {
	 		return 0;
	 }
 
	 else if(NPD_DBUS_SUCCESS == ret){

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
			  dbus_message_iter_next(&iter_struct); 		  
			  dbus_message_iter_recurse(&iter_struct,&iter_sub_array);

			  for (k = 0; k < portnum; k++){
				  DBusMessageIter iter_sub_struct;
				  dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
							  
				  dbus_message_iter_get_basic(&iter_sub_struct,&slot_no);
				  dbus_message_iter_next(&iter_sub_struct);

				  dbus_message_iter_get_basic(&iter_sub_struct,&port_no);
			         dbus_message_iter_next(&iter_sub_struct);
			         

			         dbus_message_iter_next(&iter_sub_array);
			  }	

			  dbus_message_iter_next(&iter_struct); 	

			  dbus_message_iter_recurse(&iter_struct,&iter_sub_array);

			  for (k = 0; k < vid_count; k++){
				  DBusMessageIter iter_sub_struct;
				  dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
							  
				  dbus_message_iter_get_basic(&iter_sub_struct,&vidindex);
			         dbus_message_iter_next(&iter_sub_struct);

			         dbus_message_iter_next(&iter_sub_array);
			  }
			  
			  dbus_message_iter_next(&iter_struct); 		  
			  dbus_message_iter_recurse(&iter_struct,&iter_sub_array);
			  for (k = 0; k < count; k++){
				  DBusMessageIter iter_sub_struct;
				  dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
							  
				  dbus_message_iter_get_basic(&iter_sub_struct,&index);
				  dbus_message_iter_next(&iter_sub_struct);
			  	if(index==ruleindex)
			  	{
			  		*returnGpIndex=group_num;
			  		*returnGpType=dir;
			  	}
			     dbus_message_iter_next(&iter_sub_array);
			  }		


		 dbus_message_iter_next(&iter_array);

 	     }
	 }  
	return 1;
	
}


int show_group_list(unsigned int dir,struct group_info grpInfo[],unsigned int * groupNum,unsigned int baseNum)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = { 0 };
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	unsigned int     k,ret,group_count=0,dir_info=0;
	unsigned int     group_num=0,count=0,slot_no=0,port_no=0,portnum = 0, vid_count = 0, index = 0, vidindex=0;
	int j;
	dir_info=dir;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_ACL_OBJPATH,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_METHOD_SHOW_ACL_GROUP);
	dbus_error_init(&err);
	dbus_message_append_args(query, 
						     DBUS_TYPE_UINT32,&dir_info,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	
	if (NULL == reply) 
	 {
		 if (dbus_error_is_set(&err)) 
		 {
		 	dbus_error_free(&err);
		 }
		 return 0;
	 }
	 dbus_message_iter_init(reply,&iter);
 
	 dbus_message_iter_get_basic(&iter,&ret);
	 if(NPD_DBUS_ERROR == ret) {
	 		return 0;
	 }
 	
	 else if(NPD_DBUS_SUCCESS == ret){

		 dbus_message_iter_next(&iter);  
		 dbus_message_iter_get_basic(&iter,&group_count); 

		 dbus_message_iter_next(&iter);  
 	     dbus_message_iter_recurse(&iter,&iter_array);
 			*groupNum=group_count;	

 	     for (j = 0; j < group_count; j++) {

			  DBusMessageIter iter_struct;
			  DBusMessageIter iter_sub_array;
			  
	 		  dbus_message_iter_recurse(&iter_array,&iter_struct);			  

			  dbus_message_iter_get_basic(&iter_struct,&group_num);
				grpInfo[j+baseNum].groupIndex=group_num;
				grpInfo[j+baseNum].groupType=dir;
			  dbus_message_iter_next(&iter_struct); 
	 		  dbus_message_iter_get_basic(&iter_struct,&count);
			  dbus_message_iter_next(&iter_struct); 
	 		  dbus_message_iter_get_basic(&iter_struct,&portnum);
			  dbus_message_iter_next(&iter_struct); 
	 		  dbus_message_iter_get_basic(&iter_struct,&vid_count);
			 
			  dbus_message_iter_next(&iter_struct); 		  
			  dbus_message_iter_recurse(&iter_struct,&iter_sub_array);
				grpInfo[j+baseNum].ruleNumber=count;
			  for (k = 0; k < portnum; k++){
				  DBusMessageIter iter_sub_struct;
				  dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
							  
				  dbus_message_iter_get_basic(&iter_sub_struct,&slot_no);
				  dbus_message_iter_next(&iter_sub_struct);

				  dbus_message_iter_get_basic(&iter_sub_struct,&port_no);
			         dbus_message_iter_next(&iter_sub_struct);

			         dbus_message_iter_next(&iter_sub_array);
			  }	

			  dbus_message_iter_next(&iter_struct); 	

			  dbus_message_iter_recurse(&iter_struct,&iter_sub_array);
			  for (k = 0; k < vid_count; k++){
				  DBusMessageIter iter_sub_struct;
				  dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
							  
				  dbus_message_iter_get_basic(&iter_sub_struct,&vidindex);
			         dbus_message_iter_next(&iter_sub_struct);

			         dbus_message_iter_next(&iter_sub_array);
			  }
			  dbus_message_iter_next(&iter_struct); 		  
			  dbus_message_iter_recurse(&iter_struct,&iter_sub_array);
			 for (k = 0; k < count; k++){
				  DBusMessageIter iter_sub_struct;
				  dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
							  
				  dbus_message_iter_get_basic(&iter_sub_struct,&index);
				dbus_message_iter_next(&iter_sub_struct);
			  	grpInfo[j+baseNum].ruleindex[k]=index;

			     dbus_message_iter_next(&iter_sub_array);
			  }
			dbus_message_iter_next(&iter_array);

 	     }
	 }  

	dbus_message_unref(reply);
	return 5;
	
}


int addacl_group(unsigned int dir,unsigned int groupIndex)
{
	struct list *lcontrol; 	/*解析help.txt文件的链表头*/
    lcontrol=get_chain_head("../htdocs/text/control.txt"); 
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int groupNum,op_ret;	
	unsigned int dir_info=0;



	dir_info=dir;
	
	groupNum=groupIndex;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_ACL_OBJPATH,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_METHOD_CREATE_ACL_GROUP);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,		
						     DBUS_TYPE_UINT32,&dir_info,
							 DBUS_TYPE_UINT32,&groupNum,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return CMD_SUCCESS;
	}


	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)) {
			if (NPD_DBUS_SUCCESS == op_ret ) {
				ShowAlert(search(lcontrol,"add_acl_group_suc"));
			}
			else if(NPD_DBUS_ERROR == op_ret ){
				if(dir_info==0)
					ShowAlert(search(lcontrol,"ingress_group_exist"));
				else if(dir_info==1)
					ShowAlert(search(lcontrol,"egress_group_exist"));
			}
			else if(ACL_GROUP_SAME_ID == op_ret){
				if(dir_info==0)
					ShowAlert(search(lcontrol,"ingress_group_exist"));
				else if(dir_info==1)
					ShowAlert(search(lcontrol,"egress_group_exist"));
			}
	}			
	else {

		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	release(lcontrol);
	return CMD_SUCCESS;
}

int bind_acl_group(unsigned int dir,unsigned int nodeflag,unsigned int index,unsigned int groupNum)
{
	struct list *lcontrol; 	/*解析help.txt文件的链表头*/
    lcontrol=get_chain_head("../htdocs/text/control.txt"); 

	DBusMessage *query, *reply;
	DBusError err;

	unsigned int g_index,sw_info=0,cfg_info=0;
	unsigned int group_num;
	unsigned int IsEnable=0;  
	unsigned int node_flag,dir_info=0;

	dir_info=dir;
	node_flag=nodeflag;
	g_index=index;
	group_num=groupNum;
		
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_ETHPORTS_OBJPATH,NPD_DBUS_ETHPORTS_INTERFACE,NPD_DBUS_ETHPORTS_METHOD_BIND_ACL_GROUP);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&dir_info,
							 DBUS_TYPE_UINT32,&node_flag,
							 DBUS_TYPE_UINT32,&g_index,
							 DBUS_TYPE_UINT32,&group_num,						 
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return CMD_SUCCESS;
	}
	
	
	if (dbus_message_get_args ( reply, &err,			
			DBUS_TYPE_UINT32, &sw_info,
		    DBUS_TYPE_UINT32, &cfg_info,
		    DBUS_TYPE_UINT32, &IsEnable,
			DBUS_TYPE_INVALID)) {
			  if(node_flag==0){
			  		
			  	 	if(NPD_DBUS_ERROR==IsEnable)
					{
		
					}

					else if (NPD_DBUS_ERROR_NO_SUCH_PORT == sw_info) {
						ShowAlert(search(lcontrol,"eth_Not_exist"));
					}				
											
					else if(ACL_GROUP_NOT_EXISTED == sw_info){
						ShowAlert(search(lcontrol,"group_Not_exist"));
					}
					else if(NPD_DBUS_ERROR == sw_info){
						ShowAlert(search(lcontrol,"bind_fail"));
					}
						
					else if (ACL_GROUP_PORT_BINDED ==sw_info) {
						ShowAlert(search(lcontrol,"Eth_has_bind"));
					}
					else if (NPD_DBUS_SUCCESS==cfg_info){
						ShowAlert(search(lcontrol,"bind_suc"));
						//return NPD_DBUS_SUCCESS;
					}
										
					else if(NPD_DBUS_ACL_ERR_GENERAL==cfg_info){
					}
					
					
			  	}
			  if(node_flag==1){
					if(NPD_DBUS_ERROR==IsEnable)
					{
					}
					else if (NPD_DBUS_ERROR_NO_SUCH_VLAN +1  == sw_info) 
					{
					}
					else if(NPD_VLAN_NOTEXISTS==sw_info)
					{
							ShowAlert(search(lcontrol,"VLAN_NOT_EXITSTS"));
					}
				   	else if(ACL_GROUP_NOT_EXISTED == sw_info){
					}
					else if(NPD_DBUS_ERROR == sw_info){
						ShowAlert(search(lcontrol,"bind_fail"));
					}
						
					else if (ACL_GROUP_VLAN_BINDED ==sw_info) 
					{
						ShowAlert(search(lcontrol,"vlan_has_bind"));
					}
					else if ((NPD_DBUS_SUCCESS==cfg_info)||(0==sw_info))
					{
						ShowAlert(search(lcontrol,"bind_suc"));
						//return NPD_DBUS_SUCCESS;
					}
										
					else if(NPD_DBUS_ACL_ERR_GENERAL==cfg_info){
						
					}
					
					if(ACL_GROUP_INDEX_ERROR==sw_info)
					  ShowAlert(search(lcontrol,"grp_index_outrange"));
					
			  }							
		} else {
			//vty_out(vty,"Failed get args.\n");
			if (dbus_error_is_set(&err)) {
				dbus_error_free(&err);
			}
		}
		dbus_message_unref(reply);
		release(lcontrol);
		return CMD_SUCCESS;
}

int unbind_aclgrp_port(char * grptype,char * aclgrpIndex,int portindex,unsigned int nodetype,struct list * lcontrol)
{
	DBusMessage *query, *reply;
	DBusError err;

	unsigned int g_index;
	unsigned int group_num,sw_info,cfg_info;
	unsigned int node_flag=0,dir_info=0,ret=0;

	node_flag= nodetype;
	g_index=portindex;
	if(strncmp("ingress",grptype,strlen(grptype))==0){
		dir_info = 0;
	}
	else if (strncmp("egress",grptype,strlen(grptype))==0){
		dir_info = 1;
	}
	else{
		return CMD_FAILURE;
	}
	if(strcmp(aclgrpIndex,"")!=0)
	{
		int temp=atoi(aclgrpIndex);
		if(temp<1 ||temp>1023)
		{
			ShowAlert(search(lcontrol,"illegal_input"));
			return 5;
		}
			
	}
	else
	{
		ShowAlert(search(lcontrol,"illegal_input"));
	}

	
	ret=dcli_str2ulongWS(aclgrpIndex,&group_num);
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_ETHPORTS_OBJPATH,NPD_DBUS_ETHPORTS_INTERFACE,NPD_DBUS_ETHPORTS_METHOD_DELETE_ACL_GROUP);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&dir_info,
							 DBUS_TYPE_UINT32,&node_flag,
 						     DBUS_TYPE_UINT32,&g_index,
							 DBUS_TYPE_UINT32,&group_num,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}
	
	
	if (dbus_message_get_args ( reply, &err,
		    DBUS_TYPE_UINT32, &sw_info,
			DBUS_TYPE_UINT32, &cfg_info,
			DBUS_TYPE_INVALID)) {
			  if(node_flag==0){
					if (NPD_DBUS_ERROR_NO_SUCH_PORT == sw_info) {
					}

					else if(ACL_GROUP_PORT_NOTFOUND  == sw_info){
						ShowAlert(search(lcontrol,"port_not_found"));
					}	
					else if(ACL_GROUP_NOT_EXISTED ==sw_info){
						
					}
					
					else if(NPD_DBUS_SUCCESS != cfg_info){
						ShowAlert(search(lcontrol,"unbind_cfg_fail"));
					}
					else if((NPD_DBUS_SUCCESS == cfg_info)&&(sw_info==NPD_DBUS_SUCCESS)){
						ShowAlert(search(lcontrol,"unbind_suc"));
					}
									
			  	}
			  else if(node_flag==1){
				   if (NPD_DBUS_ERROR_NO_SUCH_VLAN +1 == sw_info) {
						
					}
					else if (NPD_VLAN_NOTEXISTS == sw_info) {
						ShowAlert(search(lcontrol,"vlan_not_exist"));
					}

					else if(ACL_GROUP_NOT_BINDED == sw_info){
						ShowAlert(search(lcontrol,"vlan_not_found"));
					}	
					else if(ACL_GROUP_WRONG_INDEX ==sw_info){
						ShowAlert(search(lcontrol,"vlan_not_found"));
					}
					else if(NPD_DBUS_SUCCESS != cfg_info){
						ShowAlert(search(lcontrol,"unbind_cfg_fail"));
					}			
					else if((NPD_DBUS_SUCCESS == cfg_info)&&(sw_info==NPD_DBUS_SUCCESS)){
						ShowAlert(search(lcontrol,"unbind_suc"));
					}
			  }
			  if(ACL_GROUP_INDEX_ERROR==sw_info)
			  {}
			
		} else {
			if (dbus_error_is_set(&err)) {
				dbus_error_free(&err);
			}
		}
		dbus_message_unref(reply);
		return CMD_SUCCESS;
}

int enable_aclgrp(char * grptype,char * enable,int portindex,unsigned int bindtype,struct list * lcontrol)/*绑定前的enable操作*/
{
	DBusMessage *query, *reply;
	DBusError err;
	boolean isEnable = FALSE;
	unsigned int g_index,EnableInfo,op_ret;
	unsigned int node_flag,dir_info;

	g_index=portindex;
	node_flag = bindtype;
	if(strncmp("ingress",grptype,strlen(grptype))==0){
		dir_info = 0;
	}
	else if (strncmp("egress",grptype,strlen(grptype))==0){
		dir_info = 1;
	}	
	else{
		return CMD_FAILURE;
	}
	
	if(strncmp("enable",enable,strlen(enable))==0){
		isEnable = TRUE;
	}
	else if (strncmp("disable",enable,strlen(enable))==0){
		isEnable = FALSE;
	}
	else{
		return CMD_FAILURE;
	}	
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_ETHPORTS_OBJPATH,NPD_DBUS_ETHPORTS_INTERFACE,NPD_DBUS_ETHPORTS_METHOD_CONFIG_ACL);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
						     DBUS_TYPE_UINT32,&dir_info,
							 DBUS_TYPE_UINT32,&node_flag,
							 DBUS_TYPE_UINT32,&g_index,
							 DBUS_TYPE_BYTE,&isEnable,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&EnableInfo,
		DBUS_TYPE_UINT32, &op_ret,
		DBUS_TYPE_INVALID)) {
			if(node_flag==1){
				if(op_ret==NPD_DBUS_ERROR_NO_SUCH_VLAN +1)
				{
					//ShowAlert(search(lcontrol,"vlan_illegal"));
				}
				else if(op_ret==NPD_VLAN_NOTEXISTS)
				{
					//ShowAlert(search(lcontrol,"vlan_not_exist"));
				}
				else if(EnableInfo==NPD_DBUS_ERROR_NO_SUCH_PORT)
				{
					
				}
				else if(EnableInfo!=NPD_DBUS_SUCCESS)
				{
					//ShowAlert(search(lcontrol,"enable_fail"));
				}	
			}
			else if(node_flag==0){
				if(op_ret==NPD_DBUS_ERROR_NO_SUCH_PORT)
				{
					//ShowAlert(search(lcontrol,"no_such_port"));
				}
				else if(EnableInfo!=NPD_DBUS_SUCCESS)
				{
					//ShowAlert(search(lcontrol,"enable_fail"));
				}
			}
		
	} else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}


int acl_service_glabol_enable(char * enable_or_disable,struct list * lcontrol)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	char isEnable = 0;
	unsigned int op_ret;
	if(strncmp("enable",enable_or_disable,strlen(enable_or_disable))==0)
	{
		isEnable = 1;
	}
	else if (strncmp("disable",enable_or_disable,strlen(enable_or_disable))==0)
	{
		isEnable = 0;
	}
	else
	{
		return CMD_FAILURE;
	}
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_ACL_OBJPATH,NPD_DBUS_ACL_INTERFACE, NPD_DBUS_METHOD_CONFIG_ACL);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&isEnable,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{
			if (NPD_DBUS_SUCCESS == op_ret ) 
			{
				//vty_out(vty,"Access-list Service is %d\n",isEnable);
				ShowAlert(search(lcontrol,"Operation_Success"));
			}		
		
	} 
	else 
	{
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

char *  show_acl_service()
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = { 0 };
	unsigned int Isable;
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_ACL_OBJPATH,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_METHOD_SHOW_ACL_SERVICE);
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&Isable,
		DBUS_TYPE_INVALID))
	{						
		if(ACL_TRUE==Isable)
			return "enable";
			//vty_out(vty,"%% Global acl service is enabled!\n");
		if(ACL_FALSE==Isable)
			return "disable";
			//vty_out(vty,"%% Global acl service is disabled!\n");			
	} 
	else 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
#if 0
int show_aclrule_byGrpIndex(char * grpType,char * grpindex,int acl[],int * aclCount)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = { 0 };
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	unsigned int	 j,ret,dir_info=0;
	unsigned int	 count=0,index=0;
	unsigned int     groupIndex=0,portcount=0;


	if(strncmp("ingress",grpType,strlen(grpType))==0){
		dir_info = 0;
	}
	else if (strncmp("egress",grpType,strlen(grpType))==0){
		dir_info = 1;
	}
	else{
		return CMD_FAILURE;
	}
	groupIndex = dcli_str2ulong((char*)grpindex);
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_ACL_OBJPATH,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_METHOD_SHOW_ACL_GROUP_INDEX);
	dbus_error_init(&err);
	dbus_message_append_args(query, 
							 DBUS_TYPE_UINT32,&dir_info,
							 DBUS_TYPE_UINT32,&groupIndex,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	
	if (NULL == reply) 
	 {
		 if (dbus_error_is_set(&err)) 
		 {
			 dbus_error_free(&err);
		 }
		 return CMD_SUCCESS;
	 }

		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&ret);	
		if(NPD_DBUS_SUCCESS == ret){
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&count);
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&portcount);
			dbus_message_iter_next(&iter);			
			dbus_message_iter_recurse(&iter,&iter_array);
			*aclCount = count;
			for (j = 0; j < portcount; j++) {
				DBusMessageIter iter_struct;
				dbus_message_iter_recurse(&iter_array,&iter_struct);			
				dbus_message_iter_get_basic(&iter_struct,&index);
				dbus_message_iter_next(&iter_array); 
				
				//vty_out(vty,"%-40s: 1/%d\n","bind by port",index);
				}
			dbus_message_iter_next(&iter);			
			dbus_message_iter_recurse(&iter,&iter_array);
			for (j = 0; j < count; j++) {
				DBusMessageIter iter_struct;
				dbus_message_iter_recurse(&iter_array,&iter_struct);			
				dbus_message_iter_get_basic(&iter_struct,&index);
				dbus_message_iter_next(&iter_array); 
				
				//vty_out(vty,"%-40s: %d\n","acl index",index);
				acl[j]=index;
				}
		
					
		}
		else if(ACL_GROUP_NOT_EXISTED == ret)
		{			
		}
		if(ACL_GROUP_INDEX_ERROR==ret)
		{}

	dbus_message_unref(reply);
	 return CMD_SUCCESS;
}
#endif
int add_rule_group(char * addordel,char * ruleindex,unsigned int Grpindex,unsigned int dir,struct list * lcontrol)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int     ruleIndex ;
	unsigned int	  acl_group_num;
    unsigned int      group_inf,ret,num;
	unsigned int      op_flag,op_info,dir_info;


	if(0==strncmp("add",addordel,strlen(addordel)))
	   op_flag = 0;
	else if(0==strncmp("delete",addordel,strlen(addordel)))
	   op_flag = 1;
		  
    ret = dcli_str2ulongWS((char*)ruleindex,&ruleIndex);
    if (NPD_FAIL == ret) 
	{
		ShowAlert(search(lcontrol,"acl_index_outrange"));
		return CMD_FAILURE;
	}
	ruleIndex = ruleIndex-1;

	acl_group_num=Grpindex;
	dir_info=dir;

		
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_ACL_OBJPATH,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_METHOD_ADD_ACL_TO_GROUP);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&dir_info,
						     DBUS_TYPE_UINT32,&op_flag,
							 DBUS_TYPE_UINT32,&acl_group_num,
							 DBUS_TYPE_UINT32,&ruleIndex,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&group_inf,
		DBUS_TYPE_UINT32,&ret,
		DBUS_TYPE_UINT32,&num,
		DBUS_TYPE_UINT32,&op_info,
		DBUS_TYPE_INVALID)) {

			if(group_inf==NPD_DBUS_ERROR)	
		 		{}
			else if(op_flag==0)
			{
				if((ACL_GROUP_RULE_EXISTED==ret)&&(0!=num))					
					{
						ShowAlert(search(lcontrol,"rule_has_added"));
					}
				else if((EGRESS_ACL_GROUP_RULE_EXISTED==ret)&&(0!=num))
					{
						ShowAlert(search(lcontrol,"rule_has_added"));
					}					
				else if(ACL_GLOBAL_NOT_EXISTED==ret)
					{
						ShowAlert(search(lcontrol,"rule_not_exist"));
					}
				else if(ACL_GROUP_EGRESS_NOT_SUPPORT==op_info)
					{
						ShowAlert(search(lcontrol,"rule_EGRESS_NOT_SUPPORT"));
					}
				else if(ACL_ADD_EQUAL_RULE == op_info)
					{
						ShowAlert(search(lcontrol,"no_equl_acl"));
					}
				else if(NPD_DBUS_SUCCESS!=op_info)
					{
						ShowAlert(search(lcontrol,"add_rule_fail"));
					}
				else if(NPD_DBUS_SUCCESS == op_info)
					{
						ShowAlert(search(lcontrol,"add_rule_suc"));
					}
				else if(ACL_GROUP_EGRESS_ERROR==op_info)
					{
						ShowAlert(search(lcontrol,"egress_not_support"));
					}

			}
			else if(op_flag==1){
				if(ACL_GLOBAL_NOT_EXISTED==ret)
					{
						ShowAlert(search(lcontrol,"rule_not_exist"));
					}
				else if((ACL_GROUP_RULE_EXISTED==ret)&&(dir_info==ACL_DIRECTION_INGRESS)){
					if(num!=acl_group_num)
					{
						ShowAlert(search(lcontrol,"rule_not_exist_grp"));
					}
					else if(num==acl_group_num){
						if(NPD_DBUS_SUCCESS!=op_info)
						{
							ShowAlert(search(lcontrol,"del_rule_fail"));
						}
						else if(NPD_DBUS_SUCCESS==op_info)
						{
							ShowAlert(search(lcontrol,"del_rule_suc"));
						}
					}	
				}
				else if((EGRESS_ACL_GROUP_RULE_EXISTED==ret)&&(dir_info==ACL_DIRECTION_EGRESS)){
					if(num!=acl_group_num)						
						{
							ShowAlert(search(lcontrol,"rule_not_exist_grp"));
						}
					else if(num==acl_group_num){
						if(NPD_DBUS_SUCCESS!=op_info)
							{
								ShowAlert(search(lcontrol,"del_rule_fail"));
							}
						else if(NPD_DBUS_SUCCESS==op_info)
							{
								ShowAlert(search(lcontrol,"del_rule_suc"));
							}	
					}	
				}
				else if(NPD_DBUS_SUCCESS==ret){
					if(dir_info==ACL_DIRECTION_INGRESS)
						{
							ShowAlert(search(lcontrol,"rule_not_exist_grp"));
						}
					else if(dir_info==ACL_DIRECTION_EGRESS)
						{
							ShowAlert(search(lcontrol,"rule_not_exist_grp"));
						}	
				
				else if(ACL_RULE_INDEX_ERROR==ret){
					//vty_out(vty,"%% Acl rule range should be 1-1024!\n");
						ShowAlert(search(lcontrol,"acl_index_outrange"));
					}
				}			
			}
	 }

	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

int delete_acl_rule(char * ruleindex)
{
	struct list *lcontrol; 	/*解析help.txt文件的链表头*/
	lcontrol=get_chain_head("../htdocs/text/control.txt"); 
	  DBusMessage 	*query = NULL, *reply = NULL;
	  DBusError 	 err = { 0 };
	  unsigned int  ruleIndex,op_ret;
	  
	  
	  int ret;
	  
	  ret = dcli_str2ulongWS((char*)ruleindex,&ruleIndex);
	  if (NPD_FAIL == ret) {
			//vty_out(vty,"%% Illegal rule index!\n");
			ShowAlert(search(lcontrol,"illegal_input"));
			return CMD_FAILURE;
		}
	  
	  ruleIndex = ruleIndex-1;
      query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_ACL_OBJPATH,NPD_DBUS_ACL_INTERFACE, NPD_DBUS_METHOD_DELETE_ACL);
	  dbus_error_init(&err);
	  dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&ruleIndex,
							 DBUS_TYPE_INVALID);
      reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	  dbus_message_unref(query);
	  if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return CMD_SUCCESS;
	 }
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{						
		if (NPD_DBUS_ERROR == op_ret ) {
			ShowAlert(search(lcontrol,"range_error"));
		}
		else if(op_ret==ACL_GLOBAL_NOT_EXISTED)	{
			//Can't delete this acl since it is bound to ingress group
			
		}
		else if(op_ret == ACL_GROUP_RULE_EXISTED){
				ShowAlert(search(lcontrol,"acl_has_bind"));
		}	
		else if(op_ret ==EGRESS_ACL_GROUP_RULE_EXISTED){
				ShowAlert(search(lcontrol,"acl_has_bind"));
		}
		
		else if(ACL_RULE_INDEX_ERROR==op_ret){
			//vty_out(vty,"%% Acl rule range should be 1-1024!\n");
			ShowAlert(search(lcontrol,"acl_index_outrange"));
		}
		else if(ACL_MIRROR_USE==op_ret){
			//vty_out(vty,"%% Can't delete this acl since it used by mirror function!\n");
			ShowAlert(search(lcontrol,"acl_used_mirror"));
		}
		
	}
	else 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	release(lcontrol);
	return CMD_SUCCESS;	
}

int delete_acl_group(char * GrpNum,unsigned int dir)
{
	struct list *lcontrol; 	/*解析help.txt文件的链表头*/
	lcontrol=get_chain_head("../htdocs/text/control.txt"); 
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int groupNum,op_ret,groupInfo;
	unsigned int dir_info=0;

	dir_info=dir;

	dcli_str2ulong((char*)GrpNum,&groupNum);
			
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_ACL_OBJPATH,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_METHOD_DELETE_ACL_GROUP);
	
	dbus_error_init(&err);

	dbus_message_append_args(query, 
						     DBUS_TYPE_UINT32,&dir_info,
							 DBUS_TYPE_UINT32,&groupNum,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&groupInfo,
		DBUS_TYPE_UINT32,&op_ret,		
		DBUS_TYPE_INVALID)) {
			if(NPD_DBUS_ERROR==groupInfo)
			{
				//ShowAlert(search(lcontrol,"port_has_bind"));
			}
			else if(ACL_GROUP_PORT_BINDED==op_ret)
		    {
				ShowAlert(search(lcontrol,"port_has_bind"));
			}
			else if(NPD_DBUS_ERROR==op_ret)
			{
				ShowAlert(search(lcontrol,"del_grp_fail"));		
			}
			else if(NPD_DBUS_SUCCESS==op_ret)
			{
				ShowAlert(search(lcontrol,"del_grp_suc"));
			}
			if(ACL_GROUP_INDEX_ERROR==op_ret)
				ShowAlert(search(lcontrol,"grp_index_outrange"));
	} 
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	release(lcontrol);
	return CMD_SUCCESS;
}

struct acl_info show_aclinfo_Byindex(unsigned int index)
{
	DBusMessage *query = NULL, *reply = NULL;
	 DBusError err = { 0 };

	char actype[50],protype[10];
	struct acl_info  acl_info;
	acl_info.nextheader = -1;
	acl_info.ruleIndex=0;
	acl_info.groupIndex=0;
	acl_info.ruleType=(char *)malloc(20);
	memset(acl_info.ruleType,0,20);
	acl_info.protype=(char *)malloc(20);
	memset(acl_info.protype,0,20);
	acl_info.dip=(char *)malloc(20);
	memset(acl_info.dip,0,20);
	acl_info.sip=(char *)malloc(20);
	memset(acl_info.sip,0,20);
	acl_info.srcport=0; 
	acl_info.dstport=0;
	acl_info.icmp_code=0;
	acl_info.icmp_type=0;
	acl_info.actype=(char *)malloc(50);
	memset(acl_info.actype,0,50);
	acl_info.dmac=(char *)malloc(30);
	memset(acl_info.dmac,0,30);
	acl_info.smac=(char *)malloc(30);
	memset(acl_info.smac,0,30);
	acl_info.vlanid=0; 
	acl_info.source_port=(char *)malloc(30);
	memset(acl_info.source_port,0,30);
	acl_info.redirect_port=(char *)malloc(30);
	memset(acl_info.redirect_port,0,30);
	acl_info.analyzer_port=(char *)malloc(30);
	memset(acl_info.analyzer_port,0,30);
	acl_info.policerId=0;
	acl_info.up=0;
	acl_info.dscp=0;
	acl_info.egrUP=0;
	acl_info.egrDSCP=0;
	acl_info.modifyDSCP=0;
	acl_info.modifyUP=0;
	acl_info.SubQosMakers=0;
	acl_info.qosprofileindex=0;
	acl_info.upmm=(char *)malloc(30);
	memset(acl_info.upmm,0,30);
	acl_info.dscpmm=(char *)malloc(30);
	memset(acl_info.dscpmm,0,30);
	acl_info.precedence=0;
	acl_info.appendIndex=0;
	memset(acl_info.dipv6,0,sizeof(acl_info.dipv6));
	memset(acl_info.sipv6,0,sizeof(acl_info.sipv6));
//////////////////////////////////////////////////////	
	 unsigned int ruleIndex=0; 
	 unsigned int ruleType=0;
	 unsigned long dip=0;
	 unsigned long sip=0;
	 unsigned long  maskdip=0;
	 unsigned long  masksip=0;
	 unsigned long srcport=0;
	 unsigned long dstport=0;
	 unsigned char icmp_code=0;
	 unsigned char icmp_type=0;
	 unsigned char code_mask = 0;
	 unsigned char type_mask = 0;
	 unsigned long packetType=0;
	 unsigned long actionType=0; 
	 unsigned char dmac[6]={0},smac[6]={0};
	 unsigned int vlanid=0;
	 unsigned char sourceslot=0,sourceport=0,redirectslot=0,redirectport=0,mirrorslot=0,mirrorport=0;
	 unsigned int ret;
	 unsigned char sipBuf[MAX_IP_STRLEN] = {0};
	 unsigned char dipBuf[MAX_IP_STRLEN] = {0};  
	 unsigned char *sipPtr = sipBuf,*dipPtr = dipBuf;
	  unsigned int policer=0;
	  unsigned int  policerId=0;  
	  unsigned int  modifyUP=0;
	  unsigned int  modifyDSCP=0;
	  unsigned int  up=0;
	  unsigned int dscp=0;
	  unsigned int egrUP=0;
	  unsigned int egrDSCP=0;
	  unsigned int qosprofileindex=0;
	  unsigned int precedence=0;
	  unsigned char appendIndex=0;
	  char * upmm=(char *)malloc(10);
	  char * dscpmm=(char *)malloc(10);
	  memset(upmm,0,10);
	  memset(dscpmm,0,10);
	 
	 ruleIndex=index;
	 
	 ruleIndex -=1;
	  query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_ACL_OBJPATH,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_METHOD_SHOW_ACL_INDEX);
	  dbus_error_init(&err);
	  dbus_message_append_args(query, 
							  DBUS_TYPE_UINT32,&ruleIndex,
							  DBUS_TYPE_INVALID);
	  reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	  dbus_message_unref(query);
	  if (NULL == reply) 
	  {
		  //vty_out(vty,"failed get reply.\n");
		  if (dbus_error_is_set(&err)) 
		  {
			  dbus_error_free(&err);
		  }
		  return acl_info;
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
		 
		DBUS_TYPE_BYTE,&(acl_info.dipv6[0]),
		DBUS_TYPE_BYTE,&(acl_info.dipv6[1]),
		DBUS_TYPE_BYTE,&(acl_info.dipv6[2]),
		DBUS_TYPE_BYTE,&(acl_info.dipv6[3]),
		DBUS_TYPE_BYTE,&(acl_info.dipv6[4]),
		DBUS_TYPE_BYTE,&(acl_info.dipv6[5]),
		DBUS_TYPE_BYTE,&(acl_info.dipv6[6]),
		DBUS_TYPE_BYTE,&(acl_info.dipv6[7]),
		DBUS_TYPE_BYTE,&(acl_info.dipv6[8]),
		DBUS_TYPE_BYTE,&(acl_info.dipv6[9]),
		DBUS_TYPE_BYTE,&(acl_info.dipv6[10]),
		DBUS_TYPE_BYTE,&(acl_info.dipv6[11]),
		DBUS_TYPE_BYTE,&(acl_info.dipv6[12]),
		DBUS_TYPE_BYTE,&(acl_info.dipv6[13]),
		DBUS_TYPE_BYTE,&(acl_info.dipv6[14]),
		DBUS_TYPE_BYTE,&(acl_info.dipv6[15]),
		DBUS_TYPE_BYTE,&(acl_info.sipv6[0]),
		DBUS_TYPE_BYTE,&(acl_info.sipv6[1]),
		DBUS_TYPE_BYTE,&(acl_info.sipv6[2]),
		DBUS_TYPE_BYTE,&(acl_info.sipv6[3]),
		DBUS_TYPE_BYTE,&(acl_info.sipv6[4]),
		DBUS_TYPE_BYTE,&(acl_info.sipv6[5]),
		DBUS_TYPE_BYTE,&(acl_info.sipv6[6]),
		DBUS_TYPE_BYTE,&(acl_info.sipv6[7]),
		DBUS_TYPE_BYTE,&(acl_info.sipv6[8]),
		DBUS_TYPE_BYTE,&(acl_info.sipv6[9]),
		DBUS_TYPE_BYTE,&(acl_info.sipv6[10]),
		DBUS_TYPE_BYTE,&(acl_info.sipv6[11]),
		DBUS_TYPE_BYTE,&(acl_info.sipv6[12]),
		DBUS_TYPE_BYTE,&(acl_info.sipv6[13]),
		DBUS_TYPE_BYTE,&(acl_info.sipv6[14]),
		DBUS_TYPE_BYTE,&(acl_info.sipv6[15]),
		DBUS_TYPE_UINT32,&(acl_info.nextheader),
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
			  case 3:strcpy(protype,"ICMP");	  break;  
			  case 4:strcpy(protype,"Ethernet");  break;
			  case 5:strcpy(protype,"ARP"); 	  break; 
			  case 6:strcpy(protype,"All"); 	  break; 
			  case 7:strcpy(protype,"IPv6"); 	  break; 
			  
			  default:
						break;									  
		  }
		  
		  ip_long2str(dip,&dipPtr);
		  ip_long2str(sip,&sipPtr);

		  acl_info.ruleIndex=ruleIndex;
		  if(ruleType ==STANDARD_ACL_RULE)
		  {
		  	 strcpy(acl_info.ruleType,"standard");
			 if((packetType==0 )||(packetType==1)||(packetType==2)||(packetType==3))
			 {
			 	strcpy(acl_info.actype,actype);
			 	strcpy(acl_info.protype,protype);
			 	sprintf(acl_info.dip,"%s/%ld",dipPtr,maskdip);
			 	sprintf(acl_info.sip,"%s/%ld",sipPtr,masksip);
				if((strcmp(protype,"TCP")==0)||(strcmp(protype,"UDP")==0))
				{
					if(ACL_ANY_PORT == dstport)
				 	{
				 		acl_info.dstport = ACL_ANY_PORT;
				 	}
					else
					{
						acl_info.dstport=dstport;
					}

					if(ACL_ANY_PORT == srcport)
					{
						acl_info.srcport=ACL_ANY_PORT;
					}
					else
					{
						acl_info.srcport=srcport;
					}
					 	//acl_info.dstport=dstport;
					 	//acl_info.srcport=srcport;
				 }
				 if(strcmp(protype,"ICMP")==0)
				 {
				 	if(0 == code_mask)
				 	{
				 		acl_info.icmp_code=ICMP_WARNING;
				 	}
					else
					{
						acl_info.icmp_code=icmp_code;
					}

					if(0 == type_mask)
					{
						acl_info.icmp_type=ICMP_WARNING;
					}
					else
					{
						acl_info.icmp_type=icmp_type;
					}
				 		//acl_info.icmp_code=icmp_code;
				 		//acl_info.icmp_type=icmp_type;
				 }
				 if (strcmp(actype,"Redirect")==0)
				 {
				 	sprintf(acl_info.redirect_port,"%d/%d",redirectslot,redirectport);
				 }
				 
				 if (strcmp(actype,"MirrorToAnalyzer")==0)
				 {
				 	sprintf(acl_info.analyzer_port,"%d/%d",mirrorslot,mirrorport);
				 }
				 if(1==policer)
				 {
					acl_info.policerId=policerId;
					// vty_out(vty,"%-40s:%s\n","policer","enable");
				 }
				 
				 if(32==appendIndex)
				 {
					 //vty_out(vty,"%-40s:%d\n","ingress qos",qosprofileindex);
					 acl_info.appendIndex=appendIndex;
					 acl_info.qosprofileindex=qosprofileindex;
				 }
				 
				 
			 }
					 
			 else if((packetType==4)||(packetType==5))
			 {
				 strcpy(acl_info.actype,actype);
				 strcpy(acl_info.protype,protype);
				 sprintf(acl_info.smac,"%02x:%02x:%02x:%02x:%02x:%02x",smac[0],smac[1],smac[2],smac[3],smac[4],smac[5]);

				if(strcmp(protype,"Ethernet")==0)
				{
					sprintf(acl_info.dmac,"%02x:%02x:%02x:%02x:%02x:%02x",dmac[0],dmac[1],dmac[2],dmac[3],dmac[4],dmac[5]);
				}
			 
				 if(strcmp(protype,"ARP")==0)
				 {
					 strcpy(acl_info.dmac,"FF:FF:FF:FF:FF:FF");
					 acl_info.vlanid=vlanid;
					 sprintf(acl_info.source_port,"%d/%d",sourceslot,sourceport);
				 }
 
				if(strcmp(actype,"Redirect")==0)
				{
					sprintf(acl_info.redirect_port,"%d/%d",redirectslot,redirectport);
				}
 
				if (strcmp(actype,"MirrorToAnalyzer")==0)
				{
				  	sprintf(acl_info.analyzer_port,"%d/%d",mirrorslot,mirrorport);
				}
				 
				if(1==policer)
				{
					acl_info.policerId=policerId;
				}
				  
				if(32==appendIndex)
				{
					acl_info.appendIndex=appendIndex;
					acl_info.qosprofileindex=qosprofileindex;
				}
				 
			 }
			 else if((packetType==6))
			 {
				strcpy(acl_info.protype,protype);
				switch(modifyUP)
				{
					case 0:strcpy(upmm,"Keep"); break;
					case 1:
					case 2:strcpy(upmm,"Enable");break;
					default :break;
				 }
				 switch(modifyDSCP)
				 {
					case 0:strcpy(dscpmm,"Keep"); break;
					case 1:
					case 2:strcpy(dscpmm,"Enable");break;
					default :break;
				 }	 
								 
				strcpy(acl_info.actype,actype);
				
				if(strcmp(actype,"Ingress QoS Mark")==0)
				{
				 	acl_info.modifyDSCP=modifyDSCP;
					acl_info.modifyUP=modifyUP;
					if(0==modifyUP)
					{
						acl_info.up = 0;
					}
					else if(2==modifyUP)
					{
						acl_info.up=up;
					}
					
					if(0==modifyDSCP)
					{
						acl_info.dscp = 0;
					}
					else if(2==modifyDSCP)
					{
						acl_info.dscp=dscp;
					}
				 	

					 acl_info.qosprofileindex=qosprofileindex;
					 //vty_out(vty,"%-40s: %d\n","Mark QoS profile Table",1+qosprofileindex);
					 
					 if(precedence==0)
					 {
					 	acl_info.SubQosMakers=precedence;  //0  -  enable ,  1  - disable
					 }
						 //vty_out(vty,"%-40s: %s\n","SubQosMarkers","enable");
					 else if(precedence==1)
					 {
					 	acl_info.SubQosMakers=precedence;  //0  -  enable ,  1  - disable
					 }
						 //vty_out(vty,"%-40s: %s\n","SubQosMarkers","disable");
 
				 }
				 if(strcmp(actype,"Egress QoS Remark")==0){				 
					acl_info.up=up;
				 	acl_info.dscp=dscp;
				 	acl_info.egrUP=egrUP;
				 	acl_info.egrDSCP=egrDSCP;
				 }
				 strcpy(acl_info.upmm,upmm);
				 strcpy(acl_info.dscpmm,dscpmm);
				 
				 if(1==policer)
				 {
					 acl_info.policerId=policerId;
				 }
				 
			 }
		  }
		  else if(ruleType==EXTENDED_ACL_RULE)
		  {
		  	strcpy(acl_info.ruleType,"extended");
	  		strcpy(acl_info.actype,actype);
		 	strcpy(acl_info.protype,protype);
		 	sprintf(acl_info.dip,"%s/%ld",dipPtr,maskdip);
			if(ACL_ANY_PORT == dstport)
			{
				acl_info.dstport=ACL_ANY_PORT;
			}
			else
			{
		 		acl_info.dstport=dstport;
			}
		 	sprintf(acl_info.sip,"%s/%ld",sipPtr,masksip);
			if(ACL_ANY_PORT == srcport)
			{
				acl_info.srcport=ACL_ANY_PORT;
			}
			else
			{
		 		acl_info.srcport=srcport;
			}
		 	sprintf(acl_info.dmac,"%02x:%02x:%02x:%02x:%02x:%02x",dmac[0],dmac[1],dmac[2],dmac[3],dmac[4],dmac[5]);
			sprintf(acl_info.smac,"%02x:%02x:%02x:%02x:%02x:%02x",smac[0],smac[1],smac[2],smac[3],smac[4],smac[5]);
			acl_info.vlanid=vlanid;
			sprintf(acl_info.source_port,"%d/%d",sourceslot,sourceport);

			if(strcmp(protype,"ICMP")==0)
			{
				if(0 == code_mask)
				 {
				 	acl_info.icmp_code=ICMP_WARNING;
				 }
				else
				{
					acl_info.icmp_code=icmp_code;
				}

				if(0 == type_mask)
				{
					acl_info.icmp_type=ICMP_WARNING;
				}
				else
				{
					acl_info.icmp_type=icmp_type;
				}
				 		//acl_info.icmp_code=icmp_code;
				 		//acl_info.icmp_type=icmp_type;
			}
 
			if(strcmp(actype,"Redirect")==0)
			{
				sprintf(acl_info.redirect_port,"%d/%d",redirectslot,redirectport);
			}
			 
			if (strcmp(actype,"MirrorToAnalyzer")==0)
			{
				sprintf(acl_info.analyzer_port,"%d/%d",mirrorslot,mirrorport);
			}
				 
			if(1==policer)
			{
				acl_info.policerId=policerId;;
			}
			if(32==appendIndex)
			{
				acl_info.appendIndex=appendIndex;
				acl_info.qosprofileindex=qosprofileindex;
			}
			switch(modifyUP)
			{
				case 0:strcpy(upmm,"Keep"); break;
				case 1:
				case 2:strcpy(upmm,"Enable");break;
				default :break;
			}
			switch(modifyDSCP)
			{
				case 0:strcpy(dscpmm,"Keep"); break;
				case 1:
				case 2:strcpy(dscpmm,"Enable");break;
				default :break;
			}	

			if(strcmp(actype,"Ingress QoS Mark")==0)
			{
				acl_info.modifyDSCP=modifyDSCP;
				acl_info.modifyUP=modifyUP;
				if(0==modifyUP)
				{
					acl_info.up = 0;
				}
				else if(2==modifyUP)
				{
					acl_info.up=up;
				}
				if(0==modifyDSCP)
				{
					acl_info.dscp = 0;
				}
				else if(2==modifyDSCP)
				{
					acl_info.dscp=dscp;
				}
						 	

				acl_info.qosprofileindex=qosprofileindex;
							 //vty_out(vty,"%-40s: %d\n","Mark QoS profile Table",1+qosprofileindex);
							 
				if(precedence==0)
				{
					acl_info.SubQosMakers=precedence;  //0  -  enable ,  1  - disable
				}
								 //vty_out(vty,"%-40s: %s\n","SubQosMarkers","enable");
				else if(precedence==1)
				{
					acl_info.SubQosMakers=precedence;  //0  -  enable ,  1  - disable
				}
								 //vty_out(vty,"%-40s: %s\n","SubQosMarkers","disable");
		 
			}
			if(strcmp(actype,"Egress QoS Remark")==0)
			{				 
				acl_info.up=up;
				acl_info.dscp=dscp;
				acl_info.egrUP=egrUP;
				acl_info.egrDSCP=egrDSCP;
			}
			strcpy(acl_info.upmm,upmm);
			strcpy(acl_info.dscpmm,dscpmm);
				 
			 }

		 }
		
	dbus_message_unref(reply);
	free(upmm);
	free(dscpmm);
	 return acl_info;
}

int show_group_index(unsigned int dir,unsigned int grpNUm[])
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = { 0 };
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	unsigned int     k,j,ret,group_count=0,dir_info=0;
	unsigned int     group_num=0,count=0,index=0;
	dir_info=dir;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_ACL_OBJPATH,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_METHOD_SHOW_ACL_GROUP);
	dbus_error_init(&err);
	dbus_message_append_args(query, 
						     DBUS_TYPE_UINT32,&dir_info,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	
	if (NULL == reply) 
	 {
		 if (dbus_error_is_set(&err)) 
		 {
		 	dbus_error_free(&err);
		 }
		 return 0;
	 }
	 dbus_message_iter_init(reply,&iter);
 
	 dbus_message_iter_get_basic(&iter,&ret);
	 
	 if(NPD_DBUS_ERROR == ret) {
	 		return 0;
	 }
 
	 else if(NPD_DBUS_SUCCESS == ret){

		 dbus_message_iter_next(&iter);  
		 dbus_message_iter_get_basic(&iter,&group_count); 

		 dbus_message_iter_next(&iter);  
 	     dbus_message_iter_recurse(&iter,&iter_array);
 				
 	     for (j = 0; j < group_count; j++) {

			  DBusMessageIter iter_struct;
			  DBusMessageIter iter_sub_array;
			  
	 		  dbus_message_iter_recurse(&iter_array,&iter_struct);			  

			  dbus_message_iter_get_basic(&iter_struct,&group_num);
				grpNUm[j]=group_num;
			  dbus_message_iter_next(&iter_struct); 
	 		  dbus_message_iter_get_basic(&iter_struct,&count);
			 
			  dbus_message_iter_next(&iter_struct); 		  
			  dbus_message_iter_recurse(&iter_struct,&iter_sub_array);

			  for (k = 0; k < count; k++){
				  DBusMessageIter iter_sub_struct;
				  dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
							  
				  dbus_message_iter_get_basic(&iter_sub_struct,&index);
				dbus_message_iter_next(&iter_sub_struct);
			     dbus_message_iter_next(&iter_sub_array);
			  }			  	  
		 dbus_message_iter_next(&iter_array);

 	     }
	 }  

	dbus_message_unref(reply);
	return 1;
	
}

int addrule_trap_IP(unsigned int mode_type,char * Index,char * dipmask,char * sipmask)
{
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lcontrol; 	/*解析help.txt文件的链表头*/
	lpublic=get_chain_head("../htdocs/text/public.txt");
    lcontrol=get_chain_head("../htdocs/text/control.txt"); 

	DBusMessage 	*query = NULL, *reply = NULL;
	DBusError 		err = { 0 };
	unsigned int 	ruleIndex;
	unsigned long 	dipno, sipno,sipmaskLen,dipmaskLen;
	unsigned  int	ruleType;
	unsigned long	op_ret = 0;
	int Val=0;


	unsigned int ret = 0;
	ret = dcli_str2ulongWS((char*)Index,&ruleIndex);
	if (NPD_FAIL == ret) {
		ShowAlert(search(lcontrol,"acl_illegal_input"));
		return CMD_FAILURE;
	}
	
	
	ruleIndex = ruleIndex-1;
	if(mode_type==0)//standard
	{
		ruleType = STANDARD_ACL_RULE;
	}
	else//extended
	{
		ruleType =EXTENDED_ACL_RULE;

	}
	if(strncmp("any",dipmask,strlen(dipmask))==0)
	{
		   dipmaskLen=0;
		   dipno=0;
	}
	else
	{
		   Val=ip_address_format2ulong((char**)&dipmask,&dipno,&dipmaskLen);
		   
		   if(Val==CMD_FAILURE) 
		   {
		   	return CMD_WARNING;
		   }
		   VALUE_IP_MASK_CHECK(dipmaskLen);   
	}
	
	if(strncmp("any",sipmask,strlen(sipmask))==0){ 
			sipmaskLen=0;
		  	sipno=0;
		}
	else{
        Val=ip_address_format2ulong((char**)&sipmask,&sipno,&sipmaskLen);
		if(Val==CMD_WARNING)
		{
			return CMD_FAILURE;
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
		   
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{
			if (NPD_DBUS_SUCCESS == op_ret ) 
			{
				ShowAlert(search(lcontrol,"acl_add_suc"));
			}	
			else if(ACL_GLOBAL_EXISTED == op_ret)
				ShowAlert(search(lcontrol,"acl_existed"));
			else if(ACL_EXT_NO_SPACE==op_ret)
			{}
			else if(ACL_SAME_FIELD == op_ret)
			{
				ShowAlert(search(lcontrol,"same_field_not_set"));
			}
				//vty_out(vty,"%% identical fields of packet can not set again\n");
			else 
			{}
				//vty_out(vty,"%% set fail!\n");
	} 
	else 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}		
	}
	dbus_message_unref(reply);
	release(lpublic);  
	release(lcontrol);
	return CMD_SUCCESS;
}

int addrule_trap_UdpOrTcp(int mode_type,char * protocol, char * Index,char * dipmask,char * dPort,char * sipmask,char * sPort)
{
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lcontrol; 	/*解析help.txt文件的链表头*/
	lpublic=get_chain_head("../htdocs/text/public.txt");
    lcontrol=get_chain_head("../htdocs/text/control.txt"); 
    
	DBusMessage 	*query = NULL, *reply = NULL;
	DBusError 		err = { 0 };
	unsigned int 	ruleIndex;
	unsigned long 	dipno, sipno,sipmaskLen,dipmaskLen;
	unsigned long   srcport,dstport;
	unsigned int	ruleType;
	unsigned long	op_ret;
	int Val=0;
	char * alert=(char *)malloc(1024);
	memset(alert,0,1024);

	INDEX_LENTH_CHECK_ACL((char*)Index,4);
	INDEX_LENTH_CHECK_ACL((char*)dPort,5);
	INDEX_LENTH_CHECK_ACL((char*)sPort,5);
	

	unsigned int ret = 0;
	ret = dcli_str2ulongWS((char*)Index,&ruleIndex);
	if (NPD_FAIL == ret) {
		ShowAlert(search(lcontrol,"acl_illegal_input"));
		return CMD_FAILURE;
	}
	ruleIndex = ruleIndex-1;
	
	if(mode_type==0)//standard
	{
		ruleType = STANDARD_ACL_RULE;
	}
	else
	{
		ruleType = EXTENDED_ACL_RULE;
	}

	 if(strncmp("any",dipmask,strlen(dipmask))==0){
		dipmaskLen=0;
	  	dipno=0;
       }
	else{
		Val=ip_address_format2ulong((char**)&dipmask,&dipno,&dipmaskLen);
		VALUE_IP_MASK_CHECK(dipmaskLen);
		if(Val==CMD_FAILURE)  return CMD_FAILURE;
	}
	
	
	if(strncmp("any",dPort,strlen(dPort))==0)             
        dstport=ACL_ANY_PORT;    
    else
    	{
    		ret = dcli_str2ulongWS((char*)dPort,&dstport);
      	if (NPD_FAIL == ret) {
      			ShowAlert(search(lcontrol,"illegal_input"));
      			return CMD_FAILURE;
      		}
    	}
	
	if(strncmp("any",sipmask,strlen(sipmask))==0){ 
			sipmaskLen=0;
		  	sipno=0;
	}
	else{					
		 Val=ip_address_format2ulong((char**)&sipmask,&sipno,&sipmaskLen);
		 VALUE_IP_MASK_CHECK(sipmaskLen);
		 if(Val==CMD_FAILURE)  return CMD_FAILURE;
	}
	
	
    if(strncmp("any",sPort,strlen(sPort))==0)           
        srcport=ACL_ANY_PORT;      
    else
        {
    		ret = dcli_str2ulongWS((char*)sPort,&srcport);
      		if (NPD_FAIL == ret) {
      			ShowAlert(search(lcontrol,"illegal_input"));
      			return CMD_FAILURE;
      		}
    	}

    if(strcmp(dPort,"")!=0)
	{
		long temp1=atol(dPort);
		if(temp1<0 || temp1 >65535)
		{
			ShowAlert(search(lcontrol,"port_outrange"));
			return -1;
		}
	}
	if(strcmp(sPort,"")!=0)
	{
		long temp2=atol(sPort);
		if(temp2<0 || temp2 >65535)
		{
			ShowAlert(search(lcontrol,"port_outrange"));
			return -1;
		}
	}
     
    if(strcmp(protocol,"tcp")==0)
    	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_ACL_OBJPATH,NPD_DBUS_ACL_INTERFACE, NPD_DBUS_METHOD_CONFIG_ACL_RULE_TRAP_TCP);
	else if(strcmp(protocol,"udp")==0)
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
   
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{
			if (NPD_DBUS_SUCCESS == op_ret ) 
			{
				ShowAlert(search(lcontrol,"acl_add_suc"));
			}
			else if(ACL_GLOBAL_EXISTED == op_ret)
				ShowAlert(search(lcontrol,"acl_existed"));
			else if(ACL_EXT_NO_SPACE==op_ret)
		          //vty_out(vty,"%% because the extended should take up twice spaces than stdard acl,but %d has been set extended!\n",(ruleIndex-511));
			{
				sprintf(alert,"%s %d %s",search(lcontrol,"acl_no_space_alert_one"),(ruleIndex-511),search(lcontrol,"acl_no_space_alert_two"));
				ShowAlert(alert);
			}
			else if(ACL_SAME_FIELD == op_ret)
				//vty_out(vty,"%% identical fields of packet can not set again\n");
				{ShowAlert(search(lcontrol,"same_field_not_set"));}
			else 
				//vty_out(vty,"%% set fail!\n");
				{}
		
	} 
	else 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	release(lpublic);  
	release(lcontrol);
	free(alert);
	return CMD_SUCCESS;

}

int addrule_trap_Icmp(int mode_type,char * Index,char * dipmask,char * sipmask,char * type,char * code)
{
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lcontrol;	/*解析help.txt文件的链表头*/
	lpublic=get_chain_head("../htdocs/text/public.txt");
	lcontrol=get_chain_head("../htdocs/text/control.txt"); 

	DBusMessage 	*query = NULL, *reply = NULL;
	DBusError		err = { 0 };
	unsigned int	ruleIndex;
	unsigned long	 dipno, sipno,sipmaskLen,dipmaskLen;
	unsigned int 	ruleType;
	unsigned long	op_ret;
	unsigned char   typeno = 0,codeno = 0,acl_any = 0;
	unsigned long    tmp1=0,tmp2=0;
	int Val=0;
	char * alert=(char *)malloc(1024);
	memset(alert,0,1024);


	INDEX_LENTH_CHECK_ACL((char*)Index,4);
	INDEX_LENTH_CHECK_ACL((char*)type,3);
	INDEX_LENTH_CHECK_ACL((char*)code,3);
	if(strcmp(type,"")==0)
	{}
	else
	{
		int temp=atoi(type);
		if(temp<0 || temp>255)
		{
			ShowAlert(search(lcontrol,"illegal_input"));
			return 1;
		}
	}
	if(strcmp(code,"")==0)
	{}
	else
	{
		int temp=atoi(code);
		if(temp<0 || temp>255)
		{
			ShowAlert(search(lcontrol,"illegal_input"));
			return 1;
		}
	}
	
	unsigned int ret = 0;
	ret = dcli_str2ulongWS((char*)Index,&ruleIndex);
	if (NPD_FAIL == ret) {
		ShowAlert(search(lcontrol,"acl_illegal_input"));
		return CMD_FAILURE;
	}
	ruleIndex = ruleIndex-1;
	if(mode_type==0)//standard
	{
		ruleType = STANDARD_ACL_RULE;
	}
	else//extended
	{
		ruleType = EXTENDED_ACL_RULE;
	}

	if(strncmp("any",dipmask,strlen(dipmask))==0)
		{
			dipmaskLen=0;
			dipno=0;
		}
	else{	
			Val=ip_address_format2ulong((char**)&dipmask,&dipno,&dipmaskLen);
			VALUE_IP_MASK_CHECK(dipmaskLen);
			if(Val==CMD_FAILURE)  return CMD_FAILURE;
	}



	if(strncmp("any",sipmask,strlen(sipmask))==0)
		{ 
			sipmaskLen=0;
			sipno=0;
		}
	else
		{						
		  Val=ip_address_format2ulong((char**)&sipmask,&sipno,&sipmaskLen); 
		  VALUE_IP_MASK_CHECK(sipmaskLen); 
		  if(Val==CMD_FAILURE) return CMD_FAILURE;
	}


	
	if(strncmp("",type,strlen(type))==0)	
	{
		typeno=0;
		acl_any |= 1;

	}
	else
	{
		ret= dcli_str2ulongWS((char*)type,&tmp1);
		if (NPD_FAIL == ret) {
			ShowAlert(search(lcontrol,"illegal_input"));
			//vty_out(vty,"%% Illegal icmp type!\n");
			return CMD_FAILURE;
		}
		typeno = tmp1;
	}
		
	  
	if(strncmp("",code,strlen(code))==0)	
	{
		codeno=0;
		acl_any |= 2;

	}
	else
	{
		ret= dcli_str2ulongWS((char*)code,&tmp2);
		if (NPD_FAIL == ret) {
			ShowAlert(search(lcontrol,"illegal_input"));
			//vty_out(vty,"%% Illegal icmp type!\n");
			return CMD_FAILURE;
		}
		codeno = tmp2;
	}
	//typeno = tmp1;
	//codeno = tmp2;

	
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
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{
			if (NPD_DBUS_SUCCESS == op_ret ) 
			{
				ShowAlert(search(lcontrol,"acl_add_suc"));
			}	
			else if(ACL_GLOBAL_EXISTED == op_ret)
				ShowAlert(search(lcontrol,"acl_existed"));
			
			else if(ACL_EXT_NO_SPACE==op_ret)
		          //vty_out(vty,"%% because the extended should take up twice spaces than stdard acl,but %d has been set extended!\n",(ruleIndex-511));
			{
				sprintf(alert,"%s %d %s",search(lcontrol,"acl_no_space_alert_one"),(ruleIndex-511),search(lcontrol,"acl_no_space_alert_two"));
				ShowAlert(alert);

			}
			else if(ACL_SAME_FIELD == op_ret)
				//vty_out(vty,"%% identical fields of packet can not set again\n");
				{ShowAlert(search(lcontrol,"same_field_not_set"));}
			else 
				//vty_out(vty,"%% set fail!\n");
				{}
	} 
	else 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	release(lpublic);
	release(lcontrol);
	free(alert);
	return CMD_SUCCESS;	
}

int addrule_trap_ethernet(int mode_type,char * Index,char * dmac,char * smac)
{
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lcontrol;	/*解析help.txt文件的链表头*/
	lpublic=get_chain_head("../htdocs/text/public.txt");
	lcontrol=get_chain_head("../htdocs/text/control.txt"); 

	DBusMessage 	*query = NULL, *reply = NULL;
	DBusError		err = { 0 };
	ETHERADDR		dmacAddr,smacAddr;
	unsigned int	ruleIndex; 
	unsigned int	ruleType,op_ret;
	char * alert= (char *)malloc(1024);
	memset(alert,0,1024);
	

	INDEX_LENTH_CHECK_ACL((char*)Index,4);
	
	unsigned int ret = 0;
	ret = dcli_str2ulongWS((char*)Index,&ruleIndex);
	if (NPD_FAIL == ret) {
		ShowAlert(search(lcontrol,"acl_illegal_input"));
		return CMD_FAILURE;
	}
	ruleIndex = ruleIndex-1;
	if(mode_type==0)//standard
	{
		ruleType = STANDARD_ACL_RULE; 
	}
	else
	{
		ruleType = EXTENDED_ACL_RULE;
	}

	memset(&smacAddr,0,sizeof(ETHERADDR));
	memset(&dmacAddr,0,sizeof(ETHERADDR));


	if(strncmp("any",dmac,strlen(dmac))!=0){
		op_ret = parse_mac_addr((char *)dmac,&dmacAddr);
		if (NPD_FAIL == op_ret) {
			ShowAlert(search(lcontrol,"unkown_mac_format"));
			return CMD_SUCCESS;
		}
	}
	/*fetch the 1st param : SMAC addr*/
	if(strncmp("any",smac,strlen(smac))!=0){
		op_ret = parse_mac_addr((char *)smac,&smacAddr);
		if (NPD_FAIL == op_ret) {
			ShowAlert(search(lcontrol,"unkown_mac_format"));
			return CMD_SUCCESS;
		}
	}
		

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_ACL_OBJPATH,NPD_DBUS_ACL_INTERFACE, NPD_DBUS_METHOD_CONFIG_ACL_RULE_TRAP_MAC);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&ruleIndex,	
							 DBUS_TYPE_UINT32,&ruleType,
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
      
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{
			if (NPD_DBUS_SUCCESS == op_ret ) 
			{
				ShowAlert(search(lcontrol,"acl_add_suc"));
			}
			else if(ACL_GLOBAL_EXISTED == op_ret)
				ShowAlert(search(lcontrol,"acl_existed"));
			
			else if(ACL_EXT_NO_SPACE==op_ret)
				{
					sprintf(alert,"%s %d %s",search(lcontrol,"acl_no_space_alert_one"),(ruleIndex-511),search(lcontrol,"acl_no_space_alert_two"));
					ShowAlert(alert);

				}
		          //vty_out(vty,"%% because the extended should take up twice spaces than stdard acl,but %d has been set extended!\n",(ruleIndex-511));
	
			else if(ACL_SAME_FIELD == op_ret)
			{ShowAlert(search(lcontrol,"same_field_not_set"));}
				//vty_out(vty,"%% identical fields of packet can not set again\n");
			else 
			{}
				//vty_out(vty,"%% set fail!\n");
		
	} 
	else 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	free(alert);
	release(lpublic);
	release(lcontrol);
	return CMD_SUCCESS;
}

int addrule_trap_arp(int mode_type,char * Index,char * smac,char * vlanid,char * slotport)
{
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lcontrol;	/*解析help.txt文件的链表头*/
	lpublic=get_chain_head("../htdocs/text/public.txt");
	lcontrol=get_chain_head("../htdocs/text/control.txt"); 

	DBusMessage 	*query = NULL, *reply = NULL;
  	DBusError		err = { 0 };
	ETHERADDR		smacAddr,maskMac;
	unsigned char   slot_no=ACL_ANY_PORT_CHAR,port_no=ACL_ANY_PORT_CHAR;
	unsigned int	ruleIndex; 
	unsigned int	ruleType,op_ret,vlanId=0,ret,eth_ret;
	char *			str[20]={"ff:ff:ff:ff:ff:ff"};
	char *			strcp;
	char * alert=(char *)malloc(1024);
	memset(alert,0,1024);
	INDEX_LENTH_CHECK_ACL((char*)Index,4);
	INDEX_LENTH_CHECK_ACL((char*)vlanid,4);
	

	ret = dcli_str2ulongWS((char*)Index,&ruleIndex);
	if (NPD_FAIL == ret) {
		ShowAlert(search(lcontrol,"acl_illegal_input"));
		return CMD_FAILURE;
	}
	ruleIndex = ruleIndex-1; 	
	if(mode_type==0)//standard
	{
		ruleType = STANDARD_ACL_RULE; 
	}else
	{
		ruleType = EXTENDED_ACL_RULE;
	}
	
	memset(&smacAddr,0,sizeof(ETHERADDR));
	memset(&maskMac,0,sizeof(ETHERADDR));

	/*fetch the 1st param : SMAC addr*/
	if(strncmp("any",smac,strlen(smac))!=0){
		op_ret = parse_mac_addr((char *)smac,&smacAddr);
		if (NPD_FAIL == op_ret) {
	    	ShowAlert(search(lcontrol,"unkown_mac_format"));
			return CMD_SUCCESS;
		}
	
		/*convert mac_mask to standard format*/
		strcp=(char *)malloc(50*sizeof(char));
		memset(strcp,'\0',50);
	    strcpy(strcp,str[0]);
		
		op_ret = parse_mac_addr(strcp,&maskMac);
		if (NPD_FAIL == op_ret) {
	    	ShowAlert(search(lcontrol,"unkown_mac_format"));
			return CMD_SUCCESS;
		}
	}
    /*vlan id*/
	if(strncmp("any",vlanid,strlen(vlanid))!=0){
		ret = dcli_str2ulongWS((char*)vlanid,&vlanId);
      	if (NPD_FAIL == ret) {
      		ShowAlert(search(lcontrol,"illegal_input"));
      		return CMD_FAILURE;
      	}
	}
	/*slot/port*/
	if(strncmp("any",slotport,strlen(slotport))!=0){
		ret = parse_slotport_no((char *)slotport,&slot_no,&port_no);

		if (NPD_FAIL == ret) {
	    	ShowAlert(search(lcontrol,"illegal_slot_pot_input"));
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
   	#if 0
	   vty_out(vty,"before data taras!\n");
	   vty_out(vty,"ruleIndex %d \n",ruleIndex);

	   vty_out(vty,"source mac addr %0x:%0x:%0x:%0x:%0x:%0x.\n",smacAddr.arEther[0],
														smacAddr.arEther[1],
														smacAddr.arEther[2],
														smacAddr.arEther[3],
														smacAddr.arEther[4],
														smacAddr.arEther[5]);
		
	   vty_out(vty,"mac mask addr %0x:%0x:%0x:%0x:%0x:%0x.\n",maskMac.arEther[0],
														maskMac.arEther[1],
														maskMac.arEther[2],
														maskMac.arEther[3],
														maskMac.arEther[4],
														maskMac.arEther[5]);
	   vty_out(vty,"vlan id %d\n",vlanId);
	   vty_out(vty,"port %d-%d\n",slot_no,port_no);
	   
   	#endif
   
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_UINT32,&eth_ret,		
		DBUS_TYPE_INVALID))
	{

			if(NPD_DBUS_SUCCESS != eth_ret){
			}
			else{
				if (NPD_DBUS_SUCCESS == op_ret ) 
				{
					ShowAlert(search(lcontrol,"acl_add_suc"));
					
				}
				else if(ACL_GLOBAL_EXISTED == op_ret)
					ShowAlert(search(lcontrol,"acl_existed"));
				
				else if(ACL_EXT_NO_SPACE==op_ret)
				{
					sprintf(alert,"%s %d %s",search(lcontrol,"acl_no_space_alert_one"),(ruleIndex-511),search(lcontrol,"acl_no_space_alert_two"));
					ShowAlert(alert);
				}
			      //vty_out(vty,"%% because the extended should take up twice spaces than stdard acl,but %d has been set extended!\n",(ruleIndex-511));
		
				else if(ACL_SAME_FIELD == op_ret)
				{ShowAlert(search(lcontrol,"same_field_not_set"));}
					//vty_out(vty,"%% identical fields of packet can not set again\n");	
				else 
				{}
					//vty_out(vty,"%% set fail!\n");
			}			
	} 
	else 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	free(alert);
	release(lpublic);
	release(lcontrol);
	return CMD_SUCCESS;
}

int addrule_PermitOrDeny_arp(int mode_type,char * Index,char * action ,char * smac,char * vlanid,char * slotport,char * policerID)
{
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lcontrol;	/*解析help.txt文件的链表头*/
	lpublic=get_chain_head("../htdocs/text/public.txt");
	lcontrol=get_chain_head("../htdocs/text/control.txt"); 

	DBusMessage 	*query = NULL, *reply = NULL;
  	DBusError		err = { 0 };
	ETHERADDR		smacAddr,maskMac;
	unsigned char   slot_no=ACL_ANY_PORT_CHAR,port_no=ACL_ANY_PORT_CHAR;
	unsigned int	ruleIndex; 
	unsigned int	ruleType,actionType,op_ret,vlanId=0,ret,eth_ret;
	char *			str[20]={"ff:ff:ff:ff:ff:ff"};
	char *			strcp;
	unsigned int    policer=0,policerId=0;
	char * alert=(char *)malloc(1024);
	memset(alert,0,1024);

	INDEX_LENTH_CHECK_ACL((char*)Index,4);
	INDEX_LENTH_CHECK_ACL((char*)vlanid,4);
	

	ret = dcli_str2ulongWS((char*)Index,&ruleIndex);
	if (NPD_FAIL == ret) {
		ShowAlert(search(lcontrol,"acl_illegal_input"));
		return CMD_FAILURE;
	}
	ruleIndex = ruleIndex-1; 	
	if(mode_type==0)//standard
	{
		ruleType = STANDARD_ACL_RULE; 
	}
	else
	{
		ruleType = EXTENDED_ACL_RULE;
	}
	
	if(strncmp("permit",action,strlen(action))==0) { actionType=0;}
    else if(strncmp("deny",action,strlen(action))==0) {actionType=1;}      
   
    		
	memset(&smacAddr,0,sizeof(ETHERADDR));
	memset(&maskMac,0,sizeof(ETHERADDR));
	
	/*fetch the 1st param : SMAC addr*/
	if(strncmp("any",smac,strlen(smac))!=0){
		op_ret = parse_mac_addr((char *)smac,&smacAddr);
		if (NPD_FAIL == op_ret) {
	    	ShowAlert(search(lcontrol,"unkown_mac_format"));
			return CMD_SUCCESS;
		}
	
		/*convert mac_mask to standard format*/
		strcp=(char *)malloc(50*sizeof(char));
		memset(strcp,'\0',50);
	    strcpy(strcp,str[0]);
		
		op_ret = parse_mac_addr(strcp,&maskMac);
		if (NPD_FAIL == op_ret) {
	    	ShowAlert(search(lcontrol,"unkown_mac_format"));
			return CMD_SUCCESS;
		}
	}
    /*vlan id*/
	
	if(strncmp("any",vlanid,strlen(vlanid))!=0){
		 ret= dcli_str2ulongWS((char*)vlanid,&vlanId);
 		if(ret==NPD_FAIL)
 		{
 			ShowAlert(search(lcontrol,"illegal_input"));
 			return CMD_FAILURE;
 		}
	}
	/*slot/port*/
	if(strncmp("any",slotport,strlen(slotport))!=0){
		ret = parse_slotport_no((char *)slotport,&slot_no,&port_no);

		if (NPD_FAIL == ret) {
	    	ShowAlert(search(lcontrol,"illegal_slot_pot_input"));
			return CMD_SUCCESS;
		}
	}
	//policer
		if(actionType==0){	
			if(strcmp(policerID,"0")!=0){
				policer = 1;
				//policerId = dcli_str2ulong(policerID);
				//policerId = policerId-1;
				ret= dcli_str2ulongWS((char*)policerID,&policerId);
         		if(ret==NPD_FAIL)
         		{
         			ShowAlert(search(lcontrol,"illegal_input"));
         			return CMD_FAILURE;
         		}
			}		
		}
		else if(actionType==1){
			policer =0;
			policerId =0;
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
  
   
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_UINT32,&eth_ret,		
		DBUS_TYPE_INVALID))
	{

			if(NPD_DBUS_SUCCESS != eth_ret){
			}
			else{
				if (NPD_DBUS_SUCCESS == op_ret ) 
				{
					ShowAlert(search(lcontrol,"acl_add_suc"));
				}
				else if(ACL_GLOBAL_EXISTED == op_ret)
					ShowAlert(search(lcontrol,"acl_existed"));
				
				else if(ACL_EXT_NO_SPACE==op_ret)
				{
					sprintf(alert,"%s %d %s",search(lcontrol,"acl_no_space_alert_one"),(ruleIndex-511),search(lcontrol,"acl_no_space_alert_two"));
					ShowAlert(alert);
				}
				      //vty_out(vty,"%% because the extended should take up twice spaces than stdard acl,but %d has been set extended!\n",(ruleIndex-511));
	
				else if(ACL_SAME_FIELD == op_ret)
				{ShowAlert(search(lcontrol,"same_field_not_set"));}
					//vty_out(vty,"%% identical fields of packet can not set again\n");	
				else if(ACL_POLICER_ID_NOT_SET == op_ret)
				{}
					//vty_out(vty,"%% Policer %d not existed!\n",policerId);
				else 
				{}
					//vty_out(vty,"%% set fail!\n");
			}			
	} 
	else 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	free(alert);
	release(lpublic);
	release(lcontrol);
	return CMD_SUCCESS;
}

int addrule_PermitOrDeny_ethernet(int mode_type,char * Index,char * action,char * dmac ,char * smac,char * policerID)
{
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lcontrol;	/*解析help.txt文件的链表头*/
	lpublic=get_chain_head("../htdocs/text/public.txt");
	lcontrol=get_chain_head("../htdocs/text/control.txt"); 

	DBusMessage 	*query = NULL, *reply = NULL;
	DBusError		err = { 0 };
	ETHERADDR		dmacAddr,smacAddr;
	unsigned int	ruleIndex,actionType; 
	unsigned int	ruleType,op_ret;
	unsigned int 	policer=0,policerId=0;
	char * alert=(char *)malloc(1024);
	memset(alert,0,1024);
#if 0
	if((argc < 2)||(argc > 4))
	{
		vty_out(vty, "bad parameters number!\n");
		return CMD_FAILURE;
	}
#endif
	INDEX_LENTH_CHECK_ACL((char*)Index,4);

	int ret=0;
	ret = dcli_str2ulongWS((char*)Index,&ruleIndex);
	if (NPD_FAIL == ret) {
		ShowAlert(search(lcontrol,"acl_illegal_input"));
		return CMD_FAILURE;
	}
	ruleIndex = ruleIndex-1; 
	
	if(mode_type==0)//standard
	{
		ruleType = STANDARD_ACL_RULE; 
	}
	else
	{
		ruleType = EXTENDED_ACL_RULE;
	}

	if(strncmp("permit",action,strlen(action))==0)
        actionType=0;
    else if(strncmp("deny",action,strlen(action))==0)
        actionType=1;
   
	memset(&smacAddr,0,sizeof(ETHERADDR));
	memset(&dmacAddr,0,sizeof(ETHERADDR));
	//memset(&maskMac,0,sizeof(ETHERADDR));
	/*fetch the 1st param : DMAC addr*/
	if(strncmp("any",dmac,strlen(dmac))!=0){
		op_ret = parse_mac_addr((char *)dmac,&dmacAddr);
		if (NPD_FAIL == op_ret) {
			ShowAlert(search(lcontrol,"unkown_mac_format"));
			return CMD_SUCCESS;
		}
	}
	/*fetch the 1st param : SMAC addr*/
	if(strncmp("any",smac,strlen(smac))!=0){
		op_ret = parse_mac_addr((char *)smac,&smacAddr);
		if (NPD_FAIL == op_ret) {
			ShowAlert(search(lcontrol,"unkown_mac_format"));
			return CMD_SUCCESS;
		}
	}
	 //policer
			if(actionType==0){	
			if(strcmp(policerID,"0")!=0){
				policer = 1;
				dcli_str2ulong(policerID,&policerId);
				//policerId = policerId-1;
			}		
		}
		else if(actionType==1){
			policer =0;
			policerId =0;
		}
	/*query*/
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_ACL_OBJPATH,NPD_DBUS_ACL_INTERFACE, NPD_DBUS_METHOD_CONFIG_ACL_RULE_DENY_MAC);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&ruleIndex,	
							 DBUS_TYPE_UINT32,&ruleType,
							 DBUS_TYPE_UINT32,&actionType,
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
 
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{
			if (NPD_DBUS_SUCCESS == op_ret ) 
			{
				ShowAlert(search(lcontrol,"acl_add_suc"));
			}
			else if(ACL_GLOBAL_EXISTED == op_ret)
				ShowAlert(search(lcontrol,"acl_existed"));
			
			else if(ACL_EXT_NO_SPACE==op_ret)
			{
				sprintf(alert,"%s %d %s",search(lcontrol,"acl_no_space_alert_one"),(ruleIndex-511),search(lcontrol,"acl_no_space_alert_two"));
				ShowAlert(alert);
			}
		         // vty_out(vty,"%% because the extended should take up twice spaces than stdard acl,but %d has been set extended!\n",(ruleIndex-511));
	
			else if(ACL_SAME_FIELD == op_ret)
			{ShowAlert(search(lcontrol,"same_field_not_set"));}
				//vty_out(vty,"%% identical fields of packet can not set again\n");	
			else if(ACL_POLICER_ID_NOT_SET == op_ret)
			{}
					//vty_out(vty,"%% Policer %d not existed!\n",policerId);
			else
			{}
				//vty_out(vty,"%% set fail!\n");
		
	} 
	else 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	free(alert);
	release(lpublic);
	release(lcontrol);
	return CMD_SUCCESS;
}

int addrule_PermitOrDeny_icmp(int mode_type,char * Index,char * action,char * dipmask,char * sipmask,char * type,char * code,char * policerID)
{
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lcontrol;	/*解析help.txt文件的链表头*/
	lpublic=get_chain_head("../htdocs/text/public.txt");
	lcontrol=get_chain_head("../htdocs/text/control.txt"); 

	DBusMessage 	*query = NULL, *reply = NULL;
	DBusError		err = { 0 };
	unsigned int	ruleIndex;
	unsigned long	 dipno, sipno,sipmaskLen,dipmaskLen;
	unsigned  int	ruleType,actionType;
	unsigned long	op_ret;
	unsigned char   typeno = 0,codeno = 0,acl_any = 0;
	unsigned int	policer=0,policerId=0;
	int Val=0;	
	unsigned int     tmp1=0,tmp2=0;
	char *alert = (char *)malloc(1024);
	memset(alert,0,1024);
	
	if(strcmp(type,"")==0)
	{
	}
	else
	{
		int temp=atoi(type);
		if(temp<0 || temp>255)
		{
			ShowAlert(search(lcontrol,"illegal_input"));
			return 1;
		}
	}
	if(strcmp(code,"")==0)
	{}
	else
	{
		int temp=atoi(code);
		if(temp<0 || temp>255)
		{
			ShowAlert(search(lcontrol,"illegal_input"));
			return 1;
		}
	}
	int ret;
	ret = dcli_str2ulongWS((char*)Index,&ruleIndex);
	if (NPD_FAIL == ret) {
		ShowAlert(search(lcontrol,"acl_illegal_input"));
		return CMD_FAILURE;
	}
	ruleIndex = ruleIndex-1;

	if(mode_type==0)//standard
	{
		ruleType = STANDARD_ACL_RULE; 
	}
	else
	{
		ruleType = EXTENDED_ACL_RULE;
	}

	if(strncmp("permit",action,strlen(action))==0) { actionType=0;}
    else if(strncmp("deny",action,strlen(action))==0) {actionType=1;}      
   

	if(strncmp("any",dipmask,strlen(dipmask))==0)
		{
			dipmaskLen=0;
			dipno=0;
		}
	else
		{	
			Val=ip_address_format2ulong((char**)&dipmask,&dipno,&dipmaskLen);
			VALUE_IP_MASK_CHECK(dipmaskLen);
			if(Val==CMD_FAILURE)  return CMD_FAILURE;
	}


	if(strncmp("any",sipmask,strlen(sipmask))==0)
		{ 
			sipmaskLen=0;
			sipno=0;
		}
	else
		{						
		  Val=ip_address_format2ulong((char**)&sipmask,&sipno,&sipmaskLen);
		  VALUE_IP_MASK_CHECK(sipmaskLen);
		  if(Val==CMD_FAILURE)  return CMD_FAILURE;
		  
	}
	
	
	
	if(strncmp("",type,strlen(type))==0)	
	{
		typeno=0;
		acl_any |= 1;

	}
	else
	{
		ret= dcli_str2ulongWS((char*)type,&tmp1);
		if(ret==NPD_FAIL)
	   	{
			//vty_out(vty,"%% Illegal icmp type!\n");
			ShowAlert(search(lcontrol,"illegal_input"));
			return CMD_FAILURE;
	     	}
		typeno = tmp1;
	} 
	
		  
	if(strncmp("",code,strlen(code))==0)	
	{
		codeno=0;
		acl_any |= 2;
	}
	else{
		ret= dcli_str2ulongWS((char*)code,&tmp2);
		if(ret==NPD_FAIL)
	   	{
			//vty_out(vty,"%% Illegal icmp type!\n");
			ShowAlert(search(lcontrol,"illegal_input"));
			return CMD_FAILURE;
	     }
		codeno = tmp2;
	} 

	//policer
		if(actionType==0){	
		if(strcmp(policerID,"0")!=0){
			policer = 1;
			ret= dcli_str2ulongWS((char*)policerID,&policerId);
     		if(ret==NPD_FAIL)
     	   	{
     			//vty_out(vty,"%% Illegal icmp type!\n");
     			ShowAlert(search(lcontrol,"illegal_input"));
     			return CMD_FAILURE;
     	     }
		}		
	}
	else if(actionType==1){
		policer =0;
		policerId =0;
	}
	//typeno = tmp1;
	//codeno =tmp2;
		
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

		   
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{
			if (NPD_DBUS_SUCCESS == op_ret ) {
				ShowAlert(search(lcontrol,"acl_add_suc"));
			}
			else if(ACL_GLOBAL_EXISTED == op_ret)
				ShowAlert(search(lcontrol,"acl_existed"));
			
			else if(ACL_EXT_NO_SPACE==op_ret)
			{
				sprintf(alert,"%s %d %s",search(lcontrol,"acl_no_space_alert_one"),(ruleIndex-511),search(lcontrol,"acl_no_space_alert_two"));
				ShowAlert(alert);
			}
		           //vty_out(vty,"%% because the extended should take up twice spaces than stdard acl,but %d has been set extended!\n",(ruleIndex-511));
	
			else if(ACL_SAME_FIELD == op_ret)
			{ShowAlert(search(lcontrol,"same_field_not_set"));}
				//vty_out(vty,"%% identical fields of packet can not set again\n");	
			else if(ACL_POLICER_ID_NOT_SET == op_ret)
			{
				ShowAlert(search(lcontrol,"same_field_not_set"));
			}
				//vty_out(vty,"%% Policer %d not existed!\n",policerId);
			else
			{
				ShowAlert(search(lcontrol,"same_field_not_set"));
			}
				//vty_out(vty,"%% set fail!\n");
		
	} 
	else 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	free(alert);
	release(lpublic);
	release(lcontrol);
	return CMD_SUCCESS;
}

int addrule_PermitOrDeny_ip(int mode_type,char * Index,char * action,char * dipmask,char * sipmask,char * policerID)
{
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lcontrol;	/*解析help.txt文件的链表头*/
	lpublic=get_chain_head("../htdocs/text/public.txt");
	lcontrol=get_chain_head("../htdocs/text/control.txt"); 

	DBusMessage 	*query = NULL, *reply = NULL;
	DBusError		err = { 0 };
	unsigned int	ruleIndex;
	unsigned long	dipno, sipno,sipmaskLen,dipmaskLen;
	unsigned  int	ruleType,actionType;
	unsigned long	op_ret;
	unsigned int	policer=0,policerId=0;
	int Val=0;
	char *alert=(char *)malloc(1024);
	memset(alert,0,1024);
	
	INDEX_LENTH_CHECK_ACL((char*)Index,4);

	int ret = 0;
	ret = dcli_str2ulongWS((char*)Index,&ruleIndex);
	if (NPD_FAIL == ret) {
		ShowAlert(search(lcontrol,"acl_illegal_input"));
		return CMD_FAILURE;
	}
	ruleIndex = ruleIndex-1;

	if(mode_type==0)//standard
	{
		ruleType = STANDARD_ACL_RULE; 
	}
	else
	{
		ruleType = EXTENDED_ACL_RULE;
	}

	if(strncmp("permit",action,strlen(action))==0) { actionType=0;}
    else if(strncmp("deny",action,strlen(action))==0) {actionType=1;}      
   

	if(strncmp("any",dipmask,strlen(dipmask))==0)
		{
			dipmaskLen=0;
			dipno=0;
		}
	else
		{	
			Val=ip_address_format2ulong((char**)&dipmask,&dipno,&dipmaskLen);
			VALUE_IP_MASK_CHECK(dipmaskLen);
			if(Val==CMD_FAILURE)  return CMD_FAILURE;
	  }

	if(strncmp("any",sipmask,strlen(sipmask))==0)
		{ 
			sipmaskLen=0;
			sipno=0;
		}
	else
		{						
		  Val=ip_address_format2ulong((char**)&sipmask,&sipno,&sipmaskLen); 
		  VALUE_IP_MASK_CHECK(sipmaskLen);
		  if(Val==CMD_FAILURE)  return CMD_FAILURE;
	}
	//policer
	if(actionType==0){	
		if(strcmp(policerID,"0")!=0){
			policer = 1;
			ret = dcli_str2ulongWS((char*)policerID,&policerId);
         	if (NPD_FAIL == ret) {
         		ShowAlert(search(lcontrol,"illegal_input"));
         		return CMD_FAILURE;
         	}
			//policerId = policerId-1;
		}		
	}
	else if(actionType==1){
		policer =0;
		policerId =0;
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
	   
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{
			if (NPD_DBUS_SUCCESS == op_ret ) {
				ShowAlert(search(lcontrol,"acl_add_suc"));
			}		
			else if(ACL_GLOBAL_EXISTED == op_ret)
				ShowAlert(search(lcontrol,"acl_existed"));
			
			else if(ACL_EXT_NO_SPACE==op_ret)
			{
				sprintf(alert,"%s %d %s",search(lcontrol,"acl_no_space_alert_one"),(ruleIndex-511),search(lcontrol,"acl_no_space_alert_two"));
				ShowAlert(alert);
			}
		        //vty_out(vty,"%% because the extended should take up twice spaces than stdard acl,but %d has been set extended!\n",(ruleIndex-511));
	
			else if(ACL_SAME_FIELD == op_ret)
			{ShowAlert(search(lcontrol,"same_field_not_set"));}
				//vty_out(vty,"%% identical fields of packet can not set again\n");	
            else if(ACL_POLICER_ID_NOT_SET == op_ret)
            {}
				//vty_out(vty,"%% Policer %d not existed!\n",policerId);
			else
			{}
				//vty_out(vty,"%% set fail!\n");
		
	} 
	else 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	free(alert);
	release(lpublic);
	release(lcontrol);
	return CMD_SUCCESS;
}

int addrule_PermitOrDeny_TcpOrUdp(int mode_type,char * Index,char * action,char * protocol,char * dipmask,char * dPort,char * sipmask,char * sPort,char * policerID)
{
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lcontrol;	/*解析help.txt文件的链表头*/
	lpublic=get_chain_head("../htdocs/text/public.txt");
	lcontrol=get_chain_head("../htdocs/text/control.txt");

		DBusMessage 	*query = NULL, *reply = NULL;
	DBusError 		err = { 0 };
	unsigned int 	ruleIndex;
	unsigned long 	dipno, sipno,sipmaskLen,dipmaskLen;
    unsigned long   srcport,dstport;
    unsigned long    actionType,packetType=0;
	unsigned int     ruleType;
	unsigned long	op_ret;
	unsigned int 	policer=0,policerId=0;
	int Val = 0;
	char * alert=(char *)malloc(1024);
	memset(alert,0,1024);
	
	
	INDEX_LENTH_CHECK_ACL((char*)Index,4);
	INDEX_LENTH_CHECK_ACL((char*)dPort,5);
	INDEX_LENTH_CHECK_ACL((char*)sPort,5);
	
	int ret = 0;
	ret = dcli_str2ulongWS((char*)Index,&ruleIndex);
	if (NPD_FAIL == ret) {
		ShowAlert(search(lcontrol,"acl_illegal_input"));
		return CMD_FAILURE;
	}
	ruleIndex = ruleIndex-1;

	if(mode_type==0)//standard
	{
		ruleType = STANDARD_ACL_RULE; 
	}
	else
	{
		ruleType = EXTENDED_ACL_RULE;
	}
	
   if(strncmp("permit",action,strlen(action))==0) { actionType=0;}
    else if(strncmp("deny",action,strlen(action))==0) {actionType=1;}      
   

	if(strncmp("udp",protocol,strlen(protocol))==0)
        packetType=1;
    else if(strncmp("tcp",protocol,strlen(protocol))==0)
        packetType=2;
    
    if(strncmp("any",dipmask,strlen(dipmask))==0)
    	{
			dipmaskLen=0;
		  	dipno=0;
        }
	else{
		Val=ip_address_format2ulong((char**)&dipmask,&dipno,&dipmaskLen);
		VALUE_IP_MASK_CHECK(dipmaskLen);
		if(Val==CMD_FAILURE)  return CMD_FAILURE;
	}


	if(strncmp("any",dPort,strlen(dPort))==0)       
        dstport=ACL_ANY_PORT;
    else
		{
     		ret = dcli_str2ulongWS((char*)dPort,&dstport);
         	if (NPD_FAIL == ret) {
         		ShowAlert(search(lcontrol,"illegal_input"));
         		return CMD_FAILURE;
         	}
    	}
	
	if(strncmp("any",sipmask,strlen(sipmask))==0)
		{ 
			sipmaskLen=0;
		  	sipno=0;
		}
	else{						
		 Val=ip_address_format2ulong((char**)&sipmask,&sipno,&sipmaskLen);
		 VALUE_IP_MASK_CHECK(sipmaskLen);
		 if(Val==CMD_FAILURE)  return CMD_FAILURE;
	}
    if(strncmp("any",sPort,strlen(sPort))==0)       
        srcport=ACL_ANY_PORT;
    else
    	{
     		ret = dcli_str2ulongWS((char*)sPort,&srcport);
         	if (NPD_FAIL == ret) {
         		ShowAlert(search(lcontrol,"illegal_input"));
         		return CMD_FAILURE;
         	}
    	}


    if(strcmp(dPort,"")!=0)
	{
		long temp1=atol(dPort);
		if(temp1<0 || temp1 >65535)
		{
			ShowAlert(search(lcontrol,"port_outrange"));
			return -1;
		}
	}
	if(strcmp(sPort,"")!=0)
	{
		long temp2=atol(sPort);
		if(temp2<0 || temp2 >65535)
		{
			ShowAlert(search(lcontrol,"port_outrange"));
			return -1;
		}
	}
        
		//policer
	if(actionType==0){	
			if(strcmp(policerID,"0")!=0){
				policer = 1;
				ret = dcli_str2ulongWS((char*)policerID,&policerId);
             	if (NPD_FAIL == ret) {
             		ShowAlert(search(lcontrol,"illegal_input"));
             		return CMD_FAILURE;
             	}
				//policerId = policerId-1;
			}		
		}
		else if(actionType==1){
			policer =0;
			policerId =0;
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
  
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{
			if (NPD_DBUS_SUCCESS == op_ret ) 
			{
				ShowAlert(search(lcontrol,"acl_add_suc"));
			}
			else if(ACL_GLOBAL_EXISTED == op_ret)
				ShowAlert(search(lcontrol,"acl_existed"));
			
			else if(ACL_EXT_NO_SPACE==op_ret)
			{
				sprintf(alert,"%s %d %s",search(lcontrol,"acl_no_space_alert_one"),(ruleIndex-511),search(lcontrol,"acl_no_space_alert_two"));
				ShowAlert(alert);
			}
		          //vty_out(vty,"%% because the extended should take up twice spaces than stdard acl,but %d has been set extended!\n",(ruleIndex-511));
	
			else if(ACL_SAME_FIELD == op_ret)
			{ShowAlert(search(lcontrol,"same_field_not_set"));}
				//vty_out(vty,"%% identical fields of packet can not set again\n");	
			else if(ACL_POLICER_ID_NOT_SET == op_ret)
			{}
					//vty_out(vty,"%% Policer %d not existed!\n",policerId);
			else 
			{}
				//vty_out(vty,"%% set fail!\n");
		
	} 
	else 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	free(alert);
	release(lpublic);
	release(lcontrol);
	return CMD_SUCCESS;
}

int addrule_MirrorOrRedir_arp(int mode_type,char * Index,char * action,char * sPortNo,char * smac,char * vlanid,char * slotport,char * policerID)
{
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lcontrol;	/*解析help.txt文件的链表头*/
	lpublic=get_chain_head("../htdocs/text/public.txt");
	lcontrol=get_chain_head("../htdocs/text/control.txt");

	DBusMessage 	*query = NULL, *reply = NULL;
  	DBusError		err = { 0 };
	ETHERADDR		smacAddr,maskMac;
	unsigned char   slot_no=ACL_ANY_PORT_CHAR,port_no=ACL_ANY_PORT_CHAR;
	unsigned char   mslot=0,mport=0;
	unsigned int	ruleIndex; 
	unsigned int	ruleType,actionType,op_ret,vlanId=0,eth_ret;
	char *			str[20]={"ff:ff:ff:ff:ff:ff"};
	char *			strcp;
	unsigned int 	policer=0,policerId=0;
	char * alert=(char *)malloc(1024);
	memset(alert,0,1024);
	

	INDEX_LENTH_CHECK_ACL((char*)Index,4);
	INDEX_LENTH_CHECK_ACL((char*)vlanid,4);
	
	int ret = 0;
	ret = dcli_str2ulongWS((char*)Index,&ruleIndex);
	if (NPD_FAIL == ret) {
		ShowAlert(search(lcontrol,"acl_illegal_input"));
		return CMD_FAILURE;
	}
	ruleIndex = ruleIndex-1; 	

	if(mode_type==0)//standard
	{
		ruleType = STANDARD_ACL_RULE; 
	}
	else
	{
		ruleType = EXTENDED_ACL_RULE;
	}
	
	memset(&smacAddr,0,sizeof(ETHERADDR));
	memset(&maskMac,0,sizeof(ETHERADDR));
	
	if(strncmp("mirror",action,strlen(action))==0)
        actionType=3;
    else if(strncmp("redirect",action,strlen(action))==0)
        actionType=4;
    
   /*mirror or redirect port*/
   ret = parse_slotport_no((char *)sPortNo,&mslot,&mport);

	if (NPD_FAIL == ret) {
		ShowAlert(search(lcontrol,"illegal_slot_pot_input"));
    	//vty_out(vty,"%% Unknow portno format.\n");
		return CMD_SUCCESS;
	}
	
	/*fetch the 1st param : SMAC addr*/
	if(strncmp("any",smac,strlen(smac))!=0){
		op_ret = parse_mac_addr((char *)smac,&smacAddr);
		if (NPD_FAIL == op_ret) {
	    	ShowAlert(search(lcontrol,"unkown_mac_format"));
			return CMD_SUCCESS;
		}
	
		/*convert mac_mask to standard format*/
		strcp=(char *)malloc(50*sizeof(char));
		memset(strcp,'\0',50);
	    strcpy(strcp,str[0]);
		
		op_ret = parse_mac_addr(strcp,&maskMac);
		if (NPD_FAIL == op_ret) {
	    	ShowAlert(search(lcontrol,"unkown_mac_format"));
			return CMD_SUCCESS;
		}
	}
    /*vlan id*/
	if(strncmp("any",vlanid,strlen(vlanid))!=0){
     	ret = dcli_str2ulongWS((char*)vlanid,&vlanId);
     	if (NPD_FAIL == ret) {
     		ShowAlert(search(lcontrol,"illegal_input"));
     		return CMD_FAILURE;
	}
	}
	/*slot/port*/
	if(strncmp("any",slotport,strlen(slotport))!=0){
		ret = parse_slotport_no((char *)slotport,&slot_no,&port_no);

		if (NPD_FAIL == ret) {
			ShowAlert(search(lcontrol,"illegal_slot_pot_input"));
	    	//vty_out(vty,"%% Unknow portno format.\n");
			return CMD_SUCCESS;
		}
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
  
   
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_UINT32,&eth_ret,		
		DBUS_TYPE_INVALID))
	{

			if(NPD_DBUS_SUCCESS != eth_ret){
			}
			else{
				if (NPD_DBUS_SUCCESS == op_ret ) 
				{
					ShowAlert(search(lcontrol,"acl_add_suc"));
				}
				else if(ACL_GLOBAL_EXISTED == op_ret)
					ShowAlert(search(lcontrol,"acl_existed"));
				
				else if(ACL_EXT_NO_SPACE==op_ret)
				{
					sprintf(alert,"%s %d %s",search(lcontrol,"acl_no_space_alert_one"),(ruleIndex-511),search(lcontrol,"acl_no_space_alert_two"));
					ShowAlert(alert);
				}
				       //vty_out(vty,"%% because the extended should take up twice spaces than stdard acl,but %d has been set extended!\n",(ruleIndex-511));
	
				else if(ACL_SAME_FIELD == op_ret)
				{ShowAlert(search(lcontrol,"same_field_not_set"));}
					//vty_out(vty,"%% identical fields of packet can not set again\n");	
                else if(ACL_POLICER_ID_NOT_SET == op_ret)
                {}
					//vty_out(vty,"%% Policer %d not existed!\n",policerId);
				else 
				{}
					//vty_out(vty,"%% set fail!\n");
			}			
	} 
	else 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	free(alert);
	release(lpublic);
	release(lcontrol);
	return CMD_SUCCESS;
}

int addrule_MirrorOrRedir_ip(int mode_type,char * Index,char * action,char * sPortNo,char * dipmask,char * sipmask,char * policerID)
{
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lcontrol;	/*解析help.txt文件的链表头*/
	lpublic=get_chain_head("../htdocs/text/public.txt");
	lcontrol=get_chain_head("../htdocs/text/control.txt");


	DBusMessage 	*query = NULL, *reply = NULL;
	DBusError		err = { 0 };
	unsigned int	ruleIndex;
	unsigned long	dipno, sipno,sipmaskLen,dipmaskLen;
	unsigned  int	ruleType,actionType,temp;
	unsigned long	op_ret;
	unsigned char   slot_no,port_no;
	unsigned int	policer=0,policerId=0;
	int Val=0;
	char * alert=(char *)malloc(1024);
	memset(alert,0,1024);


	INDEX_LENTH_CHECK_ACL((char*)Index,4);
	
	int ret = 0;
	ret = dcli_str2ulongWS((char*)Index,&ruleIndex);
	if (NPD_FAIL == ret) {
		ShowAlert(search(lcontrol,"acl_illegal_input"));
		return CMD_FAILURE;
	}
	ruleIndex = ruleIndex-1;
		
	if(mode_type==0)//standard
	{
		ruleType = STANDARD_ACL_RULE; 
	}
	else
	{
		ruleType = EXTENDED_ACL_RULE;
	}

	if(strncmp("mirror",action,strlen(action))==0)
        actionType=3;
    else if(strncmp("redirect",action,strlen(action))==0)
        actionType=4;

	temp = parse_slotport_no((char*)sPortNo,&slot_no,&port_no);
	
	if (NPD_FAIL == temp) {
		ShowAlert(search(lcontrol,"illegal_slot_pot_input"));
		return CMD_FAILURE;
	}
	

	if(strncmp("any",dipmask,strlen(dipmask))==0)
		{
			dipmaskLen=0;
			dipno=0;
		}
	else
		{	
			Val=ip_address_format2ulong((char**)&dipmask,&dipno,&dipmaskLen);
			VALUE_IP_MASK_CHECK(dipmaskLen);
			if(Val==CMD_FAILURE)  return CMD_FAILURE;
		}

	
	if(strncmp("any",sipmask,strlen(sipmask))==0)
		{ 
			sipmaskLen=0;
			sipno=0;
		}
	else
		{						
		 Val = ip_address_format2ulong((char**)&sipmask,&sipno,&sipmaskLen); 
		 VALUE_IP_MASK_CHECK(sipmaskLen);
		if(Val==CMD_FAILURE)  return CMD_FAILURE;
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
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{
			if (NPD_DBUS_SUCCESS == op_ret ) 
			{
				ShowAlert(search(lcontrol,"acl_add_suc"));
			}	
			else if(NPD_DBUS_ERROR_NO_SUCH_PORT==op_ret){
				ShowAlert(search(lcontrol,"acl_existed"));
			}
			else if(ACL_GLOBAL_EXISTED == op_ret)
			{
				ShowAlert(search(lcontrol,"acl_existed"));
			}
				//vty_out(vty,"%% Access-list %d existed!\n",(ruleIndex+1));
			
			else if(ACL_EXT_NO_SPACE==op_ret)
			{
				sprintf(alert,"%s %d %s",search(lcontrol,"acl_no_space_alert_one"),(ruleIndex-511),search(lcontrol,"acl_no_space_alert_two"));
				ShowAlert(alert);
			}
		        //vty_out(vty,"%% because the extended should take up twice spaces than stdard acl,but %d has been set extended!\n",(ruleIndex-511));
	
			else if(ACL_SAME_FIELD == op_ret)
			{ShowAlert(search(lcontrol,"same_field_not_set"));}
				//vty_out(vty,"%% identical fields of packet can not set again\n");	
			else if(ACL_POLICER_ID_NOT_SET == op_ret)
			{}
					//vty_out(vty,"%% Policer %d not existed!\n",policerId);
			else 
			{}
				//vty_out(vty,"%% set fail!\n");
		
	} 
	else 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	free(alert);
	release(lpublic);
	release(lcontrol);
	return CMD_SUCCESS;
}


int addrule_MirrorOrRedir_TcpOrUdp(int mode_type,char * Index,char * action,char * sPortNo,char * protocol,char * dipmask,char * dPort,char * sipmask,char * sPort,char * policerID)
{
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lcontrol;	/*解析help.txt文件的链表头*/
	lpublic=get_chain_head("../htdocs/text/public.txt");
	lcontrol=get_chain_head("../htdocs/text/control.txt");

	DBusMessage 	*query = NULL, *reply = NULL;
	DBusError 		err = { 0 };
	unsigned int 	ruleIndex;
	unsigned long 	dipno, sipno,sipmaskLen,dipmaskLen;
    unsigned long   srcport,dstport;
    unsigned long    actionType,packetType=0;
	unsigned int     ruleType,temp;
	unsigned long	op_ret;
	unsigned char   slot_no,port_no;	
	unsigned int	policer=0,policerId=0;
	int Val =0;
	char * alert=(char *)malloc(1024);
	memset(alert,0,1024);

	INDEX_LENTH_CHECK_ACL((char*)Index,4);
	INDEX_LENTH_CHECK_ACL((char*)Index,5);
	INDEX_LENTH_CHECK_ACL((char*)sPort,5);
	
	int ret = 0;
	ret = dcli_str2ulongWS((char*)Index,&ruleIndex);
	if (NPD_FAIL == ret) {
		ShowAlert(search(lcontrol,"acl_illegal_input"));
		return CMD_FAILURE;
	}
	ruleIndex = ruleIndex-1;
				
	if(mode_type==0)//standard
	{
		ruleType = STANDARD_ACL_RULE; 
	}
	else
	{
		ruleType = EXTENDED_ACL_RULE;
	}
	
   if(strncmp("mirror",action,strlen(action))==0)
        actionType=3;
    else if(strncmp("redirect",action,strlen(action))==0)
        actionType=4;

	temp = parse_slotport_no((char*)sPortNo,&slot_no,&port_no);
	
	if (NPD_FAIL == temp) {
    	ShowAlert(search(lcontrol,"illegal_slot_pot_input"));
		return CMD_FAILURE;
	}
	
	if(strncmp("udp",protocol,strlen(protocol))==0)
        packetType=1;
    else if(strncmp("tcp",protocol,strlen(protocol))==0)
        packetType=2;

	if(strncmp("any",dipmask,strlen(dipmask))==0)
    	{
			dipmaskLen=0;
		  	dipno=0;
    }
	else{	
			Val=ip_address_format2ulong((char**)&dipmask,&dipno,&dipmaskLen);
			VALUE_IP_MASK_CHECK(dipmaskLen);
			if(Val==CMD_FAILURE)  return CMD_FAILURE;
	}
	

	
    if(strncmp("any",dPort,strlen(dPort))==0)       
        dstport=ACL_ANY_PORT;
    else
		{
        	ret = dcli_str2ulongWS((char*)dPort,&dstport);
        	if (NPD_FAIL == ret) {
        		ShowAlert(search(lcontrol,"illegal_input"));
        		return CMD_FAILURE;
        	}
		}
      
	if(strncmp("any",sipmask,strlen(sipmask))==0)
		{ 
			sipmaskLen=0;
		  	sipno=0;
		}
	else
		{						
		  Val=ip_address_format2ulong((char**)&sipmask,&sipno,&sipmaskLen);	
		  VALUE_IP_MASK_CHECK(sipmaskLen);
			if(Val==CMD_FAILURE)  return CMD_FAILURE;
	}
   

    if(strncmp("any",sPort,strlen(sPort))==0)       
        srcport=ACL_ANY_PORT;
    else
	{
    	ret = dcli_str2ulongWS((char*)sPort,&srcport);
    	if (NPD_FAIL == ret) {
    		ShowAlert(search(lcontrol,"illegal_input"));
    		return CMD_FAILURE;
    	}
	}


    if(strcmp(dPort,"")!=0)
	{
		long temp1=atol(dPort);
		if(temp1<0 || temp1 >65535)
		{
			ShowAlert(search(lcontrol,"port_outrange"));
			return -1;
		}
	}
	if(strcmp(sPort,"")!=0)
	{
		long temp2=atol(sPort);
		if(temp2<0 || temp2 >65535)
		{
			ShowAlert(search(lcontrol,"port_outrange"));
			return -1;
		}
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

	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{
			if (NPD_DBUS_SUCCESS == op_ret ) 
			{
				ShowAlert(search(lcontrol,"acl_add_suc"));
			}	
			else if(NPD_DBUS_ERROR_NO_SUCH_PORT==op_ret){
				ShowAlert(search(lcontrol,"acl_existed"));
			}
			else if(ACL_GLOBAL_EXISTED == op_ret)
			{
				ShowAlert(search(lcontrol,"acl_existed"));
			}
				//vty_out(vty,"%% Access-list %d existed!\n",(ruleIndex+1));
			
			else if(ACL_EXT_NO_SPACE==op_ret)
			{
				sprintf(alert,"%s %d %s",search(lcontrol,"acl_no_space_alert_one"),(ruleIndex-511),search(lcontrol,"acl_no_space_alert_two"));
				ShowAlert(alert);
			}
		        //vty_out(vty,"%%  because the extended should take up twice spaces than stdard acl,but %d has been set extended!\n",(ruleIndex-511));
	
			else if(ACL_SAME_FIELD == op_ret)
			{ShowAlert(search(lcontrol,"same_field_not_set"));}
				//vty_out(vty,"%%  identical fields of packet can not set again\n");	
			else if(ACL_POLICER_ID_NOT_SET == op_ret)
			{}
					//vty_out(vty,"%% Policer %d not existed!\n",policerId);
			else
			{}
				//vty_out(vty,"%%  set fail!\n");	
		
	} 
	else 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	free(alert);
	release(lpublic);
	release(lcontrol);
	return CMD_SUCCESS;
}

int addrule_MirrorOrRedir_icmp(int mode_type,char * Index,char * action,char * sPortNo,char * dipmask,char * sipmask,char * type,char * code,char * policerID)
{
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lcontrol;	/*解析help.txt文件的链表头*/
	lpublic=get_chain_head("../htdocs/text/public.txt");
	lcontrol=get_chain_head("../htdocs/text/control.txt");

	DBusMessage 	*query = NULL, *reply = NULL;
	DBusError		err = { 0 };
	unsigned int	ruleIndex;
	unsigned long	 dipno, sipno,sipmaskLen,dipmaskLen;
	unsigned int 	ruleType,temp,actionType;
	unsigned long	op_ret;
	unsigned char   slot_no,port_no;
	unsigned char    typeno = 0,codeno = 0,acl_any = 0;
	unsigned int     tmp1=0,tmp2=0;
	unsigned int	policer=0,policerId=0;
	int Val=0;
	char * alert = (char *)malloc(1024);
	memset(alert,0,1024);

	INDEX_LENTH_CHECK_ACL((char*)Index,4);
	INDEX_LENTH_CHECK_ACL((char*)type,3);
	INDEX_LENTH_CHECK_ACL((char*)code,3);
	if(strcmp(type,"")==0)
	{}
	else
	{
		int temp=atoi(type);
		if(temp<0 || temp>255)
		{
			ShowAlert(search(lcontrol,"illegal_input"));
			return 1;
		}
	}
	if(strcmp(code,"")==0)
	{}
	else
	{
		int temp=atoi(code);
		if(temp<0 || temp>255)
		{
			ShowAlert(search(lcontrol,"illegal_input"));
			return 1;
		}
	}
	
	int ret = 0;
	ret = dcli_str2ulongWS((char*)Index,&ruleIndex);
	if (NPD_FAIL == ret) {
		ShowAlert(search(lcontrol,"acl_illegal_input"));
		return CMD_FAILURE;
	}
	ruleIndex = ruleIndex-1;
		
	if(mode_type==0)//standard
	{
		ruleType = STANDARD_ACL_RULE; 
	}
	else
	{
		ruleType = EXTENDED_ACL_RULE;
	}

	if(strncmp("mirror",action,strlen(action))==0)
        actionType=3;
    else if(strncmp("redirect",action,strlen(action))==0)
        actionType=4;
	temp = parse_slotport_no((char*)sPortNo,&slot_no,&port_no);
	if (NPD_FAIL == temp) {
		ShowAlert(search(lcontrol,"illegal_slot_pot_input"));
		return CMD_FAILURE;
	}

	if(strncmp("any",dipmask,strlen(dipmask))==0)
	{
		dipmaskLen=0;
		dipno=0;
	}
	else
	{	
		Val=ip_address_format2ulong((char**)&dipmask,&dipno,&dipmaskLen);
		VALUE_IP_MASK_CHECK(dipmaskLen);
		if(Val==CMD_FAILURE)  return CMD_FAILURE;
	}

	
	if(strncmp("any",sipmask,strlen(sipmask))==0)
	{ 
		sipmaskLen=0;
		sipno=0;
	}
	else
	{						
	  Val=ip_address_format2ulong((char**)&sipmask,&sipno,&sipmaskLen);
	  VALUE_IP_MASK_CHECK(sipmaskLen); 
	  if(Val==CMD_FAILURE)	return CMD_FAILURE;
	}

	
	if(strncmp("",type,strlen(type))==0)	
	{
		typeno=0;
		acl_any |= 1;
	}
	else
	{
        	ret = dcli_str2ulongWS((char*)type,&tmp1);
        	if (NPD_FAIL == ret) {
        		ShowAlert(search(lcontrol,"illegal_input"));
        		return CMD_FAILURE;
        	}
		typeno = tmp1;
	}
	  
	if(strncmp("",code,strlen(code))==0)	   
	{
		codeno=0;
		acl_any |=2;
	}
	else
	{
        	ret = dcli_str2ulongWS((char*)code,&tmp2);
        	if (NPD_FAIL == ret) {
        		ShowAlert(search(lcontrol,"illegal_input"));
        		return CMD_FAILURE;
        	}
		codeno = tmp2;
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
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{
			if (NPD_DBUS_SUCCESS == op_ret ) 
			{
				ShowAlert(search(lcontrol,"acl_add_suc"));
			}	
			else if(NPD_DBUS_ERROR_NO_SUCH_GROUP==op_ret){
				ShowAlert(search(lcontrol,"acl_existed"));
			}
			else if(ACL_GLOBAL_EXISTED == op_ret)
			{
				ShowAlert(search(lcontrol,"acl_existed"));
			}
				//vty_out(vty,"%%   Access-list %d existed!\n",(ruleIndex+1));
			
			else if(ACL_EXT_NO_SPACE==op_ret)
			{
				sprintf(alert,"%s %d %s",search(lcontrol,"acl_no_space_alert_one"),(ruleIndex-511),search(lcontrol,"acl_no_space_alert_two"));
				ShowAlert(alert);
			}
		        //vty_out(vty,"%%   because the extended should take up twice spaces than stdard acl,but %d has been set extended!\n",(ruleIndex-511));
	
			else if(ACL_SAME_FIELD == op_ret)
			{ShowAlert(search(lcontrol,"same_field_not_set"));}
				//vty_out(vty,"%%   identical fields of packet can not set again\n");	
			else if(ACL_POLICER_ID_NOT_SET == op_ret)
			{}
				//vty_out(vty,"%% Policer %d not existed!\n",policerId);
			else 
			{}
				//vty_out(vty,"%%   set fail!\n");
	} 
	else 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	free(alert);
	release(lpublic);
	release(lcontrol);
	return CMD_SUCCESS;
}

int addrule_MirrorOrRedir_ethernet(int mode_type,char * Index,char * action,char * sPortNo,char * dmac,char * smac,char * policerID)
{
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lcontrol;	/*解析help.txt文件的链表头*/
	lpublic=get_chain_head("../htdocs/text/public.txt");
	lcontrol=get_chain_head("../htdocs/text/control.txt");
	
	DBusMessage 	*query = NULL, *reply = NULL;
	DBusError		err = { 0 };
	ETHERADDR		dmacAddr,smacAddr;
	unsigned int	ruleIndex,actionType; 
	unsigned int	ruleType,op_ret,temp;
	unsigned char   slot_no=0,port_no=0;
	unsigned int 	policer=0,policerId=0;
	char * alert=(char *)malloc(1024);
	memset(alert,0,1024);

	INDEX_LENTH_CHECK_ACL((char*)Index,4);
	
	int ret = 0;
	ret = dcli_str2ulongWS((char*)Index,&ruleIndex);
	if (NPD_FAIL == ret) {
		ShowAlert(search(lcontrol,"acl_illegal_input"));
		return CMD_FAILURE;
	}
	ruleIndex = ruleIndex-1; 
	
	if(mode_type==0)//standard
	{
		ruleType = STANDARD_ACL_RULE; 
	}
	else
	{
		ruleType = EXTENDED_ACL_RULE;
	} 

	
	if(strncmp("mirror",action,strlen(action))==0)
        actionType=3;
    else if(strncmp("redirect",action,strlen(action))==0)
        actionType=4;

	temp = parse_slotport_no((char*)sPortNo,&slot_no,&port_no);
	
	if (NPD_FAIL == temp) {
		ShowAlert(search(lcontrol,"illegal_slot_pot_input"));
		return CMD_FAILURE;
	}
	memset(&smacAddr,0,sizeof(ETHERADDR));
	memset(&dmacAddr,0,sizeof(ETHERADDR));
	//memset(&maskMac,0,sizeof(ETHERADDR));

	/*fetch the 1st param : DMAC addr*/
	if(strncmp("any",dmac,strlen(dmac))!=0){
		op_ret = parse_mac_addr((char *)dmac,&dmacAddr);
		if (NPD_FAIL == op_ret) {
			ShowAlert(search(lcontrol,"unkown_mac_format"));
			return CMD_SUCCESS;
		}
	}
	/*fetch the 1st param : SMAC addr*/
	if(strncmp("any",smac,strlen(smac))!=0){
		op_ret = parse_mac_addr((char *)smac,&smacAddr);
		if (NPD_FAIL == op_ret) {
			ShowAlert(search(lcontrol,"unkown_mac_format"));
			return CMD_SUCCESS;
		}
	}
	//policer

	/*query*/
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_ACL_OBJPATH,NPD_DBUS_ACL_INTERFACE, NPD_DBUS_METHOD_CONFIG_ACL_RULE_MIRROR_OR_REDIRECT_MAC);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&ruleIndex,	
							 DBUS_TYPE_UINT32,&ruleType,
							 DBUS_TYPE_UINT32,&actionType,
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
 
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{
			if (NPD_DBUS_SUCCESS == op_ret ) 
			{
				ShowAlert(search(lcontrol,"acl_add_suc"));
			}
			else if(NPD_DBUS_ERROR_NO_SUCH_PORT==op_ret)
				ShowAlert(search(lcontrol,"acl_existed"));
		    else if(ACL_GLOBAL_EXISTED == op_ret)
		    {
		    	ShowAlert(search(lcontrol,"acl_existed"));
		    }
				//vty_out(vty,"%%   Access-list %d existed!\n",(ruleIndex+1));
			else if(ACL_EXT_NO_SPACE==op_ret)
			{
				sprintf(alert,"%s %d %s",search(lcontrol,"acl_no_space_alert_one"),(ruleIndex-511),search(lcontrol,"acl_no_space_alert_two"));
				ShowAlert(alert);
			}
		        //vty_out(vty,"%%   because the extended should take up twice spaces than stdard acl,but %d has been set extended!\n",(ruleIndex-511));
	
			else if(ACL_SAME_FIELD == op_ret)
			{ShowAlert(search(lcontrol,"same_field_not_set"));}
				//vty_out(vty,"%%   identical fields of packet can not set again\n");	
			else if(ACL_POLICER_ID_NOT_SET == op_ret)
			{}
				//vty_out(vty,"%% Policer %d not existed!\n",policerId);
			else
			{}
				//vty_out(vty,"%%   set fail!\n");
	} 
	else 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	free(alert);
	release(lpublic);
	release(lcontrol);
	return CMD_SUCCESS;
}

int addrule_extend(char * Index,char * action,char * protocol,char * dipmask,char * dPort,char * sipmask,char * sPort,char * dmac,char * smac,char * vlanid,char * slotport,char * policerID)
{
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lcontrol;	/*解析help.txt文件的链表头*/
	lpublic=get_chain_head("../htdocs/text/public.txt");
	lcontrol=get_chain_head("../htdocs/text/control.txt");

	DBusMessage 	*query = NULL, *reply = NULL;
	DBusError		err = { 0 };
	ETHERADDR		dmacAddr,smacAddr;
	unsigned long	dipno=0,sipno=0,dipmaskLen=0,sipmaskLen=0;
	unsigned int	actionType,packetType,srcport=0,dstport=0,vlanId=0;
	unsigned char   dataslot = ACL_ANY_PORT_CHAR,dataport = ACL_ANY_PORT_CHAR;
	unsigned int     ruleType,op_ret,ret;
	unsigned int	policer=0,policerId=0,ruleIndex=0;
	int Val=0;
	char * alert=(char *)malloc(1024);
	memset(alert,0,1024);
	
	INDEX_LENTH_CHECK_ACL((char*)Index,4);
	INDEX_LENTH_CHECK_ACL((char*)dPort,5);
	INDEX_LENTH_CHECK_ACL((char*)sPort,5);
	INDEX_LENTH_CHECK_ACL((char*)slotport,4);
	
	ret = dcli_str2ulongWS((char*)Index,&ruleIndex);
	if (NPD_FAIL == ret) {
		ShowAlert(search(lcontrol,"acl_illegal_input"));
		return CMD_FAILURE;
	}
	ruleIndex = ruleIndex-1;

	ruleType = EXTENDED_ACL_RULE;
	
	if(strncmp("permit",action,strlen(action))==0)
		actionType=0;
	else if(strncmp("deny",action,strlen(action))==0)
		actionType=1;
	else if(strncmp("trap_to_cpu",action,strlen(action))==0)
		actionType=2;
	
	if(strncmp("udp",protocol,strlen(protocol))==0)
		packetType=1;
	else if(strncmp("tcp",protocol,strlen(protocol))==0)
		packetType=2;
	
	if(strncmp("any",dipmask,strlen(dipmask))==0)
	{
		dipmaskLen=0;
		dipno=0;
	}
	else{
		Val=ip_address_format2ulong((char**)&dipmask,&dipno,&dipmaskLen);
		VALUE_IP_MASK_CHECK(dipmaskLen);
		if(Val==CMD_FAILURE)  return CMD_FAILURE;
	}


	if(strncmp("any",dPort,strlen(dPort))==0)	   
		dstport=ACL_ANY_PORT;
	else
		{
         	ret = dcli_str2ulongWS((char*)dPort,&dstport);
         	if (NPD_FAIL == ret) {
         		ShowAlert(search(lcontrol,"illegal_input"));
         		return CMD_FAILURE;
         	}
		}
	
	if(strncmp("any",sipmask,strlen(sipmask))==0)
	{ 
		sipmaskLen=0;
		sipno=0;
	}
	else{						
		Val=ip_address_format2ulong((char**)&sipmask,&sipno,&sipmaskLen);
		VALUE_IP_MASK_CHECK(sipmaskLen);
	    if(Val==CMD_FAILURE)  return CMD_FAILURE;
	}

	
	if(strncmp("any",sPort,strlen(sPort))==0)	   
		srcport=ACL_ANY_PORT;
	else
	{
     	ret = dcli_str2ulongWS((char*)sPort,&srcport);
     	if (NPD_FAIL == ret) {
     		ShowAlert(search(lcontrol,"illegal_input"));
     		return CMD_FAILURE;
     	}
	}

	if(strcmp(dPort,"")!=0)
	{
		long temp1=atol(dPort);		
		if(temp1<0 || temp1 >65535)
		{

			ShowAlert(search(lcontrol,"port_outrange"));
			return -1;
		}
	}
	if(strcmp(sPort,"")!=0)
	{
		long temp2=atol(sPort);
		if(temp2<0 || temp2 >65535)
		{
			ShowAlert(search(lcontrol,"port_outrange"));
			return -1;
		}
	}
	
		
	memset(&smacAddr,0,sizeof(ETHERADDR));
	memset(&dmacAddr,0,sizeof(ETHERADDR));
	//memset(&maskMac,0,sizeof(ETHERADDR));
	/*dmac*/
	if(strncmp("any",dmac,strlen(dmac))!=0){
		op_ret = parse_mac_addr((char *)dmac,&dmacAddr);
		if (NPD_FAIL == op_ret) {
			ShowAlert(search(lcontrol,"unkown_mac_format"));
			return CMD_SUCCESS;
		}
	}
	/*smac*/
	if(strncmp("any",smac,strlen(smac))!=0){
		op_ret = parse_mac_addr((char *)smac,&smacAddr);
		if (NPD_FAIL == op_ret) {
			ShowAlert(search(lcontrol,"unkown_mac_format"));
			return CMD_SUCCESS;
		}
	}
		
	if(strncmp("any",vlanid,strlen(vlanid))!=0)
	{
		dcli_str2ulong((char*)vlanid,&vlanId);
	}
	
	if(strncmp("any",slotport,strlen(slotport))!=0)
	{
		/*dataslot/dataport*/
		ret = parse_slotport_no((char *)slotport,&dataslot,&dataport);

		if (NPD_FAIL == ret) {
			ShowAlert(search(lcontrol,"illegal_slot_pot_input"));
			return CMD_SUCCESS;
		}
	}
	//policer
	if(actionType==0){	
		if(strcmp(policerID,"0")!=0){
			policer = 1;
			 dcli_str2ulong(policerID,&policerId);
			//policerId = policerId-1;
		}		
	}
	else if(actionType==1){
		policer =0;
		policerId =0;
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
  
   
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{
			if (NPD_DBUS_SUCCESS == op_ret ) 
			{
				ShowAlert(search(lcontrol,"acl_add_suc"));
			}
			else if(NPD_DBUS_ERROR_NO_SUCH_PORT ==op_ret){
				ShowAlert(search(lcontrol,"acl_existed"));
			}
			else if(ACL_GLOBAL_EXISTED == op_ret)
			{
				ShowAlert(search(lcontrol,"acl_existed"));
			}
				//vty_out(vty,"%%  Access-list %d existed!\n",(ruleIndex+1));
			else if(ACL_EXT_NO_SPACE==op_ret)
			{
				sprintf(alert,"%s %d %s",search(lcontrol,"acl_no_space_alert_one"),(ruleIndex-511),search(lcontrol,"acl_no_space_alert_two"));
				ShowAlert(alert);
			}
			     //vty_out(vty,"%%  because the extended rule should take up twice spaces than standard acl,
			else if(ACL_SAME_FIELD == op_ret)
			{ShowAlert(search(lcontrol,"same_field_not_set"));}
				//vty_out(vty,"%%  identical fields of packet can not set again\n");	
			else if(ACL_POLICER_ID_NOT_SET == op_ret)
			{}
				//vty_out(vty,"%% Policer %d not existed!\n",policerId);
			else 
			{}
				
				//vty_out(vty,"%%  set fail!\n");
		
	} 
	else 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	release(lpublic);
	release(lcontrol);
	free(alert);
	return CMD_SUCCESS;
}

int addrule_MirrorOrRedir_extend(char * Index,char * action,char * sPortNo,char * protocol,char * dipmask,char * dPort,char * sipmask,char * sPort,char * dmac,char * smac,char * vlanid,char * slotport,char * policerID)
{
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lcontrol;	/*解析help.txt文件的链表头*/
	lpublic=get_chain_head("../htdocs/text/public.txt");
	lcontrol=get_chain_head("../htdocs/text/control.txt");

	DBusMessage 	*query = NULL, *reply = NULL;
	DBusError		err = { 0 };
	ETHERADDR		dmacAddr,smacAddr;
	unsigned long	dipno=0,sipno=0,dipmaskLen=0,sipmaskLen=0;
	unsigned int	actionType,packetType,srcport=0,dstport=0,vlanId=0;
	unsigned int	 ruleType,op_ret,ret,temp;
	unsigned char    slot_no=0,port_no=0,dataslot=ACL_ANY_PORT_CHAR,dataport=ACL_ANY_PORT_CHAR;
	unsigned int	policer=0,policerId=0,ruleIndex=0;
	int Val=0;
	char * alert=(char *)malloc(1024);
	memset(alert,0,1024);

	INDEX_LENTH_CHECK_ACL((char*)Index,4);
	INDEX_LENTH_CHECK_ACL((char*)dPort,5);
	INDEX_LENTH_CHECK_ACL((char*)sPort,5);
	INDEX_LENTH_CHECK_ACL((char*)vlanid,4);
	ret = dcli_str2ulongWS((char*)Index,&ruleIndex);
	if (NPD_FAIL == ret) {
		ShowAlert(search(lcontrol,"acl_illegal_input"));
		return CMD_FAILURE;
	}
	ruleIndex = ruleIndex-1;

	ruleType = EXTENDED_ACL_RULE;
	
	if(strncmp("mirror",action,strlen(action))==0)
		actionType=3;
	else if(strncmp("redirect",action,strlen(action))==0)
		actionType=4;
	
	temp = parse_slotport_no((char*)sPortNo,&slot_no,&port_no);
	
	if (NPD_FAIL == temp) {
		ShowAlert(search(lcontrol,"illegal_slot_pot_input"));
		return CMD_FAILURE;
	}
	
	if(0==strncmp("udp",protocol,strlen(protocol)))
		packetType=1;
	else if(0==strncmp("tcp",protocol,strlen(protocol)))
		packetType=2;
	
	if(strncmp("any",dipmask,strlen(dipmask))==0)
	{
		dipmaskLen=0;
		dipno=0;
	}
	else{
		Val=ip_address_format2ulong((char**)&dipmask,&dipno,&dipmaskLen);
		VALUE_IP_MASK_CHECK(dipmaskLen);
		if(Val==CMD_FAILURE)  return CMD_FAILURE;
	}


	if(strncmp("any",dPort,strlen(dPort))==0)	   
		dstport=ACL_ANY_PORT;
	else
		{
        	ret = dcli_str2ulongWS((char*)dPort,&dstport);
        	if (NPD_FAIL == ret) {
        		ShowAlert(search(lcontrol,"illegal_input"));
        		return CMD_FAILURE;
        	}
		}


	if(strncmp("any",sipmask,strlen(sipmask))==0)
	{ 
		sipmaskLen=0;
		sipno=0;
	}
	else{						
		 Val=ip_address_format2ulong((char**)&sipmask,&sipno,&sipmaskLen);
		 VALUE_IP_MASK_CHECK(sipmaskLen);
		 if(Val==CMD_FAILURE)  return CMD_FAILURE;
	}

	
	if(strncmp("any",sPort,strlen(sPort))==0)	   
		srcport=ACL_ANY_PORT;
	else
	{
    	ret = dcli_str2ulongWS((char*)sPort,&srcport);
    	if (NPD_FAIL == ret) {
    		ShowAlert(search(lcontrol,"illegal_input"));
    		return CMD_FAILURE;
    	}
	}

	if(strcmp(dPort,"")!=0)
	{
		long temp1=atol(dPort);
		if(temp1<0 || temp1 >65535)
		{
			ShowAlert(search(lcontrol,"port_outrange"));
			return -1;
		}
	}
	if(strcmp(sPort,"")!=0)
	{
		long temp2=atol(sPort);
		if(temp2<0 || temp2 >65535)
		{
			ShowAlert(search(lcontrol,"port_outrange"));
			return -1;
		}
	}
	
		
	memset(&smacAddr,0,sizeof(ETHERADDR));
	memset(&dmacAddr,0,sizeof(ETHERADDR));
	//memset(&maskMac,0,sizeof(ETHERADDR));
	/*dmac*/
	if(strncmp("any",dmac,strlen(dmac))!=0){
		op_ret = parse_mac_addr((char *)dmac,&dmacAddr);
		if (NPD_FAIL == op_ret) {
			ShowAlert(search(lcontrol,"unkown_mac_format"));
			return CMD_SUCCESS;
		}
	}
	/*smac*/
	if(strncmp("any",smac,strlen(smac))!=0){
		op_ret = parse_mac_addr((char *)smac,&smacAddr);
		if (NPD_FAIL == op_ret) {
			ShowAlert(search(lcontrol,"unkown_mac_format"));
			return CMD_SUCCESS;
		}
	}
		
	if(strncmp("any",vlanid,strlen(vlanid))!=0)
		dcli_str2ulong((char*)vlanid,&vlanId);
	/*dataslot/dataport*/
	if(strncmp("any",slotport,strlen(slotport))!=0){
		ret = parse_slotport_no((char *)slotport,&dataslot,&dataport);

		if (NPD_FAIL == ret) {
			ShowAlert(search(lcontrol,"illegal_slot_pot_input"));
			return CMD_SUCCESS;
		}
	}
	//policer
	if(actionType==0){	
		if(strcmp(policerID,"0")!=0){
			policer = 1;
			 dcli_str2ulong(policerID,&policerId);
			//policerId = policerId-1;
		}		
	}
	else if(actionType==1){
		policer =0;
		policerId =0;
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
  
   
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{
			if (NPD_DBUS_SUCCESS == op_ret ) 
			{
				ShowAlert(search(lcontrol,"acl_add_suc"));
			}
			else if(NPD_DBUS_ERROR_NO_SUCH_PORT ==op_ret){
			
				//vty_out(vty,"%%  Error! illegal port index\n");
			}
			else if(ACL_GLOBAL_EXISTED == op_ret)
				ShowAlert(search(lcontrol,"acl_existed"));
			else if(ACL_EXT_NO_SPACE==op_ret)
			{
				sprintf(alert,"%s %d %s",search(lcontrol,"acl_no_space_alert_one"),(ruleIndex-511),search(lcontrol,"acl_no_space_alert_two"));
				ShowAlert(alert);
			}

			else if(ACL_SAME_FIELD == op_ret)
			{ShowAlert(search(lcontrol,"same_field_not_set"));}
				//vty_out(vty,"%%  identical fields of packet can not set again\n");	
			else if(ACL_POLICER_ID_NOT_SET == op_ret)
			{}
				//vty_out(vty,"%% Policer %d not existed!\n",policerId);
			else 
			{}
				//vty_out(vty,"%%  set fail!\n");
		
	} 
	else 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	release(lpublic);
	release(lcontrol);
	free(alert);
	return CMD_SUCCESS;
}

int addrule_ingress_qos(char * ruleindex,char * qosIndex,char * sub_qos,char * source_up,char * source_dscp,char * policerID,struct list * lcontrol)
{
	DBusMessage 	*query = NULL, *reply = NULL;
	DBusError		err = { 0 };
	unsigned int	ruleIndex=0;
	unsigned int    profileIndex=0,up=10,dscp=100;
	unsigned int    op_ret;
	unsigned int    policer=0,policerId=0;
	unsigned int    remark=0;
	unsigned int    ruleType=0;
	//int ret;
	char * alert = (char *)malloc(1024);
	memset(alert,0,1024);
	
	ruleType = EXTENDED_ACL_RULE; 

	dcli_str2ulong((char *)ruleindex,&ruleIndex);
	ruleIndex -=1;
	dcli_str2ulong((char*)qosIndex,&profileIndex);
	 
	if(strncmp("enable",sub_qos,strlen(sub_qos))==0)
		remark = 0;
	else if(strncmp("disable",sub_qos,strlen(sub_qos))==0)
		remark =1 ;

	if((strncmp("none",source_up,strlen(source_up))==0)&&(strncmp("none",source_dscp,strlen(source_dscp))==0))
	{
		//vty_out(vty,"%% sourceUp and sourceDscp cannot be both none,acl set invalid!\n");
		ShowAlert(search(lcontrol,"BOTH_None_invalid"));
		return CMD_WARNING;
	}

	if(strncmp("none",source_up,strlen(source_up))!=0){
		
		dcli_str2ulong((char*)source_up,&up);	
	}
	if(strncmp("none",source_dscp,strlen(source_dscp))!=0){
		
		dcli_str2ulong((char*)source_dscp,&dscp);	
	}
	if(strcmp(source_up,"none")!=0)
	{
     	int uptemp=atoi(source_up);
     	if(uptemp<0 || uptemp>7)
     	{
     		ShowAlert(search(lcontrol,"illegal_input"));
     			return 5;
     	}
	}
	if(strcmp(source_dscp,"none")!=0)
	{
     	int dscptemp=atoi(source_dscp);
     	if(dscptemp<0 || dscptemp>63)
     	{
     		ShowAlert(search(lcontrol,"illegal_input"));
     			return 5;
     	}
	}
	//policer
	if(strcmp(policerID,"0")!=0)
	{
		int temp=atoi(policerID);
		dcli_str2ulong((char *)policerID,&policerId);
		if(temp<1 || temp >256)
		{
			ShowAlert(search(lcontrol,"illegal_input"));
			return 5;
		}
	}
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_QOS_OBJPATH,
                                         NPD_DBUS_QOS_INTERFACE,
                                     	 NPD_DBUS_METHOD_SET_QOS_INGRESS_POLICY_BASE_ON_ACL);		
	dbus_error_init(&err);
	dbus_message_append_args(query, 				  
							 DBUS_TYPE_UINT32,&ruleIndex,
							 DBUS_TYPE_UINT32,&ruleType,
							 DBUS_TYPE_UINT32,&profileIndex,
							 DBUS_TYPE_UINT32,&up,
							 DBUS_TYPE_UINT32,&dscp,
							 DBUS_TYPE_UINT32,&policer,
							 DBUS_TYPE_UINT32,&policerId,
							 DBUS_TYPE_UINT32,&remark,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,	
		DBUS_TYPE_INVALID)){	
		 if(ACL_GLOBAL_EXISTED == op_ret)
				//vty_out(vty,"%% Access-list %d existed!\n",(ruleIndex+1));
				{
					ShowAlert(search(lcontrol,"acl_existed"));
				}
			
			else if(ACL_EXT_NO_SPACE==op_ret)
			{
				sprintf(alert,"%s %d %s",search(lcontrol,"acl_no_space_alert_one"),(ruleIndex-511),search(lcontrol,"acl_no_space_alert_two"));
				ShowAlert(alert);
			}
		        //vty_out(vty,"%% because the extended should take up twice spaces than stdard acl,but %d has been set extended!\n",(ruleIndex-511));
	
			else if(ACL_SAME_FIELD == op_ret)
			{}
				//vty_out(vty,"%% identical fields of packet can not set again\n");	
			else if(QOS_PROFILE_NOT_EXISTED ==op_ret)
			{
				ShowAlert(search(lcontrol,"qos_profile_not_exist"));
			}
				//vty_out(vty,"%% QoS profile %d not existed!\n",profileIndex);
			else if(NPD_DBUS_SUCCESS!=op_ret)
			{
				ShowAlert(search(lcontrol,"QOS_base_acl_fail"));
			}
				//vty_out(vty,"%% Set QOS ingress-policy based on ACL fail!\n");	
			else if(NPD_DBUS_SUCCESS==op_ret)
			{
				ShowAlert(search(lcontrol,"acl_add_suc"));
			}
	} 		
	else {		
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	free(alert);
	return CMD_SUCCESS;
}



int addrule_egress_qos(char * ruleindex,char * egress_up,char * egress_dscp,char * source_up,char * source_dscp,struct list * lcontrol)
{
	DBusMessage 	*query = NULL, *reply = NULL;
	DBusError		err = { 0 };
	unsigned int	ruleIndex=0,ruleType=0;
	unsigned int    up=10,dscp=100,egrUp=0,egrDscp=0;
	unsigned int    op_ret;
	//unsigned int    policer=0;//policerId=0;
	char * alert = (char *)malloc(1024);
	memset(alert,0,1024);
	
	dcli_str2ulong((char *)ruleindex,&ruleIndex);
	ruleType = EXTENDED_ACL_RULE; 
	ruleIndex -=1;

	dcli_str2ulong((char*)egress_up,&egrUp);
	dcli_str2ulong((char*)egress_dscp,&egrDscp);

	if((strncmp("none",source_up,strlen(source_up))==0)&&(strncmp("none",source_dscp,strlen(source_dscp))==0))
	{
		//vty_out(vty,"%% sourceUp and sourceDscp cannot be both none,acl set invalid!\n");
		ShowAlert(search(lcontrol,"BOTH_None_invalid"));
		return CMD_FAILURE;
	}
	if(strncmp("none",source_up,strlen(source_up))!=0){
	
		dcli_str2ulong((char*)source_up,&up);
	}
	if(strncmp("none",source_dscp,strlen(source_dscp))!=0){
		
		dcli_str2ulong((char*)source_dscp,&dscp);
	}
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_QOS_OBJPATH,
                                         NPD_DBUS_QOS_INTERFACE,
                                         NPD_DBUS_METHOD_SET_QOS_EGRESS_POLICY_BASE_ON_ACL);		
	dbus_error_init(&err);
	dbus_message_append_args(query, 				  
							 DBUS_TYPE_UINT32,&ruleIndex,
							 DBUS_TYPE_UINT32,&ruleType,
							 DBUS_TYPE_UINT32,&egrUp,
							 DBUS_TYPE_UINT32,&egrDscp,
							 DBUS_TYPE_UINT32,&up,
							 DBUS_TYPE_UINT32,&dscp,					
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,	
		DBUS_TYPE_INVALID)){			  
			if(ACL_GLOBAL_EXISTED == op_ret)
				//vty_out(vty,"%% Access-list %d existed!\n",(ruleIndex+1));
				{
					ShowAlert(search(lcontrol,"acl_existed"));
				}
			
			else if(ACL_EXT_NO_SPACE==op_ret)
			{
				sprintf(alert,"%s %d %s",search(lcontrol,"acl_no_space_alert_one"),(ruleIndex-511),search(lcontrol,"acl_no_space_alert_two"));
				ShowAlert(alert);
			}
		        //vty_out(vty,"%% because the extended should take up twice spaces than stdard acl,but %d has been set extended!\n",(ruleIndex-511));
	
			else if(ACL_SAME_FIELD == op_ret)
			{}
				//vty_out(vty,"%% identical fields of packet can not set again\n");	
			else if(NPD_DBUS_SUCCESS!=op_ret)
				ShowAlert(search(lcontrol,"QOS_base_acl_fail"));
			else if(NPD_DBUS_SUCCESS==op_ret)
			{
				ShowAlert(search(lcontrol,"acl_add_suc"));
			}
	} 		
	else {		
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	free(alert);
	return CMD_SUCCESS;
}


int show_acl_group_one(char* type,char * Index,struct  acl_groupone_info *p_acl_grpone_info)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = { 0 };
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	unsigned int j,ret,dir_info=0;
	unsigned int count=0,index=0;
	unsigned int groupIndex=0,portcount=0;
	unsigned int slot_no=0,local_port_no=0;
	unsigned int	vid_count = 0, vid = 0;

	

	if(strcmp("ingress",type)==0){
		dir_info = 0;
	}
	else if (strcmp("egress",type)==0){
		dir_info = 1;
	}	
	else{
		//vty_out(vty,"bad command parameter!\n");
		return NPD_FAIL;
	}
	
	ret = dcli_str2ulongWS((char*)Index,&groupIndex);
	
	
	if (NPD_FAIL == ret) {
		//vty_out(vty,"%% Illegal group number!\n");
		return NPD_FAIL;
	}
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_ACL_OBJPATH,NPD_DBUS_ACL_INTERFACE,NPD_DBUS_METHOD_SHOW_ACL_GROUP_INDEX);
	
	dbus_error_init(&err);
	
	dbus_message_append_args(query, 
							 DBUS_TYPE_UINT32,&dir_info,
							 DBUS_TYPE_UINT32,&groupIndex,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	

	
	if (NULL == reply) 
	 {
		 //vty_out(vty,"failed get reply.\n");
		// ShowAlert(search(lcontrol,"failgetreplay"));
		 if (dbus_error_is_set(&err)) 
		 {

			
			 dbus_error_free(&err);
		 }
		 return NPD_SUCCESS;
	 }
	

		dbus_message_iter_init(reply,&iter);
		
		dbus_message_iter_get_basic(&iter,&ret);	
		if(NPD_DBUS_SUCCESS == ret){
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&count);
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&portcount);
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&vid_count);
			dbus_message_iter_next(&iter);
			dbus_message_iter_recurse(&iter,&iter_array);
			
			//vty_out(vty,"===============================================\n");	
			
			
		   		if(dir_info ==0)
				{
					/*vty_out(vty,"%-40s: %d\n","ingress acl group",groupIndex);\*/
					
					p_acl_grpone_info->groupIndex=groupIndex;
				}
				else if(dir_info ==1)
				{
					//vty_out(vty,"%-40s: %d\n","egress acl group",groupIndex);
					p_acl_grpone_info->groupIndex=groupIndex;
				}				
			
			//vty_out(vty,"%-40s: %d\n","bind by port count",portcount);
			
			p_acl_grpone_info->bind_by_port_count= portcount;
			

			for (j = 0; j < portcount; j++) {
				DBusMessageIter iter_struct;
				dbus_message_iter_recurse(&iter_array,&iter_struct);			
				dbus_message_iter_get_basic(&iter_struct,&slot_no);
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&local_port_no);
				dbus_message_iter_next(&iter_array); 
				
				//vty_out(vty,"%-40s: 1/%d\n","bind by port",index);
				
				p_acl_grpone_info->bind_by_slot[j] = slot_no;
				p_acl_grpone_info->bind_by_port[j] = local_port_no;
				
				}
				dbus_message_iter_next(&iter);	
				
				dbus_message_iter_recurse(&iter,&iter_array);
				p_acl_grpone_info->vlan_count=vid_count;
				
			//vty_out(vty,"%-40s: %d\n","binded by vlan count",vid_count);
			for (j = 0; j < vid_count; j++) 
			{
				DBusMessageIter iter_struct;
				dbus_message_iter_recurse(&iter_array,&iter_struct);			
				dbus_message_iter_get_basic(&iter_struct,&vid);
				dbus_message_iter_next(&iter_array); 

				p_acl_grpone_info->bind_by_vlan[j]=vid;
				
				//vty_out(vty,"%-40s: %d \n","binded by vlan", vid);
			}
			

			//vty_out(vty,"%-40s: %d\n","acl count",count);
			p_acl_grpone_info->acl_count = count;
			dbus_message_iter_next(&iter);			
			dbus_message_iter_recurse(&iter,&iter_array);					    
			for (j = 0; j < count; j++) {
				DBusMessageIter iter_struct;
				dbus_message_iter_recurse(&iter_array,&iter_struct);			
				dbus_message_iter_get_basic(&iter_struct,&index);
				dbus_message_iter_next(&iter_array); 
				
				//vty_out(vty,"%-40s: %d\n","acl index",index);
				p_acl_grpone_info->index[j] = index;
				}
			//vty_out(vty,"===============================================\n");
		
					
		}
		else if(ACL_GROUP_NOT_EXISTED == ret)
		{			
		   if(dir_info ==0)
		   {
				//vty_out(vty,"%% ingress acl group %d not existed!\n",groupIndex);
				return ACL_GROUP_NOT_EXISTED;
		    }
		    else if(dir_info ==1)
		    {
				//vty_out(vty,"%% egress acl group %d not existed!\n",groupIndex);
				return ACL_GROUP_NOT_EXISTED;
		     }		
		
		}
		if(ACL_GROUP_INDEX_ERROR==ret)
			 //vty_out(vty,"%% Acl group range should be 1-1023!\n");
			 //ShowAlert(search(lcontrol,"grprange"));
			 return ACL_GROUP_INDEX_ERROR;

	dbus_message_unref(reply);
	 return NPD_SUCCESS;
}

int addrule_MirrorOrRedir_TcpOrUdp_extended(char * Index,char * rule,char * portslot,char * dipmask,char * dport,char * sipmask,char * sport,char * dmac,char * smac,char *vid,char * sourcePort,char * policerID)
{	  

	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lcontrol;	/*解析help.txt文件的链表头*/
	lpublic=get_chain_head("../htdocs/text/public.txt");
	lcontrol=get_chain_head("../htdocs/text/control.txt"); 
	DBusMessage 	*query = NULL, *reply = NULL;
	DBusError		err = { 0 };
	ETHERADDR		dmacAddr,smacAddr;
	unsigned int	ruleIndex=0,dipmaskLen=0,sipmaskLen=0;
	unsigned int	actionType,packetType,srcport=0,dstport=0,vlanId=0;
	unsigned int	 ruleType,op_ret,ret,temp;
	unsigned char    slot_no=0,port_no=0,dataslot=ACL_ANY_PORT_CHAR,dataport=ACL_ANY_PORT_CHAR;
	unsigned int	policer=0,policerId=0;
	int Val=0;
	unsigned long	dipno=0,sipno=0;
	char * alert = (char *)malloc(1024);
	memset(alert,0,1024);

	dcli_str2ulong(Index,&ruleIndex);
	#if 0
	if (NPD_FAIL == ret) 
	{
		ShowAlert(search(lcontrol,"acl_illegal_input"));
		return CMD_FAILURE;
	}
	#endif
	
	//ruleIndex = Index;
	ruleType = EXTENDED_ACL_RULE; 
	
	ruleIndex -=1;
	actionType=4;
	
	temp = parse_slotport_no((char*)portslot,&slot_no,&port_no);
	
	if (NPD_FAIL == temp) 
	{
    		ShowAlert(search(lcontrol,"illegal_slot_pot_input"));
		return NPD_FAIL;
	}
	
	if(0==strncmp("udp",rule,strlen(rule)))
	{
		packetType=1;
	}
	else if(0==strncmp("tcp",rule,strlen(rule)))
	{
		packetType=2;
	}
	
	if(strncmp("any",dipmask,strlen(dipmask))==0)
	{
		dipmaskLen=0;
		dipno=0;
	}
	else
	{
		Val=ip_address_format2ulong((char**)&dipmask,&dipno,&dipmaskLen);			
		if(CMD_WARNING == Val) 
		{
			 //vty_out(vty, "%% Bad parameter %s\n", dipmask);
			 return CMD_WARNING;
		}
		VALUE_IP_MASK_CHECK(dipmaskLen);
	}
	
	if(strncmp("any",dport,strlen(dport))==0) 
	{	   
		dstport = ACL_ANY_PORT;
	}
	else
	{
		ret= dcli_str2ulongWS((char*)dport,&dstport);
		if(ret==NPD_FAIL)
		{
			//vty_out(vty,"%% Illegal destination port index!\n");
			return NPD_FAIL;
		}

	}
	if(strncmp("any",sipmask,strlen(sipmask))==0)
	{ 
		sipmaskLen=0;
		sipno=0;
	}
	else
	{						
		 Val=ip_address_format2ulong((char**)&sipmask,&sipno,&sipmaskLen); 	
		 if(CMD_WARNING == Val) 
		 {
			  //vty_out(vty, "%% Bad parameter %s\n", sipmask);
			  return CMD_WARNING;
		 }
		 VALUE_IP_MASK_CHECK(sipmaskLen);
	}
	
	if(strncmp("any",sport,strlen(sport))==0) {	   
		srcport = ACL_ANY_PORT;
	}
	else
	{
		ret	= dcli_str2ulongWS((char*)sport,&srcport);
		if(ret==NPD_FAIL)
		{
			//vty_out(vty,"%% Illegal source port index!\n");
			ShowAlert(search(lcontrol,"illegal_slot_pot_input"));
			return NPD_FAIL;
		}
	}
	memset(&smacAddr,0,sizeof(ETHERADDR));
	memset(&dmacAddr,0,sizeof(ETHERADDR));

	/*dmac*/
	if(strncmp("any",dmac,strlen(dmac))!=0)
	{
		op_ret = parse_mac_addr((char *)dmac,&dmacAddr);
		if (NPD_FAIL == op_ret) 
		{
			ShowAlert(search(lcontrol,"unkown_mac_format"));
			return NPD_FAIL;
		}
	}
	/*smac*/
	if(strncmp("any",smac,strlen(smac))!=0)
	{
		op_ret = parse_mac_addr((char *)smac,&smacAddr);
		if (NPD_FAIL == op_ret) 
		{
			ShowAlert(search(lcontrol,"unkown_mac_format"));
			return NPD_FAIL;
		}
	}
		
	if(strncmp("any",vid,strlen(vid))!=0)
	{
		dcli_str2ulong(vid,&vlanId);
	}
	/*dataslot/dataport*/
	if(strncmp("any",sourcePort,strlen(sourcePort))!=0)
	{
		ret = parse_slotport_no((char *)sourcePort,&dataslot,&dataport);

		if (NPD_FAIL == ret) 
		{
			//vty_out(vty,"%% Unknown portno format.\n");
			ShowAlert(search(lcontrol,"illegal_slot_pot_input"));
			return NPD_FAIL;
		}
	}
	if(strcmp(policerID,"0")!=0)
	{
		policer = 1;
		int temp=atoi(policerID);
		dcli_str2ulong((char *)policerID,&policerId);
		if(temp<1 || temp >256)
		{
			ShowAlert(search(lcontrol,"illegal_input"));
			return 5;
		}
	}
	else if(actionType==1){
			policer =0;
			policerId =0;
		}		
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
  
   
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{
			if (NPD_DBUS_SUCCESS == op_ret ) 
			{
				ShowAlert(search(lcontrol,"acl_add_suc"));
				return NPD_DBUS_SUCCESS;
				//vty_out(vty,"Access-list %d is successfully set!\n",(ruleIndex+1));
			}
			else if(NPD_DBUS_ERROR_NO_SUCH_PORT ==op_ret)
			{
				//ShowAlert(search(lcontrol,"acl_existed"));
				return NPD_DBUS_ERROR_NO_SUCH_PORT;
				//vty_out(vty,"%% Error! illegal port index\n");
			}
			else if(ACL_GLOBAL_EXISTED == op_ret)
			{
				ShowAlert(search(lcontrol,"acl_existed"));
				return ACL_GLOBAL_EXISTED;
				//vty_out(vty,"%% Access-list %d existed!\n",(ruleIndex+1));
			}
			else if(ACL_EXT_NO_SPACE==op_ret)
			{
				sprintf(alert,"%s %d %s",search(lcontrol,"acl_no_space_alert_one"),(ruleIndex-511),search(lcontrol,"acl_no_space_alert_two"));
				ShowAlert(alert);
				return ACL_EXT_NO_SPACE;
				 /*vty_out(vty,"%% because the extended rule should take up twice spaces than standard acl,\
				 			  but %d has been set standard rule! ~~set fail\n",(ruleIndex+513));*/
			}
			else if(ACL_SAME_FIELD == op_ret)
			{
				ShowAlert(search(lcontrol,"same_field_not_set"));
				return ACL_EXT_NO_SPACE;
				//vty_out(vty,"%% identical fields of packet can not set again\n");	
			}
			else if(ACL_POLICER_ID_NOT_SET == op_ret)
			{
				return ACL_POLICER_ID_NOT_SET;
				//vty_out(vty,"%% Policer %d not existed!\n",policerId);
			}
			else 
			{
				ShowAlert(search(lcontrol,"add_rule_fail"));
				return CMD_FAILURE;
			}
				//vty_out(vty,"%% set fail!\n");
		
	} 
	else 
	{
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	release(lpublic);
	release(lcontrol);
	free(alert);
	return CMD_SUCCESS; 
	
}
