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
* dcli_debug.c
*
*
* CREATOR:
*		caochao@autelan.com
*
* DESCRIPTION:
*		CLI definition for debug module.
*
* DATE:
*		2010-11-29 15:09:11
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.2 $	
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************
*	head files														*
**********************************************************/
#include <zebra.h>
#include <dbus/dbus.h>
#include <stdlib.h>
#include <sysdef/npd_sysdef.h>
#include <dbus/npd/npd_dbus_def.h>
#include <dbus/sem/sem_dbus_def.h>

#include "dcli_routesyn.h"

#include "command.h"
#include "dcli_main.h"


extern int execute_flag;

struct cmd_node system_debug_node = 
{
	SYSTEM_DEBUG_NODE,
	"%s(config-sys-debug)# ",
	1
};
static void dcli_route_show_drvroute
(
	struct vty *vty,
	unsigned int dip,
	unsigned int maskLen
)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	unsigned int nexthopindex = 0,ifindex = 0;
	unsigned char isTrunk = 0;
	unsigned short vid = 0;
	unsigned char	macAddr[6] = {0},devNum = 0,portNum = 0;
	int op_ret = 0;
	unsigned int refCnt = 0;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,RTDRV_DBUS_OBJPATH,RTDRV_DBUS_INTERFACE,RTDRV_DBUS_METHOD_SHOW_ENTRY);
		
	dbus_error_init(&err);
	
	dbus_message_append_args(query,
					 DBUS_TYPE_UINT32,&dip,
					 DBUS_TYPE_UINT32,&maskLen,
					 DBUS_TYPE_INVALID);
		
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
		
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	
	if (!dbus_message_get_args ( reply, &err,
							DBUS_TYPE_UINT32, &op_ret,
							DBUS_TYPE_UINT32, &nexthopindex,
							DBUS_TYPE_UINT16, &vid,
							DBUS_TYPE_UINT32, &ifindex,
							DBUS_TYPE_BYTE, &isTrunk,							
							DBUS_TYPE_BYTE, &devNum,
							DBUS_TYPE_BYTE, &portNum,
							DBUS_TYPE_BYTE, &macAddr[0],
							DBUS_TYPE_BYTE, &macAddr[1],
							DBUS_TYPE_BYTE, &macAddr[2],
							DBUS_TYPE_BYTE, &macAddr[3],
							DBUS_TYPE_BYTE, &macAddr[4],
							DBUS_TYPE_BYTE, &macAddr[5],
							DBUS_TYPE_UINT32,&refCnt,
							DBUS_TYPE_INVALID)) 
	{
			vty_out(vty,"can't get the route entry msg.\n");
	}	
	dbus_message_unref(reply);
	dip = ntohl(dip);
	if((0 == op_ret) || 
		(DCLI_ROUTE_RETCODE_ACTION_TRAP2CPU == op_ret)||
		(DCLI_ROUTE_RETCODE_ACTION_HARD_DROP== op_ret)) {
		vty_out(vty,"%-16s%-5s%-7s%-8s%-7s%-7s%-6s%-18s%-5s\n",		\
				"===============","====","======","=======","======","======","=====","=================","=====");
		vty_out(vty,"%-4s%-12s%-5s%-7s%-8s%-7s%-7s%-6s%-3s%-15s%-6s\n",	\
				"","dest-ip","mask","hw-idx","ifindex","device",isTrunk?"trunk":"port","vlan","","mac-address","ref");
		vty_out(vty,"%-16s%-5s%-7s%-8s%-7s%-7s%-6s%-18s%-5s\n",		\
				"---------------","----","------","-------","------","------","-----","-----------------","-----");
		if(DCLI_ROUTE_RETCODE_ACTION_TRAP2CPU == op_ret) {
			vty_out(vty,"%-3d.%-3d.%-3d.%-3d %-4d %-6d %-7s %-6s %-6s %-5s %-17s %-5s\n", \
					(dip>>24)&0xFF,(dip>>16)&0xFF,(dip>>8)&0xFF,dip&0xFF,maskLen,nexthopindex, 	\
					"-","-","CPU","-","-","-");
		}
		if(DCLI_ROUTE_RETCODE_ACTION_HARD_DROP == op_ret) {
			vty_out(vty,"%-3d.%-3d.%-3d.%-3d %-4d %-6d %-7s %-6s %-6s %-5s %-17s %-5s\n", \
					(dip>>24)&0xFF,(dip>>16)&0xFF,(dip>>8)&0xFF,dip&0xFF,maskLen,nexthopindex, 	\
					"-","-","DROP","-","-","-");
		}
		if( 0 == op_ret) {
			vty_out(vty,"%-3d.%-3d.%-3d.%-3d %-4d %-6d %-7d %-6d %-6d %-5d %02X:%02X:%02X:%02X:%02X:%02X %-4d\n", \
					(dip>>24)&0xFF,(dip>>16)&0xFF,(dip>>8)&0xFF,dip&0xFF,maskLen,nexthopindex, 	\
					ifindex,devNum,portNum,vid,macAddr[0],macAddr[1],macAddr[2],macAddr[3],	\
					macAddr[4],macAddr[5],refCnt);
		}
	}

	else {
		vty_out(vty,"can't get the route entry.\n");
	}

	return;
}



