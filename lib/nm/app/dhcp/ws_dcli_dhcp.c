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
* dcli_dhcp.c V1.8
* dcli_dhcp.h V1.2
* Date 2010-04-21
*
***************************************************************************/
#include "ws_dcli_dhcp.h"
#include "ws_returncode.h"

int save_dhcp_leasez(int slot_id)/*返回1表示成功，返回0表示失败,返回-1表示error*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;
	unsigned int profile = 0;
	int detect = 0;
	int retu = 0;
	void *ccgi_connection = NULL;
	ccgi_ReInitDbusConnection(&ccgi_connection, slot_id, DISTRIBUTFAG);
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_SAVE_DHCP_LEASE);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &profile,	
							 DBUS_TYPE_UINT32, &detect,					 
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block(ccgi_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply)
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		retu = -1;
	}
	if (dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, &op_ret,
		DBUS_TYPE_INVALID)) 
		{
        if(op_ret)
		{
            //vty_out(vty,"saved dhcp lease success\n");
            retu = 1;
		}
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	
	return retu;
}

int show_dhcp_lease(struct dhcp_lease_st *head,int *lnum,int slot_id)/*返回1表示成功，返回0表示失败*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_struct,iter_array;
	unsigned int op_ret = 0,i = 0,j = 0;
	unsigned int count = 0, value = 0;
	unsigned int ipaddr = 0;
	unsigned char mac[MAC_ADDRESS_LEN] = {0, 0, 0, 0, 0,0};
	int retu = 0;
    *lnum=0;
    struct dhcp_lease_st *tail=NULL;
	head->next=NULL;
	tail=head;
	void *ccgi_connection = NULL;
	ccgi_ReInitDbusConnection(&ccgi_connection, slot_id, DISTRIBUTFAG);
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_SHOW_DHCP_LEASE);

	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&value,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
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
		if(count > 0) {
			dbus_message_iter_recurse(&iter,&iter_array);
			for(i = 0; i < count; i++) {
				struct dhcp_lease_st *q=NULL;
				q=(struct dhcp_lease_st *)malloc(sizeof(struct dhcp_lease_st)+1);
				if(NULL == q)
				{
					return -1;
				}
				memset(q,0,sizeof(struct dhcp_lease_st)+1);
				q->leaseip=(char *)malloc(50);
				memset(q->leaseip,0,50);

				q->leasemac=(char *)malloc(32);
				memset(q->leasemac,0,32);
				
				dbus_message_iter_recurse(&iter_array,&iter_struct);			
				dbus_message_iter_get_basic(&iter_struct,&ipaddr);
				dbus_message_iter_next(&iter_struct);
				if(0 == ipaddr){					
					continue;
				}
				/*vty_out(vty,"ipaddr %d\n",ipaddr);*/
				sprintf(q->leaseip,"%d.%d.%d.%d",((ipaddr & 0xff000000) >> 24),((ipaddr & 0xff0000) >> 16),	\
					((ipaddr & 0xff00) >> 8),(ipaddr & 0xff));
				
				/*vty_out(vty,"%-3d.%-3d.%-3d.%-3d ",((ipaddr & 0xff000000) >> 24),((ipaddr & 0xff0000) >> 16),	\
					((ipaddr & 0xff00) >> 8),(ipaddr & 0xff));*/

				for(j = 0; j< MAC_ADDRESS_LEN; j++) {
		 			dbus_message_iter_get_basic(&iter_struct,&mac[j]);
					/*vty_out(vty,"mac[j] %d\n",mac[j]);*/
					dbus_message_iter_next(&iter_struct);
				}
				
				//vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x ",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
                sprintf(q->leasemac,"%02x:%02x:%02x:%02x:%02x:%02x ",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
				dbus_message_iter_next(&iter_array);
		       q->lnum=count;

               q->next = NULL;
			   tail->next=q;
			   tail=q;

			   retu = 1;
			}
		}

	}
	*lnum=count;
	dbus_message_unref(reply);

	return retu;
}
void Free_show_dhcp_lease(struct dhcp_lease_st *head)
{
    struct dhcp_lease_st *f1,*f2;
	f1=head->next;		
	if (NULL == f1)
	{
		return ;
	}
	f2=f1->next;
	while(f2!=NULL)
	{
	  free(f1->leaseip);
	  free(f1->leasemac);
	  free(f1);
	  f1=f2;
	  f2=f2->next;
	}
	free(f1);
}
void Free_show_dhcp_pool(struct dhcp_pool_show_st *head)
{
	struct dhcp_pool_show_st *f1,*f2;
	struct dhcp_sub_show_st *pf1,*pf2;
	f1=head->next;
	if(f1 != NULL)
	{
		f2=f1->next;
		while(f2!=NULL)
		{
			pf1=f1->sub_show.next;		
			if(pf1!=NULL)
			{
				pf2=pf1->next;
				while( pf2 != NULL )
				{
					free(pf1);
					pf1=pf2;
					if(pf2!=NULL)
						pf2=pf2->next;
				}
			}	
			free(f1->domainname);
			free(f1->poolname);
			free(f1->option43);
			free(f1->ifname);
			free(f1);
			f1=f2;
			f2=f2->next;
		}
		pf1=f1->sub_show.next;
		if(pf1!=NULL)
		{
			pf2=pf1->next;
			while( pf2 != NULL )
			{
				free(pf1);
				pf1=pf2;
				if(pf2!=NULL)
					pf2=pf2->next;
			}
		}
		free(f1->domainname);
		free(f1->poolname);
		free(f1->option43);
		free(f1->ifname);
		free(f1);
	}
}


int show_dhcp_lease_by_ip(char *ip,struct dhcp_lease_st *head,int slot_id)/*返回1表示成功，返回0表示失败*/
{

	unsigned int ipAddr = 0, ipMaskLen = 0, ip_Nums = 0;
	ipAddr = ccgi_ip2ulong((char*)ip);
	ipMaskLen = 32;
	ip_Nums = 1<<(32 - ipMaskLen);
	int retu=0;

	retu=ccgi_show_lease_by_ip(ipAddr, ip_Nums, head,slot_id);

	return retu;
}

int ccgi_show_lease_by_ip(struct dhcp_lease_st *head,unsigned int ipaddr, unsigned int ipnums,int slot_id)/*返回1表示成功，返回0表示失败*/
{
	DBusMessage 	*query = NULL, *reply = NULL;
	DBusError 		err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_struct,iter_array;
	unsigned int    ip_Addr = 0, ip_Nums = 0;
	unsigned int	op_ret = 0,  i = 0, j = 0;
	unsigned int 	count = 0,  ip_addr = 0;
	unsigned char 	mac[MAC_ADDRESS_LEN] = {0, 0, 0, 0, 0, 0};
	int retu = 1;

    struct dhcp_lease_st *tail=NULL;
	head->next=NULL;
	tail=head;

	
	ip_Addr = ipaddr;
	ip_Nums = ipnums;
	void *ccgi_connection = NULL;
	ccgi_ReInitDbusConnection(&ccgi_connection, slot_id, DISTRIBUTFAG);
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_SHOW_DHCP_LEASE_BY_IP);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &ip_Addr,
							 DBUS_TYPE_UINT32, &ip_Nums,
							 DBUS_TYPE_INVALID);
		   
	reply = dbus_connection_send_with_reply_and_block (ccgi_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		retu = 0;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&op_ret);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&count);
	dbus_message_iter_next(&iter);	
	
	if(0 == op_ret) {
		if(count > 0) {
			dbus_message_iter_recurse(&iter,&iter_array);
			for(i = 0; i < count; i++) {

				struct dhcp_lease_st *q=NULL;
				q=(struct dhcp_lease_st *)malloc(sizeof(struct dhcp_lease_st)+1);
				if(NULL == q)
				{
					return -1;
				}		
				memset(q,0,sizeof(struct dhcp_lease_st)+1);
				q->leaseip=(char *)malloc(50);
				memset(q->leaseip,0,50);

				q->leasemac=(char *)malloc(32);
				memset(q->leasemac,0,32);

				
				dbus_message_iter_recurse(&iter_array,&iter_struct);	
				dbus_message_iter_get_basic(&iter_struct,&ip_addr);
				dbus_message_iter_next(&iter_struct);
				/*vty_out(vty,"ipaddr %d\n",ipaddr);*/
				/*vty_out(vty,"%-3d.%-3d.%-3d.%-3d ",((ip_addr & 0xff000000) >> 24),((ip_addr & 0xff0000) >> 16),	\
					((ip_addr & 0xff00) >> 8),(ip_addr & 0xff));*/
				sprintf(q->leaseip,"%d.%d.%d.%d",((ip_addr & 0xff000000) >> 24),((ip_addr & 0xff0000) >> 16),	\
					((ip_addr & 0xff00) >> 8),(ip_addr & 0xff));

				for(j = 0; j< MAC_ADDRESS_LEN; j++) {
					dbus_message_iter_get_basic(&iter_struct,&mac[j]);
					/*vty_out(vty,"mac[j] %d\n",mac[j]);*/
					dbus_message_iter_next(&iter_struct);
				}
				
				//vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x ",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
                sprintf(q->leasemac,"%02x:%02x:%02x:%02x:%02x:%02x ",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
				dbus_message_iter_next(&iter_array);
				q->lnum=count;
				q->next=NULL;
				tail->next=q;
				tail=q;
			}
		}

	}
	dbus_message_unref(reply);

	return retu;
}

int show_dhcp_lease_by_ip_diff(char *ipaddr1,char *ipaddrd2,struct dhcp_lease_st *head,int slot_id)/*返回1表示成功，返回0表示失败,返回-1表示error*/
{
	unsigned int ipAddrl = 0, ipAddrb = 0, ip_Nums = 0;
	int retu=0;

	ipAddrl = ccgi_ip2ulong((char*)ipaddr1);
	ipAddrb = ccgi_ip2ulong((char*)ipaddrd2);
	if (ipAddrl > ipAddrb) {
		return -1;
	}	

	ip_Nums = ipAddrb - ipAddrl;

	retu=ccgi_show_lease_by_ip(ipAddrl, ip_Nums, head,slot_id);
	
	return retu;
}

int show_dhcp_lease_by_ip_mask(char *ip,struct dhcp_lease_st *head,int slot_id)/*返回1表示成功，返回0表示失败,返回-1表示error*/
{

	unsigned int  ipMaskLen = 0, ip_Nums = 0;
	unsigned long ipAddr = 0;
	int Val = 0,retu = 0;

	Val = ip_address_format2ulong((char**)&ip, &ipAddr, &ipMaskLen);
	if(COMMON_ERROR == Val) {		
		return -1;
	}
	
	ip_Nums = 1<<(32 - ipMaskLen);
	retu=ccgi_show_lease_by_ip(ipAddr, ip_Nums, head,slot_id);
	
	return retu;
}		

int show_dhcp_lease_by_mac(char *macz,char *leaseip,char *leasemac,int slot_id)
{
	DBusMessage 	*query = NULL, *reply = NULL;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_struct,iter_array;
	DBusError		err;
	unsigned char	macAddr[MAC_ADDRESS_LEN], mac[MAC_ADDRESS_LEN];
	unsigned int	macType = 0,op_ret = 0, ipaddr = 0, j = 0;
	ETHERADDR	fc_mac;
	int retu = 0;

	char lip[50];
	memset(lip,0,50);
	
	char lmac[32];
	memset(lmac,0,32);
	
	memset(&macAddr, 0, MAC_ADDRESS_LEN);
	memset(&mac, 0, MAC_ADDRESS_LEN);
	memset(&fc_mac, 0, sizeof(FC_ETHERADDR));
	
	op_ret = parse_mac_addr((char *)macz, &fc_mac);
	if (DHCP_SERVER_RETURN_CODE_SUCCESS != op_ret) {
    	//vty_out(vty," %% Unknow mac addr format.\n");
		return -2;
	}
	memcpy(macAddr, fc_mac.arEther, MAC_ADDRESS_LEN);
	//vty_out(vty," %% %x%x%x%x%x%x\n", macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);
	void *ccgi_connection = NULL;
	ccgi_ReInitDbusConnection(&ccgi_connection, slot_id, DISTRIBUTFAG);
	/*query*/
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_SHOW_DHCP_LEASE_BY_MAC);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&macType,
							 DBUS_TYPE_BYTE,  &macAddr[0],
							 DBUS_TYPE_BYTE,  &macAddr[1],
							 DBUS_TYPE_BYTE,  &macAddr[2],
							 DBUS_TYPE_BYTE,  &macAddr[3],
							 DBUS_TYPE_BYTE,  &macAddr[4],
							 DBUS_TYPE_BYTE,  &macAddr[5],						
							 DBUS_TYPE_INVALID);
      
	reply = dbus_connection_send_with_reply_and_block (ccgi_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return 0;
	}
	
	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &op_ret);

	dbus_message_iter_next(&iter);	
	
	if(op_ret) {
		retu  = 1;
		dbus_message_iter_recurse(&iter, &iter_array);
		dbus_message_iter_recurse(&iter_array, &iter_struct);
	

		dbus_message_iter_get_basic(&iter_struct, &ipaddr);
		dbus_message_iter_next(&iter_struct);
		/*vty_out(vty,"ipaddr %d\n",ipaddr);*/
		/*vty_out(vty,"%-3d.%-3d.%-3d.%-3d ",((ipaddr & 0xff000000) >> 24),((ipaddr & 0xff0000) >> 16),	\
			((ipaddr & 0xff00) >> 8),(ipaddr & 0xff));*/
        sprintf(lip,"%d.%d.%d.%d",((ipaddr & 0xff000000) >> 24),((ipaddr & 0xff0000) >> 16),	\
			((ipaddr & 0xff00) >> 8),(ipaddr & 0xff));
		strcpy(leaseip,lip);
        
		for(j = 0; j< MAC_ADDRESS_LEN; j++) {
 			dbus_message_iter_get_basic(&iter_struct,&mac[j]);
			/*vty_out(vty,"mac[j] %d\n",mac[j]);*/
			dbus_message_iter_next(&iter_struct);
		}
		
		//vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x ",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
        sprintf(lmac,"%02x:%02x:%02x:%02x:%02x:%02x",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
		strcpy(leasemac,lmac);
		dbus_message_iter_next(&iter_array);
	}
	else 
	{
		//vty_out(vty, "no lease for this mac \n");
		retu = -1;
	}


	dbus_message_unref(reply);
	
	return retu;
}

