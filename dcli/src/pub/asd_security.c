#ifdef _D_WCPSS_

#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <net/if_arp.h>
#include <dbus/dbus.h>

#include "wcpss/waw.h"
#include "wcpss/wid/WID.h"
#include "wcpss/asd/asd.h"
#include "dbus/asd/ASDDbusDef1.h"
#include "wid_ac.h"
#include "asd_sta.h"
#include "asd_security.h"

int RemoveListRepId(int list[],int num)
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
     return num; 

}

int parse_vlan_list(char* ptr,int* count,int vlanId[])
{
	char* endPtr = NULL;
	int   i=0;
	endPtr = ptr;
	vlan_list_state state = check_vlanid_state;

	while(1){
		switch(state){
			
		case check_vlanid_state: 
				vlanId[i] = strtoul(endPtr,&endPtr,10);
				if(vlanId[i]>0&&vlanId[i]<4095){
            		state=check_comma_state;
				}
				else
					state=check_fail_state;
				
				break;
		
		case check_comma_state: 
			if (SLOT_PORT_SPLIT_COMMA== endPtr[0]){
				endPtr = (char*)endPtr + 1;
				state = check_vlanid_state;
				i++;
				}
			else
				state = check_end_state;
			break;
				
		
		case check_fail_state:
			return -1;
			break;

		case check_end_state: 
			if ('\0' == endPtr[0]) {
				state = check_success_state;
				i++;
				}
			else
				state = check_fail_state;
				break;
			
		case check_success_state: 
			*count = i;
			return 0;
			break;
			
		default: break;
		}
		
		}
		
}

int parse_port(char* ptr,/*int* slot,int* port*/SLOT_PORT_VLAN_SECURITY* sp){

	char* endPtr = NULL;
    short slot = 0,port = 0;
    int   i=0;
	PARSE_PORT_STATE parse_state=TEST_SLOT_STATE;
	
    endPtr = ptr;

	/*use a state machine to parse the port.  six states.*/
	while(1){
			switch(parse_state)
			{
				case TEST_SLOT_STATE:
					slot = strtoul(endPtr,&endPtr,10);
					if(slot>0&&slot<=4){
            			sp->slot = slot;
						parse_state=TEST_SPLIT_STATE;
					}
					else
						parse_state=TEST_FAILURE_STATE;
					break;
					
				case TEST_SPLIT_STATE:
					if (SLOT_PORT_SPLIT_SLASH== endPtr[0]||SLOT_PORT_SPLIT_DASH == endPtr[0]) 
						parse_state=TEST_PORT_STATE;
					else
						parse_state=TEST_FAILURE_STATE;
					break;
					
				case TEST_PORT_STATE:
					port = strtoul((char *)&(endPtr[1]),&endPtr,10);
					if(port>0&&port<=24){
               			sp->port = port;
						parse_state=TEST_END_STATE;
					}
           			else
						parse_state=TEST_FAILURE_STATE;
					break;
					
				case TEST_END_STATE:
					if ('\0' == endPtr[0]) {
						parse_state=TEST_SUCESS_STATE;
						}
					else
						parse_state=TEST_FAILURE_STATE;
					break;
					
				case TEST_FAILURE_STATE:
					return -1;
					break;
					
				case TEST_SUCESS_STATE:
					return 0;
					break;
					
				default:
					break;
			}
	}
	return 0;
}

int _parse_port(char* ptr,/*int* slot,int* port*/SLOT_PORT_VLAN_ENABLE* sp){

	char* endPtr = NULL;
    short slot = 0,port = 0;
    int   i=0;
	PARSE_PORT_STATE parse_state=TEST_SLOT_STATE;
	
    endPtr = ptr;

	/*use a state machine to parse the port.  six states.*/
	while(1){
			switch(parse_state)
			{
				case TEST_SLOT_STATE:
					
					slot = strtoul(endPtr,&endPtr,10);
					/*printf("enter slot %d\n",slot);*/
					if(slot>0&&slot<=4){
            			sp->slot = slot;
						parse_state=TEST_SPLIT_STATE;
					}
					else
						parse_state=TEST_FAILURE_STATE;
					break;
					
				case TEST_SPLIT_STATE:
					/*printf("enter split\n");*/
					if (SLOT_PORT_SPLIT_SLASH== endPtr[0]||SLOT_PORT_SPLIT_DASH == endPtr[0]) 
						parse_state=TEST_PORT_STATE;
					else
						parse_state=TEST_FAILURE_STATE;
					break;
					
				case TEST_PORT_STATE:
					
					port = strtoul((char *)&(endPtr[1]),&endPtr,10);
					/*printf("enter port %d\n",port);*/
					if(port>0&&port<=24){
               			sp->port = port;
						parse_state=TEST_END_STATE;
					}
           			else
						parse_state=TEST_FAILURE_STATE;
					break;
				
					
				case TEST_END_STATE:
					/*printf("enter end\n");*/
					if ('\0' == endPtr[0]) {
						parse_state=TEST_SUCESS_STATE;
						
						}
					else
						parse_state=TEST_FAILURE_STATE;
					break;
					
				case TEST_FAILURE_STATE:
					/*printf("enter failure\n");*/
					return -1;
					
					break;
					
				case TEST_SUCESS_STATE:
					/*printf("enter sucess\n");*/
					
					return 0;
					break;
					
				default:
					break;
			}
	}
	return 0;
}

/*xm 08/08/27*/
int _parse_port_list(char* ptr,int* count,SLOT_PORT_S spL[])
{

	char* endPtr = NULL;
    short tmpslot = 0,tmpport = 0;
    int   i=0;
	int   isSame=0;
	PARSE_PORT_STATE parse_state=TEST_SLOT_STATE;

	
    endPtr = ptr;

	/*use a state machine to parse the port list.  seven states here.
	//this parse function can delete the same items in the list.*/

	while(1){

		switch(parse_state)
			{
				case TEST_SLOT_STATE:
					
					tmpslot = strtoul(endPtr,&endPtr,10);
					if(SLOT_LLEGAL(tmpslot)){
            			spL[i].slot = tmpslot;
						parse_state=TEST_SPLIT_STATE;
					}
					else
						parse_state=TEST_FAILURE_STATE;
					break;
					
				case TEST_SPLIT_STATE:
					if (SLOT_PORT_SPLIT_SLASH== endPtr[0]||SLOT_PORT_SPLIT_DASH == endPtr[0]) 
						parse_state=TEST_PORT_STATE;
					else
						parse_state=TEST_FAILURE_STATE;
					break;
					
				case TEST_PORT_STATE:
					tmpport = strtoul((char *)&(endPtr[1]),&endPtr,10);
					if(PORT_LLEGAL(tmpport)){
               			spL[i].port = tmpport;
						{
							int ii=0;
							for(ii=0;ii<i;ii++)
							if(spL[i].slot==spL[ii].slot&&spL[i].port==spL[ii].port)
								isSame=1;
						}
						parse_state=TEST_COMMA_STATE;
					}
           			else
						parse_state=TEST_FAILURE_STATE;
					break;
					
				case TEST_COMMA_STATE:
					if (SLOT_PORT_SPLIT_COMMA== endPtr[0]){
            				endPtr = (char*)endPtr + 1;
							if(isSame==1)
            					{
									isSame=0;
								}
							else
								i++;
								
							parse_state=TEST_SLOT_STATE;
						}
					else
						parse_state=TEST_END_STATE;
  					break;
					
				case TEST_END_STATE:
					if ('\0' == endPtr[0]) {
							parse_state=TEST_SUCESS_STATE;
							if(isSame==1)
								isSame=0;
							else
								i++;
						}
					else
						parse_state=TEST_FAILURE_STATE;
					break;
					
				case TEST_FAILURE_STATE:
					return -1;
					
					break;
					
				case TEST_SUCESS_STATE:
					*count = i;
					return 0;
					break;
					
				default:
					break;
			}
	}
	return 0;
}
/*xm 08/08/27*/

