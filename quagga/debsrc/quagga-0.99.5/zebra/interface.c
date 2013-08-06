/*
 * Interface function.
 * Copyright (C) 1997, 1999 Kunihiro Ishiguro
 *
 * This file is part of GNU Zebra.
 *
 * GNU Zebra is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * GNU Zebra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Zebra; see the file COPYING.  If not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.  
 */

#include <zebra.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/sockios.h>
#include <linux/if_vlan.h>
#include "if.h"
#include "vty.h"
#include "sockunion.h"
#include "prefix.h"
#include "command.h"
#include "memory.h"
#include "ioctl.h"
#include "connected.h"
#include "log.h"
#include "zclient.h"

#include "zebra/interface.h"
#include "zebra/rtadv.h"
#include "zebra/rib.h"
#include "zebra/zserv.h"
#include "zebra/redistribute.h"
#include "zebra/debug.h"
#include "zebra/irdp.h"
#include "zebra/router-id.h"
#include "zebra/tipc_client.h"
#include "zebra/if_flow_stats.h"

#ifndef SEM_SLOT_COUNT_PATH
#define SEM_SLOT_COUNT_PATH         "/dbm/product/slotcount"
#endif

extern product_inf *product;
extern unsigned int radio_interface_enable ;
extern int route_boot_errno;
extern unsigned int radio_interface_show_enable;
unsigned int time_interval = 300 ;
extern struct cmd_node wireless_interface_node;
extern  unsigned int if_nametoindex (const char *name);
extern struct timeval current_time;

int get_product_info(char *filename)
{
	int fd;
	char buff[16] = {0};
	unsigned int data;
	
	/*CID 11588 (#1 of 3): Array compared against 0 (NO_EFFECT)
	array_null: Comparing an array to null is not useful: "buff == NULL".
	So delete buff == NULL.*/

	/*if((filename == NULL) || (buff == NULL))*/
	if(filename == NULL)
	{
		return CMD_FAILURE;
	}

	fd = open(filename, O_RDONLY, 0);
	if (fd >= 0) 
	{
		if(read(fd, buff, 16) < 0) 
			zlog_debug("Read error : no value\n");
		/* for bug: AXSSZFI-292, "popen failed for pager: Too many open files" */
		close(fd);
	}
	else
	{        
		zlog_debug("Open file:%s error!\n",filename);
		return CMD_FAILURE;
	}

	data = strtoul(buff, NULL, 10);

	return data;
}

__inline__ int parse_slot_tag_no(char *str,unsigned char *slotno, unsigned int *tag1, unsigned int *tag2)
	
{
	char *endptr = NULL;
	char *endptr1 = NULL, *endptr2 = NULL;
	char * tmpstr = str;
	int slotNum = 16;
	
	if(NULL == str || NULL == slotno)
	{
		return CMD_FAILURE;
	}
	*slotno = 0;

    if(!strncmp(str,"ve",2))
	{
		tmpstr = str+2;
	}
	else if(!strncmp(str,"obc",3))
	{
	    tmpstr = str+3;
	}
	if (NULL == tmpstr) 
	{
		return CMD_FAILURE;
	}
	
	if((tmpstr[0] > '9')||(tmpstr[0] < '0'))
	{
         return CMD_FAILURE;
	}

	*slotno = (unsigned char)strtoul(tmpstr,&endptr,10);
	
	slotNum = get_product_info(SEM_SLOT_COUNT_PATH);        
/*    if((*slotno < 0) || (*slotno > slotNum)) */
	if(slotNum>0 && (*slotno > slotNum))
        return CMD_FAILURE;

	if (endptr) 
	{
		
        if(endptr[0] == '\0') 
		{
			*tag1 = 0;
			*tag2 = 0;
			return CMD_SUCCESS;
		}		
        if(('.' == endptr[0])&&(('0' < endptr[1])&&('9' >= endptr[1])))
        {
        	*tag1 = strtoul((char *)&(endptr[1]),&endptr1,10);
			if(endptr1[0] == '\0') 
			{
				*tag2 = 0;
				return CMD_SUCCESS;
			}

            if(('.' == endptr1[0])&&(('0' < endptr1[1])&&('9' >= endptr1[1])))
            {
            	*tag2 = strtoul((char *)&(endptr1[1]),&endptr2,10);
    			
            	if((NULL == endptr2)||('\0' == endptr2[0]))
				{
            		return CMD_SUCCESS;
            	}
            	return CMD_FAILURE;
            }
        }
	}

	return CMD_FAILURE;	
}

int parse_ve_slot_cpu_tag_no
(
    char *str,unsigned char *slotno,
    unsigned char *cpu_no,
    unsigned char *cpu_port_no,
    unsigned int *tag1, unsigned int *tag2
)	
{
	char *endp = NULL;	
	char *endptr = NULL;
	char *endptr1 = NULL, *endptr2 = NULL;
	char * tmpstr = str;
	int slotNum = 16;
	
	if(NULL == str || NULL == slotno){
		return CMD_FAILURE;
	}
	*slotno = 0;

    if(!strncmp(str,"ve",2)){
		tmpstr = str+2;
	}else if(!strncmp(str,"obc",3)){
	    tmpstr = str+3;
	}
	if (NULL == tmpstr) 
	{
		return CMD_FAILURE;
	}
	
	if((tmpstr[0] > '9')||(tmpstr[0] < '0')){
         return CMD_FAILURE;
	}

	*slotno = (unsigned char)strtoul(tmpstr,&endp,10);
	
	slotNum = get_product_info(SEM_SLOT_COUNT_PATH);        
	if(slotNum>0 && (*slotno > slotNum))
        return CMD_FAILURE;

	if (endp) 
	{	
		/* get cpu no */
        if('f' == endp[0])
        {
			*cpu_no = 1;			
        }
		else if('s' == endp[0])
		{
			*cpu_no = 2;			
		}
		else
		{
        	return CMD_FAILURE;			
		}
		
		if(('0' < endp[1])&&('9' >= endp[1]))
		{
			/* get cpu port no */
        	*cpu_port_no = strtoul((char *)&(endp[1]),&endptr,10);
			
			/* get .tag1.tag2 */
            if(('.' == endptr[0])&&(('0' < endptr[1])&&('9' >= endptr[1])))
            {
            	*tag1 = strtoul((char *)&(endptr[1]),&endptr1,10);
    			if(endptr1[0] == '\0') {
    				*tag2 = 0;
    				return CMD_SUCCESS;
    			}

                if(('.' == endptr1[0])&&(('0' < endptr1[1])&&('9' >= endptr1[1])))
                {
                	*tag2 = strtoul((char *)&(endptr1[1]),&endptr2,10);
        			
                	if((NULL == endptr2)||('\0' == endptr2[0])){
                		return CMD_SUCCESS;
                	}
                	return CMD_FAILURE;
                }
            }		
		}
	}

	return CMD_FAILURE;	
}

__inline__ int parse_slotport_tag_no(char *str,unsigned char *slotno,unsigned char *portno,unsigned int * tag1,unsigned int *tag2)
{
	char *endptr = NULL;
	char *endptr2 = NULL;
	char *endptr3 = NULL;
	char *endptr4 = NULL;
	char * tmpstr = str;
	
	if(NULL == str)
	{
		return CMD_FAILURE;
	}
	if((NULL == slotno)||(NULL == portno)||(NULL == tag1))
	{
		return CMD_FAILURE;
	}
	*slotno = 0;
	*portno = 0;
	*tag1 = 0;
	if(NULL != tag2)
	{
		*tag2=0;
	}
	if(!strncmp(str,"eth",3) || !strncmp(str,"mng",3))
	{
		tmpstr = str+3;
	}
	if (NULL == tmpstr) {return -1;}
	if(((tmpstr[0] == '0')&&(tmpstr[1] != '-'))|| \
		(tmpstr[0] > '9')||(tmpstr[0] < '0'))
	{
         return CMD_FAILURE;
	}
	*portno = (unsigned char)strtoul(tmpstr,&endptr,10);
	if (endptr) 
	{
		if (('-' == endptr[0])&& \
			(('0' < endptr[1])&&('9' >= endptr[1])))
		{
            *slotno = *portno;
			*portno = (char)strtoul((char *)&(endptr[1]),&endptr2,10);
			/*}
			for eth1, eth1.2, eth1.2.3
			else endptr2 = endptr;
			{
			*/
			if(endptr2)
			{	
				if('\0' == endptr2[0])
				{
					*tag1 = 0;
					return CMD_SUCCESS;
				}
				else if(('.' == endptr2[0])&&\
					(('0' < endptr2[1])&&('9' >= endptr2[1])))
				{
					*tag1 = strtoul((char *)&(endptr2[1]),&endptr3,10);
					if((NULL == endptr3)||('\0' == endptr3[0]))
					{
						if(tag2) *tag2 = 0;
						return CMD_SUCCESS;
					}
					if(!tag2) return CMD_FAILURE;
					if((endptr3 != NULL)&&(endptr3[0] == '.'))
					{
						if(('0' >= endptr3[1])||('9' < endptr3[1]))
						{
							return CMD_FAILURE;
						}
						if(tag2)
						{
							*tag2 = strtoul((char *)&(endptr3[1]),&endptr4,10);
							if((endptr4 != NULL)&&(endptr4[0] != '\0'))
							{
								return CMD_FAILURE;
							}
						}
						return CMD_SUCCESS;
					}
					return CMD_FAILURE;
					
				}
				else
				{
					*tag1 = 0;
					return CMD_FAILURE;
				}
			}
			
			*tag1 = 0;
			if(tag2) *tag2 = 0;
			return CMD_SUCCESS;
		}
	}
	*slotno = 0;
	*tag1 = 0;
	if(tag2) *tag2 = 0;
	/* return 0;*/
	return CMD_FAILURE;	
}

/* add for lacp zhangchunlong@autelan.com 2012-12-12 */
static int intf_lacp_process(struct vty *vty,char *name)
{
	unsigned char slot = 0,port = 0;
	unsigned int vid1 = 0,vid2 = 0;
	int ret = CMD_FAILURE;
	int fd = CMD_FAILURE;
	int function_type = -1;

	char file_path[64] = {0};

	if(memcmp(name,"eth",3) == 0)
	{
		ret = parse_slotport_tag_no(name,&slot,&port,&vid1,&vid2);
		if(ret != CMD_SUCCESS)
		{
			zlog_debug("parse %s Failure \n",name);
			return CMD_FAILURE;
		}
		if((vid1 == 0) && (vid2 == 0) && (slot != 0) && (port != 0))
		{
			sprintf(file_path,"/dbm/product/slot/slot%d/function_type", slot);
			function_type = get_product_info(file_path);
			if(function_type == 0x4)/*switch board*/
			{
				/*if is switch board,do not save interface cmd*/
				return CMD_SUCCESS;
			}
			else
			{
				/*zlog_debug("not a switch board !!!\n");*/
				return CMD_FAILURE;
			}
		}
		else
		{
			/*zlog_debug("param do not match ! \n");*/
			return CMD_FAILURE;
		}
	}
	else
	{
		/*not a eth port */
		return CMD_FAILURE;
	}
}
/* add end */
/*gujd : 2012-07-07 , pm 5:00 . Add code for ve sub interface change mac.*/
int ve_sub_interface_set_mac(struct interface *ifp)
{
	macAddr_data_t macAddr_data;
	int slot_num = 0;
	int ret = 0;
	

	memset(&macAddr_data, 0, sizeof(macAddr_data_t));
	
	slot_num = get_slot_num(ifp->name);
	macAddr_data.slot_id = slot_num;
    cavim_do_intf_by_ioctl(CVM_IOC_GET_ETHER_HWADDR, &macAddr_data);

	ret = if_set_mac_addr(ifp->name, macAddr_data.hwaddr);
	if(ret < 0)
	{
		zlog_warn("set mac err: %s .\n",safe_strerror(errno));
		return -1;
		}
	return 0;
	
}

/*gujd: 2012-06-06, am 11:03. Change the ve sub add and delete .
   When delete ve sub interface , rtmd go to delete (instead od npd daemon before) by using "vconfig rem" .
   Add by using "vconfig add " .*/
int
vconfig_delete_ve_sub_interface(struct interface *ifp)
{
	char cmdstr[64] = {0};
	int ret =0;
	
	zlog_debug("%s : line %d , delete VE(%s).\n",__func__,__LINE__,ifp->name);
	memset(cmdstr, 0 ,64);
	/*sprintf(cmdstr, "vconfig add ve%d %s",slot_num,token);*/
	sprintf(cmdstr, "vconfig rem %s",ifp->name);
	ret = system(cmdstr);
	ret = WEXITSTATUS(ret);
	if(ret != 0)
	{
		zlog_warn("%s failed: %s !\n",cmdstr,safe_strerror(errno));
		return -1;
	}
	zlog_debug("%s : line %d , delete over sucess !\n",__func__,__LINE__);
	return 0;

}

int
judge_ve_sub_interface_QinQ(char *name)
{
	char name_char[INTERFACE_NAMSIZ + 1];
	int point_count = 0 , i;
	
	memset(name_char,0, sizeof(name_char));
	memcpy(name_char, name ,strlen(name));/*attention: don't use sizeof(name), name is a pointer*/

	for(i =0 ; i <= INTERFACE_NAMSIZ; i++)
	{
		if(name_char[i] == '.' )
			point_count++;
	}
	/*zlog_info("####point_count [%d]#####.\n",point_count);*/
	if(point_count == 2)
		return 1;/*is QinQ*/
	else
		return 0;
	
	
}

int
vconfig_create_ve_sub_interface(struct interface *ifp)/*will change 76 ,rename*/
{
	char name_str[INTERFACE_NAMSIZ + 1];
	char cmdstr[64] = {0};
	
	memset(name_str,0,sizeof(name_str));
	memcpy(name_str,ifp->name,sizeof(ifp->name));

	char *token = NULL;
	char *split = ".";
	int slot_num =0 ;
	char *first_name = NULL;
	int ret = 0;
	int is_QinQ = 0;

	is_QinQ = judge_ve_sub_interface_QinQ(ifp->name);/*to judge the QinQ interface. return 1 : is QinQ; 0: not.*/

	token = strtok(name_str, split);
	if(token == NULL)
		zlog_err("%s : line %d, split inteface name err !\n",__func__,__LINE__);
	else
	 {
	 	first_name = token;
	/* 	zlog_info("%s : ve sub split fist name is (%s).\n",__func__,first_name);*/
		}
	token = strtok(NULL,split);/*fetch veXX.xxx ,  xxx vlan id*/
	if(token == NULL)
		zlog_err("%s : line %d, split inteface name err !\n",__func__,__LINE__);
	else
		zlog_info("%s : line %d , vlan id is %s.\n",__func__,__LINE__,token);
	if(!is_QinQ)
	{
		/*frist delete*/
		memset(cmdstr, 0 ,64);
		sprintf(cmdstr, "vconfig add %s %s",first_name,token);
		zlog_info("%s : cmd_str[%s].\n",__func__,cmdstr);
		ret = system(cmdstr);
		ret = WEXITSTATUS(ret);
		if(ret != 0)
			zlog_warn("%s : vconfig add (%s) failed[%s].\n",__func__,ifp->name,safe_strerror(errno));
		else
		   zlog_debug("%s : line %d , add(%s) sucess over !\n",__func__,__LINE__,ifp->name);
		
		/*CID 11362 (#1 of 2): Missing return statement (MISSING_RETURN)
		8. missing_return: Arriving at the end of a function without returning a value.*/
		return 0;/*add 0*/
			
	}
	else/*QinQ*/
	{
		char name_vlanid1[INTERFACE_NAMSIZ + 1];
		memset(name_vlanid1,0,sizeof(name_vlanid1));
		sprintf(name_vlanid1,"%s.%s",first_name,token);/*here token is vlan_id1*/
		zlog_info("%s :QinQ of name_vlanid1[%s].\n",__func__,name_vlanid1);

		/*fetch vlan_id2*/
		token = strtok(NULL,split);/*fetch veXX.xxx.ooo ,  ooo vlan id2*/
		if(token == NULL)
			zlog_err("%s : line %d, split inteface name err !\n",__func__,__LINE__);
		else
			zlog_info("%s : line %d , vlan id2 is %s.\n",__func__,__LINE__,token);
		
		memset(cmdstr, 0 ,64);
		sprintf(cmdstr, "vconfig add %s %s",name_vlanid1,token);
		zlog_info("%s : cmd_str[%s].\n",__func__,cmdstr);
		ret = system(cmdstr);
		ret = WEXITSTATUS(ret);
		if(ret != 0)
			zlog_warn("%s : vconfig add (%s) failed[%s].\n",__func__,ifp->name,safe_strerror(errno));
		else
		   zlog_debug("%s : line %d , add(%s) sucess over !\n",__func__,__LINE__,ifp->name);
		
		/*CID 11362 (#2 of 2): Missing return statement (MISSING_RETURN)
		10. missing_return: Arriving at the end of a function without returning a value.*/
		return 0;/*change, add 0*/
		
	}

#if 0
	/*frist delete*/
	memset(cmdstr, 0 ,64);
	sprintf(cmdstr, "vconfig add %s %s",first_name,token);
	zlog_info("%s : cmd_str[%s].\n",__func__,cmdstr);
	ret = system(cmdstr);
	ret = WEXITSTATUS(ret);
	if(ret != 0)
		zlog_warn("%s : vconfig add (%s) failed[%s].\n",__func__,ifp->name,safe_strerror(errno));
	else
	   zlog_debug("%s : line %d , add(%s) sucess over !\n",__func__,__LINE__,ifp->name);
	
	return ;
#endif

}

/*gujd : 2012-06-14, pm 3:00.  Add code for 7605i creat ve sub interface . 
   First creat obc0.xxx, second rename obc0.xxx to ve0xf1.xxx .*/
void
vconfig_create_ve_sub_interface_7605i_local_board(struct interface * ifp)
{	
	char name_str[INTERFACE_NAMSIZ + 1];
	char cmdstr[64] = {0};
	char *token = NULL;
	char *split = ".";
	int slot_num =0 ;
	char *first_name = NULL;
	int ret = 0;
	char obc0_name[INTERFACE_NAMSIZ + 1];
	char *vlan_id = NULL;
	int is_QinQ = 0;
	
	is_QinQ = judge_ve_sub_interface_QinQ(ifp->name);/*to judge the QinQ interface. return 1 : is QinQ; 0: not.*/
	/*zlog_info("#########QinQ[%d]########\n",is_QinQ);*/
	
	memset(name_str,0,sizeof(name_str));
	/*the ifp->name is ve01f1.xxx*/
	memcpy(name_str,ifp->name,sizeof(ifp->name));
	memset(cmdstr, 0 ,64);

	token = strtok(name_str, split);
	if(token == NULL)
	 {
		zlog_warn("%s : line %d, split inteface(%s) name err !\n",__func__,__LINE__,name_str);
		return ;
		}
	else
	 {
	 	first_name = token;
	/* 	zlog_info("%s : ve sub split fist name is (%s).\n",__func__,first_name);*/
		}
	token = strtok(NULL,split);/*fetch obc0.xxx ,  xxx vlan id*/
	if(token == NULL)
	 {
		zlog_warn("%s : line %d, split inteface(%s) name err !\n",__func__,__LINE__,name_str);
		return ;
		}
	else
	 {
		zlog_info("%s : line %d , vlan id is %s.\n",__func__,__LINE__,token);
		vlan_id = token;
		}
	
	if(!is_QinQ)
	{
		/*first creat obc0.xxx, token is vlan id*/
		sprintf(cmdstr, "vconfig add obc0 %s",token);
		zlog_info("%s : cmd_str[%s].\n",__func__,cmdstr);
		ret = system(cmdstr);
		ret = WEXITSTATUS(ret);
		if(ret != 0)
		 {
			zlog_warn("%s : vconfig add (%s) failed[%s].\n",__func__,ifp->name,safe_strerror(errno));
			return ;
			}
		else
		   zlog_debug("%s : line %d , add(%s) sucess over !\n",__func__,__LINE__,ifp->name);
		
		/*second rename obc0.xxx to ve01f1.xxx. 
		 Notice: The crrent ifname in kernel is obc0.xxx, 
		            but ifp->name now is ve01f1.xxx, so to update the ifname in kernel by ioctl. */
		memset(obc0_name,0,sizeof(obc0_name));
		sprintf(obc0_name,"obc0.%s",vlan_id);
		ret = if_set_name(ifp,obc0_name);
		if(ret < 0)
			zlog_warn("rename failed: %s.\n",safe_strerror(errno));
		
		/*gujd : 2012-07-07 , pm 5:00 . Add code for ve sub interface change mac.*/
		ve_sub_interface_set_mac(ifp);
		
		return ;
	}
	else
	{
		char name_vlanid1[INTERFACE_NAMSIZ + 1];
		memset(name_vlanid1,0,sizeof(name_vlanid1));
		sprintf(name_vlanid1,"%s.%s",first_name,token);/*here token is vlan_id1*/
		zlog_info("%s :QinQ of name_vlanid1[%s].\n",__func__,name_vlanid1);

		/*fetch vlan_id2*/
		token = strtok(NULL,split);/*fetch veXX.xxx.ooo ,  ooo vlan id2*/
		if(token == NULL)
			zlog_err("%s : line %d, split inteface name err !\n",__func__,__LINE__);
		else
			zlog_info("%s : line %d , vlan id2 is %s.\n",__func__,__LINE__,token);
		
		memset(cmdstr, 0 ,64);
		sprintf(cmdstr, "vconfig add %s %s",name_vlanid1,token);
		zlog_info("%s : cmd_str[%s].\n",__func__,cmdstr);
		ret = system(cmdstr);
		ret = WEXITSTATUS(ret);
		if(ret != 0)
			zlog_warn("%s : vconfig add (%s) failed[%s].\n",__func__,ifp->name,safe_strerror(errno));
		else
		   zlog_debug("%s : line %d , add(%s) sucess over !\n",__func__,__LINE__,ifp->name);
		
		return ;
		
	}

}

/*gujd : 2012-07-07, pm 3:00.  Add code for 7605i creat ve sub interface . 
   Direct creat ve0xf1.xxx .*/
void
vconfig_create_ve_sub_interface_7605i_other_board(struct interface * ifp)
{	
	char name_str[INTERFACE_NAMSIZ + 1];
	char cmdstr[64] = {0};
	char *token = NULL;
	char *split = ".";
	int slot_num =0 ;
	char *first_name = NULL;
	int ret = 0;
	char obc0_name[INTERFACE_NAMSIZ + 1];
	char *vlan_id = NULL;
	int is_QinQ = 0;
	
	is_QinQ = judge_ve_sub_interface_QinQ(ifp->name);/*to judge the QinQ interface. return 1 : is QinQ; 0: not.*/
	/*zlog_info("#########QinQ[%d]########\n",is_QinQ);*/
	
	memset(name_str,0,sizeof(name_str));
	/*the ifp->name is ve01f1.xxx*/
	memcpy(name_str,ifp->name,sizeof(ifp->name));
	memset(cmdstr, 0 ,64);

	token = strtok(name_str, split);
	if(token == NULL)
	 {
		zlog_warn("%s : line %d, split inteface(%s) name err !\n",__func__,__LINE__,name_str);
		return ;
		}
	else
	 {
	 	first_name = token;
	/* 	zlog_info("%s : ve sub split fist name is (%s).\n",__func__,first_name);*/
		}
	token = strtok(NULL,split);/*fetch obc0.xxx ,  xxx vlan id*/
	if(token == NULL)
	 {
		zlog_warn("%s : line %d, split inteface(%s) name err !\n",__func__,__LINE__,name_str);
		return ;
		}
	else
	 {
		zlog_info("%s : line %d , vlan id is %s.\n",__func__,__LINE__,token);
		vlan_id = token;
		}
	
	if(!is_QinQ)
	{
		/*first creat ve01f1.xxx, token is vlan id*/
		sprintf(cmdstr, "vconfig add %s %s",first_name,token);
		zlog_info("%s : cmd_str[%s].\n",__func__,cmdstr);
		ret = system(cmdstr);
		ret = WEXITSTATUS(ret);
		if(ret != 0)
		 {
			zlog_warn("%s : vconfig add (%s) failed[%s].\n",__func__,ifp->name,safe_strerror(errno));
			return ;
			}
		else
		   zlog_debug("%s : line %d , add(%s) sucess over !\n",__func__,__LINE__,ifp->name);
	#if 0	
		/*second rename obc0.xxx to ve01f1.xxx. 
		 Notice: The crrent ifname in kernel is obc0.xxx, 
		            but ifp->name now is ve01f1.xxx, so to update the ifname in kernel by ioctl. */
		memset(obc0_name,0,sizeof(obc0_name));
		sprintf(obc0_name,"obc0.%s",vlan_id);
		ret = if_set_name(ifp,obc0_name);
		if(ret < 0)
			zlog_warn("rename failed: %s.\n",safe_strerror(errno));
	#endif	
		
		return ;
	}
	else
	{
		char name_vlanid1[INTERFACE_NAMSIZ + 1];
		memset(name_vlanid1,0,sizeof(name_vlanid1));
		sprintf(name_vlanid1,"%s.%s",first_name,token);/*here token is vlan_id1*/
		zlog_info("%s :QinQ of name_vlanid1[%s].\n",__func__,name_vlanid1);

		/*fetch vlan_id2*/
		token = strtok(NULL,split);/*fetch veXX.xxx.ooo ,  ooo vlan id2*/
		if(token == NULL)
			zlog_err("%s : line %d, split inteface name err !\n",__func__,__LINE__);
		else
			zlog_info("%s : line %d , vlan id2 is %s.\n",__func__,__LINE__,token);
		
		memset(cmdstr, 0 ,64);
		sprintf(cmdstr, "vconfig add %s %s",name_vlanid1,token);
		zlog_info("%s : cmd_str[%s].\n",__func__,cmdstr);
		ret = system(cmdstr);
		ret = WEXITSTATUS(ret);
		if(ret != 0)
			zlog_warn("%s : vconfig add (%s) failed[%s].\n",__func__,ifp->name,safe_strerror(errno));
		else
		   zlog_debug("%s : line %d , add(%s) sucess over !\n",__func__,__LINE__,ifp->name);
		
		return ;
		
	}

}

