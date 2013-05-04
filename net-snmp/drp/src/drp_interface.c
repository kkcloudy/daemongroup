#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <dbus/dbus.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <syslog.h>
#include <errno.h>

#include "nm_list.h"
#include "drp_def.h"
#include "drp_interface.h"

#define MAX_DBUS_OBJPATH_LEN		128
#define MAX_DBUS_BUSNAME_LEN		128
#define MAX_DBUS_INTERFACE_LEN	128
#define MAX_DBUS_METHOD_LEN		128


/*base conf*/
#define DRP_DBUS_METHOD_ADD_DOMAIN			"drp_dbus_method_add_domain"
#define DRP_DBUS_METHOD_DEL_DOMAIN			"drp_dbus_method_del_domain"
#define DRP_DBUS_METHOD_GET_DOMAIN_IP		"drp_dbus_method_get_domain_ip"
#define DRP_DBUS_METHOD_ADD_DOMAIN_IP 		"drp_dbus_method_add_domain_ip"
#define DRP_DBUS_METHOD_DEL_DOMAIN_IP		"drp_dbus_method_del_domain_ip"
#define DRP_DBUS_METHOD_SHOW_DOMAIN_IP		"drp_dbus_method_show_domain_ip"
#define DRP_DBUS_METHOD_LOG_DEBUG			"drp_dbus_method_log_debug"


#define DISTRIBUTED_FILE    "/dbm/product/is_distributed"
#define ACTIVE_MASTER_SLOT_FILE		"/dbm/product/active_master_slot_id"
#define INSTANCE_MAX_NUM	16

#if 0
static int init_distributed_flag(void)
{
	int distributed_flag = 0;
	FILE *fp = NULL;    	
	fp = fopen(DISTRIBUTED_FILE, "r");
	if(NULL == fp) {
		syslog(LOG_DEBUG, "Open DISTRIBUTED_FILE error\n");
		return -1;
	}

	if(fscanf(fp, "%d", &distributed_flag) <= 0) {
		syslog(LOG_DEBUG, "Fscanf DISTRIBUTED_FILE error\n");
		return -2;
	}


	if(distributed_flag) {
		distributed_flag = 1;
		syslog(LOG_DEBUG, "The system is distributed\n");
	}else {
		syslog(LOG_DEBUG, "The system is unDistributed\n");
	}

	fclose(fp);
	return distributed_flag;
}

static int get_product_info(char *filename)
{
	int fd;
	char buff[16] = {0};
	unsigned int data;

	if(NULL == filename) {
		return -1;
	}

    fd = open(filename, O_RDONLY, 0);
    if(fd >= 0) {
        if(read(fd, buff, 16) < 0) {
            syslog(LOG_WARNING, "get_product_info: Read error : no value\n");
            close(fd);
            return 0;
        }    
    }
    else {        
        syslog(LOG_WARNING, "get_product_info: Open file:%s error!\n", filename);
        return 0;
    }
    
    data = strtoul(buff, NULL, 10);

    close(fd);
    
	return data;
}

static DBusConnection *dbus_get_tipc_connection(unsigned int slot_id,int local_slot_id,int hansitype, int insid)
{  
	if(0 == slot_id || slot_id > SLOT_MAX_NUM
		||0 == local_slot_id || local_slot_id > SLOT_MAX_NUM 
		||(0 !=hansitype ||1!=hansitype)
		||0 == insid || insid > INSTANCE_MAX_NUM)
	{
		syslog(LOG_DEBUG, "dbus_get_tipc_connection fun error: the slot_id is %d"
						" local_slot_id is %d hansitype is %d insid is %d\n",\
						slot_id,local_slot_id,hansitype,insid);
		return NULL;
	}
	
	DBusError dbus_error;
	DBusConnection *tempConnection = NULL;
	char temp_dbus_name[60] = { 0 };
	
	dbus_error_init (&dbus_error);
	
	tempConnection = dbus_bus_get_remote(DBUS_BUS_SYSTEM, slot_id, &dbus_error);
	if(NULL == tempConnection)
	{
		dbus_error_free(&dbus_error);
		addresses_shutdown_func();
		syslog(LOG_DEBUG, "The slot %d remote server is not ready.\n", slot_id);
		return NULL;
	}
	
	snprintf(temp_dbus_name, sizeof(temp_dbus_name) - 1, "aw.drp.slot%dhansitype%sinsid%d",\
							local_slot_id,(HANSI_LOCAL==hansitype)?"l":"r",insid);
	dbus_bus_request_name(tempConnection, temp_dbus_name, 0, &dbus_error);
	if (dbus_error_is_set(&dbus_error)) 
	{
		syslog(LOG_DEBUG, "request name failed: %s\n", dbus_error.message);
		dbus_error_free(&dbus_error);
		return NULL;
	}
	
	syslog(LOG_DEBUG, "The tipc connection %s of slot %d is OK\n", temp_dbus_name, slot_id);
	return tempConnection;
		
}
#endif

