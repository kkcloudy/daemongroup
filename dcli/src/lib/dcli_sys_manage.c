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
* dcli_sys_manage.c
*
* MODIFY:
*		
*
* CREATOR:
*		shanchx@autelan.com
*
* DESCRIPTION:
*		CLI definition for system manage module.
*
* DATE:
*		02/13/2009
*
*  FILE REVISION NUMBER:
*		
*  		$Revision: 1.46 $	
*  			
*******************************************************************************/
#ifdef __cplusplus
	extern "C"
	{
#endif
#include <zebra.h>
#include <dbus/dbus.h>
#include <unistd.h>
#include <sys/mman.h>
#include<dirent.h>



	
#include <sysdef/npd_sysdef.h>
#include <dbus/npd/npd_dbus_def.h>
	
#include "command.h"
#include "if.h"
#include "memory.h"
	
#include "dcli_sys_manage.h"
#include "dcli_user.h"
#include "dcli_main.h"
#include "dcli_boot.h"
#include "vtysh/vtysh.h"
#include <dbus/sem/sem_dbus_def.h>
#include "bsd/bsdpub.h"
#include "bsd_bsd.h"

#include "ac_manage_def.h"
#include "ac_manage_extend_interface.h"
#include "ac_manage_ntpsyslog_interface.h"
#include "ws_log_conf.h"
/*sfd*/
int dcli_sfd_running(struct vty *vty);

/*add by zhaocg 2012-10-19 for show memory command*/
void *creat_ps_list( )
{	
	ps_list *h=NULL;
	h=(ps_list *)malloc(sizeof(ps_list));
	h->next=NULL;
	return h;
}

int insert_node_in_ps_list(ps_list *h,char *s,int n)
{
	while(h->next!=NULL)
	{
		if(strcmp(h->next->name,s)==0)
			return 1;
		h=h->next;
	}
	ps_list *node=(ps_list *)malloc(sizeof(ps_list));
	if(!node)
		return 2;
	strcpy(node->name,s);
	node->rss=n;
	h->next=node;
	node->next=NULL;
	return 0;
}
void free_ps_list(ps_list *h)
{
	ps_list *p=NULL;
	while(h->next!=NULL)
	{   
		    p=h;
			h=h->next;
			free(p);	
	}
	free(h);
	return;
}

/*end by zhaocg 2012-10-19*/
int sync_file(char* temp,int syn_to_blk)
{
/*
	fprintf(stderr,"sync file is :%s\n",temp);
*/

	
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int op_ret;
	
	query = dbus_message_new_method_call(
								SEM_DBUS_BUSNAME,			\
								SEM_DBUS_OBJPATH,		\
								SEM_DBUS_INTERFACE,	\
								SEM_DBUS_SYN_FILE);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_STRING,&temp,
							DBUS_TYPE_INT32,&syn_to_blk,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		cli_syslog_info("failed get args.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s\n",err.name,err.message);
			dbus_error_free_for_dcli(&err);
			return CMD_WARNING;
		}
		return CMD_WARNING;
	}
	if (dbus_message_get_args ( reply, &err,
								DBUS_TYPE_UINT32,&op_ret,
								DBUS_TYPE_INVALID)) 
	{
		;
/*	
		fprintf(stderr,"dbus message reply is :%d\n",op_ret);
*/
	} 
	else {
		cli_syslog_info("Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	
	dbus_message_unref(reply);
	
	return op_ret;
}
int get_offset_time()
{
	FILE *fp;
	char buffer[100];
	unsigned int timesec;
	fp = fopen("/var/run/offset_time","r");
	if(NULL == fp)
	{
		return 0;
	}
	if(fgets(buffer,100,fp))
	{
		timesec = atoi(buffer);
		
	}
	else
	{
		timesec = 0;

	}
	fclose(fp);
	return timesec;
	
}

int set_offset_time(int offset_time)
{
	FILE *fp;
	char  buffer[100];
	char* tmpbuf;
	int ret =0;

	fp = fopen("/var/run/offset_time","w");
	if(NULL == fp)
	{
		return -1;
	}
	fprintf(fp,"%d",offset_time);

	fclose(fp);
	return ret;
	
}
long get_time_slot()
{
	FILE *fp;
	char buffer[128];
	long timesec;
	
	memset(buffer,0,128);
	
	fp = fopen(TIME_SLOT_FILE,"r");
	
	if(NULL == fp)
	{
		return 0;
	}
	if(fgets(buffer,128,fp))
	{
		timesec = atol(buffer);
		
	}
	else
	{
		timesec = 0;

	}
	fclose(fp);
	return timesec;
	
}



/*DEFUN (system_reboot,
       system_reboot_cmd,
       "reboot",
       "Reboot system\n")
{
	struct timeval tv;
#if 0
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
#endif	
	dcli_send_dbus_signal("user_reboot", "user_reboot");
	tv.tv_sec=3;
	tv.tv_usec=0;
	select(0,NULL,NULL,NULL,&tv);
	
	system ("dcli_reboot.sh");
	return CMD_SUCCESS;
}
*/
DEFUN (system_time,
       system_time_cmd,
       "show time",
       SHOW_STR
       "System time\n")
{
	char buffer[100];
	char* tmpbuf;
	unsigned long timesec;
	unsigned long days,hours,minutes,tmp;
	long t = time(NULL);
	struct timez_st timestr;
	memset(&timestr,0,sizeof(timestr));
	//"date |awk '{print $5}'"
	char zone[20] = {0};
	FILE *pp = NULL;
	pp=popen("date |awk '{print $5}'","r");
	if(pp)
	{
		fgets(zone,sizeof(zone),pp);
		pclose(pp);
	}
	if(access(TIME_SLOT_FILE,0))
	{
		FILE* fp = fopen("/proc/uptime","r");
		if(NULL == fp)
		{
			vty_out(vty,"Can't get system start time\n");
			return CMD_WARNING;
		}
		if(fgets(buffer,100,fp))
		{
			tmpbuf = strchr(buffer,'.');
			if(tmpbuf)
			{
				*tmpbuf = '\0';
				timesec = atoi(buffer)+get_offset_time();
				days = timesec/(24*60*60);
				tmp = timesec %(24*60*60);
				hours = tmp/(60*60);
				tmp = tmp % (60*60);
				minutes = tmp/60;

			}
		}
		fclose(fp);

	}
	else
	{
		timesec = t - get_time_slot() + get_offset_time();
		days = timesec/(24*60*60);
		tmp = timesec %(24*60*60);
		hours = tmp/(60*60);
		tmp = tmp % (60*60);
		minutes = tmp/60;
	}
	vty_out(vty,"===================================================================================================\n");
	vty_out(vty,"%-7s  %-43s  %-43s\n","SLOT ID","UTC",zone);
	vty_out(vty,"=======  ================================  =========================================================\n");				
    int i = 0;	
	for (i = 0 ;i<MAX_SLOT;i++)
	{
		if(dbus_connection_dcli[i]->dcli_dbus_connection) 
		{
			ac_manage_show_time(dbus_connection_dcli[i]->dcli_dbus_connection,&timestr);
			timestr.utcstr[strlen(timestr.utcstr)-1] = '\0';
			vty_out(vty,"%-7d",i);
			vty_out(vty,"%-43s",timestr.utcstr);			
			vty_out(vty,"%-43s",timestr.nowstr);
			vty_out(vty,"\n");
			
		}
	}
	vty_out(vty,"------------------------------------------------------------------\n");
	#if 0 
	struct tm *ptr;
	time_t lt;
	lt =time(NULL);
	ptr=gmtime(&lt);
	vty_out(vty,"second:%d\n",ptr->tm_sec);
	vty_out(vty,"minute:%d\n",ptr->tm_min);
	vty_out(vty,"hour:%d\n",ptr->tm_hour);
	vty_out(vty,"mday:%d\n",ptr->tm_mday);
	vty_out(vty,"month:%d\n",ptr->tm_mon+1);
	vty_out(vty,"year:%d\n",ptr->tm_year+1900);
	vty_out(vty,"------------------------------------------------------------------\n");
	#endif
	vty_out(vty,"System start %d days %d hours %d minutes\n",days,hours,minutes);
	
  	return CMD_SUCCESS;
}

int get_local_board_func(void)
{
	FILE *fd;
	int fun_type;

	fd = fopen("/dbm/local_board/function_type", "r");
	if (fd == NULL)
	{
		return -1;
	}
	fscanf(fd, "%d", &fun_type);
	fclose(fd);


	return fun_type;
}


DEFUN (set_system_time_func,
       set_system_time_func_cmd,
       "set time YYYY/MM/DD HH:MM:SS (utc|cst)",
       SETT_STR
       "Set system time\n"
       "Date such as 2000/01/01\n"
       "Time such as 09:00:00\n")
{
	char *buf;
	char timebuf[128] = {0};
	char timecst[128] = {0};
	char cmdbuf[256] = {0};
	struct tm tm;
	time_t timez;
	int timediff = 0;
	struct tm *ptr;
    int i = 0;
	int ret = 0;
	int status = 0;
	memset(timebuf,0,sizeof(timebuf));
	sprintf(timebuf,"%s %s",argv[0],argv[1]);
	/*strptime("2001-11-12 18:31:01", "%Y-%m-%d %H:%M:%S", &tm);*/
	memset (&tm, 0, sizeof (struct tm));
	buf = strptime(timebuf,"%Y/%m/%d %H:%M:%S",&tm);
	timez = mktime (&tm);
	timediff = timez - 28800;
	ptr=localtime(&timediff);
	snprintf(timecst,sizeof(timecst)-1,"%d/%d/%d %d:%d:%d",ptr->tm_year+1900,ptr->tm_mon+1,ptr->tm_mday,ptr->tm_hour,ptr->tm_min,ptr->tm_sec);
	int p_masterid = 0;	
	int localsid = 0;
	p_masterid = get_product_info(PRODUCT_ACTIVE_MASTER);
	localsid = get_product_info(PRODUCT_LOCAL_SLOTID);
	if(buf )
	{
		if(tm.tm_year<70 || tm.tm_year>=137)
		{
			vty_out(vty,"The year should be >=1970 or <= 2036\n");
			return CMD_WARNING;
		}
		//if(4 != get_local_board_func())
		if(p_masterid == localsid)
		{
			for (i = 0 ;i<MAX_SLOT;i++)
			{
				if(dbus_connection_dcli[i]->dcli_dbus_connection) 
				{
					
					if(0 == strncmp(argv[2],"utc",3))
					{
						ac_manage_set_time(dbus_connection_dcli[i]->dcli_dbus_connection,timebuf);
					}
					else if(0 == strncmp(argv[2],"cst",3))
					{
						ac_manage_set_time(dbus_connection_dcli[i]->dcli_dbus_connection,timecst);
					}
				}
			}
			memset(cmdbuf,0,sizeof(cmdbuf));
			if(0 == strncmp(argv[2],"utc",3))
	    	{
				sprintf(cmdbuf,"sudo hwclock --set --date '%s'",timebuf);
	    	}
			else if(0 == strncmp(argv[2],"cst",3))
			{
				sprintf(cmdbuf,"sudo hwclock --set --date '%s'",timecst);
	    	}
			//ret = system(cmdbuf);
			status = system(cmdbuf);
			ret = WEXITSTATUS(status);
			syslog(LOG_INFO,"#time#  ret is:%d\n",ret);
			syslog(LOG_INFO,"#time#  timecst is:%s\n",timecst);
			syslog(LOG_INFO,"#time#  timebuf is:%s\n",timebuf);
			
		}
		else
		{
			memset(cmdbuf,0,sizeof(cmdbuf));
			if(0 == strncmp(argv[2],"utc",3))
			{
				sprintf(cmdbuf,"sudo date -u -s \"%s\"> /dev/null",timebuf);
			}
			else if(0 == strncmp(argv[2],"cst",3))
			{
				sprintf(cmdbuf,"sudo date -u -s \"%s\" > /dev/null",timecst);
			}
			system(cmdbuf);
		}
	}	
	else
	{
		vty_out(vty,"The date or time format is wrong ,they must be YYYY/MM/DD HH:MM:SS\n");
		return CMD_WARNING;
	}

  return CMD_SUCCESS;
}
struct cmd_node system_mng_node =
{
  SYS_MNG_NODE,
  "	",
  1,
};
extern void dcli_user_manage_write (struct vty *);
DEFUN (set_time_offset_func,
       set_time_offset_func_cmd,
       "set offset_time MINUTES",
       SETT_STR
       "Set system offset time\n"
       "Offset time minutes\n")
{

	int offset_time = atoi(argv[0]);

	offset_time = offset_time*60;

	if(offset_time >= 0 && set_offset_time(offset_time) == 0)
	{
		vty_out(vty,"Set offset time success\n");
	}
	else
	{
		vty_out(vty,"Set offset time %d error\n",offset_time);
	}
	
  return CMD_SUCCESS;
}

DEFUN (debug_time_slot_func,
       debug_time_slot_func_cmd,
       "debug time-slot (enable|disable)",
       DEBUG_STR
       "time-slot\n"
	   "Config time-slot enable\n"
	   "Config time-slot disable\n"
	   )
{
	FILE *fp;
  	long uptime = 0;
	long starttime =0;
	long t;
	char cmd[128];

	if(strncmp(argv[0],"enable",strlen(argv[0])))
	{
		system("del_time_slot.sh");
		return CMD_SUCCESS;
	}
	fp = fopen ("/proc/uptime", "r");  
	if (fp != NULL)
	{
		char buf[BUFSIZ];
		char *b = fgets (buf, BUFSIZ, fp);
		if (b == buf) 
			{
				char *tmpbuf = strchr(buf,'.');
				if(tmpbuf)
				{
					*tmpbuf = '\0';
					uptime = atol(buf); 
					if(uptime < 0)
					{
						fclose (fp);	 
						return CMD_WARNING;
					}

				}
			}
		fclose (fp);	 
	}
	else
	{
		vty_out(vty,"Can't get system start time\n");
		return CMD_WARNING;

	}

	t= time(NULL);
	starttime = t - uptime;
	fp=fopen(TIME_SLOT_FILE,"w");
	if(fp!=NULL)
	{
		fprintf(fp,"%ld",starttime);
		fclose(fp);
	}
	else
	{
		vty_out(vty,"Can't create file %s\n",TIME_SLOT_FILE);
		return CMD_WARNING;
	}

	return sor_exec(vty,"cp",TIME_SLOT_FILE_NAME,20);
}

DEFUN (show_time_slot_func,
       show_time_slot_func_cmd,
       "show time-slot state",
       SHOW_STR
       "Time-slot\n"
       "State\n"
	   )
{
	FILE *fp;
  	double uptime = 0;
	double upsecs;
	double starttime =0;
	long t= time(NULL);
	char cmd[128];

	if(!access(TIME_SLOT_FILE,0))
	{
		vty_out(vty,"debug time-slot is enable\n");
	}
	else
	{
		vty_out(vty,"debug time-slot is disable\n");
	}
  return CMD_SUCCESS;
}


/*DEFUN (snapshot_func,
       snapshot_func_cmd,
       "snapshot",
       "snapshot\n")
{
	int ret ;
	ret = system("sudo debugdownsnapshot.sh");
  return WEXITSTATUS(ret);
}*/

char* get_sys_location(char *location)
{
	FILE* fp=NULL;
	char ptr[SYS_LOCATION_STR_LEN];

	memset(ptr,0,SYS_LOCATION_STR_LEN);
	fp = fopen(SYS_LOCATION_CONFIG_FILE,"r");
	
	if(!fp)
	{
		return NULL;
	}
	while(fgets(ptr,SYS_LOCATION_STR_LEN,fp))	
	{
		if(!strncmp(ptr,SYS_LOCATION_PREFIX,strlen(SYS_LOCATION_PREFIX)))
		{
			sprintf(location,ptr+strlen(SYS_LOCATION_PREFIX));
			fclose(fp);
			return location;

		}
	}
	fclose(fp);
	return NULL;
}

int set_sys_location(char *location)
{
	FILE* fp=NULL;
	char ptr[SYS_LOCATION_STR_LEN];

	if(!location)
		return CMD_WARNING;
	memset(ptr,0,SYS_LOCATION_STR_LEN);
	fp = fopen(SYS_LOCATION_CONFIG_FILE,"w");
	if(!fp)
		return CMD_WARNING;
	
	sprintf(ptr,"%s%s\n",SYS_LOCATION_PREFIX,location);
	fputs(ptr,fp);
	fclose(fp);
	return CMD_SUCCESS;
}


DEFUN (set_system_location_func, 
       set_system_location_func_cmd,
       "sys-location .LINE",
       "System location\n"
       "System location description\n")
{
	char* location = NULL;
	int ret;
	
	location = argv_concat(argv, argc, 0);
	if(!location)
	{
		vty_out(vty,"Can't get sys-location ,please checket");
		return CMD_WARNING;
	}
	ret = set_sys_location(location);
	XFREE(MTYPE_TMP,location);
	return ret;
}
DEFUN (show_system_location_func, 
       show_system_location_func_cmd,
       "show sys-location ",
       SHOW_STR
       "System location description\n")
{
	char location[SYS_LOCATION_STR_LEN];
	char *pstr;

	memset(location,0,SYS_LOCATION_STR_LEN);
	pstr=get_sys_location(location);
	if(pstr)
		vty_out(vty,"The system location is %s\n",pstr);
	else
	{
		vty_out(vty,"Can't get system location. \n");
	}
	return CMD_SUCCESS;

}

char* get_net_element(char *net_element)
{
	FILE* fp=NULL;
	char ptr[NET_ELEMENT_STR_LEN];

	memset(ptr,0,NET_ELEMENT_STR_LEN);
	fp = fopen(NET_ELEMENT_CONFIG_FILE,"r");
	
	if(!fp)
	{
		return NULL;
	}
	while(fgets(ptr,NET_ELEMENT_STR_LEN,fp))	
	{
		if(!strncmp(ptr,NET_ELEMENT_PREFIX,strlen(NET_ELEMENT_PREFIX)))
		{
			sprintf(net_element,ptr+strlen(NET_ELEMENT_PREFIX));
			fclose(fp);
			return net_element;

		}
	}
	fclose(fp);
	return NULL;
}

int set_net_element(char *net_element)
{
	FILE* fp=NULL;
	char ptr[NET_ELEMENT_STR_LEN];

	if(!net_element)
		return CMD_WARNING;
	memset(ptr,0,NET_ELEMENT_STR_LEN);
	fp = fopen(NET_ELEMENT_CONFIG_FILE,"w");
	if(!fp)
		return CMD_WARNING;
	
	sprintf(ptr,"%s%s\n",NET_ELEMENT_PREFIX,net_element);
	fputs(ptr,fp);
	fclose(fp);
	return CMD_SUCCESS;
}


DEFUN (net_element_func, 
       net_element_func_cmd,
       "net-element .LINE",
       "Net element\n"
       "Net element description\n")
{
	char* net_element;
	int ret;
	
	net_element = argv_concat(argv, argc, 0);
	
	if(!net_element)
	{
		vty_out(vty,"Can't get net-element ,please checket");
		return CMD_WARNING;
	}
	ret = set_net_element(net_element);
	XFREE(MTYPE_TMP,net_element);
	return ret;


}
DEFUN (show_net_element_func, 
       show_net_element_func_cmd,
       "show net-element",
       SHOW_STR
       "Net element description\n")
{
	char net_element[NET_ELEMENT_STR_LEN];
	char *pstr;

	memset(net_element,0,NET_ELEMENT_STR_LEN);
	pstr=get_net_element(net_element);
	if(pstr)
		vty_out(vty,"The net element is %s\n",pstr);
	else
	{
		vty_out(vty,"Can't get net element. \n");
	}
	return CMD_SUCCESS;


}






DEFUN(set_smux_start_func,
	set_smux_start_cmd,
	"smux peer (rip|ospf|ip) OID PASSWORD",
	"SNMP MUX protocol settings\n"
	"SNMP MUX peer settings\n"
	"Rip protocol\n"
	"Ospf protocol\n"
	"Ip protocol\n"
	"SMUX peering object ID\n"
	"SMUX peering password\n")
{
	int i=0;
	char* proto=NULL;
	char cmdstr[256]={0};

	if((strlen(argv[1])+strlen(argv[2]))>240)
	{
		vty_out(vty,"Too long OID 77 PASSWORD length\n");
		return CMD_WARNING;
	}
	if(strncmp(argv[0],"ip",strlen(argv[0]))==0)
	{
		proto = "rtmd";
	}
	else
	{
		proto = argv[0];
	}
	
	for(i=0;i<7;i++)
	{
		if(strncmp(vtysh_client[i].name,proto,strlen(proto))==0)
		{
			sprintf(cmdstr,"smux peer %s %s",argv[1],argv[2]);
			vtysh_client_execute(&vtysh_client[i],cmdstr,stdout);
			return CMD_SUCCESS;
		}
	}
	return CMD_WARNING;

	
}

DEFUN(no_smux_start_func,
	no_smux_start_cmd,
	"no smux peer (rip|ospf|ip)",
	NO_STR
	"SNMP MUX protocol settings\n"
	"SNMP MUX peer settings\n"
	"Rip protocol\n"
	"Ospf protocol\n"
	"Ip protocol\n")
{
	int i=0;
	char* proto=NULL;
	char cmdstr[256]={0};

	if(strncmp(argv[0],"ip",strlen(argv[0]))==0)
	{
		proto = "rtmd";
	}
	else
	{
		proto = argv[0];
	}
	
	for(i=0;i<7;i++)
	{
		if(strncmp(vtysh_client[i].name,proto,strlen(proto))==0)
		{
			sprintf(cmdstr,"no smux peer");
			vtysh_client_execute(&vtysh_client[i],cmdstr,stdout);
			return CMD_SUCCESS;
		}
	}
	return CMD_WARNING;

	
}








DEFUN(set_smux_debug_func,
	set_smux_debug_cmd,
	"debug snmp smux (rip|ospf|ip)",
	 NO_STR
	 DEBUG_STR
	"SNMP MUX protocol settings\n"
	"SNMP MUX peer settings\n"	
	"Rip protocol\n"
	"Ospf protocol\n"
	"Ip protocol\n")
{
	int i=0;
	char* proto=NULL;
	char cmdstr[256]={0};

	if(strncmp(argv[0],"ip",strlen(argv[0]))==0)
	{
		proto = "rtmd";
	}
	else
	{
		proto = argv[0];
	}
	
	for(i=0;i<7;i++)
	{
		if(strncmp(vtysh_client[i].name,proto,strlen(proto))==0)
		{
			sprintf(cmdstr,"debug snmp smux");
			vtysh_client_execute(&vtysh_client[i],cmdstr,stdout);
			return CMD_SUCCESS;
		}
	}
	return CMD_WARNING;

	
}

DEFUN(no_set_smux_debug_func,
	no_set_smux_debug_cmd,
	"no debug snmp smux (rip|ospf|ip)",
	 NO_STR
	 DEBUG_STR
	"SNMP MUX protocol settings\n"
	"SNMP MUX peer settings\n"
	"Rip protocol\n"
	"Ospf protocol\n"
	"Ip protocol\n")
{
	int i=0;
	char* proto=NULL;
	char cmdstr[256]={0};

	if(strncmp(argv[0],"ip",strlen(argv[0]))==0)
	{
		proto = "rtmd";
	}
	else
	{
		proto = argv[0];
	}
	
	for(i=0;i<7;i++)
	{
		if(strncmp(vtysh_client[i].name,proto,strlen(proto))==0)
		{
			sprintf(cmdstr,"no debug snmp smux");
			vtysh_client_execute(&vtysh_client[i],cmdstr,stdout);
			return CMD_SUCCESS;
		}
	}
	return CMD_WARNING;

	
}


int dcli_sys_manage_write (struct vty *vty)
{
	int ret;
	char str[NET_ELEMENT_STR_LEN],cmd[256]={0};
	char *pstr;


	
	dcli_user_manage_write(vty);
	dcli_sfd_running(vty);/*show running for sfd command*/
	memset(str,0,NET_ELEMENT_STR_LEN);
	pstr=get_net_element(str);
	if(pstr)
	{
		sprintf(cmd,"net-element %s",pstr);
		vtysh_add_show_string(cmd);
	}
	
	memset(str,0,NET_ELEMENT_STR_LEN);
	memset(cmd,0,256);
	pstr=get_sys_location(str);
	if(pstr)
	{
		sprintf(cmd,"sys-location %s",pstr);
		vtysh_add_show_string(cmd);
	}
	return 0;
}
void dcli_send_dbus_signal(const char* name,const char* str)
{
	DBusMessage *query, *reply;
	DBusMessageIter	 iter;
	DBusError err;
	char exec=1;
	int str_len = strlen(str) + 1;
	unsigned char* config_cmd = malloc(str_len); 

	memset(config_cmd,0,str_len);
	strcpy(config_cmd,str);
	query = dbus_message_new_signal(RTDRV_DBUS_OBJPATH,\
						"aw.trap",name);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		DBUS_TYPE_BYTE,&exec,								
		DBUS_TYPE_STRING,&(config_cmd),
		DBUS_TYPE_INVALID);
	dbus_connection_send (dcli_dbus_connection,query,NULL);
	dbus_connection_flush(dcli_dbus_connection);
	dbus_message_unref(query);	
	free(config_cmd);
	

}

