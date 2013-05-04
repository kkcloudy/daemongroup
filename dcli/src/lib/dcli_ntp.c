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
* dcli_ntp.c
*
* MODIFY:
*		by <zhouym@autelan.com> on 2010-03-17 15:09:03 revision <0.1>
*		
* CREATOR:
*		zhouym@autelan.com
*		
* UPDATE:
*		zengxx@autelan.com
*			
* DESCRIPTION:
*		CLI definition for ntp module.
*		
* DATE:
*		2010-03-17 12:00
*		2011-12-9   16:50
*				
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.8 $	
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

#include <string.h>
#include <stdio.h>

#include <zebra.h>
#include <dbus/dbus.h>
#include <stdlib.h>
#include <sysdef/npd_sysdef.h>
#include <dbus/npd/npd_dbus_def.h>
#include "vty.h"
#include "command.h"
#include "if.h"
#include "sysdef/returncode.h"
#include "dcli_ntp.h"  
#include "ws_log_conf.h"
#include "ws_returncode.h"
#include <sys/wait.h>
#include <ctype.h>

#include "ws_dbus_list.h"
#include "dcli_main.h"
#include "ac_manage_def.h"
#include "ac_manage_extend_interface.h"
#include "ac_manage_ntpsyslog_interface.h"
#include "ws_public.h"
////////////////////////////////////////////////////////

struct cmd_node ntp_node = 
{
	NTP_NODE,
	"%s(config-ntp)# "
};
static void
dcli_show_ntp_rule_info(DBusConnection *connection, u_int slot_id, struct vty *vty) {
	if(NULL == connection || NULL == vty) {
		return;
	}
	
	int ret;
	struct clientz_st *rule_array = NULL;
	u_long rule_num = 0;
	ret = ac_manage_show_ntp_rule(connection, NULL, &rule_array, &rule_num);
	if(AC_MANAGE_SUCCESS == ret && rule_array && rule_num) {
		vty_out(vty, "----------------------------------------------------------------------\n");

		int i = 0;
		for(i = 0; i < rule_num; i++) {
			//firewall_show_rule_entry(&rule_array[i], vty);
			vty_out(vty,"rule_array[%d] is: %s\n",i,&rule_array[i].clitipz);
			vty_out(vty,"rule_array[%d] is: %s\n",i,&rule_array[i].ifper);
			vty_out(vty,"rule_array[%d] is: %s\n",i,&rule_array[i].timeflag);
			vty_out(vty,"rule_array[%d] is: %s\n",i,&rule_array[i].slotid);			
			vty_out(vty, "----------------------------------------------------------------------\n");
		}
		Free_read_ntp_client(rule_array);
	}

    return;
}


int
ac_manage_show_ntp_running_config(DBusConnection *connection, unsigned int mode, struct vty *vty) {
    
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;

	int ret = AC_MANAGE_SUCCESS;
	unsigned int moreConfig = 0;
	char *showStr = NULL;
	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME, 
											AC_MANAGE_NTP_DBUS_OBJPATH,
											AC_MANAGE_NTP_DBUS_INTERFACE,
											AC_MANAGE_DBUS_SHOW_NTP_RUNNING_CONFIG);

    dbus_error_init(&err);
    
    dbus_message_append_args(query,
                            DBUS_TYPE_UINT32, &mode,
                            DBUS_TYPE_INVALID);

    reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);

    dbus_message_unref(query);

    if(NULL == reply) {
        if(dbus_error_is_set(&err)) {
            dbus_error_free_for_dcli(&err);
        }
        return AC_MANAGE_DBUS_ERROR;
    }
    dbus_message_iter_init(reply, &iter);
    dbus_message_iter_get_basic(&iter, &ret);

	
    dbus_message_iter_next(&iter);  
    dbus_message_iter_get_basic(&iter, &moreConfig);

    while(moreConfig) {
    
        dbus_message_iter_next(&iter);  
        dbus_message_iter_get_basic(&iter, &showStr);

		
		    //vty_out(vty, "%s\n", showStr);
            vtysh_add_show_string(showStr);

        dbus_message_iter_next(&iter);  
        dbus_message_iter_get_basic(&iter, &moreConfig);
    }    

    dbus_message_unref(reply);

    return ret;
    	                                     
}

