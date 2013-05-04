/* rdc_server.c */

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
#include "rdc_packet.h"
#include "rdc_ins.h"
#include "rdc_server.h"
#include "rdc_client.h"
#include "rdc_pktconn.h"
#include "rdc_userconn.h"
#include "rdc_coaconn.h"
#include "rdc_coa.h"

typedef int (*rdc_server_proc_req_func_t)(rdc_server_t *,
		uint32_t,
		uint16_t,
		struct rdc_packet_t *);

typedef int (*rdc_server_proc_res_func_t)(rdc_server_t *,
		uint32_t,
		uint16_t,
		struct rdc_packet_t *);

struct rdc_server {
	//int status;
	int sockfd;
	uint32_t ip;
	uint16_t port;
	eag_thread_t *t_read;
	eag_thread_master_t *master;
	rdc_ins_t *rdcins;
	rdc_server_proc_req_func_t proc_req_func;
	rdc_server_proc_res_func_t proc_res_func;

	rdc_userconn_db_t *userdb;
};

typedef enum {
	RDC_SERVER_READ,
} rdc_server_event_t;

static void
rdc_server_event(rdc_server_event_t event,
					rdc_server_t *server);

static int
server_process_request(rdc_server_t *server,
						uint32_t ip,
						uint16_t port,
						struct rdc_packet_t *rdcpkt);
static int
server_process_coa_response(rdc_server_t *server,
							uint32_t ip,
							uint16_t port,
							struct rdc_packet_t *rdcpkt);

rdc_server_t *
rdc_server_new(void)
{
	rdc_server_t *server = NULL;

	server = eag_malloc(sizeof(*server));
	if (NULL == server) {
		eag_log_err("rdc_server_new eag_malloc failed");
		return NULL;
	}

	memset(server, 0, sizeof(*server));
	server->sockfd = -1;
	server->proc_req_func = server_process_request;
	server->proc_res_func = server_process_coa_response;  // proc coa response
	
	eag_log_debug("server", "server new ok");
	return server;
}

int
rdc_server_free(rdc_server_t *server)
{
	if (NULL == server) {
		eag_log_err("rdc_server_free input error");
		return -1;
	}

	if (server->sockfd >= 0) {
		close(server->sockfd);
		server->sockfd = -1;
	}
	eag_free(server);

	eag_log_debug("server", "server free ok");
	return 0;
}