void Free_ccgi_show_static_host(struct dhcp_static_show_st *head)
{
    struct dhcp_static_show_st *f1,*f2;
	f1=head->next;		
	if (NULL == f1)
	{
		return ;
	}
	f2=f1->next;
	while(f2!=NULL)
	{
	  free(f1->ifname);
	  free(f1);
	  f1=f2;
	  f2=f2->next;
	}
	free(f1);
}
unsigned int ccgi_show_static_host(	struct dhcp_static_show_st *static_host,unsigned int *host_num,int slot_id)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_struct,iter_array;
	unsigned int op_ret = 0,i = 0,j = 0;
	unsigned int ipaddr = 0, count = 0, value = 0;
	unsigned char mac[MAC_ADDRESS_LEN] = {0, 0, 0, 0, 0,0};

	int retu = 1;
	char *ifname = NULL;

    struct dhcp_static_show_st *tail = NULL;
	static_host->next = NULL;
	tail = static_host;	
	void *ccgi_connection = NULL;
    ccgi_ReInitDbusConnection(&ccgi_connection, slot_id, DISTRIBUTFAG);
		
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_SHOW_STATIC_HOST);
	

	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&value,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return 0;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&op_ret);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&count);
	dbus_message_iter_next(&iter);	
	
	if (!op_ret) {
		if(count > 0) {	
			
			dbus_message_iter_recurse(&iter,&iter_array);
			for(i = 0; i < count; i++) {

				struct dhcp_static_show_st *q = NULL;
				q=(struct dhcp_static_show_st *)malloc(sizeof(struct dhcp_static_show_st)+1);
				if(NULL == q)
				{
					return  -1;
				}
				memset(q,0,sizeof(struct dhcp_static_show_st)+1);
				q->ifname = (char *)malloc(20);
				memset(q->ifname,0,20);
				
				dbus_message_iter_recurse(&iter_array,&iter_struct);			
				dbus_message_iter_get_basic(&iter_struct,&ipaddr);
				dbus_message_iter_next(&iter_struct);

				q->ipaddr = ipaddr;
				
				for(j = 0; j< MAC_ADDRESS_LEN; j++) {
		 			dbus_message_iter_get_basic(&iter_struct,&mac[j]);
					q->mac[j] = mac[j];
					dbus_message_iter_next(&iter_struct);
				}
				dbus_message_iter_get_basic(&iter_struct,&ifname);
				dbus_message_iter_next(&iter_struct);

				strcpy(q->ifname,ifname);
				dbus_message_iter_next(&iter_array);

				q->next = NULL;
				tail->next = q;
				tail = q;
				
			}
		}
	}
	
	*host_num = count;
	dbus_message_unref(reply);

	return retu;

}

unsigned int ccgi_show_ip_dhcp_server(struct dhcp_global_show_st *global_show,int slot_id)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0, value = 0, enable = 0, staticarp = 0;
	unsigned int	dns[3] = {0, 0, 0}, routers = 0, wins = 0, default_time = 0, max_time = 0, unicast = 0;
	char* domainname = NULL, *option43 = NULL;
	int retu=0;
	void *ccgi_connection = NULL;
	ccgi_ReInitDbusConnection(&ccgi_connection, slot_id, DISTRIBUTFAG);
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_SHOW_DHCP_GLOBAL_CONF);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &value, 
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (ccgi_connection,query,-1, &err);
				
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		retu = 0;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_UINT32, &enable,
					DBUS_TYPE_UINT32, &unicast,			
					DBUS_TYPE_UINT32, &staticarp,
					DBUS_TYPE_STRING, &domainname,
					DBUS_TYPE_STRING, &option43,
					DBUS_TYPE_UINT32, &dns[0],
					DBUS_TYPE_UINT32, &dns[1],
					DBUS_TYPE_UINT32, &dns[2],
					DBUS_TYPE_UINT32, &routers,
					DBUS_TYPE_UINT32, &wins,
					DBUS_TYPE_UINT32, &default_time,
					DBUS_TYPE_UINT32, &max_time,
					DBUS_TYPE_INVALID)) {
		if(!op_ret) {
			global_show->enable = enable;
			global_show->unicast= unicast;		
			global_show->staticarp= staticarp;	
			strcpy(global_show->domainname , domainname);
			strcpy(global_show->option43 , option43);
			global_show->dns[0] = dns[0];
			global_show->dns[1] = dns[1];
			global_show->dns[2] = dns[2];
			global_show->routers = routers;
			global_show->wins = wins;
			global_show->defaulttime = default_time;
			global_show->maxtime = max_time;
			
			dbus_message_unref(reply);
			retu = 1;
		}
	} 
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
	
		retu = -1;
	}
	
    return retu;
}

unsigned int ccgi_show_ip_pool
(
	unsigned int mode,
	unsigned int index,
	struct dhcp_pool_show_st *head,
	unsigned int *num,
	int slot_id
)
{	
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;	
	unsigned int     i = 0, j = 0,ret = 0, count=0, pool_count = 0;
	int retu  = 0;
    struct dhcp_pool_show *pool = NULL;
	struct dhcp_pool_show_st *q,*tail;
	struct dhcp_sub_show_st *pq,*ptail;

    head->next = NULL;
	tail = head;
	void *ccgi_connection = NULL;
	ccgi_ReInitDbusConnection(&ccgi_connection, slot_id, DISTRIBUTFAG);	
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_SHOW_IP_POOL_CONF);
	dbus_error_init(&err);
	dbus_message_append_args(query, 
						     DBUS_TYPE_UINT32, &mode,
						     DBUS_TYPE_UINT32, &index,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_connection,query,-1, &err);
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return 0;
	 }
	 dbus_message_iter_init(reply,&iter);
 
	 dbus_message_iter_get_basic(&iter,&ret);
 
	 if (!ret) {
	 	retu = 1;
		dbus_message_iter_next(&iter);  
		dbus_message_iter_get_basic(&iter, &pool_count);		
		pool = malloc(sizeof(struct dhcp_pool_show)*pool_count);
		memset(pool, 0, sizeof(struct dhcp_pool_show)*pool_count);		
		dbus_message_iter_next(&iter);  
		dbus_message_iter_recurse(&iter,&iter_array);
		
		for (i = 0; i < pool_count; i++) { 
			////////////////
            q = (struct dhcp_pool_show_st *)malloc(sizeof(struct dhcp_pool_show_st));
			if( NULL == q )
			{
				return -1;
			}
			memset(q,0,sizeof(struct dhcp_pool_show_st)+1);
			q->poolname = (char *)malloc(50);
			memset(q->poolname,0,50);

			q->domainname = (char *)malloc(50);
			memset(q->domainname,0,50);

			q->option43 = (char *)malloc(512);
			memset(q->option43,0,512);

			q->ifname= (char *)malloc(50);
			memset(q->ifname,0,50);
			
			////////////////			
			DBusMessageIter iter_struct;
			DBusMessageIter iter_sub_array;

			dbus_message_iter_recurse(&iter_array,&iter_struct);			  

			dbus_message_iter_get_basic(&iter_struct, &(pool[i].option_show.defaulttime));
			dbus_message_iter_next(&iter_struct); 

			q->defaulttime = pool[i].option_show.defaulttime;			
			
			dbus_message_iter_get_basic(&iter_struct, &(pool[i].option_show.maxtime));
			dbus_message_iter_next(&iter_struct); 

			q->maxtime = pool[i].option_show.maxtime;
			
			dbus_message_iter_get_basic(&iter_struct, &(pool[i].option_show.dns[0]));
			dbus_message_iter_next(&iter_struct); 

			q->dns[0] = pool[i].option_show.dns[0];
			
			dbus_message_iter_get_basic(&iter_struct, &(pool[i].option_show.dns[1]));
			dbus_message_iter_next(&iter_struct); 

			q->dns[1] = pool[i].option_show.dns[1];
			
			dbus_message_iter_get_basic(&iter_struct, &(pool[i].option_show.dns[2]));
			dbus_message_iter_next(&iter_struct); 

			q->dns[2] = pool[i].option_show.dns[2];

			/*show option138*/
			for(j = 0; j < 16 ; j++)
			{
				dbus_message_iter_get_basic(&iter_struct, &(pool[i].option_show.option138[j]));
				dbus_message_iter_next(&iter_struct); 
			}			

			dbus_message_iter_get_basic(&iter_struct, &(pool[i].option_show.routers));
			dbus_message_iter_next(&iter_struct); 

			q->routers = pool[i].option_show.routers;
			
			dbus_message_iter_get_basic(&iter_struct, &(pool[i].option_show.wins));
			dbus_message_iter_next(&iter_struct); 

			q->wins = pool[i].option_show.wins;
			
			dbus_message_iter_get_basic(&iter_struct, &(pool[i].option_show.domainname));
			dbus_message_iter_next(&iter_struct); 
            
			strcpy(q->domainname , pool[i].option_show.domainname);

			dbus_message_iter_get_basic(&iter_struct, &(pool[i].option_show.option43));
			dbus_message_iter_next(&iter_struct); 

			strcpy(q->option43, pool[i].option_show.option43);

			dbus_message_iter_get_basic(&iter_struct, &(q->option60_flag));
			dbus_message_iter_next(&iter_struct);

			for (j = 0; j < DHCP_OPTION60_DEFAULT_NUMBER; j++) {
				dbus_message_iter_get_basic(&iter_struct, &(pool[i].option_show.option60_id[j]));
				dbus_message_iter_next(&iter_struct); 		

                unsigned int option60_id_len = strlen(pool[i].option_show.option60_id[j]) + 1;
                q->option60_id[j] = (char *)malloc(option60_id_len);
                memset(q->option60_id[j], 0, option60_id_len);
                strcpy(q->option60_id[j], pool[i].option_show.option60_id[j]);
			}
			
			
			dbus_message_iter_get_basic(&iter_struct, &(pool[i].poolname));
			dbus_message_iter_next(&iter_struct); 
			
			strcpy(q->poolname,pool[i].poolname);
			
			dbus_message_iter_get_basic(&iter_struct, &(pool[i].interfacename));
			dbus_message_iter_next(&iter_struct); 
			
			strcpy(q->ifname,pool[i].interfacename);
			
			dbus_message_iter_get_basic(&iter_struct, &(pool[i].sub_count));

			q->sub_count = pool[i].sub_count;

			count = pool[i].sub_count;
			dbus_message_iter_next(&iter_struct); 		  
			dbus_message_iter_recurse(&iter_struct,&iter_sub_array);	

			pool[i].sub_show = malloc(sizeof(struct dhcp_sub_show)*count);
			memset(pool[i].sub_show, 0, sizeof(struct dhcp_sub_show)*count);

			ptail = &(q->sub_show);
			for (j = 0; j < count; j++){

				pq = (struct dhcp_sub_show_st *)malloc(sizeof(struct dhcp_sub_show_st)+1);

				DBusMessageIter iter_sub_struct;
				dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);

				dbus_message_iter_get_basic(&iter_sub_struct,&(pool[i].sub_show[j].iplow));
				dbus_message_iter_next(&iter_sub_struct);

                pq->iplow = pool[i].sub_show[j].iplow;				
				
				dbus_message_iter_get_basic(&iter_sub_struct,&(pool[i].sub_show[j].iphigh));
				dbus_message_iter_next(&iter_sub_struct);

				pq->iphigh = pool[i].sub_show[j].iphigh;
				
				dbus_message_iter_get_basic(&iter_sub_struct,&(pool[i].sub_show[j].mask));
				dbus_message_iter_next(&iter_sub_struct);

				pq->mask= pool[i].sub_show[j].mask;

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
	dbus_message_unref(reply);
	return retu;		 
}

unsigned int ccgi_create_ip_pool_name(unsigned int del,char *poolName,unsigned int *pindex,int slot_id)/*返回1表示成功，返回0表示失败，返回-1表示error，返回-2表示create ip pool fail */
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0, index = 0;
	*pindex = 0;
	int retu=0;

	if (!poolName) {
		return -1;
	}
	void *ccgi_connection = NULL;
	ccgi_ReInitDbusConnection(&ccgi_connection, slot_id, DISTRIBUTFAG);
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_CREATE_POOL_NODE);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,	
							 DBUS_TYPE_UINT32,&del,
							 DBUS_TYPE_STRING,&poolName, 
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (ccgi_connection,query,-1, &err);
				
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
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
			retu = 1;			
			dbus_message_unref(reply);			
		}
		else 
		{
			//vty_out (vty, "create ip pool fail \n");
			retu = -2;
		}
	} 
	else 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);

		retu = -1;
		
	}
	return retu;
}
int create_ip_pool_name(char *poolnamez,unsigned int *pindex,int slot_id)/*返回1表示成功，返回0表示失败*/
{
	char* poolName = NULL;
	unsigned int nameSize = 0;
	int ret = 0;
	//unsigned int op_ret = 0;
	int retu=0;
	unsigned int index = 0;
	*pindex = 0;

	/*printf("before parse_vlan_name %s length %d\n",argv[1],strlen(argv[1]));*/
	poolName = (char*)malloc(ALIAS_NAME_SIZE);
	memset(poolName, 0, ALIAS_NAME_SIZE);

	nameSize = strlen(poolnamez);
	memcpy(poolName, poolnamez, nameSize);
	//poolName[nameSize+1] = '\0';
	ret = ccgi_create_ip_pool_name(0, poolName,&index,slot_id);
	*pindex = index;
	if (ret==1) 
	{
		free(poolName);
		poolName = NULL;
		//return CMD_SUCCESS;
		retu = 1;
	}
	else 
	{
		free(poolName);
		poolName = NULL;	
		//return CMD_WARNING;
		retu = 0;
	}
	return retu;
}
int delete_ip_pool_name(char *poolnamez,unsigned int* indexz,int slot_id)/*返回1表示成功，返回0表示失败*/
{
	char* poolName = NULL;
	unsigned int nameSize = 0,index = 0;
	int ret = 0;
	//unsigned int op_ret = 0;
	int retu=0;
	*indexz = 0;

	/*printf("before parse_vlan_name %s length %d\n",argv[1],strlen(argv[1]));*/
	poolName = (char*)malloc(ALIAS_NAME_SIZE);
	memset(poolName, 0, ALIAS_NAME_SIZE);

	nameSize = strlen(poolnamez);
	memcpy(poolName, poolnamez, nameSize);
	//poolName[nameSize+1] = '\0';
	ret = ccgi_create_ip_pool_name(1, poolName,&index,slot_id);
	*indexz = index;
	if (ret==1) {
		free(poolName);
		poolName = NULL;
		//return CMD_SUCCESS;
		retu = 1;
	}
	else {
		free(poolName);
		poolName = NULL;	
		//return CMD_WARNING;
		retu = 0;
	}
	return retu;
}


