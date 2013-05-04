/* rdc_client.c */

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <syslog.h>
#include <errno.h>
//#include "rdc_ins.h"
#include "rdc_packet.h"
#include "rdc_handle.h"
//#include "eag_conf.h"
//#include "eag_interface.h"

#ifndef SLOT_IPV4_BASE
#define SLOT_IPV4_BASE  0XA9FE0200 /* 169.254.2.0 */
#endif
#ifndef HANSI_LOCAL
#define HANSI_LOCAL 1
#endif
#ifndef RDC_RADIUS_PORT_BASE
#define RDC_RADIUS_PORT_BASE	4150
#endif

#ifndef MAX_HANSI_ID
#define MAX_HANSI_ID	16
#endif

struct rdc_client_info{
	uint16_t coa_port;
	uint8_t slotid;
	uint8_t hansi_type;
	uint8_t hansi_id;
	uint8_t client_type;
	char module_desc[32];

	int log_packet;

	/*environment*/
	uint32_t rdc_server_ip;
	uint16_t rdc_server_port;
} ;
static struct rdc_client_info rdc_client;

static int rdc_server_slotid=0;
static int rdc_server_insid=0;

int 
rdc_set_server_hansi( int slotid, int insid )
{
	rdc_server_slotid = slotid;
	rdc_server_insid = insid;
	rdc_client.rdc_server_ip = SLOT_IPV4_BASE + rdc_server_slotid;
	if( HANSI_LOCAL == rdc_client.hansi_type ){
		rdc_client.rdc_server_port = RDC_RADIUS_PORT_BASE + rdc_server_insid;
	}else{
		rdc_client.rdc_server_port = 
							RDC_RADIUS_PORT_BASE + MAX_HANSI_ID + rdc_server_insid;
	}
	return 0;
}

static const char *
safe_strerror(int errnum)
{
	const char *s = strerror(errnum);
	return (s != NULL) ? s : "Unknown error";
}


static int
rdc_get_active_master_slotid(  )
{
	FILE *fp;
	int ret;

	fp = fopen("/dbm/product/active_master_slot_id", "r");
	if( NULL == fp ){
		return 0;
	}

	fscanf( fp, "%d", &ret);
	fclose( fp );

	return ret;
}

static char *
inet_ntoa_ex(unsigned long ip, char *ipstr, unsigned int buffsize)
{
	snprintf(ipstr, buffsize, "%lu.%lu.%lu.%lu", ip >> 24,
		 (ip & 0xff0000) >> 16, (ip & 0xff00) >> 8, (ip & 0xff));

	return ipstr;
}



void
rdc_set_log_packet_switch( int status )
{
	rdc_client.log_packet = status;
}

static void
rdc_log_packet( struct rdc_packet_t *packet )
{
	if( rdc_client.log_packet ){
		syslog(LOG_INFO,
				"rdc_log_packet length = %u", packet->length );
		syslog(LOG_INFO,
				"rdc_log_packet coa_port = %u", packet->coa_port );
		syslog(LOG_INFO,
				"rdc_log_packet slotid=%u  hansi_type=%u hansi_id=%u client_type=%u", 
								packet->slotid, packet->hansi_type, 
								packet->hansi_id, packet->client_type );
		syslog(LOG_INFO,
				"rdc_log_packet desc = %s", packet->desc );
	}

	return;
}