DEFUN(clean_flag_cmd_func,
	clean_flag_cmd,
	"reset media",
	"Reset"
	"Reset storage medium flag\n"
	)
{	
	char cmd[128];
	memset(cmd,0,128);
	sprintf(cmd,"echo 0 > /var/run/sad/stopflag");
	system(cmd);	
	memset(cmd,0,128);
	sprintf(cmd,"echo 0 > /var/run/sad/sadstate");
	system(cmd);
	syslog(LOG_INFO,"[sor.sh] [sad.sh] Make stopflag and sadstate 0 in dcli\n");
	return CMD_SUCCESS;

}
int
kick_out_user(char* user_name, struct vty* vty )
{
	char cmd[128];
	int ret;
	memset(cmd,0,128);
	sprintf(cmd,"sudo kick_out_user.sh %s",user_name);
	syslog(LOG_INFO,"[kick out user] call kick_out_user.sh in dcli\n");
	ret=system(cmd);
	ret=WEXITSTATUS(ret);
	switch (ret)
	{
	case 0:
		return CMD_SUCCESS;
	case 1:
		return CMD_WARNING;
	case 2:
		vty_out(vty,"Can't find user [%s] int system%s",user_name,VTY_NEWLINE);
		return CMD_WARNING;
	default:
		vty_out(vty,"SYS ERROR%s",VTY_NEWLINE);
		return CMD_WARNING;
		}
}
DEFUN(kick_user_cmd_func,
	kick_user_cmd,
	"kick USERNAME",
	"Kick out of the user from system"
	"User name\n"
	)
{	
 return kick_out_user(argv[0],vty);
}
#ifdef DISTRIBUT
DEFUN(config_dbus_session_cmd_func,
	config_dbus_session_cmd,
	"config dbus session (remote|local)",
	"Config system configuration"
	"Config dbus session"
	"Config dbus session"
	"Make dbus session remote"
	"Make dbus session local"
	"User name\n"
	)
{	
 if(strcmp("remote",argv[0]) == 0)
 {
 	if(dcli_dbus_connection_remote != NULL)
 	dcli_dbus_connection=dcli_dbus_connection_remote;
	else
	{
		vty_out(vty,"remote server is not ready\n");
		return CMD_WARNING;
	}
 }else{
 	
 	if(dcli_dbus_connection_local != NULL)
 	dcli_dbus_connection=dcli_dbus_connection_local;
	else
	{
		vty_out(vty,"local connection is not ready\n");
		return CMD_WARNING;
	}
 }
 return CMD_SUCCESS;
 	
}

DEFUN(config_dbus_session_remote_cmd_func,
	config_dbus_session_remote_cmd,
	"config dbus session remote SLOT_NUM",
	"Config system configuration"
	"Config dbus session"
	"Config dbus session"
	"Make dbus session remote"
	"Config dbus session remote"
	)
{	
	int slot_id = -1;
	DBusError dbus_error;
	sscanf(argv[0],"%d",&slot_id);
	if(slot_id < 0 && slot_id >= MAX_SLOT)
	{
		vty_out(vty,"Plese input correct slot number\n");
		return CMD_WARNING;
	}

	if(NULL != dbus_connection_dcli[slot_id] -> dcli_dbus_connection)
	{
		dcli_dbus_connection = dbus_connection_dcli[slot_id] -> dcli_dbus_connection;
	}else{
		vty_out(vty,"the slot has not connected\n");
	}
	return CMD_SUCCESS;

	
}

#endif

DEFUN(show_dbus_session_cmd_func,
	show_dbus_session_cmd,
	"show dbus session",
	SHOW_STR
	"show dbus session"
	"show dbus session"
	)

