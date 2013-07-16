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
* wp_fwrulemodify.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
*
* DESCRIPTION:
*
*
*
*******************************************************************************/

#include <dbus/dbus.h>
#include <stdlib.h>
#include <sysdef/npd_sysdef.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>


#include "cgic.h"
#include "ws_err.h"
#include "ws_public.h"
#include "ws_firewall.h"
#include "ws_init_dbus.h"
#include "ws_dcli_vrrp.h"
#include "ws_dbus_list_interface.h"

#include "ac_manage_def.h"
#include "ac_manage_firewall_interface.h"


#define _DEBUG	1

#if _DEBUG
#define debug_printf(a...) fprintf(a)
#else
#define debug_printf(a...)
#endif

#define BUF_LEN					32
#define FIREWALL_MAX_RULE_NUM			256
#define FW_MAX_PKG_STATE_LEN			64
#define FW_MAX_STR_FILTER_LEN			BUF_LEN
#define MAX_SLOT				16

static int slot_id = 1;
static instance_parameter *paraHead1 = NULL;
static void *ccgi_connection = NULL;
static char plotid[10] = {0};



static int
firewall_index_is_legal_input(const char *str_index, u_long *slot_id, u_long *index) {

	if(!str_index || !slot_id || !index )
		return -1;

	if (strchr(str_index, '-')) {
		if(2 != sscanf(str_index, "%d-%d", slot_id, index)) {
			return -1;
		}
	}

	if (!*slot_id || *slot_id > MAX_SLOT) {
		return -1;
	}

	if (!*index || *index > FIREWALL_MAX_RULE_NUM) {
		return -1;
	}
	
	return 0;
}

static int
ccgi_firewall_config_status(int status)
{
	int i = 0;
	int ret = 0;
	char err_message[64] = {0};
	for(i = 1; i < MAX_SLOT; i++) {
		ccgi_ReInitDbusConnection(&ccgi_connection, i, DISTRIBUTFAG);
		if(NULL == ccgi_connection) 
			continue;
				
		ret = ac_manage_config_firewall_service(ccgi_connection, status);
		if (0 != ret) {
			snprintf(err_message, sizeof(err_message), 
				"slot %d firewall %s failed", i, status?"start":"stop");
			ShowAlert(err_message);
		}
	}
	return 0;
}

static int
ccgi_firwall_change_index(int rule_type, const char *oldstr, const char *newstr)
{
	unsigned long old_slot_id = 0;
	unsigned long old_index = 0;
	unsigned long new_slot_id = 0;
	unsigned long new_index = 0;
	int ret = 0;

	if (firewall_index_is_legal_input(oldstr, &old_slot_id, &old_index)) {
		return -1;
	}
	if (firewall_index_is_legal_input(newstr, &new_slot_id, &new_index)) {
		return -1;
	}

	if (new_slot_id != old_slot_id) {
		return -1;
	}
	ccgi_ReInitDbusConnection(&ccgi_connection, new_slot_id, DISTRIBUTFAG);
	
	ac_manage_change_firewall_index(ccgi_connection, new_index, rule_type, old_index);
}

static int
ccgi_firewall_delete_rule(int rule_type, const char *str_index)
{
	int index;
	int ret = 0;

	if (NULL == str_index) {
		return -1;
	}
	
	debug_printf(stderr, "-------------------index=%s\n", str_index);
	index = atoi(str_index);
	#if 0
	if (firewall_index_is_legal_input(str_index, &slot_id, &index)) {
		debug_printf(stderr, "firewall_index_is_legal_input slotid=%u, index=%u\n", 
				slot_id, index);
		return -1;
	}
	ccgi_ReInitDbusConnection(&ccgi_connection, slot_id, DISTRIBUTFAG);
	#endif

	ret = ac_manage_del_firewall_rule(ccgi_connection, rule_type, index);

	return ret;

}


