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
* ACAccessCheck.c
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
#include "dbus/asd/ASDDbusDef1.h"
#include "wcpss/wid/WID.h" 
#include "CWAC.h"
#include "ACDbus.h"
#include "CWStevens.h"
#include <syslog.h>
#include "ACDbus_handler.h"

#define WTP_IP_HASH(ip) (ip[3])

struct wtp_access_info * get_ap(WID_ACCESS *AC, unsigned int ipip)
{
	struct wtp_access_info *s;
	unsigned char * ip = (unsigned char*)&(ipip);
	s = AC->wtp_list_hash[WTP_IP_HASH(ip)];
	while ((s != NULL) && (s->ip != ipip))
		s = s->hnext;
	return s;
}


static void ap_list_del(WID_ACCESS *AC, struct wtp_access_info *wtp)
{
	struct wtp_access_info *tmp;

	if (AC->wtp_list == wtp) {
		AC->wtp_list = wtp->next;
		return;
	}

	tmp = AC->wtp_list;
	while (tmp != NULL && tmp->next != wtp)
		tmp = tmp->next;
	if (tmp == NULL) {

	} else
		tmp->next = wtp->next;
}


void ap_hash_add(WID_ACCESS *AC, struct wtp_access_info *wtp)
{
	unsigned char *ipaddr = (unsigned char *)&(wtp->ip);
	wtp->hnext = AC->wtp_list_hash[WTP_IP_HASH(ipaddr)];
	AC->wtp_list_hash[WTP_IP_HASH(ipaddr)] = wtp;
}


static void ap_hash_del(WID_ACCESS *AC, struct wtp_access_info *wtp)
{
	struct wtp_access_info *s;
	unsigned char *ipaddr = (unsigned char *)&(wtp->ip);
	s = AC->wtp_list_hash[WTP_IP_HASH(ipaddr)];
	if (s == NULL) return;
	if (s->ip == wtp->ip) {
		AC->wtp_list_hash[WTP_IP_HASH(ipaddr)] = s->hnext;
		return;
	}

	while (s->hnext != NULL &&
	       (s->hnext != wtp))
		s = s->hnext;
	if (s->hnext != NULL)
		s->hnext = s->hnext->hnext;
	else
		;
}


void free_ap(WID_ACCESS *AC, struct wtp_access_info *wtp)
{
	ap_hash_del(AC, wtp);
	ap_list_del(AC, wtp);
	AC->num--;
	WID_FREE(wtp->apcode);
	WID_FREE(wtp->model);
	WID_FREE(wtp->sn);
	WID_FREE(wtp->version);
	WID_FREE(wtp->codever);
	WID_FREE(wtp->WTPMAC);
	WID_FREE(wtp->ifname);
	WID_FREE(wtp);

}


struct wtp_access_info * ap_add(WID_ACCESS *AC, struct sockaddr_in * sa, CWWTPVendorInfos *valPtr, CWWTPDescriptor *valPtr1, char * name)
{
	struct wtp_access_info *wtp;
	int i = 0;
	wtp = get_ap(AC, sa->sin_addr.s_addr);
	
