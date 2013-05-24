#include <dbus/dbus.h>
#include <stdbool.h>
#include <time.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <net/if_arp.h>
#include <net/ethernet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/msg.h>

#include "hmd.h"
#include "HmdDbus.h"
#include "HmdThread.h"
#include "HmdLog.h"
#include "hmd/hmdpub.h"
#include "dbus/hmd/HmdDbusDef.h"
#include "dbus/wcpss/ACDbusDef1.h"
#include "dbus/asd/ASDDbusDef1.h"
#include "dbus/wcpss/wsm_dbus_def.h"
#include <dbus/npd/npd_dbus_def.h>

#define VRRP_RETURN_CODE_OK (0x150001)

int vrrp_send_pkt_arp( char* ifname, char *buffer, int buflen )
{
#if 1
	struct sockaddr from;
	int	len;
	int	fd = socket(PF_PACKET, SOCK_PACKET, 0x300); /* 0x300 is magic */
// WORK:
	if( fd < 0 ){
		perror( "socket" );
		return -1;
	}

	/*
	vrrp_syslog_dbg("create socket fd %d to send packet from intf %s\n",fd,ifname);
      */
	/* build the address */
	memset( &from, 0 , sizeof(from));
	strcpy( from.sa_data, ifname );

	/* send the data */
	len = sendto( fd, buffer, buflen, 0, &from, sizeof(from) );
   
	close( fd );
	return len;
#endif
}


int send_tunnel_interface_arp(char *MAC, int addr, char* ifname )
{
struct m_arphdr
  {
    unsigned short int ar_hrd;          /* Format of hardware address.  */
    unsigned short int ar_pro;          /* Format of protocol address.  */
    unsigned char ar_hln;               /* Length of hardware address.  */
    unsigned char ar_pln;               /* Length of protocol address.  */
    unsigned short int ar_op;           /* ARP opcode (command).  */
    /* Ethernet looks like this : This bit is variable sized however...  */
    unsigned char __ar_sha[ETH_ALEN];   /* Sender hardware address.  */
    unsigned char __ar_sip[4];          /* Sender IP address.  */
    unsigned char __ar_tha[ETH_ALEN];   /* Target hardware address.  */
    unsigned char __ar_tip[4];          /* Target IP address.  */
  };
	char	buf[sizeof(struct m_arphdr)+ETHER_HDR_LEN];
	char	buflen	= sizeof(struct m_arphdr)+ETHER_HDR_LEN;
	struct ether_header 	*eth	= (struct ether_header *)buf;
	struct m_arphdr	*arph = (struct m_arphdr *)(buf+ETHER_HDR_LEN);
	char	*hwaddr	= MAC;
	int	hwlen	= ETH_ALEN;

	/* hardcoded for ethernet */
	memset( eth->ether_dhost, 0xFF, ETH_ALEN );
	memcpy( eth->ether_shost, hwaddr, hwlen );
	eth->ether_type	= htons(ETHERTYPE_ARP);

	/* build the arp payload */
	memset( arph, 0, sizeof( *arph ) );
	arph->ar_hrd	= htons(ARPHRD_ETHER);
	arph->ar_pro	= htons(ETHERTYPE_IP);
	arph->ar_hln	= 6;
	arph->ar_pln	= 4;
	arph->ar_op	= htons(ARPOP_REQUEST);
	memcpy( arph->__ar_sha, hwaddr, hwlen );
	addr = htonl(addr);
	memcpy( arph->__ar_sip, &addr, sizeof(addr) );
	memcpy( arph->__ar_tip, &addr, sizeof(addr) );
	return vrrp_send_pkt_arp( ifname, buf, buflen );
}


/* book add for had dbus,2011-5-24 */
void HMDReInitHadDbusPath(int index, char * path, char * newpath, int islocaled)
{
	sprintf(newpath,"%s%d",path,index);	
}


void HMDReInitDbusPath(int index, char * path, char * newpath, int islocaled)
{
	int len;
	//if(islocaled)
	sprintf(newpath,"%s%d_%d",path,islocaled,index);			
	//else
	//	sprintf(newpath,"%s%d",path,index);	
	if(path == ASD_DBUS_SECURITY_OBJPATH){
			len = strlen(newpath);
			sprintf(newpath+len,"/%s","security");
	}
	else if(path == ASD_DBUS_SECURITY_INTERFACE)
	{	
			len = strlen(newpath);
			sprintf(newpath+len,".%s","security");			
	}
	else if(path == ASD_DBUS_STA_OBJPATH){
			len = strlen(newpath);
			sprintf(newpath+len,"/%s","sta");
	}
	else if(path == ASD_DBUS_STA_INTERFACE)
	{	
			len = strlen(newpath);
			sprintf(newpath+len,".%s","sta");			
	}
	else if(path == WID_DBUS_WLAN_OBJPATH){
			len = strlen(newpath);
			sprintf(newpath+len,"/%s","wlan");
	}
	else if(path == WID_DBUS_WLAN_INTERFACE)
	{	
			len = strlen(newpath);
			sprintf(newpath+len,".%s","wlan");			
	}
	else if(path == WID_DBUS_WTP_OBJPATH){
			len = strlen(newpath);
			sprintf(newpath+len,"/%s","wtp");
	}
	else if(path == WID_DBUS_WTP_INTERFACE)
	{	
			len = strlen(newpath);
			sprintf(newpath+len,".%s","wtp");			
	}
	else if(path == WID_DBUS_RADIO_OBJPATH){
			len = strlen(newpath);
			sprintf(newpath+len,"/%s","radio");
	}
	else if(path == WID_DBUS_RADIO_INTERFACE)
	{	
			len = strlen(newpath);
			sprintf(newpath+len,".%s","radio");			
	}
	else if(path == WID_DBUS_QOS_OBJPATH){
			len = strlen(newpath);
			sprintf(newpath+len,"/%s","qos");
	}
	else if(path == WID_DBUS_QOS_INTERFACE)
	{	
			len = strlen(newpath);
			sprintf(newpath+len,".%s","qos");			
	}
	else if(path == WID_DBUS_EBR_OBJPATH){
			len = strlen(newpath);
			sprintf(newpath+len,"/%s","ebr");
	}
	else if(path == WID_DBUS_EBR_INTERFACE)
	{	
			len = strlen(newpath);
			sprintf(newpath+len,".%s","ebr");			
	}	
	else if(path == WID_BAK_OBJPATH){
			len = strlen(newpath);
			sprintf(newpath+len,"/%s","bak");
	}
	else if(path == WID_BAK_INTERFACE)
	{	
			len = strlen(newpath);
			sprintf(newpath+len,".%s","bak");			
	}
	else if(path == WID_DBUS_ACIPLIST_OBJPATH){
			len = strlen(newpath);
			sprintf(newpath+len,"/%s","aciplist");
	}
	else if(path == WID_DBUS_ACIPLIST_INTERFACE)
	{	
			len = strlen(newpath);
			sprintf(newpath+len,".%s","aciplist");			
	}
	else if(path == ASD_DBUS_AC_GROUP_OBJPATH){
			len = strlen(newpath);
			sprintf(newpath+len,"/%s","acgroup");
	}
	else if(path == ASD_DBUS_AC_GROUP_INTERFACE)
	{	
			len = strlen(newpath);
			sprintf(newpath+len,".%s","acgroup");			
	}
	else if(path == WID_DBUS_AP_GROUP_OBJPATH){
			len = strlen(newpath);
			sprintf(newpath+len,"/%s","apgroup");
	}
	else if(path == WID_DBUS_AP_GROUP_INTERFACE)
	{	
			len = strlen(newpath);
			sprintf(newpath+len,".%s","apgroup");			
	}
}


