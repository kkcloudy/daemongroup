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
* dcli_iptunnel.c
*
* MODIFY:
*
* CREATOR:
*		shancx@autelan.com
*
* DESCRIPTION:
*		CLI definition for ipv6 tunnel module.
*
* DATE:
*		09/22/2009	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.5 $	
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zebra.h>
#include <unistd.h>
#include <syslog.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <asm/byteorder.h>
#include "command.h"
#define ARPHRD_SIT	776		/* sit0 device - IPv6-in-IPv4	*/
#define SPRINT_BSIZE 64
#define SPRINT_BUF(x)	char x[SPRINT_BSIZE]

#ifndef __constant_htons
#define __constant_htons(x)  htons(x)
#endif

#include <linux/if_tunnel.h>

int show_stats = 0;
char* _SL_="\n";



static int do_ioctl_get_ifindex(const char *dev)
{
	struct ifreq ifr;
	int fd;
	int err;

	memset(&ifr,0,sizeof(ifr));
	strncpy(ifr.ifr_name, dev, sizeof(ifr.ifr_name));
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	err = ioctl(fd, SIOCGIFINDEX, &ifr);
	if (err) {
//		perror("do_ioctl_get_ifindex");
		close(fd);
		return 0;
	}
	close(fd);
	return ifr.ifr_ifindex;
}

static int do_ioctl_get_iftype(const char *dev)
{
	struct ifreq ifr;
	int fd;
	int err;

	strncpy(ifr.ifr_name, dev, IFNAMSIZ);
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	err = ioctl(fd, SIOCGIFHWADDR, &ifr);
	if (err) {
/*		perror("ioctl");*/
		close(fd);
		return -1;
	}
	close(fd);
	return ifr.ifr_addr.sa_family;
}


static char * do_ioctl_get_ifname(int idx)
{
	static struct ifreq ifr;
	int fd;
	int err;

	ifr.ifr_ifindex = idx;
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	err = ioctl(fd, SIOCGIFNAME, &ifr);
	if (err) {
		/*		perror("ioctl");*/
				close(fd);
		return NULL;
	}
	close(fd);
	return ifr.ifr_name;
}


static int do_get_ioctl(const char *basedev, struct ip_tunnel_parm *p)
{
	struct ifreq ifr;
	int fd;
	int err;

	strncpy(ifr.ifr_name, basedev, IFNAMSIZ);
	ifr.ifr_ifru.ifru_data = (void*)p;
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	err = ioctl(fd, SIOCGETTUNNEL, &ifr);
/*
	if (err)
		perror("ioctl");
*/
	close(fd);
	return err;
}

static int do_add_ioctl(int cmd, const char *basedev, struct ip_tunnel_parm *p)
{
	struct ifreq ifr;
	int fd;
	int err;

	memset(&ifr,0,sizeof(ifr));
	if (cmd == SIOCCHGTUNNEL && p->name[0])
		strncpy(ifr.ifr_name, p->name, IFNAMSIZ);
	else
		strncpy(ifr.ifr_name, basedev, IFNAMSIZ);
	ifr.ifr_ifru.ifru_data = (void*)p;
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	
	err = ioctl(fd, cmd, &ifr);
	
	if (err)
		perror("do_add_ioctl");
	close(fd);
	if(cmd == SIOCADDTUNNEL)
	{
		char syscmd[64]={0};
		sprintf(syscmd,"sudo ifconfig %s up",p->name);
		system(syscmd);
	}
	return err;
}

static int do_del_ioctl(const char *basedev, struct ip_tunnel_parm *p)
{
	struct ifreq ifr;
	int fd;
	int err;

	if (p->name[0])
		strncpy(ifr.ifr_name, p->name, IFNAMSIZ);
	else
		strncpy(ifr.ifr_name, basedev, IFNAMSIZ);
	ifr.ifr_ifru.ifru_data = (void*)p;
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	err = ioctl(fd, SIOCDELTUNNEL, &ifr);
	if (err)
		perror("ioctl");
	close(fd);
	return err;
}

