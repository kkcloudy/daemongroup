/*******************************************************************************
Copyright (C) Autelan Technology


This software file is owned and distributed by Autelan Technology 
********************************************************************************


THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR 
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************************
* subagent_plugin.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
*
* DESCRIPTION:
* Login ac and wtp infomation in suagent.
*
*
*******************************************************************************/
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <signal.h>
#include <syslog.h>
#include "mibs_public.h"
#include <pthread.h>
#include "ws_init_dbus.h"
#include "ws_dbus_list_interface.h"
#include "ac_manage_def.h"
#include "ws_snmpd_engine.h"
#include "ws_snmpd_manual.h"
#include "ac_manage_interface.h"
#include "netsnmp_dbus.h"

#include "ws_public.h"

#include "netsnmp_private_interface.h"

int log_contrl = 0;
int cache_time = 0;
extern FILE *cgiOut ;
void *snmp_dbus_thread();

static void
snmp_config_log_debug(void) {
    
    unsigned int debugLevel = 0;
    ac_manage_show_snmp_log_debug(ccgi_dbus_connection, &debugLevel);

    netsnmp_config_log_debug(debugLevel);

    return ;
}

void init_subagent_plugin(void)
{
	// int syslog = 0; /* change this if you want to use syslog */
	syslog(LOG_NOTICE, "autelan-plugin init");

    remove(SNMP_THREAD_INFO_PATH);
    snmp_pid_write(SNMP_THREAD_INFO_PATH, "Snmp Thread Info\nsnmp subagent thread");
	
	int ret = 0;
	int ret_remote = 0;
	int switch_remote = 0;
	pthread_attr_t attr;
	int s = PTHREAD_CREATE_DETACHED;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,s);

	ccgi_local_dbus_init();
	snmp_config_log_debug();
	
	DcliWInit();
	snmpd_dbus_init();

	snmp_tipc_session_init();
	
	pthread_t thread_dbus;
	ret = pthread_create (&thread_dbus, NULL, (void *)netsnmp_dbus_thread, NULL );	
	if (ret != 0 )
	{
	  syslog(LOG_NOTICE,"An error occurs when create dbus thread!");
	}
	else
	{
		  syslog(LOG_NOTICE,"create dbus thread success!");
	}

    if(distributed_flag) {
        pthread_t thread_dbus_remote;
        ret_remote = pthread_create (&thread_dbus_remote, NULL, (void *)tipc_dbus_connection_pthread, NULL );
        if(ret_remote) {
        	   syslog(LOG_NOTICE, "An error occurs when create tipc dbus connection thread!\n" );
        }
        else {
        	 syslog(LOG_NOTICE,"create tipc dbus connection thread success!\n");
        }
    } 
    
	STSNMPSysInfo snmp_info;
	int manage_ret = AC_MANAGE_SUCCESS;
	manage_ret = ac_manage_show_snmp_base_info(ccgi_dbus_connection, &snmp_info);
	if(AC_MANAGE_SUCCESS == manage_ret) {
		cache_time = snmp_info.cache_time;
	}
	else {
		cache_time = 300;
		syslog(LOG_NOTICE, "ac_manage_show_snmp_base_info failed!, set cache_time 300s!\n");
	}
	
	init_autelanWtpGroup();
	init_autelanAcGroup();
}

void deinit_subagent_plugin(void)
{
	syslog(LOG_NOTICE, "subagent-plugin deinit");
	return ;
}