DBusConnection * hmd_dbus_connection_init(int InstID, int islocaled)
{	
	int i = 0;
	DBusConnection * dbus_connection = NULL;
	DBusError dbus_error;
	dbus_threads_init_default();	
	dbus_error_init (&dbus_error);
	dbus_connection = dbus_bus_get_private (DBUS_BUS_SYSTEM, &dbus_error);

	if (dbus_connection == NULL) {
		hmd_syslog_err("dbus_bus_get(): %s\n", dbus_error.message);
		return NULL;
	}
	hmd_syslog_info("%s,line=%d,dbus_connection=%p.\n",__func__,__LINE__,dbus_connection);
	i = dbus_bus_request_name (dbus_connection, HMD_DBUS_BUSNAME,
			       0, &dbus_error);

	if (dbus_error_is_set (&dbus_error)) {
		hmd_syslog_err("dbus_bus_request_name(): %s",
			    dbus_error.message);
		return NULL;
	}
	if(islocaled){
		if(HOST_BOARD->Hmd_Local_Inst[InstID]){
			HOST_BOARD->Hmd_Local_Inst[InstID]->connection = dbus_connection;
			hmd_syslog_info("%s,line=%d,HOST_BOARD->Hmd_Inst[InstID]->connection=%p.\n",__func__,__LINE__,HOST_BOARD->Hmd_Local_Inst[InstID]->connection);
		}
	}else{
		if(HOST_BOARD->Hmd_Inst[InstID]){
			HOST_BOARD->Hmd_Inst[InstID]->connection = dbus_connection;
			hmd_syslog_info("%s,line=%d,HOST_BOARD->Hmd_Inst[InstID]->connection=%p.\n",__func__,__LINE__,HOST_BOARD->Hmd_Inst[InstID]->connection);
		}
	}
	return dbus_connection;
  
}

int hmd_tipc_fd_init(int InstID, int islocaled){
	int fd = 0;
	fd = socket(AF_TIPC, SOCK_RDM, 0);
	if(fd < 0){
		return -1;
	}
	if(islocaled){
		if(HOST_BOARD->Hmd_Local_Inst[InstID])
			HOST_BOARD->Hmd_Local_Inst[InstID]->tipcfd = fd;
	}else{
		if(HOST_BOARD->Hmd_Inst[InstID])
			HOST_BOARD->Hmd_Inst[InstID]->tipcfd = fd;
	}

	return fd;
}

