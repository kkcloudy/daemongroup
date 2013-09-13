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
* AsdAccounting.c
*
*
* CREATOR:
* autelan.software.WirelessControl. team
*
* DESCRIPTION:
* asd module
*
*
*******************************************************************************/

#include "includes.h"

#include "asd.h"
#include "ASDRadius/radius.h"
#include "ASDRadius/radius_client.h"
#include "circle.h"
#include "ASDAccounting.h"
#include "ASD8021XOp.h"
#include "ASDCallback.h"
#include "wcpss/waw.h"
#include "wcpss/wid/WID.h"
#include "wcpss/asd/asd.h"
#include "asd_bak.h"
#include "ASDNetlinkArpOp.h"
#include "ASDEapolSM.h"
#include "time.h"
#include "ASDStaInfo.h"
extern char is_secondary;//nl add 091208
extern char asd_sta_idle_time_switch;
extern int asd_sta_idle_time;
unsigned int ACCT_SESSION_ID_HI = 0;
unsigned int ACCT_SESSION_ID_LO = 0;

extern unsigned char gASDLOGDEBUG;
/* Default interval in seconds for polling TX/RX octets from the driver if
 * STA is not using interim accounting. This detects wrap arounds for
 * input/output octets and updates Acct-{Input,Output}-Gigawords. */
#define ACCT_DEFAULT_UPDATE_INTERVAL 300

/* from ieee802_1x.c */
const char *radius_mode_txt(struct asd_data *wasd);
int radius_sta_rate(struct asd_data *wasd, struct sta_info *sta);


static struct radius_msg * accounting_msg(struct asd_data *wasd,
					  struct sta_info *sta,struct radius_client_info *client_info,
					  int status_type)
{
	struct radius_msg *msg;
	char buf[128];
	u8 *val;
	size_t len;
	int i;
	unsigned int wtpid = 0;
	unsigned char SID = wasd->SecurityID;	
	if(sta == NULL)
		return NULL;
	struct eapol_state_machine *sm = sta->eapol_sm;
	if(!sm ||!sm->wasd )
		return NULL;
	if((sm->radius_identifier < 256)&&(sm->radius_identifier >= 0))
	{
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"free the sta prev radius msg sta->wlanid :%d\n",sm->wasd->WlanID);
		radius_client_info_free(radius_client_get_sta_info(sta,sm->wasd->WlanID,RADIUS_ACCT));		
	}
	sm->radius_identifier = radius_client_get_id(client_info);
	if((sm->radius_identifier <0)||(sm->radius_identifier > 255))
	{
		asd_printf(ASD_1X,MSG_WARNING,"Could not find the correct id!\n");
		return  NULL; 
	}
	msg = radius_msg_new(RADIUS_CODE_ACCOUNTING_REQUEST,
			     sm->radius_identifier);
	if (msg == NULL) {
		asd_printf(ASD_1X,MSG_DEBUG,"Could not create net RADIUS packet\n");
		return NULL;
	}

	if (sta) {
		radius_msg_make_authenticator(msg, (u8 *) sta, sizeof(*sta));

		os_snprintf(buf, sizeof(buf), "%08X-%08X",
			    sta->acct_session_id_hi, sta->acct_session_id_lo);
		if (!radius_msg_add_attr(msg, RADIUS_ATTR_ACCT_SESSION_ID,
					 (u8 *) buf, os_strlen(buf))) {
			asd_printf(ASD_1X,MSG_DEBUG,"Could not add Acct-Session-Id\n");
			goto fail;
		}
	} else {
		radius_msg_make_authenticator(msg, (u8 *) wasd, sizeof(*wasd));
	}

	if (!radius_msg_add_attr_int32(msg, RADIUS_ATTR_ACCT_STATUS_TYPE,
				       status_type)) {
		asd_printf(ASD_1X,MSG_DEBUG,"Could not add Acct-Status-Type\n");
		goto fail;
	}

	if (!radius_msg_add_attr_int32(msg, RADIUS_ATTR_ACCT_AUTHENTIC,
				       wasd->conf->ieee802_1x ?
				       RADIUS_ACCT_AUTHENTIC_RADIUS :
				       RADIUS_ACCT_AUTHENTIC_LOCAL)) {
		asd_printf(ASD_1X,MSG_DEBUG,"Could not add Acct-Authentic\n");
		goto fail;
	}

	if (sta) {
		if(wasd->conf->wapi_radius_auth_enable != 1) {		
			val = ieee802_1x_get_identity(sta->eapol_sm, &len);
			if (!val) {
				os_snprintf(buf, sizeof(buf), RADIUS_ADDR_FORMAT,
			   			 MAC2STR(sta->addr));
				val = (u8 *) buf;
				len = os_strlen(buf);
				}

			if (!radius_msg_add_attr(msg, RADIUS_ATTR_USER_NAME, val,
						 len)) {
				asd_printf(ASD_1X,MSG_DEBUG,"Could not add User-Name\n");
				goto fail;
				}
		}else{
				if(!radius_msg_add_attr(msg, RADIUS_ATTR_USER_NAME,
						 (u8 *)sta->wapi_sta_info.serial_no.data, sta->wapi_sta_info.serial_no.length)) {
					asd_printf(ASD_1X,MSG_DEBUG,"Could not add User-Name\n");
					goto fail;		
				}
					
				//mahz add 2010.12.7
				if(!radius_msg_add_attr_user_password(msg, wasd->conf->user_passwd, wasd->conf->user_passwd_len,
							 wasd->conf->radius->acct_server->shared_secret, 
							  wasd->conf->radius->acct_server->shared_secret_len)) {
					asd_printf(ASD_1X,MSG_DEBUG,"Could not add User_passwd\n");
					goto fail;
				}
			}
	}
	//
	if (wasd->conf->own_ip_addr.af == AF_INET &&
	    !radius_msg_add_attr(msg, RADIUS_ATTR_NAS_IP_ADDRESS,
				 (u8 *) &wasd->conf->own_ip_addr.u.v4, 4)) {
		asd_printf(ASD_1X,MSG_DEBUG,"Could not add NAS-IP-Address\n");
		goto fail;
	}

