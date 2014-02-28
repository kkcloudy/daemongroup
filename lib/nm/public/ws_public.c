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
* ws_public.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* 
*
* DESCRIPTION:
* function for web
*
*
***************************************************************************/
#ifndef __NET_INF_H
#define __NET_INF_H

#include <net/if.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>   
#include <linux/rtnetlink.h>
#include <stdbool.h>   
#include <fcntl.h>
#include <stdint.h>
#include <net/if_arp.h>
#include <arpa/inet.h>
#include <mcheck.h>
#include "ws_public.h"
#include "ws_init_dbus.h"
#include "ws_snmpd_engine.h"
#endif


ifi_info *get_ifi_info(int family, int doaliases)
{
  ifi_info *ifi, *ifihead, **ifipnext;
  int sockfd, len, lastlen, flags, myflags;
  char *ptr, *buf, lastname[IFNAMSIZ], *cptr;
  struct ifconf ifc;
  struct ifreq *ifr, ifrcopy;
  struct sockaddr_in *sinptr;

  if ((sockfd=socket(AF_INET, SOCK_DGRAM, 0))<0) {
    //printf("socket error.\n");
	return NULL;
//	exit(1);
  }

  lastlen = 0;

  len = 10*sizeof(struct ifreq);
  while (1)
  {
    buf = (char*)malloc(len);
    ifc.ifc_len = len;
    ifc.ifc_buf = buf;
    if (ioctl(sockfd, SIOCGIFCONF, &ifc)<0)
    {
      if (errno!=EINVAL||lastlen!=0)
      {
        printf("ioctl error.\n");
      }
    }
    else 
    {
      if (ifc.ifc_len == lastlen)
        break;
      lastlen = ifc.ifc_len;
    }
    len += 10*sizeof(struct ifreq);
    free(buf);
  }
  ifihead = NULL;
  ifipnext = &ifihead;
  lastname[0] = 0;

  for (ptr = buf; ptr<buf+ifc.ifc_len;)
  {
    ifr = (struct ifreq*)ptr;
#ifdef HAVE_SOCKADDR_SA_LEN
    len = sizeof(struct sockaddr)>ifr->ifr_addr.sa_len?sizeof(struct sockaddr):ifr->ifr_addr.sa_len;
#else
    switch (ifr->ifr_addr.sa_family)
    {
#ifdef IPV6
    case: AF_INET6:
      LEN = sizeof(struct sockaddr_in6);
      break;
#endif
    case AF_INET:
    default:
      len = sizeof(struct sockaddr);
      break;
    }
#endif

    ptr += sizeof(ifr->ifr_name) + len;

    if (ifr->ifr_addr.sa_family != family)
      continue;

    myflags = 0;
    if ((cptr=strchr(ifr->ifr_name, ':'))!=NULL)
      *cptr = 0;
    if (strncmp(lastname, ifr->ifr_name, IFNAMSIZ)==0)
    {
      if (doaliases == 0)
        continue;
      myflags = IFI_ALIAS;
    }

    memcpy(lastname, ifr->ifr_name, IFNAMSIZ);

    ifrcopy = *ifr;
    ioctl(sockfd, SIOCGIFFLAGS, &ifrcopy);
    flags = ifrcopy.ifr_flags;
   /* if ((flags&IFF_UP)==0)
      continue;
    
    if ((flags&IFF_BROADCAST)==0)
      continue;
    */
    ifi = calloc(1, sizeof(struct ifi_info));
    *ifipnext = ifi;
    ifipnext = &ifi->ifi_next;
    ifi->ifi_flags = flags;
    ifi->ifi_myflags = myflags;
    memcpy(ifi->ifi_name, ifr->ifr_name, IFI_NAME);
    ifi->ifi_name[IFI_NAME-1] = '\0';

    switch (ifr->ifr_addr.sa_family)
    {
    case AF_INET:
      sinptr = (struct sockaddr_in *)&ifr->ifr_addr;
      if (ifi->ifi_addr == NULL)
      {
        ifi->ifi_addr = calloc(1, sizeof(struct sockaddr_in));
        memcpy(ifi->ifi_addr, sinptr, sizeof(struct sockaddr_in));
#ifdef SIOCGIFBRDADDR
        if (flags & IFF_BROADCAST)
        {
          ioctl(sockfd, SIOCGIFBRDADDR, &ifrcopy);
          sinptr = (struct sockaddr_in *)&ifrcopy.ifr_broadaddr;
          ifi->ifi_brdaddr = calloc(1, sizeof(struct sockaddr_in));
          memcpy(ifi->ifi_brdaddr, sinptr, sizeof(struct sockaddr_in));
        }
#endif
#ifdef SIOCGIFDSTADDR
        if (flags & IFF_POINTOPOINT)
        {
          ioctl(sockfd, SIOCGIFDSTADDR, &ifrcopy);
          sinptr = (struct sockaddr_in*)&ifrcopy.ifr_dstaddr;
          ifi->ifi_dstaddr = calloc(1, sizeof(struct sockaddr_in));
          memcpy(ifi->ifi_dstaddr, sinptr, sizeof(struct sockaddr_in));
        }
#endif
///////////////
#ifdef SIOCGIFNETMASK

//if (flags & IFF_BROADCAST)
        {
          ioctl(sockfd, SIOCGIFNETMASK, &ifrcopy);
          sinptr = (struct sockaddr_in *)&ifrcopy.ifr_netmask;
          ifi->ifi_mask= calloc(1, sizeof(struct sockaddr_in));
          memcpy(ifi->ifi_mask, sinptr, sizeof(struct sockaddr_in));
        }

#endif
//////////////
      }
      break;
    default:
      break;
    }
  }
  free(buf);
  close(sockfd);
  return(ifihead);
}