HMDBool hansi_wireless_checking(int InstID, int islocaled){	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	char CMDPATH[PATH_LEN];
	DBusMessage *query, *reply;
	DBusError err;
	DBusConnection * connection = NULL;
	int ret = 0;
	int *wid_check = NULL;
	int *asd_check = NULL;	
	int *wsm_check = NULL;
	int *wid_check_timeout = NULL;
	int *asd_check_timeout = NULL;
	int *wsm_check_timeout = NULL;
	if(islocaled){
		if(HOST_BOARD->Hmd_Local_Inst[InstID]){
			connection = HOST_BOARD->Hmd_Local_Inst[InstID]->connection;
			wid_check = &(HOST_BOARD->Hmd_Local_Inst[InstID]->wid_check);
			asd_check = &(HOST_BOARD->Hmd_Local_Inst[InstID]->asd_check);
			wsm_check = &(HOST_BOARD->Hmd_Local_Inst[InstID]->wsm_check);
			wid_check_timeout = &(HOST_BOARD->Hmd_Local_Inst[InstID]->wid_check_timeout);
			asd_check_timeout = &(HOST_BOARD->Hmd_Local_Inst[InstID]->asd_check_timeout);
			wsm_check_timeout = &(HOST_BOARD->Hmd_Local_Inst[InstID]->wsm_check_timeout);
		}
	}else{
		if(HOST_BOARD->Hmd_Inst[InstID]){
			connection = HOST_BOARD->Hmd_Inst[InstID]->connection;
			wid_check = &(HOST_BOARD->Hmd_Inst[InstID]->wid_check);
			asd_check = &(HOST_BOARD->Hmd_Inst[InstID]->asd_check);
			wsm_check = &(HOST_BOARD->Hmd_Inst[InstID]->wsm_check);
			wid_check_timeout = &(HOST_BOARD->Hmd_Inst[InstID]->wid_check_timeout);
			asd_check_timeout = &(HOST_BOARD->Hmd_Inst[InstID]->asd_check_timeout);
			wsm_check_timeout = &(HOST_BOARD->Hmd_Inst[InstID]->wsm_check_timeout);
		}
	}
	if(connection == NULL){
		hmd_syslog_info("%s InstID %d islocaled %d connection == NULL",__func__,InstID,islocaled);
		hmd_dbus_connection_init(InstID,islocaled);
	}
	if((NULL == wid_check)||(NULL == asd_check)||(NULL == wsm_check)){
		hmd_syslog_err("%s,%d,connection is NULL.\n",__func__,__LINE__);
		return HMD_FALSE;
	}
	HMDReInitDbusPath(InstID,WID_DBUS_BUSNAME,BUSNAME,islocaled);
	HMDReInitDbusPath(InstID,WID_DBUS_OBJPATH,OBJPATH,islocaled);
	HMDReInitDbusPath(InstID,WID_DBUS_INTERFACE,INTERFACE,islocaled);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_CHECKING);	
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (connection,query,60000, &err);	
	dbus_message_unref(query);
	if (NULL == reply)
	{
		if (dbus_error_is_set(&err))
		{
			if(!memcmp(err.name,DBUS_ERROR_NO_REPLY,strlen(DBUS_ERROR_NO_REPLY))&&!memcmp(err.message,"Did not receive a reply.",strlen("Did not receive a reply.")))
				*wid_check_timeout += 1;
			else
				*wid_check += 1;
			dbus_error_free(&err);
		}
		else
			*wid_check +=1;
		if(*wid_check >= 5 || *wid_check_timeout >= 10){
			*wid_check = 0;
			*asd_check = 0;
			*wsm_check = 0;
			*wid_check_timeout = 0;
			*asd_check_timeout = 0;
			*wsm_check_timeout = 0;
			hmd_syslog_crit("%s  InstID %d islocaled %d wid maybe wrong",__func__,InstID,islocaled);
			return HMD_FALSE;
		}
		hmd_syslog_info("%s  InstID %d islocaled %d  wid_check %d,asd_check %d,wsm_check %d",__func__,InstID,islocaled,*wid_check,*asd_check,*wsm_check);
		hmd_syslog_info("%s  InstID %d islocaled %d  wid_check_timeout %d,asd_check_timeout %d,wsm_check_timeout %d",__func__,InstID,islocaled,*wid_check_timeout,*asd_check_timeout,*wsm_check_timeout);
	}else if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&ret,
					DBUS_TYPE_INVALID)) {
		dbus_message_unref(reply);
		memset(BUSNAME, 0, PATH_LEN);
		memset(OBJPATH, 0, PATH_LEN);
		memset(INTERFACE, 0, PATH_LEN);
		HMDReInitDbusPath(InstID,ASD_DBUS_BUSNAME,BUSNAME,islocaled);
		HMDReInitDbusPath(InstID,ASD_DBUS_STA_OBJPATH,OBJPATH,islocaled);
		HMDReInitDbusPath(InstID,ASD_DBUS_STA_INTERFACE,INTERFACE,islocaled);
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_CONF_METHOD_CHECKING);
		dbus_error_init(&err);
		reply = dbus_connection_send_with_reply_and_block (connection,query,60000, &err);	
		dbus_message_unref(query);
		if (NULL == reply)
		{
			if (dbus_error_is_set(&err))
			{
				if(!memcmp(err.name,DBUS_ERROR_NO_REPLY,strlen(DBUS_ERROR_NO_REPLY))&&!memcmp(err.message,"Did not receive a reply.",strlen("Did not receive a reply.")))
					*asd_check_timeout += 1;
				else
					*asd_check += 1;
				dbus_error_free(&err);
			}
			else
				*asd_check +=1;
			if(*asd_check >= 5 || *asd_check_timeout >= 10){
				*wid_check = 0;
				*asd_check = 0;
				*wsm_check = 0;
				*wid_check_timeout = 0;
				*asd_check_timeout = 0;
				*wsm_check_timeout = 0;
				hmd_syslog_crit("%s  InstID %d islocaled %d asd maybe wrong",__func__,InstID,islocaled);
				return HMD_FALSE;
			}
			hmd_syslog_info("%s  InstID %d islocaled %d wid_check %d,asd_check %d,wsm_check %d",__func__,InstID,islocaled,*wid_check,*asd_check,*wsm_check);
			hmd_syslog_info("%s  InstID %d islocaled %d  wid_check_timeout %d,asd_check_timeout %d,wsm_check_timeout %d",__func__,InstID,islocaled,*wid_check_timeout,*asd_check_timeout,*wsm_check_timeout);
		}else if (dbus_message_get_args ( reply, &err,
						DBUS_TYPE_UINT32,&ret,
						DBUS_TYPE_INVALID)) {			
			dbus_message_unref(reply);
			memset(BUSNAME, 0, PATH_LEN);
			memset(OBJPATH, 0, PATH_LEN);
			memset(INTERFACE, 0, PATH_LEN);
			memset(CMDPATH,0,PATH_LEN);
			HMDReInitDbusPath(InstID,WSM_DBUS_BUSNAME,BUSNAME,islocaled);
			HMDReInitDbusPath(InstID,WSM_DBUS_OBJPATH,OBJPATH,islocaled);
			HMDReInitDbusPath(InstID,WSM_DBUS_INTERFACE,INTERFACE,islocaled);
			HMDReInitDbusPath(InstID,WSM_DBUS_CONF_METHOD_CHECKING,CMDPATH,islocaled);
			query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,CMDPATH);
			dbus_error_init(&err);
			reply = dbus_connection_send_with_reply_and_block (connection,query,60000, &err);	
			dbus_message_unref(query);
			if (NULL == reply)
			{
				if (dbus_error_is_set(&err))
				{
					if(!memcmp(err.name,DBUS_ERROR_NO_REPLY,strlen(DBUS_ERROR_NO_REPLY))&&!memcmp(err.message,"Did not receive a reply.",strlen("Did not receive a reply.")))
						*wsm_check_timeout += 1;
					else
						*wsm_check += 1;
					dbus_error_free(&err);
				}
				else
					*wsm_check += 1;
				if(*wsm_check >= 5 || *wsm_check_timeout >= 10){
					*wid_check = 0;
					*asd_check = 0;
					*wsm_check = 0;
					*wid_check_timeout = 0;
					*asd_check_timeout = 0;
					*wsm_check_timeout = 0;
					hmd_syslog_crit("%s  InstID %d islocaled %d wsm maybe wrong",__func__,InstID,islocaled);
					return HMD_FALSE;
				}
				hmd_syslog_info("%s  InstID %d islocaled %d wid_check %d,asd_check %d,wsm_check %d",__func__,InstID,islocaled,*wid_check,*asd_check,*wsm_check);
				hmd_syslog_info("%s  InstID %d islocaled %d  wid_check_timeout %d,asd_check_timeout %d,wsm_check_timeout %d",__func__,InstID,islocaled,*wid_check_timeout,*asd_check_timeout,*wsm_check_timeout);
			}else if (dbus_message_get_args ( reply, &err,
							DBUS_TYPE_UINT32,&ret,
							DBUS_TYPE_INVALID)) {
				dbus_message_unref(reply);				
				*wid_check = 0;
				*asd_check = 0;
				*wsm_check = 0;
				*wid_check_timeout = 0;
				*asd_check_timeout = 0;
				*wsm_check_timeout = 0;
				return HMD_TRUE;
			}else{
				if (dbus_error_is_set(&err))
				{
					if(!memcmp(err.name,DBUS_ERROR_NO_REPLY,strlen(DBUS_ERROR_NO_REPLY))&&!memcmp(err.message,"Did not receive a reply.",strlen("Did not receive a reply.")))
						*wsm_check_timeout += 1;
					else
						*wsm_check += 1;
					dbus_error_free(&err);
				}
				else
					*wsm_check +=1;
				if(*wsm_check >= 5 || *wsm_check_timeout >= 10){
					*wid_check = 0;
					*asd_check = 0;
					*wsm_check = 0;
					*wid_check_timeout = 0;
					*asd_check_timeout = 0;
					*wsm_check_timeout = 0;
					hmd_syslog_crit("%s 2 InstID %d islocaled %d wsm maybe wrong",__func__,InstID,islocaled);
					return HMD_FALSE;
				}
				hmd_syslog_info("%s 2 InstID %d islocaled %d wid_check %d,asd_check %d,wsm_check %d",__func__,InstID,islocaled,*wid_check,*asd_check,*wsm_check);
				hmd_syslog_info("%s  InstID %d islocaled %d  wid_check_timeout %d,asd_check_timeout %d,wsm_check_timeout %d",__func__,InstID,islocaled,*wid_check_timeout,*asd_check_timeout,*wsm_check_timeout);
			}
			return HMD_TRUE;
		}else{
			if (dbus_error_is_set(&err))
			{
				if(!memcmp(err.name,DBUS_ERROR_NO_REPLY,strlen(DBUS_ERROR_NO_REPLY))&&!memcmp(err.message,"Did not receive a reply.",strlen("Did not receive a reply.")))
					*asd_check_timeout += 1;
				else
					*asd_check += 1;
				dbus_error_free(&err);
			}
			else
				*asd_check +=1;
			if(*asd_check >= 5 || *asd_check_timeout >= 10){
				*wid_check = 0;
				*asd_check = 0;
				*wsm_check = 0;
				*wid_check_timeout = 0;
				*asd_check_timeout = 0;
				*wsm_check_timeout = 0;
				hmd_syslog_crit("%s 2 InstID %d islocaled %d wsm maybe wrong",__func__,InstID,islocaled);
				return HMD_FALSE;
			}
			hmd_syslog_info("%s 2 InstID %d islocaled %d wid_check %d,asd_check %d,wsm_check %d",__func__,InstID,islocaled,*wid_check,*asd_check,*wsm_check);
			hmd_syslog_info("%s 2  InstID %d islocaled %d  wid_check_timeout %d,asd_check_timeout %d,wsm_check_timeout %d",__func__,InstID,islocaled,*wid_check_timeout,*asd_check_timeout,*wsm_check_timeout);
		}
		return HMD_TRUE;
	}else{
		if (dbus_error_is_set(&err))
		{
			if(!memcmp(err.name,DBUS_ERROR_NO_REPLY,strlen(DBUS_ERROR_NO_REPLY))&&!memcmp(err.message,"Did not receive a reply.",strlen("Did not receive a reply.")))
				*wid_check_timeout += 1;
			else
				*wid_check += 1;
			dbus_error_free(&err);
		}
		else
			*wid_check +=1;
		if(*wid_check >= 5 || *wid_check_timeout >= 10){
			*wid_check = 0;
			*asd_check = 0;
			*wsm_check = 0;
			*wid_check_timeout = 0;
			*asd_check_timeout = 0;
			*wsm_check_timeout = 0;
			hmd_syslog_crit("%s 2 InstID %d islocaled %d wsm maybe wrong",__func__,InstID,islocaled);
			return HMD_FALSE;
		}
		hmd_syslog_info("%s 2 InstID %d islocaled %d wid_check %d,asd_check %d,wsm_check %d",__func__,InstID,islocaled,*wid_check,*asd_check,*wsm_check);
		hmd_syslog_info("%s  InstID %d islocaled %d  wid_check_timeout %d,asd_check_timeout %d,wsm_check_timeout %d",__func__,InstID,islocaled,*wid_check_timeout,*asd_check_timeout,*wsm_check_timeout);
	}
	return HMD_TRUE;
}


