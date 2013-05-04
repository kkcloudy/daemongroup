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
* dcli_syslog.c
*
* MODIFY:
*		by <zhouym@autelan.com> on 2010-03-17 15:09:03 revision <0.1>
*
* CREATOR:
*		zhouym@autelan.com
*
* DESCRIPTION:
*		CLI definition for syslog module.
*
* DATE:
*		2010-03-17 12:00:11
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

#include "ws_log_conf.h"
#include "ws_returncode.h"
//#include "ws_usrinfo.h"
#include <libxml/xpathInternals.h>

#include "dcli_syslog.h"
#include <sys/wait.h>
#include <ctype.h>
#include <syslog.h>

#include "dcli_main.h"
#include "ac_manage_def.h"
#include "ac_manage_extend_interface.h"

#include "ws_dbus_list.h"
#include "ac_manage_ntpsyslog_interface.h"

#include "ws_returncode.h"
#include "ws_usrinfo.h"


struct cmd_node syslog_node = 
{
	SYSLOG_NODE,
	"%s(config-syslog)# "
};

DEFUN(conf_syslog_func,
	conf_syslog_cmd,
	"config syslog",
	CONFIG_STR
	"config syslog!\n"
)
{
    char cmd[128];
	if (CONFIG_NODE == vty->node) {
		vty->node = SYSLOG_NODE;
        ////if syslogxml and is not exist,
		if_syslog_exist();
	}
	else
	{
		vty_out (vty, "Terminal mode change must under configure mode!\n");
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}


DEFUN(logger_syslog_func,
	logger_syslog_cmd,
	"logger (inform|notice|warn|error|crit|debug) .CONTENT",
	"syslog logger!\n"
	"LOCAL7 and inform!\n"
	"LOCAL7 and notice!\n"
	"LOCAL7 and warbning!\n"
	"LOCAL7 and error!\n"
	"LOCAL7 and crit!\n"
	"debug!\n"
	"syslog logger content!\n"
)
{
	int flag  = 6;
	int argcnum = 0;
	char *content = (char *)malloc(8192);
	memset(content,0,8192);
	int i = 0;
	if (CONFIG_NODE == vty->node) 
	{
		openlog("syslog", LOG_PID, LOG_DAEMON);
		if(0 == strncmp(argv[0],"inform",6))
		{
			flag = LOG_INFO_Z;
		}
		if(0 == strncmp(argv[0],"notice",6))
		{
			flag = LOG_NOTICE_Z;
		}
		if(0 == strncmp(argv[0],"warn",4))
		{
			flag = LOG_WARNING_Z;
		}
		if(0 == strncmp(argv[0],"error",5))
		{
			flag = LOG_ERR_Z;
		}
		if(0 == strncmp(argv[0],"crit",4))
		{
			flag = LOG_CRIT_Z;
		}
		if(0 == strncmp(argv[0],"debug",5))
		{
			flag = LOG_DEBUG_Z;
		}
		for(i = 1;i<argc;i++)
		{
			strcat(content,(char*)argv[i]);
			strcat(content," ");
		}
		syslog(flag|LOG_LOCAL7, "%s",content);
	}
	else
	{
		vty_out (vty, "Terminal mode change must under configure mode!\n");
		free(content);
		return CMD_WARNING;
	}
	free(content);
	return CMD_SUCCESS;
}

DEFUN(add_syslog_des_ruler_func,
	add_syslog_des_ruler_cmd,
	"(add|delete) destination (udp|tcp) IP <1-65535> <1-6> <1-65535>",
	"create syslog ruler\n"
	"syslog add configuration\n"\
	"syslog delete configuration\n"\
	"syslog udp protocol type\n"
	"syslog tcp protocol type\n"
	"syslog ipaddress A.B.C.D\n"
	"syslog port(1-65535),advise 514 port\n"
	"syslog filter level/: Get levels define by \"show syslog-config-informations\"\n"
	"syslog ID :Get ID define by \"show syslog-config-informations\"\n"
)
{
    if (SYSLOG_NODE != vty->node) 
	{
		vty_out (vty, "Terminal mode change must under configure mode!\n");
		return CMD_WARNING;
	}
	//enable is : 1 ,default is on
	int ilevel=0,syslogflag=-1,portz=0,ipval=-1,ftime=-1,fid=0;	
	char iplen[16] = {0};
	char levelen[2] = {0};
	char idlen[6] = {0};
	strncpy(iplen,argv[2],sizeof(iplen)-1);
	strncpy(levelen,argv[4],sizeof(levelen)-1);
	strncpy(idlen,argv[5],sizeof(idlen)-1);
	
    ilevel=strtoul(levelen,0,10);
	portz=strtoul(argv[3],0,10);
	///////////////////////////////////////////////////////////
	if ((ilevel < 1) ||(ilevel > 6) )
	{
		vty_out(vty, "%% Bad parameter : %s !", levelen);
		return CMD_WARNING;
	}
	if ((portz < 1) ||(portz > 65535)) 
	{
		vty_out(vty, "%% Bad parameter : %s !", argv[3]);
		return CMD_WARNING;
	}
    ipval=parse_ip_check(iplen);
	if(CMD_SUCCESS != ipval)
	{
		vty_out(vty, "%% Bad parameter : %s !", iplen);
		return CMD_WARNING;
	}
	struct syslogrule_st rule;
	memset(&rule,0,sizeof(rule));
	if_syslog_exist();
	char getz[50] = {0};
	char timeflag[50]={0};
	struct filter_st fst,*fq;
	memset(&fst,0,sizeof(fst));
	int fnum=0,fi=0,fsz=0;
	int retu = 0;
	int flagz = 0;
	int i = 0;
	retu = read_filter_xml(&fst,&fnum);
	if(0 == retu)
	{
		fq=fst.next;
		while(fq!=NULL)
		{
			fsz++;
			if(strcmp(fq->viewz,"0")!=0)
			{
				fi++;
				if(fi==ilevel)
					break;
			}		
			fq=fq->next;
		}
	}
	if(fnum>0)
	{
		Free_read_filter_xml(&fst);
	}

	if(ilevel>fi)
	{
		vty_out(vty,"filter level not in the list\n");
		return CMD_WARNING;
	}
	else
	{
		get_second_xmlnode(XML_FPATH, NODE_FILTER, NODE_VALUE, &getz, fsz);
	}
	if(strcmp(getz,"")!=0)
	{
		strncpy(rule.udpstr,argv[1],10);
		strncpy(rule.ipstr,iplen,32);
		strncpy(rule.portstr,argv[3],10);
		strncpy(rule.filter,getz,50);
		strncpy(rule.flevel,levelen,10);
		strncpy(rule.id,idlen,10);

		if(0 == strncmp(argv[0],"add",3))
		{
			memset(timeflag,0,sizeof(timeflag));
		    ftime=if_dup_info(argv[1],iplen,argv[3], getz, &timeflag);
			find_second_xmlnode(XML_FPATH, NODE_LOG, NODE_INDEXZ, idlen, &fid);	
			if(fid!=0)
			{
				vty_out(vty,"The same id is exist\n");
				return CMD_WARNING;
			}
	        else if(ftime==1)
			{
				vty_out(vty,"The same filter level and the same ip ,protocol and port are exist\n");
				return CMD_WARNING;
			}
			else if((ftime==0)&&(fid==0))
			{
				for(i = 1; i < MAX_SLOT; i++)
				{
					if(dbus_connection_dcli[i]->dcli_dbus_connection)
					{
						ac_manage_add_syslog_rule(dbus_connection_dcli[i]->dcli_dbus_connection, &rule,OPT_ADD);
					}
				}
				//add_syslog_serve_web(XML_FPATH, CONF_ENABLE, argv[2], argv[3], getz, argv[1],argv[4],argv[5]);
			}
		}
		else if(0 == strncmp(argv[0],"del",3))
		{
			//del_syslog_serve_web(XML_FPATH, argv[2], argv[3], getz, argv[1], argv[4]);
			for(i = 1; i < MAX_SLOT; i++)
			{
				if(dbus_connection_dcli[i]->dcli_dbus_connection)
				{
					ac_manage_add_syslog_rule(dbus_connection_dcli[i]->dcli_dbus_connection, &rule,OPT_DEL);
				}
			}
		}
	}
	else
	{
		vty_out(vty,"filter level not in the list\n");
		return CMD_WARNING;
	}	
	return CMD_SUCCESS;	
}

DEFUN(contrl_service_func,
	contrl_service_cmd,
	"syslog service (enable|disable)",
	"syslog service config\n"
	"syslog service \n" 
	"Start syslog service \n"
	"Stop syslog service \n"
)
{
   //record if send syslogfiles to server

	char syslog_status[10]={0};
	int i = 0;
	if (SYSLOG_NODE != vty->node) 
	{
		vty_out (vty, "Terminal mode change must under configure mode!\n");
		return CMD_WARNING;
	}
	
	if (!strcmp(argv[0], "enable"))
	{
		strncpy( syslog_status, "start" ,sizeof(syslog_status));	
	}
	else if (!strcmp(argv[0], "disable"))
	{
		strncpy( syslog_status, "stop" ,sizeof(syslog_status));	
	}

	for(i = 1; i < MAX_SLOT; i++)
	{
		if(dbus_connection_dcli[i]->dcli_dbus_connection) 
		{
			ac_manage_set_syslogstatus_rule(dbus_connection_dcli[i]->dcli_dbus_connection, syslog_status,"all");
		}
	}
	return CMD_SUCCESS;
}

DEFUN(syslog_synflood_service_func,
	syslog_synflood_service_cmd,
	"syslog synflood (enable|disable)",
	"syslog synflood config\n"
	"syslog synflood \n" 
	"Start syslog synflood service \n"
	"Stop syslog synflood service \n"
)
{
   //record if send syslogfiles to server

	char syslog_status[10]={0};
	int i = 0;
	if (SYSLOG_NODE != vty->node) 
	{
		vty_out (vty, "Terminal mode change must under configure mode!\n");
		return CMD_WARNING;
	}
	
	if (0 == strcmp(argv[0], "enable"))
	{
		strncpy( syslog_status, "start" ,sizeof(syslog_status));	
	}
	else if (0 == strcmp(argv[0], "disable"))
	{
		strncpy( syslog_status, "stop" ,sizeof(syslog_status));	
	}
	for(i = 1; i < MAX_SLOT; i++)
	{
		if(dbus_connection_dcli[i]->dcli_dbus_connection) 
		{
			ac_manage_set_syslogstatus_rule(dbus_connection_dcli[i]->dcli_dbus_connection, syslog_status,"syn");
		}
	}
	return CMD_SUCCESS;
}

DEFUN(syslog_eaglog_service_func,
	syslog_eaglog_service_cmd,
	"syslog eaglog (enable|disable)",
	"syslog eaglog config\n"
	"syslog eaglog \n" 
	"Start syslog eaglog service \n"
	"Stop syslog eaglog service \n"
)
{
   //record if send syslogfiles to server

	char syslog_status[10]={0};
	int i = 0;
	if (SYSLOG_NODE != vty->node) 
	{
		vty_out (vty, "Terminal mode change must under configure mode!\n");
		return CMD_WARNING;
	}
	
	if (0 == strcmp(argv[0], "enable"))
	{
		strncpy( syslog_status, "start" ,sizeof(syslog_status));	
	}
	else if (0 == strcmp(argv[0], "disable"))
	{
		strncpy( syslog_status, "stop" ,sizeof(syslog_status));	
	}
	for(i = 1; i < MAX_SLOT; i++)
	{
		if(dbus_connection_dcli[i]->dcli_dbus_connection) 
		{
			ac_manage_set_syslogstatus_rule(dbus_connection_dcli[i]->dcli_dbus_connection, syslog_status,"eag");
		}
	}
	return CMD_SUCCESS;
}
static void
dcli_show_syslog_rule_info(DBusConnection *connection, u_int slot_id, struct vty *vty) {
	if(NULL == connection || NULL == vty) {
		return;
	}
	
	int ret = 0;
	struct syslogrule_st *rule_array = NULL;
	u_long rule_num = 0;
	ret = ac_manage_show_syslog_rule(connection, NULL, &rule_array, &rule_num);
	if(AC_MANAGE_SUCCESS == ret && rule_array && rule_num) {
		vty_out(vty, "----------------------------------------------------------------------\n");

		int i = 0;
		for(i = 0; i < rule_num; i++) {
			//firewall_show_rule_entry(&rule_array[i], vty);
			vty_out(vty,"rule_array[%d] is: %s\n",i,&rule_array[i].udpstr);
			vty_out(vty,"rule_array[%d] is: %s\n",i,&rule_array[i].ipstr);
			vty_out(vty,"rule_array[%d] is: %s\n",i,&rule_array[i].portstr);
			vty_out(vty,"rule_array[%d] is: %s\n",i,&rule_array[i].flevel);	
			vty_out(vty,"rule_array[%d] is: %s\n",i,&rule_array[i].id);	
			vty_out(vty, "----------------------------------------------------------------------\n");
		}
		Free_read_ntp_client(rule_array);
	}

    return;
}

DEFUN(show_syslog_info_func,
	show_syslog_info_cmd,
	"show syslog-config-informations",
	SHOW_STR
	"syslog configure\n"
	"syslog base inforamtion\n"
	"syslog base inforamtion\n"
)
{
	if (SYSLOG_NODE != vty->node) 
	{
		vty_out (vty, "Terminal mode change must under syslog mode!\n");
		return CMD_WARNING;
	}
	if_syslog_exist();
	int dcount = 0;
	int retu = 0;
	vty_out(vty,"================= Filter explanation =============================================\n");
	vty_out(vty,"%-3s  %-20s\n","ID","INFO");
	vty_out(vty,"---  -----------------\n");	
	struct filter_st fst,*fq;
	memset(&fst,0,sizeof(fst));
	int fnum=0,fi=0;
	int i = 0; 
	retu = read_filter_xml(&fst,&fnum);
	if(0 == retu)
	{
		fq=fst.next;
		while(fq!=NULL)
		{
			if(strcmp(fq->viewz,"0")!=0)
			{
				fi++;
				vty_out(vty,"%d\t",fi);
				vty_out(vty,"%s\t\n",fq->infos);			
			}
			fq=fq->next;
		}
	}
	if(fnum>0)
	{
		Free_read_filter_xml(&fst);
	}
	
	vty_out(vty,"==================== Syslog Rules ================================================\n");
	vty_out(vty,"%-3s  %-8s  %-15s %-5s %-20s\n","ID","PROTOCOL","IP","PORT","LEVEL");
	vty_out(vty,"---  --------  ---------------  -----  -------------------------------------------\n");
	#if 0
	for(i = 1; i < MAX_SLOT; i++) {
		if(dbus_connection_dcli[i]->dcli_dbus_connection) {
			vty_out(vty, "Slot %d SYSLOG Rule Info\n", i);
			dcli_show_syslog_rule_info(dbus_connection_dcli[i]->dcli_dbus_connection, i, vty);
			vty_out(vty, "========================================================================\n");
		}
	}
	#endif
	struct dest_st d_head,*dq;
    int dnum=0;
    retu = read_dest_xml(&d_head, &dnum);
	if(0 == retu)
	{
		dq=d_head.next;
		while(dq!=NULL)
		{
			if(strcmp(dq->timeflag,"")!=0)
			{
				dcount++;				
				vty_out(vty , "%-3s\t",dq->indexz);
				//vty_out(vty , "%-3d\t",dcount);
				vty_out(vty , "%-3s\t",dq->proz);
				vty_out(vty , "%-15s\t",dq->sysipz);
				vty_out(vty , "%-5s\t",dq->sysport);			
				vty_out(vty , "%-2s\t\n",dq->flevel);

			}
			dq=dq->next;
		}
	}
	Free_read_dest_xml(&d_head);
	vty_out(vty,"==================== Syslog Service ==============================================\n");
	vty_out(vty,"%-20s  %-15s\n","TYPE","STATUS");
	vty_out(vty,"--------------------  ------------------------------------------------------------\n");				
	char syslog_status[10] = {0};
	get_first_xmlnode(XML_FPATH,NODE_LSTATUS,&syslog_status);
	vty_out(vty,"%-20s","Send Server");
	if(0 == strcmp(syslog_status,"start"))
	{
		vty_out(vty,"%-7s\n","enable");
	}
	else
	{
		vty_out(vty,"%-7s\n","disable");
	}
	memset(syslog_status,0,sizeof(syslog_status));
	get_first_xmlnode(XML_FPATH,IF_SYNFLOOD,&syslog_status);
	vty_out(vty,"%-20s","Synflood");
	if(0 == strcmp(syslog_status,"start"))
	{
		vty_out(vty,"%-7s\n","enable");
	}
	else
	{
		vty_out(vty,"%-7s\n","disable");
	}
	vty_out(vty,"%-20s","Eaglog");
	//function
	memset(syslog_status,0,sizeof(syslog_status));
	get_first_xmlnode(XML_FPATH,IF_EAGLOG,&syslog_status);
	if(0 == strcmp(syslog_status,"start"))
	{
		vty_out(vty,"%-7s\n","enable");
	}
	else
	{
		vty_out(vty,"%-7s\n","disable");
	}
	vty_out(vty,"%-20s","Save Cycle");
	memset(syslog_status,0,sizeof(syslog_status));
	get_first_xmlnode(XML_FPATH,NODE_SAVECYCLE,&syslog_status);
	if(0 == strcmp(syslog_status,""))
	{
		vty_out(vty,"%-7s(days)\n","3");
	}
	else
	{
		vty_out(vty,"%-7s(days)\n",syslog_status);
	}
	
	vty_out(vty,"==================================================================================\n");
	return CMD_SUCCESS;
}

DEFUN(save_syslog_info_func,
	save_syslog_info_cmd,
	"syslog save",
	SHOW_STR
	"save syslog\n"
	"save syslog file\n"
	"save syslog file\n"
)
{
	if ((SYSLOG_NODE != vty->node) && (ENABLE_NODE!= vty->node))
	{
		vty_out (vty, "Terminal mode change must under syslog mode!\n");
		return CMD_WARNING;
	}

	int i = 0;
	for(i = 1; i < MAX_SLOT; i++) {
		if(dbus_connection_dcli[i]->dcli_dbus_connection) {
			vty_out(vty, "Slot %d SYSLOG Info save\n", i);
			ac_manage_save_syslog_rule(dbus_connection_dcli[i]->dcli_dbus_connection, "1","3");
			vty_out(vty,"==================================================================================\n");
		}
	}
	vty_out(vty,"==================================================================================\n");
	return CMD_SUCCESS;
}

DEFUN(upload_syslog_info_func,
	upload_syslog_info_cmd,
	"upload syslog ftp SERVER USERNAME PASSWORD",
	SHOW_STR
	"upload syslog\n"
	"upload syslog file\n"	
	"ftp server ipaddress\n"
	"ftp server username\n"
	"ftp server password\n"
)
{

	if ((SYSLOG_NODE != vty->node) && (ENABLE_NODE!= vty->node))
	{
		vty_out (vty, "Terminal mode change must under syslog mode!\n");
		return CMD_WARNING;
	}
	char cmdstr[128] = {0};
	char ftpip[32] = {0};
	int ret = 0;
	int slotid = 0;
	int p_masterid = 0;
	int i = 0;
	p_masterid = get_product_info(PRODUCT_ACTIVE_MASTER);
	snprintf(cmdstr,sizeof(cmdstr)-1,"mftpupload.sh %s %s %s %s %s\n",argv[0],argv[1],argv[2],"systemlog*","/var/run/");
	char infstr[20] = {0};
    strncpy(ftpip,argv[0],sizeof(ftpip)-1);	
	if(!ip_address_check(ftpip))
	{
		vty_out(vty,"IP error\n");
		return CMD_SUCCESS;
	}
	for(i = 1;i<MAX_SLOT;i++)
	{
		if(dbus_connection_dcli[i]->dcli_dbus_connection)
		{
			memset(infstr,0,sizeof(infstr));
			snprintf(infstr,sizeof(infstr)-1,"slot%d",i);
			if(p_masterid != i)
			{
				ac_manage_config_ssylogupload_pfm_requestpkts(dcli_dbus_connection, infstr, ftpip,21,OPT_ADD);
				ac_manage_config_ssylogupload_pfm_requestpkts(dcli_dbus_connection, infstr, ftpip,20,OPT_ADD);	
			}
		}
	}
	system(cmdstr);
	vty_out(vty,"==================================================================================\n");
	return CMD_SUCCESS;
}

DEFUN(show_syslog_user_func,
	show_syslog_user_cmd,
	"show syslog user",
	SHOW_STR
	"show syslog user\n"
	"show syslog user\n"	
	"show syslog user\n"
)
{

	if ((SYSLOG_NODE != vty->node))
	{
		vty_out (vty, "Terminal mode change must under syslog mode!\n");
		return CMD_WARNING;
	}
	if(access(USER_SYSLOG_XML,0)!=0)
	{		
		init_user_syslog_xml();
	}
	char log_info[64] = {0};
	char oper_info[64] = {0};
	int ret,i;
	struct group *grentry = NULL;
	char *ptr = NULL;
	vty_out(vty,"==================== Syslog Users ================================================\n");
	vty_out(vty,"%-16s  %-8s  %-15s %-20s\n","USERNAME","TYPE","OPERRATE","LOGERINFO");
	vty_out(vty,"----------------  --------  ---------------  -------------------------------------------\n");
	grentry = getgrnam(ADMINGROUP);
	if (grentry)
	{
		for(i=0;ptr=grentry->gr_mem[i];i++)
		{
			memset(log_info,0,sizeof(log_info));
			memset(oper_info,0,sizeof(oper_info));
			get_user_syslog_by_name(ptr,&log_info,&oper_info);
			vty_out(vty,"%-16s\t",ptr);
			vty_out(vty,"%-5s\t","admin");
			vty_out(vty,"%-16s\t",(strlen(log_info)==0)?"debug":log_info);
			vty_out(vty,"%-10s\t\n",(strlen(oper_info)==0)?"none":oper_info);
			if(strlen(log_info)==0)
			{
				add_user_syslog_by_name(ptr);
			}
		}
	
	}

	vty_out(vty,"==================================================================================\n");
	return CMD_SUCCESS;
}

DEFUN(modify_syslog_user_func,
	modify_syslog_user_cmd,
	"modify syslog user USERNAME (none|read|delete)",
	SHOW_STR
	"modify syslog user\n"
	"modify syslog user\n"	
	"modify syslog user\n"
)
{

	if ((SYSLOG_NODE != vty->node))
	{
		vty_out (vty, "Terminal mode change must under syslog mode!\n");
		return CMD_WARNING;
	}
	if(access(USER_SYSLOG_XML,0)!=0)
	{		
		init_user_syslog_xml();
	}
	char usrname[64] = {0};
	char optstr[20] = {0};
	int ret = 0;
	strncpy(usrname,argv[0],sizeof(usrname)-1);
	strncpy(optstr,argv[1],sizeof(optstr)-1);
	ret = mod_user_syslog_by_name(usrname,"crit",optstr);
	if(ret == 1)
	{
		vty_out(vty,"Modify user operration successfully!\n");
	}
	else 
	{
		vty_out(vty,"Modify user operration failed,this user does not exist!\n");
	}
	return CMD_SUCCESS;
}


DEFUN(set_syslog_savecycle_func,
	set_syslog_savecycle_cmd,
	"syslog savecycle <3-14>",
	SHOW_STR
	"modify syslog user\n"
	"modify syslog user\n"	
	"modify syslog user\n"
)
{

	if ((SYSLOG_NODE != vty->node))
	{
		vty_out (vty, "Terminal mode change must under syslog mode!\n");
		return CMD_WARNING;
	}
	int day_num = 0;
	day_num = atoi(argv[0]);
	if((day_num > 14)||(day_num < 3))
	{
		vty_out(vty,"Save cycle beyond the range!\n");
		return CMD_SUCCESS;		
	}
	int i = 0;
	for(i = 1; i < MAX_SLOT; i++) {
		if(dbus_connection_dcli[i]->dcli_dbus_connection) {
			ac_manage_save_syslog_rule(dbus_connection_dcli[i]->dcli_dbus_connection, "2",argv[0]);
		}
	}
	return CMD_SUCCESS;
}

DEFUN(delete_syslog_func,
	delete_syslog_cmd,
	"syslog delete all",
	SHOW_STR
	"Syslog information\n"
	"Display all keyword\n"
)
{
	int i = 0;
	char cmdstr[128] = {0};
	char buff[64] = {0};
	if(access(USER_SYSLOG_XML,0)!=0)
	{		
		init_user_syslog_xml();
	}
	snprintf(cmdstr,sizeof(cmdstr)-1,"who am i |awk '{print $1}'");
	FILE *fp = NULL;
	fp = popen(cmdstr,"r");
	if(NULL == fp)
	{
		return CMD_SUCCESS;
	}
	
	fgets(buff,sizeof(buff),fp);
	pclose(fp);
	char log_info[64] = {0};
	char oper_info[64] = {0};
	delete_line_blank(buff);
	get_user_syslog_by_name(buff,&log_info,&oper_info);
	if((0 == strncmp(buff,"root",4))||(0 == strncmp(oper_info,"delete",6)))
	{
		for(i = 1; i < MAX_SLOT; i++) {
			if(dbus_connection_dcli[i]->dcli_dbus_connection) {
				ac_manage_save_syslog_rule(dbus_connection_dcli[i]->dcli_dbus_connection, "3","all");
			}
		}
		vty_out(vty,"Delete log successfully!\n");
	}
	else
	{
		vty_out(vty,"Login user hasnot delete authority!\n");
	}	
	return CMD_SUCCESS;
}

DEFUN(show_syslog_destination_func,
	show_syslog_destination_cmd,
	"show destination",
	SHOW_STR
	"Syslog information\n"
	"show syslog destination rules\n"
)
{
	if (SYSLOG_NODE != vty->node) 
	{
		vty_out (vty, "Terminal mode change must under syslog mode!\n");
		return CMD_WARNING;
	}
	if_syslog_exist();
	int dcount = 0;
	int retu = 0;
	vty_out(vty,"================= Filter explanation =============================================\n");
	vty_out(vty,"%-20s  %-20s %-10s\n","RULENAME","IP","PORT");
	vty_out(vty,"--------------------  ----------------- ------------------------\n");	
	//////////////////////////
	struct dest_st d_head,*dq;
    int dnum=0;
    retu = read_dest_xml(&d_head, &dnum);
	if(0 == retu)
	{
		dq=d_head.next;
		while(dq!=NULL)
		{
			if(strcmp(dq->timeflag,"")!=0)
			{
				dcount++;				
				//vty_out(vty , "%-3s\t",dq->indexz);
				//vty_out(vty , "%-3d\t",dcount);
				vty_out(vty , "%-20s\t",dq->valuez);
				vty_out(vty , "%-20s\t",dq->sysipz);
				vty_out(vty , "%-10s\t\n",dq->sysport);
				//vty_out(vty , "%-2s\t\n",dq->flevel);
			}
			dq=dq->next;
		}
	}
	Free_read_dest_xml(&d_head);
	//////////////////////////
	vty_out(vty,"==================================================================================\n");
	return CMD_SUCCESS;
}

DEFUN(show_syslog_filter_func,
	show_syslog_filter_cmd,
	"show filter",
	SHOW_STR
	"Syslog information\n"
	"show syslog filter rules\n"
)
{
	if (SYSLOG_NODE != vty->node)
	{
		vty_out (vty, "Terminal mode change must under syslog mode!\n");
		return CMD_WARNING;
	}
	if_syslog_exist();
	int dcount = 0;
	int retu = 0;
	vty_out(vty,"================= Filter explanation =============================================\n");
	vty_out(vty,"%-20s %-50s\n","RULENAME","CONTENT");
	vty_out(vty,"----------------- ------------------------\n");
	struct filter_st fst,*fq;
	memset(&fst,0,sizeof(fst));
	int fnum=0,fi=0;
	int i = 0; 
	retu = read_filter_xml(&fst,&fnum);
	if(0 == retu)
	{
		fq=fst.next;
		while(fq!=NULL)
		{
			if(strcmp(fq->viewz,"2") == 0)
			{
				fi++;
				vty_out(vty,"%s\t",fq->valuez);
				vty_out(vty,"%s\t\n",fq->infos);
			}
			fq=fq->next;
		}
	}
	if(fnum>0)
	{
		Free_read_filter_xml(&fst);
	}
	vty_out(vty,"==================================================================================\n");
	return CMD_SUCCESS;
}

DEFUN(show_syslog_log_func,
	show_syslog_log_cmd,
	"show logrule",
	SHOW_STR
	"Syslog information\n"
	"show syslog log rules\n"
)
{
	if (SYSLOG_NODE != vty->node) 
	{
		vty_out (vty, "Terminal mode change must under syslog mode!\n");
		return CMD_WARNING;
	}
	if_syslog_exist();
	int dcount = 0;
	int retu = 0;
	vty_out(vty,"================= Filter explanation =============================================\n");
	vty_out(vty,"%-10s  %-40s %-50s\n","RULENAME","FILTER","DESTINATION");
	vty_out(vty,"----------    -------------------------------------    ------------------------\n");	
	struct log_st cst,*cq;
	memset(&cst,0,sizeof(cst));
	int cnum=0,fi=0;
	int i = 0; 

	retu = read_log_xml(&cst, &cnum);
	if(0 == retu)
	{
		cq=cst.next;
		while(cq!=NULL)
		{
			if(strcmp(cq->enablez,"1") == 0)
			{
				fi++;
				vty_out(vty,"%s\t",cq->timeflag);
				vty_out(vty,"%12s\t",cq->filterz);
				vty_out(vty,"%50s\t\n",cq->des);
			}
			cq=cq->next;
		}
	}
	if(cnum>0)
	{
		Free_read_log_xml(&cst);
	}
	vty_out(vty,"==================================================================================\n");
	return CMD_SUCCESS;
}

DEFUN(add_syslog_filter_ruler_func,
	add_syslog_filter_ruler_cmd,
	"add filter RULENAME match KEYWORD",
	"create syslog ruler\n"
	"syslog add configuration\n"\
	"syslog delete configuration\n"\
	"syslog COMMAND :one or multiple parameters\"\n"
)
{
    if (SYSLOG_NODE != vty->node)
	{
		vty_out (vty, "Terminal mode change must under configure mode!\n");
		return CMD_WARNING;
	}
	if_syslog_exist();	
	char rulename[20] = {0};
	strncpy(rulename,argv[0],sizeof(rulename)-1);
	char command[128] = { 0 };
	strncpy(command,argv[1],sizeof(command)-1);
	#if 0 
	unsigned int count = argc;
	vty_out(vty,"#dcli#------------argc is: %d\n",argc);

	int i = 0;
	for(i = 1; ; i++) 
	{

		if((sizeof(command) - 1) < (strlen(command) + strlen(argv[i]))) 
		{
			vty_out(vty, "input command exit max len\n");
			return CMD_WARNING;
		}

		strcat(command, argv[i]);
		vty_out(vty,"#dcli#-------argv[%d] is: %s\n",i,argv[i]);

		if((i + 2) > count) 
		{
			break;
		}

		strcat(command, "/");
	}
	#endif
	int i = 0;
	struct syslogrule_st rule;
	memset(&rule,0,sizeof(rule));
	strncpy(rule.rulename,rulename,sizeof(rule.rulename));
	strncpy(rule.keyword,command,sizeof(rule.keyword));
	for(i = 1; i < MAX_SLOT; i++)
	{
		if(dbus_connection_dcli[i]->dcli_dbus_connection)
		{
			ac_manage_config_syslog_rule(dbus_connection_dcli[i]->dcli_dbus_connection, &rule,FIL_OPT,OPT_ADD);
		}
	}

    //add_filter(rulename, command);
	//save_syslog_file();

	return CMD_SUCCESS;	
}

DEFUN(delete_syslog_filter_ruler_func,
	delete_syslog_filter_ruler_cmd,
	"delete filter RULENAME",
	"create syslog ruler\n"
	"syslog add configuration\n"\
	"syslog delete configuration\n"\
	"syslog COMMAND :one or multiple parameters\"\n"
)
{
    if (SYSLOG_NODE != vty->node) 
	{
		vty_out (vty, "Terminal mode change must under configure mode!\n");
		return CMD_WARNING;
	}
	if_syslog_exist();	
	char rulename[20] = {0};
	strncpy(rulename,argv[0],sizeof(rulename)-1);

	int i = 0;
	struct syslogrule_st rule;
	memset(&rule,0,sizeof(struct syslogrule_st));
	strncpy(rule.rulename,rulename,sizeof(rule.rulename));
	for(i = 1; i < MAX_SLOT; i++)
	{
		if(dbus_connection_dcli[i]->dcli_dbus_connection)
		{
			ac_manage_config_syslog_rule(dbus_connection_dcli[i]->dcli_dbus_connection, &rule,FIL_OPT,OPT_DEL);
		}
	}
	//del_syslogruler(XML_FPATH, NODE_FILTER,NODE_VALUE,rulename);
	//del_syslogruler(XML_FPATH, NODE_LOG,NODE_FILTER,rulename);
	//save_syslog_file();
	return CMD_SUCCESS;	
}

DEFUN(add_des_ruler_func,
	add_des_ruler_cmd,
	"add destination RULENAME IP <1-65535>",
	"create syslog ruler\n"
	"syslog add configuration\n"\
	"syslog destination rulename\n"\
	"syslog ipaddress A.B.C.D\n"
	"syslog port(1-65535),advise 514 port\n"
)
{
    if (SYSLOG_NODE != vty->node)
	{
		vty_out (vty, "Terminal mode change must under configure mode!\n");
		return CMD_WARNING;
	}
	//enable is : 1 ,default is on
	int portz=0,ipval=-1,ftime=-1;
	char rulename[20] = {0};
	strncpy(rulename,argv[0],sizeof(rulename)-1);
	char rname_end[25] = {0};
	snprintf(rname_end,sizeof(rname_end)-1,"df_%s",rulename);
	char iplen[16] = {0};
	strncpy(iplen,argv[1],sizeof(iplen)-1);
	
	portz=strtoul(argv[2],0,10);  
	///////////////////////////////////////////////////////////
	if ((portz < 1) ||(portz > 65535))
	{
		vty_out(vty, "%% Bad parameter : %s !", argv[2]);
		return CMD_WARNING;
	}
    ipval=parse_ip_check(iplen);
	if(CMD_SUCCESS != ipval)
	{  
		vty_out(vty, "%% Bad parameter : %s !", iplen);   
		return CMD_WARNING;
	}
	if_syslog_exist();
	
	char timeflag[50]={0};
	struct filter_st fst,*fq;
	memset(&fst,0,sizeof(fst));
	int fnum=0,fi=0,fsz=0;
	int retu = 0;
	int flagz = 0;
	int i = 0;
	struct syslogrule_st rule;

	strncpy(rule.ipstr,iplen,sizeof(rule.ipstr));
	strncpy(rule.portstr,argv[2],sizeof(rule.portstr));
	strncpy(rule.rulename,rname_end,sizeof(rule.rulename));

	memset(timeflag,0,sizeof(timeflag));
    ftime=if_dup_info("udp",iplen,argv[2], rname_end, &timeflag);

	

    if(ftime==1)
	{
		vty_out(vty,"The same same ip port are exist\n");
		return CMD_WARNING;
	}
	else
	{
		memset(&rule,0,sizeof(rule));
		strncpy(rule.rulename,rulename,sizeof(rule.rulename));
		strncpy(rule.ipstr,iplen,sizeof(rule.ipstr));
		strncpy(rule.portstr,argv[2],sizeof(rule.portstr));
		
		for(i = 1; i < MAX_SLOT; i++)
		{
			if(dbus_connection_dcli[i]->dcli_dbus_connection)
			{
				ac_manage_config_syslog_rule(dbus_connection_dcli[i]->dcli_dbus_connection, &rule,DES_OPT,OPT_ADD);
			}
		}
	}
	return CMD_SUCCESS;	
}
DEFUN(delete_des_ruler_func,
	delete_des_ruler_cmd,
	"delete destination RULENAME",
	"create syslog ruler\n"
	"syslog add configuration\n"\
	"syslog destination rulename\n"\
)
{
    if (SYSLOG_NODE != vty->node) 
	{
		vty_out (vty, "Terminal mode change must under configure mode!\n");
		return CMD_WARNING;
	}
	int ftime=-1;
	char rulename[20] = {0};
	strncpy(rulename,argv[0],sizeof(rulename)-1);
	
	if_syslog_exist();
	vty_out(vty,"rulename is: %s\n",rulename);
	int i = 0;
	struct syslogrule_st rule;
	memset(&rule,0,sizeof(rule));
	strncpy(rule.rulename,rulename,sizeof(rule.rulename));
	
	for(i = 1; i < MAX_SLOT; i++)
	{
		if(dbus_connection_dcli[i]->dcli_dbus_connection)
		{
			ac_manage_config_syslog_rule(dbus_connection_dcli[i]->dcli_dbus_connection, &rule,DES_OPT,OPT_DEL);
		}
	}
	return CMD_SUCCESS;	
}

DEFUN(delete_log_ruler_func,
	delete_log_ruler_cmd,
	"delete logrule RULENAME",
	"create syslog ruler\n"
	"syslog add configuration\n"\
	"syslog log rulename\n"\
)
{
    if (SYSLOG_NODE != vty->node) 
	{
		vty_out (vty, "Terminal mode change must under configure mode!\n");
		return CMD_WARNING;
	}
	int ftime=-1;	
	char rulename[20] = {0};
	strncpy(rulename,argv[0],sizeof(rulename)-1);
	
	if_syslog_exist();
	int i = 0;
	struct syslogrule_st rule;
	memset(&rule,0,sizeof(rule));
	strncpy(rule.rulename,rulename,sizeof(rule.rulename));
	
	for(i = 1; i < MAX_SLOT; i++)
	{
		if(dbus_connection_dcli[i]->dcli_dbus_connection)
		{
			ac_manage_config_syslog_rule(dbus_connection_dcli[i]->dcli_dbus_connection, &rule,LOG_OPT,OPT_DEL);
		}
	}
	return CMD_SUCCESS;	
}

DEFUN(add_log_ruler_func,
	add_log_ruler_cmd,
	"add log FILTERNAME DESNAME",
	"create syslog ruler\n"
	"syslog add logrule configuration\n"\
	"syslog filter rulename\n"\
	"syslog destination rulename\n"\
)
{
    if (SYSLOG_NODE != vty->node) 
	{
		vty_out (vty, "Terminal mode change must under configure mode!\n");
		return CMD_WARNING;
	}
	//enable is : 1 ,default is on
	char filtername[64] = {0};
	strncpy(filtername,argv[0],sizeof(filtername)-1);
	char desname[64] = {0};
	strncpy(desname,argv[1],sizeof(desname)-1);
	if_syslog_exist();
	int flag = 0;
	find_second_xmlnode(XML_FPATH, NODE_FILTER, NODE_VALUE, filtername, &flag);
	if(0 == flag)
	{
		vty_out(vty,"Not have this filter ruler\n");
		return CMD_SUCCESS;	
	}
	flag = 0;
	find_second_xmlnode(XML_FPATH, NODE_DES, NODE_VALUE, desname, &flag);
	if(0 == flag)
	{
		vty_out(vty,"Not have this destination ruler\n");
		return CMD_SUCCESS;
	}
	int i = 0;
	struct syslogrule_st rule;
	memset(&rule,0,sizeof(rule));
	strncpy(rule.rulename,filtername,sizeof(rule.rulename));
	strncpy(rule.keyword,desname,sizeof(rule.keyword));
	
	for(i = 1; i < MAX_SLOT; i++)
	{
		if(dbus_connection_dcli[i]->dcli_dbus_connection)
		{
			ac_manage_config_syslog_rule(dbus_connection_dcli[i]->dcli_dbus_connection, &rule,LOG_OPT,OPT_ADD);
		}
	}
	return CMD_SUCCESS;	
}

int dcli_syslog_show_running_config(struct vty* vty)
{
    if_syslog_exist();
	char cmdstr[256] = {0};
	int i = 0;
	int retu = 0;
	struct dest_st dst,*dq;
	memset(&dst,0,sizeof(dst));
	int dnum=0;
	retu = read_dest_xml(&dst,&dnum);

	vtysh_add_show_string( "!syslog section\n" );
	vtysh_add_show_string( "config syslog\n" );
	if(0 == retu)
	{
		dq=dst.next;
		while(dq!=NULL)
		{
			if(strcmp(dq->timeflag,"")!=0)
			{
				memset(cmdstr,0,256);
				sprintf(cmdstr," add destination %s %s %s %s %s\n",dq->proz,dq->sysipz,dq->sysport,dq->flevel,dq->indexz);
				vtysh_add_show_string(cmdstr);
			}
			dq=dq->next;
		}
	}
	Free_read_dest_xml(&dst);
	char syslog_status[10] = {0};
	get_first_xmlnode(XML_FPATH,NODE_LSTATUS,&syslog_status);
	if(0 == strcmp(syslog_status,"start"))
	{
		vtysh_add_show_string(" syslog service enable\n");
	}
	//else
	//{
		//vtysh_add_show_string(" syslog service disable\n");
	//}
	memset(syslog_status,0,sizeof(syslog_status));
	get_first_xmlnode(XML_FPATH,IF_SYNFLOOD,&syslog_status);
	if(0 == strcmp(syslog_status,"start"))
	{
		vtysh_add_show_string(" syslog synflood enable\n");
	}
	//else
	//{
		//vtysh_add_show_string(" syslog synflood disable\n");
	//}
	memset(syslog_status,0,sizeof(syslog_status));
	get_first_xmlnode(XML_FPATH,IF_EAGLOG,&syslog_status);
	if(0 == strcmp(syslog_status,"start"))
	{
		vtysh_add_show_string(" syslog eaglog enable\n");
	}
	//else
	//{
		//vtysh_add_show_string(" syslog eaglog disable\n");
	//}
	memset(syslog_status,0,sizeof(syslog_status));
	get_first_xmlnode(XML_FPATH,NODE_SAVECYCLE,&syslog_status);
	if(0 != strcmp(syslog_status,""))
	{
		memset(cmdstr,0,sizeof(cmdstr));
		sprintf(cmdstr," syslog savecycle %s\n",syslog_status);
		vtysh_add_show_string(cmdstr);
	}
	vtysh_add_show_string( " exit\n" );
	///////////////////////////////////
	return CMD_SUCCESS;	
}

void dcli_syslog_init
(
	void
)  
{
	install_node( &syslog_node, dcli_syslog_show_running_config, "SYSLOG_NODE");
	install_default(SYSLOG_NODE);

	install_element(CONFIG_NODE, &conf_syslog_cmd);
	install_element(CONFIG_NODE, &logger_syslog_cmd);
	install_element(SYSLOG_NODE, &contrl_service_cmd);
	
	install_element(SYSLOG_NODE, &show_syslog_info_cmd);

	install_element(SYSLOG_NODE, &add_syslog_des_ruler_cmd);
	//install_element(SYSLOG_NODE, &del_syslog_des_ruler_cmd); 
	//install_element(SYSLOG_NODE, &syslog_extend_command_cmd); 
	//install_element(SYSLOG_NODE, &syslog_extend_all_command_cmd); 	
	install_element(SYSLOG_NODE, &syslog_synflood_service_cmd); 
	install_element(SYSLOG_NODE, &syslog_eaglog_service_cmd); 
	install_element(VIEW_NODE, &save_syslog_info_cmd);
	install_element(ENABLE_NODE,&save_syslog_info_cmd);
	install_element(SYSLOG_NODE,&save_syslog_info_cmd);
	install_element(VIEW_NODE, &upload_syslog_info_cmd);
	install_element(ENABLE_NODE,&upload_syslog_info_cmd);
	install_element(SYSLOG_NODE,&upload_syslog_info_cmd);
	install_element(SYSLOG_NODE,&show_syslog_user_cmd);
	install_element(SYSLOG_NODE,&modify_syslog_user_cmd);
	install_element(SYSLOG_NODE,&set_syslog_savecycle_cmd);

	install_element(VIEW_NODE,&delete_syslog_cmd);
	install_element(ENABLE_NODE,&delete_syslog_cmd);
	install_element(SYSLOG_NODE,&delete_syslog_cmd);

	install_element(SYSLOG_NODE,&show_syslog_filter_cmd);
	install_element(SYSLOG_NODE,&add_syslog_filter_ruler_cmd);
	install_element(SYSLOG_NODE,&delete_syslog_filter_ruler_cmd);
	install_element(SYSLOG_NODE,&show_syslog_destination_cmd);
	install_element(SYSLOG_NODE,&add_des_ruler_cmd);
	install_element(SYSLOG_NODE,&delete_des_ruler_cmd);	
	install_element(SYSLOG_NODE,&show_syslog_log_cmd);	
	install_element(SYSLOG_NODE,&delete_log_ruler_cmd);	
	install_element(SYSLOG_NODE,&add_log_ruler_cmd);	
	
}

///////////////////////////////////////////
#ifdef __cplusplus
}
#endif

