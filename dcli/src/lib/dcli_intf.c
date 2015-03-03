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
* dcli_intf.c
*
*
* CREATOR:
*       zhubo@autelan.com
*
* DESCRIPTION:
*       CLI definition for L3 interface module.
*
* DATE:
*       02/21/2008
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.158 $	
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif
#include <sys/wait.h>

#include <stdio.h>
#include <string.h>
#include <ctype.h>

/* for mmap(), zhangdi 20110810 */
#include <fcntl.h>
#include <sys/mman.h> 
/* end */

#include <zebra.h>
#include <dbus/dbus.h>

#include "sysdef/npd_sysdef.h"
#include "sysdef/returncode.h"
#include "dbus/npd/npd_dbus_def.h"
#include "if.h"
#include "command.h"
#include "vtysh/vtysh.h"
#include "board/board_define.h"

//#undef _D_WCPSS_/*zhanglei add*/
#ifdef _D_WCPSS_/*zhanglei add*/
#include "wcpss/waw.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/ACDbusDef1.h"
#include "dcli_wireless/dcli_wqos.h"
#endif

#include "dcli_acl.h"
#include "dcli_intf.h"
#include "dcli_vlan.h"
#include "dcli_common_stp.h"
#include "dcli_sem.h"
#include "dcli_main.h"
#include "dcli_sem.h"
#include "dbus/sem/sem_dbus_def.h"
#include "dcli_sys_manage.h"


extern DBusConnection *dcli_dbus_connection;
extern struct vtysh_client vtysh_client[];
extern int dcli_slot_id_get(void);
extern int is_distributed;

char vlan_eth_port_ifname [INTERFACE_NAMSIZ] = {0};
int interrupt_rxmax = 0;
/*
char bond_name[MAXLEN_BOND_CMD] = {0};
*/	
#ifdef _D_WCPSS_ /*zhanglei add*/

#endif

int parse_intf_name(char *str,char* name)
{

    char *endptr = NULL;

    if (NULL == str) return COMMON_ERROR;
    printf("before parsing,the str :: %s\n",str);
    strncpy(name,str,16);
    name[15] = '\0';

    return COMMON_SUCCESS;
}

int parse_ip_address(char *str,unsigned int* addr)
{
    char* endptr = NULL;

    if (NULL == str)return COMMON_ERROR;
    printf("before parsing the str :: %s\n",str);

    *addr = dcli_ip2ulong(str);
    printf("IP %d\n",*addr);
    return COMMON_SUCCESS;

}

int parse_param_no(char* str,unsigned int* param)
{
    char* endptr = NULL;

    if (NULL == str)return COMMON_ERROR;
    *param = strtoul(str,&endptr,10);

    return COMMON_SUCCESS;

}



int dcli_interface_ifname_vlan(struct vty * vty,char *ptr)
{
    int ret = 0;
    int i = 0;
    unsigned long vlanid = 0;
    unsigned int advanced = 0;
    unsigned int intfinfo = 0;
    if (0 != strncmp(ptr,"vlan",4))
    {
        return CMD_SUCCESS;
    }
    char *id = (char *)malloc(sizeof(char)*25);
    if (id == NULL)
    {
        return CMD_WARNING;
    }

    memset(id,0,25);
    memcpy(id,ptr+4,(strlen(ptr)-4));
    for (i = 0; i<strlen(ptr)-4; i++)
    {
        if ((id[0] == '0')||(id[0] == '\0')||((id[i]!='\0')&&((id[i]<'0')||(id[i]>'9'))))
        {
            vty_out(vty,"%% Bad parameter: %s !\n",ptr);
            free(id);
            id = NULL;
            return CMD_WARNING;
        }
    }
    vlanid = strtoul(id,NULL,10);
    free(id);
    id = NULL;
    if ((vlanid < 1)||(vlanid >4094))
    {
        vty_out(vty,"%% Bad parameter: %s !\n",ptr);
        return CMD_WARNING;
    }
    return dcli_create_vlan_intf_by_vlan_ifname(vty,(unsigned short)vlanid);
}

int dcli_no_interface_ifname_vlan(struct vty * vty,char *ptr)
{
    int i = 0;
    unsigned long vlanid = 0;
    char *id = NULL;
    if (NULL == ptr) return CMD_WARNING;
    if (0 != strncmp(ptr,"vlan",4))
    {
        return CMD_SUCCESS;
    }
    id = (char *)malloc(sizeof(char)*25);
    if (id == NULL)
    {
        return CMD_WARNING;
    }
    memset(id,0,25);
    memcpy(id,ptr+4,(strlen(ptr)-4));
    for (i = 0; i<strlen(ptr)-4; i++)
    {
        if ((id[0] == '0')||((id[i]!='\0')&&((id[i]<'0')||(id[i]>'9'))))
        {
            vty_out(vty,"%% Bad parameter: %s !\n",ptr);
            free(id);
            id = NULL;
            return CMD_WARNING;
        }
    }
    vlanid = strtoul(id,NULL,10);
    free(id);
    id = NULL;
    if ((vlanid < 1)||(vlanid >4094))
    {
        vty_out(vty,"%% Bad parameter: %s !\n",ptr);
        return CMD_WARNING;
    }
    return dcli_del_vlan_intf(vty,(unsigned short)vlanid);
}


int dcli_interface_ifname_eth_port(struct vty * vty,char * ptr)
{
    int ret = 0;
    int i = 0;
    unsigned long vlanid = 0;
    unsigned int advanced = 0;
    unsigned char slot = 0;
    unsigned char port = 0;
    unsigned char cpu_no = 0;
    unsigned char cpu_port_no = 0;	
    unsigned int tag1 = 0;
    unsigned int tag2 = 0;
    unsigned int ifIndex = ~0UI;
    unsigned int intfinfo = 0;
    unsigned char des_slot = 0;
	char * tmpPtr = NULL;
	int product_serial = get_product_info("/dbm/product/product_serial");
	int ifnameType = CONFIG_INTF_ETH; /* 0 : eth1-1, 1: e1-1*/

	char *endp = NULL;
	if(!ptr) {
		vty_out(vty, "%% Null parameter error!\n");
		return CMD_WARNING;
	}
	
	/* modify for AX7605i by qinhs@autelan.com 2009-11-18 */
	if(!strncmp(tolower(ptr),"cscd",4)){
		ret = parse_slotport_no(ptr, &slot, &port);
		if(NPD_FAIL == ret) {
			vty_out(vty, "%% Bad parameter %s!\n", ptr);
			return CMD_WARNING;
		}
		ifnameType = CONFIG_INTF_CSCD;
		return dcli_eth_port_interface_mode_config(vty,ifnameType,slot,port);
	}

    if (0 == strncmp(ptr,"eth",3)){
		tmpPtr = ptr + 3;
    }
    else if(0 == strncmp(ptr,"mng",3)){
        tmpPtr = ptr + 3;
		ifnameType = CONFIG_INTF_MNG;
    }
    else if(0 == strncmp(ptr,"ve",2)){
        tmpPtr = ptr + 2;
		if(strtoul(tmpPtr,&endp,10)<=0)
		{
			vty_out(vty, "%s : line %d Interface name %s err .%s", __func__,__LINE__,ptr, VTY_NEWLINE);
			return CMD_WARNING;
		}
		ifnameType = CONFIG_INTF_VE;
    }
	else if(dcli_interface_new_eth_ifname_check(ptr)){
		tmpPtr = ptr + 1;
		ifnameType = CONFIG_INTF_E;
	}
	else
    {
        return CMD_SUCCESS;
    }

    char *id = (char *)malloc(sizeof(char)*25);
    if (id == NULL)
    {
        return CMD_WARNING;
    }
    memset(id,0,25);
    memcpy(id,tmpPtr,(strlen(tmpPtr)));
    for (i = 0; i<strlen(tmpPtr); i++)
    {
        if ((id[i] != '-')&&(id[i] != 'f')&&(id[i] != 's')&&((id[i] != '.')&&((id[i]!='\0')&&((id[i]<'0')||(id[i]>'9')))))
        {
            vty_out(vty,"%% Bad parameter: %s !\n",ptr);
            free(id);
            id = NULL;
            return CMD_WARNING;
        }
    }

    if(!strncmp(ptr,"ve",2))
	{
		#if 0
    	ret = parse_slot_tag_no(id,&slot,&tag1, &tag2);
		#endif
    	ret = parse_ve_slot_cpu_tag_no(id,&slot,&cpu_no,&cpu_port_no,&tag1, &tag2);
 	}
	else
	{
        ret = parse_slotport_tag_no(id,&slot,&port,&tag1,&tag2);
    }

    free(id);
    id = NULL;
    if ((COMMON_SUCCESS != ret)||(tag1>4095)||(tag2 > 4093))
    {
        vty_out(vty,"%% Bab parameter: %s ",ptr);
        return CMD_WARNING;
    }
	if((product_serial == 7) && (tag2 == 4093))
	{
		vty_out(vty,"% Vlan %d is reserved on this Product.\n",tag2);
    	return CMD_WARNING;
	}
/*  
    if(0 == strncmp(ptr,"ve",2)) 
	{
		return dcli_eth_port_interface_mode_config_ve(vty,ifnameType,slot,port,des_slot);
	}
*/  
    if (0 == tag1)
    {
        return dcli_eth_port_interface_mode_config(vty,ifnameType,slot,port);
    }
    else
    {
/*
        if((DEFAULT_VLAN_ID == tag1)||(DEFAULT_VLAN_ID == tag2)){
            vty_out(vty,"%% The tag must be 2-4094!\n");
            return CMD_WARNING;
        }
*/		if((tag1 >= 1000) && (tag2 >= 1000))
		{
			vty_out(vty,"Only one of the QinQ intf's vid  can big than 1000,please use a small one !!!");	
			return CMD_WARNING;
		}
		else
		{
        	return dcli_create_eth_port_sub_intf(vty,ifnameType,slot,port,cpu_no,cpu_port_no,tag1,tag2);
		}
    }

}

int dcli_no_interface_ifname_eth_port(struct vty * vty,char * ptr)
{
    int ret = 0;
    int i = 0;
    unsigned long vlanid = 0;
    unsigned int advanced = 0;
    unsigned char slot = 0;
    unsigned char port = 0;
    unsigned char cpu_no = 0;
    unsigned char cpu_port_no = 0;	
    unsigned int mode = 0;
    unsigned int tag = 0;
    unsigned int tag2 = 0;
    unsigned char des_slot = 0;
	char * tmpPtr = NULL;
	int ifnameType = 0; /* 0 : eth1-1, 1: e1-1*/
    if (0 == strncmp(ptr,"eth",3)){
		tmpPtr = ptr + 3;
		ifnameType = CONFIG_INTF_ETH;
    }
	else if (0 == strncmp(ptr,"ve",2)){
		tmpPtr = ptr + 2;
		ifnameType = CONFIG_INTF_VE;
    }
	else if(dcli_interface_new_eth_ifname_check(ptr)){
		tmpPtr = ptr + 1;
		ifnameType = CONFIG_INTF_E;
	}
	else
    {
        return CMD_SUCCESS;
    }
    char *id = (char *)malloc(sizeof(char)*25);
    if (id == NULL)
    {
        return CMD_WARNING;
    }
    memset(id,0,25);
    memcpy(id,tmpPtr,(strlen(tmpPtr)));
    mode = ETH_PORT_FUNC_BRIDGE;
	
    if(!strncmp(ptr,"ve",2))
	{
    	/*ret = parse_slot_tag_no(id,&slot,&tag, &tag2);*/
    	ret = parse_ve_slot_cpu_tag_no(id,&slot,&cpu_no,&cpu_port_no,&tag, &tag2);
 	}
	else
	{
        ret = parse_slotport_tag_no(id,&slot,&port,&tag,&tag2);
    }

    free(id);
    id = NULL;
    if ((NPD_SUCCESS != ret)||(tag>4094)||(tag2>4094))
    {
        vty_out(vty,"%% Bab parameter: %s ",ptr);
        return CMD_WARNING;
    }
/*	
	if(0 == strncmp(ptr,"ve",2))
	{
	    return dcli_eth_port_mode_config_ve(vty,ifnameType,slot,port,des_slot);
	}
*/
    if (0 == tag)
    {
        return dcli_eth_port_mode_config(vty,ifnameType,slot,port,mode);
    }
    else
    {
        return dcli_del_eth_port_sub_intf(vty,ifnameType,slot,port,cpu_no,cpu_port_no,tag,tag2);
    }

}
/**
 *	dcli_interface_ifname_bond - create a bonding interface
 *	@vty
 *	@ptr: a point pointed to a bondname
 *
 */
int dcli_interface_ifname_bond(struct vty * vty,char * ptr)
{
	int i   = 0;
	int ret = -1;
	unsigned int bondid = 0;
	char cmd[MAXLEN_BOND_CMD] = {0};
	char file_name[MAXLEN_BOND_CMD] = {0};
	char line_buff[MAXLEN_BOND_CMD] = {0};
	FILE *fp = NULL;
	int line_count = 0;   //How many lines in a file.
	unsigned short tag1 = 0;
	unsigned short tag2 = 0;
	if(0 != strncmp(ptr, "bond", 4)){
		return CMD_SUCCESS;
	}
	char *id = (char *)malloc(sizeof(char)*25);
	
	if(id == NULL)
		return CMD_WARNING;
	
	memset(id, 0, 25);
	memcpy(id, ptr+4, (strlen(ptr)-4));
	
	if(id[0] == '\0'){
		vty_out(vty, "%% Bad parameter: %s !\n", ptr);
		free(id);
		id = NULL;
        return CMD_WARNING;
	}
	
	for (i=0; i<strlen(ptr)-4; i++){
		if((id[i]!='\0') && (id[i]!='.') && ((id[i]<'0')||(id[i]>'9'))){
			vty_out(vty, "%% Bad parameter: %s !\n", ptr);
			free(id);
			id = NULL;
            return CMD_WARNING;
		}
	}
	bondid = strtoul(id, NULL, 10);
	
	for (i=1; i<strlen(ptr)-4; i++){
		
		if((id[i]!='\0') && (id[i]!='.'))
			continue;
		
		if(id[i] == '\0')
			break;
		
		if(id[i] == '.'){
			if(tag1 && tag2){
				vty_out(vty, "%% Bad parameter: %s !\n", ptr);
        		free(id);
        		id = NULL;
				tag1 = tag2 = 0;
                return CMD_WARNING;
			}
			if(tag1)
				tag2 = strtoul(&id[i+1], NULL, 10);
			else
				tag1 = strtoul(&id[i+1], NULL, 10);
		}
	}

	#if 0
	vty_out(vty, "%% bondid =  %d \n", bondid);
	vty_out(vty, "%% tag1 =  %d \n", tag1);
	vty_out(vty, "%% tag2 =  %d \n", tag2);
	#endif
	
	free(id);
	id = NULL;
	if( (bondid < MIN_BONDID) || (bondid > MAX_BONDID) ){
        vty_out(vty, "%% Bad parameter: %s !\n", ptr);
		return CMD_WARNING;
	}
/*
	memset(bond_name, 0, MAXLEN_BOND_NAME);
	sprintf(bond_name, "%s", ptr);
	vty->index = (void*)bond_name;
*/
	/*
	  *If exist, ret is 0; 
	  *If not exist, ret is 256.
	  */
	memset(cmd, 0, MAXLEN_BOND_CMD);
	sprintf(cmd,"test -f /proc/net/bonding/bond%d\n", bondid);
	ret = system(cmd);

	//If bondx is exist
	if(!ret){
		//bondx
		if(!tag1){
			return CMD_SUCCESS;
		}
		//bondx.y
		else{
			//to get how many lines in file /proc/net/bonding/bondx.
			memset(file_name, 0, MAXLEN_BOND_CMD);
			sprintf(file_name,"/proc/net/bonding/bond%d", bondid);
			fp = fopen( file_name, "r");
			if( NULL == fp ){
				vty_out(vty, "%% WARNING: file /proc/net/bonding/bond%d open failed.\n", bondid);
				return CMD_WARNING;
			}
			while(1){
				if( fgets(line_buff, MAXLEN_BOND_CMD, fp) == NULL ){
					if( !feof(fp) ){
						vty_out(vty, "%% WARNING: file /proc/net/bonding/bond%d read error.\n", bondid);
						return CMD_WARNING;
					}
					else
						break;
				}
				line_count++;
			}
			fclose(fp);

			/*
			 * VLANs not supported on bondx when bondx has no real dev.
			 * If there is no interface has added into bondx, line_count<10
			 */
			if( line_count < 10 ){
    			vty_out(vty, "%% WARNING: Please add at least one interface into bond%d\n", bondid);
				return CMD_WARNING;
			}
			else {
				memset(cmd, 0, MAXLEN_BOND_CMD);
            	sprintf(cmd, "sudo ifconfig bond%d up\n", bondid);
            	system(cmd);
    			memset(cmd, 0, MAXLEN_BOND_CMD);
            	sprintf(cmd, "sudo vconfig add bond%d %d\n", bondid, tag1);
            	system(cmd);
    			memset(cmd, 0, MAXLEN_BOND_CMD);
            	sprintf(cmd, "sudo ifconfig bond%d.%d up\n", bondid, tag1);
            	system(cmd);
				//bondx.y
    			if(!tag2){
    				return CMD_SUCCESS;
    			}
    			//bondx.y.z
    			else{
    				memset(cmd, 0, MAXLEN_BOND_CMD);
                	sprintf(cmd, "sudo ifconfig bond%d.%d up\n", bondid, tag1);
                	system(cmd);
        			memset(cmd, 0, MAXLEN_BOND_CMD);
                	sprintf(cmd, "sudo vconfig add bond%d.%d %d\n", bondid, tag1, tag2);
                	system(cmd);
        			memset(cmd, 0, MAXLEN_BOND_CMD);
                	sprintf(cmd, "sudo ifconfig bond%d.%d.%d up\n", bondid, tag1, tag2);
                	system(cmd);
    				return CMD_SUCCESS;
    			}
			}
			
		}
	}
	//If bondx is not exist
	else {
		//bondx
		if(!tag1){
			memset(cmd, 0, MAXLEN_BOND_CMD);
        	sprintf(cmd, "sudo modprobe bonding -o %s bondname=%s miimon=1000\n", ptr, ptr);
        	system(cmd);
        	memset(cmd, 0, MAXLEN_BOND_CMD);
        	sprintf(cmd, "sudo ifconfig %s up\n", ptr);
        	system(cmd);
			return CMD_SUCCESS;
		}
		//bondx.y
		else{
			vty_out(vty, "%% Bad parameter: %s\n", ptr);
			vty_out(vty, "  Please create bond%d first, and add at least one interface into it.\n", bondid);
			return CMD_WARNING;
		}
	}

}


/**
 *	dcli_no_interface_ifname_bond - delete a bonding interface
 *	@vty
 *	@ptr: a point pointed to a bondname
 *
 */
int dcli_no_interface_ifname_bond(struct vty * vty,char * ptr)
{
	int i	= 0;
	int ret = -1;
	unsigned long bondid = 0;
	char cmd[MAXLEN_BOND_CMD];
	unsigned short tag1 = 0;
	unsigned short tag2 = 0;
	
	char bond_file[MAXLEN_BOND_CMD] = {0};
	if(0 != strncmp(ptr, "bond", 4)){
		return CMD_SUCCESS;
	}
	char *id = (char *)malloc(sizeof(char)*25);
	
	if(id == NULL)
		return CMD_WARNING;
	
	memset(id, 0, 25);
	memcpy(id, ptr+4, (strlen(ptr)-4));
	
	if(id[0] == '\0'){
		vty_out(vty, "%% Bad parameter: %s !\n", ptr);
		free(id);
		id = NULL;
        return CMD_WARNING;
	}
	
	for (i=0; i<strlen(ptr)-4; i++){
		if((id[i]!='\0') && (id[i]!='.') && ((id[i]<'0')||(id[i]>'9'))){
			vty_out(vty, "%% Bad parameter: %s !\n", ptr);
			free(id);
			id = NULL;
            return CMD_WARNING;
		}
	}
	bondid = strtoul(id, NULL, 10);
	
	for (i=1; i<strlen(ptr)-4; i++){
		
		if((id[i]!='\0') && (id[i]!='.') )
			continue;
		
		if(id[i] == '\0')
			break;
		
		if(id[i] == '.'){
			if(tag1 && tag2){
				vty_out(vty, "%% Bad parameter: %s !\n", ptr);
        		free(id);
        		id = NULL;
				tag1 = tag2 = 0;
                return CMD_WARNING;
			}
			if(tag1)
				tag2 = strtoul(&id[i+1], NULL, 10);
			else
				tag1 = strtoul(&id[i+1], NULL, 10);
		}
	}
	
	
	free(id);
	id = NULL;
	if( (bondid < MIN_BONDID) || (bondid > MAX_BONDID) ){
        vty_out(vty, "%% Bad parameter: %s !\n", ptr);
		return CMD_WARNING;
	}

	sprintf(bond_file,"test -f /proc/net/bonding/bond%d\n", bondid);
	/*If bondx is already exist, just turn to config mode.*/
	ret = system(bond_file);
	if(ret){
		vty_out(vty, "%% %s not exists!\n", ptr);
		return CMD_WARNING;
	}
	memset(cmd, 0, MAXLEN_BOND_CMD);

	/* no interface bondx */
	if( (!tag1) && (!tag2) ){
    	sprintf(cmd, "sudo rmmod %s\n", ptr);
	}
	/* no interface bondx.y or bondx.y.z*/
	else{
		sprintf(cmd, "sudo vconfig rem %s\n", ptr);
	}

	ret = system(cmd);
	
    return CMD_SUCCESS;
    }


#if 0
    DEFUN(config_interface_eth_port_cmd_func,
          config_interface_eth_port_cmd,
          "interface port SLOTNO",
          "Create System Interface \n"
          "Specify Interface via port_index\n"
          "eth global port_index\n"
         )
    {
        DBusMessage *query, *reply;
        DBusError err;
        struct interface *ifp = NULL;
        int ret;
        unsigned int op_ret = 0,ifIndex = 0;
        unsigned char slot_no=0,port_no = 0;
        char *pname = NULL;

        //printf("before parse_form_str_no %s\n",argv[0]);
        ret = parse_slotport_no((unsigned char *)argv[0],&slot_no,&port_no);;

        if (NPD_FAIL == ret)
        {
            vty_out(vty,"Unknow portno format.\n");
            return CMD_FAILURE;
        }

        //vty->index = ethIntf;
        //printf("interface slot_no %d,port_no %d\n",slot_no,port_no);
        query = dbus_message_new_method_call(
                    NPD_DBUS_BUSNAME,           \
                    NPD_DBUS_INTF_OBJPATH,      \
                    NPD_DBUS_INTF_INTERFACE,            \
                    NPD_DBUS_INTF_METHOD_CREATE_PORT_INTF);

        dbus_error_init(&err);

        dbus_message_append_args(query,
                                 DBUS_TYPE_BYTE,&slot_no,
                                 DBUS_TYPE_BYTE,&port_no,
                                 DBUS_TYPE_INVALID);

        reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);

        dbus_message_unref(query);

        if (NULL == reply)
        {
            printf("query reply null\n");
            vty_out(vty,"failed get reply.\n");
            if (dbus_error_is_set(&err))
            {
                printf("%s raised: %s",err.name,err.message);
                dbus_error_free_for_dcli(&err);
            }
            dbus_message_unref(reply);
            return CMD_SUCCESS;
        }

        if (dbus_message_get_args(reply, &err,
                                  DBUS_TYPE_UINT32,&op_ret,
                                  DBUS_TYPE_UINT32, &ifIndex,
                                  DBUS_TYPE_STRING,&pname,
                                  DBUS_TYPE_INVALID))
        {

            if (NPD_DBUS_SUCCESS == op_ret)
            {
                //vty_out(vty,"interface  index %d\n",ifIndex);
            }
            else if (DCLI_INTF_EXISTED == op_ret)
            {
                printf("INTF EXSITED\n");
                //printf("interface Id %d\n",ifIndex);
                /*ifp = if_lookup_by_index(ifIndex);*/
            }
            else
            {
                if (NPD_DBUS_ERROR_NO_SUCH_PORT == op_ret)
                    printf("NO SUCH PORT\n");
                if (NPD_DBUS_ERROR == op_ret)
                    printf("CREATE INTF ERROR\n");
                if (DCLI_VLAN_NOTEXISTS == op_ret)
                    printf("VLAN NOT EXITSTS\n");
                if (DCLI_SET_FDB_ERR == op_ret)
                    printf("SET STATIC MAC ADDR FAILED\n");
                if (DCLI_INTF_EN_ROUTING_ERR == op_ret)
                    printf("ENABLE ROUTING ERROR\n");

                //printf("interface Id %0x\n",ifIndex);

                dbus_message_unref(reply);
                return CMD_FAILURE;
            }

        }
        else
        {
            printf("Failed get args.\n");
            vty_out(vty,"Failed get args.\n");
            if (dbus_error_is_set(&err))
            {
                printf("%s raised: %s",err.name,err.message);
                dbus_error_free_for_dcli(&err);
            }
        }
        dbus_message_unref(reply);
        //printf("query reply op %d index %d,name %s\n",op_ret,ifIndex,pname);
#if 0
        if (!ifp)
        {
            printf("Create\n");
            ifp = if_get_by_name_len(pname,strlen(pname));
            ifp->ifindex =  ifIndex;
        }

        printf("pif %p,name %s , strlen(pname) %d \n",ifp,ifp->name,strlen(ifp->name));
        vty->index = ifp;
        vty->node = INTERFACE_NODE;
#endif

        return CMD_SUCCESS;
    }


    DEFUN(no_config_interface_eth_port_cmd_func,
          no_config_interface_eth_port_cmd,
          "no interface port SLOTNO",
          "Delete System configuration\n"
          "Delete System Interface\n"
          "Specify Interface via port_index\n"
          "eth global port_index\n"
         )
    {
        DBusMessage *query, *reply;
        DBusError err;
        struct interface *ifp = NULL;
        int ret;
        unsigned int op_ret = 0,ifIndex = 0;
        unsigned char slot_no=0,port_no = 0;

        //printf("before parse_form_str_no %s\n",argv[0]);
        ret = parse_slotport_no((unsigned char *)argv[0],&slot_no,&port_no);;

        if (NPD_FAIL == ret)
        {
            vty_out(vty,"Unknow portno format.\n");
            return CMD_SUCCESS;
        }

        //vty->index = ethIntf;
        //printf("interface slot_no %d,port_no %d\n",slot_no,port_no);
        query = dbus_message_new_method_call(
                    NPD_DBUS_BUSNAME,           \
                    NPD_DBUS_INTF_OBJPATH,      \
                    NPD_DBUS_INTF_INTERFACE,            \
                    NPD_DBUS_INTF_METHOD_DEL_PORT_INTF);

        dbus_error_init(&err);

        dbus_message_append_args(query,
                                 DBUS_TYPE_BYTE,&slot_no,
                                 DBUS_TYPE_BYTE,&port_no,
                                 DBUS_TYPE_INVALID);

        reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);

        dbus_message_unref(query);

        if (NULL == reply)
        {
            printf("query reply null\n");
            vty_out(vty,"failed get reply.\n");
            if (dbus_error_is_set(&err))
            {
                printf("%s raised: %s",err.name,err.message);
                dbus_error_free_for_dcli(&err);
            }
            dbus_message_unref(reply);
            return CMD_SUCCESS;
        }
        //printf("query reply not null\n");
        if (dbus_message_get_args(reply, &err,
                                  DBUS_TYPE_UINT32,&op_ret,
                                  DBUS_TYPE_UINT32, &ifIndex,
                                  DBUS_TYPE_INVALID))
        {

            if (NPD_DBUS_SUCCESS == op_ret)
            {
                /*vty_out(vty,"interface  index %d\n",ifIndex);

                ifp = if_lookup_by_index(ifIndex);
                if_delete(ifp);*/
            }
            else
            {
                if (NPD_DBUS_ERROR_NO_SUCH_PORT == op_ret)
                    printf("NO SUCH PORT\n");
                if (DCLI_INTF_NOTEXISTED == op_ret)
                    printf("DCLI INTF NOTEXISTED\n");
                if (NPD_DBUS_ERROR == op_ret)
                    printf("DELETE INTF ERROR\n");
                if (DCLI_VLAN_NOTEXISTS == op_ret)
                    printf("VLAN NOT EXITSTS\n");
                if (DCLI_SET_FDB_ERR == op_ret)
                    printf("SET STATIC MAC ADDR FAILED\n");
                if (DCLI_INTF_DIS_ROUTING_ERR == op_ret)
                    printf("DISABLE ROUTING ERROR\n");

                //printf("interface Id %0x\n",ifIndex);
                dbus_message_unref(reply);
                return CMD_FAILURE;
            }

        }
        else
        {
            printf("Failed get args.\n");
            vty_out(vty,"Failed get args.\n");
            if (dbus_error_is_set(&err))
            {
                printf("%s raised: %s",err.name,err.message);
                dbus_error_free_for_dcli(&err);
            }
        }
        dbus_message_unref(reply);
        //printf("query reply op %d index %d\n",op_ret,ifIndex);

        return CMD_SUCCESS;
    }
