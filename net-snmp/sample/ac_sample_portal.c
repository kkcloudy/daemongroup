#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>

#include <netinet/in.h> 
#include <netdb.h> 
#include <sys/types.h> 
#include <sys/socket.h>
#include <asm/ioctls.h>
#include <sys/ioctl.h> 
#include <sys/select.h> 
#include <sys/errno.h> 

#include "nm_list.h"
#include "ws_dbus_def.h"
#include "ac_sample.h"
#include "ac_sample_err.h"
#include "ac_sample_def.h"
#include "ac_sample_dbus.h"

#include "ws_snmpd_engine.h"
#include "ws_snmpd_manual.h"
#include "ac_manage_def.h"
#include "ac_manage_interface.h"

#include "ac_sample_portal.h"

#include "hmd/hmdpub.h"
#include "ws_local_hansi.h"

#include "eag_errcode.h"
#include "eag_conf.h"
#include "eag_interface.h"
#include "ac_manage_sample_interface.h"


typedef struct {
    unsigned int local_id;
	unsigned int instance_id;
    
	char str_ip[IP_STR_LEN];
	short port;
	
}SERVER_STATUS_INFO, *PSERVER_STATUS_INFO;

typedef struct {
	int fd;	//portal socket
	
}PORTAL_PRIVATE_DATA, *PPORTAL_PRIVATE_DATA;


static int get_portal_param_by_portal_url ( 	char *portal_url, 
													char *url_prefix,
													int url_prefix_len,
													int *ip_addr, 
													int *port, 
													char * web_page,
													int web_page_len );


static char *
inet_ntoa_ex(unsigned long ip, char *ipstr, unsigned int buffsize)
{
	snprintf(ipstr, buffsize, "%lu.%lu.%lu.%lu", ip >> 24,
		 (ip & 0xff0000) >> 16, (ip & 0xff00) >> 8, (ip & 0xff));

	return ipstr;
}


#if 0
static int do_sample_portal_state( ac_sample_t *me )
{
	syslog(LOG_DEBUG, "do_sample_portal_state, line=%d", __LINE__);

	FILE * fp = NULL;
	int cur_portal_state = SERVER_REACH_THRESHOLD;
	int state = 0;	
	
	if (NULL != (fp=fopen(PORTAL_SERVER_STATUS_FILE, "r")) )
	{		
		char sstate[2]={0};
		char ip[IP_STR_LEN] = {0};
		int port	= 0;
		char sport[5] = {0};

		char buf[128] = {0};
		if (NULL!=fgets(buf, sizeof(buf), fp) )
		{
			syslog(LOG_DEBUG, "do_sample_portal_state, buf=%s", buf);
			if (strlen(buf) < 11 )
			{
				fclose(fp);
				fp = NULL;
				return cur_portal_state;
			}
			
			sscanf(buf, "%2[0-9] %16[0-9.] %5[0-9]", sstate, ip, sport);
			
			state =	strtoul(sstate, NULL, 10);
			port  = strtoul(sport, NULL, 10);

			PSERVER_STATUS_INFO userInfo = (PSERVER_STATUS_INFO)get_sample_user_data(me);

			memset(userInfo->str_ip, 0, sizeof(userInfo->str_ip));
			strcpy(userInfo->str_ip, ip);
			userInfo->port = port;	
		}	

		fclose(fp);
		fp = NULL;
	}

	if (0!=state)
	{
		cur_portal_state = SERVER_NOT_REACH_THRESHOLD;
	}

	return cur_portal_state;
}
#endif


