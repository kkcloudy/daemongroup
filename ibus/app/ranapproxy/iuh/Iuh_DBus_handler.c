#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <fcntl.h>
#include <net/if_arp.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <time.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/ioctl.h>
#include <sys/ipc.h>
#include <netinet/tcp.h> 
#include <netinet/sctp.h>  
#include <string.h>

#include "iuh/Iuh.h"
#include "Iuh_log.h"
#include "iplist.h"
#include "Iuh_Stevens.h"
#include "Iuh_DBus_handler.h"
#include "Iuh_ManageHNB.h"
#include "dbus/iuh/IuhDBusDef.h"


extern IuhMultiHomedSocket gIuhSocket;

__inline__ int IuhGetAddressSize(IuhNetworkLev4Address *addrPtr) {
	// assume address is valid
	
	switch ( ((struct sockaddr*)(addrPtr))->sa_family ) {
		
	#ifdef	IPV6
	// IPv6 is defined in Stevens' library
		case AF_INET6:
			return sizeof(struct sockaddr_in6);
			break;
	#endif
		case AF_INET:
		default:
			return sizeof(struct sockaddr_in);
	}
}


/*****************************************************
** DISCRIPTION:
**          delete hnb's socket info by hnbid 
** INPUT:
**          hnbid
** OUTPUT:
**          null
** RETURN:
**          null
*****************************************************/

void Delete_Hnb_Socket(int hnbId){
	struct IuhMultiHnbSocket *p,*p1;
	struct IuhMultiHomedInterface *q;
	
	q = gIuhSocket.interfaces;
	if(gIuhSocket.interfaces == NULL)
		return;
	while(q != NULL){
	    p1 = q->sub_sock;
	    if(p1 == NULL)
	        return;
	    if(p1->HNBID == hnbId) {
	        q->sub_sock = p1->sock_next;
	        p1->if_belong = NULL;
	        p1->sock_next = NULL;
	        close(p1->sock);
            free(p1);
            p1 = NULL;
            p1 = q->sub_sock;
	    }else {
    		p = p1->sock_next;
    		while(p != NULL){
    		    if(p->HNBID == hnbId)
    		    {
    		        p1->sock_next = p->sock_next;
        	        p->if_belong = NULL;
        	        p->sock_next = NULL;
        	        close(p->sock);
                    free(p);
                    p = NULL;
                    p = p1->sock_next;
    		    }
    		    p1 = p;
    		    p = p1->sock_next;
    		}
    	}
	}
	return;
}


/*****************************************************
** DISCRIPTION:
**          delete hnb's socket info by hnb socket and interface socket
** INPUT:
**          hnb socket, interface socket
** OUTPUT:
**          null
** RETURN:
**          null
*****************************************************/

void Delete_Hnb_Socket_by_sock(IuhSocket hnb_sock, IuhSocket sock){
	struct IuhMultiHnbSocket *p,*p1;
	struct IuhMultiHomedInterface *q;
	
	q = gIuhSocket.interfaces;
	if(gIuhSocket.interfaces == NULL)
		return;
	while(q != NULL){
	    if(q->sock == sock){
	        p1 = q->sub_sock;
	        if(p1->sock == hnb_sock) {
    	        q->sub_sock = p1->sock_next;
    	        p1->if_belong = NULL;
    	        p1->sock_next = NULL;
    	        close(p1->sock);
                free(p1);
                p1 = NULL;
                q->sub_sock_count--;
                break;
    	    }else {
        		p = p1->sock_next;
        		while(p != NULL){
        		    if(p->sock == hnb_sock){
        		        p1->sock_next = p->sock_next;
            	        p->if_belong = NULL;
            	        p->sock_next = NULL;
            	        close(p->sock);
                        free(p);
                        p = NULL;
                        p = p1->sock_next;
                        q->sub_sock_count--;
                        break;
        		    }
        		    p1 = p;
        		    p = p1->sock_next;
        		}
        	}
	    }
	    q = q->if_next;
	}
	iuh_syslog_debug_debug(IUH_DEFAULT,"Delete hnb sock end\n");
	return;
}


/*****************************************************
** DISCRIPTION:
**          set hnbid to zero by hnb socket
** INPUT:
**          hnb socket, interface socket
** OUTPUT:
**          null
** RETURN:
**          void
*****************************************************/

void IUH_BZERO_HNBID_BY_SOCK(IuhSocket hnb_sock, IuhSocket sock)
{
    struct IuhMultiHnbSocket *p,*p1;
	struct IuhMultiHomedInterface *q;
	
	q = gIuhSocket.interfaces;
	if(gIuhSocket.interfaces == NULL)
		return;
	while(q != NULL){
	    if(q->sock == sock){
	        p1 = q->sub_sock;
	        if(p1->sock == hnb_sock) {
    	        p1->HNBID = 0;
                break;
    	    }
    	    else {
        		p = p1->sock_next;
        		while(p != NULL){
        		    if(p->sock == hnb_sock){
        		        p->HNBID = 0;
                        break;
        		    }
        		    p1 = p;
        		    p = p1->sock_next;
        		}
        	}
	    }
	    q = q->if_next;
	}
    return;
}


/*****************************************************
** DISCRIPTION:
**          delete interface information by interface name
** INPUT:
**          interface name, interface index
** OUTPUT:
**          null
** RETURN:
**          void
*****************************************************/

void Delete_Interface_ByName(char *ifname, int ifindex){
	struct IuhMultiHomedInterface *p,*p1;
	struct IuhMultiHnbSocket *q;
	if(gIuhSocket.interfaces == NULL)
		return;
	p1 = gIuhSocket.interfaces;
	if((strncmp(p1->ifname,ifname,IF_NAME_LEN)==0)&&(p1->systemIndex == ifindex)){
        q = p1->sub_sock;
        while(q != NULL){
            p1->sub_sock = q->sock_next;
            q->sock_next = NULL;
            q->if_belong = NULL;
            close(q->sock);
            free(q);
            q = NULL;
            q = p1->sub_sock;
            p1->sub_sock_count--;
        }
	    p = p1;
	    gIuhSocket.interfaces = p->if_next;   //NULL
	    p->if_next = NULL;
	    p->sub_sock = NULL;
	    close(p->sock);
		free(p);
		p = NULL;
		gIuhSocket.count--;
		return;
	}
	p = p1->if_next;
	while(p != NULL){
		if((strncmp(p->ifname,ifname,IF_NAME_LEN)==0)&&(p->systemIndex == ifindex)){
            q = p->sub_sock;
            while(q != NULL){
                p->sub_sock = q->sock_next;
                q->sock_next = NULL;
                q->if_belong = NULL;
                close(q->sock);
                free(q);
                q = NULL;
                q = p->sub_sock;
                p->sub_sock_count--;
            }
			p1->if_next = p->if_next;
			p->if_next = NULL;
			p->sub_sock = NULL;
			
			close(p->sock);
			free(p);
			p = NULL;
			p = p1->if_next;
			gIuhSocket.count--;
			break;
		}
		p1 = p;
		p = p1->if_next;
	}
	return;
}


/*****************************************************
** DISCRIPTION:
**          get interface info by interface name
** INPUT:
**          interface name
** OUTPUT:
**          interface information
** RETURN:
**          0       success  
**          -1      fail
*****************************************************/

int Get_Interface_Info(char * ifname, struct ifi_info *ifi){
	int sockfd;
	struct ifreq	ifr;
	struct sockaddr_in	*sinptr;
	struct sockaddr_in6	*sin6ptr;
	sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_SCTP);
	strncpy(ifr.ifr_name,ifname, sizeof(ifr.ifr_name));	
	//iuh_syslog_debug_debug(IUH_DEFAULT,"@@@begin ioctl\n");
	if (ioctl(sockfd, SIOCGIFINDEX, &ifr) == -1){//bind to a interface 
		iuh_syslog_debug_debug(IUH_SYSLOG_DEFAULT,"hnb quit reason is SIOCGIFINDEX error");
		close(sockfd);
		return -1;
	}
	ifi->ifi_index = ifr.ifr_ifindex;
	//iuh_syslog_debug_debug(IUH_DEFAULT,"@@@ifi_index = %d\n",ifi->ifi_index);
	if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) == -1){
		iuh_syslog_debug_debug(IUH_SYSLOG_DEFAULT,"hnb quit reason is SIOCGIFFLAGS error");
		close(sockfd);
		return -1;
	}
	if(ifr.ifr_flags & IFF_UP){
		ifi->ifi_flags = ifr.ifr_flags;
		//iuh_syslog_debug_debug(IUH_DEFAULT,"@@@ifi_flags = %d\n",ifi->ifi_flags);
	}else{
		iuh_syslog_debug_debug(IUH_SYSLOG_DEFAULT,"hnb quit reason is IF_DOWN error");
	}
	if(ioctl(sockfd, SIOCGIFADDR, &ifr) == -1){
		//iuh_syslog_debug_debug(IUH_DEFAULT,"hnb quit reason is SIOCGIFADDR error");
		close(sockfd);
		return -1;
	}
	iuh_syslog_debug_debug(IUH_DEFAULT,"addr %s",inet_ntoa(((struct sockaddr_in*)(&ifr.ifr_addr))->sin_addr));
	ifi->addr_num = ipaddr_list(ifi->ifi_index, ifi->addr, IFI_ADDR_NUM);
	switch (ifr.ifr_addr.sa_family) {
		case AF_INET:
			sinptr = (struct sockaddr_in *) &ifr.ifr_addr;
			ifi->ifi_addr = (struct sockaddr*)calloc(1, sizeof(struct sockaddr_in));
			memcpy(ifi->ifi_addr, sinptr, sizeof(struct sockaddr_in));
			break;
		case AF_INET6:
			sin6ptr = (struct sockaddr_in6 *) &ifr.ifr_addr;
			ifi->ifi_addr = (struct sockaddr*)calloc(1, sizeof(struct sockaddr_in6));
			memcpy(ifi->ifi_addr, sin6ptr, sizeof(struct sockaddr_in6));
			break;
		default:
			break;
	}
	//iuh_syslog_debug_debug(IUH_DEFAULT,"@@@close socket\n");
	close(sockfd);
	return 0;
}