{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int op_ret;
	char *temp = "we will get your slot number\n";
	
	query = dbus_message_new_method_call(
								DEMO_DBUS_BUSNAME,			\
								DEMO_DBUS_OBJPATH,		\
								DEMO_DBUS_INTERFACE,	\
								DEMO_DBUS_METHOD_GET_SLOT_NUM);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_STRING,&temp,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		fprintf(stderr,"failed get args.\n");
		if (dbus_error_is_set(&err)) {
			fprintf(stderr,"%s raised: %s\n",err.name,err.message);
			dbus_error_free_for_dcli(&err);
			return CMD_WARNING;
		}
		return CMD_WARNING;
	}
	if (dbus_message_get_args ( reply, &err,
								DBUS_TYPE_UINT32,&op_ret,
								DBUS_TYPE_INVALID)) 
	{
		vty_out(vty,"got dbus session slot num is :%d\n",op_ret);
	} 
	else {
		fprintf(stderr,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			fprintf(stderr,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}


/**----------gjd: pfm show runnning config-------------***/

int
pfm_setup_show_running_config(char *tmp_str, int opt)
{
	FILE *fp = NULL;
	int ret, status;
	char setup_str[256];
	char cmd_str[256];
	char cmd_mv[128];
	int len = 0;
	
	memset(setup_str,0 ,256);
	memset(cmd_str,0 ,256);
	memset(cmd_mv,0,128);
	
	
	strcpy(setup_str,"service ");/**add cmd head**/
	strcat(setup_str,tmp_str);/**OK ,unit a whole cmd string save in setup_str**/

	if(opt == 0)/**enable : go to write setup in file**/
	{
		fp = fopen(PFM_SETUP_FILE,"a");
		if(fp==NULL)
		{
	 	//	fprintf(stderr,"Open file err : %s",safe_strerror(errno));
			return 1;
		}

		strcat(setup_str," enable\n");/**when write add '\n' **/
		fputs(setup_str,fp);

		fclose(fp);
	}
	else /**disable : detele the enable(or say rm the enable string)**/
	{
		strcat(setup_str," enable");/**when use sed delete , the '\n' make shell comand 'sed' failed ,so not add '\n' **/
	//	fprintf(stderr,"----------setup_str string :%s----------\n",setup_str);
		
		sprintf(cmd_str,"sed '/%s/d' %s >%s",setup_str,PFM_SETUP_FILE,PFM_SETUP_TMP_FILE);
	//    fprintf(stderr,"cmd_bak:%s\n",cmd_str);
		system(cmd_str);
	//	ret = WEXITSTATUS(status);

		/**mv file : pfm_setup_tmp ==> pfm_setup**/
		sprintf(cmd_mv,"mv %s %s",PFM_SETUP_TMP_FILE,PFM_SETUP_FILE);
	//	fprintf(stderr, "cmd_mv string: %s...\n",cmd_mv);
		system(cmd_mv);
				
	}
	
	return 0;
}

/**gjd : check pfm exist setup **/
int
check_pfm_setup_exist(char *search_str, int opt)
{
	FILE *fp = NULL;
	int ret;
	char setup_str[256];
	char cmd_tmp[128];
	char cmd_str[256];
	int ret2;
	
	memset(setup_str,0 ,256);
	memset(cmd_tmp,0,128);
	memset(cmd_str,0 ,256);
	
	strcpy(setup_str,"service ");/**add cmd head**/
	strcat(setup_str,search_str);/**OK ,unit a whole cmd string save in setup_str**/
	
	ret = access(PFM_SETUP_FILE, 0);/**to check the file is exist or not ?**/
   	if(ret < 0)    /**if not, creat it**/
  	{
  	  sprintf(cmd_tmp,"touch %s",PFM_SETUP_FILE);
	  system(cmd_tmp);
 	}
	
	/**check if exist the same cmd**/
	sprintf(cmd_str,"cat %s | grep \"%s\" > /dev/null 2> /dev/null",PFM_SETUP_FILE ,search_str);
	ret = system(cmd_str);
	ret2 = WEXITSTATUS(ret);
	if(ret2 == 0)/**if find**/
	{	
		if(opt == 0)/**have enable**/
		{
//			fprintf(stderr,"You have this setup already !\n");
			return 1;
		}
	}
	else/**if not find**/
	{
		if(opt == 1)/**Because every pfm setup default set is disable , so don't need to set disable**/
		{
//			fprintf(stderr,"This setup is disable already !\n");
			return 1;
		}	
	}
		
	return 0;
	
}
/**-------------------2011-05-06 : pm 2:20-----------------------**/



/**-----------gjd : add dcli communicate with pfm app by dbus -----------------**/

/**add get ifindex by ioctl through ifname**/
static int ifname2ifindex_by_ioctl(const char *dev)
{
	struct ifreq ifr;
	int fd;
	int err;

	memset(&ifr,0,sizeof(ifr));
	strncpy(ifr.ifr_name, dev, sizeof(ifr.ifr_name));
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	err = ioctl(fd, SIOCGIFINDEX, &ifr);
	if (err) 
	{
		close(fd);
		return 0;
	}
	close(fd);
	return ifr.ifr_ifindex;
}

/**gjd : dcli use dbus communicate with pfm**/
int 
dcli_communicate_pfm_by_dbus(int opt, 
							int opt_para, 
							unsigned short protocol, 
							char* ifname, 
							unsigned int src_port,
							unsigned int dest_port, 
							int slot,
							char* src_ipaddr,
							char* dest_ipaddr,
							unsigned int send_to)

{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int op_ret;
	/*
	long src_ipaddr1,src_ipaddr2,dest_ipaddr1,dest_ipaddr2;
	memcpy(&src_ipaddr1,&src_ipaddr,sizeof(src_ipaddr1));
	memcpy(&src_ipaddr2,(&src_ipaddr) + sizeof(src_ipaddr1),sizeof(src_ipaddr2));
	memcpy(&dest_ipaddr1,(&dest_ipaddr),sizeof(dest_ipaddr1));
	memcpy(&dest_ipaddr2,(&dest_ipaddr) + sizeof(dest_ipaddr1),sizeof(dest_ipaddr2));
	*/
#if 0
	fprintf(stderr,"DCLI send data to PFM are :\n");
			fprintf(stderr," opt is %d ....\n",opt);
			fprintf(stderr," protocol is %u ....\n",protocol);
			fprintf(stderr," ifindex is %s ....\n",ifname);
			fprintf(stderr," port is %d ....\n",dest_port);
			fprintf(stderr," send to is %d ....\n",send_to);
			fprintf(stderr," slot is %d ....\n",slot);
			fprintf(stderr," ipaddr is %u ....\n",dest_ipaddr);
	#endif
	if(slot == send_to)
	{
		return -1;
	}
	query = dbus_message_new_method_call(
								PFM_DBUS_BUSNAME,			\
								PFM_DBUS_OBJPATH,		\
								PFM_DBUS_INTERFACE,	\
								PFM_DBUS_METHOD_PFM_TABLE);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_INT32, &opt,
							DBUS_TYPE_INT32, &opt_para,
							DBUS_TYPE_UINT16, &protocol,
							DBUS_TYPE_STRING, &ifname,
							DBUS_TYPE_UINT32, &src_port,
							DBUS_TYPE_UINT32, &dest_port,
							DBUS_TYPE_STRING,  &src_ipaddr,
							DBUS_TYPE_STRING,  &dest_ipaddr,	
							DBUS_TYPE_INT32,  &slot,
							DBUS_TYPE_INVALID);

	if(-1 == send_to)
	{
		int i;
		for(i = 0;i < MAX_SLOT ; i++)
		{
			if(NULL != (dbus_connection_dcli[i] -> dcli_dbus_connection))
			{
				
				reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[i] -> dcli_dbus_connection,query,-1, &err);
				if (NULL == reply) {
					fprintf(stderr,"[%d]failed get args.\n",i);
					if (dbus_error_is_set(&err)) {
						fprintf(stderr,"%s raised: %s\n",err.name,err.message);
						dbus_message_unref(query);
						dbus_error_free_for_dcli(&err);
						return -1;
					}
				}
				
				if (dbus_message_get_args ( reply, &err,
											DBUS_TYPE_UINT32,&op_ret,
											DBUS_TYPE_INVALID)) 
				{
					vty_out(vty,"DCLI recv [%d] reply is :%s\n",i,op_ret == 0?"OK":"ERROR");
				} 
				else {
					fprintf(stderr,"Failed get args.\n");
					if (dbus_error_is_set(&err)) {
						fprintf(stderr,"%s raised: %s",err.name,err.message);
						dbus_error_free_for_dcli(&err);
					}
				}
				
//				dbus_message_unref(reply);
			}

		}
		dbus_message_unref(query);
		
		dbus_message_unref(reply);

		dbus_error_free_for_dcli(&err);
		
	}else{
	
		if(NULL != dbus_connection_dcli[send_to] -> dcli_dbus_connection)
		{
			
			reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[send_to] -> dcli_dbus_connection,query,-1, &err);
			if (NULL == reply){
				fprintf(stderr,"failed get args. \n");
				if (dbus_error_is_set(&err)){
					fprintf(stderr,"%s raised: %s\n",err.name,err.message);
					dbus_message_unref(query);
					dbus_error_free_for_dcli(&err);
					return -1;
				}
			}
			
			dbus_message_unref(query);
		}else{
			fprintf(stderr,"connection of board %d is not exist\n",send_to);
			return -1;
		}
	
		
			
//	#if 0
	if (dbus_message_get_args ( reply, &err,
								DBUS_TYPE_UINT32,&op_ret,
								DBUS_TYPE_INVALID)) 
	{
		vty_out(vty,"DCLI recv reply is :%s\n",op_ret == 0?"OK":"ERROR");
	} 
	else {
		fprintf(stderr,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			fprintf(stderr,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	
	dbus_message_unref(reply);
//	#endif
	}
	return 0;
}

/**gjd : add command for set pfm**/
#if 1
int get_slot_num_dcli(char *ifname)
{
	int slotnum;
	int num1,num2,num3,num4;
	int i , count = 0;
	char tmp[128] = {0};

	memcpy(tmp, ifname, strlen(ifname));
	
	if(strncmp(ifname,"eth",3)==0)/*eth : cpu*/
	 {
	 	sscanf(ifname,"eth%d-%d",&slotnum,&num1);
		return slotnum;
		}
	#if 1
	if(strncmp(ifname,"ve",2)==0)/*ve*/
	{
		/* caojia add support for new ve-interface name */
		if (tmp[4] == 'f' || tmp[4] == 's') {
			sscanf(ifname,"ve%02df%d.%d", &slotnum, &num1, &num2);
			//vty_out(vty, "slotnum %d, num1 %d, num2 %d\n", slotnum, num1, num2);
		}
		else {
			sscanf(ifname,"ve%d.%d", &slotnum, &num1);
		}
		
		return slotnum;
	}
	#endif
	if(strncmp(ifname,"r",1) == 0)/*radio*/
	 {
	 	for(i = 0; i < strlen(ifname); i++)
	 	 {
	 	 	if(tmp[i] == '-')/*use '-' to make sure this radio is local board or remote board ?*/
				count++;
	 	  }
		if(count == 2)/*local board*/
		  {
			slotnum = HostSlotId;
			return slotnum;
			}
		if(count == 3)/*remote board*/
		  {
		  	sscanf(ifname,"r%d-%d-%d-%d.%d",&slotnum,&num1,&num2,&num3,&num4);
		  	return slotnum;
		  	}
		}
	
	if(strncmp(ifname,"wlan",4) == 0)/*wlan*/
	 {
	 	for(i = 0; i < strlen(ifname); i++)
	 	 {
	 	 	if(tmp[i] == '-')/*use '-' to make sure this radio is local board or remote board ?*/
				count++;
	 	  }
		if(count == 1)/*local board*/
		  {
			slotnum = HostSlotId;
			return slotnum;
			}
		if(count == 2)/*remote board*/
		/*  {
		  	sscanf(ifname,"wlan%d-%d-%d",&slotnum,&num1,&num2);
		  	return slotnum;
		  	}
		  */
		{
		  	if(strncmp(ifname,"wlanl",5)==0)/*local hansi wlanlx-x-x*/
		  	{
		  	  sscanf(ifname,"wlanl%d-%d-%d",&slotnum,&num1,&num2);
		  	  return slotnum;
		  	}
			else
			{
		  	  sscanf(ifname,"wlan%d-%d-%d",&slotnum,&num1,&num2);/*remove hansi wlanx-x-x*/
		  	  return slotnum;
			}
		  }
	 	}
	
	if(strncmp(ifname,"ebr",3) == 0 )/*ebr*/
	 {
	 	for(i = 0; i < strlen(ifname); i++)
	 	 {
	 	 	if(tmp[i] == '-')/*use '-' to make sure this radio is local board or remote board ?*/
				count++;
	 	  }
		if(count == 1)/*local board*/
		  {
			slotnum = HostSlotId;
			return slotnum;
			}
		if(count == 2)/*remote board*/
		  /*{
		  	sscanf(ifname,"ebr%d-%d-%d",&slotnum,&num1,&num2);
		  	return slotnum;
		  	}*/
			{
			  	if(strncmp(ifname,"ebrl",4)==0)/*local hansi ebrlx-x-x*/
			  	{
			  	  sscanf(ifname,"ebrl%d-%d-%d",&slotnum,&num1,&num2);
			  	  return slotnum;
			  	}
				else
				{
			  	  sscanf(ifname,"ebr%d-%d-%d",&slotnum,&num1,&num2);/*remove hansi ebrx-x-x*/
			  	  return slotnum;
				}
			  }
	 	}
	if(strncmp(ifname,"obc0",4) == 0)
		{
			slotnum = HostSlotId;
			return slotnum;
		}
	return 0;/*err*/
	

//	return slotnum;

}

#endif
DEFUN(config_pfm_icmp_func,
	config_icmp_cmd,
	"service icmp IFNAME (enable|disable)",
	"service for Distribute System\n"
	"service for transport icmp packet to local board\n"
	"used eth*-*\n"
	"make service enable\n "
	"make service disable\n "
	)
{
	int slot_num,temp_ret,opt;
	char temp_str[128];
	memset(temp_str,0,128);
	
	slot_num = get_slot_num_dcli(argv[0]);
	if(slot_num == 0)
	{
		vty_out(vty,"error interface\n");
		return CMD_WARNING;
	}
	sprintf(temp_str,"icmp %s",argv[0]);
	if(strncmp("enable",argv[1],strlen(argv[1]))== 0)
		opt = 0;
	else
		opt = 1;

	/*vty_out(vty,"opt is :%d\n slot_num is :%d\n argv1 is :%s..strlen is %d\n",opt,slot_num,argv[1],strlen(argv[1]));*/

	temp_ret = check_pfm_setup_exist(temp_str, opt);/**check pfm exist setup**/
	
	if(temp_ret != 0)
	{
		vty_out(vty,"The configuration has been.\n");
		return CMD_WARNING;
	}
#if 0	
	ret = dcli_communicate_pfm_by_dbus(4, 0, 0, (char*)argv[0], 0, 0, 0, "all", "all",send_to_slot);/**use dbus send message to pfm**/
	if(ret != 0)
	{
		vty_out(vty,"dbus send message error_enable\n");
		return CMD_WARNING;
	}
#endif
	temp_ret = dcli_communicate_pfm_by_dbus(opt, 0, 1, (char*)argv[0],0, 0, HostSlotId,"all", "all",slot_num);/**use dbus send message to pfm**/
	if(temp_ret != 0)
	{
		vty_out(vty,"dbus send message error_table\n");
		return CMD_WARNING;
	}
	
	temp_ret = pfm_setup_show_running_config(temp_str, opt);/**when pfm set success , write the setup to a file  that used for show running.**/
	if(temp_ret != 0)
		return CMD_WARNING;

//	vtysh_pfm_config_write(vty);

	return CMD_SUCCESS;
	
	
	
}

DEFUN(config_pfm_cmd_func,
	config_pfm_cmd,
	"service (ssh|telnet) IFNAME (IP|all) (enable|disable)",
	"service for Distribute System\n"
	"support ssh forwrd for Distribute System\n"
	"support telent forwrd for Distribute System\n"
	"used eth*-*\n"
	"a ip address under a interface \n"
	"all ip address under a interface \n"	
	"Distrbute Product slotnum\n"
	"make service enable\n "
	"make service enable\n "
	)

{
	
	
	int ifindex ;
	unsigned int src_port = 0, dest_port;
	unsigned short protocol;/**tcp : 6, udp :17**/
	int slot ;
	char* ipaddr = NULL;
	int opt = 1 ;
	/*int forward_opt ;*/
	int send_to_slot = 0;
	int send_to_port = 0;
	int ret;
	char *if_name = NULL;
	char serv_name[32] = {0};
	char cmd_str[128] = {0};
	if((is_distributed == 1) && (is_active_master == 0))
	{
		vty_out(vty,"only active master can enable/disable ssh/telnet\n");
		return CMD_WARNING;
	}

#if 0
	cmd_str = argv_concat(argv, 3,0);/**unit the cmd (only argv ,not include head "service") to a string ,used for save show running config**/
										/**3 = argc -1, enable or disable not include**/
#endif
	if(argc == 4)
	{
	  if(strncmp(argv[0],"ssh",strlen(argv[0])) == 0)	
	  {
		protocol = 6;
		dest_port = 22;
		
		sprintf(serv_name,"ssh");
			if(strncmp(argv[3],"enable",strlen(argv[3])) == 0) /*enable ssh*/
		{
			char cmd[128];
			int ret;
			memset(cmd,0,128);
			sprintf(cmd,"sudo /etc/init.d/ssh restart > /dev/null 2> /dev/null");
			ret = system(cmd);
			if(WEXITSTATUS(ret)== 0)
			{
				vty_out(vty,"make ssh enable\n");
			}
			else
			{
				vty_out(vty,"SSH,some thing is Wrong%s",VTY_NEWLINE);
				return CMD_WARNING;
			}
		}

			
		}
		else if(strncmp(argv[0],"telnet",strlen(argv[0])) == 0)
		{
			int ret;
			protocol = 6;
			dest_port = 23;
			sprintf(serv_name,"telnet");
			if(strncmp(argv[3],"enable",strlen(argv[3])) == 0) /*enable telnet*/
			{
				int fp;
				struct stat sb;
				char *temp_data;
				char *data;
				char port_str[10];
				u_int32_t dest_port;
				
				memset(port_str,0,10);
				sprintf(port_str,"%d",23);
				dest_port=23;
				
				if(1)
				{
				
					int i;
					for(i=0;i<strlen(port_str);i++)
					{
						if((*(port_str+i))>'9' || (*(port_str+i))<'0')
						{
							vty_out(vty,"Please press right port number%s",VTY_NEWLINE);
							return CMD_WARNING;
						}
					}
					if(dest_port>65535 || dest_port<0)
					{
						vty_out(vty,"Port number is incorrect%s",VTY_NEWLINE);
						munmap(data,sb.st_size);
						close(fp);
						return CMD_WARNING;
					}
				}
				
				//check out port NO.in the /etc/ssh/sshd_config
				
				fp=open(SSHD_CONFIG_PATCH,O_RDWR);
				if(-1 == fp)
				{
					vty_out(vty,SYS_ERROR);
					return CMD_WARNING;
				}
				fstat(fp,&sb);
				data=mmap(NULL,sb.st_size+10,PROT_READ,MAP_SHARED,fp,0);
				if(MAP_FAILED==data)
				{
					vty_out(vty,SYS_ERROR);
					close(fp);
					return CMD_WARNING;
				}		
				temp_data=strstr(data,"Port ");
				if(dest_port==atoi(temp_data+strlen("Port ")))
				{
					
					munmap(data,sb.st_size);
					close(fp);
					vty_out(vty,"Port has been occupied by ssh%s",VTY_NEWLINE);
					return CMD_WARNING;
				}
				munmap(data,sb.st_size);
				close(fp);
				
				fp=open(SERVICES_PATH,O_RDWR);	
				if(-1==fp)
				{
					vty_out(vty,SYS_ERROR);
					return CMD_WARNING;
				}
				fstat(fp,&sb);
				data=mmap(NULL,sb.st_size,PROT_READ|PROT_WRITE,MAP_SHARED,fp,0);
				if(MAP_FAILED==data)
				{
					vty_out(vty,SYS_ERROR);
					close(fp);
					return CMD_WARNING;
				}
					
				if(1) 
				{
					//Check the correctness of port
			
					//find port for argv[0]
					temp_data=strstr(data,port_str);
					if(temp_data && (*(temp_data+(strlen(port_str))))=='/' && (*(temp_data-1))=='\t')//the port was used
					{
						if(strncmp(temp_data-(strlen("telnet")+2),"telnet",strlen("telnet")))
						{
							vty_out(vty,"Port is already in use%s",VTY_NEWLINE);
							munmap(data,sb.st_size);
							close(fp);
							return CMD_WARNING;
						}
						
					}
					
					//change the telnet port
					for(temp_data=data;temp_data<=(data+sb.st_size);temp_data=strstr(temp_data,"telnet"))
					{
						if(temp_data==NULL)
						{
							break;
						}
						if((*(temp_data-1))==10 && (*(temp_data+6))==9) //is real telnet
						{
							char cmd[64];
							char* temp;
							temp=strchr(temp_data,'\n');
							
							memset(cmd,0,64);
							sprintf(cmd,"telnet\t\t%s/tcp",port_str);
						//	sprintf(temp_data+strlen(cmd),"%s",temp);
							memmove(temp_data+strlen(cmd),temp,strlen(temp));//change the port
							memcpy(temp_data,cmd,strlen(cmd));
					//		free(temp_mem);
							temp_data++;
							continue;
						}
					temp_data++;
					}
					munmap(data,sb.st_size);
					close(fp);
				}
		#if 0
				else
				{
					
					for(temp_data=data;temp_data<=(data+sb.st_size);temp_data=strstr(temp_data,"telnet"))
					{
						if(NULL==temp_data)
						{
							break;
						}
						if((*(temp_data-1))==10 && (*(temp_data+6))==9) //is real telnet
						{
							char cmd[64];
							char* temp;
							temp=strchr(temp_data,10);
							memset(cmd,0,64);
							sprintf(cmd,"telnet\t\t%d/tcp",23);
							memmove(temp_data+strlen(cmd),temp,strlen(temp));//change the port
							memcpy(temp_data,cmd,strlen(cmd));
							
							temp_data++;
							continue;
						}
					temp_data++;
					}
					munmap(data,sb.st_size);
					close(fp);
				}	
		#endif
					//find inetd.conf file for telnet
					fp=open(INETD_PATH,O_RDWR);
			
					if(-1==fp)
						{
						vty_out(vty,SYS_ERROR);
						return CMD_WARNING;
						}
					fstat(fp,&sb);
					data=mmap(NULL,sb.st_size,PROT_READ|PROT_WRITE,MAP_SHARED,fp,0);
					if(MAP_FAILED==data)
						{
						vty_out(vty,SYS_ERROR);
						close(fp);
						return CMD_WARNING;
						}
					for(temp_data=data;temp_data<=(data+sb.st_size);temp_data=strstr(temp_data,"#elnet"))
					{
						if(!temp_data)
						{
							break;
						}
						*temp_data='t';
					}
					munmap(data,sb.st_size);
					close(fp);
					system("pkill inetd;sudo inetd >/dev/null 2>/dev/null");
					//free(temp_mem);
				
					vty_out(vty,"make telnet enable\n");
				
			}
			
		}
		else
		{
	   	    vty_out(vty,"Dont' support: %s\n",argv[0]);
			return CMD_WARNING;	
		}
		/*
		sscanf(argv[1],"eth%d-%d",&send_to_slot,&send_to_port);

		ifindex = ifname2ifindex_by_ioctl(argv[1]);
		
	   	 if(ifindex == 0)
	     	{
		   	vty_out(vty,"interface %s is not exist!\n",argv[1]);
		  	 return CMD_WARNING;
	    	 }
		*/
		send_to_slot = get_slot_num_dcli(argv[1]);
		if(send_to_slot == HostSlotId || send_to_slot == -1)
		{
			vty_out(vty,"local slot ,Has enabled\n");
			return CMD_WARNING;
		}
		if_name = argv[1];
//		fprintf(stderr,"get slot num is %d\n",send_to_slot);
		if(strncmp(argv[2],"all",strlen(argv[2])) == 0)/**for all ip forward**/
		 {
//		 	forward_opt = -2;
			ipaddr = "all";
		 }
		else
		 {
			 if(CMD_SUCCESS != parse_ip_check((char *)argv[2]))/**check the ip address is correct **/
			{
		      vty_out(vty,"%% Bad parameter: %s\n",(char *)argv[2]);
		      return CMD_WARNING;
		  	}
			 
		 	ipaddr = (argv[2]);
//		    forward_opt = 0;/**noramal**/
		 }
#if 0		
		slot = atoi(argv[3]);
		if(slot < 0 || slot > 15)/**In future slot will change , lack a product type (7605 or 8610)**/
		{
			vty_out(vty,"Input slot err ! Select it between: 7605<0--2>, 8610<0--15>");
			return CMD_WARNING;
		}
		
#endif			
		if(strncmp(argv[3],"enable",strlen(argv[3])) == 0)
		{
			opt = 0;
		}
		else
		{
			opt = 1;
		}
		
	}	

	slot = HostSlotId;
	sprintf(cmd_str,"%s %s %s",serv_name,if_name,ipaddr);
	ret = check_pfm_setup_exist(cmd_str, opt);/**check pfm exist setup**/

	if(ret != 0)
	{
		vty_out(vty,"The configuration has been.\n");
		return CMD_WARNING;
	}
//enable pfm

	ret = dcli_communicate_pfm_by_dbus(4, 0, 0, (char*)argv[1], 0, 0, 0, "all", "all",send_to_slot);/**use dbus send message to pfm**/
	if(ret != 0)
	{
		vty_out(vty,"dbus send message error_enable\n");
		return CMD_WARNING;
	}
	
	ret = dcli_communicate_pfm_by_dbus(opt, 0, protocol, (char*)argv[1],0, dest_port, slot,"all", "all",send_to_slot);/**use dbus send message to pfm**/
	if(ret != 0)
	{
		vty_out(vty,"dbus send message error_table\n");
		return CMD_WARNING;
	}
	
	ret = pfm_setup_show_running_config(cmd_str, opt);/**when pfm set success , write the setup to a file  that used for show running.**/
	if(ret != 0)
	{
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
	
}
/**-------------------2011-05-04 : pm 5:20-----------------------**/



/**gjd : add command for set pfm**/

DEFUN(config_pfm_cmd_func2,
	config_pfm_cmd_general,
	"service pfm PROTOCOL SRC_PORT DEST_PORT IFNAME (A.B.C.D/M | all)  (E.F.G.H/N | all)  SLOT (enable|disable) [ipv6]",
	"service for Distribute System\n"
	"protocol flow manage\n"
	"used  protocol for pfm\n"
	"source port:select between <1--65535>\n"
	"dest port:select between <1--65535>\n"	
	"used eth*-*\n"
	"source ip prefix under a interface <A.B.C.D\M>\n"
	"destination ip prefix under a interface <A.B.C.D\M>\n"
	"Distrbute Product slotnum\n"
	"make service enable\n "
	"make service enable\n "
	"ipv6 address"
	)

{
	int ifindex = 0;
	unsigned int src_port , dest_port;
	unsigned short protocol;/**tcp : 6, udp :17**/
	int slot ;
	long src_ipaddr, dest_ipaddr;
	unsigned int src_ipmask, dest_ipmask;
	int opt ;
	int forward_opt ;
	int command_send_to;
	int send_to_port;
	
	int ret;
									/**3 = argc -1, enable or disable not include**/	


	protocol = atoi(argv[0]);
	if (protocol > 255 || protocol < 0) {
		vty_out(vty, "intput protocl err <0--255>!\n");
		return CMD_WARNING;
	}

	src_port = atoi(argv[1]);
	if(src_port < 0 || src_port > 65535)
	 {
 	   vty_out(vty,"Input src_port err.<0--65535> !\n");
       return CMD_WARNING;
     }
	
	dest_port = atoi(argv[2]);
	if(dest_port < 0 || dest_port > 65535)
	 {
 	   vty_out(vty,"Input dest_port err.<0--65535> !\n");
       return CMD_WARNING;
     }
	
	command_send_to = get_slot_num_dcli(argv[3]);
/*
	    if(ifindex == 0)
	     {
	   vty_out(vty,"Get interface %s ifindex failed.\n",argv[3]);
		   return CMD_WARNING;
	     }
*/
    #if 0
 	if(CMD_SUCCESS != ip_address_format2ulong((char **)&argv[4],&src_ipaddr,&src_ipmask))/**check the ip address is correct **/
	{
      vty_out(vty,"%% Bad parameter: %s\n",(char *)argv[4]);
      return CMD_WARNING;
  	}
	if(CMD_SUCCESS != ip_address_format2ulong((char **)&argv[5],&dest_ipaddr,&dest_ipmask))/**check the ip address is correct **/
	{
      vty_out(vty,"%% Bad parameter: %s\n",(char *)argv[5]);
      return CMD_WARNING;
  	}
    #endif
	slot = atoi(argv[6]);
		if(slot < 0 || slot > 15)/**In future slot will change , lack a product type (7605 or 8610)**/
		{
			vty_out(vty,"Input slot err ! Select it between: 7605<0--2>, 8610<0--15>");
			return CMD_WARNING;
		}

	if(strncmp(argv[7],"enable",(strlen(argv[7]))) == 0)
	{
		if (argc == 8){
			opt = 0;
		}else if (argc == 9){
			opt = 10;
		}
	} 
	else 
	{
		if (argc == 8){
			opt = 1;
		}else if (argc == 9){
			opt = 11;
		}
	}

//	slot = 0;
#if 0
	ret = check_pfm_setup_exist(cmd_str, opt);/**check pfm exist setup**/

	if(ret != 0)
		return CMD_WARNING;
     #endif
	ret = dcli_communicate_pfm_by_dbus(opt, 0, protocol, (char*)argv[3], src_port,dest_port, slot,
												(char*)argv[4], (char*)argv[5],command_send_to);/*use dbus send message to pfm*/
	if(ret != 0)
		return CMD_WARNING;
#if 0
	ret = pfm_setup_show_running_config(cmd_str, opt);/**when pfm set success , write the setup to a file  that used for show running.**/
	if(ret != 0)
		return CMD_WARNING;
#endif
//	vtysh_pfm_config_write(vty);

	return CMD_SUCCESS;
	
}



DEFUN (service_pfm_log_func, 
       service_pfm_log_func_cmd,
       "service pfm log (enable|disable)",
       "config system service\n"
       "config pfm service\n"
       "config log of pfm\n"
       "make pfm service enable\n"
       "make pfm service disable\n")
{
	
	char *cmd_str = "pfm log";
	int opt_para;
	int ret;
	
	if(strcmp(argv[0],"enable") == 0)
		opt_para = 1;
	else
		opt_para = 0;
	
//	ret = check_pfm_setup_exist(cmd_str, !(opt_para));

//	if(ret != 0)
//		return CMD_WARNING;
	
	ret = dcli_communicate_pfm_by_dbus(2, opt_para, 0, "all", 0, 0, 0, "all", "all" ,-1);/**use dbus send message to pfm**/
	if(ret != 0)
		return CMD_WARNING;
	
//	ret = pfm_setup_show_running_config(cmd_str, !(opt_para));
//	if(ret != 0)
//		return CMD_WARNING;
		return CMD_SUCCESS;

}


DEFUN (service_pfm_func, 
       service_pfm_func_cmd,
       "service pfm (enable|disable)",
       "config system service\n"
       "config pfm service\n"
       "make pfm service enable\n"
       "make pfm service disable\n")
{
	
	char *cmd_str = "pfm";
	int opt;
	int ret;
	
	if(strcmp(argv[0],"enable") == 0)
		opt = 0;
	else
		opt = 1;
#if 0
	ret = check_pfm_setup_exist(cmd_str, opt);

	if(ret != 0)
		return CMD_WARNING;
#endif
	
	ret = dcli_communicate_pfm_by_dbus((opt?3:4), 0, 0, "all", 0, 0, 0, "all","all" ,-1);/**use dbus send message to pfm**/
	if(ret != 0)
		return CMD_WARNING;
#if 0	
	ret = pfm_setup_show_running_config(cmd_str,opt);/**when pfm set success , write the setup to a file  that used for show running.**/
	if(ret != 0)
		return CMD_WARNING;
#endif	
	return CMD_SUCCESS;

}

DEFUN (show_pfm_table_func, 
       show_pfm_table_func_cmd,
       "show pfm table",
		"show pfm table"
		"show pfm table"
		"show pfm table")
{
	
	int ret=0;

	ret = dcli_communicate_pfm_by_dbus(5, 0, 0, "all", 0, 0, 0, "all","all" ,-1);/**use dbus send message to pfm**/
	if(ret != 0)
		return CMD_WARNING;
#if 0	
	ret = pfm_setup_show_running_config(cmd_str,opt);/**when pfm set success , write the setup to a file  that used for show running.**/
	if(ret != 0)
		return CMD_WARNING;
#endif	
	return CMD_SUCCESS;

}


DEFUN (system_reset_all,
       system_reset_all_cmd,
       "reset (fast|normal) (all|SLOT_ID)",
       "hardware reset all\n"
       "product hardware reset without saving system info\n"
       "product hardware reset with saving system info\n"
       )
{
	unsigned isFast;
	FILE *fd;
	int slot_id;
	int slot_count = 10;

	/* after mcb active standby switched, the is_active_master need get value again , caojia added*/
	fd = fopen("/dbm/local_board/is_active_master", "r");
	if (fd == NULL)
	{
		fprintf(stderr,"Get production information [2] error\n");
		return -1;
	}
	fscanf(fd, "%d", &is_active_master);
	fclose(fd);

	/*fetch the param : normal ? fast*/
	if(0 == strncmp(argv[0],"fast",strlen(argv[0]))) {
		isFast = 0;
	}
	else if (0 == strncmp(argv[0],"normal",strlen(argv[0]))) {
		isFast = 1;
	}
	else {
		vty_out(vty,"% Bad parameter!\n");
		return CMD_WARNING;
	}

	fd = fopen("/dbm/product/slotcount", "r");
	if (fd == NULL)
	{
		fprintf(stderr,"Get production information [2] error\n");
		return -1;
	}
	fscanf(fd, "%d", &slot_count);
	fclose(fd);

	if(0 == strncmp(argv[1],"all",strlen(argv[1]))) {
		slot_id = 0;
	}
	else if (((strlen(argv[1]) == 1) && (argv[1][0] <= '9') && (argv[1][0] > '0')) || \
		(strlen(argv[1]) == 2) && (argv[1][0] == '1') && (argv[1][1] == '0'))
	{
		sscanf(argv[1],"%d", &slot_id);
		vty_out(vty, "reset slot_id : %d\n", slot_id);

		if (slot_id > slot_count)
		{
			vty_out(vty, "slot number range on this product : 1 ~ %d\n", slot_count);

			return CMD_SUCCESS;
		}
	}
	else
	{
		vty_out(vty, "error slot number : %s\n", argv[1]);
		vty_out(vty, "correct slot number option : all or 1 ~ %d\n", slot_count);

		return CMD_SUCCESS;
	}

	if(is_active_master && is_distributed)
	{
		DBusMessage *query, *reply;
		DBusError err;
		unsigned int op_ret;
		
		query = dbus_message_new_method_call(
									SEM_DBUS_BUSNAME,\
									SEM_DBUS_OBJPATH,\
									SEM_DBUS_INTERFACE,\
									SEM_DBUS_RESET_ALL);
		
		dbus_error_init(&err);

		dbus_message_append_args(query,
								 DBUS_TYPE_UINT32, &isFast,
								 DBUS_TYPE_UINT32, &slot_id,
								 DBUS_TYPE_INVALID);

		op_ret = dbus_connection_send(dcli_dbus_connection , query, NULL);
		
		dbus_message_unref(query);
		
		
		return CMD_SUCCESS;
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
	}else{
	vty_out(vty,"only active master can reset system\n");
	}

	return CMD_SUCCESS;
}

/* start - sfd by zhengbo */
int check_sfdkern_is_install(void )
{
	
	FILE *fp=NULL;	
	char line[MEM_COMMAND_LEN];
	char module[64];
	unsigned int size;
	int used;
	char used_by[256];
	int ret=0;
	fp = popen("sudo lsmod","r"); 
		
	if(NULL == fp)	
	{		
		vty_out (vty,"Find module error\n"); 
		return CMD_WARNING; 
	} 

	memset(line,0,MEM_COMMAND_LEN);
	fgets(line,MEM_COMMAND_LEN,fp); 
	while(fscanf(fp,"%s%u%d%s",module,&size,&used,used_by)!= -1)
	{
		if(strcmp("sfd_kern",module)==0)
			ret=1;
	}
	pclose(fp);
	return ret;
}

DEFUN (dcli_sfd_log_switch,
			dcli_sfd_log_switch_cmd,
			"service sfd log (enable|disable)",
			"config system service\n"
			"config sfd service\n"
			"config log function for sfd service\n"
			"enable log function for sfd service\n"
			"disable log function for sfd service\n")
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	int op_ret = -1;
	int sfdkern_is_exist;
	int cmd = 0;
	int i;
	int slot_count = 15;
	if(!strncmp("enable",argv[0],strlen(argv[0])))
		cmd = 1;
	query = dbus_message_new_method_call(SFD_DBUS_BUSNAME, 
											SFD_DBUS_OBJPATH, 
											SFD_DBUS_INTERFACE, 
									SFD_DBUS_METHOD_LOG);
	dbus_error_init(&err);
	dbus_message_append_args(query,
								DBUS_TYPE_INT32, &cmd,
						DBUS_TYPE_INVALID);
	for(i=1;i<=slot_count;i++)
	{
		if(NULL != (dbus_connection_dcli[i] -> dcli_dbus_connection))
		{
			reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[i] -> dcli_dbus_connection, query, 10000, &err);
			
			if(NULL == reply)
			{
				vty_out(vty, "dbus failed get reply.\n");
				if(dbus_error_is_set(&err))
				{
					vty_out(vty, "%s raised: %s", err.name, err.message);
					dbus_error_free_for_dcli(&err);
				}
				dbus_message_unref(query);
				return CMD_WARNING;
			}

			if(dbus_message_get_args(reply,
										&err,									
										DBUS_TYPE_INT32, &sfdkern_is_exist,
										DBUS_TYPE_INT32, &op_ret,
										DBUS_TYPE_INVALID)) 
			{
				dbus_message_unref(reply);
				if(sfdkern_is_exist==0)
				{
					vty_out(vty,"Slot %d service sfd is disable\n",i);
				
				}
				if(op_ret != 0) 
				{
					vty_out(vty, "Slot %d commond executed failed...\n",i);
				}
			} 
			else 
			{ 	
				dbus_message_unref(reply);
				if (dbus_error_is_set(&err)) 
				{
					vty_out(vty, "%s raised: %s", err.name, err.message);
					dbus_error_free_for_dcli(&err);
				}
				dbus_message_unref(query);
				return CMD_WARNING;
			}
		}
		
	}   /*end for*/
	dbus_message_unref(query);
	return CMD_SUCCESS;
}

DEFUN (dcli_sfd_log_switch_slot,
			dcli_sfd_log_switch_slot_cmd,
			"service sfd log <1-15> (enable|disable)",
			"config system service\n"
			"config sfd service\n"
			"config log function for sfd service\n"
			"config slot <1-15> log function for sfd service\n"
			"enable log function for sfd service\n"
			"disable log function for sfd service\n")
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	int op_ret = -1;
	int sfdkern_is_exist;
	int cmd = 0;
	int slot_id;

	slot_id= atoi(argv[0]);
	if(!strncmp("enable",argv[1],strlen(argv[1])))
		cmd = 1;
	if(NULL != (dbus_connection_dcli[slot_id] -> dcli_dbus_connection))
	{
		query = dbus_message_new_method_call(SFD_DBUS_BUSNAME, 
											SFD_DBUS_OBJPATH, 
											SFD_DBUS_INTERFACE, 
											SFD_DBUS_METHOD_LOG);
		dbus_error_init(&err);
		dbus_message_append_args(query,
							DBUS_TYPE_INT32, &cmd,
							DBUS_TYPE_INVALID);
		reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[slot_id] -> dcli_dbus_connection, query, 10000, &err);
		dbus_message_unref(query);
		if(NULL == reply) 
		{
			vty_out(vty,"<error> failed get reply.\n");
			if(dbus_error_is_set(&err))
			{
				vty_out(vty, "%s raised: %s", err.name, err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_WARNING;
		}
		if(dbus_message_get_args(reply,
									&err,									
									DBUS_TYPE_INT32, &sfdkern_is_exist,
									DBUS_TYPE_INT32, &op_ret,
									DBUS_TYPE_INVALID)) 
		{
			dbus_message_unref(reply);
			if(sfdkern_is_exist==0)
			{
				vty_out(vty,"service sfd is disable\n");
				return CMD_WARNING;
			}
			if(op_ret == 0) 
			{
				return CMD_SUCCESS;
			}
			else 
			{
				vty_out(vty, "commond executed failed...\n");
				return CMD_WARNING;
			}
		} 
		else 
		{ 	
			dbus_message_unref(reply);
			if (dbus_error_is_set(&err)) 
			{
				vty_out(vty, "%s raised: %s", err.name, err.message);
				dbus_error_free_for_dcli(&err);
			}
			
			return CMD_WARNING;
		}

	}
	else
	{
		vty_out(vty,"The slot doesn't exist");
		return CMD_WARNING;
	}
	
}

DEFUN (dcli_sfd_debug_switch,
			dcli_sfd_debug_switch_cmd,
			"service sfd debug (enable|disable)",
			"config system service\n"
			"config sfd service\n"
			"config debug mode for sfd service\n"
			"enable debug mode for sfd service\n"
			"disable debug mode for sfd service\n")
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	int op_ret = -1;
	int cmd = 0;
	int sfdkern_is_exist;
	int i;
	int slot_count = 15;
	if(!strncmp("enable", argv[0],strlen(argv[0])))
		cmd = 1;

	query = dbus_message_new_method_call(SFD_DBUS_BUSNAME, 
													SFD_DBUS_OBJPATH, 
													SFD_DBUS_INTERFACE, 
													SFD_DBUS_METHOD_DEBUG);
	dbus_error_init(&err);
	dbus_message_append_args(query,
								DBUS_TYPE_INT32, &cmd,
								DBUS_TYPE_INVALID);
	for(i=1;i<=slot_count;i++)
	{
		
		if(NULL != (dbus_connection_dcli[i] -> dcli_dbus_connection))
		{
			reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[i] -> dcli_dbus_connection, query, 10000, &err);
			
			if(NULL == reply) {
				vty_out(vty, "dbus failed get reply.\n");
				if(dbus_error_is_set(&err)) {
					vty_out(vty, "%s raised: %s", err.name, err.message);
					dbus_error_free_for_dcli(&err);
				}
				dbus_message_unref(query);
				return CMD_WARNING;
			}

			if(dbus_message_get_args(reply,
										&err,									
										DBUS_TYPE_INT32, &sfdkern_is_exist,
										DBUS_TYPE_INT32, &op_ret,
										DBUS_TYPE_INVALID)) 
			{
				dbus_message_unref(reply);
				if(sfdkern_is_exist==0)
				{
					vty_out(vty,"Slot %d service sfd is disable\n",i);
					
				}
				if(op_ret != 0) 
				{
					vty_out(vty, "Slot %d commond executed failed...\n",i);
				}
			} 
			else 
			{	
				dbus_message_unref(reply);
				if (dbus_error_is_set(&err)) 
				{
					vty_out(vty, "%s raised: %s", err.name, err.message);
					dbus_error_free_for_dcli(&err);
				}
				dbus_message_unref(query);
				return CMD_WARNING;
			}
				
		}
	}
	dbus_message_unref(query);
	return CMD_SUCCESS;
}
DEFUN (dcli_sfd_debug_switch_slot,
			dcli_sfd_debug_switch_slot_cmd,
			"service sfd debug <1-15> (enable|disable)",
			"config system service\n"
			"config sfd service\n"
			"config debug mode for sfd service\n"
			"config slot <1-15> debug mode for sfd service\n"
			"enable debug mode for sfd service\n"
			"disable debug mode for sfd service\n")
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	int op_ret = -1;
	int sfdkern_is_exist;
	int cmd = 0;
	int slot_id;

	slot_id= atoi(argv[0]);
	if(!strncmp("enable",argv[1],strlen(argv[1])))
		cmd = 1;
	if(NULL != (dbus_connection_dcli[slot_id] -> dcli_dbus_connection))
	{
		query = dbus_message_new_method_call(SFD_DBUS_BUSNAME, 
											SFD_DBUS_OBJPATH, 
											SFD_DBUS_INTERFACE, 
											SFD_DBUS_METHOD_DEBUG);
		dbus_error_init(&err);
		dbus_message_append_args(query,
							DBUS_TYPE_INT32, &cmd,
							DBUS_TYPE_INVALID);
		reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[slot_id] -> dcli_dbus_connection, query, 10000, &err);
		dbus_message_unref(query);
		if(NULL == reply) 
		{
			vty_out(vty,"<error> failed get reply.\n");
			if(dbus_error_is_set(&err))
			{
				vty_out(vty, "%s raised: %s", err.name, err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_WARNING;
		}
		if(dbus_message_get_args(reply,
									&err,									
									DBUS_TYPE_INT32, &sfdkern_is_exist,
									DBUS_TYPE_INT32, &op_ret,
									DBUS_TYPE_INVALID)) 
		{
			dbus_message_unref(reply);
			if(sfdkern_is_exist==0)
			{
				vty_out(vty,"service sfd is disable\n");
				return CMD_WARNING;
			}
			if(op_ret == 0) 
			{
				return CMD_SUCCESS;
			}
			else 
			{
				vty_out(vty, "commond executed failed...\n");
				return CMD_WARNING;
			}
		} 
		else 
		{ 	
			dbus_message_unref(reply);
			if (dbus_error_is_set(&err)) 
			{
				vty_out(vty, "%s raised: %s", err.name, err.message);
				dbus_error_free_for_dcli(&err);
			}
			
			return CMD_WARNING;
		}

	}
	else
	{
		vty_out(vty,"The slot doesn't exist");
		return CMD_WARNING;
	}
}



