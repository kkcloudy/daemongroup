#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <dbus/dbus.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <syslog.h>
#include <sys/wait.h>
#include <unistd.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>
#include "ws_dbus_def.h"
#include "ac_manage_def.h"
#include "ws_webservice_conf.h"
#include "ws_dbus_list_interface.h"
#include "ac_manage_public.h"
#include "ac_manage_apache_config.h"

static unsigned int web_por_flag;
static unsigned int web_ser_stat;
static unsigned int web_vhost_sum;

static struct webHostHead head;

int web_host_add(const char *name, const char *address, int port, int type)
{ 
	webHostPtr vh,t;
	vh = (webHostPtr)malloc(WEB_WEBHOST_SIZE);
    vh->name = strdup(name);
   	vh->address = strdup(address);
    vh->port = port; 
	vh->type = type;
    vh->count = 0;
    LINK_INIT(&vh->head);
	LINK_INSERT_HEAD(&head, vh, entries);

	if(type == PORTAL_HTTP_SERVICE || type == PORTAL_HTTPS_SERVICE || type == PORTAL_SERVICE)
	{
		web_por_flag = 1;
	}

    web_vhost_sum++;

    LOG("name:%s address:%s port:%d type:%d", name, address, port, type);
    LOG("sum: %d", web_vhost_sum);

	return WEB_SUCCESS;
}

