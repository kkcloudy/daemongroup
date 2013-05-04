/* pdc_handle.c */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <syslog.h>

#include "eag_util.h"
#include "pdc_ins.h"
#include "pdc_packet.h"
#include "pdc_handle.h"
#include "eag_conf.h"
#include "eag_interface.h"
#include "eag_errcode.h"
#include "eag_log.h"

static struct pdc_client_info{
	uint8_t slotid;
	uint8_t hansi_type;
	uint8_t hansi_id;
	uint8_t client_type;
	char module_desc[32];

	int log_packet;

	/*environment*/
	uint32_t pdc_server_ip;
	uint16_t pdc_server_port;
} pdc_client;


static int pdc_server_slotid=0;
static int pdc_server_insid=0;

int 
pdc_set_server_hansi( int slotid, int insid )
{
	pdc_server_slotid = slotid;
	pdc_server_insid = insid;
	syslog(LOG_INFO, "pdc_set_server_hansi pdc_server_slotid=%d insid=%d",
			pdc_server_slotid, pdc_server_insid);

	if (HANSI_LOCAL == pdc_client.hansi_type) {
		pdc_client.pdc_server_ip = SLOT_IPV4_BASE + 100 + pdc_server_insid;
		pdc_client.pdc_server_port = PDC_PORTAL_PORT_BASE + pdc_server_insid;
	} else {
		pdc_client.pdc_server_ip = SLOT_IPV4_BASE + pdc_server_slotid;
		pdc_client.pdc_server_port = 
			PDC_PORTAL_PORT_BASE + MAX_HANSI_ID + pdc_server_insid;
	}
	syslog(LOG_INFO,
			"pdc_set_server_hansi pdc_client.pdc_server_ip=%x  port=%d",
			pdc_client.pdc_server_ip, pdc_client.pdc_server_port);
	return 0;
}

int
pdc_get_server_hansi( int *slotid, int *insid )
{
	*slotid = pdc_server_slotid;
	*insid = pdc_server_insid;
	return 0;
}

#if 0
static int
pdc_get_active_master_slotid(  )
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
#endif

void
pdc_set_log_packet_switch( int status )
{
	pdc_client.log_packet = status;
}

static void
pdc_log_packet( struct pdc_packet_t *packet )
{
	if( pdc_client.log_packet ){
		eag_log_debug("pdc_handle",
				"pdc_log_packet length = %u", packet->length );
		eag_log_debug("pdc_handle",
				"pdc_log_packet slotid=%u  hansi_type=%u hansi_id=%u client_type=%u", 
								packet->slotid, packet->hansi_type, 
								packet->hansi_id, packet->client_type );
		eag_log_debug("pdc_handle",
				"pdc_log_packet desc = %s", packet->desc );
	}

	return;
}

void
pdc_client_init( uint8_t slotid, 
			uint8_t hansi_type, uint8_t hansi_id, 
			uint8_t client_type, char *module_desc )
{

	pdc_client.slotid = slotid;
	pdc_client.hansi_type = hansi_type;
	pdc_client.hansi_id = hansi_id;
	pdc_client.client_type = client_type;
	if( NULL != module_desc && strlen(module_desc) > 0 ){
		strncpy( pdc_client.module_desc, module_desc, 
					sizeof(pdc_client.module_desc)-1 );
	}else{
		snprintf( pdc_client.module_desc, sizeof(pdc_client.module_desc)-1,
					"%d_%d_%d %d", client_type, hansi_type, hansi_id, slotid );
	}
	
	
	pdc_server_slotid = 1;
	pdc_server_insid = 0;
	syslog(LOG_INFO, "pdc_client_init pdc_server_slotid=%d insid=%d",
			pdc_server_slotid, pdc_server_insid);
	
	if (HANSI_LOCAL == hansi_type) {
		pdc_client.pdc_server_ip = SLOT_IPV4_BASE + 100 + hansi_id;
		pdc_client.pdc_server_port = PDC_PORTAL_PORT_BASE + pdc_server_insid;
	} else {
		pdc_client.pdc_server_ip = SLOT_IPV4_BASE + pdc_server_slotid;
		pdc_client.pdc_server_port = 
			PDC_PORTAL_PORT_BASE+MAX_HANSI_ID + pdc_server_insid;
	}
	syslog(LOG_INFO,
			"pdc_client_init rdc_client.rdc_server_ip=%x  port=%d",
			pdc_client.pdc_server_ip, pdc_client.pdc_server_port);

#if 0
	/*load sem  distributed info*/
	if( 0 == pdc_server_slotid ){
		pdc_server_slotid = pdc_get_active_master_slotid();
	}
	if( pdc_server_slotid < 0 ){
		syslog(LOG_ERR,
				"pdc_client_init failed get active master slot id err  id=%d",
				pdc_server_slotid);
	}else{
		syslog(LOG_INFO, "pdc_client_init pdc_server_slotid=%d insid=%d",
							pdc_server_slotid, pdc_server_insid );
//		pdc_client.pdc_server_ip = SLOT_IPV4_BASE + pdc_server_slotid;
		if( HANSI_LOCAL == hansi_type ){
			pdc_client.pdc_server_ip = SLOT_IPV4_BASE + 100 + hansi_id;
			pdc_client.pdc_server_port = PDC_PORTAL_PORT_BASE+pdc_server_insid;
		}else{
			pdc_client.pdc_server_ip = SLOT_IPV4_BASE + pdc_server_slotid;
			pdc_client.pdc_server_port = 
						PDC_PORTAL_PORT_BASE+MAX_HANSI_ID+pdc_server_insid;
			
		}
		syslog(LOG_INFO,
				"pdc_client_init  rdc_client.rdc_server_ip=%x  port = %d",
				pdc_client.pdc_server_ip,	pdc_client.pdc_server_port);		
	}
#endif
	//pdc_client.log_packet = 1;//for debug.
	return;
}

