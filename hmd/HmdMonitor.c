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
#include "dbus/dhcp/dhcp_dbus_def.h"
#include "HmdMonitor.h"

#define VRRP_RETURN_CODE_OK (0x150001)
extern  struct Hmd_For_Dhcp_restart *DHCP_MONITOR;


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
	if(-1 == InstID){
		//hmd_syslog_info("connection_init ~~~\n");
		DHCP_MONITOR->connection = dbus_connection;
		return dbus_connection;
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
	if(-1 == InstID){
		DHCP_MONITOR->tipcfd = fd;
		return fd;
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
int dhcp_running_checking(){
	//char BUSNAME[PATH_LEN];
	//char OBJPATH[PATH_LEN];
	//char INTERFACE[PATH_LEN];
//	char CMDPATH[PATH_LEN];
	DBusMessage *query, *reply;
	DBusError err;
	DBusConnection * connection = NULL;
	int ret = 0;
	int InstID = -1,islocaled = 0;
	int *dhcp_check = NULL;
	int *dhcp_check_timeout = NULL;

	dhcp_check = &(DHCP_MONITOR->dhcp_check);
	dhcp_check_timeout = &(DHCP_MONITOR->dhcp_check_timeout);
	connection = DHCP_MONITOR->connection;
	if(connection == NULL){
		hmd_syslog_info("%s InstID %d islocaled %d connection == NULL",__func__,InstID,islocaled);
		hmd_dbus_connection_init(InstID,islocaled);
	}
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME,DHCP_DBUS_OBJPATH,DHCP_DBUS_INTERFACE,DHCP_DBUS_METHOD_HMD_RUNNING_CHECK);	
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (connection,query,60000, &err);	
	dbus_message_unref(query);
	if (NULL == reply)
	{
		if (dbus_error_is_set(&err))
		{
			if(!memcmp(err.name,DBUS_ERROR_NO_REPLY,strlen(DBUS_ERROR_NO_REPLY))&&!memcmp(err.message,"Did not receive a reply.",strlen("Did not receive a reply.")))
				*dhcp_check_timeout += 1;
			else
				*dhcp_check += 1;
			dbus_error_free(&err);
		}
		else
		*dhcp_check+=1;
		if(*dhcp_check>= 5 || *dhcp_check_timeout >= 10){
			*dhcp_check = 0;
			*dhcp_check_timeout = 0;
			hmd_syslog_crit("%s  InstID %d islocaled %d DHCP maybe wrong",__func__,InstID,islocaled);
			return HMD_FALSE;
		}
		hmd_syslog_info("%s  InstID %d islocaled %d  DHCP_check %d",__func__,InstID,islocaled,*dhcp_check);
	}else if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&ret,
					DBUS_TYPE_INVALID)){
		dbus_message_unref(reply);
	}

	return HMD_TRUE;
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
		return HMD_RELOAD_MOD_WCPSS;
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
			return HMD_RELOAD_MOD_WCPSS;
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
				return HMD_RELOAD_MOD_WCPSS;
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
					return HMD_RELOAD_MOD_WCPSS;
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
				return HMD_RELOAD_MOD_NONE;
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
					return HMD_RELOAD_MOD_WCPSS;
				}
				hmd_syslog_info("%s 2 InstID %d islocaled %d wid_check %d,asd_check %d,wsm_check %d",__func__,InstID,islocaled,*wid_check,*asd_check,*wsm_check);
				hmd_syslog_info("%s  InstID %d islocaled %d  wid_check_timeout %d,asd_check_timeout %d,wsm_check_timeout %d",__func__,InstID,islocaled,*wid_check_timeout,*asd_check_timeout,*wsm_check_timeout);
			}
			return HMD_RELOAD_MOD_NONE;
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
				return HMD_RELOAD_MOD_WCPSS;
			}
			hmd_syslog_info("%s 2 InstID %d islocaled %d wid_check %d,asd_check %d,wsm_check %d",__func__,InstID,islocaled,*wid_check,*asd_check,*wsm_check);
			hmd_syslog_info("%s 2  InstID %d islocaled %d  wid_check_timeout %d,asd_check_timeout %d,wsm_check_timeout %d",__func__,InstID,islocaled,*wid_check_timeout,*asd_check_timeout,*wsm_check_timeout);
		}
		return HMD_RELOAD_MOD_NONE;
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
			return HMD_RELOAD_MOD_WCPSS;
		}
		hmd_syslog_info("%s 2 InstID %d islocaled %d wid_check %d,asd_check %d,wsm_check %d",__func__,InstID,islocaled,*wid_check,*asd_check,*wsm_check);
		hmd_syslog_info("%s  InstID %d islocaled %d  wid_check_timeout %d,asd_check_timeout %d,wsm_check_timeout %d",__func__,InstID,islocaled,*wid_check_timeout,*asd_check_timeout,*wsm_check_timeout);
	}
	return HMD_RELOAD_MOD_NONE;
}
enum hmd_reload_type hansi_eag_checking(int InstID, int islocaled)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusConnection * connection = NULL;
	int ret = 0;
	int *eag_check = NULL;
	int *eag_check_timeout = NULL;
	if(islocaled)
	{
		if(HOST_BOARD->Hmd_Local_Inst[InstID])
		{
			connection = HOST_BOARD->Hmd_Local_Inst[InstID]->connection;
			eag_check = &(HOST_BOARD->Hmd_Local_Inst[InstID]->eag_check);
			eag_check_timeout = &(HOST_BOARD->Hmd_Local_Inst[InstID]->eag_check_timeout);
		}
	}
	else
	{
		if(HOST_BOARD->Hmd_Inst[InstID])
		{
			connection = HOST_BOARD->Hmd_Inst[InstID]->connection;
			eag_check = &(HOST_BOARD->Hmd_Inst[InstID]->eag_check);
			eag_check_timeout = &(HOST_BOARD->Hmd_Inst[InstID]->eag_check_timeout);
		}
	}
	if(connection == NULL)
	{
		hmd_syslog_info("%s InstID %d islocaled %d connection == NULL", __func__, InstID, islocaled);
		hmd_dbus_connection_init(InstID, islocaled);
	}
	if(NULL == eag_check)
	{
		hmd_syslog_err("%s,%d,connection is NULL.\n", __func__, __LINE__);
		return HMD_RELOAD_MOD_EAG;
	}

	#define EAG_DBUS_NAME_FMT		"aw.eag_%s_%d"
	#define EAG_DBUS_OBJPATH_FMT	"/aw/eag_%s_%d"
	#define EAG_DBUS_INTERFACE_FMT	"aw.eag_%s_%d"
	#define EAG_DBUS_CHECK_STATUS	"eag_dbus_method_check_status"

	char EAG_DBUS_NAME[64];
	char EAG_DBUS_OBJPATH[64];
	char EAG_DBUS_INTERFACE[64];
	if (InstID > 0) {
		snprintf(EAG_DBUS_NAME, sizeof(EAG_DBUS_NAME) - 1,	EAG_DBUS_NAME_FMT, "r", InstID );
		snprintf(EAG_DBUS_OBJPATH, sizeof(EAG_DBUS_NAME) - 1, EAG_DBUS_OBJPATH_FMT, "r", InstID );
		snprintf(EAG_DBUS_INTERFACE, sizeof(EAG_DBUS_NAME) - 1, EAG_DBUS_INTERFACE_FMT, "r", InstID );
	}
	else
	{
		snprintf(EAG_DBUS_NAME, sizeof(EAG_DBUS_NAME) - 1,	EAG_DBUS_NAME_FMT, "l", InstID );
		snprintf(EAG_DBUS_OBJPATH, sizeof(EAG_DBUS_NAME) - 1, EAG_DBUS_OBJPATH_FMT, "l", InstID );
		snprintf(EAG_DBUS_INTERFACE, sizeof(EAG_DBUS_NAME) - 1, EAG_DBUS_INTERFACE_FMT, "l", InstID );
	}
	query = dbus_message_new_method_call(EAG_DBUS_NAME,
	                                     EAG_DBUS_OBJPATH,
	                                     EAG_DBUS_INTERFACE,
	                                     EAG_DBUS_CHECK_STATUS);
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (connection, query, 3000, &err);
	dbus_message_unref(query);
	if (NULL == reply)
	{
		if (dbus_error_is_set(&err))
		{
			if(!memcmp(err.name, DBUS_ERROR_NO_REPLY, strlen(DBUS_ERROR_NO_REPLY)) && !memcmp(err.message, "Did not receive a reply.", strlen("Did not receive a reply.")))
				*eag_check_timeout += 1;
			else
				*eag_check += 1;
			dbus_error_free(&err);
		}
		else
			*eag_check += 1;
		if(*eag_check >= 3 || *eag_check_timeout >= 5)
		{
			*eag_check = 0;
			*eag_check_timeout = 0;
			hmd_syslog_crit("%s:%d	InstID %d islocaled %d eag maybe wrong", 
					__func__, __LINE__, InstID, islocaled);
			return HMD_RELOAD_MOD_EAG;
		}
		hmd_syslog_info("%s  InstID %d islocaled %d  eag_check %d", __func__, InstID, islocaled, *eag_check);
		hmd_syslog_info("%s  InstID %d islocaled %d  eag_check_timeout %d", __func__, InstID, islocaled, *eag_check_timeout);
	}
	else if (dbus_message_get_args ( reply, &err,
											DBUS_TYPE_UINT32, &ret,
											DBUS_TYPE_INVALID))
	{
			dbus_message_unref(reply);
			*eag_check = 0;
			*eag_check_timeout = 0;
			return HMD_RELOAD_MOD_NONE;
	}
	else
	{
		if (dbus_error_is_set(&err))
		{
			if(!memcmp(err.name, DBUS_ERROR_NO_REPLY, strlen(DBUS_ERROR_NO_REPLY)) && !memcmp(err.message, "Did not receive a reply.", strlen("Did not receive a reply.")))
				*eag_check_timeout += 1;
			else
				*eag_check += 1;
			dbus_error_free(&err);
		}
		else
			*eag_check += 1;
		if(*eag_check >= 3 || *eag_check_timeout >= 5)
		{
			*eag_check = 0;
			*eag_check_timeout = 0;
			hmd_syslog_crit("%s:%d InstID %d islocaled %d eag maybe wrong",
								__func__, __LINE__, InstID, islocaled);
			return HMD_RELOAD_MOD_EAG;
		}
		hmd_syslog_info("%s 2 InstID %d islocaled %d eag_check %d", __func__, InstID, islocaled, *eag_check);
		hmd_syslog_info("%s  InstID %d islocaled %d  eag_check_timeout %d", __func__, InstID, islocaled, *eag_check_timeout);
	}

	return HMD_RELOAD_MOD_NONE;
}
enum hmd_reload_type hansi_rdc_checking(int InstID, int islocaled)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusConnection * connection = NULL;
	int ret = 0;
	int *rdc_check = NULL;
	int *rdc_check_timeout = NULL;
	if(islocaled)
	{
		if(HOST_BOARD->Hmd_Local_Inst[InstID])
		{
			connection = HOST_BOARD->Hmd_Local_Inst[InstID]->connection;
			rdc_check = &(HOST_BOARD->Hmd_Local_Inst[InstID]->rdc_check);
			rdc_check_timeout = &(HOST_BOARD->Hmd_Local_Inst[InstID]->rdc_check_timeout);
		}
	}
	else
	{
		if(HOST_BOARD->Hmd_Inst[InstID])
		{
			connection = HOST_BOARD->Hmd_Inst[InstID]->connection;
			rdc_check = &(HOST_BOARD->Hmd_Inst[InstID]->rdc_check);
			rdc_check_timeout = &(HOST_BOARD->Hmd_Inst[InstID]->rdc_check_timeout);
		}
	}
	if(connection == NULL)
	{
		hmd_syslog_info("%s InstID %d islocaled %d connection == NULL", __func__, InstID, islocaled);
		hmd_dbus_connection_init(InstID, islocaled);
	}
	if(NULL == rdc_check)
	{
		hmd_syslog_err("%s,%d,connection is NULL.\n", __func__, __LINE__);
		return HMD_RELOAD_MOD_RDC;
	}

	#define RDC_DBUS_NAME_FMT		"aw.rdc_%s_%d"
	#define RDC_DBUS_OBJPATH_FMT	"/aw/rdc_%s_%d"
	#define RDC_DBUS_INTERFACE_FMT	"aw.rdc_%s_%d"
	#define RDC_DBUS_CHECK_STATUS	"rdc_dbus_method_check_status"

	char RDC_DBUS_NAME[64];
	char RDC_DBUS_OBJPATH[64];
	char RDC_DBUS_INTERFACE[64];
	if (InstID > 0) {
		snprintf(RDC_DBUS_NAME, sizeof(RDC_DBUS_NAME) - 1,	RDC_DBUS_NAME_FMT, "r", InstID );
		snprintf(RDC_DBUS_OBJPATH, sizeof(RDC_DBUS_NAME) - 1, RDC_DBUS_OBJPATH_FMT, "r", InstID );
		snprintf(RDC_DBUS_INTERFACE, sizeof(RDC_DBUS_NAME) - 1, RDC_DBUS_INTERFACE_FMT, "r", InstID );
	}
	else
	{
		snprintf(RDC_DBUS_NAME, sizeof(RDC_DBUS_NAME) - 1,	RDC_DBUS_NAME_FMT, "l", InstID );
		snprintf(RDC_DBUS_OBJPATH, sizeof(RDC_DBUS_NAME) - 1, RDC_DBUS_OBJPATH_FMT, "l", InstID );
		snprintf(RDC_DBUS_INTERFACE, sizeof(RDC_DBUS_NAME) - 1, RDC_DBUS_INTERFACE_FMT, "l", InstID );
	}
	query = dbus_message_new_method_call(RDC_DBUS_NAME,
	                                     RDC_DBUS_OBJPATH,
	                                     RDC_DBUS_INTERFACE,
	                                     RDC_DBUS_CHECK_STATUS);
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (connection, query, 3000, &err);
	dbus_message_unref(query);
	if (NULL == reply)
	{
		if (dbus_error_is_set(&err))
		{
			if(!memcmp(err.name, DBUS_ERROR_NO_REPLY, strlen(DBUS_ERROR_NO_REPLY)) && !memcmp(err.message, "Did not receive a reply.", strlen("Did not receive a reply.")))
				*rdc_check_timeout += 1;
			else
				*rdc_check += 1;
			dbus_error_free(&err);
		}
		else
			*rdc_check += 1;
		if(*rdc_check >= 5 || *rdc_check_timeout >= 10)
		{
			*rdc_check = 0;
			*rdc_check_timeout = 0;
			hmd_syslog_crit("%s:%d	InstID %d islocaled %d rdc maybe wrong", 
					__func__, __LINE__, InstID, islocaled);
			return HMD_RELOAD_MOD_RDC;
		}
		hmd_syslog_info("%s  InstID %d islocaled %d  rdc_check %d", __func__, InstID, islocaled, *rdc_check);
		hmd_syslog_info("%s  InstID %d islocaled %d  rdc_check_timeout %d", __func__, InstID, islocaled, *rdc_check_timeout);
	}
	else if (dbus_message_get_args ( reply, &err,
											DBUS_TYPE_UINT32, &ret,
											DBUS_TYPE_INVALID))
	{
			dbus_message_unref(reply);
			*rdc_check = 0;
			*rdc_check_timeout = 0;
			return HMD_RELOAD_MOD_NONE;
	}
	else
	{
		if (dbus_error_is_set(&err))
		{
			if(!memcmp(err.name, DBUS_ERROR_NO_REPLY, strlen(DBUS_ERROR_NO_REPLY)) && !memcmp(err.message, "Did not receive a reply.", strlen("Did not receive a reply.")))
				*rdc_check_timeout += 1;
			else
				*rdc_check += 1;
			dbus_error_free(&err);
		}
		else
			*rdc_check += 1;
		if(*rdc_check >= 5 || *rdc_check_timeout >= 10)
		{
			*rdc_check = 0;
			*rdc_check_timeout = 0;
			hmd_syslog_crit("%s:%d InstID %d islocaled %d rdc maybe wrong",
								__func__, __LINE__, InstID, islocaled);
			return HMD_RELOAD_MOD_RDC;
		}
		hmd_syslog_info("%s 2 InstID %d islocaled %d rdc_check %d", __func__, InstID, islocaled, *rdc_check);
		hmd_syslog_info("%s  InstID %d islocaled %d  rdc_check_timeout %d", __func__, InstID, islocaled, *rdc_check_timeout);
	}

	return HMD_RELOAD_MOD_NONE;
}

