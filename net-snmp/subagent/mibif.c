#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <errno.h>

#include <linux/types.h>   
#include <asm/types.h>   
#include <inttypes.h>   
#include <sys/file.h>   
#include <sys/user.h>   
#include <sys/socket.h>   
#include <linux/netlink.h>   
#include <linux/rtnetlink.h>   
#include <linux/if.h>   
#include <unistd.h>   
#include <stdlib.h>   
#include <string.h>   
#include <stdio.h>   
#include <stdbool.h>   
#include <errno.h>   
  
#include "mibif.h"  
  
typedef uint32_t u32;   
typedef uint16_t u16;   


#define VLAN_INFO_CONF_PATH		"/opt/services/conf/vlaninfo_conf.conf"
#define VLAN_INFO_STATU_PATH	"/opt/services/status/vlaninfo_status.status"


struct nlsock {   
    int sock;   
    int seq;   
    struct sockaddr_nl snl;   
    char *name;   
} nl_cmd = { -1, 0, {0}, "netlink-cmd" };
  
static int index_oif = 0;   
struct nl_if_info {   
    u32 addr;   
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
  
/* Receive message from netlink interface and pass those information  
   to the given function. */  
static int  
nl_parse_info ( int ( *filter ) ( struct sockaddr_nl *, struct nlmsghdr *, void * ),   
        struct nlsock *nl, void *arg )   
{   
    int status;   
    int ret = 0;   
    int error;   
  
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
            if ( nl != &nl_cmd && h->nlmsg_pid == nl_cmd.snl.nl_pid ) {   
                continue;   
            }   
  
            error = ( *filter ) ( &snl, h, arg );   
            if ( error < 0 ) {   
                fprintf ( stderr, "%s filter function error\n", nl->name );   
                ret = error;   
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
  
static void nl_parse_rtattr ( struct rtattr **tb, int max, struct rtattr *rta, int len )   
{   
    while ( RTA_OK ( rta, len ) ) {   
        if ( rta->rta_type <= max )   
            tb[rta->rta_type] = rta;   
        rta = RTA_NEXT ( rta,len );   
    }   
}
  

static int nl_get_if_dev ( struct sockaddr_nl *snl, struct nlmsghdr *h, void *arg )   
{   
    int len;   
    /*struct ifaddrmsg *ifa; */
    struct ifinfomsg *ifi;
    struct rtattr *tb [IFA_MAX + 1];    

   	   
    ifi = NLMSG_DATA ( h );  

    len = h->nlmsg_len - NLMSG_LENGTH ( sizeof ( struct ifaddrmsg ) );   
    if ( len < 0 )   
        return -1;
  
    memset ( tb, 0, sizeof tb );   
    nl_parse_rtattr ( tb, IFLA_MAX, IFLA_RTA ( ifi ), len ); 
    
    MIBIF_PRINTF("IFLA_IFNAME = %s\n", tb[IFLA_IFNAME]?RTA_DATA ( tb[IFLA_IFNAME] ):"error");

   
    return 0;   
}


static void nl_socket_close( struct nlsock *nl )
{
	if( NULL != nl && nl->sock >= 0 )
	{
		close( nl->sock );
		nl->sock = -1;
	}
}


static int diag_gindex_handle
(
	char *name,
	int *gindex,
	char opType
)
{
	int iret = 0;
	struct ifreq	ifr;
	struct sockaddr iaddr;
	int fd = -1;
	int	iocmd = 0;
	
	if((!name)||
		((0 != opType) && !gindex)){
		//printf("11111\n");
		return -1;
	}

	//printf("name = %s  gindex=%d  optype=%d\n", name, *gindex, opType );
	memset(&ifr, 0, sizeof(struct ifreq));
	memset(&iaddr, 0, sizeof(struct sockaddr));

	/* init socket */
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) {
		//printf("22222\n");		
		return -2;
	}

	/* build up ioctl arguments */
	strncpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));
	
	if(0 == opType) {
		iocmd = SIOCGGINDEX;
	}
	else if(1 == opType) {
		ifr.ifr_ifindex = *gindex;
		iocmd = SIOCSGINDEX;
	}
	else {
		close( fd );
		//printf("3333\n");		
		return -3;
	}

	/* do I/O */
	if (ioctl(fd, iocmd, (char *)&ifr)){
				//printf("4444\n");
		iret = -4;
	}
	else {
				//printf("ok\n");
		iret = 0;
		*gindex = ifr.ifr_ifindex;
	}
	
	close(fd);
	return iret;
	
}

