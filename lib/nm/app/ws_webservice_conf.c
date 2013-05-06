#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <syslog.h>
#include <arpa/inet.h>
#include <dbus/dbus.h>

#include "ws_webservice_conf.h"

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

	if(NULL == ipaddress || !strcmp(ipaddress,"0.0.0.0"))
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