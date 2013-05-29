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

********************************************************************************
* dcli_se_agent.c
*
*
* CREATOR:
*		pangyy@autelan.com.cn
*
* DESCRIPTION:
*		CLI definition for se_agent module.
*
* DATE:
*		07/26/2011
*
*  FILE REVISION NUMBER:
*  		$Revision:  $0.0	
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <zebra.h>
#include <dbus/dbus.h>
#include <linux/un.h>
#include <linux/tipc.h>
#include <sys/select.h>
#include <sys/time.h>
#include <syslog.h>
#include <stdarg.h>
#include "dcli_main.h"
#include "vty.h"
#include "command.h"
#include "dcli_system.h"
#include "dcli_se_agent.h"
#include "se_agent/se_agent_def.h"
#include "sysdef/returncode.h"

/*read_within_time  funcation return error value*/
#define READ_ERROR   (-1)
#define WRITE_ERROR  (-1)

#define CVMX_FPA_NUM_POOLS      (8)

#define INVALID_SLOTID (-1)
#define INVALID_SOCKET (-1)
/*dcli local socket file description*/
int dcli_sockfd = INVALID_SOCKET;


struct cmd_node fwd_node = {
	FAST_FWD_NODE,
	"%s(config-fastfwd)# ",
	1
};

struct cmd_node slave_fwd_node = {
	SLAVE_FAST_FWD_NODE,
	"%s(config-slave-fastfwd)# ",
	1
};

#define DISPLAY_CAPWAP_CNT     5
#define DISPLAY_ACL_CNT        5
#define DISPLAY_ACL_RULE_CNT   10
#define MAX_AGING_TIME  (4294967294)	// 2^31 s
#define MIN_AGING_TIME  (1)
#define MIN_LOG_LEVEL  (1)
#define MAX_LOG_LEVEL  (5)



struct sockaddr_tipc   se_agent_address[MAX_SLOT_NUM+1]={0};
int se_agent_addrlen = sizeof(struct sockaddr_tipc);

extern unsigned long dcli_ip2ulong(char *str);

void fill_cpu_tag(se_interative_t *cmd_data, struct vty *vty)
{
    if(NULL == vty)
        return;
    if(vty->node == SLAVE_FAST_FWD_NODE)
	    cmd_data->cpu_tag = 1;
	else 
	    cmd_data->cpu_tag = 0;
}

/*Analysis of the string is  a number*/
int parse_isnum(const char *str)
{
	int len=0,i=0;
	if(NULL == str){
		return COMMON_ERROR;
	}
	if((len=strlen(str))<=0)
		return COMMON_ERROR;
	for(i=0;i<len;i++)
	{
		if(str[i]<'0' || str[i]>'9')
			return COMMON_ERROR;
	}
	return COMMON_SUCCESS;		
}
unsigned char parse_protocol(char *str)
{
	if(NULL == str)
		return INVALID_TYPE;
	if(0 == strcmp(str,"tcp"))
		return TCP_TYPE;
	else if(0 == strcmp(str,"udp"))
		return UDP_TYPE;
	else if(0 == strcmp(str,"icmp"))
		return ICMP_TYPE;
	else
		return INVALID_TYPE;
}

/*Analysis of the string format A.B.C.D:PORT*/
int parse_ipport_check(char * str)
{
	char *sep=".";
	char *colon=":";
	char *token = NULL;
	char *tail_ptr=NULL;
	unsigned long ip_long[4] = {0}; 
	int i = 0;
	int pointCount=0,colonCount=0;
	char param[DCLI_IPPORT_STRING_MAXLEN+1]={0};
	char ipAddr[DCLI_IPPORT_STRING_MAXLEN+1]={0};
	if(str==NULL||strlen(str)>DCLI_IPPORT_STRING_MAXLEN || \
		strlen(str) < DCLI_IPPORT_STRING_MINLEN )
	{
		return COMMON_ERROR;
	}
	if((str[0] > '9')||(str[0] < '1'))
	{
		return COMMON_ERROR;
	}
	for(i=0;i<strlen(str);i++)
	{
		if('.' == str[i])
		{
            pointCount++;
			if((i == strlen(str)-1)||('0' > str[i+1])||(str[i+1] > '9'))
			{
				return COMMON_ERROR;
			}
		}
		if(':' == str[i])
		{
			colonCount++;
			if((i == strlen(str)-1)||('0' > str[i+1])||(str[i+1] > '9'))
			{
				return COMMON_ERROR;
			}
		}
		if((str[i]>'9'||str[i]<'0')&&str[i]!='.'&&str[i]!='\0'&&str[i]!=':')
		{
			return COMMON_ERROR;
		}
	}
	if(3 != pointCount|| 1!=colonCount)
	{
        return COMMON_ERROR;
	}
	strncpy((char*)param,str,strlen(str));
	token=strtok_r((char*)param,colon,&tail_ptr);
	if((NULL ==token) ||( ""==token)||(strlen(token)<1))
	{
		return COMMON_ERROR;
	}
	strcpy((char*)ipAddr,token);
	
	token=strtok_r(NULL,colon,&tail_ptr);
	if((NULL == token)||("" == token)||(strlen(token) < 1))
	{
		return COMMON_ERROR;
	}
	if(0>strtoul(token,NULL,10)|| MAX_PORT < strtoul(token,NULL,10))
		return COMMON_ERROR;
	
	token=strtok_r(ipAddr,sep,&tail_ptr);
	if((NULL == token)||("" == token)||(strlen(token) < 1)||\
		((strlen(token) > 1) && ('0' == token[0])))
	{
		return COMMON_ERROR;
	}
	if(NULL != token)
	{
	    ip_long[0] = strtoul(token,NULL,10);
	}
	else 
	{
		return COMMON_ERROR;
	}
	i=1;
	while((token!=NULL)&&(i<4))
	{
		token=strtok_r(NULL,sep,&tail_ptr);
		if((NULL == token)||("" == token)||(strlen(token) < 1)|| \
			((strlen(token) > 1) && ('0' == token[0])))
		{
			return COMMON_ERROR;
		}
		if(NULL != token)
		{
		    ip_long[i] = strtoul(token,NULL,10);
		}
		else 
		{
			return COMMON_ERROR;
		}
		i++;
	}
	for(i=0;i<4;i++)
	{
        if(ip_long[i]>255)
		{
            return COMMON_ERROR;
		}
	}
	return COMMON_SUCCESS;	
}

/**********************************************************************************
 *  se_agent_init_address
 *
 *	DESCRIPTION:
 * 		Initialize the address array of the se_agent module,are used for se_agent module address resolution
 *
 *	INPUT:
 *		slot_num 			 ->       The number of  slots on the device
 *		se_agent_addr_tbl	 ->       point to  the first item of the se_agent address table 
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:	
 *          CMD_FAILURE          ->   Failed  initialization
 *          CMD_SUCCESS         ->   Successful initialization 
 **********************************************************************************/
int se_agent_init_address(int slot_num,struct sockaddr_tipc *se_agent_addr_tbl)
{
	int i=0;
	if((slot_num <1) || (slot_num > MAX_SLOT_NUM) || (NULL == se_agent_addr_tbl))
	{
		return CMD_FAILURE;
	}
	for(i=1;i<=slot_num;i++)
	{
		(se_agent_addr_tbl+i)->family = AF_TIPC;
		(se_agent_addr_tbl+i)->addrtype = TIPC_ADDR_NAME;
		(se_agent_addr_tbl+i)->scope = TIPC_CLUSTER_SCOPE;
		(se_agent_addr_tbl+i)->addr.name.domain = 0;
		(se_agent_addr_tbl+i)->addr.name.name.type = SE_AGENT_SERVER_TYPE;
		(se_agent_addr_tbl+i)->addr.name.name.instance = (SE_AGENT_SERVER_LOWER_INST + i);
	}
	return CMD_SUCCESS;
}

/**********************************************************************************
 *  se_agent_get_slotid
 *
 *	DESCRIPTION:
 * 		Returns the slot  ID of the board where se_agent module running 
 *
 *	INPUT:
 *		vty 			 ->      struct vty,vty->slotindex  is the slot id of the  board to be accessed.
 							if vty ==NULL ,access local board
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		
 *          INVALID_SLOTID          ->   Failed  get slotid
 *          other                           ->   slot id 
 **********************************************************************************/
int se_agent_get_slotid(struct vty *vty)
{
	FILE *file_slotid = NULL;
	char str_slotid[10] = {0};
	int slotid = INVALID_SLOTID;
	if(NULL == (file_slotid = fopen("/dbm/local_board/slot_id","r")))
	{
		return INVALID_SLOTID;
	}
	if(NULL == fgets(str_slotid, 10, file_slotid))
	{
		printf("read slot_id failed!\n");
		fclose(file_slotid);
		return INVALID_SLOTID;
	}
	fclose(file_slotid);
	
	if(NULL == vty)
	{
		slotid = atoi(str_slotid);
		#ifdef DCLI_SE_AGENT_DEBUG
		printf( "local board slotid:  %d\n", slotid);
		#endif	
		return slotid;
	}
	if(FAST_FWD_NODE == vty->node || SLAVE_FAST_FWD_NODE == vty->node || CONFIG_NODE == vty->node || ENABLE_NODE == vty->node || VIEW_NODE == vty->node || HIDDENDEBUG_NODE == vty->node)
	{
		slotid = atoi(str_slotid);
		#ifdef DCLI_SE_AGENT_DEBUG
		printf( "fwd node: get_slotid_index: slot id %d\n", slotid);
		#endif	
		return slotid;
	}
	else if((HANSI_NODE == vty->node) || LOCAL_HANSI_NODE == vty->node)
	{
		slotid = vty->slotindex;
		if(slotid <1 || slotid > MAX_SLOT_NUM)
		{
			return INVALID_SLOTID;
		}
		#ifdef DCLI_SE_AGENT_DEBUG
		printf( "hansi node: get_slotid_index: slot id %d\n", slotid);
		#endif	
		return slotid;
	}
	else
	{
		return INVALID_SLOTID;
	}
}

/**********************************************************************************
 *  read_within_time
 *
 *	DESCRIPTION:
 * 		Read from se_agent within the specified time
 *
 *	INPUT:
 *		fd 			 ->       the open file description
 *		buf			 ->       pointer of store data buffer
 *		len 			 ->       the length of data to be read
 *		overtime  	 ->       read with blocking time		
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		
 *             READ_ERROR       ->   read failed
 *             other                   ->   the number of bytes actually  read  
 **********************************************************************************/
 int read_within_time(int fd,char *buf,unsigned int len,struct timeval *overtime)
{
	int ret = READ_ERROR;
	fd_set	 rset;
	if(fd<0 ||NULL==buf)
		return READ_ERROR;
	FD_ZERO(&rset);
	FD_SET(fd,&rset);
	ret=select(fd+1,&rset,NULL,NULL,overtime);
	if(ret<=0)
		return READ_ERROR;
	else
	{
		if(FD_ISSET(fd,&rset))
		{
			ret=recvfrom(fd,buf,len,0,NULL,NULL);
			if(ret<0)
				return READ_ERROR;
		}
		else
			return READ_ERROR;
	}
	return ret;	
}
/**********************************************************************************
 *  sendto_agent
 *
 *	DESCRIPTION:
 * 		Write to se_agent ,Clear the receive buffer before sending.
 *
 *	INPUT:
 *		fd 			 ->       the open file description
 *		buf			 ->       pointer of write data buffer
 *		len 			 ->       the length of data to be write	
 *		slotid            ->       board slot number 
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		
 *             WRITE_ERROR       ->   send failed
 *             other                    ->   the number of bytes actually  write  
 **********************************************************************************/
int sendto_agent(int fd,char *buf,unsigned int len ,struct vty *vty)
{
	int ret=WRITE_ERROR;
	char tmpbuf[1024]={0};
	fd_set  rset;
	int slotid = INVALID_SLOTID;
	struct timeval overtime={
		.tv_sec =0,
		.tv_usec=0
	};
	if(fd<=0 || NULL ==buf)
	{
		return WRITE_ERROR;
	}
		
	/*Clear the receive buffer before sending*/
	FD_ZERO(&rset);
	FD_SET(fd,&rset);
	while((select(fd+1,&rset,NULL,NULL,&overtime))>0)
	{
		recvfrom(fd,tmpbuf,sizeof(tmpbuf),0,0,0);   
	}
	
	slotid=se_agent_get_slotid(vty);
	if(INVALID_SLOTID == slotid)
	{
		return WRITE_ERROR;
	}
	ret=sendto(fd,buf,len,0,(struct sockaddr*)&se_agent_address[slotid],se_agent_addrlen);
	if(ret<0)
		return WRITE_ERROR;
	else
		return ret;
}




/**********************************************************************************
 *  se_agent_config_debug
 *
 *	DESCRIPTION:
 * 		Configure the se_agent  module level of debugging information
 *
 *	INPUT:
 *		vty  		-> 	vty
 *		level	->	debug level
 *		enable	->	on or off debug level
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		CMD_SUCCESS	-> 	success 
 *		CMD_FAILURE  	-> 	fail  
 **********************************************************************************/
int se_agent_config_debug(struct vty *vty,char *level,unsigned int enable)
{
	unsigned int flag = 0, ret = 0;
	
	if(!vty || !level) 
		return CMD_FAILURE;
	if(0 == strncmp(level,"all",strlen(level))) 
	{
		flag = DCLI_DEBUG_FLAG_ALL;
	}		
	else if(0 == strncmp(level,"error",strlen(level))) 
	{
		flag = DCLI_DEBUG_FLAG_ERR;
	}
	else if (0 == strncmp(level,"info",strlen(level)))
	{
		flag = DCLI_DEBUG_FLAG_WAR;
	}
	else if (0 == strncmp(level,"debug",strlen(level)))
	{
		flag = DCLI_DEBUG_FLAG_DBG;
	}
	else 
	{
		vty_out(vty,CMD_PARAMETER_ERROR);
		return CMD_FAILURE;
	}
	se_interative_t  cmd_data;
	memset(&cmd_data,0,sizeof(cmd_data));
	cmd_data.fccp_cmd.fccp_data.agent_dbg.level=flag;
	cmd_data.fccp_cmd.fccp_data.agent_dbg.enable=enable;
	strncpy(cmd_data.hand_cmd,SE_AGENT_CONFIG_AGENT_DEBUG_LEVEL,strlen(SE_AGENT_CONFIG_AGENT_DEBUG_LEVEL));
	
	ret=sendto_agent(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),vty);
	if(ret<=0)
	{
		vty_out(vty,WRITE_FAIL_STR);
		return CMD_FAILURE;
	}
	return CMD_SUCCESS;
}

char* detect_ipfwd_learn_module_name(void)
{
    FILE *file = NULL;
    file = fopen("/sys/module/ipfwd_learn/parameters/learn_enable","r");
    if(file !=NULL)
    {
        fclose(file);
        return "ipfwd_learn";
    }

    file = fopen("/sys/module/ipfwd_learn_coexist/parameters/learn_enable","r");
    if(file !=NULL)
    {
        fclose(file);
        return "ipfwd_learn_coexist";
    }

    file = fopen("/sys/module/ipfwd_learn_standalone/parameters/learn_enable","r");
    if(file !=NULL)
    {
        fclose(file);
        return "ipfwd_learn_standalone";
    }

    return NULL;
}


DEFUN(debug_ipfwd_learn_func,
		debug_ipfwd_learn_cmd,
		"debug ipfwd_learn (default|emerge|alert|crit|error|waring|notice|info|icmp|debug)",
		DEBUG_STR
		MODULE_DEBUG_STR(ipfwd_learn)
		MODULE_DEBUG_LEVEL_STR(ipfwd_learn,default)
		MODULE_DEBUG_LEVEL_STR(ipfwd_learn,emerge)
		MODULE_DEBUG_LEVEL_STR(ipfwd_learn,alert)
		MODULE_DEBUG_LEVEL_STR(ipfwd_learn,crit)
		MODULE_DEBUG_LEVEL_STR(ipfwd_learn,error)
		MODULE_DEBUG_LEVEL_STR(ipfwd_learn,waring)
		MODULE_DEBUG_LEVEL_STR(ipfwd_learn,notice)
		MODULE_DEBUG_LEVEL_STR(ipfwd_learn,info)
		MODULE_DEBUG_LEVEL_STR(ipfwd_learn,icmp)
		MODULE_DEBUG_LEVEL_STR(ipfwd_learn,debug)
)
{
	char strcmd[100]={0};
	char* ipfwd_learn_name = NULL;
	if(argc>1)
	{
		vty_out(vty,CMD_PARAMETER_NUM_ERROR);
		return CMD_FAILURE;
	}
	char *level=(char *)argv[0];
	int flag = IPFWD_DEFAULT_LVL;
	if(0 == strncmp(level,"emerge",strlen(level)))
		flag=EMERG_LVL;
	else if(0 == strncmp(level,"alert",strlen(level)))
		flag=ALERT_LVL;
	else if(0 == strncmp(level,"crit",strlen(level)))
		flag=CRIT_LVL;
	else if(0 == strncmp(level,"error",strlen(level)))
		flag=ERR_LVL;
	else if(0 == strncmp(level,"waring",strlen(level)))
		flag=WARNING_LVL;
	else if(0 == strncmp(level,"notice",strlen(level)))
		flag=NOTICE_LVL;
	else if(0 == strncmp(level,"info",strlen(level)))
		flag=INFO_LVL;
	else if(0 == strncmp(level,"icmp",strlen(level)))
		flag=ICMP_DEBUG;
	else if(0 == strncmp(level,"debug",strlen(level)))
		flag=DEBUG_LVL;
	else if(0 == strncmp(level,"default",strlen(level)))
		flag=IPFWD_DEFAULT_LVL;
	else
	{
		vty_out(vty,CMD_PARAMETER_ERROR);
		return CMD_FAILURE;
	}

	ipfwd_learn_name = detect_ipfwd_learn_module_name();
	if (NULL != ipfwd_learn_name)
	{
		sprintf(strcmd,"sudo echo %d > /sys/module/%s/parameters/log_level",flag, ipfwd_learn_name);
	}

	vty_out(vty,"%s",strcmd);

	if((system(strcmd))<0)
		vty_out(vty,"%s" COMMAND_FAIL_STR);

	return CMD_SUCCESS;
}

/**********************************************************************************
 *  se_agent_show_running_cfg
 *
 *	DESCRIPTION:
 * 		show se_agent module running configure
 *
 *	INPUT:
 *		vty  		-> 	vty
 *		
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		CMD_SUCCESS	-> 	success 
 *		CMD_FAILURE  	-> 	fail  
 **********************************************************************************/
 int se_agent_show_running_cfg(struct vty *vty)
{	
	char tmpstr[64];
	char showStr[SE_AGENT_RUNNING_CFG_MEM]={0};
	int ret = CMD_FAILURE;
	se_interative_t  cmd_data;
	struct timeval overtime;
	memset(tmpstr,0,64);
	memset(&overtime,0,sizeof(overtime));
	memset(&cmd_data,0,sizeof(cmd_data));
	sprintf(tmpstr,BUILDING_MOUDLE,"FAST_FORWARD");
	vtysh_add_show_string(tmpstr);
	
	strncpy(cmd_data.hand_cmd,SE_AGENT_SHOW_RUNNING_CFG,strlen(SE_AGENT_SHOW_RUNNING_CFG));
	ret=sendto_agent(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),vty);
	if(ret<=0)
	{
		vty_out(vty,WRITE_FAIL_STR);
		goto FUN_END;
	}
	overtime.tv_sec = DCLI_WAIT_TIME;
	overtime.tv_usec = 0;
	ret=read_within_time(dcli_sockfd,showStr,SE_AGENT_RUNNING_CFG_MEM,&overtime);
	if(ret==READ_ERROR)
	{
		/*vty_out(vty,AGENT_NO_RESPOND_STR);*/
		goto FUN_END;
	}
FUN_END:
	vtysh_add_show_string(showStr);
	return CMD_SUCCESS;
}
DEFUN(config_fastfwd_func,
		config_fastfwd_cmd,
		"config fast-forward",
		CONFIG_STR
		"config fast_forward\n"
)
{
	if(vty->node == CONFIG_NODE)
	{
		vty->node = FAST_FWD_NODE;
	}
	return CMD_SUCCESS;
}

DEFUN(config_slave_fastfwd_func,
		config_slave_fastfwd_cmd,
		"config slave_fast-forward",
		CONFIG_STR
		"config slave_fast_forward\n"
)
{
	if(vty->node == CONFIG_NODE)
	{
		vty->node = SLAVE_FAST_FWD_NODE;
	}
	return CMD_SUCCESS;
}

/*dcli command debug se_agent */
DEFUN(debug_se_agent_func ,
		debug_se_agent_cmd,
		"debug se_agent (all|error|info|debug)",
		DEBUG_STR
		MODULE_DEBUG_STR(se_agent)
		MODULE_DEBUG_LEVEL_STR(se_agent,all)
		MODULE_DEBUG_LEVEL_STR(se_agent,error)
		MODULE_DEBUG_LEVEL_STR(se_agent,info)
		MODULE_DEBUG_LEVEL_STR(se_agent,debug)
)
{	
	int ret = CMD_FAILURE;
	if(argc > 1) 
	{
		vty_out(vty,CMD_PARAMETER_NUM_ERROR);
		return CMD_WARNING;
	}
	ret = se_agent_config_debug(vty, (char*)argv[0], 1);
	return ret;
}


/*dcli command no debug se_agent */
DEFUN(no_debug_se_agent_func ,
		no_debug_se_agent_cmd,
		"no debug se_agent (all|error|info|debug)",
		NO_STR
		DEBUG_STR
		MODULE_DEBUG_STR(se_agent)
		"Close se_agent debug level all\n"
		"Close se_agent debug level error\n"
		"Close se_agent debug level info\n"
		"Close se_agent debug level debug\n"
)
{	
	int ret = 0;
	if(argc > 1) 
	{
		vty_out(vty,CMD_PARAMETER_NUM_ERROR);
		return CMD_WARNING;
	}
	ret = se_agent_config_debug(vty, (char*)argv[0], 0);
	return ret;
}

/*dcli command set fwd aging_time*/
DEFUN(config_fast_forward_aging_time_func,
		  config_fast_forward_aging_time_cmd,
		  "config  aging-time TIME",
		  CONFIG_STR
		  "aging time\n"
		  "the aging time(second) range(1-4294967294)\n"
)
{
	se_interative_t  cmd_data;
	int ret = 0;
	uint32_t aging_time = 0;
	struct timeval overtime;
	memset(&overtime,0,sizeof(overtime));
	memset(&cmd_data,0,sizeof(cmd_data));
	if(COMMON_ERROR == parse_isnum((char *)argv[0]))
	{
		vty_out(vty,CMD_PARAMETER_ERROR);
		return CMD_FAILURE;
	}
	aging_time = strtoul((char*)argv[0],NULL,10);
	if((aging_time < MIN_AGING_TIME) || aging_time > MAX_AGING_TIME )
	{
		vty_out(vty," %%aging time value %u--%u\n",MIN_AGING_TIME,MAX_AGING_TIME);
		return CMD_FAILURE;
	}
	cmd_data.fccp_cmd.fccp_data.aging_timer = aging_time;
	fill_cpu_tag(&cmd_data, vty);
	strncpy(cmd_data.hand_cmd,SE_AGENT_SET_AGING_TIME,strlen(SE_AGENT_SET_AGING_TIME));
	ret=sendto_agent(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),vty);
	if(ret<=0)
	{
		vty_out(vty,WRITE_FAIL_STR);
		return CMD_FAILURE;
	}
	memset(&cmd_data,0,sizeof(cmd_data));
	overtime.tv_sec=DCLI_WAIT_TIME;
	ret=read_within_time(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),&overtime);
	if(ret==READ_ERROR)
	{
		vty_out(vty,AGENT_NO_RESPOND_STR);
		return CMD_FAILURE;
	}
	if(cmd_data.cmd_result!=AGENT_RETURN_OK)
	{
		vty_out(vty,"%s\n",cmd_data.err_info);
		return CMD_FAILURE;
	}
	if(cmd_data.fccp_cmd.ret_val!=FCCP_RETURN_OK)
	{
		vty_out(vty,COMMAND_FAIL_STR);
		return CMD_FAILURE;
	}
	return CMD_SUCCESS;
}



