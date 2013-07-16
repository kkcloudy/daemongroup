#include <dbus/dbus.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpathInternals.h>

#include "ws_eag_conf.h"
#include "ws_eag_auto_conf.h"

int ccgi_dbus_eag_conf_nas(DBusConnection *conn, dbus_nas_conf nas_conf)
{
	DBusMessage *query, *reply;
    DBusError err;
    int ret_nas = -1;

    query = dbus_message_new_method_call(
                                    EAG_AURO_SERVER,
                                    EAG_AURO_CONF_OBJECT,
						            INTERFACE_AURO_NAS, 
						            METHOD_NAS );
	if (NULL == query) {
      fprintf(stderr, "ccgi_dbus_eag_conf_nas:Message Null\n");
    }
	
    dbus_error_init(&err);

	const char * tmp_n_nastype = nas_conf.n_nastype;
	const char * tmp_n_start = nas_conf.n_start;
	const char * tmp_n_end = nas_conf.n_end;
	
	const char * tmp_n_nasid = nas_conf.n_nasid;
	const char * tmp_n_syntaxis = nas_conf.n_syntaxis;
	const char * tmp_n_attz = nas_conf.n_attz;
	//fprintf(stderr,"eag_auto_conf n_flag=%d,n_default=%d,n_plotid=%u,n_nastype=%s,n_start=%s,n_end=%s,n_nasid=%u,n_syntaxis=%u",\
	//			nas_conf.n_flag,nas_conf.n_default,nas_conf.n_plotid,tmp_n_nastype,tmp_n_start,tmp_n_end,nas_conf.n_nasid,nas_conf.n_syntaxis);
	
	dbus_message_append_args(   query,
								DBUS_TYPE_INT32,  &nas_conf.n_flag,
								DBUS_TYPE_INT32,  &nas_conf.n_default,
								DBUS_TYPE_INT32,  &nas_conf.n_plotid,
								DBUS_TYPE_STRING, &tmp_n_nastype,
								DBUS_TYPE_STRING, &tmp_n_start,
								DBUS_TYPE_STRING, &tmp_n_end,
								DBUS_TYPE_STRING, &tmp_n_nasid,
								DBUS_TYPE_STRING, &tmp_n_syntaxis,
								DBUS_TYPE_STRING, &tmp_n_attz,
					            DBUS_TYPE_INVALID );
	
    reply = dbus_connection_send_with_reply_and_block ( conn, query, -1, &err );
    dbus_message_unref(query);
    
    if ( NULL == reply )
    {
        if (dbus_error_is_set(&err))
        {
        	fprintf(stderr, "ccgi_dbus_eag_conf_nas:send_with_reply_and_block Error:(%s)\n", err.message);
            dbus_error_free(&err);
        }
        ret_nas = ERR_DBUS_REPLY_NULL;
    }
    else
    {
        dbus_message_get_args( reply,
                                &err,
                                DBUS_TYPE_INT32, &ret_nas,
                                DBUS_TYPE_INVALID );
		dbus_message_unref(reply);
    }       
    
	return ret_nas;
}