HMDBool hansi_wireless_notice_quit(int InstID, int islocaled){	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	char CMDPATH[PATH_LEN];
	DBusMessage *query, *reply;
	DBusError err;
	DBusConnection * connection=NULL;
	int ret = 0;
	if(islocaled){
		if(HOST_BOARD->Hmd_Local_Inst[InstID])
			connection = HOST_BOARD->Hmd_Local_Inst[InstID]->connection;
	}else{
		if(HOST_BOARD->Hmd_Inst[InstID])
			connection = HOST_BOARD->Hmd_Inst[InstID]->connection;
	}
	if(NULL == connection){
		hmd_syslog_info("%s InstID %d islocaled %d connection reinit.\n",__func__,InstID,islocaled);
		hmd_dbus_connection_init(InstID,islocaled);
	}
	HMDReInitDbusPath(InstID,WID_DBUS_BUSNAME,BUSNAME,islocaled);
	HMDReInitDbusPath(InstID,WID_DBUS_OBJPATH,OBJPATH,islocaled);
	HMDReInitDbusPath(InstID,WID_DBUS_INTERFACE,INTERFACE,islocaled);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_QUIT);	
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (connection,query,-1, &err);	
	dbus_message_unref(query);
	if (NULL == reply)
	{
		if (dbus_error_is_set(&err))
		{
			dbus_error_free(&err);
		}
		//return HMD_FALSE;
	}
	else if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&ret,
					DBUS_TYPE_INVALID)) {
		dbus_message_unref(reply);
	}
	memset(BUSNAME, 0, PATH_LEN);
	memset(OBJPATH, 0, PATH_LEN);
	memset(INTERFACE, 0, PATH_LEN);
	HMDReInitDbusPath(InstID,ASD_DBUS_BUSNAME,BUSNAME,islocaled);
	HMDReInitDbusPath(InstID,ASD_DBUS_STA_OBJPATH,OBJPATH,islocaled);
	HMDReInitDbusPath(InstID,ASD_DBUS_STA_INTERFACE,INTERFACE,islocaled);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_CONF_METHOD_QUIT);
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (connection,query,-1, &err);	
	dbus_message_unref(query);
	if (NULL == reply)
	{
		if (dbus_error_is_set(&err))
		{
			dbus_error_free(&err);
		}
		//return HMD_FALSE;
	}
	else if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&ret,
					DBUS_TYPE_INVALID)) {
		dbus_message_unref(reply);
	}
	memset(BUSNAME, 0, PATH_LEN);
	memset(OBJPATH, 0, PATH_LEN);
	memset(INTERFACE, 0, PATH_LEN);
	memset(CMDPATH,0,PATH_LEN);
	HMDReInitDbusPath(InstID,WSM_DBUS_BUSNAME,BUSNAME,islocaled);
	HMDReInitDbusPath(InstID,WSM_DBUS_OBJPATH,OBJPATH,islocaled);
	HMDReInitDbusPath(InstID,WSM_DBUS_INTERFACE,INTERFACE,islocaled);
	HMDReInitDbusPath(InstID,WSM_DBUS_CONF_METHOD_QUIT,CMDPATH,islocaled);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,CMDPATH);
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (connection,query,-1, &err);	
	dbus_message_unref(query);
	if (NULL == reply)
	{
		if (dbus_error_is_set(&err))
		{
			dbus_error_free(&err);
		}
		//return HMD_FALSE;
	}
	else if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&ret,
					DBUS_TYPE_INVALID)) {
		dbus_message_unref(reply);
		return HMD_TRUE;
	}
	
	return HMD_FALSE;
}

HMDBool hansi_wireless_license_update(int InstID, int islocaled, int type){	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	DBusMessage *query, *reply;
	DBusError err;
	DBusConnection * connection=NULL;
	int ret = 0;
	int num = 0;
	if(islocaled){
		num = HOST_BOARD->L_LicenseNum[InstID][type];
		if(HOST_BOARD->Hmd_Local_Inst[InstID])
			connection = HOST_BOARD->Hmd_Local_Inst[InstID]->connection;
	}else{
		num = HOST_BOARD->R_LicenseNum[InstID][type];
		if(HOST_BOARD->Hmd_Inst[InstID])
			connection = HOST_BOARD->Hmd_Inst[InstID]->connection;
	}
	if(NULL == connection){
		hmd_syslog_info("%s InstID %d islocaled %d connection reinit.\n",__func__,InstID,islocaled);
		hmd_dbus_connection_init(InstID,islocaled);
	}
	HMDReInitDbusPath(InstID,WID_DBUS_BUSNAME,BUSNAME,islocaled);
	HMDReInitDbusPath(InstID,WID_DBUS_OBJPATH,OBJPATH,islocaled);
	HMDReInitDbusPath(InstID,WID_DBUS_INTERFACE,INTERFACE,islocaled);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_UPDATE_WTP_COUNT);	
	dbus_error_init(&err);	
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&type,
							 DBUS_TYPE_UINT32,&num,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (connection,query,-1, &err);	
	dbus_message_unref(query);
	if (NULL == reply)
	{
		if (dbus_error_is_set(&err))
		{
			dbus_error_free(&err);
		}
		//return HMD_FALSE;
	}
	else if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&ret,
					DBUS_TYPE_INVALID)) {
		dbus_message_unref(reply);
	}
	
	return HMD_TRUE;
}