void CheckSecurityType(char *type, unsigned int SecurityType){

	switch(SecurityType){
		case OPEN :
			strcpy(type, "open");
			break;
			
		case SHARED :
			strcpy(type, "shared");
			break;
		
		case IEEE8021X :
			strcpy(type, "802.1x");
			break;

		case WPA_P :
			strcpy(type, "wpa_p");
			break;
			
		case WPA2_P :
			strcpy(type, "wpa2_p");
			break;

		case WPA2_E :
			strcpy(type, "wpa2_e");
			break;
			
		case WPA_E :
			strcpy(type, "wpa_e");
			break;

		case MD5:
			strcpy(type, "md5");
			break;

		case WAPI_PSK:
			strcpy(type, "wapi_psk");
			break;

		case WAPI_AUTH:
			strcpy(type, "wapi_auth");
			break;

		//mahz add 2011.3.1
		case HYBRID_AUTH_PORTAL:
			strcpy(type, "hybrid_auth_portal");
			break;
			
		case HYBRID_AUTH_EAPOL:
			strcpy(type, "hybrid_auth_eapol");
			break;
					
		case EXTENSIBLE_AUTH:
			strcpy(type, "extensible_auth");
			break;	
			
		case MAC_AUTH:
			strcpy(type, "mac_auth");
			break;	
		case NO_NEED_AUTH:
			strcpy(type, "no_need_auth");
			break;	
		case PORTAL_AUTH:
			strcpy(type, "portal_auth");
			break;	
				
	}

}

/*	xm0701*/
void CheckRekeyMethod(char *type, unsigned char SecurityType)
{
	switch(SecurityType){
		case 0 :
			strcpy(type, "disable");
			break;
			
		case 1 :
			strcpy(type, "time based");
			break;
		
		case 2 :
			strcpy(type, "packet based");
			break;

		case 3 :
			strcpy(type, "both based");
			break;
	}

}


void CheckEncryptionType(char *type, unsigned int EncryptionType){

	switch(EncryptionType){

		case NONE :
			strcpy(type, "none");
			break;
			
		case WEP :
			strcpy(type, "wep");
			break;
		
		case AES :
			strcpy(type, "aes");
			break;

		case TKIP :
			strcpy(type, "tkip");
			break;

		case SMS4 :
			strcpy(type, "sms4");
			break;
	}

}

int Check_Local_IP(char *check_ip)
{
	int 	fd, intrface;
	char 	addr[MAXNAMSIZ];
	struct ifreq buf[MAXINTERFACES];
	struct ifconf ifc;

	if ((fd = socket (AF_INET, SOCK_DGRAM, 0)) < 0) 
		return -1;

	ifc.ifc_len = sizeof(buf);
	ifc.ifc_buf = (char *) buf;
	if (ioctl(fd, SIOCGIFCONF, (char *)&ifc) != 0){ 
		close(fd);
		return -1;
	}

	intrface = ifc.ifc_len / sizeof(struct ifreq);
	while (intrface-- > 0) 	{
		if (ioctl (fd, SIOCGIFFLAGS, (char *) &buf[intrface])){ 
			close(fd);
			return -1;
		}

		if (buf[intrface].ifr_flags & IFF_UP) {
			if (ioctl (fd, SIOCGIFADDR, (char *)&buf[intrface])){
				close(fd);
				return -1;
			}
			
			memcpy(addr,inet_ntoa(((struct sockaddr_in*)(&buf[intrface].ifr_addr))->sin_addr),MAXNAMSIZ);

			if (!strncmp(check_ip,addr,MAXNAMSIZ-1)) {
				close(fd);
				return 1;
			}
		}
	}
	
	close (fd);
	return 0;
}

int Check_IP_Format(char* str){
	char *endptr = NULL;
	int endptr1 = 0;
	char c;
	int IP,i;
	c = str[0];
	if (c>='0'&&c<='9'){
		IP= strtoul(str,&endptr,10);
		if(IP < 0||IP > 255)
			return ASD_UNKNOWN_ID;
		else if(((IP < 10)&&((endptr - str) > 1))||((IP < 100)&&((endptr - str) > 2))||((IP < 256)&&((endptr - str) > 3))){
			return ASD_UNKNOWN_ID;
		}
		for(i = 0; i < 3; i++){
			if(endptr[0] == '\0'||endptr[0] != '.')
				return ASD_UNKNOWN_ID;
			else{
				endptr1 = &endptr[1];
				IP= strtoul(&endptr[1],&endptr,10);				
				if(IP < 0||IP > 255)
					return ASD_UNKNOWN_ID;				
				else if(((IP < 10)&&((endptr - endptr1) > 1))||((IP < 100)&&((endptr - endptr1) > 2))||((IP < 256)&&((endptr - endptr1) > 3))){
					return ASD_UNKNOWN_ID;
				}
			}
		}
		if(endptr[0] == '\0' && IP > 0)
			return ASD_DBUS_SUCCESS;
		else
			return ASD_UNKNOWN_ID;
	}
	else
		return ASD_UNKNOWN_ID;

}