static int load_portal_config_by_instance(multi_sample_t *multi_sample, struct list_head *head, struct instance_portal_config *instance_config)
{	
    syslog(LOG_DEBUG, "enter load_portal_config_by_instance\n");

    if(NULL == multi_sample || NULL == head || NULL == instance_config) {
        syslog(LOG_WARNING, "load_portal_config_by_instance: input para error!\n");
        return AS_RTN_PARAM_ERR;
    }
    
    syslog(LOG_INFO, "load %s instance %d-%d portal config\n", 
                     instance_config->local_id ? "Local-hansi" : "Remote-hansi", instance_config->slot_id, instance_config->instance_id);

    

    int i = 0;
    for(i = 0; i < instance_config->portalconf.current_num; i++) {
        
        PSERVER_STATUS_INFO portal_data = (PSERVER_STATUS_INFO)SAMPLE_MALLOC(sizeof(SERVER_STATUS_INFO));
        if(NULL == portal_data) {
            syslog(LOG_ERR,"load_portal_config_by_instance: malloc error memory full!\n");
            return AS_RTN_MALLOC_ERR;
        }

        memset(portal_data, 0 , sizeof(*portal_data));
        
        int temp_ret = -1;
        char temp_buff[1024] = { 0 }, ipaddr[256] = { 0 };
        
        unsigned long portal_ip = 0;
        unsigned int port = 0;
        temp_ret = get_portal_param_by_portal_url(instance_config->portalconf.portal_srv[i].portal_url,
                                                    temp_buff,
                                                    sizeof(temp_buff),
                                                    &portal_ip,
                                                    &port,
                                                    temp_buff,
                                                    sizeof(temp_buff));

        if(0 != temp_ret) {
            free(portal_data);
            continue;
        }
        
        if(portal_ip) {
            inet_ntoa_ex(portal_ip, portal_data->str_ip, sizeof(portal_data->str_ip));
        }
        portal_data->port = port ? port : 80;
        portal_data->local_id = instance_config->local_id;
        portal_data->instance_id = instance_config->instance_id;
        
	syslog(LOG_DEBUG, "load_portal_config_by_instance slotid=%d, insid=%d, ip=%s, port=%u\n", 
			instance_config->slot_id, instance_config->instance_id, portal_data->str_ip, portal_data->port);
       temp_ret = create_config_data_3(multi_sample, head, portal_ip, portal_data);
       if(0 != temp_ret) {
            free(portal_data);
            continue;
        }
    }
	
    syslog(LOG_DEBUG, "exit load_portal_config_by_instance\n");
	return AS_RTN_OK;
}


int
get_all_portal_conf(multi_sample_t *multi_sample, struct list_head *head)
{
	syslog(LOG_DEBUG, "enter get_all_portal_conf\n");
	if (NULL == multi_sample || NULL == head) {
		syslog(LOG_WARNING, "get_all_portal_conf: input para error!\n");
		return AS_RTN_PARAM_ERR;
	}

	struct instance_portal_config *configHead = NULL;
	unsigned int config_num = 0;
	int ret = AC_MANAGE_SUCCESS;
	instance_parameter *paraHead = NULL;
	instance_parameter *paraNode = NULL;

	ret = get_master_instance_para(&paraHead);
	if (0 != ret) {
		syslog(LOG_WARNING, "get_all_portal_conf: get_master_instance_para failed!\n");
		return ret;
	}

	for (paraNode = paraHead; NULL != paraNode; paraNode = paraNode->next) {
		struct instance_portal_config *configNode = (struct instance_portal_config *)malloc(sizeof(struct instance_portal_config));
		if (NULL == configNode) {
			syslog(LOG_WARNING, "get_all_portal_conf: configNode malloc failed!\n");
			free_master_instance_para(&paraHead);
			return -1;
		}
		ret = eag_get_portal_conf(paraNode->connection, 
                                        paraNode->parameter.local_id, 
                                        paraNode->parameter.instance_id, 
                                        &configNode->portalconf);
		if (0 != ret) {
			syslog(LOG_WARNING, "get_all_portal_conf: eag_get_portal_conf failed!\n");
			free(configNode);
			continue;
		}
		configNode->slot_id = paraNode->parameter.slot_id;
		configNode->local_id = paraNode->parameter.local_id;
		configNode->instance_id = paraNode->parameter.instance_id;

		config_num++;
		ret = load_portal_config_by_instance(multi_sample, head, configNode);
		free(configNode);
	}

	free_master_instance_para(&paraHead);
	return 0;
	
}

static int portal_load_conf( multi_sample_t *multi_sample, struct list_head *head )
{
	syslog(LOG_DEBUG, "enter portal_load_conf\n");

	int ret = 0;

	if(NULL == multi_sample || NULL == head) {
		return AS_RTN_NULL_POINTER;
	}

	ret = get_all_portal_conf(multi_sample, head);
	if (0 != ret) {
		syslog(LOG_WARNING, "portal_load_conf: get_all_portal_conf failed!\n");
		return ret;
	}
    
	syslog(LOG_DEBUG, "exit portal_load_conf\n");
	return AS_RTN_OK;
}