/*dcli command show  fast_forward aging_time*/
DEFUN(show_fast_forward_aging_time_func,
		  show_fast_forward_aging_time_cmd,
		  "show  aging-time ",
		  SHOW_STR
		  "aging time (second)\n"
)
{
	se_interative_t  cmd_data;
	int ret = 0;
	struct timeval overtime;
	memset(&overtime,0,sizeof(overtime));
	memset(&cmd_data,0,sizeof(cmd_data));
	strncpy(cmd_data.hand_cmd,SE_AGENT_SHOW_AGING_TIME,strlen(SE_AGENT_SHOW_AGING_TIME));
	fill_cpu_tag(&cmd_data, vty);
	ret=sendto_agent(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),vty);
	if(ret<=0)
	{
		vty_out(vty,WRITE_FAIL_STR);
		return CMD_FAILURE;
	}
	memset(&cmd_data,0,sizeof(cmd_data));
	overtime.tv_sec=DCLI_WAIT_TIME;
	ret=read_within_time(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),&overtime);
	if(ret==READ_ERROR)
	{
		vty_out(vty,AGENT_NO_RESPOND_STR);
		return CMD_FAILURE;
	}
	
	if(cmd_data.cmd_result!=AGENT_RETURN_OK)
	{
		vty_out(vty,"%s\n",cmd_data.err_info);
		return CMD_FAILURE;
	}
	if(cmd_data.fccp_cmd.ret_val!=FCCP_RETURN_OK)
	{
		vty_out(vty,COMMAND_FAIL_STR);
		return CMD_FAILURE;
	}
	vty_out(vty,"fast_forward aging time is %u\n",cmd_data.fccp_cmd.fccp_data.aging_timer);
	return CMD_SUCCESS;
}


/*dcli command set forward tag type*/
DEFUN(config_fast_forward_tag_type_func,
      config_fast_forward_tag_type_cmd,
      "config fast-forward tag type (ordered|atomic|null)",
      CONFIG_STR
      "fast forward moudle\n"
      "tag type\n"
      "tag type\n"
      "tag ordering is maintained\n"
      "tag ordering is maintained, and at most one PP has the tag\n"
      "the work queue entry from the order\n"
)
{
	cvmx_pow_tag_type_t type ;
	se_interative_t  cmd_data;
	int ret = 0;
	struct timeval overtime;
	if(!(strcmp(argv[0],"ordered")))
		type=CVMX_POW_TAG_TYPE_ORDERED;
	else if(!(strcmp(argv[0],"atomic")))
		type=CVMX_POW_TAG_TYPE_ATOMIC;
	else if(!(strcmp(argv[0],"null")))
		type=CVMX_POW_TAG_TYPE_NULL;
	else
	{
		vty_out(vty,CMD_PARAMETER_ERROR);
		return CMD_FAILURE;
	}
	memset(&overtime,0,sizeof(overtime));
	memset(&cmd_data,0,sizeof(cmd_data));
	cmd_data.fccp_cmd.fccp_data.tag_type=type;
	fill_cpu_tag(&cmd_data, vty);
	strncpy(cmd_data.hand_cmd,SE_AGENT_SET_FWD_TAG_TYPE, strlen(SE_AGENT_SET_FWD_TAG_TYPE));
	ret=sendto_agent(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),vty);
	if(ret<=0)
	{
		vty_out(vty,WRITE_FAIL_STR);
		return CMD_FAILURE;
	}
	memset(&cmd_data,0,sizeof(cmd_data));
	overtime.tv_sec=DCLI_WAIT_TIME;
	ret=read_within_time(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),&overtime);
	if(ret==READ_ERROR)
	{
		vty_out(vty,AGENT_NO_RESPOND_STR);
		return CMD_FAILURE;
	}
	if(cmd_data.cmd_result!=AGENT_RETURN_OK)
	{
		vty_out(vty,"%s\n",cmd_data.err_info);
		return CMD_FAILURE;
	}
	return CMD_SUCCESS;
}
      

/*dcli command set fast_forward  tag type*/
DEFUN(show_fast_forward_tag_type,
      show_fast_forward_tag_type_cmd,
      "show  fast-forward tag type",
      SHOW_STR
      "fast forward moudle\n"
      "tag type\n"
      "tag type\n"
)
{
	se_interative_t  cmd_data;
	int ret;
	char *tag_type=NULL;
	struct timeval overtime;
	memset(&overtime,0,sizeof(overtime));
	memset(&cmd_data,0,sizeof(cmd_data));
	strncpy(cmd_data.hand_cmd,SE_AGENT_SHOW_FWD_TAG_TYPE, strlen(SE_AGENT_SHOW_FWD_TAG_TYPE));
	fill_cpu_tag(&cmd_data, vty);
	ret=sendto_agent(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),vty);
	if(ret<=0)
	{
		vty_out(vty,WRITE_FAIL_STR);
		return CMD_FAILURE;
	}
	memset(&cmd_data,0,sizeof(cmd_data));
	overtime.tv_sec=DCLI_WAIT_TIME;
	ret=read_within_time(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),&overtime);
	if(ret==READ_ERROR)
	{
		vty_out(vty,AGENT_NO_RESPOND_STR);
		return CMD_FAILURE;
	}
	if(cmd_data.cmd_result!=AGENT_RETURN_OK)
	{
		vty_out(vty,"%s\n",cmd_data.err_info);
		return CMD_FAILURE;
	}
	switch (cmd_data.fccp_cmd.fccp_data.tag_type)
	{
		case CVMX_POW_TAG_TYPE_ORDERED:
			tag_type="ordered";
			break;
		case CVMX_POW_TAG_TYPE_ATOMIC :
			tag_type="atomic";
			break;
		case CVMX_POW_TAG_TYPE_NULL :
			tag_type="null";
			break;
		default:
			vty_out(vty,"read tag type error\n");
			return CMD_FAILURE;
	}
	if(NULL != tag_type)
	{
		vty_out(vty,"tag type is %s\n",tag_type);
	}
	return CMD_SUCCESS;
}      


/*dcli command switch uart se*/
DEFUN(  switch_uart_to_se,
			switch_uart_to_se_cmd,
			"switch uart se",
			"switch uart \n"
			"switch uart \n"
			"switch uart to se model\n"
)
{
	se_interative_t  cmd_data;
	int ret = 0;
	struct timeval overtime;
	memset(&overtime,0,sizeof(overtime));
	memset(&cmd_data,0,sizeof(cmd_data));
	strncpy(cmd_data.hand_cmd,SE_AGENT_UART_SWITCH,strlen(SE_AGENT_UART_SWITCH));
	ret=sendto_agent(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),vty);
	if(ret<=0)
	{
		vty_out(vty,WRITE_FAIL_STR);
		return CMD_FAILURE;
	}
	memset(&cmd_data,0,sizeof(cmd_data));
	overtime.tv_sec=DCLI_WAIT_TIME;
	ret=read_within_time(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),&overtime);
	if(ret==READ_ERROR)
	{
		vty_out(vty,AGENT_NO_RESPOND_STR);
		return CMD_FAILURE;
	}
	if(cmd_data.cmd_result!=AGENT_RETURN_OK)
	{
		vty_out(vty,"%s\n",cmd_data.err_info);
		return CMD_FAILURE;
	}
	return CMD_SUCCESS;
}

/*dcli command show fpa  buff_count*/
DEFUN(  show_fpa_buff_counter,
			show_fpa_buff_counter_cmd,
			"show  fast-forward available buffer",
			SHOW_STR
			"fast forward moudle\n"
			"show fast-forward available buffer counter info \n"
			"show fast-forward available buffer counter info \n"
)
{
	se_interative_t  cmd_data;
	int ret = -1,i = 0;
	struct timeval overtime;
	//unsigned int pool_count[CVMX_FPA_NUM_POOLS]={0};
	memset(&overtime,0,sizeof(overtime));
	memset(&cmd_data,0,sizeof(cmd_data));
	strncpy(cmd_data.hand_cmd,SE_AGENT_SHOW_AVAILIABLE_BUFF_COUNT,strlen(SE_AGENT_SHOW_AVAILIABLE_BUFF_COUNT));
	fill_cpu_tag(&cmd_data, vty);
	ret=sendto_agent(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),vty);
	if(ret<=0)
	{
		vty_out(vty,WRITE_FAIL_STR);
		return CMD_FAILURE;
	}
	overtime.tv_sec=DCLI_WAIT_TIME;
	overtime.tv_usec=0;
	ret=read_within_time(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),&overtime);
	if(ret==READ_ERROR)
	{
		vty_out(vty,AGENT_NO_RESPOND_STR);
		return CMD_FAILURE;
	}
	if(cmd_data.cmd_result!=AGENT_RETURN_OK)
	{
		vty_out(vty,"%s\n",cmd_data.err_info);
		return CMD_FAILURE;
	}
	vty_out(vty,"=====================================================\n");
	for(i=0;i<CVMX_FPA_NUM_POOLS;i++)
	{
		vty_out(vty,"          pool %d available buff is %d\n",i,cmd_data.fccp_cmd.fccp_data.pool_buff_count[i]);
	}
	vty_out(vty,"=====================================================\n");
	return CMD_SUCCESS;
}




/*dcli command delete fast forward rule*/
DEFUN(delete_fast_forward_rule,
	delete_fast_forward_rule_cmd,
	"delete fast-forward rule PROTOCOL SIP:SPORT DIP:DPORT",
	"delete fast forward rule \n"
	"fast forward module\n"
	"fastfwd acl rule information\n"
	"protocol (udp | tcp)\n"
	"source ip address and source port A.B.C.D:SPORT\n"
	"destination ip address and destination port A.B.C.D:DPORT\n"
)
{
	se_interative_t  cmd_data;
	int ret = -1;
	unsigned char protocol = INVALID_TYPE;
	struct timeval overtime;
	char *token=NULL;
	char *colon=":";
	char *sport=NULL;
	char *tail_ptr=NULL;
	memset(&overtime,0,sizeof(overtime));
	memset(&cmd_data,0,sizeof(cmd_data));
	protocol=parse_protocol((char *)argv[0]);
	if((TCP_TYPE !=protocol) && (UDP_TYPE != protocol))
	{
		vty_out(vty,"protocol is not tcp or udp\n");
		return CMD_FAILURE;
	}
	if(COMMON_ERROR==parse_ipport_check((char*)argv[1]))
	{
		vty_out(vty,"source ip address and port format error\n");
		return CMD_FAILURE;
	}
	if(COMMON_ERROR==parse_ipport_check((char*)argv[2]))
	{
		vty_out(vty,"destination ip address and port format error\n");
		return CMD_FAILURE;
	}
	cmd_data.fccp_cmd.fccp_data.rule_tuple.ip_p=protocol;
	token=strtok_r((char*)argv[1],colon,&tail_ptr);
	if(token!=NULL)
		cmd_data.fccp_cmd.fccp_data.rule_tuple.ip_src=dcli_ip2ulong(token);
	token=strtok_r(NULL,colon,&tail_ptr);
	if(token!=NULL)
		cmd_data.fccp_cmd.fccp_data.rule_tuple.th_sport=atoi(token);
	token=strtok_r((char*)argv[2],colon,&tail_ptr);
	if(token!=NULL)
		cmd_data.fccp_cmd.fccp_data.rule_tuple.ip_dst=dcli_ip2ulong(token);
	token=strtok_r(NULL,colon,&tail_ptr);
	if(token!=NULL)
		cmd_data.fccp_cmd.fccp_data.rule_tuple.th_dport=atoi(token);
	strncpy(cmd_data.hand_cmd,SE_AGENT_DELETE_RULE,strlen(SE_AGENT_DELETE_RULE));
	ret=sendto_agent(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),vty);
	if(ret<=0)
	{
		vty_out(vty,WRITE_FAIL_STR);
		return CMD_FAILURE;
	}
	memset(&cmd_data,0,sizeof(cmd_data));
	overtime.tv_sec=DCLI_WAIT_TIME;
	ret=read_within_time(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),&overtime);
	if(ret==READ_ERROR)
	{
		vty_out(vty,AGENT_NO_RESPOND_STR);
		return CMD_FAILURE;
	}
	if(cmd_data.fccp_cmd.ret_val!=FCCP_RETURN_OK)
	{
		vty_out(vty,COMMAND_FAIL_STR);
		return CMD_FAILURE;
	}
}


/*dcli command show fast_forward running config*/
DEFUN(show_fast_forward_running_config,
	show_fast_forward_running_config_cmd,
	"show  running-config",	
	SHOW_STR
	"fast_forward Configuration\n"
)
{	
	char showStr[SE_AGENT_RUNNING_CFG_MEM]={0};
	int ret = -1;
	se_interative_t  cmd_data;
	struct timeval overtime;
	memset(&overtime,0,sizeof(overtime));
	memset(&cmd_data,0,sizeof(cmd_data));
	strncpy(cmd_data.hand_cmd,SE_AGENT_SHOW_RUNNING_CFG,strlen(SE_AGENT_SHOW_RUNNING_CFG));
	ret=sendto_agent(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),vty);
	if(ret<=0)
	{
		vty_out(vty,WRITE_FAIL_STR);
		return CMD_FAILURE;
	}
	overtime.tv_sec=DCLI_WAIT_TIME;
	ret=read_within_time(dcli_sockfd,showStr,SE_AGENT_RUNNING_CFG_MEM,&overtime);
	if(ret==READ_ERROR)
	{
		vty_out(vty,AGENT_NO_RESPOND_STR);
		return CMD_FAILURE;
	}
	vty_out(vty, "===============================================================\n");
	if (showStr)
	{
		vty_out(vty, "%s", showStr);
	}
	vty_out(vty, "===============================================================\n");
	return CMD_SUCCESS;
}

/*dcli command show runnig-config fast_forward*/	
DEFUN (show_fast_forward_running_cfg_cmd_func,
	   show_fast_forward_running_cfg_cmd,
	   "show running-config fast-forward",
	   SHOW_STR
	   "Current operating configuration\n"
	   "fast_forward module\n"
)
{
	char tmpstr[64];
	char showStr[SE_AGENT_RUNNING_CFG_MEM]={0};
	int ret = -1;
	se_interative_t  cmd_data;
	struct timeval overtime;
	memset(tmpstr,0,64);
	memset(&overtime,0,sizeof(overtime));
	memset(&cmd_data,0,sizeof(cmd_data));
	sprintf(tmpstr,BUILDING_MOUDLE,"FAST_FORWARD");
	vty_out(vty,"%s\n",tmpstr);
	strncpy(cmd_data.hand_cmd,SE_AGENT_SHOW_RUNNING_CFG,strlen(SE_AGENT_SHOW_RUNNING_CFG));
	ret=sendto_agent(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),vty);
	if(ret<=0)
	{
		vty_out(vty,WRITE_FAIL_STR);
		return CMD_FAILURE;
	}
	overtime.tv_sec=DCLI_WAIT_TIME;
	ret=read_within_time(dcli_sockfd,showStr,SE_AGENT_RUNNING_CFG_MEM,&overtime);
	if(ret==READ_ERROR)
	{
		vty_out(vty,AGENT_NO_RESPOND_STR);
		return CMD_FAILURE;
	}
	vty_out(vty, "%s\n", showStr);
	return CMD_SUCCESS;
}
/*dcli command read register*/ 
DEFUN(read_register_fun,
	  read_register_fun_cmd,
	  "read register ADDR",
	  "Read configuration\n"
	  "read register\n"
	  "register physic address\n"
)
{
	se_interative_t  cmd_data;
	int ret;
	uint64_t addr;
	struct timeval overtime;
	memset(&overtime,0,sizeof(overtime));
	memset(&cmd_data,0,sizeof(cmd_data));
	if((!strncmp(argv[0],"0x",2))||(!strncmp(argv[0],"0X",2)))
	{
		addr=strtoull((char*)argv[0],NULL,16);
	}
	else
	{
		addr=strtoull((char*)argv[0],NULL,10);
	}
	if(0 != (addr % 8))
	{
		vty_out(vty,"Register address is not correct, 8 bytes alignment \n");
		return CMD_FAILURE;
	}
	cmd_data.fccp_cmd.fccp_data.reg_param.address=addr;
	strncpy(cmd_data.hand_cmd,SE_AGENT_READ_REG,strlen(SE_AGENT_READ_REG));
	ret=sendto_agent(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),vty);
	if(ret<=0)
	{
		vty_out(vty,WRITE_FAIL_STR);
		return CMD_FAILURE;
	}
	memset(&cmd_data,0,sizeof(cmd_data));
	overtime.tv_sec=10;
	ret=read_within_time(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),&overtime);
	if(ret==READ_ERROR)
	{
		vty_out(vty,AGENT_NO_RESPOND_STR);
		return CMD_FAILURE;
	}
	vty_out(vty,"%-18s\t\t%-18s\n%#018llx\t\t%#018llx\n","register","data",addr,cmd_data.fccp_cmd.fccp_data.reg_param.reg_data);
	return CMD_SUCCESS;

}

/*dcli command write register*/
DEFUN(write_register_fun,
	  write_register_fun_cmd,
	  "write register ADDR VALUE",
	  "write configuration\n"
	  "write register\n"
	  "write physic address\n"
	  "write register value\n"
)
{
	se_interative_t  cmd_data;
	int ret;
	uint64_t addr;
	uint64_t reg_val;
	struct timeval overtime;
	memset(&overtime,0,sizeof(overtime));
	memset(&cmd_data,0,sizeof(cmd_data));
	if((!strncmp(argv[0],"0x",2))||(!strncmp(argv[0],"0X",2)))
	{
		addr=strtoull((char*)argv[0],NULL,16);
	}
	else
	{
		addr=strtoull((char*)argv[0],NULL,10);
	}
	if(0 != (addr % 8))
	{
		printf("Register address is not correct, 8 bytes alignment \n");
		return CMD_FAILURE;
	}
	if((!strncmp(argv[1],"0x",2))||(!strncmp(argv[1],"0X",2)))
	{
		reg_val=strtoull((char*)argv[1],NULL,16);
	}
	else
	{
		reg_val=strtoull((char*)argv[1],NULL,10);
	}
	cmd_data.fccp_cmd.fccp_data.reg_param.address=addr;
	cmd_data.fccp_cmd.fccp_data.reg_param.reg_data=reg_val;
	strncpy(cmd_data.hand_cmd,SE_AGENT_WRITE_REG,strlen(SE_AGENT_WRITE_REG));
	ret=sendto_agent(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),vty);
	if(ret<=0)
	{
		vty_out(vty,WRITE_FAIL_STR);
		return CMD_FAILURE;
	}
	memset(&cmd_data,0,sizeof(cmd_data));
	overtime.tv_sec=10;
	ret=read_within_time(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),&overtime);
	if(ret==READ_ERROR)
	{
		vty_out(vty,AGENT_NO_RESPOND_STR);
		return CMD_FAILURE;
	}
	if(cmd_data.cmd_result!=AGENT_RETURN_OK)
	{
		vty_out(vty,COMMAND_FAIL_STR);
		return CMD_FAILURE;
	}
	return CMD_SUCCESS;
}
/*dcli command show fau dump_64*/
DEFUN(show_packet_statistic_func,
	  show_packet_statistic_cmd,
	  "show  packet statistic",
	  SHOW_STR
	  "fast_forward  process packet\n"
	  "fast_forward process packet  statistic information\n"
)
{
	  se_interative_t  cmd_data;
	  int ret = -1,i = 0;
	  struct timeval overtime;
	  fau64_info_t  fau64_info;
	  memset(&overtime,0,sizeof(overtime));
	  memset(&cmd_data,0,sizeof(cmd_data));
	  memset(&fau64_info,0,sizeof(fau64_info));
	  strncpy(cmd_data.hand_cmd,SE_AGENT_SHOW_FAU64,strlen(SE_AGENT_SHOW_FAU64));
	  fill_cpu_tag(&cmd_data, vty);
	  ret=sendto_agent(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),vty);
	  if(ret<=0)
	  {
		  vty_out(vty,WRITE_FAIL_STR);
		  return CMD_FAILURE;
	  }
	  overtime.tv_sec=DCLI_WAIT_TIME;
	  ret=read_within_time(dcli_sockfd,(char*)&cmd_data,sizeof(se_interative_t),&overtime);
	  if(ret==READ_ERROR)
	  {
		  vty_out(vty,AGENT_NO_RESPOND_STR);
		  return CMD_FAILURE;
	  }
	  if(cmd_data.cmd_result!=AGENT_RETURN_OK)
      {
    	  vty_out(vty,COMMAND_FAIL_STR);
    	  return CMD_FAILURE;
      }
	  memcpy(&fau64_info, &cmd_data.fccp_cmd.fccp_data.fau64_info, sizeof(fau64_info_t));
	  vty_out(vty,"===========================================================================\n");
	  vty_out(vty,"total ethernet input bytes = %llu\r\n",fau64_info.fau_enet_input_bytes);
	  vty_out(vty,"total ethernet input packets = %llu\r\n", fau64_info.fau_enet_input_packets);
	  vty_out(vty,"total received works counter (packets and fccp) = %llu\r\n", fau64_info.fau_recv_works);
	  vty_out(vty,"total received fccp packets counter = %llu\r\n", fau64_info.fau_recv_fccp);
	  vty_out(vty,"===========================================================================\n");
	  //vty_out(vty,"FAU registers for the position in PKO command buffers = %llu\r\n", fau64_info.fau_reg_oq_addr_index);  
	  vty_out(vty,"total ethernet input noneip packets = %llu\r\n",  fau64_info.fau_enet_nonip_packets);
	  vty_out(vty,"total ethernet input eth pppoe noneip packets = %llu\r\n",  fau64_info.fau_enet_eth_pppoe_nonip_packets);
	  vty_out(vty,"total ethernet input capwap pppoe noneip packets = %llu\r\n",  fau64_info.fau_enet_capwap_pppoe_nonip_packets);
	  vty_out(vty,"total ethernet input error packets = %llu\r\n",  fau64_info.fau_enet_error_packets);
	  vty_out(vty,"total mcast packets = %llu\r\n", fau64_info.fau_mcast_packets);
	  vty_out(vty,"total rpa packets = %llu\r\n", fau64_info.fau_rpa_packets);
	  vty_out(vty,"total rpa to linux packets = %llu\r\n", fau64_info.fau_rpa_tolinux_packets);
	  vty_out(vty,"total tipc packets = %llu\r\n", fau64_info.fau_tipc_packets);
	  vty_out(vty,"total large eth->capwap packets = %llu\r\n", fau64_info.fau_large_eth2cw_packets);
	  vty_out(vty,"total large xxx->cw_rpa packets = %llu\r\n", fau64_info.fau_large_cw_rpa_fwd_packets);
	  vty_out(vty,"total large xxx->cw8023_rpa packets = %llu\r\n", fau64_info.fau_large_cw8023_rpa_fwd_packets);
	  vty_out(vty,"total ethernet input fragip packets = %llu\r\n",  fau64_info.fau_enet_fragip_packets);
	  vty_out(vty,"short IP packets rcvd = %llu\r\n",  fau64_info.fau_ip_short_packets);
	  vty_out(vty,"IP packets with bad hdr len = %llu\r\n",  fau64_info.fau_ip_bad_hdr_len);
	  vty_out(vty,"IP packets with bad len = %llu\r\n",  fau64_info.fau_ip_bad_len);
	  vty_out(vty,"IP packets with bad version = %llu\r\n", fau64_info.fau_ip_bad_version);
	  vty_out(vty,"IP packets with SKIP addr = %llu\r\n",  fau64_info.fau_ip_skip_addr);
	  vty_out(vty,"ICMP packets = %llu\r\n",  fau64_info.fau_ip_icmp);
	  vty_out(vty,"Capwap ICMP packets = %llu\r\n",  fau64_info.fau_capwap_icmp);
	  vty_out(vty,"ip packets with proto error = %llu\r\n",  fau64_info.fau_ip_proto_error);
	  vty_out(vty,"special tcp hearder = %llu\r\n", fau64_info.fau_spe_tcp_hdr);
	  vty_out(vty,"special tcp hearder over capwap = %llu\r\n", fau64_info.fau_cw_spe_tcp_hdr);
	  vty_out(vty,"udp dport=0 packets = %llu\r\n",  fau64_info.fau_udp_bad_dropt);
	  vty_out(vty,"udp packets with len error = %llu\r\n",  fau64_info.fau_udp_bad_len);
	  vty_out(vty,"udp packets that trap to Linux = %llu\r\n",  fau64_info.fau_udp_to_linux);
	  vty_out(vty,"total 802.11 bad head = %llu\r\n",  fau64_info.fau_cw_80211_err);
	  vty_out(vty,"total capwap noip packets = %llu\r\n",  fau64_info.fau_cw_noip_packets);
	  vty_out(vty,"total capwap frag packets = %llu\r\n",  fau64_info.fau_cw_frag_packets);
	  vty_out(vty,"total capwap special ip head = %llu\r\n",  fau64_info.fau_cw_spe_packets);
	  vty_out(vty,"total flow lookup failed counter = %llu\r\n",  fau64_info.fau_flow_lookup_error);
	  vty_out(vty,"total capwap 802.11 decap error = %llu\r\n", fau64_info.fau_cw802_11_decap_err);
	  vty_out(vty,"total capwap 802.3 decap error = %llu\r\n", fau64_info.fau_cw802_3_decap_err);
	  vty_out(vty,"===========================================================================\n");
	  vty_out(vty,"ACL HIT packets number = %llu\r\n",  fau64_info.fau_flowtable_hit_packets);
	  vty_out(vty,"total acl lookup counter = %llu\r\n", fau64_info.fau_acl_lookup);
	  vty_out(vty,"total acl setup and regist packets counter = %llu\r\n", fau64_info.fau_acl_reg);
	  vty_out(vty,"===========================================================================\n");
	  vty_out(vty,"total ethernet to linux packets = %llu\r\n", fau64_info.fau_enet_to_linux_packets);
	  vty_out(vty,"total ethernet to linux bytes = %llu\r\n", fau64_info.fau_enet_to_linux_bytes);
	  vty_out(vty,"total ethernet drop packets = %llu\r\n", fau64_info.fau_enet_drop_packets);
	  vty_out(vty,"total ethernet output 802.1q packets = %llu\r\n",  fau64_info.fau_enet_output_packets_8021q);
	  vty_out(vty,"total ethernet output qinq packets = %llu\r\n",  fau64_info.fau_enet_output_packets_qinq);
	  vty_out(vty,"total ethernet output eth pppoe  packets = %llu\r\n",  fau64_info.fau_enet_output_packets_eth_pppoe);
	  vty_out(vty,"total ethernet output capwap pppoe packets = %llu\r\n",  fau64_info.fau_enet_output_packets_capwap_pppoe);
	  vty_out(vty,"total ethernet output error packets = %llu\r\n",fau64_info.fau_pko_errors);
	  vty_out(vty,"total ethernet output bytes = %llu\r\n", fau64_info.fau_enet_output_bytes);
	  vty_out(vty,"total ethernet output packets = %llu\r\n", fau64_info.fau_enet_output_packets);
	  vty_out(vty,"total rule alloc memory fail counter = %llu\r\n", fau64_info.fau_alloc_rule_fail);
	  vty_out(vty,"total reach max rule entries = %llu\r\n", fau64_info.fau_max_rule_entries);
	  vty_out(vty,"===========================================================================\n");
	  return CMD_SUCCESS;
}

