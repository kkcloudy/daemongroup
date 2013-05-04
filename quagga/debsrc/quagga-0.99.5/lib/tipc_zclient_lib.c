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
*tipc_zclient_lib.c
*
* MODIFY:
*		by <shancx@autelan.com> on 01/01/2011 revision <0.1>
*
* CREATOR:
*		shancx@autelan.com
*
* DESCRIPTION:
*		ac config information dynamic config
*
* DATE:
*		01/01/2011	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.10 $	
*******************************************************************************/



#include <zebra.h>

#include "prefix.h"
#include "stream.h"
#include "buffer.h"
#include "network.h"
#include "if.h"
#include "log.h"
#include "thread.h"
#include "zclient.h"
#include "memory.h"
#include "table.h"

#include <getopt.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <linux/tipc.h>
#include <unistd.h>


#include "zebra/zserv.h"
#include "tipc_zclient_lib.h"
#include "zebra/tipc_client.h"
#include "zebra/interface.h"
/**************************RPA***************************/
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>

/********************************************************/

#define MAX_DELAY 3000		/* inactivity limit [in ms] */
enum event {ZCLIENT_SCHEDULE, ZCLIENT_READ, ZCLIENT_CONNECT};

int tipc_zclient_debug = 0;
extern struct thread_master *master;
product_inf *product = NULL;
int board_type = -1;
unsigned int radio_interface_enable = 0;
char seed_time_flags = 0;
#define SEED_TIME_SET     (1 << 0)
#define RANGE_MIN      10000
#define RANGE_MAX	   100000
int route_boot_errno = 0;
int keep_kernel_mode = 0;


#define FILE_SLOT_TYPE "/var/run/slottype"

/*************RPA****************/


#define DEV_NAME "/dev/rpa0"

#define ETH_LEN		   16
/**
 * Command number for ioctl.
 */
#define RPA_IOC_MAGIC 245
#define RPA_IOC_MAXNR 0x16

#define RPA_IOC_REG_IF		_IOWR(RPA_IOC_MAGIC, 1, struct rpa_interface_info) // read values
#define RPA_IOC_UNREG_IF    _IOWR(RPA_IOC_MAGIC, 2, struct rpa_interface_info) // read values
#define RPA_IOC_INDEX_TABLE_ADD _IOWR(RPA_IOC_MAGIC, 3, struct cvm_rpa_dev_index_info)
#define RPA_IOC_INDEX_TABLE_DEL _IOWR(RPA_IOC_MAGIC, 4, struct cvm_rpa_dev_index_info)
#define RPA_IOC_INDEX_TABLE_MDF _IOWR(RPA_IOC_MAGIC, 5, struct cvm_rpa_dev_index_info)
#define RPA_IOC_NETIF_CARRIER_SET _IOWR(RPA_IOC_MAGIC, 7, struct rpa_netif_carrier_ioc_s)

typedef struct rpa_interface_info
{
	unsigned char  if_name[ETH_LEN];
	unsigned char  mac_addr[6];	
	unsigned int   mtu;
	unsigned int   type;
	unsigned short slotNum;
	unsigned short netdevNum;
	unsigned short devNum;
	unsigned short dev_portNum;
}RPA_info;

typedef struct cvm_rpa_dev_index_info
{
	int slotNum;
	int netdevNum;
	int flag;
	int ifindex;
}index_table_info;

typedef struct rpa_netif_carrier_ioc_s
{
	char if_name[ETH_LEN];
	int carrier_flag;
}rpa_netif_carrier_ioc_t;


/*************RPA****************/

/*gjd : add code for fetch a rand time interval by using srand and rand api .*/
int
fetch_rand_time_interval()
{
	unsigned int value = 0;

	if(!CHECK_FLAG(seed_time_flags,SEED_TIME_SET))
	{
		srand((unsigned int)time(NULL));
		SET_FLAG(seed_time_flags,SEED_TIME_SET);
	}
#if 0/*0--500*/
	value = (int)(500.0*rand()/(RAND_MAX + 1.0));/*between 0---500*/
	return value;
#else/*10000 -- 100000 : if use usleep means 10ms--110ms*/
	value = (((double)rand()/(double)RAND_MAX)*RANGE_MAX + RANGE_MIN);
	return value;

#endif	
}

/**gjd : add for Distribute Product , for check interface exist or not
when add interface between master and viace board**/
struct interface *
check_interface_exist_by_name (const char *name)
{
	return if_lookup_by_name(name);
}

struct interface *
check_interface_exist_by_name_len (const char *name,size_t namelen)
{
	return if_lookup_by_name_len(name,namelen);
}


