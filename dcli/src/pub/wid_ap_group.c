#ifdef _D_WCPSS_

#include <string.h>
#include <dbus/dbus.h>
#include "wcpss/waw.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/ACDbusDef1.h"
#include "dbus/asd/ASDDbusDef1.h"
#include "wcpss/asd/asd.h"
#include "wid_ac.h"
#include "wid_wtp.h"
#include "dbus/wcpss/dcli_wid_wtp.h"

int dcli_ap_group_add_del_member(int localid,int index,unsigned int groupid,int isadd,struct tag_wtpid_list * wtplist,unsigned int **wtp_list,unsigned int *apcount,DBusConnection *dcli_dbus_connection){
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	int i;
	struct tag_wtpid *tmp = NULL;
	unsigned int num;	
	unsigned int count;
	int ret;
	unsigned int wtpid = 0;
	dbus_error_init(&err);

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_AP_GROUP_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_AP_GROUP_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_AP_GROUP_METHOD_ADD_DEL_MEMBER);

	num = wtplist->count;
	dbus_message_iter_init_append (query, &iter);
			

	dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &isadd);
	dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &groupid);
		// Total slot count
	dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &num);
	tmp = wtplist->wtpidlist;
	for(i = 0; i < num; i++){
		dbus_message_iter_append_basic (&iter,
											 DBUS_TYPE_UINT32,
											 &(tmp->wtpid));
		tmp = tmp->next;

	}	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);


	if (NULL == reply) {
		printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return -1;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&count);
	*apcount = count;
	if(count != 0){
		(*wtp_list) = (unsigned int *)malloc(count*(sizeof(unsigned int)));
	}
	for(i=0; i < count; i++){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&wtpid);
		(*wtp_list)[i] = wtpid;
	}
	dbus_message_unref(reply);	

	destroy_input_wtp_list(wtplist);
	return ret;
}
//zhangshu add , 2010-09-16
int dcli_ap_group_show_member(int localid,int index,unsigned int groupid,unsigned int **wtp_list,unsigned int *apcount,DBusConnection *dcli_dbus_connection){
    DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;

	int i = 0;	
	unsigned int count = 0;
	int ret = 0;
	unsigned int wtpid = 0;
	dbus_error_init(&err);

	printf("11111111111111111111111111\n");

    char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_AP_GROUP_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_AP_GROUP_INTERFACE,INTERFACE);

	printf("2222222222222222222222\n");
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_AP_GROUP_METHOD_SHOW_MEMBER);

    printf("3333333333333333333333333\n");
    dbus_message_iter_init_append (query, &iter);
			
	dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &groupid);

    reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return -1;
	}
    printf("44444444444444444444444\n");
    dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&count);
	printf("5555555555555555555555555\n");
	printf("count = %d\n",count);
	*apcount = count;
	if(count != 0){
		(*wtp_list) = (unsigned int *)malloc(count*(sizeof(unsigned int)));
	}
	for(i=0; i < count; i++){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&wtpid);
		(*wtp_list)[i] = wtpid;
		printf("wtpid[%d] = %d\n",i,wtpid);
	}
	printf("66666666666666666666666666\n");
	dbus_message_unref(reply);
    
    return ret;
}

#endif