unsigned int notice_wid_local_hansi_service_change_state(unsigned int InstID, unsigned int neighbor_slotid)
{
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	DBusMessage *query, *reply;
	DBusError err;	
	DBusMessageIter iter;
	DBusMessageIter iter_array, iter_array1, iter_array2;	
	char *uplink_ifname = (char *)malloc(MAX_IFNAME_LEN+1);
	char *downlink_ifname = (char *)malloc(MAX_IFNAME_LEN+1);
	char *gateway_ifname = (char *)malloc(MAX_IFNAME_LEN+1);
	int uplink_cnt = 0;
	int downlink_cnt = 0;
	int gateway_cnt = 0;
	int i = 0;
	int state = HOST_BOARD->Hmd_Local_Inst[InstID]->isActive;
	DBusConnection * connection=NULL;
	int ret = 0;
	connection = HOST_BOARD->Hmd_Local_Inst[InstID]->connection;
	hmd_syslog_info("%s connection %p\n",__func__,connection);
	if(NULL == connection){
		hmd_syslog_info("%s InstID %d islocaled 1 connection == NULL,reinit.\n",__func__,InstID);
		hmd_dbus_connection_init(InstID,1);
	}
	HMDReInitDbusPath(InstID,WID_DBUS_BUSNAME,BUSNAME,1);
	HMDReInitDbusPath(InstID,WID_BAK_OBJPATH,OBJPATH,1);
	HMDReInitDbusPath(InstID,WID_BAK_INTERFACE,INTERFACE,1);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_LOCAL_HASNI_STATE_CHANGE);	
	dbus_error_init(&err);
	hmd_syslog_info("%s  1\n",__func__);
	
	dbus_message_iter_init_append(query, &iter);

	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &(InstID));
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &state);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &neighbor_slotid);
	hmd_syslog_info("%s  2\n",__func__);

	uplink_cnt = HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_UNum;
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &uplink_cnt);
	hmd_syslog_info("%s  3\n",__func__);

	dbus_message_iter_open_container (&iter,
									DBUS_TYPE_ARRAY,
									DBUS_STRUCT_BEGIN_CHAR_AS_STRING
									DBUS_TYPE_STRING_AS_STRING
									DBUS_STRUCT_END_CHAR_AS_STRING,
									&iter_array);

	hmd_syslog_info("%s  4\n",__func__);
	for (i = 0; i < uplink_cnt; i++)
	{
		hmd_syslog_info("%s  5\n",__func__);
		memset(uplink_ifname, 0,MAX_IFNAME_LEN);
		memcpy(uplink_ifname, HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Uplink[i].ifname,MAX_IFNAME_LEN);
				
		DBusMessageIter iter_struct;
		dbus_message_iter_open_container(&iter_array,
										DBUS_TYPE_STRUCT,
										NULL,
										&iter_struct);
		dbus_message_iter_append_basic(&iter_struct,
					  DBUS_TYPE_STRING, &uplink_ifname);

		dbus_message_iter_close_container (&iter_array, &iter_struct);
	}
	hmd_syslog_info("%s 6 \n",__func__);
	dbus_message_iter_close_container(&iter, &iter_array);

	/* downlink */
	hmd_syslog_info("%s  7\n",__func__);
	downlink_cnt = HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_DNum;
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &downlink_cnt);

	dbus_message_iter_open_container (&iter,
									DBUS_TYPE_ARRAY,
									DBUS_STRUCT_BEGIN_CHAR_AS_STRING
									DBUS_TYPE_STRING_AS_STRING
									DBUS_STRUCT_END_CHAR_AS_STRING,
									&iter_array1);

	hmd_syslog_info("%s  8\n",__func__);
	for (i = 0; i < downlink_cnt; i++)
	{
		hmd_syslog_info("%s 9 \n",__func__);
		memset(downlink_ifname, 0,MAX_IFNAME_LEN);
		memcpy(downlink_ifname, HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Downlink[i].ifname,MAX_IFNAME_LEN);
		
		DBusMessageIter iter_struct1;
		dbus_message_iter_open_container(&iter_array1,
										DBUS_TYPE_STRUCT,
										NULL,
										&iter_struct1);
		dbus_message_iter_append_basic(&iter_struct1,
					  DBUS_TYPE_STRING, &downlink_ifname);
		dbus_message_iter_close_container (&iter_array1, &iter_struct1);
	}
	dbus_message_iter_close_container(&iter, &iter_array1);
	hmd_syslog_info("%s  10\n",__func__);

	/* gateway */
	gateway_cnt = HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_GNum;
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &gateway_cnt);

	dbus_message_iter_open_container (&iter,
									DBUS_TYPE_ARRAY,
									DBUS_STRUCT_BEGIN_CHAR_AS_STRING
									DBUS_TYPE_STRING_AS_STRING
									DBUS_STRUCT_END_CHAR_AS_STRING,
									&iter_array2);

	for (i = 0; i < gateway_cnt; i++)
	{
		hmd_syslog_info("%s 11 \n",__func__);
		memset(gateway_ifname, 0,MAX_IFNAME_LEN);
		memcpy(gateway_ifname, HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Gateway[i].ifname,MAX_IFNAME_LEN);
		
		DBusMessageIter iter_struct2;
		dbus_message_iter_open_container(&iter_array2,
										DBUS_TYPE_STRUCT,
										NULL,
										&iter_struct2);
		dbus_message_iter_append_basic(&iter_struct2,
					  DBUS_TYPE_STRING, &gateway_ifname);
		dbus_message_iter_close_container (&iter_array2, &iter_struct2);
	}
	hmd_syslog_info("%s 12 \n",__func__);
	dbus_message_iter_close_container(&iter, &iter_array2);

	reply = dbus_connection_send_with_reply_and_block (connection,query,-1, &err);	
	hmd_syslog_info("%s 13 \n",__func__);
	dbus_message_unref(query);
	if (NULL == reply)
	{
		hmd_syslog_info("%s 14 \n",__func__);
		if (dbus_error_is_set(&err))
		{
			dbus_error_free(&err);
		}
		free(uplink_ifname);
		uplink_ifname = NULL;
		free(downlink_ifname);
		downlink_ifname = NULL;
		free(gateway_ifname);
		gateway_ifname = NULL;
		
		return HMD_FALSE;
	}
	else if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&ret,
					DBUS_TYPE_INVALID)) {
		hmd_syslog_info("%s 15 \n",__func__);
		dbus_message_unref(reply);
	}
	free(uplink_ifname);
	uplink_ifname = NULL;
	free(downlink_ifname);
	downlink_ifname = NULL;
	free(gateway_ifname);
	gateway_ifname = NULL;
	return ret;
}