void print_tunnel(struct vty *vty,struct ip_tunnel_parm *p)
{

	char s1[1024];
	char s2[1024];
	char s3[64];
	char s4[64];

	inet_ntop(AF_INET, &p->i_key, s3, sizeof(s3));
	inet_ntop(AF_INET, &p->o_key, s4, sizeof(s4));

	/* Do not use format_host() for local addr,
	 * symbolic name will not be useful.
	 */
	vty_out(vty,"%s: %s/ip  remote %s  local %s ",
	       p->name,
	       p->iph.protocol == IPPROTO_IPIP ? "ip" :
	       (p->iph.protocol == IPPROTO_GRE ? "gre" :
		(p->iph.protocol == IPPROTO_IPV6 ? "ipv6" : "unknown")),
	       p->iph.daddr ? inet_ntop(AF_INET, &p->iph.daddr, s1, sizeof(s1))  : "any",
	       p->iph.saddr ? inet_ntop(AF_INET, &p->iph.saddr, s2, sizeof(s2)) : "any");

	if (p->link) {
		char *n = do_ioctl_get_ifname(p->link);
		if (n)
			vty_out(vty," dev %s ", n);
	}

	if (p->iph.ttl)
		vty_out(vty," ttl %d ", p->iph.ttl);
	else
		vty_out(vty," ttl inherit ");
	
	if (p->iph.tos) {
		SPRINT_BUF(b1);
		vty_out(vty," tos");
		if (p->iph.tos&1)
			vty_out(vty," inherit");
/*
		if (p->iph.tos&~1)
			vty_out(vty,"%c%s ", p->iph.tos&1 ? '/' : ' ',
			       rtnl_dsfield_n2a(p->iph.tos&~1, b1, sizeof(b1)));
*/
	}

	if (!(p->iph.frag_off&htons(IP_DF)))
		vty_out(vty," nopmtudisc");


}

static int do_tunnels_list(struct ip_tunnel_parm *p)
{
	return 0;
}

static int do_show(int argc, char **argv)
{
	return 0;
}
int ipv6_tunnel_show_running(struct vty *vty)
{
	return 1;
}
DEFUN(ipv6_tunnel_add_cmd_func,
	ipv6_tunnel_add_cmd,
	"ipv6 tunnel (add|delete) NAME remote (A.B.C.D|any) local A.B.C.D [IFNAME]",
	IP6_STR
	"Config IPV6 tunnel\n"
	"Add IPV6 tunnel\n"
	"Delete IPV6 tunne\n"
	"The tunnel name\n"
	"Tunnel remote end\n"
	"Tunnel remote end ip address\n"
	"Tunnel remote any ip address\n"
	"Tunnel local end\n"
	"Tunnel local end ip address\n"
	"The interface name\n"
	)
{
	struct ip_tunnel_parm p;
	int cmd;
	struct in_addr tmp_addr;
	

	memset(&p,0,sizeof(p));
	
	p.iph.protocol = IPPROTO_IPV6;
	p.iph.version = 4;
	p.iph.ihl = 5;
#ifndef IP_DF
#define IP_DF		0x4000		/* Flag: "Don't Fragment"	*/
#endif
	p.iph.frag_off = htons(IP_DF);

	p.iph.tos = 1;


	if(!strncmp(argv[0],"add",strlen(argv[0])))
		cmd = SIOCADDTUNNEL;	
	else if(!strncmp(argv[0],"delete",strlen(argv[0])))
		cmd = SIOCDELTUNNEL;	
	else if(!strncmp(argv[0],"change",strlen(argv[0])))
		cmd = SIOCDELTUNNEL;	
	
	strncpy(p.name, argv[1], IFNAMSIZ);
	if(!strncmp(argv[2],"any",3))
	{
			p.iph.daddr=0;
	}
	else if(inet_aton(argv[2],&tmp_addr))
	{
		p.iph.daddr=tmp_addr.s_addr;
	}
	else
	{
		vty_out(vty,"Remote ip address is wrong\n");
		return CMD_WARNING;

	}
	
	if(!inet_aton(argv[3],&tmp_addr))
	{
		vty_out(vty,"Local ip address is wrong\n");
		return CMD_WARNING;
	}
	p.iph.saddr =tmp_addr.s_addr;
	
	if(argc==5)
	p.link = do_ioctl_get_ifindex(argv[4]);
/*
	if (p.link == 0)
		return CMD_WARNING;
*/
	
	if (IN_MULTICAST(ntohl(p.iph.daddr)) && !p.iph.saddr) {
		vty_out(vty, "Broadcast tunnel requires a source address.\n");
		return CMD_WARNING;
	}

	
	return do_add_ioctl(cmd, "sit0", &p);
}