/*****************************************************
** DISCRIPTION:
**          check whether interface exists by interface name
** INPUT:
**          interface name
** OUTPUT:
**          null
** RETURN:
**          0   not exist
**          1   exist
*****************************************************/

int Check_Interface(char *ifname)
{
    if(gIuhSocket.interfaces == NULL)
        return 0;
    struct IuhMultiHomedInterface *p = gIuhSocket.interfaces;
    while(p != NULL){
        if(strcmp(p->ifname,ifname) == 0)
            return 1;
        p = p->if_next;
    }
    return 0;
}


/*****************************************************
** DISCRIPTION:
**          check and bind interface 
**          set by cmd "set auto_hnb interface add IFNAME"
** INPUT:
**          interface name
**          policy :  1  add,  0   delete
** OUTPUT:
**          0       succeed
**          1       fail
** RETURN:
**          
*****************************************************/

int Check_And_Bind_Interface_For_HNB(char * ifname, unsigned char policy){
	int ret = 0;
	int i = 0;
	if((policy == 1) && (Check_Interface(ifname) == 1))
	    return 0;
	struct ifi_info *ifi_tmp = (struct ifi_info*)calloc(1, sizeof(struct ifi_info));
	memset(ifi_tmp->ifi_name,0,sizeof(ifi_tmp->ifi_name));
	strncpy(ifi_tmp->ifi_name,ifname,sizeof(ifi_tmp->ifi_name));
		
	ret = Get_Interface_Info(ifname,ifi_tmp); 
	if(ret != 0){		
		Delete_Interface_ByName(ifname, ifi_tmp->ifi_index);
		iuh_syslog_debug_debug(IUH_DEFAULT,"Call Delete_Interface_ByName111111111111\n");
		if(ifi_tmp->ifi_addr != NULL){
			free(ifi_tmp->ifi_addr);
			ifi_tmp->ifi_addr = NULL;
		}
		if(ifi_tmp->ifi_brdaddr != NULL){
			free(ifi_tmp->ifi_brdaddr);
			ifi_tmp->ifi_brdaddr = NULL;
		}
		free(ifi_tmp);
		ifi_tmp = NULL;
		return ret;		
	}	
	if(policy == 1){
    	for(i=0;i<ifi_tmp->addr_num;i++)
    	{		
    		if(ifi_tmp->addr[i] == 0)
    			continue;
    		memcpy(&(((struct sockaddr_in *) ifi_tmp->ifi_addr)->sin_addr).s_addr,&(ifi_tmp->addr[i]),sizeof(struct in_addr));
    		((struct sockaddr_in *) ifi_tmp->ifi_addr)->sin_addr.s_addr = ifi_tmp->addr[i];
    		ret = Bind_Interface_For_HNB(ifi_tmp,Iuh_SCTP_PORT);		//°ó¶Ë¿Ú
    	}
    }
    else if(policy == 0){
        iuh_syslog_debug_debug(IUH_DEFAULT,"Call Delete_Interface_ByName222222222222\n");
        Delete_Interface_ByName(ifname, ifi_tmp->ifi_index);
    }
    
	if(ifi_tmp->ifi_addr != NULL){
		free(ifi_tmp->ifi_addr);
		ifi_tmp->ifi_addr = NULL;
	}		
	if(ifi_tmp->ifi_brdaddr != NULL){
		free(ifi_tmp->ifi_brdaddr);
		ifi_tmp->ifi_brdaddr = NULL;
	}
	free(ifi_tmp);
	ifi_tmp = NULL;
	return ret;
}


/*****************************************************
** DISCRIPTION:
**          bind interface for hnb with sctp port
** INPUT:
**          interface info
**          sctp port
** OUTPUT:
**          null
** RETURN:
**          0       succeed
**          -1      fail
*****************************************************/

int Bind_Interface_For_HNB(struct ifi_info *ifi, int port){
	int yes = 1;
	IuhSocket sock;
	struct sctp_event_subscribe events;
	struct IuhMultiHomedInterface *p;
	if((sock = socket(ifi->ifi_addr->sa_family, SOCK_STREAM, IPPROTO_SCTP)) < 0) {
		
	}
	
	bzero(&events, sizeof (events));
    events.sctp_data_io_event = 1;
    events.sctp_association_event = 1;
    events.sctp_address_event = 1;
    events.sctp_send_failure_event = 1;
    events.sctp_peer_error_event = 1;
    events.sctp_shutdown_event = 1;

    setsockopt(sock, IPPROTO_SCTP, SCTP_EVENTS, &events, sizeof(events));
    // reuse address
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
								             
	
	// bind address
	sock_set_port_cw(ifi->ifi_addr, htons(port));//zhanglei change
	if(bind(sock, (struct sockaddr*) ifi->ifi_addr, IuhGetAddressSize((IuhNetworkLev4Address*)ifi->ifi_addr)) < 0) {
		close(sock);
		
	}
	/* zhangshu add for listen */
    if(listen(sock, 5) == -1)
    {
        close(sock);

    }
    //iuh_syslog_debug_debug(IUH_DEFAULT,"@@@interface index = %d\n",ifi->ifi_index);
	// store socket inside multihomed socket
	
	IUH_CREATE_OBJECT_ERR(p, struct IuhMultiHomedInterface, { iuh_syslog_crit("Out Of Memory"); return -1; });
	memset(p->ifname, 0, IFI_NAME);
	strncpy(p->ifname, ifi->ifi_name, IFI_NAME);
	iuh_syslog_debug_debug(IUH_DEFAULT,"@@@$$$ p->ifname = %s\n",p->ifname);
	p->sock = sock;	
	p->sock_type = 1;
	p->systemIndex = ifi->ifi_index;
	p->systemIndexbinding = ifi->ifi_index_binding;
	p->addrIPv4.ss_family = AF_UNSPEC;
	p->sub_sock_count = 0;
	p->sub_sock = NULL;
	iuh_syslog_debug_debug(IUH_DEFAULT,"p->sock = %d\n",p->sock);
	iuh_syslog_debug_debug(IUH_DEFAULT,"p->systemIndex = %d\n",p->systemIndex);
	
	struct IuhMultiHomedInterface* inf = gIuhSocket.interfaces;
	if(gIuhSocket.interfaces == NULL) {
	    //iuh_syslog_debug_debug(IUH_DEFAULT,"&&&&&&&&&&&&&&&&&&&&&&\n");
		gIuhSocket.interfaces = p;
		p->if_next = NULL; 
		
	}
	else{
		while(inf->if_next != NULL)
			inf = inf->if_next;
		//iuh_syslog_debug_debug(IUH_DEFAULT,"*******************\n");
		inf->if_next = p;			
		p->if_next = NULL;
	}
	gIuhSocket.count++; // we add a socket to the multihomed socket
	
	iuh_syslog_debug_debug(IUH_DEFAULT,"@@@$$$ gIuhSocket.count = %d\n",gIuhSocket.count);
	return 0;
}


/*****************************************************
** DISCRIPTION:
**          add hnb socket into gACSocket
** INPUT:
**          interface socket
**          hnb socket
**          address
** OUTPUT:
**          null
** RETURN:
**          0       succeed
**          -1      fail
*****************************************************/