#endif

    int dcli_create_vlan_intf
    (
        struct vty* vty,
        unsigned short vid,
        unsigned int advanced
    )
    {
        DBusMessage *query, *reply;
        DBusError err;
        int ret = CMD_SUCCESS;
        unsigned int op_ret = INTERFACE_RETURN_CODE_ERROR,ifIndex = 0,vId = 0;
        char *pname = NULL;

        if (vid > 4094)
        {
            vty_out(vty,"%% input bad param.\n");
            return CMD_WARNING;
        }

        query = dbus_message_new_method_call(
                    NPD_DBUS_BUSNAME,           \
                    NPD_DBUS_INTF_OBJPATH,          \
                    NPD_DBUS_INTF_INTERFACE,                    \
                    NPD_DBUS_INTF_METHOD_CREATE_VID_INTF);

        dbus_error_init(&err);

        dbus_message_append_args(query,
                                 DBUS_TYPE_UINT16,&vid,
                                 DBUS_TYPE_UINT32,&advanced,
                                 DBUS_TYPE_INVALID);

        reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);

        dbus_message_unref(query);

        if (NULL == reply)
        {
            vty_out(vty,"query reply null\n");
            vty_out(vty,"failed get reply.\n");
            if (dbus_error_is_set(&err))
            {
                vty_out(vty,"%s raised: %s",err.name,err.message);
                dbus_error_free_for_dcli(&err);
            }
            dbus_message_unref(reply);
            return CMD_WARNING;
        }
        if (dbus_message_get_args(reply, &err,
                                  DBUS_TYPE_UINT32,&op_ret,
                                  DBUS_TYPE_UINT32,&ifIndex,
                                  DBUS_TYPE_STRING,&pname,
                                  DBUS_TYPE_INVALID))
        {
            if (INTERFACE_RETURN_CODE_SUCCESS != op_ret)
            {
                vty_out(vty,dcli_error_info_intf(op_ret));
                ret = CMD_WARNING;
            }
            dbus_message_unref(reply);
            return ret;
        }
        else
        {
            vty_out(vty,"Failed get args.\n");
            if (dbus_error_is_set(&err))
            {
                vty_out(vty,"%s raised: %s",err.name,err.message);
                dbus_error_free_for_dcli(&err);
            }
        }
        dbus_message_unref(reply);
        return CMD_WARNING;
    }

    int dcli_create_vlan_intf_by_vlan_ifname
    (
        struct vty* vty,
        unsigned short vid
    )
    {
		/* on AX8610,can not ax7605i */
        if(is_distributed == DISTRIBUTED_SYSTEM)
        {
            DBusMessage *query, *reply;
            DBusError err;
            int ret = CMD_SUCCESS;
            unsigned int op_ret = INTERFACE_RETURN_CODE_ERROR,ifIndex = 0,vId = 0;

            char *pname = NULL;

            if (vid > 4093)
            {
                vty_out(vty,"%% input bad param.\n");
                return CMD_WARNING;
            }
            int board_type = 0;
        	int master_slot_id[2] = {-1, -1};
			int i = 0,slot_id =0;
			char *master_slot_cnt_file = "/dbm/product/master_slot_count";
			
            /* get board type, bugfix:	AXSSZFI-518 */
			board_type = get_product_info("/proc/product_info/board_type_num");
			if(128 == board_type)
			{
                vty_out(vty,"%% Unsupport on board AX71_CRSMU.\n");
                return CMD_WARNING;
			}
			
        	int local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
            int master_slot_count = get_product_info(master_slot_cnt_file);
        	ret = dcli_master_slot_id_get(master_slot_id);
        	if(ret !=0 )
        	{
        		vty_out(vty,"get master_slot_id error !\n");
        		return CMD_WARNING;		
           	}
			/* Check is master board or not */
			if( (local_slot_id !=master_slot_id[0])&&(local_slot_id !=master_slot_id[1]) )
			{
				master_slot_count = 1;      /* loop one times */
				master_slot_id[0] = local_slot_id;  /* config for not master board */
			}			
	        for(i=0;i<master_slot_count;i++)
	        {
				slot_id = master_slot_id[i];
                query = dbus_message_new_method_call(
                            NPD_DBUS_BUSNAME,           \
                            NPD_DBUS_INTF_OBJPATH,          \
                            NPD_DBUS_INTF_INTERFACE,                    \
                            NPD_DBUS_INTF_METHOD_CREATE_VID_INTF_BY_VLAN_IFNAME);

                dbus_error_init(&err);

                dbus_message_append_args(query,
                                         DBUS_TYPE_UINT16,&vid,
                                         DBUS_TYPE_INVALID);

                if(NULL == dbus_connection_dcli[slot_id]->dcli_dbus_connection) 				
    			{
        			if(slot_id == local_slot_id)
    				{
                        reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
    				}
    				else 
    				{	
    				   	vty_out(vty,"Can not connect to master slot:%d \n",slot_id);	
    					continue;
    				}
                }
    			else
    			{
                    reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,query,-1, &err);				
    			}

                dbus_message_unref(query);

                if (NULL == reply)
                {
                    vty_out(vty,"query reply null,Please check slot:%d\n",slot_id);
                    dbus_message_unref(reply);
                    return CMD_WARNING;
                }
                if (dbus_message_get_args(reply, &err,
                                          DBUS_TYPE_UINT32,&op_ret,
                                          DBUS_TYPE_UINT32,&ifIndex,
                                          DBUS_TYPE_STRING,&pname,
                                          DBUS_TYPE_INVALID))
                {
                    if (INTERFACE_RETURN_CODE_SUCCESS != op_ret)
                    {
						if(INTERFACE_RETURN_CODE_IOCTL_ERROR == op_ret)
    					{
                            vty_out(vty,"% Create kap intf failed, ifindex is to large, check please! \n");    						
    					}
						else
						{
                            vty_out(vty,"% On slot %d: \n",slot_id);
                            vty_out(vty,dcli_error_info_intf(op_ret));
						}
						dbus_message_unref(reply);
        				return CMD_WARNING;											
                    }
                    /*sleep(1);*/
                    /*return ret;*/
					/*next slot */
                }
                else
                {
                    vty_out(vty,"Failed get args, Please check slot:%d\n",slot_id);
                    dbus_message_unref(reply);
    				return CMD_WARNING;					
                }
	        }
			return CMD_SUCCESS;
        }			

        DBusMessage *query, *reply;
        DBusError err;
        int ret = CMD_SUCCESS;
        unsigned int op_ret = INTERFACE_RETURN_CODE_ERROR,ifIndex = 0,vId = 0;

        char *pname = NULL;

        if (vid > 4094)
        {
            vty_out(vty,"%% input bad param.\n");
            return CMD_WARNING;
        }

        query = dbus_message_new_method_call(
                    NPD_DBUS_BUSNAME,           \
                    NPD_DBUS_INTF_OBJPATH,          \
                    NPD_DBUS_INTF_INTERFACE,                    \
                    NPD_DBUS_INTF_METHOD_CREATE_VID_INTF_BY_VLAN_IFNAME);

        dbus_error_init(&err);

        dbus_message_append_args(query,
                                 DBUS_TYPE_UINT16,&vid,
                                 DBUS_TYPE_INVALID);

        reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);

        dbus_message_unref(query);

        if (NULL == reply)
        {
            vty_out(vty,"query reply null\n");
            vty_out(vty,"failed get reply.\n");
            if (dbus_error_is_set(&err))
            {
                vty_out(vty,"%s raised: %s",err.name,err.message);
                dbus_error_free_for_dcli(&err);
            }
            dbus_message_unref(reply);
            return CMD_WARNING;
        }
        if (dbus_message_get_args(reply, &err,
                                  DBUS_TYPE_UINT32,&op_ret,
                                  DBUS_TYPE_UINT32,&ifIndex,
                                  DBUS_TYPE_STRING,&pname,
                                  DBUS_TYPE_INVALID))
        {
            if (INTERFACE_RETURN_CODE_SUCCESS != op_ret)
            {
                vty_out(vty,dcli_error_info_intf(op_ret));
                ret = CMD_WARNING;
            }
            dbus_message_unref(reply);
            /*sleep(1);*/
            return ret;
        }
        else
        {
            vty_out(vty,"Failed get args.\n");
            if (dbus_error_is_set(&err))
            {
                vty_out(vty,"%s raised: %s",err.name,err.message);
                dbus_error_free_for_dcli(&err);
            }
        }
        dbus_message_unref(reply);
        return CMD_WARNING;
    }


    int dcli_vlan_interface_advanced_routing_enable
    (
        struct vty* vty,
        unsigned short vid,
        unsigned int enable
    )
    {
        DBusMessage *query, *reply;
        DBusError err;
        int ret = CMD_SUCCESS;
        unsigned int op_ret = INTERFACE_RETURN_CODE_ERROR,ifIndex = 0,vId = 0;

        char *pname = NULL;

        if (vid > 4094)
        {
            vty_out(vty,"%% input bad param.\n");
            return CMD_WARNING;
        }

        query = dbus_message_new_method_call(
                    NPD_DBUS_BUSNAME,           \
                    NPD_DBUS_INTF_OBJPATH,          \
                    NPD_DBUS_INTF_INTERFACE,                    \
                    NPD_DBUS_INTF_METHOD_VLAN_INTERFACE_ADVANCED_ROUTING_ENABLE);

        dbus_error_init(&err);

        dbus_message_append_args(query,
                                 DBUS_TYPE_UINT16,&vid,
                                 DBUS_TYPE_UINT32,&enable,
                                 DBUS_TYPE_INVALID);

        reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);

        dbus_message_unref(query);

        if (NULL == reply)
        {
            vty_out(vty,"query reply null\n");
            vty_out(vty,"failed get reply.\n");
            if (dbus_error_is_set(&err))
            {
                vty_out(vty,"%s raised: %s",err.name,err.message);
                dbus_error_free_for_dcli(&err);
            }
            return CMD_WARNING;
        }
        if (dbus_message_get_args(reply, &err,
                                  DBUS_TYPE_UINT32,&op_ret,
                                  DBUS_TYPE_UINT32,&ifIndex,
                                  DBUS_TYPE_STRING,&pname,
                                  DBUS_TYPE_INVALID))
        {
            if (INTERFACE_RETURN_CODE_SUCCESS != op_ret)
            {
                if (INTERFACE_RETURN_CODE_INTERFACE_NOTEXIST== op_ret)
                {
                    vty_out(vty,"%% Advanced-routing already disabled!\n");
                }
                else if (INTERFACE_RETURN_CODE_ALREADY_ADVANCED== op_ret)
                {
                    vty_out(vty,"%% Advanced-routing already enabled!\n");
                }
                else
                {
                    vty_out(vty,dcli_error_info_intf(op_ret));
                }
                ret = CMD_WARNING;
            }
            dbus_message_unref(reply);
            return ret;
        }
        else
        {
            vty_out(vty,"Failed get args.\n");
            if (dbus_error_is_set(&err))
            {
                vty_out(vty,"%s raised: %s",err.name,err.message);
                dbus_error_free_for_dcli(&err);
            }
        }
        dbus_message_unref(reply);
        return CMD_WARNING;
    }

    int dcli_del_vlan_intf
    (
        struct vty * vty,
        unsigned short vId
    )
    {
		/* on AX8610,can not ax7605i */
        if(is_distributed == DISTRIBUTED_SYSTEM)
		{
            DBusMessage *query, *reply;
            DBusError err;
            struct interface *ifp = NULL;
            unsigned int op_ret = 0,ifIndex = 0;
			int ret=0;
            char name[16] = {'\0'};

            if (vId > 4093)
            {
                vty_out(vty,"%% input bad param.\n");
                return CMD_WARNING;
            }

        	int master_slot_id[2] = {-1, -1};
			int i = 0,slot_id =0;
			char *master_slot_cnt_file = "/dbm/product/master_slot_count";
        	int local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
            int master_slot_count = get_product_info(master_slot_cnt_file);
        	ret = dcli_master_slot_id_get(master_slot_id);
        	if(ret !=0 )
        	{
        		vty_out(vty,"get master_slot_id error !\n");
        		return CMD_WARNING;		
           	}
			/* Check is master board or not */
			if( (local_slot_id !=master_slot_id[0])&&(local_slot_id !=master_slot_id[1]) )
			{
				master_slot_count = 1;      /* loop one times */
				master_slot_id[0] = local_slot_id;  /* config for not master board */
			}
	        for(i=0;i<master_slot_count;i++)
	        {
				slot_id = master_slot_id[i];

                query = dbus_message_new_method_call(
                            NPD_DBUS_BUSNAME,           \
                            NPD_DBUS_INTF_OBJPATH,      \
                            NPD_DBUS_INTF_INTERFACE,            \
                            NPD_DBUS_INTF_METHOD_DEL_VID_INTF);

                dbus_error_init(&err);

                dbus_message_append_args(query,
                                         DBUS_TYPE_UINT16,&vId,
                                         DBUS_TYPE_INVALID);

                if(NULL == dbus_connection_dcli[slot_id]->dcli_dbus_connection) 				
    			{
        			if(slot_id == local_slot_id)
    				{
                        reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
    				}
    				else 
    				{	
    				   	vty_out(vty,"Can not connect to master slot:%d \n",slot_id);	
    					continue;
    				}
                }
    			else
    			{
                    reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,query,-1, &err);				
    			}

                dbus_message_unref(query);

                if (NULL == reply)
                {
                    vty_out(vty,"query reply null,Please check slot:%d\n",slot_id);
                    dbus_message_unref(reply);
                    return CMD_WARNING;
                }

                if (dbus_message_get_args(reply, &err,
                                          DBUS_TYPE_UINT32,&op_ret,
                                          DBUS_TYPE_UINT32, &ifIndex,
                                          DBUS_TYPE_INVALID))
                {
                    if (INTERFACE_RETURN_CODE_SUCCESS == op_ret)
                    {

                        sprintf(name,"vlan%d",vId);
                        dbus_message_unref(reply);
                        sleep(1);
                        /*return CMD_SUCCESS;*/
						/*next slot */
                    }
                    else
                    {
                        vty_out(vty,dcli_error_info_intf(op_ret));
                        dbus_message_unref(reply);
                        return CMD_WARNING;
                    }
                }
                else
                {
                    vty_out(vty,"Failed get args, Please check slot:%d\n",slot_id);
	                dbus_message_unref(reply);
                    return CMD_WARNING;
                }
	        }
			return CMD_SUCCESS;
		}
		
        DBusMessage *query, *reply;
        DBusError err;
        struct interface *ifp = NULL;
        unsigned int op_ret = 0,ifIndex = 0;
        char name[16] = {'\0'};

        if (vId > 4094)
        {
            vty_out(vty,"%% input bad param.\n");
            return CMD_WARNING;
        }

        query = dbus_message_new_method_call(
                    NPD_DBUS_BUSNAME,           \
                    NPD_DBUS_INTF_OBJPATH,      \
                    NPD_DBUS_INTF_INTERFACE,            \
                    NPD_DBUS_INTF_METHOD_DEL_VID_INTF);

        dbus_error_init(&err);

        dbus_message_append_args(query,
                                 DBUS_TYPE_UINT16,&vId,
                                 DBUS_TYPE_INVALID);

        reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,40000, &err);

        dbus_message_unref(query);

        if (NULL == reply)
        {
            vty_out(vty,"query reply null\n");
            vty_out(vty,"failed get reply.\n");
            if (dbus_error_is_set(&err))
            {
                vty_out(vty,"%s raised: %s",err.name,err.message);
                dbus_error_free_for_dcli(&err);
            }
            dbus_message_unref(reply);
            return CMD_WARNING;
        }

        if (dbus_message_get_args(reply, &err,
                                  DBUS_TYPE_UINT32,&op_ret,
                                  DBUS_TYPE_UINT32, &ifIndex,
                                  DBUS_TYPE_INVALID))
        {
            if (INTERFACE_RETURN_CODE_SUCCESS == op_ret)
            {

                sprintf(name,"vlan%d",vId);
                dbus_message_unref(reply);
                sleep(1);
                return CMD_SUCCESS;
            }
            else
            {
                vty_out(vty,dcli_error_info_intf(op_ret));

                dbus_message_unref(reply);
                return CMD_WARNING;
            }
        }
        else
        {
            vty_out(vty,"Failed get args.\n");
            if (dbus_error_is_set(&err))
            {
                vty_out(vty,"%s raised: %s",err.name,err.message);
                dbus_error_free_for_dcli(&err);
            }
        }
        dbus_message_unref(reply);
        return CMD_WARNING;

    }

    int dcli_create_eth_port_sub_intf
    (
        struct vty * vty,
    	int ifnameType,
        unsigned char slot_no,      
        unsigned char port_no,
        unsigned char cpu_no,
        unsigned char cpu_port_no,          
        unsigned int vid,
        unsigned int vid2
    )
    {
        DBusMessage *query, *reply;
        DBusError err;

        unsigned int op_ret = 0, cmd_ret1 = CMD_WARNING, cmd_ret2 = CMD_WARNING;
        struct interface *ifp = NULL;
		int local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
		int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
		int product_type = get_product_info(SEM_PRODUCT_TYPE_PATH);
		int function_type = -1;
		char file_path[64] = {0};
		int i=0,k=0;

        if(is_distributed == DISTRIBUTED_SYSTEM)
        {
			for(i = 1; i <= slotNum; i++)
			{
				sprintf(file_path,"/dbm/product/slot/slot%d/function_type", i);
				function_type = get_product_info(file_path);
				
				if (function_type != SWITCH_BOARD)
				{
    				if(ifnameType == CONFIG_INTF_ETH)
    				{
                        query = dbus_message_new_method_call(\
                                                             SEM_DBUS_BUSNAME,           \
                                                             SEM_DBUS_INTF_OBJPATH,      \
                                                             SEM_DBUS_INTF_INTERFACE,    \
                                                             SEM_DBUS_SUB_INTERFACE_CREATE);

                        dbus_error_init(&err);

                        dbus_message_append_args(query,
                                                 DBUS_TYPE_UINT32,&ifnameType,
                                                 DBUS_TYPE_BYTE,&slot_no,
                                                 DBUS_TYPE_BYTE,&port_no,
                                                 DBUS_TYPE_UINT32,&vid,
                                                 DBUS_TYPE_UINT32,&vid2,
                                                 DBUS_TYPE_INVALID);

                    	if(NULL == dbus_connection_dcli[i]->dcli_dbus_connection) 				
                    	{
                    		if(i == local_slot_id) 
                    		{
                    		    reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
                    		}
                    		else 
                    		{	
                    			vty_out(vty, "Connection to slot%d is not exist.\n", i);
                    			continue;
                    		}
                    	}else {
                    		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[i]->dcli_dbus_connection,\
                    				query, -1, &err);
                    	}

                        dbus_message_unref(query);

                        if (NULL == reply)
                        {
                            vty_out(vty,"failed get reply.\n");
                            if (dbus_error_is_set(&err))
                            {
                                vty_out(vty,"%s raised: %s",err.name,err.message);
                                dbus_error_free_for_dcli(&err);
                            }
                            continue;
                        }
                        DCLI_DEBUG(("query reply not null\n"));
                       
                        if (dbus_message_get_args(reply, &err,
                                  DBUS_TYPE_UINT32,&op_ret,
                                  DBUS_TYPE_INVALID))
                		{
                			if (SEM_COMMAND_FAILED == op_ret ) 
                			{
                				vty_out(vty,"%% Create sub-interface failed on slot%d.\n", i);
                			}
                			else if(SEM_COMMAND_SUCCESS == op_ret)
                			{
								cmd_ret1 = CMD_SUCCESS;
                				DCLI_DEBUG(("create sub-interface successed on slot%d.\n", i));
                			}
                			else if (SEM_WRONG_PARAM == op_ret ) 
                			{
                				vty_out(vty,"%% wrong parame\n"); 
                			}
                			else if (SEM_OPERATE_NOT_SUPPORT == op_ret) {
                				
                				vty_out(vty,"%% No support sub-interface setting on slot%d\n", i);
                			}
                			else if (SEM_COMMAND_NOT_SUPPORT == op_ret) {
                				// TODO:call npd dbus to process.
                				DCLI_DEBUG(("%% call npd dbus to process\n"));
                			}
                			else
                			{
                				vty_out(vty, "unkown return value\n");
                			}
                		} 
                		else 
                		{
                			vty_out(vty,"Failed get args.\n");
                			if (dbus_error_is_set(&err)) 
                			{
                				vty_out(vty,"%s raised: %s",err.name,err.message);
                				dbus_error_free_for_dcli(&err);
                			}
                		}  
    					dbus_message_unref(reply);
    				}
    				else if(ifnameType == CONFIG_INTF_MNG)
    				{
						/* bugfix: AXSSZFI-492 */
           				vty_out(vty,"%% Not support sub-interface setting for mng port !\n");
						return CMD_WARNING;
    				}
					else
    				{
						/* just send to the local board, Active-master or AC itself */
						if(local_slot_id!=i)
						{
							continue;
						}
                        query = dbus_message_new_method_call(\
                                                             NPD_DBUS_BUSNAME,                   \
                                                             NPD_DBUS_INTF_OBJPATH,      \
                                                             NPD_DBUS_INTF_INTERFACE,        \
                                                             NPD_DBUS_SUB_INTERFACE_CREATE);

                        dbus_error_init(&err);

                        dbus_message_append_args(query,
                                                 DBUS_TYPE_UINT32,&ifnameType,
                                                 DBUS_TYPE_BYTE,&slot_no,
                                                 DBUS_TYPE_BYTE,&port_no,
                                                 DBUS_TYPE_BYTE,&cpu_no,
                                                 DBUS_TYPE_BYTE,&cpu_port_no,                                                 
                                                 DBUS_TYPE_UINT32,&vid,
                                                 DBUS_TYPE_UINT32,&vid2,
                                                 DBUS_TYPE_INVALID);
                    	if(NULL == dbus_connection_dcli[i]->dcli_dbus_connection) 				
                    	{
                    		if(i == local_slot_id) 
                    		{
                    		    reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
                    		}
                    		else 
                    		{	
                    			vty_out(vty, "Connection to slot%d is not exist.\n", i);
                    			continue;
                    		}
                    	}else {
                    		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[i]->dcli_dbus_connection,\
                    				query, -1, &err);
                    	}

                        dbus_message_unref(query);

                        if (NULL == reply)
                        {
                            vty_out(vty,"failed get reply.\n");
                            if (dbus_error_is_set(&err))
                            {
                                vty_out(vty,"%s raised: %s",err.name,err.message);
                                dbus_error_free_for_dcli(&err);
                            }
                            continue;
                        }

                        DCLI_DEBUG(("query reply not null\n"));
                        if (dbus_message_get_args(reply, &err,
                                                  DBUS_TYPE_UINT32,&op_ret,
                                                  DBUS_TYPE_INVALID))
                        {
                            if (INTERFACE_RETURN_CODE_SUBIF_EXISTS == op_ret)
                            {
                                op_ret = INTERFACE_RETURN_CODE_SUCCESS;
								return CMD_SUCCESS;
                            }
							
                            if (INTERFACE_RETURN_CODE_SUCCESS == op_ret)
                            {
								cmd_ret1 = CMD_SUCCESS;
								DCLI_DEBUG(("create sub-interface successed on slot%d.\n", i));
                            }
							else if(INTERFACE_RETURN_CODE_SUBIF_CREATE_FAILED == op_ret)
							{
                                vty_out(vty,"%% Create sub-interface failed on slot%d.\n", i);
							}		
							else if(INTERFACE_RETURN_CODE_UNSUPPORT_COMMAND == op_ret)
							{
                                vty_out(vty,"%% Unsupported sub-interface setting for slot%d, not AC-board.\n", slot_no);
							}								
							else if(INTERFACE_RETURN_CODE_VLAN_NOTEXIST == op_ret)
							{
                                vty_out(vty,"%% The vlan does not exist on slot%d.\n", i);
							}
                            else if (INTERFACE_RETURN_CODE_QINQ_TWO_SAME_TAG == op_ret)
                            {
                                vty_out(vty,"%% The internal tag %d is the same as the external tag %d!\n",vid,vid2);
                            }
                            else if (INTERFACE_RETURN_CODE_NO_SUCH_PORT == op_ret)
                            {
                                vty_out(vty,"%% NO SUCH PORT %d/%d, tag %d!\n",slot_no,port_no,vid);
                            }
                            else if (INTERFACE_RETURN_CODE_ADD_PORT_FAILED == op_ret)
                            {
                                vty_out(vty,"%% FAILED to add port %d/%d to vlan %d!\n",slot_no,port_no,vid);
                            }
                            else
                            {
                                vty_out(vty,dcli_error_info_intf(op_ret));
                            }
                        }
                        else
                        {
                            vty_out(vty,"Failed get args.\n");
                            if (dbus_error_is_set(&err))
                            {
                                vty_out(vty,"%s raised: %s",err.name,err.message);
                                dbus_error_free_for_dcli(&err);
                            }
                        }
                        dbus_message_unref(reply);
    				}
				}
				else
				{
					/* jump not local board! zhangdi@autelan.com 2012-06-13 */
                    /*vty_out(vty,"%% Jump slot%d(switch-board).\n",i);*/
					continue;
				}

			}
			return cmd_ret1;

			#if 0   /* no use, set in npd. zhangdi@autelan.com 2012-06-05 */
			/* jump non ve port, like eth-port */
			if(ifnameType == CONFIG_INTF_VE)
			{			
				/* Update the state of ve sub-interface on Active-MCB */
				/* 1. map the share vlan file */
	    		int fd = -1;
	    		struct stat sb;
	    		vlan_list_t * mem_vlan_list =NULL;
	    		char* file_path = "/dbm/shm/vlan/shm_vlan";
    		
	    		/* open file read & write */
				fd = open(file_path, O_RDWR);
	        	if(fd < 0)
	            {
	                vty_out(vty,"Failed to open shm_vlan: %s\n", strerror(errno));
	                return CMD_WARNING;
	            }
	    		fstat(fd,&sb);
	    		/* map shared */	
	            mem_vlan_list = (vlan_list_t *)mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0 );
	            if(MAP_FAILED == mem_vlan_list)
	            {
	                vty_out(vty,"Failed to mmap for g_vlanlist[]! %s\n", strerror(errno));
	    			close(fd);
	                return CMD_WARNING;
	            }	
		
	            /* 2. If bonded to this slot, set the vlan down, then the thread will make it UP */
	            if(mem_vlan_list[vid-1].bond_slot[slot_no-1] == 1)
	            {				
				    mem_vlan_list[vid-1].updown = 0;
	    			cmd_ret2 = msync(mem_vlan_list, sizeof(vlan_list_t)*4096, MS_SYNC);
	                if( cmd_ret2!=0 )
	                {
	                    vty_out(vty,"msync shm_vlan failed \n" );
	                	return CMD_WARNING;
	                }
	            }

				/* 3. munmap mem & close fd */
	            cmd_ret2 = munmap(mem_vlan_list,sb.st_size);
	            if( cmd_ret2 != 0 )
	            {
	                vty_out(vty,"Failed to munmap for g_vlanlist[]! %s\n", strerror(errno));			
	            }	
	    		cmd_ret2 = close(fd);
	    		if( cmd_ret2 != 0 )
	            {
	                vty_out(vty,"close shm_vlan failed \n" );   
	            }   
			
				return cmd_ret1 | cmd_ret2;
			}
			#endif
        }
		else
		{
            query = dbus_message_new_method_call(\
                                                 NPD_DBUS_BUSNAME,                   \
                                                 NPD_DBUS_INTF_OBJPATH,      \
                                                 NPD_DBUS_INTF_INTERFACE,        \
                                                 NPD_DBUS_SUB_INTERFACE_CREATE);

            dbus_error_init(&err);

            dbus_message_append_args(query,
                                     DBUS_TYPE_UINT32,&ifnameType,
                                     DBUS_TYPE_BYTE,&slot_no,
                                     DBUS_TYPE_BYTE,&port_no,
                                     DBUS_TYPE_UINT32,&vid,
                                     DBUS_TYPE_UINT32,&vid2,
                                     DBUS_TYPE_INVALID);

            reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);

            dbus_message_unref(query);

            if (NULL == reply)
            {
                vty_out(vty,"failed get reply.\n");
                if (dbus_error_is_set(&err))
                {
                    vty_out(vty,"%s raised: %s",err.name,err.message);
                    dbus_error_free_for_dcli(&err);
                }
                return CMD_WARNING;
            }

            DCLI_DEBUG(("query reply not null\n"));
            if (dbus_message_get_args(reply, &err,
                                      DBUS_TYPE_UINT32,&op_ret,
                                      DBUS_TYPE_INVALID))
            {
                /*vty_out(vty,"return op_ret %d\n",op_ret);*/
                if (INTERFACE_RETURN_CODE_SUBIF_EXISTS == op_ret)
                {
                    op_ret = INTERFACE_RETURN_CODE_SUCCESS;
                }
                if (INTERFACE_RETURN_CODE_SUCCESS == op_ret)
                {
                    dbus_message_unref(reply);
                    /* It's waste time very much when lots of interfaces
                    		sleep(1);*/
                    return CMD_SUCCESS;
                }
                else if (INTERFACE_RETURN_CODE_QINQ_TWO_SAME_TAG == op_ret)
                {
                    vty_out(vty,"%% The internal tag %d is the same as the external tag %d!\n",vid,vid2);
                }
                else if (INTERFACE_RETURN_CODE_NO_SUCH_PORT == op_ret)
                {
                    vty_out(vty,"%% NO SUCH PORT %d/%d, tag %d!\n",slot_no,port_no,vid);
                }
                else if (INTERFACE_RETURN_CODE_ADD_PORT_FAILED == op_ret)
                {
                    vty_out(vty,"%% FAILED to add port %d/%d to vlan %d!\n",slot_no,port_no,vid);
                }
                else
                {
                    vty_out(vty,dcli_error_info_intf(op_ret));
                }
            }
            else
            {
                vty_out(vty,"Failed get args.\n");
                if (dbus_error_is_set(&err))
                {
                    vty_out(vty,"%s raised: %s",err.name,err.message);
                    dbus_error_free_for_dcli(&err);
                }
            }
            dbus_message_unref(reply);
            return CMD_WARNING;
		}
         return CMD_WARNING;
    }

    int dcli_del_eth_port_sub_intf
    (
        struct vty * vty,
        int ifnameType,
        unsigned char slot_no,
        unsigned char port_no,
        unsigned char cpu_no,
        unsigned char cpu_port_no,        
        unsigned int vid,
        unsigned int vid2
    )
    {
        DBusMessage *query, *reply;
        DBusError err;
        unsigned int op_ret = 0, cmd_ret = CMD_WARNING;
        struct interface *ifp = NULL;
        int local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
		int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
		int product_type = get_product_info(SEM_PRODUCT_TYPE_PATH);
		int function_type = -1;
		char file_path[64] = {0};
		int i;

        if(is_distributed == DISTRIBUTED_SYSTEM)
        {
			for(i = 1; i <= slotNum; i++)
			{
				sprintf(file_path,"/dbm/product/slot/slot%d/function_type", i);
				function_type = get_product_info(file_path);
				
				if (function_type != SWITCH_BOARD)
				{
    				if(ifnameType == CONFIG_INTF_ETH)
    				{
			
                        query = dbus_message_new_method_call(\
                                                             SEM_DBUS_BUSNAME,           \
                                                             SEM_DBUS_INTF_OBJPATH,      \
                                                             SEM_DBUS_INTF_INTERFACE,    \
                                                             SEM_DBUS_SUB_INTERFACE_DELETE);

                        dbus_error_init(&err);

                        dbus_message_append_args(query,
                                                 DBUS_TYPE_UINT32,&ifnameType,
                                                 DBUS_TYPE_BYTE,&slot_no,
                                                 DBUS_TYPE_BYTE,&port_no,
                                                 DBUS_TYPE_UINT32,&vid,
                                                 DBUS_TYPE_UINT32,&vid2,
                                                 DBUS_TYPE_INVALID);

                    	if(NULL == dbus_connection_dcli[i]->dcli_dbus_connection) 				
                    	{
                    		if(i == local_slot_id) 
                    		{
                    		    reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
                    		}
                    		else 
                    		{	
                    			vty_out(vty, "Connection to slot%d is not exist.\n", i);
                    			continue;
                    		}
                    	}else {
                    		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[i]->dcli_dbus_connection,\
                    				query, -1, &err);
                    	}

                        dbus_message_unref(query);

                        if (NULL == reply)
                        {
                            vty_out(vty,"failed get reply.\n");
                            if (dbus_error_is_set(&err))
                            {
                                vty_out(vty,"%s raised: %s",err.name,err.message);
                                dbus_error_free_for_dcli(&err);
                            }
                            continue;
                        }
                        DCLI_DEBUG(("query reply not null\n"));
           
                        if (dbus_message_get_args(reply, &err,
                                                  DBUS_TYPE_UINT32,&op_ret,
                                                  DBUS_TYPE_INVALID))
                        {
                			if (SEM_COMMAND_FAILED == op_ret ) 
                			{
                				vty_out(vty,"%% Delete sub-interface failed on slot%d.\n", i);
                			}
                			else if(SEM_COMMAND_SUCCESS == op_ret)
                			{
								cmd_ret = CMD_SUCCESS;
                				DCLI_DEBUG(("Delete sub-interface successed on slot%d.\n", i));
                			}
                			else if (SEM_WRONG_PARAM == op_ret ) 
                			{
                				vty_out(vty,"%% wrong parame\n"); 
                			}
                			else if (SEM_OPERATE_NOT_SUPPORT == op_ret) {
                				
                				vty_out(vty,"%% Unsupport sub-interface setting on slot%d.\n", i);
                			}
                			else if (SEM_COMMAND_NOT_SUPPORT == op_ret) {
                				// TODO:call npd dbus to process.
                				DCLI_DEBUG(("%% call npd dbus to process\n"));
                			}
                			else
                			{
                				vty_out(vty, "unkown return value\n");
                			}

                        }
                        else
                        {
                            vty_out(vty,"Failed get args.\n");
                            if (dbus_error_is_set(&err))
                            {
                                vty_out(vty,"%s raised: %s",err.name,err.message);
                                dbus_error_free_for_dcli(&err);
                            }
                        }
                        dbus_message_unref(reply);
                       
                    }
					else
					{
						/* just send to the local board, Active-master or AC itself */
						if(local_slot_id!=i)
						{
							continue;
						}						
                        query = dbus_message_new_method_call(\
                                                             NPD_DBUS_BUSNAME,                   \
                                                             NPD_DBUS_INTF_OBJPATH,      \
                                                             NPD_DBUS_INTF_INTERFACE,        \
                                                             NPD_DBUS_SUB_INTERFACE_DELETE);

                        dbus_error_init(&err);

                        dbus_message_append_args(query,
                								 DBUS_TYPE_UINT32,&ifnameType,
                                                 DBUS_TYPE_BYTE,&slot_no,
                                                 DBUS_TYPE_BYTE,&port_no,
                                                 DBUS_TYPE_BYTE,&cpu_no,
                                                 DBUS_TYPE_BYTE,&cpu_port_no,                                                   
                                                 DBUS_TYPE_UINT32,&vid,
                                                 DBUS_TYPE_UINT32,&vid2,
                                                 DBUS_TYPE_INVALID);

                    	if(NULL == dbus_connection_dcli[i]->dcli_dbus_connection) 				
                    	{
                    		if(i == local_slot_id) 
                    		{
                    		    reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
                    		}
                    		else 
                    		{	
                    			vty_out(vty, "Connection to slot%d is not exist.\n", i);
                    			continue;
                    		}
                    	}else {
                    		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[i]->dcli_dbus_connection,\
                    				query, -1, &err);
                    	}

                        dbus_message_unref(query);

                        if (NULL == reply)
                        {
                            vty_out(vty,"failed get reply.\n");
                            if (dbus_error_is_set(&err))
                            {
                                vty_out(vty,"%s raised: %s",err.name,err.message);
                                dbus_error_free_for_dcli(&err);
                            }
                            return CMD_WARNING;
                        }

                        DCLI_DEBUG(("query reply not null\n"));
                        if (dbus_message_get_args(reply, &err,
                                                  DBUS_TYPE_UINT32,&op_ret,
                                                  DBUS_TYPE_INVALID))
                        {
                            if (INTERFACE_RETURN_CODE_SUCCESS == op_ret)
                            {
                                cmd_ret = CMD_SUCCESS;
                				DCLI_DEBUG(("Delete sub-interface successed on slot%d.\n", i));
                            }
							else if(INTERFACE_RETURN_CODE_SUBIF_CREATE_FAILED == op_ret)
							{
                                vty_out(vty,"%% Delete sub-interface failed on slot%d.\n", i);
							}									
							else if(INTERFACE_RETURN_CODE_UNSUPPORT_COMMAND == op_ret)
							{
                                vty_out(vty,"%% Unsupported sub-interface setting for slot%d, not AC-board.\n", slot_no);
							}                           
						    else
                            {
                                vty_out(vty,dcli_error_info_intf(op_ret));
                            }
                        }
                        else
                        {
                            vty_out(vty,"Failed get args.\n");
                            if (dbus_error_is_set(&err))
                            {
                                vty_out(vty,"%s raised: %s",err.name,err.message);
                                dbus_error_free_for_dcli(&err);
                            }
                        }
                        dbus_message_unref(reply);
                        			
					}
				}
				else
				{
					/* jump not local board! zhangdi@autelan.com 2012-06-13 */
                    /*vty_out(vty,"%% Jump slot%d(switch-board).\n",i);*/
					continue;
				}				
			}

			return cmd_ret;		
        }

		
        query = dbus_message_new_method_call(\
                                             NPD_DBUS_BUSNAME,                   \
                                             NPD_DBUS_INTF_OBJPATH,      \
                                             NPD_DBUS_INTF_INTERFACE,        \
                                             NPD_DBUS_SUB_INTERFACE_DELETE);

        dbus_error_init(&err);

        dbus_message_append_args(query,
								 DBUS_TYPE_UINT32,&ifnameType,
                                 DBUS_TYPE_BYTE,&slot_no,
                                 DBUS_TYPE_BYTE,&port_no,
                                 DBUS_TYPE_UINT32,&vid,
                                 DBUS_TYPE_UINT32,&vid2,
                                 DBUS_TYPE_INVALID);

        reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,40000, &err);

        dbus_message_unref(query);

        if (NULL == reply)
        {
            vty_out(vty,"failed get reply.\n");
            if (dbus_error_is_set(&err))
            {
                vty_out(vty,"%s raised: %s",err.name,err.message);
                dbus_error_free_for_dcli(&err);
            }
            return CMD_WARNING;
        }

        DCLI_DEBUG(("query reply not null\n"));
        if (dbus_message_get_args(reply, &err,
                                  DBUS_TYPE_UINT32,&op_ret,
                                  DBUS_TYPE_INVALID))
        {
            if (INTERFACE_RETURN_CODE_SUCCESS == op_ret)
            {
                dbus_message_unref(reply);
                sleep(1);
                return CMD_SUCCESS;
            }
            else
            {
                vty_out(vty,dcli_error_info_intf(op_ret));
            }

            dbus_message_unref(reply);
            return CMD_WARNING;

        }
        else
        {
            vty_out(vty,"Failed get args.\n");
            if (dbus_error_is_set(&err))
            {
                vty_out(vty,"%s raised: %s",err.name,err.message);
                dbus_error_free_for_dcli(&err);
            }
        }
        dbus_message_unref(reply);
        return CMD_WARNING;

    }

