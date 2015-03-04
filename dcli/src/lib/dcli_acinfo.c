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
* dcli_acinfo.c
*
* MODIFY:
*		by <zhouym@autelan.com> on 2013-03-01 15:09:03 revision <0.1>
*
* CREATOR:
*		zhouym@autelan.com
*
* DESCRIPTION:
*		CLI definition for syslog module.
*
* DATE:
*		2013-03-01 12:00:11
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.10 $	
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif
#include <zebra.h>
#include <dbus/dbus.h>
#include <stdlib.h>
#include <sysdef/npd_sysdef.h>
#include <dbus/npd/npd_dbus_def.h>
#include "vty.h"
#include "command.h"
#include "if.h"
#include "sysdef/returncode.h"

#include "ws_returncode.h"
#include <libxml/xpathInternals.h>

#include "dcli_acinfo.h"
#include <sys/wait.h>
#include <ctype.h>
#include <syslog.h>

#include "dcli_main.h"
#include "ac_manage_def.h"
#include "ac_manage_extend_interface.h"

#include "ws_dbus_list.h"
#include "ws_usrinfo.h"
#include "ws_log_conf.h"
#include "ws_acinfo.h"


struct cmd_node acinfo_node = 
{
	ACINFO_NODE,
	"%s(config-acinfo)# "
};

