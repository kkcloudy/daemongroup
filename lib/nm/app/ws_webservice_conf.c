#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <syslog.h>
#include <arpa/inet.h>
#include <dbus/dbus.h>
#include "ws_dbus_def.h"
#include "ws_dbus_list_interface.h"
#include "ws_snmpd_dbus_interface.h"
#include "ac_manage_def.h"
#include "ws_snmpd_engine.h"
#include "ws_snmpd_manual.h"
#include "ws_webservice_conf.h"
#include "ac_manage_interface.h"
#include "ws_dbus_list.h"
#include "ws_init_dbus.h"




static unsigned int master_slot_id;
static unsigned int local_slot_id;

static int ccgi_web_ip_port_check(const char *address_d, const unsigned int port_d, int slot_d)
{
	int ret = 0;
	instance_parameter *para;
	if(SNMPD_DBUS_ERROR == get_slot_dbus_connection(slot_d, &para, SNMPD_SLOT_CONNECT))
	{
		return WEB_FAILURE;
	}
	
    ret = ac_manage_web_ip_port_check(para->connection, address_d, port_d);
	if (WEB_SUCCESS != ret)
	{
		return WEB_FAILURE;
	}

	return WEB_SUCCESS;
}



int web_name_valid(char* str, unsigned int len)
{
	int i;
	char c;
	if((NULL == str) || ( len== 0)){
        return WEB_FAILURE;
	}

	if(len > 10){
        return WEB_FAILURE;
	}

	for (i=0; i< len; i++){
		c = str[i];
		if( ( c >= '0' && c <= '9' ) || (c <= 'z'&&c >= 'a') || (c <= 'Z' && c >= 'A') || (c =='_')){
			continue;
		}
		else {
            LOG("hello world");
            return WEB_FAILURE;
			break;
		}
	}
    return WEB_SUCCESS;
}

static int web_addr_valid(const char *ipaddress)
{
	int ret;

	if(NULL == ipaddress /*|| !strcmp(ipaddress,"0.0.0.0")*/)
	{
		return WEB_FAILURE;
	}
	struct sockaddr_in sa;

	ret = inet_pton(AF_INET,ipaddress,&(sa.sin_addr));
	
	return ret?WEB_SUCCESS:WEB_FAILURE;
}

int web_argv_valid_(char *name, char *type, char *address, char *port,
                   webHostPtr vh)
{
    LOG("%s %s %s %s", name, type, address, port);
    if(web_name_valid(name, strlen(name)) == WEB_FAILURE)  
    {
        LOG("name invalid");
        return WEB_FAILURE;
    }

    vh->name = name;

    if(!strcmp(type, "wispr"))
        vh->type = PORTAL_HTTP_SERVICE;

    else if(!strcmp(type, "wisprssl"))
        vh->type = PORTAL_HTTPS_SERVICE;

    else if(!strcmp(type, "normal"))
        vh->type = PORTAL_SERVICE;

    else if(!strcmp(type,"http"))
        vh->type = HTTP_SERVICE;

    else if(!strcmp(type,"https"))
        vh->type = HTTPS_SERVICE;
    
	if(web_addr_valid(address) == WEB_FAILURE) {
        LOG("address invalid");
		return WEB_FAILURE;
	}
    else 
        vh->address = address;

    vh->port = atoi(port);

    if(vh->port < 0 || vh->port > 65535)
    {
        LOG("port invalid");
        return WEB_FAILURE;
    }

    return WEB_SUCCESS;
}

int web_argv_valid(char *s_type, char *s_address, int *d_type, char **d_address)
{
    if(s_type == NULL)
    {
        *d_type = PORTAL_HTTPS_SERVICE;
    } 
    else if(!strcmp(s_type,"http"))
    {
        *d_type = HTTP_SERVICE;
    }
    else if(!strcmp(s_type,"https"))
    {
        *d_type = HTTPS_SERVICE;
    }
    
	if(web_addr_valid(s_address) == WEB_FAILURE)
	{
        *d_address = NULL;
		return WEB_FAILURE;
	}
    else 
    {
        *d_address = s_address;
    }

    return WEB_SUCCESS;
}

void web_list_flush(struct webInfoHead *infohead)
{

	webInfoPtr info = NULL;
	webHostPtr vh = NULL;
    webIfPtr in = NULL;
	
	LINK_FOREACH(info, infohead, entries)
	{
        if(!LINK_EMPTY(&(info->head)))
        {
            LINK_FOREACH(vh, &(info->head), entries)
            {
                free(vh->name);
                free(vh->address);
                if(!LINK_EMPTY(&(vh->head)))
                {
                    LINK_FOREACH(in, &(vh->head), entries)
                    {
                        free(in->name);
                        free(in->ifname);
                    }
                    free(in);
                }
                free(vh);
            }
            free(info);
        }
	}
}

void web_dir_flush(struct webDirHead *head)
{
	webDirPtr pd;
	int i;
	LINK_FOREACH(pd, head, entries)
	{
		for(i = 0; i < pd->count; i++)
		{
			free(pd->dir[i]);
		}
		free(pd);
	}
}