DEFUN (dcli_sfd_switch,
		dcli_sfd_switch_cmd,
		"service (sfd|sfdtcp|sfdicmp|sfdsnmp|sfddns|sfdcapwap) (enable|disable)",
		"config system service\n"
		"config sfd service\n"
		"config tcp detection for sfd service\n"
		"config icmp detection for sfd service\n"
		"config snmp detection for sfd service\n"
		"config dns detection for sfd service\n"
		"config capwap detection for sfd service\n"
		"enable detection for sfd service\n"
		"disable detection for sfd service\n")
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	int op_ret = -1;
	int cmd = 0;
	int i;
	int sfdflag;
	int sfdkern_is_exist;
	int type_ret;
	
	int slot_count = 15;
	if(!strncmp("sfd", argv[0],strlen(argv[0])))
	{
		if(!strncmp("enable", argv[1],strlen(argv[1])))
		{	
			cmd = 1;
			sfdflag=ENABLE_SFD;
		}/*enable end*/
		else
		{
			cmd = 0;
			sfdflag=DISABLE_SFD;
		}
	}/*sfd end*/
	else if(!strncmp("sfdtcp", argv[0],strlen(argv[0])))
	{
		cmd = 1;
		if(!strncmp("enable", argv[1],strlen(argv[1])))
			sfdflag=ENABLE_TCP;
		else
			sfdflag=DISABLE_TCP;	
	}
	else if(!strncmp("sfdicmp", argv[0],strlen(argv[0])))
	{
		cmd = 1;
		if(!strncmp("enable", argv[1],strlen(argv[1])))
			sfdflag=ENABLE_ICMP;
		else
			sfdflag=DISABLE_ICMP;
	}
	else if(!strncmp("sfdsnmp", argv[0],strlen(argv[0])))
	{
		cmd = 1;
		if(!strncmp("enable", argv[1],strlen(argv[1])))
			sfdflag=ENABLE_SNMP;
		else
			sfdflag=DISABLE_SNMP;	
	}
	else if(!strncmp("sfddns", argv[0],strlen(argv[0])))
	{
		cmd = 1;
		if(!strncmp("enable", argv[1],strlen(argv[1])))
			sfdflag=ENABLE_DNS;
		else
			sfdflag=DISABLE_DNS;	
	}
	else if(!strncmp("sfdcapwap", argv[0],strlen(argv[0])))
	{
		cmd = 1;
		if(!strncmp("enable", argv[1],strlen(argv[1])))
			sfdflag=ENABLE_CAPWAP;
		else
			sfdflag=DISABLE_CAPWAP;	
	}
	/*
	vty_out(vty,"sfdlag=%d,cmd=%d\n",sfdflag,cmd);
	*/
	query = dbus_message_new_method_call(SFD_DBUS_BUSNAME, 
														SFD_DBUS_OBJPATH, 
														SFD_DBUS_INTERFACE, 
															SFD_DBUS_METHOD_SWT_SLOT);
	dbus_error_init(&err);
	dbus_message_append_args(query,
								DBUS_TYPE_INT32, &cmd,
								DBUS_TYPE_INT32, &sfdflag,
								DBUS_TYPE_INVALID);
	for(i=1;i<=slot_count;i++)
	{
		if(NULL != (dbus_connection_dcli[i] -> dcli_dbus_connection))
		{
			
			reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[i] -> dcli_dbus_connection, query, 10000, &err);
			
			if(NULL == reply)
			{
				vty_out(vty, "dbus failed get reply.\n");
				if(dbus_error_is_set(&err))
				{
					vty_out(vty, "%s raised: %s", err.name, err.message);
					dbus_error_free_for_dcli(&err);
				}
				dbus_message_unref(query);
				return CMD_WARNING;
			}

			if(dbus_message_get_args(reply,
										&err,									
										DBUS_TYPE_INT32, &sfdkern_is_exist,
										DBUS_TYPE_INT32, &type_ret,
										DBUS_TYPE_INT32, &op_ret,
										DBUS_TYPE_INVALID)) 
			{
				dbus_message_unref(reply);
				if(sfdkern_is_exist==1)
				{
					if(type_ret!=ENABLE_SFD)
					{	
						if(op_ret != 0)
						{
							vty_out(vty, "Slot %d commond executed failed...\n",i);
						}
					}
					else
					{
						vty_out(vty,"Slot %d service sfd have been enable\n",i);
						
					}
					
				}
				else if(sfdkern_is_exist==0)
				{
					if(type_ret==ENABLE_SFD)
					{
						if(op_ret != 0)  
						{
							vty_out(vty, "Slot %d commond executed failed...\n",i);
							
						}
					}
					else
					{
						vty_out(vty,"Slot %d service sfd have been disable\n",i);
						
					}
				}
				else
				{
					vty_out(vty,"Slot %d find sfd service error\n",i);
					return CMD_WARNING;
				}
				
			} 
			else 
			{ 	
				dbus_message_unref(reply);
				if (dbus_error_is_set(&err)) 
				{
					vty_out(vty, "%s raised: %s", err.name, err.message);
					dbus_error_free_for_dcli(&err);
				}
				dbus_message_unref(query);
				return CMD_WARNING;
			}
		}
	}  /*end for*/
	dbus_message_unref(query);
	return CMD_SUCCESS;	
}

DEFUN (dcli_sfd_switch_slot,
		dcli_sfd_switch_slot_cmd,
		"service (sfd|sfdtcp|sfdicmp|sfdsnmp|sfddns|sfdcapwap) <1-15> (enable|disable)",
		"config system service\n"
		"config sfd service\n"
		"config tcp detection for sfd service\n"
		"config icmp detection for sfd service\n"
		"config snmp detection for sfd service\n"
		"config dns detection for sfd service\n"
		"config capwap detection for sfd service\n"
		"config slot <1-15> detection for sfd service\n"
		"enable detection for sfd service\n"
		"disable detection for sfd service\n")
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	int op_ret = -1;
	int sfdkern_is_exist;
	int slot_id;
	int cmd = 0;
	int sfdflag;
	int type_ret;
	
	slot_id = atoi(argv[1]);
	if(!strncmp("sfd", argv[0],strlen(argv[0])))
	{
		if(!strncmp("enable", argv[2],strlen(argv[2])))
		{	
			cmd = 1;
			sfdflag=ENABLE_SFD;
		}/*enable end*/
		else
		{
			cmd = 0;
			sfdflag=DISABLE_SFD;
		}
	}/*sfd end*/
	else if(!strncmp("sfdtcp", argv[0],strlen(argv[0])))
	{	
		cmd = 1;
		if(!strncmp("enable", argv[2],strlen(argv[2])))
			sfdflag=ENABLE_TCP;
		else
			sfdflag=DISABLE_TCP;	
	}
	else if(!strncmp("sfdicmp", argv[0],strlen(argv[0])))
	{
		cmd = 1;
		if(!strncmp("enable", argv[2],strlen(argv[2])))
			sfdflag=ENABLE_ICMP;
		else
			sfdflag=DISABLE_ICMP;
	}
	else if(!strncmp("sfdsnmp", argv[0],strlen(argv[0])))
	{
		cmd = 1;
		if(!strncmp("enable", argv[2],strlen(argv[2])))
			sfdflag=ENABLE_SNMP;
		else
			sfdflag=DISABLE_SNMP;	
	}
	else if(!strncmp("sfddns", argv[0],strlen(argv[0])))
	{
		cmd = 1;
		if(!strncmp("enable", argv[2],strlen(argv[2])))
			sfdflag=ENABLE_DNS;
		else
			sfdflag=DISABLE_DNS;	
	}
	else if(!strncmp("sfdcapwap", argv[0],strlen(argv[0])))
	{
		cmd = 1;
		if(!strncmp("enable", argv[2],strlen(argv[2])))
			sfdflag=ENABLE_CAPWAP;
		else
			sfdflag=DISABLE_CAPWAP;	
	}
	/*
	vty_out(vty,"sfdlag=%d,cmd=%d\n",sfdflag,cmd);
	*/
	if(NULL != (dbus_connection_dcli[slot_id] -> dcli_dbus_connection))
	{
		query = dbus_message_new_method_call(SFD_DBUS_BUSNAME, 
											SFD_DBUS_OBJPATH, 
											SFD_DBUS_INTERFACE, 
											SFD_DBUS_METHOD_SWT_SLOT);
		dbus_error_init(&err);
		dbus_message_append_args(query,
								DBUS_TYPE_INT32, &cmd,
								DBUS_TYPE_INT32, &sfdflag,
								DBUS_TYPE_INVALID);
		reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[slot_id] -> dcli_dbus_connection, query, 10000, &err);
		dbus_message_unref(query);
		
		if(NULL == reply) 
		{
			vty_out(vty,"<error> failed get reply.\n");
			if(dbus_error_is_set(&err))
			{
				vty_out(vty, "%s raised: %s", err.name, err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_WARNING;
		}
		
		if(dbus_message_get_args(reply,
									&err,									
									DBUS_TYPE_INT32, &sfdkern_is_exist,
									DBUS_TYPE_INT32, &type_ret,
									DBUS_TYPE_INT32, &op_ret,
									DBUS_TYPE_INVALID)) 
		{
			dbus_message_unref(reply);
			if(sfdkern_is_exist==1)
			{
				if(type_ret!=ENABLE_SFD)
				{	if(op_ret == 0) 
					{
						return CMD_SUCCESS;
					}
					else 
					{
						vty_out(vty, "commond executed failed...\n");
						return CMD_WARNING;
					}
				}
				else
				{
					vty_out(vty,"service sfd have been enable\n");
					return CMD_WARNING;
				}
				
			}
			else if(sfdkern_is_exist==0)
			{
				if(type_ret==ENABLE_SFD)
				{
					if(op_ret == 0) 
					{
						return CMD_SUCCESS;
					}
					else 
					{
						vty_out(vty, "commond executed failed...\n");
						return CMD_WARNING;
					}
				}
				else
				{
					vty_out(vty,"service sfd is disable\n");
					return CMD_WARNING;
				}
			}
			else
			{
				vty_out(vty,"Find module error\n");
				return CMD_WARNING;
			}
			
		} 
		else 
		{ 	
			dbus_message_unref(reply);
			if (dbus_error_is_set(&err)) 
			{
				vty_out(vty, "%s raised: %s", err.name, err.message);
				dbus_error_free_for_dcli(&err);
			}
			
			return CMD_WARNING;
		}
	}
	else
	{
		vty_out(vty,"The slot doesn't exist");
		return CMD_WARNING;
	}	
}

DEFUN (dcli_sfd_timespan,
		dcli_sfd_timespan_cmd,
		"service sfd timespan <10-10000>",
		"config system service\n"
		"config sfd service\n"
		"config timespan for sfd detection\n"
		"<10-10000>ms, timespan of sfd detection\n")
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	int op_ret = -1;
	int value;
	int sfdkern_is_exist;
	int i;
	int slot_count = 15;
	value = atoi(argv[0]);
	query = dbus_message_new_method_call(SFD_DBUS_BUSNAME, 
													SFD_DBUS_OBJPATH, 
													SFD_DBUS_INTERFACE, 
													SFD_DBUS_METHOD_TMS);
	dbus_error_init(&err);
	dbus_message_append_args(query,
								DBUS_TYPE_INT32, &value,
								DBUS_TYPE_INVALID);
	for(i=1;i<=slot_count;i++)
	{
		if(NULL != (dbus_connection_dcli[i] -> dcli_dbus_connection))
		{
			
			reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[i] -> dcli_dbus_connection, query, 10000, &err);
			
			if(NULL == reply) 
			{
				vty_out(vty, "dbus failed get reply.\n");
				if(dbus_error_is_set(&err))
				{
					vty_out(vty, "%s raised: %s", err.name, err.message);
					dbus_error_free(&err);
				}
				dbus_message_unref(query);
				return CMD_WARNING;
			}

			if(dbus_message_get_args(reply,
											&err,									
											DBUS_TYPE_INT32, &sfdkern_is_exist,
											DBUS_TYPE_INT32, &op_ret,
											DBUS_TYPE_INVALID)) 
			{
				dbus_message_unref(reply);
				if(sfdkern_is_exist==0)
				{
					vty_out(vty,"Slot %d service sfd is disable\n",i);
				}
				if(op_ret != 0)
				{
					vty_out(vty, "Slot %d commond executed failed...\n",i);
				}
			} 
			else 
			{ 	
				dbus_message_unref(reply);
				if (dbus_error_is_set(&err)) 
				{
					vty_out(vty, "%s raised: %s", err.name, err.message);
					dbus_error_free_for_dcli(&err);
				}
				dbus_message_unref(query);
				return CMD_WARNING;
			}
		}
	}  	/*end for*/
	dbus_message_unref(query);
	return CMD_SUCCESS;
}
DEFUN (dcli_sfd_timespan_slot,
		dcli_sfd_timespan_slot_cmd,
		"service sfd <1-15> timespan <10-10000>",
		"config system service\n"
		"config sfd service\n"
		"config slot <1-15> detection for sfd service\n"
		"config timespan for sfd detection\n"
		"<10-10000>ms, timespan of sfd detection\n")
{
	
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	int op_ret = -1;
	int sfdkern_is_exist;
	int value;
	int slot_id;

	slot_id= atoi(argv[0]);
	value = atoi(argv[1]);

	if(NULL != (dbus_connection_dcli[slot_id] -> dcli_dbus_connection))
	{
		query = dbus_message_new_method_call(SFD_DBUS_BUSNAME, 
											SFD_DBUS_OBJPATH, 
											SFD_DBUS_INTERFACE, 
											SFD_DBUS_METHOD_TMS);
		dbus_error_init(&err);
		dbus_message_append_args(query,
							DBUS_TYPE_INT32, &value,
							DBUS_TYPE_INVALID);
		reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[slot_id] -> dcli_dbus_connection, query, 10000, &err);
		dbus_message_unref(query);
		if(NULL == reply) 
		{
			vty_out(vty,"<error> failed get reply.\n");
			if(dbus_error_is_set(&err))
			{
				vty_out(vty, "%s raised: %s", err.name, err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_WARNING;
		}
		
		if(dbus_message_get_args(reply,
									&err,									
									DBUS_TYPE_INT32, &sfdkern_is_exist,
									DBUS_TYPE_INT32, &op_ret,
									DBUS_TYPE_INVALID)) 
		{
			dbus_message_unref(reply);
			if(sfdkern_is_exist==0)
			{
				vty_out(vty,"service sfd is disable\n");
				return CMD_WARNING;
			}
			if(op_ret == 0) 
			{
				return CMD_SUCCESS;
			}
			else 
			{
				vty_out(vty, "commond executed failed...\n");
				return CMD_WARNING;
			}
		} 
		else 
		{ 	
			dbus_message_unref(reply);
			if (dbus_error_is_set(&err)) 
			{
				vty_out(vty, "%s raised: %s", err.name, err.message);
				dbus_error_free_for_dcli(&err);
			}
			
			return CMD_WARNING;
		}

	}
	else
	{
		vty_out(vty,"The slot doesn't exist");
		return CMD_WARNING;
	}
}

DEFUN (dcli_sfd_limitpacket,
			dcli_sfd_limitpacket_cmd,
			"service sfd packetlimit (tcp|icmp|snmp|capwap) <0-1073741824>",
			"config system service\n"
			"config sfd service\n"
			"config limit packets for sfd service\n"
			"config limit packets for tcp detection\n"
			"config limit packets for icmp detection\n"
			"config limit packets for snmp detection\n"
			"config limit packets for capwap detection\n"
			"packets, limit packets of detection\n")
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	int op_ret = -1;
	int value;
	int sfdkern_is_exist;
	int protoflag = 0;
	int i;
	int slot_count = 15;
	
	if(strncmp(argv[0],"tcp",strlen(argv[0]))==0)
		protoflag = FLAG_TCP;
	else if(strncmp(argv[0],"icmp",strlen(argv[0]))==0)
		protoflag = FLAG_ICMP;
	else if(strncmp(argv[0],"snmp",strlen(argv[0]))==0)
		protoflag = FLAG_SNMP;
	else if(strncmp(argv[0],"capwap",strlen(argv[0]))==0)
		protoflag = FLAG_CAPWAP;
	else
		protoflag = 0;
	
	value = atoi(argv[1]);
	query = dbus_message_new_method_call(SFD_DBUS_BUSNAME, 
											SFD_DBUS_OBJPATH, 
											SFD_DBUS_INTERFACE, 
											SFD_DBUS_METHOD_LMT);
	dbus_error_init(&err);
	dbus_message_append_args(query,
								DBUS_TYPE_INT32, &protoflag,
								DBUS_TYPE_INT32, &value,
								DBUS_TYPE_INVALID);
	for(i=1;i<=slot_count;i++)
	{
		if(NULL != (dbus_connection_dcli[i] -> dcli_dbus_connection))
		{
			
			reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[i] -> dcli_dbus_connection, query, 10000, &err);
			
			if(NULL == reply)
			{
				vty_out(vty, "dbus failed get reply.\n");
				if(dbus_error_is_set(&err))
				{
					vty_out(vty, "%s raised: %s", err.name, err.message);
					dbus_error_free_for_dcli(&err);
				}	
				dbus_message_unref(query);
				return CMD_WARNING;
			}

			if(dbus_message_get_args(reply,
											&err,									
											DBUS_TYPE_INT32, &sfdkern_is_exist,
											DBUS_TYPE_INT32, &op_ret,
											DBUS_TYPE_INVALID)) 
			{
				dbus_message_unref(reply);
				if(sfdkern_is_exist==0)
				{
					vty_out(vty,"Slot %d service sfd is disable\n",i);
				}
			
				if(op_ret != 0) 
				{
					vty_out(vty, "Slot %d commond executed failed...\n",i);
				}
			
				
			} 
			else 
			{ 	
				dbus_message_unref(reply);
				if (dbus_error_is_set(&err)) 
				{
					vty_out(vty, "%s raised: %s", err.name, err.message);
					dbus_error_free_for_dcli(&err);
				}
				dbus_message_unref(query);
				return CMD_WARNING;
			}
		}
	}  	/*end for*/	
	dbus_message_unref(query);
	return CMD_SUCCESS;
}

DEFUN (dcli_sfd_limitpacket_slot,
		dcli_sfd_limitpacket_slot_cmd,
		"service sfd packetlimit <1-15> (tcp|icmp|snmp|capwap) <0-1073741824>",
		"config system service\n"
		"config sfd service\n"
		"config limit packets for sfd service\n"
		"config slot <1-15> limit packets for sfd service\n"
		"config limit packets for tcp detection\n"
		"config limit packets for icmp detection\n"
		"config limit packets for snmp detection\n"
		"config limit packets for capwap detection\n"
		"packets, limit packets of detection\n")
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	int op_ret = -1;
	int value;
	int slot_id;
	int protoflag = 0;
	int sfdkern_is_exist;
	
	if(strncmp(argv[1],"tcp",strlen(argv[1]))==0)
		protoflag = FLAG_TCP;
	else if(strncmp(argv[1],"icmp",strlen(argv[1]))==0)
		protoflag = FLAG_ICMP;
	else if(strncmp(argv[1],"snmp",strlen(argv[1]))==0)
		protoflag = FLAG_SNMP;
	else if(strncmp(argv[1],"capwap",strlen(argv[1]))==0)
		protoflag = FLAG_CAPWAP;
	else
		protoflag = 0;
	
	slot_id= atoi(argv[0]);
	value = atoi(argv[2]);
	if(NULL != (dbus_connection_dcli[slot_id] -> dcli_dbus_connection))
	{
		query = dbus_message_new_method_call(SFD_DBUS_BUSNAME, 
												SFD_DBUS_OBJPATH, 
												SFD_DBUS_INTERFACE, 
												SFD_DBUS_METHOD_LMT);
		dbus_error_init(&err);
		dbus_message_append_args(query,
									DBUS_TYPE_INT32, &protoflag,
									DBUS_TYPE_INT32, &value,
									DBUS_TYPE_INVALID);
		reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[slot_id] -> dcli_dbus_connection, query, 10000, &err);
		dbus_message_unref(query);
		if(NULL == reply) 
		{
			vty_out(vty, "dbus failed get reply.\n");
			if(dbus_error_is_set(&err))
			{
				vty_out(vty, "%s raised: %s", err.name, err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_WARNING;
		}

	
		if(dbus_message_get_args(reply,
									&err,									
									DBUS_TYPE_INT32, &sfdkern_is_exist,
									DBUS_TYPE_INT32, &op_ret,
									DBUS_TYPE_INVALID)) 
		{
			dbus_message_unref(reply);
			if(sfdkern_is_exist==0)
			{
				vty_out(vty,"service sfd is disable\n");
				return CMD_WARNING;
			}
			if(op_ret == 0) 
			{
				return CMD_SUCCESS;
			}
			else 
			{
				vty_out(vty, "commond executed failed...\n");
				return CMD_WARNING;
			}
		} 
		else 
		{	
			dbus_message_unref(reply);
			if (dbus_error_is_set(&err)) 
			{
				vty_out(vty, "%s raised: %s", err.name, err.message);
				dbus_error_free_for_dcli(&err);
			}
			
			return CMD_WARNING;
		}

	}
	else
	{
		vty_out(vty,"The slot doesn't exist");
		return CMD_WARNING;
	}	
}

