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
* ACProtocol.c
*
*
* CREATOR:
* autelan.software.wireless-control. team
*
* DESCRIPTION:
* wid module
*
*
*******************************************************************************/
#include "wcpss/waw.h"

#include "wcpss/wid/WID.h"
#include "CWAC.h"
#include "ACDbus_handler.h"

#ifdef DMALLOC
#include "../dmalloc-5.5.0/dmalloc.h"
#endif

//added by weiay 20080618
CWBool CWAssembleMsgElemWTPVersion(CWProtocolMessage *msgPtr,char *version)
{

	//printf("********* CWAssembleMsgElemWTPVersion *******\n");
	if(msgPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	if(version == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);

	
	char *str_Version = version;
	unsigned int usize = 8 + strlen(str_Version);
	//printf("*** version is:<%s> size is:<%d>****\n",str_Version,strlen(str_Version));
	
	// create message
	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, usize, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	
	CWProtocolStore8(msgPtr, 1);//AC Surport version count
	CWProtocolStore8(msgPtr, 0);//reserved
	CWProtocolStore16(msgPtr, gCWImageDataPendingTimer);//reserved

	CWProtocolStore16(msgPtr, 1000);//AC Surport wtp type
	CWProtocolStore16(msgPtr, strlen(str_Version)); 
	CWProtocolStoreStr(msgPtr, str_Version); 
		
	return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_IMAGE_IDENTIFIER_CW_TYPE);

}

/*____________________________________________________________________________*/
/*  *****************************___ASSEMBLE___*****************************  */
CWBool CWAssembleMsgElemACDescriptor(CWProtocolMessage *msgPtr) {
	CWACVendorInfos infos;
	int i=0, size=0;
	if(msgPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);;
	
	if(!CWACGetVendorInfos(&infos)) { // get infos
		return CW_FALSE;
	}

	for(i = 0; i < infos.vendorInfosCount; i++) {
		size += (8 + ((infos.vendorInfos)[i]).length);
	}
	
	size += 12; // size of message in bytes (excluding vendor infos, already counted)
	
	// create message
	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, size, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	
	CWProtocolStore16(msgPtr, CWACGetStations()); // Number of mobile stations associated
	CWProtocolStore16(msgPtr, CWACGetLimit()); // Maximum number of mobile stations supported	
	CWProtocolStore16(msgPtr, CWACGetActiveWTPs()); // Number of WTPs active	
	CWProtocolStore16(msgPtr, CWACGetMaxWTPs()); // Maximum number of WTPs supported	
	CWProtocolStore8(msgPtr, CWACGetSecurity());
	CWProtocolStore8(msgPtr, CWACGetRMACField());
	CWProtocolStore8(msgPtr, 0);			//Reserved
	CWProtocolStore8(msgPtr, CWACGetDTLSPolicy()); 	// DTLS Policy
	
//	wid_syslog_debug_debug("Vendor Count: %d", infos.vendorInfosCount);

	for(i=0; i<infos.vendorInfosCount; i++) {
		CWProtocolStore32(msgPtr, ((infos.vendorInfos)[i].vendorIdentifier));
		CWProtocolStore16(msgPtr, ((infos.vendorInfos)[i].type));
		CWProtocolStore16(msgPtr, ((infos.vendorInfos)[i].length));
		if((infos.vendorInfos)[i].length == 4) {
			*((infos.vendorInfos)[i].valuePtr) = htonl(*((infos.vendorInfos)[i].valuePtr));
		}
		CWProtocolStoreRawBytes(msgPtr, (char*) ((infos.vendorInfos)[i].valuePtr), (infos.vendorInfos)[i].length);
	}
	
	CWACDestroyVendorInfos(&infos);
	
	return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_AC_DESCRIPTOR_CW_TYPE);
}

CWBool CWAssembleMsgElemACIPv4List(CWProtocolMessage *msgPtr) 
{
	int *list = NULL;
	int count, i;
	const int IPv4_List_length=4;
	if(msgPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	
	if(!CWACGetACIPv4List(&list, &count)) {
		if(list){
			free(list);
			list = NULL;
		}
		return CW_FALSE;
	}
	// create message
	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, IPv4_List_length*count, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	
	for(i = 0; i < count; i++) {
		CWProtocolStore32(msgPtr, list[i]);
//		wid_syslog_debug_debug("AC IPv4 List(%d): %d", i, list[i]);
	}
	
	CW_FREE_OBJECT(list);
				
	return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_AC_IPV4_LIST_CW_TYPE);
}

CWBool CWAssembleMsgElemACIPv6List (CWProtocolMessage *msgPtr) 
{
	struct in6_addr *list;
	const int IPv6_List_length=16;
	int count, i;
	
	if(msgPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	
	if(!CWACGetACIPv6List(&list, &count)) return CW_FALSE;
	
	// create message
	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, IPv6_List_length*count, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	
	/*--- ATTENZIONE! l'indirizzo ipv6 forse deve essere girato ---*/
	for(i = 0; i < count; i++) {
		CWProtocolStoreRawBytes(msgPtr, (char*)list[i].s6_addr, 16);
	}
	
	CW_FREE_OBJECT(list);

	return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_AC_IPV6_LIST_CW_TYPE);
}

CWBool CWAssembleMsgElemACName(CWProtocolMessage *msgPtr) {
	char *name;
	if(msgPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	
	name = CWACGetName();
	if(NULL == name){
		return CW_FALSE;
	}
	// create message
	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, (strlen(name)+1), return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	
	CWProtocolStoreStr(msgPtr, name);
	
//	wid_syslog_debug_debug("AC Name: %s", name);
	
	return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_AC_NAME_CW_TYPE);
}
/*
CWBool CWAssembleMsgElemCWControlIPv4Addresses(CWProtocolMessage *msgPtr) {
	int count, i;
	
	if(msgPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	
	count = CWACGetInterfacesCount();
	
	if(count <= 0) {
		return CWErrorRaise(CW_ERROR_NEED_RESOURCE, "No Interfaces Configured");
	}
	
	for(i = 0; i < count; i++) { // one Message Element for each interface
		CWProtocolMessage temp;
		// create message
		CW_CREATE_PROTOCOL_MESSAGE(temp, 6, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
		
		CWProtocolStore32(&temp, CWACGetInterfaceIPv4AddressAtIndex(i));
		CWProtocolStore16(&temp, CWACGetInterfaceWTPCountAtIndex(i));
		
		CWAssembleMsgElem(&temp, CW_MSG_ELEMENT_CW_CONTROL_IPV4_ADDRESS_CW_TYPE);
		
		if(i == 0) {
			CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, (temp.offset)*count, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
		}
		
		CWProtocolStoreMessage(msgPtr, &temp);
		CW_FREE_PROTOCOL_MESSAGE(temp);
	}
	
	return CW_TRUE;
}
*/
CWBool CWAssembleMsgElemCWControlIPv4Addresses(CWProtocolMessage *msgPtr, unsigned int WTPID) {
	int count, i;
	int j = 0; 
	if(msgPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	if(AC_WTP[WTPID]!=NULL){
		
		CWThreadMutexLock(&ACIPLISTMutex);
		for(j = 0; j < ACIPLIST_NUM; j++){
			if((AC_IP_GROUP[j] != NULL)&&(strcmp(AC_WTP[WTPID]->BindingIFName, (char*)AC_IP_GROUP[j]->ifname)==0)){
				break;
			}
		}
		if((j != ACIPLIST_NUM)&&(AC_IP_GROUP[j]->ipnum != 0)){
			count = AC_IP_GROUP[j]->ipnum;
			struct wid_ac_ip *tmp = AC_IP_GROUP[j]->ip_list;
			for(i = 0; i < count; i++) { // one Message Element for each interface
				CWProtocolMessage temp;
				// create message
				CW_CREATE_PROTOCOL_MESSAGE(temp, 6, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
				
				CWProtocolStore32(&temp, inet_addr(tmp->ip));
				CWProtocolStore16(&temp, 9999);
				
				CWAssembleMsgElem(&temp, CW_MSG_ELEMENT_CW_CONTROL_IPV4_ADDRESS_CW_TYPE);
				
				if(i == 0) {
					CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, (temp.offset)*count, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
				}
				
				CWProtocolStoreMessage(msgPtr, &temp);
				CW_FREE_PROTOCOL_MESSAGE(temp);
				tmp = tmp->next;
			}			
			CWThreadMutexUnlock(&ACIPLISTMutex);
			return CW_TRUE;
		}		
		CWThreadMutexUnlock(&ACIPLISTMutex);
	}
	
	count = CWACGetInterfacesIpv4Count();
	
	if(count <= 0) {
		return CWErrorRaise(CW_ERROR_NEED_RESOURCE, "No Interfaces Configured");
	}
	
	for(i = 0; i < count; i++) { // one Message Element for each interface
		CWProtocolMessage temp;
		// create message
		CW_CREATE_PROTOCOL_MESSAGE(temp, 6, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
		
		CWProtocolStore32(&temp, CWACGetInterfaceIPv4AddressAtIndex(i));
		CWProtocolStore16(&temp, CWACGetInterfaceWTPCountAtIndex(i));
		
		CWAssembleMsgElem(&temp, CW_MSG_ELEMENT_CW_CONTROL_IPV4_ADDRESS_CW_TYPE);
		
		if(i == 0) {
			CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, (temp.offset)*count, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
		}
		
		CWProtocolStoreMessage(msgPtr, &temp);
		CW_FREE_PROTOCOL_MESSAGE(temp);
	}
	
	return CW_TRUE;
}

CWBool CWAssembleMsgElemCWControlIPv6Addresses(CWProtocolMessage *msgPtr) {
	int count, i;
	if(msgPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	
	count = CWACGetInterfacesIpv6Count();
	
	for(i = 0; i < count; i++) { // one Message Element for each interface
		CWProtocolMessage temp;
		// create message
		CW_CREATE_PROTOCOL_MESSAGE(temp, 18, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
		
		CWProtocolStoreRawBytes(&temp, CWACGetInterfaceIPv6AddressAtIndex(i), 16);
		CWProtocolStore16(&temp, CWACGetInterfaceWTPCountAtIndex(i));
		
		CWAssembleMsgElem(&temp, CW_MSG_ELEMENT_CW_CONTROL_IPV6_ADDRESS_CW_TYPE);
		
		if(i == 0) {
			CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, (temp.offset)*count, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
		}
		
		CWProtocolStoreMessage(msgPtr, &temp);
		CW_FREE_PROTOCOL_MESSAGE(temp);
	}
	
	return CW_TRUE;
}

CWBool CWAssembleMsgElemCWTimer (CWProtocolMessage *msgPtr, int WTPID) 
{
	int discoveryTimer, echoTimer;
	const int CWTimer_length=2;
	if(msgPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	
	// create message
	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, CWTimer_length, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	
	if(!(CWACGetDiscoveryTimer(&discoveryTimer)) || !(CWACGetEchoRequestTimer(&echoTimer))) return CW_FALSE;
	echoTimer = AC_WTP[WTPID]->EchoTimer;
	CWProtocolStore8(msgPtr, discoveryTimer);
	CWProtocolStore8(msgPtr, echoTimer);
	
//	wid_syslog_debug_debug("Discovery Timer: %d", discoveryTimer);
//	wid_syslog_debug_debug("Echo Timer: %d", echoTimer);
	
	return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_CW_TIMERS_CW_TYPE);
}

/* Le informazioni sui Radio ID vengono prese dalle informazioni del Configure Message 
   Provvisoriamente l'error Report Period 猫 settato allo stesso valore per tutte le radio del WTP*/
CWBool CWAssembleMsgElemDecryptErrorReportPeriod (CWProtocolMessage *msgPtr, int WTPID)
{
	const int radio_Decrypt_Error_Report_Period_Length=3;
	const int reportInterval=15;
	CWProtocolMessage *msgs;
	CWRadioAdminInfoValues *radiosInfoPtr;
	int radioCount=0;
	int iPtr;
	int len = 0;
	int i;
	int j;
	if(msgPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	
	/*if((iPtr = ((int*)CWThreadGetSpecific(&gIndexSpecific))) == NULL) {
		wid_syslog_crit("Critical Error... closing thread");
		CWCloseThread();
	}*/
	iPtr = WTPID;
	radiosInfoPtr=gWTPs[iPtr].WTPProtocolManager.radioAdminInfo.radios;
	radioCount=gWTPs[iPtr].WTPProtocolManager.radioAdminInfo.radiosCount;
	
	CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgs, radioCount, return  CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	
	for (i=0; i<radioCount; i++)
	{
		// create message
		CW_CREATE_PROTOCOL_MESSAGE(msgs[i], radio_Decrypt_Error_Report_Period_Length, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
		CWProtocolStore8(&(msgs[i]), radiosInfoPtr[i].ID); // ID of the radio
		CWProtocolStore16(&(msgs[i]), reportInterval); // state of the radio
		
		if(!(CWAssembleMsgElem(&(msgs[i]), CW_MSG_ELEMENT_CW_DECRYPT_ER_REPORT_PERIOD_CW_TYPE))) {
			for(j = i; j >= 0; j--) { CW_FREE_PROTOCOL_MESSAGE(msgs[j]);}
			CW_FREE_OBJECT(msgs);
			return CW_FALSE;
		}
		
		len += msgs[i].offset;
//	wid_syslog_debug_debug("Decrypt Error Report Period: %d - %d", radiosInfoPtr[i].ID, reportInterval);
	}
	
	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, len, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	
	for(i = 0; i < radioCount; i++) {
		CWProtocolStoreMessage(msgPtr, &(msgs[i]));
		CW_FREE_PROTOCOL_MESSAGE(msgs[i]);
	}
	
	CW_FREE_OBJECT(msgs);

	return CW_TRUE;
}

CWBool CWAssembleMsgElemIdleTimeout (CWProtocolMessage *msgPtr)
{
	int idleTimeout;
	const int idle_Timeout_length=4;
	if(msgPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);

	// create message
	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, idle_Timeout_length, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	
	if(!(CWACGetIdleTimeout(&idleTimeout))) return CW_FALSE;
	CWProtocolStore32(msgPtr, idleTimeout);
	
	//wid_syslog_debug_debug("Idle Timeout: %d", idleTimeout);
	
	return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_IDLE_TIMEOUT_CW_TYPE);
}


CWBool CWAssembleMsgElemWTPFallback (CWProtocolMessage *msgPtr)
{
	int value=0; //PROVVISORIO
	const int WTP_fallback_length=1;
	if(msgPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);

	// create message
	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, WTP_fallback_length, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	
	CWProtocolStore8(msgPtr, value);
	
//	wid_syslog_debug_debug("Fallback: %d", value);
	
	return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_WTP_FALLBACK_CW_TYPE);
}

CWBool CWAssembleMsgElemAPScanningSet(CWProtocolMessage *msgPtr)
{
	//printf("#### CWAssembleMsgElemAPScanningSet ####\n");
	//printf("opstate = %d\n",gapscanset.opstate);
	//printf("reportinterval = %d\n",gapscanset.reportinterval);
	
	int value=0; //ap scanning settting 0
	const int ap_scanset_length = 4;
	if(msgPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);

	// create message
	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, ap_scanset_length, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	
	CWProtocolStore8(msgPtr, value); //ap sanning setting  0
	CWProtocolStore8(msgPtr, gapscanset.opstate);
	CWProtocolStore16(msgPtr, gapscanset.reportinterval);
		
	return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE);

}
CWBool CWAssembleMsgElemAPStatisticsSet(CWProtocolMessage *msgPtr,int apstatics)  //fengwenchao modify 20110422
{
	//printf("#### CWAssembleMsgElemAPStatisticsSet ####\n");
	//printf("apstatistics = %d\n",apstatistics);
	
	int value=1; //ap statistics settting 1
	const int ap_statistics_length = 2;
	if(msgPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	unsigned char char_apstatics = 0;  //fengwenchao add 20110422
	// create message
	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, ap_statistics_length, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	
	CWProtocolStore8(msgPtr, value); //ap statistics setting  1
	/*fengwenchao modify begin 20110422*/
	if(apstatics == -1)
	{
		CWProtocolStore8(msgPtr, apstatistics);
	}
	else
	{
		char_apstatics = (unsigned char)apstatics;
		CWProtocolStore8(msgPtr, char_apstatics);
	}
	/*fengwenchao modify end*/		
	wid_syslog_debug_debug(WID_WTPINFO,"%s,%d,apstatistics switch is %s.\n",__func__,__LINE__,(apstatistics != 0)?"enable":"disable");
	return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE);

}
CWBool CWAssembleMsgElemAPStatistics_Interval_Set(CWProtocolMessage *msgPtr,unsigned wtpid)
{
	
	int value=1; //ap statistics settting 1
	unsigned char Reserve = 0;
	unsigned short interval = 0;
	const int ap_statistics_length = 2;
	if(msgPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	interval = (unsigned short)AC_WTP[wtpid]->apstatisticsinterval;

	// create message
	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, ap_statistics_length, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	
	CWProtocolStore8(msgPtr, value); //ap statistics setting  1
	CWProtocolStore8(msgPtr, Reserve);
	CWProtocolStore16(msgPtr, interval); 
	CWProtocolStore8(msgPtr, Reserve); 
	CWProtocolStore8(msgPtr, Reserve); 
	CWProtocolStore8(msgPtr, Reserve); 
	CWProtocolStore8(msgPtr, Reserve); 
	wid_syslog_debug_debug(WID_WTPINFO,"%s,%d,apstatistics switch is %s,interval is %d.\n",__func__,__LINE__,(apstatistics != 0)?"enable":"disable",interval);
	return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_ALL_TIME_SET_CW_TYPE);

}
/*zhaoruijia,20100904,transplant ACTIMESYNCHROFAILURE from 1.2omc to 1.3,start*/

CWBool CWAssembleMsgElemAP_NTP_Set(CWProtocolMessage *msgPtr,unsigned wtpid)
{
	
	int value=10; //ap ntp settting 1
	unsigned char ipv4_v6 = 0;//default ipv4--0,ipv6--1
	unsigned char state = 0;
	struct in6_addr list;
	unsigned int interval = 0;
	unsigned char Reserve = 0;
	unsigned int ap_ntp_length = 16;
	if(ipv4_v6 == 0){
		ap_ntp_length = 16;
	}else{
		ap_ntp_length = 76;
	}
	struct sockaddr_in adr_inet;
	unsigned int ip= 0;
	if(msgPtr == NULL) 
		return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	interval = AC_WTP[wtpid]->ntp_interval;
	int ret = 0;
	wid_syslog_info("wid_set_wtp_ntp wtpid %d.\n",wtpid);
	char *addr;
	char *ifname;

	ifname = (char *)malloc(ETH_IF_NAME_LEN+1);
	if(ifname == NULL)
	{
		wid_syslog_info("%s malloc %s",__func__,strerror(errno));
		perror("malloc");
		return 0;
	}
	memset(ifname,0,ETH_IF_NAME_LEN+1);
	memcpy(ifname,AC_WTP[wtpid]->BindingIFName,strlen(AC_WTP[wtpid]->BindingIFName));

	struct ifi_info *ifi = (struct ifi_info*)calloc(1, sizeof(struct ifi_info));
	memset(ifi,0,sizeof(struct ifi_info));
	memset(ifi->ifi_name,0,sizeof(ifi->ifi_name));
	strncpy(ifi->ifi_name,ifname,sizeof(ifi->ifi_name));
	ret = Get_Interface_Info(ifname,ifi);
	if(ret != 0){
		wid_syslog_err("<err>%s ret=%d.",__func__,ret);
		if(ifi->ifi_addr != NULL){
			free(ifi->ifi_addr);
			ifi->ifi_addr = NULL;
		}		
#ifdef	SIOCGIFBRDADDR
		if(ifi->ifi_brdaddr != NULL){
			free(ifi->ifi_brdaddr);
			ifi->ifi_brdaddr = NULL;
		}
#endif
		free(ifi);
		ifi = NULL;
		CW_FREE_OBJECT(ifname);
		//return WID_DBUS_ERROR;		
	}else{
		addr = (char *)malloc(ETH_IF_NAME_LEN+1);
		if(addr == NULL)
		{
			wid_syslog_info("%s malloc %s",__func__,strerror(errno));
			perror("malloc");
			if(ifname){
				free(ifname);
				ifname = NULL;
			}
			if(ifi->ifi_addr != NULL){
				free(ifi->ifi_addr);
				ifi->ifi_addr = NULL;
			}		
#ifdef	SIOCGIFBRDADDR
			if(ifi->ifi_brdaddr != NULL){
				free(ifi->ifi_brdaddr);
				ifi->ifi_brdaddr = NULL;
			}
#endif
			if(ifi){
				free(ifi);
				ifi = NULL;
			}
			return 0;
		}
		memset(addr,0,ETH_IF_NAME_LEN+1);
		
		wid_syslog_info("netaddr:%x\n",((struct sockaddr_in*)(ifi->ifi_addr))->sin_addr);
		sprintf(addr,"%s",inet_ntoa(((struct sockaddr_in*)(ifi->ifi_addr))->sin_addr));
		
		inet_aton(addr,&adr_inet.sin_addr);
		CW_FREE_OBJECT(ifname);
		CW_FREE_OBJECT(addr);
		if(ifi->ifi_addr != NULL){
			free(ifi->ifi_addr);
			ifi->ifi_addr = NULL;
		}		
#ifdef	SIOCGIFBRDADDR
		if(ifi->ifi_brdaddr != NULL){
			free(ifi->ifi_brdaddr);
			ifi->ifi_brdaddr = NULL;
		}
#endif
		if(ifi){
			free(ifi);
			ifi = NULL;
		}
	}
	ip = adr_inet.sin_addr.s_addr;
	wid_syslog_info("iP_0x==%x\n",ip);
	state = AC_WTP[wtpid]->ntp_state;
	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, ap_ntp_length, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	CWProtocolStore8(msgPtr, value); //ap ntp  1
	CWProtocolStore8(msgPtr, Reserve); //ap statistics setting  1
	CWProtocolStore8(msgPtr, ipv4_v6);
	CWProtocolStore8(msgPtr, state); 
	CWProtocolStore32(msgPtr, interval); 
	if(ipv4_v6==0){
		CWProtocolStore32(msgPtr, ip); 
	}else{
		CWProtocolStoreRawBytes(msgPtr, (char*)list.s6_addr, 16);
	}
	CWProtocolStore8(msgPtr, Reserve); 
	CWProtocolStore8(msgPtr, Reserve); 
	CWProtocolStore8(msgPtr, Reserve); 
	CWProtocolStore8(msgPtr, Reserve); 
	wid_syslog_info("AC_WTP[%d]->ntp_interval=%d,ntp_state=%d.\n",wtpid,AC_WTP[wtpid]->ntp_interval,AC_WTP[wtpid]->ntp_state);
	return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE);

}

/*zhaoruijia,20100904,transplant ACTIMESYNCHROFAILURE from 1.2omc to 1.3,end*/

CWBool CWAssembleMsgElemAPThroughoutSet(CWProtocolMessage *msgPtr,int wtpid)
{
	wid_syslog_debug_debug(WID_WTPINFO,"#### CWAssembleMsgElemAPThroughoutSet ####\n");
	
	unsigned char value=2; //ap throughout settting 0
	//unsigned char packetlen = 2;
	unsigned char len = 0;
	unsigned char count = 0;
	int i = 0;
	if(AC_WTP[wtpid]->WTP_Radio[0] != NULL)
	{
		for(i=0; i<L_BSS_NUM; i++)
		{
			if(AC_WTP[wtpid]->WTP_Radio[0]->BSS[i] != NULL)
			{
				count++;
			}
		}
	}
	len = count*2+3;
	if(msgPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	wid_syslog_debug_debug(WID_WTPINFO,"len %d count %d \n",len,count);
	// create message
	//CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, packetlen, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, len, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	
	CWProtocolStore8(msgPtr, value); //ap throughout setting 2
	CWProtocolStore8(msgPtr, AC_WTP[wtpid]->WTP_Radio[0]->bandwidth);
	CWProtocolStore8(msgPtr, count);
	for(i=0; i<L_BSS_NUM; i++)
	{
		if(AC_WTP[wtpid]->WTP_Radio[0]->BSS[i] != NULL)
		{
			CWProtocolStore8(msgPtr, AC_WTP[wtpid]->WTP_Radio[0]->BSS[i]->WlanID);
			CWProtocolStore8(msgPtr, AC_WTP[wtpid]->WTP_Radio[0]->BSS[i]->band_width);
			wid_syslog_debug_debug(WID_WTPINFO,"bss[%d] throughput %d \n",i,AC_WTP[wtpid]->WTP_Radio[0]->BSS[i]->band_width);
		}
	}
		
	return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE);

}

CWBool CWAssembleMsgElemAPExtensinCommandSet(CWProtocolMessage *msgPtr,int wtpid,char *command)
{
	wid_syslog_debug_debug(WID_WTPINFO,"#### CWAssembleMsgElemAPExtensinCommandSet ####\n");
	//printf("command %s\n",command);
	unsigned char value=3; //ap throughout settting 0
	//unsigned char packetlen = 2 + strlen(AC_WTP[wtpid]->WTP_Radio[0]->excommand);
	unsigned short packetlen = 2 + strlen(command);
	if(msgPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);

	// create message
	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, packetlen, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	
	CWProtocolStore8(msgPtr, value); //ap sanning setting  0
	CWProtocolStore8(msgPtr, (packetlen-2));
	//CWProtocolStoreStr(msgPtr, AC_WTP[wtpid]->WTP_Radio[0]->excommand); 
	CWProtocolStoreStr(msgPtr, command); 
	return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE);

}
CWBool CWAssembleMsgElemAPOption60ParameterSet(CWProtocolMessage *msgPtr,int wtpid,char *parameter)
{
	wid_syslog_debug_debug(WID_WTPINFO,"#### CWAssembleMsgElemAPOption60ParameterSet ####\n");
	unsigned short value=28; 
	unsigned short packetlen = 4 + strlen(parameter);
	if(msgPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, packetlen, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	CWProtocolStore16(msgPtr, value); 
	CWProtocolStore16(msgPtr, (packetlen-4));
	CWProtocolStoreStr(msgPtr, parameter); 
	return CWAssembleMsgElemVendorOption60(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE);
}

//weichao add 2011.11.03
CWBool CWAssembleMsgElemAPFlowCheck(CWProtocolMessage *msgPtr,unsigned char radioid,unsigned char wlanid,unsigned short flow_check,unsigned int no_flow_time,unsigned int  limit_flow)
{
	
	char value=12; 
	const short int length = 16;
	char reserved = 0;
	if(msgPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);

	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, length, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	
	CWProtocolStore8(msgPtr, value); 
	CWProtocolStore16(msgPtr, (length-3));
	CWProtocolStore8(msgPtr, reserved); 
	CWProtocolStore8(msgPtr, radioid); 
	CWProtocolStore8(msgPtr, wlanid); 
	CWProtocolStore16(msgPtr, flow_check); 
	CWProtocolStore32(msgPtr, no_flow_time); 
	CWProtocolStore32(msgPtr, limit_flow); 
	
	wid_syslog_debug_debug(WID_WTPINFO,"value = %d\n",value);
	wid_syslog_debug_debug(WID_WTPINFO,"length = %d\n",(length-3));
	wid_syslog_debug_debug(WID_WTPINFO,"radioid = %d\n",radioid);
	wid_syslog_debug_debug(WID_WTPINFO,"wlanid = %d\n",wlanid);
	wid_syslog_debug_debug(WID_WTPINFO,"flow_check = %d\n",flow_check);
	wid_syslog_debug_debug(WID_WTPINFO,"no_flow_time = %d\n",no_flow_time);
	wid_syslog_debug_debug(WID_WTPINFO,"limit_flow = %d\n",limit_flow);
		
	return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE);

}
CWBool CWAssembleMsgElemAPPasswd(CWProtocolMessage *msgPtr,char *username,char*password)
{
	
	char  value=13; 
	char  length = 0;
	char name_length = 0;
	char passwd_length = 0;
	name_length = strlen(username);
	passwd_length =  strlen(password);
	length = 4+name_length+passwd_length;
	if(msgPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);

	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, length, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	
	CWProtocolStore8(msgPtr, value); 
	CWProtocolStore8(msgPtr, (length-4));
	CWProtocolStore8(msgPtr, name_length); 
	CWProtocolStoreStr(msgPtr,username);
	CWProtocolStore8(msgPtr, passwd_length); 
	CWProtocolStoreStr(msgPtr,password);
	
	wid_syslog_debug_debug(WID_DEFAULT,"value = %d\n",value);
	wid_syslog_debug_debug(WID_DEFAULT,"length = %d\n",(length-4));
	wid_syslog_debug_debug(WID_DEFAULT,"name_length = %d\n",name_length);
	wid_syslog_debug_debug(WID_DEFAULT,"username = %s\n",username);
	wid_syslog_debug_debug(WID_DEFAULT,"passwd_length = %d\n",passwd_length);
	wid_syslog_debug_debug(WID_DEFAULT,"password = %s\n",password);
		
	return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE);

}
CWBool CWAssembleMsgElemAPMultiUserOptimize(CWProtocolMessage *msgPtr,unsigned char wlanid,unsigned char radioid,unsigned char value)
{
	
	unsigned short   length = 13;
	unsigned short elementid = 14;
	unsigned char op_type = 5;
	unsigned char state = 0;
	if(msgPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);

	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, length, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	CWProtocolStore16(msgPtr, elementid);
	CWProtocolStore16(msgPtr, (length-8)); 
	CWProtocolStore8(msgPtr, op_type); 
	CWProtocolStore8(msgPtr, radioid); 
	CWProtocolStore8(msgPtr, wlanid); 
	CWProtocolStore8(msgPtr, state); 
	CWProtocolStore8(msgPtr,value);
	
	wid_syslog_debug_debug(WID_DEFAULT,"elementid = %d\n",elementid);
	wid_syslog_debug_debug(WID_DEFAULT,"length = %d\n",(length-8));
	wid_syslog_debug_debug(WID_DEFAULT,"op_type = %d\n",op_type);
	wid_syslog_debug_debug(WID_DEFAULT,"radioid = %d\n",radioid);
	wid_syslog_debug_debug(WID_DEFAULT,"wlanid = %d\n",wlanid);
	wid_syslog_debug_debug(WID_DEFAULT,"state = %d\n",state);
	wid_syslog_debug_debug(WID_DEFAULT,"value = %d\n",value);
		
	return CWAssembleMsgElemVendor(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE);

}
CWBool CWAssembleMsgElemAPnoRespToStaProReq(CWProtocolMessage *msgPtr,MQ_Radio radioinfo)
{
	
	short int elementid = 20; 
	short int length = 8;
	short int policy = 0;
	policy = (short int)radioinfo.id1;
	
	if(msgPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);

	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, length, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	
	CWProtocolStore16(msgPtr, elementid); 
	CWProtocolStore16(msgPtr, length-4);
	CWProtocolStore8(msgPtr, radioinfo.Radio_L_ID); 
	CWProtocolStore8(msgPtr, radioinfo.wlanid); 
	CWProtocolStore16(msgPtr, policy); 
	
	wid_syslog_debug_debug(WID_WTPINFO,"elementid = %d\n",elementid);
	wid_syslog_debug_debug(WID_WTPINFO,"radioid = %d\n",radioinfo.Radio_L_ID);
	wid_syslog_debug_debug(WID_WTPINFO,"wlanid = %d\n",radioinfo.wlanid);
	wid_syslog_debug_debug(WID_WTPINFO,"policy = %d\n",policy);
	wid_syslog_debug_debug(WID_WTPINFO,"length = %d\n",length);
		
	return CWAssembleMsgElemVendor(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE);

}

CWBool CWAssembleMsgElemAPUniMutiBroCastIsolationSWandRateSet(CWProtocolMessage *msgPtr,MQ_Radio radioinfo)
{
	
	short int elementid = 21; 
	short int length = 8;
//	char reserved = 0;
	unsigned char rate = 0;
	short int policy = 0;
	policy = (short int)radioinfo.id_char;
	
	rate = (radioinfo.id1/10)*2;
	if(msgPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);

	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, length, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	
	CWProtocolStore16(msgPtr, elementid); 
	CWProtocolStore16(msgPtr, (length-4));
	CWProtocolStore8(msgPtr, radioinfo.Radio_L_ID); 
	CWProtocolStore8(msgPtr, radioinfo.wlanid); 
	CWProtocolStore16(msgPtr, policy);
	wid_syslog_debug_debug(WID_WTPINFO,"elementid = %d\n",elementid);
	wid_syslog_debug_debug(WID_WTPINFO,"length = %d\n",length);
	wid_syslog_debug_debug(WID_WTPINFO,"radioid = %d\n",radioinfo.Radio_L_ID);
	wid_syslog_debug_debug(WID_WTPINFO,"wlanid = %d\n",radioinfo.wlanid);
	wid_syslog_debug_debug(WID_WTPINFO,"id_char=%d,policy = %d\n",radioinfo.id_char,policy);
	wid_syslog_debug_debug(WID_WTPINFO,"id1 = %d,rate=%d.\n",radioinfo.id1,rate);
		
	return CWAssembleMsgElemVendor(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE);

}

CWBool CWAssembleMsgElemAPUniMutiBroCastRateSet(CWProtocolMessage *msgPtr,MQ_Radio radioinfo)
{
	
	short int elementid = 22; 
	short int length = 12;
	//char reserved = 0;
	unsigned char rate = 0;
	
	rate = (radioinfo.id1*2)/10;
	if(msgPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);

	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, length, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	
	CWProtocolStore16(msgPtr, elementid); 
	CWProtocolStore16(msgPtr, (length-4));
	CWProtocolStore8(msgPtr, radioinfo.Radio_L_ID); 
	CWProtocolStore8(msgPtr, radioinfo.wlanid); 
	CWProtocolStore16(msgPtr, 0);
	CWProtocolStore8(msgPtr, rate); 
	CWProtocolStore8(msgPtr, 0); 
	CWProtocolStore8(msgPtr, 0); 
	CWProtocolStore8(msgPtr, 0); 

	wid_syslog_debug_debug(WID_WTPINFO,"elementid = %d\n",elementid);
	wid_syslog_debug_debug(WID_WTPINFO,"length = %d\n",length);
	wid_syslog_debug_debug(WID_WTPINFO,"radioid = %d\n",radioinfo.Radio_L_ID);
	wid_syslog_debug_debug(WID_WTPINFO,"wlanid = %d\n",radioinfo.wlanid);
	wid_syslog_debug_debug(WID_WTPINFO,"id1 = %d,rate=%d.\n",radioinfo.id1,rate);
		
	return CWAssembleMsgElemVendor(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE);

}
CWBool  CWAssembleStaticAPIP(CWProtocolMessage *msgPtr,int wtpid)
{
	wid_syslog_debug_debug(WID_WTPINFO,"#### CWAssembleStaticAPIP ####\n");
	
	unsigned char valuelen=13; //
	if(msgPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);

	// create message
	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, valuelen, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	
	CWProtocolStore32(msgPtr, AC_WTP[wtpid]->ap_ipadd);
	CWProtocolStore32(msgPtr, AC_WTP[wtpid]->ap_mask);
	CWProtocolStore32(msgPtr, AC_WTP[wtpid]->ap_gateway); 
	CWProtocolStore8(msgPtr, 1); 
	
	return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_WTP_STATIC_IP_CW_TYPE);

}
//fengwenchao add 20110126 for XJDEV-32  from 2.0
CWBool  CWAssembleWtpEthMtu(CWProtocolMessage *msgPtr,int wtpid,unsigned char eth_index)
{
	wid_syslog_debug_debug(WID_DEFAULT,"#### CWAssembleWtpEthMtu ####\n");
	unsigned char valuelen = 10; 
	unsigned int reseved = 0;
	unsigned char value = 1;
	unsigned int mtu = 1500;
	if(msgPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);

	if(eth_index < AP_ETH_IF_NUM){
		mtu = AC_WTP[wtpid]->apifinfo.eth[eth_index].eth_mtu;
	}
	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, valuelen, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	
	CWProtocolStore8(msgPtr, value);
	CWProtocolStore8(msgPtr, eth_index);
	CWProtocolStore32(msgPtr, mtu);
	CWProtocolStore32(msgPtr, reseved);

	printf("value %d eth_index %d mtu %d\n",value,eth_index,mtu);
	printf("value %d eth0 mtu %d eth1 mtu %d\n",value,AC_WTP[wtpid]->apifinfo.eth[0].eth_mtu,AC_WTP[wtpid]->apifinfo.eth[1].eth_mtu);

	return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_ETH_SET_CW_TYPE);

}
//fengnwenchao add end
CWBool  CWAssembleTimestamp(CWProtocolMessage *msgPtr,int wtpid)
{
	wid_syslog_debug_debug(WID_WTPINFO,"#### CWAssembleTimestamp ####\n");
	
	unsigned char valuelen=4; //
	int timestamp = 0;
	if(msgPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);

	// create message
	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, valuelen, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	
	CWProtocolStore32(msgPtr, timestamp);
	
	return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_TIMESTAMP_CW_TYPE);

}
CWBool  CWAssemblewtpextensioninfomation(CWProtocolMessage *msgPtr,int wtpid)
{
	wid_syslog_debug_debug(WID_WTPINFO,"#### CWAssemblewtpextensioninfomation ####\n");
	
	unsigned char valuelen = 4; //
	unsigned char value = 4;
	unsigned char flag = AC_WTP[wtpid]->wifi_extension_reportswitch;
	unsigned short interval = AC_WTP[wtpid]->wifi_extension_reportinterval;
	if(msgPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);

	// create message
	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, valuelen, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	
	CWProtocolStore8(msgPtr, value);
	CWProtocolStore8(msgPtr, flag);
	CWProtocolStore16(msgPtr, interval);
	return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE);

}
CWBool  CWAssemblewtpstainfomationreport(CWProtocolMessage *msgPtr,int wtpid)
{
	wid_syslog_debug_debug(WID_WTPINFO,"#### CWAssemblewtpstainfomationreport ####\n");
	
	unsigned char valuelen = 4; 
	unsigned char value = 6;
	unsigned char flag = AC_WTP[wtpid]->ap_sta_report_switch;
	unsigned short interval = AC_WTP[wtpid]->ap_sta_report_interval;
	if(msgPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);

	// create message
	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, valuelen, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	
	CWProtocolStore8(msgPtr, value);
	CWProtocolStore8(msgPtr, flag);
	CWProtocolStore16(msgPtr, interval);
	return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE);

}
CWBool  CWAssembleWtpStaWapiInfoReport(CWProtocolMessage *msgPtr,int wtpid)
{
	wid_syslog_debug_debug(WID_DEFAULT,"#### CWAssembleWtpStaWapiInfoReport ####\n");
	
	unsigned char valuelen = 4; 
	unsigned char value = 9;
	unsigned char flag = AC_WTP[wtpid]->ap_sta_wapi_report_switch;
	unsigned short interval = 0;
	interval  = (unsigned short)AC_WTP[wtpid]->ap_sta_wapi_report_interval;
	if(msgPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);

	// create message
	printf("value %d flag %d interval %d\n",value,flag,interval);
	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, valuelen, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	
	CWProtocolStore8(msgPtr, value);
	CWProtocolStore8(msgPtr, flag);
	CWProtocolStore16(msgPtr, interval);
	return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE);

}