void free_ifi_info(ifi_info *ifihead)
{
  ifi_info *ifi, *ifinext;
  for (ifi=ifihead; ifi!=NULL; ifi=ifinext)
  {
    if (ifi->ifi_addr!=NULL)
      free(ifi->ifi_addr);

    if (ifi->ifi_brdaddr!=NULL)
      free(ifi->ifi_brdaddr);
    if (ifi->ifi_dstaddr!=NULL)
      free(ifi->ifi_dstaddr);
	if (ifi->ifi_mask!=NULL)
      free(ifi->ifi_mask);
	
    ifinext = ifi->ifi_next;

    free(ifi);
  }
  
}

char *sock_ntop(const struct sockaddr *sa, socklen_t salen)
{
  char portstr[7];
  static char str[128];

  switch (sa->sa_family)
  {
  case AF_INET:
    {
      struct sockaddr_in *sin = (struct sockaddr_in *)sa;

      if (inet_ntop(AF_INET, &sin->sin_addr, str, sizeof(str))==NULL)
        return NULL;

      if (ntohs(sin->sin_port)!=0)
      {
        snprintf(portstr, sizeof(portstr), ".%d", ntohs(sin->sin_port));
        strcat(str, portstr);
      }
      return str;
    }
    break;
  case AF_INET6:
    {
      struct sockaddr_in6 *sin = (struct sockaddr_in6 *)sa;

      if (inet_ntop(AF_INET6, &sin->sin6_addr, str, sizeof(str))==NULL)
        return NULL;

      if (ntohs(sin->sin6_port)!=0)
      {
        snprintf(portstr, sizeof(portstr), ".%d", ntohs(sin->sin6_port));
        strcat(str, portstr);
      }
      return str;
    }
    break;
  default:
    return NULL;
    break;
  }
  

}