execute_command (const char *command, int argc, const char *arg1,
		 const char *arg2,const char *arg3)
{
  int ret;
  pid_t pid;
  int status;

  /* Call fork(). */
  pid = fork ();

  if (pid < 0)
    {
      /* Failure of fork(). */
      fprintf (stderr, "Can't fork: %s\n", safe_strerror (errno));
      exit (1);
    }
  else if (pid == 0)
    {
      /* This is child process. */
      switch (argc)
	{
	case 0:
	  ret = execlp (command, command, (const char *)NULL);
	  break;
	case 1:
	  ret = execlp (command, command, arg1, (const char *)NULL);
	  break;
	case 2:
		ret = execlp (command, command, arg1, arg2, (const char *)NULL);
		break;
	case 3:
		ret = execlp (command, command, arg1, arg2, arg3, (const char *)NULL);
		break;
	}

      /* When execlp suceed, this part is not executed. */
      fprintf (stderr, "Can't execute %s: %s\n", command, safe_strerror (errno));
      exit (1);
    }
  else
    {
      /* This is parent. */
      execute_flag ++;
      ret = wait4 (pid, &status, 0, NULL);
      execute_flag --;
    }
  return 0;
}


DEFUN(show_route_mvdrv_entry_cmd_func,
	show_route_mvdrv_entry_cmd,
	"show drvroute A.B.C.D A.B.C.D",
	SHOW_STR
	"Route table infomation on MARVELL driver \n"
	"IP address. eg:192.168.1.0\n"
	"IP mask length. eg:255.255.255.0\n"
)
{
	unsigned int DIP,masklen;

	if(argc != 2)
	{
		vty_out(vty,"parameter number is error\n");
		return CMD_SUCCESS;
	}
	if(1 != inet_atoi(argv[0],&DIP))
	{
		vty_out(vty,"can't get ip address\n");
		return CMD_SUCCESS;

	}
	masklen = get_ip_masklen(argv[1]);


	dcli_route_show_drvroute(vty,DIP,masklen);
	return CMD_SUCCESS;
}


DEFUN(show_route_mvdrv_entry1_cmd_func,
		show_route_mvdrv_entry1_cmd,
		"show drvroute A.B.C.D/M",
		SHOW_STR
		"Route table infomation on MARVELL driver \n"
		"IP address and masklen. eg:192.168.1.0/24\n"
	)
{
	unsigned int DIP,masklen;
	unsigned char ipaddrstr[17];
	int addrlen;

	if(argc != 1)
	{
		vty_out(vty,"parameter number is error\n");
		return CMD_SUCCESS;
	}
	addrlen=strchr(argv[0],'/')-argv[0];
	memcpy(ipaddrstr,argv[0],addrlen);
	ipaddrstr[addrlen]='\0';
	
	if(1 != inet_atoi(ipaddrstr,&DIP))
	{
		vty_out(vty,"can't get ip address\n");
		return CMD_SUCCESS;

	}
	masklen = atoi(argv[0]+addrlen+1);
	dcli_route_show_drvroute(vty,DIP,masklen);
	return CMD_SUCCESS;
}