	if (wtp){
		memset(wtp->WTPMAC,0,6);		
		memset(wtp->model,0,strlen(wtp->model));
		memset(wtp->apcode,0,strlen(wtp->model));
		memset(wtp->sn,0,strlen(wtp->sn));
		memset(wtp->version,0,strlen(wtp->version));
		memset(wtp->codever,0,strlen(wtp->codever));
		memset(wtp->ifname,0,16);
		for(i = 0; i < valPtr->vendorInfosCount; i++){
			if((valPtr->vendorInfos)[i].type == CW_WTP_MODEL_NUMBER){
				WID_FREE(wtp->apcode);				
				wtp->apcode= ( char *)WID_MALLOC((valPtr->vendorInfos)[i].length + 1);	
				memset(wtp->apcode,0,(valPtr->vendorInfos)[i].length + 1);
				memcpy(wtp->apcode, (valPtr->vendorInfos)[i].model, (valPtr->vendorInfos)[i].length);
			}
			else if((valPtr->vendorInfos)[i].type == CW_WTP_SERIAL_NUMBER){
				WID_FREE(wtp->sn);
				wtp->sn = ( char *)WID_MALLOC((valPtr->vendorInfos)[i].length + 1);
				memset(wtp->sn,0,(valPtr->vendorInfos)[i].length + 1);
				if(wid_illegal_character_check((char*)(valPtr->vendorInfos)[i].SN,(valPtr->vendorInfos)[i].length,1) > 0){
					memcpy(wtp->sn, (valPtr->vendorInfos)[i].SN, (valPtr->vendorInfos)[i].length);
				}else{
					wid_syslog_info("%s wtp sn %s something wrong\n",__func__,(valPtr->vendorInfos)[i].SN);
				}
			}
			else if((valPtr->vendorInfos)[i].type == CW_BOARD_MAC_ADDRESS){
				memcpy(wtp->WTPMAC, (valPtr->vendorInfos)[i].mac, (valPtr->vendorInfos)[i].length);
			}
			else if((valPtr->vendorInfos)[i].type == CW_WTP_REAL_MODEL_NUMBER){ 
				WID_FREE(wtp->model);
				wtp->model= ( char *)WID_MALLOC((valPtr->vendorInfos)[i].length + 1);
				memset(wtp->model,0,(valPtr->vendorInfos)[i].length + 1);
				memcpy(wtp->model, (valPtr->vendorInfos)[i].Rmodel, (valPtr->vendorInfos)[i].length);			
			}
			else if((valPtr->vendorInfos)[i].type == 6)//CW_WTP_CODE_VERSION = 6
			{
				WID_FREE(wtp->codever);
				wtp->codever = ( char *)WID_MALLOC((valPtr->vendorInfos)[i].length + 1);
				memset(wtp->codever,0,(valPtr->vendorInfos)[i].length + 1);
				if(wid_illegal_character_check((char*)(valPtr->vendorInfos)[i].codever,(valPtr->vendorInfos)[i].length,1) > 0){
					memcpy(wtp->codever, (valPtr->vendorInfos)[i].codever, (valPtr->vendorInfos)[i].length);	
				}else{
					wid_syslog_info("%s wtp codever %s something wrong\n",__func__,(valPtr->vendorInfos)[i].codever);
				}
			}
			else
				continue;
		}
		
		for(i = 0; i < valPtr1->vendorInfos.vendorInfosCount; i++){
			if((valPtr1->vendorInfos.vendorInfos)[i].type == CW_WTP_SOFTWARE_VERSION){
				WID_FREE(wtp->version);
				wtp->version = (char *)WID_MALLOC((valPtr1->vendorInfos.vendorInfos)[i].length+1);
				memset(wtp->version, 0,(valPtr1->vendorInfos.vendorInfos)[i].length+1);
				memcpy(wtp->version, (valPtr1->vendorInfos.vendorInfos)[i].ver, (valPtr1->vendorInfos.vendorInfos)[i].length);
			}
			else
				continue;
		}
		
		//printf("wtp->ifname:%s,name:%s,len:%d\n",wtp->ifname,name,strlen(name));
		memcpy(wtp->ifname, name, strlen(name));
		return wtp;
	}
	
	wtp = WID_MALLOC(sizeof(struct wtp_access_info));
	if (wtp == NULL) {
		return NULL;
	}
	memset(wtp, 0, sizeof(struct wtp_access_info));	
	wtp->WTPMAC = (unsigned char *)WID_MALLOC(MAC_LEN+1);
	memset(wtp->WTPMAC,0,6);
	wtp->model= ( char *)WID_MALLOC(1);
	memset(wtp->model,0,1);
	wtp->apcode= ( char *)WID_MALLOC(1);	
	memset(wtp->apcode,0,1);
	wtp->sn = ( char *)WID_MALLOC(1);
	memset(wtp->sn,0,1);
	wtp->version = ( char *)WID_MALLOC(1);
	memset(wtp->version,0,1);
	wtp->codever = ( char *)WID_MALLOC(1);
	memset(wtp->codever,0,1);
	wtp->ifname = ( char *)WID_MALLOC(16);
	memset(wtp->ifname,0,16);