CWBool  CWAssemblewtpifinforeport(CWProtocolMessage *msgPtr,int wtpid)
{
	wid_syslog_debug_debug(WID_WTPINFO,"#### CWAssemblewtpifinforeport ####\n");
	
	unsigned char valuelen = 4; 
	unsigned char value = 7;
	unsigned char flag = AC_WTP[wtpid]->apifinfo.report_switch;
	unsigned short interval = AC_WTP[wtpid]->apifinfo.report_interval;
	if(msgPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);

	// create message
	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, valuelen, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	
	CWProtocolStore8(msgPtr, value);
	CWProtocolStore8(msgPtr, flag);
	CWProtocolStore16(msgPtr, interval);
	return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE);

}

CWBool  CWAssembleWidsSet(CWProtocolMessage *msgPtr,int wtpid)
{
	wid_syslog_debug_debug(WID_WTPINFO,"#### CWAssembleWidsSet ####\n");
	
	unsigned char valuelen = 7; 
	unsigned char value = 8;
	unsigned char flooding = gwids.flooding;
	unsigned char sproof = gwids.sproof;
	unsigned char weakiv = gwids.weakiv;
	unsigned char probethreshold = gprobethreshold;
	unsigned char otherthreshold = gotherthreshold;
	unsigned char interval = gwidsinterval;
	
	if(msgPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);

	// create message
	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, valuelen, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	
	CWProtocolStore8(msgPtr, value);
	CWProtocolStore8(msgPtr, flooding);
	CWProtocolStore8(msgPtr, sproof);
	CWProtocolStore8(msgPtr, weakiv);

	CWProtocolStore8(msgPtr, otherthreshold);
	CWProtocolStore8(msgPtr, probethreshold);
	CWProtocolStore8(msgPtr, interval);
	return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE);

}

CWBool CWAssembleMsgElemAPInterfaceInfo(CWProtocolMessage *msgPtr)
{
	wid_syslog_debug_debug(WID_WTPINFO,"#### CWAssembleMsgElemAPInterfaceInfo ####\n");
	
	int value=5; //ap APInterfaceInfo
	int value_length = 3;//
	if(msgPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);

	// create message
	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, value_length, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	
	CWProtocolStore8(msgPtr, value); //ap APInterfaceInfo
	CWProtocolStore8(msgPtr, WID_SAMPLE_INFORMATION.sample_switch);
	CWProtocolStore8(msgPtr, WID_SAMPLE_INFORMATION.sample_time);
		
	return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE);

}