/*ex: eth1-2 , eth1-2.100, eth1-2.100.100*/ 
int
eth_interface_name_split_by_point(const char *name)
{
	if(strncmp(name,"eth",3) != 0)
		return -1;
	char tmp[128] = {0};
	int i , count = 0;

	
	memcpy(tmp, name, strlen(name));
	for(i=0; i< strlen(name); i++)
	{
		if(tmp[i] == '.' )/*use '.' to make sure this eth interface is eth or eth sub or eth QinQ */
			count++;
	}
	/*zlog_info("########## %s : count = %d ##########\n",name,count);*/
	/*eth1-2 : count = 0; eth1-2.100: count = 1; eth1-2.100.100: count = 2.*/
	return count;
}

/*gujd : 2012-08-02, pm 6:17 . Add for ve sub from ve1.100 change to ve01f1.100 .*/
int
ve_sub_inteface_name_check(const char *name, char *name_return)
{
	char name_str[64]={0};
	char *endp = NULL;
	unsigned int slot_id = 0;
	
	memcpy(name_str,name,strlen(name));

	slot_id = strtoul(name_str+2,&endp,10);/*use name_str+2, cut "ve" .*/
	if(slot_id <= 0)
	{
		zlog_warn("%s : strtoul get slot_id failed(%s).\n",__func__,safe_strerror(errno));
		return -1;
		
	}
	if(endp)
	{
		if(endp[0]=='.')/*like ve1.100*/
		{
			if(slot_id >=1 && slot_id <= 9)/*slot 1--9, add to '01, 02, ..., 09' */
			{
				sprintf(name_return,"ve0%df1%s",slot_id,endp);/*consider as first cpu, so add 'f1' .*/
				return 0;
			}
			else/* > 9*/
			{
				sprintf(name_return,"ve%df1%s",slot_id,endp);/*consider as first cpu, so add 'f1' .*/
				return 0;
			}
		}
		else if(endp[0]=='f'||endp[0]=='s')/*like ve01f1.100*/
		{
			return 1;
		}
	}
	else
		{
			return -1;
		}
	
	
}

int
judge_rpa_interface(struct interface *ifp)
{
	int slot;
	int ret = 0;

	DISABLE_REDISTRIBUTE_INTERFACE_EXCEPT_RADIO(ifp->name, ret);/*disable redistribute interface*/
	if(ret == 1)
		return 0;

	if(ifp->if_types == RPA_INTERFACE)/*use if_types*/
		return RPA_INTERFACE;
	if(strncmp(ifp->name,"ve",2)==0)
		return 0;
		
	slot = get_slot_num(ifp->name);
	if(product && product->board_id != slot)/*use slot*/
		return RPA_INTERFACE;

	return 0;
}
int 
judge_oct_interface(const char *name)
{
	if(strncmp(name,"oct",3)==0)
		return OCT_INTERFACE;
	else
		return OTHER_INTERFACE;
}

int 
judge_pppoe_interface(const char *name)
{
	if(strncmp(name,"ppp",3)==0)
		return PPPOE_INTERFACE;
	else
		return OTHER_INTERFACE;
}

int 
judge_loop_interface(const char *name)
{
	if(strncmp(name,"lo",2)==0)
		return LOOP_INTERFACE;
	else
		return OTHER_INTERFACE;
}

int 
judge_sit0_interface(const char *name)
{
	if(strncmp(name,"sit0",4)==0)
		return SIT0_INTERFACE;
	else
		return OTHER_INTERFACE;
}
int 
judge_pimreg_interface(const char *name)
{
	if(strncmp(name,"pimreg",6)==0)
		return PIMREG_INTERFACE;
	else
		return OTHER_INTERFACE;
}

int 
judge_gre_interface(const char *name)
{
	if(strncmp(name,"gre",3)==0)
		return GRE_INTERFACE;
	else
		return OTHER_INTERFACE;
}


int 
judge_vlan_interface(const char *name)
{
	if(strncmp(name,"vlan",4)==0)
		return VLAN_INTERFACE;
	else
		return OTHER_INTERFACE;
}
int
judge_ve_interface(const char *name)
{
	if((strncmp(name,"ve",2)==0)&&(strlen(name) < VE_SUB_INTERFACE_NAME_MIN))
		return VE_INTERFACE;
	else
		return OTHER_INTERFACE;
}

int
judge_ve_sub_interface(const char *name)
{
	if((strncmp(name,"ve",2)==0)&&(strlen(name) >= VE_SUB_INTERFACE_NAME_MIN))
		return VE_SUB_INTERFACE;
	else
		return OTHER_INTERFACE;
}

int
judge_eth_interface(const char *name)
{
	/*if((strncmp(name,"eth",3)==0)&&(strlen(name) <= ETH_MAX))*/
	if((strncmp(name,"eth",3)==0)&&(eth_interface_name_split_by_point(name)== 0))
		return ETH_INTERFACE;
	else
		return OTHER_INTERFACE;
}

