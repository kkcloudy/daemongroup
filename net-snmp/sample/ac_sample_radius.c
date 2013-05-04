#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <stdint.h>

#include   <netinet/in.h> 
#include   <netdb.h> 
#include   <sys/types.h> 
#include   <sys/socket.h>
#include   <asm/ioctls.h>
#include   <sys/ioctl.h> 
#include   <sys/select.h> 
#include   <sys/errno.h> 

#include "nm_list.h"
#include "ws_dbus_def.h"
#include "ac_sample_dbus.h"
#include "ac_sample.h"
#include "ac_sample_err.h"
#include "ac_sample_def.h"

#include "ws_snmpd_engine.h"
#include "ws_snmpd_manual.h"
#include "ac_manage_def.h"
#include "ac_manage_interface.h"

#include "acsample_util.h"
#include "ac_sample_radius.h"

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpathInternals.h>

#include "hmd/hmdpub.h"
#include "ws_local_hansi.h"

#include "eag_errcode.h"
#include "eag_conf.h"
#include "eag_interface.h"
#include "ac_manage_sample_interface.h"


#define RADIUS_SERVER_AUTH_STATUS	"/usr/bin/test_radius_auth_server.sh"
#define RADIUS_SERVER_ACCT_STATUS	"/usr/bin/test_radius_acct_server.sh"
#define RADIUS_STATUE_AUTH_FILE      "/var/run/radius_auth_server_status.log"
#define RADIUS_STATUE_ACCT_FILE      "/var/run/radius_acct_server_status.log"

#define STATUS_BUF_MIN_LEN		11

#define IP_STR_LEN 	16

typedef struct 
{
    unsigned int local_id;
    unsigned int instance_id;
    
	char ip[IP_STR_LEN];
	short port;
	
	char backup_ip[IP_STR_LEN];
	short backup_port;

	char error_ip[IP_STR_LEN];
	short error_port;

}RADIUS_SERVER_STATUS_INFO, *PRADIUS_SERVER_STATUS_INFO;

typedef struct
{
	struct radius_t radius;
	struct radius_packet_t packet;
	int error;		// 1 is no error
	
}RADIUS_PRIVATE_DATA, *PRADIUS_PRIVATE_DATA;


static char *
inet_ntoa_ex(unsigned long ip, char *ipstr, unsigned int buffsize)
{
	snprintf(ipstr, buffsize, "%lu.%lu.%lu.%lu", ip >> 24,
		 (ip & 0xff0000) >> 16, (ip & 0xff00) >> 8, (ip & 0xff));

	return ipstr;
}

int do_radius_check( int threshold, int sample )
{
	if(SERVER_REACH_THRESHOLD == sample)
	{
		syslog( LOG_INFO, "do_radius_check OVER_THRESHOLD_FLAG");
		return OVER_THRESHOLD_FLAG;
	}
	else
	{
		syslog( LOG_INFO, "do_radius_check NOT_OVER_THRESHOLD");
		return NOT_OVER_THRESHOLD;
	}
}

int init_user_data_auth_radius( PRADIUS_PRIVATE_DATA us_radius )
{
	if ( NULL==us_radius )
	{
		return AS_RTN_NULL_POINTER;
	}
	
	memset ( us_radius, 0, sizeof(RADIUS_PRIVATE_DATA));
	us_radius->error=0;
	
	int fd=-1;
	int ret=0;

	if((fd = socket(PF_INET,SOCK_DGRAM,0)) < 0 )
	{
		us_radius->error=SOCKET_CREATE_ERR;
		syslog(LOG_INFO,"portal_sample_user_data_init: socket is error errno %d\n;", errno);
		return AS_RTN_ERR;
	}

	unsigned long ul = 1;
	if(ioctl(fd, FIONBIO, &ul) == -1)
    {
        close(fd);
        return AS_RTN_ERR;
    }
	
	FILE *urandom_fp=NULL;
	struct radius_t *radius=&(us_radius->radius);
	struct radius_packet_t *radius_pack=&(us_radius->packet);

	radius->fd=fd;

	if ((radius->urandom_fp = fopen(URANDOM_FILE_PATH, "r")) == NULL) {
		us_radius->error=FILE_OPEN_ERR;
		syslog(LOG_INFO,"do_sample_radius_authfopen(%s, r) failed errno %d ", URANDOM_FILE_PATH, errno);
		close(radius->fd);
		return AS_RTN_ERR;
	}

	char *user_name="00000000";
	char *password="11111111";
	
	if ( 0!=(ret=radius_default_pack(radius, radius_pack, RADIUS_CODE_ACCESS_REQUEST)))
	{
		us_radius->error=RADIUS_DEFAULT_PACK_ERR;
	}
	
	if (0!=(ret=radius_addattr(radius, radius_pack, RADIUS_ATTR_USER_NAME, 0, 0, 0,
		 	(uint8_t*) user_name, strlen(user_name))))
	{
		us_radius->error=RADIUS_ADDATTR_ERR;
		return AS_RTN_ERR;
	}
	syslog(LOG_DEBUG,"...add attr USER_NAME(%d) value %s", RADIUS_ATTR_USER_NAME, user_name);

    	if (0!=(ret=radius_addattr(radius, radius_pack, RADIUS_ATTR_USER_PASSWORD, 0, 0, 0,
					   (uint8_t*)password, REDIR_MD5LEN)))
    	{
		us_radius->error=RADIUS_ADDATTR_ERR;
		return AS_RTN_ERR;
    	}
	syslog(LOG_DEBUG,"...add attr USER_PASSWORD(%d) value %s len %d", RADIUS_ATTR_USER_PASSWORD, password, REDIR_MD5LEN);
	

	us_radius->error=RADIUS_USER_DATA_OK;
	return AS_RTN_OK;
	
}