struct interface *
create_rpa_interface_and_get_ifindex(struct interface * ifp)
{
	int ret;

	ret = create_rpa_interface(ifp);
	 if(ret < 0)
	 {
	 	zlog_debug("%s : line %d, creat rpa interface failed : %s \n ",__func__,__LINE__,safe_strerror(errno));
		/*return NULL;*/
		return ifp;
	 }
	 else
	 {	
		/*virtual interface ==> RPA interface*/
		ifp->if_types = RPA_INTERFACE;
	 	zlog_debug("%s : line %d, creat rpa interface(%s) sucess, so virtual turn to rpa interface...\n",__func__,__LINE__,ifp->name);
	 	}
	 /*gujd : 2012-10-13 . Optimized code . Get index by netlink.*/
#if 0
	 sleep(1);
	 
	 ifp->ifindex = if_get_index_by_ioctl(ifp);
  	 if(ifp->ifindex <= 0)
  	  {
  		zlog_warn("get rpa ifindex by ioctl failed : %s ..\n",safe_strerror(errno));
		/*return NULL;*/
		return ifp;
  	  }
 	 zlog_debug("get rpa ifindex by ioctl sucess is %d .....",ifp->ifindex);
#endif

	/*for ve interface (rpa) set running state*/
	 ret = rpa_interface_running_state_set(ifp);
	 if(ret < 0)
	 {
	 	zlog_warn("set ve interface (rpa) running state failed : %s ..\n",safe_strerror(errno));
		
	 }
/*	 zlog_debug("set ve interface (rpa) running state sucess ..\n");*/
	 return ifp;
}

void
eth_interface_rpa_init()
{	
	struct interface *ifp = NULL;
	int ret;
	int id_mask = -1;
	int board_state[32] = {0};
	int i,slot = -1, num;;
	char name[32];
	
	memset(name, 0, 32);	
	if(product == NULL)
		return;
	#if 0
	if(product->board_type == BOARD_IS_VICE)
		return;
	#endif
	id_mask = product->alive_board_slotid;

	for(i = 0; i < 32 ; i++)
	 {
	 	if( (id_mask >> i) & 1)
	 	 {
	 	 	board_state[i] = BOARD_ON_PRODUCT;
	 	 }
		else
			board_state[i] = BOARD_OFF_PRODUCT;

	}
/*	if(product->product_type == PRODUCT_TYPE_76)*//*7605:cpu ethX-X*/
	if(product->product_type == PRODUCT_TYPE_7605I)/*7605:cpu ethX-X*/
	{
	
		if((board_state[0] == BOARD_ON_PRODUCT)&&(product->board_id == 1))
		{
		
			slot = 1;
			for(num = 1; num<=4; num++)
			{
			
				sprintf(name, "eth%d-%d", slot, num);
				ifp = if_get_by_name(name);
				if(ifp == NULL)
				{
				
					zlog_info("creat %s (viral) interface failed\n",name);
					continue;
				}
				ifp->devnum = 0;
				ret = register_rpa_interface_table(ifp);
				if(ret < 0)
					zlog_warn("%s : when system start , init rpa(%s) table failed (%s).\n",
						__func__,ifp->name,safe_strerror(errno));
			  }
			}

		if((board_state[1] == BOARD_ON_PRODUCT) &&(product->board_id == 2))
		{
		
			slot = 2;
			for(num = 1; num<=4; num++)
			{
			
				sprintf(name, "eth%d-%d", slot, num);
				ifp = if_get_by_name(name);
				if(ifp == NULL)
				{
				
					zlog_info("creat %s (viral) interface failed\n",name);
					continue;
				}
				ifp->devnum = 0;
				ret = register_rpa_interface_table(ifp);
				if(ret < 0)
					zlog_warn("%s : when system start , init rpa(%s) table failed (%s).\n",
					 __func__,ifp->name,safe_strerror(errno));
			  }
			}
		
	}
	/*86XX*/
	if(product->product_type == PRODUCT_TYPE_8603)/*8603 :cpu ethX-X*/
	{
		/*slot 1 : 8C or 12C*/
		if((board_state[0] == BOARD_ON_PRODUCT)&&(product->board_id == 1))
		{
		
			slot = 1;
			/*8C: at present is eth1-1, 1-2,1-3,1-4. 2012-05-07, pm 3:04. gujd*/
			if(strncmp(product->board_name,AX81_AC8C,sizeof(AX81_AC8C)) == 0)
			{
				/*zlog_info("line %d, #######%s[%d]######\n",__LINE__,product->board_name,product->board_id);*/
				for(num = 1; num<=4; num++)
				{
				
					sprintf(name, "eth%d-%d", slot, num);
					ifp = if_get_by_name(name);
					if(ifp == NULL)
					{
					
						zlog_info("creat %s (viral) interface failed\n",name);
						continue;
					}
					ifp->devnum = 0;
					ret = register_rpa_interface_table(ifp);
					if(ret < 0)
					zlog_warn("%s : when system start , init rpa(%s) table failed (%s).\n",
						__func__,ifp->name,safe_strerror(errno));
				  }
				}
			else if(strncmp(product->board_name,AX81_AC12C,sizeof(AX81_AC12C)) == 0)
				/*12C: at present is eth1-5, 1-6, pm 3:04. gujd*/
			  {
				/*  zlog_info("line %d, ######%s[%d]######\n",__LINE__,product->board_name,product->board_id);*/
				/*for(num = 5; num<=6; num++)*/
				/*gujd : 2013-03-14, am 11:00. Enlarge num form 1 to 6 , for 12C board, supporting eth1-1, 1-2,..., 1-6*/
				for(num = 1; num<=6; num++)
				{
				
					sprintf(name, "eth%d-%d", slot, num);
					ifp = if_get_by_name(name);
					if(ifp == NULL)
					{
					
						zlog_info("creat %s (viral) interface failed\n",name);
						continue;
					}
					ifp->devnum = 0;
					ret = register_rpa_interface_table(ifp);
					if(ret < 0)
					 zlog_warn("%s : when system start , init rpa(%s) table failed (%s).\n",
						__func__,ifp->name,safe_strerror(errno));
				  }
				}
			
			}

		/*slot 2 : 8C or 12C*/
		if((board_state[1] == BOARD_ON_PRODUCT)&&(product->board_id == 2))
		{
			slot = 2;
			/*8C: at present is eth2-1, 2-2,2-3,2-4. 2012-05-07, pm 3:15. gujd*/
			if(strncmp(product->board_name,AX81_AC8C,sizeof(AX81_AC8C)) == 0)
			{
				/*zlog_info("line %d, ######%s[%d]#####\n",__LINE__,product->board_name,product->board_id);*/
				for(num = 1; num<=4; num++)
				{
				
					sprintf(name, "eth%d-%d", slot, num);
					ifp = if_get_by_name(name);
					if(ifp == NULL)
					{
					
						zlog_info("creat %s (viral) interface failed\n",name);
						continue;
					}
					ifp->devnum = 0;
					ret = register_rpa_interface_table(ifp);
					if(ret < 0)
					zlog_warn("%s : when system start , init rpa(%s) table failed (%s).\n",
						__func__,ifp->name,safe_strerror(errno));
				  }
				}
			else if(strncmp(product->board_name,AX81_AC12C,sizeof(AX81_AC12C)) == 0)
				/*12C: at present is eth2-5, 2-6, pm 3:15. gujd*/
			  {
				/*  zlog_info("line %d, ######%s[%d]######\n",__LINE__,product->board_name,product->board_id);*/
				/*for(num = 5; num<=6; num++)*/				
				/*gujd : 2013-03-14, am 11:00. Enlarge num form 1 to 6 , for 12C board, supporting eth1-1, 1-2,..., 1-6*/
				for(num = 1; num<=6; num++)
				{
				
					sprintf(name, "eth%d-%d", slot, num);
					ifp = if_get_by_name(name);
					if(ifp == NULL)
					{
					
						zlog_info("creat %s (viral) interface failed\n",name);
						continue;
					}
					ifp->devnum = 0;
					ret = register_rpa_interface_table(ifp);
					if(ret < 0)
					 zlog_warn("%s : when system start , init rpa(%s) table failed (%s).\n",
						__func__,ifp->name,safe_strerror(errno));
				  }
				}
			
			}
				
	}

	/*8606, 8610, don't need.*/

		
}

/*gujd: 2012-06-06, am 11:03. Add for changing the ve sub (like ve1.100) to support multi-core (like ve01f1.100).*/
void
register_rpa_interface_table_byname(char *name)
{
	struct interface *ifp = NULL;
	int ret = 0;
	
	ifp = if_lookup_by_name(name);
	if(ifp == NULL)
	{
		zlog_info("Not Find [%s] .\n",name);
		return ;
	}
	ret = register_rpa_interface_table(ifp);
	if(ret < 0)
		zlog_info("Add [%s] to rpa table failed .\n",ifp->name);

	return ;
}

void
ve_interface_rpa_init_7605i()
{	
	struct interface *ifp = NULL;
	int ret;
	int id_mask = -1;
	int board_state[32] = {0};
	int i,slot = -1;
	char name[32];
	
	memset(name, 0, 32);	
	/*
	if(product == NULL)
		return;
	*/

	if(product->product_type != PRODUCT_TYPE_7605I)
	{
		zlog_warn(" Not 7605i product ,so retrun .\n");
		return;
	}

	id_mask = product->alive_board_slotid;

	for(i = 0; i < 32 ; i++)
	 {
	 	if( (id_mask >> i) & 1)
	 	 {
	 	 	board_state[i] = BOARD_ON_PRODUCT;
	 	 }
		else
			board_state[i] = BOARD_OFF_PRODUCT;

	}

	for( i = 0; i < 32 ; i++ )
    {
  	 if(board_state[i] == BOARD_ON_PRODUCT)
  	 {
  	 	slot = i + 1;
	   /*
	   	not local , creat ve rpa. Foe example:
		slot 1 creat  ve02f1, ve03f1; 
		slot 2 creat  ve01f1, ve03f1; 
		slot 3 creat  ve01f1, ve02f1 .
		*/
  	 	if(product->board_id != slot)
  	 	{
  	 		sprintf(name, "ve0%df1", slot);
			ifp = if_get_by_name(name);
			if(ifp == NULL)
			{
				zlog_info("creat %s (viral) interface failed\n",name);
				return;
				}
		
			ifp = create_rpa_interface_and_get_ifindex(ifp);/**creat rpa and register**/
			
			if(ifp->ifindex == IFINDEX_INTERNAL)
			   zlog_info("creat ve(rpa) interface failed\n");
			
  	 	}
		else/*local : registet rpa table*/
		{
			memset(name,0,32);/*一定要有，排除上面sprintf(name, "ve0%df1", slot);影响*/
			strncpy(name,"obc0",4);
			ifp = if_get_by_name(name);
			if(ifp == NULL)
			{
				zlog_info("register %s (viral) interface failed\n",name);
				return;
			}
		
			register_rpa_interface_table(ifp);				
	  	
		}
		
  	 }
  }
		
}

/*gujd: 2012-06-06, am 11:03. Add for changing the ve sub (like ve1.100) to support multi-core (like ve01f1.100).*/
void
ve_interface_rpa_init_v2()
{	
	struct interface *ifp = NULL;
	int ret;
	int slot = -1;
	char name[32];
	
	memset(name, 0, 32);	
	if(product == NULL )
		return;
	if(product->product_type == PRODUCT_TYPE_7605I)
	{
		ve_interface_rpa_init_7605i();
		return;
	}
	
	if(((product->product_type == PRODUCT_TYPE_8603)
		||(product->product_type == PRODUCT_TYPE_8606)
		||(product->product_type == PRODUCT_TYPE_8610)
		||(product->product_type == PRODUCT_TYPE_8800))
		&&(strncmp(product->board_name,AX81_2X12G12S,sizeof(AX81_2X12G12S) == 0)))/*86xx: switch board don't go to creat ve rpa*/
	{
		zlog_debug(" The board name is %s , don't go to creat ve(rpa) .\n",product->board_name);
		return;
	}
	slot = product->board_id;
/*
8c: ve01f1
12c: ve01f1   ve01s1  
1x:  ve01f1
2x: no 
12x : no
4x:  ve01f1  ve01s1 ve01s2 ve01s3 ve01s4 
81SMU:  ve05f1
71SMU:  obc0
*/
	  
	  /*86XX: veX */
	  if(strncmp(product->board_name,AX81_SMU,sizeof(AX81_SMU)) == 0)/*81_smu*/
  		{
  		   /*
  			zlog_debug("86: The board %s is master, skip yoursel to registe ve(rpa) table .\n",product->board_name);
  			continue;
  			*/
  		#if 0
		   sprintf(name, "ve0%df1", slot);
  		   zlog_info("%s: 81_SMU system init to register [%s] rpa table.\n",__func__,name);
		   register_rpa_interface_table_byname(name);
		#else
		   sprintf(name, "ve0%df1", slot);/*8610: ve05f1 or ve06f1. 8606: ve03f1 or ve04f1.*/
  		   zlog_info("%s: 81_SMU system init to creat [%s] rpa .\n",__func__,name);
		   ifp = if_get_by_name(name);
		   if(ifp == NULL)
			{
				zlog_info("creat %s (viral) interface failed\n",name);
				return;
			}
			ifp = create_rpa_interface_and_get_ifindex(ifp);/**creat rpa and register**/
			
			if(ifp->ifindex == IFINDEX_INTERNAL)
			   zlog_info("creat ve(rpa) interface failed\n");
		#endif
  			
  		}
	  else if(strncmp(product->board_name,AX81_AC8C,sizeof(AX81_AC8C)) == 0)/*8c*/
  		{
  		  if(slot < 10)
  		  {
		    sprintf(name, "ve0%df1", slot);/*01, 02, ..., 09*/
  		  	}
		  else
		   {
			 sprintf(name, "ve%df1", slot);/*10, 11, ...*/
		  	}
  		   zlog_info("%s: 81_AC8C system init to register [%s] rpa table.\n",__func__,name);
		   register_rpa_interface_table_byname(name);
  			
  		}
	  else if(strncmp(product->board_name,AX81_AC12C,sizeof(AX81_AC12C)) == 0)/*12c*/
  		{
  		  /*first cpu*/
  		  if(slot < 10)
  		  {
		    sprintf(name, "ve0%df1", slot);/*01, 02, ..., 09*/
  		  	}
		  else
		   {
			 sprintf(name, "ve%df1", slot);/*10, 11, ...*/
		  	}
  		   zlog_info("%s: 81_AC12C system init to register [%s] rpa table.\n",__func__,name);			   
		   register_rpa_interface_table_byname(name);
		   
		   /*second cpu*/
		   if(slot < 10)
  		  {
		    sprintf(name, "ve0%ds1", slot);/*01, 02, ..., 09*/
  		  	}
		  else
		   {
			 sprintf(name, "ve%ds1", slot);/*10, 11, ...*/
		  	}
  		   zlog_info("%s: 81_AC12C system init to register [%s] rpa table.\n",__func__,name);			   
		   register_rpa_interface_table_byname(name);
  			
  		}
	  else
	  if(strncmp(product->board_name,AX81_1X12G12S,sizeof(AX81_1X12G12S)) == 0)/*1X*/
  		{
  		  if(slot < 10)
  		  {
		    sprintf(name, "ve0%df1", slot);/*01, 02, ..., 09*/
  		  	}
		  else
		   {
			 sprintf(name, "ve%df1", slot);/*10, 11, ...*/
		  	}
  		   zlog_info("%s: 81_1X system init to register [%s] rpa table.\n",__func__,name);
		   register_rpa_interface_table_byname(name);
  			
  		}
	  else
	  	{
	  		zlog_info("othe board .\n");
	  	}
		/*will enlarge 4X board: have 4 cpu*/
		
	  
}

void
ve_interface_rpa_init()
{	
	struct interface *ifp = NULL;
	int ret;
	int id_mask = -1;
	int board_state[32] = {0};
	int i,slot = -1;
	char name[32];
	
	memset(name, 0, 32);	
	if(product == NULL)
		return;
	#if 0
	if(product->board_type == BOARD_IS_VICE)
		return;
	#endif
	
#if 0
	if((product->product_type == PRODUCT_TYPE_86)&&
		(strncmp(product->board_name,AX81_2X12G12S,sizeof(AX81_2X12G12S)) == 0))/*86xx: switch board don't go to creat ve rpa*/
	{
		zlog_debug(" The board name is %s , don't go to creat ve(rpa) .\n",product->board_name);
		return;
	}
#else
	if(((product->product_type == PRODUCT_TYPE_8603)
		||(product->product_type == PRODUCT_TYPE_8606)
		||(product->product_type == PRODUCT_TYPE_8610)
		||(product->product_type == PRODUCT_TYPE_8800))
		&&(strncmp(product->board_name,AX81_2X12G12S,sizeof(AX81_2X12G12S) == 0)))/*86xx: switch board don't go to creat ve rpa*/
	{
		zlog_debug(" The board name is %s , don't go to creat ve(rpa) .\n",product->board_name);
		return;
	}

#endif
	id_mask = product->alive_board_slotid;

	for(i = 0; i < 32 ; i++)
	 {
	 	if( (id_mask >> i) & 1)
	 	 {
	 	 	board_state[i] = BOARD_ON_PRODUCT;
	 	 }
		else
			board_state[i] = BOARD_OFF_PRODUCT;

	}

	for( i = 0; i < 32 ; i++ )
	  {
	  	 if(board_state[i] == BOARD_ON_PRODUCT)
	  	 {
	  	 	slot = i + 1;
	  	 	if(product->board_id != slot)/*not local , creat rpa*/
	  	 	{
	  	 	#if 0
	  	 		if(slot == 5 || slot == 6)/*86xx:  master board (include active and bakup) don't creat ve5,ve6 . so every board also don't creat ve5 and ve6*/
	  	 		{
	  	 			zlog_debug("when creat ve(rpa) we skip slot of ve%d .\n ",slot);
					continue;
	  	 		}
			#endif	
	  	 		sprintf(name, "ve%d", slot);
				ifp = if_get_by_name(name);
				if(ifp == NULL)
				{
					zlog_info("creat %s (viral) interface failed\n",name);
					return;
					}
			
				ifp = create_rpa_interface_and_get_ifindex(ifp);/**creat rpa and register**/
				/*
				if(ifp == NULL)
					zlog_info("creat ve(rpa) interface failed\n");
					*/
/*
				if(ifp->ifindex == IFINDEX_INTERNAL)
				   zlog_info("creat ve(rpa) interface failed\n");
	*/			
	  	 	}
			#if 1
			else/*local : registet rpa table*/
			{
			/*  if(product->product_type == PRODUCT_TYPE_76)*//*7605:obc0*/
			  if(product->product_type == PRODUCT_TYPE_7605I)/*7605:obc0*/
			  	{
					strncpy(name,"obc0",4);
					ifp = if_get_by_name(name);
					if(ifp == NULL)
					{
						zlog_info("register %s (viral) interface failed\n",name);
						return;
					}
				
					register_rpa_interface_table(ifp);				
			  	}
			  else										/*86XX: veX */
			  	{
				  if(strncmp(product->board_name,AX81_SMU,sizeof(AX81_SMU)) == 0)
			  		{
			  			zlog_debug("86: The board %s is master, skip yoursel to registe ve(rpa) table .\n",product->board_name);
			  			continue;
			  		}
						
			  		sprintf(name, "ve%d", slot);
					ifp = if_get_by_name(name);
					if(ifp == NULL)
					{
						zlog_info("creat %s (viral) interface failed\n",name);
						return;
					}
				
					register_rpa_interface_table(ifp);	
			  	}
			}
			#endif
			
	  	 }
	  }
		
}


/* Called when new interface is added. */
static int
if_zebra_new_hook (struct interface *ifp)
{
  struct zebra_if *zebra_if;

  zebra_if = XMALLOC (MTYPE_TMP, sizeof (struct zebra_if));
  memset (zebra_if, 0, sizeof (struct zebra_if));

  zebra_if->multicast = IF_ZEBRA_MULTICAST_UNSPEC;
  zebra_if->shutdown = IF_ZEBRA_SHUTDOWN_UNSPEC;

#ifdef RTADV
  {
    /* Set default router advertise values. */
    struct rtadvconf *rtadv;

    rtadv = &zebra_if->rtadv;

    rtadv->AdvSendAdvertisements = 0;
    rtadv->MaxRtrAdvInterval = RTADV_MAX_RTR_ADV_INTERVAL;
    rtadv->MinRtrAdvInterval = RTADV_MIN_RTR_ADV_INTERVAL;
    rtadv->AdvIntervalTimer = 0;
    rtadv->AdvManagedFlag = 0;
    rtadv->AdvOtherConfigFlag = 0;
    rtadv->AdvHomeAgentFlag = 0;
    rtadv->AdvLinkMTU = 0;
    rtadv->AdvReachableTime = 0;
    rtadv->AdvRetransTimer = 0;
    rtadv->AdvCurHopLimit = 0;
    rtadv->AdvDefaultLifetime = RTADV_ADV_DEFAULT_LIFETIME;
    rtadv->HomeAgentPreference = 0;
    rtadv->HomeAgentLifetime = RTADV_ADV_DEFAULT_LIFETIME;
    rtadv->AdvIntervalOption = 0;

    rtadv->AdvPrefixList = list_new ();
  }    
#endif /* RTADV */

  /* Initialize installed address chains tree. */
  zebra_if->ipv4_subnets = route_table_init ();

  ifp->info = zebra_if;
  return 0;
}

/* Called when interface is deleted. */
static int
if_zebra_delete_hook (struct interface *ifp)
{
  struct zebra_if *zebra_if;
  
  if (ifp->info)
    {
      zebra_if = ifp->info;

      /* Free installed address chains tree. */
      if (zebra_if->ipv4_subnets)
	route_table_finish (zebra_if->ipv4_subnets);

      XFREE (MTYPE_TMP, zebra_if);
    }

  return 0;
}

/* Tie an interface address to its derived subnet list of addresses. */
int
if_subnet_add (struct interface *ifp, struct connected *ifc)
{
  struct route_node *rn;
  struct zebra_if *zebra_if;
  struct prefix cp;
  struct list *addr_list;

#if 0
  assert (ifp && ifp->info && ifc);
#else
	if(!ifp || !ifc ||  !ifp->info)
	{
		zlog(NULL, LOG_CRIT, "line %u, function %s",
			 __LINE__,(__func__ ? __func__ : "?"));
		zlog_backtrace(LOG_CRIT);
		return 0;
	}

#endif

  
  zebra_if = ifp->info;

  /* Get address derived subnet node and associated address list, while marking
     address secondary attribute appropriately. */
  cp = *ifc->address;
  apply_mask (&cp);
  rn = route_node_get (zebra_if->ipv4_subnets, &cp);

  if ((addr_list = rn->info))
    SET_FLAG (ifc->flags, ZEBRA_IFA_SECONDARY);
  else
    {
      UNSET_FLAG (ifc->flags, ZEBRA_IFA_SECONDARY);
      rn->info = addr_list = list_new ();
      route_lock_node (rn);
    }

  /* Tie address at the tail of address list. */
  listnode_add (addr_list, ifc);
  
  /* Return list element count. */
  return (addr_list->count);
}