void dcli_free_security(struct dcli_security *sec)
{
	if(sec == NULL)
		return ;
	if(sec->name != NULL){
		free(sec->name);
		sec->name = NULL;
	}
	if(sec->host_ip != NULL){
		free(sec->host_ip);
		sec->host_ip = NULL;
	}
	if(sec->SecurityKey != NULL){
		free(sec->SecurityKey);
		sec->SecurityKey = NULL;
	}
	if(sec->wapi_as.as_ip != NULL){
		free(sec->wapi_as.as_ip);
		sec->wapi_as.as_ip = NULL;
	}
	if(sec->wapi_as.certification_path != NULL){
		free(sec->wapi_as.certification_path);
		sec->wapi_as.certification_path = NULL;
	}
	if(sec->wapi_as.ae_cert_path != NULL){
		free(sec->wapi_as.ae_cert_path);
		sec->wapi_as.ae_cert_path = NULL;
	}	
	if(sec->wapi_as.ca_cert_path != NULL){
		free(sec->wapi_as.ca_cert_path);
		sec->wapi_as.ca_cert_path = NULL;
	}
	if(sec->wapi_as.unite_cert_path != NULL){
		free(sec->wapi_as.unite_cert_path);
		sec->wapi_as.unite_cert_path = NULL;
	}
	/*free auth and acct*/
	if(sec->auth.auth_ip != NULL){
		free(sec->auth.auth_ip);
		sec->auth.auth_ip = NULL;
	}
	if(sec->auth.auth_shared_secret != NULL){
		free(sec->auth.auth_shared_secret);
		sec->auth.auth_shared_secret = NULL;
	}
	if(sec->auth.secondary_auth_ip != NULL){
		free(sec->auth.secondary_auth_ip);
		sec->auth.secondary_auth_ip = NULL;
	}
	if(sec->auth.secondary_auth_shared_secret != NULL){
		free(sec->auth.secondary_auth_shared_secret);
		sec->auth.secondary_auth_shared_secret = NULL;
	}

	if(sec->acct.acct_ip != NULL){
		free(sec->acct.acct_ip);
		sec->acct.acct_ip = NULL;
	}
	if(sec->acct.acct_shared_secret != NULL){
		free(sec->acct.acct_shared_secret);
		sec->acct.acct_shared_secret = NULL;
	}
	if(sec->acct.secondary_acct_ip != NULL){
		free(sec->acct.secondary_acct_ip);
		sec->acct.secondary_acct_ip = NULL;
	}
	if(sec->acct.secondary_acct_shared_secret != NULL){
		free(sec->acct.secondary_acct_shared_secret);
		sec->acct.secondary_acct_shared_secret = NULL;
	}
	//mahz add 2010.12.10
	if(sec->user_passwd != NULL){
		free(sec->user_passwd);
		sec->user_passwd = NULL;
	}
	//	
	//qiuchen
	if(sec->ac_radius_name != NULL){
		free(sec->ac_radius_name);
		sec->ac_radius_name = NULL;
	}
	//end
	free(sec);
	sec = NULL;

	return ;
	
}

void dcli_free_security_list(struct dcli_security *sec)
{
	struct dcli_security *tmp = NULL;
	int i = 0;
	while(sec != NULL) {
		tmp = sec;
		sec = tmp->next;
		tmp->next = NULL;
		dcli_free_security(tmp);
	}

	return ;
}


struct dcli_security* show_security_list(DBusConnection *dcli_dbus_connection,int index, unsigned char *security_num, int localid, unsigned int *ret)
{
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusError	err;
	unsigned char 	*name = NULL;
	unsigned char 	*host_ip = NULL;
	unsigned int 	i,j;
	struct dcli_security *head = NULL;
	struct dcli_security *tail = NULL;
	struct dcli_security *tmp = NULL;
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SHOW_SECURITY_LIST);

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		*ret = ASD_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	if(*ret == 0){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,security_num);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);

		for(i=0; i<*security_num; i++){
			DBusMessageIter iter_struct;
			
			if((tmp = (struct dcli_security*)malloc(sizeof(*tmp))) == NULL){
				*ret = ASD_DBUS_MALLOC_FAIL;
				return NULL;
			}
			memset(tmp,0,sizeof(*tmp));
			if(head == NULL)
				head = tmp;
			else 
				tail->next = tmp;
			tail = tmp;

			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(tmp->SecurityID));
		
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(tmp->RadiusID));

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(name));
			if((tmp->name = (unsigned char *)malloc(strlen(name)+1)) != NULL) {
				memset(tmp->name,0,strlen(name)+1);
				memcpy(tmp->name,name,strlen(name));
			}
			
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(host_ip));
			if((tmp->host_ip = (unsigned char *)malloc(strlen(host_ip)+1)) != NULL) {
				memset(tmp->host_ip,0,strlen(host_ip)+1);
				memcpy(tmp->host_ip,host_ip,strlen(host_ip));
			}
					
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(tmp->SecurityType));
					
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(tmp->EncryptionType));
					
			dbus_message_iter_next(&iter_array);
		}

	}
	dbus_message_unref(reply);
	
	return head;
}


struct dcli_security* show_security_id(DBusConnection *dcli_dbus_connection,int index, unsigned char security_id, int localid, unsigned int *ret)
{
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusError	err;
	unsigned char	*key = NULL;
	unsigned char	*name = NULL;
	unsigned char	*host_ip = NULL;
	unsigned int	i,j;
	struct dcli_security *tmp = NULL;
	
	unsigned char	*as_ip = NULL;
	unsigned char	*certification_path = NULL;
	unsigned char	*ae_cert_path = NULL;
	unsigned char 	*ca_cert_path = NULL;

	char *auth_ip = NULL;
	char *auth_shared_secret = NULL;
	char *secondary_auth_ip = NULL;
	char *secondary_auth_shared_secret = NULL;

	char *acct_ip = NULL;
	char *acct_shared_secret = NULL;
	char *secondary_acct_ip = NULL;               
	char *secondary_acct_shared_secret = NULL;