DEFUN(show_part_packet_statistic_func,
	  show_part_packet_statistic_cmd,
	  "show  part packet statistic",
	  SHOW_STR
	  "part of process \n"
	  "fast_forward  process packet\n"
	  "fast_forward process packet  statistic information\n"
)
{
	  se_interative_t  cmd_data;
	  int ret = -1,i = 0;
	  struct timeval overtime;
	  fau64_part_info_t  fau64_part_info;
	  memset(&overtime,0,sizeof(overtime));
	  memset(&cmd_data,0,sizeof(cmd_data));
	  memset(&fau64_part_info,0,sizeof(fau64_part_info));
	  strncpy(cmd_data.hand_cmd,SE_AGENT_SHOW_PART_FAU64,strlen(SE_AGENT_SHOW_PART_FAU64));
	  fill_cpu_tag(&cmd_data, vty);
	  ret=sendto_agent(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),vty);
	  if(ret<=0)
	  {
		  vty_out(vty,WRITE_FAIL_STR);
		  return CMD_FAILURE;
	  }
	  overtime.tv_sec=DCLI_WAIT_TIME;
	  ret=read_within_time(dcli_sockfd,(char*)&cmd_data,sizeof(se_interative_t),&overtime);
	  if(ret==READ_ERROR)
	  {
		  vty_out(vty,AGENT_NO_RESPOND_STR);
		  return CMD_FAILURE;
	  }
	  if(cmd_data.cmd_result!=AGENT_RETURN_OK)
      {
    	  vty_out(vty,COMMAND_FAIL_STR);
    	  return CMD_FAILURE;
      }
	  memcpy(&fau64_part_info, &cmd_data.fccp_cmd.fccp_data.fau64_part_info, sizeof(fau64_part_info_t));
	  vty_out(vty,"===========================================================================\n");
	  vty_out(vty,"total ethernet input eth packets = %llu\r\n",  fau64_part_info.fau_enet_input_packets_eth);
	  vty_out(vty,"total ethernet input capwap packets = %llu\r\n",  fau64_part_info.fau_enet_input_packets_capwap);
	  vty_out(vty,"total ethernet input rpa packets = %llu\r\n",  fau64_part_info.fau_enet_input_packets_rpa);
	  vty_out(vty,"total ethernet output eth packets = %llu\r\n",  fau64_part_info.fau_enet_output_packets_eth);
	  vty_out(vty,"total ethernet output capwap packets = %llu\r\n",  fau64_part_info.fau_enet_output_packets_capwap);
	  vty_out(vty,"total ethernet output rpa packets = %llu\r\n",  fau64_part_info.fau_enet_output_packets_rpa);
	  vty_out(vty,"total ethernet input eth bytes = %llu\r\n",  fau64_part_info.fau_enet_input_bytes_eth);
	  vty_out(vty,"total ethernet input capwap bytes = %llu\r\n",  fau64_part_info.fau_enet_input_bytes_capwap);
	  vty_out(vty,"total ethernet input rpa bytes = %llu\r\n",  fau64_part_info.fau_enet_input_bytes_rpa);
	  vty_out(vty,"total ethernet output eth bytes = %llu\r\n",  fau64_part_info.fau_enet_output_bytes_eth);
	  vty_out(vty,"total ethernet output capwap bytes = %llu\r\n",  fau64_part_info.fau_enet_output_bytes_capwap);
	  vty_out(vty,"total ethernet output rpa bytes = %llu\r\n",  fau64_part_info.fau_enet_output_bytes_rpa);
	  vty_out(vty,"===========================================================================\n");
	  return CMD_SUCCESS;
}


/*dcli command clear fau64  */
DEFUN(  clear_part_packet_statistic_func,
			clear_part_packet_statistic_cmd,
			"clear  part packet statistic",
			CLEAR_STR
			"part of process \n"
			"fast_forward  process packet \n"
	  		"fast_forward process packet  statistic information\n"
)
{
	se_interative_t  cmd_data;
	int ret = -1,i = 0;
	struct timeval overtime;
	memset(&overtime,0,sizeof(overtime));
	memset(&cmd_data,0,sizeof(cmd_data));
	strncpy(cmd_data.hand_cmd,SE_AGENT_CLEAR_PART_FAU64,strlen(SE_AGENT_CLEAR_PART_FAU64));
	fill_cpu_tag(&cmd_data, vty);
	ret=sendto_agent(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),vty);
	if(ret<=0)
	{
		vty_out(vty,WRITE_FAIL_STR);
		return CMD_FAILURE;
	}
	overtime.tv_sec=DCLI_WAIT_TIME;
	ret=read_within_time(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),&overtime);
	if(ret==READ_ERROR)
	{
		vty_out(vty,AGENT_NO_RESPOND_STR);
		return CMD_FAILURE;
	}
	if(cmd_data.cmd_result!=AGENT_RETURN_OK)
	{
		vty_out(vty,"%s\n",cmd_data.err_info);
		return CMD_FAILURE;
	}
	return CMD_SUCCESS;
}

DEFUN(  clear_packet_statistic_func,
			clear_packet_statistic_cmd,
			"clear  packet statistic",
			CLEAR_STR
			"fast_forward  process packet \n"
	  		"fast_forward process packet  statistic information\n"
)
{
	se_interative_t  cmd_data;
	int ret = -1,i = 0;
	struct timeval overtime;
	memset(&overtime,0,sizeof(overtime));
	memset(&cmd_data,0,sizeof(cmd_data));
	strncpy(cmd_data.hand_cmd,SE_AGENT_CLEAR_FAU64,strlen(SE_AGENT_CLEAR_FAU64));
	fill_cpu_tag(&cmd_data, vty);
	ret=sendto_agent(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),vty);
	if(ret<=0)
	{
		vty_out(vty,WRITE_FAIL_STR);
		return CMD_FAILURE;
	}
	overtime.tv_sec=DCLI_WAIT_TIME;
	ret=read_within_time(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),&overtime);
	if(ret==READ_ERROR)
	{
		vty_out(vty,AGENT_NO_RESPOND_STR);
		return CMD_FAILURE;
	}
	if(cmd_data.cmd_result!=AGENT_RETURN_OK)
	{
		vty_out(vty,"%s\n",cmd_data.err_info);
		return CMD_FAILURE;
	}
	return CMD_SUCCESS;
}


DEFUN(fastfwd_learned_icmp_enable_func,
	  fastfwd_learned_icmp_enable_cmd,
	  "config  fast-icmp (enable|disable)",
	  CONFIG_STR
	  "fast icmp  feature\n"
	  "fast_forward learned icmp packet enable\n"
	  "fast_forward learned icmp packet disable\n"
)
{
	
	char *enable=(char *)argv[0];
	int flag = FUNC_DISABLE;
	se_interative_t  cmd_data;
	int ret = -1,i = 0;
	struct timeval overtime;
	memset(&overtime,0,sizeof(overtime));
	memset(&cmd_data,0,sizeof(cmd_data));
	if(argc > 1) 
	{
		vty_out(vty,CMD_PARAMETER_NUM_ERROR);
		return CMD_WARNING;
	}
	if(0==strncmp(enable,"disable",strlen(enable)))
	{
		flag=FUNC_DISABLE;
	}
	else if(0==strncmp(enable,"enable",strlen(enable)))
	{
		flag=FUNC_ENABLE;
	}
	else
	{
		vty_out(vty,CMD_PARAMETER_ERROR);
		return CMD_FAILURE;
	}
	cmd_data.fccp_cmd.fccp_data.module_enable=flag;
	strncpy(cmd_data.hand_cmd,SE_AGENT_ICMP_ENABLE,strlen(SE_AGENT_ICMP_ENABLE));
	ret=sendto_agent(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),vty);
	if(ret<=0)
	{
		vty_out(vty,WRITE_FAIL_STR);
		return CMD_FAILURE;
	}
	overtime.tv_sec=DCLI_WAIT_TIME;
	ret=read_within_time(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),&overtime);
	if(ret==READ_ERROR)
	{
		vty_out(vty,AGENT_NO_RESPOND_STR);
		return CMD_FAILURE;
	}
	if(cmd_data.cmd_result!=AGENT_RETURN_OK)
	{
		vty_out(vty,"%s\n",cmd_data.err_info);
		return CMD_FAILURE;
	}
	return CMD_SUCCESS;
}

DEFUN(fastfwd_pure_ip_enable_func,
	  fastfwd_pure_ip_enable_cmd,
	  "config  fast-pure-ip (enable|disable)",
	  CONFIG_STR
	  "fast pure ip feature\n"
	  "fast_forward pure ip forward enable\n"
	  "fast_forward pure ip forward disable\n"
)
{
	
	char *enable=(char *)argv[0];
	int flag = FUNC_DISABLE;
	se_interative_t  cmd_data;
	int ret = -1,i = 0;
	struct timeval overtime;
	memset(&overtime,0,sizeof(overtime));
	memset(&cmd_data,0,sizeof(cmd_data));
	if(argc > 1) 
	{
		vty_out(vty,CMD_PARAMETER_NUM_ERROR);
		return CMD_WARNING;
	}
	if(0==strncmp(enable,"disable",strlen(enable)))
	{
		flag=FUNC_DISABLE;
	}
	else if(0==strncmp(enable,"enable",strlen(enable)))
	{
		flag=FUNC_ENABLE;
	}
	else
	{
		vty_out(vty,CMD_PARAMETER_ERROR);
		return CMD_FAILURE;
	}
	cmd_data.fccp_cmd.fccp_data.module_enable=flag;
	strncpy(cmd_data.hand_cmd,SE_AGENT_PURE_IP_ENABLE,strlen(SE_AGENT_PURE_IP_ENABLE));
	ret=sendto_agent(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),vty);
	if(ret<=0)
	{
		vty_out(vty,WRITE_FAIL_STR);
		return CMD_FAILURE;
	}
	overtime.tv_sec=DCLI_WAIT_TIME;
	ret=read_within_time(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),&overtime);
	if(ret==READ_ERROR)
	{
		vty_out(vty,AGENT_NO_RESPOND_STR);
		return CMD_FAILURE;
	}
	if(cmd_data.cmd_result!=AGENT_RETURN_OK)
	{
		vty_out(vty,"%s\n",cmd_data.err_info);
		return CMD_FAILURE;
	}
	return CMD_SUCCESS;
}

DEFUN(show_fwd_pure_ip_enable_func,
	  show_fwd_pure_ip_enable_cmd,
	  "show  fast-pure-ip enable",
	  SHOW_STR
	  "fast-pure-ip  \n"
	  "fast_forward pure ip forward whether enable \n"
)
{
	se_interative_t  cmd_data;
	int ret = 0;
	struct timeval overtime;
	memset(&overtime, 0, sizeof(overtime));
	memset(&cmd_data, 0, sizeof(cmd_data));
	strncpy(cmd_data.hand_cmd, SE_AGENT_SHOW_PURE_IP_ENABLE, strlen(SE_AGENT_SHOW_PURE_IP_ENABLE));
	ret = sendto_agent(dcli_sockfd, (char*)&cmd_data, sizeof(cmd_data), vty);
	if(ret <= 0)
	{
		vty_out(vty,WRITE_FAIL_STR);
		return CMD_FAILURE;
	}
	memset(&cmd_data, 0, sizeof(cmd_data));
	overtime.tv_sec = DCLI_WAIT_TIME;
 	ret = read_within_time(dcli_sockfd, (char*)&cmd_data, sizeof(cmd_data), &overtime);
 	if(ret == READ_ERROR)
	{
		vty_out(vty, AGENT_NO_RESPOND_STR);
		return CMD_FAILURE;
	}
	
	if(cmd_data.cmd_result != AGENT_RETURN_OK)
	{
		vty_out(vty, "%s\n", cmd_data.err_info);
		return CMD_FAILURE;
	}
	if(cmd_data.fccp_cmd.ret_val != FCCP_RETURN_OK)
	{
		vty_out(vty, COMMAND_FAIL_STR);
		return CMD_FAILURE;
	}
	vty_out(vty, "fast_forward pure ip forward is %s\n", (cmd_data.fccp_cmd.fccp_data.module_enable == 1) ? "enable ": "disable");

	return CMD_SUCCESS;
}


DEFUN(config_fast_forward_enable_func,
	config_fast_forward_enable_cmd,
	"config fast-forward (enable|disable)",
	CONFIG_STR
	"fast forward module\n"
	"fast forward function enable\n"
	"fast forward function disable\n"
)
{
	char *enable=(char *)argv[0];
	int flag = FUNC_ENABLE;
	se_interative_t  cmd_data;
	int ret = -1,i = 0;
	struct timeval overtime;
	memset(&overtime,0,sizeof(overtime));
	memset(&cmd_data,0,sizeof(cmd_data));
	if(argc > 1) 
	{
		vty_out(vty,CMD_PARAMETER_NUM_ERROR);
		return CMD_WARNING;
	}
	if(0==strncmp(enable,"disable",strlen(enable)))
	{
		flag=FUNC_DISABLE;
	}
	else if(0==strncmp(enable,"enable",strlen(enable)))
	{
		flag=FUNC_ENABLE;
	}
	else
	{
		vty_out(vty,CMD_PARAMETER_ERROR);
		return CMD_WARNING;
	}
	cmd_data.fccp_cmd.fccp_data.module_enable=flag;
	strncpy(cmd_data.hand_cmd,SE_AGENT_FASTFWD_ENABLE,strlen(SE_AGENT_FASTFWD_ENABLE));
	ret=sendto_agent(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),vty);
	if(ret<=0)
	{
		vty_out(vty,WRITE_FAIL_STR);
		return CMD_FAILURE;
	}
	overtime.tv_sec=DCLI_WAIT_TIME;
	ret=read_within_time(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),&overtime);
	if(ret==READ_ERROR)
	{
		vty_out(vty,AGENT_NO_RESPOND_STR);
		return CMD_FAILURE;
	}
	if(cmd_data.cmd_result!=AGENT_RETURN_OK)
	{
		vty_out(vty,"%s\n",cmd_data.err_info);
		return CMD_FAILURE;
	}
	return CMD_SUCCESS;
}