DEFUN(show_ntp_client_info_func,
	show_ntp_client_info_cmd,
	"show ntpclient info",
	SHOW_STR
	"ntp client configure\n"
	"configure inforamtion\n"
)
{
	vty_out(vty, "========================================================================\n");

	int i = 1; 
	for(i = 1; i < MAX_SLOT; i++) {
		if(dbus_connection_dcli[i]->dcli_dbus_connection) {
			vty_out(vty, "Slot %d NTP Rule Info\n", i);
			dcli_show_ntp_rule_info(dbus_connection_dcli[i]->dcli_dbus_connection, i, vty);
			vty_out(vty, "========================================================================\n");
		}
	}

	return CMD_SUCCESS;
}
static void
dcli_add_upper_ntp(DBusConnection *connection, struct clientz_st *rule_array,struct vty *vty) {
	if(NULL == connection || NULL == vty) {
		return;
	}
	int ret = 0;
	int config_type = 0;
	ret = ac_manage_add_ntpserver_rule(connection, rule_array, config_type);
    return;
}
//config ntp 
DEFUN(conf_ntp_func,
	conf_ntp_cmd,
	"config ntp",
	CONFIG_STR
	"config  Network Time Protocol module!\n"
)

{
	char cmd[128]={0};
	if(CONFIG_NODE == vty->node)
	{
		vty->node = NTP_NODE;
		if_ntp_exist ();		
	}
	else
	{
		vty_out (vty, "Terminal mode change must under configure mode!\n");
		return CMD_WARNING;
	}
	return CMD_SUCCESS;

}


//ntp service (enable | disable) disable 
DEFUN(contrl_ntp_service_func,
	contrl_ntp_service_cmd,
	"service (enable | disable |reload)",
	"ntp service config\n" 
	"enable ntp service \n"
	"disable ntp service \n"
	"reload ntp service \n"
)
{
	int i = 0;
	int status = 0;	
	char state[20] = {0};
	int p_masterid = 0;
	p_masterid = get_product_info(PRODUCT_ACTIVE_MASTER);
	char ifname[32] = {0};
	if (NTP_NODE != vty->node) 
	{
		vty_out (vty, "Terminal mode change must under configure mode!\n");
		return CMD_WARNING;
	}
	if (!strcmp(argv[0], "enable"))
	{
		status = start_ntp();
		memset(state,0,sizeof(state));
		strncpy(state,"enable",sizeof(state)-1);
	}
	else if(!strcmp(argv[0], "disable"))
	{	
		status = stop_ntp();
		memset(state,0,sizeof(state));
		strncpy(state,"disable",sizeof(state)-1);
	}
	else if(!strcmp(argv[0], "reload"))
	{
		status = restart_ntp();
		memset(state,0,sizeof(state));
		strncpy(state,"enable",sizeof(state)-1);
	}
	if(status == -1)
	{
		vty_out(vty, "operation is failed ,you should contact admin!\n");
		return CMD_WARNING;
	}
	mod_first_xmlnode(NTP_XML_FPATH, NODE_LSTATUS, state);
	for(i = 1; i < MAX_SLOT; i++)
	{
		if(dbus_connection_dcli[i]->dcli_dbus_connection) 
		{
			ac_manage_set_ntpstatus_rule(dbus_connection_dcli[i]->dcli_dbus_connection, argv[0]);
			if(p_masterid == i)
			{
				set_master_default_server_func();
			}
			else
			{
				ac_manage_inside_ntp_rule(dbus_connection_dcli[i]->dcli_dbus_connection);
				//memset(ifname,0,sizeof(ifname));
				//snprintf(ifname,sizeof(ifname)-1,"slot%d",i);
				//ac_manage_config_ntp_pfm_requestpkts(dcli_dbus_connection,ifname,1);				
			}

		}
	}
	return CMD_SUCCESS;
}