unsigned int ccgi_config_ip_pool_name(char *poolName,unsigned int *pindex,int slot_id)/*返回1表示成功，返回0表示失败，返回-1表示error*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0, index = 0;
	int retu = 0;
	void *ccgi_connection = NULL;
	ccgi_ReInitDbusConnection(&ccgi_connection, slot_id, DISTRIBUTFAG);
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_ENTRY_POOL_NODE);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_STRING,&poolName, 
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (ccgi_connection,query,-1, &err);
				
	dbus_message_unref(query);
	
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return -1;
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
	} 
	else 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);

	    retu = -1;
		
	}
	return retu;
}
int config_ip_pool_name(char *poolnamez,unsigned int *pindex,int slot_id)/*返回1表示成功，返回0表示失败，返回-1表示config ip pool fail*/
{

	char* poolName = NULL;
	unsigned int nameSize = 0;
	int ret = 0;
	unsigned int  index = 0;
	//unsigned int op_ret = 0, nodeSave = 0;
	int retu = 0;
	*pindex = 0;

	poolName = (char*)malloc(ALIAS_NAME_SIZE);
	memset(poolName, 0, ALIAS_NAME_SIZE);

	nameSize = strlen(poolnamez);
	memcpy(poolName, poolnamez, nameSize);
	ret = ccgi_config_ip_pool_name(poolName,&index,slot_id);
	*pindex = index;
	if (ret==1) 
	{
		free(poolName);		
		poolName = NULL;
		//return CMD_SUCCESS;
		retu = 1;
	}
	else
	{	
		free(poolName);
		poolName = NULL;
		//return CMD_WARNING;
		retu = -1;
	}
	
	return retu;
}

unsigned int ccgi_get_slot_id_by_ifname(const char *ifname)
{
	unsigned int slotnum = 0;
	int i = 0;
	int count = 0;
	char tmp[32];

	memset(tmp, 0, sizeof(tmp));
	memcpy(tmp, ifname, strlen(ifname));

	/* eth : cpu */
	if (0 == strncmp(ifname, "eth", 3)) {
		sscanf(ifname, "eth%d-%*d", &slotnum);
	}

	/* ve */
	else if (0 == strncmp(ifname, "ve", 2)) {
		sscanf(ifname, "ve%d.%*d", &slotnum);
	} 

	/* vlan */
	else if (0 == strncmp(ifname, "vlan", 4)) {
		slotnum = 0xffff;	/* invalid slot number */
	} 

	/* radio */
	else if (0 == strncmp(ifname, "r", 1)) {
		for (i = 0; i < strlen(ifname); i++) {
			/*use '-' to make sure this radio is local board or remote board */
			if (tmp[i] == '-') {
				count++;
			}			
		}
		
		if (2 == count) {	/*local board*/
			slotnum = get_product_info(PRODUCT_LOCAL_SLOTID);
		} else if(3 == count) {	/*remote board*/
			sscanf(ifname, "r%d-%*d-%*d-%d.%*d", &slotnum);
		}
	}

	#if 1
	/* wlan */
	else if(0 == strncmp(ifname, "wlanl", 5)) {
		sscanf(ifname, "wlanl%d-%*d-%*d", &slotnum);
	}
	else if ((0 == strncmp(ifname, "wlan", 4)) && (strncmp(ifname, "wlanl", 5))) {
		sscanf(ifname, "wlan%d-%*d-%*d", &slotnum);
	}
	#else
	else if (0 == strncmp(ifname, "wlan", 4)) {
		for (i = 0; i < strlen(ifname); i++) {
			if(tmp[i] == '-') {
				count++;
			}
		}
		
		if (1 == count) {	/*local board*/
			slotnum = dcli_dhcp_get_board_slot_id();
		} else if (2 == count) {	/*remote board*/
			sscanf(ifname, "wlan%d-%*d-%*d", &slotnum);
		}
	}
	#endif

	/* ebr */
	#if 1
	/* ebrl */
	else if (0 == strncmp(ifname, "ebrl", 4)) {
		sscanf(ifname, "ebrl%d-%*d-%*d", &slotnum);
	}

	/* ebr */
	else if ((0 == strncmp(ifname, "ebr", 3)) && (strncmp(ifname, "ebrl", 4))) {
		sscanf(ifname, "ebr%d-%*d-%*d", &slotnum);
	}
	else if (0 == strncmp(ifname, "mng", 3)) {
		sscanf(ifname, "mng%d-%*d", &slotnum);
	}
	#else
	else if (0 == strncmp(ifname, "ebr", 3)) {
		for (i = 0; i < strlen(ifname); i++) {
			if (tmp[i] == '-') {
				count++;
			}
		}
		if (1 == count) {	/*local board*/
			slotnum = dcli_dhcp_get_board_slot_id();
		} else if (2 == count) {	/*remote board*/
			sscanf(ifname, "ebr%d-%*d-%*d", &slotnum);
		}
	}
	#endif
	return slotnum;
}

int	ccgi_check_slot_wheather_empty(int slot_id)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusMessageIter iter;
	DBusError err;

	int i;
	int board_code;
	int is_master, is_active_master;
	unsigned int function_type;
	char *name;
	int slot_count;
	unsigned int board_on_mask;
	int board_state;
	//int slot_sum = 0 ;
	
	query = dbus_message_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
										 SEM_DBUS_INTERFACE, SEM_DBUS_SHOW_SLOT_INFO);
	if (!query)
	{
		//printf("show slot id query failed\n");
		return -1;
	}

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block(ccgi_dbus_connection, query, -1, &err);
	
	dbus_message_unref(query);

	if (!reply)
	{
		//printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		
		return 0;
	}

	dbus_message_iter_init(reply,&iter);
	
	dbus_message_iter_get_basic(&iter, &slot_count);	
	dbus_message_iter_next(&iter);

	dbus_message_iter_get_basic(&iter, &board_on_mask); 
	dbus_message_iter_next(&iter);
	
	for (i=0; i<slot_count; i++)
	{	
		dbus_message_iter_get_basic(&iter, &board_state);	
		dbus_message_iter_next(&iter);

		dbus_message_iter_get_basic(&iter, &board_code);
		dbus_message_iter_next(&iter);

		dbus_message_iter_get_basic(&iter, &function_type);
		dbus_message_iter_next(&iter);
		
		dbus_message_iter_get_basic(&iter, &is_master);
		dbus_message_iter_next(&iter);

		dbus_message_iter_get_basic(&iter, &is_active_master);
		dbus_message_iter_next(&iter);
		
		dbus_message_iter_get_basic(&iter, &name);
		if (i < slot_count-1)
		{
			dbus_message_iter_next(&iter);
		}
		
		if (board_on_mask & (0x1<<i))
		{
			
			if (board_state <= 1)
			{
				//vty_out(vty, "slot %d:not work normal\n", i+1);
				continue;
			}
			if((i+1) == slot_id){
				return 2;
			}
			/*
			vty_out(vty, "slot %d:\n", i+1);
			vty_out(vty, "\tBOARD_CODE:\t\t0x%x\n", board_code);
			vty_out(vty, "\tFUNCTION_TYPE:\t\t0x%x\n", function_type);
			vty_out(vty, "\tIS_MASTER:\t\t%s\n", is_master ? "YES" : "NO");
			vty_out(vty, "\tIS_ACTIVE_MASTER:\t%s\n", is_active_master ? "YES" : "NO");
			vty_out(vty, "\tBOARD_NAME:\t\t%s\n", name);
			*/
		}
		else
		{
			//vty_out(vty, "slot %d is empty\n", i+1);
		}
	}
	dbus_message_unref(reply);
	return 0;
}

int	ccgi_dhcp_check_ve_interface(char *ifname)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	int ret = 0;
	unsigned int slot_id = 0;
	slot_id = ccgi_get_slot_id_by_ifname(ifname);
	ret = ccgi_check_slot_wheather_empty(slot_id);
	if(slot_id < 1 || slot_id > 17){
		return -1;
	}
	if(2 != ret){
		//printf("slot id is empty\n");
		return -1;
	}
	void *connection = NULL;
	char *name=NULL;
	if(SNMPD_DBUS_SUCCESS != get_slot_dbus_connection(slot_id, &connection, SNMPD_INSTANCE_MASTER_V3))
	{
		return -1;
	}

	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_CHECK_INTERFACE_VE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_STRING, &ifname,					 
							DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block(connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return 0;
	}
	if (dbus_message_get_args(reply, &err,
		DBUS_TYPE_STRING, &name,
		DBUS_TYPE_INVALID)) {
	} 
	else {		
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	memset(ifname, 0, ALIAS_NAME_SIZE);
	memcpy(ifname, name, strlen(name));
	return 0;
}