int Add_Hnb_Sock(IuhSocket ifsock, IuhSocket hnbsock, struct sockaddr *addrPtr)
{   
    struct IuhMultiHnbSocket *p;
    IUH_CREATE_OBJECT_ERR(p, struct IuhMultiHnbSocket,{ iuh_syslog_crit("Out Of Memory"); return -1; });
    p->sock = hnbsock;	
    p->sock_type = 2;
	Iuh_COPY_NET_ADDR_PTR(&(p->addr), ((IuhNetworkLev4Address*)&(p->addr)) );
	struct IuhMultiHomedInterface* inf = gIuhSocket.interfaces;
	struct IuhMultiHnbSocket* sub_inf;
	if(gIuhSocket.interfaces == NULL) {
	    return -1;
	}	
	else{
		while(inf->if_next != NULL){
		    if(inf->sock == ifsock)
		    {
		        sub_inf = inf->sub_sock;
		        if(sub_inf == NULL){
		            sub_inf->sock_next = p;			
            		p->sock_next = NULL;
            		p->if_belong = inf;
            		inf->sub_sock_count++;
            		break;
		        }
		            
		        while(sub_inf->sock_next != NULL)
        			sub_inf = sub_inf->sock_next;
        		sub_inf->sock_next = p;			
        		p->sock_next = NULL;
        		p->if_belong = inf;
        		inf->sub_sock_count++;
        		break;
		    }
			inf = inf->if_next;
        }
	}
	return 0;
}


/*****************************************************
** DISCRIPTION:
**          bind hnb with interface
** INPUT:
**          hnbid,      interface name
** OUTPUT:
**          null
** RETURN:
**          0       succeed
**          other   fail
*****************************************************/

int Iuh_BINDING_IF_APPLY_HNB(unsigned int HNBID, char * ifname)
{
	if(IUH_HNB[HNBID]->isused == 1)
	{
	//	printf("*** error this WTP is used and active, you can not binding interface ***\n");
		iuh_syslog_debug_debug(IUH_DEFAULT,"*** error this WTP is used and active, you can not binding interface ***\n");
		return IUH_BE_USING;
	}
	int sockfd = -1;
	int isystemindex = -1;
	//int sockdes = -1;
	//CWBool bretflag = CW_FALSE;
	struct ifreq	ifr;
	int ret = Check_And_Bind_Interface_For_HNB(ifname, 1);
	if(ret != 0)
		return ret;

	if((sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_SCTP)) < 0)
	{
		iuh_syslog_debug_debug(IUH_DEFAULT,"*** can't create socket connection ***\n");
		return INTERFACE_NOT_EXIST;
	}
	
	//we should check ifname error
	strncpy(ifr.ifr_name,ifname, sizeof(ifr.ifr_name));
	
	if (ioctl(sockfd, SIOCGIFINDEX, &ifr) == -1){//Retrieve  the interface index  
		iuh_syslog_debug_debug(IUH_DEFAULT,"*** can't retrieve the interface index ***\n");
		close(sockfd);
		return INTERFACE_NOT_EXIST;
	}
	close(sockfd);
	isystemindex = ifr.ifr_ifindex;
	iuh_syslog_debug_debug(IUH_DEFAULT,"*** binding iterface name isystemindex is %d ***\n",isystemindex);
	
	if(IUH_HNB[HNBID] != NULL)
	{
		IUH_HNB[HNBID]->BindingSystemIndex= isystemindex;
		memset(IUH_HNB[HNBID]->BindingIFName, 0, IF_NAME_LEN);
		memcpy(IUH_HNB[HNBID]->BindingIFName,ifname, strnlen(ifname,IF_NAME_LEN));
		
		iuh_syslog_debug_debug(IUH_DEFAULT,"*** binding iterface name to wtp success ***\n");
		return 0;
	}
	else
	{
		iuh_syslog_debug_debug(IUH_DEFAULT,"*** can't binding iterface name, please make sure you have create wtp ***\n");
		return IUH_ID_NOT_EXIST;
	}
		
}


/*****************************************************
** DISCRIPTION:
**          add iuh interface into interface list
** INPUT:
**          interface name
** OUTPUT:
**          null
** RETURN:
**          0       succeed
**          other   fail
*****************************************************/

int iuh_auto_hnb_login_insert_iflist(char *ifname)
{
	//get if index
	unsigned int ifindex = 0;
	int sockfd;
	struct ifreq	ifr;
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	strncpy(ifr.ifr_name,ifname, sizeof(ifr.ifr_name));			
	if (ioctl(sockfd, SIOCGIFINDEX, &ifr) == -1)
	{
//		printf("SIOCGIFINDEX error\n");
		iuh_syslog_debug_debug(IUH_DEFAULT,"SIOCGIFINDEX error\n");
		close(sockfd);
		return APPLY_IF_FAIL;
	}
	//iuh_syslog_debug_debug(IUH_DEFAULT,"ifindex %d\n",ifr.ifr_ifindex);
	
	ifindex = ifr.ifr_ifindex;
	close(sockfd);
	//insert to the list
	iuh_auto_hnb_if *wif = NULL;
	iuh_auto_hnb_if *wifnext = NULL;
	
	wif = (iuh_auto_hnb_if *)malloc(sizeof(iuh_auto_hnb_if));
	wif->ifindex = ifindex;

	memset(wif->ifname,0,IF_NAME_LEN);
	memcpy(wif->ifname,ifname,strnlen(ifname,IF_NAME_LEN));
	wif->ifnext = NULL;
	
	if(g_auto_hnb_login.auto_hnb_if == NULL)
	{
		g_auto_hnb_login.auto_hnb_if = wif ;
		g_auto_hnb_login.auto_hnb_if->ifnext = NULL;
		g_auto_hnb_login.ifnum = 1;
	}
	else
	{
		wifnext = g_auto_hnb_login.auto_hnb_if;
		while(wifnext != NULL)
		{	
			if(strncmp((char*)wifnext->ifname,ifname,strnlen(ifname,IF_NAME_LEN)) == 0)
			{
				//printf("already in the list\n");
				free(wif);
				wif = NULL;
				return 0;
			}
			wifnext = wifnext->ifnext;
		}
		
		wifnext = g_auto_hnb_login.auto_hnb_if;
		while(wifnext->ifnext != NULL)
		{	
			wifnext = wifnext->ifnext;
		}
		
		wifnext->ifnext = wif;
		g_auto_hnb_login.ifnum++;
		iuh_syslog_debug_debug(IUH_DEFAULT,"g_auto_hnb_login.ifnum = %d\n",g_auto_hnb_login.ifnum);
	}
	//iuh_syslog_debug_debug(IUH_DEFAULT,"999999999999999999\n");
	return 0;
}


/*****************************************************
** DISCRIPTION:
**          delete interface info from interface list
** INPUT:
**          interface name
** OUTPUT:
**          null
** RETURN:
**          0       succeed
**          other   fail
*****************************************************/

int iuh_auto_hnb_login_remove_iflist(char *ifname)
{
	//iuh_syslog_debug_debug(IUH_DEFAULT,"wid_auto_ap_login_remove_iflist ifname %s\n",ifname);

	iuh_auto_hnb_if *wif = NULL;
	iuh_auto_hnb_if *wifnext = NULL;

	wifnext = g_auto_hnb_login.auto_hnb_if;
	
	if(g_auto_hnb_login.auto_hnb_if != NULL)
	{
		if(strncmp((char*)wifnext->ifname,ifname,strnlen(ifname,IF_NAME_LEN)) == 0)
		{
			g_auto_hnb_login.auto_hnb_if = wifnext->ifnext;
			g_auto_hnb_login.ifnum--;
			free(wifnext);
			wifnext = NULL;		
			//printf("delete ifname %s from list\n",ifname);
		}
		else
		{
			while(wifnext->ifnext != NULL)
			{	
				if(strncmp((char*)wifnext->ifnext->ifname,ifname,strnlen(ifname,IF_NAME_LEN)) == 0)
				{
					wif = wifnext->ifnext;
					wifnext->ifnext = wifnext->ifnext->ifnext;
					g_auto_hnb_login.ifnum--;
					iuh_syslog_debug_debug(IUH_DEFAULT,"g_auto_hnb_login.ifnum = %d\n",g_auto_hnb_login.ifnum);
					free(wif);
					wif = NULL;				
					//printf("delete ifname %s from list\n",ifname);
					return 0;
				}
				wifnext = wifnext->ifnext;
			}
		}
	}
	//iuh_syslog_debug_debug(IUH_DEFAULT,"888888888888888888\n");
	return 0;
	
}


/*****************************************************
** DISCRIPTION:
**          create new hnb info table
** INPUT:
**          hnb name
**          hnbid
**          hnb register message struct
**          hnb socket
** OUTPUT:
**          null
** RETURN:
**          0           succeed
**          other       fail
*****************************************************/