static void ccgi_dcli_web_slot_init()
{
    master_slot_id = get_product_info(SEM_ACTIVE_MASTER_SLOT_ID_PATH);
    local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
}

static int ccgi_dcli_web_vhost_show(struct webInfoHead *infohead)
{
    ccgi_dcli_web_slot_init();

	struct webHostHead vhead;
	webInfoPtr winfo = NULL;

	unsigned int stat, flag;
    unsigned int sum = 0;
	int i=0 , ret;
	
    instance_parameter *paraHead2 = NULL;
    instance_parameter *pq = NULL;
    list_instance_parameter(&paraHead2, SNMPD_SLOT_CONNECT);
	for(pq=paraHead2;(NULL != pq);pq=pq->next)
	{
		if(pq->connection)
		{
            LINK_INIT(&vhead);
            ret = ac_manage_web_show(pq->connection, &vhead, &stat, &flag);
            if(ret != WEB_FAILURE)
            {
                winfo = (webInfoPtr)malloc(WEB_WEBINFO_SIZE);
                winfo->slotid = pq->parameter.slot_id;
                winfo->server_stat = stat;
                winfo->portal_flag = flag;
                winfo->head = vhead;
                LINK_INSERT_HEAD(infohead, winfo, entries);
            }
            if(pq->parameter.slot_id == master_slot_id)
            {
                sum  = ret;
            }
        }
    }
    return sum;
}

static int ccgi_dcli_web_vhost_add_valid(webHost host, unsigned int slot)
{

	struct webInfoHead infohead;
	
	LINK_INIT(&infohead);
	int ret=0;

	int sum = ccgi_dcli_web_vhost_show(&infohead);

    if(sum >= MAX_VHOST_NUM)
    {
        web_list_flush(&infohead);
        return WEB_OVERMAX;
    }
	
	webInfoPtr info;
	webHostPtr vh;

	LINK_FOREACH(info, &infohead, entries)
    {
        /* portal */
        if(host.type == PORTAL_HTTPS_SERVICE || host.type == PORTAL_HTTP_SERVICE || host.type == PORTAL_SERVICE)
        {
            if(info->portal_flag == 1)
            {
				#if 0
				if(info->slotid == slot)
                {
                    web_list_flush(&infohead);
                    return WEB_PORTAL_ERROR;
                }
				#endif
                if(info->server_stat&PORTAL_SERVICE_ENABLE)
                {
                    web_list_flush(&infohead);
                    return WEB_RUNNING;
                }
            }
        }

        /* service */
	#if 0
        if(host.type == HTTP_SERVICE || host.type == HTTPS_SERVICE)
        {
            if(info->slotid == master_slot_id)
            {
                if(info->server_stat&WEB_SERVICE_ENABLE)
                {
                    web_list_flush(&infohead);
                    return WEB_RUNNING;
                }
            }
        }
	#endif

    }
    
    /* interface */
	LINK_FOREACH(info, &infohead, entries)
	{
        LINK_FOREACH(vh, &(info->head), entries)
        {
            if(!strcmp(vh->name, host.name))
            {
                web_list_flush(&infohead);
                return WEB_NAME_EXISIT;
            }
            #if 1
            if(0 == strcmp(vh->address,host.address) 
            	&& vh->port == host.port 
            	&& info->slotid == slot)
            {
                web_list_flush(&infohead);
                return WEB_EXISIT;
            }
	    if(0 == strcmp(host.address,"0.0.0.0") 
            	&& vh->port == host.port 
            	&& info->slotid == slot) 
            {
		web_list_flush(&infohead);
                return WEB_EXISIT;
            }
            if(0 == strcmp(vh->address,"0.0.0.0") 
            	&& vh->port == host.port 
            	&& info->slotid == slot) 
            {
		web_list_flush(&infohead);
                return WEB_EXISIT;
            }
            #endif
        }
	}
	web_list_flush(&infohead);

	
	/* check ip or port is or not using */
	ret = ccgi_web_ip_port_check(host.address, host.port, slot);
	if (WEB_SUCCESS != ret) {
		return WEB_IP_PORT_ERROR;
	}
    return WEB_SUCCESS; 
}



int ccgi_set_interval_portal_cmd(char *name, char *type, char *ip_addr, char *port, char *slot)
											//0:成功;-1:invalid arguments;_2:invalid slot;-3:too much webservice;-4:service is running;
											//-5:service name is exist;-6:service ip:port exist;-7:portal slot conflict;-8:add failed;-8:port error, can not be used
{
    webHost vh;
	instance_parameter *para;
    unsigned int dslot = 0;

    int ret;

    if(web_argv_valid_(name, type, ip_addr, port, &vh) != WEB_SUCCESS)
    {
        return -1;
    }

    dslot = atoi(slot);

	if(SNMPD_DBUS_ERROR == get_slot_dbus_connection(dslot, &para, SNMPD_SLOT_CONNECT))
	{
		return -2;
	}
	
    switch(ccgi_dcli_web_vhost_add_valid(vh, dslot))
    {
        case WEB_OVERMAX:
			return -3;
        case WEB_RUNNING:
			return -4;
        case WEB_NAME_EXISIT:
			return -5;
        case WEB_EXISIT:
			return -6;
        case WEB_PORTAL_ERROR:
			return -7;		
		case WEB_IP_PORT_ERROR:
	        return -8;
        default:
            break;
    }
    if((ret = ac_manage_web_edit(para->connection, (void *)&vh, HOST_ADD)) != WEB_SUCCESS)
	{
		return -8;
	}

	return 0;
}

