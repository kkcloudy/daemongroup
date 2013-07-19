#ifdef __cplusplus
extern "C"
{
#endif

#include <config/auteware_config.h>
#include <zebra.h>
#include <dbus/dbus.h>
#include <stdlib.h>
#include <sysdef/npd_sysdef.h>
#include <dbus/npd/npd_dbus_def.h>
#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "vty.h"
#include "command.h"
#include "if.h"
#include "sysdef/returncode.h"
#include "dcli_snmp.h"
#include "ws_sysinfo.h"
#include <sys/wait.h>
#include "ws_snmpd_dbus_interface.h"
#include "ac_manage_def.h"
#include "ws_snmpd_engine.h"
#include "ws_snmpd_manual.h"
#include "ws_webservice_conf.h"
#include "ac_manage_interface.h"
#include "dcli_main.h"
#include "dcli_sem.h"
#include "dcli_apache.h"

static unsigned int master_slot_id;
static unsigned int local_slot_id;

struct cmd_node web_service_node =
{
    WEBSERVICE_NODE,
    "%s(config-web-service)# "
};

/*
ret = dcli_communicate_pfm_by_dbus(ADD_PFM_FORWORD_TABLE, 0, protocol, 
								ifname, 
								src_port, dest_port, 
								active_master_slot,
								src_ipaddr, dest_ipaddr,
								send_to_slot);
*/

static void dcli_web_slot_init()
{
    master_slot_id = get_product_info(SEM_ACTIVE_MASTER_SLOT_ID_PATH);
    local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
}

static int dcli_web_ip_port_check(const char *address_d, const unsigned int port_d, int slot_d)
{
	int ret = 0;
	
    ret = ac_manage_web_ip_port_check(dbus_connection_dcli[slot_d]->dcli_dbus_connection, address_d, port_d);
	if (WEB_SUCCESS != ret)
	{
		return WEB_FAILURE;
	}

	return WEB_SUCCESS;
}

static int dcli_web_vhost_show(struct webInfoHead *infohead)
{
    dcli_web_slot_init();

	struct webHostHead vhead;
	webInfoPtr winfo = NULL;

	unsigned int stat, flag;
    unsigned int sum = 0;
	int i , ret;

	for(i = 0; i< MAX_SLOT; i++)
	{
		if(dbus_connection_dcli[i]->dcli_dbus_connection)
		{
            LINK_INIT(&vhead);
            ret = ac_manage_web_show(dbus_connection_dcli[i]->dcli_dbus_connection, &vhead, &stat, &flag);
            if(ret != WEB_FAILURE)
            {
                winfo = (webInfoPtr)malloc(WEB_WEBINFO_SIZE);
                winfo->slotid = i;
                winfo->server_stat = stat;
                winfo->portal_flag = flag;
                winfo->head = vhead;
                LINK_INSERT_HEAD(infohead, winfo, entries);
            }
            if(i == master_slot_id)
            {
                sum  = ret;
            }
        }
    }
    return sum;
}

static int dcli_web_vhost_add_valid(webHost host, unsigned int slot)
{
	struct webInfoHead infohead;
	
	LINK_INIT(&infohead);

	int sum = dcli_web_vhost_show(&infohead);
	int ret = 0;
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
            #endif
        }
	}
	web_list_flush(&infohead);

	/* check ip or port is or not using */
	ret = dcli_web_ip_port_check(host.address, host.port, slot);
	if (WEB_SUCCESS != ret) {
		return WEB_IP_PORT_ERROR;
	}
	
    return WEB_SUCCESS; 
}

