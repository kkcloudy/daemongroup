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
* ws_dcli_dhcp.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* function for web
*
* dcli_dhcp6.c
* dcli_dhcp6.h
* Date 2011-05-05
*
***************************************************************************/
#include "ws_dhcpv6.h"
#include "ws_returncode.h"

#define ALIAS_NAME_SIZE 		0x15
#define MAC_ADDRESS_LEN			6
#define MAX_IP_STRING_LEN		16

#define DHCP6_RETURN_SUCCESS	0
#define DHCP6_RETURN_ERROR	1
#define IPV6_MAX_LEN    (sizeof("ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255") + 1)

#define MAX_OPTION52_ADDRESS_LIST 	8

static int
str2_ipv6_addr
( 
	char *str,
	struct iaddr *p
)
{
	int ret;

	ret = inet_pton(AF_INET6, str, p->iabuf);
	if (ret != 1) {
		return 0;
	}
	p->len= 16;

	return ret;
}


char* ipv6_long2string(unsigned long ipAddress, unsigned char *buff)
{
	unsigned long	cnt = 0;
	unsigned char *tmpPtr = buff;
	
	cnt = sprintf((char*)tmpPtr,"%ld.%ld.%ld.%ld\n",(ipAddress>>24) & 0xFF, \
			(ipAddress>>16) & 0xFF,(ipAddress>>8) & 0xFF,ipAddress & 0xFF);
	
	return tmpPtr;
}
void Free_ccgi_show_dhcp6_lease(struct dhcp6_pool_show_st *head)
{
	struct dhcp6_pool_show_st *f1,*f2;
	struct dhcp6_sub *pf1,*pf2;
	f1=head->next;
	if(f1 != NULL)
	{
		f2=f1->next;
		while(f2!=NULL)
		{
			pf1=f1->ipv6_subnet.next;
			if(pf1!=NULL)
			{
				pf2=pf1->next;
				while( pf2 != NULL )
				{
					free(pf1->range_high_ip);
					free(pf1->range_low_ip);
					free(pf1);
					pf1=pf2;
					if(pf2!=NULL)
						pf2=pf2->next;
				}
			}	
			free(f1->poolname);
			free(f1->domain_name);
			free(f1->option52);
			free(f1->dnsip);
			free(f1);
			f1=f2;
			f2=f2->next;
		}
		pf1=f1->ipv6_subnet.next;
		if(pf1!=NULL)
		{
			pf2=pf1->next;
			while( pf2 != NULL )
			{
				free(pf1->range_high_ip);
				free(pf1->range_low_ip);
				free(pf1);
				pf1=pf2;
				if(pf2!=NULL)
					pf2=pf2->next;
			}
		}
		free(f1->poolname);
		free(f1->domain_name);
		free(f1->option52);
		free(f1->dnsip);
		free(f1);
		free(f1);
	}
}

