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
* igmp_snoop_com.c
*
*
* CREATOR:
* 		chenbin@autelan.com
*
* DESCRIPTION:
* 		igmp inter source, assemble socket and timer control.
*
* DATE:
*		6/19/2008
*
* FILE REVISION NUMBER:
*  		$Revision: 1.9 $
*
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif
#include "igmp_snoop_com.h"
#include "igmp_snp_log.h"
#include "igmp_snoop.h"
#include "sysdef/returncode.h"
int creat_servsocket(char *path)
{
	int sock,len,oldmask;
	struct sockaddr_un serv;
	
	if(!path)
		return -1;
	
	unlink(path);
	oldmask =umask(0077);
	if((sock = socket(AF_UNIX,SOCK_DGRAM,0))<0)
	{
		igmp_snp_syslog_err("Creat servsocket failed.\n");
		return -1;
	}
	
	memset(&serv,0,sizeof(struct sockaddr_un));
	serv.sun_family = AF_UNIX;
	strncpy(serv.sun_path,path,strlen(path));
	len = strlen(serv.sun_path) + sizeof(serv.sun_family);
	if(bind(sock,(struct sockaddr*)&serv,len)<0)
	{
		igmp_snp_syslog_err("servsocket Bind failed.\n");
		close(sock);
		return -1;
	}
	umask(oldmask);
	return sock;
}

int send_datasocket(char *path,char *data,int data_len)
{
	int sock,send_len;
	struct sockaddr_un send;
	char buf[1024];
	
	if( !path||!data)
		return -1;
	if(1024<data_len)
		return -1;
		
	if((sock = socket(AF_UNIX,SOCK_DGRAM,0))<0)
	{
		igmp_snp_syslog_err("Creat send_datasocket failed.\n");
		return -1;
	}
	
	memset(&send,0,sizeof(struct sockaddr_un));
	send.sun_family = AF_UNIX;
	strncpy(send.sun_path,path,strlen(path));
	memset(buf,0,sizeof(char)*1024);
	memcpy(buf,data,data_len);
	send_len = sendto(sock,buf,data_len,0,(struct sockaddr*)&send,
					sizeof(struct sockaddr_un));
	if(send_len<0)
	{
		igmp_snp_syslog_err("Send data failed.\n");
		close(sock);
		return -1;
	}
	close(sock);
	return 0;
}
/*******************************************************************************
 * creatclientsock_stream
 *
 * DESCRIPTION:
 *   		create the client sock and return the sock value.
 *
 * INPUTS:
 * 		path - igmp snoop server socket path		
 *
 * OUTPUTS:
 *		sock - return the sock value  		  	
 *
 * RETURNS:
 * 		IGMPSNP_RETURN_CODE_OK - create the sock success
 *		IGMPSNP_RETURN_CODE_GET_SOCKET_E - path error or get socket or bind or connect error
 *
 * COMMENTS:
 *     
 **
 ********************************************************************************/
int creatclientsock_stream(char *path,int *sock)
{
	int len = 0;
	struct sockaddr_un serv,local;
	char *local_path = IGMP_SNOOP_PKT_CLNT_SOCK_PATH;
	
	if(!path)
	{
		igmp_snp_syslog_err("socket file path NULL!\n");
		return IGMPSNP_RETURN_CODE_GET_SOCKET_E;
	}
	if((*sock = socket(AF_UNIX,SOCK_STREAM,0))<0)
	{
		igmp_snp_syslog_err("igmp_snoop_com::Creat socket failed.\n");
		return IGMPSNP_RETURN_CODE_GET_SOCKET_E;
	}
	igmp_snp_syslog_dbg("igmp packet socket client build ok,fd = %d\n",*sock);

	memset(&serv,0,sizeof(struct sockaddr_un));
	serv.sun_family = AF_UNIX;
	strncpy(serv.sun_path,path,strlen(path));
	len = strlen(serv.sun_path)+sizeof(serv.sun_family);

	memset(&local,0,sizeof(struct sockaddr_un));
	local.sun_family = AF_UNIX;
	strncpy(local.sun_path,local_path,strlen(local_path));

	igmp_snp_syslog_dbg("igmp packet socket client set path ok,fd = %d\n",*sock);
	
 	if(bind(*sock,(struct sockaddr*)&local, len) < 0){
		igmp_snp_syslog_err("igmp snooping bind to client socket fail");	
		return IGMPSNP_RETURN_CODE_GET_SOCKET_E;
	}
	igmp_snp_syslog_dbg("client connect path :%s\n",path);
	if(connect(*sock,(struct sockaddr*)&serv,len)<0)
	{
		igmp_snp_syslog_err("igmp_snoop_com::::Connect failed.\n");
		close(*sock);
		return IGMPSNP_RETURN_CODE_GET_SOCKET_E;
	}
	return IGMPSNP_RETURN_CODE_OK;
}

