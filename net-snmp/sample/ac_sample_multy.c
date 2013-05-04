
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include "nm_list.h"
#include "ac_sample.h"
#include "ac_sample_err.h"
#include "ac_sample_def.h"
#include "ac_sample_multy.h"
#include "ac_manage_def.h"
#include "ws_dbus_def.h"
#include "ac_sample_dbus.h"
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpathInternals.h>
#include "wcpss/asd/asd.h"
#include "ws_sta.h"
#include "wcpss/wid/WID.h"
#include "board/board_define.h"
#include "ws_dbus_list_interface.h"

#include "hmd/hmdpub.h"
#include "ws_local_hansi.h"

#include "eag_errcode.h"
#include "eag_conf.h"
#include "eag_interface.h"


#define AWK_GET_DHCP_IP_POOL_INFO 	"/opt/www/htdocs/dhcp/pool_seg.xml"
#define AWK_GET_SYN_ATTACK_INFO		" dmesg | tail -n 3 | grep SYN_FLOOD_DETECTED | awk ' { print $3}' "

#define GET_CMD_STDOUT(buff,buff_size,cmd)\
	{\
		FILE *fp;\
		fp = popen( cmd,"r" );\
		if( NULL != fp ){\
			memset(buff,0,sizeof(buff));\
			fgets( buff, buff_size, fp );\
			pclose(fp);\
		}\
	}

#define BUF_LEN 					1024

struct online_user_info
{
	unsigned long long used;
    unsigned long long total;
};

struct ip_pool_info
{
	unsigned long long used;
    unsigned long long total;
};

struct porter_req_info
{
	unsigned long long used;
    unsigned long long total;
};

struct dhcp_ippool_info
{
	unsigned long long used;
    unsigned long long total;
};

struct syn_attack_info
{
	unsigned long long used;
    unsigned long long total;
};

int under_threshold_check(int threshold, int sample_value)
{
	int iRet = NOT_OVER_THRESHOLD;
	
	if (sample_value < threshold )
	{
		iRet = OVER_THRESHOLD_FLAG;
	}

	return  iRet;
}


static int do_sample_online_user( ac_sample_t *me )
{
	unsigned int sample = 0;
	
#if 0	
	instance_parameter *paraHead = NULL, *paraNode = NULL;
    list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER);
    for(paraNode = paraHead; NULL != paraNode; paraNode = paraNode->next) {
    
        int result = 0;
        struct dcli_ac_info *ac = NULL;
	    result = show_station_list_by_group (paraNode->parameter, paraNode->connection,&ac);

        if (1==result)
        {
        	sample += ac->num_sta_all;
        	Free_sta_summary(ac);
        }
    }    
    free_instance_parameter_list(&paraHead);
#endif    

	return sample;
}

int ac_sample_online_user_send_over_threshold_signal( ac_sample_t *this, int type )
{
	unsigned int sampleuser=0;
	
	syslog( LOG_INFO, "ac_sample_online_user_send_over_threshold_signal  type=%d\n", type);

	sampleuser = sw_get_sample_value(this);
	if (AS_RTN_PARAM_ERR == sampleuser )
	{
		return AS_RTN_ERR;
	}

	return ac_sample_dbus_send_signal( AC_SAMPLE_OVER_THRESHOLD_SIGNAL_MAX_USER, 
										 DBUS_TYPE_INT32, &type,
										 DBUS_TYPE_UINT32,&sampleuser,
										 DBUS_TYPE_INVALID );
}