int interface_list_ioctl (int af,struct inf * interface)
{
	ifi_info *ifi, *ifihead;
	infi *p_tail = NULL;
	struct sockaddr *sa;
	u_char *ptr;
	int i, family, doaliases;
	
	if (af == 0)
		family = AF_INET;
	else if (af == 1)
		family =AF_INET6;
	else {
		//printf("invalid <address-family>");
		return -1;
	}
	
	interface->next = NULL;
	p_tail = interface;
	// infi *p_head = NULL;

  
  for(ifihead = ifi = get_ifi_info(family, doaliases);
      ifi!=NULL;ifi=ifi->ifi_next)
  {
   infi * p = (infi *)malloc(sizeof(infi));
  memset(p,0,sizeof(infi));
    strcpy(p->if_name,ifi->ifi_name);
   // printf("%s:<", ifi->ifi_name);
    if (ifi->ifi_flags&IFF_UP) 
    	{
    	//	printf("UP");
    		strcpy(p->if_stat,"UP");
		    p->upflag=1;
    	}
    else
    	{
    	//	printf("DOWN");
    		strcpy(p->if_stat,"DOWN");
		    p->upflag=0;
    	}
  //  if (ifi->ifi_flags&IFF_BROADCAST) printf("BCAST");
   // if (ifi->ifi_flags&IFF_MULTICAST) printf("MCAST");
   // if (ifi->ifi_flags&IFF_LOOPBACK) printf("LOOP");
   // if (ifi->ifi_flags&IFF_POINTOPOINT) printf("P2P");
   // printf(">\n");

    if ((i=ifi->ifi_hlen)>0)
    {
      ptr = ifi->ifi_haddr;
      do
      {
       // printf("%s%x", (i==ifi->ifi_hlen)?" ":":", *ptr++);
      }while(--i>0);
      
      printf("\n");
    }

    if ((sa=ifi->ifi_addr)!=NULL)
    {
     // printf(" IP addr: %s\n",
     //        sock_ntop(sa, sizeof(*sa)));
       strcpy(p->if_addr,sock_ntop(sa, sizeof(*sa)));
        }
    if ((sa=ifi->ifi_brdaddr)!=NULL)
    	{}
     // printf(" broadcast addr: %s\n",
     //        sock_ntop(sa, sizeof(*sa)));
    if ((sa=ifi->ifi_dstaddr)!=NULL)
    	{}
     // printf(" destnation addr: %s\n",
      //       sock_ntop(sa, sizeof(*sa)));

   //fprintf(stderr,"zhouym : before netmask:\n");
   
   if ((sa=ifi->ifi_mask)!=NULL)
   {
       //fprintf(stderr,"zhouym : enter netmask:\n");
       strcpy(p->if_mask,sock_ntop(sa, sizeof(*sa)));
   }

	
    p -> next = NULL;
    p_tail -> next = p;
    p_tail = p;
    
  }
  free_ifi_info(ifihead);
}

void free_inf(infi * infter)
{
	infi * f1,*f2;
	f1 = infter->next;
	f2 = f1->next;
	while(f2!=NULL)
		{
			free(f1);
			f1 = f2;
			f2 = f2->next;
			
		}
	free(f1);
}


unsigned long dcli_ip2ulong(char *str)
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
}



void check_trap_conf_file(void)
{
	if (access(TRAP_CONF_PATH, 0) != 0)
	{
		system("sudo cp /opt/www/htdocs/trap/trapconf_option "TRAP_CONF_PATH";sudo chmod a+rw "TRAP_CONF_PATH);
	}
}



#if 0
int main(int argc, char *argv[])
{
  mtrace();  
 infi  interf;
interface_list_ioctl (0,&interf);
infi * q ;
q = interf.next;
while(q)
{
	printf("interface.if_name=%s",q->if_name);
	printf("interface.if_addr=%s",q->if_addr);
	printf("interface.if_addr=%s\n",q->if_stat);
	q = q->next;
}
  free_inf(&interf);
  exit(0);
}

#endif
struct nlsock {  
    int sock;  
    int seq;  
    struct sockaddr_nl snl;  
    char *name;  
}net_cmd = { -1, 0, {0}, "netlink-cmd" };  

struct nl_if_info {   
    uint32_t addr;   
    char *name;   
};