int 
ccgi_dcli_communicate_pfm_by_dbus(int opt, 
							int opt_para, 
							unsigned short protocol, 
							char* ifname, 
							unsigned int src_port,
							unsigned int dest_port, 
							int slot,
							char* src_ipaddr,
							char* dest_ipaddr,
							unsigned int send_to)

{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int op_ret;	
    instance_parameter *paraHead2 = NULL;
    instance_parameter *pq = NULL;
    instance_parameter *slot_con = NULL;
	int retu=0;
    

	
	if(slot == send_to)
	{
		return -1;
	}
	query = dbus_message_new_method_call(
								PFM_DBUS_BUSNAME,			
								PFM_DBUS_OBJPATH,		
								PFM_DBUS_INTERFACE,	
								PFM_DBUS_METHOD_PFM_TABLE);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_INT32, &opt,
							DBUS_TYPE_INT32, &opt_para,
							DBUS_TYPE_UINT16, &protocol,
							DBUS_TYPE_STRING, &ifname,
							DBUS_TYPE_UINT32, &src_port,
							DBUS_TYPE_UINT32, &dest_port,
							DBUS_TYPE_STRING,  &src_ipaddr,
							DBUS_TYPE_STRING,  &dest_ipaddr,	
							DBUS_TYPE_INT32,  &slot,
							DBUS_TYPE_INVALID);

	if(-1 == send_to)
	{
		list_instance_parameter(&paraHead2, SNMPD_SLOT_CONNECT);
		for(pq=paraHead2;(NULL != pq);pq=pq->next)
		{
			if(NULL != (pq->connection))
			{
				
				reply = dbus_connection_send_with_reply_and_block (pq->connection,query,-1, &err);
				if (NULL == reply) {
					if (dbus_error_is_set(&err)) {
						dbus_message_unref(query);
						dbus_error_free(&err);
						return -1;
					}
				}
				
				if (dbus_message_get_args ( reply, &err,
											DBUS_TYPE_UINT32,&op_ret,
											DBUS_TYPE_INVALID)) 
				{	
					if(op_ret == 0)
						retu=0;
					else
						retu=-2;
				} 
				else {
					if (dbus_error_is_set(&err)) {
						dbus_error_free(&err);
					}
				}				
			}
		}
		dbus_message_unref(query);		
		dbus_message_unref(reply);
		dbus_error_free(&err);
		
	}
	else
	{	
		if(SNMPD_DBUS_ERROR == get_slot_dbus_connection(send_to, &slot_con, SNMPD_SLOT_CONNECT))
		{
			return -3;
		}
		if(NULL != slot_con->connection)
		{
			
			reply = dbus_connection_send_with_reply_and_block (slot_con->connection,query,-1, &err);
			if (NULL == reply)
			{
				if (dbus_error_is_set(&err))
				{
					dbus_message_unref(query);
					dbus_error_free(&err);
					return -1;
				}
			}
			dbus_message_unref(query);
		}
		else
		{
			return -1;
		}
		if (dbus_message_get_args ( reply, &err,
									DBUS_TYPE_UINT32,&op_ret,
									DBUS_TYPE_INVALID)) 
		{
			if(op_ret == 0)
				retu=0;
			else
				retu=-2;
		} 
		else 
		{
			if (dbus_error_is_set(&err)) 
			{
				dbus_error_free(&err);
			}
		}
		dbus_message_unref(reply);
	}
	return retu;
}