void
pdc_client_uninit()
{
	return;
}

ssize_t
pdc_sendto(int s, const void *buf, 
			         size_t len, int flags, 
			         const struct sockaddr *to, socklen_t tolen)
{
	ssize_t sendlen=0;
	struct pdc_packet_t pdc_packet;
	struct sockaddr_in addr_in;
	char ipstr[32];

	if( NULL == to ){
		syslog(LOG_ERR,
				"pdc_sendto sockaddr to = NULL" );
		return sendto( s, buf, len, flags, to, tolen );
	}

	if( len > sizeof(pdc_packet.data)){
		syslog(LOG_ERR,
				"pdc_sendto length = %u over limite %d", 
					len, sizeof(pdc_packet) - PDC_PACKET_HEADSIZE );
		return -1;
	}
	
	addr_in = *((struct sockaddr_in *)to);
	memset( &pdc_packet, 0, sizeof(pdc_packet) );
	pdc_packet.ip = addr_in.sin_addr.s_addr;
	pdc_packet.port = addr_in.sin_port;
	pdc_packet.length = len + PDC_PACKET_HEADSIZE;
	pdc_packet.length = htons(pdc_packet.length);
	pdc_packet.slotid = pdc_client.slotid;
	pdc_packet.hansi_type = pdc_client.hansi_type;
	pdc_packet.hansi_id = pdc_client.hansi_id;
	pdc_packet.client_type = pdc_client.client_type;
	strncpy( (char *)pdc_packet.desc, (char *)pdc_client.module_desc, 
				sizeof(pdc_packet.desc)-1 );
	memcpy( pdc_packet.data, buf, len );


	addr_in.sin_addr.s_addr = htonl(pdc_client.pdc_server_ip);
	addr_in.sin_port = htons(pdc_client.pdc_server_port);
	eag_log_debug("pdc_handle",
			"pdc_sendto ip:port = %x:%u", addr_in.sin_addr.s_addr, addr_in.sin_port );
	
	ip2str( pdc_packet.ip, ipstr, sizeof(ipstr) );
	eag_log_debug("pdc_handle",
			"pdc_sendto ip:port = %s:%u", ipstr, pdc_packet.port );

	pdc_log_packet( &pdc_packet );
	sendlen = sendto( s, &pdc_packet, len + PDC_PACKET_HEADSIZE, flags, 
					(struct sockaddr *)(&addr_in), tolen );
	if( sendlen > PDC_PACKET_HEADSIZE ){
		return sendlen - PDC_PACKET_HEADSIZE;
	}

	return 0;
}


ssize_t
pdc_recvfrom(int s, void *buf, size_t len, int flags,
					 struct sockaddr *from, socklen_t *fromlen)
{
	ssize_t recvlen = 0;
	struct pdc_packet_t pdc_packet;
	struct sockaddr_in *addr_in;
	char ipstr[32] = "";

	memset( &pdc_packet, 0, sizeof(pdc_packet));
	recvlen = recvfrom(s, &pdc_packet, sizeof(pdc_packet), flags,
					from, fromlen );
	if( recvlen < 0 ){
		syslog(LOG_ERR,
				"pdc_recvfrom length = %u over limite %d", 
				len, sizeof(pdc_packet) - PDC_PACKET_HEADSIZE );
		return -1;
	}
	if( recvlen <= PDC_PACKET_HEADSIZE ){
		syslog(LOG_ERR,
				"pdc_recvfrom recvlen  <= headsize for bad pkg!");
		return -1;
	}

	if( recvlen != ntohs(pdc_packet.length) ){
		syslog(LOG_ERR,
				"pdc_recvfrom pkg error: recvlen not equal to length in packet!");
		return -1;
	}