CWBool CWAssembleMsgElemRadioOperationalState(int radioID, CWProtocolMessage *msgPtr) 
{
	const int radio_Operational_State_Length=3;
	CWRadiosOperationalInfo infos;
	memset(&infos,0,sizeof(CWRadiosOperationalInfo));
	CWProtocolMessage *msgs=NULL;
	int len = 0;
	int i;
	int j;
	if(msgPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
	
	if(!(CWGetWTPRadiosOperationalState(radioID, &infos))) {
		return CW_FALSE;
	}
	
	CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgs, (infos.radiosCount), return  CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	
	for(i = 0; i < infos.radiosCount; i++) {
		// create message
		CW_CREATE_PROTOCOL_MESSAGE(msgs[i], radio_Operational_State_Length, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
		CWProtocolStore8(&(msgs[i]), infos.radios[i].ID); // ID of the radio
		CWProtocolStore8(&(msgs[i]), infos.radios[i].state); // state of the radio
		CWProtocolStore8(&(msgs[i]), infos.radios[i].cause);
		
		if(!(CWAssembleMsgElem(&(msgs[i]), CW_MSG_ELEMENT_RADIO_OPERAT_STATE_CW_TYPE))) {
			for(j = i; j >= 0; j--) { CW_FREE_PROTOCOL_MESSAGE(msgs[j]);}
			CW_FREE_OBJECT(infos.radios);
			CW_FREE_OBJECT(msgs);
			return CW_FALSE;
		}
		
		len += msgs[i].offset;
//		wid_syslog_debug_debug("Radio operational State: %d - %d - %d", infos.radios[i].ID, infos.radios[i].state, infos.radios[i].cause);
	}
	
	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, len, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	
	for(i = 0; i < infos.radiosCount; i++) {
		CWProtocolStoreMessage(msgPtr, &(msgs[i]));
		CW_FREE_PROTOCOL_MESSAGE(msgs[i]);
	}
	
	CW_FREE_OBJECT(msgs);
	CW_FREE_OBJECT(infos.radios);

	return CW_TRUE;
}


/*_________________________________________________________________________*/
/*  *****************************___PARSE___*****************************  */
CWBool CWParseACNameWithIndex (CWProtocolMessage *msgPtr, int len, CWACNameWithIndexValues *valPtr)
{
	CWParseMessageElementStart();

	valPtr->index = CWProtocolRetrieve8(msgPtr);
	//wid_syslog_debug_debug("CW_MSG_ELEMENT_WTP_RADIO_ID: %d",	(valPtr->radios)[radioIndex].ID);
	
	valPtr->ACName = CWProtocolRetrieveStr(msgPtr,len-1);
	//wid_syslog_debug_debug("CW_MSG_ELEMENT_WTP_RADIO_TYPE: %d",	(valPtr->radios)[radioIndex].type);
	
	//wid_syslog_debug_debug("AC Name with index: %d - %s", valPtr->index, valPtr->ACName);
	
	CWParseMessageElementEnd();
}

CWBool CWParseDiscoveryType(CWProtocolMessage *msgPtr, int len, CWDiscoveryRequestValues *valPtr) {	
	CWParseMessageElementStart();
										
	valPtr->type = CWProtocolRetrieve8(msgPtr);
	//wid_syslog_debug_debug("CW_MSG_ELEMENT_DISCOVERY_TYPE: %d",	valPtr->type);
	
	CWParseMessageElementEnd();
}

CWBool CWParseLocationData(CWProtocolMessage *msgPtr, int len, char **valPtr) {	
	CWParseMessageElementStart();
	
	*valPtr = CWProtocolRetrieveStr(msgPtr, len);
	if(valPtr == NULL) return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
	//wid_syslog_debug_debug("Location Data:%s", *valPtr);
	
	CWParseMessageElementEnd();
}

CWBool CWParseMsgElemDuplicateIPv4Address(CWProtocolMessage *msgPtr, int len, WTPDuplicateIPv4 *valPtr) 
{
	CWParseMessageElementStart();
	
	valPtr->ipv4Address =  CWProtocolRetrieve32(msgPtr);
	valPtr->status = CWProtocolRetrieve8(msgPtr);
	valPtr->length = CWProtocolRetrieve8(msgPtr);
	valPtr->MACoffendingDevice_forIpv4 = (unsigned char*)CWProtocolRetrieveRawBytes(msgPtr, valPtr->length);
	
	//valPtr->MACoffendingDevice_forIpv4 = (unsigned char*)CWProtocolRetrieveRawBytes(msgPtr,6);
	//valPtr->status = CWProtocolRetrieve8(msgPtr);
	//wid_syslog_debug_debug("Duplicate IPv4: %d", valPtr->ipv4Address);

	CWParseMessageElementEnd();
}

CWBool CWParseMsgElemDuplicateIPv6Address(CWProtocolMessage *msgPtr, int len, WTPDuplicateIPv6 *valPtr) 
{
	CWParseMessageElementStart();
	
	int i;
	for(i=0; i<16; i++)
	{	char *aux;
		aux= CWProtocolRetrieveRawBytes(msgPtr, 1);
		(valPtr->ipv6Address).s6_addr[i] = *aux;
	}

	//wid_syslog_debug_debug("Duplicate IPv6");
	//valPtr->MACoffendingDevice_forIpv6 = (unsigned char*)CWProtocolRetrieveRawBytes(msgPtr,6);

	valPtr->status = CWProtocolRetrieve8(msgPtr);

	valPtr->length = CWProtocolRetrieve8(msgPtr);

	valPtr->MACoffendingDevice_forIpv6 = (unsigned char*)CWProtocolRetrieveRawBytes(msgPtr, valPtr->length);

	CWParseMessageElementEnd();
}

//added by weiay 20080702
CWBool CWParseMsgElemCWStationInfoValue(CWProtocolMessage *msgPtr, int len, CWStationInfoValues *valPtr)
{
	//printf("*** CWParseMsgElemCWStationInfoValue ***\n");
	//printf("*** len:%d ***\n",len);
	
	CWParseMessageElementStart();
	valPtr->radio_id = CWProtocolRetrieve8(msgPtr);
	valPtr->mac_length = CWProtocolRetrieve8(msgPtr);
	if(valPtr->mac_length<6||valPtr->mac_length>64){
		wid_syslog_err("%s mac_length==%d\n",__func__,valPtr->mac_length);
	}else{
		//printf("*** len:%d ***\n",(int)valPtr->mac_length);
		valPtr->mac_addr =  CWProtocolRetrieveStr(msgPtr,valPtr->mac_length);
	}
	//printf("*** radio id:%d mac length:%d mac value:%s ***\n",valPtr->radio_id,valPtr->mac_length,valPtr->mac_addr);
	//printf("mac value = ");
	//int i;
	//for (i = 0; i < 6; i++)
	//{
	//	printf("%02x", valPtr->mac_addr[i]);
	//}
	//printf("\n");
	
	CWParseMessageElementEnd();	

}

CWBool CWParseMsgElemCWWtpStaIpMacReportInfo(CWProtocolMessage *msgPtr, int len, CWStationReportInfo *valPtr)
{
	//printf("*** CWParseMsgElemCWStationInfoValue ***\n");
	//printf("*** len:%d ***\n",len);
	int i = 0;
	unsigned char *ip = NULL;
	CWParseMessageElementStart();
	
	valPtr->op = 0;
	valPtr->radioId = 0;
	valPtr->wlanId = 0;
	valPtr->vlanId = 0;
	valPtr->length = 0;
	valPtr->ipv4Address = 0;
	
	valPtr->op=   CWProtocolRetrieve8(msgPtr);
	printf("##staIpMac##op :%d\n",valPtr->op);

	valPtr->radioId= CWProtocolRetrieve8(msgPtr);
	printf("##staIpMac##radioId :%d\n",valPtr->radioId);
	//valPtr->mac_length = CWProtocolRetrieve8(msgPtr);

	valPtr->wlanId= CWProtocolRetrieve8(msgPtr);
	printf("##staIpMac##wlanId :%d\n",valPtr->wlanId);
	valPtr->vlanId= CWProtocolRetrieve32(msgPtr);
	printf("##staIpMac##vlanId :%d\n",valPtr->vlanId);

	unsigned char* mac = (unsigned char*)CWProtocolRetrieveStr(msgPtr,6);
	printf("##staIpMac##mac :%s\n",mac);
	
	memcpy(valPtr->mac, mac, 6);
	free(mac);
	mac = NULL;

	//valPtr->mac =  CWProtocolRetrieveStr(msgPtr,valPtr->mac_length);
	valPtr->length = CWProtocolRetrieve8(msgPtr);
	printf("##staIpMac##lenth :%d\n",valPtr->length);

	if(valPtr->length == 16){

		for(i=0; i<16; i++)
		{	char *aux;
			aux= CWProtocolRetrieveRawBytes(msgPtr, 1);
			(valPtr->ipv6Address).s6_addr[i] = *aux;
			if(aux){
				free(aux);
				aux = NULL;
			}
			printf("##staIpMac##ipv6Address :%d",(valPtr->ipv6Address).s6_addr[i]);
		}
		printf("\n");
	}else {
		valPtr->ipv4Address =  CWProtocolRetrieve32(msgPtr);
		ip = (unsigned char *)&valPtr->ipv4Address;
		printf("##staIpMac##ipv4Address :%d.%d.%d.%d\n",ip[0],ip[1],ip[2],ip[3]);
	}
	//printf("*** radio id:%d mac length:%d mac value:%s ***\n",valPtr->radio_id,valPtr->mac_length,valPtr->mac_addr);
	//printf("mac value = ");
	//int i;
	//for (i = 0; i < 6; i++)
	//{
	//	printf("%02x", valPtr->mac_addr[i]);
	//}
	//printf("\n");
	
	CWParseMessageElementEnd();	

}
CWBool CWParseMsgElemAPNeighborAPInfos(CWProtocolMessage *msgPtr, int len, Neighbor_AP_INFOS *valPtr)
{
	wid_syslog_debug_debug(WID_WTPINFO,"*** CWParseMsgElemAPNeighborAPInfos ***\n");
	wid_syslog_debug_debug(WID_WTPINFO,"*** len:%d ***\n",len);
	
	CWParseMessageElementStart();
	int i = 0;
	int essidlen = 0;
	int ielen = 0;
	char *ptr = NULL;
	char *essid = NULL;
	unsigned int count_n = 0;
	unsigned int oldOffset2 = 0;
	oldOffset2 = msgPtr->offset;
	/*
	value = CWProtocolRetrieve8(msgPtr);//AP_SCANNING_INFO = 1
	printf("*** value:%d ***\n",value);

	if(value != 1)
	{
		valPtr->neighborapInfosCount = 0;
		valPtr->neighborapInfos = NULL;
		return CW_FALSE;
	}
	*/
	valPtr->DeviceInterference =0;
	valPtr->DeviceInterference = CWProtocolRetrieve8(msgPtr);	//wtpDeviceInterferenceDetectedTrap
	valPtr->neighborapInfosCount = CWProtocolRetrieve8(msgPtr);
	valPtr->neighborapInfos = NULL;
	struct Neighbor_AP_ELE *neighborapelem = NULL;
	struct Neighbor_AP_ELE *phead = NULL;

	for(i=0; i<valPtr->neighborapInfosCount; i++)
	{
		CW_CREATE_OBJECT_SIZE_ERR(neighborapelem, sizeof(struct Neighbor_AP_ELE), return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););		
		memset(neighborapelem, 0, sizeof(struct Neighbor_AP_ELE));		
		char * str = CWProtocolRetrieveStr(msgPtr,6);
		memcpy(neighborapelem->BSSID ,str, 6);
		free(str);
		str = NULL;
		
		neighborapelem->Rate = CWProtocolRetrieve16(msgPtr);

		neighborapelem->Channel = CWProtocolRetrieve8(msgPtr);
		neighborapelem->RSSI = CWProtocolRetrieve8(msgPtr);
		neighborapelem->NOISE = CWProtocolRetrieve8(msgPtr);
		neighborapelem->BEACON_INT = CWProtocolRetrieve8(msgPtr);
		
		neighborapelem->status = 1;
		neighborapelem->opstatus = CWProtocolRetrieve8(msgPtr);
		neighborapelem->capabilityinfo = CWProtocolRetrieve16(msgPtr);
		neighborapelem->wtpid = 0;

		essidlen = CWProtocolRetrieve8(msgPtr);
		if((essidlen<0) || (essidlen>32)){    //fengwenchao change "&&" to "||" for autelan-3251 20121129
			wid_syslog_err("%s essidlen==%d\n",__func__,essidlen);
			essidlen = 32;
			valPtr->neighborapInfosCount = count_n;
			wid_syslog_warning("<warning>%s line==%d,count_n =%d.\n",__func__,__LINE__,count_n);
			CW_FREE_OBJECT(neighborapelem);			//fengwenchao add for autelan-3251 20121129
			oldOffset2 = msgPtr->offset;
			msgPtr->offset += len - oldOffset2;
			return CW_TRUE;
		}else{
		}
		//neighborapelem->ESSID = CWProtocolRetrieveStr(msgPtr,essidlen);
		essid = CWProtocolRetrieveStr(msgPtr,essidlen);
		if(check_ascii_32_to126(essid) == CW_FALSE)
		{
			CW_FREE_OBJECT(essid);
			//CW_CREATE_OBJECT_SIZE_ERR(neighborapelem->ESSID, ESSID_DEFAULT_LEN, return CW_FALSE;);
			memset(neighborapelem->ESSID, 0, 5);
			memcpy(neighborapelem->ESSID, "none", 4);
			//neighborapelem->ESSID[0] = '\0';
		}
		else
		{
			memset(neighborapelem->ESSID, 0, strlen(essid)+1);
			memcpy(neighborapelem->ESSID,  essid, strlen(essid));
			CW_FREE_OBJECT(essid);
		}
		
		ielen = CWProtocolRetrieve8(msgPtr);
		if((ielen<0)||(ielen > 64)){
			ielen = 64;
			wid_syslog_err("%s ielen==%d\n",__func__,ielen);
			valPtr->neighborapInfosCount = count_n;
			wid_syslog_warning("<warning>%s line==%d,count_n =%d.\n",__func__,__LINE__,count_n);
			CW_FREE_OBJECT(neighborapelem);		//fengwenchao add for autelan-3251 20121129	
			oldOffset2 = msgPtr->offset;
			msgPtr->offset += len - oldOffset2;
			return CW_TRUE;
		}else{

		}
		neighborapelem->IEs_INFO = CWProtocolRetrieveStr(msgPtr,ielen);
		if(check_ascii_32_to126(neighborapelem->IEs_INFO) == CW_FALSE)
		{
			CW_FREE_OBJECT(neighborapelem->IEs_INFO);
			CW_CREATE_OBJECT_SIZE_ERR(neighborapelem->IEs_INFO, 1, return CW_FALSE;);
			neighborapelem->IEs_INFO[0] = '\0';
		}

		time(&neighborapelem->fst_dtc_tm);

		ptr = strchr(neighborapelem->IEs_INFO, 'P'); 

		if(ptr)
		{
			neighborapelem->encrp_type = 1;
			ptr = strstr(neighborapelem->IEs_INFO, "WPA"); 
			if(ptr)
			{
				neighborapelem->encrp_type = 2;
			}
			ptr = strstr(neighborapelem->IEs_INFO, "RSN"); 
			if(ptr)
			{
				neighborapelem->encrp_type = 3;
			}
		}
		else
		{
			neighborapelem->encrp_type = 0;
		}
		ptr = NULL;
		
		neighborapelem->polcy = 0;
		
		neighborapelem->next = NULL;

		if(valPtr->neighborapInfos == NULL)
		{
			//printf("parse first ap info\n");
			valPtr->neighborapInfos = neighborapelem;
			phead = neighborapelem;
			neighborapelem = NULL;
		}
		else
		{
			//printf("parse more ap info\n");
			phead->next = neighborapelem;
			phead = neighborapelem;
			neighborapelem = NULL;
		}
		count_n ++;
		if(count_n >= 20){/*msg buff is 2048,so we can only save 2048/sizeof(struct Neighbor_AP_ELE) = 25,here we only save firt 20 neighbor ap*/
			valPtr->neighborapInfosCount = count_n;
			wid_syslog_warning("<warning>%s,%d,neighbo ap count_n access to %d,we will save first %d only.\n",__func__,__LINE__,count_n,count_n);
			oldOffset2 = msgPtr->offset;
			msgPtr->offset += len - oldOffset2;
			return CW_TRUE;
//			break;
		}
	}	

	CWParseMessageElementEnd();	

}

CWBool CWParseMsgElemAPWidsInfos(CWProtocolMessage *msgPtr, int len, wid_wids_device *valPtr,int wtpindex)
{
	wid_syslog_debug_debug(WID_WTPINFO,"*** CWParseMsgElemAPWidsInfos ***\n");
	wid_syslog_debug_debug(WID_WTPINFO,"*** len:%d ***\n",len);
	
	printf("*** CWParseMsgElemAPWidsInfos ***\n");
	printf("*** len:%d ***\n",len);
	
	CWParseMessageElementStart();
	int i = 0;
	unsigned char * str = NULL;

	valPtr->count = CWProtocolRetrieve8(msgPtr);
	
	valPtr->wids_device_info = NULL;
	
	struct tag_wids_device_ele *wids_device_ele = NULL;
	struct tag_wids_device_ele *phead = NULL;

	for(i=0; i<valPtr->count; i++)
	{
		CW_CREATE_OBJECT_SIZE_ERR(wids_device_ele, sizeof(struct tag_wids_device_ele), return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););		


		wids_device_ele->attacktype = CWProtocolRetrieve8(msgPtr);

		//printf("03333 %d\n",wids_device_ele->attacktype);
		if(wids_device_ele->attacktype == 1)
		{
			AC_WTP[wtpindex]->wids_statist.floodingcount++;
		}
		if(wids_device_ele->attacktype == 2)
		{
			AC_WTP[wtpindex]->wids_statist.sproofcount++;
		}
		if(wids_device_ele->attacktype == 3)
		{
			AC_WTP[wtpindex]->wids_statist.weakivcount++;
		}
		else
		{
			wid_syslog_debug_debug(WID_WTPINFO,"*** CWParseMsgElemAPWidsInfos  format error***\n");	
		}
		//printf("03333 %d %d %d\n",AC_WTP[wtpindex]->wids_statist.floodingcount,AC_WTP[wtpindex]->wids_statist.sproofcount,AC_WTP[wtpindex]->wids_statist.weakivcount);
		str = (unsigned char*)CWProtocolRetrieveStr(msgPtr,6);
		memcpy(wids_device_ele->bssid,str, 6);
		free(str);
		str = NULL;

		str = (unsigned char*)CWProtocolRetrieveStr(msgPtr,6);
		memcpy(wids_device_ele->vapbssid,str, 6);
		free(str);
		str = NULL;
						
		wids_device_ele->frametype = CWProtocolRetrieve8(msgPtr);
		wids_device_ele->channel = CWProtocolRetrieve8(msgPtr);
		wids_device_ele->rssi = CWProtocolRetrieve8(msgPtr);


		time(&wids_device_ele->fst_attack);
		time(&wids_device_ele->lst_attack);
		wids_device_ele->attackcount = 1;
		
		wids_device_ele->next = NULL;

		wid_asd_send_wids_info(wids_device_ele,wtpindex);
		
		if(valPtr->wids_device_info== NULL)
		{
			//printf("parse first ap info\n");
			valPtr->wids_device_info = wids_device_ele;
			phead = wids_device_ele;
			wids_device_ele = NULL;
		}
		else
		{
			//printf("parse more ap info\n");
			phead->next = wids_device_ele;
			phead = wids_device_ele;
			wids_device_ele = NULL;
		}
		
	}	

	CWParseMessageElementEnd();	

}


CWBool CWParseMsgElemAPInterfaceInfo(CWProtocolMessage *msgPtr, int len, wid_sample_rate_info *valPtr)
{
	wid_syslog_debug_debug(WID_WTPINFO,"*** CWParseMsgElemAPInterfaceInfo ***\n");
	wid_syslog_debug_debug(WID_WTPINFO,"*** len:%d ***\n",len);
	
	CWParseMessageElementStart();
	
	valPtr->current_uplink_throughput = CWProtocolRetrieve32(msgPtr);	
	valPtr->current_downlink_throughput = CWProtocolRetrieve32(msgPtr);

	CWParseMessageElementEnd(); 

}
CWBool CWParseMsgElemAPStaInfoReport(CWProtocolMessage *msgPtr, int len, WIDStationInfo *valPtr,int wtpindex)
{
	wid_syslog_debug_debug(WID_WTPINFO,"*** CWParseMsgElemAPStaInfoReport ***\n");
	wid_syslog_debug_debug(WID_WTPINFO,"*** len:%d ***\n",len);
	int i = 0;
	unsigned int count = 0;
	unsigned int count1 = 0;
	WIDStationInfo valPtr1[64];
	int needoffset = 0;
	CWParseMessageElementStart();
	count = CWProtocolRetrieve8(msgPtr);
	count1 = count;
	memset(valPtr1,0, 64*sizeof(WIDStationInfo));
	//printf("count %d\n",count);
	if((count*15 != len-1)&&(count*23 != len-1)&&(count*65 != len-1) && (count * 69) != len - 1){
		wid_syslog_info("__func__ something wrong count %d len %d\n",count,len);
		needoffset = len-1;
		count1 = 0;
	}
	if(count > 64){
		needoffset = len - 1 - (len-1)/count*64;
		count1 = 64;
		wid_syslog_info("__func__ count %d large than 64 needoffset %d\n",count,needoffset);
	}
	for(i=0;i<count1;i++)
	{
		valPtr1[i].radioId = CWProtocolRetrieve8(msgPtr);	
		valPtr1[i].wlanId = CWProtocolRetrieve8(msgPtr);
		
		unsigned char* mac = (unsigned char*)CWProtocolRetrieveStr(msgPtr,6);
		memcpy(valPtr1[i].mac, mac, 6);
		free(mac);
		mac = NULL;
			
		//for(i=0;i<6;i++)
		//{
		//	valPtr->mac[i] = CWProtocolRetrieve8(msgPtr);
		//}
		
		valPtr1[i].mode = CWProtocolRetrieve8(msgPtr);
		valPtr1[i].channel = CWProtocolRetrieve8(msgPtr);
		valPtr1[i].rssi = CWProtocolRetrieve8(msgPtr);
		valPtr1[i].tx_Rate = CWProtocolRetrieve16(msgPtr);
		valPtr1[i].isPowerSave = CWProtocolRetrieve8(msgPtr);
		valPtr1[i].isQos = CWProtocolRetrieve8(msgPtr);

		wid_syslog_debug_debug(WID_DEFAULT,"valPtr->radioId:%d ***\n",valPtr1[i].radioId);
		wid_syslog_debug_debug(WID_DEFAULT,"valPtr->wlanId:%d ***\n",valPtr1[i].wlanId);
		wid_syslog_debug_debug(WID_DEFAULT,"valPtr->mac:%s ***\n",valPtr1[i].mac);
		wid_syslog_debug_debug(WID_DEFAULT,"valPtr->mode:%d ***\n",valPtr1[i].mode);
		wid_syslog_debug_debug(WID_DEFAULT,"valPtr->channel:%d ***\n",valPtr1[i].channel);
		wid_syslog_debug_debug(WID_DEFAULT,"valPtr->rssi:%d ***\n",valPtr1[i].rssi);
		wid_syslog_debug_debug(WID_DEFAULT,"valPtr->tx_Rate:%d ***\n",valPtr1[i].tx_Rate);
		wid_syslog_debug_debug(WID_DEFAULT,"valPtr->isPowerSave:%d ***\n",valPtr1[i].isPowerSave);
		wid_syslog_debug_debug(WID_DEFAULT,"valPtr->isQos:%d ***\n",valPtr1[i].isQos);
		
		if((len-1)/count > 15){
			valPtr1[i].rx_bytes = CWProtocolRetrieve32(msgPtr);
			valPtr1[i].tx_bytes = CWProtocolRetrieve32(msgPtr);
		}
		/* zhangshu append 2010-08-28 */
        if((len-1)/count > 23){
            CWProtocolRetrieve64(msgPtr, &valPtr1[i].rx_data_bytes);
            //wid_syslog_debug_debug(WID_DEFAULT,"rx_data_bytes %llu\n",valPtr->rx_data_bytes);

			CWProtocolRetrieve64(msgPtr, &valPtr1[i].tx_data_bytes);
			//wid_syslog_debug_debug(WID_DEFAULT,"tx_data_bytes %llu\n",valPtr->tx_data_bytes);

			valPtr1[i].rx_data_frames = CWProtocolRetrieve32(msgPtr);
			//wid_syslog_debug_debug(WID_DEFAULT,"rx_data_frames %d\n",valPtr->rx_data_frames);

			valPtr1[i].tx_data_frames = CWProtocolRetrieve32(msgPtr);
			//wid_syslog_debug_debug(WID_DEFAULT,"tx_data_frames %d M\n",valPtr->tx_data_frames);

			valPtr1[i].rx_frames = CWProtocolRetrieve32(msgPtr);
			//wid_syslog_debug_debug(WID_DEFAULT,"rx_frames %d\n",valPtr->rx_frames);

			valPtr1[i].tx_frames = CWProtocolRetrieve32(msgPtr);
			//wid_syslog_debug_debug(WID_DEFAULT,"tx_frames %d\n",valPtr->tx_frames);

			valPtr1[i].rx_frag_packets = CWProtocolRetrieve32(msgPtr);
			//wid_syslog_debug_debug(WID_DEFAULT,"rx_frag_packets %d \n",valPtr->rx_frag_packets);
			
			valPtr1[i].tx_frag_packets = CWProtocolRetrieve32(msgPtr);
			//wid_syslog_debug_debug(WID_DEFAULT,"tx_frag_packets %d\n",valPtr->tx_frag_packets);

			valPtr1[i].rx_Rate = CWProtocolRetrieve16(msgPtr);
			//wid_syslog_debug_debug(WID_DEFAULT,"rx_Rate %d\n",valPtr->rx_Rate);
			
			wid_syslog_debug_debug(WID_DEFAULT,"valPtr->rx_data_bytes:%llu ***\n",valPtr1[i].rx_data_bytes);
			wid_syslog_debug_debug(WID_DEFAULT,"valPtr->tx_data_bytes:%llu ***\n",valPtr1[i].tx_data_bytes);
			wid_syslog_debug_debug(WID_DEFAULT,"valPtr->rx_data_frames:%u ***\n",valPtr1[i].rx_data_frames);
			wid_syslog_debug_debug(WID_DEFAULT,"valPtr->tx_data_frames:%u ***\n",valPtr1[i].tx_data_frames);
			wid_syslog_debug_debug(WID_DEFAULT,"valPtr->rx_frames:%u ***\n",valPtr1[i].rx_frames);
			wid_syslog_debug_debug(WID_DEFAULT,"valPtr->tx_frames:%u ***\n",valPtr1[i].tx_frames);
			wid_syslog_debug_debug(WID_DEFAULT,"valPtr->rx_frag_packets:%u ***\n",valPtr1[i].rx_frag_packets);
			wid_syslog_debug_debug(WID_DEFAULT,"valPtr->tx_frag_packets:%u ***\n",valPtr1[i].tx_frag_packets);
			wid_syslog_debug_debug(WID_DEFAULT,"valPtr->rx_Rate:%u ***\n",valPtr1[i].rx_Rate);
        }
		if ((len -1) / count > (23 + 42))
		{
			valPtr1[i].MAXofRateset = CWProtocolRetrieve32(msgPtr);
			wid_syslog_debug_debug(WID_DEFAULT,"valPtr->MAXofRateset:%u ***\n",valPtr1[i].MAXofRateset);
		}
		/*
		printf("radioId %d\n",valPtr->radioId);
		printf("wlanId %d\n",valPtr->wlanId);
		printf("mac %02X:%02X:%02X:%02X:%02X:%02X\n",valPtr->mac[0],valPtr->mac[1],valPtr->mac[2],valPtr->mac[3],valPtr->mac[4],valPtr->mac[5]);
		printf("mode %X\n",valPtr->mode);
		printf("channel %d\n",valPtr->channel);
		printf("rssi %d\n",valPtr->rssi);
		printf("nrate %d M\n",valPtr->nRate);
		printf("isPowerSave %d\n",valPtr->isPowerSave);
		printf("isQos %d\n",valPtr->isQos);
		*/
	}
	if(count1 != 0)
		WidAsd_StationInfoUpdate(wtpindex,count1,valPtr1);
	if(needoffset != 0){		
		(msgPtr->offset) += needoffset;
	}
	CWParseMessageElementEnd(); 

}
CWBool CWParseMsgElemAPIfInfoReport(CWProtocolMessage *msgPtr, int len, wid_ap_if_state_time *valPtr,int wtpindex)
{
	wid_syslog_debug_debug(WID_WTPINFO,"*** CWParseMsgElemAPIfInfoReport ***\n");
	wid_syslog_debug_debug(WID_WTPINFO,"*** len:%d ***\n",len);
	int i = 0;
	int state = 0;
	
	CWParseMessageElementStart();
	AC_WTP[wtpindex]->apifinfo.eth_num = CWProtocolRetrieve8(msgPtr);
	if(AC_WTP[wtpindex]->apifinfo.eth_num > AP_ETH_IF_NUM){
		AC_WTP[wtpindex]->apifinfo.eth_num = 1;
		wid_syslog_err("CWParseMsgElemAPIfInfoReport ap send eth num error\n");
		return CW_FALSE;
	}
	wid_syslog_debug_debug(WID_WTPINFO,"eth_num %d\n",AC_WTP[wtpindex]->apifinfo.eth_num);
	
	for(i=0;i<(AC_WTP[wtpindex]->apifinfo.eth_num);i++)
	{
		AC_WTP[wtpindex]->apifinfo.eth[i].type = 0;// 0-eth
		AC_WTP[wtpindex]->apifinfo.eth[i].ifindex = CWProtocolRetrieve8(msgPtr);
		state = CWProtocolRetrieve8(msgPtr);// 0-not exist/1-up/2-down/3-error
		if((state != 0)&&(state != AC_WTP[wtpindex]->apifinfo.eth[i].state))
		{
			time_t t;
			time(&t);
			AC_WTP[wtpindex]->apifinfo.eth[i].state_time = t;
		}
		
		AC_WTP[wtpindex]->apifinfo.eth[i].state = state;

		wid_syslog_debug_debug(WID_WTPINFO,"eth type %d 0-eth 1-wifi\n",AC_WTP[wtpindex]->apifinfo.eth[i].type);
		wid_syslog_debug_debug(WID_WTPINFO,"eth index %d \n",AC_WTP[wtpindex]->apifinfo.eth[i].ifindex);
		wid_syslog_debug_debug(WID_WTPINFO,"eth state %d 0-not exist/1-up/2-down/3-error\n",AC_WTP[wtpindex]->apifinfo.eth[i].state);
		
	}
	/* zhangshu modify for show over 2 eth ap information, 2010-09-27
	//added by weianying for ZheJiang telecom test ,we only display one eth interface

	if(AC_WTP[wtpindex]->apifinfo.eth_num > 1)
	{
		AC_WTP[wtpindex]->apifinfo.eth_num = 1;
	}*/

	AC_WTP[wtpindex]->apifinfo.wifi_num = CWProtocolRetrieve8(msgPtr);
	if(AC_WTP[wtpindex]->apifinfo.wifi_num > AP_WIFI_IF_NUM){
		AC_WTP[wtpindex]->apifinfo.wifi_num = 1;
		wid_syslog_err("CWParseMsgElemAPIfInfoReport ap send wifi num error\n");
		return CW_FALSE;
	}
	wid_syslog_debug_debug(WID_WTPINFO,"wifi_num %d\n",AC_WTP[wtpindex]->apifinfo.wifi_num);
	
	for(i=0;i<(AC_WTP[wtpindex]->apifinfo.wifi_num);i++)
	{
		AC_WTP[wtpindex]->apifinfo.wifi[i].type = 1;// 1-wifi
		AC_WTP[wtpindex]->apifinfo.wifi[i].ifindex = CWProtocolRetrieve8(msgPtr);
		state = CWProtocolRetrieve8(msgPtr);// 0-not exist/1-up/2-down/3-error
		if((state != 0)&&(state != AC_WTP[wtpindex]->apifinfo.wifi[i].state))
		{
			time_t t;
			time(&t);
			AC_WTP[wtpindex]->apifinfo.wifi[i].state_time = t;
		}
		
		AC_WTP[wtpindex]->apifinfo.wifi[i].state = state;

		wid_syslog_debug_debug(WID_WTPINFO,"wifi type %d 0-eth 1-wifi\n",AC_WTP[wtpindex]->apifinfo.wifi[i].type);
		wid_syslog_debug_debug(WID_WTPINFO,"wifi index %d \n",AC_WTP[wtpindex]->apifinfo.wifi[i].ifindex);
		wid_syslog_debug_debug(WID_WTPINFO,"wifi state %d 0-not exist/1-up/2-down/3-error\n",AC_WTP[wtpindex]->apifinfo.wifi[i].state);
	}
	
	CWParseMessageElementEnd(); 
}	