static int dcli_web_vhost_del_valid(const char *name, unsigned int *slot)
{
	struct webInfoHead infohead;
	
	LINK_INIT(&infohead);

	dcli_web_vhost_show(&infohead);
	
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

static int dcli_web_int_add_valid(webIf n, int *slot)
{
    struct webInfoHead infohead;

    LINK_INIT(&infohead);

    dcli_web_vhost_show(&infohead);

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

static int dcli_web_int_del_valid(webIf n, int *slot)
{

    struct webInfoHead infohead;

    LINK_INIT(&infohead);

    dcli_web_vhost_show(&infohead);

    webInfoPtr info;
    webHostPtr vh;
    webIfPtr in;

    unsigned int tmp = 0; 
    unsigned int type = 0;
    unsigned int found1 = 1;
    unsigned int found2 = 1;

    LINK_FOREACH(info, &infohead, entries)
    {
        LINK_FOREACH(vh, &(info->head), entries)
        {
            if(!strcmp(vh->name, n.name))
            {
                found1 = 0;
                LINK_FOREACH(in, &(vh->head), entries)
                {
                    if(!strcmp(in->ifname, n.ifname))
                    {
                        found2 = 0;
                        tmp = info->slotid;
                        type = vh->type;

                    }
                }
            }
        }
    }

    if(found1 || found2) {
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

    if(tmp == 0)
        return WEB_NOTFOUND;
    else 
        *slot = tmp;
    return WEB_SUCCESS;

}

int dcli_web_get_dir(struct webDirHead *head, int *sum)
{
	webDirPtr pd;
	int count, i;	
	for(i = 0; i < MAX_SLOT; i++)
	{
		if(dbus_connection_dcli[i]->dcli_dbus_connection)
		{
			pd = (webDirPtr)malloc(WEB_WEBDIR_SIZE);
			ac_manage_web_show_pages(dbus_connection_dcli[i]->dcli_dbus_connection, pd->dir, &count);
			pd->count = count;
			pd->slot = i;
			*sum += count;
			LINK_INSERT_HEAD(head, pd, entries);
		}
	}
}

DEFUN(conf_web_service_func,
      conf_web_service_cmd,
      "config web service",
      CONFIG_STR
      "Config web server apache2!\n"
     )
{
    if (CONFIG_NODE == vty->node)
    {
        dcli_web_slot_init();
        if(master_slot_id != local_slot_id) 
        {
            vty_out(vty, "not active master board. failed\n");
            return CMD_WARNING;
        }
        vty->node = WEBSERVICE_NODE;
    }
    else
    {
        vty_out (vty, "terminal mode change must under configure mode\n");
        return CMD_WARNING;
    }
    return CMD_SUCCESS;
}

DEFUN(contrl_enable_web_service_func,
      contrl_enable_webservice_cmd,
      "service webconfig enable",
      "web service control\n"
      "web service \n"
      "enable webconfig service \n"
     )
{
    struct webInfoHead infohead;
    
    LINK_INIT(&infohead);

	int sum = dcli_web_vhost_show(&infohead);

    if(sum < 1)
    {
        vty_out(vty, "webconfig service not config. falied\n");
        return CMD_WARNING;
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
                #if 1
                vty_out(vty, "webconfig service is running.\n");
				web_list_flush(&infohead);
                return CMD_WARNING;
                #else
                webconfig_enable++;
                continue;
                #endif
            }

            //dslot = info->slotid;

            //ret = ac_manage_web_conf(dbus_connection_dcli[dslot]->dcli_dbus_connection, WEB_START);

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
                            dcli_communicate_pfm_by_dbus(0, 0, 6, 
                                    in->ifname, 0, vh->port, info->slotid, "all", buf, in->slot);
                        }
                    }
                }
            }
        }
    }
    
    ret = ac_manage_web_conf(dbus_connection_dcli[master_slot_id]->dcli_dbus_connection, WEB_START);

    web_list_flush(&infohead);

    if(flag) {
		vty_out(vty, "webconfig service not config. falied\n");
        return CMD_SUCCESS;
	}
	
	if (0 < webconfig_enable) {
        vty_out(vty, "webconfig service is running.\n");
        return CMD_SUCCESS;
	}
    
    if(ret == WEB_FAILURE) {
        vty_out(vty, "webconfig service enable failed, ip or port error\n");
	}
	
    return CMD_SUCCESS;
}

DEFUN(contrl_disable_service_func,
      contrl_disable_webservice_cmd,
      "service webconfig disable",
      "web service control\n"
      "web service \n"
      "stop webconfig service \n"
     )
{
    struct webInfoHead infohead;
    
    LINK_INIT(&infohead);

	int sum = dcli_web_vhost_show(&infohead);

    if(sum < 1)
    {
        vty_out(vty, "webconfig service not config. falied\n");
        return CMD_WARNING;
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
                #if 1
        		vty_out(vty, "webconfig service is not running.\n");
                web_list_flush(&infohead);
                return CMD_WARNING;
                #else
                webconfig_disable++;
                continue;
                #endif
            }

            //dslot = info->slotid;

            //ret = ac_manage_web_conf(dbus_connection_dcli[dslot]->dcli_dbus_connection, WEB_STOP);

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
                            dcli_communicate_pfm_by_dbus(1, 0, 6, 
                                    in->ifname, 0, vh->port, info->slotid, "all", buf, in->slot);
                        }
                    }
                }
            }
        }
    }
    
    ret = ac_manage_web_conf(dbus_connection_dcli[master_slot_id]->dcli_dbus_connection, WEB_STOP);

    web_list_flush(&infohead);
    
    if(flag) {
		vty_out(vty, "webconfig service not config. falied\n");
        return CMD_SUCCESS;
	}
	
	if (0 < webconfig_disable) {
        vty_out(vty, "webconfig service is not running.\n");
        return CMD_SUCCESS;
	}

    if(ret == WEB_FAILURE) {
		vty_out(vty, "webconfig service disable falied, ip or port error");
    }
	
	return CMD_SUCCESS;
}