static int do_sample_portal_state( ac_sample_t *me )
{
	syslog(LOG_INFO,"do_sample_portal_state entry.\n"); //test

	int fd=-1;
	int ret=0;

	if((fd = socket(PF_INET,SOCK_STREAM,0)) == -1 )
	{
		syslog(LOG_INFO,"portal_sample_user_data_init: socket is error\n;");
		return PORTAL_REACHABLE_VALUE;
	}

	unsigned long ul = 1;
	if(ioctl(fd, FIONBIO, &ul) == -1)
    {
        close(fd);
        return PORTAL_REACHABLE_VALUE;
    }
	
	PSERVER_STATUS_INFO user_data = ( PSERVER_STATUS_INFO ) get_subsample_user_data ( me );

	struct sockaddr_in to_addr;
	bzero( &(to_addr), sizeof(to_addr) );
	to_addr.sin_family = PF_INET;
	to_addr.sin_port = htons(user_data->port);
	to_addr.sin_addr.s_addr = inet_addr(user_data->str_ip);
	
	syslog(LOG_INFO, "connect portal %s %d",user_data->str_ip ,user_data->port); 

	if(connect(fd, (void *)&to_addr, sizeof(to_addr)) < 0)	
	{	
		if(errno != EINPROGRESS)
		{
			ret = PORTAL_UNREACHABLE_VALUE;
			goto out;
		}
		fd_set set;
		int len = sizeof(int) , error=-1;
		struct timeval timeout=gTimeout;
		FD_ZERO(&set);
		FD_SET(fd, &set);
		
		if( select(fd+1, NULL, &set, NULL, &timeout) > 0)
		{
			getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, (socklen_t *)&len);
			if(error == 0)
			{
				ret = PORTAL_REACHABLE_VALUE;
			}
			else 
				ret = PORTAL_UNREACHABLE_VALUE;
			
		} else
		{
			syslog(LOG_INFO,"portal connect timeout.");
			ret = PORTAL_UNREACHABLE_VALUE;
		}
		
		syslog(LOG_INFO, "portal %s %d connect ret %d",user_data->str_ip ,user_data->port, ret);

	out:	
		close ( fd );
		return ret;   
	}   
	
	close ( fd );
	
	syslog(LOG_INFO,"do_sample_portal_state exit.\n");	//test
	return PORTAL_REACHABLE_VALUE;
}

#if 0
void portal_sample_user_data_init ( PORTAL_PRIVATE_DATA user_data )
{
	int fd=0;

	if((fd = socket(PF_INET,SOCK_STREAM,0)) == -1 )
	   {
		   syslog(LOG_INFO,"portal_sample_user_data_init: socket is error\n;");
		   return;
	   }
	
	user_data->fd=fd;

	return;
	
}

void portal_sample_user_data_free ( PORTAL_PRIVATE_DATA user_data)
{
	if ( user_data->fd < 0 )
		return;

	close ( user_data->fd );

	return;
}
#endif

void portal_subsample_user_data_free ( void *user_data)
{
	if ( NULL==user_data )
		return ;
	
	PSERVER_STATUS_INFO subInfo =(PSERVER_STATUS_INFO) user_data;

	SAMPLE_FREE( subInfo );

	return;
}

int ac_sample_portal_state_send_signal( ac_sample_t *this, int type )
{	
	syslog( LOG_DEBUG, "ac_sample_portal_state_send_signal  type=%d\n", type);

	PSERVER_STATUS_INFO pInfo = ( PSERVER_STATUS_INFO )get_subsample_user_data(this);
	if ( NULL==pInfo )
	{
		return AS_RTN_PARAM_ERR;
	}
	syslog( LOG_DEBUG, "ac_sample_portal_state_send_signal  ip=%s, port=%d\n", pInfo->str_ip, pInfo->port);
	char *str = pInfo->str_ip;
	return ac_sample_dbus_send_active_signal( AC_SAMPLE_OVER_THRESHOLD_SIGNAL_PORTAL_REACH, 
    										 DBUS_TYPE_UINT32,  &type,
    										 DBUS_TYPE_STRING,  &str,
    										 DBUS_TYPE_UINT16,  &pInfo->port,
    										 DBUS_TYPE_UINT32,  &pInfo->local_id,
    										 DBUS_TYPE_UINT32,  &pInfo->instance_id,
    										 DBUS_TYPE_INVALID );
	
}

ac_sample_t *create_ac_sample_portal_server(  
                                unsigned int sample_interval, 
                                unsigned int statistics_time )		//do
{
	ac_sample_t *pret = NULL;
	
	acsample_conf config;
	int ret=-1;

	memset(&config, 0, sizeof(config));
	set_default_config( &config );
	if ((ret=acsample_read_conf( &config, SAMPLE_NAME_PORTER_NOT_REACH ) ))
	{
		syslog(LOG_INFO,"%s acsample_read_conf:error return %d load default config", SAMPLE_NAME_PORTER_NOT_REACH, ret );
	}

    pret = create_ac_sample(SAMPLE_NAME_PORTER_NOT_REACH, config.sample_interval, config.statistics_time, config.resend_interval );

	if( NULL != pret )
	{
		//system(PORTAL_SERVER_STATUS);
		//static PORTAL_PRIVATE_DATA portal_status = {.fd = 0 };

		//syslog( LOG_DEBUG, "create_ac_sample_portal_server  userdata ip=%s, port=%d", portal_status.str_ip, portal_status.port);

		set_multisample_conf( pret, portal_load_conf, config.reload_config_interval, portal_subsample_user_data_free, NULL);
		set_do_sample_cb( pret, do_sample_portal_state );
		set_sample_state( pret, config.sample_on );
		set_sample_threshold( pret, config.threshold );
		set_over_threshold_type( pret, OVER_THRESHOLD_CHECK_TYPE_NORMAL);
		set_clear_type(pret, OVER_THRESHOLD_CHECK_TYPE_IMMEDIATELY);
		//set_sample_user_data(pret, NULL, NULL);
		set_do_check_cb(pret, NULL);
		set_over_threshold_cb( pret, ac_sample_portal_state_send_signal );
	}

	syslog( LOG_DEBUG, "create_ac_sample_portal_server  %p", pret );
	
	syslog(LOG_INFO,"config: sample_on=%d, statistics_time=%d, sample_interval=%d, resend_interval=%d, reload_config_interval=%d, over_threshold_check_type=%d, clear_check_type=%d, threshold=%d",
							config.sample_on, config.statistics_time, config.sample_interval, config.resend_interval,\
							config.reload_config_interval, config.over_threshold_check_type, config.clear_check_type, config.threshold );

	return pret;
}

