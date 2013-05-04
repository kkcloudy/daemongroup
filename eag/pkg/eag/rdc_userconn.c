/* rdc_userconn.c */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <error.h>
#include <mcheck.h>

#include "eag_errcode.h"
#include "nm_list.h"
#include "eag_log.h"
#include "eag_mem.h"
#include "eag_blkmem.h"
#include "limits2.h"
#include "eag_util.h"
#include "hashtable.h"
#include "eag_thread.h"

#include "radius_packet.h"
#include "rdc_ins.h"
#include "rdc_userconn.h"

#define USERCOON_BLKMEM_SIZE		512

struct rdc_userconn_db {
	int num;
	struct list_head head;
	eag_blk_mem_t *userblkmem;
};

struct rdc_userconn {
	struct list_head node;
	rdc_userconn_db_t *db;
	char username[USERNAMESIZE];
//	unsigned char usermac[PKT_ETH_ALEN];
	char sessionid[MAX_SESSION_ID_LEN];
	uint32_t user_ip;
	uint32_t client_ip;
	uint16_t client_coa_port;
	uint32_t start_time;
	uint32_t last_time;
	int acct_status;
//	uint8_t id;
};

#define eag_malloc malloc
#define eag_free free

rdc_userconn_db_t * 
rdc_userconn_db_create()
{
	rdc_userconn_db_t *new_db = NULL;
	int iret = EAG_ERR_UNKNOWN;

	new_db = (rdc_userconn_db_t *) eag_malloc(sizeof (rdc_userconn_db_t));
	if (NULL == new_db) {
		eag_log_err("rdc_userconn_db_create malloc error!\n");
		return NULL;
	}
	memset(new_db, 0, sizeof (rdc_userconn_db_t));
	INIT_LIST_HEAD(&(new_db->head));

	iret = eag_blkmem_create(&(new_db->userblkmem),
				 "rdc_userconn_db_blk", sizeof (struct rdc_userconn),
				 USERCOON_BLKMEM_SIZE, MAX_BLK_NUM);
	if (EAG_RETURN_OK != iret) {
		eag_log_err("rdc_userconn_db create blkmem failed!");
		eag_free(new_db);
		return NULL;
	}

	return new_db;
}

