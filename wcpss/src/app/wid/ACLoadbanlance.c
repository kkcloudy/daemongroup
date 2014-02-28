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
* ACLoadbanlance.c
*
*
* CREATOR:
* autelan.software.wireless-control. team
*
* DESCRIPTION:
* wid module
*
*
*******************************************************************************/

#include "ACLoadbanlance.h"


int get_ipv4addr_by_ifname(unsigned char ID)
{
	int ret = 0;
	struct ifi_info *ifi = (struct ifi_info*)calloc(1, sizeof(struct ifi_info));
	if (NULL == ifi)
	{
		return -1;
	}
	memset(ifi->ifi_name,0,sizeof(ifi->ifi_name));
	strncpy((char*)ifi->ifi_name,(char*)AC_IP_GROUP[ID]->ifname,sizeof(ifi->ifi_name));
	ret = Get_Interface_Info((char*)AC_IP_GROUP[ID]->ifname,ifi);
	if(ifi->addr_num == 0){	//xiaodawei add for interface no ip address, 20110324
		if(ifi->ifi_addr != NULL){
			WID_FREE(ifi->ifi_addr);
			ifi->ifi_addr = NULL;
		}		
		if(ifi->ifi_brdaddr != NULL){
			WID_FREE(ifi->ifi_brdaddr);
			ifi->ifi_brdaddr = NULL;
		}
		WID_FREE(ifi);
		ifi = NULL;
		return INTERFACE_HAVE_NO_IP_ADDR;		

	}
	if((ret != 0)/*||(ifi->addr_num == 0)*/){
		if(ifi->ifi_addr != NULL){
			WID_FREE(ifi->ifi_addr);
			ifi->ifi_addr = NULL;
		}		
		if(ifi->ifi_brdaddr != NULL){
			WID_FREE(ifi->ifi_brdaddr);
			ifi->ifi_brdaddr = NULL;
		}
		WID_FREE(ifi);
		ifi = NULL;
		return WID_DBUS_ERROR;		
	}
	AC_IP_GROUP[ID]->ipaddr = (unsigned char *)WID_MALLOC(ETH_IF_NAME_LEN+1);
	if(AC_IP_GROUP[ID]->ipaddr == NULL)
	{
		wid_syslog_crit("%s malloc %s",__func__,strerror(errno));
		perror("malloc");
		WID_FREE(ifi);
		ifi = NULL;
		return MALLOC_ERROR;
	}
	memset(AC_IP_GROUP[ID]->ipaddr,0,ETH_IF_NAME_LEN+1);
	sprintf((char*)AC_IP_GROUP[ID]->ipaddr,"%s",inet_ntoa(((struct sockaddr_in*)(ifi->ifi_addr))->sin_addr));
	printf("get_ipv4addr_by_ifname = %s \n",AC_IP_GROUP[ID]->ipaddr);

	if(ifi->ifi_addr != NULL){
		WID_FREE(ifi->ifi_addr);
		ifi->ifi_addr = NULL;
	}		
	if(ifi->ifi_brdaddr != NULL){
		WID_FREE(ifi->ifi_brdaddr);
		ifi->ifi_brdaddr = NULL;
	}
	WID_FREE(ifi);
	ifi = NULL;

	return WID_DBUS_SUCCESS;

}

int init_client_socket()
{
	int sockfd;

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	{
		perror("socket error\n");
		printf("init_client_socket error\n");
		return -1;
	}
	
	printf("init_client_socket = %d\n",sockfd);
	
	return sockfd;
	
}

void SendActiveWTPCount(int inum)
{
	int count = inum;
	int i;
	struct wid_ac_ip *tmp;
	struct sockaddr_in servaddr;
	printf("3333333333333333333333333\n"); 
	
	for(i = 1; i < ACIPLIST_NUM; i++)
	{
		if((AC_IP_GROUP[i] != NULL)&&(AC_IP_GROUP[i]->load_banlance == 1))
		{
			printf("44444444444444444444444444 %d \n",i); 
			tmp = AC_IP_GROUP[i]->ip_list;	
			while(tmp != NULL)
			{
				//send date
				printf("5555555555555555555555555555 %d sock = %d\n",i,AC_IP_GROUP[i]->isock); 
				if((strcmp((char*)AC_IP_GROUP[i]->ipaddr,tmp->ip)==0))
				{
					tmp->wtpcount = gActiveWTPs;
					make_link_sequence_by_wtpcount(i);
					printf("change our active wtp count %d \n",gActiveWTPs); 
				}
				else
				{
					memset(&servaddr,0,sizeof(servaddr));
					servaddr.sin_family = AF_INET;
					inet_pton(AF_INET, tmp->ip, &(servaddr.sin_addr.s_addr));
					servaddr.sin_port = htons(SERVPORT);
					if(sendto(AC_IP_GROUP[i]->isock,&count,4,0,(struct sockaddr*)&servaddr,sizeof(servaddr)) != 4)
					{
						perror("sendto error"); 
						printf("sendto error\n"); 
						continue;
					}	
					printf("sendto SendActiveWTPCount ok\n"); 
				}
				
						
				tmp = tmp->next;
			}
		}
	}	
}