static int nl_socket ( struct nlsock *nl, unsigned long groups )  
{  
    int ret;  
    struct sockaddr_nl snl;  
    int sock;  
    int namelen;  
  
    sock = socket ( AF_NETLINK, SOCK_RAW, NETLINK_ROUTE );  
    if ( sock < 0 ) {  
        fprintf ( stderr,  "Can't open %s socket: %s", nl->name,  
                strerror ( errno ) );  
        return -1;  
    }  
  
    ret = fcntl ( sock, F_SETFL, O_NONBLOCK );  
    if ( ret < 0 ) {  
        fprintf ( stderr,  "Can't set %s socket flags: %s", nl->name,  
                strerror ( errno ) );  
        close ( sock );  
        return -1;  
    }  
  
    memset ( &snl, 0, sizeof snl );  
    snl.nl_family = AF_NETLINK;  
    snl.nl_groups = groups;  
  
    /* Bind the socket to the netlink structure for anything. */  
    ret = bind ( sock, ( struct sockaddr * ) &snl, sizeof snl );  
    if ( ret < 0 ) {  
        fprintf ( stderr,  "Can't bind %s socket to group 0x%x: %s",  
                nl->name, snl.nl_groups, strerror ( errno ) );  
        close ( sock );  
        return -1;  
    }  
  
    /* multiple netlink sockets will have different nl_pid */  
    namelen = sizeof snl;  
    ret = getsockname ( sock, ( struct sockaddr * ) &snl, &namelen );  
    if ( ret < 0 || namelen != sizeof snl ) {  
        fprintf ( stderr,  "Can't get %s socket name: %s", nl->name,  
                strerror ( errno ) );  
        close ( sock );  
        return -1;  
    }  
  
    nl->snl = snl;  
    nl->sock = sock;  
    return ret;  
}  

static int nl_request ( int family, int type, struct nlsock *nl )  
{  
    int ret;  
    struct sockaddr_nl snl;  
  
    struct {  
        struct nlmsghdr nlh;  
        struct rtgenmsg g;  
    } req;  
  
  
    /* Check netlink socket. */  
    if ( nl->sock < 0 ) {  
        fprintf ( stderr, "%s socket isn't active.", nl->name );  
        return -1;  
    }  
  
    memset ( &snl, 0, sizeof snl );  
    snl.nl_family = AF_NETLINK;  
  
    req.nlh.nlmsg_len = sizeof req;  
    req.nlh.nlmsg_type = type;  
    req.nlh.nlmsg_flags = NLM_F_ROOT | NLM_F_MATCH | NLM_F_REQUEST;  
    req.nlh.nlmsg_pid = 0;  
    req.nlh.nlmsg_seq = ++nl->seq;  
    req.g.rtgen_family = family;  
  
    ret = sendto ( nl->sock, ( void* ) &req, sizeof req, 0,  
            ( struct sockaddr* ) &snl, sizeof snl );  
    if ( ret < 0 ) {  
        fprintf ( stderr, "%s sendto failed: %s", nl->name, strerror ( errno ) );  
        return -1;  
    }  
    return 0;  
}  

static void nl_parse_rtattr ( struct rtattr **tb, int max, struct rtattr *rta, int len )  
{  
    while ( RTA_OK ( rta, len ) ) {  
        if ( rta->rta_type <= max )  
            tb[rta->rta_type] = rta;  
        rta = RTA_NEXT ( rta,len );  
    }  
} 

static void nl_socket_close( struct nlsock *nl )
{
	if( NULL != nl && nl->sock >= 0 )
	{
		close( nl->sock );
		nl->sock = -1;
	}
}