DEFUN(show_rule_five_tuple_func,
	show_rule_five_tuple_cmd,
	"show fast-forward rule PROTOCOL SIP:SPORT DIP:DPORT",
	SHOW_STR
	"fast forward module\n"
	"fastfwd acl rule information\n"
	"protocol (udp | tcp)\n"
	"source ip address and source port A.B.C.D:SPORT\n"
	"destination ip address and destination port A.B.C.D:DPORT\n"
)
{
	se_interative_t  cmd_data;
	int ret = -1;
	unsigned char protocol = INVALID_TYPE;
	struct timeval overtime;
	char *token=NULL;
	char *colon=":";
	char *sport=NULL;
	char *tail_ptr=NULL;
	memset(&overtime,0,sizeof(overtime));
	memset(&cmd_data,0,sizeof(cmd_data));
	protocol=parse_protocol((char *)argv[0]);
	if((TCP_TYPE !=protocol) && (UDP_TYPE != protocol))
	{
		vty_out(vty,"protocol is not tcp or udp\n");
		return CMD_FAILURE;
	}
	if(COMMON_ERROR==parse_ipport_check((char*)argv[1]))
	{
		vty_out(vty,"source ip address and port format error\n");
		return CMD_FAILURE;
	}
	if(COMMON_ERROR==parse_ipport_check((char*)argv[2]))
	{
		vty_out(vty,"destination ip address and port format error\n");
		return CMD_FAILURE;
	}
	cmd_data.fccp_cmd.fccp_data.rule_tuple.ip_p=protocol;
	token=strtok_r((char*)argv[1],colon,&tail_ptr);
	if(token!=NULL)
		cmd_data.fccp_cmd.fccp_data.rule_tuple.ip_src=dcli_ip2ulong(token);
	token=strtok_r(NULL,colon,&tail_ptr);
	if(token!=NULL)
		cmd_data.fccp_cmd.fccp_data.rule_tuple.th_sport=atoi(token);
	token=strtok_r((char*)argv[2],colon,&tail_ptr);
	if(token!=NULL)
		cmd_data.fccp_cmd.fccp_data.rule_tuple.ip_dst=dcli_ip2ulong(token);
	token=strtok_r(NULL,colon,&tail_ptr);
	if(token!=NULL)
		cmd_data.fccp_cmd.fccp_data.rule_tuple.th_dport=atoi(token);
	strncpy(cmd_data.hand_cmd,SE_AGENT_SHOW_FIVE_TUPLE_ACL,strlen(	SE_AGENT_SHOW_FIVE_TUPLE_ACL));
	ret=sendto_agent(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),vty);
	if(ret<=0)
	{
		vty_out(vty,WRITE_FAIL_STR);
		return CMD_FAILURE;
	}
	memset(&cmd_data,0,sizeof(cmd_data));
	overtime.tv_sec=DCLI_WAIT_TIME;
	ret=read_within_time(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),&overtime);
	if(ret==READ_ERROR)
	{
		vty_out(vty,AGENT_NO_RESPOND_STR);
		return CMD_FAILURE;
	}
	
	if(cmd_data.cmd_result!=AGENT_RETURN_OK)
	{
		vty_out(vty,"%s\n", cmd_data.err_info);
		return CMD_FAILURE;
	}
	vty_out(vty,"rule information:\n");
	vty_out(vty,"    %u.%u.%u.%u:%u  ==> %u.%u.%u.%u:%u %s\n",
		IP_FMT(cmd_data.fccp_cmd.fccp_data.rule_info.rule_param.sip),
		cmd_data.fccp_cmd.fccp_data.rule_info.rule_param.sport,
		IP_FMT(cmd_data.fccp_cmd.fccp_data.rule_info.rule_param.dip),
		cmd_data.fccp_cmd.fccp_data.rule_info.rule_param.dport,
		PROTO_STR(cmd_data.fccp_cmd.fccp_data.rule_info.rule_param.protocol));
	if(cmd_data.fccp_cmd.fccp_data.rule_info.rule_param.rule_state == RULE_IS_LEARNING)
	{
		vty_out(vty,"    rule_state = LEARNING    ");
		if(cmd_data.fccp_cmd.fccp_data.rule_info.rule_param.time_stamp==RULE_IS_AGE)
			vty_out(vty,"age\n");
		else if(cmd_data.fccp_cmd.fccp_data.rule_info.rule_param.time_stamp==RULE_IS_NEW)
			vty_out(vty,"new\n");
		else 
			return CMD_SUCCESS;
		return CMD_SUCCESS;	
	}
	if(cmd_data.fccp_cmd.fccp_data.rule_info.rule_param.action_type == FLOW_ACTION_DROP)
	{
		vty_out(vty,"    action_type = FLOW_ACTION_DROP\n");
		return CMD_SUCCESS;
	}
	if(cmd_data.fccp_cmd.fccp_data.rule_info.rule_param.action_type == FLOW_ACTION_TOLINUX)
	{
		vty_out(vty,"    action_type = FLOW_ACTION_TOLINUX\n");
		return CMD_SUCCESS;
	}

	vty_out(vty,"    smac: %02x-%02x-%02x-%02x-%02x-%02x", MAC_FMT(cmd_data.fccp_cmd.fccp_data.rule_info.rule_param.ether_shost));
	vty_out(vty,"    dmac: %02x-%02x-%02x-%02x-%02x-%02x\n", MAC_FMT(cmd_data.fccp_cmd.fccp_data.rule_info.rule_param.ether_dhost));
	vty_out(vty,"    eth protocol: %04x\n", cmd_data.fccp_cmd.fccp_data.rule_info.rule_param.ether_type);

	switch(cmd_data.fccp_cmd.fccp_data.rule_info.rule_param.action_type)
	{
		case FLOW_ACTION_ETH_FORWARD:
			vty_out(vty,"    action_type = FLOW_ACTION_ETH_FORWARD\n");
			break;
		case FLOW_ACTION_CAP802_3_FORWARD:
			vty_out(vty,"    action_type = FLOW_ACTION_CAP802_3_FORWARD\n");
			vty_out(vty,"    capwap use_num = %d\n", cmd_data.fccp_cmd.fccp_data.rule_info.cw_cache.use_num);
			vty_out(vty,"    capwap tunnel: %d.%d.%d.%d:%d => %d.%d.%d.%d:%d  tos = 0x%02x\n",
					IP_FMT(cmd_data.fccp_cmd.fccp_data.rule_info.cw_cache.sip), cmd_data.fccp_cmd.fccp_data.rule_info.cw_cache.sport,
					IP_FMT(cmd_data.fccp_cmd.fccp_data.rule_info.cw_cache.dip), cmd_data.fccp_cmd.fccp_data.rule_info.cw_cache.dport,  
					cmd_data.fccp_cmd.fccp_data.rule_info.cw_cache.tos);
			break;
		case FLOW_ACTION_CAPWAP_FORWARD:
			vty_out(vty,"    action_type = FLOW_ACTION_CAPWAP_FORWARD\n");
			vty_out(vty,"    capwap use_num = %d\n", cmd_data.fccp_cmd.fccp_data.rule_info.cw_cache.use_num);
			vty_out(vty,"    capwap tunnel: %d.%d.%d.%d:%d => %d.%d.%d.%d:%d  tos = 0x%02x\n",
					IP_FMT(cmd_data.fccp_cmd.fccp_data.rule_info.cw_cache.sip), cmd_data.fccp_cmd.fccp_data.rule_info.cw_cache.sport,
					IP_FMT(cmd_data.fccp_cmd.fccp_data.rule_info.cw_cache.dip), cmd_data.fccp_cmd.fccp_data.rule_info.cw_cache.dport,  
					cmd_data.fccp_cmd.fccp_data.rule_info.cw_cache.tos);
			break;
		default:
			vty_out(vty,"    action_type = UNKNOWN\n");
			break;
	}
	vty_out(vty,"    forward port = %d\n", cmd_data.fccp_cmd.fccp_data.rule_info.rule_param.forward_port);
	if(cmd_data.fccp_cmd.fccp_data.rule_info.rule_param.time_stamp == RULE_IS_AGE)
		vty_out(vty,"   age \n");
	else
		vty_out(vty,"   new \n");
	if(cmd_data.fccp_cmd.fccp_data.rule_info.rule_param.rule_state == RULE_IS_STATIC)
	{
		vty_out(vty,"  rule_state = STATIC\n");
	}
	else
	{
		if(cmd_data.fccp_cmd.fccp_data.rule_info.rule_param.rule_state == RULE_IS_LEARNED)
		{
			vty_out(vty,"    rule_state = LEARNED\t");
		}
	}
	vty_out(vty,"    dsa_info: 0x%08x\n", cmd_data.fccp_cmd.fccp_data.rule_info.rule_param.dsa_info);
	vty_out(vty,"    out_type:0x%02x   out_tag:0x%02x   in_type:0x%02x   in_tag:0x%02x\n",
					cmd_data.fccp_cmd.fccp_data.rule_info.rule_param.out_ether_type, 
					cmd_data.fccp_cmd.fccp_data.rule_info.rule_param.out_tag, 
					cmd_data.fccp_cmd.fccp_data.rule_info.rule_param.in_ether_type, 
					cmd_data.fccp_cmd.fccp_data.rule_info.rule_param.in_tag);
	vty_out(vty,"    action mask = 0x%x\n", cmd_data.fccp_cmd.fccp_data.rule_info.rule_param.action_mask);
	return CMD_SUCCESS;
}
DEFUN(show_rule_stats_func,
	show_rule_stats_cmd,
	"show fast-forward rule statistic",
	SHOW_STR
	"fast forward module\n"
	"fastfwd acl rule information\n"
	"fastfwd acl rule statistic information\n"
)
{
	se_interative_t  cmd_data;
	int ret;
	struct timeval overtime;
	memset(&overtime,0,sizeof(overtime));
	memset(&cmd_data,0,sizeof(cmd_data));
	strncpy(cmd_data.hand_cmd,SE_AGENT_SHOW_ACL_STATS,strlen(SE_AGENT_SHOW_ACL_STATS));
	fill_cpu_tag(&cmd_data, vty);
	ret=sendto_agent(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),vty);
	if(ret<=0)
	{
		vty_out(vty,WRITE_FAIL_STR);
		return CMD_FAILURE;
	}
	memset(&cmd_data,0,sizeof(cmd_data));
	overtime.tv_sec=15;
	ret=read_within_time(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),&overtime);
	if(ret==READ_ERROR)
	{
		vty_out(vty,AGENT_NO_RESPOND_STR);
		return CMD_FAILURE;
	}
	
	if(cmd_data.cmd_result!=AGENT_RETURN_OK)
	{
		vty_out(vty,"%s\n", cmd_data.err_info);
		return CMD_FAILURE;
	}
	vty_out(vty,"==============================================\n");
	vty_out(vty,"acl_bucket_tbl count:\n");
	vty_out(vty,"  entry num: %u\n", cmd_data.fccp_cmd.fccp_data.rule_sum.acl_static_tbl_size);
	vty_out(vty,"  free num: %u\n", (cmd_data.fccp_cmd.fccp_data.rule_sum.acl_static_tbl_size - \
		                               cmd_data.fccp_cmd.fccp_data.rule_sum.s_tbl_used_rule));
	vty_out(vty,"  used num: %u\n", cmd_data.fccp_cmd.fccp_data.rule_sum.s_tbl_used_rule);
	vty_out(vty,"  aged rules num: %u\n", cmd_data.fccp_cmd.fccp_data.rule_sum.s_tbl_aged_rule);
	vty_out(vty,"  new rules num: %u\n", (cmd_data.fccp_cmd.fccp_data.rule_sum.s_tbl_used_rule - \
		                                 cmd_data.fccp_cmd.fccp_data.rule_sum.s_tbl_aged_rule));
	vty_out(vty,"  use rate: %02f%%\n", (float)(cmd_data.fccp_cmd.fccp_data.rule_sum.s_tbl_used_rule - \
		                                   cmd_data.fccp_cmd.fccp_data.rule_sum.s_tbl_aged_rule)/\
		                                   (float)cmd_data.fccp_cmd.fccp_data.rule_sum.acl_static_tbl_size *100);
	vty_out(vty,"  learned rules num: %u\n", cmd_data.fccp_cmd.fccp_data.rule_sum.s_tbl_learned_rule);
	vty_out(vty,"  learning rules num: %u\n", cmd_data.fccp_cmd.fccp_data.rule_sum.s_tbl_learning_rule);
	vty_out(vty,"  static insert rules num: %u\n", cmd_data.fccp_cmd.fccp_data.rule_sum.s_tbl_static_rule);

	vty_out(vty,"==============================================\n");
	vty_out(vty,"acl_dynamic_tbl count:\n");
	vty_out(vty,"  entry num: %u\n", cmd_data.fccp_cmd.fccp_data.rule_sum.acl_dynamic_tbl_size);
	vty_out(vty,"  free num: %u\n", (cmd_data.fccp_cmd.fccp_data.rule_sum.acl_dynamic_tbl_size - \
		                             cmd_data.fccp_cmd.fccp_data.rule_sum.d_tbl_used_rule));
	vty_out(vty,"  used num: %u\n", cmd_data.fccp_cmd.fccp_data.rule_sum.d_tbl_used_rule);
	vty_out(vty,"  aged rules num: %u\n", cmd_data.fccp_cmd.fccp_data.rule_sum.d_tbl_aged_rule);
	vty_out(vty,"  new rules num: %u\n", (cmd_data.fccp_cmd.fccp_data.rule_sum.d_tbl_used_rule - \
		                                 cmd_data.fccp_cmd.fccp_data.rule_sum.d_tbl_aged_rule));
	vty_out(vty,"  use rate: %02f%%\n", (float)(cmd_data.fccp_cmd.fccp_data.rule_sum.d_tbl_used_rule - \
		                                    cmd_data.fccp_cmd.fccp_data.rule_sum.d_tbl_aged_rule) /\
		                                    (float)cmd_data.fccp_cmd.fccp_data.rule_sum.acl_dynamic_tbl_size *100);
	vty_out(vty,"  learned rules num: %u\n", cmd_data.fccp_cmd.fccp_data.rule_sum.d_tbl_learned_rule);
	vty_out(vty,"  learning rules num: %u\n", cmd_data.fccp_cmd.fccp_data.rule_sum.d_tbl_learning_rule);
	vty_out(vty,"  static insert rules num: %u\n", cmd_data.fccp_cmd.fccp_data.rule_sum.d_tbl_static_rule);

	vty_out(vty,"==============================================\n");
	vty_out(vty,"capwap cache table:\n");
	vty_out(vty,"  capwap table entry num: %u\n", cmd_data.fccp_cmd.fccp_data.rule_sum.capwap_cache_tbl_size);
	vty_out(vty,"  capwap table used entry num: %u\n", cmd_data.fccp_cmd.fccp_data.rule_sum.cw_tbl_used);
	vty_out(vty,"  capwap table 802.3 entry num: %u\n", cmd_data.fccp_cmd.fccp_data.rule_sum.cw_tbl_802_3_num);
	vty_out(vty,"  capwap table 802.11 entry num: %u\n", cmd_data.fccp_cmd.fccp_data.rule_sum.cw_tbl_802_11_num);
	vty_out(vty,"==============================================\n");
	return CMD_SUCCESS;
}
DEFUN(clear_rule_all_func,
	clear_rule_all_cmd,
	"clear fast-forward rule all",
	CLEAR_STR
	"fastfwd module\n"
	"fastfwd acl information\n"
	"all fastfwd rule information\n"
	)
{
	se_interative_t  cmd_data;
	int ret;
	struct timeval overtime;
	memset(&overtime,0,sizeof(overtime));
	memset(&cmd_data,0,sizeof(cmd_data));
	strncpy(cmd_data.hand_cmd,SE_AGENT_CLEAR_RULE_ALL,strlen(SE_AGENT_CLEAR_RULE_ALL));
	fill_cpu_tag(&cmd_data, vty);
	ret=sendto_agent(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),vty);
	if(ret<=0)
	{
		vty_out(vty,WRITE_FAIL_STR);
		return CMD_FAILURE;
	}
	memset(&cmd_data,0,sizeof(cmd_data));
	overtime.tv_sec=15;
	ret=read_within_time(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),&overtime);
	if(ret==READ_ERROR)
	{
		vty_out(vty,AGENT_NO_RESPOND_STR);
		return CMD_FAILURE;
	}
	if(cmd_data.cmd_result!=AGENT_RETURN_OK)
	{
		vty_out(vty,"%s\n",cmd_data.err_info);
		return CMD_FAILURE;
	}
	if(cmd_data.fccp_cmd.ret_val!=FCCP_RETURN_OK)
	{
		vty_out(vty,COMMAND_FAIL_STR);
		return CMD_FAILURE;
	}
	return CMD_SUCCESS;
}
DEFUN(show_capwap_tbl_func,
	show_capwap_tbl_cmd,
	"show fast-forward rule capwap",
	SHOW_STR
	"fast forward module\n"
	"fastfwd capwap table\n"
	"fastfwd capwap table\n"
)
{
	se_interative_t  cmd_data;
	int ret;
	struct timeval overtime;
	uint32_t cw_cache_index=0;
	uint32_t cw_cache_cnt=0;
	int loop=0,j=0,i=0;
	char str[2]={0};
	memset(&overtime,0,sizeof(overtime));
	memset(&cmd_data,0,sizeof(cmd_data));
	cmd_data.fccp_cmd.fccp_data.cw_cache_info.cw_cache_index=cw_cache_index;
	strncpy(cmd_data.hand_cmd,SE_AGENT_SHOW_CAPWAP,strlen(SE_AGENT_SHOW_CAPWAP));
	ret = sendto_agent(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),vty);
	if(ret<=0)
	{
		vty_out(vty,WRITE_FAIL_STR);
		return CMD_FAILURE;
	}
	memset(&cmd_data,0,sizeof(cmd_data));
	overtime.tv_sec=DCLI_WAIT_TIME;
	ret=read_within_time(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),&overtime);
	if(ret==READ_ERROR)
	{
		vty_out(vty,AGENT_NO_RESPOND_STR);
		return CMD_FAILURE;
	}
	if(cmd_data.cmd_result!=AGENT_RETURN_OK)
	{
		vty_out(vty,"%s\n",cmd_data.err_info);
		return CMD_FAILURE;
	}
	if(cw_cache_index == 0)
	{
		cw_cache_cnt=cmd_data.fccp_cmd.fccp_data.cw_cache_info.cw_use_cnt;
	}
	vty_out(vty,"capwap cache total count:%d\n",cw_cache_cnt);
	if(cw_cache_cnt == 0)
		return CMD_SUCCESS;
	vty_out(vty,"=====================================================\n");
display_loop:
	if(cw_cache_cnt < DISPLAY_CAPWAP_CNT)
		loop = cw_cache_cnt;
	else
		loop = DISPLAY_CAPWAP_CNT;
	for(i=0;i<loop;i++)
	{
		memset(&overtime,0,sizeof(overtime));
		memset(&cmd_data,0,sizeof(cmd_data));
		cmd_data.fccp_cmd.fccp_data.cw_cache_info.cw_cache_index=cw_cache_index;
		strncpy(cmd_data.hand_cmd,SE_AGENT_SHOW_CAPWAP,strlen(SE_AGENT_SHOW_CAPWAP));
		ret=sendto_agent(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),vty);
		if(ret<=0)
		{
			vty_out(vty,WRITE_FAIL_STR);
			return CMD_FAILURE;
		}
		memset(&cmd_data,0,sizeof(cmd_data));
		overtime.tv_sec=DCLI_WAIT_TIME;
		ret=read_within_time(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),&overtime);
		if(ret==READ_ERROR)
		{
			vty_out(vty,AGENT_NO_RESPOND_STR);
			return CMD_FAILURE;
		}
		if(cmd_data.cmd_result!=AGENT_RETURN_OK)
		{
			vty_out(vty,"s\n",cmd_data.err_info);
			return CMD_FAILURE;
		}
		cw_cache_index = cmd_data.fccp_cmd.fccp_data.cw_cache_info.cw_cache_index;
		vty_out(vty,"capwap tunnel: %d.%d.%d.%d:%d => %d.%d.%d.%d:%d  tos = 0x%02x\n",
					IP_FMT(cmd_data.fccp_cmd.fccp_data.cw_cache_info.cw_cache_rule.sip), cmd_data.fccp_cmd.fccp_data.cw_cache_info.cw_cache_rule.sport,
					IP_FMT(cmd_data.fccp_cmd.fccp_data.cw_cache_info.cw_cache_rule.dip), cmd_data.fccp_cmd.fccp_data.cw_cache_info.cw_cache_rule.dport,  
					cmd_data.fccp_cmd.fccp_data.cw_cache_info.cw_cache_rule.tos);
		vty_out(vty,"    capwap use_num = %d\n", cmd_data.fccp_cmd.fccp_data.cw_cache_info.cw_cache_rule.use_num);
		vty_out(vty,"capwap header:\n");
		for(j = 0; j < CW_H_LEN; j++)
		{
			vty_out(vty,"0x%x  ", cmd_data.fccp_cmd.fccp_data.cw_cache_info.cw_cache_rule.cw_hd[j]);
		}
		vty_out(vty,"\n\n");	
	}
	cw_cache_cnt -= loop;
	if(cw_cache_cnt == 0)
		return CMD_SUCCESS;
	else
	{
		vty_out(vty,"c[continue] q[quit]\n");
		while(1)
		{
			int flag=getchar();
			if(flag== (int)('c'))
				goto display_loop;
			else if(flag == (int)('q'))
				return CMD_SUCCESS;
			else
				continue;
		}
	}
	return CMD_SUCCESS;
}
DEFUN(clear_aging_rule_func,
	clear_aging_rule_cmd,
	"clear fast-forward aging-rule",
	CLEAR_STR
	"fastfwd module\n"
	"fastfwd aging rule \n"
	)
{
	se_interative_t  cmd_data;
	int ret = -1;

	memset(&cmd_data,0,sizeof(cmd_data));
	strncpy(cmd_data.hand_cmd,SE_AGENT_CLEAR_AGING_RULE,strlen(SE_AGENT_CLEAR_AGING_RULE));
	ret=sendto_agent(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),vty);
	if(ret<=0)
	{
		vty_out(vty,WRITE_FAIL_STR);
		return CMD_FAILURE;
	}

	return CMD_SUCCESS;
}
DEFUN(show_aging_rule_cnt_func,
	show_aging_rule_cnt_cmd,
	"show fast-forward aging-rule-cnt <1-100000>",
	SHOW_STR
	"fast forward module\n"
	"aging rule cnt\n"
	"time (minute 1-100000)\n"
)
{
	se_interative_t  cmd_data;
	int ret;
	struct timeval overtime;
	memset(&overtime,0,sizeof(overtime));
	memset(&cmd_data,0,sizeof(cmd_data));
	cmd_data.fccp_cmd.fccp_data.aging_timer=strtoul((char*)argv[0],NULL,10);
	if((cmd_data.fccp_cmd.fccp_data.aging_timer < 0) || (cmd_data.fccp_cmd.fccp_data.aging_timer > 100000))
		vty_out(vty,CMD_PARAMETER_ERROR);
	
	strncpy(cmd_data.hand_cmd,SE_AGENT_SHOW_AGING_RULE_CNT,strlen(SE_AGENT_SHOW_AGING_RULE_CNT));
	ret=sendto_agent(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),vty);
	if(ret<=0)
	{
		vty_out(vty,WRITE_FAIL_STR);
		return CMD_FAILURE;
	}
	memset(&cmd_data,0,sizeof(cmd_data));
	overtime.tv_sec=DCLI_WAIT_TIME;
	ret=read_within_time(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),&overtime);
	if(ret==READ_ERROR)
	{
		vty_out(vty,AGENT_NO_RESPOND_STR);
		return CMD_FAILURE;
	}
	if(cmd_data.cmd_result!=AGENT_RETURN_OK)
	{
		vty_out(vty,"%s\n",cmd_data.err_info);
		return CMD_FAILURE;
	}
	vty_out(vty,"The number of total aging rule   :%llu\n",cmd_data.fccp_cmd.fccp_data.rule_aging_cnt.static_aging_cnt + \
															cmd_data.fccp_cmd.fccp_data.rule_aging_cnt.dynamic_aging_cnt);
	vty_out(vty,"The number of dynamic aging rule :%llu\n", cmd_data.fccp_cmd.fccp_data.rule_aging_cnt.dynamic_aging_cnt);
	vty_out(vty,"The number of static aging rule  :%llu\n", cmd_data.fccp_cmd.fccp_data.rule_aging_cnt.static_aging_cnt);
	
	return CMD_SUCCESS;
}

DEFUN(show_user_acl_stats_func,
	show_user_acl_stats_cmd,
	"show fast-forward user-rule-statistic A.B.C.D",
	SHOW_STR
	"fast forward module\n"
	"user statistics used acl rule \n"
	"user ip address\n"
)
{
	se_interative_t  cmd_data;
	int ret;
	struct timeval overtime;
	memset(&overtime,0,sizeof(overtime));
	memset(&cmd_data,0,sizeof(cmd_data));
	
	if(COMMON_ERROR==parse_ip_check((char*)argv[0]))
	{
		vty_out(vty,CMD_PARAMETER_ERROR);
		return CMD_FAILURE;
	}
	cmd_data.fccp_cmd.fccp_data.rule_tuple.ip_src=dcli_ip2ulong((char*)argv[0]);
	strncpy(cmd_data.hand_cmd,SE_AGENT_SHOW_USER_ACL,strlen(SE_AGENT_SHOW_USER_ACL));
	ret=sendto_agent(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),vty);
	if(ret<=0)
	{
		vty_out(vty,WRITE_FAIL_STR);
		return CMD_FAILURE;
	}
	memset(&cmd_data,0,sizeof(cmd_data));
	overtime.tv_sec=DCLI_WAIT_TIME;
	ret=read_within_time(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),&overtime);
	if(ret==READ_ERROR)
	{
		vty_out(vty,AGENT_NO_RESPOND_STR);
		return CMD_FAILURE;
	}
	if(cmd_data.cmd_result!=AGENT_RETURN_OK)
	{
		vty_out(vty,"%s\n",cmd_data.err_info);
		return CMD_FAILURE;
	}
	vty_out(vty,"==============================================\n");
	vty_out(vty,"  total used num     : %lu,    (static: %lu, dynamic: %lu)\n", cmd_data.fccp_cmd.fccp_data.rule_sum.s_tbl_used_rule + cmd_data.fccp_cmd.fccp_data.rule_sum.d_tbl_used_rule,
		                                         cmd_data.fccp_cmd.fccp_data.rule_sum.s_tbl_used_rule, cmd_data.fccp_cmd.fccp_data.rule_sum.d_tbl_used_rule);
	vty_out(vty,"  uplink rules num   : %lu,    (static: %lu, dynamic: %lu)\n", cmd_data.fccp_cmd.fccp_data.rule_sum.s_tbl_uplink_rule + cmd_data.fccp_cmd.fccp_data.rule_sum.d_tbl_uplink_rule, 
		                                         cmd_data.fccp_cmd.fccp_data.rule_sum.s_tbl_uplink_rule, cmd_data.fccp_cmd.fccp_data.rule_sum.d_tbl_uplink_rule);
	vty_out(vty,"  downlink rules num : %lu,    (static: %lu, dynamic: %lu)\n", cmd_data.fccp_cmd.fccp_data.rule_sum.s_tbl_downlink_rule + cmd_data.fccp_cmd.fccp_data.rule_sum.d_tbl_downlink_rule, 
	                                         cmd_data.fccp_cmd.fccp_data.rule_sum.s_tbl_downlink_rule, cmd_data.fccp_cmd.fccp_data.rule_sum.d_tbl_downlink_rule);
	vty_out(vty,"  new rules num      : %lu,    (static: %lu, dynamic: %lu)\n", 
		(cmd_data.fccp_cmd.fccp_data.rule_sum.s_tbl_used_rule - cmd_data.fccp_cmd.fccp_data.rule_sum.s_tbl_aged_rule + cmd_data.fccp_cmd.fccp_data.rule_sum.d_tbl_used_rule - cmd_data.fccp_cmd.fccp_data.rule_sum.d_tbl_aged_rule),
		                                                                         cmd_data.fccp_cmd.fccp_data.rule_sum.s_tbl_used_rule - cmd_data.fccp_cmd.fccp_data.rule_sum.s_tbl_aged_rule, 
		                                                                         cmd_data.fccp_cmd.fccp_data.rule_sum.d_tbl_used_rule - cmd_data.fccp_cmd.fccp_data.rule_sum.d_tbl_aged_rule);
	vty_out(vty,"  aged rules num     : %lu,    (static: %lu, dynamic: %lu)\n", cmd_data.fccp_cmd.fccp_data.rule_sum.s_tbl_aged_rule+cmd_data.fccp_cmd.fccp_data.rule_sum.d_tbl_aged_rule,
		                                         cmd_data.fccp_cmd.fccp_data.rule_sum.s_tbl_aged_rule, cmd_data.fccp_cmd.fccp_data.rule_sum.d_tbl_aged_rule);
	vty_out(vty,"  learned rules num  : %lu,    (static: %lu, dynamic: %lu)\n", cmd_data.fccp_cmd.fccp_data.rule_sum.s_tbl_learned_rule+cmd_data.fccp_cmd.fccp_data.rule_sum.d_tbl_learned_rule, 
		                                         cmd_data.fccp_cmd.fccp_data.rule_sum.s_tbl_learned_rule, 
		                                         cmd_data.fccp_cmd.fccp_data.rule_sum.d_tbl_learned_rule);
	vty_out(vty,"  learning rules num : %lu,    (static: %lu, dynamic: %lu)\n", cmd_data.fccp_cmd.fccp_data.rule_sum.s_tbl_learning_rule+cmd_data.fccp_cmd.fccp_data.rule_sum.d_tbl_learning_rule,
		                                         cmd_data.fccp_cmd.fccp_data.rule_sum.s_tbl_learning_rule, cmd_data.fccp_cmd.fccp_data.rule_sum.d_tbl_learning_rule);
	
	return CMD_SUCCESS;
}
DEFUN(show_tolinux_flow_func,
	show_tolinux_flow_cmd,
	"show fast-forward to-linux-flow ",
	SHOW_STR
	"fast forward moudle\n"
	"fastfwd to linux flow statistic\n"
)
{
	se_interative_t  cmd_data;
	int ret=0;
	struct timeval overtime;
	memset(&overtime,0,sizeof(overtime));
	memset(&cmd_data,0,sizeof(cmd_data));
	
	strncpy(cmd_data.hand_cmd,SE_AGENT_SHOW_TOLINUX_FLOW,strlen(SE_AGENT_SHOW_TOLINUX_FLOW));
	ret=sendto_agent(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),vty);
	if(ret<=0)
	{
		vty_out(vty,WRITE_FAIL_STR);
		return CMD_FAILURE;
	}
	memset(&cmd_data,0,sizeof(cmd_data));
	overtime.tv_sec=DCLI_WAIT_TIME;
	ret=read_within_time(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),&overtime);
	if(ret==READ_ERROR)
	{
		vty_out(vty,AGENT_NO_RESPOND_STR);
		return CMD_FAILURE;
	}
	if(cmd_data.cmd_result!=AGENT_RETURN_OK)
	{
		vty_out(vty,"%s\n",cmd_data.err_info);
		return CMD_FAILURE;
	}
	vty_out(vty,"To linux flow  %llu bps\n",cmd_data.fccp_cmd.fccp_data.to_linux_flow.to_linux_bps);
	vty_out(vty,"To linux flow  %llu pps\n",cmd_data.fccp_cmd.fccp_data.to_linux_flow.to_linux_pps);
	return CMD_SUCCESS;
}

