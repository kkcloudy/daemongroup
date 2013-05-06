#ifdef __cplusplus
extern "C"
{
#endif

#include "ws_dcli_ac_roaming.h"

/*dcli_ac_roaming.c V1.6*/
/*author qiaojie*/
/*update time 10-04-01*/

int show_inter_ac_roaming_count_cmd(dbus_parameter parameter, DBusConnection *connection, struct roaming_count_profile *count_info)/*返回0表示失败，返回1表示成功*/
																																			/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
    if(NULL == connection)
        return 0;
	
	if(NULL == count_info)
		return 0;
	
	int ret;
	int in;
	int out;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,ASD_DBUS_BUSNAME,BUSNAME);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,ASD_DBUS_AC_GROUP_OBJPATH,OBJPATH);
	ccgi_ReInitDbusPath_v2(parameter.local_id, parameter.instance_id,ASD_DBUS_AC_GROUP_INTERFACE,INTERFACE);

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_AC_GROUP_METHOD_ROAMING_COUNT);
	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return SNMPD_CONNECTION_ERROR;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&in);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&out);

	/*vty_out(vty,"Inter AC Roaming total Count:	%d\n",ret); 
	vty_out(vty,"Inter AC Roaming in Count:  %d\n",in);
	vty_out(vty,"Inter AC Roaming out Count:  %d\n",out);*/
	count_info->total_count = ret;
	count_info->in_count = in;
	count_info->out_count = out;
	
	dbus_message_unref(reply);
	return 1;
}




#ifdef __cplusplus
}
#endif

