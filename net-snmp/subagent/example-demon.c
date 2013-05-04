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
* example-demon.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
*
* DESCRIPTION:
* subagent's enter function.
*
*
*******************************************************************************/

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <signal.h>
#include "mibs_public.h"
//#include "autelanAcGroup.h"
//#include "autelanWtpGroup.h"
//#include "autelanWlanGroup.h"

/********
ws_sysinfo.o中引用了 cgiout，需要将其去掉。
*****/
#include <stdio.h>
FILE *cgiOut = NULL;

void open_cgiout()
{
	cgiOut = fopen( "/dev/null", "rw" );
}

void close_cgiout()
{
	if(  NULL != cgiOut )
	{
		fclose( cgiOut );
		cgiOut = NULL;
	}
}

/********end*****/
static int keep_running;

RETSIGTYPE
stop_server(int a) {
    keep_running = 0;
}

/*******************************
记录最后一次跟新snmp的时间。
************************************/
static char latest_time[256]="never";
static unsigned int timelist[3]={0};
void refresh_last_time()
{
//每次get的时候，都会有三个连续的pdu的包，这里需要判断，最后一次才能跟新时间。
//只有在timelist的最后3个中的时间时间差小于20tick的时候，才进行跟新。
	FILE *fp = NULL;
	char time_tick[64];
	int i;
	char *temp;
#define GET_TIMETICK_CMD	"cat /proc/stat | sed \"1,1d\" | sed \"2,100d\" | sed \"s/ /\\n/g\" | sed \"1,1d\" | awk 'BEGIN{total=0}{total += $1}END{print total}'"

	
	GET_CMD_STDOUT(time_tick,sizeof(time_tick),GET_TIMETICK_CMD);
	
	for(i=0;i<sizeof(timelist)/sizeof(timelist[0])-1;timelist[i]=timelist[i+1],i++)
	timelist[2] = strtoul( time_tick, &temp, 10 );
	if( timelist[2] - timelist[0] <= 20 )//0.2s  是一个经验值，
	{
		GET_CMD_STDOUT(latest_time,sizeof(latest_time),"date");
	}
	return;
}

char *get_latest_time()
{
	return 	latest_time;
}
/**********************************
************************************/
int
main (int argc, char **argv) {
  int agentx_subagent=1; /* change this if you want to be a SNMP master agent */
  int background = 0; /* change this if you want to run in the background */
  int syslog = 0; /* change this if you want to use syslog */
  DcliWInit();
  snmpd_dbus_init();
  /* print log errors to syslog or stderr */
  open_cgiout();
  
  if (syslog)
    snmp_enable_calllog();
  else
    snmp_enable_stderrlog();

  /* we're an agentx subagent? */
  if (agentx_subagent) {
    /* make us a agentx client. */
    netsnmp_ds_set_boolean(NETSNMP_DS_APPLICATION_ID, NETSNMP_DS_AGENT_ROLE, 1);
  }

  /* run in background, if requested */
  if (background && netsnmp_daemonize(1, !syslog))
      exit(1);

  /* initialize tcpip, if necessary */
  SOCK_STARTUP;

  /* initialize the agent library */
  init_agent("example-demon");

  /* initialize mib code here */

  /* mib code: init_nstAgentSubagentObject from nstAgentSubagentObject.C */
init_autelanWtpGroup();
init_autelanAcGroup();
//init_autelanWlanGroup();

  /* initialize vacm/usm access control  */
  if (!agentx_subagent) {
      init_vacm_vars();
      init_usmUser();
  }

  /* example-demon will be used to read example-demon.conf files. */
  init_snmp("example-demon");

 /* If we're going to be a snmp master agent, initial the ports */
  if (!agentx_subagent)
    init_master_agent();  /* open the port to listen on (defaults to udp:161) */

  /* In case we recevie a request to stop (kill -TERM or kill -INT) */
  keep_running = 1;
  signal(SIGTERM, stop_server);
  signal(SIGINT, stop_server);

 // snmp_log(LOG_INFO,"autelan-subagent is up and running.\n");

  /* your main loop here... */
  while(keep_running) {
    /* if you use select(), see snmp_select_info() in snmp_api(3) */
    /*     --- OR ---  */
    agent_check_and_process(1); /* 0 == don't block */
    
    //跟新最后一次访问snmp的时间。 add  by  shaojunwu 2009-3-25 11:30:14
    refresh_last_time();
  }

  /* at shutdown time */
  snmp_shutdown("example-demon");
  SOCK_CLEANUP;

	close_cgiout();
  return 0;
}