/*dcli command set fwd aging_time*/
DEFUN(set_fastfwd_bucket_entry_func,
		  set_fastfwd_bucket_entry_cmd,
		  "config fast-forward bucket-entry  <2-32767>",
		  CONFIG_STR
		  "fast forward module\n"
		  "fast forward rule bucket entry count\n"
		  "fast forward rule bucket entry count\n"
)
{
	se_interative_t  cmd_data;
	int ret;
	struct timeval overtime;
	memset(&overtime,0,sizeof(overtime));
	memset(&cmd_data,0,sizeof(cmd_data));
	cmd_data.fccp_cmd.fccp_data.bucket_max_entry=strtoul((char*)argv[0],NULL,10);
	strncpy(cmd_data.hand_cmd,SE_AGENT_SET_BUCKET_ENTRY,strlen(SE_AGENT_SET_BUCKET_ENTRY));
	ret=sendto_agent(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),vty);
	if(ret<=0)
	{
		vty_out(vty,WRITE_FAIL_STR);
		return CMD_FAILURE;
	}
	memset(&cmd_data,0,sizeof(cmd_data));
	overtime.tv_sec=DCLI_WAIT_TIME;
	ret=read_within_time(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),&overtime);
	if(ret==READ_ERROR)
	{
		vty_out(vty,AGENT_NO_RESPOND_STR);
		return CMD_FAILURE;
	}
	if(cmd_data.cmd_result!=AGENT_RETURN_OK)
	{
		vty_out(vty,"%s\n",cmd_data.err_info);
		return CMD_FAILURE;
	}
	if(cmd_data.fccp_cmd.ret_val!=FCCP_RETURN_OK)
	{
		vty_out(vty,COMMAND_FAIL_STR);
		return CMD_FAILURE;
	}
	return CMD_SUCCESS;
}

/*dcli command show  fast_forward aging_time*/
DEFUN(show_fastfwd_bucket_entry_func,
		  show_fastfwd_bucket_entry_cmd,
		  "show  fast-forward bucket-entry ",
		  SHOW_STR
		  "fast forward module\n"
		  "fast forward rule bucket entry count\n"
)
{
	se_interative_t  cmd_data;
	int ret;
	struct timeval overtime;
	memset(&overtime,0,sizeof(overtime));
	memset(&cmd_data,0,sizeof(cmd_data));
	strncpy(cmd_data.hand_cmd,SE_AGENT_SHOW_BUCKET_ENTRY,strlen(SE_AGENT_SHOW_BUCKET_ENTRY));
	ret=sendto_agent(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),vty);
	if(ret<=0)
	{
		vty_out(vty,WRITE_FAIL_STR);
		return CMD_FAILURE;
	}
	memset(&cmd_data,0,sizeof(cmd_data));
	overtime.tv_sec=DCLI_WAIT_TIME;
	ret=read_within_time(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),&overtime);
	if(ret==READ_ERROR)
	{
		vty_out(vty,AGENT_NO_RESPOND_STR);
		return CMD_FAILURE;
	}
	if(cmd_data.cmd_result!=AGENT_RETURN_OK)
	{
		vty_out(vty,"%s\n",cmd_data.err_info);
		return CMD_FAILURE;
	}
	if(cmd_data.fccp_cmd.ret_val!=FCCP_RETURN_OK)
	{
		vty_out(vty,COMMAND_FAIL_STR);
		return CMD_FAILURE;
	}
	vty_out(vty,"fastfwd rule bucket entry %lu\n",cmd_data.fccp_cmd.fccp_data.bucket_max_entry);
	return CMD_SUCCESS;
}

void dcli_show_fastfwd_rule_rpa(se_interative_t  *cmd_data,struct vty *vty)
{
	vty_out(vty,"    rap header :\n");
	vty_out(vty,"    rpa_type : %u,rpa_d_s_slotNum : 0x%x\n",\
		cmd_data->fccp_cmd.fccp_data.acl_info.acl_param.rpa_header.rpa_type,\
		cmd_data->fccp_cmd.fccp_data.acl_info.acl_param.rpa_header.rpa_d_s_slotNum);
	vty_out(vty,"    rpa_dnetdevNum : %u,rpa_snetdevNum : %u\n\n",\
		cmd_data->fccp_cmd.fccp_data.acl_info.acl_param.rpa_header.rpa_dnetdevNum,\
		cmd_data->fccp_cmd.fccp_data.acl_info.acl_param.rpa_header.rpa_snetdevNum);
	return;
}
void dcli_show_fastfwd_rule(se_interative_t  *cmd_data,struct vty *vty)
{

	vty_out(vty,"rule information:\n");
		
	vty_out(vty,"    %u.%u.%u.%u:%u  ==> %u.%u.%u.%u:%u %s\n",
		IP_FMT(cmd_data->fccp_cmd.fccp_data.acl_info.acl_param.sip),
		cmd_data->fccp_cmd.fccp_data.acl_info.acl_param.sport,
		IP_FMT(cmd_data->fccp_cmd.fccp_data.acl_info.acl_param.dip),
		cmd_data->fccp_cmd.fccp_data.acl_info.acl_param.dport,
		PROTO_STR(cmd_data->fccp_cmd.fccp_data.acl_info.acl_param.protocol));

	if(cmd_data->fccp_cmd.fccp_data.acl_info.acl_param.rule_state == RULE_IS_LEARNING)
	{
		vty_out(vty,"    rule_state = LEARNING\t");
	
		if(cmd_data->fccp_cmd.fccp_data.acl_info.acl_param.time_stamp == RULE_IS_AGE)
			vty_out(vty,"    age\n");
		else if(cmd_data->fccp_cmd.fccp_data.acl_info.acl_param.time_stamp == RULE_IS_NEW)
			vty_out(vty,"    new\n");
		else
			vty_out(vty,"unknow\n");
		return ;
	}
	else if(cmd_data->fccp_cmd.fccp_data.acl_info.acl_param.rule_state == RULE_IS_LEARNED)
	{
		vty_out(vty,"    rule_state = LEARNED\n");
		switch(cmd_data->fccp_cmd.fccp_data.acl_info.acl_param.action_type)
		{
			case FLOW_ACTION_DROP:
				vty_out(vty,"    action_type = FLOW_ACTION_DROP\n");
				return;
			case FLOW_ACTION_TOLINUX:
				vty_out(vty,"    action_type = FLOW_ACTION_TOLINUX\n");
				return;
			case FLOW_ACTION_ICMP:
				vty_out(vty,"  action_type = FLOW_ACTION_ICMP\n");
				return;
			case FLOW_ACTION_CAPWAP_802_11_ICMP:
				vty_out(vty,"  action_type = FLOW_ACTION_CAPWAP_802_11_ICMP\n");
				return;
			case FLOW_ACTION_CAPWAP_802_3_ICMP:
				vty_out(vty,"  action_type = FLOW_ACTION_CAPWAP_802_3_ICMP\n");
				return;
			case FLOW_ACTION_RPA_ICMP:
				vty_out(vty,"  action_type = FLOW_ACTION_RPA_ICMP\n");
				dcli_show_fastfwd_rule_rpa(cmd_data,vty);
				return;
			case FLOW_ACTION_RPA_CAPWAP_802_11_ICMP:
				vty_out(vty,"  action_type = FLOW_ACTION_RPA_CAPWAP_802_11_ICMP\n");
				dcli_show_fastfwd_rule_rpa(cmd_data,vty);
				return;
			case FLOW_ACTION_RPA_CAPWAP_802_3_ICMP:
				vty_out(vty,"  action_type = FLOW_ACTION_RPA_CAPWAP_802_3_ICMP\n");
				dcli_show_fastfwd_rule_rpa(cmd_data,vty);
				return;
			case FLOW_ACTION_ETH_FORWARD:
				vty_out(vty,"    action_type = FLOW_ACTION_ETH_FORWARD\n");
				break;
			case FLOW_ACTION_CAP802_3_FORWARD:
				vty_out(vty,"    action_type = FLOW_ACTION_CAP802_3_FORWARD\n");
				break;
			case FLOW_ACTION_CAPWAP_FORWARD:
				vty_out(vty,"    action_type = FLOW_ACTION_CAPWAP_FORWARD\n");
				break;
			case FLOW_ACTION_RPA_ETH_FORWARD:
				vty_out(vty,"    action_type = FLOW_ACTION_RPA_ETH_FORWARD\n");
				dcli_show_fastfwd_rule_rpa(cmd_data,vty);
				break;
			case FLOW_ACTION_RPA_CAP802_3_FORWARD:
				vty_out(vty,"    action_type = FLOW_ACTION_RPA_CAP802_3_FORWARD\n");
				dcli_show_fastfwd_rule_rpa(cmd_data,vty);
				break;
			case FLOW_ACTION_RPA_CAPWAP_FORWARD:
				vty_out(vty,"    action_type = FLOW_ACTION_RPA_CAPWAP_FORWARD\n");
				dcli_show_fastfwd_rule_rpa(cmd_data,vty);
				break;
			default:
				vty_out(vty,"    action_type = UNKNOWN\n");
				break;
		}

		vty_out(vty,"    smac: %02x-%02x-%02x-%02x-%02x-%02x", MAC_FMT(cmd_data->fccp_cmd.fccp_data.acl_info.acl_param.ether_shost));
		vty_out(vty,"    dmac: %02x-%02x-%02x-%02x-%02x-%02x\n", MAC_FMT(cmd_data->fccp_cmd.fccp_data.acl_info.acl_param.ether_dhost));
		vty_out(vty,"    eth protocol: %04x\n", cmd_data->fccp_cmd.fccp_data.acl_info.acl_param.ether_type);
		vty_out(vty,"    forward port = %d\n", cmd_data->fccp_cmd.fccp_data.acl_info.acl_param.forward_port);
		
		if(cmd_data->fccp_cmd.fccp_data.acl_info.acl_param.time_stamp == RULE_IS_AGE)
			vty_out(vty,"    age\n");
		else if(cmd_data->fccp_cmd.fccp_data.acl_info.acl_param.time_stamp == RULE_IS_NEW)
			vty_out(vty,"    new\n");
		else
			vty_out(vty,"unknow\n");
		
	
		vty_out(vty,"    dsa_info: 0x%08x\n", cmd_data->fccp_cmd.fccp_data.acl_info.acl_param.dsa_info);
		vty_out(vty,"    out_type:0x%02x   out_tag:0x%02x   in_type:0x%02x   in_tag:0x%02x\n",
						cmd_data->fccp_cmd.fccp_data.acl_info.acl_param.out_ether_type, 
						cmd_data->fccp_cmd.fccp_data.acl_info.acl_param.out_tag, 
						cmd_data->fccp_cmd.fccp_data.acl_info.acl_param.in_ether_type, 
						cmd_data->fccp_cmd.fccp_data.acl_info.acl_param.in_tag);
		vty_out(vty,"    action mask = 0x%x\n", cmd_data->fccp_cmd.fccp_data.acl_info.acl_param.action_mask);

		/*wangjian add for slave cpu ip is 0 20121214 */
		if (cmd_data->fccp_cmd.fccp_data.acl_info.acl_param.nat_flag == 1)
		{
			vty_out(vty,"nat_info:    %u.%u.%u.%u:%u  ==> %u.%u.%u.%u:%u \n",
			IP_FMT(cmd_data->fccp_cmd.fccp_data.acl_info.acl_param.nat_sip),
			cmd_data->fccp_cmd.fccp_data.acl_info.acl_param.nat_sport,
			IP_FMT(cmd_data->fccp_cmd.fccp_data.acl_info.acl_param.nat_dip),
			cmd_data->fccp_cmd.fccp_data.acl_info.acl_param.nat_dport);
		}
		/*add by wangjian for support pppoe 2013-3-15*/
		if (cmd_data->fccp_cmd.fccp_data.acl_info.acl_param.pppoe_flag == 1)
		{
			vty_out(vty,"pppoe_info:    session_id:%u \n", cmd_data->fccp_cmd.fccp_data.acl_info.acl_param.pppoe_session_id);
		}
	}
}

/*2013.2.28 
Function for showing information of user table,just for printing,return NULL*/
void dcli_show_user_rule(se_interative_t  *cmd_data,struct vty *vty)
{
    vty_out(vty, "user rule information:\n"
                 "    user_ip: %u.%u.%u.%u \n"
                 "    user_state: %d\n"
                 "    user_index: %d\n"
                 "    user_link_index: %d\n"
                 "    forward_up_bytes:     %llu \n"
                 "    forward_up_packet:    %llu \n"
                 "    forward_down_bytes:   %llu \n"
                 "    forward_down_packet:  %llu \n",
                 IP_FMT(cmd_data->fccp_cmd.fccp_data.user_rule.user_info.user_ip),
                 cmd_data->fccp_cmd.fccp_data.user_rule.user_info.user_state,
                 cmd_data->fccp_cmd.fccp_data.user_rule.user_index,
                 cmd_data->fccp_cmd.fccp_data.user_rule.user_link_index,
                 cmd_data->fccp_cmd.fccp_data.user_rule.user_info.forward_up_bytes,
                 cmd_data->fccp_cmd.fccp_data.user_rule.user_info.forward_up_packet,
                 cmd_data->fccp_cmd.fccp_data.user_rule.user_info.forward_down_bytes,
                 cmd_data->fccp_cmd.fccp_data.user_rule.user_info.forward_down_packet);
}

DEFUN(show_acl_learned_func,
	show_acl_learned_cmd,
	"show fast-forward rule learned",
	SHOW_STR
	"fast forward module\n"
	"fast forward acl rule information\n"
	"fast forward learned acl information\n"
)
{
	se_interative_t  cmd_data;
	int ret;
	struct timeval overtime;
	uint32_t acl_static_index=0xFFFFFFFF;
	uint32_t acl_dynamic_index=0;
	uint32_t acl_cnt=0;
	uint32_t acl_static_cnt=0;
	int loop=0,j=0,i=0;
	memset(&overtime,0,sizeof(overtime));
	memset(&cmd_data,0,sizeof(cmd_data));
	cmd_data.fccp_cmd.fccp_data.acl_info.acl_static_index = acl_static_index;
	strncpy(cmd_data.hand_cmd,SE_AGENT_SHOW_ACL_LEARNED,strlen(SE_AGENT_SHOW_ACL_LEARNED));
	ret = sendto_agent(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),vty);
	if(ret<=0)
	{
		vty_out(vty,WRITE_FAIL_STR);
		return CMD_FAILURE;
	}
	memset(&cmd_data,0,sizeof(cmd_data));
	overtime.tv_sec=DCLI_WAIT_TIME;
	ret=read_within_time(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),&overtime);
	if(ret==READ_ERROR)
	{
		vty_out(vty,AGENT_NO_RESPOND_STR);
		return CMD_FAILURE;
	}
	if(cmd_data.cmd_result!=AGENT_RETURN_OK)
	{
		vty_out(vty,"%s\n",cmd_data.err_info);
		return CMD_FAILURE;
	}

	if(acl_static_index == 0xFFFFFFFF)
	{
		acl_cnt=cmd_data.fccp_cmd.fccp_data.acl_info.acl_cnt;
		acl_static_cnt=cmd_data.fccp_cmd.fccp_data.acl_info.acl_static_cnt;
		acl_static_index = cmd_data.fccp_cmd.fccp_data.acl_info.acl_static_index;
		acl_dynamic_index = cmd_data.fccp_cmd.fccp_data.acl_info.acl_dynamic_index;
	}
	vty_out(vty,"acl learned total :%u\n",acl_cnt);
	/* if(acl_cnt == 0) */
	if(acl_cnt == 0)
		return CMD_SUCCESS;
	vty_out(vty,"=====================================================\n");
display_loop:
	if(acl_cnt < DISPLAY_ACL_CNT)
		loop = acl_cnt;
	else
		loop = DISPLAY_ACL_CNT;

	for(i=0;i<loop;i++)
	{
		memset(&overtime,0,sizeof(overtime));
		memset(&cmd_data,0,sizeof(cmd_data));
		cmd_data.fccp_cmd.fccp_data.acl_info.acl_static_index=acl_static_index;
		cmd_data.fccp_cmd.fccp_data.acl_info.acl_dynamic_index=acl_dynamic_index;
		strncpy(cmd_data.hand_cmd,SE_AGENT_SHOW_ACL_LEARNED,strlen(SE_AGENT_SHOW_ACL_LEARNED));
		ret=sendto_agent(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),vty);
		if(ret<=0)
		{
			vty_out(vty,WRITE_FAIL_STR);
			return CMD_FAILURE;
		}
		memset(&cmd_data,0,sizeof(cmd_data));
		overtime.tv_sec=DCLI_WAIT_TIME;
		ret=read_within_time(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),&overtime);
		if(ret==READ_ERROR)
		{
			vty_out(vty,AGENT_NO_RESPOND_STR);
			return CMD_FAILURE;
		}
		if(cmd_data.cmd_result!=AGENT_RETURN_OK)
		{
			vty_out(vty,"%s\n",cmd_data.err_info);
			return CMD_FAILURE;
		}
		acl_static_index=cmd_data.fccp_cmd.fccp_data.acl_info.acl_static_index;
		acl_dynamic_index=cmd_data.fccp_cmd.fccp_data.acl_info.acl_dynamic_index;
		
		dcli_show_fastfwd_rule(&cmd_data,vty);
		
	}
display_end:
	acl_cnt -= loop;
	if(acl_cnt == 0)
		return CMD_SUCCESS;
	else
	{
		vty_out(vty,"c[continue] q[quit]\n");
		while(1)
		{
			int flag=getchar();
			if((flag== (int)('c')) || (flag == (int)(' ')))
				goto display_loop;
			else if(flag == (int)('q'))
				return CMD_SUCCESS;
			else
				continue;
		}
	}
	return CMD_SUCCESS;
}
DEFUN(show_acl_learning_func,
	show_acl_learning_cmd,
	"show fast-forward rule learning",
	SHOW_STR
	"fast forward module\n"
	"fast forward acl rule information\n"
	"fast forward learning acl information\n"
)
{
	se_interative_t  cmd_data;
	int ret;
	struct timeval overtime;
	uint32_t acl_static_index = 0xFFFFFFFF;
	uint32_t acl_dynamic_index = 0;
	uint32_t acl_cnt = 0;
	uint32_t acl_static_cnt = 0;
	int loop=0,j=0,i=0;
	memset(&overtime,0,sizeof(overtime));
	memset(&cmd_data,0,sizeof(cmd_data));
	cmd_data.fccp_cmd.fccp_data.acl_info.acl_static_index = acl_static_index;
	strncpy(cmd_data.hand_cmd,SE_AGENT_SHOW_ACL_LEARNING,strlen(SE_AGENT_SHOW_ACL_LEARNING));
	ret = sendto_agent(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),vty);
	if(ret<=0)
	{
		vty_out(vty,WRITE_FAIL_STR);
		return CMD_FAILURE;
	}
	memset(&cmd_data,0,sizeof(cmd_data));
	overtime.tv_sec=DCLI_WAIT_TIME;
	ret=read_within_time(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),&overtime);
	if(ret==READ_ERROR)
	{
		vty_out(vty,AGENT_NO_RESPOND_STR);
		return CMD_FAILURE;
	}
	if(cmd_data.cmd_result!=AGENT_RETURN_OK)
	{
		vty_out(vty,"%s\n",cmd_data.err_info);
		return CMD_FAILURE;
	}

	if(acl_static_index == 0xFFFFFFFF)
	{
		acl_cnt=cmd_data.fccp_cmd.fccp_data.acl_info.acl_cnt;
		acl_static_cnt=cmd_data.fccp_cmd.fccp_data.acl_info.acl_static_cnt;
		acl_static_index = cmd_data.fccp_cmd.fccp_data.acl_info.acl_static_index;
		acl_dynamic_index = cmd_data.fccp_cmd.fccp_data.acl_info.acl_dynamic_index;
	}
	vty_out(vty,"acl learning total :%u\n",acl_cnt);
	/* if(acl_cnt == 0) */
	if(acl_cnt == 0)
		return CMD_SUCCESS;
	vty_out(vty,"=====================================================\n");
display_loop:
	if(acl_cnt < DISPLAY_ACL_CNT)
		loop = acl_cnt;
	else
		loop = DISPLAY_ACL_CNT;

	for(i=0;i<loop;i++)
	{
		memset(&overtime,0,sizeof(overtime));
		memset(&cmd_data,0,sizeof(cmd_data));
		cmd_data.fccp_cmd.fccp_data.acl_info.acl_static_index=acl_static_index;
		cmd_data.fccp_cmd.fccp_data.acl_info.acl_dynamic_index=acl_dynamic_index;
		strncpy(cmd_data.hand_cmd,SE_AGENT_SHOW_ACL_LEARNING,strlen(SE_AGENT_SHOW_ACL_LEARNING));
		ret=sendto_agent(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),vty);
		if(ret<=0)
		{
			vty_out(vty,WRITE_FAIL_STR);
			return CMD_FAILURE;
		}
		memset(&cmd_data,0,sizeof(cmd_data));
		overtime.tv_sec=DCLI_WAIT_TIME;
		ret=read_within_time(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),&overtime);
		if(ret==READ_ERROR)
		{
			vty_out(vty,AGENT_NO_RESPOND_STR);
			return CMD_FAILURE;
		}
		if(cmd_data.cmd_result!=AGENT_RETURN_OK)
		{
			vty_out(vty,"%s\n",cmd_data.err_info);
			return CMD_FAILURE;
		}
		acl_static_index=cmd_data.fccp_cmd.fccp_data.acl_info.acl_static_index;
		acl_dynamic_index=cmd_data.fccp_cmd.fccp_data.acl_info.acl_dynamic_index;
		
		dcli_show_fastfwd_rule(&cmd_data,vty);
	}