int IUH_CREATE_NEW_HNB(unsigned char *HNBNAME, int HNBID, HnbRegisterRequestValue *requestValue, int socket)
{
    if(gStaticHNBs > HNB_DEFAULT_NUM_AUTELAN){
		return HNB_OVER_MAX_NUM;
	}

	if(!check_hnbid_func(HNBID)){
		iuh_syslog_err("hnbid is larger than max hnb number\n");
		return HNB_ID_INVALID;
	}
	if(requestValue == NULL){
	    iuh_syslog_err("request information is invalid\n");
	    return HNB_REGIST_INFO_INVALID;
	}
	iuh_syslog_debug_debug(IUH_DEFAULT,"CREATE NEW HNB %d:\n",HNBID);
	//iuh_syslog_debug_debug(IUH_DEFAULT,"socket = %d\n",socket);
	struct IuhMultiHnbSocket *sock_info = IUH_FIND_HNB_SOCK(socket);
	if(sock_info == NULL){
	    iuh_syslog_err("socket is not exist\n");
	    return HNB_ID_INVALID;
	}
	sock_info->HNBID = HNBID;
    //iuh_syslog_debug_debug(IUH_DEFAULT,"aaa\n");
	IUH_HNB[HNBID] = (Iuh_HNB*)malloc(sizeof(Iuh_HNB));
	memset(IUH_HNB[HNBID], 0, sizeof(Iuh_HNB));	
	memcpy(IUH_HNB[HNBID]->HNBNAME,HNBNAME,strnlen(HNBNAME, HNB_NAME_LEN));	
    iuh_syslog_debug_debug(IUH_DEFAULT,"---HNBNAME = %s\n",(char*)IUH_HNB[HNBID]->HNBNAME);
    IUH_HNB[HNBID]->socket = sock_info->sock;
    iuh_syslog_debug_debug(IUH_DEFAULT,"---sock = %d\n",IUH_HNB[HNBID]->socket);
    IUH_HNB[HNBID]->address = sock_info->addr;
    IUH_HNB[HNBID]->HNBID = HNBID;
    memcpy(IUH_HNB[HNBID]->BindingIFName,sock_info->if_belong->ifname,\
        strnlen(sock_info->if_belong->ifname, IF_NAME_LEN));
    iuh_syslog_debug_debug(IUH_DEFAULT,"---BindingIFName = %s\n",IUH_HNB[HNBID]->BindingIFName);
    IUH_HNB[HNBID]->BindingSystemIndex = sock_info->if_belong->systemIndex;
    iuh_syslog_debug_debug(IUH_DEFAULT,"---BindingSystemIndex = %d\n",IUH_HNB[HNBID]->BindingSystemIndex);

    char *ipaddr = inet_ntoa(((struct sockaddr_in*)&sock_info->addr)->sin_addr);
    if(ipaddr != NULL){
        memcpy(IUH_HNB[HNBID]->HNBIP, ipaddr, strnlen(ipaddr,HNB_IP_LEN));
    }
    else{
        strncpy(IUH_HNB[HNBID]->HNBIP, "---* .* .* .*", HNB_IP_LEN);
    }
    iuh_syslog_debug_debug(IUH_DEFAULT,"---HNBIP = %s\n",(char*)IUH_HNB[HNBID]->HNBIP);
    
    memcpy(IUH_HNB[HNBID]->HnbIdentity, requestValue->HnbIdentity, \
	    strnlen(requestValue->HnbIdentity, HNB_IDENTITY_LEN));
	iuh_syslog_debug_debug(IUH_DEFAULT,"---HnbIdentity = %s\n",(char*)IUH_HNB[HNBID]->HnbIdentity);
    
    IUH_HNB[HNBID]->HnbLocationInfo.macroCoverageInfo = NULL;
    IUH_HNB[HNBID]->HnbLocationInfo.gographicalLocation = NULL;
    IUH_HNB[HNBID]->HnbLocationInfo.ipv4info = NULL;


    /* copy value of Location information */
	if(requestValue->HnbLocationInfo.macroCoverageInfo != NULL){
	    IUH_HNB[HNBID]->HnbLocationInfo.macroCoverageInfo = \
	        (struct  macroCellID*)malloc(sizeof(struct  macroCellID));
	    memset(IUH_HNB[HNBID]->HnbLocationInfo.macroCoverageInfo,0,sizeof(struct  macroCellID));
	    IUH_HNB[HNBID]->HnbLocationInfo.macroCoverageInfo->present = \
	        requestValue->HnbLocationInfo.macroCoverageInfo->present;
	    IUH_HNB[HNBID]->HnbLocationInfo.macroCoverageInfo->choice = \
	        requestValue->HnbLocationInfo.macroCoverageInfo->choice;
	}
	//iuh_syslog_debug_debug(IUH_DEFAULT,"111a\n");
	if(requestValue->HnbLocationInfo.gographicalLocation != NULL){
	    IUH_HNB[HNBID]->HnbLocationInfo.gographicalLocation = \
	        (struct  geographicalLocation*)malloc(sizeof(struct  geographicalLocation));
	    memset(IUH_HNB[HNBID]->HnbLocationInfo.gographicalLocation,0,sizeof(struct  geographicalLocation));
	    IUH_HNB[HNBID]->HnbLocationInfo.gographicalLocation->GeographicalCoordinates = \
	        requestValue->HnbLocationInfo.gographicalLocation->GeographicalCoordinates;
	    IUH_HNB[HNBID]->HnbLocationInfo.gographicalLocation->AltitudeAndDirection = \
	        requestValue->HnbLocationInfo.gographicalLocation->AltitudeAndDirection;
	}
	//iuh_syslog_debug_debug(IUH_DEFAULT,"111b\n");
	if(requestValue->HnbLocationInfo.ipv4info != NULL){
	    IUH_HNB[HNBID]->HnbLocationInfo.ipv4info = \
	        (struct  IPAddress*)malloc(sizeof(struct  IPAddress));
	    memset(IUH_HNB[HNBID]->HnbLocationInfo.ipv4info,0,sizeof(struct  IPAddress));
	    IUH_HNB[HNBID]->HnbLocationInfo.ipv4info->present = \
	        requestValue->HnbLocationInfo.ipv4info->present;
	    IUH_HNB[HNBID]->HnbLocationInfo.ipv4info->choice = \
	        requestValue->HnbLocationInfo.ipv4info->choice;
	}
	//iuh_syslog_debug_debug(IUH_DEFAULT,"111c\n");
	//IUH_HNB[HNBID]->plmnId = requestValue->plmnId;
	IUH_HNB[HNBID]->acl_switch = requestValue->acl_switch;   //book add, 2012-1-4
	memcpy(IUH_HNB[HNBID]->plmnId, requestValue->plmnId, PLMN_LEN);
	memcpy(IUH_HNB[HNBID]->cellId, requestValue->cellId, CELLID_LEN);
	memcpy(IUH_HNB[HNBID]->lac, requestValue->lac, LAC_LEN);
	memcpy(IUH_HNB[HNBID]->rac, requestValue->rac, RAC_LEN);
	memcpy(IUH_HNB[HNBID]->sac, requestValue->sac, SAC_LEN);

	iuh_hnb_show_str("---plmnid = ", IUH_HNB[HNBID]->plmnId, 1, PLMN_LEN);
	iuh_hnb_show_str("---cellId = ", IUH_HNB[HNBID]->cellId, 0, CELLID_LEN);
	iuh_hnb_show_str("---lac = ", IUH_HNB[HNBID]->lac, 0, LAC_LEN);
	iuh_hnb_show_str("---rac = ", IUH_HNB[HNBID]->rac, 0, RAC_LEN);
	iuh_hnb_show_str("---sac = ", IUH_HNB[HNBID]->sac, 0, SAC_LEN);
    
    
    IUH_HNB[HNBID]->HNB_UE = NULL;
    IUH_HNB[HNBID]->state = REGISTED;
    IUH_HNB[HNBID]->current_UE_number = 0;
    IUH_HNB[HNBID]->accessmode = ACCESS_OPEN;
    
    IUH_HNB[HNBID]->rncId = gRncId;
    IUH_HNB[HNBID]->muxPortNum = 0; //it's wrong, must modify later
    //iuh_syslog_debug_debug(IUH_DEFAULT,"333\n");
    //IUH_HNB[HNBID]->HNBSN = NULL;
    //IUH_HNB[HNBID]->HNBModel = NULL;
    
	IUH_HNB[HNBID]->imsi_white_list = NULL;
	IUH_HNB[HNBID]->IMSI_WL_Switch = 1;	//for test
    gStaticHNBs++;
	//iuh_syslog_debug_debug(IUH_DEFAULT,"444\n");
    return 0;
}


/*****************************************************
** DISCRIPTION:
**          update hnb information
** INPUT:
**          hnbid
**          hnb register request message
** OUTPUT:
**          null
** RETURN:
**          0           succeed
**          other       fail
*****************************************************/

