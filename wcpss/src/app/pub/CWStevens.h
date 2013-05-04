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
* CWStevens.c
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


#ifndef __CAPWAP_CWStevens_HEADER__
#define __CAPWAP_CWStevens_HEADER__
 
#include <net/if.h>
#include <sys/ioctl.h>

#define	IFI_NAME	16			/* same as IFNAMSIZ in <net/if.h> */
#define	IFI_HADDR	 8			/* allow for 64-bit EUI-64 in future */
#define	IFI_ADDR_NUM	8
#define	IFI_IF_NUM	8

#define IPV6 1
#define IPv6 1
struct ifi_info {
  char    ifi_name[IFI_NAME];	/* interface name, null-terminated */
  short   ifi_index;			/* interface index */
  short   ifi_index_binding; //added by weiay 20080610
  short   ifi_flags;			/* IFF_xxx constants from <net/if.h> */
  short	  ifi_bflags;          /*wuwl add,the same interface cannot be bingded in two or more hansi.*/
  struct sockaddr  *ifi_addr;	/* primary address */
  struct sockaddr  *ifi_addr6;	/* primary address */
  struct sockaddr  *ifi_brdaddr;/* broadcast address */
  struct ifi_info  *ifi_next;	/* next of these structures */
  unsigned int	addr[IFI_ADDR_NUM];
  unsigned int	check_addr[IFI_ADDR_NUM];
  unsigned int	check_brdaddr;
  unsigned int	addr_num;
  unsigned char mac[6];
};

struct ifi_info* get_ifi_info(int, int);
void free_ifi_info(struct ifi_info *);
char *sock_ntop_r(const struct sockaddr *sa, char *str);
char *sock_ntop_r1(const struct sockaddr *sa, char *str);
int sock_cpy_addr_port(struct sockaddr *sa1, const struct sockaddr *sa2);
void sock_set_port_cw(struct sockaddr *sa, int port);
int sock_cmp_addr(const struct sockaddr *sa1, const struct sockaddr *sa2, socklen_t salen);
int mcast_join(int sockfd, const struct sockaddr *grp, socklen_t grplen, const char *ifname, u_int ifindex);

#endif	/* __CAPWAP_CWStevens_HEADER__ */