int conf_drp_set_dbus_connect_params(int slotid)
{
	int fd = 0;
	int ret = 0;
	char buf[1024] = {0};

	if ((fd = open (DRP_CONFIG_FILE,O_WRONLY|O_CREAT|O_TRUNC,S_IRUSR |S_IWUSR)) < 0){
		//printf ("open error:%s",strerror(errno));
		return -1;
	}
	
	if (snprintf(buf,sizeof(buf),"%d",slotid) < 0){
		//printf ("snprintf error:%s",strerror(errno));
		ret = -2;
		goto err;
	}

	if ((ret = write (fd,buf,strlen(buf))) < 0){
		//printf ("write error:%s",strerror(errno));
		ret = -3;
		goto err;
	}

err:
	close (fd);
	return ret;
}

//ret < 0 error else ret is read bytes count
int conf_drp_get_dbus_connect_params(int *slotid)
{
	int fd = 0;
	int ret = 0;
	char buf[1024] = {0};

	if ((fd = open(DRP_CONFIG_FILE, O_RDONLY)) < 0 ){
		if (errno == ENOENT) {
			//printf ("file not exist!\n");
			//file not exist use default config;
			return 0;
		}else{
			//printf ("file open error:%s\n",strerror(errno));
			return -1;
		}
	}
	if((ret = read (fd,buf,sizeof(buf))) < 0 ){
		//printf ("read error:%s\n",strerror(errno));
		ret = -2;
		goto err;
	}
	buf[ret] = '\0';

	if ((ret = sscanf (buf,"%d", slotid)) <= 0){
		//printf ("sscanf error(%d):%s\n",ret,ret==0?"no match":strerror(errno));
		ret = -3;
		goto err;
	}

	//printf ("file content is %s,distributeflag=%d,slotid=%d.\n",buf,distributeflag,slotid);
	
err:
	close(fd);
	return ret;
}


/*this function need domain_name param*/
int
conf_drp_add_domain(DBusConnection *connection,
		domain_pt *domain)
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=DRP_ERR_UNKNOWN;
	char *domain_name = NULL;
	
	query = dbus_message_new_method_call(
			DRP_DBUS_BUSNAME,
			DRP_DBUS_OBJPATH,
			DRP_DBUS_INTERFACE, 
			DRP_DBUS_METHOD_ADD_DOMAIN );
	dbus_error_init(&err);

	domain_name = domain->domain_name;
	dbus_message_append_args(	query,
			DBUS_TYPE_STRING,  &domain_name,
			DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block ( connection, query, -1, &err );

	dbus_message_unref(query);

	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return DRP_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args(	reply,
				&err,
				DBUS_TYPE_INT32, &iRet,
				DBUS_TYPE_INVALID );
	}

	dbus_message_unref(reply);

	return iRet;
}

/*this function need domain_name param*/
int 
conf_drp_del_domain(DBusConnection *connection,
		domain_pt *domain)
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=DRP_ERR_UNKNOWN;
	char *domain_name = NULL;
	
	query = dbus_message_new_method_call(
			DRP_DBUS_BUSNAME,
			DRP_DBUS_OBJPATH,
			DRP_DBUS_INTERFACE, 
			DRP_DBUS_METHOD_DEL_DOMAIN );
	dbus_error_init(&err);

	domain_name = domain->domain_name;
	dbus_message_append_args(	query,
			DBUS_TYPE_STRING,  &domain_name,
			DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block ( connection, query, -1, &err );

	dbus_message_unref(query);

	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return DRP_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args(	reply,
				&err,
				DBUS_TYPE_INT32, &iRet,
				DBUS_TYPE_INVALID );
	}

	dbus_message_unref(reply);

	return iRet;
}