int init_user_data_acct_radius( PRADIUS_PRIVATE_DATA us_radius )
{
	if ( NULL==us_radius )
	{
		return AS_RTN_NULL_POINTER;
	}
	
	memset ( us_radius, 0, sizeof(RADIUS_PRIVATE_DATA));
	us_radius->error=0;
	
	int fd=-1;
	int ret=0;

	if((fd = socket(PF_INET,SOCK_DGRAM,0)) < 0 )
	{
		us_radius->error=SOCKET_CREATE_ERR;
		syslog(LOG_INFO,"portal_sample_user_data_init: socket is error errno %d\n;", errno);
		return AS_RTN_ERR;
	}

	unsigned long ul = 1;
	if(ioctl(fd, FIONBIO, &ul) == -1)
    {
        close(fd);
        return AS_RTN_ERR;
    }
	
	FILE *urandom_fp=NULL;
	struct radius_t *radius=&(us_radius->radius);
	struct radius_packet_t *radius_pack=&(us_radius->packet);

	radius->fd=fd;

	if ((radius->urandom_fp = fopen(URANDOM_FILE_PATH, "r")) == NULL) {
		us_radius->error=FILE_OPEN_ERR;
		syslog(LOG_INFO,"do_sample_radius_authfopen(%s, r) failed errno %d ", URANDOM_FILE_PATH, errno);
		close(radius->fd);
		return AS_RTN_ERR;
	}

	char *user_name="00000000";
	char *password="11111111";
	
	if ( 0!=(ret=radius_default_pack(radius, radius_pack, RADIUS_CODE_ACCOUNTING_REQUEST)))
	{
		us_radius->error=RADIUS_DEFAULT_PACK_ERR;
	}
	
	if (0!=(ret=radius_addattr(radius, radius_pack, RADIUS_ATTR_USER_NAME, 0, 0, 0,
		 	(uint8_t*) user_name, strlen(user_name))))
	{
		us_radius->error=RADIUS_ADDATTR_ERR;
		return AS_RTN_ERR;
	}
	syslog(LOG_DEBUG,"...add attr USER_NAME(%d) value %s", RADIUS_ATTR_USER_NAME, user_name);

    	if (0!=(ret=radius_addattr(radius, radius_pack, RADIUS_ATTR_USER_PASSWORD, 0, 0, 0,
					   (uint8_t*)password, REDIR_MD5LEN)))
    	{
		us_radius->error=RADIUS_ADDATTR_ERR;
		return AS_RTN_ERR;
    	}
	syslog(LOG_DEBUG,"...add attr USER_PASSWORD(%d) value %s len %d", RADIUS_ATTR_USER_PASSWORD, password, REDIR_MD5LEN);
	

	us_radius->error=RADIUS_USER_DATA_OK;
	return AS_RTN_OK;
	
}


void radius_sample_user_data_free ( void *user_data)
{
	if ( NULL==user_data )
		return ;
	
	PRADIUS_PRIVATE_DATA subInfo=(PRADIUS_PRIVATE_DATA) user_data;

	SAMPLE_FREE( subInfo );

	return;
}