ac_sample_t *create_ac_sample_max_user(  
                                unsigned int sample_interval, 
                                unsigned int statistics_time )
{
	ac_sample_t *pret = NULL;

	
	acsample_conf config;
	int ret=-1;

	memset(&config, 0, sizeof(config));
	set_default_config( &config );
	if ((ret=acsample_read_conf( &config, SAMPLE_NAME_MAX_USER ) ))
	{
		syslog(LOG_INFO,"%s acsample_read_conf:error return %d load default config", SAMPLE_NAME_MAX_USER, ret );
	}
	
    pret = create_ac_sample(SAMPLE_NAME_MAX_USER, config.sample_interval, config.statistics_time, config.resend_interval );

	if( NULL != pret )
	{
		set_do_sample_cb( pret, do_sample_online_user );
		set_sample_state( pret, config.sample_on );
		
		set_sample_threshold( pret, config.threshold );
		set_over_threshold_type(pret, OVER_THRESHOLD_CHECK_TYPE_IMMEDIATELY );
		
		set_over_threshold_cb( pret, ac_sample_online_user_send_over_threshold_signal );
	}

	syslog(LOG_INFO, "create_ac_sample_max_user  %p", pret );
	
	syslog(LOG_INFO,"config: sample_on=%d, statistics_time=%d, sample_interval=%d, resend_interval=%d, reload_config_interval=%d, over_threshold_check_type=%d, clear_check_type=%d, threshold=%d",
							config.sample_on, config.statistics_time, config.sample_interval, config.resend_interval,\
							config.reload_config_interval, config.over_threshold_check_type, config.clear_check_type, config.threshold );
	return pret;
}

static void 
create_radius_req_data(struct instance_radius_req **radius_req_data) {
    
    *radius_req_data = NULL;
    
	struct instance_radius_req *temp = NULL;
	temp = (struct instance_radius_req *)malloc(sizeof(struct instance_radius_req));
	if(NULL == temp) {
        syslog(LOG_WARNING, "create_radius_req_data: malloc radius req data failed!\n");
	    return ;
	}

    memset(temp, 0, sizeof(struct instance_radius_req));
    *radius_req_data = temp;
    
	return ;
}

static void
destroy_radius_req_info(struct instance_radius_req **radius_req_info) {
    if(NULL == radius_req_info || NULL == *radius_req_info) {
        return ;
    }
    
    free(*radius_req_info);
    *radius_req_info = NULL;
    
    return ;
}

static void
destroy_radius_req_user_data(void *userData) {

	struct list_head *list = ( struct list_head *)userData;	
	if(NULL == list){
		return ;
	}

	struct instance_radius_req *pos=NULL;
	struct instance_radius_req *next = NULL;

	list_for_each_entry_safe(pos, next, list, node) {
		list_del(&(pos->node));
		destroy_radius_req_info(&pos);
	}

	return;
}