static int nl_get_if_addr ( struct sockaddr_nl *snl, struct nlmsghdr *h, void *arg, if3 *if_node )  
{  
    int len;  
    struct ifaddrmsg *ifa;  
    struct rtattr *tb [IFA_MAX + 1];  
    //void *addr = NULL;  
    //void *broad = NULL;  
    //u_char flags = 0;  
    //char *label = NULL;  
    uint32_t ifa_addr, ifa_local;  
    char ifa_label[MAX_IF_IFNAME_LEN + 1];  
   
    ifa = NLMSG_DATA ( h );  
    if ( ifa->ifa_family != AF_INET ){  
		 FREE_OBJECT(if_node);
    	 return -1;  
    }
    if ( h->nlmsg_type != RTM_NEWADDR && h->nlmsg_type != RTM_DELADDR ){  
		FREE_OBJECT(if_node);
        return -1;  
    }
    len = h->nlmsg_len - NLMSG_LENGTH ( sizeof ( struct ifaddrmsg ) );  
    if ( len < 0 ){  
		FREE_OBJECT(if_node);
        return -1;  
    }
    memset ( tb, 0, sizeof tb );  
  
    nl_parse_rtattr ( tb, IFA_MAX, IFA_RTA ( ifa ), len );  
      
    if (tb[IFA_ADDRESS] == NULL){  
    	tb[IFA_ADDRESS] = tb[IFA_LOCAL];  
    }
    if ( tb[IFA_ADDRESS] ){  
    	 ifa_addr = *(uint32_t *) RTA_DATA ( tb[IFA_ADDRESS] );  
    }
    if ( tb[IFA_LOCAL] ){  
    	 ifa_local = *(uint32_t *) RTA_DATA ( tb[IFA_LOCAL] );  
    }
    if ( tb[IFA_LABEL] ){  
     	strncpy( ifa_label, RTA_DATA ( tb[IFA_LABEL] ), MAX_IF_IFNAME_LEN );   
    }
  
    // 储存所有地址信息   
    struct in_addr if_addr;
    struct in_addr if_mask;
     
    memcpy(&if_addr, &ifa_addr, 4);
    memcpy(&if_mask, &ifa_local, 4);
     
	if(if_node)
	{
		memset(if_node->ifname,0,sizeof(if_node->ifname));
		strncpy(if_node->ifname,ifa_label,sizeof(if_node->ifname)-1);
		memset(if_node->ipaddr,0,sizeof(if_node->ipaddr));
		strncpy(if_node->ipaddr,inet_ntoa(if_addr),sizeof(if_node->ipaddr)-1);
		if_node->mask = ifa->ifa_prefixlen;
	}
	return 0;  
}  

/* Receive message from netlink interface and pass those information 
   to the given function. */  
