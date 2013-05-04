/******************************************************************************
Copyright (C) Autelan Technology

This software file is owned and distributed by Autelan Technology 
*******************************************************************************

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
*******************************************************************************
* wsm_shared_mem.c
*
*
* DESCRIPTION:
*  Shared memory between wsm and wifi.ko.
*
* DATE:
*  2009-07-11
*
* CREATOR:
*  guoxb@autelan.com
*
* CHANGE LOG:
*  2008-07-11 <guoxb> Create file.
*  2010-02-09 <guoxb> Modified files name, copyright etc.
*
******************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/select.h>
#include <fcntl.h>
#include <linux/ioctl.h>
#include <syslog.h>

#include "wsm_types.h"
#include "wsm_shared_mem.h"
#include "hmd/hmdpub.h"

extern __inline__ void WSMLog(unsigned char type, char *format, ...);
extern unsigned int vrrid;

struct WSM_MMAP_HEAD *MM_Head;
struct mmap_head *toWifi_Head;
struct mmap_head *fromWifi_Head;
unsigned short gCoreMask;

#ifdef __wsm__

/** 
* Description:
*  WSM&Wiki.ko shared memory initialize function.
*  Wifi kernel module malloc memory and mapping it to a device in dev 
*  directory(/dev/wifi0).This is the shared memory between wsm and 
*  wifi.ko to transfer packet buffer.
*
* Parameter:
*  void.
*
* Return:
*  -1: failed, 0: Successed
*
*/
int wsm_mmap_init(void)
{
	int fd_oct = 0, fd = 0;
	sh_mem_t mem;
	char *vm = NULL;
	int i = 0;

	bzero(&mem, sizeof(sh_mem_t));
	mem.vrid = local*MAX_INSTANCE+vrrid;
	
	fd_oct = open("/dev/wifi0", O_RDWR);
//	WSMLog(L_CRIT, "%s: fd_oct=%d.\n", __func__,fd_oct);
//	printf("%s: fd_oct=%d.\n", __func__,fd_oct);
	if (ioctl(fd_oct, WIFI_IOC_VRRP_MMAP, &mem) < 0)
	{
		WSMLog(L_CRIT, "%s: ioctl failed.\n", __func__);
		return -1;
	}
	WSMLog(L_INFO, "#####test4 %s ,mem.mm_addr=%#x.\n",__func__,mem.mm_addr);
	if((mem.mm_addr & 0xf0000000) != 0x70000000){
		fd = fd_oct;
		WSMLog(L_INFO, "#####11test4 %s,fd=%d.\n",__func__,fd);
	}else{
		fd = open("/dev/mem", O_RDWR);
		WSMLog(L_INFO, "#####22test4 %s,fd=%d.\n",__func__,fd);
	}
	vm = mmap((0x70000000 + mem.vrid*0x400000), PAGES * PAGE_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, mem.mm_addr);
	
	WSMLog(L_INFO, "#####test4 %s ,vm=%p,mem.mm_addr=%llu.\n",__func__,vm,mem.mm_addr);
	if (vm == NULL)
	{
		WSMLog(L_CRIT, "%s: mmap failed. vm = NULL\n", __func__);
		return -1;
	}
	
	MM_Head = (struct WSM_MMAP_HEAD*)vm;
	gCoreMask = *((unsigned short *)(vm + PAGE_SIZE * (PAGES / (WSM_QUEUE_NUM * 2))));
	toWifi_Head = MM_Head->toWifi;
	fromWifi_Head = MM_Head->fromWifi;

	return 0;
}
#endif

/**
* Description:
*  From shared memory get a free node which used by transfer packet
*  to wifi.ko or received packet from wifi.ko.  
*
* Parameter:
*  type:  ToWifi or FromWifi type buffer
*  index: Number of Queue,in the wsm there has 16 Queues, and every 
*              queue has self-governed thread to deal with its. the index is
*              queue number, but also is thread index.
* 
* Return:
*  buff: successed, NULL: failed
*
*/
 __inline__ unsigned char* wsm_get_mem_node(int type, int index, int *tmp_index)
{
	unsigned char *buff = NULL;
	struct timespec tv;
	int loop_cnt = 0, loop_max = 256;

	tv.tv_sec = 0;
	tv.tv_nsec = 1;

	if (type == WSM_TOWIFI_TYPE)
	{	
		buff = (unsigned char*)(((unsigned char*)MM_Head + toWifi_Head[index].offset) + 
				(toWifi_Head[index].wsm_index) * BUFF_SIZE);
		*tmp_index = toWifi_Head[index].wsm_index;
			
		while(1)
		{
			if (((struct page_head*)buff)->finish == WSM_NEW_BUFF)
			{
				return buff;
			}
			else
			{
				++loop_cnt;
				if (loop_cnt > loop_max)
				{
					loop_cnt = 0;
			//		nanosleep(&tv, NULL);
					usleep(1000);	
				}
			}
		}
	}
	
	else if (type == WSM_FROMWIFI_TYPE)
	{
		buff = ((unsigned char*)MM_Head + fromWifi_Head[index].offset) + 
				(fromWifi_Head[index].wsm_index) * BUFF_SIZE;
			
		while(1)
		{
			if (((struct page_head*)buff)->finish == WSM_FINISHED)
			{
				return buff;
			}
			else
			{
				++loop_cnt;
				if (loop_cnt > loop_max)
				{
					loop_cnt = 0;
				//	nanosleep(&tv, NULL);
					usleep(1000);	
				}
			}
		}
	}

	else
	{
		WSMLog(L_ERR, "%s: unsupport type: %d\n", __func__, type);
		return NULL;
	}
}
 

/**
* Description:
*  Change the flag of shared memory node, when the packed has been assembled
*  completely, we should set the "flag" to tell partner who was waiting for this 
*  packet that you can get the packet now.  
*
* Parameter:
*  type:  type of to wifi packet or from wifi packet
*  index: Number of Queue,in the wsm there has 16 Queues, and every 
*              queue has self-governed thread to deal with its. the index is
*              queue number, but also is thread index.
*  buff: the buff who contain the assembled packet.
*
* Return:
*  void.
*
*/
__inline__ void set_mem_index(int type, int index, unsigned char *buff)
{
	if (type == WSM_TOWIFI_TYPE)
	{
		struct page_head *ptmp = (struct page_head*)buff;

		ptmp->finish = WSM_FINISHED;
		/* range of index: [0, WSM_QUEUW_LEN-2] */  
		toWifi_Head[index].wsm_index =  (toWifi_Head[index].wsm_index + 1) % WSM_QUEUE_LEN; 
			
		return;
	}

	else if (type == WSM_FROMWIFI_TYPE)
	{
		struct page_head *tmp = (struct page_head*)buff;

		tmp->finish = WSM_NOT_DONE;
		/* The first element is empty and not used */
		fromWifi_Head[index].wsm_index =  (fromWifi_Head[index].wsm_index + 1) % WSM_QUEUE_LEN; 
			
		return;
	}

	else
	{
		WSMLog(L_ERR, "%s: Unsupport type[%d].\n", __func__, type);
		return;
	}
}