#if 0
DEFUN(config_interface_vlanId_advan_cmd_func,
	config_interface_vanId_advan_cmd,
	"interface vlan <1-4094> advanced-routing",
	"Create System Interface\n"
	"Specify Interface via vlanId \n"
	"vlanId NO\n"
	"Config advanced route mode"
	)
{

	int ret;
	unsigned int vid = 0;
	unsigned int advanced = 1;

	/*printf("before interface vlan parse %s\n",argv[0]);*/
	ret = parse_param_no((char *)argv[0],&vid);

	if (COMMON_ERROR == ret) {
    	vty_out(vty,"Unknow portno format.\n");
		return CMD_WARNING;
	}

#if 0
	if(argc > 1) {
		if(0 == strcmp(argv[1],"advanced-routing"))
			advanced = 1;
		else {
			vty_out(vty,"input bad param\n");
			return CMD_FAILURE;
		}
			
	}
	else if(argc > 2) {
		vty_out(vty,"too many param\n");
		return CMD_FAILURE;
	}
#endif
	
	/*vty->index = ethIntf;
	//printf("interface vId %d\n",vId);*/
	return dcli_create_vlan_intf(vty,vid,advanced);

}

DEFUN(config_interface_vlanId_cmd_func,
	config_interface_vanId_cmd,
	"interface vlan <1-4094>",
	"Create System Interface\n"
	"Specify Interface via vlanId \n"
	"vlanId NO\n"
)
{

	int ret;
	unsigned int vid = 0;
	char *pname = NULL;
	unsigned int advanced = 0;

	/*printf("before interface vlan parse %s\n",argv[0]);*/
	ret = parse_param_no((char *)argv[0],&vid);

	if (COMMON_ERROR == ret) {
    	vty_out(vty,"Unknow vlan id %s\n",argv[0]);
		return CMD_WARNING;
	}

	return dcli_create_vlan_intf(vty,vid,advanced);

}


DEFUN(no_config_interface_vlanId_cmd_func,
	no_config_interface_vanId_cmd,
	"no interface vlan <1-4094>",
	"Delete System configuration\n"
	"Delete System Interface\n"
	"Specify Interface via vlanId \n"
	"vlanId NO\n"
	)
{
	int ret;
	unsigned int input = 0;
	unsigned short vId = 0;
	
	ret = parse_param_no((char *)argv[0],&input);

	if (COMMON_ERROR == ret) {
    	vty_out(vty,"Unknow vlan id %s\n",argv[0]);
		return CMD_WARNING;
	}
	vId = (unsigned short)input;
	
	return dcli_del_vlan_intf(vty,vId);
}


DEFUN(config_sub_interface_cmd_func,
	config_sub_interface_cmd,
	"interface eth-port PORTNO tag <1-4094>",
	"System Interface\n"
	"config eth-port \n"
	"port form as example 3/4 means slot 3 port 4.\n"
	"tag value accord to 802.1Q \n"
	"vlan id"
	)
{
	
	int ret;
    unsigned int slot_no = 0;
	unsigned char port_no = 0;
    unsigned int vid = 0;

	DCLI_DEBUG(("before parse_slotport_no %s\n",argv[0]));
	if(argc >2) {
		vty_out(vty,"input bad param\n");
	}
	ret = parse_slotport_no((unsigned char *)argv[0],&slot_no,&port_no);;
	if (NPD_FAIL == ret) {
		vty_out(vty,"Unknow portno format.\n");
		return CMD_SUCCESS;
	}

	ret = parse_param_no((char *)argv[1],&vid);
	if (COMMON_ERROR == ret) {
		vty_out(vty,"Unknow portno format.\n");
		return CMD_SUCCESS;
	}
	DCLI_DEBUG(("after parse_slotport_no vid %d\n",vid));

	return dcli_create_eth_port_sub_intf(vty,slot_no,port_no,vid,0);

}


DEFUN(config_del_sub_interface_cmd_func,
	config_del_sub_interface_cmd,
	"no interface eth-port PORTNO tag <1-4094>",
	"Delete System configuration\n"
	"System Interface\n"
	"config eth-port \n"
	"port form as example 3/4 means slot 3 port 4.\n"
	"tag value accord to 802.1Q \n"
	"vlan id"
	)
{
	DBusMessage *query, *reply;
	DBusError err;
	int ret;
	unsigned int op_ret = 0 ;
	unsigned char slot_no = 0, port_no = 0;
	unsigned int vid = 0;
	struct interface *ifp = NULL;

	DCLI_DEBUG(("before parse_slotport_no %S\n",argv[0]));
	if(argc >2) {
		vty_out(vty,"input bad param\n");
	}

	ret = parse_slotport_no((unsigned char *)argv[0],&slot_no,&port_no);;
	if (NPD_FAIL == ret) {
    	vty_out(vty,"Unknow portno format.\n");
		return CMD_SUCCESS;
	}
	
	ret = parse_param_no((char *)argv[1],&vid);
	if (COMMON_ERROR == ret) {
    	vty_out(vty,"Unknow portno format.\n");
		return CMD_SUCCESS;
	}
	DCLI_DEBUG(("after vid %d\n",vid));

	return dcli_del_eth_port_sub_intf(vty,slot_no,port_no,vid,0);
}




DEFUN(interface_advanced_routing_cmd_func,
	interface_advanced_routing_interface_cmd,
	"advanced-routing (enable|disable)",
	"config advanced route mode\n"
	"enable advanced route\n"
	"disable advanced route\n"
	)
{
	DBusMessage *query, *reply;
	DBusError err;
	int ret;
	unsigned int op_ret = 0;
	unsigned int vid = 0;
	unsigned int isEnable = 0;

	if(argc > 1){
		vty_out(vty,"input too many params\n ");
		return CMD_WARNING;
	}
	
	DCLI_DEBUG(("before parse value %s\n",argv[0]));
	if(strcmp(argv[0] , "enable") == 0){
		isEnable = 1;
	}
	else if (strcmp(argv[0],"disable")== 0){
		isEnable = 0;
	}
	else {
		vty_out(vty,"input  value must be enable or disable\n");
		return CMD_WARNING;
	}

	vid = (unsigned int)vty->index;
	vty_out(vty,"vid %d\n",vid);
	
	query = dbus_message_new_method_call(			\
							NPD_DBUS_BUSNAME,					\
							NPD_DBUS_INTF_OBJPATH,		\
							NPD_DBUS_INTF_INTERFACE,		\
							NPD_DBUS_INTERFACE_ADVANCED_ROUTING);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&isEnable,
							 DBUS_TYPE_UINT32,&vid,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	DCLI_DEBUG(("query reply not null\n"));
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)) {
			DCLI_DEBUG(("return op_ret %d\n",op_ret));
			if (INTERFACE_RETURN_CODE_SUCCESS == op_ret ) {
				DCLI_DEBUG(("success\n"));
			}
			else{
				vty_out(vty,dcli_error_info_intf(op_ret));
			}
			dbus_message_unref(reply);
			return CMD_WARNING;
			
	} 
	else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);

	return CMD_SUCCESS;
}
#endif
DEFUN(vlan_eth_port_interface_advanced_routing_cmd_func,
	interface_advanced_routing_vlan_eth_port_cmd,
	"advanced-routing (enable|disable)",
	"config advanced route mode\n"
	"enable advanced route\n"
	"disable advanced route\n"
	)
{
	unsigned int vid = 0;
	unsigned int isEnable = 0;
	char ifName[INTERFACE_NAMSIZ+1] = {0};
	unsigned char port_no = 0,slot_no = 0;
	
	int i = 0;
	
	/*To get ifname from vty*/
	memcpy((void *)ifName,(void *)vlan_eth_port_ifname,INTERFACE_NAMSIZ);
	if((0 != strncmp(ifName,"vlan",4))&&(0 != strncmp(ifName,"eth",3))&&(0 != strncmp(ifName,"cscd",4))&&\
		(FALSE == dcli_interface_new_eth_ifname_check(ifName))){
		vty_out(vty,"%% Advanced-routing unsupported!\n");
        return CMD_WARNING;
	}
	if(argc > 1){
		vty_out(vty,"%% Input too many params!\n ");
		return CMD_WARNING;
	}
	
	if(strncmp(argv[0] , "enable",strlen(argv[0])) == 0){
		isEnable = 1;
	}
	else if (strncmp(argv[0],"disable",strlen(argv[0]))== 0){
		isEnable = 0;
	}
	else {
		vty_out(vty,"%% Input  value must be enable or disable!\n");
		return CMD_WARNING;
	}
	if(CMD_SUCCESS != dcli_advanced_routing_config(vty,ifName,isEnable)){
		return CMD_WARNING;
	}
	else {
		return CMD_SUCCESS;
	}
	vty_out(vty,"%% Advanced-routing unsupported!\n");
	return CMD_WARNING;

}


DEFUN(show_interface_advanced_routing_vlan_eth_port_cmd_func,
	show_interface_advanced_routing_vlan_eth_port_cmd,
	"show advanced-routing",
	SHOW_STR
	"show advaced-routing config of this interface\n"
	)
{
	unsigned int vid = 0;
	unsigned int isEnable = 0;
	char ifName[INTERFACE_NAMSIZ+1] = {0};
	unsigned char port_no = 0,slot_no = 0;
	unsigned int flag = 2;	
	int i = 0;
	
	/*To get ifname from vty*/
	memcpy((void *)ifName,(void *)vlan_eth_port_ifname,INTERFACE_NAMSIZ);
	if((0 != strncmp(ifName,"vlan",4))&&(0 != strncmp(ifName,"eth",3))&&\
		(FALSE == dcli_interface_new_eth_ifname_check(ifName))){
		vty_out(vty,"%% Interface %s not support advaced-routing\n",vlan_eth_port_ifname);
        return CMD_WARNING;
	}
	if(argc != 0){
		vty_out(vty,"%% Input too many params!\n ");
		return CMD_WARNING;
	}
	flag = parse_param_ifName(ifName,&slot_no,&port_no,&vid);
	
	return dcli_intf_vlan_eth_port_interface_show_advanced_routing(vty,flag,(unsigned int)slot_no,(unsigned int)port_no,vid);

}

DEFUN(interface_bond_set_mode_cmd_func,
	interface_bond_set_mode_cmd,
	"set mode (balance-rr|active-backup|balance-xor|broadcast|802.3ad|balance-tlb|balance-alb)",
	"Set bonding mode.\n"
	"Specifies one of the bonding policies.\n"
	"Bonding.\n"
	"The default is balance-rr (round robin)."
)
{
	unsigned int mode = 0;
	char cmd[MAXLEN_BOND_CMD];
	char *ptr = NULL;
	
	if(0 == strncmp(argv[0], "balance-rr", strlen(argv[0]))) {
		mode = 0;
	}
	else if(0 == strncmp(argv[0], "active-backup", strlen(argv[0]))) {
		mode = 1;
	}
	else if(0 == strncmp(argv[0], "balance-xor", strlen(argv[0]))) {
		mode = 2;
	}
	else if(0 == strncmp(argv[0], "broadcast", strlen(argv[0]))) {
		mode = 3;
	}
	else if(0 == strncmp(argv[0], "802.3ad", strlen(argv[0]))) {
		mode = 4;
	}
	else if(0 == strncmp(argv[0], "balance-tlb", strlen(argv[0]))) {
		mode = 5;
	}
	else if(0 == strncmp(argv[0], "balance-alb", strlen(argv[0]))) {
		mode = 6;
	}
	else {
		vty_out(vty, "% Bad parameter!\n");
		return CMD_WARNING;
	}
	
	ptr = (char *)(vty->index);
	if( (NULL == ptr) || (0 != strncmp(ptr, "bond", 4)) ){
		vty_out(vty,"% Warning: Only bonding interface support the operation.\n");
		return CMD_WARNING;
	}
	
	memset(cmd, 0, MAXLEN_BOND_CMD);
	sprintf(cmd, "sudo ifconfig %s down\n", ptr);
	system(cmd);
	memset(cmd, 0, MAXLEN_BOND_CMD);
	sprintf(cmd, "sudo echo %d > /sys/class/net/%s/bonding/mode\n", mode, ptr);
	system(cmd);
	memset(cmd, 0, MAXLEN_BOND_CMD);
	sprintf(cmd, "sudo ifconfig %s up\n", ptr);
	system(cmd);

	return CMD_SUCCESS;
}

DEFUN(interface_bond_set_lacp_rate_cmd_func,
	interface_bond_set_lacp_rate_cmd,
	"set lacp_rate (slow|fast)",
	"set lacp_rate.\n"
	"set lacp_rate.\n"
	"Bonding.\n"
	"set lacp_rate."
)
{
	unsigned int lacp_rate = 0;
	char cmd[MAXLEN_BOND_CMD];
	char *ptr = NULL;
	
	if(0 == strncmp(argv[0], "slow", strlen(argv[0]))) {
		lacp_rate = 0;
	}
	else if(0 == strncmp(argv[0], "fast", strlen(argv[0]))) {
		lacp_rate = 1;
	}
	else {
		vty_out(vty, "% Bad parameter!\n");
		return CMD_WARNING;
	}
	
	ptr = (char *)(vty->index);
	if( (NULL == ptr) || (0 != strncmp(ptr, "bond", 4)) ){
		vty_out(vty,"% Warning: Only bonding interface support the operation.\n");
		return CMD_WARNING;
	}
	
	memset(cmd, 0, MAXLEN_BOND_CMD);
	sprintf(cmd, "sudo ifconfig %s down\n", ptr);
	system(cmd);
	memset(cmd, 0, MAXLEN_BOND_CMD);
	sprintf(cmd, "sudo echo %d > /sys/class/net/%s/bonding/lacp_rate\n", lacp_rate, ptr);
	system(cmd);
	memset(cmd, 0, MAXLEN_BOND_CMD);
	sprintf(cmd, "sudo ifconfig %s up\n", ptr);
	system(cmd);

	return CMD_SUCCESS;
}


DEFUN(interface_bond_set_xmit_hash_policy_cmd_func,
	interface_bond_set_xmit_hash_policy_cmd,
	"set xmit_hash_policy (layer2|layer3+4)",
	"set xmit_hash_policy.\n"
	"set xmit_hash_policy.\n"
	"Bonding.\n"
	"set xmit_hash_policy."
)
{
	unsigned int xmit_hash_policy = 0;
	char cmd[MAXLEN_BOND_CMD];
	char *ptr = NULL;
	
	if(0 == strncmp(argv[0], "layer2", strlen(argv[0]))) {
		xmit_hash_policy = 0;
	}
	else if(0 == strncmp(argv[0], "layer3+4", strlen(argv[0]))) {
		xmit_hash_policy = 1;
	}
	else {
		vty_out(vty, "% Bad parameter!\n");
		return CMD_WARNING;
	}
	
	ptr = (char *)(vty->index);
	if( (NULL == ptr) || (0 != strncmp(ptr, "bond", 4)) ){
		vty_out(vty,"% Warning: Only bonding interface support the operation.\n");
		return CMD_WARNING;
	}
	
	memset(cmd, 0, MAXLEN_BOND_CMD);
	sprintf(cmd, "sudo ifconfig %s down\n", ptr);
	system(cmd);
	memset(cmd, 0, MAXLEN_BOND_CMD);
	sprintf(cmd, "sudo echo %d > /sys/class/net/%s/bonding/xmit_hash_policy\n", xmit_hash_policy, ptr);
	system(cmd);
	memset(cmd, 0, MAXLEN_BOND_CMD);
	sprintf(cmd, "sudo ifconfig %s up\n", ptr);
	system(cmd);

	return CMD_SUCCESS;
}


#if 0
DEFUN(interface_bond_show_params_cmd_func,
	interface_bond_show_params_cmd,
	"show params",
	"show bonding interface parameters.\n"
	"show bonding interface parameters.\n"
	"Bonding.\n"
	"show bonding interface parameters."
)
{}
#endif


DEFUN(interface_bond_add_del_port_cmd_func,
	interface_bond_add_del_port_cmd,
	"(add|delete) bonding INTERFACE",
	"Add port to bonding interface.\n"
	"Delete port from) bonding interface.\n"
	"Bonding.\n"
	"Slaved interface name"
)
{
	int ret = -1;
	int isAdd = 0;
	unsigned int bondid = 0;
	char cmd[MAXLEN_BOND_CMD];
	char *ptr = NULL;
	
	/*fetch the 1st param : add ? delete*/
	if(0 == strncmp(argv[0], "add", strlen(argv[0]))) {
		isAdd = TRUE;
	}
	else if (0 == strncmp(argv[0], "delete", strlen(argv[0]))) {
		isAdd= FALSE;
	}
	else {
		vty_out(vty, "% Bad parameter!\n");
		return CMD_WARNING;
	}

	/* fetch the 2nd param : ethm-n/vlann. */
	if( (strncmp(argv[1], "eth", 3) != 0) && (strncmp(argv[1], "vlan", 4) != 0)&&\
		(FALSE == dcli_interface_new_eth_ifname_check(argv[1]))) {
		vty_out(vty,"% Bad parameter: %s\n", argv[1]);
		return CMD_WARNING;
	}
	
	ptr = (char *)(vty->index);
	if( (NULL == ptr) || (0 != strncmp(ptr, "bond", 4)) ){
		vty_out(vty,"% Warning: Only bonding interface support add/delete operation.\n");
		return CMD_WARNING;
	}
	
	memset(cmd, 0, MAXLEN_BOND_CMD);
	if(isAdd){
		sprintf(cmd, "sudo ifenslave -f %s %s\n", ptr, argv[1]);
	}else{
		sprintf(cmd, "sudo ifenslave -d %s %s\nsudo ifconfig %s up\n", ptr, argv[1], argv[1]);
	}
	
	system(cmd);
	return CMD_SUCCESS;
}

DEFUN(show_bond_slave_cmd_func,
	show_bond_slave_cmd,
	"show bonding BOND",
	SHOW_STR
	"Bonding\n"
	"Show all interface slaved to bondx.\n"
	)
{
	unsigned int bondid = 0;
	unsigned int nodesave = 0;
	char cmd[MAXLEN_BOND_CMD];
	char *ptr = NULL;
	int i;
	char bond_file[MAXLEN_BOND_CMD];
	int ret;

	/*match bondx*/
	if(0 != strncmp(argv[0], "bond", 4)) {
		vty_out(vty, "% Bad parameter!\n");
		return CMD_WARNING;
	}

	ptr = (char *)argv[0];
	char *id = (char *)malloc(sizeof(char)*25);
	if(id == NULL)
		return CMD_WARNING;
	
	memset(id, 0, 25);
	memcpy(id, ptr+4, (strlen(ptr)-4));
	for (i=0; i<strlen(ptr)-4; i++){
		if((id[0] == '\0')||((id[i]!='\0')&&((id[i]<'0')||(id[i]>'9')))){
			vty_out(vty, "%% Bad parameter: %s !\n", ptr);
			free(id);
			id = NULL;
            return CMD_WARNING;
		}
	}
	
	bondid = strtoul(id, NULL, 10);
	free(id);
	id = NULL;
	if( (bondid < MIN_BONDID) || (bondid > MAX_BONDID) ){
        vty_out(vty, "%% Bad parameter: %s !\n", ptr);
		return CMD_WARNING;
	}
	sprintf(bond_file,"test -f /proc/net/bonding/%s\n",ptr);
	/*If bondx is already exist, just turn to config mode.*/
	ret = system(bond_file);
	if(!ret){
		memset(cmd, 0, MAXLEN_BOND_CMD);
    	sprintf(cmd, "sudo cat /proc/net/bonding/bond%d\n", bondid);
    	system(cmd);
		return CMD_SUCCESS;
	}
	else{
		vty_out(vty, "%% %s not exist!\n", ptr);
		return CMD_WARNING;
	}
}

#define CVM_IOC_MAGIC 242
#define CVM_IOC_SET_INTERRUPT_RXMAX    _IOWR(CVM_IOC_MAGIC, 24, int) 
#define CVM_IOC_READ_INTERRUPT_RXMAX   _IOWR(CVM_IOC_MAGIC, 25, int) 
DEFUN(show_cvm_rxmax_per_interrupt_cmd_func,
    show_cvm_rxmax_per_interrupt_cmd,
    "show interrupt rxmax",
    SHOW_STR
    "show interrupt rxmax\n"
    "show rxmax in an interrupt.\n"
    )
{
    int fd; 
	int ret; 
    int rxmax_in_an_interrupt = 0;

	fd = open("/dev/oct0",0);	
	if(fd < 0)
	{ 	
		vty_out(vty,"%% Open /dev/oct0 failed!\n");
        return CMD_WARNING;
	}		
	ret = ioctl(fd, CVM_IOC_READ_INTERRUPT_RXMAX, &rxmax_in_an_interrupt);
	if(ret == -1)
	{		
		close(fd);
		vty_out(vty,"%% IOCTL /dev/oct0 failed!\n");
        return CMD_WARNING;
	}
	else
	{		
		vty_out(vty," %d\n", rxmax_in_an_interrupt);
	}
    
	close(fd);
	return CMD_SUCCESS;
    
}

DEFUN(set_cvm_rxmax_per_interrupt_cmd_func,
    set_cvm_rxmax_per_interrupt_cmd,
    "set interrupt rxmax RXMAX",
    SHOW_STR
    "set interrupt rxmax\n"
    "set rxmax in an interrupt.\n"
    )
{
    int fd; 
    int ret; 
    int rxmax_in_an_interrupt = 0;
    char ** endptr = NULL;
        
    rxmax_in_an_interrupt = strtoul(argv[0],endptr,NULL);
    if ((0 >= rxmax_in_an_interrupt)||\
        ((NULL != endptr)&&('\0' != endptr[0])))
    {
        vty_out(vty,"%% Bad parameter %s\n",argv[0]);
        return CMD_WARNING;
    }
    interrupt_rxmax = rxmax_in_an_interrupt;
    fd = open("/dev/oct0",0);   
    if(fd < 0)
    {   
        vty_out(vty,"%% Open /dev/oct0 failed!\n");
        return CMD_WARNING;
    }       
    ret = ioctl(fd, CVM_IOC_SET_INTERRUPT_RXMAX, &rxmax_in_an_interrupt);
    if(ret == -1)
    {       
        close(fd);
        vty_out(vty,"%% IOCTL /dev/oct0 failed!\n");
        return CMD_WARNING;
    }
    else
    {       
        /*vty_out(vty,"%% Receive %d in an interrupt.\n", rxmax_in_an_interrupt);*/
    }
    
    close(fd);
    return CMD_SUCCESS;
}

    unsigned int dcli_intf_vlan_eth_port_interface_show_advanced_routing
    (
        struct vty * vty,
        unsigned int flag,
        unsigned int slot_no,
        unsigned int port_no,
        unsigned int vid
    )
    {
        DBusMessage *query, *reply;
        DBusError err;
        int ret = CMD_SUCCESS;
        unsigned int op_ret = INTERFACE_RETURN_CODE_ERROR;
        unsigned int isEnable = 0;
        if (0 == flag)
        {
            if (vid > 4094)
            {
                vty_out(vty,"%% Vlan id %d is out of range!\n",vid);
                return CMD_WARNING;
            }
        }
        if (flag > 2)
        {
            vty_out("%% Interface %s not support advaced-routing\n",vlan_eth_port_ifname);
            return CMD_WARNING;
        }

        query = dbus_message_new_method_call(
                    NPD_DBUS_BUSNAME,           \
                    NPD_DBUS_INTF_OBJPATH,          \
                    NPD_DBUS_INTF_INTERFACE,                    \
                    NPD_DBUS_INTF_METHOD_VLAN_ETH_PORT_INTERFACE_ADVANCED_ROUTING_SHOW);

        dbus_error_init(&err);

        dbus_message_append_args(query,
                                 DBUS_TYPE_UINT32,&flag,
                                 DBUS_TYPE_UINT32,&slot_no,
                                 DBUS_TYPE_UINT32,&port_no,
                                 DBUS_TYPE_UINT32,&vid,
                                 DBUS_TYPE_INVALID);

        reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);

        dbus_message_unref(query);

        if (NULL == reply)
        {
            vty_out(vty,"query reply null\n");
            vty_out(vty,"failed get reply.\n");
            if (dbus_error_is_set(&err))
            {
                vty_out(vty,"%s raised: %s",err.name,err.message);
                dbus_error_free_for_dcli(&err);
            }
            return CMD_WARNING;
        }
        if (dbus_message_get_args(reply, &err,
                                  DBUS_TYPE_UINT32,&op_ret,
                                  DBUS_TYPE_UINT32,&isEnable,
                                  DBUS_TYPE_INVALID))
        {
            if (INTERFACE_RETURN_CODE_SUCCESS != op_ret)
            {
                if (COMMON_RETURN_CODE_BADPARAM == op_ret)
                {
                    vty_out(vty,"%% Input bad parameter!\n");
                }
                else
                {
                    vty_out(vty,dcli_error_info_intf(op_ret));
                }
                ret = CMD_WARNING;
            }
            else
            {
                vty_out(vty," advanced-routing %s\n",isEnable ? "enabled":"disabled");
            }
            dbus_message_unref(reply);
            return ret;
        }
        else
        {
            vty_out(vty,"Failed get args.\n");
            if (dbus_error_is_set(&err))
            {
                vty_out(vty,"%s raised: %s",err.name,err.message);
                dbus_error_free_for_dcli(&err);
            }
        }
        dbus_message_unref(reply);
        return CMD_WARNING;
    }
#ifdef _D_WCPSS_

    DEFUN(interface_forward_mode_cmd_func,
          interface_forward_mode_cmd,
          "forward mode (bridge|route)",
          "config forward mode\n"
          "forward mode bridge\n"
          "forward mode route\n"
         )
    {
        int ret;
        unsigned int mode = 0;
        char ifName[INTERFACE_NAMSIZ+1] = {0};
        unsigned int flag = 2;

        /*To get ifname from vty*/
        memcpy((void *)ifName,(void *)vlan_eth_port_ifname,INTERFACE_NAMSIZ);

        if ((0 == strncmp(ifName,"radio",5)))
        {
            flag = 0;
        }		
        else if ((0 == strncmp(ifName,"r",1)))
        {
            flag = 0;
        }
        else
        {
            vty_out(vty,"%% forward mode unsupported, be sure interface is radio interface\n");
            return CMD_WARNING;
        }
        if (argc > 1)
        {
            vty_out(vty,"%% Input too many params!\n ");
            return CMD_WARNING;
        }

        /*printf(("before parse value %s\n",argv[0]));*/
        if (strncmp(argv[0] , "bridge",strlen(argv[0])) == 0)
        {
            mode = 1;
        }
        else if (strncmp(argv[0],"route",strlen(argv[0]))== 0)
        {
            mode = 2;
        }
        else
        {
            vty_out(vty,"%% Input  value must be bridge or route!\n");
            return CMD_WARNING;
        }
        if (0 == flag)
        {
            if (CMD_SUCCESS != dcli_forward_mode_config(vty,ifName,mode))
            {
                return CMD_WARNING;
            }
            else
            {
                return CMD_SUCCESS;
            }
        }
        vty_out(vty,"%% forward mode unsupported, be sure interface is radio interface!\n");
        return CMD_WARNING;

    }

    DEFUN(interface_tunnel_mode_cmd_func,
          interface_tunnel_mode_cmd,
		  "tunnel mode (capwap802dot11|capwap802dot3|ipip)",
          "config tunnel mode\n"
          "tunnel mode capwap\n"
          "tunnel mode ipip\n"
         )
    {
        int ret;
        unsigned int mode = 0;
        char ifName[INTERFACE_NAMSIZ+1] = {0};
        unsigned int flag = 2;
		unsigned int modeflag = 0;
		char nodeFlag = 0;
		unsigned int wlanid = 0;
        /*To get ifname from vty*/
        memcpy((void *)ifName,(void *)vlan_eth_port_ifname,INTERFACE_NAMSIZ);

        if ((0 == strncmp(ifName,"radio",5))||(0 == strncmp(ifName,"r",1)))
        {
            flag = 0;
			modeflag = 1;
		}else if(0 == strncmp(ifName,"wlan",4)){
			flag = 0;
			modeflag = 2;
        }
        else
        {
            vty_out(vty,"%% tunnel mode unsupported, be sure interface is radio interface\n");
            return CMD_WARNING;
        }
        if (argc > 1)
        {
            vty_out(vty,"%% Input too many params!\n ");
            return CMD_WARNING;
        }

        /*printf(("before parse value %s\n",argv[0]));*/
		if (strncmp(argv[0] , "capwap802dot3",strlen(argv[0])) == 0)
		{
			mode = 2;
		}
		else if (strncmp(argv[0] , "capwap802dot11",strlen(argv[0])) == 0)
        {
            mode = 4;
        }
        else if (strncmp(argv[0],"ipip",strlen(argv[0]))== 0)
        {
            mode = 8;
        }
        else
        {
            vty_out(vty,"%% Input  value must be capwap or ipip!\n");
            return CMD_WARNING;
        }
        if (0 == flag)
        {
            if (CMD_SUCCESS != dcli_tunnel_mode_config(vty,ifName,mode,modeflag,nodeFlag,wlanid))
            {
                return CMD_WARNING;
            }
            else
            {
				vty_out(vty,"set tunnel mode %s successfull!\n",argv[0]);
                return CMD_SUCCESS;
            }
        }
        vty_out(vty,"%% tunnel mode unsupported, be sure interface is radio interface!\n");
        return CMD_WARNING;


    }


DEFUN(interface_wlan_tunnel_mode_cmd_func,
	  interface_wlan_tunnel_mode_cmd,
	  "set wlan tunnel mode (capwap802dot11|capwap802dot3|ipip)",
	  "config tunnel mode\n"
	  "tunnel mode capwap\n"
	  "tunnel mode ipip\n"
	 )
{
	int ret;
	unsigned int mode = 0;
	unsigned int modeflag = 2;
	char nodeFlag = 2;
	unsigned int wtpindex = 0;
	unsigned int wlanid = 0;
	unsigned int radioid = 0;
	if (argc > 1)
	{
		vty_out(vty,"%% Input too many params!\n ");
		return CMD_WARNING;
	}

	if (strncmp(argv[0] , "capwap802dot3",strlen(argv[0])) == 0)
	{
		mode = 2;
	}
	else if (strncmp(argv[0] , "capwap802dot11",strlen(argv[0])) == 0)
	{
		mode = 4;
	}
	else if (strncmp(argv[0],"ipip",strlen(argv[0]))== 0)
	{
		mode = 8;
	}
	else
	{
		vty_out(vty,"%% Input  value must be capwap or ipip!\n");
		return CMD_WARNING;
	}
	
	if (CMD_SUCCESS != dcli_tunnel_mode_config(vty,NULL,mode,modeflag,nodeFlag,wlanid))
	{
		return CMD_WARNING;
	}
	else
	{
		vty_out(vty,"set tunnel mode %s successfull!\n",argv[0]);
		return CMD_SUCCESS;
	}
	return CMD_WARNING;


}