/*this function need domain_name ip_index ip_addr params*/
	int
conf_drp_add_domain_ip(DBusConnection *connection,
		domain_pt *domain)
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=0;
	char *domain_name = NULL;

	query = dbus_message_new_method_call(
			DRP_DBUS_BUSNAME,
			DRP_DBUS_OBJPATH,
			DRP_DBUS_INTERFACE, 
			DRP_DBUS_METHOD_ADD_DOMAIN_IP );
	dbus_error_init(&err);

	domain_name = domain->domain_name;
	dbus_message_append_args(	query,
			DBUS_TYPE_STRING, &domain_name,
			DBUS_TYPE_INT32, &(domain->index),
			DBUS_TYPE_UINT32, &(domain->ipaddr),
			DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block (
			connection, query, -1, &err );

	dbus_message_unref(query);

	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return DRP_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args( reply,
				&err,
				DBUS_TYPE_INT32, &iRet,
				DBUS_TYPE_INVALID );
	}

	dbus_message_unref(reply);	
	return iRet;
}

	int
conf_drp_get_domain_ip(DBusConnection *remote_connection,
							domain_pt *domain, domain_ct *domain_ret)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	int iRet=0;
	int i = 0, num = 0;	
	char *domain_name = NULL;

	query = dbus_message_new_method_call(
			DRP_DBUS_BUSNAME,
			DRP_DBUS_OBJPATH,
			DRP_DBUS_INTERFACE, 
			DRP_DBUS_METHOD_GET_DOMAIN_IP );
	dbus_error_init(&err);
	domain_name = domain->domain_name;
	dbus_message_append_args( query,
			DBUS_TYPE_STRING, &domain_name,
			DBUS_TYPE_INVALID );
	
	reply = dbus_connection_send_with_reply_and_block (
										remote_connection, query, -1, &err );
	dbus_message_unref(query);

	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return DRP_ERR_DBUS_FAILED;
	}else{
		memset (domain_ret, 0, sizeof(domain_ret));
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter, &iRet);

		if( DRP_RETURN_OK == iRet ){
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter, &domain_name);
			strncpy((domain_ret->domain_name), domain_name,\
					sizeof(domain_ret->domain_name));
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter, &num);
			if ( 0 == num){
				domain_ret->num = num;
				return DRP_RETURN_OK;
			}

			if( num > MAX_DOMAIN_IPADDR ){
				num = MAX_DOMAIN_IPADDR;
			}
			domain_ret->num = num;
			if( num > 0 ){
				dbus_message_iter_next(&iter);	
				dbus_message_iter_recurse(&iter,&iter_array);			

				for( i=0; i<num; i++ ){
					DBusMessageIter iter_struct;
					dbus_message_iter_recurse(&iter_array,&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, &(domain_ret->domain_ip[i].index));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, &(domain_ret->domain_ip[i].ipaddr));
					dbus_message_iter_next(&iter_array);
				}
			}
		}
	}

	dbus_message_unref(reply);
	return iRet;
}


#if 0
	int
conf_drp_get_domain_ip(DBusConnection *connection,
							int local_slot_id, int hansitype, int insid, 
							domain_pt *domain, domain_ct *domain_ret)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	int iRet=0;
	int i = 0, num = 0;
	static int distributed = 0;
	static int active_master_slot_id = 0;
	int active_master_slot_id_tmp = 0;
	static DBusConnection *remote_dbus_conn = NULL;
	DBusConnection *remote_dbus_conn_tmp = NULL;
	DBusConnection *connection_real = NULL;
		

	query = dbus_message_new_method_call(
			DRP_DBUS_BUSNAME,
			DRP_DBUS_OBJPATH,
			DRP_DBUS_INTERFACE, 
			DRP_DBUS_METHOD_GET_DOMAIN_IP );
	dbus_error_init(&err);
	dbus_message_append_args( query,
			DBUS_TYPE_STRING, &(domain->domain_name),
			DBUS_TYPE_INVALID );