void
rdc_client_init( uint16_t coa_port, uint8_t slotid, 
			uint8_t hansi_type, uint8_t hansi_id, 
			uint8_t client_type, char *module_desc )
{
	rdc_client.coa_port = coa_port;
	rdc_client.slotid = slotid;
	rdc_client.hansi_type = hansi_type;
	rdc_client.hansi_id = hansi_id;
	rdc_client.client_type = client_type;
	if( NULL != module_desc && strlen(module_desc) > 0 ){
		strncpy( rdc_client.module_desc, module_desc, 
					sizeof(rdc_client.module_desc)-1 );
	}else{
		snprintf( rdc_client.module_desc, sizeof(rdc_client.module_desc)-1,
					"%d_%d_%d %d", client_type, hansi_type, hansi_id, slotid );
	}

	/*load sem  distributed info*/
	if( 0 == rdc_server_slotid ){
		rdc_server_slotid = rdc_get_active_master_slotid();
	}
	if( rdc_server_slotid < 0 ){
		syslog(LOG_ERR,
				"rdc_client_init failed get active master slot id err  id=%d",
				rdc_server_slotid);
	}else{
		syslog(LOG_INFO, "pdc_client_init pdc_server_slotid=%d insid=%d",
							rdc_server_slotid, rdc_server_insid );	
		rdc_client.rdc_server_ip = SLOT_IPV4_BASE + rdc_server_slotid;
		if( HANSI_LOCAL == hansi_type ){
			rdc_client.rdc_server_port = RDC_RADIUS_PORT_BASE + rdc_server_insid;
		}else{
			rdc_client.rdc_server_port = 
							RDC_RADIUS_PORT_BASE + MAX_HANSI_ID + rdc_server_insid;
		}
		syslog(LOG_INFO,
				"rdc_client_init  rdc_client.rdc_server_ip=%x  port = %d",
				rdc_client.rdc_server_ip,	rdc_client.rdc_server_port);
		
	}

	//rdc_client.log_packet = 1;//for debug.
	return;
}

void
rdc_client_uninit()
{
	return;
}

ssize_t
rdc_sendto(int s, const void *buf, 
			         size_t len, int flags, 
			         const struct sockaddr *to, socklen_t tolen,
			         unsigned int rslot, unsigned int rinst)
{
	ssize_t sendlen=0;
	struct rdc_packet_t rdc_packet;
	struct sockaddr_in addr_in;
	char ipstr[32];

	if( NULL == to ){
		syslog(LOG_ERR,
				"rdc_sendto sockaddr to = NULL" );
		return sendto( s, buf, len, flags, to, tolen );
	}

	if( len > sizeof(rdc_packet.data)){
		syslog(LOG_ERR,
				"rdc_sendto length = %u over limite %d", 
					len, sizeof(rdc_packet) - RDC_PACKET_HEADSIZE );
		return -1;
	}
	
	addr_in = *((struct sockaddr_in *)to);
	memset( &rdc_packet, 0, sizeof(rdc_packet) );
	rdc_packet.ip = addr_in.sin_addr.s_addr;
	rdc_packet.port = addr_in.sin_port;
	rdc_packet.length = len + RDC_PACKET_HEADSIZE;
	rdc_packet.length = htons(rdc_packet.length);
	rdc_packet.coa_port = htons(rdc_client.coa_port);
	rdc_packet.slotid = rdc_client.slotid;
	rdc_packet.hansi_type = rdc_client.hansi_type;
	rdc_packet.hansi_id = rdc_client.hansi_id;
	rdc_packet.client_type = rdc_client.client_type;
	strncpy( (char *)rdc_packet.desc, (char *)rdc_client.module_desc, 
				sizeof(rdc_packet.desc)-1 );
	memcpy( rdc_packet.data, buf, len );


	rdc_set_server_hansi(rslot, rinst);
	addr_in.sin_addr.s_addr = htonl(rdc_client.rdc_server_ip);
	addr_in.sin_port = htons(rdc_client.rdc_server_port);
	
	syslog(LOG_INFO,
			"rdc_sendto ip:port = %x:%u", addr_in.sin_addr.s_addr, addr_in.sin_port );
	
	inet_ntoa_ex( rdc_packet.ip, ipstr, sizeof(ipstr) );
	syslog(LOG_INFO,
			"rdc_sendto ip:port = %s:%u", ipstr, rdc_packet.port );

	rdc_log_packet( &rdc_packet );
	sendlen = sendto( s, &rdc_packet, len + RDC_PACKET_HEADSIZE, flags, 
					(struct sockaddr *)(&addr_in), tolen );
	if(  sendlen < 0 ){
		syslog(LOG_ERR,
				"rdc_sendto sendto error  err=%s",  safe_strerror(errno));
		return sendlen;
	}
	if( sendlen > RDC_PACKET_HEADSIZE ){
		return sendlen - RDC_PACKET_HEADSIZE;
	}

	return -1;
}