int ccgi_dbus_eag_conf_vlanmaping(DBusConnection *conn, dbus_vlan_conf vlan_conf)
{
	DBusMessage *query, *reply;
    DBusError err;
    int ret_vlan = -1;

    query = dbus_message_new_method_call(
                                    EAG_AURO_SERVER,
                                    EAG_AURO_CONF_OBJECT,
						            INTERFACE_AURO_VLANMAPING, 
						            METHOD_VLANMAPING );
	if (NULL == query) {
      fprintf(stderr, "dbus_eag_conf_vlanmaping:dbus_message_new_method_call==Null\n");
    }

    dbus_error_init(&err);

	//fprintf(stderr,"eag_auto_conf v_wlan_begin_id=%d,v_wlan_end_id=%d,v_wtp_begin_id=%u,v_wtp_end_id=%u",\
	//			vlan_conf.v_wlan_begin_id,vlan_conf.v_wlan_end_id,vlan_conf.v_wtp_begin_id,vlan_conf.v_wtp_end_id);
	
	const char * tmp_wlan_begin_id = vlan_conf.v_wlan_begin_id;
	const char * tmp_wlan_end_id = vlan_conf.v_wlan_end_id;
	const char * tmp_wtp_begin_id = vlan_conf.v_wtp_begin_id;
	const char * tmp_wtp_end_id = vlan_conf.v_wtp_end_id;
	const char * nasportid = vlan_conf.nasportid;
	const char * tmp_attz = vlan_conf.v_attz;
	
	dbus_message_append_args(   query,
								DBUS_TYPE_INT32,  &vlan_conf.v_flag,
								DBUS_TYPE_INT32,  &vlan_conf.v_strategy_id,
								DBUS_TYPE_STRING, &tmp_wlan_begin_id,
								DBUS_TYPE_STRING, &tmp_wlan_end_id,
								DBUS_TYPE_STRING, &tmp_wtp_begin_id,
								DBUS_TYPE_STRING, &tmp_wtp_end_id,
								DBUS_TYPE_STRING, &nasportid,
								DBUS_TYPE_STRING, &tmp_attz,
					            DBUS_TYPE_INVALID );
	
    reply = dbus_connection_send_with_reply_and_block ( conn, query, -1, &err );
    dbus_message_unref(query);
    
    if ( NULL == reply )
    {
        if (dbus_error_is_set(&err))
        {
        	//fprintf(stderr, "dbus_message_new_method_call Error:%s\n", err.message);
            dbus_error_free(&err);
        }
        ret_vlan = ERR_DBUS_REPLY_NULL;
    }
    else
    {
        dbus_message_get_args( reply,
                                &err,
                                DBUS_TYPE_INT32, &ret_vlan,
                                DBUS_TYPE_INVALID );
		dbus_message_unref(reply);
    }       
    
	return ret_vlan;
}

int ccgi_dbus_eag_conf_vlan_nasportid_map(DBusConnection *conn, dbus_vlan_nasportid_conf vlan_nasportid_conf)
{
	DBusMessage *query, *reply;
	DBusError err;
	int ret_vlan = -1;

	query = dbus_message_new_method_call(
									EAG_AURO_SERVER,
									EAG_AURO_CONF_OBJECT,
									INTERFACE_AURO_VLANMAPING, 
									METHOD_VLAN_NASPORTID );
	if (NULL == query) 
	{
	  	fprintf(stderr, "dbus_eag_conf_vlan_nasportid_map:dbus_message_new_method_call==Null\n");
	}

	dbus_error_init(&err);

	fprintf(stderr,"eag_auto_conf vlan_begin_id=%s,vlan_end_id=%s,nasportid=%s",\
				vlan_nasportid_conf.vlan_begin_id,vlan_nasportid_conf.vlan_end_id,vlan_nasportid_conf.nasportid);
	
	const char * tmp_vlan_begin_id = vlan_nasportid_conf.vlan_begin_id;
	const char * tmp_vlan_end_id = vlan_nasportid_conf.vlan_end_id;
	const char * tmp_nasportid = vlan_nasportid_conf.nasportid;
	const char * tmp_attz = vlan_nasportid_conf.n_attz;
	
	dbus_message_append_args(   query,
								DBUS_TYPE_INT32,  &vlan_nasportid_conf.n_flag,
								DBUS_TYPE_INT32,  &vlan_nasportid_conf.n_strategy_id,
								DBUS_TYPE_STRING, &tmp_vlan_begin_id,
								DBUS_TYPE_STRING, &tmp_vlan_end_id,
								DBUS_TYPE_STRING, &tmp_nasportid,
								DBUS_TYPE_STRING, &tmp_attz,
					           		 DBUS_TYPE_INVALID );
	
    reply = dbus_connection_send_with_reply_and_block ( conn, query, -1, &err );
    dbus_message_unref(query);
    
    if ( NULL == reply )
    {
        if (dbus_error_is_set(&err))
        {
        	//fprintf(stderr, "dbus_message_new_method_call Error:%s\n", err.message);
            dbus_error_free(&err);
        }
        ret_vlan = ERR_DBUS_REPLY_NULL;
    }
    else
    {
        dbus_message_get_args( reply,
                                &err,
                                DBUS_TYPE_INT32, &ret_vlan,
                                DBUS_TYPE_INVALID );
		dbus_message_unref(reply);
    }       
    
	return ret_vlan;
}