static int
ccgi_firewall_get_input(fwRule *rule, u_long *config_type, int rule_type)
{

	if (NULL == rule || NULL == config_type) {
		return -1;
	}
	char inif[32] 		= "";
	char outif[32] 		= "";

	char *ipAddrTypeIndex[] = {"0","1","2","3","4","5"};
	char *ipProtocl[] = {"0","1","2","3","4"};
	char *portType[] = {"0","1","2","3"};
	char *filterAction[] = {"allow","deny","reject","tcpmss"};
	int resoult = 0;
	
	debug_printf(stderr, "-------------------------\n");

/*config type*/
{
	char str_config[32] = "";
	cgiFormStringNoNewlines("editType", str_config, sizeof(str_config));
	if(0 == strncmp(str_config, "edit", 1)) {
		*config_type = 1;
	} else if(strncmp(str_config, "add", 1)) {
		return -1;
	}
}

	debug_printf(stderr, "111111111111111111111111111111111\n");

/* rule type*/
{
	rule->type = rule_type;
}
/*rule index*/
{
	char str_index[32] = "";
	int index;
	cgiFormStringNoNewlines("ruleIndexUserSelected", str_index, sizeof(str_index));
	debug_printf(stderr, "-------------------index=%s\n", str_index);
	sscanf(str_index, "%d", &(index));
	#if 0
	if(firewall_index_is_legal_input(str_index, &slot_id, &index)) {
		debug_printf(stderr, "firewall_index_is_legal_input slotid=%u, index=%u\n", 
				slot_id, index);
		return -1;
	}
	ccgi_ReInitDbusConnection(&ccgi_connection, slot_id, DISTRIBUTFAG);
	#endif
	rule->id = index;
}
	debug_printf(stderr, "22222222222222222222222222222222\n");

/*intf*/
{
	char in_interface[32]	= "";
	char out_interface[32]	= "";
	cgiFormStringNoNewlines("inEthValue", in_interface, sizeof(in_interface));
	cgiFormStringNoNewlines("outEthValue", out_interface, sizeof(out_interface));
	debug_printf(stderr, "-------------------in_interface=%s, out_interface=%s\n", in_interface, out_interface);
	if (!strncmp(in_interface, "ve", 2)) {
		if (ve_interface_parse(in_interface, inif, sizeof(inif))) {
			return -1;
		}
	} else {
		strncpy(inif, in_interface, sizeof(inif) - 1);
	}

    	if (!strncmp(out_interface, "ve", 2)) {
		if (ve_interface_parse(out_interface, outif, sizeof(outif))) {
			return -1;
		}
	} else {
		strncpy(outif, out_interface, sizeof(outif) - 1);
	}
	rule->ineth = inif;
	rule->outeth = outif;
}
	debug_printf(stderr, "33333333333333333333333333333333333333\n");

/*source ip*/
{
	cgiFormRadio("Package.SourceAddress", ipAddrTypeIndex, 6, &resoult, 0) ;
	rule->srctype = ipAddrTypeIndex[resoult][0]-'0';
	
	//根据ip地址类型得到ip地址
	rule->srcadd = (char *)malloc(BUF_LEN);
	memset( rule->srcadd, 0, BUF_LEN );
	switch(rule->srctype)
	{
		case FW_IPSINGLE:
			cgiFormStringNoNewlines("Package.SourceAddress.Single.IP", rule->srcadd, BUF_LEN);
			break;
		case FW_IPHOST:
			cgiFormStringNoNewlines("Package.SourceAddress.DefinedHost.Select", rule->srcadd, BUF_LEN);
			break;
		case FW_IPMASK:
			cgiFormStringNoNewlines("Package.SourceAddress.AddrMask.Addr", rule->srcadd, BUF_LEN);
			if (BUF_LEN - strlen( rule->srcadd ) > 2) {
				strcat(rule->srcadd, "/" );
				cgiFormStringNoNewlines("Package.SourceAddress.AddrMask.Mask", rule->srcadd+strlen(rule->srcadd), BUF_LEN-strlen(rule->srcadd));
			}			
			break;
		case FW_IPNET:
			cgiFormStringNoNewlines( "Package.SourceAddress.DefinedNetwork", rule->srcadd, BUF_LEN );
			break;
		case FW_IPRANG:
			cgiFormStringNoNewlines( "Package.SourceAddress.AddressRange.Begin", rule->srcadd, BUF_LEN );
			if( BUF_LEN - strlen( rule->srcadd ) > 2 ) {
				strcat( rule->srcadd, "-" );
				cgiFormStringNoNewlines( "Package.SourceAddress.AddressRange.End", rule->srcadd+strlen(rule->srcadd), BUF_LEN-strlen(rule->srcadd) );
			}
			break;
		case 0:
		default:
			strcpy( rule->srcadd, "any" );
			break;	
	}
}
	debug_printf(stderr, "4444444444444444444444444444444444444444444444\n");

/*dst ip*/
{
	//得到包匹配的目的地址类型
	cgiFormRadio("Package.DestinationAddress", ipAddrTypeIndex, 6, &resoult, 0) ;
	rule->dsttype = ipAddrTypeIndex[resoult][0]-'0';
	
	//根据ip地址类型得到ip地址
	rule->dstadd = (char *)malloc(BUF_LEN);
	memset( rule->dstadd, 0, BUF_LEN );
	switch(rule->dsttype)
	{
		case FW_IPSINGLE:
			cgiFormStringNoNewlines( "Package.DestinationAddress.Single.IP", rule->dstadd, BUF_LEN );
			break;
		case FW_IPHOST:
			cgiFormStringNoNewlines( "Package.DestinationAddress.DefinedHost.Select", rule->dstadd, BUF_LEN );
			break;
		case FW_IPMASK:
			cgiFormStringNoNewlines( "Package.DestinationAddress.AddrMask.Addr", rule->dstadd, BUF_LEN );
			if( BUF_LEN - strlen( rule->dstadd ) > 2 )
			{
				strcat( rule->dstadd, "/" );
				cgiFormStringNoNewlines( "Package.DestinationAddress.AddrMask.Mask", rule->dstadd+strlen(rule->dstadd), BUF_LEN-strlen(rule->dstadd) );
			}			
			break;
		case FW_IPNET:
			cgiFormStringNoNewlines( "Package.DestinationAddress.DefinedNetwork", rule->dstadd, BUF_LEN );
			break;
		case FW_IPRANG:
			cgiFormStringNoNewlines( "Package.DestinationAddress.AddressRange.Begin", rule->dstadd, BUF_LEN );
			if( BUF_LEN - strlen( rule->dstadd ) > 2 )
			{
				strcat( rule->dstadd, "-" );
				cgiFormStringNoNewlines( "Package.DestinationAddress.AddressRange.End", rule->dstadd+strlen(rule->dstadd), BUF_LEN-strlen(rule->dstadd) );
			}
			break;
		case 0:
		default:
			strcpy( rule->dstadd, "any" );
			break;	
	}
}
	debug_printf(stderr, "5555555555555555555555555555555555555555\n");

/*proto and port*/
{
	cgiFormSelectSingle( "Protocol", ipProtocl, 5, &resoult, 0 );
	rule->protocl = resoult;//FW_PTCP;
	if( 1 == rule->protocl || 2 == rule->protocl || 3 == rule->protocl )
	{
		//收集端口信息
		cgiFormRadio( "Package.SourcePort", portType, 4, &resoult, 0 ) ;
		rule->sptype = resoult;
		rule->sport = (char *)malloc(BUF_LEN);
		memset( rule->sport, 0, BUF_LEN );
		switch( rule->sptype )
		{
			case 1:
				cgiFormStringNoNewlines( "Package.SourcePort.Single.Port", rule->sport, BUF_LEN );
				break;
			case 2:
				cgiFormStringNoNewlines( "Package.SourcePort.Range.Begin", rule->sport, BUF_LEN );
				strcat( rule->sport, ":" );
				cgiFormStringNoNewlines( "Package.SourcePort.Range.End", rule->sport+strlen(rule->sport), BUF_LEN-strlen(rule->sport) );
				break;
			case 3:
				strcpy( rule->sport, "port collection" );
				break;
			case 0:
			default:
				strcpy( rule->sport, "any" );
				break;
		}

		cgiFormRadio( "Package.DestinationPort", portType, 4, &resoult, 0 ) ;
		rule->dptype = resoult;
		rule->dport = (char *)malloc(BUF_LEN);
		memset( rule->dport, 0, BUF_LEN );
		switch( rule->dptype )
		{
			case 1:
				cgiFormStringNoNewlines( "Package.DestinationPort.Single.Port", rule->dport, BUF_LEN );
				break;
			case 2:
				cgiFormStringNoNewlines( "Package.DestinationPort.Range.Begin", rule->dport, BUF_LEN );
				strcat( rule->dport, ":" );
				cgiFormStringNoNewlines( "Package.DestinationPort.Range.End", rule->dport+strlen(rule->dport), BUF_LEN-strlen(rule->dport) );
				break;
			case 3:
				strcpy( rule->dport, "port collection" );
				break;
			case 0:
			default:
				strcpy( rule->dport, "any" );
				break;
		}
	}
}
	debug_printf(stderr, "6666666666666666666666666666666666666666666666\n");

//state
{
	char *states[]={"NEW","ESTABLISHED","RELATED","INVALID"};
	int invalid[]={0,0,0,0};
	int result[4];
	int i;
	
	cgiFormCheckboxMultiple( "state", states, 4, result, invalid );
	if (result[0] != 0 || result[1] != 0 || result[2] != 0 || result[3] != 0) {
		if (rule->pkg_state == NULL) {
			rule->pkg_state = (char *)malloc( 64 );
			if (NULL == rule->pkg_state) {
				return -1;	
			}
			memset(rule->pkg_state, 0, 64);
			for (i=0; i<4; i++) {
				if(1 == result[i]) {
					if (0 == strlen(rule->pkg_state)) {
						strcat( rule->pkg_state, states[i] );	
					} else {
						strcat( rule->pkg_state, "," );
						strcat( rule->pkg_state, states[i] );
					}
				}		
			}
		}
	}
}
	debug_printf(stderr, "777777777777777777777777777777777777777777777777777\n");

//string_filter
{
	if( NULL == rule->string_filter ) {
		rule->string_filter	= (char*)malloc(BUF_LEN);
		if (NULL == rule->string_filter) {
			return -1;	
		}
	}
	memset( rule->string_filter, 0, BUF_LEN );
	cgiFormStringNoNewlines( "string_filter", rule->string_filter, BUF_LEN );
}
	debug_printf(stderr, "8888888888888888888888888888888888888888888\n");

/*action*/
{
	switch(rule->type) {
	case FW_SNAT://SNAT
	case FW_DNAT://DNAT	
		cgiFormRadio( "NatTranslation.IP", ipAddrTypeIndex, 6, &resoult, 0 ) ;
		rule->natiptype = resoult;
		rule->natipadd = (char *)malloc(BUF_LEN);
		memset( rule->natipadd, 0, BUF_LEN );
		switch(rule->natiptype) {
		case FW_IPSINGLE:
			cgiFormStringNoNewlines( "NatSingleIpAddress", rule->natipadd, BUF_LEN );
			break;
		case FW_IPHOST:
			break;
		case FW_IPMASK:
			cgiFormStringNoNewlines( "NatAddressNetmaskAddr", rule->natipadd, BUF_LEN );
			if( BUF_LEN - strlen( rule->natipadd ) > 2 )
			{
				strcat( rule->natipadd, "/" );
				cgiFormStringNoNewlines( "NatAddressNetmaskMask", rule->natipadd+strlen(rule->natipadd), BUF_LEN-strlen(rule->natipadd) );
			}			
			break;
		case FW_IPNET:
			break;
		case FW_IPRANG:
			cgiFormStringNoNewlines( "NatAddrRangeBegin", rule->natipadd, BUF_LEN );
			if( BUF_LEN - strlen( rule->natipadd ) > 2 )
			{
				strcat( rule->natipadd, "-" );
				cgiFormStringNoNewlines( "NatAddrRangeEnd", rule->natipadd+strlen(rule->natipadd), BUF_LEN-strlen(rule->natipadd) );
			}
			break;
		case 0:
		default:
			strcpy( rule->natipadd, "any" );
			break;	
		}
					
		if( 1 == rule->protocl || 2 == rule->protocl || 3 == rule->protocl )//如果设置了协议  有端口设置
		{
			cgiFormRadio( "NatTranslation.Port", ipAddrTypeIndex, 6, &resoult, 0 ) ;
			rule->natpttype = resoult;
			
			rule->natport = (char *)malloc(BUF_LEN);
			memset( rule->natport, 0, BUF_LEN );
			switch( resoult )
			{
				case 1:
					cgiFormStringNoNewlines( "NatSinglePort", rule->natport, BUF_LEN );
					break;
				case 2:
					cgiFormStringNoNewlines( "NatPortRangBegin", rule->natport, BUF_LEN );
					if( BUF_LEN - strlen( rule->natport ) > 2 )
					{
						strcat( rule->natport, "-" );
						cgiFormStringNoNewlines( "NatPortRangEnd", rule->natport+strlen(rule->natport), BUF_LEN-strlen(rule->natport) );
					}						
					break;
				case 0:
				default:
					strcat( rule->natport, "any" );
					break;
			}
		}
		break;
	case FW_WALL:// fire wall
	case FW_INPUT:
	default:
		cgiFormSelectSingle( "filterAction", filterAction, 4, &resoult, 0);
		rule->act = resoult;
		//tcpmss
		if( 3 == rule->act && rule->type != FW_INPUT)
		{
			rule->tcpmss_var = (char*)malloc(BUF_LEN);
			memset( rule->tcpmss_var, 0, BUF_LEN );
			cgiFormStringNoNewlines( "tcpmss_val", rule->tcpmss_var, BUF_LEN );
		}
		break;
	}

}
	debug_printf(stderr, "99999999999999999999999999999999999999999999999\n");

/*rule status*/
{
	char **ruleEnableed; 
	if (cgiFormNotFound != cgiFormStringMultiple("ruleEnabledCHECKBOX", &ruleEnableed)) {
		rule->enable = 1;	
	} else {
		rule->enable = 0;	
	}
}

	return 0;
}