static int
redius_req_rate_do_statistics(ac_sample_t *me) {

    if(NULL == me) {
        syslog(LOG_WARNING, "redius_req_rate_do_statistics: input para error!\n");
        return AS_RTN_NULL_POINTER;
    }

	ac_sample_t *pRadiusReq = get_ac_sample_by_name(SAMPLE_NAME_RADIUS_REQ_SUC);
	
	int sample_status = get_sample_state(pRadiusReq);
	const char *name  = get_sample_name(pRadiusReq);
	if( SAMPLE_ON != sample_status ) {
		syslog(LOG_DEBUG, "sample %s is not on!", name);
		return AS_RTN_OK;
	}

	int threshold = get_sample_threshold(pRadiusReq);
	int resend_interval = get_resend_interval(pRadiusReq);
	syslog(LOG_DEBUG, "sample %s threshold =%d, drop_do_statistics\n", name, threshold);

    struct instance_radius_req *pos = NULL;
    struct list_head *iflist_head = NULL;
    iflist_head = (struct list_head *)get_sample_user_data( get_ac_sample_by_name(SAMPLE_NAME_RADIUS_REQ_SUC));

    list_for_each_entry(pos, iflist_head, node) {        

        if(SAMPLE_INSTANCE_STANDBY == pos->instance_status) {
            pos->pre_instance_status = pos->instance_status;
            continue;
        }
        
        unsigned int time_now = time(NULL);
            
        if(threshold < 100) {
            if(pos->access_accept_rate < threshold) {
	        	syslog(LOG_DEBUG, "%s instance %d %s, check over threshold!  threshold: %d  latest:%d\n", 
	        	                    pos->local_id ? "Local-hansi" : "Remote-hansi", pos->instance_id, name, threshold, pos->access_accept_rate);
										
				if(0 == resend_interval) {/*do not resend if resend signal inverval is 0*/
					if(NOT_OVER_THRESHOLD == pos->last_status) {
					    
                        pos->last_over_time = time_now;
                        pos->last_status    = OVER_THRESHOLD_FLAG;

						ac_sample_radius_req_suc_rate_under_threshold_signal(pos->local_id, 
						                                                     pos->instance_id, 
						                                                     pos->access_accept_rate, 
						                                                     threshold, 
						                                                     pos->last_status);
					}
					else {
						syslog(LOG_DEBUG, "%s instance %d req_rate = %d, not resend signal because resend interval is 0\n", 
						                    pos->local_id ? "Local-hansi" : "Remote-hansi", pos->instance_id, pos->access_accept_rate);
					}
				}
				else if((time_now - pos->last_over_time) > resend_interval) {/*check signal resend time!*/
					if(NOT_OVER_THRESHOLD == pos->last_status) {
						syslog(LOG_DEBUG, "%s call callback to send signal!\n", name );
					}
					else {
						syslog(LOG_DEBUG, "%s call callback to resend signal!  resend!\n", name );
					}
					
                    pos->last_over_time = time_now;
                    pos->last_status    = OVER_THRESHOLD_FLAG;
                    
                    ac_sample_radius_req_suc_rate_under_threshold_signal(pos->local_id, 
                                                                         pos->instance_id, 
                                                                         pos->access_accept_rate, 
                                                                         threshold, 
                                                                         pos->last_status);
				}
				else {
					syslog(LOG_DEBUG, "%s resend signal interval %d! last send time %d, now time %d, div %d\n", 
											name, 
											resend_interval,
											pos->last_over_time,
											time_now,
											time_now - pos->last_over_time);
				}
            }
            else {
                if(OVER_THRESHOLD_FLAG == pos->last_status && SAMPLE_INSTANCE_ACTIVE == pos->pre_instance_status) {
					syslog(LOG_DEBUG, "%s instance %d %s send trap clear signal, latest :%d\n",
					                    pos->local_id ? "Local-hansi" : "Remote-hansi", pos->instance_id, name, pos->access_accept_rate);

                    pos->last_over_time = time_now;
                    pos->last_status    = NOT_OVER_THRESHOLD;
                    
                    ac_sample_radius_req_suc_rate_under_threshold_signal(pos->local_id, 
                                                                         pos->instance_id, 
                                                                         pos->access_accept_rate, 
                                                                         threshold, 
                                                                         pos->last_status);
                    
                }
            }
        }
        else if(NOT_OVER_THRESHOLD == pos->last_status || (resend_interval && (time_now - pos->last_over_time) > resend_interval)){
			syslog(LOG_DEBUG, "%s instance %d %s, send trap signal because threshold set to 100!!", 
			                     pos->local_id ? "Local-hansi" : "Remote-hansi", pos->instance_id, name);

            pos->last_over_time = time_now;
            pos->last_status    = OVER_THRESHOLD_FLAG;

            ac_sample_radius_req_suc_rate_under_threshold_signal(pos->local_id, 
                                                                 pos->instance_id, 
                                                                 pos->access_accept_rate, 
                                                                 threshold, 
                                                                 pos->last_status);
        }

        pos->pre_instance_status = pos->instance_status;
        pos->instance_status = SAMPLE_INSTANCE_STANDBY;
    }

    return AS_RTN_OK;
}