int ccgi_show_dhcp6_lease
(
	unsigned int mode,
	unsigned int index,
	struct dhcp6_pool_show *head,
	unsigned int *num
)/*0:fail;1:succ*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_struct,iter_array,  iter_ipv6_array;;
	unsigned int op_ret = 0,i = 0,j = 0;
	unsigned int count = 0, value = 0;
	char *ipaddr ;
	unsigned char mac[MAC_ADDRESS_LEN] = {0, 0, 0, 0, 0,0};
	int ipv6_num = 0;
	int retu = 0;
	char poolmac[128] = {0};

	struct dhcp6_pool_show *q, *tail;
	struct dhcp6_sub *sq,*stail;
	head->next = NULL;
	tail = head;
	
	query = dbus_message_new_method_call(DHCP6_DBUS_BUSNAME, 
									DHCP6_DBUS_OBJPATH, 
									DHCP6_DBUS_INTERFACE, 
									DHCP6_DBUS_METHOD_SHOW_DHCP_LEASE);
	

	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&value,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return 0;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&op_ret);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&count);
	dbus_message_iter_next(&iter);	
	
	if(0 == op_ret) {
		retu = 1;
		if(count > 0) {		
			q = (struct dhcp6_pool_show *)malloc(sizeof(struct dhcp6_pool_show));
			if( NULL == q )
			{
				return -1;
			}
			memset(q,0,sizeof(struct dhcp6_pool_show)+1);
			q->poolname = (char *)malloc(128);
			memset(q->poolname,0,128);

			q->domain_name = (char *)malloc(128);
			memset(q->domain_name,0,128);

			//q->option52 = (char *)malloc(32);
			//q->dnsip = (char *)malloc(32);
			
			dbus_message_iter_recurse(&iter,&iter_array);
			for(i = 0; i < count; i++) {				
				dbus_message_iter_recurse(&iter_array,&iter_struct);				

				
				for(j = 0; j< MAC_ADDRESS_LEN; j++) 
				{
		 			dbus_message_iter_get_basic(&iter_struct,&mac[j]);					
					dbus_message_iter_next(&iter_struct);
				}
				memset(poolmac,0,sizeof(poolmac));
				snprintf(poolmac,sizeof(poolmac),"%02x:%02x:%02x:%02x:%02x:%02x ",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
				
				dbus_message_iter_get_basic(&iter_struct, &(ipv6_num));			
				dbus_message_iter_next(&iter_struct); 
				*num = ipv6_num;
				
				dbus_message_iter_recurse(&iter_struct,&iter_ipv6_array);				

				stail = &(q->ipv6_subnet);
				for (j = 0; j < ipv6_num; j++){
					
					sq = (struct dhcp6_sub *)malloc(sizeof(struct dhcp6_sub)+1);
					
					DBusMessageIter iter_ipv6_struct;
					dbus_message_iter_recurse(&iter_ipv6_array,&iter_ipv6_struct);
					
					dbus_message_iter_get_basic(&iter_ipv6_struct,&ipaddr);		
					dbus_message_iter_next(&iter_ipv6_struct);
					dbus_message_iter_next(&iter_ipv6_array);
					
					sq->range_high_ip = (char *)malloc(128);
					memset(sq->range_high_ip,0,128);
					
					sq->range_low_ip = (char *)malloc(128);
					memset(sq->range_low_ip,0,128);
					
					strncpy(sq->range_low_ip,ipaddr,128);
					
					//vty_out(vty,"%s ", ipaddr);
					//vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x ",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);					
					//vty_out(vty,"\n");
				}				
				dbus_message_iter_next(&iter_array);
			}
		}

	}
	dbus_message_unref(reply);	
	return retu;
}
unsigned int ccgi_show_ipv6_dhcp_server
(
	struct dhcp6_show *owned_option	
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0, value = 0, enable;
	struct dhcp6_show option;
	struct iaddr ipAddr[3];
	int retu = 0;
	
	query = dbus_message_new_method_call(DHCP6_DBUS_BUSNAME, 
									DHCP6_DBUS_OBJPATH, 
									DHCP6_DBUS_INTERFACE, 
									DHCP6_DBUS_METHOD_SHOW_DHCP_GLOBAL_CONF);	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &value, 
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
				
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return 0;
	}	
	if (dbus_message_get_args ( reply, &err,					
					DBUS_TYPE_UINT32, &enable,
					DBUS_TYPE_STRING, &(option.domainsearch),					
					DBUS_TYPE_UINT32, &(option.dnsnum),					
					DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[0]),
					DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[1]),
					DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[2]),
					DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[3]),
					DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[4]),
					DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[5]),
					DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[6]),
					DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[7]),
					DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[8]),
					DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[9]),
					DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[10]),
					DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[11]),
					DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[12]),
					DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[13]),
					DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[14]),							
					DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[15]),
					DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[0]),
					DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[1]),
					DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[2]),
					DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[3]),
					DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[4]),
					DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[5]),
					DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[6]),
					DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[7]),
					DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[8]),
					DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[9]),
					DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[10]),
					DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[11]),
					DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[12]),
					DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[13]),
					DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[14]),							
					DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[15]),	
					DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[0]),
					DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[1]),
					DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[2]),
					DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[3]),
					DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[4]),
					DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[5]),
					DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[6]),
					DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[7]),
					DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[8]),
					DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[9]),
					DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[10]),
					DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[11]),
					DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[12]),
					DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[13]),
					DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[14]),							
					DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[15]),						
					DBUS_TYPE_UINT32, &(option.defaulttime),					
					DBUS_TYPE_INVALID)) {
		 owned_option->enable = enable;		
		 owned_option->defaulttime = option.defaulttime;		 
		 owned_option->dnsnum = option.dnsnum;
		 
		 //owned_option->domainsearch = option.domainsearch;
		 strncpy(owned_option->domainsearch,option.domainsearch,128);
		 
		 
		 memcpy(owned_option->dnsip[0].iabuf,  ipAddr[0].iabuf, 16);
		 memcpy(owned_option->dnsip[1].iabuf,  ipAddr[1].iabuf, 16);
		 memcpy(owned_option->dnsip[2].iabuf,  ipAddr[2].iabuf, 16);		

		 dbus_message_unref(reply);
		 retu  = 1;
	} 
	else {
		
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
	
		retu = 0;
	}
	return retu;

}

void Free_ccgi_show_ipv6_pool(struct dhcp6_pool_show_st *head)
{
	struct dhcp6_pool_show_st *f1,*f2;
	struct dhcp6_sub *pf1,*pf2;
	int i = 0;
	f1=head->next;
	if(f1 != NULL)
	{
		f2=f1->next;
		while(f2!=NULL)
		{
			pf1=f1->ipv6_subnet.next;
			if(pf1!=NULL)
			{
				pf2=pf1->next;
				while( pf2 != NULL )
				{
					free(pf1->range_high_ip);
					free(pf1->range_low_ip);
					free(pf1);
					pf1=pf2;
					if(pf2!=NULL)
						pf2=pf2->next;
				}
			}	
			free(f1->poolname);
			free(f1->domain_name);
			for (i=0;i<8;i++)
			{
				free(f1->option52[i]);
			}
			for (i=0;i<3;i++)
			{
				free(f1->dnsip[i]);
			}
			free(f1);
			f1=f2;
			f2=f2->next;
		}
		pf1=f1->ipv6_subnet.next;
		if(pf1!=NULL)
		{
			pf2=pf1->next;
			while( pf2 != NULL )
			{
				free(pf1->range_high_ip);
				free(pf1->range_low_ip);
				free(pf1);
				pf1=pf2;
				if(pf2!=NULL)
					pf2=pf2->next;
			}
		}
		free(f1->poolname);
		free(f1->domain_name);
		for (i=0;i<8;i++)
		{
			free(f1->option52[i]);
		}
		for (i=0;i<3;i++)
		{
			free(f1->dnsip[i]);
		}
		free(f1);
	}
}
int ccgi_show_ipv6_pool
(
	unsigned int mode,
	unsigned int index,
	struct dhcp6_pool_show_st *head,
	unsigned int *num
)
{	
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	struct dhcp6_pool_show* pool = NULL;
	struct dhcp6_pool_show_st *q,*tail;
	struct dhcp6_sub *pq,*ptail;
	head->next = NULL;
	tail = head;

	unsigned int     i = 0, j = 0,ret = 0, count=0, pool_count = 0;
	//char *tmpstr = NULL;
	int retu = 0;

	query = dbus_message_new_method_call(DHCP6_DBUS_BUSNAME, 
									DHCP6_DBUS_OBJPATH, 
									DHCP6_DBUS_INTERFACE, 
									DHCP6_DBUS_METHOD_SHOW_IP_POOL_CONF);	
	dbus_error_init(&err);
	dbus_message_append_args(query, 
						     DBUS_TYPE_UINT32, &mode,
						     DBUS_TYPE_UINT32, &index,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"--------------> %s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}		
		return 0;
	 }
	 dbus_message_iter_init(reply,&iter); 	
	 dbus_message_iter_get_basic(&iter,&ret);

	
	 if (!ret) {
		dbus_message_iter_next(&iter);  
		dbus_message_iter_get_basic(&iter, &pool_count); 
		pool = malloc(sizeof(struct dhcp6_pool_show)*pool_count);
		memset(pool, 0, sizeof(struct dhcp6_pool_show)*pool_count);
		dbus_message_iter_next(&iter);
		dbus_message_iter_recurse(&iter,&iter_array);

		for (i = 0; i < pool_count; i++)
		{
			retu = 1;
			q = (struct dhcp6_pool_show_st*)malloc(sizeof(struct dhcp6_pool_show_st));
			if (NULL == q)
			{
				return -1;
			}
			memset(q,0,sizeof(struct dhcp6_pool_show_st));
			q->poolname = (char *)malloc(128);
			memset(q->poolname,0,128);

			q->domain_name = (char *)malloc(128);
			memset(q->domain_name,0,128);

			int k = 0;			

			for (k = 0;k<8;k++)
			{
				q->option52[k] = (char *)malloc(128);
				memset(q->option52[k],0,128);
			}

			for (k = 0;k < 3; k++)
			{
				q->dnsip[k] = (char *)malloc(128);
				memset(q->dnsip[k],0,128);
			}

			DBusMessageIter iter_struct;
			DBusMessageIter iter_sub_array;
			
			dbus_message_iter_recurse(&iter_array,&iter_struct);			  

			dbus_message_iter_get_basic(&iter_struct, &(pool[i].poolname));
			dbus_message_iter_next(&iter_struct);
			strncpy(q->poolname,pool[i].poolname,128);

			dbus_message_iter_get_basic(&iter_struct, &(pool[i].domain_name));
			dbus_message_iter_next(&iter_struct); 
			strncpy(q->domain_name,pool[i].domain_name,128);
			
			dbus_message_iter_get_basic(&iter_struct, &(pool[i].option52[0]));
			dbus_message_iter_next(&iter_struct);
			strncpy(q->option52[0],pool[i].option52[0],128);			

			dbus_message_iter_get_basic(&iter_struct, &(pool[i].option52[1]));
			dbus_message_iter_next(&iter_struct);
			strncpy(q->option52[1],pool[i].option52[1],128);

			dbus_message_iter_get_basic(&iter_struct, &(pool[i].option52[2]));
			dbus_message_iter_next(&iter_struct);
			strncpy(q->option52[2],pool[i].option52[2],128);

			dbus_message_iter_get_basic(&iter_struct, &(pool[i].option52[3]));
			dbus_message_iter_next(&iter_struct);
			strncpy(q->option52[3],pool[i].option52[3],128);

			dbus_message_iter_get_basic(&iter_struct, &(pool[i].option52[4]));
			dbus_message_iter_next(&iter_struct);
			strncpy(q->option52[4],pool[i].option52[4],128);

			dbus_message_iter_get_basic(&iter_struct, &(pool[i].option52[5]));
			dbus_message_iter_next(&iter_struct);
			strncpy(q->option52[5],pool[i].option52[5],128);

			dbus_message_iter_get_basic(&iter_struct, &(pool[i].option52[6]));
			dbus_message_iter_next(&iter_struct);
			strncpy(q->option52[6],pool[i].option52[6],128);

			dbus_message_iter_get_basic(&iter_struct, &(pool[i].option52[7]));
			dbus_message_iter_next(&iter_struct);
			strncpy(q->option52[7],pool[i].option52[7],128);

			dbus_message_iter_get_basic(&iter_struct, &(pool[i].option_adr_num));
			dbus_message_iter_next(&iter_struct);
			q->option_adr_num = pool[i].option_adr_num;
			
			dbus_message_iter_get_basic(&iter_struct, &(pool[i].dnsip[0]));
			dbus_message_iter_next(&iter_struct);
			strncpy(q->dnsip[0],pool[i].dnsip[0],128);
			
			dbus_message_iter_get_basic(&iter_struct, &(pool[i].dnsip[1]));
			dbus_message_iter_next(&iter_struct);
			strncpy(q->dnsip[1],pool[i].dnsip[1],128);
			
			dbus_message_iter_get_basic(&iter_struct, &(pool[i].dnsip[2]));
			dbus_message_iter_next(&iter_struct);
			strncpy(q->dnsip[2],pool[i].dnsip[2],128);
			
			dbus_message_iter_get_basic(&iter_struct, &(pool[i].dnsnum));
			dbus_message_iter_next(&iter_struct); 
			q->dnsnum = pool[i].dnsnum;
			
			dbus_message_iter_get_basic(&iter_struct, &(pool[i].defaulttime));
			dbus_message_iter_next(&iter_struct);
			q->defaulttime = pool[i].defaulttime;

			dbus_message_iter_get_basic(&iter_struct, &(pool[i].sub_count));

			count = pool[i].sub_count;
			dbus_message_iter_next(&iter_struct); 		  
			dbus_message_iter_recurse(&iter_struct,&iter_sub_array);	

			pool[i].ipv6_subnet = malloc(sizeof(struct dhcp6_sub)*count);
			memset(pool[i].ipv6_subnet, 0, sizeof(struct dhcp6_sub)*count);

			ptail = &(q->ipv6_subnet);
			for (j = 0; j < count; j++)
			{

				pq = (struct dhcp6_sub *)malloc(sizeof(struct dhcp6_sub));
				memset(pq,0,sizeof(struct dhcp6_sub));
				
				pq->range_high_ip = (char *)malloc(128);
				memset(pq->range_high_ip,0,128);
				
				pq->range_low_ip= (char *)malloc(128);
				memset(pq->range_low_ip,0,128);
				
				DBusMessageIter iter_sub_struct;
				dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
				
				dbus_message_iter_get_basic(&iter_sub_struct,&(pool[i].ipv6_subnet[j].range_low_ip));		
				dbus_message_iter_next(&iter_sub_struct);
				strncpy(pq->range_low_ip,pool[i].ipv6_subnet[j].range_low_ip,128);
				
				dbus_message_iter_get_basic(&iter_sub_struct,&(pool[i].ipv6_subnet[j].range_high_ip));			
				dbus_message_iter_next(&iter_sub_struct);
				strncpy(pq->range_high_ip,pool[i].ipv6_subnet[j].range_high_ip,128);
				
				dbus_message_iter_get_basic(&iter_sub_struct,&(pool[i].ipv6_subnet[j].prefix_length));
				dbus_message_iter_next(&iter_sub_struct);
				pq->prefix_length = pool[i].ipv6_subnet[j].prefix_length;

				dbus_message_iter_next(&iter_sub_array);
				pq->next = NULL;
				ptail->next = pq;
				ptail = pq;
			}			  	  
			dbus_message_iter_next(&iter_array);
			q->next = NULL;
			tail->next = q;
			tail = q;
		}/*for*/
	}  /*if*/
	
	*num = pool_count;
	//*poolshow = pool;
	dbus_message_unref(reply);
	 return retu;		 
}
int ccgi_create_ipv6_pool_name
(
	unsigned int del,
	char *poolName,
	unsigned int *pindex
)/*1:succ;0:fail;-1:poolname is null;-2:delete ip pool fail;-3:create ip pool fail----0:add,1:delete*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0, index = 0;
	int retu = 0;
	*pindex = 0;

	//if (!poolName) {
	//	return -1;
	//}
	query = dbus_message_new_method_call(DHCP6_DBUS_BUSNAME, 
									DHCP6_DBUS_OBJPATH, 
									DHCP6_DBUS_INTERFACE, 
									DHCP6_DBUS_METHOD_CREATE_POOL_NODE);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,	
							 DBUS_TYPE_UINT32,&del,
							 DBUS_TYPE_STRING,&poolName, 
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
				
	dbus_message_unref(query);
	
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return 0;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_UINT32, &index,
					DBUS_TYPE_INVALID)) {
		*pindex = index;
		if(!op_ret) {
			if (del) {
			}
			else {
				
				retu = 1;
				#if 0 
				if(CONFIG_NODE == vty->node) {
					pindex = index;
					retu = 1;
					//vty->node = POOLV6_NODE;
					//vty->index = (void *)index;
					/*vty_out(vty, "create pool index is %d \n", vty->index);*/
				}
				else {
					//vty_out (vty, "Terminal mode change must under configure mode!\n", VTY_NEWLINE);
					return CMD_WARNING;
				}
				#endif
			}
			dbus_message_unref(reply);
			retu = 1;
		}
		else {
			if (del) {			
				//vty_out (vty, "delete ip pool fail \n");
				retu = -2;
			}
			else {
				//vty_out (vty, "create ip pool fail \n");
				retu = -3;
			}
		}
	} 
	else {
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);	
		retu = 0;
	}
	return retu;
}
int ccgi_config_ipv6_pool_name(char *pool_name,unsigned int *pindex)/*1:succ;0:fail*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0, index = 0;
	*pindex = 0;
	int retu = 0;
	
	query = dbus_message_new_method_call(DHCP6_DBUS_BUSNAME, 
									DHCP6_DBUS_OBJPATH, 
									DHCP6_DBUS_INTERFACE, 
									DHCP6_DBUS_METHOD_ENTRY_POOL_NODE);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_STRING,&pool_name, 
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
				
	dbus_message_unref(query);
	
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return 0;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_UINT32, &index,
					DBUS_TYPE_INVALID)) {
		*pindex = index;	
		if(!op_ret) 
		{					
			dbus_message_unref(reply);	
			retu = 1;
		}
		retu = 1;
	} 
	else 
	{
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);	
		retu = 0;
		}
	return retu;
}
int ccgi_config_dhcp_pool_ipv6_range(char *opt,char *lowip,char *highip,char *predix,unsigned int pindex)
{
	struct iaddr ipAddrl, ipAddrh;
	unsigned int ret = 0, add = 0, index = 0, prefix_length = 0;
	int retu = 0;
	
	memset(&ipAddrh, 0 ,sizeof(struct iaddr));
	memset(&ipAddrl, 0 ,sizeof(struct iaddr));
	//index = (unsigned int *)(vty->index);
	index = pindex;

	/*add 0 means add, add 1 means delete*/
	if(strncmp("add", opt, strlen(opt))==0) {
		add = 0;
	}
	else if (strncmp("delete", opt, strlen(opt))==0) {
		add = 1;
	}
	else {
		//vty_out(vty,"bad command parameter!\n");
		return -1;
	}
	
	ret = str2_ipv6_addr((char*)lowip, &ipAddrl);
	if (!ret) {
		return -3;
	}
	
	ret = str2_ipv6_addr((char*)highip, &ipAddrh);
	if (!ret) {
		return -3;
	}
	/* need do it
	if (ipAddrl > ipAddrh) {
		return CMD_WARNING;
	}*/		
	prefix_length = atoi((char *)predix);

	ret = ccgi_add_dhcp_pool_ipv6_range(add, &ipAddrl, &ipAddrh, prefix_length,index);
	if (ret) {
		//vty_out(vty, "%s ip range fail \n", add ? "delete" : "add");
		retu = -2;
	}
	
	return retu;
}
int ccgi_addordel_ipv6pool(char *opt,char *startip,char *endip,char *prefix,	unsigned int index)
{
	struct iaddr ipAddrl, ipAddrh;
	unsigned int ret = 0, add = 0, prefix_length = 0;
	int retu = 0;

	memset(&ipAddrh, 0 ,sizeof(struct iaddr));
	memset(&ipAddrl, 0 ,sizeof(struct iaddr));

	/*add 0 means add, add 1 means delete*/
	if(strncmp("add", opt, strlen(opt))==0) {
		add = 0;
	}
	else if (strncmp("delete", opt, strlen(opt))==0) {
		add = 1;
	}
	else {
		return -1;
	}
	
	ret = ccgi_str2_ipv6_addr((char*)startip, &ipAddrl);
	if (!ret) {
		return -2;
	}
	
	ret = ccgi_str2_ipv6_addr((char*)endip, &ipAddrh);
	if (!ret) {
		return -3;
	}
	prefix_length = atoi((char *)prefix);

	ret = ccgi_add_dhcp_pool_ipv6_range(add, &ipAddrl, &ipAddrh, prefix_length,index);
	if (1 == ret)
	{
		retu = 1;
	}
	else
	{
		retu = 0;
	}
	return retu;
}
int ccgi_add_dhcp_pool_ipv6_range
(
	unsigned int add,
	struct iaddr *ipaddrl, 
	struct iaddr *ipaddrh,
	unsigned int prefix_length,
	unsigned int index
)
{
	DBusMessage 	*query = NULL, *reply = NULL;
	DBusError 		err;
	//DBusMessageIter	 iter;
	//DBusMessageIter	 iter_struct,iter_array;
	unsigned int	op_ret = 0;
	//ret = 0;
	int retu = 0;

	query = dbus_message_new_method_call(DHCP6_DBUS_BUSNAME, 
									DHCP6_DBUS_OBJPATH, 
									DHCP6_DBUS_INTERFACE, 
									DHCP6_DBUS_METHOD_ADD_IP_POOL_RANGE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &add,
							DBUS_TYPE_BYTE, &(ipaddrl->iabuf[0]),
							DBUS_TYPE_BYTE, &(ipaddrl->iabuf[1]),
							DBUS_TYPE_BYTE, &(ipaddrl->iabuf[2]),
							DBUS_TYPE_BYTE, &(ipaddrl->iabuf[3]),
							DBUS_TYPE_BYTE, &(ipaddrl->iabuf[4]),
							DBUS_TYPE_BYTE, &(ipaddrl->iabuf[5]),
							DBUS_TYPE_BYTE, &(ipaddrl->iabuf[6]),
							DBUS_TYPE_BYTE, &(ipaddrl->iabuf[7]),
							DBUS_TYPE_BYTE, &(ipaddrl->iabuf[8]),
							DBUS_TYPE_BYTE, &(ipaddrl->iabuf[9]),
							DBUS_TYPE_BYTE, &(ipaddrl->iabuf[10]),
							DBUS_TYPE_BYTE, &(ipaddrl->iabuf[11]),
							DBUS_TYPE_BYTE, &(ipaddrl->iabuf[12]),
							DBUS_TYPE_BYTE, &(ipaddrl->iabuf[13]),
							DBUS_TYPE_BYTE, &(ipaddrl->iabuf[14]),							
							DBUS_TYPE_BYTE, &(ipaddrl->iabuf[15]),
							DBUS_TYPE_BYTE, &(ipaddrh->iabuf[0]),
							DBUS_TYPE_BYTE, &(ipaddrh->iabuf[1]),
							DBUS_TYPE_BYTE, &(ipaddrh->iabuf[2]),
							DBUS_TYPE_BYTE, &(ipaddrh->iabuf[3]),
							DBUS_TYPE_BYTE, &(ipaddrh->iabuf[4]),
							DBUS_TYPE_BYTE, &(ipaddrh->iabuf[5]),
							DBUS_TYPE_BYTE, &(ipaddrh->iabuf[6]),
							DBUS_TYPE_BYTE, &(ipaddrh->iabuf[7]),
							DBUS_TYPE_BYTE, &(ipaddrh->iabuf[8]),
							DBUS_TYPE_BYTE, &(ipaddrh->iabuf[9]),
							DBUS_TYPE_BYTE, &(ipaddrh->iabuf[10]),
							DBUS_TYPE_BYTE, &(ipaddrh->iabuf[11]),
							DBUS_TYPE_BYTE, &(ipaddrh->iabuf[12]),
							DBUS_TYPE_BYTE, &(ipaddrh->iabuf[13]),
							DBUS_TYPE_BYTE, &(ipaddrh->iabuf[14]),							
							DBUS_TYPE_BYTE, &(ipaddrh->iabuf[15]),	
							DBUS_TYPE_UINT32, &prefix_length,
							DBUS_TYPE_UINT32, &index, 
							DBUS_TYPE_INVALID);		   
   reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
			   
   dbus_message_unref(query);
   
   if (NULL == reply) {
	   if (dbus_error_is_set(&err)) {
		   dbus_error_free(&err);
	   }
	   return 0;
   }
   
   if (dbus_message_get_args ( reply, &err,
				   DBUS_TYPE_UINT32, &op_ret,
				   DBUS_TYPE_INVALID)) {
	   dbus_message_unref(reply);
	   if(!op_ret) {
		   retu = 1;
	   }
	   else 
	   {
			//return op_ret;
			retu = -1;
	   }
   } 
   else {
	   if (dbus_error_is_set(&err)) {
		   dbus_error_free(&err);
	   }
	   dbus_message_unref(reply);   
	  retu = 0;
   }
   return retu;
}
int  ccgi_set_server_lease_default_ipv6
(
	unsigned int lease_default,	
	unsigned int mode,
	unsigned int index,
	unsigned int del
)/*1:succ;0:fail*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;

	query = dbus_message_new_method_call(DHCP6_DBUS_BUSNAME, 
									DHCP6_DBUS_OBJPATH, 
									DHCP6_DBUS_INTERFACE, 
									DHCP6_DBUS_METHOD_SET_SERVER_LEASE_DEFAULT);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &lease_default, 
							 DBUS_TYPE_UINT32, &mode,
							 DBUS_TYPE_UINT32, &index,
							 DBUS_TYPE_UINT32, &del,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
				
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		//return CMD_SUCCESS;
		return 0 ;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) {
		if(!op_ret) {
	     
			dbus_message_unref(reply);
			//return CMD_SUCCESS;
			return 1;
		}
	} 
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
	
		return 0;
	}
}

int ccgi_set_server_option52_ipv6
(
	char *optstr,	
	unsigned int mode,
	unsigned int index,
	unsigned int del
)/*1:succ;0:fai; -1:ap_via_address is null*/
{
	char **ap_via_address = NULL;
	//unsigned int size = 0,  len = 0;
	unsigned int  op_ret = 0;
	int i = 0;
	int j = 0;
	unsigned int ipv6num = 0;
	if (NULL == optstr)
	{
		return -1;
	}
	char *optionstr[8];
	for (i = 0;i < 8; i++)
	{
		optionstr[i] = (char *)malloc(128);
		memset(optionstr[i],0,128);
	}

	char *p = NULL;
	p = strtok(optstr,",");
	i = 0;
	while(p != NULL)
	{
		strncpy(optionstr[i],p,128);
		p = strtok(NULL,",");	
		i++;
	}
	ipv6num = i;
	
	for(i = 0 ; i < ipv6num ; ++i)
	{
		if(ccgi_check_ipv6_address(optionstr[i]))
		{
		 	//vty_out(vty, "%% address %s  is Illegal\n", argv[i]);
			for (i = 0;i < 8; i++)
			{
				free(optionstr[i]);
			}
			return -1;
		}		
	}
	
	ap_via_address = (char **)malloc(sizeof(char *) * MAX_OPTION52_ADDRESS_LIST);
	if(NULL == ap_via_address)
	{
		//vty_out(vty, "Failure to apply memory resources\n");
		for (i = 0;i < 8; i++)
		{
			free(optionstr[i]);
		}
		return -2;
	}
	memset(ap_via_address, 0, sizeof(char *) * MAX_OPTION52_ADDRESS_LIST);

	for(i = 0; i < MAX_OPTION52_ADDRESS_LIST; ++i)
	{
		ap_via_address[i] = malloc(IPV6_MAX_LEN);
		if(NULL == ap_via_address[i])
		{
			break;
		}
		memset(ap_via_address[i], 0 , IPV6_MAX_LEN);
	}

	if(i  <  MAX_OPTION52_ADDRESS_LIST )
	{
		for(j = 0; j < i; ++j)
		{
			if(ap_via_address[j] != NULL)
			{
				free(ap_via_address[j]);
			}
		}

		if(ap_via_address != NULL)
		{
			free(ap_via_address);
		}
		//vty_out(vty, "Failure to apply memory resources\n");
		for (i = 0;i < 8; i++)
		{
			free(optionstr[i]);
		}
		return -3;
	}
	
	for(i = 0; i < ipv6num ; ++i)
	{
		memcpy(ap_via_address[i], optionstr[i],  strlen(optionstr[i]) + 1);
	}

	for (i = 0;i < 8; i++)
	{
		free(optionstr[i]);
	}
	////////////////////////

	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;

	query = dbus_message_new_method_call(DHCP6_DBUS_BUSNAME, 
									DHCP6_DBUS_OBJPATH, 
									DHCP6_DBUS_INTERFACE, 
									DHCP6_DBUS_METHOD_SET_SERVER_OPTION52);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_STRING, &ap_via_address[0],
							 DBUS_TYPE_STRING, &ap_via_address[1],
							 DBUS_TYPE_STRING, &ap_via_address[2],
							 DBUS_TYPE_STRING, &ap_via_address[3],
							 DBUS_TYPE_STRING, &ap_via_address[4],
							 DBUS_TYPE_STRING, &ap_via_address[5],
							 DBUS_TYPE_STRING, &ap_via_address[6],
							 DBUS_TYPE_STRING, &ap_via_address[7],
							 DBUS_TYPE_UINT32, &ipv6num,
							 DBUS_TYPE_UINT32, &mode,							 
							 DBUS_TYPE_UINT32, &index, 							 
							 DBUS_TYPE_UINT32, &del, 
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
				
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		goto free_exit;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) {
		if(!op_ret) {
	
			dbus_message_unref(reply);
			goto free_exit;
		}
	} 
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);

		for(i = 0; i < MAX_OPTION52_ADDRESS_LIST; ++i){
			if(ap_via_address[i] != NULL){
				free(ap_via_address[i]);
			}
		}

		if(ap_via_address != NULL){
			free(ap_via_address);
		}	
		return -1;
	}