/*init remote dbus connection*/
	distributed = init_distributed_flag();
	if (distributed == 1){
		active_master_slot_id_tmp = get_product_info(ACTIVE_MASTER_SLOT_FILE);
		if (0 == active_master_slot_id || active_master_slot_id_tmp != active_master_slot_id){
			if (NULL == remote_dbus_conn){
				remote_dbus_conn_tmp = dbus_get_tipc_connection(active_master_slot_id_tmp,local_slot_id, hansitype, insid);
				if (NULL == remote_dbus_conn_tmp){
					syslog(LOG_ERR,"connect to drp dbus_get_tipc_connection error!");
					dbus_message_unref(query);
					iRet = DRP_ERR_REMOTE_DBUS_CONNECT_FAILED;
					goto error;
				}
			}else{
				dbus_connection_close(remote_dbus_conn);
				remote_dbus_conn = NULL;
				remote_dbus_conn_tmp = dbus_get_tipc_connection(active_master_slot_id_tmp,local_slot_id, hansitype, insid);
				if (NULL == remote_dbus_conn_tmp){
					syslog(LOG_ERR,"connect to drp dbus_get_tipc_connection error!");
					dbus_message_unref(query);
					iRet = DRP_ERR_REMOTE_DBUS_CONNECT_FAILED;
					goto error;
				}
			}
		}
		active_master_slot_id = active_master_slot_id_tmp;
		remote_dbus_conn = remote_dbus_conn_tmp;
		connection_real = remote_dbus_conn;
		syslog(LOG_INFO,"connect to drp remote active_master_slot_id is %d ",active_master_slot_id);
	}else if( 0 == distributed ){
		connection_real = connection;
	}else{
		syslog(LOG_ERR, "connect to drp error distributed state wrong! flag = %d \n use local dbus connection to drp!",distributed);
		connection_real = connection;
	}
	
	reply = dbus_connection_send_with_reply_and_block (
										connection_real, query, -1, &err );
	dbus_message_unref(query);

	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return DRP_ERR_DBUS_FAILED;
	}else{
		memset (domain_ret, 0, sizeof(domain_ret));
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter, &iRet);

		if( DRP_RETURN_OK == iRet ){
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter, &num);
			if( num > MAX_DOMAIN_IPADDR ){
				num = MAX_DOMAIN_IPADDR;
			}

			strncpy(domain_ret->domain_name, domain->domain_name,\
					sizeof(domain_ret->domain_name));
			domain_ret->num = num;

			if( num > 0 ){
				dbus_message_iter_next(&iter);	
				dbus_message_iter_recurse(&iter,&iter_array);			

				for( i=0; i<num; i++ ){
					DBusMessageIter iter_struct;
					dbus_message_iter_recurse(&iter_array,&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, &(domain_ret->domain_ip[i].index));
					dbus_message_iter_get_basic(&iter_struct, &(domain_ret->domain_ip[i].ipaddr));
					dbus_message_iter_next(&iter_array);
				}
			}
		}
	}


	dbus_message_unref(reply);
error:

	return iRet;
}
#endif

/*this function need domain_name ip_index params*/
	int
conf_drp_del_domain_ip(DBusConnection *connection,
		domain_pt *domain)
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=0;
	char *domain_name = NULL; 

	query = dbus_message_new_method_call(
			DRP_DBUS_BUSNAME,
			DRP_DBUS_OBJPATH,
			DRP_DBUS_INTERFACE, 
			DRP_DBUS_METHOD_DEL_DOMAIN_IP );
	dbus_error_init(&err);
	domain_name = domain->domain_name;
	dbus_message_append_args(	query,
			DBUS_TYPE_STRING, &domain_name,
			DBUS_TYPE_INT32, &(domain->index),
			DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block (
			connection, query, -1, &err );

	dbus_message_unref(query);

	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return DRP_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args( reply,
				&err,
				DBUS_TYPE_INT32, &iRet,
				DBUS_TYPE_INVALID );
	}

	dbus_message_unref(reply);	
	return iRet;
}

	int