/*eth sub include : eth1-2.100(eth sub interface) and eth1-2.100.100(QinQ interface).
	In rtmd they are treat as same , not distinguish.*/
int
judge_eth_sub_interface(const char *name)
{
	/*if((strncmp(name,"eth",3)==0)&&(strlen(name) >= ETH_SUB_MIN))*/
	if((strncmp(name,"eth",3)==0)&&(eth_interface_name_split_by_point(name) > 0))
		return ETH_SUB_INTERFACE;
	else
		return OTHER_INTERFACE;
}

int
judge_eth_debug_interface(const char *name)
{
	if((strncmp(name,"eth3",4)==0)&&(strlen(name) == ETH_DEBUG_LEN))
		return ETH_DEBUG_INTERFACE;
	else
		return OTHER_INTERFACE;
}


int
judge_obc_interface(const char *name)
{
	if(strncmp(name,"obc",3)==0)
		return OBC_INTERFACE;
	else
		return OTHER_INTERFACE;
}

int
judge_radio_interface(const char *name)
{
	if(strncmp(name,"r",1)==0)
	{
		if(radio_interface_enable)
			return  ENABLE_RADIO_INTERFACE;
		else
			return  DISABLE_RADIO_INTERFACE;
		
		}
	
	return OTHER_INTERFACE;
}

int
judge_mng_interface(const char * name)
{
	if(strncmp(name,"mng",3)==0)
		return MNG_INTERFACE;
	else
		return OTHER_INTERFACE;
		
}

void
tipc_packet_create_header (struct stream *s, uint16_t cmd)
{
  /* length placeholder, caller can update */
  stream_putw (s, TIPC_PACKET_HEADER_SIZE);
  stream_putc (s, TIPC_PACKET_HEADER_MARKER);
  stream_putc (s, TIPC_PACKET_VERSION);
  stream_putw (s, cmd);
}

int get_slot_type(void)
{
	char* cmdstr="cat "FILE_SLOT_TYPE;
	char buf[32];
	int ret = 0;/*add*/
	FILE *fp = 	NULL;
	fp = popen( cmdstr, "r" );
	
	if(!fp)
		return -1;
	else
	{
		memset(buf,0,32);
		if(fgets(buf,32,fp))
			ret =  atoi(buf);
		pclose(fp);
	}
	/*CID 14332 (#1 of 1): Uninitialized scalar variable (UNINIT)
	4. uninit_use: Using uninitialized value "ret".
	Add initialize ret = 0*/
	return ret;
}