/*add_info 1:ip pool POOLNAME*/
/*add_info 0:no ip pool POOLNAME*/
unsigned int ccgi_set_interface_ip_pool(char* poolName,char* ifname,unsigned int add_info,unsigned int unbindflag,int slot_id)
															/*表示0表示失败，返回1表示成功*/
															/*返回-1表示pool name is too long*/
															/*返回-2表示pool has already binded to interface*/
															/*返回-3表示pool has no subnet*/
															/*返回-4表示not found pool，返回-5表示error*/
{
	if((NULL == poolName) || (NULL == ifname))
		return 0;
	
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0, ret = 0;
	int retu = 0;

	if (strlen(poolName) >= ALIAS_NAME_SIZE)
	{
		return -1;
	}		

	if(strncmp(ifname, "ve", 2) == 0)
	{
		ccgi_dhcp_check_ve_interface(ifname); 
	}

	ret = ccgi_dhcp_check_relay_interface_iSbusy(ifname,slot_id);
	if(1 == ret)
	{
		return 0;
	}
	void *ccgi_connection = NULL;
	ccgi_ReInitDbusConnection(&ccgi_connection, slot_id, DISTRIBUTFAG);
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_SET_INTERFACE_POOL);	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_STRING, &poolName,							
							DBUS_TYPE_STRING, &ifname,
							DBUS_TYPE_UINT32, &add_info,
							DBUS_TYPE_UINT32, &unbindflag,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (ccgi_connection,query,-1, &err);
				
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		retu = 0;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) {
		dbus_message_unref(reply);
		
		//return op_ret; 
		if( op_ret == 0 )
		{
			retu = 1;
		}
		else
		{
			if(DHCP_HAVE_BIND_INTERFACE == op_ret)
			{
				retu = -2;
			}
			else if(DHCP_POOL_SUBNET_NULL == op_ret)
			{
				retu = -3;
			}
			else if (DHCP_NOT_FOUND_POOL == op_ret)
			{
				retu = -4;
			}
			else
			{
				retu = 0;
			}
			return retu;
		}
	} 
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
	
		retu = -5;
	}
	return retu ;
}
int set_interface_ip_pool(char *poolnamez,int slot_id)/*返回1表示成功，返回0表示失败，返回-1表示interface bind pool fail*/
{

	char* poolName = NULL, *ifname = NULL;;
	unsigned int nameSize = 0;
	int ret = 0;
	int retu = 0;

	char vlan_eth_port_ifname [20] = {0};

	poolName = (char*)malloc(ALIAS_NAME_SIZE);
	ifname = (char*)malloc(ALIAS_NAME_SIZE);
	memset(poolName, 0, ALIAS_NAME_SIZE);
	memset(ifname, 0, ALIAS_NAME_SIZE);
	
	nameSize = strlen(poolnamez);
	memcpy(poolName, poolnamez, nameSize);
	
	nameSize = strlen(vlan_eth_port_ifname);
	memcpy(ifname, vlan_eth_port_ifname, nameSize);

	ret = ccgi_set_interface_ip_pool(poolName, ifname, 1,BIND_POOL_ON_INTERFACE,slot_id);
	
	if (ret==1) {
		free(poolName);
		free(ifname);
		poolName = NULL;
		retu = 1;
	}
	else {	
		//vty_out(vty,"interface bind pool fail\n");		
		free(poolName);
		free(ifname);
		poolName = NULL;
		retu =-1;
	}
	return retu;
}
int del_interface_ip_pool(char *poolnamez,int slot_id)/*返回1表示成功，返回0表示失败，返回-1表示interface unbind pool fail*/
{

	char* poolName = NULL, *ifname = NULL;;
	unsigned int nameSize = 0;
	int ret = 0;
	int retu = 0;

	char vlan_eth_port_ifname [20] = {0};


	poolName = (char*)malloc(ALIAS_NAME_SIZE);
	ifname = (char*)malloc(ALIAS_NAME_SIZE);
	memset(poolName, 0, ALIAS_NAME_SIZE);
	memset(ifname, 0, ALIAS_NAME_SIZE);
	
	nameSize = strlen(poolnamez);
	memcpy(poolName, poolnamez, nameSize);
	
	nameSize = strlen(vlan_eth_port_ifname);
	memcpy(ifname, vlan_eth_port_ifname, nameSize);

	ret = ccgi_set_interface_ip_pool(poolName, ifname, 0,UNBIND_POOL_ON_INTERFACE,slot_id);
	
	if (ret==1) {
		free(poolName);
		free(ifname);
		poolName = NULL;
		retu = 1;
	}
	else {	
		//vty_out(vty,"interface unbind pool fail\n");		
		free(poolName);
		free(ifname);
		poolName = NULL;
		retu = -1;
	}
	return retu;
}


int ccgi_add_dhcp_pool_ip_range
(
	unsigned int add,
	unsigned int ipaddrl, 
	unsigned int ipaddrh,
	unsigned int ipmask,
	unsigned int index,
	int slot_id
)/*返回1表示成功，返回0表示失败，返回-1表示errror*/
{
	DBusMessage 	*query = NULL, *reply = NULL;
	DBusError 		err;
	unsigned int	op_ret = 0;
	int retu = 0;
	void *ccgi_connection = NULL;
	ccgi_ReInitDbusConnection(&ccgi_connection, slot_id, DISTRIBUTFAG);
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_ADD_IP_POOL_RANGE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &add,
							DBUS_TYPE_UINT32, &ipaddrl,	
							DBUS_TYPE_UINT32, &ipaddrh,
							DBUS_TYPE_UINT32, &ipmask,							
							DBUS_TYPE_UINT32, &index, 
							DBUS_TYPE_INVALID);		   
   reply = dbus_connection_send_with_reply_and_block (ccgi_connection,query,-1, &err);
			   
   dbus_message_unref(query);
   
   if (NULL == reply) {
	   if (dbus_error_is_set(&err)) {
		   dbus_error_free(&err);
	   }
	   return -1;
   }
   
   if (dbus_message_get_args ( reply, &err,
				   DBUS_TYPE_UINT32, &op_ret,
				   DBUS_TYPE_INVALID)) {
	   dbus_message_unref(reply);
	   if(!op_ret) {
		   retu = 1;		   
	   }
	   else {
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

int add_dhcp_pool_ip_range(char *addordel,char *startip,char *endip,char *maskz,unsigned int pindex,int slot_id)/*返回1表示成功，返回0表示失败，返回-1表示bad command parameter，返回-2表示结束ip不大于起始ip*/
{
	unsigned int ipAddrl = 0, ipAddrh = 0, ipmask = 0, ip_Nums = 0;
	unsigned int ret = 0, add = 0, index = 0;

	//index = (unsigned int *)(vty->index);

	index = pindex;

	/*add 0 means add, add 1 means delete*/
	if(strncmp("add", addordel, strlen(addordel))==0) {
		add = 0;
	}
	else if (strncmp("delete", addordel, strlen(addordel))==0) {
		add = 1;
	}
	else {
		//vty_out(vty,"bad command parameter!\n");
		return -1;
	}
	
	ipAddrl = ccgi_ip2ulong((char*)startip);
	ipAddrh = ccgi_ip2ulong((char*)endip);
	if (ipAddrl > ipAddrh) {
		return -2;
	}		
	ipmask = ccgi_ip2ulong((char*)maskz);

	ip_Nums = ipAddrh - ipAddrl;

	ret = ccgi_add_dhcp_pool_ip_range(add, ipAddrl, ipAddrh, ipmask, index,slot_id);

	if(ret==1)
		return 1;
	else
		return 0;
		
}


int ccgi_add_dhcp_static_host
(
	unsigned int ipaddr, 
	unsigned char* mac,
	unsigned char* ifname,
	unsigned int add,
	int slot_id
)/*返回1表示成功，返回0表示失败，返回-1表示error*/
{
	DBusMessage 	*query = NULL, *reply = NULL;
	DBusError 		err;
	unsigned int	op_ret = 0;
	int retu = 0;

	void *ccgi_connection = NULL;
	ccgi_ReInitDbusConnection(&ccgi_connection, slot_id, DISTRIBUTFAG);
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_ADD_STATIC_HOST);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &ipaddr,
							DBUS_TYPE_UINT32, &add,
							DBUS_TYPE_BYTE, &mac[0],	
							DBUS_TYPE_BYTE, &mac[1],
							DBUS_TYPE_BYTE, &mac[2],
							DBUS_TYPE_BYTE, &mac[3], 
							DBUS_TYPE_BYTE, &mac[4],
							DBUS_TYPE_BYTE, &mac[5],
							DBUS_TYPE_STRING, &ifname, 
							DBUS_TYPE_INVALID);		   
   reply = dbus_connection_send_with_reply_and_block (ccgi_connection,query,-1, &err);
			   
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
		   retu = 1;
	   }
	   else {
			retu = -1;
	   }
   } 
   else {
	   if (dbus_error_is_set(&err)) {
		   dbus_error_free(&err);
	   }
	   dbus_message_unref(reply);
   
	   retu = -1;
   }
   return retu;
}
int add_dhcp_static_host(char *ipz,char *macz,char *ifname,int slot_id)/*返回1表示成功，返回0表示失败*/
{
	unsigned int ipAddr = 0;
	unsigned int ret = 0;
	unsigned char macAddr[MAC_ADDRESS_LEN];
	ipAddr = ccgi_ip2ulong((char*)ipz);
	
	memset(&macAddr, 0, MAC_ADDRESS_LEN);	
	ret = parse_mac_addr((char *)macz, &macAddr);

	ret = ccgi_add_dhcp_static_host(ipAddr, macAddr,ifname, 1,slot_id);
	
	return ret;
}
int delete_dhcp_static_host(char *ipz,char *macz,char *ifname,int slot_id)/*返回1表示成功，返回0表示失败*/
{
	unsigned int ipAddr = 0;
	unsigned int ret = 0;
	unsigned char macAddr[MAC_ADDRESS_LEN];
	ipAddr = ccgi_ip2ulong((char*)ipz);
	
	memset(&macAddr, 0, MAC_ADDRESS_LEN);	
	ret = parse_mac_addr((char *)macz, &macAddr);

	ret = ccgi_add_dhcp_static_host(ipAddr, macAddr,ifname, 0,slot_id);
	
	return ret;
}

unsigned int ccgi_set_server_domain_name
(
	char *domainName,	
	unsigned int mode,
	unsigned int index,
	unsigned int del,
	int slot_id
)/*返回1表示成功，返回0表示失败，返回-1表示error，返回-2表示domainname非法*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;
	int retu = 0;

	if (!domainName) {
		return -2;
	}
	void *ccgi_connection = NULL;
	ccgi_ReInitDbusConnection(&ccgi_connection, slot_id, DISTRIBUTFAG);
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_SET_SERVER_DOMAIN_NAME);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_STRING, &domainName, 
							 DBUS_TYPE_UINT32, &mode,							 
							 DBUS_TYPE_UINT32, &index, 							 
							 DBUS_TYPE_UINT32, &del, 
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (ccgi_connection,query,-1, &err);
				
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
			retu = 1;
		}
	} 
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
	
		retu = -1;
	}
	return retu;
}
int ip_dhcp_server_domain_name(char *domainnamez,int modez,unsigned int pindex,int slot_id)/*返回1表示成功，返回0表示失败，返回-2表示Bad parameter,pool domainname NOT exists*/
{
	//char* domainName = NULL;
	unsigned int nameSize = 0,  index = 0;
	unsigned int ret = 0,  mode = 0;
	int retu=0;

	index = pindex;

	/*printf("before parse_vlan_name %s length %d\n",argv[1],strlen(argv[1]));*/
	//domainName = (char*)malloc(ALIAS_NAME_SIZE);
	//memset(domainName, 0, ALIAS_NAME_SIZE);

	//nameSize = strlen(domainnamez);
	//memcpy(domainName, domainnamez, nameSize);
	//domainName[nameSize+1] = '\0';
	/*mode 0 means global mode, 1 means pool mode */
	mode=(unsigned int)modez;
	/*if(CONFIG_NODE == vty->node) {
		mode = 0;
	}
	else if(POOL_NODE == vty->node){
		mode = 1;		
		index = (unsigned int *)(vty->index);
	}
	else {		
		//vty_out (vty, "Terminal mode change must under configure mode!\n", VTY_NEWLINE);
		return -2;
	}*/
	
	ret = ccgi_set_server_domain_name(domainnamez, mode, index, 0,slot_id);
	if (ret==1) {
		retu = 1;
	}
	else {
		//vty_out(vty,"% Bad parameter,pool %s NOT exists.\n", domainName);		
		retu = -2;
	}
	return retu;
}
int no_ip_dhcp_server_domain_name(char *domainnamez,unsigned int  index,int modez,int slot_id)/*返回1表示成功，返回0表示失败，返回-1表示Bad parameter,pool domainname NOT exists*/
{
	unsigned int ret = 0;
	int retu = 0;
	unsigned int mode = 0;
	mode=(unsigned int)modez;
	
	/*if(CONFIG_NODE == vty->node) {
		mode = 0;
	}
	else if(POOL_NODE == vty->node){
		mode = 1;		
		index = (unsigned int *)(vty->index);
	}
	else {		
		vty_out (vty, "Terminal mode change must under configure mode!\n", VTY_NEWLINE);
		return CMD_WARNING;
	}*/
	
	ret = ccgi_set_server_domain_name(domainnamez, mode, index, 1,slot_id);
	if (1 == ret) {
		retu = 1;
	}
	else {
		//vty_out(vty,"% Bad parameter,pool %s NOT exists.\n", domainName);		
		retu = -1;
	}
	return retu;
}
unsigned int ccgi_set_server_option43
(
	char *veo,	
	unsigned int mode,
	unsigned int index,
	unsigned int del,
	int slot_id
)/*返回1表示成功，返回0表示失败，返回-1表示error，返回-2*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;
	int retu = 0;

	if (!veo) {
		return -2;
	}
	void *ccgi_connection = NULL;
	ccgi_ReInitDbusConnection(&ccgi_connection, slot_id, DISTRIBUTFAG);
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_SET_SERVER_VEO_OLD);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_STRING, &veo, 
							 DBUS_TYPE_UINT32, &mode,							 
							 DBUS_TYPE_UINT32, &index, 							 
							 DBUS_TYPE_UINT32, &del, 
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (ccgi_connection,query,-1, &err);
				
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
			retu = 1;
		}
	} 
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
	
		retu = -1;
	}
	return retu;
}

int ip_dhcp_server_option43(char *veoz,int modez,unsigned int pindex,int slot_id)/*返回1表示成功，返回0表示失败，返回-1表示Bad parameter,pool veo NOT exists*/
{
	char* veo = NULL;
	unsigned int size = 0, index = 0;
	unsigned int ret = 0,  mode = 0;

	int retu = 0;
	size = strlen(veoz);
	veo = (char*)malloc(size + 1);
	memset(veo, 0, (size + 1));
	
	memcpy(veo, veoz, size);
	/*mode 0 means global mode, 1 means pool mode */
    mode=(unsigned int)modez;
	index = pindex;
    /*
	if(CONFIG_NODE == vty->node) {
		mode = 0;
	}
	else if(POOL_NODE == vty->node) {
		mode = 1;		
		index = (unsigned int *)(vty->index);
	}
	else {		
		vty_out (vty, "Terminal mode change must under configure mode!\n", VTY_NEWLINE);
		return CMD_WARNING;
	}
	*/
	
	ret = ccgi_set_server_option43(veo, mode, index, 0,slot_id);
	if (ret==1) {
		retu = 1;
	}
	else {
		//vty_out(vty,"% Bad parameter,pool %s NOT exists.\n", veo);		
		retu = -1;
	}
	if(NULL!=veo)
	{
		free(veo);
		veo=NULL;
	}
	return retu;
}
int no_ip_dhcp_server_option43(char *domainnamez,int modez,int slot_id)/*返回1表示成功，返回0表示失败，返回-1表示error*/
{
	char* veo = NULL;
	unsigned int ret = 0,  mode = 0;
	int retu=0;

	/*printf("before parse_vlan_name %s length %d\n",argv[1],strlen(argv[1]));*/
	veo = (char*)malloc(ALIAS_NAME_SIZE);
	memset(veo, 0, ALIAS_NAME_SIZE);

    mode=(unsigned int)modez;
	/*mode 0 means global mode, 1 means pool mode */
	
	/*if(CONFIG_NODE == vty->node) {
		mode = 0;
	}
	else if(POOL_NODE == vty->node){
		mode = 1;		
		index = (unsigned int *)(vty->index);
	}
	else {		
		vty_out (vty, "Terminal mode change must under configure mode!\n", VTY_NEWLINE);
		return CMD_WARNING;
	}*/
	
	ret = ccgi_set_server_option43(veo, mode, index, 1,slot_id);
	if (!ret) {
		retu = 1;
	}
	else {
		//vty_out(vty,"% Bad parameter,pool %s NOT exists.\n", veo);		
		retu = -1;
	}
	if(NULL!=veo)
	{
		free(veo);
		veo=NULL;
	}
	return retu;
}