int creatclientsock_stream_unblock(char *path)
{
	int sock,len,flags;
	struct sockaddr_un serv;
	
	if(!path)
		return -1;
	
	if((sock = socket(AF_UNIX,SOCK_STREAM,0))<0)
	{
		igmp_snp_syslog_err("Creatclientsock_stream_unblock socket failed.\n");
		
		return -1;
	}
	
	flags = fcntl(sock,F_GETFL,0);
	fcntl(sock,F_SETFL,flags|O_NONBLOCK);
	
	memset(&serv,0,sizeof(struct sockaddr_un));
	serv.sun_family = AF_UNIX;
	strncpy(serv.sun_path,path,strlen(path));
	len = strlen(serv.sun_path)+sizeof(serv.sun_family);
	
	if(connect(sock,(struct sockaddr*)&serv,len)<0)
	{
		if( errno != EINPROGRESS )
		{
			igmp_snp_syslog_err("Creatclientsock_stream_unblock socket Connect failed.\n");
			
			close(sock);
			return -1;
		}
	}
	return sock;
}

int creatservsock_stream(char *path)
{
	int sock,len;
	struct sockaddr_un serv;
	
	if( !path )
		return -1;
	
	unlink(path);
	
	if((sock = socket(AF_UNIX,SOCK_STREAM,0))<0)
	{
		igmp_snp_syslog_err("Creatservsock_stream socket failed.\n");
		return -1;
	}
	
	memset(&serv,0,sizeof(struct sockaddr_un));
	serv.sun_family = AF_UNIX;
	strncpy(serv.sun_path,path,strlen(path));
	len = sizeof(serv.sun_family)+strlen(serv.sun_path);
	
	if( bind(sock,(struct sockaddr*)&serv,len)<0)
	{
		igmp_snp_syslog_err("Creatservsock_stream Bind failed.\n");
		close(sock);
		return -1;
	}
	
	if( listen(sock,5)<0 )
	{
		igmp_snp_syslog_err("Creatservsock_stream Listen failed.\n");
		close(sock);
		return -1;
	}
	
	return sock;
}

/*读取配置文件的默认函数*/
void *def_func(struct cfg_element *cur,void *value)
{	
	return;
}

/*************************************建立定时器************************/
/*******************************************************************************
 * create_timer
 *
 * DESCRIPTION:
 * 		create a new timer add use pointer new timer point to it  	
 *
 * INPUTS:
 * 		type - the timer type (loop or noloop)
 *		pri - the timer priority in the timer list
 *		interval - the time value
 *		func -when the time is over to run the function
 *		data - the timer's data info
 *		datalen -data length
 *
 * OUTPUTS:
 *    	new_timer - the timer id which will be added to the timer list
 *
 * RETURNS:
 *		IGMPSNP_RETURN_CODE_OK - add success
 *		IGMPSNP_RETURN_CODE_NULL_PTR - the timerlist or new timer is null
 *		IGMPSNP_RETURN_CODE_OUT_RANGE - the type or priority out of the range
 *		IGMPSNP_RETURN_CODE_ALLOC_MEM_NULL - malloc memery for new timer fail
 *
 * COMMENTS:
 *  
 ********************************************************************************/