/* Untie an interface address from its derived subnet list of addresses. */
int
if_subnet_delete (struct interface *ifp, struct connected *ifc)
{
  struct route_node *rn;
  struct zebra_if *zebra_if;
  struct list *addr_list;

#if 0
	assert (ifp && ifp->info && ifc);
#else
	  if(!ifp || !ifc ||  !ifp->info)
	  {
		  zlog(NULL, LOG_CRIT, "line %u, function %s",
			   __LINE__,(__func__ ? __func__ : "?"));
		  zlog_backtrace(LOG_CRIT);
		  return 0;
	  }
  
#endif
  
  zebra_if = ifp->info;

  /* Get address derived subnet node. */
  rn = route_node_lookup (zebra_if->ipv4_subnets, ifc->address);
  if (! (rn && rn->info))
    return -1;
  route_unlock_node (rn);
  
  /* Untie address from subnet's address list. */
  addr_list = rn->info;
  listnode_delete (addr_list, ifc);
  route_unlock_node (rn);

  /* Return list element count, if not empty. */
   if (addr_list->count)
   {
	 if(product)/*distribute system*/
	 {
		 /* If deleted address is primary, mark subsequent one as such and distribute. */
		 if (! CHECK_FLAG (ifc->flags, ZEBRA_IFA_SECONDARY))
		 {
		   int ret = 0;
		   ifc = listgetdata (listhead (addr_list));

		   /*first delete it from secondary state*/
		   if(product->board_type == BOARD_IS_ACTIVE_MASTER)
		   	{
		   		zebra_interface_address_delete_update (ifp, ifc);/*active master*/
		   	}
		   else
		   	{
		   		vice_redistribute_interface_address_delete(ifp,ifc);/*vice and backup master*/
		   	}
		   
		   UNSET_FLAG (ifc->flags, ZEBRA_IFA_SECONDARY);
		   /*then install kernel*/
		   if(ifc->address->family == AF_INET)
			{
			 	ret = if_set_prefix (ifp, ifc);
			 	if(ret < 0)
			   	{
				   zlog_warn("%s : line %d, set ip(v4) failed(%s).\n",__func__,__LINE__,safe_strerror(errno));
			   	}
			 }
		 	else
			{
			 	ret = if_prefix_add_ipv6 (ifc->ifp, ifc);
			 	if(ret < 0)
			    {
				  zlog_warn("%s : line %d, set ip(v6) failed(%s).\n",__func__,__LINE__,safe_strerror(errno));
			    }
			  }

		   /*add as first state and redistribute other board or daemon.*/
		   if(product->board_type== BOARD_IS_ACTIVE_MASTER)
		   	{
		   		zebra_interface_address_add_update (ifp, ifc);
		   	}
		   else
		   	{
		   		vice_redistribute_interface_address_add(ifp,ifc);
		   	}
		   
		 }
		   
		   return addr_list->count;
	 }
	 else
	{
		 /* If deleted address is primary, mark subsequent one as such and distribute. */
		 if (! CHECK_FLAG (ifc->flags, ZEBRA_IFA_SECONDARY))
		 {
		   ifc = listgetdata (listhead (addr_list));
		   zebra_interface_address_delete_update (ifp, ifc);
		   UNSET_FLAG (ifc->flags, ZEBRA_IFA_SECONDARY);
		   zebra_interface_address_add_update (ifp, ifc);
		 }
		   
		   return addr_list->count;
	  }
   	}
  
  /* Otherwise, free list and route node. */
  list_free (addr_list);
  rn->info = NULL;
  route_unlock_node (rn);

  return 0;
}

/* if_flags_mangle: A place for hacks that require mangling
 * or tweaking the interface flags.
 *
 * ******************** Solaris flags hacks **************************
 *
 * Solaris IFF_UP flag reflects only the primary interface as the
 * routing socket only sends IFINFO for the primary interface.  Hence  
 * ~IFF_UP does not per se imply all the logical interfaces are also   
 * down - which we only know of as addresses. Instead we must determine
 * whether the interface really is up or not according to how many   
 * addresses are still attached. (Solaris always sends RTM_DELADDR if
 * an interface, logical or not, goes ~IFF_UP).
 *
 * Ie, we mangle IFF_UP to *additionally* reflect whether or not there
 * are addresses left in struct connected, not just the actual underlying
 * IFF_UP flag.
 *
 * We must hence remember the real state of IFF_UP, which we do in
 * struct zebra_if.primary_state.
 *
 * Setting IFF_UP within zebra to administratively shutdown the
 * interface will affect only the primary interface/address on Solaris.
 ************************End Solaris flags hacks ***********************
 */
static inline void
if_flags_mangle (struct interface *ifp, uint64_t *newflags)
{
#ifdef SUNOS_5
  struct zebra_if *zif = ifp->info;
  
  zif->primary_state = *newflags & (IFF_UP & 0xff);
  
  if (CHECK_FLAG (zif->primary_state, IFF_UP)
      || listcount(ifp->connected) > 0)
    SET_FLAG (*newflags, IFF_UP);
  else
    UNSET_FLAG (*newflags, IFF_UP);
#endif /* SUNOS_5 */
}


/* Update the flags field of the ifp with the new flag set provided.
 * Take whatever actions are required for any changes in flags we care
 * about.
 *
 * newflags should be the raw value, as obtained from the OS.
 */
void
if_flags_update_bak (struct interface *ifp, uint64_t newflags)
{
  if_flags_mangle (ifp, &newflags);
    
  if (if_is_operative (ifp))
    {
      /* operative -> inoperative? */
      ifp->flags = newflags;
      if (!if_is_operative (ifp))
        if_down_redistribute(ifp);
    }
  else
    {
      /* inoperative -> operative? */
      ifp->flags = newflags;
      if (if_is_operative (ifp))
        if_up_redistribute(ifp);
    }
}

/* Update the flags field of the ifp with the new flag set provided.
 * Take whatever actions are required for any changes in flags we care
 * about.
 *
 * newflags should be the raw value, as obtained from the OS.
 */
void
if_flags_update (struct interface *ifp, uint64_t newflags)
{
  if_flags_mangle (ifp, &newflags);
    
  if (if_is_operative (ifp))
    {
      /* operative -> inoperative? */
      ifp->flags = newflags;
      if (!if_is_operative (ifp))
        if_down (ifp);
    }
  else
    {
      /* inoperative -> operative? */
      ifp->flags = newflags;
      if (if_is_operative (ifp))
        if_up (ifp);
    }
}

/* Wake up configured address if it is not in current kernel
   address. */
static void
if_addr_wakeup (struct interface *ifp)
{
  struct listnode *node, *nnode;
  struct connected *ifc;
  struct prefix *p;
  int ret;

  for (ALL_LIST_ELEMENTS (ifp->connected, node, nnode, ifc))
    {
      p = ifc->address;
	
      if (CHECK_FLAG (ifc->conf, ZEBRA_IFC_CONFIGURED)
	  && ! CHECK_FLAG (ifc->conf, ZEBRA_IFC_REAL))
	{
	  /* Address check. */
	  if (p->family == AF_INET)
	    {
	      if (! if_is_up (ifp))
		{
		  /* XXX: WTF is it trying to set flags here?
		   * caller has just gotten a new interface, has been
                   * handed the flags already. This code has no business
                   * trying to override administrative status of the interface.
                   * The only call path to here which doesn't originate from
                   * kernel event is irdp - what on earth is it trying to do?
                   *
                   * further RUNNING is not a settable flag on any system
                   * I (paulj) am aware of.
                   */
		  if_set_flags (ifp, IFF_UP | IFF_RUNNING);
		  if_refresh (ifp);
		}

	      ret = if_set_prefix (ifp, ifc);
	      if (ret < 0)
		{
		  zlog_warn ("%s: line %d ,Can't set interface's address: %s", 
			     __func__,__LINE__,safe_strerror(errno));
		  continue;
		}

	      /* Add to subnet chain list. */
	      if_subnet_add (ifp, ifc);

	      SET_FLAG (ifc->conf, ZEBRA_IFC_REAL);

	      zebra_interface_address_add_update (ifp, ifc);

	      if (if_is_operative(ifp))
		connected_up_ipv4 (ifp, ifc);
	    }
#ifdef HAVE_IPV6
	  if (p->family == AF_INET6)
	    {
	      if (! if_is_up (ifp))
				{
				  /* XXX: See long comment above */
				  if_set_flags (ifp, IFF_UP | IFF_RUNNING);
				  if_refresh (ifp);
				}

	      ret = if_prefix_add_ipv6 (ifp, ifc);
	      if (ret < 0)
				{
				  zlog_warn ("%s : line %d, Can't set interface's address: %s", 
					    __func__,__LINE__, safe_strerror(errno));
				  continue;
				}
	      SET_FLAG (ifc->conf, ZEBRA_IFC_REAL);

	      zebra_interface_address_add_update (ifp, ifc);

	      if (if_is_operative(ifp))
		connected_up_ipv6 (ifp, ifc);
	    }
#endif /* HAVE_IPV6 */
	}
    }
}

/* Handle interface addition */
void
if_add_update (struct interface *ifp)
{
  struct zebra_if *if_data;

  if_data = ifp->info;
  if (if_data->multicast == IF_ZEBRA_MULTICAST_ON)
    if_set_flags (ifp, IFF_MULTICAST);
  else if (if_data->multicast == IF_ZEBRA_MULTICAST_OFF)
    if_unset_flags (ifp, IFF_MULTICAST);

  zebra_interface_add_update (ifp);

  if (! CHECK_FLAG (ifp->status, ZEBRA_INTERFACE_ACTIVE))
    {
      SET_FLAG (ifp->status, ZEBRA_INTERFACE_ACTIVE);

	 if((CHECK_FLAG(ifp->if_scope, INTERFACE_LOCAL))
	 	&&((judge_real_local_interface(ifp->name))== OTHER_BOARD_INTERFACE))
	 {
	 	 zlog_info("The interface(%s)is set local and not this board interface , so not to wakeup the ip address for kernel .\n",ifp->name);
		 return;
	 	}
      if_addr_wakeup (ifp);

      if (IS_ZEBRA_DEBUG_KERNEL)
	zlog_debug ("interface %s index %d becomes active.", 
		    ifp->name, ifp->ifindex);
    }
  else
    {
      if (IS_ZEBRA_DEBUG_KERNEL)
	zlog_debug ("interface %s index %d is added.", ifp->name, ifp->ifindex);
    }
}

/* Handle an interface delete event */
void 
if_delete_update (struct interface *ifp)
{
  struct listnode *node;
  struct listnode *next;
  struct listnode *first;
  struct listnode *last;
  struct connected *ifc;
  struct prefix *p;
  struct route_node *rn;
  struct zebra_if *zebra_if;
  struct list *addr_list;

  zebra_if = ifp->info;

  if (if_is_up(ifp))
    {
      zlog_err ("interface %s index %d is still up while being deleted.",
	    ifp->name, ifp->ifindex);
      return;
    }

  /* Mark interface as inactive */
  UNSET_FLAG (ifp->status, ZEBRA_INTERFACE_ACTIVE);
  
  if (IS_ZEBRA_DEBUG_KERNEL)
    zlog_debug ("interface %s index %d is now inactive.",
	       ifp->name, ifp->ifindex);

  /* Delete connected routes from the kernel. */
  if (ifp->connected)
    {
      last = NULL;
      while ((node = (last ? last->next : listhead (ifp->connected))))
	{
	  ifc = listgetdata (node);
	  p = ifc->address;

	  
	  if (p->family == AF_INET)
	    {
	    
		  if((rn = route_node_lookup (zebra_if->ipv4_subnets, p)))
		  {
				route_unlock_node (rn);
				addr_list = (struct list *) rn->info;
				
				/* Remove addresses, secondaries first. */
				first = listhead (addr_list);
				for (node = first->next; node || first; node = next)
			  {
			  
				if (! node)
				  {
					node = first;
					first = NULL;
				  }
				next = node->next;
	  
				ifc = listgetdata (node);
				p = ifc->address;
	  
				connected_down_ipv4 (ifp, ifc);
	  
				zebra_interface_address_delete_update (ifp, ifc);
	  
				UNSET_FLAG (ifc->conf, ZEBRA_IFC_REAL);
	  
				/* Remove from subnet chain. */
				list_delete_node (addr_list, node);
				route_unlock_node (rn);
				/* Remove from interface address list (unconditionally). */
				router_id_del_address(ifc);/*add*/
				listnode_delete (ifp->connected, ifc);
					connected_free (ifc);
			  }
	  
				/* Free chain list and respective route node. */
				list_delete (addr_list);
				rn->info = NULL;
				route_unlock_node (rn);

		  	}
		  	else
		  	{
		  	
				router_id_del_address(ifc);
				listnode_delete (ifp->connected, ifc);
					connected_free (ifc);
			}
		  
	    }
#ifdef HAVE_IPV6
	  else if (p->family == AF_INET6)
	    {
	      connected_down_ipv6 (ifp, ifc);

	      zebra_interface_address_delete_update (ifp, ifc);

	      UNSET_FLAG (ifc->conf, ZEBRA_IFC_REAL);

	      if (CHECK_FLAG (ifc->conf, ZEBRA_IFC_CONFIGURED))
		last = node;
	      else
			{
			
				router_id_del_address(ifc);
				listnode_delete (ifp->connected, ifc);
				connected_free (ifc);
			}
	    }
#endif /* HAVE_IPV6 */

	}
    }
  if((judge_eth_sub_interface(ifp->name)== ETH_SUB_INTERFACE) 
	||(judge_ve_sub_interface(ifp->name) == VE_SUB_INTERFACE)
	||(judge_eth_interface(ifp->name)==ETH_INTERFACE))/*ve sub interface and eth sub interface deal as normal*/
  	{
  		zebra_interface_delete_update (ifp);
		#if 0/*gujd : 2012-05-05, am 11:35 . Detele it for interface local (ve sub interface).*/
		if((judge_ve_sub_interface(ifp->name) == VE_SUB_INTERFACE)&&(CHECK_FLAG(ifp->if_scope,INTERFACE_LOCAL)))
		{
			int slot = 0;
			slot = get_slot_num(ifp->name);
			if(slot == product->board_id)
			{
			/*local interface not delete on active master (rtmd), remain the ifp,  but kernel delete and free the netdev .*/
				zlog_debug("%s : line %d , the interface %s is local .\n",__func__,__LINE__,ifp->name);
				ifp->if_types = VIRTUAL_INTERFACE;
				ifp->ifindex = IFINDEX_INTERNAL;
				return;
			 }
			else
			{
				if(product->board_type != BOARD_IS_ACTIVE_MASTER)
				{
					zlog_debug("%s : line %d , the interface %s is not local .\n",__func__,__LINE__,ifp->name);
					if_delete(ifp);
					return;
					}
				else/*active master remain struct ifp*/
				{
					zlog_debug("%s : line %d , active master remain the interface %s is not local .\n",__func__,__LINE__,ifp->name);
					ifp->if_types = VIRTUAL_INTERFACE;
					ifp->ifindex = IFINDEX_INTERNAL;
					return;
				}
			}
		}
		#endif
		if_delete(ifp);
		return;
  	}
	
  if(ifp->if_types == REAL_INTERFACE )
  {
  	/*zlog_debug("%s : line %d, if_type = %d \n ",__func__,__LINE__,ifp->if_types);*/
  	unregister_rpa_interface_table(ifp);
  	}
  if(product != NULL && product->board_type == BOARD_IS_ACTIVE_MASTER )
  {
  
  /*zlog_debug("%s : line %d, if_type = %d \n ",__func__,__LINE__,ifp->if_types);*/
  	zebra_interface_delete_update (ifp);
  	}
  else 
  	if(product != NULL && product->board_type != BOARD_IS_ACTIVE_MASTER &&ifp->if_types != VIRTUAL_INTERFACE  )
	 {
		/*zlog_debug("%s : line %d, if_type = %d \n ",__func__,__LINE__,ifp->if_types);*/
		/*vice_redistribute_interface_delete(ifp);*/
		zebra_interface_delete_update(ifp);
	 }
	else if(product == NULL)
	 {
		zebra_interface_delete_update (ifp);
	 }
#if 0
  /* Update ifindex after distributing the delete message.  This is in
     case any client needs to have the old value of ifindex available
     while processing the deletion.  Each client daemon is responsible
     for setting ifindex to IFINDEX_INTERNAL after processing the
     interface deletion message. */
  ifp->ifindex = IFINDEX_INTERNAL;
#else

	if(product != NULL )/**Distribute System**/
	{ 
		if(ifp->if_types != RPA_INTERFACE)/*not rpa*/
		{
			if_delete(ifp);
			return;
		}
		else/*rpa*/
		{
			/*vice board (not include bakup master) to delete rpa interface when set local*/
			if((ifp->if_types == RPA_INTERFACE)
				&&(CHECK_FLAG(ifp->if_scope,INTERFACE_LOCAL))
				&&(product->board_type == BOARD_IS_VICE))
			{
				if_delete(ifp);
				return;
			}
			
			ifp->if_types = VIRTUAL_INTERFACE;
			ifp->ifindex = IFINDEX_INTERNAL;
			return;
		}
	}
	else
		{
			if_delete(ifp);
		}
#endif
}


/* Interface is up. */
void
if_up_redistribute(struct interface *ifp)
{
  struct listnode *node;
  struct listnode *next;
  struct connected *ifc;
  struct prefix *p;
  int ret;
  struct timeval timer_now = {0};

  if (IS_ZEBRA_DEBUG_RIB)
    zlog_debug ("%s: start ", __func__);

  /* Notify the protocol daemons and vice(or master) board for Distribute System */
//  zebra_interface_up_update (ifp);

	ifp->if_up_cnt++;

  /* Install connected routes to the kernel. */
  if (ifp->connected)
    {
      for (ALL_LIST_ELEMENTS (ifp->connected, node, next, ifc))
	{
	  p = ifc->address;

	  if (p->family == AF_INET)
	    connected_up_ipv4 (ifp, ifc);
#ifdef HAVE_IPV6
	  else if (p->family == AF_INET6){
			if(!CHECK_FLAG (ifc->conf, ZEBRA_IFC_REAL)&&CHECK_FLAG (ifc->conf, ZEBRA_IFC_CONFIGURED)){
				ret = if_prefix_add_ipv6 (ifp, ifc);
				if (ret < 0)
				{
					zlog_warn ("%s : line %d, Can't set interface's address: %s", 
							 __func__,__LINE__,safe_strerror(errno));
					continue;
				}
			SET_FLAG (ifc->conf, ZEBRA_IFC_REAL);
			zebra_interface_address_add_update (ifp, ifc);
	
	}
	    connected_up_ipv6 (ifp, ifc);
			
		}
#endif /* HAVE_IPV6 */
	}
    }
  if (IS_ZEBRA_DEBUG_RIB)
    zlog_debug ("%s: goto rib_update ", __func__);

  gettimeofday(&timer_now,0);
  ifp->time = timer_now.tv_sec ;
  /* Examine all static routes. */
   rib_update ();
}

/* Interface is up. */
void
if_up (struct interface *ifp)
{
  struct listnode *node;
  struct listnode *next;
  struct connected *ifc;
  struct prefix *p;
  int ret;
  struct timeval timer_now = {0};

  if (IS_ZEBRA_DEBUG_RIB)
    zlog_debug ("%s: start ", __func__);

  /* Notify the protocol daemons. */
  zebra_interface_up_update (ifp);

	ifp->if_up_cnt++;


  /* Install connected routes to the kernel. */
  if (ifp->connected)
    {
      for (ALL_LIST_ELEMENTS (ifp->connected, node, next, ifc))
	{
	  p = ifc->address;

	  if (p->family == AF_INET)
	    connected_up_ipv4 (ifp, ifc);
#ifdef HAVE_IPV6
	  else if (p->family == AF_INET6){
			if(!CHECK_FLAG (ifc->conf, ZEBRA_IFC_REAL)&&CHECK_FLAG (ifc->conf, ZEBRA_IFC_CONFIGURED)){
				ret = if_prefix_add_ipv6 (ifp, ifc);
				if (ret < 0)
				{
					zlog_warn ("%s : line %d, Can't set interface's address: %s", 
							__func__,__LINE__, safe_strerror(errno));
					continue;
				}
			SET_FLAG (ifc->conf, ZEBRA_IFC_REAL);
			zebra_interface_address_add_update (ifp, ifc);
	
	}
	    connected_up_ipv6 (ifp, ifc);
			
		}
#endif /* HAVE_IPV6 */
	}
    }
  if (IS_ZEBRA_DEBUG_RIB)
    zlog_debug ("%s: goto rib_update ", __func__);

  gettimeofday(&timer_now,0);
  ifp->time = timer_now.tv_sec ;
  /* Examine all static routes. */
   rib_update ();
}

/* Interface goes down.  We have to manage different behavior of based
   OS. */
void
if_down (struct interface *ifp)
{
  struct listnode *node;
  struct listnode *next;
  struct connected *ifc;
  struct prefix *p;
  struct timeval timer_now = {0};

  if (IS_ZEBRA_DEBUG_RIB)
    zlog_debug ("%s: start ", __func__);

  /* Notify to the protocol daemons. */
  zebra_interface_down_update (ifp);

	ifp->if_down_cnt++;


  /* Delete connected routes from the kernel. */
  if (ifp->connected)
    {
      for (ALL_LIST_ELEMENTS (ifp->connected, node, next, ifc))
	{
	  p = ifc->address;

	  if (p->family == AF_INET)
	    connected_down_ipv4 (ifp, ifc);
#ifdef HAVE_IPV6
	  else if (p->family == AF_INET6)
	    connected_down_ipv6 (ifp, ifc);
#endif /* HAVE_IPV6 */
	}
    }
  gettimeofday(&timer_now,0);
  ifp->time = timer_now.tv_sec ;

  /* Examine all static routes which direct to the interface. */
  if (IS_ZEBRA_DEBUG_RIB)
    zlog_debug ("%s: goto rib_update ", __func__);
  rib_update ();
}

void
if_down_redistribute (struct interface *ifp)
{
  struct listnode *node;
  struct listnode *next;
  struct connected *ifc;
  struct prefix *p;
  struct timeval timer_now = {0};

  if (IS_ZEBRA_DEBUG_RIB)
    zlog_debug ("%s: start ", __func__);

 /* Notify the protocol daemons and vice(or master) board for Distribute System */
/*  zebra_interface_down_update (ifp);*//*业务板和备用主控同步完up，down，running等状态后不再送active主控避免板间死循环*/

	ifp->if_down_cnt++;


  /* Delete connected routes from the kernel. */
  if (ifp->connected)
    {
      for (ALL_LIST_ELEMENTS (ifp->connected, node, next, ifc))
	{
	  p = ifc->address;

	  if (p->family == AF_INET)
	    connected_down_ipv4 (ifp, ifc);
#ifdef HAVE_IPV6
	  else if (p->family == AF_INET6)
	    connected_down_ipv6 (ifp, ifc);
#endif /* HAVE_IPV6 */
	}
    }
  gettimeofday(&timer_now,0);
  ifp->time = timer_now.tv_sec ;

  /* Examine all static routes which direct to the interface. */
  if (IS_ZEBRA_DEBUG_RIB)
    zlog_debug ("%s: goto rib_update ", __func__);
  rib_update ();
}


void
if_refresh_bak (struct interface *ifp)
{
  if_get_flags_bak(ifp);
}



void
if_refresh (struct interface *ifp)
{
  if_get_flags (ifp);
}

/* Output prefix string to vty. */
static int
prefix_vty_out (struct vty *vty, struct prefix *p)
{
  char str[INET6_ADDRSTRLEN];

  inet_ntop (p->family, &p->u.prefix, str, sizeof (str));
  vty_out (vty, "%s", str);
  return strlen (str);
}

/* Dump if address information to vty. */
static void
connected_dump_vty (struct vty *vty, struct connected *connected)
{
  struct prefix *p;
  struct interface *ifp;
//  zlog_debug("++++++++++++++++++++1 %s line = %d",__func__,__LINE__);

  /* Set interface pointer. */
  ifp = connected->ifp;
//  zlog_debug("++++++++++++++++++++1 %s line = %d",__func__,__LINE__);
 

  /* Print interface address. */
  p = connected->address;
  if(p)/**gjd : changed for distribute system ,avoid for process dead**/
  {
  	vty_out (vty, "  %s ", prefix_family_str (p));
  
  	/*CID 11027 (#1 of 1): Dereference after null check (FORWARD_NULL)
	3. var_deref_model: Passing null pointer "p" to function "prefix_vty_out(struct vty *, struct prefix *)", which dereferences it.
	A bug , some time p is null . So add check p before.*/
  	prefix_vty_out (vty, p);/*move to if(p) {----} */
  	vty_out (vty, "/%d", p->prefixlen);
  }

  /* If there is destination address, print it. */
  p = connected->destination;
  if (p)
    {
      if (p->family == AF_INET)
	if (ifp->flags & IFF_BROADCAST)
	  {
	    vty_out (vty, " broadcast ");
	    prefix_vty_out (vty, p);
	  }

      if (ifp->flags & IFF_POINTOPOINT)
	{
	  vty_out (vty, " pointopoint ");
	  prefix_vty_out (vty, p);
	}
    }

  if (CHECK_FLAG (connected->flags, ZEBRA_IFA_SECONDARY))
    vty_out (vty, " secondary");

  if (connected->label)
    vty_out (vty, " %s", connected->label);

  vty_out (vty, "%s", VTY_NEWLINE);
}
#if (defined _D_WCPSS_ || defined _D_CC_)	