CWBool CWParseMsgElemAPExtensionInfo(CWProtocolMessage *msgPtr, int len, wid_wifi_info *valPtr)
{
	wid_syslog_debug_debug(WID_WTPINFO,"*** CWParseMsgElemAPExtensionInfo ***\n");
	wid_syslog_debug_debug(WID_WTPINFO,"*** len:%d ***\n",len);
	if(len < 103)//if less than this len,it means the msg is sent by a ap which is running past software version
	{
		wid_syslog_debug_debug(WID_WTPINFO,"***CWParseMsgElemAPExtensionInfo len:%d too short***\n",len);
		return CW_FALSE;
	}
	int i = 0;
	int num = 0;
	CWParseMessageElementStart();
	int start = msgPtr->offset;  //fengwenchao add 20120314 for onlinebug-162//qiuchen copy from v1.3v
	valPtr->cpu = CWProtocolRetrieve32(msgPtr);	
	valPtr->tx_mgmt = CWProtocolRetrieve32(msgPtr);
	valPtr->rx_mgmt = CWProtocolRetrieve32(msgPtr);
	valPtr->tx_packets = CWProtocolRetrieve32(msgPtr);
	valPtr->tx_errors = CWProtocolRetrieve32(msgPtr);
	valPtr->tx_retry = CWProtocolRetrieve32(msgPtr);

	valPtr->tx_unicast = CWProtocolRetrieve32(msgPtr);
	valPtr->tx_broadcast = CWProtocolRetrieve32(msgPtr);
	valPtr->tx_multicast = CWProtocolRetrieve32(msgPtr);
	valPtr->tx_drop = CWProtocolRetrieve32(msgPtr);
	valPtr->rx_unicast = CWProtocolRetrieve32(msgPtr);
	valPtr->rx_broadcast = CWProtocolRetrieve32(msgPtr);
	valPtr->rx_multicast = CWProtocolRetrieve32(msgPtr);
	valPtr->rx_drop = CWProtocolRetrieve32(msgPtr);

	valPtr->wpi_replay_error = CWProtocolRetrieve32(msgPtr);
	valPtr->wpi_decryptable_error = CWProtocolRetrieve32(msgPtr);
	valPtr->wpi_mic_error = CWProtocolRetrieve32(msgPtr);
	//printf("wpi_replay_error %d\n",valPtr->wpi_replay_error);
	//printf("wpi_decryptable_error %d\n",valPtr->wpi_decryptable_error);
	//printf("wpi_mic_error %d\n",valPtr->wpi_mic_error);
	
	valPtr->disassoc_unnormal = CWProtocolRetrieve32(msgPtr);
	valPtr->rx_assoc_norate = CWProtocolRetrieve32(msgPtr);
	valPtr->rx_assoc_capmismatch = CWProtocolRetrieve32(msgPtr);
	valPtr->assoc_invaild = CWProtocolRetrieve32(msgPtr);
	valPtr->reassoc_deny = CWProtocolRetrieve32(msgPtr);
	//printf("disassoc_unnormal %d\n",valPtr->disassoc_unnormal);
	//printf("rx_assoc_norate %d\n",valPtr->rx_assoc_norate);
	//printf("rx_assoc_capmismatch %d\n",valPtr->rx_assoc_capmismatch);
	//printf("assoc_invaild %d\n",valPtr->assoc_invaild);
	//printf("reassoc_deny %d\n",valPtr->reassoc_deny);
	//sz0803
	valPtr->ipmode = CWProtocolRetrieve8(msgPtr);
	valPtr->memoryall = CWProtocolRetrieve16(msgPtr);
	valPtr->memoryuse = CWProtocolRetrieve8(msgPtr);
	valPtr->flashall = CWProtocolRetrieve16(msgPtr);
	valPtr->flashempty = CWProtocolRetrieve32(msgPtr);
	valPtr->wifi_snr = CWProtocolRetrieve8(msgPtr);
	//sz add  04-15
	valPtr->temperature = CWProtocolRetrieve8(msgPtr);
	
	valPtr->eth_count = CWProtocolRetrieve8(msgPtr);
	if(valPtr->eth_count > AP_ETH_IF_NUM){
		valPtr->eth_count = 1;
		wid_syslog_err("CWParseMsgElemAPExtensionInfo ap send eth num error\n");
		return CW_FALSE;
	}
	for(i=0;i<valPtr->eth_count;i++)
	{
		num = CWProtocolRetrieve8(msgPtr);
		if(num >= AP_ETH_IF_NUM)
		{
			wid_syslog_debug_debug(WID_WTPINFO,"CWParseMsgElemAPExtensionInfo ap send eth num error\n");
			return CW_FALSE;
		}
		valPtr->eth_updown_time[num] = CWProtocolRetrieve8(msgPtr);
		wid_syslog_debug_debug(WID_WTPINFO,"num %d valPtr->eth_updown_time %d\n",num,valPtr->eth_updown_time[num]);
	}
	valPtr->ath_count = CWProtocolRetrieve8(msgPtr);
	if(valPtr->ath_count > AP_ATH_IF_NUM){
		valPtr->ath_count = 1;
		wid_syslog_err("CWParseMsgElemAPExtensionInfo ap send ath num error\n");
		return CW_FALSE;
	}
	for(i=0;i<valPtr->ath_count;i++)
	{
		valPtr->ath_if_info[i].radioid = CWProtocolRetrieve8(msgPtr);
		valPtr->ath_if_info[i].wlanid = CWProtocolRetrieve8(msgPtr);
		valPtr->ath_if_info[i].ath_updown_times = CWProtocolRetrieve8(msgPtr);
		if((valPtr->ath_if_info[i].radioid > L_RADIO_NUM)||(valPtr->ath_if_info[i].wlanid > WLAN_NUM))
		{
			wid_syslog_debug_debug(WID_WTPINFO,"CWParseMsgElemAPExtensionInfo ap send radio num error\n");
			return CW_FALSE;
		}
		wid_syslog_debug_debug(WID_WTPINFO,"radioid %d wlanid %d ath_updown_times %d\n",valPtr->ath_if_info[i].radioid,valPtr->ath_if_info[i].wlanid,valPtr->ath_if_info[i].ath_updown_times);
	}
	//sz add  04-15
	valPtr->wifi_count = CWProtocolRetrieve8(msgPtr);
	if(valPtr->wifi_count > AP_WIFI_IF_NUM){
		valPtr->wifi_count = 1;
		wid_syslog_err("CWParseMsgElemAPExtensionInfo ap send wifi num error\n");
		return CW_FALSE;
	}
	for(i=0;i<valPtr->wifi_count;i++)
	{
		num = CWProtocolRetrieve8(msgPtr);
		if(num >= AP_WIFI_IF_NUM)
		{
			wid_syslog_err("CWParseMsgElemAPExtensionInfo ap send wifi num error\n");
			return CW_FALSE;
		}
		valPtr->wifi_state[num] = CWProtocolRetrieve8(msgPtr);
		wid_syslog_debug_debug(WID_WTPINFO,"num %d valPtr->wifi_state %d\n",num,valPtr->wifi_state[num]);
	}
	/*fengwenchao add 20120314 for onlinebug-162*///qiuchen copy from v1.3
	if((msgPtr->offset - start)<len)
	{
		for(i=0;i<valPtr->wifi_count;i++)
		{
			valPtr->wifi_snr_new[i] = CWProtocolRetrieve8(msgPtr);
			valPtr->wifi_noise_new[i] = CWProtocolRetrieve16(msgPtr);
			wid_syslog_debug_debug(WID_WTPINFO,"CWParseMsgElemAPExtensionInfo ap send radio %d wifi_snr_new  %d\n",i,valPtr->wifi_snr_new[i]);
			wid_syslog_debug_debug(WID_WTPINFO,"CWParseMsgElemAPExtensionInfo ap send radio %d wifi_noise_new  %d\n",i,valPtr->wifi_noise_new[i]);
		}	
	}
	else
	{
		for(i=0;i<valPtr->wifi_count;i++){
			valPtr->wifi_snr_new[i] = valPtr->wifi_snr;
			valPtr->wifi_noise_new[i] = 95;
			wid_syslog_debug_debug(WID_WTPINFO,"CWParseMsgElemAPExtensionInfo not receiv  %d wifi_snr_new  %d\n",i,valPtr->wifi_snr_new[i]);
			wid_syslog_debug_debug(WID_WTPINFO,"CWParseMsgElemAPExtensionInfo not receiv %d wifi_noise_new	%d\n",i,valPtr->wifi_noise_new[i]);
		}
	}
	/*fengwenchao add end*/
	/*
	printf("cpu %d\n",valPtr->cpu);
	printf("tx_mgmt %d\n",valPtr->tx_mgmt);
	printf("rx_mgmt %d\n",valPtr->rx_mgmt);
	printf("tx_packets %d\n",valPtr->tx_packets);
	printf("tx_errors %d\n",valPtr->tx_errors);
	printf("tx_retry %d\n",valPtr->tx_retry);
	printf("ipmode %d\n",valPtr->ipmode);
	printf("memoryuse %d\n",valPtr->memoryuse);
	printf("flashall %d\n",valPtr->flashall);
	printf("flashempty %d\n",valPtr->cpu);
	printf("eth_count %d\n",valPtr->eth_count);
	printf("ath_count %d\n",valPtr->ath_count);
	printf("temperature %d\n",valPtr->temperature);
	printf("wifi_count %d\n",valPtr->wifi_count);
	*/
	if(((msgPtr->offset) - oldOffset) == len){
		return CW_TRUE;
	}else{
		if(((msgPtr->offset) - oldOffset) > len){
			wid_syslog_err("%s,%d,parse more than msg,((msgPtr->offset) - oldOffset)=%d,len=%d.\n",__func__,__LINE__,((msgPtr->offset) - oldOffset),len);
		}else{
			wid_syslog_err("%s,%d,parse less than msg,((msgPtr->offset) - oldOffset)=%d,len=%d.\n",__func__,__LINE__,((msgPtr->offset) - oldOffset),len);
		}
		return CW_FALSE;
	}
	CWParseMessageElementEnd(); 

}

CWBool CWParseMsgElemAPStaWapiInfos(CWProtocolMessage *msgPtr, int len, WIDStaWapiInfoList *valPtr,int wtpindex)
{
	wid_syslog_debug_debug(WID_DEFAULT,"*** CWParseMsgElemAPStaWapiInfos ***\n");
	wid_syslog_debug_debug(WID_DEFAULT,"*** len:%d ***\n",len);
	int i = 0;
	//int state = 0;
	
	CWParseMessageElementStart();
	valPtr->WTPID = wtpindex;
	valPtr->sta_num = CWProtocolRetrieve8(msgPtr);
	
	wid_syslog_debug_debug(WID_DEFAULT,"eth_num %d\n",AC_WTP[wtpindex]->apifinfo.eth_num);
	
	for(i=0;i<(valPtr->sta_num)&& i < 64;i++)
	{
		valPtr->StaWapiInfo[i].RadioId = CWProtocolRetrieve8(msgPtr);		
		valPtr->StaWapiInfo[i].WlanId = CWProtocolRetrieve8(msgPtr);		
		char * str = CWProtocolRetrieveStr(msgPtr,6);
		memcpy(valPtr->StaWapiInfo[i].mac,str, 6);
		free(str);
		valPtr->StaWapiInfo[i].WAPIVersion = CWProtocolRetrieve32(msgPtr); 
		valPtr->StaWapiInfo[i].ControlledPortStatus = CWProtocolRetrieve8(msgPtr);		
		str =  CWProtocolRetrieveStr(msgPtr,4);
		memcpy(valPtr->StaWapiInfo[i].SelectedUnicastCipher,str, 4);
		free(str);		
		valPtr->StaWapiInfo[i].WPIReplayCounters= CWProtocolRetrieve32(msgPtr); 
		valPtr->StaWapiInfo[i].WPIDecryptableErrors = CWProtocolRetrieve32(msgPtr); 
		valPtr->StaWapiInfo[i].WPIMICErrors = CWProtocolRetrieve32(msgPtr); 	
		printf("wtpid %d,radioid %d,wlanid %d\nsta mac %02X:%02X:%02X:%02X:%02X:%02X\nversion %d,controlStatus %d,cipher %02X-%02X-%02X-%02X, replay %d,decryerror %d,micerror %d",
			wtpindex,valPtr->StaWapiInfo[i].RadioId,valPtr->StaWapiInfo[i].WlanId,valPtr->StaWapiInfo[i].mac[0],valPtr->StaWapiInfo[i].mac[1],valPtr->StaWapiInfo[i].mac[2],valPtr->StaWapiInfo[i].mac[3],valPtr->StaWapiInfo[i].mac[4],valPtr->StaWapiInfo[i].mac[5],
		valPtr->StaWapiInfo[i].WAPIVersion,valPtr->StaWapiInfo[i].ControlledPortStatus,valPtr->StaWapiInfo[i].SelectedUnicastCipher[0],valPtr->StaWapiInfo[i].SelectedUnicastCipher[1],valPtr->StaWapiInfo[i].SelectedUnicastCipher[2],valPtr->StaWapiInfo[i].SelectedUnicastCipher[3],
		valPtr->StaWapiInfo[i].WPIReplayCounters,valPtr->StaWapiInfo[i].WPIDecryptableErrors,valPtr->StaWapiInfo[i].WPIMICErrors);
	}
	WidAsdStaWapiInfoUpdate(wtpindex, valPtr);
	CWParseMessageElementEnd(); 
}	


CWBool CWParseSessionID(CWProtocolMessage *msgPtr, int len, CWProtocolJoinRequestValues *valPtr) {
	
	CWParseMessageElementStart();
	
	valPtr->sessionID= CWProtocolRetrieve32(msgPtr);
	wid_syslog_debug_debug(WID_WTPINFO,"Session ID: %d",valPtr->sessionID);
	
	CWParseMessageElementEnd();
}

CWBool CWParseWTPStatisticsTimer (CWProtocolMessage *msgPtr, int len, int *valPtr)
{
	CWParseMessageElementStart();
	
	*valPtr = CWProtocolRetrieve16(msgPtr);
	
	//wid_syslog_debug_debug("WTP Statistics Timer: %d", *valPtr);
	
	CWParseMessageElementEnd();
}

CWBool CWParseWTPBoardData (CWProtocolMessage *msgPtr, int len, CWWTPVendorInfos *valPtr)
{
	int theOffset, i, vendorID;
	char *strmodel = NULL;
	char *codever = NULL;
	unsigned short unserved;
	char *SN_t = NULL;
	CWParseMessageElementStart();
	
	valPtr->vendorInfosCount = 0;
	
	// see how many vendor ID we have in the message
	vendorID = CWProtocolRetrieve32(msgPtr); // ID
	theOffset = msgPtr->offset;
	while((msgPtr->offset-oldOffset) < len) {	// oldOffset stores msgPtr->offset's value at the beginning of this function.
							// See the definition of the CWParseMessageElementStart() macro.
		int tmp;

		CWProtocolRetrieve16(msgPtr); // type
		tmp = CWProtocolRetrieve16(msgPtr);
		msgPtr->offset += tmp; // len
		valPtr->vendorInfosCount++;
	}
	
	msgPtr->offset = theOffset;
//	printf("valPtr->vendorInfosCount %d\n",valPtr->vendorInfosCount);
	// actually read each vendor ID
	CW_CREATE_ARRAY_ERR(valPtr->vendorInfos, valPtr->vendorInfosCount, CWWTPVendorInfoValues, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	memset(valPtr->vendorInfos,0,sizeof(CWWTPVendorInfoValues)*valPtr->vendorInfosCount);
	for(i = 0; i < valPtr->vendorInfosCount; i++) {
		(valPtr->vendorInfos)[i].vendorIdentifier = vendorID;
		(valPtr->vendorInfos)[i].type = CWProtocolRetrieve16(msgPtr);																
		(valPtr->vendorInfos)[i].length = CWProtocolRetrieve16(msgPtr);
		if((valPtr->vendorInfos)[i].type == CW_WTP_SERIAL_NUMBER)
		{	
		//	printf("(valPtr->vendorInfos)[i].length %d\n",(valPtr->vendorInfos)[i].length);
		//	printf("(msgPtr->msg)[(msgPtr->offset)] %s\n",(msgPtr->msg)+(msgPtr->offset));
			if(((valPtr->vendorInfos)[i].length  < 0)||((valPtr->vendorInfos)[i].length > 64)){
				wid_syslog_err("<error>CW_WTP_SERIAL_NUMBER %s length==%d\n",__func__,(valPtr->vendorInfos)[i].length);
				(valPtr->vendorInfos)[i].length = 64;
			}else{
			
			}
			SN_t = (CWProtocolRetrieveRawBytes(msgPtr, (valPtr->vendorInfos)[i].length));	
			(valPtr->vendorInfos)[i].SN = (unsigned char *)malloc(((valPtr->vendorInfos)[i].length)+1);
			memset((valPtr->vendorInfos)[i].SN,0,(((valPtr->vendorInfos)[i].length)+1));
			if(wid_illegal_character_check(SN_t, strlen(SN_t),0) == 1){
				memcpy((valPtr->vendorInfos)[i].SN,SN_t,(valPtr->vendorInfos)[i].length);
			}else{
				wid_syslog_info("%s CW_WTP_SERIAL_NUMBER %s something wrong\n",__func__,SN_t);
			}
			if(SN_t != NULL){
				free(SN_t);
				SN_t = NULL;
			}
			//(valPtr->vendorInfos)[i].SN = (unsigned char*)(CWProtocolRetrieveRawBytes(msgPtr, (valPtr->vendorInfos)[i].length));	
			if((valPtr->vendorInfos)[i].SN == NULL) return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
		//	printf("CW_WTP_SERIAL_NUMBER\n");
		wid_syslog_debug_debug(WID_WTPINFO,"WTP Board Data: %d - %d - %d - %s", (valPtr->vendorInfos)[i].vendorIdentifier, (valPtr->vendorInfos)[i].type, (valPtr->vendorInfos)[i].length, (valPtr->vendorInfos)[i].SN);//sz to be determined
		}
		else if((valPtr->vendorInfos)[i].type == CW_BOARD_MAC_ADDRESS){
//			printf("mac\n");
//			printf("(valPtr->vendorInfos)[i].length %d",(valPtr->vendorInfos)[i].length);
//			printf("len:%d,msgPtr->offset:%d,mac: %x",msgPtr->msgLen,msgPtr->offset,((msgPtr->msg)[(msgPtr->offset)]));
			if(((valPtr->vendorInfos)[i].length<6)||((valPtr->vendorInfos)[i].length>64)){
				wid_syslog_err("<error>CW_BOARD_MAC_ADDRESS %s length==%d\n",__func__,(valPtr->vendorInfos)[i].length);
				(valPtr->vendorInfos)[i].length = 64;
			}else{

			}
			(valPtr->vendorInfos)[i].mac = (unsigned char*)(CWProtocolRetrieveRawBytes(msgPtr, (valPtr->vendorInfos)[i].length));	
		wid_syslog_debug_debug(WID_WTPINFO,"WTP MAC:%02X %02X %02X %02X %02X %02X\n",(valPtr->vendorInfos)[i].mac[0],(valPtr->vendorInfos)[i].mac[1],(valPtr->vendorInfos)[i].mac[2],(valPtr->vendorInfos)[i].mac[3],(valPtr->vendorInfos)[i].mac[4],(valPtr->vendorInfos)[i].mac[5]);
		//printf("WTP MAC:%02X %02X %02X %02X %02X %02X\n",(valPtr->vendorInfos)[i].mac[0],(valPtr->vendorInfos)[i].mac[1],(valPtr->vendorInfos)[i].mac[2],(valPtr->vendorInfos)[i].mac[3],(valPtr->vendorInfos)[i].mac[4],(valPtr->vendorInfos)[i].mac[5]);
		}
		else if((valPtr->vendorInfos)[i].type == CW_WTP_MODEL_NUMBER)//added 2008/0923
		{
			
			if(((valPtr->vendorInfos)[i].length<0)||((valPtr->vendorInfos)[i].length>64)){
				wid_syslog_err("<error>CW_WTP_MODEL_NUMBER %s length==%d\n",__func__,(valPtr->vendorInfos)[i].length);
				(valPtr->vendorInfos)[i].length = 64;
			}else{
			}
			strmodel = (CWProtocolRetrieveRawBytes(msgPtr, (valPtr->vendorInfos)[i].length));	

			(valPtr->vendorInfos)[i].model = (unsigned char *)malloc(((valPtr->vendorInfos)[i].length)+1);
			memset((valPtr->vendorInfos)[i].model,0,(((valPtr->vendorInfos)[i].length)+1));
			if(wid_illegal_character_check(strmodel, strlen(strmodel),0) == 1){
				memcpy((valPtr->vendorInfos)[i].model,strmodel,(((valPtr->vendorInfos)[i].length)));
			}else{
				wid_syslog_info("%s CW_WTP_MODEL_NUMBER %s something wrong\n",__func__,strmodel);
			}
			free(strmodel);
			strmodel = NULL;
			if((valPtr->vendorInfos)[i].model == NULL) return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);

			if((valPtr->vendorInfos)[i].length == 4)
			{
				(valPtr->vendorInfos)[i].valuePtr = (int*) ((valPtr->vendorInfos)[i].model);
			}
			else
			{
				(valPtr->vendorInfos)[i].valuePtr = NULL;
			}
			
			wid_syslog_debug_debug(WID_WTPINFO,"WTP Board Data: %d - %d - %d - %s", (valPtr->vendorInfos)[i].vendorIdentifier, (valPtr->vendorInfos)[i].type, (valPtr->vendorInfos)[i].length, (valPtr->vendorInfos)[i].model);
		}
		else if((valPtr->vendorInfos)[i].type == CW_WTP_REAL_MODEL_NUMBER){
			
			if(((valPtr->vendorInfos)[i].length<0)||((valPtr->vendorInfos)[i].length>64)){
				wid_syslog_err("<error>CW_WTP_REAL_MODEL_NUMBER %s length==%d\n",__func__,(valPtr->vendorInfos)[i].length);
				(valPtr->vendorInfos)[i].length = 64;
			}else{
			}
			strmodel = (CWProtocolRetrieveRawBytes(msgPtr, (valPtr->vendorInfos)[i].length));	

			(valPtr->vendorInfos)[i].Rmodel = (unsigned char *)malloc(((valPtr->vendorInfos)[i].length)+1);
			memset((valPtr->vendorInfos)[i].Rmodel,0,(((valPtr->vendorInfos)[i].length)+1));
			if(wid_illegal_character_check(strmodel, strlen(strmodel),0) == 1){
				memcpy((valPtr->vendorInfos)[i].Rmodel,strmodel,(((valPtr->vendorInfos)[i].length)));
			}else{
				wid_syslog_info("%s CW_WTP_REAL_MODEL_NUMBER %s something wrong\n",__func__,strmodel);
			}
			free(strmodel);
			strmodel = NULL;
			if((valPtr->vendorInfos)[i].Rmodel == NULL) return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);

			if((valPtr->vendorInfos)[i].length == 4)
			{
				(valPtr->vendorInfos)[i].valuePtr = (int*) ((valPtr->vendorInfos)[i].Rmodel);
			}
			else
			{
				(valPtr->vendorInfos)[i].valuePtr = NULL;
			}
			//printf("Rmodel :%s\n",(valPtr->vendorInfos)[i].Rmodel);			
		}
		else if((valPtr->vendorInfos)[i].type == CW_WTP_CODE_VERSION){
			
			if(((valPtr->vendorInfos)[i].length<0)||((valPtr->vendorInfos)[i].length>64)){
				wid_syslog_err("<error>CW_WTP_CODE_VERSION %s length==%d\n",__func__,(valPtr->vendorInfos)[i].length);
				(valPtr->vendorInfos)[i].length = 64;
			}else{
			}
			codever = (CWProtocolRetrieveRawBytes(msgPtr, (valPtr->vendorInfos)[i].length));	

			(valPtr->vendorInfos)[i].codever = (unsigned char *)malloc(((valPtr->vendorInfos)[i].length)+1);
			memset((valPtr->vendorInfos)[i].codever,0,(((valPtr->vendorInfos)[i].length)+1));
			if(wid_illegal_character_check(codever, strlen(codever),0) == 1){
				memcpy((valPtr->vendorInfos)[i].codever,codever,(((valPtr->vendorInfos)[i].length)));
			}else{
				wid_syslog_info("%s CW_WTP_CODE_VERSION %s something wrong\n",__func__,codever);
			}
			free(codever);
			codever = NULL;
			if((valPtr->vendorInfos)[i].codever == NULL) return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
			
			printf("codever :%s\n",(valPtr->vendorInfos)[i].codever);			
			wid_syslog_debug_debug(WID_WTPINFO,"codever :%s\n",(valPtr->vendorInfos)[i].codever);			
		}
		else if((valPtr->vendorInfos)[i].type == CW_WTP_MANUFACTURE_OPTION)
		{
			(valPtr->vendorInfos)[i].manuoption =  CWProtocolRetrieve16(msgPtr);
			unserved = 	CWProtocolRetrieve16(msgPtr);		
			printf("############manuoption :%d\n",(valPtr->vendorInfos)[i].manuoption);			
			wid_syslog_debug_debug(WID_WTPINFO,"manuoption :%d\n",(valPtr->vendorInfos)[i].manuoption);			
		}		
		else
		{			
			//if(((valPtr->vendorInfos)[i].length == 4)&&((valPtr->vendorInfos)[i].type != CW_WTP_SERIAL_NUMBER))
			//*((valPtr->vendorInfos)[i].valuePtr) = ntohl(*((valPtr->vendorInfos)[i].valuePtr));

		}

		//special upgrade 20081007 by weiay
		//if(((valPtr->vendorInfos)[i].length == 4)&&((valPtr->vendorInfos)[i].type != CW_WTP_SERIAL_NUMBER))	
		//{
			//printf("##### moded type char* to int ######\n");		
			//(valPtr->vendorInfos)[i].valuePtr = (int*) ((valPtr->vendorInfos)[i].model);	
			//if((valPtr->vendorInfos)[i].valuePtr == NULL) return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
			
			//printf("WTP Board Data: %d - %d - %d - %d\n", (valPtr->vendorInfos)[i].vendorIdentifier, (valPtr->vendorInfos)[i].type, (valPtr->vendorInfos)[i].length, *(valPtr->vendorInfos)[i].valuePtr);
		//}
		
	//wid_syslog_debug_debug("WTP Board Data: %d - %d - %d - %d", (valPtr->vendorInfos)[i].vendorIdentifier, (valPtr->vendorInfos)[i].type, (valPtr->vendorInfos)[i].length, (valPtr->vendorInfos)[i].valuePtr);
	}
//	printf("CWParseWTPBoardData finish\n");
	CWParseMessageElementEnd();
}