#ifdef ASD_IPV6
	if (wasd->conf->own_ip_addr.af == AF_INET6 &&
	    !radius_msg_add_attr(msg, RADIUS_ATTR_NAS_IPV6_ADDRESS,
				 (u8 *) &wasd->conf->own_ip_addr.u.v6, 16)) {
		asd_printf(ASD_1X,MSG_DEBUG,"Could not add NAS-IPv6-Address\n");
		goto fail;
	}
#endif /* ASD_IPV6 */

	//mahz add 2010.11.30
	if (sta && !radius_msg_add_attr(msg, RADIUS_ATTR_FRAME_IP_ADDRESS,		
				 (u8 *) &sta->ip_addr, 4)) {
		asd_printf(ASD_1X,MSG_DEBUG,"Could not add FRAME-IP-Address in %s\n",__func__);
		goto fail;
	}
	//asd_printf(ASD_1X,MSG_DEBUG,"FRAME-IP-Address: %s\n",sta->in_addr);
	//	
	pthread_mutex_lock(&asd_g_hotspot_mutex);
	if((ASD_HOTSPOT[wasd->hotspot_id]!= NULL)&&(ASD_HOTSPOT[wasd->hotspot_id]->nasid_len != 0))
	{
		if (!radius_msg_add_attr(msg, RADIUS_ATTR_NAS_IDENTIFIER,
						 (u8 *) ASD_HOTSPOT[wasd->hotspot_id]->nas_identifier,
						 os_strlen(ASD_HOTSPOT[wasd->hotspot_id]->nas_identifier))) {
				asd_printf(ASD_1X,MSG_DEBUG,"Could not add NAS-Identifier\n");
				pthread_mutex_unlock(&asd_g_hotspot_mutex);
				goto fail;
			}
	}
	else{
		if (wasd->conf->nas_identifier &&
		    !radius_msg_add_attr(msg, RADIUS_ATTR_NAS_IDENTIFIER,
					 (u8 *) wasd->conf->nas_identifier,
					 os_strlen(wasd->conf->nas_identifier))) {
			asd_printf(ASD_1X,MSG_DEBUG,"Could not add NAS-Identifier\n");
			pthread_mutex_unlock(&asd_g_hotspot_mutex);
			goto fail;
		}
	}	

	if (sta &&
	    !radius_msg_add_attr_int32(msg, RADIUS_ATTR_NAS_PORT, sta->aid)) {
		asd_printf(ASD_1X,MSG_DEBUG,"Could not add NAS-Port\n");
		pthread_mutex_unlock(&asd_g_hotspot_mutex);
		goto fail;
	}

	if((ASD_HOTSPOT[wasd->hotspot_id]!= NULL)&&(ASD_HOTSPOT[wasd->hotspot_id]->nasid_len != 0))
	{
		if (!radius_msg_add_attr(msg, RADIUS_ATTR_NAS_PORT_ID,
					(u8 *) ASD_HOTSPOT[wasd->hotspot_id]->nas_port_id, 
					os_strlen(ASD_HOTSPOT[wasd->hotspot_id]->nas_port_id))) {
			asd_printf(ASD_1X,MSG_DEBUG,"Could not add Nas-Port-Id\n");
			pthread_mutex_unlock(&asd_g_hotspot_mutex);
			goto fail;
		}

	}
	else if(wasd->nas_port_id[0] != 0){
		//mahz add 2011.5.26
		memset(buf,0,sizeof(buf));
		memcpy(buf,wasd->nas_port_id,os_strlen(wasd->nas_port_id));
		buf[sizeof(buf) - 1] = '\0';
		if (!radius_msg_add_attr(msg, RADIUS_ATTR_NAS_PORT_ID, (u8 *) buf, os_strlen(buf))) {
			asd_printf(ASD_1X,MSG_DEBUG,"Could not add Nas-Port-Id\n");
			pthread_mutex_unlock(&asd_g_hotspot_mutex);
			goto fail;
		}
	}
	pthread_mutex_unlock(&asd_g_hotspot_mutex);
	wtpid = (wasd->BSSIndex)/L_BSS_NUM/L_RADIO_NUM;
	if(ASD_WTP_AP[wtpid])//qiuchen
	{		
		os_snprintf(buf, sizeof(buf), RADIUS_802_1X_ADDR_FORMAT ":%s",
				MAC2STR(ASD_WTP_AP[wtpid]->WTPMAC), wasd->conf->ssid.ssid);
	}
	else
	{
		os_snprintf(buf, sizeof(buf), RADIUS_802_1X_ADDR_FORMAT ":%s",
				    MAC2STR(wasd->own_addr), wasd->conf->ssid.ssid);
	}
	buf[sizeof(buf) - 1] = '\0';
	if (!radius_msg_add_attr(msg, RADIUS_ATTR_CALLED_STATION_ID,
				 (u8 *) buf, os_strlen(buf))) {
		asd_printf(ASD_1X,MSG_DEBUG,"Could not add Called-Station-Id\n");
		goto fail;
	}

	if (sta) {
		os_snprintf(buf, sizeof(buf), RADIUS_802_1X_ADDR_FORMAT,
			    MAC2STR(sta->addr));
		if (!radius_msg_add_attr(msg, RADIUS_ATTR_CALLING_STATION_ID,
					 (u8 *) buf, os_strlen(buf))) {
			asd_printf(ASD_1X,MSG_DEBUG,"Could not add Calling-Station-Id\n");
			goto fail;
		}
		if((ASD_SECURITY[SID] != NULL)&&(ASD_SECURITY[SID]->wired_radius == 1)){
			if (!radius_msg_add_attr_int32(
					msg, RADIUS_ATTR_NAS_PORT_TYPE,
					RADIUS_NAS_PORT_TYPE_IEEE_802_3)) {
				asd_printf(ASD_1X,MSG_DEBUG,"Could not add NAS-Port-Type\n");
				goto fail;
			}
		}else{
			if (!radius_msg_add_attr_int32(
					msg, RADIUS_ATTR_NAS_PORT_TYPE,
					RADIUS_NAS_PORT_TYPE_IEEE_802_11)) {
				asd_printf(ASD_1X,MSG_DEBUG,"Could not add NAS-Port-Type\n");
				goto fail;
			}
		}

		os_snprintf(buf, sizeof(buf), "CONNECT %d%sMbps %s",
			    radius_sta_rate(wasd, sta) / 2,
			    (radius_sta_rate(wasd, sta) & 1) ? ".5" : "",
			    radius_mode_txt(wasd));
		if (!radius_msg_add_attr(msg, RADIUS_ATTR_CONNECT_INFO,
					 (u8 *) buf, os_strlen(buf))) {
			asd_printf(ASD_1X,MSG_DEBUG,"Could not add Connect-Info\n");
			goto fail;
		}

		for (i = 0; ; i++) {
			val = ieee802_1x_get_radius_class(sta->eapol_sm, &len,
							  i);
			if (val == NULL)
				break;

			if (!radius_msg_add_attr(msg, RADIUS_ATTR_CLASS,
						 val, len)) {
				asd_printf(ASD_1X,MSG_DEBUG,"Could not add Class\n");
				goto fail;
			}
		}
	}

	return msg;

 fail:
	radius_msg_free(msg);
	os_free(msg);
	msg = NULL;
	return NULL;
}