int
rdc_userconn_db_destroy(rdc_userconn_db_t *db)
{

	if (NULL == db) {
		eag_log_err("rdc_userconn_db_destroy error!");
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	if (NULL != db->userblkmem) {
		eag_blkmem_destroy(&(db->userblkmem));
	}
	eag_free(db);
	return 0;
}


int
rdc_userconn_db_clear(rdc_userconn_db_t *userdb)
{
	rdc_userconn_t *userconn=NULL;
	rdc_userconn_t *n=NULL;
	
	if (NULL == userdb) {
		eag_log_err("rdc_userconn_db_clear error!");
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	list_for_each_entry_safe(userconn, n, &(userdb->head), node){
		rdc_userconn_db_del (userdb, userconn);
		rdc_userconn_free (userdb, userconn);
	}
	return 0;
}


int
rdc_userconn_db_add(rdc_userconn_db_t *db,
						rdc_userconn_t *userconn)
{	
	char ip_str[32];

	if( NULL == db || NULL == userconn ){
		eag_log_err("rdc_userconn_db_add input param error!");
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	list_add_tail(&(userconn->node), &(db->head));
	ip2str( userconn->user_ip, ip_str, sizeof(ip_str));
	eag_log_info("rdc_userconn_db_add sessionid=%s ip=%s username=%s ", 
						userconn->sessionid, ip_str, userconn->username);
	return EAG_RETURN_OK;
}

int 
rdc_userconn_db_del(rdc_userconn_db_t *db,
						rdc_userconn_t *userconn)
{
	char ip_str[32];
	if( NULL == db || NULL == userconn ){
		eag_log_err("rdc_userconn_db_del input param error!");
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	
	ip2str( userconn->user_ip, ip_str, sizeof(ip_str));
	eag_log_info("rdc_userconn_db_del sessionid=%s ip=%s username=%s ", 
						userconn->sessionid, ip_str, userconn->username);
	list_del(&(userconn->node));
	return EAG_RETURN_OK;
}

rdc_userconn_t *
rdc_userconn_db_find_sessionid(rdc_userconn_db_t *db,
							char *sessionid )
{	
	char ip_str[32];
	rdc_userconn_t *userconn = NULL;
	
	list_for_each_entry(userconn, &(db->head), node){
		if (strcmp(sessionid,userconn->sessionid)==0) {
			ip2str( userconn->user_ip, ip_str, sizeof(ip_str));
			eag_log_debug("userconn","rdc_userconn_db_find_sessionid sessionid=%s"\
							"ip=%s username=%s ", sessionid, ip_str, userconn->username);
			return userconn;
		}
	}
	eag_log_debug("userconn","rdc_userconn_db_find_sessionid failed "\
					"sessionid=%s", sessionid );
	return NULL;
}

rdc_userconn_t *
rdc_userconn_db_find_username( rdc_userconn_db_t *db,
							char *username )
{	
	char ip_str[32];
	rdc_userconn_t *userconn = NULL;
	
	list_for_each_entry(userconn, &(db->head), node){
		if (strcmp(username,userconn->username)==0) {
			ip2str( userconn->user_ip, ip_str, sizeof(ip_str));
			eag_log_debug("userconn","rdc_userconn_db_find_username sessionid=%s"\
							"ip=%s username=%s ", userconn->sessionid, ip_str, username);
			return userconn;
		}
	}
	eag_log_debug("userconn","rdc_userconn_db_find_username failed "\
					"username=%s", username );
	return NULL;
}


rdc_userconn_t *
rdc_userconn_db_find_user(rdc_userconn_db_t *db,
							char *username, char *sessionid,
							uint32_t ip )
{	
	char ip_str[32];
	rdc_userconn_t *userconn = NULL;

	ip2str( ip, ip_str, sizeof(ip_str));

	list_for_each_entry(userconn, &(db->head), node){
		if( strlen(sessionid)>0 && (strcmp(sessionid,userconn->sessionid)!=0 ) ){
			continue;
		}
		if( strlen(username)>0 && strcmp(username,userconn->username)!=0 ){
			eag_log_err("rdc_userconn_db_find_user get sessionid but username not the same!");
			continue;
		}

		if( ip !=0 && userconn->user_ip != ip ){
			eag_log_err("rdc_userconn_db_find_user get sessionid and username but ip not the same!");			
			continue;
		}

		return userconn;
	}
	
	eag_log_debug("userconn","rdc_userconn_db_find_user failed "\
					"name=%s sessionid=%s ip=%s", username, sessionid, ip_str );
	return NULL;
}


rdc_userconn_t *
rdc_userconn_new(rdc_userconn_db_t * db, 
		char *username, char *sessionid, uint32_t userip )
{
	char ip_str[32];
	rdc_userconn_t *userconn = eag_blkmem_malloc_item(db->userblkmem);
	if (NULL == userconn) {
		eag_log_err("rdc_new_userconn return NULL");
		return NULL;
	}
	userconn->db = db;
	strncpy( userconn->username, username, sizeof(userconn->username)-1);
	strncpy( userconn->sessionid, sessionid, sizeof(userconn->sessionid)-1);
	userconn->user_ip = userip;
	
	ip2str( userip, ip_str, sizeof(ip_str));
	eag_log_debug("userconn", "rdc_userconn_new ret=%p name=%s sessionid=%s ip=%s",
					userconn, username, sessionid, ip_str );
	return userconn;
}

int
rdc_userconn_set_client( rdc_userconn_t *userconn, uint32_t client_ip,
							uint16_t client_coa_port )
{
	char ip_str[32];

	if( NULL == userconn || 0==client_ip || 0==client_coa_port ){
		eag_log_err("rdc_userconn_set_client input error userconn=%p ip=%x port=%u",
					userconn, client_ip, client_coa_port );
		return EAG_ERR_NULL_POINTER;
	}

	userconn->client_ip = client_ip;
	userconn->client_coa_port = client_coa_port;
	ip2str( client_ip, ip_str, sizeof(ip_str));
	
	eag_log_debug("userconn", "rdc_userconn_set_client set %s %s client--%s:%d",
					userconn->username, userconn->sessionid, ip_str, client_coa_port );
	return EAG_RETURN_OK;
}

int
rdc_userconn_get_client( rdc_userconn_t *userconn, uint32_t *client_ip,
							uint16_t *client_coa_port )
{
	char ip_str[32];
	if( NULL == userconn || NULL == client_ip || NULL == client_coa_port ){
		eag_log_err("rdc_userconn_get_client userconn=%p client_ip=%p "\
					"client_coa_port=%p", userconn, client_ip, client_coa_port );
		return EAG_ERR_NULL_POINTER;
	}

	*client_ip = userconn->client_ip;
	*client_coa_port = userconn->client_coa_port;
	ip2str( userconn->client_ip, ip_str, sizeof(ip_str));
	eag_log_debug("userconn", "rdc_userconn_get_client %s:%s client--%s:%d",
					userconn->username, userconn->sessionid, ip_str, userconn->client_coa_port );
	return EAG_RETURN_OK;
}

char *
rdc_userconn_get_username( rdc_userconn_t *userconn )
{
	return (NULL==userconn)?NULL:userconn->username;
}

char *
rdc_userconn_get_sessionid( rdc_userconn_t *userconn )
{
	return (NULL==userconn)?NULL:userconn->sessionid;
}


int
rdc_userconn_set_start_time( rdc_userconn_t *userconn, uint32_t start_time )
{
	if( NULL == userconn ){
		eag_log_err("rdc_userconn_set_start_time userconn is NULL");
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	userconn->start_time = start_time;
	eag_log_debug("userconn","rdc_userconn_set_start_time %s %s %ul",
					userconn->username, userconn->sessionid, start_time );
	return EAG_RETURN_OK;
}

int
rdc_userconn_set_last_time( rdc_userconn_t *userconn, uint32_t last_time )
{
	if( NULL == userconn ){
		eag_log_err("rdc_userconn_set_last_time userconn is NULL");
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	userconn->last_time = last_time;
	eag_log_debug("userconn","rdc_userconn_set_last_time %s %s %ul",
					userconn->username, userconn->sessionid, last_time );
	return EAG_RETURN_OK;
}

int
rdc_userconn_set_last_acct_status( rdc_userconn_t *userconn, int acct_status )
{
	if( NULL == userconn 
		|| ( acct_status != RADIUS_STATUS_TYPE_START 
			 && acct_status != RADIUS_STATUS_TYPE_STOP 
		     && acct_status != RADIUS_STATUS_TYPE_INTERIM_UPDATE ) )
	{
		eag_log_err("rdc_userconn_set_last_acct_status input err! userconn=%p status=%d",
						userconn, acct_status );
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	userconn->acct_status = acct_status;
	return EAG_RETURN_OK;
}


int
rdc_userconn_free(rdc_userconn_db_t * db, rdc_userconn_t *userconn)
{
	if (NULL == db || NULL == db->userblkmem || NULL == userconn) {
		eag_log_err("rdc_userconn_free error db=%p blkmem=%p userconn=%p",
						db, (db==NULL)?NULL:db->userblkmem, userconn);
		return EAG_ERR_INPUT_PARAM_ERR;
	}
	eag_log_debug("userconn", "rdc_userconn_free %p  %s:%s", userconn,
					userconn->username, userconn->sessionid );
	return eag_blkmem_free_item(db->userblkmem, userconn);
}

int
rdc_userconn_db_log_all( rdc_userconn_db_t *db )
{	
	char user_ip_str[32] = "";
	char client_ip_str[32] = "";
	rdc_userconn_t *userconn = NULL;
	int num = 0;

	if (NULL == db) {
		eag_log_err("rdc_userconn_db_log_all input error");
		return -1;
	}

	eag_log_info("-----rdc log all userconn begin-----");
	list_for_each_entry(userconn, &(db->head), node) {
		num++;
		eag_log_info("%-5d username=%s sessionid=%s user_ip=%s " 
			"client_ip=%s client_coa_port=%u start_time=%d last_time=%d",
			num, userconn->username, userconn->sessionid, 
			ip2str( userconn->user_ip, user_ip_str, sizeof(user_ip_str)),
			ip2str( userconn->client_ip, client_ip_str, sizeof(client_ip_str)),
			userconn->client_coa_port, userconn->start_time, userconn->last_time );
	}
	eag_log_info("-----rdc log all userconn end, num: %d-----", num);
	
	return 0;
}

int
rdc_userconn_db_check_timeout( rdc_userconn_db_t *userdb, 
			uint32_t timenow, uint32_t interval_timeout, uint32_t stop_timeout )
{
	char ip_str[32]="";
	rdc_userconn_t *userconn = NULL;
	rdc_userconn_t *next = NULL;
	
	if( NULL == userdb ){
		eag_log_err("rdc_userconn_db_check_timeout input userdb is NULL!");
		return EAG_ERR_INPUT_PARAM_ERR;
	}

	list_for_each_entry_safe(userconn, next, &(userdb->head), node){
		if( timenow < userconn->last_time ){
			eag_log_err("rdc check userconn time error last(%ul) > timenow(%ul)",
						userconn->last_time, timenow);
			userconn->last_time = timenow;
			continue;
		} 
#if 0		
		if( RADIUS_STATUS_TYPE_STOP == userconn->acct_status  
			&& timenow - userconn->last_time > stop_timeout )
		{
			eag_log_info("rdc del userconn get stop:%s %s %s last:%ul now:%ul", 
						userconn->username, userconn->sessionid, ip_str,
						userconn->last_time, timenow );		
			ip2str( userconn->user_ip, ip_str, sizeof(ip_str));
			rdc_userconn_db_del( userdb, userconn );
			rdc_userconn_free( userdb, userconn );
		}else 
#endif		
		if( timenow - userconn->last_time > interval_timeout ){
			eag_log_err("rdc del userconn not get stop:%s %s %s last:%ul now:%ul", 
						userconn->username, userconn->sessionid, ip_str,
						userconn->last_time, timenow );		
			rdc_userconn_db_del( userdb, userconn );
			rdc_userconn_free( userdb, userconn );
		}
	}
	return EAG_RETURN_OK;
}

#ifdef rdc_userconn_test

#include "eag_errcode.c"
#include "eag_log.c"
#include "eag_mem.c"
#include "eag_blkmem.c"
#include "eag_util.c"

int main(void)
{
	printf("start\n");
	eag_log_init("userconn");
	setenv("MALLOC_TRACE", "log123", 1);
	mtrace();
	
//	int ret = -1;
	rdc_userconn_db_t *db = NULL;
	rdc_userconn_t *findconn;

	malloc(123);

	db = rdc_userconn_db_create();
	if (NULL == db){
		printf("rdc_userconn_db_create err\n");
	};
	
	struct in_addr addr1;
	struct in_addr addr1_ser;
	char *userip1 = "11.22.33.44";
	char *serip1 = "11.12.33.55";
	inet_aton(userip1, &addr1 );
	inet_aton(serip1, &addr1_ser );

	rdc_userconn_t *userconn1 = 
			rdc_userconn_new(db, "aaaa","1234561234566",ntohl(addr1.s_addr));
	rdc_userconn_set_client( userconn1, ntohl(addr1_ser.s_addr), 3799);
	rdc_userconn_db_add(db, userconn1);


	struct in_addr addr2;
	struct in_addr addr2_ser;
	char *userip2 = "123.222.33.44";
	char *serip2 = "123.132.33.55";
	inet_aton(userip2, &addr2 );
	inet_aton(serip2, &addr2_ser );

	rdc_userconn_t *userconn2 = 
			rdc_userconn_new(db, "bbbb","3245434214",ntohl(addr2.s_addr));
	rdc_userconn_set_client( userconn2, ntohl(addr2_ser.s_addr), 3799);
	rdc_userconn_db_add(db, userconn2);

	
	findconn = rdc_userconn_db_find_sessionid(db,userconn1->sessionid);
	if( findconn != userconn1 ){
		printf("find userconn1 failed!");
	}

	findconn = rdc_userconn_db_find_user(db,userconn2->username,
						userconn2->sessionid,userconn2->user_ip );
	if( findconn != userconn2 ){
		printf("find userconn2 failed!");
	}

	rdc_userconn_db_log_all(db);

	rdc_userconn_db_del(db, userconn2);
	findconn = rdc_userconn_db_find_user(db,userconn2->username,
						userconn2->sessionid,userconn2->user_ip );
	if( findconn != NULL ){
		printf("userconn2 del failed!");
	}
	rdc_userconn_free(db,userconn2);


	rdc_userconn_db_del(db, userconn1);
	findconn = rdc_userconn_db_find_user(db,userconn1->username,
						userconn1->sessionid,userconn1->user_ip );
	if( findconn != NULL ){
		printf("userconn1 del failed!");
	}

	
	rdc_userconn_free(db,userconn1);
	

	if (EAG_RETURN_OK != rdc_userconn_db_destroy(db)){
		printf("rdc_userconn_db_destroy err\n");
	}
	
	muntrace();
	eag_log_uninit();
	printf("end\n");
	return 0;
}

#endif

