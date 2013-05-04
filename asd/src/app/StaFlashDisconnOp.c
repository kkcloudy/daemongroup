#include <time.h>
#include <sys/time.h>
#include "includes.h"

#include "asd.h"
#include "ASDStaInfo.h"
#include "wcpss/waw.h"
#include "wcpss/wid/WID.h"
#include "wcpss/asd/asd.h"
#include "dbus/asd/ASDDbusDef.h"
#include "common.h"
#include "ap.h"
#include "config.h"
#include "circle.h"
#include "StaFlashDisconnOp.h"
#include "ASDDbus_handler.h"
struct FLASHDISCONN_STAINFO * flashdisconn_get_sta(asd_sta_flash_disconn *FDStas, const u8 *sta)
{
	struct FLASHDISCONN_STAINFO *s;

	s = FDStas->fd_sta_hash[STA_HASH(sta)];
	while (s != NULL && os_memcmp(s->addr, sta, 6) != 0)
		s = s->hnext;
	return s;
}


static void flashdisconn_sta_list_del(asd_sta_flash_disconn *FDStas, struct FLASHDISCONN_STAINFO *sta)
{
	struct FLASHDISCONN_STAINFO *tmp;

	if (FDStas->fd_sta_list == sta) {
		FDStas->fd_sta_list = sta->next;
		return;
	}

	tmp = FDStas->fd_sta_list;
	while (tmp != NULL && tmp->next != sta)
		tmp = tmp->next;
	if (tmp == NULL) {
		asd_printf(ASD_DEFAULT,MSG_ERROR, "Could not remove STA " MACSTR " from "
			   "list.", MAC2STR(sta->addr));
	} else
		tmp->next = sta->next;
	return;
}


void flashdisconn_sta_hash_add(asd_sta_flash_disconn *FDStas, struct FLASHDISCONN_STAINFO *sta)
{
	sta->hnext = FDStas->fd_sta_hash[STA_HASH(sta->addr)];
	FDStas->fd_sta_hash[STA_HASH(sta->addr)] = sta;
	return ;
}


static void flashdisconn_sta_hash_del(asd_sta_flash_disconn *FDStas, struct FLASHDISCONN_STAINFO *sta)
{
	struct FLASHDISCONN_STAINFO *s;

	s = FDStas->fd_sta_hash[STA_HASH(sta->addr)];
	if (s == NULL) return;
	if (os_memcmp(s->addr, sta->addr, 6) == 0) {
		FDStas->fd_sta_hash[STA_HASH(sta->addr)] = s->hnext;
		return;
	}

	while (s->hnext != NULL &&
	       os_memcmp(s->hnext->addr, sta->addr, ETH_ALEN) != 0)
		s = s->hnext;
	if (s->hnext != NULL)
		s->hnext = s->hnext->hnext;
	else
		asd_printf(ASD_DEFAULT,MSG_ERROR, "AP: could not remove STA " MACSTR
			   " from hash table", MAC2STR(sta->addr));
	return ;
}


void flashdisconn_free_sta(asd_sta_flash_disconn *FDStas, struct FLASHDISCONN_STAINFO *sta)
{
	circle_cancel_timeout(sta_flash_disconn_timer, FDStas, sta);		
	flashdisconn_sta_hash_del(FDStas, sta);
	flashdisconn_sta_list_del(FDStas, sta);
	FDStas->fd_num_sta--;
	os_free(sta);

}