DEFUN(show_webservice_info_func,
      show_webservice_info_cmd,
      "show webconfig configuration",
      SHOW_STR
      "webconfig configure\n"
      "configuration for web service\n"
     )
{
	struct webInfoHead infohead;
	
	LINK_INIT(&infohead);
	int sum = dcli_web_vhost_show(&infohead);
    LOG("sum = %d ", sum);

    if(sum < 1)
    {
        return CMD_WARNING;
    }
	
	webInfoPtr info = NULL;
	webHostPtr vh = NULL;
    webIfPtr in = NULL;
	
	LINK_FOREACH(info, &infohead, entries)
	{
		if(info->slotid ==  master_slot_id)
		{
			if(info->server_stat & WEB_SERVICE_ENABLE)
				vty_out(vty, "status: running\n");
			else
				vty_out(vty, "status: not running\n");
				
			vty_out(vty,"%-8s%-11s%-16s%-6s%-s\n","type","name","ip","port","ifname");
			vty_out(vty,"--------------------------------------------------------------------\n");
			LINK_FOREACH(vh, &(info->head), entries)
			{
				if(vh->type == HTTP_SERVICE) {
					vty_out(vty, "%-8s","http");
				} else if (vh->type == HTTPS_SERVICE) {
                    vty_out(vty, "%-8s", "https");
                } else {
					continue;
                }
			    vty_out(vty,"%-11s%-16s%-6d",vh->name, vh->address, vh->port); 
                if(!LINK_EMPTY(&vh->head))
                {
                    LINK_FOREACH(in, &(vh->head), entries)
                    {
                        vty_out(vty, "%s ", in->ifname);
                    }
                }
                vty_out(vty, "\n");
			}
			vty_out(vty,"--------------------------------------------------------------------\n");
		}
	}

	web_list_flush(&infohead);
	return CMD_SUCCESS;
}

DEFUN(add_web_forword_func,
      add_web_forword_cmd,
	  "add webforward WEBNAME IFNAME",
	  "add webforward\n" \
	  "web forward\n" \
      "service name (specify webservice name)\n" \
      "interface name (e.g. eth*-*)"
     )
{
    int sslot = 0, dslot = 0;
    int ret = 0; 

    if(web_name_valid(argv[0], strlen(argv[0])) == WEB_FAILURE)
    {
		vty_out(vty,"invalid arguments. failed\n");
        return WEB_SUCCESS;
    }

    sslot = get_slot_num_dcli(argv[1]);
    if(sslot < 0 || sslot > 16)
    {
        vty_out(vty,"invalid ifname. failed\n");
        return CMD_WARNING;
    }

    if(dbus_connection_dcli[sslot]->dcli_dbus_connection == NULL)
    {
		vty_out(vty,"invalid ifname. failed\n");
        return CMD_WARNING;
    }

    webIf in;

    in.name = argv[0];
    in.ifname = argv[1];
    in.slot = sslot;

    switch(dcli_web_int_add_valid(in, &dslot))
    {
        case WEB_OVERMAX:
		    vty_out(vty,"to much interface. failed\n");
            return CMD_WARNING;

        case WEB_NOTFOUND:
		    vty_out(vty,"service name not exist. failed\n");
            return CMD_WARNING;

        case WEB_EXISIT:
		    vty_out(vty,"interface exist. failed\n");
            return CMD_WARNING;

        case WEB_RUNNING:
		    vty_out(vty,"is running. failed\n");
            return CMD_WARNING;

        default:
            break;
    }

    LOG("------------------------------1 %d", dslot);

    if((ret = ac_manage_web_edit(dbus_connection_dcli[dslot]->dcli_dbus_connection, (void *)&in, IFNAME_ADD)) != WEB_SUCCESS)
    {
		vty_out(vty, "failed.\n");
		return CMD_WARNING;
    }

    return WEB_SUCCESS;
}