#define PARSE_RADIO_IFNAME_SUB '-'
#define PARSE_RADIO_IFNAME_POINT '.'

typedef enum{
	check_wtpid=0,
	check_sub,
	check_sub2,	
	check_sub3,
	check_radioid,
	check_wlanid,
	check_point,
	check_fail,
	check_end,
	check_success,
	check_vrrip,	
	check_slotid
}radio_ifname_state;
typedef enum{
	cnu_check_cnuid=0,
	cnu_check_cbatid,
	cnu_check_sub,
	cnu_check_ethid,
	cnu_check_point,
	cnu_check_fail,
	cnu_check_end,
	cnu_check_success
}cnu_ifname_state;

int parse_char_ID(char* str,unsigned char* ID){
	 /* before modify*/
	char *endptr = NULL;
	char c;
	c = str[0];
	if (c>='1'&&c<='9'){
		*ID= strtoul(str,&endptr,10);
		if(endptr[0] == '\0')
			return 1;
		else
			return 0;
	}
	else
		return 0;
	
}
int parse_wlan_ebr_ifname(char* ptr,int *vrrid,int *wlanid)
{
	
    radio_ifname_state state = check_vrrip;
	char *str = NULL;
	str = ptr;
   
	while(1){
		switch(state){
			
		case check_vrrip: 
			
			*vrrid = strtoul(str,&str,10);
			
			if(*vrrid >= 0 && *vrrid < 17){
        		state=check_sub;
			}
			else state=check_fail;
			
			break;

		case check_sub: 
		
			if (PARSE_RADIO_IFNAME_SUB == str[0]){
		
				state = check_wlanid;
				}
			else
				state = check_fail;
			break;

		case check_wlanid: 
		
			*wlanid = strtoul((char *)&(str[1]),&str,10);

			if(*wlanid > 0 && *wlanid < 129){/*radioid range 0 1 2 3*/
        		state=check_end;
			}
			else state=check_fail;
			
			break;		
		
		case check_fail:
	
		
            return -1;
			break;

		case check_end: 
	
			if ('\0' == str[0]) {
				state = check_success;
				
				}
			else
				state = check_fail;
				break;
			
		case check_success: 
		
			return 0;
			break;
			
		default: break;
		}
		
		}
		
}

int parse_wlan_ebr_ifname_v2(char* ptr,int *slotid,int *vrrid,int *wlanid)
{
	
    radio_ifname_state state = check_slotid;
	char *str = NULL;
	str = ptr;
   
	while(1){
		switch(state){
		case check_slotid: 
			
			*slotid = strtoul(str,&str,10);
			
			if(*slotid >= 0 && *slotid < 16){
				state=check_sub;
			}
			else state=check_fail;
			
			break;			
			
		case check_vrrip: 
			
			*vrrid = strtoul((char *)&(str[1]),&str,10);
			
			if(*vrrid >= 0 && *vrrid < 17){
        		state=check_sub2;
			}
			else state=check_fail;
			
			break;
		case check_sub: 
		
			if (PARSE_RADIO_IFNAME_SUB == str[0]){
		
				state = check_vrrip;
				}
			else
				state = check_fail;
			break;			

		case check_sub2: 
		
			if (PARSE_RADIO_IFNAME_SUB == str[0]){
		
				state = check_wlanid;
				}
			else
				state = check_fail;
			break;

		case check_wlanid: 
		
			*wlanid = strtoul((char *)&(str[1]),&str,10);

			if(*wlanid > 0 && *wlanid < 129){/*radioid range 0 1 2 3*/
        		state=check_end;
			}
			else state=check_fail;
			
			break;		
		
		case check_fail:
	
		
            return -1;
			break;

		case check_end: 
	
			if ('\0' == str[0]) {
				state = check_success;
				
				}
			else
				state = check_fail;
				break;
			
		case check_success: 
		
			return 0;
			break;
			
		default: break;
		}
		
		}
		
}


/*
  *****************************************************************************
  *  
  * NOTES:	 
  * INPUT:	   
  * OUTPUT:	  
  * return:	  
  *  
  * author: 		Huang Leilei 
  * begin time:	2012-11-19 14:00  
  * finish time:		2012-11-19 1549
  * history:	
  * 
  **************************************************************************** 
  */
int parse_wlan_ebr_ifname_v2_H(char* ptr,int *slotid,int *vrrid,int *ebrid)
{
	
    radio_ifname_state state = check_slotid;
	char *str = NULL;
	str = ptr;
   
	while(1){
		switch(state){
		case check_slotid: 
			
			*slotid = strtoul(str,&str,10);
			
			if(*slotid >= 0 && *slotid < 16){
				state=check_sub;
			}
			else state=check_fail;
			
			break;			
			
		case check_vrrip: 
			
			*vrrid = strtoul((char *)&(str[1]),&str,10);
			
			if(*vrrid >= 0 && *vrrid < 17){
        		state=check_sub2;
			}
			else state=check_fail;
			
			break;
		case check_sub: 
		
			if (PARSE_RADIO_IFNAME_SUB == str[0]){
		
				state = check_vrrip;
				}
			else
				state = check_fail;
			break;			

		case check_sub2: 
		
			if (PARSE_RADIO_IFNAME_SUB == str[0]){
		
				state = check_wlanid;
				}
			else
				state = check_fail;
			break;

		case check_wlanid: 
		
			*ebrid = strtoul((char *)&(str[1]),&str,10);

			if(*ebrid > 0 && *ebrid < 1024){/*radioid range 0 1 2 3*/
        		state=check_end;
			}
			else state=check_fail;
			
			break;		
		
		case check_fail:
	
		
            return -1;
			break;

		case check_end: 
	
			if ('\0' == str[0]) {
				state = check_success;
				
				}
			else
				state = check_fail;
				break;
			
		case check_success: 
		
			return 0;
			break;
			
		default: break;
		}
		
		}
		
}

int parse_radio_ifname(char* ptr,int *wtpid,int *radioid,int *wlanid)
{
	
    radio_ifname_state state = check_wtpid;
	char *str = NULL;
	str = ptr;
   
	while(1){
		switch(state){
			
		case check_wtpid: 
			
			*wtpid = strtoul(str,&str,10);
			
			if(*wtpid > 0 && *wtpid <= 4096){
        		state=check_sub;
			}
			else state=check_fail;
			
			break;

		case check_sub: 
		
			if (PARSE_RADIO_IFNAME_SUB == str[0]){
		
				state = check_radioid;
				}
			else
				state = check_fail;
			break;

		case check_radioid: 
		
			*radioid = strtoul((char *)&(str[1]),&str,10);

			if(*radioid >= 0 && *radioid < 4){/*radioid range 0 1 2 3*/
        		state=check_point;
			}
			else state=check_fail;
			
			break;

		case check_point: 
		
			if (PARSE_RADIO_IFNAME_POINT == str[0]){
			
				state = check_wlanid;
				
				}
			else
				state = check_fail;
			break;
				
		case check_wlanid: 
		
			*wlanid = strtoul((char *)&(str[1]),&str,10);

			if(*wlanid > 0 && *wlanid < 129){
        		state=check_end;
			}
			else state=check_fail;
			
			break;
			
		
		
		case check_fail:
	
		
            return -1;
			break;

		case check_end: 
	
			if ('\0' == str[0]) {
				state = check_success;
				
				}
			else
				state = check_fail;
				break;
			
		case check_success: 
		
			return 0;
			break;
			
		default: break;
		}
		
		}
		
}

int parse_radio_ifname_v2(char* ptr,int *instid,int *wtpid,int *radioid,int *wlanid)
{
	
    radio_ifname_state state = check_vrrip;
	char *str = NULL;
	str = ptr;
   
	while(1){
		switch(state){
		case check_vrrip: 
			
			*instid = strtoul(str,&str,10);
			
			if(*instid > 0 && *instid < 17){
				state=check_sub;
			}
			else state=check_fail;
			
			break;
			
		case check_wtpid: 
			
			*wtpid = strtoul((char *)&(str[1]),&str,10);
			
			if(*wtpid > 0 && *wtpid <= 4096){
        		state=check_sub2;
			}
			else state=check_fail;
			
			break;
		case check_sub: 
		
			if (PARSE_RADIO_IFNAME_SUB == str[0]){
		
				state = check_wtpid;
				}
			else
				state = check_fail;
			break;

		case check_sub2: 
		
			if (PARSE_RADIO_IFNAME_SUB == str[0]){
		
				state = check_radioid;
				}
			else
				state = check_fail;
			break;

		case check_radioid: 
		
			*radioid = strtoul((char *)&(str[1]),&str,10);

			if(*radioid >= 0 && *radioid < 4){/*radioid range 0 1 2 3*/
        		state=check_point;
			}
			else state=check_fail;
			
			break;

		case check_point: 
		
			if (PARSE_RADIO_IFNAME_POINT == str[0]){
			
				state = check_wlanid;
				
				}
			else
				state = check_fail;
			break;
				
		case check_wlanid: 
		
			*wlanid = strtoul((char *)&(str[1]),&str,10);

			if(*wlanid > 0 && *wlanid < 129){
        		state=check_end;
			}
			else state=check_fail;
			
			break;
			
		
		
		case check_fail:
	
		
            return -1;
			break;

		case check_end: 
	
			if ('\0' == str[0]) {
				state = check_success;
				
				}
			else
				state = check_fail;
				break;
			
		case check_success: 
		
			return 0;
			break;
			
		default: break;
		}
		
		}
		
}
int parse_radio_ifname_v3(char* ptr,int *slotid,int *instid,int *wtpid,int *radioid,int *wlanid)
{
	
	radio_ifname_state state = check_slotid;
	char *str = NULL;
	str = ptr;
   
	while(1){
		switch(state){
		case check_slotid: 
			
			*slotid = strtoul(str,&str,10);
			
			if(*slotid > 0 && *slotid < 16){
				state=check_sub;
			}
			else state=check_fail;
			
			break;

		case check_vrrip: 
			
			*instid = strtoul((char *)&(str[1]),&str,10);
			
			if(*instid > 0 && *instid < 17){
				state=check_sub2;
			}
			else state=check_fail;
			
			break;
			
		case check_wtpid: 
			
			*wtpid = strtoul((char *)&(str[1]),&str,10);
			
			if(*wtpid > 0 && *wtpid <= 4096){
				state=check_sub3;
			}
			else state=check_fail;
			
			break;
		case check_sub: 
		
			if (PARSE_RADIO_IFNAME_SUB == str[0]){
		
				state = check_vrrip;
				}
			else
				state = check_fail;
			break;

		case check_sub2: 
		
			if (PARSE_RADIO_IFNAME_SUB == str[0]){
		
				state = check_wtpid;
				}
			else
				state = check_fail;
			break;

		case check_sub3: 
		
			if (PARSE_RADIO_IFNAME_SUB == str[0]){
		
				state = check_radioid;
				}
			else
				state = check_fail;
			break;

		case check_radioid: 
		
			*radioid = strtoul((char *)&(str[1]),&str,10);

			if(*radioid >= 0 && *radioid < 4){/*radioid range 0 1 2 3*/
				state=check_point;
			}
			else state=check_fail;
			
			break;

		case check_point: 
		
			if (PARSE_RADIO_IFNAME_POINT == str[0]){
			
				state = check_wlanid;
				
				}
			else
				state = check_fail;
			break;
				
		case check_wlanid: 
		
			*wlanid = strtoul((char *)&(str[1]),&str,10);

			if(*wlanid > 0 && *wlanid < 129){
				state=check_end;
			}
			else state=check_fail;
			
			break;
			
		
		
		case check_fail:
	
		
			return -1;
			break;

		case check_end: 
	
			if ('\0' == str[0]) {
				state = check_success;
				
				}
			else
				state = check_fail;
				break;
			
		case check_success: 
		
			return 0;
			break;
			
		default: break;
		}
		
		}
		
}



/*wuwl add 20100813 for dcli_intf.c*/
int parse_cnu_ifname(char* ptr,int *cbatid,int *cnuid,int *ethid)
{
	
    cnu_ifname_state state = cnu_check_cbatid;
	char *str = NULL;
	str = ptr;
   
	while(1){
		switch(state){

			case cnu_check_cbatid: 
				*cbatid = strtoul(str,&str,10);
				if(*cbatid > 0 && *cbatid <= 4096){
					//printf("1\n");
	        		state=cnu_check_sub;
				}
				else state=cnu_check_fail;
				//printf("2\n");
				break;
			case cnu_check_sub: 
				if (PARSE_RADIO_IFNAME_SUB == str[0]){
					//printf("3\n");
						state = cnu_check_cnuid;
					}
				else
					state = cnu_check_fail;
				//printf("4\n");
				break;
			case cnu_check_cnuid: 
				*cnuid = strtoul((char *)&(str[1]),&str,10);
				if(*cnuid > 0 && *cnuid <= 64){
	        		state=cnu_check_point;
				}
				else state=cnu_check_fail;
				
				break;
			case cnu_check_point: 
				if (PARSE_RADIO_IFNAME_POINT == str[0]){
						state = cnu_check_ethid;
					}
				else
					state = cnu_check_fail;
				break;
					
			case cnu_check_ethid: 
				*ethid = strtoul((char *)&(str[1]),&str,10);
				if(*ethid >= 0 && *ethid < 4 +1){
	        		state=cnu_check_end;
				}
				else state=cnu_check_fail;
				break;
			case cnu_check_fail:
	            return -1;
				break;
			case cnu_check_end: 
				if ('\0' == str[0]) {
					state = cnu_check_success;
					}
				else
					state = cnu_check_fail;
					break;
			case cnu_check_success: 
				return 0;
				break;
			default: break;
		}
		
	}
		
}

#endif

#ifdef RTADV
/* Dump interface ND information to vty. */
static void
nd_dump_vty (struct vty *vty, struct interface *ifp)
{
  struct zebra_if *zif;
  struct rtadvconf *rtadv;
  int interval;

  zif = (struct zebra_if *) ifp->info;
  rtadv = &zif->rtadv;

  if (rtadv->AdvSendAdvertisements)
    {
      vty_out (vty, "  ND advertised reachable time is %d milliseconds%s",
	       rtadv->AdvReachableTime, VTY_NEWLINE);
      vty_out (vty, "  ND advertised retransmit interval is %d milliseconds%s",
	       rtadv->AdvRetransTimer, VTY_NEWLINE);
      interval = rtadv->MaxRtrAdvInterval;
      if (interval % 1000)
        vty_out (vty, "  ND router advertisements are sent every "
			"%d milliseconds%s", interval,
		 VTY_NEWLINE);
      else
        vty_out (vty, "  ND router advertisements are sent every "
			"%d seconds%s", interval / 1000,
		 VTY_NEWLINE);
      vty_out (vty, "  ND router advertisements live for %d seconds%s",
	       rtadv->AdvDefaultLifetime, VTY_NEWLINE);
      if (rtadv->AdvManagedFlag)
	vty_out (vty, "  Hosts use DHCP to obtain routable addresses.%s",
		 VTY_NEWLINE);
      else
	vty_out (vty, "  Hosts use stateless autoconfig for addresses.%s",
		 VTY_NEWLINE);
      if (rtadv->AdvHomeAgentFlag)
      	vty_out (vty, "  ND router advertisements with "
				"Home Agent flag bit set.%s",
		 VTY_NEWLINE);
      if (rtadv->AdvIntervalOption)
      	vty_out (vty, "  ND router advertisements with Adv. Interval option.%s",
		 VTY_NEWLINE);
    }
}
#endif /* RTADV */

/*gjd : add for deal with ip address fist setction is '0' , like  0.x.x.x/x illegal address .*/
int
check_ip_first_char(char *address_char)
{
	char address[64];
	
	memset(address, 0 ,64);
	strncpy(address, address_char ,strlen(address_char));

	if(address[0] == '0')
	 return -1 ;
	else
	 return 0 ;
	
}


/* Interface's information print out to vty interface. */
static void
if_dump_vty (struct vty *vty, struct interface *ifp)
{
#ifdef HAVE_SOCKADDR_DL
  struct sockaddr_dl *sdl;
#endif /* HAVE_SOCKADDR_DL */
  struct connected *connected;
  struct listnode *node;
  struct route_node *rn;
  struct zebra_if *zebra_if;


  
  if((memcmp(ifp->name,"sit0",4) == 0)
	||(memcmp(ifp->name,"pimreg",6) == 0)
	||((memcmp(ifp->name,"r",1) == 0)&&(radio_interface_show_enable == 0)))
   {
	return;
  }

  zebra_if = ifp->info;

  vty_out (vty, "Interface %s is ", ifp->name);
  if (if_is_up(ifp)) {
    vty_out (vty, "up, line protocol ");
    
    if (CHECK_FLAG(ifp->status, ZEBRA_INTERFACE_LINKDETECTION)) {
      if (if_is_running(ifp))
       vty_out (vty, "is up%s", VTY_NEWLINE);
      else
	vty_out (vty, "is down%s", VTY_NEWLINE);
    } else {
      vty_out (vty, "detection is disabled%s", VTY_NEWLINE);
    }
  } else {
    vty_out (vty, "down%s", VTY_NEWLINE);
  }

  if (ifp->desc)
  	{
      vty_out (vty, "  Description: %s%s", ifp->desc,VTY_NEWLINE);
	  if(CHECK_FLAG(ifp->desc_scope,INTERFACE_DESCIPTION_LOCAL))
		vty_out (vty, "  Description Mode: local %s", VTY_NEWLINE);
  	}
  #if 0 /**gjd : changed for distribute system ,used for virtual interface**/
  if (ifp->ifindex == IFINDEX_INTERNAL)
    {
      vty_out(vty, "  pseudo interface%s", VTY_NEWLINE);
      return;
    }
  #endif
//  else 
  	if (! CHECK_FLAG (ifp->status, ZEBRA_INTERFACE_ACTIVE))
    {
      vty_out(vty, "  index %d inactive interface%s", 
	      ifp->ifindex, 
	      VTY_NEWLINE);
//      return;  /**gjd : changed for distribute system ,used for virtual interface**/
    }

  vty_out (vty, "  index %d metric %d mtu %d  ",
	   ifp->ifindex, ifp->metric, ifp->mtu);
#ifdef HAVE_IPV6
  if (ifp->mtu6 != ifp->mtu)
    vty_out (vty, "mtu6 %d  ", ifp->mtu6);
#endif 
 /*gujd : 2013-01-15, pm 5:40. Add for interface local show*/
  if(CHECK_FLAG(ifp->if_scope, INTERFACE_LOCAL))
	  vty_out (vty, "mode: local ");

  vty_out (vty, "%s  flags: %s%s", VTY_NEWLINE,
           if_flag_dump (ifp->flags), VTY_NEWLINE);
  
  /* Hardware address. */
#ifdef HAVE_SOCKADDR_DL
  sdl = &ifp->sdl;
  if (sdl != NULL && sdl->sdl_alen != 0)
    {
      int i;
      u_char *ptr;

      vty_out (vty, "  HWaddr: ");
      for (i = 0, ptr = (u_char *)LLADDR (sdl); i < sdl->sdl_alen; i++, ptr++)
        vty_out (vty, "%s%02x", i == 0 ? "" : ":", *ptr);
      vty_out (vty, "%s", VTY_NEWLINE);
    }
#else
  if (ifp->hw_addr_len != 0)
    {
      int i;

      vty_out (vty, "  HWaddr: ");
      for (i = 0; i < ifp->hw_addr_len; i++)
	vty_out (vty, "%s%02x", i == 0 ? "" : ":", ifp->hw_addr[i]);
      vty_out (vty, "%s", VTY_NEWLINE);
    }
#endif /* HAVE_SOCKADDR_DL */
  
  /* Bandwidth in kbps */
  if (ifp->bandwidth != 0)
    {
      vty_out(vty, "  bandwidth %u kbps", ifp->bandwidth);
      vty_out(vty, "%s", VTY_NEWLINE);
    }

  for (rn = route_top (zebra_if->ipv4_subnets); rn; rn = route_next (rn))
    {
      if (! rn->info)
	continue;
      
      for (ALL_LIST_ELEMENTS_RO ((struct list *)rn->info, node, connected))
        connected_dump_vty (vty, connected);
    }

  for (ALL_LIST_ELEMENTS_RO (ifp->connected, node, connected))
    {
/*change by gjd*/

	  if ((CHECK_FLAG (connected->conf, ZEBRA_IFC_REAL)||CHECK_FLAG (connected->conf, ZEBRA_IFC_CONFIGURED ))&&
	  (connected->address->family == AF_INET6))
	connected_dump_vty (vty, connected);
    }

#ifdef RTADV
  nd_dump_vty (vty, ifp);
#endif /* RTADV */

#ifdef HAVE_PROC_NET_DEV
  /* Statistics print out using proc file system. */
  vty_out (vty, "    %llu input packets (%llu multicast), %llu bytes, "
	   "%llu dropped%s",
	   ifp->stats.rx_packets, ifp->stats.rx_multicast,
	   ifp->stats.rx_bytes, ifp->stats.rx_dropped, VTY_NEWLINE);

  vty_out (vty, "    %llu input errors, %llu length, %llu overrun,"
	   " %llu CRC, %llu frame%s",
	   ifp->stats.rx_errors, ifp->stats.rx_length_errors,
	   ifp->stats.rx_over_errors, ifp->stats.rx_crc_errors,
	   ifp->stats.rx_frame_errors, VTY_NEWLINE);

  vty_out (vty, "    %llu fifo, %llu missed%s", ifp->stats.rx_fifo_errors,
	   ifp->stats.rx_missed_errors, VTY_NEWLINE);

  vty_out (vty, "    %llu output packets, %llu bytes, %llu dropped%s",
	   ifp->stats.tx_packets, ifp->stats.tx_bytes,
	   ifp->stats.tx_dropped, VTY_NEWLINE);

  vty_out (vty, "    %llu output errors, %llu aborted, %llu carrier,"
	   " %llu fifo, %llu heartbeat%s",
	   ifp->stats.tx_errors, ifp->stats.tx_aborted_errors,
	   ifp->stats.tx_carrier_errors, ifp->stats.tx_fifo_errors,
	   ifp->stats.tx_heartbeat_errors, VTY_NEWLINE);

  vty_out (vty, "    %llu window, %llu collisions%s",
	   ifp->stats.tx_window_errors, ifp->stats.collisions, VTY_NEWLINE);
#endif /* HAVE_PROC_NET_DEV */

#ifdef HAVE_NET_RT_IFLIST
#if defined (__bsdi__) || defined (__NetBSD__)
  /* Statistics print out using sysctl (). */
  vty_out (vty, "    input packets %qu, bytes %qu, dropped %qu,"
	   " multicast packets %qu%s",
	   ifp->stats.ifi_ipackets, ifp->stats.ifi_ibytes,
	   ifp->stats.ifi_iqdrops, ifp->stats.ifi_imcasts,
	   VTY_NEWLINE);

  vty_out (vty, "    input errors %qu%s",
	   ifp->stats.ifi_ierrors, VTY_NEWLINE);

  vty_out (vty, "    output packets %qu, bytes %qu, multicast packets %qu%s",
	   ifp->stats.ifi_opackets, ifp->stats.ifi_obytes,
	   ifp->stats.ifi_omcasts, VTY_NEWLINE);

  vty_out (vty, "    output errors %qu%s",
	   ifp->stats.ifi_oerrors, VTY_NEWLINE);

  vty_out (vty, "    collisions %qu%s",
	   ifp->stats.ifi_collisions, VTY_NEWLINE);
#else
  /* Statistics print out using sysctl (). */
  vty_out (vty, "    input packets %lu, bytes %lu, dropped %lu,"
	   " multicast packets %lu%s",
	   ifp->stats.ifi_ipackets, ifp->stats.ifi_ibytes,
	   ifp->stats.ifi_iqdrops, ifp->stats.ifi_imcasts,
	   VTY_NEWLINE);

  vty_out (vty, "    input errors %lu%s",
	   ifp->stats.ifi_ierrors, VTY_NEWLINE);

  vty_out (vty, "    output packets %lu, bytes %lu, multicast packets %lu%s",
	   ifp->stats.ifi_opackets, ifp->stats.ifi_obytes,
	   ifp->stats.ifi_omcasts, VTY_NEWLINE);

  vty_out (vty, "    output errors %lu%s",
	   ifp->stats.ifi_oerrors, VTY_NEWLINE);

  vty_out (vty, "    collisions %lu%s",
	   ifp->stats.ifi_collisions, VTY_NEWLINE);
#endif /* __bsdi__ || __NetBSD__ */
#endif /* HAVE_NET_RT_IFLIST */
}

/* Check supported address family. */
static int
if_supported_family (int family)
{
  if (family == AF_INET)
    return 1;
#ifdef HAVE_IPV6
  if (family == AF_INET6)
    return 1;
#endif /* HAVE_IPV6 */
  return 0;
}

