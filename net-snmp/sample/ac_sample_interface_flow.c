
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include "board/board_define.h"
#include "ac_sample.h"
#include "ac_sample_container.h"
#include "ac_sample_err.h"
#include "ac_sample_def.h"
#include "ws_dbus_def.h"
#include "ac_sample_dbus.h"

#include "ws_intf.h"

#include "ac_manage_def.h"
#include "ws_snmpd_engine.h"
#include "ws_snmpd_manual.h"
#include "ac_manage_interface.h"

#include "ac_sample_interface_flow.h"
#include "ac_sample_interface.h"

#define DEF_BAND_WIDTH			(1000*1024*1024/8)

static struct sample_rtmd_info intf_flow_info = {0};

int create_if_flow_info( struct if_flow_info_s **pifinfo  )
{
	struct if_flow_info_s *pret=NULL;

	pret = (struct if_flow_info_s *)malloc(sizeof(struct if_flow_info_s));
	if( NULL != pret )
	{
		memset( pret, 0, sizeof(struct if_flow_info_s) );
		*pifinfo = pret;

		return 0;
	}

	return -1;
}


int destroy_if_flow_info( struct if_flow_info_s **pifinfo )
{
	if (NULL != pifinfo)
	{
		return -1;
	}
    if(NULL != *pifinfo )
    {
        return -1;
    }

	free( *pifinfo );
	*pifinfo = NULL;

	return 0;
}

void destroy_iface_user_data( void *userData )
{
	struct list_head *list = ( struct list_head *)userData;
	

	if (NULL==list)
	{
		return;
	}

	struct if_flow_info_s *pos=NULL;
	struct if_flow_info_s *next = NULL;

	list_for_each_entry_safe(pos,next,list,node)
	{
		list_del(&(pos->node));
		destroy_if_flow_info(&pos);
	}

	return;
}


static int if_flow_info_set_sample_data( ac_sample_t *me, struct if_stats_list *if_stats_node )
{
	struct if_flow_info_s *pos=NULL;
	U64 pre_rxbytes = 0;
	U64 pre_txbytes = 0;
	unsigned int pre_time = 0;
	struct list_head *iflist_head = NULL;

	iflist_head = (struct list_head *)get_sample_user_data( me );

	list_for_each_entry( pos, iflist_head, node )
	{
		if(0 == strcmp(pos->name, if_stats_node->ifname)) {
			//pos->name
			pre_rxbytes = pos->rxbytes;
			pre_txbytes = pos->txtytes;
			pre_time = pos->sample_time;

			pos->sample_time = time(0);
			pos->rxbytes = if_stats_node->stats.rx_bytes;
			pos->txtytes = if_stats_node->stats.tx_bytes; 

			/*
			 * lixiang modify at 2012-2-15 
			 * check (pos->sample_time - pre_time) <= 0
			 */
			if(pos->sample_time > pre_time) {
				pos->txbandwidth = (pos->txtytes > pre_txbytes) ? ((pos->txtytes - pre_txbytes) / (pos->sample_time - pre_time)) : 0;
				pos->rxbandwidth = (pos->rxbytes > pre_rxbytes) ? ((pos->rxbytes - pre_rxbytes) / (pos->sample_time - pre_time)) : 0;
			} else {
				pos->txbandwidth = 0;
				pos->rxbandwidth = 0;
			}
			
			//for drop trate
			pos->packets = if_stats_node->stats.rx_packets;
			pos->drops   = if_stats_node->stats.rx_dropped;
			
			syslog( LOG_DEBUG, "pos->name = %s\n!", pos->name );
			syslog( LOG_DEBUG, "pre_rxbytes = %llu\n!", pre_rxbytes );
			syslog( LOG_DEBUG, "pre_txtytes = %llu\n!", pre_txbytes );
			syslog( LOG_DEBUG, "pos->rxbytes = %llu\n!", pos->rxbytes );
			syslog( LOG_DEBUG, "pos->txtytes = %llu\n!", pos->txtytes );
			syslog( LOG_DEBUG, "pos->txbandwidth = %llu\n!", pos->txbandwidth );
			syslog( LOG_DEBUG, "pos->rxbandwidth = %llu\n!", pos->rxbandwidth );
			syslog( LOG_DEBUG, "pos->packets = %llu\n!", 	pos->packets);
			syslog( LOG_DEBUG, "pos->drop = %llu\n!", 		pos->drops);

			return 0;
		}
	}

	
	create_if_flow_info(&pos);
	if (NULL==pos)
	{
		return -1;
	}
	list_add( &(pos->node), iflist_head);
	pos->sample_time = time(0);
	pos->rxbytes = if_stats_node->stats.rx_bytes;
	pos->txtytes = if_stats_node->stats.tx_bytes; 
	strncpy(pos->name, if_stats_node->ifname, sizeof(pos->name) - 1);
	syslog(LOG_DEBUG, "list add: pos->name = %s, ifname=%s\n!", pos->name, if_stats_node->ifname);
	if(0 != get_if_index(if_stats_node->ifname, &pos->ifindex)) {
		syslog( LOG_WARNING, "get if %s index error\n", if_stats_node->ifname);
	}

	//for drop trate
	pos->packets = if_stats_node->stats.rx_packets;
	pos->drops   = if_stats_node->stats.rx_dropped;
	pos->drop_info.last_send_over_time = 0;
	pos->drop_info.last_status = NOT_OVER_THRESHOLD;

	//for band_width
	pos->band_info.last_send_over_time = 0;
	pos->band_info.last_status = NOT_OVER_THRESHOLD;
	
//the first time do not  co	

	return 0;
}