	char *user_passwd = NULL;		//mahz add 2010.12.9
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	char *ac_radius_name = NULL;//qiuchen
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SHOW_SECURITY);

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&security_id,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		*ret = ASD_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	if((tmp = (struct dcli_security*)malloc(sizeof(*tmp))) == NULL){
		*ret = ASD_DBUS_MALLOC_FAIL;
		return NULL;
	}
	memset(tmp,0,sizeof(*tmp));

	if(dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,ret,
					DBUS_TYPE_BYTE,&(tmp->wapi_ucast_rekey_method),
					DBUS_TYPE_UINT32,&(tmp->wapi_ucast_rekey_para_t),
					DBUS_TYPE_UINT32,&(tmp->wapi_ucast_rekey_para_p),

					DBUS_TYPE_BYTE,&(tmp->wapi_mcast_rekey_method),
					DBUS_TYPE_UINT32,&(tmp->wapi_mcast_rekey_para_t),
					DBUS_TYPE_UINT32,&(tmp->wapi_mcast_rekey_para_p),	/*	xm0701*/
					
					DBUS_TYPE_STRING,&(name),
					DBUS_TYPE_BYTE,&(tmp->SecurityID),
					DBUS_TYPE_STRING,&(host_ip),
					DBUS_TYPE_UINT32,&(tmp->auth.auth_port),
					DBUS_TYPE_STRING,&auth_ip,
					DBUS_TYPE_STRING,&auth_shared_secret,

					DBUS_TYPE_UINT32,&(tmp->auth.secondary_auth_port),
					DBUS_TYPE_STRING,&secondary_auth_ip,
					DBUS_TYPE_STRING,&secondary_auth_shared_secret,	
					DBUS_TYPE_UINT32,&(tmp->acct.secondary_acct_port),
					DBUS_TYPE_STRING,&secondary_acct_ip,
					DBUS_TYPE_STRING,&secondary_acct_shared_secret,	
					DBUS_TYPE_UINT32,&(tmp->eap_reauth_period),			/*xm 08/09/03	*/	
					DBUS_TYPE_UINT32,&(tmp->acct_interim_interval),			/*ht 090206	*/
					DBUS_TYPE_UINT32,&(tmp->quiet_period),			/*ht 090727 */

					DBUS_TYPE_UINT32,&(tmp->acct.acct_port),
					DBUS_TYPE_STRING,&acct_ip,
					DBUS_TYPE_STRING,&acct_shared_secret,					
					DBUS_TYPE_UINT32,&(tmp->SecurityType),
					DBUS_TYPE_UINT32,&(tmp->EncryptionType),
					DBUS_TYPE_STRING,&(key),
					DBUS_TYPE_UINT32,&(tmp->keyInputType),    /*xm add 08/11/25*/
					DBUS_TYPE_UINT32,&(tmp->extensible_auth),
					DBUS_TYPE_UINT32,&(tmp->wired_radius),
					DBUS_TYPE_UINT32,&(tmp->wapi_radius_auth),		//mahz add
					DBUS_TYPE_STRING,&user_passwd,					//mahz add 2010.12.9
			
					DBUS_TYPE_STRING,&as_ip,
					DBUS_TYPE_BYTE,&(tmp->wapi_as.multi_cert),
					DBUS_TYPE_STRING,&certification_path,
					DBUS_TYPE_STRING,&ae_cert_path,
					DBUS_TYPE_STRING,&ca_cert_path,
					DBUS_TYPE_UINT32,&(tmp->wapi_as.certification_type),
					DBUS_TYPE_BYTE,&(tmp->security_index),	
					DBUS_TYPE_UINT32,&(tmp->hybrid_auth),		//mahz add 2011.2.28
					DBUS_TYPE_UINT32,&(tmp->mac_auth),
					DBUS_TYPE_BYTE,&(tmp->eap_sm_run_activated),//Qc	
					//qiuchen add it for master_bak radius server configuration 2012.12.11
					DBUS_TYPE_BYTE,&(tmp->radius_server_binding_type),
					DBUS_TYPE_BYTE,&(tmp->radius_heart_test_type),
					DBUS_TYPE_DOUBLE,&(tmp->radius_res_fail_percent),
					DBUS_TYPE_DOUBLE,&(tmp->radius_res_suc_percent),
					DBUS_TYPE_UINT32,&(tmp->radius_access_test_interval),
					DBUS_TYPE_UINT32,&(tmp->radius_server_change_test_timer),
					DBUS_TYPE_UINT32,&(tmp->radius_server_reuse_test_timer),
					DBUS_TYPE_STRING,&(ac_radius_name),
					DBUS_TYPE_BYTE,&(tmp->heart_test_on),
					DBUS_TYPE_BYTE,&(tmp->acct_server_current_use),
					DBUS_TYPE_BYTE,&(tmp->auth_server_current_use),
					//end
					DBUS_TYPE_INVALID))	{
		if((name != NULL) && ((tmp->name = (unsigned char *)malloc(strlen(name)+1)) != NULL)) {
			memset(tmp->name,0,strlen(name)+1);
			memcpy(tmp->name,name,strlen(name));
		}
		if((host_ip != NULL) && ((tmp->host_ip = (unsigned char *)malloc(strlen(host_ip)+1)) != NULL)) {
			memset(tmp->host_ip,0,strlen(host_ip)+1);
			memcpy(tmp->host_ip,host_ip,strlen(host_ip));
		}
		if((key != NULL) && ((tmp->SecurityKey = (unsigned char *)malloc(strlen(key)+1)) != NULL)) {
			memset(tmp->SecurityKey,0,strlen(key)+1);
			memcpy(tmp->SecurityKey,key,strlen(key));
		}
		if((as_ip != NULL) && ((tmp->wapi_as.as_ip = (unsigned char *)malloc(strlen(as_ip)+1)) != NULL)) {
			memset(tmp->wapi_as.as_ip,0,strlen(as_ip)+1);
			memcpy(tmp->wapi_as.as_ip,as_ip,strlen(as_ip));
			
		}
		if((certification_path != NULL) && ((tmp->wapi_as.certification_path = (unsigned char *)malloc(strlen(certification_path)+1)) != NULL)) {
			memset(tmp->wapi_as.certification_path,0,strlen(certification_path)+1);
			memcpy(tmp->wapi_as.certification_path,certification_path,strlen(certification_path));
			
		}	
		if((ae_cert_path != NULL) && ((tmp->wapi_as.ae_cert_path = (unsigned char *)malloc(strlen(ae_cert_path)+1)) != NULL)) {
			memset(tmp->wapi_as.ae_cert_path,0,strlen(ae_cert_path)+1);
			memcpy(tmp->wapi_as.ae_cert_path,ae_cert_path,strlen(ae_cert_path));
			
		}
		if((ca_cert_path != NULL) && ((tmp->wapi_as.ca_cert_path = (unsigned char *)malloc(strlen(ca_cert_path)+1)) != NULL)) {
			memset(tmp->wapi_as.ca_cert_path,0,strlen(ca_cert_path)+1);
			memcpy(tmp->wapi_as.ca_cert_path,ca_cert_path,strlen(ca_cert_path));
			
		}
		if((ca_cert_path != NULL) && ((tmp->wapi_as.ca_cert_path = (unsigned char *)malloc(strlen(ca_cert_path)+1)) != NULL)) {
			memset(tmp->wapi_as.ca_cert_path,0,strlen(ca_cert_path)+1);
			memcpy(tmp->wapi_as.ca_cert_path,ca_cert_path,strlen(ca_cert_path));
			
		}
		/*for auth*/
		if((auth_ip != NULL) && ((tmp->auth.auth_ip = (unsigned char *)malloc(strlen(auth_ip)+1)) != NULL)) {
			memset(tmp->auth.auth_ip,0,strlen(auth_ip)+1);
			memcpy(tmp->auth.auth_ip,auth_ip,strlen(auth_ip));
			
		}
		if((auth_shared_secret != NULL) && ((tmp->auth.auth_shared_secret = (unsigned char *)malloc(strlen(auth_shared_secret)+1)) != NULL)) {
			memset(tmp->auth.auth_shared_secret,0,strlen(auth_shared_secret)+1);
			memcpy(tmp->auth.auth_shared_secret,auth_shared_secret,strlen(auth_shared_secret));
			
		}
		if((secondary_auth_ip != NULL) && ((tmp->auth.secondary_auth_ip = (unsigned char *)malloc(strlen(secondary_auth_ip)+1)) != NULL)) {
			memset(tmp->auth.secondary_auth_ip,0,strlen(secondary_auth_ip)+1);
			memcpy(tmp->auth.secondary_auth_ip,secondary_auth_ip,strlen(secondary_auth_ip));
			
		}
		if((secondary_auth_shared_secret != NULL) && ((tmp->auth.secondary_auth_shared_secret = (unsigned char *)malloc(strlen(secondary_auth_shared_secret)+1)) != NULL)) {
			memset(tmp->auth.secondary_auth_shared_secret,0,strlen(secondary_auth_shared_secret)+1);
			memcpy(tmp->auth.secondary_auth_shared_secret,secondary_auth_shared_secret,strlen(secondary_auth_shared_secret));
			
		}
		/*for acct*/
		if((acct_ip != NULL) && ((tmp->acct.acct_ip = (unsigned char *)malloc(strlen(acct_ip)+1)) != NULL)) {
			memset(tmp->acct.acct_ip,0,strlen(acct_ip)+1);
			memcpy(tmp->acct.acct_ip,acct_ip,strlen(acct_ip));
			
		}
		if((acct_shared_secret != NULL) && ((tmp->acct.acct_shared_secret = (unsigned char *)malloc(strlen(acct_shared_secret)+1)) != NULL)) {
			memset(tmp->acct.acct_shared_secret,0,strlen(acct_shared_secret)+1);
			memcpy(tmp->acct.acct_shared_secret,acct_shared_secret,strlen(acct_shared_secret));
			
		}
		if((secondary_acct_ip != NULL) && ((tmp->acct.secondary_acct_ip = (unsigned char *)malloc(strlen(secondary_acct_ip)+1)) != NULL)) {
			memset(tmp->acct.secondary_acct_ip,0,strlen(secondary_acct_ip)+1);
			memcpy(tmp->acct.secondary_acct_ip,secondary_acct_ip,strlen(secondary_acct_ip));
			
		}
		if((secondary_acct_shared_secret != NULL) && ((tmp->acct.secondary_acct_shared_secret = (unsigned char *)malloc(strlen(secondary_acct_shared_secret)+1)) != NULL)) {
			memset(tmp->acct.secondary_acct_shared_secret,0,strlen(secondary_acct_shared_secret)+1);
			memcpy(tmp->acct.secondary_acct_shared_secret,secondary_acct_shared_secret,strlen(secondary_acct_shared_secret));
			
		}

		//mahz add 2010.12.9
		if((user_passwd != NULL) && ((tmp->user_passwd = (unsigned char *)malloc(strlen(user_passwd)+1)) != NULL)) {
			memset(tmp->user_passwd,0,strlen(user_passwd)+1);
			memcpy(tmp->user_passwd,user_passwd,strlen(user_passwd));
			
		}
		//
		//qiuchen add 2012.12.11
		if((ac_radius_name != NULL) && ((tmp->ac_radius_name = malloc(strlen(ac_radius_name)+1)) != NULL)){
			memset(tmp->ac_radius_name,0,strlen(ac_radius_name)+1);
			memcpy(tmp->ac_radius_name,ac_radius_name,strlen(ac_radius_name));
		}
		//end
	}

	dbus_message_unref(reply);
	
	return tmp;
}