int cgiMain()
{
	fwRule rule;
	u_long config_type= 0;
	int rule_type = 0;
	
	char encry[BUF_LEN] = {0};
	char *str = NULL;
	char ruleTypeStr[16] = "";
	char changeIndexOldRuleIndex[16] = {0};
	char changeIndexNewRuleIndex[16] = {0};
	char delRuleIndex[16] = {0};
	char doFireWall[16] = {0};
	int ret = 0;

	struct list *lpublic;
	struct list *lfirewall;

	lpublic=get_chain_head("../htdocs/text/public.txt");
	lfirewall=get_chain_head("../htdocs/text/firewall.txt");
	DcliWInit();
	ccgi_dbus_init();

	memset(plotid,0,sizeof(plotid));
	cgiFormStringNoNewlines("plotid", plotid, sizeof(plotid)); 
	
	list_instance_parameter(&paraHead1, SNMPD_SLOT_CONNECT);
	if (NULL == paraHead1) {
		return 0;
	}
	if(strcmp(plotid, "") == 0) {
		slot_id = paraHead1->parameter.slot_id;
	} else {
		slot_id = atoi(plotid);
	}
	ccgi_ReInitDbusConnection(&ccgi_connection, slot_id, DISTRIBUTFAG);
	fprintf(stderr, "----------------------------------------------plotid=%s\n", plotid);

	memset(&rule, 0, sizeof(rule));
//	memset(rule, 0, sizeof(rule));
  	cgiFormStringNoNewlines("UN", encry, BUF_LEN);
  	
  	str = dcryption(encry);

  	if(str == NULL)
  	{
		ShowErrorPage(search(lpublic,"ill_user")); 		 /*用户非法*/
		return 0;
  	}

	cgiFormStringNoNewlines("ruleType", ruleTypeStr, sizeof(ruleTypeStr));
	if (strcmp("FW_DNAT", ruleTypeStr) == 0) {
		rule_type = FW_DNAT;
	} else if(strcmp("FW_SNAT",ruleTypeStr) == 0) {
		rule_type = FW_SNAT;
	} else if(strcmp("FW_INPUT",ruleTypeStr) == 0) {
		rule_type = FW_INPUT;
	} else {
		rule_type = FW_WALL;
	}

	cgiHeaderContentType("text/html");	

	if(cgiFormSubmitClicked("submit_addrule") == cgiFormSuccess)
	{
		ret = ccgi_firewall_get_input(&rule, &config_type, rule_type);
		if (0 == ret) {
			ret = ac_manage_config_firewall_rule(ccgi_connection, &rule, config_type);
			debug_printf(stderr, "----------------------------------ret=%d\n", ret);
		}
	}
	else if( cgiFormStringNoNewlines( "delRuleIndex", delRuleIndex, sizeof(delRuleIndex) ) == cgiFormSuccess )
	{
		debug_printf(stderr, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n");
		ccgi_firewall_delete_rule(rule_type, delRuleIndex);
	}
	else if( cgiFormStringNoNewlines( "doFireWall", doFireWall, sizeof(doFireWall) ) == cgiFormSuccess )
	{		
		debug_printf(stderr, "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb\n");
		if (strcmp( doFireWall, "stop" ) == 0) {
			ccgi_firewall_config_status(0);
		} else {
			ccgi_firewall_config_status(1);
		}
	}
	else if(cgiFormStringNoNewlines( "oldIndex", changeIndexOldRuleIndex, sizeof(changeIndexOldRuleIndex) ) == cgiFormSuccess &&
		cgiFormStringNoNewlines( "newIndex", changeIndexNewRuleIndex, sizeof(changeIndexNewRuleIndex) ) == cgiFormSuccess )
	{
		debug_printf(stderr, "cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc\n");
		//ccgi_firwall_change_index(rule_type, changeIndexOldRuleIndex, changeIndexNewRuleIndex);
	}
	
	debug_printf(stderr, "dddddddddddddddddddddddddddddddddddddddddddddddddddd\n");
	fprintf( cgiOut, "<html xmlns=\"http://www.w3.org/1999/xhtml\"> \n" );
	fprintf( cgiOut, "<head> \n" );
	fprintf( cgiOut, "<meta http-equiv=Content-Type content=text/html; charset=gb2312> \n" );
	//下面三句话用于禁止页面缓存
  	fprintf( cgiOut, "<META   HTTP-EQUIV=\"pragma\"   CONTENT=\"no-cache\"> \n");
  	fprintf( cgiOut, "<META   HTTP-EQUIV=\"Cache-Control\"   CONTENT=\"no-cache,   must-revalidate\"> \n" );
  	fprintf( cgiOut, "<META   HTTP-EQUIV=\"expires\"   CONTENT=\"Wed,   26   Feb   1997   08:21:57   GMT\">	\n");

	debug_printf(stderr, "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee\n");
  	
	fprintf( cgiOut, "<title>%s</title> \n", search( lfirewall, "title_fw" ) );
	fprintf( cgiOut, "<link rel=stylesheet href=/style.css type=text/css> \n" );
	fprintf( cgiOut, "<style type=text/css> \n" );
	fprintf( cgiOut, ".usrlis {overflow-x:hidden; overflow:auto; width: 416px; height: 270px; clip: rect( ); padding-top: 0px; padding-right: 0px; padding-bottom: 0px; padding-left: 0px} \n" );
	fprintf( cgiOut, "</style> \n" );
	fprintf( cgiOut, "<style type=text/css> \n" );
	fprintf( cgiOut, "tr.even td { \n" );
	fprintf( cgiOut, "background-color: #eee; \n" );
	fprintf( cgiOut, "} \n" );
	fprintf( cgiOut, "tr.odd td { \n" );
	fprintf( cgiOut, "background-color: #fff; \n" );
	fprintf( cgiOut, "} \n" );
	fprintf( cgiOut, "tr.changed td { \n" );
	fprintf( cgiOut, "background-color: #ffd; \n" );
	fprintf( cgiOut, "} \n" );
	fprintf( cgiOut, " \n" ); 
	fprintf( cgiOut, "tr.new td { \n" );  
	fprintf( cgiOut, "background-color: #dfd; \n" );
	fprintf( cgiOut, "} \n" );
	fprintf( cgiOut, "</style> \n" );
	fprintf( cgiOut, "</head> \n" );
	fprintf( cgiOut, "<script src=\"/fw.js\"></script> \n" );
		
	fprintf( cgiOut, "<body> \n" );
	
	debug_printf(stderr, "fffffffffffffffffffffffffffffffffffffffffffffffffffffff\n");
	//将当前页面转到ruleview!
	fprintf( cgiOut, "<script type='text/javascript'>\n" );
	fprintf( cgiOut, "window.location.href='wp_fwruleview.cgi?UN=%s&ruleType=%s';\n", encry, ruleTypeStr );
	fprintf( cgiOut, "</script>\n" );
		
	fprintf( cgiOut, "</body>\n" );
	fprintf( cgiOut, "</html>\n" );

	release(lpublic); 
	release(lfirewall); 
	
	return 0;	
}