//client add A.B.C.D/X.X.X.X
DEFUN(ntp_client_add_func,
	ntp_client_add_cmd,
	"client (add|delete) A.B.C.D/X.X.X.X [SLOT]",
	"ntp client config\n"\
	"add ntp client\n"\
	"delete ntp client\n"\
	"<IP address/Subnetmask>\n"
)
{

    char sname[20] ={0};
    char strIpAddrMask[32] = {0};
    int ipvalid = -1;
	int slotid = 0;
	int retu = -1;
    if (NTP_NODE != vty->node) 
	{
		vty_out (vty, "Terminal mode change must under configure mode!\n");
		return CMD_WARNING;
	}
	
	 
	if_ntp_exist();

    strncpy(strIpAddrMask,argv[1],sizeof(strIpAddrMask)-1);   
	
	char *sntpip = NULL; 
    char *smask = strIpAddrMask;

	sntpip = strsep(&smask,"/");
	
	if(smask == NULL){
		vty_out(vty,"IP/Submask error\n");
		return CMD_WARNING;
	}
	if(!ip_address_check(sntpip))
	{
		vty_out(vty,"IP error\n");
		smask = NULL;
		return CMD_WARNING;
	}
	if(!submask_address_check(smask))
	{
		vty_out(vty,"submask error\n");
		sntpip = NULL;
		smask = NULL;
		return CMD_WARNING;
	}
	if(argc == 3)
	{
		struct serverz_st rule;
		memset(&rule,0,sizeof(rule));
		
		strcpy(rule.servipz,sntpip);
		strcpy(rule.maskz,smask);
		slotid = atoi(argv[2]);
		if((slotid < 1)||(slotid > 16))
		{
			vty_out(vty,"Slot ID is error\n");
			return CMD_SUCCESS;
		}
		if(dbus_connection_dcli[slotid]->dcli_dbus_connection) 
		{
			if(0 == strncmp(argv[0],"add",3))
			{
				retu = ac_manage_add_ntpclient_rule(dbus_connection_dcli[slotid]->dcli_dbus_connection, &rule,OPT_ADD);
				if(0 == retu)
				{
					vty_out(vty,"Add ruler successfully!\n");
				}
				else
				{
					vty_out(vty,"Add ruler failed!\n");
				}
			}
			else if(0 == strncmp(argv[0],"del",3))
			{
				retu = ac_manage_add_ntpclient_rule(dbus_connection_dcli[slotid]->dcli_dbus_connection, &rule,OPT_DEL);
				if(0 == retu)
				{
					vty_out(vty,"Delete ruler successfully!\n");
				}
				else
				{
					vty_out(vty,"Delete ruler failed!\n");
				}
			}
		}
		else
		{
			vty_out(vty,"Cannot connection designed slot!\n");
		}
	}
	else 
	{
		if(0 == strncmp(argv[0],"add",3))
		{
			if(ntp_serverip_duplication(sntpip) )
			{
				if(0 == strncmp(sntpip,"169.254.1.0",11))
				{
					vty_out(vty,"IP address duplicates the default config\n");
				}
				else
				{
					vty_out(vty,"IP address already exists\n");
				}
				smask = NULL;
				return CMD_SUCCESS;
			}
		    retu = add_ntp_server(NTP_XML_FPATH, sntpip, smask);
			if(0 == retu)
			{
				vty_out(vty,"Add ruler successfully!\n");
			}
			else
			{
				vty_out(vty,"Add ruler failed!\n");
			}

		}
		else if(0 == strncmp(argv[0],"del",3))
		{
			int flagz = 0;
			find_second_xmlnode(NTP_XML_FPATH,  NTP_SERVZ, NTP_SIPZ,sntpip,&flagz);
			if(0 != flagz)
			{
				retu = del_second_xmlnode(NTP_XML_FPATH,  NTP_SERVZ , flagz);
				if(0 == retu)
				{
					vty_out(vty,"Delete ruler successfully!\n");
				}
				else
				{
					vty_out(vty,"Delete ruler failed!\n");
				}
			}
			else
			{
					vty_out(vty,"This ruler doesnot exist!\n");
			}
		}
	    save_ntp_conf ();		
	}	
	sntpip = NULL; 
    smask = NULL;
	return CMD_SUCCESS;	
}