int IUH_UPDATE_HNB(int HNBID, HnbRegisterRequestValue *requestValue)
{
    char name[HNB_NAME_LEN] = {0};
    memcpy(name, IUH_HNB[HNBID]->HNBNAME, strnlen(IUH_HNB[HNBID]->HNBNAME, HNB_NAME_LEN));

    int socket = IUH_HNB[HNBID]->socket;
    
    IUH_DELETE_HNB(HNBID);
    IUH_CREATE_NEW_HNB(name, HNBID, requestValue, socket);

    return 0;
}


/*****************************************************
** DISCRIPTION:
**          delete hnb information from hnblist
** INPUT:
**          hnbid
** OUTPUT:
**          null
** RETURN:
**          0
*****************************************************/

int IUH_DELETE_HNB(int HNBID)
{
    iuh_syslog_debug_debug(IUH_DEFAULT,"### IUH_DELETE_HNB ###");
    iuh_syslog_debug_debug(IUH_DEFAULT,"HNBID = %d\n",HNBID);
    if(IUH_HNB[HNBID] == NULL) return 0;
    
	if(IUH_HNB[HNBID]->current_UE_number != 0){
	    Iuh_HNB_UE *tmpUE = NULL;
	    tmpUE = IUH_HNB[HNBID]->HNB_UE;
	    while((tmpUE = IUH_HNB[HNBID]->HNB_UE) != NULL){
	        IUH_DELETE_UE(tmpUE->UEID);
	    }
	    //return HNB_ID_UESD_BY_UE;
	}

	IUH_HNB[HNBID]->HNB_UE = NULL;
	
	IUH_FREE_OBJECT(IUH_HNB[HNBID]->HnbLocationInfo.gographicalLocation);
	IUH_FREE_OBJECT(IUH_HNB[HNBID]->HnbLocationInfo.macroCoverageInfo);
	IUH_FREE_OBJECT(IUH_HNB[HNBID]->HnbLocationInfo.ipv4info);

	IUH_FREE_OBJECT(IUH_HNB[HNBID]);
	gStaticHNBs--;

	iuh_syslog_debug_debug(IUH_DEFAULT,"gStaticHNBs = %d\n",gStaticHNBs);
    return 0;
}


/*****************************************************
** DISCRIPTION:
**          find hnb socket by hnbid
** INPUT:
**          hnbid
** OUTPUT:
**          null
** RETURN:
**          hnb socket struct
*****************************************************/

struct IuhMultiHnbSocket * IUH_FIND_HNB_ID(int HNBID)
{
    struct IuhMultiHomedInterface *inf = gIuhSocket.interfaces;
    struct IuhMultiHnbSocket *sock_info;
    struct IuhMultiHnbSocket *hnbinf;
    while(inf != NULL){
        hnbinf = inf->sub_sock;
        while(hnbinf != NULL){
            if(hnbinf->HNBID == HNBID){
                sock_info = hnbinf;
                iuh_syslog_debug_debug(IUH_DEFAULT,"find sock\n");
                return sock_info;
            }
            else{
                hnbinf = hnbinf->sock_next;
            }
        }
        inf = inf->if_next;
    }

    return NULL;
}


/*****************************************************
** DISCRIPTION:
**          find hnb socket struct by socket
** INPUT:
**          socket
** OUTPUT:
**          null
** RETURN:
**          hnb socket struct
*****************************************************/

struct IuhMultiHnbSocket * IUH_FIND_HNB_SOCK(int socket)
{
    struct IuhMultiHomedInterface *inf = gIuhSocket.interfaces;
    struct IuhMultiHnbSocket *sock_info;
    struct IuhMultiHnbSocket *hnbinf;
    while(inf != NULL){
        hnbinf = inf->sub_sock;
        while(hnbinf != NULL){
            if(hnbinf->sock == socket){
                sock_info = hnbinf;
                iuh_syslog_debug_debug(IUH_DEFAULT,"find sock\n");
                return sock_info;
            }
            else{
                hnbinf = hnbinf->sock_next;
            }
        }
        inf = inf->if_next;
    }

    return NULL;
}


/*****************************************************
** DISCRIPTION:
**          free hnb register request struct
** INPUT:
**          the struct name to free
** OUTPUT:
**          null
** RETURN:
**          void
*****************************************************/

void IUH_FREE_HnbRegisterRequestValue(HnbRegisterRequestValue *requestValue)
{
    IUH_FREE_OBJECT(requestValue->HnbLocationInfo.gographicalLocation);
    IUH_FREE_OBJECT(requestValue->HnbLocationInfo.macroCoverageInfo);
    IUH_FREE_OBJECT(requestValue->HnbLocationInfo.ipv4info);
    iuh_syslog_debug_debug(IUH_DEFAULT,"free hnb register request over\n");
    
    return;
}


/*****************************************************
** DISCRIPTION:
**          check ue id
** INPUT:
**          ueid
** OUTPUT:
**          null
** RETURN:
**          true
**          false
*****************************************************/

IuhBool check_ueid_func(unsigned int UEID){
	if((UEID > UE_MAX_NUM_AUTELAN) || (UEID == 0)){
		iuh_syslog_err("<error> invalid UEid:%d\n",UEID);
		return Iuh_FALSE;
	}else{
		return Iuh_TRUE;
	}
}


/*****************************************************
** DISCRIPTION:
**          create new ue
** INPUT:
**          hnbid,  ueid,  ue register request message
** OUTPUT:
**          null
** RETURN:
**          0           succeed
**          other       fail
*****************************************************/

int IUH_CREATE_UE(int HNBID, int UEID, UERegisterRequestValue *UERequestValue)
{
    if(gStaticUEs > UE_MAX_NUM_AUTELAN){
		return UE_OVER_MAX_NUM;
	}
	if(IUH_HNB[HNBID] == NULL){
	    return HNB_ID_INVALID;
	}
    if(IUH_HNB[HNBID]->current_UE_number >= HNB_MAX_UE_NUM){
        return UE_OVER_HNB_MAX_NUM;
    }
	if(!check_ueid_func(UEID)){
		iuh_syslog_err("hnbid is larger than max hnb number\n");
		return UE_ID_INVALID;
	}
	
	IUH_FREE_OBJECT(IUH_HNB_UE[UEID]); //book add, 2012-1-18

	IUH_HNB_UE[UEID] = (Iuh_HNB_UE *)malloc(sizeof(Iuh_HNB_UE));
	memset(IUH_HNB_UE[UEID], 0, sizeof(Iuh_HNB_UE));

	memset(IUH_HNB_UE[UEID]->IMSI,0,IMSI_LEN);
	memcpy(IUH_HNB_UE[UEID]->IMSI,UERequestValue->IMSI,IMSI_LEN);

    IUH_HNB_UE[UEID]->UEID = UEID;
    IUH_HNB_UE[UEID]->UE_Identity = UERequestValue->UE_Identity;
    IUH_HNB_UE[UEID]->Capabilities = UERequestValue->Capabilities;
    IUH_HNB_UE[UEID]->registrationCause = UERequestValue->registrationCause;
    IUH_HNB_UE[UEID]->HNBID = HNBID;
    IUH_HNB_UE[UEID]->state = REGISTED;
    IUH_HNB_UE[UEID]->context_id = UEID*4;
    memcpy(IUH_HNB_UE[UEID]->context_id_str, ((char*)&(IUH_HNB_UE[UEID]->context_id)+1), CONTEXTID_LEN);//int is bigend  , char is small end
    memset(IUH_HNB_UE[UEID]->iuCSSigConId, 0xff, IU_SIG_CONN_ID_LEN);
	memset(IUH_HNB_UE[UEID]->iuPSSigConId, 0xff, IU_SIG_CONN_ID_LEN);
	memset(IUH_HNB_UE[UEID]->iuDefSigConId, 0xff, IU_SIG_CONN_ID_LEN);

    // insert ue in the head of hnb_ue_list
    IUH_HNB_UE[UEID]->ue_next = IUH_HNB[HNBID]->HNB_UE;
	IUH_HNB_UE[UEID]->UE_RAB = NULL;
    IUH_HNB[HNBID]->HNB_UE = IUH_HNB_UE[UEID];
    IUH_HNB[HNBID]->current_UE_number++;

    gStaticUEs++;

	/* book add, 2011-12-21 */
	msgq qData;
	qData.mqid = (IUH_HNB_UE[UEID]->HNBID)%THREAD_NUM+1;
	qData.mqinfo.type = CONTROL_DATA;
	qData.mqinfo.HNBID = HNBID;
	qData.mqinfo.u.ueInfo.op = UE_ADD;
	qData.mqinfo.u.ueInfo.ueInfo.UEID = UEID;
	memcpy(qData.mqinfo.u.ueInfo.ueInfo.ContextID, IUH_HNB_UE[UEID]->context_id_str, CONTEXTID_LEN);
	memcpy(qData.mqinfo.u.ueInfo.ueInfo.IMSI, IUH_HNB_UE[UEID]->IMSI, IMSI_LEN);
	
	iuh_syslog_debug_debug(IUH_DEFAULT,"send msg IuhMsgQid = %d\n",IuhMsgQid);
	if (msgsnd(IuhMsgQid, (msgq *)&qData, sizeof(qData.mqinfo), 0) == -1)
		perror("msgsnd");
	
    return 0;
}