DEFUN(interface_radio_tunnel_mode_cmd_func,
	  interface_radio_tunnel_mode_cmd,
	  "wlan WLANID tunnel mode (capwap802dot11|capwap802dot3|ipip)",
	  "config tunnel mode\n"
	  "tunnel mode capwap\n"
	  "tunnel mode ipip\n"
	 )
{
	int ret;
	unsigned int mode = 0;
	unsigned int modeflag = 1;
	char nodeFlag = 1;
	unsigned int wtpindex = 0;
	unsigned int wlanid = 0;
	unsigned int radioid = 0;
	if (argc > 2)
	{
		vty_out(vty,"%% Input too many params!\n ");
		return CMD_WARNING;
	}
	ret = parse_int_ID((char*)argv[0], &wlanid);
	if (ret != WID_DBUS_SUCCESS)
	{	
       if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
       }
	   else{
			   vty_out(vty,"<error> input parameter %s error\n",argv[0]);
	   }
		
		return CMD_SUCCESS;
	}
	if(wlanid >= WLAN_NUM || wlanid == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}

	if (strncmp(argv[1] , "capwap802dot3",strlen(argv[1])) == 0)
	{
		mode = 2;
	}
	else if (strncmp(argv[1] , "capwap802dot11",strlen(argv[1])) == 0)
	{
		mode = 4;
	}
	else if (strncmp(argv[1],"ipip",strlen(argv[1]))== 0)
	{
		mode = 8;
	}
	else
	{
		vty_out(vty,"%% Input  value must be capwap or ipip!\n");
		return CMD_WARNING;
	}
	if (CMD_SUCCESS != dcli_tunnel_mode_config(vty,NULL,mode,modeflag,nodeFlag,wlanid))
	{
		return CMD_WARNING;
	}
	else
	{
		vty_out(vty,"set tunnel mode %s successfull!\n",argv[1]);
		return CMD_SUCCESS;
	}
	return CMD_WARNING;


}

#endif


    int dcli_advanced_routing_config(struct vty *vty,char * ifName,unsigned int isEnable)
    {
        unsigned int mode = 0;
        unsigned int flag = 2;
        unsigned char slot_no = 0,port_no =0;
        unsigned int vid = 0;

        if (1 == isEnable)
        {
            mode = ETH_PORT_FUNC_MODE_PROMISCUOUS;
        }
        else if (0 == isEnable)
        {
            mode = ETH_PORT_FUNC_IPV4;
        }
        else
        {
            return CMD_WARNING;
        }
        flag = parse_param_ifName(ifName,&slot_no,&port_no,&vid);

        if (0 == flag)
        {
            if (CMD_SUCCESS != dcli_vlan_interface_advanced_routing_enable(vty,(unsigned short)vid,isEnable))
            {
                return CMD_WARNING;
            }
        }
        else if (1 == flag)
        {
            if (0 == vid)
            {
                if (CMD_SUCCESS != dcli_eth_port_mode_config(vty,0,slot_no,port_no,mode))
                {/* for ifname like eth1-1*/
                    return CMD_WARNING;
                }
            }
            else
            {
                vty_out(vty,"%% Sub interface not support advanced-routing!\n");
                return CMD_WARNING;
            }
        }
        else if (2 == flag)
        {
            if (0 == vid)
            {/*for ifname like e1-1*/
                if (CMD_SUCCESS != dcli_eth_port_mode_config(vty,1,slot_no,port_no,mode))
                {
                    return CMD_WARNING;
                }
            }
            else
            {
                vty_out(vty,"%% Sub interface not support advanced-routing!\n");
                return CMD_WARNING;
            }
        }
        else
        {
            /* support not not ?*/
        }
        return CMD_SUCCESS;
    }

#if 1
    DEFUN(config_ip_static_arp_cmd_func,
          config_ip_static_arp_cmd,
          "ip static-arp PORTNO MAC A.B.C.D/M <1-4095>",
          "IP configuration\n"
          "Config static arp entry\n"
          CONFIG_ETHPORT_STR
          "Config MAC address format as HH:HH:HH:HH:HH:HH\n"
          "Config IP address format as <A.B.C.D/M>\n"
          "Config VLAN identifier in range <1-4095>\n"
         )
    {
        DBusMessage *query, *reply;
        DBusError err;
        /*variables*/
        unsigned int ret = 0;
        unsigned int int_slot_no = 0,int_port_no = 0;
        unsigned char slot_no = 0,port_no = 0;
        unsigned long ipno = 0;
        unsigned int ipmaskLen = 0;
        unsigned short vlanId = 0;
        ETHERADDR macAddr;

        memset(&macAddr,0,sizeof(ETHERADDR));
        /*parse argcs*/
        ret = parse_slotno_localport_include_slot0((char *)argv[0],&int_slot_no,&int_port_no);
        if (NPD_FAIL == ret)
        {
            vty_out(vty,"%% Bad parameter:Unknown port format.\n");
            return CMD_WARNING;
        }
        else if (1 == ret)
        {
            vty_out(vty,"%% Bad parameter:bad slot/port number.\n");
            return CMD_WARNING;
        }
        slot_no = (unsigned char)int_slot_no;
        port_no = (unsigned char)int_port_no;

        ret = parse_mac_addr((char *)argv[1],&macAddr);
        if (NPD_FAIL == ret)
        {
            vty_out(vty,"%% Unknow mac addr format!\n");
            return CMD_WARNING;

        }

        ret = is_muti_brc_mac(&macAddr);
        if (ret==1)
        {
            vty_out(vty,"%% Input broadcast or multicast mac!\n");
            return CMD_WARNING;
        }
        ret=ip_address_format2ulong((char **)&argv[2],&ipno,&ipmaskLen);
        if (CMD_WARNING == ret)
        {
            vty_out(vty, "%% Bad parameter %s\n", argv[2]);
            return CMD_WARNING;
        }
        IP_MASK_CHECK(ipmaskLen);
        if (32 != ipmaskLen)
        {
            vty_out(vty,"%% Ip address mask must be 32!\n");
            return CMD_WARNING;
        }
        if (argc > 3)
        {
            ret = parse_short_parse((char*)argv[3], &vlanId);
            if (NPD_FAIL == ret)
            {
                vty_out(vty,"%% Unknow vlan id format!\n");
                return CMD_WARNING;
            }
            if (vlanId <MIN_VLANID||vlanId>MAX_L3INTF_VLANID)
            {
                vty_out(vty,"%% Vlan out of range!\n");
                return CMD_WARNING;
            }
        }

        /*appends and gets*/
        query = dbus_message_new_method_call(\
                                             NPD_DBUS_BUSNAME,                   \
                                             NPD_DBUS_INTF_OBJPATH,      \
                                             NPD_DBUS_INTF_INTERFACE,        \
                                             NPD_DBUS_INTERFACE_IP_STATIC_ARP);

        dbus_error_init(&err);

        dbus_message_append_args(query,
                                 DBUS_TYPE_BYTE,&slot_no,
                                 DBUS_TYPE_BYTE,&port_no,
                                 DBUS_TYPE_BYTE,&macAddr.arEther[0],
                                 DBUS_TYPE_BYTE,&macAddr.arEther[1],
                                 DBUS_TYPE_BYTE,&macAddr.arEther[2],
                                 DBUS_TYPE_BYTE,&macAddr.arEther[3],
                                 DBUS_TYPE_BYTE,&macAddr.arEther[4],
                                 DBUS_TYPE_BYTE,&macAddr.arEther[5],
                                 DBUS_TYPE_UINT32,&ipno,
                                 DBUS_TYPE_UINT32,&ipmaskLen,
                                 DBUS_TYPE_UINT32,&vlanId,
                                 DBUS_TYPE_INVALID);

        reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);

        dbus_message_unref(query);

        if (NULL == reply)
        {
            vty_out(vty,"failed get reply.\n");
            if (dbus_error_is_set(&err))
            {
                vty_out(vty,"%s raised: %s",err.name,err.message);
                dbus_error_free_for_dcli(&err);
            }
            return CMD_SUCCESS;
        }

        DCLI_DEBUG(("query reply not null\n"));
        if (dbus_message_get_args(reply, &err,
                                  DBUS_TYPE_UINT32,&ret,
                                  DBUS_TYPE_INVALID))
        {
            if ((INTERFACE_RETURN_CODE_SUCCESS != ret)&&\
                (ARP_RETURN_CODE_SUCCESS != ret))
            {
                if(ARP_RETURN_CODE_UNSUPPORTED_COMMAND == ret){
                    vty_out(vty,"%% This command not support vlan advanced-routing interface or sub interface!\n");
                }
                else{
                    vty_out(vty,dcli_error_info_intf(ret));
                }
            }

        }
        else
        {
            vty_out(vty,"Failed get args.\n");
            if (dbus_error_is_set(&err))
            {
                vty_out(vty,"%s raised: %s",err.name,err.message);
                dbus_error_free_for_dcli(&err);
            }
        }
        dbus_message_unref(reply);

        return CMD_SUCCESS;
    }
    ALIAS(config_ip_static_arp_cmd_func,
          config_ip_static_arp_for_rgmii_cmd,
          "ip static-arp PORTNO MAC A.B.C.D/M",
          "IP configuration\n"
          "Config static arp entry\n"
          CONFIG_ETHPORT_STR
          "Config MAC address format as HH:HH:HH:HH:HH:HH\n"
          "Config IP address format as <A.B.C.D/M>\n"
         );
#if 0
    DEFUN(config_ip_static_arp_for_rgmii_cmd_func,
          config_ip_static_arp_for_rgmii_cmd,
          "ip static-arp PORTNO MAC A.B.C.D/M",
          "IP configuration\n"
          "Config static arp entry\n"
          CONFIG_ETHPORT_STR
          "Config MAC address format as HH:HH:HH:HH:HH:HH\n"
          "Config IP address format as <A.B.C.D/M>\n"
         )
    {
        DBusMessage *query, *reply;
        DBusError err;
        /*variables*/
        unsigned int ret = 0;
        unsigned int int_slot_no = 0,int_port_no = 0;
        unsigned char slot_no = 0,port_no = 0;
        unsigned long ipno = 0;
        unsigned int ipmaskLen = 0;
        unsigned short vlanId = 0;
        ETHERADDR macAddr;

        memset(&macAddr,0,sizeof(ETHERADDR));
        /*parse argcs*/
        ret = parse_slotno_localport_include_slot0((char *)argv[0],&int_slot_no,&int_port_no);
        if (NPD_FAIL == ret)
        {
            vty_out(vty,"%% Bad parameter:Unknown portno format!\n");
            return CMD_WARNING;
        }
        else if (1 == ret)
        {
		vty_out(vty,"%% Bad parameter: bad slot/port %s number!\n",argv[0]);
            return CMD_WARNING;
        }
        slot_no = (unsigned char)int_slot_no;
        port_no = (unsigned char)int_port_no;

        ret = parse_mac_addr((char *)argv[1],&macAddr);
        if (NPD_FAIL == ret)
        {
            vty_out(vty,"%% Unknow mac addr format!\n");
            return CMD_WARNING;

        }

        ret = is_muti_brc_mac(&macAddr);
        if (ret==1)
        {
            vty_out(vty,"%% Input broadcast or multicast mac!\n");
            return CMD_WARNING;
        }
        ret=ip_address_format2ulong((char **)&argv[2],&ipno,&ipmaskLen);
        if (CMD_WARNING == ret)
        {
            vty_out(vty, "%% Bad parameter %s\n", argv[2]);
            return CMD_WARNING;
        }
        IP_MASK_CHECK(ipmaskLen);
        if (32 != ipmaskLen)
        {
            vty_out(vty,"%% Ip address mask must be 32!\n");
            return CMD_WARNING;
        }

        /*appends and gets*/
        query = dbus_message_new_method_call(\
                                             NPD_DBUS_BUSNAME,                   \
                                             NPD_DBUS_INTF_OBJPATH,      \
                                             NPD_DBUS_INTF_INTERFACE,        \
                                             NPD_DBUS_INTERFACE_IP_STATIC_ARP);

        dbus_error_init(&err);

        dbus_message_append_args(query,
                                 DBUS_TYPE_BYTE,&slot_no,
                                 DBUS_TYPE_BYTE,&port_no,
                                 DBUS_TYPE_BYTE,&macAddr.arEther[0],
                                 DBUS_TYPE_BYTE,&macAddr.arEther[1],
                                 DBUS_TYPE_BYTE,&macAddr.arEther[2],
                                 DBUS_TYPE_BYTE,&macAddr.arEther[3],
                                 DBUS_TYPE_BYTE,&macAddr.arEther[4],
                                 DBUS_TYPE_BYTE,&macAddr.arEther[5],
                                 DBUS_TYPE_UINT32,&ipno,
                                 DBUS_TYPE_UINT32,&ipmaskLen,
                                 DBUS_TYPE_UINT32,&vlanId,
                                 DBUS_TYPE_INVALID);

        reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);

        dbus_message_unref(query);

        if (NULL == reply)
        {
            vty_out(vty,"failed get reply.\n");
            if (dbus_error_is_set(&err))
            {
                vty_out(vty,"%s raised: %s",err.name,err.message);
                dbus_error_free_for_dcli(&err);
            }
            return CMD_SUCCESS;
        }

        DCLI_DEBUG(("query reply not null\n"));
        if (dbus_message_get_args(reply, &err,
                                  DBUS_TYPE_UINT32,&ret,
                                  DBUS_TYPE_INVALID))
        {
            if ((INTERFACE_RETURN_CODE_SUCCESS != ret)&&\
                (ARP_RETURN_CODE_SUCCESS != ret))
            {
                vty_out(vty,dcli_error_info_intf(ret));
            }

        }
        else
        {
            vty_out(vty,"Failed get args.\n");
            if (dbus_error_is_set(&err))
            {
                vty_out(vty,"%s raised: %s",err.name,err.message);
                dbus_error_free_for_dcli(&err);
            }
        }
        dbus_message_unref(reply);

        return CMD_SUCCESS;
    }
#endif

    DEFUN(config_no_ip_static_arp_cmd_func,
          config_no_ip_static_arp_cmd,
          "no ip static-arp PORTNO MAC A.B.C.D/M <1-4095>",
          NO_STR
          "IP configuration\n"
          "Config static arp entry\n"
          CONFIG_ETHPORT_STR
          "Config MAC address format as HH:HH:HH:HH:HH:HH\n"
          "Config IP address format as <A.B.C.D/M>\n"
          "Config VLAN identifier in range <1-4095>\n"
         )
    {
        DBusMessage *query, *reply;
        DBusError err;
        /*variables*/
        unsigned int ret = 0;
        unsigned int int_slot_no = 0,int_port_no = 0;
        unsigned char slot_no = 0,port_no = 0;
        unsigned long ipno = 0;
        unsigned int ipmaskLen = 0;
        unsigned short vlanId = 0;
        ETHERADDR macAddr;

        memset(&macAddr,0,sizeof(ETHERADDR));
        /*parse argcs*/
        ret = parse_slotno_localport_include_slot0((char *)argv[0],&int_slot_no,&int_port_no);
        if (NPD_FAIL == ret)
        {
            vty_out(vty,"%% Bad parameter:unknow portno format!\n");
            return CMD_WARNING;
        }
        else if (1 == ret)
        {
            vty_out(vty,"%% Bad parameter: bad slot/port %s number!\n",argv[0]);
            return CMD_WARNING;
        }
        slot_no = (unsigned char)int_slot_no;
        port_no = (unsigned char)int_port_no;

        ret = parse_mac_addr((char *)argv[1],&macAddr);
        if (NPD_FAIL == ret)
        {
            vty_out(vty,"%% Unknown mac addr format!\n");
		return CMD_WARNING;

        }
        ret = is_muti_brc_mac(&macAddr);
        if (ret==1)
        {
            vty_out(vty,"%% Input broadcast or multicast mac!\n");
            return CMD_WARNING;
        }

        ret=ip_address_format2ulong((char**)&argv[2],&ipno,&ipmaskLen);
        if (CMD_WARNING == ret)
        {
            vty_out(vty, "%% Bad parameter %s\n", argv[2]);
            return CMD_WARNING;
        }
        IP_MASK_CHECK(ipmaskLen);
        if (32 != ipmaskLen)
        {
            vty_out(vty,"%% Ip address mask must be 32!\n");
            return CMD_WARNING;
        }

        if (argc > 3)
        {
            ret = parse_short_parse((char*)argv[3], &vlanId);
            if (NPD_FAIL == ret)
            {
                vty_out(vty,"%% Unknown vlan id format!\n");
                return CMD_WARNING;
            }
            if (vlanId <MIN_VLANID||vlanId>MAX_L3INTF_VLANID)
            {
                vty_out(vty,"%% Vlan out of range!\n");
                return CMD_WARNING;
            }
        }
        /*appends and gets*/
        query = dbus_message_new_method_call(\
                                             NPD_DBUS_BUSNAME,                   \
                                             NPD_DBUS_INTF_OBJPATH,      \
                                             NPD_DBUS_INTF_INTERFACE,        \
                                             NPD_DBUS_INTERFACE_NO_IP_STATIC_ARP);

        dbus_error_init(&err);

        dbus_message_append_args(query,
                                 DBUS_TYPE_BYTE,&slot_no,
                                 DBUS_TYPE_BYTE,&port_no,
                                 DBUS_TYPE_BYTE,&macAddr.arEther[0],
                                 DBUS_TYPE_BYTE,&macAddr.arEther[1],
                                 DBUS_TYPE_BYTE,&macAddr.arEther[2],
                                 DBUS_TYPE_BYTE,&macAddr.arEther[3],
                                 DBUS_TYPE_BYTE,&macAddr.arEther[4],
                                 DBUS_TYPE_BYTE,&macAddr.arEther[5],
                                 DBUS_TYPE_UINT32,&ipno,
                                 DBUS_TYPE_UINT32,&ipmaskLen,
                                 DBUS_TYPE_UINT32,&vlanId,
                                 DBUS_TYPE_INVALID);

        reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);

        dbus_message_unref(query);

        if (NULL == reply)
        {
            vty_out(vty,"failed get reply.\n");
            if (dbus_error_is_set(&err))
            {
                vty_out(vty,"%s raised: %s",err.name,err.message);
                dbus_error_free_for_dcli(&err);
            }
            return CMD_SUCCESS;
        }

        DCLI_DEBUG(("query reply not null\n"));
        if (dbus_message_get_args(reply, &err,
                                  DBUS_TYPE_UINT32,&ret,
                                  DBUS_TYPE_INVALID))
        {
            if ((INTERFACE_RETURN_CODE_SUCCESS != ret)&&\
                (ARP_RETURN_CODE_SUCCESS != ret))
            {
                vty_out(vty,"Err%d:%s",ret,dcli_error_info_intf(ret));
            }
        }
        else
        {
            vty_out(vty,"Failed get args.\n");
            if (dbus_error_is_set(&err))
            {
                vty_out(vty,"%s raised: %s",err.name,err.message);
                dbus_error_free_for_dcli(&err);
            }
        }
        dbus_message_unref(reply);

        return CMD_SUCCESS;
    }
    ALIAS(config_no_ip_static_arp_cmd_func,
          config_no_ip_static_arp_for_rgmii_cmd,
          "no ip static-arp PORTNO MAC A.B.C.D/M",
          NO_STR
          "IP configuration\n"
          "Config static arp entry\n"
          CONFIG_ETHPORT_STR
          "Config MAC address format as HH:HH:HH:HH:HH:HH\n"
          "Config IP address format as <A.B.C.D/M>\n"
         );
#if 0
    DEFUN(config_no_ip_static_arp_for_rgmii_cmd_func,
          config_no_ip_static_arp_for_rgmii_cmd,
          "no ip static-arp PORTNO MAC A.B.C.D/M",
          NO_STR
          "IP configuration\n"
          "Config static arp entry\n"
          CONFIG_ETHPORT_STR
          "Config MAC address format as HH:HH:HH:HH:HH:HH\n"
          "Config IP address format as <A.B.C.D/M>\n"
         )
    {
        DBusMessage *query, *reply;
        DBusError err;
        /*variables*/
        unsigned int ret = 0;
        unsigned int int_slot_no = 0,int_port_no = 0;
        unsigned char slot_no = 0,port_no = 0;
        unsigned long ipno = 0;
        unsigned int ipmaskLen = 0;
        unsigned short vlanId = 0;
        ETHERADDR macAddr;

        memset(&macAddr,0,sizeof(ETHERADDR));
        /*parse argcs*/
        ret = parse_slotno_localport_include_slot0((char *)argv[0],&int_slot_no,&int_port_no);
        if (NPD_FAIL == ret)
        {
            vty_out(vty,"%% Bad parameter:unknow portno format!\n");
            return CMD_WARNING;
        }
        else if (1 == ret)
        {
            vty_out(vty,"%% Bad parameter: bad slot/port %s number or bad vlan id!\n",argv[0]);
            return CMD_WARNING;
        }
        slot_no = (unsigned char)int_slot_no;
        port_no = (unsigned char)int_port_no;

        ret = parse_mac_addr((char *)argv[1],&macAddr);
        if (NPD_FAIL == ret)
        {
            vty_out(vty,"%% Unknown mac addr format!\n");
            return CMD_SUCCESS;

        }
        ret = is_muti_brc_mac(&macAddr);
        if (ret==1)
        {
            vty_out(vty,"%% Input broadcast or multicast mac!\n");
            return CMD_WARNING;
        }

        ret=ip_address_format2ulong((char**)&argv[2],&ipno,&ipmaskLen);
        if (CMD_WARNING == ret)
        {
            vty_out(vty, "%% Bad parameter %s\n", argv[2]);
            return CMD_WARNING;
        }
        IP_MASK_CHECK(ipmaskLen);
        if (32 != ipmaskLen)
        {
            vty_out(vty,"%% Ip address mask must be 32!\n");
            return CMD_WARNING;
        }


        /*appends and gets*/
        query = dbus_message_new_method_call(\
                                             NPD_DBUS_BUSNAME,                   \
                                             NPD_DBUS_INTF_OBJPATH,      \
                                             NPD_DBUS_INTF_INTERFACE,        \
                                             NPD_DBUS_INTERFACE_NO_IP_STATIC_ARP);

        dbus_error_init(&err);

        dbus_message_append_args(query,
                                 DBUS_TYPE_BYTE,&slot_no,
                                 DBUS_TYPE_BYTE,&port_no,
                                 DBUS_TYPE_BYTE,&macAddr.arEther[0],
                                 DBUS_TYPE_BYTE,&macAddr.arEther[1],
                                 DBUS_TYPE_BYTE,&macAddr.arEther[2],
                                 DBUS_TYPE_BYTE,&macAddr.arEther[3],
                                 DBUS_TYPE_BYTE,&macAddr.arEther[4],
                                 DBUS_TYPE_BYTE,&macAddr.arEther[5],
                                 DBUS_TYPE_UINT32,&ipno,
                                 DBUS_TYPE_UINT32,&ipmaskLen,
                                 DBUS_TYPE_UINT32,&vlanId,
                                 DBUS_TYPE_INVALID);

        reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);

        dbus_message_unref(query);

        if (NULL == reply)
        {
            vty_out(vty,"failed get reply.\n");
            if (dbus_error_is_set(&err))
            {
                vty_out(vty,"%s raised: %s",err.name,err.message);
                dbus_error_free_for_dcli(&err);
            }
            return CMD_SUCCESS;
        }

        DCLI_DEBUG(("query reply not null\n"));
        if (dbus_message_get_args(reply, &err,
                                  DBUS_TYPE_UINT32,&ret,
                                  DBUS_TYPE_INVALID))
        {
            if ((INTERFACE_RETURN_CODE_SUCCESS != ret)&&\
                (ARP_RETURN_CODE_SUCCESS != ret))
            {
                vty_out(vty,dcli_error_info_intf(ret));
            }
        }
        else
        {
            vty_out(vty,"Failed get args.\n");
            if (dbus_error_is_set(&err))
            {
                vty_out(vty,"%s raised: %s",err.name,err.message);
                dbus_error_free_for_dcli(&err);
            }
        }
        dbus_message_unref(reply);

        return CMD_SUCCESS;
    }
#endif

    DEFUN(config_ip_static_arp_trunk_cmd_func,
          config_ip_static_arp_trunk_cmd,
          "ip static-arp <1-127> MAC A.B.C.D/M <1-4095>",
          "IP configuration\n"
          "Config static arp entry\n"
          "Config trunk number in range <1-127>\n"
          "Config MAC address format as HH:HH:HH:HH:HH:HH\n"
          "Config IP address format as <A.B.C.D/M>\n"
          "Config VLAN identifier in range <1-4095>\n"
         )
    {
        DBusMessage *query, *reply;
        DBusError err;
        /*variables*/
        unsigned int ret = 0;
        unsigned int trunkId = 0;
        unsigned long ipno = 0;
        unsigned int ipmaskLen = 0;
        unsigned short vlanId = 0;
        ETHERADDR macAddr;

        memset(&macAddr,0,sizeof(ETHERADDR));
        /*parse argcs*/
        if (4 != argc)
        {
            vty_out(vty,"%% Bad parameter:Wrong parameter count\n");
        }
        trunkId = strtoul(argv[0],NULL,NULL);

        ret = parse_mac_addr((char *)argv[1],&macAddr);
        if (NPD_FAIL == ret)
        {
            vty_out(vty,"%% Unknow mac addr format!\n");
            return CMD_WARNING;

        }

        ret = is_muti_brc_mac(&macAddr);
        if (ret == 1)
        {
            vty_out(vty,"%% Input broadcast or multicast mac!\n");
            return CMD_WARNING;
        }
        ret = ip_address_format2ulong((char **)&argv[2],&ipno,&ipmaskLen);
        if (CMD_WARNING == ret)
        {
            vty_out(vty, "%% Bad parameter %s\n", argv[2]);
            return CMD_WARNING;
        }
        IP_MASK_CHECK(ipmaskLen);
        if (32 != ipmaskLen)
        {
            vty_out(vty,"%% Ip address mask must be 32!\n");
            return CMD_WARNING;
        }
        ret = parse_short_parse((char*)argv[3], &vlanId);
        if (NPD_FAIL == ret)
        {
            vty_out(vty,"%% Unknow vlan id format!\n");
            return CMD_WARNING;
        }
        if (vlanId <MIN_VLANID||vlanId>MAX_L3INTF_VLANID)
        {
            vty_out(vty,"%% Vlan out of rang!\n");
            return CMD_WARNING;
        }

        /*appends and gets*/
        query = dbus_message_new_method_call(\
                                             NPD_DBUS_BUSNAME,                   \
                                             NPD_DBUS_INTF_OBJPATH,      \
                                             NPD_DBUS_INTF_INTERFACE,        \
                                             NPD_DBUS_INTERFACE_IP_STATIC_ARP_FOR_TRUNK);

        dbus_error_init(&err);

        dbus_message_append_args(query,
                                 DBUS_TYPE_UINT32,&trunkId,
                                 DBUS_TYPE_BYTE,&macAddr.arEther[0],
                                 DBUS_TYPE_BYTE,&macAddr.arEther[1],
                                 DBUS_TYPE_BYTE,&macAddr.arEther[2],
                                 DBUS_TYPE_BYTE,&macAddr.arEther[3],
                                 DBUS_TYPE_BYTE,&macAddr.arEther[4],
                                 DBUS_TYPE_BYTE,&macAddr.arEther[5],
                                 DBUS_TYPE_UINT32,&ipno,
                                 DBUS_TYPE_UINT32,&ipmaskLen,
                                 DBUS_TYPE_UINT32,&vlanId,
                                 DBUS_TYPE_INVALID);

        reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);

        dbus_message_unref(query);

        if (NULL == reply)
        {
            vty_out(vty,"failed get reply.\n");
            if (dbus_error_is_set(&err))
            {
                vty_out(vty,"%s raised: %s",err.name,err.message);
                dbus_error_free_for_dcli(&err);
            }
            return CMD_SUCCESS;
        }

        DCLI_DEBUG(("query reply not null\n"));
        if (dbus_message_get_args(reply, &err,
                                  DBUS_TYPE_UINT32,&ret,
                                  DBUS_TYPE_INVALID))
        {
            if ((INTERFACE_RETURN_CODE_SUCCESS != ret)&&\
                (ARP_RETURN_CODE_SUCCESS != ret))
            {
                vty_out(vty,dcli_error_info_intf(ret));
            }

        }
        else
        {
            vty_out(vty,"Failed get args.\n");
            if (dbus_error_is_set(&err))
            {
                vty_out(vty,"%s raised: %s",err.name,err.message);
                dbus_error_free_for_dcli(&err);
            }
        }
        dbus_message_unref(reply);

        return CMD_SUCCESS;
    }

    DEFUN(config_no_ip_static_arp_trunk_cmd_func,
          config_no_ip_static_arp_trunk_cmd,
          "no ip static-arp <1-127> MAC A.B.C.D/M <1-4095>",
          NO_STR
          "IP configuration\n"
          "Config static arp entry\n"
          "Config trunk number in range <1-127>\n"
          "Config MAC address format as HH:HH:HH:HH:HH:HH\n"
          "Config IP address format as <A.B.C.D/M>\n"
          "Config VLAN identifier in range <1-4095>\n"
         )
    {
        DBusMessage *query, *reply;
        DBusError err;
        /*variables*/
        unsigned int ret = 0;
        unsigned int trunkId = 0;
        unsigned long ipno = 0;
        unsigned int ipmaskLen = 0;
        unsigned short vlanId = 0;
        ETHERADDR macAddr;

        memset(&macAddr,0,sizeof(ETHERADDR));
        /*parse argcs*/
        if (4 != argc)
        {
            vty_out(vty,"%% Bad parameter:Wrong parameter count\n");
        }
        trunkId = strtoul(argv[0],NULL,NULL);
        if ((trunkId > 127)||(trunkId < 1))
        {
            vty_out("%% Bad parameter : trunk id %d \n",trunkId);
            return CMD_WARNING;
        }
        ret = parse_mac_addr((char *)argv[1],&macAddr);
        if (NPD_FAIL == ret)
        {
            vty_out(vty,"%% Unknow mac addr format!\n");
            return CMD_WARNING;

        }

        ret = is_muti_brc_mac(&macAddr);
        if (ret == 1)
        {
            vty_out(vty,"%% Input broadcast or multicast mac!\n");
            return CMD_WARNING;
        }
        ret = ip_address_format2ulong((char **)&argv[2],&ipno,&ipmaskLen);
        if (CMD_WARNING == ret)
        {
            vty_out(vty, "%% Bad parameter %s\n", argv[2]);
            return CMD_WARNING;
        }
        IP_MASK_CHECK(ipmaskLen);
        if (32 != ipmaskLen)
        {
            vty_out(vty,"%% Ip address mask must be 32!\n");
            return CMD_WARNING;
        }
        ret = parse_short_parse((char*)argv[3], &vlanId);
        if (NPD_FAIL == ret)
        {
            vty_out(vty,"%% Unknow vlan id format!\n");
            return CMD_WARNING;
        }
        if (vlanId <MIN_VLANID||vlanId>MAX_L3INTF_VLANID)
        {
            vty_out(vty,"%% Vlan out of rang!\n");
            return CMD_WARNING;
        }

        /*appends and gets*/
        query = dbus_message_new_method_call(\
                                             NPD_DBUS_BUSNAME,                   \
                                             NPD_DBUS_INTF_OBJPATH,      \
                                             NPD_DBUS_INTF_INTERFACE,        \
                                             NPD_DBUS_INTERFACE_NO_IP_STATIC_ARP_FOR_TRUNK);

        dbus_error_init(&err);

        dbus_message_append_args(query,
                                 DBUS_TYPE_UINT32,&trunkId,
                                 DBUS_TYPE_BYTE,&macAddr.arEther[0],
                                 DBUS_TYPE_BYTE,&macAddr.arEther[1],
                                 DBUS_TYPE_BYTE,&macAddr.arEther[2],
                                 DBUS_TYPE_BYTE,&macAddr.arEther[3],
                                 DBUS_TYPE_BYTE,&macAddr.arEther[4],
                                 DBUS_TYPE_BYTE,&macAddr.arEther[5],
                                 DBUS_TYPE_UINT32,&ipno,
                                 DBUS_TYPE_UINT32,&ipmaskLen,
                                 DBUS_TYPE_UINT32,&vlanId,
                                 DBUS_TYPE_INVALID);

        reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);

        dbus_message_unref(query);

        if (NULL == reply)
        {
            vty_out(vty,"failed get reply.\n");
            if (dbus_error_is_set(&err))
            {
                vty_out(vty,"%s raised: %s",err.name,err.message);
                dbus_error_free_for_dcli(&err);
            }
            return CMD_SUCCESS;
        }

        DCLI_DEBUG(("query reply not null\n"));
        if (dbus_message_get_args(reply, &err,
                                  DBUS_TYPE_UINT32,&ret,
                                  DBUS_TYPE_INVALID))
        {
            if ((INTERFACE_RETURN_CODE_SUCCESS != ret)&&\
                (ARP_RETURN_CODE_SUCCESS != ret))
            {
                vty_out(vty,dcli_error_info_intf(ret));
            }

        }
        else
        {
            vty_out(vty,"Failed get args.\n");
            if (dbus_error_is_set(&err))
            {
                vty_out(vty,"%s raised: %s",err.name,err.message);
                dbus_error_free_for_dcli(&err);
            }
        }
        dbus_message_unref(reply);

        return CMD_SUCCESS;
    }

    int dcli_ip_arp_show_running_config()
    {
        char *showStr = NULL;
        DBusMessage *query = NULL, *reply = NULL;
        DBusError err;
        int ret = 1;
        query = dbus_message_new_method_call(
                    NPD_DBUS_BUSNAME,                   \
                    NPD_DBUS_INTF_OBJPATH,      \
                    NPD_DBUS_INTF_INTERFACE,    \
                    NPD_DBUS_STATIC_ARP_METHOD_SHOW_RUNNING_CONFIG);

        dbus_error_init(&err);

        reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);

        dbus_message_unref(query);
        if (NULL == reply)
        {
		printf("show ip_arp running config failed get reply.\n");
            if (dbus_error_is_set(&err))
            {
                printf("%s raised: %s",err.name,err.message);
                dbus_error_free_for_dcli(&err);
            }
            return ret;
        }

        if (dbus_message_get_args(reply, &err,
                                  DBUS_TYPE_STRING, &showStr,
                                  DBUS_TYPE_INVALID))
        {

            char _tmpstr[64];
            memset(_tmpstr,0,64);
            sprintf(_tmpstr,BUILDING_MOUDLE,"STATIC ARP");
            vtysh_add_show_string(_tmpstr);
            vtysh_add_show_string(showStr);
            ret = 0;
        }
        else
        {
            printf("Failed get args.\n");
            if (dbus_error_is_set(&err))
            {
                printf("%s raised: %s",err.name,err.message);
                dbus_error_free_for_dcli(&err);
            }
        }

        dbus_message_unref(reply);
        /*printf("%s",showStr);*/
        return ret;
    }