CWBool CWCheckWTPBoardData(int WTPIndex, CWWTPVendorInfos *valPtr){
	//int mac = -1;
	int i;
	int isbasemac = 0;
	char *savesn = NULL;
//	printf("CWCheckWTPBoardData start\n");
	wid_syslog_debug_debug(WID_WTPINFO," check WTP SN whether match wtps we have known");
	for(i = 0; i < valPtr->vendorInfosCount; i++){
		if((valPtr->vendorInfos)[i].type == CW_WTP_MODEL_NUMBER){
			if((AC_WTP[WTPIndex]->APCode != NULL)&&(memcmp((valPtr->vendorInfos)[i].model, AC_WTP[WTPIndex]->APCode, strlen(AC_WTP[WTPIndex]->APCode)) == 0)) //fengwenchao modify 20110427
			//if(*((valPtr->vendorInfos)[i].valuePtr) == AC_WTP[WTPIndex]->WTPModel)
				continue;
			else
			{
				//special upgrade 20081007 by weiay
				if((((valPtr->vendorInfos)[i].valuePtr != NULL)&&(*((valPtr->vendorInfos)[i].valuePtr) == 1000))
					||((AC_WTP[WTPIndex]->WTPModel != NULL)&&(strncasecmp((char*)((valPtr->vendorInfos)[i].model), AC_WTP[WTPIndex]->WTPModel, strlen(AC_WTP[WTPIndex]->WTPModel)) == 0)))  //fengwenchao modify 20110427
				{
					//printf("##### model int 1000 success######\n");
					continue;
				}
				wid_syslog_debug_debug(WID_WTPINFO,"%s not mattach model %s\n",(valPtr->vendorInfos)[i].model, AC_WTP[WTPIndex]->APCode);
				//return CW_FALSE;
			}
			}
		else if((valPtr->vendorInfos)[i].type == CW_WTP_SERIAL_NUMBER){
			char * sn = (char *)((valPtr->vendorInfos)[i].SN);
			str2higher(&sn);
			(valPtr->vendorInfos)[i].SN = (unsigned char *)sn;
		
			if((AC_WTP[WTPIndex]->WTPSN != NULL)&&(memcmp((valPtr->vendorInfos)[i].SN, AC_WTP[WTPIndex]->WTPSN, strlen(AC_WTP[WTPIndex]->WTPSN)) == 0))    //fengwenchao modify 20110427
			{
				wid_syslog_debug_debug(WID_WTPINFO,"WTP %d sn %s",WTPIndex,AC_WTP[WTPIndex]->WTPSN);
				isbasemac = 1;//wuwl add 20101118 to resolv prolem for jinhua,ng'ap have the same sn.wtp1 maybe run into another wtp.
				continue;
			}
			else{
				wid_syslog_debug_debug(WID_WTPINFO,"%s not mattach sn %s\n",(valPtr->vendorInfos)[i].SN, AC_WTP[WTPIndex]->WTPSN);
				if((AC_WTP[WTPIndex]->WTPSN != NULL)&&(memcmp(AC_WTP[WTPIndex]->WTPSN, gdefaultsn,strlen(AC_WTP[WTPIndex]->WTPSN)) == 0))    //fengwenchao modify 20110427
				{
					if(savesn){
						free(savesn);
						savesn = NULL;
					}
					savesn = (char*)malloc((valPtr->vendorInfos)[i].length +1);
					memset(savesn, 0, (valPtr->vendorInfos)[i].length+1);
					/*??*/
					if((valPtr->vendorInfos)[i].length > NAS_IDENTIFIER_NAME){
						wid_syslog_info("SN length %d\n",(valPtr->vendorInfos)[i].length);
						memcpy(savesn, (valPtr->vendorInfos)[i].SN, NAS_IDENTIFIER_NAME);
					}else
						memcpy(savesn, (valPtr->vendorInfos)[i].SN, (valPtr->vendorInfos)[i].length);
					isbasemac = 1;
					continue;
				}
				else
				{
					if(savesn){
						free(savesn);
						savesn = NULL;
					}
					savesn = (char*)malloc((valPtr->vendorInfos)[i].length +1);
					memset(savesn, 0, (valPtr->vendorInfos)[i].length+1);
					if((valPtr->vendorInfos)[i].length > NAS_IDENTIFIER_NAME){
						wid_syslog_info("SN length %d\n",(valPtr->vendorInfos)[i].length);
						memcpy(savesn, (valPtr->vendorInfos)[i].SN, NAS_IDENTIFIER_NAME);
					}else
						memcpy(savesn, (valPtr->vendorInfos)[i].SN, (valPtr->vendorInfos)[i].length);
					isbasemac = 1;
					continue;
					//return CW_FALSE;
				}
			}
				
		}
		else if((valPtr->vendorInfos)[i].type == CW_BOARD_ID)
			continue;
		else if((valPtr->vendorInfos)[i].type == CW_BOARD_REVISION)
			continue;
		else if((valPtr->vendorInfos)[i].type == CW_BOARD_MAC_ADDRESS){
//			printf("CW_BOARD_MAC_ADDRESS\n");
//			printf("%X %X %X %X %X %X",(valPtr->vendorInfos)[i].mac[0],(valPtr->vendorInfos)[i].mac[1],(valPtr->vendorInfos)[i].mac[2],(valPtr->vendorInfos)[i].mac[3],(valPtr->vendorInfos)[i].mac[4],(valPtr->vendorInfos)[i].mac[5]);

			if(isbasemac == 1)
			{
				isbasemac = 0;

				if(((valPtr->vendorInfos)[i].mac[0] == AC_WTP[WTPIndex]->WTPMAC[0])&&((valPtr->vendorInfos)[i].mac[1] == AC_WTP[WTPIndex]->WTPMAC[1])&&((valPtr->vendorInfos)[i].mac[2] == AC_WTP[WTPIndex]->WTPMAC[2])\
					&&((valPtr->vendorInfos)[i].mac[3] == AC_WTP[WTPIndex]->WTPMAC[3])&&((valPtr->vendorInfos)[i].mac[4] == AC_WTP[WTPIndex]->WTPMAC[4])&&((valPtr->vendorInfos)[i].mac[5] == AC_WTP[WTPIndex]->WTPMAC[5]))
				{
					//printf("dddddddddd%d\n",strlen(savesn));
					/*	default code			
					free(AC_WTP[WTPIndex]->WTPSN);
					AC_WTP[WTPIndex]->WTPSN = NULL;
					
					AC_WTP[WTPIndex]->WTPSN = (char*)malloc(strlen(savesn)+1);
					memset(AC_WTP[WTPIndex]->WTPSN, 0, strlen(savesn)+1);
					memcpy(AC_WTP[WTPIndex]->WTPSN, savesn, strlen(savesn));					
					*/
					if(savesn != NULL){
					if(wid_illegal_character_check(savesn, strlen(savesn),0) == 1){
						if(AC_WTP[WTPIndex]->WTPSN != NULL)      //fengwenchao modify 20110427
						{
							memset(AC_WTP[WTPIndex]->WTPSN, 0, NAS_IDENTIFIER_NAME);
							memcpy(AC_WTP[WTPIndex]->WTPSN, savesn, strlen(savesn));							
						}
					}else{
						wid_syslog_info("%s wtp %d sn %s something wrong\n",__func__,WTPIndex,savesn);
					}
                        free(savesn);
                        savesn = NULL;
					}
					continue;
				}
				else
				{
					if(savesn){
					free(savesn);
					savesn = NULL;
					}
					return CW_FALSE;
				}
			}
			else
			{
				memcpy(AC_WTP[WTPIndex]->WTPMAC, (valPtr->vendorInfos)[i].mac, 6);
				wid_syslog_debug_debug(WID_WTPINFO,"WTP %d MAC:%X %X %X %X %X %X",WTPIndex,AC_WTP[WTPIndex]->WTPMAC[0],AC_WTP[WTPIndex]->WTPMAC[1],AC_WTP[WTPIndex]->WTPMAC[2],AC_WTP[WTPIndex]->WTPMAC[3],AC_WTP[WTPIndex]->WTPMAC[4],AC_WTP[WTPIndex]->WTPMAC[5]);
				continue;
			}
		}
		else if((valPtr->vendorInfos)[i].type == CW_WTP_REAL_MODEL_NUMBER)
		{   
		    /* book add,2011-11-04 */
		    if((AC_WTP[WTPIndex]->WTPModel != NULL)&&((valPtr->vendorInfos)[i].Rmodel != NULL)&&(memcmp((valPtr->vendorInfos)[i].Rmodel, AC_WTP[WTPIndex]->WTPModel, strlen(AC_WTP[WTPIndex]->WTPModel)) == 0))
			{
			    continue;
			}
			else{
			    wid_syslog_info("Error:model not match\n");
			    wid_syslog_debug_debug(WID_WTPINFO,"WTPModel = %s, Rmodel = %s\n",AC_WTP[WTPIndex]->WTPModel,(valPtr->vendorInfos)[i].Rmodel);
				CW_FREE_OBJECT(savesn);
			    return CW_FALSE;
			}
	    }
		else
			continue;
	}
/*	printf("mac : %d",mac);
	if(mac > 0)
	memcpy(AC_WTP[WTPIndex]->WTPMAC, (valPtr->vendorInfos)[i].mac, 6);	
	printf("CWCheckWTPBoardData finish\n");*/
	CW_FREE_OBJECT(savesn);
	return CW_TRUE;
}

CWBool CWCheckRunWTPBoardData(int WTPIndex, CWWTPVendorInfos *valPtr){
	int i;
	msgq msg;
	int WTPMsgqID;
	CWGetMsgQueue(&WTPMsgqID);
	for(i = 0; i < valPtr->vendorInfosCount; i++){
		if((valPtr->vendorInfos)[i].type == CW_WTP_SERIAL_NUMBER){
			char * sn = (char *)((valPtr->vendorInfos)[i].SN);
			str2higher(&sn);
			(valPtr->vendorInfos)[i].SN = (unsigned char *)sn;
		
			if(memcmp((valPtr->vendorInfos)[i].SN, AC_WTP[WTPIndex]->WTPSN, strlen(AC_WTP[WTPIndex]->WTPSN)) == 0)
			{
				continue;
			}else{
				return CW_FALSE;
			}						
		}
		else if((valPtr->vendorInfos)[i].type == CW_BOARD_MAC_ADDRESS){

				if(((valPtr->vendorInfos)[i].mac[0] == AC_WTP[WTPIndex]->WTPMAC[0])&&((valPtr->vendorInfos)[i].mac[1] == AC_WTP[WTPIndex]->WTPMAC[1])&&((valPtr->vendorInfos)[i].mac[2] == AC_WTP[WTPIndex]->WTPMAC[2])\
					&&((valPtr->vendorInfos)[i].mac[3] == AC_WTP[WTPIndex]->WTPMAC[3])&&((valPtr->vendorInfos)[i].mac[4] == AC_WTP[WTPIndex]->WTPMAC[4])&&((valPtr->vendorInfos)[i].mac[5] == AC_WTP[WTPIndex]->WTPMAC[5]))
				{
					continue;
				}
				else
				{
					return CW_FALSE;
				}
		}
		else
			continue;
	}
	wid_syslog_err("\n\n check discovery ,and ap quit\n\n");
	gWTPs[WTPIndex].isRequestClose = CW_TRUE;	
	memset((char*)&msg, 0, sizeof(msg));
	msg.mqid = WTPIndex%THREAD_NUM+1;
	msg.mqinfo.WTPID = WTPIndex;
	msg.mqinfo.type = CONTROL_TYPE;
	msg.mqinfo.subtype = WTP_S_TYPE;
	msg.mqinfo.u.WtpInfo.Wtp_Op = WTP_REBOOT;
	msg.mqinfo.u.WtpInfo.WTPID = WTPIndex;
	if (msgsnd(WTPMsgqID, (msgq *)&msg, sizeof(msg.mqinfo), 0) == -1){
		wid_syslog_crit("%s msgsend %s",__func__,strerror(errno));
		perror("msgsnd");
	}
	return CW_TRUE;
}

int CWCmpWTPAttach(CWNetworkLev4Address *addrPtr){
	int ret;
	int i;
	for(i = 0; i < WTP_NUM; i++) {
		if(AC_ATTACH[i] != NULL && (&(AC_ATTACH[i]->address) != NULL) &&
					!sock_cmp_addr((struct sockaddr*)addrPtr, (struct sockaddr*)&(AC_ATTACH[i]->address), sizeof(CWNetworkLev4Address))) { 
			ret = AC_ATTACH[i]->WTPID;
			if(!check_wtpid_func(ret)){
				wid_syslog_err("%s\n",__func__);
				return -1;
			}else{
			}
			if((AC_WTP[ret]==NULL)||((AC_WTP[ret]!=NULL)&&(AC_WTP[ret]->WTPStat != 7)))
			{
				free(AC_ATTACH[i]);
				AC_ATTACH[i] = NULL;
				continue;
			}
			free(AC_ATTACH[i]);
			AC_ATTACH[i] = NULL;
			return ret;
		}
	}
	
	return -1;

}



CWBool CWDisCheckWTPBoardData(int bindingSystemIndex,CWNetworkLev4Address *addrPtr, CWWTPVendorInfos *valPtr, unsigned int *WTPID){
	int i = 1, j = 0, i1 = 0;	
	for(i1 = 0; i1 < WTP_NUM; i1++) {
		if(AC_ATTACH[i1] != NULL && (&(AC_ATTACH[i1]->address) != NULL) &&
					!sock_cmp_addr((struct sockaddr*)addrPtr, (struct sockaddr*)&(AC_ATTACH[i1]->address), sizeof(CWNetworkLev4Address))) { 
			*WTPID = AC_ATTACH[i1]->WTPID;
			return CW_TRUE;
		}
	}
			
	while(i<WTP_NUM){
		CWThreadMutexLock(&(gWTPs[i].WTPThreadMutex));
		if((AC_WTP[i] != NULL)&&(AC_WTP[i]->isused == 1)){
			if(((AC_WTP[i]->WTPStat == 7)&&(CWCheckWTPBoardData(i, valPtr)))
			/* book add state 8 for image_data,  9 for back_run, 2011-5-11 */
				||((AC_WTP[i]->WTPStat != 7)&&(CWCheckRunWTPBoardData(i, valPtr)))){
				while(j<WTP_NUM){
					if(AC_ATTACH[j] == NULL){						
					//printf("one attach\n");
					wid_syslog_debug_debug(WID_WTPINFO,"one attach");
						AC_ATTACH[j] = (CWWTPAttach*)malloc(sizeof(CWWTPAttach));
						AC_ATTACH[j]->WTPID = i;						
						CW_COPY_NET_ADDR_PTR(&(AC_ATTACH[j]->address), addrPtr);
						*WTPID = i;
						CWThreadMutexUnlock(&(gWTPs[i].WTPThreadMutex));	
						return CW_TRUE;
					}
					j++;
				}
				//printf("all AC_ATTACH full\n");
				wid_syslog_crit("all AC_ATTACH full\n");
				CWThreadMutexUnlock(&(gWTPs[i].WTPThreadMutex));	
				return CW_FALSE;
			}
			//printf("AC_WTP[%d]\n",i);
			wid_syslog_debug_debug(WID_WTPINFO,"AC_WTP[%d]",i);
		}
		CWThreadMutexUnlock(&(gWTPs[i].WTPThreadMutex));	
		i++;	
	}
	//printf("no attach AC_WTP\n");
	return CW_FALSE;

}

CWBool CWAddAC_ATTACH_For_Auto(CWNetworkLev4Address *addrPtr, unsigned int WTPID)
{
	int j = 0, i1 = 0;	
	for(i1 = 0; i1 < WTP_NUM; i1++) {
		if(AC_ATTACH[i1] != NULL && (&(AC_ATTACH[i1]->address) != NULL) &&
					!sock_cmp_addr((struct sockaddr*)addrPtr, (struct sockaddr*)&(AC_ATTACH[i1]->address), sizeof(CWNetworkLev4Address))) { 
			return CW_TRUE;
		}
	}
			

	if((AC_WTP[WTPID] != NULL)&&(AC_WTP[WTPID]->WTPStat == 7)&&(AC_WTP[WTPID]->isused == 1)){
		while(j<WTP_NUM){
			if(AC_ATTACH[j] == NULL){						
				AC_ATTACH[j] = (CWWTPAttach*)malloc(sizeof(CWWTPAttach));
				AC_ATTACH[j]->WTPID = WTPID;						
				CW_COPY_NET_ADDR_PTR(&(AC_ATTACH[j]->address), addrPtr);	
				return CW_TRUE;
			}
			j++;
		}
		return CW_FALSE;
	}
	return CW_FALSE;

}


//added by weiay 20080617
CWBool CWWTPMatchBindingInterface(int wtpid,int bindingSystemIndex)
{
	wid_syslog_debug_debug(WID_WTPINFO,"*** BindingSystemIndex is %d input index is %d\n", AC_WTP[wtpid]->BindingSystemIndex,bindingSystemIndex); 
	
	if(AC_WTP[wtpid]->BindingSystemIndex == -1)
	{
		wid_syslog_debug_debug(WID_WTPINFO,"***admin doesn't binding any interface allow all interface access**\n");
		return CW_TRUE;
	}
	else
	{
		if(AC_WTP[wtpid]->BindingSystemIndex == bindingSystemIndex)
		{
			wid_syslog_debug_debug(WID_WTPINFO,"*** this sock has banding access success\n");
			return CW_TRUE;
		}					

		else
		{
			wid_syslog_debug_debug(WID_WTPINFO,"*** this BindingSystemIndex hasn't banding access fail\n");
			return CW_FALSE;
		}
	}
}

