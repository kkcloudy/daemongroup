#ifdef __cplusplus
extern "C"
{
#endif
#include <zebra.h>
#include <dbus/dbus.h>
#include <stdlib.h>
#include <sysdef/npd_sysdef.h>
#include <dbus/npd/npd_dbus_def.h>
#include <syslog.h>
#include "dcli_main.h"
#include "vty.h"
#include "command.h"
#include "if.h"
//#include "sysdef/returncode.h"
#include "dcli_domain.h"
#include "drp_interface.h"
#include "drp_def.h"
#include <sys/wait.h>
#include <ctype.h>
#include <stdio.h>

/*syslog  level*/
#define LOG_EMERG_MASK			(LOG_MASK(LOG_EMERG))
#define LOG_ALERT_MASK			(LOG_MASK(LOG_ALERT))
#define LOG_CRIT_MASK			(LOG_MASK(LOG_CRIT))
#define LOG_ERR_MASK			(LOG_MASK(LOG_ERR))
#define LOG_WARNING_MASK		(LOG_MASK(LOG_WARNING))
#define LOG_NOTICE_MASK			(LOG_MASK(LOG_NOTICE))
#define LOG_INFO_MASK			(LOG_MASK(LOG_INFO))
#define LOG_DEBUG_MASK			(LOG_MASK(LOG_DEBUG))
#define LOG_USER_MASK			(LOG_MASK(LOG_USER))

#define LOG_AT_LEAST_ALERT		LOG_EMERG_MASK|LOG_ALERT_MASK
#define LOG_AT_LEAST_CRIT		LOG_AT_LEAST_ALERT|LOG_CRIT_MASK
#define LOG_AT_LEAST_ERR		LOG_AT_LEAST_CRIT|LOG_ERR_MASK
#define LOG_AT_LEAST_WARNING	LOG_AT_LEAST_ERR|LOG_WARNING_MASK
#define LOG_AT_LEAST_NOTICE		LOG_AT_LEAST_WARNING|LOG_NOTICE_MASK
#define LOG_AT_LEAST_INFO		LOG_AT_LEAST_NOTICE|LOG_INFO_MASK
#define LOG_AT_LEAST_DEBUG		LOG_AT_LEAST_INFO|LOG_DEBUG_MASK
#define LOG_AT_LEAST_USER		LOG_AT_LEAST_DEBUG|LOG_USER_MASK

static DBusConnection *dcli_dbus_connection_curr ;

static char *
inet_ntoa_ex(unsigned long ip, char *ipstr, unsigned int buffsize)
{
	snprintf(ipstr, buffsize, "%lu.%lu.%lu.%lu", ip >> 24,
		 (ip & 0xff0000) >> 16, (ip & 0xff00) >> 8, (ip & 0xff));

	return ipstr;
}

#define DNS_CACHE_DISTRIBUTE_CONFIG() {\
	int slotid_H = HostSlotId;	\
	int ret_H = 0;	\
	ret_H = conf_drp_get_dbus_connect_params(&slotid_H);	\
	if (ret_H < 0){	\
		vty_out(vty, "eag get drp connection config error:%d\n",ret_H);	\
		return CMD_FAILURE;	\
	}	\
	ReInitDbusConnection(&dcli_dbus_connection_curr,slotid_H,distributFag);	\
}


struct cmd_node  dns_cache_node = 
{
	DNSCACHE_NODE,
	"%s(config-dns-cache)# "
};