static int
instance_redius_req_set_sample_data(ac_sample_t *me, struct instance_radius_req *radius_req_data) {
    
	struct instance_radius_req *pos = NULL;
    struct list_head *req_head = NULL;
    
    req_head = (struct list_head *)get_sample_user_data(me);
	list_for_each_entry(pos, req_head, node)
	{
	    if(pos->local_id == radius_req_data->local_id && pos->instance_id == radius_req_data->instance_id) {

            pos->local_id = radius_req_data->local_id;
            pos->instance_id = radius_req_data->instance_id;

            if(SAMPLE_INSTANCE_ACTIVE == pos->pre_instance_status) {
                unsigned long pre_access_request_count = pos->access_request_count;                
                unsigned long pre_access_accept_count = pos->access_accept_count;

                pos->access_request_count = radius_req_data->access_request_count;
                pos->access_accept_count = radius_req_data->access_accept_count;
                
                if(pos->access_request_count == pre_access_request_count) {
                    pos->access_accept_rate = 100;
                }
                else {
                    pos->access_accept_rate = (pos->access_accept_count - pre_access_accept_count) / (pos->access_request_count - pre_access_request_count);
                }
            }
            else {
                pos->access_request_count = radius_req_data->access_request_count;
                pos->access_accept_count = radius_req_data->access_accept_count;
                pos->access_accept_rate = 100;
            }
            
            pos->instance_status = SAMPLE_INSTANCE_ACTIVE;
            
            syslog(LOG_DEBUG, "instance_redius_req_set_sample_data: %s instance %d, ", pos->local_id ? "Local-hansi" : "Remote-hansi", pos->instance_id);
            syslog(LOG_DEBUG, "access_request_count = %d, access_accept_count = %d, access_accept_rate = %d\n", pos->access_request_count, pos->access_accept_count, pos->access_accept_rate);

            return 0;
	    }
	}
	
    create_radius_req_data(&pos);
    if(NULL == pos) {
        return -1;
    }

	list_add(&(pos->node), req_head);

    pos->local_id = radius_req_data->local_id;
    pos->instance_id = radius_req_data->instance_id;
    pos->instance_status = SAMPLE_INSTANCE_ACTIVE;

    pos->access_request_count = radius_req_data->access_request_count;
    pos->access_accept_count = radius_req_data->access_accept_count;
    pos->access_accept_rate = 100;
    
    syslog(LOG_DEBUG, "instance_redius_req_set_sample_data: create %s instance %d, ", pos->local_id ? "Local-hansi" : "Remote-hansi", pos->instance_id);
    syslog(LOG_DEBUG, "access_request_count = %d, access_accept_count = %d, access_accept_rate = %d\n", pos->access_request_count, pos->access_accept_count, pos->access_accept_rate);

    return 0;
}

