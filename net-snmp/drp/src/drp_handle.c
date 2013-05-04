/* drp_handle.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dbus/dbus.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <syslog.h>

#include "nm_list.h"
#include "drp_def.h"
#include "drp_log.h"
#include "drp_mem.h"
#include "drp_util.h"
#include "drp_interface.h"
#include "drp_dbus.h"
#include "drp_opxml.h"
#include "drp_handle.h"


DBusMessage *
drp_dbus_method_add_domain( DBusConnection *conn, 
		DBusMessage *msg, 
		void *user_data )
{
	DBusMessage* reply = NULL;
	DBusMessageIter iter = {0};
	DBusError		err = {0};
	int ret = -1;
	char *domain_name = NULL;
	char command[256] = {0};
	int status = 0;

	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		drp_log_err("drp_dbus_method_add_domain "\
				"DBUS new reply message error!\n");
		return NULL;
	}

	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg ,&err,
					DBUS_TYPE_STRING, &domain_name,
					DBUS_TYPE_INVALID))){
		drp_log_err("drp_dbus_method_add_domain "\
				"unable to get input args\n");
		if (dbus_error_is_set(&err)) {
			drp_log_err("drp_dbus_method_add_domain %s raised:%s\n",
					err.name, err.message);
			dbus_error_free(&err);
		}
		ret = DRP_ERR_DBUS_FAILED;
		goto replyx;
	}	

	drp_log_info("drp_dbus_method_add_domain domain_name is %s",domain_name);

	if(access(XML_DOMAIN_PATH ,0)!=0)
	{
		memset(command,0, sizeof(command));
		sprintf(command,"sudo cp %s  %s",XML_DOMAIN_D,XML_DOMAIN_PATH);
		system(command);			
		memset(command,0, sizeof(command));
		sprintf(command,"sudo chmod 666 %s",XML_DOMAIN_PATH);
		system(command);
	}

	memset(command,0,sizeof(command));
	snprintf(command, sizeof(command)-1, DNS_RESOLVE_DOMAIN_TO_XML, domain_name);
	status = system(command);
	ret = WEXITSTATUS(status);
	if (0 != ret  )
	{
		switch (ret){
			case 1:
				ret  = DRP_ERR_DNS_RESOLVE_PARAM_INPUT;
				break;
			case 2:
				ret  = DRP_ERR_DNS_RESOLVE_DOMAIN_EXIST;
				break;
			case 3:
				ret = DRP_ERR_DNS_SERVER_NOT_CONFIGED;
				break;
			case 4:
				ret = DRP_ERR_DNS_RESOLVE_FAILED;
				break;
			case 5:
				ret = DRP_ERR_DNS_RESULT_WRITE_XML_FAILED;
				break;
			case 6:
				ret = DRP_ERR_DNS_RESULT_XML_FORMAT_WRIONG;
				break;
			default:
				break;
		}
		goto replyx;
	}

replyx:
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
			DBUS_TYPE_INT32, &ret);
	return reply;
}

DBusMessage *
drp_dbus_method_del_domain( DBusConnection *conn, 
		DBusMessage *msg, 
		void *user_data )
{
	DBusMessage* reply = NULL;
	DBusMessageIter iter = {0};
	DBusError		err = {0};
	int ret = -1;
	char *domain_name = NULL;
	
	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		drp_log_err("drp_dbus_method_del_domain "\
				"DBUS new reply message error!\n");
		return NULL;
	}

	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg ,&err,
					DBUS_TYPE_STRING, &domain_name,
					DBUS_TYPE_INVALID))){
		drp_log_err("drp_dbus_method_del_domain "\
				"unable to get input args\n");
		if (dbus_error_is_set(&err)) {
			drp_log_err("drp_dbus_method_del_domain %s raised:%s\n",
					err.name, err.message);
			dbus_error_free(&err);
		}
		ret = DRP_ERR_DBUS_FAILED;
		goto replyx;
	}	

	drp_log_info("drp_dbus_method_del_domain domain_name is %s",domain_name);

	ret = delete_domain_second_xmlnode(XML_DOMAIN_PATH,"domain","attribute", domain_name);

replyx:
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
			DBUS_TYPE_INT32, &ret);
	return reply;
}


DBusMessage *
drp_dbus_method_add_domain_ip( DBusConnection *conn, 
		DBusMessage *msg, 
		void *user_data )
{
	DBusMessage* reply = NULL;
	DBusMessageIter iter = {0};
	DBusError		err = {0};
	int ret = -1;
	char *domain_name = NULL;
	int ip_index = 0;
	unsigned int ip_addr = 0;
	char ip_name[10] = {0};
	char ip_str[32] = {0};
	char command[256] = {0};

	memset (&ip_name, 0, sizeof(ip_name));
	memset (&ip_str, 0, sizeof(ip_str));
	memset (&command, 0, sizeof(command));

	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		drp_log_err("drp_dbus_method_add_domain_ip "\
				"DBUS new reply message error!\n");
		return NULL;
	}

	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg ,&err,
					DBUS_TYPE_STRING, &domain_name,
					DBUS_TYPE_INT32, &ip_index,
					DBUS_TYPE_UINT32, &ip_addr, 
					DBUS_TYPE_INVALID))){
		drp_log_err("drp_dbus_method_add_domain_ip "\
				"unable to get input args\n");
		if (dbus_error_is_set(&err)) {
			drp_log_err("drp_dbus_method_add_domain_ip %s raised:%s\n",
					err.name, err.message);
			dbus_error_free(&err);
		}
		ret = DRP_ERR_DBUS_FAILED;
		goto replyx;
	}	
	
	drp_log_info("drp_dbus_method_add_domain_ip domain_name"
			" is %s index is %d ip_addr is %u",domain_name,ip_index,ip_addr);

	if(access(XML_DOMAIN_PATH ,0)!=0)
	{
		memset(command,0, sizeof(command));
		sprintf(command,"sudo cp %s  %s",XML_DOMAIN_D,XML_DOMAIN_PATH);
		system(command);			
		memset(command,0, sizeof(command));
		sprintf(command,"sudo chmod 666 %s",XML_DOMAIN_PATH);
		system(command);
	}

	add_domain_second_node_attr(XML_DOMAIN_PATH,"domain","attribute", domain_name);
	snprintf(ip_name,sizeof(ip_name), "ip%d",ip_index);
	inet_int2str( ip_addr, ip_str, sizeof(ip_str));
	ret = add_domain_third_node(XML_DOMAIN_PATH,"domain","attribute",domain_name, ip_name, ip_str);

replyx:
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
			DBUS_TYPE_INT32, &ret);
	return reply;

}

DBusMessage *
drp_dbus_method_get_domain_ip( DBusConnection *conn, 
		DBusMessage *msg, 
		void *user_data )
{
	DBusMessage* reply = NULL;
	DBusMessageIter iter = {0};
	DBusError		err = {0};
	int ret = -1;
	char *domain_name = NULL;
	char command[256] = {0};
	int ip_num = 0;
	int count = 0;
	int free_domain_flag = 0;
	unsigned int ip_addr = 0;
	int status = 0;
	struct domain_st domain_head,*domain_cq=NULL;	

	memset (&domain_head,0,sizeof(domain_head));
	memset (command, 0, sizeof(command));

	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		drp_log_err("drp_dbus_method_get_domain_ip "\
				"DBUS new reply message error!\n");
		return NULL;
	}

	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg ,&err,
					DBUS_TYPE_STRING, &domain_name,
					DBUS_TYPE_INVALID))){
		drp_log_err("drp_dbus_method_get_domain_ip "\
				"unable to get input args\n");
		if (dbus_error_is_set(&err)) {
			drp_log_err("drp_dbus_method_get_domain_ip %s raised:%s\n",
					err.name, err.message);
			dbus_error_free(&err);
		}
		ret = DRP_ERR_DBUS_FAILED;
		goto replyx;
	}	

	drp_log_info("drp_dbus_method_get_domain_ip domain_name %s" ,domain_name);

	if(access(XML_DOMAIN_PATH ,0)!=0)
	{
		memset(command,0, sizeof(command));
		sprintf(command,"sudo cp %s  %s",XML_DOMAIN_D,XML_DOMAIN_PATH);
		system(command);			
		memset(command,0, sizeof(command));
		sprintf(command,"sudo chmod 666 %s",XML_DOMAIN_PATH);
		system(command);
	}

	memset(command,0,sizeof(command));
	snprintf(command, sizeof(command)-1, DNS_RESOLVE_DOMAIN_TO_XML, domain_name);
	status = system(command);
	ret = WEXITSTATUS(status);
	if (0 != ret && 2 != ret )
	{
		switch (ret){
			case 1:
				ret  = DRP_ERR_DNS_RESOLVE_PARAM_INPUT;
				break;
			case 2:
				ret  = DRP_ERR_DNS_RESOLVE_DOMAIN_EXIST;
				break;
			case 3:
				ret = DRP_ERR_DNS_SERVER_NOT_CONFIGED;
				break;
			case 4:
				ret = DRP_ERR_DNS_RESOLVE_FAILED;
				break;
			case 5:
				ret = DRP_ERR_DNS_RESULT_WRITE_XML_FAILED;
				break;
			case 6:
				ret = DRP_ERR_DNS_RESULT_XML_FORMAT_WRIONG;
				break;
			default:
				break;
		}
		goto replyx;
	}

	ret = check_and_get_domainname_xml(XML_DOMAIN_PATH,&domain_head,"domain",domain_name,&ip_num);
	if ( 0 != ret){
		drp_log_err("check_and_get_domainname_xml ret=%d", ret);
		goto replyx;
	}
	
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
					DBUS_TYPE_INT32, &ret);
	dbus_message_iter_append_basic(&iter,
		DBUS_TYPE_STRING, &domain_name );
	if( 0 == ip_num )
	{		
		drp_log_err("check_and_get_domainname_xml domain %s ip num is zero !",domain_name );
		dbus_message_iter_append_basic(&iter,
				DBUS_TYPE_INT32, &ip_num);
		goto error2;
	}
	else
	{
		if (ip_num > MAX_DOMAIN_IPADDR){
			ip_num = MAX_DOMAIN_IPADDR;
		}
		dbus_message_iter_append_basic(&iter,
				DBUS_TYPE_INT32, &ip_num);
		DBusMessageIter  iter_array;
		dbus_message_iter_open_container (&iter,
				DBUS_TYPE_ARRAY,
				DBUS_STRUCT_BEGIN_CHAR_AS_STRING
				DBUS_TYPE_INT32_AS_STRING	 //index
				DBUS_TYPE_UINT32_AS_STRING 	 //ip																		
				DBUS_STRUCT_END_CHAR_AS_STRING,
				&iter_array);

		for( count = 1,domain_cq=domain_head.next; \
				(domain_cq != NULL) && (count <= ip_num); 
				(domain_cq=domain_cq->next, count++))
		{
			DBusMessageIter iter_struct;
			dbus_message_iter_open_container (&iter_array,
					DBUS_TYPE_STRUCT,
					NULL,
					&iter_struct);

			if (drp_check_ip_format(domain_cq->domain_ip) != DRP_RETURN_OK){
				drp_log_err("DNS resolved ip num: %d address: %s format error!",\
						domain_cq->ip_index, domain_cq->domain_ip );
				free_read_domain_xml(&domain_head,&free_domain_flag);
				ret = DRP_ERR_DNS_RESOLVE_IP_FORMAT_WRONG;
				goto error1;
			}

			dbus_message_iter_append_basic(&iter_struct,
					DBUS_TYPE_INT32, &(domain_cq->ip_index));
			drp_inet_atoi(domain_cq->domain_ip,&ip_addr);
			dbus_message_iter_append_basic(&iter_struct,
					DBUS_TYPE_UINT32, &ip_addr);

			drp_log_debug ("domain %s: num=%d,ip_index=%d ip_addr_s=%s ip_addr_ul=%u.\n",\
					domain_name,count, domain_cq->ip_index,\
					domain_cq->domain_ip,ip_addr);	//for tests 					

			dbus_message_iter_close_container (&iter_array, &iter_struct);
		}
		dbus_message_iter_close_container (&iter, &iter_array);
	}

	free_read_domain_xml(&domain_head,&free_domain_flag);

error2:
	return reply;

error1:
	free_read_domain_xml(&domain_head,&free_domain_flag);
replyx:
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
			DBUS_TYPE_INT32, &ret);
	return reply;
}


DBusMessage *
drp_dbus_method_del_domain_ip( DBusConnection *conn, 
		DBusMessage *msg, 
		void *user_data )
{
	DBusMessage* reply = NULL;
	DBusMessageIter iter = {0};
	DBusError		err = {0};
	int ret = -1;
	char *domain_name = NULL;
	int ip_index = 0;
	char ip_name[10] = {0};
	char command[256] = {0};

	memset (command, 0, sizeof(command));
	memset (&ip_name, 0, sizeof(ip_name));

	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		drp_log_err("drp_dbus_method_del_domain_ip "\
				"DBUS new reply message error!\n");
		return NULL;
	}

	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg ,&err,
					DBUS_TYPE_STRING, &domain_name,
					DBUS_TYPE_INT32, &ip_index,
					DBUS_TYPE_INVALID))){
		drp_log_err("drp_dbus_method_del_domain_ip "\
				"unable to get input args\n");
		if (dbus_error_is_set(&err)) {
			drp_log_err("drp_dbus_method_del_domain_ip %s raised:%s\n",
					err.name, err.message);
			dbus_error_free(&err);
		}
		ret = DRP_ERR_DBUS_FAILED;
		goto replyx;
	}	

	drp_log_info("drp_dbus_method_del_domain_ip domain is %s index %d", domain_name, ip_index );

	if(access(XML_DOMAIN_PATH ,0)!=0)
	{
		memset(command,0, sizeof(command));
		sprintf(command,"sudo cp %s  %s",XML_DOMAIN_D,XML_DOMAIN_PATH);
		system(command);			
		memset(command,0, sizeof(command));
		sprintf(command,"sudo chmod 666 %s",XML_DOMAIN_PATH);
		system(command);
		ret = DRP_RETURN_OK;
		goto replyx;
	}

	snprintf(ip_name,sizeof(ip_name),"ip%d",ip_index);
	ret = delete_domain_third_xmlnodel(XML_DOMAIN_PATH,"domain","attribute",domain_name, ip_name);
	drp_log_debug ("delete_domain_third_xmlnodel domain name is %s ip index %s  ret = %d",domain_name, ip_name, ret);
	if ( ret == 1 ){
		ret = delete_domain_second_xmlnode(XML_DOMAIN_PATH,"domain","attribute", domain_name);
		drp_log_debug ("delete_domain_second_xmlnode domain name is %s ret = %d",domain_name,ret);
	}

replyx:
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
			DBUS_TYPE_INT32, &ret);
	return reply;
}

DBusMessage *
drp_dbus_method_show_domain_ip( DBusConnection *conn, 
		DBusMessage *msg, 
		void *user_data )
{
	DBusMessage* reply = NULL;
	DBusMessageIter iter = {0};
	DBusError		err = {0};
	int ret = -1;
	int count = 0;
	int free_domain_flag = 0;
	unsigned int ip_addr = 0;
	int domain_num = 0;
	int ip_num = 0;
	char *domain_name = NULL;
	int ip_index = 0;
	struct domain_st domain_head,*domain_cq=NULL;

	memset (&domain_head,0,sizeof(domain_head));
	drp_log_info("drp_dbus_method_show_domain_ip");

	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		drp_log_err("drp_dbus_method_get_domain_ip "\
				"DBUS new reply message error!\n");
		return NULL;
	}

	dbus_error_init(&err);
	dbus_message_iter_init_append(reply, &iter);

	if(access(XML_DOMAIN_PATH ,0)  != 0)
	{
		ret = DRP_ERR_OPXML_FILE_NOT_EXIST;
		goto replyx;
	}

	ret = get_domainname_ip_xml(XML_DOMAIN_PATH,&domain_head,"domain",&domain_num,&ip_num);
	if ( DRP_RETURN_OK != ret ){
		drp_log_err("get_domainname_ip_xml failed ret=%d", ret);
		goto replyx;
	}
	dbus_message_iter_append_basic(&iter,
				DBUS_TYPE_INT32, &ret);
	
	if( 0 == ip_num )
	{		
		drp_log_err("check_and_get_domainname_xml domain num is %d ip_num is %d !",domain_num,ip_num );
		dbus_message_iter_append_basic(&iter,
				DBUS_TYPE_INT32, &ip_num);
		goto error;
	}
	else
	{
		if (ip_num > MAX_DOMAIN_IPADDR*MAX_DOMAIN_CONFIG_NUM){
			ip_num = MAX_DOMAIN_IPADDR*MAX_DOMAIN_CONFIG_NUM;
		}
		dbus_message_iter_append_basic(&iter,
				DBUS_TYPE_INT32, &ip_num);
		
		DBusMessageIter  iter_array;
		dbus_message_iter_open_container (&iter,
				DBUS_TYPE_ARRAY,
				DBUS_STRUCT_BEGIN_CHAR_AS_STRING
					DBUS_TYPE_STRING_AS_STRING	//domain_name
					DBUS_TYPE_INT32_AS_STRING	 //index
					DBUS_TYPE_UINT32_AS_STRING 	 //ip																		
				DBUS_STRUCT_END_CHAR_AS_STRING,
				&iter_array);

		for( count = 0,domain_cq=domain_head.next; \
						(domain_cq != NULL) && (count < ip_num); 
						(domain_cq=domain_cq->next, count++))
		{
			DBusMessageIter iter_struct;
			dbus_message_iter_open_container (&iter_array,
					DBUS_TYPE_STRUCT,
					NULL,
					&iter_struct);
				if (drp_check_ip_format(domain_cq->domain_ip) != DRP_RETURN_OK){
					drp_log_err("DNS resolved ip index: %d address: %s format error!",\
							domain_cq->ip_index, domain_cq->domain_ip );
					ret = DRP_ERR_DNS_RESOLVE_IP_FORMAT_WRONG;
					goto error1;
				}
				domain_name = domain_cq->domain_name;
				dbus_message_iter_append_basic(&iter_struct,
						DBUS_TYPE_STRING, &domain_name);
				ip_index = domain_cq->ip_index;
				dbus_message_iter_append_basic(&iter_struct,
						DBUS_TYPE_INT32, &ip_index);
				drp_inet_atoi(domain_cq->domain_ip,&ip_addr);
				dbus_message_iter_append_basic(&iter_struct,
						DBUS_TYPE_UINT32, &ip_addr);

				drp_log_debug ("domain %s: num=%d,ip_index=%d ip_addr_s=%s ip_addr_ul=%u.\n",\
						domain_cq->domain_name,(count+1),domain_cq->ip_index,\
						domain_cq->domain_ip,ip_addr);	//for tests 					

				dbus_message_iter_close_container (&iter_array, &iter_struct);
		}
		dbus_message_iter_close_container (&iter, &iter_array);
	}


error1:
	free_read_domain_xml(&domain_head,&free_domain_flag);
replyx:
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
			DBUS_TYPE_INT32, &ret);
error:
	return reply;
}