int ccgi_enable_interval_portal_service_cmd()
	//0:成功;-1:connetion is error;-2:webpotal service not config;-3:webportal service is running;-4:webportal service enablefailed,ip or port error
{
	struct webInfoHead infohead;
	instance_parameter *para;
	
	LINK_INIT(&infohead);
	ccgi_dcli_web_vhost_show(&infohead);
	
	webInfoPtr info;
	webHostPtr vh;
	webIfPtr in;

	char buf[32];
	int flag = 1, ret, dslot;
	static int webportal_enable = 0;
	
	LINK_FOREACH(info, &infohead, entries)
	{
		if(info->portal_flag)
		{
			flag = 0;

			if(info->server_stat&PORTAL_SERVICE_ENABLE)
			{
				webportal_enable++;
				continue;
			}

			dslot = info->slotid;
			
			if(SNMPD_DBUS_ERROR == get_slot_dbus_connection(dslot, &para, SNMPD_SLOT_CONNECT))
			{
				return -1;
			}

			ret = ac_manage_web_conf(para->connection, PORTAL_START);

			LINK_FOREACH(vh, &(info->head), entries)
			{
				if(vh->type == PORTAL_HTTP_SERVICE || vh->type == PORTAL_HTTPS_SERVICE || vh->type == PORTAL_SERVICE)
				{
					if(!LINK_EMPTY(&vh->head))
					{
						LINK_FOREACH(in, &(vh->head), entries)
						{
							memset(buf, 0, sizeof(buf));
							sprintf(buf, "%s/32", vh->address);
							ccgi_dcli_communicate_pfm_by_dbus(0, 0, 6, 
									in->ifname, 0, vh->port, info->slotid, "all", buf, in->slot);
						}
					}
				}
			}
		}
	}
	
	web_list_flush(&infohead);
	
	if(flag) {
		return -2;
	}
	
	if (0 < webportal_enable) {
		return -3;
	}

	if(ret == WEB_FAILURE) {
		return -4;
	}
	
	return 0;
}
int disable_interval_portal_service_cmd()
//0:成功;-1:connetion is error;-2:webpotal service not config;-3:webportal service is running;-4:webportal service enablefailed,ip or port error
{
    struct webInfoHead infohead;
	instance_parameter *para;
    
    LINK_INIT(&infohead);
    ccgi_dcli_web_vhost_show(&infohead);
    
    webInfoPtr info;
    webHostPtr vh;
    webIfPtr in;

    char buf[32];
    int flag = 1, ret, dslot;
    static int webportal_disable = 0;
    
    LINK_FOREACH(info, &infohead, entries)
    {
        if(info->portal_flag)
        {
            flag = 0;

            if(!(info->server_stat&PORTAL_SERVICE_ENABLE))
            {
				webportal_disable++;
				continue;
            }

            dslot = info->slotid;
			
			if(SNMPD_DBUS_ERROR == get_slot_dbus_connection(dslot, &para, SNMPD_SLOT_CONNECT))
			{
				return -1;
			}

            ret = ac_manage_web_conf(para->connection, PORTAL_STOP);

            LINK_FOREACH(vh, &(info->head), entries)
            {
                if(vh->type == PORTAL_HTTP_SERVICE || vh->type == PORTAL_HTTPS_SERVICE || vh->type == PORTAL_SERVICE)
                {
                    if(!LINK_EMPTY(&vh->head))
                    {
                        LINK_FOREACH(in, &(vh->head), entries)
                        {
                            memset(buf, 0, sizeof(buf));
                            sprintf(buf, "%s/32", vh->address);
                            ccgi_dcli_communicate_pfm_by_dbus(1, 0, 6, 
                                    in->ifname, 0, vh->port, info->slotid, "all", buf, in->slot);
                        }
                    }
                }
            }
        }
    }
    
    web_list_flush(&infohead);

    if(flag) {
		return -2;
	}
	
    if (0 < webportal_disable) {
		return -3;
    }

    if(ret == WEB_FAILURE) {
		return -4;
    }

    return 0;
}	

int ccgi_add_http_https_ip_port_cmd(char *name,char *type,char *ip_addr, char *port)
												//0:成功;-1:invalid arguments;-2:to much webservice;-3:service is running;
												//-4:service name  exist;-5:ip:port exist;-6:connection is error;-7:set failed;-8:port error, can not be used
{
    ccgi_dcli_web_slot_init();
	
	instance_parameter *para;
    webHost vh;
    int  ret;



	if(web_argv_valid_(name, type, ip_addr, port, &vh) != WEB_SUCCESS)
	{
		return -1;
	}
	int rett=ccgi_dcli_web_vhost_add_valid(vh, master_slot_id);

    switch(rett)
    {
        case WEB_OVERMAX:
            return -2;
        case WEB_NAME_EXISIT:
            return -4;

        case WEB_EXISIT:
            return -5;
		case WEB_IP_PORT_ERROR:
			return -8;
        default:
            break;
    }
	
	if(SNMPD_DBUS_ERROR == get_slot_dbus_connection(master_slot_id, &para, SNMPD_SLOT_CONNECT))
	{
		return -6;
	}

    if((ret = ac_manage_web_edit(para->connection, (void *)&vh, HOST_ADD)) != WEB_SUCCESS)
	{
		return -7;
	}

	return 0;
}