int get_slot_num(char *ifname)
{
	int slotnum;
	int num1,num2,num3,num4;
	int i , count = 0;
	char tmp[128] = {0};
	char cpu_num = 0;

	memcpy(tmp, ifname, strlen(ifname));

/*	zlog_debug("---------------------ifname = %s--------------------------\n",ifname);*/
	
	if(strncmp(ifname,"eth",3)==0)/*eth : cpu*/
	 {
	 	sscanf(ifname,"eth%d-%d",&slotnum,&num1);
		return slotnum;
		}
	
	/*gujd: 2012-06-06, am 11:03. Change the ve sub (like ve1.100) to support multi-core (like ve01f1.100).*/
	#if 0
	if(strncmp(ifname,"ve",2)==0)/*ve*/
	 {
		sscanf(ifname,"ve%d.%d",&slotnum,&num1);
		return slotnum;
		}
	#else
	else if(strncmp(ifname,"ve",2)==0)
	 {
	 	if(judge_ve_sub_interface(ifname)== VE_SUB_INTERFACE)
		 {
		 	/*ve01f1.100, ve05s1.200*/
			sscanf(ifname,"ve%d%c%d.%d",&slotnum,&cpu_num,&num1,&num2);
			zlog_info("name[%s],slot(%d),cpu(%c),cpu_n(%d),vlan_id(%d).\n",ifname,slotnum,cpu_num,num1,num2);
			return slotnum;
		 }
		else if(judge_ve_interface(ifname)== VE_INTERFACE)
		 {
		 	/*ve01f1, ve05s1*/
			sscanf(ifname,"ve%d%c%d",&slotnum,&cpu_num,&num1);
			zlog_info("name[%s],slot(%d),cpu(%c),cpu_n(%d).\n",ifname,slotnum,cpu_num,num1);
			return slotnum;
		 }
		else
		 {
		 	zlog_warn("%s : unknown ve interface type (%s).\n",__func__,ifname);
			return 0;
			}
	   }
	#endif
	else if(strncmp(ifname,"r",1) == 0)/*radio*/
	 {
	 	for(i = 0; i < strlen(ifname); i++)
	 	 {
	 	 	if(tmp[i] == '-')/*use '-' to make sure this radio is local board or remote board ?*/
				count++;
	 	  }
		if(count == 2)/*local board*/
		  {
			slotnum = product->board_id;
			return slotnum;
			}
		else if(count == 3)/*remote board*/
		  {
		  	sscanf(ifname,"r%d-%d-%d-%d.%d",&slotnum,&num1,&num2,&num3,&num4);
		  	return slotnum;
		  	}
		}
	else if(strncmp(ifname,"wlan",4) == 0)/*wlan*/
	 {
	 	for(i = 0; i < strlen(ifname); i++)
	 	 {
	 	 	if(tmp[i] == '-')/*use '-' to make sure this radio is local board or remote board ?*/
				count++;
	 	  }
		if(count == 1)/*local board*/
		  {
			slotnum = product->board_id;
			return slotnum;
			}
		else if(count == 2)/*remote board*/
		  {
		  	if(strncmp(ifname,"wlanl",5)==0)/*local hansi wlanlx-x-x*/
		  	{
		  	  sscanf(ifname,"wlanl%d-%d-%d",&slotnum,&num1,&num2);
		  	  return slotnum;
		  	}
			else
			{
		  	  sscanf(ifname,"wlan%d-%d-%d",&slotnum,&num1,&num2);/*remove hansi wlanx-x-x*/
		  	  return slotnum;
			}
		  }
	 	}
	else if(strncmp(ifname,"ebr",3) == 0 )/*ebr*/
	 {
	 	for(i = 0; i < strlen(ifname); i++)
	 	 {
	 	 	if(tmp[i] == '-')/*use '-' to make sure this radio is local board or remote board ?*/
				count++;
	 	  }
		if(count == 1)/*local board*/
		  {
			slotnum = product->board_id;
			return slotnum;
			}
		else if(count == 2)/*remote board*/
		  {
		  	if(strncmp(ifname,"ebrl",4)==0)/*local hansi ebrlx-x-x*/
		  	{
		  	  sscanf(ifname,"ebrl%d-%d-%d",&slotnum,&num1,&num2);
		  	  return slotnum;
		  	}
			else
			{
		  	  sscanf(ifname,"ebr%d-%d-%d",&slotnum,&num1,&num2);/*remove hansi ebrx-x-x*/
		  	  return slotnum;
			}
		  }
	 	}
	else if(strncmp(ifname,"obc0",4) == 0)
	{
			slotnum = product->board_id;
			return slotnum;
		}
	else if(strncmp(ifname,"mng",3)==0)/*mng : cpu*/
	 {
	 	sscanf(ifname,"mng%d-%d",&slotnum,&num1);
		return slotnum;
		}
	else if((strncmp(ifname,"mng",3)==0)
	  	||(strncmp(ifname,"sit0",4) == 0)
	  	||(strncmp(ifname,"pimreg",6) == 0)
	  	||(strncmp(ifname,"cnu",3) == 0)
	  	||(strncmp(ifname,"ppp",3) == 0)
	  	||(strncmp(ifname,"oct",3) == 0)
	  	||(strncmp(ifname,"lo",2) == 0))
	{
		slotnum = product->board_id;
		return slotnum;
	}
	else
	{
	
		zlog_warn("get slot num err[%s] !\n",ifname);
		return 0;/*err*/
	}
	

//	return slotnum;

}

/**gjd : register/unregister rpa table, used for local board **/
int register_rpa_interface_table(struct interface *ifp)
{
	int fd, ret;
	int flag;
	index_table_info dev_index_info;

	
	ifp->slot = get_slot_num(ifp->name);
	
	dev_index_info.slotNum = ifp->slot - 1;
	dev_index_info.netdevNum =ifp->devnum;

	/**0: not rpa dev (real interface), 1:rpa dev**/
	flag = 0;
	dev_index_info.flag = flag;
	dev_index_info.ifindex = ifp->ifindex;
	fd = open(DEV_NAME, O_RDWR);
	if(fd < 0)
	{
		zlog_warn("ioctl open error : %s .\n",safe_strerror(errno));
		return -1;
	}
		
	ret = ioctl(fd, RPA_IOC_INDEX_TABLE_ADD, &dev_index_info);
	if(ret < 0)
	{
		zlog_warn("ioctl error : %s .\n",safe_strerror(errno));
		close(fd);
		return -1;
	}

	close(fd);
	ifp->devnum = ret;/**fetch the devnum**/
	return 0;
		

}