DEFUN(ipv6_tunnel_show_cmd_func,
	ipv6_tunnel_show_cmd,
	"show ipv6 tunnel [NAME]",
	SHOW_STR
	IP6_STR
	"IPV6 tunnel\n"
	"The tunnel name\n"
	)
{
	char name[IFNAMSIZ];
	unsigned long  rx_bytes, rx_packets, rx_errs, rx_drops,
	rx_fifo, rx_frame,
	tx_bytes, tx_packets, tx_errs, tx_drops,
	tx_fifo, tx_colls, tx_carrier, rx_multi;
	int type,tunnel_num=0;
	struct ip_tunnel_parm p1;
	char buf[512];
	FILE *fp=NULL;

  if(argc==1)
	{
		type = do_ioctl_get_iftype(argv[0]);
		if (type == -1) {
			vty_out(vty, "Failed to get type of [%s]\n", argv[0]);
			return CMD_WARNING;
		}
		if ( type != ARPHRD_SIT){
			vty_out(vty, "The dev [%s] isn't ipv6-in-ip tunnel\n", argv[0]);
			return CMD_WARNING;
		}
		memset(&p1, 0, sizeof(p1));
		if (do_get_ioctl(name, &p1)){
			print_tunnel(vty,&p1);
			return 	CMD_SUCCESS;
		}
		else{
			vty_out(vty, "Failed to get infomation of [%s]\n", argv[0]);
			return CMD_WARNING;
		}
	}

	fp = fopen("/proc/net/dev", "r");

	if (fp == NULL) {
		vty_out(vty,"System error,cann't get tunnel list!");
		return CMD_WARNING;
	}

	fgets(buf, sizeof(buf), fp);
	fgets(buf, sizeof(buf), fp);

	while (fgets(buf, sizeof(buf), fp) != NULL) {
		char *ptr;
		buf[sizeof(buf) - 1] = 0;
		if ((ptr = strchr(buf, ':')) == NULL ||
				(*ptr++ = 0, sscanf(buf, "%s", name) != 1)) {
			vty_out(vty, "Wrong format of system file. Sorry.\n");
			fclose(fp);
			return CMD_WARNING;
		}
		type = do_ioctl_get_iftype(name);
		if (type == -1) {
			vty_out(vty, "Failed to get type of [%s]\n", name);
			continue;
		}
		if ( type != ARPHRD_SIT)
			continue;
		memset(&p1, 0, sizeof(p1));
		if (do_get_ioctl(name, &p1))
			continue;
		print_tunnel(vty,&p1);
		tunnel_num++;
		vty_out(vty,"\n");

	}
	fclose(fp);
	vty_out(vty,"There are %d tunnel total\n",tunnel_num);
	return CMD_SUCCESS;
}
int dcli_iptunnel_showrunning(struct vty* vty)
{
		char name[IFNAMSIZ];
		unsigned long  rx_bytes, rx_packets, rx_errs, rx_drops,
		rx_fifo, rx_frame,
		tx_bytes, tx_packets, tx_errs, tx_drops,
		tx_fifo, tx_colls, tx_carrier, rx_multi;
		int type,tunnel_num=0;
		struct ip_tunnel_parm p1;
		char buf[512];
		FILE *fp=NULL;
	
		fp = fopen("/proc/net/dev", "r");
	
		if (fp == NULL) {
			fprintf(stdout,"System error,cann't get tunnel list!");
			return CMD_WARNING;
		}
	
		fgets(buf, sizeof(buf), fp);
		fgets(buf, sizeof(buf), fp);
	
		while (fgets(buf, sizeof(buf), fp) != NULL) {
			char *ptr;
			buf[sizeof(buf) - 1] = 0;
			if ((ptr = strchr(buf, ':')) == NULL ||
					(*ptr++ = 0, sscanf(buf, "%s", name) != 1)) {
				fprintf(stdout, "Wrong format of system file. Sorry.\n");
				fclose(fp);
				return CMD_WARNING;
			}
			if(!strcmp(name,"sit0"))
				continue;
			type = do_ioctl_get_iftype(name);
			if (type == -1) {
				fprintf(stdout, "Failed to get type of [%s]\n", name);
				continue;
			}
			if ( type != ARPHRD_SIT )
				continue;
			memset(&p1, 0, sizeof(p1));
			if (do_get_ioctl(name, &p1))
				continue;
#if 0			
			print_tunnel(vty,&p1);
#else
			{

				char s1[1024];
				char s2[1024];
				char s3[64];
				char s4[64];
				char tmp_str[2048];
				char *dev_name;

				inet_ntop(AF_INET, &p1.i_key, s3, sizeof(s3));
				inet_ntop(AF_INET, &p1.o_key, s4, sizeof(s4));

				/* Do not use format_host() for local addr,
				 * symbolic name will not be useful.
				 */
				memset(tmp_str,0,2048);
				
				if (p1.link) {
					dev_name = do_ioctl_get_ifname(p1.link);
				}
				
				sprintf(tmp_str,"ipv6 tunnel add %s remote %s local %s %s",
				       p1.name,
				       p1.iph.daddr ? inet_ntop(AF_INET, &p1.iph.daddr, s1, sizeof(s1))  : "any",
				       p1.iph.saddr ? inet_ntop(AF_INET, &p1.iph.saddr, s2, sizeof(s2)) : "any",
				       dev_name ? dev_name : "");

			vtysh_add_show_string(tmp_str);


			}

#endif
	
		}
		fclose(fp);
		return CMD_SUCCESS;

}

struct cmd_node ipv6_tunnel_node =
{
  IP_TUNNEL_NODE,
  "	",
  1,
};



void dcli_iptunnel_init()
{
	
	install_node(&ipv6_tunnel_node,dcli_iptunnel_showrunning,"IP_TUNNEL_NODE");

	install_element(CONFIG_NODE,  &ipv6_tunnel_add_cmd);
	install_element(CONFIG_NODE,  &ipv6_tunnel_show_cmd);
	install_element(ENABLE_NODE,  &ipv6_tunnel_show_cmd);

	return;
}