/* Wrapper hook point for zebra daemon so that ifindex can be set 
 * DEFUN macro not used as extract.pl HAS to ignore this
 * See also interface_cmd in lib/if.c
 */ 
DEFUN_NOSH (zebra_interface,
	    zebra_interface_cmd,
	    "interface IFNAME",
	    "Select an interface to configure\n"
	    "Interface's name\n")
{
  int ret;
  struct interface * ifp;
  
  /* Call lib interface() */
  if ((ret = interface_cmd.func (self, vty, argc, argv)) != CMD_SUCCESS)
    return ret;

#if 0
		ifp = (struct interface *) vty->index;
#else
		ifp = if_get_by_vty_index(vty->index);
		if(NULL == ifp)
			{
			vty_out (vty, "%% Interface %s does not exist%s",(char*)(vty->index), VTY_NEWLINE);
			return CMD_WARNING;
		}
#endif
  if (ifp->ifindex == IFINDEX_INTERNAL)
    /* Is this really necessary?  Shouldn't status be initialized to 0
       in that case? */
    UNSET_FLAG (ifp->status, ZEBRA_INTERFACE_ACTIVE);

  return ret;
}

struct cmd_node interface_node =
{
  INTERFACE_NODE,
  "%s(config-if)# ",
  1
};

/*2013-06-04 , am 5:00. Add for set uplink interface mark.*/

DEFUN (interface_set_uplink_flag,
       interface_set_uplink_flag_cmd,
       "set uplink",
       "Mark the selected interface as uplink interface.\n")
{
  struct interface *ifp;
  
#if 0
	  ifp = (struct interface *) vty->index;
#else
	  ifp = if_get_by_vty_index(vty->index);
	  if(NULL == ifp)
		{
		  vty_out (vty, "%% Interface %s does not exist%s", (char*)(vty->index), VTY_NEWLINE);
		  return CMD_WARNING;
	  }
	  
#endif
  SET_FLAG(ifp->uplink_flag, INTERFACE_SET_UPLINK);

  /*gjd : add for Distribute System, active master send turn on linkdetect to oter boards (keep sync ifp->status).*/
  if(product != NULL && product->board_type == BOARD_IS_ACTIVE_MASTER)
  {
	zebra_interface_uplink_state(ifp,1);
  }

  return CMD_SUCCESS;
}

DEFUN (interface_unset_uplink_flag,
       interface_unset_uplink_flag_cmd,
       "unset uplink",
       "Mark the selected interface as uplink interface.\n")
{
  struct interface *ifp;
  
#if 0
	  ifp = (struct interface *) vty->index;
#else
	  ifp = if_get_by_vty_index(vty->index);
	  if(NULL == ifp)
		{
		  vty_out (vty, "%% Interface %s does not exist%s", (char*)(vty->index), VTY_NEWLINE);
		  return CMD_WARNING;
	  }
	  
#endif
  UNSET_FLAG(ifp->uplink_flag, INTERFACE_SET_UPLINK);
  ifp->uplink_flag = 0;

  /*gjd : add for Distribute System, active master send turn on linkdetect to oter boards (keep sync ifp->status).*/
  if(product != NULL && product->board_type == BOARD_IS_ACTIVE_MASTER)
  {
	zebra_interface_uplink_state(ifp,0);
  }

  return CMD_SUCCESS;
}


DEFUN (interface_desc_func, 
       interface_desc_cmd_rtmd,
       "description .LINE",
       "Interface specific description\n"
       "Characters describing this interface\n")
{
  struct interface *ifp;

  if (argc == 0)
    return CMD_SUCCESS;
#if 0
  ifp = vty->index;
#else
  ifp = if_get_by_vty_index(vty->index);
  if(NULL == ifp)
  	{
	  vty_out (vty, "%% Interface %s does not exist%s", argv[0], VTY_NEWLINE);
	  return CMD_WARNING;
  }
  
  UNSET_FLAG(ifp->desc_scope,INTERFACE_DESCIPTION_LOCAL);
#endif
  if (ifp->desc)
    XFREE (MTYPE_TMP, ifp->desc);
  ifp->desc = argv_concat(argv, argc, 0);
  
  /*gujd: 2013-01-07, pm 4:05. Add for interface description sync.*/
  zebra_interface_description_update(ZEBRA_INTERFACE_DESCRIPTION_SET,ifp);

  return CMD_SUCCESS;
}

DEFUN (interface_desc_func2, 
       interface_desc_local_cmd_rtmd,
	   "local description .LINE",
	   "The description scope in local board\n"
	   "Characters describing this interface\n"
       "Interface specific description\n")
{
  struct interface *ifp;

  if (argc < 1)
    return CMD_SUCCESS;
#if 0
  ifp = vty->index;
#else
  ifp = if_get_by_vty_index(vty->index);
  if(NULL == ifp)
  	{
	  vty_out (vty, "%% Interface %s does not exist%s", argv[0], VTY_NEWLINE);
	  return CMD_WARNING;
  }
  
#endif
  if (ifp->desc)
    XFREE (MTYPE_TMP, ifp->desc);
  
  SET_FLAG(ifp->desc_scope,INTERFACE_DESCIPTION_LOCAL);
  ifp->desc = argv_concat(argv, argc, 0);

  /*gujd: 2013-01-07, pm 4:05. Add for interface description sync.*/
  zebra_interface_description_update(ZEBRA_INTERFACE_DESCRIPTION_SET,ifp);

  return CMD_SUCCESS;
}

DEFUN (no_interface_desc_func, 
       no_interface_desc_cmd_rtmd,
       "no description",
       NO_STR
       "Interface specific description\n")
{
  struct interface *ifp;

#if 0
  ifp = vty->index;
#else
  ifp = if_get_by_vty_index(vty->index);
  if(NULL == ifp)
  	{
	  vty_out (vty, "%% Interface %s does not exist%s", argv[0], VTY_NEWLINE);
	  return CMD_WARNING;
  }
  
#endif
  if (ifp->desc)
    XFREE (MTYPE_TMP, ifp->desc);
  ifp->desc = NULL;
  
  UNSET_FLAG(ifp->desc_scope,INTERFACE_DESCIPTION_LOCAL);
  /*gujd: 2013-01-07, pm 4:05. Add for interface description sync.*/
  zebra_interface_description_update(ZEBRA_INTERFACE_DESCRIPTION_UNSET,ifp);

  return CMD_SUCCESS;
}


/*gjd : add for set time interval for interface packets statistics */
DEFUN(interface_packets_statistics_sync_func,
	set_interface_packets_statistics_sync_time_interval_cmd,
	"time interval <10-3600>",
	"set interface packets statistics sync time interval.\n" )
{
	unsigned int time_set = 0;
	
	if(!product)
	 {
		zlog_warn("The product memory is NULL .\n");
		return CMD_WARNING;
		}
	if(product->board_type != BOARD_IS_ACTIVE_MASTER)
	 {
	 	vty_out(vty,"Only active master board can set time interval .\n");
		return CMD_WARNING;
		}
	
	time_set = atoi(argv[0]);
	if(time_set < 10 || time_set > 3600)
	{
		vty_out(vty,"The time interval is between 10s and 3600s .\n");
		return CMD_WARNING;
	}
	
	time_interval = time_set;
	
	return CMD_SUCCESS;
	
}
/*2011-11-2 : pm 5:00*/

DEFUN (interface_local,
       interface_local_cmd,
       "interface local",
       "make the selected interface turn to local.\n")
{
  int ret;
  struct interface *ifp;

  struct listnode *node;
  struct listnode *next;
  struct listnode *first;
  struct listnode *last;
  struct connected *ifc;
  struct prefix *p;
  struct route_node *rn;
  struct zebra_if *zebra_if;
  struct list *addr_list;
  
  
/*  struct zebra_if *if_data;*/

#if 0
	  ifp = (struct interface *) vty->index;
#else
	  ifp = if_get_by_vty_index(vty->index);
	  if(NULL == ifp)
		{
		  vty_out (vty, "%% Interface %s does not exist%s", (char*)(vty->index), VTY_NEWLINE);
		  return CMD_WARNING;
	  }
	  
#endif
#if 0
	if(product->product_type!= PRODUCT_TYPE_8610)
	{
		vty_out(vty,"Warnning: Only 86xx support set ve sub interface local and global !%s",VTY_NEWLINE);
		return CMD_WARNING;
	}
#endif
#if 0
	if(judge_ve_sub_interface(ifp->name) != VE_SUB_INTERFACE)
	{
		vty_out(vty,"Warnning: Only ve sub interface support set local and global !%s",VTY_NEWLINE);
		return CMD_WARNING;
	}
#endif
char cmdstr[64] = {0};
/*char *tmp=(char *)malloc(64);*/

if(CHECK_FLAG(ifp->if_scope, INTERFACE_LOCAL))
{
	vty_out(vty, "Interface %s is local already now !%s",ifp->name,VTY_NEWLINE);
	return CMD_WARNING;
}
SET_FLAG(ifp->if_scope, INTERFACE_LOCAL);
if(CHECK_FLAG(ifp->if_scope, INTERFACE_GLOBAL))
{
	UNSET_FLAG(ifp->if_scope, INTERFACE_GLOBAL);
}

#if 1
zebra_if = ifp->info;

  if (ifp->connected)
	{
	      last = NULL;
	      while ((node = (last ? last->next : listhead (ifp->connected))))
		{
		/*zlog_debug("%s : line %d ....\n",__func__,__LINE__);*/
		  ifc = listgetdata (node);
		  p = ifc->address;

		  
		  if (p->family == AF_INET)
		    {
		    
			/*zlog_debug("%s : line %d ....\n",__func__,__LINE__);*/
			  if((rn = route_node_lookup (zebra_if->ipv4_subnets, p)))
			  {
					route_unlock_node (rn);
					addr_list = (struct list *) rn->info;
					
					/* Remove addresses, secondaries first. */
					first = listhead (addr_list);
					for (node = first->next; node || first; node = next)
				  {
				  
					if (! node)
					  {
						node = first;
						first = NULL;
					  }
					next = node->next;
		  
					ifc = listgetdata (node);
					p = ifc->address;
					ret = if_unset_prefix (ifp, ifc);
	  				if (ret < 0)
				    {
				      vty_out (vty, "%% Can't unset interface IP address: %s.%s", 
					       safe_strerror(errno), VTY_NEWLINE);
				   //   return CMD_WARNING;
				    }
		  
					connected_down_ipv4 (ifp, ifc);
		  
					zebra_interface_address_delete_update (ifp, ifc);
		  
					UNSET_FLAG (ifc->conf, ZEBRA_IFC_REAL);
		  
					/* Remove from subnet chain. */
					list_delete_node (addr_list, node);
					route_unlock_node (rn);
					
					router_id_del_address(ifc);/*add*/
					/* Remove from interface address list (unconditionally). */
					listnode_delete (ifp->connected, ifc);
						connected_free (ifc);
				  }
		  
					/* Free chain list and respective route node. */
					list_delete (addr_list);
					rn->info = NULL;
					route_unlock_node (rn);

			  	}
			  	else
			  	{
					/*zlog_debug("%s : line %d ,#########################\n",__func__,__LINE__);*/
					router_id_del_address(ifc);
					listnode_delete (ifp->connected, ifc);
						connected_free (ifc);
				}
			  
		    }
#ifdef HAVE_IPV6
		  else if (p->family == AF_INET6)
		    {
		    
			/*zlog_debug("%s : line %d ,#############ipv6 1 ############\n",__func__,__LINE__);*/
			 ret = if_prefix_delete_ipv6 (ifp, ifc);

	      if (ret < 0)
				{
				  vty_out (vty, "%% %s : line %d ,Can't set interface IP address: %s.%s", 
					   __func__,__LINE__,safe_strerror(errno), VTY_NEWLINE);
	//			  return CMD_WARNING;
				}
		      connected_down_ipv6 (ifp, ifc);

		      zebra_interface_address_delete_update (ifp, ifc);

		      UNSET_FLAG (ifc->conf, ZEBRA_IFC_REAL);

		      if (CHECK_FLAG (ifc->conf, ZEBRA_IFC_CONFIGURED))
			last = node;
		      else
			{
			
				router_id_del_address(ifc);
			/*zlog_debug("%s : line %d ,##########ipv6 2###############\n",__func__,__LINE__);*/
			  listnode_delete (ifp->connected, ifc);
			  connected_free (ifc);
			}
		    }
#endif /* HAVE_IPV6 */

		}
	    }
#endif
/*gujd : 2012-07-28 , am 11: 50 . Delete it for change interface local/global switch rule (because vrrp).
  When set local mode , every board hold the interface (not delete) . Then ip address only sync to 
  the interface for his local board .*/
/*
zebra_interface_delete_update(ifp);
*/

  return CMD_SUCCESS;
}




DEFUN (interface_global,
       interface_global_cmd,
       "interface global",
       "make the selected interface turn to local.\n")
{
  
  int ret;
  struct interface *ifp;

  
  struct connected *ifc2;
  struct listnode *node;
  struct listnode *next;
  struct listnode *first;
  struct listnode *last;
  struct connected *ifc;
  struct prefix *p;
  struct route_node *rn;
  struct zebra_if *zebra_if;
  struct list *addr_list;

  
  u_char family;


#if 0
	  ifp = (struct interface *) vty->index;
#else
	  ifp = if_get_by_vty_index(vty->index);
	  if(NULL == ifp)
		{
		  vty_out (vty, "%% Interface %s does not exist%s", (char*)(vty->index), VTY_NEWLINE);
		  return CMD_WARNING;
	  }
	  
#endif
#if 0
	if(product->product_type!= PRODUCT_TYPE_8610)
	{
		vty_out(vty,"Warnning: Only 86xx support set ve sub interface local and global !%s",VTY_NEWLINE);
		return CMD_WARNING;
	}
#endif
#if 0
	if(judge_ve_sub_interface(ifp->name) != VE_SUB_INTERFACE)
	{
		vty_out(vty,"Warnning: Only ve sub interface support set local and global !%s",VTY_NEWLINE);
		return CMD_WARNING;
	}
#endif

char cmdstr[64] = {0};
if(CHECK_FLAG(ifp->if_scope, INTERFACE_LOCAL))
{
	UNSET_FLAG(ifp->if_scope, INTERFACE_LOCAL);
}
SET_FLAG(ifp->if_scope, INTERFACE_GLOBAL);
/*gujd : 2012-07-28 , am 11: 50 . Delete it for change interface local/global switch rule (because vrrp).
  When set local mode , every board hold the interface (not delete) . Then ip address only sync to 
  the interface for his local board .*/
/*
zebra_interface_add_update(ifp);
*/
if(if_is_up(ifp))
	zebra_interface_up_update(ifp);
else
	zebra_interface_down_update(ifp);

for (ALL_LIST_ELEMENTS (ifp->connected, node, next, ifc)) 
{
	family = ifc->address->family;
	if(family == AF_INET)
	{
	 ret = if_set_prefix (ifp, ifc);
	 if (ret < 0)
	  {
		zlog_err("When turn to local , can't set interface IPv4 address for kernel: %s.\n", 
			 safe_strerror(errno));
	  }
				  
	 SET_FLAG (ifc->conf, ZEBRA_IFC_REAL);
	 /*gujd : 2012-07-26, pm 5:00. Delete it for reduplicating connect route.
		(rib->refcnt++, cased cann't delete it when no ip).Fixed bug AXSSZFI-1010. */
#if 0
	 if(if_is_operative(ifp))
	 {
		zlog_debug("%s : line %d, goto up connect route(ipv4) when turn global......\n",__func__,__LINE__);
	 	connected_up_ipv4(ifp,ifc);
	 	}
#endif	 
	 if (CHECK_FLAG (ifc->conf, ZEBRA_IFC_REAL)) 
	 {
		 zlog_debug("%s : line %d, goto send interface address(ipv4) when turn global......\n",__func__,__LINE__);
		 SET_FLAG (ifc->conf, ZEBRA_IFC_CONFIGURED);
		 zebra_interface_address_add_update(ifp,ifc);
		}	
	}
	else if(family == AF_INET6 )
	{
	 ret = if_prefix_add_ipv6(ifp, ifc);
	 if (ret < 0)
	  {
		zlog_err("When turn to local , can't set interface IPv6 address for kernel: %s.\n", 
			 safe_strerror(errno));
	  }
				  
	 SET_FLAG (ifc->conf, ZEBRA_IFC_REAL); 
	 if(if_is_operative(ifp))
	 {
		zlog_debug("%s : line %d, goto up connect route(ipv6) when turn global......\n",__func__,__LINE__);
	 	connected_up_ipv6(ifp,ifc);
	 	}
	 
	 if (CHECK_FLAG (ifc->conf, ZEBRA_IFC_REAL)) 
	 {
		 zlog_debug("%s : line %d, goto send interface address(ipv6) when turn global......\n",__func__,__LINE__);
		 SET_FLAG (ifc->conf, ZEBRA_IFC_CONFIGURED);
		 zebra_interface_address_add_update(ifp,ifc);
		}	
	}
	else
		{
			zlog_err("Unkown protocol !\n");
		}
}

/*
	if (IS_ZEBRA_DEBUG_KERNEL)
	  zlog_debug ("interface %s index %d becomes active.", 
			  ifp->name, ifp->ifindex);
*/


  return CMD_SUCCESS;
}

/*gujd : 2013-02-22, pm 4:43. Add code for radio interface info switch.
For radio get netlink info (used with under system debug node 
of command :interface radio level (0|1|2) ) when show interface.*/
DEFUN (radio_interface_show, 
       radio_interface_show_cmd,
       "radio interface show (enable|disable)",
       "Interface specific description\n"
       "Characters describing this interface\n"
       "Display info\n"
       "Display radio interface info\n"
       "Hide radio interface info\n")
{
  struct interface *ifp;
  struct listnode *node;
  
  if(strncmp(argv[0],"enable",6)== 0)
  	radio_interface_show_enable = 1;
  else if(strncmp(argv[0],"disable",7)== 0)
	radio_interface_show_enable = 0;
  else
    return CMD_WARNING;

  return CMD_SUCCESS;
  
}



/*radio interface done*/
DEFUN (radio_interface_deal, 
       radio_interface_deal_cmd,
       "radio interface (enable|disable)",
       "Interface specific description\n"
       "Characters describing this interface\n")
{
  struct interface *ifp;
  struct listnode *node;
  
  
  if(strncmp(argv[0],"enable",6)== 0)
  	radio_interface_enable = 1;
  else
  	if(strncmp(argv[0],"disable",7)== 0)
	  radio_interface_enable = 0;
  else
    return CMD_WARNING;

  if(radio_interface_enable == 1)
  {
	 for (ALL_LIST_ELEMENTS_RO (iflist, node, ifp))
	 {
	   if (strncmp(ifp->name,"r",1) == 0)
		zebra_interface_add_update(ifp);
	 /*aster_send_interface_add(master_board,ifp);*/
	 }
  }
  else
  	{
  	
	  radio_interface_enable = 0;
	 return CMD_SUCCESS;
  	}
  	

  return CMD_SUCCESS;
}


/* Show all or specified interface to vty. */
DEFUN (show_interface, show_interface_cmd,
       "show interface [IFNAME]",  
       SHOW_STR
       "Interface status and configuration\n"
       "Inteface name\n")
{
  struct listnode *node;
  struct interface *ifp;
  
#ifdef HAVE_PROC_NET_DEV
  /* If system has interface statistics via proc file system, update
     statistics. */
  ifstat_update_proc ();
#endif /* HAVE_PROC_NET_DEV */
#ifdef HAVE_NET_RT_IFLIST
  ifstat_update_sysctl ();
#endif /* HAVE_NET_RT_IFLIST */

  /*INTERFACE_FLOW_STATISTICS_SAMPLING_INTEGRATED_RTM*/
  if( product && product->board_type == BOARD_IS_ACTIVE_MASTER)
   rtm_deal_interface_flow_stats_sampling_integrated(INTERFACE_FLOW_STATISTICS_SAMPLING_INTEGRATED_RTM, NULL, PROCESS_NAME_SNMP);

  /* Specified interface print. */
  if (argc != 0)
    {
#if (defined _D_WCPSS_ || defined _D_CC_)	

		int wtpid,radioid,wlanid,cnuid,ethid,cbat_id;
		unsigned char id;
		char name[32];
		int index = 0;
		int slotid = 0;
		memset(name, 0, 32);
	  if (!strncasecmp(argv[0],"wlanl",5)){
		  if(parse_char_ID(argv[0]+5, &id)){
			  sprintf(name,"wlanl%d-%d",index,id);
			  ifp = if_lookup_by_name (name);
		  }else{
			  ifp = if_lookup_by_name (argv[0]);
		  }
	  }else if ((!strncasecmp(argv[0],"wlan",4))&&(!strncasecmp(argv[0],"wlanl",5))){
		if(parse_char_ID(argv[0]+4, &id)){
			sprintf(name,"wlan%d-%d",index,id);
			ifp = if_lookup_by_name (name);
		}else{
			ifp = if_lookup_by_name (argv[0]);
		}
	  }else if (!strncasecmp(argv[0],"radio",5)){
		  if(parse_radio_ifname(argv[0]+5, &wtpid,&radioid,&wlanid) == 0){
			  sprintf(name,"r%d-%d-%d.%d",index,wtpid,radioid,wlanid);
			  ifp = if_lookup_by_name (name);
		  }else{
		  	sprintf(name,"r%s",argv[0]+5);
			  ifp = if_lookup_by_name (name);
		  }		
	  }else if (!strncasecmp(argv[0],"r",1)){
		  if(parse_radio_ifname(argv[0]+1, &wtpid,&radioid,&wlanid) == 0){
			  sprintf(name,"r%d-%d-%d.%d",index,wtpid,radioid,wlanid);
			  ifp = if_lookup_by_name (name);
		  }else{
			  ifp = if_lookup_by_name (argv[0]);
		  }		
	  }
	  else if (!strncasecmp(argv[0],"ebrl",4)){
		  if(parse_char_ID(argv[0]+4, &id)){
			  sprintf(name,"ebrl%d-%d",index,id);
			  ifp = if_lookup_by_name (name);
		  }else{
			  ifp = if_lookup_by_name (argv[0]);
		  }
		
	  }else if ((!strncasecmp(argv[0],"ebr",3))&&(strncasecmp(argv[0],"ebrl",4))){
		  if(parse_char_ID(argv[0]+3, &id)){
			  sprintf(name,"ebr%d-%d",index,id);
			  ifp = if_lookup_by_name (name);
		  }else{
			  ifp = if_lookup_by_name (argv[0]);
		  }
		
	  }else if (!strncasecmp(argv[0],"cnu",3)){
		  if(parse_cnu_ifname(argv[0]+3,&cbat_id,&cnuid,&ethid) == 0){
			  sprintf(name,"cnu%d-%d-%d.%d",index,cbat_id,cnuid,ethid);
			  ifp = if_lookup_by_name (name);
		  }else{
			  ifp = if_lookup_by_name (argv[0]);
		  }
		
	  }else
#endif
	      ifp = if_lookup_by_name (argv[0]);
      if (ifp == NULL) 
	{
	  vty_out (vty, "%% Can't find interface %s%s", argv[0],
		   VTY_NEWLINE);
	  return CMD_WARNING;
	}
      if_dump_vty (vty, ifp);
      return CMD_SUCCESS;
    }

  /* All interface print. */
  for (ALL_LIST_ELEMENTS_RO (iflist, node, ifp))
    if_dump_vty (vty, ifp);

  return CMD_SUCCESS;
}

DEFUN (show_interface_desc,
       show_interface_desc_cmd,
       "show interface description",
       SHOW_STR
       "Interface status and configuration\n"
       "Interface description\n")
{
  struct listnode *node;
  struct interface *ifp;

  vty_out (vty, "Interface       Status  Protocol  Description%s", VTY_NEWLINE);
  for (ALL_LIST_ELEMENTS_RO (iflist, node, ifp))
    {
      int len;

      len = vty_out (vty, "%s", ifp->name);
      vty_out (vty, "%*s", (16 - len), " ");
      
      if (if_is_up(ifp))
	{
	  vty_out (vty, "up      ");
	  if (CHECK_FLAG(ifp->status, ZEBRA_INTERFACE_LINKDETECTION))
	    {
	      if (if_is_running(ifp))
		vty_out (vty, "up        ");
	      else
		vty_out (vty, "down      ");
	    }
	  else
	    {
	      vty_out (vty, "unknown   ");
	    }
	}
      else
	{
	  vty_out (vty, "down    down      ");
	}

      if (ifp->desc)
	vty_out (vty, "%s", ifp->desc);
      vty_out (vty, "%s", VTY_NEWLINE);
    }
  return CMD_SUCCESS;
}