enum hmd_reload_type hansi_pdc_checking(int InstID, int islocaled)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusConnection * connection = NULL;
	int ret = 0;
	int *pdc_check = NULL;
	int *pdc_check_timeout = NULL;
	if(islocaled)
	{
		if(HOST_BOARD->Hmd_Local_Inst[InstID])
		{
			connection = HOST_BOARD->Hmd_Local_Inst[InstID]->connection;
			pdc_check = &(HOST_BOARD->Hmd_Local_Inst[InstID]->pdc_check);
			pdc_check_timeout = &(HOST_BOARD->Hmd_Local_Inst[InstID]->pdc_check_timeout);
		}
	}
	else
	{
		if(HOST_BOARD->Hmd_Inst[InstID])
		{
			connection = HOST_BOARD->Hmd_Inst[InstID]->connection;
			pdc_check = &(HOST_BOARD->Hmd_Inst[InstID]->pdc_check);
			pdc_check_timeout = &(HOST_BOARD->Hmd_Inst[InstID]->pdc_check_timeout);
		}
	}
	if(connection == NULL)
	{
		hmd_syslog_info("%s InstID %d islocaled %d connection == NULL", __func__, InstID, islocaled);
		hmd_dbus_connection_init(InstID, islocaled);
	}
	if(NULL == pdc_check)
	{
		hmd_syslog_err("%s,%d,connection is NULL.\n", __func__, __LINE__);
		return HMD_RELOAD_MOD_PDC;
	}

	#define PDC_DBUS_NAME_FMT		"aw.pdc_%s_%d"
	#define PDC_DBUS_OBJPATH_FMT	"/aw/pdc_%s_%d"
	#define PDC_DBUS_INTERFACE_FMT	"aw.pdc_%s_%d"
	#define PDC_DBUS_CHECK_STATUS	"pdc_dbus_method_check_status"

	char PDC_DBUS_NAME[64];
	char PDC_DBUS_OBJPATH[64];
	char PDC_DBUS_INTERFACE[64];
	if (InstID > 0) {
		snprintf(PDC_DBUS_NAME, sizeof(PDC_DBUS_NAME) - 1,	PDC_DBUS_NAME_FMT, "r", InstID );
		snprintf(PDC_DBUS_OBJPATH, sizeof(PDC_DBUS_NAME) - 1, PDC_DBUS_OBJPATH_FMT, "r", InstID );
		snprintf(PDC_DBUS_INTERFACE, sizeof(PDC_DBUS_NAME) - 1, PDC_DBUS_INTERFACE_FMT, "r", InstID );
	}
	else
	{
		snprintf(PDC_DBUS_NAME, sizeof(PDC_DBUS_NAME) - 1,	PDC_DBUS_NAME_FMT, "l", InstID );
		snprintf(PDC_DBUS_OBJPATH, sizeof(PDC_DBUS_NAME) - 1, PDC_DBUS_OBJPATH_FMT, "l", InstID );
		snprintf(PDC_DBUS_INTERFACE, sizeof(PDC_DBUS_NAME) - 1, PDC_DBUS_INTERFACE_FMT, "l", InstID );
	}
	query = dbus_message_new_method_call(PDC_DBUS_NAME,
	                                     PDC_DBUS_OBJPATH,
	                                     PDC_DBUS_INTERFACE,
	                                     PDC_DBUS_CHECK_STATUS);
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (connection, query, 3000, &err);
	dbus_message_unref(query);
	if (NULL == reply)
	{
		if (dbus_error_is_set(&err))
		{
			if(!memcmp(err.name, DBUS_ERROR_NO_REPLY, strlen(DBUS_ERROR_NO_REPLY)) && !memcmp(err.message, "Did not receive a reply.", strlen("Did not receive a reply.")))
				*pdc_check_timeout += 1;
			else
				*pdc_check += 1;
			dbus_error_free(&err);
		}
		else
			*pdc_check += 1;
		if(*pdc_check >= 5 || *pdc_check_timeout >= 10)
		{
			*pdc_check = 0;
			*pdc_check_timeout = 0;
			hmd_syslog_crit("%s:%d	InstID %d islocaled %d pdc maybe wrong", 
					__func__, __LINE__, InstID, islocaled);
			return HMD_RELOAD_MOD_PDC;
		}
		hmd_syslog_info("%s  InstID %d islocaled %d  pdc_check %d", __func__, InstID, islocaled, *pdc_check);
		hmd_syslog_info("%s  InstID %d islocaled %d  pdc_check_timeout %d", __func__, InstID, islocaled, *pdc_check_timeout);
	}
	else if (dbus_message_get_args ( reply, &err,
											DBUS_TYPE_UINT32, &ret,
											DBUS_TYPE_INVALID))
	{
			dbus_message_unref(reply);
			*pdc_check = 0;
			*pdc_check_timeout = 0;
			return HMD_RELOAD_MOD_NONE;
	}
	else
	{
		if (dbus_error_is_set(&err))
		{
			if(!memcmp(err.name, DBUS_ERROR_NO_REPLY, strlen(DBUS_ERROR_NO_REPLY)) && !memcmp(err.message, "Did not receive a reply.", strlen("Did not receive a reply.")))
				*pdc_check_timeout += 1;
			else
				*pdc_check += 1;
			dbus_error_free(&err);
		}
		else
			*pdc_check += 1;
		if(*pdc_check >= 5 || *pdc_check_timeout >= 10)
		{
			*pdc_check = 0;
			*pdc_check_timeout = 0;
			hmd_syslog_crit("%s:%d InstID %d islocaled %d rdc maybe wrong",
								__func__, __LINE__, InstID, islocaled);
			return HMD_RELOAD_MOD_PDC;
		}
		hmd_syslog_info("%s 2 InstID %d islocaled %d rdc_check %d", __func__, InstID, islocaled, *pdc_check);
		hmd_syslog_info("%s  InstID %d islocaled %d  rdc_check_timeout %d", __func__, InstID, islocaled, *pdc_check_timeout);
	}

	return HMD_RELOAD_MOD_NONE;
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