DEFUN (dcli_sfd_user_add,
		dcli_sfd_user_add_cmd,
		"service sfd user add MAC IP",
		"config system service\n"
		"config sfd service\n"
		"config user infomation for sfd service\n"
		"add new user infomation for sfd service\n"
		"MAC address of user\n"
		"IP address of user\n")
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	int op_ret = -1;
	int sfdkern_is_exist;
	int i;
	int slot_count = 15;
	query = dbus_message_new_method_call(SFD_DBUS_BUSNAME, 
														SFD_DBUS_OBJPATH, 
														SFD_DBUS_INTERFACE, 
														SFD_DBUS_METHOD_USERADD);
	dbus_error_init(&err);
	dbus_message_append_args(query,
								DBUS_TYPE_STRING, &argv[0],
								DBUS_TYPE_STRING, &argv[1],
								DBUS_TYPE_INVALID);
	for(i=1;i<=slot_count;i++)
	{
		if(NULL != (dbus_connection_dcli[i] -> dcli_dbus_connection))
		{
			reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[i] -> dcli_dbus_connection, query, 10000, &err);
			
			if(NULL == reply)
			{
				vty_out(vty, "dbus failed get reply.\n");
				if(dbus_error_is_set(&err))
				{
					vty_out(vty, "%s raised: %s", err.name, err.message);
					dbus_error_free_for_dcli(&err);
				}
				dbus_message_unref(query);
				return CMD_WARNING;
			}

			
			if(dbus_message_get_args(reply,
										&err,									
										DBUS_TYPE_INT32, &sfdkern_is_exist,
										DBUS_TYPE_INT32, &op_ret,
										DBUS_TYPE_INVALID)) 
			{
				dbus_message_unref(reply);
				if(sfdkern_is_exist==0)
				{
					vty_out(vty,"Slot %d service sfd is disable\n",i);
				}
				if(op_ret != 0)  
				{
					vty_out(vty, "Slot %d commond executed failed...\n",i);
				}
			} 
			else 
			{	
				dbus_message_unref(reply);
				if (dbus_error_is_set(&err)) 
				{
					vty_out(vty, "%s raised: %s", err.name, err.message);
					dbus_error_free_for_dcli(&err);
				}
				dbus_message_unref(query);
				return CMD_WARNING;
			}
		}
	}	/*end for*/
	dbus_message_unref(query);
	return CMD_SUCCESS;
}

DEFUN (dcli_sfd_user_add_slot,
			dcli_sfd_user_add_slot_cmd,
			"service sfd user add <1-15> MAC IP",
			"config system service\n"
			"config sfd service\n"
			"config user infomation for sfd service\n"
			"add new user infomation for sfd service\n"
			"add new user infomation to slot <1-15> for sfd service\n"
			"MAC address of user\n"
			"IP address of user\n")
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	int op_ret = -1;
	int slot_id;
	int sfdkern_is_exist;
	slot_id= atoi(argv[0]);
	if(NULL != (dbus_connection_dcli[slot_id] -> dcli_dbus_connection))
	{
	
		query = dbus_message_new_method_call(SFD_DBUS_BUSNAME, 
												SFD_DBUS_OBJPATH, 
												SFD_DBUS_INTERFACE, 
												SFD_DBUS_METHOD_USERADD);
		dbus_error_init(&err);
		dbus_message_append_args(query,
									DBUS_TYPE_STRING, &argv[1],
									DBUS_TYPE_STRING, &argv[2],
									DBUS_TYPE_INVALID);
		reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[slot_id] -> dcli_dbus_connection, query, 10000, &err);
		dbus_message_unref(query);
	
		if(NULL == reply) 
		{
			vty_out(vty, "dbus failed get reply.\n");
			if(dbus_error_is_set(&err))
			{
				vty_out(vty, "%s raised: %s", err.name, err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_WARNING;
		}
		
		if(dbus_message_get_args(reply,
									&err,									
									DBUS_TYPE_INT32, &sfdkern_is_exist,
									DBUS_TYPE_INT32, &op_ret,
									DBUS_TYPE_INVALID)) 
		{
			dbus_message_unref(reply);
			if(sfdkern_is_exist==0)
			{
				vty_out(vty,"service sfd is disable\n");
				return CMD_WARNING;
			}
			if(op_ret == 0) 
			{
				return CMD_SUCCESS;
			}
			else 
			{
				vty_out(vty, "commond executed failed...\n");
				return CMD_WARNING;
			}
		} 
		else 
		{	
			dbus_message_unref(reply);
			if (dbus_error_is_set(&err)) 
			{
				vty_out(vty, "%s raised: %s", err.name, err.message);
				dbus_error_free_for_dcli(&err);
			}
			
			return CMD_WARNING;
		}
	}
	else
	{
		vty_out(vty,"The slot doesn't exist");
		return CMD_WARNING;
	}
}

DEFUN (dcli_sfd_user_del,
		dcli_sfd_user_del_cmd,
		"service sfd user del MAC",
		"config system service\n"
		"config sfd service\n"
		"config user infomation for service\n"
		"delete user infomation for sfd service\n"
		"MAC address of user\n")
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	int op_ret = -1;
	int sfdkern_is_exist;
	int i;
	int slot_count = 15;
	query = dbus_message_new_method_call(SFD_DBUS_BUSNAME, 
													SFD_DBUS_OBJPATH, 
													SFD_DBUS_INTERFACE, 
													SFD_DBUS_METHOD_USERDEL);
	dbus_error_init(&err);
	dbus_message_append_args(query,
								DBUS_TYPE_STRING, &argv[0],
								DBUS_TYPE_INVALID);
	for(i=1;i<=slot_count;i++)
	{
		if(NULL != (dbus_connection_dcli[i] -> dcli_dbus_connection))
		{
			reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[i] -> dcli_dbus_connection, query, 10000, &err);
			
			if(NULL == reply)
			{
				vty_out(vty, "dbus failed get reply.\n");
				if(dbus_error_is_set(&err))
				{
					vty_out(vty, "%s raised: %s", err.name, err.message);
					dbus_error_free_for_dcli(&err);
				}
				dbus_message_unref(query);
				return CMD_WARNING;
			}

			if(dbus_message_get_args(reply,
										&err,									
										DBUS_TYPE_INT32, &sfdkern_is_exist,
										DBUS_TYPE_INT32, &op_ret,
										DBUS_TYPE_INVALID)) 
			{
				dbus_message_unref(reply);
				if(sfdkern_is_exist==0)
				{
					vty_out(vty,"Slot %d service sfd is disable\n",i);
				}
				if(op_ret != 0) 
				{
					vty_out(vty, "Slot %d commond executed failed...\n",i);
				}
			} 
			else 
			{	
				dbus_message_unref(reply);
				if (dbus_error_is_set(&err)) 
				{
					vty_out(vty, "%s raised: %s", err.name, err.message);
					dbus_error_free_for_dcli(&err);
				}
				dbus_message_unref(query);
				return CMD_WARNING;
			}
		}
	}
	dbus_message_unref(query);
	return CMD_SUCCESS;
}
DEFUN (dcli_sfd_user_del_slot,
			dcli_sfd_user_del_slot_cmd,
			"service sfd user del <1-15> MAC",
			"config system service\n"
			"config sfd service\n"
			"config user infomation for sfd service\n"
			"delete user infomation for sfd service\n"
			"delete user infomation from slot <1-15> for sfd service\n"
			"MAC address of user\n")
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	int op_ret = -1;
	int slot_id;
	int sfdkern_is_exist;
	
	slot_id= atoi(argv[0]);
	if(NULL != (dbus_connection_dcli[slot_id] -> dcli_dbus_connection))
	{
	
		query = dbus_message_new_method_call(SFD_DBUS_BUSNAME, 
												SFD_DBUS_OBJPATH, 
												SFD_DBUS_INTERFACE, 
												SFD_DBUS_METHOD_USERDEL);
		dbus_error_init(&err);
		dbus_message_append_args(query,
									DBUS_TYPE_STRING, &argv[1],
									DBUS_TYPE_INVALID);
		reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[slot_id] -> dcli_dbus_connection, query, 10000, &err);
		dbus_message_unref(query);
	
		if(NULL == reply) 
		{
			vty_out(vty, "dbus failed get reply.\n");
			if(dbus_error_is_set(&err)) 
			{
				vty_out(vty, "%s raised: %s", err.name, err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_WARNING;
		}
		
		if(dbus_message_get_args(reply,
									&err,									
									DBUS_TYPE_INT32, &sfdkern_is_exist,
									DBUS_TYPE_INT32, &op_ret,
									DBUS_TYPE_INVALID)) 
		{
			dbus_message_unref(reply);
			if(sfdkern_is_exist==0)
			{
				vty_out(vty,"service sfd is disable\n");
				return CMD_WARNING;
			}
			if(op_ret == 0) 
			{
				return CMD_SUCCESS;
			}
			else 
			{
				vty_out(vty, "commond executed failed...\n");
				return CMD_WARNING;
			}
		} 
		else 
		{	
			dbus_message_unref(reply);
			if (dbus_error_is_set(&err)) 
			{
				vty_out(vty, "%s raised: %s", err.name, err.message);
				dbus_error_free_for_dcli(&err);
			}
			
			return CMD_WARNING;
		}
	}
	else
	{
		vty_out(vty,"The slot doesn't exist");
		return CMD_WARNING;
	}
}

DEFUN (dcli_sfd_arpswitch,
		dcli_sfd_arpswitch_cmd,
		"service sfdarp (enable|disable)",
		"config system service\n"
		"config arp detection for sfd service\n"
		"enable detection for sfd service\n"
		"disable detection for sfd service\n")
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	int op_ret = -1;
	int cmd = 0;
	int sfdkern_is_exist;
	int i;
	int slot_count = 15;
	if(!strncmp("enable", argv[0],strlen(argv[0])))
		cmd = 1;
	
	query = dbus_message_new_method_call(SFD_DBUS_BUSNAME, 
													SFD_DBUS_OBJPATH, 
													SFD_DBUS_INTERFACE, 
													SFD_DBUS_METHOD_ARP_SWT);
	dbus_error_init(&err);
	dbus_message_append_args(query,
								DBUS_TYPE_INT32, &cmd,
								DBUS_TYPE_INVALID);	
	for(i=1;i<=slot_count;i++)
	{
		if(NULL != (dbus_connection_dcli[i] -> dcli_dbus_connection))
		{
			reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[i] -> dcli_dbus_connection, query, 10000, &err);
			
			if(NULL == reply)
			{
				vty_out(vty, "dbus failed get reply.\n");
				if(dbus_error_is_set(&err))
				{
					vty_out(vty, "%s raised: %s", err.name, err.message);
					dbus_error_free_for_dcli(&err);
				}
				dbus_message_unref(query);
				return CMD_WARNING;
			}

			if(dbus_message_get_args(reply,
											&err,									
											DBUS_TYPE_INT32, &sfdkern_is_exist,
											DBUS_TYPE_INT32, &op_ret,
											DBUS_TYPE_INVALID)) 
			{
				dbus_message_unref(reply);
				if(sfdkern_is_exist==0)
				{
					vty_out(vty,"Slot %d service sfd is disable\n",i);
				}
				if(op_ret != 0) 
				{
					vty_out(vty, "Slot %d commond executed failed...\n",i);
				}
			} 
			else 
			{	
				dbus_message_unref(reply);
				if (dbus_error_is_set(&err)) 
				{
					vty_out(vty, "%s raised: %s", err.name, err.message);
					dbus_error_free_for_dcli(&err);
				}
				dbus_message_unref(query);
				return CMD_WARNING;
			}
		}
	}
	dbus_message_unref(query);
	return CMD_SUCCESS;
}

DEFUN (dcli_sfd_arpswitch_slot,
		dcli_sfd_arpswitch_slot_cmd,
		"service sfdarp <1-15> (enable|disable)",
		"config system service\n"
		"config arp detection for sfd service\n"
		"config slot <1-15> detection for sfd service\n"
		"enable detection for sfd service\n"
		"disable detection for sfd service\n")
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	int op_ret = -1;
	int cmd = 0;
	int slot_id;
	int sfdkern_is_exist;
	slot_id= atoi(argv[0]);
	if(NULL != (dbus_connection_dcli[slot_id] -> dcli_dbus_connection))
	{
	
		if(!strncmp("enable", argv[1],strlen(argv[1])))
			cmd = 1;

		query = dbus_message_new_method_call(SFD_DBUS_BUSNAME, 
												SFD_DBUS_OBJPATH, 
												SFD_DBUS_INTERFACE, 
												SFD_DBUS_METHOD_ARP_SWT);
		dbus_error_init(&err);
		dbus_message_append_args(query,
									DBUS_TYPE_INT32, &cmd,
									DBUS_TYPE_INVALID);
		reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[slot_id] -> dcli_dbus_connection, query, 10000, &err);
		dbus_message_unref(query);

		if(NULL == reply) 
		{
			vty_out(vty, "dbus failed get reply.\n");
			if(dbus_error_is_set(&err))
			{
				vty_out(vty, "%s raised: %s", err.name, err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_WARNING;
		}
		if(dbus_message_get_args(reply,
									&err,									
									DBUS_TYPE_INT32, &sfdkern_is_exist,
									DBUS_TYPE_INT32, &op_ret,
									DBUS_TYPE_INVALID)) 
		{
			dbus_message_unref(reply);
			if(sfdkern_is_exist==0)
			{
				vty_out(vty,"service sfd is disable\n");
				return CMD_WARNING;
			}
			if(op_ret == 0) 
			{
				return CMD_SUCCESS;
			}
			else 
			{
				vty_out(vty, "commond executed failed...\n");
				return CMD_WARNING;
			}
		} 
		else 
		{	
			dbus_message_unref(reply);
			if (dbus_error_is_set(&err)) 
			{
				vty_out(vty, "%s raised: %s", err.name, err.message);
				dbus_error_free_for_dcli(&err);
			}
			
			return CMD_WARNING;
		}
	}
	else
	{
		vty_out(vty,"The slot doesn't exist");
		return CMD_WARNING;
	}
}

DEFUN (dcli_sfd_arplimitpacket,
		dcli_sfd_arplimitpacket_cmd,
		"service sfd packetlimit arp <0-1073741824>",
		"config system service\n"
		"config sfd service\n"
		"config limit packets for sfd service\n"
		"config limit packets for arp detection\n"
		"packets, limit packets of detection\n"
		)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	int op_ret = -1;
	int value;
	int sfdkern_is_exist;
	int i;
	int slot_count = 15;

	value = atoi(argv[0]);
	query = dbus_message_new_method_call(SFD_DBUS_BUSNAME, 
													SFD_DBUS_OBJPATH, 
													SFD_DBUS_INTERFACE, 
													SFD_DBUS_METHOD_ARP_LMT);
	dbus_error_init(&err);
	dbus_message_append_args(query,
								DBUS_TYPE_INT32, &value,
								DBUS_TYPE_INVALID);
	for(i=1;i<=slot_count;i++)
	{
		if(NULL != (dbus_connection_dcli[i] -> dcli_dbus_connection))
		{
			reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[i] -> dcli_dbus_connection, query, 10000, &err);
			
			if(NULL == reply)
			{
				vty_out(vty, "dbus failed get reply.\n");
				if(dbus_error_is_set(&err)) {
					vty_out(vty, "%s raised: %s", err.name, err.message);
					dbus_error_free_for_dcli(&err);
				}
				dbus_message_unref(query);
				return CMD_WARNING;
			}

			if(dbus_message_get_args(reply,
										&err,									
										DBUS_TYPE_INT32, &sfdkern_is_exist,
										DBUS_TYPE_INT32, &op_ret,
										DBUS_TYPE_INVALID)) 
			{
				dbus_message_unref(reply);
				if(sfdkern_is_exist==0)
				{
					vty_out(vty,"Slot %d service sfd is disable\n",i);
				}
				if(op_ret != 0)
				{
					vty_out(vty, "Slot %d commond executed failed...\n",i);
				}
			} 
			else 
			{	
				dbus_message_unref(reply);
				if (dbus_error_is_set(&err)) 
				{
					vty_out(vty, "%s raised: %s", err.name, err.message);
					dbus_error_free_for_dcli(&err);
				}
				dbus_message_unref(query);
				return CMD_WARNING;
			}
		}
	}
	dbus_message_unref(query);
	return CMD_SUCCESS;
}

DEFUN (dcli_sfd_arplimitpacket_slot,
		dcli_sfd_arplimitpacket_slot_cmd,
		"service sfd packetlimit <1-15> arp <0-1073741824>",
		"config system service\n"
		"config sfd service\n"
		"config limit packets for sfd service\n"
		"config limit packets for arp detection\n"
		"config slot <>1-15 limit packets for sfd service\n"
		"packets, limit packets of detection\n")
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	int op_ret = -1;
	int value;
	int slot_id;
	int sfdkern_is_exist;
	slot_id= atoi(argv[0]);
	if(NULL != (dbus_connection_dcli[slot_id] -> dcli_dbus_connection))
	{
		value = atoi(argv[1]);

		query = dbus_message_new_method_call(SFD_DBUS_BUSNAME, 
												SFD_DBUS_OBJPATH, 
												SFD_DBUS_INTERFACE, 
												SFD_DBUS_METHOD_ARP_LMT);
		dbus_error_init(&err);
		dbus_message_append_args(query,
									DBUS_TYPE_INT32, &value,
									DBUS_TYPE_INVALID);
		reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[slot_id] -> dcli_dbus_connection, query, 10000, &err);
		dbus_message_unref(query);

		if(NULL == reply) 
		{
			vty_out(vty, "dbus failed get reply.\n");
			if(dbus_error_is_set(&err)) 
			{
				vty_out(vty, "%s raised: %s", err.name, err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_WARNING;
		}
	
		if(dbus_message_get_args(reply,
									&err,									
									DBUS_TYPE_INT32, &sfdkern_is_exist,
									DBUS_TYPE_INT32, &op_ret,
									DBUS_TYPE_INVALID)) 
		{
			dbus_message_unref(reply);
			if(sfdkern_is_exist==0)
			{
				vty_out(vty,"service sfd is disable\n");
				return CMD_WARNING;
			}
			if(op_ret == 0) 
			{
				return CMD_SUCCESS;
			}
			else 
			{
				vty_out(vty, "commond executed failed...\n");
				return CMD_WARNING;
			}
		} 
		else 
		{	
			dbus_message_unref(reply);
			if (dbus_error_is_set(&err)) 
			{
				vty_out(vty, "%s raised: %s", err.name, err.message);
				dbus_error_free_for_dcli(&err);
			}
			
			return CMD_WARNING;
		}
	}
	else
	{
		vty_out(vty,"The slot doesn't exist");
		return CMD_WARNING;
	}
}
DEFUN (dcli_sfd_user_addarp,
		dcli_sfd_user_addarp_cmd,
		"service sfd user addarp MAC",
		"config system service\n"
		"config sfd service\n"
		"config user infomation for sfd service\n"
		"add new arp user infomation for sfd service\n"
		"MAC address of arp user\n")
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	int op_ret = -1;
	int sfdkern_is_exist;
	int i;
	int slot_count = 15;
	query = dbus_message_new_method_call(SFD_DBUS_BUSNAME, 
														SFD_DBUS_OBJPATH, 
														SFD_DBUS_INTERFACE, 
														SFD_DBUS_METHOD_ARPUSERADD);
	dbus_error_init(&err);
	dbus_message_append_args(query,
								DBUS_TYPE_STRING, &argv[0],
								DBUS_TYPE_INVALID);
	for(i=1;i<=slot_count;i++)
	{
		if(NULL != (dbus_connection_dcli[i] -> dcli_dbus_connection))
		{
			reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[i] -> dcli_dbus_connection, query, 10000, &err);
			
			if(NULL == reply)
			{
				vty_out(vty, "dbus failed get reply.\n");
				if(dbus_error_is_set(&err))
				{
					vty_out(vty, "%s raised: %s", err.name, err.message);
					dbus_error_free_for_dcli(&err);
				}
				dbus_message_unref(query);
				return CMD_WARNING;
			}

			
			if(dbus_message_get_args(reply,
										&err,									
										DBUS_TYPE_INT32, &sfdkern_is_exist,
										DBUS_TYPE_INT32, &op_ret,
										DBUS_TYPE_INVALID)) 
			{
				dbus_message_unref(reply);
				if(sfdkern_is_exist==0)
				{
					vty_out(vty,"Slot %d service sfd is disable\n",i);
				}
				if(op_ret != 0)  
				{
					vty_out(vty, "Slot %d commond executed failed...\n",i);
				}
			} 
			else 
			{	
				dbus_message_unref(reply);
				if (dbus_error_is_set(&err)) 
				{
					vty_out(vty, "%s raised: %s", err.name, err.message);
					dbus_error_free_for_dcli(&err);
				}
				dbus_message_unref(query);
				return CMD_WARNING;
			}
		}
	}	/*end for*/
	dbus_message_unref(query);
	return CMD_SUCCESS;
}
DEFUN (dcli_sfd_user_addarp_slot,
			dcli_sfd_user_addarp_slot_cmd,
			"service sfd user addarp <1-15> MAC",
			"config system service\n"
			"config sfd service\n"
			"config user infomation for sfd service\n"
			"add new arp user infomation for sfd service\n"
			"add new arp user infomation to slot <1-15> for sfd service\n"
			"MAC address of arp user\n")
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	int op_ret = -1;
	int slot_id;
	int sfdkern_is_exist;
	slot_id= atoi(argv[0]);
	if(NULL != (dbus_connection_dcli[slot_id] -> dcli_dbus_connection))
	{
	
		query = dbus_message_new_method_call(SFD_DBUS_BUSNAME, 
												SFD_DBUS_OBJPATH, 
												SFD_DBUS_INTERFACE, 
												SFD_DBUS_METHOD_ARPUSERADD);
		dbus_error_init(&err);
		dbus_message_append_args(query,
									DBUS_TYPE_STRING, &argv[1],
									DBUS_TYPE_INVALID);
		reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[slot_id] -> dcli_dbus_connection, query, 10000, &err);
		dbus_message_unref(query);
	
		if(NULL == reply) 
		{
			vty_out(vty, "dbus failed get reply.\n");
			if(dbus_error_is_set(&err))
			{
				vty_out(vty, "%s raised: %s", err.name, err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_WARNING;
		}
		
		if(dbus_message_get_args(reply,
									&err,									
									DBUS_TYPE_INT32, &sfdkern_is_exist,
									DBUS_TYPE_INT32, &op_ret,
									DBUS_TYPE_INVALID)) 
		{
			dbus_message_unref(reply);
			if(sfdkern_is_exist==0)
			{
				vty_out(vty,"service sfd is disable\n");
				return CMD_WARNING;
			}
			if(op_ret == 0) 
			{
				return CMD_SUCCESS;
			}
			else 
			{
				vty_out(vty, "commond executed failed...\n");
				return CMD_WARNING;
			}
		} 
		else 
		{	
			dbus_message_unref(reply);
			if (dbus_error_is_set(&err)) 
			{
				vty_out(vty, "%s raised: %s", err.name, err.message);
				dbus_error_free_for_dcli(&err);
			}
			
			return CMD_WARNING;
		}
	}
	else
	{
		vty_out(vty,"The slot doesn't exist");
		return CMD_WARNING;
	}
}
DEFUN (dcli_sfd_user_delarp,
		dcli_sfd_user_delarp_cmd,
		"service sfd user delarp MAC",
		"config system service\n"
		"config sfd service\n"
		"config user infomation for sfd service\n"
		"delete arp user infomation for sfd service\n"
		"MAC address of arp user\n")
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	int op_ret = -1;
	int sfdkern_is_exist;
	int i;
	int slot_count = 15;
	query = dbus_message_new_method_call(SFD_DBUS_BUSNAME, 
													SFD_DBUS_OBJPATH, 
													SFD_DBUS_INTERFACE, 
													SFD_DBUS_METHOD_ARPUSERDEL);
	dbus_error_init(&err);
	dbus_message_append_args(query,
								DBUS_TYPE_STRING, &argv[0],
								DBUS_TYPE_INVALID);
	for(i=1;i<=slot_count;i++)
	{
		if(NULL != (dbus_connection_dcli[i] -> dcli_dbus_connection))
		{
			reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[i] -> dcli_dbus_connection, query, 10000, &err);
			
			if(NULL == reply)
			{
				vty_out(vty, "dbus failed get reply.\n");
				if(dbus_error_is_set(&err))
				{
					vty_out(vty, "%s raised: %s", err.name, err.message);
					dbus_error_free_for_dcli(&err);
				}
				dbus_message_unref(query);
				return CMD_WARNING;
			}

			if(dbus_message_get_args(reply,
										&err,									
										DBUS_TYPE_INT32, &sfdkern_is_exist,
										DBUS_TYPE_INT32, &op_ret,
										DBUS_TYPE_INVALID)) 
			{
				dbus_message_unref(reply);
				if(sfdkern_is_exist==0)
				{
					vty_out(vty,"Slot %d service sfd is disable\n",i);
				}
				if(op_ret != 0) 
				{
					vty_out(vty, "Slot %d commond executed failed...\n",i);
				}
			} 
			else 
			{	
				dbus_message_unref(reply);
				if (dbus_error_is_set(&err)) 
				{
					vty_out(vty, "%s raised: %s", err.name, err.message);
					dbus_error_free_for_dcli(&err);
				}
				dbus_message_unref(query);
				return CMD_WARNING;
			}
		}
	}
	dbus_message_unref(query);
	return CMD_SUCCESS;
}
DEFUN (dcli_sfd_user_delarp_slot,
			dcli_sfd_user_delarp_slot_cmd,
			"service sfd user delarp <1-15> MAC",
			"config system service\n"
			"config sfd service\n"
			"config user infomation for sfd service\n"
			"delete arp user infomation for sfd service\n"
			"delete arp user infomation from slot <1-15> for sfd service\n"
			"MAC address of arp user\n")
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	int op_ret = -1;
	int slot_id;
	int sfdkern_is_exist;
	
	slot_id= atoi(argv[0]);
	if(NULL != (dbus_connection_dcli[slot_id] -> dcli_dbus_connection))
	{
	
		query = dbus_message_new_method_call(SFD_DBUS_BUSNAME, 
												SFD_DBUS_OBJPATH, 
												SFD_DBUS_INTERFACE, 
												SFD_DBUS_METHOD_ARPUSERADD);
		dbus_error_init(&err);
		dbus_message_append_args(query,
									DBUS_TYPE_STRING, &argv[1],
									DBUS_TYPE_INVALID);
		reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[slot_id] -> dcli_dbus_connection, query, 10000, &err);
		dbus_message_unref(query);
	
		if(NULL == reply) 
		{
			vty_out(vty, "dbus failed get reply.\n");
			if(dbus_error_is_set(&err)) 
			{
				vty_out(vty, "%s raised: %s", err.name, err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_WARNING;
		}
		
		if(dbus_message_get_args(reply,
									&err,									
									DBUS_TYPE_INT32, &sfdkern_is_exist,
									DBUS_TYPE_INT32, &op_ret,
									DBUS_TYPE_INVALID)) 
		{
			dbus_message_unref(reply);
			if(sfdkern_is_exist==0)
			{
				vty_out(vty,"service sfd is disable\n");
				return CMD_WARNING;
			}
			if(op_ret == 0) 
			{
				return CMD_SUCCESS;
			}
			else 
			{
				vty_out(vty, "commond executed failed...\n");
				return CMD_WARNING;
			}
		} 
		else 
		{	
			dbus_message_unref(reply);
			if (dbus_error_is_set(&err)) 
			{
				vty_out(vty, "%s raised: %s", err.name, err.message);
				dbus_error_free_for_dcli(&err);
			}
			
			return CMD_WARNING;
		}
	}
	else
	{
		vty_out(vty,"The slot doesn't exist");
		return CMD_WARNING;
	}
}