#if 0
//mahz add for sta flow in acct msg
static int accounting_sta_update_stats_bak(struct asd_data *wasd,
				       struct sta_info *sta,
				       struct asd_sta_driver_data *data)
{
	data->rx_bytes = sta->rxbytes;
	data->tx_bytes = sta->txbytes;
	data->rx_packets = sta->rxpackets;
	data->tx_packets = sta->txpackets;

	int i = 0;
	u32 val = 1024*1024*1024;

	if(data->rx_bytes > val){
		sta->acct_input_gigawords = data->rx_bytes/val;
		i = data->rx_bytes/val;
		data->rx_bytes -= i*val;
	}
		
	if(data->tx_bytes > val){
		sta->acct_output_gigawords = data->tx_bytes/val;
		i = data->tx_bytes/val;
		data->tx_bytes -= i*val;
	}
	
	asd_logger(wasd, sta->addr, asd_MODULE_RADIUS,
		       asd_LEVEL_DEBUG, "updated TX/RX stats: "
		       "Acct-Input-Octets=%lu Acct-Input-Gigawords=%u "
		       "Acct-Output-Octets=%lu Acct-Output-Gigawords=%u",
		       data->rx_bytes, sta->acct_input_gigawords,
		       data->tx_bytes, sta->acct_output_gigawords);