static int do_sample_radius_req( ac_sample_t *me )
{
    syslog(LOG_DEBUG, "enter do_sample_radius_req\n");
    unsigned int local_slotID = 0;
	if(VALID_DBM_FLAG == get_dbm_effective_flag())
	{
		local_slotID = sample_get_product_info(PRODUCT_LOCAL_SLOTID);
	}
    if(0 == local_slotID) {

        return -1;
    }
    
    int ret = 0;
    struct Hmd_Board_Info_show *instance_head = NULL;
    syslog(LOG_DEBUG, "do_sample_radius_req: before show_broad_instance_info\n");
    ret = show_broad_instance_info(ac_sample_dbus_get_connection(), &instance_head);
    syslog(LOG_DEBUG, "do_sample_radius_req: after show_broad_instance_info, ret = %d\n", ret);
    if(1 == ret && instance_head) {

        struct Hmd_Board_Info_show *instance_node = NULL;
        for(instance_node = instance_head->hmd_board_list; NULL != instance_node; instance_node = instance_node->next) {

            if(instance_node->slot_no != local_slotID) {
                syslog(LOG_DEBUG, "do_sample_radius_req: init slot %d connection error\n", instance_node->slot_no);
                continue;
            }

            unsigned int instance_state = 0;
            int manual_ret = AC_MANAGE_SUCCESS;

            manual_ret = ac_manage_show_snmp_manual_instance(ac_sample_dbus_get_connection(), &instance_state);
            syslog(LOG_DEBUG,"do_sample_radius_req: after show slot %d manual set instance state: manual_ret = %d, instance_state = %d\n",
                              instance_node->slot_no, manual_ret, instance_state);
            
            int i = 0;
            for(i = 0; i < instance_node->InstNum; i++) {
                if(0 >= instance_node->Hmd_Inst[i]->Inst_ID){
                    continue;
                }
                
                if ((1 == instance_node->Hmd_Inst[i]->isActive) || 
                    (AC_MANAGE_SUCCESS == manual_ret && (instance_state & (0x1 << (instance_node->Hmd_Inst[i]->Inst_ID - 1))))){
			syslog(LOG_DEBUG, "do_sample_radius_req: remote hansi %d-%d is master\n", 
                                        instance_node->slot_no, instance_node->Hmd_Inst[i]->Inst_ID);

			struct instance_radius_req temp_data = { 0 };
                    
			temp_data.local_id = 0;
			temp_data.instance_id = instance_node->Hmd_Inst[i]->Inst_ID;

			struct list_head ap_stat = {0};
			struct eag_ap_stat *tmp = NULL;

			eag_get_ap_statistics(ac_sample_dbus_get_connection(), temp_data.local_id, temp_data.instance_id, &ap_stat);
  			list_for_each_entry(tmp, &ap_stat, node) {
				temp_data.access_request_count += tmp->access_request_count;
 				temp_data.access_accept_count += tmp->access_accept_count;
  			}
			eag_free_ap_statistics(&ap_stat);
                                            
			instance_redius_req_set_sample_data(me, &temp_data);       
                }
            }

            for(i = 0; i < instance_node->LocalInstNum; i++) { 
                if(0 >= instance_node->Hmd_Local_Inst[i]->Inst_ID){
                    continue;
                }
                
                if((1 == instance_node->Hmd_Local_Inst[i]->isActive) || 
                    (AC_MANAGE_SUCCESS == manual_ret && (instance_state & (0x1 << (instance_node->Hmd_Local_Inst[i]->Inst_ID + INSTANCE_NUM - 1))))) {

                    	syslog(LOG_DEBUG, "do_sample_radius_req: local hansi %d-%d is master\n", 
                                        instance_node->slot_no, instance_node->Hmd_Local_Inst[i]->Inst_ID);

                   	struct instance_radius_req temp_data = { 0 };
                    
                    	temp_data.local_id = 1;
                 	temp_data.instance_id = instance_node->Hmd_Local_Inst[i]->Inst_ID;

                    	struct list_head ap_stat = { 0 };
			struct eag_ap_stat *tmp = NULL;
                    
    			eag_get_ap_statistics(ac_sample_dbus_get_connection(), temp_data.local_id, temp_data.instance_id, &ap_stat);
    				list_for_each_entry(tmp, &ap_stat, node)
    				{
    					temp_data.access_request_count += tmp->access_request_count;
 						temp_data.access_accept_count += tmp->access_accept_count;
    				}
    				eag_free_ap_statistics(&ap_stat);
                                            
                    	instance_redius_req_set_sample_data(me, &temp_data);       
                }
            }
        }
    }

	free_broad_instance_info(&instance_head);

    redius_req_rate_do_statistics(me);
    
	return 0;
	
}

int ac_sample_radius_req_suc_rate_under_threshold_signal(unsigned int local_id, 
                                                                        unsigned int instance_id, 
                                                                        unsigned int rate, 
                                                                        unsigned int threshold,
                                                                        unsigned int type)
{
	unsigned int samplerate = 0;
	
	syslog(LOG_INFO, "ac_sample_radius_req_send_over_threshold_signal %s instance %d , rate = %d, threshold = %d,  type = %d\n",
	                    local_id ? "Local-hansi" : "Remote-hansi", instance_id, rate, threshold, type);
	
	return ac_sample_dbus_send_signal( AC_SAMPLE_OVER_THRESHOLD_SIGNAL_RADIUS_REQ,
                                         DBUS_TYPE_INT32,  &type,
                                         DBUS_TYPE_UINT32, &local_id,
	                                     DBUS_TYPE_UINT32, &instance_id,
	                                     DBUS_TYPE_UINT32, &rate,
										 DBUS_TYPE_UINT32, &threshold,
										 DBUS_TYPE_INVALID );
}