#if 0
DEFUN (dcli_sfd_show_var,
		dcli_sfd_show_var_cmd,
		"show sfd",
		"show system service\n"
		"show sfd service functions\n")
{
	DBusMessage *query=NULL; 
	DBusMessage *reply=NULL;
	DBusMessageIter  iter;
	DBusError err;
	char *ret = NULL;
	int varnum = 0;
	int cmd=1;
	int i,j;
	int sfdkern_is_exist;
	int slot_count = 15;
	query = dbus_message_new_method_call(SFD_DBUS_BUSNAME, 
												SFD_DBUS_OBJPATH, 
												SFD_DBUS_INTERFACE, 
												SFD_DBUS_METHOD_VAR);
	dbus_error_init(&err);
	dbus_message_append_args(query,
								DBUS_TYPE_INT32, &cmd,
								DBUS_TYPE_INVALID);
	
	for(i=1;i<=slot_count;i++)
	{
		if(NULL != (dbus_connection_dcli[i] -> dcli_dbus_connection))
		{
			reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[i] -> dcli_dbus_connection, query, 10000, &err);
			
			if(NULL == reply)
			{
				vty_out(vty, "dbus failed get reply.\n");
				dbus_message_unref(query);

				if(dbus_error_is_set(&err)) 
				{
					vty_out(vty, "%s raised: %s", err.name, err.message);
					dbus_error_free_for_dcli(&err);
				}
				dbus_message_unref(query);
				return CMD_WARNING;
			}

			dbus_message_iter_init(reply,&iter);
			dbus_message_iter_get_basic(&iter,&sfdkern_is_exist);
			if(sfdkern_is_exist==0)
			{
				vty_out(vty,"Slot %d service sfd is disable\n",i);
			}
			else if(sfdkern_is_exist==1)
			{
				vty_out(vty,"Slot %d :\n",i);
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&varnum);
				for(j=0;j < varnum;j++)
				{	
					dbus_message_iter_next(&iter);	
					dbus_message_iter_get_basic(&iter,&ret);
					vty_out(vty,"%s", ret);
					
				}	
			}
			dbus_message_unref(reply);
		}
	}
	dbus_message_unref(query);
	return CMD_SUCCESS;
}

DEFUN (dcli_sfd_show_var_slot,
		dcli_sfd_show_var_slot_cmd,
		"show sfd <1-15>",
		"show system service\n"
		"show sfd service functions\n"
		"show slot <1-15> sfd service functions\n")
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusMessageIter  iter;
	DBusError err = {0};
	char *ret = NULL;
	int op_ret = -1;
	int cmd=1;
	int slot_id;
	int i;
	int varnum = 0;
	int sfdkern_is_exist;
	slot_id= atoi(argv[0]);
	
	if(NULL != (dbus_connection_dcli[slot_id] -> dcli_dbus_connection))
	{
		query = dbus_message_new_method_call(SFD_DBUS_BUSNAME, 
												SFD_DBUS_OBJPATH, 
												SFD_DBUS_INTERFACE, 
												SFD_DBUS_METHOD_VAR);
		dbus_error_init(&err);
		dbus_message_append_args(query,
									DBUS_TYPE_INT32, &cmd,
									DBUS_TYPE_INVALID);
		reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[slot_id] -> dcli_dbus_connection, query, 10000, &err);
		dbus_message_unref(query);

		if(NULL == reply)
		{
			vty_out(vty, "dbus failed get reply.\n");
			if(dbus_error_is_set(&err)) 
			{
				vty_out(vty, "%s raised: %s", err.name, err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_WARNING;
		}

		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&sfdkern_is_exist);
		if(sfdkern_is_exist==0)
		{
			vty_out(vty,"service sfd is disable\n");
		}
		else if(sfdkern_is_exist==1)
		{
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&varnum);
			for(i=0;i < varnum;i++)
			{	
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&ret);
				vty_out(vty,"%s", ret);
				
			}	
		}
		
		dbus_message_unref(reply);
		return CMD_SUCCESS;
	}
	else
	{
		vty_out(vty,"The slot doesn't exist");
		return CMD_WARNING;
	}
}
#else
DEFUN (dcli_sfd_show_var,
		dcli_sfd_show_var_cmd,
		"show sfd ",
		"show system service\n"
		"show sfd service functions\n")
{
	DBusMessage *query=NULL; 
	DBusMessage *reply=NULL;
	DBusMessageIter  iter;
	DBusError err;
	
	int cmd=1;
	int i,j;
	int sfdkern_is_exist;
	int slot_count = 15;
	
	int sfd_enable = 0;
	int log_enable = 0;
	int debug_enable = 0;
	int tcp_enable = 0;
	int icmp_enable = 0;
	int snmp_enable = 0;
	int dns_enable = 0;
	int capwap_enable = 0;
	int arp_enable = 0;

	int timespan = 0;
	
	int limitpacket_snmp = 0;
	int limitpacket_tcp = 0;
	int limitpacket_icmp = 0;
	int limitpacket_arp = 0;
	int limitpacket_capwap =0;
	
	
	query = dbus_message_new_method_call(SFD_DBUS_BUSNAME, 
												SFD_DBUS_OBJPATH, 
												SFD_DBUS_INTERFACE, 
												SFD_DBUS_METHOD_VAR);
	dbus_error_init(&err);
	dbus_message_append_args(query,
								DBUS_TYPE_INT32, &cmd,
								DBUS_TYPE_INVALID);
	
	for(i=1;i<=slot_count;i++)
	{
		if(NULL != (dbus_connection_dcli[i] -> dcli_dbus_connection))
		{
			reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[i] -> dcli_dbus_connection, query, 10000, &err);
			
			if(NULL == reply)
			{
				vty_out(vty, "dbus failed get reply.\n");

				if(dbus_error_is_set(&err)) 
				{
					vty_out(vty, "%s raised: %s", err.name, err.message);
					dbus_error_free_for_dcli(&err);
				}
				dbus_message_unref(query);
				return CMD_WARNING;
			}

			dbus_message_iter_init(reply,&iter);
			dbus_message_iter_get_basic(&iter,&sfdkern_is_exist);
			if(sfdkern_is_exist==0)
			{
				vty_out(vty,"Slot %d service sfd is disable\n",i);
			}
			else if(sfdkern_is_exist==1)
			{
	
				vty_out(vty,"Slot %d :\n",i);
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&sfd_enable);
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&log_enable);
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&debug_enable);
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&timespan);
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&tcp_enable);
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&limitpacket_tcp);
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&icmp_enable);
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&limitpacket_icmp);
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&snmp_enable);
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&limitpacket_snmp);
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&arp_enable);
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&limitpacket_arp);
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&dns_enable);
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&capwap_enable);
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&limitpacket_capwap);
				if(sfd_enable)
					vty_out(vty,"service sfd is enable\n");
				else
					vty_out(vty,"service sfd is disable\n");
				
				if(log_enable)
					vty_out(vty,"service sfd log is enable\n");
				else
					vty_out(vty,"service sfd log is disable\n");
				
				if(debug_enable)
					vty_out(vty,"service sfd debug is enable\n");
				else
					vty_out(vty,"service sfd debug is disable\n");
				
				vty_out(vty,"service sfd timespan is %dms\n",timespan);
				
				if(tcp_enable)
					vty_out(vty,"service sfd tcp  is enable\n");
				else
					vty_out(vty,"service sfd tcp  is disable\n");
				
				if(icmp_enable)
					vty_out(vty,"service sfd icmp is enable  LIMITPACKET is %d\n",limitpacket_icmp);
				else
					vty_out(vty,"service sfd icmp is disable LIMITPACKET is %d\n",limitpacket_icmp);
				
				if(snmp_enable)
					vty_out(vty,"service sfd snmp is enable  LIMITPACKET is %d\n",limitpacket_snmp);
				else
					vty_out(vty,"service sfd snmp is disable LIMITPACKET is %d\n",limitpacket_snmp);

				if(arp_enable)
					vty_out(vty,"service sfd arp  is enable  LIMITPACKET is %d\n",limitpacket_arp);
				else
					vty_out(vty,"service sfd arp  is disable LIMITPACKET is %d\n",limitpacket_arp);
				if(capwap_enable)
					vty_out(vty,"service sfd capwap is enable  LIMITPACKET is %d\n",limitpacket_capwap);
				else
					vty_out(vty,"service sfd capwap is disable LIMITPACKET is %d\n",limitpacket_capwap);	
				if(dns_enable)
					vty_out(vty,"service sfd dns  is enable\n");
				else
					vty_out(vty,"service sfd dns  is disable\n");
		
			}
			dbus_message_unref(reply);
		}
	}
	dbus_message_unref(query);
	return CMD_SUCCESS;
}

DEFUN (dcli_sfd_show_var_slot,
		dcli_sfd_show_var_slot_cmd,
		"show sfd <1-15>",
		"show system service\n"
		"show sfd service functions\n"
		"show slot <1-15> sfd service functions\n")
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusMessageIter  iter;
	DBusError err = {0};
	int op_ret = -1;
	int cmd=1;
	int slot_id;
	int sfdkern_is_exist;
	int sfd_enable = 0;
	int log_enable = 0;
	int debug_enable = 0;
	int tcp_enable = 0;
	int icmp_enable = 0;
	int snmp_enable = 0;
	int dns_enable = 0;
	int capwap_enable = 0;
	int arp_enable = 0;
	int timespan = 0;
	
	int limitpacket_snmp = 0;
	int limitpacket_tcp = 0;
	int limitpacket_icmp = 0;
	int limitpacket_arp = 0;
	int limitpacket_capwap = 0;
	
	slot_id= atoi(argv[0]);
	
	if(NULL != (dbus_connection_dcli[slot_id] -> dcli_dbus_connection))
	{
		query = dbus_message_new_method_call(SFD_DBUS_BUSNAME, 
												SFD_DBUS_OBJPATH, 
												SFD_DBUS_INTERFACE, 
												SFD_DBUS_METHOD_VAR);
		dbus_error_init(&err);
		dbus_message_append_args(query,
									DBUS_TYPE_INT32, &cmd,
									DBUS_TYPE_INVALID);
		reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[slot_id] -> dcli_dbus_connection, query, 10000, &err);
		dbus_message_unref(query);

		if(NULL == reply)
		{
			vty_out(vty, "dbus failed get reply.\n");
			if(dbus_error_is_set(&err)) 
			{
				vty_out(vty, "%s raised: %s", err.name, err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_WARNING;
		}

		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&sfdkern_is_exist);
		if(sfdkern_is_exist==0)
		{
			vty_out(vty,"service sfd is disable\n");
		}
		else if(sfdkern_is_exist==1)
		{
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&sfd_enable);
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&log_enable);
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&debug_enable);
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&timespan);
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&tcp_enable);
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&limitpacket_tcp);
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&icmp_enable);
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&limitpacket_icmp);
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&snmp_enable);
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&limitpacket_snmp);
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&arp_enable);
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&limitpacket_arp);
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&dns_enable);
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&capwap_enable);
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&limitpacket_capwap);
				if(sfd_enable)
					vty_out(vty,"service sfd is enable\n");
				else
					vty_out(vty,"service sfd is disable\n");
				
				if(log_enable)
					vty_out(vty,"service sfd log is enable\n");
				else
					vty_out(vty,"service sfd log is disable\n");
				
				if(debug_enable)
					vty_out(vty,"service sfd debug is enable\n");
				else
					vty_out(vty,"service sfd debug is disable\n");
				
				vty_out(vty,"service sfd timespan is %dms\n",timespan);
				
				if(tcp_enable)
					vty_out(vty,"service sfd tcp  is enable  LIMITPACKET is %d\n",limitpacket_tcp);
				else
					vty_out(vty,"service sfd tcp  is disable LIMITPACKET is %d\n",limitpacket_tcp);
				
				if(icmp_enable)
					vty_out(vty,"service sfd icmp is enable  LIMITPACKET is %d\n",limitpacket_icmp);
				else
					vty_out(vty,"service sfd icmp is disable LIMITPACKET is %d\n",limitpacket_icmp);
				
				if(snmp_enable)
					vty_out(vty,"service sfd snmp is enable  LIMITPACKET is %d\n",limitpacket_snmp);
				else
					vty_out(vty,"service sfd snmp is disable LIMITPACKET is %d\n",limitpacket_snmp);

				if(arp_enable)
					vty_out(vty,"service sfd arp  is enable  LIMITPACKET is %d\n",limitpacket_arp);
				else
					vty_out(vty,"service sfd arp  is disable LIMITPACKET is %d\n",limitpacket_arp);
				if(capwap_enable)
					vty_out(vty,"service sfd capwap is enable  LIMITPACKET is %d\n",limitpacket_capwap);
				else
					vty_out(vty,"service sfd capwap is disable LIMITPACKET is %d\n",limitpacket_capwap);				
				
				if(dns_enable)
					vty_out(vty,"service sfd dns  is enable\n");
				else
					vty_out(vty,"service sfd dns  is disable\n");
		
			}
		
		dbus_message_unref(reply);
		return CMD_SUCCESS;
	}
	else
	{
		vty_out(vty,"The slot doesn't exist");
		return CMD_WARNING;
	}
}
#endif

int dcli_sfd_running(struct vty *vty)	
{
	DBusMessage *query=NULL; 
	DBusMessage *reply=NULL;
	DBusMessageIter  iter;
	DBusError err;
	char *ret = NULL;
	char _tmpstr[64] = {0};
	char buf[64]= {0};
	
	int cmd = 1;
	int i;
	int slot_count = 15;
	
	int sfdkern_is_exist;
	int sfd_enable = 0;
	int log_enable = 0;
	int debug_enable = 0;
	int tcp_enable = 0;
	int icmp_enable = 0;
	int snmp_enable = 0;
	int dns_enable = 0;
	int capwap_enable = 0;
	int arp_enable = 0;
	int arpsuppress_enable = 0;
	int arplimit_enable = 0;

	int timespan = 0;
	
	int limitpacket_snmp = 0;
	int limitpacket_tcp = 0;
	int limitpacket_icmp = 0;
	int limitpacket_arp = 0;
	int limitpacket_capwap = 0;
	
	sprintf(_tmpstr,BUILDING_MOUDLE,"SFD CONFIG");
	vtysh_add_show_string(_tmpstr);
	
	query = dbus_message_new_method_call(SFD_DBUS_BUSNAME, 
												SFD_DBUS_OBJPATH, 
												SFD_DBUS_INTERFACE, 
												SFD_DBUS_METHOD_VAR);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
									DBUS_TYPE_INT32, &cmd,
									DBUS_TYPE_INVALID);
	for(i=1;i<=slot_count;i++)
	{
		if(NULL != (dbus_connection_dcli[i] -> dcli_dbus_connection))
		{
			reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[i] -> dcli_dbus_connection, query, 10000, &err);
			
			if(NULL == reply)
			{
				vty_out(vty, "dbus failed get reply.\n");
				dbus_message_unref(query);

				if(dbus_error_is_set(&err)) 
				{
					vty_out(vty, "%s raised: %s", err.name, err.message);
					dbus_error_free_for_dcli(&err);
				}
		
				return CMD_WARNING;
			}

			dbus_message_iter_init(reply,&iter);
			dbus_message_iter_get_basic(&iter,&sfdkern_is_exist);
			if(sfdkern_is_exist==0)
			{
				//vty_out(vty,"service sfd %d disable\n",i);
			}
			else if(sfdkern_is_exist==1)
			{
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&sfd_enable);
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&log_enable);
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&debug_enable);
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&timespan);
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&tcp_enable);
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&limitpacket_tcp);
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&icmp_enable);
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&limitpacket_icmp);
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&snmp_enable);
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&limitpacket_snmp);
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&arp_enable);
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&limitpacket_arp);
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&dns_enable);
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&capwap_enable);
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&limitpacket_capwap);
				if(sfd_enable)
				{
					sprintf(buf,"service sfd %d enable\n",i);
					vtysh_add_show_string(buf);
				}
				if(log_enable)
				{
					sprintf(buf,"service sfd log %d enable\n",i);
					vtysh_add_show_string(buf);
				}
				
				if(debug_enable)
				{
					sprintf(buf,"service sfd debug %d enable\n",i);
					vtysh_add_show_string(buf);
				}
			
				if(timespan != TIMESPAN_DEF)
				{
					sprintf(buf,"service sfd %d timespan %d\n",i,timespan);
					vtysh_add_show_string(buf);
				}
				
				if(tcp_enable)
				{
					sprintf(buf,"service sfdtcp %d enable\n",i);
					vtysh_add_show_string(buf);
				}
			
				if(icmp_enable)
				{
					sprintf(buf,"service sfdicmp %d enable\n",i);
					vtysh_add_show_string(buf);
				}
				
				if(snmp_enable)
				{
					sprintf(buf,"service sfdsnmp %d enable\n",i);
					vtysh_add_show_string(buf);
				}

				if(arp_enable)
				{
					sprintf(buf,"service sfdarp %d enable\n",i);
					vtysh_add_show_string(buf);
				}		
				
				if(dns_enable)
				{
					sprintf(buf,"service sfddns %d enable\n",i);
					vtysh_add_show_string(buf);
				}

				if(capwap_enable)
				{
					sprintf(buf,"service sfdcapwap %d enable\n",i);
					vtysh_add_show_string(buf);
				}
				if(limitpacket_tcp != TCP_PACKET_DEF)
				{
					sprintf(buf,"service sfd packetlimit %d tcp %d\n",i,limitpacket_tcp);
					vtysh_add_show_string(buf);
				}
					
				if(limitpacket_icmp != ICMP_PACKET_DEF)
				{
					sprintf(buf,"service sfd packetlimit %d icmp %d\n",i,limitpacket_icmp);
					vtysh_add_show_string(buf);
				}
					
				if(limitpacket_snmp != SNMP_PACKET_DEF)
				{
					sprintf(buf,"service sfd packetlimit %d snmp %d\n",i,limitpacket_snmp);
					vtysh_add_show_string(buf);
				}
					
				if(limitpacket_arp != ARP_PACKET_DEF)
				{
					sprintf(buf,"service sfd packetlimit %d arp %d\n",i,limitpacket_arp);
					vtysh_add_show_string(buf);
				}
				if(limitpacket_capwap!= CAPWAP_PACKET_DEF)
				{
					sprintf(buf,"service sfd packetlimit %d capwap %d\n",i,limitpacket_capwap);
					vtysh_add_show_string(buf);
				}
			}
			dbus_message_unref(reply);
		}
	}
	dbus_message_unref(query);
	return CMD_SUCCESS;	
}
/* end - sfd by zhengbo */

/**hxx added for dns service**/

int get_dns_str(char **name)
{
	FILE *fp=NULL;
	char *ptr=NULL;
	int i=0;

	fp = fopen("/etc/resolv.conf","r");
	
	if(!fp)
		return -1;
	ptr=malloc(128);
	if(!ptr)
		{
		fclose(fp);
		return -1;
	}
	while(fgets(ptr,128,fp))
	{
		if(!strncmp(ptr,"nameserver ",11))
		{
			sprintf(*(name+i),ptr+11);
			i++;			
			if (i>=3)
				break;
		}
	}
	free(ptr);
	fclose(fp);
	return i;
	
}
int set_dns_str(const char **name,int num)
{
	FILE * fp=NULL;
	char *ptr=NULL;
	int i=0;

	fp = fopen("/etc/resolv.conf","w");
	if(!fp)
		return -1;

	ptr=malloc(128);
	if(!ptr)
	{
		fclose(fp);
		return -1;
	}
	for(i=0;i<num;i++)
	{
			sprintf(ptr,"nameserver %s\n",*(name+i));
			fputs(ptr,fp);
	}
	
	free(ptr);
	fclose(fp);
	return 0;

}