CWBool CWParseWTPDescriptor(CWProtocolMessage *msgPtr, int len, CWWTPDescriptor *valPtr) {
	int theOffset, i;
	char *strversion = NULL;
	CWParseMessageElementStart();

	valPtr->maxRadios = CWProtocolRetrieve8(msgPtr);
	//wid_syslog_debug_debug("WTP Descriptor Max Radios: %d", valPtr->maxRadios);
	
	valPtr->radiosInUse = CWProtocolRetrieve8(msgPtr);
	//wid_syslog_debug_debug("WTP Descriptor Active Radios: %d",	valPtr->radiosInUse);
		
	valPtr->encCapabilities	= CWProtocolRetrieve16(msgPtr);					
	//wid_syslog_debug_debug("WTP Descriptor Encryption Capabilities: %d", valPtr->encCapabilities);
		
	valPtr->vendorInfos.vendorInfosCount = 0;
	
	theOffset = msgPtr->offset;
	
	// see how many vendor ID we have in the message
	while((msgPtr->offset-oldOffset) < len) {	// oldOffset stores msgPtr->offset's value at the beginning of this function.
												// See the definition of the CWParseMessageElementStart() macro.
		int tmp;
		CWProtocolRetrieve32(msgPtr); // ID
		CWProtocolRetrieve16(msgPtr); // type
		tmp = CWProtocolRetrieve16(msgPtr); // len
		msgPtr->offset += tmp;
		valPtr->vendorInfos.vendorInfosCount++;
	}
	
	msgPtr->offset = theOffset;
	
	// actually read each vendor ID
	CW_CREATE_ARRAY_ERR(valPtr->vendorInfos.vendorInfos, valPtr->vendorInfos.vendorInfosCount, CWWTPVendorInfoValues,
		return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	
	for(i = 0; i < valPtr->vendorInfos.vendorInfosCount; i++) {
		(valPtr->vendorInfos.vendorInfos)[i].vendorIdentifier = CWProtocolRetrieve32(msgPtr);
		(valPtr->vendorInfos.vendorInfos)[i].type = CWProtocolRetrieve16(msgPtr);																
		(valPtr->vendorInfos.vendorInfos)[i].length = CWProtocolRetrieve16(msgPtr);
				
		//changed by weiay 20080618
		
		if((valPtr->vendorInfos.vendorInfos)[i].type == CW_WTP_HARDWARE_VERSION)//CW_WTP_SOFTWARE_VERSION = 1
		{
			//(valPtr->vendorInfos.vendorInfos)[i].ver = CWProtocolRetrieveRawBytes(msgPtr, (valPtr->vendorInfos.vendorInfos)[i].length);
			strversion = (CWProtocolRetrieveRawBytes(msgPtr, (valPtr->vendorInfos.vendorInfos)[i].length));	

			(valPtr->vendorInfos.vendorInfos)[i].sysver = (unsigned char *)malloc(((valPtr->vendorInfos.vendorInfos)[i].length)+1);
			memset((valPtr->vendorInfos.vendorInfos)[i].sysver,0,(((valPtr->vendorInfos.vendorInfos)[i].length)+1));
			memcpy((valPtr->vendorInfos.vendorInfos)[i].sysver,strversion,(((valPtr->vendorInfos.vendorInfos)[i].length)));
			free(strversion);
			strversion = NULL;
			wid_syslog_debug_debug(WID_WTPINFO,"*** id is:%d sys_version is:%s ***",i,(valPtr->vendorInfos.vendorInfos)[i].sysver);

		}
		else if((valPtr->vendorInfos.vendorInfos)[i].type == CW_WTP_SOFTWARE_VERSION)//CW_WTP_SOFTWARE_VERSION = 1
		{
			//(valPtr->vendorInfos.vendorInfos)[i].ver = CWProtocolRetrieveRawBytes(msgPtr, (valPtr->vendorInfos.vendorInfos)[i].length);
			strversion = (CWProtocolRetrieveRawBytes(msgPtr, (valPtr->vendorInfos.vendorInfos)[i].length));	

			(valPtr->vendorInfos.vendorInfos)[i].ver = (unsigned char *)malloc(((valPtr->vendorInfos.vendorInfos)[i].length)+1);
			memset((valPtr->vendorInfos.vendorInfos)[i].ver,0,(((valPtr->vendorInfos.vendorInfos)[i].length)+1));
			memcpy((valPtr->vendorInfos.vendorInfos)[i].ver,strversion,(((valPtr->vendorInfos.vendorInfos)[i].length)));
			free(strversion);
			strversion = NULL;



			wid_syslog_debug_debug(WID_WTPINFO,"*** id is:%d sw_version is:%s len %d***",i,(valPtr->vendorInfos.vendorInfos)[i].ver, ((valPtr->vendorInfos.vendorInfos)[i].length));

		}
		else
		{
			(valPtr->vendorInfos.vendorInfos)[i].valuePtr = (int*) (CWProtocolRetrieveRawBytes(msgPtr, (valPtr->vendorInfos.vendorInfos)[i].length));
			
			if((valPtr->vendorInfos.vendorInfos)[i].valuePtr == NULL) return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
			
			if((valPtr->vendorInfos.vendorInfos)[i].length == 4) {
				*((valPtr->vendorInfos.vendorInfos)[i].valuePtr) = ntohl(*((valPtr->vendorInfos.vendorInfos)[i].valuePtr));
			}
		}
		
	//wid_syslog_debug_debug("WTP Descriptor Vendor ID: %d", (valPtr->vendorInfos.vendorInfos)[i].vendorIdentifier);
	//wid_syslog_debug_debug("WTP Descriptor Type: %d", (valPtr->vendorInfos.vendorInfos)[i].type);
	//wid_syslog_debug_debug("WTP Descriptor Length: %d", (valPtr->vendorInfos.vendorInfos)[i].length);
	//wid_syslog_debug_debug("WTP Descriptor Value: %d", *((valPtr->vendorInfos.vendorInfos)[i].valuePtr));
	}
	
	CWParseMessageElementEnd();
}

CWBool CWParseWTPFrameTunnelMode(CWProtocolMessage *msgPtr, int len, CWframeTunnelMode *valPtr) {	
	CWParseMessageElementStart();
	
	*valPtr = CWProtocolRetrieve8(msgPtr);					
	//wid_syslog_debug_debug("CW_MSG_ELEMENT_WTP_FRAME_ENCAPSULATION_TYPE: %d", valPtr->frameTunnelMode);
	
	CWParseMessageElementEnd();
}

CWBool CWParseWTPIPv4Address(CWProtocolMessage *msgPtr, int len, CWProtocolJoinRequestValues *valPtr) {
	
	CWParseMessageElementStart();
	
	valPtr->addr.sin_addr.s_addr = htonl(CWProtocolRetrieve32(msgPtr));
	valPtr->addr.sin_family = AF_INET;
	valPtr->addr.sin_port = htons(CW_CONTROL_PORT);
	//wid_syslog_debug_debug("WTP Address: %s", sock_ntop((struct sockaddr*) (&(valPtr->addr)), sizeof(valPtr->addr)));
	
	CWParseMessageElementEnd();
}

CWBool CWParseWTPMACType(CWProtocolMessage *msgPtr, int len, CWMACType *valPtr) {	
	
	CWParseMessageElementStart();
										
	*valPtr = CWProtocolRetrieve8(msgPtr);
	wid_syslog_debug_debug(WID_WTPINFO,"CW_MSG_ELEMENT_WTP_MAC_TYPE: %s",	valPtr/*->MACType*/);
	
	CWParseMessageElementEnd();
}

CWBool CWParseWTPName(CWProtocolMessage *msgPtr, int len, char **valPtr) {	
	
	CWParseMessageElementStart();
	
	*valPtr = CWProtocolRetrieveStr(msgPtr, len);
	if(valPtr == NULL) return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
	wid_syslog_debug_debug(WID_WTPINFO,"WTP Name:%s enter join state", *valPtr);
	
	CWParseMessageElementEnd();
}

CWBool CWParseWTPRebootStatistics (CWProtocolMessage *msgPtr, int len, WTPRebootStatisticsInfo *valPtr)
{
	CWParseMessageElementStart();
	
	valPtr->rebootCount=CWProtocolRetrieve16(msgPtr);
	valPtr->ACInitiatedCount=CWProtocolRetrieve16(msgPtr);
	valPtr->linkFailurerCount=CWProtocolRetrieve16(msgPtr);
	valPtr->SWFailureCount=CWProtocolRetrieve16(msgPtr);
	valPtr->HWFailuireCount=CWProtocolRetrieve16(msgPtr);
	valPtr->otherFailureCount=CWProtocolRetrieve16(msgPtr);
	valPtr->unknownFailureCount=CWProtocolRetrieve16(msgPtr);
	valPtr->lastFailureType=CWProtocolRetrieve8(msgPtr);
	

//	wid_syslog_debug_debug("WTPRebootStat(1): %d - %d - %d", valPtr->rebootCount, valPtr->ACInitiatedCount, valPtr->linkFailurerCount);
//	wid_syslog_debug_debug("WTPRebootStat(2): %d - %d - %d", valPtr->SWFailureCount, valPtr->HWFailuireCount, valPtr->otherFailureCount);
//	wid_syslog_debug_debug("WTPRebootStat(3): %d - %d", valPtr->unknownFailureCount, valPtr->lastFailureType);
	
	
	CWParseMessageElementEnd();
}

CWBool CWParseWTPRadioStatistics(CWProtocolMessage *msgPtr, int len, WTPRadioStatisticsValues *valPtr) 
{
	CWParseMessageElementStart();
	
	valPtr->radioID = CWProtocolRetrieve8(msgPtr);
	valPtr->WTPRadioStatistics.lastFailureType = CWProtocolRetrieve8(msgPtr);
	valPtr->WTPRadioStatistics.resetCount = CWProtocolRetrieve16(msgPtr);
	valPtr->WTPRadioStatistics.SWFailureCount = CWProtocolRetrieve16(msgPtr);
	valPtr->WTPRadioStatistics.HWFailuireCount = CWProtocolRetrieve16(msgPtr);
	valPtr->WTPRadioStatistics.otherFailureCount = CWProtocolRetrieve16(msgPtr);
	valPtr->WTPRadioStatistics.unknownFailureCount = CWProtocolRetrieve16(msgPtr);
	valPtr->WTPRadioStatistics.configUpdateCount = CWProtocolRetrieve16(msgPtr);
	valPtr->WTPRadioStatistics.channelChangeCount = CWProtocolRetrieve16(msgPtr);
	valPtr->WTPRadioStatistics.bandChangeCount = CWProtocolRetrieve16(msgPtr);
	valPtr->WTPRadioStatistics.currentNoiseFloor = CWProtocolRetrieve16(msgPtr);
	

//	wid_syslog_debug_debug("WTPRadioStatistics of radio: \"%d\"", valPtr->radioID);
//	wid_syslog_debug_debug("WTPRadioStatistics(1): %d - %d - %d", valPtr->WTPRadioStatistics.lastFailureType, valPtr->WTPRadioStatistics.resetCount, valPtr->WTPRadioStatistics.SWFailureCount);
//	wid_syslog_debug_debug("WTPRadioStatistics(2): %d - %d - %d", valPtr->WTPRadioStatistics.HWFailuireCount, valPtr->WTPRadioStatistics.otherFailureCount, valPtr->WTPRadioStatistics.unknownFailureCount);
//	wid_syslog_debug_debug("WTPRadioStatistics(3): %d - %d - %d - %d", valPtr->WTPRadioStatistics.configUpdateCount, valPtr->WTPRadioStatistics.channelChangeCount, valPtr->WTPRadioStatistics.bandChangeCount, valPtr->WTPRadioStatistics.currentNoiseFloor);

	CWParseMessageElementEnd();
}

CWBool CWParseWTPOperationalStatistics(CWProtocolMessage *msgPtr, int len, WTPOperationalStatisticsValues *valPtr) 
{
	CWParseMessageElementStart();
	valPtr->eth_count = 1;
	valPtr->eth_rate = 100;
	
	memset(valPtr->cpuType,0,WTP_TYPE_DEFAULT_LEN);
	memcpy(valPtr->cpuType,"soc",3);
	memset(valPtr->memType,0,WTP_TYPE_DEFAULT_LEN);
	memcpy(valPtr->memType,"flash",5);
	memset(valPtr->flashType,0,WTP_TYPE_DEFAULT_LEN);
	memcpy(valPtr->flashType,"flash",5);/*wuwl add.when wtp into run ,but ap(old ap) didn't support report cpu,mem,flash type ,display this*/

	valPtr->radioID = CWProtocolRetrieve8(msgPtr);
	valPtr->TxQueueLevel = CWProtocolRetrieve8(msgPtr);
	valPtr->wirelessLinkFramesPerSec = CWProtocolRetrieve16(msgPtr);
	valPtr->ElectrifyRegisterCircle = CWProtocolRetrieve16(msgPtr);
	valPtr->ColdStart = CWProtocolRetrieve8(msgPtr);

	if(len > 7)
	{
		valPtr->ipmask = CWProtocolRetrieve32(msgPtr);
		valPtr->ipgateway = CWProtocolRetrieve32(msgPtr);
		valPtr->ipdnsfirst = CWProtocolRetrieve32(msgPtr);
		valPtr->ipdnssecend = CWProtocolRetrieve32(msgPtr);

		printf("%u %u %u %u\n",valPtr->ipmask,valPtr->ipgateway,valPtr->ipdnsfirst,valPtr->ipdnssecend);
	}
	if(len > 23)
	{
	
		unsigned char* cpuType = (unsigned char*)CWProtocolRetrieveStr(msgPtr,32);
		memcpy(valPtr->cpuType, cpuType, 32);
		printf("cpuType %s\n",cpuType);
		free(cpuType);
		cpuType = NULL;
		printf("cpuType %s\n",valPtr->cpuType);

		unsigned char* flashType = (unsigned char*)CWProtocolRetrieveStr(msgPtr,32);
		memcpy(valPtr->flashType, flashType, 32);
		printf("flashType  %s\n",flashType);
		free(flashType);
		flashType = NULL;
		printf("flashType  %s\n",valPtr->flashType);

		//valPtr->flashSize = CWProtocolRetrieve16(msgPtr);

		unsigned char* memType = (unsigned char*)CWProtocolRetrieveStr(msgPtr,32);
		memcpy(valPtr->memType, memType, 32);
		printf("memType %s\n",memType);
		free(memType);
		memType = NULL;
		//valPtr->memSize = CWProtocolRetrieve16(msgPtr);
		printf("memType %s\n",valPtr->memType);
	}
	if(len > 35)
	{
		//valPtr->eth_count = 1;
		valPtr->eth_count = CWProtocolRetrieve8(msgPtr);
		printf("eth_count %d\n",valPtr->eth_count);
		/*fengwenchao add 20120330 for autelan-2882*/
		if(valPtr->eth_count > AP_ETH_IF_NUM)
		{
			valPtr->eth_count =1;
			wid_syslog_err("%s ap send eth num error\n",__func__);
			return CW_FALSE;
		}
		/*fengwenchao add end*/
		//valPtr->eth_rate = 100;
		valPtr->eth_rate  = CWProtocolRetrieve32(msgPtr);
		printf("eth_rate %d\n",valPtr->eth_rate);
	}
	//wid_syslog_debug_debug("WTPOperationalStatistics of radio \"%d\": %d - %d", valPtr->radioID, valPtr->TxQueueLevel, valPtr->wirelessLinkFramesPerSec);

	CWParseMessageElementEnd();
}


CWBool CWParseMsgElemDecryptErrorReport(CWProtocolMessage *msgPtr, int len, CWDecryptErrorReportValues *valPtr) 
{
	CWParseMessageElementStart();
	valPtr->ID = CWProtocolRetrieve8(msgPtr);
	valPtr->numEntries = CWProtocolRetrieve8(msgPtr);

	valPtr->length = CWProtocolRetrieve8(msgPtr);

	valPtr->decryptErrorMACAddressList = NULL;
	if((valPtr->numEntries) > 0)
	{
		CW_CREATE_ARRAY_ERR(valPtr->decryptErrorMACAddressList, valPtr->numEntries, CWMACAddress,  return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
		int size=sizeof(CWMACAddress)*(valPtr->numEntries);
		char *ptr = CWProtocolRetrieveRawBytes(msgPtr, size);
		CW_COPY_MEMORY(valPtr->decryptErrorMACAddressList, ptr, size);
		if(ptr){
			free(ptr);
			ptr = NULL;
		}
		//CW_COPY_MEMORY(valPtr->decryptErrorMACAddressList, CWProtocolRetrieveRawBytes(msgPtr, size), size);
		//valPtr->decryptErrorMACAddressList =(unsigned char*) CWProtocolRetrieveRawBytes(msgPtr, sizeof(CWMACAddress)*(valPtr->numEntries));
		//CW_COPY_MEMORY(&((valPtr->ACIPv6List)[i]), CWProtocolRetrieveRawBytes(msgPtr, 16), 16);
		/*	
		int j;
		for (j=0;j<(sizeof(CWMACAddress)*(valPtr->numEntries)); j++)
		wid_syslog_debug_debug("##(%d/6) = %d", j%6, (valPtr->decryptErrorMACAddressList)[j/6][j%6]);
		*/
	}
	

	//wid_syslog_debug_debug("Radio Decrypt Error Report of radio \"%d\": %d", valPtr->ID, valPtr->numEntries);
		
	CWParseMessageElementEnd();
}

/*
CWBool CWParseWTPRadioInfo(CWPr<otocolMessage *msgPtr, int len, CWRadiosInformation *valPtr, int radioIndex) {	
	CWParseMessageElementStart();

	(valPtr->radios)[radioIndex].ID = CWProtocolRetrieve8(msgPtr);
	(valPtr->radios)[radioIndex].type = CWProtocolRetrieve32(msgPtr);
	wid_syslog_debug_debug("WTP Radio info: %d %d ", (valPtr->radios)[radioIndex].ID, (valPtr->radios)[radioIndex].type);
	
	CWParseMessageElementEnd();
}
*/
/*fengwenchao add for GM-3 ,20111121*/
CWBool CWParselinkquality(CWProtocolMessage *msgPtr, int len, int WTPIndex)
{
	wid_syslog_debug_debug(WID_WTPINFO,"*** CWParseLinkQuality ***\n");
	wid_syslog_debug_debug(WID_WTPINFO,"*** len:%d ***\n",len);

	unsigned int heart_time_delay = 0;
	unsigned int heart_transfer_pkt = 0;
	unsigned int heart_lose_pkt = 0;
	msgPtr->offset +=3;//skip 2 byte : length , 1 byte :resev
	heart_time_delay = CWProtocolRetrieve32(msgPtr);
	heart_transfer_pkt = CWProtocolRetrieve32(msgPtr);
	heart_lose_pkt = CWProtocolRetrieve32(msgPtr);
	AC_WTP[WTPIndex]->heart_time.heart_time_delay = heart_time_delay;
	AC_WTP[WTPIndex]->heart_time.heart_transfer_pkt = heart_transfer_pkt;
	AC_WTP[WTPIndex]->heart_time.heart_lose_pkt = heart_lose_pkt;

	wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo  wtp %d	heart_time_delay = %d \n",WTPIndex,AC_WTP[WTPIndex]->heart_time.heart_time_delay);
	wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo  wtp %d	heart_transfer_pkt = %d \n",WTPIndex,AC_WTP[WTPIndex]->heart_time.heart_transfer_pkt); 
	wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo  wtp %d	heart_lose_pkt = %d \n",WTPIndex,AC_WTP[WTPIndex]->heart_time.heart_lose_pkt); 	

	return CW_TRUE;
}

/*fengwenchao add end*/
CWBool CWParseAPStatisInfo(CWProtocolMessage *msgPtr, int len, int WTPIndex)
{
	wid_syslog_debug_debug(WID_WTPINFO,"*** CWParseAPStatisInfo ***\n");
	wid_syslog_debug_debug(WID_WTPINFO,"*** len:%d ***\n",len);

	int i=0;
	unsigned char count = 0;
	//unsigned int rx_mgmt = 0;
	//unsigned int tx_mgmt = 0;
	unsigned long long rx_bytes_ull = 0;
	unsigned long long tx_bytes_ull = 0;
	unsigned int rx_pkt_mgmt = 0;	//xiaodawei add, rx mgmt pkts, 20110225
	unsigned int tx_pkt_mgmt = 0;	//xiaodawei add, tx mgmt pkts, 20110225
	count = CWProtocolRetrieve8(msgPtr);
	wid_syslog_debug_debug(WID_WTPINFO,"*** count:%d ***\n",count);

	if(count > TOTAL_AP_IF_NUM)
	{
		(msgPtr->offset) += (len-1);
		return CW_TRUE;
	}
	wid_apstatsinfo_init(WTPIndex);		//xiaodawei add for apstatsinfo init, 20110107
	for(i=0; ((i<count)&&(i<TOTAL_AP_IF_NUM)); i++)
	{
		AC_WTP[WTPIndex]->apstatsinfo[i].type = CWProtocolRetrieve8(msgPtr);
		AC_WTP[WTPIndex]->apstatsinfo[i].radioId = CWProtocolRetrieve8(msgPtr);
		AC_WTP[WTPIndex]->apstatsinfo[i].wlanId = CWProtocolRetrieve8(msgPtr);
		
		unsigned char* mac = (unsigned char*)CWProtocolRetrieveStr(msgPtr,6);
		memcpy(AC_WTP[WTPIndex]->apstatsinfo[i].mac, mac, 6);
		free(mac);
		mac = NULL;

		AC_WTP[WTPIndex]->apstatsinfo[i].rx_pkt_data = CWProtocolRetrieve32(msgPtr);	//xiaodawei modify, ap report data pkts not all pkts, 20110225
		AC_WTP[WTPIndex]->apstatsinfo[i].tx_pkt_data = CWProtocolRetrieve32(msgPtr);	//xiaodawei modify, 20110225
		
		AC_WTP[WTPIndex]->apstatsinfo[i].rx_errors = CWProtocolRetrieve32(msgPtr);
		AC_WTP[WTPIndex]->apstatsinfo[i].tx_errors = CWProtocolRetrieve32(msgPtr);
		
		AC_WTP[WTPIndex]->apstatsinfo[i].rx_bytes = (unsigned long long)CWProtocolRetrieve32(msgPtr);
		AC_WTP[WTPIndex]->apstatsinfo[i].tx_bytes = (unsigned long long)CWProtocolRetrieve32(msgPtr);
		
		AC_WTP[WTPIndex]->apstatsinfo[i].rx_bytes = AC_WTP[WTPIndex]->apstatsinfo[i].rx_bytes*1024;
		AC_WTP[WTPIndex]->apstatsinfo[i].tx_bytes = AC_WTP[WTPIndex]->apstatsinfo[i].tx_bytes*1024;

		if(AC_WTP[WTPIndex]->apstatsinfo[i].type == 2){
			
			AC_WTP[WTPIndex]->rx_bytes = AC_WTP[WTPIndex]->rx_bytes_before + (unsigned long long)(AC_WTP[WTPIndex]->apstatsinfo[i].rx_bytes);
			AC_WTP[WTPIndex]->tx_bytes = AC_WTP[WTPIndex]->tx_bytes_before + (unsigned long long)(AC_WTP[WTPIndex]->apstatsinfo[i].tx_bytes);
			wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo type:%d(0-ath,1-eth,2-wifi) rx_bytes %llu \n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].rx_bytes);
			wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo type:%d(0-ath,1-eth,2-wifi) tx_bytes %llu \n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].tx_bytes);
		}

		AC_WTP[WTPIndex]->apstatsinfo[i].rx_rate = CWProtocolRetrieve32(msgPtr);
		AC_WTP[WTPIndex]->apstatsinfo[i].tx_rate = CWProtocolRetrieve32(msgPtr);
		
		AC_WTP[WTPIndex]->apstatsinfo[i].ast_rx_crcerr = CWProtocolRetrieve32(msgPtr);
		AC_WTP[WTPIndex]->apstatsinfo[i].ast_rx_badcrypt = CWProtocolRetrieve32(msgPtr);
		AC_WTP[WTPIndex]->apstatsinfo[i].ast_rx_badmic = CWProtocolRetrieve32(msgPtr);
		AC_WTP[WTPIndex]->apstatsinfo[i].ast_rx_phyerr = CWProtocolRetrieve32(msgPtr);
		wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo type:%d(0-ath,1-eth,2-wifi) rx_pkt_data %u \n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].rx_pkt_data);
		wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo type:%d(0-ath,1-eth,2-wifi) tx_pkt_data %u \n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].tx_pkt_data);
		wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo type:%d(0-ath,1-eth,2-wifi) rx_errors %u \n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].rx_errors);
		wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo type:%d(0-ath,1-eth,2-wifi) tx_errors %u \n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].tx_errors);
		wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo type:%d(0-ath,1-eth,2-wifi) rx_bytes %llu \n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].rx_bytes);
		wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo type:%d(0-ath,1-eth,2-wifi) tx_bytes %llu \n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].tx_bytes);
		wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo type:%d(0-ath,1-eth,2-wifi) rx_rate %u \n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].rx_rate);
		wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo type:%d(0-ath,1-eth,2-wifi) tx_rate %u \n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].tx_rate);
		wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo type:%d(0-ath,1-eth,2-wifi) ast_rx_crcerr %u \n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].ast_rx_crcerr);
		wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo type:%d(0-ath,1-eth,2-wifi) ast_rx_badcrypt %u \n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].ast_rx_badcrypt);
		wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo type:%d(0-ath,1-eth,2-wifi) ast_rx_badmic %u \n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].ast_rx_badmic);
		wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo type:%d(0-ath,1-eth,2-wifi) ast_rx_phyerr %u \n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].ast_rx_phyerr);
		AC_WTP[WTPIndex]->apstatsinfo[i].rx_packets = AC_WTP[WTPIndex]->apstatsinfo[i].rx_pkt_data; //xiaodawei add for old ap(e.g. 1.2.22.1), report only data pkts, 20110225
		AC_WTP[WTPIndex]->apstatsinfo[i].tx_packets = AC_WTP[WTPIndex]->apstatsinfo[i].tx_pkt_data; //xiaodawei add, 20110225
		wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo type:%d(0-ath,1-eth,2-wifi) rx_packets(data pkts only) %u \n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].rx_packets);
		wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo type:%d(0-ath,1-eth,2-wifi) tx_packets(data pkts only) %u \n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].tx_packets);

		if((len-1)/count >= 65)
		{		
			AC_WTP[WTPIndex]->apstatsinfo[i].rx_drop = CWProtocolRetrieve32(msgPtr);
			AC_WTP[WTPIndex]->apstatsinfo[i].tx_drop = CWProtocolRetrieve32(msgPtr);
			wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo type:%d(0-ath,1-eth,2-wifi) rx_drop %u\n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].rx_drop);
			wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo type:%d(0-ath,1-eth,2-wifi) tx_drop %u\n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].tx_drop);
		}
		
		if((len-1)/count > 65){
	
			AC_WTP[WTPIndex]->apstatsinfo[i].rx_pkt_multicast = CWProtocolRetrieve32(msgPtr);
			AC_WTP[WTPIndex]->apstatsinfo[i].tx_pkt_multicast = CWProtocolRetrieve32(msgPtr);
	
			AC_WTP[WTPIndex]->apstatsinfo[i].rx_pkt_broadcast = CWProtocolRetrieve32(msgPtr);
			AC_WTP[WTPIndex]->apstatsinfo[i].tx_pkt_broadcast = CWProtocolRetrieve32(msgPtr);
	
			AC_WTP[WTPIndex]->apstatsinfo[i].rx_pkt_unicast = CWProtocolRetrieve32(msgPtr);
			AC_WTP[WTPIndex]->apstatsinfo[i].tx_pkt_unicast = CWProtocolRetrieve32(msgPtr);
	
			AC_WTP[WTPIndex]->apstatsinfo[i].rx_multicast = CWProtocolRetrieve32(msgPtr);
			AC_WTP[WTPIndex]->apstatsinfo[i].tx_multicast = CWProtocolRetrieve32(msgPtr);
	
			AC_WTP[WTPIndex]->apstatsinfo[i].rx_broadcast = CWProtocolRetrieve32(msgPtr);
			AC_WTP[WTPIndex]->apstatsinfo[i].tx_broadcast = CWProtocolRetrieve32(msgPtr);
	
			AC_WTP[WTPIndex]->apstatsinfo[i].rx_unicast = CWProtocolRetrieve32(msgPtr);
			AC_WTP[WTPIndex]->apstatsinfo[i].tx_unicast = CWProtocolRetrieve32(msgPtr);
	
	
			AC_WTP[WTPIndex]->apstatsinfo[i].rx_retry = CWProtocolRetrieve32(msgPtr);
			AC_WTP[WTPIndex]->apstatsinfo[i].tx_retry = CWProtocolRetrieve32(msgPtr);
			AC_WTP[WTPIndex]->apstatsinfo[i].rx_pkt_retry = CWProtocolRetrieve32(msgPtr);
			AC_WTP[WTPIndex]->apstatsinfo[i].tx_pkt_retry = CWProtocolRetrieve32(msgPtr);
			wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo type:%d(0-ath,1-eth,2-wifi) rx_pkt_multicast %u\n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].rx_pkt_multicast);
			wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo type:%d(0-ath,1-eth,2-wifi) tx_pkt_multicast %u\n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].tx_pkt_multicast);
			wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo type:%d(0-ath,1-eth,2-wifi) rx_pkt_broadcast %u\n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].rx_pkt_broadcast);
			wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo type:%d(0-ath,1-eth,2-wifi) tx_pkt_broadcast %u\n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].tx_pkt_broadcast);
			wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo type:%d(0-ath,1-eth,2-wifi) rx_pkt_unicast %u\n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].rx_pkt_unicast);
			wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo type:%d(0-ath,1-eth,2-wifi) tx_pkt_unicast %u\n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].tx_pkt_unicast);
			wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo type:%d(0-ath,1-eth,2-wifi) rx_multicast %u\n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].rx_multicast);
			wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo type:%d(0-ath,1-eth,2-wifi) tx_multicast %u\n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].tx_multicast);
			wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo type:%d(0-ath,1-eth,2-wifi) rx_broadcast %u\n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].rx_broadcast);
			wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo type:%d(0-ath,1-eth,2-wifi) tx_broadcast %u\n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].tx_broadcast);
			wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo type:%d(0-ath,1-eth,2-wifi) rx_unicast %u\n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].rx_unicast);
			wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo type:%d(0-ath,1-eth,2-wifi) tx_unicast %u\n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].tx_unicast);
			wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo type:%d(0-ath,1-eth,2-wifi) rx_retry %u\n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].rx_retry);
			wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo type:%d(0-ath,1-eth,2-wifi) tx_retry %u\n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].tx_retry);
			wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo type:%d(0-ath,1-eth,2-wifi) rx_pkt_retry %u\n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].rx_pkt_retry);
			wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo type:%d(0-ath,1-eth,2-wifi) tx_pkt_retry %u\n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].tx_pkt_retry);
		}
		if((len-1)/count > 129){
			
			rx_pkt_mgmt = CWProtocolRetrieve32(msgPtr);	//xiaodawei modify, 20110225
			tx_pkt_mgmt = CWProtocolRetrieve32(msgPtr);	//xiaodawei modify, 20110225
			AC_WTP[WTPIndex]->apstatsinfo[i].rx_pkt_mgmt = rx_pkt_mgmt;	//xiaodawei add, 20110225
			AC_WTP[WTPIndex]->apstatsinfo[i].tx_pkt_mgmt = tx_pkt_mgmt;	//xiaodawei add, 20110225
			
			wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo type:%d(0-ath,1-eth,2-wifi)	rx_pkt_mgmt %u \n",AC_WTP[WTPIndex]->apstatsinfo[i].type,rx_pkt_mgmt);
			wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo type:%d(0-ath,1-eth,2-wifi)	tx_pkt_mgmt %u \n",AC_WTP[WTPIndex]->apstatsinfo[i].type,tx_pkt_mgmt);

			AC_WTP[WTPIndex]->apstatsinfo[i].rx_packets = rx_pkt_mgmt + AC_WTP[WTPIndex]->apstatsinfo[i].rx_pkt_data;	//xiaodawei add, 20110225
			AC_WTP[WTPIndex]->apstatsinfo[i].tx_packets = rx_pkt_mgmt + AC_WTP[WTPIndex]->apstatsinfo[i].tx_pkt_data;	//xiaodawei add, 20110225
			wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo type:%d(0-ath,1-eth,2-wifi)	after##########rx_packets %u \n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].rx_packets);
			wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo type:%d(0-ath,1-eth,2-wifi)	after##########tx_packets %u \n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].tx_packets);
		}
		if((len-1)/count > 137){
			
			CWProtocolRetrieve64(msgPtr,&rx_bytes_ull);
			CWProtocolRetrieve64(msgPtr,&tx_bytes_ull);
			AC_WTP[WTPIndex]->apstatsinfo[i].rx_bytes = rx_bytes_ull;
			AC_WTP[WTPIndex]->apstatsinfo[i].tx_bytes = tx_bytes_ull;

			if(AC_WTP[WTPIndex]->apstatsinfo[i].type == 2){
				
				AC_WTP[WTPIndex]->rx_bytes = AC_WTP[WTPIndex]->rx_bytes_before + (AC_WTP[WTPIndex]->apstatsinfo[i].rx_bytes);
				AC_WTP[WTPIndex]->tx_bytes = AC_WTP[WTPIndex]->tx_bytes_before + (AC_WTP[WTPIndex]->apstatsinfo[i].tx_bytes);
				wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo type:%d(0-ath,1-eth,2-wifi) (len-1)/count > 137 \n");			
				wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo type:%d(0-ath,1-eth,2-wifi) rx_bytes %llu \n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].rx_bytes);
				wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo type:%d(0-ath,1-eth,2-wifi) tx_bytes %llu \n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].tx_bytes);

			}
			wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo type:%d(0-ath,1-eth,2-wifi)	rx_bytes_ull :%llu\n",AC_WTP[WTPIndex]->apstatsinfo[i].type,rx_bytes_ull);
			wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo type:%d(0-ath,1-eth,2-wifi)	tx_bytes_ull :%llu\n",AC_WTP[WTPIndex]->apstatsinfo[i].type,tx_bytes_ull);
		}
	}

	return CW_TRUE;
}
CWBool CWParseAPStatisInfo_v2(CWProtocolMessage *msgPtr, int len,char *valPtr, int WTPIndex)
{
	wid_syslog_debug_debug(WID_DEFAULT,"*** CWParseAPStatisInfo_v2 ***\n");
	wid_syslog_debug_debug(WID_DEFAULT,"*** len:%d ***\n",len);
	int i=0;
	unsigned char count = 0;
	//unsigned long long rx_tmp = 0;
	//unsigned long long tx_tmp = 0;
	unsigned long long rx_bytes_ull = 0;
	unsigned long long tx_bytes_ull = 0;
	unsigned long long rx_mgmt=0;
	unsigned long long tx_mgmt=0;
	unsigned int rx_pkt_mgmt = 0;
	unsigned int tx_pkt_mgmt = 0;
	unsigned long long rx_sum_bytes = 0;        /* zhangshu add 2010-09-13 */
	unsigned long long tx_sum_bytes = 0;        /* zhangshu add 2010-09-13 */
	unsigned long long rx_unicast = 0;        // zhangshu add 64bit rx_unicast 2010-09-13
	unsigned long long tx_unicast = 0;        // zhangshu add 64bit tx_unicast 2010-09-13
	unsigned int rx_pkt_control; //zhaoruijia,     20100907，add control packet
	unsigned int tx_pkt_control; //zhaoruijia，20100907，add control packet
	unsigned int rx_errors_frames = 0;   //zhangshu add for error frames, 2010-09-26
	unsigned int is_refuse_lowrssi = 0;  //fengwenchao add for chinamobile-177,20111122
	//CWParseMessageElementStart();
	count = CWProtocolRetrieve8(msgPtr);
	wid_syslog_debug_debug(WID_DEFAULT,"*** count:%d ***\n",count);

	if(count > TOTAL_AP_IF_NUM)
	{
		(msgPtr->offset) += (len-1);
		return CW_TRUE;
	}
	wid_apstatsinfo_init(WTPIndex);		//xiaodawei add for apstatsinfo init, 20110107
	for(i=0; ((i<count)&&(i<TOTAL_AP_IF_NUM)); i++)
	{
		AC_WTP[WTPIndex]->apstatsinfo[i].type = CWProtocolRetrieve8(msgPtr);
		AC_WTP[WTPIndex]->apstatsinfo[i].radioId = CWProtocolRetrieve8(msgPtr);
		AC_WTP[WTPIndex]->apstatsinfo[i].wlanId = CWProtocolRetrieve8(msgPtr);
		
		unsigned char* mac = (unsigned char*)CWProtocolRetrieveStr(msgPtr,6);
		memcpy(AC_WTP[WTPIndex]->apstatsinfo[i].mac, mac, 6);
		free(mac);
		mac = NULL;

		AC_WTP[WTPIndex]->apstatsinfo[i].rx_pkt_data = CWProtocolRetrieve32(msgPtr);    //book modify
		AC_WTP[WTPIndex]->apstatsinfo[i].tx_pkt_data = CWProtocolRetrieve32(msgPtr);    //book modify
		
		AC_WTP[WTPIndex]->apstatsinfo[i].rx_errors = CWProtocolRetrieve32(msgPtr);
		AC_WTP[WTPIndex]->apstatsinfo[i].tx_errors = CWProtocolRetrieve32(msgPtr);
		
		AC_WTP[WTPIndex]->apstatsinfo[i].rx_bytes = (unsigned long long)CWProtocolRetrieve32(msgPtr);
		AC_WTP[WTPIndex]->apstatsinfo[i].tx_bytes = (unsigned long long)CWProtocolRetrieve32(msgPtr);
		
		AC_WTP[WTPIndex]->apstatsinfo[i].rx_bytes = AC_WTP[WTPIndex]->apstatsinfo[i].rx_bytes*1024;
		AC_WTP[WTPIndex]->apstatsinfo[i].tx_bytes = AC_WTP[WTPIndex]->apstatsinfo[i].tx_bytes*1024;
		if(AC_WTP[WTPIndex]->apstatsinfo[i].type == 2){
			
			AC_WTP[WTPIndex]->rx_bytes = AC_WTP[WTPIndex]->rx_bytes_before + (unsigned long long)(AC_WTP[WTPIndex]->apstatsinfo[i].rx_bytes);
			AC_WTP[WTPIndex]->tx_bytes = AC_WTP[WTPIndex]->tx_bytes_before + (unsigned long long)(AC_WTP[WTPIndex]->apstatsinfo[i].tx_bytes);
			wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo type:%d(0-ath,1-eth,2-wifi) rx_bytes %llu \n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].rx_bytes);
			wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo type:%d(0-ath,1-eth,2-wifi) tx_bytes %llu \n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].tx_bytes);

		}
		AC_WTP[WTPIndex]->apstatsinfo[i].rx_rate = CWProtocolRetrieve32(msgPtr);
		AC_WTP[WTPIndex]->apstatsinfo[i].tx_rate = CWProtocolRetrieve32(msgPtr);
		
		AC_WTP[WTPIndex]->apstatsinfo[i].ast_rx_crcerr = CWProtocolRetrieve32(msgPtr);
		AC_WTP[WTPIndex]->apstatsinfo[i].ast_rx_badcrypt = CWProtocolRetrieve32(msgPtr);
		AC_WTP[WTPIndex]->apstatsinfo[i].ast_rx_badmic = CWProtocolRetrieve32(msgPtr);
		AC_WTP[WTPIndex]->apstatsinfo[i].ast_rx_phyerr = CWProtocolRetrieve32(msgPtr);
		wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo_v2 type:%d(0-ath,1-eth,2-wifi) rx_pkt_data %u \n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].rx_pkt_data);
		wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo_v2 type:%d(0-ath,1-eth,2-wifi) tx_pkt_data %u \n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].tx_pkt_data);
		wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo_v2 type:%d(0-ath,1-eth,2-wifi) rx_errors %u \n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].rx_errors);
		wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo_v2 type:%d(0-ath,1-eth,2-wifi) tx_errors %u \n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].tx_errors);
		wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo_v2 type:%d(0-ath,1-eth,2-wifi) rx_bytes %llu \n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].rx_bytes);
		wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo_v2 type:%d(0-ath,1-eth,2-wifi) tx_bytes %llu \n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].tx_bytes);
		wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo_v2 type:%d(0-ath,1-eth,2-wifi) rx_rate %u \n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].rx_rate);
		wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo_v2 type:%d(0-ath,1-eth,2-wifi) tx_rate %u \n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].tx_rate);
		wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo_v2 type:%d(0-ath,1-eth,2-wifi) ast_rx_crcerr %u \n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].ast_rx_crcerr);
		wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo_v2 type:%d(0-ath,1-eth,2-wifi) ast_rx_badcrypt %u \n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].ast_rx_badcrypt);
		wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo_v2 type:%d(0-ath,1-eth,2-wifi) ast_rx_badmic %u \n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].ast_rx_badmic);
		wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo_v2 type:%d(0-ath,1-eth,2-wifi) ast_rx_phyerr %u \n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].ast_rx_phyerr);
		AC_WTP[WTPIndex]->apstatsinfo[i].rx_packets = AC_WTP[WTPIndex]->apstatsinfo[i].rx_pkt_data;	//xiaodawei add for old ap(e.g. 1.2.22.1), report only data pkts, 20110225
		AC_WTP[WTPIndex]->apstatsinfo[i].tx_packets = AC_WTP[WTPIndex]->apstatsinfo[i].tx_pkt_data;	//xiaodawei add, 20110225
		wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo_v2 type:%d(0-ath,1-eth,2-wifi) rx_packets(data pkts only) %u \n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].rx_packets);
		wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo_v2 type:%d(0-ath,1-eth,2-wifi) tx_packets(data pkts only) %u \n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].tx_packets);

		if((len-1)/count >= 65)
		{
			AC_WTP[WTPIndex]->apstatsinfo[i].rx_drop = CWProtocolRetrieve32(msgPtr);
			AC_WTP[WTPIndex]->apstatsinfo[i].tx_drop = CWProtocolRetrieve32(msgPtr);
			wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo_v2 type:%d(0-ath,1-eth,2-wifi) rx_drop %u\n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].rx_drop);
			wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo_v2 type:%d(0-ath,1-eth,2-wifi) tx_drop %u\n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].tx_drop);
		}

		if((len-1)/count > 65){

			AC_WTP[WTPIndex]->apstatsinfo[i].rx_pkt_multicast = CWProtocolRetrieve32(msgPtr);
			AC_WTP[WTPIndex]->apstatsinfo[i].tx_pkt_multicast = CWProtocolRetrieve32(msgPtr);

			AC_WTP[WTPIndex]->apstatsinfo[i].rx_pkt_broadcast = CWProtocolRetrieve32(msgPtr);
			AC_WTP[WTPIndex]->apstatsinfo[i].tx_pkt_broadcast = CWProtocolRetrieve32(msgPtr);

			AC_WTP[WTPIndex]->apstatsinfo[i].rx_pkt_unicast = CWProtocolRetrieve32(msgPtr);
			AC_WTP[WTPIndex]->apstatsinfo[i].tx_pkt_unicast = CWProtocolRetrieve32(msgPtr);

			AC_WTP[WTPIndex]->apstatsinfo[i].rx_multicast = CWProtocolRetrieve32(msgPtr);
			AC_WTP[WTPIndex]->apstatsinfo[i].tx_multicast = CWProtocolRetrieve32(msgPtr);

			AC_WTP[WTPIndex]->apstatsinfo[i].rx_broadcast = CWProtocolRetrieve32(msgPtr);
			AC_WTP[WTPIndex]->apstatsinfo[i].tx_broadcast = CWProtocolRetrieve32(msgPtr);

			AC_WTP[WTPIndex]->apstatsinfo[i].rx_unicast = CWProtocolRetrieve32(msgPtr);
			AC_WTP[WTPIndex]->apstatsinfo[i].tx_unicast = CWProtocolRetrieve32(msgPtr);


			AC_WTP[WTPIndex]->apstatsinfo[i].rx_retry = CWProtocolRetrieve32(msgPtr);
			AC_WTP[WTPIndex]->apstatsinfo[i].tx_retry = CWProtocolRetrieve32(msgPtr);
			AC_WTP[WTPIndex]->apstatsinfo[i].rx_pkt_retry = CWProtocolRetrieve32(msgPtr);
			AC_WTP[WTPIndex]->apstatsinfo[i].tx_pkt_retry = CWProtocolRetrieve32(msgPtr);
			wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo_v2 type:%d(0-ath,1-eth,2-wifi) rx_pkt_multicast %u\n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].rx_pkt_multicast);
			wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo_v2 type:%d(0-ath,1-eth,2-wifi) tx_pkt_multicast %u\n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].tx_pkt_multicast);
			wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo_v2 type:%d(0-ath,1-eth,2-wifi) rx_pkt_broadcast %u\n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].rx_pkt_broadcast);
			wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo_v2 type:%d(0-ath,1-eth,2-wifi) tx_pkt_broadcast %u\n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].tx_pkt_broadcast);
			wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo_v2 type:%d(0-ath,1-eth,2-wifi) rx_pkt_unicast %u\n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].rx_pkt_unicast);
			wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo_v2 type:%d(0-ath,1-eth,2-wifi) tx_pkt_unicast %u\n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].tx_pkt_unicast);
			wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo_v2 type:%d(0-ath,1-eth,2-wifi) rx_multicast %u\n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].rx_multicast);
			wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo_v2 type:%d(0-ath,1-eth,2-wifi) tx_multicast %u\n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].tx_multicast);
			wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo_v2 type:%d(0-ath,1-eth,2-wifi) rx_broadcast %u\n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].rx_broadcast);
			wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo_v2 type:%d(0-ath,1-eth,2-wifi) tx_broadcast %u\n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].tx_broadcast);
			wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo_v2 type:%d(0-ath,1-eth,2-wifi) rx_unicast %u\n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].rx_unicast);
			wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo_v2 type:%d(0-ath,1-eth,2-wifi) tx_unicast %u\n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].tx_unicast);
			wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo_v2 type:%d(0-ath,1-eth,2-wifi) rx_retry %u\n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].rx_retry);
			wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo_v2 type:%d(0-ath,1-eth,2-wifi) tx_retry %u\n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].tx_retry);
			wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo_v2 type:%d(0-ath,1-eth,2-wifi) rx_pkt_retry %u\n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].rx_pkt_retry);
			wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo_v2 type:%d(0-ath,1-eth,2-wifi) tx_pkt_retry %u\n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].tx_pkt_retry);
		}
		
		if((len-1)/count > 129){
			
			rx_pkt_mgmt = CWProtocolRetrieve32(msgPtr);
			tx_pkt_mgmt = CWProtocolRetrieve32(msgPtr);
			
			AC_WTP[WTPIndex]->apstatsinfo[i].rx_pkt_mgmt = rx_pkt_mgmt;  //fengwenchao add 20110104
			AC_WTP[WTPIndex]->apstatsinfo[i].tx_pkt_mgmt = tx_pkt_mgmt;  //fengwenchao add 20110104
			wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo_v2 type:%d(0-ath,1-eth,2-wifi) rx_pkt_mgmt %u\n",AC_WTP[WTPIndex]->apstatsinfo[i].type,rx_pkt_mgmt);
			wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo_v2 type:%d(0-ath,1-eth,2-wifi) tx_pkt_mgmt %u\n",AC_WTP[WTPIndex]->apstatsinfo[i].type,tx_pkt_mgmt);

			AC_WTP[WTPIndex]->apstatsinfo[i].rx_packets = rx_pkt_mgmt + AC_WTP[WTPIndex]->apstatsinfo[i].rx_pkt_data;   //book modify
			AC_WTP[WTPIndex]->apstatsinfo[i].tx_packets = tx_pkt_mgmt + AC_WTP[WTPIndex]->apstatsinfo[i].tx_pkt_data;   //book modify
			wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo_v2 type:%d(0-ath,1-eth,2-wifi) rx_packets %u\n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].rx_packets);
			wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo_v2 type:%d(0-ath,1-eth,2-wifi) tx_packets %u\n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].tx_packets);
			
		}
		if((len-1)/count > 137){
			
			CWProtocolRetrieve64(msgPtr,&rx_bytes_ull);
			CWProtocolRetrieve64(msgPtr,&tx_bytes_ull);
			AC_WTP[WTPIndex]->apstatsinfo[i].rx_bytes = rx_bytes_ull;
			AC_WTP[WTPIndex]->apstatsinfo[i].tx_bytes = tx_bytes_ull;

            if(AC_WTP[WTPIndex]->apstatsinfo[i].type == 2){//book addback		
				AC_WTP[WTPIndex]->rx_bytes = AC_WTP[WTPIndex]->rx_bytes_before + (AC_WTP[WTPIndex]->apstatsinfo[i].rx_bytes);
				AC_WTP[WTPIndex]->tx_bytes = AC_WTP[WTPIndex]->tx_bytes_before + (AC_WTP[WTPIndex]->apstatsinfo[i].tx_bytes);
				wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo type:%d(0-ath,1-eth,2-wifi) (len-1)/count > 137 \n");			
				wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo type:%d(0-ath,1-eth,2-wifi) rx_bytes %llu \n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].rx_bytes);
				wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo type:%d(0-ath,1-eth,2-wifi) tx_bytes %llu \n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].tx_bytes);
			}
			
			wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo_v2 type:%d(0-ath,1-eth,2-wifi)  rx_bytes_ull:%llu\n",AC_WTP[WTPIndex]->apstatsinfo[i].type, rx_bytes_ull);
			wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo_v2 type:%d(0-ath,1-eth,2-wifi)  tx_bytes_ull :%llu\n",AC_WTP[WTPIndex]->apstatsinfo[i].type, tx_bytes_ull);
		}
		if((len-1)/count > 153){
			CWProtocolRetrieve64(msgPtr,&rx_mgmt);
			CWProtocolRetrieve64(msgPtr,&tx_mgmt);
			AC_WTP[WTPIndex]->apstatsinfo[i].rx_mgmt=rx_mgmt;
			AC_WTP[WTPIndex]->apstatsinfo[i].tx_mgmt=tx_mgmt;
			wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo_v2 type:%d(0-ath,1-eth,2-wifi)   rx_mgmt :%llu\n",AC_WTP[WTPIndex]->apstatsinfo[i].type,rx_mgmt);
			wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo_v2 type:%d(0-ath,1-eth,2-wifi)   tx_mgmt :%llu\n",AC_WTP[WTPIndex]->apstatsinfo[i].type,tx_mgmt);

		}
		/* zhangshu append 2010-09-13 */
		if((len-1)/count > 169){
			CWProtocolRetrieve64(msgPtr,&rx_sum_bytes);
			CWProtocolRetrieve64(msgPtr,&tx_sum_bytes);
			AC_WTP[WTPIndex]->apstatsinfo[i].rx_sum_bytes=rx_sum_bytes;
			AC_WTP[WTPIndex]->apstatsinfo[i].tx_sum_bytes=tx_sum_bytes;
			if(AC_WTP[WTPIndex]->apstatsinfo[i].type == 2){
				
				AC_WTP[WTPIndex]->rx_bytes = AC_WTP[WTPIndex]->rx_bytes_before + (AC_WTP[WTPIndex]->apstatsinfo[i].rx_sum_bytes);
				AC_WTP[WTPIndex]->tx_bytes = AC_WTP[WTPIndex]->tx_bytes_before + (AC_WTP[WTPIndex]->apstatsinfo[i].tx_sum_bytes);
				wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo type:%d(0-ath,1-eth,2-wifi) (len-1)/count > 169 \n");			
				wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo type:%d(0-ath,1-eth,2-wifi) rx_bytes %llu \n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].rx_bytes);
				wid_syslog_debug_debug(WID_WTPINFO,"CWParseAPStatisInfo type:%d(0-ath,1-eth,2-wifi) tx_bytes %llu \n",AC_WTP[WTPIndex]->apstatsinfo[i].type,AC_WTP[WTPIndex]->apstatsinfo[i].tx_bytes);

			}
			wid_syslog_debug_debug(WID_WTPINFO,"type:%d(0-ath,1-eth,2-wifi)  rx_sum_bytes :%llu\n",AC_WTP[WTPIndex]->apstatsinfo[i].type, rx_sum_bytes);
			wid_syslog_debug_debug(WID_WTPINFO,"type:%d(0-ath,1-eth,2-wifi)  rx_sum_bytes :%llu\n",AC_WTP[WTPIndex]->apstatsinfo[i].type, tx_sum_bytes);

		}
		/* zhangshu add 2010-09-13 */
		if((len-1)/count > 185){
			CWProtocolRetrieve64(msgPtr,&rx_unicast);
			CWProtocolRetrieve64(msgPtr,&tx_unicast);
			AC_WTP[WTPIndex]->apstatsinfo[i].rx_unicast = rx_unicast;
			AC_WTP[WTPIndex]->apstatsinfo[i].tx_unicast = tx_unicast;
			wid_syslog_debug_debug(WID_WTPINFO,"type:%d(0-ath,1-eth,2-wifi)  rx_unicast :%llu\n",AC_WTP[WTPIndex]->apstatsinfo[i].type, AC_WTP[WTPIndex]->apstatsinfo[i].rx_unicast);
			wid_syslog_debug_debug(WID_WTPINFO,"type:%d(0-ath,1-eth,2-wifi)  tx_unicast :%llu\n",AC_WTP[WTPIndex]->apstatsinfo[i].type, AC_WTP[WTPIndex]->apstatsinfo[i].tx_unicast);

		}
		/*zhaoruijia,20100913,增加接收，发送控制帧,start*/
		if((len-1)/count > 201){
			rx_pkt_control = CWProtocolRetrieve32(msgPtr);
			tx_pkt_control = CWProtocolRetrieve32(msgPtr);
			AC_WTP[WTPIndex]->apstatsinfo[i].rx_pkt_control=rx_pkt_control;
			AC_WTP[WTPIndex]->apstatsinfo[i].tx_pkt_control=tx_pkt_control;
			wid_syslog_debug_debug(WID_WTPINFO,"type:%d(0-ath,1-eth,2-wifi)  control_len = %d",len);
			wid_syslog_debug_debug(WID_WTPINFO,"type:%d(0-ath,1-eth,2-wifi)  rx_pkt_control :%d\n",AC_WTP[WTPIndex]->apstatsinfo[i].type, AC_WTP[WTPIndex]->apstatsinfo[i].rx_pkt_control=rx_pkt_control);
			wid_syslog_debug_debug(WID_WTPINFO,"type:%d(0-ath,1-eth,2-wifi)  tx_pkt_control :%d\n",AC_WTP[WTPIndex]->apstatsinfo[i].type,  AC_WTP[WTPIndex]->apstatsinfo[i].tx_pkt_control=tx_pkt_control);

		}
		/* zhangshu add for error packets and error frames, 2010-09-26 */
		if((len-1)/count > 209){
			rx_errors_frames = CWProtocolRetrieve32(msgPtr);
			AC_WTP[WTPIndex]->apstatsinfo[i].rx_errors_frames = rx_errors_frames;
			wid_syslog_debug_debug(WID_WTPINFO,"type:%d(0-ath,1-eth,2-wifi)  rx_errors_frames :%u\n",AC_WTP[WTPIndex]->apstatsinfo[i].type, AC_WTP[WTPIndex]->apstatsinfo[i].rx_errors_frames);
		}

		/*fengwenchao add for chinamobile-177,20111122*/
		if((len-1)/count > 213)
		{
			is_refuse_lowrssi = CWProtocolRetrieve32(msgPtr);
			AC_WTP[WTPIndex]->apstatsinfo[i].is_refuse_lowrssi = is_refuse_lowrssi;
			wid_syslog_debug_debug(WID_WTPINFO,"type:%d(0-ath,1-eth,2-wifi)  is_refuse_lowrssi :%u\n",AC_WTP[WTPIndex]->apstatsinfo[i].type, AC_WTP[WTPIndex]->apstatsinfo[i].is_refuse_lowrssi);
		}
		/*fengwenchao add end*/

		
	}

	return CW_TRUE;
}
/*zhaoruijia,20100904,transplant ACTIMESYNCHROFAILURE from 1.2omc to 1.3,start*/
CWBool CWParseAP_Ntp_resultcode(CWProtocolMessage *msgPtr, int len,char *valPtr, int WTPIndex)
{
	wid_syslog_debug_debug(WID_DEFAULT,"*** CWParseAP_Ntp_resultcode ***\n");
	wid_syslog_debug_debug(WID_DEFAULT,"*** len:%d ***\n",len);
	printf("*** CWParseAP_Ntp_resultcode ***\n");
	printf("*** len:%d ***\n",len);
	unsigned char resultcode = 0;
	unsigned char reserve = 0;
	resultcode = CWProtocolRetrieve8(msgPtr);
	reserve = CWProtocolRetrieve8(msgPtr);
	reserve = CWProtocolRetrieve8(msgPtr);
	printf("resultcode=%d\n",resultcode);
	if(resultcode != 0){
		if(gtrapflag>=4){//trap
			printf("resultcode=%d,gtrapflag=%d\n",resultcode,gtrapflag);
			wid_dbus_trap_wtp_ap_ACTimeSynchroFailure(WTPIndex,0);
			AC_WTP[WTPIndex]->ntp_trap_flag = 1;
		}
	}else{
		if(AC_WTP[WTPIndex]->ntp_trap_flag == 1){//trap clear
			wid_dbus_trap_wtp_ap_ACTimeSynchroFailure(WTPIndex,1);
			AC_WTP[WTPIndex]->ntp_trap_flag = 0;
		}
	}
	return CW_TRUE;
}