#endif


int dcli_interrupt_rxmax_show_running_config()
   {
	   char *showStr = NULL;
	   DBusMessage *query = NULL, *reply = NULL;
	   DBusError err;
	   int ret = 1;
	   query = dbus_message_new_method_call(
				   NPD_DBUS_BUSNAME,				   \
				   NPD_DBUS_INTF_OBJPATH,	   \
				   NPD_DBUS_INTF_INTERFACE,    \
				   NPD_DBUS_INTERRUPT_RXMAX_SHOW_RUNNING_CONFIG);

	   dbus_error_init(&err);

	   dbus_message_append_args(query,		
								DBUS_TYPE_UINT32,&interrupt_rxmax,
							 	DBUS_TYPE_INVALID);


	   reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);

	   dbus_message_unref(query);
	   if (NULL == reply)
	   {
	   printf("show ip_arp running config failed get reply.\n");
		   if (dbus_error_is_set(&err))
		   {
			   printf("%s raised: %s",err.name,err.message);
			   dbus_error_free_for_dcli(&err);
		   }
		   return ret;
	   }

	   if (dbus_message_get_args(reply, &err,
								 DBUS_TYPE_STRING, &showStr,
								 DBUS_TYPE_INVALID))
	   {

		   char _tmpstr[64];
		   memset(_tmpstr,0,64);
		   sprintf(_tmpstr,BUILDING_MOUDLE,"INTERRUPT RXMAX");
		   vtysh_add_show_string(_tmpstr);
		   vtysh_add_show_string(showStr);
		   ret = 0;
	   }
	   else
	   {
		   printf("Failed get args.\n");
		   if (dbus_error_is_set(&err))
		   {
			   printf("%s raised: %s",err.name,err.message);
			   dbus_error_free_for_dcli(&err);
		   }
	   }

	   dbus_message_unref(reply);
	   /*printf("%s",showStr);*/
	   return ret;
   }

/*wangchao add*/

DEFUN( config_dynamic_arp_cmd_func,
        config_dynamic_arp_cmd,
        "no dynamic-arp A.B.C.D",
        NO_STR
        "Delete dynamic arp entry\n"
        "Config IP address format as <A.B.C.D>\n"
       )
{
	 DBusMessage *query, *reply;
	 DBusError err;

	 int ret, ret2; 
	 int i;
	 unsigned long ipno = 0;
	 char cmdstr[128];
	 memset(cmdstr,0,128);
	 if(CMD_SUCCESS != parse_ip_check((char *)argv[0])){
          vty_out(vty,"%% Bad parameter: %s\n",(char *)argv[0]);
          return CMD_WARNING;
      }

	  ipno = dcli_ip2ulong((char *)argv[0]);
	 /* sprintf(cmdstr,"/usr/sbin/arp -d %d.%d.%d.%d\n",(ipno>>24)&0xFF,(ipno>>16)&0xFF,(ipno>>8)&0xFF,ipno&0xFF);
	      vty_out(vty,"/usr/sbin/arp -d %d.%d.%d.%d\n",(ipno>>24)&0xFF,(ipno>>16)&0xFF,(ipno>>8)&0xFF,ipno&0xFF);*/
      query = dbus_message_new_method_call(\
                                           NPD_DBUS_BUSNAME,     \
                                           NPD_DBUS_INTF_OBJPATH,   \
                                           NPD_DBUS_INTF_INTERFACE,  \
                                           NPD_DBUS_DYNAMIC_ARP);

      dbus_error_init(&err);
      dbus_message_append_args(query,
                               DBUS_TYPE_UINT32,&ipno,
                               DBUS_TYPE_INVALID);
	
	 for ( i = 0; i< MAX_SLOT; i++) {
          
	  	  if (NULL != dbus_connection_dcli[i]->dcli_dbus_connection) 
	  	   {
         		reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[i]->dcli_dbus_connection,query,-1, &err);
         		if (NULL == reply)
         		{
            		 vty_out(vty,"failed get reply.\n");
             		 if (dbus_error_is_set(&err))
            		 {
                 		vty_out(vty,"%s raised: %s",err.name,err.message);
                 		dbus_error_free_for_dcli(&err);
            		 }
             		  //return CMD_SUCCESS;
             		  continue;
         		 }
         		 DCLI_DEBUG(("query reply not null\n"));
            	 if (dbus_message_get_args(reply, &err,
                                DBUS_TYPE_UINT32,&ret,
                                DBUS_TYPE_INVALID))
            	 {    
            	     vty_out(vty,"In slot %d : ",i);        	 
                 	 if (ARP_RETURN_CODE_SUCCESS == ret)
                 	 {
                 	 	 vty_out(vty,"Arp return code success\n");	
                 	 }else {
						 vty_out(vty,dcli_error_info_intf(ret));     
					 }
             
            	 }
                 else
                 {
                     vty_out(vty,"Failed get args.\n");
                     if (dbus_error_is_set(&err))
                     {
                         vty_out(vty,"%s raised: %s",err.name,err.message);
                         dbus_error_free_for_dcli(&err);
                     }
                 }
         		 dbus_message_unref(reply);
         
	  	  	}	 
	}
	  dbus_message_unref(query);
	  return CMD_SUCCESS;
}

int parse_wlan_ebr_no(char *str,unsigned char *slotno,unsigned char *portno,unsigned short * vlanid)
{
	char *endptr = NULL;
	char *endptr2 = NULL;
	char *endptr3 = NULL;
	char *endptr4 = NULL;
	char * tmpstr = str;
	
	if(NULL == str){
		return NPD_FAIL;
	}
	if((NULL == slotno)||(NULL == portno)){
		return -1;
	}
	*slotno = 0;
	*portno = 0;
	*vlanid= 0;
	if(!strncmp(str,"ebr",3)){
		tmpstr = str+3;
	}
	if(!strncmp(str,"ebrl",4) || !strncmp(str,"wlan",4)){
		tmpstr = str + 4;
	} 
	if (!strncmp(str,"wlanl",5)){
		tmpstr = str + 5;
	}
	
	if (NULL == tmpstr) {return -1;}
	
	*portno = (char)strtoul(tmpstr,&endptr,10);
	if (endptr) {
		if (SLOT_PORT_SPLIT_DASH == endptr[0]){
            *slotno = *portno;
			*portno = (char)strtoul((char *)&(endptr[1]),&endptr2,10);
			
			if (endptr2){
				
				if ((SLOT_PORT_SPLIT_DASH == endptr2[0])) {
					*vlanid = (unsigned short)strtoul((char*)&(endptr2[1]),NULL,10);
					return 0;
				}
			}else {
				return 0;
			}
		} 
	}
	
	return -1;
}
#define IS_ETH_PORT_INTERFACE 	0
#define IS_VLAN_INTERFACE 		1
#define IS_VLAN_ADVANCED 		2
/*wangchao add*/
#define IS_MNG_INTERFACE 		3
#define IS_VE_INTERFACE 		4
#define IS_EBR_INTERFACE 		5
#define IS_EBRL_INTERFACE 		6
#define IS_WLAN_INTERFACE 		7
#define IS_WLANL_INTERFACE 		8
#define IS_ETH_SUBINTERFACE		9

DEFUN( config_arp_stale_time_cmd_func,
        config_set_arp_stale_time_cmd,
        "config arp_stale_time TIME",
        CONFIG_STR
        "set arp stale time for interface\n"
        "input the arp stale time (second)\n"
       )
{
	     DBusMessage *query, *reply;
		 DBusError err;
		 unsigned char slot = 0,port = 0;
		 int stale_time = 0;
		 unsigned short vlanId = 0;
		 unsigned int isVlanIntf = 0;
		 unsigned char cpu_no, cpu_port_no;
		 unsigned int  tag1 = 0,tag2 = 0;
		 int ret = COMMON_SUCCESS;
		 if (argc != 1) {
			vty_out(vty, "argument error!\n");
			return CMD_WARNING;
		 }

		 
		 stale_time = atoi(argv[0]);
		 /* max base_reachable_time_ms is 0xFFFFFFFF(4294967295)  */
         if(stale_time > 4200000 || stale_time <= 0)
         {
			vty_out(vty,"%% the arp_stale time %s!\n", (stale_time>0)?"too big(should < 4200000)":"must Positive number");
            return CMD_WARNING;
		 }
		 
		 if(!strncmp(vlan_eth_port_ifname,"vlan",4)){
			 isVlanIntf = IS_VLAN_ADVANCED;
			 vlanId = (unsigned short)strtoul(vlan_eth_port_ifname+4,NULL,0);
			 slot = port = 0;
		 }
		 else if(!strncmp(vlan_eth_port_ifname,"eth",3)){
			 isVlanIntf = IS_ETH_PORT_INTERFACE;
			 ret = parse_slotport_tag_no(vlan_eth_port_ifname+3,&slot,&port,&tag1,&tag2);
			 vlanId = (unsigned short)tag1;
			 if(COMMON_SUCCESS != ret){
				 vty_out(vty,"%% Bad parameter: ifname %s\n",vlan_eth_port_ifname);
				 return CMD_WARNING;
			 }		

			 if (vlanId != 0){
			     isVlanIntf = IS_ETH_SUBINTERFACE;
			 }
		 }
		 else if (!strncmp (vlan_eth_port_ifname, "ve",2)) {
	  	   	isVlanIntf = IS_VE_INTERFACE;
			/*wangchong for AXSSZFI-1540 ,the "port" used for "cpu_no&cpu_port_no"*/
			ret = parse_ve_slot_cpu_tag_no(vlan_eth_port_ifname, &slot, &cpu_no, &cpu_port_no, &tag1, &tag2);
			vlanId = (unsigned short)tag1;
			port = cpu_no;
			port = ((port << 4) | cpu_port_no);						
	     }
		 else if(0 == strncmp(vlan_eth_port_ifname,"ebr",3) )/*ebr*/
         {
         	  isVlanIntf = IS_EBR_INTERFACE;
              char tmp[128] = {0};
   		      memcpy(tmp, vlan_eth_port_ifname, strlen(vlan_eth_port_ifname));
   		      int i;
   		      int count = 0;
   
        	   for(i = 0; i < strlen(vlan_eth_port_ifname); i++)
        	   {
        		  if(tmp[i] == '-')  //use '-' to make sure this radio is local board or remote board ?
        			  count++;
        	   }
   	    
        	   if(count == 1)       /*local board*/
        	   {
        	   	    isVlanIntf = IS_EBRL_INTERFACE;
   			  		sscanf(vlan_eth_port_ifname,"ebrl%d-%d",&slot,&port);
               }
   		
        	   if(count == 2)/*remote board*/
        	   {
        		  if(strncmp(vlan_eth_port_ifname,"ebrl",4)==0)/*local hansi ebrlx-x-x*/
        		  {
        		  	isVlanIntf = IS_EBRL_INTERFACE;
					if (-1 == parse_wlan_ebr_no(vlan_eth_port_ifname,&slot, &port,&vlanId)){
						vty_out(vty,"%% Bad parameter: ifname %s\n",vlan_eth_port_ifname);
						return CMD_WARNING;
					}
					
        		  }
        		  else
        		  {
        		  	  if (-1 == parse_wlan_ebr_no(vlan_eth_port_ifname,&slot, &port,&vlanId)){
						  vty_out(vty,"%% Bad parameter: ifname %s\n",vlan_eth_port_ifname);
						  return CMD_WARNING;
					  }
        		  }
        	   }
      	  }
	      else if(0 == strncmp(vlan_eth_port_ifname, "wlanl", 5)) 
     	  {
     	  	  isVlanIntf = IS_WLANL_INTERFACE;
			  if (-1 == parse_wlan_ebr_no(vlan_eth_port_ifname,&slot, &port,&vlanId)){
				  vty_out(vty,"%% Bad parameter: ifname %s\n",vlan_eth_port_ifname);
				  return CMD_WARNING;
			  }

     	  }
	      else if ((0 == strncmp(vlan_eth_port_ifname, "wlan", 4)) /*&& (strncmp(vlan_eth_port_ifname, "wlanl", 5))*/) 
    	  {
    	  	  isVlanIntf = IS_WLAN_INTERFACE;
			  if (-1 == parse_wlan_ebr_no(vlan_eth_port_ifname,&slot, &port,&vlanId)){
				  vty_out(vty,"%% Bad parameter: ifname %s\n",vlan_eth_port_ifname);
				  return CMD_WARNING;
			  }
    	  }

		  else 
		  {
			 vty_out(vty, "Can't support this interface\n");
			 return CMD_WARNING;
		 }


		 query = dbus_message_new_method_call(\
													NPD_DBUS_BUSNAME,					\
													NPD_DBUS_INTF_OBJPATH,		\
													NPD_DBUS_INTF_INTERFACE,		\
													NPD_DBUS_INTERFACE_SET_STALE_TIME);
		 
		 dbus_error_init(&err);
		 
         dbus_message_append_args(query,
      							DBUS_TYPE_UINT32,&isVlanIntf,
      							DBUS_TYPE_BYTE,&slot,
      							DBUS_TYPE_BYTE,&port,
      							DBUS_TYPE_UINT16,&vlanId,
      							DBUS_TYPE_UINT32,&stale_time,
      							DBUS_TYPE_INVALID);


		 

		 if (is_distributed) {

			   int i ;
     	 	   for ( i = 0; i< MAX_SLOT; i++) {

               if (NULL != dbus_connection_dcli[i]->dcli_dbus_connection) {
			   	
                      reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[i]->dcli_dbus_connection,query,-1, &err);
                /*vty_out(vty,"slot == %d\n",slot);
				vty_out(vty,"port == %d\n",port);
				vty_out(vty,"stale_time == %d\n",stale_time);*/
				
                     // dbus_message_unref(query);
                      if (NULL == reply)
                      {
                          vty_out(vty,"failed get reply.\n");
                          if (dbus_error_is_set(&err))
                          {
                              vty_out(vty,"%s raised: %s",err.name,err.message);
                              dbus_error_free_for_dcli(&err);
                          }
                          return CMD_SUCCESS;
                      }
                
                      DCLI_DEBUG(("query reply not null\n"));
                      if (dbus_message_get_args(reply, &err,
                                                DBUS_TYPE_UINT32,&ret,
                                                DBUS_TYPE_INVALID))
                      {
                          if ((INTERFACE_RETURN_CODE_SUCCESS != ret)&&\
                              (ARP_RETURN_CODE_SUCCESS != ret))
                          {
                          	  vty_out(vty,"In slot %d :  ", i);
                              vty_out(vty,dcli_error_info_intf(ret));
                          }
                
                      }
                      else
                      {
                          vty_out(vty,"Failed get args.\n");
                          if (dbus_error_is_set(&err))
                          {
                              vty_out(vty,"%s raised: %s",err.name,err.message);
                              dbus_error_free_for_dcli(&err);
                          }
                      }
					 
                      dbus_message_unref(reply);                  
                }
     	  	 }
		  
		     dbus_message_unref(query);
			 return CMD_SUCCESS;
		}
		else 
		{
						
			  reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);
		/*vty_out(vty,"slot == %d\n",slot);
		vty_out(vty,"port == %d\n",port);
		vty_out(vty,"stale_time == %d\n",stale_time);*/
		
			  dbus_message_unref(query);
			  if (NULL == reply)
			  {
				  vty_out(vty,"failed get reply.\n");
				  if (dbus_error_is_set(&err))
				  {
					  vty_out(vty,"%s raised: %s",err.name,err.message);
					  dbus_error_free_for_dcli(&err);
				  }
				  return CMD_SUCCESS;
			  }
		
			  DCLI_DEBUG(("query reply not null\n"));
			  if (dbus_message_get_args(reply, &err,
										DBUS_TYPE_UINT32,&ret,
										DBUS_TYPE_INVALID))
			  {
				  if ((INTERFACE_RETURN_CODE_SUCCESS != ret)&&\
					  (ARP_RETURN_CODE_SUCCESS != ret))
				  {
					  vty_out(vty,dcli_error_info_intf(ret));
				  }
		
			  }
			  else
			  {
				  vty_out(vty,"Failed get args.\n");
				  if (dbus_error_is_set(&err))
				  {
					  vty_out(vty,"%s raised: %s",err.name,err.message);
					  dbus_error_free_for_dcli(&err);
				  }
			  }
			 
			  dbus_message_unref(reply);				  
	   }
	return CMD_SUCCESS;
}



DEFUN(config_interface_static_arp_cmd_func,
        config_interface_static_arp_eth_port_cmd,
        "config static-arp MAC A.B.C.D",
        CONFIG_STR
        "Config static arp entry\n"
        "Config MAC address format as HH:HH:HH:HH:HH:HH\n"
        "Config IP address format as <A.B.C.D>\n"
        "Config static arp for this port\n"
       )
  {
      DBusMessage *query, *reply;
      DBusError err;
      /*variables*/
      unsigned int ret = 0;
      unsigned long ipno = 0;
      unsigned short vlanId = 0;
      ETHERADDR macAddr;
      unsigned int isVlanIntf = 0;
      unsigned char slot = 0,port = 0;
	  unsigned char cpu_no = 0, cpu_port_no = 0;
      unsigned int  tag1 = 0,tag2 = 0;
      unsigned int int_slot_no = 0,int_port_no = 0;
      unsigned int isAdd = TRUE;
	  int ifnameType = 0;
      int count = -1;

      memset(&macAddr,0,sizeof(ETHERADDR));
      
      if(0 == strncmp(vlan_eth_port_ifname,"vlan",4)){
          isVlanIntf = IS_VLAN_ADVANCED;
          vlanId = (unsigned short)strtoul(vlan_eth_port_ifname+4,NULL,0);
          slot = port = 0;
      }
      else if(0 == strncmp(vlan_eth_port_ifname,"eth",3)){
          isVlanIntf = IS_ETH_PORT_INTERFACE;
          ret = parse_slotport_tag_no(vlan_eth_port_ifname+3,&slot,&port,&tag1,&tag2);
          vlanId = (unsigned short)tag1;
          if(COMMON_SUCCESS != ret){
              vty_out(vty,"%% Bad parameter: ifname %s\n",vlan_eth_port_ifname);
              return CMD_WARNING;
          }          
		  if (vlanId != 0){
			  isVlanIntf = IS_ETH_SUBINTERFACE;
		  }
      }
	  else if(dcli_interface_new_eth_ifname_check(vlan_eth_port_ifname)){	      
          isVlanIntf = IS_ETH_PORT_INTERFACE;
		  ifnameType = 1;
          ret = parse_slotport_tag_no(vlan_eth_port_ifname+1,&slot,&port,&tag1,&tag2);
          vlanId = (unsigned short)tag1;
          if(COMMON_SUCCESS != ret){
              vty_out(vty,"%% Bad parameter: ifname %s\n",vlan_eth_port_ifname);
              return CMD_WARNING;
          }          
      }
	  /*wangchao add ,support mng interface*/
	  else if (!strncmp(vlan_eth_port_ifname,"mng",3)) {
		   isVlanIntf = IS_MNG_INTERFACE;	   
		   ret = parse_slotport_tag_no(vlan_eth_port_ifname+3,&slot,&port,&tag1,&tag2);
		   if (COMMON_SUCCESS != ret) {
			  vty_out(vty,"%% Bad parameter: ifname %s\n",vlan_eth_port_ifname);
              return CMD_WARNING;
		   }
	  }
	  else if (!strncmp (vlan_eth_port_ifname, "ve",2)) {
	  		isVlanIntf = IS_VE_INTERFACE;
			ret = parse_ve_slot_cpu_tag_no(vlan_eth_port_ifname, &slot, &cpu_no, &cpu_port_no, &tag1, &tag2);
			vlanId = (unsigned short)tag1;
			port = cpu_no;
			port = ((port << 4) | cpu_port_no);	
			if(COMMON_SUCCESS != ret){
              vty_out(vty,"%% Bad parameter: ifname %s\n",vlan_eth_port_ifname);
              return CMD_WARNING;
          	}
	  }
	  else if(0 == strncmp(vlan_eth_port_ifname,"ebr",3) )/*ebr*/
      {
      
           char tmp[128] = {0};
		   memcpy(tmp, vlan_eth_port_ifname, strlen(vlan_eth_port_ifname));
		   int i;
		   int count = 0;

     	   for(i = 0; i < strlen(vlan_eth_port_ifname); i++)
     	   {
     		  if(tmp[i] == '-')  //use '-' to make sure this radio is local board or remote board ?
     			  count++;
     	   }
	    
	       isVlanIntf = IS_EBR_INTERFACE;
     	   if(count == 1)       /*local board*/
     	   {
			   if (-1 == parse_wlan_ebr_no(vlan_eth_port_ifname,&slot, &port,&vlanId)){
				   vty_out(vty,"%% Bad parameter: ifname %s\n",vlan_eth_port_ifname);
				   return CMD_WARNING;
			   }

           }
		
     	   if(count == 2)/*remote board*/
     	   {
     		  if(strncmp(vlan_eth_port_ifname,"ebrl",4)==0)/*local hansi ebrlx-x-x*/
     		  {
     		  	isVlanIntf = IS_EBRL_INTERFACE;
				if (-1 == parse_wlan_ebr_no(vlan_eth_port_ifname,&slot, &port,&vlanId)){
					vty_out(vty,"%% Bad parameter: ifname %s\n",vlan_eth_port_ifname);
					return CMD_WARNING;
				}
     		  }
     		  else
     		  {
     			sscanf(tmp,"ebr%d-%d-%d",&slot,&port,&vlanId);/*remove hansi ebrx-x-x*/
				if (-1 == parse_wlan_ebr_no(vlan_eth_port_ifname,&slot, &port,&vlanId)){
					vty_out(vty,"%% Bad parameter: ifname %s\n",vlan_eth_port_ifname);
					return CMD_WARNING;
				}
     		  }
     	   }
   	  }
	  else if(0 == strncmp(vlan_eth_port_ifname, "wlanl", 5)) 
	  {
	  	  isVlanIntf = IS_WLANL_INTERFACE;
		  if (-1 == parse_wlan_ebr_no(vlan_eth_port_ifname,&slot, &port,&vlanId)){
			  vty_out(vty,"%% Bad parameter: ifname %s\n",vlan_eth_port_ifname);
			  return CMD_WARNING;
		  }
	  }
	  else if ((0 == strncmp(vlan_eth_port_ifname, "wlan", 4)) && (strncmp(vlan_eth_port_ifname, "wlanl", 5))) 
	  {
	  	  isVlanIntf = IS_WLAN_INTERFACE;
		  if (-1 == parse_wlan_ebr_no(vlan_eth_port_ifname,&slot, &port,&vlanId)){
			  vty_out(vty,"%% Bad parameter: ifname %s\n",vlan_eth_port_ifname);
			  return CMD_WARNING;
		  }
	  }
      else {
          vty_out(vty,"%% Not support this type interface!!!!!!\n");
          return CMD_WARNING;
      }
      /*parse argcs*/
      if ((2 > argc)||(3 < argc))
      {
          vty_out(vty,"%% Bad parameter:Wrong parameter count\n");
      }
	  #if 0
      if(3 == argc){
          ret = parse_slotno_localport_include_slot0((char *)argv[2],&int_slot_no,&int_port_no);
          if (NPD_FAIL == ret)
          {
              vty_out(vty,"%% Bad parameter:unknow portno format!\n");
              return CMD_WARNING;
          }
          else if (1 == ret)
          {
              vty_out(vty,"%% Bad parameter: bad slot/port %s number!\n",argv[2]);
              return CMD_WARNING;
          }
          slot = (unsigned char)int_slot_no;
          port = (unsigned char)int_port_no;
          isVlanIntf = IS_VLAN_INTERFACE;
      }
	  #endif
      ret = parse_mac_addr((char *)argv[0],&macAddr);
      if (NPD_FAIL == ret)
      {
          vty_out(vty,"%% Unknow mac addr format!\n");
          return CMD_WARNING;
      }

      ret = is_muti_brc_mac(&macAddr);
      if (ret == 1)
      {
          vty_out(vty,"%% Input broadcast or multicast MAC!\n");
          return CMD_WARNING;
      }
      if(CMD_SUCCESS != parse_ip_check((char *)argv[1])){
          vty_out(vty,"%% Bad parameter: %s\n",(char *)argv[1]);
          return CMD_WARNING;
      }

	  /*it's illegal to config one IP entry correspond to muilty MAC entries */
      count = static_arp_entry_count((char*)argv[1]);
	  if (count >= 1){
          vty_out(vty,"It's illegal to config one IP entry correspond to muilty MAC entries\n");
		  return CMD_WARNING;
	  }
	  
	  ret = is_muti_brc_ip((char *)argv[1]);
	  if (ret == 1)
	  {
			vty_out(vty,"%% Input broadcast or multicast ip!\n");
			return CMD_WARNING;
	  }
      ipno = dcli_ip2ulong((char *)argv[1]);
      
      if (vlanId>MAX_L3INTF_VLANID)
      {
          vty_out(vty,"%% Vlan out of rang!\n");
          return CMD_WARNING;
      }

      /*appends and gets*/
      query = dbus_message_new_method_call(\
                                           NPD_DBUS_BUSNAME,                   \
                                           NPD_DBUS_INTF_OBJPATH,      \
                                           NPD_DBUS_INTF_INTERFACE,        \
                                           NPD_DBUS_INTERFACE_STATIC_ARP);

      dbus_error_init(&err);

      dbus_message_append_args(query,
                               DBUS_TYPE_UINT32,&isVlanIntf,
                               DBUS_TYPE_UINT32,&ifnameType,
                               DBUS_TYPE_BYTE,&slot,
                               DBUS_TYPE_BYTE,&port,
                               DBUS_TYPE_UINT16,&vlanId,
                               DBUS_TYPE_UINT32,&tag2,
                               DBUS_TYPE_BYTE,&macAddr.arEther[0],
                               DBUS_TYPE_BYTE,&macAddr.arEther[1],
                               DBUS_TYPE_BYTE,&macAddr.arEther[2],
                               DBUS_TYPE_BYTE,&macAddr.arEther[3],
                               DBUS_TYPE_BYTE,&macAddr.arEther[4],
                               DBUS_TYPE_BYTE,&macAddr.arEther[5],
                               DBUS_TYPE_UINT32,&ipno,
                               DBUS_TYPE_UINT32,&isAdd,
                               DBUS_TYPE_INVALID);


/*wangchao*/
     if (is_distributed) {
     
     	  int i ;
     	  for ( i = 0; i< MAX_SLOT; i++) {

               if (NULL != dbus_connection_dcli[i]->dcli_dbus_connection) {
			   	
                      reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[i]->dcli_dbus_connection,query,-1, &err);
                
                     // dbus_message_unref(query);
                      if (NULL == reply)
                      {
                          vty_out(vty,"failed get reply.\n");
                          if (dbus_error_is_set(&err))
                          {
                              vty_out(vty,"%s raised: %s",err.name,err.message);
                              dbus_error_free_for_dcli(&err);
                          }
                          return CMD_SUCCESS;
                      }
                
                      DCLI_DEBUG(("query reply not null\n"));
                      if (dbus_message_get_args(reply, &err,
                                                DBUS_TYPE_UINT32,&ret,
                                                DBUS_TYPE_INVALID))
                      {
                          if ((INTERFACE_RETURN_CODE_SUCCESS != ret)&&\
                              (ARP_RETURN_CODE_SUCCESS != ret))
                          {
                          	  vty_out(vty,"In slot %d :  ", i);
                              vty_out(vty,dcli_error_info_intf(ret));
                          }
                
                      }
                      else
                      {
                          vty_out(vty,"Failed get args.\n");
                          if (dbus_error_is_set(&err))
                          {
                              vty_out(vty,"%s raised: %s",err.name,err.message);
                              dbus_error_free_for_dcli(&err);
                          }
                      }
					 
                      dbus_message_unref(reply);                  
                }
     	  	 }
		  
		     dbus_message_unref(query);
			 return CMD_SUCCESS;
		}
	   else {
        	  reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);
        
              dbus_message_unref(query);
        
              if (NULL == reply)
              {
                  vty_out(vty,"failed get reply.\n");
                  if (dbus_error_is_set(&err))
                  {
                      vty_out(vty,"%s raised: %s",err.name,err.message);
                      dbus_error_free_for_dcli(&err);
                  }
                  return CMD_SUCCESS;
              }
        
              DCLI_DEBUG(("query reply not null\n"));
              if (dbus_message_get_args(reply, &err,
                                        DBUS_TYPE_UINT32,&ret,
                                        DBUS_TYPE_INVALID))
              {
          if ((INTERFACE_RETURN_CODE_SUCCESS != ret)&&\
              (ARP_RETURN_CODE_SUCCESS != ret))
          {
              vty_out(vty,dcli_error_info_intf(ret));
          }

      }
      else
      {
          vty_out(vty,"Failed get args.\n");
          if (dbus_error_is_set(&err))
          {
              vty_out(vty,"%s raised: %s",err.name,err.message);
              dbus_error_free_for_dcli(&err);
          }
      }
      dbus_message_unref(reply);

      return CMD_SUCCESS;
       	} 
}
/*wangchao delete*/
#if 0
ALIAS(config_interface_static_arp_cmd_func,
        config_interface_static_arp_cmd,
        "config static-arp MAC A.B.C.D",
        CONFIG_STR
        "Config static arp entry\n"
        "Config MAC address format as HH:HH:HH:HH:HH:HH\n"
        "Config IP address format as <A.B.C.D>\n"
     );


DEFUN(config_interface_static_arp_trunk_cmd_func,
        config_interface_static_arp_trunk_cmd,
        "config static-arp MAC A.B.C.D <1-127>",
        CONFIG_STR
        "Config static arp entry\n"
        "Config MAC address format as HH:HH:HH:HH:HH:HH\n"
        "Config IP address format as <A.B.C.D>\n"
        "Config static arp for this trunk\n"
       )
  {
      DBusMessage *query, *reply;
      DBusError err;
      /*variables*/
      unsigned int ret = 0;
      unsigned long ipno = 0;
      unsigned short vlanId = 0;
      ETHERADDR macAddr;
      unsigned int isVlanIntf = 0;
      unsigned int trunkId = 0;
      unsigned int isAdd = TRUE;


      memset(&macAddr,0,sizeof(ETHERADDR));
      
      if(!strncmp(vlan_eth_port_ifname,"vlan",4)){
          isVlanIntf = IS_VLAN_INTERFACE;
          vlanId = (unsigned short)strtoul(vlan_eth_port_ifname+4,NULL,0);
      }
      else {
          vty_out(vty,"%% Bad parameter!\n");
          return CMD_WARNING;
      }
      /*parse argcs*/
      if (3 != argc)
      {
          vty_out(vty,"%% Bad parameter:Wrong parameter count\n");
      }
      trunkId = strtoul((char *)argv[2],NULL,0);
      ret = parse_mac_addr((char *)argv[0],&macAddr);
      if (NPD_FAIL == ret)
      {
          vty_out(vty,"%% Unknow mac addr format!\n");
          return CMD_WARNING;
      }

      ret = is_muti_brc_mac(&macAddr);
      if (ret == 1)
      {
          vty_out(vty,"%% Input broadcast or multicast mac!\n");
          return CMD_WARNING;
      }
      if(CMD_SUCCESS != parse_ip_check((char *)argv[1])){
          vty_out(vty,"%% Bad parameter: %s\n",(char *)argv[1]);
          return CMD_WARNING;
      }
      ipno = dcli_ip2ulong((char *)argv[1]);
      
      if (vlanId>MAX_L3INTF_VLANID)
      {
          vty_out(vty,"%% Vlan out of rang!\n");
          return CMD_WARNING;
      }

      /*appends and gets*/
      query = dbus_message_new_method_call(\
                                           NPD_DBUS_BUSNAME,                   \
                                           NPD_DBUS_INTF_OBJPATH,      \
                                           NPD_DBUS_INTF_INTERFACE,        \
                                           NPD_DBUS_INTERFACE_STATIC_ARP_TRUNK);

      dbus_error_init(&err);

      dbus_message_append_args(query,
                               DBUS_TYPE_UINT32,&isVlanIntf,
                               DBUS_TYPE_UINT32,&vlanId,
                               DBUS_TYPE_UINT32,&trunkId,
                               DBUS_TYPE_BYTE,&macAddr.arEther[0],
                               DBUS_TYPE_BYTE,&macAddr.arEther[1],
                               DBUS_TYPE_BYTE,&macAddr.arEther[2],
                               DBUS_TYPE_BYTE,&macAddr.arEther[3],
                               DBUS_TYPE_BYTE,&macAddr.arEther[4],
                               DBUS_TYPE_BYTE,&macAddr.arEther[5],
                               DBUS_TYPE_UINT32,&ipno,
                               DBUS_TYPE_UINT32,&isAdd,
                               DBUS_TYPE_INVALID);

      reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);

      dbus_message_unref(query);

      if (NULL == reply)
      {
          vty_out(vty,"failed get reply.\n");
          if (dbus_error_is_set(&err))
          {
              vty_out(vty,"%s raised: %s",err.name,err.message);
              dbus_error_free_for_dcli(&err);
          }
          return CMD_SUCCESS;
      }

      DCLI_DEBUG(("query reply not null\n"));
      if (dbus_message_get_args(reply, &err,
                                DBUS_TYPE_UINT32,&ret,
                                DBUS_TYPE_INVALID))
      {
          if ((INTERFACE_RETURN_CODE_SUCCESS != ret)&&\
              (ARP_RETURN_CODE_SUCCESS != ret))
          {
              vty_out(vty,dcli_error_info_intf(ret));
          }

      }
      else
      {
          vty_out(vty,"Failed get args.\n");
          if (dbus_error_is_set(&err))
          {
              vty_out(vty,"%s raised: %s",err.name,err.message);
              dbus_error_free_for_dcli(&err);
          }
      }
      dbus_message_unref(reply);

      return CMD_SUCCESS;
  }