int ccgi_get_slot_num_dcli(char *ifname)
{
	int slotnum;
	int num1,num2,num3,num4;
	int i , count = 0;
	char tmp[128] = {0};

	memcpy(tmp, ifname, strlen(ifname));
	
	if(strncmp(ifname,"eth",3)==0)/*eth : cpu*/
	 {
	 	sscanf(ifname,"eth%d-%d",&slotnum,&num1);
		return slotnum;
		}
	#if 1
	if(strncmp(ifname,"ve",2)==0)/*ve*/
	{
		/* caojia add support for new ve-interface name */
		if (tmp[4] == 'f' || tmp[4] == 's') {
			sscanf(ifname,"ve%02df%d.%d", &slotnum, &num1, &num2);
		}
		else {
			sscanf(ifname,"ve%d.%d", &slotnum, &num1);
		}
		
		return slotnum;
	}
	#endif
	if(strncmp(ifname,"r",1) == 0)/*radio*/
	 {
	 	for(i = 0; i < strlen(ifname); i++)
	 	 {
	 	 	if(tmp[i] == '-')/*use '-' to make sure this radio is local board or remote board ?*/
				count++;
	 	  }
		if(count == 2)/*local board*/
		  {
			slotnum = local_slot_id;
			return slotnum;
			}
		if(count == 3)/*remote board*/
		  {
		  	sscanf(ifname,"r%d-%d-%d-%d.%d",&slotnum,&num1,&num2,&num3,&num4);
		  	return slotnum;
		  	}
		}
	
	if(strncmp(ifname,"wlan",4) == 0)/*wlan*/
	 {
	 	for(i = 0; i < strlen(ifname); i++)
	 	 {
	 	 	if(tmp[i] == '-')/*use '-' to make sure this radio is local board or remote board ?*/
				count++;
	 	  }
		if(count == 1)/*local board*/
		  {
			slotnum = local_slot_id;
			return slotnum;
			}
		if(count == 2)/*remote board*/
		/*  {
		  	sscanf(ifname,"wlan%d-%d-%d",&slotnum,&num1,&num2);
		  	return slotnum;
		  	}
		  */
		{
		  	if(strncmp(ifname,"wlanl",5)==0)/*local hansi wlanlx-x-x*/
		  	{
		  	  sscanf(ifname,"wlanl%d-%d-%d",&slotnum,&num1,&num2);
		  	  return slotnum;
		  	}
			else
			{
		  	  sscanf(ifname,"wlan%d-%d-%d",&slotnum,&num1,&num2);/*remove hansi wlanx-x-x*/
		  	  return slotnum;
			}
		  }
	 	}
	
	if(strncmp(ifname,"ebr",3) == 0 )/*ebr*/
	 {
	 	for(i = 0; i < strlen(ifname); i++)
	 	 {
	 	 	if(tmp[i] == '-')/*use '-' to make sure this radio is local board or remote board ?*/
				count++;
	 	  }
		if(count == 1)/*local board*/
		  {
			slotnum = local_slot_id;
			return slotnum;
			}
		if(count == 2)/*remote board*/
		  /*{
		  	sscanf(ifname,"ebr%d-%d-%d",&slotnum,&num1,&num2);
		  	return slotnum;
		  	}*/
			{
			  	if(strncmp(ifname,"ebrl",4)==0)/*local hansi ebrlx-x-x*/
			  	{
			  	  sscanf(ifname,"ebrl%d-%d-%d",&slotnum,&num1,&num2);
			  	  return slotnum;
			  	}
				else
				{
			  	  sscanf(ifname,"ebr%d-%d-%d",&slotnum,&num1,&num2);/*remove hansi ebrx-x-x*/
			  	  return slotnum;
				}
			  }
	 	}
	if(strncmp(ifname,"obc0",4) == 0)
		{
			slotnum = local_slot_id;
			return slotnum;
		}
	return 0;/*err*/
	
}

static int ccgi_dcli_web_int_add_valid(webIf n, int *slot)

{
    struct webInfoHead infohead;

    LINK_INIT(&infohead);

    ccgi_dcli_web_vhost_show(&infohead);

    webInfoPtr info;
    webHostPtr vh;
    webIfPtr in;

    unsigned int type = 0;
    unsigned int tmp  = 0;
    unsigned int found = 1;

    LINK_FOREACH(info, &infohead, entries)
    {
        LINK_FOREACH(vh, &(info->head), entries)
        {
            if(!strcmp(vh->name, n.name))
            {
                tmp = info->slotid;
                type = vh->type;
                found = 0;

                LINK_FOREACH(in, &(vh->head), entries)
                {
                    if(vh->count >= MAX_INTERFACE_NUM)
                    {
                        web_list_flush(&infohead);
                        return WEB_OVERMAX;
                    }
                    if(!strcmp(in->ifname, n.ifname))
                    {
                        web_list_flush(&infohead);
                        return WEB_EXISIT;
                    }
                }
            }
        }
    }

    if(found)
    {
        web_list_flush(&infohead);
        return WEB_NOTFOUND;
    }

    if(type)
    {
        LINK_FOREACH(info, &infohead, entries)
        {
            /* portal */
            if(type == PORTAL_HTTPS_SERVICE || type == PORTAL_HTTP_SERVICE || type == PORTAL_SERVICE)
            {
                if(info->portal_flag == 1)
                {
                    if(info->server_stat&PORTAL_SERVICE_ENABLE)
                    {
                        web_list_flush(&infohead);
                        return WEB_RUNNING;
                    }
                }
            }

            /* service */
            if(type == HTTP_SERVICE || type == HTTPS_SERVICE)
            {
                if(info->slotid == master_slot_id)
                {
                    if(info->server_stat&WEB_SERVICE_ENABLE)
                    {
                        web_list_flush(&infohead);
                        return WEB_RUNNING;
                    }
                }
            }
        }
    }

    web_list_flush(&infohead);
    *slot = tmp;
    return WEB_SUCCESS;
}