void radius_subsample_user_data_free ( void *user_data)
{
	if ( NULL==user_data )
		return ;
	
	PRADIUS_SERVER_STATUS_INFO subInfo=(PRADIUS_SERVER_STATUS_INFO) user_data;

	SAMPLE_FREE( subInfo );

	return;
}

static load_radius_count_config_by_instance ( multi_sample_t *multi_sample, struct list_head *head, struct instance_radius_config *instance_config)
{   
    syslog(LOG_DEBUG, "enter load_radius_count_config_by_instance\n");

    if(NULL == multi_sample || NULL == head || NULL == instance_config) {
        syslog(LOG_WARNING, "load_radius_count_config_by_instance: input para error!\n");
        return AS_RTN_PARAM_ERR;
    }
    
    syslog(LOG_INFO, "load %s instance %d-%d radius count config\n", 
                     instance_config->local_id ? "Local-hansi" : "Remote-hansi", instance_config->slot_id, instance_config->instance_id);

    int i = 0;
	for(i = 0; i < instance_config->radiusconf.current_num; i++) {
        
		char count_ip[32] = { 0 };
		char back_count_ip[32] = { 0 };
		
        PRADIUS_SERVER_STATUS_INFO radius_count_data = (PRADIUS_SERVER_STATUS_INFO)SAMPLE_MALLOC(sizeof(RADIUS_SERVER_STATUS_INFO));
        if(NULL == radius_count_data) {
            syslog(LOG_ERR,"load_radius_count_config_by_instance: malloc error memory full!\n");
            return AS_RTN_MALLOC_ERR;
        }
        memset(radius_count_data, 0 , sizeof(*radius_count_data));
        
		inet_ntoa_ex(instance_config->radiusconf.radius_srv[i].acct_ip, count_ip, sizeof(count_ip));
		
        strncpy(radius_count_data->ip, count_ip, IP_STR_LEN - 1);                    
        radius_count_data->port = instance_config->radiusconf.radius_srv[i].acct_port;

        if(instance_config->radiusconf.radius_srv[i].backup_acct_ip) { 
            inet_ntoa_ex(instance_config->radiusconf.radius_srv[i].backup_acct_ip, back_count_ip, sizeof(back_count_ip));
            strncpy(radius_count_data->backup_ip, back_count_ip, IP_STR_LEN - 1);
            radius_count_data->backup_port = instance_config->radiusconf.radius_srv[i].backup_acct_port;
        }
        
        radius_count_data->local_id = instance_config->local_id;
        radius_count_data->instance_id = instance_config->instance_id;
        
        create_config_data_2(multi_sample, head, instance_config->radiusconf.radius_srv[i].acct_ip, radius_count_data);
        syslog(LOG_DEBUG,"load_radius_auth_config: match_word (%u) ip (%s) port (%d) backup ip (%s) port (%d)",
                                instance_config->radiusconf.radius_srv[i].acct_ip, radius_count_data->ip, radius_count_data->port, 
                                radius_count_data->backup_ip, radius_count_data->backup_port ); //test
	
	}
	
    syslog(LOG_DEBUG, "exit load_radius_count_config_by_instance\n");
	return AS_RTN_OK;
}
static int load_radius_count_config(multi_sample_t *multi_sample, struct list_head *head )
{
	//syslog(LOG_INFO,"load_radius_count_config: entry.\n"); //test
		
	if(NULL == multi_sample || NULL == head) {
		return AS_RTN_NULL_POINTER;
	}

    DBusConnection *active_connection = ac_sample_dbus_get_active_connection();
    if(NULL == active_connection) {
        return AS_RTN_SAMPLE_DBUS_CONNECTION_ERR;
    }

    struct instance_radius_config *config_array = NULL;
    unsigned int config_num = 0;
    int ret = AC_MANAGE_SUCCESS;
    
    syslog(LOG_INFO, "load_radius_count_config: before ac_manage_show_radius_config\n");
    ret = ac_manage_show_radius_config(active_connection, &config_array, &config_num);
    syslog(LOG_INFO, "load_radius_count_config: after ac_manage_show_radius_config, ret = %d\n", ret);
	    
    if(AC_MANAGE_SUCCESS == ret && config_array) {
        int i = 0;
        for(i = 0; i < config_num; i++) {
            if(load_radius_count_config_by_instance(multi_sample, head, &(config_array[i]))) {
                syslog(LOG_INFO, "load_radius_count_config_by_instance error\n");
                continue;
            }
        }   
        
        free(config_array);
    }
    else {
        return AS_RTN_ERR;
    }
        
	return AS_RTN_OK;
}


