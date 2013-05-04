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

/* drp_log.h */
#ifndef _DRP_UTIL_H
#define _DRP_UTIL_H

#ifdef  __cplusplus
extern "C" {
#endif

#define MIN(a,b) ((a)>(b))?(b):(a)
#define MAX(a,b) ((a)>(b))?(a):(b)

int
sockopt_reuseaddr(int sock);
int 
set_nonblocking(int fd);
int 
clear_nonblocking(int fd);


struct timeval
 timeval_adjust(struct timeval a);
struct timeval
 timeval_subtract(struct timeval a, struct timeval b);
long
 timeval_cmp(struct timeval a, struct timeval b);
unsigned long
 timeval_elapsed(struct timeval a, struct timeval b);

void
rand_buff(unsigned char *buff, unsigned long size);
char *
inet_ntoa_ex(unsigned long ip, char *ipstr,
		   unsigned int buffsize);
int
mac2str(char *dst, unsigned long dstlen, unsigned char *src, char format);
int
str2mac(unsigned char *dst, unsigned long dstlen, char *src);
void 
delete_0d0a(char *buff);

int drp_check_ip_format(const char *str);

char * 
inet_int2str( unsigned int ip_addr, char *ip_str, unsigned int buffsize );

int
is_net_overlap( unsigned long ip1, unsigned long mask1,
					unsigned long ip2, unsigned long mask2 );

int
is_mask( unsigned long mask );



#ifdef  __cplusplus
}
#endif
#endif				/* _DRP_LOG_H */