ac_sample_t *create_ac_sample_radius_req(  
                                unsigned int sample_interval, 
                                unsigned int statistics_time )
{
	static struct list_head req_list_head = LIST_HEAD_INIT(req_list_head);

	ac_sample_t *pret = NULL;

	
	acsample_conf config;
	int ret=-1;

	memset(&config, 0, sizeof(config));
	set_default_config( &config );
	if ((ret=acsample_read_conf( &config, SAMPLE_NAME_RADIUS_REQ_SUC ) ))
	{
		syslog(LOG_INFO,"%s acsample_read_conf:error return %d load default config", SAMPLE_NAME_RADIUS_REQ_SUC, ret );
	}

    pret = create_ac_sample(SAMPLE_NAME_RADIUS_REQ_SUC, config.sample_interval, config.statistics_time, config.resend_interval );

	if( NULL != pret )
	{
		set_do_sample_cb( pret, do_sample_radius_req );
		set_sample_state( pret, config.sample_on );
		
		set_sample_threshold( pret, config.threshold );
		set_do_check_cb(pret, NULL);
		
		set_sample_user_data( pret, &req_list_head, destroy_radius_req_user_data);
	}

	syslog( LOG_INFO, "create_ac_sample_radius_req  %p", pret );

	syslog(LOG_INFO,"config: sample_on=%d, statistics_time=%d, sample_interval=%d, resend_interval=%d, reload_config_interval=%d, over_threshold_check_type=%d, clear_check_type=%d, threshold=%d",
							config.sample_on, config.statistics_time, config.sample_interval, config.resend_interval,\
							config.reload_config_interval, config.over_threshold_check_type, config.clear_check_type, config.threshold );
	return pret;
}


#if 0
/////////get dhcp trap effictive information////////////////////
int dhcp_get(int *whole,int *seg)
{
	xmlDocPtr doc = NULL;   //定义解析文档指针
	xmlNodePtr cur = NULL;
	xmlNodePtr root_node=NULL; //定义结点指针(你需要它为了在各个结点间移动)
	char tmp[BUF_LEN] = { 0 };
	int ret = 0;
	
	if ( -1==(ret=access(AWK_GET_DHCP_IP_POOL_INFO, 0)) )
	{
		return OPT_RTN_FILE_NOT_EXIST;
	}
	doc = xmlReadFile( AWK_GET_DHCP_IP_POOL_INFO, "UTF-8", BUF_LEN); //解析文件
	if (NULL==doc)
	{
		syslog(LOG_ERR, "Document not parsed successfully. %s\n", AWK_GET_DHCP_IP_POOL_INFO);
		return OPT_RTN_FILE_NOT_EXIST;
	}	   
	
	root_node= xmlDocGetRootElement(doc);
	if (NULL==root_node)
	{	
		xmlFreeDoc(doc);
		return OPT_RTN_FILE_ERR;
	}
	if (xmlStrcmp(root_node->name, (const xmlChar *)"root"))
	{	
		syslog(LOG_ERR, "document of the wrong type, root node != root %s\n", AWK_GET_DHCP_IP_POOL_INFO);	
		xmlFreeDoc(doc);	
		return OPT_RTN_FILE_ERR;
	}
	
	cur = root_node->xmlChildrenNode;
	while (NULL!=cur)
	{
		if ((!xmlStrcmp(cur->name, (const xmlChar *)"whole")))
		{
			xmlChar *value;
	 	  	value = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
		  
		  	if (NULL!=value)
		  	{
	 	  		strncpy(tmp,(char*)value, sizeof(tmp));
		  		*whole = atoi(tmp);
	   	  		xmlFree(value);
		  	}
	   }
	   else if ((!xmlStrcmp(cur->name, (const xmlChar *)"seg")))
	   {
	  		xmlChar *segvalue;
	  		segvalue = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);

			if( NULL != segvalue )
			{
				strncpy( tmp, (char*)segvalue, sizeof(tmp) );
	 			*seg = atoi(tmp);
	  			xmlFree(segvalue);
			}
	   }
	   
	   cur = cur->next;
	}

	xmlFreeDoc(doc);
	xmlCleanupParser();
	
	return OPT_RTN_OK;
	
}