DEFUN(del_web_forword_func,
      del_web_forword_cmd,
	  "del webforward WEBNAME IFNAME",
	  "del web forward\n" \
	  "del forward\n" \
      "service name (specify webservice name)\n" \
      "interface name (e.g. eth*-*)"
     )
{
    int sslot = 0, dslot = 0; 
    int ret = 0; 
    if(web_name_valid(argv[0], strlen(argv[0])) == WEB_FAILURE)
    {
		vty_out(vty,"invalid arguments. failed\n");
        return WEB_SUCCESS;
    }

    sslot = get_slot_num_dcli(argv[1]);
    if(sslot < 0 || sslot > 16)
    {
		vty_out(vty,"invalid ifname. failed\n");
        return CMD_WARNING;
    }
    if(dbus_connection_dcli[sslot]->dcli_dbus_connection == NULL)
    {
		vty_out(vty,"invalid ifname. failed\n");
        return CMD_WARNING;
    }

    webIf in;

    in.name = argv[0];
    in.ifname = argv[1];
    in.slot = sslot;

    switch(dcli_web_int_del_valid(in, &dslot))
    {
        case WEB_NOTFOUND:
            vty_out(vty, "service name or ifname not found. failed.\n");
            return CMD_WARNING;

        case WEB_RUNNING:
            vty_out(vty, "is running. failed.\n");
            return CMD_WARNING;

        default:
            break;
    }

    LOG("------------------------------1 %d", dslot);

    if((ret = ac_manage_web_edit(dbus_connection_dcli[dslot]->dcli_dbus_connection, (void *)&in, IFNAME_DEL)) != WEB_SUCCESS)
    {
		vty_out(vty, "failed.\n");
		return CMD_WARNING;
    }

    return WEB_SUCCESS;
}