static int get_portal_param_by_portal_url
(
char *portal_url, 
char *url_prefix,
int url_prefix_len,
int *ip_addr, 
int *port, 
char * web_page,
int web_page_len
)
{
	if (	NULL == portal_url	|| NULL == ip_addr 
		||	NULL == port 		|| NULL == web_page
		||	NULL == url_prefix	|| 0 >= web_page_len
		||	0 >= url_prefix_len)
	{
		syslog(LOG_DEBUG,"get_portal_param_by_portal_url: param err!\n");
		return -1;
	}
	
	int ip_part_value = -1;
	int ip_part_num = -1;
	int i = 0;
	
	char *p_url = NULL;	
	p_url = portal_url;
	
	if ( 0 == strncmp(p_url,"http://",strlen("http://")))
	{
		strncpy(url_prefix,"http://", url_prefix_len);
		p_url += strlen("http://");
	}
	else if( 0 == strncmp(p_url,"https://",strlen("https://")))
	{
		strncpy(url_prefix,"https://", url_prefix_len);
		p_url += strlen("https://");
	}
	else
	{
		syslog(LOG_DEBUG,"get_portal_param_by_portal_url: url err! url must start as http:// or https:// \n");
		return -2;
	}
	
	/* get ip */
	ip_part_value = -1;
	ip_part_num = 1;
	*ip_addr = 0;
	
	for(i=0; i<17; i++)
	{
		if('\0' == *p_url)
		{
			syslog(LOG_DEBUG,"get_portal_param_by_portal_url: url ip err!\n");
			return -3;
		}
		else if ('/' == *p_url || ':' == *p_url)
		{
			if (0 <= ip_part_value && 255 >= ip_part_value && 4 == ip_part_num)
			{
				/* success */
				*ip_addr += ip_part_value;
				break;
			}
			else
			{
				syslog(LOG_DEBUG,"get_portal_param_by_portal_url: url ip err!\n");
				return -4;
			}
		}
		else if ('.' == *p_url)
		{
			if (		((1 == ip_part_num && 0 < ip_part_value) || (1 < ip_part_num && 0 <= ip_part_value))
					&&	255 >= ip_part_value
					&&	4 > ip_part_num)
			{
				/* legal */
				*ip_addr += ip_part_value << ((4-ip_part_num)*8);
				ip_part_value = -1;
				ip_part_num++;
			}
			else
			{
				syslog(LOG_DEBUG,"get_portal_param_by_portal_url: url ip err!\n");
				return -5;
			}
		}
		else if ('0' <= *p_url || '9' >= *p_url)
		{
			if (-1 == ip_part_value)
			{
				ip_part_value = 0;
			}
			else
			{
				ip_part_value *= 10;
			}
			
			ip_part_value += (*p_url - '0');
		}
		else
		{
			syslog(LOG_INFO, "url ip err!\n");
			return -6;
		}
		p_url++;
	}
	
	
	/* get port */
	*port = 0;
	if (':' == *p_url)
	{
		p_url++;
		while (NULL != p_url && '0' <= *p_url && '9' >= *p_url)
		{
			*port *= 10;
			*port += (*p_url - '0');			
			p_url++;
		}

		if (65535 < *port || 0 >= *port)
		{
				syslog(LOG_DEBUG,"get_portal_param_by_portal_url: url port err!\n");
				return -7;
		}		
	}
	else if ('/' == *p_url)
	{
		*port = 80;		
	}
	else
	{
		syslog(LOG_DEBUG,"get_portal_param_by_portal_url: url port err!\n");
		return -8;
	}
	
	/* get web page */
	if ('/' != *p_url)	
	{
		syslog(LOG_DEBUG,"get_portal_param_by_portal_url: web page err!\n");
		return -8;
	}
	else
	{		
		strncpy(web_page,p_url,web_page_len);
	}
	
	return 0;
}