int unregister_rpa_interface_table(struct interface *ifp)
{
	int fd, ret;
	index_table_info dev_index_info;

	ifp->slot = get_slot_num(ifp->name);
	
	dev_index_info.slotNum = ifp->slot - 1;
	dev_index_info.netdevNum = ifp->devnum;
	fd = open(DEV_NAME, O_RDWR);
	if(fd < 0)
	{
		zlog_warn("ioctl open error : %s .\n",safe_strerror(errno));
		return -1;
	}
		
	ret = ioctl(fd, RPA_IOC_INDEX_TABLE_DEL, &dev_index_info);
	if(ret < 0)
	{
		zlog_warn("ioctl error : %s .\n",safe_strerror(errno));
		close(fd);
		return -1;
	}
	close(fd);
	return 0;

}

#if 0
/**gjd:  creat/delete  rpa interface ,used for remote board **/
int create_rpa_interface(struct interface *ifp)
{
	int fd, ret = 0;
	//int i = 0;
    struct rpa_interface_info rpa_info;
	int slot_temp,port_temp;
	unsigned short slot_num, port_num;

	unsigned char mac[6] = {0x00, 0x1f, 0x64, 0x11, 0x02, 0xc0};
	zlog_debug("enter func %s ....to creat a rpa interface...\n",__func__);
	
	strncpy(rpa_info.if_name, ifp->name, ETH_LEN);
#ifdef TIPC_ZCLIENT_DEBUG
	zlog_debug(" rpa_info.if_name = %s\n", rpa_info.if_name);
#endif
	memcpy(rpa_info.mac_addr, mac, 6);
	rpa_info.type = 1;
#ifdef TIPC_ZCLIENT_DEBUG
	zlog_debug("%s : line %d, ifname is %s \n",__func__,__LINE__,ifp->name);
#endif
    sscanf(rpa_info.if_name,"eth%u-%u",&slot_temp,&port_temp);
	slot_num 	= slot_temp;
	port_num 	= port_temp;
	#ifdef TIPC_ZCLIENT_DEBUG
	zlog_debug("%s : line %d, slot_num = %u, port_num = %u \n",__func__,__LINE__,slot_temp,port_temp);

	zlog_debug("%s : line %d, slot_num = %u, port_num = %u \n",__func__,__LINE__,slot_num,port_num);
	#endif
	rpa_info.slotNum = slot_num -1;
	rpa_info.portNum = port_num ;
	#ifdef TIPC_ZCLIENT_DEBUG
	zlog_debug("%s : line %d, rpa_info.slotNum = %u, rpa_info.portNum = %u \n",__func__,__LINE__,rpa_info.slotNum,rpa_info.portNum);
	#endif

	fd = open(DEV_NAME, O_RDWR);
	if(fd == -1){
		zlog_debug("open error!\n");
		return -1;
	}
	
	ret = ioctl(fd, RPA_IOC_REG_IF, &rpa_info);
	if(ret == -1){
		zlog_debug("IOCTL error!\n");
		return -1;
	}

	zlog_debug("leave func %s ....to creat a rpa interface...\n",__func__);
	close(fd);

	return ret;
}
/**gjd: for del rpa**/
int delete_rpa_interface(struct interface*ifp)
{

  int fd, ret = 0;
  struct rpa_interface_info rpa_info;
  
  int slot_temp,port_temp;
  unsigned short slot_num, port_num;
 
 zlog_debug("enter func %s ....to delete a rpa interface...\n",__func__);

  unsigned char mac[6] = {0x00, 0x1f, 0x64, 0x11, 0x02, 0xc0};
  strncpy(rpa_info.if_name, ifp->name, ETH_LEN);
  #ifdef TIPC_ZCLIENT_DEBUG
  zlog_debug("[main()] : rpa_info.if_name = %s\n", rpa_info.if_name);
  #endif
  memcpy(rpa_info.mac_addr, mac, 6);
  
  rpa_info.type = 1;

    sscanf(rpa_info.if_name,"eth%u-%u",&slot_temp,&port_temp);
	slot_num 	= slot_temp;
	port_num 	= port_temp;
	#ifdef TIPC_ZCLIENT_DEBUG
	zlog_debug("%s : line %d, slot_num = %u, port_num = %u \n",__func__,__LINE__,slot_temp,port_temp);

	zlog_debug("%s : line %d, slot_num = %u, port_num = %u \n",__func__,__LINE__,slot_num,port_num);
    #endif
	rpa_info.slotNum = slot_num;
	rpa_info.portNum = port_num;
	#ifdef TIPC_ZCLIENT_DEBUG
	zlog_debug("%s : line %d, rpa_info.slotNum = %u, rpa_info.portNum = %u \n",__func__,__LINE__,rpa_info.slotNum,rpa_info.portNum);
  	#endif
  fd = open(DEV_NAME, O_RDWR);	

  if(fd == -1){			
  	zlog_debug("open error!\n");
	return -1;
  }

  ret = ioctl(fd, RPA_IOC_UNREG_IF, &rpa_info);

  if(ret == -1){
  	zlog_debug("IOCTL error!\n");
	return -1;
  }	
  
  zlog_debug("leave func %s ....to delete a rpa interface...\n",__func__);
  close(fd);
  return ret;
}	
#endif
int create_rpa_interface(struct interface * ifp)
{
	int fd, ret;
	struct rpa_interface_info rpa_info;
	int type = 0 ;/*add*/

	ifp->slot = get_slot_num(ifp->name);
	strncpy(rpa_info.if_name, ifp->name, ETH_LEN);
//	memcpy(rpa_info.mac_addr, mac, 6);

/*	if(product->product_type == PRODUCT_TYPE_76)*//*2011-7-26: add mac for 'obc0'(or say 've' ) for 7605i */
	if(product->product_type == PRODUCT_TYPE_7605I)/*2011-7-26: add mac for 'obc0'(or say 've' ) for 7605i */
	{
		if(strncmp(ifp->name,"ve01",4) == 0)
		{
			unsigned mac1[6] = {0x00, 0x1f, 0x64, 0x11, 0x02, 0xf0};
			memcpy(rpa_info.mac_addr, mac1 , 6);
		}
		if(strncmp(ifp->name,"ve02",4) == 0)
		{
			unsigned mac2[6] = {0x00, 0x1f, 0x64, 0x11, 0x02, 0xf1};
			memcpy(rpa_info.mac_addr, mac2 , 6);
		}
		if(strncmp(ifp->name,"ve03",4) == 0)
		{
			unsigned mac3[6] = {0x00, 0x1f, 0x64, 0x11, 0x02, 0xf2};
			memcpy(rpa_info.mac_addr, mac3 , 6);
		}
	}
	
	rpa_info.mtu = ifp->mtu;
	rpa_info.slotNum = ifp->slot - 1;
	rpa_info.netdevNum = ifp->devnum;
	if(strncmp(ifp->name,"eth",3)== 0)
		type = 1;
	if(strncmp(ifp->name,"ve",2)== 0)
		type = 2;
	if((strncmp(ifp->name,"wlan",4) == 0) || (strncmp(ifp->name,"ebr",3) == 0) 
		|| (strncmp(ifp->name,"r",1) == 0)
		|| (strncmp(ifp->name,"vlan",4) == 0) )
		type = 3;
	
	/*CID 14331 (#1 of 1): Uninitialized scalar variable (UNINIT)
	12. uninit_use: Using uninitialized value "type".
	Add initialize type=0*/
	rpa_info.type = type;
	fd = open(DEV_NAME, O_RDWR);
	if(fd < 0)
	{
		zlog_warn("ioctl open error : %s .\n",safe_strerror(errno));
		return -1;
	}
		
	ret = ioctl(fd, RPA_IOC_REG_IF, &rpa_info);
	if(ret < 0)
	{
		zlog_warn("ioctl error : %s .\n",safe_strerror(errno));
		close(fd);
		return -1;
	}
	if(ifp->devnum == 0)
	{
		ifp->devnum = ret;
	}

	close(fd);
	return 0;


}

