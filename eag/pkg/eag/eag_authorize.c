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
* portal_ha.c
*
*
* CREATOR:
* autelan.software.xxx. team
*
* DESCRIPTION:
* xxx module main routine
*
*
*******************************************************************************/
#include <stdlib.h>
#include <dbus/dbus.h>
#include <stdio.h>
#include <string.h>
#include "limits2.h"
#include "eag_util.h"
#include "eag_errcode.h"
#include "eag_log.h"
#include "eag_conf.h"
#include "eag_interface.h"
#include "session.h"
#include "eag_wireless.h"
#include "eag_captive.h"
#include "eag_blkmem.h"
#include "hashtable.h"
#include "appconn.h"
#include "eag_ipset.h"
#include "eag_ipset6.h" /* add by houyongtao */
#include "eag_iptables.h"
#include "eag_ip6tables.h" /* add by houyongtao */
#include "eag_authorize.h"



struct eag_authorize{
	eag_captive_t *cap;
	DBusConnection *dbusconn;
	int (*do_authorize)( eag_authorize_t *this, struct appsession *appsession );
	int (*de_authorize)( eag_authorize_t *this, struct appsession *appsession );
	#if 0
	int (*do_eap_authorize)( eag_authorize_t *this, unsigned int user_ip);
	int (*del_eap_authorize)( eag_authorize_t *this, unsigned int user_ip);
	#endif
	int (*do_macpre_authorize)( eag_authorize_t *this, user_addr_t *user_addr);
	int (*del_macpre_authorize)( eag_authorize_t *this, user_addr_t *user_addr);
	int (*do_flux)( eag_authorize_t *this, struct appsession *appsession );
	void *param;
};

static eag_authorize_t eag_authorize_ipset;
static eag_authorize_t eag_authorize_iptable;

int 
eag_authorize_do_authorize( eag_authorize_t *this, struct appsession *appsession )
{
	if( NULL != this && NULL != this->do_authorize ){
		return this->do_authorize(this, appsession );
	}

	eag_log_err("eag_authorize_do_authorize param error this=%p "\
				"this->do_authorize=%p", this, (this==NULL)?NULL:this->do_authorize);
	return EAG_ERR_INPUT_PARAM_ERR;
}

int 
eag_authorize_de_authorize( eag_authorize_t *this, struct appsession *appsession )
{
	if( NULL != this && NULL != this->de_authorize ){
		return this->de_authorize(this, appsession );
	}

	eag_log_err("eag_authorize_de_authorize param error this=%p "\
				"this->de_authorize=%p", this, (this==NULL)?NULL:this->de_authorize);
	return EAG_ERR_INPUT_PARAM_ERR;
}
#if 0
int 
eag_authorize_do_eap_authorize( eag_authorize_t *this, unsigned int user_ip )
{
	if( NULL != this && NULL != this->do_eap_authorize ){
		return this->do_eap_authorize(this, user_ip );
	}

	eag_log_err("eag_authorize_do_eap_authorize param error this=%p "\
				"this->do_eap_authorize=%p", this, (this==NULL)?NULL:this->do_eap_authorize);
	return EAG_ERR_INPUT_PARAM_ERR;
}

int 
eag_authorize_del_eap_authorize( eag_authorize_t *this, unsigned int user_ip )
{
	if( NULL != this && NULL != this->del_eap_authorize ){
		return this->del_eap_authorize(this, user_ip );
	}

	eag_log_err("eag_authorize_del_eap_authorize param error this=%p "\
				"this->del_eap_authorize=%p", this, (this==NULL)?NULL:this->del_eap_authorize);
	return EAG_ERR_INPUT_PARAM_ERR;
}
#endif
int 
eag_authorize_do_macpre_authorize( eag_authorize_t *this, user_addr_t *user_addr )
{
	if( NULL != this && NULL != this->do_macpre_authorize){
		return this->do_macpre_authorize(this, user_addr );
	}

	eag_log_err("eag_authorize_do_macpre_authorize param error this=%p "\
				"this->do_macpre_authorize=%p", this, (this==NULL)?NULL:this->do_macpre_authorize);
	return EAG_ERR_INPUT_PARAM_ERR;
}