static int nl_parse_info ( int ( *filter ) ( struct sockaddr_nl *, struct nlmsghdr *, void *, if3 * ),  
        struct nlsock *nl, void *arg, if_list_p *iflist_t)  
{  
    int status;  
    int ret = 0;  
    int error;  
  	if3 *p = NULL,*tail = NULL;
	if(iflist_t == NULL)
	{
		return -1;  
	}
	if(iflist_t)
	{
		iflist_t->if_num = 0;
		iflist_t->if_head = (if3 *)malloc(sizeof(if3));			
		if(iflist_t->if_head)
		{
			memset(iflist_t->if_head,0,sizeof(iflist_t->if_head));
			tail = iflist_t->if_head;
		}
	}
    while ( 1 ) {  
        char buf[4096];  
        struct iovec iov = { buf, sizeof buf };  
        struct sockaddr_nl snl;  
        struct msghdr msg = { ( void* ) &snl, sizeof snl, &iov, 1, NULL, 0, 0};  
        struct nlmsghdr *h;  
  
        status = recvmsg ( nl->sock, &msg, 0 );  
  
        if ( status < 0 ) {  
            if ( errno == EINTR )  
                continue;  
            if ( errno == EWOULDBLOCK || errno == EAGAIN )  
                break;  
            fprintf ( stderr, "%s recvmsg overrun", nl->name );  
            continue;  
        }  
  
        if ( snl.nl_pid != 0 ) {  
            fprintf ( stderr, "Ignoring non kernel message from pid %u",  
                    snl.nl_pid );  
            continue;  
        }  
  
        if ( status == 0 ) {  
            fprintf ( stderr, "%s EOF", nl->name );  
            return -1;  
        }  
  
        if ( msg.msg_namelen != sizeof snl ) {  
            fprintf ( stderr, "%s sender address length error: length %d",  
                    nl->name, msg.msg_namelen );  
            return -1;  
        }  
  
        for ( h = ( struct nlmsghdr * ) buf; NLMSG_OK ( h, status );  
                h = NLMSG_NEXT ( h, status ) ) {  
            /* Finish of reading. */  
            if ( h->nlmsg_type == NLMSG_DONE )  
                return ret;  
  
            /* Error handling. */  
            if ( h->nlmsg_type == NLMSG_ERROR ) {  
                struct nlmsgerr *err = ( struct nlmsgerr * ) NLMSG_DATA ( h );  
  
                /* If the error field is zero, then this is an ACK */  
                if ( err->error == 0 ) {  
                    /* return if not a multipart message, otherwise continue */  
                    if ( ! ( h->nlmsg_flags & NLM_F_MULTI ) ) {  
                        return 0;  
                    }  
                    continue;  
                }  
  
                if ( h->nlmsg_len < NLMSG_LENGTH ( sizeof ( struct nlmsgerr ) ) ) {  
                    fprintf ( stderr, "%s error: message truncated",  
                            nl->name );  
                    return -1;  
                }  
                fprintf ( stderr, "%s error: %s, type=%u, seq=%u, pid=%d",  
                        nl->name, strerror ( -err->error ),  
                        err->msg.nlmsg_type, err->msg.nlmsg_seq,  
                        err->msg.nlmsg_pid );  
                /* 
                ret = -1; 
                continue; 
                */  
                return -1;  
            }  
  
  
            /* skip unsolicited messages originating from command socket */  
            if ( nl != &net_cmd && h->nlmsg_pid == net_cmd.snl.nl_pid ) {  
                continue;  
            }  

  			p = (if3 *)malloc(sizeof(if3));			
			memset(p,0,sizeof(if3));
            error = ( *filter ) ( &snl, h, arg, p);  
            if ( error < 0 ) {  
                fprintf ( stderr, "%s filter function error/n", nl->name );  
                ret = error;  
            }  
			else if( error == 0 )
			{
				if(p)
				{
					p->next = NULL;
					if(tail)
					{
						tail->next = p;
						tail = p;					
						if(iflist_t)
						{
							iflist_t->if_num++;
						}
					}
					else
					{
						FREE_OBJECT(p);	
					}
				}
			}
			
        }  
  
        /* After error care. */  
        if ( msg.msg_flags & MSG_TRUNC ) {  
            fprintf ( stderr, "%s error: message truncated", nl->name );  
            continue;  
        }  
        if ( status ) {  
            fprintf ( stderr, "%s error: data remnant size %d", nl->name,  
                    status );  
            return -1;  
        }  
    }  
    return ret;  
}   

void Free_get_all_if_info(if_list_p *iflist_t)
{
	int i = 0;
	if3 *f1 = NULL,*f2 = NULL;
	if(iflist_t)
	{
		f1 = iflist_t->if_head;
		if(f1)
		{			
			for(i = 0,f2 = f1->next; ((i < iflist_t->if_num)&&(NULL != f2)); i++,f2 = f2->next)
			{
				FREE_OBJECT(f1);
				f1 = f2;
			}
			FREE_OBJECT(f1);
		}
	}
}

/*获取所有三层接口的列表*/
/*返回0时，调用Free_get_all_if_info释放空间*/
int get_all_if_info( if_list_p *iflist_t )
{
	int ret = 0;  
	//char if_name[256] = { 0 };  
	//char *p = NULL;  
	struct nl_if_info if_info = { -1, " " };  

	if(iflist_t == NULL)
	{
		return -1;  
	}
	ret = nl_socket ( &net_cmd, 0 );
	if ( ret < 0 ) 
	{  
		return ret;  
	}  
	ret = nl_request ( AF_INET, RTM_GETADDR, &net_cmd );  
	if ( ret < 0 ) {  
		nl_socket_close(&net_cmd);
		return ret;  
	}
	ret = nl_parse_info ( nl_get_if_addr, &net_cmd, &if_info, iflist_t); 
	if ( ret < 0 ) {  
		nl_socket_close(&net_cmd);
		return ret;  
	}
	nl_socket_close(&net_cmd);
	return 0;
}