int delete_rpa_interface(struct interface * ifp)
{
	int fd, ret;
	struct rpa_interface_info rpa_info;

	zlog_info(" Go to detele rpa interface %s .\n",ifp->name);
	ifp->slot = get_slot_num(ifp->name);
	rpa_info.slotNum = ifp->slot - 1;
	rpa_info.netdevNum = ifp->devnum;
	fd = open(DEV_NAME, O_RDWR);
	if(fd < 0)
	{
		zlog_warn("ioctl open error : %s .\n",safe_strerror(errno));
		return -1;
	}
			
	ret = ioctl(fd, RPA_IOC_UNREG_IF, &rpa_info);
	if(ret < 0)
	{			
		zlog_warn("ioctl error : %s .\n",safe_strerror(errno));
		close(fd);
		return -1;
	}
	close(fd);
	return 0;


}

int
rpa_interface_running_state_set(struct interface *ifp)
{
	int fd, ret;
	rpa_netif_carrier_ioc_t rpa_netif_carrier_info;

	strncpy(rpa_netif_carrier_info.if_name, ifp->name, ETH_LEN);
	rpa_netif_carrier_info.carrier_flag = 1;/*1 : set running state*/
	fd = open(DEV_NAME, O_RDWR);
	if(fd < 0)
	{
		zlog_warn("open error!\n");
		return -1;
	}
		
	ret = ioctl(fd, RPA_IOC_NETIF_CARRIER_SET, &rpa_netif_carrier_info);
	if(ret < 0)
	{
		zlog_warn("IOCTL error!\n");
		close(fd);
		return -1;
	}
	close(fd);
	return 0;

}