int ccgi_dbus_eag_conf_debug_filter(DBusConnection *conn, 
										dbus_debug_filter_conf debug_filter_conf)
{
	DBusMessage *query, *reply;
	DBusError err;
	int ret_debug_filter = -1;

	query = dbus_message_new_method_call(
							EAG_AURO_SERVER,
							EAG_AURO_CONF_OBJECT,
							INTERFACE_AURO_DEBUG_FILTER, 
							METHOD_DEBUG_FILTER );
	if (NULL == query) {
	  	fprintf(stderr, "ccgi_dbus_eag_conf_debug_filter:Message Null\n");
	}

	dbus_error_init(&err);

	const char * tmp_debug_value = debug_filter_conf.value;

	dbus_message_append_args(   query,
								DBUS_TYPE_INT32,  &debug_filter_conf.status,
								DBUS_TYPE_INT32,  &debug_filter_conf.strategy_id,
								DBUS_TYPE_INT32, &debug_filter_conf.key,
								DBUS_TYPE_STRING, &tmp_debug_value,
								DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block ( conn, query, -1, &err );
	dbus_message_unref(query);

	if ( NULL == reply )
	{
	    if (dbus_error_is_set(&err))
	    {
	    	fprintf(stderr, "ccgi_dbus_eag_conf_debug_filter:send_with_reply_and_block Error:(%s)\n", err.message);
	        dbus_error_free(&err);
	    }
	    ret_debug_filter = ERR_DBUS_REPLY_NULL;
	}
	else
	{
		dbus_message_get_args( reply,
	                            &err,
	                            DBUS_TYPE_INT32, &ret_debug_filter,
	                            DBUS_TYPE_INVALID );
		dbus_message_unref(reply);
	}       

	return ret_debug_filter;
}

int ccgi_dbus_eag_show_debug_filter(DBusConnection *conn,
										int strategy_id,
										dbus_debug_filter_list *debug_filter_list)
{
	DBusMessage *query, *reply;
	DBusError err;
	int ret_debug_filter = -1;
	char *filter_list = NULL;
	
	query = dbus_message_new_method_call(
							EAG_AURO_SERVER,
							EAG_AURO_CONF_OBJECT,
							INTERFACE_AURO_DEBUG_FILTER_SHOW, 
							METHOD_DEBUG_FILTERS_SHOW );
	if (NULL == query) {
	  	fprintf(stderr, "ccgi_dbus_eag_show_debug_filter:Message Null\n");
	}

	dbus_error_init(&err);
	dbus_message_append_args(   query,
								DBUS_TYPE_INT32,  &strategy_id,
								DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block ( conn, query, -1, &err );
	dbus_message_unref(query);

	if ( NULL == reply )
	{
	    if (dbus_error_is_set(&err))
	    {
	    	fprintf(stderr, "ccgi_dbus_eag_show_debug_filter:send_with_reply_and_block Error:(%s)\n", err.message);
	        dbus_error_free(&err);
	    }
	    ret_debug_filter = ERR_DBUS_REPLY_NULL;
	}
	else
	{
		dbus_message_get_args( reply,
	                            &err,
	                            DBUS_TYPE_INT32, &ret_debug_filter,
	                            DBUS_TYPE_STRING, &filter_list,
	                            DBUS_TYPE_INVALID );
		dbus_message_unref(reply);

		snprintf(debug_filter_list->list,sizeof(debug_filter_list->list)-1,"%s",filter_list);
		//fprintf(stderr,"ccgi_dbus_eag_show_debug_filter:debug_filter_list=%s\n",debug_filter_list->list);
	}       

	return ret_debug_filter;
}