	return 0;
}
//
#endif
static int accounting_sta_update_stats(struct asd_data *wasd,
				       struct sta_info *sta,
				       struct asd_sta_driver_data *data)
{
	/*if (asd_read_sta_data(wasd, data, sta->addr))
		return -1;

	if (sta->last_rx_bytes > data->rx_bytes)
		sta->acct_input_gigawords++;
	if (sta->last_tx_bytes > data->tx_bytes)
		sta->acct_output_gigawords++;
	sta->last_rx_bytes = data->rx_bytes;
	sta->last_tx_bytes = data->tx_bytes;

	asd_logger(wasd, sta->addr, asd_MODULE_RADIUS,
		       asd_LEVEL_DEBUG, "updated TX/RX stats: "
		       "Acct-Input-Octets=%lu Acct-Input-Gigawords=%u "
		       "Acct-Output-Octets=%lu Acct-Output-Gigawords=%u",
		       sta->last_rx_bytes, sta->acct_input_gigawords,
		       sta->last_tx_bytes, sta->acct_output_gigawords);

	return 0;*/
	
	data->rx_packets = (unsigned long)sta->rxpackets;
	data->tx_packets = (unsigned long)sta->txpackets;
	data->rx_bytes = (unsigned long )sta->rxbytes;
	data->tx_bytes =(unsigned long ) sta->txbytes;
	sta->acct_input_gigawords = (unsigned long )(sta->rxbytes>>32);
	sta->acct_output_gigawords = (unsigned long)(sta->txbytes>>32);