static int 
load_radius_auth_config_by_instance ( multi_sample_t *multi_sample, struct list_head *head, struct instance_radius_config *instance_config)
{
    syslog(LOG_DEBUG, "enter load_radius_auth_config_by_instance\n");

    if(NULL == multi_sample || NULL == head || NULL == instance_config) {
        syslog(LOG_WARNING, "load_radius_auth_config_by_instance: input para error!\n");
        return AS_RTN_PARAM_ERR;
    }
    
    syslog(LOG_INFO, "load %s instance %d-%d radius auth config\n", 
                     instance_config->local_id ? "Local-hansi" : "Remote-hansi", instance_config->slot_id, instance_config->instance_id);
	
    int i = 0;
	for(i = 0; i < instance_config->radiusconf.current_num; i++) {
        
		char auth_ip[32] = { 0 };
		char back_auth_ip[32] = { 0 };
		
        PRADIUS_SERVER_STATUS_INFO radius_auth_data = (PRADIUS_SERVER_STATUS_INFO)SAMPLE_MALLOC(sizeof(RADIUS_SERVER_STATUS_INFO));
        if(NULL == radius_auth_data) {
            syslog(LOG_ERR,"load_radius_auth_config: malloc error memory full!\n");
            return AS_RTN_MALLOC_ERR;
        }

        memset(radius_auth_data, 0 , sizeof(*radius_auth_data));
        
		inet_ntoa_ex(instance_config->radiusconf.radius_srv[i].auth_ip, auth_ip, sizeof(auth_ip));
        strncpy(radius_auth_data->ip, auth_ip, IP_STR_LEN - 1);                    
        radius_auth_data->port = instance_config->radiusconf.radius_srv[i].auth_port;

        if(instance_config->radiusconf.radius_srv[i].backup_auth_ip) {
            inet_ntoa_ex(instance_config->radiusconf.radius_srv[i].backup_auth_ip, back_auth_ip, sizeof(back_auth_ip));
            strncpy(radius_auth_data->backup_ip, back_auth_ip, IP_STR_LEN - 1);                    
            radius_auth_data->backup_port = instance_config->radiusconf.radius_srv[i].backup_auth_port;
        }
        
        radius_auth_data->local_id = instance_config->local_id;
        radius_auth_data->instance_id = instance_config->instance_id;
        
        create_config_data_2(multi_sample, head, instance_config->radiusconf.radius_srv[i].auth_ip, radius_auth_data);
        syslog(LOG_DEBUG,"load_radius_auth_config: match_word (%u) ip (%s) port (%d) backup ip (%s) port (%d)",
                                instance_config->radiusconf.radius_srv[i].auth_ip, radius_auth_data->ip, radius_auth_data->port, 
                                radius_auth_data->backup_ip, radius_auth_data->backup_port ); //test
	
	}
	
    syslog(LOG_DEBUG, "exit load_radius_auth_config_by_instance\n");
	return AS_RTN_OK;
}

static int load_radius_auth_config ( multi_sample_t *multi_sample, struct list_head *head )
{
	//syslog(LOG_INFO,"load_radius_count_config: entry.\n"); //test
		
	if(NULL == multi_sample || NULL == head) {
		return AS_RTN_NULL_POINTER;
	}

    DBusConnection *active_connection = ac_sample_dbus_get_active_connection();
    if(NULL == active_connection) {
        return AS_RTN_SAMPLE_DBUS_CONNECTION_ERR;
    }
    
    struct instance_radius_config *config_array = NULL;
    unsigned int config_num = 0;
    int ret = AC_MANAGE_SUCCESS;
    
    syslog(LOG_INFO, "load_radius_auth_config: before ac_manage_show_radius_config\n");
    ret = ac_manage_show_radius_config(active_connection, &config_array, &config_num);
    syslog(LOG_INFO, "load_radius_auth_config: after ac_manage_show_radius_config, ret = %d\n", ret);
	    
    if(AC_MANAGE_SUCCESS == ret && config_array) {
        int i = 0;
        for(i = 0; i < config_num; i++) {
            if(load_radius_auth_config_by_instance(multi_sample, head, &(config_array[i]))) {
                syslog(LOG_INFO, "load_radius_count_config_by_instance error\n");
                continue;
            }
        }   
        
        free(config_array);
    }
    else {
        return AS_RTN_ERR;
    }

	return AS_RTN_OK;
}