/*add the ip and port for http or https service */
DEFUN(add_http_https_ip_port_func,
      add_http_https_ip_port_cmd,
	  "add webconfig WEBNAME (http|https) A.B.C.D <1-65535>",
	  "add webconfig http or https \n" \
	  "web config\n" \
      "web config name (alphanumeric and '_' only, maximize 10 characters)\n" \
	  "web type http\n"\
	  "web type https\n"\
	  "IP address (e.g 192.168.1.1)\n"\
	  "port (e.g http:80, https:443)\n" \
     )
{
    dcli_web_slot_init();

    struct webHostHead head;
    webHost vh;

    unsigned int type = 0;
    unsigned int port = 0; 
    char *address = NULL; 
    char *name = NULL;

    int i, ret;

	if(web_argv_valid_(argv[0], argv[1], argv[2], argv[3], &vh) != WEB_SUCCESS)
	{
		vty_out(vty,"invalid arguments. failed\n");
		return CMD_WARNING;
	}

    LOG("%s %d %d", vh.name, vh.address, vh.port, vh.type);

    switch(dcli_web_vhost_add_valid(vh, master_slot_id))
    {
        case WEB_OVERMAX:
		    vty_out(vty,"to much webservice. failed\n");
            return CMD_WARNING;

        case WEB_RUNNING:
		    vty_out(vty,"is running. failed\n");
            return CMD_WARNING;

        case WEB_NAME_EXISIT:
		    vty_out(vty,"service name exist. failed\n");
            return CMD_WARNING;

        case WEB_EXISIT:
		    vty_out(vty,"service ip:port exist. failed\n");
            return CMD_WARNING;
            
		case WEB_IP_PORT_ERROR:
			vty_out(vty, "service ip:port error, can not be used. failed\n");
            return CMD_WARNING;
        default:
            break;
    }

    LOG("%s %s %d %d", vh.name, vh.address, vh.port, vh.type);
    if((ret = ac_manage_web_edit(dbus_connection_dcli[master_slot_id]->dcli_dbus_connection, (void *)&vh, HOST_ADD)) != WEB_SUCCESS)
	{
		vty_out(vty, "failed.\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
} 

/*delete the  ip and port for http or https*/
DEFUN(delete_http_https_config_func,
      delete_http_https_config_cmd,
      "del webconfig WEBNAME",
      "del webconfig\n" \
	  "web config name\n" \
      "web config name \n" 
     )
{
    char *name = argv[0];
    unsigned int dslot = 0;
    int ret;
    if(web_name_valid(name, strlen(name)) == WEB_FAILURE)
    {
        vty_out(vty,"invalid arguments. failed\n");
        return WEB_SUCCESS;
    }

    switch(dcli_web_vhost_del_valid(name, &dslot))
    {
        case WEB_NOTFOUND:
            vty_out(vty, "not found. failed\n");
            return CMD_WARNING;
        case WEB_RUNNING:
            vty_out(vty,"is running. failed\n");
            return CMD_WARNING;
        default:
            break;
    }

    if((ret = ac_manage_web_edit(dbus_connection_dcli[dslot]->dcli_dbus_connection, (void *)name, HOST_DEL)) != WEB_SUCCESS)
    {
        vty_out(vty, "failed.\n");
        return CMD_WARNING;
    }

	return CMD_SUCCESS;
}

DEFUN(set_interval_portal_func,
      set_interval_portal_cmd,
      "add webportal WEBNAME (normal|wispr|wisprssl) A.B.C.D <1-65535> <1-16>",
      "add webportal normal|wispr|wisprssl\n" \
      "web service\n" \
      "web service name (alphanumeric and '_' only, maximize 10 characters)\n" \
      "type normal\n"\
      "type wispr\n"\
      "type wisprssl\n"\
      "IP address (e.g 192.168.1.1)\n"\
      "port \n" \
      "slot destination service enable"
     )
{
    struct webHostHead head;
    webHost vh;

    unsigned int dslot = 0;

    int i, ret;

    if(web_argv_valid_(argv[0], argv[1], argv[2], argv[3], &vh) != WEB_SUCCESS)
    {
        vty_out(vty,"invalid arguments. failed\n");
        return CMD_WARNING;
    }

    dslot = atoi(argv[4]);

    if(dbus_connection_dcli[dslot]->dcli_dbus_connection == NULL)
    {
        vty_out(vty,"invalid slot . failed\n");
        return CMD_WARNING;
    }

    switch(dcli_web_vhost_add_valid(vh, dslot))
    {
        case WEB_OVERMAX:
            vty_out(vty, "to much webservice. failed\n");
            return CMD_WARNING;

        case WEB_RUNNING:
            vty_out(vty, "is running. failed\n");
            return CMD_WARNING;

        case WEB_NAME_EXISIT:
            vty_out(vty, "service name exist. failed\n");
            return CMD_WARNING;

        case WEB_EXISIT:
            vty_out(vty, "service ip:port exist. failed\n");
            return CMD_WARNING;

        case WEB_PORTAL_ERROR:
            vty_out(vty, "portal slot conflict. failed\n");
            return CMD_WARNING;
            
        case WEB_IP_PORT_ERROR:
			vty_out(vty, "service ip:port error, can not be used. failed\n");
            return CMD_WARNING;
        default:
            break;
    }

    LOG("%s %s %d %d", vh.name, vh.address, vh.port, vh.type);

    if((ret = ac_manage_web_edit(dbus_connection_dcli[dslot]->dcli_dbus_connection, (void *)&vh, HOST_ADD)) != WEB_SUCCESS)
	{
		vty_out(vty, "failed.\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
}

DEFUN(delete_portal_config_func,
      delete_portal_config_cmd,
      "del webportal WEBNAME",
      "del webportal \n" \
	  "web portal name\n" \
      "web portal name \n" 
     )
{
    char *name = argv[0];
    unsigned int dslot = 0;
    int ret;
    if(web_name_valid(name, strlen(name)) == WEB_FAILURE)
    {
        vty_out(vty,"invalid arguments. failed\n");
        return WEB_SUCCESS;
    }

    if(dcli_web_vhost_del_valid(name, &dslot) != WEB_SUCCESS)
    {
        vty_out(vty, "not found. failed\n");
        return CMD_WARNING;
    }

    if((ret = ac_manage_web_edit(dbus_connection_dcli[dslot]->dcli_dbus_connection, (void *)name, HOST_DEL)) != WEB_SUCCESS)
    {
        vty_out(vty, "failed.\n");
        return CMD_WARNING;
    }

	return CMD_SUCCESS;
}

DEFUN(enable_interval_portal_service_func,
      enable_interval_portal_service_cmd,
      "service webportal enable ",
      "config webportal service \n" \
      "webportal service \n" \
      "start webportal service \n"
     )
{
    struct webInfoHead infohead;
    
    LINK_INIT(&infohead);
    dcli_web_vhost_show(&infohead);
    
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

	        ret = ac_manage_web_conf(dbus_connection_dcli[dslot]->dcli_dbus_connection, PORTAL_START);

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
                            dcli_communicate_pfm_by_dbus(0, 0, 6, 
                                    in->ifname, 0, vh->port, info->slotid, "all", buf, in->slot);
                        }
                    }
                }
            }
        }
    }
    
    web_list_flush(&infohead);
    
    if(flag) {
		vty_out(vty, "webportal service not config. falied\n");
        return CMD_SUCCESS;
	}
	
    if (0 < webportal_enable) {
    	vty_out(vty, "webportal service is running.\n");
        return CMD_SUCCESS;
    }

    if(ret == WEB_FAILURE) {
		vty_out(vty, "webportal service enable falied, ip or port error\n");
    }
	
    return CMD_SUCCESS;
}