//server add A.B.C.D (on | off)
DEFUN(ntp_server_add_func,
	ntp_server_add_cmd,
	"server (add|delete) A.B.C.D (on | off)",
	"ntp server config\n"
	"add ntp server\n"\
	"delete ntp server\n"\
	"<IP address>\n"\
	"Priority on\n"\
	"Priority off\n"
)
{

    char cntpip[16] = {0};
    char cperz[10] ={0};
	int slotid = 0;
	int ret = -1;
	u_long config_type = 0;

    if (NTP_NODE != vty->node) 
	{
		vty_out (vty, "Terminal mode change must under configure mode!\n");
		return CMD_WARNING;
	}	
	if_ntp_exist();
	
   	strncpy(cntpip,argv[1],sizeof(cntpip)-1);
    strncpy(cperz,argv[2],sizeof(cperz) - 1);
	if ( !ip_address_check(cntpip) )
    {
		vty_out(vty, "IP error\n");
		return CMD_WARNING;
	}
	if(0 == strncmp(argv[0],"add",3))
	{
		if(ntp_clientip_duplication(cntpip))
		{
			vty_out(vty,"IP address already exists\n");
			return CMD_SUCCESS;
		}
	    ret = add_ntp_client (NTP_XML_FPATH, cntpip, cperz);
		if(ret == 0)
		{
			vty_out(vty,"Add ruler successfully!\n");
		}
		else
		{
			vty_out(vty,"Add ruler failed!\n");
		}
	}
	else if(0 == strncmp(argv[0],"del",3))
	{
		int flagz = 0;
		find_second_xmlnode(NTP_XML_FPATH, NTP_CLIZ, NTP_CIPZ,cntpip,&flagz);
		if(0 != flagz)
		{
			ret = del_second_xmlnode(NTP_XML_FPATH, NTP_CLIZ , flagz);
			if(ret == 0)
			{
				vty_out(vty,"Delete ruler successfully!\n");
			}
			else
			{
				vty_out(vty,"Delete ruler failed!\n");
			}
		}
		else
		{
				vty_out(vty,"This ruler doesnot exist!\n");
		}
	}
    save_ntp_conf ();
	return CMD_SUCCESS;	
	
}