static int do_sample_radius_auth( struct ac_sample_t *me)
{
	syslog(LOG_INFO,"do_sample_radius_auth: entry.");  //test
	
	int ret=-1;
	PRADIUS_PRIVATE_DATA us_radius=get_sample_user_data( me );
	if ( NULL==us_radius )
	{
		syslog(LOG_INFO,"do_sample_radius_auth: PRADIUS_PRIVATE_DATA is NULL");
		return RADIUS_REACHABLE_VALUE;
	}else
	{
		if ( RADIUS_USER_DATA_OK!=us_radius->error )
		if ( 0!=init_user_data_auth_radius( us_radius ) )
		{
			syslog(LOG_INFO,"do_sample_radius_auth: init_user_data_radius error errno %d",us_radius->error);
			return RADIUS_REACHABLE_VALUE;
		}
	}
	
	struct radius_t *radius=&(us_radius->radius);
	struct radius_packet_t *packet=&(us_radius->packet);

	PRADIUS_SERVER_STATUS_INFO radius_auth_data=get_subsample_user_data( me );
	
	struct sockaddr_in ser_radius;
	bzero( &(ser_radius), sizeof(ser_radius) );
	ser_radius.sin_family = PF_INET;
	ser_radius.sin_port = htons(radius_auth_data->port);
	ser_radius.sin_addr.s_addr = inet_addr(radius_auth_data->ip);

	int len=sizeof(struct radius_packet_t);

	syslog(LOG_INFO, "do_sample_radius_auth master radius: sendto %s %d.",radius_auth_data->ip,radius_auth_data->port);
	if (sendto(radius->fd, packet, len, 0,
				(struct sockaddr *) &ser_radius, sizeof(struct sockaddr_in)) < 0) {
		syslog(LOG_INFO, "do_sample_radius_auth master radius: sendto failed!");
		return RADIUS_REACHABLE_VALUE;
	}

	if ( 0!=radius_auth_data->backup_ip[0] )
	{	
		syslog(LOG_INFO, "do_sample_radius_auth backup radius: sendto %s %d.",radius_auth_data->backup_ip,radius_auth_data->backup_port);
		struct sockaddr_in backup_radius;
		bzero( &(backup_radius), sizeof(backup_radius) );
		backup_radius.sin_family = PF_INET;
		backup_radius.sin_port = htons(radius_auth_data->backup_port);
		backup_radius.sin_addr.s_addr = inet_addr(radius_auth_data->backup_ip);

		if (sendto(radius->fd, packet, len, 0,
					(struct sockaddr *) &backup_radius, sizeof(struct sockaddr_in)) < 0) {
			syslog(LOG_INFO, "do_sample_radius_auth backup radius: sendto failed!");
			return RADIUS_REACHABLE_VALUE;
		}
	}
	
	struct sockaddr_in addr;
	fd_set set;
	int fromlen=0, status=-1;
	struct timeval timeout=gTimeout; 
	FD_ZERO(&set);
	FD_SET(radius->fd, &set);

	if( select(radius->fd+1, &set, NULL, NULL, &timeout) > 0)
	{
		struct radius_packet_t recv_pack;
		
		if ((status = recvfrom(radius->fd, &recv_pack, sizeof(recv_pack), 0, 
						(struct sockaddr *) &addr, &fromlen)) <= 0) 
		{
			syslog(LOG_INFO, "do_sample_radius_auth: recvfrom failed");
			return RADIUS_UNREACHABLE_VALUE;
		}
		
		if (status < RADIUS_HDRSIZE) 
		{
			syslog(LOG_INFO,"do_sample_radius_auth Received radius packet which is too short: %d < %d!",
					status, RADIUS_HDRSIZE);
			return RADIUS_UNREACHABLE_VALUE;
		}
		
		if (ntohs(recv_pack.length) != (uint16_t)status)
		{
			syslog(LOG_INFO,"do_sample_radius_auth Received radius packet with wrong length field %d != %d!",
					ntohs(recv_pack.length), status);
			return RADIUS_UNREACHABLE_VALUE;
		}
		
		/* TODO: Check consistency of attributes vs packet length */
		
		//switch (recv_pack.code) {
		//	}

		ret=RADIUS_REACHABLE_VALUE;
		syslog(LOG_DEBUG, "do_sample_radius_auth: reachable ret %d.", ret);
		
	}else
	{
		syslog(LOG_INFO,"do_sample_radius_auth: timeout.");
		ret=RADIUS_UNREACHABLE_VALUE;
	}

	syslog(LOG_INFO,"do_sample_radius_auth: exit.");  //test
	return ret;   
}