DEFUN(disable_interval_portal_service_func,
      disable_interval_portal_service_cmd,
      "service webportal disable",
      "config webportal service \n" \
      "webportal service \n" \
      "stop webportal service \n"
     )
{
    struct webInfoHead infohead;
    
    LINK_INIT(&infohead);
    dcli_web_vhost_show(&infohead);
    
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

            ret = ac_manage_web_conf(dbus_connection_dcli[dslot]->dcli_dbus_connection, PORTAL_STOP);

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
                            dcli_communicate_pfm_by_dbus(1, 0, 6, 
                                    in->ifname, 0, vh->port, info->slotid, "all", buf, in->slot);
                        }
                    }
                }
            }
        }
    }
    
    web_list_flush(&infohead);

    if(flag) {
        vty_out(vty, "webportal service not config. falied\n");
        return CMD_SUCCESS;
	}
	
    if (0 < webportal_disable) {
    	vty_out(vty, "webportal service is not running.");
        return CMD_SUCCESS;
    }

    if(ret == WEB_FAILURE) {
		vty_out(vty, "webportal service disable falied, ip or port error");
    }

    return CMD_SUCCESS;
}

DEFUN(show_interval_portalservice_info_func,
      show_interval_portalservice_info_cmd,
      "show webportal configuration",
      SHOW_STR
      "Internal portal config\n"

     )
{  
	struct webInfoHead infohead;
	
	LINK_INIT(&infohead);
	dcli_web_vhost_show(&infohead);
	
	webInfoPtr info;
	webHostPtr vh;
    webIfPtr in;
	
	LINK_FOREACH(info, &infohead, entries)
	{
		if(info->portal_flag)
		{

			if(info->server_stat & PORTAL_SERVICE_ENABLE)
				vty_out(vty, "status: running");
			else
                vty_out(vty, "status: not running");

            vty_out(vty, "\tslot %d\n",  info->slotid);

            vty_out(vty,"%-8s%-11s%-16s%-6s%-s\n","type","name","ip","port","ifname");
            vty_out(vty,"--------------------------------------------------------------------\n");
            LINK_FOREACH(vh, &(info->head), entries)
            {
                if(vh->type == PORTAL_HTTP_SERVICE) {
                    vty_out(vty, "%-8s","wispr");
                } else if (vh->type == PORTAL_HTTPS_SERVICE) {
                    vty_out(vty, "%-8s", "wisprssl");
                } else if(vh->type == PORTAL_SERVICE) {
                    vty_out(vty, "%-8s", "normal");
                } else {
					continue;
                }
                vty_out(vty,"%-11s%-16s%-6d",vh->name, vh->address, vh->port); 

                if(!LINK_EMPTY(&vh->head))
                {
                    LINK_FOREACH(in, &(vh->head), entries)
                    {
                        vty_out(vty, "%s ", in->ifname);
                    }
                }
                vty_out(vty, "\n");
            }
            vty_out(vty,"--------------------------------------------------------------------\n");
		}
	}
	web_list_flush(&infohead);	
    return CMD_SUCCESS;
}


DEFUN(download_internal_portal_func,
      download_internal_portal_cmd,
      "download A.B.C.D PATH USRNAME PASSWD <1-16>",
      "download portal pages (only htm,html,js,css,jpg,png,bmp allowed)\n"
      "IP address (as ftp://A.B.C.D)\n"
      "path\n"
      "username\n"
      "passwdword\n")
{
	/*
	 *	argv[0]: A.B.C.D	argv[1]:SRC 	argv[2]:USRNAME 	argv[3]:PASSWD		argv[4]: SLOT 
	 */
	struct webDirHead head;
	webDirPtr pd;
	int id, ret, count, i, sum = 0;

	id = atoi(argv[4]);
	if(dbus_connection_dcli[id]->dcli_dbus_connection == NULL)
	{
		vty_out(vty,"invalid slot. failed\n");
		return CMD_WARNING;
	}
	
	LINK_INIT(&head);
	dcli_web_get_dir(&head, &sum);

	if(sum != 0)
	{
		LINK_FOREACH(pd, &head, entries)
		{	
			if(pd->slot == id)
			{
				if(pd->count >= MAX_INTERNAL_PORTAL)
				{
					vty_out(vty, "too much pages. failed\n");
					web_dir_flush(&head);
					return CMD_WARNING;
				}
				else
				{
					for(i = 0; i < pd->count; i++)
					{
						if(!strcmp(pd->dir[i], argv[1]))
						{
							vty_out(vty, "exist. failed\n");
							web_dir_flush(&head);
							return CMD_WARNING;
						}
					}
				}
			}
		}
	}
	web_dir_flush(&head);

	char *p[] = {argv[0], argv[1], argv[2], argv[3], "end"};

	ret = ac_manage_web_download(dbus_connection_dcli[id]->dcli_dbus_connection, p);

	switch(ret)
	{
		case WEB_SUCCESS:
			break;
		case WEB_CONN_REF:
			vty_out(vty, "error. failed\n");
			break;
		case WEB_DIR_EMPRY:
			vty_out(vty, "empty. failed\n");
			break;
		case WEB_FAILURE:
			vty_out(vty, "failed\n");
			break;		
	}
	return CMD_SUCCESS;
	
}