static int ac_sample_if_send_signal( U32 ifindex, U32 sample_value, 
																int type, const char *signal_name )
{	
	syslog( LOG_INFO, "ac_sample_drop_rate_threshold_signal  type=%d\n", type);
	
	return ac_sample_dbus_send_signal( signal_name, 
										 DBUS_TYPE_INT32, &type,
										 DBUS_TYPE_UINT32, &ifindex,
										 DBUS_TYPE_UINT32, &sample_value,
										 DBUS_TYPE_INVALID );
}

static int drop_rate_do_statistics(ac_sample_t *me)
{
	if (NULL==me)
	{
		return AS_RTN_NULL_POINTER;
	}

	ac_sample_t *pDrop = get_ac_sample_by_name(SAMPLE_NAME_DROPRATE);
	
	int sample_status = get_sample_state( pDrop );
	const char *name  = get_sample_name( pDrop );	
	if( SAMPLE_ON != sample_status )
	{
		syslog( LOG_DEBUG, "sample %s is not on!", name );
		return AS_RTN_OK;
	}

	int threshold = get_sample_threshold( pDrop );
	int resend_interval = get_resend_interval( pDrop );
	syslog( LOG_DEBUG, "sample %s threshold =%d, drop_do_statistics\n", 
	    							name, threshold);

	struct if_flow_info_s *pos = NULL;
	struct list_head *iflist_head = NULL;
	iflist_head = (struct list_head *)get_sample_user_data( get_ac_sample_by_name(SAMPLE_NAME_INTERFACE_FLOW));

	list_for_each_entry( pos, iflist_head, node )
	{
		unsigned int time_now=0;
		unsigned int cur_status = NOT_OVER_THRESHOLD;
		U32 ifindex = pos->ifindex;
		U32 last_over_time = pos->drop_info.last_send_over_time;
		U32 last_status = pos->drop_info.last_status;
	    
	    if( threshold > 0 )
	    {
	    	U32 drop_rate = 0;

			if (0!=pos->packets &&
				(drop_rate=100*pos->drops/pos->packets) > threshold )
	    	{
				cur_status = OVER_THRESHOLD_FLAG;
			}
			
	        if( OVER_THRESHOLD_FLAG == cur_status )
	        {
	        	time_now = time(NULL);
				
	        	syslog( LOG_INFO, "%s  index=%d, check over threshold!  threshold: %d  latest:%d",
										name, ifindex, threshold, drop_rate);

				if( 0 == resend_interval )/*do not resend if resend signal inverval is 0*/
				{
					if( NOT_OVER_THRESHOLD == last_status )
					{
						ac_sample_if_send_signal(ifindex, drop_rate, cur_status, AC_SAMPLE_OVER_THRESHOLD_SIGNAL_DROP_RATE);
					}
					else
					{
						syslog( LOG_INFO, "%s index=%d, not resend signal because resend interval is 0", 
								name, pos->ifindex);
					}
				}
				else if( (time_now-last_over_time > resend_interval) )/*check signal resend time!*/
				{
					if( NOT_OVER_THRESHOLD == last_status)
					{
						syslog( LOG_INFO, "%s call callback to send signal!", name );
					}
					else
					{
						syslog( LOG_INFO, "%s call callback to resend signal!  resend!", name );
					}

					ac_sample_if_send_signal(ifindex, drop_rate, cur_status, AC_SAMPLE_OVER_THRESHOLD_SIGNAL_DROP_RATE);				
				}
				else
				{
					syslog( LOG_INFO, "%s is not on resend signal time! ", name );	
					syslog( LOG_INFO, "%s resend signal interval %d! last send time %d, now time %d, div %d", 
											name, 
											resend_interval,
											last_over_time,
											time_now,
											time_now-last_over_time);
				}

			}
			else /*check if need  send clear signal*/
			{
				if( OVER_THRESHOLD_FLAG == last_status)
				{
					syslog( LOG_INFO, "%s send trap clear signal, latest :%d", 
											name,  drop_rate );

					ac_sample_if_send_signal(ifindex, drop_rate, cur_status, AC_SAMPLE_OVER_THRESHOLD_SIGNAL_DROP_RATE);
				}
			}
	        
	    }
		else if( OVER_THRESHOLD_FLAG == last_status )/*user might set me->threshold to 0 to get clear!*/
		{
			syslog( LOG_INFO, "%s index=%d, send trap clear signal because threshold set to 0!!",
					name, pos->ifindex);

			ac_sample_if_send_signal(ifindex, 0, cur_status, AC_SAMPLE_OVER_THRESHOLD_SIGNAL_DROP_RATE);
		}	

		pos->drop_info.last_send_over_time = time_now;
		pos->drop_info.last_status 		   = cur_status;
	}
	
    return AS_RTN_OK;

}