unsigned int ccgi_set_server_dns
(
	unsigned int *ipAddr,
	unsigned int ipNum,
	unsigned int mode,
	unsigned int index,
	unsigned int del,
	int slot_id
)/*返回1表示成功，返回0表示失败，返回-1表示error*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;
	int retu=0;
	void *ccgi_connection = NULL;
	ccgi_ReInitDbusConnection(&ccgi_connection, slot_id, DISTRIBUTFAG);
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_SET_SERVER_DNS);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &ipNum,							 
							 DBUS_TYPE_UINT32, &ipAddr[0], 
							 DBUS_TYPE_UINT32, &ipAddr[1], 
							 DBUS_TYPE_UINT32, &ipAddr[2], 
							 DBUS_TYPE_UINT32, &mode,							 
							 DBUS_TYPE_UINT32, &index, 
							 DBUS_TYPE_UINT32, &del,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (ccgi_connection,query,-1, &err);
				
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
			retu = 1;
			}
		else if(DHCP_SET_ORDER_WANNING == op_ret)
		{
			//vty_out (vty, "first set ip range , Have not configured address pool range!\n");
			dbus_message_unref(reply);
			retu = -2;
		}
		else if(DHCP_NOT_FOUND_POOL  == op_ret) 
		{
			//vty_out (vty, "Not found the pool\n");
			dbus_message_unref(reply);
			retu = -3;
		}

	} 
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
	
		retu = -1;
	}
	return retu;
}

/*未定参数*/
int ip_dhcp_server_dns(int pmode,unsigned int pindex,char *ipaddr,int slot_id)
{
	unsigned int ipAddr[3] = {0, 0, 0}, ipnum = 0;
	unsigned int ret = 0, mode = 0, index = 0, i = 0,j = 0;
	int retu=0;	
	char *sep=",";
	char *token = NULL;
	char *iptmp=(char *)malloc(150);
	memset(iptmp,0,150);
	strcpy(iptmp,ipaddr);
	char *tmp=(char *)malloc(50);
	memset(tmp,0,50);

	char *ipaddrz[3];
	
	for(j=0;j<3;j++)
	{
		ipaddrz[j]=(char *)malloc(32);
	}

	mode = (unsigned int)pmode;
	index = pindex;

	token=strtok(iptmp,sep);
	i=0;
	while(token!=NULL)
	{			    
		memset(tmp,0,50);
		strcpy(tmp,token);
		strcpy(ipaddrz[i],tmp);
		token = strtok(NULL,sep);	
		i++;
		if(i==3)
			break;
	}   
	ipnum = i;
	
    for(i=0;i<3;i++)
    {
        ipAddr[i] = ccgi_ip2ulong((char*)ipaddrz[i]);	
    }
	for(j=0;j<3;j++)
	{
		free(ipaddrz[j]);		
	}
	//param is struct

	/*
	ipnum = argc;
	for(;i < ipnum; i++) {
		ipAddr[i] = ccgi_ip2ulong((char*)argv[i]);
	}
	*/

	/*
	if(CONFIG_NODE == vty->node) {
		mode = 0;
	}
	else if(POOL_NODE == vty->node) {
		mode = 1;		
		index = (unsigned int *)(vty->index);
	}
	else {		
		vty_out (vty, "Terminal mode change must under configure mode!\n", VTY_NEWLINE);
		return CMD_WARNING;
	}
	*/
	ret = ccgi_set_server_dns(ipAddr, ipnum, mode, index, 0,slot_id);
	if (ret==1) {
		retu = 1;
	}
	else {
		retu = -1;
	}
	free(iptmp);
	free(tmp);
	return retu;
}

int no_ip_dhcp_server_dns(int modez,unsigned int index,int slot_id)/*返回1表示成功，返回0表示失败，返回-1表示error*/
{
	unsigned int ipAddr[3] = {0, 0, 0}, ipnum = 0;
	unsigned int ret = 0,  mode = 0;
	int retu = 0;

    mode=(unsigned int)modez;
	/*if(CONFIG_NODE == vty->node) {
		mode = 0;
	}
	else if(POOL_NODE == vty->node) {
		mode = 1;		
		index = (unsigned int *)(vty->index);
	}
	else {		
		vty_out (vty, "Terminal mode change must under configure mode!\n", VTY_NEWLINE);
		return CMD_WARNING;
	}
	*/
	
	ret = ccgi_set_server_dns(ipAddr, ipnum, mode, index, 1,slot_id);
	if (ret==1) {
		retu = 1;
	}
	else {
		retu = -1;
	}
	return retu;
}

unsigned int ccgi_set_server_routers_ip
(
	unsigned int routers,	
	unsigned int mode,
	unsigned int index,
	unsigned int del,
	int slot_id
)/*返回1表示成功，返回0表示失败，返回-1表示error*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;
	int retu = 0;

	void *ccgi_connection = NULL;
	ccgi_ReInitDbusConnection(&ccgi_connection, slot_id, DISTRIBUTFAG);
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_SET_SERVER_ROUTERS_IP);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &routers, 
							 DBUS_TYPE_UINT32, &mode,							 
							 DBUS_TYPE_UINT32, &index,
							 DBUS_TYPE_UINT32, &del,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (ccgi_connection,query,-1, &err);
				
	dbus_message_unref(query);
	
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return 0;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) {
		if(!op_ret) 
		{	
			retu = 1;
			dbus_message_unref(reply);
			
		}else if(DHCP_SET_ORDER_WANNING == op_ret)
		{
			//vty_out (vty, "first set ip range , Have not configured address pool range!\n");
			dbus_message_unref(reply);
			retu = -2;
		}
		else if(DHCP_NOT_FOUND_POOL  == op_ret) 
		{
			//vty_out (vty, "Not found the pool\n");
			dbus_message_unref(reply);
			retu = -3;
		}

	} 
	else 
	{
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
	
		retu = -1;
	}
	return retu;
}
int ip_dhcp_server_routers_ip(char *ipz,unsigned int pindex,int pmode,int slot_id)/*返回1表示成功，返回0表示失败，返回-1表示error*/
{
	unsigned int ipAddr = 0, index = 0;
	unsigned int ret = 0,  mode = 0;
	int retu=0;

	ipAddr = ccgi_ip2ulong((char*)ipz);
	index = pindex;

	mode=(unsigned int)pmode;

	/*mode 0 means global mode, 1 means pool mode */
	/*if(CONFIG_NODE == vty->node) {
		mode = 0;
	}
	else if(POOL_NODE == vty->node){
		mode = 1;		
		index = (unsigned int *)(vty->index);
	}
	else {		
		vty_out (vty, "Terminal mode change must under configure mode!\n", VTY_NEWLINE);
		return CMD_WARNING;
	}*/
	ret = ccgi_set_server_routers_ip(ipAddr, mode, index, 0,slot_id);
	if (ret==1) 
	{
		retu  = 1;
	}
	else 
	{
		retu = -1;
	}
	return retu;
}
int no_ip_dhcp_server_routers_ip(int modez,unsigned int index,int slot_id)/*返回1表示成功，返回0表示失败，返回-1表示error*/
{
	unsigned int ipAddr = 0;
	unsigned int ret = 0,  mode = 0;
	int retu = 0;

    mode=(unsigned int)modez;
	/*mode 0 means global mode, 1 means pool mode */
	/*if(CONFIG_NODE == vty->node) {
		mode = 0;
	}
	else if(POOL_NODE == vty->node){
		mode = 1;		
		index = (unsigned int *)(vty->index);
	}
	else {		
		vty_out (vty, "Terminal mode change must under configure mode!\n", VTY_NEWLINE);
		return CMD_WARNING;
	}*/
	
	ret = ccgi_set_server_routers_ip(ipAddr, mode, index, 1,slot_id);
	if (ret==1) {
		retu = 1;
	}
	else {
		retu = -1;
	}
	return retu;	
}

unsigned int ccgi_set_server_wins_ip
(
	unsigned int wins,	
	unsigned int mode,
	unsigned int index,
	unsigned int del,
	int slot_id
)/*返回1表示成功，返回0表示失败，返回-1表示error*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;
	int retu = 0;

	void *ccgi_connection = NULL;
	ccgi_ReInitDbusConnection(&ccgi_connection, slot_id, DISTRIBUTFAG);
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_SET_SERVER_WINS_IP);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &wins, 
							 DBUS_TYPE_UINT32, &mode,							 
							 DBUS_TYPE_UINT32, &index,
							 DBUS_TYPE_UINT32, &del,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (ccgi_connection,query,-1, &err);
				
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return 0;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) {
		if(!op_ret) 
		{	
			dbus_message_unref(reply);
		    retu = 1;
		}
		else if(DHCP_SET_ORDER_WANNING == op_ret)
		{
			//vty_out (vty, "first set ip range , Have not configured address pool range!\n");
			dbus_message_unref(reply);
			retu = -2;
		}
		else if(DHCP_NOT_FOUND_POOL  == op_ret) 
		{
			//vty_out (vty, "Not found the pool\n");
			dbus_message_unref(reply);
			retu = -3;
		}

	} 
	else 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);	
		retu = -1;
	}
	return retu;
}
int ip_dhcp_server_wins_ip(char *ipaddrz,int modez,unsigned int pindex,int slot_id)/*返回1表示成功，返回0表示失败，返回-1表示error*/
{
	unsigned int ipAddr = 0, index = 0;
	unsigned int ret = 0,  mode = 0;
	int retu=0;

	ipAddr = ccgi_ip2ulong((char*)ipaddrz);
	mode=(unsigned int)modez;
    index = pindex;
	/*mode 0 means global mode, 1 means pool mode */
	/*if(CONFIG_NODE == vty->node) {
		mode = 0;
	}
	else if(POOL_NODE == vty->node){
		mode = 1;		
		index = (unsigned int *)(vty->index);
	}
	else {		
		//vty_out (vty, "Terminal mode change must under configure mode!\n", VTY_NEWLINE);
		return CMD_WARNING;
	}*/
	if (0 == ipAddr)
	{
		return -1;
	}
	
	ret = ccgi_set_server_wins_ip(ipAddr, mode, index, 0,slot_id);
	if (ret==1) 
	{
		retu = 1;
	}
	else 
	{
		retu = -1;
	}
	return retu;
}
int no_ip_dhcp_server_wins_ip(int modez,unsigned int index,int slot_id)/*返回1表示成功，返回0表示失败，返回-1表示error*/
{
	unsigned int ipAddr = 0;
	unsigned int ret = 0, mode = 0;
	int retu = 0;

	/*mode 0 means global mode, 1 means pool mode */
    mode=(unsigned int)modez;
	/*if(CONFIG_NODE == vty->node) {
		mode = 0;
	}
	else if(POOL_NODE == vty->node){
		mode = 1;		
		index = (unsigned int *)(vty->index);
	}
	else {		
		vty_out (vty, "Terminal mode change must under configure mode!\n", VTY_NEWLINE);
		return CMD_WARNING;
	}*/
	if (0 == ipAddr)
	{
		return -1;
	}
	ret = ccgi_set_server_wins_ip(ipAddr, mode, index, 1,slot_id);
	if (ret==1) {
		retu = 1;
	}
	else {
		retu = -1;
	}
	return retu;
}