#define PATH_LEN 64


extern char BSD_DBUS_BUSNAME[PATH_LEN];
extern char BSD_DBUS_OBJPATH[PATH_LEN];
extern char BSD_DBUS_INTERFACE[PATH_LEN];

extern char BSD_COPY_FILES_BETEWEEN_BORADS[PATH_LEN];

int
pfm_dns_show_running_config(char *tmp_str, int opt)
{
	FILE *fp = NULL,*fpbk=NULL;
	int ret, status;
	char setup_str[256];
	char buf[512]={0};
	int len = 0;
	
	set_file_attr(PFM_DNS_FILE);
	set_file_attr(PFM_DNS_FILE_BK);
	
	memset(setup_str,0 ,256);
	
	fp = fopen(PFM_DNS_FILE,"r");
	fpbk = fopen(PFM_DNS_FILE_BK,"w");
		
	strcpy(setup_str,"service dns ");/**add cmd head**/
	strcat(setup_str,tmp_str);/**OK ,unit a whole cmd string save in setup_str**/
	strcat(setup_str," enable\n");

	if(NULL == fp)
	{		
		fp = fopen(PFM_DNS_FILE,"w");
		if(fp==NULL)
		{
			if(fpbk)
				fclose(fpbk);
			return 1;
		}
		fputs(setup_str,fp);
		fclose(fp);
		
		if(fpbk)
			fclose(fpbk);
		unlink(PFM_DNS_FILE_BK);
	}
	else if(NULL == fpbk) 
	{
		if(fp)
			fclose(fp);

		return 1;

	}
	else 		
	{

		if(opt == 0)/**enable : go to write setup in file**/
		{
			while(fgets(buf,512,fp))
			{				
				if(0 == strcmp(buf,setup_str))
					continue;
				else
					fprintf(fpbk,buf);			
			}
			
			fprintf(fpbk,"%s",setup_str);

			fclose(fp);
			fclose(fpbk);

			unlink (PFM_DNS_FILE);
			rename ( PFM_DNS_FILE_BK,PFM_DNS_FILE);
		}
		else /**disable : detele the enable(or say rm the enable string)**/
		{
			while(fgets(buf,512,fp))
			{
				if(0 == strcmp(buf,setup_str))
					continue;
				else
					fprintf(fpbk,buf);
			}
			

			fclose(fp);
			fclose(fpbk);

			unlink (PFM_DNS_FILE);
			rename ( PFM_DNS_FILE_BK,PFM_DNS_FILE);
		}
	}
	
	return 0;
}
int
pfm_radius_show_running_config(char *port,char *ip,char *ifname, int opt)
{
	FILE *fp = NULL,*fpbk=NULL;
	int ret, status;
	char setup_str[256];
	char buf[512]={0};
	int len = 0;
	if(!port || !ip || !ifname)
		return 1;
	
	set_file_attr(PFM_RADIUS_FILE);
	set_file_attr(PFM_RADIUS_FILE_BK);
	
	memset(setup_str,0 ,256);
	
	fp = fopen(PFM_RADIUS_FILE,"r");
	fpbk = fopen(PFM_RADIUS_FILE_BK,"w");
		
/*	strcpy(setup_str,"service dns ");
	strcat(setup_str,tmp_str);
	strcat(setup_str," enable\n");*/
	sprintf(setup_str,"service radius %s %s %s enable\n",port,ip,ifname);

	if(NULL == fp)
	{		
		fp = fopen(PFM_RADIUS_FILE,"w");
		if(fp==NULL)
		{
			if(fpbk)
				fclose(fpbk);
			return 1;
		}
		fputs(setup_str,fp);
		fclose(fp);
		
		if(fpbk)
			fclose(fpbk);
		unlink(PFM_RADIUS_FILE_BK);
	}
	else if(NULL == fpbk) 
	{
		if(fp)
			fclose(fp);

		return 1;

	}
	else 		
	{

		if(opt == 0)/**enable : go to write setup in file**/
		{
			while(fgets(buf,512,fp))
			{				
				if(0 == strcmp(buf,setup_str))
					continue;
				else
					fprintf(fpbk,buf);			
			}
			
			fprintf(fpbk,"%s",setup_str);

			fclose(fp);
			fclose(fpbk);

			unlink (PFM_RADIUS_FILE);
			rename ( PFM_RADIUS_FILE_BK,PFM_RADIUS_FILE);
		}
		else /**disable : detele the enable(or say rm the enable string)**/
		{
			while(fgets(buf,512,fp))
			{
				if(0 == strcmp(buf,setup_str))
					continue;
				else
					fprintf(fpbk,buf);
			}
			

			fclose(fp);
			fclose(fpbk);

			unlink (PFM_RADIUS_FILE);
			rename ( PFM_RADIUS_FILE_BK,PFM_RADIUS_FILE);
		}
	}
	
	return 0;
}


void sync_hostname()
{
	int i;
	DBusMessage *query = NULL;		
	DBusMessage *reply = NULL;	
	DBusError err = {0};
	unsigned int result = 0;
			
	unsigned int src_slotid = 0;
	unsigned int des_slotid = 0;
	unsigned int product_serial = 0;		
	char file_path_temp[64] = "/proc/sys/kernel/hostname";
	char *src_path_temp = file_path_temp;
	char *des_path_temp = file_path_temp;
	int fd;		
	int tar_switch = 0;
	int op = BSD_TYPE_CORE;
	int length = 0;
	char dns[128];
	int count = 0;
	int ID[MAX_SLOT_NUM] = {0};

	
	fd = fopen("/dbm/product/product_serial", "r");		
	fscanf(fd, "%d", &product_serial);
	fclose(fd);

	fd = fopen("/dbm/product/master_slot_id", "r");
	fscanf(fd, "%d", &src_slotid);
	fclose(fd);
#if 0
	if(product_serial == 8)
	{
		src_slotid = 5;
	}
	else if(product_serial == 7)
	{			
		src_slotid = 1;
	}
#endif	
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,src_slotid,distributFag);
	
	count = dcli_bsd_get_slot_ids(dcli_dbus_connection,ID,op);
    
	if(count != 0)
	{
		for(i = 0; i < count; i++)
		{
    		des_slotid = ID[i];
			
		    query = dbus_message_new_method_call(BSD_DBUS_BUSNAME,BSD_DBUS_OBJPATH,\
								BSD_DBUS_INTERFACE,BSD_COPY_FILES_BETEWEEN_BORADS);
			dbus_error_init(&err);
			dbus_message_append_args(query,
									 DBUS_TYPE_UINT32,&des_slotid,
									 DBUS_TYPE_STRING,&src_path_temp,
									 DBUS_TYPE_STRING,&des_path_temp,
									 DBUS_TYPE_UINT32,&tar_switch,
									 DBUS_TYPE_UINT32,&op,
									 DBUS_TYPE_INVALID);
		
			reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
			dbus_message_unref(query);
			if (NULL == reply) {
				vty_out(vty,"<error> failed get reply.\n");
				if (dbus_error_is_set(&err)) {
					vty_out(vty,"%s raised: %s",err.name,err.message);
					dbus_error_free_for_dcli(&err);
				}
				return CMD_SUCCESS;
			}
			dbus_message_unref(reply);
		}
	}
}


/*gujd:2013-03-01, am 10:27. Add set hostname independent between every board.*/  
void sync_hostname_v2(int slot_ID)
{
	int i;
	DBusMessage *query = NULL;		
	DBusMessage *reply = NULL;	
	DBusError err = {0};
	unsigned int result = 0;
	unsigned int src_slotid = 0;
	unsigned int des_slotid = 0;
	char filename[128] = {0};
	
	char *src_path_temp = NULL;
	char *des_path_temp = "/proc/sys/kernel/hostname";
	int fd;		
	int tar_switch = 0;
	int op = BSD_TYPE_CORE;
	int length = 0;
	char dns[128];
	int count = 0;
	int ID[MAX_SLOT_NUM] = {0};
	
	/*the file is "/dbm/product/slot/slotX/host_name"*/
	sprintf(filename,"%sslot%d%s",PRODUCT_SLOT_DIR,slot_ID,HOSTNAME_FILE);
	/*vty_out(vty,"%s: line %d ..filename[%s]..\n",__func__,__LINE__,filename);*/
	src_path_temp = filename;
	
	fd = fopen("/dbm/product/master_slot_id", "r");
	fscanf(fd, "%d", &src_slotid);
	fclose(fd);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,src_slotid,distributFag);
	
	count = dcli_bsd_get_slot_ids(dcli_dbus_connection,ID,op);
	if(count != 0)
	{
		/*slot_ID = slot_ID - 1;*/
		for(i = 0; i < count; i++)
		{
    		des_slotid = ID[i];
			if(des_slotid == slot_ID)
			{
			    query = dbus_message_new_method_call(BSD_DBUS_BUSNAME,BSD_DBUS_OBJPATH,\
									BSD_DBUS_INTERFACE,BSD_COPY_FILES_BETEWEEN_BORADS);
				dbus_error_init(&err);
				dbus_message_append_args(query,
										 DBUS_TYPE_UINT32,&des_slotid,
										 DBUS_TYPE_STRING,&src_path_temp,
										 DBUS_TYPE_STRING,&des_path_temp,
										 DBUS_TYPE_UINT32,&tar_switch,
										 DBUS_TYPE_UINT32,&op,
										 DBUS_TYPE_INVALID);
			
				reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
				dbus_message_unref(query);
				if (NULL == reply) {
					vty_out(vty,"<error> failed get reply.\n");
					if (dbus_error_is_set(&err)) {
						vty_out(vty,"%s raised: %s",err.name,err.message);
						dbus_error_free_for_dcli(&err);
					}
					return CMD_SUCCESS;
				}
				dbus_message_unref(reply);
			}
			
		}
	}
}


DEFUN (set_ip_dns_func,
			set_ip_dns_func_cmd,
			"ip dns A.B.C.D",
			"Interface Internet Protocol config commands\n"
			"Set ip dns server\n"
			"Dns ip address\n")
{
	char *dnsstr[3];
	int ret,i;

	for(i=0;i<3;i++)
	{
		dnsstr[i] = malloc(128);
		if(!dnsstr[i])
			goto ret_err;
		memset(dnsstr[i],0,128);
	}	

	ret = get_dns_str(&dnsstr);
	if(ret<0)
	{
		vty_out(vty,"Can't get system dns seting\n");
		goto ret_err;
	}
	else if(ret >= 3)
	{
		vty_out(vty,"The system has 3 dns,can't set again\n");
		goto ret_err;
	}
	else
	{
		int i;
		DBusMessage *query = NULL;		
		DBusMessage *reply = NULL;	
		DBusError err = {0};
//		DBusConnection *dcli_dbus_connection = NULL;
		unsigned int result = 0;
				
		unsigned int src_slotid = 0;
		unsigned int des_slotid = 0;
		unsigned int product_serial = 0;		
		char file_path_temp[64] = "/etc/resolv.conf";
		char *src_path_temp = file_path_temp;
		char *des_path_temp = file_path_temp;
		int fd;		
		int tar_switch = 0;
		int op = BSD_TYPE_NORMAL;
		int length = 0;
		char dns[128];
		int ID[MAX_SLOT_NUM] = {0};
		int count = 0;

		
		fd = fopen("/dbm/product/master_slot_id", "r");
		fscanf(fd, "%d", &src_slotid);
		fclose(fd);

/*		
		fd = fopen("/dbm/product/product_serial", "r");		
		fscanf(fd, "%d", &product_serial);
		fclose(fd);

		if(product_serial == 8)
		{
			src_slotid = 5;
			des_slotid = 6;
		}
		else if(product_serial == 7)
		{			
			src_slotid = 1;
			des_slotid = 2;
		}
*/		
		count = dcli_bsd_get_slot_ids(dcli_dbus_connection,ID,1);

		for(i=0;i<ret;i++)
		{
			memset(dns,0,128);
			length = strlen(dnsstr[i]);
			strncpy(dns,dnsstr[i],length-1);
			if(!strcmp(argv[0],dns))
				break;
		}
		if(i==ret)
			sprintf(dnsstr[ret],(char*)argv[0]);
		else
		{
			vty_out(vty,"The dns server %s is exist,can't set again\n",argv[0]);
			goto ret_err;
		}		
		if(set_dns_str(&dnsstr,ret+1))
		{
			vty_out(vty,"Set system dns error\n");
			goto ret_err;

		}
		if(count != 0)
		{
			for(i = 0; i < count; i++)
			{
				des_slotid = ID[i];
				ReInitDbusConnection(&dcli_dbus_connection,src_slotid,distributFag);
			    
			    query = dbus_message_new_method_call(BSD_DBUS_BUSNAME,BSD_DBUS_OBJPATH,\
									BSD_DBUS_INTERFACE,BSD_COPY_FILES_BETEWEEN_BORADS);
				dbus_error_init(&err);
				dbus_message_append_args(query,
										 DBUS_TYPE_UINT32,&des_slotid,
										 DBUS_TYPE_STRING,&src_path_temp,
										 DBUS_TYPE_STRING,&des_path_temp,
										 DBUS_TYPE_UINT32,&tar_switch,
										 DBUS_TYPE_UINT32,&op,
										 DBUS_TYPE_INVALID);
			
				reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,60000, &err);
				dbus_message_unref(query);
				if (NULL == reply) {
					vty_out(vty,"<error> failed get reply.\n");
					if (dbus_error_is_set(&err)) {
						vty_out(vty,"%s raised: %s",err.name,err.message);
						dbus_error_free_for_dcli(&err);
					}
					return CMD_SUCCESS;
				}
				dbus_message_unref(reply);
			}
		}
	}
	for(i=0;i<3;i++)
	{
		if(dnsstr[i])
			free(dnsstr[i]);
	}	
#if 0	
	{
		int ret;
		int i;
		for(i = 0;i < MAX_SLOT ; i++)
		{
			if((NULL != (dbus_connection_dcli[i] -> dcli_dbus_connection)) && i != HostSlotId)
			{
				ret = dcli_communicate_pfm_by_dbus(0, 0, 17, "all", 53,0, HostSlotId,
													"all", "all",i);/*use dbus send message to pfm*/
				if(ret != 0)
				{
					vty_out(vty,"warning:pfm send error\n");
					return CMD_WARNING;
				}
			}
		}
	}
#endif
	return CMD_SUCCESS;

ret_err:
	
	for(i=0;i<3;i++)
	{
		if(dnsstr[i])
			free(dnsstr[i]);
	}	
	return CMD_WARNING;

}
DEFUN (delete_ip_dns_func,
       delete_ip_dns_func_cmd,
       "no ip dns A.B.C.D",
	   NO_STR
	   "Interface Internet Protocol config commands\n"
       "Set ip dns server\n"
       "Dns ip address\n")
{
	char *dnsstr[3];
	int ret,i;

	for(i=0;i<3;i++)
	{
		dnsstr[i] = malloc(128);
		if(!dnsstr[i])
			goto ret_err;
		memset(dnsstr[i],0,128);
	}	

	
	ret = get_dns_str(&dnsstr);

	if(ret<0 || ret > 3)
	{
		vty_out(vty,"Can't get system dns seting\n");
		goto ret_err;
	}
	else
	{
		int i=0;
		
		DBusMessage *query = NULL;		
		DBusMessage *reply = NULL;	
		DBusError err = {0};
//		DBusConnection *dcli_dbus_connection = NULL;
		unsigned int result = 0;
				
		unsigned int src_slotid = 0;
		unsigned int des_slotid = 0;
		unsigned int product_serial = 0;		
		char file_path_temp[64] = "/etc/resolv.conf";
		char *src_path_temp = file_path_temp;
		char *des_path_temp = file_path_temp;
		int fd;		
		int tar_switch = 0;
		int op = BSD_TYPE_NORMAL;
		int ID[MAX_SLOT_NUM] = {0};
		int count = 0;

/*		
		fd = fopen("/dbm/product/product_serial", "r");		
		fscanf(fd, "%d", &product_serial);
		fclose(fd);

		if(product_serial == 8)
		{
			src_slotid = 5;
			des_slotid = 6;
		}
		else if(product_serial == 7)
		{			
			src_slotid = 1;
			des_slotid = 2;
		}
*/
		fd = fopen("/dbm/product/master_slot_id", "r");
			fscanf(fd, "%d", &src_slotid);
			fclose(fd);

		count = dcli_bsd_get_slot_ids(dcli_dbus_connection,ID,1);

		
		for(i=0;i<ret;i++)
		{
			if(!strncmp(argv[0],dnsstr[i],strlen(argv[0])))
			{
				int j ;
				if(i<ret-1)
				{	
					for(j=i;j<ret-1;j++)
					{
						sprintf(dnsstr[j],dnsstr[j+1]);
					}
				}
				break;
			}
		}
		if(i >= ret)
		{
			vty_out(vty,"Can't get the dns %s\n",argv[0]);
			goto ret_err;

		}
		if(set_dns_str(&dnsstr,ret-1))
		{
			vty_out(vty,"Delete system dns error\n");
			goto ret_err;

		}
		
		if(count != 0)
		{
			for(i = 0; i < count; i++)
			{
				des_slotid = ID[i];
				ReInitDbusConnection(&dcli_dbus_connection,src_slotid,distributFag);
			    query = dbus_message_new_method_call(BSD_DBUS_BUSNAME,BSD_DBUS_OBJPATH,\
									BSD_DBUS_INTERFACE,BSD_COPY_FILES_BETEWEEN_BORADS);
				dbus_error_init(&err);
				dbus_message_append_args(query,
										 DBUS_TYPE_UINT32,&des_slotid,
										 DBUS_TYPE_STRING,&src_path_temp,
										 DBUS_TYPE_STRING,&des_path_temp,
										 DBUS_TYPE_UINT32,&tar_switch,
										 DBUS_TYPE_UINT32,&op,
										 DBUS_TYPE_INVALID);
				reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,60000, &err);
				dbus_message_unref(query);
				if (NULL == reply) {
					vty_out(vty,"<error> failed get reply.\n");
					if (dbus_error_is_set(&err)) {
						vty_out(vty,"%s raised: %s",err.name,err.message);
						dbus_error_free_for_dcli(&err);
					}
					return CMD_SUCCESS;
				}
				dbus_message_unref(reply);
			}
		}
		
	}

	return CMD_SUCCESS;
ret_err:
	
	for(i=0;i<3;i++)
	{
		if(dnsstr[i])
			free(dnsstr[i]);
	}	
	return CMD_WARNING;

	
}
DEFUN (show_ip_dns_func,
       show_ip_dns_func_cmd,
       "show ip dns",
	   SHOW_STR
	   "Interface Internet Protocol config commands\n"
       "ip dns server\n")
{
	char *dnsstr[3];
	int ret,i;

	for(i=0;i<3;i++)
	{
		dnsstr[i] = malloc(128);
		if(!dnsstr[i])
			goto ret_err;
		memset(dnsstr[i],0,128);
	}	
	ret = get_dns_str(&dnsstr);
	if(ret<0)
	{
		vty_out(vty,"Can't get system dns seting\n");
		goto ret_err;
	}
	else if(ret > 3)
	{
		vty_out(vty,"Get system dns seting error\n");
		goto ret_err;
	}
	else
	{
		if(ret == 0)
				vty_out(vty,"DNS server is not set\n");
		else
		{
			vty_out(vty,"DNS server is:\n");
			for(i=0;i<ret;i++)
			{			
				vty_out(vty,"%s",(dnsstr[i]));
			}
		}
		
		for(i=0;i<3;i++)
		{
			if(dnsstr[i])
				free(dnsstr[i]);
		}	
		return CMD_SUCCESS;
	}
ret_err:
	
	for(i=0;i<3;i++)
	{
		if(dnsstr[i])
			free(dnsstr[i]);
	}	
	return CMD_WARNING;

}
#if 1
DEFUN (show_all_slot_sys_info_func,
			show_sll_slot_sys_info_func_cmd,
			"show all info",
			SHOW_STR
			"Set ip dns server\n"
			"Dns ip address\n"
			"make service enable\n "
			"make service disable\n ")
{
	DBusMessage *query=NULL, *reply=NULL;
	DBusError err;
	int i;
	char *ret;


	query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
												 SEM_DBUS_INTERFACE, SEM_DBUS_SHOW_ALL_SLOT_SYS_INFO);
	
	dbus_error_init(&err);
	
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &i,
							DBUS_TYPE_INVALID);
	for(i = 0;i < MAX_SLOT ; i++)
	{
		if(NULL != (dbus_connection_dcli[i] -> dcli_dbus_connection))
		{

			reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[i] -> dcli_dbus_connection, query, -1, &err);
			

			if (NULL == reply){
				vty_out(vty,"<error> failed get reply.\n");
				if (dbus_error_is_set(&err)) {
					vty_out(vty,"%s raised: %s",err.name,err.message);
					dbus_error_free_for_dcli(&err);
				}
				return CMD_WARNING;
			}

			if (dbus_message_get_args (reply, &err,
							DBUS_TYPE_STRING, &ret,
							DBUS_TYPE_INVALID)) {

					vty_out(vty,"%s\n", ret);
				
			}
		}
	}
	if(query)
	dbus_message_unref(query);
	if(reply)
	dbus_message_unref(reply);

	return CMD_SUCCESS;
}
#endif

DEFUN (service_ip_dns_func,
			service_ip_dns_func_cmd,
			"service dns A.B.C.D (enable|disable)",
			"service for ip dns\n"
			"Set ip dns server\n"
			"Dns ip address\n"
			"make service enable\n "
			"make service disable\n ")
{
	
	int ret;
	int i;
	int opt;
	char *cmd_str = NULL;
	char temp_str[128];
	memset(temp_str,0,128);

	if(strncmp("enable",argv[1],strlen(argv[1]))== 0)
		opt = 0;
	else
		opt = 1;

	if(0 == opt)
	{
		ret = dcli_communicate_pfm_by_dbus(4, 0, 0, "all", 0, 0, 0, "all", "all",-1);/**use dbus send message to pfm**/
		if(ret != 0)
		{
			vty_out(vty,"dbus send message error_enable\n");
			return CMD_WARNING;
		}
	}
	
	for(i = 0;i < MAX_SLOT ; i++)
	{
		if((NULL != (dbus_connection_dcli[i] -> dcli_dbus_connection)) && i != HostSlotId)
		{
			ret = dcli_communicate_pfm_by_dbus(opt, 0, 17, "all", 53,0, HostSlotId,
												"all", (char *)argv[0],i);/*use dbus send message to pfm*/
			if(ret != 0)
			{
				vty_out(vty,"warning:pfm send error\n");
				return CMD_WARNING;
			}
		}
	}
	strcpy(temp_str,argv[0]);

	ret = pfm_dns_show_running_config(temp_str, opt);
	if(ret != 0)
		return CMD_WARNING;

	return CMD_SUCCESS;
}