ssize_t
rdc_recvfrom(int s, void *buf, size_t len, int flags,
					 struct sockaddr *from, socklen_t *fromlen)
{
	int recvlen;//qiuchen
	struct rdc_packet_t rdc_packet;
	struct sockaddr_in *addr_in;
	char ipstr[32];

	memset( &rdc_packet, 0, sizeof(rdc_packet));
	recvlen = recvfrom(s, &rdc_packet, sizeof(rdc_packet), flags,
					from, fromlen );

	if( recvlen < 0 ){
		syslog(LOG_ERR,
				"rdc_recvfrom length = %u over limite %d", 
				len, sizeof(rdc_packet) - RDC_PACKET_HEADSIZE );
		return -1;
	}
	if( recvlen <= RDC_PACKET_HEADSIZE ){
		syslog(LOG_ERR,
				"rdc_recvfrom recvlen  <= headsize for bad pkg!");
		return -1;
	}

	if( recvlen != ntohs(rdc_packet.length) ){
		syslog(LOG_ERR,
				"rdc_recvfrom pkg error: recvlen(%d) not equal to length(%d) in packet!",
				recvlen, ntohs(rdc_packet.length));
		return -1;
	}

	if( recvlen - RDC_PACKET_HEADSIZE < len ){
		len = recvlen - RDC_PACKET_HEADSIZE;
	}

	memcpy( buf, rdc_packet.data, len );

	addr_in = (struct sockaddr_in *)from;
	addr_in->sin_addr.s_addr = rdc_packet.ip;
	addr_in->sin_port = rdc_packet.port;

	inet_ntoa_ex( rdc_packet.ip, ipstr, sizeof(ipstr) );
	syslog(LOG_INFO,
			"rdc_recvfrom ip:port = %s:%u", ipstr, rdc_packet.port );
	rdc_log_packet( &rdc_packet );
	
	return len;	
}


#if 0

int main()
{
	struct sockaddr_in address;
	struct sockaddr_in from;
	socklen_t fromlen;
	char buff[4000];
	
	memset( buff, '1', sizeof(buff) );
	
	int fd;
	rdc_client_init(3799, 1, 1, 12, 0, NULL);
	
	
	memset(&address, 0, sizeof(struct sockaddr_in));
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if( fd < 0 ){
		printf("socket create failed!");
		return -1;
	}
	address.sin_family = AF_INET;
	inet_aton( "192.168.7.103" , &(address.sin_addr) );
	address.sin_port = htons(4000);

	if (0 != bind(fd, (struct sockaddr *) &address, sizeof (address))){
		close( fd );
		printf("socket bind failed!");
		return -1;
	}

	rdc_client.rdc_server_ip = htonl(address.sin_addr.s_addr);
	rdc_client.rdc_server_port = 4000;

	
	while( 1 ){
		fd_set readfds;
		int num;
		
		struct timeval timer_wait;
		timer_wait.tv_sec = 0;
		timer_wait.tv_usec = 50000;

		printf("heart beat! before send to\n");
		rdc_sendto( fd, buff, sizeof(buff), 0, 
					(struct sockaddr *)&address, sizeof(address));

		FD_ZERO(&(readfds));
		FD_SET(fd, &(readfds));
		num = select(fd+1, &readfds, NULL, NULL, &timer_wait);
		printf("num = %d\n", num);
		if( num > 0 ){
			int rcvlen;
			rcvlen = rdc_recvfrom( fd, buff, sizeof(buff), 0,
								 (struct sockaddr *)&from, &fromlen);
		}
		sleep(2);
	}

	rdc_client_uninit();
	return 0;
}

#endif