conf_drp_show_domain_ip(DBusConnection *connection,
		domain_cst *domain_configs_ret)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	int iRet=0;
	int i = 0, ip_num = 0;
	char *domain_name = NULL;
	int index = 0;
	unsigned long ipaddr = 0;

	query = dbus_message_new_method_call(
			DRP_DBUS_BUSNAME,
			DRP_DBUS_OBJPATH,
			DRP_DBUS_INTERFACE, 
			DRP_DBUS_METHOD_SHOW_DOMAIN_IP );
	dbus_error_init(&err);
	dbus_message_append_args( query,
					DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block (
			connection, query, -1, &err );

	dbus_message_unref(query);
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return DRP_ERR_DBUS_FAILED;
	}else{
		dbus_message_iter_init(reply,&iter);
		//dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter, &iRet);
		if( DRP_RETURN_OK == iRet ){
			memset (domain_configs_ret, 0, sizeof(domain_configs_ret));
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter, &ip_num);
			if( ip_num > MAX_DOMAIN_CONFIG_NUM*MAX_DOMAIN_IPADDR ){
				ip_num = MAX_DOMAIN_CONFIG_NUM*MAX_DOMAIN_IPADDR;
			}
			domain_configs_ret->num = ip_num;
			if( ip_num > 0 ){
				dbus_message_iter_next(&iter);	
				dbus_message_iter_recurse(&iter,&iter_array);			
				for( i=0; i<ip_num; i++ ){
					DBusMessageIter iter_struct;
					dbus_message_iter_recurse(&iter_array,&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, &domain_name);
					strncpy((domain_configs_ret->domain[i].domain_name), domain_name,\
							sizeof(domain_configs_ret->domain[i].domain_name));
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,	\
							&(index));
					domain_configs_ret->domain[i].index = index;
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct, \
							&(ipaddr));
					domain_configs_ret->domain[i].ipaddr = ipaddr;
					dbus_message_iter_next(&iter_array);
#ifdef drp_test_interface	
					printf("domain %s ip index %d ipaddr %lu\n",\
							(domain_configs_ret->domain[i].domain_name),\
							(domain_configs_ret->domain[i].index),\
							(domain_configs_ret->domain[i].ipaddr));
#endif
				}
			}
		}
	}

	dbus_message_unref(reply);

	return iRet;
}

int
conf_drp_dbus_method_log_debug ( DBusConnection *conn, 
									int loglevel)
{
	DBusMessage *query, *reply;
	DBusError err;
	int iRet=DRP_ERR_UNKNOWN;
	query = dbus_message_new_method_call(
									DRP_DBUS_BUSNAME,
									DRP_DBUS_OBJPATH,
									DRP_DBUS_INTERFACE, 
									DRP_DBUS_METHOD_LOG_DEBUG );
	dbus_error_init(&err);
	
	dbus_message_append_args(	query,
								DBUS_TYPE_INT32,  &loglevel,
								DBUS_TYPE_INVALID );

	reply = dbus_connection_send_with_reply_and_block ( conn, query, -1, &err );

	dbus_message_unref(query);
	
	if ( NULL == reply ){	
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
		return DRP_ERR_DBUS_FAILED;
	}else{
		dbus_message_get_args(	reply,
								&err,
								DBUS_TYPE_INT32, &iRet,
								DBUS_TYPE_INVALID );
	}

	
	dbus_message_unref(reply);
	
	return iRet;
}