//server add A.B.C.D (on | off)
DEFUN(ntp_serverupper_config_func,
	ntp_serverupper_config_cmd,
	"upperserver (add|delete) A.B.C.D (on | off) IFNAME SLOT",
	"ntp server config\n"
	"add ntp upper server\n"\
	"delete ntp upper server\n"\
	"<IP address>\n"\
	"Priority on\n"\
	"Priority off\n"
	"interface on slot\n"
	"slot id\n"
)
{

    char cntpip[16] = {0};
    char cperz[10] ={0};
	int slotid = 0;
	int ret = 0;
	char ifname[64] = {0};
	char veifname[64] = {0};
	int if_ve = 0;
    if (NTP_NODE != vty->node) 
	{
		vty_out (vty, "Terminal mode change must under configure mode!\n");
		return CMD_WARNING;
	}

   	strncpy(cntpip,argv[1],sizeof(cntpip)-1);
    strncpy(cperz,argv[2],sizeof(cperz) - 1);

	int p_masterid = 0;
	p_masterid = get_product_info(PRODUCT_ACTIVE_MASTER);

	if(NULL == argv[3])
	{
		vty_out (vty, "Interface name connont be null!\n");
		return CMD_SUCCESS;
	}
	strncpy(ifname,argv[3],sizeof(ifname)-1);
	if_ve = ve_interface_parse(ifname, &veifname, sizeof(veifname));
	if(0 != if_ve)
	{
		memset(veifname,0,sizeof(veifname));
		strncpy(veifname,ifname,sizeof(veifname)-1);
	}
	slotid = atoi(argv[4]);
	if((slotid < 1)||(slotid > 16))
	{
		vty_out(vty,"Slot ID is error\n");
		return CMD_SUCCESS;
	}
	if(p_masterid == slotid)
	{
		vty_out(vty,"Slot ID cannot be master\n");
		return CMD_SUCCESS;
	}
	struct clientz_st rule_array;
	memset(&rule_array,0,sizeof(rule_array));
	
	strcpy(rule_array.clitipz,cntpip);
	strcpy(rule_array.ifper,cperz);
	strncpy(rule_array.slotid,argv[3],sizeof(rule_array.slotid)-1);
	
	if(dbus_connection_dcli[slotid]->dcli_dbus_connection) 
	{
		if(0 == strncmp(argv[0],"add",3))
		{
			ret = ac_manage_add_ntpserver_rule(dbus_connection_dcli[slotid]->dcli_dbus_connection, &rule_array, OPT_ADD);
			ac_manage_config_ntp_pfm_requestpkts(dcli_dbus_connection, veifname, cntpip,OPT_ADD);	
			if(0 == ret)
			{
				vty_out(vty,"Add ruler successfully!\n");
			}
			else
			{
				vty_out(vty,"Add ruler failed!\n");
			}
		}
		else if(0 == strncmp(argv[0],"del",3))
		{
			ret = ac_manage_add_ntpserver_rule(dbus_connection_dcli[slotid]->dcli_dbus_connection, &rule_array, OPT_DEL);
			ac_manage_config_ntp_pfm_requestpkts(dcli_dbus_connection, veifname, cntpip,OPT_DEL);	
			if(0 == ret)
			{
				vty_out(vty,"Delete ruler successfully!\n");
			}
			else
			{
				vty_out(vty,"Delete ruler failed!\n");
			}

		}
	}
	else
	{
		vty_out(vty,"Cannot connection designed slot!\n");
	}
	return CMD_SUCCESS;	
	
}

DEFUN(ntp_type_config_func,
	ntp_type_config_cmd,
	"ntptype (ntpv3 | ntpv4)",
	"ntp type config\n"\
	"ntpv3\n"\
	"ntpv4"
)
{

	char ntptype[10] = {0};

    if (NTP_NODE != vty->node) 
	{
		vty_out (vty, "Terminal mode change must under configure mode!\n");
		return CMD_WARNING;
	}
	
	if_ntp_exist();

	strncpy(ntptype,argv[0],sizeof(ntptype)-1);
	
	if(!strcmp(ntptype,"ntpv3"))  
		mod_first_xmlnode(NTP_XML_FPATH, "ntpv", "3");
	
	if(!strcmp(ntptype,"ntpv4"))
		mod_first_xmlnode(NTP_XML_FPATH, "ntpv", "4");

	return CMD_SUCCESS;	
}


DEFUN(ntp_ipv4type_config_func,
	ntp_ipv4type_config_cmd,
	"iptype ipv4 (listen|ignore)",
	"ntp iptype config\n"\
	"ipv4 configure\n"
	"ipv4 listen\n"\
	"ipv4 ignore"
)
{
	char str_ipv4type[10] = {0};

    if (NTP_NODE != vty->node) 
	{
		vty_out (vty, "Terminal mode change must under configure mode!\n");
		return CMD_WARNING;
	}
    
	if_ntp_exist();//找对应的xml文件,不存在则创建

	strncpy(str_ipv4type,argv[0],9);
	
	if(!strcmp(str_ipv4type,"listen"))  
		mod_first_xmlnode(NTP_XML_FPATH, NODE_IPV4, "listen");
	
	if(!strcmp(str_ipv4type,"ignore"))
		mod_first_xmlnode(NTP_XML_FPATH, NODE_IPV4, "ignore");

	return CMD_SUCCESS;	

}