unsigned int  ccgi_set_server_lease_default
(
	unsigned int lease_default,	
	unsigned int mode,
	unsigned int index,
	unsigned int del,
	int slot_id
)/*返回1表示成功，返回0表示失败，返回-1表示error*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;
	int retu = 0;
	void *ccgi_connection = NULL;
	ccgi_ReInitDbusConnection(&ccgi_connection, slot_id, DISTRIBUTFAG);
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_SET_SERVER_LEASE_DEFAULT);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &lease_default, 
							 DBUS_TYPE_UINT32, &mode,							 
							 DBUS_TYPE_UINT32, &index,
							 DBUS_TYPE_UINT32, &del,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (ccgi_connection,query,-1, &err);
				
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
			retu = 1;
		}
		else if(DHCP_SET_ORDER_WANNING == op_ret)
		{
			//vty_out (vty, "first set ip range , Have not configured address pool range!\n");
			dbus_message_unref(reply);
			retu = -2;
		}
		else if(DHCP_NOT_FOUND_POOL  == op_ret) 
		{
			//vty_out (vty, "Not found the pool\n");
			dbus_message_unref(reply);
			retu = -3;
		}

	} 
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
	
		retu = -1;
	}
	return retu;
}
int ip_dhcp_server_lease_default(char *leasez,int modez,unsigned int pindex,int slot_id)/*返回1表示成功，返回0表示失败，返回-1表示error*/
{
	unsigned int lease_default = 0, index = 0;
	unsigned int ret = 0,  mode = 0;
	int retu = 0;

	lease_default = (unsigned int)strtoul(leasez,0,10);
	
    mode=(unsigned int)modez;
	index = pindex;
	
	/*mode 0 means global mode, 1 means pool mode */
	/*if(CONFIG_NODE == vty->node) {
		mode = 0;
	}
	else if(POOL_NODE == vty->node){
		mode = 1;		
		index = (unsigned int *)(vty->index);
	}
	else {		
		vty_out (vty, "Terminal mode change must under configure mode!\n", VTY_NEWLINE);
		return CMD_WARNING;
	}*/	
	ret = ccgi_set_server_lease_default(lease_default, mode, index, 0,slot_id);
	if (ret == 1) {
		retu = 1;
	}
	else {
		retu = -1;
	}
	return retu;
}

int no_ip_dhcp_server_lease_default(int modez,unsigned int index,int slot_id)/*返回1表示成功，返回0表示失败，返回-1表示error*/
{
	unsigned int lease_default = 0;
	unsigned int ret = 0,  mode = 0;
	int retu = 0;

	/*mode 0 means global mode, 1 means pool mode */
    mode=(unsigned int)modez;
	/*if(CONFIG_NODE == vty->node) {
		mode = 0;
	}
	else if(POOL_NODE == vty->node){
		mode = 1;		
		index = (unsigned int *)(vty->index);
	}
	else {		
		vty_out (vty, "Terminal mode change must under configure mode!\n", VTY_NEWLINE);
		return CMD_WARNING;
	}*/
	
	ret = ccgi_set_server_lease_default(lease_default, mode, index, 1,slot_id);

	if (ret==1) {
		retu = 1;
	}
	else {
		retu = -1;
	}
	return retu;
}

unsigned int ccgi_set_server_lease_max
(
	unsigned int lease_max,	
	unsigned int mode,
	unsigned int index,
	unsigned int del
)/*返回1表示成功，返回0表示失败，返回-1表示error*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;
	int retu = 0;

	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_SET_SERVER_LEASE_MAX);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &lease_max, 
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
			retu = 1;
		}
		else if(DHCP_SET_ORDER_WANNING == op_ret)
		{
			//vty_out (vty, "first set ip range , Have not configured address pool range!\n");
			dbus_message_unref(reply);
			retu = -2;
		}
		else if(DHCP_NOT_FOUND_POOL  == op_ret) 
		{
			//vty_out (vty, "Not found the pool\n");
			dbus_message_unref(reply);
			retu = -3;
		}

	} 
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
	
		retu = -1;
	}
	return retu;
}
int ip_dhcp_server_lease_max(char *leasemaxz,int modez,unsigned int pindex)/*返回1表示成功，返回0表示失败，返回-1表示error*/
{
	unsigned int lease_max = 0, index = 0;
	unsigned int ret = 0,  mode = 0;
	int retu = 0;

	lease_max = atoi((char*)leasemaxz);
	lease_max = lease_max*60*60*24;

	mode=(unsigned int)modez;
	index = pindex;
	
	/*mode 0 means global mode, 1 means pool mode */
	/*if(CONFIG_NODE == vty->node) {
		mode = 0;
	}
	else if(POOL_NODE == vty->node){
		mode = 1;		
		index = (unsigned int *)(vty->index);
	}
	else {		
		vty_out (vty, "Terminal mode change must under configure mode!\n", VTY_NEWLINE);
		return CMD_WARNING;
	}*/
	
	ret = ccgi_set_server_lease_max(lease_max, mode, index, 0);
	if (ret==1) 
	{
		retu = 1;
	}
	else 
	{
		retu = -1;
	}
	return retu;
}
int no_ip_dhcp_server_lease_max(int modez,unsigned int pindex)/*返回1表示成功，返回0表示失败*/
{
	unsigned int lease_max = 0, index = 0;
	unsigned int ret = 0,  mode = 0;
	int retu = 0;

	mode=(unsigned int)modez;
	index = pindex;

	/*mode 0 means global mode, 1 means pool mode */
	/*
	if(CONFIG_NODE == vty->node) {
		mode = 0;
	}
	else if(POOL_NODE == vty->node){
		mode = 1;		
		index = (unsigned int *)(vty->index);
	}
	else {		
		vty_out (vty, "Terminal mode change must under configure mode!\n", VTY_NEWLINE);
		return CMD_WARNING;
	}
	*/
	
	ret = ccgi_set_server_lease_max(lease_max, mode, index, 1);
	if (ret==1) {
		retu = 1;
	}
	else {
		retu = 0;
	}
	return retu;
}

unsigned int ccgi_set_server_enable(unsigned int enable,int slot_id)/*返回1表示成功，返回0表示失败，返回-1表示error*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;
	int retu=0;

	void *ccgi_connection = NULL;
	ccgi_ReInitDbusConnection(&ccgi_connection, slot_id, DISTRIBUTFAG);
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_SET_SERVER_ENABLE);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &enable, 
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (ccgi_connection,query,-1, &err);
				
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
			retu = 1;
		}
	} 
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
	
		retu = -1;
	}
	return retu;
}
int ip_dhcp_server_enable(char *endis,int slot_id)/*返回1表示成功，返回0表示失败，返回-2表示bad command parameter*/
{
	unsigned int ret = 0, isEnable = 0;
	int retu=0;

	if(strncmp("enable",endis,strlen(endis))==0) {
		isEnable = 1;
	}
	else if (strncmp("disable",endis,strlen(endis))==0) {
		isEnable = 0;
	}
	else {
		//vty_out(vty,"bad command parameter!\n");
		return -2;
	}

	ret = ccgi_set_server_enable(isEnable,slot_id);
	
	if (ret==1) {
		retu = 1;
	}
	else {
		retu = 0;
	}
	return retu;
}

unsigned int ccgi_set_failover
(
	unsigned int mode,
	unsigned int index,
	struct dhcp_failover_show* failover_conf,
	int slot_id
)/*返回1表示成功，返回0表示失败*/
{
	DBusMessage 	*query = NULL, *reply = NULL;
	DBusError 		err;
	unsigned int op_ret = 0;
	char *failname = NULL;
	int retu = 0;

	failname = malloc(strlen(failover_conf->name) + 1);
	memset(failname, 0, (strlen(failover_conf->name) + 1));
	memcpy(failname, failover_conf->name, strlen(failover_conf->name));

	void *ccgi_connection = NULL;
	ccgi_ReInitDbusConnection(&ccgi_connection, slot_id, DISTRIBUTFAG);
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_ADD_DHCP_FAILOVER_PEER);
	dbus_error_init(&err);
	dbus_message_append_args(query,		
							DBUS_TYPE_UINT32, &mode,
							DBUS_TYPE_UINT32, &index,
							DBUS_TYPE_STRING, &(failname),
							DBUS_TYPE_UINT32, &(failover_conf->primary),
							DBUS_TYPE_UINT32, &(failover_conf->split),
							DBUS_TYPE_UINT32, &(failover_conf->mclt),
							DBUS_TYPE_UINT32, &(failover_conf->dstip),
							DBUS_TYPE_UINT32, &(failover_conf->dstport),
							DBUS_TYPE_UINT32, &(failover_conf->srcip),
							DBUS_TYPE_UINT32, &(failover_conf->srcport),
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (ccgi_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		if(NULL!=failname)
		{
			free(failname);
			failname=NULL;
		}
		return 0;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32, &op_ret,
		DBUS_TYPE_INVALID)) {
		if (!op_ret ) {
			retu = 1;
		}			
	} 
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return retu ;

}
int add_dhcp_failover_pool(int modez,char *fname,char *prim,char *splitz,char *mcltz,char *dstipz,char *dstportz,char *srcipz,char *srcportz,int slot_id)
{
	unsigned int nameSize = 0, mode = 0, index = 0, ret = 0;
	struct dhcp_failover_show conf;

	/*if (CONFIG_NODE == vty->node) {
		mode = 0;		
		vty_out(vty, "configure failover must under pool mode!\n", VTY_NEWLINE);
		return CMD_WARNING;
	}
	else if (POOL_NODE == vty->node) {
		mode = 1;		
		index = (unsigned int *)(vty->index);
	}*/
	mode=(unsigned int)modez;
	
	memset(&conf, 0, sizeof(struct dhcp_failover_show));
	
	nameSize = strlen(fname);
	if (nameSize >= ALIAS_NAME_SIZE) {
		//vty_out(vty, "failover name is too long!\n", VTY_NEWLINE);
		return -2;
	}
	memcpy(conf.name, fname, nameSize);

	if (strncmp("primary", prim, strlen(prim))) {
		conf.primary = 1;
	}
	
	conf.split = atoi((char*)splitz);
				
	conf.mclt = atoi((char*)mcltz);
	
	conf.dstip = ccgi_ip2ulong((char*)dstipz);

	conf.dstport = atoi((char*)dstportz);
	
	conf.srcip = ccgi_ip2ulong((char*)srcipz);

	conf.srcport = atoi((char*)srcportz);
	
	ret = ccgi_set_failover(mode, index, &conf,slot_id);	
	return 0;
}

unsigned int  ccgi_config_failover_pool
(
	unsigned int mode,
	unsigned int index,
	struct dhcp_failover_show* failover_conf,
	int slot_id
)/*返回1表示成功，返回0表示失败*/
{
	DBusMessage 	*query = NULL, *reply = NULL;
	DBusError 		err;
	unsigned int op_ret = 0;
	char *failname = NULL;
	int retu = 0 ;

	failname = malloc(strlen(failover_conf->name) + 1);
	memset(failname, 0, (strlen(failover_conf->name) + 1));
	memcpy(failname, failover_conf->name, strlen(failover_conf->name));

	void *ccgi_connection = NULL;
	ccgi_ReInitDbusConnection(&ccgi_connection, slot_id, DISTRIBUTFAG);
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_CFG_DHCP_FAILOVER_PEER);
	dbus_error_init(&err);
	dbus_message_append_args(query,		
							DBUS_TYPE_UINT32, &mode,
							DBUS_TYPE_UINT32, &index,
							DBUS_TYPE_STRING, &(failname),
							DBUS_TYPE_UINT32, &(failover_conf->primary),
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (ccgi_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		if(NULL!=failname)
		{
			free(failname);
			failname=NULL;
		}
		return 0;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32, &op_ret,
		DBUS_TYPE_INVALID)) {
		if (!op_ret ) {
			retu = 1;
		}			
	} 
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return retu ;

}
int cfg_dhcp_failover_pool(int modez,char *fname,char *prim,int slot_id)
{
	unsigned int nameSize = 0, mode = 0, index = 0, ret = 0;
	struct dhcp_failover_show conf;

	/*if (CONFIG_NODE == vty->node) {
		mode = 0;		
		vty_out(vty, "configure failover must under pool mode!\n", VTY_NEWLINE);
		return CMD_WARNING;
	}
	else if (POOL_NODE == vty->node) {
		mode = 1;		
		index = (unsigned int *)(vty->index);
	}
	*/
	mode=(unsigned int)modez;
	memset(&conf, 0, sizeof(struct dhcp_failover_show));
	
	nameSize = strlen(fname);
	if (nameSize >= ALIAS_NAME_SIZE) {
		//vty_out(vty, "failover name is too long!\n", VTY_NEWLINE);
		return -2;
	}
	memcpy(conf.name, fname, nameSize);

	if (strncmp("primary", prim, strlen(prim))) {
		conf.primary = 1;
	}
	
	ret = ccgi_config_failover_pool(mode, index, &conf,slot_id);	

	if(ret==1)
		return 1;
	else
		return 0;

	
}