display_end:
	acl_cnt -= loop;
	if(acl_cnt == 0)
		return CMD_SUCCESS;
	else
	{
		vty_out(vty,"c[continue] q[quit]\n");
		while(1)
		{
			int flag=getchar();
			if((flag== (int)('c')) || (flag == (int)(' ')))
				goto display_loop;
			else if(flag == (int)('q'))
				return CMD_SUCCESS;
			else
				continue;
		}
	}
	return CMD_SUCCESS;
}
DEFUN(show_user_flow_func,
	show_user_flow_cmd,
	"show user flow_statistic ip A.B.C.D",
	SHOW_STR
	"user \n"
	"user flow statistic information\n"
	"user ip address\n"
	"user ip address \n")
{	
	se_interative_t  cmd_data;
	int ret = 0;
	struct timeval overtime;
	memset(&overtime,0,sizeof(overtime));
	memset(&cmd_data,0,sizeof(cmd_data));
	if(argc > 1)
	{
		vty_out(vty,CMD_PARAMETER_ERROR);
		return CMD_FAILURE;
	}
	if(COMMON_ERROR==parse_ip_check((char*)argv[0]))
	{
		vty_out(vty,CMD_PARAMETER_ERROR);
		return CMD_FAILURE;	
	}
	
	cmd_data.fccp_cmd.fccp_data.user_info.user_ip = dcli_ip2ulong((char*)argv[0]);
	strncpy(cmd_data.hand_cmd,SE_AGENT_GET_USER_FLOWS,strlen(SE_AGENT_GET_USER_FLOWS));
	ret=sendto_agent(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),vty);
	if(ret<=0)
	{
		vty_out(vty,WRITE_FAIL_STR);
		return CMD_FAILURE;
	}
	memset(&cmd_data,0,sizeof(cmd_data));
	overtime.tv_sec=10;
	ret=read_within_time(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),&overtime);
	if(ret==READ_ERROR)
	{
		vty_out(vty,AGENT_NO_RESPOND_STR);
		return CMD_FAILURE;
	}
	
	if(cmd_data.cmd_result!=AGENT_RETURN_OK)
	{
		vty_out(vty,"%s\n",cmd_data.err_info);
		return CMD_FAILURE;
	}
	vty_out(vty,"forward up bytes    = %llu\n",cmd_data.fccp_cmd.fccp_data.user_info.forward_up_bytes);
	vty_out(vty,"forward up packet   = %llu\n",cmd_data.fccp_cmd.fccp_data.user_info.forward_up_packet);
	vty_out(vty,"forward down bytes  = %llu\n",cmd_data.fccp_cmd.fccp_data.user_info.forward_down_bytes);
	vty_out(vty,"forward down packet = %llu\n",cmd_data.fccp_cmd.fccp_data.user_info.forward_down_packet);
	return CMD_SUCCESS;
}
DEFUN(config_traffic_monitor_func,
	  config_traffic_monitor_cmd,
	  "config traffic-monitor (enable|disable)",
	  CONFIG_STR
	  "traffic monitor \n"
	  "traffic monitor enable\n"
	  "traffic monitor disable\n"
)
{
	
	char *enable = (char *)argv[0];
	int flag = FUNC_DISABLE;
	se_interative_t  cmd_data;
	int ret = 0;
	struct timeval overtime;
	memset(&overtime,0,sizeof(overtime));
	memset(&cmd_data,0,sizeof(cmd_data));
	if(argc > 1) 
	{
		vty_out(vty,CMD_PARAMETER_NUM_ERROR);
		return CMD_WARNING;
	}
	if(0 == strncmp(enable,"disable",strlen(enable)))
	{
		flag = FUNC_DISABLE;
	}
	else if(0 == strncmp(enable,"enable",strlen(enable)))
	{
		flag = FUNC_ENABLE;
	}
	else
	{
		vty_out(vty,CMD_PARAMETER_ERROR);
		return CMD_WARNING;
	}
	cmd_data.fccp_cmd.fccp_data.module_enable = flag;
	strncpy(cmd_data.hand_cmd,SE_AGENT_CONFIG_TRAFFIC_MONITOR,strlen(SE_AGENT_CONFIG_TRAFFIC_MONITOR));
	ret = sendto_agent(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),vty);
	if(ret<=0)
	{
		vty_out(vty,WRITE_FAIL_STR);
		return CMD_FAILURE;
	}
	overtime.tv_sec = DCLI_WAIT_TIME;
	ret = read_within_time(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),&overtime);
	if(ret == READ_ERROR)
	{
		vty_out(vty,AGENT_NO_RESPOND_STR);
		return CMD_FAILURE;
	}
	if(cmd_data.cmd_result!=AGENT_RETURN_OK)
	{
		vty_out(vty,"%s\n", cmd_data.err_info);
		return CMD_FAILURE;
	}
	if(cmd_data.fccp_cmd.ret_val!=FCCP_RETURN_OK)
	{
		vty_out(vty,COMMAND_FAIL_STR);
		return CMD_FAILURE;
	}
	
	return CMD_SUCCESS;
}


DEFUN(clear_traffic_monitor_func,
	  clear_traffic_monitor_cmd,
	  "clear traffic-monitor statistics",
	  CLEAR_STR
	  "traffic monitor \n"
	  "traffic monitor statistics\n"
)
{
	se_interative_t  cmd_data;
	int ret = 0;
	struct timeval overtime;
	memset(&overtime,0,sizeof(overtime));
	memset(&cmd_data,0,sizeof(cmd_data));
	
	strncpy(cmd_data.hand_cmd,SE_AGENT_CLEAR_TRAFFIC_MONITOR,strlen(SE_AGENT_CLEAR_TRAFFIC_MONITOR));
	ret = sendto_agent(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),vty);
	if(ret<=0)
	{
		vty_out(vty,WRITE_FAIL_STR);
		return CMD_FAILURE;
	}
	overtime.tv_sec = DCLI_WAIT_TIME;
	ret = read_within_time(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),&overtime);
	if(ret == READ_ERROR)
	{
		vty_out(vty,AGENT_NO_RESPOND_STR);
		return CMD_FAILURE;
	}
	if(cmd_data.cmd_result!=AGENT_RETURN_OK)
	{
		vty_out(vty,"%s\n", cmd_data.err_info);
		return CMD_FAILURE;
	}
	return CMD_SUCCESS;
}

DEFUN(show_traffic_monitor_func,
	  show_traffic_monitor_cmd,
	  "show traffic-monitor statistics",
	  SHOW_STR
	  "traffic monitor \n"
	  "traffic monitor statistics\n"
)
{
	se_interative_t  cmd_data;
	int ret = 0;
	struct timeval overtime;
	memset(&overtime,0,sizeof(overtime));
	memset(&cmd_data,0,sizeof(cmd_data));
	
	strncpy(cmd_data.hand_cmd,SE_AGENT_SHOW_TRAFFIC_MONITOR,strlen(SE_AGENT_SHOW_TRAFFIC_MONITOR));
	ret = sendto_agent(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),vty);
	if(ret<=0)
	{
		vty_out(vty,WRITE_FAIL_STR);
		return CMD_FAILURE;
	}
	overtime.tv_sec = DCLI_WAIT_TIME;
	ret = read_within_time(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),&overtime);
	if(ret == READ_ERROR)
	{
		vty_out(vty,AGENT_NO_RESPOND_STR);
		return CMD_FAILURE;
	}
	if(cmd_data.cmd_result != AGENT_RETURN_OK)
	{
		vty_out(vty,"%s\n", cmd_data.err_info);
		return CMD_FAILURE;
	}
	vty_out(vty,"%-25s%-15s%-15s\n","packet type","total packet","rate(pps)");
	vty_out(vty,"====================================================\n");
	
	vty_out(vty,"%-25s%-15u%-15u\n","total packet",\
		                                 cmd_data.fccp_cmd.fccp_data.traffic_stats.total_pkt,\
		                                 cmd_data.fccp_cmd.fccp_data.traffic_stats.total_pps);
	vty_out(vty,"%-25s%-15u%-15u\n","error packet",\
		                                 cmd_data.fccp_cmd.fccp_data.traffic_stats.err_pkt,\
		                                 cmd_data.fccp_cmd.fccp_data.traffic_stats.err_pps);
	vty_out(vty,"%-25s%-15u%-15u\n","broadcast packet",\
		                                 cmd_data.fccp_cmd.fccp_data.traffic_stats.bcast_pkt,\
		                                 cmd_data.fccp_cmd.fccp_data.traffic_stats.bcast_pps);
	vty_out(vty,"%-25s%-15u%-15u\n","multicast packet",\
		                                 cmd_data.fccp_cmd.fccp_data.traffic_stats.mcast_pkt,\
		                                 cmd_data.fccp_cmd.fccp_data.traffic_stats.mcast_pps);
	vty_out(vty,"\n");
	
	vty_out(vty,"%-25s%-15u%-15u\n","total noip packet",\
		                                 cmd_data.fccp_cmd.fccp_data.traffic_stats.total_noip_pkt,\
		                                 cmd_data.fccp_cmd.fccp_data.traffic_stats.total_noip_pps);
	vty_out(vty,"%-25s%-15u%-15u\n","arp packet",\
		                                 cmd_data.fccp_cmd.fccp_data.traffic_stats.arp_pkt,\
		                                 cmd_data.fccp_cmd.fccp_data.traffic_stats.arp_pps);
	vty_out(vty,"%-25s%-15u%-15u\n","rarp packet",\
		                                 cmd_data.fccp_cmd.fccp_data.traffic_stats.rarp_pkt,\
		                                 cmd_data.fccp_cmd.fccp_data.traffic_stats.rarp_pps);
	vty_out(vty,"%-25s%-15u%-15u\n","vrrp packet",\
		                                 cmd_data.fccp_cmd.fccp_data.traffic_stats.vrrp_pkt,\
		                                 cmd_data.fccp_cmd.fccp_data.traffic_stats.vrrp_pps);
	vty_out(vty,"%-25s%-15u%-15u\n","802.1x packet",\
		                                 cmd_data.fccp_cmd.fccp_data.traffic_stats._802_1x_pkt,\
		                                 cmd_data.fccp_cmd.fccp_data.traffic_stats._802_1x_pps);
	vty_out(vty,"\n");
	
	vty_out(vty,"%-25s%-15u%-15u\n","total ip packet",\
		                                 cmd_data.fccp_cmd.fccp_data.traffic_stats.total_ip_pkt,\
		                                 cmd_data.fccp_cmd.fccp_data.traffic_stats.total_ip_pps);
	
	vty_out(vty,"%-25s%-15u%-15u\n","error ip packet",\
		                                 cmd_data.fccp_cmd.fccp_data.traffic_stats.ip_exception_pkt,\
		                                 cmd_data.fccp_cmd.fccp_data.traffic_stats.ip_exception_pps);
	vty_out(vty,"%-25s%-15u%-15u\n","external frag packet",\
		                                 cmd_data.fccp_cmd.fccp_data.traffic_stats.frag_pkt,\
		                                 cmd_data.fccp_cmd.fccp_data.traffic_stats.frag_pps);
	vty_out(vty,"%-25s%-15u%-15u\n","ipv6 packet",\
		                                 cmd_data.fccp_cmd.fccp_data.traffic_stats.ipv6_pkt,\
		                                 cmd_data.fccp_cmd.fccp_data.traffic_stats.ipv6_pps);
	vty_out(vty,"%-25s%-15u%-15u\n","icmp packet",\
		                                 cmd_data.fccp_cmd.fccp_data.traffic_stats.icmp_pkt,\
		                                 cmd_data.fccp_cmd.fccp_data.traffic_stats.icmp_pps);
	vty_out(vty,"%-25s%-15u%-15u\n","L4 error packet",\
		                                 cmd_data.fccp_cmd.fccp_data.traffic_stats.l4_err_pkt,\
		                                 cmd_data.fccp_cmd.fccp_data.traffic_stats.l4_err_pps);
	vty_out(vty,"\n");
	
	vty_out(vty,"%-25s%-15u%-15u\n","total tcp packet",\
		                                 cmd_data.fccp_cmd.fccp_data.traffic_stats.total_tcp_pkt,\
		                                 cmd_data.fccp_cmd.fccp_data.traffic_stats.total_tcp_pps);
	vty_out(vty,"%-25s%-15u%-15u\n","telnet packet",\
		                                 cmd_data.fccp_cmd.fccp_data.traffic_stats.telnet_pkt,\
		                                 cmd_data.fccp_cmd.fccp_data.traffic_stats.telnet_pps);
	vty_out(vty,"%-25s%-15u%-15u\n","ssh packet",\
		                                 cmd_data.fccp_cmd.fccp_data.traffic_stats.ssh_pkt,\
		                                 cmd_data.fccp_cmd.fccp_data.traffic_stats.err_pps);
	vty_out(vty,"\n");
	
	vty_out(vty,"%-25s%-15u%-15u\n","total udp packet",\
		                                 cmd_data.fccp_cmd.fccp_data.traffic_stats.total_udp_pkt,\
		                                 cmd_data.fccp_cmd.fccp_data.traffic_stats.total_udp_pps);
	vty_out(vty,"%-25s%-15u%-15u\n","access radius packet",\
		                                 cmd_data.fccp_cmd.fccp_data.traffic_stats.access_radius_pkt,\
		                                 cmd_data.fccp_cmd.fccp_data.traffic_stats.access_radius_pps);
	vty_out(vty,"%-25s%-15u%-15u\n","account radius packet",\
		                                 cmd_data.fccp_cmd.fccp_data.traffic_stats.account_radius_pkt,\
		                                 cmd_data.fccp_cmd.fccp_data.traffic_stats.account_radius_pps);
	vty_out(vty,"%-25s%-15u%-15u\n","portal packet",\
		                                 cmd_data.fccp_cmd.fccp_data.traffic_stats.portal_pkt,\
		                                 cmd_data.fccp_cmd.fccp_data.traffic_stats.portal_pps);
	vty_out(vty,"%-25s%-15u%-15u\n","dhcp packet",\
		                                 cmd_data.fccp_cmd.fccp_data.traffic_stats.dhcp_pkt,\
		                                 cmd_data.fccp_cmd.fccp_data.traffic_stats.dhcp_pps);
	vty_out(vty,"%-25s%-15u%-15u\n","rip packet",\
		                                 cmd_data.fccp_cmd.fccp_data.traffic_stats.rip_pkt,\
		                                 cmd_data.fccp_cmd.fccp_data.traffic_stats.rip_pps);
	vty_out(vty,"%-25s%-15u%-15u\n","capwap control packet",\
		                                 cmd_data.fccp_cmd.fccp_data.traffic_stats.cw_ctl_pkt,\
		                                 cmd_data.fccp_cmd.fccp_data.traffic_stats.cw_ctl_pps);
	vty_out(vty,"\n");
	
	vty_out(vty,"%-25s%-15u%-15u\n","capwap data packet",\
		                                 cmd_data.fccp_cmd.fccp_data.traffic_stats.cw_dat_pkt,\
		                                 cmd_data.fccp_cmd.fccp_data.traffic_stats.cw_dat_pps);
	vty_out(vty,"%-25s%-15u%-15u\n","capwap arp packet",\
		                                 cmd_data.fccp_cmd.fccp_data.traffic_stats.cw_arp_pkt,\
		                                 cmd_data.fccp_cmd.fccp_data.traffic_stats.cw_arp_pps);
	vty_out(vty,"%-25s%-15u%-15u\n","capwap error ip packet",\
		                                 cmd_data.fccp_cmd.fccp_data.traffic_stats.cw_ip_exc_pkt,\
		                                 cmd_data.fccp_cmd.fccp_data.traffic_stats.cw_ip_exc_pps);
	vty_out(vty,"%-25s%-15u%-15u\n","capwap tcp packet",\
		                                 cmd_data.fccp_cmd.fccp_data.traffic_stats.cw_tcp_pkt,\
		                                 cmd_data.fccp_cmd.fccp_data.traffic_stats.cw_tcp_pps);
	vty_out(vty,"%-25s%-15u%-15u\n","capwap udp packet",\
		                                 cmd_data.fccp_cmd.fccp_data.traffic_stats.cw_udp_pkt,\
		                                 cmd_data.fccp_cmd.fccp_data.traffic_stats.cw_udp_pps);
	vty_out(vty,"%-25s%-15u%-15u\n","capwap icmp packet",\
		                                 cmd_data.fccp_cmd.fccp_data.traffic_stats.cw_icmp_pkt,\
		                                 cmd_data.fccp_cmd.fccp_data.traffic_stats.cw_icmp_pps);
	vty_out(vty,"%-25s%-15u%-15u\n","capwap frag packet",\
		                                 cmd_data.fccp_cmd.fccp_data.traffic_stats.cw_ip_frag_pkt,\
		                                 cmd_data.fccp_cmd.fccp_data.traffic_stats.cw_ip_frag_pps);
	vty_out(vty,"%-25s%-15u%-15u\n","capwap 802.3 packet",\
		                                 cmd_data.fccp_cmd.fccp_data.traffic_stats.cw_8023_dat_pkt,\
		                                 cmd_data.fccp_cmd.fccp_data.traffic_stats.cw_8023_dat_pps);
	vty_out(vty,"%-25s%-15u%-15u\n","inter ac roaming packet",\
		                                 cmd_data.fccp_cmd.fccp_data.traffic_stats.inter_ac_roaming_pkt,\
		                                 cmd_data.fccp_cmd.fccp_data.traffic_stats.inter_ac_roaming_pps);
	return CMD_SUCCESS;
}
DEFUN(config_pure_payload_acct_func,
	  config_pure_payload_acct_cmd,
	  "config  pure-payload-acct (enable | disable)",
	  CONFIG_STR
	  "pure payload account function\n"
	  "enable\n"
	  "disable\n"
)
{
	
	char *enable = (char *)argv[0];
	int flag = FUNC_DISABLE;
	int ret = 0;
	se_interative_t  cmd_data;
	struct timeval overtime;
	memset(&overtime,0,sizeof(overtime));
	memset(&cmd_data,0,sizeof(cmd_data));
	if(argc > 1) 
	{
		vty_out(vty,CMD_PARAMETER_NUM_ERROR);
		return CMD_WARNING;
	}
	if(0==strncmp(enable,"enable",strlen(enable)))
	{
		flag = FUNC_ENABLE;
	}
	else if(0==strncmp(enable,"disable",strlen(enable)))
	{
		flag = FUNC_DISABLE;
	}
	
	else
	{
		vty_out(vty,CMD_PARAMETER_ERROR);
		return CMD_FAILURE;
	}
	cmd_data.fccp_cmd.fccp_data.module_enable = flag;
	strncpy(cmd_data.hand_cmd,SE_AGENT_CONFIG_PURE_PAYLOAD_ACCT,strlen(SE_AGENT_CONFIG_PURE_PAYLOAD_ACCT));
	ret=sendto_agent(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),vty);
	if(ret<=0)
	{
		vty_out(vty,WRITE_FAIL_STR);
		return CMD_FAILURE;
	}
	overtime.tv_sec = DCLI_WAIT_TIME;
	ret=read_within_time(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),&overtime);
	if(ret==READ_ERROR)
	{
		vty_out(vty,AGENT_NO_RESPOND_STR);
		return CMD_FAILURE;
	}
	if(cmd_data.cmd_result!=AGENT_RETURN_OK)
	{
		vty_out(vty,"%s\n",cmd_data.err_info);
		return CMD_FAILURE;
	}
	if(cmd_data.fccp_cmd.ret_val!= FCCP_RETURN_OK)
	{
		vty_out(vty,COMMAND_FAIL_STR);
		return CMD_FAILURE;
	}
	return CMD_SUCCESS;
}


//#ifdef DCLI_SE_AGENT_DEBUG
DEFUN(set_user_online_func,
	set_user_online_cmd,
	"add user ip A.B.C.D",
	"add user\n"
	"user \n"
	"user ip address\n"
	"user ip address \n")
{	
	se_interative_t  cmd_data;
	int ret = 0;
	struct timeval overtime;
	memset(&overtime,0,sizeof(overtime));
	memset(&cmd_data,0,sizeof(cmd_data));
	if(argc > 1)
	{
		vty_out(vty,"param wrong\n");
		return CMD_FAILURE;
	}
	if(COMMON_ERROR==parse_ip_check((char*)argv[0]))
	{
		vty_out(vty,"param wrong\n");
		return CMD_FAILURE;	
	}

	cmd_data.fccp_cmd.fccp_data.user_info.user_ip = dcli_ip2ulong((char*)argv[0]);
	strncpy(cmd_data.hand_cmd,SE_AGENT_USER_ONLINE,strlen(SE_AGENT_USER_ONLINE));
	ret=sendto_agent(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),vty);
	if(ret<=0)
	{
		vty_out(vty," %%Write command to se_agent failed\n");
		return CMD_FAILURE;
	}
	return CMD_SUCCESS;
}

DEFUN(set_user_offline_func,
	set_user_offline_cmd,
	"delete user ip A.B.C.D",
	"delete user\n"
	"user \n"
	"user ip address\n"
	"user ip address \n")
{	
	se_interative_t  cmd_data;
	int ret = 0;
	struct timeval overtime;
	memset(&overtime,0,sizeof(overtime));
	memset(&cmd_data,0,sizeof(cmd_data));
	if(argc > 1)
	{
		vty_out(vty,"param wrong\n");
		return CMD_FAILURE;
	}
	if(COMMON_ERROR==parse_ip_check((char*)argv[0]))
	{
		vty_out(vty,"param wrong\n");
		return CMD_FAILURE;	
	}

	cmd_data.fccp_cmd.fccp_data.user_info.user_ip = dcli_ip2ulong((char*)argv[0]);
	strncpy(cmd_data.hand_cmd,SE_AGENT_USER_OFFLINE,strlen(SE_AGENT_USER_OFFLINE));
	ret=sendto_agent(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),vty);
	if(ret<=0)
	{
		vty_out(vty," %%Write command to se_agent failed\n");
		return CMD_FAILURE;
	}
	return CMD_SUCCESS;
}
//#endif

/*wangjian clear*/
DEFUN(clear_rule_ip_func,
	clear_rule_ip_cmd,
	"clear fast-forward rule ip IP",
	CLEAR_STR
	"fastfwd module\n"
	"fastfwd acl information\n"
	"all fastfwd rule information\n"
	"del ip address A.B.C.D\n"
	)
{
	se_interative_t  cmd_data;
	int ret;
	
	memset(&cmd_data,0,sizeof(cmd_data));
	if(COMMON_ERROR==parse_ip_check((char*)argv[0]))
	{
		vty_out(vty,"ip address format error\n");
		return CMD_FAILURE;
	}
	cmd_data.fccp_cmd.fccp_data.user_info.user_ip=dcli_ip2ulong((char*)argv[0]);
	strncpy(cmd_data.hand_cmd,SE_AGENT_CLEAR_RULE_IP,strlen(SE_AGENT_CLEAR_RULE_IP));
	ret=sendto_agent(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),vty);
	if(ret<=0)
	{
		vty_out(vty,WRITE_FAIL_STR);
		return CMD_FAILURE;
	}

	return CMD_SUCCESS;
}


/*show fwd info  1.fwd exist 2.fwd open 3.fwd occupy core
 * wangjian add 20120705
 */