free_exit:

	for(i = 0; i < MAX_OPTION52_ADDRESS_LIST; ++i)
	{
			if(ap_via_address[i] != NULL)
			{
				free(ap_via_address[i]);
			}
	}
	if(ap_via_address != NULL)
	{
		free(ap_via_address);
	}
	return 1;	
}

int ccgi_set_server_domain_search_ipv6
(
	char *domainName,	
	unsigned int mode,
	unsigned int index,
	unsigned int del
)/*1:succ;0:fail;-1:domainName is null*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;

	if (!domainName) {
		return -1;
	}
	query = dbus_message_new_method_call(DHCP6_DBUS_BUSNAME, 
									DHCP6_DBUS_OBJPATH, 
									DHCP6_DBUS_INTERFACE, 
									DHCP6_DBUS_METHOD_SET_SERVER_DOMAIN_SEARCH);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_STRING, &domainName, 
							 DBUS_TYPE_UINT32, &mode,							 
							 DBUS_TYPE_UINT32, &index, 							 
							 DBUS_TYPE_UINT32, &del, 
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
				
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return 0;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) {
		if(!op_ret) {
	
			dbus_message_unref(reply);
			return 1;
		}
	} 
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);	
		return 0;
	}
}

int ccgi_set_server_name_servers_ipv6
(
	char *dnsstr,
	unsigned int mode,
	unsigned int index,
	unsigned int del
)/*1:succ;0:fail;-1:dns is null*/
{

	struct iaddr ipAddr[3];
	memset(ipAddr, 0, 3*sizeof(struct iaddr));
	unsigned int ret = 0;
	unsigned int ipNum = 0;
	int i = 0;	
	if (1 != del)
	{
		if (NULL == dnsstr)
		{
			return -1;
		}

		char *p = NULL;
		p = strtok(dnsstr,",");
		i = 0;
		while(p != NULL)
		{
			ccgi_str2_ipv6_addr((char*)p, &ipAddr[i]);		
			p = strtok(NULL,",");	
			i++;
		}
		ipNum = i;
	}
	#if 1 
	/*for(;i < ipnum; i++) 
	{
		ccgi_str2_ipv6_addr((char*)argv[i], &ipAddr[i]);
	}*/

	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;

	query = dbus_message_new_method_call(DHCP6_DBUS_BUSNAME, 
									DHCP6_DBUS_OBJPATH, 
									DHCP6_DBUS_INTERFACE, 
									DHCP6_DBUS_METHOD_SET_SERVER_DNS);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &ipNum,							 
							DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[0]),
							DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[1]),
							DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[2]),
							DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[3]),
							DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[4]),
							DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[5]),
							DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[6]),
							DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[7]),
							DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[8]),
							DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[9]),
							DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[10]),
							DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[11]),
							DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[12]),
							DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[13]),
							DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[14]),	
							DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[15]),
							DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[0]),
							DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[1]),
							DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[2]),
							DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[3]),
							DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[4]),
							DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[5]),
							DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[6]),
							DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[7]),
							DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[8]),
							DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[9]),
							DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[10]),
							DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[11]),
							DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[12]),
							DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[13]),
							DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[14]),							
							DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[15]),	
							DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[0]),
							DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[1]),
							DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[2]),
							DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[3]),
							DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[4]),
							DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[5]),
							DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[6]),
							DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[7]),
							DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[8]),
							DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[9]),
							DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[10]),
							DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[11]),
							DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[12]),
							DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[13]),
							DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[14]),							
							DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[15]),	
							DBUS_TYPE_UINT32, &mode,							 
							DBUS_TYPE_UINT32, &index, 
							DBUS_TYPE_UINT32, &del,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
				
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return 0;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) {
		if(!op_ret) {
	
			dbus_message_unref(reply);
			return 1;
		}
	} 
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);	
		return 0;
	}
	#endif
}
int  ccgi_str2_ipv6_addr
( 
	char *str,
	struct iaddr *p
)
{
	int ret;

	ret = inet_pton(AF_INET6, str, p->iabuf);
	if (ret != 1) {
		return 0;
	}
	p->len= 16;

	return ret;
}