DEFUN (service_radius_func,
			service_radius_func_cmd,
			"service radius SRC_PORT A.B.C.D IFNAME (enable|disable)",
			"service for radius forward\n"
			"Radius service\n"
			"Port of radius\n"			
			"Radius server address\n"
			"ifname\n"
			"make service enable\n "
			"make service disable\n ")
{
	
	int ret;
	int i;
	int opt;
	int fd;
	int src_port;
	unsigned int master_slotid = 0;
	int command_send_to;
	char *cmd_str = NULL;
	char temp_str[128];
	memset(temp_str,0,128);
	src_port = atoi(argv[0]);
	if(src_port < 0 || src_port > 65535)
	 {
	   vty_out(vty,"Input src_port err.<0--65535> !\n");
	   return CMD_WARNING;
	 }
	command_send_to = get_slot_num_dcli(argv[2]);

	if(strncmp("enable",argv[3],strlen(argv[3]))== 0)
		opt = 0;
	else
		opt = 1;

	if(0 == opt)
	{
		ret = dcli_communicate_pfm_by_dbus(4, 0, 0, "all", 0, 0, 0, "all", "all",-1);/**use dbus send message to pfm**/
		if(ret != 0)
		{
			vty_out(vty,"dbus send message error_enable\n");
			return CMD_WARNING;
		}
	}
	
	fd = fopen("/dbm/product/master_slot_id", "r");
	fscanf(fd, "%d", &master_slotid);
	fclose(fd);

	
	ret = dcli_communicate_pfm_by_dbus(opt, 0, 17, (char*)argv[2],src_port,0, master_slotid,
										(char *)argv[1], "all",command_send_to);/*use dbus send message to pfm*/
	if(ret != 0)
	{
		vty_out(vty,"warning:pfm send error\n");
		return CMD_WARNING;
	}


	ret = pfm_radius_show_running_config(argv[0],argv[1],argv[2], opt);
	if(ret != 0)
		return CMD_WARNING;

	return CMD_SUCCESS;
}


/*add by zhaocg 2012-10-11 pm 15:30*/

/*Show slabinfo content*/
DEFUN (show_memory_kernel,
		show_memory_kernel_cmd,
		"show memory kernel",
		SHOW_STR
		"Memory statistics\n"
		"show slabinfo content\n")
{
	FILE *file=NULL;	
	char line[MEM_COMMAND_LEN];
	struct _slabinfo slabinfo;
	struct _slabinfo_title slabinfo_title;
	
	file = popen("cat /proc/slabinfo","r");	
	
	if(NULL == file)	
	{		
		vty_out (vty,"Can not get memory of kernel\n");	
   		return CMD_WARNING;	
	} 

	memset(line,0,MEM_COMMAND_LEN);
	memset(&slabinfo,0,sizeof(slabinfo));
	memset(&slabinfo_title,0,sizeof(slabinfo_title));
	
	fgets(line,MEM_COMMAND_LEN,file);	

	fscanf(file,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",\
		slabinfo_title.prompt,\
		slabinfo_title.name,\
		slabinfo_title.active_objs,\
		slabinfo_title.num_objs,\
		slabinfo_title.objsize,\
		slabinfo_title.objperslab,\
		slabinfo_title.pagesperslab,\
		slabinfo_title.colon1,\
		slabinfo_title.tunables,\
		slabinfo_title.limit,\
		slabinfo_title.batchcount,\
		slabinfo_title.sharedfactor,\
		slabinfo_title.colon2,\
		slabinfo_title.slabdata,\
		slabinfo_title.active_slabs,\
		slabinfo_title.num_slabs,\
		slabinfo_title.sharedavail\
		);
	
	vty_out(vty,"%-17s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s\n",\
		slabinfo_title.name,\
		slabinfo_title.active_objs,\
		slabinfo_title.num_objs,\
		slabinfo_title.objsize,\
		slabinfo_title.objperslab,\
		slabinfo_title.pagesperslab,\
		slabinfo_title.colon1,\
		slabinfo_title.tunables,\
		slabinfo_title.limit,\
		slabinfo_title.batchcount,\
		slabinfo_title.sharedfactor,\
		slabinfo_title.colon2,\
		slabinfo_title.slabdata,\
		slabinfo_title.active_slabs,\
		slabinfo_title.num_slabs,\
		slabinfo_title.sharedavail);
	
	while(fscanf(file,"%s%d%d%d%d%d%s%s%d%d%d%s%s%d%d%d",\
		slabinfo.name,\
		&slabinfo.act_objs,\
		&slabinfo.num_objs,\
		&slabinfo.objsize,\
		&slabinfo.objperslab,\
		&slabinfo.pagesperslab,\
		slabinfo.colon1,\
		slabinfo.tunable,\
		&slabinfo.limit,\
		&slabinfo.batchcount,\
		&slabinfo.sharedfactor,\
		slabinfo.colon2,\
		slabinfo.slabdata,\
		&slabinfo.act_slabs,\
		&slabinfo.num_slabs,\
		&slabinfo.sharedatabail)!=-1)	
		{					
			vty_out(vty,\
				"%-32s%8d %8d %8d %4d %4d %s %-8s\t %4d %4d %4d %s %-8s\t%4d %4d %4d\n\n",\
				slabinfo.name,\
				slabinfo.act_objs,\
				slabinfo.num_objs,\
				slabinfo.objsize,\
				slabinfo.objperslab,\
				slabinfo.pagesperslab,\
				slabinfo.colon1,\
				slabinfo.tunable,\
				slabinfo.limit,\
				slabinfo.batchcount,\
				slabinfo.sharedfactor,\
				slabinfo.colon2,\
				slabinfo.slabdata,\
				slabinfo.act_slabs,\
				slabinfo.num_slabs,\
				slabinfo.sharedatabail);	
		}
		pclose(file);
	
		return CMD_SUCCESS;
}


/*Show the size of subdirectories in the root directory*/
DEFUN (show_memory_ramfilesystem,
		show_memory_ramfilesystem_cmd,
		"show memory ramfilesystem",
		SHOW_STR
		"Memory statistics\n"
		"Show the size of subdirectories in the root directory\n")
{
	system("du / --max-depth=1 -h 2>/dev/null |grep -v 'sys'|grep -v 'proc'");
	return CMD_SUCCESS;
}

/*Display occupancy situation of process in memory*/
DEFUN (show_memory_process,
		show_memory_process_cmd,
		"show memory process",
		SHOW_STR
		"Memory statistics\n"
		"Show each process in memory occupancy situation\n")
{
    char *memprocess[16];
    FILE *file=NULL;
	char line[MEM_COMMAND_LEN];
	char *token=NULL;
	int i=0;
	
	file = popen("ps aux","r");
	
	if(NULL == file)
	{
		vty_out(vty,"Can not show occupancy situation of processes in memory\n");	

   		return CMD_WARNING;
	}
	
	memset(memprocess,0,16);
	memset(line,0,MEM_COMMAND_LEN);
	
	while(fgets(line,MEM_COMMAND_LEN,file)!= NULL)
	{	
		i=0;
		token = strtok(line," ");
		
		while(token)
		{  
			memprocess[i]=token;
			i++;
			if(i==10)
			{
				token = strtok(NULL,"\r");
				memprocess[i]=token;
				break;
			}
			token = strtok(NULL," ");
		}
		
		for(i=0;i<11;i++)
		{	
			if(i==0||i==4||i==5||i==6||i==7)
				continue;
			vty_out(vty,"%s\t",memprocess[i]);
		}
		
		vty_out(vty,"\n");
	}
	
	pclose(file);
	
	return CMD_SUCCESS;
}

/*Display residual memory size of SD card */
DEFUN (show_memory_storagecard,
		show_memory_storagecard_cmd,
		"show memory storage-card",
		SHOW_STR
		"Memory statistics\n"
		"Show residual memory size of SD card\n")
{
	system("echo -ne 'freespace\t';cat /var/run/sad/freespace;echo -ne 'totalspace\t';cat /var/run/sad/totalspace");
	
	return CMD_SUCCESS;
}


/*Display detailed statistics about memory*/
DEFUN (show_memory_stat,
		show_memory_stat_cmd,
		"show memory stat",
		SHOW_STR
		"Memory statistics\n"
		"Detailed statistics about memory ")
{
	FILE *file_ps=NULL;
	FILE *file_du=NULL;
	FILE *file_meminfo=	NULL;
	DIR* dir_du=NULL;
	struct dirent *ptr=NULL;
	char *memprocess[16];
	char line[MEM_COMMAND_LEN];
	char *token=NULL;
	char memname[32],memprompt[8],ramfs_name[32];
	int i=0;
	long memsize=0,memtotal_size=0,memfree_size=0,memuse_size=0,\
		buffer_size=0,cached_size=0,slab_size=0,ramfs_size=0\
		,psmem_size=0,tmp_size=0;
	ps_list *head=creat_ps_list();
	ps_list *cur=head;
	int ret=CMD_WARNING;

	memset(memprocess,0,16);
	memset(line,0,MEM_COMMAND_LEN);
	memset(memname,0,32);
	memset(memprompt,0,8);
	memset(ramfs_name,0,32);

	file_ps= popen("ps aux","r");
	
	if(NULL == file_ps)
	{	
		vty_out(vty,"Can't show statistics about memory\n");
		ret = CMD_WARNING;
		goto close_return;
	}

	dir_du= opendir("/");
	
	if(NULL== dir_du)
	{
		vty_out(vty,"Can't show statistics about memory\n");
		ret = CMD_WARNING;
		goto close_return;
	}

	file_meminfo= fopen("/proc/meminfo","r");
	
	if(NULL == file_meminfo)
	{
		vty_out(vty,"Can't show statistics about memory\n");
		ret = CMD_WARNING;
		goto close_return;
	}

	while(fgets(line,MEM_COMMAND_LEN,file_ps)!= NULL)
	{	
		i=0;
		token = strtok(line," ");
		while(token)
		{  
			memprocess[i]=token;
			i++;
			
			if(i == 10)
			{
				token = strtok(NULL,"\r");
				memprocess[i]=token;
				break;
			}
			token = strtok(NULL," ");
		}
		
		insert_node_in_ps_list(head,memprocess[10],atoi(memprocess[5]));
		
	}



	while(cur->next!=NULL)
	{
		psmem_size+=cur->next->rss;
		cur=cur->next;
	}
	
	free_ps_list(head);
	head=NULL;

	while((ptr=readdir(dir_du))!=NULL)
	{
		if(ptr->d_type==DT_DIR&&(strcmp(".",ptr->d_name)!=0)&&(strcmp("..",ptr->d_name)!=0)\
			&&(strcmp("proc",ptr->d_name)!=0)&&(strcmp("sys",ptr->d_name)!=0))
		{
			sprintf(ramfs_name,"du --max-depth=0 /%s",ptr->d_name);
			file_du=popen(ramfs_name,"r");
			
			if(NULL == file_du)
			{
				vty_out(vty,"Can't show statistics about memory\n");
				ret = CMD_WARNING;
				goto close_return;
			}
			
			fscanf(file_du,"%ld",&tmp_size);
			ramfs_size+=tmp_size;
			pclose(file_du);
			file_du=NULL;
		}
	}

	while(fscanf(file_meminfo,"%s%ld%s",memname,&memsize,memprompt)!=-1)
	{
		if(strncmp("MemTotal:",memname,strlen(memname)+1)==0)
			memtotal_size=memsize;
		else if(strncmp("MemFree:",memname,strlen(memname)+1)==0)
			memfree_size=memsize;
		else if(strncmp("Buffers:",memname,strlen(memname)+1)==0)
			buffer_size=memsize;
		else if(strncmp("Cached:",memname,strlen(memname)+1)==0)
			cached_size=memsize;
		else if(strncmp("Slab:",memname,strlen(memname)+1)==0)
			 slab_size=memsize;

	}
	
	vty_out(vty,"Total   : total usable memory (i.e. physical ram minus a few reserved bits and the kernel binary code)\n");
	vty_out(vty,"Used    : already be using memory\n");
	vty_out(vty,"Free    : total free memory\n");
	vty_out(vty,"Buffers : buffer cache\n");
	vty_out(vty,"Cached  : cache memory\n");
	vty_out(vty,"Slab    : slab cache\n");
	vty_out(vty,"Ramfs   : size of file system in memory\n");
	vty_out(vty,"Psmem   : resident memory part of processes\n");
	vty_out(vty,"\n");
	vty_out(vty,"total(KB) used(KB)  free(KB)  buffers(KB) cached(KB) slab(KB)  ramfs(KB) psmem(KB)\n");
	vty_out(vty,"%-10ld%-10ld%-10ld%-12ld%-11ld%-10ld%-10ld%-10ld\n",memtotal_size,memtotal_size-memfree_size,\
		memfree_size,buffer_size,cached_size,slab_size,ramfs_size,psmem_size);
	vty_out(vty,"\n");

	ret = CMD_SUCCESS;
	
close_return:

	if(file_ps)
	{
		pclose(file_ps);
	}

	if(file_meminfo)
	{
		fclose(file_meminfo);
	}

	if(dir_du)
	{
		closedir(dir_du);
	}

	if(file_du)
	{
		pclose(file_du);
	}
	if(head)
	{
		free_ps_list(head);
		head=NULL;

	}
	return ret;
}
struct cmd_node dns_ser_node =
{
  DNS_SER_NODE,
  "%s(config-dns-server)# ",
};
DEFUN (config_dns_server_func,
	 config_dns_server_cmd,
	 "config dns server",
	 CONFIG_STR
	 "Dns server\n"
	 "Dns server")
{
  vty->node = DNS_SER_NODE;
  return CMD_SUCCESS;
}

DEFUN (service_dns_func,
	 service_dns_cmd,
	 "service dns (enable|disable)",
	 "System service\n"
	 "Dns server\n"
	 "Dns server enable\n"
	 "Dns server disable")
{
  char *cmd=NULL;
  unsigned int i=0,ret=0;

  DBusMessage *query=NULL; 
  DBusMessage *reply=NULL;
  DBusMessageIter  iter;
  DBusError err;

  cmd = malloc(256);
  if(!cmd)
  {
	vty_out(vty,"system error!\n");
	return CMD_WARNING;
  }
  if(!strncmp(argv[0],"enable",strlen(argv[0])))
  {
    sprintf(cmd,"sudo /etc/init.d/bind9 start" );
/*	
	system("sudo /etc/init.d/bind9 start ");
*/	
  }
  else if(!strncmp(argv[0],"disable",strlen(argv[0])))
  {
	  sprintf(cmd,"sudo /etc/init.d/bind9 stop" );
/*
	  system("sudo /etc/init.d/bind9 stop ");
*/
  }
/*  
  system(cmd);
*/
  for(i = 0;i < MAX_SLOT ; i++)
  {
	  if(NULL != (dbus_connection_dcli[i] -> dcli_dbus_connection))
	  {

		  dbus_error_init(&err);

		query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
			 SEM_DBUS_INTERFACE,SEM_DBUS_EXECUTE_SYSTEM_COMMAND);


		dbus_message_append_args(query,
			DBUS_TYPE_UINT32, &i,
			DBUS_TYPE_STRING, &cmd,
						DBUS_TYPE_INVALID);

		reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[i] -> dcli_dbus_connection, query, 60000, &err);

		dbus_message_unref(query);
		if (NULL == reply)
		{
			vty_out(vty,"<error> failed get reply.\n");
			
			if (dbus_error_is_set(&err)) 
			{
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			
			if(cmd)
			  free(cmd);
			return CMD_WARNING;
		}

		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&ret);


		dbus_message_unref(reply);
		}

  }
  if(cmd)
  	free(cmd);
  return CMD_SUCCESS;
}




int dcli_dns_server_write (struct vty *vty)
{
	int ret;
	char str[128],cmd[256]={0};
	char show_str[256]={0};
	char *pstr;
	FILE *fp =	NULL;

	sprintf(cmd,"ps -ef | grep named | grep -v \"grep named\" | wc -l ");

	fp = popen(cmd,"r");
	
	if(fp && fgets(str,128,fp))
	{	
		int i = atoi(str);
		
		if(atoi(str) > 0)		
		{
			sprintf(show_str,"config dns server\n service dns enable\n exit\n");
			vtysh_add_show_string(show_str);
		}

	}
	if(fp)
		pclose(fp);
	return 0;
}

void dcli_sys_manage_init()
{

	install_node(&system_mng_node,dcli_sys_manage_write,"SYS_MNG_NODE");
	install_node(&dns_ser_node,dcli_dns_server_write,"DNS_SER_NODE");
	install_default (DNS_SER_NODE);

	/*install_element (CONFIG_NODE, &system_reboot_cmd);*/
	/*nstall_element (ENABLE_NODE, &system_reboot_cmd);*/
	install_element (ENABLE_NODE, &show_sll_slot_sys_info_func_cmd);	
	/*install_element (HIDDENDEBUG_NODE, &system_reboot_cmd);*/
	install_element (ENABLE_NODE, &system_time_cmd);	
	install_element (VIEW_NODE, &system_time_cmd);	
	install_element (ENABLE_NODE, &set_system_time_func_cmd);	
	install_element (CONFIG_NODE, &set_system_time_func_cmd);	
	install_element (HIDDENDEBUG_NODE, &set_time_offset_func_cmd);	
	//install_element (HIDDENDEBUG_NODE, &snapshot_func_cmd);	
	install_element (ENABLE_NODE, &show_system_location_func_cmd);	
	install_element (VIEW_NODE, &show_system_location_func_cmd);	
	install_element (CONFIG_NODE, &show_system_location_func_cmd);	
	install_element (ENABLE_NODE, &set_system_location_func_cmd);	
	install_element (CONFIG_NODE, &set_system_location_func_cmd);	
	install_element (CONFIG_NODE, &config_icmp_cmd);	

	install_element (ENABLE_NODE, &show_net_element_func_cmd);	
	install_element (VIEW_NODE, &show_net_element_func_cmd);	
	install_element (CONFIG_NODE, &show_net_element_func_cmd);	
	install_element (ENABLE_NODE, &net_element_func_cmd); 
	install_element (CONFIG_NODE, &net_element_func_cmd); 
	//install_element (HIDDENDEBUG_NODE, &terminal_monitor_module_func_cmd); 
	//install_element (HIDDENDEBUG_NODE, &no_terminal_monitor_module_func_cmd); 
	install_element (HIDDENDEBUG_NODE, &show_time_slot_func_cmd); 
	install_element (HIDDENDEBUG_NODE, &debug_time_slot_func_cmd); 
	install_element(CONFIG_NODE, &set_smux_start_cmd);
	install_element(CONFIG_NODE, &no_smux_start_cmd);
	install_element(ENABLE_NODE,&clean_flag_cmd);
	install_element(ENABLE_NODE,&kick_user_cmd);
	install_element(CONFIG_NODE,&kick_user_cmd);
	install_element(CONFIG_NODE,&clean_flag_cmd);
	#ifdef DISTRIBUT
	install_element(CONFIG_NODE,&config_dbus_session_cmd);
	install_element(CONFIG_NODE,&config_dbus_session_remote_cmd);
	#endif
	install_element(CONFIG_NODE,&show_dbus_session_cmd);

	/**gjd : support for pfm moudle**/
	install_element (CONFIG_NODE, &config_pfm_cmd);	
	install_element (HIDDENDEBUG_NODE, &service_pfm_log_func_cmd);	
	install_element (HIDDENDEBUG_NODE, &service_pfm_func_cmd);
	install_element (HIDDENDEBUG_NODE, &config_pfm_cmd_general);
	
	install_element (HIDDENDEBUG_NODE, &show_pfm_table_func_cmd);
	install_element (ENABLE_NODE, &system_reset_all_cmd);
	install_element (CONFIG_NODE, &system_reset_all_cmd);

	/* start - sfd by zhengbo */
	install_element (CONFIG_NODE, &dcli_sfd_log_switch_cmd);
	install_element (CONFIG_NODE, &dcli_sfd_log_switch_slot_cmd);
	install_element (CONFIG_NODE, &dcli_sfd_debug_switch_cmd);
	install_element (CONFIG_NODE, &dcli_sfd_debug_switch_slot_cmd);
	install_element (CONFIG_NODE, &dcli_sfd_switch_cmd);
	install_element (CONFIG_NODE, &dcli_sfd_switch_slot_cmd);
	install_element (CONFIG_NODE, &dcli_sfd_timespan_cmd);
	install_element (CONFIG_NODE, &dcli_sfd_timespan_slot_cmd);

	install_element (CONFIG_NODE, &dcli_sfd_limitpacket_cmd);
	install_element (CONFIG_NODE, &dcli_sfd_limitpacket_slot_cmd);
	install_element (CONFIG_NODE, &dcli_sfd_user_add_cmd);
	install_element (CONFIG_NODE, &dcli_sfd_user_add_slot_cmd);
	install_element (CONFIG_NODE, &dcli_sfd_user_del_cmd);
	install_element (CONFIG_NODE, &dcli_sfd_user_del_slot_cmd);

	install_element (CONFIG_NODE, &dcli_sfd_arpswitch_cmd);
	install_element (CONFIG_NODE, &dcli_sfd_arpswitch_slot_cmd);
	install_element (CONFIG_NODE, &dcli_sfd_arplimitpacket_cmd);
	install_element (CONFIG_NODE, &dcli_sfd_arplimitpacket_slot_cmd);
	install_element (CONFIG_NODE, &dcli_sfd_user_addarp_cmd);
	install_element (CONFIG_NODE, &dcli_sfd_user_addarp_slot_cmd);
	install_element (CONFIG_NODE, &dcli_sfd_user_delarp_cmd);
	install_element (CONFIG_NODE, &dcli_sfd_user_delarp_slot_cmd);
	install_element (CONFIG_NODE, &dcli_sfd_show_var_cmd);
	install_element (ENABLE_NODE, &dcli_sfd_show_var_cmd);
	install_element (VIEW_NODE, &dcli_sfd_show_var_cmd);
	install_element (CONFIG_NODE, &dcli_sfd_show_var_slot_cmd);
	install_element (ENABLE_NODE, &dcli_sfd_show_var_slot_cmd);
	install_element (VIEW_NODE, &dcli_sfd_show_var_slot_cmd);
	/* end - sfd by zhengbo */
	install_element (CONFIG_NODE, &dcli_sfd_arpswitch_cmd);

	/**hxx added for dns service**/
	install_element (VIEW_NODE, &show_ip_dns_func_cmd);
	install_element (ENABLE_NODE, &show_ip_dns_func_cmd);
	install_element (CONFIG_NODE, &set_ip_dns_func_cmd);
	install_element (CONFIG_NODE, &delete_ip_dns_func_cmd);
	install_element (CONFIG_NODE, &service_ip_dns_func_cmd);
	install_element (CONFIG_NODE, &service_radius_func_cmd);
	install_element (ENABLE_NODE, &set_ip_dns_func_cmd);
	install_element (ENABLE_NODE, &delete_ip_dns_func_cmd);
	install_element (ENABLE_NODE, &service_ip_dns_func_cmd);
	install_element (ENABLE_NODE, &service_radius_func_cmd);

/*
	install_element (CONFIG_NODE, &terminal_monitor_module_func_cmd); 
	install_element (CONFIG_NODE, &no_terminal_monitor_module_func_cmd); 
*/

/*add by zhaocg 2012-10-11 pm 15:30*/
  install_element (VIEW_NODE, &show_memory_kernel_cmd);
  install_element (VIEW_NODE, &show_memory_ramfilesystem_cmd);
  install_element (VIEW_NODE, &show_memory_process_cmd);
  install_element (VIEW_NODE, &show_memory_storagecard_cmd);
  install_element (ENABLE_NODE, &show_memory_kernel_cmd);
  install_element (ENABLE_NODE, &show_memory_ramfilesystem_cmd);
  install_element (ENABLE_NODE, &show_memory_process_cmd);
  install_element (ENABLE_NODE, &show_memory_storagecard_cmd);
  
  install_element (SYSTEM_DEBUG_NODE, &show_memory_kernel_cmd);
  install_element (SYSTEM_DEBUG_NODE, &show_memory_ramfilesystem_cmd);
  install_element (SYSTEM_DEBUG_NODE, &show_memory_process_cmd);
  install_element (SYSTEM_DEBUG_NODE, &show_memory_storagecard_cmd);
  
  install_element (VIEW_NODE, &show_memory_stat_cmd);
  install_element (ENABLE_NODE, &show_memory_stat_cmd);
  install_element (SYSTEM_DEBUG_NODE, &show_memory_stat_cmd);

 /*end by zhaocg 2012-10-12 pm 18:30*/
 install_element (CONFIG_NODE, &config_dns_server_cmd);
 install_element (DNS_SER_NODE, &service_dns_cmd);

install_element (CONFIG_NODE, &no_set_smux_debug_cmd);
install_element (CONFIG_NODE, &set_smux_debug_cmd);
}


#ifdef __cplusplus
}
#endif