unsigned int notice_eag_local_hansi_service_change_state(unsigned int InstID, unsigned int neighbor_slotid)
{
	DBusMessage *query, *reply;
	DBusError err;	
	DBusMessageIter iter;
	DBusMessageIter iter_array, iter_array1, iter_array2;	
	char *uplink_ifname = (char *)malloc(MAX_IFNAME_LEN+1);
	char *downlink_ifname = (char *)malloc(MAX_IFNAME_LEN+1);
	char *gateway_ifname = (char *)malloc(MAX_IFNAME_LEN+1);
	int uplink_cnt = 0;
	int downlink_cnt = 0;
	int gateway_cnt = 0;
	int i = 0;
	int state = HOST_BOARD->Hmd_Local_Inst[InstID]->isActive;
	DBusConnection * connection;
	int ret = 0;
	connection = HOST_BOARD->Hmd_Local_Inst[InstID]->connection;
	hmd_syslog_info("%s connection %p\n",__func__,connection);
	
#define EAG_DBUS_NAME_FMT		"aw.eag_%s_%d"
#define EAG_DBUS_OBJPATH_FMT	"/aw/eag_%s_%d"
#define EAG_DBUS_INTERFACE_FMT	"aw.eag_%s_%d"
#define EAG_DBUS_SET_STATE		"eag_hansi_dbus_hmd_state_change_func"
	
	char EAG_DBUS_NAME[64];
	char EAG_DBUS_OBJPATH[64];
	char EAG_DBUS_INTERFACE[64];
	snprintf(EAG_DBUS_NAME,sizeof(EAG_DBUS_NAME)-1,	EAG_DBUS_NAME_FMT,"l",InstID );
	snprintf(EAG_DBUS_OBJPATH,sizeof(EAG_DBUS_NAME)-1, EAG_DBUS_OBJPATH_FMT,"l",InstID );
	snprintf(EAG_DBUS_INTERFACE,sizeof(EAG_DBUS_NAME)-1, EAG_DBUS_INTERFACE_FMT,"l",InstID );
	
	query = dbus_message_new_method_call(EAG_DBUS_NAME,
				                         EAG_DBUS_OBJPATH,
										 EAG_DBUS_INTERFACE,
										 EAG_DBUS_SET_STATE);
	dbus_error_init(&err);
	hmd_syslog_info("%s  1\n",__func__);
	
	dbus_message_iter_init_append(query, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &(InstID));
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &state);
	hmd_syslog_info("%s  2\n",__func__);
	
	uplink_cnt = HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_UNum;
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &uplink_cnt);
	hmd_syslog_info("%s  uplink_cnt:%d\n",__func__,uplink_cnt);

	dbus_message_iter_open_container (&iter,
									DBUS_TYPE_ARRAY,
									DBUS_STRUCT_BEGIN_CHAR_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING
									DBUS_TYPE_STRING_AS_STRING
									DBUS_STRUCT_END_CHAR_AS_STRING,
									&iter_array);
	/* uplink */
	hmd_syslog_info("%s  4\n",__func__);
	for (i = 0; i < uplink_cnt; i++)
	{
		hmd_syslog_info("%s  5\n",__func__);
		memset(uplink_ifname, 0,MAX_IFNAME_LEN);
		memcpy(uplink_ifname, HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Uplink[i].ifname,MAX_IFNAME_LEN);
				
		DBusMessageIter iter_struct;
		dbus_message_iter_open_container(&iter_array,
										DBUS_TYPE_STRUCT,
										NULL,
										&iter_struct);
		dbus_message_iter_append_basic(&iter_struct,
					  DBUS_TYPE_UINT32, &HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Uplink[i].real_ip);
		dbus_message_iter_append_basic(&iter_struct,
					  DBUS_TYPE_UINT32, &HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Uplink[i].remote_r_ip);
		dbus_message_iter_append_basic(&iter_struct,
					  DBUS_TYPE_UINT32, &HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Uplink[i].vir_ip);
		dbus_message_iter_append_basic(&iter_struct,
					  DBUS_TYPE_STRING, &uplink_ifname);
		dbus_message_iter_close_container (&iter_array, &iter_struct);
	}
	hmd_syslog_info("%s 6 \n",__func__);
	dbus_message_iter_close_container(&iter, &iter_array);

	/* downlink */
	downlink_cnt = HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_DNum;
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &downlink_cnt);
	hmd_syslog_info("%s  downlink_cnt:%d\n",__func__,downlink_cnt);
	dbus_message_iter_open_container (&iter,
									DBUS_TYPE_ARRAY,
									DBUS_STRUCT_BEGIN_CHAR_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING
									DBUS_TYPE_STRING_AS_STRING
									DBUS_STRUCT_END_CHAR_AS_STRING,
									&iter_array1);
	
	hmd_syslog_info("%s  7\n",__func__);
	for (i = 0; i < downlink_cnt; i++)
	{
		hmd_syslog_info("%s  8\n",__func__);
		memset(downlink_ifname, 0,MAX_IFNAME_LEN);
		memcpy(downlink_ifname, HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Downlink[i].ifname,MAX_IFNAME_LEN);
				
		DBusMessageIter iter_struct;
		dbus_message_iter_open_container(&iter_array1,
										DBUS_TYPE_STRUCT,
										NULL,
										&iter_struct);
		dbus_message_iter_append_basic(&iter_struct,
					  DBUS_TYPE_UINT32, &HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Downlink[i].real_ip);
		dbus_message_iter_append_basic(&iter_struct,
					  DBUS_TYPE_UINT32, &HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Downlink[i].remote_r_ip);
		dbus_message_iter_append_basic(&iter_struct,
					  DBUS_TYPE_UINT32, &HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Downlink[i].vir_ip);
		dbus_message_iter_append_basic(&iter_struct,
					  DBUS_TYPE_STRING, &downlink_ifname);
		dbus_message_iter_close_container (&iter_array1, &iter_struct);
	}
	hmd_syslog_info("%s 9 \n",__func__);
	dbus_message_iter_close_container(&iter, &iter_array1);

	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &HOST_BOARD->Hmd_Local_Inst[InstID]->slot_no1);
	hmd_syslog_info("%s  slot_no1:%d\n",__func__,HOST_BOARD->Hmd_Local_Inst[InstID]->slot_no1);
	
	/* vgateway interface */
	gateway_cnt = HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_GNum;
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &gateway_cnt);
	hmd_syslog_info("%s  gateway_cnt:%d\n",__func__,gateway_cnt);
	dbus_message_iter_open_container (&iter,
									DBUS_TYPE_ARRAY,
									DBUS_STRUCT_BEGIN_CHAR_AS_STRING
									DBUS_TYPE_STRING_AS_STRING
									DBUS_TYPE_UINT32_AS_STRING
									DBUS_STRUCT_END_CHAR_AS_STRING,
									&iter_array2);
	hmd_syslog_info("%s  10\n",__func__);
	for (i = 0; i < gateway_cnt; i++)
	{
		hmd_syslog_info("%s  11\n",__func__);
		memset(gateway_ifname, 0,MAX_IFNAME_LEN);
		memcpy(gateway_ifname, HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Gateway[i].ifname,MAX_IFNAME_LEN);
				
		DBusMessageIter iter_struct;
		dbus_message_iter_open_container(&iter_array2,
										DBUS_TYPE_STRUCT,
										NULL,
										&iter_struct);
		dbus_message_iter_append_basic(&iter_struct,
					  DBUS_TYPE_STRING, &gateway_ifname);
		dbus_message_iter_append_basic(&iter_struct,
					  DBUS_TYPE_UINT32, &HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Gateway[i].vir_ip);
		dbus_message_iter_close_container (&iter_array2, &iter_struct);
	}
	hmd_syslog_info("%s 12 \n",__func__);
	dbus_message_iter_close_container(&iter, &iter_array2);

	//reply = dbus_connection_send_with_reply_and_block (vrrp_notify_dbus_connection,query,5000, &err);
	reply = dbus_connection_send_with_reply_and_block (connection,query,60000, &err);	
	hmd_syslog_info("%s 13 \n",__func__);
	dbus_message_unref(query);
	if (NULL == reply)
	{
		hmd_syslog_info("%s 14 \n",__func__);
		if (dbus_error_is_set(&err))
		{
			dbus_error_free(&err);
		}
		free(uplink_ifname);
		uplink_ifname = NULL;
		free(downlink_ifname);
		downlink_ifname = NULL;
		free(gateway_ifname);
		gateway_ifname = NULL;
		
		return HMD_FALSE;
	}
	else if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&ret,
					DBUS_TYPE_INVALID)) {
		hmd_syslog_info("%s 15 \n",__func__);
		dbus_message_unref(reply);
	}
	free(uplink_ifname);
	uplink_ifname = NULL;
	free(downlink_ifname);
	downlink_ifname = NULL;
	free(gateway_ifname);
	gateway_ifname = NULL;
	return ret;
}