static int do_sample_ip_pool( ac_sample_t *me )
{
	unsigned long long sample = 0;
	int assigned_ip = 0;
	int total_ip    = 0;
	if (OPT_RTN_OK == dhcp_get(&total_ip, &assigned_ip) && 
			0!=total_ip )
	{
		sample = assigned_ip*100/total_ip;
	}

	return (int)sample;
}

int ac_sample_ippool_send_over_threshold_signal( ac_sample_t *this, int type )
{
	unsigned int samplerate = 0;
	
	syslog( LOG_INFO, "ac_sample_ippool_send_over_threshold_signal  type=%d\n", type);

	samplerate = sw_get_sample_value(this);
	if (AS_RTN_PARAM_ERR == samplerate)
	{
		return AS_RTN_ERR;
	}
	
	return ac_sample_dbus_send_signal( AC_SAMPLE_OVER_THRESHOLD_SIGNAL_IP_POOL, 
										 DBUS_TYPE_INT32, &type,
										 DBUS_TYPE_UINT32,&samplerate,
										 DBUS_TYPE_INVALID );
}

ac_sample_t *create_ac_sample_ip_pool(  
                                unsigned int sample_interval, 
                                unsigned int statistics_time )		//do
{
	ac_sample_t *pret = NULL;
	
    pret = create_ac_sample(SAMPLE_NAME_IP_POOL, sample_interval, statistics_time );

	if( NULL != pret )
	{
		set_do_sample_cb( pret, do_sample_ip_pool );
		set_sample_state( pret, SAMPLE_ON );
		
		set_sample_threshold( pret, DEFAULT_IPPOOL_RATE_THRESHOLD );
		
		set_over_threshold_cb( pret, ac_sample_ippool_send_over_threshold_signal );
	}

	syslog( LOG_DEBUG, "create_ac_sample_ip_pool  %p", pret );
	return pret;
}



static int do_sample_dhcp_ippool( ac_sample_t *me )
{
	int has_assigned_ip = 0;
	int ip_pool_total = 0;
	
	int dhcp_ret = dhcp_get (&ip_pool_total,&has_assigned_ip);
	if (OPT_RTN_OK != dhcp_ret)
	{
		return 0;
	}
	set_sample_threshold( me, ip_pool_total);

	return has_assigned_ip;
}

int ac_sample_dhcp_exhaust_send_over_threshold_signal( ac_sample_t *this, int type )
{
	unsigned int samplerate=0;
	
	syslog( LOG_INFO, "ac_sample_dhcp_exhaust_send_over_threshold_signal  type=%d\n", type);

	samplerate = sw_get_sample_value(this);
	if (AS_RTN_PARAM_ERR == samplerate)
	{
		return AS_RTN_ERR;
	}
	
	return ac_sample_dbus_send_signal( AC_SAMPLE_OVER_THRESHOLD_SIGNAL_DHCP_EXHAUST, 
										 DBUS_TYPE_INT32, &type,
										 DBUS_TYPE_UINT32,&samplerate,
										 DBUS_TYPE_INVALID );
}