DEFUN(ntp_ipv6type_config_func,
	ntp_ipv6type_config_cmd,
	"iptype ipv6 (listen|ignore)",
	"ntp iptype config\n"\
	"ipv6 configure\n"
	"ipv6 listen\n"\
	"ipv6 ignore"
)
{
	char str_ipv6type[10] = {0};
	memset(str_ipv6type,0,sizeof(str_ipv6type));

    if (NTP_NODE != vty->node) 
	{
		vty_out (vty, "Terminal mode change must under configure mode!\n");
		return CMD_WARNING;
	}
    
	if_ntp_exist();//找对应的xml文件,不存在则创建

	strcpy(str_ipv6type,argv[0]);
	
	if(!strcmp(str_ipv6type,"listen"))  
		mod_first_xmlnode(NTP_XML_FPATH, NODE_IPV6, "listen");
	
	if(!strcmp(str_ipv6type,"ignore"))
		mod_first_xmlnode(NTP_XML_FPATH, NODE_IPV6, "ignore");
	
	return CMD_SUCCESS;	

}


//Synchronous period 
DEFUN(ntp_cronttab_config_func,
	ntp_cronttab_config_cmd,
	"cronttab (minute | hour | day) TIME",
	"ntp cronttab config\n"\
	"minute 10~59\n"
	"hour 1~23\n"\
	"day 1~31\n"\
	"cronttab time"
)
{
	
	char type[10] = {0};
	char time[10] = {0};
	char time_t[10] = {0};	
	char cmd[128]  = {0};
	int flag_t = 0;
	int temp = 0;
	
	if (NTP_NODE != vty->node) 
	{
		vty_out (vty, "Terminal mode change must under configure mode!\n");
		return CMD_WARNING;
	}
	
	if_ntp_exist();

	strncpy(type,argv[0],sizeof(type) - 1);
	strncpy(time,argv[1],sizeof(time) - 1);
	
	flag_t = strcmp(type,"hour");
	
	if(flag_t < 0){
		temp = atoi(argv[1]);
		if(temp > 0 && temp < 32){
			sprintf(time_t,"%d^%s",temp,"days");
		}
		else {
			vty_out(vty,"input is overflow\n");
			return CMD_WARNING;
		}	
	}
	else if(flag_t == 0){
		temp = atoi(argv[1]);
		if(temp > 0 && temp < 32){
			sprintf(time_t,"%d^%s",temp,"hours");
		}
		else {
			vty_out(vty,"input is overflow\n");
			return CMD_WARNING;
		}
	}
	else{
		temp = atoi(argv[1]);
		if(temp > 9 && temp < 60){
			sprintf(time_t,"%d^%s",temp,"mins");
		}
		else {
			vty_out(vty,"input is overflow\n");
			return CMD_WARNING;
		}
	}
	
	mod_first_xmlnode(NTP_XML_FPATH, "cront",time_t);
	save_ntp_conf ();
	strcat(cmd,"sudo cronntp.sh > /dev/null");
	int status = system(cmd);
	if(status == -1){
		vty_out(vty,"operation is failed ,you should contact admin!\n");
		return CMD_WARNING;
	}
	
	return CMD_SUCCESS;	
}