/*zhaoruijia,20100904,transplant ACTIMESYNCHROFAILURE from 1.2omc to 1.3,end*/

/*wuwl add*/
CWBool CWParseAttack_addr_Redirect(CWProtocolMessage *msgPtr, int len, int WTPIndex)
{
	wid_syslog_debug_debug(WID_DEFAULT,"*** CWParseAttack_addr_RD ***\n");
	wid_syslog_debug_debug(WID_DEFAULT,"*** len:%d ***\n",len);
	printf("*** CWParseAttack_addr_RD ***\n");
	printf("*** len:%d ***\n",len);
	WIDStaWapiInfoList *valPtr = NULL;
	unsigned short rd_sta_count;
	unsigned char radio_id;
	unsigned char wlan_id;
	unsigned char flag;/*1--trap;0--trap clear*/
//	unsigned char sta_mac[6];
	unsigned int reserve = 0;
	unsigned int i = 0;
	unsigned char* stamac = NULL;
	reserve = CWProtocolRetrieve32(msgPtr);
	rd_sta_count = CWProtocolRetrieve16(msgPtr);

	CW_CREATE_OBJECT_ERR(valPtr, WIDStaWapiInfoList, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););	
	memset(valPtr, 0, sizeof(WIDStaWapiInfoList));
	valPtr->WTPID = WTPIndex;
	valPtr->sta_num = rd_sta_count;

	for(i=0;((i<rd_sta_count)&&(i<64));i++){
		radio_id = CWProtocolRetrieve8(msgPtr);
		wlan_id = CWProtocolRetrieve8(msgPtr);
		flag  = CWProtocolRetrieve8(msgPtr);
		stamac = (unsigned char*)CWProtocolRetrieveStr(msgPtr,6);
	
	
		wid_syslog_debug_debug(WID_DEFAULT,"i=%d,rd_sta_count=%d\n",i,rd_sta_count);
		wid_syslog_debug_debug(WID_DEFAULT,"i=%d,radio_id=%d\n",i,radio_id);
		wid_syslog_debug_debug(WID_DEFAULT,"i=%d,wlan_id=%d\n",i,wlan_id);
		wid_syslog_debug_debug(WID_DEFAULT,"i=%d,flag=%d\n",i,flag);
		wid_syslog_debug_debug(WID_DEFAULT,"i=%d,mac:%02X:%02X:%02X:%02X:%02X:%02X \n",i,stamac[0],stamac[1],stamac[2],stamac[3],stamac[4],stamac[5]);
		wid_syslog_debug_debug(WID_DEFAULT,"i=%d,flag=%d,gtrapflag=%d\n",i,flag,gtrapflag);

		if(flag == 1){
			valPtr->StaWapiInfo[i].wapi_trap_flag = flag;
		}else{
			valPtr->StaWapiInfo[i].wapi_trap_flag = 2;
		}
		valPtr->StaWapiInfo[i].RadioId = radio_id;
		valPtr->StaWapiInfo[i].WlanId = wlan_id;
		memset(valPtr->StaWapiInfo[i].mac,0,6);
		memcpy(valPtr->StaWapiInfo[i].mac,stamac,6);
	}
	WidAsdStaWapiInfoUpdate(WTPIndex, valPtr);

	free(stamac);
	stamac = NULL;
	return CW_TRUE;
}