struct FLASHDISCONN_STAINFO * flashdisconn_sta_add(asd_sta_flash_disconn *FDStas, const u8 *addr, unsigned int BSSIndex, unsigned char WLANID)
{
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"now in %s\n",__func__);
	struct FLASHDISCONN_STAINFO *sta;
	struct sta_info *flash_sta;				//weichao add 
	struct asd_data *wasd = NULL;
	pthread_mutex_lock(&asd_flash_disconn_mutex);	
	
	wasd = AsdCheckBSSIndex(BSSIndex);
	if(wasd == NULL){
		asd_printf(ASD_DEFAULT,MSG_DEBUG," the wasd is not exit\n");
		pthread_mutex_unlock(&asd_flash_disconn_mutex); 
		return NULL;
	}

	flash_sta = ap_get_sta(wasd, addr);

	if(flash_sta == NULL){
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"the sta is not exist\n");
		pthread_mutex_unlock(&asd_flash_disconn_mutex); 
		return NULL;		
	}
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"rx,tx %llu,%llu\n",flash_sta->rxpackets,flash_sta->txpackets);
		sta = flashdisconn_get_sta(FDStas, addr);
		if (sta){
			asd_printf(ASD_DEFAULT,MSG_DEBUG,"flashdisconn find sta in add\n");
			//weichao add
			sta->rxbytes =flash_sta->rxbytes + flash_sta->pre_rxbytes;
			sta->txbytes = flash_sta->txbytes +  flash_sta->pre_txbytes;
			sta->rxpackets = flash_sta->rxpackets + flash_sta->pre_rxpackets;
			sta->txpackets = flash_sta->txpackets +  flash_sta->pre_txpackets;			
			pthread_mutex_unlock(&asd_flash_disconn_mutex);	
			return sta;
		}
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"flashdisconn didn't find sta \n");
		sta = os_zalloc(sizeof(struct FLASHDISCONN_STAINFO));
		if (sta == NULL) {
			asd_printf(ASD_DEFAULT,MSG_ERROR, "malloc failed");
			pthread_mutex_unlock(&asd_flash_disconn_mutex);	
			return NULL;
		}	
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"flashdisconn add sta in add\n");
		os_memcpy(sta->addr, addr, ETH_ALEN);
		sta->bssindex = BSSIndex;
		sta->wtpid = ((BSSIndex)/L_BSS_NUM)/L_RADIO_NUM;
		sta->wlanid = WLANID;
		sta->ipv4Address = flash_sta->ip_addr.s_addr;
		//weichao add
		sta->rxbytes = flash_sta->rxbytes + flash_sta->pre_rxbytes;
		sta->txbytes = flash_sta->txbytes +  flash_sta->pre_txbytes;
		sta->rxpackets = flash_sta->rxpackets + flash_sta->pre_rxpackets;
		sta->txpackets = flash_sta->txpackets +  flash_sta->pre_txpackets;
		
		sta->next = FDStas->fd_sta_list;

		FDStas->fd_sta_list = sta;
		FDStas->fd_num_sta++;
		flashdisconn_sta_hash_add(FDStas, sta);	

		asd_printf(ASD_DEFAULT,MSG_DEBUG,"flashsta add success\n");
		circle_register_timeout(ASD_NOTICE_STA_INFO_TO_PORTAL_TIMER, 0, sta_flash_disconn_timer, FDStas, sta);		//ht add 090219
		pthread_mutex_unlock(&asd_flash_disconn_mutex); 
		return sta;
	
}

void flashdisconn_del_all_sta(asd_sta_flash_disconn *FDStas)
{
	struct FLASHDISCONN_STAINFO *sta, *prev;
	pthread_mutex_lock(&asd_flash_disconn_mutex); 
	sta = FDStas->fd_sta_list;
	while(sta){
		prev = sta;
		sta = sta->next;
		flashdisconn_free_sta(FDStas,prev);
	}
	pthread_mutex_unlock(&asd_flash_disconn_mutex); 

}

void sta_flash_disconn_timer(void *eloop_ctx, void *timeout_ctx)
{
	pthread_mutex_lock(&asd_flash_disconn_mutex); 
	asd_sta_flash_disconn *FDStas = eloop_ctx;
	struct FLASHDISCONN_STAINFO *sta = timeout_ctx;
	UpdateStaInfoToEAG(sta->bssindex,sta,WID_DEL);	//weichao modify
	flashdisconn_free_sta(FDStas, sta);
	pthread_mutex_unlock(&asd_flash_disconn_mutex); 
	return;
}


void sta_flash_disconn_check(asd_sta_flash_disconn *FDStas, const unsigned char *addr)
{
	struct FLASHDISCONN_STAINFO *sta = NULL;	
	pthread_mutex_lock(&asd_flash_disconn_mutex); 
	sta = flashdisconn_get_sta(FDStas, addr);
	if (sta == NULL){
		printf("%s sta == NULL\n",__func__);
		pthread_mutex_unlock(&asd_flash_disconn_mutex); 
		return;
	}
	UpdateStaInfoToEAG(sta->bssindex,sta,OPEN_ROAM);		//weichao modify
	
	struct sta_info *flash_sta;				//weichao add 
	struct asd_data *wasd = NULL;
	wasd = AsdCheckBSSIndex(sta->bssindex);
	if(wasd == NULL){
		asd_printf(ASD_DEFAULT,MSG_DEBUG," the wasd is not exit\n");
		pthread_mutex_unlock(&asd_flash_disconn_mutex); 
		return;
	}

	flash_sta = ap_get_sta(wasd, addr);

	if(flash_sta == NULL){
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"the sta is not exist\n");
		pthread_mutex_unlock(&asd_flash_disconn_mutex); 
		return;		
	}
	flash_sta->pre_rxbytes = sta->rxbytes;
	flash_sta->pre_txbytes = sta->txbytes;
	flash_sta->pre_rxpackets = sta->rxpackets;
	flash_sta->pre_txpackets = sta->txpackets;
	flashdisconn_free_sta(FDStas, sta);	
	pthread_mutex_unlock(&asd_flash_disconn_mutex); 
	return;
}