#endif
DEFUN(no_interface_static_arp_cmd_func,
        no_interface_static_arp_eth_port_cmd,
        "no static-arp MAC A.B.C.D",
        NO_STR
        "No static arp entry\n"
        "No MAC address format as HH:HH:HH:HH:HH:HH\n"
        "No IP address format as <A.B.C.D>\n"
        "No static arp for this port\n"
       )
  {
      DBusMessage *query, *reply;
      DBusError err;
      /*variables*/
      unsigned int ret = 0;
      unsigned long ipno = 0;
      unsigned short vlanId = 0;
      ETHERADDR macAddr;
      unsigned int isVlanIntf = 0;
      unsigned char slot = 0,port = 0;
	  unsigned int tag1 = 0;
      unsigned int  tag2 = 0;
      unsigned int int_slot_no = 0,int_port_no = 0;
      unsigned int isAdd = FALSE;
	  int ifnameType = 0;

	  unsigned char cpu_no = 0, cpu_port_no = 0;



      memset(&macAddr,0,sizeof(ETHERADDR));
      
      if(!strncmp(vlan_eth_port_ifname,"vlan",4)){
          isVlanIntf = IS_VLAN_ADVANCED;
          vlanId = (unsigned short)strtoul(vlan_eth_port_ifname+4,NULL,0);
          slot = port = 0;
      }
      else if(0 == strncmp(vlan_eth_port_ifname,"eth",3)){
          isVlanIntf = IS_ETH_PORT_INTERFACE;
          ret = parse_slotport_tag_no(vlan_eth_port_ifname+3,&slot,&port,&tag1,&tag2);		  
          vlanId = (unsigned short)tag1;
          if(COMMON_SUCCESS != ret){
              vty_out(vty,"%% Bad parameter: ifname %s\n",vlan_eth_port_ifname);
              return CMD_WARNING;
          }       

		  if (vlanId != 0){
			  isVlanIntf = IS_ETH_SUBINTERFACE;
		  }
		  
      }
	  else if(dcli_interface_new_eth_ifname_check(vlan_eth_port_ifname)){
	  	  isVlanIntf = IS_ETH_PORT_INTERFACE;
		  ifnameType = 1;
          ret = parse_slotport_tag_no(vlan_eth_port_ifname+3,&slot,&port,&vlanId,&tag2);
          if(COMMON_SUCCESS != ret){
              vty_out(vty,"%% Bad parameter: ifname %s\n",vlan_eth_port_ifname);
              return CMD_WARNING;
          }          
      }	  /*wangchao add ,support mng interface*/
	  else if (0 == strncmp(vlan_eth_port_ifname,"mng",3)) {
		   isVlanIntf = IS_MNG_INTERFACE;	   
		   ret = parse_slotport_tag_no(vlan_eth_port_ifname+3,&slot,&port,&vlanId,&tag2);
		   if (COMMON_SUCCESS != ret) {
			  vty_out(vty,"%% Bad parameter: ifname %s\n",vlan_eth_port_ifname);
              return CMD_WARNING;
		   }
	  }
	  else if (0 == strncmp (vlan_eth_port_ifname, "ve",2)) 
	  {
	  		isVlanIntf = IS_VE_INTERFACE;
			ret = parse_ve_slot_cpu_tag_no(vlan_eth_port_ifname, &slot, &cpu_no, &cpu_port_no, &tag1, &tag2);
			vlanId = (unsigned short)tag1;
			port = cpu_no;
			port = ((port << 4) | cpu_port_no);	
			if(COMMON_SUCCESS != ret){
              vty_out(vty,"%% Bad parameter: ifname %s\n",vlan_eth_port_ifname);
              return CMD_WARNING;
          	}
	  }  
	  else if(0 == strncmp(vlan_eth_port_ifname,"ebr",3) )/*ebr*/
      {
      
           char tmp[128] = {0};
		   memcpy(tmp, vlan_eth_port_ifname, strlen(vlan_eth_port_ifname));
		   int i;
		   int count = 0;

     	   for(i = 0; i < strlen(vlan_eth_port_ifname); i++)
     	   {
     		  if(tmp[i] == '-')  //use '-' to make sure this radio is local board or remote board ?
     			  count++;
     	   }
	    
	       isVlanIntf = IS_EBR_INTERFACE;
     	   if(count == 1)       /*local board*/
     	   { 	   	  
			  if (-1 == parse_wlan_ebr_no(vlan_eth_port_ifname,&slot, &port,&vlanId)){
				  vty_out(vty,"%% Bad parameter: ifname %s\n",vlan_eth_port_ifname);
				  return CMD_WARNING;
			  }
     		 // slot = HostSlotId;
           }
		
     	   if(count == 2)/*remote board*/
     	   {
     		  if(0 == strncmp(vlan_eth_port_ifname,"ebrl",4))/*local hansi ebrlx-x-x*/
     		  {
     		  	isVlanIntf = IS_EBRL_INTERFACE;
				if (-1 == parse_wlan_ebr_no(vlan_eth_port_ifname,&slot, &port,&vlanId)){
					vty_out(vty,"%% Bad parameter: ifname %s\n",vlan_eth_port_ifname);
					return CMD_WARNING;
				}
     		  }
     		  else
     		  {
				  if (-1 == parse_wlan_ebr_no(vlan_eth_port_ifname,&slot, &port,&vlanId)){
					  vty_out(vty,"%% Bad parameter: ifname %s\n",vlan_eth_port_ifname);
					  return CMD_WARNING;
				  }
     		  }
     	   }
   	  }
	  else if(0 == strncmp(vlan_eth_port_ifname, "wlanl", 5)) 
	  {
	  	  isVlanIntf = IS_WLANL_INTERFACE;
		  if (-1 == parse_wlan_ebr_no(vlan_eth_port_ifname,&slot, &port,&vlanId)){
			  vty_out(vty,"%% Bad parameter: ifname %s\n",vlan_eth_port_ifname);
			  return CMD_WARNING;
		  }
	  }
	  else if ((0 == strncmp(vlan_eth_port_ifname, "wlan", 4)) && (strncmp(vlan_eth_port_ifname, "wlanl", 5))) 
	  {
	  	  isVlanIntf = IS_WLAN_INTERFACE;
		  if (-1 == parse_wlan_ebr_no(vlan_eth_port_ifname,&slot, &port,&vlanId)){
			  vty_out(vty,"%% Bad parameter: ifname %s\n",vlan_eth_port_ifname);
			  return CMD_WARNING;
		  }
	  }
      else {
          vty_out(vty,"%% Not support this type interface\n");
          return CMD_WARNING;
      }
      /*parse argcs*/
      if ((2 > argc)||(3 < argc))
      {
          vty_out(vty,"%% Bad parameter:Wrong parameter count\n");
      }
      if(3 == argc){
          ret = parse_slotno_localport_include_slot0((char *)argv[0],&int_slot_no,&int_port_no);
          if (NPD_FAIL == ret)
          {
              vty_out(vty,"%% Bad parameter:unknow portno format!\n");
              return CMD_WARNING;
          }
          else if (1 == ret)
          {
              vty_out(vty,"%% Bad parameter: bad slot/port %s number!\n",argv[0]);
              return CMD_WARNING;
          }
          slot = (unsigned char)int_slot_no;
          port = (unsigned char)int_port_no;
          isVlanIntf = IS_VLAN_INTERFACE;
      }
      ret = parse_mac_addr((char *)argv[0],&macAddr);
      if (NPD_FAIL == ret)
      {
          vty_out(vty,"%% Unknow mac addr format!\n");
          return CMD_WARNING;
      }

      ret = is_muti_brc_mac(&macAddr);
      if (ret == 1)
      {
          vty_out(vty,"%% Input broadcast or multicast mac!\n");
          return CMD_WARNING;
      }
      if(CMD_SUCCESS != parse_ip_check((char *)argv[1])){
          vty_out(vty,"%% Bad parameter: %s\n",(char *)argv[1]);
          return CMD_WARNING;
      }
      ipno = dcli_ip2ulong((char *)argv[1]);
      
      if (vlanId>MAX_L3INTF_VLANID)
      {
          vty_out(vty,"%% Vlan out of rang!\n");
          return CMD_WARNING;
      }

      /*appends and gets*/
      query = dbus_message_new_method_call(\
                                           NPD_DBUS_BUSNAME,                   \
                                           NPD_DBUS_INTF_OBJPATH,      \
                                           NPD_DBUS_INTF_INTERFACE,        \
                                           NPD_DBUS_INTERFACE_STATIC_ARP);

      dbus_error_init(&err);

      dbus_message_append_args(query,
                               DBUS_TYPE_UINT32,&isVlanIntf,
                               DBUS_TYPE_UINT32,&ifnameType,
                               DBUS_TYPE_BYTE,&slot,
                               DBUS_TYPE_BYTE,&port,
                               DBUS_TYPE_UINT16,&vlanId,
                               DBUS_TYPE_UINT32,&tag2,
                               DBUS_TYPE_BYTE,&macAddr.arEther[0],
                               DBUS_TYPE_BYTE,&macAddr.arEther[1],
                               DBUS_TYPE_BYTE,&macAddr.arEther[2],
                               DBUS_TYPE_BYTE,&macAddr.arEther[3],
                               DBUS_TYPE_BYTE,&macAddr.arEther[4],
                               DBUS_TYPE_BYTE,&macAddr.arEther[5],
                               DBUS_TYPE_UINT32,&ipno,
                               DBUS_TYPE_UINT32,&isAdd,
                               DBUS_TYPE_INVALID);

	  if (is_distributed) {
		  
		   int i ;
		   for ( i = 0; i< MAX_SLOT; i++) {
					 
           	   if (NULL != dbus_connection_dcli[i]->dcli_dbus_connection) {
           
                 reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[i]->dcli_dbus_connection,query,-1, &err);
           
                 //dbus_message_unref(query);
           
                 if (NULL == reply)
                 {
                     vty_out(vty,"failed get reply.\n");
                     if (dbus_error_is_set(&err))
                     {
                         vty_out(vty,"%s raised: %s",err.name,err.message);
                         dbus_error_free_for_dcli(&err);
                     }
                     return CMD_SUCCESS;
                 }
           
                 DCLI_DEBUG(("query reply not null\n"));
                 if (dbus_message_get_args(reply, &err,
                                           DBUS_TYPE_UINT32,&ret,
                                           DBUS_TYPE_INVALID))
                 {
                     if ((INTERFACE_RETURN_CODE_SUCCESS != ret)&&\
                         (ARP_RETURN_CODE_SUCCESS != ret))
                     {
 						 vty_out(vty, "In slot %d",i);                    
                         vty_out(vty,dcli_error_info_intf(ret));
                     }
           
                 }
                 else
                 {
                     vty_out(vty,"Failed get args.\n");
                     if (dbus_error_is_set(&err))
                     {
                         vty_out(vty,"%s raised: %s",err.name,err.message);
                         dbus_error_free_for_dcli(&err);
                     }
                 }
                 dbus_message_unref(reply);
           
                // return CMD_SUCCESS;
             }
			   
  		  }
		   
		  dbus_message_unref(query);
		  return CMD_SUCCESS;
	  	}
       else
		{
        	  reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);
        
              dbus_message_unref(query);
        
              if (NULL == reply)
              {
                  vty_out(vty,"failed get reply.\n");
                  if (dbus_error_is_set(&err))
                  {
                      vty_out(vty,"%s raised: %s",err.name,err.message);
                      dbus_error_free_for_dcli(&err);
                  }
                  return CMD_SUCCESS;
              }
        
              DCLI_DEBUG(("query reply not null\n"));
              if (dbus_message_get_args(reply, &err,
                                        DBUS_TYPE_UINT32,&ret,
                                        DBUS_TYPE_INVALID))
              {
                  if ((INTERFACE_RETURN_CODE_SUCCESS != ret)&&\
                      (ARP_RETURN_CODE_SUCCESS != ret))
                  {
                      vty_out(vty,dcli_error_info_intf(ret));
                  }
        
              }
              else
              {
                  vty_out(vty,"Failed get args.\n");
                  if (dbus_error_is_set(&err))
                  {
                      vty_out(vty,"%s raised: %s",err.name,err.message);
                      dbus_error_free_for_dcli(&err);
                  }
              }
              dbus_message_unref(reply);
        
              return CMD_SUCCESS;
      }
}
#if 0
ALIAS(no_interface_static_arp_cmd_func,
        no_interface_static_arp_cmd,
        "no static-arp MAC A.B.C.D",
        "No configuration\n"
        "No static arp entry\n"
        "No MAC address format as HH:HH:HH:HH:HH:HH\n"
        "No IP address format as <A.B.C.D/M>\n"
     );


DEFUN(no_interface_static_arp_trunk_cmd_func,
        no_interface_static_arp_trunk_cmd,
        "no static-arp MAC A.B.C.D <1-127>",
        NO_STR
        "No static arp entry\n"
        "No MAC address format as HH:HH:HH:HH:HH:HH\n"
        "No IP address format as <A.B.C.D/M>\n"
        "Config trunk number in range <1-127>\n"
       )
  {
      DBusMessage *query, *reply;
      DBusError err;
      /*variables*/
      unsigned int ret = 0;
      unsigned long ipno = 0;
      unsigned short vlanId = 0;
      ETHERADDR macAddr;
      unsigned int isVlanIntf = 0;
      unsigned int trunkId = 0;
      unsigned int isAdd = FALSE;


      memset(&macAddr,0,sizeof(ETHERADDR));
      
      if(!strncmp(vlan_eth_port_ifname,"vlan",4)){
          isVlanIntf = IS_VLAN_INTERFACE;
          vlanId = (unsigned short)strtoul(vlan_eth_port_ifname+4,NULL,0);
      }
      else {
          vty_out(vty,"%% Bad parameter!\n");
          return CMD_WARNING;
      }
      /*parse argcs*/
      if (3 != argc)
      {
          vty_out(vty,"%% Bad parameter:Wrong parameter count\n");
      }
      trunkId = strtoul((char *)argv[2],NULL,0);
      ret = parse_mac_addr((char *)argv[0],&macAddr);
      if (NPD_FAIL == ret)
      {
          vty_out(vty,"%% Unknow mac addr format!\n");
          return CMD_WARNING;
      }

      ret = is_muti_brc_mac(&macAddr);
      if (ret == 1)
      {
          vty_out(vty,"%% Input broadcast or multicast mac!\n");
          return CMD_WARNING;
      }
      if(CMD_SUCCESS != parse_ip_check((char *)argv[1])){
          vty_out(vty,"%% Bad parameter: %s\n",(char *)argv[1]);
          return CMD_WARNING;
      }
      ipno = dcli_ip2ulong((char *)argv[1]);
      
      if (vlanId>MAX_L3INTF_VLANID)
      {
          vty_out(vty,"%% Vlan out of rang!\n");
          return CMD_WARNING;
      }

      /*appends and gets*/
      query = dbus_message_new_method_call(\
                                           NPD_DBUS_BUSNAME,                   \
                                           NPD_DBUS_INTF_OBJPATH,      \
                                           NPD_DBUS_INTF_INTERFACE,        \
                                           NPD_DBUS_INTERFACE_STATIC_ARP_TRUNK);

      dbus_error_init(&err);

      dbus_message_append_args(query,
                               DBUS_TYPE_UINT32,&isVlanIntf,
                               DBUS_TYPE_UINT32,&vlanId,
                               DBUS_TYPE_UINT32,&trunkId,
                               DBUS_TYPE_BYTE,&macAddr.arEther[0],
                               DBUS_TYPE_BYTE,&macAddr.arEther[1],
                               DBUS_TYPE_BYTE,&macAddr.arEther[2],
                               DBUS_TYPE_BYTE,&macAddr.arEther[3],
                               DBUS_TYPE_BYTE,&macAddr.arEther[4],
                               DBUS_TYPE_BYTE,&macAddr.arEther[5],
                               DBUS_TYPE_UINT32,&ipno,
                               DBUS_TYPE_UINT32,&isAdd,
                               DBUS_TYPE_INVALID);

      reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);

      dbus_message_unref(query);

      if (NULL == reply)
      {
          vty_out(vty,"failed get reply.\n");
          if (dbus_error_is_set(&err))
          {
              vty_out(vty,"%s raised: %s",err.name,err.message);
              dbus_error_free_for_dcli(&err);
          }
          return CMD_SUCCESS;
      }

      DCLI_DEBUG(("query reply not null\n"));
      if (dbus_message_get_args(reply, &err,
                                DBUS_TYPE_UINT32,&ret,
                                DBUS_TYPE_INVALID))
      {
          if ((INTERFACE_RETURN_CODE_SUCCESS != ret)&&\
              (ARP_RETURN_CODE_SUCCESS != ret))
          {
              vty_out(vty,dcli_error_info_intf(ret));
          }

      }
      else
      {
          vty_out(vty,"Failed get args.\n");
          if (dbus_error_is_set(&err))
          {
              vty_out(vty,"%s raised: %s",err.name,err.message);
              dbus_error_free_for_dcli(&err);
          }
      }
      dbus_message_unref(reply);

      return CMD_SUCCESS;
  }

#endif
char dcli_vty_ifname[INTERFACE_NAMSIZ+1] = {0};
    DEFUN(
        vtysh_interface,
        vtysh_interface_cmd,
        "interface IFNAME",
        "Select an interface to configure\n"
        "Interface's name\n")
    {
        size_t sl;
        int ret = 0;
        char cmd[INTERFACE_NAMSIZ+40];
        int i;
        int cmd_stat;
        char line[80];
        FILE *fp = NULL;
        fp = stdout;
        int slot_index = 0;

        if ((sl = strlen(argv[0])) > INTERFACE_NAMSIZ)
        {
            vty_out(vty, "%% Interface name %s is invalid: length exceeds "
                    "%d characters%s",
                    argv[0], INTERFACE_NAMSIZ, VTY_NEWLINE);
            return CMD_WARNING;
        }

		strcpy(dcli_vty_ifname,argv[0]);
		
        /*get interface name , will be used for vlan interface or eth-port interface
        // when config "advanced-routing enable/disable" in interface node*/
        memcpy((void *)vlan_eth_port_ifname,(void *)argv[0],INTERFACE_NAMSIZ);
        /*fprintf(stdout,"IFNAME:%s\n",argv[0]);*/
#ifdef _D_WCPSS_/*zhanglei add*/
        /*if ifname is wlan*/
        if (!strncasecmp(argv[0],"wlan",4))
        {
			if((vty->node != HANSI_NODE)&&(vty->node != LOCAL_HANSI_NODE)){
				vty_out(vty, "%s,%d,invalid node:%d.",__func__,__LINE__,vty->node);
				return CMD_WARNING;
			}else{

			}
			ret = wid_interface_ifname_wlan((char*)argv[0],vty,line);
            if (ret == CMD_SUCCESS)
            {
                if ((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE))
                {
                    vty->node = INTERFACE_NODE;
                }
                else if (vty->node == HANSI_NODE)
                {
                    vty->node = INTERFACE_NODE;
                    vty->prenode = HANSI_NODE;
                    vty->index_sub = vty->index;
                }
                else if (vty->node == LOCAL_HANSI_NODE)
                {
                    vty->node = INTERFACE_NODE;
                    vty->prenode = LOCAL_HANSI_NODE;
                    vty->index_sub = vty->index;
                }
                for (i = 0; i < 7; i++)
                {
                    cmd_stat = vtysh_client_execute(&vtysh_client[i], line, fp);
                    if (cmd_stat == CMD_WARNING||cmd_stat == CMD_FAILURE)
                        break;
                }
				vty->index = dcli_vty_ifname;
                return CMD_SUCCESS;
            }
            else
            {
                return CMD_SUCCESS;
            }
        }


        /*if the ifname is radio*/
        else if ((!strncasecmp(argv[0],"radio",5))||(!strncasecmp(argv[0],"r",1)))
        {
			if((vty->node != HANSI_NODE)&&(vty->node != LOCAL_HANSI_NODE)){
				vty_out(vty, "%s,%d,invalid node:%d.",__func__,__LINE__,vty->node);
				return CMD_WARNING;
			}else{

			}
            ret = wid_interface_ifname_radio((char *)argv[0],vty,line);
            if (ret == CMD_SUCCESS)
            {
                if ((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE))
                {
                    vty->node = INTERFACE_NODE;
                    vty->prenode = 0;
                }
                else if (vty->node == HANSI_NODE)
                {
                    vty->node = INTERFACE_NODE;
                    vty->prenode = HANSI_NODE;
                    vty->index_sub = vty->index;
                }
                else if (vty->node == LOCAL_HANSI_NODE)
                {
                    vty->node = INTERFACE_NODE;
                    vty->prenode = LOCAL_HANSI_NODE;
                    vty->index_sub = vty->index;
                }
                for (i = 0; i < 7; i++)
                {
                    cmd_stat = vtysh_client_execute(&vtysh_client[i], line, fp);
                    if (cmd_stat == CMD_WARNING||cmd_stat == CMD_FAILURE)
                        break;
                }
				
				vty->index = dcli_vty_ifname;
                return CMD_SUCCESS;
            }
            else
            {
                return CMD_WARNING;
            }
        }
        /*if the ifname is ebr*/
        else if (!strncasecmp(argv[0],"ebr",3))
        {
			if((vty->node != HANSI_NODE)&&(vty->node != LOCAL_HANSI_NODE)){
				vty_out(vty, "%s,%d,invalid node:%d.",__func__,__LINE__,vty->node);
				return CMD_WARNING;
			}else{

			}
            ret = wid_interface_ifname_ebr((char *)argv[0],vty,line);
            if (ret == CMD_SUCCESS)
            {
                if ((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE))
                {
                    vty->node = INTERFACE_NODE;
                }
                else if (vty->node == HANSI_NODE)
                {
                    vty->node = INTERFACE_NODE;
                    vty->prenode = HANSI_NODE;
                    vty->index_sub = vty->index;
                }
                else if (vty->node == LOCAL_HANSI_NODE)
                {
                    vty->node = INTERFACE_NODE;
                    vty->prenode = LOCAL_HANSI_NODE;
                    vty->index_sub = vty->index;
                }
                for (i = 0; i < 7; i++)
                {
                    cmd_stat = vtysh_client_execute(&vtysh_client[i], line, fp);
                    if (cmd_stat == CMD_WARNING||cmd_stat == CMD_FAILURE)
                        break;
                }
				
				vty->index = dcli_vty_ifname;
                return CMD_SUCCESS;
            }
            else
            {
                return CMD_WARNING;
            }
        }
        else
            /*the other ifnames,add here*/
#endif
        {
            if (!strncmp(argv[0],"vlan",4))
            {
                if (CMD_SUCCESS != dcli_interface_ifname_vlan(vty,(char *)argv[0]))
                {
                    return CMD_WARNING;
                }
            }
			#if 0
			else if(!(strncmp(tolower(argv[0]), "obc", 3))){
				if (CMD_SUCCESS != dcli_create_vlan_intf_by_vlan_ifname(vty,4093))
                {
                    return CMD_WARNING;
                }
			}
			#endif
			else if (!strncmp(argv[0],"eth",3) || !strncmp(argv[0],"mng",3))
            {
            	if (CMD_SUCCESS != dcli_interface_ifname_eth_port(vty,(char *)argv[0]))
            	{
                	return CMD_WARNING;
            	}
            }
            else if ((!strncmp(tolower(argv[0]), "cscd", 4)) ||\	
				(!strncmp(tolower(argv[0]), "obc", 3))||\
				(dcli_interface_new_eth_ifname_check((unsigned char *)argv[0]))||\
				(!strncmp(argv[0],"ve",2)))
            {
                if (CMD_SUCCESS != dcli_interface_ifname_eth_port(vty,(char *)argv[0]))
                {
                    return CMD_WARNING;
                }
            }
            else if (!strncmp(argv[0],"bond",4))
            {
				#if 0
                if (CMD_SUCCESS != dcli_interface_ifname_bond(vty,(char *)argv[0]))
                {
                    return CMD_WARNING;
                }
				#endif
				vty_out(vty,"%% Bad param !\n");
				return CMD_WARNING;
            }
            else 
            {
                if (strcmp(argv[0],"lo")&&strcmp(argv[0],"sit0"))
                {
                    vty_out(vty,"%% Error IFNAME\n");
                    return CMD_WARNING;
                }
            }
			
        }

        if ((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE))
        {
            vty->node = INTERFACE_NODE;
           	vty->prenode = 0;
        }
        else if (vty->node == HANSI_NODE)
        {
            vty->node = INTERFACE_NODE;
            vty->prenode = HANSI_NODE;
            vty->index_sub = vty->index;
        }
        else if (vty->node == LOCAL_HANSI_NODE)
        {
            vty->node = INTERFACE_NODE;
            vty->prenode = LOCAL_HANSI_NODE;
            vty->index_sub = vty->index;
        }
        sprintf(line,"interface %s",argv[0]);
        for (i = 0; i < 7; i++)
        {
            cmd_stat = vtysh_client_execute(&vtysh_client[i], line, fp);
            if (cmd_stat == CMD_WARNING||cmd_stat == CMD_FAILURE)
                break;
        }
		vty->index = dcli_vty_ifname;
		
        return CMD_SUCCESS;
    }
	/*del pfm config func such as: servers telnet eth1-1 enable*/

	void del_pfm_config(char* ifname_str)
	{
		char cmd_str[256];
			sprintf(cmd_str,"sed '/%s/d' %s >%s 2> /dev/null",ifname_str,PFM_SETUP_FILE,PFM_SETUP_TMP_FILE);
		//	  fprintf(stderr,"cmd_bak:%s\n",cmd_str);
			system(cmd_str);
		//	ret = WEXITSTATUS(status);
		
			/**mv file : pfm_setup_tmp ==> pfm_setup**/
			sprintf(cmd_str,"mv %s %s",PFM_SETUP_TMP_FILE,PFM_SETUP_FILE);
		//	fprintf(stderr, "cmd_mv string: %s...\n",cmd_mv);
			system(cmd_str);
	}

    /* TODO Implement "no interface command in isisd. */
    DEFUN(vtysh_no_interface,
          vtysh_no_interface_cmd,
          "no interface IFNAME",
          NO_STR
          "Delete a pseudo interface's configuration\n"
          "Interface's name\n")
    {
        /*fprintf(stdout,"IFNAME:%s\n",argv[0]);*/
		/*del pfm config such as: servers telnet eth1-1 enable*/
		del_pfm_config(argv[0]);
		
#ifdef _D_WCPSS_  /*zhanglei add*/


        int ret = 0;
	char cmd[128];
	pid_t status;
	memset(cmd,0,128);
	sprintf(cmd,"sudo brctl show |grep %s > /dev/null 2>&1",argv[0]);
	status = system(cmd);
	if (status != -1) {
		if (WIFEXITED(status)) {
			if (0 == WEXITSTATUS(status)) {
				vty_out(vty, "The interface already in the ebr.\n");
				return CMD_SUCCESS;
			}
		}
	}
	
        /*if ifname is wlan*/
        if (!strncasecmp(argv[0],"wlan",4))
        {
		if((vty->node != HANSI_NODE)&&(vty->node != LOCAL_HANSI_NODE)){
			vty_out(vty, "%s,%d,invalid node:%d.",__func__,__LINE__,vty->node);
			return CMD_WARNING;
		}else{
		}
            ret = wid_no_interface_ifname_wlan((char*)argv[0],vty);
            if (ret == CMD_SUCCESS)
            {
                return CMD_SUCCESS;
            }
            else
            {
                return CMD_WARNING;
             }
        }

        /*if the ifname is radio*/
        else if ((!strncasecmp(argv[0],"radio",5))||(!strncasecmp(argv[0],"r",1)))
        {
        		if((vty->node != HANSI_NODE)&&(vty->node != LOCAL_HANSI_NODE)){
			vty_out(vty, "%s,%d,invalid node:%d.",__func__,__LINE__,vty->node);
			return CMD_WARNING;
		}else{
		}
            ret = wid_no_interface_ifname_radio((char *)argv[0],vty);
            if (ret == CMD_SUCCESS)
            {
                return CMD_SUCCESS;
            }
            else
            {
                return CMD_WARNING;
            }
        }

		else 
#endif
#ifdef _D_WCPSS_
		if (!strncasecmp(argv[0],"ebr",3))
		{
			if((vty->node != HANSI_NODE)&&(vty->node != LOCAL_HANSI_NODE)){
				vty_out(vty, "%s,%d,invalid node:%d.",__func__,__LINE__,vty->node);
				return CMD_WARNING;
			}else{
			}
			   ret = wid_no_interface_ifname_ebr((char *)argv[0],vty);
		            if (ret == CMD_SUCCESS)
		            {
		                return CMD_SUCCESS;
		            }
		            else
		            {
		                return CMD_WARNING;
		            }
		}
        /*the other ifnames,add here*/
        else
#endif
        {


#ifdef _D_WCPSS_  /*zhanglei add*/
            /*add something wcpss used  */
#endif

            if (!strncmp(argv[0],"vlan",4))
            {
                if (CMD_SUCCESS != dcli_no_interface_ifname_vlan(vty,(char *)argv[0]))
                {
                    return CMD_WARNING;
                }
            }
            else if (!strncmp(argv[0],"eth",3) || !strncmp(argv[0],"mng",3))
            {
            	if (CMD_SUCCESS != dcli_no_interface_ifname_eth_port(vty,(char *)argv[0]))
            	{
                	return CMD_WARNING;
            	}
            }
			else if ((!strncmp(argv[0],"ve",2))||\
				(dcli_interface_new_eth_ifname_check((unsigned char *)argv[0])))
			{
				if (CMD_SUCCESS != dcli_no_interface_ifname_eth_port(vty,(char *)argv[0]))
                {
                    return CMD_WARNING;
                }
			}
			else if(!strncmp(tolower(argv[0]),"cscd", 4)) {
				unsigned char slotno = 0, portno = 0;
				if(NPD_SUCCESS == parse_slotport_no(argv[0], &slotno, &portno)) {
					return dcli_eth_port_mode_config(vty, 0, slotno, portno, ETH_PORT_FUNC_BRIDGE);
				}
				else {
					vty_out(vty, "%% Bad parameter %s!\n", argv[0]);
					return CMD_WARNING;
				}
			}
            else if (!strncmp(argv[0],"bond",4))
            {
				#if 0
                if (CMD_SUCCESS != dcli_no_interface_ifname_bond(vty,(char *)argv[0]))
                {
                    return CMD_WARNING;
                }
				#endif
				vty_out(vty, "%% Bad parameter %s!\n", argv[0]);
				return CMD_WARNING;
            }
            else
            {
                if (strcmp(argv[0],"lo") &&strncmp(argv[0],"sit",3))
                {
                    vty_out(vty,"%% Error IFNAME\n");
                    return CMD_WARNING;
                }
            }

            return CMD_SUCCESS;
        }

    }

      char * dcli_error_info_intf(int errorCode)
      {
          switch (errorCode)
          {
              case COMMON_SUCCESS :  /*0*/
                  return NULL;
              case COMMON_RETURN_CODE_BADPARAM:
                  return "%% Bad parameter!\n";
              case COMMON_RETURN_CODE_NULL_PTR:
                  return "%% Null pointer error!\n";
              case ARP_RETURN_CODE_ERROR:
                  return "%% Arp operation Failed!\n";
              case ARP_RETURN_CODE_CHECK_IP_ERROR:         /*0x20014*/
                  return "%% Check ip address FAILED!\n";
              case ARP_RETURN_CODE_STATIC_EXIST :
                  return "%% Static arp already exist!\n";
              case ARP_RETURN_CODE_NOTEXISTS:
                  return "%% Static arp not exist!\n";
              case ARP_RETURN_CODE_PORT_NOTMATCH :
                  return "%% The static arp not for this port!\n";
              case ARP_RETURN_CODE_BADPARAM :
                  return "%% Input bad parameter!\n";
              case ARP_RETURN_CODE_STATIC_ARP_FULL:
                  return "%% Static arp items already FULL!\n";
              case ARP_RETURN_CODE_HASH_OP_FAILED:
                  return "%% Arp hash operation FAILED!\n";
              case ARP_RETURN_CODE_VLAN_NOTEXISTS :
                  return "%% The vlan not exists!\n";
              case ARP_RETURN_CODE_MAC_MATCHED_BASE_MAC :
                  return "%% Can't add static-arp with system mac address!\n";
              case ARP_RETURN_CODE_NO_HAVE_ANY_IP :
                  return "%% The layer 3 interface not config ip address!\n";
              case ARP_RETURN_CODE_HAVE_THE_IP :
                  return "%% Can't config the same ip with the interface!\n";
              case ARP_RETURN_CODE_NOT_SAME_SUB_NET :
                  return "%% The ip is not the same subnet with the layer 3 interface!\n";
              case ARP_RETURN_CODE_TRUNK_NOT_EXISTS:
                  return "%% The trunk not exists!\n";
              case ARP_RETURN_CODE_TRUNK_NOT_IN_VLAN:
                  return "%% The trunk is not in the vlan!\n";
              case ARP_RETURN_CODE_MAC_MATCHED_INTERFACE_MAC:
                  return "%% Can't add static-arp with interface mac address!\n";
              case ARP_RETURN_CODE_NAM_ERROR:
                  return "%% Hardware operation FAILED when delete arp!\n";
              case ARP_RETURN_CODE_NO_VID:
                  return "%% Must input a vlan ID for this port!\n";
              case ARP_RETURN_CODE_PORT_OR_TRUNK_NEEDED:
                  return "%% Vlan interface,must input port number or trunk ID!\n";
              case ARP_RETURN_CODE_PORT_OR_TRUNK_NOT_NEEDED:
                  return "%% NOT need port number or trunk ID!\n";
              case ARP_RETURN_CODE_NO_SUCH_PORT:              /*0x2001D*/
              case INTERFACE_RETURN_CODE_NO_SUCH_PORT :       /*0x90019*/
                  return " %% No such port!\n";
              case ARP_RETURN_CODE_UNSUPPORTED_COMMAND :
              case INTERFACE_RETURN_CODE_UNSUPPORT_COMMAND: /*0x90004*/
                  return "%% Unsupport this command!\n";
              case ARP_RETURN_CODE_PORT_NOT_IN_VLAN:
              case INTERFACE_RETURN_CODE_PORT_NOT_IN_VLAN :
                  return "%% The port isn't in the vlan!\n";
              case ARP_RETURN_CODE_INTERFACE_NOTEXIST:
              case INTERFACE_RETURN_CODE_INTERFACE_NOTEXIST:  /*                     interface not existed*/
                  return "%% Interface not exists!\n";
			  case ARP_RETURN_CPU_INTERFACE_CODE_SUCCESS:   /*wangchao add*/
			  	  return "%% The arp cpu interface operation success!\n";
			  case ARP_RETURN_ARP_ENTRY_NOT_EXISTED:
			  	  return "%% The deleted arp entry not exist!\n";
			  case ARP_RETURN_CODE_SET_STALE_TIME_ERR:
			  	  return "%% The arp set stale time error\n";  	  
              case INTERFACE_RETURN_CODE_CHECK_MAC_ERROR:
                  return "%% Check interface's mac address FAILED!\n";
              case INTERFACE_RETURN_CODE_ERROR :
                  return "%% Interface operation Failed!\n";
              case INTERFACE_RETURN_CODE_INTERFACE_EXIST:  /*                          interface existed*/
                  return "%% Interface already exists!\n";
              case INTERFACE_RETURN_CODE_ROUTE_CREATE_SUBIF:
                  return "%% Not create route port sub interface!\n";
              case INTERFACE_RETURN_CODE_VLAN_IF_CREATE_SUBIF:
                  return "%% Not create vlan interface sub interface!\n";
              case INTERFACE_RETURN_CODE_ALREADY_ADVANCED:
                  return "%% Advanced-routing already enabled!\n";
              case INTERFACE_RETURN_CODE_NOT_ADVANCED:
                  return "%% Advanced-routing already disabled!\n";
              case INTERFACE_RETURN_CODE_PARENT_IF_NOTEXIST:    /*   parent  interface not existed*/
                  return "%% Parent interface not exists!\n";
              case INTERFACE_RETURN_CODE_SUBIF_EXISTS:        /*   promis sub interface existed*/
              case INTERFACE_RETURN_CODE_PORT_HAS_SUB_IF:
                  return "%% Sub interface exists! Please delete sub interface first.\n";
              case INTERFACE_RETURN_CODE_SUBIF_NOTEXIST:    /*  promis sub interface not existed*/
                  return "%% Sub interface not exists\n";
              case INTERFACE_RETURN_CODE_GET_SYSMAC_ERROR:
                  return "%% Get system mac FAILED!\n";
              case INTERFACE_RETURN_CODE_CONTAIN_PROMI_PORT:
                  return "%% Vlan contains promiscuous port!\n";
              case INTERFACE_RETURN_CODE_SUBIF_CREATE_FAILED:
                  return "%% Sub interface create failed!";
              case INTERFACE_RETURN_CODE_ADD_PORT_FAILED:
                  return "%% Add port to vlan FAILED while creating sub interface!\n";
              case INTERFACE_RETURN_CODE_QINQ_TWO_SAME_TAG:
                  return "%% The internal tag is the same as the external tag!\n"; /* permited now */
              case INTERFACE_RETURN_CODE_DEFAULT_VLAN_IS_L3_VLAN:
                  return "%% Advanced-routing enable Failed!vlan for advanced-routing is l3 interface\n";
              case INTERFACE_RETURN_CODE_TAG_IS_ADVACED_VID:
                  return "%% The tag can't be advanced-routing default vid!\n";
              case INTERFACE_RETURN_CODE_VLAN_NOTEXIST:
                  return "%% The vlan does not exist!\n";
              case INTERFACE_RETURN_CODE_ADVAN_VLAN_SET2_INTF:
                  return "%% The advanced-routing default vlan can't set l3 interface!\n";
              case INTERFACE_RETURN_CODE_DEL_PORT_FAILED:
                  return "%% Delete port FAILED when config interface!\n";
              case INTERFCE_RETURN_CODE_ADVANCED_VLAN_NOT_EXISTS:
                  return "%% Config advanced-routing default-vid first!\n";
              case INTERFACE_RETURN_CODE_CAN_NOT_SET2_EMPTY:
                  return "%% Eth-port advanced-routing interfaces exist!\n";
              case INTERFACE_RETURN_CODE_PORT_CONFLICT:
                  return "%% Port conflict,delete the port from vlan first!\n";
              case INTERFACE_RETURN_CODE_QINQ_TYPE_FULL:
                  return "%% Type values already reach MAX count!\n";
              case INTERFACE_RETURN_CODE_VLAN_IS_L3INTF:
                  return "%% The vlan is l3 interface,can't set to advanced-routing default-vid!\n";
			  case COMMON_PRODUCT_NOT_SUPPORT_FUCTION:
				  return "%% Product not support this function!\n";
			  case INTERFACE_RETURN_CODE_SHORTEN_IFNAME_NOT_SUPPORT:
			  	  return "%% This interface not support shorten ifname!\n";
			  case INTERFACE_RETURN_CODE_INTERFACE_ISSLAVE:
			  	  return "%% The interface is added to dynamic trunk,can't enable it now!\n";
              default:
                  return "%% Execute command failed!\n";
          }
      }


    unsigned int parse_param_ifName
    (
        char * ifName,
		unsigned char * slot_no,
		unsigned char * port_no,
        unsigned int  * vid
    )
    {
        char * endptr = NULL;
        unsigned int flag = 3;
        unsigned int vid2;
        if (0 == strncmp(ifName,"vlan",4))
        {
            *vid = (unsigned int)strtoul(ifName+4,&endptr,10);
            if ((*vid) != 0)
            {
                flag = 0;
            }
        }
        else if (0 == strncmp(ifName,"eth",3))
        {
    		if(NPD_SUCCESS == parse_slotport_tag_no(ifName+3,slot_no,port_no,vid,&vid2)){
                {
                    flag = 1;
                }
            }
        }
		else if(0 == strncmp(ifName, "cscd",4)) {
			if(NPD_SUCCESS == parse_slotport_no(ifName, slot_no, port_no)) {
				flag = 1;
			}
		}
		else if(dcli_interface_new_eth_ifname_check(ifName)){
			if(NPD_SUCCESS == parse_slotport_tag_no(ifName+1,slot_no,port_no,vid,&vid2)){
                {
                    flag = 2;
                }
            }
        }
        return flag;
    }
        
    DEFUN(show_advanced_routing_cmd_func,
          show_vlan_eth_port_advanced_routing_cmd,
          "show advanced-routing-interface (vlan|eth-port)",
          SHOW_STR
          "show advanced-routing interfaces' info\n"
          "show vlan advanced-routing interface info\n"
          "show eth-port advanced-routing interface info\n")
    {
        if (NULL == vty)
        {
            return CMD_WARNING;
        }
        if (argc > 1)
        {
            vty_out("%% Bad parameter: %s \n",argv[1]);
            return CMD_WARNING;
        }

        vty_out(vty,"==========================\n");
        vty_out(vty," Advanced-routing Config\n");
        vty_out(vty,"==========================\n");
        if ((0 == argc)||(0 == strncmp("eth-port",argv[0],strlen(argv[0]))))
        {
            dcli_intf_show_advanced_routing(vty,FALSE,TRUE);
        }
        if ((0 == argc)||(0 == strncmp("vlan",argv[0],strlen(argv[0]))))
        {
            dcli_intf_show_advanced_routing(vty,TRUE,FALSE);
        }
        vty_out(vty,"==========================\n");
        return CMD_SUCCESS;
    }
    DEFUN(config_advanced_routing_default_vid_cmd_func,
          config_advanced_routing_default_vid_cmd,
          "config advanced-routing default-vid (<1-4094>|empty)",
          CONFIG_STR
          "config advanced-routing's configuration\n"
          "config eth-port advanced-routing's default-vid\n"
          "set the default-vid to <1-4094>\n"
          "empty for clear the default-vid\n")
    {
        DBusMessage *query = NULL, *reply = NULL;
        DBusError err;
        unsigned short vid = 0;
        char ** endptr = NULL;
        int ret = COMMON_SUCCESS;
        if (NULL == vty)
        {
            return CMD_WARNING;
        }
        if (argc != 1)
        {
            vty_out(vty,"%% Bad parameter!\n");
            return CMD_WARNING;
        }
        if (0 == strncmp("empty",argv[0],strlen(argv[0])))
        {
            vid = 0;
        }
        else
        {
            vid = strtoul(argv[0],endptr,NULL);
            if (((0 >= vid)||(4094 < vid))||\
                ((NULL != endptr)&&('\0' != endptr[0])))
            {
                vty_out(vty,"%% Bad parameter %s\n",argv[0]);
                return CMD_WARNING;
            }
        }

        query = dbus_message_new_method_call(
                    NPD_DBUS_BUSNAME,                   \
                    NPD_DBUS_INTF_OBJPATH,      \
                    NPD_DBUS_INTF_INTERFACE,    \
                    NPD_DBUS_CONFIG_ADVANCED_ROUTING_DEFAULT_VID);

        dbus_error_init(&err);

        dbus_message_append_args(query,
                                 DBUS_TYPE_UINT16,&vid,
                                 DBUS_TYPE_INVALID);

        reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);

        dbus_message_unref(query);
        if (NULL == reply)
        {
            vty_out(vty,"Failed get reply.\n");
            if (dbus_error_is_set(&err))
            {
                vty_out(vty,"%s raised: %s",err.name,err.message);
                dbus_error_free_for_dcli(&err);
            }
            return CMD_WARNING;
        }

        if (dbus_message_get_args(reply, &err,
                                  DBUS_TYPE_UINT32, &ret,
                                  DBUS_TYPE_INVALID))
        {
            if (INTERFACE_RETURN_CODE_SUCCESS == ret)
            {
                dbus_message_unref(reply);
                return CMD_SUCCESS;
            }
            else
            {
                vty_out(vty,dcli_error_info_intf(ret));
            }
            dbus_message_unref(reply);
            return CMD_WARNING;
        }
        else
        {
            vty_out(vty,"Failed get args.\n");
            if (dbus_error_is_set(&err))
            {
                vty_out(vty,"%s raised: %s",err.name,err.message);
                dbus_error_free_for_dcli(&err);
            }
        }

        dbus_message_unref(reply);
        return CMD_SUCCESS;
    }
    DEFUN(show_advanced_routing_default_vid_cmd_func,
          show_advanced_routing_default_vid_cmd,
          "show advanced-routing default-vid",
          SHOW_STR
          "show advanced-routing's configuration\n"
          "show eth-port advanced-routing's default-vid\n")
    {
        DBusMessage *query = NULL, *reply = NULL;
        DBusError err;
        unsigned short vid = 0;
        char ** endptr = NULL;
        if (NULL == vty)
        {
            return CMD_WARNING;
        }

        query = dbus_message_new_method_call(
                    NPD_DBUS_BUSNAME,                   \
                    NPD_DBUS_INTF_OBJPATH,      \
                    NPD_DBUS_INTF_INTERFACE,    \
                    NPD_DBUS_SHOW_ADVANCED_ROUTING_DEFAULT_VID);

        dbus_error_init(&err);

        reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);

        dbus_message_unref(query);
        if (NULL == reply)
        {
            vty_out(vty,"Failed get reply.\n");
            if (dbus_error_is_set(&err))
            {
                vty_out(vty,"%s raised: %s",err.name,err.message);
                dbus_error_free_for_dcli(&err);
            }
            return CMD_WARNING;
        }

        if (dbus_message_get_args(reply, &err,
                                  DBUS_TYPE_UINT32, &vid,
                                  DBUS_TYPE_INVALID))
        {
            vty_out(vty,"=============\n");
            if (vid)
                vty_out(vty," VID: %d\n",vid);
            else
                vty_out(vty," VID: %s\n","empty");
            vty_out(vty,"-------------\n");
            dbus_message_unref(reply);
            return CMD_WARNING;
        }
        else
        {
            vty_out(vty,"Failed get args.\n");
            if (dbus_error_is_set(&err))
            {
                vty_out(vty,"%s raised: %s",err.name,err.message);
                dbus_error_free_for_dcli(&err);
            }
        }

        dbus_message_unref(reply);
        return CMD_SUCCESS;
    }

    ALIAS(show_advanced_routing_cmd_func,
          show_advanced_routing_cmd,
          "show advanced-routing-interface",
          SHOW_STR
         );