/*0 del 1 add*/
int  ccgi_set_interface_ipv6_pool
(
	char* poolName,
	char* ifname,
	unsigned int add_info
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;
	query = dbus_message_new_method_call(DHCP6_DBUS_BUSNAME, 
									DHCP6_DBUS_OBJPATH, 
									DHCP6_DBUS_INTERFACE, 
									DHCP6_DBUS_METHOD_SET_INTERFACE_POOL);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_STRING, &poolName,							
							DBUS_TYPE_STRING, &ifname,
							DBUS_TYPE_UINT32, &add_info,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
				
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return 0;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) {
		dbus_message_unref(reply);		
		return 1;
	} 
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);	
		return 0;
	}
}
int ccgi_check_ipv6_address(char *ipv6_address)
{
	char addrptr[128] = {0};
	if(NULL == ipv6_address)
	{
		return 1;
	}

	if(inet_pton(AF_INET6, ipv6_address, addrptr) != 1)
	{
		return 1;
	}

	return 0;
}
 int ccgi_set_dhcpv6_server_enable(char *serveropt)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int ret = 0, op_ret = 0, isEnable = 0;

	if(strncmp("enable",serveropt,strlen(serveropt))==0) {
		isEnable = 1;
	}
	else if (strncmp("disable",serveropt,strlen(serveropt))==0) {
		isEnable = 0;
	}
	else {
		//vty_out(vty,"bad command parameter!\n");
		return -1;
	}


	query = dbus_message_new_method_call(DHCP6_DBUS_BUSNAME, 
									DHCP6_DBUS_OBJPATH, 
									DHCP6_DBUS_INTERFACE, 
									DHCP6_DBUS_METHOD_SET_SERVER_ENABLE);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &isEnable, 
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
				
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		//return CMD_SUCCESS;
		return 0;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) {
		if(!op_ret) {
	
			dbus_message_unref(reply);
			//return CMD_SUCCESS;
			return 1;
		}
	} 
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
	
		//return CMD_WARNING;
		return 0;
	}
}