static int ac_sample_radius_auth_send_signal( ac_sample_t *this, int type )
{

	PRADIUS_SERVER_STATUS_INFO pInfo = ( PRADIUS_SERVER_STATUS_INFO )get_subsample_user_data(this);
	if ( NULL==pInfo )
	{
		return AS_RTN_PARAM_ERR;
	}

	char *str_ip = pInfo->ip;
	short port=pInfo->port;
	syslog(LOG_DEBUG, "ac_sample_radius_auth_send_signal, type=%d, ip=%s, port=%d", 
			type, str_ip, port);
	
	return ac_sample_dbus_send_active_signal(AC_SAMPLE_OVER_THRESHOLD_SIGNAL_RADIUS_AUTH, 
    										 DBUS_TYPE_UINT32, &type,
    										 DBUS_TYPE_STRING, &str_ip,
    										 DBUS_TYPE_UINT16, &port,
                                             DBUS_TYPE_UINT32, &pInfo->local_id,
                                             DBUS_TYPE_UINT32, &pInfo->instance_id,
    										 DBUS_TYPE_INVALID );
}

ac_sample_t *create_ac_sample_radius_auth(  
                                unsigned int sample_interval, 
                                unsigned int statistics_time )
{
	ac_sample_t *pret = NULL;
	
	acsample_conf config;
	int ret=-1;

	memset(&config, 0, sizeof(config));
	set_default_config( &config );
	if ((ret=acsample_read_conf( &config, SAMPLE_NAME_RADIUS_AUTH ) ))
	{
		syslog(LOG_INFO,"%s acsample_read_conf:error return %d load default config", SAMPLE_NAME_RADIUS_AUTH, ret );
	}

    pret = create_ac_sample(SAMPLE_NAME_RADIUS_AUTH, config.sample_interval, config.statistics_time, config.resend_interval );

	if( NULL != pret )
	{
		//system( RADIUS_SERVER_AUTH_STATUS );
		//static RADIUS_SERVER_STATUS_INFO server_status = {.ip = "", .port = 0 };
		static RADIUS_PRIVATE_DATA radius;
		init_user_data_auth_radius(&radius);
		
		set_multisample_conf( pret, load_radius_auth_config, config.reload_config_interval, radius_subsample_user_data_free, NULL);
		set_do_sample_cb( pret, do_sample_radius_auth );
		set_sample_state( pret, config.sample_on );
		set_sample_threshold( pret, config.threshold );
		set_over_threshold_type( pret, OVER_THRESHOLD_CHECK_TYPE_NORMAL);
		set_clear_type(pret, OVER_THRESHOLD_CHECK_TYPE_IMMEDIATELY);
		set_sample_user_data(pret, &radius, NULL);
		set_do_check_cb(pret, NULL);
		set_over_threshold_cb( pret, ac_sample_radius_auth_send_signal );
	}

	syslog( LOG_DEBUG, "create_ac_sample_radius_auth  %p", pret );
	
	syslog(LOG_INFO,"config: sample_on=%d, statistics_time=%d, sample_interval=%d, resend_interval=%d, reload_config_interval=%d, over_threshold_check_type=%d, clear_check_type=%d, threshold=%d",
							config.sample_on, config.statistics_time, config.sample_interval, config.resend_interval,\
							config.reload_config_interval, config.over_threshold_check_type, config.clear_check_type, config.threshold );
	return pret;
}

