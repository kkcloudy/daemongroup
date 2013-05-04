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
*
*
* CREATOR:
* autelan.software.xxx. team
*
* DESCRIPTION:
* xxx module main routine
*
*
*******************************************************************************/

/* drp_util.c */
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <syslog.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <dbus/dbus.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <errno.h>

#include "drp_def.h"
#include "drp_log.h"
#include "drp_util.h"

/* Struct timeval's tv_usec one second value.  */
#define TIMER_SECOND_MICRO 1000000L
/* macro definition */
#define DRP_URANDOM_FILE_PATH "/dev/urandom"

int
sockopt_reuseaddr(int sock)
{
	int ret;
	int on = 1;

	ret = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
			 (void *) &on, sizeof (on));
	if (ret < 0) {
		drp_log_warning("can't set sockopt SO_REUSEADDR to socket %d",
				sock);
		return -1;
	}
	return 0;
}

int 
set_nonblocking(int fd) 
{
	int flags = fcntl(fd, F_GETFL);
	if (flags < 0) return -1;
	fcntl(fd, F_SETFL, flags | O_NONBLOCK);
	return 0;
}

int 
clear_nonblocking(int fd) 
{
	int flags = fcntl(fd, F_GETFL);
	if (flags < 0) return -1;
	fcntl(fd, F_SETFL, flags & (~O_NONBLOCK));
	return 0;
}


/* Adjust so that tv_usec is in the range [0,TIMER_SECOND_MICRO).
   And change negative values to 0. */
struct timeval
timeval_adjust(struct timeval a)
{
	while (a.tv_usec >= TIMER_SECOND_MICRO) {
		a.tv_usec -= TIMER_SECOND_MICRO;
		a.tv_sec++;
	}

	while (a.tv_usec < 0) {
		a.tv_usec += TIMER_SECOND_MICRO;
		a.tv_sec--;
	}

	if (a.tv_sec < 0) {
		/* Change negative timeouts to 0. */
		a.tv_sec = a.tv_usec = 0;
	}

	return a;
}

struct timeval
timeval_subtract(struct timeval a, struct timeval b)
{
	struct timeval ret;

	ret.tv_usec = a.tv_usec - b.tv_usec;
	ret.tv_sec = a.tv_sec - b.tv_sec;

	return timeval_adjust(ret);
}

long
timeval_cmp(struct timeval a, struct timeval b)
{
	return (a.tv_sec == b.tv_sec
		? a.tv_usec - b.tv_usec : a.tv_sec - b.tv_sec);
}

unsigned long
timeval_elapsed(struct timeval a, struct timeval b)
{
	return (((a.tv_sec - b.tv_sec) * TIMER_SECOND_MICRO)
		+ (a.tv_usec - b.tv_usec));
}

void
rand_buff(unsigned char *buff, unsigned long size)
{

	static FILE *fp = NULL;

#if 1
	if (NULL == fp) {
		fp = fopen(DRP_URANDOM_FILE_PATH, "r");
	}

	if (NULL != fp) {
		fread(buff, 1, size, fp);
	}
#else
	memcpy(buff, "0123456789012345678901234567890123456789"
	       "0123456789012345678901234567890123456789"
	       "0123456789012345678901234567890123456789", size);
#endif
	return;
}

/* This function is copyed from WID_Check_IP_Format() in wid_wtp.h . */
int drp_check_ip_format(const char *str)
{
	char *endptr = NULL;
	char* endptr1 = NULL;
	char c;
	int IP,i;

	c = str[0];
	if (c>='0'&&c<='9'){
		IP= strtoul(str,&endptr,10);
		if(IP < 0||IP > 255)
			return DRP_FAILURE;
		else if(((IP < 10)&&((endptr - str) > 1))||((IP < 100)&&((endptr - str) > 2))||((IP < 256)&&((endptr - str) > 3)))
			return DRP_FAILURE;
		for(i = 0; i < 3; i++){
			if(endptr[0] == '\0'||endptr[0] != '.')
				return DRP_FAILURE;
			else{
				endptr1 = &endptr[1];
				IP= strtoul(&endptr[1],&endptr,10); 			
				if(IP < 0||IP > 255)
					return DRP_FAILURE;				
				else if(((IP < 10)&&((endptr - endptr1) > 1))||((IP < 100)&&((endptr - endptr1) > 2))||((IP < 256)&&((endptr - endptr1) > 3)))
					return DRP_FAILURE;
			}
		}
		if(endptr[0] == '\0' && IP >= 0)
			return DRP_SUCCESS;
		else
			return DRP_FAILURE;
	}
	else
		return DRP_FAILURE;		
}


char *
inet_ntoa_ex(unsigned long ip, char *ipstr, unsigned int buffsize)
{
	snprintf(ipstr, buffsize, "%lu.%lu.%lu.%lu", ip >> 24,
		 (ip & 0xff0000) >> 16, (ip & 0xff00) >> 8, (ip & 0xff));

	return ipstr;
}

int
mac2str(char *dst, unsigned long dstlen,unsigned char *src, char format)
{
	if (NULL == dst || NULL == src) {
		return -1;
	}

	if (':' != format && '-' != format && '_' != format) {
		format = ':';	/*default to : */
	}

	snprintf(dst, dstlen-1,"%02X%c%02X%c%02X%c%02X%c%02X%c%02X",
		src[0], format, src[1], format,
		src[2], format, src[3], format, src[4], format, src[5]);
	return 0;
}

