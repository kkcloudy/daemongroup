/* rdc_coa.c */

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include "nm_list.h"
#include "hashtable.h"
#include "eag_mem.h"
#include "eag_log.h"
#include "eag_blkmem.h"
#include "eag_thread.h"
#include "rdc_interface.h"
#include "eag_dbus.h"
#include "eag_time.h"
#include "limits2.h"
#include "eag_util.h"

#include "radius_packet.h"
#include "rdc_ins.h"
#include "rdc_server.h"
#include "rdc_client.h"
#include "rdc_coaconn.h"
#include "rdc_userconn.h"



struct rdc_coa {
	//int status;
	int sockfd;
	uint32_t ip;
	uint16_t port;
	eag_thread_t *t_read;
	eag_thread_master_t *master;
	rdc_ins_t *rdcins;
	rdc_userconn_db_t *userdb;
	rdc_coaconn_db_t *coadb;
	rdc_server_t *server;
	struct rdc_coa_radius_conf * radius_conf;
};





typedef enum {
	RDC_COA_READ,
} rdc_coa_event_t;

static void
rdc_coa_event(rdc_coa_event_t event,
					rdc_coa_t *coa);

static int
coa_process_request(rdc_coa_t *coa,
							uint32_t ip,
							uint16_t port,
							struct radius_packet_t *radpkt);

static struct radius_srv_coa *
rdc_coa_find_radius_srv_by_ip( struct  rdc_coa_radius_conf *radiusconf, const unsigned long ip);
static int
rdc_coa_send_DM_response(rdc_coa_t *coa,
							struct radius_packet_t * radius_packet,
							uint32_t ip,
							uint16_t port);

rdc_coa_t *
rdc_coa_new(void)
{
	rdc_coa_t *coa = NULL;

	coa = eag_malloc(sizeof(*coa));
	if (NULL == coa) {
		eag_log_err("rdc_coa_new eag_malloc failed");
		return NULL;
	}

	memset(coa, 0, sizeof(*coa));
	coa->sockfd = -1;
//	coa->proc_req_func = coa_process_request;
//	coa->proc_res_func = NULL;  // proc coa response
	
	eag_log_debug("coa", "coa new ok");
	return coa;
}

int
rdc_coa_free(rdc_coa_t *coa)
{
	if (NULL == coa) {
		eag_log_err("rdc_coa_free input error");
		return -1;
	}

	if (coa->sockfd >= 0) {
		close(coa->sockfd);
		coa->sockfd = -1;
	}
	eag_free(coa);

	eag_log_debug("coa", "coa free ok");
	return 0;
}