int
rdc_server_set_userdb( rdc_server_t *server, rdc_userconn_db_t *userdb )
{
	if( NULL == server || NULL == userdb ){
		eag_log_err("rdc_server_set_userdb input err server=%p userdb=%p", 
					server, userdb );
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	server->userdb = userdb;
	return EAG_RETURN_OK;
}


int
rdc_server_start(rdc_server_t *server)
{
	int ret = 0;
	struct sockaddr_in addr = {0};
	char ipstr[32] = "";
	
	if (NULL == server) {
		eag_log_err("rdc_server_start input error");
		return EAG_ERR_RDC_SERVER_NULL;
	}
	/*
	if (1 == server->status) {
		eag_log_err("rdc_server_start already started");
		return -1;
	}
	*/
	if (server->sockfd >= 0) {
		eag_log_err("rdc_server_start already start fd(%d)", 
			server->sockfd);
		return EAG_RETURN_OK;
	}
		
	server->sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (server->sockfd  < 0) {
		eag_log_err("Can't create server dgram socket: %s",
			safe_strerror(errno));
		server->sockfd = -1;
		return EAG_ERR_SOCKET_FAILED;
	}

	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(server->port);
#ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
	addr.sin_len = sizeof(struct sockaddr_in);
#endif
	addr.sin_addr.s_addr = htonl(server->ip);

	#if 0
	ret = setsockopt(server->sockfd, SOL_SOCKET, SO_REUSEADDR,
					&opt, sizeof(opt));
	if (0 != ret) {
		eag_log_err("Can't set REUSEADDR to server socket(%d): %s",
			server->sockfd, safe_strerror(errno));
	}
	#endif
	ret  = bind(server->sockfd, (struct sockaddr *)&addr, sizeof(addr));
	if (ret < 0) {
		eag_log_err("Can't bind to server socket(%d): %s",
			server->sockfd, safe_strerror(errno));
		close(server->sockfd);
		server->sockfd = -1;
		return EAG_ERR_SOCKET_BIND_FAILED;
	}

	rdc_server_event(RDC_SERVER_READ, server);
	
	eag_log_info("server(%s:%u) fd(%d) start ok", 
			ip2str(server->ip, ipstr, sizeof(ipstr)),
			server->port,
			server->sockfd);
	
	//server->status = 1;

	return EAG_RETURN_OK;
}

int
rdc_server_stop(rdc_server_t *server)
{
	char ip[32] = "";

	if (NULL == server) {
		eag_log_err("rdc_server_stop input error");
		return EAG_ERR_RDC_SERVER_NULL;
	}
	/*
	if (0 == server->status) {
		eag_log_err("rdc_server_stop already stoped");
		return -1;
	}
	*/
	if (NULL != server->t_read) {
		eag_thread_cancel(server->t_read);
		server->t_read = NULL;
	}
	if (server->sockfd >= 0)
	{
		close(server->sockfd);
		server->sockfd = -1;
	}
	eag_log_info("server(%s:%u) stop ok",
			ip2str(server->ip, ip, sizeof(ip)),
			server->port);

	//server->status = 0;

	return EAG_RETURN_OK;
}

static int
rdc_server_record_userconn( rdc_userconn_db_t *userdb,
					struct radius_packet_t *radpkt, 
					uint32_t ip, uint16_t coa_port )
{
	struct radius_attr_t *attr = NULL;
	char ipstr[32] = "";
	char coaip[32] = "";
	uint32_t user_ip=0;
	rdc_userconn_t *userconn=NULL;

	char sessionid[MAX_SESSION_ID_LEN]="";
	char user_name[USERNAMESIZE]="";
	int acct_state;
	
	struct timeval tv;
	uint32_t timenow;
	eag_time_gettimeofday( &tv, NULL );
	timenow = tv.tv_sec;

	/* User-Name */
	if (!radius_getattr(radpkt, &attr, RADIUS_ATTR_USER_NAME, 0, 0, 0)) {
		memset(user_name, 0, sizeof(user_name));
		memcpy(user_name, attr->v.t, attr->l-2);
	}else{
		eag_log_err("rdc_server_record_userconn cannot get username!");
	}

	/* Framed-IP-Address */
	if (!radius_getattr(radpkt, &attr, RADIUS_ATTR_FRAMED_IP_ADDRESS, 0, 0, 0)) {
		user_ip = ntohl(attr->v.i);
		ip2str(user_ip, ipstr, sizeof(ipstr));
	}else{
		eag_log_err("rdc_server_record_userconn cannot get user ip!");
	}
	
	/* Acct-Session-ID */
	if (!radius_getattr(radpkt, &attr, RADIUS_ATTR_ACCT_SESSION_ID, 0, 0, 0)) {
		memcpy(sessionid, attr->v.t, attr->l-2);
	}else{
		eag_log_err("rdc_server_record_userconn can not get sessionid!");
	}

	if (!radius_getattr(radpkt, &attr, RADIUS_ATTR_ACCT_STATUS_TYPE, 0, 0, 0)) {
		acct_state = ntohl(attr->v.i);
	}else{
		eag_log_err("rdc_server_record_userconn acct pkg has no acct status!");
		return EAG_ERR_UNKNOWN;
	}

	ip2str(ip, coaip, sizeof(coaip));
	eag_log_debug("userconn","record userconn user:%s %s %s %s ip:%s coa:%u",
					user_name, ipstr, sessionid, 
					(RADIUS_STATUS_TYPE_START==acct_state)?"start":
						((RADIUS_STATUS_TYPE_STOP==acct_state)?"stop":"interval"),
					coaip, coa_port );
		
	userconn = rdc_userconn_db_find_user( userdb, user_name, sessionid, user_ip);
	if( NULL != userconn  ){
		//rdc_userconn_set_client( userconn, ip, coa_port );
		rdc_userconn_set_last_time( userconn, timenow );
		rdc_userconn_set_last_acct_status( userconn, acct_state );
		if( RADIUS_STATUS_TYPE_START == acct_state ){
			eag_log_warning("rdc_server_record_userconn get acct"\
							" start but userconn already exist!");
		}else if( RADIUS_STATUS_TYPE_STOP == acct_state ){
			eag_log_debug("userconn","get stop for userconn %s:%s", 
							rdc_userconn_get_username(userconn), 
							rdc_userconn_get_sessionid(userconn) );
			rdc_userconn_db_del( userdb, userconn );
			rdc_userconn_free( userdb, userconn );
		}
	}else{
		if( RADIUS_STATUS_TYPE_START != acct_state ){
			eag_log_warning("rdc_server_record_userconn get acct update or "\
						"stop but userconn not exist!");
			eag_log_warning("rdc_server_record_userconn user is"\
						" %s  %s  %s", user_name, sessionid, ipstr );
		}
		if( RADIUS_STATUS_TYPE_STOP == acct_state ){
			eag_log_warning("get stop and userconn not exist, might stop"\
							" pkg is resend. do not record the user!");
			return EAG_RETURN_OK;
		}
		userconn = rdc_userconn_db_find_sessionid( userdb, sessionid );
		if( NULL != userconn ){
			eag_log_warning("rdc_server_record_userconn get same "\
						"session but username or userip different!");
		}else{
			userconn = rdc_userconn_new( userdb, user_name,sessionid,user_ip);
			if( NULL == userconn ){
				eag_log_err("rdc_server_record_userconn rdc_userconn_new failed"\
						" for %s  %s  %s", user_name, sessionid, ipstr );
				return EAG_ERR_MALLOC_FAILED;
			}
			rdc_userconn_set_client( userconn, ip, coa_port );
			rdc_userconn_set_start_time( userconn, timenow );
			rdc_userconn_set_last_time( userconn, timenow );
			rdc_userconn_set_last_acct_status( userconn, acct_state );
			rdc_userconn_db_add( userdb, userconn );
		}
	}


	
	return EAG_RETURN_OK;
}


static int
server_receive(eag_thread_t *thread)
{
	rdc_server_t *server = NULL;
	struct sockaddr_in addr = {0};
	socklen_t len = 0;
	ssize_t nbyte = 0;
	struct rdc_packet_t rdcpkt = {0};
	struct radius_packet_t *radpkt = NULL;
	char ipstr[32] = "";
	uint32_t ip = 0;
	uint16_t port = 0;
	
	if (NULL == thread) {
		eag_log_err("server_receive input error");
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	
	server = eag_thread_get_arg(thread);
	if (NULL == server) {
		eag_log_err("server_receive server null");
		return EAG_ERR_NULL_POINTER;
	}

	len = sizeof(addr);
	nbyte = recvfrom(server->sockfd, &rdcpkt, sizeof(rdcpkt), 0,
					(struct sockaddr *)&addr, &len);
	if (nbyte < 0) {
		eag_log_err("server_receive recvfrom failed: %s",
			safe_strerror (errno));
		return EAG_ERR_SOCKET_RECV_FAILED;
	}

	ip = ntohl(addr.sin_addr.s_addr);
	port = ntohs(addr.sin_port);
	eag_log_debug("server", "server fd(%d) receive %d bytes from %s:%u",
		server->sockfd,
		nbyte,
		ip2str(ip, ipstr, sizeof(ipstr)),
		port);
			
	if (nbyte < RDC_PACKET_MINSIZE) {
		eag_log_warning("server_receive packet size %d < min %d",
			nbyte, RDC_PACKET_MINSIZE);
		return -1;
	}

	if (nbyte > RDC_PACKET_SIZE) {
		eag_log_warning("server_receive packet size %d > max %d",
			nbyte, RDC_PACKET_SIZE);
		return -1;
	}

	radpkt = (struct radius_packet_t *)(&(rdcpkt.data));
	if (nbyte != ntohs(radpkt->length) + RDC_PACKET_HEADSIZE) {
		eag_log_warning("server_receive packet size %d != len %d",
			nbyte, ntohs(radpkt->length) + RDC_PACKET_HEADSIZE);
		return -1;
	}

	if (RADIUS_CODE_ACCESS_REQUEST == radpkt->code
		|| RADIUS_CODE_ACCOUNTING_REQUEST == radpkt->code) {
		if (NULL != server->proc_req_func) {
			server->proc_req_func(server, ip, port, &rdcpkt);
		}
		else {
			eag_log_warning("server_receive proc_req_func null");
		}
		if( RADIUS_CODE_ACCOUNTING_REQUEST == radpkt->code ){
			rdc_server_record_userconn( server->userdb,
						radpkt, ip, ntohs(rdcpkt.coa_port) );
		}
	}
	else if (RADIUS_CODE_COA_ACK == radpkt->code
		|| RADIUS_CODE_COA_NAK == radpkt->code
		|| RADIUS_CODE_DISCONNECT_ACK == radpkt->code
		|| RADIUS_CODE_DISCONNECT_NAK == radpkt->code) {
		if (NULL != server->proc_res_func) {
			server->proc_res_func(server, ip, port, &rdcpkt);
		}
		else {
			eag_log_warning("server_receive recv_res_func null");
		}
	}
	else {
		eag_log_warning("server_receive unexpected packet code %d",
			radpkt->code);
		return -1;
	}

	return EAG_RETURN_OK;
}

static int
server_process_request(rdc_server_t *server,
							uint32_t ip,
							uint16_t port,
							struct rdc_packet_t *rdcpkt)
{
	struct radius_packet_t *radpkt = NULL;
	uint32_t toip = 0;
	uint16_t toport = 0;
	uint16_t coaport = 0;
	uint8_t id = 0;
	char ipstr[32] = "";
	char toipstr[32] = "";
	rdc_ins_t *rdcins = NULL;
	rdc_pktconn_db_t *db = NULL;
	rdc_client_t *client = NULL;
	rdc_pktconn_t *pktconn = NULL;
	char desc[64] = "";
	
	if (NULL == server || NULL == rdcpkt) {
		eag_log_err("server_process_request input error");
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	toip = ntohl(rdcpkt->ip);
	toport = ntohs(rdcpkt->port);
	coaport = ntohs(rdcpkt->coa_port);
	radpkt = (struct radius_packet_t *)(&(rdcpkt->data));
	id = radpkt->id;
	/*
	eag_log_debug("server", "server process request "
		"pktconn(%s,%u,%u) coaport(%u) radius(%s:%u)",
		ip2str(ip, ipstr, sizeof(ipstr)),
		port,
		id,
		coaport,
		ip2str(toip, ipstr, sizeof(toipstr)),
		toport);
	*/
	memcpy(desc, rdcpkt->desc, sizeof(rdcpkt->desc));
	eag_log_debug("server", "server process request "
		"pktconn(%s,%u,%u) coaport(%u) radius(%s:%u) "
		"length(%u) slotid(%u) hansiType(%u) hansiId(%u) "
		"clientType(%d) desc(%s)",
		ip2str(ip, ipstr, sizeof(ipstr)),
		port,
		id,
		coaport,
		ip2str(toip, toipstr, sizeof(toipstr)),
		toport,
		ntohs(rdcpkt->length),
		rdcpkt->slotid,
		rdcpkt->hansi_type,
		rdcpkt->hansi_id,
		rdcpkt->client_type,
		desc);
	
	rdcins = server->rdcins;
	if (NULL == rdcins) {
		eag_log_err("server_process_request rdcins null");
		return -1;
	}
	
	db = rdc_ins_get_pktconn_db(rdcins);
	if (NULL == db) {
		eag_log_err("server_process_request db null");
		return -1;
	}

	client = rdc_ins_get_client(rdcins);
	if (NULL == client) {
		eag_log_err("server_process_request client null");
		return -1;
	}
	
	pktconn = rdc_pktconn_db_lookup(db, ip, port, id);
	if (NULL == pktconn) {
		eag_log_debug("server", "server process request "
			"not find pktconn(%s,%u,%u) in pktconn db",
			ip2str(ip, ipstr, sizeof(ipstr)),
			port,
			id);
		pktconn = rdc_pktconn_new(db, ip, port, id);
		if (NULL == pktconn) {
			eag_log_err("server_process_request pktconn_new failed");
			return -1;
		}
		rdc_pktconn_db_add(db, pktconn);
		rdc_pktconn_wait(pktconn);
	}

	rdc_pktconn_inc_req(pktconn);
	
	rdc_client_send_request(client, pktconn, rdcpkt);
	
	eag_log_debug("server", "server process request ok");
	return EAG_RETURN_OK;
}


static int
server_process_coa_response(rdc_server_t *server,
							uint32_t ip,
							uint16_t port,
							struct rdc_packet_t *rdcpkt)
{
	struct radius_packet_t *radpkt = NULL;
	uint32_t toip = 0;
	uint16_t toport = 0;
	uint8_t id = 0;
	char ipstr[32] = "";
	char toipstr[32] = "";
	rdc_ins_t *rdcins = NULL;
	rdc_coaconn_db_t *coadb = NULL;
	rdc_coaconn_t *coaconn = NULL;
	rdc_coa_t *coa = NULL;
	char desc[64] = "";
	
	if (NULL == server || NULL == rdcpkt) {
		eag_log_err("server_process_request input error");
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	toip = ntohl(rdcpkt->ip);
	toport = ntohs(rdcpkt->port);
	radpkt = (struct radius_packet_t *)(&(rdcpkt->data));
	id = radpkt->id;
	/*
	eag_log_debug("server", "server process request "
		"pktconn(%s,%u,%u) coaport(%u) radius(%s:%u)",
		ip2str(ip, ipstr, sizeof(ipstr)),
		port,
		id,
		coaport,
		ip2str(toip, ipstr, sizeof(toipstr)),
		toport);
	*/
	memcpy(desc, rdcpkt->desc, sizeof(rdcpkt->desc));
	eag_log_debug("coa:server", "server process request "
		"pktconn(%s,%u,%u) radius(%s:%u) "
		"length(%u) slotid(%u) hansiType(%u) hansiId(%u) "
		"clientType(%d) desc(%s)",
		ip2str(ip, ipstr, sizeof(ipstr)),
		port,
		id,
		ip2str(toip, toipstr, sizeof(toipstr)),
		toport,
		ntohs(rdcpkt->length),
		rdcpkt->slotid,
		rdcpkt->hansi_type,
		rdcpkt->hansi_id,
		rdcpkt->client_type,
		desc);
	
	rdcins = server->rdcins;
	if (NULL == rdcins) {
		eag_log_err("server_process_request rdcins null");
		return -1;
	}
	
	coadb = rdc_ins_get_coaconn_db(rdcins);
	if (NULL == coadb) {
		eag_log_err("server_process_request coadb null");
		return -1;
	}
	coa = rdc_ins_get_coa( rdcins );
	if( NULL == coa ){
		eag_log_err("server_process_request coa is null");
		return -1;
	}
	
	coaconn = rdc_coaconn_db_lookup(coadb, toip, toport, id);
	if (NULL == coaconn) {
		eag_log_err("server process coa response "
			"not find coaconn(%s,%u,%u) in pktconn db",
			ip2str(toip, ipstr, sizeof(ipstr)),
			toport, 
			id);
		return -1;
	}

	rdc_coaconn_inc_req(coaconn);
	
	rdc_coa_send_response(coa, coaconn, rdcpkt);
	
	eag_log_debug("server", "server process coa response ok");
	return EAG_RETURN_OK;
}



int
rdc_server_send_response(rdc_server_t *server,
								rdc_pktconn_t *pktconn,
								struct rdc_packet_t *rdcpkt)
{
	struct sockaddr_in addr = {0};
	size_t length = 0;
	ssize_t nbyte = 0;
	uint32_t ip = 0;
	uint16_t port = 0;
	
	if (NULL == server || NULL == pktconn || NULL == rdcpkt) {
		eag_log_err("rdc_server_send_response input error");
		return -1;
	}

	ip = rdc_pktconn_get_ip(pktconn);
	port = rdc_pktconn_get_port(pktconn);
	
	length = ntohs(rdcpkt->length);
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
#ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
	addr.sin_len = sizeof(struct sockaddr_in);
#endif
	addr.sin_addr.s_addr = htonl(ip);

	nbyte = sendto(server->sockfd, rdcpkt, length,
				0, (struct sockaddr *)&addr, sizeof(addr));
	if (nbyte < 0) {
		eag_log_err("rdc_server_send_response sendto failed: %s",
			safe_strerror(errno));
		return -1;
	}

	eag_log_debug("server", "server send response ok");
	return EAG_RETURN_OK;
}

int
rdc_server_send_coa_request( rdc_server_t *server,
						rdc_coaconn_t *coaconn,
						struct rdc_packet_t *rdcpkt )
{
	char ipstr[32];
	struct sockaddr_in addr = {0};
	size_t length = 0;
	ssize_t nbyte = 0;
	uint32_t coaip;
	uint16_t coaport;

	if (NULL == server || NULL == coaconn || NULL == rdcpkt) {
		eag_log_err("rdc_server_send_coa_request input error");
		return -1;
	}

	
	coaip = rdc_coaconn_get_coaip(coaconn);
	coaport = rdc_coaconn_get_coaport(coaconn);
	length = ntohs(rdcpkt->length);
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(coaport);
#ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
	addr.sin_len = sizeof(struct sockaddr_in);
#endif
	addr.sin_addr.s_addr = htonl(coaip);

	nbyte = sendto(server->sockfd, rdcpkt, length,
				0, (struct sockaddr *)&addr, sizeof(addr));
	if (nbyte < 0) {
		eag_log_err("rdc_server_send_response sendto failed: %s",
			safe_strerror(errno));
		return -1;
	}

	ip2str(coaip,ipstr,sizeof(ipstr));
	eag_log_info("rdc_server_send_coa_request to %s:%u", ipstr, coaport );
	eag_log_debug("server", "server send coa request ok");
	return EAG_RETURN_OK;
}


int 
rdc_server_set_thread_master(rdc_server_t *server,
								eag_thread_master_t *master)
{
	if (NULL == server) {
		eag_log_err("rdc_server_set_thread_master input error");
		return -1;
	}

	server->master = master;

	return 0;
}

int 
rdc_server_set_rdcins(rdc_server_t *server,
						rdc_ins_t *rdcins)
{
	if (NULL == server) {
		eag_log_err("rdc_server_set_rdcins input error");
		return -1;
	}

	server->rdcins = rdcins;

	return 0;
}

int 
rdc_server_set_ip(rdc_server_t *server,
						uint32_t ip)
{
	char ipstr[32] = "";
	
	if (NULL == server) {
		eag_log_err("rdc_server_set_ip input error");
		return -1;
	}

	server->ip = ip;

	eag_log_info("server set ip %s",
		ip2str(ip, ipstr, sizeof(ipstr)));
	return 0;
}

int 
rdc_server_set_port(rdc_server_t *server,
						uint16_t port)
{
	if (NULL == server) {
		eag_log_err("rdc_server_set_port input error");
		return -1;
	}

	server->port = port;
	
	eag_log_info("server set port %u", port);
	return 0;
}

static void
rdc_server_event(rdc_server_event_t event,
					rdc_server_t *server)
{
	if (NULL == server) {
		eag_log_err("rdc_server_event input error");
		return;
	}
	
	switch (event) {
	case RDC_SERVER_READ:
		server->t_read =
		    eag_thread_add_read(server->master, server_receive,
					server, server->sockfd);
		break;
	default:
		break;
	}
}