unsigned int notice_had_to_change_vrrp_state(unsigned int InstID, int op)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err = {0};
	char OBJPATH[PATH_LEN] = {0};
	char BUSNAME[PATH_LEN] = {0};
	int ret = 0;	
	DBusConnection * connection = HOST_BOARD->Hmd_Inst[InstID]->connection;
	HMDReInitHadDbusPath(InstID,VRRP_DBUS_BUSNAME,BUSNAME,0);//book modify
	HMDReInitHadDbusPath(InstID,VRRP_DBUS_OBJPATH,OBJPATH,0);//book modify
	query = dbus_message_new_method_call(BUSNAME,
										OBJPATH,
										VRRP_DBUS_INTERFACE,
										VRRP_DBUS_METHOD_VRRP_STATE_CHANGE);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &InstID,
							DBUS_TYPE_UINT32, &op,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block(connection, query, 150000, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		hmd_syslog_err("delete hansi %d faild1.\n",InstID);
		return HMD_FALSE;
	}
	/* book modify */
	else if (!(dbus_message_get_args (reply, &err, DBUS_TYPE_UINT32,&ret, DBUS_TYPE_INVALID)))
	{		
	    //hmd_syslog_info("333333\n");
		if (dbus_error_is_set(&err)) 
		{
		    //hmd_syslog_info("444444\n");
			dbus_error_free(&err);
		}
	}
	
	dbus_message_unref(reply);
	if (VRRP_RETURN_CODE_OK != ret) 
	{//book modify
		hmd_syslog_err("change instrance %d state to %d faild.\n", InstID,op);
	}
	/* [2] kill the special had instance process which instance no is profile. */
	/* book add for stop had */
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
		if(HMD_DHCP_TO_START == op){
		fd = DHCP_MONITOR->tipcfd;
	//	hmd_syslog_info("~~~~~~fd id %d\n",fd);
		}
	}
	if(fd <= 0){
		hmd_syslog_err("%s,%d,invaid fd:%d.\n",__func__,__LINE__,fd);
		return -1;
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
int notice_hmd_server_delete_hansi(int InstID, int islocaled, HmdOP op){
	struct HmdMsg tmsg;
	int fd;
	tmsg.S_SlotID = HOST_SLOT_NO;
	tmsg.InstID = InstID;
	if(islocaled){
		if(HOST_BOARD->Hmd_Local_Inst[InstID]){
			fd = HOST_BOARD->Hmd_Local_Inst[InstID]->tipcfd;
		}
	}else{
		if(HOST_BOARD->Hmd_Inst[InstID]){
			fd = HOST_BOARD->Hmd_Inst[InstID]->tipcfd;
		}
	}
	tmsg.op = op;
	if(islocaled)
		tmsg.type = HMD_LOCAL_HANSI;
	else
		tmsg.type = HMD_HANSI;
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

int hmd_load_config(char *ConfigPath)
{
	char cmd[128] = {0};

	if (NULL == ConfigPath)
	{
		hmd_syslog_err("%s:%d parameter null\n", __func__, __LINE__);
		return -1;
	}

	hmd_syslog_info("load config %s...\n", ConfigPath);

	memset(cmd, 0, sizeof(cmd));
	snprintf(cmd, sizeof(cmd), "/opt/bin/vtysh -f %s -b", ConfigPath);
	system(cmd);

	return 0;
}


void hmd_eag_restart(unsigned islocal, unsigned int vrrid)
{
	char cmd[128];
	
	if (0 == vrrid) {
		islocal = 1;
	}
	snprintf(cmd, sizeof(cmd), "sudo /etc/init.d/eag stop %d %d", islocal, vrrid);
	system(cmd);
	snprintf(cmd, sizeof(cmd), "sudo /etc/init.d/eag start %d %d", islocal, vrrid);
	system(cmd);

	return;
}

void hmd_rdc_restart(unsigned islocal, unsigned int vrrid)
{
	char cmd[128];
	
	if (0 == vrrid) {
		islocal = 1;
	}
	snprintf(cmd, sizeof(cmd), "sudo /etc/init.d/rdcd stop %d %d", islocal, vrrid);
	system(cmd);
	snprintf(cmd, sizeof(cmd), "sudo /etc/init.d/rdcd start %d %d", islocal, vrrid);
	system(cmd);

	return;
}
void hmd_pdc_restart(unsigned islocal, unsigned int vrrid)
{
	char cmd[128];
	
	if (0 == vrrid) {
		islocal = 1;
	}
	snprintf(cmd, sizeof(cmd), "sudo /etc/init.d/pdcd stop %d %d", islocal, vrrid);
	system(cmd);
	snprintf(cmd, sizeof(cmd), "sudo /etc/init.d/pdcd start %d %d", islocal, vrrid);
	system(cmd);

	return;
}

int dhcp_to_start(){
	char buf[128] = {0};
	int ret = 0;
	char defaultSlotPath[] = "/var/run/config/slot";
	int SlotID =0 ,InstID = -1,islocaled =0;
	
	sprintf(buf,"sudo /usr/bin/dhcp_restart.sh");
	ret = system(buf);
	if(WEXITSTATUS(ret) == 2) {
			hmd_syslog_err("dhcp restart falsed.\n");
	}
	if(HOST_SLOT_NO != MASTER_SLOT_NO)
	notice_hmd_server_state_change(InstID, islocaled, HMD_DHCP_TO_START, 0);
	DHCP_MONITOR->RestartTimes+=1;
	if((HOST_SLOT_NO == MASTER_SLOT_NO)){
		SlotID = HOST_SLOT_NO;
		memset(buf, 0, 128);	
		sprintf(buf,"sudo /opt/bin/vtysh -f %s%d/%s -b &", defaultSlotPath,SlotID,"dhcp_poll");
		system(buf);
		for(InstID = 0;InstID <=16;InstID++){
		memset(buf, 0, 128);
		sprintf(buf,"sudo /opt/bin/vtysh -f %s%d/%s%d -b &", defaultSlotPath,SlotID,"hansi_dhcp",InstID);
		system(buf);
		}
	}

	return 0;
}

int hmd_eag_reload(unsigned slotid, unsigned int islocal, unsigned int vrrid)
{
	char path[128] = {0};
	
	snprintf(path, sizeof(path), "%s/slot%d/hansi_eag%d", DEFAULT_CONFIG_DIR, slotid, vrrid);
	hmd_syslog_info("path=%s\n", path);
	hmd_syslog_info("%s cfg hansi %d lock type %d\n", __func__, vrrid, HMD_RELOAD_MOD_EAG);
	//HMD_CFG_LOCK(vrrid, HMD_RELOAD_MOD_EAG);
	

	hmd_syslog_info("wait hansi %d restart eag thread\n", vrrid);
	
	if(HOST_SLOT_NO == MASTER_SLOT_NO)
	{
		if (slotid == HOST_SLOT_NO) {
			hmd_syslog_info("starta eag %d for slot %d\n", vrrid, slotid);
			hmd_eag_restart(islocal, vrrid);
		}
		sleep(5);
		hmd_load_config(path);
	}
	else
	{	/* active master rcv msg will reload InstID config */
		hmd_syslog_info("startb eag %d for slot %d\n", vrrid, slotid);
		hmd_eag_restart(islocal, vrrid);
		notice_hmd_server_state_change(vrrid, islocal, HMD_RELOAD_CONFIG_FOR_EAG, 0);
	}
		
	//HMD_CFG_UNLOCK(vrrid, HMD_RELOAD_MOD_EAG);
	hmd_syslog_info("%s cfg hansi %d unlock type %d\n", __func__, vrrid, HMD_RELOAD_MOD_EAG);
	
	//hmd_set_hot_restart_flag(HMD_CLR_FLAG, HOST_SLOT_NO, islocal, vrrid);

	return 0;
}

int hmd_rdc_reload(unsigned slotid, unsigned int islocal, unsigned int vrrid)
{
	char path[128] = {0};
	
	//hmd_set_hot_restart_flag(HMD_SET_FLAG, HOST_SLOT_NO, islocal, vrrid);
	
	snprintf(path, sizeof(path), "%s/slot%d/hansi_rdc%d", DEFAULT_CONFIG_DIR, slotid, vrrid);
	hmd_syslog_info("path=%s\n", path);
	
	
	hmd_syslog_info("%s cfg hansi %d lock type %d\n", __func__, vrrid, HMD_RELOAD_MOD_RDC);
	//HMD_CFG_LOCK(vrrid, HMD_RELOAD_MOD_RDC);
	
	hmd_rdc_restart(islocal, vrrid);

	hmd_syslog_info("wait hansi %d restart eag thread\n", vrrid);
	sleep(5);
	
	if(HOST_SLOT_NO == MASTER_SLOT_NO)
	{
			if (slotid == HOST_SLOT_NO) {
				hmd_syslog_info("starta rdc %d for slot %d\n", vrrid, slotid);
				hmd_rdc_restart(islocal, vrrid);
			}
			sleep(5);
			hmd_load_config(path);
	}
	else
	{	/* active master rcv msg will reload InstID config */
			hmd_syslog_info("startb rdc for slot %d\n", vrrid, slotid);
			hmd_rdc_restart(islocal, vrrid);
			notice_hmd_server_state_change(vrrid, islocal, HMD_RELOAD_CONFIG_FOR_RDC, 0);
	}
		
	//HMD_CFG_UNLOCK(vrrid, HMD_RELOAD_MOD_RDC);
	hmd_syslog_info("%s cfg hansi %d unlock type %d\n", __func__, vrrid, HMD_RELOAD_MOD_RDC);

	//hmd_set_hot_restart_flag(HMD_CLR_FLAG, HOST_SLOT_NO, islocal, vrrid);

	return 0;
}

int hmd_pdc_reload(unsigned slotid, unsigned int islocal, unsigned int vrrid)
{
	char path[128] = {0};
	
	//hmd_set_hot_restart_flag(HMD_SET_FLAG, HOST_SLOT_NO, islocal, vrrid);
	
	snprintf(path, sizeof(path), "%s/slot%d/hansi_pdc%d", DEFAULT_CONFIG_DIR, slotid, vrrid);
	hmd_syslog_info("path=%s\n", path);
	
	hmd_syslog_info("%s cfg hansi %d lock type %d\n", __func__, vrrid, HMD_RELOAD_MOD_PDC);
	//HMD_CFG_LOCK(vrrid, HMD_RELOAD_MOD_RDC);
	
	hmd_pdc_restart(islocal, vrrid);

	hmd_syslog_info("wait hansi %d restart eag thread\n", vrrid);
	sleep(5);
	
	if(HOST_SLOT_NO == MASTER_SLOT_NO)
	{
			if (slotid == HOST_SLOT_NO) {
				hmd_syslog_info("starta pdc for slot %d\n", vrrid, slotid);
				hmd_pdc_restart(islocal, vrrid);
			}
			sleep(5);
			hmd_load_config(path);
	}
	else
	{	/* active master rcv msg will reload InstID config */
			hmd_syslog_info("startb pdc %d for slot %d\n", vrrid, slotid);
			hmd_pdc_restart(islocal, vrrid);
			notice_hmd_server_state_change(vrrid, islocal, HMD_RELOAD_CONFIG_FOR_PDC, 0);
	}
		
	//HMD_CFG_UNLOCK(vrrid, HMD_RELOAD_MOD_RDC);
	hmd_syslog_info("%s cfg hansi %d unlock type %d\n", __func__, vrrid, HMD_RELOAD_MOD_PDC);

	//hmd_set_hot_restart_flag(HMD_CLR_FLAG, HOST_SLOT_NO, islocal, vrrid);

	return 0;
}

/* copy form aw3.0: notice AAT to clean sta list when hmd reset wcpss instance. 2014-11-1 by yjl */
int hmd_notice_aat_clean_stas(unsigned int vrrid)
{
	struct io_info tmp;
	int ret = 0;	
	static int fd = -1;

	if (vrrid >= MAX_INSTANCE)
	{
		hmd_syslog_err("vrrid %d not vaild range <0-16>\n", vrrid);
		return 1;
	}
	
	if (fd < 0)
	{
		if ((fd = open("/dev/aat0", O_RDWR)) < 0)
		{		
			hmd_syslog_err("%s: open failed:%s\n", __func__, strerror(errno));
			return 1;
		}
	}
	
	memset(&tmp, 0, sizeof(struct io_info));
	tmp.vrrid = vrrid;
	
	ret = ioctl(fd, AAT_IOC_CLEAN_STAS, &tmp);

	//close(fd);
	
	return 0;
}

int take_snapshot_timer_id;
int hmd_wcpss_reload(int vrrid,int islocal)
{
	char buf[128] = {0};
	char defaultPath[] = "/var/run/config/Instconfig";
	hmd_syslog_info("###%s hmd begin vrrid is %d islocal is %d line %d###\n",__func__,vrrid,islocal,__LINE__);

	hmd_syslog_info("%s notice AAT clean hansi %d STAS\n", __func__, vrrid);
	hmd_notice_aat_clean_stas(vrrid);

	if(HOST_BOARD->Hmd_Inst[vrrid] != NULL){
		notice_vrrp_config_service_change_state(vrrid, 0);
		hmd_syslog_info("###%s hmd begin vrrid is %d islocal is %d line %d###\n",__func__,vrrid,islocal,__LINE__);
		sprintf(buf,"sudo /etc/init.d/wcpss stop %d %d",islocal, vrrid);
		system(buf);			
		memset(buf, 0, 128);
		sprintf(buf,"sudo /etc/init.d/wcpss start %d %d",islocal, vrrid);
		system(buf);
		memset(buf, 0, 128);
		sprintf(buf,"sudo /etc/init.d/had start %d", vrrid);
		system(buf);
		hmd_syslog_info("###%s hmd begin vrrid is %d islocal is %d line %d###\n",__func__,vrrid,islocal,__LINE__);
 
		if(HOST_SLOT_NO != MASTER_SLOT_NO)
		{
			hmd_syslog_info("###%s hmd begin vrrid is %d islocal is %d line %d###\n",__func__,vrrid,islocal,__LINE__);
			notice_hmd_server_state_change(vrrid, islocal, HMD_RESTART, 0);
		}
		HOST_BOARD->Hmd_Inst[vrrid]->RestartTimes += 1;
		if(HOST_SLOT_NO == MASTER_SLOT_NO){
			hmd_syslog_info("###%s hmd begin vrrid is %d islocal is %d line %d###\n",__func__,vrrid,islocal,__LINE__);			
			memset(buf, 0, 128);
			sprintf(buf,"/opt/bin/vtysh -f %s%d-0-%d -b &",defaultPath,HOST_BOARD->slot_no,vrrid);
			system(buf);
		}
	}
   
	return 0;
}

int flush_hansi_rule(int islocaled, int insid)
{
	int ret = 0;
	char cmd[256] = "";
	//extern nmp_mutex_t eag_iptables_lock;

	//nmp_mutex_lock(&eag_iptables_lock);
	snprintf( cmd, sizeof(cmd)-1, 
			"sudo /usr/bin/cp_flush_hansi_rule.sh %d %d", islocaled, insid);
	ret = system(cmd);
	ret = WEXITSTATUS(ret);
	//nmp_mutex_unlock(&eag_iptables_lock);
	hmd_syslog_info("flush_hansi_rule cmd=%s ret=%d", cmd, ret);
	return ret;
}


int hansi_state_check(int InstID, int islocaled, enum hmd_reload_type type){
	char buf[128] = {0};
	char defaultPath[] = "/var/run/config/Instconfig";
	int ret = 0;	
	
	if(islocaled){
		if(HOST_BOARD->Hmd_Local_Inst[InstID] != NULL){
			sprintf(buf,"sudo /etc/init.d/wcpss stop %d %d",islocaled, InstID);
			system(buf);
			memset(buf, 0, 128);
			sprintf(buf,"sudo /etc/init.d/wcpss start %d %d",islocaled, InstID);
			system(buf);

			hmd_syslog_info("ready to delbr for inst %d\n", InstID);
			//delete ebr or wlan for this inst from bridge
			memset(buf, 0, 128);
			sprintf(buf,"clear_ebr.sh ebr%d-%d-", HOST_BOARD->slot_no, InstID);
			system(buf);
			memset(buf, 0, 128);
			sprintf(buf,"clear_ebr.sh wlan%d-%d-", HOST_BOARD->slot_no, InstID);
			system(buf);
			hmd_syslog_info("delbr done \n");
			
			//unregist radio if for this Inst from kernel
			hmd_syslog_info("ready to unregist radio if. refer to dmesg\n");
			memset(buf, 0, 128);
			sprintf(buf,"clear_radio_if %d", InstID);
			system(buf);
			hmd_syslog_info("unregist radio if done\n");
			
			hmd_syslog_info("###%s line %d hmd begin load takesnapshot.sh g_loable_takesnapshot_flag is %d###\n",__func__,__LINE__,g_loable_takesnapshot_flag);
			if(g_loable_takesnapshot_flag == 0)
			{
				g_loable_takesnapshot_flag = 1;
				hmd_syslog_info("takesnapshotfun ###%s %d ###\n",__func__,__LINE__);
				HMDTimerRequest(600,&take_snapshot_timer_id, HMD_TIMER_TAKESNAPSHOT, 0, 0);
				ret = system("takesnapshot.sh 1 3&");
				ret = WEXITSTATUS(ret);
				hmd_syslog_info("###takesnapshotfun %s %d ret is %d###\n",__func__,__LINE__,ret);

			}
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
			if (HMD_RELOAD_MOD_EAG == type) {
				hmd_syslog_warning("reload eag %s %d\n", __FUNCTION__, __LINE__);
				flush_hansi_rule(islocaled, InstID);
				hmd_eag_reload(HOST_SLOT_NO, islocaled, InstID);
			}
			else if (HMD_RELOAD_MOD_RDC == type) {
				hmd_syslog_warning("reload rdc %s %d\n", __FUNCTION__, __LINE__);
				hmd_rdc_reload(HOST_SLOT_NO, islocaled, InstID);
			} else if (HMD_RELOAD_MOD_PDC == type){
				hmd_syslog_warning("reload pdc %s %d\n", __FUNCTION__, __LINE__);
				hmd_pdc_reload(HOST_SLOT_NO, islocaled, InstID);
			} else if (HMD_RELOAD_MOD_WCPSS == type) {
				
				notice_vrrp_config_service_change_state(InstID, 0);
				sprintf(buf,"sudo /etc/init.d/wcpss stop %d %d",islocaled, InstID);
				system(buf);			
				memset(buf, 0, 128);
				sprintf(buf,"sudo /etc/init.d/wcpss start %d %d",islocaled, InstID);
				system(buf);
				memset(buf, 0, 128);
				sprintf(buf,"sudo /etc/init.d/had start %d", InstID);
				system(buf);

				hmd_syslog_info("ready to delbr for inst %d\n", InstID);
				//delete ebr or wlan for this inst from bridge
				memset(buf, 0, 128);
				sprintf(buf,"clear_ebr.sh ebr%d-%d-", HOST_BOARD->slot_no, InstID);
				system(buf);
				memset(buf, 0, 128);
				sprintf(buf,"clear_ebr.sh wlan%d-%d-", HOST_BOARD->slot_no, InstID);
				system(buf);
				hmd_syslog_info("delbr done \n");
					
				//unregist radio if for this Inst from kernel
				hmd_syslog_info("ready to unregist radio if. refer to dmesg\n");
				memset(buf, 0, 128);
				sprintf(buf,"clear_radio_if %d", InstID);
				system(buf);
				hmd_syslog_info("unregist radio if done\n");
				
				hmd_syslog_info("###%s hmd begin load takesnapshot.sh ###\n",__func__);
				if(g_loable_takesnapshot_flag == 0)
				{
					g_loable_takesnapshot_flag = 1;
					hmd_syslog_info("###takesnapshotfun %s %d ###\n",__func__,__LINE__);
					HMDTimerRequest(600,&take_snapshot_timer_id, HMD_TIMER_TAKESNAPSHOT, 0, 0);
					ret = system("takesnapshot.sh 1 3&");
					ret = WEXITSTATUS(ret);
					hmd_syslog_info("###takesnapshotfun %s %d ret is %d###\n",__func__,__LINE__,ret);

				}
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
			} else {
				hmd_syslog_warning("unknown type %d\n", type);
			}
		}
	}
	return 0;
}
void DHCP_CHECK(){
//	hmd_syslog_info("DHCP_CHECK Doing\n");
	int ret;
	if(DHCP_RESTART_FLAG){
		ret = dhcp_running_checking();
		if(ret == HMD_FALSE){
			hmd_syslog_warning("check dhcp false,need restart dhcp.\n");
			ret = dhcp_to_start();
		}
	}
	HMDTimerRequest(HMD_CHECKING_TIMER,&(DHCP_MONITOR->HmdTimerID), HMD_CHECK_FOR_DHCP, 0, 0);
	
}

/* yjl add for checking memory and process of asd ,wid and had. 2014-10-30 */ 
/* get memory check flag from file. 
    flag = 1, asd memory usage out of limits.
    flag = 2, wid
    flag = 3, had */
int get_memory_check_flag(int InstID, int islocaled, int* flag)
{
    char defaultPath[128] = {0};
	FILE* fp = NULL;
	enum wcpss_restart_type type = 0;

	sprintf(defaultPath,"/var/run/wcpss/memory_check_flag%d_%d", islocaled, InstID);
	
	fp = fopen(defaultPath, "r");
	if(NULL == fp){
		hmd_syslog_warning("open file %s failed .\n", defaultPath);
		return -1;
	}
	fscanf(fp, "%d", flag);
	fclose(fp);

	switch(*flag){
		case 1:
			type = ASD_MEMORY_LEAK;
			hmd_syslog_warning("asd memory usage out of limits, need restart wcpss, type %d.\n", type);
		    break;
		case 2:
			type = WID_MEMORY_LEAK;
			hmd_syslog_warning("wid memory usage out of limits, need restart wcpss, type %d.\n", type);
			break;
		case 3:
			type = HAD_MEMORY_LEAK;
			hmd_syslog_warning("had memory usage out of limits, need restart wcpss, type %d.\n", type);
			break;
		default:
			break;
	}				
	
	return 0;
}

/* get process check flag from file. 
    flag = 1, asd process hang dead
    flag = 2, wid
    flag = 3, had */
int get_process_check_flag(int InstID, int islocaled, int* flag)
{
    char defaultPath[128] = {0};
	FILE* fp = NULL;
	enum wcpss_restart_type type = 0;

	sprintf(defaultPath,"/var/run/wcpss/process_check_flag%d_%d", islocaled, InstID);
	
	fp = fopen(defaultPath, "r");
	if(NULL == fp){
		hmd_syslog_warning("open file %s failed .\n", defaultPath);
		return -1;
	}
	fscanf(fp, "%d", flag);
	fclose(fp);

	switch(*flag){
		case 1:
		    type = ASD_HANG_DEAD;
			hmd_syslog_warning("asd process hang dead, need restart wcpss, type %d.\n", type);
		    break;
		case 2:
			type = WID_HANG_DEAD;
			hmd_syslog_warning("wid process hang dead, need restart wcpss, type %d.\n", type);
			break;
		case 3:
			type = HAD_HANG_DEAD;
			hmd_syslog_warning("had process hang dead, need restart wcpss, type %d.\n", type);
			break;
		default:
			break;
	}				
	
	return 0;
}

void HANSI_CHECKING(int InstID, int islocaled){
	//int ret = HMD_TRUE;
	int i =0;
	char * ifname = NULL;
	char * mac = NULL;
	int vip;
	int mask;
	enum hmd_reload_type type = 0;
	int process_check_flag = 0, memory_check_flag = 0; 
	char buf[128] = {0};

	hmd_syslog_warning("HANSI_CHECKING\n");
	if(HANSI_CHECK_OP){
		hmd_syslog_warning("checking wcpss\n");
		type = hansi_wireless_checking(InstID, islocaled);
		if (HMD_RELOAD_MOD_NONE != type) {
			hmd_syslog_warning("check wireless false,need restart wcpss.\n");
			hansi_state_check(InstID, islocaled, type);
		}
				
		/* yjl add for checking memory and process of asd ,wid and had. 2014-10-30 */ 
		get_process_check_flag(InstID, islocaled, &process_check_flag);
		get_memory_check_flag(InstID, islocaled, &memory_check_flag);
		if((process_check_flag != 0) || (memory_check_flag != 0)){
			type = HMD_RELOAD_MOD_WCPSS;
		}
		
		if (HMD_RELOAD_MOD_NONE != type){
			hmd_syslog_warning("check wireless false,need restart wcpss.\n");
			hansi_state_check(InstID, islocaled, type);

			/* wcpss restarted, change check flag to 0 */
			if(process_check_flag != 0){
				sprintf(buf, "echo 0 > /var/run/wcpss/process_check_flag%d_%d", islocaled, InstID);
				system(buf);
			}
			if(memory_check_flag != 0){
				memset(buf, 0, sizeof(buf));
				sprintf(buf, "echo 0 > /var/run/wcpss/memory_check_flag%d_%d", islocaled, InstID);
				system(buf);
			}
		}
		hmd_syslog_warning("checking eag\n");
		type = hansi_eag_checking(InstID, islocaled);
		if (HMD_RELOAD_MOD_NONE != type)
		{
			hmd_syslog_warning("check eag fail ,need restart type %d.\n", type);
			hansi_state_check(InstID, islocaled, type);
		}
		hmd_syslog_warning("checking rdc\n");
		type = hansi_rdc_checking(InstID, islocaled);
		if (HMD_RELOAD_MOD_NONE != type)
		{
			hmd_syslog_warning("check rdc fail ,need restart type %d.\n", type);
			hansi_state_check(InstID, islocaled, type);
		}

		hmd_syslog_warning("checking pdc\n");
		type = hansi_pdc_checking(InstID, islocaled);
		if (HMD_RELOAD_MOD_NONE != type)
		{
			hmd_syslog_warning("check pdc fail ,need restart type %d.\n", type);
			hansi_state_check(InstID, islocaled, type);
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
			/*notice server the hansi is deleting,config hansi is not permitted*/
			if(HOST_SLOT_NO != MASTER_SLOT_NO){
				notice_hmd_server_delete_hansi(InstID, islocaled, HMD_IS_DELETE_HANSI);
			}
			else{
				HOST_BOARD->Hmd_Inst[InstID]->delete_flag = 1;
			}
			/*end*/
			notice_vrrp_config_service_change_state(InstID, 0);
			hansi_wireless_notice_quit(InstID,islocaled);
			/*notice server free hansi*/
			if(HOST_SLOT_NO != MASTER_SLOT_NO){
				notice_hmd_server_delete_hansi(InstID, islocaled, HMD_SERVER_DELETE_HANSI);
			}
			else{
				/*this place no useful only for up*/
				HOST_BOARD->Hmd_Inst[InstID]->delete_flag = 1;
			}
			/*end*/
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
			case HMD_DHCP_CHECKING:
		//	hmd_syslog_info("HMD_DHCP_CHECKING~~~~\n");
				DHCP_CHECK();
				break;
			default:
				break;
		}
	}
	return NULL;
}