int ccgi_add_web_forword_cmd(char *webname, char *infname)
	//-1:invalid arguments webname;-2:invalid ifname;-3:invalid connection;-8:failed
	//-4:to much interface;-5:service name not exist;-6:interface exist;-7:is running
{
    int sslot = 0, dslot = 0;
    int ret = 0,cgi_ret=0; 
	instance_parameter *para;
	instance_parameter *para1;

    if(web_name_valid(webname, strlen(webname)) == WEB_FAILURE)
    {
        return -1;
    }

    sslot = ccgi_get_slot_num_dcli(infname);
    if(sslot < 0 || sslot > 16)
    {
        return -2;
    }

	if(SNMPD_DBUS_ERROR == get_slot_dbus_connection(sslot, &para, SNMPD_SLOT_CONNECT))
	{
		return -3;
	}

    webIf in;

    in.name = webname;
    in.ifname = infname;
    in.slot = sslot;

	cgi_ret =ccgi_dcli_web_int_add_valid(in, &dslot);
    switch(cgi_ret)
    {
        case WEB_OVERMAX:
            return -4;

        case WEB_NOTFOUND:
            return -5;

        case WEB_EXISIT:
            return -6;

        default:
            break;
    }

    if((ret = ac_manage_web_edit(ccgi_dbus_connection, (void *)&in, IFNAME_ADD)) != WEB_SUCCESS)
    {
		return -8;
    }

    return 0;
}
void ccgi_free_show_interval_portalservice_info_cmd(struct web_info *WtpIfHead)
{
	struct web_info *f1 = NULL, *f2 = NULL;
		
	if(WtpIfHead)
	{
		f1=WtpIfHead->next; 	 
		if(f1)
		{
			f2=f1->next;
			while(f2!=NULL)
			{
				FREE_OBJECT(f1->name);
				FREE_OBJECT(f1->address);
				FREE_OBJECT(f1->infname);
				free(f1);
				f1=f2;
				f2=f2->next;
			}
			
			FREE_OBJECT(f1->name);
			FREE_OBJECT(f1->address);
			FREE_OBJECT(f1->infname);
			free(f1);
		}
	}
}

int ccgi_show_interval_portalservice_info_cmd(struct web_info *WtpIfHead,int *num,int slot)
{  
	struct webInfoHead infohead;
	
	LINK_INIT(&infohead);
	int sum = ccgi_dcli_web_vhost_show(&infohead);
	*num=sum;
	
	webInfoPtr info;
	webHostPtr vh;
    webIfPtr in;
	char buf[128] = {0};

	struct web_info *tail = NULL,*q = NULL;
	WtpIfHead->next = NULL;
	tail = WtpIfHead;
	LINK_FOREACH(info, &infohead, entries)
	{
		if(info->portal_flag)
		{
            if(info->slotid == slot)
            {
				LINK_FOREACH(vh, &(info->head), entries)
				{
					if((vh->type == PORTAL_HTTP_SERVICE) || (vh->type == PORTAL_HTTPS_SERVICE)||(vh->type == PORTAL_SERVICE))
					{
						if(!LINK_EMPTY(&vh->head))
						{
							LINK_FOREACH(in, &(vh->head), entries)
							{
								strcat(buf,in->ifname);
							}
						}

					
						q = (struct web_info*)malloc(sizeof(struct web_info));
						if(q)
						{
							q->name = (unsigned char*)malloc(strlen(vh->name)+1);
							if(q->name)
							{
								memset(q->name, 0, (strlen(vh->name)+1));
								strncpy(q->name, vh->name, strlen(vh->name));
							}
							q->type=vh->type;
							q->address = (unsigned char*)malloc(strlen(vh->address)+1);
							if(q->address)
							{
								memset(q->address, 0, (strlen(vh->address)+1));
								strncpy(q->address, vh->address, strlen(vh->address));
							}
							q->port=vh->port;
							
							q->infname = (unsigned char*)malloc(strlen(buf)+1);
							if(q->infname)
							{
								memset(q->infname, 0, (strlen(buf)+1));
								strncpy(q->infname, buf, strlen(buf));
							}
						}
						if((q)&&(tail))
						{
							q->next = NULL;
							tail->next = q;
							tail = q;
						}
					}
				}							
			}
		}
	}
	web_list_flush(&infohead);	
    return 0;
}	
void ccgi_free_show_webservice_info_cmd(struct web_info *WtpIfHead)
{
	struct web_info *f1 = NULL, *f2 = NULL;
		
	if(WtpIfHead)
	{
		f1=WtpIfHead->next; 	 
		if(f1)
		{
			f2=f1->next;
			while(f2!=NULL)
			{
				FREE_OBJECT(f1->name);
				FREE_OBJECT(f1->address);
				FREE_OBJECT(f1->infname);
				free(f1);
				f1=f2;
				f2=f2->next;
			}
			
			FREE_OBJECT(f1->name);
			FREE_OBJECT(f1->address);
			FREE_OBJECT(f1->infname);
			free(f1);
		}
	}
}