DEFUN(show_fast_forward_info,
		  show_fast_forward_info_cmd,
		  "show fast-forward-info",
		  SHOW_STR
		  "fast forward info \n"
)
{
	se_interative_t  cmd_data;
	int ret = 0;
	struct timeval overtime;
	
	memset(&overtime,0,sizeof(overtime));
	memset(&cmd_data,0,sizeof(cmd_data));
	
	strncpy(cmd_data.hand_cmd,SE_AGENT_SHOW_FAST_FWD_INFO,strlen(SE_AGENT_SHOW_FAST_FWD_INFO));
	ret=sendto_agent(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),vty);
	if(ret<=0)
	{
		vty_out(vty,WRITE_FAIL_STR);
		return CMD_FAILURE;
	}
	
	memset(&cmd_data,0,sizeof(cmd_data));
	overtime.tv_sec=DCLI_WAIT_TIME;
	ret=read_within_time(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),&overtime);
	if(ret==READ_ERROR)
	{
		vty_out(vty,AGENT_NO_RESPOND_STR);
		return CMD_FAILURE;
	}
	
	if(cmd_data.cmd_result!=AGENT_RETURN_OK)
	{
		vty_out(vty,"%s\n",cmd_data.err_info);
		return CMD_FAILURE;
	}
	
	if(cmd_data.fccp_cmd.ret_val!=FCCP_RETURN_OK)
	{
		vty_out(vty,COMMAND_FAIL_STR);
		return CMD_FAILURE;
	}
	
	vty_out(vty,"fast_forward is exist! \n");
	vty_out(vty,"fast_forward is %s\n", (cmd_data.fccp_cmd.fccp_data.fast_fwd_info.fast_fwd_enable == 1) ? "enable" : "disable"); 
	vty_out(vty,"fast_forward coremask is 0x%02x\n",cmd_data.fccp_cmd.fccp_data.fast_fwd_info.fast_fwd_coremask);
	
	return CMD_SUCCESS;
}

/*wangjian 2012.07.09 add ip */
DEFUN(show_rule_by_ip_func,
	show_rule_by_ip_cmd,
	"show fast-forward rule ip IP",
	SHOW_STR
	"fastfwd module\n"
	"fastfwd acl information\n"
	"fastfwd rule information by ip\n"
	"ip address A.B.C.D\n"
	)
{
	se_interative_t  cmd_data;
	int ret;
	struct timeval overtime;
	uint32_t acl_static_index = 0xFFFFFFFF;
	uint32_t acl_cnt=0;
	uint32_t acl_static_cnt=0;
	int loop=0,j=0,i=0;
	uint8_t rule_state = 0;
	uint32_t user_ip;
	
	memset(&overtime,0,sizeof(overtime));
	memset(&cmd_data,0,sizeof(cmd_data));
	
	if(COMMON_ERROR==parse_ip_check((char*)argv[0]))
	{
		vty_out(vty,"ip address format error\n");
		return CMD_FAILURE;
	}
	user_ip=dcli_ip2ulong((char*)argv[0]);
	cmd_data.fccp_cmd.fccp_data.acl_info.acl_param.sip=user_ip; /*wangjian to do sip*/
	cmd_data.fccp_cmd.fccp_data.acl_info.acl_static_index=acl_static_index;
	
	strncpy(cmd_data.hand_cmd,SE_AGENT_SHOW_RULE_IP,strlen(SE_AGENT_SHOW_RULE_IP));
	ret=sendto_agent(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),vty);
	if(ret<=0)
	{
		vty_out(vty,WRITE_FAIL_STR);
		return CMD_FAILURE;
	}
	
	memset(&cmd_data,0,sizeof(cmd_data));
	overtime.tv_sec=DCLI_WAIT_TIME;
	ret=read_within_time(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),&overtime);
	if(ret==READ_ERROR)
	{
		vty_out(vty,AGENT_NO_RESPOND_STR);
		return CMD_FAILURE;
	}
	
	if(cmd_data.cmd_result!=AGENT_RETURN_OK)
	{
		vty_out(vty,"%s\n",cmd_data.err_info);
		return CMD_FAILURE;
	}

	if(acl_static_index == 0xFFFFFFFF)
	{
		acl_cnt=cmd_data.fccp_cmd.fccp_data.acl_info.acl_cnt;
		acl_static_cnt=cmd_data.fccp_cmd.fccp_data.acl_info.acl_static_cnt;
		acl_static_index = cmd_data.fccp_cmd.fccp_data.acl_info.acl_static_index;
	}
	vty_out(vty,"acl rule by ip total :%u\n",acl_cnt);

	if(acl_static_cnt == 0)
		return CMD_SUCCESS;

	vty_out(vty,"=====================================================\n");
display_loop:
        if(acl_static_cnt < DISPLAY_ACL_CNT)
            loop = acl_static_cnt;
        else
            loop = DISPLAY_ACL_CNT;
    
        for(i=0;i<loop;i++)
        {
            memset(&overtime,0,sizeof(overtime));
            memset(&cmd_data,0,sizeof(cmd_data));
            cmd_data.fccp_cmd.fccp_data.acl_info.acl_static_index=acl_static_index;
	        cmd_data.fccp_cmd.fccp_data.acl_info.acl_param.sip=user_ip;
            strncpy(cmd_data.hand_cmd,SE_AGENT_SHOW_RULE_IP,strlen(SE_AGENT_SHOW_RULE_IP));
            ret=sendto_agent(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),vty);
            if(ret<=0)
            {
                vty_out(vty,WRITE_FAIL_STR);
                return CMD_FAILURE;
            }
            memset(&cmd_data,0,sizeof(cmd_data));
            overtime.tv_sec=DCLI_WAIT_TIME;
            ret=read_within_time(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),&overtime);
            if(ret==READ_ERROR)
            {
                vty_out(vty,AGENT_NO_RESPOND_STR);
                return CMD_FAILURE;
            }
			
            if(cmd_data.cmd_result!=AGENT_RETURN_OK)
            {
                vty_out(vty,"%s\n",cmd_data.err_info);
                return CMD_FAILURE;
            }
			
            acl_static_index=cmd_data.fccp_cmd.fccp_data.acl_info.acl_static_index;
            rule_state = cmd_data.fccp_cmd.fccp_data.acl_info.acl_param.rule_state;
            vty_out(vty,"acl information:\n");
            vty_out(vty,"    %u.%u.%u.%u:%u  ==> %u.%u.%u.%u:%u protocol=%d (%s)\t",
            IP_FMT(cmd_data.fccp_cmd.fccp_data.acl_info.acl_param.sip),
            cmd_data.fccp_cmd.fccp_data.acl_info.acl_param.sport,
            IP_FMT(cmd_data.fccp_cmd.fccp_data.acl_info.acl_param.dip),
            cmd_data.fccp_cmd.fccp_data.acl_info.acl_param.dport,
            cmd_data.fccp_cmd.fccp_data.acl_info.acl_param.protocol,
            PROTO_STR(cmd_data.fccp_cmd.fccp_data.acl_info.acl_param.protocol));
            
            vty_out(vty,"\n    rule_state is %s\n", rule_state == RULE_IS_LEARNED ? "learned" : (rule_state == RULE_IS_LEARNING ? "learning" :"unknow"));
            if(cmd_data.fccp_cmd.fccp_data.acl_info.acl_param.time_stamp == RULE_IS_AGE) 
            {
                vty_out(vty,"    age\n");
            }
            else
            {
                vty_out(vty,"    new\n");
            }

            
            if (rule_state == RULE_IS_LEARNED) 
            {
                if(cmd_data.fccp_cmd.fccp_data.acl_info.acl_param.action_type == FLOW_ACTION_DROP)
                {
                    vty_out(vty,"    action_type = FLOW_ACTION_DROP\n");
                    continue;
                }
                if(cmd_data.fccp_cmd.fccp_data.acl_info.acl_param.action_type == FLOW_ACTION_TOLINUX)
                {
                    vty_out(vty,"    action_type = FLOW_ACTION_TOLINUX\n");
                    continue;
                }

                vty_out(vty,"    smac: %02x-%02x-%02x-%02x-%02x-%02x", MAC_FMT(cmd_data.fccp_cmd.fccp_data.acl_info.acl_param.ether_shost));
                vty_out(vty,"    dmac: %02x-%02x-%02x-%02x-%02x-%02x\n", MAC_FMT(cmd_data.fccp_cmd.fccp_data.acl_info.acl_param.ether_dhost));
                vty_out(vty,"    eth protocol: %04x\n", cmd_data.fccp_cmd.fccp_data.acl_info.acl_param.ether_type);

                switch(cmd_data.fccp_cmd.fccp_data.acl_info.acl_param.action_type)
                {
                    case FLOW_ACTION_ETH_FORWARD:
                        vty_out(vty,"    action_type = FLOW_ACTION_ETH_FORWARD\n");
                        break;
                    case FLOW_ACTION_CAP802_3_FORWARD:
                        vty_out(vty,"    action_type = FLOW_ACTION_CAP802_3_FORWARD\n");
        
                        break;
                    case FLOW_ACTION_CAPWAP_FORWARD:
                        vty_out(vty,"    action_type = FLOW_ACTION_CAPWAP_FORWARD\n");
                        break;
                    default:
                        vty_out(vty,"    action_type = UNKNOWN\n");
                        break;
                }
                vty_out(vty,"    forward port = %d\n", cmd_data.fccp_cmd.fccp_data.acl_info.acl_param.forward_port);

                vty_out(vty,"    dsa_info: 0x%08x\n", cmd_data.fccp_cmd.fccp_data.acl_info.acl_param.dsa_info);
                vty_out(vty,"    out_type:0x%02x   out_tag:0x%02x   in_type:0x%02x   in_tag:0x%02x\n",
                        cmd_data.fccp_cmd.fccp_data.acl_info.acl_param.out_ether_type, 
                        cmd_data.fccp_cmd.fccp_data.acl_info.acl_param.out_tag, 
                        cmd_data.fccp_cmd.fccp_data.acl_info.acl_param.in_ether_type, 
                        cmd_data.fccp_cmd.fccp_data.acl_info.acl_param.in_tag);
                vty_out(vty,"    action mask = 0x%x\n", cmd_data.fccp_cmd.fccp_data.acl_info.acl_param.action_mask);

				/*add by wangjian for support pppoe 2013-3-15*/
				if (cmd_data.fccp_cmd.fccp_data.acl_info.acl_param.pppoe_flag == 1)
				{
					vty_out(vty,"    pppoe_info:    session_id:%u \n", cmd_data.fccp_cmd.fccp_data.acl_info.acl_param.pppoe_session_id);
				}
			}
            //if learned
        }
display_end:
        acl_static_cnt -= loop;
        if(acl_static_cnt == 0)
        {
            return CMD_SUCCESS;
        }
        else
        {
            vty_out(vty,"c[continue] q[quit]\n");
            while(1)
            {
                int flag=getchar();
                if((flag == (int)('c')) || (flag == (int)(' ')))
                    goto display_loop;
                else if(flag == (int)('q'))
                    return CMD_SUCCESS;
                else
                    continue;
            }
        }
        return CMD_SUCCESS;
}

DEFUN(fastfwd_equipment_test_func,
	fastfwd_equipment_test_cmd,
	"equipment test (enable | disable) TYPE",
	"equipment\n"
	"test\n"
	"enable\n"
	"disable\n"
	"board type (AC4X)\n"
	)
{
	se_interative_t  cmd_data;
	int ret;
	
	memset(&cmd_data,0,sizeof(cmd_data));

    if(strcmp(argv[0], "enable") == 0)
    {
        cmd_data.fccp_cmd.fccp_data.equipment_test.enable = FUNC_ENABLE;
    }
    else if(strcmp(argv[0], "disable") == 0)
    {
        cmd_data.fccp_cmd.fccp_data.equipment_test.enable = FUNC_DISABLE;
    }
    else
        return CMD_FAILURE;
        
    if(strcmp(argv[1], "AC4X") == 0)
    {
        cmd_data.fccp_cmd.fccp_data.equipment_test.board_type = AUTELAN_BOARD_AX81_AC_4X;
    }
	strncpy(cmd_data.hand_cmd,SE_AGENT_EQUIPMENT_TEST_ENABLE,strlen(SE_AGENT_EQUIPMENT_TEST_ENABLE));
	ret=sendto_agent(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),vty);
	if(ret<=0)
	{
		vty_out(vty,WRITE_FAIL_STR);
		return CMD_FAILURE;
	}  

	return CMD_SUCCESS;
}

/* fast-fwd group add for logsave */
DEFUN(config_fwd_debug_log_enable_func,
	  config_fwd_debug_log_enable_cmd,
	  "config  fast-log (enable|disable)",
	  CONFIG_STR
	  "fast-log  \n"
	  "fast_forward debug log to memory enable\n"
	  "fast_forward debug log to memory disable\n"
)
{
	
	char *enable = (char *)argv[0];
	int flag = FUNC_DISABLE;
	se_interative_t  cmd_data;
	int ret = -1;
	struct timeval overtime;
	memset(&overtime, 0, sizeof(overtime));
	memset(&cmd_data, 0, sizeof(cmd_data));
	if(argc > 1) 
	{
		vty_out(vty, CMD_PARAMETER_NUM_ERROR);
		return CMD_WARNING;
	}
	if(0 == strncmp(enable, "disable", strlen(enable)))
	{
		flag=FUNC_DISABLE;
	}
	else if(0 == strncmp(enable, "enable", strlen(enable)))
	{
		flag=FUNC_ENABLE;
	}
	else
	{
		vty_out(vty, CMD_PARAMETER_ERROR);
		return CMD_FAILURE;
	}
	cmd_data.fccp_cmd.fccp_data.module_enable = flag;
	
	strncpy(cmd_data.hand_cmd, SE_AGENT_CONFIG_FWDLOG_ENABLE, strlen(SE_AGENT_CONFIG_FWDLOG_ENABLE));
	ret = sendto_agent(dcli_sockfd, (char*)&cmd_data, sizeof(cmd_data), vty);
	if(ret <= 0)
	{
		vty_out(vty, WRITE_FAIL_STR);
		return CMD_FAILURE;
	}
	overtime.tv_sec = DCLI_WAIT_TIME;
	ret = read_within_time(dcli_sockfd, (char*)&cmd_data, sizeof(cmd_data), &overtime);
	if(ret == READ_ERROR)
	{
		vty_out(vty, AGENT_NO_RESPOND_STR);
		return CMD_FAILURE;
	}
	if(cmd_data.cmd_result != AGENT_RETURN_OK)
	{
		vty_out(vty, "%s\n", cmd_data.err_info);
		return CMD_FAILURE;
	}
	
	return CMD_SUCCESS;
}

DEFUN(show_fwd_debug_log_enable_func,
	  show_fwd_debug_log_enable_cmd,
	  "show  fast-log enable",
	  SHOW_STR
	  "fast-log-mem  \n"
	  "fast_forward debug log to memory whether enable \n"
)
{
	se_interative_t  cmd_data;
	int ret = 0;
	struct timeval overtime;
	memset(&overtime, 0, sizeof(overtime));
	memset(&cmd_data, 0, sizeof(cmd_data));
	strncpy(cmd_data.hand_cmd, SE_AGENT_SHOW_FWDLOG_ENABLE, strlen(SE_AGENT_SHOW_FWDLOG_ENABLE));
	ret = sendto_agent(dcli_sockfd, (char*)&cmd_data, sizeof(cmd_data), vty);
	if(ret <= 0)
	{
		vty_out(vty,WRITE_FAIL_STR);
		return CMD_FAILURE;
	}
	memset(&cmd_data, 0, sizeof(cmd_data));
	overtime.tv_sec = DCLI_WAIT_TIME;
 	ret = read_within_time(dcli_sockfd, (char*)&cmd_data, sizeof(cmd_data), &overtime);
 	if(ret == READ_ERROR)
	{
		vty_out(vty, AGENT_NO_RESPOND_STR);
		return CMD_FAILURE;
	}
	
	if(cmd_data.cmd_result != AGENT_RETURN_OK)
	{
		vty_out(vty, "%s\n", cmd_data.err_info);
		return CMD_FAILURE;
	}
	if(cmd_data.fccp_cmd.ret_val != FCCP_RETURN_OK)
	{
		vty_out(vty, COMMAND_FAIL_STR);
		return CMD_FAILURE;
	}
	vty_out(vty, "fast_forward log to memory is %s\n", (cmd_data.fccp_cmd.fccp_data.module_enable == 1) ? "enable ": "disable");

	return CMD_SUCCESS;
}


/*dcli command set fwd log_level*/
DEFUN(config_fwd_debug_log_level_func,
	  config_fwd_debug_log_level_cmd,
	  "config  fast-log-level LEVEL",
	  CONFIG_STR
	  "fastfwd debug log level \n"
	  "the debug log level range(1-5)\n"
)
{
	se_interative_t  cmd_data;
	int ret = 0;
	uint32_t log_level = 0;
	struct timeval overtime;
	memset(&overtime,0,sizeof(overtime));
	memset(&cmd_data,0,sizeof(cmd_data));
	if(COMMON_ERROR == parse_isnum((char *)argv[0]))
	{
		vty_out(vty, CMD_PARAMETER_ERROR);
		return CMD_FAILURE;
	}
	log_level = strtoul((char*)argv[0], NULL, 10);
	if((log_level < MIN_LOG_LEVEL) || (log_level > MAX_LOG_LEVEL))
	{
		vty_out(vty," %%log level value %u--%u\n",MIN_LOG_LEVEL,MAX_LOG_LEVEL);
		return CMD_FAILURE;
	}
	cmd_data.fccp_cmd.fccp_data.share = log_level;
	strncpy(cmd_data.hand_cmd, SE_AGENT_CONFIG_FWDLOG_LEVEL, strlen(SE_AGENT_CONFIG_FWDLOG_LEVEL));
	ret = sendto_agent(dcli_sockfd,(char*)&cmd_data, sizeof(cmd_data), vty);
 	if(ret <= 0)
 	{
		vty_out(vty, WRITE_FAIL_STR);
		return CMD_FAILURE;
	}
	memset(&cmd_data, 0, sizeof(cmd_data));
	overtime.tv_sec = DCLI_WAIT_TIME;
	ret = read_within_time(dcli_sockfd, (char*)&cmd_data, sizeof(cmd_data), &overtime);
	if(ret == READ_ERROR)
	{
		vty_out(vty, AGENT_NO_RESPOND_STR);
		return CMD_FAILURE;
	}
	if(cmd_data.cmd_result != AGENT_RETURN_OK)
	{
		vty_out(vty,"%s\n",cmd_data.err_info);
		return CMD_FAILURE;
	}
	if(cmd_data.fccp_cmd.ret_val != FCCP_RETURN_OK)
	{
		vty_out(vty,COMMAND_FAIL_STR);
		return CMD_FAILURE;
	}
	
	return CMD_SUCCESS;
}

DEFUN(show_fwd_debug_log_leve_func,
	  show_fwd_debug_log_level_cmd,
	  "show  fast-log-level",
	  SHOW_STR
	  "fast-log-level  \n"
)
{
	se_interative_t  cmd_data;
	int ret = 0;
	struct timeval overtime;
	memset(&overtime, 0, sizeof(overtime));
	memset(&cmd_data, 0, sizeof(cmd_data));
	strncpy(cmd_data.hand_cmd, SE_AGENT_SHOW_FWDLOG_LEVEL, strlen(SE_AGENT_SHOW_FWDLOG_LEVEL ));
	ret = sendto_agent(dcli_sockfd, (char*)&cmd_data, sizeof(cmd_data),vty);
 	if(ret <= 0)
 	{
		vty_out(vty,WRITE_FAIL_STR);
		return CMD_FAILURE;
	}
	memset(&cmd_data, 0, sizeof(cmd_data));
	overtime.tv_sec = DCLI_WAIT_TIME;
	ret = read_within_time(dcli_sockfd, (char*)&cmd_data, sizeof(cmd_data), &overtime);
	if(ret == READ_ERROR)
	{
		vty_out(vty, AGENT_NO_RESPOND_STR);
		return CMD_FAILURE;
	}
	
	if(cmd_data.cmd_result != AGENT_RETURN_OK)
	{
 		vty_out(vty, "%s\n", cmd_data.err_info);
 		return CMD_FAILURE;
	}
	if(cmd_data.fccp_cmd.ret_val != FCCP_RETURN_OK)
	{
		vty_out(vty, COMMAND_FAIL_STR);
		return CMD_FAILURE;
	}
	vty_out(vty, "fast_forward log level is %u\n", cmd_data.fccp_cmd.fccp_data.share);

	return CMD_SUCCESS;
}

DEFUN(clear_fwd_debug_log_func,
	clear_fwd_debug_log_cmd,
	"clear fast-forward log",
	CLEAR_STR
	"fastfwd module\n"
	"fastfwd log clear\n"
	)
{
	se_interative_t  cmd_data;
	int ret = 0;
	struct timeval overtime;
	memset(&overtime,0,sizeof(overtime));
	memset(&cmd_data,0,sizeof(cmd_data));
	strncpy(cmd_data.hand_cmd,SE_AGENT_CLEAR_FWDLOG,strlen(SE_AGENT_CLEAR_FWDLOG));
	ret=sendto_agent(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data), vty);
	if(ret<=0)
	{
		vty_out(vty,WRITE_FAIL_STR);
		return CMD_FAILURE;
	}
	memset(&cmd_data,0,sizeof(cmd_data));
	overtime.tv_sec = DCLI_WAIT_TIME;
	ret=read_within_time(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),&overtime);
	if(ret==READ_ERROR)
	{
		vty_out(vty,AGENT_NO_RESPOND_STR);
		return CMD_FAILURE;
	}
	if(cmd_data.cmd_result!=AGENT_RETURN_OK)
	{
		vty_out(vty,"%s\n",cmd_data.err_info);
		return CMD_FAILURE;
	}

	return CMD_SUCCESS;
}


/*2013.2.28
Command for showing information of user table,including the total num,the used num,and also the information
of specific user*/
DEFUN(show_user_statistic_func,
	show_user_statistic_cmd,
	"show user statistic",
	SHOW_STR
	"user\n"
	"fast forward user rule information\n"
)
{
	se_interative_t  cmd_data;
	int ret = -1;
	struct timeval overtime;
	
	memset(&overtime,0,sizeof(overtime));
	memset(&cmd_data,0,sizeof(cmd_data));
	strncpy(cmd_data.hand_cmd,SE_AGENT_SHOW_FWD_USER_STATS,strlen(SE_AGENT_SHOW_FWD_USER_STATS));
	
	ret = sendto_agent(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),vty);
	if(ret<=0)
	{
		vty_out(vty,WRITE_FAIL_STR);
		return CMD_FAILURE;
	}
	memset(&cmd_data,0,sizeof(cmd_data));
	overtime.tv_sec=DCLI_WAIT_TIME;
	ret=read_within_time(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),&overtime);
	if(ret==READ_ERROR)
	{
		vty_out(vty,AGENT_NO_RESPOND_STR);
		return CMD_FAILURE;
	}
	if(cmd_data.cmd_result!=AGENT_RETURN_OK)
	{
		vty_out(vty,"%s\n",cmd_data.err_info);
		return CMD_FAILURE;
	}

	vty_out(vty,"==============================================\n");
	vty_out(vty,"user_bucket_tbl count:\n");
	vty_out(vty,"  entry num: %u\n", cmd_data.fccp_cmd.fccp_data.user_stats.user_static_tbl_size);
	vty_out(vty,"  free num: %u\n", (cmd_data.fccp_cmd.fccp_data.user_stats.user_static_tbl_size - \
		                               cmd_data.fccp_cmd.fccp_data.user_stats.s_tbl_used_rule));
	vty_out(vty,"  used num: %u\n", cmd_data.fccp_cmd.fccp_data.user_stats.s_tbl_used_rule);
	vty_out(vty,"  use rate: %02f%%\n", (float)cmd_data.fccp_cmd.fccp_data.user_stats.s_tbl_used_rule/ \
		                                   (float)cmd_data.fccp_cmd.fccp_data.user_stats.user_static_tbl_size *100);
	vty_out(vty,"==============================================\n");
    vty_out(vty,"user_dynamic_tbl count:\n");
	vty_out(vty,"  entry num: %u\n", cmd_data.fccp_cmd.fccp_data.user_stats.user_dynamic_tbl_size);
	vty_out(vty,"  free num: %u\n", (cmd_data.fccp_cmd.fccp_data.user_stats.user_dynamic_tbl_size - \
		                               cmd_data.fccp_cmd.fccp_data.user_stats.d_tbl_used_rule));
	vty_out(vty,"  used num: %u\n", cmd_data.fccp_cmd.fccp_data.user_stats.d_tbl_used_rule);
	vty_out(vty,"  use rate: %02f%%\n", (float)cmd_data.fccp_cmd.fccp_data.user_stats.d_tbl_used_rule/ \
		                                   (float)cmd_data.fccp_cmd.fccp_data.user_stats.user_dynamic_tbl_size *100);
	vty_out(vty,"==============================================\n");

	return CMD_SUCCESS;
}