DEFUN (multicast,
       multicast_cmd,
       "multicast",
       "Set multicast flag to interface\n")
{
  int ret;
  struct interface *ifp;
  struct zebra_if *if_data;

#if 0
	ifp = (struct interface *) vty->index;
#else
	ifp = if_get_by_vty_index(vty->index);
	if(NULL == ifp)
	  {
		vty_out (vty, "%% Interface %s does not exist%s",(char*)(vty->index), VTY_NEWLINE);
		return CMD_WARNING;
	}
	
#endif

  if (CHECK_FLAG (ifp->status, ZEBRA_INTERFACE_ACTIVE))
    {
      ret = if_set_flags (ifp, IFF_MULTICAST);
      if (ret < 0)
	{
	  vty_out (vty, "Can't set multicast flag%s", VTY_NEWLINE);
	  return CMD_WARNING;
	}
      if_refresh (ifp);
    }
  if_data = ifp->info;
  if_data->multicast = IF_ZEBRA_MULTICAST_ON;

  return CMD_SUCCESS;
}

DEFUN (no_multicast,
       no_multicast_cmd,
       "no multicast",
       NO_STR
       "Unset multicast flag to interface\n")
{
  int ret;
  struct interface *ifp;
  struct zebra_if *if_data;

#if 0
	  ifp = (struct interface *) vty->index;
#else
	  ifp = if_get_by_vty_index(vty->index);
	  if(NULL == ifp)
		{
		  vty_out (vty, "%% Interface %s does not exist%s", (char*)(vty->index), VTY_NEWLINE);
		  return CMD_WARNING;
	  }
	  
#endif
  if (CHECK_FLAG (ifp->status, ZEBRA_INTERFACE_ACTIVE))
    {
      ret = if_unset_flags (ifp, IFF_MULTICAST);
      if (ret < 0)
	{
	  vty_out (vty, "Can't unset multicast flag%s", VTY_NEWLINE);
	  return CMD_WARNING;
	}
      if_refresh (ifp);
    }
  if_data = ifp->info;
  if_data->multicast = IF_ZEBRA_MULTICAST_OFF;

  return CMD_SUCCESS;
}

DEFUN (linkdetect,
       linkdetect_cmd,
       "link-detect",
       "Enable link detection on interface\n")
{
  struct interface *ifp;
  int if_was_operative;
  
#if 0
	  ifp = (struct interface *) vty->index;
#else
	  ifp = if_get_by_vty_index(vty->index);
	  if(NULL == ifp)
		{
		  vty_out (vty, "%% Interface %s does not exist%s", (char*)(vty->index), VTY_NEWLINE);
		  return CMD_WARNING;
	  }
	  
#endif
  if_was_operative = if_is_operative(ifp);
  SET_FLAG(ifp->status, ZEBRA_INTERFACE_LINKDETECTION);

  /* When linkdetection is enabled, if might come down */
  /*gjd : add for Distribute System , when turn on linkdetect , this step only deal loard board .*/
  SET_FLAG(ifp->linkdetection_flags,SET_LINKDETECTION);/*used to limit to send to other board*/
  if (!if_is_operative(ifp) && if_was_operative) if_down(ifp);  
  UNSET_FLAG(ifp->linkdetection_flags,SET_LINKDETECTION);

  /*gjd : add for Distribute System, active master send turn on linkdetect to oter boards (keep sync ifp->status).*/
  if(product != NULL && product->board_type == BOARD_IS_ACTIVE_MASTER)
  {
	redistribute_interface_linkdetection(ifp,1);
  }
  /* FIXME: Will defer status change forwarding if interface
     does not come down! */

  return CMD_SUCCESS;
}


DEFUN (no_linkdetect,
       no_linkdetect_cmd,
       "no link-detect",
       NO_STR
       "Disable link detection on interface\n")
{
  struct interface *ifp;
  int if_was_operative;

#if 0
	  ifp = (struct interface *) vty->index;
#else
	  ifp = if_get_by_vty_index(vty->index);
	  if(NULL == ifp)
		{
		  vty_out (vty, "%% Interface %s does not exist%s", (char*)(vty->index), VTY_NEWLINE);
		  return CMD_WARNING;
	  }
	  
#endif
  if_was_operative = if_is_operative(ifp);
  UNSET_FLAG(ifp->status, ZEBRA_INTERFACE_LINKDETECTION);
  
  /* Interface may come up after disabling link detection */
  /*gjd : add for Distribute System , when turn off linkdetect , this step only deal loard board .*/
  SET_FLAG(ifp->linkdetection_flags,SET_LINKDETECTION);
  if (if_is_operative(ifp) && !if_was_operative) if_up(ifp);
  UNSET_FLAG(ifp->linkdetection_flags,SET_LINKDETECTION);
  
  /*gjd : add for Distribute System, active master send turn off linkdetect to oter boards (keep sync ifp->status).*/
  if(product != NULL && product->board_type == BOARD_IS_ACTIVE_MASTER)
  {
	redistribute_interface_linkdetection(ifp,0);
  }

  /* FIXME: see linkdetect_cmd */

  return CMD_SUCCESS;
}

DEFUN (shutdown_if,
       shutdown_if_cmd,
       "shutdown",
       "Shutdown the selected interface\n")
{
  int ret;
  struct interface *ifp;
  struct zebra_if *if_data;

#if 0
	  ifp = (struct interface *) vty->index;
#else
	  ifp = if_get_by_vty_index(vty->index);
	  if(NULL == ifp)
		{
		  vty_out (vty, "%% Interface %s does not exist%s", (char*)(vty->index), VTY_NEWLINE);
		  return CMD_WARNING;
	  }
	  
#endif

#if 0/*gujd :2012-08-14,pm 5:15. Delete it for when interface local shutdown or no shutdown ,no ip address cannot delete connect route.*/
if((judge_ve_sub_interface(ifp->name) == VE_SUB_INTERFACE) && (CHECK_FLAG(ifp->if_scope,INTERFACE_LOCAL)))
{
	zlog_debug("go to down the interface .\n");
	ifp->flags &= ~(IFF_UP|IFF_RUNNING);
	zebra_interface_down_update(ifp);
	if_down_redistribute(ifp);/*gujd: 2012-05-31, am 11:01 . Add for interface local shutdown ,for connected route update.*/
	
	return CMD_SUCCESS;
}
#endif
  ret = if_unset_flags (ifp, IFF_UP);
  if (ret < 0)
    {
      vty_out (vty, "Can't shutdown interface%s", VTY_NEWLINE);
      return CMD_WARNING;
    }
  if_refresh (ifp);
  if_data = ifp->info;
  if_data->shutdown = IF_ZEBRA_SHUTDOWN_ON;

  /*gjd : add for Distribute System .*/
  if(product != NULL)/*when turn on detection , and interface is up not running , active don't send down state to vice .so add for send to vice*/
  	{
  		if(CHECK_FLAG(ifp->status, ZEBRA_INTERFACE_LINKDETECTION))
  			zebra_interface_down_update(ifp);
  	}
  /*2011-8-25:am 10:10*/

  return CMD_SUCCESS;
}

DEFUN (no_shutdown_if,
       no_shutdown_if_cmd,
       "no shutdown",
       NO_STR
       "Shutdown the selected interface\n")
{
  int ret;
  struct interface *ifp;
  struct zebra_if *if_data;

#if 0
	  ifp = (struct interface *) vty->index;
#else
	  ifp = if_get_by_vty_index(vty->index);
	  if(NULL == ifp)
		{
		  vty_out (vty, "%% Interface %s does not exist%s", (char*)(vty->index), VTY_NEWLINE);
		  return CMD_WARNING;
	  }
	  
#endif

#if 0/*gujd :2012-08-14,pm 5:15. Delete it for when interface local shutdown or no shutdown ,no ip address cannot delete connect route.*/
  if((judge_ve_sub_interface(ifp->name) == VE_SUB_INTERFACE) && (CHECK_FLAG(ifp->if_scope,INTERFACE_LOCAL)))
  {
	  zlog_debug("go to up the interface .\n");
	  ifp->flags |= (IFF_UP|IFF_RUNNING);
	  zebra_interface_up_update(ifp);
	  if_up_redistribute(ifp);/*gujd: 2012-05-31, am 11:01 . Add for interface local no shutdown ,for connected route update.*/
	  
	  return CMD_SUCCESS;
  }
#endif

  ret = if_set_flags (ifp, IFF_UP | IFF_RUNNING);
  if (ret < 0)
    {
      vty_out (vty, "Can't up interface%s", VTY_NEWLINE);
      return CMD_WARNING;
    }
  if_refresh (ifp);
  if_data = ifp->info;
  if_data->shutdown = IF_ZEBRA_SHUTDOWN_OFF;
  
  /*gjd : add for Distribute System .*/
  if(product != NULL)/*when turn on detection , and interface is down , active don't send up state to vice .so add for send to vice*/
  	{
  		if(CHECK_FLAG(ifp->status, ZEBRA_INTERFACE_LINKDETECTION))
  		  zebra_interface_up_update(ifp);
  	}
  /*2011-8-25:am 10:10*/

  return CMD_SUCCESS;
}

DEFUN (bandwidth_if,
       bandwidth_if_cmd,
       "bandwidth <1-10000000>",
       "Set bandwidth informational parameter\n"
       "Bandwidth in kilobits\n")
{
  struct interface *ifp;   
  unsigned int bandwidth;
  
#if 0
	  ifp = (struct interface *) vty->index;
#else
	  ifp = if_get_by_vty_index(vty->index);
	  if(NULL == ifp)
		{
		  vty_out (vty, "%% Interface %s does not exist%s", (char*)(vty->index), VTY_NEWLINE);
		  return CMD_WARNING;
	  }
	  
#endif
  bandwidth = strtol(argv[0], NULL, 10);

  /* bandwidth range is <1-10000000> */
  if (bandwidth < 1 || bandwidth > 10000000)
    {
      vty_out (vty, "Bandwidth is invalid%s", VTY_NEWLINE);
      return CMD_WARNING;
    }
  
  ifp->bandwidth = bandwidth;

  /* force protocols to recalculate routes due to cost change */
  if (if_is_operative (ifp))
    zebra_interface_up_update (ifp);
  
  return CMD_SUCCESS;
}

DEFUN (no_bandwidth_if,
       no_bandwidth_if_cmd,
       "no bandwidth",
       NO_STR
       "Set bandwidth informational parameter\n")
{
  struct interface *ifp;   
  
#if 0
	  ifp = (struct interface *) vty->index;
#else
	  ifp = if_get_by_vty_index(vty->index);
	  if(NULL == ifp)
		{
		  vty_out (vty, "%% Interface %s does not exist%s", (char*)(vty->index), VTY_NEWLINE);
		  return CMD_WARNING;
	  }
	  
#endif

  ifp->bandwidth = 0;
  
  /* force protocols to recalculate routes due to cost change */
  if (if_is_operative (ifp))
    zebra_interface_up_update (ifp);

  return CMD_SUCCESS;
}

ALIAS (no_bandwidth_if,
       no_bandwidth_if_val_cmd,
       "no bandwidth <1-10000000>",
       NO_STR
       "Set bandwidth informational parameter\n"
       "Bandwidth in kilobits\n")


       
static int ip_address_install(struct vty *vty, struct interface *ifp,
		    const char *addr_str, const char *peer_str,
		    const char *label)
{
  struct prefix_ipv4 cp;
  struct prefix_ipv4 mycp;
  struct prefix_ipv4 myprefix;
  struct connected *ifc;
  struct connected *myifc;
  struct prefix_ipv4 *p;
  struct listnode *myld;
  struct listnode *mylistd;
  struct interface *myifp;
  struct listnode *node;
  struct listnode *next;
  struct connected *ifc2;
  int count = 0;
  int ret;
  int ip_have_set = 0;/*gujd: 2012-08-13, pm 5:53. Add  for when interface local, for ip not install kernel ,
  						the ZEBRA_IFC_REAL not set , will case ip set under interface reduplicate by if_subnet_add */

  ret = str2prefix_ipv4 (addr_str, &cp);
  if (ret <= 0)
    {
      vty_out (vty, "%% Malformed address %s", VTY_NEWLINE);
      return CMD_WARNING;
    }
  /*gjd: add  for check ip address like 0.x.x.x/x .*/
  if(ZERONET(cp.prefix.s_addr))
  	
  	{
      vty_out (vty, "%% Malformed address %s %s", addr_str,VTY_NEWLINE);
      return CMD_WARNING;
    }

  
/**add gjd : for limit ip(ipv4 and ipv6) amount to 8 ,not include system default ipv6 fe80:** **/
#if 0
	assert(ifp);
	assert(ifp->connected);
#else
	if(! ifp || !ifp->connected)
	{
		zlog(NULL, LOG_CRIT, "line %u, function %s",
			 __LINE__,(__func__ ? __func__ : "?"));
		zlog_backtrace(LOG_CRIT);
		return CMD_WARNING;
	}
#endif
	if (ifp->connected)
    {
      for (ALL_LIST_ELEMENTS (ifp->connected, node, next, ifc2))
		{
		  	if(CHECK_FLAG (ifc2->conf, ZEBRA_IFC_CONFIGURED))
				count ++;
		}
    }
   if(IFC_MAX <= count)
	{
		vty_out(vty,"%% Warning: There already has %d address on %s.%s",IFC_MAX,ifp->name,VTY_NEWLINE);
		return CMD_WARNING;
	}

 #if 0
  if(IFC_MAX <= ifp->connected->count){
		vty_out(vty,"%% There already has %d address on %s.%s",IFC_MAX,
		ifp->name,
			VTY_NEWLINE);
	return CMD_WARNING;
  }
  #endif
  if(cp.prefixlen>30&&strcmp(ifp->name,"lo")){
  	vty_out(vty,"%% Masklen must shorter than 30.%s",VTY_NEWLINE);
	return CMD_WARNING;
  }
  if(prefix_verify_ipv4(PREFIXHOST|PREFIXNORMAL,&cp)){
  	vty_out(vty,"%%  Please enter a host ip address.%s",VTY_NEWLINE);
	return CMD_WARNING;
  }
#if 0  
  for(ALL_LIST_ELEMENTS_RO(iflist,myld,myifp)){
  	  for (ALL_LIST_ELEMENTS_RO (myifp->connected, mylistd, myifc)){
	  		myprefix.family=myifc->address->family;
			myprefix.prefixlen=(myifc->address->prefixlen>cp.prefixlen ? cp.prefixlen : myifc->address->prefixlen);
			myprefix.prefix=(myifc->address->u).prefix4;
	  		apply_mask_ipv4(&myprefix);
			mycp.family=cp.family;
			mycp.prefixlen=(cp.prefixlen>myprefix.prefixlen ? myprefix.prefixlen : cp.prefixlen);
			mycp.prefix=cp.prefix;
			if(prefix_match((struct prefix *)&myprefix,(struct prefix *)&mycp)){
				vty_out(vty,"%% There already has the same network on %s.%s",
					myifp->name,
						VTY_NEWLINE);
				return CMD_WARNING;
			}
  	   }
   }
#endif
  /*************2008-11-7 9:27:47************************/
  ifc = connected_check (ifp, (struct prefix *) &cp);
  if (! ifc)
    {
      ifc = connected_new ();
      ifc->ifp = ifp;

      /* Address. */
      p = prefix_ipv4_new ();
      *p = cp;
      ifc->address = (struct prefix *) p;

      /* Broadcast. */
      if (p->prefixlen <= IPV4_MAX_PREFIXLEN-2)
	{
	  p = prefix_ipv4_new ();
	  *p = cp;
	  p->prefix.s_addr = ipv4_broadcast_addr(p->prefix.s_addr,p->prefixlen);
	  ifc->destination = (struct prefix *) p;
	}

      /* Label. */
      if (label)
	ifc->label = XSTRDUP (MTYPE_CONNECTED_LABEL, label);

      /* Add to linked list. */
      listnode_add (ifp->connected, ifc);
    }
  else
  	{
  		zlog_info("this ip is set so avoid to set again.\n");
  		ip_have_set = 1;
  	}

  /* This address is configured from zebra. */
  if (! CHECK_FLAG (ifc->conf, ZEBRA_IFC_CONFIGURED))
    SET_FLAG (ifc->conf, ZEBRA_IFC_CONFIGURED);

  /* In case of this route need to install kernel. */
  if (! CHECK_FLAG (ifc->conf, ZEBRA_IFC_REAL)
      && (CHECK_FLAG (ifp->status, ZEBRA_INTERFACE_ACTIVE)||product != NULL))
    {

	int intfdown =0;
	int intf_is_run=0;
/*changed  by scx */
	  /* Some system need to up the interface to set IP address. */
/*	  if (! if_is_up (ifp))*/

	/* Distibute System. interface : eth-sub, ve-sub and eth*/
   if((judge_eth_sub_interface(ifp->name)==ETH_SUB_INTERFACE)
		||(judge_ve_sub_interface(ifp->name)==VE_SUB_INTERFACE)
		||(judge_eth_interface(ifp->name)==ETH_INTERFACE)
		||(strncmp(ifp->name,"mng",3)== 0))
   	{
   		/*when ve sub interface set local ,in active master is virtual ,so not goto install kernel ,only send to his board .*/
   		if((judge_ve_sub_interface(ifp->name)==VE_SUB_INTERFACE)&&(CHECK_FLAG(ifp->if_scope, INTERFACE_LOCAL)))
		{
			if(ip_have_set == 1)
			{
				zlog_info("the interface is set local and this ip is set .\n");
				
				return CMD_SUCCESS;
			}
			if(product->product_type == PRODUCT_TYPE_8610
				||product->product_type == PRODUCT_TYPE_8606
				||product->product_type == PRODUCT_TYPE_8800)/*8606 and 8610*/
			{
				zlog_debug("The interface %s set local , not let kernel install .Only send to his board .\n",ifp->name);
				/*goto skip;*/
				if_subnet_add (ifp, ifc);/*USE for write loadconfig*/
				zebra_interface_address_add_update (ifp, ifc);
				if (if_is_operative(ifp))
					connected_up_ipv4 (ifp, ifc);
				
				return CMD_SUCCESS;
			}
			else
			  if(product->product_type == PRODUCT_TYPE_7605I
			  	||product->product_type == PRODUCT_TYPE_8603)/*7605i and 8603*/
			  {
			  	int slot_num = 0;

				slot_num = get_slot_num(ifp->name);
				zlog_debug("The interface %s set local .\n",ifp->name);
				/*goto skip;*/
				if(slot_num == product->board_id)/*76 and 8603 local interface : install to kernel.*/
				{
					ret = if_set_prefix (ifp, ifc);
     				 if (ret < 0)
					{
					  vty_out (vty, "%Can't set interface IP address: %s.%s", safe_strerror(errno), VTY_NEWLINE);		
					  return CMD_WARNING;
					}
					zlog_debug("The interface %s set local and ip install kernel sucess .\n",ifp->name);
				}
				
				if_subnet_add (ifp, ifc);/*USE for write loadconfig*/
				
				if(slot_num == product->board_id)/*76: local real interface*/
				  SET_FLAG (ifc->conf, ZEBRA_IFC_REAL);
				
				zebra_interface_address_add_update (ifp, ifc);
				if (if_is_operative(ifp))
				 connected_up_ipv4 (ifp, ifc);
				
				 return CMD_SUCCESS;
				
			  }
			  else
				{
					zlog_warn("Unkown product type !\n");
					return CMD_WARNING;
				}
			
		}
   		ret = if_set_prefix (ifp, ifc);
      if (ret < 0)
		{
		  vty_out (vty, "%% Can't set interface IP address: %s.%s", 
			   safe_strerror(errno), VTY_NEWLINE);		
		  return CMD_WARNING;
		}
	  
      /* Add to subnet chain list (while marking secondary attribute). */
      if_subnet_add (ifp, ifc);

      /* IP address propery set. */
      SET_FLAG (ifc->conf, ZEBRA_IFC_REAL);

      /* Update interface address information to protocol daemon. */
      zebra_interface_address_add_update (ifp, ifc);

      /* If interface is up register connected route. */
      if (if_is_operative(ifp))
		connected_up_ipv4 (ifp, ifc);
	
		
		return CMD_SUCCESS;
	  
   		
   	}

   /* Distibute System. interface : wlan ,ebr and radio*/
   if(product != NULL)
   {
     if(strncmp(ifp->name,"ve",2)!=0 && strncmp(ifp->name,"eth",3)!=0)
   	 {
/*   	 	if((((strncmp(ifp->name,"wlanl",5)!= 0)&&(strncmp(ifp->name,"wlan",4)==0))
		||((strncmp(ifp->name,"ebrl",4)!= 0)&&(strncmp(ifp->name,"ebr",3)==0)))
  			&& (CHECK_FLAG(ifp->if_scope, INTERFACE_LOCAL)))
*/
  			
   	 	if(((strncmp(ifp->name,"wlan",4)==0)||(strncmp(ifp->name,"ebr",3)==0))
  			&& (CHECK_FLAG(ifp->if_scope, INTERFACE_LOCAL)))
   	    {
   	    	/*is rpa interface already or real interface*/
   	    	if((ifp->if_types == RPA_INTERFACE)
				|| ((ifp->if_types == REAL_INTERFACE || ifp->if_types == 0)&&(ifp->ifindex != IFINDEX_INTERNAL)))
	   	    {
				if(product->product_type == PRODUCT_TYPE_8610
					||product->product_type == PRODUCT_TYPE_8606
					||product->product_type == PRODUCT_TYPE_8800)/*8610 and 8606*/
				{
					zlog_debug("The interface %s set local , not let kernel install .Only send to his board .\n",ifp->name);
					/*goto skip;*/
					if_subnet_add (ifp, ifc);/*USE for write loadconfig*/
					zebra_interface_address_add_update (ifp, ifc);
					if (if_is_operative(ifp))
						connected_up_ipv4 (ifp, ifc);
					
					return CMD_SUCCESS;
				}
				else
				  if(product->product_type == PRODUCT_TYPE_7605I
				  	||product->product_type == PRODUCT_TYPE_8603)/*7605i and 8603*/
				  {
				  	int slot_num = 0;

					slot_num = get_slot_num(ifp->name);
					zlog_debug("The interface %s set local .\n",ifp->name);
					/*goto skip;*/
					if(slot_num == product->board_id)/*76 local interface : install to kernel.*/
					{
						ret = if_set_prefix (ifp, ifc);
	     				 if (ret < 0)
						{
						  vty_out (vty, "Can't set interface IP address: %s.%s", safe_strerror(errno), VTY_NEWLINE);		
						  return CMD_WARNING;
						}
						zlog_debug("The interface %s set local and ip install kernel sucess .\n",ifp->name);
					}
					
					if_subnet_add (ifp, ifc);/*USE for write loadconfig*/
					
					if(slot_num == product->board_id)/*76: local real interface*/
					  SET_FLAG (ifc->conf, ZEBRA_IFC_REAL);
					
					zebra_interface_address_add_update (ifp, ifc);
					if (if_is_operative(ifp))
					 connected_up_ipv4 (ifp, ifc);
					
					 return CMD_SUCCESS;
					
				  }
				  else
					{
						zlog_warn("Unkown product type !\n");
						return CMD_WARNING;
					}
	   	 		}
			else
			if(ifp->if_types == VIRTUAL_INTERFACE)/*virtual interface */
			 {   
			   /*create_rpa_interface();*/
			   ret = create_rpa_interface(ifp);
				if(ret < 0)
				 {
				   zlog_debug("%s : line %d, creat rpa interface(%s) failed : %s \n ",__func__,__LINE__,safe_strerror(errno),ifp->name);
		   		  /*return NULL;*/
				 }
				 else
				 { 
				   /*virtual interface ==> RPA interface*/
				   ifp->if_types = RPA_INTERFACE;
				   zlog_debug("%s : line %d, creat rpa interface(%s) sucess, so virtual turn to rpa interface...\n",__func__,__LINE__,ifp->name);
				 }

				sleep(1);
				
	/*			ifp->ifindex = if_get_if_index_by_ioctl(ifp);*/
				ifp->ifindex = if_get_index_by_ioctl(ifp);

				if(ifp->ifindex <= 0)
				 {
				   zlog_warn("get rpa(%s) ifindex by ioctl failed : %s ..\n",ifp->name,safe_strerror(errno));
				   return CMD_WARNING;
				 }
				zlog_debug("get rpa(%s) ifindex by ioctl sucess is %d .....",ifp->name,ifp->ifindex);

				/*gujd : 2012-08-01, Add for ospf , rip update the interface index when create rpa .*/
				zebra_interface_add_update(ifp);
				
				if_subnet_add (ifp, ifc);/*USE for write loadconfig*/
				zebra_interface_address_add_update (ifp, ifc);
				if (if_is_operative(ifp))
				  connected_up_ipv4 (ifp, ifc);

				return CMD_SUCCESS;
				
	   		 }
		  }
   	  int count_ip = 0;
	 /* count_ip = ip_address_count(ifp);*/
	  count_ip = ip_address_count_except_fe80(ifp);
	  zlog_debug("####### interface(%s ) ip count(%d)######\n",ifp->name,count_ip);
	
	 if((ifp->if_types == REAL_INTERFACE) && count_ip <= 1)
	   {
		   ifp->slot = get_slot_num(ifp->name);
		   zlog_debug("get slot num %d ....\n",ifp->slot);
		   if(strncmp(ifp->name, "ve",2) != 0)
		   {
		   	ret = register_rpa_interface_table(ifp);
			if(ret < 0)
			   {
				   zlog_warn("register rpa interface(%s) table in local board failed : %s ..\n",ifp->name,safe_strerror(errno));
			   }
		   }
	   }
	  if((ifp->if_types == VIRTUAL_INTERFACE) && strncmp(ifp->name,"ve",2)!=0)
	   {   
		   /*create_rpa_interface();*/
		   
		   ret = create_rpa_interface(ifp);
			if(ret < 0)
			 {
			   zlog_debug("%s : line %d, creat rpa interface(%s) failed : %s \n ",__func__,__LINE__,ifp->name,safe_strerror(errno));
	   		  /*return NULL;*/
			 }
			else
			 { 
			   /*virtual interface ==> RPA interface*/
			   ifp->if_types = RPA_INTERFACE;
			   zlog_debug("%s : line %d, creat rpa interface(%s) sucess, so virtual turn to rpa interface...\n",__func__,__LINE__,ifp->name);
			 }

			sleep(1);
			
/*			ifp->ifindex = if_get_if_index_by_ioctl(ifp);*/
			ifp->ifindex = if_get_index_by_ioctl(ifp);

			if(ifp->ifindex <= 0)
			 {
			   zlog_warn("get rpa(%s) ifindex by ioctl failed : %s ..\n",ifp->name,safe_strerror(errno));
			   return CMD_WARNING;
			 }
			zlog_debug("get rpa(%s) ifindex by ioctl sucess is %d .....",ifp->name,ifp->ifindex);
			/*gujd : 2012-08-01, Add for ospf , rip update the interface index when create rpa .*/
			zebra_interface_add_update(ifp);
			
   		 }
	  
	  /*gujd : 2012-06-29, am 10:06 . Add code for rtmd add ip , then go to tell kernel .
	    When rtmd listen ip add/del info from kernl , but the info cased by rtmd , rtmd will ingnore these info .
	    Only rtmd deal with the info cased by kernl or other process , not include rtmd yoursel. */
	  ret = if_set_prefix (ifp, ifc);
      if (ret < 0)
		{
		  vty_out (vty, "%% Can't set interface IP address: %s.%s", 
			   safe_strerror(errno), VTY_NEWLINE);		
		  return CMD_WARNING;
		}
	  if(ifp->ifindex > 0)
	  {
	      /* Add to subnet chain list (while marking secondary attribute). */
	      if_subnet_add (ifp, ifc);

	      /* IP address propery set. */
	      SET_FLAG (ifc->conf, ZEBRA_IFC_REAL);

	      /* Update interface address information to protocol daemon. */
	      zebra_interface_address_add_update (ifp, ifc);

	      /* If interface is up register connected route. */
		  /*if (if_is_operative(ifp))*/
		  /*gujd: 2012-12-13, pm 3:56. For wlan or ebr rpa interface. Before register rpa interface, wlan or ebr interface is not active,
		  so when get netlink info of rpa registing form kernel, wlan and ebr updating state ,status, index, and connect route.
		  If here not active to creat connect route will case when deleting ip adress, connect route not delete ( with rib->refcnt++, cannot delete and free).
		  So if interface not active here , not to create connect route.*/
	      if ((if_is_operative(ifp))&&(CHECK_FLAG (ifp->status, ZEBRA_INTERFACE_ACTIVE)))
			connected_up_ipv4 (ifp, ifc);
		
			
			return CMD_SUCCESS;
		 }
	 
	 }
   	}


if(0)
	{
	  intfdown = 1;
	  intf_is_run = if_is_running(ifp);
	  if_set_flags (ifp, IFF_UP | IFF_RUNNING);
	  if_refresh (ifp);
	}
      ret = if_set_prefix (ifp, ifc);
      if (ret < 0)
	{
	  vty_out (vty, "%% Can't set interface IP address: %s.%s", 
		   safe_strerror(errno), VTY_NEWLINE);
 
/*added  by scx */
	  
	if (intfdown)
	  {
		if_unset_flags (ifp, IFF_UP | ((~intf_is_run)&IFF_RUNNING));
		if_refresh (ifp);
	  }
	  return CMD_WARNING;
	}
	if(product == NULL)/*Not Distibute System.*/
	{
      /* Add to subnet chain list (while marking secondary attribute). */
      if_subnet_add (ifp, ifc);

      /* IP address propery set. */
      SET_FLAG (ifc->conf, ZEBRA_IFC_REAL);

      /* Update interface address information to protocol daemon. */
      zebra_interface_address_add_update (ifp, ifc);

      /* If interface is up register connected route. */
      if (if_is_operative(ifp))
		connected_up_ipv4 (ifp, ifc);
	}

/*added  by scx */
	if (intfdown)
	{
		if_unset_flags (ifp, IFF_UP | ((~intf_is_run)&IFF_RUNNING));
		if_refresh (ifp);
	}
    }

  return CMD_SUCCESS;
}

