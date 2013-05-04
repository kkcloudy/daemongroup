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
* wsm_shared_mem.h
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

#ifndef _WSM_SHARED_MEM_H
#define _WSM_SHARED_MEM_H

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <linux/ioctl.h>

#include "CWCommon.h"
#include "CWProtocol.h"
#include "CWNetwork.h"


#define PAGES					1024 
#define WSM_QUEUE_NUM			16  
#define PAGE_SIZE				4096
#define BUFF_SIZE				2048

#define WSM_FINISHED			1
#define WSM_NOT_DONE			0
#define WSM_NEW_BUFF			2

#define WSM_TOWIFI_TYPE		0x1
#define WSM_FROMWIFI_TYPE		0x2

#define WSM_QUEUE_LEN			(((PAGES / (WSM_QUEUE_NUM*2)) - 1) * 2)
#define WIFI_IOC_MMAP			_IOWR(243, 3, unsigned long long)
#define WIFI_IOC_VRRP_MMAP	_IOWR(243, 4, sh_mem_t)
#define ARRAY_LEN				WSM_QUEUE_LEN
#define PAGE_HEAD_LEN			sizeof(struct page_head)
#define __aligned					__attribute__((aligned(128)))
#define WIFI_PHY 0xA80000007

#ifdef __aligned
struct mmap_head {
	int offset;  /*from head of shared memory to target queue */
	int wifi_index;  /* only used by wifi */
	int wsm_index;  /* only used by wsm */
}  __aligned;
#else
struct mmap_head {
	int offset;  /* from head of shared memory to target queue */
	int wifi_index;  /* only used by wifi */
	int wsm_index;  /* only used by wsm */
};
#endif

struct page_head {
	volatile unsigned int finish;
	int len;
	unsigned int offset;
	unsigned int Wlanid;
	unsigned int BSSIndex;
};

struct WSM_MMAP_HEAD {
	struct mmap_head toWifi[WSM_QUEUE_NUM];
	struct mmap_head fromWifi[WSM_QUEUE_NUM];
};

struct wsm_index_t {
	int toWifiIndex;
};

struct wsm_data_t {
	int ac_fd;
	CWProtocolMessage cw_data;
	struct sockaddr_storage sock_data;
};

typedef struct vrid_shared_mem {
	unsigned int vrid;
	unsigned long long mm_addr;
}sh_mem_t;


int wsm_mmap_init(void);
__inline__ unsigned char* wsm_get_mem_node(int type, int index, int *tmp_index);
__inline__ void set_mem_index(int type, int index, unsigned char *buff);

#endif