struct dcli_security* show_radius_id(DBusConnection *dcli_dbus_connection,int index, unsigned char security_id, int localid, unsigned int *ret)
{
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusError	err;
	unsigned int	i,j;
	struct dcli_security *tmp = NULL;

	char *auth_ip = NULL;
	char *auth_shared_secret = NULL;
	char *secondary_auth_ip = NULL;
	char *secondary_auth_shared_secret = NULL;

	char *acct_ip = NULL;
	char *acct_shared_secret = NULL;
	char *secondary_acct_ip = NULL;               
	char *secondary_acct_shared_secret = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SHOW_RADIUS);

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&security_id,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		*ret = ASD_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	if((tmp = (struct dcli_security*)malloc(sizeof(*tmp))) == NULL){
		*ret = ASD_DBUS_MALLOC_FAIL;
		return NULL;
	}
	memset(tmp,0,sizeof(*tmp));

	if(dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,ret,
					
					DBUS_TYPE_UINT32,&(tmp->auth.auth_port),
					DBUS_TYPE_STRING,&auth_ip,
					DBUS_TYPE_STRING,&auth_shared_secret,
					DBUS_TYPE_UINT32,&(tmp->auth.secondary_auth_port),
					DBUS_TYPE_STRING,&secondary_auth_ip,
					DBUS_TYPE_STRING,&secondary_auth_shared_secret,	
	
					DBUS_TYPE_UINT32,&(tmp->acct.acct_port),
					DBUS_TYPE_STRING,&acct_ip,
					DBUS_TYPE_STRING,&acct_shared_secret,					
					DBUS_TYPE_UINT32,&(tmp->acct.secondary_acct_port),
					DBUS_TYPE_STRING,&secondary_acct_ip,
					DBUS_TYPE_STRING,&secondary_acct_shared_secret,	
					DBUS_TYPE_INVALID))	{
					/*for auth*/
		if((auth_ip != NULL) && ((tmp->auth.auth_ip = (unsigned char *)malloc(strlen(auth_ip)+1)) != NULL)) {
			memset(tmp->auth.auth_ip,0,strlen(auth_ip)+1);
			memcpy(tmp->auth.auth_ip,auth_ip,strlen(auth_ip));
			
		}
		if((auth_shared_secret != NULL) && ((tmp->auth.auth_shared_secret = (unsigned char *)malloc(strlen(auth_shared_secret)+1)) != NULL)) {
			memset(tmp->auth.auth_shared_secret,0,strlen(auth_shared_secret)+1);
			memcpy(tmp->auth.auth_shared_secret,auth_shared_secret,strlen(auth_shared_secret));
			
		}
		if((secondary_auth_ip != NULL) && ((tmp->auth.secondary_auth_ip = (unsigned char *)malloc(strlen(secondary_auth_ip)+1)) != NULL)) {
			memset(tmp->auth.secondary_auth_ip,0,strlen(secondary_auth_ip)+1);
			memcpy(tmp->auth.secondary_auth_ip,secondary_auth_ip,strlen(secondary_auth_ip));
			
		}
		if((secondary_auth_shared_secret != NULL) && ((tmp->auth.secondary_auth_shared_secret = (unsigned char *)malloc(strlen(secondary_auth_shared_secret)+1)) != NULL)) {
			memset(tmp->auth.secondary_auth_shared_secret,0,strlen(secondary_auth_shared_secret)+1);
			memcpy(tmp->auth.secondary_auth_shared_secret,secondary_auth_shared_secret,strlen(secondary_auth_shared_secret));
			
		}
		/*for acct*/
		if((acct_ip != NULL) && ((tmp->acct.acct_ip = (unsigned char *)malloc(strlen(acct_ip)+1)) != NULL)) {
			memset(tmp->acct.acct_ip,0,strlen(acct_ip)+1);
			memcpy(tmp->acct.acct_ip,acct_ip,strlen(acct_ip));
			
		}
		if((acct_shared_secret != NULL) && ((tmp->acct.acct_shared_secret = (unsigned char *)malloc(strlen(acct_shared_secret)+1)) != NULL)) {
			memset(tmp->acct.acct_shared_secret,0,strlen(acct_shared_secret)+1);
			memcpy(tmp->acct.acct_shared_secret,acct_shared_secret,strlen(acct_shared_secret));
			
		}
		if((secondary_acct_ip != NULL) && ((tmp->acct.secondary_acct_ip = (unsigned char *)malloc(strlen(secondary_acct_ip)+1)) != NULL)) {
			memset(tmp->acct.secondary_acct_ip,0,strlen(secondary_acct_ip)+1);
			memcpy(tmp->acct.secondary_acct_ip,secondary_acct_ip,strlen(secondary_acct_ip));
			
		}
		if((secondary_acct_shared_secret != NULL) && ((tmp->acct.secondary_acct_shared_secret = (unsigned char *)malloc(strlen(secondary_acct_shared_secret)+1)) != NULL)) {
			memset(tmp->acct.secondary_acct_shared_secret,0,strlen(secondary_acct_shared_secret)+1);
			memcpy(tmp->acct.secondary_acct_shared_secret,secondary_acct_shared_secret,strlen(secondary_acct_shared_secret));
			
		}
			
	}

	dbus_message_unref(reply);
	
	return tmp;
}