int ccgi_show_webservice_info_cmd(struct web_info *WtpIfHead,int *num,int *slot)
{
	struct webInfoHead infohead;
	char buf[128] = {0};
	
	LINK_INIT(&infohead);
	int sum = ccgi_dcli_web_vhost_show(&infohead);

	*num=sum;
	
	webInfoPtr info = NULL;
	webHostPtr vh = NULL;
    webIfPtr in = NULL;


	struct web_info *tail = NULL,*q = NULL;
	WtpIfHead->next = NULL;
	tail = WtpIfHead;
	
	LINK_FOREACH(info, &infohead, entries)
	{
		if(info->slotid ==  master_slot_id)
		{
			LINK_FOREACH(vh, &(info->head), entries)
			{
				if((vh->type == HTTP_SERVICE) || (vh->type == HTTPS_SERVICE))
				{
					memset(buf,0,128);
					if(!LINK_EMPTY(&vh->head))
					{
						LINK_FOREACH(in, &(vh->head), entries)
						{
							if(in->ifname)
								strcat(buf,in->ifname);
						}
					}
					q = (struct web_info*)malloc(sizeof(struct web_info));
					if(q)
					{
						q->name = (unsigned char*)malloc(strlen(vh->name)+1);
						if(q->name)
						{
							memset(q->name, 0, (strlen(vh->name)+1));
							strncpy(q->name, vh->name, strlen(vh->name));
						}
						q->type=vh->type;
						q->address = (unsigned char*)malloc(strlen(vh->address)+1);
						if(q->address)
						{
							memset(q->address, 0, (strlen(vh->address)+1));
							strncpy(q->address, vh->address, strlen(vh->address));
						}
						q->port=vh->port;
						
						q->infname = (unsigned char*)malloc(strlen(buf)+1);
						if(q->infname)
						{
							memset(q->infname, 0, (strlen(buf)+1));
							strncpy(q->infname, buf, strlen(buf));
						}
					}
					if((q)&&(tail))
					{
						q->next = NULL;
						tail->next = q;
						tail = q;
					}
				}			
			}
		}
	}
	web_list_flush(&infohead);	
	return 0;
}	
static int ccgi_dcli_web_vhost_del_valid(const char *name, unsigned int *slot)
{
	struct webInfoHead infohead;
	
	LINK_INIT(&infohead);

	ccgi_dcli_web_vhost_show(&infohead);
	
	webInfoPtr info;
	webHostPtr vh;

    unsigned int type = 0;
    unsigned int tmp  = 0;
	
    /* interface */
	LINK_FOREACH(info, &infohead, entries)
	{
        LINK_FOREACH(vh, &(info->head), entries)
        {
            if(!strcmp(vh->name, name))
            {
                tmp = info->slotid;
                type = vh->type;
            }
        }
	}

    if(type)
    {
        LINK_FOREACH(info, &infohead, entries)
        {
            /* portal */
            if(type == PORTAL_HTTPS_SERVICE || type == PORTAL_HTTP_SERVICE || type == PORTAL_SERVICE)
            {
                if(info->portal_flag == 1)
                {
                    if(info->server_stat&PORTAL_SERVICE_ENABLE)
                    {
                        web_list_flush(&infohead);
                        return WEB_RUNNING;
                    }
                }
            }

            /* service */
            if(type == HTTP_SERVICE || type == HTTPS_SERVICE)
            {
                if(info->slotid == master_slot_id)
                {
                    if(info->server_stat&WEB_SERVICE_ENABLE)
                    {
                        web_list_flush(&infohead);
                        return WEB_RUNNING;
                    }
                }
            }
        }
    }

	web_list_flush(&infohead);

    if(tmp == 0)
        return WEB_NOTFOUND;
    else 
        *slot = tmp;
    return WEB_SUCCESS;
}

int ccgi_delete_portal_config_cmd(char *name)
	//0:success-1:invalid name;-2:web is running;-3:no found this name
	//-4connection is error;-5:failed
{
    unsigned int dslot = 0;
    int ret;
	instance_parameter *para;
    if(web_name_valid(name, strlen(name)) == WEB_FAILURE)
    {
        return -1;
    }

    switch(ccgi_dcli_web_vhost_del_valid(name, &dslot))
    {
        case WEB_NOTFOUND:
            return -3;

        case WEB_RUNNING:
            return -2;

        default:
            break;
    }
	
	if(SNMPD_DBUS_ERROR == get_slot_dbus_connection(dslot, &para, SNMPD_SLOT_CONNECT))
	{
		return -4;
	}

    if((ret = ac_manage_web_edit(para->connection, (void *)name, HOST_DEL)) != WEB_SUCCESS)
    {
        return -5;
    }

	return 0;
}	

