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
* capture.c
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

#ifndef __NM_STATUS_H__
#define __NM_STATUS_H__


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "ws_init_dbus.h"
#include <dbus/dbus.h>
#include "sysdef/npd_sysdef.h"
#include "dbus/npd/npd_dbus_def.h"
#include "util/npd_list.h"
#include "npd/nam/npd_amapi.h"
#include "ws_sysinfo.h"







#define NM_OUT(x) printf x
#define NM_DEBUG(x) printf x


#define HW_PCB_VER(ver) (((ver) & 0x0000FFFF) >> 8)
//indicate pcb version number,start with 0,then 1, 2, 3, and so on.
#define HW_CPLD_VER(ver) ((ver) & 0x000000FF)
//indicate cpld version number, start withs 0, then 1,2,3,

#define NPD_DBUS_BUSNAME "aw.npd"

#define NPD_DBUS_OBJPATH "/aw/npd"
#define NPD_DBUS_INTERFACE "aw.npd"

#define NPD_DBUS_ETHPORTS_OBJPATH "/aw/ethports"
#define NPD_DBUS_ETHPORTS_INTERFACE "aw.ethports"

#define NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_ETHPORT_LIST "show_ethport_list"
/*
NOTE: This method should also work for box product
If it's a box product, then total_slot_count is 1 and the mainboard information will be returned.
 arg lists for method NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_ETHPORT_LIST
  in arg list:
	NONE
  out arg list:  // in the order as they are appended in the dbus message.
	byte total_slot_count    // 1 for box, more for chassis
 	Array of ethport information of each slotslot
		byte slot no
		byte total_port_count
		Array of eth port information of this slot
			byte port no
			byte port type
			uint32 attr_bitmap
			uint32 MTU
		
*/

#define NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_ETHPORT_ONE "show_ethport_attr"

/*
 arg lists for method NPD_DBUS_INTERFACE_METHOD_VER
  in arg list:
	NONE
  out arg list:  // in the order as they are appended in the dbus message.
	uint32 product_id	// unsigned int of product type. uniquely identify a product, defined in npd_sysdef.h ,
	uint32 sw_version      // bitmap definition in npd_sysdef.h
  	string product_name  // backplane info for chassis product, board info for box product
	string product_base_mac_addr  // 12 char of mac address  with no : or - spliter.
  	string product_serial_number  // 32 bytes string
	string sw_name		// software description string
*/

#define NPD_DBUS_INTERFACE_METHOD_HWCONF "show_hwconf"

//modify by shaojunwu   the macro defined in Npd_sysdef.h

#define SOURLEN 10
#define BFLAG 3

#if 1
#include "sysdef/npd_sysdef.h"
#else
#define ETH_ATTR_ON 1
#define ETH_ATTR_OFF 0
#define ETH_ATTR_ENABLE 1
#define ETH_ATTR_DISABLE 0
#define ETH_ATTR_LINKUP 1
#define ETH_ATTR_LINKDOWN 0
#define ETH_ATTR_DUPLEX_FULL 1
#define ETH_ATTR_DUPLEX_HALF 0
#define ETH_ATTR_AUTONEG_DONE 1
#define ETH_ATTR_AUTONEG_NOT_DONE 0
#define ETH_ATTR_FC_ENABLE 1
#define ETH_ATTR_FC_DISABLE 0

// Bits 0~11 to have 12 kinds of binary attributes
#define ETH_ATTR_ADMIN_STATUS 	0x1		//bit0
#define ETH_ATTR_LINK_STATUS 		0x2		//bit1
#define ETH_ATTR_AUTONEG 			0x4		//bit2
#define ETH_ATTR_DUPLEX 			0x8		//bit3
#define ETH_ATTR_FLOWCTRL 		0x10	//bit4
#define ETH_ATTR_BACKPRESSURE 	0x20	//bit5

#define ETH_ADMIN_STATUS_BIT	0x0
#define ETH_LINK_STATUS_BIT	0x1
#define ETH_AUTONEG_BIT		0x2
#define ETH_DUPLEX_BIT			0x3
#define ETH_FLOWCTRL_BIT		0x4
#define ETH_BACKPRESSURE_BIT	0x5

#define ETH_ATTR_SPEED_MASK 		0xF000
#define ETH_ATTR_SPEED_MAX 		0xF

#define ETH_SPEED_BIT				0xC
#endif




 
extern char *slot_status_str[MODULE_STAT_MAX];
extern char *eth_port_type_str[ETH_MAX];
extern char *link_status_str[2];
extern char *doneOrnot_status_str[2];
extern char *onoff_status_str[2];
extern char *duplex_status_str[2];
extern char *eth_speed_str[ETH_ATTR_SPEED_MAX];
extern char *eth_media_str[3];



struct slot
{
   unsigned char module_status;  /*  slot_status_str[module_status]
                                              0--NONE
                                              1--INITING
                                              2--RUNNING
                                              3--DISABLED
                                         */
   char *modname;     //  DEFMODAX7-6GTX
   char *sn;          // DEFmodsn2222
   unsigned char hw_ver;        /*
                                       HW_PCB_VER(hw_ver),indicate pcb version number,start with 0,then 1,2,3,and so on.
                                           HW_CPLD_VER(hw_ver),indicate cpld version number,start with 0,then 1,2,3.
                                      */
   unsigned char ext_slot_num;  //扩展槽位  
   unsigned int module_id;   //模块 id
  
};

struct sys_envir
{   
   unsigned char fan_power;    /*AC风扇及三个电源风扇状态
                                            FAN     (fan_power & ( 1<<0 ))==1--normal       (fan_power & ( 1<<0 ))==0--alarm                                        
                                            M1       (fan_power & ( 1<<1))==1--normal       (fan_power & ( 1<<1 ))==0--alarm                                                
                                            M2       (fan_power & ( 1<<2))==1--normal       (fan_power & ( 1<<2 ))==0--alarm    
                                            M3	      (fan_power & ( 1<<3))==1--normal	   (fan_power & ( 1<<3 ))==0--alarm    
                                          */   
   unsigned short core_tmprt;       /*cpu core temperature*/
   unsigned short surface_tmprt;    /*cpu surface temperature*/
};



//extern int show_sys_ver(struct sys_ver *SysV);
extern int nm_show_hw_config(int sno, struct slot *SlotV);   //input slotnum(0--4),return slot information
extern int show_eth_port_atrr(unsigned int value,unsigned char type,struct global_ethport_s *PortV);/*value=slot_no; value=(value << 8) |port_no; type=0;*/
extern int show_sys_envir(struct sys_envir *SysV);   /*显示风扇状态及CPU温度*/

#endif