int 
eag_authorize_del_macpre_authorize( eag_authorize_t *this, user_addr_t *user_addr )
{
	if( NULL != this && NULL != this->del_macpre_authorize){
		return this->del_macpre_authorize(this, user_addr );
	}

	eag_log_err("eag_authorize_del_macpre_authorize param error this=%p "\
				"this->del_macpre_authorize=%p", this, (this==NULL)?NULL:this->del_macpre_authorize);
	return EAG_ERR_INPUT_PARAM_ERR;
}

int 
eag_authorize_do_flux( eag_authorize_t *this, struct appsession *appsession )
{
	if( NULL != this && NULL != this->do_flux ){
		return this->do_flux(this, appsession );
	}

	eag_log_err("eag_authorize_do_flux param error this=%p "\
				"this->do_flux=%p", this, (this==NULL)?NULL:this->do_flux);
	return EAG_ERR_INPUT_PARAM_ERR;
}

eag_authorize_t *
eag_authorize_get_ipset_auth()
{
	return &eag_authorize_ipset;
}

eag_authorize_t *
eag_authorieze_get_iptables_auth()
{
	return &eag_authorize_iptable;
}



static int 
eag_authorize_ipset_do_authorize( eag_authorize_t *this, struct appsession *appsession )
{
	if( NULL == this ) {
        eag_log_err("eag_authorize_ipset_do_authorize this = %p", this);
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	if( NULL == this->cap ){
		eag_log_err("eag_authorize_ipset_do_authorize this = %p this->cap =%p",
					this, this->cap);
		return EAG_ERR_INPUT_PARAM_ERR;	
	}
	if( NULL == appsession ){
		eag_log_err("eag_authorize_ipset_do_authorize appsession = NULL");
		return EAG_ERR_INPUT_PARAM_ERR;	
	}
	#if 0
	if( 0 == appsession->user_ip ){
		char ipstr[32];
		ip2str( appsession->user_ip, ipstr, sizeof(ipstr) );
		eag_log_err("eag_authorize_ipset_do_authorize appsession if=%s ip=%s",
					appsession->intf, ipstr );
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	return add_user_to_set( eag_captive_get_capid(this->cap),
							eag_captive_get_hansitype(this->cap),
							appsession->user_ip );
	#endif /* modify by houyongtao */
	return set_user_in_ipset6( eag_captive_get_capid(this->cap),
							eag_captive_get_hansitype(this->cap),
							&(appsession->user_addr), 
							IPSET6_CMD_ADD );
}

static int 
eag_authorize_ipset_de_authorize( eag_authorize_t *this, struct appsession *appsession )
{
    if( NULL == this ){
		eag_log_err( "eag_authorize_ipset_de_authorize this = %p", this);
		return EAG_ERR_INPUT_PARAM_ERR; 
	}
	if( NULL == this->cap ){
		eag_log_err( "eag_authorize_ipset_de_authorize this = %p  this->cap = %p",
					this, this->cap );
		return EAG_ERR_INPUT_PARAM_ERR;	
	}
	if( NULL == appsession ){
		eag_log_err( "eag_authorize_ipset_de_authorize appsession = NULL" );
		return EAG_ERR_INPUT_PARAM_ERR;	
	}
	#if 0
	if( strlen(appsession->intf)==0 || 0 == appsession->user_ip ){
		char ipstr[32];
		ip2str( appsession->user_ip, ipstr, sizeof(ipstr) );
		eag_log_err( "eag_authorize_ipset_de_authorize appsession if=%s ip=%s",
						appsession->intf, ipstr );
		return EAG_ERR_INPUT_PARAM_ERR;	
	}
	return del_user_from_set( eag_captive_get_capid(this->cap),
								eag_captive_get_hansitype(this->cap),
								appsession->user_ip );
	#endif /* modify by houyongtao */
	return set_user_in_ipset6( eag_captive_get_capid(this->cap),
								eag_captive_get_hansitype(this->cap),
								&(appsession->user_addr), 
								IPSET6_CMD_DEL );
}

static int 
eag_authorize_ipset_do_flux( eag_authorize_t *this, struct appsession *appsession )
{
	return EAG_RETURN_OK;
}
#if 0
static int 
eag_authorize_ipset_do_eap_authorize( eag_authorize_t *this, unsigned int user_ip )
{
	return EAG_RETURN_OK;
}

static int 
eag_authorize_ipset_del_eap_authorize( eag_authorize_t *this, unsigned int user_ip )
{
	return EAG_RETURN_OK;
}
#endif
static int 
eag_authorize_ipset_do_macpre_authorize( eag_authorize_t *this, user_addr_t *user_addr)
{
    if( NULL == this || NULL == user_addr){
		eag_log_err("eag_authorize_ipset_do_macpre_authorize this = %p, user_addr = %p", this, user_addr);
		return EAG_ERR_INPUT_PARAM_ERR; 
	}
	if( NULL == this->cap ){
		eag_log_err("eag_authorize_ipset_do_macpre_authorize this = %p this->cap =%p",
					this, this->cap);
		return EAG_ERR_INPUT_PARAM_ERR;	
	}
	#if 0
	unsigned int user_ip = user_addr->user_ip;
	if( 0 == user_ip ){
		char ipstr[32];
		ip2str( user_ip, ipstr, sizeof(ipstr) );
		eag_log_err("eag_authorize_ipset_do_macpre_authorize ip=%s", ipstr );
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	return add_preauth_user_to_set( eag_captive_get_capid(this->cap),
							eag_captive_get_hansitype(this->cap),
							user_ip );
	#endif /* modify by houyongtao */
	return set_preauth_user_in_ipset6( eag_captive_get_capid(this->cap),
									eag_captive_get_hansitype(this->cap),
									user_addr, 
									IPSET6_CMD_ADD );
	//return EAG_RETURN_OK;
}

static int 
eag_authorize_ipset_del_macpre_authorize( eag_authorize_t *this, user_addr_t *user_addr )
{
    if( NULL == this || NULL == user_addr){
		eag_log_err("eag_authorize_ipset_del_macpre_authorize this = %p, user_addr = %p", this, user_addr);
		return EAG_ERR_INPUT_PARAM_ERR; 
	}
	if( NULL == this->cap ){
		eag_log_err( "eag_authorize_ipset_del_macpre_authorize this = %p  this->cap = %p",
					this, this->cap );
		return EAG_ERR_INPUT_PARAM_ERR;	
	}
	#if 0
	unsigned int user_ip = user_addr->user_ip;
	if( 0 == user_ip ){
		char ipstr[32];
		ip2str( user_ip, ipstr, sizeof(ipstr) );
		eag_log_err( "eag_authorize_ipset_del_macpre_authorize ip=%s", ipstr );
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	return del_preauth_user_from_set( eag_captive_get_capid(this->cap),
								eag_captive_get_hansitype(this->cap),
								user_ip );
	#endif /* modify by houyongtao */
	return set_preauth_user_in_ipset6( eag_captive_get_capid(this->cap),
									eag_captive_get_hansitype(this->cap),
									user_addr, 
									IPSET6_CMD_DEL );
	//return EAG_RETURN_OK;
}

static int 
eag_authorize_iptables_do_authorize( eag_authorize_t *this, struct appsession *appsession )
{
	char user_ipstr[IPX_LEN] = "";
	int ret = 0;
	
    if( NULL == this ){
		eag_log_err("eag_authorize_iptables_do_authorize this = %p", this);
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	if( NULL == this->cap ){
		eag_log_err("eag_authorize_iptables_do_authorize this = %p  this->cap = %p",
					this, this->cap);
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	if( NULL == appsession ){
		eag_log_err("eag_authorize_iptables_do_authorize appsession = NULL");
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	ipx2str(&(appsession->user_addr), user_ipstr, sizeof(user_ipstr));
	if( strlen(appsession->intf)==0 || 0 == memcmp_ipx(&(appsession->user_addr), NULL)){
		eag_log_err("eag_authorize_iptables_do_authorize appsession if=%s ip=%s",
						appsession->intf, user_ipstr);
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	if (EAG_IPV6 == appsession->user_addr.family
		|| EAG_MIX == appsession->user_addr.family) {
		ret = ipv6_connect_up(eag_captive_get_capid(this->cap), 
                        eag_captive_get_hansitype(this->cap), 
                        appsession->intf, 
                        &(appsession->user_addr.user_ipv6));
	}
	if (EAG_IPV4 == appsession->user_addr.family
		|| EAG_MIX == appsession->user_addr.family) {
		ret = connect_up(eag_captive_get_capid(this->cap), 
                    eag_captive_get_hansitype(this->cap), 
                    appsession->intf, 
                    appsession->user_addr.user_ip);
	}

	return ret;
}

static int 
eag_authorize_iptables_de_authorize( eag_authorize_t *this, struct appsession *appsession )
{
	char user_ipstr[IPX_LEN] = "";
	int ret = 0;
	
    if( NULL == this ){
		eag_log_err("eag_authorize_iptables_de_authorize this = %p", this);
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	if( NULL == this->cap ){
		eag_log_err("eag_authorize_iptables_de_authorize this = %p  this->cap = %p",
					this, this->cap);
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	if( NULL == appsession ){
		eag_log_err("eag_authorize_iptables_de_authorize appsession = NULL");
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	
	ipx2str(&(appsession->user_addr), user_ipstr, sizeof(user_ipstr));
	
	if( strlen(appsession->intf)==0 || 0 == memcmp_ipx(&(appsession->user_addr), NULL)){
		eag_log_err("eag_authorize_iptables_de_authorize appsession if=%s ip=%s",
						appsession->intf, user_ipstr);
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	if (EAG_IPV6 == appsession->user_addr.family
		|| EAG_MIX == appsession->user_addr.family) {
		ret = ipv6_connect_down(eag_captive_get_capid(this->cap), 
	                            eag_captive_get_hansitype(this->cap), 
	            				appsession->intf, 
	            				&(appsession->user_addr.user_ipv6));
	}
	if (EAG_IPV4 == appsession->user_addr.family
		|| EAG_MIX == appsession->user_addr.family) {
		ret = connect_down( eag_captive_get_capid(this->cap), 
                            eag_captive_get_hansitype(this->cap), 
							appsession->intf, 
							appsession->user_addr.user_ip );
	}
	
	return ret;
}
#if 0
static int 
eag_authorize_iptables_do_eap_authorize( eag_authorize_t *this, unsigned int user_ip )
{
    if( NULL == this ){
		eag_log_err("eag_authorize_iptables_do_eap_authorize this = %p", this);
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	if( NULL == this->cap ){
		eag_log_err("eag_authorize_iptables_do_eap_authorize this = %p  this->cap = %p",
					this, this->cap);
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	if( 0 == user_ip ){
		eag_log_err("eag_authorize_iptables_do_eap_authorize user_ip = 0");
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	return eap_connect_up(  eag_captive_get_capid(this->cap), 
						eag_captive_get_hansitype(this->cap), user_ip);
}

static int 
eag_authorize_iptables_del_eap_authorize( eag_authorize_t *this, unsigned int user_ip )
{
    if( NULL == this ){
		eag_log_err("eag_authorize_iptables_del_eap_authorize this = %p", this);
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	if( NULL == this->cap ){
		eag_log_err("eag_authorize_iptables_del_eap_authorize this = %p  this->cap = %p",
					this, this->cap);
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	if( 0 == user_ip ){
		eag_log_err("eag_authorize_iptables_del_eap_authorize  user_ip = 0");
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	return eap_connect_down( eag_captive_get_capid(this->cap), 
						eag_captive_get_hansitype(this->cap), user_ip);

}
#endif
static int 
eag_authorize_iptables_do_macpre_authorize( eag_authorize_t *this, user_addr_t *user_addr )
{
	char user_ipstr[IPX_LEN] = "";
	int ret = 0;
	
    if( NULL == this || NULL == user_addr){
		eag_log_err("eag_authorize_iptables_do_macpre_authorize this = %p, user_addr = %p", this, user_addr);
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	if( NULL == this->cap ){
		eag_log_err("eag_authorize_iptables_do_macpre_authorize this = %p  this->cap = %p",
					this, this->cap);
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	if(0 == memcmp_ipx(user_addr, NULL)){
		eag_log_err("eag_authorize_iptables_do_macpre_authorize user_ip = 0");
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	ipx2str(user_addr, user_ipstr, sizeof(user_ipstr));
	
	eag_log_debug("eag_macauth", "macpre_connect_up BEGIN!user_ip:%s", user_ipstr);
	if (EAG_IPV6 == user_addr->family
		|| EAG_MIX == user_addr->family) {
		ret = ipv6_macpre_connect_up(eag_captive_get_capid(this->cap), 
			                        eag_captive_get_hansitype(this->cap), 
			                        &(user_addr->user_ipv6));
	}
	if (EAG_IPV4 == user_addr->family
		|| EAG_MIX == user_addr->family) {
		ret = macpre_connect_up(eag_captive_get_capid(this->cap), 
								eag_captive_get_hansitype(this->cap), 
								user_addr->user_ip);
	}
	
	return ret;
}

static int 
eag_authorize_iptables_del_macpre_authorize( eag_authorize_t *this, user_addr_t *user_addr )
{
	char user_ipstr[IPX_LEN] = "";
	int ret = 0;
	
    if( NULL == this || NULL == user_addr){
		eag_log_err("eag_authorize_ipset_del_macpre_authorize this = %p, user_addr = %p", this, user_addr);
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	if( NULL == this->cap ){
		eag_log_err("eag_authorize_iptables_del_macpre_authorize this = %p  this->cap = %p",
					this, this->cap);
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	if(0 == memcmp_ipx(user_addr, NULL)){
		eag_log_err("eag_authorize_iptables_do_macpre_authorize user_ip = 0");
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	ipx2str(user_addr, user_ipstr, sizeof(user_ipstr));
	
	eag_log_debug("eag_macauth", "macpre_connect_down BEGIN!user_ip:%s", user_ipstr);
	if (EAG_IPV6 == user_addr->family
		|| EAG_MIX == user_addr->family) {
		ret = ipv6_macpre_connect_down(eag_captive_get_capid(this->cap), 
			                        eag_captive_get_hansitype(this->cap), 
			                        &(user_addr->user_ipv6));
	}
	if (EAG_IPV4 == user_addr->family
		|| EAG_MIX == user_addr->family) {
		ret = macpre_connect_down(eag_captive_get_capid(this->cap), 
								eag_captive_get_hansitype(this->cap), 
								user_addr->user_ip);
	}
	
	return ret;
}

static int 
eag_authorize_iptables_do_flux( eag_authorize_t *this, struct appsession *appsession )
{
	return EAG_RETURN_OK;
}


int
eag_authorize_ipset_auth_init( eag_captive_t *cap,DBusConnection *dbusconn, void *param )
{
	memset( &eag_authorize_ipset, 0, sizeof(eag_authorize_ipset));
	eag_authorize_ipset.cap = cap;
	eag_authorize_ipset.dbusconn = dbusconn;
	eag_authorize_ipset.do_authorize = eag_authorize_ipset_do_authorize;
	eag_authorize_ipset.de_authorize = eag_authorize_ipset_de_authorize;
	#if 0
	eag_authorize_ipset.do_eap_authorize = eag_authorize_ipset_do_eap_authorize;
	eag_authorize_ipset.del_eap_authorize = eag_authorize_ipset_del_eap_authorize;
	#endif
	eag_authorize_ipset.do_macpre_authorize = eag_authorize_ipset_do_macpre_authorize;
	eag_authorize_ipset.del_macpre_authorize = eag_authorize_ipset_del_macpre_authorize;
	eag_authorize_ipset.do_flux = eag_authorize_ipset_do_flux;

	eag_ipset_init();
	eag_ipset6_init(); /* add by houyongtao */

	return EAG_RETURN_OK;
}

int
eag_authorize_ipset_auth_uninit(  )
{
	eag_ipset_exit();
	eag_ipset6_exit(); /* add by houyongtao */
	
	return EAG_RETURN_OK;
}

int
eag_authorize_iptables_auth_init( eag_captive_t *cap,
						DBusConnection *dbusconn, void *param )
{
	memset( &eag_authorize_iptable, 0, sizeof(eag_authorize_iptable));
	eag_authorize_iptable.cap = cap;
	eag_authorize_iptable.dbusconn = dbusconn;
	eag_authorize_iptable.do_authorize = eag_authorize_iptables_do_authorize;
	eag_authorize_iptable.de_authorize = eag_authorize_iptables_de_authorize;
	#if 0
	eag_authorize_iptable.do_eap_authorize = eag_authorize_iptables_do_eap_authorize;
	eag_authorize_iptable.del_eap_authorize = eag_authorize_iptables_del_eap_authorize;
	#endif
	eag_authorize_iptable.do_macpre_authorize = eag_authorize_iptables_do_macpre_authorize;
	eag_authorize_iptable.del_macpre_authorize = eag_authorize_iptables_del_macpre_authorize;
	eag_authorize_iptable.do_flux = eag_authorize_iptables_do_flux;	
	return EAG_RETURN_OK;
}

int
eag_authorize_iptables_auth_uninit( )
{
	return EAG_RETURN_OK;
}



