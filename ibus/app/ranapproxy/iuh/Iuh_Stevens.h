

#ifndef _IUH_DBUS_Stevens_HEADER__
#define _IUH_DBUS_Stevens_HEADER__

#include <net/if.h>
#include <sys/ioctl.h>


#define	IFI_NAME	(16)			/* same as IFNAMSIZ in <net/if.h> */

#ifndef IFI_HADDR
#define	IFI_HADDR	 (8)			/* allow for 64-bit EUI-64 in future */
#endif

#define	IFI_ADDR_NUM	(8)

struct ifi_info {
  char    ifi_name[IFI_NAME];	/* interface name, null-terminated */
  short   ifi_index;			/* interface index */
  short   ifi_index_binding; //added by weiay 20080610
  short   ifi_flags;			/* IFF_xxx constants from <net/if.h> */
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

struct ifi {
  char    ifi_name[IF_NAME_LEN];	/* interface name, null-terminated */
  short   ifi_index;			/* interface index */
  unsigned int nas_id_len;
  char nas_id[NAS_IDENTIFIER_NAME];
  char isipv6addr;
  struct ifi  *ifi_next;	/* next of these structures */
};

struct ifi_info* get_ifi_info(int, int);
void free_ifi_info(struct ifi_info *);
char *sock_ntop_r(const struct sockaddr *sa, char *str);
char *sock_ntop_r1(const struct sockaddr *sa, char *str);
int sock_cpy_addr_port(struct sockaddr *sa1, const struct sockaddr *sa2);
void sock_set_port_cw(struct sockaddr *sa, int port);
int sock_cmp_addr(const struct sockaddr *sa1, const struct sockaddr *sa2, socklen_t salen);

#endif	/* __CAPWAP_CWStevens_HEADER__ */
