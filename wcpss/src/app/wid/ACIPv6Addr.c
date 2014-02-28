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
* ACIPv6Addr.c
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

#include "ACIPv6Addr.h"
#include "dbus/wcpss/ACDbusDef1.h"

char *ipv6addr (struct sockaddr_in6 *sa)
{
	static unsigned char string[40];
	unsigned char *p, *q = string;
	int i = 1;
	for (p = sa->sin6_addr.in6_u.u6_addr8; i<=8; i++) {
		sprintf ((char*)q, "%02x", *p++); q+=2;
		sprintf ((char*)q, "%02x", *p++); q+=2;
		sprintf ((char*)q++, ":");
	}
	return (char*)string;
}


int get_if_addr_ipv6(const char *ifname,
                        struct sockaddr_in6 *ipaddr,
                        int *sysindex)
{
    FILE *fp = fopen(PROC_IFINET6_PATH, "r");
	char addrstr[INET6_ADDRLEN];
    char seg[8][5];
    int index, plen, scope, flags;
    char ifn[IFNAMSIZ];
    int ret = -1;
                                                                                                                                               
    if (fp)
    {
        while (fscanf(fp, "%4s%4s%4s%4s%4s%4s%4s%4s %02x %02x %02x %02x %s\n",
                      seg[0], seg[1], seg[2], seg[3], seg[4], seg[5], seg[6],
                      seg[7], &index, &plen, &scope, &flags, ifn) != EOF)
        {
            //if ((strlen(ifname) == strlen(ifn))&&(strcmp(ifn, ifname) == 0))
			if ((strcmp(seg[0],"fe80") != 0)&&(strlen(ifname) == strlen(ifn))&&(strcmp(ifn, ifname) == 0))
            {
                sprintf(addrstr, "%s:%s:%s:%s:%s:%s:%s:%s", seg[0], seg[1],
                        seg[2], seg[3], seg[4], seg[5], seg[6], seg[7]);
                ret = inet_pton(AF_INET6, addrstr, &(ipaddr->sin6_addr));
				*sysindex = index;
				 goto out;
            }
        }
                                                                                                                                               
        errno = ENXIO;
out:
        fclose(fp);
    }

																																			   
    return ret;
}

int get_if_addr_ipv6_list(const char *ifname,
                        struct tag_ipv6_addr_list *ipv6list)
{
    FILE *fp = NULL;//fopen(PROC_IFINET6_PATH, "r");
    char seg[8][5];
    int index, plen, scope, flags;
    char ifn[IFNAMSIZ];
    int ret = -1;
                                                                                                                                               
	if ((fp = popen("sudo cat /proc/net/if_inet6", "r")) == NULL)
	{
		wid_syslog_info("%s: popen failed.\n", __func__);
		return -1;
	}
	
    if (fp)
    {
        while (fscanf(fp, "%4s%4s%4s%4s%4s%4s%4s%4s %02x %02x %02x %02x %s\n",
                      seg[0], seg[1], seg[2], seg[3], seg[4], seg[5], seg[6],
                      seg[7], &index, &plen, &scope, &flags, ifn) != EOF)
        {
            //if ((strlen(ifname) == strlen(ifn))&&(strcmp(ifn, ifname) == 0))
			if ((strcmp(seg[0],"fe80") != 0)&&(strlen(ifname) == strlen(ifn))&&(strcmp(ifn, ifname) == 0))
            {
                struct tag_ipv6_addr *ipv6addr = (struct tag_ipv6_addr *)WID_MALLOC(sizeof(struct tag_ipv6_addr));
				if(ipv6addr == NULL)
				{
					 goto out;
				}
				ipv6addr->next = NULL;	
                sprintf((char*)ipv6addr->ipv6addr, "%s:%s:%s:%s:%s:%s:%s:%s", seg[0], seg[1],
                        seg[2], seg[3], seg[4], seg[5], seg[6], seg[7]);

				if(ipv6list->ipv6list == NULL)
				{
					ipv6list->ifindex = index;
					ipv6list->ipv6num = 1;
					ipv6list->ipv6list = ipv6addr;
					ret = 0;
				}
				else
				{
					ret = 0;
					ipv6list->ifindex = index;
					ipv6list->ipv6num++;
					ipv6addr->next = ipv6list->ipv6list;
					ipv6list->ipv6list = ipv6addr;
				}
               
            }
        }
                                                                                                                                               
        errno = ENXIO;
out:
	   pclose(fp);
    }

																																			   
    return ret;
}