int
rpa_interface_running_state_unset(struct interface *ifp)
{
	int fd, ret;
	rpa_netif_carrier_ioc_t rpa_netif_carrier_info;

	strncpy(rpa_netif_carrier_info.if_name, ifp->name, ETH_LEN);
	rpa_netif_carrier_info.carrier_flag = 0;/*0 : unset running state*/
	fd = open(DEV_NAME, O_RDWR);
	if(fd < 0)
	{
		zlog_warn("open error!\n");
		return -1;
	}
		
	ret = ioctl(fd, RPA_IOC_NETIF_CARRIER_SET, &rpa_netif_carrier_info);
	if(ret < 0)
	{
		zlog_warn("IOCTL error!\n");
		close(fd);
		return -1;
	}
	close(fd);
	return 0;

}



#if 1
int 
search_ipv4(struct interface* ifp) 
{
   struct connected *c;
   struct listnode *cnode, *cnnode;
   int i = 0;
   
   for (ALL_LIST_ELEMENTS (ifp->connected, cnode, cnnode, c)) 
   {
		if (c->address->family == AF_INET) 
			++i;
   }
   return i; 
}
#endif

#if 1
/**gjd : under a interface , calculate the all ip address(include ipv4 and ipv6) num**/
int 
ip_address_count(struct interface* ifp) 
{
   struct connected *c;
   struct listnode *cnode, *cnnode;
   int i = 0;

   #if 0
   for (ALL_LIST_ELEMENTS (ifp->connected, cnode, cnnode, c)) 
   {
	//	if ((c->address->family == AF_INET)||(c->address->family == AF_INET6)) 
			
		if ((c->address->family == AF_INET)) 
			++i;
   }
   return i; 
   #endif

   if (ifp->connected)
    {
      for (ALL_LIST_ELEMENTS (ifp->connected, cnode, cnnode, c))
		{
/*		  	if(CHECK_FLAG (c->conf, ZEBRA_IFC_CONFIGURED))*/
				if(CHECK_FLAG (c->conf, ZEBRA_IFC_CONFIGURED)||CHECK_FLAG (c->conf, ZEBRA_IFC_REAL))
				i++;
		}
    }
   /*zlog_info("############## i = %d #################\n",i);*/
   return i;
   
}
#endif

/**Only for real interface first time register rpa table .not include ipv6 address: fe80: xx**/
int 
ip_address_count_except_fe80(struct interface* ifp) 
{
   struct connected *c;
   struct listnode *cnode, *cnnode;
   int i = 0;

   if (ifp->connected)
    {
      for (ALL_LIST_ELEMENTS (ifp->connected, cnode, cnnode, c))
		{
			if(CHECK_FLAG (c->conf, ZEBRA_IFC_CONFIGURED)||CHECK_FLAG (c->conf, ZEBRA_IFC_REAL))
			{
				if((c->address->family == AF_INET6) 
					&&(IPV6_ADDR_FE80(c->address->u.prefix6.in6_u.u6_addr16[0])))
					continue;
				else
					i++;
				}
				
		}
    }
   
  /* zlog_info("############## i = %d #################\n",i);*/
   return i;
   
}


/*judge the real_interface ,left for future use*/
int
judge_real_interface(char* if_name)
{
#if 0
	int local_if_slot;
	int if_slot;
	
//	local_if_slot = get_slot_type();
	if_slot = product->board_id;
#ifdef TIPC_ZCLIENT_DEBUG
	zlog_debug("%s: line  %d, product->board_id == %d\n",__func__,__LINE__, product->board_id);
#endif

	sscanf(if_name, "eth%d-", &local_if_slot);
#ifdef TIPC_ZCLIENT_DEBUG
	zlog_debug("%d, local_if_slot == %d\n",__LINE__,local_if_slot);
#endif
	if (if_slot == local_if_slot){
	//	ifp->if_types = REAL_INTERFACE;
		zlog_debug("%s, line == %d: %s is real_interface\n",__func__, __LINE__, if_name);
		return 1;
	} 
	zlog_debug("%s, line == %d: %s is not real_interface\n",__func__, __LINE__, if_name);
	return 0;
	#endif
	return 0;
}


/*judge the real local board interface , but cann't  useful for interface "obc", "pimreg", "lo", "ve parent", "sit0", "mng". */
int
judge_real_local_interface(char* ifname)
{
	int if_slot;

	if_slot = get_slot_num(ifname);/*fetch interface belongs to which board .*/
	
	if(product->board_id == if_slot)/*local board.*/
		
		return LOCAL_BOARD_INTERFACE;
	else							/*other board.*/
		
		return OTHER_BOARD_INTERFACE;

}

