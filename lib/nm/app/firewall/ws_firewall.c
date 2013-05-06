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
* ws_firewall.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* shaojw@autelan.com
*
* DESCRIPTION:
* function for web
*
*
***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#include "ac_manage_def.h"

#include "ws_firewall.h"


#define BUF_LEN 32
#define CMD_LEN 1024
//#define RULES_XML_FILE "/opt/www/ccgi-bin/fwRules.xml"
#define RULES_XML_FILE "/opt/services/option/firewall_option"
//#define CMD_START	"/opt/services/init/iptables_init start"
#define CMD_START	"/opt/services/init/iptables_init start firewall >/dev/null 2>&1"
//#define CMD_START	"/usr/bin/firewall_init.sh start_c >/dev/null 2>&1"
//#define CMD_STOP	"/opt/services/init/iptables_init stop"
#define CMD_STOP	"/opt/services/init/iptables_init stop firewall >/dev/null 2>&1"
//#define CMD_FLUSH	"/opt/services/init/iptables_init flush"
#define CMD_FLUSH	"/opt/services/init/iptables_init flush firewall >/dev/null 2>&1"
//"/opt/services/init/iptables_init flush firewall"
//#define CMD_SAVE_CONF "/opt/services/init/iptables_init save"
#define CMD_SAVE_CONF "/opt/services/init/iptables_init save firewall >/dev/null 2>&1"
#define CMD_STATUS_IS_START "read fwstatus < /opt/services/status/firewall_status.status && test $fwstatus = start"
#define IPTABLES_PATH	"/opt/bin/"

#define FW_FILTER_CHAIN "FW_FILTER"
#define FW_SNAT_CHAIN "FW_SNAT"
#define FW_DNAT_CHAIN "FW_DNAT"
#define TC_MANGLE_CHAIN "TRAFFIC_CONTROL"
#define TC_FILTER_CHAIN "TRAFFIC_CONTROL"


void
firewall_free_ruleDate(fwRule *rule) {
	if(NULL == rule) {
		return ;
	}
	
	MANAGE_FREE(rule->name);
	MANAGE_FREE(rule->comment);
	MANAGE_FREE(rule->ineth);
	MANAGE_FREE(rule->outeth);
	MANAGE_FREE(rule->srcadd);
	MANAGE_FREE(rule->dstadd);
	MANAGE_FREE(rule->sport);
	MANAGE_FREE(rule->dport);
	MANAGE_FREE(rule->connlimit);
	MANAGE_FREE(rule->tcpmss_var);
	MANAGE_FREE(rule->pkg_state);
	MANAGE_FREE(rule->string_filter);
	MANAGE_FREE(rule->natipadd);
	MANAGE_FREE(rule->natport);

	return ;
}


void 
firewall_free_array(fwRule **array, unsigned int count) {
	if(NULL == array || NULL == *array || 0 == count) {
		return ;
	}
	int i = 0;
	fwRule *temp_array = *array;
	
	for(i = 0; i < count; i++) {
		firewall_free_ruleDate(&temp_array[i]);
	}
	MANAGE_FREE(*array);

	return ;
}

//-j SNAT  -j DNAT 目标地址不能写掩码，但是能些范围，需要将ip/mask转化为ip range
void ipmask2iprange( char *src, char *dst )
{
	unsigned int m0,m1,m2,m3;
	unsigned int iIPbegin,iIPend;
	unsigned int iMask;
	char chIP[32] = "";
	char chMask[32] = "";
	char *temp;
	
	temp = strchr( src, '/' );
	strncpy( chIP, src, temp - src );
	strcpy( chMask, temp+1 );
	
	sscanf( chIP, "%u.%u.%u.%u", &m3,&m2,&m1,&m0 );
	iIPbegin = m3*256*256*256 + m2*256*256 + m1*256 + m0;
	
	sscanf( chMask, "%u.%u.%u.%u", &m3,&m2,&m1,&m0 );
	iMask = m3*256*256*256 + m2*256*256 + m1*256 + m0;
	
	iIPbegin &= iMask;
	iIPend = iIPbegin | (~iMask);
	iIPbegin++;	/*Remove subnet address*/
	iIPend--;	/*Remove subnet broadcast address*/
	
	sprintf(dst, "%u.%u.%u.%u-%u.%u.%u.%u", 
				(iIPbegin >> 24) & 0xff, (iIPbegin >> 16) & 0xff, 
				(iIPbegin >> 8) & 0xff, iIPbegin & 0xff,
				(iIPend >> 24) & 0xff, (iIPend >> 16) & 0xff, 
				(iIPend >> 8) & 0xff, iIPend & 0xff);
}


static int is_radio( char *ifname )
{
	if( ifname[0] == 'r' && ifname[1] >='0' && ifname[1] <= '9' ){
		return 1;
	}

	return 0;
}

//返回值 0:成功	
int fwServiceStart()
{
	int ret = -1;
	int status;

	status = system(CMD_START);
	ret = WEXITSTATUS(status);

	return ret;
}

//返回值 0:成功	
int fwServiceStop()
{
	int ret = -1;
	int status;

	status = system(CMD_STOP);
	ret = WEXITSTATUS(status);

	return ret;
}

//返回值 0:成功	
int fwServiceSaveConf()
{
	int ret = -1;
	int status;

	status = system(CMD_SAVE_CONF);
	ret = WEXITSTATUS(status);

	return ret;
}

//返回值 0:成功	
int fwServiceFlush()
{
	int ret = -1;
	int status;
	
	status = system( CMD_FLUSH );
	ret = WEXITSTATUS(status);

	return ret;
}

