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
* dot11InformationInfo.h
*
*
* CREATOR:
* autelan.software.Network Dep. team
*
* DESCRIPTION:
* 
*
*
*******************************************************************************/




#ifndef DOT11INFORMATIONINFO_H
#define DOT11INFORMATIONINFO_H

/* function declarations */
void init_dot11InformationInfo(void);
Netsnmp_Node_Handler handle_informationinfo;

#if 0
typedef enum{
    REQ_GET_ONLINE_NUM=1,
    ACK_GET_ONLINE_NUM,
    REQ_GET_USR_BY_INDEX_RANG,
    ACK_GET_USR_BY_INDEX_RANG,
    REQ_LOGOUT_BY_INFO,
    ACK_LOGOUT_BY_INFO,
    /*use for search*/
    REQ_GET_BY_INDEX,
    ACK_GET_BY_INDEX,
    REQ_GET_BY_NAME,
    ACK_GET_BY_NAME,
    REQ_GET_BY_MAC,
    ACK_GET_BY_MAC,
    REQ_GET_BY_IP,
    ACK_GET_BY_IP,
    SET_TIME_MSG,		//ADD BY LT FOR RJ 
}USER_MNG_TYPE;


typedef struct {
    int index;
    int ipaddr;
    char  username[64];/**/
    unsigned char usermac[6];
    int session_time;
    /*add for guangzhou mobile test*/
    uint64_t input_octets;
    uint64_t output_octets;

    char sessionid[64];/*need??*/
}STOnlineUserInfo;

typedef struct {
    int pkg_type;
    unsigned int len;
    unsigned int all_user_num;/*在线用户的总数*/
    int err_code;
    union {
    	struct {
			union {
				struct {
					unsigned int idx_begin;
					unsigned int idx_end;
				} idx;
				unsigned int idx_get;
				char username[64];
				unsigned char user_mac[6];
				unsigned int user_ip;
				STOnlineUserInfo user_info;
			} reqinfo;
    	}req;    	
    	struct {
			int LOGOUT_FLAG; /* 用来记录只回复logout 消息的包*/
    		int user_num_in_pkg;/*0 or more当前包中记录的用户数*/
    		STOnlineUserInfo users[0];/*实际大小由user_num来确定,创建pkg的大小时，也需要根据大小先计算包的大小*/
    	} ack;
    char msg[512];
    } data;
}STUserManagePkg;
static void MNG_PKG_HTON( STUserManagePkg *p )
{
	STUserManagePkg *_pkg_mng = p;
	int i = 0;
	
	if( NULL == p )
	{
		return;	
	}
	switch( _pkg_mng->pkg_type )
	{
		case REQ_GET_ONLINE_NUM:
			break;
		case REQ_GET_USR_BY_INDEX_RANG:
			_pkg_mng->data.req.reqinfo.idx.idx_begin = htonl(_pkg_mng->data.req.reqinfo.idx.idx_begin);
			_pkg_mng->data.req.reqinfo.idx.idx_end = htonl(_pkg_mng->data.req.reqinfo.idx.idx_end);
			break;
		case REQ_LOGOUT_BY_INFO:
			break;
		case REQ_GET_BY_INDEX:
			_pkg_mng->data.req.reqinfo.idx_get = htonl(_pkg_mng->data.req.reqinfo.idx_get);
			break;
		case REQ_GET_BY_NAME:
			break;
		case REQ_GET_BY_MAC:
			break;
		case REQ_GET_BY_IP:
			_pkg_mng->data.req.reqinfo.user_ip = htonl(_pkg_mng->data.req.reqinfo.user_ip);
			break;
		/*for ack*/
		case ACK_GET_ONLINE_NUM:
			break;
		case ACK_GET_USR_BY_INDEX_RANG:
		case ACK_LOGOUT_BY_INFO:
		case ACK_GET_BY_INDEX:
		case ACK_GET_BY_NAME:
		case ACK_GET_BY_MAC:
		case ACK_GET_BY_IP:			
			for( i=0; i< _pkg_mng->data.ack.user_num_in_pkg; i++ )
			{
				STOnlineUserInfo *users = _pkg_mng->data.ack.users;
				users[i].index = htonl( users[i].index );
				users[i].ipaddr = htonl( users[i].ipaddr );
				users[i].session_time = htonl( users[i].session_time );				
			}

			_pkg_mng->data.ack.user_num_in_pkg = htonl(_pkg_mng->data.ack.user_num_in_pkg);
			break;
		default:
			break;
	}
	
	_pkg_mng->len = htonl( _pkg_mng->len );
	_pkg_mng->pkg_type = htonl( _pkg_mng->pkg_type );
	_pkg_mng->all_user_num = htonl( _pkg_mng->all_user_num );
	
	return;
}
#endif

static int conn_to_unix_socket( char *sockpath )/*this func is defined in ws_user_manage.c!  should it be......*/
{
	struct sockaddr_un remote;
	int len;
	int fd=-1;

/*	chmod_file( sockpath );*/
	fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if ( fd == -1) {
		fprintf(stderr,"create_unix_socket create unix socket failed");
		return fd;
	}

	remote.sun_family = AF_UNIX;
	strcpy(remote.sun_path, sockpath);/*how len of sun_path??*/
	len = strlen(remote.sun_path) + sizeof(remote.sun_family);
	if (connect(fd, (struct sockaddr *)&remote, len) == -1) {
		fprintf(stderr, "conn_to_unix_socket socket fd %d	 %s\n", fd, strerror(errno));
		close(fd);
		fd = -1; 
		return fd;
	}

	return fd;
}

static int send_msg(char * s_msg, int len)
{
	struct sockaddr_in servadd;
	struct sockaddr_un remote;
	struct hostent	*hp;
	int sock_id,sock_fd;
	char message[BUFSIZ];
	int	messlen;
	STUserManagePkg *preq=NULL;
	int pkg_len=0;

	pkg_len = sizeof(STUserManagePkg) + len;
	preq = (STUserManagePkg *)malloc( pkg_len );
	if(NULL == preq )
	{
		printf("send_msg malloc failed!");
		return -1;
	}
	memset( preq, 0, pkg_len );
	
	
	preq->pkg_type = SET_TIME_MSG;
	preq->data.msg.msg_len = len;
	
	memcpy(preq->data.msg.msg_eapol, s_msg, len);
	sock_id = conn_to_unix_socket( EAGINS_UNIXSOCK_PREFIX"1" );
	if( sock_id < 0 )
	{
		printf("conn_to_unix_socket failed!");
		return -1;
	}
	printf("conn_to_unix_socket sock_id=%d\n", sock_id );
		
	int sendfg = 0;
	MNG_PKG_HTON( &preq );

	sendfg = send(sock_id, preq, pkg_len,0);
	printf("sendfg =%d\n", sendfg );
	#if 0
	int fl;
	if( (fl = fcntl(sock_id,F_GETFL,0)) < 0)
		oops("fcntl");
	fl |= O_NONBLOCK;
	if(fcntl(sock_id,F_SETFL,fl) < 0)
		oops("fcntl");
	

	{
	  	int flag =  read(sock_id,message,BUFSIZ) ;
		      printf("flag==%d\n",flag);
			printf("message==%s\n",message);
	}	
	if(messlen == -1)
		oops("read");
	if(write(1,message,messlen) != messlen)
		oops("write");
	
	#endif
	close(sock_id);
 return 0;
}


#endif /* DOT11SPEEDLIMIT_H */