	for(i = 0; i < valPtr->vendorInfosCount; i++){
		if((valPtr->vendorInfos)[i].type == CW_WTP_MODEL_NUMBER){	
			WID_FREE(wtp->apcode);
			wtp->apcode= ( char *)WID_MALLOC((valPtr->vendorInfos)[i].length + 1);	
			memset(wtp->apcode,0,(valPtr->vendorInfos)[i].length + 1);
			memcpy(wtp->apcode, (valPtr->vendorInfos)[i].model, (valPtr->vendorInfos)[i].length);
		}
		else if((valPtr->vendorInfos)[i].type == CW_WTP_SERIAL_NUMBER){
			WID_FREE(wtp->sn);
			wtp->sn = ( char *)WID_MALLOC((valPtr->vendorInfos)[i].length + 1);
			memset(wtp->sn,0,(valPtr->vendorInfos)[i].length + 1);
			memcpy(wtp->sn, (valPtr->vendorInfos)[i].SN, (valPtr->vendorInfos)[i].length);
		}
		else if((valPtr->vendorInfos)[i].type == CW_BOARD_MAC_ADDRESS){			
			memset(wtp->WTPMAC,0,6);
			memcpy(wtp->WTPMAC, (valPtr->vendorInfos)[i].mac, (valPtr->vendorInfos)[i].length);
		}
		else if((valPtr->vendorInfos)[i].type == CW_WTP_REAL_MODEL_NUMBER){
			WID_FREE(wtp->model);
			wtp->model= ( char *)WID_MALLOC((valPtr->vendorInfos)[i].length + 1);
			memset(wtp->model,0,(valPtr->vendorInfos)[i].length + 1);
			memcpy(wtp->model, (valPtr->vendorInfos)[i].Rmodel, (valPtr->vendorInfos)[i].length);			
		}
		else if((valPtr->vendorInfos)[i].type == 6)//CW_WTP_CODE_VERSION = 6
		{
			WID_FREE(wtp->codever);
			wtp->codever = ( char *)WID_MALLOC((valPtr->vendorInfos)[i].length + 1);
			memset(wtp->codever,0,(valPtr->vendorInfos)[i].length + 1);		
			if(wid_illegal_character_check((char*)(valPtr->vendorInfos)[i].codever,(valPtr->vendorInfos)[i].length,1) > 0){
				memcpy(wtp->codever, (valPtr->vendorInfos)[i].codever, (valPtr->vendorInfos)[i].length);	
			}else{
				wid_syslog_info("%s wtp codever %s something wrong\n",__func__,(valPtr->vendorInfos)[i].codever);
			}
		}		
		else
			continue;
	}

	for(i = 0; i < valPtr1->vendorInfos.vendorInfosCount; i++){
		if((valPtr1->vendorInfos.vendorInfos)[i].type == CW_WTP_SOFTWARE_VERSION){
			WID_FREE(wtp->version);
			wtp->version = (char *)WID_MALLOC((valPtr1->vendorInfos.vendorInfos)[i].length+1);
			memset(wtp->version, 0,(valPtr1->vendorInfos.vendorInfos)[i].length+1);
			memcpy(wtp->version, (valPtr1->vendorInfos.vendorInfos)[i].ver, (valPtr1->vendorInfos.vendorInfos)[i].length);
		}
		else
			continue;
	}

	printf("wtp->ifname:%s,name:%s,len:%d\n",wtp->ifname,name,strlen(name));
	memcpy(wtp->ifname, name, strlen(name));
	printf("wtp->ifname:%s,len:%d,name:%s,len:%d\n",wtp->ifname,strlen(wtp->ifname),name,strlen(name));
//	memcpy(&(wtp->ip),&(sa->sin_addr.s_addr),sizeof(int));
	wtp->ip = sa->sin_addr.s_addr;
	wtp->next = AC->wtp_list;
	AC->wtp_list = wtp;
	AC->num++;
	ap_hash_add(AC, wtp);
	unsigned char *ip = (unsigned char*)&(wtp->ip);
//	printf("num %d\n",AC->num);
	printf("%02X:%02X:%02X:%02X:%02X:%02X %d.%d.%d.%d %-12s %-10s %-7s %s\n",
				wtp->WTPMAC[0],wtp->WTPMAC[1],wtp->WTPMAC[2],wtp->WTPMAC[3],wtp->WTPMAC[4],wtp->WTPMAC[5],
				ip[0],ip[1],ip[2],ip[3],
				wtp->model,
				wtp->apcode,
				wtp->version,
				wtp->ifname);
//	printf("SN:%s\n",wtp->sn);
	return wtp;
	
}

void del_all_ap(WID_ACCESS *AC)
{
	struct wtp_access_info *wtp, *prev;
	if(AC != NULL){
		wtp = AC->wtp_list;
		while(wtp){
			prev = wtp;
			wtp = wtp->next;
			free_ap(AC,prev);
		}
	}
}