static int do_sample_radius_count( struct ac_sample_t *me )
{
	syslog(LOG_INFO,"do_sample_radius_count: entry.");  //test
	
	int ret=-1;
	PRADIUS_PRIVATE_DATA us_radius=get_sample_user_data( me );
	if ( NULL==us_radius )
	{
		syslog(LOG_INFO,"do_sample_radius_count: PRADIUS_PRIVATE_DATA is NULL");
		return RADIUS_REACHABLE_VALUE;
	}else
	{
		if ( RADIUS_USER_DATA_OK!=us_radius->error )
		if ( 0!=init_user_data_acct_radius( us_radius ) )
		{
			syslog(LOG_INFO,"do_sample_radius_count: init_user_data_radius error errno %d",us_radius->error);
			return RADIUS_REACHABLE_VALUE;
		}
	}
	
	struct radius_t *radius=&(us_radius->radius);
	struct radius_packet_t *packet=&(us_radius->packet);

	PRADIUS_SERVER_STATUS_INFO radius_auth_data=get_subsample_user_data( me );
	
	struct sockaddr_in ser_radius;
	bzero( &(ser_radius), sizeof(ser_radius) );
	ser_radius.sin_family = PF_INET;
	ser_radius.sin_port = htons(radius_auth_data->port);
	ser_radius.sin_addr.s_addr = inet_addr(radius_auth_data->ip);

	int len=sizeof(struct radius_packet_t);

	syslog(LOG_INFO, "do_sample_radius_count master radius: sendto %s %d.",radius_auth_data->ip,radius_auth_data->port);
	if (sendto(radius->fd, packet, len, 0,
				(struct sockaddr *) &ser_radius, sizeof(struct sockaddr_in)) < 0) {
		syslog(LOG_INFO, "do_sample_radius_count master radius: sendto() failed!");
		return RADIUS_REACHABLE_VALUE;
	}

	if ( 0!=radius_auth_data->backup_ip[0] )
	{	
		syslog(LOG_INFO, "do_sample_radius_count backup radius: sendto %s %d.",radius_auth_data->backup_ip,radius_auth_data->backup_port);
		struct sockaddr_in backup_radius;
		bzero( &(backup_radius), sizeof(backup_radius) );
		backup_radius.sin_family = PF_INET;
		backup_radius.sin_port = htons(radius_auth_data->backup_port);
		backup_radius.sin_addr.s_addr = inet_addr(radius_auth_data->backup_ip);

		if (sendto(radius->fd, packet, len, 0,
					(struct sockaddr *) &backup_radius, sizeof(struct sockaddr_in)) < 0) {
			syslog(LOG_INFO, "do_sample_radius_count backup radius: sendto() failed!");
			return RADIUS_REACHABLE_VALUE;
		}
	}
	
	struct sockaddr_in addr;
	fd_set set;
	int fromlen=0, status=-1;
	struct timeval timeout=gTimeout; 
	FD_ZERO(&set);
	FD_SET(radius->fd, &set);

	if( select(radius->fd+1, &set, NULL, NULL, &timeout) > 0)
	{
		struct radius_packet_t recv_pack;
		
		if ((status = recvfrom(radius->fd, &recv_pack, sizeof(recv_pack), 0, 
						(struct sockaddr *) &addr, &fromlen)) <= 0) 
		{
			syslog(LOG_INFO, "do_sample_radius_count: recvfrom() failed");
			return RADIUS_UNREACHABLE_VALUE;
		}
		
		if (status < RADIUS_HDRSIZE) 
		{
			syslog(LOG_INFO,"do_sample_radius_count Received radius packet which is too short: %d < %d!",
					status, RADIUS_HDRSIZE);
			return RADIUS_UNREACHABLE_VALUE;
		}
		
		if (ntohs(recv_pack.length) != (uint16_t)status)
		{
			syslog(LOG_INFO,"do_sample_radius_count Received radius packet with wrong length field %d != %d!",
					ntohs(recv_pack.length), status);
			return RADIUS_UNREACHABLE_VALUE;
		}
		
		/* TODO: Check consistency of attributes vs packet length */
		
		//switch (recv_pack.code) {
		//	}

		ret=RADIUS_REACHABLE_VALUE;
		syslog(LOG_DEBUG, "do_sample_radius_count: reachable ret %d.", ret);
	}else
	{
		syslog(LOG_INFO,"do_sample_radius_count: timeout.");
		ret=RADIUS_UNREACHABLE_VALUE;
	}
		
	syslog(LOG_INFO,"do_sample_radius_count: exit.");  //test
	return ret;
}

static int ac_sample_radius_acct_send_signal( ac_sample_t *this, int type )
{
	PRADIUS_SERVER_STATUS_INFO pInfo = ( PRADIUS_SERVER_STATUS_INFO )get_subsample_user_data(this);
	if ( NULL==pInfo )
	{
		return AS_RTN_PARAM_ERR;
	}

	char *str_ip = pInfo->ip;
	short port=pInfo->port;
	syslog(LOG_DEBUG, "ac_sample_radius_acct_send_signal, type=%d, ip=%s, port=%d", 
			type, str_ip, port);
	
	return ac_sample_dbus_send_active_signal(AC_SAMPLE_OVER_THRESHOLD_SIGNAL_RADIUS_ACC, 
    										 DBUS_TYPE_UINT32, &type,
    										 DBUS_TYPE_STRING, &str_ip,
    										 DBUS_TYPE_UINT16, &port,
                                             DBUS_TYPE_UINT32, &pInfo->local_id,
                                             DBUS_TYPE_UINT32, &pInfo->instance_id,
    										 DBUS_TYPE_INVALID );
}