int ipv6_bind_interface_for_wid(struct ifi_info *ifi, int port)
{
	int ret = 0;
	int sockfd;
	int yes = 1;
	int i = 0;

	struct sockaddr_in6 servaddr;
	struct CWMultiHomedInterface *p;

	((struct sockaddr_in6*)ifi->ifi_addr6)->sin6_family = AF_INET6;
	((struct sockaddr_in6*)ifi->ifi_addr6)->sin6_port = port;
	
	//memcpy(ifi->ifi_addr, sin6ptr, sizeof(struct sockaddr_in6));
	
	/*
	ret = get_if_addr_ipv6(ifi->ifi_name,(struct sockaddr_in6*)ifi->ifi_addr6,sysindex);
	
	printf("002 systemindec = %d\n",*sysindex);

	if(ret == -1)
	{
		return BINDING_IPV6_ADDRE_RROR;
	}

	*/
	
	if ((sockfd = socket (AF_INET6, SOCK_DGRAM, 0)) < 0)
	{
		perror("socket");
		wid_syslog_info("ipv6_bind_interface_for_wid socket error");
		printf("ipv6_bind_interface_for_wid socket error");
		return BINDING_IPV6_ADDRE_RROR;
	}
	printf("ipv6 fd= %d\n",sockfd);
	
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

	
	bzero(&servaddr,sizeof (servaddr));
	servaddr.sin6_family = AF_INET6;
	servaddr.sin6_port = htons (port) ;

	//printf("002 ipv6 addr = %s\n",addrstr);
	//ret = inet_pton(AF_INET6, addrstr, &servaddr.sin6_addr);	 
	
	memcpy(&(&servaddr)->sin6_addr,
					   &((struct sockaddr_in6 *)ifi->ifi_addr6)->sin6_addr,
					   sizeof(struct in6_addr));
					   
	//servaddr.sin6_addr = in6addr_any;

	if(bind(sockfd, (struct sockaddr*)&servaddr, sizeof(struct sockaddr_in6)) < 0)
	{
	
		//return BINDING_IPV6_ADDRE_RROR;
/////////////////////////////////////////////////
		if(istrybindipv6addr == 0)
		{
			for(i=0;i<READ_IFNET_INFO_COUNT;i++)
			{
				struct timeval tval;
				tval.tv_sec = 0;
				tval.tv_usec = 500000;
				select(0,NULL,NULL,NULL,&tval); 
				
				ret = bind(sockfd, (struct sockaddr*)&servaddr, sizeof(struct sockaddr_in6));
				wid_syslog_debug_debug(WID_DEFAULT,"bindding ipv6 retry count %d",i);

				if(ret == 0)
				{	
					istrybindipv6addr = 1;
					break;
				}
				istrybindipv6addr = 1;
			}
		}
		else
		{
			close(sockfd);
			CWUseSockNtop(ifi->ifi_addr6,
				wid_syslog_info("bindding ipv6 failed %s", str);
				);
			CWUseSockNtop(ifi->ifi_addr6,
				printf("bindding ipv6 failed %s", str);
				);			
			perror("bind");
			wid_syslog_info("bindding ipv6 error, error num is %d %s", errno,strerror(errno));
			printf("binding sock fail\n");
		}
		
		if(ret != 0)
		{
			ret = BINDING_IPV6_ADDRE_RROR;
			close(sockfd);
			CWUseSockNtop(ifi->ifi_addr6,
				wid_syslog_info("bindding ipv6 failed %s", str);
				);
			CWUseSockNtop(ifi->ifi_addr6,
				printf("bindding ipv6 failed %s", str);
				);			
			perror("bind");
			wid_syslog_info("bindding ipv6 error, error num is %d %s", errno,strerror(errno));
			printf("binding sock fail\n");

			wid_syslog_info("bindding ipv6 error, error num is %d %s", errno,strerror(errno));
			return ret;
		}

////////////////////////////////////////////
	}
	//printf("binding sock success\n");

	//printf("CWUseSockNtop family=%d\n",ifi->ifi_addr6->sa_family);

	CWUseSockNtop(ifi->ifi_addr6,
			wid_syslog_notice("bound %s (%d, %s)", str, ifi->ifi_index, ifi->ifi_name);
	);	
	CWUseSockNtop(ifi->ifi_addr6,
			printf("bound %s (%d, %s)", str, ifi->ifi_index, ifi->ifi_name);
	);	

	// store socket inside multihomed socket
		
	CW_CREATE_OBJECT_ERR_WID(p, struct CWMultiHomedInterface, close(sockfd); return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
	memset(p->ifname, 0, IFI_NAME);
	strncpy(p->ifname, ifi->ifi_name, IFI_NAME);
	p->sock = sockfd; 	
	wid_syslog_notice("p->sock %d\n",p->sock);
	if(strncmp(ifi->ifi_name, "lo", 2)) { // don't consider loopback an interface
		wid_syslog_debug_debug(WID_DEFAULT,"Primary Address");
		wid_syslog_debug_debug(WID_DBUS,"Primary Address");
		p->kind = CW_PRIMARY;
	} else {
		p->kind = CW_BROADCAST_OR_ALIAS; // should be BROADCAST_OR_ALIAS_OR_MULTICAST_OR_LOOPBACK ;-)
#ifdef CW_DEBUGGING
			if(!strncmp(ifi->ifi_name, "lo", 2)) {
				p->kind = CW_PRIMARY;
			}
#endif
	}
	p->systemIndex = ifi->ifi_index;
	
	p->systemIndexbinding = ifi->ifi_index_binding;
	p->ipv6Flag = 1;
	// the next field is useful only if we are an IPv6 server. In this case, p->addr contains the IPv6
	// address of the interface and p->addrIPv4 contains the equivalent IPv4 address. On the other side,
	// if we are an IPv4 server p->addr contains the IPv4 address of the interface and p->addrIPv4 is
	// garbage.
	p->addrIPv4.ss_family = AF_UNSPEC;
	CW_COPY_NET_ADDR_PTR(&(p->addr), ifi->ifi_addr6);
	if(p->kind == CW_PRIMARY)
	for(i = 0; i < gMaxInterfacesCount; i++) {
		if(gInterfaces[i].enable == 0){
			CW_COPY_NET_ADDR_PTR(&(gInterfaces[i].addr), ((CWNetworkLev4Address*)&(p->addr)) );
			CW_COPY_NET_ADDR_PTR(&(gInterfaces[i].addrIPv6), ((struct sockaddr_in6 *)&(p->addr)) );
			//CW_COPY_NET_ADDR_PTR(&(s->addrIPv4), ifi->ifi_addr);
			gInterfaces[i].enable = 1;
			gInterfaces[i].WTPCount = 0;
			p->gIf_Index = i;
			break;
		}
	}
	struct CWMultiHomedInterface* inf = gACSocket.interfaces;
	if(gACSocket.interfaces == NULL)
		gACSocket.interfaces = p;
	else{
		while(inf->if_next != NULL)
			inf = inf->if_next;
		inf->if_next = p;			
		p->if_next = NULL;
		
	}
	gACSocket.count++; // we add a socket to the multihomed socket
	Check_gACSokcet_Poll(&gACSocket);

	//display_ginterface_list();
	//display_gmlltisock_list(&gACSocket);
	
	//printf("save sock success %d\n",gACSocket.count);	
	return 0;

		
}

void free_ipv6_addr_list(struct tag_ipv6_addr_list *ipv6list)
{
	if((ipv6list == NULL)||(ipv6list->ipv6num == 0)||(ipv6list->ipv6list == NULL))
		return ;
		
	struct tag_ipv6_addr *ipv6addr = ipv6list->ipv6list;
	struct tag_ipv6_addr *ipv6addr_next = ipv6addr;
	WID_FREE(ipv6list);
	ipv6list = NULL;

	while(ipv6addr != NULL)
	{
		ipv6addr_next = ipv6addr->next;
		
		WID_FREE(ipv6addr);
		ipv6addr = NULL;
		ipv6addr = ipv6addr_next;
	}		

}

void display_ginterface_list()
{
	int i = 0;
	char ipaddr[128];
	char *paddr;
	for(i = 0; i < gMaxInterfacesCount; i++) 
	{
		if(gInterfaces[i].enable == 1)
		{
			paddr = sock_ntop_r(((struct sockaddr*)&(gInterfaces[i].addr)), ipaddr);
			printf("the %d addr is:%s\n",i,paddr);
			
		}
	}
}

void display_gmlltisock_list(CWMultiHomedSocket *sockPtr)
{
	int i = 0;
	char ipaddr[128];
	char *paddr;
	struct CWMultiHomedInterface *inf;
	if(sockPtr == NULL || sockPtr->interfaces == NULL) 
	{
		printf("binding sock is null\n");
		return;
	}
	
	inf = sockPtr->interfaces;
	printf("all multi sock is %d\n",sockPtr->count);
	for(i = 0; (i < sockPtr->count)&&(inf != NULL); i++) {
		
		paddr = sock_ntop_r(((struct sockaddr*)&(gInterfaces[i].addr)), ipaddr);
		printf("ifname=%s addr=%s sock=%d sysindex=%d bindingindex=%d gindex=%d\n",\
			inf->ifname,paddr,inf->sock,inf->systemIndex,inf->systemIndexbinding,inf->gIf_Index);
		
		if(inf->if_next == NULL)
			break;
		inf = inf->if_next;
	}

}

void display_ipv6_addr_list(struct tag_ipv6_addr_list *ipv6list)
{
	if((ipv6list == NULL)||(ipv6list->ipv6num == 0)||(ipv6list->ipv6list == NULL))
		return ;

	struct tag_ipv6_addr *ipv6addr = ipv6list->ipv6list;
	int i=0;
	printf("interface index is %d\n",ipv6list->ifindex);
	printf("interface ipv6 addr count is %d\n",ipv6list->ipv6num);

	for(i=0; i<ipv6list->ipv6num; i++)
	{
		printf("the %d ipv6addr is %s\n",i,ipv6addr->ipv6addr);
		ipv6addr = ipv6addr->next;
	}	
}


struct tag_ipv6_addr_list * get_ipv6_addr_list(char * ifname)
{
	int fd = 0, i = 0;
	dev_ipv6_addr_t addr;
	struct tag_ipv6_addr_list *ipv6list = NULL;
	bzero(&addr, sizeof(addr));
	memcpy(addr.ifname, ifname, strlen(ifname));
	
	fd = open("/dev/wifi0", O_RDWR);
	if(fd < 0){
		return NULL;
	}
	if (ioctl(fd, WIFI_IOC_GET_V6ADDR, &addr) < 0) {
			perror("ioctl");
			close(fd);
			return NULL;
	}
	
	//printf("ioctl ok.\n");
	//printf ("ifindex = %d, stat = %d, cnt = %d\n", addr.ifindex, addr.stat, addr.addr_cnt);
	i = addr.addr_cnt;

	ipv6list = (struct tag_ipv6_addr_list *)WID_MALLOC(sizeof(struct tag_ipv6_addr_list));
	if (NULL == ipv6list)
	{
		goto out;
	}
	ipv6list->ifindex = addr.ifindex;
	ipv6list->ipv6list = NULL;
	ipv6list->ipv6num = 0;
	//printf("ioctl ok.\n");
	for (i = 0; i < addr.addr_cnt; i++)
	{
		//printf ("addr: %02x%02x:%02x%02x\n", addr.addr[i].addr[0], addr.addr[i].addr[1], addr.addr[i].addr[14], addr.addr[i].addr[15]);
		if(addr.addr[i].addr[0] != 0xfe)
		{
			//printf("strcmp after\n");	
			struct tag_ipv6_addr *ipv6addr = (struct tag_ipv6_addr *)WID_MALLOC(sizeof(struct tag_ipv6_addr));
			if(ipv6addr == NULL)
			{
				CW_FREE_OBJECT_WID(ipv6list);
				 goto out;
			}
			ipv6addr->next = NULL;	
			sprintf((char*)ipv6addr->ipv6addr, "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x", addr.addr[i].addr[0],addr.addr[i].addr[1],\
					addr.addr[i].addr[2],addr.addr[i].addr[3],addr.addr[i].addr[4],addr.addr[i].addr[5],\
					addr.addr[i].addr[6],addr.addr[i].addr[7],addr.addr[i].addr[8],addr.addr[i].addr[9],\
					addr.addr[i].addr[10],addr.addr[i].addr[11],addr.addr[i].addr[12],addr.addr[i].addr[13],\
					addr.addr[i].addr[14],addr.addr[i].addr[15]);
		
			if(ipv6list->ipv6list == NULL)
			{
				ipv6list->ipv6num = 1;
				ipv6list->ipv6list = ipv6addr;
			}
			else
			{
				ipv6list->ipv6num++;
				ipv6addr->next = ipv6list->ipv6list;
				ipv6list->ipv6list = ipv6addr;
			}
		   
		}
			
	}
	
	out:
	//printf("close fd\n");	
	close(fd);

	return ipv6list;

}