DEFUN(show_internal_portal_func,
      show_internal_portal_cmd,
      "show pages",
      SHOW_STR
      "download portal pages\n")
{
	struct webDirHead head;

	webDirPtr pd;
	int count, i, sum = 0;
	
	LINK_INIT(&head);
	dcli_web_get_dir(&head, &sum);

	if(sum != 0)
	{
		vty_out(vty, "%s  %s\n", "slot", "pages");
	    vty_out(vty,"---------------------------------------\n");
		LINK_FOREACH(pd, &head, entries)
		{	
			if(pd->count > 0)
			{
				vty_out(vty, "  %d ", pd->slot);
				for(i = 0; i < pd->count; i++)
				{
					vty_out(vty, "  %-6s", pd->dir[i]);
				}
				vty_out(vty, "\n");
			}
		}
	    vty_out(vty,"---------------------------------------\n");
	}
	else
	{
		vty_out(vty, "no pages.\n");
	}
	web_dir_flush(&head);
	return CMD_SUCCESS;
}

DEFUN(del_internal_portal_func,
      del_internal_portal_cmd,
      "del pages DIRECTORY <1-16>",
      "del pages\n"
      "del pages\n"
      "path\n"
      "slot\n")
{
	struct webDirHead head;
	webDirPtr pd;
	int i, id, ret, sum = 0;
	
	id = atoi(argv[1]);
	if(!dbus_connection_dcli[id]->dcli_dbus_connection)
	{
		vty_out(vty,"invalid slot. failed\n");
		return CMD_WARNING;
		
	}

	LINK_INIT(&head);
	dcli_web_get_dir(&head, &sum);

	if(sum != 0)
	{
		LINK_FOREACH(pd, &head, entries)
		{	
			if(pd->slot == id)
			{
				for(i = 0; i < pd->count; i++)
				{
					if(!strcmp(pd->dir[i],argv[0]))
					{
						ret = ac_manage_web_del_pages(dbus_connection_dcli[id]->dcli_dbus_connection, argv[0]);
						if(ret != WEB_SUCCESS)
						{
							vty_out(vty, "failed\n");
						}
						web_dir_flush(&head);
						return CMD_SUCCESS;
					}
				}
			}
		}
	}
	vty_out(vty, "not found. failed\n");
	web_dir_flush(&head);
	return CMD_WARNING;
}