ac_sample_t *create_ac_sample_radius_count(  
                                unsigned int sample_interval, 
                                unsigned int statistics_time )
{
	ac_sample_t *pret = NULL;	
	
	acsample_conf config;
	int ret=-1;

	memset(&config, 0, sizeof(config));
	set_default_config( &config );
	if ((ret=acsample_read_conf( &config, SAMPLE_NAME_RADIUS_COUNT ) ))
	{
		syslog(LOG_INFO,"%s acsample_read_conf:error return %d load default config", SAMPLE_NAME_RADIUS_COUNT, ret );
	}

    pret = create_ac_sample(SAMPLE_NAME_RADIUS_COUNT, config.sample_interval, config.statistics_time , config.resend_interval );

	if( NULL != pret )
	{
		//system( RADIUS_SERVER_AUTH_STATUS );
		//static RADIUS_SERVER_STATUS_INFO server_status = {.ip = "", .port = 0 };
		static RADIUS_PRIVATE_DATA radius;
		init_user_data_acct_radius(&radius);
		
		set_multisample_conf( pret, load_radius_count_config, config.reload_config_interval, radius_subsample_user_data_free, NULL);
		set_do_sample_cb( pret, do_sample_radius_count );
		set_sample_state( pret, config.sample_on );
		set_sample_threshold( pret, config.threshold );
		set_over_threshold_type( pret, OVER_THRESHOLD_CHECK_TYPE_NORMAL);
		set_clear_type(pret, OVER_THRESHOLD_CHECK_TYPE_IMMEDIATELY);
		set_sample_user_data(pret, &radius, NULL);
		set_do_check_cb(pret, NULL);
		set_over_threshold_cb( pret, ac_sample_radius_acct_send_signal );
	}

	syslog( LOG_DEBUG, "create_ac_sample_radius_count  %p", pret );
	
	syslog(LOG_INFO,"config: sample_on=%d, statistics_time=%d, sample_interval=%d, resend_interval=%d, reload_config_interval=%d, over_threshold_check_type=%d, clear_check_type=%d, threshold=%d",
							config.sample_on, config.statistics_time, config.sample_interval, config.resend_interval,\
							config.reload_config_interval, config.over_threshold_check_type, config.clear_check_type, config.threshold );
	return pret;
}

#if 0
int do_sample_radius(struct ac_sample_t *me, char *file_name)
{
	if (NULL==file_name || NULL==*file_name)
	{
		return SERVER_NOT_REACH_THRESHOLD;
	}
	
	FILE * fp = NULL;
	int cur_state = SERVER_NOT_REACH_THRESHOLD;
	int state = 0;	

	if (NULL != (fp=fopen(file_name, "r")) )
	{		
		char sstate[2]={0};
		char ip[IP_STR_LEN] = {0};
		int port	= 0;
		char sport[5] = {0};

		char buf[128] = {0};
		if (NULL!=fgets(buf, sizeof(buf), fp) )
		{
			syslog(LOG_DEBUG, "do_sample_radius, buf=%s", buf);
			if (strlen(buf) < STATUS_BUF_MIN_LEN )
			{
				fclose(fp);
				fp = NULL;
				return cur_state;
			}
			
			sscanf(buf, "%2[0-9] %16[0-9.] %5[0-9]", sstate, ip, sport);
			
			state =	strtoul(sstate, NULL, 10);
			port  = strtoul(sport, NULL, 10);

			PRADIUS_SERVER_STATUS_INFO userInfo = (PRADIUS_SERVER_STATUS_INFO)get_sample_user_data(me);
			syslog(LOG_DEBUG, "do_sample_radius, preIP=%s, prePort=%d", userInfo->ip, userInfo->port);

			memset(userInfo->ip, 0, sizeof(userInfo->ip));
			strcpy(userInfo->ip, ip);
			userInfo->port = port;			
		}	

		fclose(fp);
		fp = NULL;
	}

	if (1==state)
	{
		cur_state = SERVER_REACH_THRESHOLD;
	}

	return cur_state;
}
#endif 