/*nl add 20100112*/
struct dcli_security* show_security_wapi_info(DBusConnection *dcli_dbus_connection,int index, unsigned char security_id, int localid, unsigned int *ret)
{
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusError	err;
	struct dcli_security *sec = NULL;
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SHOW_SECURITY_WAPI_INFO);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
						 DBUS_TYPE_BYTE,&security_id,
						 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		*ret = ASD_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	if((sec = (struct dcli_security*)malloc(sizeof(*sec))) == NULL){
		*ret = ASD_DBUS_MALLOC_FAIL;
		return NULL;
	}
	memset(sec,0,sizeof(*sec));
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	if(*ret == 0){
		dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&sec->CertificateUpdateCount);
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&sec->MulticastUpdateCount);
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&sec->UnicastUpdateCount);
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&sec->BKLifetime);
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&sec->BKReauthThreshold);
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&sec->SATimeout);
		
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&sec->WapiPreauth);
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&sec->MulticaseRekeyStrict);
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&sec->UnicastCipherEnabled);
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&sec->AuthenticationSuiteEnable);
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&sec->MulticastCipher[0]);
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&sec->MulticastCipher[1]);
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&sec->MulticastCipher[2]);
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&sec->MulticastCipher[3]);

	}
	
	dbus_message_unref(reply);
	
	return sec;
}

/*nl add 20100125*/
struct dcli_security* show_wlan_security_wapi_info(DBusConnection *dcli_dbus_connection,int index, unsigned char wlan_id, int localid, unsigned int *ret)
{
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusError	err;
	struct dcli_security *sec = NULL;
	char *ae_cert_path;
	char *certification_path;
	char *as_ip;
	char *SecurityKey;

	int keytype = -1;
		
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);

	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SHOW_WLAN_SECURITY_WAPI_CONF);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
						 DBUS_TYPE_BYTE,&wlan_id,
						 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		*ret = ASD_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	if((sec = (struct dcli_security*)malloc(sizeof(*sec))) == NULL){
		*ret = ASD_DBUS_MALLOC_FAIL;
		return NULL;
	}
	memset(sec,0,sizeof(*sec));
	
	/*dbus_message_get_args ( reply, &err,
	DBUS_TYPE_UINT32,ret,
	DBUS_TYPE_UINT32,&sec->CertificateUpdateCount,
	DBUS_TYPE_UINT32,&sec->MulticastUpdateCount,
	DBUS_TYPE_UINT32,&sec->UnicastUpdateCount,

	DBUS_TYPE_UINT32,&sec->BKLifetime,
	DBUS_TYPE_UINT32,&sec->BKReauthThreshold,
	DBUS_TYPE_UINT32,&sec->SATimeout,	
				
	DBUS_TYPE_BYTE,&sec->WapiPreauth,
	DBUS_TYPE_BYTE,&sec->MulticaseRekeyStrict,
	DBUS_TYPE_BYTE,&sec->UnicastCipherEnabled,
	DBUS_TYPE_BYTE,&sec->AuthenticationSuiteEnable,
	DBUS_TYPE_BYTE,&sec->MulticastCipher[0],
	DBUS_TYPE_BYTE,&sec->MulticastCipher[1],
	DBUS_TYPE_BYTE,&sec->MulticastCipher[2],
	DBUS_TYPE_BYTE,&sec->MulticastCipher[3],
	DBUS_TYPE_STRING,&as_ip,

	DBUS_TYPE_BYTE,&sec->wapi_ucast_rekey_method,	
	DBUS_TYPE_UINT32,&sec->wapi_ucast_rekey_para_t,
	DBUS_TYPE_UINT32,&sec->wapi_ucast_rekey_para_p,
	DBUS_TYPE_BYTE,&sec->wapi_mcast_rekey_method,
	DBUS_TYPE_UINT32,&sec->wapi_mcast_rekey_para_t,
	DBUS_TYPE_UINT32,&sec->wapi_mcast_rekey_para_p,
	
	DBUS_TYPE_STRING,&(sec->wapi_as.certification_path),
	DBUS_TYPE_STRING,&(sec->wapi_as.ae_cert_path),
	DBUS_TYPE_UINT32,&(sec->wapi_as.certification_type),
	DBUS_TYPE_UINT32,&sec->SecurityType,
	DBUS_TYPE_UINT32,&sec->EncryptionType,
	DBUS_TYPE_BYTE,&sec->SecurityID,
	DBUS_TYPE_STRING,&SecurityKey,
	DBUS_TYPE_UINT32,&sec->keyInputType,				
	DBUS_TYPE_INVALID);		
	
	dbus_message_unref(reply);*/

	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	if(*ret == 0){

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&sec->CertificateUpdateCount);			
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&sec->MulticastUpdateCount);	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&sec->UnicastUpdateCount); 
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&sec->BKLifetime);	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&sec->BKReauthThreshold);	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&sec->SATimeout);	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&sec->WapiPreauth);		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&sec->MulticaseRekeyStrict);	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&sec->UnicastCipherEnabled);	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&sec->AuthenticationSuiteEnable);	
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&sec->MulticastCipher[0]);	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&sec->MulticastCipher[1]);	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&sec->MulticastCipher[2]);	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&sec->MulticastCipher[3]);	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&as_ip);	

		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&sec->wapi_ucast_rekey_method);	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&sec->wapi_ucast_rekey_para_t);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&sec->wapi_ucast_rekey_para_p);	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&sec->wapi_mcast_rekey_method);	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&sec->wapi_mcast_rekey_para_t);	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&sec->wapi_mcast_rekey_para_p);	

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&certification_path);	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&ae_cert_path);
		dbus_message_iter_next(&iter);	

		dbus_message_iter_get_basic(&iter,&sec->wapi_as.certification_type);	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&sec->SecurityType);	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&sec->EncryptionType);
		dbus_message_iter_next(&iter);	


		dbus_message_iter_get_basic(&iter,&sec->SecurityID);		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&SecurityKey);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&sec->keyInputType);
		dbus_message_iter_next(&iter);	
		

	
		if((as_ip != NULL) && ((sec->wapi_as.as_ip = (unsigned char *)malloc(strlen(as_ip)+1)) != NULL)) {
			memset(sec->wapi_as.as_ip,0,strlen(as_ip)+1);
			memcpy(sec->wapi_as.as_ip,as_ip,strlen(as_ip));
			
		}

		if((certification_path != NULL) && ((sec->wapi_as.certification_path = (unsigned char *)malloc(strlen(certification_path)+1)) != NULL)) {
			memset(sec->wapi_as.certification_path,0,strlen(certification_path)+1);
			memcpy(sec->wapi_as.certification_path,certification_path,strlen(certification_path));
			
		}
		
		if((ae_cert_path != NULL) && ((sec->wapi_as.ae_cert_path = (unsigned char *)malloc(strlen(ae_cert_path)+1)) != NULL)) {
			memset(sec->wapi_as.ae_cert_path,0,strlen(ae_cert_path)+1);
			memcpy(sec->wapi_as.ae_cert_path,ae_cert_path,strlen(ae_cert_path));
			
		}

		if((SecurityKey != NULL) && ((sec->SecurityKey = (unsigned char *)malloc(strlen(SecurityKey)+1)) != NULL)) {
			memset(sec->SecurityKey,0,strlen(SecurityKey)+1);
			memcpy(sec->SecurityKey,SecurityKey,strlen(SecurityKey));
			
		}

		sec->ConfigVersion = 1;
		sec->WapiSupported = 1;
		sec->WapiPreauthEnabled = 0;
		sec->UnicastKeysSupported = 2;
		sec->AuthenticationSuiteEnabled = 1;
		sec->MulticastRekeyStrict = 0;
		
		sec->MulticastRekeyPackets = 1000;	
		sec->UnicastRekeyPackets= 1000;
		sec->UnicastCipherSize = 256;
		sec->MulticastCipherSize = 256;
		
		unsigned char UnicastCipherSelected[4] = {0x00, 0x14, 0x72, 0x01};
		unsigned char MulticastCipherSelected[4] = {0x00, 0x14, 0x72, 0x01};
		unsigned char UnicastCipherRequested[4] = {0x00, 0x14, 0x72, 0x01};
		unsigned char MulticastCipherRequested[4] = {0x00, 0x14, 0x72, 0x01};
		unsigned char MulticastCipher[4] = {0x00, 0x14, 0x72, 0x01};
		unsigned char UnicastCipher[4] = {0x00, 0x14, 0x72, 0x01};
		unsigned char AuthSuitSelected_Auth[4] = {0x00, 0x14, 0x72, 0x01};	
		unsigned char AuthSuitRequested_Psk[4] = {0x00, 0x14, 0x72, 0x02};
	    unsigned char AuthenticationSuite[4] = 	{0x00, 0x14, 0x72, 0x02};	

		memcpy(sec->UnicastCipherSelected,UnicastCipherSelected,4);
		memcpy(sec->MulticastCipherSelected,MulticastCipherSelected,4);
		memcpy(sec->UnicastCipherRequested,UnicastCipherRequested,4);
		memcpy(sec->MulticastCipherRequested,MulticastCipherRequested,4);
		memcpy(sec->MulticastCipher,MulticastCipher,4);
		memcpy(sec->UnicastCipher,UnicastCipher,4);
		memcpy(sec->AuthSuitSelected_Auth,AuthSuitSelected_Auth,4);
		memcpy(sec->AuthSuitRequested_Psk,AuthSuitRequested_Psk,4);
		memcpy(sec->AuthenticationSuite,AuthenticationSuite,4);

		if(strcmp(sec->wapi_as.ae_cert_path, " ") == 0){
			sec->IsInstalledCer = 0;	
		}
		else {	
			sec->IsInstalledCer = 1;
		}

		if (sec->AuthenticationSuiteEnable == 1){
			sec->ControlledAuthControl = 1;		
		}
		else if (sec->AuthenticationSuiteEnable== 0){
			sec->ControlledAuthControl = 0;	
		}

		CheckRekeyMethod(sec->RekeyMethod,sec->wapi_ucast_rekey_method);
		CheckRekeyMethod(sec->RekeyMethod,sec->wapi_mcast_rekey_method);
		
	}

	dbus_message_unref(reply);
	
	return sec;	
}