DEFUN(show_ntp_config_func,
	show_ntp_config_cmd,
	"show configure",
	"show ntp config inforamtion\n"
	"ntp configuration"
)
{

	if (NTP_NODE != vty->node) 
	{
		vty_out (vty, "Terminal mode change must under ntp mode!\n");
		return CMD_WARNING;
	}
	if_ntp_exist();

	struct clientz_st clitst ,*cq,upclitst ,*uq;
	struct serverz_st servst ,*sq;
	
	memset(&clitst,0,sizeof(clitst));
	memset(&upclitst,0,sizeof(upclitst));
	memset(&servst,0,sizeof(servst));

	int clinum=0;    
	int upclinum = 0;
	int servnum  =  0;
	int count  =  0;
	
	int ntpstatus =0;
	ntpstatus = if_ntp_enable();
	
	vty_out(vty,"---------------------------------------\n");

	if(ntpstatus == 0)
	{
		vty_out(vty,"Status:  disable!\n");
	}

    
	else
	{
		vty_out(vty,"Status: enable!\n");
	}

	vty_out(vty,"---------------------------------------\n");
	
	read_ntp_client(NTP_XML_FPATH, &clitst, &clinum);
	read_upper_ntp(NTP_XML_FPATH, &upclitst, &upclinum);
	cq=clitst.next;
	uq=upclitst.next;
	
	count=0;
	vty_out(vty,"Server  \n");
	vty_out(vty,"id\tip\t\tpriority\tupper\n");
	while(uq!=NULL)
	{
	   count++;   
       vty_out(vty,"%d\t%-20s\t",count,uq->clitipz);
	   vty_out(vty,"%-10s",uq->ifper); 
	   
	   vty_out(vty,"%-10s\n","yes");
	   
       
       uq = uq->next;

	}
	while(cq!=NULL)
	{
	   if(0 != strncmp(cq->timeflag,"def",3))
	   	{
		   count++;
	       vty_out(vty,"%d\t%-20s\t",count,cq->clitipz);
		   vty_out(vty,"%-10s",cq->ifper); 
		   
	       vty_out(vty,"%-10s\n","no");
	   	}
       cq = cq->next;

	}

	vty_out(vty,"---------------------------------------\n");
	
	if(clinum>0)
    {   
		Free_read_ntp_client(&clitst);
        
    }
	if(upclinum>0)
    {   
		Free_read_upper_ntp(&upclitst);
        
    }

	count = 0;
	
	read_ntp_server(  NTP_XML_FPATH,   &servst,   &servnum  );

    sq = servst.next;                    

	vty_out(vty,"Client\n");

	vty_out(vty,"id\tip\t\tmask\t\n");
    while(sq!=NULL)
     {
		if(0 != strncmp(sq->timeflag,"def",3))
		{
			count++;       
	        vty_out(vty,"%d\t",count);

			vty_out(vty , "%s\t",sq->servipz);

			vty_out(vty , "%s\n",sq->maskz);	

		}
        sq = sq->next;
                                        
     }  
	
    vty_out(vty,"---------------------------------------\n");        
      if(servnum>0)
      {                  
         Free_read_ntp_server(&servst);                                            
      }      
	  
    char gets_ntptype[10] = {0};   

	get_first_xmlnode(NTP_XML_FPATH,"ntpv",gets_ntptype);

	vty_out(vty,"NTP type : NTPV%s\n",gets_ntptype);

	vty_out(vty,"---------------------------------------\n"); 

	char gets_cronttab[10] = {0};
	
	get_first_xmlnode(NTP_XML_FPATH, "cront",gets_cronttab);

	vty_out(vty,"Sync period : %s\n",gets_cronttab);

	vty_out(vty,"---------------------------------------\n"); 

	char gets_ipv4[10] = {0};
	
	get_first_xmlnode(NTP_XML_FPATH, NODE_IPV4, gets_ipv4);

	vty_out(vty,"IPV4 : %s\t",gets_ipv4);
		
	char gets_ipv6[10] = {0};
	get_first_xmlnode(NTP_XML_FPATH, NODE_IPV6, gets_ipv6);

	vty_out(vty,"IPV6 : %s\n",gets_ipv6);

	vty_out(vty,"---------------------------------------\n"); 
	
	return CMD_SUCCESS;

}