	asd_logger(wasd, sta->addr, asd_MODULE_RADIUS,
		       asd_LEVEL_DEBUG, "updated TX/RX stats: "
		       "Acct-Input-Octets=%lu Acct-Input-Gigawords=%d "
		       "Acct-Output-Octets=%lu Acct-Output-Gigawords=%d",
		       data->rx_bytes, sta->acct_input_gigawords,
		      data->tx_bytes, sta->acct_output_gigawords);
	return 0;
}


static void accounting_interim_update(void *circle_ctx, void *timeout_ctx)
{
	struct asd_data *wasd = circle_ctx;
	struct sta_info *sta = timeout_ctx;
	int interval;

	if (sta->acct_interim_interval) {
		accounting_sta_interim(wasd, sta);
		interval = sta->acct_interim_interval;
	} else {
		struct asd_sta_driver_data data;
		accounting_sta_update_stats(wasd, sta, &data);
		accounting_sta_interim(wasd, sta);				//mahz add for test 2010.12.9
		interval = ACCT_DEFAULT_UPDATE_INTERVAL;
	}

	circle_register_timeout(interval, 0, accounting_interim_update,
			       wasd, sta);
}


void accounting_sta_start(struct asd_data *wasd, struct sta_info *sta)
{
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"now in func %s\n",__func__); //weichao test
	struct radius_msg *msg;
	int interval;
	struct radius_client_info *client_info;
	if (sta->acct_session_started)
		return;

	//time(&sta->acct_session_start);
	get_sysruntime(&sta->acct_session_start);//qiuchen change it
	asd_sta_clear_stats(wasd, sta->addr);

	client_info = radius_client_get_sock(wasd->WlanID,0);	
	if(client_info == NULL)
	{
		asd_printf(ASD_1X,MSG_ERROR,"Find radius acct socket error,there is no socket exist!\n");
		return;
	}
	if (sta->acct_interim_interval)
		interval = sta->acct_interim_interval;
	else
		interval = ACCT_DEFAULT_UPDATE_INTERVAL;
	circle_register_timeout(interval, 0, accounting_interim_update,
			       wasd, sta);
	accounting_get_session_id(client_info, sta);
	sta->acct_session_started = 1;
	sta_acct_info_add(sta);
	//qiuchen change it
	if (is_secondary == 1)
		return;
	sta->last_rx_bytes = sta->last_tx_bytes = 0;
	sta->acct_input_gigawords = sta->acct_output_gigawords = 0;
	msg = accounting_msg(wasd, sta,client_info, RADIUS_ACCT_STATUS_TYPE_START);
	if ((msg)&&(is_secondary != 1))
		radius_client_send(client_info, msg, RADIUS_ACCT, sta);

	
}