#ifndef INTF_BOUNDING_SHOW_STR_LEN
#define INTF_BOUNDING_SHOW_STR_LEN 1024
#endif
    int dcli_intf_bonding_show_running(void)
    {
    	int ret,len;
        int i;
        char *showStr = NULL;
        char ifname[INTERFACE_NAMSIZ+64]={0};
        char fileexiststr[128] = {0};
        FILE *fp;
        char buf[128]={0};
		char *p_mode;
		char *p_policy;
        char *tmp;
        char tmpShowArr[INTF_BOUNDING_SHOW_STR_LEN] = {'\0'};
		
        for (i = 0 ; i <= MAX_BONDID; i++)
        {
            sprintf(ifname,"/proc/net/bonding/bond%d",i);
            fp = fopen(ifname,"r");
            if (NULL == fp)
            {
                continue;
            }
            sprintf(tmpShowArr,"%sinterface bond%d\n",tmpShowArr,i);

            while (fgets(buf,128,fp))
            {
            	if (!strncmp(buf,"Bonding Mode:",strlen("Bonding Mode:")))
            	{
            		p_mode = buf + strlen("Bonding Mode:") + 1;
                	if (!strncmp(p_mode, "load balancing (round-robin)", strlen("load balancing (round-robin)")))
                        sprintf(tmpShowArr, "%s set mode balance-rr\n", tmpShowArr);
    				else if (!strncmp(p_mode,"IEEE 802.3ad", strlen("IEEE 802.3ad")))
                        sprintf(tmpShowArr, "%s set mode 802.3ad\n", tmpShowArr);
    				else if (!strncmp(p_mode, "adaptive load balancing", strlen("adaptive load balancing")))
                        sprintf(tmpShowArr, "%s set mode balance-alb\n", tmpShowArr);
    				else if (!strncmp(p_mode, "transmit load balancing", strlen("transmit load balancing")))
                        sprintf(tmpShowArr, "%s set mode mode balance-tlb\n", tmpShowArr);
    				else if (!strncmp(p_mode, "fault-tolerance (broadcast)", strlen("fault-tolerance (broadcast)")))
                        sprintf(tmpShowArr, "%s set mode broadcast\n", tmpShowArr);
    				else if (!strncmp(p_mode, "fault-tolerance (active-backup)", strlen("fault-tolerance (active-backup)")))
                        sprintf(tmpShowArr, "%s set mode active-backup\n", tmpShowArr);
    				else if (!strncmp(p_mode, "load balancing (xor)", strlen("load balancing (xor)")))
                        sprintf(tmpShowArr, "%s set mode balance-xor\n", tmpShowArr);
            	}

				if (!strncmp(buf,"Transmit Hash Policy:",strlen("Transmit Hash Policy:")))
				{
					p_policy = buf + strlen("Transmit Hash Policy:") + 1;
					if (!strncmp(p_policy, "layer2", strlen("layer2")))
                        sprintf(tmpShowArr, "%s set xmit_hash_policy layer2\n", tmpShowArr);
    				else if (!strncmp(p_policy,"layer3+4", strlen("layer3+4")))
                        sprintf(tmpShowArr, "%s set xmit_hash_policy layer3+4\n", tmpShowArr);
				}
				
				if (!strncmp(buf,"LACP rate:",strlen("LACP rate:")))
                {
                    tmp=buf+strlen("LACP rate:")+1;
                    sprintf(tmpShowArr,"%s set lacp_rate %s\n",tmpShowArr,tmp);
                }

                if (!strncmp(buf,"Slave Interface:",strlen("Slave Interface:")))
                {
                    tmp=buf+strlen("Slave Interface:")+1;
                    sprintf(tmpShowArr,"%s add bonding %s\n",tmpShowArr,tmp);
                }
                memset(buf,0,128);
            }

            sprintf(tmpShowArr,"%s exit\n",tmpShowArr);
            fclose(fp);

        }

        vtysh_add_show_string_parse(tmpShowArr);

    }

/*wangchao add 2012/5/3*/
#define STATIC_ARP_CONFIG_FILE_PATH "/var/run/static_arp_file"

int dcli_intf_interface_static_arp_show_running(struct vty* vty)
{
	char *start_pos;
	char *end_pos;
	void* data;
	int fd = -1;
	
	char * showStr ;
	showStr = malloc(256);
	struct stat sb;

	fd=open(STATIC_ARP_CONFIG_FILE_PATH,O_RDWR);
	
	if(fd < 0)
	{		
		free(showStr);
		return 0;
	}
	
	fstat(fd,&sb);
	data=mmap(NULL,sb.st_size,PROT_READ,MAP_PRIVATE,fd,0);
	if(MAP_FAILED==data)
	{
		free(showStr);
		close(fd);
		return -1;
	}	

	start_pos = data;
	end_pos = strstr(start_pos+strlen("interface"), "interface");
	
	while(end_pos)
	{
		memcpy(showStr, start_pos,end_pos-start_pos);
		showStr[end_pos - start_pos] = '\0';

		vtysh_add_show_string_parse(showStr);
		
		start_pos = end_pos;
		end_pos = strstr(start_pos+strlen("interface"), "interface");
	}
	
	showStr[0] = '\0';
	strcat(showStr,start_pos);
	vtysh_add_show_string_parse(showStr);
	
	free(showStr);
	munmap(data,sb.st_size);
	close(fd);
	return 0;
}

#if 0

    int dcli_intf_interface_static_arp_show_running
        (
            struct vty* vty
        )
        {
           
        unsigned char *showStr = NULL;
        DBusMessage *query, *reply;
	    DBusMessageIter	 iter;
        DBusError err;
        int ret = 0;
#ifndef INTF_ADVANCED_ROUTING_SHOW_STR_LEN
#define INTF_ADVANCED_ROUTING_SHOW_STR_LEN 30
#endif
        char * tmpShowStr = NULL;
        char tmpShowArr[INTF_ADVANCED_ROUTING_SHOW_STR_LEN] = {'\0'};
        unsigned int tmpStrLen = 0;
        unsigned int haveMore = 0;

        query = dbus_message_new_method_call(\
                                             NPD_DBUS_BUSNAME,           \
                                             NPD_DBUS_INTF_OBJPATH,      \
                                             NPD_DBUS_INTF_INTERFACE,            \
                                             NPD_DBUS_INTF_INTERFACE_STATIC_ARP_SHOW_RUNNING);

        dbus_error_init(&err);


        reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);

        dbus_message_unref(query);
        if (NULL == reply)
        {
            printf("failed get reply.\n");
            if (dbus_error_is_set(&err))
            {
                printf("%s raised: %s",err.name,err.message);
                dbus_error_free_for_dcli(&err);
            }
            return ret;
        }
        
	    dbus_message_iter_init(reply,&iter);
        dbus_message_iter_get_basic(&iter,&haveMore);
	    dbus_message_iter_next(&iter);	
        while(haveMore){
            dbus_message_iter_get_basic(&iter,&showStr);
            dbus_message_iter_next(&iter);
            vtysh_add_show_string_parse(showStr);
            
            dbus_message_iter_get_basic(&iter,&haveMore);
            dbus_message_iter_next(&iter);
        }
        ret = 0;
        dbus_message_unref(reply);
        return ret;
    }
#endif
    int dcli_intf_advanced_routing_show_running
    (
        void
    )
    {
        if (dcli_intf_show_advanced_routing(NULL,FALSE,FALSE))
        {
            return dcli_intf_show_advanced_routing(NULL,TRUE,FALSE);
        }
        else
        {
            return 0;
        }

    }
    int dcli_intf_show_advanced_routing
    (
        struct vty * vty,
        unsigned int vlanAdv,
        unsigned int includeRgmii
    )
    {

        char *showStr = NULL;
        DBusMessage *query, *reply;
        DBusError err;
        int ret = 0;
#ifndef INTF_ADVANCED_ROUTING_SHOW_STR_LEN
#define INTF_ADVANCED_ROUTING_SHOW_STR_LEN 30
#endif
        char * tmpShowStr = NULL;
        char tmpShowArr[INTF_ADVANCED_ROUTING_SHOW_STR_LEN] = {'\0'};
        unsigned int tmpStrLen = 0;

        query = dbus_message_new_method_call(\
                                             NPD_DBUS_BUSNAME,           \
                                             NPD_DBUS_INTF_OBJPATH,      \
                                             NPD_DBUS_INTF_INTERFACE,            \
                                             NPD_DBUS_INTF_ADVANCED_ROUTING_SAVE_CFG);

        dbus_error_init(&err);

        dbus_message_append_args(query,
                                 DBUS_TYPE_UINT32,&vlanAdv,
                                 DBUS_TYPE_UINT32,&includeRgmii,
                                 DBUS_TYPE_INVALID);

        reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);

        dbus_message_unref(query);
        if (NULL == reply)
        {
            printf("failed get reply.\n");
            if (dbus_error_is_set(&err))
            {
                printf("%s raised: %s",err.name,err.message);
                dbus_error_free_for_dcli(&err);
            }
            return ret;
        }
        if (dbus_message_get_args(reply, &err,
                                  DBUS_TYPE_STRING, &showStr,
                                  DBUS_TYPE_INVALID))
        {
            if (NULL != vty)
            {
                tmpShowStr = strtok(showStr,"\n");
                while (NULL != tmpShowStr)
                {
                    if (0 == strncmp(tmpShowStr,"interface",strlen("interface")))
                    {
                        vty_out(vty,"   %s\n",tmpShowStr);
                    }
                    tmpShowStr = strtok(NULL,"\n");
                }
            }
            else
            {
                vtysh_add_show_string_parse(showStr);
            }
            ret = 1;
        }
        else
        {
            ret = 0;
            printf("Failed get args.\n");
            if (dbus_error_is_set(&err))
            {
                printf("%s raised: %s",err.name,err.message);
                dbus_error_free_for_dcli(&err);
            }
        }
        dbus_message_unref(reply);
        return ret;
    }

int dcli_intf_eth_interface_enable_show_running
    (
        void
    )
    {

        char *showStr = NULL;
        DBusMessage *query, *reply;
        DBusError err;
        int ret = 0;

        query = dbus_message_new_method_call(\
                                             NPD_DBUS_BUSNAME,           \
                                             NPD_DBUS_INTF_OBJPATH,      \
                                             NPD_DBUS_INTF_INTERFACE,            \
                                             NPD_DBUS_INTF_ETH_INTERFACE_ENABLE_SET_SAVE_CFG);

        dbus_error_init(&err);

        reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);

        dbus_message_unref(query);
        if (NULL == reply)
        {
            printf("failed get reply.\n");
            if (dbus_error_is_set(&err))
            {
                printf("%s raised: %s",err.name,err.message);
                dbus_error_free_for_dcli(&err);
            }
            return ret;
        }
        if (dbus_message_get_args(reply, &err,
                                  DBUS_TYPE_STRING, &showStr,
                                  DBUS_TYPE_INVALID))
        {
            vtysh_add_show_string_parse(showStr);
            ret = 1;
        }
        else
        {
            ret = 0;
            printf("Failed get args.\n");
            if (dbus_error_is_set(&err))
            {
                printf("%s raised: %s",err.name,err.message);
                dbus_error_free_for_dcli(&err);
            }
        }
        dbus_message_unref(reply);
        return ret;
    }

    DEFUN(set_intf_qinq_type_cmd_func,
          set_intf_qinq_type_cmd,
          "config qinq-type VALUE",
          CONFIG_STR
          "config qinq-type value for qinq interface\n"
          "config qinq-type this value. eg. 0x88A8,...\n")
    {
        unsigned char intfName[INTERFACE_NAMSIZ+1] = {0};
        unsigned char type [16] = {0};
        unsigned char * ptr = NULL;
		unsigned long value = 0;

        if(argc < 1){
            vty_out(vty,"Command incompleted!\n");
            return CMD_WARNING;
        }
        if(argc == 1){
            memcpy(type,(char *)argv[0],strlen(argv[0])<16 ? strlen(argv[0]) : 15);
            memcpy(intfName, vlan_eth_port_ifname,INTERFACE_NAMSIZ);
        }
        ptr = type;
        while(*ptr != '\0'){
            if((*ptr != 'x')&&(*ptr != 'X')&&\
                ((*ptr < '0')||(*ptr > '9'))&&\
                ((*ptr < 'a')||(*ptr > 'f'))&&\
                ((*ptr < 'A')||(*ptr > 'F'))){
                vty_out(vty,"Bad Parameter %s! %s\n",(char *)argv[0],vlan_eth_port_ifname);
                return CMD_WARNING;
            }
            ptr++;
        }
		value = strtoul(type,NULL,0);
		if((value < 0x600) || (value > 0xffff))
		{
			vty_out(vty,"Bad Parameter %s! Please input a number between 0x600 and 0xFFFF\n",(char *)argv[0]);
            return CMD_WARNING;
		}
        return dcli_intf_subif_set_qinq_type(vty,intfName,type);
        
    }

int dcli_intf_subif_set_qinq_type
(
    struct vty * vty,
    unsigned char *intfName,
    unsigned char *type
)
{

    DBusMessage *query, *reply;
    DBusError err;
    unsigned char slot = 0;
    unsigned char port = 0;
	unsigned char cpu_no = 0;
	unsigned char cpu_port_no = 0;
    unsigned int  vid1 = 0;
    unsigned int  vid2 = 0;
    int ret = 0,ifnameType = CONFIG_INTF_ETH;
	int local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
	int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
	int product_type = get_product_info(SEM_PRODUCT_TYPE_PATH);
	int function_type = -1;
	char file_path[64] = {0};
	int i=0;


    if(is_distributed == DISTRIBUTED_SYSTEM)
    {

        if(!strncmp(intfName,"eth",3))
        {
            ifnameType = CONFIG_INTF_ETH;
            ret = parse_slotport_tag_no((char *)(intfName + 3),&slot,&port,&vid1,&vid2);
            if(CMD_SUCCESS!= ret)
            {
                vty_out(vty,"%% Bad parameter %s!\n",intfName);
                return CMD_WARNING;
            }
        }
        else if(!strncmp(intfName,"mng",3))
        {
            ifnameType = CONFIG_INTF_MNG;
            ret = parse_slotport_tag_no((char *)(intfName + 3),&slot,&port,&vid1,&vid2);
            if(CMD_SUCCESS!= ret)
            {
                vty_out(vty,"%% Bad parameter %s!\n",intfName);
                return CMD_WARNING;
            }
        }
        else if(!strncmp(intfName,"ve",2))
        {
            ifnameType = CONFIG_INTF_VE;
			ret = parse_ve_slot_cpu_tag_no((char *)(intfName + 2),&slot,&cpu_no,&cpu_port_no,&vid1, &vid2);
            if(CMD_SUCCESS!= ret)
            {
                vty_out(vty,"%% Bad parameter %s!\n",intfName);
                return CMD_WARNING;
            }
        }
		else if(dcli_interface_new_eth_ifname_check(intfName))
        {
            ifnameType = CONFIG_INTF_E;
			ret = parse_slotport_tag_no((char *)(intfName + 1),&slot,&port,&vid1,&vid2);
            if(CMD_SUCCESS != ret)
            {
                vty_out(vty,"%% Bad parameter %s!\n",intfName);
                return CMD_WARNING;
            }
		}
        else
        {
            vty_out(vty,"%% Unsupport this command!\n");
            return CMD_WARNING;
        }
		if((!vid1)||((vid1)&&(vid2)))/*judgement for the given interface is a subintf or not*/
		{
			vty_out(vty,"%% Unsupport this command!\n");
            return CMD_WARNING;
		}
		for(i = 1; i <= slotNum; i++)
		{
			sprintf(file_path,"/dbm/product/slot/slot%d/function_type", i);
			function_type = get_product_info(file_path);
				
			if (function_type != SWITCH_BOARD)
			{
		        query = dbus_message_new_method_call(\
		                                                 SEM_DBUS_BUSNAME,           \
		                                                 SEM_DBUS_OBJPATH,      \
		                                                 SEM_DBUS_INTERFACE,            \
		                                                 SEM_DBUS_SUB_INTERFACE_ETHPORT_QINQ_TYPE_SET);
		        dbus_error_init(&err);
		        
		        dbus_message_append_args(query,
										 DBUS_TYPE_UINT32,&ifnameType,
		                                 DBUS_TYPE_UINT16,&slot,
		                                 DBUS_TYPE_UINT16,&port,
		                                 DBUS_TYPE_UINT16,&cpu_no,
		                                 DBUS_TYPE_UINT16,&cpu_port_no,
		                                 DBUS_TYPE_UINT32,&vid1,
		                                 DBUS_TYPE_UINT32,&vid2,                                     
		                                 DBUS_TYPE_STRING,&type,
		                                 DBUS_TYPE_INVALID);

		        
		        if(NULL == dbus_connection_dcli[i]->dcli_dbus_connection) 				
    			{
        			if(i == local_slot_id)
    				{
                        reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
    				}
    				else 
    				{	
    				   	vty_out(vty,"Can not connect to slot:%d \n",i);	
    					continue;
    				}
                }
    			else
    			{
                    reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[i]->dcli_dbus_connection,query,-1, &err);				
    			}
		        

		        dbus_message_unref(query);
		        if (NULL == reply)
		        {
		            printf("failed get reply.\n");
		            if (dbus_error_is_set(&err))
		            {
		                printf("%s raised: %s",err.name,err.message);
		                dbus_error_free_for_dcli(&err);
		            }
		            return CMD_WARNING;
		        }
		        if (dbus_message_get_args(reply, &err,
		                                  DBUS_TYPE_UINT32, &ret,
		                                  DBUS_TYPE_INVALID))
		        {
		            if(INTERFACE_RETURN_CODE_SUCCESS != ret){
		                vty_out(vty,dcli_error_info_intf(ret));
		            }
		            dbus_message_unref(reply);
		            continue;
		        }
		        else
		        {
		            printf("Failed get args.\n");
		            if (dbus_error_is_set(&err))
		            {
		                printf("%s raised: %s",err.name,err.message);
		                dbus_error_free_for_dcli(&err);
		            }
		        }
		        dbus_message_unref(reply);
			}
			else
			{
				continue;
			}
		}
        return ret;
    }

    query = dbus_message_new_method_call(\
                                                 NPD_DBUS_BUSNAME,           \
                                                 NPD_DBUS_INTF_OBJPATH,      \
                                                 NPD_DBUS_INTF_INTERFACE,            \
                                                 NPD_DBUS_INTF_SUBIF_SET_QINQ_TYPE);
    
    dbus_error_init(&err);
    if(!strncmp(intfName,"eth",3)){
        ret = parse_slotport_tag_no((char *)(intfName + 3),&slot,&port,&vid1,&vid2);
        if(NPD_SUCCESS != ret){
            vty_out(vty,"%% Bad parameter %s!\n",intfName);
            dbus_message_unref(query);
            return CMD_WARNING;
        }
    }
	else if(dcli_interface_new_eth_ifname_check(intfName)){
		ret = parse_slotport_tag_no((char *)(intfName + 1),&slot,&port,&vid1,&vid2);
		ifnameType = 1;
        if(NPD_SUCCESS != ret){
            vty_out(vty,"%% Bad parameter %s!\n",intfName);
            dbus_message_unref(query);
            return CMD_WARNING;
        }
	}
    else{
        vty_out(vty,"%% Unsupport this command!\n");
        dbus_message_unref(query);
        return CMD_WARNING;
    }
    
    dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&ifnameType,
                             DBUS_TYPE_UINT16,&slot,
                             DBUS_TYPE_UINT16,&port,
                             DBUS_TYPE_UINT32,&vid1,
                             DBUS_TYPE_UINT32,&vid2,                                     
                             DBUS_TYPE_STRING,&type,
                             DBUS_TYPE_INVALID);

    reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);

    dbus_message_unref(query);
    if (NULL == reply)
    {
        printf("failed get reply.\n");
        if (dbus_error_is_set(&err))
        {
            printf("%s raised: %s",err.name,err.message);
            dbus_error_free_for_dcli(&err);
        }
        return CMD_WARNING;
    }
    if (dbus_message_get_args(reply, &err,
                              DBUS_TYPE_UINT32, &ret,
                              DBUS_TYPE_INVALID))
    {
        if(INTERFACE_RETURN_CODE_SUCCESS != ret){
            vty_out(vty,dcli_error_info_intf(ret));
        }
        dbus_message_unref(reply);
        return CMD_SUCCESS;
    }
    else
    {
        printf("Failed get args.\n");
        if (dbus_error_is_set(&err))
        {
            printf("%s raised: %s",err.name,err.message);
            dbus_error_free_for_dcli(&err);
        }
    }
    dbus_message_unref(reply);
    return ret;
}
        
        int dcli_config_qinq_type_show_running
         (
             struct vty * vty
         )
        {   
        char *showStr = NULL;
        DBusMessage *query, *reply;
        DBusError err;
        int ret = 0;

        query = dbus_message_new_method_call(\
                                             NPD_DBUS_BUSNAME,           \
                                             NPD_DBUS_INTF_OBJPATH,      \
                                             NPD_DBUS_INTF_INTERFACE,            \
                                             NPD_DBUS_INTF_SET_QINQ_TYPE_SAVE_CFG);

        dbus_error_init(&err);

        reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);

        dbus_message_unref(query);
        if (NULL == reply)
        {
            printf("failed get reply.\n");
            if (dbus_error_is_set(&err))
            {
                printf("%s raised: %s",err.name,err.message);
                dbus_error_free_for_dcli(&err);
            }
            return CMD_WARNING;
        }
        if (dbus_message_get_args(reply, &err,
                                  DBUS_TYPE_STRING, &showStr,
                                  DBUS_TYPE_INVALID))
        {
            if (NULL != vty)            
            {
                vtysh_add_show_string_parse(showStr);
            }
            ret = CMD_SUCCESS;
        }
        else
        {
            ret = CMD_WARNING;
            printf("Failed get args.\n");
            if (dbus_error_is_set(&err))
            {
                printf("%s raised: %s",err.name,err.message);
                dbus_error_free_for_dcli(&err);
            }
        }
        dbus_message_unref(reply);
        return ret;
             
        }


    extern int dcli_static_arp_show_running_config(struct vty* vty);
    struct cmd_node static_arp_node =
    {
        STATIC_ARP_NODE,
        "	",
        1,
    };
    struct cmd_node qinq_type_node =
    {
        QINQ_TYPE_NODE,
        "	",
        1,
    };