unsigned int notice_vrrp_config_service_change_state(unsigned int InstID, unsigned int enable)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err = {0};
	char OBJPATH[PATH_LEN] = {0};
	char BUSNAME[PATH_LEN] = {0};
	int ret = 0;	
	char cmd[128] = {0};
	DBusConnection * connection = HOST_BOARD->Hmd_Inst[InstID]->connection;
	HMDReInitHadDbusPath(InstID,VRRP_DBUS_BUSNAME,BUSNAME,0);//book modify
	HMDReInitHadDbusPath(InstID,VRRP_DBUS_OBJPATH,OBJPATH,0);//book modify
	query = dbus_message_new_method_call(BUSNAME,
										OBJPATH,
										VRRP_DBUS_INTERFACE,
										VRRP_DBUS_METHOD_VRRP_SERVICE_ENABLE);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &InstID,
							DBUS_TYPE_UINT32, &enable,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block(connection, query, 150000, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			hmd_syslog_err("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		hmd_syslog_err("delete hansi %d faild1.\n",InstID);
		ret = -1;
		//return HMD_FALSE;
	}
	/* book modify */
	else if (!(dbus_message_get_args (reply, &err, DBUS_TYPE_UINT32,&ret, DBUS_TYPE_INVALID))){		
	    //hmd_syslog_info("333333\n");
		if (dbus_error_is_set(&err)) {
		    //hmd_syslog_info("444444\n");
			dbus_error_free(&err);
		}
	}
	if(NULL != reply)
		dbus_message_unref(reply);
	if (VRRP_RETURN_CODE_OK != ret) {//book modify
		hmd_syslog_err("config hansi %d service disable faild,ret=%d.\n", InstID,ret);
	}
	/* [2] kill the special had instance process which instance no is profile. */
	/* book add for stop had */
	#if 0
	sprintf(cmd,
			"PIDS=`ps -ef | grep \"had %d$\" | grep -v grep | awk '{print $2}'`; father=`echo $PIDS | awk '{print $1}'`; sudo kill $father",
			profile);
	#else
	sprintf(cmd, "sudo /etc/init.d/had stop %d ", InstID);
	#endif
	if (system(cmd)) {
		hmd_syslog_err("delete hansi %d faild2.\n", InstID);
		return HMD_FALSE;
	}
	/* [3] clear tmp file of had instance */
	memset(cmd, 0, 128);
	sprintf(cmd,
			"sudo rm /var/run/had%d.pidmap",
			InstID);
	if (system(cmd)) {
		hmd_syslog_err("delete hansi %d faild3.\n",InstID);
		return HMD_FALSE;
	}
	memset(cmd, 0, 128);
	sprintf(cmd,
			"sudo rm /var/run/had%d.pid",
			InstID);
	if (system(cmd)) {
		hmd_syslog_err("delete hansi %d faild4,cmd:%s.\n",InstID,cmd);
		//return HMD_FALSE;
	}	
	return HMD_TRUE;
}

int notice_hmd_server_state_change(int InstID, int islocaled, HmdOP op, InstState prestate){
	struct HmdMsg tmsg;
	int state = 0;
	int fd =-1;
	tmsg.S_SlotID = HOST_SLOT_NO;
	tmsg.InstID = InstID;
	if(islocaled){
		if(HOST_BOARD->Hmd_Local_Inst[InstID]){
			state = HOST_BOARD->Hmd_Local_Inst[InstID]->InstState;
			fd = HOST_BOARD->Hmd_Local_Inst[InstID]->tipcfd;
		}
	}else{
		if(HOST_BOARD->Hmd_Inst[InstID]){
			state = HOST_BOARD->Hmd_Inst[InstID]->InstState;
			fd = HOST_BOARD->Hmd_Inst[InstID]->tipcfd;
		}
	}
	if(fd <= 0){
		hmd_syslog_err("%s,%d,invaid fd:%d.\n",__func__,__LINE__,fd);
		return 0;
	}
	tmsg.op = op;
	if(islocaled)
		tmsg.type = HMD_LOCAL_HANSI;
	else
		tmsg.type = HMD_HANSI;
	if(op == HMD_STATE_SWITCH){
		tmsg.u.statechange.prestate = prestate;
		tmsg.u.statechange.nowstate = state;
	}
	if (0 > sendto(fd, &tmsg, sizeof(tmsg), 0,(struct sockaddr*)&(HOST_BOARD->tipcaddr), sizeof(HOST_BOARD->tipcaddr))) 
	{		
		perror("Client: failed to send");		
	}
	if(islocaled){
		if(HOST_BOARD->Hmd_Local_Inst[InstID])
			if(HOST_BOARD->Hmd_Local_Inst[InstID]->slot_no1 != -1){
				if (0 > sendto(fd, &tmsg, sizeof(tmsg), 0,(struct sockaddr*)&(HOST_BOARD->Hmd_Local_Inst[InstID]->tipcaddr), sizeof(HOST_BOARD->tipcaddr))) 
				{		
					perror("Client: failed to send");		
				}			
			}
	}
	return 0;
}

int hansi_state_check(int InstID, int islocaled){
	char buf[128] = {0};
	char defaultPath[] = "/var/run/config/Instconfig";
	if(islocaled){
		if(HOST_BOARD->Hmd_Local_Inst[InstID] != NULL){
			sprintf(buf,"sudo /etc/init.d/wcpss stop %d %d",islocaled, InstID);
			system(buf);
			memset(buf, 0, 128);
			sprintf(buf,"sudo /etc/init.d/wcpss start %d %d",islocaled, InstID);
			system(buf);
			if(HOST_SLOT_NO != MASTER_SLOT_NO)
				notice_hmd_server_state_change(InstID, islocaled, HMD_RESTART, 0);
			if(HOST_BOARD->Hmd_Local_Inst[InstID]->slot_no == HOST_SLOT_NO)
				HOST_BOARD->Hmd_Local_Inst[InstID]->RestartTimes += 1;
			else if(HOST_BOARD->Hmd_Local_Inst[InstID]->slot_no1 == HOST_SLOT_NO)
				HOST_BOARD->Hmd_Local_Inst[InstID]->RestartTimes1 += 1;				
			if(HOST_SLOT_NO == MASTER_SLOT_NO){
				#if 0
				memset(buf, 0, 128);
				sprintf(buf,"mv /mnt/cli.conf /mnt/cli.conf_bak");
				system(buf);				
				sprintf(newpath,"cp %s%d-1-%d /mnt/cli.conf",defaultPath,HOST_BOARD->slot_no,InstID);
				system(newpath);
				sleep(1);
				#endif
				memset(buf, 0, 128);
				sprintf(buf,"/opt/bin/vtysh -f %s%d-1-%d -b &",defaultPath,HOST_BOARD->slot_no,InstID);
				system(buf);
//				memset(buf, 0, 128);
//				sprintf(buf,"mv /mnt/cli.conf_bak /mnt/cli.conf");
//				system(buf);				
			}
		}
	}else{
		if(HOST_BOARD->Hmd_Inst[InstID] != NULL){
			notice_vrrp_config_service_change_state(InstID, 0);
			sprintf(buf,"sudo /etc/init.d/wcpss stop %d %d",islocaled, InstID);
			system(buf);			
			memset(buf, 0, 128);
			sprintf(buf,"sudo /etc/init.d/wcpss start %d %d",islocaled, InstID);
			system(buf);
			memset(buf, 0, 128);
			sprintf(buf,"sudo /etc/init.d/had start %d", InstID);
			system(buf);
			if(HOST_SLOT_NO != MASTER_SLOT_NO)
				notice_hmd_server_state_change(InstID, islocaled, HMD_RESTART, 0);
			
			HOST_BOARD->Hmd_Inst[InstID]->RestartTimes += 1;
			if(HOST_SLOT_NO == MASTER_SLOT_NO){
				#if 0
				memset(buf, 0, 128);
				sprintf(buf,"mv /mnt/cli.conf /mnt/cli.conf_bak");
				system(buf);				
				sprintf(newpath,"cp %s%d-0-%d /mnt/cli.conf",defaultPath,HOST_BOARD->slot_no,InstID);
				system(newpath);
				sleep(1);
				#endif
				memset(buf, 0, 128);
				sprintf(buf,"/opt/bin/vtysh -f %s%d-0-%d -b &",defaultPath,HOST_BOARD->slot_no,InstID);
				system(buf);
//				memset(buf, 0, 128);
//				sprintf(buf,"mv /mnt/cli.conf_bak /mnt/cli.conf");
//				system(buf);				
			}
		}
	}
	return 0;
}