int set_mibif_gindex( mibif *mif, unsigned int gindex )
{
	return diag_gindex_handle( mif->ifname, &gindex, 1 );
}
static unsigned int get_mibif_gindex( mibif *mif )
{
	int gindex = 0;
	
	diag_gindex_handle( mif->ifname, &gindex, 0 );

	printf("get_mibif_gindex = %d\n", gindex );
	return gindex;
}

static int get_mibif_dev( struct sockaddr_nl *snl, struct nlmsghdr *h, void *arg )
{   
    int len;   
    struct ifinfomsg *ifi;
    struct rtattr *tb [IFLA_MAX + 1]; 
    mibiflst *iflst=arg;
    struct ifreq ifr;
    short flags;
    unsigned long  mask;
    int udp_socket = 0;

	MIBIF_PRINTF("get_if_dev\n");
	
	if( NULL == iflst )
   	{
   		MIBIF_PRINTF("iflst = NULL\n");
   		return -1;
   	}
    ifi = NLMSG_DATA ( h );
    len = h->nlmsg_len - NLMSG_LENGTH ( sizeof ( struct ifaddrmsg ) );   
    if ( len < 0 )   
    {
    	MIBIF_PRINTF("len < 0\n");
        return -1;
    }

    memset ( tb, 0, sizeof tb );   
    nl_parse_rtattr ( tb, IFLA_MAX, IFLA_RTA ( ifi ), len );
      
    if( NULL == iflst->lst )
    {
    	iflst->lst = (mibif *)malloc(32*sizeof(mibif));
    	if( NULL == iflst->lst )
    	{
    		MIBIF_PRINTF("iflst->lst=NULL\n");
    		return -1;
    	}
    	memset( iflst->lst, 0, 32*sizeof(mibif) );
    	iflst->mibif_num = 0;
    	iflst->buf_mibif_num = 32;
    }
    else if( iflst->mibif_num == iflst->buf_mibif_num )
    {
    	mibif *temp;
    	temp = (mibif*)realloc( iflst->lst, iflst->buf_mibif_num*2*sizeof(mibif) );
    	if( NULL == iflst->lst )
    	{
    		MIBIF_PRINTF("iflst->lst=NULL\n");
    		return -1;
    	}
    	iflst->buf_mibif_num *=2;
    	iflst->lst = temp;
    }

    if( !tb[IFLA_IFNAME] )
    {
    	MIBIF_PRINTF("has no IFLA_IFNAME\n");
    	return -1;
    }

    if( strstr( RTA_DATA ( tb[IFLA_IFNAME] ), "radio") != NULL )
    {
    	MIBIF_PRINTF("it radio if!\n");
    	return -1;
    }
	MIBIF_PRINTF("tb[IFLA_IFNAME] = %p\n", tb[IFLA_IFNAME]);
	MIBIF_PRINTF("tb[IFLA_IFNAME] = %s\n", RTA_DATA ( tb[IFLA_IFNAME] ) );
	MIBIF_PRINTF("iflst->lst= %p  mibif_num = %d\n", iflst->lst, iflst->mibif_num  );
    strncpy( iflst->lst[iflst->mibif_num].ifname, RTA_DATA ( tb[IFLA_IFNAME] ), 
					sizeof(iflst->lst[iflst->mibif_num].ifname)-1 );
	MIBIF_PRINTF("qqqqqq\n");
    iflst->lst[iflst->mibif_num].index = ifi->ifi_index;
    
	bcopy(iflst->lst[iflst->mibif_num].ifname, ifr.ifr_name, sizeof(ifr.ifr_name)-1);
	/*get ip & mask*/
		
	if ((udp_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		return 0;
	}
#if 1	/*may not use flags*/
	if (ioctl(udp_socket, SIOCGIFFLAGS, (char *)&ifr) < 0)
	{
		close(udp_socket);
	    return 0;
	}
	flags = ifr.ifr_flags;
	if ((flags & IFF_LOOPBACK) == IFF_LOOPBACK )
	{
		close(udp_socket);
		return 0;
	}
#endif	
	if (ioctl(udp_socket, SIOCGIFADDR, (char *)&ifr) < 0) {
		mask = 0;
	} else {
	    mask = ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr;
	}
	iflst->lst[iflst->mibif_num].ipaddr = mask;	
	
	if (ioctl(udp_socket, SIOCGIFNETMASK, (char *)&ifr) < 0) {
		mask = 0xffffffff;
	} else {
	    mask = ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr;
	}
	close( udp_socket );
	iflst->lst[iflst->mibif_num].mask = mask;


	/*get gindex of if*/
	iflst->lst[iflst->mibif_num].gindex = 
				get_mibif_gindex( &(iflst->lst[iflst->mibif_num]) );
	
	iflst->mibif_num++;
    MIBIF_PRINTF("IFLA_IFNAME = %s   index = %d\n\n", RTA_DATA ( tb[IFLA_IFNAME] ), ifi->ifi_index );
    return 0;   
}


/*0、得到当前所有三层接口的列表,不显示radio口*/
int get_all_mibif( mibiflst *iflst )/*使用完成后需要释放!!!!  返回值为if个数*/
{
	int ret;

	if( nl_socket ( &nl_cmd, 0 ) < 0 )
	{
		return -1;
	}

	ret = nl_request( AF_INET, RTM_GETLINK, &nl_cmd );
	if ( ret < 0 ) {
		nl_socket_close( &nl_cmd );
		return ret;
	}

	nl_parse_info ( get_mibif_dev, &nl_cmd, iflst );

	nl_socket_close( &nl_cmd );
	return iflst->mibif_num;
}

const char *main_mibif_name_list[] = {
	"eth0-1",
	"eth0-2",
	"eth0-3",
	"eth0-4"
};

#define MAIN_MIBIF_NUM	sizeof(main_mibif_name_list)/sizeof(main_mibif_name_list[0])
int if_main_mibif( const char *name )
{
	int i;
	
	for( i=0; i<MAIN_MIBIF_NUM; i++ )
	{
		if( strcmp(name,main_mibif_name_list[i]) == 0 )
		{
			return 1;	
		}			
	}
	
	return 0;
}


/*2、得到当前接口是否为子接口*/
/*同上，也是根据ifname得来的，当前只能针对7000，其它设备都得不到*/
int if_sub_mibif( const char *name )
{
	char *ifname=NULL;
	char *vlanid=NULL;
	
	ifname = (char *)malloc( strlen(name)+2);
	if( NULL == ifname )
	{
		return 0;	
	}
	vlanid = strchr( name, '.' );
	if( NULL != vlanid )
	{
		return 1;
	}

	return 0;	
}

int get_sub_mibif_vlanid( const char *name )
{
	char *vlanid=NULL;
	
	vlanid = strchr( name, '.' );
	if( NULL != vlanid )
	{
		return strtoul( vlanid+1, NULL, 10 );
	}

	return 0;	
}

int get_sub_mibif_mainif( const char *name, char *mainif, int len )
{
	char *vlanid=NULL;
	
	strncpy( mainif, name, len-1 );
	vlanid = strchr( mainif, '.' );
	if( NULL != vlanid )
	{
		*vlanid = 0;
	}
	else
	{
		return -1;
	}

	return 0;	
}

static int curnum = 0;
static char *vlaninfo[4096];

static int init_vlan_info( )
{
	FILE *fp=NULL;
	char buff[1024];
	
	curnum = 0;
	fp = fopen( VLAN_INFO_CONF_PATH, "r" );
	if( NULL == fp )
	{
		printf( "fopen %s error!\n", VLAN_INFO_CONF_PATH );
		return 0;	
	}

	
	memset( vlaninfo, 0, sizeof(vlaninfo) );
	
	memset( buff, 0, sizeof(buff) );
	fseek( fp, 0, SEEK_END );
	printf("fp=%x  tell=%d\n",fp, ftell(fp ));
	fseek( fp, 0, SEEK_SET );
	while( fgets(buff, sizeof(buff), fp) != NULL )
	{
		printf("buff = %s curnum = %d\n",buff, curnum);
		if( strlen(buff) <= 2 )
		{
			continue;
		}
		
		vlaninfo[curnum] = (char *)malloc( 1024 );
		if( NULL == vlaninfo[curnum] )
		{
			continue;
		}
		printf("buff = %s\n",buff);
		strncpy( vlaninfo[curnum], buff, 1023 );
		curnum ++;
	}
	
	fclose( fp );
	
	printf("load ok! curnum=%d\n", curnum);
	return 0;
}

static int set_vlan_info( int vlanid, char *info )
{

	int i = 0;
	int flag = 0;
	char vlanstr[32];
	
	sprintf( vlanstr, "%d", vlanid );
		
	for( i = 0; i<curnum; i++ )
	{	
		if( strncmp( vlaninfo[i], vlanstr, strlen(vlanstr)) == 0 &&
				vlaninfo[i][strlen(vlanstr)] == '#' )/*mod*/
		{
			flag = 1;
			sprintf( vlaninfo[i], "%d#%s\n", vlanid, info );
			break;
		}
	}
	
	if( 1 != flag )/*add new*/
	{
		vlaninfo[curnum] = (char *)malloc( 1024 );
		if( NULL == vlaninfo[curnum] )
		{
			printf("malloc failed!\n");
			return -1;	
		}
		sprintf( vlaninfo[curnum], "%d#%s\n", vlanid, info );
		curnum++;
	}
	
	printf("set ok xxx!\n");
	return 0;
}

static int get_vlan_info( int vlanid, char *info, int size )
{
	char *info_get = NULL;
	int i;
	int ret = -1;
	char vlanstr[32];

	sprintf( vlanstr, "%d", vlanid );
	for( i = 0; i<curnum; i++ )
	{	
		if( (strncmp( vlaninfo[i], vlanstr, strlen(vlanstr) )== 0) &&
				('#' == vlaninfo[i][strlen(vlanstr)]) )
		{
			info_get = strchr( vlaninfo[i], '#' );
			strncpy( info, info_get+1, size-1 );
			ret = 0;
			break;
		}
	}

	for( ;(*info != 0x0a) && (*info !=0x0d) && (*info != 0); info++ );
	*info = 0;
	printf("get okfff!\n");
	return ret;
}

static int del_vlan_info( int vlanid )
{
	int i;
	char vlanstr[32];

	sprintf( vlanstr, "%d", vlanid );	
	for( i = 0; i<curnum; i++ )
	{	
		if( strncmp( vlaninfo[i], vlanstr, strlen(vlanstr)) == 0 &&
				vlaninfo[i][strlen(vlanstr)] == '#' )
		{
			break;
		}
	}

	if( i != curnum )
	{
		if( NULL != vlaninfo[i] )
		{
			free( vlaninfo[i] );
		}
		curnum--;
		memcpy( vlaninfo+i, vlaninfo+(i+1), sizeof(vlaninfo)-i*sizeof(vlaninfo[0]) );
	}
	printf("del ok!\n");
	return 0;		
}

static int save_vlan_info()
{
	FILE *fp=NULL;
	int i;

	fp = fopen( VLAN_INFO_CONF_PATH, "w+" );
	if( NULL == fp )
	{
		return -1;
	}
	
	for( i=0; i<curnum; i++ )
	{
		if( strlen(vlaninfo[i]) > 0 )
		{
			fwrite( vlaninfo[i], strlen(vlaninfo[i]), 1, fp);
		}
	}
	printf("save ok222 curnum=%d!\n", curnum);
	fclose( fp );
	return 0;
}

static int destroy_vlan_info()
{
	int i;
	
	for( i=0; i < curnum; i++ )
	{
		if( NULL != vlaninfo[i] )
		{
			free( vlaninfo[i] );
	
		}
	}
	memset( vlaninfo, 0, sizeof(vlaninfo) );
	curnum = 0;
	return 0;
}




int get_all_submibif( mibsubiflst *subiflst )
{
	mibiflst iflst;
	int ifnum;
	int subifnum=0;
	int i;
	
	memset( &iflst, 0, sizeof(iflst ) );
	ifnum = get_all_mibif( &iflst );
	if( ifnum <= 0 )
	{
		return 0;
	}
	
	subiflst->buf_mibif_num = iflst.mibif_num;
	subiflst->lst = (mibsubif *)malloc(iflst.mibif_num*sizeof(mibsubif));/*not all if in iflst is subif! but......*/
	if( NULL == subiflst->lst )
	{
		free( iflst.lst );
		iflst.lst = NULL;
		return 0;	
	}
	
	init_vlan_info();
	for( i=0; i<ifnum; i++ )
	{
		if( 1 == if_sub_mibif(iflst.lst[i].ifname) )
		{
			subiflst->lst[subifnum].subif_vlanid = get_sub_mibif_vlanid(iflst.lst[i].ifname);
			if( subiflst->lst[subifnum].subif_vlanid <= 0 || subiflst->lst[subifnum].subif_vlanid >4095 )
			{
				continue;
			}
			get_sub_mibif_mainif(   iflst.lst[i].ifname, 
									subiflst->lst[subifnum].main_ifname, 
									sizeof(subiflst->lst[subifnum].main_ifname));
			memcpy( &(subiflst->lst[subifnum]), &(iflst.lst[i]), sizeof(iflst.lst[i]) );
			if( 0 != get_vlan_info(  subiflst->lst[subifnum].subif_vlanid,
							subiflst->lst[subifnum].info,
							sizeof( subiflst->lst[subifnum].info ) ) )
			{
				strncpy( subiflst->lst[subifnum].info, "get info error! please reset this info!", 
							sizeof(subiflst->lst[subifnum].info) );
			}
			subiflst->lst[subifnum].gindex = iflst.lst[i].gindex;
			subifnum++;
		}
	}

	destroy_vlan_info();
	subiflst->mibif_num = subifnum;
	free( iflst.lst );
	iflst.lst = NULL;
	
	/*get all vlan info!!*/
	
	return subifnum;
}

/*3、创建子接口，针对子接口的描述文字，需要有文件来保存。使用最简单的文本文件。*/
int create_sub_mibif( int vlanid, char *main_if, char *info )
{
	char command[256]="";
	if( 1 != if_main_mibif(main_if) )
	{
		return -1;	
	}
	
	sprintf( command, "sudo /usr/bin/if_cfg_subif.sh add %s %d", main_if, vlanid );
	system(command);
	
	if( 0 != init_vlan_info() )
	{
		return 0;	
	}
	set_vlan_info( vlanid, info );
	save_vlan_info();
	destroy_vlan_info();
	return 0;
}

/*4、删除子接口，*/
int delete_sub_mibif( int vlanid, char *main_if )
{
	char command[256]="";

	sprintf( command, "sudo /usr/bin/if_cfg_subif.sh del %s %d", main_if, vlanid );
	system(command);
	
	init_vlan_info();
	del_vlan_info( vlanid );
	save_vlan_info();
	destroy_vlan_info();
		
	return 0;
}

int mod_sub_mibif_mainif( int vlanid, char *old_main_if, char *new_main_if )
{
	char oldinfo[1024]="";

	if( strcmp( old_main_if, new_main_if )== 0 )
	{
		return 0;
	}
	if( 1 != if_main_mibif(new_main_if) || 1 != if_main_mibif(old_main_if) )
	{
		return -1;
	}
	
	init_vlan_info();
	get_vlan_info( vlanid, oldinfo, sizeof(oldinfo)-1 );
	destroy_vlan_info();
	
	delete_sub_mibif( vlanid, old_main_if );
	create_sub_mibif( vlanid, new_main_if, oldinfo );


	return 0;	
}


int mod_sub_mibif_info( int vlanid, char *new_info )
{
	
	init_vlan_info();
	set_vlan_info( vlanid, new_info );
	save_vlan_info();
	destroy_vlan_info();
	return 0;	
}

int mod_sub_mibif_gindex( int vlanid, char *main_if, int gindex )
{
	char subifname[32]="";

	snprintf(subifname, sizeof(subifname), "%s.%d", main_if, vlanid );
	return diag_gindex_handle( subifname, &gindex, 1 );
}


/*5、修改三层接口的ip地址*/
int set_mibif_ip( mibif *mif, unsigned int ipaddr )
{
	int udp_socket;
    struct  sockaddr_in     addr;
    struct  ifreq           ifr;

	if ((udp_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		return 0;
	}

    bzero(&addr, sizeof(addr));
    addr.sin_family = PF_INET;
    addr.sin_addr.s_addr = ipaddr;

    //printf("Interface:%s\n",mif->ifname);
    strncpy(ifr.ifr_name, mif->ifname, IFNAMSIZ);

    memcpy(&ifr.ifr_addr,(struct sockaddr *)&addr, sizeof(addr));

	if(ioctl(udp_socket, SIOCSIFADDR, &ifr) == -1) {
		perror("Ioctl Error");
		close(udp_socket);
		return -1;
	}
	
	set_mibif_mask( mif, mif->mask );
	
	close(udp_socket);
    return 0;

}


/*6、修改三层接口的mask*/
int set_mibif_mask( mibif *mif, unsigned int mask )
{
	int udp_socket;
    struct  sockaddr_in     addr;
    struct  ifreq           ifr;

	if ((udp_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		return 0;
	}

    bzero(&addr, sizeof(addr));
    addr.sin_family = PF_INET;
    addr.sin_addr.s_addr = mask;

    printf("Interface:%s\n",mif->ifname);
    strncpy(ifr.ifr_name, mif->ifname, IFNAMSIZ);

    memcpy(&ifr.ifr_addr,(struct sockaddr *)&addr, sizeof(addr));

	if(ioctl(udp_socket, SIOCSIFNETMASK, &ifr) == -1) {
		perror("Ioctl Error");
		close(udp_socket);
		return -1;
	}
	
	
	close(udp_socket);	
	return -1;	
}



