

#ifndef __WIFI_SM_H__
#define __WIFI_SM_H__


#define   PAGES_ORDER   10
#define   PAGES   		1024
//#define	  PAGE_SIZE		4096
#define     BUFF_SIZE 2048
#define	  RX_OFFSET		0
#define	  TX_OFFSET		54
#define	  QUEUE_NUM		16
#define	  VM_BLOCK_SIZE	(PAGES*PAGE_SIZE/(QUEUE_NUM*2))
#define	  ARRAY_LEN		((PAGES/(QUEUE_NUM*2) - 1) * 2)
#define	  FINISHED		1
#define	  UNFINISHED	0	
#define	  COREMASK_OFFSET		VM_BLOCK_SIZE



struct page_index
{
	int offset;		//the offset from vm to the beginning of the queue
	unsigned int wifi_index;	//only used by wifi
	unsigned int wsm_index;	//only used by wsm
}__attribute__ ((aligned (128))); 

struct mmap_head
{
	struct page_index wsmTOwifi_array[QUEUE_NUM];
	struct page_index wifiTOwsm_array[QUEUE_NUM];
};

struct page_head
{
	int	finish;
	int	len;
	int	offset;	//802.3 offset
	int	wlanID;
	int BSSIndex;
};


#endif