int get_peer_slot_from_tipc(int peer_fd)
{
	struct sockaddr temp;
	struct sockaddr_tipc *temp_tipc=(struct sockaddr_tipc *)&temp;
	int len;
	int ret = 0;
	len=sizeof(temp);
	if(!getpeername(peer_fd,(&temp),&len))
	{
		int a;
		zlog_debug("peer id is :%d\n",temp_tipc->addr.id.node);
		a=~0xfffff000 & (temp_tipc->addr.id.node);
		return a;
	}
	else
	{
		zlog_warn("get peer node error\n");
		return -1;
	}
	
}


unsigned int wait_for_server(void)
{
	struct sockaddr_tipc topsrv;
	struct tipc_subscr subscr = 
		{ {SERVER_NAME, 0, 0}, MAX_DELAY, TIPC_SUB_SERVICE, {} };
	struct tipc_event event;
	int sd;

	memset(&topsrv, 0, sizeof(topsrv));
	topsrv.family = AF_TIPC;
	topsrv.addrtype = TIPC_ADDR_NAME;
	topsrv.addr.name.name.type = TIPC_TOP_SRV;
	topsrv.addr.name.name.instance = TIPC_TOP_SRV;

	if ((sd = socket(AF_TIPC, SOCK_SEQPACKET, 0)) < 0) {
		zlog_warn("failed to create socket");
		return -1;
	}
	if (connect(sd, (struct sockaddr *)&topsrv, sizeof(topsrv)) < 0) {
		zlog_warn("failed to connect to topology server");
		/*CID 13488 (#1 of 17): Resource leak (RESOURCE_LEAK)
		6. leaked_handle: Handle variable "sd" going out of scope leaks the handle.
		Add close sd.*/
		close(sd);
		return -1;
	}
	if (send(sd, &subscr, sizeof(subscr), MSG_NOSIGNAL) != sizeof(subscr)) {
		zlog_warn("failed to send subscription");
		/*CID 13488 (#2 of 17): Resource leak (RESOURCE_LEAK)
		8. leaked_handle: Handle variable "sd" going out of scope leaks the handle.
		Add close sd.*/
		close(sd);
		return -1;
	}
	if (recv(sd, &event, sizeof(event), 0) != sizeof(event)) {
		zlog_warn("Failed to receive event");
		/*CID 13488 (#3 of 17): Resource leak (RESOURCE_LEAK)
		10. leaked_handle: Handle variable "sd" going out of scope leaks the handle.
		Add close sd.*/
		close(sd);
		return -1;
	}
	if (event.event != TIPC_PUBLISHED) {
		zlog_warn("Server %u,%u not published within %u [s]\n",
		       subscr.seq.type, subscr.seq.lower, MAX_DELAY/1000);
		/*CID 13488 (#4 of 17): Resource leak (RESOURCE_LEAK)
		11. leaked_handle: Handle variable "sd" going out of scope leaks the handle.
		Add close sd.*/
		close(sd);
		return -1;
	}
	close(sd);

	return event.port.node;
}


/* Make socket to zebra daemon. Return zebra socket. */
int
tipc_client_socket(int ins)
{
	int listener_sd = -1;
	struct sockaddr_tipc server_addr;
	int ret;

  /* We should think about IPv6 connection. */
  
  listener_sd = socket (AF_TIPC, SOCK_STREAM,0);
 /* listener_sd = socket (AF_TIPC, SOCK_SEQPACKET,0);*/
  if (listener_sd < 0)
    return -1;
  
  memset (&server_addr, 0, sizeof (struct sockaddr_tipc));
#if 0  
  server_addr.family = AF_TIPC;
  server_addr.addrtype = TIPC_ADDR_NAMESEQ;
  server_addr.addr.nameseq.type = SERVER_TYPE;
  server_addr.addr.nameseq.lower = 0;
  server_addr.addr.nameseq.upper = 0;
  server_addr.scope = TIPC_CLUSTER_SCOPE;
#else
	server_addr.family = AF_TIPC;
	server_addr.addrtype = TIPC_ADDR_NAME;
	server_addr.addr.name.name.type = SERVER_TYPE;
	server_addr.addr.name.name.instance = ins;
	server_addr.addr.name.domain = 0;

#endif
  /* Connect to zebra. */
  ret = connect (listener_sd, (struct sockaddr *) &server_addr, sizeof (server_addr));
  if (ret < 0)
    {
    
	  zlog_err("Can't connect: errno = %d %s\n",errno,safe_strerror (errno));
      close (listener_sd);
      return -1;
    }
  zlog_debug("client creat socket sucess\n");
  return listener_sd;
}