int
rdc_coa_set_userdb( rdc_coa_t *coa, rdc_userconn_db_t *userdb )
{
	if( NULL == coa || NULL == userdb ){
		eag_log_err("rdc_coa_set_userdb input err coa=%p userdb=%p", 
					coa, userdb );
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	coa->userdb = userdb;
	return EAG_RETURN_OK;
}


int
rdc_coa_start(rdc_coa_t *coa)
{
	int ret = 0;
	struct sockaddr_in addr = {0};
	char ipstr[32] = "";
	
	if (NULL == coa) {
		eag_log_err("rdc_coa_start input error");
		return EAG_ERR_RDC_SERVER_NULL;
	}
	/*
	if (1 == coa->status) {
		eag_log_err("rdc_coa_start already started");
		return -1;
	}
	*/
	if (coa->sockfd >= 0) {
		eag_log_err("rdc_coa_start already start fd(%d)", 
			coa->sockfd);
		return EAG_RETURN_OK;
	}
		
	coa->sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (coa->sockfd  < 0) {
		eag_log_err("Can't create coa dgram socket: %s",
			safe_strerror(errno));
		coa->sockfd = -1;
		return EAG_ERR_SOCKET_FAILED;
	}

	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(coa->port);
#ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
	addr.sin_len = sizeof(struct sockaddr_in);
#endif
	addr.sin_addr.s_addr = htonl(coa->ip);
#if 0
	ret = setsockopt(coa->sockfd, SOL_SOCKET, SO_REUSEADDR,
					&opt, sizeof(opt));
	if (0 != ret) {
		eag_log_err("Can't set REUSEADDR to coa socket(%d): %s",
			coa->sockfd, safe_strerror(errno));
	}
#endif
	ret  = bind(coa->sockfd, (struct sockaddr *)&addr, sizeof(addr));
	if (ret < 0) {
		eag_log_err("Can't bind to coa socket(%d): %s",
			coa->sockfd, safe_strerror(errno));
		close(coa->sockfd);
		coa->sockfd = -1;
		return EAG_ERR_SOCKET_BIND_FAILED;
	}

	rdc_coa_event(RDC_COA_READ, coa);
	
	eag_log_info("coa(%s:%u) fd(%d) start ok", 
			ip2str(coa->ip, ipstr, sizeof(ipstr)),
			coa->port,
			coa->sockfd);
	
	//coa->status = 1;

	return EAG_RETURN_OK;
}

int
rdc_coa_stop(rdc_coa_t *coa)
{
	char ip[32] = "";

	if (NULL == coa) {
		eag_log_err("rdc_coa_stop input error");
		return EAG_ERR_RDC_SERVER_NULL;
	}
	/*
	if (0 == coa->status) {
		eag_log_err("rdc_coa_stop already stoped");
		return -1;
	}
	*/
	if (NULL != coa->t_read) {
		eag_thread_cancel(coa->t_read);
		coa->t_read = NULL;
	}
	if (coa->sockfd >= 0)
	{
		close(coa->sockfd);
		coa->sockfd = -1;
	}
	eag_log_info("coa(%s:%u) stop ok",
			ip2str(coa->ip, ip, sizeof(ip)),
			coa->port);

	//coa->status = 0;

	return EAG_RETURN_OK;
}



static int
coa_receive(eag_thread_t *thread)
{
	rdc_coa_t *coa = NULL;
	struct sockaddr_in addr = {0};
	socklen_t len = 0;
	ssize_t nbyte = 0;
	struct radius_packet_t radpkt = {0};
	char ipstr[32] = "";
	uint32_t ip = 0;
	uint16_t port = 0;
	
	if (NULL == thread) {
		eag_log_err("coa_receive input error");
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	
	coa = eag_thread_get_arg(thread);
	if (NULL == coa) {
		eag_log_err("coa_receive coa null");
		return EAG_ERR_NULL_POINTER;
	}

	len = sizeof(addr);
	nbyte = recvfrom(coa->sockfd, &radpkt, sizeof(radpkt), 0,
					(struct sockaddr *)&addr, &len);
	if (nbyte < 0) {
		eag_log_err("coa_receive recvfrom failed: %s",
			safe_strerror (errno));
		return EAG_ERR_SOCKET_RECV_FAILED;
	}

	ip = ntohl(addr.sin_addr.s_addr);
	port = ntohs(addr.sin_port);
	eag_log_debug("coa", "coa fd(%d) receive %d bytes from %s:%u",
		coa->sockfd,
		nbyte,
		ip2str(ip, ipstr, sizeof(ipstr)),
		port);
			
	if (nbyte > RDC_PACKET_SIZE-RDC_PACKET_HEADSIZE) {
		eag_log_err("coa_receive packet size %d > max %d",
			nbyte, RADIUS_PACKSIZE-RDC_PACKET_HEADSIZE);
		return -1;
	}
	
	if( nbyte != htons(radpkt.length)){
		eag_log_err("coa_receive get size(%ul) != pkt length(%ul)",
					nbyte, htons(radpkt.length));
		return -1;
	}
	
	if( RADIUS_CODE_COA_REQUEST != radpkt.code 
		&& RADIUS_CODE_DISCONNECT_REQUEST != radpkt.code ){
		eag_log_err("coa_receive get an error radpkt!");
		return -1;
	}

	return coa_process_request( coa, ip, port, &radpkt );
}


static int
rdc_coa_send_DM_response(rdc_coa_t *coa,
								struct radius_packet_t * radius_packet,
								uint32_t ip,
								uint16_t port)
{
	struct sockaddr_in addr = {0};
	size_t length = 0;
	ssize_t nbyte = 0;
	
	if (NULL == coa  || NULL == radius_packet) {
		eag_log_err("rdc_coa_send_DM_response input error");
		return   EAG_ERR_INPUT_PARAM_ERR;
	}
	
	length = ntohs(radius_packet->length);
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
#ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
	addr.sin_len = sizeof(struct sockaddr_in);
#endif
	addr.sin_addr.s_addr = htonl(ip);

	nbyte = sendto(coa->sockfd, radius_packet, length,
				0, (struct sockaddr *)&addr, sizeof(addr));
	if (nbyte < 0) {
		eag_log_err("rdc_coa_send_DM_NAK sendto failed: %s",
			safe_strerror(errno));
		return   EAG_ERR_FILE_WRITE_FAILED;
	}

	eag_log_info("rdc_coa send DM_NAK ok");
	return EAG_RETURN_OK;
}

static int  
rdc_coa_process_NAK(rdc_coa_t *coa,struct radius_packet_t *radiuscoa,
						struct radius_srv_coa * radius_srv,
						uint32_t dst_ip,
						uint16_t dst_port){
	struct radius_packet_t radius_pack={0};
	struct radius_attr_t *ma = NULL;	/* Message authenticator */
	char radius_secret[RADIUS_SECRETSIZE]={0};
	memcpy(radius_secret,radius_srv->auth_secret,sizeof(radius_secret)-1);
	int  secret_len=radius_srv->auth_secretlen;
	int ret=0;
		
	if(RADIUS_CODE_COA_REQUEST == radiuscoa->code)
	{
		radius_pack.code=RADIUS_CODE_COA_NAK;
	}
	else
	{
		radius_pack.code=RADIUS_CODE_DISCONNECT_NAK;
	}
	radius_pack.id = radiuscoa->id;
	radius_pack.length = htons(RADIUS_HDRSIZE);
	radius_addattr(&(radius_pack), RADIUS_ATTR_ERROR_CAUSE, 0, 0,
			RADIUS_ERROR_CAUSE_503, NULL, 0);

	/* Prepare for message authenticator TODO */
	memset(radius_pack.authenticator, 0, RADIUS_AUTHLEN);
	memcpy(radius_pack.authenticator,
	       radiuscoa->authenticator, RADIUS_AUTHLEN);

	/* If packet contains message authenticator: Calculate it! */
	if (!radius_getattr(radiuscoa, &ma,
	     RADIUS_ATTR_MESSAGE_AUTHENTICATOR, 0, 0, 0)) {
		radius_hmac_md5(&(radius_pack), radius_secret,
				secret_len, ma->v.t);
	}
	radius_authresp_authenticator(&(radius_pack),
			      radiuscoa->authenticator, 
			      radius_secret,secret_len);
	admin_log_notice("RadiusDisconnectNAK,RADIUS_ERROR_CAUSE_503");
	ret=rdc_coa_send_DM_response(coa,&(radius_pack),dst_ip,dst_port);
	return ret;
}

static rdc_userconn_t *
rdc_coa_find_userconn( rdc_coa_t *coa,
			struct radius_packet_t *radpkt )
{
	struct radius_attr_t *attr = NULL;
	char ipstr[32] = "";
	uint32_t user_ip=0;

	char sessionid[MAX_SESSION_ID_LEN]="";
	char user_name[USERNAMESIZE]="";
	
	/* User-Name */
	if (!radius_getattr(radpkt, &attr, RADIUS_ATTR_USER_NAME, 0, 0, 0)) {
		memset(user_name, 0, sizeof(user_name));
		memcpy(user_name, attr->v.t, attr->l-2);
		eag_log_debug("userconn", "rdc_coa_find_userconn get coa username=%s",
					user_name );
	}else{
		eag_log_warning("rdc_coa_find_userconn cannot get username!");
	}

	/* Framed-IP-Address */
	if (!radius_getattr(radpkt, &attr, RADIUS_ATTR_FRAMED_IP_ADDRESS, 0, 0, 0)) {
		user_ip = ntohl(attr->v.i);
		ip2str(user_ip, ipstr, sizeof(ipstr));
		eag_log_debug("userconn", "rdc_coa_find_userconn get coa user ip %s",
					ipstr );
	}else{
		eag_log_warning("rdc_coa_find_userconn cannot get user ip!");
	}
	
	/* Acct-Session-ID */
	if (!radius_getattr(radpkt, &attr, RADIUS_ATTR_ACCT_SESSION_ID, 0, 0, 0)) {
		memcpy(sessionid, attr->v.t, attr->l-2);
		eag_log_debug("userconn", "rdc_coa_find_userconn get coa sessionid %s",
					sessionid );
	}else{
		eag_log_warning("rdc_coa_find_userconn can not get sessionid!");
	}

	
	return rdc_userconn_db_find_user( coa->userdb, 
									user_name, sessionid, user_ip);
}


static int
coa_process_request(rdc_coa_t *coa,
							uint32_t ip,
							uint16_t port,
							struct radius_packet_t *radpkt)
{
	
	struct rdc_packet_t rdcpkt = {0};
	uint8_t id = 0;
	uint32_t coaip;
	uint16_t coaport;
	rdc_coaconn_t *coaconn = NULL;
	rdc_userconn_t *userconn = NULL;
	uint16_t radpktlen = 0;
	struct radius_srv_coa* radius_srv  =NULL;
	
	if (NULL == coa || NULL == radpkt) {
		eag_log_err("coa_process_request input error");
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	if (NULL == coa->rdcins) {
		eag_log_err("coa_process_request rdcins null");
		return -1;
	}
	
	radpktlen = ntohs(radpkt->length);
	if( radpktlen > sizeof(rdcpkt.data) ){
		eag_log_err("coa_process_request radpktlen(%d) > rdcpkt.data(%d)",
					radpktlen, sizeof(rdcpkt.data) );
		return -1;
	}	

	id = radpkt->id;
	coaconn = rdc_coaconn_db_lookup(coa->coadb, ip, port, id);
	if( NULL == coaconn ){
		userconn = rdc_coa_find_userconn(coa,radpkt);
		if( NULL == userconn ){
			eag_log_debug("coa_process_request","RDC response directly!");
			radius_srv=rdc_coa_find_radius_srv_by_ip(coa->radius_conf,ip);
			if( NULL != radius_srv )
			{
				rdc_coa_process_NAK(coa,radpkt,radius_srv,ip,port);
			}
			return -1;
		}
		
		coaconn = rdc_coaconn_new(coa->coadb, ip, port, id);
		if( NULL == coaconn ){
			eag_log_err("coa_process_request create new coaconn failed!");
			return -1;
		}

		
		rdc_userconn_get_client( userconn, &coaip/* &coaconn->coaip*/, &coaport/*&coaconn->coaport*/);
		rdc_coaconn_set_coaip( coaconn, coaip );
		rdc_coaconn_set_coaport( coaconn, coaport );
		rdc_coaconn_set_server(coaconn,rdc_ins_get_server(coa->rdcins));

		if( 0 != rdc_coaconn_db_add( coa->coadb, coaconn )){
			eag_log_err("coa_process_request add coaconn to db failed!");
			rdc_coaconn_free( coaconn );
			return -1;
		}
		rdc_coaconn_wait(coaconn);
	}

	memset( &rdcpkt, 0, sizeof(rdcpkt));
	memcpy( rdcpkt.data, radpkt, radpktlen );
	//rdcpkt.ip = htonl(coaip);
	rdcpkt.ip = htonl(ip);
	//rdcpkt.port = htons(coaport);
	rdcpkt.port = htons(port);
	rdcpkt.length = htons(radpktlen+RDC_PACKET_HEADSIZE);

	rdcpkt.slotid = rdc_ins_get_slot_id(coa->rdcins);
	rdcpkt.hansi_type = rdc_ins_get_hansi_type(coa->rdcins);
	rdcpkt.hansi_id = rdc_ins_get_hansi_id(coa->rdcins);
	rdcpkt.client_type = RDC_SELF;

	snprintf((char *)(rdcpkt.desc), sizeof(rdcpkt.desc),
		"SLOT%u,%sHansi%uRDC",
		rdcpkt.slotid,
		HANSI_LOCAL==rdcpkt.hansi_type?"Local":"Remote",
		rdcpkt.hansi_id);

	rdc_coaconn_inc_res(coaconn);
	rdc_server_send_coa_request( coa->server, coaconn, &rdcpkt );
	
	return EAG_RETURN_OK;
}

int
rdc_coa_send_response(rdc_coa_t *coa,
								rdc_coaconn_t *coaconn,
								struct rdc_packet_t *rdcpkt)
{
	struct radius_packet_t *radpkt=NULL;
	struct sockaddr_in addr = {0};
	size_t length = 0;
	ssize_t nbyte = 0;
	uint32_t ip = 0;
	uint16_t port = 0;
	
	if (NULL == coa || NULL == coaconn || NULL == rdcpkt) {
		eag_log_err("rdc_coa_send_response input error");
		return -1;
	}

	ip = ntohl(rdcpkt->ip);
	port = ntohs(rdcpkt->port);

	radpkt = (struct radius_packet_t *)rdcpkt->data;
	
	length = ntohs(radpkt->length);
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
#ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
	addr.sin_len = sizeof(struct sockaddr_in);
#endif
	addr.sin_addr.s_addr = htonl(ip);

	nbyte = sendto(coa->sockfd, radpkt, length,
				0, (struct sockaddr *)&addr, sizeof(addr));
	if (nbyte < 0) {
		eag_log_err("rdc_coa_send_response sendto failed: %s",
			safe_strerror(errno));
		return -1;
	}

	eag_log_debug("coa", "coa send response ok");
	return EAG_RETURN_OK;
}

int 
rdc_coa_set_thread_master(rdc_coa_t *coa,
								eag_thread_master_t *master)
{
	if (NULL == coa) {
		eag_log_err("rdc_coa_set_thread_master input error");
		return -1;
	}

	coa->master = master;

	return 0;
}

int 
rdc_coa_set_rdcins(rdc_coa_t *coa,
						rdc_ins_t *rdcins)
{
	if (NULL == coa) {
		eag_log_err("rdc_coa_set_rdcins input error");
		return -1;
	}

	coa->rdcins = rdcins;

	return 0;
}

int 
rdc_coa_set_nasip(rdc_coa_t *coa,
						uint32_t ip)
{
	char ipstr[32] = "";
	
	if (NULL == coa) {
		eag_log_err("rdc_coa_set_nasip input error");
		return -1;
	}

	coa->ip = ip;

	eag_log_info("coa set nasip %s",
		ip2str(ip, ipstr, sizeof(ipstr)));
	return 0;
}

int 
rdc_coa_set_port(rdc_coa_t *coa,
						uint16_t port)
{
	if (NULL == coa) {
		eag_log_err("rdc_coa_set_port input error");
		return -1;
	}

	coa->port = port;
	
	eag_log_info("coa set port %u", port);
	return 0;
}

int
rdc_coa_set_coadb( rdc_coa_t *coa,
					rdc_coaconn_db_t *coadb )
{
	if (NULL == coa) {
		eag_log_err("rdc_coa_set_coadb input error");
		return -1;
	}

	coa->coadb = coadb;
	
	eag_log_info("coa set coadb %p", coadb);
	return 0;
}

int
rdc_coa_set_server( rdc_coa_t *coa,
					rdc_server_t *server )
{
	if (NULL == coa) {
		eag_log_err("rdc_coa_set_server input error");
		return -1;
	}

	coa->server = server;
	
	eag_log_info("coa set server %p", server);
	return 0;
}

int 
rdc_coa_set_radiusconf( rdc_coa_t *this, struct rdc_coa_radius_conf *radiusconf )
{
	if(NULL == this ){
		eag_log_err("rdc_coa_set_radiusconf input error");
		return -1;
	}
	this->radius_conf = radiusconf;
	eag_log_info("coa set radiusconf %p", radiusconf);
	return  0;
}

static void
rdc_coa_event(rdc_coa_event_t event,
					rdc_coa_t *coa)
{
	if (NULL == coa) {
		eag_log_err("rdc_coa_event input error");
		return;
	}
	
	switch (event) {
	case RDC_COA_READ:
		coa->t_read =
		    eag_thread_add_read(coa->master, coa_receive,
					coa, coa->sockfd);
		break;
	default:
		break;
	}
}

static struct radius_srv_coa *  
rdc_coa_find_radius_srv_by_ip( struct  rdc_coa_radius_conf *radiusconf, const unsigned long ip)
{
	int i=0;
	struct radius_srv_coa *srv = NULL;
	char ip_str[32]="";
	
	if( NULL == radiusconf || 0 == ip ){
		eag_log_err("rdc_coa_check_radius_srv input param error!");
		return NULL;
	}
	
	for( i=0; i<radiusconf->current_num; i++ ){
		srv = &(radiusconf->radius_srv[i]);
		if(ip == srv->auth_ip){
			ip2str(ip,ip_str,sizeof(ip_str));
			eag_log_debug("rdc_coa","find radius server ip=%s",ip_str);
			return  srv;
		}
	}
	eag_log_warning("can't find the radius server by ip when rdc directly response DM");
	return NULL;
}

struct  radius_srv_coa  *
rdc_coa_check_radius_srv( struct  rdc_coa_radius_conf *radiusconf, const unsigned long ip,const unsigned short  port, char * secret)
{
	int i;
	struct radius_srv_coa *srv = NULL;

	if( NULL == radiusconf || NULL == secret
			|| strlen(secret)==0 
			|| strlen(secret)>sizeof(radiusconf->radius_srv[0].auth_secret)-1 ){
		eag_log_err("rdc_coa_check_radius_srv input param error!");
		return NULL;
	}
	
	for( i=0; i<radiusconf->current_num; i++ ){
		srv = &(radiusconf->radius_srv[i]);
		if( 0 == strcmp(secret, srv->auth_secret) 
			&& (ip == srv->auth_ip)
			&& (port == srv->auth_port)){
			return srv;
		}
	}

	return NULL;
}

struct radius_srv_coa *
rdc_coa_conf_radius_srv(struct rdc_coa_radius_conf *radiusconf)
{
	struct radius_srv_coa *srv = NULL;
	if( NULL == radiusconf ){
		return NULL;
	}

	if( radiusconf->current_num >= MAX_RADIUS_SRV_NUM ){
		return NULL;
	}
	srv = &(radiusconf->radius_srv[radiusconf->current_num]);
	
	radiusconf->current_num ++;

	return srv;
}

int
rdc_coa_set_radius_srv(struct radius_srv_coa *radius_srv,
		    unsigned long auth_ip,
		    unsigned short auth_port,
		    char *auth_secret, unsigned int auth_secretlen)
{
	if (auth_secretlen > sizeof (radius_srv->auth_secret)) {
		return -1;
	}
	radius_srv->auth_ip = auth_ip;
	radius_srv->auth_port = auth_port;
	memset(radius_srv->auth_secret, 0, RADIUS_SECRETSIZE);
	memcpy(radius_srv->auth_secret, auth_secret, auth_secretlen);
	radius_srv->auth_secretlen = auth_secretlen;

	return 0;
}

int
rdc_coa_del_radius_srv( struct rdc_coa_radius_conf *radiusconf,unsigned long auth_ip, unsigned short auth_port,char *auth_secret )
{
	struct radius_srv_coa *srv = NULL;
	int i=0;
	
	if( NULL == radiusconf || NULL == auth_secret
			|| strlen(auth_secret)==0 
			|| strlen(auth_secret)>sizeof(radiusconf->radius_srv[0].auth_secret)-1 ){
		return -1;
	}

	if( radiusconf->current_num == 0 ){
		return -1;
	}

	for( i=0; i<radiusconf->current_num; i++ ){
		srv = &(radiusconf->radius_srv[i]);
		if( 0 == strcmp(auth_secret, srv->auth_secret)
			&& (auth_ip == srv->auth_ip)
			&& (auth_port == srv->auth_port)){
			radiusconf->current_num--;
			memcpy( srv, srv+1, sizeof(struct radius_srv_coa)*(radiusconf->current_num-i));
			return 0;
		}
	}
	
	return -1;
}