DEFUN(conf_dns_cache_func,
	conf_dns_cache_cmd,
	"config dns-cache",
	CONFIG_STR
	"Config dns cache automatic\n"
)
{
         char cmd[128]={0};
	if (CONFIG_NODE == vty->node)
	{
		  vty->node = DNSCACHE_NODE;   	               
       		 if(access(XML_DOMAIN_PATH ,0)!=0)
            	 {
                            memset(cmd,0,128);
                            sprintf(cmd,"sudo cp %s  %s",XML_DOMAIN_D,XML_DOMAIN_PATH);
                            system(cmd);			 
                            memset(cmd,0,128);
                            sprintf(cmd,"sudo chmod 666 %s",XML_DOMAIN_PATH);
                            system(cmd);
       	         }
	}
	else
	{
		vty_out (vty, "Terminal mode change must under configure mode!\n");
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}

DEFUN(show_dns_cache_info_func,
	 show_dns_cache_info_cmd,
	"show dns-cache-informations",
	SHOW_STR
	"DNS cache configure\n"
)
{
	int i = 0, ret = 0;
	char ipstr[32] = {0};

	if (DNSCACHE_NODE != vty->node) 
	{
		vty_out (vty, "Terminal mode change must under dns-cache mode!\n");
		return CMD_WARNING;
	}
	DNS_CACHE_DISTRIBUTE_CONFIG();

	domain_cst domain_ret;
	memset (&domain_ret,0,sizeof(domain_ret));
	ret = conf_drp_show_domain_ip(dcli_dbus_connection_curr, &domain_ret);
	_drp_return_if_fail(0 == ret,ret,CMD_WARNING);

	vty_out(vty,"================================================================================\n");
	if( domain_ret.num != 0 )
	{
		for ( i = 0;i < domain_ret.num; i++ ){
			vty_out(vty,"domain name: ");
			vty_out(vty , "%s\t",domain_ret.domain[i].domain_name);
			vty_out(vty,"domain IP : ");
			inet_ntoa_ex(domain_ret.domain[i].ipaddr, ipstr,sizeof(ipstr));
			vty_out(vty , "%s\t",ipstr);			        		
			vty_out(vty,"IP index for the domain name: ");
			vty_out(vty , "%d\n",domain_ret.domain[i].index);
	        }
	}
	vty_out(vty,"===============================================================================\n");
	return CMD_SUCCESS;
}


/*add the ip and port for dns cache*/
DEFUN(add_dns_cache_domain_ip_func,
	add_dns_cache_domain_ip_cmd,
	"add dns_cache auto DOMAIN  IP IP_INDEX",
	"Add  domain name and corresponding IP address for portal white-list\n"
	"DNS  cache for portal white-list\n"
	"Automatic create by device,not config manually \n"\
	"Domain name for portal white-list \n"\
	"IP address for the Domain name in portal white-list\n"
	"IP index for this domain name in cache file\n"
)
{
	
    if (DNSCACHE_NODE != vty->node) 
	{
		vty_out (vty, "Terminal mode change must under configure mode!\n");
		return CMD_WARNING;
	}
	/*test whether  apache  service  enable*/
	int  ipval=-1;	   
	int ret =-1;
	char ip_name[128]={0};	
	unsigned int ip_addr = 0;
		
	//vty_out(vty, "input parameter 2  ip= %s !", argv[1]);
	DNS_CACHE_DISTRIBUTE_CONFIG();

	if( 0 != strcmp("0.0.0.0",argv[1]))
	{
		 ipval=parse_ip_check((char *)argv[1]);
		if(CMD_SUCCESS != ipval)
		{
			vty_out(vty, "%% Bad parameter : %s !", argv[1]);
			return CMD_WARNING;
		}	
	}

	domain_pt domain_conf;
	memset (&domain_conf,0,sizeof(domain_conf));
	strncpy((domain_conf.domain_name),(char *)argv[0],sizeof(domain_conf.domain_name));	
	inet_atoi((char *)argv[1], &ip_addr);
	domain_conf.ipaddr = ip_addr;
	domain_conf.index = atoi(argv[2]);
	ret = conf_drp_add_domain_ip(dcli_dbus_connection_curr,&domain_conf);
	_drp_return_if_fail(0 == ret,ret,CMD_WARNING);
	
	return CMD_SUCCESS;
}

DEFUN(add_dns_cache_domain_func,
	add_dns_cache_domain_cmd,
	"add dns_cache auto DOMAIN",
	"Add  domain name and corresponding IP address for portal white-list\n"
	"DNS  cache for portal white-list\n"
	"Automatic create by device,not config manually \n"\
	"Domain name for portal white-list \n"\
)
{
	
    if (DNSCACHE_NODE != vty->node) 
	{
		vty_out (vty, "Terminal mode change must under configure mode!\n");
		return CMD_WARNING;
	}

	int ret =-1;
	domain_pt domain_conf;
	DNS_CACHE_DISTRIBUTE_CONFIG();
	memset (&domain_conf,0,sizeof(domain_conf));
	
	strncpy((domain_conf.domain_name),(char *)argv[0],sizeof(domain_conf.domain_name));	
	ret = conf_drp_add_domain(dcli_dbus_connection_curr,&domain_conf);
	_drp_return_if_fail(0 == ret,ret,CMD_WARNING);
	
	return CMD_SUCCESS;
}

DEFUN(del_dns_cache_domain_func,
	del_dns_cache_domain_cmd,
	"del dns_cache auto DOMAIN",
	"del  domain name and corresponding IP address for portal white-list\n"
	"DNS  cache for portal white-list\n"
	"Automatic create by device,not config manually \n"\
	"Domain name for portal white-list \n"\
)
{
	
    if (DNSCACHE_NODE != vty->node) 
	{
		vty_out (vty, "Terminal mode change must under configure mode!\n");
		return CMD_WARNING;
	}

	int ret =-1;
	
	domain_pt domain_conf;
	DNS_CACHE_DISTRIBUTE_CONFIG();
	memset (&domain_conf,0,sizeof(domain_conf));
	strncpy((domain_conf.domain_name),(char *)argv[0],sizeof(domain_conf.domain_name));	
	ret = conf_drp_del_domain(dcli_dbus_connection_curr,&domain_conf);
	_drp_return_if_fail(0 == ret,ret,CMD_WARNING);
	
	return CMD_SUCCESS;
}


/*delete the  ip and port for http or https*/
DEFUN(delete_dns_cache_domain_ip_func,
	delete_dns_cache_domain_ip_cmd,
	"del dns_cache auto DOMAIN IP_INDEX",
	"delete  domain name and corresponding IP address for portal white-list\n"
	"DNS  cache for portal white-list\n"
	"Automatic create by device,not config manually \n"\
	"Domain name for portal white-list \n"\
	"IP index for this domain name in cache file\n"
)
{	
       if (DNSCACHE_NODE != vty->node) 
	{
		vty_out (vty, "Terminal mode change must under configure mode!\n");
		return CMD_WARNING;
	}

	//vty_out(vty, "input parameter 2  ip= %s !", argv[1]);
	int  ipval=-1;
	int ret = 0;

	domain_pt domain_conf;
	DNS_CACHE_DISTRIBUTE_CONFIG();
	memset (&domain_conf,0,sizeof(domain_conf));
	strncpy((domain_conf.domain_name),(char *)argv[0],sizeof(domain_conf.domain_name));	
	domain_conf.index = atoi(argv[1]);
	ret = conf_drp_del_domain_ip(dcli_dbus_connection_curr,&domain_conf);
	_drp_return_if_fail(0 == ret,ret,CMD_WARNING);
	
	return CMD_SUCCESS;
}

/*set drp distribute flag and slotid*/
DEFUN(set_dns_cache_params_func,
	set_dns_cache_params_cmd,
	"set dns_cache <1-16>",
	"set dns_cache\n"
	"set dns_cache instance\n"
	"set dns_cache slotid\n"
)
{	
	const char *conf_value = NULL;
	int distributeflag = 0;
	int slotid = 0;
	int ret = 0;
	
       if (DNSCACHE_NODE != vty->node) 
	{
		vty_out (vty, "Terminal mode change must under configure mode!\n");
		return CMD_WARNING;
	}

	slotid = strtol(argv[0],NULL,10);
	if (slotid > SLOT_MAX_NUM || slotid <= 0){
		vty_out (vty, "set dns_cache param SLOTID out of range error!\n");
		return CMD_FAILURE;
	}
	
	if ((ret = conf_drp_set_dbus_connect_params(slotid)) < 0){
		switch(ret){
			case -1:
				vty_out (vty, "set dns_cache param %s file open error!\n",DRP_CONFIG_FILE);
			case -2:
				vty_out (vty, "set dns_cache param error!\n");
			case -3:
				vty_out (vty, "set dns_cache param %s file write error!\n",DRP_CONFIG_FILE);
			
		}
		return CMD_FAILURE;
	}
	
	return CMD_SUCCESS;
}


DEFUN(set_drp_log_level_func,
			set_drp_log_level_cmd,
			"set drp log-debug (on|off)",
			"set drp info \n"
			"set drp log-debug \n"
			"set drp log-debug \n"
			"set drp log-debug on\n"
			"set drp log-debug off\n"
		)
{
	int iRet = DRP_RETURN_OK;
	int nlevel = LOG_AT_LEAST_INFO;
	
	const char *conf_value = NULL;
	conf_value = argv[0];	
	
	if (0 == strcmp( conf_value, "on") )
	{
		nlevel = LOG_AT_LEAST_DEBUG;
	}
	//vty_out(vty,"type %s state=%d level=%d\n", leveltype, value, nlevel);
	DNS_CACHE_DISTRIBUTE_CONFIG();
	iRet = conf_drp_dbus_method_log_debug(dcli_dbus_connection_curr, nlevel);
	_drp_return_if_fail(0 == iRet,iRet,CMD_FAILURE);

	return CMD_SUCCESS;
}


int dcli_dns_cache_show_running_config(struct vty* vty)
{    	
	int i = 0, ret = 0;
	char cmdstr[512] = {0};
	char ipstr[32] = {0};
	int slotid = HostSlotId;

	vtysh_add_show_string( "!DNS cache section\n" );
	
	ret = conf_drp_get_dbus_connect_params(&slotid);
	if (ret < 0){
		return CMD_SUCCESS;
	}
	
	ReInitDbusConnection(&dcli_dbus_connection_curr,slotid,distributFag);
	domain_cst domain_ret;
	memset (&domain_ret,0,sizeof(domain_ret));
	ret = conf_drp_show_domain_ip(dcli_dbus_connection_curr, &domain_ret);
	//_drp_return_if_fail(0 == ret,ret,CMD_WARNING);
	if (0 != ret){
		return CMD_SUCCESS;
	}

	if( (domain_ret.num != 0) ||(slotid != HostSlotId) )
	{
		vtysh_add_show_string("config dns-cache\n");
		if ( slotid != HostSlotId ){
			memset(cmdstr,0,sizeof(cmdstr));
			snprintf(cmdstr,sizeof(cmdstr)," set dns_cache %d\n",slotid);
			vtysh_add_show_string(cmdstr);
		}
		
		if (domain_ret.num != 0){
			for ( i = 0;i < domain_ret.num; i++ ){
				memset(cmdstr,0,sizeof(cmdstr));
				memset(ipstr,0, sizeof(ipstr));
				inet_ntoa_ex(domain_ret.domain[i].ipaddr, ipstr,sizeof(ipstr));
				snprintf(cmdstr,sizeof(cmdstr)-1," add dns_cache auto %s  %s  %d",
						domain_ret.domain[i].domain_name,\
							ipstr, (domain_ret.domain[i].index));
				vtysh_add_show_string(cmdstr);
			}
		}
		vtysh_add_show_string("exit\n");
	}
	
	 return CMD_SUCCESS;		
}

void dcli_dnscache_init
(
	void
)  
{
	install_node( &dns_cache_node, dcli_dns_cache_show_running_config, "DNSCACHE_NODE");
	install_default(DNSCACHE_NODE);

	install_element(CONFIG_NODE, &conf_dns_cache_cmd);	
	install_element(DNSCACHE_NODE, &show_dns_cache_info_cmd);
	
	install_element(DNSCACHE_NODE, &set_dns_cache_params_cmd);
	
	install_element(DNSCACHE_NODE, &add_dns_cache_domain_cmd); 
	install_element(DNSCACHE_NODE, &del_dns_cache_domain_cmd); 

	install_element(DNSCACHE_NODE, &add_dns_cache_domain_ip_cmd); 
	//install_element(DNSCACHE_NODE, &delete_dns_cache_domain_ip_cmd); 

	install_element(DNSCACHE_NODE, &set_drp_log_level_cmd); 
	
}

#ifdef __cplusplus
}
#endif