unsigned int create_timer
(	unsigned int type,
	unsigned int pri,
	unsigned long interval,
	void (*func)(timer_element *),
	void *data,
	unsigned int datalen,
	timer_element **new_timer)
{
	igmp_snp_syslog_dbg("START:create_timer\n");

	if( TIMER_TYPE_MAX < type )/*type = TIMER_TYPE_LOOP*/
	{
		igmp_snp_syslog_err("type error.\n");
		igmp_snp_syslog_dbg("END:create_timer\n");
		return IGMPSNP_RETURN_CODE_OUT_RANGE;
	}
	if( TIMER_PRIORI_HIGH < pri )/*pri = TIMER_PRIORI_NORMAL*/
	{
		igmp_snp_syslog_err("priority error.\n");
		igmp_snp_syslog_dbg("END:create_timer\n");
		return IGMPSNP_RETURN_CODE_OUT_RANGE;
	}
	if( NULL == func)
	{
		igmp_snp_syslog_err("handle function is not existence.\n");
		igmp_snp_syslog_dbg("END:create_timer\n");
		return IGMPSNP_RETURN_CODE_NULL_PTR;
	}
	
	(*new_timer) = (timer_element *)malloc(sizeof(timer_element));
	if( NULL == (*new_timer) )
	{
		igmp_snp_syslog_err("new_timer malloc memory failed.\n");
		igmp_snp_syslog_dbg("END:create_timer\n");
		return IGMPSNP_RETURN_CODE_ALLOC_MEM_NULL;
	}
	memset((*new_timer),0,sizeof(timer_element));
	(*new_timer)->next = NULL;
	(*new_timer)->type = type;
	(*new_timer)->priority = pri;
	(*new_timer)->expires = interval;/*interval = 1*/
	(*new_timer)->current = 0;
	(*new_timer)->data = data;
	(*new_timer)->datalen = datalen;
	(*new_timer)->func = func;

	igmp_snp_syslog_dbg("END:create_timer\n");
	return IGMPSNP_RETURN_CODE_OK;
}

/*********************************增加定时器***************************/
/*******************************************************************************
 * add_timer
 *
 * DESCRIPTION:
 * 		add a new timer to the timer list  	
 *
 * INPUTS:
 * 		head - timelist head pointer
 *		new timer - pointer to created new timer
 *
 * OUTPUTS:
 *    	ptimer_id - the timer id which add to the timer list
 *
 * RETURNS:
 *		IGMPSNP_RETURN_CODE_OK - add success
 *		IGMPSNP_RETURN_CODE_NULL_PTR - the timerlist or new timer is null
 *		IGMPSNP_RETURN_CODE_OUT_RANGE - timer list is full
 * COMMENTS:
 *  
 ********************************************************************************/

int add_timer(timer_list *head, timer_element *new_timer, unsigned long *ptimer_id)
{
	int i,timer_id,flag;
	timer_element *tnext = NULL;
	timer_element *tprev = NULL;
	
	if( !head || !new_timer )
	{
		igmp_snp_syslog_err("add_timer:parameter error.\n");
		return IGMPSNP_RETURN_CODE_NULL_PTR;
	}
	timer_id = rand();
	while(head->lock)
		head->lock = 0;
	
	if( head->cnt >= (TIMER_LIST_MAX - 1) )
	{
		head->lock = 1;
		igmp_snp_syslog_err("add_timer:timer element too many.\n");
		return IGMPSNP_RETURN_CODE_OUT_RANGE;
	}
	flag = 1;
	do{
		tnext = head->first_timer;
		while(tnext)
		{
			if( tnext->id == timer_id )
			{
				timer_id = rand();
				break;
			}
			tnext = tnext->next;
		}
	}while(tnext);	/*timers assigned different timer_id with each other. */
	
	new_timer->id = timer_id;
	if( NULL != ptimer_id )
		*ptimer_id = timer_id;
	if( NULL == head->first_timer )
	{
		head->first_timer = new_timer;
		head->cnt = 1;
	}
	else
	{
		tnext = head->first_timer;
		tprev = head->first_timer;
		
		while(tnext->type > new_timer->type)/*type decrease */
		{
			tprev = tnext;
			tnext = tnext->next;
			if( NULL == tnext )
				break;
		}
		if( NULL == tnext )	/*链表尾*/
		{
			tprev->next = new_timer;
		}
		else
		{
			while( tnext->priority >= new_timer->priority )/*priority decrease*/
			{
				tprev = tnext;
				tnext = tnext->next;
				if( NULL == tnext )
					break;
			}
			tprev->next = new_timer;
			new_timer->next = tnext;
		}
		head->cnt++;
	}
	head->lock = 1;
	return IGMPSNP_RETURN_CODE_OK;
}