int band_width_do_statistics(ac_sample_t *me)
{
	if (NULL==me)
	{
		return AS_RTN_NULL_POINTER;
	}

	ac_sample_t *pBand = get_ac_sample_by_name(SAMPLE_NAME_INTERFACE_FLOW);
	
	const char *name  = get_sample_name( pBand );
	int sample_status = get_sample_state(pBand);	
	if( SAMPLE_ON != sample_status )
	{
		syslog( LOG_DEBUG, "sample %s is not on!", name );
		return AS_RTN_OK;
	}

	int threshold = get_sample_threshold( pBand );
	int resend_interval = get_resend_interval(pBand);
	
	syslog( LOG_DEBUG, "sample %s threshold =%d, band_width_do_statistics\n", 
	    							name, threshold);

	struct if_flow_info_s *pos = NULL;
	struct list_head *iflist_head = NULL;
	iflist_head = (struct list_head *)get_sample_user_data( get_ac_sample_by_name(SAMPLE_NAME_INTERFACE_FLOW));

	list_for_each_entry( pos, iflist_head, node )
	{
		unsigned int time_now=0;
		unsigned int cur_status = NOT_OVER_THRESHOLD;
		unsigned int last_status = pos->band_info.last_status;
		unsigned int last_over_time = pos->band_info.last_send_over_time;
		unsigned int ifindex = pos->ifindex;
	    
	    if( threshold > 0 )
	    {
	    	U32 band_used = 100*pos->rxbandwidth/DEF_BAND_WIDTH;

			if ( band_used > threshold )
	    	{
				cur_status = OVER_THRESHOLD_FLAG;
			}
			
	        if( OVER_THRESHOLD_FLAG == cur_status )
	        {
	        	time_now = time(NULL);
				
	        	syslog( LOG_INFO, "%s  index=%d, check over threshold!  threshold: %d  latest:%d",
										name, ifindex, threshold, band_used);

				if( 0 == resend_interval )/*do not resend if resend signal inverval is 0*/
				{
					if( NOT_OVER_THRESHOLD == last_status )
					{
						ac_sample_if_send_signal(ifindex, band_used, cur_status, AC_SAMPLE_OVER_THRESHOLD_SIGNAL_BANDWITH);
					}
					else
					{
						syslog( LOG_INFO, "%s index=%d, not resend signal because resend interval is 0", 
								name, ifindex);
					}
				}
				else if( (time_now-last_over_time > resend_interval) )/*check signal resend time!*/
				{
					if( NOT_OVER_THRESHOLD == last_status )
					{
						syslog( LOG_INFO, "%s call callback to send signal!", name );
					}
					else
					{
						syslog( LOG_INFO, "%s call callback to resend signal!  resend!", name );
					}

					ac_sample_if_send_signal(ifindex, band_used, cur_status, AC_SAMPLE_OVER_THRESHOLD_SIGNAL_BANDWITH);
				}
				else
				{
					syslog( LOG_INFO, "%s is not on resend signal time! ", name );	
					syslog( LOG_INFO, "%s resend signal interval %d! last send time %d, now time %d, div %d", 
											name, 
											resend_interval,
											last_over_time,
											time_now,
											time_now-last_over_time);
				}

			}
			else /*check if need  send clear signal*/
			{
				if( OVER_THRESHOLD_FLAG == last_status)
				{
					syslog( LOG_INFO, "%s send trap clear signal, latest :%d", 
											name,  band_used );

					ac_sample_if_send_signal(ifindex, band_used, cur_status, AC_SAMPLE_OVER_THRESHOLD_SIGNAL_BANDWITH);
				}
			}
	        
	    }
		else if( OVER_THRESHOLD_FLAG == last_status )/*user might set me->threshold to 0 to get clear!*/
		{
			syslog( LOG_INFO, "%s index=%d, send trap clear signal because threshold set to 0!!",
					name, ifindex);

			ac_sample_if_send_signal(ifindex, 0, cur_status, AC_SAMPLE_OVER_THRESHOLD_SIGNAL_BANDWITH);
		}	

		pos->band_info.last_send_over_time = time_now;
		pos->band_info.last_status 		   = cur_status;
	}
	
    return AS_RTN_OK;

}