void accounting_sta_report(struct asd_data *wasd, struct sta_info *sta,
			   int stop)
{
	struct radius_msg *msg;
	int cause = sta->acct_terminate_cause;
	struct asd_sta_driver_data data;
	struct radius_client_info *client_info = NULL;
	u32 gigawords;
	time_t now_sysrun;//qiuchen add it
	time_t session_time = 0;
	get_sysruntime(&now_sysrun);
	client_info = radius_client_get_sock( wasd->WlanID,0);
	if(client_info == NULL)
	{
		asd_printf(ASD_1X,MSG_ERROR,"Find radius acct socket error,there is no socket exist!\n");
		return;
	}
	if (is_secondary == 1)
		return;
	msg = accounting_msg(wasd, sta,client_info,
			     stop ? RADIUS_ACCT_STATUS_TYPE_STOP :
			     RADIUS_ACCT_STATUS_TYPE_INTERIM_UPDATE);
	if (!msg) {
		asd_printf(ASD_1X,MSG_DEBUG,"Could not create RADIUS Accounting message\n");
		return;
	}
	//qiuchen
	session_time = now_sysrun - sta->acct_session_start;
	if(asd_sta_idle_time_switch == 1){
		if(session_time > asd_sta_idle_time*3600)
			session_time = asd_sta_idle_time*3600;
	}
	if (!radius_msg_add_attr_int32(msg, RADIUS_ATTR_ACCT_SESSION_TIME,
				       //time(NULL) - sta->acct_session_start)) {
						session_time)) {//qiuchen 
		asd_printf(ASD_1X,MSG_DEBUG,"Could not add Acct-Session-Time\n");
		goto fail;
	}

	if (accounting_sta_update_stats(wasd, sta, &data) == 0) {
		if (!radius_msg_add_attr_int32(msg,
					       RADIUS_ATTR_ACCT_OUTPUT_PACKETS,
					       data.rx_packets)) {
			asd_printf(ASD_1X,MSG_DEBUG,"Could not add Acct-Input-Packets\n");
			goto fail;
		}
		if (!radius_msg_add_attr_int32(msg,
					       RADIUS_ATTR_ACCT_INPUT_PACKETS,
					       data.tx_packets)) {
			asd_printf(ASD_1X,MSG_DEBUG,"Could not add Acct-Output-Packets\n");
			goto fail;
		}
		if (!radius_msg_add_attr_int32(msg,
					       RADIUS_ATTR_ACCT_OUTPUT_OCTETS,
					       data.rx_bytes)) {
			asd_printf(ASD_1X,MSG_DEBUG,"Could not add Acct-Input-Octets\n");
			goto fail;
		}
		gigawords = sta->acct_input_gigawords;
#if __WORDSIZE == 64
		gigawords += data.rx_bytes >> 32;
#endif
		if (/*gigawords &&*/
		    !radius_msg_add_attr_int32(
			    msg, RADIUS_ATTR_ACCT_OUTPUT_GIGAWORDS,
			    gigawords)) {
			asd_printf(ASD_1X,MSG_DEBUG,"Could not add Acct-Input-Gigawords\n");
			goto fail;
		}
		if (!radius_msg_add_attr_int32(msg,
					       RADIUS_ATTR_ACCT_INPUT_OCTETS,
					       data.tx_bytes)) {
			asd_printf(ASD_1X,MSG_DEBUG,"Could not add Acct-Output-Octets\n");
			goto fail;
		}
		gigawords = sta->acct_output_gigawords;
#if __WORDSIZE == 64
		gigawords += data.tx_bytes >> 32;
#endif
		if (/*gigawords &&*/
		    !radius_msg_add_attr_int32(
			    msg, RADIUS_ATTR_ACCT_INPUT_GIGAWORDS,
			    gigawords)) {
			asd_printf(ASD_1X,MSG_DEBUG,"Could not add Acct-Output-Gigawords\n");
			goto fail;
		}
	}
	time_t now;
	time(&now);

	if (!radius_msg_add_attr_int32(msg, RADIUS_ATTR_EVENT_TIMESTAMP,
				       time(NULL))) {
		asd_printf(ASD_1X,MSG_DEBUG,"Could not add Event-Timestamp\n");
		goto fail;
	}

	if (circle_terminated())
		cause = RADIUS_ACCT_TERMINATE_CAUSE_ADMIN_REBOOT;

	if (stop && cause &&
	    !radius_msg_add_attr_int32(msg, RADIUS_ATTR_ACCT_TERMINATE_CAUSE,
				       cause)) {
		asd_printf(ASD_1X,MSG_DEBUG,"Could not add Acct-Terminate-Cause\n");
		goto fail;
	}
	//if(is_secondary != 1)
	radius_client_send(client_info, msg,
			   stop ? RADIUS_ACCT : RADIUS_ACCT_INTERIM,
			   sta);
	if(stop && gASDLOGDEBUG & BIT(1))
		asd_syslog_auteview_acct_stop(wasd,sta,&data,session_time);
	return;

 fail:
	radius_msg_free(msg);
	os_free(msg);
	msg = NULL;
}


void accounting_sta_interim(struct asd_data *wasd, struct sta_info *sta)
{
	if (sta->acct_session_started)
		accounting_sta_report(wasd, sta, 0);
}


void accounting_sta_stop(struct asd_data *wasd, struct sta_info *sta)
{
	asd_printf(ASD_DEFAULT,MSG_DEBUG,"now in func %s\n",__func__);
	if (sta->acct_session_started) {
		accounting_sta_report(wasd, sta, 1);
		circle_cancel_timeout(accounting_interim_update, wasd, sta);
		sta->acct_session_started = 0;
		//
		u8 acct_id[ACCT_ID_LEN+1] = {0};		
		os_snprintf((char *)acct_id, sizeof(acct_id), "%08X-%08X", sta->acct_session_id_hi, sta->acct_session_id_lo);			
		sta_acct_info_del(acct_id);
	}
}