/*********************************删除定时器***************************/
/*******************************************************************************
 * del_timer
 *
 * DESCRIPTION:
 * 		delete the timer in the timerlist based the timer_id  	
 *
 * INPUTS:
 * 		head - timelist head pointer
 *		timer_id - the timer id which will be delete
 *
 * OUTPUTS:
 *    	
 *
 * RETURNS:
 *		IGMPSNP_RETURN_CODE_OK - delete success
 *		-1 - the timerlist or timer id is null or can find timer based the timer id
 *
 * COMMENTS:
 *  
 ********************************************************************************/

int del_timer(timer_list *head, unsigned int timer_id)
{
    int ret = -1;
	timer_element *tnext = NULL;
	timer_element *tprev = NULL;
	
	if( !head || !timer_id )
	{
		igmp_snp_syslog_err("del_timer:parameter failed.\n");
		return ret;
	}
	
	while(head->lock)
		head->lock = 0;
		
	tnext = head->first_timer;
	tprev = head->first_timer;
	while(tnext)
	{
		if( tnext->id == timer_id )
		{
			if( tprev != tnext )
			{
				tprev->next = tnext->next;
			}
			else
			{
				head->first_timer = tnext->next;
			}
			head->lock = 1;
			free(tnext);
			head->cnt--;
			if( 0 == head->cnt )
				head->first_timer = NULL;
			return IGMPSNP_RETURN_CODE_OK;
		}
		tprev = tnext;
		tnext = tnext->next;
	}
	igmp_snp_syslog_err("del_timer:can not find timer.\n");
	return ret;
}

/**********************************删除所有定时器***************************/
/*******************************************************************************
 * del_all_timer
 *
 * DESCRIPTION:
 * 		delete all the timer in the timerlist  	
 *
 * INPUTS:
 * 		head - timelist head pointer
 *
 * OUTPUTS:
 *    	
 *
 * RETURNS:
 *		IGMPSNP_RETURN_CODE_OK - delete success
 *		IGMPSNP_RETURN_CODE_NULL_PTR - the pointer is null
 *
 * COMMENTS:
 *  
 ********************************************************************************/

int del_all_timer(timer_list *head)
{
	int id,cnt;
	
	if( !head )
	{
		igmp_snp_syslog_err("add_timer:parameter error.\n");
		return IGMPSNP_RETURN_CODE_NULL_PTR;
	}
	
	cnt = head->cnt;
	while(cnt)
	{
		if( NULL != head->first_timer )
		{
			id = head->first_timer->id;
			del_timer(head,id);
		}
		cnt--;
	}
	return IGMPSNP_RETURN_CODE_OK;
}

/**********************************************************************
 * get_num_from_file
 *
 *  DESCRIPTION:
 *          Get number from proc file.
 *
 *  INPUT:
 *          filename    - the path of proc file
 *
 *  OUTPUT:
 *          None
 *
 *  RETURN:
 *          number
 *          DATA_INVALID
 *
 *  COMMENTS:
 *          None
 *
 **********************************************************************/
int igmp_get_num_from_file(const char *filename)
{
    FILE *fp = NULL;
    int data = -1;
    
    fp = fopen(filename, "r");
    if (fp)
    {
        fscanf(fp, "%d", &data);
		fclose(fp);
    }
	else
	{
        igmp_snp_syslog_err("%s open error!\n", filename);
	}
    igmp_snp_syslog_dbg("read %s return:%d \n",filename,data);
    return data;
}

#ifdef __cplusplus
}
#endif