DEFUN(show_user_rule_all_func,
	show_user_rule_all_cmd,
	"show user rule all",
	SHOW_STR
	"user\n"
	"fast forward user rule information\n"
	"all user info\n"
)
{
	se_interative_t  cmd_data;
	int ret = -1;
	struct timeval overtime;
	int32_t static_index = 0;
	uint32_t dynamic_index = 0;
	int loop=5,i=0,total_count = 0;

	while(1)
	{
    	for(i=0;i<loop;i++)
    	{
    		memset(&overtime,0,sizeof(overtime));
    		memset(&cmd_data,0,sizeof(cmd_data));
    		cmd_data.fccp_cmd.fccp_data.user_rule.static_index = static_index;
    		cmd_data.fccp_cmd.fccp_data.user_rule.dynamic_index = dynamic_index;
    		strncpy(cmd_data.hand_cmd,SE_AGENT_SHOW_FWD_USER_RULE_ALL,strlen(SE_AGENT_SHOW_FWD_USER_RULE_ALL));
    		ret=sendto_agent(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),vty);
    		if(ret<=0)
    		{
    			vty_out(vty,WRITE_FAIL_STR);
    			return CMD_FAILURE;
    		}
    		memset(&cmd_data,0,sizeof(cmd_data));
    		overtime.tv_sec=DCLI_WAIT_TIME;
    		ret=read_within_time(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),&overtime);
    		if(ret==READ_ERROR)
    		{
    			vty_out(vty,AGENT_NO_RESPOND_STR);
    			return CMD_FAILURE;
    		}
    		if(cmd_data.cmd_result!=AGENT_RETURN_OK)
    		{
    			vty_out(vty,"%s\n",cmd_data.err_info);
    			return CMD_FAILURE;
    		}
            if(cmd_data.fccp_cmd.fccp_data.user_rule.static_index == -1)
            {
                if(total_count == 0)
                    vty_out(vty,"%%No any user\n");
                return CMD_SUCCESS;
            }
    		dcli_show_user_rule(&cmd_data,vty);
    		total_count++;
    		static_index = cmd_data.fccp_cmd.fccp_data.user_rule.static_index;
    		dynamic_index = cmd_data.fccp_cmd.fccp_data.user_rule.dynamic_index;
    	}
    	vty_out(vty,"c[continue] q[quit]\n");
    	while(1)
    	{
    		int flag=getchar();
    		if((flag== (int)('c')) || (flag == (int)(' ')))
    			break;
    		else if(flag == (int)('q'))
    			return CMD_SUCCESS;
    	}
    }
}
/*Set the interval of clearing aged rules*/
DEFUN(config_clear_aged_rule_time_func,
		  config_clear_aged_rule_time_cmd,
		  "config clear-aged-rule-time TIME",
		  CONFIG_STR
		  "clear time\n"
		  "seconds that can be divided by 5s,if 0,clear-aged-rule will be stopped.\n"
)
{
	se_interative_t  cmd_data;
	int ret = 0;
	uint32_t aging_time = 0;
	struct timeval overtime;
	memset(&overtime,0,sizeof(overtime));
	memset(&cmd_data,0,sizeof(cmd_data));
	if(COMMON_ERROR == parse_isnum((char *)argv[0]))
	{
		vty_out(vty,CMD_PARAMETER_ERROR);
		return CMD_FAILURE;
	}
	aging_time = strtoul((char*)argv[0],NULL,10);
	if((aging_time < MIN_AGING_TIME - 1) || aging_time > MAX_AGING_TIME )
	{
		vty_out(vty," %%interval time value %u--%u\n",MIN_AGING_TIME-1,MAX_AGING_TIME);
		return CMD_FAILURE;
	}
	cmd_data.fccp_cmd.fccp_data.aging_timer = aging_time;
	strncpy(cmd_data.hand_cmd,SE_AGENT_SET_CLEAR_AGED_RULE_TIME,strlen(SE_AGENT_SET_CLEAR_AGED_RULE_TIME));
	ret=sendto_agent(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),vty);
	if(ret<=0)
	{
		vty_out(vty,WRITE_FAIL_STR);
		return CMD_FAILURE;
	}
	memset(&cmd_data,0,sizeof(cmd_data));
	overtime.tv_sec=DCLI_WAIT_TIME;
	ret=read_within_time(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),&overtime);
	if(ret==READ_ERROR)
	{
		vty_out(vty,AGENT_NO_RESPOND_STR);
		return CMD_FAILURE;
	}
	if(cmd_data.cmd_result!=AGENT_RETURN_OK)
	{
		vty_out(vty,"%s\n",cmd_data.err_info);
		return CMD_FAILURE;
	}
	
	return CMD_SUCCESS;
}

/*Get the interval of clearing aged rules*/
DEFUN(show_clear_aged_rule_time_func,
		  show_clear_aged_rule_time_cmd,
		  "show clear-aged-rule-time",
		  SHOW_STR
		  "time (second)\n"
)
{
	se_interative_t  cmd_data;
	int ret = 0;
	struct timeval overtime;
	memset(&overtime,0,sizeof(overtime));
	memset(&cmd_data,0,sizeof(cmd_data));
	strncpy(cmd_data.hand_cmd,SE_AGENT_GET_CLEAR_AGED_RULE_TIME,strlen(SE_AGENT_GET_CLEAR_AGED_RULE_TIME));
	ret=sendto_agent(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),vty);
	if(ret<=0)
	{
		vty_out(vty,WRITE_FAIL_STR);
		return CMD_FAILURE;
	}
	memset(&cmd_data,0,sizeof(cmd_data));
	overtime.tv_sec=DCLI_WAIT_TIME;
	ret=read_within_time(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),&overtime);
	if(ret==READ_ERROR)
	{
		vty_out(vty,AGENT_NO_RESPOND_STR);
		return CMD_FAILURE;
	}
	
	if(cmd_data.cmd_result!=AGENT_RETURN_OK)
	{
		vty_out(vty,"%s\n",cmd_data.err_info);
		return CMD_FAILURE;
	}
	
	vty_out(vty,"Interval time of clearing aged rule is %u\n",cmd_data.fccp_cmd.fccp_data.aging_timer);
	return CMD_SUCCESS;
}

/*Show information of specific user*/
DEFUN(show_user_rule_ip_func,
	show_user_rule_ip_cmd,
	"show user rule by ip A.B.C.D",
	SHOW_STR
	"user \n"
	"user flow statistic information\n"
	"by \n"
	"ip \n"
	"user ip address A.B.C.D \n")
{	
	se_interative_t  cmd_data;
	int ret = 0;
	struct timeval overtime;
	memset(&overtime,0,sizeof(overtime));
	memset(&cmd_data,0,sizeof(cmd_data));
	if(argc > 1)
	{
		vty_out(vty,CMD_PARAMETER_ERROR);
		return CMD_FAILURE;
	}
	if(COMMON_ERROR==parse_ip_check((char*)argv[0]))
	{
		vty_out(vty,CMD_PARAMETER_ERROR);
		return CMD_FAILURE;	
	}
	
	cmd_data.fccp_cmd.fccp_data.user_rule.user_info.user_ip = dcli_ip2ulong((char*)argv[0]);
	strncpy(cmd_data.hand_cmd,SE_AGENT_SHOW_USER_RULE_BY_IP,strlen(SE_AGENT_SHOW_USER_RULE_BY_IP));
	ret=sendto_agent(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),vty);
	if(ret<=0)
	{
		vty_out(vty,WRITE_FAIL_STR);
		return CMD_FAILURE;
	}
	memset(&cmd_data,0,sizeof(cmd_data));
	overtime.tv_sec=10;
	ret=read_within_time(dcli_sockfd,(char*)&cmd_data,sizeof(cmd_data),&overtime);
	if(ret==READ_ERROR)
	{
		vty_out(vty,AGENT_NO_RESPOND_STR);
		return CMD_FAILURE;
	}
	
	if(cmd_data.cmd_result!=AGENT_RETURN_OK)
	{
		vty_out(vty,"%s\n",cmd_data.err_info);
		return CMD_FAILURE;
	}
	dcli_show_user_rule(&cmd_data, vty);
	return CMD_SUCCESS;
}

void dcli_se_agent_init(void)
{
	if(CMD_FAILURE == se_agent_init_address(MAX_SLOT_NUM,se_agent_address))
	{
		return ;
	}
	dcli_sockfd = socket(AF_TIPC,SOCK_DGRAM,0);
	if(dcli_sockfd < 0)
	{
		return ;
	}	
	install_node(&fwd_node,se_agent_show_running_cfg,"FAST_FWD_NODE");
	install_default(FAST_FWD_NODE);
	install_node(&slave_fwd_node,se_agent_show_running_cfg,"SLAVE_FAST_FWD_NODE");
	install_default(SLAVE_FAST_FWD_NODE);
	install_element(ENABLE_NODE,&show_fast_forward_running_cfg_cmd);
	install_element(CONFIG_NODE,&debug_se_agent_cmd);
	install_element(CONFIG_NODE,&config_fastfwd_cmd);
	install_element(CONFIG_NODE,&no_debug_se_agent_cmd);
	install_element(FAST_FWD_NODE,&config_fast_forward_aging_time_cmd);
	install_element(FAST_FWD_NODE,&switch_uart_to_se_cmd);
	install_element(FAST_FWD_NODE,&show_fpa_buff_counter_cmd);
	install_element(FAST_FWD_NODE,&delete_fast_forward_rule_cmd);
	install_element(FAST_FWD_NODE,&config_fast_forward_tag_type_cmd);
	install_element(FAST_FWD_NODE,&show_fast_forward_running_config_cmd);
	install_element(FAST_FWD_NODE,&show_fast_forward_tag_type_cmd);
	install_element(FAST_FWD_NODE,&show_fast_forward_aging_time_cmd);
	install_element(FAST_FWD_NODE,&read_register_fun_cmd);
	install_element(FAST_FWD_NODE,&write_register_fun_cmd);
	install_element(FAST_FWD_NODE,&show_packet_statistic_cmd);
	install_element(FAST_FWD_NODE,&show_part_packet_statistic_cmd);
	install_element(FAST_FWD_NODE,&clear_packet_statistic_cmd);
	//install_element(FAST_FWD_NODE,&clear_part_packet_statistic_cmd);
	install_element(FAST_FWD_NODE,&debug_ipfwd_learn_cmd);
	install_element(FAST_FWD_NODE,&fastfwd_learned_icmp_enable_cmd);
	install_element(FAST_FWD_NODE,&fastfwd_pure_ip_enable_cmd);
	install_element(FAST_FWD_NODE,&show_fwd_pure_ip_enable_cmd);
	install_element(FAST_FWD_NODE,&show_rule_stats_cmd);
	install_element(FAST_FWD_NODE,&clear_rule_all_cmd);
	install_element(FAST_FWD_NODE,&clear_aging_rule_cmd);
	install_element(FAST_FWD_NODE,&show_capwap_tbl_cmd);
	install_element(FAST_FWD_NODE,&config_fast_forward_enable_cmd);
	install_element(FAST_FWD_NODE,&show_rule_five_tuple_cmd);
	install_element(FAST_FWD_NODE,&show_aging_rule_cnt_cmd);
	install_element(FAST_FWD_NODE,&show_user_acl_stats_cmd);
	install_element(FAST_FWD_NODE,&show_acl_learned_cmd);
	install_element(FAST_FWD_NODE,&show_acl_learning_cmd);
	install_element(FAST_FWD_NODE,&show_tolinux_flow_cmd);
	install_element(FAST_FWD_NODE,&set_fastfwd_bucket_entry_cmd);
	install_element(FAST_FWD_NODE,&show_fastfwd_bucket_entry_cmd);	
	install_element(FAST_FWD_NODE,&config_traffic_monitor_cmd);
	install_element(FAST_FWD_NODE,&clear_traffic_monitor_cmd);
	install_element(FAST_FWD_NODE,&show_traffic_monitor_cmd);
	install_element(FAST_FWD_NODE,&config_pure_payload_acct_cmd);
	install_element(FAST_FWD_NODE,&show_user_flow_cmd);
	install_element(FAST_FWD_NODE,&clear_rule_ip_cmd);          /*wangjian clear  */
	install_element(FAST_FWD_NODE,&show_fast_forward_info_cmd);     /*wangjian 2012.07.09 add fwd info */
	install_element(FAST_FWD_NODE,&show_rule_by_ip_cmd);         	/*wangjian 2012.07.09 add ip */
	//install_element(FAST_FWD_NODE,&config_fwd_debug_log_enable_cmd);  
	//install_element(FAST_FWD_NODE,&show_fwd_debug_log_enable_cmd);  
	//install_element(FAST_FWD_NODE,&config_fwd_debug_log_level_cmd);
	//install_element(FAST_FWD_NODE,&show_fwd_debug_log_level_cmd);
	//install_element(FAST_FWD_NODE,&fastfwd_equipment_test_cmd);		/*zhaohan 2012.8.22 equipment test*/
	install_element(FAST_FWD_NODE,&show_user_statistic_cmd);
	install_element(FAST_FWD_NODE,&config_clear_aged_rule_time_cmd);
	install_element(FAST_FWD_NODE,&show_clear_aged_rule_time_cmd);
	install_element(FAST_FWD_NODE,&show_user_rule_ip_cmd);
	install_element(FAST_FWD_NODE,&show_user_rule_all_cmd);
	
	/*HANSI_NODE*/
	install_element(HANSI_NODE,&config_fast_forward_aging_time_cmd);
	install_element(HANSI_NODE,&show_fast_forward_aging_time_cmd);
	install_element(HANSI_NODE,&show_packet_statistic_cmd);
	install_element(HANSI_NODE,&show_part_packet_statistic_cmd);
	install_element(HANSI_NODE,&clear_packet_statistic_cmd);
	//install_element(HANSI_NODE,&clear_part_packet_statistic_cmd);
	install_element(HANSI_NODE,&fastfwd_learned_icmp_enable_cmd);
	install_element(HANSI_NODE,&fastfwd_pure_ip_enable_cmd);
	install_element(HANSI_NODE,&show_fwd_pure_ip_enable_cmd);
	install_element(HANSI_NODE,&show_fpa_buff_counter_cmd);
	install_element(HANSI_NODE,&config_fast_forward_tag_type_cmd);
	install_element(HANSI_NODE,&show_fast_forward_running_config_cmd);
	install_element(HANSI_NODE,&show_fast_forward_tag_type_cmd);
	install_element(HANSI_NODE,&show_rule_stats_cmd);
	install_element(HANSI_NODE,&clear_rule_all_cmd);
	install_element(HANSI_NODE,&clear_aging_rule_cmd);
	install_element(HANSI_NODE,&show_capwap_tbl_cmd);
	install_element(HANSI_NODE,&config_fast_forward_enable_cmd);
	install_element(HANSI_NODE,&show_rule_five_tuple_cmd);
	install_element(HANSI_NODE,&show_aging_rule_cnt_cmd);
	install_element(HANSI_NODE,&show_user_acl_stats_cmd);
	install_element(HANSI_NODE,&show_acl_learned_cmd);
	install_element(HANSI_NODE,&show_acl_learning_cmd);
	install_element(HANSI_NODE,&show_tolinux_flow_cmd);
	install_element(HANSI_NODE,&set_fastfwd_bucket_entry_cmd);
	install_element(HANSI_NODE,&show_fastfwd_bucket_entry_cmd);
	install_element(HANSI_NODE,&config_traffic_monitor_cmd);
	install_element(HANSI_NODE,&clear_traffic_monitor_cmd);
	install_element(HANSI_NODE,&show_traffic_monitor_cmd);
	install_element(HANSI_NODE,&config_pure_payload_acct_cmd);
	install_element(HANSI_NODE,&show_user_flow_cmd);
	install_element(HANSI_NODE,&clear_rule_ip_cmd); //wangjian
	install_element(HANSI_NODE,&show_fast_forward_info_cmd);     /*wangjian 2012.07.09 add fwd info */
	install_element(HANSI_NODE,&show_rule_by_ip_cmd);         /*wangjian 2012.07.09 add ip */	
	//install_element(HANSI_NODE,&config_fwd_debug_log_enable_cmd);
	//install_element(HANSI_NODE,&show_fwd_debug_log_enable_cmd); 
	//install_element(HANSI_NODE,&config_fwd_debug_log_level_cmd);
	//install_element(HANSI_NODE,&show_fwd_debug_log_level_cmd);
	install_element(HANSI_NODE,&show_user_statistic_cmd);
	install_element(HANSI_NODE,&config_clear_aged_rule_time_cmd);
	install_element(HANSI_NODE,&show_clear_aged_rule_time_cmd);
	install_element(HANSI_NODE,&show_user_rule_ip_cmd);
	install_element(HANSI_NODE,&show_user_rule_all_cmd);

	
	
	/*HANSI_NODE*/
	install_element(LOCAL_HANSI_NODE,&config_fast_forward_aging_time_cmd);
	install_element(LOCAL_HANSI_NODE,&show_fast_forward_aging_time_cmd);
	install_element(LOCAL_HANSI_NODE,&show_packet_statistic_cmd);
	install_element(LOCAL_HANSI_NODE,&show_part_packet_statistic_cmd);
	install_element(LOCAL_HANSI_NODE,&clear_packet_statistic_cmd);
	//install_element(LOCAL_HANSI_NODE,&clear_part_packet_statistic_cmd);
	install_element(LOCAL_HANSI_NODE,&fastfwd_learned_icmp_enable_cmd);
	install_element(LOCAL_HANSI_NODE,&fastfwd_pure_ip_enable_cmd);
	install_element(LOCAL_HANSI_NODE,&show_fwd_pure_ip_enable_cmd);
	install_element(LOCAL_HANSI_NODE,&show_fpa_buff_counter_cmd);
	install_element(LOCAL_HANSI_NODE,&config_fast_forward_tag_type_cmd);
	install_element(LOCAL_HANSI_NODE,&show_fast_forward_running_config_cmd);
	install_element(LOCAL_HANSI_NODE,&show_fast_forward_tag_type_cmd);
	install_element(LOCAL_HANSI_NODE,&show_rule_stats_cmd);
	install_element(LOCAL_HANSI_NODE,&clear_rule_all_cmd);
	install_element(LOCAL_HANSI_NODE,&clear_aging_rule_cmd);
	install_element(LOCAL_HANSI_NODE,&show_capwap_tbl_cmd);
	install_element(LOCAL_HANSI_NODE,&config_fast_forward_enable_cmd);
	install_element(LOCAL_HANSI_NODE,&show_rule_five_tuple_cmd);
	install_element(LOCAL_HANSI_NODE,&show_aging_rule_cnt_cmd);
	install_element(LOCAL_HANSI_NODE,&show_user_acl_stats_cmd);
	install_element(LOCAL_HANSI_NODE,&show_acl_learned_cmd);
	install_element(LOCAL_HANSI_NODE,&show_acl_learning_cmd);
	install_element(LOCAL_HANSI_NODE,&show_tolinux_flow_cmd);
	install_element(LOCAL_HANSI_NODE,&set_fastfwd_bucket_entry_cmd);
	install_element(LOCAL_HANSI_NODE,&show_fastfwd_bucket_entry_cmd);
	install_element(LOCAL_HANSI_NODE,&config_traffic_monitor_cmd);
	install_element(LOCAL_HANSI_NODE,&clear_traffic_monitor_cmd);
	install_element(LOCAL_HANSI_NODE,&show_traffic_monitor_cmd);
	install_element(LOCAL_HANSI_NODE,&config_pure_payload_acct_cmd);
	install_element(LOCAL_HANSI_NODE,&show_user_flow_cmd);
	install_element(LOCAL_HANSI_NODE,&clear_rule_ip_cmd); //wangjian
	install_element(LOCAL_HANSI_NODE,&show_fast_forward_info_cmd);     /*wangjian 2012.07.09 add fwd info */
	install_element(LOCAL_HANSI_NODE,&show_rule_by_ip_cmd);         /*wangjian 2012.07.09 add ip */	
	//install_element(LOCAL_HANSI_NODE,&config_fwd_debug_log_enable_cmd);
	//install_element(LOCAL_HANSI_NODE,&show_fwd_debug_log_enable_cmd); 
	//install_element(LOCAL_HANSI_NODE,&config_fwd_debug_log_level_cmd);
	//install_element(LOCAL_HANSI_NODE,&show_fwd_debug_log_level_cmd);
	install_element(LOCAL_HANSI_NODE,&show_user_statistic_cmd);
	install_element(LOCAL_HANSI_NODE,&config_clear_aged_rule_time_cmd);
	install_element(LOCAL_HANSI_NODE,&show_clear_aged_rule_time_cmd);
	install_element(LOCAL_HANSI_NODE,&show_user_rule_ip_cmd);
	install_element(LOCAL_HANSI_NODE,&show_user_rule_all_cmd);

		
//#ifdef DCLI_SE_AGENT_DEBUG
	install_element(FAST_FWD_NODE,&set_user_offline_cmd);
	install_element(FAST_FWD_NODE,&set_user_online_cmd);
//#endif

	install_element(HIDDENDEBUG_NODE,&config_fwd_debug_log_enable_cmd);  /* fast-fwd group add for  logsave */
	install_element(HIDDENDEBUG_NODE,&show_fwd_debug_log_enable_cmd); 
	install_element(HIDDENDEBUG_NODE,&config_fwd_debug_log_level_cmd);
	install_element(HIDDENDEBUG_NODE,&show_fwd_debug_log_level_cmd);
	install_element(HIDDENDEBUG_NODE,&clear_fwd_debug_log_cmd);
	install_element(HIDDENDEBUG_NODE,&fastfwd_equipment_test_cmd);		/*zhaohan 2012.8.22 equipment test*/
	
    /*SLAVE_FAST_FWD_NODE*/
    install_element(CONFIG_NODE,&config_slave_fastfwd_cmd);
	install_element(SLAVE_FAST_FWD_NODE,&config_fast_forward_aging_time_cmd);
	install_element(SLAVE_FAST_FWD_NODE,&show_fpa_buff_counter_cmd);
	install_element(SLAVE_FAST_FWD_NODE,&config_fast_forward_tag_type_cmd);
	install_element(SLAVE_FAST_FWD_NODE,&show_fast_forward_tag_type_cmd);
	install_element(SLAVE_FAST_FWD_NODE,&show_fast_forward_aging_time_cmd);
	install_element(SLAVE_FAST_FWD_NODE,&show_packet_statistic_cmd);
	install_element(SLAVE_FAST_FWD_NODE,&show_part_packet_statistic_cmd);
	install_element(SLAVE_FAST_FWD_NODE,&clear_packet_statistic_cmd);
	//install_element(SLAVE_FAST_FWD_NODE,&clear_part_packet_statistic_cmd);
	install_element(SLAVE_FAST_FWD_NODE,&show_rule_stats_cmd);
	install_element(SLAVE_FAST_FWD_NODE,&clear_rule_all_cmd);
}
	
