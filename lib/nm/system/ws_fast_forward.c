/* cgicTempDir is the only setting you are likely to need
	to change in this file. */

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
* ws_fast_forward.c
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
#ifdef __cplusplus
	extern "C"
	{
#endif


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dbus/dbus.h>
//#include <sys/un.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <linux/un.h>
#include <linux/tipc.h>
#include <sys/select.h>
#include <sys/time.h>
#include <syslog.h>
#include <stdarg.h>
#include "sysdef/returncode.h"
#include "ws_fast_forward.h"






#define INVALID_SLOTID (-1)
#define INVALID_SOCKET (-1)
/*dcli local socket file description*/

int ccgi_sockfd = INVALID_SOCKET;

struct sockaddr_tipc se_agent_address[MAX_SLOT_NUM+1]={0};
int se_agent_addrlen = sizeof(struct sockaddr_tipc);

int ccgi_parse_ipport_check(char * str)
{
	char *sep=".";
	char *colon=":";
	char *token = NULL;
	char *tail_ptr=NULL;
	unsigned long ip_long[4] = {0}; 
	int i = 0;
	int port_num=0;
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
	port_num = strtoul(token,NULL,10);
	if((0>port_num)||( MAX_FAST_PORT < port_num))
	{
		return COMMON_ERROR;
	}
	
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

/*unsigned long dcli_ip2ulong(char *str)
{
	char *sep=".";
	char *token;
	unsigned long ip_long[4]; 
	unsigned long ip;
	int i = 1;
	
	token=strtok(str,sep);
	ip_long[0] = strtoul(token,NULL,10);
	while((token!=NULL)&&(i<4))
	{
		token=strtok(NULL,sep);
		ip_long[i] = strtoul(token,NULL,10);
		i++;
	}

	ip=(ip_long[0]<<24)+(ip_long[1]<<16)+(ip_long[2]<<8)+ip_long[3];

	return ip;
}*/


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

int ccgi_se_agent_init(void)
{
	if(CMD_FAILURE == se_agent_init_address(MAX_SLOT_NUM,se_agent_address))
	{
		return -1;
	}
	ccgi_sockfd = socket(AF_TIPC,SOCK_DGRAM,0);
	if(ccgi_sockfd < 0)
	{
		return -2;
	}	
	return 0;
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
 int ccgi_read_within_time(int fd,char *buf,unsigned int len,struct timeval *overtime)
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
 *  ccgi_sendto_agent
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
int ccgi_sendto_agent(int fd,char *buf,unsigned int len,int slot_id)
{
	int ret=WRITE_ERROR;
	char tmpbuf[1024]={0};
	fd_set  rset;
	int slotid = INVALID_SLOTID;
	struct timeval overtime={
		.tv_sec =0,
		.tv_usec=0
	};
	
	slotid=slot_id;
	
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

unsigned char ccgi_parse_protocol(char *str)
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

int ccgi_config_fast_forward_enable_cmd(char *state,int slot_id)
															//0:success;	-1:socket failed;-2:Failed:communication failure with agent module;
															//-3:Failed:agent is not responding;-4:cmd_data.cmd_result!=AGENT_RETURN_OK;
{
	int flag = FUNC_ENABLE;
	se_interative_t  cmd_data;
	int ret = -1;
	struct timeval overtime;
	memset(&overtime,0,sizeof(overtime));
	memset(&cmd_data,0,sizeof(cmd_data));

	ret=ccgi_se_agent_init();
	if(ret!=0)
	{
		return -1;
	}

	if(0==strncmp(state,"disable",strlen(state)))
	{
		flag=FUNC_DISABLE;
	}
	else if(0==strncmp(state,"enable",strlen(state)))
	{
		flag=FUNC_ENABLE;
	}

	
	cmd_data.fccp_cmd.fccp_data.module_enable=flag;
	strncpy(cmd_data.hand_cmd,SE_AGENT_FASTFWD_ENABLE,strlen(SE_AGENT_FASTFWD_ENABLE));
	ret=ccgi_sendto_agent(ccgi_sockfd,(char*)&cmd_data,sizeof(cmd_data),slot_id);
	if(ret<=0)
	{
		close(ccgi_sockfd);
		ccgi_sockfd=-1;
		return -2;
	}
	overtime.tv_sec=DCLI_WAIT_TIME;
	ret=ccgi_read_within_time(ccgi_sockfd,(char*)&cmd_data,sizeof(cmd_data),&overtime);
	if(ret==READ_ERROR)
	{
		close(ccgi_sockfd);
		ccgi_sockfd=-1;
		return -3;
	}
	if(cmd_data.cmd_result!=AGENT_RETURN_OK)
	{
		close(ccgi_sockfd);
		ccgi_sockfd=-1;
		return -4;
	}
	close(ccgi_sockfd);
	ccgi_sockfd=-1;
	return 0;
}

int ccgi_show_fast_forward_info_cmd(int slot_id,struct fast_fwd_info *info)
																//0:success;	-1:socket failed;-2:Failed:communication failure with agent module;
																//-3:Failed:agent is not responding;-4:cmd_data.cmd_result!=AGENT_RETURN_OK;-5:Failed:command failed
{
	se_interative_t  cmd_data;
	int ret = 0;
	struct timeval overtime;
	
	memset(&overtime,0,sizeof(overtime));
	memset(&cmd_data,0,sizeof(cmd_data));

	
	ret=ccgi_se_agent_init();
	if(ret!=0)
	{
		return -1;
	}

	strncpy(cmd_data.hand_cmd,SE_AGENT_SHOW_FAST_FWD_INFO,strlen(SE_AGENT_SHOW_FAST_FWD_INFO));
	ret=ccgi_sendto_agent(ccgi_sockfd,(char*)&cmd_data,sizeof(cmd_data),slot_id);
	if(ret<=0)
	{
		close(ccgi_sockfd);
		ccgi_sockfd=-1;
		return -2;
	}
	
	memset(&cmd_data,0,sizeof(cmd_data));
	overtime.tv_sec=DCLI_WAIT_TIME;
	ret=ccgi_read_within_time(ccgi_sockfd,(char*)&cmd_data,sizeof(cmd_data),&overtime);
	if(ret==READ_ERROR)
	{
		close(ccgi_sockfd);
		ccgi_sockfd=-1;
		return -3;
	}
	
	if(cmd_data.cmd_result!=AGENT_RETURN_OK)
	{
		close(ccgi_sockfd);
		ccgi_sockfd=-1;
		return -4;
	}
	
	if(cmd_data.fccp_cmd.ret_val!=FCCP_RETURN_OK)
	{
		close(ccgi_sockfd);
		ccgi_sockfd=-1;
		return -5;
	}
	info->fast_fwd_enable=cmd_data.fccp_cmd.fccp_data.fast_fwd_info.fast_fwd_enable;
	info->fast_fwd_coremask=cmd_data.fccp_cmd.fccp_data.fast_fwd_info.fast_fwd_coremask;
	close(ccgi_sockfd);
	ccgi_sockfd=-1;
	return 0;
}

int ccgi_set_fastfwd_bucket_entry_cmd(char *value,int slot_id)
															//0:success;	-1:Failed:agent is not responding;-2:cmd_data.cmd_result!=AGENT_RETURN_OK;
															//-3:cmd_data.fccp_cmd.ret_val!=FCCP_RETURN_OK;-4:ccgi_se_agent_init failed;
{
	se_interative_t  cmd_data;
	int ret;
	struct timeval overtime;
	memset(&overtime,0,sizeof(overtime));
	memset(&cmd_data,0,sizeof(cmd_data));

	ret=ccgi_se_agent_init();
	if(ret!=0)
	{
		return -4;
	}

	cmd_data.fccp_cmd.fccp_data.bucket_max_entry=strtoul(value,NULL,10);
	strncpy(cmd_data.hand_cmd,SE_AGENT_SET_BUCKET_ENTRY,strlen(SE_AGENT_SET_BUCKET_ENTRY));
	
	ret=ccgi_sendto_agent(ccgi_sockfd,(char*)&cmd_data,sizeof(cmd_data),slot_id);
	if(ret<=0)
	{
		close(ccgi_sockfd);
		ccgi_sockfd=-1;
		return -1;
	}
	memset(&cmd_data,0,sizeof(cmd_data));
	overtime.tv_sec=DCLI_WAIT_TIME;
	ret=ccgi_read_within_time(ccgi_sockfd,(char*)&cmd_data,sizeof(cmd_data),&overtime);
	if(ret==READ_ERROR)
	{
		close(ccgi_sockfd);
		ccgi_sockfd=-1;
		return -1;
	}
	if(cmd_data.cmd_result!=AGENT_RETURN_OK)
	{
		close(ccgi_sockfd);
		ccgi_sockfd=-1;
		return -2;
	}
	if(cmd_data.fccp_cmd.ret_val!=FCCP_RETURN_OK)
	{
		close(ccgi_sockfd);
		ccgi_sockfd=-1;
		return -3;
	}
	close(ccgi_sockfd);
	ccgi_sockfd=-1;
	return 0;
}

int ccgi_show_fastfwd_bucket_entry_cmd(int slot_id, unsigned int *max_entry)
																	//-1:Failed:communication failure with agent module;-2:Failed:agent is not responding;
																	//-3:cmd_data.cmd_result!=AGENT_RETURN_OK;-4:cmd_data.fccp_cmd.ret_val!=FCCP_RETURN_OK;
																	//-5:ccgi_se_agent_init failed;0:success
{
	se_interative_t  cmd_data;
	int ret;
	struct timeval overtime;
	memset(&overtime,0,sizeof(overtime));
	memset(&cmd_data,0,sizeof(cmd_data));

	ret=ccgi_se_agent_init();
	if(ret!=0)
	{
		return -5;
	}
	strncpy(cmd_data.hand_cmd,SE_AGENT_SHOW_BUCKET_ENTRY,strlen(SE_AGENT_SHOW_BUCKET_ENTRY));
	ret=ccgi_sendto_agent(ccgi_sockfd,(char*)&cmd_data,sizeof(cmd_data),slot_id);
	if(ret<=0)
	{
		close(ccgi_sockfd);
		ccgi_sockfd=-1;
		return -1;
	}
	memset(&cmd_data,0,sizeof(cmd_data));
	overtime.tv_sec=DCLI_WAIT_TIME;
	ret=ccgi_read_within_time(ccgi_sockfd,(char*)&cmd_data,sizeof(cmd_data),&overtime);
	if(ret==READ_ERROR)
	{
		close(ccgi_sockfd);
		ccgi_sockfd=-1;
		return -2;
	}
	if(cmd_data.cmd_result!=AGENT_RETURN_OK)
	{
		close(ccgi_sockfd);
		ccgi_sockfd=-1;
		return -3;
	}
	if(cmd_data.fccp_cmd.ret_val!=FCCP_RETURN_OK)
	{
		close(ccgi_sockfd);
		ccgi_sockfd=-1;
		return -4;
	}
	*max_entry = cmd_data.fccp_cmd.fccp_data.bucket_max_entry;
	close(ccgi_sockfd);
	ccgi_sockfd=-1;
	return 0;
}

int ccgi_config_fast_forward_tag_type_cmd(char *state,int slot_id,int flag)
																				//0:success;-1:ccgi_se_agent_init failed;-2:communication failure with agent module;
																				//-3:agent is not responding;-4:cmd_data.cmd_result!=AGENT_RETURN_OK;
{
	cvmx_pow_tag_type_t type ;
	se_interative_t  cmd_data;
	int ret = 0;
	struct timeval overtime;
	
	ret=ccgi_se_agent_init();
	if(ret!=0)
	{
		return -1;
	}

	if(!(strcmp(state,"ordered")))
		type=CVMX_POW_TAG_TYPE_ORDERED;
	else if(!(strcmp(state,"atomic")))
		type=CVMX_POW_TAG_TYPE_ATOMIC;
	else
		type=CVMX_POW_TAG_TYPE_NULL;
	
	memset(&overtime,0,sizeof(overtime));
	memset(&cmd_data,0,sizeof(cmd_data));
	cmd_data.fccp_cmd.fccp_data.tag_type=type;
	
	 cmd_data.cpu_tag= flag; 	//1:CPU_TAG_SLAVE;0:CPU_TAG_MASTER
	strncpy(cmd_data.hand_cmd,SE_AGENT_SET_FWD_TAG_TYPE, strlen(SE_AGENT_SET_FWD_TAG_TYPE));
	ret=ccgi_sendto_agent(ccgi_sockfd,(char*)&cmd_data,sizeof(cmd_data),slot_id);
	if(ret<=0)
	{
		close(ccgi_sockfd);
		ccgi_sockfd=-1;
		return -2;
	}
	memset(&cmd_data,0,sizeof(cmd_data));
	overtime.tv_sec=DCLI_WAIT_TIME;
	ret=ccgi_read_within_time(ccgi_sockfd,(char*)&cmd_data,sizeof(cmd_data),&overtime);
	if(ret==READ_ERROR)
	{
		close(ccgi_sockfd);
		ccgi_sockfd=-1;
		return -3;
	}
	if(cmd_data.cmd_result!=AGENT_RETURN_OK)
	{
		close(ccgi_sockfd);
		ccgi_sockfd=-1;
		return -4;
	}
	close(ccgi_sockfd);
	ccgi_sockfd=-1;
	return 0;
}	

int ccgi_show_fast_forward_tag_type_cmd(int slot_id,int flag,char *info)
																		//0:success;-1:ccgi_se_agent_init failed;-2:communication failure with agent module;
																		//-3:agent is not responding;-4:cmd_data.cmd_result!=AGENT_RETURN_OK;-5:read tag type error
{
	se_interative_t  cmd_data;
	int ret;
	char *tag_type=NULL;
	struct timeval overtime;
	memset(&overtime,0,sizeof(overtime));
	memset(&cmd_data,0,sizeof(cmd_data));
	
	ret=ccgi_se_agent_init();
	if(ret!=0)
	{
		return -1;
	}
	strncpy(cmd_data.hand_cmd,SE_AGENT_SHOW_FWD_TAG_TYPE, strlen(SE_AGENT_SHOW_FWD_TAG_TYPE));
	cmd_data.cpu_tag= flag;    //1:CPU_TAG_SLAVE;0:CPU_TAG_MASTER
	ret=ccgi_sendto_agent(ccgi_sockfd,(char*)&cmd_data,sizeof(cmd_data),slot_id);
	if(ret<=0)
	{	
		close(ccgi_sockfd);
		ccgi_sockfd=-1;
		return -2;
	}
	memset(&cmd_data,0,sizeof(cmd_data));
	overtime.tv_sec=DCLI_WAIT_TIME;
	ret=ccgi_read_within_time(ccgi_sockfd,(char*)&cmd_data,sizeof(cmd_data),&overtime);
	if(ret==READ_ERROR)
	{
		close(ccgi_sockfd);
		ccgi_sockfd=-1;
		return -3;
	}
	if(cmd_data.cmd_result!=AGENT_RETURN_OK)
	{
		close(ccgi_sockfd);
		ccgi_sockfd=-1;
		return -4;
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
			close(ccgi_sockfd);
			ccgi_sockfd=-1;
			return -5;
	}
	if(NULL != tag_type)
	{
		strncpy(info,tag_type,strlen(tag_type));
	}
	close(ccgi_sockfd);
	ccgi_sockfd=-1;
	return 0;
}	

int ccgi_clear_aging_rule_cmd(int slot_id)
											//0:success;-1:ccgi_se_agent_init failed;-2:communication failure with agent module;
{
	se_interative_t  cmd_data;
	int ret = -1;

	ret=ccgi_se_agent_init();
	if(ret!=0)
	{
		return -1;
	}

	memset(&cmd_data,0,sizeof(cmd_data));
	strncpy(cmd_data.hand_cmd,SE_AGENT_CLEAR_AGING_RULE,strlen(SE_AGENT_CLEAR_AGING_RULE));
	ret=ccgi_sendto_agent(ccgi_sockfd,(char*)&cmd_data,sizeof(cmd_data),slot_id);
	if(ret<=0)
	{
		close(ccgi_sockfd);
		ccgi_sockfd=-1;
		return -2;
	}
	close(ccgi_sockfd);
	ccgi_sockfd=-1;
	return 0;
}	


int ccgi_clear_rule_all_cmd(int slot_id,int flag)
													//0:success;-1:ccgi_se_agent_init failed;-2:communication failure with agent module;-3:agent is not responding;
													//-4:cmd_data.cmd_result!=AGENT_RETURN_OK;-5:cmd_data.fccp_cmd.ret_val!=FCCP_RETURN_Ok
{
	se_interative_t  cmd_data;
	int ret;
	struct timeval overtime;
	memset(&overtime,0,sizeof(overtime));
	memset(&cmd_data,0,sizeof(cmd_data));

	ret=ccgi_se_agent_init();
	if(ret!=0)
	{
		return -1;
	}
	
	strncpy(cmd_data.hand_cmd,SE_AGENT_CLEAR_RULE_ALL,strlen(SE_AGENT_CLEAR_RULE_ALL));
	cmd_data.cpu_tag= flag;    //1:CPU_TAG_SLAVE;0:CPU_TAG_MASTER
	ret=ccgi_sendto_agent(ccgi_sockfd,(char*)&cmd_data,sizeof(cmd_data),slot_id);
	if(ret<=0)
	{
		close(ccgi_sockfd);
		ccgi_sockfd=-1;
		return -2;
	}
	memset(&cmd_data,0,sizeof(cmd_data));
	overtime.tv_sec=15;
	ret=ccgi_read_within_time(ccgi_sockfd,(char*)&cmd_data,sizeof(cmd_data),&overtime);
	if(ret==READ_ERROR)
	{
		close(ccgi_sockfd);
		ccgi_sockfd=-1;
		return -3;
	}
	if(cmd_data.cmd_result!=AGENT_RETURN_OK)
	{
		close(ccgi_sockfd);
		ccgi_sockfd=-1;
		return -4;
	}
	if(cmd_data.fccp_cmd.ret_val!=FCCP_RETURN_OK)
	{
		close(ccgi_sockfd);
		ccgi_sockfd=-1;
		return -5;
	}
	close(ccgi_sockfd);
	ccgi_sockfd=-1;
	return 0;
}	


int ccgi_clear_rule_ip_cmd(char *ip,int slot_id)
												//-1:ip address format error;-2:ccgi_se_agent_init failed;-3:communication failure with agent module
{
	se_interative_t  cmd_data;
	int ret;
	
	memset(&cmd_data,0,sizeof(cmd_data));
	if(COMMON_ERROR==parse_ip_check(ip))
	{
		return -1;
	}
	
	ret=ccgi_se_agent_init();
	if(ret!=0)
	{
		return -2;
	}
	
	cmd_data.fccp_cmd.fccp_data.user_info.user_ip=dcli_ip2ulong(ip);
	strncpy(cmd_data.hand_cmd,SE_AGENT_CLEAR_RULE_IP,strlen(SE_AGENT_CLEAR_RULE_IP));
	ret=ccgi_sendto_agent(ccgi_sockfd,(char*)&cmd_data,sizeof(cmd_data),slot_id);
	if(ret<=0)
	{
		close(ccgi_sockfd);
		ccgi_sockfd=-1;
		return -3;
	}
	close(ccgi_sockfd);
	ccgi_sockfd=-1;
	return 0;
}	

int ccgi_fastfwd_learned_icmp_enable_cmd(char *state,int slot_id)
													//0:success;-1:ccgi_se_agent_init failed;-2:command parameter format error
													//-3:communication failure with agent module;-4:agent is not responding;
													//-5:cmd_data.cmd_result!=AGENT_RETURN_OK;
{
	
	int flag = FUNC_DISABLE;
	se_interative_t  cmd_data;
	int ret = -1;
	struct timeval overtime;
	memset(&overtime,0,sizeof(overtime));
	memset(&cmd_data,0,sizeof(cmd_data));
	
	ret=ccgi_se_agent_init();
	if(ret!=0)
	{
		return -1;
	}
	
	if(0==strncmp(state,"disable",strlen(state)))
	{
		flag=FUNC_DISABLE;
	}
	else if(0==strncmp(state,"enable",strlen(state)))
	{
		flag=FUNC_ENABLE;
	}
	else
	{
		close(ccgi_sockfd);
		ccgi_sockfd=-1;
		return -2;
	}
	cmd_data.fccp_cmd.fccp_data.module_enable=flag;
	strncpy(cmd_data.hand_cmd,SE_AGENT_ICMP_ENABLE,strlen(SE_AGENT_ICMP_ENABLE));
	ret=ccgi_sendto_agent(ccgi_sockfd,(char*)&cmd_data,sizeof(cmd_data),slot_id);
	if(ret<=0)
	{
		close(ccgi_sockfd);
		ccgi_sockfd=-1;
		return -3;
	}
	overtime.tv_sec=DCLI_WAIT_TIME;
	ret=ccgi_read_within_time(ccgi_sockfd,(char*)&cmd_data,sizeof(cmd_data),&overtime);
	if(ret==READ_ERROR)
	{
		close(ccgi_sockfd);
		ccgi_sockfd=-1;
		return -4;
	}
	if(cmd_data.cmd_result!=AGENT_RETURN_OK)
	{
		close(ccgi_sockfd);
		ccgi_sockfd=-1;
		return -5;
	}
	close(ccgi_sockfd);
	ccgi_sockfd=-1;
	return 0;
}

int ccgi_fastfwd_pure_ip_enable_cmd(int slot_id,char *state)
																//0:success;-1:ccgi_se_agent_init failed;-2:command parameter format error
																//-3:communication failure with agent module;-4:agent is not responding;
																//-5:cmd_data.cmd_result!=AGENT_RETURN_OK;
{	
	int flag = FUNC_DISABLE;
	se_interative_t  cmd_data;
	int ret = -1;
	struct timeval overtime;
	memset(&overtime,0,sizeof(overtime));
	memset(&cmd_data,0,sizeof(cmd_data));
	ret=ccgi_se_agent_init();
	if(ret!=0)
	{
		return -1;
	}
	if(0==strncmp(state,"disable",strlen(state)))
	{
		flag=FUNC_DISABLE;
	}
	else if(0==strncmp(state,"enable",strlen(state)))
	{
		flag=FUNC_ENABLE;
	}
	else
	{
		close(ccgi_sockfd);
		ccgi_sockfd=-1;
		return -2;
	}
	cmd_data.fccp_cmd.fccp_data.module_enable=flag;
	strncpy(cmd_data.hand_cmd,SE_AGENT_PURE_IP_ENABLE,strlen(SE_AGENT_PURE_IP_ENABLE));
	ret=ccgi_sendto_agent(ccgi_sockfd,(char*)&cmd_data,sizeof(cmd_data),slot_id);
	if(ret<=0)
	{
		close(ccgi_sockfd);
		ccgi_sockfd=-1;
		return -3;
	}
	overtime.tv_sec=DCLI_WAIT_TIME;
	ret=ccgi_read_within_time(ccgi_sockfd,(char*)&cmd_data,sizeof(cmd_data),&overtime);
	if(ret==READ_ERROR)
	{
		close(ccgi_sockfd);
		ccgi_sockfd=-1;
		return -4;
	}
	if(cmd_data.cmd_result!=AGENT_RETURN_OK)
	{
		close(ccgi_sockfd);
		ccgi_sockfd=-1;
		return -5;
	}
	close(ccgi_sockfd);
	ccgi_sockfd=-1;
	return 0;
}	

int ccgi_show_fwd_pure_ip_enable_cmd(int slot_id, int *state)
																	//0:success;-1:ccgi_se_agent_init failed;-2:communication failure with agent module;-3:agent is not responding;
																	//--4:cmd_data.cmd_result!=AGENT_RETURN_OK;-5:cmd_data.fccp_cmd.ret_val != FCCP_RETURN_OK
{
	se_interative_t  cmd_data;
	int ret = 0;
	struct timeval overtime;
	memset(&overtime, 0, sizeof(overtime));
	memset(&cmd_data, 0, sizeof(cmd_data));
	
	ret=ccgi_se_agent_init();
	if(ret!=0)
	{
		return -1;
	}
	
	strncpy(cmd_data.hand_cmd, SE_AGENT_SHOW_PURE_IP_ENABLE, strlen(SE_AGENT_SHOW_PURE_IP_ENABLE));
	ret = ccgi_sendto_agent(ccgi_sockfd, (char*)&cmd_data, sizeof(cmd_data), slot_id);
	if(ret <= 0)
	{
		close(ccgi_sockfd);
		ccgi_sockfd=-1;
		return -2;
	}
	memset(&cmd_data, 0, sizeof(cmd_data));
	overtime.tv_sec = DCLI_WAIT_TIME;
 	ret = ccgi_read_within_time(ccgi_sockfd, (char*)&cmd_data, sizeof(cmd_data), &overtime);
 	if(ret == READ_ERROR)
	{
		close(ccgi_sockfd);
		ccgi_sockfd=-1;
		return -3;
	}
	if(cmd_data.cmd_result != AGENT_RETURN_OK)
	{
		close(ccgi_sockfd);
		ccgi_sockfd=-1;
		return -4;
	}
	if(cmd_data.fccp_cmd.ret_val != FCCP_RETURN_OK)
	{
		close(ccgi_sockfd);
		ccgi_sockfd=-1;
		return -5;
	}
	*state = cmd_data.fccp_cmd.fccp_data.module_enable;
//	vty_out(vty, "fast_forward pure ip forward is %s\n", (cmd_data.fccp_cmd.fccp_data.module_enable == 1) ? "enable ": "disable");
	close(ccgi_sockfd);
	ccgi_sockfd=-1;
	return 0;
}	

int ccgi_show_aging_rule_cnt_cmd(char *value,int slot_id,unsigned long long *dynamic_a,unsigned long long *static_ag)
																				//0:success;-1:ccgi_se_agent_init failed;-2:command parameter format error;-3:communication failure with agent module;
																				//-4:agent is not responding;-5:cmd_data.cmd_result!=AGENT_RETURN_OK;
{
	se_interative_t  cmd_data;
	int ret;
	struct timeval overtime;
	memset(&overtime,0,sizeof(overtime));
	memset(&cmd_data,0,sizeof(cmd_data));
	
	ret=ccgi_se_agent_init();
	if(ret!=0)
	{
		return -1;
	}

	cmd_data.fccp_cmd.fccp_data.aging_timer=strtoul(value,NULL,10);
	if((cmd_data.fccp_cmd.fccp_data.aging_timer < 0) || (cmd_data.fccp_cmd.fccp_data.aging_timer > 100000))
	{
		close(ccgi_sockfd);
		ccgi_sockfd=-1;
		return -2;
	}		
	
	strncpy(cmd_data.hand_cmd,SE_AGENT_SHOW_AGING_RULE_CNT,strlen(SE_AGENT_SHOW_AGING_RULE_CNT));
	ret=ccgi_sendto_agent(ccgi_sockfd,(char*)&cmd_data,sizeof(cmd_data),slot_id);
	if(ret<=0)
	{
		close(ccgi_sockfd);
		ccgi_sockfd=-1;
		return -3;
	}
	memset(&cmd_data,0,sizeof(cmd_data));
	overtime.tv_sec=DCLI_WAIT_TIME;
	ret=ccgi_read_within_time(ccgi_sockfd,(char*)&cmd_data,sizeof(cmd_data),&overtime);
	if(ret==READ_ERROR)
	{
		close(ccgi_sockfd);
		ccgi_sockfd=-1;
		return -4;
	}
	if(cmd_data.cmd_result!=AGENT_RETURN_OK)
	{
		close(ccgi_sockfd);
		ccgi_sockfd=-1;
		return -5;
	}
	
	*dynamic_a = cmd_data.fccp_cmd.fccp_data.rule_aging_cnt.dynamic_aging_cnt;
	*static_ag = cmd_data.fccp_cmd.fccp_data.rule_aging_cnt.static_aging_cnt;
	close(ccgi_sockfd);
	ccgi_sockfd=-1;
	return 0;
}


int ccgi_show_rule_five_tuple_cmd(char *pro,char *sport,char *dport,int slot_id,se_interative_t  *info)
														//0:success;-1:ccgi_se_agent_init failed;-2:protocol is not tcp or udp;-3:source ip address and port format error;
														//-4:destination ip address and port format error;-5:communication failure with agent module;
														//-6:agent is not responding;-7:cmd_data.cmd_result!=AGENT_RETURN_OK;
{
	se_interative_t  cmd_data;
	int ret = -1;
	unsigned char protocol = INVALID_TYPE;
	struct timeval overtime;
	char *token=NULL;
	char *colon=":";
	char *tail_ptr=NULL;

	ret=ccgi_se_agent_init();
	if(ret!=0)
	{
		return -1;
	}
	memset(&overtime,0,sizeof(overtime));
	memset(&cmd_data,0,sizeof(cmd_data));
	protocol=ccgi_parse_protocol(pro);
	if((TCP_TYPE !=protocol) && (UDP_TYPE != protocol))
	{
		close(ccgi_sockfd);
		ccgi_sockfd=-1;
		return -2;
	}
	if(COMMON_ERROR==ccgi_parse_ipport_check(sport))
	{
		close(ccgi_sockfd);
		ccgi_sockfd=-1;
		return -3;
	}
	if(COMMON_ERROR==ccgi_parse_ipport_check(dport))
	{
		close(ccgi_sockfd);
		ccgi_sockfd=-1;
		return -4;
	}
	
	cmd_data.fccp_cmd.fccp_data.rule_tuple.ip_p=protocol;
	token=strtok_r(sport,colon,&tail_ptr);
	if(token!=NULL)
		cmd_data.fccp_cmd.fccp_data.rule_tuple.ip_src=dcli_ip2ulong(token);
	token=strtok_r(NULL,colon,&tail_ptr);
	if(token!=NULL)
		cmd_data.fccp_cmd.fccp_data.rule_tuple.th_sport=atoi(token);
	token=strtok_r(dport,colon,&tail_ptr);
	if(token!=NULL)
		cmd_data.fccp_cmd.fccp_data.rule_tuple.ip_dst=dcli_ip2ulong(token);
	token=strtok_r(NULL,colon,&tail_ptr);
	if(token!=NULL)
		cmd_data.fccp_cmd.fccp_data.rule_tuple.th_dport=atoi(token);
	strncpy(cmd_data.hand_cmd,SE_AGENT_SHOW_FIVE_TUPLE_ACL,strlen(	SE_AGENT_SHOW_FIVE_TUPLE_ACL));
	ret=ccgi_sendto_agent(ccgi_sockfd,(char*)&cmd_data,sizeof(cmd_data),slot_id);
	if(ret<=0)
	{
		close(ccgi_sockfd);
		ccgi_sockfd=-1;
		return -5;
	}
	memset(&cmd_data,0,sizeof(cmd_data));
	overtime.tv_sec=DCLI_WAIT_TIME;
	ret=ccgi_read_within_time(ccgi_sockfd,(char*)&cmd_data,sizeof(cmd_data),&overtime);
	if(ret==READ_ERROR)
	{
		close(ccgi_sockfd);
		ccgi_sockfd=-1;
		return -6;
	}
	
	if(cmd_data.cmd_result!=AGENT_RETURN_OK)
	{
		close(ccgi_sockfd);
		ccgi_sockfd=-1;
		return -7;
	}
	*info = cmd_data;
	close(ccgi_sockfd);
	ccgi_sockfd=-1;
	return 0;
}	

int ccgi_show_rule_stats_cmd(int slot_id,int flag,rule_stats_t *statistics)
																//0:success;-1:ccgi_se_agent_init failed;-2:communication failure with agent module;-3:agent is not responding;
																//-4:cmd_data.cmd_result!=AGENT_RETURN_OK;
{
	se_interative_t  cmd_data;
	int ret;
	struct timeval overtime;
	memset(&overtime,0,sizeof(overtime));
	memset(&cmd_data,0,sizeof(cmd_data));
	
	ret=ccgi_se_agent_init();
	if(ret!=0)
	{
		return -1;
	}
	
	strncpy(cmd_data.hand_cmd,SE_AGENT_SHOW_ACL_STATS,strlen(SE_AGENT_SHOW_ACL_STATS));
	cmd_data.cpu_tag = flag;
	ret=ccgi_sendto_agent(ccgi_sockfd,(char*)&cmd_data,sizeof(cmd_data),slot_id);
	if(ret<=0)
	{
		close(ccgi_sockfd);
		ccgi_sockfd=-1;
		return -2;
	}
	memset(&cmd_data,0,sizeof(cmd_data));
	overtime.tv_sec=15;
	ret=ccgi_read_within_time(ccgi_sockfd,(char*)&cmd_data,sizeof(cmd_data),&overtime);
	if(ret==READ_ERROR)
	{
		close(ccgi_sockfd);
		ccgi_sockfd=-1;
		return -3;
	}
	
	if(cmd_data.cmd_result!=AGENT_RETURN_OK)
	{
		close(ccgi_sockfd);
		ccgi_sockfd=-1;
		return -4;
	}

	statistics->acl_static_tbl_size = cmd_data.fccp_cmd.fccp_data.rule_sum.acl_static_tbl_size;
	statistics->acl_dynamic_tbl_size = 	cmd_data.fccp_cmd.fccp_data.rule_sum.acl_dynamic_tbl_size;
	statistics->s_tbl_used_rule = cmd_data.fccp_cmd.fccp_data.rule_sum.s_tbl_used_rule;
	statistics->s_tbl_aged_rule = cmd_data.fccp_cmd.fccp_data.rule_sum.s_tbl_aged_rule;
	statistics->s_tbl_learned_rule = cmd_data.fccp_cmd.fccp_data.rule_sum.s_tbl_learned_rule;
	statistics->s_tbl_learning_rule = 	cmd_data.fccp_cmd.fccp_data.rule_sum.s_tbl_learning_rule;	
	statistics->s_tbl_static_rule = cmd_data.fccp_cmd.fccp_data.rule_sum.s_tbl_static_rule;
	statistics->d_tbl_used_rule = cmd_data.fccp_cmd.fccp_data.rule_sum.d_tbl_used_rule;
	statistics->d_tbl_aged_rule = cmd_data.fccp_cmd.fccp_data.rule_sum.d_tbl_aged_rule;
	statistics->d_tbl_learned_rule = cmd_data.fccp_cmd.fccp_data.rule_sum.d_tbl_learned_rule;
	statistics->d_tbl_learning_rule = cmd_data.fccp_cmd.fccp_data.rule_sum.d_tbl_learning_rule;
	statistics->d_tbl_static_rule = cmd_data.fccp_cmd.fccp_data.rule_sum.d_tbl_static_rule;
	statistics->capwap_cache_tbl_size = cmd_data.fccp_cmd.fccp_data.rule_sum.capwap_cache_tbl_size;
	statistics->cw_tbl_used = cmd_data.fccp_cmd.fccp_data.rule_sum.cw_tbl_used;
	statistics->cw_tbl_802_3_num = cmd_data.fccp_cmd.fccp_data.rule_sum.cw_tbl_802_3_num;
	statistics->cw_tbl_802_11_num = cmd_data.fccp_cmd.fccp_data.rule_sum.cw_tbl_802_11_num;
	statistics->s_tbl_uplink_rule = 0;
	statistics->d_tbl_uplink_rule = 0;
	statistics->s_tbl_downlink_rule = 0;
	statistics->d_tbl_downlink_rule = 0;
	close(ccgi_sockfd);
	ccgi_sockfd=-1;

	return 0;
}	

int ccgi_show_tolinux_flow_cmd(int slot_id, struct to_linux_flow_r *data)
																		//0:success;-1:ccgi_se_agent_init failed;-2:communication failure with agent module;-3:agent is not responding;
																		//-4:cmd_data.cmd_result!=AGENT_RETURN_OK;
{
	se_interative_t  cmd_data;
	int ret=0;
	struct timeval overtime;
	memset(&overtime,0,sizeof(overtime));
	memset(&cmd_data,0,sizeof(cmd_data));
	
	ret=ccgi_se_agent_init();
	if(ret!=0)
	{
		return -1;
	}
	
	strncpy(cmd_data.hand_cmd,SE_AGENT_SHOW_TOLINUX_FLOW,strlen(SE_AGENT_SHOW_TOLINUX_FLOW));
	ret=ccgi_sendto_agent(ccgi_sockfd,(char*)&cmd_data,sizeof(cmd_data),slot_id);
	if(ret<=0)
	{
		close(ccgi_sockfd);
		ccgi_sockfd=-1;
		return -2;
	}
	memset(&cmd_data,0,sizeof(cmd_data));
	overtime.tv_sec=DCLI_WAIT_TIME;
	ret=ccgi_read_within_time(ccgi_sockfd,(char*)&cmd_data,sizeof(cmd_data),&overtime);
	if(ret==READ_ERROR)
	{
		close(ccgi_sockfd);
		ccgi_sockfd=-1;
		return -3;
	}
	if(cmd_data.cmd_result!=AGENT_RETURN_OK)
	{
		close(ccgi_sockfd);
		ccgi_sockfd=-1;
		return -4;
	}
	data->to_linux_bps = cmd_data.fccp_cmd.fccp_data.to_linux_flow.to_linux_bps;
	data->to_linux_pps = cmd_data.fccp_cmd.fccp_data.to_linux_flow.to_linux_pps;
	close(ccgi_sockfd);
	ccgi_sockfd=-1;
	return 0;
}


int ccgi_show_user_acl_stats_cmd(char *ip,int slot_id,rule_stats_t *statistics)
																					//0:success;-1:ccgi_se_agent_init failed;-2:command parameter format error;-3:communication failure with agent module;
																					//-4:agent is not responding;-5:cmd_data.cmd_result!=AGENT_RETURN_OK;
{
	se_interative_t  cmd_data;
	int ret;
	struct timeval overtime;
	memset(&overtime,0,sizeof(overtime));
	memset(&cmd_data,0,sizeof(cmd_data));
	
	ret=ccgi_se_agent_init();
	if(ret!=0)
	{
		return -1;
	}

	if(COMMON_ERROR==parse_ip_check(ip))
	{
		close(ccgi_sockfd);
		ccgi_sockfd=-1;
		return -2;
	}
	cmd_data.fccp_cmd.fccp_data.rule_tuple.ip_src=dcli_ip2ulong(ip);
	strncpy(cmd_data.hand_cmd,SE_AGENT_SHOW_USER_ACL,strlen(SE_AGENT_SHOW_USER_ACL));
	ret=ccgi_sendto_agent(ccgi_sockfd,(char*)&cmd_data,sizeof(cmd_data),slot_id);
	if(ret<=0)
	{
		close(ccgi_sockfd);
		ccgi_sockfd=-1;
		return -3;
	}
	memset(&cmd_data,0,sizeof(cmd_data));
	overtime.tv_sec=DCLI_WAIT_TIME;
	ret=ccgi_read_within_time(ccgi_sockfd,(char*)&cmd_data,sizeof(cmd_data),&overtime);
	if(ret==READ_ERROR)
	{
		close(ccgi_sockfd);
		ccgi_sockfd=-1;
		return -4;
	}
	if(cmd_data.cmd_result!=AGENT_RETURN_OK)
	{
		close(ccgi_sockfd);
		ccgi_sockfd=-1;
		return -5;
	}
	statistics->d_tbl_used_rule = cmd_data.fccp_cmd.fccp_data.rule_sum.d_tbl_used_rule;
	statistics->s_tbl_used_rule = cmd_data.fccp_cmd.fccp_data.rule_sum.s_tbl_used_rule;
	statistics->s_tbl_uplink_rule = cmd_data.fccp_cmd.fccp_data.rule_sum.s_tbl_uplink_rule;
	statistics->d_tbl_uplink_rule = cmd_data.fccp_cmd.fccp_data.rule_sum.d_tbl_uplink_rule;
	statistics->s_tbl_downlink_rule = cmd_data.fccp_cmd.fccp_data.rule_sum.s_tbl_downlink_rule;
	statistics->d_tbl_downlink_rule = cmd_data.fccp_cmd.fccp_data.rule_sum.d_tbl_downlink_rule;
	statistics->s_tbl_aged_rule = cmd_data.fccp_cmd.fccp_data.rule_sum.s_tbl_aged_rule;
	statistics->d_tbl_aged_rule = cmd_data.fccp_cmd.fccp_data.rule_sum.d_tbl_aged_rule;
	statistics->s_tbl_learned_rule = cmd_data.fccp_cmd.fccp_data.rule_sum.s_tbl_learned_rule;
	statistics->d_tbl_learned_rule = cmd_data.fccp_cmd.fccp_data.rule_sum.d_tbl_learned_rule;
	statistics->s_tbl_learning_rule = cmd_data.fccp_cmd.fccp_data.rule_sum.s_tbl_learning_rule;
	statistics->d_tbl_learning_rule = cmd_data.fccp_cmd.fccp_data.rule_sum.d_tbl_learning_rule;
	close(ccgi_sockfd);
	ccgi_sockfd=-1;
	return 0;
}	


int ccgi_show_fpa_buff_counter_cmd(int slot_id, int flag,unsigned int *p)
																				//0:success;-1:ccgi_se_agent_init failed;-2:communication failure with agent module;
																				//-3:agent is not responding;-4:cmd_data.cmd_result!=AGENT_RETURN_OK;
{
	se_interative_t  cmd_data;
	int ret = -1,i = 0;
	struct timeval overtime;
	memset(&overtime,0,sizeof(overtime));
	memset(&cmd_data,0,sizeof(cmd_data));
	
	ret=ccgi_se_agent_init();
	if(ret!=0)
	{
		return -1;
	}

	strncpy(cmd_data.hand_cmd,SE_AGENT_SHOW_AVAILIABLE_BUFF_COUNT,strlen(SE_AGENT_SHOW_AVAILIABLE_BUFF_COUNT));
	cmd_data.cpu_tag = flag;   //1:CPU_TAG_SLAVE;0:CPU_TAG_MASTER
	ret=ccgi_sendto_agent(ccgi_sockfd,(char*)&cmd_data,sizeof(cmd_data),slot_id);
	if(ret<=0)
	{
		close(ccgi_sockfd);
		ccgi_sockfd=-1;
		return -2;
	}
	overtime.tv_sec=DCLI_WAIT_TIME;
	overtime.tv_usec=0;
	ret=ccgi_read_within_time(ccgi_sockfd,(char*)&cmd_data,sizeof(cmd_data),&overtime);
	if(ret==READ_ERROR)
	{
		close(ccgi_sockfd);
		ccgi_sockfd=-1;
		return -3;
	}
	if(cmd_data.cmd_result!=AGENT_RETURN_OK)
	{
		close(ccgi_sockfd);
		ccgi_sockfd=-1;
		return -4;
	}
	for(i=0;i<CVMX_FPA_NUM_POOLS;i++)
	{
		*(p+i) = cmd_data.fccp_cmd.fccp_data.pool_buff_count[i];
	}
	close(ccgi_sockfd);
	ccgi_sockfd=-1;
	return 0;
}

int ccgi_show_fast_forward_running_config_cmd(int slot_id,int *state)
																	//0:success;-1:ccgi_se_agent_init failed;-2:communication failure with agent module;
																	//-3:agent is not responding;
{	
	char showStr[SE_AGENT_RUNNING_CFG_MEM]={0};
	int ret = -1;
	se_interative_t  cmd_data;
	struct timeval overtime;
	char *str_tok=NULL;
	memset(&overtime,0,sizeof(overtime));
	memset(&cmd_data,0,sizeof(cmd_data));
	
	ret=ccgi_se_agent_init();
	if(ret!=0)
	{
		return -1;
	}
	cmd_data.cpu_tag = 0; //0:master_cpu;1:slave_cpu
	strncpy(cmd_data.hand_cmd,SE_AGENT_SHOW_RUNNING_CFG,strlen(SE_AGENT_SHOW_RUNNING_CFG));
	ret=ccgi_sendto_agent(ccgi_sockfd,(char*)&cmd_data,sizeof(cmd_data),slot_id);
	if(ret<=0)
	{
		close(ccgi_sockfd);
		ccgi_sockfd=-1;
		return -2;
	}
	overtime.tv_sec=DCLI_WAIT_TIME;
	ret=ccgi_read_within_time(ccgi_sockfd,showStr,SE_AGENT_RUNNING_CFG_MEM,&overtime);
	if(ret==READ_ERROR)
	{
		close(ccgi_sockfd);
		ccgi_sockfd=-1;
		return -3;
	}
	if (showStr)
	{
		str_tok=strstr(showStr,"fast-icmp");
		if (str_tok) {
			*state=1;
		} 
		else {
			*state=0;
		}
	}
	close(ccgi_sockfd);
	ccgi_sockfd=-1;
	return 0;
}

#ifdef __cplusplus
}
#endif