DEFUN(ntp_clean_configure_func,
	ntp_clean_configure_cmd,
	"clean ntpconfigure (SLOTID|all)",
	"clean ntpconfigure command\n"
	"clean ntpconfigure command\n"
    "slot ID\n"
    "slot all\n"
    "exec set localserver command\n"
)
{
	int i = 0;	
	for(i = 1; i < MAX_SLOT; i++)
	{
		if(dbus_connection_dcli[i]->dcli_dbus_connection) 
		{
			ac_manage_clean_ntp_rule(dbus_connection_dcli[i]->dcli_dbus_connection);

		}
	}	
	return CMD_SUCCESS;
}

int  dcli_ntp_show_running_config(struct vty* vty)
{

	char showStr[256] = {0};
	char newc[10] = {0};
	int ret = -1;
    if_ntp_exist();
	vtysh_add_show_string("!ntp section \n");
	vtysh_add_show_string("config ntp \n");		

    int i = 1;
    for(i = 1; i < MAX_SLOT; i++) {
        if(dbus_connection_dcli[i]->dcli_dbus_connection) {
            ac_manage_show_ntp_running_config(dbus_connection_dcli[i]->dcli_dbus_connection, SHOW_RUNNING_NTP_CONFIG, vty);
        }
    }
	memset(newc,0,sizeof(newc));
    get_first_xmlnode(NTP_XML_FPATH,NODE_LSTATUS,&newc);		
	if(0 != strcmp(newc,""))
	{
			memset(showStr,0,sizeof(showStr));
			snprintf(showStr, sizeof(showStr)-1, " service %s \n",newc);
			vtysh_add_show_string(showStr);
	}
	vtysh_add_show_string(" exit\n");
    return CMD_SUCCESS;

}

DEFUN(show_ntp_running_config_func,
	show_ntp_running_config_cmd,
	"show ntp_running_config",
	SHOW_STR
	"show ntp running config\n" 
)
{
    vty_out(vty, "============================================================================\n");
    
    int i = 1;
    for(i = 1; i < MAX_SLOT; i++) {
        if(dbus_connection_dcli[i]->dcli_dbus_connection) {
			vty_out(vty,"============== SLOT %d ==========================\n",i);
            ac_manage_show_ntp_running_config(dbus_connection_dcli[i]->dcli_dbus_connection, SHOW_RUNNING_NTP_CONFIG, vty);
        }
    }

    vty_out(vty, "============================================================================\n");
	return CMD_SUCCESS;
}


void dcli_ntp_init
(
	void
)  
{
	install_node(&ntp_node, dcli_ntp_show_running_config,"NTP_NODE");
    
	install_default(NTP_NODE);

	install_element(CONFIG_NODE, &conf_ntp_cmd);
    
	install_element(NTP_NODE, &contrl_ntp_service_cmd);
	
	//install_element(NTP_NODE, &show_ntp_running_config_cmd);
	install_element(NTP_NODE, &ntp_client_add_cmd);
    install_element(NTP_NODE, &ntp_server_add_cmd);
	install_element(NTP_NODE, &ntp_serverupper_config_cmd);	
    
    //install_element(NTP_NODE, &show_ntp_server_cmd);
    //install_element(NTP_NODE, &show_ntp_client_cmd);
    
    //install_element(NTP_NODE, &del_ntp_server_cmd); 
    //install_element(NTP_NODE, &del_ntp_client_cmd); 

	install_element(NTP_NODE, &ntp_ipv4type_config_cmd); 

	install_element(NTP_NODE, &ntp_ipv6type_config_cmd); 

	install_element(NTP_NODE, &ntp_type_config_cmd); 
	
	install_element(NTP_NODE, &ntp_cronttab_config_cmd); 

	install_element(NTP_NODE, &show_ntp_config_cmd); 
	install_element(NTP_NODE, &ntp_clean_configure_cmd);
	//install_element(NTP_NODE, &show_ntp_client_info_cmd);
	
	
}


///////////////////////////////////////////
#ifdef __cplusplus
}
#endif