DEFUN (vtysh_ping,
       vtysh_ping_cmd,
       "ping WORD",
       "Send echo messages\n"
       "Ping destination address or hostname\n")
{
  execute_command ("ping", 1, argv[0], NULL,NULL);
  return CMD_SUCCESS;
}

ALIAS (vtysh_ping,
       vtysh_ping_ip_cmd,
       "ping ip WORD",
       "Send echo messages\n"
       "IP echo\n"
       "Ping destination address or hostname\n")

       
DEFUN (vtysh_ping_c_arg,
       vtysh_ping_count_cmd,
       "ping WORD -c  COUNT",
       "Send echo messages\n"
       "Ping destination address or hostname\n"
	   "The numer of ping count\n"
	   "The count\n"
		)
{
	char buf[10] = "-c";
	execute_command("ping",3,argv[0],buf,argv[1]);
	return CMD_SUCCESS;
}

DEFUN (vtysh_ping_t_arg,
       vtysh_ping_time_cmd,
       "ping WORD -t TTL",
       "Send echo messages\n"
       "Ping destination address or hostname\n"
	   "The ttl number of send packets\n"
	   "Ttl\n")
{
	char buf[10] = "-t";
	execute_command("ping",3,argv[0],buf,argv[1]);
	return CMD_SUCCESS;
}