DEFUN(conf_acinfo_func,
	conf_acinfo_cmd,
	"config acinfo",
	CONFIG_STR
	"config acinfo!\n"
)
{
    char cmd[128];
	if (CONFIG_NODE == vty->node) {
		vty->node = ACINFO_NODE;
	}
	else
	{
		vty_out (vty, "Terminal mode change must under configure mode!\n");
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}

DEFUN(config_accontact_func,
	config_accontact_cmd,
	"set contact-info INFO",
	SHOW_STR
	"Config contact information\n"
	"show contact information\n"
	"Contact information\n"

)
{
	if (ACINFO_NODE != vty->node) 
	{
		vty_out (vty, "Terminal mode change must under acinfo mode!\n");
		return CMD_WARNING;
	}
	int i = 0;
	for(i = 1; i < MAX_SLOT; i++)
	{
		if(dbus_connection_dcli[i]->dcli_dbus_connection)
		{
			ac_manage_set_acinfo_rule(dbus_connection_dcli[i]->dcli_dbus_connection, CON_TYPE, argv[0],OPT_ADD);
		}
	}

	//char contact_cmd[512] = { 0 };
	//snprintf(contact_cmd, sizeof(contact_cmd)-1, "echo -e \"%s\" > /var/run/ac_contact_info", argv[0]);
	//system(contact_cmd);
	return CMD_SUCCESS;
}

DEFUN(show_accontact_func,
	show_accontact_cmd,
	"show contact-info",
	SHOW_STR
	"Contact information\n"
	"show contact information\n"
)
{
	if (ACINFO_NODE != vty->node) 
	{
		vty_out (vty, "Terminal mode change must under acinfo mode!\n");
		return CMD_WARNING;
	}
	int retu = 0;
	char contact_info[512]="";

	FILE *contact_fp=fopen("/var/run/ac_contact_info", "r+");
	if (NULL == contact_fp)
	{
		contact_info[0]='\0';
	}
	else
	{
		fread(contact_info, sizeof(char), sizeof(contact_info)-1, contact_fp);
		fclose(contact_fp);
		contact_info[strlen(contact_info)-1] = '\0';
	}

	vty_out(vty,"================= Contact information ============================================\n");
	vty_out(vty,"%s\n",contact_info);
	vty_out(vty,"==================================================================================\n");
	return CMD_SUCCESS;
}
DEFUN(config_location_func,
	config_location_cmd,
	"set location INFO",
	SHOW_STR
	"Config location information\n"
	"show location information\n"
	"Location information\n"

)
{
	if (ACINFO_NODE != vty->node) 
	{
		vty_out (vty, "Terminal mode change must under acinfo mode!\n");
		return CMD_WARNING;
	}
	int i = 0;
	for(i = 1; i < MAX_SLOT; i++)
	{
		if(dbus_connection_dcli[i]->dcli_dbus_connection)
		{
			ac_manage_set_acinfo_rule(dbus_connection_dcli[i]->dcli_dbus_connection, LOC_TYPE, argv[0],OPT_ADD);
		}
	}
	//char command[512] = { 0 };
	//snprintf(command,sizeof(command)-1,"sudo sys_location.sh  %s >/dev/null",argv[0]);		
	//system(command);
	return CMD_SUCCESS;
}

DEFUN(show_location_func,
	show_location_cmd,
	"show location-info",
	SHOW_STR
	"Location information\n"
	"show location information\n"
)
{
	if (ACINFO_NODE != vty->node) 
	{
		vty_out (vty, "Terminal mode change must under acinfo mode!\n");
		return CMD_WARNING;
	}
	FILE *fp = NULL;
	char result[256] = { 0 };
	memset(result,0,256);
	char acLocationInfo[256] = { 0 };
	memset(acLocationInfo,0,256);
	char *temp = NULL;

	fp = fopen("/var/run/sys_location","r");
	if(fp)
	{
		memset(result,0,256);
		fgets(result,256,fp);
		temp=strchr(result,':');
		memset(acLocationInfo,0,256);
		if(temp)
		{
			strncpy(acLocationInfo,temp+1,sizeof(acLocationInfo)-1);
		}
		fclose(fp);
	}		

	vty_out(vty,"================= Location information ===========================================\n");
	vty_out(vty,"%s\n",acLocationInfo);
	vty_out(vty,"==================================================================================\n");
	return CMD_SUCCESS;
}


DEFUN(show_netelement_func,
	show_netelement_cmd,
	"show netelement-code",
	SHOW_STR
	"Netelement code information\n"
	"show netelement code information\n"
)
{
	if (ACINFO_NODE != vty->node) 
	{
		vty_out (vty, "Terminal mode change must under acinfo mode!\n");
		return CMD_WARNING;
	}
	FILE *fp = NULL;
	char result[256] = { 0 };
	memset(result,0,256);
	char acNetElementCode[256] = { 0 };
	memset(acNetElementCode,0,256);
	char *temp = NULL;

	fp = fopen("/var/run/net_elemnet","r");
	
	if(fp)
	{
		memset(result,0,256);
		fgets(result,256,fp);
		temp=strchr(result,':');
		memset(acNetElementCode,0,256);
		if(temp)
		{
			strncpy(acNetElementCode,temp+1,sizeof(acNetElementCode)-1);
		}
		fclose(fp);
	}		

	vty_out(vty,"================= Netelement Code information ====================================\n");
	vty_out(vty,"%s\n",acNetElementCode);
	vty_out(vty,"==================================================================================\n");
	return CMD_SUCCESS;
}
DEFUN(config_netelement_func,
	config_netelement_cmd,
	"set netelement INFO",
	SHOW_STR
	"Config netelement information\n"
	"Config netelement information\n"
	"Netelement information\n"

)
{
	if (ACINFO_NODE != vty->node) 
	{
		vty_out (vty, "Terminal mode change must under acinfo mode!\n");
		return CMD_WARNING;
	}
	int i = 0;
	for(i = 1; i < MAX_SLOT; i++)
	{
		if(dbus_connection_dcli[i]->dcli_dbus_connection)
		{
			ac_manage_set_acinfo_rule(dbus_connection_dcli[i]->dcli_dbus_connection, NET_TYPE, argv[0],OPT_ADD);
		}
	}
	//char command[512] = { 0 };
	//snprintf(command,sizeof(command)-1,"sudo net_elemnet.sh  %s >/dev/null",argv[0]); 	
	//system(command);
	return CMD_SUCCESS;
}

DEFUN(delete_acinfo_func,
	delete_location_cmd,
	"detele acinfo (location|contact)",
	SHOW_STR
	"Config location information\n"
	"show location information\n"
	"Location information\n"

)
{
	if (ACINFO_NODE != vty->node) 
	{
		vty_out (vty, "Terminal mode change must under acinfo mode!\n");
		return CMD_WARNING;
	}
	
	int i = 0;
	
	if(0 == strcmp(argv[0],"location"))
	{
		for(i = 1; i < MAX_SLOT; i++)
		{
			if(dbus_connection_dcli[i]->dcli_dbus_connection)
			{
				ac_manage_set_acinfo_rule(dbus_connection_dcli[i]->dcli_dbus_connection, LOC_TYPE, argv[0],OPT_DEL);
			}
		}

	}
	else if(0 == strcmp(argv[0],"contact"))
	{
		for(i = 1; i < MAX_SLOT; i++)
		{
			if(dbus_connection_dcli[i]->dcli_dbus_connection)
			{
				ac_manage_set_acinfo_rule(dbus_connection_dcli[i]->dcli_dbus_connection, CON_TYPE, argv[0],OPT_DEL);
			}
		}
	}
	
	return CMD_SUCCESS;
}


DEFUN(config_backupstatus_func,
	config_backupstatus_cmd,
	"set redundancy backup status (enable|disable)",
	SHOW_STR
	"Config backupstatus information\n"
	"Config backupstatus information\n"
	"Backupstatus information\n"

)
{
	if (ACINFO_NODE != vty->node) 
	{
		vty_out (vty, "Terminal mode change must under acinfo mode!\n");
		return CMD_WARNING;
	}
	struct bkacinfo_st rule;
	memset(&rule,0,sizeof(struct bkacinfo_st));
	strncpy(rule.key,argv[0],sizeof(rule.key));
	int i = 0;
	for(i = 1; i < MAX_SLOT; i++)
	{
		if(dbus_connection_dcli[i]->dcli_dbus_connection)
		{
			ac_manage_set_bkacinfo_rule(dbus_connection_dcli[i]->dcli_dbus_connection, STATUS_TYPE,&rule, OPT_ADD);
		}
	}

	//mod_first_xmlnode(ACBACKUPFILE, AC_STATUS, argv[0]);
	return CMD_SUCCESS;
}


DEFUN(config_backupmode_func,
	config_backupmode_cmd,
	"set redundancy backup mode (hot-backup|cold-backup|null)",
	SHOW_STR
	"Config backupmode information\n"
	"Config backupmode information\n"
	"backupmode information\n"

)
{
	if (ACINFO_NODE != vty->node) 
	{
		vty_out (vty, "Terminal mode change must under acinfo mode!\n");
		return CMD_WARNING;
	}
	struct bkacinfo_st rule;
	memset(&rule,0,sizeof(struct bkacinfo_st));
	strncpy(rule.key,argv[0],sizeof(rule.key));
	int i = 0;
	for(i = 1; i < MAX_SLOT; i++)
	{
		if(dbus_connection_dcli[i]->dcli_dbus_connection)
		{
			ac_manage_set_bkacinfo_rule(dbus_connection_dcli[i]->dcli_dbus_connection, MODE_TYPE,&rule, OPT_ADD);
		}
	}
	return CMD_SUCCESS;
}

DEFUN(config_backupnetip_func,
	config_backupnetip_cmd,
	"set redundancy backup network-manage-ipaddress (local_hansi|remote_hansi) A-B ip A.B.C.D",
	SHOW_STR
	"Config backupnetip information\n"
	"Config backupnetip information\n"
	"Backupnetip information\n"

)
{

    unsigned int local_id = 0;
    unsigned int slot_id = 0;
    unsigned int instance_id = 0;
	char   hansi_id[10] = {0};
	//char   change[10] = {0};
	struct bkacinfo_st rule;
		
	if (ACINFO_NODE != vty->node) 
	{
		vty_out (vty, "Terminal mode change must under acinfo mode!\n");
		return CMD_WARNING;
	}

	
    if(0 == strcmp(argv[0], "local_hansi")) {
        local_id = 1;
    }
    else if(0 == strcmp(argv[0], "remote_hansi")) {
        local_id = 0;
    }
    else {
        vty_out(vty, "Input hansi type is error!\n");
        return CMD_WARNING;
    }
	
    if(0 == sscanf(argv[1], "%d-%d", &slot_id, &instance_id)) {
		vty_out(vty, "Input instance (%s) type is error\n", argv[1]);
        return CMD_WARNING;
    }
    if(0 == slot_id || slot_id > MAX_SLOT) {
		vty_out(vty, "Error slot id input : %s\n", argv[1]);
        return CMD_WARNING;
    }
    if(0 == instance_id || instance_id > 16) {
		vty_out(vty, "Error instance id input : %s\n", argv[1]);
        return CMD_WARNING;
    }
	snprintf(hansi_id,sizeof(hansi_id)-1,"%d-%d-%d",slot_id,local_id,instance_id);
	/*strcpy(hansi_id,change);
	strcat(hansi_id,"-");
	sprintf(change,"%d",local_id);
	strcat(hansi_id,change);
	strcat(hansi_id,"-");
	sprintf(change,"%d",instance_id);
	strcat(hansi_id,change);*/

	memset(&rule,0,sizeof(struct bkacinfo_st));
	strncpy(rule.insid,hansi_id,sizeof(rule.insid));
	strncpy(rule.netip,argv[2],sizeof(rule.netip));

	if(dbus_connection_dcli[slot_id]->dcli_dbus_connection)
	{
		ac_manage_set_bkacinfo_rule(dbus_connection_dcli[slot_id]->dcli_dbus_connection, NETIP_TYPE,&rule, OPT_ADD);
	}
	else
	{
		vty_out(vty, "no connection to slot %d \n",slot_id);
        return CMD_WARNING;
	}
	return CMD_SUCCESS;
}

DEFUN(config_backupnetip_ipv6_func,
	config_backupnetip_ipv6_cmd,
	"set redundancy backup network-manage-ipaddress (local_hansi|remote_hansi) A-B ipv6 IP",
	SHOW_STR
	"Config backupnetipV6 information\n"
	"Config backupnetipV6 information\n"
	"BackupnetipV6 information\n"

)
{
	int ret=-1;
	struct in6_addr s;
	unsigned int local_id = 0;
	unsigned int slot_id = 0;
	unsigned int slot_num = 0;
	unsigned int instance_id = 0;
	char   hansi_id[10] = {0};
	struct bkacinfo_st rule;
		
	if (ACINFO_NODE != vty->node) 
	{
		vty_out (vty, "Terminal mode change must under acinfo mode!\n");
		return CMD_WARNING;
	}

	if(0 == strcmp(argv[0], "local_hansi")) {
		local_id = 1;
	}
	else if(0 == strcmp(argv[0], "remote_hansi")) {
		local_id = 0;
	}
	else {
		vty_out(vty, "Input hansi type is error!\n");
		return CMD_WARNING;
	}

	if(0 == sscanf(argv[1], "%d-%d", &slot_id, &instance_id)) {
		vty_out(vty, "Input instance (%s) type is error\n", argv[1]);
		return CMD_WARNING;
	}
	if(0 == slot_id || slot_id > MAX_SLOT) {
		vty_out(vty, "Error slot id input : %s\n", argv[1]);
		return CMD_WARNING;
	}
	//slot_num = slot_id;
	if(0 == instance_id || instance_id > 16) {
		vty_out(vty, "Error instance id input : %s\n", argv[1]);
		return CMD_WARNING;
	}
	snprintf(hansi_id,sizeof(hansi_id)-1,"%d-%d-%d",slot_id,local_id,instance_id);

	ret = inet_pton(AF_INET6, argv[2], (void *)&s);//1:表示成功;0表示格式错误;-1表示解析错误
	if(ret !=1)
	{
		vty_out(vty, "IPv6 address doesn't meet format!");
		return CMD_WARNING;
	}

	memset(&rule,0,sizeof(struct bkacinfo_st));
	strncpy(rule.insid,hansi_id,sizeof(rule.insid));
	strncpy(rule.netip,argv[2],sizeof(rule.netip));

	if(dbus_connection_dcli[slot_id]->dcli_dbus_connection)
	{
		ac_manage_set_bkacinfo_rule(dbus_connection_dcli[slot_id]->dcli_dbus_connection, NETIP_TYPE_IPV6,&rule, OPT_ADD);
	}
	else
	{
		vty_out(vty, "no connection to slot %d \n",slot_id);
        return CMD_WARNING;
	}
	return CMD_SUCCESS;
}


DEFUN(config_backupidentity_func,
	config_backupidentity_cmd,
	"set redundancy backup attribute (master|slave|null)",
	SHOW_STR
	"Config backupidentity information\n"
	"Config backupidentity information\n"
	"Backupidentity information\n"

)
{
	if (ACINFO_NODE != vty->node) 
	{
		vty_out (vty, "Terminal mode change must under acinfo mode!\n");
		return CMD_WARNING;
	}
	struct bkacinfo_st rule;
	memset(&rule,0,sizeof(struct bkacinfo_st));
	strncpy(rule.key,argv[0],sizeof(rule.key));
	int i = 0;
	for(i = 1; i < MAX_SLOT; i++)
	{
		if(dbus_connection_dcli[i]->dcli_dbus_connection)
		{
			ac_manage_set_bkacinfo_rule(dbus_connection_dcli[i]->dcli_dbus_connection, IDEN_TYPE,&rule, OPT_ADD);
		}
	}
	return CMD_SUCCESS;
}


DEFUN(show_backupidentity_func,
	show_backupidentity_cmd,
	"show backupidentity",
	SHOW_STR
	"Backupidentity information\n"
	"show backupidentity information\n"
)
{
	if (ACINFO_NODE != vty->node) 
	{
		vty_out (vty, "Terminal mode change must under acinfo mode!\n");
		return CMD_WARNING;
	}
	if(access(ACBACKUPFILE,0) != 0)
	{
		new_xml_file(ACBACKUPFILE);
	}
	struct acbackup_st ahead;
	struct netipbk_st *aq = NULL;
	memset(&ahead,0,sizeof(struct acbackup_st));
	int confnum = 0;
	int retu = 0;
	retu = read_acinfo_xml(&ahead,&confnum);
	
	vty_out(vty,"================= Backup information =====================================\n");
	vty_out(vty,"Identity :%s\n",ahead.identity);
	vty_out(vty,"Mode :%s\n",ahead.mode);
	vty_out(vty,"Status :%s\n",ahead.status);
	vty_out(vty,"--  -----------------\n");	
	vty_out(vty,"%-2s  %-20s\n","ID","IPV4");
	if(0 == retu)
	{
		aq = ahead.netipst.next;
		while(aq != NULL)
		{
			vty_out(vty,"%s\n",aq->netip);
			aq = aq->next;
		}
	}
	vty_out(vty,"--  -----------------\n");	
	vty_out(vty,"%-2s  %-20s\n","ID","IPV6");
	if(0 == retu)
	{
		aq = ahead.netipst_ipv6.next;
		while(aq != NULL)
		{
			vty_out(vty,"%s\n",aq->netip);
			aq = aq->next;
		}
	}
	vty_out(vty,"===========================================================================\n");
	Free_read_acinfo_xml(&ahead);
	return CMD_SUCCESS;
}

int dcli_acinfo_show_running_config(struct vty* vty)
{
	FILE *fp = NULL;
	vtysh_add_show_string( "!acinfo section\n" );
	vtysh_add_show_string( "config acinfo\n" );

	fp=fopen("/var/run/ac_contact_info", "r+");
	char contact_info[512] = {0};
	char cmd[128] = {0};
	if (NULL == fp)
	{
		contact_info[0]='\0';
	}
	else
	{
		fread(contact_info, sizeof(char), sizeof(contact_info)-1, fp);
		fclose(fp);
		contact_info[strlen(contact_info)-1] = '\0';
	}
	if(0 != strlen(contact_info))
	{
		memset(cmd,0,sizeof(cmd)-1);
		snprintf(cmd,sizeof(cmd)-1," set contact-info %s\n",contact_info);
		vtysh_add_show_string(cmd );
	}
	//////////////////////////////////////////
	char result[256] = { 0 };
	memset(result,0,256);
	char acLocationInfo[256] = { 0 };
	memset(acLocationInfo,0,256);
	char *temp = NULL;

	fp = fopen("/var/run/sys_location","r");
	if(fp)
	{
		memset(result,0,256);
		fgets(result,256,fp);
		temp=strchr(result,':');
		memset(acLocationInfo,0,256);
		if(temp)
		{
			strncpy(acLocationInfo,temp+1,sizeof(acLocationInfo)-1);
		}
		fclose(fp);
	}		
	if(0 != strlen(acLocationInfo))
	{
		memset(cmd,0,sizeof(cmd)-1);
		snprintf(cmd,sizeof(cmd)-1," set location %s\n",acLocationInfo);
		vtysh_add_show_string(cmd );
	}
	//////////////////////////////////////////
	char acNetElementCode[256] = { 0 };
	memset(acNetElementCode,0,256);
	temp = NULL;

	fp = fopen("/var/run/net_elemnet","r");
	
	if(fp)
	{
		memset(result,0,256);
		fgets(result,256,fp);
		temp=strchr(result,':');
		memset(acNetElementCode,0,256);
		if(temp)
		{
			strncpy(acNetElementCode,temp+1,sizeof(acNetElementCode)-1);
		}
		fclose(fp);
	}
	if(0 != strlen(acNetElementCode))
	{
		memset(cmd,0,sizeof(cmd)-1);
		snprintf(cmd,sizeof(cmd)-1," set netelement %s\n",acNetElementCode);
		vtysh_add_show_string(cmd );
	}
	///////////////////
		struct acbackup_st ahead;
	struct netipbk_st *aq = NULL;
	memset(&ahead,0,sizeof(struct acbackup_st));
	int confnum = 0;
	int retu = 0;
	char sid[10] = {0};
	char netip[32] = {0};
	int slot=0;
	int type=0;
	int hansi=0;
	retu = read_acinfo_xml(&ahead,&confnum);
	
	if(0 == retu)
	{

		if(0 != strlen(ahead.identity))
		{
			memset(cmd,0,sizeof(cmd)-1);
			snprintf(cmd,sizeof(cmd)-1," set redundancy backup attribute %s\n",ahead.identity);
			vtysh_add_show_string(cmd );
		}
		if(0 != strlen(ahead.mode))
		{
			memset(cmd,0,sizeof(cmd)-1);
			snprintf(cmd,sizeof(cmd)-1," set redundancy backup mode %s\n",ahead.mode);
			vtysh_add_show_string(cmd );
		}

		aq = ahead.netipst.next;
		while(aq != NULL)
		{
			slot=0;
			type=0;
			hansi=0;
			memset(sid,0,sizeof(sid));
			memset(netip,0,sizeof(netip));
			sscanf(aq->netip,"%s %s",sid,netip);
			sscanf(sid,"%d-%d-%d",&slot,&type,&hansi);
			memset(cmd,0,sizeof(cmd)-1);
			if(type==0)
				snprintf(cmd,sizeof(cmd)-1,"set redundancy backup network-manage-ipaddress remote_hansi %d-%d ip %s\n",slot,hansi,netip);
			else if(type==1)
				snprintf(cmd,sizeof(cmd)-1,"set redundancy backup network-manage-ipaddress local_hansi %d-%d ip %s\n",slot,hansi,netip);
			vtysh_add_show_string(cmd );
			aq = aq->next;
		}
		aq = ahead.netipst_ipv6.next;
		while(aq != NULL)
		{
			slot=0;
			type=0;
			hansi=0;
			memset(sid,0,sizeof(sid));
			memset(netip,0,sizeof(netip));
			sscanf(aq->netip,"%s %s",sid,netip);
			sscanf(sid,"%d-%d-%d",&slot,&type,&hansi);
			memset(cmd,0,sizeof(cmd)-1);
			if(type==0)
				snprintf(cmd,sizeof(cmd)-1,"set redundancy backup network-manage-ipaddress remote_hansi %d-%d ipv6 %s\n",slot,hansi,netip);
			else if(type==1)
				snprintf(cmd,sizeof(cmd)-1,"set redundancy backup network-manage-ipaddress local_hansi %d-%d ipv6 %s\n",slot,hansi,netip);
			vtysh_add_show_string(cmd );
			aq = aq->next;
		}
	}
	Free_read_acinfo_xml(&ahead);

	///////////////////
	vtysh_add_show_string( " exit\n" );
	return CMD_SUCCESS;	
}

void dcli_acinfo_init
(
	void
)  
{
	install_node( &acinfo_node, dcli_acinfo_show_running_config, "ACINFO_NODE");
	install_default(ACINFO_NODE);

	install_element(CONFIG_NODE, &conf_acinfo_cmd);
	
	install_element(ACINFO_NODE, &show_accontact_cmd);
	install_element(ACINFO_NODE, &config_accontact_cmd);
	install_element(ACINFO_NODE, &show_location_cmd);
	install_element(ACINFO_NODE, &config_location_cmd);
	install_element(ACINFO_NODE, &delete_location_cmd);
	install_element(ACINFO_NODE, &show_netelement_cmd);
	install_element(ACINFO_NODE, &config_netelement_cmd);
	install_element(ACINFO_NODE, &config_backupidentity_cmd);
	install_element(ACINFO_NODE, &config_backupmode_cmd);
	install_element(ACINFO_NODE, &show_backupidentity_cmd);
	install_element(ACINFO_NODE, &config_backupstatus_cmd);
	install_element(ACINFO_NODE, &config_backupnetip_cmd);
	install_element(ACINFO_NODE, &config_backupnetip_ipv6_cmd);
}

///////////////////////////////////////////
#ifdef __cplusplus
}
#endif