/*****************************************************
** DISCRIPTION:
**          free pdp info list
** INPUT:
**          pdp info list
** OUTPUT:
**          null
** RETURN:
**          void
*****************************************************/
void IUH_FREE_PDPLIST(struct pdpTypeInfo *pdplist){
	iuh_syslog_debug_debug(IUH_DEFAULT,"IUH_FREE_PDPLIST\n");
	if(NULL == pdplist) return;
	
    struct pdpTypeInfo *tmp = NULL;
    tmp = pdplist;
    while(tmp != NULL){
		pdplist = tmp->next;
		iuh_syslog_debug_debug(IUH_DEFAULT,"1\n");
        IUH_FREE_OBJECT(tmp);
		iuh_syslog_debug_debug(IUH_DEFAULT,"2\n");
        tmp = pdplist;
    }
    return;
}



/*****************************************************
** DISCRIPTION:
**          free rab info list
** INPUT:
**          rab info list
** OUTPUT:
**          null
** RETURN:
**          void
*****************************************************/
void IUH_FREE_RABLIST(Iu_RAB * rablist){
	iuh_syslog_debug_debug(IUH_DEFAULT,"IUH_FREE_RABLIST\n");
    if(rablist == NULL)
        return;
    Iu_RAB *tmp = NULL;
    tmp = rablist;
    
    while(tmp != NULL){
		rablist = tmp->rab_next;
        if(tmp->pdp_type_list != NULL){
            IUH_FREE_PDPLIST(tmp->pdp_type_list->pdp_type_info);
			iuh_syslog_debug_debug(IUH_DEFAULT,"333\n");
            IUH_FREE_OBJECT(tmp->pdp_type_list);
			iuh_syslog_debug_debug(IUH_DEFAULT,"444\n");
        }
		iuh_syslog_debug_debug(IUH_DEFAULT,"555\n");
        IUH_FREE_OBJECT(tmp);
        tmp = rablist; // book modify, 2011-12-15
        iuh_syslog_debug_debug(IUH_DEFAULT,"666\n");
    }
    return;
}


/*****************************************************
** DISCRIPTION:
**          delete ue information by ue id
** INPUT:
**          ueid
** OUTPUT:
**          null
** RETURN:
**          0       succeed
**          other   fail
*****************************************************/

int IUH_DELETE_UE(int UEID)
{
    iuh_syslog_debug_debug(IUH_DEFAULT,"### IUH_DELETE_UE ###");
    iuh_syslog_debug_debug(IUH_DEFAULT,"UEID = %d\n",UEID);
    
    if(IUH_HNB_UE[UEID] == NULL) return 0;

	/* remove ue from hnb_ue list */
	Iuh_HNB_UE *tmpUE,*p;
	int HNBID = IUH_HNB_UE[UEID]->HNBID;
	tmpUE = IUH_HNB[HNBID]->HNB_UE;
	if((tmpUE != NULL) && (tmpUE->UEID == UEID)){
	    IUH_HNB[HNBID]->HNB_UE = IUH_HNB_UE[UEID]->ue_next;
	    IUH_HNB_UE[UEID]->ue_next = NULL;
	}
	else{
	    p = tmpUE;
	    tmpUE = p->ue_next;
    	while(tmpUE != NULL){
    	    if(tmpUE->UEID == UEID){
    	        p->ue_next = IUH_HNB_UE[UEID]->ue_next;
    	        IUH_HNB_UE[UEID]->ue_next = NULL;
    	        break;
    	    }
    	    p = tmpUE;
    	    tmpUE = p->ue_next;
    	}
	}

#ifdef RAB_INFO
	IUH_FREE_RABLIST(IUH_HNB_UE[UEID]->UE_RAB);
#endif
    //IUH_FREE_OBJECT(IUH_HNB_UE[UEID]->UEMAC);
    //IUH_FREE_OBJECT(IUH_HNB_UE[UEID]->UEIP);
	IUH_FREE_OBJECT(IUH_HNB_UE[UEID]);
	IUH_HNB_UE[UEID] = NULL;
	IUH_HNB[HNBID]->current_UE_number--;
	gStaticUEs--;
	/* book add, 2011-12-21 */
	msgq qData;
	qData.mqid = HNBID%THREAD_NUM+1;
	qData.mqinfo.type = CONTROL_DATA;
	qData.mqinfo.HNBID = HNBID;
	qData.mqinfo.u.ueInfo.op = UE_DEL;
	qData.mqinfo.u.ueInfo.ueInfo.UEID = UEID;
	
	iuh_syslog_debug_debug(IUH_DEFAULT,"send msg IuhMsgQid = %d\n",IuhMsgQid);
	if (msgsnd(IuhMsgQid, (msgq *)&qData, sizeof(qData.mqinfo), 0) == -1)
		perror("msgsnd");

	iuh_syslog_debug_debug(IUH_DEFAULT,"delete UE %d successful\n",UEID);
    return 0;
}


/*****************************************************
** DISCRIPTION:
**          search for ue id by ue context id
** INPUT:
**          hnbid,      contextid
** OUTPUT:
**          null
** RETURN:
**          UeId        succeed
**          0               fail
*****************************************************/

int FINDUEID(int HNBID, unsigned char *ContextID)
{
	iuh_hnb_show_str("ContextID = ", ContextID, 0, CONTEXTID_LEN);
    int UEID = 0;
    if(IUH_HNB[HNBID] == NULL) return 0;
    Iuh_HNB_UE *tempUE = IUH_HNB[HNBID]->HNB_UE;
    while(tempUE != NULL){
        if(memcmp(tempUE->context_id_str, ContextID, CONTEXTID_LEN) == 0){
            UEID = tempUE->UEID;
            break;
        }
        tempUE = tempUE->ue_next;
    }
    return UEID;
}


/*****************************************************
** DISCRIPTION:
**          Search for HnbId by RncId
** INPUT:
**          RncId
** OUTPUT:
**          null
** RETURN:
**          HnbId       succeed
**          0               fail
*****************************************************/

int IUH_FIND_HNB_RNCID(uint16_t RncId)
{
    iuh_syslog_debug_debug(IUH_DEFAULT,"RNCID = %d\n",RncId);
    int HNBID = 0;
    int i;
    for(i = 0; i < HNB_DEFAULT_NUM_AUTELAN; i++){
        if((IUH_HNB[i] != NULL) && (IUH_HNB[i]->rncId == RncId)){
            HNBID = i;
            break;
        }
    }
    
    return HNBID;
}


/*****************************************************
** DISCRIPTION:
**          Search for HnbId by sub ue ContextId
** INPUT:
**          ContextId
** OUTPUT:
**          null
** RETURN:
**          HnbId       succeed
**          0               fail
*****************************************************/

int IUH_FIND_HNB_CONTEXT(char * contextId)
{
	iuh_syslog_debug_debug(IUH_DEFAULT,"IUH_FIND_HNB_CONTEXT = %d\n");

    iuh_syslog_debug_debug(IUH_DEFAULT,"contextId = %d\n",*((int*)contextId));
    int i;
    for(i = 1; i < HNB_DEFAULT_NUM_AUTELAN; i++){
        if(IUH_HNB[i] != NULL) {
            Iuh_HNB_UE * tmpUE = IUH_HNB[i]->HNB_UE;
            while(tmpUE != NULL){
                if(memcmp(tmpUE->context_id_str, contextId, CONTEXTID_LEN) == 0){
                    return i;
                }
                tmpUE = tmpUE->ue_next;
            }
        }
    }
    
    return 0;
}


/*****************************************************
** DISCRIPTION:
**          Search for UeId by IMSI
** INPUT:
**          IMSI,       RncId
** OUTPUT:
**          null
** RETURN:
**          UeId        succeed
**          0               fail
*****************************************************/

int IUH_FIND_UE_IMSI(char * IMSI, int HNBID)
{
    iuh_syslog_debug_debug(IUH_DEFAULT,"IUH_FIND_UE_IMSI\n");
    int UEID = 0;
    int i;
    for(i = 1; i < HNB_DEFAULT_NUM_AUTELAN; i++){
        if((IUH_HNB[i] != NULL) && (IUH_HNB[i]->HNBID == HNBID)){
            Iuh_HNB_UE *tempUE = IUH_HNB[i]->HNB_UE;
            while(tempUE != NULL){
                if(memcmp(tempUE->IMSI, IMSI, IMSI_LEN) == 0){
                    UEID = tempUE->UEID;
                    return UEID;
                }
                tempUE = tempUE->ue_next;
            }
        }
    }
    
    return UEID;
}