#if 0
static manage_message *
sample_get_slot_interface_info(unsigned int slot_id) {
	manage_message *query = NULL, *reply = NULL;
	manage_session *session = sample_get_tipc_session();
	if(NULL == session) {
		syslog(LOG_WARNING, "sample_get_slot_interface_info: get sample session fail\n");
		return NULL;
	}

	manage_tipc_addr_group addr_group;	
	memset(&addr_group, 0, sizeof(manage_tipc_addr_group));
	addr_group.dest.type = MANAGE_TIPC_TYPE;	/*tipc*/
	addr_group.dest.instance = 0x1000 + slot_id;		/*slot id*/
	memcpy(&(addr_group.sour), session->local, sizeof(manage_tipc_addr));
	
	query = manage_message_new(AC_MANAGE_TASK_METHOD_INTERFACE_INFO, 
									NULL, 0);
	syslog(LOG_DEBUG, "after manage_message_new: slot %d query = %p\n", slot_id, query);
	
	reply = manage_message_send_with_reply_and_block(session, query, 500, &addr_group, sizeof(manage_tipc_addr_group));
	syslog(LOG_DEBUG, "after manage_message_send_with_reply_and_block: slot %d reply = %p\n", slot_id, reply);

	MANAGE_FREE(query);
	return reply;
}

static struct if_stats_list *
sample_search_interface_stats(struct if_stats_list *if_array, unsigned int if_num, const char *ifname) {
	if(NULL == if_array || NULL == ifname) {
		return NULL;
	}
	
	int i = 0;
	for(; i < if_num; i++) {
		if(0 == strcmp(if_array[i].ifname, ifname)) {
			return &if_array[i];
		}
	}

	return NULL;
}