int send_reboot_dbus_message(DBusConnection* temp)
{
	
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int op_ret;
	
	query = dbus_message_new_method_call(
								SEM_DBUS_BUSNAME,			\
								SEM_DBUS_OBJPATH,		\
								SEM_DBUS_INTERFACE,	\
								SEM_DBUS_REBOOT);
	
	dbus_error_init(&err);


	op_ret = dbus_connection_send (temp,query,NULL);
	dbus_message_unref(query);
	return 0;
	#if 0
	if (NULL == reply) {
		fprintf(stderr,"failed get args.\n");
		if (dbus_error_is_set(&err)) {
			fprintf(stderr,"%s raised: %s\n",err.name,err.message);
			return -1;
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return -1;
	}
	if (dbus_message_get_args ( reply, &err,
								DBUS_TYPE_UINT32,&op_ret,
								DBUS_TYPE_INVALID)) 
	{
		;
	} 
	else {
		fprintf(stderr,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			fprintf(stderr,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	
	dbus_message_unref(reply);
	return op_ret;
#endif
}
/*
DEFUN (system_reboot,
       system_reboot_cmd,
       "reboot",
       "Reboot system\n")
{
	struct timeval tv;
//#if 0
	DBusMessage *query, *reply;
	DBusMessageIter	 iter;
	DBusError err;
	char reboot_cmd=1;
	


	query = dbus_message_new_signal(RTDRV_DBUS_OBJPATH,\
						"aw.trap","reboot");
	dbus_error_init(&err);

	dbus_message_append_args(query,
						DBUS_TYPE_BYTE,&reboot_cmd,								
						DBUS_TYPE_INVALID);

	dbus_connection_send (dcli_dbus_connection,query,NULL);
	
	dbus_message_unref(query);	

	

	dcli_send_dbus_signal("user_reboot", "user_reboot");
	tv.tv_sec=3;
	tv.tv_usec=0;
	select(0,NULL,NULL,NULL,&tv);
	
	system ("dcli_reboot.sh");

	return CMD_SUCCESS	;
	//#endif



	if(-1 != send_reboot_dbus_message(dcli_dbus_connection))
	{
		vty_out(vty,"reboot message send OK\n");
		return CMD_SUCCESS;
	}
	else
	{
		vty_out(vty,"reboot message send ERROR\n");
		return CMD_WARNING;
	}
}
*/

DEFUN (system_reboot_all,
       system_reboot_all_cmd,
       "reboot all",
       "Reboot all\n"
       "Reboot all\n")
{
	int i;
	for(i=1;i<16;i++)
	{
		if(NULL != (dbus_connection_dcli[i] -> dcli_dbus_connection))
		{
			if(-1 != send_reboot_dbus_message((dbus_connection_dcli[i]->dcli_dbus_connection)))
			{
				vty_out(vty,"reboot message send [%d] OK\n",i);
			}
			else
			{
				vty_out(vty,"reboot message send [%d] ERROR\n",i);
			}
		}
	}
	return CMD_SUCCESS;
	
}

DEFUN (system_reboot_slot,
       system_reboot_slot_cmd,
       "reboot SLOT_ID",
       "Reboot system\n"
       "Reboot whole system\n")
{
	int i;
	sscanf(argv[0],"%d",&i);

	if((dbus_connection_dcli[i] -> dcli_dbus_connection)!= NULL)
	{
		if(-1 != send_reboot_dbus_message(dbus_connection_dcli[i]->dcli_dbus_connection))
		{
			vty_out(vty,"reboot message send [%d] OK\n",i);
		}
		else
		{
			vty_out(vty,"reboot message send [%d] ERROR\n",i);
		}
	}else{

		vty_out(vty,"slot %d is not exist\n",i);
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
	
}



DEFUN (snapshot_func,
       snapshot_func_cmd,
       "snapshot",
       "snapshot\n")
{
	int ret ;
	ret = system("sudo debugdownsnapshot.sh");
  return WEXITSTATUS(ret);
}


DEFUN (vtysh_ssh,
       vtysh_ssh_cmd,
       "ssh WORD",
       "Open an ssh connection\n"
       "[user@]host\n")
{
  execute_command ("ssh", 1, argv[0], NULL,NULL);
  return CMD_SUCCESS;
}


DEFUN (vtysh_telnet,
       vtysh_telnet_cmd,
       "telnet WORD",
       "Open a telnet connection\n"
       "IP address or hostname of a remote system\n")
{
  execute_command ("telnet", 1, argv[0], NULL,NULL);
  return CMD_SUCCESS;
}


DEFUN (vtysh_telnet_port,
       vtysh_telnet_port_cmd,
       "telnet WORD PORT",
       "Open a telnet connection\n"
       "IP address or hostname of a remote system\n"
       "TCP Port number\n")
{
  execute_command ("telnet", 2, argv[0], argv[1],NULL);
  return CMD_SUCCESS;
}


DEFUN (terminal_monitor_module_func, 
       terminal_monitor_module_func_cmd,
       "terminal monitor [KEYWORD]",
       "Set terminal line parameters\n"
       "Copy debug output to the current terminal line\n"
       "Keyn words\n")
{
	char cmdstr[128],result_str[256]; 
	int pid,ret,status ;

	memset(cmdstr,0,128);
	if(argc == 0)
		sprintf(cmdstr,"tail -f /var/log/system.log &");
	else
		sprintf(cmdstr,"tail -f /var/log/system.log | grep %s &",argv[0]);

	system(cmdstr);


#if 0
	pid = fork();
	
	if (pid < 0)
	  {
		/* Failure of fork(). */
		vty_out(vty, "Can't fork: %s\n", safe_strerror (errno));
		return CMD_WARNING;
	  }
	else if (pid == 0){
		FILE *fp;
		memset(cmdstr,0,128);
		if(argc == 0)
			sprintf(cmdstr,"tail -f /var/log/system.log &");
		else
			sprintf(cmdstr,"tail -f /var/log/system.log | grep %s &",argv[0]);
		
/*		system(cmdstr);*/
		vty_out(vty,"%s\n",result_str);
		memset(result_str,0,256);
		}
		exit(0);
	}
	else
	{
		vty_out(vty,"parents child pid = %d\n",pid);
/*
		ret = wait4 (pid, &status, 0, NULL);
*/
	}

#endif
	return CMD_SUCCESS;

	

}

DEFUN (vtysh_traceroute,
       vtysh_traceroute_cmd,
       "traceroute WORD",
       "Trace route to destination\n"
       "Trace route to destination address or hostname\n")
{
  	execute_command ("traceroute", 2, "-I", argv[0], NULL);
  return CMD_SUCCESS;
}


DEFUN (vtysh_traceroute_udp,
       vtysh_traceroute_udp_cmd,
       "traceroute WORD udp",
       "Trace route to destination\n"
       "Trace route to destination address or hostname\n"
       "Trace route to destination with udp packet\n")
{
  	execute_command ("traceroute", 1, argv[0], NULL, NULL);
  return CMD_SUCCESS;
}

ALIAS (vtysh_traceroute,
       vtysh_traceroute_ip_cmd,
       "traceroute ip WORD",
       "Trace route to destination\n"
       "IP trace\n"
       "Trace route to destination address or hostname\n")
ALIAS (vtysh_traceroute_udp,
	   vtysh_traceroute_ip_udp_cmd,
	   "traceroute ip WORD udp",
	   "Trace route to destination\n"
	   "IP trace\n"
	   "Trace route to destination address or hostname\n"
	   "Trace route to destination with udp packet\n")

#ifdef HAVE_IPV6

DEFUN (vtysh_ping6,
       vtysh_ping6_cmd,
       "ping ipv6 WORD",
       "Send echo messages\n"
       "IPv6 echo\n"
       "Ping destination address or hostname\n")
{
  execute_command ("ping6", 1, argv[0], NULL, NULL);
  return CMD_SUCCESS;
}
DEFUN (vtysh_traceroute6,
       vtysh_traceroute6_cmd,
       "traceroute ipv6 WORD",
       "Trace route to destination\n"
       "IPv6 trace\n"
       "Trace route to destination address or hostname\n")
{
  execute_command ("traceroute6", 1, argv[0], NULL,NULL);
  return CMD_SUCCESS;
}
#endif





DEFUN (no_terminal_monitor_module_func, 
       no_terminal_monitor_module_func_cmd,
       "no terminal monitor",
       NO_STR
       "Set terminal line parameters.\n"
       "Copy debug output to the current terminal line.\n")
{
	char buf[256],buf1[32],buf2[32],terminal_name[32]; 
	int pid,ppid ;
	FILE *fp;

	fp=popen("ps -ef | grep 'tail -f /var/log/system.log'","r");

	if(!fp)
	{
		return CMD_WARNING;
	}
	while( fgets( buf, 256, fp ))
	{
		/*root      1763     1  0 05:37 pts/0    00:00:00 tail -f /var/log/system.log*/
		if(5==sscanf(buf,"root      %d     %d  0 %s %s    %s tail -f /var/log/system.log",&pid,&ppid,buf1,terminal_name,buf2))
		{
			if(strstr(ttyname(0),terminal_name)&& ppid == 1)
			{
				char  cmdstr[32];
				sprintf(cmdstr,"kill %d\n",pid);
				system(cmdstr);
			}

		}

	}
	pclose(fp);
	return CMD_SUCCESS;

}



DEFUN(conf_system_debug_func,
	conf_system_debug_cmd,
	"system debug",
	"Config debug node\n"
)
{
	
	vty->node = SYSTEM_DEBUG_NODE;
	vty->index = NULL;	
	return CMD_SUCCESS;
}
#if 0 //define in vtysh.c
DEFUN (show_process,
       show_process_cmd,
       "show process",
       SHOW_STR
       "Show cpu process\n")
{
  system ("top");
  return CMD_SUCCESS;
}
#endif

void dcli_system_debug_init()
{
	install_node(&system_debug_node, NULL, "SYSTEM_DEBUG_NODE");
	install_default(SYSTEM_DEBUG_NODE);	
	install_element(CONFIG_NODE, &conf_system_debug_cmd);
//	install_element(SYSTEM_DEBUG_NODE, &show_process_cmd);
	install_element(SYSTEM_DEBUG_NODE, &show_route_mvdrv_entry_cmd);
	install_element(SYSTEM_DEBUG_NODE, &show_route_mvdrv_entry1_cmd);
	install_element(SYSTEM_DEBUG_NODE, &vtysh_ping_cmd);
	install_element(SYSTEM_DEBUG_NODE, &vtysh_ping_count_cmd);
	install_element(SYSTEM_DEBUG_NODE, &vtysh_ping_time_cmd);
	install_element(SYSTEM_DEBUG_NODE, &vtysh_ping6_cmd);
/*	install_element(SYSTEM_DEBUG_NODE, &system_reboot_cmd);*/
	install_element(SYSTEM_DEBUG_NODE, &snapshot_func_cmd);
	install_element(SYSTEM_DEBUG_NODE, &vtysh_ssh_cmd);
	install_element(SYSTEM_DEBUG_NODE, &vtysh_telnet_cmd);
	install_element(SYSTEM_DEBUG_NODE, &vtysh_telnet_port_cmd);
	install_element(SYSTEM_DEBUG_NODE, &terminal_monitor_module_func_cmd);
	install_element(SYSTEM_DEBUG_NODE, &vtysh_traceroute_udp_cmd);
	install_element(SYSTEM_DEBUG_NODE, &vtysh_traceroute6_cmd);
	install_element(SYSTEM_DEBUG_NODE, &no_terminal_monitor_module_func_cmd);

	
	install_element (VIEW_NODE, &vtysh_ping_cmd);
	install_element (ENABLE_NODE, &vtysh_ping_cmd);
	install_element (CONFIG_NODE, &vtysh_ping_cmd);
	install_element (INTERFACE_NODE, &vtysh_ping_cmd);
	install_element (HIDDENDEBUG_NODE, &vtysh_ping_cmd);

	

	install_element (VIEW_NODE, &vtysh_ping_ip_cmd);
	install_element (VIEW_NODE, &vtysh_traceroute_ip_cmd);
	install_element (VIEW_NODE, &vtysh_traceroute_ip_udp_cmd);


	install_element (ENABLE_NODE, &vtysh_ping_ip_cmd);
	install_element (ENABLE_NODE, &vtysh_traceroute_ip_cmd);
	install_element (ENABLE_NODE, &vtysh_traceroute_ip_udp_cmd);

	install_element (CONFIG_NODE, &vtysh_ping_ip_cmd);
	install_element (CONFIG_NODE, &vtysh_traceroute_ip_udp_cmd);


	install_element (INTERFACE_NODE, &vtysh_ping_ip_cmd);
	install_element (INTERFACE_NODE, &vtysh_traceroute_ip_udp_cmd);

	install_element (HIDDENDEBUG_NODE, &vtysh_ping_ip_cmd);
	install_element (HIDDENDEBUG_NODE, &vtysh_traceroute_ip_udp_cmd);

	install_element(VIEW_NODE,&vtysh_ping_count_cmd);
	install_element(ENABLE_NODE,&vtysh_ping_count_cmd);
	install_element(CONFIG_NODE,&vtysh_ping_count_cmd);
	install_element(INTERFACE_NODE,&vtysh_ping_count_cmd);
	install_element(HIDDENDEBUG_NODE,&vtysh_ping_count_cmd);

	install_element(VIEW_NODE,&vtysh_ping_time_cmd);
	install_element(ENABLE_NODE,&vtysh_ping_time_cmd);
	install_element(CONFIG_NODE,&vtysh_ping_time_cmd);
	install_element(INTERFACE_NODE,&vtysh_ping_time_cmd);
	install_element(HIDDENDEBUG_NODE,&vtysh_ping_time_cmd);

/*	install_element (CONFIG_NODE, &system_reboot_cmd);*/
/*	install_element (ENABLE_NODE, &system_reboot_cmd);	*/
/*	install_element (HIDDENDEBUG_NODE, &system_reboot_cmd);*/

	//install_element (CONFIG_NODE, &system_reboot_all_cmd);
	//install_element (ENABLE_NODE, &system_reboot_all_cmd);	
	install_element (HIDDENDEBUG_NODE, &system_reboot_all_cmd);

	//install_element (CONFIG_NODE, &system_reboot_slot_cmd);
	//install_element (ENABLE_NODE, &system_reboot_slot_cmd);	
	install_element (HIDDENDEBUG_NODE, &system_reboot_slot_cmd);

	install_element (HIDDENDEBUG_NODE, &snapshot_func_cmd);

	install_element (VIEW_NODE, &vtysh_ssh_cmd);
	install_element (ENABLE_NODE, &vtysh_ssh_cmd);
	install_element (CONFIG_NODE, &vtysh_ssh_cmd);
	install_element (INTERFACE_NODE, &vtysh_ssh_cmd);
	install_element (HIDDENDEBUG_NODE, &vtysh_ssh_cmd);

	install_element (VIEW_NODE, &vtysh_telnet_cmd);
	install_element (ENABLE_NODE, &vtysh_telnet_cmd);
	install_element (CONFIG_NODE, &vtysh_telnet_cmd);
	install_element (INTERFACE_NODE, &vtysh_telnet_cmd);
	install_element (HIDDENDEBUG_NODE, &vtysh_telnet_cmd);

	install_element (VIEW_NODE, &vtysh_telnet_port_cmd);
	install_element (ENABLE_NODE, &vtysh_telnet_port_cmd);
	install_element (CONFIG_NODE, &vtysh_telnet_port_cmd);
	install_element (INTERFACE_NODE, &vtysh_telnet_port_cmd);
	install_element (HIDDENDEBUG_NODE, &vtysh_telnet_port_cmd);

	install_element (HIDDENDEBUG_NODE, &terminal_monitor_module_func_cmd); 

	install_element (VIEW_NODE, &vtysh_traceroute_cmd);
	install_element (ENABLE_NODE, &vtysh_traceroute_cmd);
	install_element (CONFIG_NODE, &vtysh_traceroute_cmd);

	install_element (VIEW_NODE, &vtysh_traceroute_udp_cmd);
	install_element (ENABLE_NODE, &vtysh_traceroute_udp_cmd);
	install_element (CONFIG_NODE, &vtysh_traceroute_udp_cmd);
	install_element (INTERFACE_NODE, &vtysh_traceroute_udp_cmd);
	install_element (HIDDENDEBUG_NODE, &vtysh_traceroute_udp_cmd);

	 install_element (VIEW_NODE, &vtysh_ping6_cmd);
	 install_element (ENABLE_NODE, &vtysh_ping6_cmd);
	 install_element (CONFIG_NODE, &vtysh_ping6_cmd);
	 install_element (INTERFACE_NODE, &vtysh_ping6_cmd);
	 install_element (HIDDENDEBUG_NODE, &vtysh_ping6_cmd);

	  install_element (VIEW_NODE, &vtysh_traceroute6_cmd);
	  install_element (ENABLE_NODE, &vtysh_traceroute6_cmd);
	  install_element (CONFIG_NODE, &vtysh_traceroute6_cmd);
	  install_element (INTERFACE_NODE, &vtysh_traceroute6_cmd);
	  install_element (HIDDENDEBUG_NODE, &vtysh_traceroute6_cmd);

	  install_element (HIDDENDEBUG_NODE, &no_terminal_monitor_module_func_cmd); 

//	  install_element (ENABLE_NODE, &show_process_cmd);
//	   install_element (HIDDENDEBUG_NODE, &show_process_cmd);
}