/*****************************************************
** DISCRIPTION:
**          Search UeId by ContextId
** INPUT:
**          ContextId
** OUTPUT:
**          null
** RETURN:
**          UeId        succeed
**          0               fail
*****************************************************/

int IUH_FIND_UE_BY_CTXID(char * contextId)
{
    iuh_syslog_debug_debug(IUH_DEFAULT,"IUH_FIND_UE_BY_CTXID \n");
    int UEID = 0;
    int i;
    for(i = 0; i < HNB_DEFAULT_NUM_AUTELAN; i++){
        if((IUH_HNB[i] != NULL)){
            Iuh_HNB_UE *tempUE = IUH_HNB[i]->HNB_UE;
            while(tempUE != NULL){
                if(memcmp(tempUE->context_id_str, contextId, CONTEXTID_LEN) == 0){
                    UEID = tempUE->UEID;
					iuh_syslog_debug_debug(IUH_DEFAULT,"UEID = %d \n",UEID);
                    break;
                }
                tempUE = tempUE->ue_next;
            }
        }
    }
    
    return UEID;
}


/*****************************************************
** DISCRIPTION:
**          Search for UeId by IMSI
** INPUT:
**          IMSI
** OUTPUT:
**          null
** RETURN:
**          UeId        succeed
**          0               fail
*****************************************************/

int IUH_FIND_UE_BY_IMSI(char * IMSI)
{
    iuh_syslog_debug_debug(IUH_DEFAULT,"IUH_FIND_UE_BY_IMSI\n");
    int UEID = 0;
    int i;
    for(i = 1; i < UE_MAX_NUM_AUTELAN; i++){
        if(IUH_HNB_UE[i] != NULL){
            if(memcmp(IUH_HNB_UE[i]->IMSI, IMSI, IMSI_LEN) == 0){
                UEID = i;
                break;
            }
        }
    }

    return UEID;
}


/*****************************************************
** DISCRIPTION:
**          Update ue information by ue register request value
** INPUT:
**          UeId
**          HnbId
**          UeRegisterRequestValue struct
** OUTPUT:
**          null
** RETURN:
**          void
*****************************************************/

void IUH_UPDATE_UE(int UEID, int HNBID, UERegisterRequestValue *UERequestValue)
{
    iuh_syslog_debug_debug(IUH_DEFAULT,"IUH_UPDATE_UE\n");
    if(gStaticUEs > UE_MAX_NUM_AUTELAN){
		return;
	}
	if(IUH_HNB[HNBID] == NULL){
	    return;
	}
	if(!check_ueid_func(UEID)){
		iuh_syslog_err("hnbid is larger than max hnb number\n");
		return;
	}

    IUH_HNB_UE[UEID]->UE_Identity = UERequestValue->UE_Identity;
    IUH_HNB_UE[UEID]->Capabilities = UERequestValue->Capabilities;
    IUH_HNB_UE[UEID]->registrationCause = UERequestValue->registrationCause;
    IUH_HNB_UE[UEID]->HNBID = HNBID;
    IUH_HNB_UE[UEID]->state = REGISTED;

	/* book add, 2011-12-21 */
	msgq qData;
	qData.mqid = (IUH_HNB_UE[UEID]->HNBID)%THREAD_NUM+1;
	qData.mqinfo.type = CONTROL_DATA;
	qData.mqinfo.HNBID = HNBID;
	qData.mqinfo.u.ueInfo.op = UE_UPDATE;
	qData.mqinfo.u.ueInfo.ueInfo.UEID = UEID;
	memcpy(qData.mqinfo.u.ueInfo.ueInfo.ContextID, IUH_HNB_UE[UEID]->context_id_str, CONTEXTID_LEN);
	memcpy(qData.mqinfo.u.ueInfo.ueInfo.IMSI, IUH_HNB_UE[UEID]->IMSI, IMSI_LEN);
	
	iuh_syslog_debug_debug(IUH_DEFAULT,"send msg IuhMsgQid = %d\n",IuhMsgQid);
	if (msgsnd(IuhMsgQid, (msgq *)&qData, sizeof(qData.mqinfo), 0) == -1)
		perror("msgsnd");
    
    return;
}



/*****************************************************
** DISCRIPTION:
**          change the sequence of the str
** INPUT:
**          char[]
** OUTPUT:
**          NULL
** RETURN:
**          char *
** book add, 2011-12-20
*****************************************************/
unsigned char * iuh_hnb_change_str(unsigned char *my_str, int len){
    int i = 0;
    
    if(my_str == NULL) return (unsigned char*)"NULL";

    if(len == 0) return (unsigned char*)"NULL";

    for(i = 0; i < len; i++){
		my_str[i] = ((my_str[i] & 0x0f) << 4) | (my_str[i] >> 4);
    }

    return my_str;
}



/*****************************************************
** DISCRIPTION:
**          show str as hexdecimal
** INPUT:
**          char[]
** OUTPUT:
**          show string
** RETURN:
**          void
** book add, 2011-12-20
*****************************************************/
void iuh_hnb_show_str(const char * strname, unsigned char * my_str, int flag, int len){

    unsigned char *tmp;
    unsigned char sstr[64] = {0};
    if(flag)
        tmp = iuh_hnb_change_str(my_str,len);
    else
        tmp = my_str;

    if((tmp == NULL) || (strcmp((char*)tmp, "Null") == 0)){
		 iuh_syslog_debug_debug(IUH_DEFAULT, "%s is NULL",strname);
        return;
    }
    
    int i = 0;
    int slen = strnlen((char*)strname,16);
    sprintf((char*)sstr,"%s",strname);
    for(i = 0; i < len; i++){
        sprintf((char*)(sstr+slen+2*i),"%.2x",my_str[i]);
    }
    iuh_syslog_debug_debug(IUH_DEFAULT, "%s", sstr);
    return;
}



/*****************************************************
** DISCRIPTION:
**          init rab information
** INPUT:
**          null
** OUTPUT:
**          Iu_RAB
** RETURN:
**          void
*****************************************************/

void IUH_INIT_RAB(Iu_RAB *myRab)
{
	iuh_syslog_debug_debug(IUH_DEFAULT,"IUH_INIT_RAB\n");
    memset(myRab->RABID, 0, RAB_ID_LEN);
    memset(myRab->nAS_ScrIndicator, 0, NAS_INDICATOR_LEN);
    myRab->rab_para_list.deliveryOrder = 0;
    myRab->rab_para_list.maxSDUSize = 0;
    myRab->rab_para_list.rab_ass_indicator = 0;
    myRab->rab_para_list.traffic_class = 0;
    myRab->user_plane_info.user_plane_mode = 0;
    memset(myRab->user_plane_info.up_modeVersions, 0, USER_PLANE_MODE_LEN);
    myRab->trans_layer_info.iu_trans_assoc.present = 0;
    memset(myRab->trans_layer_info.iu_trans_assoc.choice.gtp_tei, 0, GTP_TEID_LEN);
    myRab->service_handover = 0;
    myRab->isPsDomain = 0;
    myRab->data_vol_rpt = 0;
    myRab->dl_GTP_PDU_SeqNum = 0;
    myRab->ul_GTP_PDU_seqNum = 0;
    myRab->dl_N_PDU_SeqNum = 0;
    myRab->ul_N_PDU_SeqNum = 0;
    myRab->pdp_type_list = NULL;
    myRab->rab_next = NULL;
    return;
}


/*****************************************************
** DISCRIPTION:
**          release rab by rab id
** INPUT:
**          rabid
**          ueid
** OUTPUT:
**          null
** RETURN:
**          void
*****************************************************/
void Iuh_release_rab(int UEID, const char *RABID)
{
	iuh_syslog_debug_debug(IUH_DEFAULT,"Iuh_release_rab\n");
    if(IUH_HNB_UE[UEID] == NULL){
        iuh_syslog_err("invalid ue id");
        return;
    }

    Iu_RAB *temp_rab = IUH_HNB_UE[UEID]->UE_RAB;
    if(memcmp(temp_rab->RABID, RABID, RAB_ID_LEN) == 0){
        IUH_HNB_UE[UEID]->UE_RAB = temp_rab->rab_next;
        temp_rab->rab_next = NULL;
        Iuh_free_rab(temp_rab);
        return;
    }
    Iu_RAB *p = IUH_HNB_UE[UEID]->UE_RAB;
    temp_rab = p->rab_next;
    while(temp_rab != NULL){
        if(memcmp(temp_rab->RABID, RABID, RAB_ID_LEN) == 0){
            p->rab_next = temp_rab->rab_next;
            temp_rab->rab_next = NULL;
            Iuh_free_rab(temp_rab);
            break;
        }
        p = temp_rab;
        temp_rab = p->rab_next;
    }
    
    return;
}