//mahz add for mib request , 2011.1.27 , dot11AKMConfigTable
struct dcli_security* show_security_wapi_config_of_all_wlan(DBusConnection *dcli_dbus_connection,int index, unsigned int *wlan_num, int localid, int *ret)
{
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_wlan_array;
	DBusMessageIter  iter_wlan_struct;
	DBusError	err;
	struct dcli_security *sec = NULL;
	struct dcli_security *head = NULL;
	struct dcli_security *tmp = NULL;
	int i = 0;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SHOW_SECURITY_WAPI_CONF_OF_ALL_WLAN);
	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);

	if (NULL == reply) {
		*ret = ASD_DBUS_ERROR;
		//printf("reply is null\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,wlan_num);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_recurse(&iter,&iter_wlan_array);

	if((*ret == 0)&&(*wlan_num != 0)){		
		for(i=0;i<*wlan_num;i++){
			if((sec = (struct dcli_security*)malloc(sizeof(*sec))) == NULL){
				*ret = ASD_DBUS_MALLOC_FAIL;
				dbus_message_unref(reply);	
				return NULL;
			}
			memset(sec,0,sizeof(*sec));

			if(head == NULL)
				head = sec;
			else
				tmp->next = sec;
			tmp = sec;

			dbus_message_iter_recurse(&iter_wlan_array,&iter_wlan_struct);
			dbus_message_iter_get_basic(&iter_wlan_struct,&sec->SecurityID);
			//在此利用security_id 来接收wlan_id，复用sec 结构体

			dbus_message_iter_next(&iter_wlan_struct);	
			dbus_message_iter_get_basic(&iter_wlan_struct,&sec->AuthenticationSuiteEnable);//传值与接收值不一致
			dbus_message_iter_next(&iter_wlan_struct);	
			dbus_message_iter_get_basic(&iter_wlan_struct,&sec->AuthenticationSuite[0]);	
			dbus_message_iter_next(&iter_wlan_struct);	
			dbus_message_iter_get_basic(&iter_wlan_struct,&sec->AuthenticationSuite[1]); 
			dbus_message_iter_next(&iter_wlan_struct);	
			dbus_message_iter_get_basic(&iter_wlan_struct,&sec->AuthenticationSuite[2]);	
			dbus_message_iter_next(&iter_wlan_struct);	
			dbus_message_iter_get_basic(&iter_wlan_struct,&sec->AuthenticationSuite[3]);	

			sec->AuthenticationSuiteEnabled = 1;
			//unsigned char MulticastCipher[4] = {0x00, 0x14, 0x72, 0x01};
		    unsigned char AuthenticationSuite[4] = 	{0x00, 0x14, 0x72, 0x02};	
			memcpy(sec->AuthenticationSuite,AuthenticationSuite,4);

			dbus_message_iter_next(&iter_wlan_array);
		}
	}
	dbus_message_unref(reply);
	return head;	
}