void accounting_get_session_id(struct radius_client_info *client_info, struct sta_info *sta)
{
	sta->acct_session_id_lo = ACCT_SESSION_ID_LO++;
	if (ACCT_SESSION_ID_LO == 0) {
		ACCT_SESSION_ID_HI++;
	}
	sta->acct_session_id_hi = ACCT_SESSION_ID_HI;
}
void accounting_sta_get_id(struct asd_data *wasd, struct sta_info *sta)
{
	sta->acct_session_id_lo = wasd->acct_session_id_lo++;
	if (wasd->acct_session_id_lo == 0) {
		wasd->acct_session_id_hi++;
	}
	sta->acct_session_id_hi = wasd->acct_session_id_hi;
}


/* Process the RADIUS frames from Accounting Server */
static RadiusRxResult
accounting_receive(struct radius_msg *msg, struct radius_msg *req,
		   u8 *shared_secret, size_t shared_secret_len, void *data)
{
	if (msg->hdr->code != RADIUS_CODE_ACCOUNTING_RESPONSE) {
		asd_printf(ASD_1X,MSG_DEBUG,"Unknown RADIUS message code\n");
		return RADIUS_RX_UNKNOWN;
	}

	if (radius_msg_verify(msg, shared_secret, shared_secret_len, req, 0)) {
		asd_printf(ASD_1X,MSG_DEBUG,"Incoming RADIUS packet did not have correct "
		       "Authenticator - dropped\n");
		return RADIUS_RX_INVALID_AUTHENTICATOR;
	}

	return RADIUS_RX_PROCESSED;
}

#if 0
static void accounting_report_state(struct asd_data *wasd, int on)
{
	struct radius_msg *msg;

	if (!wasd->conf->radius->acct_server || wasd->radius == NULL)
		return;

	/* Inform RADIUS server that accounting will start/stop so that the
	 * server can close old accounting sessions. */
	msg = accounting_msg(wasd, NULL,
			     on ? RADIUS_ACCT_STATUS_TYPE_ACCOUNTING_ON :
			     RADIUS_ACCT_STATUS_TYPE_ACCOUNTING_OFF);
	if (!msg)
		return;

	if (!radius_msg_add_attr_int32(msg, RADIUS_ATTR_ACCT_TERMINATE_CAUSE,
				       RADIUS_ACCT_TERMINATE_CAUSE_NAS_REBOOT))
	{
		asd_printf(ASD_1X,MSG_DEBUG,"Could not add Acct-Terminate-Cause\n");
		radius_msg_free(msg);
		os_free(msg);
		return;
	}
	if(is_secondary != 1)
	radius_client_send(wasd->radius, msg, RADIUS_ACCT, NULL);
}
#endif

int accounting_init(const int  wlanid)
{
	/* Acct-Session-Id should be unique over reboots. If reliable clock is
	 * not available, this could be replaced with reboot counter, etc. */
	struct radius_client_info *radius_client;
	if(ASD_WLAN[wlanid]&&(ASD_WLAN[wlanid]->radius))
	{
		radius_client = ASD_WLAN[wlanid]->radius->acct;
		while(radius_client)
		{
			radius_client->acct_session_id_hi = time(NULL);
			if(radius_client_register(radius_client,accounting_receive,radius_client))
				return -1;
			radius_client = radius_client->next;
		}

	}

//	if((wasd->radius) && (!wasd->radius->accounting_on_disable))
//		accounting_report_state(wasd, 1);

	return 0;
}


void accounting_deinit(struct asd_data *wasd)
{
//	if((wasd->radius) && (!wasd->radius->accounting_on_disable))
//		accounting_report_state(wasd, 0);
}


int accounting_reconfig(struct asd_data *wasd,
			struct asd_config *oldconf)
{
	if (!wasd->radius_client_reconfigured)
		return 0;

	accounting_deinit(wasd);
	return accounting_init(wasd->WlanID);
}