/* zhangshu add for challenge replay, 2010-09-26 */
CWBool CWParseAP_challenge_replay(CWProtocolMessage *msgPtr, int len, int WTPIndex)
{
	wid_syslog_debug_debug(WID_DEFAULT,"*** CWParseAP_challenge_replay ***\n");
	wid_syslog_debug_debug(WID_DEFAULT,"*** len:%d ***\n",len);
	printf("*** CWParseAP_challenge_replay ***\n");
	printf("*** len:%d ***\n",len);
	WIDStaWapiInfoList *valPtr = NULL;
	unsigned short rd_sta_count;
	unsigned char radio_id;
	unsigned char wlan_id;
	unsigned char flag;/*1--trap;0--trap clear*/
	//unsigned char sta_mac[6];
	unsigned int reserve = 0;
	unsigned int i = 0;
	unsigned char* stamac = NULL;
	reserve = CWProtocolRetrieve32(msgPtr);
	rd_sta_count = CWProtocolRetrieve16(msgPtr);

	CW_CREATE_OBJECT_ERR(valPtr, WIDStaWapiInfoList, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););	
	memset(valPtr, 0, sizeof(WIDStaWapiInfoList));
	valPtr->WTPID = WTPIndex;
	valPtr->sta_num = rd_sta_count;

	for(i=0;((i<rd_sta_count)&&(i<64));i++){
		radio_id = CWProtocolRetrieve8(msgPtr);
		wlan_id = CWProtocolRetrieve8(msgPtr);
		flag  = CWProtocolRetrieve8(msgPtr);
		stamac = (unsigned char*)CWProtocolRetrieveStr(msgPtr,6);
	
	
		wid_syslog_debug_debug(WID_DEFAULT,"i=%d,rd_sta_count=%d\n",i,rd_sta_count);
		wid_syslog_debug_debug(WID_DEFAULT,"i=%d,radio_id=%d\n",i,radio_id);
		wid_syslog_debug_debug(WID_DEFAULT,"i=%d,wlan_id=%d\n",i,wlan_id);
		wid_syslog_debug_debug(WID_DEFAULT,"i=%d,flag=%d\n",i,flag);
		wid_syslog_debug_debug(WID_DEFAULT,"i=%d,mac:%02X:%02X:%02X:%02X:%02X:%02X \n",i,stamac[0],stamac[1],stamac[2],stamac[3],stamac[4],stamac[5]);
		wid_syslog_debug_debug(WID_DEFAULT,"i=%d,flag=%d,gtrapflag=%d\n",i,flag,gtrapflag);

		if(flag == 1){
			valPtr->StaWapiInfo[i].wapi_trap_flag = 3;
		}else{
			valPtr->StaWapiInfo[i].wapi_trap_flag = 4;
		}
		valPtr->StaWapiInfo[i].RadioId = radio_id;
		valPtr->StaWapiInfo[i].WlanId = wlan_id;
		memset(valPtr->StaWapiInfo[i].mac,0,6);
		memcpy(valPtr->StaWapiInfo[i].mac,stamac,6);
		if(stamac){
			free(stamac);
			stamac = NULL;
		}
	}
	WidAsdStaWapiInfoUpdate(WTPIndex, valPtr);
	if(valPtr){

		free(valPtr);
		valPtr = NULL;
	}
	return CW_TRUE;
}
//weichao add for flow check report 2011.11.03
CWBool CWPareseWtp_Sta_Flow_Check_Report(CWProtocolMessage *msgPtr, int len, WIDStationInfo *valPtr,int wtpindex)
{
	wid_syslog_debug_debug(WID_WTPINFO,"***%s****\n",__func__);
	CWParseMessageElementStart();
	unsigned short length = 0;
	char reserved = 0;
	//
	length = CWProtocolRetrieve16(msgPtr);
	wid_syslog_debug_debug(WID_DEFAULT,"length = %d\n",length);
	reserved = CWProtocolRetrieve8(msgPtr);
	
	unsigned char* mac = (unsigned char*)CWProtocolRetrieveStr(msgPtr,6);
	memcpy(valPtr->mac, mac, 6);
	free(mac);
	mac = NULL;
	
	valPtr->rx_frames = CWProtocolRetrieve32(msgPtr);
	valPtr->tx_frames = CWProtocolRetrieve32(msgPtr);
	CWProtocolRetrieve64(msgPtr, &valPtr->rx_data_bytes);
	CWProtocolRetrieve64(msgPtr, &valPtr->tx_data_bytes);

//	wid_syslog_debug_debug(WID_DEFAULT,"valPtr->mac:%s ***\n",valPtr->mac);
	wid_syslog_debug_debug(WID_DEFAULT,"%02X:%02X:%02X:%02X:%02X:%02X\n",valPtr->mac[0],valPtr->mac[1],valPtr->mac[2],valPtr->mac[3],valPtr->mac[4],valPtr->mac[5]);
	wid_syslog_debug_debug(WID_DEFAULT,"valPtr->rx_data_bytes:%llu ***\n",valPtr->rx_data_bytes);
	wid_syslog_debug_debug(WID_DEFAULT,"valPtr->tx_data_bytes:%llu ***\n",valPtr->tx_data_bytes);
	wid_syslog_debug_debug(WID_DEFAULT,"valPtr->rx_frames:%u ***\n",valPtr->rx_frames);
	wid_syslog_debug_debug(WID_DEFAULT,"valPtr->tx_frames:%u ***\n",valPtr->tx_frames);
		
	WidAsd_StationInfoUpdate1(wtpindex,valPtr);
	CWParseMessageElementEnd(); 
}
CWBool CWPareseWtp_Sta_leave_Report(CWProtocolMessage *msgPtr, int len, WIDStationInfo *valPtr,int wtpindex)
{
	wid_syslog_debug_debug(WID_WTPINFO,"***%s****\n",__func__);
	
	WIDStationInfo valPtr1[20];
	memset(valPtr1,0,sizeof(WIDStationInfo));
	CWParseMessageElementStart();
	unsigned short length = 0;
	unsigned char radioid = 0;
	unsigned char wlanid = 0;
	unsigned char sta_count = 0;
	unsigned char* mac = NULL;
	unsigned char sta_reason;
	unsigned short sub_reason;
	int i = 0;
	length = CWProtocolRetrieve16(msgPtr);
	wid_syslog_debug_debug(WID_DEFAULT,"in case CWPareseWtp_Sta_leave_Report length = %d\n",length);

	radioid = CWProtocolRetrieve8(msgPtr);
	wlanid = CWProtocolRetrieve8(msgPtr);
	sta_count = CWProtocolRetrieve8(msgPtr);
	sta_reason = CWProtocolRetrieve8(msgPtr);
	sub_reason = CWProtocolRetrieve16(msgPtr);
	wid_syslog_debug_debug(WID_DEFAULT,"radioid = %d\n",radioid);
	wid_syslog_debug_debug(WID_DEFAULT,"wlanid = %d\n",wlanid);
	wid_syslog_debug_debug(WID_DEFAULT,"count = %d\n",sta_count);
	wid_syslog_debug_debug(WID_DEFAULT,"sta_reason = %d\n",sta_reason);
	wid_syslog_debug_debug(WID_DEFAULT,"sub_reason = %d\n",sub_reason);
	if(sta_count == 255){
		valPtr1[0].wlanId = wlanid ;
		valPtr1[0].radioId = radioid ;
		valPtr1[0].sta_reason = sta_reason;
		valPtr1[0].sub_reason = sub_reason;
		
	}
	else{
		if(sta_count > 20){
			wid_syslog_debug_debug(WID_WTPINFO,"count = %d, change to 20!\n",sta_count);
			sta_count = 20;
		}
		for( i  = 0 ; i < sta_count ; i++){
			valPtr1[i].wlanId = wlanid;
			valPtr1[i].radioId = radioid;
			valPtr1[i].sta_reason = sta_reason;
			valPtr1[i].sub_reason = sub_reason;
			mac = (unsigned char*)CWProtocolRetrieveStr(msgPtr,6);
			memcpy(valPtr1[i].mac, mac, 6);
			if(mac){
				free(mac);
				mac = NULL;
			}
			CWProtocolRetrieve64(msgPtr, &valPtr1[i].rx_data_bytes);
			CWProtocolRetrieve64(msgPtr, &valPtr1[i].tx_data_bytes);
			valPtr1[i].rx_data_frames = CWProtocolRetrieve32(msgPtr);
			valPtr1[i].tx_data_frames = CWProtocolRetrieve32(msgPtr);
			valPtr1[i].rx_frames = CWProtocolRetrieve32(msgPtr);
			valPtr1[i].tx_frames = CWProtocolRetrieve32(msgPtr);
			valPtr1[i].rx_frag_packets = CWProtocolRetrieve32(msgPtr);
			valPtr1[i].tx_frag_packets = CWProtocolRetrieve32(msgPtr);	
			wid_syslog_debug_debug(WID_DEFAULT,"valPtr mac:%02X:%02X:%02X:%02X:%02X:%02X\n",valPtr1[i].mac[0],valPtr[i].mac[1],valPtr[i].mac[2],valPtr[i].mac[3],valPtr[i].mac[4],valPtr[i].mac[5]);
			wid_syslog_debug_debug(WID_DEFAULT,"valPtr->rx_data_bytes:%llu ***\n",valPtr1[i].rx_data_bytes);
			wid_syslog_debug_debug(WID_DEFAULT,"valPtr->tx_data_bytes:%llu ***\n",valPtr1[i].tx_data_bytes);
			wid_syslog_debug_debug(WID_DEFAULT,"valPtr->rx_frames:%u ***\n",valPtr1[i].rx_frames);
			wid_syslog_debug_debug(WID_DEFAULT,"valPtr->tx_frames:%u ***\n",valPtr1[i].tx_frames);
			wid_syslog_debug_debug(WID_DEFAULT,"valPtr->sta_reason = %d\n",valPtr1[i].sta_reason);	
			wid_syslog_debug_debug(WID_DEFAULT,"valPtr->sub_reason = %d\n",valPtr1[i].sub_reason);	
		}
	}	
	WidAsd_StationLeaveReport(wtpindex,sta_count,valPtr1);
	CWParseMessageElementEnd(); 
}

CWBool CWParaseWTPTerminalStatistics(
				CWProtocolMessage *msgPtr, 
				int len,
				struct WID_WTP_TERMINAL_STATISTICS *wtp_terminal_statistics, 
				int wtpindex)
{
	wid_syslog_debug_debug(WID_DEFAULT, "------ %s %d ------\n", __func__, __LINE__);
	if (msgPtr == NULL || len <= 0 || wtpindex < 0 || wtpindex >WTP_NUM)
		return CW_FALSE;
	unsigned short sta_count = 0;
	int i = 0, j = 0;
	int length = CWProtocolRetrieve16(msgPtr);
	wid_syslog_debug_debug(WID_DEFAULT, "------ %s %d: length = %d ------\n", __func__, __LINE__, length);
	char * sta_mac = NULL;
	WIDStationInfo valPtr1[64];
	memset(valPtr1,0, 64*sizeof(WIDStationInfo));
	if (length >= len)
	{
		wid_syslog_crit("%s %d element's length is error: %d > %d\n", __func__, __LINE__, length, len);
		return CW_FALSE;
	}
	sta_count = CWProtocolRetrieve16(msgPtr);
	wid_syslog_debug_debug(WID_DEFAULT, "------ %s %d: sta_count = %d ------\n", __func__, __LINE__, sta_count);
	if (sta_count > 64)
	{
		wid_syslog_crit(" %s %d: station count is too big\n");
		return CW_FALSE;
	}
	for (i = 0; i < sta_count; i++)
	{
		sta_mac = CWProtocolRetrieveStr(msgPtr, 6);
		memcpy(&(valPtr1[i].mac), sta_mac, 6);
		wid_syslog_debug_debug(WID_DEFAULT, "------ %s %d [%d]mac: %x:%x:%x:%x:%x:%x ------\n", __func__, __LINE__, i, (char *)sta_mac, (char *)(sta_mac + 1), (char *)(sta_mac + 2), (char *)(sta_mac + 3), (char *)(sta_mac + 4), (char *)(sta_mac + 5));
		for (j = 0; j < WTP_SUPPORT_RATE_NUM; j ++)
		{
			valPtr1[i].wtp_sta_statistics_info.APStaTxDataRatePkts[j] = CWProtocolRetrieve32(msgPtr);
		}
		for (j = 0; j < WTP_SUPPORT_RATE_NUM; j ++)
		{
			valPtr1[i].wtp_sta_statistics_info.APStaRxDataRatePkts[j] = CWProtocolRetrieve32(msgPtr);
		}
		for (j = 0; j < WTP_RSSI_INTERVAL_NUM; j ++)
		{
			valPtr1[i].wtp_sta_statistics_info.APStaTxSignalStrengthPkts[j] = CWProtocolRetrieve32(msgPtr);		
		}
#if 0
			int m = 0;
			int mcs_idx = 0;
			char strRate[44][6]={"1M","2M","5.5M","6M","9M","11M","12M","18M","24M","36M","48M","54M"};//1-12
			char strSignalStrengthRegion[17][8]={">-10","-10~-19","-20~-39","-40~-49","-50~-59","-60~-64","-65~-67","-68~-70","-71~-73",
							"-74~-76","-77~-79","-80~-82","-83~-85","-86~-88","-89~-91","-92~-94","<-94"};
			wid_syslog_debug_debug(WID_DEFAULT, "------------------------------------------------\n");
			wid_syslog_debug_debug(WID_DEFAULT, "TxDataRatePkts:\n");
			for(m=0,mcs_idx=0;m<44;m++)
			{
				if(m<12)
					wid_syslog_debug_debug(WID_DEFAULT,
								"%5s:%-10u",
								strRate[m], 
								valPtr1[i].wtp_sta_statistics_info.APStaRxDataRatePkts[m]
								);
				else
					wid_syslog_debug_debug(WID_DEFAULT,
								"MCS%02d:%-10u",
								mcs_idx++, 
								valPtr1[i].wtp_sta_statistics_info.APStaRxDataRatePkts[m]
								);
			}			
			wid_syslog_debug_debug(WID_DEFAULT,"\nRxDataRatePkts:\n");
			for(m=0,mcs_idx=0;m<44;m++)
			{
				if(m<12)
					wid_syslog_debug_debug(WID_DEFAULT,
								"%5s:%-10u",
								strRate[m],
								valPtr1[i].wtp_sta_statistics_info.APStaTxDataRatePkts[m]
								);
				else
					wid_syslog_debug_debug(WID_DEFAULT,
								"MCS%02d:%-10u",
								mcs_idx++,
								valPtr1[i].wtp_sta_statistics_info.APStaTxDataRatePkts[m]);
			}
			wid_syslog_debug_debug(WID_DEFAULT,"\nTxSignalStrengthPkts:\n");
			for(m=0;m<17;m++)
			{
				wid_syslog_debug_debug(WID_DEFAULT,
								"[%7s]:%-10u",
								strSignalStrengthRegion[m],
								valPtr1[i].wtp_sta_statistics_info.APStaTxSignalStrengthPkts[m]
								);
			}
			wid_syslog_debug_debug(WID_DEFAULT, "\n");
			wid_syslog_debug_debug(WID_DEFAULT, "------------------------------------------------\n");
#endif
	}
	if (sta_count != 0)
		WidAsd_WTPTerminalStatisticsUpdate(wtpindex, sta_count, valPtr1);
	return CW_TRUE;
}
/* zhangshu add for Terminal Disturb Info Report, 2010-10-08 */
CWBool CWParseWtp_Sta_Terminal_Disturb_Report(CWProtocolMessage *msgPtr, int len,char *valPtr, int WTPIndex)
{
    wid_syslog_debug_debug(WID_DEFAULT,"*** CWParseWtp_Sta_Terminal_Disturb_Report ***\n");
	wid_syslog_debug_debug(WID_DEFAULT,"*** len:%d ***\n",len);
	printf("*** CWParseWtp_Sta_Terminal_Disturb_Report ***\n");
	printf("*** len:%d ***\n",len);
	unsigned short sta_trap_count = 0;
	unsigned char radio_id = 0;
	unsigned char wlan_id = 0;
	unsigned char state = 0;/*1--trap;0--trap clear*/
	unsigned char *sta_mac = NULL;
	unsigned int i = 0;
	unsigned char currentchannel = 0;
	int temp = 0;
    
    radio_id = CWProtocolRetrieve8(msgPtr);
    wlan_id = CWProtocolRetrieve8(msgPtr);
    state = CWProtocolRetrieve8(msgPtr);
    temp = CWProtocolRetrieve32(msgPtr);
    sta_trap_count = CWProtocolRetrieve16(msgPtr);

    wid_syslog_debug_debug(WID_DEFAULT,"sta_trap_count = %d\n",sta_trap_count);
	wid_syslog_debug_debug(WID_DEFAULT,"radio_id = %d\n",radio_id);
	wid_syslog_debug_debug(WID_DEFAULT,"wlan_id = %d\n",wlan_id);
	wid_syslog_debug_debug(WID_DEFAULT,"state = %d\n",state);

    currentchannel = AC_WTP[WTPIndex]->WTP_Radio[radio_id]->Radio_Chan;
    
    for(i=0; i<sta_trap_count; i++)
    {
        sta_mac = (unsigned char*)CWProtocolRetrieveStr(msgPtr,6);
        
        wid_syslog_debug_debug(WID_DEFAULT,"i=%d,sta_mac:%02X:%02X:%02X:%02X:%02X:%02X \n",i,sta_mac[0],sta_mac[1],sta_mac[2],sta_mac[3],sta_mac[4],sta_mac[5]); 
	    wid_syslog_debug_debug(WID_DEFAULT,"i=%d,gtrapflag=%d\n",i,gtrapflag);
	    
        if(gtrapflag>=24)
        {
			if(state == 1)
			{
				if(AC_WTP[WTPIndex]->wid_trap.rogue_terminal_trap_flag == 0)  //fengwenchao add 20110221
					{
			    		printf("111111111 Call trap func 111111111\n");
						wid_dbus_trap_wtp_channel_terminal_interference(WTPIndex, radio_id, currentchannel, sta_mac);
						AC_WTP[WTPIndex]->wid_trap.rogue_terminal_trap_flag = 1;   //fengwenchao add 20110221
					}
			}
			else
			{
				if(AC_WTP[WTPIndex]->wid_trap.rogue_terminal_trap_flag == 1)    //fengwenchao add 20110221
					{
			    		printf("222222222 Call trap clear func 222222222\n");
			    		wid_dbus_trap_wtp_channel_terminal_interference_clear(WTPIndex, radio_id, currentchannel, sta_mac);
						AC_WTP[WTPIndex]->wid_trap.rogue_terminal_trap_flag = 0;   //fengwenchao add 20110221
					}
			}
		}
		//memset(sta_mac, 0, MAC_LEN);
    
    if(sta_mac)
    {
        free(sta_mac);
	    sta_mac = NULL;
	}
    }
	return CW_TRUE;
	
}

CWBool  CWAssembleStaticAPIPDNS(CWProtocolMessage *msgPtr,int wtpid)
{
	wid_syslog_debug_debug(WID_WTPINFO,"#### CWAssembleStaticAPIPDNS ####\n");
	
	unsigned char valuelen=24; //
	unsigned char reserved = 1;				//xiaodawei modify, change 0 to 1, static, 20101230
	if(msgPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);

	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, valuelen, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	
	CWProtocolStore32(msgPtr, AC_WTP[wtpid]->ap_ipadd);
	CWProtocolStore32(msgPtr, AC_WTP[wtpid]->ap_mask_new);
	CWProtocolStore32(msgPtr, AC_WTP[wtpid]->ap_gateway); 
	CWProtocolStore32(msgPtr, AC_WTP[wtpid]->ap_dnsfirst); 
	CWProtocolStore32(msgPtr, AC_WTP[wtpid]->ap_dnssecend); 
	CWProtocolStore8(msgPtr, reserved);			//xiaodawei modify, 20101231
	CWProtocolStore8(msgPtr, 0); 
	CWProtocolStore16(msgPtr, 0); 
	
	return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_WTP_STATIC_IP_CW_TYPE_V2);

}
CWBool  CWAssembleSnoopingAble(CWProtocolMessage *msgPtr,int wtpid)
{
	wid_syslog_debug_debug(WID_WTPINFO,"#### CWAssembleSnoopingAble ####\n");
	
	unsigned char valuelen=8; //
	//unsigned int reserved = 0;
	if(msgPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);

	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, valuelen, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	
	CWProtocolStore8(msgPtr, WTP);// type
	CWProtocolStore8(msgPtr, DHCP_SNOOPING);// op
	CWProtocolStore8(msgPtr, 0);//Radio id
	CWProtocolStore8(msgPtr, 0); //wlan id
	CWProtocolStore8(msgPtr, AC_WTP[wtpid]->dhcp_snooping); 
	CWProtocolStore8(msgPtr, 0); 
	CWProtocolStore16(msgPtr, 0); 
	//CWProtocolStore32(msgPtr, AC_WTP[wtpid]->ap_dnssecend); 
	//CWProtocolStore32(msgPtr, reserved); 	
	
	return CWAssembleMsgElem(msgPtr, BINDING_MSG_ELEMENT_TYPE_WTP_RADIO_SET);

}
CWBool  CWAssembleIpMacReport(CWProtocolMessage *msgPtr,int wtpid)
{
	wid_syslog_debug_debug(WID_WTPINFO,"#### CWAssembleIpMacReport ####\n");
	
	unsigned char valuelen=8; //
	//unsigned int reserved = 0;
	if(msgPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);

	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, valuelen, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	
	CWProtocolStore8(msgPtr, STA);//type
	CWProtocolStore8(msgPtr, STA_IP_MAC_REPORT);// op
	CWProtocolStore8(msgPtr, 0);//Radio id
	CWProtocolStore8(msgPtr, 0); //wlan id
	CWProtocolStore8(msgPtr, AC_WTP[wtpid]->sta_ip_report);
	CWProtocolStore8(msgPtr, 0);
	CWProtocolStore16(msgPtr, 0); //value
	//CWProtocolStore32(msgPtr, AC_WTP[wtpid]->ap_dnssecend); 
	//CWProtocolStore32(msgPtr, reserved); 	
	
	return CWAssembleMsgElem(msgPtr, BINDING_MSG_ELEMENT_TYPE_WTP_RADIO_SET);

}

/* zhangshu add for Terminal Disturb Info Report, 2010-10-08 */
CWBool  CWAssembleTerminalDisturbInfoReport(CWProtocolMessage *msgPtr,int wtpid)
{
    printf("1111111  Send Terminal Disturb info to ap  111111111\n");
	unsigned char valuelen = 10; 
	unsigned char value = 11;
	unsigned char flag = AC_WTP[wtpid]->ter_dis_info.reportswitch;
	unsigned short trap_pkt = AC_WTP[wtpid]->ter_dis_info.reportpkt;
	unsigned short sta_count = AC_WTP[wtpid]->ter_dis_info.sta_trap_count;
	int jump = 0;
	if(msgPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);

    wid_syslog_debug_debug(WID_DEFAULT,"1111111 Send Terminal Disturb info to ap 111111\n");
	wid_syslog_debug_debug(WID_DEFAULT,"value = %d\n",value);
	wid_syslog_debug_debug(WID_DEFAULT,"flag = %d\n",flag);
	wid_syslog_debug_debug(WID_DEFAULT,"sta_count = %d\n",sta_count);
	wid_syslog_debug_debug(WID_DEFAULT,"trap_pkt = %d\n",trap_pkt);

	// create message
	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, valuelen, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	
	CWProtocolStore8(msgPtr, value);
	CWProtocolStore8(msgPtr, flag);
	CWProtocolStore16(msgPtr, sta_count);
	CWProtocolStore16(msgPtr, trap_pkt);
	CWProtocolStore32(msgPtr, jump);
	
	return CWAssembleMsgElem(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE);
}
CWBool  CWAssembleWtpStaDeauthreport(CWProtocolMessage *msgPtr,int wtpid)
{
	wid_syslog_debug_debug(WID_WTPINFO,"#### CWAssemblewtpstadeauthreport ####\n");
	
	unsigned short elementid = 26;
	unsigned short length = 5; 
	unsigned char flag = AC_WTP[wtpid]->sta_deauth_message_reportswitch;
	if(msgPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);

	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, length, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	
	CWProtocolStore16(msgPtr, elementid);
	CWProtocolStore16(msgPtr, length-4);
	CWProtocolStore8(msgPtr, flag);
	return CWAssembleMsgElemVendor(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE);

}
CWBool  CWAssembleWtpStaFlowInformationreport(CWProtocolMessage *msgPtr,int wtpid)
{
	wid_syslog_debug_debug(WID_WTPINFO,"#### CWAssemblewtpstadeauthreport ####\n");
	
	unsigned short elementid = 27;
	unsigned short length = 5; 
	unsigned char flag = AC_WTP[wtpid]->sta_flow_information_reportswitch;
	if(msgPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);

	CW_CREATE_PROTOCOL_MESSAGE(*msgPtr, length, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	
	CWProtocolStore16(msgPtr, elementid);
	CWProtocolStore16(msgPtr, length-4);
	CWProtocolStore8(msgPtr, flag);
	return CWAssembleMsgElemVendor(msgPtr, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE);

}