int dcli_webservice_show_running_config(struct vty* vty)
{
	vtysh_add_show_string("!web service  section\n" );
	vtysh_add_show_string(" config web service\n" );

    char command[1024];

    struct webInfoHead infohead;
    
    LINK_INIT(&infohead);
    dcli_web_vhost_show(&infohead);
    
    webInfoPtr info;
    webHostPtr vh;
    webIfPtr in;
    static int webportal_enable_num = 0;
    static int webconfig_enable_num = 0;
    
    LINK_FOREACH(info, &infohead, entries)
    {
        if(info->portal_flag)
        {
            LINK_FOREACH(vh, &(info->head), entries)
            {
                if(vh->type == PORTAL_HTTPS_SERVICE || vh->type == PORTAL_HTTP_SERVICE || vh->type == PORTAL_SERVICE)
                {
                    memset(command, 0 ,sizeof(command));
                    if(vh->type == PORTAL_HTTP_SERVICE)
                    {
                        sprintf(command, " add webportal %s wispr %s %d %d\n",vh->name, vh->address, vh->port, info->slotid);
                    }
                    else if(vh->type == PORTAL_HTTPS_SERVICE)
                    {
                        sprintf(command, " add webportal %s wisprssl %s %d %d\n",vh->name, vh->address, vh->port, info->slotid);
                    }
                    else if(vh->type == PORTAL_SERVICE)
                    {
                        sprintf(command, " add webportal %s normal %s %d %d\n",vh->name, vh->address, vh->port, info->slotid);
                    }

                    vtysh_add_show_string(command);

                    if(!LINK_EMPTY(&vh->head))
                    {
                        LINK_FOREACH(in, &(vh->head), entries)
                        {
                            memset(command, 0, sizeof(command));
                            sprintf(command, " add webforward %s %s\n", in->name, in->ifname);
                            vtysh_add_show_string(command);
                        }
                    }
                }
            }

            if(info->server_stat & PORTAL_SERVICE_ENABLE) {
				webportal_enable_num++;
			}
        }
	}
	if (0 < webportal_enable_num) {
		vtysh_add_show_string(" service webportal enable\n");
	}
	
    #ifdef __WITH_AUTEWARE_WEB
    LINK_FOREACH(info, &infohead, entries)
    {
        if(info->slotid == master_slot_id)
        {
            LINK_FOREACH(vh, &(info->head), entries)
            {
                if(vh->type == HTTP_SERVICE || vh->type == HTTPS_SERVICE)
                {
                    memset(command, 0 ,sizeof(command));
                    if(vh->type == HTTP_SERVICE)
                    {
                        sprintf(command, " add webconfig  %s http %s %d\n",vh->name, vh->address, vh->port);
                    }
                    else if(vh->type == HTTPS_SERVICE)
                    {
                        sprintf(command, " add webconfig  %s https %s %d\n",vh->name, vh->address, vh->port);
                    }

                    vtysh_add_show_string(command);

                    if(!LINK_EMPTY(&vh->head))
                    {
                        LINK_FOREACH(in, &(vh->head), entries)
                        {
                            memset(command, 0, sizeof(command));
                            sprintf(command, " add webforward %s %s\n", in->name, in->ifname);
                            vtysh_add_show_string(command);
                        }
                    }
                }
            }

            if(info->server_stat & WEB_SERVICE_ENABLE) {
				webconfig_enable_num++;
            }
        }
    }
    #endif
	if (0 < webconfig_enable_num) {
        vtysh_add_show_string(" service webconfig enable\n");
	}
	vtysh_add_show_string( " exit\n" );
    web_list_flush(&infohead);  
    return CMD_SUCCESS;
}

void dcli_webservice_init(void)
{
    install_node( &web_service_node, dcli_webservice_show_running_config, "WEBSERVICE_NODE");
    install_default(WEBSERVICE_NODE);
	
#ifdef __WITH_AUTEWARE_WEB
    install_element(CONFIG_NODE, &conf_web_service_cmd);
    install_element(WEBSERVICE_NODE, &contrl_enable_webservice_cmd);
	install_element(WEBSERVICE_NODE, &contrl_disable_webservice_cmd);
    install_element(WEBSERVICE_NODE, &show_webservice_info_cmd);
    install_element(WEBSERVICE_NODE, &add_http_https_ip_port_cmd);
    install_element(WEBSERVICE_NODE, &delete_http_https_config_cmd);
#else
	install_element(CONFIG_NODE, &conf_web_service_cmd);
    install_element(HIDDENDEBUG_NODE, &contrl_enable_webservice_cmd);
	install_element(HIDDENDEBUG_NODE, &contrl_disable_webservice_cmd);
    install_element(HIDDENDEBUG_NODE, &show_webservice_info_cmd);
    install_element(HIDDENDEBUG_NODE, &add_http_https_ip_port_cmd);
    install_element(HIDDENDEBUG_NODE, &delete_http_https_config_cmd);

    install_element(HIDDENDEBUG_NODE, &add_web_forword_cmd);
    install_element(HIDDENDEBUG_NODE, &del_web_forword_cmd);
#endif
    install_element(WEBSERVICE_NODE, &set_interval_portal_cmd);
    install_element(WEBSERVICE_NODE, &show_interval_portalservice_info_cmd);
    install_element(WEBSERVICE_NODE, &enable_interval_portal_service_cmd);
    install_element(WEBSERVICE_NODE, &disable_interval_portal_service_cmd);
    install_element(WEBSERVICE_NODE, &delete_portal_config_cmd);

    install_element(WEBSERVICE_NODE, &download_internal_portal_cmd);
    install_element(WEBSERVICE_NODE, &show_internal_portal_cmd);
    install_element(WEBSERVICE_NODE, &del_internal_portal_cmd);
    
    install_element(WEBSERVICE_NODE, &add_web_forword_cmd);
    install_element(WEBSERVICE_NODE, &del_web_forword_cmd);
}

#ifdef __cplusplus
}
#endif