void HANSI_CHECKING(int InstID, int islocaled){
	int ret = HMD_TRUE;
	int i =0;
	char * ifname = NULL;
	char * mac = NULL;
	int vip;
	int mask;
	if(HANSI_CHECK_OP){
		ret = hansi_wireless_checking(InstID, islocaled);
		if(ret == HMD_FALSE){
			hmd_syslog_warning("check wireless false,need restart wcpss.\n");
			hansi_state_check(InstID, islocaled);
		}
		if(islocaled){
			if((HOST_BOARD->Hmd_Local_Inst[InstID])&&(HOST_BOARD->Hmd_Local_Inst[InstID]->isActive == INST_ACTIVE)){
				for(i = 0; i <	HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_DNum; i++){
					ifname = HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Downlink[i].ifname;
					vip = HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Downlink[i].vir_ip;
					mask = HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Downlink[i].mask;
					mac = HOST_BOARD->Hmd_Local_Inst[InstID]->Inst_Downlink[i].mac;
					send_tunnel_interface_arp(mac,vip,ifname);	
				}
			}
			if(HOST_BOARD->Hmd_Local_Inst[InstID])
				HMDTimerRequest(HMD_CHECKING_TIMER,&(HOST_BOARD->Hmd_Local_Inst[InstID]->HmdTimerID), HMD_CHECKING, InstID, islocaled);
		}else{
			if(HOST_BOARD->Hmd_Inst[InstID])
				HMDTimerRequest(HMD_CHECKING_TIMER,&(HOST_BOARD->Hmd_Inst[InstID]->HmdTimerID), HMD_CHECKING, InstID, islocaled);
		}
	}
}
/*shaojunwu add for del eag hansi  20110620*/
static void HANSI_DELETE_EAG( int InstID, int islocaled ){
	char command[256];
	if( islocaled ){
		memset(command, 0, sizeof(command));
		sprintf(command,"sudo /etc/init.d/eag_modules stop 1 %d &",InstID);
		system(command);
	}else{
		memset(command, 0, sizeof(command));
		sprintf(command,"sudo /etc/init.d/eag_modules stop 0 %d &",InstID);
		system(command);		
	}
}
/*end add for del eag hansi*/

/*lixiang add for del pppoe hansi 20120817*/
#ifndef _VERSION_18SP7_
static void HANSI_DELETE_PPPOE( int InstID, int islocaled ){
	char command[256];
	if( islocaled ){
		memset(command, 0, sizeof(command));
		sprintf(command,"sudo /etc/init.d/pppoe stop 1 %d &",InstID);
		system(command);
	}else{
		memset(command, 0, sizeof(command));
		sprintf(command,"sudo /etc/init.d/pppoe stop 0 %d &",InstID);
		system(command);		
	}
}
#endif
/*end add for del pppoe hansi*/

/*xiaodw add for delete femto iu&iuh*/
static void HANSI_DELETE_FEMTO( int InstID, int islocaled ){
	char command[SYS_COMMAND_LEN];
	memset(command, 0, SYS_COMMAND_LEN);
	if(islocaled){
		sprintf(command, "sudo /etc/init.d/iuh stop 1 %d &", InstID);
		system(command);
		memset(command, 0, SYS_COMMAND_LEN);
		sprintf(command, "sudo /etc/init.d/ranapproxy stop 1 %d &", InstID);
		system(command);
	}else{
		sprintf(command, "sudo /etc/init.d/iuh stop 0 %d &", InstID);
		system(command);
		memset(command, 0, SYS_COMMAND_LEN);
		sprintf(command, "sudo /etc/init.d/ranapproxy stop 0 %d &", InstID);
		system(command);
	}
}
void HANSI_DELETE(int InstID, int islocaled){
	if(islocaled){
		if(HOST_BOARD->Hmd_Local_Inst[InstID] != NULL){
			HmdTimerCancel(&(HOST_BOARD->Hmd_Local_Inst[InstID]->HmdTimerID),1);
			hansi_wireless_notice_quit(InstID,islocaled);
			if(HOST_BOARD->Hmd_Local_Inst[InstID]->connection){
				dbus_connection_close(HOST_BOARD->Hmd_Local_Inst[InstID]->connection);
				HOST_BOARD->Hmd_Local_Inst[InstID]->connection = NULL;
			}
			if(HOST_BOARD->Hmd_Local_Inst[InstID]->tipcfd != 0){
				close(HOST_BOARD->Hmd_Local_Inst[InstID]->tipcfd);
			}
			free(HOST_BOARD->Hmd_Local_Inst[InstID]);
			HOST_BOARD->Hmd_Local_Inst[InstID] = NULL;
		}
	}else{
		if(HOST_BOARD->Hmd_Inst[InstID] != NULL){			
			HmdTimerCancel(&(HOST_BOARD->Hmd_Inst[InstID]->HmdTimerID),1);
			notice_vrrp_config_service_change_state(InstID, 0);
			hansi_wireless_notice_quit(InstID,islocaled);
			if(HOST_BOARD->Hmd_Inst[InstID]->connection){
			    dbus_connection_close(HOST_BOARD->Hmd_Inst[InstID]->connection);
				HOST_BOARD->Hmd_Inst[InstID]->connection = NULL;
			}
			if(HOST_BOARD->Hmd_Inst[InstID]->tipcfd != 0){
				close(HOST_BOARD->Hmd_Inst[InstID]->tipcfd);
			}
			free(HOST_BOARD->Hmd_Inst[InstID]);
			HOST_BOARD->Hmd_Inst[InstID] = NULL;  //book add,201105-24
		}
	}

/*shaojunwu add for del eag hansi  20110620*/
	HANSI_DELETE_EAG( InstID, islocaled );
/*end add for del eag hansi*/

	/*lixiang add for del pppoe hansi 20120817*/
#ifndef _VERSION_18SP7_ 
	HANSI_DELETE_PPPOE(InstID, islocaled);
#endif
	/*end add for del pppoe hansi*/

	/*xiaodw add for del femto iu&iuh*/
	HANSI_DELETE_FEMTO(InstID, islocaled);
}

void * HMDHansiMonitor(void *arg) {	
	int QID = ((HMDThreadArg*)arg)->QID;
	int islocaled = ((HMDThreadArg*)arg)->islocaled;
	int InstID = ((HMDThreadArg*)arg)->InstID;
	hmd_pid_write_v2("HansiMonitor",QID);
	int MsgqID;
	HMD_FREE_OBJECT(arg);
	HmdGetMsgQueue(&MsgqID);
	if(NULL == hmd_dbus_connection_init(InstID,islocaled)){
		hmd_syslog_info("%s hmd_dbus_connection_init failed",__func__);
		return NULL;
	}else if(hmd_tipc_fd_init(InstID,islocaled) < 0){
		hmd_syslog_info("%s hmd_tipc_fd_init failed",__func__);
		return NULL;
	}
	while(1)
	{
		struct HmdMsgQ HMsgq;
		memset((char*)&HMsgq, 0, sizeof(HMsgq));
		if (msgrcv(MsgqID, (struct HmdMsgQ*)&HMsgq, sizeof(HMsgq.mqinfo), QID, 0) == -1) {
			hmd_syslog_info("%s msgrcv %s",__func__,strerror(errno));
			perror("msgrcv");
			continue;
		}

		switch(HMsgq.mqinfo.op){
			case HMD_HANSI_CHECKING:
				HANSI_CHECKING(InstID, islocaled);
				break;
			case HMD_DELETE:				
				HANSI_DELETE(InstID,islocaled);
				return NULL;
			case HMD_STATE_SWITCH:
				
				break;
			case HMD_LICENSE_UPDATE:
				hansi_wireless_license_update(InstID,islocaled,HMsgq.mqinfo.u.LicenseInfo.licenseType);
				break;
			default:
				break;
		}
	}
	return NULL;
}