/* withdraw a connected address */
static void
interface_local_del_ip_address(struct connected *ifc)
{
  if (! ifc)
    return;

  /* Update interface address information to protocol daemon. */
 /* if (CHECK_FLAG (ifc->conf, ZEBRA_IFC_REAL))*/
    {
      zebra_interface_address_delete_update (ifc->ifp, ifc);

      if_subnet_delete (ifc->ifp, ifc);
      
      if (ifc->address->family == AF_INET)
        connected_down_ipv4 (ifc->ifp, ifc);
#ifdef HAVE_IPV6
      else
        connected_down_ipv6 (ifc->ifp, ifc);
#endif

      UNSET_FLAG (ifc->conf, ZEBRA_IFC_REAL);
    }

/*  if (!CHECK_FLAG (ifc->conf, ZEBRA_IFC_CONFIGURED))*/
    {
	   router_id_del_address(ifc);/*add*/
      listnode_delete (ifc->ifp->connected, ifc);
      connected_free (ifc);
    }
}


static int
ip_address_uninstall (struct vty *vty, struct interface *ifp,
		      const char *addr_str, const char *peer_str,
		      const char *label)
{
  struct prefix_ipv4 cp;
  struct connected *ifc;
  int ret;
  
  /* Convert to prefix structure. */
  ret = str2prefix_ipv4 (addr_str, &cp);
  if (ret <= 0)
    {
      vty_out (vty, "%% Malformed address %s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  /* Check current interface address. */
  ifc = connected_check (ifp, (struct prefix *) &cp);
  if (! ifc)
    {
      vty_out (vty, "%% Can't find address%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

 /*gjd: 2011-12-17,delete it for Rtsuit restart .(if the Rtsuit restart , the ip address is not configred, we cant't delete. )*/
 /* This is not configured address. */
 /* if (! CHECK_FLAG (ifc->conf, ZEBRA_IFC_CONFIGURED))
         return CMD_WARNING;*/
         
  if(CHECK_FLAG(ifc->conf, ZEBRA_IFC_CONFIGURED))
    UNSET_FLAG(ifc->conf, ZEBRA_IFC_CONFIGURED);
  if(CHECK_FLAG(ifp->if_scope, INTERFACE_LOCAL)&&product)
  	{
		int slot_num = 0;
		slot_num = get_slot_num(ifp->name);
		if(slot_num != product->board_id)
		{
		  interface_local_del_ip_address(ifc);
		  return CMD_SUCCESS;
		}
  	}
  
  /* This is not real address or interface is not active. */
  if (! CHECK_FLAG (ifc->conf, ZEBRA_IFC_REAL)
      || ! CHECK_FLAG (ifp->status, ZEBRA_INTERFACE_ACTIVE))
    {
		router_id_del_address(ifc);
      listnode_delete (ifp->connected, ifc);
      connected_free (ifc);
      return CMD_WARNING;
    }

  /* This is real route. */
  ret = if_unset_prefix (ifp, ifc);
  if (ret < 0)
    {
      /*deal with delete secondary ip failed.*/
	  if(route_boot_errno == -EADDRNOTAVAIL)/*EADDRNOTAVAIL: 126, Cannot assign requested address */
	  {
	  	zlog_info("%s: line %d , delete secondary IP(v4) address.\n",__func__,__LINE__);/*go on to delete as usual.*/
	  }
	  else
	  {
      	vty_out (vty, "%% Can't unset interface IP address: %s.%s", safe_strerror(errno), VTY_NEWLINE);
      	return CMD_WARNING;
	   }
    }
  
#if 1
	if(product != NULL)
	{
		if((judge_eth_sub_interface(ifp->name)== ETH_SUB_INTERFACE) 
			||(judge_ve_sub_interface(ifp->name) == VE_SUB_INTERFACE)
			||(judge_eth_interface(ifp->name)==ETH_INTERFACE))/*ve sub interface and eth sub interface deal as normal*/
			{
				/*	return CMD_SUCCESS;*/
				goto skip;
			}
		
	   ret = ip_address_count(ifp);
	   if(ret < 1 )
	   {
	   	  SET_FLAG(ifp->ip_flags,IPV4_ADDR_DEL);
		  if(ifp->if_types == REAL_INTERFACE )
			{
			 if(strncmp(ifp->name, "ve",2) != 0)
			 {
			  unregister_rpa_interface_table(ifp);
	/*	  ifp->if_types = 0;*/ /**置回0**/
			 }
			}
		  if(ifp->if_types == RPA_INTERFACE)
			{
			  
			  delete_rpa_interface(ifp);
	//		   ifp->if_types = VIRTUAL_INTERFACE; /*赋值让内核处理完后，zebra监听到内核消息再处理*/
			}
	   }
	}
#endif 

/*gujd : 2012-06-29, am 10:06 . Add code for rtmd add ip , then go to tell kernel .
  When rtmd listen ip add/del info from kernl , but the info cased by rtmd , rtmd will ingnore these info .
  Only rtmd deal with the info cased by kernl or other process , not include rtmd yoursel. */

 	
skip:

#if 1

  /* Redistribute this information. */
  zebra_interface_address_delete_update (ifp, ifc);
  
  if_subnet_delete(ifc->ifp, ifc);

  /* Remove connected route. */
  connected_down_ipv4 (ifp, ifc);
  router_id_del_address(ifc);/*add*/

  /* Free address information. */
  listnode_delete (ifp->connected, ifc);
  connected_free (ifc);
#endif

  return CMD_SUCCESS;
}

DEFUN (ip_address,
       ip_address_cmd,
       "ip address A.B.C.D/M",
       "Interface Internet Protocol config commands\n"
       "Set the IP address of an interface\n"
       "IP address (e.g. 10.0.0.1/8)\n")
{
  struct interface *ifp = if_get_by_vty_index(vty->index);
	if(NULL == ifp)
	{
		vty_out (vty, "%% Interface %s does not exist%s", (char*)(vty->index), VTY_NEWLINE);
		return CMD_WARNING;
	}

  return ip_address_install (vty, ifp, argv[0], NULL, NULL);
}

DEFUN (no_ip_address,
       no_ip_address_cmd,
       "no ip address A.B.C.D/M",
       NO_STR
       "Interface Internet Protocol config commands\n"
       "Set the IP address of an interface\n"
       "IP Address (e.g. 10.0.0.1/8)")
{
	struct interface *ifp = if_get_by_vty_index(vty->index);

	if(NULL == ifp)
	{
		vty_out (vty, "%% Interface %s does not exist%s", (char*)(vty->index), VTY_NEWLINE);
		return CMD_WARNING;
	}

  return ip_address_uninstall (vty, ifp, argv[0], NULL, NULL);
}

#ifdef HAVE_NETLINK
DEFUN (ip_address_label,
       ip_address_label_cmd,
       "ip address A.B.C.D/M label LINE",
       "Interface Internet Protocol config commands\n"
       "Set the IP address of an interface\n"
       "IP address (e.g. 10.0.0.1/8)\n"
       "Label of this address\n"
       "Label\n")
{
	struct interface *ifp = if_get_by_vty_index(vty->index);
	if(NULL == ifp)
	{
		vty_out (vty, "%% Interface %s does not exist%s", (char*)(vty->index), VTY_NEWLINE);
		return CMD_WARNING;
	}

  return ip_address_install (vty, ifp, argv[0], NULL, argv[1]);
}

DEFUN (no_ip_address_label,
       no_ip_address_label_cmd,
       "no ip address A.B.C.D/M label LINE",
       NO_STR
       "Interface Internet Protocol config commands\n"
       "Set the IP address of an interface\n"
       "IP address (e.g. 10.0.0.1/8)\n"
       "Label of this address\n"
       "Label\n")
{
	struct interface *ifp = if_get_by_vty_index(vty->index);
	if(NULL == ifp)
	{
		vty_out (vty, "%% Interface %s does not exist%s", (char*)(vty->index), VTY_NEWLINE);
		return CMD_WARNING;
	}

  return ip_address_uninstall (vty, ifp, argv[0], NULL, argv[1]);
}
#endif /* HAVE_NETLINK */

#ifdef HAVE_IPV6
static int
ipv6_address_install (struct vty *vty, struct interface *ifp,
		      const char *addr_str, const char *peer_str,
		      const char *label, int secondary)
{
  struct prefix_ipv6 cp;
  struct connected *ifc;
  struct prefix_ipv6 *p;
  struct listnode *node;
  struct listnode *next;
  struct connected *ifc2;
  int count = 0;
  int ret;
  int ifc_new = 0;

  ret = str2prefix_ipv6 (addr_str, &cp);
  if (ret <= 0)
    {
      vty_out (vty, "%% Malformed address %s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  
    if (ifp->connected)
    {
      for (ALL_LIST_ELEMENTS (ifp->connected, node, next, ifc2))
		{
		  	if(CHECK_FLAG (ifc2->conf, ZEBRA_IFC_CONFIGURED))
				count ++;
		}
    }
	else
	{
		vty_out (vty, "%% ifp->connected is NULL %s", VTY_NEWLINE);
		return CMD_WARNING;

	}
	
	if(IFC_MAX <= count)
	{
		vty_out(vty,"%%Warning :There already has %d address on %s.%s",IFC_MAX,ifp->name,VTY_NEWLINE);
		return CMD_WARNING;
	}

#if 0
  if(IFC_MAX <= ifp->connected->count){
		vty_out(vty,"%%[][][][][][] There already has %d address on %s.%s",IFC_MAX,
		ifp->name,
			VTY_NEWLINE);
	return CMD_WARNING;
  }
#endif
  ifc = connected_check (ifp, (struct prefix *) &cp);
  if (! ifc)
    {
      ifc = connected_new ();
      ifc->ifp = ifp;

      /* Address. */
      p = prefix_ipv6_new ();
      *p = cp;
      ifc->address = (struct prefix *) p;
	  ifc_new = 1;
	  
	  /**gjd: add for check ipv6 address like prefix is fe80:: , for Distribute System .**/
#if 1
	  if(IPV6_ADDR_FE80(ifc->address->u.prefix6.in6_u.u6_addr16[0]))
	  {
	  	vty_out(vty,"You can't set ipv6 address like : fe80::xxx/x !\n");
		/*CID 13530 (#1 of 1): Resource leak (RESOURCE_LEAK)
		19. leaked_storage: Variable "ifc" going out of scope leaks the storage it points to.
		Free ifc . So add it .*/
		if(ifc_new)
		 connected_free (ifc);/*add*/
		
		return CMD_SUCCESS;
	  	}
#endif
		/**2011-09-16: pm 5:45.**/

      /* Secondary. */
      if (secondary)
	SET_FLAG (ifc->flags, ZEBRA_IFA_SECONDARY);

      /* Label. */
      if (label)
	ifc->label = XSTRDUP (MTYPE_CONNECTED_LABEL, label);

      /* Add to linked list. */
      listnode_add (ifp->connected, ifc);
    }

  /* This address is configured from zebra. */
  if (! CHECK_FLAG (ifc->conf, ZEBRA_IFC_CONFIGURED))
    SET_FLAG (ifc->conf, ZEBRA_IFC_CONFIGURED);

  /* In case of this route need to install kernel. */
  if (! CHECK_FLAG (ifc->conf, ZEBRA_IFC_REAL)
      && (CHECK_FLAG (ifp->status, ZEBRA_INTERFACE_ACTIVE)||product != NULL))
    {
      /* Some system need to up the interface to set IP address. */
	  /*
			if (! if_is_up (ifp))
			{
			  if_set_flags (ifp, IFF_UP | IFF_RUNNING);
			  if_refresh (ifp);
			}
		*/
		if((judge_eth_sub_interface(ifp->name)==ETH_SUB_INTERFACE)
				||(judge_ve_sub_interface(ifp->name)==VE_SUB_INTERFACE)
				||(judge_eth_interface(ifp->name)==ETH_INTERFACE)
				||(strncmp(ifp->name,"mng",3)== 0))
	  {
		{
	      ret = if_prefix_add_ipv6 (ifp, ifc);

      if (ret < 0)
			{
			  vty_out (vty, "%% Can't set interface IP address: %s.%s", 
				   safe_strerror(errno), VTY_NEWLINE);
			  return CMD_WARNING;
			}
		 }

      /* IP address propery set. */
      SET_FLAG (ifc->conf, ZEBRA_IFC_REAL);

      /* Update interface address information to protocol daemon. */
      zebra_interface_address_add_update (ifp, ifc);

      /* If interface is up register connected route. */
      if (if_is_operative(ifp))
		connected_up_ipv6 (ifp, ifc);
	  
	  return CMD_SUCCESS;
	  
	}
		
	if(product != NULL)/*rpa interface*/
	 {
	   if(strncmp(ifp->name,"ve",2)!=0 && strncmp(ifp->name,"eth",3)!=0)
	   	{
	   	  int count_ip = 0;
		 /* count_ip = ip_address_count(ifp);*/
		 
		 count_ip = ip_address_count_except_fe80(ifp);
		
		 if((ifp->if_types == REAL_INTERFACE) && count_ip <= 1)
		   {
			   ifp->slot = get_slot_num(ifp->name);
			   zlog_debug("get slot num %d ....\n",ifp->slot);
			   if(strncmp(ifp->name, "ve",2) != 0)
			   {
			   	ret = register_rpa_interface_table(ifp);
				if(ret < 0)
				   {
					   zlog_warn("register rpa interface table in local board failed : %s ..\n",safe_strerror(errno));
				   }
			   }
		   }
		  if((ifp->if_types == VIRTUAL_INTERFACE) && strncmp(ifp->name,"ve",2)!=0)
		   {   
			   /*create_rpa_interface();*/
			   
			   #if 1
			   ret = create_rpa_interface(ifp);
				if(ret < 0)
				 {
				   zlog_debug("%s : line %d, creat rpa interface failed : %s \n ",__func__,__LINE__,safe_strerror(errno));
		   /*	   return NULL;*/
				 }
				else
				 { 
				   /*virtual interface ==> RPA interface*/
				   ifp->if_types = RPA_INTERFACE;
				   zlog_debug("%s : line %d, creat rpa interface sucess, so virtual turn to rpa interface...\n",__func__,__LINE__);
				 }

				sleep(1);
				
	/*			ifp->ifindex = if_get_if_index_by_ioctl(ifp);*/
				ifp->ifindex = if_get_index_by_ioctl(ifp);

				if(ifp->ifindex <= 0)
				 {
				   zlog_warn("get rpa ifindex by ioctl failed : %s ..\n",safe_strerror(errno));
				   return CMD_WARNING;
				 }
				zlog_debug("get rpa ifindex by ioctl sucess is %d .....",ifp->ifindex);
				/*gujd : 2012-08-01, Add for ospf , rip update the interface index when create rpa .*/
				zebra_interface_add_update(ifp);
				
		#endif
	   		}
		 
		 }
	   	}

	
	      ret = if_prefix_add_ipv6 (ifp, ifc);

	      if (ret < 0)
			{
			  vty_out (vty, "%% Can't set interface IP address: %s.%s", 
				   safe_strerror(errno), VTY_NEWLINE);
			  return CMD_WARNING;
			}
		  zlog_debug("%s : line %d ....\n",__func__,__LINE__);
		  /* IP address propery set. */
      SET_FLAG (ifc->conf, ZEBRA_IFC_REAL);

      /* Update interface address information to protocol daemon. */
      zebra_interface_address_add_update (ifp, ifc);

      /* If interface is up register connected route. */
      if (if_is_operative(ifp))
	connected_up_ipv6 (ifp, ifc);
    }

  return CMD_SUCCESS;
}

static int
ipv6_address_uninstall (struct vty *vty, struct interface *ifp,
			const char *addr_str, const char *peer_str,
			const char *label, int secondry)
{
  struct prefix_ipv6 cp;
  struct connected *ifc;
  int ret;

  /* Convert to prefix structure. */
  ret = str2prefix_ipv6 (addr_str, &cp);
  if (ret <= 0)
    {
      vty_out (vty, "%% Malformed address %s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  /* Check current interface address. */
  ifc = connected_check (ifp, (struct prefix *) &cp);
  if (! ifc)
    {
      vty_out (vty, "%% Can't find address%s", VTY_NEWLINE);
      return CMD_WARNING;
    }
  
  /*gjd: 2011-12-17,delete it for Rtsuit restart .(if the Rtsuit restart , the ip address is not configred, we cant't delete. )*/
  /* This is not configured address. */
  /* if (! CHECK_FLAG (ifc->conf, ZEBRA_IFC_CONFIGURED))
         return CMD_WARNING;*/

  /* This is not real address or interface is not active. */
  if (! CHECK_FLAG (ifc->conf, ZEBRA_IFC_REAL)
      || ! CHECK_FLAG (ifp->status, ZEBRA_INTERFACE_ACTIVE))
    {
		router_id_del_address(ifc);
      listnode_delete (ifp->connected, ifc);
      connected_free (ifc);
      return CMD_WARNING;
    }

  /* This is real route. */
  ret = if_prefix_delete_ipv6 (ifp, ifc);
  #if 1
  if (ret < 0)
    {
      vty_out (vty, "%% Can't unset interface IP address: %s.%s", 
	       safe_strerror(errno), VTY_NEWLINE);
      return CMD_WARNING;
    }
  #else
  if (ret < 0)
    {
	  if(route_boot_errno == -EADDRNOTAVAIL)/*EADDRNOTAVAIL: 126, Cannot assign requested address */
	  {
	  	zlog_info("%s: line %d , delete secondary IP(v6) address.\n",__func__,__LINE__);
	  }
	  else
	  {
      	vty_out (vty, "%% Can't unset interface IP address: %s.%s", safe_strerror(errno), VTY_NEWLINE);
      	return CMD_WARNING;
	   }
    }
   #endif
  /* Redistribute this information. */
  zebra_interface_address_delete_update (ifp, ifc);

  /* Remove connected route. */
  connected_down_ipv6 (ifp, ifc);

  /* Free address information. */
  listnode_delete (ifp->connected, ifc);
  connected_free (ifc);
  
#if 1
		if(product != NULL)
		{
			if((judge_eth_sub_interface(ifp->name)== ETH_SUB_INTERFACE) 
				||(judge_ve_sub_interface(ifp->name) == VE_SUB_INTERFACE)
				||(judge_eth_interface(ifp->name)==ETH_INTERFACE))/*ve sub interface and eth sub interface deal as normal*/
				
			return CMD_SUCCESS;
			
		   ret = ip_address_count(ifp);
		   if(ret < 1 )
		   {
			  SET_FLAG(ifp->ip_flags,IPV4_ADDR_DEL);
			  if(ifp->if_types == REAL_INTERFACE )
				{
				 if(strncmp(ifp->name, "ve",2) != 0)
				 {
				  unregister_rpa_interface_table(ifp);
		/*	  ifp->if_types = 0;*/ /**置回0**/
				 }
				}
			  if(ifp->if_types == RPA_INTERFACE)
				{
				  
				  delete_rpa_interface(ifp);
		//		   ifp->if_types = VIRTUAL_INTERFACE; /*赋值让内核处理完后，zebra监听到内核消息再处理*/
				}
		   }
		}
#endif 

  return CMD_SUCCESS;
}

DEFUN (ipv6_address,
       ipv6_address_cmd,
       "ipv6 address X:X::X:X/M",
       "Interface IPv6 config commands\n"
       "Set the IP address of an interface\n"
       "IPv6 address (e.g. 3ffe:506::1/48)\n")
{
	struct interface *ifp = if_get_by_vty_index(vty->index);
	if(NULL == ifp)
	{
		vty_out (vty, "%% Interface %s does not exist%s", (char*)(vty->index), VTY_NEWLINE);
		return CMD_WARNING;
	}

  return ipv6_address_install (vty, ifp, argv[0], NULL, NULL, 0);
}

DEFUN (no_ipv6_address,
       no_ipv6_address_cmd,
       "no ipv6 address X:X::X:X/M",
       NO_STR
       "Interface IPv6 config commands\n"
       "Set the IP address of an interface\n"
       "IPv6 address (e.g. 3ffe:506::1/48)\n")
{
	struct interface *ifp = if_get_by_vty_index(vty->index);
	if(NULL == ifp)
	{
		vty_out (vty, "%% Interface %s does not exist%s", (char*)(vty->index), VTY_NEWLINE);
		return CMD_WARNING;
	}

  return ipv6_address_uninstall (vty, ifp, argv[0], NULL, NULL, 0);
}
#endif /* HAVE_IPV6 */

/**************************************
* interface qinq type save
*
*input Params: 
*	 vty --       
*       name:interface name
*output params: none
*
*return:CMD_FAILURE --  1:parse interface name error
*					       2:not a eth or ve sub interface
*						3:create socket failed
*						4:subint do not exists
*						5:get qinq type from kernel failed
*						6:is a subintf but do not set qinq type
*						7:is not a subintf
*		CMD_SUCCESS -- save subintf qinq type successfully
*
****************************************/
static int intf_qinq_type_save(struct vty *vty,char *name)
{

	int slot = 0,port = 0,vid1 = 0,vid2 = 0;
	unsigned char cpu_no = 0,cpu_port_no = 0;
	int fd = CMD_FAILURE;
	struct vlan_ioctl_args if_request;
	unsigned short qinqType = 0;
	unsigned int if_index = 0;
	int ret = CMD_FAILURE;

	memset(if_request.device1,0,24);
	if(memcmp(name,"eth",3) == 0)
	{
		ret = parse_slotport_tag_no(name,&slot,&port,&vid1,&vid2);
		if(ret != CMD_SUCCESS)
		{
			zlog_debug("parse %s Failure \n",name);
			return CMD_FAILURE;
		}
	}
	else if(memcmp(name,"ve",2) == 0)
	{
		ret = parse_ve_slot_cpu_tag_no(name,&cpu_no,&cpu_port_no,&slot,&vid1,&vid2);
		if(ret != CMD_SUCCESS)
		{
			zlog_debug("parse %s Failure \n",name);
			return CMD_FAILURE;
		}
	}
	else
	{
		return CMD_FAILURE;		/*if not eth port or ve port */
	}
	
	if((vid1) && (!vid2))	/*if is subintf  such as eth1-1.100*/
	{
		if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
		{
			zlog_debug("Can't open socket to get qinq type !\n");
			return CMD_FAILURE;
		}
		if_request.cmd = GET_VLAN_VID_CMD + 2;	/*get qinq type cmd */
		
		if_index = if_nametoindex(name);	/*get index from kernel*/
		if(if_index) 	/*if  subintf exists*/
		{
	  		sprintf(if_request.device1,"%s",name);
	    	if(ioctl(fd, SIOCGIFVLAN, &if_request)<0)	/*get qinq type*/
	    	{
				zlog_debug("get %s qinq type Failed !\n",name);
				close(fd);
				return CMD_FAILURE;
	    	}
	    	qinqType = (unsigned short)(if_request.u.flag);
	    	if((qinqType) && (qinqType != 0x8100))
			{
				vty_out (vty, "interface %s%s config qinq-type %#x%s",name,VTY_NEWLINE,qinqType,VTY_NEWLINE);
	    	}
			else
			{
				ret = CMD_FAILURE;		/*if do not set qinq type of qinq type is default*/
			}
		}
		else
		{
			ret = CMD_FAILURE;		/*subintf do not exists*/
		}
		/*fixed bug AXSSZFI-961*/
		close(fd);
	}
	else
	{
		return CMD_FAILURE;		/*is not subintf interface*/
	}
	return ret;
}

static int if_pfm_config_write(struct vty *vty, char *ifname)
{
	FILE *fp;
	char buf[1024];
	char cmdstr[256] = {0};
	pid_t status;
	char *name, *temp, *rulestr;

	sprintf(cmdstr, "grep '%s' /var/run/pfm.conf", ifname);
	status = system(cmdstr);
	if (0 == WEXITSTATUS(status)) {
		fp = fopen("/var/run/pfm.conf", "r");
		if (fp == NULL)
		{
			return CMD_WARNING;
		}

		while (fgets(buf, 1024, fp) != NULL)
		{
			name = strtok(buf, ":");
			//vty_out(vty, "name : %s\n", name);
			//vty_out(vty, "ifname : %s\n", ifname);
			if (!strcmp(name, ifname)) {
				temp = strtok(NULL, ":");
				//vty_out(vty, "temp : %s\n", temp);
				rulestr = strtok(NULL, ":");
				vty_out(vty, " %s%s", rulestr, VTY_NEWLINE);
			}
		}

		fclose(fp);
		
		return CMD_SUCCESS;
	}

	return CMD_SUCCESS;
}


static int
if_config_write (struct vty *vty)
{
  struct listnode *node;
  struct interface *ifp;
  char buf[BUFSIZ];

  for (ALL_LIST_ELEMENTS_RO (iflist, node, ifp))
    {
      struct zebra_if *if_data;
      struct listnode *addrnode;
      struct connected *ifc;
      struct prefix *p;

      if_data = ifp->info;
	  if((memcmp(ifp->name,"wlan",4) == 0)
	  	||(memcmp(ifp->name,"radio",5) == 0)	  	
	  	||(memcmp(ifp->name,"r",1) == 0)
	  	||(memcmp(ifp->name,"ebr",3) == 0)
	  	||(memcmp(ifp->name,"sit0",4) == 0)
	  	||(memcmp(ifp->name,"pimreg",6) == 0)
	    ||(memcmp(ifp->name,"obc0",4) == 0)
	  	||(memcmp(ifp->name,"cnu",3) == 0)
	  	||(memcmp(ifp->name,"ppp",3) == 0)
	  	||(memcmp(ifp->name,"oct",3) == 0)
	  	||(judge_ve_interface(ifp->name)==VE_INTERFACE)){
		continue;
	  }
	  
	  /*zhangcl added for interface qinq type save*/
	  /* Modify for lacp zhangchunlong@autelan.com 2012-12-12 */
	  if(0 != intf_lacp_process(vty,ifp->name))/*if switch board intf enable lacp fuction,do not save intf cmd*/
	  {
		  if(0 != intf_qinq_type_save(vty,ifp->name))	/*if does not set qinq type*/
		  {
	      	vty_out (vty, "interface %s%s", ifp->name,VTY_NEWLINE);
		  }
	  }
	  else
	  {
	  	continue;
	  }

      if (ifp->desc)
      {
        if(CHECK_FLAG(ifp->desc_scope,INTERFACE_DESCIPTION_LOCAL))
          {
	      	vty_out (vty, " local description %s%s", ifp->desc,VTY_NEWLINE);
        	}
		else
			vty_out (vty, " description %s%s", ifp->desc,VTY_NEWLINE);
      	}
	  
	  /*gjd : add for interface local or global of Distribute System .*/
	  if(CHECK_FLAG(ifp->if_scope, INTERFACE_LOCAL))
	  	vty_out(vty," interface local%s",VTY_NEWLINE);
	  /*2011-09-08: am 10:00*/

      /* Assign bandwidth here to avoid unnecessary interface flap
	 while processing config script */
      if (ifp->bandwidth != 1000000)
	vty_out(vty, " bandwidth %u%s", ifp->bandwidth, VTY_NEWLINE); 

      if (CHECK_FLAG(ifp->status, ZEBRA_INTERFACE_LINKDETECTION))
		vty_out(vty, " link-detect%s", VTY_NEWLINE);

      for (ALL_LIST_ELEMENTS_RO (ifp->connected, addrnode, ifc))
	  {
	    if (CHECK_FLAG (ifc->conf, ZEBRA_IFC_CONFIGURED))
	      {
		p = ifc->address;
		vty_out (vty, " ip%s address %s/%d",
			 p->family == AF_INET ? "" : "v6",
			 inet_ntop (p->family, &p->u.prefix, buf, BUFSIZ),
			 p->prefixlen);

		if (ifc->label)
		  vty_out (vty, " label %s", ifc->label);

		vty_out (vty, "%s", VTY_NEWLINE);
	      }
	  }

      if (if_data)
	{
	  if (if_data->shutdown == IF_ZEBRA_SHUTDOWN_ON)
	    vty_out (vty, " shutdown%s", VTY_NEWLINE);

	  if (if_data->multicast != IF_ZEBRA_MULTICAST_UNSPEC)
	    vty_out (vty, " %smulticast%s",
		     if_data->multicast == IF_ZEBRA_MULTICAST_ON ? "" : "no ",
		     VTY_NEWLINE);
	}
	
	if_pfm_config_write(vty, ifp->name);

#ifdef RTADV
      rtadv_config_write (vty, ifp);
#endif /* RTADV */

#ifdef HAVE_IRDP
      irdp_config_write (vty, ifp);
#endif /* IRDP */
	if(CHECK_FLAG(ifp->uplink_flag,INTERFACE_SET_UPLINK))
		vty_out(vty," set uplink%s",VTY_NEWLINE);
	
      vty_out (vty, " exit%s", VTY_NEWLINE);
    }
  return 0;
}


static int
wlan_if_config_write (struct vty *vty, int islocaled, int slotid ,int index, int type)
{
  struct listnode *node;
  struct interface *ifp;
  char buf[BUFSIZ];

  for (ALL_LIST_ELEMENTS_RO (iflist, node, ifp))
    {
      struct zebra_if *if_data;
      struct listnode *addrnode;
      struct connected *ifc;
      struct prefix *p;	  
	  char *endptr = NULL;
	  char name[16];
	  int sid = 0;
	  int iid = 0;
	  int id = 0;
	  int id1 = 0;
	  int id2 = 0;	  
	  int id3 =0;
	  int ret = 0;
      if_data = ifp->info;
      if(islocaled){	  
		  if((type == 0)&&(memcmp(ifp->name,"wlanl",5) == 0)){
			  ret = parse_wlan_ebr_ifname_v2(ifp->name+5,&sid,&iid,&id1);
			  if(ret!=0 || iid != index || sid != slotid)
				  continue;
			  
			  vty_out (vty, "interface wlan%d%s", id1,
				   VTY_NEWLINE);
		  }else if((type == 2)&&(memcmp(ifp->name,"radio",5) == 0)){
			  ret = parse_radio_ifname_v2(ifp->name+5,&iid,&id1,&id2,&id3);
			  if((ret != 0)||(iid != index))
				  continue;
			  
			  vty_out (vty, "interface r%d-%d.%d%s", id1,id2,id3,
				   VTY_NEWLINE);

		  }else if((type == 2)&&(memcmp(ifp->name,"r",1)) == 0){
			  ret = parse_radio_ifname_v2(ifp->name+1,&iid,&id1,&id2,&id3);
			  if((ret != 0)||(iid != index))
				  continue;
			  
			  vty_out (vty, "interface r%d-%d.%d%s", id1,id2,id3,
				   VTY_NEWLINE);

		  }
		  else if((type == 1)&&(memcmp(ifp->name,"ebrl",4)) == 0){
			  ret = parse_wlan_ebr_ifname_v2_H(ifp->name+4,&sid,&iid,&id1);
			  if(ret!=0 || iid != index || sid != slotid)
				  continue;
			  
			  vty_out (vty, "interface ebr%d%s", id1,
				   VTY_NEWLINE);

		  }else if((type == 0)&&(memcmp(ifp->name,"cnu",3)) == 0){
			  id = strtoul(ifp->name+3,&endptr,10);
			  if(id != index)
				  continue;
			  vty_out (vty, "interface cnu%s%s", endptr+1,
				   VTY_NEWLINE);

		  }else
		  		continue;
	  }
	  else{
		  if((type == 0)&&(memcmp(ifp->name,"wlan",4) == 0)&&(memcmp(ifp->name,"wlanl",5) != 0)){
			  ret = parse_wlan_ebr_ifname_v2(ifp->name+4,&sid,&iid,&id1);
			  if(ret!=0 || iid != index || sid != slotid)
				  continue;
			  vty_out (vty, "interface wlan%d%s", id1,
				   VTY_NEWLINE);
		  }else if((type == 2)&&(memcmp(ifp->name,"radio",5) == 0)){
			  ret = parse_radio_ifname_v3(ifp->name+5,&sid,&iid,&id1,&id2,&id3);
			  if((ret != 0)||(iid != index) || sid != slotid)
				  continue;
			  
			  vty_out (vty, "interface r%d-%d.%d%s", id1,id2,id3,
				   VTY_NEWLINE);

		  }else if((type == 2)&&(memcmp(ifp->name,"r",1)) == 0){
			  ret = parse_radio_ifname_v3(ifp->name+1,&sid,&iid,&id1,&id2,&id3);
			  if((ret != 0)||(iid != index) || sid != slotid)
				  continue;
			  
			  vty_out (vty, "interface r%d-%d.%d%s", id1,id2,id3,
				   VTY_NEWLINE);

		  }
		  else if((type == 1)&&(memcmp(ifp->name,"ebr",3)== 0)&&(memcmp(ifp->name,"ebrl",4)!= 0)){
		  		ret = parse_wlan_ebr_ifname_v2_H(ifp->name + 3, &sid, &iid, &id1);
			  if(ret!=0 || iid != index || sid != slotid)
				  continue;
			  
			  vty_out (vty, "interface ebr%d%s", id1,
				   VTY_NEWLINE);

		  }else if((type == 0)&&(memcmp(ifp->name,"cnu",3)) == 0){
			  id = strtoul(ifp->name+3,&endptr,10);
			  if(id != index)
				  continue;
			  vty_out (vty, "interface cnu%s%s", endptr+1,
				   VTY_NEWLINE);

		  }else
		  		continue;
	  }
      /*vty_out (vty, "interface %s%s", ifp->name,
	       VTY_NEWLINE);*/

      if (ifp->desc)
	  {
		if(CHECK_FLAG(ifp->desc_scope,INTERFACE_DESCIPTION_LOCAL))
		{
			vty_out (vty, " local description  %s%s", ifp->desc,VTY_NEWLINE);
			}
		else
			vty_out (vty, " description  %s%s", ifp->desc,VTY_NEWLINE);
		}
	  /*gjd : add for interface local or global of Distribute System .*/
	  if(CHECK_FLAG(ifp->if_scope, INTERFACE_LOCAL))
	  	vty_out(vty," interface local%s",VTY_NEWLINE);
	  /*2012-05-03: pm 2:36*/

      /* Assign bandwidth here to avoid unnecessary interface flap
	 while processing config script */
      if (ifp->bandwidth != 1000000)
	vty_out(vty, " bandwidth %u%s", ifp->bandwidth, VTY_NEWLINE); 

      if (CHECK_FLAG(ifp->status, ZEBRA_INTERFACE_LINKDETECTION))
	vty_out(vty, " link-detect%s", VTY_NEWLINE);

      for (ALL_LIST_ELEMENTS_RO (ifp->connected, addrnode, ifc))
	  {
	    if (CHECK_FLAG (ifc->conf, ZEBRA_IFC_CONFIGURED))
	      {
		p = ifc->address;
		vty_out (vty, " ip%s address %s/%d",
			 p->family == AF_INET ? "" : "v6",
			 inet_ntop (p->family, &p->u.prefix, buf, BUFSIZ),
			 p->prefixlen);

		if (ifc->label)
		  vty_out (vty, " label %s", ifc->label);

		vty_out (vty, "%s", VTY_NEWLINE);
	      }
	  }

      if (if_data)
	{
	  if (if_data->shutdown == IF_ZEBRA_SHUTDOWN_ON)
	    vty_out (vty, " shutdown%s", VTY_NEWLINE);

	  if (if_data->multicast != IF_ZEBRA_MULTICAST_UNSPEC)
	    vty_out (vty, " %smulticast%s",
		     if_data->multicast == IF_ZEBRA_MULTICAST_ON ? "" : "no ",
		     VTY_NEWLINE);
	}
	  
    if_pfm_config_write(vty, ifp->name);
	
#ifdef RTADV
      rtadv_config_write (vty, ifp);
#endif /* RTADV */

#ifdef HAVE_IRDP
      irdp_config_write (vty, ifp);
#endif /* IRDP */
      vty_out (vty, " exit%s", VTY_NEWLINE);

    }
  return 0;
}

/*gjd : add for Rtsuit restart .*/
int
if_config_write_wlan_ebr_radio (struct vty *vty)
{
  struct listnode *node = NULL;
  struct interface *ifp = NULL;
  char buf[BUFSIZ] = {0};

  for (ALL_LIST_ELEMENTS_RO (iflist, node, ifp))
    {
      struct zebra_if *if_data;
      struct listnode *addrnode;
      struct connected *ifc;
      struct prefix *p;

      if_data = ifp->info;
	  #if 0
	  if((memcmp(ifp->name,"wlan",4) == 0)
	  	||(memcmp(ifp->name,"radio",5) == 0)
	  	||(memcmp(ifp->name,"r",1) == 0)
	  	||(memcmp(ifp->name,"ebr",3) == 0)
	  	||(memcmp(ifp->name,"sit0",4) == 0)
	  	||(memcmp(ifp->name,"pimreg",6) == 0)
	  	||(memcmp(ifp->name,"cnu",3) == 0)){
		continue;
	  }
	  #else
	  
	  if((memcmp(ifp->name,"eth",3) == 0)
	  	||(memcmp(ifp->name,"vlan",4) == 0)
	  	||(memcmp(ifp->name,"ve",2) == 0)
	  	||(memcmp(ifp->name,"obc",3) == 0)
	  	/*2013-05-09, pm 5:47. Delete radio , use wifi command of 'set wlan forwarding mode tunnel'.*/
		||(memcmp(ifp->name,"radio",5) == 0)	  
		||(memcmp(ifp->name,"r",1) == 0)
	  	||(memcmp(ifp->name,"sit0",4) == 0)
	  	||(memcmp(ifp->name,"pimreg",6) == 0)
	  	||(memcmp(ifp->name,"cnu",3) == 0)
		||(memcmp(ifp->name,"lo",2) == 0)
	  	||(memcmp(ifp->name,"mng",3) == 0)
	  	||(memcmp(ifp->name,"ppp",3) == 0)
		||(memcmp(ifp->name,"oct",3) == 0)
	  	)
	  {
		continue;
	  }
	  
	  #endif
      vty_out (vty, "interface %s%s", ifp->name,
	       VTY_NEWLINE);

      if (ifp->desc)
	  {
		if(CHECK_FLAG(ifp->desc_scope,INTERFACE_DESCIPTION_LOCAL))
		{
			vty_out (vty, " local description %s%s", ifp->desc,VTY_NEWLINE);
			}
		else
			vty_out (vty, " description %s%s", ifp->desc,VTY_NEWLINE);
		}
	  
	  /*gjd : add for interface local or global of Distribute System .*/
	  if(CHECK_FLAG(ifp->if_scope, INTERFACE_LOCAL))
	  	vty_out(vty," interface local%s",VTY_NEWLINE);
	  /*2012-05-03: pm 2:36*/

      /* Assign bandwidth here to avoid unnecessary interface flap
	 while processing config script */
/*	 
      if (ifp->bandwidth != 0)
*/
	if (ifp->bandwidth != 1000000)
		vty_out(vty, " bandwidth %u%s", ifp->bandwidth, VTY_NEWLINE); 

      if (CHECK_FLAG(ifp->status, ZEBRA_INTERFACE_LINKDETECTION))
		vty_out(vty, " link-detect%s", VTY_NEWLINE);

      for (ALL_LIST_ELEMENTS_RO (ifp->connected, addrnode, ifc))
	  {
	    if (CHECK_FLAG (ifc->conf, ZEBRA_IFC_CONFIGURED))
	      {
		p = ifc->address;
		vty_out (vty, " ip%s address %s/%d",
			 p->family == AF_INET ? "" : "v6",
			 inet_ntop (p->family, &p->u.prefix, buf, BUFSIZ),
			 p->prefixlen);

		if (ifc->label)
		  vty_out (vty, " label %s", ifc->label);

		vty_out (vty, "%s", VTY_NEWLINE);
	      }
	  }

      if (if_data)
	{
	  if (if_data->shutdown == IF_ZEBRA_SHUTDOWN_ON)
	    vty_out (vty, " shutdown%s", VTY_NEWLINE);

	  if (if_data->multicast != IF_ZEBRA_MULTICAST_UNSPEC)
	    vty_out (vty, " %smulticast%s",
		     if_data->multicast == IF_ZEBRA_MULTICAST_ON ? "" : "no ",
		     VTY_NEWLINE);
	}
#if 1
#ifdef RTADV
      rtadv_config_write (vty, ifp);
#endif /* RTADV */

#ifdef HAVE_IRDP
      irdp_config_write (vty, ifp);
#endif /* IRDP */
#endif
      vty_out (vty, " exit%s", VTY_NEWLINE);
    }
  return 0;
}

/*2012-01-11 : pm 5:36*/


DEFUN (show_wireless_if_config,
       show_wireless_if_config_cmd,
       "show interface (wireless_config|ebr_config) (local|remote) SLOTID ID",
       NO_STR)
{
  int index = atoi(argv[3]);
  int slotid = atoi(argv[2]); 
  int type = 0;
  int islocaled = 1;
  if(strcmp(argv[0],"ebr_config") == 0){
	type = 1;
  }  
  if(strcmp(argv[1],"remote") == 0){
	islocaled = 0;
  }
  return wlan_if_config_write (vty, islocaled, slotid,index, type);
}


/* Allocate and initialize interface vector. */
void
zebra_if_init (void)
{
  /* Initialize interface and new hook. */
  if_init ();
  if_add_hook (IF_NEW_HOOK, if_zebra_new_hook);
  if_add_hook (IF_DELETE_HOOK, if_zebra_delete_hook);
  
  /* Install configuration write function. */
  install_node (&interface_node, if_config_write, "INTERFACE_NODE");
  install_node(&wireless_interface_node ,if_config_write_wlan_ebr_radio,"INTERFACE_NODE");

  install_element (VIEW_NODE, &show_interface_cmd);
  install_element (ENABLE_NODE, &show_interface_cmd);
  install_element (CONFIG_NODE, &show_interface_cmd);
  install_element (INTERFACE_NODE, &show_interface_cmd);
  install_element (ENABLE_NODE, &show_interface_desc_cmd);
  install_element (VIEW_NODE, &show_interface_desc_cmd);
  install_element (CONFIG_NODE, &zebra_interface_cmd);
  install_element (CONFIG_NODE, &no_interface_cmd);
  install_default (INTERFACE_NODE);
  install_element (INTERFACE_NODE, &interface_desc_cmd_rtmd);
  install_element (INTERFACE_NODE, &interface_desc_local_cmd_rtmd);
  install_element (INTERFACE_NODE, &no_interface_desc_cmd_rtmd);
  install_element (INTERFACE_NODE, &multicast_cmd);
  install_element (INTERFACE_NODE, &no_multicast_cmd);
  install_element (INTERFACE_NODE, &linkdetect_cmd);
  install_element (INTERFACE_NODE, &no_linkdetect_cmd);
  install_element (INTERFACE_NODE, &shutdown_if_cmd);
  install_element (INTERFACE_NODE, &no_shutdown_if_cmd);
  install_element (INTERFACE_NODE, &bandwidth_if_cmd);
  install_element (INTERFACE_NODE, &no_bandwidth_if_cmd);
  install_element (INTERFACE_NODE, &no_bandwidth_if_val_cmd);
  install_element (INTERFACE_NODE, &ip_address_cmd);
  install_element (INTERFACE_NODE, &no_ip_address_cmd);
  install_element (ENABLE_NODE, &show_wireless_if_config_cmd);
  install_element (VIEW_NODE, &show_wireless_if_config_cmd);
  /*radio interface*/
  install_element (CONFIG_NODE, &radio_interface_deal_cmd);
  /*show radio interface infomation.*/
  install_element (CONFIG_NODE, &radio_interface_show_cmd);

  /*for set/unset interface uplink flag.*/
  install_element (INTERFACE_NODE, &interface_set_uplink_flag_cmd);
  install_element (INTERFACE_NODE, &interface_unset_uplink_flag_cmd);
  
  install_element (INTERFACE_NODE, &interface_local_cmd);
  install_element (INTERFACE_NODE, &interface_global_cmd);
  
  /*gjd : add for set time interval for interface packets statistics */
  install_element (CONFIG_NODE, &set_interface_packets_statistics_sync_time_interval_cmd);
  
  
#ifdef HAVE_IPV6
  install_element (INTERFACE_NODE, &ipv6_address_cmd);
  install_element (INTERFACE_NODE, &no_ipv6_address_cmd);
#endif /* HAVE_IPV6 */
#ifdef HAVE_NETLINK
  install_element (INTERFACE_NODE, &ip_address_label_cmd);
  install_element (INTERFACE_NODE, &no_ip_address_label_cmd);
#endif /* HAVE_NETLINK */
}