int web_host_del(const char *name)
{
    LOG("%s", name);
    webHostPtr vh;
    webIfPtr in = NULL;
	LINK_FOREACH(vh, &head, entries)
	{
        if(!strcmp(vh->name, name))
        {
			LINK_REMOVE(vh, entries);
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
	}

    web_vhost_sum--;
    if(web_vhost_sum == 0)
    {
        web_por_flag = 0;
    }

    return WEB_SUCCESS;
}

int web_interface_add(const char *name, const char *ifname, unsigned int slot)
{
    webHostPtr vh = NULL;
    webIfPtr in = NULL;
    unsigned int flag = 1;

    LINK_FOREACH(vh, &head, entries)
    {
        if(!strcmp(vh->name, name))
        {
            vh->count++;
            in = (webIfPtr)malloc(WEB_WEBIFNAME_SIZE);
            in->ifname = strdup(ifname);
            in->name = strdup(name);
            in->slot = slot;
            in->opt = 0;
            LOG("%s %s %d", in->name, in->ifname, slot);
            LINK_INSERT_HEAD(&vh->head, in, entries);
            flag = 0;
        }
    }
    if(flag)
    {
        LOG("not found");
        return WEB_NOTFOUND;
    }

    return WEB_SUCCESS;
}

int web_interface_del(const char *name, const char *ifname)
{
    LOG("%s %s", name, ifname);
    webHostPtr vh = NULL;
    webIfPtr in = NULL;
    unsigned int flag = 1;

    LINK_FOREACH(vh, &head, entries)
    {
        if(!strcmp(vh->name, name))
        {
            if(!LINK_EMPTY(&(vh->head)))
            {
                LINK_FOREACH(in, &(vh->head), entries)
                {
                    if(!strcmp(in->ifname, ifname))
                    {
                        free(in->name);
                        free(in->ifname);
                        vh->count--;
			            LINK_REMOVE(in, entries);
                    }
                }
            }
        }
    }
    return WEB_SUCCESS;
}

int web_host_delete(int type)
{
	webHostPtr t;

	LINK_FOREACH(t, &head, entries)
	{
		if(t->type == type)
		{	
			LINK_REMOVE(t, entries);
			free(t->address);
			free(t);

			if(type == PORTAL_HTTP_SERVICE)
			{
				web_por_flag = 0;
			}
			return WEB_SUCCESS;
		}
	}
	return WEB_NOTFOUND;
}

int web_host_show(struct webHostHead *fhead, unsigned int *n1, unsigned int *n2)
{
	webHostPtr vh = NULL;
    webIfPtr in = NULL;
	
	fhead->lh_first = head.lh_first;

	*n1 = web_ser_stat;
	*n2 = web_por_flag;

	return web_vhost_sum;
}

void web_host_buf(char *command, webHostPtr vh)
{
    char tmp[8] = {0};
    strcat(command," ");
    strcat(command,vh->address);
    strcat(command,":"); 
    sprintf(tmp,"%d",vh->port);
    strcat(command,tmp);
}

int web_host_site(const char *path, const char *buf , int type)
{
	if(!strcmp(buf,""))
	{
		if(access(path, F_OK) == 0)
		{
			if(unlink(path) != -1)
			LOG("unlink %s success", path);
		}
		return WEB_SUCCESS;
	}
	 
	char *available = NULL;
	FILE *fp;

	if((fp = fopen(path, "w")) == NULL)
	{
		LOG("open %s failed", path);
		return WEB_FAILURE;
	}

	switch(type)
	{
		case HTTP_SERVICE:
			available = SITES_AVALIB_DEFAULT;
			break;
		case HTTPS_SERVICE:
			available = SITES_AVALIB_SSLDEF;
			break;
        case PORTAL_SERVICE:
            available = SITES_AVALIB_PORTAL;
            break;
		case PORTAL_HTTP_SERVICE:
			available = SITES_AVALIB_PORTALNORAMAL;
			break;
        case PORTAL_HTTPS_SERVICE:
			available = SITES_AVALIB_PORTALSSL;
            break;
		default:
			break;	
	}
	
	fprintf(fp,"Listen %s\n",buf);		
	fprintf(fp,"<VirtualHost%s>\n",buf);	
	fprintf(fp,"Include \"%s\"\n",available);	
	fprintf(fp,"</VirtualHost>");

	fclose(fp);
	
	LOG("%s", buf);
	return WEB_SUCCESS;
}

int web_host_conf(void)
{
    LOG("-----Begin-----");
	char buf_http[128] = {0};
	char buf_https[128] = {0};
    char buf_portal[128] = {0};
	char buf_portal_normal[128] = {0};
	char buf_portal_ssl[128] = {0};

	webHostPtr vh;
	
	LINK_FOREACH(vh, &head, entries)
	{
	
		switch(vh->type)
		{
			case HTTP_SERVICE:
				web_host_buf(buf_http, vh);
				break;
			case HTTPS_SERVICE:
				web_host_buf(buf_https, vh);
				break;
            case PORTAL_SERVICE:
                web_host_buf(buf_portal, vh);
                break;
			case PORTAL_HTTP_SERVICE:
				web_host_buf(buf_portal_normal, vh);
				break;
			case PORTAL_HTTPS_SERVICE:
				web_host_buf(buf_portal_ssl, vh);
				break;
			default:
				break;	
		}
	}
	
	if(WEB_FAILURE == web_host_site(SITES_ENABEL_DEFAULT, buf_http, HTTP_SERVICE) ||
		WEB_FAILURE == web_host_site(SITES_ENABEL_SSLDEF, buf_https, HTTPS_SERVICE) ||
		WEB_FAILURE == web_host_site(SITES_ENABEL_PORTAL, buf_portal, PORTAL_SERVICE) || 
		WEB_FAILURE == web_host_site(SITES_ENABEL_PORTALNORMAL, buf_portal_normal, PORTAL_HTTP_SERVICE) ||
		WEB_FAILURE == web_host_site(SITES_ENABEL_PORTALSSL, buf_portal_ssl, PORTAL_HTTPS_SERVICE ))
    {
        LOG("-----End FAILUE-----");
		return WEB_FAILURE;
    }
	else
    {
        LOG("-----End SUCCESS-----");
		return WEB_SUCCESS;
    }
}

int web_host_command(const char *command)
{
	LOG("%s", command);
	return WEXITSTATUS(system(command));	
}

int 
web_host_service(int type)
{
	web_host_conf();
	
	char command[128] = {0};
	int ret;

	LOG("web_ser_stat : %d", web_ser_stat);
	
	switch(type)
	{
		case WEB_START:
			if(web_ser_stat&WEB_SERVICE_ENABLE)
				return WEB_ENABLE;
			else
			{
				if(web_ser_stat&PORTAL_SERVICE_ENABLE)
					strcat(command, "sudo /etc/init.d/apache2 restart 2>/dev/null;");
				else
					strcat(command, "sudo /etc/init.d/apache2 start 2>/dev/null;");
			}

			if(0 != web_host_command(command))
			{
				return WEB_FAILURE;	
			}

			web_ser_stat |= WEB_SERVICE_ENABLE;
		break;
		case WEB_STOP:
			if(!(web_ser_stat&WEB_SERVICE_ENABLE))
				return WEB_DISABLE;
			else
			{
				if(web_ser_stat&PORTAL_SERVICE_ENABLE)
					strcat(command, "sudo /etc/init.d/apache2 restart 2>/dev/null;");
				else
					strcat(command, "sudo /etc/init.d/apache2 stop 2>/dev/null;");	
			}
			if(0 != web_host_command(command))
			{
				return WEB_FAILURE;	
			}

			web_ser_stat &= ~WEB_SERVICE_ENABLE;
		break;
		case PORTAL_START:
			if(web_ser_stat&PORTAL_SERVICE_ENABLE)
				return WEB_ENABLE;
			else
			{
				if(web_ser_stat&WEB_SERVICE_ENABLE)
					strcat(command, "sudo /etc/init.d/apache2 restart 2>/dev/null;");
				else
					strcat(command, "sudo /etc/init.d/apache2 start 2>/dev/null;");
			}

			if(0 != web_host_command(command))
			{
				return WEB_FAILURE;	
			}

			web_ser_stat |= PORTAL_SERVICE_ENABLE;
		break;
		case PORTAL_STOP:
			if(!(web_ser_stat&PORTAL_SERVICE_ENABLE))
				return WEB_DISABLE;
			else
			{
				if(web_ser_stat&WEB_SERVICE_ENABLE)
					strcat(command, "sudo /etc/init.d/apache2 restart 2>/dev/null;");
				else
					strcat(command, "sudo /etc/init.d/apache2 stop 2>/dev/null;");	
			}

			if(0 != web_host_command(command))
			{
				return WEB_FAILURE;	
			}

			web_ser_stat &= ~PORTAL_SERVICE_ENABLE;
		break;
		default:
            _exit(0);
		break;
	}
	
	return WEB_SUCCESS;
}


int web_dir_download(const char *addr, const char *path, const char *usr, const char *pass)
{
	
	int ret;
	
	char cmd[256];
	memset(cmd, 0, 256);
	sprintf(cmd, "sudo ftpbatch.sh %s %s %s %s", addr, path, usr, pass);
	syslog(LOG_DEBUG, "%s", cmd);
	ret = WEXITSTATUS(system(cmd));
	
	switch(ret)
	{
		case 0:
			return WEB_SUCCESS;
			break;
		case 1:
			return WEB_CONN_REF;
			break;
		case 3:
			return WEB_DIR_EMPRY;
			break;
		default:
			return WEB_FAILURE;
			break;		
	}

}

void web_dir_show(char **dir, int *s)
{
    int count = 0;
    DIR *dp;
    struct dirent *entry;
    struct stat statbuf;
    if((dp = opendir("/opt/eag/www")) == NULL) {
        return;
    }
    chdir("/opt/eag/www");
    while((entry = readdir(dp)) != NULL) {
        if( -1 == lstat(entry->d_name,&statbuf)){
			closedir(dp);
			return;
		};
        if(S_ISDIR(statbuf.st_mode)) {
            /* Found a directory, but ignore . and .. */
            if(strcmp(".",entry->d_name) == 0 || strcmp("..",entry->d_name) == 0)
                continue;
            *dir++ = strdup(entry->d_name);
            count++;
        }
    }
    chdir("..");
    closedir(dp);
    *s = count;
    return;
}

int web_dir_del(const char *dir)
{
	char command[128];
	int ret, stat;
	
	sprintf(command,"sudo ftpdealsd.sh %s >/dev/null",dir);
	
	stat = system(command);
    ret = WEXITSTATUS(stat);	

	if(ret == 0)
		return WEB_SUCCESS;
	else
		return WEB_FAILURE;
	
	return 0;
}