/*drp interface function test*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <dbus/dbus.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <syslog.h>

#include "nm_list.h"
#include "drp_def.h"
#include "drp_interface.h"

#define DRP_DBUS_TEST_NAME  "drp.testuse"

static char *
inet_ntoa_ex(unsigned long ip, char *ipstr, unsigned int buffsize)
{
	snprintf(ipstr, buffsize, "%lu.%lu.%lu.%lu", ip >> 24,
		 (ip & 0xff0000) >> 16, (ip & 0xff00) >> 8, (ip & 0xff));

	return ipstr;
}

DBusConnection *
drp_dbus_connection_init(void)
{
	int ret;
   	DBusError 	dbus_error;
	DBusConnection *conn = NULL;

	dbus_connection_set_change_sigpipe (TRUE);
	dbus_error_init (&dbus_error);

	conn = dbus_bus_get(DBUS_BUS_SYSTEM, 
						&dbus_error);

	if( NULL == conn  ){
		return NULL;
	}

	ret = dbus_bus_request_name( conn,DRP_DBUS_TEST_NAME, 0, &dbus_error );
	if( -1 == ret ){	
		printf("eag_dbus_new dbus_bus_request_name failed!");
		if ( dbus_error_is_set (&dbus_error) ){
			printf("eag_dbus_new request bus name %s with error %s", 
							"aw.eag.test", dbus_error.message);
	    }
		conn = NULL;
	}

	return conn;

}

int main(int argc , char **argv)
{
	int ret = 0;
	int i = 0;
	char ipstr[32] = {0};
	domain_pt domain_conf;
	DBusConnection *conn = NULL;
	domain_cst domain_ret;
	domain_ct domain_ctr;
	memset (&domain_ret,0,sizeof(domain_ret));
	memset (&domain_conf,0,sizeof(domain_conf));
	memset (&domain_ctr,0,sizeof(domain_ctr));

	printf("++++++++1\n");
	conn = drp_dbus_connection_init();
	printf("++++++++2\n");
	if ( NULL == conn){
		exit (-1);
	}

	memset (&domain_conf,0,sizeof(domain_conf));
	strncpy((domain_conf.domain_name),"www.google.com",sizeof(domain_conf.domain_name));
	memset (&domain_ctr,0,sizeof(domain_ctr));
	ret = conf_drp_get_domain_ip(conn,	&domain_conf, &domain_ctr);
	printf("conf_drp_get_domain_ip 1 ret = %d \n",ret);
	if ( 0 == ret){
		printf("domain %s ip num %d \n",domain_ctr.domain_name,domain_ctr.num);
		for (i = 0; i < domain_ctr.num ; i++){
			printf("domain ip index %d\n",domain_ctr.domain_ip[i].index);
			printf("domain ip addr %lu\n",domain_ctr.domain_ip[i].ipaddr);
		}
	}


	return 0;


//for show
	memset (&domain_ret,0,sizeof(domain_ret));
	ret = conf_drp_show_domain_ip(conn, &domain_ret);
	printf("conf_drp_show_domain_ip 1 ret = %d domain_ret.num =%d\n",ret,domain_ret.num);
	printf("================================================================================\n");
	if( domain_ret.num != 0 )
	{
		for ( i = 0;i < domain_ret.num; i++ ){
			printf("domain name: ");
			printf("%s\t",domain_ret.domain[i].domain_name);
			printf("domain IP : ");
			inet_ntoa_ex(domain_ret.domain[i].ipaddr, ipstr,sizeof(ipstr));
			printf("%s\t",ipstr);			        		
			printf("IP index for the domain name: ");
			printf("%d\n",domain_ret.domain[i].index);
	        }
	}
	printf("===============================================================================\n");
//end

	memset (&domain_conf,0,sizeof(domain_conf));
	strncpy((domain_conf.domain_name),"www.baidu.com",sizeof(domain_conf.domain_name));	
	ret = conf_drp_add_domain(conn, &domain_conf);
	printf("conf_drp_add_domain 1 ret = %d\n",ret);

//for show
	memset (&domain_ret,0,sizeof(domain_ret));
	ret = conf_drp_show_domain_ip(conn, &domain_ret);
	printf("conf_drp_show_domain_ip 2 ret = %d domain_ret.num =%d\n",ret,domain_ret.num);
	printf("================================================================================\n");
	if( domain_ret.num != 0 )
	{
		for ( i = 0;i < domain_ret.num; i++ ){
			printf("domain name: ");
			printf("%s\t",domain_ret.domain[i].domain_name);
			printf("domain IP : ");
			inet_ntoa_ex(domain_ret.domain[i].ipaddr, ipstr,sizeof(ipstr));
			printf("%s\t",ipstr);							
			printf("IP index for the domain name: ");
			printf("%d\n",domain_ret.domain[i].index);
			}
	}
	printf("===============================================================================\n");
//end

	memset (&domain_conf,0,sizeof(domain_conf));
	printf("++++++++++++1-0\n");
	strncpy((domain_conf.domain_name),"www.baidu.com",sizeof(domain_conf.domain_name));	
	domain_conf.ipaddr = 1234;
	domain_conf.index = 90;
	printf("++++++++++++1-1\n");
	ret = conf_drp_add_domain_ip(conn,&domain_conf);
	printf("++++++++++++1-2\n");
	printf("conf_drp_add_domain_ip 1 ret = %d\n",ret);
	//for show
		memset (&domain_ret,0,sizeof(domain_ret));
		ret = conf_drp_show_domain_ip(conn, &domain_ret);
		printf("conf_drp_show_domain_ip 3 ret = %d domain_ret.num =%d\n",ret,domain_ret.num);
		printf("================================================================================\n");
		if( domain_ret.num != 0 )
		{
			for ( i = 0;i < domain_ret.num; i++ ){
				printf("domain name: ");
				printf("%s\t",domain_ret.domain[i].domain_name);
				printf("domain IP : ");
				inet_ntoa_ex(domain_ret.domain[i].ipaddr, ipstr,sizeof(ipstr));
				printf("%s\t",ipstr);							
				printf("IP index for the domain name: ");
				printf("%d\n",domain_ret.domain[i].index);
				}
		}
		printf("===============================================================================\n");
	//end
	
	memset (&domain_conf,0,sizeof(domain_conf));
	strncpy((domain_conf.domain_name),"www.test.com",sizeof(domain_conf.domain_name));	
	domain_conf.ipaddr = 123456;;
	domain_conf.index = 5;
	ret = conf_drp_add_domain_ip(conn,&domain_conf);
	
	printf("conf_drp_add_domain_ip 2 ret = %d\n",ret);
	//for show
		memset (&domain_ret,0,sizeof(domain_ret));
		ret = conf_drp_show_domain_ip(conn, &domain_ret);
		printf("conf_drp_show_domain_ip 4 ret = %d domain_ret.num =%d\n",ret,domain_ret.num);
		printf("================================================================================\n");
		if( domain_ret.num != 0 )
		{
			for ( i = 0;i < domain_ret.num; i++ ){
				printf("domain name: ");
				printf("%s\t",domain_ret.domain[i].domain_name);
				printf("domain IP : ");
				inet_ntoa_ex(domain_ret.domain[i].ipaddr, ipstr,sizeof(ipstr));
				printf("%s\t",ipstr);							
				printf("IP index for the domain name: ");
				printf("%d\n",domain_ret.domain[i].index);
				}
		}
		printf("===============================================================================\n");
	//end


	memset (&domain_conf,0,sizeof(domain_conf));
	strncpy((domain_conf.domain_name),"www.baidu.com",sizeof(domain_conf.domain_name));	
	domain_conf.index = 2;
	ret = conf_drp_del_domain_ip(conn,&domain_conf);
	
	printf("conf_drp_del_domain_ip 1 ret = %d\n",ret);
	//for show
		memset (&domain_ret,0,sizeof(domain_ret));
		ret = conf_drp_show_domain_ip(conn, &domain_ret);
		printf("conf_drp_show_domain_ip 5 ret = %d domain_ret.num =%d\n",ret,domain_ret.num);
		printf("================================================================================\n");
		if( domain_ret.num != 0 )
		{
			for ( i = 0;i < domain_ret.num; i++ ){
				printf("domain name: ");
				printf("%s\t",domain_ret.domain[i].domain_name);
				printf("domain IP : ");
				inet_ntoa_ex(domain_ret.domain[i].ipaddr, ipstr,sizeof(ipstr));
				printf("%s\t",ipstr);							
				printf("IP index for the domain name: ");
				printf("%d\n",domain_ret.domain[i].index);
				}
		}
		printf("===============================================================================\n");
	//end

	memset (&domain_conf,0,sizeof(domain_conf));
	strncpy((domain_conf.domain_name),"www.baidu.com",sizeof(domain_conf.domain_name));	
	domain_conf.index = 7;
	ret = conf_drp_del_domain_ip(conn,&domain_conf);
	
	printf("conf_drp_del_domain_ip 2 ret = %d\n",ret);
	//for show
		memset (&domain_ret,0,sizeof(domain_ret));
		ret = conf_drp_show_domain_ip(conn, &domain_ret);
		printf("conf_drp_show_domain_ip  6 ret = %d domain_ret.num =%d\n",ret,domain_ret.num);
		printf("================================================================================\n");
		if( domain_ret.num != 0 )
		{
			for ( i = 0;i < domain_ret.num; i++ ){
				printf("domain name: ");
				printf("%s\t",domain_ret.domain[i].domain_name);
				printf("domain IP : ");
				inet_ntoa_ex(domain_ret.domain[i].ipaddr, ipstr,sizeof(ipstr));
				printf("%s\t",ipstr);							
				printf("IP index for the domain name: ");
				printf("%d\n",domain_ret.domain[i].index);
				}
		}
		printf("===============================================================================\n");
	//end

	memset (&domain_conf,0,sizeof(domain_conf));
	strncpy((domain_conf.domain_name),"www.baidu.com",sizeof(domain_conf.domain_name));	
	domain_conf.index = 90;
	ret = conf_drp_del_domain_ip(conn,&domain_conf);
	
	printf("conf_drp_del_domain_ip 3 ret = %d\n",ret);
	//for show
		memset (&domain_ret,0,sizeof(domain_ret));
		ret = conf_drp_show_domain_ip(conn, &domain_ret);
		printf("conf_drp_show_domain_ip 7 ret = %d domain_ret.num =%d\n",ret,domain_ret.num);
		printf("================================================================================\n");
		if( domain_ret.num != 0 )
		{
			for ( i = 0;i < domain_ret.num; i++ ){
				printf("domain name: ");
				printf("%s\t",domain_ret.domain[i].domain_name);
				printf("domain IP : ");
				inet_ntoa_ex(domain_ret.domain[i].ipaddr, ipstr,sizeof(ipstr));
				printf("%s\t",ipstr);							
				printf("IP index for the domain name: ");
				printf("%d\n",domain_ret.domain[i].index);
				}
		}
		printf("===============================================================================\n");
	//end

	memset (&domain_conf,0,sizeof(domain_conf));
	strncpy((domain_conf.domain_name),"www.test.com",sizeof(domain_conf.domain_name)); 
	domain_conf.index = 5;
	ret = conf_drp_del_domain_ip(conn,&domain_conf);
	
	printf("conf_drp_del_domain_ip 4 ret = %d\n",ret);
	//for show
		memset (&domain_ret,0,sizeof(domain_ret));
		ret = conf_drp_show_domain_ip(conn, &domain_ret);
		printf("conf_drp_show_domain_ip 8 ret = %d domain_ret.num =%d\n",ret,domain_ret.num);
		printf("================================================================================\n");
		if( domain_ret.num != 0 )
		{
			for ( i = 0;i < domain_ret.num; i++ ){
				printf("domain name: ");
				printf("%s\t",domain_ret.domain[i].domain_name);
				printf("domain IP : ");
				inet_ntoa_ex(domain_ret.domain[i].ipaddr, ipstr,sizeof(ipstr));
				printf("%s\t",ipstr);							
				printf("IP index for the domain name: ");
				printf("%d\n",domain_ret.domain[i].index);
				}
		}
		printf("===============================================================================\n");
	//end

	strncpy((domain_conf.domain_name),"www.baidu.com",sizeof(domain_conf.domain_name));	
	ret = conf_drp_del_domain(conn, &domain_conf);
	printf("conf_drp_del_domain 1 ret = %d\n",ret);
	//for show
		memset (&domain_ret,0,sizeof(domain_ret));
		ret = conf_drp_show_domain_ip(conn, &domain_ret);
		printf("conf_drp_show_domain_ip 9 ret = %d domain_ret.num =%d\n",ret,domain_ret.num);
		printf("================================================================================\n");
		if( domain_ret.num != 0 )
		{
			for ( i = 0;i < domain_ret.num; i++ ){
				printf("domain name: ");
				printf("%s\t",domain_ret.domain[i].domain_name);
				printf("domain IP : ");
				inet_ntoa_ex(domain_ret.domain[i].ipaddr, ipstr,sizeof(ipstr));
				printf("%s\t",ipstr);							
				printf("IP index for the domain name: ");
				printf("%d\n",domain_ret.domain[i].index);
				}
		}
		printf("===============================================================================\n");
	//end

	strncpy((domain_conf.domain_name),"www.test.com",sizeof(domain_conf.domain_name));	
	ret = conf_drp_del_domain(conn, &domain_conf);
	printf("conf_drp_del_domain 2 ret = %d\n",ret);
	
//for show
	memset (&domain_ret,0,sizeof(domain_ret));
	ret = conf_drp_show_domain_ip(conn, &domain_ret);
	printf("conf_drp_show_domain_ip 10 ret = %d domain_ret.num =%d\n",ret,domain_ret.num);
	printf("================================================================================\n");
	if( domain_ret.num != 0 )
	{
		for ( i = 0;i < domain_ret.num; i++ ){
			printf("domain name: ");
			printf("%s\t",domain_ret.domain[i].domain_name);
			printf("domain IP : ");
			inet_ntoa_ex(domain_ret.domain[i].ipaddr, ipstr,sizeof(ipstr));
			printf("%s\t",ipstr);							
			printf("IP index for the domain name: ");
			printf("%d\n",domain_ret.domain[i].index);
			}
	}
	printf("===============================================================================\n");
//end

	
	printf("++++++++end\n");

	return 0;
}