void set_wapi_p12_cert_path(DBusConnection *dcli_dbus_connection, int index, unsigned char security_id, unsigned int type, 
	unsigned char *cert_path,  unsigned char *password, int localid, unsigned int *ret)
{
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError	err;
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SET_WAPI_P12_CERT_PATH);

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&security_id,
							 DBUS_TYPE_UINT32,&type,
							 DBUS_TYPE_STRING,&cert_path,
							 DBUS_TYPE_STRING,&password,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);


	free(cert_path);
	cert_path = NULL;
	free(password);
	password = NULL;
	
	if (NULL == reply) {
		*ret = ASD_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return ;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	dbus_message_unref(reply);
	
	return ;
}


//mahz add for mib request , 2011.1.12
struct dcli_security* show_radius_all(DBusConnection *dcli_dbus_connection,int index, unsigned char *security_num, int localid, unsigned int *ret)
{
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusError	err;
	
	unsigned int	i,j;
	struct dcli_security *head = NULL;
	struct dcli_security *tail = NULL;
	struct dcli_security *tmp = NULL;

	char *auth_ip = NULL;
	char *auth_shared_secret = NULL;
	char *secondary_auth_ip = NULL;
	char *secondary_auth_shared_secret = NULL;

	char *acct_ip = NULL;
	char *acct_shared_secret = NULL;
	char *secondary_acct_ip = NULL;               
	char *secondary_acct_shared_secret = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SHOW_RADIUS_ALL);

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		*ret = ASD_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,security_num);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_recurse(&iter,&iter_array);
	
	if(*ret == 0){		

		for(i=0; i<*security_num; i++){

			if((tmp = (struct dcli_security*)malloc(sizeof(*tmp))) == NULL){
				*ret = ASD_DBUS_MALLOC_FAIL;
				return NULL;
			}
			memset(tmp,0,sizeof(*tmp));
			if(head == NULL)
				head = tmp;
			else 
				tail->next = tmp;
			tail = tmp;
			
			DBusMessageIter iter_struct;
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(tmp->SecurityID));
		
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(tmp->RadiusID));

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(tmp->auth.auth_port));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(auth_ip));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(auth_shared_secret));

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(tmp->auth.secondary_auth_port));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(secondary_auth_ip));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(secondary_auth_shared_secret));

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(tmp->acct.acct_port));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(acct_ip));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(acct_shared_secret));

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(tmp->acct.secondary_acct_port));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(secondary_acct_ip));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(secondary_acct_shared_secret));
								
			dbus_message_iter_next(&iter_array);

			//for auth
			if((auth_ip != NULL) && ((tmp->auth.auth_ip = (unsigned char *)malloc(strlen(auth_ip)+1)) != NULL)) {
				memset(tmp->auth.auth_ip,0,strlen(auth_ip)+1);
				memcpy(tmp->auth.auth_ip,auth_ip,strlen(auth_ip));
			}

			if((auth_shared_secret != NULL) && ((tmp->auth.auth_shared_secret = (unsigned char *)malloc(strlen(auth_shared_secret)+1)) != NULL)) {
				memset(tmp->auth.auth_shared_secret,0,strlen(auth_shared_secret)+1);
				memcpy(tmp->auth.auth_shared_secret,auth_shared_secret,strlen(auth_shared_secret));
			}

			if((secondary_auth_ip != NULL) && ((tmp->auth.secondary_auth_ip = (unsigned char *)malloc(strlen(secondary_auth_ip)+1)) != NULL)) {
				memset(tmp->auth.secondary_auth_ip,0,strlen(secondary_auth_ip)+1);
				memcpy(tmp->auth.secondary_auth_ip,secondary_auth_ip,strlen(secondary_auth_ip));
			}

			if((secondary_auth_shared_secret != NULL) && ((tmp->auth.secondary_auth_shared_secret = (unsigned char *)malloc(strlen(secondary_auth_shared_secret)+1)) != NULL)) {
				memset(tmp->auth.secondary_auth_shared_secret,0,strlen(secondary_auth_shared_secret)+1);
				memcpy(tmp->auth.secondary_auth_shared_secret,secondary_auth_shared_secret,strlen(secondary_auth_shared_secret));
			}

			//	for acct
			if((acct_ip != NULL) && ((tmp->acct.acct_ip = (unsigned char *)malloc(strlen(acct_ip)+1)) != NULL)) {
				memset(tmp->acct.acct_ip,0,strlen(acct_ip)+1);
				memcpy(tmp->acct.acct_ip,acct_ip,strlen(acct_ip));
			}

			if((acct_shared_secret != NULL) && ((tmp->acct.acct_shared_secret = (unsigned char *)malloc(strlen(acct_shared_secret)+1)) != NULL)) {
				memset(tmp->acct.acct_shared_secret,0,strlen(acct_shared_secret)+1);
				memcpy(tmp->acct.acct_shared_secret,acct_shared_secret,strlen(acct_shared_secret));
			}

			if((secondary_acct_ip != NULL) && ((tmp->acct.secondary_acct_ip = (unsigned char *)malloc(strlen(secondary_acct_ip)+1)) != NULL)) {
				memset(tmp->acct.secondary_acct_ip,0,strlen(secondary_acct_ip)+1);
				memcpy(tmp->acct.secondary_acct_ip,secondary_acct_ip,strlen(secondary_acct_ip));
			}

			if((secondary_acct_shared_secret != NULL) && ((tmp->acct.secondary_acct_shared_secret = (unsigned char *)malloc(strlen(secondary_acct_shared_secret)+1)) != NULL)) {
				memset(tmp->acct.secondary_acct_shared_secret,0,strlen(secondary_acct_shared_secret)+1);
				memcpy(tmp->acct.secondary_acct_shared_secret,secondary_acct_shared_secret,strlen(secondary_acct_shared_secret));
			}
		}
	}

	dbus_message_unref(reply);

	return head;
}

//mahz add 2011.11.18
struct dcli_security* show_asd_rdc_info(DBusConnection *dcli_dbus_connection,int index, unsigned char *security_num, int localid, unsigned int *ret)
{
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusError	err;
	
	unsigned int	i,j;
	struct dcli_security *head = NULL;
	struct dcli_security *tail = NULL;
	struct dcli_security *tmp = NULL;

	unsigned int slot_value;
	unsigned int inst_value;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SHOW_ASD_RDC_INFO);

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		*ret = ASD_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,security_num);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_recurse(&iter,&iter_array);
	
	if(*ret == 0){		
		for(i=0; i<*security_num; i++){
			if((tmp = (struct dcli_security*)malloc(sizeof(*tmp))) == NULL){
				*ret = ASD_DBUS_MALLOC_FAIL;
				return NULL;
			}
			memset(tmp,0,sizeof(*tmp));
			if(head == NULL)
				head = tmp;
			else 
				tail->next = tmp;
			tail = tmp;
			
			DBusMessageIter iter_struct;
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(tmp->SecurityID));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(tmp->slot_value));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(tmp->inst_value));
			dbus_message_iter_next(&iter_array);
		}
	}

	dbus_message_unref(reply);
	return head;
}

#endif