ac_sample_t *create_ac_sample_dhcp_exhaust(  
                                unsigned int sample_interval, 
                                unsigned int statistics_time )		//do
{
	ac_sample_t *pret = NULL;

	
    pret = create_ac_sample(SAMPLE_NAME_DHCP_IPPOOL, sample_interval, statistics_time );

	if( NULL != pret )
	{
		set_do_sample_cb( pret, do_sample_dhcp_ippool );
		set_sample_state( pret, SAMPLE_ON );

		int has_assigned_ip = 0;
		int ip_pool_total = 0;
		int dhcp_ret = dhcp_get (&ip_pool_total,&has_assigned_ip);
		if (OPT_RTN_OK != dhcp_ret)
		{
			set_sample_threshold(pret, 1);
		}
		else
		{
			set_sample_threshold( pret, ip_pool_total);
		}
		set_over_threshold_type( pret, OVER_THRESHOLD_CHECK_TYPE_IMMEDIATELY);
		
		set_over_threshold_cb( pret, ac_sample_dhcp_exhaust_send_over_threshold_signal );
	}

	syslog( LOG_DEBUG, "create_ac_sample_dhcp_exhaust  %p", pret );
	return pret;
}

#endif

static int do_sample_syn_attack( ac_sample_t *me )
{
	FILE * fp = NULL;
	char buf[4096] = { 0 };
	unsigned int sample = NORM_STAT_THRESHOLD;
	
	if (NULL!=(fp=popen(AWK_GET_SYN_ATTACK_INFO, "r")) )
	{
		fgets( buf, 4096, fp );
		if (0!=strcmp(buf,""))
		{
			sample = FIND_ATTACK_THRESHOLD;
		}
		
		pclose(fp);
	}

	return sample;
}

int ac_sample_synattack_send_over_threshold_signal( ac_sample_t *this, int type )
{
	unsigned int samplerate = 0;
	
	syslog( LOG_INFO, "ac_sample_synattack_send_over_threshold_signal  type=%d\n", type);

	samplerate = sw_get_sample_value(this);
	if (AS_RTN_PARAM_ERR == samplerate)
	{
		return AS_RTN_ERR;
	}
	
	return ac_sample_dbus_send_signal( AC_SAMPLE_OVER_THRESHOLD_SIGNAL_FIND_SYN_ATTACK, 
										 DBUS_TYPE_INT32, &type,
										 DBUS_TYPE_UINT32,&samplerate,
										 DBUS_TYPE_INVALID );
}

ac_sample_t *create_ac_sample_find_attack(  
                                unsigned int sample_interval, 
                                unsigned int statistics_time )		//do
{
	ac_sample_t *pret = NULL;

	acsample_conf config;
	int ret=-1;

	memset(&config, 0, sizeof(config));
	set_default_config( &config );
	if ((ret=acsample_read_conf( &config, SAMPLE_NAME_FINE_SYN_ATTACK ) ))
	{
		syslog(LOG_INFO,"%s acsample_read_conf:error return %d load default config", SAMPLE_NAME_FINE_SYN_ATTACK, ret );
	}
	
    pret = create_ac_sample(SAMPLE_NAME_FINE_SYN_ATTACK, config.sample_interval, config.statistics_time, config.resend_interval );

	if( NULL != pret )
	{
		set_do_sample_cb( pret, do_sample_syn_attack );
		set_sample_state( pret, config.sample_on );
		set_sample_threshold( pret, config.threshold );
		set_over_threshold_type( pret, OVER_THRESHOLD_CHECK_TYPE_IMMEDIATELY);
		
		set_over_threshold_cb( pret, ac_sample_synattack_send_over_threshold_signal );
	}

	syslog( LOG_DEBUG, "create_ac_sample_find_syn_attack  %p", pret );
	
	syslog(LOG_INFO,"config: sample_on=%d, statistics_time=%d, sample_interval=%d, resend_interval=%d, reload_config_interval=%d, over_threshold_check_type=%d, clear_check_type=%d, threshold=%d",
							config.sample_on, config.statistics_time, config.sample_interval, config.resend_interval,\
							config.reload_config_interval, config.over_threshold_check_type, config.clear_check_type, config.threshold );
	return pret;
}