int
str2mac(unsigned char *dst, unsigned long dstlen, char *src)
{
	char format;
	int a1, a2, a3, a4, a5, a6;
	int num = 0;

	if (NULL == dst || NULL == src || dstlen < 6 ) {
		return -1;
	}

	format = src[2];

	switch (format) {
	case ':':
		num =
		    sscanf(src, "%x:%x:%x:%x:%x:%x", &a1, &a2, &a3, &a4, &a5,
			   &a6);
		break;
	case '-':
		num =
		    sscanf(src, "%x-%x-%x-%x-%x-%x", &a1, &a2, &a3, &a4, &a5,
			   &a6);
		break;
	case '_':
		num =
		    sscanf(src, "%x_%x_%x_%x_%x_%x", &a1, &a2, &a3, &a4, &a5,
			   &a6);
		break;
	default:
		break;
	}

	if (6 == num) {
		dst[0] = a1;
		dst[1] = a2;
		dst[2] = a3;
		dst[3] = a4;
		dst[4] = a5;
		dst[5] = a6;
	}

	return (6 == num) ? 0 : -1;
}

void
delete_0d0a(char *buff)
{
	if (NULL != buff) {
		for (; *buff && *buff != 0x0d && *buff != 0x0a; buff++) {;
		}
		*buff = 0;
	}

}


char * inet_int2str( unsigned int ip_addr, char *ip_str, unsigned int buffsize )
{
	if(ip_str == NULL)
	{
		return "";
	}
	snprintf( ip_str, buffsize, "%u.%u.%u.%u", ip_addr>>24, (ip_addr&0xff0000)>>16,
				(ip_addr&0xff00)>>8, (ip_addr&0xff) );
				
	return ip_str;
}



int
is_mask( unsigned long mask )
{
	int i;
	long temp = (long)mask;/*”–∑˚∫≈”““∆±£¡Ù∑˚∫≈Œª*/

	for( i=0;i<32;i++ ){
		//printf("temp = %08X\n",(unsigned int)temp);
		if( 1 == (temp&0x1) ){/*right shift until last bit to  1*/
			break;
		}
		
		temp >>= 1;
	}

	return ((~temp)==0);
}

int
is_net_overlap( unsigned long ip1, unsigned long mask1,
					unsigned long ip2, unsigned long mask2 )
{
	unsigned long minmask;
	if( (!is_mask(mask1))
		|| (!is_mask(mask2))){
		return 0;
	}
	minmask = MIN(mask1,mask2);
	return ((ip1&minmask)==(ip2&minmask));
}

#ifdef drp_util_test
#include "mem.c"
#include "log.c"
#include "drp_def.h"


int
main()
{
	printf("is_mask(255) = %d\n",is_mask(255) );
	printf("is_mask(254) = %d\n",is_mask(254) );
	printf("is_mask(0xfffffe00) = %d\n",is_mask(0xfffffe00) );
	printf("is_mask(0xfffffa00) = %d\n",is_mask(0xfffffa00) );
	printf("is_mask(0xfffffb00) = %d\n",is_mask(0xfffffb00) );
	printf("is_mask(0xfffffc00) = %d\n",is_mask(0xfffffc00) );
	printf("is_mask(0xffffe000) = %d\n",is_mask(0xffffe000) );
	printf("is_mask(0xff400000) = %d\n",is_mask(0xff400000) );	
	printf("is_mask(0xff800000) = %d\n",is_mask(0xff800000) );
	printf("is_mask(0xffffc000) = %d\n",is_mask(0xffffc000) );
	printf("is_mask(0xffffef00) = %d\n",is_mask(0xffffef00) );
	printf("is_mask(0xffff) = %d\n",is_mask(0xffff) );
	printf("is_mask(0xeffffe00) = %d\n",is_mask(0xeffffe00) );

	struct in_addr addr;
	unsigned long ip1,mask1;
	unsigned long ip2,mask2;
	
	inet_aton("192.168.1.1", &addr);
	ip1 = ntohl(addr.s_addr);
	inet_aton("255.255.0.0", &addr);
	mask1 = ntohl(addr.s_addr);
	inet_aton("192.168.1.1", &addr);
	ip2 = ntohl(addr.s_addr);
	inet_aton("255.255.255.0", &addr);
	mask2 = ntohl(addr.s_addr);
	printf("192.168.1.1 255.255.0.0 192.168.1.1 255.255.255.0 = %d\n",
				is_net_overlap(ip1,mask1,ip2,mask2) );

	inet_aton("192.168.1.1", &addr);
	ip1 = ntohl(addr.s_addr);
	inet_aton("255.255.0.0", &addr);
	mask1 = ntohl(addr.s_addr);
	inet_aton("192.167.1.1", &addr);
	ip2 = ntohl(addr.s_addr);
	inet_aton("255.255.255.0", &addr);
	mask2 = ntohl(addr.s_addr);
	printf("192.168.1.1 255.255.0.0 192.168.1.1 255.255.255.0 = %d\n",
				is_net_overlap(ip1,mask1,ip2,mask2) );

	inet_aton("192.168.2.1", &addr);
	ip1 = ntohl(addr.s_addr);
	inet_aton("255.255.0.0", &addr);
	mask1 = ntohl(addr.s_addr);
	inet_aton("192.168.1.1", &addr);
	ip2 = ntohl(addr.s_addr);
	inet_aton("255.255.255.0", &addr);
	mask2 = ntohl(addr.s_addr);
	printf("192.168.2.1 255.255.0.0 192.168.1.1 255.255.255.0 = %d\n",
				is_net_overlap(ip1,mask1,ip2,mask2) );		
	return 0;
}
#endif

