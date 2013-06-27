#ifdef _D_WCPSS_

#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

#include <dbus/dbus.h>
#include "wcpss/waw.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/ACDbusDef1.h"
#include "dbus/asd/ASDDbusDef1.h"
#include "wcpss/asd/asd.h"
#include "wid_ac.h"
#include "wid_wtp.h"
#include "dbus/wcpss/dcli_wid_wtp.h"
int wid_oui_mac_format_check(char* str,int len){
	int i = 0;
	unsigned int result = 0;
	char c = 0;
	if( 8 != len){
	   return -1;
	}
	for(;i<len;i++) {
		c = str[i];
		if((2 == i)||(5 == i)){
			if((':'!=c)&&('-'!=c)&&('.'!=c))
				return -1;
		}
		else if((c>='0'&&c<='9')||
			(c>='A'&&c<='F')||
			(c>='a'&&c<='f'))
			continue;
		else {
			result = -1;
			return result;
		}
    }
	if((str[2] != str[5])){
		result = -1;
		return result;
	}
	return result;
}
 int wid_parse_oui_mac_addr(char* input,unsigned char* oui_mac) 
 {
	int i = 0;
	char cur = 0,value = 0;
	if((NULL == input)||(NULL == oui_mac)) return -1;
	if(-1 == wid_oui_mac_format_check(input,strlen(input))) {
		return -1;
	}	
	for(i = 0; i < OUI_LEN;i++) {
		cur = *(input++);
		if((cur == ':') ||(cur == '-')||(cur == '.')){
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
		oui_mac[i] = value;
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
		oui_mac[i] = (oui_mac[i]<< 4)|value;
	}
	return 0;
} 
void str2higher(char **str) {  
	int i,len;
	char *ptr;

	len = strlen(*str);
	ptr = *str;

	for(i=0; i<len ; i++) {
		if((ptr[i]<='z')&&(ptr[i]>='a'))  
			ptr[i] = ptr[i]-'a'+'A';  
	}
	
	return;
}
unsigned int ip_long2str(unsigned long ipAddress,unsigned char **buff){
	unsigned long	 cnt = 0;
	unsigned char *tmpPtr = *buff;
 
	cnt = sprintf((char*)tmpPtr,"%ld.%ld.%ld.%ld",(ipAddress>>24) & 0xFF, \
			(ipAddress>>16) & 0xFF,(ipAddress>>8) & 0xFF,ipAddress & 0xFF);
	 
	return cnt;
}

unsigned long wid_ip2ulong(char *str)
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


void CheckWTPState(char *state, unsigned char WTPstate){
	
	switch(WTPstate){

		case 2 :
			strcpy(state, "join");
			break;
			
		case 3 :
			strcpy(state, "configure");
			break;
		
		case 4 :
			strcpy(state, "datacheck");
			break;

		case 5 :
			strcpy(state, "run");
			break;
			
		case 7 :
			strcpy(state, "quit");
			break;				

		case 8 :
			strcpy(state, "imagedata");
			break;
		case 9 :
			strcpy(state, "bak_run");
			break;			
		
	}

}

void CheckWTPQuitReason(char *quitreason,unsigned char quitstate){

	switch(quitstate){

		case 0 :
			strcpy(quitreason, "unknown");
			break;
			
		case 1 :
			strcpy(quitreason, "wtp_unused");
			break;
		
		case 2 :
			strcpy(quitreason, "wtp_normal");
			break;

		case 3 :
			strcpy(quitreason, "no_interface");
			break;
			
		case 4 :
			strcpy(quitreason, "no_if_flag");
			break;				

		case 5 :
			strcpy(quitreason, "if_down");
			break;
		case 6 :
			strcpy(quitreason, "no_if_addr");
			break;
		case 7 :
			strcpy(quitreason, "wtp_timeout");
			break;
		
	}

}

int parse_int_ID(char* str,unsigned int* ID){
	char *endptr = NULL;
	char c;
	c = str[0];
	if (c>='0'&&c<='9'){
		*ID= strtoul(str,&endptr,10);
		if((c=='0')&&(str[1]!='\0')){
			 return WID_UNKNOWN_ID;
		}
		if((endptr[0] == '\0')||(endptr[0] == '\n'))
			return WID_DBUS_SUCCESS;
		else
			return WID_UNKNOWN_ID;
	}
	else
		return WID_UNKNOWN_ID;
}

int parse_int_value(char* str, int* ID){
	char *endptr = NULL;
	char c;
	c = str[0];
	if (('-' == c) || (('0' <= c) && ('9' >= c)))
	{
		*ID= strtol(str, &endptr, 10);
		if(('0' == c) && ('\0' != str[1]))
		{
			 return WID_UNKNOWN_ID;
		}
		if(('-' == c) && ('\0' == str[1]))
		{
			return WID_UNKNOWN_ID;
		}
		if(('-' == c) && ('0' == str[1]))
		{
			 return WID_UNKNOWN_ID;
		}
		if((endptr[0] == '\0')||(endptr[0] == '\n'))
		{
			return WID_DBUS_SUCCESS;
		}
		else
		{
			return WID_UNKNOWN_ID;
		}
	}
	else
	{
		return WID_UNKNOWN_ID;
	}
}

int read_ac_info(char *FILENAME,char *buff)
{
	int len,fd;
	
	fd = open(FILENAME,O_RDONLY);
	if (fd < 0) {
		return 1;
	}	
	len = read(fd,buff,DEFAULT_LEN);	
	if (len < 0) {
		close(fd);
		return 1;
	}	
	close(fd);
	return 0;
}

char * WID_parse_ap_extension_command(const char **argv, int argc)
{
  int i;
  size_t len;
  char *str;
  char *p;
  int j = 1;

  len = 0;
  for (i = 0; i < argc; i++)
    len += strlen(argv[i])+1;
  if (!len)
    return NULL;
  p = str = malloc(len);
  memset(p,0,len);
  for (i = 0; i < argc; i++)
    {
      size_t arglen;
      memcpy(p, argv[i], (arglen = strlen(argv[i])));
      p += arglen;
      *p++ = ' ';
    }
  *(p-1) = '\0';
  return str;
}

char * WID_parse_ap_option60_parameter(const char **argv, int argc)
{
  int i;
  size_t len;
  char *str;
  char *p;
  int j = 1;

  len = 0;
  for (i = 0; i < argc; i++)
    len += strlen(argv[i])+1;
  if (!len)
    return NULL;
  p = str = malloc(len);
  memset(p,0,len);
  for (i = 0; i < argc; i++)
    {
      size_t arglen;
      memcpy(p, argv[i], (arglen = strlen(argv[i])));
      p += arglen;
      *p++ = ' ';
    }
  *(p-1) = '\0';
  return str;
}


char * insert_ten_tcpdump_command(char *command,char *command1,char a,char b,char c)
{
	int m=0; if(command==NULL) return NULL;
	int n=strlen(command);
	int i;
	for(i=0;i<n;i++)
	{
			command1[m]=command[i];
			if(command[i]=='-'&&command[i+1]=='c')
				{	
					command1[m+1]='c';				
					command1[m+2]=a;
					command1[m+3]=b;
					command1[m+4]=c;
					m+=4;
					i+=1;
					//printf("command[i]=%d\n",command[i]);
					//printf("command1[m]=%d\n",command[m]);
				}
			m+=1;
	}
	command1[m]='\0';
	//printf("command1=%s\n",command1);
	return command1;
}

int WID_Check_IP_Format(char* str){
	char *endptr = NULL;
	int endptr1 = 0;
	char c;
	int IP,i;
	c = str[0];
	if (c>='0'&&c<='9'){
		IP= strtoul(str,&endptr,10);
		if(IP < 0||IP > 255)
			return WID_UNKNOWN_ID;
		else if(((IP < 10)&&((endptr - str) > 1))||((IP < 100)&&((endptr - str) > 2))||((IP < 256)&&((endptr - str) > 3))){
			return WID_UNKNOWN_ID;
		}
		for(i = 0; i < 3; i++){
			if(endptr[0] == '\0'||endptr[0] != '.')
				return WID_UNKNOWN_ID;
			else{
				endptr1 = &endptr[1];
				IP= strtoul(&endptr[1],&endptr,10);				
				if(IP < 0||IP > 255)
					return WID_UNKNOWN_ID;				
				else if(((IP < 10)&&((endptr - endptr1) > 1))||((IP < 100)&&((endptr - endptr1) > 2))||((IP < 256)&&((endptr - endptr1) > 3))){
					return WID_UNKNOWN_ID;
				}
			}
		}
		if(endptr[0] == '\0' && IP > 0)
			return WID_DBUS_SUCCESS;
		else
			return WID_UNKNOWN_ID;
	}
	else
		return WID_UNKNOWN_ID;

}
/*fengwenchao add 20111101*/
int check_mask_Legal(char* str)
{
	char *endptr = NULL;
	int endptr1 = 0;
	char c;
	int IP,i,k;
	c = str[0];
	int flag = 0;
	
	IP= strtoul(str,&endptr,10);
	if((IP&0xff) == 0xff)
	{	
		for(i =0;i<3;i++)
		{			
			endptr1 = &endptr[1];
			IP= strtoul(&endptr[1],&endptr,10);

			if((IP&0xff) == 0xff)
			{
				continue;
			}
			else if((IP&0xff) != 0xff)
			{
				int flag_zero = 0;
				for(k=0;k<8;k++)
				{
					if(((IP<<k)&128)==0)
					{
						flag_zero =1;
					}
					else if((((IP<<k)&128)==128)&&(flag_zero == 0))
					{
						continue;
					}
					else if((((IP<<k)&128)==128)&&(flag_zero == 1))
					{
						goto return_fail;
					}						
				}
				continue;
			}
			else
			{
				return -1;
			}
		}
	}
	else
	{
		return -1;
	}
	
	return 0;
	return_fail:
		return -1;
}
/*fengwenchao add end*/

int WID_Check_Mask_Format(char* str){
	char *endptr = NULL;
	int endptr1 = 0;
	char c;
	int IP,i;
	c = str[0];
	if (c>='0'&&c<='9'){
		IP= strtoul(str,&endptr,10);
		if(IP < 0||IP > 255)
			return WID_UNKNOWN_ID;
		else if(((IP < 10)&&((endptr - str) > 1))||((IP < 100)&&((endptr - str) > 2))||((IP < 256)&&((endptr - str) > 3))){
			return WID_UNKNOWN_ID;
		}
		for(i = 0; i < 3; i++){
			if(endptr[0] == '\0'||endptr[0] != '.')
				return WID_UNKNOWN_ID;
			else{
				endptr1 = &endptr[1];
				IP= strtoul(&endptr[1],&endptr,10);				
				if(IP < 0||IP > 255)
					return WID_UNKNOWN_ID;				
				else if(((IP < 10)&&((endptr - endptr1) > 1))||((IP < 100)&&((endptr - endptr1) > 2))||((IP < 256)&&((endptr - endptr1) > 3))){
					return WID_UNKNOWN_ID;
				}
			}
		}
		if(endptr[0] == '\0' && IP >= 0)
			return WID_DBUS_SUCCESS;
		else
			return WID_UNKNOWN_ID;
	}
	else
		return WID_UNKNOWN_ID;

}

char* WID_parse_CMD_str(const char **argv, int argc, const char *buf, int preflag)
{	
	int i;
	size_t len;
	char *str = NULL;
	char *p = NULL;
	int j = 1;
	if(buf != NULL){	
		len = strlen(buf);
		for (i = 0; i < argc; i++){
		  if(len == strlen(argv[i])&&(strcmp(buf,argv[i])==0)){
		  	if(preflag == 1){
				str = WID_parse_ap_extension_command(&argv[0],i);
				return str;
			}else if(i < argc-1){		
				str = WID_parse_ap_extension_command(&argv[i+1],argc-1-i);
				return str;
			}else
				return NULL;
		  }
		}
		if(i == argc)
			return NULL;
	}else
		str = WID_parse_ap_extension_command(argv,argc);
	return str;
}
int wid_wtp_parse_char_ID(char* str,unsigned char* ID)
{
	char *endptr = NULL;
	char c;
	c = str[0];
	if (c>='0'&&c<='9')
	{
		*ID= strtoul(str,&endptr,10);
		if(endptr[0] == '\0')
		{
			return WID_DBUS_SUCCESS;
		}
		else
		{
			return WID_UNKNOWN_ID;
		}
	}
	else
	{
		return WID_UNKNOWN_ID;
	}
}

int wtp_check_wtp_ip_addr(char *ipaddr, char *WTPIP)
{
	char c;
	c = WTPIP[0];
	int ret = 0;
	if(c == '\0')
	{
		/*strcpy(ipaddr, "***.***.***.***:*****");*/
		strcpy(ipaddr, "  *.  *.  *.  *:****");
		ret = 1;
	}
	else
	{
		ret = 0;
	}
	
	return ret;
}

int check_ip_with_mask(unsigned long ipaddr,unsigned long mask,char * WTPIP)
{
	unsigned long wtpip = wid_ip2ulong(WTPIP);

	if(((((ipaddr>>24) & 0xFF) & ((mask>>24) & 0xFF)) == (((wtpip>>24) & 0xFF) & ((mask>>24) & 0xFF)))&&
		((((ipaddr>>16) & 0xFF) & ((mask>>16) & 0xFF)) == (((wtpip>>16) & 0xFF) & ((mask>>16) & 0xFF)))&&
		((((ipaddr>>8) & 0xFF) & ((mask>>8) & 0xFF)) == (((wtpip>>8) & 0xFF) & ((mask>>8) & 0xFF)))&&
		((((ipaddr) & 0xFF) & ((mask) & 0xFF)) == (((wtpip) & 0xFF) & ((mask)& 0xFF)))
		)
	{
		return 1;
	}
	else
	{
		return 0;
	}	

}
int check_mac_with_mask(WIDMACADDR *macaddr,WIDMACADDR *macmask,char *WTPMAC)
{
	if(((macaddr->macaddr[0] & macmask->macaddr[0]) == (WTPMAC[0] & macmask->macaddr[0]))&&
		((macaddr->macaddr[1] & macmask->macaddr[1]) == (WTPMAC[1] & macmask->macaddr[1]))&&
		((macaddr->macaddr[2] & macmask->macaddr[2]) == (WTPMAC[2] & macmask->macaddr[2]))&&
		((macaddr->macaddr[3] & macmask->macaddr[3]) == (WTPMAC[3] & macmask->macaddr[3]))&&
		((macaddr->macaddr[4] & macmask->macaddr[4]) == (WTPMAC[4] & macmask->macaddr[4]))&&
		((macaddr->macaddr[5] & macmask->macaddr[5]) == (WTPMAC[5] & macmask->macaddr[5]))
		)
	{
		return 1;
	}
	else
	{
		return 0;
	}	


}
int dcli_wtp_remove_list_repeat(int list[],int num)
{
	int i,j,k;
	for(i=0;i<num;i++){ 
        for(j=i+1;j<num;j++)  { 
              if(list[i]==list[j])  { 
                  num--;
                  for(k=j;k<num;k++) { 
                       list[k]=list[k+1]; 
                  } 
                  j--; 
               } 
         } 
     } 
	/*for(i=0;i<num;i++)
	 {
	 	//printf("%d\n",list[i]);
	 }*/
     return num; 

}


#if 0
int dcli_wtp_parse_wtp_list(char* ptr,int* count,int wtpId[])
{
	char* endPtr = NULL;
	int   i=0;
	endPtr = ptr;
	wtp_list_state state = dcli_wtp_check_wtpid;

	while(1)
	{
		switch(state)
		{
			
			case dcli_wtp_check_wtpid: 
									wtpId[i] = strtoul(endPtr,&endPtr,10);
									if(wtpId[i]>0&&wtpId[i]<WTP_NUM)
									{
					            		state=dcli_wtp_check_comma;
									}
									else
										state=dcli_wtp_check_fail;
									break;
		
			case dcli_wtp_check_comma: 
									if (WTP_LIST_SPLIT_COMMA== endPtr[0])
									{
										endPtr = (char*)endPtr + 1;
										state = dcli_wtp_check_wtpid;
										i++;
									}
									else
										state = dcli_wtp_check_end;
									break;
				
		
			case dcli_wtp_check_fail:
									return -1;
									break;

			case dcli_wtp_check_end: 
									if ('\0' == endPtr[0]) 
									{
										state = dcli_wtp_check_success;
										i++;
									}
									else
										state = dcli_wtp_check_fail;
									break;
			
			case dcli_wtp_check_success: 
									*count = i;
									return 0;
									break;
			
			default: break;
		}
		
	}
		
}

int parse_wtpid_list(char* ptr,update_wtp_list **wtplist)
{
	char* endPtr = NULL;
	int   wtpid1 = 0;
	int   wtpid2 = 0;
	int   min = 0;
	int	  max = 0;
	endPtr = ptr;
	wtp_list_state state = dcli_wtp_check_wtpid;
	struct tag_wtpid *wtp_id = NULL;
	
	while(1)
	{
		switch(state)
		{
			
			case dcli_wtp_check_wtpid: 
									wtpid1 = strtoul(endPtr,&endPtr,10);
									if(wtpid1>0&&wtpid1<WTP_NUM)
									{
					            		state=dcli_wtp_check_comma;
									}
									else
										state=dcli_wtp_check_fail;
									break;
		
			case dcli_wtp_check_comma: 
				
									if(WTP_LIST_SPLIT_COMMA == endPtr[0])
									{
										endPtr = (char*)endPtr + 1;
										state = dcli_wtp_check_wtpid;
										//save wtpid1
										wtp_id = (struct tag_wtpid*)malloc(sizeof(struct tag_wtpid));
										wtp_id->wtpid = wtpid1;
										wtp_id->next = NULL;

										//insert to list
										wtp_id->next = (*wtplist)->wtpidlist;
										(*wtplist)->wtpidlist = wtp_id;
										(*wtplist)->count++;
																				
									}
									else if(WTP_LIST_SPLIT_BAR == endPtr[0])
									{
										endPtr = (char*)endPtr + 1;
										wtpid2 = strtoul(endPtr,&endPtr,10);
										if(wtpid2>0&&wtpid2<WTP_NUM)
										{
						            		//save wtpid1
											min = (wtpid2 > wtpid1)?wtpid1:wtpid2;
											max = (wtpid2 > wtpid1)?wtpid2:wtpid1;
											while(min <= max)
											{
												wtp_id = (struct tag_wtpid*)malloc(sizeof(struct tag_wtpid));
												wtp_id->wtpid = min;
												wtp_id->next = NULL;

												//insert to list
												wtp_id->next = (*wtplist)->wtpidlist;
												(*wtplist)->wtpidlist = wtp_id;
												(*wtplist)->count++;	
													
												min++;
											}
											if('\0' == endPtr[0])
											{
												return 0;
											}
											else
											{											
												endPtr = (char*)endPtr + 1;
												state=dcli_wtp_check_wtpid;
											}
										}
										else
										{
											state = dcli_wtp_check_fail;
										}
										
									}
									else
										state = dcli_wtp_check_end;
									break;
				
		
			case dcli_wtp_check_fail:
				
									return -1;
									break;

			case dcli_wtp_check_end: 
				
									if ('\0' == endPtr[0]) 
									{
										state = dcli_wtp_check_success;
									}
									else
										state = dcli_wtp_check_fail;
									break;
			
			case dcli_wtp_check_success: 
				
										//save wtpid1
										wtp_id = (struct tag_wtpid*)malloc(sizeof(struct tag_wtpid));
										wtp_id->wtpid = wtpid1;
										wtp_id->next = NULL;

										//insert to list
										wtp_id->next = (*wtplist)->wtpidlist;
										(*wtplist)->wtpidlist = wtp_id;
										(*wtplist)->count++;				
									return 0;
									break;
			
			default:

				break;
		}
		
	}
	
}
#endif
void destroy_input_wtp_list(update_wtp_list *pwtplist)
{

	if(pwtplist == NULL)
	{
		return;
	}

	struct tag_wtpid *phead = NULL;
	struct tag_wtpid *pnext = NULL;
	phead = pwtplist->wtpidlist;
	
	
	while(phead != NULL)
	{			
		pnext = phead->next;	
		free(phead);
		phead = NULL;
		phead = pnext;

	}	
	
	free(pwtplist);
	pwtplist = NULL;
}
void delsame(update_wtp_list *pwtplist)
{
	struct tag_wtpid *p,*q,*temp1,*temp2;
	
	p = pwtplist->wtpidlist;
	if(p == NULL) return;
	
	q = p->next;
	if(q == NULL) return;
	
	while((p != NULL)&&(p->next != NULL))
	{
	  temp1=p;
	  q=p->next;  
	  while(q)
	  {
		if(p->wtpid != q->wtpid)
	 	{
  			q=q->next;temp1=temp1->next;
	    }
		else
		{
			temp2=q;
			q=q->next;
			temp1->next=q;
			free(temp2);
			temp2 = NULL;
			pwtplist->count--;
			
		}
	  }
	  p=p->next;
	  
	}
}

int dcli_wtp_add_wlanid_node(WID_WTP **list,struct wlanid *ele){

	int ret;
//	struct wlanid *node=NULL;

	if(ele==NULL)
		return -1;

	if((*list) == NULL){
	//	CW_CREATE_OBJECT_ERR(node,struct wlanid, return -1;);
	//	node->wlanid =ele->wlanid;
		ele->next=NULL;

	//	(*list)->count++;
		(*list)->Wlan_Id = ele;

	}else{
		//	CW_CREATE_OBJECT_ERR(node, struct wlanid, return -1;);
		//	node->wlanid =ele->wlanid;
			ele->next=(*list)->Wlan_Id;
			(*list)->Wlan_Id = ele;
			
		//	(*list)->count++;			
		
	}

	return 0;
}

int dcli_wtp_method_parse_fist(char *DBUS_METHOD){
	unsigned int sn = 0;
	if (!strcmp(DBUS_METHOD,WID_DBUS_CONF_METHOD_SHOWWTP))
	{
		sn = 1;
	}		
	else if (!strcmp(DBUS_METHOD,WID_DBUS_CONF_METHOD_WTPLIST_NEW))
	{
		sn = 2;
	}
	else if (!strcmp(DBUS_METHOD,WID_DBUS_CONF_METHOD_WTPLIST))
	{
		sn = 3;
	}
	else if (!strcmp(DBUS_METHOD,WID_DBUS_CONF_METHOD_WTPLIST_BYMAC))
	{
		sn = 4;
	}
	else if (!strcmp(DBUS_METHOD,WID_DBUS_CONF_METHOD_ACVERSION))
	{
		sn = 5;
	}	
	else if(!strcmp(DBUS_METHOD,WID_DBUS_CONF_METHOD_WTPLIST_UPDATE_FAIL_LIST))
	{
		sn = 6;
	}	
	else if(!strcmp(DBUS_METHOD,WID_DBUS_CONF_METHOD_AC_ACCESS_WTPLIST))
	{
		sn = 7;
	}	
	else if(!strcmp(DBUS_METHOD,WID_DBUS_WTP_METHOD_SHOW_WTP_MODEL_CODE_VERSION))
	{
		sn = 8;
	}
	else if(!strcmp(DBUS_METHOD,WID_DBUS_CONF_METHOD_WTPLIST_UPDATE))
	{
		sn = 9;
	}
	else if(!strcmp(DBUS_METHOD,WID_DBUS_CONF_METHOD_WTPLIST_NEW_BYINTERFACE))	//xiaodawei add for telecom test, 20110301
	{
		sn = 10;
	}
	//fengwenchao add 20110226
	else if(!strcmp(DBUS_METHOD,WID_DBUS_CONF_METHOD_SHOWWTP_BYMODEL))
	{
		sn = 11;
	}	
	else if(!strcmp(DBUS_METHOD,WID_DBUS_CONF_METHOD_SHOWWTP_BYVERSION))
	{
		sn = 12;
	}
	//fengwenchao add end
	else{}
	return sn ;
}
int dcli_wtp_method_parse_two(char *DBUS_METHOD){
	unsigned int sn = 0;
	if (!strcmp(DBUS_METHOD,WID_DBUS_CONF_METHOD_SHOW_WID_WTP_SAMPLE_THROUGHPUT_INFO))
	{
		sn = 1;
	}		
	else if (!strcmp(DBUS_METHOD,WID_DBUS_CONF_METHOD_AP_SHOW_WTP_EXTENSION_INFOMATION_V3))
	{
		sn = 2;
	}
	else if (!strcmp(DBUS_METHOD,WID_DBUS_CONF_METHOD_SHOW_AP_CM_STATISTICS))
	{
		sn = 3;
	}
	else if (!strcmp(DBUS_METHOD,WID_DBUS_WTP_METHOD_SHOW_WTP_BSS_PKT_INFO))
	{
		sn = 4;
	}
	else if (!strcmp(DBUS_METHOD,WID_DBUS_WTP_METHOD_SHOW_WTP_ETH_PKT_INFO))
	{
		sn = 5;
	}	
	else if(!strcmp(DBUS_METHOD,WID_DBUS_WTP_METHOD_SHOW_WTP_WIFI_SNR_INFO))
	{
		sn = 6;
	}	
	else if(!strcmp(DBUS_METHOD,WID_DBUS_WTP_METHOD_SHOW_WTP_RADIO_PKT_INFO))
	{
		sn = 7;
	}	
	else if(!strcmp(DBUS_METHOD,WID_DBUS_CONF_METHOD_AP_SHOW_WTP_EXTENSION_INFOMATION_V4))
	{
		sn = 8;
	}
	else if(!strcmp(DBUS_METHOD,WID_DBUS_CONF_METHOD_SHOW_AP_MIB_INFO))
	{
		sn = 9;
	}
	else if(!strcmp(DBUS_METHOD,WID_DBUS_WTP_METHOD_SHOW_WTP_IF_INFO))
	{
		sn = 10;
	}
	
	return sn ;
}
int dcli_wtp_method_parse_three(char *DBUS_METHOD){
	unsigned int sn = 0;
	if (!strcmp(DBUS_METHOD,WID_DBUS_CONF_METHOD_ECHOTIMER))
	{
		sn = 1;
	}		
	else if (!strcmp(DBUS_METHOD,WID_DBUS_CONF_METHOD_SHOW_WID_WTP_RUNTIME))
	{
		sn = 2;
	}
	else if (!strcmp(DBUS_METHOD,WID_DBUS_CONF_METHOD_SHOW_WID_WTP_LOCATION))
	{
		sn = 3;
	}
	else if (!strcmp(DBUS_METHOD,WID_DBUS_CONF_METHOD_SHOW_WID_WTP_NETID))
	{
		sn = 4;
	}
	else if (!strcmp(DBUS_METHOD,WID_DBUS_CONF_METHOD_SHOW_WID_WTP_WLAN_VLAN_INFO))
	{
		sn = 5;
	}	
	else if(!strcmp(DBUS_METHOD,WID_DBUS_WTP_METHOD_SHOW_WTP_WIDS_SET))
	{
		sn = 6;
	}	
	else if(!strcmp(DBUS_METHOD,WID_DBUS_CONF_METHOD_AP_SHOW_WTP_MAX_POWER))
	{
		sn = 7;
	}	
	else if(!strcmp(DBUS_METHOD,WID_DBUS_CONF_METHOD_SHOW_UPDATE_CONFIG))
	{
		sn = 8;
	}
	else if(!strcmp(DBUS_METHOD,WID_DBUS_CONF_METHOD_SHOW_OLD_AP_IMG))
	{
		sn = 9;
	}
	else if (!strcmp(DBUS_METHOD, WID_DBUS_WTP_METHOD_SHOW_DHCP_FLOODING_STATUS_SET))
	{
		sn = 16;	
	}
	else //if(!strcmp(DBUS_METHOD,WID_DBUS_WTP_METHOD_SHOW_sWTP_IF_INFO))
	{
		//sn = 10;
	}
	
	return sn ;
}

int dcli_wtp_add_wtp_node(WID_WTP_INFO**list,WID_WTP *ele){

	int ret;
	//wlan_stats_info *node=NULL;
	WID_WTP *tail = NULL;
	tail = (*list)->WTP_LIST;
	if(ele==NULL)
		return -1;

	if(0==(*list)->list_len){
	//	CW_CREATE_OBJECT_ERR(node, wlan_stats_info, return -1;);
		ele->next=NULL;

		(*list)->list_len++;
		(*list)->WTP_LIST = ele;

	}else{
		//	CW_CREATE_OBJECT_ERR(node, struct oui_node, return -1;);
//		ele->next=(*list)->WTP_LIST;
//		(*list)->WTP_LIST=ele;		
		while(tail->next != NULL){
			tail = tail->next;
		}
		tail->next = ele;
		(*list)->list_len++;
		ele->next = NULL;
	}

	return 0;
}
//fengwenchao add 20101223
struct WTP_CONFIG_INFORMATION* show_wtp_config_all_wtp(DBusConnection *dbus_connection,int index,int localid,int* ret,int* wtp_num)
{
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusMessageIter  iter_struct;
	DBusMessageIter	 iter_sub_array;
	DBusMessageIter  iter_sub_struct;
	DBusError err;
	dbus_error_init(&err);

	int i = 0;
	int j = 0;
	char* wlanname = NULL;
	char* wtpBindPort = NULL;

	struct WTP_CONFIG_INFORMATION *WtpHead = NULL;
	struct WTP_CONFIG_INFORMATION *WtpNode = NULL;
	struct WLAN_INFO *wlannode = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SHOW_CONFIG_WTP);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		*ret = -1;
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);	

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,wtp_num);	
	//printf("ret = %d\n",*ret);
	//printf("wtp_num = %d\n",*wtp_num);
	if((*ret == 0)&&(*wtp_num != 0))
	{
		if((WtpHead = (struct WTP_CONFIG_INFORMATION*)malloc(sizeof(struct WTP_CONFIG_INFORMATION))) == NULL)
		{
			*ret = MALLOC_ERROR;
			dbus_message_unref(reply);
			return NULL;
		}
		memset(WtpHead,0,sizeof(struct WTP_CONFIG_INFORMATION));
		WtpHead->wtp_config_list= NULL;
		WtpHead->wtp_config_last= NULL;
			
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);

		for(i=0;i<(*wtp_num);i++)
		{
			if((WtpNode = (struct WTP_CONFIG_INFORMATION*)malloc(sizeof(struct WTP_CONFIG_INFORMATION))) == NULL){
			dcli_free_all_wtp_config_info(WtpHead);
			*ret = MALLOC_ERROR;
			dbus_message_unref(reply);
			return NULL;
			}
			
			memset(WtpNode,0,sizeof(struct WTP_CONFIG_INFORMATION));
			WtpNode->next = NULL;


			WtpNode->wtpBindPort = (char *)malloc(ETH_IF_NAME_LEN +1);
			memset(WtpNode->wtpBindPort,0,(ETH_IF_NAME_LEN +1));

			WtpNode->wtpMacAddr= (char *)malloc(MAC_LEN+1);     //fengwenchao add 20110125
			memset(WtpNode->wtpMacAddr,0,(MAC_LEN +1));

			if(WtpHead->wtp_config_list == NULL){
				WtpHead->wtp_config_list = WtpNode;
				WtpHead->next = WtpNode;
			}
			else{
				WtpHead->wtp_config_last->next = WtpNode;
			}
			WtpHead->wtp_config_last = WtpNode;	

			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->WTPID));	

			//printf("WtpNode->WTPID = %d\n",WtpNode->WTPID);

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpLoadBalanceTrigerBaseUsr));

			//printf("WtpNode->wtpLoadBalanceTrigerBaseUsr = %u\n",WtpNode->wtpLoadBalanceTrigerBaseUsr);

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpLoadBalanceTrigerBaseFlow));

			//printf("WtpNode->wtpLoadBalanceTrigerBaseFlow = %u\n",WtpNode->wtpLoadBalanceTrigerBaseFlow);

			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMaxStaNum));

			//printf("WtpNode->wtpMaxStaNum = %u\n",WtpNode->wtpMaxStaNum);

		//	wtpBindPort = (char *)malloc(ETH_IF_NAME_LEN +1);
		//	memset(wtpBindPort,0,(ETH_IF_NAME_LEN +1));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(wtpBindPort));

			memcpy(WtpNode->wtpBindPort,wtpBindPort,ETH_IF_NAME_LEN);

			//printf("WtpNode->wtpBindPort = %s \n",WtpNode->wtpBindPort);
			/*if(wtpBindPort){
			free(wtpBindPort);
			wtpBindPort = NULL;}*/
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpused));

		//	printf("WtpNode->wtpused = %u \n",WtpNode->wtpused);

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->apply_wlan_num));

		//	printf("WtpNode->apply_wlan_num = %u \n",WtpNode->apply_wlan_num);		

			//fengwenchao add 20110125
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[0]));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[1]));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[2]));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[3]));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[4]));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[5]));

			//fengwenchao add 20110125 end
 			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_recurse(&iter_struct,&iter_sub_array);			

			for(j=0;j<WtpNode->apply_wlan_num;j++)
			{
				if((wlannode = (struct WLAN_INFO*)malloc(sizeof(struct WLAN_INFO))) == NULL){
					dcli_free_all_wtp_config_info(WtpHead);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
				 }
				   memset(wlannode,0,sizeof(struct WLAN_INFO));
				   wlannode->next = NULL;
				   wlannode->wlan_info_list = NULL;
				   wlannode->wlan_info_last = NULL;

				   wlannode->wlanname = (char *)malloc(WID_DEFAULT_NUM+1);
				   memset(wlannode->wlanname,0,(WID_DEFAULT_NUM+1));

                  if(WtpNode->wlan_info_head== NULL){
					   WtpNode->wlan_info_head = wlannode;
				    }
				    else{
					WtpNode->wlan_info_head->wlan_info_last->next = wlannode;
				   }
				
				 WtpNode->wlan_info_head->wlan_info_last = wlannode;
				 dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);

				 dbus_message_iter_get_basic(&iter_sub_struct,&(wlannode->Wlanid)); 

			//	 printf("wlannode->Wlanid = %u \n",wlannode->Wlanid);

				// wlanname = (char *)malloc(WID_DEFAULT_NUM +1);
				// memset(wlanname,0,(WID_DEFAULT_NUM +1));

				 dbus_message_iter_next(&iter_sub_struct);	
				 dbus_message_iter_get_basic(&iter_sub_struct,&(wlanname));	

				 memcpy(wlannode->wlanname,wlanname,WID_DEFAULT_NUM);

			//	 printf("wlannode->wlanname = %s \n",wlannode->wlanname);				 

			/*	if(wlanname){
						 printf("1111111111111111111111 \n");				
				free(wlanname);
				 printf("222222222222222222222222 \n");	
				wlanname = NULL;
				 printf("33333333333333333333333333 \n");	
					}*/

				//fengwenchao add 20110125
				 dbus_message_iter_next(&iter_sub_struct);  
				 dbus_message_iter_get_basic(&iter_sub_struct,&(wlannode->indorpPkts)); 

				 dbus_message_iter_next(&iter_sub_struct);	
				 dbus_message_iter_get_basic(&iter_sub_struct,&(wlannode->intotlePkts));	

				 dbus_message_iter_next(&iter_sub_struct);	
				 dbus_message_iter_get_basic(&iter_sub_struct,&(wlannode->outdorpPkts));	

				 dbus_message_iter_next(&iter_sub_struct);	
				 dbus_message_iter_get_basic(&iter_sub_struct,&(wlannode->outtotlePkts));	

				 dbus_message_iter_next(&iter_sub_struct);	
				 dbus_message_iter_get_basic(&iter_sub_struct,&(wlannode->SSIDDownBandWidthRate));	

				 dbus_message_iter_next(&iter_sub_struct);	
				 dbus_message_iter_get_basic(&iter_sub_struct,&(wlannode->SSIDUpBandWidthRate));

				 //fengwenchao add end

				 dbus_message_iter_next(&iter_sub_array);

			}
			/*if(wtpBindPort){
			free(wtpBindPort);
			wtpBindPort = NULL;
				}*/
			dbus_message_iter_next(&iter_array);	
		}
		dbus_message_unref(reply);	

	}

	return WtpHead;
}
//fengwenchao add end
//fengwenchao add 20101227
void* dcli_wtp_show_api_group_one(
	int index,
	unsigned int id,
	unsigned int id1,
	unsigned int id2,
	unsigned int id3,
	unsigned int* num,
	unsigned int* ret,
	unsigned int* num3,
	unsigned char *num4,
	unsigned char *num5,
	int *num6,
	//DCLI_WTP_API_GROUP_ONE *LIST,	
	DBusConnection *dcli_dbus_connection,
	char *DBUS_METHOD
	)
{	
	//int ret;
	int i = 0;	
	unsigned int dcli_sn = 0;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusError err;
	int wtpid = id1;
	
	DCLI_WTP_API_GROUP_ONE *LIST = NULL;
	int localid = *num6;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);

	//printf("bbbbbbbbbbbbbbbbbbb\n");
	dcli_sn = dcli_wtp_method_parse_fist(DBUS_METHOD);
	if((dcli_sn == 1)||(dcli_sn == 8)){	
		//printf("aaaaaaaaaaaaaaaaaaa\n");
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,DBUS_METHOD);
		dbus_error_init(&err);
		dbus_message_append_args(query,
								 DBUS_TYPE_UINT32,&wtpid,
								 DBUS_TYPE_INVALID);
		
	}
	else if((dcli_sn == 2)||(dcli_sn == 3)||(dcli_sn == 4)||(dcli_sn == 5)||(dcli_sn == 6)||(dcli_sn == 7)||(dcli_sn == 9)||(dcli_sn == 10)||(dcli_sn == 11)||(dcli_sn == 12)){
		//printf("sn=2 22222222\n");
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,DBUS_METHOD);
		dbus_error_init(&err);

	}
/*	else if(dcli_sn == 3){
		//printf("sn=3  333333333\n");
			query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_WTPLIST);
			dbus_error_init(&err);
	}
	else if(dcli_sn == 4){
			query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_WTPLIST_BYMAC);
			dbus_error_init(&err);
	}
	else if(dcli_sn == 5){
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_ACVERSION);
		dbus_error_init(&err);	

	}
	else if(dcli_sn == 6){
		//printf("sn=6   666666\n");
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_WTPLIST_UPDATE_FAIL_LIST);
		dbus_error_init(&err);

	}
	else if(dcli_sn == 7){
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_AC_ACCESS_WTPLIST);
		dbus_error_init(&err);
	}
	else if(dcli_sn == 8){
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WTP_METHOD_SHOW_WTP_MODEL_CODE_VERSION);
 		dbus_error_init(&err);
	
		dbus_message_append_args(query,
								 DBUS_TYPE_UINT32,&wtpid,
								 DBUS_TYPE_INVALID);

	}
	else if(dcli_sn == 9){}*/
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		//printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		*ret = -1;
		return NULL;
	}

	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
//printf("ccccccccc  ret is %d \n",ret);
	if((*ret) == 0 )
	{	
		char* wtpip = NULL;
		char* wtpsn = NULL;
		char* apcode = NULL;
		char* wtpname = NULL;
		char* wtpmodel = NULL;
		char* sysver = NULL;
		char* version = NULL;
		char* codever = NULL;
		char* updatepath = NULL;
		char* updateversion = NULL;
		char* apply_interface_name = NULL;
		
		CW_CREATE_OBJECT_ERR(LIST, DCLI_WTP_API_GROUP_ONE, return NULL;);	
		LIST->WTP = NULL;	
		LIST->WTP_INFO = NULL;	
		LIST->WTP_M_V = NULL;     /*fengwenchao add 20110307*/
		LIST->AP_VERSION = NULL;	
		{
			LIST->num = 0;
			LIST->TotalNum = 0;
		
			LIST->join_num = 0;
			LIST->configure_num = 0;
			LIST->datacheck_num = 0;
			LIST->run_num = 0;
			LIST->quit_num = 0;
			LIST->imagedata_num = 0;
			LIST->bak_run_num = 0;
			LIST->wtp_model_type = 0;   /*fengwenchao add 20110307*/
			LIST->wtp_version_type = 0; /*fengwenchao add 20110314*/
		}	
	if(dcli_sn == 1){	
		unsigned char bwlannum = 0;
		
		//WID_WTP *WTP = NULL;
		//LIST->WTP = (WID_WTP*)malloc(sizeof(WID_WTP));
		//LIST->WTP = WTP;
		//char *WTPMAC = NULL;
		LIST->WTP =  (WID_WTP*)malloc(sizeof(WID_WTP*));/*分配一个空间即可*/
		LIST->WTP[0] =  (WID_WTP*)malloc(sizeof(WID_WTP));
		LIST->WTP[0]->WTPMAC = (unsigned char *)malloc(MAC_LEN +1);
		memset(LIST->WTP[0]->WTPMAC,0,(MAC_LEN +1));
		
		LIST->WTP[0]->Wlan_Id = NULL;
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(LIST->WTP[0]->wtp_triger_num));/*xm*/
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(LIST->WTP[0]->wtp_flow_triger));/*xm*/
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(LIST->WTP[0]->wtp_allowed_max_sta_num));/*xm*/
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(LIST->WTP[0]->WTPID));	
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&wtpsn);	/*(LIST->WTP[0]->WTPSN)*/
		
		LIST->WTP[0]->WTPSN = (char*)malloc(strlen(wtpsn)+1);
		memset(LIST->WTP[0]->WTPSN,0,strlen(wtpsn)+1);
		memcpy(LIST->WTP[0]->WTPSN,wtpsn,strlen(wtpsn));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&wtpname);/*(LIST->WTP[0]->WTPNAME)*/	

		LIST->WTP[0]->WTPNAME = (char*)malloc(strlen(wtpname)+1);
		memset(LIST->WTP[0]->WTPNAME,0,strlen(wtpname)+1);

		memcpy(LIST->WTP[0]->WTPNAME,wtpname,strlen(wtpname));

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&wtpmodel);/*(LIST->WTP[0]->WTPModel)*/	

		LIST->WTP[0]->WTPModel = (char*)malloc(strlen(wtpmodel)+1);
		memset(LIST->WTP[0]->WTPModel,0,strlen(wtpmodel)+1);
		memcpy(LIST->WTP[0]->WTPModel,wtpmodel,strlen(wtpmodel));
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(LIST->WTP[0]->RadioCount));	
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(LIST->WTP[0]->WTPStat));	
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&wtpip);/*(LIST->WTP[0]->WTPIP)*/	

			LIST->WTP[0]->WTPIP = (char*)malloc(strlen(wtpip)+1);
			memset(LIST->WTP[0]->WTPIP,0,strlen(wtpip)+1);
			memcpy(LIST->WTP[0]->WTPIP,wtpip,strlen(wtpip));
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(LIST->WTP[0]->WFR_Index));	
//printf("eeeeeeeeeeffffffffffffffffffffff \n");
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(LIST->WTP[0]->WTPMAC[0]));	
//printf("eeeeeeeeeeeddddddddddddddddddddddddddd\n");
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(LIST->WTP[0]->WTPMAC[1]));	
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(LIST->WTP[0]->WTPMAC[2]));	
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(LIST->WTP[0]->WTPMAC[3]));	
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(LIST->WTP[0]->WTPMAC[4]));	
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(LIST->WTP[0]->WTPMAC[5]));	
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(LIST->WTP[0]->CTR_ID));
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(LIST->WTP[0]->DAT_ID));	

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&sysver);/*(LIST->WTP[0]->sysver)*/	

			LIST->WTP[0]->sysver = (char*)malloc(strlen(sysver)+1);
			memset(LIST->WTP[0]->sysver,0,strlen(sysver)+1);
			memcpy(LIST->WTP[0]->sysver,sysver,strlen(sysver));

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&version);/*(LIST->WTP[0]->ver)*/	

			LIST->WTP[0]->ver = (char*)malloc(strlen(version)+1);
			memset(LIST->WTP[0]->ver,0,strlen(version)+1);
			memcpy(LIST->WTP[0]->ver,version,strlen(version));
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&updatepath);/*(LIST->WTP[0]->updatepath)*/	

			LIST->WTP[0]->updatepath = (char*)malloc(strlen(updatepath)+1);
			memset(LIST->WTP[0]->updatepath,0,strlen(updatepath)+1);
			memcpy(LIST->WTP[0]->updatepath,updatepath,strlen(updatepath));
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&updateversion);/*(LIST->WTP[0]->updateversion)*/			

			LIST->WTP[0]->updateversion = (char*)malloc(strlen(updateversion)+1);
			memset(LIST->WTP[0]->updateversion,0,strlen(updateversion)+1);
			memcpy(LIST->WTP[0]->updateversion,updateversion,strlen(updateversion));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&apply_interface_name);/*(LIST->WTP[0]->apply_interface_name)*/	

			LIST->WTP[0]->apply_interface_name = (char*)malloc(strlen(apply_interface_name)+1);
			memset(LIST->WTP[0]->apply_interface_name,0,strlen(apply_interface_name)+1);
			memcpy(LIST->WTP[0]->apply_interface_name,apply_interface_name,strlen(apply_interface_name));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(LIST->WTP[0]->isused));	
		
		/*added by weiay 20081011*/
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(LIST->WTP[0]->quitreason));	
//printf("iiiiiiiiiiiiiiiiiiiiiiiiiii \n");
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(LIST->WTP[0]->apply_wlan_num));	
//printf("jjjjjjjjjjjjjjjjjjjjjjjjj \n");


		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
//printf("WTP->apply_wlan_num is %d \n",LIST->WTP[0]->apply_wlan_num);
////printf("WTP->apply_wlan_num is## %s\n",bwlannum);
		//num2 = WTP->apply_wlan_num;
		for (i = 0; i < LIST->WTP[0]->apply_wlan_num; i++)
		{	
	//		struct wlanid *wlanid[i] = NULL;
	//		wlanid[i] = (WID_WTP_RADIO*)malloc(sizeof(struct wlanid));
//printf("zzzzzzzzzzzzzzzzzzzzzzj \n");
			DBusMessageIter iter_struct;
			dbus_message_iter_recurse(&iter_array,&iter_struct);
						
			dbus_message_iter_get_basic(&iter_struct,&LIST->WTP[0]->apply_wlanid[i]);
//printf("xxxxxxxxxxxxxxxxxxxxxxxxxxx \n");
			dbus_message_iter_next(&iter_array);

	//		dcli_wtp_add_wlanid_node(LIST->WTP->Wlan_Id,wlanid[i]);
		}
//printf("cccccccccccccccccccccccccccccc \n");
		/*added end*/

		char r_num = 0;
		dbus_message_iter_next(&iter);			
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->radio_num);	
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
//printf("vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv r_num %d \n",LIST->WTP[0]->radio_num);
		num4 = LIST->WTP[0]->radio_num;
		for (i = 0; i < LIST->WTP[0]->radio_num; i++) {
			DBusMessageIter iter_struct;
			//WID_WTP_RADIO *radio[L_RADIO_NUM];
		//	LIST->WTP->WTP_Radio[L_RADIO_NUM]
			LIST->WTP[0]->WTP_Radio[i] = (WID_WTP_RADIO*)malloc(sizeof(WID_WTP_RADIO));
		//	LIST->WTP->WTP_Radio[i] = radio[i];
//printf("bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb \n");
			dbus_message_iter_recurse(&iter_array,&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(LIST->WTP[0]->WTP_Radio[i]->Radio_L_ID));
//printf("ssssssssssssssssssssssssssssssssssssssssss \n");
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(LIST->WTP[0]->WTP_Radio[i]->Radio_G_ID));
//printf("dddddddddddddddddddddddddddddddddd \n");
			dbus_message_iter_next(&iter_struct);
	
			dbus_message_iter_get_basic(&iter_struct,&(LIST->WTP[0]->WTP_Radio[i]->Radio_Type));
		
			dbus_message_iter_next(&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(LIST->WTP[0]->WTP_Radio[i]->Radio_Chan));
					
			dbus_message_iter_next(&iter_struct);
					
			dbus_message_iter_get_basic(&iter_struct,&(LIST->WTP[0]->WTP_Radio[i]->Radio_TXP));

			dbus_message_iter_next(&iter_array);
		}
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(LIST->WTP[0]->rx_bytes));	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(LIST->WTP[0]->tx_bytes));	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(LIST->WTP[0]->apstatisticsinterval)); 
//printf("eeeeeeeeeeeeeeeeeeeeeeeeeeeeee \n");
	//	memcpy(LIST->WTP->WTPMAC,WTPMAC,6);
//printf("eeeeeeeeeeessssssssssssssssssssssssssss \n");
	}
	else if(dcli_sn == 2){
//printf("dcli_sn == 2 \n");
		int num = 0;
		CW_CREATE_OBJECT_ERR(LIST->WTP_INFO, WID_WTP_INFO, return NULL;);	
		LIST->WTP_INFO->WTP_LIST = NULL;
		LIST->WTP_INFO->list_len = 0;
		WID_WTP **WTP;	
	//	WTP = malloc(WTP_NUM*(sizeof(WID_WTP *)));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->num);
	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);

		
		WTP = malloc((LIST->num)*(sizeof(WID_WTP *)));
		LIST->WTP = WTP;
		for (i = 0; i < LIST->num; i++) {
			DBusMessageIter iter_struct;
			
			WTP[i] = (WID_WTP*)malloc(sizeof(WID_WTP));
			WTP[i]->WTPMAC = (unsigned char *)malloc(MAC_LEN +1);
			memset(WTP[i]->WTPMAC,0,(MAC_LEN +1));
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WTP[i]->WTPID));
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&WTP[i]->WTPMAC[0]);
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&WTP[i]->WTPMAC[1]);
			
			dbus_message_iter_next(&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&WTP[i]->WTPMAC[2]);
		
			dbus_message_iter_next(&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&WTP[i]->WTPMAC[3]);
		
			dbus_message_iter_next(&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&WTP[i]->WTPMAC[4]);
		
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&WTP[i]->WTPMAC[5]);	
		
			dbus_message_iter_next(&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(wtpip));
			WTP[i]->WTPIP = (char*)malloc(strlen(wtpip)+1);
			memset(WTP[i]->WTPIP,0,strlen(wtpip)+1);
			memcpy(WTP[i]->WTPIP,wtpip,strlen(wtpip));

			dbus_message_iter_next(&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&wtpmodel);/*(WTP[i]->WTPModel)*/

				WTP[i]->WTPModel = (char*)malloc(strlen(wtpmodel)+1);
				memset(WTP[i]->WTPModel,0,strlen(wtpmodel)+1);
				memcpy(WTP[i]->WTPModel,wtpmodel,strlen(wtpmodel));

			dbus_message_iter_next(&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(WTP[i]->WTPStat));

			dbus_message_iter_next(&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(WTP[i]->isused));

			dbus_message_iter_next(&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&wtpname);/*(WTP[i]->WTPNAME)*/

				WTP[i]->WTPNAME = (char*)malloc(strlen(wtpname)+1);
				memset(WTP[i]->WTPNAME,0,strlen(wtpname)+1);
				memcpy(WTP[i]->WTPNAME,wtpname,strlen(wtpname));
			/*fengwenchao modify begin 20110530*/
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&wtpsn);

			WTP[i]->WTPSN= (char*)malloc(strlen(wtpsn)+1);
			memset(WTP[i]->WTPSN,0,strlen(wtpsn)+1);
			memcpy(WTP[i]->WTPSN,wtpsn,strlen(wtpsn));
			/*fengwenchao modify end*/
			dbus_message_iter_next(&iter_array);
			if(WTP[i]->WTPStat == 5)
			{
				LIST->TotalNum++;
			}
			switch(WTP[i]->WTPStat)
			{	
				case 2	:	LIST->join_num++;
							break;
				case 3	:	LIST->configure_num++;
							break;
				case 4	:	LIST->datacheck_num++;
							break;
				case 5	:	LIST->run_num++;
							break;	
				case 7	:	LIST->quit_num++;
							break;
				case 8	:	LIST->imagedata_num++;
							break;
				case 9	:	LIST->bak_run_num++;
							break;
				default	:	break;
			}
			dcli_wtp_add_wtp_node(&LIST->WTP_INFO,WTP[i]);
		}
/*		int j = 0;		
		if((LIST != NULL)&&(LIST->WTP_INFO != NULL)&&(LIST->WTP_INFO->WTP_LIST)){
			WID_WTP *head = NULL;
			head = LIST->WTP_INFO->WTP_LIST;
			for(j=0;j<2;j++){
				if(head != NULL)
					//printf("4444444444444 wptid is %d \n",head->WTPID);
				head = head->next;
			}
		
			//printf("3333333333333333 wptid is %d \n",LIST->WTP_INFO->WTP_LIST->WTPID);
		}*/
	}
	else if(dcli_sn == 3){
		//printf("dcli_sn == 3 \n");
		WID_WTP **WTP;	
		CW_CREATE_OBJECT_ERR(LIST->WTP_INFO, WID_WTP_INFO, return NULL;);	
		LIST->WTP_INFO->WTP_LIST = NULL;
		LIST->WTP_INFO->list_len = 0;
		//WTP = malloc(WTP_NUM*(sizeof(WID_WTP *)));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->num);
	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);

		WTP = malloc((LIST->num)*(sizeof(WID_WTP *)));
		LIST->WTP = WTP;
		//printf("num is %d \n",LIST->num);
		for (i = 0; i < LIST->num; i++) {
			DBusMessageIter iter_struct;
			char *wtpname;
			
			WTP[i] = (WID_WTP*)malloc(sizeof(WID_WTP));
			WTP[i]->WTPMAC = (unsigned char *)malloc(MAC_LEN +1);
			memset(WTP[i]->WTPMAC,0,(MAC_LEN +1));

			dbus_message_iter_recurse(&iter_array,&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(WTP[i]->WTPID));
		
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&wtpsn);/*(WTP[i]->WTPSN)*/

				WTP[i]->WTPSN = (char*)malloc(strlen(wtpsn)+1);
				memset(WTP[i]->WTPSN,0,strlen(wtpsn)+1);
				memcpy(WTP[i]->WTPSN,wtpsn,strlen(wtpsn));
		
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&wtpname);/*(WTP[i]->WTPNAME)*/

				WTP[i]->WTPNAME = (char*)malloc(strlen(wtpname)+1);
				memset(WTP[i]->WTPNAME,0,strlen(wtpname)+1);
				memcpy(WTP[i]->WTPNAME,wtpname,strlen(wtpname));

			dbus_message_iter_next(&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&wtpmodel);/*(WTP[i]->WTPModel)*/

				WTP[i]->WTPModel = (char*)malloc(strlen(wtpmodel)+1);
				memset(WTP[i]->WTPModel,0,strlen(wtpmodel)+1);
				memcpy(WTP[i]->WTPModel,wtpmodel,strlen(wtpmodel));

			dbus_message_iter_next(&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(WTP[i]->RadioCount));

			dbus_message_iter_next(&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(WTP[i]->WFR_Index));

			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(WTP[i]->WTPStat));
			if(WTP[i]->WTPStat == 5)
				LIST->TotalNum++;
			dbus_message_iter_next(&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(WTP[i]->CTR_ID));

			dbus_message_iter_next(&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(WTP[i]->DAT_ID));

			dbus_message_iter_next(&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(WTP[i]->isused));

			dbus_message_iter_next(&iter_array);

			switch(WTP[i]->WTPStat)
			{	
				case 2	:	LIST->join_num++;
							break;
				case 3	:	LIST->configure_num++;
							break;
				case 4	:	LIST->datacheck_num++;
							break;
				case 5	:	LIST->run_num++;
							break;	
				case 7	:	LIST->quit_num++;
							break;
				case 8	:	LIST->imagedata_num++;
							break;
				case 9	:	LIST->bak_run_num++;
							break;
				default	:	break;
			}
			dcli_wtp_add_wtp_node(&LIST->WTP_INFO,WTP[i]);
		}
	}
	else if(dcli_sn == 4){
			//printf("dcli_sn == 3 \n");
			WID_WTP **WTP;	
			CW_CREATE_OBJECT_ERR(LIST->WTP_INFO, WID_WTP_INFO, return NULL;);	
			LIST->WTP_INFO->WTP_LIST = NULL;
			LIST->WTP_INFO->list_len = 0;
		//	WTP = malloc(WTP_NUM*(sizeof(WID_WTP *)));
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->num);
		
			dbus_message_iter_next(&iter);	
			dbus_message_iter_recurse(&iter,&iter_array);

			WTP = malloc((LIST->num)*(sizeof(WID_WTP *)));
			LIST->WTP = WTP;
			//printf("num is %d \n",LIST->num);
			for (i = 0; i < LIST->num; i++) {
				DBusMessageIter iter_struct;
				
				WTP[i] = (WID_WTP*)malloc(sizeof(WID_WTP));
				WTP[i]->WTPMAC = (unsigned char *)malloc(MAC_LEN +1);
				memset(WTP[i]->WTPMAC,0,(MAC_LEN +1));
				dbus_message_iter_recurse(&iter_array,&iter_struct);
			
				dbus_message_iter_get_basic(&iter_struct,&(WTP[i]->WTPID));
			
				
				dbus_message_iter_next(&iter_struct);	
				
				dbus_message_iter_get_basic(&iter_struct,&(WTP[i]->WTPMAC[0])); 
				dbus_message_iter_next(&iter_struct);	
				
				dbus_message_iter_get_basic(&iter_struct,&(WTP[i]->WTPMAC[1])); 			
				dbus_message_iter_next(&iter_struct);	
				
				dbus_message_iter_get_basic(&iter_struct,&(WTP[i]->WTPMAC[2])); 	
				dbus_message_iter_next(&iter_struct);
	
				dbus_message_iter_get_basic(&iter_struct,&(WTP[i]->WTPMAC[3])); 
				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct,&(WTP[i]->WTPMAC[4])); 
				dbus_message_iter_next(&iter_struct);	
				
				dbus_message_iter_get_basic(&iter_struct,&(WTP[i]->WTPMAC[5])); 
				dbus_message_iter_next(&iter_struct);	
	
				dbus_message_iter_get_basic(&iter_struct,&wtpname);/*(WTP[i]->WTPNAME)*/

					WTP[i]->WTPNAME = (char*)malloc(strlen(wtpname)+1);
					memset(WTP[i]->WTPNAME,0,strlen(wtpname)+1);
					memcpy(WTP[i]->WTPNAME,wtpname,strlen(wtpname));
			
				dbus_message_iter_next(&iter_struct);
			
				dbus_message_iter_get_basic(&iter_struct,&wtpmodel);/*(WTP[i]->WTPModel)*/

					WTP[i]->WTPModel = (char*)malloc(strlen(wtpmodel)+1);
					memset(WTP[i]->WTPModel,0,strlen(wtpmodel)+1);
					memcpy(WTP[i]->WTPModel,wtpmodel,strlen(wtpmodel));
	
				dbus_message_iter_next(&iter_struct);
			
				dbus_message_iter_get_basic(&iter_struct,&(WTP[i]->RadioCount));
	
				dbus_message_iter_next(&iter_struct);
			
				dbus_message_iter_get_basic(&iter_struct,&(WTP[i]->WFR_Index));
	
				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct,&(WTP[i]->WTPStat));
				if(WTP[i]->WTPStat == 5)
					LIST->TotalNum++;
				dbus_message_iter_next(&iter_struct);
			
				dbus_message_iter_get_basic(&iter_struct,&(WTP[i]->CTR_ID));
	
				dbus_message_iter_next(&iter_struct);
			
				dbus_message_iter_get_basic(&iter_struct,&(WTP[i]->DAT_ID));
	
				dbus_message_iter_next(&iter_struct);
			
				dbus_message_iter_get_basic(&iter_struct,&(WTP[i]->isused));
	
				dbus_message_iter_next(&iter_array);
	
				switch(WTP[i]->WTPStat)
				{	
					case 2	:	LIST->join_num++;
								break;
					case 3	:	LIST->configure_num++;
								break;
					case 4	:	LIST->datacheck_num++;
								break;
					case 5	:	LIST->run_num++;
								break;	
					case 7	:	LIST->quit_num++;
								break;
					case 8	:	LIST->imagedata_num++;
								break;
					case 9	:	LIST->bak_run_num++;
								break;
					default :	break;
				}
				dcli_wtp_add_wtp_node(&LIST->WTP_INFO,WTP[i]);
			}
	}
	else if (dcli_sn == 5){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->num);
	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);

		//AP_VERSION_ELE **AP_VERSION = NULL;
		LIST->AP_VERSION = malloc((LIST->num)*(sizeof(struct AP_VERSION*)));

		for (i = 0; i < LIST->num; i++) {
			DBusMessageIter iter_struct;
			//AP_VERSION_ELE **AP_VERSION = NULL;
			LIST->AP_VERSION[i] = (struct AP_VERSION*)malloc(sizeof(struct AP_VERSION));
			//CW_CREATE_OBJECT_ERR(AP_VERSION, AP_VERSION_ELE, return NULL;);		
			dbus_message_iter_recurse(&iter_array,&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(wtpmodel));	
			LIST->AP_VERSION[i]->apmodel = (char *)malloc(strlen(wtpmodel)+1);
			memset(LIST->AP_VERSION[i]->apmodel,0,(strlen(wtpmodel)+1));
			memcpy(LIST->AP_VERSION[i]->apmodel,wtpmodel,strlen(wtpmodel));
			
			dbus_message_iter_next(&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(version));
			LIST->AP_VERSION[i]->versionname= (char *)malloc(strlen(version)+1);
			memset(LIST->AP_VERSION[i]->versionname,0,(strlen(version)+1));
			memcpy(LIST->AP_VERSION[i]->versionname,version,strlen(version));
		
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(updatepath));
			LIST->AP_VERSION[i]->versionpath= (char *)malloc(strlen(updatepath)+1);
			memset(LIST->AP_VERSION[i]->versionpath,0,(strlen(updatepath)+1));
			memcpy(LIST->AP_VERSION[i]->versionpath,updatepath,strlen(updatepath));
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(LIST->AP_VERSION[i]->radionum));	

			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(LIST->AP_VERSION[i]->bssnum));	
			
			dbus_message_iter_next(&iter_array);
		}
	}
	else if(dcli_sn == 6){
		//LIST->WTP = (WID_WTP*)malloc(WTP_NUM*sizeof(WID_WTP*));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->num);
	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);

		LIST->WTP = (WID_WTP*)malloc((LIST->num)*sizeof(WID_WTP*));
		for (i = 0; i < LIST->num; i++) {
			DBusMessageIter iter_struct;
			
			LIST->WTP[i] = (WID_WTP*)malloc(sizeof(WID_WTP));
			LIST->WTP[i]->WTPMAC = (unsigned char *)malloc(MAC_LEN +1);
			memset(LIST->WTP[i]->WTPMAC,0,(MAC_LEN +1));
			dbus_message_iter_recurse(&iter_array,&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(LIST->WTP[i]->WTPID));
		
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&LIST->WTP[i]->WTPMAC[0]);
			
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&LIST->WTP[i]->WTPMAC[1]);
			
			dbus_message_iter_next(&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&LIST->WTP[i]->WTPMAC[2]);
		
			dbus_message_iter_next(&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&LIST->WTP[i]->WTPMAC[3]);
		
			dbus_message_iter_next(&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&LIST->WTP[i]->WTPMAC[4]);
		
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&LIST->WTP[i]->WTPMAC[5]);	
		
			dbus_message_iter_next(&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(wtpip));
			LIST->WTP[i]->WTPIP = (char*)malloc(strlen(wtpip)+1);
			memset(LIST->WTP[i]->WTPIP,0,strlen(wtpip)+1);
			memcpy(LIST->WTP[i]->WTPIP,wtpip,strlen(wtpip));

			dbus_message_iter_next(&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&wtpmodel);/*(LIST->WTP[i]->WTPModel)*/

				LIST->WTP[i]->WTPModel = (char*)malloc(strlen(wtpmodel)+1);
				memset(LIST->WTP[i]->WTPModel,0,strlen(wtpmodel)+1);
				memcpy(LIST->WTP[i]->WTPModel,wtpmodel,strlen(wtpmodel));

			dbus_message_iter_next(&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(LIST->WTP[i]->WTPStat));

			dbus_message_iter_next(&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(LIST->WTP[i]->updatefailstate));
			
			dbus_message_iter_next(&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(LIST->WTP[i]->updatefailcount));			

			dbus_message_iter_next(&iter_array);

			if(LIST->WTP[i]->WTPStat == 5)
			{
				LIST->TotalNum++;
			}
		}
	}
	else if(dcli_sn == 7){
		//LIST->WTP = (WID_WTP *)malloc(WTP_NUM*sizeof(WID_WTP *));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->num);
	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);

		LIST->WTP = (WID_WTP *)malloc((LIST->num)*sizeof(WID_WTP *));
		for (i = 0; i < LIST->num; i++) {
			DBusMessageIter iter_struct;
			unsigned int ip;
			LIST->WTP[i] = (WID_WTP*)malloc(sizeof(WID_WTP));
			LIST->WTP[i]->WTPMAC = (unsigned char *)malloc(MAC_LEN +1);
			dbus_message_iter_recurse(&iter_array,&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(ip));
		
				LIST->WTP[i]->WTPIP = (char*)malloc(4+1);
				memset(LIST->WTP[i]->WTPIP,0,5);
				memcpy(LIST->WTP[i]->WTPIP,&ip,4);
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(LIST->WTP[i]->WTPMAC[0]));
		
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(LIST->WTP[i]->WTPMAC[1]));
		
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(LIST->WTP[i]->WTPMAC[2]));
		
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(LIST->WTP[i]->WTPMAC[3]));
		
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(LIST->WTP[i]->WTPMAC[4]));
		
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(LIST->WTP[i]->WTPMAC[5]));
		
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&wtpmodel);/*(LIST->WTP[i]->WTPModel)*/

				LIST->WTP[i]->WTPModel = (char*)malloc(strlen(wtpmodel)+1);
				memset(LIST->WTP[i]->WTPModel,0,strlen(wtpmodel)+1);
				memcpy(LIST->WTP[i]->WTPModel,wtpmodel,strlen(wtpmodel));
		
			dbus_message_iter_next(&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&apcode);/*(LIST->WTP[i]->APCode)*/

				LIST->WTP[i]->APCode = (char*)malloc(strlen(apcode)+1);
				memset(LIST->WTP[i]->APCode,0,strlen(apcode)+1);
				memcpy(LIST->WTP[i]->APCode,apcode,strlen(apcode));

			dbus_message_iter_next(&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&wtpsn);/*(LIST->WTP[i]->WTPSN)*/

				LIST->WTP[i]->WTPSN = (char*)malloc(strlen(wtpsn)+1);
				memset(LIST->WTP[i]->WTPSN,0,strlen(wtpsn)+1);
				memcpy(LIST->WTP[i]->WTPSN,wtpsn,strlen(wtpsn));

			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&apply_interface_name);/*(LIST->WTP[i]->apply_interface_name)*/

				LIST->WTP[i]->apply_interface_name = (char*)malloc(strlen(apply_interface_name)+1);
				memset(LIST->WTP[i]->apply_interface_name,0,strlen(apply_interface_name)+1);
				memcpy(LIST->WTP[i]->apply_interface_name,apply_interface_name,strlen(apply_interface_name));

			dbus_message_iter_next(&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&version);/*(LIST->WTP[i]->ver)*/

				LIST->WTP[i]->ver = (char*)malloc(strlen(version)+1);
				memset(LIST->WTP[i]->ver,0,strlen(version)+1);
				memcpy(LIST->WTP[i]->ver,version,strlen(version));
			
			dbus_message_iter_next(&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&codever);/*(LIST->WTP[i]->codever)*/			

				LIST->WTP[i]->codever = (char*)malloc(strlen(codever)+1);
				memset(LIST->WTP[i]->codever,0,strlen(codever)+1);
				memcpy(LIST->WTP[i]->codever,codever,strlen(codever));
			dbus_message_iter_next(&iter_array);
		//LIST->WTP[i]->WTPIP = &ip;
		}
	}
	else if(dcli_sn == 8){	

		LIST->WTP = (WID_WTP*)malloc(sizeof(WID_WTP*));
		LIST->WTP[0] = (WID_WTP*)malloc(sizeof(WID_WTP));
//printf("(dcli_sn == 8)  \n");
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&wtpmodel);/*(LIST->WTP[0]->WTPModel)*/	

			LIST->WTP[0]->WTPModel = (char*)malloc(strlen(wtpmodel)+1);
			memset(LIST->WTP[0]->WTPModel,0,strlen(wtpmodel)+1);
			memcpy(LIST->WTP[0]->WTPModel,wtpmodel,strlen(wtpmodel));

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&apcode);	/*(LIST->WTP[0]->APCode)*/

			LIST->WTP[0]->APCode = (char*)malloc(strlen(apcode)+1);
			memset(LIST->WTP[0]->APCode,0,strlen(apcode)+1);
			memcpy(LIST->WTP[0]->APCode,apcode,strlen(apcode));
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&version);/*(LIST->WTP[0]->ver)*/	

			LIST->WTP[0]->ver = (char*)malloc(strlen(version)+1);
			memset(LIST->WTP[0]->ver,0,strlen(version)+1);
			memcpy(LIST->WTP[0]->ver,version,strlen(version));

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&codever);/*(LIST->WTP[0]->codever)*/	

			LIST->WTP[0]->codever = (char*)malloc(strlen(codever)+1);
			memset(LIST->WTP[0]->codever,0,strlen(codever)+1);
			memcpy(LIST->WTP[0]->codever,codever,strlen(codever));

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(LIST->WTP[0]->img_now_state));	
	
	}
	else if (dcli_sn == 9){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->num);
	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
		
		LIST->WTP = (WID_WTP *)malloc((LIST->num)*sizeof(WID_WTP *));
		for (i = 0; i < LIST->num; i++) {
			DBusMessageIter iter_struct;
			
			LIST->WTP[i] = (WID_WTP*)malloc(sizeof(WID_WTP));
			LIST->WTP[i]->WTPMAC = (unsigned char *)malloc(MAC_LEN +1);
			memset(LIST->WTP[i]->WTPMAC,0,(MAC_LEN +1));
			dbus_message_iter_recurse(&iter_array,&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(LIST->WTP[i]->WTPID));
		
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&LIST->WTP[i]->WTPMAC[0]);
			
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&LIST->WTP[i]->WTPMAC[1]);
			
			dbus_message_iter_next(&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&LIST->WTP[i]->WTPMAC[2]);
		
			dbus_message_iter_next(&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&LIST->WTP[i]->WTPMAC[3]);
		
			dbus_message_iter_next(&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&LIST->WTP[i]->WTPMAC[4]);
		
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&LIST->WTP[i]->WTPMAC[5]);	
		
			dbus_message_iter_next(&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&wtpip);/*(LIST->WTP[i]->WTPIP)*/

				LIST->WTP[i]->WTPIP = (char*)malloc(strlen(wtpip)+1);
				memset(LIST->WTP[i]->WTPIP,0,strlen(wtpip)+1);
				memcpy(LIST->WTP[i]->WTPIP,wtpip,strlen(wtpip));

			dbus_message_iter_next(&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&wtpmodel);/*(LIST->WTP[i]->WTPModel)*/

				LIST->WTP[i]->WTPModel = (char*)malloc(strlen(wtpmodel)+1);
				memset(LIST->WTP[i]->WTPModel,0,strlen(wtpmodel)+1);
				memcpy(LIST->WTP[i]->WTPModel,wtpmodel,strlen(wtpmodel));

			dbus_message_iter_next(&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(LIST->WTP[i]->WTPStat));

			dbus_message_iter_next(&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(LIST->WTP[i]->updateStat));
			
			dbus_message_iter_next(&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(LIST->WTP[i]->manual_update_time));			

			dbus_message_iter_next(&iter_array);

			if(LIST->WTP[i]->WTPStat == 5)
			{
				LIST->TotalNum++;
			}
		}
	}
	else if(dcli_sn == 10){
		//printf("dcli_sn == 10 \n");
		int num = 0;
		CW_CREATE_OBJECT_ERR(LIST->WTP_INFO, WID_WTP_INFO, return NULL;);	
		LIST->WTP_INFO->WTP_LIST = NULL;
		LIST->WTP_INFO->list_len = 0;
		WID_WTP **WTP;	
	//	WTP = malloc(WTP_NUM*(sizeof(WID_WTP *)));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->num);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
		
		WTP = (WID_WTP *)malloc((LIST->num)*(sizeof(WID_WTP *)));
		for (i = 0; i < LIST->num; i++) {
			DBusMessageIter iter_struct;
			WTP[i] = (WID_WTP*)malloc(sizeof(WID_WTP));
			WTP[i]->WTPMAC = (unsigned char *)malloc(MAC_LEN +1);
			memset(WTP[i]->WTPMAC,0,(MAC_LEN +1));

			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WTP[i]->WTPID));

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&WTP[i]->WTPMAC[0]);
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&WTP[i]->WTPMAC[1]);
			
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&WTP[i]->WTPMAC[2]);
		
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&WTP[i]->WTPMAC[3]);
		
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&WTP[i]->WTPMAC[4]);
		
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&WTP[i]->WTPMAC[5]);	

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(wtpip));
				WTP[i]->WTPIP= (char*)malloc(strlen(wtpip)+1);
				memset(WTP[i]->WTPIP,0,strlen(wtpip)+1);
				memcpy(WTP[i]->WTPIP,wtpip,strlen(wtpip));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&wtpsn);/*(WTP[i]->WTPSN)*/

				WTP[i]->WTPSN = (char*)malloc(strlen(wtpsn)+1);
				memset(WTP[i]->WTPSN,0,strlen(wtpsn)+1);
				memcpy(WTP[i]->WTPSN,wtpsn,strlen(wtpsn));

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&apply_interface_name);/*(LIST->WTP[i]->apply_interface_name)*/

				WTP[i]->apply_interface_name = (char*)malloc(strlen(apply_interface_name)+1);
				memset(WTP[i]->apply_interface_name,0,strlen(apply_interface_name)+1);
				memcpy(WTP[i]->apply_interface_name,apply_interface_name,strlen(apply_interface_name));

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&wtpip);/*(LIST->WTP[i]->apply_interface_name)*/
			
				//WTP[i]->login_interfaceIP= (char*)malloc(strlen(wtpip)+1);
				memset(WTP[i]->login_interfaceIP,0,strlen(wtpip)+1);
				memcpy(WTP[i]->login_interfaceIP,wtpip,strlen(wtpip));

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&wtpmodel);/*(WTP[i]->WTPModel)*/

				WTP[i]->WTPModel = (char*)malloc(strlen(wtpmodel)+1);
				memset(WTP[i]->WTPModel,0,strlen(wtpmodel)+1);
				memcpy(WTP[i]->WTPModel,wtpmodel,strlen(wtpmodel));

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WTP[i]->WTPStat));

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WTP[i]->isused));

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&wtpname);/*(WTP[i]->WTPNAME)*/

				WTP[i]->WTPNAME = (char*)malloc(strlen(wtpname)+1);
				memset(WTP[i]->WTPNAME,0,strlen(wtpname)+1);
				memcpy(WTP[i]->WTPNAME,wtpname,strlen(wtpname));

			dbus_message_iter_next(&iter_array);
			if(WTP[i]->WTPStat == 5)
			{
				LIST->TotalNum++;
			}
			switch(WTP[i]->WTPStat)
			{	
				case 2	:	LIST->join_num++;
							break;
				case 3	:	LIST->configure_num++;
							break;
				case 4	:	LIST->datacheck_num++;
							break;
				case 5	:	LIST->run_num++;
							break;	
				case 7	:	LIST->quit_num++;
							break;
				case 8	:	LIST->imagedata_num++;
							break;
				case 9	:	LIST->bak_run_num++;
							break;
				default	:	break;
			}
			dcli_wtp_add_wtp_node(&LIST->WTP_INFO,WTP[i]);
		}
		CW_FREE_OBJECT(WTP);

	}
	//fengwenchao add 20110226
	else if(dcli_sn == 11){
		
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->num);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_recurse(&iter,&iter_array);
			
			LIST->WTP_M_V = (struct WTP_MODEL_VERSION *)malloc((LIST->num)*sizeof(struct WTP_MODEL_VERSION *));
			LIST->WTP = (WID_WTP *)malloc((LIST->num)*sizeof(WID_WTP *));
			int j = 0;
			
			for (i = 0; i < LIST->num; i++) {
				
			DBusMessageIter iter_struct;
					
			LIST->WTP[i] = (WID_WTP*)malloc(sizeof(WID_WTP));
			LIST->WTP_M_V[i] = (struct WTP_MODEL_VERSION *)malloc(sizeof(struct WTP_MODEL_VERSION));
			LIST->WTP_M_V[i]->wtp_model_num = 0;
			//LIST->WTP_M_V[i]->wtpid_flag = 0;
			LIST->WTP[i]->WTPMAC = (unsigned char *)malloc(MAC_LEN +1);
			memset(LIST->WTP[i]->WTPMAC,0,(MAC_LEN +1));
			
			dbus_message_iter_recurse(&iter_array,&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(LIST->WTP[i]->WTPID));
		
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&LIST->WTP[i]->WTPMAC[0]);
			
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&LIST->WTP[i]->WTPMAC[1]);
			
			dbus_message_iter_next(&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&LIST->WTP[i]->WTPMAC[2]);
		
			dbus_message_iter_next(&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&LIST->WTP[i]->WTPMAC[3]);
		
			dbus_message_iter_next(&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&LIST->WTP[i]->WTPMAC[4]);
		
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&LIST->WTP[i]->WTPMAC[5]); 

			dbus_message_iter_next(&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&wtpmodel);/*(LIST->WTP[i]->WTPModel)*/

			LIST->WTP[i]->WTPModel = (char*)malloc(strlen(wtpmodel)+1);
			memset(LIST->WTP[i]->WTPModel,0,strlen(wtpmodel)+1);
			memcpy(LIST->WTP[i]->WTPModel,wtpmodel,strlen(wtpmodel));

			//printf("LIST->WTP[%d]->WTPModel = %s \n",i,LIST->WTP[i]->WTPModel);
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&apcode);	

			LIST->WTP[i]->APCode = (char*)malloc(strlen(apcode)+1);
			memset(LIST->WTP[i]->APCode,0,strlen(apcode)+1);
			memcpy(LIST->WTP[i]->APCode,apcode,strlen(apcode));
			//printf("LIST->WTP[%d]->APCode = %s \n",i,LIST->WTP[i]->APCode);
		
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&version);	

			LIST->WTP[i]->ver = (char*)malloc(strlen(version)+1);
			memset(LIST->WTP[i]->ver,0,strlen(version)+1);
			memcpy(LIST->WTP[i]->ver,version,strlen(version));
			//printf("LIST->WTP[%d]->ver = %s \n",i,LIST->WTP[i]->ver);

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&codever);

			LIST->WTP[i]->codever = (char*)malloc(strlen(codever)+1);
			memset(LIST->WTP[i]->codever,0,strlen(codever)+1);
			memcpy(LIST->WTP[i]->codever,codever,strlen(codever));
			//printf("LIST->WTP[%d]->codever = %s \n",i,LIST->WTP[i]->codever);

			dbus_message_iter_next(&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(LIST->WTP[i]->WTPStat));	

			dbus_message_iter_next(&iter_struct);
	
			dbus_message_iter_get_basic(&iter_struct,&(LIST->WTP[i]->isused));		

			dbus_message_iter_next(&iter_array);

			//printf("11111111111111111111111111111111 \n");
			if(LIST->WTP[i]->WTPStat == 5)
			{
				LIST->TotalNum++;
			}

			LIST->WTP_M_V[i]->wtp_model = NULL;

		//	memset(LIST->WTP_M_V[i]->wtpid_group,0, 2048);
			if(i == 0)
			{
			//printf("2222222222222222222222222222222 \n");
				LIST->WTP_M_V[i]->wtp_model = (char*)malloc(strlen(wtpmodel)+1);
				memset(LIST->WTP_M_V[i]->wtp_model,0,strlen(wtpmodel)+1);
				memcpy(LIST->WTP_M_V[i]->wtp_model,wtpmodel,strlen(wtpmodel));
				LIST->wtp_model_type++;
				LIST->WTP_M_V[i]->wtp_model_num++;
				//int flag0 = 0;
				//flag0 = LIST->WTP_M_V[i]->wtpid_flag;
				//LIST->WTP_M_V[i]->wtpid_group[flag0] = LIST->WTP[i]->WTPID;
				//LIST->WTP_M_V[i]->wtpid_flag++;
			}
			else
			{
				int retmodel = 0;
				/*for(j = 0; j < i; j++)
				{
					printf("555555555555555555555555555555555 \n");
					if(!strcmp(wtpmodel,LIST->WTP_M_V[j]->wtp_model))
					{
						printf("666666666666666666666666666666 \n");
						LIST->WTP_M_V[j]->wtp_model_num++;
						retmodel = 1;
						break;
					}
				}*/
				for(j = 0; j < (LIST->wtp_model_type);j++)
				{
					//printf("33333333333333333333333 \n");
					if(!strcmp(wtpmodel,LIST->WTP_M_V[j]->wtp_model))
					{
						LIST->WTP_M_V[j]->wtp_model_num++;
						retmodel = 1;
						//int flag1 = 0;
						//flag1 = LIST->WTP_M_V[j]->wtpid_flag;
						//LIST->WTP_M_V[j]->wtpid_group[flag1] = LIST->WTP[i]->WTPID;
						//LIST->WTP_M_V[j]->wtpid_flag++;
						//printf("3#3# \n");
						break;
					}
				}
				if(!retmodel)
				{
					//printf("44444444444444444444444 \n");
					LIST->WTP_M_V[LIST->wtp_model_type]->wtp_model = (char*)malloc(strlen(wtpmodel)+1);
					memset(LIST->WTP_M_V[LIST->wtp_model_type]->wtp_model,0,strlen(wtpmodel)+1);
					memcpy(LIST->WTP_M_V[LIST->wtp_model_type]->wtp_model,wtpmodel,strlen(wtpmodel));
					LIST->WTP_M_V[LIST->wtp_model_type]->wtp_model_num++;
					LIST->wtp_model_type++;
					//int flag = 0;
					//flag = LIST->WTP_M_V[LIST->wtp_model_type]->wtpid_flag;
					//printf("flag  =  %d  \n",flag);
					//LIST->WTP_M_V[LIST->wtp_model_type]->wtpid_group[flag] = LIST->WTP[i]->WTPID;
					//LIST->WTP_M_V[LIST->wtp_model_type]->wtpid_flag++;
				}
				/*if(!retmodel)
				{
					//printf("77777777777777777777777777777 \n");
					LIST->WTP_M_V[i]->wtp_model = (char*)malloc(strlen(wtpmodel)+1);
					memset(LIST->WTP_M_V[i]->wtp_model,0,strlen(wtpmodel)+1);
					memcpy(LIST->WTP_M_V[i]->wtp_model,wtpmodel,strlen(wtpmodel));
					LIST->WTP_M_V[i]->wtp_model_num++;
					LIST->wtp_model_type++;
				}*/
			}
		}
	   }
	//fengwenchap add end
	//fengwenchao add 20110314
	else if(dcli_sn == 12){
		
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->num);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_recurse(&iter,&iter_array);
			
			LIST->WTP_M_V = (struct WTP_MODEL_VERSION *)malloc((LIST->num)*sizeof(struct WTP_MODEL_VERSION *));
			LIST->WTP = (WID_WTP *)malloc((LIST->num)*sizeof(WID_WTP *));
			int j = 0;
			
			for (i = 0; i < LIST->num; i++) {
				
			DBusMessageIter iter_struct;
					
			LIST->WTP[i] = (WID_WTP*)malloc(sizeof(WID_WTP));
			LIST->WTP_M_V[i] = (struct WTP_MODEL_VERSION *)malloc(sizeof(struct WTP_MODEL_VERSION));
			LIST->WTP_M_V[i]->wtp_version_num = 0;
			LIST->WTP[i]->WTPMAC = (unsigned char *)malloc(MAC_LEN +1);
			memset(LIST->WTP[i]->WTPMAC,0,(MAC_LEN +1));
			
			dbus_message_iter_recurse(&iter_array,&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(LIST->WTP[i]->WTPID));
		
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&LIST->WTP[i]->WTPMAC[0]);
			
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&LIST->WTP[i]->WTPMAC[1]);
			
			dbus_message_iter_next(&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&LIST->WTP[i]->WTPMAC[2]);
		
			dbus_message_iter_next(&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&LIST->WTP[i]->WTPMAC[3]);
		
			dbus_message_iter_next(&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&LIST->WTP[i]->WTPMAC[4]);
		
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&LIST->WTP[i]->WTPMAC[5]); 

			dbus_message_iter_next(&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&wtpmodel);/*(LIST->WTP[i]->WTPModel)*/

			LIST->WTP[i]->WTPModel = (char*)malloc(strlen(wtpmodel)+1);
			memset(LIST->WTP[i]->WTPModel,0,strlen(wtpmodel)+1);
			memcpy(LIST->WTP[i]->WTPModel,wtpmodel,strlen(wtpmodel));

			//printf("LIST->WTP[%d]->WTPModel = %s \n",i,LIST->WTP[i]->WTPModel);
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&apcode);	

			LIST->WTP[i]->APCode = (char*)malloc(strlen(apcode)+1);
			memset(LIST->WTP[i]->APCode,0,strlen(apcode)+1);
			memcpy(LIST->WTP[i]->APCode,apcode,strlen(apcode));
			//printf("LIST->WTP[%d]->APCode = %s \n",i,LIST->WTP[i]->APCode);
		
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&version);	

			LIST->WTP[i]->ver = (char*)malloc(strlen(version)+1);
			memset(LIST->WTP[i]->ver,0,strlen(version)+1);
			memcpy(LIST->WTP[i]->ver,version,strlen(version));
			//printf("LIST->WTP[%d]->ver = %s \n",i,LIST->WTP[i]->ver);

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&codever);

			LIST->WTP[i]->codever = (char*)malloc(strlen(codever)+1);
			memset(LIST->WTP[i]->codever,0,strlen(codever)+1);
			memcpy(LIST->WTP[i]->codever,codever,strlen(codever));
			//printf("LIST->WTP[%d]->codever = %s \n",i,LIST->WTP[i]->codever);

			dbus_message_iter_next(&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(LIST->WTP[i]->WTPStat));	

			dbus_message_iter_next(&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(LIST->WTP[i]->isused));

			dbus_message_iter_next(&iter_array);


			if(LIST->WTP[i]->WTPStat == 5)
			{
				LIST->TotalNum++;
			}

			LIST->WTP_M_V[i]->wtp_version = NULL;
			if(i == 0)
			{
				LIST->WTP_M_V[i]->wtp_version = (char*)malloc(strlen(codever)+1);
				memset(LIST->WTP_M_V[i]->wtp_version,0,strlen(codever)+1);
				memcpy(LIST->WTP_M_V[i]->wtp_version,codever,strlen(codever));
				LIST->wtp_version_type++;
				LIST->WTP_M_V[i]->wtp_version_num++;
			}
			else
			{
				int retversion = 0;

				for(j = 0; j < (LIST->wtp_version_type);j++)
				{
					if(!strcmp(codever,LIST->WTP_M_V[j]->wtp_version))
					{
						LIST->WTP_M_V[j]->wtp_version_num++;
						retversion = 1;
						break;
					}
				}
				if(!retversion)
				{
					LIST->WTP_M_V[LIST->wtp_version_type]->wtp_version= (char*)malloc(strlen(codever)+1);
					memset(LIST->WTP_M_V[LIST->wtp_version_type]->wtp_version,0,strlen(codever)+1);
					memcpy(LIST->WTP_M_V[LIST->wtp_version_type]->wtp_version,codever,strlen(codever));
					LIST->WTP_M_V[LIST->wtp_version_type]->wtp_version_num++;
					LIST->wtp_version_type++;
				}
			}
		}
	   }
	//fengwenchao add end
	}
	dbus_message_unref(reply);
	return LIST;

}

//fengwenchao add 20110126 for XJDEV-32  from 2.0
int dcli_set_ap_eth_if_mtu(int index,int localid,unsigned char policy,unsigned int mtu,unsigned int wtp_id,DBusConnection *dcli_dbus_connection){
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	int i;
	int ret;
	dbus_error_init(&err);

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WTP_METHOD_SET_WTP_INTERFACE_ETH_MTU);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&wtp_id,	
							 DBUS_TYPE_BYTE,&policy,
							 DBUS_TYPE_UINT32,&mtu,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);


	if (NULL == reply) {
		//printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return -1;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	dbus_message_unref(reply);	

	return ret;
}
//fengwenchao add end
void* dcli_wtp_show_api_group_two(
	int index,
	unsigned int id,
	unsigned int id1,
	unsigned int id2,
	unsigned int id3,
	unsigned int* num,
	unsigned int* num2,
	unsigned int* num3,
	unsigned char *num4,
	unsigned char *num5,
	int *num6,
//	DCLI_WTP_API_GROUP_TWO *LIST,	
	DBusConnection *dcli_dbus_connection,
	char *DBUS_METHOD
	)
{	
	int ret;
	int i = 0;	
	unsigned int dcli_sn = 0;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusError err;
	DCLI_WTP_API_GROUP_TWO *LIST = NULL;
	int wtpid = id1;
	int localid = *num6;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_INTERFACE,INTERFACE);

	//printf("bbbbbbbbbbbbbbbbbbb\n");
	dcli_sn = dcli_wtp_method_parse_two(DBUS_METHOD);
	if((dcli_sn == 1)||(dcli_sn == 2)||(dcli_sn == 3)||(dcli_sn == 4)||(dcli_sn == 5)||(dcli_sn == 6)||(dcli_sn == 7)||(dcli_sn == 8)||(dcli_sn == 9)||(dcli_sn == 10)){
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,DBUS_METHOD);
		dbus_error_init(&err);	
		dbus_message_append_args(query,
								 DBUS_TYPE_UINT32,&wtpid,
								 DBUS_TYPE_INVALID);
	}
/*	else if(dcli_sn == 2){
//printf("(dcli_sn == 2)\n");
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_AP_SHOW_WTP_EXTENSION_INFOMATION_V3);
		dbus_error_init(&err);
		dbus_message_append_args(query,
								 DBUS_TYPE_UINT32,&wtpid,
								 DBUS_TYPE_INVALID);
	}
	else if(dcli_sn == 3){
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SHOW_AP_CM_STATISTICS);
		dbus_error_init(&err);
		dbus_message_append_args(query,
								 DBUS_TYPE_UINT32,&wtpid,
								 DBUS_TYPE_INVALID);
		}
	else if(dcli_sn == 4){
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WTP_METHOD_SHOW_WTP_BSS_PKT_INFO);
		dbus_error_init(&err);
	
		dbus_message_append_args(query,
								 DBUS_TYPE_UINT32,&wtpid,
								 DBUS_TYPE_INVALID);
	}
	else if(dcli_sn == 5){
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WTP_METHOD_SHOW_WTP_ETH_PKT_INFO);
		dbus_error_init(&err);
		dbus_message_append_args(query,
								 DBUS_TYPE_UINT32,&wtpid,
								 DBUS_TYPE_INVALID);
	}
	else if(dcli_sn == 6){
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WTP_METHOD_SHOW_WTP_WIFI_SNR_INFO);
		dbus_error_init(&err);
		dbus_message_append_args(query,
								 DBUS_TYPE_UINT32,&wtpid,
								 DBUS_TYPE_INVALID);
	}
	else if(dcli_sn == 7){
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WTP_METHOD_SHOW_WTP_RADIO_PKT_INFO);
		dbus_error_init(&err);
		dbus_message_append_args(query,
								 DBUS_TYPE_UINT32,&wtpid,
								 DBUS_TYPE_INVALID);
	}
	else if(dcli_sn == 8){
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_AP_SHOW_WTP_EXTENSION_INFOMATION_V4);
		dbus_error_init(&err);
	
		dbus_message_append_args(query,
								 DBUS_TYPE_UINT32,&wtpid,
								 DBUS_TYPE_INVALID);
	}
	else if(dcli_sn == 9){
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SHOW_AP_MIB_INFO);
		dbus_error_init(&err);
		dbus_message_append_args(query,
								 DBUS_TYPE_UINT32,&wtpid,
								 DBUS_TYPE_INVALID);
	}
	else if(dcli_sn == 10){
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WTP_METHOD_SHOW_WTP_IF_INFO);
		dbus_error_init(&err);
		dbus_message_append_args(query,
								 DBUS_TYPE_UINT32,&wtpid,
								 DBUS_TYPE_INVALID);
	}
*/
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		//printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		ret = -1;
		return NULL;
	}

	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	*num = ret;
//printf("ccccccccc  ret is %d \n",ret);
	if(ret == 0 )
	{	
		CW_CREATE_OBJECT_ERR(LIST, DCLI_WTP_API_GROUP_TWO, return NULL;);	
		LIST->WTP = NULL;
		if(dcli_sn == 1){	
			
			LIST->WTP = (WID_WTP*)malloc(sizeof(WID_WTP*));
			LIST->WTP[0] = (WID_WTP*)malloc(sizeof(WID_WTP));
			{
				LIST->WTP[0]->wid_sample_throughput.time = 0;
				LIST->WTP[0]->wid_sample_throughput.past_downlink_throughput = 0;
				LIST->WTP[0]->wid_sample_throughput.past_uplink_throughput =0;
				LIST->WTP[0]->wid_sample_throughput.current_downlink_throughput = 0;
				LIST->WTP[0]->wid_sample_throughput.current_uplink_throughput = 0;
				LIST->WTP[0]->wid_sample_throughput.uplink_rate = 0;
				LIST->WTP[0]->wid_sample_throughput.downlink_rate =0;
			}
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wid_sample_throughput.time);
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wid_sample_throughput.past_uplink_throughput);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wid_sample_throughput.past_downlink_throughput);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wid_sample_throughput.current_uplink_throughput);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wid_sample_throughput.current_downlink_throughput);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wid_sample_throughput.uplink_rate);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wid_sample_throughput.downlink_rate);

		
	}
	else if(dcli_sn == 2){	
		LIST->WTP = (WID_WTP*)malloc(sizeof(WID_WTP*));
		LIST->WTP[0] = (WID_WTP*)malloc(sizeof(WID_WTP));
		{
				LIST->WTP[0]->wifi_extension_reportswitch = 0;
				LIST->WTP[0]->wifi_extension_reportinterval = 0;
				LIST->WTP[0]->wifi_extension_info.cpu = 0;
				LIST->WTP[0]->wifi_extension_info.tx_mgmt = 0;
				LIST->WTP[0]->wifi_extension_info.rx_mgmt = 0;
				LIST->WTP[0]->wifi_extension_info.tx_packets = 0;
				LIST->WTP[0]->wifi_extension_info.tx_errors = 0;
				LIST->WTP[0]->wifi_extension_info.tx_retry = 0;
				LIST->WTP[0]->wifi_extension_info.ipmode = 0;
				LIST->WTP[0]->wifi_extension_info.memoryall = 0;
				LIST->WTP[0]->wifi_extension_info.memoryuse = 0;
				LIST->WTP[0]->wifi_extension_info.flashall = 0;
				LIST->WTP[0]->wifi_extension_info.flashempty = 0;
				LIST->WTP[0]->wifi_extension_info.wifi_snr = 0;
				LIST->WTP[0]->wifi_extension_info.eth_count = 0;
				LIST->WTP[0]->wifi_extension_info.collect_time = 0;
				LIST->WTP[0]->wifi_extension_info.ath_count = 0;
				LIST->WTP[0]->wifi_extension_info.temperature = 0;
				LIST->WTP[0]->wifi_extension_info.wifi_count = 0;
				memset(LIST->WTP[0]->wifi_extension_info.wifi_state,0,AP_WIFI_IF_NUM);
				memset(LIST->WTP[0]->wifi_extension_info.eth_updown_time,0,AP_ETH_IF_NUM);
				memset(LIST->WTP[0]->wifi_extension_info.ath_if_info,0,sizeof(wid_ap_ath_info)*AP_ATH_IF_NUM);
			}
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wifi_extension_info.cpu);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wifi_extension_info.tx_mgmt);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wifi_extension_info.rx_mgmt);
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wifi_extension_info.tx_packets);
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wifi_extension_info.tx_errors);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wifi_extension_info.tx_retry);

		/*add something diff from v1*/
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wifi_extension_reportswitch);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wifi_extension_reportinterval);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wifi_extension_info.ipmode);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wifi_extension_info.memoryall);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wifi_extension_info.memoryuse);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wifi_extension_info.flashall);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wifi_extension_info.flashempty);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wifi_extension_info.wifi_snr);
		/*end*/
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wifi_extension_info.eth_count);
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wifi_extension_info.collect_time);

		for(i=0;i<AP_ETH_IF_NUM;i++)
		{		
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wifi_extension_info.eth_updown_time[i]);
		}

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wifi_extension_info.ath_count);
		/*fengwenchao modify begin for autelan-2917 20120502*/
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);		
		for(i=0;i<LIST->WTP[0]->wifi_extension_info.ath_count;i++)
		{	
			DBusMessageIter  iter_struct;
			dbus_message_iter_recurse(&iter_array,&iter_struct);		
			dbus_message_iter_get_basic(&iter_struct,&LIST->WTP[0]->wifi_extension_info.ath_if_info[i].radioid);

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&LIST->WTP[0]->wifi_extension_info.ath_if_info[i].wlanid);

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&LIST->WTP[0]->wifi_extension_info.ath_if_info[i].ath_updown_times);

			dbus_message_iter_next(&iter_array);	
		}
		/*fengwenchao modify end*/
		/*add something diff from v2*/
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wifi_extension_info.temperature);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wifi_extension_info.wifi_count);

		for(i=0;i<AP_WIFI_IF_NUM;i++)
		{		
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wifi_extension_info.wifi_state[i]);
		}
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wifi_extension_info.rx_broadcast);
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wifi_extension_info.rx_unicast);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wifi_extension_info.tx_broadcast);
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wifi_extension_info.tx_unicast);
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wifi_extension_info.rx_multicast);
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wifi_extension_info.tx_multicast);
	}
	else if(dcli_sn == 3){	
		char *cpuType = NULL;
		char *flashType = NULL ;
		char *memType = NULL;
		LIST->WTP = (WID_WTP*)malloc(sizeof(WID_WTP*));
		LIST->WTP[0] = (WID_WTP*)malloc(sizeof(WID_WTP));
		LIST->WTP[0]->apcminfo.cpu_average = 0;
		LIST->WTP[0]->apcminfo.cpu_peak_value = 0;
		LIST->WTP[0]->apcminfo.cpu_times = 0;
		LIST->WTP[0]->apcminfo.mem_average = 0;
		LIST->WTP[0]->apcminfo.mem_peak_value = 0;
		LIST->WTP[0]->apcminfo.mem_times  = 0;
		memset(LIST->WTP[0]->cpuType,0,WTP_TYPE_DEFAULT_LEN);
		memset(LIST->WTP[0]->flashType,0,WTP_TYPE_DEFAULT_LEN);
		memset(LIST->WTP[0]->memType,0,WTP_TYPE_DEFAULT_LEN);
		LIST->WTP[0]->flashSize = 0;
		LIST->WTP[0]->memSize = 0;

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->apcminfo.cpu_average);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->apcminfo.cpu_peak_value);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->apcminfo.cpu_times);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->apcminfo.mem_average);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->apcminfo.mem_peak_value);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->apcminfo.mem_times);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&cpuType);
			memset(LIST->WTP[0]->cpuType,0,strlen(cpuType)+1);
			memcpy(LIST->WTP[0]->cpuType,cpuType,strlen(cpuType));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&flashType);
			memset(LIST->WTP[0]->flashType,0,strlen(flashType)+1);
			memcpy(LIST->WTP[0]->flashType,flashType,strlen(flashType));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&memType);
			memset(LIST->WTP[0]->memType,0,strlen(memType)+1);
			memcpy(LIST->WTP[0]->memType,memType,strlen(memType));
	}
	else if(dcli_sn == 4){
		CW_CREATE_OBJECT_ERR(LIST, DCLI_WTP_API_GROUP_TWO, return NULL;);	
		LIST->WTP = NULL;
		LIST->WTP = (WID_WTP*)malloc(sizeof(WID_WTP*));
		LIST->WTP[0] = (WID_WTP*)malloc(sizeof(WID_WTP));
		LIST->WTP[0]->WTP_Radio[0] = (WID_WTP_RADIO*)malloc(sizeof(WID_WTP_RADIO));
		//LIST->WTP[0]->WTP_Radio[0]->BSS[0]= (WID_BSS*)malloc(sizeof(WID_BSS));
		int bsscount = 0;
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&bsscount);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
		LIST->WTP[0]->WTP_Radio[0]->bss_num = bsscount;
//printf("bsscount is %d.\n",bsscount);
		for(i=0;i<bsscount;i++)
		{
			LIST->WTP[0]->WTP_Radio[0]->BSS[i]= (WID_BSS*)malloc(sizeof(WID_BSS));
			DBusMessageIter iter_struct;
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&LIST->WTP[0]->WTP_Radio[0]->BSS[i]->BSS_pkt_info.BSSIndex);

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&LIST->WTP[0]->WTP_Radio[0]->BSS[i]->BSS_pkt_info.Radio_G_ID);

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&LIST->WTP[0]->WTP_Radio[0]->BSS[i]->BSS_pkt_info.rx_unicast);

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&LIST->WTP[0]->WTP_Radio[0]->BSS[i]->BSS_pkt_info.tx_unicast);

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&LIST->WTP[0]->WTP_Radio[0]->BSS[i]->BSS_pkt_info.rx_broadcast);

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&LIST->WTP[0]->WTP_Radio[0]->BSS[i]->BSS_pkt_info.tx_broadcast);

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&LIST->WTP[0]->WTP_Radio[0]->BSS[i]->BSS_pkt_info.rx_pkt_unicast);

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&LIST->WTP[0]->WTP_Radio[0]->BSS[i]->BSS_pkt_info.tx_pkt_unicast);

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&LIST->WTP[0]->WTP_Radio[0]->BSS[i]->BSS_pkt_info.rx_pkt_broadcast);

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&LIST->WTP[0]->WTP_Radio[0]->BSS[i]->BSS_pkt_info.tx_pkt_broadcast);
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&LIST->WTP[0]->WTP_Radio[0]->BSS[i]->BSS_pkt_info.retry);

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&LIST->WTP[0]->WTP_Radio[0]->BSS[i]->BSS_pkt_info.retry_pkt);

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&LIST->WTP[0]->WTP_Radio[0]->BSS[i]->BSS_pkt_info.err);

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&LIST->WTP[0]->WTP_Radio[0]->BSS[i]->WlanID);

			dbus_message_iter_next(&iter_array);
		}
	}
	else if (dcli_sn == 5){	
		LIST->WTP = (WID_WTP*)malloc(sizeof(WID_WTP*));
		LIST->WTP[0] = (WID_WTP*)malloc(sizeof(WID_WTP));
		LIST->WTP[0]->WTP_Radio[0] = (WID_WTP*)malloc(sizeof(WID_WTP_RADIO));
		LIST->WTP[0]->WTP_Radio[0]->BSS[0] = (WID_WTP*)malloc(sizeof(WID_BSS));
		{
			LIST->WTP[0]->WTP_Radio[0]->BSS[0]->BSS_pkt_info.rx_unicast = 0;
			LIST->WTP[0]->WTP_Radio[0]->BSS[0]->BSS_pkt_info.tx_unicast = 0;
			LIST->WTP[0]->WTP_Radio[0]->BSS[0]->BSS_pkt_info.rx_broadcast = 0;
			LIST->WTP[0]->WTP_Radio[0]->BSS[0]->BSS_pkt_info.tx_broadcast = 0;
			LIST->WTP[0]->WTP_Radio[0]->BSS[0]->BSS_pkt_info.rx_pkt_unicast = 0;
			LIST->WTP[0]->WTP_Radio[0]->BSS[0]->BSS_pkt_info.tx_pkt_unicast = 0;
			LIST->WTP[0]->WTP_Radio[0]->BSS[0]->BSS_pkt_info.rx_pkt_broadcast = 0;
			LIST->WTP[0]->WTP_Radio[0]->BSS[0]->BSS_pkt_info.tx_pkt_broadcast = 0;
			LIST->WTP[0]->WTP_Radio[0]->BSS[0]->BSS_pkt_info.retry = 0;
			LIST->WTP[0]->WTP_Radio[0]->BSS[0]->BSS_pkt_info.retry_pkt = 0;
			LIST->WTP[0]->WTP_Radio[0]->BSS[0]->BSS_pkt_info.err = 0;
		}
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->WTP_Radio[0]->BSS[0]->BSS_pkt_info.rx_unicast);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->WTP_Radio[0]->BSS[0]->BSS_pkt_info.tx_unicast);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->WTP_Radio[0]->BSS[0]->BSS_pkt_info.rx_broadcast);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->WTP_Radio[0]->BSS[0]->BSS_pkt_info.tx_broadcast);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->WTP_Radio[0]->BSS[0]->BSS_pkt_info.rx_pkt_unicast);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->WTP_Radio[0]->BSS[0]->BSS_pkt_info.tx_pkt_unicast);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->WTP_Radio[0]->BSS[0]->BSS_pkt_info.rx_pkt_broadcast);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->WTP_Radio[0]->BSS[0]->BSS_pkt_info.tx_pkt_broadcast);
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->WTP_Radio[0]->BSS[0]->BSS_pkt_info.retry);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->WTP_Radio[0]->BSS[0]->BSS_pkt_info.retry_pkt);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->WTP_Radio[0]->BSS[0]->BSS_pkt_info.err);

	}
	else if(dcli_sn == 6){	
		LIST->WTP = (WID_WTP*)malloc(sizeof(WID_WTP*));
		LIST->WTP[0] = (WID_WTP*)malloc(sizeof(WID_WTP));
		{
			LIST->WTP[0]->wtp_wifi_snr_stats.ifindex = 0;
			LIST->WTP[0]->wtp_wifi_snr_stats.snr_average = 0;
			LIST->WTP[0]->wtp_wifi_snr_stats.snr_max = 0;
			LIST->WTP[0]->wtp_wifi_snr_stats.snr_min = 0;
			memset(LIST->WTP[0]->wtp_wifi_snr_stats.snr,0,10);
		}
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wtp_wifi_snr_stats.ifindex);
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wtp_wifi_snr_stats.snr_average);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wtp_wifi_snr_stats.snr_max);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wtp_wifi_snr_stats.snr_min);
	}
	else if(dcli_sn == 7){	
		LIST->WTP = (WID_WTP*)malloc(sizeof(WID_WTP*));
		LIST->WTP[0] = (WID_WTP*)malloc(sizeof(WID_WTP));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->radio_num);
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
	//printf("(dcli_sn == 7) jieshou\n");	
		//LIST->WTP[0]->WTP_Radio = (WID_WTP_RADIO*)malloc(radio_num*sizeof(WID_WTP_RADIO*));
		for(i=0;i<LIST->WTP[0]->radio_num;i++)
		{	
			LIST->WTP[0]->WTP_Radio[i]= (WID_WTP_RADIO*)malloc(sizeof(WID_WTP_RADIO));
			LIST->WTP[0]->WTP_Radio[i]->BSS[0] = (WID_BSS *)malloc(sizeof(WID_BSS));
			DBusMessageIter iter_struct;
			dbus_message_iter_recurse(&iter_array,&iter_struct);
				
			dbus_message_iter_get_basic(&iter_struct,&LIST->WTP[0]->WTP_Radio[i]->BSS[0]->BSS_pkt_info.Radio_G_ID);

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&LIST->WTP[0]->WTP_Radio[i]->BSS[0]->BSS_pkt_info.rx_unicast);

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&LIST->WTP[0]->WTP_Radio[i]->BSS[0]->BSS_pkt_info.tx_unicast);

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&LIST->WTP[0]->WTP_Radio[i]->BSS[0]->BSS_pkt_info.rx_broadcast);

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&LIST->WTP[0]->WTP_Radio[i]->BSS[0]->BSS_pkt_info.tx_broadcast);

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&LIST->WTP[0]->WTP_Radio[i]->BSS[0]->BSS_pkt_info.rx_pkt_unicast);

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&LIST->WTP[0]->WTP_Radio[i]->BSS[0]->BSS_pkt_info.tx_pkt_unicast);

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&LIST->WTP[0]->WTP_Radio[i]->BSS[0]->BSS_pkt_info.rx_pkt_broadcast);

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&LIST->WTP[0]->WTP_Radio[i]->BSS[0]->BSS_pkt_info.tx_pkt_broadcast);
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&LIST->WTP[0]->WTP_Radio[i]->BSS[0]->BSS_pkt_info.retry);

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&LIST->WTP[0]->WTP_Radio[i]->BSS[0]->BSS_pkt_info.retry_pkt);

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&LIST->WTP[0]->WTP_Radio[i]->BSS[0]->BSS_pkt_info.err);

			dbus_message_iter_next(&iter_array);
		}
	}
	else if(dcli_sn == 8){	
	
		LIST->WTP = (WID_WTP*)malloc(sizeof(WID_WTP*));
		LIST->WTP[0] = (WID_WTP*)malloc(sizeof(WID_WTP));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wifi_extension_info.cpu);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wifi_extension_info.tx_mgmt);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wifi_extension_info.rx_mgmt);
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wifi_extension_info.tx_packets);
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wifi_extension_info.tx_errors);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wifi_extension_info.tx_retry);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wifi_extension_reportswitch);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wifi_extension_reportinterval);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wifi_extension_info.ipmode);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wifi_extension_info.memoryall);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wifi_extension_info.memoryuse);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wifi_extension_info.flashall);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wifi_extension_info.flashempty);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wifi_extension_info.wifi_snr);
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wifi_extension_info.eth_count);

		for(i=0;i<AP_ETH_IF_NUM;i++)
		{		
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wifi_extension_info.eth_updown_time[i]);
		}

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wifi_extension_info.ath_count);

		/*fengwenchao modify begin for autelan-2917 20120502*/
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);			
		for(i=0;i<LIST->WTP[0]->wifi_extension_info.ath_count;i++)
		{	
			DBusMessageIter  iter_struct;
			dbus_message_iter_recurse(&iter_array,&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&LIST->WTP[0]->wifi_extension_info.ath_if_info[i].radioid);

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&LIST->WTP[0]->wifi_extension_info.ath_if_info[i].wlanid);

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&LIST->WTP[0]->wifi_extension_info.ath_if_info[i].ath_updown_times);

			dbus_message_iter_next(&iter_array);	
		}
		/*fengwenchao modify end*/
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wifi_extension_info.temperature);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wifi_extension_info.wifi_count);

		for(i=0;i<AP_WIFI_IF_NUM;i++)
		{		
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wifi_extension_info.wifi_state[i]);
		}

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wifi_extension_info.tx_unicast);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wifi_extension_info.tx_broadcast);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wifi_extension_info.tx_multicast);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wifi_extension_info.tx_drop);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wifi_extension_info.rx_unicast);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wifi_extension_info.rx_broadcast);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wifi_extension_info.rx_multicast);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wifi_extension_info.rx_drop);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wifi_extension_info.wpi_replay_error);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wifi_extension_info.wpi_decryptable_error);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wifi_extension_info.wpi_mic_error);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wifi_extension_info.disassoc_unnormal);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wifi_extension_info.rx_assoc_norate);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wifi_extension_info.rx_assoc_capmismatch);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wifi_extension_info.assoc_invaild);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->wifi_extension_info.reassoc_deny);
		
	}
	else if(dcli_sn == 9){	
		LIST->WTP = (WID_WTP*)malloc(sizeof(WID_WTP*));
		LIST->WTP[0] = (WID_WTP*)malloc(sizeof(WID_WTP));
		
		LIST->WTP[0]->mib_info.dos_def_switch = 0;
		LIST->WTP[0]->mib_info.igmp_snoop_switch = 0;
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->mib_info.dos_def_switch);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->mib_info.igmp_snoop_switch);

		/*fengwenchao modify begin for autelan-2917 20120502*/
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);		

		for(i=0;i<L_BSS_NUM;i++)
		{

			LIST->WTP[0]->mib_info.wlan_l2isolation[i].wlanid = 0;
			LIST->WTP[0]->mib_info.wlan_l2isolation[i].l2_isolation_switch = 0;
			DBusMessageIter  iter_struct;
			dbus_message_iter_recurse(&iter_array,&iter_struct);				
			dbus_message_iter_get_basic(&iter_struct,&LIST->WTP[0]->mib_info.wlan_l2isolation[i].wlanid);

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&LIST->WTP[0]->mib_info.wlan_l2isolation[i].l2_isolation_switch);

			dbus_message_iter_next(&iter_array);	
		}
		/*fengwenchao modify end*/
	}
	else if(dcli_sn == 10){	
		LIST->WTP = (WID_WTP*)malloc(sizeof(WID_WTP*));
		LIST->WTP[0] = (WID_WTP*)malloc(sizeof(WID_WTP));
		{
			LIST->WTP[0]->apifinfo.report_switch = 0;
			LIST->WTP[0]->apifinfo.report_interval = 0;
			LIST->WTP[0]->apifinfo.eth_num = 0;
			LIST->WTP[0]->apifinfo.wifi_num = 0;
			LIST->WTP[0]->RadioCount = 0;
		}
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->apifinfo.report_switch);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->apifinfo.report_interval);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->apifinfo.eth_num);

		for(i=0;i<LIST->WTP[0]->apifinfo.eth_num;i++)
		{	
			LIST->WTP[0]->apifinfo.eth[i].type = 0;
			LIST->WTP[0]->apifinfo.eth[i].ifindex = 0;
			LIST->WTP[0]->apifinfo.eth[i].state = 0;
			LIST->WTP[0]->apifinfo.eth[i].state_time = 0;
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->apifinfo.eth[i].type);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->apifinfo.eth[i].ifindex);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->apifinfo.eth[i].state);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->apifinfo.eth[i].state_time);
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->apifinfo.eth[i].eth_rate);
			//fengwenchao add 20110126 for XJDEV-32 from 2.0
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->apifinfo.eth[i].eth_mtu);
			//fengwenchao add end
		}

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->apifinfo.wifi_num);
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->RadioCount);	//xiaodawei add for radiocount from wtpcompatible.xml, 20110124

		for(i=0;i<LIST->WTP[0]->apifinfo.wifi_num && i<LIST->WTP[0]->RadioCount;i++)
		{	
			LIST->WTP[0]->apifinfo.wifi[i].type = 0;
			LIST->WTP[0]->apifinfo.wifi[i].ifindex = 0;
			LIST->WTP[0]->apifinfo.wifi[i].state = 0;
			LIST->WTP[0]->apifinfo.wifi[i].state_time = 0;
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->apifinfo.wifi[i].type);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->apifinfo.wifi[i].ifindex);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->apifinfo.wifi[i].state);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->apifinfo.wifi[i].state_time);
		}
	}
	}
	dbus_message_unref(reply);
	return LIST;

}

void* dcli_wtp_show_api_group_three(
	int index,
	unsigned int id,
	unsigned int id1,
	unsigned int id2,
	unsigned int id3,
	unsigned int* num,
	unsigned int* num2,
	unsigned int* num3,
	unsigned char *num4,
	unsigned char *num5,
	int *num6,
//	DCLI_WTP_API_GROUP_THREE *LIST,	
	DBusConnection *dcli_dbus_connection,
	char *DBUS_METHOD
	)
{	
	int ret = 0;
	int i = 0;
	unsigned int dcli_sn = 0;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusError err;
	
	DCLI_WTP_API_GROUP_THREE *LIST = NULL;

	int WTPID = id1;
	int haveret = id2;
	int localid = *num6;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_INTERFACE,INTERFACE);

	//printf("bbbbbbbbbbbbbbbbbbb\n");
	dcli_sn = dcli_wtp_method_parse_three(DBUS_METHOD);
	if((dcli_sn == 1)||(dcli_sn == 9)){
//printf("bbbbbb(dcli_sn == 1)bbbbbbbbbbbbb\n");
		ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
		ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,DBUS_METHOD);
		dbus_error_init(&err);	
	}
	else if((dcli_sn == 2)||(dcli_sn == 3)||(dcli_sn == 4)||(dcli_sn == 5)||(dcli_sn == 7)){
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,DBUS_METHOD);
		dbus_error_init(&err);
		dbus_message_append_args(query,
								 DBUS_TYPE_UINT32,&WTPID,
								 DBUS_TYPE_INVALID);
	}
/*	else if(dcli_sn == 3){
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SHOW_WID_WTP_LOCATION);
		dbus_error_init(&err);
		dbus_message_append_args(query,
								 DBUS_TYPE_UINT32,&WTPID,
								 DBUS_TYPE_INVALID);
	}
	else if(dcli_sn == 4){
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SHOW_WID_WTP_NETID);
		dbus_error_init(&err);
		dbus_message_append_args(query,
								 DBUS_TYPE_UINT32,&WTPID,
								 DBUS_TYPE_INVALID);
	}
	else if(dcli_sn == 5){
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SHOW_WID_WTP_WLAN_VLAN_INFO);
		dbus_error_init(&err);
		dbus_message_append_args(query,
								 DBUS_TYPE_UINT32,&WTPID,
								 DBUS_TYPE_INVALID);
	}*/
	else if(dcli_sn == 6){
		ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
		ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,DBUS_METHOD);
		dbus_error_init(&err);
		dbus_message_append_args(query,
								 DBUS_TYPE_UINT32,&WTPID,
								 DBUS_TYPE_INVALID);
	}
	else if(dcli_sn == 16){
		ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
		ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,DBUS_METHOD);
		dbus_error_init(&err);
	}
/*	else if(dcli_sn == 7){
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_AP_SHOW_WTP_MAX_POWER);
		dbus_error_init(&err);
		dbus_message_append_args(query,
								 DBUS_TYPE_UINT32,&WTPID,
								 DBUS_TYPE_INVALID);
	}*/
	else if(dcli_sn == 8){
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,DBUS_METHOD);
		dbus_error_init(&err);	
	}
/*	else if(dcli_sn == 9){
//printf("dcli,dcli_sn is 9\n");
		ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
		ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SHOW_OLD_AP_IMG);
		dbus_error_init(&err);

	}*/

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		//printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		ret = -1;
		return NULL;
	}

	dbus_message_iter_init(reply,&iter);

	if(haveret == 0){
		dbus_message_iter_get_basic(&iter,&ret);
//printf("ccccccccc  ret is %d \n",ret);
		*num = ret;
		if(ret == 0 )
		{	
			char *model = NULL;
			char *versionname = NULL;
			char *versionpath = NULL;
			char *apcode = NULL;
			CW_CREATE_OBJECT_ERR(LIST, DCLI_WTP_API_GROUP_THREE, return NULL;); 
			memset(LIST,0,sizeof(DCLI_WTP_API_GROUP_THREE));
			LIST->ElectrifyRegisterCircle = 0;
			LIST->addtime = 0;
			LIST->starttime = 0;
			LIST->imagadata_time = 0;
			LIST->config_update_time = 0;
			LIST->next = NULL;

			if(dcli_sn == 1){
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&LIST->echotimer);
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&LIST->checktimer);
			}
			else if(dcli_sn == 2){
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&(LIST->addtime));	

				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&(LIST->ElectrifyRegisterCircle));

				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&(LIST->imagadata_time));

				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&(LIST->config_update_time));

				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&(LIST->quittime));
				
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&(LIST->checktimes));
			}
			else if(dcli_sn == 3){	
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&LIST->wtp_location);
			
			}
			else if(dcli_sn == 4){	
				char* netid = NULL;
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&netid);/*LIST->netid*/

					LIST->netid = (char*)malloc(strlen(netid)+1);
					memset(LIST->netid,0,strlen(netid)+1);
					memcpy(LIST->netid,netid,strlen(netid));
			}
			else if (dcli_sn == 5){	
				int wlan_num = 0;
				/*fengwenchao add 20120428 for autelan-2917*/
				DBusMessageIter  iter_array;
				DBusMessageIter  iter_struct;
				/*fengwenchao add end*/
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&LIST->wlan_num);
				if(LIST->wlan_num > 0){
					dbus_message_iter_next(&iter);	
					dbus_message_iter_recurse(&iter,&iter_array);
				}
				LIST->WLAN = (WID_WLAN*)malloc((LIST->wlan_num)*sizeof(WID_WLAN*));
				for(i=0;i<LIST->wlan_num;i++)
				{	
					LIST->WLAN[i] = (WID_WLAN*)malloc(sizeof(WID_WLAN));
					dbus_message_iter_recurse(&iter_array,&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,&LIST->WLAN[i]->WlanID);

					dbus_message_iter_next(&iter_struct);	
					dbus_message_iter_get_basic(&iter_struct,&LIST->WLAN[i]->vlanid);

					dbus_message_iter_next(&iter_struct);	
					dbus_message_iter_get_basic(&iter_struct,&LIST->WLAN[i]->wlan_1p_priority);

					dbus_message_iter_next(&iter_array);
				}
			}
			else if(dcli_sn == 6){	
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&LIST->wids.flooding);

				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&LIST->wids.sproof);

				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&LIST->wids.weakiv);

				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&LIST->interval);

				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&LIST->probethreshold);

				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&LIST->otherthreshold);

				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&LIST->lasttime);
			}
			else if(dcli_sn == 16) {
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&LIST->dhcp_flooding_status);
			}
			else if(dcli_sn == 7){	
			
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&model);/*LIST->model*/

					LIST->model = (char*)malloc(strlen(model)+1);
					memset(LIST->model,0,strlen(model)+1);
					memcpy(LIST->model,model,strlen(model));

				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&LIST->txpower);

			}
			else if(dcli_sn == 8){	
				if(LIST){
					dcli_wtp_free_ap_update_config_fun(LIST);
					LIST = NULL;
				}
				char i=0,j=0,k=0,m=0;
				unsigned char Count_onetimeupdt;

				dbus_message_iter_next(&iter);		
				dbus_message_iter_get_basic(&iter,&Count_onetimeupdt);	
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&j);/*(upgrade ap model num)*/	
				
				for(i=0;i<j;i++){
					dbus_message_iter_next(&iter);	
					dbus_message_iter_get_basic(&iter,&k);/*(upgrade ap code num of one model)*/	
					
					for(m=0;m<k;m++){
						model = NULL;
						versionname = NULL;
						versionpath = NULL;
						
						DCLI_WTP_API_GROUP_THREE *tmp = NULL;
						CW_CREATE_OBJECT_ERR(tmp, DCLI_WTP_API_GROUP_THREE, return NULL;); 
						memset(tmp,0,sizeof(DCLI_WTP_API_GROUP_THREE));
						tmp->next = NULL;

						dbus_message_iter_next(&iter);	
						dbus_message_iter_get_basic(&iter,&model);/*(tmp->model)*/	

							tmp->model = (char*)malloc(strlen(model)+1);
							memset(tmp->model,0,strlen(model)+1);
							memcpy(tmp->model,model,strlen(model));
								
						dbus_message_iter_next(&iter);
						dbus_message_iter_get_basic(&iter,&versionname);/*(tmp->versionname)*/

							tmp->versionname = (char*)malloc(strlen(versionname)+1);
							memset(tmp->versionname,0,strlen(versionname)+1);
							memcpy(tmp->versionname,versionname,strlen(versionname));
					
						dbus_message_iter_next(&iter);		
						dbus_message_iter_get_basic(&iter,&versionpath);/*(tmp->versionpath)*/

							tmp->versionpath = (char*)malloc(strlen(versionpath)+1);
							memset(tmp->versionpath,0,strlen(versionpath)+1);
							memcpy(tmp->versionpath,versionpath,strlen(versionpath));

						tmp->Count_onetimeupdt = Count_onetimeupdt;
						
						if(LIST == NULL){
							LIST = tmp;
						}else{
							tmp->next = LIST;
							LIST = tmp;
						}	
					}
				}
			}
			else{}
		}
	}
	else{
		if(dcli_sn == 9){
			CW_CREATE_OBJECT_ERR(LIST, DCLI_WTP_API_GROUP_THREE, return NULL;); 

			dbus_message_iter_get_basic(&iter,&LIST->old_ap_img_state);
		}
	}
	dbus_message_unref(reply);
	return LIST;

}
void dcli_wtp_free_fun(char *DBUS_METHOD,DCLI_WTP_API_GROUP_ONE*WTPINFO)
{
		int i = 0;
		int dcli_sn = 0;
		dcli_sn = dcli_wtp_method_parse_fist(DBUS_METHOD);

		switch(dcli_sn){
			case 1 :{
				if((WTPINFO)&&(WTPINFO->WTP)&&(WTPINFO->WTP[0])){
					CW_FREE_OBJECT(WTPINFO->WTP[0]->WTPMAC);
					CW_FREE_OBJECT(WTPINFO->WTP[0]->WTPSN);
					CW_FREE_OBJECT(WTPINFO->WTP[0]->WTPNAME);
					CW_FREE_OBJECT(WTPINFO->WTP[0]->WTPModel);
					CW_FREE_OBJECT(WTPINFO->WTP[0]->WTPIP);
					CW_FREE_OBJECT(WTPINFO->WTP[0]->sysver);
					CW_FREE_OBJECT(WTPINFO->WTP[0]->ver);
					CW_FREE_OBJECT(WTPINFO->WTP[0]->updatepath);
					CW_FREE_OBJECT(WTPINFO->WTP[0]->updateversion);
					CW_FREE_OBJECT(WTPINFO->WTP[0]->apply_interface_name);
					for (i = 0; i < WTPINFO->WTP[0]->radio_num; i++) {
						CW_FREE_OBJECT(WTPINFO->WTP[0]->WTP_Radio[i]);
					}
					CW_FREE_OBJECT(WTPINFO->WTP[0]);
				}
				if(WTPINFO){
					CW_FREE_OBJECT(WTPINFO->WTP);
					CW_FREE_OBJECT(WTPINFO);
				}
			}
			break;
			case 2 :{
						WID_WTP *phead = NULL;
						WID_WTP *pnext = NULL;
						if((WTPINFO)&&(WTPINFO->WTP_INFO)){
							phead = WTPINFO->WTP_INFO->WTP_LIST;
							WTPINFO->WTP_INFO->WTP_LIST = NULL;
							
							while(phead != NULL)
							{
									pnext = phead->next;
									
									CW_FREE_OBJECT(phead->WTPMAC);
									CW_FREE_OBJECT(phead->WTPModel);
									CW_FREE_OBJECT(phead->WTPNAME);
									CW_FREE_OBJECT(phead->WTPSN);   //fengwenchao add 20110530
									CW_FREE_OBJECT(phead);
									phead = pnext;		
							}
						}
						if(WTPINFO){
							CW_FREE_OBJECT(WTPINFO->WTP_INFO);
							CW_FREE_OBJECT(WTPINFO->WTP);
							CW_FREE_OBJECT(WTPINFO);
						}
				}
			break;
			case 3 :{
					if((WTPINFO)&&(WTPINFO->WTP_INFO)&&(WTPINFO->WTP_INFO->WTP_LIST)){

						WID_WTP *phead = NULL;
						WID_WTP *pnext = NULL;
						phead = WTPINFO->WTP_INFO->WTP_LIST;
						WTPINFO->WTP_INFO->WTP_LIST = NULL;
						WTPINFO->WTP_INFO->list_len = 0;
						while(phead != NULL){

							pnext = phead->next;
							
							CW_FREE_OBJECT(phead->WTPMAC);
							CW_FREE_OBJECT(phead->WTPSN);
							CW_FREE_OBJECT(phead->WTPNAME);
							CW_FREE_OBJECT(phead->WTPModel);
							CW_FREE_OBJECT(phead);
							phead = pnext;				
						
						}
					}
					if(WTPINFO){
						CW_FREE_OBJECT(WTPINFO->WTP_INFO);
						CW_FREE_OBJECT(WTPINFO->WTP);
						CW_FREE_OBJECT(WTPINFO);
					}
				}
			break;
			case 4 :{
				{
					if((WTPINFO)&&(WTPINFO->WTP_INFO)&&(WTPINFO->WTP_INFO->WTP_LIST)){

						WID_WTP *phead = NULL;
						WID_WTP *pnext = NULL;
						phead = WTPINFO->WTP_INFO->WTP_LIST;
						WTPINFO->WTP_INFO->WTP_LIST = NULL;
						WTPINFO->WTP_INFO->list_len = 0;
						//for (i = 0; i < WTPINFO->WTP_INFO->list_len; i++) {
						while(phead != NULL){
							pnext = phead->next;
							CW_FREE_OBJECT(phead->WTPMAC);
							CW_FREE_OBJECT(phead->WTPNAME);
							CW_FREE_OBJECT(phead->WTPModel);
							CW_FREE_OBJECT(phead);
							phead = pnext;				
						}
					}
					//CW_FREE_OBJECT(WTPINFO->WTP_INFO->WTP_LIST);
					if(WTPINFO){
						CW_FREE_OBJECT(WTPINFO->WTP_INFO);
						CW_FREE_OBJECT(WTPINFO->WTP);
						CW_FREE_OBJECT(WTPINFO);
					}
				}
			}
			break;			
			case 5 :{
				if((WTPINFO)&&(WTPINFO->AP_VERSION)){
					for (i = 0; i < WTPINFO->num; i++) {
						CW_FREE_OBJECT(WTPINFO->AP_VERSION[i]);
					}
				}	
				if(WTPINFO){
					CW_FREE_OBJECT(WTPINFO->AP_VERSION);
					CW_FREE_OBJECT(WTPINFO);
				}
			}
			break;
			case 6 :{
				if(WTPINFO){
					for (i = 0; i < WTPINFO->num; i++) { 
						if((WTPINFO->WTP)&&(WTPINFO->WTP[i])){
							free(WTPINFO->WTP[i]->WTPMAC);
							WTPINFO->WTP[i]->WTPMAC = NULL;
							free(WTPINFO->WTP[i]->WTPModel);
							WTPINFO->WTP[i]->WTPModel = NULL;
							free(WTPINFO->WTP[i]);
							WTPINFO->WTP[i] = NULL;
						}
					}
				}
				if(WTPINFO){
					free(WTPINFO->WTP);
					WTPINFO->WTP = NULL;
				}
				free(WTPINFO);
				WTPINFO = NULL;
			}
			break;
			case 7 :{
				if(WTPINFO){
					for (i = 0; i < WTPINFO->num; i++) {	
						if((WTPINFO->WTP)&&(WTPINFO->WTP[i])){
							CW_FREE_OBJECT(WTPINFO->WTP[i]->WTPMAC);
							WTPINFO->WTP[i]->WTPMAC = NULL;
							CW_FREE_OBJECT(WTPINFO->WTP[i]->WTPIP);
							WTPINFO->WTP[i]->WTPIP = NULL;
							CW_FREE_OBJECT(WTPINFO->WTP[i]->WTPModel);
							WTPINFO->WTP[i]->WTPModel = NULL;
							CW_FREE_OBJECT(WTPINFO->WTP[i]->APCode);
							WTPINFO->WTP[i]->APCode = NULL;
							CW_FREE_OBJECT(WTPINFO->WTP[i]->WTPSN);
							WTPINFO->WTP[i]->WTPSN = NULL;
							CW_FREE_OBJECT(WTPINFO->WTP[i]->apply_interface_name);
							WTPINFO->WTP[i]->apply_interface_name = NULL;
							CW_FREE_OBJECT(WTPINFO->WTP[i]->ver);
							WTPINFO->WTP[i]->ver = NULL;
							CW_FREE_OBJECT(WTPINFO->WTP[i]->codever);
							WTPINFO->WTP[i]->codever = NULL;
							CW_FREE_OBJECT(WTPINFO->WTP[i]);
							WTPINFO->WTP[i] = NULL;
						}
					}
					if(WTPINFO){
						CW_FREE_OBJECT(WTPINFO->WTP);
						WTPINFO->WTP = NULL;
						CW_FREE_OBJECT(WTPINFO);
						WTPINFO = NULL;
					}
				}
			}
			break;
			case 8 :{
				if((WTPINFO != NULL)&&(WTPINFO->WTP)&&(WTPINFO->WTP[0])){
					CW_FREE_OBJECT(WTPINFO->WTP[0]->WTPModel);
					CW_FREE_OBJECT(WTPINFO->WTP[0]->APCode);
					CW_FREE_OBJECT(WTPINFO->WTP[0]->ver);
					CW_FREE_OBJECT(WTPINFO->WTP[0]->codever);
					if(WTPINFO->WTP[0]){
						free(WTPINFO->WTP[0]);
						WTPINFO->WTP[0] = NULL;
					}
				}
				if(WTPINFO){
					free(WTPINFO->WTP);
					WTPINFO->WTP = NULL;
					free(WTPINFO);
					WTPINFO = NULL;
				}
			}
			break;			
			case 9 :{
				int i = 0;
				if(WTPINFO){
					for (i = 0; i < ((WTPINFO->num)); i++) {	
						if((WTPINFO)&&(WTPINFO->WTP)&&(WTPINFO->WTP[i])){
							CW_FREE_OBJECT(WTPINFO->WTP[i]->WTPMAC);
							CW_FREE_OBJECT(WTPINFO->WTP[i]->WTPModel);
							CW_FREE_OBJECT(WTPINFO->WTP[i]->WTPIP);
							CW_FREE_OBJECT(WTPINFO->WTP[i]);
						}						
					}
					CW_FREE_OBJECT(WTPINFO->WTP);
					free(WTPINFO);
					WTPINFO = NULL;
				}
			}
			break;
			case 10 :{	//xiaodawei transplant from 2.0 for telecom test, 20110301
						WID_WTP *phead = NULL;
						WID_WTP *pnext = NULL;
						if((WTPINFO)&&(WTPINFO->WTP_INFO)&&(WTPINFO->WTP_INFO->WTP_LIST)){
						
							phead = WTPINFO->WTP_INFO->WTP_LIST;
							WTPINFO->WTP_INFO->WTP_LIST = NULL;
							
							while(phead != NULL)
							{
									pnext = phead->next;
									
									CW_FREE_OBJECT(phead->WTPMAC);
									CW_FREE_OBJECT(phead->WTPModel);
									CW_FREE_OBJECT(phead->WTPNAME);
									CW_FREE_OBJECT(phead->WTPSN);
									CW_FREE_OBJECT(phead->apply_interface_name);
									CW_FREE_OBJECT(phead->WTPIP);
									CW_FREE_OBJECT(phead);
									phead = pnext;		
							}
						}
						CW_FREE_OBJECT(WTPINFO->WTP_INFO);
						CW_FREE_OBJECT(WTPINFO);
			}
			break;
			case 11 :{
				//fengwenchao add 20110226
				int i = 0;
				if(WTPINFO){
					//printf("6666666666666666666666666666!\n");
					//printf("WTPINFO->num   =  %d !\n",WTPINFO->num);
					for (i = 0; i < (WTPINFO->num); i++) {	
					//printf("777777777777777777777777777777!\n");
						if((WTPINFO)&&(WTPINFO->WTP)&&(WTPINFO->WTP[i])){
							//printf("8888888888888888888888888888888!\n");
							CW_FREE_OBJECT(WTPINFO->WTP[i]->WTPMAC);
							CW_FREE_OBJECT(WTPINFO->WTP[i]->WTPModel);
							CW_FREE_OBJECT(WTPINFO->WTP[i]->APCode);
							CW_FREE_OBJECT(WTPINFO->WTP[i]->ver);
							CW_FREE_OBJECT(WTPINFO->WTP[i]->codever);
							CW_FREE_OBJECT(WTPINFO->WTP[i]);
						}
						if((WTPINFO)&&(WTPINFO->WTP_M_V)&&(WTPINFO->WTP_M_V[i])&&((WTPINFO->WTP_M_V[i]->wtp_model))){
							//printf("99999999999999999999999999999999!\n");
							CW_FREE_OBJECT(WTPINFO->WTP_M_V[i]->wtp_model);
							//printf("qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqq!\n");
							CW_FREE_OBJECT(WTPINFO->WTP_M_V[i]);
							//printf("wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww!\n");
						}
						else if((WTPINFO)&&(WTPINFO->WTP_M_V)&&(WTPINFO->WTP_M_V[i]))
						{
							//printf("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa!\n");
							CW_FREE_OBJECT(WTPINFO->WTP_M_V[i]);
						}
						//printf("eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee!\n");

					}
					/*for(i = 0; i < (WTPINFO->wtp_model_type);i++){
					if((WTPINFO)&&(WTPINFO->WTP_M_V)&&(WTPINFO->WTP_M_V[i])){
							printf("99999999999999999999999999999999!\n");
							CW_FREE_OBJECT(WTPINFO->WTP_M_V[i]->wtp_model);
							CW_FREE_OBJECT(WTPINFO->WTP_M_V[i]);
						}
					}*/
					//printf("0000000000000000000000000000000000000!\n");
					CW_FREE_OBJECT(WTPINFO->WTP);
					CW_FREE_OBJECT(WTPINFO->WTP_M_V);
					free(WTPINFO);
					WTPINFO = NULL;
				}
				//fengwenchao add end
			}
			break;
			case 12 :{
				//fengwenchao add 20110226
				int i = 0;
				if(WTPINFO){
					for (i = 0; i < (WTPINFO->num); i++) {	

						if((WTPINFO)&&(WTPINFO->WTP)&&(WTPINFO->WTP[i])){

							CW_FREE_OBJECT(WTPINFO->WTP[i]->WTPMAC);
							CW_FREE_OBJECT(WTPINFO->WTP[i]->WTPModel);
							CW_FREE_OBJECT(WTPINFO->WTP[i]->APCode);
							CW_FREE_OBJECT(WTPINFO->WTP[i]->ver);
							CW_FREE_OBJECT(WTPINFO->WTP[i]->codever);
							CW_FREE_OBJECT(WTPINFO->WTP[i]);
						}
						if((WTPINFO)&&(WTPINFO->WTP_M_V)&&(WTPINFO->WTP_M_V[i])&&((WTPINFO->WTP_M_V[i]->wtp_version))){

							CW_FREE_OBJECT(WTPINFO->WTP_M_V[i]->wtp_version);

							CW_FREE_OBJECT(WTPINFO->WTP_M_V[i]);

						}
						else if((WTPINFO)&&(WTPINFO->WTP_M_V)&&(WTPINFO->WTP_M_V[i]))
						{

							CW_FREE_OBJECT(WTPINFO->WTP_M_V[i]);
						}

					}

					CW_FREE_OBJECT(WTPINFO->WTP);
					CW_FREE_OBJECT(WTPINFO->WTP_M_V);
					free(WTPINFO);
					WTPINFO = NULL;
				}
				//fengwenchao add end	
			}
			break;
			case 13 :{

			}
			break;
			case 14 :{

			}
			break;
			default : break;
			
		}
}
void dcli_wtp_free_fun_two(char *DBUS_METHOD,DCLI_WTP_API_GROUP_TWO*LIST)
{
	int i = 0;
	int dcli_sn = 0;
	dcli_sn = dcli_wtp_method_parse_two(DBUS_METHOD);
//	if(ret == 0){
		switch(dcli_sn){
			case 1 :{
				if((LIST)&&(LIST->WTP)){
					CW_FREE_OBJECT(LIST->WTP[0]);
					CW_FREE_OBJECT(LIST->WTP);
				}
			}
			break;
			case 2 :{
				if((LIST)&&(LIST->WTP)){
					CW_FREE_OBJECT(LIST->WTP[0]);
					CW_FREE_OBJECT(LIST->WTP);
				}
			}
			break;
			case 3 :{
				if((LIST)&&(LIST->WTP)){
					CW_FREE_OBJECT(LIST->WTP[0]);		
					CW_FREE_OBJECT(LIST->WTP);
				}
			}
			break;
			case 4 :{
				if((LIST)&&(LIST->WTP)&&(LIST->WTP[0])){
					if(LIST->WTP[0]->WTP_Radio[0] != NULL){
						for(i= 0;i<LIST->WTP[0]->WTP_Radio[0]->bss_num;i++){
							
							CW_FREE_OBJECT(LIST->WTP[0]->WTP_Radio[0]->BSS[i]);
						}
					}
					CW_FREE_OBJECT(LIST->WTP[0]->WTP_Radio[0]);
				}
				if((LIST)&&(LIST->WTP)){
					CW_FREE_OBJECT(LIST->WTP[0]);
					CW_FREE_OBJECT(LIST->WTP);
				}
			}
			break;			
			case 5 :{
				if((LIST)&&(LIST->WTP)&&(LIST->WTP[0])&&(LIST->WTP[0]->WTP_Radio[0])){
					CW_FREE_OBJECT(LIST->WTP[0]->WTP_Radio[0]->BSS[0]);
					CW_FREE_OBJECT(LIST->WTP[0]->WTP_Radio[0]);
				}
				if((LIST)&&(LIST->WTP)){
					CW_FREE_OBJECT(LIST->WTP[0]);
					CW_FREE_OBJECT(LIST->WTP);
				}
			}
			break;
			case 6 :{
				if((LIST)&&(LIST->WTP)){
					CW_FREE_OBJECT(LIST->WTP[0]);
					CW_FREE_OBJECT(LIST->WTP);
				}
			}
			break;
			case 7 :{
				if((LIST)&&(LIST->WTP[0])){
				for(i=0;((i<LIST->WTP[0]->radio_num)&&(i < L_RADIO_NUM));i++){
					if((LIST)&&(LIST->WTP)&&(LIST->WTP[0])&&(LIST->WTP[0]->WTP_Radio)&&(LIST->WTP[0]->WTP_Radio[i])){
						CW_FREE_OBJECT(LIST->WTP[0]->WTP_Radio[i]->BSS[0]);
						CW_FREE_OBJECT(LIST->WTP[0]->WTP_Radio[i]);
						}
					}
				}
				if((LIST)&&(LIST->WTP)){
					CW_FREE_OBJECT(LIST->WTP[0]);
					CW_FREE_OBJECT(LIST->WTP);
				}
			}
			break;
			case 8 :{
				if((LIST)&&(LIST->WTP)){
					CW_FREE_OBJECT(LIST->WTP[0]);	
					CW_FREE_OBJECT(LIST->WTP);	
				}
			}
			break;			
			case 9 :{
				if((LIST)&&(LIST->WTP)){
					CW_FREE_OBJECT(LIST->WTP[0]);
					CW_FREE_OBJECT(LIST->WTP);
				}
			}
			break;
			case 10 :{
				if((LIST)&&(LIST->WTP)){
					CW_FREE_OBJECT(LIST->WTP[0]);
					CW_FREE_OBJECT(LIST->WTP);
				}
			}
			break;
			case 11 :{

			}
			break;
			case 12 :{

			}
			break;
			case 13 :{

			}
			break;
			case 14 :{

			}
			break;
			default : break;
			
		}
//	}
//	else{
		CW_FREE_OBJECT(LIST);
//		}
}
void dcli_wtp_free_fun_three(char *DBUS_METHOD,DCLI_WTP_API_GROUP_THREE*LIST)
{
	int i = 0;
	int dcli_sn = 0;
	dcli_sn = dcli_wtp_method_parse_three(DBUS_METHOD);
//	if(haveret == 0){
//		if(ret == 0){
			switch(dcli_sn){
				case 1 :{
				}
				break;
				case 2 :{

				}
				break;
				case 3 :{

				}
				break;
				case 4 :{
					if(LIST){
						CW_FREE_OBJECT(LIST->netid);
					}
				}
				break;			
				case 5 :{
					if(LIST){
						for(i=0;i<LIST->wlan_num;i++)
						{
							if(LIST->WLAN)
							CW_FREE_OBJECT(LIST->WLAN[i]);
						}
						CW_FREE_OBJECT(LIST->WLAN);
					}
				}
				break;
				case 6 :{

				}
				break;
				case 7 :{
					if(LIST){
						CW_FREE_OBJECT(LIST->model);
					}
				}
				break;
				case 8 :{
					if(LIST){
						CW_FREE_OBJECT(LIST->model);
						CW_FREE_OBJECT(LIST->versionname);
						CW_FREE_OBJECT(LIST->versionpath);
					}
				}
				break;			
				case 9 :{

				}
				break;
				case 10 :{

				}
				break;
				case 11 :{

				}
				break;
				case 12 :{

				}
				break;
				case 13 :{

				}
				break;
				case 14 :{

				}
				break;
				default : break;
				
			}
	//	}
	/*	else{
			CW_FREE_OBJECT(LIST);
			}*/
//	}
//	else{
		switch(dcli_sn){
			case 9 :{

			}
			break;
			default :break;

		}

//	}
	
	CW_FREE_OBJECT(LIST);
}
void dcli_wtp_free_ap_update_config_fun(DCLI_WTP_API_GROUP_THREE*LIST){
	while(LIST){
		DCLI_WTP_API_GROUP_THREE *tmp = LIST;
		LIST = LIST->next;
		CW_FREE_OBJECT(tmp->model);
		CW_FREE_OBJECT(tmp->versionname);
		CW_FREE_OBJECT(tmp->versionpath);
		CW_FREE_OBJECT(tmp);
	}
}

/*liuzhenhua append 2010-05-28*/
/*mib table 25*/
void dcli_free_wtp_sta_info_head(struct WtpStaInfo*head)
{
	struct WtpStaInfo * sta_temp=NULL;
	while(head){
		sta_temp=head;
		head=head->next;

		if(sta_temp->wtpMacAddr){
			free(sta_temp->wtpMacAddr);
			sta_temp->wtpMacAddr=NULL;
			}
		if(sta_temp->wtpTerminalMacAddr){
			free(sta_temp->wtpTerminalMacAddr);
			sta_temp->wtpTerminalMacAddr=NULL;
			}
		if(sta_temp->wtpMacTermAPReceivedStaSignalStrength){
			free(sta_temp->wtpMacTermAPReceivedStaSignalStrength);
			sta_temp->wtpMacTermAPReceivedStaSignalStrength=NULL;
			}
		if(sta_temp->wtpterminalaccesstime){
			free(sta_temp->wtpterminalaccesstime);
			sta_temp->wtpterminalaccesstime=NULL;
			}
		if(sta_temp->wtpName){
			free(sta_temp->wtpName);
			sta_temp->wtpName = NULL;
		}
		if(sta_temp->identity){
			free(sta_temp->identity);
			sta_temp->identity = NULL;
		}
		free(sta_temp);
		sta_temp=NULL;
		}
}

/*liuzhenhua append 2010-05-27*/
/*mib table 24*/
void dcli_free_wtp_wlan_data_pktsinfo_head(struct WtpWlanDataPktsInfo* head)
{
	struct WtpWlanDataPktsInfo *wtp_temp;
	struct WlanDataPktsInfo * wlan_temp;
	while(head){
		wtp_temp=head;
		head=head->next;

		if(wtp_temp->wtpMacAddr){
			free(wtp_temp->wtpMacAddr);
			wtp_temp->wtpMacAddr=NULL;	
			}
		
		while(wtp_temp->wlan_list){
			wlan_temp=wtp_temp->wlan_list;
			wtp_temp->wlan_list=wlan_temp->next;
			if(wlan_temp->wtpWirelessWrongPktsRate){
				free(wlan_temp->wtpWirelessWrongPktsRate);
				wlan_temp->wtpWirelessWrongPktsRate=NULL;
				}
			if(wlan_temp->wtpNetWiredRxWrongPktsRate){
				free(wlan_temp->wtpNetWiredRxWrongPktsRate);
				wlan_temp->wtpNetWiredRxWrongPktsRate=NULL;
				}
			if(wlan_temp->wtpNetWiredTxWrongPktsRate){
				free(wlan_temp->wtpNetWiredTxWrongPktsRate);
				wlan_temp->wtpNetWiredTxWrongPktsRate=NULL;
				}
			free(wlan_temp);
			wlan_temp=NULL;
			}
		free(wtp_temp);
		wtp_temp=NULL;
		}
}

void dcli_free_wtp_wlan_radio_info_head(struct WtpWlanRadioInfo* head){

	struct WtpWlanRadioInfo *wtp_temp = NULL;
    while(head){
		wtp_temp=head;
		head=head->next;
        free(wtp_temp);
		wtp_temp=NULL;
	  }

}

/*mib table 23*/
void dcli_free_wtp_terminalinfo_head(struct WtpTerminalInfo *head) 
{
	struct WtpTerminalInfo *wtp_tmp = NULL; 
	struct Wtp_TerminalInfo *term_tmp = NULL;
		
	while(head){
		wtp_tmp=head;
		head=wtp_tmp->next;
		
		if(wtp_tmp->wtpMacAddr){
			free(wtp_tmp->wtpMacAddr);
			wtp_tmp->wtpMacAddr = NULL;
		}
		
		while(wtp_tmp->terminalInfo_list){
			term_tmp=wtp_tmp->terminalInfo_list;
			wtp_tmp->terminalInfo_list=term_tmp->next;
			
			if(term_tmp->wtpTerminalMacAddr){
				free(term_tmp->wtpTerminalMacAddr);
				term_tmp->wtpTerminalMacAddr = NULL;
			}
			if(term_tmp->wtpStaSSIDName){
				free(term_tmp->wtpStaSSIDName);
				term_tmp->wtpStaSSIDName = NULL;
			}
			free(term_tmp);
			term_tmp=NULL;
			}
		
		free(wtp_tmp);
		wtp_tmp=NULL;
		}

	return ;
}

/*table 1*/
void dcli_free_WtpBasicInfo(struct WtpBasicInfo *WtpNode)
{
	struct WtpBasicInfo *tmp = NULL;

	if(WtpNode == NULL)
		return ;
	
	if(WtpNode->WtpBasicInfo_last != NULL) {
		WtpNode->WtpBasicInfo_last = NULL;
	}

	while(WtpNode->WtpBasicInfo_list != NULL) {
		tmp = WtpNode->WtpBasicInfo_list;
		WtpNode->WtpBasicInfo_list = tmp->next;
		DCLI_FORMIB_FREE_OBJECT(tmp->wtpMacAddr);
		DCLI_FORMIB_FREE_OBJECT(tmp->wtpDomain);
		DCLI_FORMIB_FREE_OBJECT(tmp->wtpDevTypeNum);

		DCLI_FORMIB_FREE_OBJECT(tmp->wtpSysSoftName);
		DCLI_FORMIB_FREE_OBJECT(tmp->wtpVersionInfo);
		DCLI_FORMIB_FREE_OBJECT(tmp->wtpSysVersion);
		
		DCLI_FORMIB_FREE_OBJECT(tmp->wtpPosInfo);
		DCLI_FORMIB_FREE_OBJECT(tmp->wtpProduct);
		DCLI_FORMIB_FREE_OBJECT(tmp->wtpSeriesNum);
		
		DCLI_FORMIB_FREE_OBJECT(tmp->wtpDevName);
		DCLI_FORMIB_FREE_OBJECT(tmp->wtpModel);
		DCLI_FORMIB_FREE_OBJECT(tmp->wtpSysSoftProductor);
		tmp->next = NULL;
		free(tmp);
		tmp = NULL;
	}
	free(WtpNode);
	WtpNode = NULL;
	return ;
	
}
/*table 3*/
void dcli_free_WtpCollectInfo(struct WtpCollectInfo *WtpNode)
{
	struct WtpCollectInfo *tmp = NULL;

	if(WtpNode == NULL)
		return ;
	
	if(WtpNode->WtpCollectInfo_last != NULL){
		WtpNode->WtpCollectInfo_last = NULL;
	}
	
	while(WtpNode->WtpCollectInfo_list != NULL){
		tmp = WtpNode->WtpCollectInfo_list;
		WtpNode->WtpCollectInfo_list = tmp->next;
		
		DCLI_FORMIB_FREE_OBJECT(tmp->wtpMacAddr);

		tmp->next = NULL;
		free(tmp);
		tmp =NULL;
	}
	free(WtpNode);
	WtpNode = NULL;
	return ;
	
}
/*table 2*/
void dcli_free_WtpParaInfo(struct WtpParaInfo *WtpNode)
{
	struct WtpParaInfo *tmp = NULL;

	if(WtpNode == NULL)
		return ;
	
	if(WtpNode->WtpParaInfo_last != NULL) {
		WtpNode->WtpParaInfo_last = NULL;
	}
	while(WtpNode->WtpParaInfo_list != NULL) {
		tmp = WtpNode->WtpParaInfo_list;
		WtpNode->WtpParaInfo_list = tmp->next;
	
		DCLI_FORMIB_FREE_OBJECT(tmp->wtpMacAddr);		
		DCLI_FORMIB_FREE_OBJECT(tmp->wtpNetElementCode);
		DCLI_FORMIB_FREE_OBJECT(tmp->wtpCurBssid);
		DCLI_FORMIB_FREE_OBJECT(tmp->wtpModel);
		DCLI_FORMIB_FREE_OBJECT(tmp->wtpGateAddr);
		DCLI_FORMIB_FREE_OBJECT(tmp->wtpIfType);
		DCLI_FORMIB_FREE_OBJECT(tmp->wtpAddrMask);
		DCLI_FORMIB_FREE_OBJECT(tmp->wtpMacConApAc);
		DCLI_FORMIB_FREE_OBJECT(tmp->wtpIP);
		DCLI_FORMIB_FREE_OBJECT(tmp->wtpName);
		
		tmp->next = NULL;
		free(tmp);
		tmp =NULL;
	}
	free(WtpNode);
	WtpNode = NULL;
	return ;
}
/*table 4-2*/
void dcli_free_wtp_wireless_ifstats_radio_Info(struct WtpWirelessIfstatsInfo_radio *RadioNode)
{
	struct WtpWirelessIfstatsInfo_radio *tmp = NULL;
	struct WtpWirelessIfstatsInfo_radio *t = RadioNode;

	if(RadioNode == NULL)
		return ;
	
	if(RadioNode->WtpWirelessIfstatsInfo_radio_last != NULL) {
		RadioNode->WtpWirelessIfstatsInfo_radio_last = NULL;
	}
	
	while(t != NULL) {
		tmp = t;
		t = t->next;
		tmp->next =NULL;
		free(tmp);
	}
	return ;
}
/*table 4*/
void dcli_free_wtp_wireless_ifstats_Info(struct WtpWirelessIfstatsInfo *WtpNode)
{
	struct WtpWirelessIfstatsInfo *tmp = NULL;
	struct Neighbor_AP_ELE *tmp_neighbor = NULL; //fengwenchao add 20110523
	if(WtpNode == NULL)
		return ;
	
	if(WtpNode->WtpWirelessIfstatsInfo_last != NULL) {
		WtpNode->WtpWirelessIfstatsInfo_last = NULL;
	}
	while(WtpNode->WtpWirelessIfstatsInfo_list != NULL) {
		tmp = WtpNode->WtpWirelessIfstatsInfo_list;
		WtpNode->WtpWirelessIfstatsInfo_list = tmp->next;
	
		DCLI_FORMIB_FREE_OBJECT(tmp->wtpMacAddr);	
		/*fengwenchao add 20110521*/
		if(tmp->neighbor_wtp->neighborapInfos_last!= NULL) 
		{
			tmp->neighbor_wtp->neighborapInfos_last = NULL;
		}
		while(tmp->neighbor_wtp != NULL)
		{
			tmp_neighbor = tmp->neighbor_wtp;
			tmp->neighbor_wtp = tmp->neighbor_wtp->next;
			//DCLI_FORMIB_FREE_OBJECT(tmp_neighbor->ESSID);
			tmp_neighbor->next = NULL;
			DCLI_FORMIB_FREE_OBJECT(tmp_neighbor);
		}
		/*fengwenchao add end*/
		tmp->next = NULL;
		dcli_free_wtp_wireless_ifstats_radio_Info(tmp->wireless_sub_radio_head);
		free(tmp);
		tmp =NULL;
		
	}
	free(WtpNode);
	WtpNode = NULL;
	return ;
}
/*table 5*/
void dcli_free_wtp_device_Info(struct WtpDeviceInfo *WtpNode)
{
	struct WtpDeviceInfo *tmp = NULL;

	if(WtpNode == NULL)
		return ;
	
	if(WtpNode->WtpDeviceInfo_last != NULL) {
		WtpNode->WtpDeviceInfo_last = NULL;
	}
	while(WtpNode->WtpDeviceInfo_list != NULL) {
		tmp = WtpNode->WtpDeviceInfo_list;
		WtpNode->WtpDeviceInfo_list = tmp->next;
	
		DCLI_FORMIB_FREE_OBJECT(tmp->wtpMacAddr);	
		DCLI_FORMIB_FREE_OBJECT(tmp->wtpModel);
		DCLI_FORMIB_FREE_OBJECT(tmp->WtpIP);
		
		tmp->next = NULL;
		free(tmp);
		tmp=NULL;
	}
	free(WtpNode);
	WtpNode = NULL;
	return ;
}
void dcli_free_wtp_data_pkts_Info(struct WtpDataPktsInfo *WtpNode)
{
	struct WtpDataPktsInfo *tmp = NULL;

	if(WtpNode == NULL)
		return ;
	
	if(WtpNode->WtpDataPktsInfo_last != NULL) {
		WtpNode->WtpDataPktsInfo_last = NULL;
	}
	while(WtpNode->WtpDataPktsInfo_list != NULL) {
		tmp = WtpNode->WtpDataPktsInfo_list;
		WtpNode->WtpDataPktsInfo_list = tmp->next;
	
		DCLI_FORMIB_FREE_OBJECT(tmp->wtpMacAddr);		
		
		tmp->next = NULL;
		free(tmp);
		tmp =NULL;
	}
	free(WtpNode);
	WtpNode = NULL;
	return ;
}

void dcli_free_wtp_stats_Info(struct WtpStatsInfo *WtpNode)
{
	struct WtpStatsInfo *tmp = NULL;

	if(WtpNode == NULL)
		return ;
	
	if(WtpNode->WtpStatsInfo_last != NULL) {
		WtpNode->WtpStatsInfo_last = NULL;
	}
	while(WtpNode->WtpStatsInfo_list != NULL) {
		tmp = WtpNode->WtpStatsInfo_list;
		WtpNode->WtpStatsInfo_list = tmp->next;
	
		DCLI_FORMIB_FREE_OBJECT(tmp->wtpMacAddr);		
		DCLI_FORMIB_FREE_OBJECT(tmp->wtpModel);
		
		tmp->next = NULL;
		free(tmp);
		tmp =NULL;
	}
	free(WtpNode);
	WtpNode = NULL;
	return ;
}

void dcli_free_wlan_stats_sub_wlan_Info(struct WtpWlanStatsInfo_wlan *WlanNode)
	
{
	struct WtpWlanStatsInfo_wlan *tmp = NULL;
	struct WtpWlanStatsInfo_wlan *t = WlanNode;

	if(WlanNode == NULL)
		return ;
	
	if(WlanNode->WtpWlanStatsInfo_wlan_last != NULL) {
		WlanNode->WtpWlanStatsInfo_wlan_last = NULL;
	}

	while(t != NULL) {
		tmp = t;
		t = t->next;
		DCLI_FORMIB_FREE_OBJECT(tmp->wirelessSSID);	
		DCLI_FORMIB_FREE_OBJECT(tmp->wlan_essid);
		DCLI_FORMIB_FREE_OBJECT(tmp->wtpConRadiusServerIP);
		
		tmp->next = NULL;
		free(tmp);
		tmp = NULL;
	}
	return ;
}

void dcli_free_wlan_stats_Info(struct WtpWlanStatsInfo *WtpNode)
{
	struct WtpWlanStatsInfo *tmp = NULL;

	if(WtpNode == NULL)
		return ;
	
	if(WtpNode->WtpWlanStatsInfo_last != NULL) {
		WtpNode->WtpWlanStatsInfo_last = NULL;
	}
	while(WtpNode->WtpWlanStatsInfo_list != NULL) {
		tmp = WtpNode->WtpWlanStatsInfo_list;
		WtpNode->WtpWlanStatsInfo_list = tmp->next;
	
		DCLI_FORMIB_FREE_OBJECT(tmp->wtpMacAddr);	
		
		if(tmp->WtpWlanStatsInfo_wlan_head)
			dcli_free_wlan_stats_sub_wlan_Info(tmp->WtpWlanStatsInfo_wlan_head);
		
		tmp->next = NULL;
		free(tmp);
		tmp= NULL;
	}
	free(WtpNode);
	WtpNode = NULL;
	return ;
}

void dcli_free_wlan_ssid_stats_sub_wlan_Info(struct SSIDStatsInfo_sub_wlan *WlanNode)
	
{
	struct SSIDStatsInfo_sub_wlan *tmp = NULL;
	struct SSIDStatsInfo_sub_wlan *t = WlanNode;

	if(WlanNode == NULL)
		return ;
	
	if(WlanNode->SSIDStatsInfo_sub_wlan_last != NULL){
		WlanNode->SSIDStatsInfo_sub_wlan_last = NULL;
	}
	while(t != NULL){
		tmp = t;
		t = t->next;
		DCLI_FORMIB_FREE_OBJECT(tmp->wtpSSIDName);
		DCLI_FORMIB_FREE_OBJECT(tmp->wtpSSIDESSID);
		DCLI_FORMIB_FREE_OBJECT(tmp->WlanName);
		tmp->next = NULL;
		free(tmp);
		tmp = NULL;
	}
	return ;
}
//fengwenchao add 20101220
void dcli_free_neighborapInfo_node(struct Neighbor_AP_ELE *neighborapInfos_node)
{
	struct Neighbor_AP_ELE *tmp = NULL;
	struct Neighbor_AP_ELE *t = neighborapInfos_node;  //fengwenchao modify 20110217
	if(neighborapInfos_node == NULL)
		return;
	if(neighborapInfos_node->neighborapInfos_last != NULL)
		{
			neighborapInfos_node->neighborapInfos_last = NULL;
		}
	while(t !=NULL)
		{
			tmp =t;
			t = t->next;
			//DCLI_FORMIB_FREE_OBJECT(tmp->ESSID);      //fengwenchao modify 20110217
			DCLI_FORMIB_FREE_OBJECT(tmp->IEs_INFO);   //fengwenchao modify 20110217
			tmp->next = NULL;
			free(tmp);
			tmp = NULL;
		}
}
//fengwenchao add end

//mahz add 2011.11.9 for GuangZhou Mobile
void dcli_free_wtp_sta_info(struct WtpStationinfo*head)
{
	struct WtpStationinfo * sta_temp=NULL;
	while(head){
		sta_temp=head;
		head=head->next;

		if(sta_temp->wtpMacAddr){
			free(sta_temp->wtpMacAddr);
			sta_temp->wtpMacAddr=NULL;
		}
		free(sta_temp);
		sta_temp=NULL;
	}
}

#if 0 //for old version
void dcli_free_ssid_stats_Info(struct SSIDStatsInfo *WtpNode)
{
	struct SSIDStatsInfo *tmp = NULL;

	if(WtpNode == NULL)
		return ;
	
	if(WtpNode->SSIDStatsInfo_last != NULL){
		WtpNode->SSIDStatsInfo_last = NULL;
	}
	while(WtpNode->SSIDStatsInfo_list != NULL){
		tmp = WtpNode->SSIDStatsInfo_list;
		WtpNode->SSIDStatsInfo_list = tmp->next;
	
		DCLI_FORMIB_FREE_OBJECT(tmp->wtpMacAddr);	
		
		if(tmp->SSIDStatsInfo_sub_wlan_head)
			dcli_free_wlan_ssid_stats_sub_wlan_Info(tmp->SSIDStatsInfo_sub_wlan_head);
		
		tmp->next = NULL;
		free(tmp);
		tmp = NULL;	
	}
	free(WtpNode);
	WtpNode = NULL;
	return ;
}
#endif

void dcli_free_all_wtp_config_info(struct WTP_CONFIG_INFORMATION *WtpNode)
{
	struct WTP_CONFIG_INFORMATION *WtpNodetmp = NULL;
	struct WLAN_INFO *wlannode = NULL;
	struct WLAN_INFO *wlannodetmp = NULL;
	struct ifi *ifinode = NULL;
	struct ifi *ifitmp = NULL;
	if(WtpNode == NULL)
		return;
	if(WtpNode->wtp_config_last != NULL){
		WtpNode->wtp_config_last = NULL;
		}
	while(WtpNode->wtp_config_list != NULL)
		{
			WtpNodetmp = WtpNode->wtp_config_list;
			WtpNode->wtp_config_list = WtpNodetmp->next;
			DCLI_FORMIB_FREE_OBJECT(WtpNodetmp->wtpBindPort);
			DCLI_FORMIB_FREE_OBJECT(WtpNodetmp->wtpMacAddr);
			if(WtpNodetmp->wlan_info_head)
				{
       				wlannode = WtpNodetmp->wlan_info_head;
	   				while(wlannode != NULL)
						{
				           wlannodetmp = wlannode;
						   wlannode = wlannode->next;
						   DCLI_FORMIB_FREE_OBJECT(wlannodetmp->wlanname);
							if(wlannodetmp->ifi_head != NULL)
							{
								ifinode = wlannodetmp->ifi_head;
								while(ifinode != NULL)
								{
									ifitmp = ifinode;
									ifinode = ifinode->ifi_next;
								    //DCLI_FORMIB_FREE_OBJECT(ifitmp->ifi_name);
								   // DCLI_FORMIB_FREE_OBJECT(ifitmp->nas_id);
									ifitmp->ifi_next = NULL;
									free(ifitmp);
									ifitmp = NULL;
								}
							}
				           wlannodetmp->next = NULL;
					       free(wlannodetmp);
					       wlannodetmp = NULL;
					   }	   
    			}		
			WtpNodetmp->next = NULL;
			free(WtpNodetmp);
			WtpNodetmp = NULL;	
		}
	free(WtpNode);
	WtpNode = NULL;
	return ;
}
//fengwenchao add end
/* zhangshu copy from 1.2, 2010-09-13 */
void dcli_free_ssid_stats_Info(struct SSIDStatsInfo *WtpNode)
{
	struct SSIDStatsInfo *WtpNodetmp = NULL;
	 struct SSIDStatsInfo_Radioid_info *RadioIdNode = NULL;
    struct SSIDStatsInfo_Radioid_info  *RadioIdNodetmp = NULL;
	if(WtpNode == NULL)
		return ;
	
	if(WtpNode->SSIDStatsInfo_last != NULL){
		WtpNode->SSIDStatsInfo_last = NULL;
	}
	while(WtpNode->SSIDStatsInfo_list != NULL){
		WtpNodetmp = WtpNode->SSIDStatsInfo_list;
		WtpNode->SSIDStatsInfo_list = WtpNodetmp->next;
	
		DCLI_FORMIB_FREE_OBJECT(WtpNodetmp->wtpMacAddr);	
		if(WtpNodetmp->SSIDStatsInfo_Radioid_info_head){
               RadioIdNode = WtpNodetmp->SSIDStatsInfo_Radioid_info_head;
			   while(RadioIdNode != NULL){
                   RadioIdNodetmp = RadioIdNode;
				   RadioIdNode = RadioIdNode->next;
				   if(RadioIdNodetmp->SSIDStatsInfo_sub_wlan_head)
			          dcli_free_wlan_ssid_stats_sub_wlan_Info(RadioIdNodetmp->SSIDStatsInfo_sub_wlan_head);
                   RadioIdNodetmp->next = NULL;
			       free(RadioIdNodetmp);
			       RadioIdNodetmp = NULL;
			   }
			   
		    }
		
		
		WtpNodetmp->next = NULL;
		free(WtpNodetmp);
		WtpNodetmp = NULL;	
	}
	free(WtpNode);
	WtpNode = NULL;
	return ;
}
//fengwenchao add 20101220
void dcli_free_allwtp_neighbor_ap(struct allwtp_neighborap *all_wtp_neighbor_apNODE)
{
	struct allwtp_neighborap *tmp = NULL;
	struct allwtp_neighborap_radioinfo *radioinfo = NULL;
	struct allwtp_neighborap_radioinfo *radioinfotmp = NULL;

	if(all_wtp_neighbor_apNODE == NULL)
		return;
	if(all_wtp_neighbor_apNODE->allwtp_neighborap_last != NULL)
		{
			all_wtp_neighbor_apNODE->allwtp_neighborap_last = NULL;
		}
	while(all_wtp_neighbor_apNODE->allwtp_neighborap_list != NULL)
		{
			tmp = all_wtp_neighbor_apNODE->allwtp_neighborap_list;
			all_wtp_neighbor_apNODE->allwtp_neighborap_list = tmp->next;
			if(tmp->radioinfo_head)
				{
					radioinfo = tmp->radioinfo_head;
					while(radioinfo != NULL)
						{
							radioinfotmp = radioinfo;
							radioinfo = radioinfo->next;
							if(radioinfotmp->neighborapInfos_head != NULL)
								{
									dcli_free_neighborapInfo_node(radioinfotmp->neighborapInfos_head);
									radioinfotmp->next = NULL;
									free(radioinfotmp);
									radioinfotmp = NULL;
								}
						}
				}
			tmp->next = NULL;
			free(tmp);
			tmp = NULL;
		}
	free(all_wtp_neighbor_apNODE);
	all_wtp_neighbor_apNODE= NULL;
	return;
}
//fengwenchao add end	
void dcli_free_WtpIfInfo_sub_info(struct WtpIfInfo_sub_info *SubNode)
{
	struct WtpIfInfo_sub_info *tmp = NULL;
		struct WtpIfInfo_sub_info *t = SubNode;
	
		if(SubNode == NULL)
			return ;
		
		if(SubNode->WtpIfInfo_sub_info_last != NULL){
			SubNode->WtpIfInfo_sub_info_last = NULL;
		}
		while(t != NULL){
			tmp = t;
			t = t->next;
			tmp->next = NULL;
			DCLI_FORMIB_FREE_OBJECT(tmp->wtpIfName);
			DCLI_FORMIB_FREE_OBJECT(tmp->wtpIfIntro);
			
			free(tmp);
			tmp = NULL;
		}
		return ;


}

void dcli_free_WtpIfnameInfo(struct WtpIfnameInfo *WtpNode)
{
	struct WtpIfnameInfo *tmp = NULL;
	struct WtpIfInfo_sub_info *tmp_sub = NULL;
	struct WtpIfInfo_sub_info *tmp_sub1 = NULL;

	if(WtpNode == NULL)
		return ;
	
	if(WtpNode->WtpIfnameInfo_last != NULL) {
		WtpNode->WtpIfnameInfo_last = NULL;
	}

	while(WtpNode->WtpIfnameInfo_list != NULL) {
		tmp = WtpNode->WtpIfnameInfo_list;
		WtpNode->WtpIfnameInfo_list = tmp->next;
		tmp_sub = tmp->WtpIfInfo_sub_info_head;
		while(tmp_sub){
			tmp_sub1 = tmp_sub->next;
			DCLI_FORMIB_FREE_OBJECT(tmp_sub);	
			tmp_sub = tmp_sub1;
		}
		DCLI_FORMIB_FREE_OBJECT(tmp->wtpMacAddr);
		DCLI_FORMIB_FREE_OBJECT(tmp->wtpModel);
		
		tmp->next = NULL;
		free(tmp);
		tmp = NULL;
	}
	free(WtpNode);
	WtpNode = NULL;
	return ;
	
}
void dcli_free_wtp_Sub_RadioParaInfo(struct Sub_RadioParaInfo *RadioNode)
{
	struct Sub_RadioParaInfo *tmp = NULL;
	struct Sub_RadioParaInfo *t = RadioNode;

	if(RadioNode == NULL)
		return ;
	
	if(RadioNode->Sub_RadioParaInfo_last != NULL) {
		RadioNode->Sub_RadioParaInfo_last = NULL;
	}
	
	while(t != NULL) {
		tmp = t;
		t = t->next;
		tmp->next =NULL;
		free(tmp);
	}
	return ;
}
void dcli_free_wtp_WtpRadioParaInfo(struct WtpRadioParaInfo *WtpNode)
{
	struct WtpRadioParaInfo *tmp = NULL;

	if(WtpNode == NULL)
		return ;
	
	if(WtpNode->WtpRadioParaInfo_last != NULL) {
		WtpNode->WtpRadioParaInfo_last = NULL;
	}
	while(WtpNode->WtpRadioParaInfo_list != NULL) {
		tmp = WtpNode->WtpRadioParaInfo_list;
		WtpNode->WtpRadioParaInfo_list = tmp->next;
	
		DCLI_FORMIB_FREE_OBJECT(tmp->wtpMacAddr);		
		tmp->next = NULL;
		dcli_free_wtp_Sub_RadioParaInfo(tmp->Sub_RadioParaInfo_head);
		free(tmp);
		tmp =NULL;
		
	}
	free(WtpNode);
	WtpNode = NULL;
	return ;
}

void dcli_free_WtpEthPortInfo(struct WtpEthPortInfo *WtpNode)
{
	struct WtpEthPortInfo *tmp = NULL;

	if(WtpNode == NULL)
		return ;
	
	if(WtpNode->WtpEthPortInfo_last != NULL) {
		WtpNode->WtpEthPortInfo_last = NULL;
	}

	while(WtpNode->WtpEthPortInfo_list != NULL) {
		tmp = WtpNode->WtpEthPortInfo_list;
		WtpNode->WtpEthPortInfo_list = tmp->next;
		
		DCLI_FORMIB_FREE_OBJECT(tmp->wtpMacAddr);
		tmp->next = NULL;
		free(tmp);
		tmp = NULL;
	}
	free(WtpNode);
	WtpNode = NULL;
	return ;
	
}
void dcli_free_wtp_Sub_RadioStatsInfo(struct Sub_RadioStatsInfo *RadioNode)
{
	struct Sub_RadioStatsInfo *tmp = NULL;
	struct Sub_RadioStatsInfo *t = RadioNode;

	if(RadioNode == NULL)
		return ;
	
	if(RadioNode->Sub_RadioStatsInfo_last != NULL) {
		RadioNode->Sub_RadioStatsInfo_last = NULL;
	}
	
	while(t != NULL) {
		tmp = t;
		t = t->next;
		tmp->next =NULL;
		free(tmp);
	}
	return ;
}

void dcli_free_wtp_RadioStatsInfo(struct RadioStatsInfo *WtpNode)
{
	struct RadioStatsInfo *tmp = NULL;

	if(WtpNode == NULL)
		return ;
	
	if(WtpNode->RadioStatsInfo_last != NULL) {
		WtpNode->RadioStatsInfo_last = NULL;
	}
	while(WtpNode->RadioStatsInfo_list != NULL) {
		tmp = WtpNode->RadioStatsInfo_list;
		WtpNode->RadioStatsInfo_list = tmp->next;
	
		DCLI_FORMIB_FREE_OBJECT(tmp->wtpMacAddr);		
		tmp->next = NULL;
		dcli_free_wtp_Sub_RadioStatsInfo(tmp->Sub_RadioStatsInfo_head);
		free(tmp);
		tmp =NULL;
		
	}
	free(WtpNode);
	WtpNode = NULL;
	return ;
}
/*16-2*/
void dcli_free_wtp_Sub_WtpConfigRadioInfo(struct Sub_WtpConfigRadioInfo *RadioNode)
{
	struct Sub_WtpConfigRadioInfo *tmp = NULL;
	struct Sub_WtpConfigRadioInfo *t = RadioNode;

	if(RadioNode == NULL)
		return ;
	
	if(RadioNode->Sub_WtpConfigRadioInfo_last != NULL) {
		RadioNode->Sub_WtpConfigRadioInfo_last = NULL;
	}
	
	while(t != NULL){
		tmp = t;
		t = t->next;
		tmp->next =NULL;
		free(tmp);
	}
	return ;
}
/*16*/
void dcli_free_WtpConfigRadioInfo(struct WtpConfigRadioInfo *WtpNode)
{
	struct WtpConfigRadioInfo *tmp = NULL;

	if(WtpNode == NULL)
		return ;
	
	if(WtpNode->WtpConfigRadioInfo_last != NULL) {
		WtpNode->WtpConfigRadioInfo_last = NULL;
	}
	while(WtpNode->WtpConfigRadioInfo_list != NULL) {
		tmp = WtpNode->WtpConfigRadioInfo_list;
		WtpNode->WtpConfigRadioInfo_list = tmp->next;
	
		DCLI_FORMIB_FREE_OBJECT(tmp->wtpMacAddr);	
		
		tmp->next = NULL;
		dcli_free_wtp_Sub_WtpConfigRadioInfo(tmp->Sub_WtpConfigRadioInfo_head);
		free(tmp);
		tmp =NULL;
		
	}
	free(WtpNode);
	WtpNode = NULL;
	return ;
}
/*17*/
void dcli_free_UsrLinkInfo(struct UsrLinkInfo *WtpNode)
{
	struct UsrLinkInfo *tmp = NULL;
	
	if(WtpNode == NULL)
		return ;
	
	if(WtpNode->UsrLinkInfo_last != NULL) {
		WtpNode->UsrLinkInfo_last = NULL;
	}

	while(WtpNode->UsrLinkInfo_list != NULL) {
		tmp = WtpNode->UsrLinkInfo_list;
		WtpNode->UsrLinkInfo_list = tmp->next;
		
		DCLI_FORMIB_FREE_OBJECT(tmp->wtpMacAddr);
		tmp->next = NULL;
		free(tmp);
		tmp = NULL;
	}
	free(WtpNode);
	WtpNode = NULL;
	return ;
	
}
/*19*/
void dcli_free_wtp_WiredIfStats_Info(struct WtpWiredIfStatsInfo *WtpNode)
{
	struct WtpWiredIfStatsInfo *tmp = NULL;
	struct WiredIfStatsInfo *tmp2 = NULL;

	if(WtpNode == NULL)
		return ;
	
	if(WtpNode->WtpWiredIfStatsInfo_last != NULL) {
		WtpNode->WtpWiredIfStatsInfo_last = NULL;
	}
	while(WtpNode->WtpWiredIfStatsInfo_list != NULL) {
		tmp = WtpNode->WtpWiredIfStatsInfo_list;
		WtpNode->WtpWiredIfStatsInfo_list = tmp->next;
		DCLI_FORMIB_FREE_OBJECT(tmp->wtpMacAddr);	
        /* book modify, 2011-1-19 */
        while(tmp->EthInfo != NULL){
            tmp2 = tmp->EthInfo;
            tmp->EthInfo = tmp2->next;
            tmp2->next = NULL;
            free(tmp2);
            tmp2 = NULL;
        }
		
		tmp->next = NULL;
		free(tmp);
		tmp=NULL;
	}
	free(WtpNode);
	WtpNode = NULL;
	return ;
}
/*20-2*/
void dcli_free_Sub_WtpWirelessIf_Info(struct Sub_WtpWirelessIfInfo *RadioNode)
{
	struct Sub_WtpWirelessIfInfo *tmp = NULL;
	struct Sub_WtpWirelessIfInfo *t = RadioNode;

	if(RadioNode == NULL)
		return ;
	
	if(RadioNode->Sub_WtpWirelessIfInfo_last != NULL) {
		RadioNode->Sub_WtpWirelessIfInfo_last = NULL;
	}
	
	while(t != NULL){
		tmp = t;
		t = t->next;
		DCLI_FORMIB_FREE_OBJECT(tmp->wtpWirelessIfDescr);
		DCLI_FORMIB_FREE_OBJECT(tmp->wtpWirelessIfPhysAddress);
		
		tmp->next =NULL;
		free(tmp);
	}
	return ;
}

/*20*/
void dcli_free_wtp_WtpWirelessIf_Info(struct WtpWirelessIfInfo *WtpNode)
{
	struct WtpWirelessIfInfo *tmp = NULL;

	if(WtpNode == NULL)
		return ;
	
	if(WtpNode->WtpWirelessIfInfo_last != NULL) {
		WtpNode->WtpWirelessIfInfo_last = NULL;
	}
	while(WtpNode->WtpWirelessIfInfo_list != NULL) {
		tmp = WtpNode->WtpWirelessIfInfo_list;
		WtpNode->WtpWirelessIfInfo_list = tmp->next;
		
		DCLI_FORMIB_FREE_OBJECT(tmp->wtpMacAddr);			
		DCLI_FORMIB_FREE_OBJECT(tmp->wtpModel);	
		dcli_free_Sub_WtpWirelessIf_Info(tmp->Sub_WtpWirelessIfInfo_head);
		tmp->next = NULL;
		free(tmp);
		tmp=NULL;
	}
	free(WtpNode);
	WtpNode = NULL;
	return ;
}
/*26*/
void dcli_free_NewWtpWirelessIfInfo(struct NewWtpWirelessIfInfo *WtpNode)
{
	struct NewWtpWirelessIfInfo *tmp = NULL;
	
	if(WtpNode == NULL)
		return ;
	
	if(WtpNode->NewWtpWirelessIfInfo_last != NULL) {
		WtpNode->NewWtpWirelessIfInfo_last = NULL;
	}

	while(WtpNode->NewWtpWirelessIfInfo_list != NULL) {
		tmp = WtpNode->NewWtpWirelessIfInfo_list;
		WtpNode->NewWtpWirelessIfInfo_list = tmp->next;
		
		DCLI_FORMIB_FREE_OBJECT(tmp->wtpMacAddr);	
		tmp->next = NULL;
		free(tmp);
		tmp = NULL;
	}
	
	free(WtpNode);
	WtpNode = NULL;
	return ;
	
}

void dcli_free_new_wtp_wireless_ifstats_radio_Info(struct NewWtpWirelessIfstatsInfo_radio *RadioNode)
{
	struct NewWtpWirelessIfstatsInfo_radio *tmp = NULL;
	struct NewWtpWirelessIfstatsInfo_radio *t = RadioNode;

	if(RadioNode == NULL)
		return ;
	
	if(RadioNode->NewWtpWirelessIfstatsInfo_radio_last != NULL) {
		RadioNode->NewWtpWirelessIfstatsInfo_radio_last = NULL;
	}
	
	while(t != NULL) {
		tmp = t;
		t = t->next;
		tmp->next =NULL;
		free(tmp);
	}
	return ;
}

void dcli_free_new_wtp_wireless_ifstats_Info(struct NewWtpWirelessIfstatsInfo *WtpNode)
{
	struct NewWtpWirelessIfstatsInfo *tmp = NULL;

	if(WtpNode == NULL)
		return ;
	
	if(WtpNode->NewWtpWirelessIfstatsInfo_last != NULL) {
		WtpNode->NewWtpWirelessIfstatsInfo_last = NULL;
	}
	while(WtpNode->NewWtpWirelessIfstatsInfo_list != NULL) {
		tmp = WtpNode->NewWtpWirelessIfstatsInfo_list;
		WtpNode->NewWtpWirelessIfstatsInfo_list = tmp->next;
	
		DCLI_FORMIB_FREE_OBJECT(tmp->wtpMacAddr);		
		tmp->next = NULL;
		dcli_free_new_wtp_wireless_ifstats_radio_Info(tmp->NewWtpWirelessIfstatsInfo_radio_head);
		free(tmp);
		tmp =NULL;
		
	}
	free(WtpNode);
	WtpNode = NULL;
	return ;
}
/*table 29-2 */
void dcli_free_Sub_RogueAPInfo_Info(struct Sub_RogueAPInfo *SubNode)
{
	struct Sub_RogueAPInfo *tmp = NULL;
	struct Sub_RogueAPInfo *t = SubNode;

	if(SubNode == NULL)
		return ;
	
	if(SubNode->Sub_RogueAPInfo_last != NULL) {
		SubNode->Sub_RogueAPInfo_last = NULL;
	}
	
	while(t != NULL) {
		tmp = t;
		t = t->next;
		//DCLI_FORMIB_FREE_OBJECT(tmp->rogueAPEssid);
		DCLI_FORMIB_FREE_OBJECT(tmp->rogueAPElemInfo);
		
		tmp->next =NULL;
		free(tmp);
	}
	return ;
}

/*table 29*/
void dcli_free_wtp_RogueAPInfo_Info(struct RogueAPInfo *WtpNode)
{
	struct RogueAPInfo *tmp = NULL;

	if(WtpNode == NULL)
		return ;
	
	if(WtpNode->RogueAPInfo_last != NULL) {
		WtpNode->RogueAPInfo_last = NULL;
	}
	while(WtpNode->RogueAPInfo_list != NULL) {
		tmp = WtpNode->RogueAPInfo_list;
		WtpNode->RogueAPInfo_list = tmp->next;
	
		DCLI_FORMIB_FREE_OBJECT(tmp->wtpMacAddr);		
		tmp->next = NULL;
		dcli_free_Sub_RogueAPInfo_Info(tmp->Sub_RogueAPInfo_head);
		free(tmp);
		tmp =NULL;
		
	}
	free(WtpNode);
	WtpNode = NULL;
	return ;
}
/*30-2*/
void dcli_free_Sub_SecurityMechInfo(struct Sub_SecurityMechInfo *SubNode)
{
	struct Sub_SecurityMechInfo *tmp = NULL;
	struct Sub_SecurityMechInfo *t = SubNode;

	if(SubNode == NULL)
		return ;
	
	if(SubNode->Sub_SecurityMechInfo_last != NULL) {
		SubNode->Sub_SecurityMechInfo_last = NULL;
	}
	
	while(t != NULL) {
		tmp = t;
		t = t->next;
		DCLI_FORMIB_FREE_OBJECT(tmp->wtpWirelessSecurMechName);
		DCLI_FORMIB_FREE_OBJECT(tmp->wtpWirelessSecurMechSecurPolicyKey);
		tmp->next =NULL;
		free(tmp);
	}
	return ;
}

/*30*/
void dcli_free_SecurityMechInfo(struct SecurityMechInfo *WtpNode)
{
	struct SecurityMechInfo *tmp = NULL;
	
	if(WtpNode == NULL)
		return ;
	
	if(WtpNode->SecurityMechInfo_last != NULL) {
		WtpNode->SecurityMechInfo_last = NULL;
	}

	while(WtpNode->SecurityMechInfo_list != NULL) {
		tmp = WtpNode->SecurityMechInfo_list;
		WtpNode->SecurityMechInfo_list = tmp->next;
		tmp->next = NULL;
		DCLI_FORMIB_FREE_OBJECT(tmp->wtpMacAddr);
		dcli_free_Sub_SecurityMechInfo(tmp->Sub_SecurityMechInfo_head);
		free(tmp);
		tmp = NULL;
	}
	free(WtpNode);
	WtpNode = NULL;
	return ;
	
}
/*33*/
void dcli_free_WtpInfo(struct WtpInfor *WtpNode)
{
	struct WtpInfor *tmp = NULL;
	
	if(WtpNode == NULL)
		return ;
	
	if(WtpNode->WtpInfor_last != NULL) {
		WtpNode->WtpInfor_last = NULL;
	}

	while(WtpNode->WtpInfor_list != NULL) {
		tmp = WtpNode->WtpInfor_list;
		WtpNode->WtpInfor_list = tmp->next;
		tmp->next = NULL;
		DCLI_FORMIB_FREE_OBJECT(tmp->wtpMacAddr);
		free(tmp);
		tmp = NULL;
	}
	free(WtpNode);
	WtpNode = NULL;
	return ;
	
}
/*xdw add for free WtpNetworkInfo, 20101215*/
void dcli_free_WtpNetworkInfo(struct WtpNetworkInfo *WtpNode)
{
	struct WtpNetworkInfo *tmp = NULL;
	
	if(WtpNode == NULL)
		return ;
	
	if(WtpNode->WtpNetworkInfo_last != NULL) {
		WtpNode->WtpNetworkInfo_last = NULL;
	}

	while(WtpNode->WtpNetworkInfo_list != NULL) {
		tmp = WtpNode->WtpNetworkInfo_list;
		WtpNode->WtpNetworkInfo_list = tmp->next;
		tmp->next = NULL;
		DCLI_FORMIB_FREE_OBJECT(tmp->WTPMAC);
		DCLI_FORMIB_FREE_OBJECT(tmp->WTPIP);
		free(tmp);
		tmp = NULL;
	}
	free(WtpNode);
	WtpNode = NULL;
	return ;
	
}

void dcli_free_WtpAthStatisticInfo(struct WtpAthStatisticInfo* WtpHead){
	struct WtpAthStatisticInfo* tmp=NULL;
	while(WtpHead){
		tmp=WtpHead;
		WtpHead=WtpHead->next;

		if(tmp->wtpMacAddr){
			free(tmp->wtpMacAddr);
			tmp->wtpMacAddr=NULL;
		}
		free(tmp);
		tmp=NULL;
	}
	return ;
}
void dcli_free_WtpList(struct WtpList *WtpNode)
{
	struct WtpList *tmp = NULL;
	
	if(WtpNode == NULL)
		return ;
	
	if(WtpNode->WtpList_last != NULL) {
		WtpNode->WtpList_last = NULL;
	}

	while(WtpNode->WtpList_list != NULL){
		tmp = WtpNode->WtpList_list;
		WtpNode->WtpList_list = tmp->next;
		tmp->next = NULL;
		//DCLI_FORMIB_FREE_OBJECT(tmp->wtpMacAddr);
		free(tmp);
		tmp = NULL;
	}
	
	free(WtpNode);
	WtpNode = NULL;
	return ;
}

/*void dcli_free_WtpList(struct WtpList* WtpHead){
	struct WtpList* tmp=NULL;
	while(WtpHead){
		tmp=WtpHead;
		WtpHead=WtpHead->next;
		free(tmp);
		tmp=NULL;
	}
	return ;
}*/


int dcli_wtp_list_set_dhcp_snooping(int index,int localid,int policy,struct tag_wtpid_list * wtplist,DBusConnection *dcli_dbus_connection){
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter iter_array;
	DBusError err;
	int i;
	struct tag_wtpid *tmp = NULL;
	unsigned int num;	
	int ret;
	dbus_error_init(&err);

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WTP_LIST_METHOD_SET_DHCP_SNOOPING);

	num = wtplist->count;
	dbus_message_iter_init_append (query, &iter);
			

	dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &policy);

		// Total slot count
	dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &num);
	tmp = wtplist->wtpidlist;
	dbus_message_iter_open_container (&iter,
									DBUS_TYPE_ARRAY,
									DBUS_STRUCT_BEGIN_CHAR_AS_STRING
											DBUS_TYPE_UINT32_AS_STRING
									DBUS_STRUCT_END_CHAR_AS_STRING,
									&iter_array);
	
	for(i = 0; i < num; i++){			
		DBusMessageIter iter_struct;		/* Huangleilei add it by AXSSZFI-1621: dbus may be not accept more than 255 elements, as string add to it */
		dbus_message_iter_open_container(&iter_array, DBUS_TYPE_STRUCT,NULL, &iter_struct);
		dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &(tmp->wtpid));
		dbus_message_iter_close_container (&iter_array, &iter_struct);
		tmp = tmp->next;
	}
	dbus_message_iter_close_container (&iter, &iter_array);
	#if 0
	for(i = 0; i < num; i++){
		
		dbus_message_iter_append_basic (&iter,
											 DBUS_TYPE_UINT32,
											 &(tmp->wtpid));
		tmp = tmp->next;

	}	
	#endif
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);


	if (NULL == reply) {
		printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return -1;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	dbus_message_unref(reply);	

	destroy_input_wtp_list(wtplist);
	return ret;
}

int dcli_wtp_set_dhcp_snooping(int index,int localid,int policy,unsigned int wtpid,DBusConnection *dcli_dbus_connection){
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	int i;
	int ret = 0;
	dbus_error_init(&err);

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WTP_METHOD_SET_DHCP_SNOOPING);

	dbus_message_iter_init_append (query, &iter);
			

	dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &policy);

		// Total slot count
	dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &wtpid);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);


	if (NULL == reply) {
		printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return -1;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	dbus_message_unref(reply);	

	return ret;
}

//fengwenchao add 20110221
int set_wid_rogue_danger_unsafe_attack_trap_state_cmd_trap(int index,int localid,int set_trap_type,int wid_trap_state,DBusConnection *dcli_dbus_connection,unsigned int wtp_id)
{
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	dbus_error_init(&err);	
	int ret = 0;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHD_SET_WID_ROGUE_DANGER_UNSAFE_ATTACK_TRAP_STATE);
                                                                   
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&set_trap_type,
							 DBUS_TYPE_UINT32,&wid_trap_state,
							 DBUS_TYPE_UINT32,&wtp_id,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
		if (NULL == reply) {
		printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return -1;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	dbus_message_unref(reply);	

	return ret;
}
//fengwenchao add end

int dcli_wtp_list_sta_info_report(int index,int localid,int policy,struct tag_wtpid_list * wtplist,DBusConnection *dcli_dbus_connection){
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter iter_array;
	DBusError err;
	int i;
	struct tag_wtpid *tmp = NULL;
	unsigned int num;	
	int ret;
	dbus_error_init(&err);

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WTP_LIST_METHOD_STA_INFO_REPORT);

	num = wtplist->count;
	dbus_message_iter_init_append (query, &iter);
			

	dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &policy);

		// Total slot count
	dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &num);
	tmp = wtplist->wtpidlist;
	dbus_message_iter_open_container (&iter,
									DBUS_TYPE_ARRAY,
									DBUS_STRUCT_BEGIN_CHAR_AS_STRING
											DBUS_TYPE_UINT32_AS_STRING
									DBUS_STRUCT_END_CHAR_AS_STRING,
									&iter_array);
	
	for(i = 0; i < num; i++){			
		DBusMessageIter iter_struct;		/* Huangleilei add it by AXSSZFI-1621: dbus may be not accept more than 255 elements, as string add to it */
		dbus_message_iter_open_container(&iter_array, DBUS_TYPE_STRUCT, NULL, &iter_struct);
		dbus_message_iter_append_basic(&iter_struct, DBUS_TYPE_UINT32, &(tmp->wtpid));
		dbus_message_iter_close_container (&iter_array, &iter_struct);
		tmp = tmp->next;
	}
	dbus_message_iter_close_container (&iter, &iter_array);
	#if 0
	for(i = 0; i < num; i++){
		
		dbus_message_iter_append_basic (&iter,
											 DBUS_TYPE_UINT32,
											 &(tmp->wtpid));
		tmp = tmp->next;

	}	
	#endif
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);


	if (NULL == reply) {
		printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return -1;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	dbus_message_unref(reply);	

	destroy_input_wtp_list(wtplist);
	return ret;
}

int dcli_wtp_sta_info_report(int index,int localid,int policy,unsigned int wtpid,DBusConnection *dcli_dbus_connection){
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	int i;
	int ret;
	dbus_error_init(&err);

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WTP_METHOD_STA_INFO_REPORT);

	dbus_message_iter_init_append (query, &iter);
			

	dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &policy);

		// Total slot count
	dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &wtpid);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);


	if (NULL == reply) {
		printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return -1;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	dbus_message_unref(reply);	

	return ret;
}


void* dcli_show_wtp_trap_threshod(int index,int localid,unsigned int wtpid,int *ret,DBusConnection *dcli_dbus_connection)
{	
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusError err;
	int i = 0;
	WID_TRAP_THRESHOLD *DCLI_INFIO = NULL;
	CW_CREATE_OBJECT_ERR(DCLI_INFIO, WID_TRAP_THRESHOLD, return NULL;);	
	DCLI_INFIO->cpu = 0;
	DCLI_INFIO->memoryuse = 0;
	DCLI_INFIO->collecttime = 0;
	DCLI_INFIO->rogue_ap_threshold = 0;
	DCLI_INFIO->rogue_termi_threshold = 0;
	DCLI_INFIO->wtpid = 0;
	DCLI_INFIO->trap_switch = 99;
	DCLI_INFIO->samechannelrssi_theshold = 0;
	DCLI_INFIO->neighborchannelrssi_theshold = 0;
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SHOW_AP_TRAP_ROGUE_AP_TERMINAL_CPU_MEM_THRESHOLD);

	dbus_error_init(&err);
	
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&wtpid,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		//printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		if(DCLI_INFIO){
			free(DCLI_INFIO);
			DCLI_INFIO = NULL;
		}
		*ret = -1;
		return -1;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&(*ret));
	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&DCLI_INFIO->trap_switch);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&DCLI_INFIO->rogue_ap_threshold);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&DCLI_INFIO->rogue_termi_threshold);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&DCLI_INFIO->cpu);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&DCLI_INFIO->memoryuse);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&DCLI_INFIO->collecttime);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&DCLI_INFIO->samechannelrssi_theshold);
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&DCLI_INFIO->neighborchannelrssi_theshold);

	dbus_message_unref(reply);

	return DCLI_INFIO;
}

int dcli_set_ap_rogueap_rogueterminal_cpu_mem_threshold(int index,int localid,unsigned int policy,unsigned int value,unsigned int wtp_id,DBusConnection *dcli_dbus_connection){
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	int i;
	int ret;
	dbus_error_init(&err);

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SET_WTP_ROGUEAP_ROGUETERMINAL_CPU_MEM_TRAP_THRESHOLD);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&wtp_id,
							 DBUS_TYPE_UINT32,&policy,
							 DBUS_TYPE_UINT32,&value,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);


	if (NULL == reply) {
		printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return -1;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	dbus_message_unref(reply);	

	return ret;
}
void dcli_wtp_free_fun_trap_threshold(WID_TRAP_THRESHOLD*LIST)
{
	CW_FREE_OBJECT(LIST);
}

int dcli_set_ap_trap_switch_able(int index,int localid,unsigned int policy,unsigned int wtp_id,DBusConnection *dcli_dbus_connection){
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	int i;
	int ret;
	dbus_error_init(&err);

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SET_WTP_TRAP_SWITCH);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&wtp_id,
							 DBUS_TYPE_UINT32,&policy,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);


	if (NULL == reply) {
		printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return -1;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	dbus_message_unref(reply);	

	return ret;
}

/*wcl add*/

int dcli_set_ap_seqnum_switch_able(int index,int localid,unsigned int policy,unsigned int wtp_id,DBusConnection *dcli_dbus_connection){
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	int i;
	int ret;
	dbus_error_init(&err);

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SET_WTP_SEQNUM_SWITCH);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&wtp_id,
							 DBUS_TYPE_UINT32,&policy,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);


	if (NULL == reply) {
		printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return -1;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	dbus_message_unref(reply);	

	return ret;
}

/*end*/
int dcli_set_ap_sta_wapi_info_report_able(int index,int localid,unsigned int policy,unsigned int wtp_id,DBusConnection *dcli_dbus_connection){
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	int i;
	int ret;
	dbus_error_init(&err);

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WTP_METHOD_SET_WTP_STA_WAPI_INFO_SWITCH);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&wtp_id,
							 DBUS_TYPE_UINT32,&policy,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);


	if (NULL == reply) {
		printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return -1;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	dbus_message_unref(reply);	

	return ret;
}


int dcli_set_ap_sta_wapi_info_report_reportinterval(int index,int localid,unsigned int interval,unsigned int wtp_id,DBusConnection *dcli_dbus_connection){
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	int i;
	int ret;
	dbus_error_init(&err);

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WTP_METHOD_SET_WTP_STA_WAPI_INFO_REPORTINTERVAL);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&wtp_id,	
							 DBUS_TYPE_UINT32,&interval,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);


	if (NULL == reply) {
		//printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return -1;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	dbus_message_unref(reply);	

	return ret;
}
/*for mib table 1 WtpBasicInfo 20100707 by nl*/
/*liuzhenhua add 2010-05-28*/
/*mib table 25*/
struct WtpStaInfo* show_sta_info_of_all_wtp(int index,int localid,DBusConnection *dcli_dbus_connection, unsigned int *wtp_num, unsigned int *ret)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter iter;
	DBusMessageIter iter_wtp_array;
	DBusMessageIter iter_wtp;
	DBusMessageIter iter_sta_array;
	DBusMessageIter iter_sta;
	
	struct WtpStaInfo * StaHead = NULL;
	struct WtpStaInfo * StaNode = NULL;
	struct WtpStaInfo * StaTail = NULL;

	int i,j;
	int sta_num=0;
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,\
			INTERFACE,ASD_DBUS_STA_METHOD_SHOW_STA_INFO_OF_ALL_WTP);

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);			
	dbus_message_unref(query);

	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		*ret=ASD_DBUS_ERROR;
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,wtp_num);	

	dbus_message_iter_next(&iter);
	dbus_message_iter_recurse(&iter,&iter_wtp_array);

	char * wtp_mac=NULL;
	char * sta_mac=NULL;
	char * snrz = NULL;
	char * stactime=NULL;
	char * wtpName = NULL;
	 
	char * identity = NULL;
	if((*ret == 0)&&(*wtp_num !=0)){
		for(i=0;i<*wtp_num;i++){
			dbus_message_iter_recurse(&iter_wtp_array,&iter_wtp);
			dbus_message_iter_get_basic(&iter_wtp,&wtp_mac);
			dbus_message_iter_next(&iter_wtp);
			dbus_message_iter_get_basic(&iter_wtp,&sta_num);
			
			dbus_message_iter_next(&iter_wtp);
			dbus_message_iter_recurse(&iter_wtp,&iter_sta_array);
			for(j=0;j<sta_num;j++){
				if((StaNode=(struct WtpStaInfo*)malloc(sizeof(struct WtpStaInfo)))==NULL){
					dcli_free_wtp_sta_info_head(StaHead);
					*ret=MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
					}
				memset(StaNode,0,sizeof(struct WtpStaInfo));
				if(StaHead==NULL){
					StaHead=StaNode;
					StaTail=StaNode;
					}
				else{
					StaTail->next=StaNode;
					StaTail=StaNode;
					}
				dbus_message_iter_recurse(&iter_sta_array,&iter_sta);
				dbus_message_iter_get_basic(&iter_sta,&sta_mac);
				dbus_message_iter_next(&iter_sta);
				dbus_message_iter_get_basic(&iter_sta,&StaNode->wtpStaIp);
				dbus_message_iter_next(&iter_sta);
				dbus_message_iter_get_basic(&iter_sta,&StaNode->wtpWirelessClientSNR);
				dbus_message_iter_next(&iter_sta);
				dbus_message_iter_get_basic(&iter_sta,&StaNode->wtpSendTerminalPackMount);
				dbus_message_iter_next(&iter_sta);
				dbus_message_iter_get_basic(&iter_sta,&StaNode->wtpSendTerminalDataPackMount);	//xiaodawei add data pkts sent to terminal
				dbus_message_iter_next(&iter_sta);
				dbus_message_iter_get_basic(&iter_sta,&StaNode->wtpSendTerminalByteMount);
				dbus_message_iter_next(&iter_sta);
				dbus_message_iter_get_basic(&iter_sta,&StaNode->wtpTerminalRecvPackMount);
				dbus_message_iter_next(&iter_sta);
				dbus_message_iter_get_basic(&iter_sta,&StaNode->wtpTerminalRecvDataPackMount);	//xiaodawei add data pkts recv from terminal
				dbus_message_iter_next(&iter_sta);
				dbus_message_iter_get_basic(&iter_sta,&StaNode->wtpTerminalRecvByteMount);
				dbus_message_iter_next(&iter_sta);
				dbus_message_iter_get_basic(&iter_sta,&StaNode->wtpTerminalResPack);
				dbus_message_iter_next(&iter_sta);
				dbus_message_iter_get_basic(&iter_sta,&StaNode->wtpTerminalResByte);
				dbus_message_iter_next(&iter_sta);
				dbus_message_iter_get_basic(&iter_sta,&StaNode->wtpTerminalRecvWrongPack);
				dbus_message_iter_next(&iter_sta);
				dbus_message_iter_get_basic(&iter_sta,&StaNode->wtpMacTermStaTxFragmentedPkts);	//xiaodawei add for fragmentedPkts, 20110104
				dbus_message_iter_next(&iter_sta);
				dbus_message_iter_get_basic(&iter_sta,&StaNode->wtpMacTermAPTxFragmentedPkts);	//xiaodawei add for fragmentedPkts, 20110104
				dbus_message_iter_next(&iter_sta);
				dbus_message_iter_get_basic(&iter_sta,&StaNode->wtpMacTermAddrUsrOnlineTime);
				dbus_message_iter_next(&iter_sta);
				dbus_message_iter_get_basic(&iter_sta,&StaNode->wtpMacTermAddrUsrSendSpd);
				dbus_message_iter_next(&iter_sta);
				dbus_message_iter_get_basic(&iter_sta,&StaNode->wtpMacTermAddrUsrRecvSpd);
				dbus_message_iter_next(&iter_sta);
				dbus_message_iter_get_basic(&iter_sta,&StaNode->wtpMacTermAddrUsrAllThroughput);
				dbus_message_iter_next(&iter_sta);
				dbus_message_iter_get_basic(&iter_sta,&snrz);
				dbus_message_iter_next(&iter_sta);
				dbus_message_iter_get_basic(&iter_sta,&StaNode->wtpBelongAPID);
				dbus_message_iter_next(&iter_sta);
				dbus_message_iter_get_basic(&iter_sta,&stactime);
				dbus_message_iter_next(&iter_sta);
				dbus_message_iter_get_basic(&iter_sta,&StaNode->wtpterminalaccesstime_int);
				//qiuchen add it 
				dbus_message_iter_next(&iter_sta);
				dbus_message_iter_get_basic(&iter_sta,&StaNode->sta_online_time);
				//end
				dbus_message_iter_next(&iter_sta);
				dbus_message_iter_get_basic(&iter_sta,&wtpName);
				dbus_message_iter_next(&iter_sta);
				dbus_message_iter_get_basic(&iter_sta,&StaNode->security_type);
				
				dbus_message_iter_next(&iter_sta);
				dbus_message_iter_get_basic(&iter_sta,&identity);
				dbus_message_iter_next(&iter_sta);
				dbus_message_iter_get_basic(&iter_sta,&StaNode->sta_access_time);
				dbus_message_iter_next(&iter_sta);
				dbus_message_iter_get_basic(&iter_sta,&StaNode->sta_online_time_new);
				dbus_message_iter_next(&iter_sta);
				dbus_message_iter_get_basic(&iter_sta, &StaNode->MAXofRateset);

				int k = 0;
				for (k = 0; k < WTP_SUPPORT_RATE_NUM; k ++)
				{
					dbus_message_iter_next(&iter_sta);
					dbus_message_iter_get_basic(&iter_sta, &(StaNode->wtp_sta_statistics_info.APStaRxDataRatePkts[k]));
					dbus_message_iter_next(&iter_sta);
					dbus_message_iter_get_basic(&iter_sta, &(StaNode->wtp_sta_statistics_info.APStaTxDataRatePkts[k]));
				}
				for (k = 0; k < WTP_RSSI_INTERVAL_NUM; k ++)
				{
					dbus_message_iter_next(&iter_sta);
					dbus_message_iter_get_basic(&iter_sta, &(StaNode->wtp_sta_statistics_info.APStaTxSignalStrengthPkts[k]));
				}
				
				if((StaNode->wtpMacAddr=(char*)malloc(strlen(wtp_mac)+1))==NULL
					||(StaNode->wtpMacTermAPReceivedStaSignalStrength=(char*)malloc(strlen(snrz)+1))==NULL
					||(StaNode->wtpTerminalMacAddr=(char*)malloc(strlen(sta_mac)+1))==NULL
					||(StaNode->wtpterminalaccesstime=(char*)malloc(strlen(stactime)+1))==NULL
					||(StaNode->wtpName = (char *)malloc(strlen(wtpName)+1)) == NULL
					|| (NULL == (StaNode->identity = (char *)malloc(strlen(identity)+1)))){
					*ret=MALLOC_ERROR;
					dcli_free_wtp_sta_info_head(StaHead);
					dbus_message_unref(reply);
					return NULL;
					}
				memset(StaNode->wtpMacAddr,0,(strlen(wtp_mac)+1));
				memset(StaNode->wtpMacTermAPReceivedStaSignalStrength,0,(strlen(snrz)+1));
				memset(StaNode->wtpTerminalMacAddr,0,(strlen(sta_mac)+1));
				memset(StaNode->wtpterminalaccesstime,0,(strlen(stactime)+1));
				memset(StaNode->wtpName,0,(strlen(wtpName)+1));
				memset(StaNode->identity,0,(strlen(identity)+1));
				memcpy(StaNode->wtpMacAddr,wtp_mac,strlen(wtp_mac));
				memcpy(StaNode->wtpMacTermAPReceivedStaSignalStrength,snrz,strlen(snrz));
				memcpy(StaNode->wtpTerminalMacAddr,sta_mac,strlen(sta_mac));
				memcpy(StaNode->wtpterminalaccesstime,stactime,strlen(stactime));
				memcpy(StaNode->wtpName,wtpName,strlen(wtpName));
				memcpy(StaNode->identity, identity, strlen(identity));
				 
				dbus_message_iter_next(&iter_sta_array);
				}
			dbus_message_iter_next(&iter_wtp_array);
			}
		}

	dbus_message_unref(reply);
	return StaHead;	
}

/*liuzhenhua append 2010-05-26*/
/*mib table 24*/
struct WtpWlanDataPktsInfo* show_WlanDataPkts_Info_of_all_wtp(int index,int localid,DBusConnection *dcli_dbus_connection, unsigned int *wtp_num, unsigned int *ret)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter iter;
	DBusMessageIter iter_wtp_array;
	DBusMessageIter iter_wtp;
	DBusMessageIter iter_wlan_array;
	DBusMessageIter	iter_wlan;

	int i=0,j=0;

	struct WtpWlanDataPktsInfo *WtpNode = NULL;
	struct WtpWlanDataPktsInfo *WtpHead = NULL;
	struct WtpWlanDataPktsInfo *WtpTail = NULL;

	struct WlanDataPktsInfo *WtpWlanNode =NULL;
	struct WlanDataPktsInfo *WtpWlanTail =NULL;
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,\
						INTERFACE,WID_DBUS_CONF_METHOD_SHOW_ALL_WTP_WLAN_DATA_PKTS_INFORMATION);
	dbus_error_init(&err);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);	
	dbus_message_unref(query);

	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		*ret=WID_DBUS_ERROR;
		return NULL;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,wtp_num);		
	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_recurse(&iter,&iter_wtp_array);


	char * wtp_mac=NULL;
	unsigned long long tx_bytes;
	unsigned long long rx_bytes;
	unsigned long long tx_sum_bytes;	//xiaodawei append, 20101116
	unsigned long long rx_sum_bytes;	//xiaodawei append, 20101116
	unsigned int tx_pkt_data;			//xiaodawei append, 20101116
	unsigned int rx_pkt_data;			//xiaodawei append, 20101116
	unsigned int rx_pkts;
	unsigned int tx_pkts;
	unsigned int rx_errors;
	unsigned int tx_errors;
	unsigned int tx_pkt_retry = 0;
	unsigned int rx_rate;
	unsigned int tx_rate;
	unsigned int tx_broadcast_pkt;
	unsigned int tx_drop = 0;
	unsigned int rx_drop = 0;
	
	if((*ret == 0)&&(*wtp_num !=0)){
		for(i=0;i<*wtp_num;i++){
			if((WtpNode = (struct WtpWlanDataPktsInfo*)malloc(sizeof(struct WtpWlanDataPktsInfo))) == NULL){
						dcli_free_wtp_wlan_data_pktsinfo_head(WtpHead);
						*ret = MALLOC_ERROR;
						dbus_message_unref(reply);
						return NULL;
				}
			memset(WtpNode,0,sizeof(struct WtpWlanDataPktsInfo));
			if(WtpHead == NULL){
				WtpHead = WtpNode;
				WtpTail = WtpNode;
				}
			else{
				WtpTail->next=WtpNode;
				WtpTail=WtpNode;
				}
			dbus_message_iter_recurse(&iter_wtp_array,&iter_wtp);
			
			dbus_message_iter_get_basic(&iter_wtp,&WtpNode->wtpCurrID);
			dbus_message_iter_next(&iter_wtp);
			dbus_message_iter_get_basic(&iter_wtp,&wtp_mac);
			dbus_message_iter_next(&iter_wtp);
			dbus_message_iter_get_basic(&iter_wtp,&WtpNode->wlan_num);
			dbus_message_iter_next(&iter_wtp);

			if((WtpNode->wtpMacAddr=malloc(strlen(wtp_mac)+1))==NULL){
				dcli_free_wtp_wlan_data_pktsinfo_head(WtpHead);
				*ret = MALLOC_ERROR;
				return NULL;
				}
			memset(WtpNode->wtpMacAddr,0,strlen(wtp_mac)+1);
			memcpy(WtpNode->wtpMacAddr,wtp_mac,strlen(wtp_mac));
			
			dbus_message_iter_recurse(&iter_wtp,&iter_wlan_array);
			for(j=0;j<WtpNode->wlan_num;j++){
				if((WtpWlanNode=(struct WlanDataPktsInfo*)malloc(sizeof(struct WlanDataPktsInfo))) == NULL){
					dcli_free_wtp_wlan_data_pktsinfo_head(WtpHead);
					*ret = MALLOC_ERROR;
					return NULL;
					}
				memset(WtpWlanNode,0,sizeof(struct WlanDataPktsInfo));
				if(WtpNode->wlan_list==NULL){
					WtpNode->wlan_list=WtpWlanNode;
					WtpWlanTail=WtpWlanNode;
					}
				else{
					WtpWlanTail->next=WtpWlanNode;
					WtpWlanTail=WtpWlanNode;
					}
				dbus_message_iter_recurse(&iter_wlan_array,&iter_wlan);
				
				dbus_message_iter_get_basic(&iter_wlan,&WtpWlanNode->wlanCurrID);
				dbus_message_iter_next(&iter_wlan);
				dbus_message_iter_get_basic(&iter_wlan,&tx_bytes);
				dbus_message_iter_next(&iter_wlan);
				dbus_message_iter_get_basic(&iter_wlan,&rx_bytes);
				dbus_message_iter_next(&iter_wlan);							/*xiaodawei add 20101116*/
				dbus_message_iter_get_basic(&iter_wlan,&tx_sum_bytes);
				dbus_message_iter_next(&iter_wlan);
				dbus_message_iter_get_basic(&iter_wlan,&rx_sum_bytes);
				dbus_message_iter_next(&iter_wlan);
				dbus_message_iter_get_basic(&iter_wlan,&tx_pkt_data);
				dbus_message_iter_next(&iter_wlan);
				dbus_message_iter_get_basic(&iter_wlan,&rx_pkt_data);		/*######END######*/
				dbus_message_iter_next(&iter_wlan);
				dbus_message_iter_get_basic(&iter_wlan,&rx_pkts);
				dbus_message_iter_next(&iter_wlan);
				dbus_message_iter_get_basic(&iter_wlan,&tx_pkts);
				dbus_message_iter_next(&iter_wlan);
				dbus_message_iter_get_basic(&iter_wlan,&rx_errors);
				dbus_message_iter_next(&iter_wlan);
				dbus_message_iter_get_basic(&iter_wlan,&tx_errors);
				dbus_message_iter_next(&iter_wlan);
				dbus_message_iter_get_basic(&iter_wlan,&tx_pkt_retry);
				dbus_message_iter_next(&iter_wlan);
				dbus_message_iter_get_basic(&iter_wlan,&rx_rate);
				dbus_message_iter_next(&iter_wlan);
				dbus_message_iter_get_basic(&iter_wlan,&tx_rate);
				dbus_message_iter_next(&iter_wlan);
				dbus_message_iter_get_basic(&iter_wlan,&tx_broadcast_pkt);/*add 20100825 by nl*/
				dbus_message_iter_next(&iter_wlan);
				dbus_message_iter_get_basic(&iter_wlan,&tx_drop);
				dbus_message_iter_next(&iter_wlan);
				dbus_message_iter_get_basic(&iter_wlan,&rx_drop);

				
				WtpWlanNode->wtpSsidSendTermAllByte		= tx_sum_bytes; //book modify, 2011-1-20
				WtpWlanNode->wtpSsidRecvTermAllPack		= rx_pkts; //book modify, 2011-1-20
				WtpWlanNode->wtpSsidRecvTermAllByte		= rx_sum_bytes;//book modify
				WtpWlanNode->wtpSsidWirelessMacRecvDataRightByte = rx_bytes;/*xiaodawei modify,20101116, 接收总字节数*/
				WtpWlanNode->wtpSsidWirelessMacSendDataRightByte = tx_bytes;/*xiaodawei modify,20101116, 发送总字节数*/
				WtpWlanNode->wtpSsidWiredMacRecvDataWrongPack = rx_errors;
				WtpWlanNode->wtpNetWiredRecvPack		= tx_pkts + tx_errors;
				WtpWlanNode->wtpUsrWirelessMacRecvDataPack = rx_pkt_data;		/*xiaodawei modify, 20101116*/
				WtpWlanNode->wtpUsrWirelessMacSendDataPack = tx_pkt_data;		/*xiaodawei add, 20101116*/
				WtpWlanNode->wtpNetWiredSendPack		= rx_pkts + rx_errors;
				WtpWlanNode->WtpWirelessSendFailPkts	= tx_errors;
				WtpWlanNode->wtpWirelessResendPkts		= tx_pkt_retry;            //xiaodawei modify, 20110425
				WtpWlanNode->wtpWirelessSendBroadcastMsgNum = tx_broadcast_pkt;
				WtpWlanNode->wtpStaUplinkMaxRate		= tx_rate;
				WtpWlanNode->wtpStaDwlinkMaxRate		= rx_rate;
				WtpWlanNode->wtpNetWiredRecvErrPack		= tx_errors;
				WtpWlanNode->wtpNetWiredRecvRightPack	= tx_pkts;//book modify, 2011-1-20
				WtpWlanNode->wtpNetWiredRecvByte		= tx_bytes;
				WtpWlanNode->wtpNetWiredSendByte		= rx_bytes;
				WtpWlanNode->wtpNetWiredSendErrPack		= rx_errors;
				WtpWlanNode->wtpNetWiredSendRightPack	= rx_pkts;//book modify, 2011-1-20
				WtpWlanNode->wtpSsidSendDataAllPack		= tx_pkt_data;
				WtpWlanNode->wtpSsidTxDataDropPkts		= tx_drop;
				WtpWlanNode->wtpSsidRxDataDropPkts		= rx_drop;

				if((WtpWlanNode->wtpWirelessWrongPktsRate=(char*)malloc(10))==NULL
					|| (WtpWlanNode->wtpNetWiredRxWrongPktsRate=(char*)malloc(10))==NULL
					|| (WtpWlanNode->wtpNetWiredTxWrongPktsRate=(char*)malloc(10))==NULL){
					dcli_free_wtp_wlan_data_pktsinfo_head(WtpHead);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
					}
				memset(WtpWlanNode->wtpWirelessWrongPktsRate,0,10);
				memset(WtpWlanNode->wtpNetWiredRxWrongPktsRate,0,10);
				memset(WtpWlanNode->wtpNetWiredTxWrongPktsRate,0,10);
				
				if(tx_pkts+rx_pkts>0){
					snprintf(WtpWlanNode->wtpWirelessWrongPktsRate,10,"%d",(rx_errors+tx_errors)/(rx_pkts+tx_pkts+rx_errors+tx_errors)*100);//book modify
					}
				else{
					snprintf(WtpWlanNode->wtpWirelessWrongPktsRate,"%d",0);
					}

				
				if(tx_pkts>0){
					snprintf(WtpWlanNode->wtpNetWiredRxWrongPktsRate,10,"%.2f\%",(float)tx_errors/(float)(tx_pkts+tx_errors)*100);//book modify
					}
				else{
					snprintf(WtpWlanNode->wtpNetWiredRxWrongPktsRate,10,"%.2f\%",0.00);
					}
				
				if(rx_pkts>0){
					snprintf(WtpWlanNode->wtpNetWiredTxWrongPktsRate,10,"%.2f\%",(float)rx_errors/(float)(rx_pkts+rx_errors)*100);//book modify
					}
				else{
					snprintf(WtpWlanNode->wtpNetWiredTxWrongPktsRate,10,"%.2f\%",0.00);
					}				
				
				dbus_message_iter_next(&iter_wlan_array);
				}
			dbus_message_iter_next(&iter_wtp_array);
			}
		}
	
	dbus_message_unref(reply);

	return WtpHead;
}

/*for mib table WtpTerminalInfo liuzhenhua add 2010-05-13
table 23 */
struct WtpTerminalInfo* show_terminal_info_of_all_wtp(int index,int localid,DBusConnection *dcli_dbus_connection, unsigned int *wtp_num, unsigned int *ret)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter  iter;
	DBusMessageIter  iter_wtp;
	DBusMessageIter  iter_bss;
	DBusMessageIter  iter_sta;
	DBusMessageIter	 iter_array;
	DBusMessageIter	 iter_sub_array;
	DBusMessageIter	 iter_struct;
	int i=0,j=0,k=0;

	unsigned int bss_num=0;
	unsigned int sta_num=0;
	
	struct WtpTerminalInfo *WtpNode = NULL;
	struct WtpTerminalInfo *WtpHead = NULL;
	struct WtpTerminalInfo *WtpTail = NULL;

	struct Wtp_TerminalInfo *WtpTermNode =NULL;
	struct Wtp_TerminalInfo *WtpTermTail =NULL;
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,\
			INTERFACE,ASD_DBUS_STA_METHOD_SHOW_TERMINAL_INFO_OF_ALL_WTP);
	dbus_error_init(&err);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);			
	dbus_message_unref(query);

	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		*ret=ASD_DBUS_ERROR;
		return NULL;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,wtp_num);		

	dbus_message_iter_next(&iter);	
	dbus_message_iter_recurse(&iter,&iter_array);
	unsigned int wtp_id;
	unsigned int security_id;
	unsigned int wlan_id;
	unsigned int radio_id;
	unsigned int total_sta;
	char *wtp_mac = NULL;
	char *sta_mac = NULL;
	char *wlanname = NULL;
	
	if((*ret == 0)&&(*wtp_num !=0)){
		for(i=0;i<*wtp_num;i++){
			if((WtpNode = (struct WtpTerminalInfo*)malloc(sizeof(struct WtpTerminalInfo))) == NULL){
						dcli_free_wtp_terminalinfo_head(WtpHead);
						*ret = MALLOC_ERROR;
						dbus_message_unref(reply);
						return NULL;
				}
			memset(WtpNode,0,sizeof(struct WtpTerminalInfo));
			if(WtpHead == NULL){
				WtpHead = WtpNode;
				WtpTail = WtpNode;
			}else{
				WtpTail->next=WtpNode;
				WtpTail=WtpNode;
			}
			total_sta=0;
			dbus_message_iter_recurse(&iter_array,&iter_wtp);
			dbus_message_iter_get_basic(&iter_wtp,&(wtp_id));
			dbus_message_iter_next(&iter_wtp);
			dbus_message_iter_get_basic(&iter_wtp,&(wtp_mac));
			dbus_message_iter_next(&iter_wtp);
			dbus_message_iter_get_basic(&iter_wtp,&(bss_num));
			dbus_message_iter_next(&iter_wtp);
			dbus_message_iter_recurse(&iter_wtp,&iter_sub_array);
			
			for(j=0;j<bss_num;j++){
				dbus_message_iter_recurse(&iter_sub_array,&iter_bss);
				dbus_message_iter_get_basic(&iter_bss,&security_id);
				dbus_message_iter_next(&iter_bss);
				dbus_message_iter_get_basic(&iter_bss,&wlan_id);
				dbus_message_iter_next(&iter_bss);
				dbus_message_iter_get_basic(&iter_bss,&radio_id);
				dbus_message_iter_next(&iter_bss);
				dbus_message_iter_get_basic(&iter_bss,&sta_num);
				dbus_message_iter_next(&iter_bss);
				dbus_message_iter_recurse(&iter_bss,&iter_struct);
				total_sta += sta_num;
				for(k=0;k<sta_num;k++){
					if((WtpTermNode = (struct Wtp_TerminalInfo*)malloc(sizeof(struct Wtp_TerminalInfo))) == NULL){
						dcli_free_wtp_terminalinfo_head(WtpHead);
						*ret = MALLOC_ERROR;
						dbus_message_unref(reply);
						return NULL;
					}
					memset(WtpTermNode,0,sizeof(struct Wtp_TerminalInfo));
					if(WtpNode->terminalInfo_list == NULL){
						WtpNode->terminalInfo_list = WtpTermNode;
						WtpTermTail = WtpTermNode;
					}else{
						WtpTermTail->next = WtpTermNode;
						WtpTermTail = WtpTermNode;
					}
					
					WtpTermNode->staqSecID = security_id;
					WtpTermNode->staqWlanID = wlan_id;
					WtpTermNode->staqRadioID = radio_id;
					
					dbus_message_iter_recurse(&iter_struct,&iter_sta);
					dbus_message_iter_get_basic(&iter_sta,&sta_mac);
					dbus_message_iter_next(&iter_sta);
					dbus_message_iter_get_basic(&iter_sta,&WtpTermNode->wtpEndWMMSta);
					dbus_message_iter_next(&iter_sta);
					dbus_message_iter_get_basic(&iter_sta,&WtpTermNode->wtpStaIPAddress);
					
					dbus_message_iter_next(&iter_sta);
					dbus_message_iter_get_basic(&iter_sta,&WtpTermNode->wtpStaRadioMode);
					
					dbus_message_iter_next(&iter_sta);
					dbus_message_iter_get_basic(&iter_sta,&WtpTermNode->wtpStaRadioChannel);
					dbus_message_iter_next(&iter_sta);
					dbus_message_iter_get_basic(&iter_sta,&WtpTermNode->wtpAPTxRates);
					dbus_message_iter_next(&iter_sta);
					dbus_message_iter_get_basic(&iter_sta,&WtpTermNode->wtpAPRxRates);//xiaodawei add for sta tx rates(ap rx rates), 20101228
					dbus_message_iter_next(&iter_sta);
					dbus_message_iter_get_basic(&iter_sta,&WtpTermNode->wtpStaPowerSaveMode);
					dbus_message_iter_next(&iter_sta);
					dbus_message_iter_get_basic(&iter_sta,&WtpTermNode->wtpStaVlanId);
					dbus_message_iter_next(&iter_sta);
					dbus_message_iter_get_basic(&iter_sta,&wlanname);
					dbus_message_iter_next(&iter_sta);
					dbus_message_iter_get_basic(&iter_sta,&WtpTermNode->wtpStaAuthenMode);
					dbus_message_iter_next(&iter_sta);
					dbus_message_iter_get_basic(&iter_sta,&WtpTermNode->wtpStaSecurityCiphers);
					dbus_message_iter_next(&iter_sta);
					dbus_message_iter_get_basic(&iter_sta,&WtpTermNode->wtpAutenModel);
					dbus_message_iter_next(&iter_sta);
					dbus_message_iter_get_basic(&iter_sta,&WtpTermNode->wtpEndStaID);

					WtpTermNode->wtpTerminalMacAddr= (unsigned char *)malloc(strlen(sta_mac) +1);
					memset(WtpTermNode->wtpTerminalMacAddr,0,(strlen(sta_mac) +1));
					memcpy(WtpTermNode->wtpTerminalMacAddr,sta_mac,strlen(sta_mac));
					
					WtpTermNode->wtpStaSSIDName=(char*)malloc(strlen(wlanname)+1);
					memset(WtpTermNode->wtpStaSSIDName,0,strlen(wlanname)+1);
					memcpy(WtpTermNode->wtpStaSSIDName,wlanname,strlen(wlanname));
					dbus_message_iter_next(&iter_sta);
					dbus_message_iter_get_basic(&iter_sta,&WtpTermNode->encryption_type);

					dbus_message_iter_next(&iter_struct);
					}
				dbus_message_iter_next(&iter_sub_array);
				}	
			
			WtpNode->wtpCurrID = wtp_id;
			WtpNode->wtpMacAddr = (unsigned char *)malloc(strlen(wtp_mac) +1);
			memset(WtpNode->wtpMacAddr,0,(strlen(wtp_mac) +1));
			memcpy(WtpNode->wtpMacAddr,wtp_mac,strlen(wtp_mac));
			WtpNode->sta_num=total_sta;
			
			dbus_message_iter_next(&iter_array);
		}
	}

	dbus_message_unref(reply);

#if 0
	//ASD operation 
	DBusMessageIter iter_radio_array;
	DBusMessageIter iter_radio;
	
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_STA_METHOD_SHOW_TERMINAL_INFO_OF_ALL_WTP);
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		dcli_free_wtp_terminalinfo_head(WtpHead);
		*ret=WID_DBUS_ERROR;
		return NULL;
	}

	unsigned int radio_num=0;
	unsigned int radio_type=0;
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&radio_num);
	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_recurse(&iter,&iter_radio_array);
	for(i=0;i<radio_num;i++){
		dbus_message_iter_recurse(&iter_radio_array,&iter_radio);
		dbus_message_iter_get_basic(&iter_radio,&radio_id);
		dbus_message_iter_next(&iter_radio);
		dbus_message_iter_get_basic(&iter_radio,&radio_type);

		WtpNode = WtpHead;
		while(WtpNode){
			WtpTermNode = WtpNode->terminalInfo_list;
			while(WtpTermNode){
				
				if(WtpTermNode->staqRadioID == radio_id)
					WtpTermNode->wtpStaRadioMode = radio_type;
				
				WtpTermNode = WtpTermNode->next;
			}
			WtpNode = WtpNode->next;
		}		
		dbus_message_iter_next(&iter_radio_array);
	}

	dbus_message_unref(reply);
#endif
	return WtpHead;

}

struct WtpBasicInfo* show_basic_info_of_all_wtp(int index,int localid,DBusConnection *dcli_dbus_connection, unsigned int *num, unsigned int *ret)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;	
	
	struct WtpBasicInfo *WtpNode = NULL;
	struct WtpBasicInfo *WtpHead = NULL;
	int i =0;
	time_t addtime,now;
	unsigned int ElectrifyRegisterCircle = 0;
	char *name = "CHINA";
	time(&now);
	char * wtpSysSoftName = NULL;
	char *wtpVersionInfo= NULL;
	char *wtpSysVersion = NULL;
	char *wtpPosInfo = NULL;
	char *wtpProduct = NULL;								
	char *wtpSeriesNum = NULL;
	char *wtpDevName = NULL;
	char * wtpModel = NULL;
	char *wtpSysSoftProductor = NULL;
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SHOW_ALL_WTP_BASIC_INFORMATION);

	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		*ret = WID_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,num);

	if((*ret == 0) && (num != 0))
	{
			if((WtpHead = (struct WtpBasicInfo*)malloc(sizeof(struct WtpBasicInfo))) == NULL)
			{
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			memset(WtpHead,0,sizeof(struct WtpBasicInfo));
			WtpHead->WtpBasicInfo_list = NULL;
			WtpHead->WtpBasicInfo_last = NULL;

			dbus_message_iter_next(&iter);	
			dbus_message_iter_recurse(&iter,&iter_array);
			
			for (i = 0; i < *num; i++) 
			{
				DBusMessageIter iter_struct;
				if((WtpNode = (struct WtpBasicInfo*)malloc(sizeof(struct WtpBasicInfo))) == NULL){
						dcli_free_WtpBasicInfo(WtpHead);
						*ret = MALLOC_ERROR;
						dbus_message_unref(reply);
						return NULL;
				}
				
				memset(WtpNode,0,sizeof(struct WtpBasicInfo));
				WtpNode->next = NULL;

				WtpNode->wtpMacAddr = (unsigned char *)malloc(MAC_LEN +1);
				memset(WtpNode->wtpMacAddr,0,(MAC_LEN +1));
	
				if(WtpHead->WtpBasicInfo_list == NULL){
					WtpHead->WtpBasicInfo_list = WtpNode;
					WtpHead->next = WtpNode;
				}else{
					WtpHead->WtpBasicInfo_last->next = WtpNode;
					
				}
				WtpHead->WtpBasicInfo_last = WtpNode;
				
				dbus_message_iter_recurse(&iter_array,&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpCurrID));
			
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&(wtpSeriesNum));
			
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&(wtpDevName));
			
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&(wtpModel));
				
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&(wtpProduct));

				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&(wtpSysSoftProductor));

				dbus_message_iter_next(&iter_struct);	
				dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpOnlineTime));	

				dbus_message_iter_next(&iter_struct);	
				dbus_message_iter_get_basic(&iter_struct,&(WtpNode->ElectrifyRegisterCircle));

				dbus_message_iter_next(&iter_struct);	
				dbus_message_iter_get_basic(&iter_struct,&(WtpNode->acNeighbordeadTimes));

				dbus_message_iter_next(&iter_struct);	
				dbus_message_iter_get_basic(&iter_struct,&(wtpPosInfo));

				dbus_message_iter_next(&iter_struct);	
				dbus_message_iter_get_basic(&iter_struct,&(wtpSysVersion));

				dbus_message_iter_next(&iter_struct);	
				dbus_message_iter_get_basic(&iter_struct,&(wtpVersionInfo));

				dbus_message_iter_next(&iter_struct);	
				dbus_message_iter_get_basic(&iter_struct,&(wtpSysSoftName));

				dbus_message_iter_next(&iter_struct);	
				dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[0]));
				
				dbus_message_iter_next(&iter_struct);	
				dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[1]));
				
				dbus_message_iter_next(&iter_struct);	
				dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[2]));
				
				dbus_message_iter_next(&iter_struct);	
				dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[3]));
				
				dbus_message_iter_next(&iter_struct);	
				dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[4]));
				
				dbus_message_iter_next(&iter_struct);	
				dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[5]));

				if(WtpNode->wtpOnlineTime == 0)
					WtpNode->wtpUpTime =0;
				else 
					WtpNode->wtpUpTime = now - WtpNode->wtpOnlineTime;

				if(WtpNode->ElectrifyRegisterCircle == 0){
					WtpNode->Mib_wtpUpTime = 0 ;
				}
				else{
					WtpNode->Mib_wtpUpTime = WtpNode->wtpUpTime + WtpNode->ElectrifyRegisterCircle ; 
				}
				
				WtpNode->wtpSysRestart = 1;
				WtpNode->wtpSysReset = 1;
				WtpNode->wtpColdReboot = 1;
				
				/*=========================================================*/

				WtpNode->wtpSeriesNum = (char*)malloc(strlen(wtpSeriesNum)+1);
				memset(WtpNode->wtpSeriesNum, 0, strlen(wtpSeriesNum)+1);
				memcpy(WtpNode->wtpSeriesNum, wtpSeriesNum, strlen(wtpSeriesNum));	
				
				WtpNode->wtpDevName = (char*)malloc(strlen(wtpDevName)+1);
				memset(WtpNode->wtpDevName, 0, strlen(wtpDevName)+1);
				memcpy(WtpNode->wtpDevName, wtpDevName, strlen(wtpDevName));	
			
				WtpNode->wtpModel = (char*)malloc(strlen(wtpModel)+1);
				memset(WtpNode->wtpModel, 0, strlen(wtpModel)+1);
				memcpy(WtpNode->wtpModel, wtpModel, strlen(wtpModel));	
				
				WtpNode->wtpProduct = (char*)malloc(strlen(wtpProduct)+1);
				memset(WtpNode->wtpProduct, 0, strlen(wtpProduct)+1);
				memcpy(WtpNode->wtpProduct, wtpProduct, strlen(wtpProduct));
				
				WtpNode->wtpSysSoftProductor = (char*)malloc(strlen(wtpSysSoftProductor)+1);
				memset(WtpNode->wtpSysSoftProductor, 0, strlen(wtpSysSoftProductor)+1);
				memcpy(WtpNode->wtpSysSoftProductor, wtpSysSoftProductor, strlen(wtpSysSoftProductor));
				
				WtpNode->wtpPosInfo = (char*)malloc(strlen(wtpPosInfo)+1);
				memset(WtpNode->wtpPosInfo, 0, strlen(wtpPosInfo)+1);
				memcpy(WtpNode->wtpPosInfo, wtpPosInfo, strlen(wtpPosInfo));		

				WtpNode->wtpSysVersion = (char*)malloc(strlen(wtpSysVersion)+1);
				memset(WtpNode->wtpSysVersion, 0, strlen(wtpSysVersion)+1);
				memcpy(WtpNode->wtpSysVersion, wtpSysVersion, strlen(wtpSysVersion));
				
				WtpNode->wtpVersionInfo = (char*)malloc(strlen(wtpVersionInfo)+1);
				memset(WtpNode->wtpVersionInfo, 0, strlen(wtpVersionInfo)+1);
				memcpy(WtpNode->wtpVersionInfo, wtpVersionInfo, strlen(wtpVersionInfo));

				WtpNode->wtpSysSoftName = (char*)malloc(strlen(wtpSysSoftName)+1);
				memset(WtpNode->wtpSysSoftName, 0, strlen(wtpSysSoftName)+1);
				memcpy(WtpNode->wtpSysSoftName, wtpSysSoftName, strlen(wtpSysSoftName));
				/*=========================================================*/
				
				WtpNode->wtpDomain = (char*)malloc(strlen(name)+1);
				memset(WtpNode->wtpDomain, 0, strlen(name)+1);
				memcpy(WtpNode->wtpDomain, name, strlen(name));		

				WtpNode->wtpDevTypeNum = (char*)malloc(strlen(WtpNode->wtpModel)+1);
				memset(WtpNode->wtpDevTypeNum, 0, strlen(WtpNode->wtpModel)+1);
				memcpy(WtpNode->wtpDevTypeNum, WtpNode->wtpModel, strlen(WtpNode->wtpModel));		

				dbus_message_iter_next(&iter_array);
	
			}
	
		}
		
		dbus_message_unref(reply);

		return WtpHead;

}
/*table 2*/
struct WtpParaInfo* show_para_info_of_all_wtp(int index,int localid,DBusConnection *dcli_dbus_connection, unsigned int *num, unsigned int *ret)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;	
	DBusMessageIter iter_struct;
	DBusMessageIter  iter_sub_array;	
	DBusMessageIter	 iter_sub_struct;
	DBusMessageIter iter_sub_sub_array;
	DBusMessageIter iter_sub_sub_struct;
	
	struct WtpParaInfo *WtpNode = NULL;
	struct WtpParaInfo *WtpHead = NULL;
	struct WtpParaInfo *WtpSearchNode = NULL;
	
	int i = 0;
	char *netid = NULL; 
	char *wtpModel =NULL;

	unsigned int result = 0;
	char ip[WTP_WTP_IP_LEN+1]={0};
	char *wtp_name = NULL;
	char * wtpip = NULL;
	unsigned char mymaskBuf[16] = {0};	
	unsigned char *mymaskPtr = mymaskBuf;
	unsigned int wtp_mask = 0;
	
	unsigned char *Bssid =NULL;
	unsigned int wtp_gatewayaddr = 0;
	int wtp_retval = 0;
	unsigned char wtp_mygatewayBuf[16] = {0};	
	unsigned char *wtp_mygatewayPtr =wtp_mygatewayBuf;
	unsigned int wtp_bssid_num = 0;
	unsigned char * wtp_all_bssid =(unsigned char *)malloc(300);	
	unsigned char *bssid = NULL;
	bssid = (unsigned char *)malloc(6);

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);


	query = dbus_message_new_method_call(BUSNAME,OBJPATH,\
						INTERFACE,WID_DBUS_CONF_METHOD_SHOW_ALL_WTP_PARA_INFORMATION);
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		*ret = WID_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		if(wtp_all_bssid){
			free(wtp_all_bssid);
			wtp_all_bssid = NULL;
		}
		if(bssid){
			free(bssid);
			bssid = NULL;
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,num);
	
	if((*ret == 0) && (num != 0)){
		if((WtpHead = (struct WtpParaInfo*)malloc(sizeof(struct WtpParaInfo))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				if(wtp_all_bssid){
					free(wtp_all_bssid);
					wtp_all_bssid = NULL;
				}
				if(bssid){
					free(bssid);
					bssid = NULL;
				}
				return NULL;
		}
		memset(WtpHead,0,sizeof(struct WtpParaInfo));
		WtpHead->WtpParaInfo_list = NULL;
		WtpHead->WtpParaInfo_last = NULL;
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
		
		for (i = 0; i < *num; i++) {
			if((WtpNode = (struct WtpParaInfo*)malloc(sizeof(struct WtpParaInfo))) == NULL){
					dcli_free_WtpParaInfo(WtpHead);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
				if(wtp_all_bssid){
					free(wtp_all_bssid);
					wtp_all_bssid = NULL;
				}
				if(bssid){
					free(bssid);
					bssid = NULL;
				}
					return NULL;
			}
			
			memset(WtpNode,0,sizeof(struct WtpParaInfo));
			WtpNode->next = NULL;

			WtpNode->wtpMacAddr = (unsigned char *)malloc(MAC_LEN +1);
			memset(WtpNode->wtpMacAddr,0,(MAC_LEN +1));

			if(WtpHead->WtpParaInfo_list == NULL){
				WtpHead->WtpParaInfo_list = WtpNode;
				WtpHead->next = WtpNode;
			}else{
				WtpHead->WtpParaInfo_last->next = WtpNode;
				
			}
			WtpHead->WtpParaInfo_last = WtpNode;
			
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpCurrID));
		
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[0]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[1]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[2]));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[3]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[4]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[5]));
			
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(wtpModel));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&netid);

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&wtp_mask);

			WtpNode->wtpNetElementCode = (char*)malloc(strlen(netid)+1);
			memset(WtpNode->wtpNetElementCode, 0, strlen(netid)+1);
			memcpy(WtpNode->wtpNetElementCode, netid, strlen(netid));

			WtpNode->wtpModel = (char*)malloc(strlen(wtpModel)+1);
			memset(WtpNode->wtpModel, 0, strlen(wtpModel)+1);
			memcpy(WtpNode->wtpModel, wtpModel, strlen(wtpModel));	

			WtpNode->wtpAddrMask  = (char*)malloc(16+1);
			memset(WtpNode->wtpAddrMask, 0, 16+1);

			ip_long2str(wtp_mask,&mymaskPtr);
			memcpy(WtpNode->wtpAddrMask, mymaskPtr, strlen(mymaskPtr));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&wtp_gatewayaddr);

			WtpNode->wtpGateAddr  = (char*)malloc(16+1);
			memset(WtpNode->wtpGateAddr, 0, 16+1);
			wtp_retval = ip_long2str(wtp_gatewayaddr,&wtp_mygatewayPtr);
			strcpy(WtpNode->wtpGateAddr,wtp_mygatewayPtr);

			WtpNode->wtpWorkMode = 2;
			WtpNode->wtpBridgingWorkMode = 2;
			WtpNode->wtpsupport11b = 1;
			WtpNode->wtpsupport11g = 1;
			
			WtpNode->wtpIfType = (char *)malloc(70);
			memset(WtpNode->wtpIfType,0,70);
			char iftype1[] = "Ethernet";
			char iftype2[] = "Wifi";
			sprintf(WtpNode->wtpIfType,"%s,%s",iftype1,iftype2);
			
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpCurrAPMode));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&WtpNode->wtpDefenceDOSAttack);
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&WtpNode->wtpIGMPSwitch);
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&WtpNode->wtpReceiverSignalPWL);
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&WtpNode->wtpRemoteRestartFun);
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&WtpNode->wtpState);

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&wtpip);
					
			WtpNode->wtpIP=(char *)malloc(WTP_WTP_IP_LEN+1);
			memset(WtpNode->wtpIP,0,WTP_WTP_IP_LEN+1);
			result = wtp_check_wtp_ip_addr(ip,wtpip);
			if(result != 1)
				memcpy(WtpNode->wtpIP,wtpip,strlen(wtpip));
			else
				memcpy(WtpNode->wtpIP,ip,strlen(ip));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&wtp_name);

			WtpNode->wtpName = (char*)malloc(strlen(wtp_name)+1);
			memset(WtpNode->wtpName, 0, strlen(wtp_name)+1);
			memcpy(WtpNode->wtpName, wtp_name, strlen(wtp_name));
			/*xiaodawei transplant from 2.0 for telecom test, 20110302*///printf("222222222222222222222222222\n");
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&WtpNode->add_time);
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&WtpNode->ElectrifyRegisterCircle);
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&WtpNode->imagadata_time);
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&WtpNode->config_update_time);

			dbus_message_iter_next(&iter_array);
		}
		/*for receiving ssid*/
			unsigned int another_wtp_num;
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&another_wtp_num);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);

		for(i=0;i<*num; i++){
			unsigned int wtp_id = 0;
			unsigned int total_lenth =0;
			int j = 0,k = 0;
			unsigned char wtp_radio_num = 0;
			unsigned int wtp_bssid_num = 0;
			memset(wtp_all_bssid,0,300);
			unsigned char *curser = wtp_all_bssid;
				unsigned char ap_eth_num = 0;
			
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&wtp_radio_num);

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&wtp_id);

			dbus_message_iter_next(&iter_struct);	
				dbus_message_iter_get_basic(&iter_struct,&ap_eth_num);

				dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_recurse(&iter_struct,&iter_sub_array);

			for(j=0;j<wtp_radio_num;j++){
					unsigned char radio_bssid_num;
				dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&radio_bssid_num);

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_recurse(&iter_sub_struct,&iter_sub_sub_array);

				for(k=0;k<radio_bssid_num;k++){
					memset(bssid,0,6);
					dbus_message_iter_recurse(&iter_sub_sub_array,&iter_sub_sub_struct);
					dbus_message_iter_get_basic(&iter_sub_sub_struct,&(bssid[0]));
					dbus_message_iter_next(&iter_sub_sub_struct);
					dbus_message_iter_get_basic(&iter_sub_sub_struct,&(bssid[1]));
					dbus_message_iter_next(&iter_sub_sub_struct);
					dbus_message_iter_get_basic(&iter_sub_sub_struct,&(bssid[2]));
					dbus_message_iter_next(&iter_sub_sub_struct);
					dbus_message_iter_get_basic(&iter_sub_sub_struct,&(bssid[3]));
					
					dbus_message_iter_next(&iter_sub_sub_struct);
					dbus_message_iter_get_basic(&iter_sub_sub_struct,&(bssid[4]));
					dbus_message_iter_next(&iter_sub_sub_struct);
					dbus_message_iter_get_basic(&iter_sub_sub_struct,&(bssid[5]));
					
					memcpy(curser,bssid,6);
					curser = curser + 6;
					total_lenth = total_lenth + 6;
					wtp_bssid_num ++;
					dbus_message_iter_next(&iter_sub_sub_array);
				}
				dbus_message_iter_next(&iter_sub_array);
			}
				unsigned char total_if_num = wtp_radio_num + ap_eth_num;		/*add for total_if_num 2010-08-24*/

			WtpSearchNode = WtpHead->WtpParaInfo_list;
			while(WtpSearchNode!= NULL){
				if(WtpSearchNode->wtpCurrID == wtp_id){
					WtpSearchNode->wtpCurBssid = (unsigned char*)malloc(300);
					memset(WtpSearchNode->wtpCurBssid, 0, total_lenth+1);
					memcpy(WtpSearchNode->wtpCurBssid, wtp_all_bssid, total_lenth);
					WtpSearchNode->wtpCurBssidNumber = wtp_bssid_num;
						WtpSearchNode->total_if_num = total_if_num;
				}
				WtpSearchNode = WtpSearchNode->next;
			}
			dbus_message_iter_next(&iter_array);
		}
	}
	free(wtp_all_bssid);
	wtp_all_bssid = NULL;
	free(bssid);
	bssid = NULL;
	dbus_message_unref(reply);

	return WtpHead;
}
/*table 3*/
struct WtpCollectInfo* show_collect_info_of_all_wtp(int index,int localid,DBusConnection *dcli_dbus_connection, unsigned int *num, unsigned int *ret)
{
	DBusMessage *query, *reply;
	DBusMessage *query2, *reply2;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;	
	
	struct WtpCollectInfo *WtpNode = NULL;
	struct WtpCollectInfo *WtpHead = NULL;
	struct WtpCollectInfo *WtpSearchNode = NULL;
	DBusMessageIter iter_struct;
	
	int i=0;
	int ehcotimer =0 ;
	unsigned int wtp_num = 0;
	unsigned int wtp_id = 0;
	unsigned int assoc_req_num = 0;
	unsigned int reassoc_request_num = 0;
	unsigned int num_assoc_failure = 0;
	unsigned int num_reassoc_failure = 0;
	unsigned int assoc_success = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,\
						INTERFACE,WID_DBUS_CONF_METHOD_SHOW_ALL_WTP_COLLECT_INFORMATION);
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		*ret = WID_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,num);

	if((*ret == 0) && (num != 0)){
			if((WtpHead = (struct WtpCollectInfo*)malloc(sizeof(struct WtpCollectInfo))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			memset(WtpHead,0,sizeof(struct WtpCollectInfo));
			WtpHead->WtpCollectInfo_list = NULL;
			WtpHead->WtpCollectInfo_last = NULL;
	
			dbus_message_iter_next(&iter);	
			dbus_message_iter_recurse(&iter,&iter_array);
			
			for (i = 0; i < *num; i++) {
				if((WtpNode = (struct WtpCollectInfo*)malloc(sizeof(struct WtpCollectInfo))) == NULL){
						dcli_free_WtpCollectInfo(WtpHead);
						*ret = MALLOC_ERROR;
						dbus_message_unref(reply);
						return NULL;
				}
				
				memset(WtpNode,0,sizeof(struct WtpCollectInfo));
				WtpNode->next = NULL;

				WtpNode->wtpMacAddr = (unsigned char *)malloc(MAC_LEN +1);
				memset(WtpNode->wtpMacAddr,0,(MAC_LEN +1));
	
				if(WtpHead->WtpCollectInfo_list == NULL){
					WtpHead->WtpCollectInfo_list = WtpNode;
					WtpHead->next = WtpNode;
				}else{
					WtpHead->WtpCollectInfo_last->next = WtpNode;
					
				}
				WtpHead->WtpCollectInfo_last = WtpNode;
				
				dbus_message_iter_recurse(&iter_array,&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpCurrID));
			
				dbus_message_iter_next(&iter_struct);	
				dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[0]));
				dbus_message_iter_next(&iter_struct);	
				dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[1]));
				dbus_message_iter_next(&iter_struct);	
				dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[2]));
				
				dbus_message_iter_next(&iter_struct);	
				dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[3]));
				dbus_message_iter_next(&iter_struct);	
				dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[4]));
				dbus_message_iter_next(&iter_struct);	
				dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[5]));

				dbus_message_iter_next(&iter_struct);	
				dbus_message_iter_get_basic(&iter_struct,&ehcotimer);
				
				dbus_message_iter_next(&iter_struct);	
				dbus_message_iter_get_basic(&iter_struct,&(WtpNode->MemRTUsage));
				dbus_message_iter_next(&iter_struct);	
				dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpIfIndiscardPkts));
				dbus_message_iter_next(&iter_struct);	
				dbus_message_iter_get_basic(&iter_struct,&(WtpNode->CPURTUsage));

				dbus_message_iter_next(&iter_struct);	
				dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpStaTxBytes));
				dbus_message_iter_next(&iter_struct);	
				dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpStaRxBytes));

				dbus_message_iter_next(&iter_struct);	
				dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMomentCollectSwith));

				if(ehcotimer == 5){
					WtpNode->wtpRtCollectOnOff = 1;
				}
				else {
					WtpNode->wtpRtCollectOnOff = 2;
				}
				dbus_message_iter_next(&iter_array);
			}
		}

		else {
			dbus_message_unref(reply);
			return NULL;
		}
		dbus_message_unref(reply);

	/*ASD SENDING*/
		ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
		ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
		ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	
		query2 = dbus_message_new_method_call(BUSNAME,OBJPATH,\
				INTERFACE,ASD_DBUS_STA_METHOD_SHOW_BASIC_INFO_OF_ALL_WTP);

		dbus_error_init(&err);
		
		reply2 = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query2,-1, &err);	
		
		dbus_message_unref(query2);
		if (NULL == reply2) {
			if (dbus_error_is_set(&err)) {
				dbus_error_free(&err);
			}
			dcli_free_WtpCollectInfo(WtpHead);
			return NULL;
		}
		
		dbus_message_iter_init(reply2,&iter);
		dbus_message_iter_get_basic(&iter,&ret);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(wtp_num));
		
		dbus_message_iter_next(&iter);
		dbus_message_iter_recurse(&iter,&iter_array);
		
		if((ret == 0)&&(wtp_num !=0)){	
			
			for(i=0;i<wtp_num;i++){
				dbus_message_iter_recurse(&iter_array,&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct,&(wtp_id));
				dbus_message_iter_next(&iter_struct);	
				
				dbus_message_iter_get_basic(&iter_struct,&(assoc_req_num));
				dbus_message_iter_next(&iter_struct);	
				
				dbus_message_iter_get_basic(&iter_struct,&(reassoc_request_num));
				dbus_message_iter_next(&iter_struct);	

				dbus_message_iter_get_basic(&iter_struct,&(num_assoc_failure));
				dbus_message_iter_next(&iter_struct);	
				
				dbus_message_iter_get_basic(&iter_struct,&(num_reassoc_failure));			
				dbus_message_iter_next(&iter_struct);	
				
				dbus_message_iter_get_basic(&iter_struct,&(assoc_success));	//xiaodawei add assoc success times, 20110418
				
				WtpSearchNode = WtpHead->WtpCollectInfo_list;
				
				while(WtpSearchNode!= NULL){
					if(WtpSearchNode->wtpCurrID == wtp_id){
						WtpSearchNode->wtpAssocTimes = assoc_req_num;
						WtpSearchNode->wtpReassocTimes = reassoc_request_num;
						WtpSearchNode->wtpAssocFailtimes = num_assoc_failure;		//AP 关联失败次数
						WtpSearchNode->wtpReassocFailure = num_reassoc_failure;
						WtpSearchNode->wtpSuccAssociatedNum = assoc_success;
					}
					WtpSearchNode = WtpSearchNode->next;
				}
				dbus_message_iter_next(&iter_array);
			}
		}
		dbus_message_unref(reply2);

		return WtpHead;
}
/*table 4*/
struct WtpWirelessIfstatsInfo * show_wirelessifstatsInfo_info_of_all_wtp(int index,int localid,DBusConnection *dcli_dbus_connection, unsigned int *wtp_num, unsigned int *ret)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;	
	
	struct WtpWirelessIfstatsInfo *WtpNode = NULL;
	struct WtpWirelessIfstatsInfo *WtpHead = NULL;
	struct WtpWirelessIfstatsInfo *WtpSearchNode = NULL;
	struct WtpWirelessIfstatsInfo_radio *sub_radio_node = NULL;
	struct WtpWirelessIfstatsInfo_radio *sub_radio_search = NULL;
	struct Neighbor_AP_ELE *sub_wtp_neighbor_node = NULL;  //fengwenchao add 20110523
	int i=0,j=0;
	char *wtpModel =NULL;
	char  NOISE_VALUE = 95 ;
	double D_NOISE_VALUE = 95; 
	/*fengwenchao add 20110521*/
	int k = 0;
	char* essid = NULL;
	
	/*fengwenchao add end*/
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,\
						INTERFACE,WID_DBUS_CONF_METHOD_SHOW_ALL_WTP_WIRELESS_IFSTATS_INFORMATION);
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		*ret = WID_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,wtp_num);

	if((*ret == 0) && (wtp_num != 0)){
		if((WtpHead = (struct WtpWirelessIfstatsInfo*)malloc(sizeof(struct WtpWirelessIfstatsInfo))) == NULL){
			*ret = MALLOC_ERROR;
			dbus_message_unref(reply);
			return NULL;
		}
		memset(WtpHead,0,sizeof(struct WtpWirelessIfstatsInfo));
		WtpHead->WtpWirelessIfstatsInfo_list = NULL;
		WtpHead->WtpWirelessIfstatsInfo_last = NULL;
		WtpHead->wireless_sub_radio_head = NULL;
		WtpHead->neighbor_wtp = NULL;   //fengwenchao add 20110523

		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
		
		for (i = 0; i < *wtp_num; i++) {
			DBusMessageIter iter_struct;
			DBusMessageIter iter_neighbor_array;  //fengwenchao add 20110521
			DBusMessageIter iter_sub_array; 	
			unsigned char another_wtpWirelessIfIndex = 0;
			if((WtpNode = (struct WtpWirelessIfstatsInfo*)malloc(sizeof(struct WtpWirelessIfstatsInfo))) == NULL){
					dcli_free_wtp_wireless_ifstats_Info(WtpHead);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(WtpNode,0,sizeof(struct WtpWirelessIfstatsInfo));
			WtpNode->next = NULL;
			WtpNode->wireless_sub_radio_head = NULL;
			WtpNode->neighbor_wtp = NULL;   //fengwenchao add 20110523
			WtpNode->wtpMacAddr = (unsigned char *)malloc(MAC_LEN +1);
			memset(WtpNode->wtpMacAddr,0,(MAC_LEN +1));
			essid =(char*) malloc(ESSID_DEFAULT_LEN+1);  //fengwenchao add 20110523
			if(WtpHead->WtpWirelessIfstatsInfo_list == NULL){
				WtpHead->WtpWirelessIfstatsInfo_list = WtpNode;
				WtpHead->next = WtpNode;
			}else{
				WtpHead->WtpWirelessIfstatsInfo_last->next = WtpNode;
				
			}
			WtpHead->WtpWirelessIfstatsInfo_last = WtpNode;
			
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpCurrID));
		
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[0]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[1]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[2]));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[3]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[4]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[5]));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wifiExtensionInfoReportswitch));
			//qiuchen copy from v1.3
			#if 0
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wirelessIfHighestRxSignalStrength));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wirelessIfLowestRxSignalStrength));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wirelessIfAvgRxSignalStrength));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->math_wirelessIfAvgRxSignalStrength));
			char wirelessIfHighestRxSignalStrength2 = (char)WtpNode->wirelessIfHighestRxSignalStrength ;
			char wirelessIfLowestRxSignalStrength2 = (char)WtpNode->wirelessIfLowestRxSignalStrength ;
			char wirelessIfAvgRxSignalStrength2 = (char)WtpNode->wirelessIfAvgRxSignalStrength;

			WtpNode->wirelessIfHighestRxSignalStrength2 = 
				(wirelessIfHighestRxSignalStrength2 == 0? 
				0:(wirelessIfHighestRxSignalStrength2 - NOISE_VALUE) );
			
			WtpNode->wirelessIfLowestRxSignalStrength2 = 
				(wirelessIfLowestRxSignalStrength2 == 0? 
				0:(wirelessIfLowestRxSignalStrength2 - NOISE_VALUE));

			WtpNode->wirelessIfAvgRxSignalStrength2 = 
				(wirelessIfAvgRxSignalStrength2 == 0 ? 
				0 : (wirelessIfAvgRxSignalStrength2 - NOISE_VALUE));

			WtpNode->math_wirelessIfAvgRxSignalStrength = 
				(WtpNode->math_wirelessIfAvgRxSignalStrength ==0 ? 
				0:(WtpNode->math_wirelessIfAvgRxSignalStrength -D_NOISE_VALUE));
			#endif//qiuchen copy end
			/*
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(another_wtpWirelessIfIndex));*/
			/*packets information*/
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wirelessIfRxDataPkts));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wirelessIfTxDataPkts));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wirelessIfChStatsFrameErrorCnt));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wirelessIfDwlinkDataOctets));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wirelessIfUplinkDataOctets));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wirelessIfChStatsMacFcsErrPkts));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wirelessIfChStatsMacDecryptErrPkts));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wirelessIfChStatsMacMicErrPkts));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wirelessIfChStatsPhyErrPkts));

			/*fengwenchao add 20110521*/
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->neighbor_ap_count));	
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_recurse(&iter_struct,&iter_neighbor_array);

			if(WtpNode->neighbor_ap_count > 0)
			{	
				for(j=0; j < WtpNode->neighbor_ap_count; j++)
				{
					if((sub_wtp_neighbor_node = (struct Neighbor_AP_ELE *)malloc(sizeof(struct Neighbor_AP_ELE))) == NULL)
					{
						dcli_free_wtp_wireless_ifstats_Info(WtpHead);
						*ret = MALLOC_ERROR;
						dbus_message_unref(reply);
						if(essid){
							free(essid);
							essid = NULL;
						}
						return NULL;
					}
					memset(sub_wtp_neighbor_node,0,sizeof(struct Neighbor_AP_ELE));
					sub_wtp_neighbor_node->next = NULL;
					sub_wtp_neighbor_node->neighborapInfos_list = NULL;
					sub_wtp_neighbor_node->neighborapInfos_last = NULL;
					
					if(WtpNode->neighbor_wtp == NULL)
					{	
						WtpNode->neighbor_wtp = sub_wtp_neighbor_node;
					}
					else
					{
						WtpNode->neighbor_wtp->neighborapInfos_last->next  = sub_wtp_neighbor_node;
					}

					WtpNode->neighbor_wtp->neighborapInfos_last = sub_wtp_neighbor_node;

					memset(essid,0,ESSID_DEFAULT_LEN+1);	
					DBusMessageIter iter_sub_neighbor_struct;
					DBusMessageIter iter_sub_neighbor_array;
					DBusMessageIter iter_sub_sub_neighbor_struct;

					dbus_message_iter_recurse(&iter_neighbor_array,&iter_sub_neighbor_struct);
					dbus_message_iter_get_basic(&iter_sub_neighbor_struct,&(sub_wtp_neighbor_node->wtpid));
					dbus_message_iter_next(&iter_sub_neighbor_struct);
					dbus_message_iter_recurse(&iter_sub_neighbor_struct,&iter_sub_neighbor_array);
					for(k=0;k < ESSID_DEFAULT_LEN+1; k++)
					{
						dbus_message_iter_recurse(&iter_sub_neighbor_array,&iter_sub_sub_neighbor_struct);
						//dbus_message_iter_get_basic(&iter_sub_sub_neighbor_struct,&essid[k]);
						dbus_message_iter_get_basic(&iter_sub_sub_neighbor_struct,&sub_wtp_neighbor_node->ESSID[k]);
						dbus_message_iter_next(&iter_sub_sub_neighbor_struct);
						dbus_message_iter_next(&iter_sub_neighbor_array);
					}
					//printf("essid = %s \n",essid);
					dbus_message_iter_next(&iter_sub_neighbor_struct);
					dbus_message_iter_next(&iter_neighbor_array);
					/*sub_wtp_neighbor_node->ESSID = (char*)malloc(ESSID_DEFAULT_LEN+1);
					memset(sub_wtp_neighbor_node->ESSID,0,ESSID_DEFAULT_LEN+1);
					memcpy(sub_wtp_neighbor_node->ESSID,essid,ESSID_DEFAULT_LEN);*/
					//printf("WtpNode->neighbor_wtp->ESSID   =  %s  \n",sub_wtp_neighbor_node->ESSID);
					//printf("sub_wtp_neighbor_node->ESSID   =  %s  \n",sub_wtp_neighbor_node->ESSID);
					//WtpNode->neighbor_wtp = WtpNode->neighbor_wtp->next;
				}
			}
			else
			{
				if((sub_wtp_neighbor_node = (struct Neighbor_AP_ELE *)malloc(sizeof(struct Neighbor_AP_ELE))) == NULL)
				{
					dcli_free_wtp_wireless_ifstats_Info(WtpHead);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					if(essid){
						free(essid);
						essid = NULL;
					}
					return NULL;
				}
				memset(sub_wtp_neighbor_node,0,sizeof(struct Neighbor_AP_ELE));
				sub_wtp_neighbor_node->next = NULL;
				sub_wtp_neighbor_node->neighborapInfos_list = NULL;
				sub_wtp_neighbor_node->neighborapInfos_last = NULL;


				if(WtpNode->neighbor_wtp == NULL)
				{	
					WtpNode->neighbor_wtp= sub_wtp_neighbor_node;
				}
				else
				{ 
					WtpNode->neighbor_wtp->neighborapInfos_last->next = sub_wtp_neighbor_node;
				}

				WtpNode->neighbor_wtp->neighborapInfos_last = sub_wtp_neighbor_node;

				memset(essid,0,ESSID_DEFAULT_LEN+1);	
				DBusMessageIter iter_sub_neighbor_struct;
				DBusMessageIter iter_sub_neighbor_array;
				DBusMessageIter iter_sub_sub_neighbor_struct;

				dbus_message_iter_recurse(&iter_neighbor_array,&iter_sub_neighbor_struct);
				dbus_message_iter_get_basic(&iter_sub_neighbor_struct,&(sub_wtp_neighbor_node->wtpid));
				dbus_message_iter_next(&iter_sub_neighbor_struct);
				dbus_message_iter_recurse(&iter_sub_neighbor_struct,&iter_sub_neighbor_array);

				for(k=0;k < ESSID_DEFAULT_LEN+1; k++)
				{
					dbus_message_iter_recurse(&iter_sub_neighbor_array,&iter_sub_sub_neighbor_struct);
					//dbus_message_iter_get_basic(&iter_sub_sub_neighbor_struct,&essid[k]);
					dbus_message_iter_get_basic(&iter_sub_sub_neighbor_struct,&sub_wtp_neighbor_node->ESSID[k]);
					dbus_message_iter_next(&iter_sub_sub_neighbor_struct);
					dbus_message_iter_next(&iter_sub_neighbor_array);
				}
				//printf("essid = %s \n",essid);
				dbus_message_iter_next(&iter_sub_neighbor_struct);
				dbus_message_iter_next(&iter_neighbor_array);
				
				/*sub_wtp_neighbor_node->ESSID = (char*)malloc(ESSID_DEFAULT_LEN+1);
				memset(sub_wtp_neighbor_node->ESSID,0,ESSID_DEFAULT_LEN+1);
				memcpy(sub_wtp_neighbor_node->ESSID,essid,ESSID_DEFAULT_LEN);*/
				//printf("WtpNode->neighbor_wtp->ESSID   =  %s  \n",WtpNode->neighbor_wtp->ESSID);
				//printf("sub_wtp_neighbor_node->ESSID   =  %s  \n",sub_wtp_neighbor_node->ESSID);
			}
			if(essid)
			{
				free(essid);
				essid = NULL;
			}
			/*fengwenchao add end*/
			/*for radio information*/
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtp_radio_num));	

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_recurse(&iter_struct,&iter_sub_array);

			for(j=0; j<WtpNode->wtp_radio_num; j++){	
				DBusMessageIter iter_sub_struct;
				if((sub_radio_node = (struct WtpWirelessIfstatsInfo_radio*)malloc(sizeof(struct WtpWirelessIfstatsInfo_radio))) == NULL){
					dcli_free_wtp_wireless_ifstats_Info(WtpHead);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
				}
			
				memset(sub_radio_node,0,sizeof(struct WtpWirelessIfstatsInfo_radio));
				sub_radio_node->next = NULL;
				sub_radio_node->WtpWirelessIfstatsInfo_radio_list = NULL;
				sub_radio_node->WtpWirelessIfstatsInfo_radio_last = NULL;

				if(WtpNode->wireless_sub_radio_head == NULL){
					WtpNode->wireless_sub_radio_head = sub_radio_node;
				}else{
					WtpNode->wireless_sub_radio_head->WtpWirelessIfstatsInfo_radio_last->next = sub_radio_node;
					
				}
				WtpNode->wireless_sub_radio_head->WtpWirelessIfstatsInfo_radio_last = sub_radio_node;

				dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->wtpWirelessIfIndex));	


				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->wirelessIfUpdownTimes));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->wirelessIfChStatsUplinkUniFrameCnt));
				
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->wirelessIfChStatsDwlinkUniFrameCnt));	
				
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->wirelessIfUpChStatsFrameNonUniFrameCnt));	
				
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->wirelessIfDownChStatsFrameNonUniFrameCnt));	
				
				sub_radio_node->wirelessIfApChStatsFrameFragRate = 0;

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->sub_wirelessIfRxDataPkts));
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->sub_wirelessIfTxDataPkts));

				sub_radio_node->wirelessIfRxDataFrameCnt = sub_radio_node->sub_wirelessIfRxDataPkts;//book modify,2011-5-19
				sub_radio_node->wirelessIfTxDataFrameCnt = sub_radio_node->sub_wirelessIfTxDataPkts;//book modify
				
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->sub_wirelessIfChStatsFrameErrorCnt));
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->sub_wirelessIfRxErrPkts));
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->sub_wirelessIfTxDropPkts));
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->sub_wirelessIfRxDropPkts));
				
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->sub_wirelessIfUplinkDataOctets));
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->sub_wirelessIfDwlinkDataOctets));
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->sub_wirelessIfChStatsMacFcsErrPkts));
				
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->sub_wirelessIfChStatsMacDecryptErrPkts));
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->sub_wirelessIfChStatsMacMicErrPkts));
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->sub_wirelessIfChStatsPhyErrPkts));
				#if 0//qiuchen copy from v1.3
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->radio_cur_snr));
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->radio_max_snr));
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->radio_min_snr));
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->radio_aver_snr));
				#endif
				/*fengwenchao add 20110926*/
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->radio_cur_snr2));
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->radio_max_snr2));
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->radio_min_snr2));
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->radio_aver_snr));	
				//printf("rec!!! snr max is %d,snr min is %d.\n!!!!!!!!!!!!!!!!!",sub_radio_node->radio_max_snr2,sub_radio_node->radio_min_snr2);
				/*fengwenchao add end*/
				//qiuchen copy end
				/*add for new mib requirement 20100809*/
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->sub_rx_pkt_mgmt));
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->sub_tx_pkt_mgmt));
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->sub_rx_mgmt));
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->sub_tx_mgmt));

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->sub_total_rx_bytes));
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->sub_total_tx_bytes));
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->sub_total_rx_pkt));
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->sub_total_tx_pkt));
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->wirelessIfTxCtrlFrameCnt));
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->wirelessIfRxCtrlFrameCnt));

				/*for signal information*/
    			dbus_message_iter_next(&iter_sub_struct);	
    			dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->wirelessIfTxSignalPkts));

    			dbus_message_iter_next(&iter_sub_struct);	
    			dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->wirelessIfRxSignalPkts));

    			dbus_message_iter_next(&iter_sub_struct);	
    			dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->wirelessIfChStatsDwlinkTotRetryPkts));
    			
    			dbus_message_iter_next(&iter_sub_struct);	
    			dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->wirelessIfChStatsFrameRetryCnt));
             		/*fengwenchao add 20110617*/
			 dbus_message_iter_next(&iter_sub_struct);	 
			 dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->wirelessIfRxDataFrameCnt));
			// printf(" wirelessIfRxDataFrameCnt  =  %d \n ",sub_radio_node->wirelessIfRxDataFrameCnt);
			 dbus_message_iter_next(&iter_sub_struct);	 
			 dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->wirelessIfTxDataFrameCnt));
			//  printf(" wirelessIfTxDataFrameCnt  =  %d \n ",sub_radio_node->wirelessIfTxDataFrameCnt);
			/*fengwenchao add 20110617*/
			#if 0//qiuchen copy from v1.3
				char radio_cur_snr2 = (char)sub_radio_node->radio_cur_snr;
				char radio_max_snr2 = (char)sub_radio_node->radio_max_snr;
				char radio_min_snr2 = (char)sub_radio_node->radio_min_snr;
				double radio_aver_snr = sub_radio_node->radio_aver_snr;

				//printf("####radio_max_snr2 %d \n",radio_max_snr2);
				//printf("####radio_min_snr2 %d \n",radio_min_snr2);
				/*sub_radio_node->radio_cur_snr2 = radio_cur_snr2 - NOISE_VALUE;
				sub_radio_node->radio_max_snr2 = radio_max_snr2 - NOISE_VALUE;
				sub_radio_node->radio_min_snr2 = radio_min_snr2 - NOISE_VALUE;
				sub_radio_node->radio_aver_snr = sub_radio_node->radio_aver_snr - NOISE_VALUE;*/
				
				sub_radio_node->radio_cur_snr2 = 
					(radio_cur_snr2 == 0? 0:(radio_cur_snr2 - NOISE_VALUE));
				
				sub_radio_node->radio_max_snr2 = 
					((radio_max_snr2 == 0)? 0:(radio_max_snr2 - NOISE_VALUE));

				sub_radio_node->radio_min_snr2 = 
					((radio_min_snr2 == 0) ? 0 : (radio_min_snr2 - NOISE_VALUE));

				sub_radio_node->radio_aver_snr = 
					(radio_aver_snr ==0 ? 0:(radio_aver_snr -D_NOISE_VALUE));
			#endif//qiuchen copy from v1.3 end

				dbus_message_iter_next(&iter_sub_array);
			}
			dbus_message_iter_next(&iter_array);
		}
	}
	else{
		dbus_message_unref(reply);
		return NULL;
	}
	
	dbus_message_unref(reply);


/*receive ASD massege in this function*/
	DBusMessage *query2, *reply2; 
	DBusError err2;
	unsigned int ret2;
	unsigned int asd_wtp_num = 0;
	unsigned int asd_radio_num = 0;
	unsigned int asd_radio_id = 0;
	unsigned int asd_wtp_id = 0;	

	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	
	query2 = dbus_message_new_method_call(BUSNAME,OBJPATH,\
						INTERFACE,ASD_DBUS_STA_METHOD_SHOW_RADIO_WIRELESS_INFO_BYWTPID);
	
	dbus_error_init(&err2);

	reply2 = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query2,-1, &err2);
	
	dbus_message_unref(query2);
	if (NULL == reply2) {
		if (dbus_error_is_set(&err2)) {
			dbus_error_free(&err2);
		}
		dcli_free_wtp_wireless_ifstats_Info(WtpHead);
		return NULL;
	}
	dbus_message_iter_init(reply2,&iter);
	dbus_message_iter_get_basic(&iter,&ret2);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(asd_wtp_num));

	dbus_message_iter_next(&iter);
	dbus_message_iter_recurse(&iter,&iter_array);

	if(ret2 == 0){	
		for(j=0;j<asd_wtp_num;j++){
			DBusMessageIter	 iter_struct;
			DBusMessageIter	 iter_sub_array;	

			dbus_message_iter_recurse(&iter_array,&iter_struct);
			//dbus_message_iter_get_basic(&iter_struct,&(asd_radio_id));// a0
				
		//	dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(asd_wtp_id));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(asd_radio_num));
			
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_recurse(&iter_struct,&iter_sub_array);
			
			for(i=0; i<asd_radio_num; i++) {		
				DBusMessageIter  iter_sub_struct;
				unsigned int auth_tms=0;
				unsigned int repauth_tms=0;
				unsigned int assoc_req_num = 0; 
				unsigned int assoc_resp_num = 0; 
				
				unsigned int rx_data_pkts = 0;
				unsigned int tx_data_pkts = 0;
				
				unsigned int sta_num = 0;
	
				dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(asd_radio_id));// a0
				
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(auth_tms));// a1
				
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(repauth_tms));//
				
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(assoc_req_num));//
				
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(assoc_resp_num));//
				
				
				/*dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(rx_data_pkts));//
				printf("   rx_data_pkts  =  %d \n",rx_data_pkts);
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(tx_data_pkts));// a10
				printf("   tx_data_pkts  =  %d \n",tx_data_pkts);*/
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta_num));//

				WtpSearchNode = WtpHead->WtpWirelessIfstatsInfo_list;

				while(WtpSearchNode!= NULL){
					if(WtpSearchNode->wtpCurrID == asd_wtp_id){

						if(WtpSearchNode->wireless_sub_radio_head == NULL)
							break;

						else{
							WtpSearchNode->asd_radio_num= asd_radio_num ;
							sub_radio_search = WtpSearchNode->wireless_sub_radio_head;
							while(sub_radio_search !=NULL){
								
								unsigned char asd_radio_local_id = asd_radio_id%4;
								if(sub_radio_search->wtpWirelessIfIndex == asd_radio_local_id){
									
									//sub_radio_search->wirelessIfRxDataFrameCnt = rx_data_pkts;
									sub_radio_search->wirelessIfRxAuthenFrameCnt = auth_tms;
									sub_radio_search->wirelessIfRxAssociateFrameCnt = assoc_req_num;
									
									//sub_radio_search->wirelessIfTxDataFrameCnt = tx_data_pkts;
									sub_radio_search->wirelessIfTxAuthenFrameCnt = repauth_tms;
									sub_radio_search->wirelessIfTxAssociateFrameCnt = assoc_resp_num;
									sub_radio_search->wirelessIfChStatsNumStations = sta_num;
								}
								sub_radio_search = sub_radio_search->next;
							}
						}
					}
					WtpSearchNode = WtpSearchNode->next;
				}
				dbus_message_iter_next(&iter_sub_array);
			}
			dbus_message_iter_next(&iter_array);
		}
	}
	
	dbus_message_unref(reply2);
	return WtpHead;
}
/*table 5*/
struct WtpDeviceInfo* show_WtpDeviceInfo_of_all_wtp(int index,int localid,DBusConnection *dcli_dbus_connection, unsigned int *num, unsigned int *ret)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;	
	DBusMessageIter iter_struct;
	
	struct WtpDeviceInfo *WtpNode = NULL;
	struct WtpDeviceInfo *WtpHead = NULL;
	struct WtpDeviceInfo *WtpSearchNode = NULL;
	
	int i =0;
	char *wtpModel =NULL;
	char * wtpip =NULL;
	unsigned int result = 0;
	char ip[WTP_WTP_IP_LEN+1]={0};

	unsigned int cpu = 0;
	unsigned char ipmode = 0;
	unsigned short memoryall = 0;
	unsigned char memoryuse = 0;
	unsigned short flashall = 0;
	unsigned int flashempty = 0;
	unsigned char temperature = 0;

	char *cpuType_str = NULL;
	char *flashType_str = NULL ;
	char *memType_str = NULL;
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,\
						INTERFACE,WID_DBUS_CONF_METHOD_SHOW_ALL_WTP_DEVICE_INFORMATION);
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		*ret = WID_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,num);

	if((*ret == 0) && (num !=0 )){
		if((WtpHead = (struct WtpDeviceInfo*)malloc(sizeof(struct WtpDeviceInfo))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
		memset(WtpHead,0,sizeof(struct WtpDeviceInfo));
		WtpHead->WtpDeviceInfo_list = NULL;
		WtpHead->WtpDeviceInfo_last = NULL;

		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
			
		for (i = 0; i < *num; i++) {
			if((WtpNode = (struct WtpDeviceInfo*)malloc(sizeof(struct WtpDeviceInfo))) == NULL){
					dcli_free_wtp_device_Info(WtpHead);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(WtpNode,0,sizeof(struct WtpDeviceInfo));
			WtpNode->next = NULL;

			WtpNode->wtpMacAddr = (unsigned char *)malloc(MAC_LEN +1);
			memset(WtpNode->wtpMacAddr,0,(MAC_LEN +1));

			if(WtpHead->WtpDeviceInfo_list == NULL){
				WtpHead->WtpDeviceInfo_list = WtpNode;
				WtpHead->next = WtpNode;
			}else{
				WtpHead->WtpDeviceInfo_last->next = WtpNode;
				
			}
			WtpHead->WtpDeviceInfo_last = WtpNode;
			
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpCurrID));
		
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[0]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[1]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[2]));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[3]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[4]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[5]));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpCPUusageThreshhd));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMemUsageThreshhd));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(wtpModel));

			WtpNode->wtpModel = (char*)malloc(strlen(wtpModel)+1);
			memset(WtpNode->wtpModel, 0, strlen(wtpModel)+1);
			memcpy(WtpNode->wtpModel, wtpModel, strlen(wtpModel));	

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(cpuType_str));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(memType_str));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(flashType_str));

			
           //fengwenchao add 20101231
			if(strlen(cpuType_str) <= WTP_TYPE_DEFAULT_LEN)
			{
				memset(WtpNode->cpuType_str, 0, WTP_TYPE_DEFAULT_LEN);
				memcpy(WtpNode->cpuType_str, cpuType_str, strlen(cpuType_str));	
			}						
			if(strlen(memType_str) <= WTP_TYPE_DEFAULT_LEN)
			{
				memset(WtpNode->memType_str, 0, WTP_TYPE_DEFAULT_LEN);
			 	memcpy(WtpNode->memType_str, memType_str, strlen(memType_str));
			}
			if(strlen(flashType_str) <= WTP_TYPE_DEFAULT_LEN)
			{
				memset(WtpNode->flashType_str,0,WTP_TYPE_DEFAULT_LEN);
			 	memcpy(WtpNode->flashType_str,flashType_str,strlen(flashType_str));
			}							
			//fengwenchao add end
			

			/*ap  flash information*/
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&cpu);//unsigned int cpu;

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&ipmode);//unsigned char ipmode;

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&memoryall);//unsigned short memoryall;

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&memoryuse);//unsigned char memoryuse;
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&flashall);//unsigned short flashall;

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&flashempty);//unsigned int flashempty;
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&temperature);//unsigned char temperature;
			
			WtpNode->wtpCPURTUsage = cpu;						
			WtpNode->wtpMemoryCapacity = (1024*1024)*(memoryall);
			WtpNode->wtpMemRTUsage = (unsigned int)memoryuse;	
			
			WtpNode->wtpFlashCapacity = (1024*1024)*(flashall);	
			WtpNode->wtpFlashRemain = flashempty;				
			WtpNode->wtpWayGetIP = ipmode;						
			WtpNode->wtpWorkTemp = temperature;					

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&WtpNode->wtpCPUAvgUsage);
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&WtpNode->wtpCPUPeakUsage);
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&WtpNode->wtpMemAvgUsage);
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&WtpNode->wtpMemPeakUsage);

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&wtpip);

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->cpu_collect_average));

			WtpNode->WtpIP=(char *)malloc(WTP_WTP_IP_LEN+1);
			memset(WtpNode->WtpIP,0,WTP_WTP_IP_LEN+1);

			result = wtp_check_wtp_ip_addr(ip,wtpip);
			
			if(result != 1)
				memcpy(WtpNode->WtpIP,wtpip,strlen(wtpip));
			else
				memcpy(WtpNode->WtpIP,ip,strlen(ip));

			dbus_message_iter_next(&iter_array);
		}
	}

	dbus_message_unref(reply);

	return WtpHead;
}
/*table 6*/
struct WtpDataPktsInfo* show_WtpDataPktsInfo_of_all_wtp(int index,int localid,DBusConnection *dcli_dbus_connection, unsigned int *num, unsigned int *ret)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;	
	DBusMessageIter iter_struct;
	
	struct WtpDataPktsInfo *WtpNode = NULL;
	struct WtpDataPktsInfo *WtpHead = NULL;
	struct WtpDataPktsInfo *WtpSearchNode = NULL;
	
	int i=0;
	char * wtpip=NULL;
	unsigned int result = 0;
	char ip[WTP_WTP_IP_LEN+1];

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	//printf("BUSNAME=%s,OBJPATH=%s,INTERFACE=%s.\n",BUSNAME,OBJPATH,INTERFACE);

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,\
						INTERFACE,WID_DBUS_CONF_METHOD_SHOW_ALL_WTP_DATA_PKTS_INFORMATION);
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		*ret = WID_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,num);

	if((*ret == 0) && (num != 0))
	{
		if((WtpHead = (struct WtpDataPktsInfo*)malloc(sizeof(struct WtpDataPktsInfo))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
		memset(WtpHead,0,sizeof(struct WtpDataPktsInfo));
		WtpHead->WtpDataPktsInfo_list = NULL;
		WtpHead->WtpDataPktsInfo_last = NULL;

		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
			
		for (i = 0; i < *num; i++) {
			if((WtpNode = (struct WtpDataPktsInfo*)malloc(sizeof(struct WtpDataPktsInfo))) == NULL){
					dcli_free_wtp_data_pkts_Info(WtpHead);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(WtpNode,0,sizeof(struct WtpDataPktsInfo));
			WtpNode->next = NULL;

			WtpNode->wtpMacAddr = (unsigned char *)malloc(MAC_LEN +1);
			memset(WtpNode->wtpMacAddr,0,(MAC_LEN +1));

			if(WtpHead->WtpDataPktsInfo_list == NULL){
				WtpHead->WtpDataPktsInfo_list = WtpNode;
				WtpHead->next = WtpNode;
			}else{
				WtpHead->WtpDataPktsInfo_last->next = WtpNode;
				
			}
			WtpHead->WtpDataPktsInfo_last = WtpNode;
			
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpCurrID));
		
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[0]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[1]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[2]));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[3]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[4]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[5]));

			/*wireless information*/
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpWirelessMacRecvAllByte));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpWirelessMacSendAllByte));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpWirelessMacRecvPack));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpWirelessMacRecvWrongPack));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpDropPkts));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpWirelessMacSendPack));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpWirelessMacSendErrPack));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpWirelessMacRecvRightPack));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpWirelessMacSendRightPack));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpWirelessradioRecvBytes));
			

			/*wired information*/
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpWiredMacRecvRightByte));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpWiredMacSendAllByte));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpWiredMacRecvWrongPack));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpWiredMacRecvPack));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpWiredMacRecvRightPack));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpWiredMacSendPack));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpWiredMacSendErrPack));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpWiredMacSendRightPack));

			/*ath information*/
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpWirelesscoreRecvBytes));

			/*wtp*/
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpWirelessRecvFlowByte));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpWirelessSendFlowByte));
			
			dbus_message_iter_next(&iter_array);
		}
	}

	dbus_message_unref(reply);

	return WtpHead;
}
/*table 7*/
struct WtpStatsInfo* show_WtpStatsInfo_of_all_wtp(int index,int localid,DBusConnection *dcli_dbus_connection, unsigned int *num, unsigned int *ret)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;	
	
	struct WtpStatsInfo *WtpNode = NULL;
	struct WtpStatsInfo *WtpHead = NULL;
	struct WtpStatsInfo *WtpSearchNode = NULL;
	
	int i=0,j=0;
	char * wtpip = NULL;
	unsigned int result = 0;
	char * wtpModel = NULL;
	unsigned char wlanid[WLAN_NUM] = {0}; //should make 
	unsigned char eth_num = 0;
	unsigned int is_refuse_lowrssi = 0; //fengwenchao add 20111122 for chinamobile-177
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,\
						INTERFACE,WID_DBUS_CONF_METHOD_SHOW_ALL_WTP_STATS_INFORMATION);
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		*ret = WID_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,num);

	if((*ret == 0) && (num != 0)){
		if((WtpHead = (struct WtpStatsInfo*)malloc(sizeof(struct WtpStatsInfo))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
		memset(WtpHead,0,sizeof(struct WtpStatsInfo));
		WtpHead->WtpStatsInfo_list = NULL;
		WtpHead->WtpStatsInfo_last = NULL;

		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
			
		for (i = 0; i < *num; i++) {
			DBusMessageIter iter_struct;
			DBusMessageIter iter_sub_array; 	
			if((WtpNode = (struct WtpStatsInfo*)malloc(sizeof(struct WtpStatsInfo))) == NULL){
					dcli_free_wtp_stats_Info(WtpHead);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(WtpNode,0,sizeof(struct WtpStatsInfo));
			WtpNode->next = NULL;

			WtpNode->wtpMacAddr = (unsigned char *)malloc(MAC_LEN +1);
			memset(WtpNode->wtpMacAddr,0,(MAC_LEN +1));

			if(WtpHead->WtpStatsInfo_list == NULL){
				WtpHead->WtpStatsInfo_list = WtpNode;
				WtpHead->next = WtpNode;
			}else{
				WtpHead->WtpStatsInfo_last->next = WtpNode;
				
			}
			WtpHead->WtpStatsInfo_last = WtpNode;
			
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpCurrID));
		
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[0]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[1]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[2]));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[3]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[4]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[5]));

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(wtpModel));

			WtpNode->wtpModel = (char*)malloc(strlen(wtpModel)+1);
			memset(WtpNode->wtpModel, 0, strlen(wtpModel)+1);
			memcpy(WtpNode->wtpModel, wtpModel, strlen(wtpModel));	

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpSSIDNum));			//可配置的最大wlan数

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpConfigBSSIDNum));	//existing bss count
			
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpRadioCount));

			/*fengwenchao add for GM-3, 20111125*/
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->heart_time_avarge));

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->heart_lose_pkt));

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->heart_transfer_pkt));			
			/*fengwenchao add end*/
			
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpAllowConUsrMnt));

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(eth_num));

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpBwlanNum));

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_recurse(&iter_struct,&iter_sub_array);

			for(j=0; j<WtpNode->wtpBwlanNum; j++){	
				DBusMessageIter iter_sub_struct;
				
				dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(wlanid[j]));	

				WtpNode->wtpSupportSSID[j] = wlanid[j];
				WtpNode->total_if_num = eth_num + WtpNode->wtpRadioCount ;

				dbus_message_iter_next(&iter_sub_array);
			}
			/*fengwenchao add for chinamobile-177,20111122*/
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(is_refuse_lowrssi));
			WtpNode->is_refuse_lowrssi = is_refuse_lowrssi;
			/*fengwenchao add end*/
			dbus_message_iter_next(&iter_array);
		}
	}

	else {
		dbus_message_unref(reply);
		return NULL;
	}
	dbus_message_unref(reply);


	/*Receive ASD information*/
	unsigned int assoc_num = 0; 
	unsigned int reassoc_num = 0; 
	unsigned int assoc_success_num = 0;
	unsigned int reassoc_success_num = 0;
	unsigned int assoc_failure_num = 0; 
	unsigned int reassoc_failure_num = 0;
	unsigned int abnormal_down_num = 0; 
	unsigned int stanum = 0; 
	unsigned int deny_num = 0; 
	unsigned int acc_tms = 0; 
	unsigned int auth_tms = 0; 
	unsigned int repauth_tms = 0; 
	unsigned int auth_succ_tms = 0; 
	unsigned int auth_fail_tms = 0; 
	time_t StaTime;
	unsigned int asd_wtp_num = 0;
	unsigned int asd_wtp_id ;
	unsigned int ret2 = 0;
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,\
						INTERFACE,ASD_DBUS_STA_METHOD_SHOW_STATS_INFO_OF_ALL_WTP);

	dbus_error_init(&err);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);	
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		dcli_free_wtp_stats_Info(WtpHead);
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(asd_wtp_num));
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_recurse(&iter,&iter_array);
		
		if((ret2 == 0)&&(asd_wtp_num !=0)){	
		for(i=0;i<asd_wtp_num;i++){
			DBusMessageIter iter_struct;
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(asd_wtp_id));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(assoc_num));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(reassoc_num));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(assoc_success_num));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(reassoc_success_num));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(assoc_failure_num));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(reassoc_failure_num));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(abnormal_down_num));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(stanum));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(StaTime));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(deny_num));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(acc_tms));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(auth_tms));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(repauth_tms));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(auth_succ_tms));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(auth_fail_tms));

			WtpSearchNode = WtpHead->WtpStatsInfo_list;
			while(WtpSearchNode!= NULL){
				if(WtpSearchNode->wtpCurrID == asd_wtp_id){
					WtpSearchNode->wtpMountConUsrTimes = assoc_num;
					WtpSearchNode->wtpMountConReasTimes = reassoc_num;
					WtpSearchNode->wtpMountConSuccTimes = assoc_success_num;
					WtpSearchNode->wtpMountReConSuccTimes  = reassoc_success_num;
					WtpSearchNode->wtpMountConFailTimes = assoc_failure_num;
					WtpSearchNode->wtpMountReConFailTimes = reassoc_failure_num;
					WtpSearchNode->wtpStatsDisassociated = abnormal_down_num;
					WtpSearchNode->wtpOnlineUsrNum = stanum;
					WtpSearchNode->wtpAllStaAssTime = StaTime;
					WtpSearchNode->wtpUsrRequestConnect = auth_tms; 	
					WtpSearchNode->wtpResponseUsrConnect = repauth_tms;		
					WtpSearchNode->wtpLoginSuccNum = acc_tms;			
					WtpSearchNode->wtpLessResourceRefuseNewUsrMount = deny_num;		
					WtpSearchNode->wtpResponseUsrSuccConnect = auth_succ_tms;			
					WtpSearchNode->wtpResponseUsrFailConnect = auth_fail_tms;			
				}
				WtpSearchNode = WtpSearchNode->next;
			}
			
			dbus_message_iter_next(&iter_array);
		}
	}

	dbus_message_unref(reply);

	return WtpHead;
}
/*table 8*/
struct WtpWlanStatsInfo* show_WtpWlanStatsInfo_of_all_wtp(int index,int localid,DBusConnection *dcli_dbus_connection, unsigned int *num, unsigned int *ret)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;	
	
	struct WtpWlanStatsInfo  *WtpNode = NULL;
	struct WtpWlanStatsInfo  *WtpHead = NULL;
	struct WtpWlanStatsInfo  *WtpSearchNode = NULL;
	struct WtpWlanStatsInfo_wlan  *sub_wlan_node =NULL;
	struct WtpWlanStatsInfo_wlan  *sub_wlan_search =NULL;
	
	int i=0,j=0,l = 0;
	char * wtpip = NULL;
	char *wlan_essid =(char*)malloc(ESSID_DEFAULT_LEN);
	if(!wlan_essid){
		printf("%s,%d,wlan_essid malloc error\n",__func__,__LINE__);
		return NULL;
	}
	memset(wlan_essid,0,ESSID_DEFAULT_LEN);
	unsigned int result = 0;
	unsigned char wlanid[WLAN_NUM] = {0}; //should make 

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,\
						INTERFACE,WID_DBUS_CONF_METHOD_SHOW_ALL_WLAN_STATS_INFORMATION);
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		*ret = WID_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,num);

	if((*ret == 0) && (num != 0)){
		if((WtpHead = (struct WtpWlanStatsInfo*)malloc(sizeof(struct WtpWlanStatsInfo))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
		memset(WtpHead,0,sizeof(struct WtpWlanStatsInfo));
		WtpHead->WtpWlanStatsInfo_list = NULL;
		WtpHead->WtpWlanStatsInfo_last = NULL;

		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
			
		for (i = 0; i < *num; i++) {
			DBusMessageIter iter_struct;
 			DBusMessageIter iter_sub_array; 	
			if((WtpNode = (struct WtpWlanStatsInfo*)malloc(sizeof(struct WtpWlanStatsInfo))) == NULL){
					dcli_free_wlan_stats_Info(WtpHead);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(WtpNode,0,sizeof(struct WtpWlanStatsInfo));
			WtpNode->next = NULL;

			WtpNode->wtpMacAddr = (unsigned char *)malloc(MAC_LEN +1);
			memset(WtpNode->wtpMacAddr,0,(MAC_LEN +1));



			if(WtpHead->WtpWlanStatsInfo_list == NULL){
				WtpHead->WtpWlanStatsInfo_list = WtpNode;
			}else{
				WtpHead->WtpWlanStatsInfo_last->next = WtpNode;
				
			}
			WtpHead->WtpWlanStatsInfo_last = WtpNode;
			
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpCurrID));
		
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[0]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[1]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[2]));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[3]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[4]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[5]));

 			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpL2Insulate));	//wtpL2Insulate

 			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpBwlanNum));
			
 			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_recurse(&iter_struct,&iter_sub_array);

 
			 for(j=0; j<WtpNode->wtpBwlanNum; j++){  
				 DBusMessageIter iter_sub_struct;
				 DBusMessageIter iter_sub_essid_struct;
				  DBusMessageIter iter_sub_essid_array;
				 if((sub_wlan_node = (struct WtpWlanStatsInfo_wlan*)malloc(sizeof(struct WtpWlanStatsInfo_wlan))) == NULL){
					free(sub_wlan_node);
					sub_wlan_node =NULL;
					dcli_free_wlan_stats_Info(WtpHead);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
				}
				memset(sub_wlan_node,0,sizeof(struct WtpWlanStatsInfo_wlan));
				sub_wlan_node->next = NULL;
				sub_wlan_node->WtpWlanStatsInfo_wlan_list = NULL;
				sub_wlan_node->WtpWlanStatsInfo_wlan_last = NULL;

				if(WtpNode->WtpWlanStatsInfo_wlan_head == NULL){
					WtpNode->WtpWlanStatsInfo_wlan_head = sub_wlan_node;
					WtpHead->next = WtpNode;
				}else{
					WtpNode->WtpWlanStatsInfo_wlan_head->WtpWlanStatsInfo_wlan_last->next = sub_wlan_node;
				}
				
				WtpNode->WtpWlanStatsInfo_wlan_head->WtpWlanStatsInfo_wlan_last = sub_wlan_node;

				sub_wlan_node->wirelessSSID = (unsigned char *)malloc(MAC_LEN +1);
				memset(sub_wlan_node->wirelessSSID,0,(MAC_LEN +1));

				sub_wlan_node->wlan_essid = (char *)malloc(ESSID_DEFAULT_LEN+1);
				memset(sub_wlan_node->wlan_essid, 0, ESSID_DEFAULT_LEN+1);

				 dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
				 dbus_message_iter_get_basic(&iter_sub_struct,&(wlanid[j])); 
				 
				 dbus_message_iter_next(&iter_sub_struct);
				 dbus_message_iter_get_basic(&iter_sub_struct,&(sub_wlan_node->wlanL2Isolation)); 		//xiaodawei add wlanL2isolation, 20110304

				 dbus_message_iter_next(&iter_sub_struct);
				 dbus_message_iter_recurse(&iter_sub_struct,&iter_sub_essid_array);
				  for(l=0;l<ESSID_DEFAULT_LEN;l++){
					  dbus_message_iter_recurse(&iter_sub_essid_array,&iter_sub_essid_struct);
					  dbus_message_iter_get_basic(&iter_sub_essid_struct,&wlan_essid[l]);
					   dbus_message_iter_next(&iter_sub_essid_struct);
					  dbus_message_iter_next(&iter_sub_essid_array);
				  }
				 memcpy(sub_wlan_node->wlan_essid, wlan_essid, ESSID_DEFAULT_LEN);
				 
				 dbus_message_iter_next(&iter_sub_struct);
				 dbus_message_iter_get_basic(&iter_sub_struct,&(sub_wlan_node->wtpUsrNumAppendThreshold)); 
				 
				 dbus_message_iter_next(&iter_sub_struct);
				 dbus_message_iter_get_basic(&iter_sub_struct,&(sub_wlan_node->wtpAttachedFlowThreshold)); 
				 
				 dbus_message_iter_next(&iter_sub_struct);
				 dbus_message_iter_get_basic(&iter_sub_struct,&(sub_wlan_node->wtpLoadBalance)); 
				 
				 dbus_message_iter_next(&iter_sub_struct);
				 dbus_message_iter_get_basic(&iter_sub_struct,&(sub_wlan_node->wtpSsidBroadcast)); 
				 
				 WtpNode->wtpSupportSSID[j] = wlanid[j];
				 sub_wlan_node->wlan_id = wlanid[j];

				 dbus_message_iter_next(&iter_sub_struct);	
				 dbus_message_iter_get_basic(&iter_sub_struct,&(sub_wlan_node->wirelessSSID[0]));
				 
				 dbus_message_iter_next(&iter_sub_struct);	
				 dbus_message_iter_get_basic(&iter_sub_struct,&(sub_wlan_node->wirelessSSID[1]));
				 
				 dbus_message_iter_next(&iter_sub_struct);	
				 dbus_message_iter_get_basic(&iter_sub_struct,&(sub_wlan_node->wirelessSSID[2]));
				 
				 dbus_message_iter_next(&iter_sub_struct);	
				 dbus_message_iter_get_basic(&iter_sub_struct,&(sub_wlan_node->wirelessSSID[3]));
				 
				 dbus_message_iter_next(&iter_sub_struct);	
				 dbus_message_iter_get_basic(&iter_sub_struct,&(sub_wlan_node->wirelessSSID[4]));
				 
				 dbus_message_iter_next(&iter_sub_struct);	
				 dbus_message_iter_get_basic(&iter_sub_struct,&(sub_wlan_node->wirelessSSID[5]));
				/*fengwenchao add 20110426 for dot11WlanDataPktsTable*/
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_wlan_node->ethernetRecvCorrectFrames));
			
				//printf("sub_wlan_node->ethernetSendCorrectBytes = %d \n",sub_wlan_node->ethernetRecvCorrectFrames);
				
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_wlan_node->ethernetRecvCorrectBytes));
				
				//printf("sub_wlan_node->ethernetSendCorrectBytes = %llu \n",sub_wlan_node->ethernetRecvCorrectBytes);
				
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_wlan_node->ethernetSendCorrectBytes));
				
				//printf("sub_wlan_node->ethernetSendCorrectBytes = %llu \n",sub_wlan_node->ethernetSendCorrectBytes);
				/*fengwenchao add end*/
				 dbus_message_iter_next(&iter_sub_array);
			 }

			dbus_message_iter_next(&iter_array);
		}
	}

	else {
		dbus_message_unref(reply);
		return NULL;
	}
	dbus_message_unref(reply);
	if(wlan_essid){
		free(wlan_essid);
		wlan_essid = NULL;
	}
	/*Receive ASD information*/
	unsigned char asd_wlan_id,asd_wlan_num,asd_security_id;
	unsigned int auth_port,securityType,wtpEAPAuthenSupport;
	unsigned char *auth_ip = NULL;
	unsigned int wtpAcAppointDistOnlineUsrNum = 0;
	unsigned int sta_num_of_wlan = 0;
	unsigned int ret2 = 0;
	
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,\
			INTERFACE,ASD_DBUS_STA_METHOD_SHOW_STATS_INFO_BY_WLAN_OF_ALL_WTP);
		
	dbus_error_init(&err);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);	
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		dcli_free_wlan_stats_Info(WtpHead);
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret2);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(asd_wlan_num));
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_recurse(&iter,&iter_array);
			
	if((ret2 == 0)&&(asd_wlan_num !=0)){	
		for(i=0;i<asd_wlan_num;i++){
			DBusMessageIter iter_struct;
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(asd_wlan_id));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(asd_security_id));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(auth_port));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(auth_ip));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(securityType));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(wtpEAPAuthenSupport));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(wtpAcAppointDistOnlineUsrNum));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(sta_num_of_wlan));
			
			WtpSearchNode = WtpHead->WtpWlanStatsInfo_list;

			while(WtpSearchNode!= NULL){
				if(WtpSearchNode->WtpWlanStatsInfo_wlan_head == NULL)
					{	//break;   //fengwenchao comment 20110609如果前面AP未绑定WLAN ,导致WtpSearchNode不会偏移
					}

				else{
					if((WtpSearchNode->wtpBwlanNum !=0)&&(WtpSearchNode->WtpWlanStatsInfo_wlan_head !=NULL)){
						sub_wlan_search = WtpSearchNode->WtpWlanStatsInfo_wlan_head;
					
						while(sub_wlan_search !=NULL){
							if(sub_wlan_search->wlan_id == asd_wlan_id){
								sub_wlan_search->wtpConRadiusServerPort = auth_port;
								sub_wlan_search->wtpConfSecurMech = securityType;
								sub_wlan_search->wtpEAPAuthenSupport = wtpEAPAuthenSupport;
								sub_wlan_search->wtpSsidCurrOnlineUsrNum = sta_num_of_wlan;
								sub_wlan_search->wtpAcAppointDistOnlineUsrNum = wtpAcAppointDistOnlineUsrNum;
								sub_wlan_search->wlanSecID = asd_security_id;

								sub_wlan_search->wtpConRadiusServerIP = (unsigned char *)malloc(strlen(auth_ip) +1);
								memset(sub_wlan_search->wtpConRadiusServerIP,0,(strlen(auth_ip) +1) );
								memcpy(sub_wlan_search->wtpConRadiusServerIP,auth_ip,strlen(auth_ip));
							}
							sub_wlan_search = sub_wlan_search->next;
						}
					}
				}
				WtpSearchNode = WtpSearchNode->next;
			}
			dbus_message_iter_next(&iter_array);
		}
	}
	dbus_message_unref(reply);
	return WtpHead;
}

//mahz add 2011.11.9 for GuangZhou Mobile
struct WtpStationinfo* show_station_information_of_all_wtp(int index,int localid,DBusConnection *dcli_dbus_connection, unsigned int *wtp_num, unsigned int *ret)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter iter;
	DBusMessageIter iter_wtp_array;
	DBusMessageIter iter_wtp;
	DBusMessageIter iter_sta_array;
	DBusMessageIter iter_sta;
	
	struct WtpStationinfo * StaHead = NULL;
	struct WtpStationinfo * StaNode = NULL;
	struct WtpStationinfo * StaTail = NULL;

	int i=0,j=0;
	unsigned int bss_num=0;
	char * wtp_mac=NULL;
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,\
			INTERFACE,ASD_DBUS_STA_METHOD_SHOW_STA_STATIS_INFO_OF_ALL_WTP);

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);			
	dbus_message_unref(query);

	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		*ret=ASD_DBUS_ERROR;
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,wtp_num);	
	dbus_message_iter_next(&iter);
	dbus_message_iter_recurse(&iter,&iter_wtp_array);

	if((*ret == 0)&&(*wtp_num !=0)){
		for(i=0;i<*wtp_num;i++){
			if((StaNode=(struct WtpStationinfo*)malloc(sizeof(struct WtpStationinfo)))==NULL){
				dcli_free_wtp_sta_info(StaHead);
				*ret=MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			memset(StaNode,0,sizeof(struct WtpStationinfo));
			if(StaHead==NULL){
				StaHead=StaNode;
				StaTail=StaNode;
			}
			else{
				StaTail->next=StaNode;
				StaTail=StaNode;
			}
			dbus_message_iter_recurse(&iter_wtp_array,&iter_sta);
			dbus_message_iter_get_basic(&iter_sta,&StaNode->wtpid);
			dbus_message_iter_next(&iter_sta);
			dbus_message_iter_get_basic(&iter_sta,&wtp_mac);
			dbus_message_iter_next(&iter_sta);
			dbus_message_iter_get_basic(&iter_sta,&StaNode->no_auth_sta_num);
			dbus_message_iter_next(&iter_sta);
			dbus_message_iter_get_basic(&iter_sta,&StaNode->assoc_auth_sta_num);
			dbus_message_iter_next(&iter_sta);
			dbus_message_iter_get_basic(&iter_sta,&StaNode->no_auth_accessed_total_time);
			dbus_message_iter_next(&iter_sta);
			dbus_message_iter_get_basic(&iter_sta,&StaNode->assoc_auth_accessed_total_time);
			dbus_message_iter_next(&iter_sta);
			dbus_message_iter_get_basic(&iter_sta,&StaNode->no_auth_sta_abnormal_down_num);
			dbus_message_iter_next(&iter_sta);
			dbus_message_iter_get_basic(&iter_sta,&StaNode->assoc_auth_sta_abnormal_down_num);
			dbus_message_iter_next(&iter_sta);
			dbus_message_iter_get_basic(&iter_sta,&StaNode->assoc_auth_req_num);
			dbus_message_iter_next(&iter_sta);
			dbus_message_iter_get_basic(&iter_sta,&StaNode->assoc_auth_succ_num);
			dbus_message_iter_next(&iter_sta);
			dbus_message_iter_get_basic(&iter_sta,&StaNode->assoc_auth_fail_num);
			dbus_message_iter_next(&iter_sta);
			dbus_message_iter_get_basic(&iter_sta,&StaNode->assoc_auth_online_sta_num);	/* WEP/PSK assoc auth */		
			dbus_message_iter_next(&iter_sta);
			dbus_message_iter_get_basic(&iter_sta,&StaNode->auto_auth_online_sta_num);	/* SIM/PEAP */		
			dbus_message_iter_next(&iter_sta);
			dbus_message_iter_get_basic(&iter_sta,&StaNode->all_assoc_auth_sta_total_time);	/* WEP/PSK assoc auth */	
			dbus_message_iter_next(&iter_sta);
			dbus_message_iter_get_basic(&iter_sta,&StaNode->auto_auth_sta_total_time);
			dbus_message_iter_next(&iter_sta);
			dbus_message_iter_get_basic(&iter_sta,&StaNode->assoc_auth_sta_drop_cnt);/* WEP/PSK assoc auth */	
			dbus_message_iter_next(&iter_sta);
			dbus_message_iter_get_basic(&iter_sta,&StaNode->auto_auth_sta_drop_cnt);	/* EAP TYPE SIM/PEAP auth */	
			dbus_message_iter_next(&iter_sta);
			dbus_message_iter_get_basic(&iter_sta,&StaNode->auto_auth_req_cnt);
			dbus_message_iter_next(&iter_sta);
			dbus_message_iter_get_basic(&iter_sta,&StaNode->auto_auth_suc_cnt);
			dbus_message_iter_next(&iter_sta);
			dbus_message_iter_get_basic(&iter_sta,&StaNode->auto_auth_fail_cnt);
			dbus_message_iter_next(&iter_wtp_array);
			if((StaNode->wtpMacAddr=(char*)malloc(strlen(wtp_mac)+1))==NULL){
				*ret=MALLOC_ERROR;
				dcli_free_wtp_sta_info(StaHead);
				dbus_message_unref(reply);
				return NULL;
			}
			memset(StaNode->wtpMacAddr,0,(strlen(wtp_mac)+1));
			memcpy(StaNode->wtpMacAddr,wtp_mac,strlen(wtp_mac));
		}
	}

	dbus_message_unref(reply);
	return StaHead;	
}
struct WtpStationinfo* show_ac_station_information_of_all(int index,int localid,DBusConnection *dcli_dbus_connection, unsigned int *bss_num, unsigned int *ret)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter iter;
	
	int i=0,j=0;
	struct WtpStationinfo * StaNode = NULL;
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,\
			INTERFACE,ASD_DBUS_STA_METHOD_SHOW_AC_STA_INFO_OF_ALL);

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);			
	dbus_message_unref(query);

	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		*ret=ASD_DBUS_ERROR;
		return NULL;
	}
	
	if((StaNode=(struct WtpStationinfo*)malloc(sizeof(struct WtpStationinfo)))==NULL){
		dcli_free_wtp_sta_info(StaNode);
		*ret=MALLOC_ERROR;
		dbus_message_unref(reply);
		return NULL;
	}
	memset(StaNode,0,sizeof(struct WtpStationinfo));

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,bss_num);	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&StaNode->no_auth_sta_num);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&StaNode->assoc_auth_sta_num);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&StaNode->no_auth_accessed_total_time);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&StaNode->assoc_auth_accessed_total_time);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&StaNode->no_auth_sta_abnormal_down_num);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&StaNode->assoc_auth_sta_abnormal_down_num);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&StaNode->assoc_auth_req_num);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&StaNode->assoc_auth_succ_num);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&StaNode->assoc_auth_fail_num);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&StaNode->auto_auth_resp_cnt);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &StaNode->weppsk_assoc_req_cnt);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &StaNode->weppsk_assoc_succ_cnt);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter, &StaNode->weppsk_assoc_fail_cnt);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&StaNode->assoc_auth_online_sta_num); /* WEP/PSK assoc auth */		
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&StaNode->auto_auth_online_sta_num);	/* SIM/PEAP */		
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&StaNode->assoc_auth_sta_drop_cnt);/* WEP/PSK assoc auth */	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&StaNode->auto_auth_sta_drop_cnt);	/* EAP TYPE SIM/PEAP auth */	

	dbus_message_unref(reply);
	return StaNode;	
}

#if 0 //for old version
/*nl add 20100511 for showting ssidstats infor table 9*/
struct SSIDStatsInfo* show_wtp_wlan_SSIDStatsInfo_of_all_wtp(int index,DBusConnection *dcli_dbus_connection, unsigned int *num, unsigned int *ret)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter  iter_array;	
	
	struct SSIDStatsInfo  *WtpNode = NULL;
	struct SSIDStatsInfo  *WtpHead = NULL;
	struct SSIDStatsInfo  *WtpSearchNode = NULL;
	struct SSIDStatsInfo_sub_wlan *sub_wlan_node = NULL;
	
	int i=0, j=0;
	char * wtpip = NULL;
	unsigned int rx_rate = 0;
	unsigned int tx_rate = 0;

	unsigned long long rx_unicast;
	unsigned long long tx_unicast;
	unsigned long long rx_broadcast;
	unsigned long long tx_broadcast;
	unsigned long long retry;
 	
	unsigned char wlanid[WLAN_NUM] = {0}; //should make 

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,\
						INTERFACE,WID_DBUS_CONF_METHOD_SHOW_ALL_WLAN_SSID_STATS_INFORMATION);
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		*ret = WID_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	if(*ret == 0){
		if((WtpHead = (struct SSIDStatsInfo*)malloc(sizeof(struct SSIDStatsInfo))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
		memset(WtpHead,0,sizeof(struct SSIDStatsInfo));
		WtpHead->SSIDStatsInfo_list = NULL;
		WtpHead->SSIDStatsInfo_last = NULL;
		WtpHead->SSIDStatsInfo_sub_wlan_head = NULL;
			
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,num);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
			
		for (i = 0; i < *num; i++) {
			DBusMessageIter iter_struct;
 			DBusMessageIter iter_sub_array; 	
			if((WtpNode = (struct SSIDStatsInfo*)malloc(sizeof(struct SSIDStatsInfo))) == NULL){
					dcli_free_ssid_stats_Info(WtpHead);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(WtpNode,0,sizeof(struct SSIDStatsInfo));
			WtpNode->next = NULL;
			WtpNode->SSIDStatsInfo_sub_wlan_head = NULL;

			WtpNode->wtpMacAddr = (unsigned char *)malloc(MAC_LEN +1);
			memset(WtpNode->wtpMacAddr,0,(MAC_LEN +1));

			if(WtpHead->SSIDStatsInfo_list == NULL){
				WtpHead->SSIDStatsInfo_list = WtpNode;
				WtpHead->next = WtpNode;
			}
			else{
				WtpHead->SSIDStatsInfo_last->next = WtpNode;
			}
			WtpHead->SSIDStatsInfo_last = WtpNode;
			
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpCurrID));
		
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[0]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[1]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[2]));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[3]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[4]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[5]));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->SSIDTxSignalPkts));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->SSIDRxSignalPkts));

			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->SSIDRxDataPkts));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->SSIDTxDataPkts));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->SSIDUplinkDataOctets));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->SSIDDwlinkDataOctets));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&rx_rate);
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&tx_rate);

			WtpNode->SSIDApChStatsFrameFragRate = rx_rate + tx_rate ;

 			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpBwlanNum));
			
 			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_recurse(&iter_struct,&iter_sub_array);

			for(j=0; j<WtpNode->wtpBwlanNum; j++){  
				 DBusMessageIter iter_sub_struct;
				 char *Essid = NULL;
				 unsigned char *WlanName = NULL;
				 if((sub_wlan_node = (struct SSIDStatsInfo_sub_wlan*)malloc(sizeof(struct SSIDStatsInfo_sub_wlan))) == NULL){
					free(sub_wlan_node);
					sub_wlan_node =NULL;
					dcli_free_ssid_stats_Info(WtpHead);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
				}
				memset(sub_wlan_node,0,sizeof(struct SSIDStatsInfo_sub_wlan));
				sub_wlan_node->next = NULL;
				sub_wlan_node->SSIDStatsInfo_sub_wlan_list = NULL;
				sub_wlan_node->SSIDStatsInfo_sub_wlan_last = NULL;


				if(WtpNode->SSIDStatsInfo_sub_wlan_head == NULL){
					WtpNode->SSIDStatsInfo_sub_wlan_head = sub_wlan_node;
				}else{
					WtpNode->SSIDStatsInfo_sub_wlan_head->SSIDStatsInfo_sub_wlan_last->next = sub_wlan_node;
				}
				
				WtpNode->SSIDStatsInfo_sub_wlan_head->SSIDStatsInfo_sub_wlan_last = sub_wlan_node;

				sub_wlan_node->wtpSSIDName = (unsigned char *)malloc(MAC_LEN +1);
				memset(sub_wlan_node->wtpSSIDName,0,(MAC_LEN +1));

				 dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
				 dbus_message_iter_get_basic(&iter_sub_struct,&(wlanid[j])); 
				
				 dbus_message_iter_next(&iter_sub_struct);	
				 dbus_message_iter_get_basic(&iter_sub_struct,&rx_unicast);

				 dbus_message_iter_next(&iter_sub_struct);	
				 dbus_message_iter_get_basic(&iter_sub_struct,&tx_unicast);

				 dbus_message_iter_next(&iter_sub_struct);	
				 dbus_message_iter_get_basic(&iter_sub_struct,&rx_broadcast);

				 dbus_message_iter_next(&iter_sub_struct);	
				 dbus_message_iter_get_basic(&iter_sub_struct,&tx_broadcast);

				 dbus_message_iter_next(&iter_sub_struct);	
				 dbus_message_iter_get_basic(&iter_sub_struct,&retry);
				 
				 dbus_message_iter_next(&iter_sub_struct);	 
				 dbus_message_iter_get_basic(&iter_sub_struct,&(sub_wlan_node->wtpSSIDName[0])); //unsigned char
				 dbus_message_iter_next(&iter_sub_struct);	 
				 dbus_message_iter_get_basic(&iter_sub_struct,&(sub_wlan_node->wtpSSIDName[1])); //unsigned char
				 dbus_message_iter_next(&iter_sub_struct);	 
				 dbus_message_iter_get_basic(&iter_sub_struct,&(sub_wlan_node->wtpSSIDName[2])); //unsigned char
				 
				 dbus_message_iter_next(&iter_sub_struct);	 
				 dbus_message_iter_get_basic(&iter_sub_struct,&(sub_wlan_node->wtpSSIDName[3])); //unsigned char
				 dbus_message_iter_next(&iter_sub_struct);	 
				 dbus_message_iter_get_basic(&iter_sub_struct,&(sub_wlan_node->wtpSSIDName[4])); //unsigned char
				 dbus_message_iter_next(&iter_sub_struct);	 
				 dbus_message_iter_get_basic(&iter_sub_struct,&(sub_wlan_node->wtpSSIDName[5])); //unsigned char
				 
				 dbus_message_iter_next(&iter_sub_struct);	
				 dbus_message_iter_get_basic(&iter_sub_struct,&(sub_wlan_node->wtpSSIDState)); //unsigned char
 
				 dbus_message_iter_next(&iter_sub_struct);	
				 dbus_message_iter_get_basic(&iter_sub_struct,&sub_wlan_node->wtpSSIDMaxLoginUsr);//unsigned int
 
				 dbus_message_iter_next(&iter_sub_struct);	
				 dbus_message_iter_get_basic(&iter_sub_struct,&sub_wlan_node->wtpSSIDLoadBalance);//unsigned char
 
				 dbus_message_iter_next(&iter_sub_struct);	
				 dbus_message_iter_get_basic(&iter_sub_struct,&sub_wlan_node->wtpSSIDSecurityPolicyID);//unsigned char
				
				 dbus_message_iter_next(&iter_sub_struct);	
				 dbus_message_iter_get_basic(&iter_sub_struct,&Essid);//string

				 dbus_message_iter_next(&iter_sub_struct);	
				 dbus_message_iter_get_basic(&iter_sub_struct,&WlanName);//string

				 WtpNode->wtpSupportSSID[j] = wlanid[j];
				 sub_wlan_node->SSIDChStatsDwlinkTotRetryPkts = retry;
				 sub_wlan_node->SSIDChStatsUplinkUniFrameCnt = rx_unicast;
				 sub_wlan_node->SSIDChStatsDwlinkUniFrameCnt = tx_unicast;
				 sub_wlan_node->SSIDUpChStatsFrameNonUniFrameCnt = rx_broadcast;
				 sub_wlan_node->SSIDDownChStatsFrameNonUniFrameCnt = tx_broadcast;
				 sub_wlan_node->SSIDUplinkTotFrameCnt = rx_unicast+ rx_broadcast;
				 sub_wlan_node->SSIDDwlinkTotFrameCnt = tx_unicast + tx_broadcast;
				 sub_wlan_node->wlanCurrID = wlanid[j] ;

				 sub_wlan_node->WlanName = (unsigned char*)malloc(strlen(WlanName)+1);
				 memset(sub_wlan_node->WlanName, 0, strlen(WlanName)+1);
				 memcpy(sub_wlan_node->WlanName, WlanName, strlen(WlanName));	

	 			 sub_wlan_node->wtpSSIDESSID = (char*)malloc(strlen(Essid)+1);
				 memset(sub_wlan_node->wtpSSIDESSID, 0, strlen(Essid)+1);
				 memcpy(sub_wlan_node->wtpSSIDESSID, Essid, strlen(Essid));	

				 dbus_message_iter_next(&iter_sub_array);
			 }
  
			dbus_message_iter_next(&iter_array);
		}
		
	}

	dbus_message_unref(reply);
	
	/*Receive ASD information*/
	DBusMessage *query2, *reply2;
	DBusError err2;
	DBusMessageIter	 iter2;
	DBusMessageIter  iter_array2;
	unsigned int ret2;

	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);

	query2 = dbus_message_new_method_call(BUSNAME,OBJPATH,\
			INTERFACE,ASD_DBUS_STA_METHOD_SHOW_SSID_STATS_INFO_BY_WLAN_OF_ALL_WTP);
		
	dbus_error_init(&err2);
	
	reply2 = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query2,-1, &err2);	
			
	dbus_message_unref(query2);
	if (NULL == reply2) {
		if (dbus_error_is_set(&err2)) {
			dbus_error_free(&err2);
		}
		return NULL;
	}

	int asd_wtp_num;		//u32
	unsigned int asd_wtpCurrID;		//32
	unsigned int asd_wtpBssNum;		//32
	unsigned char asd_bss_index;

	dbus_message_iter_init(reply2,&iter2);
	dbus_message_iter_get_basic(&iter2,&ret2);

	dbus_message_iter_next(&iter2);	
	dbus_message_iter_get_basic(&iter2,&asd_wtp_num);

	dbus_message_iter_next(&iter2);	
	dbus_message_iter_recurse(&iter2,&iter_array2);

	if((ret2==0)&&(asd_wtp_num!=0)){
		for(i=0;i<asd_wtp_num;i++){
			DBusMessageIter iter_struct;
			DBusMessageIter iter_sub_array; 	
			DBusMessageIter iter_sub_struct;

			dbus_message_iter_recurse(&iter_array2,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(asd_wtpCurrID));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(asd_wtpBssNum));

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_recurse(&iter_struct,&iter_sub_array);
			
			unsigned char asd_bss_index;
			unsigned char asd_bss_wlan_id;
			unsigned int auth_tms;
			unsigned int repauth_tms;
			unsigned int assoc_req;
			unsigned int assoc_resp;
			unsigned int rx_ctrl_pkts;
			unsigned int tx_ctrl_pkts;
			unsigned int rx_data_pkts;
			unsigned int tx_data_pkts;
			unsigned int SSIDApChStatsNumStations;
			
		 	for(j=0; j<asd_wtpBssNum; j++){  
				dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(asd_bss_index)); 

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&asd_bss_wlan_id);

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&auth_tms);

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&repauth_tms);

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&assoc_req);

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&assoc_resp);

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&rx_ctrl_pkts);

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&tx_ctrl_pkts);

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&rx_data_pkts);

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&tx_data_pkts);

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&SSIDApChStatsNumStations);

				WtpSearchNode = WtpHead->SSIDStatsInfo_list;

				while(WtpSearchNode!= NULL){
					if(WtpSearchNode->SSIDStatsInfo_sub_wlan_head == NULL){
						break;
					}

					else{
						if((WtpSearchNode->wtpBwlanNum!=0)&&(asd_wtpCurrID==WtpSearchNode->wtpCurrID)){
							struct SSIDStatsInfo_sub_wlan *sub_wlan_search =NULL;
							sub_wlan_search = WtpSearchNode->SSIDStatsInfo_sub_wlan_head;
						
							while(sub_wlan_search !=NULL){
								if(sub_wlan_search->wlanCurrID == asd_bss_wlan_id){
									sub_wlan_search->SSIDRxCtrlFrameCnt = rx_ctrl_pkts;
									sub_wlan_search->SSIDRxDataFrameCnt = rx_data_pkts;
									sub_wlan_search->SSIDRxAuthenFrameCnt = auth_tms;
									
									sub_wlan_search->SSIDRxAssociateFrameCnt = assoc_req;
									sub_wlan_search->SSIDTxCtrlFrameCnt = tx_ctrl_pkts;
									sub_wlan_search->SSIDTxDataFrameCnt = tx_data_pkts;
									
									sub_wlan_search->SSIDTxAuthenFrameCnt = repauth_tms;
									sub_wlan_search->SSIDTxAssociateFrameCnt = assoc_resp;
									sub_wlan_search->SSIDTxAssociateFrameCnt = SSIDApChStatsNumStations;
								}
								sub_wlan_search = sub_wlan_search->next;
							}
						}
					}
					WtpSearchNode = WtpSearchNode->next;
				}
			 	dbus_message_iter_next(&iter_sub_array);
			}
			dbus_message_iter_next(&iter_array2);
		}
	}
	
	dbus_message_unref(reply2);

	return WtpHead;
}
#endif
/* zhangshu copy from 1.2 ,2010-09-13 */
/*nl add 20100511 for showting ssidstats infor table 9*/
struct SSIDStatsInfo* show_wtp_wlan_SSIDStatsInfo_of_all_wtp(int index,int localid,DBusConnection *dcli_dbus_connection, unsigned int *num, unsigned int *ret)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter  iter_array;	
	
	struct SSIDStatsInfo  *WtpNode = NULL;
	struct SSIDStatsInfo  *WtpHead = NULL;
	struct SSIDStatsInfo  *WtpSearchNode = NULL;
    struct SSIDStatsInfo_Radioid_info *Radioid_info_node = NULL;
	struct SSIDStatsInfo_sub_wlan *sub_wlan_node = NULL;
	
	int i;//wtpNum
	int j;//RadioNum
	int k;//RadioBindToWlanNum
	int l;//ssid length
	char * wtpip = NULL;
	unsigned int rx_rate = 0;
	unsigned int tx_rate = 0;

	unsigned long long rx_unicast = 0;
	unsigned long long tx_unicast = 0;
	unsigned long long rx_broadcast = 0;
	unsigned long long tx_broadcast = 0;
	unsigned long long retry = 0;
 	unsigned int Radioid[L_RADIO_NUM] = {0};//zhaoruijia,添加Radioid信息
	unsigned char wlanid[WLAN_NUM] = {0}; //should make 

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	//printf("show_wtp_wlan_SSIDStatsInfo_of_all_wtp_start_acd\n");

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,\
						INTERFACE,WID_DBUS_CONF_METHOD_SHOW_ALL_WLAN_SSID_STATS_INFORMATION);
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		*ret = WID_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
            //printf("))))))))))))))))\n");
		}
		return NULL;
	}
    //printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,num);
	
	if(*ret == 0){
		if((WtpHead = (struct SSIDStatsInfo*)malloc(sizeof(struct SSIDStatsInfo))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
		memset(WtpHead,0,sizeof(struct SSIDStatsInfo));
		WtpHead->SSIDStatsInfo_list = NULL;
		WtpHead->SSIDStatsInfo_last = NULL;
		WtpHead->SSIDStatsInfo_Radioid_info_head = NULL;
			
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
			
		for (i = 0; i < *num; i++) {
			DBusMessageIter iter_struct;
 			DBusMessageIter iter_sub_array;
			DBusMessageIter iter_sub_struct;
		    DBusMessageIter iter_sub_sub_array;
			DBusMessageIter iter_sub_sub_struct;
			DBusMessageIter iter_sub_essid_struct;
			DBusMessageIter iter_sub_essid_array;
			if((WtpNode = (struct SSIDStatsInfo*)malloc(sizeof(struct SSIDStatsInfo))) == NULL){
					dcli_free_ssid_stats_Info(WtpHead);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(WtpNode,0,sizeof(struct SSIDStatsInfo));
			WtpNode->next = NULL;
			WtpNode->SSIDStatsInfo_Radioid_info_head = NULL;

			WtpNode->wtpMacAddr = (unsigned char *)malloc(MAC_LEN +1);
			memset(WtpNode->wtpMacAddr,0,(MAC_LEN +1));

			if(WtpHead->SSIDStatsInfo_list == NULL){
				WtpHead->SSIDStatsInfo_list = WtpNode;
				WtpHead->next = WtpNode;
			}
			else{
				WtpHead->SSIDStatsInfo_last->next = WtpNode;
			}
			WtpHead->SSIDStatsInfo_last = WtpNode;
			
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpCurrID));
		    
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[0]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[1]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[2]));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[3]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[4]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[5]));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->SSIDTxSignalPkts));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->SSIDRxSignalPkts));

			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->SSIDRxDataPkts));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->SSIDTxDataPkts));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->SSIDUplinkDataOctets));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->SSIDDwlinkDataOctets));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&rx_rate);
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&tx_rate);

			WtpNode->SSIDApChStatsFrameFragRate = rx_rate + tx_rate ;

 			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpBwlanRadioNum));
		    //printf("#################wid_wtp_WtpNode->wtpBwlanRadioNum=%d############\n",WtpNode->wtpBwlanRadioNum);
			
 			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_recurse(&iter_struct,&iter_sub_array);
			for(j=0;j<WtpNode->wtpBwlanRadioNum;j++)
			{
                  //建立Radioin_node 链表
                 
				   if((Radioid_info_node = (struct SSIDStatsInfo_Radioid_info*)malloc(sizeof(struct SSIDStatsInfo_Radioid_info))) == NULL){
					dcli_free_ssid_stats_Info(WtpHead);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
				 }
				   memset(Radioid_info_node,0,sizeof(struct SSIDStatsInfo_Radioid_info));
				   Radioid_info_node->next = NULL;
				   Radioid_info_node->SSIDStatsInfo_Radioid_info_list = NULL;
				   Radioid_info_node->SSIDStatsInfo_Radioid_info_last = NULL;
				   Radioid_info_node->SSIDStatsInfo_sub_wlan_head =NULL;
                  if(WtpNode->SSIDStatsInfo_Radioid_info_head == NULL){
					   WtpNode->SSIDStatsInfo_Radioid_info_head = Radioid_info_node;
				    }
				    else{
					WtpNode->SSIDStatsInfo_Radioid_info_head->SSIDStatsInfo_Radioid_info_last->next = Radioid_info_node;
				   }
				
				 WtpNode->SSIDStatsInfo_Radioid_info_head->SSIDStatsInfo_Radioid_info_last = Radioid_info_node;
				 dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
				 dbus_message_iter_get_basic(&iter_sub_struct,&(Radioid[j])); 
				 //printf("#################wid_wtp_Radioid=%d############\n",Radioid[j]);
				 dbus_message_iter_next(&iter_sub_struct);	
				 dbus_message_iter_get_basic(&iter_sub_struct,&(Radioid_info_node->radioBwlanNum)); 
				 //printf("#################wid_wtp_Radioid_info_node->radioBwlanNum=%d############\n",Radioid_info_node->radioBwlanNum);

				 dbus_message_iter_next(&iter_sub_struct);	                                  
				 dbus_message_iter_get_basic(&iter_sub_struct,&(Radioid_info_node->Mixed_Green_Field.Mixed_Greenfield));      //fengwenchao add 20110331


				 Radioid_info_node->radioId = Radioid[j];
				 WtpNode->wtpSupportRadioId[j] = Radioid[j];

				 dbus_message_iter_next(&iter_sub_struct);
			     dbus_message_iter_recurse(&iter_sub_struct,&iter_sub_sub_array);
				
                     //建立Radio 绑定的wlan 链表
                    for(k=0;k<Radioid_info_node->radioBwlanNum;k++ )
                    	{
                           if((sub_wlan_node = (struct SSIDStatsInfo_sub_wlan*)malloc(sizeof(struct SSIDStatsInfo_sub_wlan))) == NULL){

								dcli_free_ssid_stats_Info(WtpHead);
					           *ret = MALLOC_ERROR;
					            dbus_message_unref(reply);
					            return NULL;
				               }
								memset(sub_wlan_node,0,sizeof(struct SSIDStatsInfo_sub_wlan));
                             
								sub_wlan_node->next = NULL;
								sub_wlan_node->SSIDStatsInfo_sub_wlan_list = NULL;
								sub_wlan_node->SSIDStatsInfo_sub_wlan_last = NULL;


								if(Radioid_info_node->SSIDStatsInfo_sub_wlan_head == NULL){
									Radioid_info_node->SSIDStatsInfo_sub_wlan_head = sub_wlan_node;
								}else{
									Radioid_info_node->SSIDStatsInfo_sub_wlan_head->SSIDStatsInfo_sub_wlan_last->next = sub_wlan_node;
								}
								
								 Radioid_info_node->SSIDStatsInfo_sub_wlan_head->SSIDStatsInfo_sub_wlan_last = sub_wlan_node;
						         dbus_message_iter_recurse(&iter_sub_sub_array,&iter_sub_sub_struct);
                                 dbus_message_iter_get_basic(&iter_sub_sub_struct,&(wlanid[k])); 
								 //printf("#################wtp_wid_wlanid[k]=%d############\n",wlanid[k]);

								 dbus_message_iter_next(&iter_sub_sub_struct);	
								 dbus_message_iter_get_basic(&iter_sub_sub_struct,&rx_unicast);
								 //printf("#################wtp_wid_rx_unicast=%llu############\n",rx_unicast);

								 dbus_message_iter_next(&iter_sub_sub_struct);	
								 dbus_message_iter_get_basic(&iter_sub_sub_struct,&tx_unicast);
								 //printf("#################wtp_wid_tx_unicast=%llu############\n",tx_unicast);

								 dbus_message_iter_next(&iter_sub_sub_struct);	
								 dbus_message_iter_get_basic(&iter_sub_sub_struct,&rx_broadcast);
								 //printf("#################wtp_wid_rx_broadcast=%llu############\n",rx_broadcast);

								 dbus_message_iter_next(&iter_sub_sub_struct);	
								 dbus_message_iter_get_basic(&iter_sub_sub_struct,&tx_broadcast);
								 //printf("#################wtp_wid_tx_broadcast=%llu############\n",tx_broadcast);

								 dbus_message_iter_next(&iter_sub_sub_struct);	
								 dbus_message_iter_get_basic(&iter_sub_sub_struct,&retry);
								 //printf("#################wtp_wid_retry=%llu############\n",retry);

								 sub_wlan_node->SSIDChStatsDwlinkTotRetryPkts = retry;
								 sub_wlan_node->SSIDChStatsUplinkUniFrameCnt = rx_unicast;
								 sub_wlan_node->SSIDChStatsDwlinkUniFrameCnt = tx_unicast;
								 sub_wlan_node->SSIDUpChStatsFrameNonUniFrameCnt = rx_broadcast;
								 sub_wlan_node->SSIDDownChStatsFrameNonUniFrameCnt = tx_broadcast;
								 sub_wlan_node->SSIDUplinkTotFrameCnt = rx_unicast+ rx_broadcast;
								 sub_wlan_node->SSIDDwlinkTotFrameCnt = tx_unicast + tx_broadcast;

								 sub_wlan_node->wlanCurrID = wlanid[k] ;
								 Radioid_info_node->RadioidSupportSSID[k] = wlanid[k];

								 sub_wlan_node->wtpSSIDName = (unsigned char *)malloc(MAC_LEN +1);
								 memset(sub_wlan_node->wtpSSIDName,0,(MAC_LEN +1));

								 dbus_message_iter_next(&iter_sub_sub_struct);	 
								 dbus_message_iter_get_basic(&iter_sub_sub_struct,&(sub_wlan_node->wtpSSIDName[0])); //unsigned char
								
								 dbus_message_iter_next(&iter_sub_sub_struct);	 
								 dbus_message_iter_get_basic(&iter_sub_sub_struct,&(sub_wlan_node->wtpSSIDName[1])); //unsigned char
								

								 dbus_message_iter_next(&iter_sub_sub_struct);	 
								 dbus_message_iter_get_basic(&iter_sub_sub_struct,&(sub_wlan_node->wtpSSIDName[2])); //unsigned char
								
								 dbus_message_iter_next(&iter_sub_sub_struct);	 
								 dbus_message_iter_get_basic(&iter_sub_sub_struct,&(sub_wlan_node->wtpSSIDName[3])); //unsigned char
                                 
								 dbus_message_iter_next(&iter_sub_sub_struct);	 
								 dbus_message_iter_get_basic(&iter_sub_sub_struct,&(sub_wlan_node->wtpSSIDName[4])); //unsigned char
                                
								 dbus_message_iter_next(&iter_sub_sub_struct);	 
								 dbus_message_iter_get_basic(&iter_sub_sub_struct,&(sub_wlan_node->wtpSSIDName[5])); //unsigned char
								 
								 dbus_message_iter_next(&iter_sub_sub_struct);	
								 dbus_message_iter_get_basic(&iter_sub_sub_struct,&(sub_wlan_node->wtpSSIDState)); //unsigned char
				                 //printf("######ssub_wlan_node->wtpSSIDState = %d\n",sub_wlan_node->wtpSSIDState);
								 dbus_message_iter_next(&iter_sub_sub_struct);	
								 dbus_message_iter_get_basic(&iter_sub_sub_struct,&sub_wlan_node->wtpSSIDMaxLoginUsr);//unsigned int
				                //printf("######sub_wlan_node->wtpSSIDMaxLoginUsr = %d\n",sub_wlan_node->wtpSSIDMaxLoginUsr);
								 dbus_message_iter_next(&iter_sub_sub_struct);	
								 dbus_message_iter_get_basic(&iter_sub_sub_struct,&sub_wlan_node->wtpSSIDLoadBalance);//unsigned char
				                //printf("######sub_wlan_node->wtpSSIDLoadBalance = %d\n",sub_wlan_node->wtpSSIDLoadBalance);
								 dbus_message_iter_next(&iter_sub_sub_struct);	
								 dbus_message_iter_get_basic(&iter_sub_sub_struct,&sub_wlan_node->wtpSSIDSecurityPolicyID);//unsigned char
                                 //printf("######sub_wlan_node->wtpSSIDSecurityPolicyID = %d\n",sub_wlan_node->wtpSSIDSecurityPolicyID);
								char* essid = NULL;
								essid =(char*) malloc(ESSID_DEFAULT_LEN);
								memset(essid,0,ESSID_DEFAULT_LEN);
								  dbus_message_iter_next(&iter_sub_sub_struct);
								  dbus_message_iter_recurse(&iter_sub_sub_struct,&iter_sub_essid_array);
								  l= 0;
								  for(l=0;l<ESSID_DEFAULT_LEN;l++){
									  dbus_message_iter_recurse(&iter_sub_essid_array,&iter_sub_essid_struct);
									  dbus_message_iter_get_basic(&iter_sub_essid_struct,&essid[l]);
									   dbus_message_iter_next(&iter_sub_essid_struct);
									  dbus_message_iter_next(&iter_sub_essid_array);
								  }

					 			 sub_wlan_node->wtpSSIDESSID = (char*)malloc(ESSID_DEFAULT_LEN+1);
								 memset(sub_wlan_node->wtpSSIDESSID, 0, ESSID_DEFAULT_LEN+1);
								 memcpy(sub_wlan_node->wtpSSIDESSID, essid, ESSID_DEFAULT_LEN);	
                                //printf("#################wtp_wid_sub_wlan_node->wtpSSIDESSID=%s############\n",sub_wlan_node->wtpSSIDESSID);
								if(essid){
									free(essid);
									essid=NULL;
								}
								dbus_message_iter_next(&iter_sub_sub_struct);
								dbus_message_iter_get_basic(&iter_sub_sub_struct,&sub_wlan_node->vlanid);//unsigned int
								dbus_message_iter_next(&iter_sub_sub_struct);
								dbus_message_iter_get_basic(&iter_sub_sub_struct,&sub_wlan_node->SSIDRxDataFrameCnt);//fengwenchao add 20110617
								printf("  SSIDRxDataFrameCnt   =  %d \n",sub_wlan_node->SSIDRxDataFrameCnt);
								dbus_message_iter_next(&iter_sub_sub_struct);
								dbus_message_iter_get_basic(&iter_sub_sub_struct,&sub_wlan_node->SSIDTxDataFrameCnt);//fengwenchao add 20110617
								printf("  SSIDTxDataFrameCnt   =  %d \n",sub_wlan_node->SSIDTxDataFrameCnt);
                                //fengwenchao add 20110127
								dbus_message_iter_next(&iter_sub_sub_struct);
								dbus_message_iter_get_basic(&iter_sub_sub_struct,&sub_wlan_node->SSIDDwErrPkts);	

								dbus_message_iter_next(&iter_sub_sub_struct);
								dbus_message_iter_get_basic(&iter_sub_sub_struct,&sub_wlan_node->SSIDDwDropPkts);	

								dbus_message_iter_next(&iter_sub_sub_struct);
								dbus_message_iter_get_basic(&iter_sub_sub_struct,&sub_wlan_node->SSIDDwTotErrFrames);	

								dbus_message_iter_next(&iter_sub_sub_struct);
								dbus_message_iter_get_basic(&iter_sub_sub_struct,&sub_wlan_node->SSIDUpErrPkts);	

								dbus_message_iter_next(&iter_sub_sub_struct);
								dbus_message_iter_get_basic(&iter_sub_sub_struct,&sub_wlan_node->SSIDUpDropPkts);	

								dbus_message_iter_next(&iter_sub_sub_struct);
								dbus_message_iter_get_basic(&iter_sub_sub_struct,&sub_wlan_node->SSIDUpTotErrFrames);	

								dbus_message_iter_next(&iter_sub_sub_struct);
								dbus_message_iter_get_basic(&iter_sub_sub_struct,&sub_wlan_node->WlanradioRecvSpeed);

								dbus_message_iter_next(&iter_sub_sub_struct);
								dbus_message_iter_get_basic(&iter_sub_sub_struct,&sub_wlan_node->WlanradioSendSpeed);
                                //fengwenchao add end	

								/*fengwenchao add 20110331*/
								dbus_message_iter_next(&iter_sub_sub_struct);
								dbus_message_iter_get_basic(&iter_sub_sub_struct,&sub_wlan_node->traffic_limit_able);

								dbus_message_iter_next(&iter_sub_sub_struct);
								dbus_message_iter_get_basic(&iter_sub_sub_struct,&sub_wlan_node->send_traffic_limit);
								/*fengwenchao add end*/
								
						   //printf("jiafsdfoasdfijaosfdajfdaodfjoa\n");
                         dbus_message_iter_next(&iter_sub_sub_array);
						  //printf("mmmmmmmmmmmmmmmmmmmmmmmm\n");
					    }
              dbus_message_iter_next(&iter_sub_array);
			  //printf("nnnnnnnnnnnnnnnnnnnn\n");
			 }

		  dbus_message_iter_next(&iter_array);
		  //printf("oooooooooooooooooooooooooooooo\n");
		}
		
	}

	else {
		dbus_message_unref(reply);
		return NULL;
	}
    
	dbus_message_unref(reply);
	//printf("ASD\n");
	/*Receive ASD information*/
	DBusMessage *query2, *reply2;
	DBusError err2;
	DBusMessageIter	 iter2;
	DBusMessageIter  iter_array2;
	unsigned int ret2;

	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
  
	query2 = dbus_message_new_method_call(BUSNAME,OBJPATH,\
			INTERFACE,ASD_DBUS_STA_METHOD_SHOW_SSID_STATS_INFO_BY_WLAN_OF_ALL_WTP);
		
	dbus_error_init(&err2);
	
	reply2 = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query2,-1, &err2);	
			
	dbus_message_unref(query2);
	if (NULL == reply2) {
		if (dbus_error_is_set(&err2)) {
			dbus_error_free(&err2);
		}
		dcli_free_ssid_stats_Info(WtpHead);
		return NULL;
	}

	int asd_wtp_num;		//u32
	unsigned int asd_wtpCurrID;		//32
    unsigned int asd_wtpRadioNum;//zhaoruijia
	unsigned int asd_wtpBssNum;		//32
	unsigned int asd_wtpRadioId;//zhaoruijia
	unsigned char asd_bss_index;    
	unsigned int  asd_wtpRadioBindToWlanNum;

	dbus_message_iter_init(reply2,&iter2);
	dbus_message_iter_get_basic(&iter2,&ret2);

	dbus_message_iter_next(&iter2);	
	dbus_message_iter_get_basic(&iter2,&asd_wtp_num);
	
    //printf("#################wid_wtp_asd_wtp_num=%d############\n",asd_wtp_num);
	

	if((ret2==0)&&(asd_wtp_num!=0)){
	    dbus_message_iter_next(&iter2);	
	    dbus_message_iter_recurse(&iter2,&iter_array2);
		for(i=0;i<asd_wtp_num;i++){
			DBusMessageIter iter_struct;
 			DBusMessageIter iter_sub_array;
			DBusMessageIter iter_sub_struct;
		    DBusMessageIter iter_sub_sub_array;
			DBusMessageIter iter_sub_sub_struct;

			dbus_message_iter_recurse(&iter_array2,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(asd_wtpCurrID));
			//printf("#################wid_wtp_aasd_wtpCurrID=%u############\n",asd_wtpCurrID);

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(asd_wtpRadioNum));
            //printf("#################wid_wtp_asd_wtpRadioNum=%u############\n",asd_wtpRadioNum);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_recurse(&iter_struct,&iter_sub_array);
			unsigned int radioid;
			for(j=0;j<asd_wtpRadioNum;j++){
             dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
			 dbus_message_iter_get_basic(&iter_sub_struct,&(asd_wtpRadioId));
			 //printf("#################wid_wtp_asd_wtpRadioId=%u############\n",asd_wtpRadioId);

			 dbus_message_iter_next(&iter_sub_struct);	
			 dbus_message_iter_get_basic(&iter_sub_struct,&(asd_wtpRadioBindToWlanNum));
			 //printf("#################wid_wtp_asd_wtpRadioBindToWlanNum=%u############\n",asd_wtpRadioBindToWlanNum);
			 dbus_message_iter_next(&iter_sub_struct);
			 dbus_message_iter_recurse(&iter_sub_struct,&iter_sub_sub_array);
             unsigned char asd_bss_index;
			 unsigned char asd_bss_wlan_id;
			 unsigned int acc_tms;				//xiaodawei add access times for telecom test, 20110301
			 unsigned int auth_tms;
			 unsigned int repauth_tms;
			 unsigned int assoc_req;
			 unsigned int assoc_resp;
			 unsigned int rx_ctrl_pkts;
			 unsigned int tx_ctrl_pkts;
			 unsigned int rx_data_pkts;
			 unsigned int tx_data_pkts;
			 unsigned int SSIDApChStatsNumStations;
             //fengwenchao add 20110127
			 unsigned long long  wl_up_flow; 
    		 unsigned long long wl_dw_flow; 
		     unsigned long long ch_dw_pck;		
		     unsigned long long ch_dw_los_pck;   
		     unsigned long long ch_dw_mac_err_pck;  
		     unsigned long long ch_dw_resend_pck;  
		     unsigned long long ch_up_frm;		 
		     unsigned long long ch_dw_frm;    
		     unsigned long long ch_dw_err_frm;	  
		     unsigned long long ch_dw_los_frm;  
		     unsigned long long ch_dw_resend_frm; 
		     unsigned long long ch_up_los_frm;        //fengwenchao modify 20110224
		     unsigned long long ch_up_resend_frm;      //fengwenchao modify 20110224
             //fengwenchao add end
			 for(k=0;k<asd_wtpRadioBindToWlanNum;k++){
                  dbus_message_iter_recurse(&iter_sub_sub_array,&iter_sub_sub_struct);
                  dbus_message_iter_get_basic(&iter_sub_sub_struct,&(asd_bss_index)); 
                  //printf("#################wid_wtp_asd_bss_index=%d############\n",asd_bss_index);
				  dbus_message_iter_next(&iter_sub_sub_struct);	
				  dbus_message_iter_get_basic(&iter_sub_sub_struct,&asd_bss_wlan_id);

				  dbus_message_iter_next(&iter_sub_sub_struct);	
				  dbus_message_iter_get_basic(&iter_sub_sub_struct,&acc_tms);				//xiaodawei add access times for telecom test, 20110301
				  
				  dbus_message_iter_next(&iter_sub_sub_struct);	
				  dbus_message_iter_get_basic(&iter_sub_sub_struct,&auth_tms);

				  dbus_message_iter_next(&iter_sub_sub_struct);	
				  dbus_message_iter_get_basic(&iter_sub_sub_struct,&repauth_tms);

				  dbus_message_iter_next(&iter_sub_sub_struct);	
				  dbus_message_iter_get_basic(&iter_sub_sub_struct,&assoc_req);

				  dbus_message_iter_next(&iter_sub_sub_struct);	
				  dbus_message_iter_get_basic(&iter_sub_sub_struct,&assoc_resp);

				  dbus_message_iter_next(&iter_sub_sub_struct);	
				  dbus_message_iter_get_basic(&iter_sub_sub_struct,&rx_ctrl_pkts);

				  dbus_message_iter_next(&iter_sub_sub_struct);	
				  dbus_message_iter_get_basic(&iter_sub_sub_struct,&tx_ctrl_pkts);

				/*  dbus_message_iter_next(&iter_sub_sub_struct);	
				  dbus_message_iter_get_basic(&iter_sub_sub_struct,&rx_data_pkts);
					printf("  rx_data_pkts   =  %d \n",rx_data_pkts);
				  dbus_message_iter_next(&iter_sub_sub_struct);	
				  dbus_message_iter_get_basic(&iter_sub_sub_struct,&tx_data_pkts);
				  printf("	tx_data_pkts	 =	%d \n",tx_data_pkts);*/

				  dbus_message_iter_next(&iter_sub_sub_struct);	
				  dbus_message_iter_get_basic(&iter_sub_sub_struct,&SSIDApChStatsNumStations); 


				  
                 //fengwenchao add 20110127
				  dbus_message_iter_next(&iter_sub_sub_struct);	
				  dbus_message_iter_get_basic(&iter_sub_sub_struct,&wl_up_flow);
				//  printf("wl_up_flow:%lld\n",wl_up_flow);
				 // printf("wl_up_flow:%llu\n",wl_up_flow);
				  dbus_message_iter_next(&iter_sub_sub_struct);	
				  dbus_message_iter_get_basic(&iter_sub_sub_struct,&wl_dw_flow);
				//  printf("wl_dw_flow:%lld\n",wl_dw_flow);
				//  printf("wl_up_flow:%llu\n",wl_up_flow);
				  dbus_message_iter_next(&iter_sub_sub_struct);	
				  dbus_message_iter_get_basic(&iter_sub_sub_struct,&ch_dw_pck);

				  dbus_message_iter_next(&iter_sub_sub_struct);	
				  dbus_message_iter_get_basic(&iter_sub_sub_struct,&ch_dw_los_pck);

				  dbus_message_iter_next(&iter_sub_sub_struct);	
				  dbus_message_iter_get_basic(&iter_sub_sub_struct,&ch_dw_mac_err_pck);

				  dbus_message_iter_next(&iter_sub_sub_struct);	
				  dbus_message_iter_get_basic(&iter_sub_sub_struct,&ch_dw_resend_pck);

				  dbus_message_iter_next(&iter_sub_sub_struct);	
				  dbus_message_iter_get_basic(&iter_sub_sub_struct,&ch_up_frm);				  

				  dbus_message_iter_next(&iter_sub_sub_struct);	
				  dbus_message_iter_get_basic(&iter_sub_sub_struct,&ch_dw_frm);

				  dbus_message_iter_next(&iter_sub_sub_struct);	
				  dbus_message_iter_get_basic(&iter_sub_sub_struct,&ch_dw_err_frm);
				  
				  dbus_message_iter_next(&iter_sub_sub_struct);	
				  dbus_message_iter_get_basic(&iter_sub_sub_struct,&ch_dw_los_frm);
				  
				  dbus_message_iter_next(&iter_sub_sub_struct);	
				  dbus_message_iter_get_basic(&iter_sub_sub_struct,&ch_dw_resend_frm);
				  //printf("ch_dw_resend_frm:%ld\n",ch_dw_resend_frm);
				  dbus_message_iter_next(&iter_sub_sub_struct);	
				  dbus_message_iter_get_basic(&iter_sub_sub_struct,&ch_up_los_frm);
				  
				  dbus_message_iter_next(&iter_sub_sub_struct);	
				  dbus_message_iter_get_basic(&iter_sub_sub_struct,&ch_up_resend_frm);

				  //printf("wl_up_flow = %lu\n",wl_up_flow);
				  // printf("wl_up_flow =  %llu\n",wl_up_flow);
				  //fengwenchao add end
				  
                  WtpSearchNode = WtpHead->SSIDStatsInfo_list;
                  while(WtpSearchNode!= NULL){
                     if(WtpSearchNode->SSIDStatsInfo_Radioid_info_head== NULL){
						break;
					 }
                    else {
                      if((WtpSearchNode->wtpBwlanRadioNum!=0)&&(asd_wtpCurrID==WtpSearchNode->wtpCurrID)){
					  	    struct SSIDStatsInfo_Radioid_info *Radioid_info_search =NULL;
							Radioid_info_search = WtpSearchNode->SSIDStatsInfo_Radioid_info_head;
							while(Radioid_info_search != NULL){
                               if((Radioid_info_search->radioBwlanNum !=0)&&(asd_wtpRadioId==Radioid_info_search->radioId))
                               	{   
                               	    struct SSIDStatsInfo_sub_wlan *sub_wlan_search =NULL;
							        sub_wlan_search = Radioid_info_search->SSIDStatsInfo_sub_wlan_head;
									while(sub_wlan_search !=NULL){

                                      if(sub_wlan_search->wlanCurrID == asd_bss_wlan_id){
										   sub_wlan_search->SSIDRxCtrlFrameCnt = rx_ctrl_pkts;
										//   sub_wlan_search->SSIDRxDataFrameCnt = rx_data_pkts;
										   sub_wlan_search->SSIDAccessTimes = acc_tms;
										   sub_wlan_search->SSIDRxAuthenFrameCnt = auth_tms;
										
										   sub_wlan_search->SSIDRxAssociateFrameCnt = assoc_req;
										   sub_wlan_search->SSIDTxCtrlFrameCnt = tx_ctrl_pkts;
										 //  sub_wlan_search->SSIDTxDataFrameCnt = tx_data_pkts;
										 
										   sub_wlan_search->SSIDTxAuthenFrameCnt = repauth_tms;
										   sub_wlan_search->SSIDTxAssociateFrameCnt = assoc_resp;
										   sub_wlan_search->SSIDTxAssociateFrameCnt = SSIDApChStatsNumStations;
										   //fengwenchao add 20110127
										   sub_wlan_search->wl_up_flow = wl_up_flow;
										   sub_wlan_search->wl_dw_flow = wl_dw_flow;
										   sub_wlan_search->ch_dw_pck = ch_dw_pck;
										   sub_wlan_search->ch_dw_los_pck = ch_dw_los_pck;
										   sub_wlan_search->ch_dw_mac_err_pck = ch_dw_mac_err_pck;
										   sub_wlan_search->ch_dw_resend_pck = ch_dw_resend_pck;
										   sub_wlan_search->ch_up_frm = ch_up_frm;
										   sub_wlan_search->ch_dw_frm = ch_dw_frm;
										   sub_wlan_search->ch_dw_err_frm = ch_dw_err_frm;
										   sub_wlan_search->ch_dw_los_frm = ch_dw_los_frm;
										   sub_wlan_search->ch_dw_resend_frm = ch_dw_resend_frm;
										   sub_wlan_search->ch_up_los_frm = ch_up_los_frm;
										   sub_wlan_search->ch_up_resend_frm = ch_up_resend_frm;
										//printf("sub_wlan_search->wl_up_flow = %llu\n",sub_wlan_search->wl_up_flow );
										//printf("wl_up_flow = %llu\n",wl_up_flow);
										  //fengwenchao add end
                                      	}
									  sub_wlan_search = sub_wlan_search->next;
									}
									

							    }
                              Radioid_info_search = Radioid_info_search->next;
							}
                      	}
                      
					 }
                    WtpSearchNode = WtpSearchNode->next;
				  }

                   dbus_message_iter_next(&iter_sub_sub_array);
			     }
			 
               dbus_message_iter_next(&iter_sub_array);
			}

		   dbus_message_iter_next(&iter_array2);
		}
		dbus_message_unref(reply2);
	}
	 //printf("show_wtp_wlan_SSIDStatsInfo_of_all_wtp_end_asd\n");
	

	return WtpHead;
}
/*table 10*/


/*table 10*/
struct WtpIfnameInfo * show_WtpIfnameInfo_of_all_wtp(int index,int localid,DBusConnection *dcli_dbus_connection, unsigned int *num, unsigned int *ret)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;	
	DBusMessageIter iter_struct;
	DBusMessageIter	iter_sub_array;
	DBusMessageIter iter_sub_struct;
	
	struct WtpIfnameInfo *WtpNode = NULL;
	struct WtpIfnameInfo *WtpHead = NULL;
	struct WtpIfInfo_sub_info *Wtp_sub_Node = NULL;
	int i, j;
	char * wtpModel = NULL;
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,\
						INTERFACE,WID_DBUS_CONF_METHOD_SHOW_ALL_WTP_IFNAME_INFORMATION);
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		*ret = WID_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	if(*ret == 0){
			if((WtpHead = (struct WtpIfnameInfo*)malloc(sizeof(struct WtpIfnameInfo))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			memset(WtpHead,0,sizeof(struct WtpIfnameInfo));
			WtpHead->WtpIfnameInfo_list = NULL;
			WtpHead->WtpIfnameInfo_last = NULL;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,num);
	
			dbus_message_iter_next(&iter);	
			dbus_message_iter_recurse(&iter,&iter_array);
			
			for (i = 0; i < *num; i++) {
				if((WtpNode = (struct WtpIfnameInfo*)malloc(sizeof(struct WtpIfnameInfo))) == NULL){
						dcli_free_WtpIfnameInfo(WtpHead);
						*ret = MALLOC_ERROR;
						dbus_message_unref(reply);
						return NULL;
				}
				
				memset(WtpNode,0,sizeof(struct WtpIfnameInfo));
				WtpNode->next = NULL;

				WtpNode->wtpMacAddr = (unsigned char *)malloc(MAC_LEN +1);
				memset(WtpNode->wtpMacAddr,0,(MAC_LEN +1));
	
				if(WtpHead->WtpIfnameInfo_list == NULL){
					WtpHead->WtpIfnameInfo_list = WtpNode;
					WtpHead->next = WtpNode;
				}else{
					WtpHead->WtpIfnameInfo_last->next = WtpNode;
					
				}
				WtpHead->WtpIfnameInfo_last = WtpNode;
				
				dbus_message_iter_recurse(&iter_array,&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpCurrID));

				dbus_message_iter_next(&iter_struct);	
				dbus_message_iter_get_basic(&iter_struct,&(wtpModel));

				dbus_message_iter_next(&iter_struct);	
				dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[0]));
				
				dbus_message_iter_next(&iter_struct);	
				dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[1]));
				
				dbus_message_iter_next(&iter_struct);	
				dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[2]));
				
				dbus_message_iter_next(&iter_struct);	
				dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[3]));
				
				dbus_message_iter_next(&iter_struct);	
				dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[4]));
				
				dbus_message_iter_next(&iter_struct);	
				dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[5]));

				//WtpNode->wtpMTU = 1500;

				/*=========================================================*/
				WtpNode->wtpModel = (char*)malloc(strlen(wtpModel)+1);
				memset(WtpNode->wtpModel, 0, strlen(wtpModel)+1);
				memcpy(WtpNode->wtpModel, wtpModel, strlen(wtpModel));	
				/*=========================================================*/
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpIfIndexNum));
				
	 			dbus_message_iter_next(&iter_struct);
				dbus_message_iter_recurse(&iter_struct,&iter_sub_array);

				for(j=0; j<WtpNode->wtpIfIndexNum; j++){  
					unsigned char type;
					unsigned char ifindex;
					unsigned char state;/*0-not exist/1-up/2-down/3-error*/
					unsigned int eth_rate;/*10M,100M*/
					time_t state_time;
					time_t now = 0;
					time_t last_chage = 0;
					time(&now);

					if((Wtp_sub_Node = (struct WtpIfInfo_sub_info*)malloc(sizeof(struct WtpIfInfo_sub_info))) == NULL){
						dcli_free_WtpIfnameInfo(WtpHead);
						*ret = MALLOC_ERROR;
						dbus_message_unref(reply);
						return NULL;
					}
					
					memset(Wtp_sub_Node,0,sizeof(struct WtpIfInfo_sub_info));
					Wtp_sub_Node->next = NULL;
					Wtp_sub_Node->WtpIfInfo_sub_info_list = NULL;
					Wtp_sub_Node->WtpIfInfo_sub_info_last = NULL;


					if(WtpNode->WtpIfInfo_sub_info_head == NULL){
						WtpNode->WtpIfInfo_sub_info_head = Wtp_sub_Node;
					}
					else{
						WtpNode->WtpIfInfo_sub_info_head->WtpIfInfo_sub_info_last->next = Wtp_sub_Node;
					}
					
					WtpNode->WtpIfInfo_sub_info_head->WtpIfInfo_sub_info_last = Wtp_sub_Node;
					
					dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
					dbus_message_iter_get_basic(&iter_sub_struct,&(Wtp_sub_Node->wtpIfType));

					dbus_message_iter_next(&iter_sub_struct);	
					dbus_message_iter_get_basic(&iter_sub_struct,&Wtp_sub_Node->wtpifinfo_report_switch);

					dbus_message_iter_next(&iter_sub_struct);	
					dbus_message_iter_get_basic(&iter_sub_struct,&Wtp_sub_Node->wtpIfIndex);

					dbus_message_iter_next(&iter_sub_struct);	
					dbus_message_iter_get_basic(&iter_sub_struct,&state);

					dbus_message_iter_next(&iter_sub_struct);	
					dbus_message_iter_get_basic(&iter_sub_struct,&state_time);
					
					Wtp_sub_Node->wtpIfAdminStatus = state;
					Wtp_sub_Node->wtpIfOperStatus = state;
					Wtp_sub_Node->state_time = state_time;

					dbus_message_iter_next(&iter_sub_struct);	
					dbus_message_iter_get_basic(&iter_sub_struct,&eth_rate);
					//fengwenchao add 20110127 for XJDEV-32 from 2.0
					dbus_message_iter_next(&iter_sub_struct);	
					dbus_message_iter_get_basic(&iter_sub_struct,&Wtp_sub_Node->wtpMTU);
					//fengwenchao add end
					dbus_message_iter_next(&iter_sub_struct);	
					dbus_message_iter_get_basic(&iter_sub_struct,&Wtp_sub_Node->wtpIfUplinkRealtimeRate);

					dbus_message_iter_next(&iter_sub_struct);	
					dbus_message_iter_get_basic(&iter_sub_struct,&Wtp_sub_Node->wtpIfDownlinkRealtimeRate);
					
					Wtp_sub_Node->wtpIfSpeed = eth_rate*1000;
					if(state != 0)  //fengwenchao add 20111013 for chinamobile-136
						last_chage =( now - state_time)*100  ;
					
					Wtp_sub_Node->wtpIfLastChange = last_chage;
					
					dbus_message_iter_next(&iter_sub_array);
				 }
				dbus_message_iter_next(&iter_array);
			}
		}
		dbus_message_unref(reply);

		return WtpHead;
}

/*nl add for showing radio para information 20100519*//*table 11*/
struct WtpRadioParaInfo * show_WtpRadioParaInfo_of_all_wtp(int index,int localid,DBusConnection *dcli_dbus_connection, unsigned int *wtp_num, unsigned int *ret)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;	
	DBusMessageIter iter_struct;
	DBusMessageIter iter_sub_array; 	
	DBusMessageIter iter_sub_struct;
	
	struct WtpRadioParaInfo *WtpNode = NULL;
	struct WtpRadioParaInfo *WtpHead = NULL;
	struct WtpRadioParaInfo *WtpSearchNode = NULL;
	struct Sub_RadioParaInfo *sub_radio_node = NULL;
	struct Sub_RadioParaInfo *sub_radio_search = NULL;
	
	int i, j;
	char *wtpModel;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,\
						INTERFACE,WID_DBUS_CONF_METHOD_SHOW_ALL_WTP_RADIO_PARA_INFOR_INFORMATION);
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		*ret = WID_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	if(*ret == 0){
		if((WtpHead = (struct WtpRadioParaInfo*)malloc(sizeof(struct WtpRadioParaInfo))) == NULL){
			*ret = MALLOC_ERROR;
			dbus_message_unref(reply);
			return NULL;
		}
		memset(WtpHead,0,sizeof(struct WtpRadioParaInfo));
		WtpHead->WtpRadioParaInfo_list = NULL;
		WtpHead->WtpRadioParaInfo_last = NULL;
		WtpHead->Sub_RadioParaInfo_head = NULL;
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,wtp_num);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
		
		for (i = 0; i < *wtp_num; i++) {
			if((WtpNode = (struct WtpRadioParaInfo*)malloc(sizeof(struct WtpRadioParaInfo))) == NULL){
					dcli_free_wtp_WtpRadioParaInfo(WtpHead);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(WtpNode,0,sizeof(struct WtpRadioParaInfo));
			WtpNode->next = NULL;
			WtpNode->Sub_RadioParaInfo_head = NULL;

			WtpNode->wtpMacAddr = (unsigned char *)malloc(MAC_LEN +1);
			memset(WtpNode->wtpMacAddr,0,(MAC_LEN +1));

			if(WtpHead->WtpRadioParaInfo_list == NULL){
				WtpHead->WtpRadioParaInfo_list = WtpNode;
				WtpHead->next = WtpNode;
			}else{
				WtpHead->WtpRadioParaInfo_last->next = WtpNode;
				
			}
			WtpHead->WtpRadioParaInfo_last = WtpNode;
			
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpCurrID));
		
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[0]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[1]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[2]));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[3]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[4]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[5]));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpSignalSNR));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpRadioNum));

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_recurse(&iter_struct,&iter_sub_array);

			for(j=0; j<WtpNode->wtpRadioNum; j++){	
				if((sub_radio_node = (struct Sub_RadioParaInfo*)malloc(sizeof(struct Sub_RadioParaInfo))) == NULL){
					dcli_free_wtp_WtpRadioParaInfo(WtpHead);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
				}
			
				memset(sub_radio_node,0,sizeof(struct Sub_RadioParaInfo));
				sub_radio_node->next = NULL;
				sub_radio_node->Sub_RadioParaInfo_list = NULL;
				sub_radio_node->Sub_RadioParaInfo_last = NULL;

				if(WtpNode->Sub_RadioParaInfo_head == NULL){
					WtpNode->Sub_RadioParaInfo_head = sub_radio_node;
				}else{
					WtpNode->Sub_RadioParaInfo_head->Sub_RadioParaInfo_last->next = sub_radio_node;
					
				}
				WtpNode->Sub_RadioParaInfo_head->Sub_RadioParaInfo_last = sub_radio_node;

				dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->wtpRadCurrID));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->wtpRadLocalID));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->wtpMessageNeafThreshold));
				
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->wtpPreambleLen));
				
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->wtpRTSThreshold));	
				
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->wtpConfigLongRetransThreshold));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->wtpSignalAveIntensity));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->wtpFrequencyHopTimes));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->wtpFreHopDetectTime));	

				dbus_message_iter_next(&iter_sub_array);
			}
			dbus_message_iter_next(&iter_array);
		}
	}
	dbus_message_unref(reply);
	
	return WtpHead;
}
/*nl add 20100519 for showing WtpEthPortInfo table 12*/
struct WtpEthPortInfo * show_WtpEthPortInfo_of_all_wtp(int index,int localid,DBusConnection *dcli_dbus_connection, unsigned int *num, unsigned int *ret)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;	
	
	struct WtpEthPortInfo *WtpNode = NULL;
	struct WtpEthPortInfo *WtpHead = NULL;
	int i;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,\
						INTERFACE,WID_DBUS_CONF_METHOD_SHOW_ALL_WTP_ETH_PORT_INFORMATION);
	
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		*ret = WID_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	if(*ret == 0){
			if((WtpHead = (struct WtpEthPortInfo*)malloc(sizeof(struct WtpEthPortInfo))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			memset(WtpHead,0,sizeof(struct WtpEthPortInfo));
			WtpHead->WtpEthPortInfo_list = NULL;
			WtpHead->WtpEthPortInfo_last = NULL;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,num);
	
			dbus_message_iter_next(&iter);	
			dbus_message_iter_recurse(&iter,&iter_array);
			
			for (i = 0; i < *num; i++) {
				DBusMessageIter iter_struct;
				if((WtpNode = (struct WtpEthPortInfo*)malloc(sizeof(struct WtpEthPortInfo))) == NULL){
						dcli_free_WtpEthPortInfo(WtpHead);
						*ret = MALLOC_ERROR;
						dbus_message_unref(reply);
						return NULL;
				}
				
				memset(WtpNode,0,sizeof(struct WtpEthPortInfo));
				WtpNode->next = NULL;

				WtpNode->wtpMacAddr = (unsigned char *)malloc(MAC_LEN +1);
				memset(WtpNode->wtpMacAddr,0,(MAC_LEN +1));
	
				if(WtpHead->WtpEthPortInfo_list == NULL){
					WtpHead->WtpEthPortInfo_list = WtpNode;
					WtpHead->next = WtpNode;
				}else{
					WtpHead->WtpEthPortInfo_last->next = WtpNode;
					
				}
				WtpHead->WtpEthPortInfo_last = WtpNode;
				
				dbus_message_iter_recurse(&iter_array,&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpCurrID));
			
				dbus_message_iter_next(&iter_struct);	
				dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[0]));
				
				dbus_message_iter_next(&iter_struct);	
				dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[1]));
				
				dbus_message_iter_next(&iter_struct);	
				dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[2]));
				
				dbus_message_iter_next(&iter_struct);	
				dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[3]));
				
				dbus_message_iter_next(&iter_struct);	
				dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[4]));
				
				dbus_message_iter_next(&iter_struct);	
				dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[5]));

				dbus_message_iter_next(&iter_struct);	
				dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpWirelessUpPortRate));//tx_rate

				dbus_message_iter_next(&iter_struct);	
				dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpWirelessDownPortRate));//rate

				dbus_message_iter_next(&iter_struct);	
				dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpWirelessUpPortUDTimes));
							
				dbus_message_iter_next(&iter_struct);	
				dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpWirelessDownPortUDTimes));

				dbus_message_iter_next(&iter_struct);	
				dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpUplinkDataThroughput));
											
				dbus_message_iter_next(&iter_struct);	
				dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpDownlinkDataThroughput));

				dbus_message_iter_next(&iter_struct);	
				dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpPhyInterMnt));
				
				dbus_message_iter_next(&iter_array);
			}
		}
		dbus_message_unref(reply);
		return WtpHead;
}
/*for showting RadioStatsInfo by nl 20100520 table 13*/
struct RadioStatsInfo * show_RadioStatsInfo_of_all_wtp(int index,int localid,DBusConnection *dcli_dbus_connection, unsigned int *wtp_num, unsigned int *ret)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;	
	DBusMessageIter iter_struct;
	DBusMessageIter iter_sub_array; 	
	DBusMessageIter iter_sub_struct;
	
	struct RadioStatsInfo *WtpNode = NULL;
	struct RadioStatsInfo *WtpHead = NULL;
	struct RadioStatsInfo *WtpSearchNode = NULL;
	struct Sub_RadioStatsInfo *sub_radio_node = NULL;
	struct Sub_RadioStatsInfo *sub_radio_search = NULL;
	
	int i ,j;
	char *wtpModel;
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
		
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,\
						INTERFACE,WID_DBUS_CONF_METHOD_SHOW_ALL_WTP_RADIO_STATS_INFORMATION);
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		*ret = WID_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	if(*ret == 0){
		if((WtpHead = (struct RadioStatsInfo*)malloc(sizeof(struct RadioStatsInfo))) == NULL){
			*ret = MALLOC_ERROR;
			dbus_message_unref(reply);
			return NULL;
		}
		memset(WtpHead,0,sizeof(struct RadioStatsInfo));
		WtpHead->RadioStatsInfo_list = NULL;
		WtpHead->RadioStatsInfo_last = NULL;
		WtpHead->Sub_RadioStatsInfo_head = NULL;
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,wtp_num);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
		
		for (i = 0; i < *wtp_num; i++) {
			if((WtpNode = (struct RadioStatsInfo*)malloc(sizeof(struct RadioStatsInfo))) == NULL){
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					dcli_free_wtp_RadioStatsInfo(WtpHead);					
					return NULL;
			}
			
			memset(WtpNode,0,sizeof(struct RadioStatsInfo));
			WtpNode->next = NULL;
			WtpNode->Sub_RadioStatsInfo_head = NULL;

			WtpNode->wtpMacAddr = (unsigned char *)malloc(MAC_LEN +1);
			memset(WtpNode->wtpMacAddr,0,(MAC_LEN +1));

			if(WtpHead->RadioStatsInfo_list == NULL){
				WtpHead->RadioStatsInfo_list = WtpNode;
				WtpHead->next = WtpNode;
			}else{
				WtpHead->RadioStatsInfo_last->next = WtpNode;
				
			}
			WtpHead->RadioStatsInfo_last = WtpNode;
			
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpCurrID));
		
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[0]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[1]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[2]));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[3]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[4]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[5]));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpPowerManage));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpSampleTime));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtp_radio_num));	
			
			/*for radio information*/
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_recurse(&iter_struct,&iter_sub_array);

			for(j=0; j<WtpNode->wtp_radio_num; j++){	
				unsigned short Radio_TXP = 0;
				if((sub_radio_node = (struct Sub_RadioStatsInfo*)malloc(sizeof(struct Sub_RadioStatsInfo))) == NULL){
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					dcli_free_wtp_RadioStatsInfo(WtpHead);
					return NULL;
				}
			
				memset(sub_radio_node,0,sizeof(struct Sub_RadioStatsInfo));
				sub_radio_node->next = NULL;
				sub_radio_node->Sub_RadioStatsInfo_list = NULL;
				sub_radio_node->Sub_RadioStatsInfo_last = NULL;

				if(WtpNode->Sub_RadioStatsInfo_head == NULL){
					WtpNode->Sub_RadioStatsInfo_head = sub_radio_node;
				}else{
					WtpNode->Sub_RadioStatsInfo_head->Sub_RadioStatsInfo_last->next = sub_radio_node;
					
				}
				WtpNode->Sub_RadioStatsInfo_head->Sub_RadioStatsInfo_last = sub_radio_node;

				dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->wtpRadCurrID));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->wtpRadLocalID));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(Radio_TXP));	
				sub_radio_node->wtpCurSendPower = Radio_TXP;
				sub_radio_node->wtpRecvPower = Radio_TXP;

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->wtpCurConfChannel));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->radio_type));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->wtpconfigBeaconFrameBlank));

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->wtpTerminalConRate));

				dbus_message_iter_next(&iter_sub_array);
			}
			dbus_message_iter_next(&iter_array);
		}
	}
	dbus_message_unref(reply);

	return WtpHead;
}

/*for showting RadioConfigInfo by nl 20100524 table 14*/
struct WtpConfigRadioInfo * show_WtpConfigRadioInfo_of_all_wtp(int index,int localid,DBusConnection *dcli_dbus_connection, unsigned int *wtp_num, unsigned int *ret)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;	
	DBusMessageIter iter_struct;
	DBusMessageIter iter_sub_array; 	
	DBusMessageIter iter_sub_struct;
	DBusMessageIter iter_sub_sub_array;
	DBusMessageIter iter_sub_sub_struct;
	DBusMessageIter iter_sub_sub_sub_array;	/* Huang Leilei add for Hu Pingxin:AXSSZFI-1323 */
	DBusMessageIter iter_sub_sub_sub_struct;	/* Huang Leilei add for Hu Pingxin:AXSSZFI-1323 */
	
	struct WtpConfigRadioInfo *WtpNode = NULL;
	struct WtpConfigRadioInfo *WtpHead = NULL;
	struct WtpConfigRadioInfo *WtpSearchNode = NULL;
	struct Sub_WtpConfigRadioInfo *sub_radio_node = NULL;
	struct Sub_WtpConfigRadioInfo *sub_radio_search = NULL;
	
	int i,j,k = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,\
						INTERFACE,WID_DBUS_CONF_METHOD_SHOW_ALL_WTP_RADIO_CONFIG_INFORMATION);
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		*ret = WID_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	if(*ret == 0){
		if((WtpHead = (struct WtpConfigRadioInfo*)malloc(sizeof(struct WtpConfigRadioInfo))) == NULL){
			*ret = MALLOC_ERROR;
			dbus_message_unref(reply);
			return NULL;
		}
		memset(WtpHead,0,sizeof(struct WtpConfigRadioInfo));
		WtpHead->WtpConfigRadioInfo_list = NULL;
		WtpHead->WtpConfigRadioInfo_last = NULL;
		WtpHead->Sub_WtpConfigRadioInfo_head = NULL;
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,wtp_num);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
		
		for (i = 0; i < *wtp_num; i++) {
			if((WtpNode = (struct WtpConfigRadioInfo*)malloc(sizeof(struct WtpConfigRadioInfo))) == NULL){
					dcli_free_WtpConfigRadioInfo(WtpHead);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(WtpNode,0,sizeof(struct WtpConfigRadioInfo));
			WtpNode->next = NULL;
			WtpNode->Sub_WtpConfigRadioInfo_head = NULL;

			WtpNode->wtpMacAddr = (unsigned char *)malloc(MAC_LEN +1);
			memset(WtpNode->wtpMacAddr,0,(MAC_LEN +1));

			if(WtpHead->WtpConfigRadioInfo_list == NULL){
				WtpHead->WtpConfigRadioInfo_list = WtpNode;
				WtpHead->next = WtpNode;
			}else{
				WtpHead->WtpConfigRadioInfo_last->next = WtpNode;
				
			}
			WtpHead->WtpConfigRadioInfo_last = WtpNode;
			
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpCurrID));
		
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[0]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[1]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[2]));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[3]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[4]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[5]));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtp_radio_num));	
			
			/*for radio information*/
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_recurse(&iter_struct,&iter_sub_array);

			for(j=0; j<WtpNode->wtp_radio_num; j++){	
				unsigned int qosid;
				unsigned int radio_rate;
				unsigned char AdStat = 0;
				if((sub_radio_node = (struct Sub_WtpConfigRadioInfo*)malloc(sizeof(struct Sub_WtpConfigRadioInfo))) == NULL){
					dcli_free_WtpConfigRadioInfo(WtpHead);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
				}
			
				memset(sub_radio_node,0,sizeof(struct Sub_WtpConfigRadioInfo));
				sub_radio_node->next = NULL;
				sub_radio_node->Sub_WtpConfigRadioInfo_list = NULL;
				sub_radio_node->Sub_WtpConfigRadioInfo_last = NULL;

				if(WtpNode->Sub_WtpConfigRadioInfo_head == NULL){
					WtpNode->Sub_WtpConfigRadioInfo_head = sub_radio_node;
				}else{
					WtpNode->Sub_WtpConfigRadioInfo_head->Sub_WtpConfigRadioInfo_last->next = sub_radio_node;
					
				}
				WtpNode->Sub_WtpConfigRadioInfo_head->Sub_WtpConfigRadioInfo_last = sub_radio_node;

				dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->wtpRadCurrID));	
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->wtpRadLocalID));	
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->radioType));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->radioBeaf));		//FragThreshold
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->radioBeacon));	//radioBeacon
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->radioLeadCode));	//radioLeadCode 要做判断

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->radioDtim));		//DTIMPeriod
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->radioRTS));		//rtsthreshold
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->radioShortRetry));	//

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->radioLongRetry));	//
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->radioChannel));		//unsigned char
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->radioTxPower));		//unsigned short

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(AdStat));					//unsigned char
				
				if(AdStat== 2){
					sub_radio_node->radioService=1;
				}
				else{
					sub_radio_node->radioService=2;
				}
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->radioMaxFlow));	
				
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(qosid));	

				dbus_message_iter_next(&iter_sub_struct);	 
				dbus_message_iter_get_basic(&iter_sub_struct,&(radio_rate));  
				
				dbus_message_iter_next(&iter_sub_struct);	 
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->radioWlanNum));  
				
				sub_radio_node->radioSpeed = radio_rate;
				sub_radio_node->radioMaxSpeed =radio_rate;
				sub_radio_node->radioBindQos = qosid;	
				sub_radio_node->radioDelbindQos = qosid;  
				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_recurse(&iter_sub_struct,&iter_sub_sub_array);
				for(k=0;k<sub_radio_node->radioWlanNum;k++){
					unsigned char radioBindWlan;
					dbus_message_iter_recurse(&iter_sub_sub_array,&iter_sub_sub_struct);
					dbus_message_iter_get_basic(&iter_sub_sub_struct,&(radioBindWlan));	
					sub_radio_node->radioBindWlan[k] = radioBindWlan;
					dbus_message_iter_next(&iter_sub_sub_array);
				}

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct, &(sub_radio_node->AmpduAble));
				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct, &(sub_radio_node->AmsduAble));
				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct, &(sub_radio_node->cwmode));
				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct, &(sub_radio_node->guardinterval));
				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct, &(sub_radio_node->Mixed_Greenfield));
				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct, &(sub_radio_node->Radio_Type));
				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct, &(sub_radio_node->mcs_count));
				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_recurse(&iter_sub_struct,&iter_sub_sub_sub_array);
				for (k = 0; k < sub_radio_node->mcs_count; k++)
				{
					dbus_message_iter_recurse(&iter_sub_sub_sub_array, &iter_sub_sub_sub_struct);
					dbus_message_iter_get_basic(&iter_sub_sub_sub_struct, &(sub_radio_node->mcs_list[k]));
					dbus_message_iter_next(&iter_sub_sub_sub_array);
				}

				
				dbus_message_iter_next(&iter_sub_array);
			}
			dbus_message_iter_next(&iter_array);
		}
	}
	dbus_message_unref(reply);

	return WtpHead;
}
/*for showting UsrLinkInfo by nl 20100525 table 15*/
struct UsrLinkInfo* show_UsrLinkInfo_of_all_wtp(int index,int localid,DBusConnection *dcli_dbus_connection, unsigned int *num, unsigned int *ret)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;	
	
	struct UsrLinkInfo *WtpNode = NULL;
	struct UsrLinkInfo *WtpHead = NULL;
	struct UsrLinkInfo *WtpSearchNode = NULL;
	int i;int j;
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,\
							INTERFACE,ASD_DBUS_STA_METHOD_SHOW_USER_LINK_INFO_OF_ALL_WTP);
	
	dbus_error_init(&err);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);	
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(*ret == 0){
		if((WtpHead = (struct UsrLinkInfo*)malloc(sizeof(struct UsrLinkInfo))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
		}
		memset(WtpHead,0,sizeof(struct UsrLinkInfo));
		WtpHead->UsrLinkInfo_list = NULL;
		WtpHead->UsrLinkInfo_last = NULL;
			
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,num);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);

		for (i = 0; i < *num; i++) {
			DBusMessageIter iter_struct;
			DBusMessageIter iter_sub_array; 	
			if((WtpNode = (struct UsrLinkInfo*)malloc(sizeof(struct UsrLinkInfo))) == NULL){
					dcli_free_UsrLinkInfo(WtpHead);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(WtpNode,0,sizeof(struct UsrLinkInfo));
			WtpNode->next = NULL;

			WtpNode->wtpMacAddr = (unsigned char *)malloc(MAC_LEN +1);
			memset(WtpNode->wtpMacAddr,0,(MAC_LEN +1));

			if(WtpHead->UsrLinkInfo_list == NULL){
				WtpHead->UsrLinkInfo_list = WtpNode;
				WtpHead->next = WtpNode;
			}else{
				WtpHead->UsrLinkInfo_last->next = WtpNode;
				
			}
			WtpHead->UsrLinkInfo_last = WtpNode;
			
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpCurrID));
		
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[0]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[1]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[2]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[3]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[4]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[5]));

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpStaNewTotalOnlineTime));			//fot total online time 64

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpReasonInvalidFailLinkTimes));	
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpReasonTimeOutFailLinkTimes));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpReasonRefuseFailLinkTimes));

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpReasonOthersFailLinkTimes));	//a4
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpSolutionLinksVerifyLinkTimes));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpReasonLackAbilityVerifyLinkTimes));

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpReasonExceptionVerifyLinkTimes));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpReasonOtherVerfyLinkTimes));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpReasonUsrLeaveVerfiyLinkTimes));//a9

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpStaOnlineTime));//sta time

			dbus_message_iter_next(&iter_array);
		}
	}

	dbus_message_unref(reply);

	return WtpHead;
}
/*for showing WtpWiredIfStatsInfo  by nl 20100526 table 16*/
struct WtpWiredIfStatsInfo* show_WtpWiredIfStatsInfo_of_all_wtp(int index,int localid,DBusConnection *dcli_dbus_connection, unsigned int *num, unsigned int *ret)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;	
	DBusMessageIter iter_struct;
	DBusMessageIter	 iter_sub_array;
	DBusMessageIter	 iter_sub_struct;
	
	struct WtpWiredIfStatsInfo *WtpNode = NULL;
	struct WtpWiredIfStatsInfo *WtpHead = NULL;
	//struct WtpWiredIfStatsInfo *WtpSearchNode = NULL;
	struct WiredIfStatsInfo *EthTail = NULL;
	struct WiredIfStatsInfo *EthNode = NULL;
	int i=0,k=0;
	unsigned int rx_pkt_broadcast = 0;//zhaoruijia,20100831,根据需求接收的是包数,start
	unsigned int rx_pkt_unicast = 0;
	unsigned int tx_pkt_broadcast = 0; 
	unsigned int tx_pkt_unicast = 0;
	unsigned int rx_pkt_multicast = 0;
	unsigned int tx_pkt_multicast = 0; //zhaoruijia,20100831,根据需求接收的是包数,end
	unsigned int rx_packets = 0;
	unsigned int tx_packets = 0;
	unsigned int rx_errors = 0;
	unsigned int tx_errors = 0;
	unsigned long long rx_bytes = 0;
	unsigned long long tx_bytes = 0;
	unsigned int rx_drop = 0;
	unsigned int tx_drop = 0;

	unsigned long long rx_sum_bytes =0;  // zhangshu add , 2010-10-08
   	unsigned long long tx_sum_bytes = 0; // zhangshu add , 2010-10-08

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	//ReInitDbusPath(index,WID_DBUS_BUSNAME,BUSNAME);
	//ReInitDbusPath(index,WID_DBUS_OBJPATH,OBJPATH);
	//ReInitDbusPath(index,WID_DBUS_INTERFACE,INTERFACE);
	
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,\
						INTERFACE,WID_DBUS_CONF_METHOD_SHOW_ALL_WTP_WIRED_STATS_INFORMATION);
	
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		*ret = WID_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	if(*ret == 0){
		if((WtpHead = (struct WtpWiredIfStatsInfo*)malloc(sizeof(struct WtpWiredIfStatsInfo))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
		memset(WtpHead,0,sizeof(struct WtpWiredIfStatsInfo));
		WtpHead->WtpWiredIfStatsInfo_list = NULL;
		WtpHead->WtpWiredIfStatsInfo_last = NULL;
			
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,num);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
			
		for (i = 0; i < *num; i++) {
			if((WtpNode = (struct WtpWiredIfStatsInfo*)malloc(sizeof(struct WtpWiredIfStatsInfo))) == NULL){
					dcli_free_wtp_WiredIfStats_Info(WtpHead);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(WtpNode,0,sizeof(struct WtpWiredIfStatsInfo));
			WtpNode->next = NULL;

			WtpNode->wtpMacAddr = (unsigned char *)malloc(MAC_LEN +1);
			memset(WtpNode->wtpMacAddr,0,(MAC_LEN +1));

			if(WtpHead->WtpWiredIfStatsInfo_list == NULL){
				WtpHead->WtpWiredIfStatsInfo_list = WtpNode;
				WtpHead->next = WtpNode;
			}
			else{
				WtpHead->WtpWiredIfStatsInfo_last->next = WtpNode;
			}
			WtpHead->WtpWiredIfStatsInfo_last = WtpNode;
			
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpCurrID));
		
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[0]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[1]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[2]));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[3]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[4]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[5]));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpWireIfNum));

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_recurse(&iter_struct,&iter_sub_array);
			
			for(k=0;k<WtpNode->wtpWireIfNum;k++){

                if((EthNode = (struct WiredIfStatsInfo*)malloc(sizeof(struct WiredIfStatsInfo))) == NULL){
			        dcli_free_wtp_WiredIfStats_Info(WtpHead);
    				*ret = MALLOC_ERROR;
    				dbus_message_unref(reply);
    				return NULL;
    			}
        		memset(EthNode,0,sizeof(struct WiredIfStatsInfo));
        		EthNode->next = NULL;

        		if(WtpNode->EthInfo == NULL){
    				WtpNode->EthInfo = EthNode;
    				EthTail = EthNode;
    			}
    			else{
    			    EthTail->next = EthNode;
    			    EthTail = EthNode;	
    			}

    			rx_pkt_broadcast = 0;
            	rx_pkt_unicast = 0;
            	tx_pkt_broadcast = 0;
            	tx_pkt_unicast = 0;
            	rx_pkt_multicast = 0;
            	tx_pkt_multicast = 0;
            	rx_packets = 0;
            	tx_packets = 0;
            	rx_errors = 0;
            	tx_errors = 0;
            	rx_bytes = 0;
            	tx_bytes = 0;
            	rx_drop = 0;
            	tx_drop = 0;
            	rx_sum_bytes =0;
               	tx_sum_bytes = 0;
			
				dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(EthNode->wtpIfIndex));	
				
				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(EthNode->wtpWiredififUpDwnTimes));	

                dbus_message_iter_next(&iter_sub_struct);	
    			dbus_message_iter_get_basic(&iter_sub_struct,&(rx_pkt_broadcast));
    			dbus_message_iter_next(&iter_sub_struct);	
    			dbus_message_iter_get_basic(&iter_sub_struct,&(rx_pkt_unicast));
    			dbus_message_iter_next(&iter_sub_struct);	
    			dbus_message_iter_get_basic(&iter_sub_struct,&(tx_pkt_broadcast));

    			dbus_message_iter_next(&iter_sub_struct);	
    			dbus_message_iter_get_basic(&iter_sub_struct,&(tx_pkt_unicast));
    			dbus_message_iter_next(&iter_sub_struct);	
    			dbus_message_iter_get_basic(&iter_sub_struct,&(rx_pkt_multicast));
    			dbus_message_iter_next(&iter_sub_struct);	
    			dbus_message_iter_get_basic(&iter_sub_struct,&(tx_pkt_multicast));

    			dbus_message_iter_next(&iter_sub_struct);	
    			dbus_message_iter_get_basic(&iter_sub_struct,&(rx_packets));
    			dbus_message_iter_next(&iter_sub_struct);	
    			dbus_message_iter_get_basic(&iter_sub_struct,&(tx_packets));
    			dbus_message_iter_next(&iter_sub_struct);	
    			dbus_message_iter_get_basic(&iter_sub_struct,&(rx_errors));
    			dbus_message_iter_next(&iter_sub_struct);	
    			dbus_message_iter_get_basic(&iter_sub_struct,&(tx_errors));
    			
    			dbus_message_iter_next(&iter_sub_struct);	
    			dbus_message_iter_get_basic(&iter_sub_struct,&(rx_bytes));
    			dbus_message_iter_next(&iter_sub_struct);	
    			dbus_message_iter_get_basic(&iter_sub_struct,&(tx_bytes));
    			dbus_message_iter_next(&iter_sub_struct);	
    			dbus_message_iter_get_basic(&iter_sub_struct,&(rx_drop));
    			dbus_message_iter_next(&iter_sub_struct);	
    			dbus_message_iter_get_basic(&iter_sub_struct,&(tx_drop));

    			/*zhangshu add , 2010-10-08*/
    			dbus_message_iter_next(&iter_sub_struct);	
    			dbus_message_iter_get_basic(&iter_sub_struct,&(rx_sum_bytes));
    			dbus_message_iter_next(&iter_sub_struct);	
    			dbus_message_iter_get_basic(&iter_sub_struct,&(tx_sum_bytes));

    			EthNode->wtpWiredifInUcastPkts = rx_pkt_unicast; 
    			EthNode->wtpWiredififInNUcastPkts = rx_pkt_multicast; //book modify, 2011-1-20
    			EthNode->wtpWiredififOutUcastPkts = tx_pkt_unicast; 
    			EthNode->wtpWiredififOutNUcastPkts = tx_pkt_multicast; //book modify, 2011-1-20

    			EthNode->wtpWiredififInPkts = rx_packets; 
    			EthNode->wtpWiredififInDiscardPkts = rx_drop; 
    			EthNode->wtpWiredififInErrors = rx_errors; 
    			EthNode->wtpWiredififOutPkts = tx_packets; 
    			EthNode->wtpWiredififOutDiscardPkts = tx_drop; 
    			EthNode->wtpWiredififOutErrors = tx_errors; 
    			EthNode->wtpWiredififInOctets = rx_bytes; 
    			EthNode->wtpWiredififOutOctets = tx_bytes; 

    			EthNode->rx_sum_bytes = rx_sum_bytes; //zhangshu add 2010-10-08 
    			EthNode->tx_sum_bytes = tx_sum_bytes; //zhangshu add 2010-10-08
				
				dbus_message_iter_next(&iter_sub_array);
			}
			dbus_message_iter_next(&iter_array);
		}
	}

	dbus_message_unref(reply);

	return WtpHead;
}
#if 0

struct WtpWiredIfStatsInfo* show_WtpWiredIfStatsInfo_of_all_wtp(int index,int localid,DBusConnection *dcli_dbus_connection, unsigned int *num, unsigned int *ret)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;	
	DBusMessageIter iter_struct;
	DBusMessageIter	 iter_sub_array;
	DBusMessageIter	 iter_sub_struct;
	
	struct WtpWiredIfStatsInfo *WtpNode = NULL;
	struct WtpWiredIfStatsInfo *WtpHead = NULL;
	struct WiredIfStatsInfo *EthTail = NULL;
	struct WiredIfStatsInfo *EthNode = NULL;
	
	//struct WtpWiredIfStatsInfo *WtpSearchNode = NULL;
	int i=0,k=0;
	unsigned int rx_pkt_broadcast = 0;//zhaoruijia,20100831,根据需求接收的是包数,start
	unsigned int rx_pkt_unicast = 0;
	unsigned int tx_pkt_broadcast = 0; 
	unsigned int tx_pkt_unicast = 0;
	unsigned int rx_pkt_multicast = 0;
	unsigned int tx_pkt_multicast = 0; //zhaoruijia,20100831,根据需求接收的是包数,end
	unsigned int rx_packets = 0;
	unsigned int tx_packets = 0;
	unsigned int rx_errors = 0;
	unsigned int tx_errors = 0;
	unsigned long long rx_bytes = 0;
	unsigned long long tx_bytes = 0;
	unsigned int rx_drop = 0;
	unsigned int tx_drop = 0;

	unsigned long long rx_sum_bytes =0;  // zhangshu add , 2010-10-08
   	unsigned long long tx_sum_bytes = 0; // zhangshu add , 2010-10-08

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,\
						INTERFACE,WID_DBUS_CONF_METHOD_SHOW_ALL_WTP_WIRED_STATS_INFORMATION);
	
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		*ret = WID_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	if(*ret == 0){
		if((WtpHead = (struct WtpWiredIfStatsInfo*)malloc(sizeof(struct WtpWiredIfStatsInfo))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
		memset(WtpHead,0,sizeof(struct WtpWiredIfStatsInfo));
		WtpHead->WtpWiredIfStatsInfo_list = NULL;
		WtpHead->WtpWiredIfStatsInfo_last = NULL;
			
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,num);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
			
		for (i = 0; i < *num; i++) {
			if((WtpNode = (struct WtpWiredIfStatsInfo*)malloc(sizeof(struct WtpWiredIfStatsInfo))) == NULL){
					dcli_free_wtp_WiredIfStats_Info(WtpHead);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(WtpNode,0,sizeof(struct WtpWiredIfStatsInfo));
			WtpNode->next = NULL;

			WtpNode->wtpMacAddr = (unsigned char *)malloc(MAC_LEN +1);
			memset(WtpNode->wtpMacAddr,0,(MAC_LEN +1));

			if(WtpHead->WtpWiredIfStatsInfo_list == NULL){
				WtpHead->WtpWiredIfStatsInfo_list = WtpNode;
				WtpHead->next = WtpNode;
			}
			else{
				WtpHead->WtpWiredIfStatsInfo_last->next = WtpNode;
			}
			WtpHead->WtpWiredIfStatsInfo_last = WtpNode;
			
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpCurrID));
		
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[0]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[1]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[2]));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[3]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[4]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[5]));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpWireIfNum));

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_recurse(&iter_struct,&iter_sub_array);
			
			for(k=0;k<WtpNode->wtpWireIfNum;k++){
			    if((EthNode = (struct WiredIfStatsInfo*)malloc(sizeof(struct WiredIfStatsInfo))) == NULL){
			        dcli_free_wtp_WiredIfStats_Info(WtpHead);
    				*ret = MALLOC_ERROR;
    				dbus_message_unref(reply);
    				return NULL;
    			}
        		memset(EthNode,0,sizeof(struct WiredIfStatsInfo));
        		EthNode->next = NULL;

        		if(WtpNode->EthInfo == NULL){
    				WtpNode->EthInfo = EthNode;
    				EthTail = EthNode;
    			}
    			else{
    			    EthTail->next = EthNode;
    			    EthTail = EthNode;	
    			}

    			rx_pkt_broadcast = 0;
            	rx_pkt_unicast = 0;
            	tx_pkt_broadcast = 0;
            	tx_pkt_unicast = 0;
            	rx_pkt_multicast = 0;
            	tx_pkt_multicast = 0;
            	rx_packets = 0;
            	tx_packets = 0;
            	rx_errors = 0;
            	tx_errors = 0;
            	rx_bytes = 0;
            	tx_bytes = 0;
            	rx_drop = 0;
            	tx_drop = 0;
            	rx_sum_bytes =0;
               	tx_sum_bytes = 0;
			    
				dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(EthNode->wtpIfIndex));	
				
				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(EthNode->wtpWiredififUpDwnTimes));

				dbus_message_iter_next(&iter_sub_struct);	
    			dbus_message_iter_get_basic(&iter_sub_struct,&(rx_pkt_broadcast));
    			dbus_message_iter_next(&iter_sub_struct);	
    			dbus_message_iter_get_basic(&iter_sub_struct,&(rx_pkt_unicast));
    			dbus_message_iter_next(&iter_sub_struct);	
    			dbus_message_iter_get_basic(&iter_sub_struct,&(tx_pkt_broadcast));

    			dbus_message_iter_next(&iter_sub_struct);	
    			dbus_message_iter_get_basic(&iter_sub_struct,&(tx_pkt_unicast));
    			dbus_message_iter_next(&iter_sub_struct);	
    			dbus_message_iter_get_basic(&iter_sub_struct,&(rx_pkt_multicast));
    			dbus_message_iter_next(&iter_sub_struct);	
    			dbus_message_iter_get_basic(&iter_sub_struct,&(tx_pkt_multicast));

    			dbus_message_iter_next(&iter_sub_struct);	
    			dbus_message_iter_get_basic(&iter_sub_struct,&(rx_packets));
    			dbus_message_iter_next(&iter_sub_struct);	
    			dbus_message_iter_get_basic(&iter_sub_struct,&(tx_packets));
    			dbus_message_iter_next(&iter_sub_struct);	
    			dbus_message_iter_get_basic(&iter_sub_struct,&(rx_errors));
    			dbus_message_iter_next(&iter_sub_struct);	
    			dbus_message_iter_get_basic(&iter_sub_struct,&(tx_errors));
    			
    			dbus_message_iter_next(&iter_sub_struct);	
    			dbus_message_iter_get_basic(&iter_sub_struct,&(rx_bytes));
    			dbus_message_iter_next(&iter_sub_struct);	
    			dbus_message_iter_get_basic(&iter_sub_struct,&(tx_bytes));
    			dbus_message_iter_next(&iter_sub_struct);	
    			dbus_message_iter_get_basic(&iter_sub_struct,&(rx_drop));
    			dbus_message_iter_next(&iter_sub_struct);	
    			dbus_message_iter_get_basic(&iter_sub_struct,&(tx_drop));
    			
    			/*add for new requirement 20100910  begin nl*/
    			dbus_message_iter_next(&iter_sub_struct);	
    			dbus_message_iter_get_basic(&iter_sub_struct,&(rx_sum_bytes));
    			dbus_message_iter_next(&iter_sub_struct);	
    			dbus_message_iter_get_basic(&iter_sub_struct,&(tx_sum_bytes));

    			/*add for new requirement 20100910 end nl*/
    			
    			EthNode->wtpWiredifInUcastPkts = rx_pkt_unicast; 
    			EthNode->wtpWiredififInNUcastPkts = rx_pkt_multicast; //zhangshu modify 2010-12-24
    			EthNode->wtpWiredififOutUcastPkts = tx_pkt_unicast; 
    			EthNode->wtpWiredififOutNUcastPkts = tx_pkt_multicast; //zhangshu modify 2010-12-24
    			EthNode->wtpWiredifInBcastPkts = rx_pkt_broadcast;
				EthNode->wtpWiredifOutBcastPkts = tx_pkt_broadcast;

    			EthNode->wtpWiredififInPkts = rx_packets; 
    			EthNode->wtpWiredififInDiscardPkts = rx_drop; 
    			EthNode->wtpWiredififInErrors = rx_errors; 
    			EthNode->wtpWiredififOutPkts = tx_packets; 
    			EthNode->wtpWiredififOutDiscardPkts = tx_drop; 
    			EthNode->wtpWiredififOutErrors = tx_errors; 
    			EthNode->wtpWiredififInOctets = rx_bytes; 
    			EthNode->wtpWiredififOutOctets = tx_bytes; 
    			EthNode->rx_sum_bytes = rx_sum_bytes;
    			EthNode->tx_sum_bytes = tx_sum_bytes;
				
				dbus_message_iter_next(&iter_sub_array);
			}
			dbus_message_iter_next(&iter_array);
		}
	}

	dbus_message_unref(reply);

	return WtpHead;
}
#endif

/*for showing WtpWirelessIfStatsInfo  by nl 20100526 table 17*/
struct WtpWirelessIfInfo* show_WtpWirelessIfStatsInfo_of_all_wtp(int index,int localid,DBusConnection *dcli_dbus_connection, unsigned int *num, unsigned int *ret)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;	
	DBusMessageIter	 iter_sub_array;
	DBusMessageIter iter_sub_struct;
	DBusMessageIter	 iter_sub_sub_array;
	DBusMessageIter	 iter_sub_sub_struct;
	
	struct WtpWirelessIfInfo *WtpNode = NULL;
	struct WtpWirelessIfInfo *WtpHead = NULL;
	struct WtpWirelessIfInfo *WtpSearchNode = NULL;
	struct Sub_WtpWirelessIfInfo *RadioNode = NULL;
	int i, j, k;
	char *wtpModel =NULL;
	unsigned int txpower;
	//unsigned char support_mode;
	unsigned char Radio_Chan;
	unsigned char auto_channel_cont;
	unsigned short Radio_TXP;						/*xiaodawei modify for radio tx power, 20101202*/
	unsigned char txpowerautostate;
	unsigned char state ;
	unsigned short Radio_TXPSTEP;
	time_t online_time,now = 0;
	time_t state_time = 0;
					
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,\
						INTERFACE,WID_DBUS_CONF_METHOD_SHOW_ALL_WTP_WIRELESS_STATS_INFORMATION);
	
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		*ret = WID_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	if(*ret == 0){
		if((WtpHead = (struct WtpWirelessIfInfo*)malloc(sizeof(struct WtpWirelessIfInfo))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
		}
		memset(WtpHead,0,sizeof(struct WtpWirelessIfInfo));
		WtpHead->WtpWirelessIfInfo_list = NULL;
		WtpHead->WtpWirelessIfInfo_last = NULL;
		WtpHead->Sub_WtpWirelessIfInfo_head = NULL;
			
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,num);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
			
		for (i = 0; i < *num; i++) {
			DBusMessageIter iter_struct;
			if((WtpNode = (struct WtpWirelessIfInfo*)malloc(sizeof(struct WtpWirelessIfInfo))) == NULL){
					dcli_free_wtp_WtpWirelessIf_Info(WtpHead);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(WtpNode,0,sizeof(struct WtpWirelessIfInfo));
			WtpNode->next = NULL;

			WtpNode->wtpMacAddr = (unsigned char *)malloc(MAC_LEN +1);
			memset(WtpNode->wtpMacAddr,0,(MAC_LEN +1));

			if(WtpHead->WtpWirelessIfInfo_list == NULL){
				WtpHead->WtpWirelessIfInfo_list = WtpNode;
				WtpHead->next = WtpNode;
			}
			else{
				WtpHead->WtpWirelessIfInfo_last->next = WtpNode;
			}
			WtpHead->WtpWirelessIfInfo_last = WtpNode;
			
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpCurrID));
		
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[0]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[1]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[2]));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[3]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[4]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[5]));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(wtpModel));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpWirelessIfAntennaGain));
			
			WtpNode->wtpModel = (char*)malloc(strlen(wtpModel)+1);
			memset(WtpNode->wtpModel, 0, strlen(wtpModel)+1);
			memcpy(WtpNode->wtpModel, wtpModel, strlen(wtpModel));	
			WtpNode->wtpWirelessIfMTU = 1500;
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(txpower));
			WtpNode->wtpWirelessIfMaxTxPwrLvl = txpower;
			WtpNode->wtpWirelessIfPwrAttRange = txpower;

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpWirelessIfMaxStationNumPermitted));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wifi_num));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->radio_count));	//xiaodawei add radiocount from wtpcompatible.xml, 20110124

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_recurse(&iter_struct,&iter_sub_array);

			for(j=0; j<WtpNode->wifi_num && j<WtpNode->radio_count; j++){  
				if((RadioNode = (struct Sub_WtpWirelessIfInfo*)malloc(sizeof(struct Sub_WtpWirelessIfInfo))) == NULL){
					dcli_free_wtp_WtpWirelessIf_Info(WtpHead);
					dbus_message_unref(reply);
					*ret = MALLOC_ERROR;
					return NULL;
				}
				
				memset(RadioNode,0,sizeof(struct Sub_WtpWirelessIfInfo));
				RadioNode->next = NULL;
				RadioNode->Sub_WtpWirelessIfInfo_list = NULL;
				RadioNode->Sub_WtpWirelessIfInfo_last = NULL;

				if(WtpNode->Sub_WtpWirelessIfInfo_head == NULL){
					WtpNode->Sub_WtpWirelessIfInfo_head = RadioNode;
				}
				else{
					WtpNode->Sub_WtpWirelessIfInfo_head->Sub_WtpWirelessIfInfo_last->next = RadioNode;
				}
				
				WtpNode->Sub_WtpWirelessIfInfo_head->Sub_WtpWirelessIfInfo_last = RadioNode;
				
				dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(RadioNode->wtpWirelessIfIndex));

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&RadioNode->g_radio_id);

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&Radio_Chan);

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&auto_channel_cont);

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&Radio_TXP);

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&txpowerautostate);

				if(auto_channel_cont==0){
					RadioNode->wtpWirelessIfRadioChannelAutoSelectEnable = 1;
				}
				else{
					RadioNode->wtpWirelessIfRadioChannelAutoSelectEnable = 2;
				}

				RadioNode->wtpWirelessIfRadioChannelConfig = Radio_Chan;
				RadioNode->wtpWirelessIfPwrAttValue = Radio_TXP;

				if(txpowerautostate==0){
					RadioNode->wtpWirelessIfPowerMgmtEnable = 1;
				}
				else if(txpowerautostate==1){
					RadioNode->wtpWirelessIfPowerMgmtEnable = 0;
				}

				RadioNode->wtpWirelessIfType = 41;
				RadioNode->wtpWirelessIfDiversitySelectionRx = 0;
				
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&RadioNode->wtpWirelessIfCurrRadioModeSupport);			//a5
#if 0
				switch(support_mode)
				{
					case 1:RadioNode->wtpWirelessIfCurrRadioModeSupport = 2;
						   break;
					case 2:RadioNode->wtpWirelessIfCurrRadioModeSupport = 4;
						   break;
					case 3:RadioNode->wtpWirelessIfCurrRadioModeSupport = 6;
						   break;
					case 6:RadioNode->wtpWirelessIfCurrRadioModeSupport = 16;
						   break;
					case 8:RadioNode->wtpWirelessIfCurrRadioModeSupport = 1;
						   break;
					case 9:RadioNode->wtpWirelessIfCurrRadioModeSupport = 3;
						   break;
					case 10:RadioNode->wtpWirelessIfCurrRadioModeSupport = 5;
						    break;
					case 11:RadioNode->wtpWirelessIfCurrRadioModeSupport = 7;
						    break;
					case 12:RadioNode->wtpWirelessIfCurrRadioModeSupport = 8;
						    break;
					default:RadioNode->wtpWirelessIfCurrRadioModeSupport = 0;
						    break;
				}
#endif
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&state);			//state

				if(1==state){
					RadioNode->wtpWirelessIfOperStatus = 1;
				}
				else{
					RadioNode->wtpWirelessIfOperStatus = 2;
				}
				
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&state_time);			//a5

				time(&now);
				online_time = now - state_time;									
				RadioNode->wtpWirelessIfLastChange = online_time*100;

				RadioNode->wtpWirelessIfPhysAddress = (unsigned char *)malloc(MAC_LEN +1);
				memset(RadioNode->wtpWirelessIfPhysAddress,0,(MAC_LEN +1));

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(RadioNode->radio_has_bss));	//b1

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(RadioNode->wtpWirelessIfPhysAddress[0]));
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(RadioNode->wtpWirelessIfPhysAddress[1]));
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(RadioNode->wtpWirelessIfPhysAddress[2]));
				
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(RadioNode->wtpWirelessIfPhysAddress[3]));
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(RadioNode->wtpWirelessIfPhysAddress[4]));
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(RadioNode->wtpWirelessIfPhysAddress[5]));
				
				unsigned char radio_state;
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(radio_state));		//b8
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(Radio_TXPSTEP));//zhaoruijia,20100917,add txpower step

				RadioNode->wtpWirelessIfTxPwrStep = Radio_TXPSTEP;
				RadioNode->wtpWirelessIfAdminStatus = radio_state;

				//printf("@@RadioNode->wtpWirelessIfTxPwrStep = %d@@\n",RadioNode->wtpWirelessIfTxPwrStep);

                /*
				if(RadioNode->radio_has_bss == 0){
					RadioNode->wtpWirelessIfAdminStatus = 2;
				}
				else{
					if(bss_state == 0)
						RadioNode->wtpWirelessIfAdminStatus = 2;
					else 
						RadioNode->wtpWirelessIfAdminStatus = 1;
				}*/
				/*---------------------------for rate begin---------------------------*/
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&RadioNode->surport_rate_num );	//for showing
					
				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_recurse(&iter_sub_struct,&iter_sub_sub_array);

				for(k=0; k<RadioNode->surport_rate_num; k++){  
					dbus_message_iter_recurse(&iter_sub_sub_array,&iter_sub_sub_struct);
					dbus_message_iter_get_basic(&iter_sub_sub_struct,&(RadioNode->wtpWirelessIfTransmitSpeedConfig[k]));
					dbus_message_iter_next(&iter_sub_sub_array);
				}
				
				if(RadioNode->surport_rate_num != 0 ){
					RadioNode->wtpWirelessIfSpeed = RadioNode->wtpWirelessIfTransmitSpeedConfig[0];
				}
				else{
					RadioNode->wtpWirelessIfSpeed = 0;
				}
				/*---------------------------for rate end---------------------------*/
				dbus_message_iter_next(&iter_sub_array);
			}
			dbus_message_iter_next(&iter_array);
		}
	}

	dbus_message_unref(reply);

	return WtpHead;
}
/* table 26 dot11NewWtpWirelessIfConfigTable  ,  for showting NewWtpWirelessIfInfo  by nl 20100603 b18*/
struct NewWtpWirelessIfInfo * show_NewWtpWirelessIfInfo_of_all_wtp(int index,int localid,DBusConnection *dcli_dbus_connection, unsigned int *total_radio_num, unsigned int *ret)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;	
	
	struct NewWtpWirelessIfInfo *RadioNode = NULL;
	struct NewWtpWirelessIfInfo *RadioHead = NULL;
	struct NewWtpWirelessIfInfo *RadioSearchNode = NULL;
	
	int i;
	int j;
	unsigned int wtp_num = 0;
	unsigned char wtp_radio_num = 0;
	int wtpCurrID;
	*total_radio_num = 0;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,\
						INTERFACE,WID_DBUS_CONF_METHOD_SHOW_ALL_WTP_NEW_WTPWIRELESS_IF_INFORMATION);
	
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		*ret = WID_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	if(*ret == 0){
		if((RadioHead = (struct NewWtpWirelessIfInfo*)malloc(sizeof(struct NewWtpWirelessIfInfo))) == NULL){
			*ret = MALLOC_ERROR;
			dbus_message_unref(reply);
			return NULL;
		}
		memset(RadioHead,0,sizeof(struct NewWtpWirelessIfInfo));
		RadioHead->NewWtpWirelessIfInfo_list = NULL;
		RadioHead->NewWtpWirelessIfInfo_last = NULL;
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&wtp_num);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
		
		for (i = 0; i < wtp_num; i++) {
			DBusMessageIter iter_struct;
			DBusMessageIter iter_sub_array; 	
			
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(wtpCurrID));
		
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(wtp_radio_num));	

			*total_radio_num = (*total_radio_num) + (unsigned int)wtp_radio_num;
			
			/*for radio information*/
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_recurse(&iter_struct,&iter_sub_array);

			for(j=0; j<wtp_radio_num; j++){	
				DBusMessageIter iter_sub_struct;
				if((RadioNode = (struct NewWtpWirelessIfInfo*)malloc(sizeof(struct NewWtpWirelessIfInfo))) == NULL){
					dcli_free_NewWtpWirelessIfInfo(RadioHead);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
				}
				
				memset(RadioNode,0,sizeof(struct NewWtpWirelessIfInfo));
				RadioNode->next = NULL;

				if(RadioHead->NewWtpWirelessIfInfo_list == NULL){
					RadioHead->NewWtpWirelessIfInfo_list = RadioNode;
					//RadioHead->next = RadioNode;
				}else{
					RadioHead->NewWtpWirelessIfInfo_last->next = RadioNode;
					
				}
				RadioHead->NewWtpWirelessIfInfo_last = RadioNode;

				RadioNode->wtpMacAddr = (unsigned char *)malloc(MAC_LEN +1);
				memset(RadioNode->wtpMacAddr,0,(MAC_LEN +1));

				dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(RadioNode->wtpRadCurrID));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(RadioNode->wtpRadLocalID));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(RadioNode->wtpMacAddr[0]));
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(RadioNode->wtpMacAddr[1]));
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(RadioNode->wtpMacAddr[2]));
				
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(RadioNode->wtpMacAddr[3]));
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(RadioNode->wtpMacAddr[4]));
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(RadioNode->wtpMacAddr[5]));

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(RadioNode->NewFragThreshlod));		//short
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(RadioNode->NewwtpWirelessIfBeaconIntvl));		//short
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(RadioNode->NewWtpPreambleLen));	//lchar

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(RadioNode->NewwtpWirelessIfDtimIntvl));			//char
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(RadioNode->NewRtsThreshold));		//short
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(RadioNode->NewwtpWirelessIfShtRetryThld));			//char
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(RadioNode->NewwtpWirelessIfLongRetryThld));			//char

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(RadioNode->wtpWirelessIfMaxRxLifetime));			//unsigned int /t31

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(RadioNode->wtpWiredIfOutMulticastPkts));			//t27
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(RadioNode->wtpWiredIfInMulticastPkts));			//t27

				RadioNode->wtpCurrID = wtpCurrID;
				RadioNode->NewapWirelessIfIndex = RadioNode->wtpRadLocalID + 1;
				//RadioNode->NewwtpWirelessIfMaxRxLifetime = 7;
				RadioNode->NewwtpWirelessIfMaxRxLifetime = RadioNode->wtpWirelessIfMaxRxLifetime;

				dbus_message_iter_next(&iter_sub_array);
			}
			dbus_message_iter_next(&iter_array);
		}
	}
	dbus_message_unref(reply);

	return RadioHead;
}
/*table 34 b19*/
struct NewWtpWirelessIfstatsInfo * show_New_wirelessifstatsInfo_info_of_all_wtp(int index,int localid,DBusConnection *dcli_dbus_connection, unsigned int *wtp_num, unsigned int *ret)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;	
	
	struct NewWtpWirelessIfstatsInfo *WtpNode = NULL;
	struct NewWtpWirelessIfstatsInfo *WtpHead = NULL;
	struct NewWtpWirelessIfstatsInfo *WtpSearchNode = NULL;
	struct NewWtpWirelessIfstatsInfo_radio *sub_radio_node = NULL;
	struct NewWtpWirelessIfstatsInfo_radio *sub_radio_search = NULL;
	
	int i,j,k;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
 
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,\
						INTERFACE,WID_DBUS_CONF_METHOD_SHOW_ALL_NEW_WTP_WIRELESS_IFSTATS_INFORMATION);
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		*ret = WID_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	if(*ret == 0){
		if((WtpHead = (struct NewWtpWirelessIfstatsInfo*)malloc(sizeof(struct NewWtpWirelessIfstatsInfo))) == NULL){
			*ret = MALLOC_ERROR;
			dbus_message_unref(reply);
			return NULL;
		}
		memset(WtpHead,0,sizeof(struct NewWtpWirelessIfstatsInfo));
		WtpHead->NewWtpWirelessIfstatsInfo_list = NULL;
		WtpHead->NewWtpWirelessIfstatsInfo_last = NULL;
		WtpHead->NewWtpWirelessIfstatsInfo_radio_head = NULL;
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,wtp_num);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
		
		for (i = 0; i < *wtp_num; i++) {
			DBusMessageIter iter_struct;
			DBusMessageIter iter_sub_array; 	
			if((WtpNode = (struct NewWtpWirelessIfstatsInfo*)malloc(sizeof(struct NewWtpWirelessIfstatsInfo))) == NULL){
					dcli_free_new_wtp_wireless_ifstats_Info(WtpHead);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(WtpNode,0,sizeof(struct NewWtpWirelessIfstatsInfo));
			WtpNode->next = NULL;
			WtpNode->NewWtpWirelessIfstatsInfo_radio_head = NULL;

			WtpNode->wtpMacAddr = (unsigned char *)malloc(MAC_LEN +1);
			memset(WtpNode->wtpMacAddr,0,(MAC_LEN +1));

			if(WtpHead->NewWtpWirelessIfstatsInfo_list == NULL){
				WtpHead->NewWtpWirelessIfstatsInfo_list = WtpNode;
				WtpHead->next = WtpNode;
			}
			else{
				WtpHead->NewWtpWirelessIfstatsInfo_last->next = WtpNode;
			}
			WtpHead->NewWtpWirelessIfstatsInfo_last = WtpNode;
			
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpCurrID));
		
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[0]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[1]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[2]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[3]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[4]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[5]));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wifiExtensionInfoReportswitch));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wirelessIfHighestRxSignalStrength));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wirelessIfLowestRxSignalStrength));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wirelessIfAvgRxSignalStrength));

			unsigned char another_wtpWirelessIfIndex = 0;
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(another_wtpWirelessIfIndex));
			/*packets information*/
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wirelessIfRxDataPkts));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wirelessIfTxDataPkts));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wirelessIfChStatsFrameErrorCnt));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wirelessIfDwlinkDataOctets));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wirelessIfUplinkDataOctets));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wirelessIfChStatsMacFcsErrPkts));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wirelessIfChStatsMacDecryptErrPkts));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wirelessIfChStatsMacMicErrPkts));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wirelessIfChStatsPhyErrPkts));
			/*for signal information*/
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wirelessIfTxSignalPkts));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wirelessIfRxSignalPkts));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wirelessIfChStatsFrameRetryCnt));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wirelessIfChStatsDwlinkTotRetryPkts));
			/*for radio information*/
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtp_radio_num));	

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_recurse(&iter_struct,&iter_sub_array);
			
			for(j=0; j<WtpNode->wtp_radio_num; j++){	
				DBusMessageIter iter_sub_struct;
				if((sub_radio_node = (struct NewWtpWirelessIfstatsInfo_radio*)malloc(sizeof(struct NewWtpWirelessIfstatsInfo_radio))) == NULL){
					free(sub_radio_node);
					sub_radio_node =NULL;
					dcli_free_new_wtp_wireless_ifstats_Info(WtpHead);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
				}
			
				memset(sub_radio_node,0,sizeof(struct NewWtpWirelessIfstatsInfo_radio));
				sub_radio_node->next = NULL;
				sub_radio_node->NewWtpWirelessIfstatsInfo_radio_list = NULL;
				sub_radio_node->NewWtpWirelessIfstatsInfo_radio_last = NULL;

				if(WtpNode->NewWtpWirelessIfstatsInfo_radio_head == NULL){
					WtpNode->NewWtpWirelessIfstatsInfo_radio_head = sub_radio_node;
				}
				else{
					WtpNode->NewWtpWirelessIfstatsInfo_radio_head->NewWtpWirelessIfstatsInfo_radio_last->next = sub_radio_node;
				}
				WtpNode->NewWtpWirelessIfstatsInfo_radio_head->NewWtpWirelessIfstatsInfo_radio_last = sub_radio_node;

				dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->wtpWirelessIfIndex));	

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->wirelessIfUpdownTimes));	
				
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->wirelessIfChStatsUplinkUniFrameCnt));
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->wirelessIfChStatsDwlinkUniFrameCnt));	
				
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->wirelessIfUpChStatsFrameNonUniFrameCnt));	
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->wirelessIfDownChStatsFrameNonUniFrameCnt));	
				/*fengwenchao add begin 20110617*/
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->wirelessIfRxDataFrameCnt));
				printf(" wirelessIfRxDataFrameCnt  =  %d \n  ",sub_radio_node->wirelessIfRxDataFrameCnt);
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->wirelessIfTxDataFrameCnt));
				printf(" wirelessIfTxDataFrameCnt  =  %d \n  ",sub_radio_node->wirelessIfTxDataFrameCnt);
				/*fengwenchao add end*/
				sub_radio_node->wirelessIfApChStatsFrameFragRate = 0;

				dbus_message_iter_next(&iter_sub_struct);	 
				dbus_message_iter_get_basic(&iter_sub_struct,&(sub_radio_node->radioWlanNum));  
				
 				DBusMessageIter iter_sub_sub_array; 	
 				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_recurse(&iter_sub_struct,&iter_sub_sub_array);
				
				for(k=0;k<sub_radio_node->radioWlanNum;k++){
					unsigned char radioBindWlan;
					DBusMessageIter iter_sub_sub_struct;
					dbus_message_iter_recurse(&iter_sub_sub_array,&iter_sub_sub_struct);
					dbus_message_iter_get_basic(&iter_sub_sub_struct,&(radioBindWlan));	
					sub_radio_node->radioBindWlan[k] = radioBindWlan;
					dbus_message_iter_next(&iter_sub_sub_array);
				}
				dbus_message_iter_next(&iter_sub_array);
			}
			dbus_message_iter_next(&iter_array);
		}
	}
	else{
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return NULL;
	}
	dbus_message_unref(reply);

/*receive ASD massege in this function*/
	DBusMessage *query2, *reply2; 
	DBusError err2;
	unsigned int ret2;
	unsigned int asd_wtp_num = 0;
	unsigned int asd_radio_num = 0;
	unsigned int asd_radio_id = 0;
	unsigned int asd_wtp_id = 0;	

	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	
	query2 = dbus_message_new_method_call(BUSNAME,OBJPATH,\
						INTERFACE,ASD_DBUS_STA_METHOD_SHOW_RADIO_NEW_WIRELESS_INFO_BYWTPID);
	
	dbus_error_init(&err2);

	reply2 = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query2,-1, &err2);
	
	dbus_message_unref(query2);
	if (NULL == reply2) {
		if (dbus_error_is_set(&err2)) {
			dbus_error_free(&err2);
		}
		dcli_free_new_wtp_wireless_ifstats_Info(WtpHead);
		return NULL;
	}
	dbus_message_iter_init(reply2,&iter);
	dbus_message_iter_get_basic(&iter,&ret2);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(asd_wtp_num));

	dbus_message_iter_next(&iter);
	dbus_message_iter_recurse(&iter,&iter_array);

	if(ret2 == 0){	
		for(j=0;j<asd_wtp_num;j++){
			DBusMessageIter	 iter_struct;
			DBusMessageIter	 iter_sub_array;	

			dbus_message_iter_recurse(&iter_array,&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(asd_wtp_id));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(asd_radio_num));
			
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_recurse(&iter_struct,&iter_sub_array);

			for(i=0; i<asd_radio_num; i++) {		
				DBusMessageIter  iter_sub_struct;
				unsigned int auth_tms=0;
				unsigned int repauth_tms=0;
				unsigned int assoc_req_num = 0; 
				unsigned int assoc_resp_num = 0; 
				unsigned int rx_mgmt_pkts = 0;
				
				unsigned int tx_mgmt_pkts = 0;
				unsigned int rx_ctrl_pkts = 0;
				unsigned int tx_ctrl_pkts = 0;
				unsigned int rx_data_pkts = 0;
				unsigned int tx_data_pkts = 0;
				unsigned int sta_num = 0;
	
				dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(asd_radio_id));// a0
				
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(auth_tms));// a1
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(repauth_tms));//
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(assoc_req_num));//
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(assoc_resp_num));//
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(rx_mgmt_pkts));// a5

 				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(tx_mgmt_pkts));// a6
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(rx_ctrl_pkts));//
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(tx_ctrl_pkts));//
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(rx_data_pkts));//
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(tx_data_pkts));// a10

				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(sta_num));//

				WtpSearchNode = WtpHead->NewWtpWirelessIfstatsInfo_list;

				while(WtpSearchNode!= NULL){
					if(WtpSearchNode->wtpCurrID == asd_wtp_id){
						if(WtpSearchNode->NewWtpWirelessIfstatsInfo_radio_head == NULL)
							break;
						else{
							WtpSearchNode->asd_radio_num= asd_radio_num ;
							sub_radio_search = WtpSearchNode->NewWtpWirelessIfstatsInfo_radio_head;
							while(sub_radio_search !=NULL){
								
								unsigned char asd_radio_local_id = asd_radio_id%4;
								if(sub_radio_search->wtpWirelessIfIndex == asd_radio_local_id){
									sub_radio_search->wirelessIfRxMgmtFrameCnt = rx_mgmt_pkts;
									sub_radio_search->wirelessIfRxCtrlFrameCnt = rx_ctrl_pkts;
									sub_radio_search->wirelessIfRxDataFrameCnt = rx_data_pkts;
									sub_radio_search->wirelessIfRxAuthenFrameCnt = auth_tms;
									sub_radio_search->wirelessIfRxAssociateFrameCnt = assoc_req_num;
									
									sub_radio_search->wirelessIfTxMgmtFrameCnt = tx_mgmt_pkts;
									sub_radio_search->wirelessIfTxCtrlFrameCnt = tx_ctrl_pkts;
									sub_radio_search->wirelessIfTxDataFrameCnt = tx_data_pkts;
									sub_radio_search->wirelessIfTxAuthenFrameCnt = repauth_tms;
									sub_radio_search->wirelessIfTxAssociateFrameCnt = assoc_resp_num;
									sub_radio_search->wirelessIfChStatsNumStations = sta_num;
								}
								sub_radio_search = sub_radio_search->next;
							}
						}
					}
					WtpSearchNode = WtpSearchNode->next;
				}
				dbus_message_iter_next(&iter_sub_array);
			}
			dbus_message_iter_next(&iter_array);
		}
	}
	
	dbus_message_unref(reply2);
	return WtpHead;
}
/*for showring table 29 dot11RogueAPTable  RogueAPInfo by nl 20100610 b20*/
struct RogueAPInfo * show_RogueAPInfo_of_all_wtp(int index,int localid,DBusConnection *dcli_dbus_connection, unsigned int *num, unsigned int *ret)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;	
	DBusMessageIter	 iter_struct;
	DBusMessageIter	 iter_sub_array;	
	DBusMessageIter  iter_sub_struct;
	DBusMessageIter	 iter_sub_essid_array;	
	DBusMessageIter  iter_sub_essid_struct;
	struct RogueAPInfo  *WtpNode = NULL;
	struct RogueAPInfo  *WtpHead = NULL;
	struct RogueAPInfo  *WtpSearchNode = NULL;
	struct Sub_RogueAPInfo *Sub_Ap_Node = NULL;	
	
	int i,j,l;
	char *rogueAPEssid = (char *)malloc(ESSID_DEFAULT_LEN);
	if(!rogueAPEssid){
		printf("%s,%d,rogueAPEssid malloc error\n",__func__,__LINE__);
		return NULL;
	}
	memset(rogueAPEssid,0,ESSID_DEFAULT_LEN);
	char *rogueAPElemInfo = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,\
						INTERFACE,WID_DBUS_CONF_METHOD_SHOW_ALL_ROGUE_WTP_INFORMATION);
	
	dbus_error_init(&err);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		*ret = WID_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	if(*ret == 0){
		if((WtpHead = (struct RogueAPInfo*)malloc(sizeof(struct RogueAPInfo))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
		}
		
		memset(WtpHead,0,sizeof(struct RogueAPInfo));
		WtpHead->RogueAPInfo_list = NULL;
		WtpHead->RogueAPInfo_last = NULL;
			
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,num);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
			
		for (i = 0; i < *num; i++) {
			if((WtpNode = (struct RogueAPInfo*)malloc(sizeof(struct RogueAPInfo))) == NULL){
					dcli_free_wtp_RogueAPInfo_Info(WtpHead);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(WtpNode,0,sizeof(struct RogueAPInfo));
			WtpNode->next = NULL;

			WtpNode->wtpMacAddr = (unsigned char *)malloc(MAC_LEN +1);
			memset(WtpNode->wtpMacAddr,0,(MAC_LEN +1));

			if(WtpHead->RogueAPInfo_list == NULL){
				WtpHead->RogueAPInfo_list = WtpNode;
			}
			else{
				WtpHead->RogueAPInfo_last->next = WtpNode;
			}
			
			WtpHead->RogueAPInfo_last = WtpNode;
			
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpCurrID));
		
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[0]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[1]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[2]));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[3]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[4]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[5]));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->rogue_ap_num));

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_recurse(&iter_struct,&iter_sub_array);

			for(j=0;j<WtpNode->rogue_ap_num;j++){
				if((Sub_Ap_Node = (struct Sub_RogueAPInfo*)malloc(sizeof(struct Sub_RogueAPInfo))) == NULL){
					dcli_free_wtp_RogueAPInfo_Info(WtpHead);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
				}
			
				memset(Sub_Ap_Node,0,sizeof(struct Sub_RogueAPInfo));
				Sub_Ap_Node->next = NULL;
				Sub_Ap_Node->Sub_RogueAPInfo_list = NULL;
				Sub_Ap_Node->Sub_RogueAPInfo_last = NULL;

				if(WtpNode->Sub_RogueAPInfo_head == NULL){
					WtpNode->Sub_RogueAPInfo_head = Sub_Ap_Node;
				}
				else{
					WtpNode->Sub_RogueAPInfo_head->Sub_RogueAPInfo_last->next = Sub_Ap_Node;
				}
				
				WtpNode->Sub_RogueAPInfo_head->Sub_RogueAPInfo_last = Sub_Ap_Node;

				dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
				
				dbus_message_iter_get_basic(&iter_sub_struct,&(Sub_Ap_Node->rogueAPMac[0]));
				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(Sub_Ap_Node->rogueAPMac[1]));
				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(Sub_Ap_Node->rogueAPMac[2]));
				
				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(Sub_Ap_Node->rogueAPMac[3]));
				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(Sub_Ap_Node->rogueAPMac[4]));
				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(Sub_Ap_Node->rogueAPMac[5]));

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(Sub_Ap_Node->rogueAPIndex));				//short
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(Sub_Ap_Node->rogueAPRate));				//short
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(Sub_Ap_Node->rogueAPChannel));			//char
				
				dbus_message_iter_next(&iter_sub_struct);				
				dbus_message_iter_get_basic(&iter_sub_struct,&(Sub_Ap_Node->rogueAPRssi));				//char
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(Sub_Ap_Node->rogueAPNoise));			//char
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(Sub_Ap_Node->rogueAPBeaconInterval));		//char
				
				dbus_message_iter_next(&iter_sub_struct);	
				dbus_message_iter_get_basic(&iter_sub_struct,&(Sub_Ap_Node->rogueAPCapability));	//short
				dbus_message_iter_next(&iter_sub_struct);
				  dbus_message_iter_recurse(&iter_sub_struct,&iter_sub_essid_array);
				  for(l=0;l<ESSID_DEFAULT_LEN;l++){
					  dbus_message_iter_recurse(&iter_sub_essid_array,&iter_sub_essid_struct);
					  dbus_message_iter_get_basic(&iter_sub_essid_struct,&rogueAPEssid[l]);
					   dbus_message_iter_next(&iter_sub_essid_struct);
					  dbus_message_iter_next(&iter_sub_essid_array);
				  }
				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(rogueAPElemInfo));			//char *
				
				//Sub_Ap_Node->rogueAPEssid = (char*)malloc(strlen(rogueAPEssid)+1);
				memset(Sub_Ap_Node->rogueAPEssid, 0, strlen(rogueAPEssid)+1);
				memcpy(Sub_Ap_Node->rogueAPEssid, rogueAPEssid, strlen(rogueAPEssid));	
				
				Sub_Ap_Node->rogueAPElemInfo = (char*)malloc(strlen(rogueAPElemInfo)+1);
				memset(Sub_Ap_Node->rogueAPElemInfo,0,strlen(rogueAPElemInfo)+1);
				memcpy(Sub_Ap_Node->rogueAPElemInfo,rogueAPElemInfo,strlen(rogueAPElemInfo));

				Sub_Ap_Node->wtpID = WtpNode->wtpCurrID;
				
				dbus_message_iter_next(&iter_sub_array);
			}
			dbus_message_iter_next(&iter_array);
		}
	}
	dbus_message_unref(reply);
	if(rogueAPEssid){
		free(rogueAPEssid);
		rogueAPEssid = NULL;
	}
	return WtpHead;
}

/*for table 30 dot11SecurityMechTable :showting security info by wtp nl 20100617 b21*/
struct SecurityMechInfo* show_SecurityMechInfo_of_all_wtp(int index,int localid,DBusConnection *dcli_dbus_connection, unsigned int *num, unsigned int *ret)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;	
	DBusMessageIter iter_struct;
	DBusMessageIter iter_sub_array; 	
	DBusMessageIter iter_sub_struct;
	
	struct SecurityMechInfo *WtpNode = NULL;
	struct SecurityMechInfo *WtpHead = NULL;
	struct SecurityMechInfo *WtpSearchNode = NULL;
	struct Sub_SecurityMechInfo *SubWlanNode = NULL;
	int i, j;

	char *Security_key = NULL;
	char *Security_name = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,\
							INTERFACE,ASD_DBUS_STA_METHOD_SHOW_SECURITY_INFO_BYWTPID);
	
	dbus_error_init(&err);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);	
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(*ret == 0){
		if((WtpHead = (struct SecurityMechInfo*)malloc(sizeof(struct SecurityMechInfo))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
		memset(WtpHead,0,sizeof(struct SecurityMechInfo));
		WtpHead->SecurityMechInfo_list = NULL;
		WtpHead->SecurityMechInfo_last = NULL;
			
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,num);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);

		for (i = 0; i < *num; i++) {
			if((WtpNode = (struct SecurityMechInfo*)malloc(sizeof(struct SecurityMechInfo))) == NULL){
					dcli_free_SecurityMechInfo(WtpHead);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(WtpNode,0,sizeof(struct SecurityMechInfo));
			WtpNode->next = NULL;

			WtpNode->wtpMacAddr = (unsigned char *)malloc(MAC_LEN +1);
			memset(WtpNode->wtpMacAddr,0,(MAC_LEN +1));

			if(WtpHead->SecurityMechInfo_list == NULL){
				WtpHead->SecurityMechInfo_list = WtpNode;
				WtpHead->next = WtpNode;
			}else{
				WtpHead->SecurityMechInfo_last->next = WtpNode;
				
			}
			WtpHead->SecurityMechInfo_last = WtpNode;
			
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpCurrID));
		
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[0]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[1]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[2]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[3]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[4]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[5]));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wlan_num));

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_recurse(&iter_struct,&iter_sub_array);
			for(j=0;j<WtpNode->wlan_num;j++){
				if((SubWlanNode = (struct Sub_SecurityMechInfo*)malloc(sizeof(struct Sub_SecurityMechInfo))) == NULL){
					dcli_free_SecurityMechInfo(WtpHead);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
				}
			
				memset(SubWlanNode,0,sizeof(struct Sub_SecurityMechInfo));
				SubWlanNode->next = NULL;
				SubWlanNode->Sub_SecurityMechInfo_list = NULL;
				SubWlanNode->Sub_SecurityMechInfo_last = NULL;

				if(WtpNode->Sub_SecurityMechInfo_head == NULL){
					WtpNode->Sub_SecurityMechInfo_head = SubWlanNode;
				}
				else{
					WtpNode->Sub_SecurityMechInfo_head->Sub_SecurityMechInfo_last->next = SubWlanNode;
				}
				
				WtpNode->Sub_SecurityMechInfo_head->Sub_SecurityMechInfo_last = SubWlanNode;

				dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(SubWlanNode->wlanCurrID));
					
				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,
											&(SubWlanNode->wlanSecID));	//unsigned char
											
				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,
											&(SubWlanNode->wtpWirelessSecurMechType));//unsigned int
				
				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,
											&(SubWlanNode->wtpWirelessSecurMechEncryType));//unsigned int
											
				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,
											&(SubWlanNode->wtpWirelessSecurMechKeyInputType));//unsigned int
				
				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(Security_key));//char *
				
				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&(Security_name));//char *

				SubWlanNode->wtpWirelessSecurMechSecurPolicyKey = (char*)malloc(strlen(Security_key)+1);
				memset(SubWlanNode->wtpWirelessSecurMechSecurPolicyKey, 0, strlen(Security_key)+1);
				memcpy(SubWlanNode->wtpWirelessSecurMechSecurPolicyKey, Security_key, strlen(Security_key));	
				
				SubWlanNode->wtpWirelessSecurMechName = (char*)malloc(strlen(Security_name)+1);
				memset(SubWlanNode->wtpWirelessSecurMechName,0,strlen(Security_name)+1);
				memcpy(SubWlanNode->wtpWirelessSecurMechName,Security_name,strlen(Security_name));

				SubWlanNode->wtpWirelessSecurMechID = SubWlanNode->wlanSecID;
				
				dbus_message_iter_next(&iter_sub_array);
			}
			dbus_message_iter_next(&iter_array);
		}
	}
	dbus_message_unref(reply);

	return WtpHead;
}

struct WtpAthStatisticInfo * show_ath_statistics_info_of_all_wtp(int index,int localid,DBusConnection *dcli_dbus_connection, unsigned int *wtp_num, unsigned int *ret)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	iter;
	DBusMessageIter	iter_wtp_array;
	DBusMessageIter iter_wtp;
	DBusMessageIter iter_ath_array;
	DBusMessageIter iter_ath;
	
	struct WtpAthStatisticInfo *WtpNode = NULL;
	struct WtpAthStatisticInfo *WtpHead = NULL;
	struct WtpAthStatisticInfo *WtpTail = NULL;
	struct WtpAthStatisticInfo *WtpSearchNode = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	//wid--dbus--operation
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,\
						INTERFACE,WID_DBUS_CONF_METHOD_SHOW_ALL_WTP_ATH_STATISTICS_INFOMATION);

	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		*ret = WID_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,wtp_num);
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_recurse(&iter,&iter_wtp_array);


	unsigned wtp_mac=NULL;
	unsigned wtp_id=0;
	unsigned ath_num=0;
	char  NOISE_VALUE = 95 ;     //fengwenchao add 20101228
	double D_NOISE_VALUE = 95;   //fengwenchao add 20101228

	
	if(*ret==0 && *wtp_num >0){
		int i=0;
		for(i=0;i<*wtp_num;i++){
			dbus_message_iter_recurse(&iter_wtp_array,&iter_wtp);
			dbus_message_iter_get_basic(&iter_wtp,&wtp_id);

			dbus_message_iter_next(&iter_wtp);
			dbus_message_iter_get_basic(&iter_wtp,&wtp_mac);
			dbus_message_iter_next(&iter_wtp);
			dbus_message_iter_get_basic(&iter_wtp,&ath_num);

			dbus_message_iter_next(&iter_wtp);
			dbus_message_iter_recurse(&iter_wtp,&iter_ath_array);

			//printf("dcli :: ath num =%d\n",ath_num);
			int j=0;
			for(j=0;j<ath_num;j++){
				if((WtpNode = (struct WtpAthStatisticInfo*)malloc(sizeof(struct WtpAthStatisticInfo))) == NULL){
							dcli_free_WtpAthStatisticInfo(WtpHead);
							*ret = MALLOC_ERROR;
							dbus_message_unref(reply);
							return NULL;
					}
				memset(WtpNode,0,sizeof(struct WtpAthStatisticInfo));
				if(WtpHead == NULL){
					WtpHead = WtpNode;
					WtpTail = WtpNode;
				}else{
					WtpTail->next=WtpNode;
					WtpTail=WtpNode;
				}
				dbus_message_iter_recurse(&iter_ath_array,&iter_ath);
				dbus_message_iter_get_basic(&iter_ath,&(WtpNode->wtpwirelessifindex));
				
				dbus_message_iter_next(&iter_ath);
				dbus_message_iter_get_basic(&iter_ath,&WtpNode->wlanid);
				
				dbus_message_iter_next(&iter_ath);
				dbus_message_iter_get_basic(&iter_ath,&WtpNode->wirelessIfUpdownTimes);

				dbus_message_iter_next(&iter_ath);
				dbus_message_iter_get_basic(&iter_ath,&(WtpNode->wirelessIfTxSignalPkts));
				
				dbus_message_iter_next(&iter_ath);
				dbus_message_iter_get_basic(&iter_ath,&(WtpNode->wirelessIfRxSignalPkts));
				
				dbus_message_iter_next(&iter_ath);
				dbus_message_iter_get_basic(&iter_ath,&(WtpNode->wirelessIfTxDataPkts));
				
				dbus_message_iter_next(&iter_ath);
				dbus_message_iter_get_basic(&iter_ath,&(WtpNode->wirelessIfRxDataPkts));
					
				dbus_message_iter_next(&iter_ath);
				dbus_message_iter_get_basic(&iter_ath,&(WtpNode->wirelessIfUplinkDataOctets));
				
				dbus_message_iter_next(&iter_ath);
				dbus_message_iter_get_basic(&iter_ath,&(WtpNode->wirelessIfDwlinkDataOctets));
				
				dbus_message_iter_next(&iter_ath);
				dbus_message_iter_get_basic(&iter_ath,&(WtpNode->wirelessIfChStatsDwlinkTotRetryPkts));
				
				dbus_message_iter_next(&iter_ath);
				dbus_message_iter_get_basic(&iter_ath,&(WtpNode->wirelessIfChStatsFrameRetryCnt));

				dbus_message_iter_next(&iter_ath);
				dbus_message_iter_get_basic(&iter_ath,&(WtpNode->wirelessifUplinkUniPktCnt));

				dbus_message_iter_next(&iter_ath);
				dbus_message_iter_get_basic(&iter_ath,&(WtpNode->wirelessifDwlinkUniPktCnt));

				dbus_message_iter_next(&iter_ath);
				dbus_message_iter_get_basic(&iter_ath,&(WtpNode->wirelessifUpNonUniPktCnt));

				dbus_message_iter_next(&iter_ath);
				dbus_message_iter_get_basic(&iter_ath,&(WtpNode->wirelessifDownNonUniPktCnt));
				WtpNode->wirelessIfUplinkTotFrameCnt = WtpNode->wirelessifUplinkUniPktCnt+WtpNode->wirelessifUpNonUniPktCnt;
				WtpNode->wirelessIfDwlinkTotFrameCnt = WtpNode->wirelessifDwlinkUniPktCnt+WtpNode->wirelessifDownNonUniPktCnt;
				
				dbus_message_iter_next(&iter_ath);
				dbus_message_iter_get_basic(&iter_ath,&(WtpNode->wirelessifUplinkCtrlOctets));
				
				dbus_message_iter_next(&iter_ath);
				dbus_message_iter_get_basic(&iter_ath,&(WtpNode->wirelessifDownlinkCtrlOctets));

				
				dbus_message_iter_next(&iter_ath);
				dbus_message_iter_get_basic(&iter_ath,&(WtpNode->wirelessIfChStatsMacFcsErrPkts));
				dbus_message_iter_next(&iter_ath);
				dbus_message_iter_get_basic(&iter_ath,&(WtpNode->wirelessIfChStatsMacMicErrPkts));
				dbus_message_iter_next(&iter_ath);
				dbus_message_iter_get_basic(&iter_ath,&(WtpNode->wirelessIfChStatsMacDecryptErrPkts));
				dbus_message_iter_next(&iter_ath);
				dbus_message_iter_get_basic(&iter_ath,&(WtpNode->wirelessIfChStatsFrameErrorCnt));

				
				dbus_message_iter_next(&iter_ath);
				dbus_message_iter_get_basic(&iter_ath,&WtpNode->wirelessifRxDataFrameCnt);
				dbus_message_iter_next(&iter_ath);
				dbus_message_iter_get_basic(&iter_ath,&WtpNode->wirelessifTxDataFrameCnt);


				dbus_message_iter_next(&iter_ath);
				dbus_message_iter_get_basic(&iter_ath,&WtpNode->wirelessIfRxMgmtFrameCnt);
				dbus_message_iter_next(&iter_ath);
				dbus_message_iter_get_basic(&iter_ath,&WtpNode->wirelessIfTxMgmtFrameCnt);

				dbus_message_iter_next(&iter_ath);
				dbus_message_iter_get_basic(&iter_ath,&WtpNode->wirelessifRxCtrlFrameCnt);
				dbus_message_iter_next(&iter_ath);
				dbus_message_iter_get_basic(&iter_ath,&WtpNode->wirelessifTxCtrlFrameCnt);

				//fengwenchao add 20101228
				dbus_message_iter_next(&iter_ath);
				dbus_message_iter_get_basic(&iter_ath,&WtpNode->wirelessIfChStatsPhyErrPkts);
				//fengwenchao add end

				//fengwenchao add 20101228
				dbus_message_iter_next(&iter_ath);
				dbus_message_iter_get_basic(&iter_ath,&WtpNode->wirelessIfHighestRxSignalStrength);

				dbus_message_iter_next(&iter_ath);
				dbus_message_iter_get_basic(&iter_ath,&WtpNode->wirelessIfLowestRxSignalStrength);

				dbus_message_iter_next(&iter_ath);
				dbus_message_iter_get_basic(&iter_ath,&WtpNode->wirelessIfAvgRxSignalStrength);

				char radio_max_snr2 = (char)WtpNode->wirelessIfHighestRxSignalStrength;
				char radio_min_snr2 = (char)WtpNode->wirelessIfLowestRxSignalStrength;
				double radio_aver_snr = WtpNode->wirelessIfAvgRxSignalStrength;
				

				WtpNode->wirelessIfHighestRxSignalStrength2 = 
					((radio_max_snr2 == 0)? 0:(radio_max_snr2 - NOISE_VALUE));

				WtpNode->wirelessIfLowestRxSignalStrength2 = 
					((radio_min_snr2 == 0) ? 0 : (radio_min_snr2 - NOISE_VALUE));

				WtpNode->wirelessIfAvgRxSignalStrength = 
					((radio_aver_snr ==0) ? 0:(radio_aver_snr -D_NOISE_VALUE));
				
				//fengwenchao add end				

				WtpNode->wtpwirelessifindex ++;
				WtpNode->wtpCurrID = wtp_id;
				if(!(WtpNode->wtpMacAddr=(unsigned char*)malloc(strlen(wtp_mac)+1))){
						dcli_free_WtpAthStatisticInfo(WtpHead);
						*ret = MALLOC_ERROR;
						dbus_message_unref(reply);
						return NULL;
					}
				memset(WtpNode->wtpMacAddr,0,strlen(wtp_mac)+1);
				strncpy(WtpNode->wtpMacAddr,wtp_mac,strlen(wtp_mac));
			
				dbus_message_iter_next(&iter_ath_array);
			}
			
			dbus_message_iter_next(&iter_wtp_array);
		}
	}
	dbus_message_unref(reply);
	
	/*xiaodawei modify for ASDDbus operation, add AssociateFrame and AuthenFrame, 20101230*/
	//mahz changed to match wid , 2011.1.26
	DBusMessage *query2, *reply2;
	DBusError err2;
	DBusMessageIter	 iter_array;
	unsigned int ret2;
	unsigned int asd_wtp_num = 0;
	unsigned int asd_wtp_id = 0;
	unsigned int radio_num = 0;
	unsigned int wirelessifNumStations = 0;
	unsigned int auth_tms = 0;
	unsigned int repauth_tms = 0;
	unsigned int assoc_req_num = 0;
	unsigned int assoc_resp_num = 0;
	unsigned int assoc_success = 0;
	unsigned int acc_tms = 0;
	unsigned int i = 0;
	unsigned int j = 0;
	unsigned int k = 0;
	unsigned int bss_num = 0;
	unsigned char wlan_id;
	unsigned int radio_id;
	//qiuchen
	unsigned int sta_online_time = 0;
	unsigned int sta_drop_cnt = 0;
	unsigned int auth_req_cnt = 0, auth_suc_cnt = 0, auth_fail_cnt = 0;
	DBusMessageIter iter_struct;
	DBusMessageIter iter_sub_array;
	DBusMessageIter iter_sub_struct;
	DBusMessageIter iter_bss_array;
	DBusMessageIter iter_bss_struct;
	
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query2 = dbus_message_new_method_call(BUSNAME,OBJPATH,\
							INTERFACE,ASD_DBUS_STA_METHOD_SHOW_BSS_STA_NUM_BY_WLANID_AND_RADIOID);
	dbus_error_init(&err2);
	
	reply2 = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query2,-1, &err2);

	dbus_message_unref(query2);
	if (NULL == reply2) {
		if (dbus_error_is_set(&err2)) {
			dbus_error_free(&err2);
		}
		*ret=ASD_DBUS_ERROR;
		dcli_free_WtpAthStatisticInfo(WtpHead);
		//dbus_message_unref(reply2);
		return NULL;
	}
		
	dbus_message_iter_init(reply2,&iter);
	dbus_message_iter_get_basic(&iter,&ret2);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&asd_wtp_num);

	if((0 == ret2)&&(asd_wtp_num != 0)){
		dbus_message_iter_next(&iter);
		dbus_message_iter_recurse(&iter,&iter_array);
		
		for(i=0; i<asd_wtp_num; i++){	
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&asd_wtp_id);
	
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&radio_num);
		
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_recurse(&iter_struct,&iter_bss_array);
			
			for(j=0; j<radio_num; j++){
				dbus_message_iter_recurse(&iter_bss_array,&iter_bss_struct);
				dbus_message_iter_get_basic(&iter_bss_struct,&radio_id);
			
				dbus_message_iter_next(&iter_bss_struct);
				dbus_message_iter_get_basic(&iter_bss_struct,&bss_num);

				dbus_message_iter_next(&iter_bss_struct);
				dbus_message_iter_recurse(&iter_bss_struct,&iter_sub_array);

				for(k=0; k<bss_num; k++){
					dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);	
					dbus_message_iter_get_basic(&iter_sub_struct,&wlan_id);
					
					dbus_message_iter_next(&iter_sub_struct);	
					dbus_message_iter_get_basic(&iter_sub_struct,&wirelessifNumStations);

					dbus_message_iter_next(&iter_sub_struct);	
					dbus_message_iter_get_basic(&iter_sub_struct,&auth_tms);
					
					dbus_message_iter_next(&iter_sub_struct);	
					dbus_message_iter_get_basic(&iter_sub_struct,&repauth_tms);

					dbus_message_iter_next(&iter_sub_struct);	
					dbus_message_iter_get_basic(&iter_sub_struct,&assoc_req_num);
					
					dbus_message_iter_next(&iter_sub_struct);	
					dbus_message_iter_get_basic(&iter_sub_struct,&assoc_resp_num);
					
					dbus_message_iter_next(&iter_sub_struct);	
					dbus_message_iter_get_basic(&iter_sub_struct,&assoc_success);//xiaodawei add assoc success times, 20110418

					dbus_message_iter_next(&iter_sub_struct);	
					dbus_message_iter_get_basic(&iter_sub_struct,&acc_tms);
					//qiuchen
					
					dbus_message_iter_next(&iter_sub_struct);	
					dbus_message_iter_get_basic(&iter_sub_struct,&sta_online_time);
					dbus_message_iter_next(&iter_sub_struct);	
					dbus_message_iter_get_basic(&iter_sub_struct,&sta_drop_cnt);
					dbus_message_iter_next(&iter_sub_struct);	
					dbus_message_iter_get_basic(&iter_sub_struct,&auth_req_cnt);
					dbus_message_iter_next(&iter_sub_struct);	
					dbus_message_iter_get_basic(&iter_sub_struct,&auth_suc_cnt);
					dbus_message_iter_next(&iter_sub_struct);	
					dbus_message_iter_get_basic(&iter_sub_struct,&auth_fail_cnt);

					
					//end
					WtpSearchNode = WtpHead;
					while(WtpSearchNode!= NULL){
						if((WtpSearchNode->wtpCurrID == asd_wtp_id)&&((WtpSearchNode->wtpwirelessifindex-1) == (radio_id%4))&&(WtpSearchNode->wlanid == wlan_id)){
							WtpSearchNode->wirelessifNumStations = wirelessifNumStations;
							WtpSearchNode->wirelessifRxAuthenFrameCnt = auth_tms;
							WtpSearchNode->wirelessifTxAuthenFrameCnt = repauth_tms;
							WtpSearchNode->wirelessifRxAssociateFrameCnt = assoc_req_num;
							WtpSearchNode->wirelessifTxAssociateFrameCnt = assoc_resp_num;
							WtpSearchNode->wirelessIfSuccAssociatedNum = assoc_success;
							WtpSearchNode->wirelessIfAssociatedUserNum = acc_tms;
							WtpSearchNode->AllApUserOnlineTime = sta_online_time;
							WtpSearchNode->APUserLostConnectionCnt = sta_drop_cnt;
							WtpSearchNode->APAuthReqCnt = auth_req_cnt;
							WtpSearchNode->APAuthSucCnt = auth_suc_cnt;
							WtpSearchNode->APAuthFailCnt = auth_fail_cnt;
						}
						WtpSearchNode = WtpSearchNode->next;
					}
					dbus_message_iter_next(&iter_sub_array);
				}
				dbus_message_iter_next(&iter_bss_array);
			}
			dbus_message_iter_next(&iter_array);
		}
	}
	dbus_message_unref(reply2);
	return WtpHead;
#if 0
	//asd dbus operation
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,\
							INTERFACE,ASD_DBUS_STA_METHOD_SHOW_BSS_STA_NUM_BY_WLANID_AND_RADIOID);
	dbus_error_init(&err);

	WtpNode=WtpHead;
	while(WtpNode){
		unsigned int l_radio_id = 0;
		l_radio_id = (unsigned int)WtpNode->wtpwirelessifindex-1;
		dbus_message_append_args(query,
								DBUS_TYPE_UINT32,&WtpNode->wtpCurrID,
								DBUS_TYPE_UINT32,&l_radio_id,
								DBUS_TYPE_UINT32,&WtpNode->wlanid,
								DBUS_TYPE_INVALID);
								
		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);			

		if (NULL == reply) {
			if (dbus_error_is_set(&err)) {
				dbus_error_free(&err);
			}
			*ret=ASD_DBUS_ERROR;
			dcli_free_WtpAthStatisticInfo(WtpHead);
			dbus_message_unref(query);
			return NULL;
		}
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,ret);

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&WtpNode->wirelessifNumStations);
		
		WtpNode	= WtpNode->next;
		dbus_message_unref(reply);
	}
	dbus_message_unref(query);
	
	return WtpHead;
#endif	
}

/*for showting all wtp infor & dot11ConjunctionTable  by nl 20100622 b22*/
struct WtpInfor* show_infor_of_all_wtp(int index,int localid,DBusConnection *dcli_dbus_connection, unsigned int *num, unsigned int *ret)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;	
	
	struct WtpInfor *WtpNode = NULL;
	struct WtpInfor *WtpHead = NULL;
	struct WtpInfor *WtpSearchNode = NULL;
	int i;
	int j;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,\
							INTERFACE,ASD_DBUS_STA_METHOD_SHOW_ALL_WTP_INFORMATION);
	
	dbus_error_init(&err);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);	
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(*ret == 0){
		if((WtpHead = (struct WtpInfor*)malloc(sizeof(struct WtpInfor))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
		}
		memset(WtpHead,0,sizeof(struct WtpInfor));
		WtpHead->WtpInfor_list = NULL;
		WtpHead->WtpInfor_last = NULL;
			
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,num);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);

		for (i = 0; i < *num; i++) {
			DBusMessageIter iter_struct;
			DBusMessageIter iter_sub_array; 	
			if((WtpNode = (struct WtpInfor*)malloc(sizeof(struct WtpInfor))) == NULL){
					dcli_free_WtpInfo(WtpHead);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(WtpNode,0,sizeof(struct WtpInfor));
			WtpNode->next = NULL;

			WtpNode->wtpMacAddr = (unsigned char *)malloc(MAC_LEN +1);
			memset(WtpNode->wtpMacAddr,0,(MAC_LEN +1));

			if(WtpHead->WtpInfor_list == NULL){
				WtpHead->WtpInfor_list = WtpNode;
				WtpHead->next = WtpNode;
			}
			else{
				WtpHead->WtpInfor_last->next = WtpNode;
			}
			WtpHead->WtpInfor_last = WtpNode;
			
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpCurrID));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[0]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[1]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[2]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[3]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[4]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtpMacAddr[5]));
			
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->wtp_total_online_time));	//z1	
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->num_assoc_failure)); 		//a1a2


			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->acc_tms));				//e1
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->auth_tms));	
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->repauth_tms));			//e3
			
			
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->auth_success_num)); 	//b1
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->auth_fail_num));	
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->auth_invalid_num));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->auth_timeout_num));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->auth_refused_num));		//b5
			
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->auth_others_num));		//b6
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->assoc_req_num));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->assoc_resp_num));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->assoc_invalid_num));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->assoc_timeout_num));	//b10

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->assoc_success_num));

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->assoc_refused_num));		//b11
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->assoc_others_num));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->reassoc_request_num));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->reassoc_success_num));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->reassoc_invalid_num));		//b15

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->reassoc_timeout_num));		//b16
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->reassoc_refused_num));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->reassoc_others_num));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->identify_request_num));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->identify_success_num));		//b20

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->abort_key_error_num));		//b21
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->abort_invalid_num));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->abort_timeout_num));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->abort_refused_num));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->abort_others_num));		//b25

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->deauth_request_num));		//b26
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->deauth_user_leave_num));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->deauth_ap_unable_num));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->deauth_abnormal_num));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->deauth_others_num));		//b30

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->disassoc_request_num));		//b31
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->disassoc_user_leave_num));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->disassoc_ap_unable_num));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->disassoc_abnormal_num));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->disassoc_others_num));		//b35

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->rx_mgmt_pkts)); 			//c1
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->tx_mgmt_pkts));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->rx_ctrl_pkts));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->tx_ctrl_pkts));				//c4

			/*dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->rx_data_pkts)); 			//c5
			printf("   WtpNode->rx_data_pkts   =  %d  \n ",WtpNode->rx_data_pkts);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->tx_data_pkts));
			printf("   WtpNode->tx_data_pkts   =  %d  \n ",WtpNode->tx_data_pkts);*/
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->rx_auth_pkts));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->tx_auth_pkts));				//c8

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->rx_assoc_norate));				//c8
			
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->rx_assoc_capmismatch));				//c8
			
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->assoc_invaild));				//c8
			
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->reassoc_deny));				//c8

			//mahz add 2011.5.5
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->usr_auth_tms_record));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->ac_rspauth_tms_record));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->auth_fail_record));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->auth_success_record));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->num_assoc_record));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->num_reassoc_record));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->num_assoc_failure_record));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->num_reassoc_failure_record));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->assoc_success_record));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->reassoc_success_record));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->assoc_req_record));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->assoc_resp_record));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->total_ap_flow_record));
			//			
			dbus_message_iter_next(&iter_array);
		}
	}
	else{
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return NULL;
	}
	dbus_message_unref(reply);

	/*fengwenchao add 20110617*/
	DBusMessage *query2, *reply2;
	DBusError err2;
	DBusMessageIter	 iter2;
	DBusMessageIter  iter_array2;
	
	unsigned int rx_data_pkts = 0;			 
	unsigned int tx_data_pkts = 0;
	unsigned int wtpid = 0;
	unsigned int ret2= 0;
	unsigned int wtp_num = 0;
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);

	query2 = dbus_message_new_method_call(BUSNAME,OBJPATH,\
			INTERFACE,WID_DBUS_CONF_METHOD_SHOW_ALL_WTP_INFORMATION);
						  
	dbus_error_init(&err2);
	
	reply2 = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query2,-1, &err2);	
			
	dbus_message_unref(query2);
	if (NULL == reply2) {
		if (dbus_error_is_set(&err2)) {
			dbus_error_free(&err2);
		}
		dcli_free_WtpInfo(WtpHead);
		return NULL;
	}	

	dbus_message_iter_init(reply2,&iter2);
	dbus_message_iter_get_basic(&iter2,&ret2);

	dbus_message_iter_next(&iter2);	
	dbus_message_iter_get_basic(&iter2,&wtp_num);

	if((ret2 ==0)&&(wtp_num != 0))
	{
	    dbus_message_iter_next(&iter2);	
	    dbus_message_iter_recurse(&iter2,&iter_array2);
		
	    for(i = 0; i < wtp_num; i++)
	    {
			DBusMessageIter iter_struct;
			
			dbus_message_iter_recurse(&iter_array2,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(wtpid));
			
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(rx_data_pkts));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(tx_data_pkts));

			WtpSearchNode = WtpHead->WtpInfor_list;

			while(WtpSearchNode != NULL)
			{
				if(WtpSearchNode->wtpCurrID == wtpid)
				{
					WtpSearchNode->rx_data_pkts = rx_data_pkts;
					//printf("   WtpSearchNode->rx_data_pkts   =  %d  \n ",WtpSearchNode->rx_data_pkts);
					WtpSearchNode->tx_data_pkts = tx_data_pkts;
					//printf("   WtpSearchNode->tx_data_pkts   =  %d  \n ",WtpSearchNode->tx_data_pkts);
				}
				WtpSearchNode = WtpSearchNode->next;
			}
			 dbus_message_iter_next(&iter_array2);
	    }
		dbus_message_unref(reply2);
	}	
    /*fengwenchao add end*/

	return WtpHead;
}
/*xdw add for show ap network information of all wtp, 20101215*/
struct WtpNetworkInfo* show_all_wtp_network_info(int index,int localid,DBusConnection *dcli_dbus_connection, unsigned int *num, unsigned int *ret)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;	
	
	struct WtpNetworkInfo *WtpNode = NULL;
	struct WtpNetworkInfo *WtpHead = NULL;
	int i = 0;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,\
							INTERFACE,WID_DBUS_CONF_METHOD_SHOW_ALL_WTP_NETWORK_INFO);
	
	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);	

	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(*ret == 0){
		char * wtpip = NULL;
		if((WtpHead = (struct WtpNetworkInfo*)malloc(sizeof(struct WtpNetworkInfo))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
		}
		memset(WtpHead,0,sizeof(struct WtpNetworkInfo));
		WtpHead->WtpNetworkInfo_list = NULL;
		WtpHead->WtpNetworkInfo_last = NULL;

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,num);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);

		for (i = 0; i < *num; i++) {
			DBusMessageIter iter_struct;
			DBusMessageIter iter_sub_array; 	
			if((WtpNode = (struct WtpNetworkInfo*)malloc(sizeof(struct WtpNetworkInfo))) == NULL){
					dcli_free_WtpNetworkInfo(WtpHead);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
	
			memset(WtpNode,0,sizeof(struct WtpNetworkInfo));
			WtpNode->next = NULL;

			WtpNode->WTPMAC = (unsigned char *)malloc(MAC_LEN +1);
			memset(WtpNode->WTPMAC,0,(MAC_LEN +1));

			if(WtpHead->WtpNetworkInfo_list == NULL){
				WtpHead->WtpNetworkInfo_list = WtpNode;
				WtpHead->next = WtpNode;
			}
			else{
				WtpHead->WtpNetworkInfo_last->next = WtpNode;
			}
			WtpHead->WtpNetworkInfo_last = WtpNode;

			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->WTPID));

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->WTPMAC[0]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->WTPMAC[1]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->WTPMAC[2]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->WTPMAC[3]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->WTPMAC[4]));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->WTPMAC[5]));

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&wtpip);
			WtpNode->WTPIP = (char*)malloc(strlen(wtpip)+1);
			memset(WtpNode->WTPIP,0,strlen(wtpip)+1);
			memcpy(WtpNode->WTPIP,wtpip,strlen(wtpip));
	
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->ap_mask_new));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->ap_gateway));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->ap_dnsfirst));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(WtpNode->ap_dnssecend));
		
			dbus_message_iter_next(&iter_array);
		}
	}

	dbus_message_unref(reply);

	return WtpHead;
}
/*nl add for ap group 20100912*/
struct WtpList * wtp_apply_wlan_cmd_wtp_apply_wlan_id(int index,int localid,
	   DBusConnection * dbus_connection, unsigned int type,unsigned int id,
	   unsigned char wlan_id,int *count,unsigned int *ret){
	   
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	
	int i = 0;
	int wtpid = 0;
	char fail_reason = 0;
	struct WtpList	*WtpNode = NULL;
	struct WtpList *wtp_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WTP_METHOD_APPAY_WTP_WLANID);

	dbus_error_init(&err);

	dbus_message_append_args(query,
						DBUS_TYPE_UINT32,&type,
						DBUS_TYPE_UINT32,&id,
						DBUS_TYPE_BYTE,&wlan_id,						 
						DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		printf("## count : %d\n",(*count));
		if((*count) != 0){
			if((wtp_list_head = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(wtp_list_head,0,sizeof(struct WtpList));
			wtp_list_head->WtpList_list = NULL;
			wtp_list_head->WtpList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((WtpNode = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
				dcli_free_WtpList(wtp_list_head);
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(WtpNode,0,sizeof(struct WtpList));
			WtpNode->next = NULL;

			if(wtp_list_head->WtpList_list == NULL){
				wtp_list_head->WtpList_list = WtpNode;
				wtp_list_head->next = WtpNode;
			}
			else{
				wtp_list_head->WtpList_last->next = WtpNode;
			}
			
			wtp_list_head->WtpList_last = WtpNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&wtpid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			WtpNode->WtpId = wtpid;
			WtpNode->FailReason =fail_reason;
		}
	}

	dbus_message_unref(reply);

	return wtp_list_head ;
}
/*nl add for ap group 20100912*/
struct WtpList * wtp_apply_interface_cmd_wtp_apply_interface(int index,int localid,
	   DBusConnection * dbus_connection, unsigned int type,unsigned int id,
	   char *if_name,int *count,unsigned int *ret,unsigned int *ret6){
	   
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	
	int i = 0;
	int wtpid = 0;
	char fail_reason = 0;
	struct WtpList	*WtpNode = NULL;
	struct WtpList *wtp_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WTP_METHOD_APPAY_WTP_IFNAME);

	dbus_error_init(&err);

	dbus_message_append_args(query,
						DBUS_TYPE_UINT32,&type,
						DBUS_TYPE_UINT32,&id,
						DBUS_TYPE_STRING,if_name,						 
						DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,ret6);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if((*count) != 0){
			if((wtp_list_head = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(wtp_list_head,0,sizeof(struct WtpList));
			wtp_list_head->WtpList_list = NULL;
			wtp_list_head->WtpList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((WtpNode = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
					dcli_free_WtpList(wtp_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(WtpNode,0,sizeof(struct WtpList));
			WtpNode->next = NULL;

			if(wtp_list_head->WtpList_list == NULL){
				wtp_list_head->WtpList_list = WtpNode;
				wtp_list_head->next = WtpNode;
			}
			else{
				wtp_list_head->WtpList_last->next = WtpNode;
			}
			
			wtp_list_head->WtpList_last = WtpNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&wtpid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			WtpNode->WtpId = wtpid;
			WtpNode->FailReason = fail_reason;
		}
	}

	dbus_message_unref(reply);
	return wtp_list_head ;
}
/*nl add for group 20100913*/
struct WtpList * config_wtp_service_cmd_wtp_used(int index,int localid,
	DBusConnection *dbus_connection,unsigned int type,unsigned int id,
	unsigned int stat,int *count,unsigned int *ret){
	
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int wtpid = 0;
	char fail_reason = 0;
	struct WtpList  *WtpNode = NULL;
	struct WtpList *wtp_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WTP_METHOD_WTP_USED);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_UINT32,&stat, 					 
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			//(wtp_list_head) = (WtpList *)malloc((*count)*(sizeof(WtpList)));
			if((wtp_list_head = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(wtp_list_head,0,sizeof(struct WtpList));
			wtp_list_head->WtpList_list = NULL;
			wtp_list_head->WtpList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((WtpNode = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
					dcli_free_WtpList(wtp_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(WtpNode,0,sizeof(struct WtpList));
			WtpNode->next = NULL;

			if(wtp_list_head->WtpList_list == NULL){
				wtp_list_head->WtpList_list = WtpNode;
				wtp_list_head->next = WtpNode;
			}
			else{
				wtp_list_head->WtpList_last->next = WtpNode;
			}
			
			wtp_list_head->WtpList_last = WtpNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&wtpid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			WtpNode->FailReason = fail_reason;
			WtpNode->WtpId = wtpid;
		}
	}
	
	dbus_message_unref(reply);
	return wtp_list_head ;
}

/*nl add for group 20100913*/
struct WtpList * wtp_disable_wlan_cmd_wtp_disable_wlan(int index,int localid,
	DBusConnection *dbus_connection,unsigned int type,unsigned int id,
	unsigned char wlan_id, int *count,unsigned int *ret){
	
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int wtpid = 0;
	char fail_reason = 0;
	struct WtpList  *WtpNode = NULL;
	struct WtpList *wtp_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WTP_METHOD_DISABLE_WLAN_ID);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_BYTE,&wlan_id, 					 
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			//(wtp_list_head) = (WtpList *)malloc((*count)*(sizeof(WtpList)));
			if((wtp_list_head = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(wtp_list_head,0,sizeof(struct WtpList));
			wtp_list_head->WtpList_list = NULL;
			wtp_list_head->WtpList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((WtpNode = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
					dcli_free_WtpList(wtp_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(WtpNode,0,sizeof(struct WtpList));
			WtpNode->next = NULL;

			if(wtp_list_head->WtpList_list == NULL){
				wtp_list_head->WtpList_list = WtpNode;
				wtp_list_head->next = WtpNode;
			}
			else{
				wtp_list_head->WtpList_last->next = WtpNode;
			}
			
			wtp_list_head->WtpList_last = WtpNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&wtpid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			WtpNode->FailReason = fail_reason;
			WtpNode->WtpId = wtpid;
		}
	}
	
	dbus_message_unref(reply);
	return wtp_list_head ;
}


/*nl add for group 20100913*/
struct WtpList * wtp_enable_wlan_cmd_wtp_enable_wlan(int index,int localid,
	DBusConnection *dbus_connection,unsigned int type,unsigned int id,
	unsigned char wlan_id, int *count,unsigned int *ret){
	
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int wtpid = 0;
	char fail_reason = 0;
	struct WtpList  *WtpNode = NULL;
	struct WtpList *wtp_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WTP_METHOD_ENABLE_WLAN_ID);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_BYTE,&wlan_id, 					 
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((wtp_list_head = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(wtp_list_head,0,sizeof(struct WtpList));
			wtp_list_head->WtpList_list = NULL;
			wtp_list_head->WtpList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((WtpNode = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
					dcli_free_WtpList(wtp_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(WtpNode,0,sizeof(struct WtpList));
			WtpNode->next = NULL;

			if(wtp_list_head->WtpList_list == NULL){
				wtp_list_head->WtpList_list = WtpNode;
				wtp_list_head->next = WtpNode;
			}
			else{
				wtp_list_head->WtpList_last->next = WtpNode;
			}
			
			wtp_list_head->WtpList_last = WtpNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&wtpid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			WtpNode->FailReason = fail_reason;
			WtpNode->WtpId = wtpid;
		}
	}
	
	dbus_message_unref(reply);
	return wtp_list_head ;
}

/*nl add for group 20100913*/
struct WtpList * wtp_triger_num_cmd_set_wtp_flow_triger(int index,int localid,
	DBusConnection *dbus_connection,unsigned int type,unsigned int id,
	unsigned int triger, int *count,unsigned int *ret){
	
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int wtpid = 0;
	char fail_reason = 0;
	struct WtpList  *WtpNode = NULL;
	struct WtpList *wtp_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WTP_METHOD_APPAY_WTP_FLOW_TRIGER);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
										DBUS_TYPE_UINT32,&type,
										DBUS_TYPE_UINT32,&id,
										DBUS_TYPE_UINT32,&triger, 					 
										DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if((type == 1)&&(*ret == WID_DBUS_SUCCESS)){
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((wtp_list_head = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(wtp_list_head,0,sizeof(struct WtpList));
			wtp_list_head->WtpList_list = NULL;
			wtp_list_head->WtpList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((WtpNode = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
					dcli_free_WtpList(wtp_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(WtpNode,0,sizeof(struct WtpList));
			WtpNode->next = NULL;

			if(wtp_list_head->WtpList_list == NULL){
				wtp_list_head->WtpList_list = WtpNode;
				wtp_list_head->next = WtpNode;
			}
			else{
				wtp_list_head->WtpList_last->next = WtpNode;
			}
			
			wtp_list_head->WtpList_last = WtpNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&wtpid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			WtpNode->FailReason = fail_reason;
			WtpNode->WtpId = wtpid;
		}
	}
	
	dbus_message_unref(reply);
	return wtp_list_head ;
}

/*nl add for group 20100920*/
struct WtpList * set_ap_max_throughout_cmd_set_ap_max_throught(int index,int localid,
	DBusConnection *dbus_connection,unsigned int type,unsigned int id,
	unsigned char bandwidth, int *count,unsigned int *ret){
	
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int wtpid = 0;
	char fail_reason = 0;
	struct WtpList  *WtpNode = NULL;
	struct WtpList *wtp_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SET_WID_AP_MAX_THROUGHOUT);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
										DBUS_TYPE_UINT32,&type,
										DBUS_TYPE_UINT32,&id,
										DBUS_TYPE_BYTE,&bandwidth,
										DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if((type == 1)&&(*ret == WID_DBUS_SUCCESS)){
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((wtp_list_head = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(wtp_list_head,0,sizeof(struct WtpList));
			wtp_list_head->WtpList_list = NULL;
			wtp_list_head->WtpList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((WtpNode = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
					dcli_free_WtpList(wtp_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(WtpNode,0,sizeof(struct WtpList));
			WtpNode->next = NULL;

			if(wtp_list_head->WtpList_list == NULL){
				wtp_list_head->WtpList_list = WtpNode;
				wtp_list_head->next = WtpNode;
			}
			else{
				wtp_list_head->WtpList_last->next = WtpNode;
			}
			
			wtp_list_head->WtpList_last = WtpNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&wtpid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			WtpNode->FailReason = fail_reason;
			WtpNode->WtpId = wtpid;
		}
	}
	
	dbus_message_unref(reply);
	return wtp_list_head ;
}
int wtp_check_sta_num_asd(int index,int localid,DBusConnection *dbus_connection,int wtpid)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;
	//printf("$$$$$$$$\n");
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	int sder = 3;
	int stanum = 0;
	int ID = 0;
	ID = wtpid;
	unsigned int radioid = 0;  //fengwenchao add 20110513
	//printf("access asd\n");
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_GET_STA_INFO);
	//printf("dddddddddd\n");				
	dbus_message_append_args(query,
								DBUS_TYPE_UINT32,&sder,
								DBUS_TYPE_UINT32,&ID,
								DBUS_TYPE_UINT32,&radioid,    //fengwenchao add 20110513
								DBUS_TYPE_INVALID);
	dbus_error_init(&err);                                     
	//printf("aaaaaaaaaaaa\n");	
	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
					
	dbus_message_unref(query);
					
	if (NULL == reply)
		{
			if (dbus_error_is_set(&err))
				{
					dbus_error_free(&err);
				}
			return NULL;
		}
	//printf("sssssssssss\n");			
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&stanum);
	dbus_message_unref(reply);

	//printf("asd OK\n");		
	return stanum;
}
struct WtpList * wtp_max_sta_cmd_set_wtp_max_sta_num(int index,int localid,
	DBusConnection *dbus_connection,unsigned int type,unsigned int id,unsigned int wtp_max_sta,
	unsigned int *stanum,int *count,int *finalnum,unsigned int *ret,int *ret1)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;
	printf("access struct \n");
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	int accessnum = 0;
	//int failnum = 0;
	int i = 0;
	int j = 0;
	//int sder = 3;
	int wtpid = 0;
	//int access_then_failnum= 0;
	char fail_reason = 0;
	struct WtpList *accesswtp = NULL;
	//struct WtpList *failwtp = NULL;
	//struct WtpList *access_then_fail = NULL;
	struct WtpList  *WtpNode = NULL;
	struct WtpList *wtp_list_head = NULL;
  	struct WtpList *wtp_show_node = NULL;
	//struct WtpList *count_wtp = NULL;
	struct WtpList *final_wtp = NULL;
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WTP_INTERFACE_CHECK_WTP_STA);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
										DBUS_TYPE_UINT32,&type,
										DBUS_TYPE_UINT32,&id, 					 
										DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	//printf("3333333333\n");
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	//printf("44444444444\n");
	if((type==0)&&(*ret == WID_DBUS_SUCCESS))
		{	//printf("access type0 wtp max sta\n");
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,count);
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&wtpid);
			dbus_message_unref(reply);
			//printf("#########\n");
			*stanum = wtp_check_sta_num_asd(index,localid,dbus_connection,wtpid);
			if(*stanum>wtp_max_sta)
				{
					*ret1 = -1;
					//printf("ret1 = %d\n",*ret1);
				}
			else
				{
				  	memset(BUSNAME,0,PATH_LEN);
					memset(OBJPATH,0,PATH_LEN);
					memset(INTERFACE,0,PATH_LEN);
					//printf("@@@@@@@\n");
					ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
					ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_OBJPATH,OBJPATH);
					ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_INTERFACE,INTERFACE);
		
					query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WTP_METHOD_APPAY_WTP_MAX_STA);
					dbus_error_init(&err);

					dbus_message_append_args(query,
				   							DBUS_TYPE_UINT32,&wtp_max_sta,
											DBUS_TYPE_UINT32,&wtpid, 					 
											DBUS_TYPE_INVALID);

					reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
					dbus_message_unref(query);
	
					if (NULL == reply)
						{
							if (dbus_error_is_set(&err))
								{
									dbus_error_free(&err);
								}
							return NULL;
						}	
					dbus_message_iter_init(reply,&iter);
					dbus_message_iter_get_basic(&iter,ret1);
					dbus_message_unref(reply);
					//printf("ret1 = %d\n",*ret1);
				}
		}
		
	
	if((type == 1)&&(*ret == WID_DBUS_SUCCESS))
		{
			printf("access type1 wtp max sta\n");
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,count);
			printf("count = %d\n",*count);
			if(*count != 0)
				{
					if((wtp_list_head = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL)
						{
							*ret = MALLOC_ERROR;
							dbus_message_unref(reply);
							return NULL;
						}
				
					memset(wtp_list_head,0,sizeof(struct WtpList));
					wtp_list_head->WtpList_list = NULL;
					wtp_list_head->WtpList_last = NULL;
				}
		
			for(i=0; i < (*count); i++)
				{
					if((WtpNode = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL)
						{
							dcli_free_WtpList(wtp_list_head);
							*ret = MALLOC_ERROR;
							dbus_message_unref(reply);
							return NULL;
						}
			
					memset(WtpNode,0,sizeof(struct WtpList));
					WtpNode->next = NULL;

					if(wtp_list_head->WtpList_list == NULL)
						{
							wtp_list_head->WtpList_list = WtpNode;
							wtp_list_head->next = WtpNode;
						}
					else
						{
							wtp_list_head->WtpList_last->next = WtpNode;
						}
			
					wtp_list_head->WtpList_last = WtpNode;
			
					dbus_message_iter_next(&iter);	
					dbus_message_iter_get_basic(&iter,&wtpid);
			
					WtpNode->WtpId = wtpid;
					printf("wtpid = %d\n",wtpid);
				}
	
	   		dbus_message_unref(reply);
			accesswtp = (struct WtpList *)malloc(*count *(sizeof(struct WtpList)));
			memset(accesswtp,0,*count *(sizeof(struct WtpList)));
			//failwtp =  (struct WtpList *)malloc(*count *(sizeof(struct WtpList)));
			//access_then_fail = (struct WtpList *)malloc(*count *(sizeof(struct WtpList)));
			//count_wtp = (struct WtpList *)malloc(*count *(sizeof(struct WtpList)));
			final_wtp = (struct WtpList *)malloc(*count *(sizeof(struct WtpList)));
			memset(final_wtp,0,*count *(sizeof(struct WtpList)));
			for(i=0; i<*count; i++)
			{
				 if(wtp_show_node == NULL)
				 wtp_show_node = wtp_list_head->WtpList_list;
			 	else 
			 	wtp_show_node = wtp_show_node->next;

				 if(wtp_show_node == NULL)
				 	break;
			 
				printf("QQQQQQQQQQQ \n");		
				*stanum = wtp_check_sta_num_asd(index,localid,dbus_connection,wtp_show_node->WtpId);
			

			
				if(*stanum>wtp_max_sta)
					{
						//failwtp[i].WtpId = wtp_show_node->WtpId;
						//failnum++;
						//failwtp[i].WtpStaNum = *stanum;
						
						final_wtp[i].WtpId = wtp_show_node->WtpId;
						final_wtp[i].FailReason = 'f';
						final_wtp[i].WtpStaNum = *stanum;
						//*finalnum++;
						j++;
						printf("j = %d\n",j);
						printf("final_wtp[%d].WtpStaNum = %d\n",i,final_wtp[i].WtpStaNum);
						//printf("failwtp[%d].wtpid = %d\n",i,failwtp[i].WtpId);
						//printf("failwtp[%d].WtpStaNum = %d\n",i,failwtp[i].WtpStaNum);
					}
				else
					{
						accesswtp[i].WtpId = wtp_show_node->WtpId;
						accessnum++;
						printf("wtp_show_node->WtpId = %d\n",wtp_show_node->WtpId);
						printf("accesswtp[i].WtpId = %d\n",accesswtp[i].WtpId);

						printf("WWWWWWWWWWW\n");
						memset(BUSNAME,0,PATH_LEN);
						memset(OBJPATH,0,PATH_LEN);
						memset(INTERFACE,0,PATH_LEN);

						ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
						ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_OBJPATH,OBJPATH);
						ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_INTERFACE,INTERFACE);
		
						query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WTP_METHOD_APPAY_WTP_MAX_STA);
						dbus_error_init(&err);
						//printf("accessnum = %d\n",accessnum);

						dbus_message_append_args(query,
				   						DBUS_TYPE_UINT32,&wtp_max_sta,
										DBUS_TYPE_UINT32,&accesswtp[i].WtpId, 					 
										DBUS_TYPE_INVALID);
						printf("accesswtp.wtpid =  %d\n",accesswtp[i].WtpId);
						reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
						dbus_message_unref(query);
	
						if (NULL == reply)
						{
							if (dbus_error_is_set(&err))
								{
									dbus_error_free(&err);
								}
							return NULL;
						}	
						dbus_message_iter_init(reply,&iter);
						dbus_message_iter_get_basic(&iter,ret1);

						if(*ret1 == 0)
							{
					
							}
						else
							{
								//access_then_fail[i].WtpId = accesswtp[i].WtpId;
								//access_then_failnum++;
								final_wtp[i].WtpId = accesswtp[i].WtpId;
								//*finalnum++;
								j++;
								//printf("access_then_failnum\n",access_then_failnum);
							}
								
				
						/*if(count_wtp[i].WtpId == failwtp[i].WtpId)
							{
								final_wtp[i].WtpId = failwtp[i].WtpId;
								final_wtp[i].FailReason = 'f';
								final_wtp[i].WtpStaNum = failwtp[i].WtpStaNum;
								//*finalnum++;
								j++;
							}*/
						/*else if(count_wtp[i].WtpId == access_then_fail[i].WtpId)
							{
								final_wtp[i].WtpId = access_then_fail[i].WtpId;
								//*finalnum++;
								j++;
							}*/

						//printf("access_then_failnum = %d\n",access_then_failnum);
						//printf("j =%d\n",j);
					}
						
				}
				printf("j = %d\n",j);
				*finalnum = j;
				printf("finalnum = %d\n",*finalnum);
				dcli_free_WtpList(accesswtp);
				dcli_free_WtpList(wtp_list_head);
				//dcli_free_WtpList(failwtp);
				//dcli_free_WtpList(access_then_fail);
				//dcli_free_WtpList(count_wtp);
			}
			/*printf("WWWWWWWWWWW\n");
			memset(BUSNAME,0,PATH_LEN);
			memset(OBJPATH,0,PATH_LEN);
			memset(INTERFACE,0,PATH_LEN);

			ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
			ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_OBJPATH,OBJPATH);
			ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_INTERFACE,INTERFACE);
		
			query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WTP_METHOD_APPAY_WTP_MAX_STA);
			dbus_error_init(&err);
			printf("accessnum = %d\n",accessnum);
			for(i=0;i<accessnum;i++)
			{
				dbus_message_append_args(query,
				   						DBUS_TYPE_UINT32,&wtp_max_sta,
										DBUS_TYPE_UINT32,&accesswtp[i].WtpId, 					 
										DBUS_TYPE_INVALID);
				printf("accesswtp.wtpid =  %d\n",accesswtp[i].WtpId);
				reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
				dbus_message_unref(query);
	
				if (NULL == reply)
					{
						if (dbus_error_is_set(&err))
							{
								dbus_error_free(&err);
							}
						return NULL;
					}	
				dbus_message_iter_init(reply,&iter);
				dbus_message_iter_get_basic(&iter,ret1);

				if(*ret1 == 0)
					{
					
					}
				else
					{
						access_then_fail[i].WtpId = accesswtp[i].WtpId;
						access_then_failnum++;
					}
			}*/
			/*for(i=0;i<*count;i++)
				{
					if(count_wtp[i].WtpId == failwtp[i].WtpId)
						{
							final_wtp[i].WtpId = failwtp[i].WtpId;
							final_wtp[i].FailReason = 'f';
							final_wtp[i].WtpStaNum = failwtp[i].WtpStaNum;
							*finalnum++;
						}
					else if(count_wtp[i].WtpId == access_then_fail[i].WtpId)
						{
							final_wtp[i].WtpId = access_then_fail[i].WtpId;
							*finalnum++;
						}
				}*/

				/*dcli_free_WtpList(accesswtp);
				dcli_free_WtpList(failwtp);
				dcli_free_WtpList(access_then_fail);
				dcli_free_WtpList(count_wtp);*/
		

		return final_wtp;
}

/*nl add for group 20100913*/
struct WtpList * wtp_triger_num_cmd_set_wtp_number_triger(int index,int localid,
	DBusConnection *dbus_connection,unsigned int type,unsigned int id,
	unsigned int triger, int *count,unsigned int *ret){
	
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int wtpid = 0;
	char fail_reason = 0;
	struct WtpList  *WtpNode = NULL;
	struct WtpList *wtp_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WTP_METHOD_APPAY_WTP_TRIGER);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
										DBUS_TYPE_UINT32,&type,
										DBUS_TYPE_UINT32,&id,
										DBUS_TYPE_UINT32,&triger, 					 
										DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if((type == 1)&&(*ret == WID_DBUS_SUCCESS)){
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((wtp_list_head = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(wtp_list_head,0,sizeof(struct WtpList));
			wtp_list_head->WtpList_list = NULL;
			wtp_list_head->WtpList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((WtpNode = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
					dcli_free_WtpList(wtp_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(WtpNode,0,sizeof(struct WtpList));
			WtpNode->next = NULL;

			if(wtp_list_head->WtpList_list == NULL){
				wtp_list_head->WtpList_list = WtpNode;
				wtp_list_head->next = WtpNode;
			}
			else{
				wtp_list_head->WtpList_last->next = WtpNode;
			}
			
			wtp_list_head->WtpList_last = WtpNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&wtpid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			WtpNode->FailReason = fail_reason;
			WtpNode->WtpId = wtpid;
		}
	}
	
	dbus_message_unref(reply);
	return wtp_list_head ;
}

struct WtpList * tcpdump_ap_extension_command_cmd_tcpdump(int index,int localid,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,unsigned char *command,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	//char *wccommand = NULL;
	//wccommand = (char *)malloc(strlen(command)+1);
	//wccommand = command;
	int i = 0;
	int wtpid = 0;
	char fail_reason = 0;
	struct WtpList  *WtpNode = NULL;
	struct WtpList *wtp_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_INTERFACE,INTERFACE);

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_TCPDUMP_AP_EXTENSION_COMMAND);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_STRING,&command,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((wtp_list_head = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(wtp_list_head,0,sizeof(struct WtpList));
			wtp_list_head->WtpList_list = NULL;
			wtp_list_head->WtpList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((WtpNode = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
					dcli_free_WtpList(wtp_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(WtpNode,0,sizeof(struct WtpList));
			WtpNode->next = NULL;

			if(wtp_list_head->WtpList_list == NULL){
				wtp_list_head->WtpList_list = WtpNode;
				wtp_list_head->next = WtpNode;
			}
			else{
				wtp_list_head->WtpList_last->next = WtpNode;
			}
			
			wtp_list_head->WtpList_last = WtpNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&wtpid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			WtpNode->FailReason = fail_reason;
			WtpNode->WtpId = wtpid;
		}
	}
	
	dbus_message_unref(reply);
	return wtp_list_head ;
}

struct WtpList * set_ap_extension_command_cmd_set_ap_extension_command(int index,int localid,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,unsigned char *command,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int wtpid = 0;
	char fail_reason = 0;
	struct WtpList  *WtpNode = NULL;
	struct WtpList *wtp_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_INTERFACE,INTERFACE);

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SET_WID_AP_EXTENSION_COMMAND);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_STRING,&command,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((wtp_list_head = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(wtp_list_head,0,sizeof(struct WtpList));
			wtp_list_head->WtpList_list = NULL;
			wtp_list_head->WtpList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((WtpNode = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
					dcli_free_WtpList(wtp_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(WtpNode,0,sizeof(struct WtpList));
			WtpNode->next = NULL;

			if(wtp_list_head->WtpList_list == NULL){
				wtp_list_head->WtpList_list = WtpNode;
				wtp_list_head->next = WtpNode;
			}
			else{
				wtp_list_head->WtpList_last->next = WtpNode;
			}
			
			wtp_list_head->WtpList_last = WtpNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&wtpid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			WtpNode->FailReason = fail_reason;
			WtpNode->WtpId = wtpid;
		}
	}
	
	dbus_message_unref(reply);
	return wtp_list_head ;
}

struct WtpList * set_wtp_location_cmd_set_wtp_location(int index,int localid,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,char * location,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int wtpid = 0;
	char fail_reason = 0;
	struct WtpList  *WtpNode = NULL;
	struct WtpList *wtp_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_INTERFACE,INTERFACE);

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SET_WID_WTP_LOCATION);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_STRING,&location,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((wtp_list_head = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(wtp_list_head,0,sizeof(struct WtpList));
			wtp_list_head->WtpList_list = NULL;
			wtp_list_head->WtpList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((WtpNode = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
					dcli_free_WtpList(wtp_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(WtpNode,0,sizeof(struct WtpList));
			WtpNode->next = NULL;

			if(wtp_list_head->WtpList_list == NULL){
				wtp_list_head->WtpList_list = WtpNode;
				wtp_list_head->next = WtpNode;
			}
			else{
				wtp_list_head->WtpList_last->next = WtpNode;
			}
			
			wtp_list_head->WtpList_last = WtpNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&wtpid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			WtpNode->FailReason = fail_reason;
			WtpNode->WtpId = wtpid;
		}
	}
	
	dbus_message_unref(reply);
	return wtp_list_head ;
}

struct WtpList * set_ap_channel_disturb_trap_cmd_channel_disturb_trap(int index,int localid,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,int policy,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int wtpid = 0;
	char fail_reason = 0;
	struct WtpList  *WtpNode = NULL;
	struct WtpList *wtp_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_INTERFACE,INTERFACE);

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_AP_CHANNEL_DISTURB_TRAP);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_UINT32,&policy,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((wtp_list_head = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(wtp_list_head,0,sizeof(struct WtpList));
			wtp_list_head->WtpList_list = NULL;
			wtp_list_head->WtpList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((WtpNode = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
					dcli_free_WtpList(wtp_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(WtpNode,0,sizeof(struct WtpList));
			WtpNode->next = NULL;

			if(wtp_list_head->WtpList_list == NULL){
				wtp_list_head->WtpList_list = WtpNode;
				wtp_list_head->next = WtpNode;
			}
			else{
				wtp_list_head->WtpList_last->next = WtpNode;
			}
			
			wtp_list_head->WtpList_last = WtpNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&wtpid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			WtpNode->FailReason = fail_reason;
			WtpNode->WtpId = wtpid;
		}
	}
	
	dbus_message_unref(reply);
	return wtp_list_head ;
}


struct WtpList * set_wtp_netid_cmd_set_wtp_netid(int index,int localid,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,char * netid,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int wtpid = 0;
	char fail_reason = 0;
	struct WtpList  *WtpNode = NULL;
	struct WtpList *wtp_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_INTERFACE,INTERFACE);

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SET_WID_WTP_NETID);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_STRING,&netid,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((wtp_list_head = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(wtp_list_head,0,sizeof(struct WtpList));
			wtp_list_head->WtpList_list = NULL;
			wtp_list_head->WtpList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((WtpNode = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
					dcli_free_WtpList(wtp_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(WtpNode,0,sizeof(struct WtpList));
			WtpNode->next = NULL;

			if(wtp_list_head->WtpList_list == NULL){
				wtp_list_head->WtpList_list = WtpNode;
				wtp_list_head->next = WtpNode;
			}
			else{
				wtp_list_head->WtpList_last->next = WtpNode;
			}
			
			wtp_list_head->WtpList_last = WtpNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&wtpid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			WtpNode->FailReason = fail_reason;
			WtpNode->WtpId = wtpid;
		}
	}
	
	dbus_message_unref(reply);
	return wtp_list_head ;
}


struct WtpList * set_wtp_wtpname_cmd_set_wtp_name(int index,int localid,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,char * name,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int wtpid = 0;
	char fail_reason = 0;
	struct WtpList  *WtpNode = NULL;
	struct WtpList *wtp_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_INTERFACE,INTERFACE);

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WTP_METHOD_SET_WTPNAME);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_STRING,&name,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((wtp_list_head = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(wtp_list_head,0,sizeof(struct WtpList));
			wtp_list_head->WtpList_list = NULL;
			wtp_list_head->WtpList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((WtpNode = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
					dcli_free_WtpList(wtp_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(WtpNode,0,sizeof(struct WtpList));
			WtpNode->next = NULL;

			if(wtp_list_head->WtpList_list == NULL){
				wtp_list_head->WtpList_list = WtpNode;
				wtp_list_head->next = WtpNode;
			}
			else{
				wtp_list_head->WtpList_last->next = WtpNode;
			}
			
			wtp_list_head->WtpList_last = WtpNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&wtpid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			WtpNode->FailReason = fail_reason;
			WtpNode->WtpId = wtpid;
		}
	}
	
	dbus_message_unref(reply);
	return wtp_list_head ;
}

struct WtpList * set_ap_extension_infomation_enable_cmd_set_ap_extension_infomation_switch(int index,int localid,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,int policy,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int wtpid = 0;
	char fail_reason = 0;
	struct WtpList  *WtpNode = NULL;
	struct WtpList *wtp_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_INTERFACE,INTERFACE);

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WTP_METHOD_SET_WTP_EXTENSION_INFOMATION_SWITCH);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_UINT32,&policy,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((wtp_list_head = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(wtp_list_head,0,sizeof(struct WtpList));
			wtp_list_head->WtpList_list = NULL;
			wtp_list_head->WtpList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((WtpNode = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
					dcli_free_WtpList(wtp_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(WtpNode,0,sizeof(struct WtpList));
			WtpNode->next = NULL;

			if(wtp_list_head->WtpList_list == NULL){
				wtp_list_head->WtpList_list = WtpNode;
				wtp_list_head->next = WtpNode;
			}
			else{
				wtp_list_head->WtpList_last->next = WtpNode;
			}
			
			wtp_list_head->WtpList_last = WtpNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&wtpid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			WtpNode->FailReason = fail_reason;
			WtpNode->WtpId = wtpid;
		}
	}
	
	dbus_message_unref(reply);
	return wtp_list_head ;
}

struct WtpList * set_ap_terminal_distrub_infomation_switch_cmd_set_ap_switch(int index,int localid,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,int policy,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int wtpid = 0;
	char fail_reason = 0;
	struct WtpList  *WtpNode = NULL;
	struct WtpList *wtp_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_INTERFACE,INTERFACE);

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WTP_METHOD_SET_WTP_TERMINAL_DISTRUB_INFOMATION_SWITCH);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_UINT32,&policy,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((wtp_list_head = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(wtp_list_head,0,sizeof(struct WtpList));
			wtp_list_head->WtpList_list = NULL;
			wtp_list_head->WtpList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((WtpNode = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
					dcli_free_WtpList(wtp_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(WtpNode,0,sizeof(struct WtpList));
			WtpNode->next = NULL;

			if(wtp_list_head->WtpList_list == NULL){
				wtp_list_head->WtpList_list = WtpNode;
				wtp_list_head->next = WtpNode;
			}
			else{
				wtp_list_head->WtpList_last->next = WtpNode;
			}
			
			wtp_list_head->WtpList_last = WtpNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&wtpid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			WtpNode->FailReason = fail_reason;
			WtpNode->WtpId = wtpid;
		}
	}
	
	dbus_message_unref(reply);
	return wtp_list_head ;
}


struct WtpList * set_ap_terminal_distrub_infomation_pkt_cmd_set_ap_terminal_distrub_infomation_reportpkt(int index,int localid,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,int pkt,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int wtpid = 0;
	char fail_reason = 0;
	struct WtpList  *WtpNode = NULL;
	struct WtpList *wtp_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_INTERFACE,INTERFACE);

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WTP_METHOD_SET_WTP_TERMINAL_DISTRUB_INFOMATION_PKT);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_UINT32,&pkt,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((wtp_list_head = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(wtp_list_head,0,sizeof(struct WtpList));
			wtp_list_head->WtpList_list = NULL;
			wtp_list_head->WtpList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((WtpNode = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
					dcli_free_WtpList(wtp_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(WtpNode,0,sizeof(struct WtpList));
			WtpNode->next = NULL;

			if(wtp_list_head->WtpList_list == NULL){
				wtp_list_head->WtpList_list = WtpNode;
				wtp_list_head->next = WtpNode;
			}
			else{
				wtp_list_head->WtpList_last->next = WtpNode;
			}
			
			wtp_list_head->WtpList_last = WtpNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&wtpid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			WtpNode->FailReason = fail_reason;
			WtpNode->WtpId = wtpid;
		}
	}
	
	dbus_message_unref(reply);
	return wtp_list_head ;
}

struct WtpList * set_ap_terminal_distrub_infomation_sta_num_cmd_sta_num(int index,int localid,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,unsigned int sta_num,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int wtpid = 0;
	char fail_reason = 0;
	struct WtpList  *WtpNode = NULL;
	struct WtpList *wtp_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_INTERFACE,INTERFACE);

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WTP_METHOD_SET_WTP_TERMINAL_DISTRUB_INFOMATION_STA_NUM);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_UINT32,&sta_num,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((wtp_list_head = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(wtp_list_head,0,sizeof(struct WtpList));
			wtp_list_head->WtpList_list = NULL;
			wtp_list_head->WtpList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((WtpNode = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
					dcli_free_WtpList(wtp_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(WtpNode,0,sizeof(struct WtpList));
			WtpNode->next = NULL;

			if(wtp_list_head->WtpList_list == NULL){
				wtp_list_head->WtpList_list = WtpNode;
				wtp_list_head->next = WtpNode;
			}
			else{
				wtp_list_head->WtpList_last->next = WtpNode;
			}
			
			wtp_list_head->WtpList_last = WtpNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&wtpid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			WtpNode->FailReason = fail_reason;
			WtpNode->WtpId = wtpid;
		}
	}
	
	dbus_message_unref(reply);
	return wtp_list_head ;
}

struct WtpList * set_ap_extension_infomation_reportinterval_cmd_set_ap_extension_infomation_reportinterval(
	int index,int localid,DBusConnection *dbus_connection,unsigned int type,unsigned int id,unsigned int interval,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int wtpid = 0;
	char fail_reason = 0;
	struct WtpList  *WtpNode = NULL;
	struct WtpList *wtp_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_INTERFACE,INTERFACE);

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WTP_METHOD_SET_WTP_EXTENSION_INFOMATION_REPORTINTERVAL);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_UINT32,&interval,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((wtp_list_head = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(wtp_list_head,0,sizeof(struct WtpList));
			wtp_list_head->WtpList_list = NULL;
			wtp_list_head->WtpList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((WtpNode = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
					dcli_free_WtpList(wtp_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(WtpNode,0,sizeof(struct WtpList));
			WtpNode->next = NULL;

			if(wtp_list_head->WtpList_list == NULL){
				wtp_list_head->WtpList_list = WtpNode;
				wtp_list_head->next = WtpNode;
			}
			else{
				wtp_list_head->WtpList_last->next = WtpNode;
			}
			
			wtp_list_head->WtpList_last = WtpNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&wtpid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			WtpNode->FailReason = fail_reason;
			WtpNode->WtpId = wtpid;
		}
	}
	
	dbus_message_unref(reply);
	return wtp_list_head ;
}

struct WtpList * set_ap_l2_isolation_cmd_set_wlan_l2_isolation(int index,int localid,
	DBusConnection *dbus_connection,unsigned int type,unsigned int id,int policy,unsigned char wlanid,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int wtpid = 0;
	char fail_reason = 0;
	struct WtpList  *WtpNode = NULL;
	struct WtpList *wtp_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_INTERFACE,INTERFACE);

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SET_AP_L2_ISOLATION_ABLE);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_UINT32,&policy,
									DBUS_TYPE_BYTE,&wlanid,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((wtp_list_head = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(wtp_list_head,0,sizeof(struct WtpList));
			wtp_list_head->WtpList_list = NULL;
			wtp_list_head->WtpList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((WtpNode = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
					dcli_free_WtpList(wtp_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(WtpNode,0,sizeof(struct WtpList));
			WtpNode->next = NULL;

			if(wtp_list_head->WtpList_list == NULL){
				wtp_list_head->WtpList_list = WtpNode;
				wtp_list_head->next = WtpNode;
			}
			else{
				wtp_list_head->WtpList_last->next = WtpNode;
			}
			
			wtp_list_head->WtpList_last = WtpNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&wtpid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			WtpNode->FailReason = fail_reason;
			WtpNode->WtpId = wtpid;
		}
	}
	
	dbus_message_unref(reply);
	return wtp_list_head ;
}

 		
 	

struct WtpList * set_ap_dos_def_cmd_set_ap_dos_def(int index,int localid,
	DBusConnection *dbus_connection,unsigned int type,unsigned int id,int policy,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int wtpid = 0;
	char fail_reason = 0;
	struct WtpList  *WtpNode = NULL;
	struct WtpList *wtp_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_INTERFACE,INTERFACE);

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SET_AP_PREVENT_DOS_ATTACK);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_UINT32,&policy, 					 
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((wtp_list_head = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(wtp_list_head,0,sizeof(struct WtpList));
			wtp_list_head->WtpList_list = NULL;
			wtp_list_head->WtpList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((WtpNode = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
					dcli_free_WtpList(wtp_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(WtpNode,0,sizeof(struct WtpList));
			WtpNode->next = NULL;

			if(wtp_list_head->WtpList_list == NULL){
				wtp_list_head->WtpList_list = WtpNode;
				wtp_list_head->next = WtpNode;
			}
			else{
				wtp_list_head->WtpList_last->next = WtpNode;
			}
			
			wtp_list_head->WtpList_last = WtpNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&wtpid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			WtpNode->FailReason = fail_reason;
			WtpNode->WtpId = wtpid;
		}
	}
	
	dbus_message_unref(reply);
	return wtp_list_head ;
}

struct WtpList * set_ap_igmp_snoop_cmd_set_ap_igmp_snoop(int index,int localid,
	DBusConnection *dbus_connection,unsigned int type,unsigned int id,int policy,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int wtpid = 0;
	char fail_reason = 0;
	struct WtpList  *WtpNode = NULL;
	struct WtpList *wtp_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_INTERFACE,INTERFACE);

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SET_AP_IGMP_SNOOPING);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_UINT32,&policy, 					 
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((wtp_list_head = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(wtp_list_head,0,sizeof(struct WtpList));
			wtp_list_head->WtpList_list = NULL;
			wtp_list_head->WtpList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((WtpNode = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
					dcli_free_WtpList(wtp_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(WtpNode,0,sizeof(struct WtpList));
			WtpNode->next = NULL;

			if(wtp_list_head->WtpList_list == NULL){
				wtp_list_head->WtpList_list = WtpNode;
				wtp_list_head->next = WtpNode;
			}
			else{
				wtp_list_head->WtpList_last->next = WtpNode;
			}
			
			wtp_list_head->WtpList_last = WtpNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&wtpid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			WtpNode->FailReason = fail_reason;
			WtpNode->WtpId = wtpid;
		}
	}
	
	dbus_message_unref(reply);
	return wtp_list_head ;
}


struct WtpList * set_ap_sta_infomation_report_enable_cmd_set_ap_sta_infomation_report_switch(int index,int localid,
	DBusConnection *dbus_connection,unsigned int type,unsigned int id,int policy,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int wtpid = 0;
	char fail_reason = 0;
	struct WtpList  *WtpNode = NULL;
	struct WtpList *wtp_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_INTERFACE,INTERFACE);

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WTP_METHOD_SET_WTP_STA_INFOMATION_SWITCH);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_UINT32,&policy, 					 
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((wtp_list_head = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(wtp_list_head,0,sizeof(struct WtpList));
			wtp_list_head->WtpList_list = NULL;
			wtp_list_head->WtpList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((WtpNode = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
					dcli_free_WtpList(wtp_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(WtpNode,0,sizeof(struct WtpList));
			WtpNode->next = NULL;

			if(wtp_list_head->WtpList_list == NULL){
				wtp_list_head->WtpList_list = WtpNode;
				wtp_list_head->next = WtpNode;
			}
			else{
				wtp_list_head->WtpList_last->next = WtpNode;
			}
			
			wtp_list_head->WtpList_last = WtpNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&wtpid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			WtpNode->FailReason = fail_reason;
			WtpNode->WtpId = wtpid;
		}
	}
	
	dbus_message_unref(reply);
	return wtp_list_head ;
}

struct WtpList * set_ap_sta_infomation_reportinterval_cmd_set_ap_sta_infomation_reportinterval(int index,int localid,
	DBusConnection *dbus_connection,unsigned int type,unsigned int id,unsigned int interval,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int wtpid = 0;
	char fail_reason = 0;
	struct WtpList  *WtpNode = NULL;
	struct WtpList *wtp_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WTP_METHOD_SET_WTP_STA_INFOMATION_REPORTINTERVAL);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_UINT32,&interval, 					 
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	printf("dcli_ret= %d",*ret);


	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((wtp_list_head = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(wtp_list_head,0,sizeof(struct WtpList));
			wtp_list_head->WtpList_list = NULL;
			wtp_list_head->WtpList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((WtpNode = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
					dcli_free_WtpList(wtp_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(WtpNode,0,sizeof(struct WtpList));
			WtpNode->next = NULL;

			if(wtp_list_head->WtpList_list == NULL){
				wtp_list_head->WtpList_list = WtpNode;
				wtp_list_head->next = WtpNode;
			}
			else{
				wtp_list_head->WtpList_last->next = WtpNode;
			}
			
			wtp_list_head->WtpList_last = WtpNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&wtpid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			WtpNode->FailReason = fail_reason;
			WtpNode->WtpId = wtpid;
		}
	}
	
	dbus_message_unref(reply);
	return wtp_list_head ;
}


/*nl add for group 20100913*/
struct WtpList * set_ap_if_info_report_enable_cmd_sep_ap_interface_information_report_switch(int index,int localid,
	DBusConnection *dbus_connection,unsigned int type,unsigned int id,
	int policy, int *count,unsigned int *ret){
	
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int wtpid = 0;
	char fail_reason = 0;
	struct WtpList  *WtpNode = NULL;
	struct WtpList *wtp_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WTP_METHOD_SET_WTP_IF_INFO_SWITCH);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_UINT32,&policy, 					 
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((wtp_list_head = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(wtp_list_head,0,sizeof(struct WtpList));
			wtp_list_head->WtpList_list = NULL;
			wtp_list_head->WtpList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((WtpNode = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
					dcli_free_WtpList(wtp_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(WtpNode,0,sizeof(struct WtpList));
			WtpNode->next = NULL;

			if(wtp_list_head->WtpList_list == NULL){
				wtp_list_head->WtpList_list = WtpNode;
				wtp_list_head->next = WtpNode;
			}
			else{
				wtp_list_head->WtpList_last->next = WtpNode;
			}
			
			wtp_list_head->WtpList_last = WtpNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&wtpid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			WtpNode->FailReason = fail_reason;
			WtpNode->WtpId = wtpid;
		}
	}
	
	dbus_message_unref(reply);
	return wtp_list_head ;
}

/*nl add for group 20100913*/
struct WtpList * set_ap_if_info_report_enable_cmd_sep_ap_interface_information_report_interval
	(int index,int localid,DBusConnection *dbus_connection,unsigned int type,unsigned int id,
	unsigned int interval, int *count,unsigned int *ret){
	
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int wtpid = 0;
	char fail_reason = 0;
	struct WtpList  *WtpNode = NULL;
	struct WtpList *wtp_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										WID_DBUS_WTP_METHOD_SET_WTP_IF_INFO_REPORTINTERVAL);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_UINT32,&interval, 					 
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((wtp_list_head = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(wtp_list_head,0,sizeof(struct WtpList));
			wtp_list_head->WtpList_list = NULL;
			wtp_list_head->WtpList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((WtpNode = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
					dcli_free_WtpList(wtp_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(WtpNode,0,sizeof(struct WtpList));
			WtpNode->next = NULL;

			if(wtp_list_head->WtpList_list == NULL){
				wtp_list_head->WtpList_list = WtpNode;
				wtp_list_head->next = WtpNode;
			}
			else{
				wtp_list_head->WtpList_last->next = WtpNode;
			}
			
			wtp_list_head->WtpList_last = WtpNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&wtpid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			WtpNode->FailReason = fail_reason;
			WtpNode->WtpId = wtpid;
		}
	}
	
	dbus_message_unref(reply);
	return wtp_list_head ;
}

struct WtpList * set_ap_if_updown_cmd_set_ap_interface(int index,int localid,DBusConnection *dbus_connection,unsigned int type1,unsigned int id,
	unsigned char type,unsigned char ifindex,unsigned char policy,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int wtpid = 0;
	char fail_reason = 0;
	struct WtpList  *WtpNode = NULL;
	struct WtpList *wtp_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										WID_DBUS_WTP_METHOD_SET_WTP_INTERFACE_UPDOWN);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type1,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_BYTE,&type,
									DBUS_TYPE_BYTE,&ifindex,
									DBUS_TYPE_BYTE,&policy,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type1 == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((wtp_list_head = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(wtp_list_head,0,sizeof(struct WtpList));
			wtp_list_head->WtpList_list = NULL;
			wtp_list_head->WtpList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((WtpNode = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
					dcli_free_WtpList(wtp_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(WtpNode,0,sizeof(struct WtpList));
			WtpNode->next = NULL;

			if(wtp_list_head->WtpList_list == NULL){
				wtp_list_head->WtpList_list = WtpNode;
				wtp_list_head->next = WtpNode;
			}
			else{
				wtp_list_head->WtpList_last->next = WtpNode;
			}
			
			wtp_list_head->WtpList_last = WtpNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&wtpid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			WtpNode->FailReason = fail_reason;
			WtpNode->WtpId = wtpid;
		}
	}
	
	dbus_message_unref(reply);
	return wtp_list_head ;
}

struct WtpList * get_wtp_bss_pkt_info_cmd_update_wtp_bss_pakets_infomation(int index,int localid,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int wtpid = 0;
	char fail_reason = 0;
	struct WtpList  *WtpNode = NULL;
	struct WtpList *wtp_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										WID_DBUS_WTP_METHOD_GET_WTP_BSS_PKT_INFO);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((wtp_list_head = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(wtp_list_head,0,sizeof(struct WtpList));
			wtp_list_head->WtpList_list = NULL;
			wtp_list_head->WtpList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((WtpNode = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
					dcli_free_WtpList(wtp_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(WtpNode,0,sizeof(struct WtpList));
			WtpNode->next = NULL;

			if(wtp_list_head->WtpList_list == NULL){
				wtp_list_head->WtpList_list = WtpNode;
				wtp_list_head->next = WtpNode;
			}
			else{
				wtp_list_head->WtpList_last->next = WtpNode;
			}
			
			wtp_list_head->WtpList_last = WtpNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&wtpid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			WtpNode->FailReason = fail_reason;
			WtpNode->WtpId = wtpid;
		}
	}
	
	dbus_message_unref(reply);
	return wtp_list_head ;
}

struct WtpList * set_ap_if_eth_rate_cmd_set_ap_interface_eth_rate(int index,int localid,DBusConnection *dbus_connection,unsigned int type,
	unsigned int id,unsigned char ifindex,unsigned int rate,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int wtpid = 0;
	char fail_reason = 0;
	struct WtpList  *WtpNode = NULL;
	struct WtpList *wtp_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										WID_DBUS_WTP_METHOD_SET_WTP_INTERFACE_ETH_RATE);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_BYTE,&ifindex,
									DBUS_TYPE_UINT32,&rate,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((wtp_list_head = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(wtp_list_head,0,sizeof(struct WtpList));
			wtp_list_head->WtpList_list = NULL;
			wtp_list_head->WtpList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((WtpNode = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
					dcli_free_WtpList(wtp_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(WtpNode,0,sizeof(struct WtpList));
			WtpNode->next = NULL;

			if(wtp_list_head->WtpList_list == NULL){
				wtp_list_head->WtpList_list = WtpNode;
				wtp_list_head->next = WtpNode;
			}
			else{
				wtp_list_head->WtpList_last->next = WtpNode;
			}
			
			wtp_list_head->WtpList_last = WtpNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&wtpid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			WtpNode->FailReason = fail_reason;
			WtpNode->WtpId = wtpid;
		}
	}
	
	dbus_message_unref(reply);
	return wtp_list_head ;
}

struct WtpList * set_ac_ap_ntp_cmd_set_ap_ntp(int index,int localid,DBusConnection *dbus_connection,unsigned int type,
	unsigned int id,unsigned int policy,unsigned int ntpinterval,int *count,unsigned *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int wtpid = 0;
	char fail_reason = 0;
	struct WtpList  *WtpNode = NULL;
	struct WtpList *wtp_list_head = NULL; 
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										WID_DBUS_WTP_METHOD_SET_WTP_NTPCLIENT);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_UINT32,&policy,
									DBUS_TYPE_UINT32,&ntpinterval,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((wtp_list_head = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(wtp_list_head,0,sizeof(struct WtpList));
			wtp_list_head->WtpList_list = NULL;
			wtp_list_head->WtpList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((WtpNode = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
					dcli_free_WtpList(wtp_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(WtpNode,0,sizeof(struct WtpList));
			WtpNode->next = NULL;

			if(wtp_list_head->WtpList_list == NULL){
				wtp_list_head->WtpList_list = WtpNode;
				wtp_list_head->next = WtpNode;
			}
			else{
				wtp_list_head->WtpList_last->next = WtpNode;
			}
			
			wtp_list_head->WtpList_last = WtpNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&wtpid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			WtpNode->FailReason = fail_reason;
			WtpNode->WtpId = wtpid;
		}
	}
	
	dbus_message_unref(reply);
	return wtp_list_head ;
}

struct WtpList * set_ap_config_update_cmd_set_ap_config_update_from_ip(int index,int localid,DBusConnection *dbus_connection,unsigned int type,
	unsigned int id,char *ip,int *count,unsigned *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int wtpid = 0;
	char fail_reason = 0;
	struct WtpList  *WtpNode = NULL;
	struct WtpList *wtp_list_head = NULL; 
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										WID_DBUS_WTP_METHOD_UPDATE_WTP_CONFIG);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_STRING,&ip,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((wtp_list_head = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(wtp_list_head,0,sizeof(struct WtpList));
			wtp_list_head->WtpList_list = NULL;
			wtp_list_head->WtpList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((WtpNode = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
					dcli_free_WtpList(wtp_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(WtpNode,0,sizeof(struct WtpList));
			WtpNode->next = NULL;

			if(wtp_list_head->WtpList_list == NULL){
				wtp_list_head->WtpList_list = WtpNode;
				wtp_list_head->next = WtpNode;
			}
			else{
				wtp_list_head->WtpList_last->next = WtpNode;
			}
			
			wtp_list_head->WtpList_last = WtpNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&wtpid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			WtpNode->FailReason = fail_reason;
			WtpNode->WtpId = wtpid;
		}
	}
	
	dbus_message_unref(reply);
	return wtp_list_head ;
}

struct WtpList * set_ap_reboot_cmd_set_ap_reboot(int index,int localid,DBusConnection *dbus_connection,unsigned int type,
	unsigned int id,int *count,unsigned *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int wtpid = 0;
	char fail_reason = 0;
	struct WtpList  *WtpNode = NULL;
	struct WtpList *wtp_list_head = NULL; 
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										WID_DBUS_WTP_METHOD_SET_WTP_REBOOT);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((wtp_list_head = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(wtp_list_head,0,sizeof(struct WtpList));
			wtp_list_head->WtpList_list = NULL;
			wtp_list_head->WtpList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((WtpNode = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
					dcli_free_WtpList(wtp_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(WtpNode,0,sizeof(struct WtpList));
			WtpNode->next = NULL;

			if(wtp_list_head->WtpList_list == NULL){
				wtp_list_head->WtpList_list = WtpNode;
				wtp_list_head->next = WtpNode;
			}
			else{
				wtp_list_head->WtpList_last->next = WtpNode;
			}
			
			wtp_list_head->WtpList_last = WtpNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&wtpid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			WtpNode->FailReason = fail_reason;
			WtpNode->WtpId = wtpid;
		}
	}
	
	dbus_message_unref(reply);
	return wtp_list_head ;
}


struct WtpList * update_wtp_img_cmd_update_ap_img_file_version(int index,int localid,DBusConnection *dbus_connection,unsigned int type,
	unsigned int id,char *buf_version,char *buf_path,unsigned char commandmode,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int wtpid = 0;
	char fail_reason = 0;
	struct WtpList  *WtpNode = NULL;
	struct WtpList *wtp_list_head = NULL; 
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										WID_DBUS_WTP_METHOD_UPDATE_WTP_IMG);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_STRING,&buf_version,
									DBUS_TYPE_STRING,&buf_path,
									DBUS_TYPE_BYTE,&commandmode,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((wtp_list_head = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(wtp_list_head,0,sizeof(struct WtpList));
			wtp_list_head->WtpList_list = NULL;
			wtp_list_head->WtpList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((WtpNode = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
					dcli_free_WtpList(wtp_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(WtpNode,0,sizeof(struct WtpList));
			WtpNode->next = NULL;

			if(wtp_list_head->WtpList_list == NULL){
				wtp_list_head->WtpList_list = WtpNode;
				wtp_list_head->next = WtpNode;
			}
			else{
				wtp_list_head->WtpList_last->next = WtpNode;
			}
			
			wtp_list_head->WtpList_last = WtpNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&wtpid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			WtpNode->FailReason = fail_reason;
			WtpNode->WtpId = wtpid;
		}
	}
	
	dbus_message_unref(reply);
	return wtp_list_head ;
}

struct WtpList * clear_wtp_img_cmd_clear_ap_img_info(int index,int localid,DBusConnection *dbus_connection,unsigned int type,
	unsigned int id,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int wtpid = 0;
	char fail_reason = 0;
	struct WtpList  *WtpNode = NULL;
	struct WtpList *wtp_list_head = NULL; 
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										WID_DBUS_WTP_METHOD_CLEAR_UPDATE_WTP_IMG);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((wtp_list_head = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(wtp_list_head,0,sizeof(struct WtpList));
			wtp_list_head->WtpList_list = NULL;
			wtp_list_head->WtpList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((WtpNode = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
					dcli_free_WtpList(wtp_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(WtpNode,0,sizeof(struct WtpList));
			WtpNode->next = NULL;

			if(wtp_list_head->WtpList_list == NULL){
				wtp_list_head->WtpList_list = WtpNode;
				wtp_list_head->next = WtpNode;
			}
			else{
				wtp_list_head->WtpList_last->next = WtpNode;
			}
			
			wtp_list_head->WtpList_last = WtpNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&wtpid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			WtpNode->FailReason = fail_reason;
			WtpNode->WtpId = wtpid;
		}
	}
	
	dbus_message_unref(reply);
	return wtp_list_head ;
}

struct WtpList * set_wtp_sn_cmd_set_wtp_sn_SN(int index,int localid,DBusConnection *dbus_connection,unsigned int type,
	unsigned int id,char *name,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int wtpid = 0;
	char fail_reason = 0;
	struct WtpList  *WtpNode = NULL;
	struct WtpList *wtp_list_head = NULL; 
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										WID_DBUS_WTP_METHOD_SET_WTPSN);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_STRING,&name,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((wtp_list_head = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(wtp_list_head,0,sizeof(struct WtpList));
			wtp_list_head->WtpList_list = NULL;
			wtp_list_head->WtpList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((WtpNode = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
					dcli_free_WtpList(wtp_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(WtpNode,0,sizeof(struct WtpList));
			WtpNode->next = NULL;

			if(wtp_list_head->WtpList_list == NULL){
				wtp_list_head->WtpList_list = WtpNode;
				wtp_list_head->next = WtpNode;
			}
			else{
				wtp_list_head->WtpList_last->next = WtpNode;
			}
			
			wtp_list_head->WtpList_last = WtpNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&wtpid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			WtpNode->FailReason = fail_reason;
			WtpNode->WtpId = wtpid;
		}
	}
	
	dbus_message_unref(reply);
	return wtp_list_head ;
}

struct WtpList * set_ap_networkaddr_command_cmd_set_ap_network_address_mask_gateway_fstdns_snddns(int index,int localid,DBusConnection *dbus_connection,unsigned int type,
	unsigned int id,unsigned int ip,unsigned int mask,unsigned int gateway,unsigned int fstdns,
	unsigned int snddns,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int wtpid = 0;
	char fail_reason = 0;
	struct WtpList  *WtpNode = NULL;
	struct WtpList *wtp_list_head = NULL; 
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										WID_DBUS_CONF_METHOD_SET_WID_AP_IP_GATEWAY_DNS);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_UINT32,&ip,
									DBUS_TYPE_UINT32,&mask,
									DBUS_TYPE_UINT32,&gateway,
									DBUS_TYPE_UINT32,&fstdns,
									DBUS_TYPE_UINT32,&snddns,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((wtp_list_head = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(wtp_list_head,0,sizeof(struct WtpList));
			wtp_list_head->WtpList_list = NULL;
			wtp_list_head->WtpList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((WtpNode = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
					dcli_free_WtpList(wtp_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(WtpNode,0,sizeof(struct WtpList));
			WtpNode->next = NULL;

			if(wtp_list_head->WtpList_list == NULL){
				wtp_list_head->WtpList_list = WtpNode;
				wtp_list_head->next = WtpNode;
			}
			else{
				wtp_list_head->WtpList_last->next = WtpNode;
			}
			
			wtp_list_head->WtpList_last = WtpNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&wtpid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			WtpNode->FailReason = fail_reason;
			WtpNode->WtpId = wtpid;
		}
	}
	
	dbus_message_unref(reply);
	return wtp_list_head ;
}

struct WtpList * set_wtp_dhcp_snooping_enable_cmd_set_wtp_dhcp_snooping(int index,int localid,DBusConnection *dbus_connection,unsigned int type,
	unsigned int id,unsigned int policy,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int wtpid = 0;
	char fail_reason = 0;
	struct WtpList  *WtpNode = NULL;
	struct WtpList *wtp_list_head = NULL; 
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										WID_DBUS_WTP_METHOD_SET_DHCP_SNOOPING);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_UINT32,&policy,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((wtp_list_head = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(wtp_list_head,0,sizeof(struct WtpList));
			wtp_list_head->WtpList_list = NULL;
			wtp_list_head->WtpList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((WtpNode = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
					dcli_free_WtpList(wtp_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(WtpNode,0,sizeof(struct WtpList));
			WtpNode->next = NULL;

			if(wtp_list_head->WtpList_list == NULL){
				wtp_list_head->WtpList_list = WtpNode;
				wtp_list_head->next = WtpNode;
			}
			else{
				wtp_list_head->WtpList_last->next = WtpNode;
			}
			
			wtp_list_head->WtpList_last = WtpNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&wtpid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			WtpNode->FailReason = fail_reason;
			WtpNode->WtpId = wtpid;
		}
	}
	
	dbus_message_unref(reply);
	return wtp_list_head ;
}

struct WtpList * set_wtp_sta_info_report_enable_cmd_set_wtp_sta_info_report_enable(int index,int localid,DBusConnection *dbus_connection,unsigned int type,
	unsigned int id,unsigned int policy,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int wtpid = 0;
	char fail_reason = 0;
	struct WtpList  *WtpNode = NULL;
	struct WtpList *wtp_list_head = NULL; 
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										WID_DBUS_WTP_METHOD_STA_INFO_REPORT);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_UINT32,&policy,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((wtp_list_head = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(wtp_list_head,0,sizeof(struct WtpList));
			wtp_list_head->WtpList_list = NULL;
			wtp_list_head->WtpList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((WtpNode = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
					dcli_free_WtpList(wtp_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(WtpNode,0,sizeof(struct WtpList));
			WtpNode->next = NULL;

			if(wtp_list_head->WtpList_list == NULL){
				wtp_list_head->WtpList_list = WtpNode;
				wtp_list_head->next = WtpNode;
			}
			else{
				wtp_list_head->WtpList_last->next = WtpNode;
			}
			
			wtp_list_head->WtpList_last = WtpNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&wtpid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			WtpNode->FailReason = fail_reason;
			WtpNode->WtpId = wtpid;
		}
	}
	
	dbus_message_unref(reply);
	return wtp_list_head ;
}


struct WtpList * set_wtp_trap_switch_cmd_set_wtp_trap_switch(int index,int localid,DBusConnection *dbus_connection,unsigned int type,
	unsigned int id,unsigned int policy,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int wtpid = 0;
	char fail_reason = 0;
	struct WtpList  *WtpNode = NULL;
	struct WtpList *wtp_list_head = NULL; 
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										WID_DBUS_CONF_METHOD_SET_WTP_TRAP_SWITCH);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_UINT32,&policy,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((wtp_list_head = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(wtp_list_head,0,sizeof(struct WtpList));
			wtp_list_head->WtpList_list = NULL;
			wtp_list_head->WtpList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((WtpNode = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
					dcli_free_WtpList(wtp_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(WtpNode,0,sizeof(struct WtpList));
			WtpNode->next = NULL;

			if(wtp_list_head->WtpList_list == NULL){
				wtp_list_head->WtpList_list = WtpNode;
				wtp_list_head->next = WtpNode;
			}
			else{
				wtp_list_head->WtpList_last->next = WtpNode;
			}
			
			wtp_list_head->WtpList_last = WtpNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&wtpid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			WtpNode->FailReason = fail_reason;
			WtpNode->WtpId = wtpid;
		}
	}
	
	dbus_message_unref(reply);
	return wtp_list_head ;
}

/*wcl add*/


struct WtpList * set_wtp_seqnum_switch_cmd_set_wtp_seqnum_switch(int index,int localid,DBusConnection *dbus_connection,unsigned int type,
	unsigned int id,unsigned int policy,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int wtpid = 0;
	char fail_reason = 0;
	struct WtpList  *WtpNode = NULL;
	struct WtpList *wtp_list_head = NULL; 
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										WID_DBUS_CONF_METHOD_SET_WTP_SEQNUM_SWITCH);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_UINT32,&policy,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((wtp_list_head = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(wtp_list_head,0,sizeof(struct WtpList));
			wtp_list_head->WtpList_list = NULL;
			wtp_list_head->WtpList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((WtpNode = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
					dcli_free_WtpList(wtp_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(WtpNode,0,sizeof(struct WtpList));
			WtpNode->next = NULL;

			if(wtp_list_head->WtpList_list == NULL){
				wtp_list_head->WtpList_list = WtpNode;
				wtp_list_head->next = WtpNode;
			}
			else{
				wtp_list_head->WtpList_last->next = WtpNode;
			}
			
			wtp_list_head->WtpList_last = WtpNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&wtpid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			WtpNode->FailReason = fail_reason;
			WtpNode->WtpId = wtpid;
		}
	}
	
	dbus_message_unref(reply);
	return wtp_list_head ;
}

/*end*/
struct WtpList * set_ap_sta_wapi_info_report_enable_cmd_set_ap_sta_wapi_info_report_switch(int index,int localid,DBusConnection *dbus_connection,unsigned int type,
	unsigned int id,int policy,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int wtpid = 0;
	char fail_reason = 0;
	struct WtpList  *WtpNode = NULL;
	struct WtpList *wtp_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										WID_DBUS_WTP_METHOD_SET_WTP_STA_WAPI_INFO_SWITCH);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_UINT32,&policy,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((wtp_list_head = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(wtp_list_head,0,sizeof(struct WtpList));
			wtp_list_head->WtpList_list = NULL;
			wtp_list_head->WtpList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((WtpNode = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
					dcli_free_WtpList(wtp_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(WtpNode,0,sizeof(struct WtpList));
			WtpNode->next = NULL;

			if(wtp_list_head->WtpList_list == NULL){
				wtp_list_head->WtpList_list = WtpNode;
				wtp_list_head->next = WtpNode;
			}
			else{
				wtp_list_head->WtpList_last->next = WtpNode;
			}
			
			wtp_list_head->WtpList_last = WtpNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&wtpid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			WtpNode->FailReason = fail_reason;
			WtpNode->WtpId = wtpid;
		}
	}
	
	dbus_message_unref(reply);
	return wtp_list_head ;
}

struct WtpList * set_ap_sta_wapi_info_reportinterval_cmd_set_ap_sta_wapi_info_reportinterval(int index,int localid,DBusConnection *dbus_connection,unsigned int type,
	unsigned int id,unsigned char interval,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int wtpid = 0;
	char fail_reason = 0;
	struct WtpList  *WtpNode = NULL;
	struct WtpList *wtp_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WTP_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										WID_DBUS_WTP_METHOD_SET_WTP_STA_WAPI_INFO_REPORTINTERVAL);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_BYTE,&interval, 					 
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((wtp_list_head = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(wtp_list_head,0,sizeof(struct WtpList));
			wtp_list_head->WtpList_list = NULL;
			wtp_list_head->WtpList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((WtpNode = (struct WtpList*)malloc(sizeof(struct WtpList))) == NULL){
					dcli_free_WtpList(wtp_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(WtpNode,0,sizeof(struct WtpList));
			WtpNode->next = NULL;

			if(wtp_list_head->WtpList_list == NULL){
				wtp_list_head->WtpList_list = WtpNode;
				wtp_list_head->next = WtpNode;
			}
			else{
				wtp_list_head->WtpList_last->next = WtpNode;
			}
			
			wtp_list_head->WtpList_last = WtpNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&wtpid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			WtpNode->FailReason = fail_reason;
			WtpNode->WtpId = wtpid;
		}
	}
	
	dbus_message_unref(reply);
	return wtp_list_head ;
}


int show_wtp_running_config(int index,int localid,int wtpid,DBusConnection *dcli_dbus_connection,int wtp_max, char **showRunningCfg) {	
	char *showStr = NULL,*cursor = NULL;
	DBusMessage *query, *reply;
	DBusError err;
	int res1 = 0, res2 = 0;
	char *tmp = NULL;
	//int index;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SHOW_WTP_RUNNING_CONFIG);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&wtpid,	
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);

	if (NULL == reply) {
		//printf("show wtp config failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return -1;
	}
	char *showRunningCfg_str = NULL;
	int totalLen = 0;

	showRunningCfg_str = (char *)malloc(wtp_max*1024);
	if (NULL == showRunningCfg_str) {
		return 1;
	}
	memset(showRunningCfg_str, 0, (wtp_max*1024));
	cursor = showRunningCfg_str;
	totalLen += sprintf(cursor, "==================================================\n");
	cursor = showRunningCfg_str + totalLen;



	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_STRING, &showStr,
					DBUS_TYPE_INVALID)) 
	{
		totalLen += sprintf(cursor, "%s", showStr);
		cursor = showRunningCfg_str + totalLen;
		/* add string "exit" */
		totalLen += sprintf(cursor, "==================================================\n");
		cursor = showRunningCfg_str + totalLen;
		*showRunningCfg = showRunningCfg_str;
		//vty_out(vty, "%s", showRunningCfg_str);
	} 
	else 
	{
		//printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		if(showRunningCfg_str){
			free(showRunningCfg_str);
			showRunningCfg_str = NULL;
		}
		return -1;
	}
	dbus_message_unref(reply);
	//free(showRunningCfg_str);

	return 0;	
}

struct WtpWlanRadioInfo* show_Wlan_Radio_Info_of_all_Wtp(DBusConnection *dcli_dbus_connection, unsigned int *wlan_num, unsigned int *ret,int index,int localid){

	DBusMessage *query = NULL;
    DBusMessage *reply = NULL;
	DBusError err;
	DBusMessageIter iter;
	DBusMessageIter iter_wtp_array;
	DBusMessageIter iter_wtp;
	DBusMessageIter iter_wlan_array;
	DBusMessageIter	iter_wlan;
	int i=0,j=0;
	unsigned int WlanradioSendSpeed = 0;
	unsigned int WlanradioRecvSpeed = 0;

	struct WtpWlanRadioInfo *WtpNode = NULL;
	struct WtpWlanRadioInfo *WtpHead = NULL;
	struct WtpWlanRadioInfo *WtpTail = NULL;



	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SHOW_WLAN_RADIO_INFORMATION);
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);	
	dbus_message_unref(query);

	if (NULL == reply)
	{
	   if (dbus_error_is_set(&err))
		{
			dbus_error_free(&err);
		}
		
		return NULL;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,wlan_num); 	
		
	dbus_message_iter_next(&iter);	
	dbus_message_iter_recurse(&iter,&iter_wtp_array);
    if((*ret == 0)&&(*wlan_num !=0)){
      for(i=0;i<*wlan_num;i++){
			if((WtpNode = (struct WtpWlanRadioInfo*)malloc(sizeof(struct WtpWlanRadioInfo))) == NULL){
							dcli_free_wtp_wlan_radio_info_head(WtpHead);
							*ret = MALLOC_ERROR;
							dbus_message_unref(reply);
							return NULL;
					}

			memset(WtpNode,0,sizeof(struct WtpWlanRadioInfo));
			if(WtpHead == NULL){
					WtpHead = WtpNode;
					WtpTail = WtpNode;
				}
			else{
					WtpTail->next=WtpNode;
					WtpTail=WtpNode;
				}
			WtpTail->next = NULL;
			
			dbus_message_iter_recurse(&iter_wtp_array,&iter_wtp);
			dbus_message_iter_get_basic(&iter_wtp,&WtpNode->WlanradioSendSpeed);
            dbus_message_iter_next(&iter_wtp);
			dbus_message_iter_get_basic(&iter_wtp,&WtpNode->WlanradioRecvSpeed);
			dbus_message_iter_next(&iter_wtp);
			dbus_message_iter_get_basic(&iter_wtp,&WtpNode->wlanid);
            dbus_message_iter_next(&iter_wtp_array);
		  
		}

	}

	dbus_message_unref(reply);

	return WtpHead;
}
/*fengwenchao add 20110329 for dot11WtpStatisticsTable, dot11WtpChannelTable*/
struct WtpAthStatisticInfo * show_statistcs_information_of_all_wtp_whole(int index,int localid,DBusConnection *dcli_dbus_connection, unsigned int *wtp_num, unsigned int *ret)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	iter;
	DBusMessageIter	iter_wtp_array;
	DBusMessageIter iter_wtp;

	struct WtpAthStatisticInfo *WtpNode = NULL;
	struct WtpAthStatisticInfo *WtpHead = NULL;
	struct WtpAthStatisticInfo *WtpTail = NULL;
	struct WtpAthStatisticInfo *WtpSearch = NULL;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,\
						INTERFACE,WID_DBUS_CONF_METHOD_SHOW_STATISTICS_INFOMATION_OF_ALL_WTP_WHOLE);
					
				
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		*ret = WID_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,wtp_num);

	dbus_message_iter_next(&iter);
	dbus_message_iter_recurse(&iter,&iter_wtp_array);

	unsigned wtp_mac=NULL;
	unsigned wtp_id=0;
	
	if(*ret==0 && *wtp_num >0)
	{
				int i=0;
		for(i=0;i<*wtp_num;i++)
		{
			if((WtpNode = (struct WtpAthStatisticInfo*)malloc(sizeof(struct WtpAthStatisticInfo))) == NULL)
			{
				dcli_free_WtpAthStatisticInfo(WtpHead);
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			memset(WtpNode,0,sizeof(struct WtpAthStatisticInfo));
			if(WtpHead == NULL)
			{
				WtpHead = WtpNode;
				WtpTail = WtpNode;
			}
			else
			{
				WtpTail->next=WtpNode;
				WtpTail=WtpNode;
			}	
		
			dbus_message_iter_recurse(&iter_wtp_array,&iter_wtp);
			dbus_message_iter_get_basic(&iter_wtp,&wtp_id);

			dbus_message_iter_next(&iter_wtp);
			dbus_message_iter_get_basic(&iter_wtp,&wtp_mac);

			/*dbus_message_iter_next(&iter_wtp);
			dbus_message_iter_get_basic(&iter_wtp,&WtpNode->wtpRxPacketLossRate);

			dbus_message_iter_next(&iter_wtp);
			dbus_message_iter_get_basic(&iter_wtp,&WtpNode->wtpTxPacketLossRate);*/

			dbus_message_iter_next(&iter_wtp);
			dbus_message_iter_get_basic(&iter_wtp,&WtpNode->wtpTotalRx_Drop);

			dbus_message_iter_next(&iter_wtp);
			dbus_message_iter_get_basic(&iter_wtp,&WtpNode->wtpTotalTx_Drop);

			dbus_message_iter_next(&iter_wtp);
			dbus_message_iter_get_basic(&iter_wtp,&WtpNode->wtpTotalRx_Pkt);

			dbus_message_iter_next(&iter_wtp);
			dbus_message_iter_get_basic(&iter_wtp,&WtpNode->wtpTotalTx_Pkt);			

			dbus_message_iter_next(&iter_wtp);
			dbus_message_iter_get_basic(&iter_wtp,&WtpNode->wtpDownBandwidthUtilization);

			dbus_message_iter_next(&iter_wtp);
			dbus_message_iter_get_basic(&iter_wtp,&WtpNode->wtpUpBandwidthUtilization);

			dbus_message_iter_next(&iter_wtp);
			dbus_message_iter_get_basic(&iter_wtp,&WtpNode->wtpReceiveRate);

			dbus_message_iter_next(&iter_wtp);
			dbus_message_iter_get_basic(&iter_wtp,&WtpNode->wtpSendRate);

			dbus_message_iter_next(&iter_wtp);
			dbus_message_iter_get_basic(&iter_wtp,&WtpNode->ast_rx_phyerr);

			dbus_message_iter_next(&iter_wtp);
			dbus_message_iter_get_basic(&iter_wtp,&WtpNode->tx_packets);

			dbus_message_iter_next(&iter_wtp);
			dbus_message_iter_get_basic(&iter_wtp,&WtpNode->tx_errors);

			dbus_message_iter_next(&iter_wtp);
			dbus_message_iter_get_basic(&iter_wtp,&WtpNode->tx_retry);	

			dbus_message_iter_next(&iter_wtp);
			dbus_message_iter_get_basic(&iter_wtp,&WtpNode->monitor_time);

			dbus_message_iter_next(&iter_wtp);
			dbus_message_iter_get_basic(&iter_wtp,&WtpNode->wtpExtensionUsed);       //fengwenchao add 20110503  for dot11WtpExtensionTable

			dbus_message_iter_next(&iter_wtp);
			dbus_message_iter_get_basic(&iter_wtp,&WtpNode->wtpFloodAttackTimes);       //fengwenchao add 20110503  for dot11WtpWidStatisticsTable

			dbus_message_iter_next(&iter_wtp);
			dbus_message_iter_get_basic(&iter_wtp,&WtpNode->wtpSpoofingAttackTimes);       //fengwenchao add 20110503  for dot11WtpWidStatisticsTable

			dbus_message_iter_next(&iter_wtp);
			dbus_message_iter_get_basic(&iter_wtp,&WtpNode->wtpWeakIVAttackTimes);       //fengwenchao add 20110503  for dot11WtpWidStatisticsTable	

			dbus_message_iter_next(&iter_wtp);
			dbus_message_iter_get_basic(&iter_wtp,&WtpNode->BssIDTatolNum);       //fengwenchao add 20110503  for dot11WtpBssIDNumTable			
			
			//printf("WtpNode[%d].wtpRxPacketLossRate = %d\n",i,WtpNode[i].wtpRxPacketLossRate);
			//printf("WtpNode[%d].wtpTxPacketLossRate = %d\n",i,WtpNode[i].wtpTxPacketLossRate);
			//printf("WtpNode[%d].wtpDownBandwidthUtilization = %d\n",i,WtpNode[i].wtpDownBandwidthUtilization);
			//printf("WtpNode[%d].wtpUpBandwidthUtilization = %d\n",i,WtpNode[i].wtpUpBandwidthUtilization);
			//printf("WtpNode[%d].wtpReceiveRate = %d\n",i,WtpNode[i].wtpReceiveRate);
			//printf("WtpNode[%d].wtpSendRate = %d\n",i,WtpNode[i].wtpSendRate);
			
			WtpNode->wtpCurrID = wtp_id;
			
			if(!(WtpNode->wtpMacAddr=(unsigned char*)malloc(strlen(wtp_mac)+1))){
					dcli_free_WtpAthStatisticInfo(WtpHead);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
				}
			memset(WtpNode->wtpMacAddr,0,strlen(wtp_mac)+1);
			strncpy(WtpNode->wtpMacAddr,wtp_mac,strlen(wtp_mac));			

			dbus_message_iter_next(&iter_wtp_array);
		}
	}
	dbus_message_unref(reply);

	DBusMessage *query2, *reply2;
	DBusError err2;
	DBusMessageIter	 iter2;
	DBusMessageIter  iter_array2;

	DBusMessageIter iter_struct2;
	DBusMessageIter iter_sub_struct;
	//printf("!!!!!!!!!!!!!!!!!!!!!\n");
	unsigned int ret2;
	unsigned int asd_wtp_num = 0;
	unsigned int asd_wtp_id = 0;
	unsigned int radio_num = 0;
	unsigned int sta_num = 0;

	unsigned int bss_num = 0;
	unsigned char wlan_id;
	unsigned int radio_id;
	//printf("#################\n");
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	//printf("$$$$$$$$$$$$$$$$$\n");
	query2 = dbus_message_new_method_call(BUSNAME,OBJPATH,\
							INTERFACE,ASD_DBUS_CONF_METHOD_SHOW_STATISTICS_INFOMATION_OF_ALL_WTP_WHOLE);
									
	dbus_error_init(&err2);
	reply2 = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query2,-1, &err2);

	dbus_message_unref(query2);
	if (NULL == reply2) {
		if (dbus_error_is_set(&err2)) {
			dbus_error_free(&err2);
		}
		*ret=ASD_DBUS_ERROR;
		dcli_free_WtpAthStatisticInfo(WtpHead);
		//dbus_message_unref(reply2);
		return NULL;
	}
		
	dbus_message_iter_init(reply2,&iter2);
	dbus_message_iter_get_basic(&iter2,&ret2);

	dbus_message_iter_next(&iter2);	
	dbus_message_iter_get_basic(&iter2,&asd_wtp_num);

	if((ret2==0)&&(asd_wtp_num!=0))
	{
		dbus_message_iter_next(&iter2);	
	    dbus_message_iter_recurse(&iter2,&iter_array2);

		int i = 0;
		for(i=0;i<asd_wtp_num;i++)
		{
			dbus_message_iter_recurse(&iter_array2,&iter_struct2);
			dbus_message_iter_get_basic(&iter_struct2,&asd_wtp_id);

			dbus_message_iter_next(&iter_struct2);
			dbus_message_iter_get_basic(&iter_struct2,&sta_num);	

			//printf("sta_num = %d \n",sta_num);
			
			WtpSearch = WtpHead;
			while(WtpSearch != NULL)
			{
				if(asd_wtp_id == WtpSearch->wtpCurrID)
				{
					WtpSearch->sta_num = sta_num;
				}
				WtpSearch = WtpSearch->next;
			}
			dbus_message_iter_next(&iter_array2);
		}
	}
	dbus_message_unref(reply2);
	
	return WtpHead;
}
/*fengwenchao add end*/
void  dcli_add_black_white_oui_mac(unsigned int index,int localid,unsigned int *ret,unsigned char *oui_mac,unsigned int ouiXmlType,DBusConnection *dcli_dbus_connection){
	DBusMessage *query = NULL;
    DBusMessage *reply = NULL;
	DBusMessageIter  iter;
	DBusError err;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_ADD_BLACK_WHITE_OUI_MAC);
	dbus_error_init(&err);
	dbus_message_append_args(query,
									 DBUS_TYPE_UINT32,&ouiXmlType,
									 DBUS_TYPE_BYTE,&oui_mac[0],
									 DBUS_TYPE_BYTE,&oui_mac[1],
									 DBUS_TYPE_BYTE,&oui_mac[2],
									 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);	
	dbus_message_unref(query);
	if (NULL == reply)
	{
	   if (dbus_error_is_set(&err))
		{
			dbus_error_free(&err);
		}
		return NULL;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
}
void  dcli_del_black_white_oui_mac(unsigned int index,int localid,unsigned int *ret,unsigned char *oui_mac,unsigned int ouiXmlType,DBusConnection *dcli_dbus_connection){
	DBusMessage *query = NULL;
    DBusMessage *reply = NULL;
	DBusMessageIter  iter;
	DBusError err;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_DEL_BLACK_WHITE_OUI_MAC);
	dbus_error_init(&err);
	dbus_message_append_args(query,
									 DBUS_TYPE_UINT32,&ouiXmlType,
									 DBUS_TYPE_BYTE,&oui_mac[0],
									 DBUS_TYPE_BYTE,&oui_mac[1],
									 DBUS_TYPE_BYTE,&oui_mac[2],
									 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);	
	dbus_message_unref(query);
	if (NULL == reply)
	{
	   if (dbus_error_is_set(&err))
		{
			dbus_error_free(&err);
		}
		return NULL;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
}
void dcli_show_black_white_oui_info_list(unsigned int index,int localid,unsigned int *ret,unsigned int ouiXmlType,WtpOUIInfo **ouiInfoList,DBusConnection *dcli_dbus_connection){
	DBusMessage *query = NULL;
    DBusMessage *reply = NULL;
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusMessageIter  iter_struct;
	DBusError err;
	int i = 0;
	int oui_num = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	WtpOUIInfo * OuIInfoHeadNode = NULL;
	WtpOUIInfo * OuIInfoNode = NULL;
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SHOW_BLACK_WHITE_OUI_INFO);
	dbus_error_init(&err);
	dbus_message_append_args(query,
									 DBUS_TYPE_UINT32,&ouiXmlType,
									 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);	
	dbus_message_unref(query);
	if (NULL == reply)
	{
	   if (dbus_error_is_set(&err))
		{
			dbus_error_free(&err);
		}
		return NULL;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(oui_num));
	dbus_message_iter_next(&iter);	
	dbus_message_iter_recurse(&iter,&iter_array);
    if(*ret == 0){
	   CW_CREATE_OBJECT_ERR(OuIInfoHeadNode, WtpOUIInfo, return NULL;);
	   memset(OuIInfoHeadNode,0,sizeof(WtpOUIInfo));
	   OuIInfoHeadNode->oui_list = NULL;
	   OuIInfoHeadNode->oui_last = NULL;
	   OuIInfoHeadNode->next = NULL; 
       OuIInfoHeadNode->oui_num = oui_num;
		for(i =0;i<oui_num;i++){
			 CW_CREATE_OBJECT_ERR(OuIInfoNode, WtpOUIInfo, return NULL;);
		     memset(OuIInfoNode,0,sizeof(WtpOUIInfo));
			 CW_CREATE_STRING_ERR(OuIInfoNode->oui_mac, OUI_LEN, return NULL;);
			 memset(OuIInfoNode->oui_mac,0,(OUI_LEN+1));
			 if(OuIInfoHeadNode->oui_list == NULL){
                  OuIInfoHeadNode->oui_list = OuIInfoNode;
				  OuIInfoHeadNode->next = OuIInfoNode;
             }
			 else{
                  OuIInfoHeadNode->oui_last->next = OuIInfoNode;
			 }
             OuIInfoHeadNode->oui_last = OuIInfoNode;
			 dbus_message_iter_recurse(&iter_array,&iter_struct);
			 dbus_message_iter_get_basic(&iter_struct,&(OuIInfoNode->oui_mac[0]));
			 dbus_message_iter_next(&iter_struct);
	         dbus_message_iter_get_basic(&iter_struct,&(OuIInfoNode->oui_mac[1]));
			 dbus_message_iter_next(&iter_struct);
             dbus_message_iter_get_basic(&iter_struct,&(OuIInfoNode->oui_mac[2]));
			 dbus_message_iter_next(&iter_array);
		}
} 
 *ouiInfoList = OuIInfoHeadNode;
 dbus_message_unref(reply);
}
void dcli_free_oui_info_list(WtpOUIInfo *ouiInfoList){
	WtpOUIInfo * ouiInfoList_tmp = NULL;
	if(ouiInfoList == NULL)
		return;
	if(ouiInfoList->oui_last != NULL){
        ouiInfoList->oui_last = NULL;
    }
	while(ouiInfoList->oui_list != NULL){
		ouiInfoList_tmp = ouiInfoList->oui_list;
		ouiInfoList->oui_list = ouiInfoList_tmp->next;
		CW_FREE_OBJECT(ouiInfoList_tmp->oui_mac);
		ouiInfoList_tmp->next = NULL;
		CW_FREE_OBJECT(ouiInfoList_tmp);
     }
	CW_FREE_OBJECT(ouiInfoList);
	return ;
}
void dcli_update_black_white_oui_info_list(unsigned int index,int localid,unsigned int *ret,unsigned int ouiXmlType,DBusConnection *dcli_dbus_connection){
	DBusMessage *query = NULL;
    DBusMessage *reply = NULL;
	DBusMessageIter  iter;
	DBusError err;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_UPDATE_BLACK_WHITE_OUI_INFO_LIST);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							  DBUS_TYPE_UINT32,&ouiXmlType,
							  DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);	
	dbus_message_unref(query);
	if (NULL == reply)
	{
	   if (dbus_error_is_set(&err))
		{
			dbus_error_free(&err);
		}
		return NULL;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
}
void dcli_use_black_white_none_oui_policy(unsigned int index,int localid,unsigned int *ret,unsigned int ouiPolicyType,DBusConnection *dcli_dbus_connection){
	DBusMessage *query = NULL;
    DBusMessage *reply = NULL;
	DBusMessageIter  iter;
	DBusError err;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_USE_BLACK_WHITE_NONE_OUI_POLICY);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&ouiPolicyType,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);	
	dbus_message_unref(query);
	if (NULL == reply)
	{
	   if (dbus_error_is_set(&err))
		{
			dbus_error_free(&err);
		}
		return NULL;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
}
void dcli_show_oui_policy(unsigned int index,int localid,unsigned int *ret,unsigned int *ouiPolicyType,DBusConnection *dcli_dbus_connection){
	DBusMessage *query = NULL;
    DBusMessage *reply = NULL;
	DBusMessageIter  iter;
	DBusError err;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SHOW_OUI_POLICY);
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);	
	dbus_message_unref(query);
	if (NULL == reply)
	{
	   if (dbus_error_is_set(&err))
		{
			dbus_error_free(&err);
		}
		return NULL;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,ouiPolicyType);
}

void dcli_show_conflict_wtp_list(unsigned int index,int localid,unsigned int *ret,struct conflict_wtp_info **conflict_wtp,DBusConnection *dcli_dbus_connection){
	DBusMessage *query = NULL;
    DBusMessage *reply = NULL;
	DBusMessageIter  iter;
	DBusMessageIter	iter_array;
	DBusMessageIter iter_struct;
	DBusMessageIter iter_sub_array;
	DBusError err;
	int i = 0,j=0;
	unsigned int list_len = 0;
	unsigned int conf_wtp_count = 0;
	unsigned int wtpid = 0;

	struct ConflictWtp *tmp_node = NULL;
	struct conflict_wtp_info *con_list = NULL;
	struct conflict_wtp_info *list_node = NULL;
	char mac[MAC_LEN];
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SHOW_CONFLICT_WTP_LIST);
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);	
	dbus_message_unref(query);
	if (NULL == reply)
	{
	   if (dbus_error_is_set(&err))
		{
			dbus_error_free(&err);
		}
		return NULL;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&list_len);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_recurse(&iter,&iter_array);
	
	for (i = 0; i < list_len; i++) {
		memset(mac,0,MAC_LEN);
		list_node = malloc(sizeof(struct conflict_wtp_info));
		memset(list_node,0,sizeof(struct conflict_wtp_info));
		list_node->next = con_list;
		con_list = list_node;
		dbus_message_iter_recurse(&iter_array,&iter_struct);
	
		dbus_message_iter_get_basic(&iter_struct,&mac[0]);
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&mac[1]);
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&mac[2]);
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&mac[3]);
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&mac[4]);
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&mac[5]);
		dbus_message_iter_next(&iter_struct);
		
		dbus_message_iter_get_basic(&iter_struct,&conf_wtp_count);
		dbus_message_iter_next(&iter_struct);
		memcpy(list_node->wtpmac,mac,MAC_LEN);
		
		dbus_message_iter_recurse(&iter_struct,&iter_sub_array);
		j = 0;
		for(j=0;j<conf_wtp_count;j++){
			tmp_node = malloc(sizeof(struct ConflictWtp));
			memset(tmp_node,0,sizeof(struct ConflictWtp));
			tmp_node->next = list_node->wtpindexInfo;
			list_node->wtpindexInfo = tmp_node;
			DBusMessageIter iter_sub_struct;
			dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
			
			dbus_message_iter_get_basic(&iter_sub_struct,&wtpid);
			dbus_message_iter_next(&iter_sub_struct);

			tmp_node->wtpindex = wtpid;
			//printf("wtpid =%d.\n",wtpid);
			dbus_message_iter_next(&iter_sub_array);
		}
		dbus_message_iter_next(&iter_array);
	}

	*conflict_wtp = con_list;
	dbus_message_unref(reply);

	return ;

}
void conflict_wtp_list_free(struct conflict_wtp_info *conflict_wtp)
{
	struct ConflictWtp *tmp_node = NULL;
	struct ConflictWtp *tmp_node2 = NULL;
	struct conflict_wtp_info *list_node = NULL;
	struct conflict_wtp_info *list_node2 = NULL;

	list_node = conflict_wtp;
	while(list_node){
		tmp_node = list_node->wtpindexInfo;
		while(tmp_node){
			tmp_node2 = tmp_node;
			tmp_node = tmp_node->next;
			free(tmp_node2);
			tmp_node2 = NULL;
		}
		list_node2 = list_node;
		list_node = list_node->next;
		free(list_node2);
		list_node2 = NULL;
	}
	return NULL;
}


#endif