void make_link_sequence_by_wtpcount(unsigned char ID)
{
	struct wid_ac_ip *p;
	struct wid_ac_ip *q;	
	struct wid_ac_ip *tmp = NULL;
	int tmpcount = 0;
	unsigned char tmppri = 0;
	char *tmpip;
	
	if(AC_IP_GROUP[ID] != NULL)
	{

		for(p=AC_IP_GROUP[ID]->ip_list;p->next!=NULL;p=p->next) 
		{
			tmp = p; 

			for(q=p->next;q;q=q->next)
			{
				//if(q->wtpcount< tmp->wtpcount) //
				if(((tmp->wtpcount > tmp->threshold)&&(q->wtpcount < q->threshold))\
					||((tmp->wtpcount > tmp->threshold)&&(q->wtpcount > q->threshold)\
					&&((tmp->wtpcount - tmp->threshold) > (q->wtpcount - q->threshold))))
				tmp=q; 
			}
				
			if(tmp != p) 
			{ 
				tmpcount = tmp->wtpcount;
				tmp->wtpcount = p->wtpcount;
				p->wtpcount = tmpcount;

				tmppri = tmp->priority;
				tmp->priority = p->priority;
				p->priority = tmppri;

				tmpip =  tmp->ip;
				tmp->ip = p->ip;
				p->ip = tmpip;
			} 
		}
	}
}

void make_link_sequence_by_priority(unsigned char ID)
{
	struct wid_ac_ip *p;
	struct wid_ac_ip *q;	
	struct wid_ac_ip *tmp = NULL;
	int tmpcount = 0;
	unsigned char tmppri = 0;
	char *tmpip;
	
	if(AC_IP_GROUP[ID] != NULL)
	{

		for(p=AC_IP_GROUP[ID]->ip_list;p->next!=NULL;p=p->next) 
		{
			tmp = p; 

			for(q=p->next;q;q=q->next)
			{
				if(q->priority > tmp->priority) //
				tmp=q; 
			}
				
			if(tmp != p) 
			{ 
				tmpcount = tmp->wtpcount;
				tmp->wtpcount = p->wtpcount;
				p->wtpcount = tmpcount;

				tmppri = tmp->priority;
				tmp->priority = p->priority;
				p->priority = tmppri;

				tmpip =  tmp->ip;
				tmp->ip = p->ip;
				p->ip = tmpip;
			} 
		}
	}
}

CW_THREAD_RETURN_TYPE CWLoadbanlanceThread(void * arg)
{
	int n = 0;
	int i = 0;
	int count = 0;
	socklen_t len;
	struct wid_ac_ip *tmp;
	int wtpcountchange = 0;

	int sockfd;
	struct sockaddr_in my_addr;
	struct sockaddr_in remote_addr;
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	{
		perror("socket error");
		return NULL;
	}
		
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(SERVPORT);
	my_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(my_addr.sin_zero),8);
	if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1)
	{
		close(sockfd);
		perror("bind serror");
		return NULL;
	}

	while(1)
	{
		n = recvfrom(sockfd, (char*)&count, 4, 0, ( struct sockaddr *)&remote_addr, &len);
		printf("received a connection from %s count is %d\n", inet_ntoa(remote_addr.sin_addr),count);

		
		//
		//prccess 
		CWThreadMutexLock(&ACIPLISTMutex);
		
		for(i = 1; i < ACIPLIST_NUM; i++)
		{
			if((AC_IP_GROUP[i] != NULL)&&(AC_IP_GROUP[i]->load_banlance == 1))
			{
				printf("kkkkkkkkkkkkkkkkkkkkkkkk %d \n",i); 
				tmp = AC_IP_GROUP[i]->ip_list;	
				while(tmp != NULL)
				{
					//change wtp count
					if((strcmp(inet_ntoa(remote_addr.sin_addr),tmp->ip)==0))
					{
						tmp->wtpcount = count;
						wtpcountchange = 1;
						
						printf("change remote active wtp count %d \n",count); 
						break;
					}					
							
					tmp = tmp->next;
				}
			}

			if(wtpcountchange ==1)
			{
				wtpcountchange = 0;
				//change date link sequence
				make_link_sequence_by_wtpcount(i);
				
			}
		}		

		CWThreadMutexUnlock(&ACIPLISTMutex);
		
	}

	return NULL;
}