unsigned int ccgi_delete_failover_pool
(
	unsigned int mode,
	unsigned int index,
	struct dhcp_failover_show* failover_conf,
	int slot_id
)/*返回1表示成功，返回0表示失败*/
{
	DBusMessage 	*query = NULL, *reply = NULL;
	DBusError 		err;
	unsigned int op_ret = 0;
	char *failname = NULL;
	int retu = 0;

	failname = malloc(strlen(failover_conf->name) + 1);
	memset(failname, 0, (strlen(failover_conf->name) + 1));
	memcpy(failname, failover_conf->name, strlen(failover_conf->name));
	void *ccgi_connection = NULL;
	ccgi_ReInitDbusConnection(&ccgi_connection, slot_id, DISTRIBUTFAG);	
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_DEL_DHCP_FAILOVER_PEER);
	dbus_error_init(&err);
	dbus_message_append_args(query,		
							DBUS_TYPE_UINT32, &mode,
							DBUS_TYPE_UINT32, &index,
							DBUS_TYPE_STRING, &(failname),
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (ccgi_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		if(NULL!=failname)
		{
			free(failname);
			failname=NULL;
		}
		return 0;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32, &op_ret,
		DBUS_TYPE_INVALID)) {
		if (!op_ret ) {
			retu = 1;
		}			
	} 
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return retu;

}
int del_dhcp_failover_pool(int modez,char *fname,int slot_id)/*返回1表示成功，返回0表示失败，返回-1表示failover name is too long*/
{
	unsigned int nameSize = 0, mode = 0, index = 0,  ret = 0;
	struct dhcp_failover_show conf;

	/*if (CONFIG_NODE == vty->node) {
		mode = 0;		
		vty_out(vty, "configure failover must under pool mode!\n", VTY_NEWLINE);
		return CMD_WARNING;
	}
	else if (POOL_NODE == vty->node) {
		mode = 1;		
		index = (unsigned int *)(vty->index);
	}*/
	mode=(unsigned int)modez;
	memset(&conf, 0, sizeof(struct dhcp_failover_show));
	
	nameSize = strlen(fname);
	if (nameSize >= ALIAS_NAME_SIZE) {
		//vty_out(vty, "failover name is too long!\n", VTY_NEWLINE);
		return -1;
	}
	memcpy(conf.name, fname, nameSize);
	
	ret = ccgi_delete_failover_pool(mode, index, &conf,slot_id);	

	if(ret==1)
		return 1;
	else
		return 0;
	
}


unsigned int ccgi_show_failover_configure
(
	unsigned int mode,
	unsigned int index,
	struct dhcp_failover_show *failover_cfg,
	int slot_id
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;
	char* failname = NULL;
	void *ccgi_connection = NULL;
	ccgi_ReInitDbusConnection(&ccgi_connection, slot_id, DISTRIBUTFAG);
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_SHOW_FAILOVER_CFG);
	

	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &mode,
							 DBUS_TYPE_UINT32, &index,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_connection,query,-1, &err);	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return 0;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32, &op_ret,
		DBUS_TYPE_UINT32, &(failover_cfg->primary),
		DBUS_TYPE_UINT32, &(failover_cfg->split),
		DBUS_TYPE_UINT32, &(failover_cfg->mclt),
		DBUS_TYPE_UINT32, &(failover_cfg->dstip),
		DBUS_TYPE_UINT32, &(failover_cfg->dstport),
		DBUS_TYPE_UINT32, &(failover_cfg->srcip),
		DBUS_TYPE_UINT32, &(failover_cfg->srcport),
		DBUS_TYPE_STRING, &(failname),
		DBUS_TYPE_INVALID)) {
		if (!op_ret ) {
			strcpy(failover_cfg->name, failname);
		}			
	} 
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);


	return 1;

}
char* ip_long2string(unsigned long ipAddress, char *buff)
{
	unsigned long	cnt = 0;
	unsigned char *tmpPtr = buff;
	
	cnt = sprintf((char*)tmpPtr,"%ld.%ld.%ld.%ld\n",(ipAddress>>24) & 0xFF, \
			(ipAddress>>16) & 0xFF,(ipAddress>>8) & 0xFF,ipAddress & 0xFF);
	
	return tmpPtr;
}


int if_mask_bit(char *mask)
{
	int k=0,i;
	int flag = -1;
    char *mask_val[32] = {
             "128.0.0.0",
             "192.0.0.0",
             "224.0.0.0",
             "240.0.0.0",
             "248.0.0.0",
             "252.0.0.0",
             "254.0.0.0",
             "255.0.0.0",
             "255.128.0.0",
             "255.192.0.0",
             "255.224.0.0",
             "255.240.0.0",
             "255.248.0.0",
             "255.252.0.0",
             "255.254.0.0",
             "255.255.0.0",
             "255.255.128.0",
             "255.255.192.0",
             "255.255.224.0",
             "255.255.240.0",
             "255.255.248.0",
             "255.255.252.0",
             "255.255.254.0",
             "255.255.255.0",
             "255.255.255.128",
             "255.255.255.192",
             "255.255.255.224",
             "255.255.255.240",
             "255.255.255.248",
             "255.255.255.252",
             "255.255.255.254",
             "255.255.255.255"
    	};

	for(i=0;i<32;i++)
	{
	    k ++;
	    if(strcmp(mask,mask_val[i])==0)
		{
		    flag = 0;
			break;
		}		
	}
	if(flag == 0)
	    return k;
	else
		return flag;
}

unsigned int ccgi_set_server_static_arp_enable(unsigned int enable,int slot_id)/*返回1表示成功，返回0表示失败*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;
	int retu = 0;
	void *ccgi_connection = NULL;
	ccgi_ReInitDbusConnection(&ccgi_connection, slot_id, DISTRIBUTFAG);
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_SET_SERVER_STATIC_ENABLE);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &enable, 
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (ccgi_connection,query,-1, &err);
				
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
			retu = 1;
		}
	} 
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
	
		retu = -1;
	}
	return retu;
}

int ccgi_dhcp_check_relay_interface_iSbusy(char *ifname,int slot_id)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;
	int detect = 0;
	int retu = 1;
	void *ccgi_connection = NULL;
	ccgi_ReInitDbusConnection(&ccgi_connection, slot_id, DISTRIBUTFAG);
	query = dbus_message_new_method_call(DHCRELAY_DBUS_BUSNAME, 
									DHCRELAY_DBUS_OBJPATH, 
									DHCRELAY_DBUS_INTERFACE, 
									DHCRELAY_DBUS_METHOD_CHECK_RELAY_INTERFACE);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_STRING, &ifname,
							DBUS_TYPE_UINT32, &detect,					 
							DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block(ccgi_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return 0;
	}
	if (dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, &op_ret,
		DBUS_TYPE_INVALID)) {
        if(op_ret){
            /*vty_out(vty,"saved dhcp lease success\n");*/
		}
	} 
	else {		
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	
	return op_ret;

}

/*dhcp relay config function*/

/*param = 1:enable,param = 0:disable */
unsigned int ccgi_set_relay_enable(unsigned int enable,int slot_id)/*return 1:succ; return 0:fail*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;
	int retu = 0;

	void *ccgi_connection = NULL;
	ccgi_ReInitDbusConnection(&ccgi_connection, slot_id, DISTRIBUTFAG);
	query = dbus_message_new_method_call(DHCRELAY_DBUS_BUSNAME, 
									DHCRELAY_DBUS_OBJPATH, 
									DHCRELAY_DBUS_INTERFACE, 
									DHCRELAY_DBUS_METHOD_SET_RELAY_ENABLE);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &enable, 
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (ccgi_connection,query,-1, &err);
				
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
			retu = 1;
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

/*0 del 1 add*/
unsigned int ccgi_set_interface_ip_relay
(
	char *upifname,
	char *downifname,
	unsigned int ipaddr,
	unsigned int add_info,
	int slot_id
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0, ret = 0;

	ret = ccgi_dhcp_check_server_interface_iSbusy(upifname, downifname,1);
	if (ret != 1) {
		//vty_out(vty, "interface is busy\n");
		return -1;
	}
	void *ccgi_connection = NULL;
	ccgi_ReInitDbusConnection(&ccgi_connection, slot_id, DISTRIBUTFAG);
	query = dbus_message_new_method_call(DHCRELAY_DBUS_BUSNAME, 
									DHCRELAY_DBUS_OBJPATH, 
									DHCRELAY_DBUS_INTERFACE, 
									DHCRELAY_DBUS_METHOD_SET_DHCP_RELAY);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_STRING, &downifname,
							DBUS_TYPE_STRING, &upifname,
							DBUS_TYPE_UINT32, &ipaddr,
							DBUS_TYPE_UINT32, &add_info,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (ccgi_connection,query,-1, &err);
				
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

		if (op_ret == 0)
		{
			return 1;
		}
		else
		{
			return -2;
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
int ccgi_dhcp_check_server_interface_iSbusy
(
	char *upifname,
	char *downifname,
	int slot_id
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;
	int detect = 0;
	int retu = 0;
	void *ccgi_connection = NULL;
	ccgi_ReInitDbusConnection(&ccgi_connection, slot_id, DISTRIBUTFAG);	
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_CHECK_SERVER_INTERFACE);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_STRING, &upifname,					
							DBUS_TYPE_STRING, &downifname,
							DBUS_TYPE_UINT32, &detect,					 
							DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block(ccgi_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return 0;
	}
	if (dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, &op_ret,
		DBUS_TYPE_INVALID)) {
        if(op_ret){
            /*vty_out(vty,"saved dhcp lease success\n");*/
			retu = 1;
		}
	} 
	else {		
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	
	return retu;
}


unsigned int ccgi_show_dhcp_relay
(
struct dhcp_relay_show_st *relayhead,
unsigned int *node_num,
unsigned int *ifenable,
int slot_id
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_struct,iter_array;
	unsigned int op_ret = 0,i = 0;
	unsigned int ipaddr = 0, count = 0, value = 0, isenable= 0;
	char *upifname = NULL, *downifname = NULL;
	int retu = 0;
	void *ccgi_connection = NULL;	
	ccgi_ReInitDbusConnection(&ccgi_connection, slot_id, DISTRIBUTFAG);	
	query = dbus_message_new_method_call(DHCRELAY_DBUS_BUSNAME, 
									DHCRELAY_DBUS_OBJPATH, 
									DHCRELAY_DBUS_INTERFACE, 
									DHCRELAY_DBUS_METHOD_SHOW_DHCP_RELAY);
	

	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&value,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		retu = 0;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&op_ret);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&isenable);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&count);
	dbus_message_iter_next(&iter);	
	*ifenable = isenable;

	struct dhcp_relay_show_st *relaytail = NULL;
	relayhead->next = NULL;
	relaytail = relayhead;
	
	if (!op_ret) {
		if(count > 0) {	
			dbus_message_iter_recurse(&iter,&iter_array);
			for(i = 0; i < count; i++) 
			{
				struct dhcp_relay_show_st *rq = NULL;
				rq = (struct dhcp_relay_show_st *)malloc(sizeof(struct dhcp_relay_show_st)+1);
				if (NULL == rq)
				{
					return -1;
				}
				memset(rq, 0, (sizeof(struct dhcp_relay_show_st)+1));

				dbus_message_iter_recurse(&iter_array,&iter_struct);
			
				dbus_message_iter_get_basic(&iter_struct, &ipaddr);
				dbus_message_iter_next(&iter_struct);
				
	 			dbus_message_iter_get_basic(&iter_struct, &upifname);
				dbus_message_iter_next(&iter_struct);

			 	dbus_message_iter_get_basic(&iter_struct, &downifname);
				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_next(&iter_array);

				rq->ipaddr = ipaddr;
				strcpy(rq->upifname, upifname);		
				strcpy(rq->downifname, downifname);

				rq->next = NULL;
				relaytail->next = rq;
				relaytail = rq;
			}
			retu = 1;
		}
	}
	*node_num = count;
	dbus_message_unref(reply);
	return retu;
}
void Free_dhcprelay_info(struct dhcp_relay_show_st *head)
{
    struct dhcp_relay_show_st *f1,*f2;
	f1=head->next;	
	if (NULL == f1)
	{
		return ;
	}
	f2=f1->next;
	while(f2!=NULL)
	{
	  free(f1);
	  f1=f2;
	  f2=f2->next;
	}
	free(f1);
}


unsigned int  ccgi_set_server_no_option43(	unsigned int index,int slot_id)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;

	void *ccgi_connection = NULL;
	ccgi_ReInitDbusConnection(&ccgi_connection, slot_id, DISTRIBUTFAG);
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_SET_SERVER_NO_VEO);

	dbus_error_init(&err);
	dbus_message_append_args(query,							 					 
							 DBUS_TYPE_UINT32, &index, 						
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (ccgi_connection,query,-1, &err);
				
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
		}else{

			//vty_out (vty, "fail operation\n");
			dbus_message_unref(reply);
			return -1;
		}
	}else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
 
		return -2;
	}	
}


unsigned int ccgi_set_server_option43_veo
(
	char *ap_via_address,	
	unsigned int mode,
	unsigned int index,
	unsigned int del,
	int slot_id
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;
	int retu = 0;

	if (NULL == ap_via_address ) {
		return -1;
	}
	void *ccgi_connection = NULL;
	ccgi_ReInitDbusConnection(&ccgi_connection, slot_id, DISTRIBUTFAG);
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_SET_SERVER_VEO);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_STRING, &ap_via_address , 							  
							 DBUS_TYPE_UINT32, &mode,							 
							 DBUS_TYPE_UINT32, &index, 							 
							 DBUS_TYPE_UINT32, &del, 
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (ccgi_connection,query,-1, &err);
				
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		retu = -2;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) {
		if(!op_ret) {	
			dbus_message_unref(reply);
			retu = 1;
		}else if(DHCP_SET_ORDER_WANNING == op_ret){
			//vty_out (vty, "first set ip range , Have not configured address pool range!\n");
			dbus_message_unref(reply);
			retu = -4;
		}else if(DHCP_NOT_FOUND_POOL  == op_ret) {
			//vty_out (vty, "Not found the pool\n");
			dbus_message_unref(reply);
			retu = -5;

		}else if(DHCP_MAX_OPTION_LIST_WANNING  == op_ret) {
			//vty_out (vty, "Max set sixteen AP address\n");
			dbus_message_unref(reply);
			retu = -6;

		}else if(DHCP_ALREADY_ADD_OPTION  == op_ret) {
			//vty_out (vty, "The option43 already add\n");
			dbus_message_unref(reply);
			retu = -7;

		}else{
			//vty_out (vty, "fail operation\n");
			dbus_message_unref(reply);
			retu = -8;
		}
	} 
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return -9;
	}
	return retu;
}