int get_int_from_file(char *filename)
{
	int fd;
	unsigned int data;

	if(filename == NULL)
	{
		return -1;
	}

	fd = fopen(filename, "r");
	if (fd == NULL)
	{
        printf("Open file:%s error!\n",filename);
		return -1;
	}
	
	fscanf(fd, "%d", &data);
	fclose(fd);

	return data;
}

int get_str_from_file(char *filename, char *buff)
{
	int fd;

	if((filename == NULL) || (buff == NULL))
	{
		return -1;
	}

	fd = fopen(filename, "r");
	if (fd == NULL)
	{
        printf("Open file:%s error!\n",filename);
		return -1;
	}

    fscanf(fd, "%s", buff);
    fclose(fd);
    return 0;
}
int ifname2ifindex_by_ioctl(const char *dev)
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

static int CHECK_IP_DOT(int a)
{
	if(a<0 || a>255)
		return -1;
	else return 0;
}
static int check_ip_part(char * src)
{
	int ipPart[4];
	char * endptr;
	char * token = NULL;
	char * ipSplit = ".";
	int i = 1;
	
	if( src == NULL )
	{
		return -1;
	}
	/*alloc temp memery to strtok to avoid  error when occurs in strtok*/
	char src_back[32];
	memset(src_back, 0, 32);
	strncpy(src_back, src, 32);

	token = strtok( src_back, ipSplit );
	ipPart[0] = strtoul(token, &endptr, 10);
	if( 0 != CHECK_IP_DOT(ipPart[0]) )
	{
		return -2;
	}

	
	while( token != NULL && i < 4 )
	{
		token = strtok(NULL, ipSplit);
		ipPart[i] = strtoul(token, &endptr, 10);

		if( 0 != CHECK_IP_DOT(ipPart[i]) )
		{
			return -2;
		}
		i++;
	}

	return 0;
}

int checkIpFormatValid(char *ipAddress)
{
	if( ipAddress == NULL )
	{
		return -1;
	}

	int i;
	/*int mask;
	char * endptr;
	char * strPart1, *strPart2;
	char * strSplit = "/";*/
	char * str = NULL;
	str = ipAddress ;
	int length = strlen(str);
	if( length > WEB_IPMASK_STRING_MAXLEN || length < WEB_IPMASK_STRING_MINLEN )
	{
		return -1;
	}
	
	for( i=0; i<length; i++ )
	{
		if( str[i] != '.'  && \
			str[i] != '/'  && \
			str[i] != '\0' && \
			( str[i] > '9' || str[i] < '0' )
		  )
		  return -1;
	}
		
	#if 0       //为检测MASK合法性，目前只检测IP地址
	strPart1 = NULL;
	strPart1 = strtok(str, strSplit);
	strPart2 = strtok(NULL, strSplit);
	fprintf(stderr,"strPart1=%s--strPart2=%s\n",strPart1,strPart2);
	mask = strtoul(strPart2, &endptr, 0);
	if( mask < 0 || mask > 32 )
	{
		return INPUT_IP_MASK_ERROR;
	}
	
	
	if( INPUT_OK != check_ip_part(strPart1) )
	{
		return INPUT_IP_ERROR;
	}
	#endif
	
	if( 0 != check_ip_part(str) )
	{
		return -1;
	}
	return 0;
}

int trap_name_is_legal_input(const char *str) {

	if (NULL == str || '\0' == str[0] || strlen(str) > (MAX_SNMP_NAME_LEN - 1))
		return 0;

	const char *p = NULL;
	for (p = str; *p; p++)
		if (!isgraph(*p))
			return 0;
		
	return 1;
}