	if( recvlen - PDC_PACKET_HEADSIZE < len ){
		len = recvlen - PDC_PACKET_HEADSIZE;
	}

	memcpy( buf, pdc_packet.data, len );
	
	addr_in = (struct sockaddr_in *)from;
	addr_in->sin_family = AF_INET;	//for fault address family
	addr_in->sin_addr.s_addr = pdc_packet.ip;
	addr_in->sin_port = pdc_packet.port;

	ip2str( pdc_packet.ip, ipstr, sizeof(ipstr) );
	eag_log_debug("pdc_handle",
			"pdc_recvfrom ip:port = %s:%u", ipstr, pdc_packet.port );
	pdc_log_packet( &pdc_packet );
	
	return len;	
}

/* this func has been exist in eag_log.h */
/*
static const char *
safe_strerror(int errnum)
{
	const char *s = strerror(errnum);
	return (s != NULL) ? s : "Unknown error";
}
*/

/*add to send user map to pdc*/
int
pdc_send_usermap( int fd, uint32_t userip, 
		uint8_t slot_id, uint8_t hansi_type, uint8_t hansi_id )
{
	struct pdc_usermap_pkt_t map_pkt={0};
	char ipstr[32];
	struct sockaddr_in addr_in;
	uint32_t toip = 0;
	uint16_t toport = 0;
	ssize_t sendlen=0;
	socklen_t tolen = 0;
	
	map_pkt.client_type = 1;/*1 means eag*/
	map_pkt.slot_id = slot_id;
	map_pkt.hansi_id = hansi_id;
	map_pkt.hansi_type = hansi_type;
	map_pkt.user_ip = htonl(userip);

	ip2str( userip, ipstr, sizeof(ipstr)-1 );
	snprintf( (char *)map_pkt.desc, sizeof(map_pkt.desc)-1,
				"map %s:%u-%u", ipstr, slot_id, hansi_id );

	toip = pdc_server_slotid + SLOT_IPV4_BASE;
	toport = pdc_server_insid + PDC_USERMAP_PORT_BASE;
	
	addr_in.sin_addr.s_addr = htonl(toip);
	addr_in.sin_port = htons(toport);
	addr_in.sin_family = AF_INET;

	tolen = sizeof(addr_in);
	sendlen = sendto( fd, &map_pkt, sizeof(map_pkt), 0, 
					(struct sockaddr *)(&addr_in), tolen );
	if( sendlen != sizeof(map_pkt) ){
		syslog(LOG_ERR,
				"pdc_send_usermap sendto error: %s", safe_strerror(errno));
		return -1;
	}
	//printf("toip = %x  toport=%u", toip, toport);
	eag_log_debug("pdc_handle","pdc_send_usermap desc:%s", map_pkt.desc );
	return 0;
}


#if pdc_handle_test

int main()
{
	struct sockaddr_in address;
	struct sockaddr_in from;
	socklen_t fromlen;
	char buff[4000];
	
	memset( buff, '1', sizeof(buff) );
	
	int fd;
	pdc_set_server_hansi( 2, 4);
	pdc_client_init( 2, 0, 4, 0, NULL);
	
	
	memset(&address, 0, sizeof(struct sockaddr_in));
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if( fd < 0 ){
		printf("socket create failed!");
		return -1;
	}
	address.sin_family = AF_INET;
	inet_aton( "169.254.2.2" , &(address.sin_addr) );
	address.sin_port = htons(3000);

	if (0 != bind(fd, (struct sockaddr *) &address, sizeof (address))){
		close( fd );
		printf("socket bind failed!");
		return -1;
	}

	//pdc_send_usermap( fd, 123456, 2, 0,4);
	pdc_send_usermap( fd, 2, 4, 123456, 2, 0, 4);
#if 0
	pdc_client.pdc_server_ip = htonl(address.sin_addr.s_addr);
	pdc_client.pdc_server_port = 2000;

	
	while( 1 ){
		fd_set readfds;
		int num;
		
		struct timeval timer_wait;
		timer_wait.tv_sec = 0;
		timer_wait.tv_usec = 50000;

		printf("heart beat! before send to\n");
		pdc_sendto( fd, buff, sizeof(buff), 0, 
					(struct sockaddr *)&address, sizeof(address));

		FD_ZERO(&(readfds));
		FD_SET(fd, &(readfds));
		num = select(fd+1, &readfds, NULL, NULL, &timer_wait);
		printf("num = %d\n", num);
		if( num > 0 ){
			int rcvlen;
			rcvlen = pdc_recvfrom( fd, buff, sizeof(buff), 0,
								 (struct sockaddr *)&from, &fromlen);
		}
		sleep(2);
	}
#endif
	pdc_client_uninit();
	return 0;
}

#endif