static void
sample_accumulate_interface_stats(struct if_stats_list *destNode, struct if_stats_list *sourNode) {
    if(NULL == destNode || NULL == sourNode) {
        return ;
    }

    destNode->stats.rx_packets           += sourNode->stats.rx_packets;
    destNode->stats.tx_packets           += sourNode->stats.tx_packets;
    destNode->stats.rx_bytes             += sourNode->stats.rx_bytes;
    destNode->stats.tx_bytes             += sourNode->stats.tx_bytes;
    destNode->stats.rx_errors            += sourNode->stats.rx_errors;
    destNode->stats.tx_errors            += sourNode->stats.tx_errors;
    destNode->stats.rx_dropped           += sourNode->stats.rx_dropped;
    destNode->stats.tx_dropped           += sourNode->stats.tx_dropped;
    destNode->stats.rx_multicast         += sourNode->stats.rx_multicast;
    destNode->stats.tx_multicast         += sourNode->stats.tx_multicast;
    destNode->stats.rx_compressed        += sourNode->stats.rx_compressed;
    destNode->stats.tx_compressed        += sourNode->stats.tx_compressed;
    destNode->stats.collisions           += sourNode->stats.collisions;

    
    /* detailed rx_errors: */
    destNode->stats.rx_length_errors     += sourNode->stats.rx_length_errors;
    destNode->stats.rx_over_errors       += sourNode->stats.rx_over_errors;
    destNode->stats.rx_crc_errors        += sourNode->stats.rx_crc_errors;
    destNode->stats.rx_frame_errors      += sourNode->stats.rx_frame_errors;
    destNode->stats.rx_fifo_errors       += sourNode->stats.rx_fifo_errors;
    destNode->stats.rx_missed_errors     += sourNode->stats.rx_missed_errors;

    
    /* detailed tx_errors */
    destNode->stats.tx_aborted_errors    += sourNode->stats.tx_aborted_errors;
    destNode->stats.tx_carrier_errors    += sourNode->stats.tx_carrier_errors;
    destNode->stats.tx_fifo_errors       += sourNode->stats.tx_fifo_errors;
    destNode->stats.tx_heartbeat_errors  += sourNode->stats.tx_heartbeat_errors;
    destNode->stats.tx_window_errors     += sourNode->stats.tx_window_errors;
    
    return ;
}

static int do_sample_interface_flow( ac_sample_t *me )
{
	unsigned int if_num = 0;
	struct if_stats_list *if_stats_array = NULL;

	int i, j;
	unsigned int local_slotid= 0, board_state = 0;

	manage_message *reply= NULL;
	manage_message *reply_array[SLOT_MAX_NUM];

	memset(reply_array, 0, sizeof(reply_array));

	if(VALID_DBM_FLAG == get_dbm_effective_flag())
	{
		local_slotid = sample_get_product_info(PRODUCT_LOCAL_SLOTID);
	}
	if(0 == local_slotid || local_slotid > SLOT_MAX_NUM) {
		syslog(LOG_WARNING, "do_sample_interface_flow: get local slot id failed!\n");
		return -1;
	}

	reply = sample_get_slot_interface_info(local_slotid);
	if(reply) {
		if_num = reply->data_length / sizeof(struct if_stats_list);
		if(reply->data_length == if_num * sizeof(struct if_stats_list)) {
			if_stats_array = (void *)reply + sizeof(manage_message);
			reply_array[local_slotid - 1] = reply;
		} else {
			syslog(LOG_WARNING, "the packet is error\n");
			MANAGE_FREE(reply);
			return -1;
		}
	} else {
		syslog(LOG_WARNING, "interface info get fail\n");
		return -1;
	}
	
	board_state = sample_get_board_state();

	for(i = 0; i < SLOT_MAX_NUM; i++) {
		if(((i + 1) != local_slotid) && (board_state & (0x1 << i))) {
			reply = sample_get_slot_interface_info(i + 1);
			if(reply) {
				unsigned int temp_num = reply->data_length / sizeof(struct if_stats_list);
				if(reply->data_length == temp_num * sizeof(struct if_stats_list)) {
					struct if_stats_list *temp_array = (void *)reply + sizeof(manage_message);
					for(j = 0; j < temp_num; j++) {
						struct if_stats_list *tempDest = sample_search_interface_stats(if_stats_array, if_num, temp_array[j].ifname);
						if(NULL == tempDest) {
							continue;
						}
						sample_accumulate_interface_stats(tempDest, &temp_array[j]);	
						syslog(LOG_DEBUG, "temp_array[%d].ifname = %s, tempDest->ifname = %s, tempDest->stats.tx_bytes = %llu", 
										j, temp_array[j].ifname, tempDest->ifname, tempDest->stats.tx_bytes);
					}

					reply_array[i] = reply;
				} else {
					syslog(LOG_WARNING, "the packet is error\n");
					MANAGE_FREE(reply);
					continue;
				}
			}
		}
	}

	for(i = 0; i < if_num; i++) {
		if_flow_info_set_sample_data(me, &if_stats_array[i]);
	}
	
	for(i = 0; i < SLOT_MAX_NUM; i++) {
		MANAGE_FREE(reply_array[i]);
	}
	
/*
	对每个接口进行丢包，带宽阀值检查，有必要发送trap消息
*/
	drop_rate_do_statistics(me);
	band_width_do_statistics(me);
	
	return 0;
}
#endif