/*****************************************************
** DISCRIPTION:
**          free rab memory
** INPUT:
**          rab
** OUTPUT:
**          null
** RETURN:
**          void
*****************************************************/
void Iuh_free_rab(Iu_RAB *myRab)
{
    if(myRab == NULL) return;

    if(myRab->pdp_type_list == NULL) return;

    int i = 0;
    struct pdpTypeInfo *tmp_info;
    tmp_info = myRab->pdp_type_list->pdp_type_info;
    for(i = 0; i < myRab->pdp_type_list->count; i++){
        if(tmp_info != NULL){
            myRab->pdp_type_list->pdp_type_info = tmp_info->next;
            tmp_info->next = NULL;
            IUH_FREE_OBJECT(tmp_info);
        }
    }

    IUH_FREE_OBJECT(myRab->pdp_type_list);
    IUH_FREE_OBJECT(myRab);
    return;
}



/*****************************************************
** DISCRIPTION:
**          optimize paging hnb by imsi or lai
** INPUT:
**          hnbid
**          Iuh2IuMsg
** OUTPUT:
**          null
** RETURN:
**          1           yes
**          0           no
*****************************************************/
int IUH_OPTIMIZE_PAGING_HNB(int HNBID, Iuh2IuMsg *sigMsg)
{
	iuh_syslog_debug_debug(IUH_DEFAULT,"paging_imsi = %d, paging_lai = %d\n",gSwitch.paging_imsi,gSwitch.paging_lai);
    if(IUH_HNB[HNBID] == NULL) return 0;
    
    if(gSwitch.paging_imsi == 1){
        int UEID = 0;
        UEID = IUH_FIND_UE_IMSI(sigMsg->imsi, IUH_HNB[HNBID]->rncId);
        if((IUH_HNB_UE[UEID] != NULL) && (IUH_HNB_UE[UEID]->HNBID == HNBID)){
            return 1;
        }
		else{
			return 0;
		}
    }
    else if(gSwitch.paging_lai == 1){
        if((memcmp(IUH_HNB[HNBID]->plmnId, sigMsg->lai.plmnid, PLMN_LEN) == 0) \
            && (memcmp(IUH_HNB[HNBID]->lac, sigMsg->lai.lac, LAC_LEN) == 0)){
            return 1;
        }
		else{
			return 0;
		}
    }
    
    return 1;
}


/*****************************************************
** DISCRIPTION:
**          command delete hnb by hnbid
** INPUT:
**          hnbid
** OUTPUT:
**          null
** RETURN:
**          1           yes
**          0           no
*****************************************************/
int IUH_CMD_DELETE_HNB(int HNBID)
{
    iuh_syslog_debug_debug(IUH_DEFAULT,"call IUH_CMD_DELETE_HNB\n");
    if(IUH_HNB[HNBID] == NULL)
        return 0;
    
    int ret;   
    IuhProcotolMsg msgPtr;
    if(!IuhAssembleHNBDeRegister(&msgPtr)){
        iuh_syslog_err("Iuh assamble hnb deregister failed.\n");
    }
    
    if(!IuhSendMessage(HNBID, &msgPtr, HNBAP_PPID)) {
        iuh_syslog_err("Error Send UE De-Register Message.\n");
    }

    ret = IUH_DELETE_HNB(HNBID);

    return ret;
}


/*****************************************************
** DISCRIPTION:
**          command delete ue by ueid
** INPUT:
**          ue
** OUTPUT:
**          null
** RETURN:
**          1           yes
**          0           no
*****************************************************/
int IUH_CMD_DELETE_UE(int UEID)
{
    iuh_syslog_debug_debug(IUH_DEFAULT,"call IUH_CMD_DELETE_UE\n");
    if(IUH_HNB_UE[UEID] == NULL)
        return 0;
        
    int ret;   
    IuhProcotolMsg msgPtr;
    if(!IuhAssembleUEDeRegister(UEID, &msgPtr)){
        iuh_syslog_err("Iuh assamble hnb deregister failed.\n");
    }

    if(!IuhSendMessage(IUH_HNB_UE[UEID]->HNBID, &msgPtr, HNBAP_PPID)) {
        iuh_syslog_err("Error Send UE De-Register Message.\n");
    }

    ret = IUH_DELETE_UE(UEID);

    return ret;
}


/*****************************************************
** DISCRIPTION:
**          Search for HnbId by CellId
** INPUT:
**          CellId
** OUTPUT:
**          null
** RETURN:
**          HnbId       succeed
**          0               fail
*****************************************************/

int IUH_FIND_HNB_CellId(const char *CellId)
{
	if(CellId == NULL){
		iuh_syslog_err("error: CellId is NULL\n");
		return 0;
	}
    iuh_syslog_debug_debug(IUH_DEFAULT,"CellId = %d\n",*((int*)CellId));
    int HNBID = 0;
    int i;
    for(i = 0; i < HNB_DEFAULT_NUM_AUTELAN; i++){
        if((IUH_HNB[i] != NULL) && (IUH_HNB[i]->cellId != NULL) && (memcmp(IUH_HNB[i]->cellId, CellId, CELLID_LEN) == 0)){
            HNBID = i;
            break;
        }
    }
    
    return HNBID;
}
int IUH_FIND_IMSI_IN_ACL_WHITE_LIST(IMSIWHITELIST *head, unsigned char* imsi)
{
	int ret = IMSI_NOT_EXIST_IN_LIST;
	
	while(head)
	{
		if(!memcmp(head->imsi, imsi, IMSI_LEN))
		{
			ret = IMSI_EXIST_IN_LIST;
			break;
		}
		head = head->next;
	}
	return ret;
}

int IUH_ADD_FEMTO_ACL_WHITE_LIST(IMSIWHITELIST **head, unsigned char* imsi)
{
	int ret = FEMTO_ACL_SUCCESS;
	ret = IUH_FIND_IMSI_IN_ACL_WHITE_LIST(*head, imsi);
	if(ret == IMSI_NOT_EXIST_IN_LIST)
	{
		IMSIWHITELIST* node = NULL;
		if((node = (IMSIWHITELIST*)malloc(sizeof(IMSIWHITELIST)))!=NULL)
		{
			memcpy(node->imsi, imsi, IMSI_LEN);
			node->next = NULL;
		}
		else
			return MALLOC_ERROR;
		
		if(*head == NULL)
		{
			*head = node;
			node = NULL;
			ret = FEMTO_ACL_SUCCESS;
		}
		else
		{
			IMSIWHITELIST* tmpnode = *head;
			while(tmpnode->next)
				tmpnode = tmpnode->next;
			
			tmpnode->next = node;
			node = NULL;
			ret = FEMTO_ACL_SUCCESS;
		}
	}
	return ret;
}
int IUH_DEL_FEMTO_ACL_WHITE_LIST(IMSIWHITELIST **head, unsigned char* imsi)
{
	int ret = IMSI_NOT_EXIST_IN_LIST;
	ret = IUH_FIND_IMSI_IN_ACL_WHITE_LIST(*head, imsi);
	if(ret == IMSI_EXIST_IN_LIST)
	{
		if(!memcmp((*head)->imsi, imsi, IMSI_LEN))	//the first node will be deleted
		{
			IMSIWHITELIST *tmp = *head;
			*head = (*head)->next;
			IUH_FREE_OBJECT(tmp);
			ret = FEMTO_ACL_SUCCESS;
		}
		else
		{
			IMSIWHITELIST *tmp = *head;
			while(tmp->next)
			{
				if(!memcmp(tmp->next->imsi, imsi, IMSI_LEN))
				{
					IMSIWHITELIST *tmpnode = tmp->next;
					tmp->next = tmp->next->next;
					IUH_FREE_OBJECT(tmpnode);
					ret = FEMTO_ACL_SUCCESS;
					break;
				}
				tmp = tmp->next;
			}
		}
	}
	return ret;
}
void IUH_IMSI_DIGIT_CONVERT(unsigned char* imsi, unsigned char* imsi_str)
{
	int i = 0;
	int j = 0;
	unsigned char IMSI[IMSI_DIGIT_LEN+1] = {0};
	memcpy(IMSI, imsi, IMSI_DIGIT_LEN);
	IMSI[IMSI_DIGIT_LEN] = 0x0f;
	
	for(i=0; i<IMSI_DIGIT_LEN+1 &&j<IMSI_LEN; i=i+2)
	{
		imsi_str[j++] = (IMSI[i]&0x0f) | ((IMSI[i+1]&0x0f)<<4);
	}
	return;
}