int ccgi_delete_http_https_config_cmd(char *name)
	//0:success-1:invalid name;-2:web is running;-3:no found this name
	//-4connection is error;-5:failed;
{
    unsigned int dslot = 0;
    int ret;
	instance_parameter *para;
	
    ccgi_dcli_web_slot_init();
    if(web_name_valid(name, strlen(name)) == WEB_FAILURE)
    {
        return -1;
    }

    switch(ccgi_dcli_web_vhost_del_valid(name, &dslot))
    {
        case WEB_NOTFOUND:
            return -3;
        default:
            break;
    }
	if(SNMPD_DBUS_ERROR == get_slot_dbus_connection(master_slot_id, &para, SNMPD_SLOT_CONNECT))
	{
		return -4;
	}

    if((ret = ac_manage_web_edit(para->connection, (void *)name, HOST_DEL)) != WEB_SUCCESS)
    {
		return -5;
    }

	return 0;
}	


int ccgi_contrl_enable_webservice_cmd()
//-1:webconfig service not config;-2:webconfig service is running
//-4:connection is error;-5:webconfig service is running;-6:webconfig service enable failed
{
	struct webInfoHead infohead;
	instance_parameter *para;
	
	LINK_INIT(&infohead);

	int sum = ccgi_dcli_web_vhost_show(&infohead);

	if(sum < 1)
	{
		return -1;
	}
	
	webInfoPtr info;
	webHostPtr vh;
	webIfPtr in;

	char buf[32] = {0};
	int flag = 1, ret, dslot;
	static int webconfig_enable = 0;
	
	LINK_FOREACH(info, &infohead, entries)
	{
		if(info->slotid == master_slot_id)
		{
			flag = 0;
			
			if(info->server_stat&WEB_SERVICE_ENABLE)
			{
				web_list_flush(&infohead);
				return -2;
			}
			LINK_FOREACH(vh, &(info->head), entries)
			{
				if(vh->type == HTTP_SERVICE || vh->type == HTTPS_SERVICE)
				{
					if(!LINK_EMPTY(&vh->head))
					{
						LINK_FOREACH(in, &(vh->head), entries)
						{
							memset(buf, 0, sizeof(buf));
							sprintf(buf, "%s/32", vh->address);
							ccgi_dcli_communicate_pfm_by_dbus(0, 0, 6, 
									in->ifname, 0, vh->port, info->slotid, "all", buf, in->slot);
						}
					}
				}
			}
		}
	}
	
	if(SNMPD_DBUS_ERROR == get_slot_dbus_connection(master_slot_id, &para, SNMPD_SLOT_CONNECT))
	{
		return -3;
	}
	ret = ac_manage_web_conf(para->connection, WEB_START);

	web_list_flush(&infohead);

	if(flag) {
		return -4;
	}
	
	if (0 < webconfig_enable) {
		return -5;
	}
	
	if(ret == WEB_FAILURE) {
		return -6;
	}
	return 0;
}
int ccgi_contrl_disable_webservice_cmd()
	//-1:webconfig service not config;-2:webconfig service is not running
	//-4:connection is error;-5:webconfig service is not running;-6:webconfig service enable failed
{
	struct webInfoHead infohead;
	instance_parameter *para;
	
	LINK_INIT(&infohead);

	int sum = ccgi_dcli_web_vhost_show(&infohead);

	if(sum < 1)
	{
		return -1;
	}
	
	webInfoPtr info;
	webHostPtr vh;
	webIfPtr in;

	char buf[32];
	int flag = 1, ret, dslot;
	static int webconfig_disable = 0;
	
	LINK_FOREACH(info, &infohead, entries)
	{
		if(info->slotid == master_slot_id)
		{
			flag = 0;

			if(!(info->server_stat&WEB_SERVICE_ENABLE))
			{
				web_list_flush(&infohead);
				return -2;
			}
			LINK_FOREACH(vh, &(info->head), entries)
			{
				if(vh->type == HTTP_SERVICE || vh->type == HTTPS_SERVICE)
				{
					if(!LINK_EMPTY(&vh->head))
					{
						LINK_FOREACH(in, &(vh->head), entries)
						{
							memset(buf, 0, sizeof(buf));
							sprintf(buf, "%s/32", vh->address);
							ccgi_dcli_communicate_pfm_by_dbus(1, 0, 6, 
									in->ifname, 0, vh->port, info->slotid, "all", buf, in->slot);
						}
					}
				}
			}
		}
	}
	if(SNMPD_DBUS_ERROR == get_slot_dbus_connection(master_slot_id, &para, SNMPD_SLOT_CONNECT))
	{
		return -3;
	}
	ret = ac_manage_web_conf(para->connection, WEB_STOP);

	web_list_flush(&infohead);
	
	if(flag) {
		return -4;
	}
	
	if (0 < webconfig_disable) {
		return -5;
	}

	if(ret == WEB_FAILURE) {
		return -6;
	}
	return 0;
}