int snmp_ipaddr_is_legal_input(const char *str) {

	char *endptr = NULL;
	char* endptr1 = NULL;
	char c;
	int IP, i;
	
	c = str[0];
	if (c>='0'&&c<='9'){
		IP= strtoul(str,&endptr,10);
		if(IP < 0||IP > 255)
			return 0;
		else if(((IP < 10)&&((endptr - str) > 1))||((IP < 100)&&((endptr - str) > 2))||((IP < 256)&&((endptr - str) > 3)))
			return 0;
		for(i = 0; i < 3; i++){
			if(endptr[0] == '\0'||endptr[0] != '.')
				return 0;
			else{
				endptr1 = &endptr[1];
				IP= strtoul(&endptr[1],&endptr,10); 			
				if(IP < 0||IP > 255)
					return 0;				
				else if(((IP < 10)&&((endptr - endptr1) > 1))||((IP < 100)&&((endptr - endptr1) > 2))||((IP < 256)&&((endptr - endptr1) > 3)))
					return 0;
			}
		}
		if(endptr[0] == '\0' && IP >= 0)
			return 1;
		else
			return 0;
	}
	else
		return 0;		
}

int
ve_interface_parse(const char *str, char *ifname, unsigned int size) {
	unsigned int slotno;
	char *cursor, *endp;
	
	if (!str || !ifname || !size)
		return -1;

    if (strncmp(str, "ve", 2)) 
		return -1;

	cursor = str + 2;
	if (cursor[0] > '9' || cursor[0] < '0')
		return -1;

	memset(ifname, 0, size);
	slotno = strtoul(cursor, &endp, 10);
	if (endp && '.' == endp[0]) {
		snprintf(ifname, size, "ve%02uf1%s", slotno, endp);			
	} else {
		strncpy(ifname, str, size - 1);
	}

	return 0;
}


void
snmp_pid_write(const char *pid_file, const char *thread_desc) {

    if(NULL == pid_file || NULL == thread_desc) {
        return ;
    }
    
	FILE *fp = NULL;
	pid_t thread_pid = 0;
    char pidBuf[256] = { 0 };
	
	fp = fopen(pid_file, "a+");
	if(NULL == fp) {
        return ;
	}
	
	thread_pid = getpid();	
	snprintf(pidBuf, sizeof(pidBuf) - 1, "%s: %d\n", thread_desc, thread_pid);

	fputs(pidBuf, fp);
	fclose(fp);
	
	return ;
}

int ac_trap_get_flag(char *fpath)
{
	FILE *fp = NULL;
	int debug_em_switch = 0;
	
	fp = fopen(fpath, "r");
	if (fp)
	{
		fscanf(fp, "%d", &debug_em_switch);
		fclose(fp);		
	}
	
	return debug_em_switch;
}

void ac_trap_set_flag(char *fpath,int debug_em_switch)
{
	FILE *fp = NULL;
	if ( (fp = fopen(fpath, "w")) != NULL) {
		fprintf(fp, "%d", debug_em_switch);
		fclose(fp);
	}
}
void ac_set_vrrp_preempt(int state,int interval)
{
	FILE *fp = NULL;

	if ( (fp = fopen("/var/run/preempt_state", "w")) != NULL) {
		fprintf(fp, "%d", state);
		fclose(fp);
	}
	fp = NULL;
	if ( (fp = fopen("/var/run/preempt_interval", "w")) != NULL) {
		fprintf(fp, "%d", interval);
		fclose(fp);
	}
}
int ac_preempt_switch_trap_has_sent(void)
{
	 FILE *fd =NULL;
	int state = 0;
	int interval = 20;
	fd = fopen("/var/run/preempt_state", "r");
	if (fd == NULL)
	{
		return 0;
	}
	fscanf(fd, "%d", &state);
	fclose(fd);
	fd = NULL;
	fd = fopen("/var/run/preempt_interval", "r");
	if (fd == NULL)
	{
		return 0;
	}
	fscanf(fd, "%d", &interval);
	fclose(fd);

	return state;
}
int ac_preempt_interval_trap_has_sent(void)
{
	 FILE *fd =NULL;
	int interval = 20;
	fd = fopen("/var/run/preempt_interval", "r");
	if (fd == NULL)
	{
		return 0;
	}
	fscanf(fd, "%d", &interval);
	fclose(fd);

	return interval;
}