int ccgi_show_lease_state
(	
	struct lease_state  *total_state, 
	struct sub_lease_state *sub_state,
	unsigned int *subnet_num,
	int slot_id
)/*0:fail; 1:success; -1:error*/
{
	char *cursor = NULL;
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
    int ret = 1;
	int retu  = 0;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_struct,iter_array;
	unsigned int sub_num = 0;
	int i = 0;
	
	if(!total_state || !sub_state || !subnet_num){
		//return CMD_WARNING;
		return -1;
	}
	//ReInitDbusConnection(&ccgi_dbus_connection, slot_id, distributFag);
	void *ccgi_connection = NULL;
	ccgi_ReInitDbusConnection(&ccgi_connection, slot_id, DISTRIBUTFAG);
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_SHOW_LEASE_STATE);
	
	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (ccgi_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		//printf("DHCP show lease state failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		//return ret;
		return 0;
	}	

	dbus_message_iter_init(reply,&iter);
	
	dbus_message_iter_get_basic(&iter,&(total_state->total_lease_num));
	dbus_message_iter_next(&iter);	
	
	dbus_message_iter_get_basic(&iter,&(total_state->active_lease_num));
	dbus_message_iter_next(&iter);	

	dbus_message_iter_get_basic(&iter,&(total_state->free_lease_num));
	dbus_message_iter_next(&iter);	

	dbus_message_iter_get_basic(&iter,&(total_state->backup_lease_num));
	dbus_message_iter_next(&iter);		
	
	dbus_message_iter_get_basic(&iter,&sub_num);
	dbus_message_iter_next(&iter);		

	*subnet_num = sub_num;
	

	dbus_message_iter_recurse(&iter,&iter_array);

	/*get subnet lease state*/
	for (i = 0; i < *subnet_num; ++i) {

		retu = 1;
		dbus_message_iter_recurse(&iter_array,&iter_struct);
		
		dbus_message_iter_get_basic(&iter_struct, &(sub_state[i].subnet));
		dbus_message_iter_next(&iter_struct); 
	
		dbus_message_iter_get_basic(&iter_struct, &(sub_state[i].mask));
		dbus_message_iter_next(&iter_struct); 
		
		dbus_message_iter_get_basic(&iter_struct, &(sub_state[i].subnet_lease_state.total_lease_num));
		dbus_message_iter_next(&iter_struct); 
		
		dbus_message_iter_get_basic(&iter_struct, &(sub_state[i].subnet_lease_state.active_lease_num));
		dbus_message_iter_next(&iter_struct); 
		
		dbus_message_iter_get_basic(&iter_struct, &(sub_state[i].subnet_lease_state.free_lease_num));
		dbus_message_iter_next(&iter_struct); 
		
		dbus_message_iter_get_basic(&iter_struct, &(sub_state[i].subnet_lease_state.backup_lease_num));
		dbus_message_iter_next(&iter_struct); 

		dbus_message_iter_next(&iter_array);
	} 

	dbus_message_unref(reply);

	return retu;	
}


int ccgi_show_lease_state_slot
(	
	struct lease_state  *total_state, 
	struct sub_lease_state *sub_state,
	unsigned int *subnet_num,
	DBusConnection *connection
)/*0:fail; 1:success; -1:error*/
{
	char *cursor = NULL;
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
    int ret = 1;
	int retu  = 0;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_struct,iter_array;
	unsigned int sub_num = 0;
	int i = 0;
	
	if(!total_state || !sub_state || !subnet_num){
		//return CMD_WARNING;
		return -1;
	}
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_SHOW_LEASE_STATE);
	
	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		//printf("DHCP show lease state failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		//return ret;
		return 0;
	}	

	dbus_message_iter_init(reply,&iter);
	
	dbus_message_iter_get_basic(&iter,&(total_state->total_lease_num));
	dbus_message_iter_next(&iter);	
	
	dbus_message_iter_get_basic(&iter,&(total_state->active_lease_num));
	dbus_message_iter_next(&iter);	

	dbus_message_iter_get_basic(&iter,&(total_state->free_lease_num));
	dbus_message_iter_next(&iter);	

	dbus_message_iter_get_basic(&iter,&(total_state->backup_lease_num));
	dbus_message_iter_next(&iter);		
	
	dbus_message_iter_get_basic(&iter,&sub_num);
	dbus_message_iter_next(&iter);		

	*subnet_num = sub_num;
	

	dbus_message_iter_recurse(&iter,&iter_array);

	/*get subnet lease state*/
	for (i = 0; i < *subnet_num; ++i) {

		retu = 1;
		dbus_message_iter_recurse(&iter_array,&iter_struct);
		
		dbus_message_iter_get_basic(&iter_struct, &(sub_state[i].subnet));
		dbus_message_iter_next(&iter_struct); 
	
		dbus_message_iter_get_basic(&iter_struct, &(sub_state[i].mask));
		dbus_message_iter_next(&iter_struct); 
		
		dbus_message_iter_get_basic(&iter_struct, &(sub_state[i].subnet_lease_state.total_lease_num));
		dbus_message_iter_next(&iter_struct); 
		
		dbus_message_iter_get_basic(&iter_struct, &(sub_state[i].subnet_lease_state.active_lease_num));
		dbus_message_iter_next(&iter_struct); 
		
		dbus_message_iter_get_basic(&iter_struct, &(sub_state[i].subnet_lease_state.free_lease_num));
		dbus_message_iter_next(&iter_struct); 
		
		dbus_message_iter_get_basic(&iter_struct, &(sub_state[i].subnet_lease_state.backup_lease_num));
		dbus_message_iter_next(&iter_struct); 

		dbus_message_iter_next(&iter_array);
	} 

	dbus_message_unref(reply);

	return retu;	
}
int ccgi_show_dhcp_pool_statistics(int slot_id, struct all_lease_state_p *head)
{	
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_struct,iter_array;

	int ret = 1, i = 0;
	unsigned int subnet_num = 0;
	float tmp = 0;
	int len = 0;
	char *cursor = NULL;
	char buf[256];
	struct lease_state total_state;
	struct sub_lease_state sub_state[MAX_SUB_NET];

	memset(&total_state, 0, sizeof(struct lease_state));
	memset(sub_state, 0, sizeof(sub_state));

	void  *ccgi_connection = NULL;
	ccgi_ReInitDbusConnection(&ccgi_connection, slot_id, DISTRIBUTFAG);	
	
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_SHOW_LEASE_STATISTICS);
	
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (ccgi_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return 0;
	}	

	dbus_message_iter_init(reply, &iter);
	
	dbus_message_iter_get_basic(&iter,&(total_state.total_lease_num));
	dbus_message_iter_next(&iter);	

	head->all_lease_state.total_lease_num = total_state.total_lease_num;
	
	dbus_message_iter_get_basic(&iter,&(total_state.active_lease_num));
	dbus_message_iter_next(&iter);	

	head->all_lease_state.active_lease_num = total_state.active_lease_num;

	dbus_message_iter_get_basic(&iter,&(total_state.free_lease_num));
	dbus_message_iter_next(&iter);	

	head->all_lease_state.free_lease_num = total_state.free_lease_num;
	
	dbus_message_iter_get_basic(&iter,&(total_state.backup_lease_num));
	dbus_message_iter_next(&iter);		
	
	dbus_message_iter_get_basic(&iter,&subnet_num);
	dbus_message_iter_next(&iter);

	head->subnet_num = subnet_num;

	dbus_message_iter_recurse(&iter,&iter_array);

	for (i = 0; i < subnet_num; ++i) {

        char *temp_subnet = NULL;
        char *temp_mask = NULL;
        char *poolname = NULL;
        
		dbus_message_iter_recurse(&iter_array,&iter_struct);
		
		dbus_message_iter_get_basic(&iter_struct, &temp_subnet);
		dbus_message_iter_next(&iter_struct); 
        
		strncpy(head->sub_state[i].subnet, temp_subnet, sizeof(head->sub_state[i].subnet) - 1);
	
		dbus_message_iter_get_basic(&iter_struct, &temp_mask);
		dbus_message_iter_next(&iter_struct); 

		strncpy(head->sub_state[i].mask, temp_mask, sizeof(head->sub_state[i].mask) - 1);
		
		dbus_message_iter_get_basic(&iter_struct, &poolname);
		dbus_message_iter_next(&iter_struct); 

		strncpy(head->sub_state[i].poolname, poolname, sizeof(head->sub_state[i].poolname) - 1);
		
		dbus_message_iter_get_basic(&iter_struct, &(sub_state[i].subnet_lease_state.total_lease_num));
		dbus_message_iter_next(&iter_struct); 

		head->sub_state[i].subnet_lease_state.total_lease_num = sub_state[i].subnet_lease_state.total_lease_num;
		
		dbus_message_iter_get_basic(&iter_struct, &(sub_state[i].subnet_lease_state.active_lease_num));
		dbus_message_iter_next(&iter_struct); 

		head->sub_state[i].subnet_lease_state.active_lease_num = sub_state[i].subnet_lease_state.active_lease_num;
		
		dbus_message_iter_get_basic(&iter_struct, &(sub_state[i].subnet_lease_state.free_lease_num));
		dbus_message_iter_next(&iter_struct); 

		head->sub_state[i].subnet_lease_state.free_lease_num = sub_state[i].subnet_lease_state.free_lease_num;
		
		dbus_message_iter_get_basic(&iter_struct, &(sub_state[i].subnet_lease_state.backup_lease_num));
		dbus_message_iter_next(&iter_struct); 

		head->sub_state[i].subnet_lease_state.backup_lease_num = sub_state[i].subnet_lease_state.backup_lease_num;
		
		dbus_message_iter_get_basic(&iter_struct, &(sub_state[i].info.discover_times));
		dbus_message_iter_next(&iter_struct); 

		head->sub_state[i].info.discover_times = sub_state[i].info.discover_times;
		
		dbus_message_iter_get_basic(&iter_struct, &(sub_state[i].info.offer_times));
		dbus_message_iter_next(&iter_struct); 

		head->sub_state[i].info.offer_times = sub_state[i].info.offer_times;
		
		dbus_message_iter_get_basic(&iter_struct, &(sub_state[i].info.requested_times));
		dbus_message_iter_next(&iter_struct); 

		head->sub_state[i].info.requested_times = sub_state[i].info.requested_times;
		
		dbus_message_iter_get_basic(&iter_struct, &(sub_state[i].info.ack_times));
		dbus_message_iter_next(&iter_struct); 

		head->sub_state[i].info.ack_times = sub_state[i].info.ack_times;

		dbus_message_iter_next(&iter_array);
	} 

	dbus_message_unref(reply);

	return ret;	
}
/**wangchao add**/
int ccgi_show_dhcpv6_pool_statistics(int slot_id, struct dhcpv6_statistics_info* info)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	int ret = 1;
//	struct dhcpv6_statistics_info info;

	//int localid = 1, index = 0;
	//get_slotid_index(vty, &index, &slot_id, &localid);

	DBusConnection  *ccgi_connection = NULL;
	ccgi_ReInitDbusConnection(&ccgi_connection, slot_id, DISTRIBUTFAG);	
	

	query = dbus_message_new_method_call(DHCP6_DBUS_BUSNAME, 
									DHCP6_DBUS_OBJPATH, 
									DHCP6_DBUS_INTERFACE, 
									DHCP6_DBUS_METHOD_GET_STATISTICS_INFO);
	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (ccgi_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return 0;
	}

	dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &(info[slot_id].dhcpv6_solicit_times),
					DBUS_TYPE_UINT32, &(info[slot_id].dhcpv6_advertise_times),
					DBUS_TYPE_UINT32, &(info[slot_id].dhcpv6_request_times),
					DBUS_TYPE_UINT32, &(info[slot_id].dhcpv6_renew_times),
					DBUS_TYPE_UINT32, &(info[slot_id].dhcpv6_reply_times),
					//DBUS_TYPE_UINT32, &(info.ack_times),
					DBUS_TYPE_INVALID); 		

#if 0
	printf("===================================================\n");
	printf("total SOLICIT packet:	%d\n", info.dhcpv6_solicit_times);
	printf("total ADVERTISE packet: %d\n", info.dhcpv6_advertise_times);
	printf("total REQUESET packet:%d\n", info.dhcpv6_request_times);	
	printf("total RENEW packet:	%d\n", info.dhcpv6_renew_times);
	printf("total REPLY packet: %d\n", info.dhcpv6_reply_times);
	//vty_out(vty, "total ACK packet: 	%d\n", info.ack_times);
	printf("===================================================\n");
#endif
	dbus_message_unref(reply);
	return ret; 
}