void  dcli_qinq_type_init()
{
    install_node(&qinq_type_node,dcli_config_qinq_type_show_running, "QINQ_TYPE_NODE");
    install_element(INTERFACE_NODE, &set_intf_qinq_type_cmd);    
}

    DEFUN(set_intf_change_intf_name_cmd_func,
          set_intf_change_intf_name_cmd,
          "(shorten|regular) ifname",
          "config interface-name to shorten format,as e1-11.11\n"
          "config interface-name to regular format,as eth1-11.11\n"
          "config interface-name format\n")
    {/*******************************************************************
    		Not register command:
    			this command would cause the old ip address lose in show running 
			and if you config the second ip address after change name the ifconfig 
			command would list two rename interface each with one ip address 
	   *******************************************************************/
            DBusMessage *query, *reply;
            DBusError err;
			unsigned int ret = 0;
			int ifnameType = 0;
			unsigned char slot = 0, port = 0;
			unsigned int tag1 = 0,tag2 = 0;
			unsigned char newIfname[INTERFACE_NAMSIZ + 1] = {0};
            query = dbus_message_new_method_call(\
                                                 NPD_DBUS_BUSNAME,           \
                                                 NPD_DBUS_INTF_OBJPATH,      \
                                                 NPD_DBUS_INTF_INTERFACE,            \
                                                 NPD_DBUS_INTF_CHANGE_INTF_NAME);
            dbus_error_init(&err);
            if(1 == argc){
				if(!strncmp((unsigned char *)argv[0],"shorten",strlen((unsigned char*)argv[0]))){
					ifnameType = 1;
				}
				else if(!strncmp((unsigned char *)argv[0],"regular",strlen((unsigned char*)argv[0]))){
					ifnameType = 0;
				}
				else{
					vty_out(vty,"%% Bad parameter %s\n",argv[0]);
					return CMD_WARNING;
				}
            }
			else{
				vty_out(vty,"%% Bad parameter number!\n");
				return CMD_WARNING;
			}
			if(!strncmp(vlan_eth_port_ifname,"eth",3)){/* regular eth ifname*/
				  ret = parse_slotport_tag_no(vlan_eth_port_ifname+3,&slot,&port,&tag1,&tag2);
		          if(COMMON_SUCCESS != ret){
		              vty_out(vty,"%% Bad parameter: ifname %S\n",vlan_eth_port_ifname);
		              return CMD_WARNING;
		          }
				  sprintf(newIfname,"e%s",(vlan_eth_port_ifname + 3));
				  if((tag1)&&(!tag2)&&(!ifnameType)){/* already regular,do nothing*/
				  	  return CMD_SUCCESS;
				  }
			}
			else if(dcli_interface_new_eth_ifname_check(vlan_eth_port_ifname)){/*shorten eth ifname*/
				ret = parse_slotport_tag_no(vlan_eth_port_ifname+1,&slot,&port,&tag1,&tag2);
    	        if(COMMON_SUCCESS != ret){
    	            vty_out(vty,"%% Bad parameter: ifname %S\n",vlan_eth_port_ifname);
    	            return CMD_WARNING;
    	        }
				sprintf(newIfname,"eth%s",(vlan_eth_port_ifname + 1));
				if((tag1)&&(!tag2)&&(ifnameType)){/* already regular,do nothing*/
					return CMD_SUCCESS;
				}
			}
			else{
				vty_out(vty,"%% This interface not support shorten ifname!\n");
				return CMD_WARNING;
			}
            dbus_message_append_args(query,
                                     DBUS_TYPE_UINT32,&ifnameType,
                                     DBUS_TYPE_BYTE,&slot,
                                     DBUS_TYPE_BYTE,&port,
                                     DBUS_TYPE_UINT32,&tag1,
                                     DBUS_TYPE_UINT32,&tag2,
                                     DBUS_TYPE_INVALID);
            reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);
            dbus_message_unref(query);
            if (NULL == reply)
            {
                printf("failed get reply.\n");
                if (dbus_error_is_set(&err))
                {
                    printf("%s raised: %s",err.name,err.message);
                    dbus_error_free_for_dcli(&err);
                }
                return CMD_WARNING;
            }
            if (dbus_message_get_args(reply, &err,
                                      DBUS_TYPE_UINT32, &ret,
                                      DBUS_TYPE_INVALID))
            {
                if(INTERFACE_RETURN_CODE_SUCCESS != ret){
                    vty_out(vty,dcli_error_info_intf(ret));
	                dbus_message_unref(reply);
	                return CMD_WARNING;
                }
				memcpy(vlan_eth_port_ifname,newIfname,INTERFACE_NAMSIZ);
                dbus_message_unref(reply);
                return CMD_SUCCESS;
            }
            else
            {
                printf("Failed get args.\n");
                if (dbus_error_is_set(&err))
                {
                    printf("%s raised: %s",err.name,err.message);
                    dbus_error_free_for_dcli(&err);
                }
            }
            dbus_message_unref(reply);
            return ret;
    }
	int dcli_interface_new_eth_ifname_check(unsigned char * ifname)
	{
		if(!ifname){
			return FALSE;
		}
		if(('e' == ifname[0])&&\
			(('0' <= ifname[1])&&('9' >= ifname[1]))&&\
			('-' == ifname[2])&&\
			(('0' <= ifname[3])&&('9' >= ifname[3]))){
			return TRUE;
		}
		return FALSE;
	}

	int dcli_interface_eth_interface_l3_enable_set
	(
		struct vty * vty,
		unsigned char * ifname,
		unsigned int 	enable
	)
	{
		int ret = 0;
        int i = 0;
        unsigned char slot = 0;
        unsigned char port = 0;
        unsigned int tag1 = 0;
        unsigned int tag2 = 0;
		char * tmpPtr = NULL;
		
        if (0 == strncmp(ifname,"eth",3)){
			tmpPtr = ifname + 3;
        }
		else if(dcli_interface_new_eth_ifname_check(ifname)){
			tmpPtr = ifname + 1;
		}
		else
        {
			vty_out(vty,"%% This interface unsupport this command!\n");
            return CMD_SUCCESS;
        }
        char *id = (char *)malloc(sizeof(char)*25);
        if (id == NULL)
        {
            return CMD_WARNING;
        }
        memset(id,0,25);
        memcpy(id,tmpPtr,(strlen(tmpPtr)));
        for (i = 0; i<strlen(tmpPtr); i++)
        {
            if ((id[i] != '-')&&((id[i] != '.')&&((id[i]!='\0')&&((id[i]<'0')||(id[i]>'9')))))
            {
                vty_out(vty,"%% Bad parameter: %s !\n",ifname);
                free(id);
                id = NULL;
                return CMD_WARNING;
            }
        }
        ret = parse_slotport_tag_no(id,&slot,&port,&tag1,&tag2);
        free(id);
        id = NULL;
        if ((COMMON_SUCCESS != ret)||(tag1>4095)||(tag2 > 4095))
        {
            vty_out(vty,"%% Bab parameter: %s !\n",ifname);
            return CMD_WARNING;
        }
		return dcli_interface_dbus_eth_interface_enable_set(vty, slot, port, tag1, tag2, enable);
	}

	int dcli_interface_dbus_eth_interface_enable_set
	(
		struct vty * vty,
		unsigned char slot,
		unsigned char port,
		unsigned int tag1,
		unsigned int tag2,
		unsigned int enable
	)
	{
			DBusMessage *query, *reply;
			DBusError err;
			unsigned int ret = 0;
			
            query = dbus_message_new_method_call(\
                                                 NPD_DBUS_BUSNAME,           \
                                                 NPD_DBUS_INTF_OBJPATH,      \
                                                 NPD_DBUS_INTF_INTERFACE,            \
                                                 NPD_DBUS_INTF_ETH_INTERFACE_ENABLE_SET);
            dbus_error_init(&err);
           
            dbus_message_append_args(query,
                                     DBUS_TYPE_BYTE,&slot,
                                     DBUS_TYPE_BYTE,&port,
                                     DBUS_TYPE_UINT32,&tag1,
                                     DBUS_TYPE_UINT32,&tag2,
                                     DBUS_TYPE_UINT32,&enable,                                     
                                     DBUS_TYPE_INVALID);
            reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);
            dbus_message_unref(query);
            if (NULL == reply)
            {
                printf("failed get reply.\n");
                if (dbus_error_is_set(&err))
                {
                    printf("%s raised: %s",err.name,err.message);
                    dbus_error_free_for_dcli(&err);
                }
                return CMD_WARNING;
            }
            if (dbus_message_get_args(reply, &err,
                                      DBUS_TYPE_UINT32, &ret,
                                      DBUS_TYPE_INVALID))
            {
                if(INTERFACE_RETURN_CODE_SUCCESS != ret){
                    vty_out(vty,dcli_error_info_intf(ret));
	                dbus_message_unref(reply);
	                return CMD_WARNING;
                }
                dbus_message_unref(reply);
                return CMD_SUCCESS;
            }
            else
            {
                printf("Failed get args.\n");
                if (dbus_error_is_set(&err))
                {
                    printf("%s raised: %s",err.name,err.message);
                    dbus_error_free_for_dcli(&err);
                }
            }
            dbus_message_unref(reply);
            return CMD_WARNING;		
	}

/*********************************************************************
 *	DISCRIPT:
 *		set eth port interface hardware enable/disable 
 *			and set the port to L3 vlan or default vlan
 *	INPUT:
 *		slot -uint8
 *		port -uint8
 *		tag1 - uint32
 *		tag2 - uint32
 *		enable - uint32
 *	OUTPUT:
 *		ret  - uint32	return code
 *			INTERFACE_RETURN_CODE_SUCCESS
 *			INTERFACE_RETURN_CODE_NAM_ERROR
 *			INTERFACE_RETURN_CODE_FDB_SET_ERROR
 *			INTERFACE_RETURN_CODE_UNSUPPORT_COMMAND
 *			INTERFACE_RETURN_CODE_ERROR
 *
 **********************************************************************/
	DEFUN(set_eth_intf_l3_enable_cmd_func,
          set_eth_intf_l3_enable_cmd,
          "l3-function (enable|disable)",
          "config interface l3 function\n"
          "config interface l3 function enable\n"
          "config interface l3 function disable\n")
    {
		unsigned int enable = 2;
		unsigned int arglen = 0;
		
		if(1 != argc){
			vty_out(vty,"%% Bad parameter number!\n");
		}
		arglen = strlen((char*)argv[0]);
		if(!strncmp((char *)argv[0], "enable", arglen)){
			enable = 1;
		}
		else if(!strncmp((char *)argv[0], "disable", arglen)){
			enable = 0;
		}
		else{
			vty_out(vty,"%% Bad parameter %s!",(char *)argv[0]);
			return CMD_WARNING;
		}
		return dcli_interface_eth_interface_l3_enable_set(vty, vlan_eth_port_ifname, enable);
	}

extern int sync_file(char* temp,int syn_to_blk);

int ve_ifname_transfer_conversion(char *new_ifname, const char *original_ifname)
{
	int slotNum, portNum, vlanNum;
	char tmpStr[32];

	memcpy(tmpStr, original_ifname, strlen(original_ifname));

	if (!strncmp(tmpStr, "ve", 2)) {
		if (tmpStr[4] == 'f' || tmpStr[4] == 's') {
			memcpy(new_ifname, original_ifname, strlen(original_ifname));
			return 0;
		}
		else {
			sscanf(original_ifname,"ve%d.%d", &slotNum, &vlanNum);
			sprintf(new_ifname, "ve%02df%d.%d", slotNum, 1, vlanNum);
			return 0;
		}
	}
	else {
		/*
		vty_out(vty, "[%s] is not a ve-interface name.\n", original_ifname);
		*/
		memcpy(new_ifname, original_ifname, strlen(original_ifname));
		
		return 0;
	}

	return 0;
}

DEFUN(set_intf_pkt_refwd_cmd_func,
	set_intf_pkt_refwd_cmd,
	"packet-reforward (srcip|dstip) (A.B.C.D/M | all) slot SLOT_ID (enable|disable) [ipv6]",
	"IP packet-reforward function for distributed product\n"
	"Packets reforwarded according to source IP address in IP packet\n"
	"Packets reforwarded according to destination IP address in IP packet\n"
	"Specific IP address\n"
	"All packets\n"
	"Packets reforward to slot\n"
	"Slot id on product\n"
	"enable function\n"
	"disable function\n"
	"ipv6 standard\n")
{
	FILE *fd;
	int src_dst_mode; /* for 0 : src mode, for 1 : dst mode */
	int slot_count, slot_id;
	int opt;
	int new_opt;
	int command_send_to;
	int ret;
	char ifname[64] = {0};

	unsigned int board_on_mask;
	int i;
	char rulestr[128] = {0};
	char tempstr[128] = {0};
	char *tempstr1, *tempstr2;
	char cmdstr[256] = {0};
	pid_t status;
	int replace_flag = 0;

	/* after set pfm rules and rpa broadcast-mask, the is_active_master need get value again , caojia added*/
	fd = fopen("/dbm/local_board/is_active_master", "r");
	if (fd == NULL)
	{
		fprintf(stderr,"Get production information [1] error\n");
		return -1;
	}
	fscanf(fd, "%d", &is_active_master);
	fclose(fd);

	if ((is_active_master != 1) || (is_distributed == NON_DISTRIBUTED_SYSTEM))
	{
		vty_out(vty, "This command is only surpported by distributed system and only on active master board\n");

		return CMD_SUCCESS;
	}

	fd = fopen("/dbm/product/slotcount", "r");
	if (fd == NULL)
	{
		fprintf(stderr, "Get production information slotcount error\n");
		return CMD_WARNING;
	}
	fscanf(fd, "%d", &slot_count);
	fclose(fd);

	fd = fopen("/dbm/product/board_on_mask", "r");
	if (fd == NULL)
	{
		fprintf(stderr, "Get production information board_on_mask error\n");
		return CMD_WARNING;
	}
	fscanf(fd, "%d", &board_on_mask);
	fclose(fd);

	/* check the slot_id */
	slot_id = atoi(argv[2]);
	if(slot_id <= 0 || slot_id > slot_count)
	{
		vty_out(vty, "illegal slot_id! Select it between 1 - %d on the current product.\n", slot_count);
		return CMD_WARNING;
	}

	if (!(board_on_mask & (1 << (slot_id - 1)))) {
		vty_out(vty, "No board on slot %d.\n", slot_id);
		return CMD_WARNING;
	}

	vty_out(vty, "Interface name : %s\n", dcli_vty_ifname);
	ve_ifname_transfer_conversion(ifname, dcli_vty_ifname);
	vty_out(vty, "Interface name : %s\n", ifname);
	command_send_to = get_slot_num_dcli(ifname);

	/* srcip or dstip */
	if (strncmp(argv[0], "srcip", (strlen(argv[0]))) == 0) {
		src_dst_mode = 0;
	}
	else if (strncmp(argv[0], "dstip", (strlen(argv[0]))) == 0) {
		src_dst_mode = 1;
	}
	
	/* enable or disable */
	if (strncmp(argv[3], "enable", (strlen(argv[3]))) == 0) {
		if (argc == 4) {
			opt = 0;
		}else if (argc == 5) {
			opt = 10;
		}
	} 
	else if (strncmp(argv[3], "disable", (strlen(argv[3]))) == 0) {
		if (argc == 4) {
			opt = 1;
		}else if (argc == 5) {
			opt = 11;
		}
	}

	/* check the conf file is created ? */
	memset(cmdstr, 0, 256);
	sprintf(cmdstr, "sudo touch /var/run/pfm.conf > /dev/null 2>&1");
	system(cmdstr);

	/* check if the rule already exist. */
	if (argc == 4) {
		sprintf(tempstr, "%s:4:packet-reforward %s %s slot %s %s", ifname, argv[0], argv[1], argv[2], argv[3]);
	}
	else {
		sprintf(tempstr, "%s:6:packet-reforward %s %s slot %s %s %s", ifname, argv[0], argv[1], argv[2], argv[3], argv[4]);
	}
	memset(cmdstr, 0, 256);
	sprintf(cmdstr, "grep '%s' /var/run/pfm.conf > /dev/null 2>&1", tempstr);

	status = system(cmdstr);
	if (status != -1) {
		if (WIFEXITED(status)) {
			if (0 == WEXITSTATUS(status)) {
				vty_out(vty, "The packet-reforward rule already exist.\n");
				return CMD_SUCCESS;
			}
		}
	}

	/* if cmd is disable, check if the corresponding enable rule has set */
	if (opt == 1 || opt == 11) {
		memset(tempstr, 0, 128);
		if (opt == 1)
			sprintf(tempstr,  "%s:4:packet-reforward %s %s slot %s enable", ifname, argv[0], argv[1], argv[2]);
		else
			sprintf(tempstr, "%s:6:packet-reforward %s %s slot %s enable %s", ifname, argv[0], argv[1], argv[2], argv[4]);

		memset(cmdstr, 0, 256);
		sprintf(cmdstr, "grep '%s' /var/run/pfm.conf > /dev/null 2>&1", tempstr);
		status = system(cmdstr);
		if (status != -1) {
			if (WIFEXITED(status)) {
				if (WEXITSTATUS(status)) {
					vty_out(vty, "The corresponding rule \"%s\" not exist.\n", tempstr);
					vty_out(vty, "Can't set this rule.\n");
					return CMD_SUCCESS;
				}
			}
		}
	}

	/* set pfm rules */
	if (src_dst_mode) {
		ret = dcli_communicate_pfm_by_dbus(opt, 0, 6, ifname, 0, 0, slot_id, "all", (char*)argv[1],command_send_to);/*use dbus send message to pfm*/
		if(ret != 0) {
			vty_out(vty, "Rules of packet-reforward set failed.\n");
			return CMD_WARNING;
		}
		ret = dcli_communicate_pfm_by_dbus(opt, 0, 17, ifname, 0, 0, slot_id, "all", (char*)argv[1],command_send_to);/*use dbus send message to pfm*/
		if(ret != 0) {
			vty_out(vty, "Rules of packet-reforward set failed.\n");
			return CMD_WARNING;
		}
		ret = dcli_communicate_pfm_by_dbus(opt, 0, 1, ifname, 0, 0, slot_id, "all", (char*)argv[1],command_send_to);/*use dbus send message to pfm*/
		if(ret != 0) {
			vty_out(vty, "Rules of packet-reforward set failed.\n");
			return CMD_WARNING;
		}
	}
	else {
		ret = dcli_communicate_pfm_by_dbus(opt, 0, 6, ifname, 0, 0, slot_id, (char*)argv[1], "all", command_send_to);/*use dbus send message to pfm*/
		if(ret != 0) {
			vty_out(vty, "Rules of packet-reforward set failed.\n");
			return CMD_WARNING;
		}
		ret = dcli_communicate_pfm_by_dbus(opt, 0, 17, ifname, 0, 0, slot_id, (char*)argv[1], "all", command_send_to);/*use dbus send message to pfm*/
		if(ret != 0) {
			vty_out(vty, "Rules of packet-reforward set failed.\n");
			return CMD_WARNING;
		}
		ret = dcli_communicate_pfm_by_dbus(opt, 0, 1, ifname, 0, 0, slot_id, (char*)argv[1], "all", command_send_to);/*use dbus send message to pfm*/
		if(ret != 0) {
			vty_out(vty, "Rules of packet-reforward set failed.\n");
			return CMD_WARNING;
		}
	}

	/* update pfm.conf */
	if (opt == 0 || opt == 10) {
		if (argc == 4) {
			/* if the same ip rules had set */
			memset(cmdstr, 0, 256);
			sprintf(cmdstr, 
				"grep '%s' /var/run/pfm.conf | grep '%s' | grep '%s' | grep '%s' | grep -v 'ipv6' > /dev/null 2>&1",
				ifname, argv[0], argv[1], argv[3]);
			status = system(cmdstr);
			if (0 == WEXITSTATUS(status)) {
				memset(cmdstr, 0, 256);
				memset(tempstr, 0, 128);
				sprintf(tempstr, "%s:4:packet-reforward %s %s", ifname, argv[0], argv[1]);
				if (strncmp(argv[1], "all", (strlen(argv[1]))) == 0) {
					sprintf(cmdstr, "sed -i '/%s/ d' /var/run/pfm.conf > /dev/null 2>&1", tempstr);
				}
				else {
					tempstr1 = strtok(tempstr, "/");
					tempstr2 = strtok(NULL, "/");
					sprintf(cmdstr, "sed -i '/%s\\/%s/ d' /var/run/pfm.conf > /dev/null 2>&1", tempstr1, tempstr2);
				}
				system(cmdstr);
				replace_flag = 1;
			}

			/* new ip rules */
			memset(cmdstr, 0, 256);
			sprintf(rulestr, "%s:4:packet-reforward %s %s slot %s %s", ifname, argv[0], argv[1], argv[2], argv[3]);
			sprintf(cmdstr, "echo %s >> /var/run/pfm.conf", rulestr);
			system(cmdstr);
		}
		else {
			/* if the same ip rules had set */
			memset(cmdstr, 0, 256);
			sprintf(cmdstr, 
				"grep '%s' /var/run/pfm.conf | grep '%s' | grep '%s' | grep '%s' | grep 'ipv6' > /dev/null 2>&1",
				ifname, argv[0], argv[1], argv[3]);
			status = system(cmdstr);
			if (0 == WEXITSTATUS(status)) {
				memset(cmdstr, 0, 256);
				memset(tempstr, 0, 128);
				sprintf(tempstr, "%s:6:packet-reforward %s %s", ifname, argv[0], argv[1]);
				if (strncmp(argv[1], "all", (strlen(argv[1]))) == 0) {
					sprintf(cmdstr, "sed -i '/%s/ d' /var/run/pfm.conf > /dev/null 2>&1", tempstr);
				}
				else {
					tempstr1 = strtok(tempstr, "/");
					tempstr2 = strtok(NULL, "/");
					sprintf(cmdstr, "sed -i '/%s\\/%s/ d' /var/run/pfm.conf > /dev/null 2>&1", tempstr1, tempstr2);
				}
				system(cmdstr);
			}

			/* new ip rules */
			memset(cmdstr, 0, 256);
			sprintf(rulestr, "%s:6:packet-reforward %s %s slot %s %s %s", ifname, argv[0], argv[1], argv[2], argv[3], argv[4]);
			sprintf(cmdstr, "echo %s >> /var/run/pfm.conf", rulestr);
			system(cmdstr);
		}
		
		memset(cmdstr, 0, 256);
		memset(tempstr, 0, 128);
		if (argc == 4) {
			sprintf(tempstr, "%s:4:packet-reforward %s %s slot %s %s", ifname, argv[0], argv[1], argv[2], "disable");
		}
		else {
			sprintf(tempstr, "%s:6:packet-reforward %s %s slot %s %s %s", ifname, argv[0], argv[1], argv[2], "disable", argv[4]);
		}
		sprintf(cmdstr, "grep '%s' /var/run/pfm.conf > /dev/null 2>&1", tempstr);
		status = system(cmdstr);

		if (0 == WEXITSTATUS(status)) {
			memset(cmdstr, 0, 256);
			tempstr1 = strtok(tempstr, "/");
			tempstr2 = strtok(NULL, "/");
			sprintf(cmdstr, "sed -i '/%s\\/%s/ d' /var/run/pfm.conf > /dev/null 2>&1", tempstr1, tempstr2);
			system(cmdstr);
		}
	}
	else if (opt == 1 || opt == 11) {
		memset(cmdstr, 0, 256);
		memset(tempstr, 0, 128);
		if (argc == 4) {
			sprintf(tempstr, "%s:4:packet-reforward %s %s slot %s %s", ifname, argv[0], argv[1], argv[2], "enable");
		}
		else {
			sprintf(tempstr, "%s:6:packet-reforward %s %s slot %s %s %s", ifname, argv[0], argv[1], argv[2], "enable", argv[4]);
		}
		sprintf(cmdstr, "grep '%s' /var/run/pfm.conf > /dev/null 2>&1", tempstr);

		status = system(cmdstr);
		if (status != -1) {
			if (WIFEXITED(status)) {
				if (0 == WEXITSTATUS(status)) {
					memset(cmdstr, 0, 256);
					if (strncmp(argv[1], "all", (strlen(argv[1]))) == 0) {
						sprintf(cmdstr, "sed -i '/%s/ d' /var/run/pfm.conf > /dev/null 2>&1", tempstr);
					}
					else {
						tempstr1 = strtok(tempstr, "/");
						tempstr2 = strtok(NULL, "/");
						sprintf(cmdstr, "sed -i '/%s\\/%s/ d' /var/run/pfm.conf > /dev/null 2>&1", tempstr1, tempstr2);
					}
					system(cmdstr);
				}
				else {
					memset(cmdstr, 0, 256);
					if (argc == 4) {
						sprintf(rulestr, "%s:4:packet-reforward %s %s slot %s %s", ifname, argv[0], argv[1], argv[2], argv[3]);
					}
					else {
						sprintf(rulestr, "%s:6:packet-reforward %s %s slot %s %s %s", ifname, argv[0], argv[1], argv[2], argv[3], argv[4]);
					}
					sprintf(cmdstr, "echo %s >> /var/run/pfm.conf", rulestr);
					system(cmdstr);
				}
			}
			else {
				/* exit */
			}
		}
		else {
			/* system error! */
		}
		
	}

	sync_file("/var/run/pfm.conf", 0);
	
	return CMD_SUCCESS;
}

    void dcli_interface_element_init(void)
    {
        /*install_element(CONFIG_NODE, &config_interface_eth_port_cmd);
        //install_element(CONFIG_NODE, &config_interface_vanId_cmd);
        //install_element(CONFIG_NODE, &config_interface_vanId_advan_cmd);
        //install_element(CONFIG_NODE, &no_config_interface_vanId_cmd);
        //install_element(CONFIG_NODE, &config_sub_interface_cmd);
        //install_element(CONFIG_NODE, &config_del_sub_interface_cmd);*/
        install_node(&static_arp_node,dcli_intf_interface_static_arp_show_running,"STATIC_ARP_NODE");
		install_element(CONFIG_NODE,  &config_dynamic_arp_cmd);
        install_element(CONFIG_NODE,  &vtysh_no_interface_cmd);
        install_element(CONFIG_NODE,  &vtysh_interface_cmd);
		/* jump for error, zhangdi 2012-02-13 */
        install_element(HANSI_NODE,  &vtysh_interface_cmd);
        install_element(LOCAL_HANSI_NODE,  &vtysh_interface_cmd);
        install_element(HANSI_NODE,  &vtysh_no_interface_cmd);
        install_element(LOCAL_HANSI_NODE,  &vtysh_no_interface_cmd);
      
		
		install_element(INTERFACE_NODE,  &config_dynamic_arp_cmd);
		install_element(INTERFACE_NODE,  &config_set_arp_stale_time_cmd);
		install_element(INTERFACE_NODE, &config_interface_static_arp_eth_port_cmd);

		install_element(INTERFACE_NODE, &no_interface_static_arp_eth_port_cmd);
		/*install_element(INTERFACE_NODE, &set_intf_change_intf_name_cmd);*/
		install_element(INTERFACE_NODE, &set_eth_intf_l3_enable_cmd);
        install_element(CONFIG_NODE,  &show_cvm_rxmax_per_interrupt_cmd);
        install_element(CONFIG_NODE,  &set_cvm_rxmax_per_interrupt_cmd);
        install_element(CONFIG_NODE,  &show_advanced_routing_default_vid_cmd);
		install_element(INTERFACE_NODE, &set_intf_pkt_refwd_cmd);
        #if 0/*wangchao delete*/
        install_element(CONFIG_NODE,  &config_ip_static_arp_cmd);
        install_element(CONFIG_NODE,  &config_ip_static_arp_for_rgmii_cmd);
        install_element(CONFIG_NODE,  &config_ip_static_arp_trunk_cmd);
        install_element(CONFIG_NODE,  &config_no_ip_static_arp_for_rgmii_cmd);
        install_element(CONFIG_NODE,  &config_no_ip_static_arp_cmd);
        install_element(CONFIG_NODE,  &config_no_ip_static_arp_trunk_cmd);
	    install_element(INTERFACE_NODE, &config_interface_static_arp_cmd); 
	    install_element(INTERFACE_NODE, &config_interface_static_arp_trunk_cmd); 
	    install_element(INTERFACE_NODE, &no_interface_static_arp_cmd); 	
	    install_element(INTERFACE_NODE, &no_interface_static_arp_trunk_cmd);
		install_element(CONFIG_NODE,  &show_advanced_routing_cmd);
		install_element(CONFIG_NODE,  &show_vlan_eth_port_advanced_routing_cmd);
		install_element(ENABLE_NODE,  &show_advanced_routing_cmd);
        install_element(ENABLE_NODE,  &show_vlan_eth_port_advanced_routing_cmd);
		install_element(INTERFACE_NODE,  &show_advanced_routing_cmd);
		install_element(INTERFACE_NODE,  &show_vlan_eth_port_advanced_routing_cmd);
        install_element(INTERFACE_NODE, &interface_advanced_routing_vlan_eth_port_cmd);
        install_element(INTERFACE_NODE, &show_interface_advanced_routing_vlan_eth_port_cmd);
        install_element(INTERFACE_NODE, &interface_bond_add_del_port_cmd);
		install_element(INTERFACE_NODE, &interface_bond_set_mode_cmd);
		install_element(INTERFACE_NODE, &interface_bond_set_lacp_rate_cmd);
		install_element(INTERFACE_NODE, &interface_bond_set_xmit_hash_policy_cmd);
		install_element(CONFIG_NODE,  &show_bond_slave_cmd);
        install_element(ENABLE_NODE,  &show_bond_slave_cmd);
		install_element(CONFIG_NODE,  &config_advanced_routing_default_vid_cmd);
		#endif
#ifdef _D_WCPSS_

        install_element(INTERFACE_NODE,&interface_forward_mode_cmd);
        install_element(INTERFACE_NODE,&interface_tunnel_mode_cmd);
        //install_element(WLAN_NODE,&interface_wlan_tunnel_mode_cmd);
        /*
	        install_element(HANSI_WLAN_NODE,&interface_wlan_tunnel_mode_cmd);
	        //install_element(CONFIG_NODE,&interface_wlan_tunnel_mode_cmd);
	        install_element(HANSI_NODE,&interface_wlan_tunnel_mode_cmd);
	        //install_element(RADIO_NODE,&interface_radio_tunnel_mode_cmd);
	        install_element(HANSI_RADIO_NODE,&interface_radio_tunnel_mode_cmd);

	        install_element(LOCAL_HANSI_WLAN_NODE,&interface_wlan_tunnel_mode_cmd);
	        install_element(LOCAL_HANSI_NODE,&interface_wlan_tunnel_mode_cmd);
	        install_element(LOCAL_HANSI_RADIO_NODE,&interface_radio_tunnel_mode_cmd);
	        */
#endif
    }
#ifdef __cplusplus
}
#endif