static int do_sample_interface_flow( ac_sample_t *me )
{
	int if_num = 0;
	struct if_stats_list *if_stats_array = NULL;
	int ret = 0;

	int i, j;
	unsigned int local_slotid = 0, board_state = 0;
	unsigned int master_slotid = 0;

	if (VALID_DBM_FLAG == get_dbm_effective_flag()) {
		local_slotid = sample_get_product_info(PRODUCT_LOCAL_SLOTID);
		master_slotid = sample_get_product_info(PRODUCT_ACTIVE_MASTER);
	}
	
	if (0 == local_slotid || local_slotid > SLOT_MAX_NUM) {
		syslog(LOG_WARNING, "do_sample_interface_flow: get local slot id failed!\n");
		return -1;
	}
	
	syslog(LOG_DEBUG, "do_sample_interface_flow: slotid=%d, cmd is %d!\n", local_slotid, intf_flow_info.cmd);

	if (local_slotid != master_slotid && 
		INTERFACE_FLOW_STATISTICS_SAMPLING_INTEGRATED_ACSAMPLE == intf_flow_info.cmd) {
		return 0;
	}

	ret = sample_rtmd_get_interface_flow(&intf_flow_info, &if_num, &if_stats_array);
	if (0 != ret) {
		syslog(LOG_WARNING, "do_sample_interface_flow: sample_rtmd_get_interface_flow failed! ret = %d\n", ret);
		return ret;
	}

	for (i = 0; i < if_num; i++) {
		if_flow_info_set_sample_data(me, &if_stats_array[i]);
	}

	if (NULL != if_stats_array) {
		free(if_stats_array);
		if_stats_array = NULL;
	}	
/*
	对每个接口进行丢包，带宽阀值检查，有必要发送trap消息
*/
	drop_rate_do_statistics(me);
	band_width_do_statistics(me);
	
	return 0;
}


ac_sample_t *create_ac_sample_interface_flow(  
                                unsigned int sample_interval, 
                                unsigned int statistics_time )
{
	static struct list_head iflist_head = LIST_HEAD_INIT(iflist_head);

	ac_sample_t *pret = NULL;
	
	acsample_conf config;
	int ret=-1;

	memset(&config, 0, sizeof(config));
	set_default_config( &config );
	if ((ret=acsample_read_conf( &config, SAMPLE_NAME_INTERFACE_FLOW ) ))
	{
		syslog(LOG_INFO,"%s acsample_read_conf:error return %d load default config", SAMPLE_NAME_INTERFACE_FLOW, ret );
	}

	ret = sample_rtmd_init_socket(&intf_flow_info, PROCESS_NAME_ACSAMPLE,
			INTERFACE_FLOW_STATISTICS_SAMPLING_DIVIDED_ACSAMPLE, 1000);
	if (0 != ret) {
		syslog(LOG_INFO,"%s sample_rtmd_init_socket:error return %d init socket", SAMPLE_NAME_INTERFACE_FLOW, ret);
	}
	
	syslog(LOG_INFO,"%s sample_rtmd_init_socket:return %d init socket", SAMPLE_NAME_INTERFACE_FLOW, ret);
    pret = create_ac_sample(SAMPLE_NAME_INTERFACE_FLOW, config.sample_interval, config.statistics_time, config.resend_interval );//, do_sample_memusage, NULL );

	if( NULL != pret )
	{
		set_do_sample_cb( pret, do_sample_interface_flow );
		set_sample_state( pret, config.sample_on );
		
		set_sample_threshold( pret, config.threshold);
		set_over_threshold_cb( pret, NULL );

		//ac_sample_t *me, void *user_data, USER_DATA_FREE free_cb
		set_sample_user_data( pret, &iflist_head, destroy_iface_user_data );
	}

	syslog( LOG_DEBUG, "create_ac_sample_interface_flow  %p", pret );
	
create_return:
	syslog(LOG_INFO,"config: sample_on=%d, statistics_time=%d, sample_interval=%d, resend_interval=%d, reload_config_interval=%d, over_threshold_check_type=%d, clear_check_type=%d, threshold=%d",
							config.sample_on, config.statistics_time, config.sample_interval, config.resend_interval,\
							config.reload_config_interval, config.over_threshold_check_type, config.clear_check_type, config.threshold );
	return pret;
}


