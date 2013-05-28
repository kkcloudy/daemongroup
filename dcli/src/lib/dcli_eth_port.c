
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
* dcli_eth_port.c
*
*
* CREATOR:
*		qinhs@autelan.com
*
* DESCRIPTION:
*		CLI definition for ethernet port configuration.
*
* DATE:
*		02/21/2008	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.156 $	
*******************************************************************************/
#ifdef __cplusplus
	extern "C"
	{
#endif
#include <string.h>
#include <ctype.h>
#include <zebra.h>
#include <dbus/dbus.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h> 
#include <sys/types.h>   
#include <asm/types.h> 
#include <errno.h>

#include "sysdef/npd_sysdef.h"
#include "sysdef/returncode.h"
#include "dbus/npd/npd_dbus_def.h"

#include "command.h"
#include "if.h"

#include "util/npd_list.h"
#include "npd/nam/npd_amapi.h"
#include "dcli_vlan.h"
#include "dcli_eth_port.h"
#include "dcli_common_stp.h"
#include "dcli_intf.h"
#include "npd/nbm/npd_bmapi.h"
#include "dcli_main.h"
#include "dcli_sem.h"
#include "dbus/sem/sem_dbus_def.h"

extern int is_distributed;
#define TRUE 	1
#define FALSE 	0

extern struct global_ethport_s **global_ethport;
extern struct global_ethport_s *start_fp[MAX_SLOT_COUNT];
int dcli_debug_out = 0; // For debug dcli

char *slot_status_str[MODULE_STAT_MAX] = {
	"NONE",
	"INITING",
	"RUNNING",
	"DISABLED"
};

#define ETH_OCTEON_PORT 0
#define	ETH_ASIC_PORT   1
#define ETHPORT_SHOWRUN_CFG_SIZE	(3*1024) /* for all 24GE ports configuration */
typedef enum {
	PORT_SPEED_10_E,
	PORT_SPEED_100_E,
	PORT_SPEED_1000_E,
	PORT_SPEED_10000_E,
	PORT_SPEED_12000_E,
	PORT_SPEED_2500_E,
	PORT_SPEED_5000_E
} PORT_SPEED_ENT;

/**
  * Enums from
  */
typedef enum {
	PORT_FULL_DUPLEX_E,
	PORT_HALF_DUPLEX_E
} PORT_DUPLEX_ENT;

char *eth_port_type_str[ETH_MAX] = {
	"ETH_INVALID",
	"ETH_FE_TX",
	"ETH_FE_FIBER",
	"ETH_GTX",
	"ETH_GE_FIBER",
	"ETH_GE_SFP",
	"ETH_XGE_XFP",
	"ETH_XGTX",
	"ETH_XGE_FIBER"
};

char *eth_port_test[2] = {
	"ETH_XGTX",
	"ETH_XGE_FIBER"	
};

char *link_status_str[2] = {
	"DOWN",
	"UP"
};

char *onoff_status_str[2] = {
	"-",
	"ON"
};
char *doneOrnot_status_str[2] = {
	"Incomplete",
	"Completed"
};
char *duplex_status_str[2] = {
	"FULL",
	"HALF"
};

char *eth_speed_str[ETH_ATTR_SPEED_MAX] = {
	"10M",
	"100M",
	"1000M",
	"10G",
	"12G",
	"2.5G",
	"5G"
};

extern int is_distributed;
extern DBusConnection *dcli_dbus_connection;
extern dbus_connection *dbus_connection_dcli[];
extern int dcli_slot_id_get(void);
extern int get_product_info(char *filename);

struct cmd_node eth_port_node = 
{
	ETH_PORT_NODE,
	"%s(config-eth-port)# "
};

__inline__ int parse_int_parse(char* str,unsigned int* shot){
	char *endptr = NULL;

	if (NULL == str) return NPD_FAIL;
	*shot= strtoul(str,&endptr,10);
	return NPD_SUCCESS;	
}
__inline__ int parse_short_parse(char* str,unsigned short* shot){
	char *endptr = NULL;

	if (NULL == str) return NPD_FAIL;
	*shot= strtoul(str,&endptr,10);
	return NPD_SUCCESS;	
}

__inline__ int parse_slotport_no(char *str,unsigned char *slotno,unsigned char *portno) {
	char *endptr = NULL;
	char *endptr2 = NULL;
    char c = 0;
	
	if (NULL == str) return NPD_FAIL;
	/* add for AX7605i cscd port by qinhs@autelan.com 2009-11-18 */
	if(!strncmp(tolower(str), "cscd", 4)) {
		*slotno = AX7i_XG_CONNECT_SLOT_NUM;		
		if(strlen(str) > strlen("cscd*")) {
			return NPD_FAIL;
		}
		else if('0' == str[4]) {
			*portno = 0;
		}
		else if('1' == str[4]) {
			*portno = 1;
		}
		else {
			return NPD_FAIL;
		}
		return NPD_SUCCESS;
	}
	else if(!strncmp(tolower(str), "obc", 3)) {
		*slotno = AX7i_XG_CONNECT_SLOT_NUM;		
		if(strlen(str) > strlen("obc*")) {
			return NPD_FAIL;
		}
		else if('0' == str[3]) {
			*portno = 0;
		}
		else if('1' == str[3]) {
			*portno = 1;
		}
		else {
			return NPD_FAIL;
		}
		return NPD_SUCCESS;
	}
	c = str[0];
	if (c>='0' && c<='9'){
	    *slotno = strtoul(str,&endptr,10);
        if ((SLOT_PORT_SPLIT_DASH == endptr[0])||(SLOT_PORT_SPLIT_SLASH == endptr[0])) {
            if (endptr) {
        		*portno = strtoul((char *)&(endptr[1]),&endptr2,10);
        		if('\0' == endptr2[0]) {
        			return NPD_SUCCESS;
        		}
        	}
        }
	}else{
    	return NPD_FAIL;
	}
}

__inline__ int parse_slotport_ve_tag_no(char *str,unsigned char *slotno,unsigned char *portno,unsigned char *des_slot)
	
{
	char *endptr = NULL;
	char *endptr2 = NULL;
	char *endptr3 = NULL;
	char *endptr4 = NULL;
	char *endptr5 = NULL;
	char *endptr6 = NULL;
	char * tmpstr = str;
	
	if(NULL == str){
		return NPD_FAIL;
	}
	if((NULL == slotno)||(NULL == portno)){
		return NPD_FAIL;
	}
	*slotno = 0;
	*portno = 0;

    if(0 == strncmp(str,"ve",2)){
		tmpstr = str+2;
	}
	if (NULL == tmpstr) {return NPD_FAIL;}
	if(((tmpstr[0] == '0')&&(tmpstr[1] != SLOT_PORT_SPLIT_DASH))|| \
		(tmpstr[0] > '9')||(tmpstr[0] < '0') || ((tmpstr[2] == '0')&&(tmpstr[3] != SLOT_PORT_SPLIT_DASH))|| \
		(tmpstr[2] > '9')||(tmpstr[2] < '0') || (tmpstr[4] > '9')||(tmpstr[4] < '0')){
         return NPD_FAIL;
	}
	*des_slot = (char)strtoul(tmpstr,&endptr,10);
    if((*des_slot <1) || (*des_slot >2))
        return NPD_FAIL;
	if (endptr) {
		if ((SLOT_PORT_SPLIT_DASH == endptr[0])&& \
			(('0' < endptr[1])&&('9' >= endptr[1]))){
			*slotno= (char)strtoul((char *)&(endptr[1]),&endptr2,10);
            if((*slotno < 1) || (*slotno > 3) || (*des_slot == *slotno))
				return NPD_FAIL;

			if(((SLOT_PORT_SPLIT_DASH == endptr2[0])&& \
			(('0' < endptr2[1])&&('9' >= endptr2[1]))))
				{
					*portno = (char)strtoul((char *)&(endptr2[1]),&endptr3,10);
                    if((SLOT_PORT_SPLIT_DASH == endptr3[0]) || (*portno < 0) || (*portno > 26))
						return NPD_FAIL;
				}
			return NPD_SUCCESS;
		}
	}
	*slotno = 0;
	/* return NPD_SUCCESS;*/
	return NPD_FAIL;	
}

__inline__ int parse_slot_tag_no(char *str,unsigned char *slotno, unsigned int *tag1, unsigned int *tag2)
	
{
	char *endptr = NULL;
	char *endptr1 = NULL, *endptr2 = NULL;
	char * tmpstr = str;
	int slotNum = 16;
	
	if(NULL == str || NULL == slotno){
		return NPD_FAIL;
	}
	*slotno = 0;

    if(!strncmp(str,"ve",2)){
		tmpstr = str+2;
	}else if(!strncmp(str,"obc",3)){
	    tmpstr = str+3;
	}
	if (NULL == tmpstr) 
	{
		return NPD_FAIL;
	}
	
	if((tmpstr[0] > '9')||(tmpstr[0] < '0')){
         return NPD_FAIL;
	}

	*slotno = (char)strtoul(tmpstr,&endptr,10);
	
	slotNum = get_product_info(SEM_SLOT_COUNT_PATH);        
    if((*slotno < 0) || (*slotno > slotNum))
        return NPD_FAIL;

	if (endptr) 
	{
/*		
        if(endptr[0] == '\0') {
			*tag1 = 0;
			*tag2 = 0;
			return NPD_SUCCESS;
		}
*/		
        if(('.' == endptr[0])&&(('0' < endptr[1])&&('9' >= endptr[1])))
        {
        	*tag1 = strtoul((char *)&(endptr[1]),&endptr1,10);
			if(endptr1[0] == '\0') {
				*tag2 = 0;
				return NPD_SUCCESS;
			}

            if(('.' == endptr1[0])&&(('0' < endptr1[1])&&('9' >= endptr1[1])))
            {
            	*tag2 = strtoul((char *)&(endptr1[1]),&endptr2,10);
    			
            	if((NULL == endptr2)||('\0' == endptr2[0])){
            		return NPD_SUCCESS;
            	}
            	return NPD_FAIL;
            }
        }
	}

	return NPD_FAIL;	
}

__inline__ int parse_slotport_tag_no(char *str,unsigned char *slotno,unsigned char *portno,unsigned int * tag1,unsigned int *tag2)
{
	char *endptr = NULL;
	char *endptr2 = NULL;
	char *endptr3 = NULL;
	char *endptr4 = NULL;
	char * tmpstr = str;
	
	if(NULL == str){
		return NPD_FAIL;
	}
	if((NULL == slotno)||(NULL == portno)||(NULL == tag1)){
		return NPD_FAIL;
	}
	*slotno = 0;
	*portno = 0;
	*tag1 = 0;
	if(NULL == tag2){
		*tag2 = 0;
	}
	if(!strncmp(str,"eth",3) || !strncmp(str,"mng",3)){
		tmpstr = str+3;
	}
	if (NULL == tmpstr) {return NPD_FAIL;}
	if(((tmpstr[0] == '0')&&(tmpstr[1] != SLOT_PORT_SPLIT_DASH))|| \
		(tmpstr[0] > '9')||(tmpstr[0] < '0')){
         return NPD_FAIL;
	}
	*portno = (char)strtoul(tmpstr,&endptr,10);
	if (endptr) {
		if ((SLOT_PORT_SPLIT_DASH == endptr[0])&& \
			(('0' < endptr[1])&&('9' >= endptr[1]))){
            *slotno = *portno;
			*portno = (char)strtoul((char *)&(endptr[1]),&endptr2,10);
			/*}
			for eth1, eth1.2, eth1.2.3
			else endptr2 = endptr;
			{
			*/
			if(endptr2){	
				if('\0' == endptr2[0]){
					*tag1 = 0;
					return NPD_SUCCESS;
				}
				else if(('.' == endptr2[0])&&\
					(('0' < endptr2[1])&&('9' >= endptr2[1]))){
					*tag1 = strtoul((char *)&(endptr2[1]),&endptr3,10);
					if((NULL == endptr3)||('\0' == endptr3[0])){
						if(tag2) *tag2 = 0;
						return NPD_SUCCESS;
					}
					if(!tag2) return NPD_FAIL;
					if((endptr3 != NULL)&&(endptr3[0] == '.')){
						if(('0' >= endptr3[1])||('9' < endptr3[1])){
							return NPD_FAIL;
						}
						if(tag2){
							*tag2 = strtoul((char *)&(endptr3[1]),&endptr4,10);
							if((endptr4 != NULL)&&(endptr4[0] != '\0')){
								return NPD_FAIL;
							}
						}
						return NPD_SUCCESS;
					}
					return NPD_FAIL;
					
				}
				else{
					*tag1 = 0;
					return NPD_FAIL;
				}
			}
			
			*tag1 = 0;
			if(tag2) *tag2 = 0;
			return NPD_SUCCESS;
		}
	}
	*slotno = 0;
	*tag1 = 0;
	if(tag2) *tag2 = 0;
	/* return NPD_SUCCESS;*/
	return NPD_FAIL;	
}

/* parse slot_id,cpu_no,cpu_port_no,tag1 & tag2 of ve
 * zhangdi@autelan.com 2012-06-05
 */
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
		return NPD_FAIL;
	}
	*slotno = 0;

    if(!strncmp(str,"ve",2)){
		tmpstr = str+2;
	}else if(!strncmp(str,"obc",3)){
	    tmpstr = str+3;
	}
	if (NULL == tmpstr) 
	{
		return NPD_FAIL;
	}
	
	if((tmpstr[0] > '9')||(tmpstr[0] < '0')){
         return NPD_FAIL;
	}

	*slotno = (char)strtoul(tmpstr,&endp,10);
	
	slotNum = get_product_info(SEM_SLOT_COUNT_PATH);        
    if((*slotno < 0) || (*slotno > slotNum))
        return NPD_FAIL;

	if (endp) 
	{
		/* to support ve10.tag1.tag2 */
		if('.' == endp[0])
		{
			endptr = endp;
			/* set cpu_no & cpu_port_no to default 1 */
			*cpu_no = 1;			
			*cpu_port_no = 1;			
			
			/* get .tag1.tag2 */
            if(('.' == endptr[0])&&(('0' < endptr[1])&&('9' >= endptr[1])))
            {
            	*tag1 = strtoul((char *)&(endptr[1]),&endptr1,10);
    			if(endptr1[0] == '\0') {
    				*tag2 = 0;
    				return NPD_SUCCESS;
    			}

                if(('.' == endptr1[0])&&(('0' < endptr1[1])&&('9' >= endptr1[1])))
                {
                	*tag2 = strtoul((char *)&(endptr1[1]),&endptr2,10);
        			
                	if((NULL == endptr2)||('\0' == endptr2[0])){
                		return NPD_SUCCESS;
                	}
                	return NPD_FAIL;
                }
            }		
			
		}
		else
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
            	return NPD_FAIL;			
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
        				return NPD_SUCCESS;
        			}

                    if(('.' == endptr1[0])&&(('0' < endptr1[1])&&('9' >= endptr1[1])))
                    {
                    	*tag2 = strtoul((char *)&(endptr1[1]),&endptr2,10);
            			
                    	if((NULL == endptr2)||('\0' == endptr2[0])){
                    		return NPD_SUCCESS;
                    	}
                    	return NPD_FAIL;
                    }
                }		
    		}
		}
		
	}

	return NPD_FAIL;	
}


int dcli_get_slot_port_by_portindex
(
	struct vty* vty,
	unsigned int port_index,
	unsigned char* slot,
	unsigned char* port
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	int op_ret = 0;
	unsigned int eth_g_index = 0;
	unsigned char slot_no = 0, port_no = 0;
    int local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
	if (is_distributed == DISTRIBUTED_SYSTEM)
	{
    	dbus_error_init(&err);
    	query = dbus_message_new_method_call(				\
                                        SEM_DBUS_BUSNAME, 	\
										SEM_DBUS_ETHPORTS_OBJPATH,	\
										SEM_DBUS_ETHPORTS_INTERFACE, \
										SEM_DBUS_ETHPORTS_GET_SLOT_PORT
    										);
    	  		
    	dbus_message_append_args(query,
    									DBUS_TYPE_UINT32,&port_index,																			
    									DBUS_TYPE_INVALID);

        SLOT_PORT_ANALYSIS_SLOT(port_index, slot_no);
		DCLI_DEBUG(("port_index = %d, slot_no = %d\n", port_index, slot_no));
    	if(NULL == dbus_connection_dcli[slot_no]->dcli_dbus_connection) 				
    	{
    		if(slot_no == local_slot_id) 
    		{
    		    reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
    		}
    		else 
    		{	
    			vty_out(vty, "Connection to slot%d is not exist.\n", slot_no);
    			return CMD_WARNING;
    		}
    	}else {
    		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_no]->dcli_dbus_connection,\
    				query, -1, &err);
    	}
    	
    	dbus_message_unref(query);
    	if (NULL == reply) {
    		vty_out(vty,"failed get re_slot_port reply.\n");
    		if (dbus_error_is_set(&err)) {
    			vty_out(vty,"%s raised: %s",err.name,err.message);
    			dbus_error_free_for_dcli(&err);
    		}
    		dbus_message_unref(reply);
    		return CMD_WARNING;
    	}
    	
    	if (dbus_message_get_args ( reply, &err,
    								DBUS_TYPE_UINT32,&op_ret,
    								DBUS_TYPE_BYTE,&slot_no,
    								DBUS_TYPE_BYTE,&port_no,
    								DBUS_TYPE_INVALID)) {
    		if(SEM_COMMAND_NOT_SUPPORT == op_ret)
    		{
				// TODO:call npd dbus to process.
				DCLI_DEBUG(("%% call npd dbus to process\n"));
				dbus_message_unref(reply);					
			}
    		else if(SEM_COMMAND_SUCCESS == op_ret){
    			*slot = slot_no;
    			*port = port_no;
    			dbus_message_unref(reply);
    			return CMD_SUCCESS;
    		}
    		else if(SEM_COMMAND_FAILED== op_ret){
    			/*DCLI_DEBUG(("execute command failed\n");*/
    			dbus_message_unref(reply);
    			return CMD_WARNING;
    		}
    	} else {
    		vty_out(vty,"Failed get args.\n");
    		if (dbus_error_is_set(&err)) {
    				vty_out(vty,"%s raised: %s",err.name,err.message);
    				dbus_error_free_for_dcli(&err);
    		}
    		dbus_message_unref(reply);
    		return CMD_WARNING;
    	}
	}

	dbus_error_init(&err);
	query = dbus_message_new_method_call(				\
											NPD_DBUS_BUSNAME, 	\
											NPD_DBUS_ETHPORTS_OBJPATH,	\
											NPD_DBUS_ETHPORTS_INTERFACE, \
											NPD_DBUS_ETHPORTS_INTERFACE_METHOD_GET_SLOT_PORT
											);
	
		
	DCLI_DEBUG(("the slot_no: %d	port_no: %d\n",slot_no,port_no));
	
	//tran_value=ETH_GLOBAL_INDEX_FROM_SLOT_PORT_LOCAL_NO(slot_no,port_no);
	//DCLI_DEBUG(("changed value : slot %d,port %d\n",slot_no,port_no));
	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&port_index,																			
									DBUS_TYPE_INVALID);


	 reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get re_slot_port reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return CMD_WARNING;
	}
	
	if (dbus_message_get_args ( reply, &err,
								DBUS_TYPE_UINT32,&op_ret,
								DBUS_TYPE_BYTE,&slot_no,
								DBUS_TYPE_BYTE,&port_no,
								DBUS_TYPE_INVALID)) {
		if(NPD_DBUS_SUCCESS == op_ret){
			*slot = slot_no;
			*port = port_no;
			dbus_message_unref(reply);
			return CMD_SUCCESS;
		}
		else if(NPD_DBUS_ERROR == op_ret){
			/*DCLI_DEBUG(("execute command failed\n");*/
			dbus_message_unref(reply);
			return CMD_WARNING;
		}
	} else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return CMD_WARNING;
	}
		
}

DEFUN(config_port_mode_cmd_func,
	config_port_mode_cmd,
	"config eth-port PORTNO mode (switch|route|promiscuous)",
	CONFIG_STR
	"Config eth-port\n"
	CONFIG_ETHPORT_STR
	"Config specify mode\n"
	"Config ethernet port as layer 2 switch port(default mode)\n"
	"Config ethernet port as layer 3 route interface\n"
	"Config ethernet port as MUX port(private vlan isolate port)\n"
)
{
	unsigned char *modeStr=NULL;
	unsigned int 	op_ret,mode;
	unsigned char   slot_no,port_no;
	int ifnameType = 0;
	op_ret = parse_slotport_no((char *)argv[0], &slot_no, &port_no);
	if (NPD_FAIL == op_ret) {
		vty_out(vty,"input bad slot/port!\n");
		return CMD_SUCCESS;
	}

	modeStr = (char*)argv[1];
	if(0 == strncmp(modeStr,"switch",strlen(modeStr)))
		mode = ETH_PORT_FUNC_BRIDGE;
	else if(0 == strncmp(modeStr,"route",strlen(modeStr)))
		mode =ETH_PORT_FUNC_IPV4;
	else if(0 == strncmp(modeStr,"promiscuous",strlen(modeStr)))
		mode = ETH_PORT_FUNC_MODE_PROMISCUOUS;
	else {
		vty_out(vty,"input bad mode!\n");
		return CMD_WARNING;
	}
	return dcli_eth_port_mode_config(vty,ifnameType,slot_no,port_no,mode);
}

int dcli_eth_port_mode_config
(
    struct vty * vty,
    int ifnameType,
    unsigned char slot_no,
    unsigned char port_no,
    unsigned int mode
)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int op_ret = INTERFACE_RETURN_CODE_SUCCESS;
	int master_slot_id[2] = {-1, -1};
   	int local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);

	dcli_master_slot_id_get(master_slot_id);
	
	/*vty_out(vty,"FDB agingtime %d. \n",agingtime);
	//printf("mode %d\n",mode);*/
	if (is_distributed == DISTRIBUTED_SYSTEM)
	{
		query = dbus_message_new_method_call(SEM_DBUS_BUSNAME,		\
									SEM_DBUS_OBJPATH,		\
									SEM_DBUS_INTERFACE,		\
									SEM_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_ETHPORT_MODE);
		dbus_error_init(&err);

		dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&ifnameType,		
									DBUS_TYPE_BYTE,&slot_no,
									DBUS_TYPE_BYTE,&port_no,
								 	DBUS_TYPE_UINT32,&mode,
								 	DBUS_TYPE_INVALID);


		/* send the dbus msg to any other board */
        if(NULL == dbus_connection_dcli[slot_no]->dcli_dbus_connection) 				
		{
			if(slot_no == local_slot_id)
			{
                reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query, 40000, &err);
			}
			else 
			{	
			   	vty_out(vty,"Can not connect to slot:%d \n",slot_no);	
    			return CMD_WARNING;
			}
        }
		else
		{
            reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_no]->dcli_dbus_connection,query, 40000, &err);				
		}

		dbus_message_unref(query);
		if (NULL == reply) {
			vty_out(vty,"failed get reply.\n");
			if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_WARNING;
		}

		if (dbus_message_get_args ( reply, &err,
			DBUS_TYPE_UINT32,&op_ret,
			DBUS_TYPE_INVALID)) {
			if(INTERFACE_RETURN_CODE_SUCCESS == op_ret){
	            dbus_message_unref(reply);
				sleep(1);
				return CMD_SUCCESS;
			}
			else if(INTERFACE_RETURN_CODE_NO_SUCH_PORT == op_ret){
				vty_out(vty,"%% NO SUCH PORT %d/%d!\n",slot_no,port_no);
				dbus_message_unref(reply);
		        return CMD_WARNING;	
			}
			else if(INTERFACE_RETURN_CODE_UNSUPPORT_COMMAND == op_ret){
				// TODO:call npd dbus to process.
				DCLI_DEBUG(("%% call npd dbus to process\n"));
                dbus_message_unref(reply);
			}
			else if(INTERFACE_RETURN_CODE_ALREADY_THIS_MODE == op_ret){
                if(ETH_PORT_FUNC_BRIDGE == mode){
                    vty_out(vty,"%% Port interface not exists!\n");
				}
				else if(ETH_PORT_FUNC_IPV4 == mode){
					vty_out(vty,"%% Advanced-routing already disabled!\n");
				}
				else if(ETH_PORT_FUNC_MODE_PROMISCUOUS == mode){
					vty_out(vty,"%% Advanced-routing already enabled!\n");
				}
				dbus_message_unref(reply);
		        return CMD_WARNING;	
			}
			else{
				vty_out(vty,dcli_error_info_intf(op_ret));	
				dbus_message_unref(reply);
		        return CMD_WARNING;	
			}
		}
		else {
			vty_out(vty,"Failed get args.\n");
			if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			dbus_message_unref(reply);
		    return CMD_WARNING;	
		}
	}
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,		\
								NPD_DBUS_ETHPORTS_OBJPATH,		\
								NPD_DBUS_ETHPORTS_INTERFACE,		\
								NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_ETHPORT_MODE);
	dbus_error_init(&err);

	dbus_message_append_args(query,
								DBUS_TYPE_UINT32,&ifnameType,		
								DBUS_TYPE_BYTE,&slot_no,
								DBUS_TYPE_BYTE,&port_no,
							 	DBUS_TYPE_UINT32,&mode,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,40000, &err);
	
	
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)) {
			if(INTERFACE_RETURN_CODE_SUCCESS == op_ret){
	            dbus_message_unref(reply);
				sleep(1);
				return CMD_SUCCESS;
			}
			else if(INTERFACE_RETURN_CODE_NO_SUCH_PORT == op_ret){
				vty_out(vty,"%% NO SUCH PORT %d/%d!\n",slot_no,port_no);
			}
			else if(INTERFACE_RETURN_CODE_ALREADY_THIS_MODE == op_ret){
                if(ETH_PORT_FUNC_BRIDGE == mode){
                    vty_out(vty,"%% Port interface not exists!\n");
				}
				else if(ETH_PORT_FUNC_IPV4 == mode){
					vty_out(vty,"%% Advanced-routing already disabled!\n");
				}
				else if(ETH_PORT_FUNC_MODE_PROMISCUOUS == mode){
					vty_out(vty,"%% Advanced-routing already enabled!\n");
				}
			}
			else
				vty_out(vty,dcli_error_info_intf(op_ret));
			
	}
	else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_WARNING;

}
int dcli_eth_port_mode_config_ve
(
    struct vty * vty,
    int ifnameType,
    unsigned char slot_no,
    unsigned char port_no,
    unsigned char des_slot
)
{
	DBusMessage *query, *reply, *reply_remote;
	DBusError err;
	unsigned int op_ret = INTERFACE_RETURN_CODE_SUCCESS;

	if(NULL == dbus_connection_dcli[slot_no]->dcli_dbus_connection){
        vty_out(vty,"Could not delete ve port,slot %d is not connected\n",slot_no);
		return CMD_WARNING;
    }
	/*vty_out(vty,"FDB agingtime %d. \n",agingtime);
	//printf("mode %d\n",mode);*/
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,		\
								NPD_DBUS_ETHPORTS_OBJPATH,		\
								NPD_DBUS_ETHPORTS_INTERFACE,		\
								NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_ETHPORT_MODE_DEL_VE);
	dbus_error_init(&err);

	dbus_message_append_args(query,
								DBUS_TYPE_UINT32,&ifnameType,		
								DBUS_TYPE_BYTE,&slot_no,
								DBUS_TYPE_BYTE,&port_no,
							 	DBUS_TYPE_BYTE,&des_slot,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,40000, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)) {
			if(INTERFACE_RETURN_CODE_SUCCESS == op_ret){
	            dbus_message_unref(reply);
				sleep(1);
				return CMD_SUCCESS;
			}
			else if(INTERFACE_RETURN_CODE_NO_SUCH_PORT == op_ret){
				vty_out(vty,"%% NO SUCH PORT %d-%d-%d!\n",des_slot,slot_no,port_no);
			}
			else
				vty_out(vty,dcli_error_info_intf(op_ret));
			
	}
	else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);

	reply_remote = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_no],query,40000, &err);
	
	dbus_message_unref(query);
	if (NULL == reply_remote) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}

	if (dbus_message_get_args ( reply_remote, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)) {
			if(INTERFACE_RETURN_CODE_SUCCESS == op_ret){
	            dbus_message_unref(reply_remote);
				sleep(1);
				return CMD_SUCCESS;
			}
			else if(INTERFACE_RETURN_CODE_NO_SUCH_PORT == op_ret){
				vty_out(vty,"%% NO SUCH PORT %d-%d-%d!\n",des_slot,slot_no,port_no);
			}
			else
				vty_out(vty,dcli_error_info_intf(op_ret));
			
	}
	else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply_remote);
	return CMD_WARNING;

}


int dcli_eth_port_interface_mode_config_ve
(
    struct vty * vty,
    int ifnameType,
    unsigned char slot_no,
    unsigned char port_no,
    unsigned char des_slot_no
    /*unsigned int ifIndex*/
)
{
	DBusMessage *query, *reply,*reply_remote;
	DBusError err;
	unsigned int op_ret = NPD_DBUS_ERROR;
	unsigned int tmpIfIndex = ~0UI;


    if(NULL == dbus_connection_dcli[slot_no]->dcli_dbus_connection){
        vty_out(vty,"Could not create ve port,slot %d is not connected\n",slot_no);
		return CMD_WARNING;
    }
	/* *ifIndex = ~0UI;
	//vty_out(vty,"FDB agingtime %d. \n",agingtime);
	//printf("mode %d\n",mode);*/
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,		\
								NPD_DBUS_ETHPORTS_OBJPATH,		\
								NPD_DBUS_ETHPORTS_INTERFACE,		\
								NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_ETHPORT_MODE_VE);
	dbus_error_init(&err);

	dbus_message_append_args(query,
								DBUS_TYPE_UINT32,&ifnameType,
								DBUS_TYPE_BYTE,&slot_no,
								DBUS_TYPE_BYTE,&port_no,
								DBUS_TYPE_BYTE,&des_slot_no,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_UINT32,&tmpIfIndex,
		DBUS_TYPE_INVALID)) {
			if(INTERFACE_RETURN_CODE_SUCCESS == op_ret){
				/**ifIndex = tmpIfIndex;*/
	            dbus_message_unref(reply);
/*				sleep(1);*/
				return CMD_SUCCESS;
			}
			else if(INTERFACE_RETURN_CODE_NO_SUCH_PORT == op_ret){
				vty_out(vty,"%% NO SUCH PORT %d-%d-%d!\n",des_slot_no,slot_no,port_no);
			}
			else
				vty_out(vty,dcli_error_info_intf(op_ret));
			
	}
	else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);

	reply_remote = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_no],query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply_remote) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}

	if (dbus_message_get_args ( reply_remote, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_UINT32,&tmpIfIndex,
		DBUS_TYPE_INVALID)) {
			if(INTERFACE_RETURN_CODE_SUCCESS == op_ret){
				/**ifIndex = tmpIfIndex;*/
	            dbus_message_unref(reply_remote);
/*				sleep(1);*/
				return CMD_SUCCESS;
			}
			else if(INTERFACE_RETURN_CODE_NO_SUCH_PORT == op_ret){
				vty_out(vty,"%% NO SUCH PORT %d-%d-%d!\n",des_slot_no,slot_no,port_no);
			}
			else
				vty_out(vty,dcli_error_info_intf(op_ret));
			
	}
	else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply_remote);
	
	
	return CMD_WARNING;

}

/* config eth-port to route or promiscuous mode */
int dcli_eth_port_interface_mode_config
(
    struct vty * vty,
    int ifnameType,
    unsigned char slot_no,
    unsigned char port_no
    /*unsigned int ifIndex*/
)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int op_ret = NPD_DBUS_ERROR;
	unsigned int tmpIfIndex = ~0UI;
	int master_slot_id[2] = {-1, -1};
   	int local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);

	dcli_master_slot_id_get(master_slot_id);
	
	/* *ifIndex = ~0UI;
	//vty_out(vty,"FDB agingtime %d. \n",agingtime);
	//printf("mode %d\n",mode);*/
	if (is_distributed == DISTRIBUTED_SYSTEM)
	{
		query = dbus_message_new_method_call(SEM_DBUS_BUSNAME,		\
									SEM_DBUS_OBJPATH,		\
									SEM_DBUS_INTERFACE,		\
									SEM_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_ETHPORT_INTERFACE);
		dbus_error_init(&err);

		dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&ifnameType,
									DBUS_TYPE_BYTE,&slot_no,
									DBUS_TYPE_BYTE,&port_no,
								 	DBUS_TYPE_INVALID);
		
		/* send the dbus msg to any other board */
        if(NULL == dbus_connection_dcli[slot_no]->dcli_dbus_connection) 				
		{
			if(slot_no == local_slot_id)
			{
                reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
			}
			else 
			{	
			   	vty_out(vty,"Can not connect to slot:%d \n",slot_no);	
    			return CMD_WARNING;
			}
        }
		else
		{
            reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_no]->dcli_dbus_connection,query,-1, &err);				
		}

		dbus_message_unref(query);
		if (NULL == reply) {
			vty_out(vty,"failed get reply.\n");
			if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_WARNING;
		}

		if (dbus_message_get_args ( reply, &err,
			DBUS_TYPE_UINT32,&op_ret,
			DBUS_TYPE_UINT32,&tmpIfIndex,
			DBUS_TYPE_INVALID)) {
			if(INTERFACE_RETURN_CODE_SUCCESS == op_ret){
				/**ifIndex = tmpIfIndex;*/
	            dbus_message_unref(reply);
/*				sleep(1);*/
				return CMD_SUCCESS;
			}
			else if(INTERFACE_RETURN_CODE_NO_SUCH_PORT == op_ret){
				vty_out(vty,"%% NO SUCH PORT %d/%d!\n",slot_no,port_no);
				dbus_message_unref(reply);
		        return CMD_WARNING;
			}
			else if(INTERFACE_RETURN_CODE_UNSUPPORT_COMMAND == op_ret){
				// TODO:call npd dbus to process.
				DCLI_DEBUG(("%% call npd dbus to process\n"));
                dbus_message_unref(reply);
			}
			else if(INTERFACE_RETURN_CODE_INTERFACE_NOTEXIST == op_ret){   
			    vty_out(vty,"%% interface not exist!\n");
				dbus_message_unref(reply);
		        return CMD_WARNING;
			}
			else{
				vty_out(vty,dcli_error_info_intf(op_ret));
				dbus_message_unref(reply);
		        return CMD_WARNING;
			}				
		}
		else {
			vty_out(vty,"Failed get args.\n");
			if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			dbus_message_unref(reply);
		    return CMD_WARNING;
		}	
	}

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,		\
								NPD_DBUS_ETHPORTS_OBJPATH,		\
								NPD_DBUS_ETHPORTS_INTERFACE,		\
								NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_ETHPORT_INTERFACE);
	dbus_error_init(&err);

	dbus_message_append_args(query,
								DBUS_TYPE_UINT32,&ifnameType,
								DBUS_TYPE_BYTE,&slot_no,
								DBUS_TYPE_BYTE,&port_no,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_UINT32,&tmpIfIndex,
		DBUS_TYPE_INVALID)) {
			if(INTERFACE_RETURN_CODE_SUCCESS == op_ret){
				/**ifIndex = tmpIfIndex;*/
	            dbus_message_unref(reply);
/*				sleep(1);*/
				return CMD_SUCCESS;
			}
			else if(INTERFACE_RETURN_CODE_NO_SUCH_PORT == op_ret){
				vty_out(vty,"%% NO SUCH PORT %d/%d!\n",slot_no,port_no);
			}
			else
				vty_out(vty,dcli_error_info_intf(op_ret));
			
	}
	else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_WARNING;

}

/* Added by Jia Lihui for show eth-port list on single slot. 2011.7.17 */
DEFUN(show_ethport_slotx_list_cmd_func,
	show_ethport_slotx_list_cmd,
	"show eth-port slot <1-10> list",
	SHOW_STR
	"Display Ethernet Port information on slotx\n"
	"List Ethernet Port list summary on slotx\n"
	)
{
	unsigned int portNum, slot_no;
	int slot_id;
	int i = 0;
	char *endptr = NULL;
    int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
	
	if (is_distributed == DISTRIBUTED_SYSTEM)	
	{
        slot_no = strtoul(argv[0], &endptr, 10);

		if(slot_no > slotNum || slot_no <= 0)
		{
			vty_out(vty,"%% NO SUCH SLOT %d!\n", slot_no);
            return CMD_WARNING;
		}
		
		vty_out(vty,"%-9s%-16s%-5s%-5s%-7s%-9s%-6s%-11s\n","PORTNO","PORTTYPE","LINK","AUTO","DUPLEX","FLOWCTRL","SPEED","MTU(BYTES)");
        
		for(i = 0; i < BOARD_GLOBAL_ETHPORTS_MAXNUM; i++)
		{	
            if(start_fp[slot_no-1][i].isValid == VALID_ETHPORT)
			{
				unsigned port_no = start_fp[slot_no-1][i].local_port_no;
				unsigned int attr_map = start_fp[slot_no-1][i].attr_bitmap;
				unsigned int port_type = start_fp[slot_no-1][i].port_type;
				int mtu = start_fp[slot_no-1][i].mtu;
			    vty_out(vty,"%2d-%-2d    ",slot_no, port_no);

				if(ETH_ATTR_LINKUP == ((attr_map & ETH_ATTR_LINK_STATUS)>>ETH_LINK_STATUS_BIT)) {
    				vty_out(vty,"%-16s",eth_port_type_str[port_type]);
					vty_out(vty,"%-5s",link_status_str[(attr_map & ETH_ATTR_LINK_STATUS) >> ETH_LINK_STATUS_BIT]);
					vty_out(vty,"%-5s",onoff_status_str[(attr_map & ETH_ATTR_AUTONEG) >> ETH_AUTONEG_BIT]);
					vty_out(vty,"%-7s",duplex_status_str[(attr_map & ETH_ATTR_DUPLEX) >> ETH_DUPLEX_BIT]);
					vty_out(vty,"%-9s",onoff_status_str[(attr_map & ETH_ATTR_FLOWCTRL) >> ETH_FLOWCTRL_BIT]);
					vty_out(vty,"%-6s",eth_speed_str[(attr_map & ETH_ATTR_SPEED_MASK) >> ETH_SPEED_BIT]);
					vty_out(vty,"%-11d",mtu);
					vty_out(vty,"\n");
				}
				else {
    				vty_out(vty,"%-16s",eth_port_type_str[port_type]);/*port type*/
					vty_out(vty,"%-5s",link_status_str[(attr_map & ETH_ATTR_LINK_STATUS) >> ETH_LINK_STATUS_BIT]);/*link status*/
					vty_out(vty,"%-5s","-");/*auto-negotiation*/
					vty_out(vty,"%-7s","-");/*duplex status*/
					vty_out(vty,"%-9s","-");/*flow control state*/
					vty_out(vty,"%-6s","-");/*speed*/
					vty_out(vty,"%-11s","-");/*mtu*/
					vty_out(vty,"\n");
				}			
			}
		}
	}

	return CMD_SUCCESS;
}


DEFUN(show_ethport_list_cmd_func,
	show_ethport_list_cmd,
	"show eth-port list",
	SHOW_STR
	"Display Ethernet Port information\n"
	"List Ethernet Port list summary\n"
	)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusMessage *npd_query = NULL, *npd_reply = NULL;
	DBusMessage *sem_query = NULL, *sem_reply = NULL;
	DBusError err, sem_err, npd_err;
	DBusMessageIter	 iter, npd_iter, sem_iter;
	DBusMessageIter	 iter_array, npd_iter_array, sem_iter_array;
	unsigned char slot_count = 0;
	unsigned int slotNum, portNum, slot_no;
	int local_slot_id;
	int i = 0, j = 0;
	local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);

	if (is_distributed == DISTRIBUTED_SYSTEM)	
	{
    	int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
		
		vty_out(vty,"%-9s%-16s%-5s%-5s%-7s%-9s%-6s%-11s\n","PORTNO","PORTTYPE","LINK","AUTO","DUPLEX","FLOWCTRL","SPEED","MTU(BYTES)");

		for(i = 0; i < BOARD_GLOBAL_ETHPORTS_MAXNUM*slotNum; i++)
		{	
            if(global_ethport[i]->isValid == VALID_ETHPORT)
			{
				slot_no = global_ethport[i]->slot_no;
				unsigned port_no = global_ethport[i]->local_port_no;
				unsigned int attr_map = global_ethport[i]->attr_bitmap;
				unsigned int port_type = global_ethport[i]->port_type;
				int mtu = global_ethport[i]->mtu;
			    vty_out(vty,"%2d-%-2d    ",slot_no, port_no);

				if(ETH_ATTR_LINKUP == ((attr_map & ETH_ATTR_LINK_STATUS)>>ETH_LINK_STATUS_BIT)) {
    				vty_out(vty,"%-16s",eth_port_type_str[port_type]);
					vty_out(vty,"%-5s",link_status_str[(attr_map & ETH_ATTR_LINK_STATUS) >> ETH_LINK_STATUS_BIT]);
					vty_out(vty,"%-5s",onoff_status_str[(attr_map & ETH_ATTR_AUTONEG) >> ETH_AUTONEG_BIT]);
					vty_out(vty,"%-7s",duplex_status_str[(attr_map & ETH_ATTR_DUPLEX) >> ETH_DUPLEX_BIT]);
					vty_out(vty,"%-9s",onoff_status_str[(attr_map & ETH_ATTR_FLOWCTRL) >> ETH_FLOWCTRL_BIT]);
					vty_out(vty,"%-6s",eth_speed_str[(attr_map & ETH_ATTR_SPEED_MASK) >> ETH_SPEED_BIT]);
					vty_out(vty,"%-11d",mtu);
					vty_out(vty,"\n");
				}
				else {
    				vty_out(vty,"%-16s",eth_port_type_str[port_type]);/*port type*/
					vty_out(vty,"%-5s",link_status_str[(attr_map & ETH_ATTR_LINK_STATUS) >> ETH_LINK_STATUS_BIT]);/*link status*/
					vty_out(vty,"%-5s","-");/*auto-negotiation*/
					vty_out(vty,"%-7s","-");/*duplex status*/
					vty_out(vty,"%-9s","-");/*flow control state*/
					vty_out(vty,"%-6s","-");/*speed*/
					vty_out(vty,"%-11s","-");/*mtu*/
					vty_out(vty,"\n");
				}			
			}
		}
	}
	else
	{
    	query = dbus_message_new_method_call(
    								NPD_DBUS_BUSNAME,		\
    								NPD_DBUS_ETHPORTS_OBJPATH,		\
    								NPD_DBUS_ETHPORTS_INTERFACE,		\
    								NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_ETHPORT_LIST);
    	

    	dbus_error_init(&err);
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

    	dbus_message_iter_init(reply,&iter);
    	dbus_message_iter_get_basic(&iter,&slot_count);

    	/* NOTICE We used 68 bytes here, we still got 12 bytes of summary info*/
    	vty_out(vty,"%-9s%-16s%-5s%-5s%-7s%-9s%-6s%-11s\n","PORTNO","PORTTYPE","LINK","AUTO","DUPLEX","FLOWCTRL","SPEED","MTU(BYTES)");
    	
    	
    	dbus_message_iter_next(&iter);
    	
    	dbus_message_iter_recurse(&iter,&iter_array);

    	for (i = 0; i < slot_count; i++) {
    		DBusMessageIter iter_struct;
    		DBusMessageIter iter_sub_array;
    		unsigned char slotno = 0;
    		unsigned char local_port_count = 0;
    		int j = 0;
    		
    		
    		dbus_message_iter_recurse(&iter_array,&iter_struct);
    		
    		dbus_message_iter_get_basic(&iter_struct,&slotno);
    		/*vty_out(vty,"slotno %d\n",slotno);*/
    		
    		dbus_message_iter_next(&iter_struct);
    		dbus_message_iter_get_basic(&iter_struct,&local_port_count);
    		/*vty_out(vty,"local port count %d\n",local_port_count);*/
    		
    		
    		dbus_message_iter_next(&iter_struct);
    		dbus_message_iter_recurse(&iter_struct,&iter_sub_array);
    		
    		for (j = 0; j < local_port_count; j++) {
    			DBusMessageIter iter_sub_struct;
    			unsigned char portno,porttype;
    			unsigned int attr_map,mtu;
    			unsigned int link_keep_time;
    	/*		
    			Array of Port Infos.
    			port no
    			port type
    			port attriute_bitmap
    			port MTU
    	*/
    			
    			dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
    			
    			dbus_message_iter_get_basic(&iter_sub_struct,&portno);
    			dbus_message_iter_next(&iter_sub_struct);
    			/*vty_out(vty,"local port no %d\n",portno);*/
    			
    			dbus_message_iter_get_basic(&iter_sub_struct,&porttype);
    			dbus_message_iter_next(&iter_sub_struct);
    			
    			dbus_message_iter_get_basic(&iter_sub_struct,&attr_map);
    			dbus_message_iter_next(&iter_sub_struct);
    			
    			dbus_message_iter_get_basic(&iter_sub_struct,&mtu);
    			dbus_message_iter_next(&iter_sub_struct);

    			dbus_message_iter_get_basic(&iter_sub_struct,&link_keep_time);
    			dbus_message_iter_next(&iter_sub_struct);			
    			if(ETH_MAX == porttype) {				
    				dbus_message_iter_next(&iter_sub_array);
    				continue;
    			}
    			if (1 == slot_count) {
    				vty_out(vty,"%-9d",portno);
    			}
    			else if((4 == slot_count) && (2 == local_port_count)) {/* special treat for AX7605i-alpha  by qinhs@autelan 2009-11-19 */
    				vty_out(vty,"cscd%d    ", portno-1);
    			}
    			else {
    				vty_out(vty,"%2d-%-2d    ",slotno,portno);
    			}
    			if(ETH_ATTR_LINKUP == ((attr_map & ETH_ATTR_LINK_STATUS)>>ETH_LINK_STATUS_BIT)) {
    				vty_out(vty,"%-16s",eth_port_type_str[porttype]);
    				vty_out(vty,"%-5s",link_status_str[(attr_map & ETH_ATTR_LINK_STATUS) >> ETH_LINK_STATUS_BIT]);
    				vty_out(vty,"%-5s",onoff_status_str[(attr_map & ETH_ATTR_AUTONEG) >> ETH_AUTONEG_BIT]);
    				vty_out(vty,"%-7s",duplex_status_str[(attr_map & ETH_ATTR_DUPLEX) >> ETH_DUPLEX_BIT]);
    				vty_out(vty,"%-9s",onoff_status_str[(attr_map & ETH_ATTR_FLOWCTRL) >> ETH_FLOWCTRL_BIT]);
    				vty_out(vty,"%-6s",eth_speed_str[(attr_map & ETH_ATTR_SPEED_MASK) >> ETH_SPEED_BIT]);
    				vty_out(vty,"%-11d",mtu);
    				vty_out(vty,"\n");
    			}
    			else {
    				vty_out(vty,"%-16s",eth_port_type_str[porttype]);/*port type*/
    				vty_out(vty,"%-5s",link_status_str[(attr_map & ETH_ATTR_LINK_STATUS) >> ETH_LINK_STATUS_BIT]);/*link status*/
    				vty_out(vty,"%-5s","-");/*auto-negotiation*/
    				vty_out(vty,"%-7s","-");/*duplex status*/
    				vty_out(vty,"%-9s","-");/*flow control state*/
    				vty_out(vty,"%-6s","-");/*speed*/
    				vty_out(vty,"%-11s","-");/*mtu*/
    				vty_out(vty,"\n");
    			}
    			dbus_message_iter_next(&iter_sub_array);
    		
    		}
    		
    		dbus_message_iter_next(&iter_array);
    		
    	}
    		
    	dbus_message_unref(reply);
	}
	
	return CMD_SUCCESS;	
}
int show_eth_port_ipg
(
	struct vty *vty,
	unsigned int value,
	unsigned char type
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter	 iter;
	unsigned char slot_no = 0, port_no = 0;
    unsigned int port_ipg = 0;
	unsigned int op_ret = 0;
	if(0 == type){
		slot_no = (unsigned char)((value>>8)&0xff);
		port_no = (unsigned char)(value & 0xff);
		value = 0xffff; 
	}

    if (is_distributed == DISTRIBUTED_SYSTEM)
    {
		unsigned int slot_no;
        int local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
		
		SLOT_PORT_ANALYSIS_SLOT(value, slot_no);
		
    	query = dbus_message_new_method_call(
    								NPD_DBUS_BUSNAME,		\
    								NPD_DBUS_ETHPORTS_OBJPATH,	\
    								NPD_DBUS_ETHPORTS_INTERFACE,	\
    								NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_ETHPORT_IPG);
    	
    	dbus_error_init(&err);

    	dbus_message_append_args(query,
    							 DBUS_TYPE_BYTE,&type,
    							 DBUS_TYPE_BYTE,&slot_no,
    							 DBUS_TYPE_BYTE,&port_no,
    							 DBUS_TYPE_UINT32,&value,
    							 DBUS_TYPE_INVALID);

    	if(NULL == dbus_connection_dcli[slot_no]->dcli_dbus_connection) 				
    	{
    		if(slot_no == local_slot_id) 
    		{
    		    reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
    		}
    		else 
    		{	
    			vty_out(vty, "Connection to slot%d is not exist.\n", slot_no);
    			return CMD_WARNING;
    		}
    	}else {
    		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_no]->dcli_dbus_connection,\
    				query, -1, &err);
    	}
		
		dbus_message_unref(query);
    	if (NULL == reply) {
    		vty_out(vty,"failed get reply.\n");
    		if (dbus_error_is_set(&err)) {
    			vty_out(vty,"%s raised: %s",err.name,err.message);
    			dbus_error_free_for_dcli(&err);
    		}
    		return CMD_SUCCESS;
    	}

    	if (dbus_message_get_args ( reply, &err,
    		DBUS_TYPE_UINT32,&op_ret,
    		DBUS_TYPE_UINT32,&port_ipg,
    		DBUS_TYPE_INVALID)) 
    	{
			if(ETHPORT_RETURN_CODE_ERR_NONE == op_ret)
			{
				vty_out(vty,"=================================\n");
			    vty_out(vty,"%-18s%-18s\n","IPG ","VALUE");
	            vty_out(vty,"%-18s%-18s\n","---------------","---------------");
				vty_out(vty,"%-18s %d\n","ipg",port_ipg);
				vty_out(vty,"=================================\n");
			}
    		else if(ETHPORT_RETURN_CODE_NO_SUCH_PORT == op_ret)
    		{
    			vty_out(vty,"%% The eth-port is not asic port, check please!\n");
    		}			
			else if (ETHPORT_RETURN_CODE_ERR_HW == op_ret ) 
			{
			    vty_out(vty,"%% Execute failed on hardware !\n"); 
			}
			else
			{
			    vty_out(vty,"%% Execute failed !\n"); 				
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
	else
	{
    	query = dbus_message_new_method_call(
    								NPD_DBUS_BUSNAME,		\
    								NPD_DBUS_ETHPORTS_OBJPATH,	\
    								NPD_DBUS_ETHPORTS_INTERFACE,	\
    								NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_ETHPORT_IPG);
    	
    	dbus_error_init(&err);

    	dbus_message_append_args(query,
    							 DBUS_TYPE_BYTE,&type,
    							 DBUS_TYPE_BYTE,&slot_no,
    							 DBUS_TYPE_BYTE,&port_no,
    							 DBUS_TYPE_UINT32,&value,
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

    	if (dbus_message_get_args ( reply, &err,
    		DBUS_TYPE_UINT32,&op_ret,
    		DBUS_TYPE_BYTE,&port_ipg,
    		DBUS_TYPE_INVALID)) {
    			if(ETHPORT_RETURN_CODE_ERR_NONE == op_ret)
    			{
    				vty_out(vty,"=================================\n");
    			    vty_out(vty," %-18s %-20s\n","IPG ","VALUE");
    	            vty_out(vty,"%-20s%-20s\n","-----------------","-----------------");
    				vty_out(vty,"%-20s %d\n","ipg",port_ipg);
    				vty_out(vty,"=================================\n");
    			}
    			else if(ETHPORT_RETURN_CODE_BOARD_IPG == op_ret)
    			{
    				vty_out(vty,"=================================\n");
    			    vty_out(vty," %-18s %-20s\n","IPG ","VALUE");
    	            vty_out(vty,"%-20s%-20s\n","-----------------","-----------------");
    				vty_out(vty,"%-20s %d\n","ipg",port_ipg);
    				/*vty_out(vty,"ipg value is = %d \n",port_ipg);*/
    				vty_out(vty,"=================================\n");

    			}
    			else if (ETHPORT_RETURN_CODE_NO_SUCH_PORT == op_ret) 
    			{
    				vty_out(vty,"Error:Illegal %d/%d,No such port.\n",slot_no,port_no);/*Stay here,not enter eth_config_node CMD.*/
    			}
    			else if (ETHPORT_RETURN_CODE_ERR_GENERAL == op_ret ) 
    			{
    			    vty_out(vty,"execute command failed.\n"); 
    			}
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
}


int show_eth_port_atrr
(
	struct vty *vty,
	unsigned int value,
	unsigned char type
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	unsigned char slot_no = 0, port_no = 0;
	struct eth_port_s ethPortInfo;
	unsigned int attr_map = 0;
	unsigned int op_ret = 0, link_keep_time = 0;;
	
    int local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
	memset(&ethPortInfo,0,sizeof(struct eth_port_s));
	
	if (is_distributed == DISTRIBUTED_SYSTEM)
	{
		unsigned int eth_g_index = value;
		slot_no = global_ethport[eth_g_index]->slot_no;
		port_no = global_ethport[eth_g_index]->local_port_no;
        ethPortInfo.port_type = global_ethport[eth_g_index]->port_type;
		attr_map = global_ethport[eth_g_index]->attr_bitmap;
		ethPortInfo.mtu = global_ethport[eth_g_index]->mtu;
				
		vty_out(vty,"Detail Information of Ethernet Port %d-%d\n", slot_no, port_no);
		vty_out(vty,"========================================\n");
		vty_out(vty,"%-18s: %-18s\n"," Port Type",				\
							eth_port_type_str[ethPortInfo.port_type]);
		vty_out(vty,"%-18s: %-18s\n"," Admin Status",			\
							onoff_status_str[(attr_map & ETH_ATTR_ADMIN_STATUS) >> ETH_ADMIN_STATUS_BIT]);
		vty_out(vty,"%-18s: %-18s\n"," Link Status",			\
							link_status_str[(attr_map & ETH_ATTR_LINK_STATUS) >> ETH_LINK_STATUS_BIT]);
		/*vty_out(vty,"%-18s: %-18s\n"," Auto-Negotiation",		\
							doneOrnot_status_str[(attr_map & ETH_ATTR_AUTONEG) >> ETH_AUTONEG_BIT]);*/
		vty_out(vty,"%-18s: %-18s\n"," AN-SPEED",					\
							onoff_status_str[(attr_map & ETH_ATTR_AUTONEG_SPEED) >> ETH_AUTONEG_SPEED_BIT]);
		vty_out(vty,"%-18s: %-18s\n"," AN-DUPLEX",					\
							onoff_status_str[(attr_map & ETH_ATTR_AUTONEG_DUPLEX) >> ETH_AUTONEG_DUPLEX_BIT]);
		vty_out(vty,"%-18s: %-18s\n"," AN-FLOWCTRL",					\
							onoff_status_str[(attr_map & ETH_ATTR_AUTONEG_FLOWCTRL) >> ETH_AUTONEG_FLOWCTRL_BIT]);
		vty_out(vty,"%-18s: %-18s\n"," Duplex Mode",			\
							duplex_status_str[(attr_map & ETH_ATTR_DUPLEX) >> ETH_DUPLEX_BIT]);
		vty_out(vty,"%-18s: %-18s\n"," Flow Control",			\
							onoff_status_str[(attr_map & ETH_ATTR_FLOWCTRL) >> ETH_FLOWCTRL_BIT]);
		vty_out(vty,"%-18s: %-18s\n"," Back Pressure",			\
							onoff_status_str[(attr_map & ETH_ATTR_BACKPRESSURE) >> ETH_BACKPRESSURE_BIT]);
		vty_out(vty,"%-18s: %-18s\n"," Speed",					\
							eth_speed_str[(attr_map & ETH_ATTR_SPEED_MASK) >> ETH_SPEED_BIT]);
		
		if(((attr_map & ETH_ATTR_PREFERRED_COPPER_MEDIA) >> ETH_PREFERRED_COPPER_MEDIA_BIT))
			vty_out(vty,"%-18s: %-18s\n"," Preferred Media","COPPER");
		else if(((attr_map & ETH_ATTR_PREFERRED_FIBER_MEDIA) >> ETH_PREFERRED_FIBER_MEDIA_BIT))
			vty_out(vty,"%-18s: %-18s\n"," Preferred Media","FIBER");
		else
			vty_out(vty,"%-18s: %-18s\n"," Preferred Media","-");
		
		vty_out(vty,"%-18s: %d\n"," MTU",ethPortInfo.mtu);
		vty_out(vty,"========================================\n");

    	return CMD_SUCCESS;

	}

	if(0 == type){
		slot_no = (unsigned char)((value>>8)&0xff);
		port_no = (unsigned char)(value & 0xff);
		value = 0xffff; 
	}
	
	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,		\
								NPD_DBUS_ETHPORTS_OBJPATH,	\
								NPD_DBUS_ETHPORTS_INTERFACE,	\
								NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_ETHPORT_ATTR);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&type,
							 DBUS_TYPE_BYTE,&slot_no,
							 DBUS_TYPE_BYTE,&port_no,
							 DBUS_TYPE_UINT32,&value,
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

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32, &op_ret,
		DBUS_TYPE_UINT32,&(ethPortInfo.port_type),
		DBUS_TYPE_UINT32,&(ethPortInfo.attr_bitmap),
		DBUS_TYPE_UINT32,&(ethPortInfo.mtu),
		DBUS_TYPE_UINT32,&(link_keep_time),		
		DBUS_TYPE_INVALID)) {
			if (ETHPORT_RETURN_CODE_ERR_NONE == op_ret ) {
				attr_map = ethPortInfo.attr_bitmap;
				if(0 == type)
					vty_out(vty,"Detail Information of Ethernet Port %d-%d\n",slot_no,port_no);
				else {
					dcli_get_slot_port_by_portindex(vty,value, &slot_no,&port_no);
					vty_out(vty,"Detail Information of Ethernet Port %d-%d\n",slot_no,port_no);
				}
				vty_out(vty,"========================================\n");
				vty_out(vty,"%-18s: %-18s\n"," Port Type",				\
									eth_port_type_str[ethPortInfo.port_type]);
				vty_out(vty,"%-18s: %-18s\n"," Admin Status",			\
									onoff_status_str[(attr_map & ETH_ATTR_ADMIN_STATUS) >> ETH_ADMIN_STATUS_BIT]);
				vty_out(vty,"%-18s: %-18s\n"," Link Status",			\
									link_status_str[(attr_map & ETH_ATTR_LINK_STATUS) >> ETH_LINK_STATUS_BIT]);
				/*vty_out(vty,"%-18s: %-18s\n"," Auto-Negotiation",		\
									doneOrnot_status_str[(attr_map & ETH_ATTR_AUTONEG) >> ETH_AUTONEG_BIT]);*/
				vty_out(vty,"%-18s: %-18s\n"," AN-SPEED",					\
									onoff_status_str[(attr_map & ETH_ATTR_AUTONEG_SPEED) >> ETH_AUTONEG_SPEED_BIT]);
				vty_out(vty,"%-18s: %-18s\n"," AN-DUPLEX",					\
									onoff_status_str[(attr_map & ETH_ATTR_AUTONEG_DUPLEX) >> ETH_AUTONEG_DUPLEX_BIT]);
				vty_out(vty,"%-18s: %-18s\n"," AN-FLOWCTRL",					\
									onoff_status_str[(attr_map & ETH_ATTR_AUTONEG_FLOWCTRL) >> ETH_AUTONEG_FLOWCTRL_BIT]);
				vty_out(vty,"%-18s: %-18s\n"," Duplex Mode",			\
									duplex_status_str[(attr_map & ETH_ATTR_DUPLEX) >> ETH_DUPLEX_BIT]);
				vty_out(vty,"%-18s: %-18s\n"," Flow Control",			\
									onoff_status_str[(attr_map & ETH_ATTR_FLOWCTRL) >> ETH_FLOWCTRL_BIT]);
				vty_out(vty,"%-18s: %-18s\n"," Back Pressure",			\
									onoff_status_str[(attr_map & ETH_ATTR_BACKPRESSURE) >> ETH_BACKPRESSURE_BIT]);
				vty_out(vty,"%-18s: %-18s\n"," Speed",					\
									eth_speed_str[(attr_map & ETH_ATTR_SPEED_MASK) >> ETH_SPEED_BIT]);

				if(((attr_map & ETH_ATTR_PREFERRED_COPPER_MEDIA) >> ETH_PREFERRED_COPPER_MEDIA_BIT))
					vty_out(vty,"%-18s: %-18s\n"," Preferred Media","COPPER");
				else if(((attr_map & ETH_ATTR_PREFERRED_FIBER_MEDIA) >> ETH_PREFERRED_FIBER_MEDIA_BIT))
					vty_out(vty,"%-18s: %-18s\n"," Preferred Media","FIBER");
				else
					vty_out(vty,"%-18s: %-18s\n"," Preferred Media","-");
				
				vty_out(vty,"%-18s: %d\n"," MTU",ethPortInfo.mtu);
				vty_out(vty,"========================================\n");
			}
			else if (ETHPORT_RETURN_CODE_NO_SUCH_PORT == op_ret) 
			{
        		vty_out(vty,"%% Error:Illegal %d/%d,No such port.\n",slot_no,port_no);/*Stay here,not enter eth_config_node CMD.*/
			}
			else if (ETHPORT_RETURN_CODE_ERR_GENERAL == op_ret ) 
			{
				vty_out(vty,"%% Execute command failed.\n"); 
			}
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
DEFUN(show_ethport_cn_ipg_cmd_func,
	show_ethport_cn_ipg_cmd,
	"show eth-port PORTNO ipg",
	SHOW_STR
	"Display Ethernet Port information\n"
	CONFIG_ETHPORT_STR 
	"Display Ethernet Port ipg information\n"
	)
{

	unsigned char slot_no = 0, port_no = 0;
	unsigned char type = 0;
	unsigned int value = 0;
	int ret = 0;

	ret = parse_slotport_no((char *)argv[0],&slot_no,&port_no);
	if (NPD_FAIL == ret) {
    	vty_out(vty,"Unknow portno format.\n");
		return CMD_WARNING;
	}

	if (is_distributed == DISTRIBUTED_SYSTEM)	
	{
		int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
		if(slot_no > slotNum || slot_no <= 0)
		{
			vty_out(vty,"%% NO SUCH SLOT %d!\n", slot_no);
            return CMD_WARNING;
		}
	}
	
	type = 0;
	value = slot_no;
	value =  (value << 8) |port_no;
	ret = show_eth_port_ipg(vty,value,type);
	if(ret != CMD_SUCCESS)
		vty_out(vty,"show attrs error\n");
	
	return ret;
	
}

DEFUN(show_ethport_ipg_cmd_func,
	show_ethport_ipg_cmd,
	"show eth-port ipg",
	SHOW_STR
	"Display Ethernet Port information\n"
	"Display Ethernet Port attributes information\n"
	)
{

	int ret = 0;
	unsigned char type = 0;
	unsigned int value = (unsigned int)vty->index;
	type = 1;
	ret = show_eth_port_ipg(vty,value,type);
	if(ret != CMD_SUCCESS)
		vty_out(vty,"show attrs error\n");
	
	return ret;
	
}


DEFUN(show_ethport_cn_attr_cmd_func,
	show_ethport_cn_attr_cmd,
	"show eth-port PORTNO attributes",
	SHOW_STR
	"Display Ethernet Port information\n"
	CONFIG_ETHPORT_STR 
	"Display Ethernet Port attributes information\n"
	)
{

	unsigned char slot_no = 0, port_no = 0;
	unsigned char type = 0;
	unsigned int value = 0;
	int ret = 0;
	
	ret = parse_slotport_no((char *)argv[0],&slot_no,&port_no);

	if (NPD_FAIL == ret) {
    	vty_out(vty,"Unknow portno format.\n");
		return CMD_WARNING;
	}

	if (is_distributed == DISTRIBUTED_SYSTEM)	
	{
		int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
		if(slot_no > slotNum || slot_no <= 0)
		{
			vty_out(vty,"%% NO SUCH SLOT %d!\n", slot_no);
            return CMD_WARNING;
		}
	}
	
	DCLI_DEBUG(("392 :: show slot/port %d/%d info\n",slot_no,port_no));
	
	type = 0;
	value = slot_no;
	value =  (value << 8) |port_no;
	
    if (is_distributed == DISTRIBUTED_SYSTEM)
    {
        value = SLOT_PORT_COMBINATION(slot_no, port_no);
	}
	
	ret = show_eth_port_atrr(vty,value,type);
	if(ret != CMD_SUCCESS)
		vty_out(vty,"show attrs error\n");
	
	return ret;
	
}

DEFUN(show_ethport_en_attr_cmd_func,
	show_ethport_en_attr_cmd,
	"show eth-port attributes",
	SHOW_STR
	"Display Ethernet Port information\n"
	"Display Ethernet Port attributes information\n"
	)
{

	int ret = 0;
	unsigned char type = 0;
	unsigned int value = (unsigned int)vty->index;

	/*vty_out(vty,"show port_index %d info\n",value);*/
	type = 1;
	ret = show_eth_port_atrr(vty,value,type);
	if(ret != CMD_SUCCESS)
		vty_out(vty,"show attrs error\n");
	
	return ret;
	
}

int show_eth_port_stat
(
	struct vty* vty,
	unsigned int value,
	unsigned char type
)
{
	if(is_distributed == DISTRIBUTED_SYSTEM)
	{
    	DBusMessage *query = NULL, *reply = NULL;
    	DBusError err;
    	DBusMessageIter	 iter;
    	DBusMessageIter	 iter_struct,iter_array;
    	unsigned char slot_no = 0, port_no = 0;
    	unsigned int op_ret = 0, ret = 0, i = 0;
    	unsigned long long tmp = 0;
    	unsigned int linkupcount = 0, linkdowncount = 0;
        int local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);    
    	struct eth_port_counter_s *ptr = NULL, stat;

    	ptr = &stat;
    	memset(ptr,0,sizeof(struct eth_port_counter_s));

		slot_no = (unsigned char)(((value>>6) & 0x1f) + 1);
		port_no = (unsigned char)((value & 0x3f) + 1);

    	query = dbus_message_new_method_call(
    								SEM_DBUS_BUSNAME,		\
    								SEM_DBUS_ETHPORTS_OBJPATH,		\
    								SEM_DBUS_ETHPORTS_INTERFACE,		\
    								SEM_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_ETHPORT_STAT);
    	

    	dbus_error_init(&err);
    	dbus_message_append_args(query,
    							 DBUS_TYPE_BYTE,&type,
    							 DBUS_TYPE_BYTE,&slot_no,
    							 DBUS_TYPE_BYTE,&port_no,
    							 DBUS_TYPE_UINT32,&value,
    							 DBUS_TYPE_INVALID);
		
    	if(NULL == dbus_connection_dcli[slot_no]->dcli_dbus_connection) 				
    	{
    		if(slot_no == local_slot_id) 
    		{
    		    reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
    		}
    		else 
    		{	
    			vty_out(vty, "Connection to slot%d is not exist.\n", slot_no);
    			return CMD_WARNING;
    		}
    	}else {
    		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_no]->dcli_dbus_connection,\
    				query, -1, &err);
    	}
		
    	dbus_message_unref(query);
    	if (NULL == reply) {
    		vty_out(vty,"failed get reply...Please check sem on slot:%d\n",slot_no);

    		if (dbus_error_is_set(&err)) {
    			dbus_error_free_for_dcli(&err);
    		}
    		return CMD_SUCCESS;
    	}

    	dbus_message_iter_init(reply,&iter);
    	dbus_message_iter_get_basic(&iter,&op_ret);

    	/* NOTICE We used 68 bytes here, we still got 12 bytes of summary info
    	//vty_out(vty,"here start\n");*/
    	if(ETHPORT_RETURN_CODE_ERR_NONE == op_ret){
    		
    		dbus_message_iter_next(&iter);	
    		dbus_message_iter_recurse(&iter,&iter_array);

    		for(i = 0; i < 32 ; i++)
    		{
    			dbus_message_iter_recurse(&iter_array,&iter_struct);
    			dbus_message_iter_get_basic(&iter_struct,&tmp);
    			/*vty_out(vty,"tmp %d value %lld\n",i,tmp);*/
    			dbus_message_iter_next(&iter_struct);
    			*((unsigned long long*)ptr + i) = tmp;
    			tmp = 0ULL;
    			
    			dbus_message_iter_get_basic(&iter_struct,&linkupcount);
    			dbus_message_iter_next(&iter_struct);
    			dbus_message_iter_get_basic(&iter_struct,&linkdowncount);
    			dbus_message_iter_next(&iter_struct);
    			
    			dbus_message_iter_next(&iter_array);
    		}
    	
    		vty_out(vty,"======================================================\n");
    		vty_out(vty,"RX\t");
    		tmp = stat.rx.goodbytesh;
    		tmp = (tmp<<32) | stat.rx.goodbytesl;

			vty_out(vty,"octets:              %lld\n", tmp);		
            vty_out(vty,"\tpackets:             %lld\n", stat.rx.badbytes);		
			vty_out(vty,"\tgoodOctets:          %lld\n", stat.rx.uncastframes);
			vty_out(vty,"\tbroadcastPkts:       %lld\n", stat.rx.bcastframes);
			vty_out(vty,"\tmutilcastPkts:       %lld\n", stat.rx.mcastframes);
    		vty_out(vty,"\tgoodPackets:         %lld\n", stat.rx.fcframe);
    		vty_out(vty,"\tdroppedPkts:         %lld\n", stat.rx.fifooverruns);
    		vty_out(vty,"\terrorPkts:           %lld\n", stat.rx.underSizepkt);
    		vty_out(vty,"\tfcsAlign_errPkts:    %lld\n", stat.rx.fragments);
    		vty_out(vty,"\toverSizePkts:        %lld\n", stat.rx.overSizepkt);
    		vty_out(vty,"\toverSizeCrcPkts:     %lld\n", stat.rx.jabber);	
    		vty_out(vty,"\tunderSizePkts:       %lld\n", stat.rx.errorframe);
            vty_out(vty,"\tunderSizeCrcPkts:    %lld\n", stat.otc.badCRC);
			vty_out(vty,"\tpciRawPkts:          %lld\n", stat.otc.collision);
			vty_out(vty,"\n");
    		vty_out(vty,"\t1518tomaxPkts:       %lld\n", stat.otc.late_collision);
    		vty_out(vty,"\t1024to1518Pkts:      %lld\n", stat.otc.b1024oct2max);
    		vty_out(vty,"\t512to1023Pkts:       %lld\n", stat.otc.b512oct21023);
    		vty_out(vty,"\t256to511Pkts:        %lld\n", stat.otc.b256oct511);
			vty_out(vty,"\t128to255Pkts:        %lld\n", stat.otc.b128oct255);
			vty_out(vty,"\t65to127Pkts:         %lld\n", stat.otc.b64oct127);			
			vty_out(vty,"\t64Pkts:              %lld\n", stat.otc.b64oct);
            
    		vty_out(vty,"\nTX");
    		/*
    		vty_out(vty,"packets:%lld  errors:%lld  dropped:%lld  overruns:%lld  frame:%lld\n",  \
    								stat.tx.packets,stat.tx.errors,stat.tx.dropped,stat.tx.overruns,stat.tx.frame);
    		*/
    		tmp = stat.tx.goodbytesh;
    		tmp = (tmp<<32) | stat.tx.goodbytesl;
    		vty_out(vty,"\tgoodOctets:          %lld\n", tmp);    		
    		vty_out(vty,"\tpackets:             %lld\n", stat.tx.uncastframe);
			vty_out(vty,"\tdoorBell:            %lld\n",  stat.tx.excessiveCollision << 32 | stat.tx.sent_deferred);
		
    		vty_out(vty,"======================================================\n");

    		/*vty_out(vty,"\tlinkupcount:%d linkdowncount:%d",linkupcount,linkdowncount);*/	
			
        	dbus_message_unref(reply);
        	return CMD_SUCCESS;
    	}
    	else if (ETHPORT_RETURN_CODE_NO_SUCH_PORT == op_ret) {
        	vty_out(vty,"Error:Illegal %d/%d,No such port.\n",slot_no,port_no);//Stay here,not enter eth_config_node CMD.
            dbus_message_unref(reply);
    	    return CMD_WARNING;	    	
		}
    	else if (INTERFACE_RETURN_CODE_FPGA_ETH_XGE_FIBER == op_ret) {
        	vty_out(vty,"this port don't support this command!\n",slot_no,port_no);
            dbus_message_unref(reply);
    	    return CMD_WARNING;	    	
		}
		else if(INTERFACE_RETURN_CODE_UNSUPPORT_COMMAND == op_ret){
			// TODO:call npd dbus to process.
			DCLI_DEBUG(("%% call npd dbus to process\n"));
            dbus_message_unref(reply);
		}
    	else if (ETHPORT_RETURN_CODE_ERR_GENERAL == op_ret ) {
    		vty_out(vty,"execute command failed.\n"); 
            dbus_message_unref(reply);
    	    return CMD_WARNING;
    	}
        
    	query = dbus_message_new_method_call(
    								NPD_DBUS_BUSNAME,		\
    								NPD_DBUS_ETHPORTS_OBJPATH,		\
    								NPD_DBUS_ETHPORTS_INTERFACE,		\
    								NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_ETHPORT_STAT);
    	

    	dbus_error_init(&err);
    	dbus_message_append_args(query,
    							 DBUS_TYPE_BYTE,&type,
    							 DBUS_TYPE_BYTE,&slot_no,
    							 DBUS_TYPE_BYTE,&port_no,
    							 DBUS_TYPE_UINT32,&value,
    							 DBUS_TYPE_INVALID);
		
    	if(NULL == dbus_connection_dcli[slot_no]->dcli_dbus_connection) 				
    	{
    		if(slot_no == local_slot_id) 
    		{
    		    reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
    		}
    		else 
    		{	
    			vty_out(vty, "Connection to slot%d is not exist.\n", slot_no);
    			return CMD_WARNING;
    		}
    	}else {
    		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_no]->dcli_dbus_connection,\
    				query, -1, &err);
    	}
		
    	dbus_message_unref(query);
    	if (NULL == reply) {
    		vty_out(vty,"failed get reply...Please check npd on slot:%d\n",slot_no);

    		if (dbus_error_is_set(&err)) {
    			dbus_error_free_for_dcli(&err);
    		}
    		return CMD_SUCCESS;
    	}

    	dbus_message_iter_init(reply,&iter);
    	dbus_message_iter_get_basic(&iter,&op_ret);

    	/* NOTICE We used 68 bytes here, we still got 12 bytes of summary info
    	//vty_out(vty,"here start\n");*/
    	if(ETHPORT_RETURN_CODE_ERR_NONE == op_ret){
    		
    		dbus_message_iter_next(&iter);	
    		dbus_message_iter_recurse(&iter,&iter_array);

    		for(i = 0; i < 32 ; i++)
    		{
    			dbus_message_iter_recurse(&iter_array,&iter_struct);
    			dbus_message_iter_get_basic(&iter_struct,&tmp);
    			/*vty_out(vty,"tmp %d value %lld\n",i,tmp);*/
    			dbus_message_iter_next(&iter_struct);
    			*((unsigned long long*)ptr + i) = tmp;
    			tmp = 0ULL;
    			
    			dbus_message_iter_get_basic(&iter_struct,&linkupcount);
    			dbus_message_iter_next(&iter_struct);
    			dbus_message_iter_get_basic(&iter_struct,&linkdowncount);
    			dbus_message_iter_next(&iter_struct);
    			
    			dbus_message_iter_next(&iter_array);
    		}
    		/*vty_out(vty,"here end\n");*/
    		

    		if(0 == type)
    			vty_out(vty,"Detail Information of Ethernet Port %d-%d\n",slot_no,port_no);
    		else {
    			dcli_get_slot_port_by_portindex(vty,value, &slot_no,&port_no);
    			vty_out(vty,"Detail Information of Ethernet Port %d-%d\n",slot_no,port_no);
    		}
    		vty_out(vty,"======================================================\n");
    		vty_out(vty,"RX\t");
    		tmp = stat.rx.goodbytesh;
    		tmp = (tmp<<32) | stat.rx.goodbytesl;

			vty_out(vty,"goodbytes:            %lld  ( %lld MiB )\n",	\
    								tmp, (tmp!=0) ? (tmp>>20) : 0);		
            vty_out(vty,"\tbadbytes:             %lld  ( %lld MiB )\n",	\
    								stat.rx.badbytes,(stat.rx.badbytes!=0) ? (stat.rx.badbytes>>20) : 0);		
			vty_out(vty,"\tunicastpkts:          %lld\n", stat.rx.uncastframes);
			vty_out(vty,"\tbroadcastpkts:        %lld\n", stat.rx.bcastframes);
			vty_out(vty,"\tmutilcastpkts:        %lld\n", stat.rx.mcastframes);
    		vty_out(vty,"\tfcframe:              %lld\n", stat.rx.fcframe);
    		vty_out(vty,"\tfifooverruns:         %lld\n", stat.rx.fifooverruns);
    		vty_out(vty,"\tunderSizeframe:       %lld\n", stat.rx.underSizepkt);
    		vty_out(vty,"\tfragments:            %lld\n", stat.rx.fragments);
    		vty_out(vty,"\toverSizeframe:        %lld \n", stat.rx.overSizepkt);
    		vty_out(vty,"\tjabber:               %lld\n", stat.rx.jabber);	
    		vty_out(vty,"\terrorframe:           %lld\n", stat.rx.errorframe);	
    		vty_out(vty,"\nTX");
    		/*
    		vty_out(vty,"packets:%lld  errors:%lld  dropped:%lld  overruns:%lld  frame:%lld\n",  \
    								stat.tx.packets,stat.tx.errors,stat.tx.dropped,stat.tx.overruns,stat.tx.frame);
    		*/
    		tmp = stat.tx.goodbytesh;
    		tmp = (tmp<<32) | stat.tx.goodbytesl;
    		vty_out(vty,"\tgoodbytes:            %lld  ( %lld MiB )\n", tmp, (tmp!=0) ? (tmp>>20) : 0);
    		vty_out(vty,"\tsent_deferred:        %lld\n", stat.tx.sent_deferred);

    		vty_out(vty,"\tunicastframe:         %lld\n", stat.tx.uncastframe);
    		vty_out(vty,"\texcessiveCollision:   %lld\n", stat.tx.excessiveCollision);
			vty_out(vty,"\tmutilcastframe:       %lld\n", stat.tx.mcastframe);
			vty_out(vty,"\tbroadcastframe:       %lld\n", stat.tx.bcastframe);
			vty_out(vty,"\tsentMutiple:          %lld\n", stat.tx.sentMutiple);
    		vty_out(vty,"\tfcframe:              %lld\n", stat.tx.fcframe);
    		vty_out(vty,"\tcrcerror_fifooverrun: %lld\n", stat.tx.crcerror_fifooverrun);
    		vty_out(vty,"\n");
    		vty_out(vty,"\tBadCrc:               %lld\n", stat.otc.badCRC);		
    		vty_out(vty,"\tcollision:            %lld\n", stat.otc.collision);
    		vty_out(vty,"\tlate_collision:       %lld\n", stat.otc.late_collision);
    		vty_out(vty,"\t1024tomax_frame:      %lld\n", stat.otc.b1024oct2max);
    		vty_out(vty,"\t512to1023oct_frame:   %lld\n", stat.otc.b512oct21023);
    		vty_out(vty,"\t256to511oct_frame:    %lld\n", stat.otc.b256oct511);
			vty_out(vty,"\t128to255oct_frame:    %lld\n", stat.otc.b128oct255);
			vty_out(vty,"\t65to127oct_frame:     %lld\n", stat.otc.b64oct127);			
			vty_out(vty,"\t64oct_frame:          %lld\n", stat.otc.b64oct);
    		vty_out(vty,"======================================================\n");

    		/*vty_out(vty,"\tlinkupcount:%d linkdowncount:%d",linkupcount,linkdowncount);*/		
    	}
    	else if (ETHPORT_RETURN_CODE_NO_SUCH_PORT == op_ret) {
        	vty_out(vty,"Error:Illegal %d/%d,No such port.\n",slot_no,port_no);//Stay here,not enter eth_config_node CMD.
    	}
    	else if (ETHPORT_RETURN_CODE_ERR_GENERAL == op_ret ) {
    		vty_out(vty,"execute command failed.\n"); 
    	}
    	dbus_message_unref(reply);

    	return CMD_SUCCESS;		
	}
	
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_struct,iter_array;
	unsigned char slot_no = 0, port_no = 0;
	unsigned int op_ret = 0, ret = 0, i = 0;
	unsigned long long tmp = 0;
	unsigned int linkupcount = 0, linkdowncount = 0;

	struct eth_port_counter_s *ptr = NULL, stat;

	ptr = &stat;
	memset(ptr,0,sizeof(struct eth_port_counter_s));

	if(0 == type){
		slot_no = (unsigned char)((value>>8)&0xff);
		port_no = (unsigned char)(value & 0xff);
		value = 0xffff; 
	}

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,		\
								NPD_DBUS_ETHPORTS_OBJPATH,		\
								NPD_DBUS_ETHPORTS_INTERFACE,		\
								NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_ETHPORT_STAT);
	

	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&type,
							 DBUS_TYPE_BYTE,&slot_no,
							 DBUS_TYPE_BYTE,&port_no,
							 DBUS_TYPE_UINT32,&value,
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

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&op_ret);

	/* NOTICE We used 68 bytes here, we still got 12 bytes of summary info
	//vty_out(vty,"here start\n");*/
	if(ETHPORT_RETURN_CODE_ERR_NONE == op_ret){
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);

		for(i = 0; i < 32 ; i++)
		{
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&tmp);
			/*vty_out(vty,"tmp %d value %lld\n",i,tmp);*/
			dbus_message_iter_next(&iter_struct);
			*((unsigned long long*)ptr + i) = tmp;
			tmp = 0ULL;
			
			dbus_message_iter_get_basic(&iter_struct,&linkupcount);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&linkdowncount);
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_next(&iter_array);
		}
		/*vty_out(vty,"here end\n");*/
		

		if(0 == type)
			vty_out(vty,"Detail Information of Ethernet Port %d-%d\n",slot_no,port_no);
		else {
			dcli_get_slot_port_by_portindex(vty,value, &slot_no,&port_no);
			vty_out(vty,"Detail Information of Ethernet Port %d-%d\n",slot_no,port_no);
		}
		vty_out(vty,"================================================================================\n");
		vty_out(vty,"RX\t");
		tmp = stat.rx.goodbytesh;
		tmp = (tmp<<32) | stat.rx.goodbytesl;
		vty_out(vty,"goodbytes:%lld  ( %lld  MiB ) badbytes:%lld  ( %lld  MiB )\n",	\
								tmp, (tmp!=0) ? (tmp>>20) : 0,stat.rx.badbytes,(stat.rx.badbytes!=0) ? (stat.rx.badbytes>>20) : 0);		
		vty_out(vty,"\tuncastpkts:%lld  bcastpkts:%lld  mcastpkts:%lld\n",  \
								stat.rx.uncastframes,stat.rx.bcastframes,stat.rx.mcastframes);
		vty_out(vty,"\tfcframe:%lld  fifooverruns:%lld\n",  \
								stat.rx.fcframe,stat.rx.fifooverruns);
		vty_out(vty,"\tunderSizeframe:%lld  fragments:%lld  overSizeframe:%lld \n",  \
								stat.rx.underSizepkt,stat.rx.fragments,stat.rx.overSizepkt);
		vty_out(vty,"\tjabber:%lld  errorframe:%lld\n",  \
								stat.rx.jabber,stat.rx.errorframe);	
		vty_out(vty,"\nTX");
		/*
		vty_out(vty,"packets:%lld  errors:%lld  dropped:%lld  overruns:%lld  frame:%lld\n",  \
								stat.tx.packets,stat.tx.errors,stat.tx.dropped,stat.tx.overruns,stat.tx.frame);
		*/
		tmp = stat.tx.goodbytesh;
		tmp = (tmp<<32) | stat.tx.goodbytesl;
		vty_out(vty,"\tgoodbytes:%lld  ( %lld MiB )  sent_deferred:%lld\n",tmp, (tmp!=0) ? (tmp>>20) : 0,stat.tx.sent_deferred);
		vty_out(vty,"\tuncastframe:%lld  excessiveCollision:%lld\n",  \
								stat.tx.uncastframe,stat.tx.excessiveCollision);
		vty_out(vty,"\tmcastframe:%lld  bcastframe:%lld  sentMutiple:%lld\n",  \
								stat.tx.mcastframe,stat.tx.bcastframe,stat.tx.sentMutiple);
		vty_out(vty,"\tfcframe:%lld  crcerror_fifooverrun:%lld\n",stat.tx.fcframe,stat.tx.crcerror_fifooverrun);
		vty_out(vty,"\n");
		vty_out(vty,"\tBadCrc: %lld  collision:%lld  late_collision:%lld\n",\
								stat.otc.badCRC,stat.otc.collision,stat.otc.late_collision);		
		vty_out(vty,"\t1024tomax_frame:%lld  512to1023oct_frame:%lld  256to511oct_frame:%lld\n",  \
								stat.otc.b1024oct2max,stat.otc.b512oct21023,stat.otc.b256oct511);
		vty_out(vty,"\t128to255oct_frame:%lld  65to127oct_frame:%lld  64oct_frame:%lld\n",  \
								stat.otc.b128oct255,stat.otc.b64oct127,stat.otc.b64oct);
		vty_out(vty,"================================================================================\n");

		/*vty_out(vty,"\tlinkupcount:%d linkdowncount:%d",linkupcount,linkdowncount);*/		
	}
	else if (ETHPORT_RETURN_CODE_NO_SUCH_PORT == op_ret) {
    	vty_out(vty,"Error:Illegal %d/%d,No such port.\n",slot_no,port_no);//Stay here,not enter eth_config_node CMD.
	}
	else if (ETHPORT_RETURN_CODE_ERR_GENERAL == op_ret ) {
		vty_out(vty,"execute command failed.\n"); 
	}
	dbus_message_unref(reply);

	return CMD_SUCCESS;
}

#define HSP_UMSG_HEADER_ERROR \
"No high speed port on this product\n"

#define HSP_UMSG_TAIL_ERROR \
""


#define HSP_UMSG_HEADER_A \
"+--------------+---------------+---------------+\n" \
"|              |    Rx(Hex)    |   tx(Hex)     |\n" \
"+ Description  +---------------+---------------+\n" \
"|              |    HSP0       |   HSP0        |\n" \
"+--------------+---------------+---------------+\n"

#define HSP_UMSG_TAIL_A \
"+--------------+---------------+---------------+\n"

#define HSP_UMSG_HEADER_B \
"+--------------+-----------------------------+-----------------------------+\n" \
"|              |              Rx(Hex)        |              Tx(Hex)        |\n" \
"+ Description  +---------+---------+---------+---------+---------+---------+\n" \
"|              |   HSP0  |   HSP1  |   HSP2  |   HSP0  |   HSP1  |   HSP2  |\n" \
"+--------------+---------+---------+---------+---------+---------+---------+\n"

#define HSP_UMSG_TAIL_B \
"+--------------+---------+---------+---------+---------+---------+--------+\n"


char * portStatDesc[15] = { \
	"Unicast Frames",
	"Bcast Frames",
	"Mcast Frames",
	"FC Frames",
	"Good ByteLow",
	"Good ByteHigh",
	"Bad Bytes",
	"CRC Error"
};

#define HSP_UMSG_HEADER_PRODUCT_AX7605 	HSP_UMSG_HEADER_B
#define HSP_UMSG_HEADER_PRODUCT_AX5612	HSP_UMSG_HEADER_A
#define HSP_UMSG_HEADER_PRODUCT_AX5612i	HSP_UMSG_HEADER_A

#define HSP_UMSG_TAIL_PRODUCT_AX7605 HSP_UMSG_TAIL_B
#define HSP_UMSG_TAIL_PRODUCT_AX5612 HSP_UMSG_TAIL_A
#define HSP_UMSG_TAIL_PRODUCT_AX5612i HSP_UMSG_TAIL_A

int show_eth_xg_port_stat
(
	struct vty* vty
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_struct, iter_array;
	unsigned int op_ret = 0;

	unsigned int j = 0, count =0;
	unsigned int i = 0;
	
	unsigned int stat[MAX_HSP_COUNT][HSP_MIB_ITEM_COUNT_EACH] = {0};
	char *uMsgHeader = NULL, *uMsgTail = NULL;
	unsigned int product_id = 0, hsp_count =0;
	unsigned int value = 0;

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,		\
								NPD_DBUS_ETHPORTS_OBJPATH,		\
								NPD_DBUS_ETHPORTS_INTERFACE,		\
								NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_XG_ETHPORT_STAT);
	dbus_error_init(&err);
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

    dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&op_ret);
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&product_id);
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&hsp_count);
	/*vty_out(vty,"dcli value op_ret %d product_id %d hsp_count %d \n",op_ret,product_id,hsp_count);*/
	
	if(ETHPORT_RETURN_CODE_ERR_NONE == op_ret){	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);	
		for(i = 0; i < hsp_count; i++) {	
			for(j = 0; j < HSP_MIB_ITEM_COUNT_EACH ; j++){	
				dbus_message_iter_recurse(&iter_array,&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&value);
				dbus_message_iter_next(&iter_struct);
				stat[i][j] = value;
				/*vty_out(vty,"%d",value);*/
				dbus_message_iter_next(&iter_array);
			}
		}
	}


	switch(product_id) {
		default:
			uMsgHeader = HSP_UMSG_HEADER_ERROR;
			uMsgTail = HSP_UMSG_TAIL_ERROR;
			break;
		case PRODUCT_ID_AX7K:
			uMsgHeader = HSP_UMSG_HEADER_PRODUCT_AX7605;
			uMsgTail = HSP_UMSG_TAIL_PRODUCT_AX7605;
			break;
		case PRODUCT_ID_AX5K:
			uMsgHeader = HSP_UMSG_HEADER_PRODUCT_AX5612;
			uMsgTail = HSP_UMSG_TAIL_PRODUCT_AX5612;
			break;
		case PRODUCT_ID_AX5K_I:
			uMsgHeader = HSP_UMSG_HEADER_PRODUCT_AX5612i;
			uMsgTail = HSP_UMSG_TAIL_PRODUCT_AX5612i;
			break;
	}

	/* give out user message header */
	vty_out(vty, "%s", uMsgHeader);
	if(hsp_count) {
		for(i = 0; i < (HSP_MIB_ITEM_COUNT_EACH/2); i++) {
			vty_out(vty, "%-1s%-14s%-1s","|", portStatDesc[i],"|");

			/* Rx counter */
			for(j = 0; j < hsp_count; j++) {
				if(PRODUCT_ID_AX7K == product_id) {
					vty_out(vty, "%-9d%-1s",stat[j][i],"|");
					
				}
				else {
					vty_out(vty, "%-15d%-1s",stat[j][i],"|");
				}
			}
			
			/* Tx counter */
			for(j = 0; j < hsp_count; j++) {
				if(PRODUCT_ID_AX7K == product_id) {
				  vty_out(vty, "%-9d%-1s",stat[j][i+(HSP_MIB_ITEM_COUNT_EACH/2)],"|");
				  if(2 == j) {
					vty_out(vty,"    +--------------+---------+---------+---------+---------+---------+---------+");
					                
				  }
				  else{ ;}
				}
				  
			
				else {
					vty_out(vty, "%-15d%-1s\n",stat[j][i+(HSP_MIB_ITEM_COUNT_EACH/2)],"|");
					vty_out(vty,"+--------------+---------------+---------------+");
				}
			}
			
			vty_out(vty, "\n");
		}
	}	
	/* give out user message tail */
	/*vty_out(vty, "%s", uMsgTail);*/
	dbus_message_unref(reply);

	return CMD_SUCCESS;
}

DEFUN(show_ethport_cn_stat_cmd_func,
	show_ethport_cn_stat_cmd,
	"show eth-port PORTNO statistics",
	SHOW_STR
	"Display Ethernet Port information\n"
	CONFIG_ETHPORT_STR 
	"Display Ethernet Port statistics information\n"
	)
{
	int ret = 0;
	unsigned char slot_no = 0,port_no = 0;
	unsigned char type = 0;
	unsigned int value = 0;
	
	ret = parse_slotport_no((char *)argv[0],&slot_no,&port_no);
	if (NPD_FAIL == ret) {
    	vty_out(vty,"Unknow portno format.\n");
		return CMD_WARNING;
	}

	if (is_distributed == DISTRIBUTED_SYSTEM)	
	{
		int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
		if(slot_no > slotNum || slot_no <= 0)
		{
			vty_out(vty,"%% NO SUCH SLOT %d!\n", slot_no);
            return CMD_WARNING;
		}
	}
	
	value = slot_no;
	value = (value << 8) |port_no;
	
    if (is_distributed == DISTRIBUTED_SYSTEM)
    {
        value = SLOT_PORT_COMBINATION(slot_no, port_no);
	}
	
	ret = show_eth_port_stat(vty,value,type);
	if(ret != CMD_SUCCESS)
		vty_out(vty,"show eth-port stat error\n");

	return ret;
}

DEFUN(show_ethport_en_stat_cmd_func,
	show_ethport_en_stat_cmd,
	"show eth-port statistics",
	SHOW_STR
	"Display Ethernet Port information\n"
	"Display Ethernet Port statistics information\n"
	)
{
	int ret = 0;
	unsigned char slot_no = 0, port_no = 0;
	unsigned char type = 1;
	unsigned int value = 0;
	
	value = (unsigned int)vty->index;
	/*DCLI_DEBUG(("show port_index %d info\n",value));*/

	ret = show_eth_port_stat(vty,value,type);
	if(ret != CMD_SUCCESS)
		vty_out(vty,"show eth-port stat error\n");

	return ret;
}
DEFUN(show_xg_ethport_cn_stat_cmd_func,
	show_xg_ethport_cn_stat_cmd,
	"show xg_port statistics",
	SHOW_STR
	"Display Ethernet xg_Port information\n"
	"Display Ethernet xg_Port statistics information\n"
	)
{
	int ret = 0;
	ret = show_eth_xg_port_stat(vty);
	if(ret != CMD_SUCCESS)
		vty_out(vty,"show eth-port stat error\n");

	return ret;
}
int clear_eth_port_stat
(
	struct vty* vty,
	unsigned int value,
	unsigned char type
)
{
	if(is_distributed == DISTRIBUTED_SYSTEM)
	{
    	DBusMessage *query = NULL, *reply = NULL;
    	DBusError err;
    	DBusMessageIter	 iter;
    	DBusMessageIter	 iter_struct, iter_array;
    	unsigned char slot_no = 0, port_no = 0;
    	unsigned int op_ret = 0, ret = 0, i = 0;
    	unsigned long long tmp = 0;
    	struct eth_port_counter_s *ptr, stat = {{0}};
    	ptr = &stat;

		slot_no = (unsigned char)(((value>>6) & 0x1f) + 1);
		port_no = (unsigned char)((value & 0x3f) + 1);
		
        int local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
		
    	/*DCLI_DEBUG(("445 :: show slot/port %d/%d info\n",slot_no,port_no));*/

    	query = dbus_message_new_method_call(
    								SEM_DBUS_BUSNAME,		\
    								SEM_DBUS_ETHPORTS_OBJPATH,		\
    								SEM_DBUS_ETHPORTS_INTERFACE,		\
    								SEM_DBUS_ETHPORTS_INTERFACE_METHOD_CLEAR_ETHPORT_STAT);
    	

    	dbus_error_init(&err);
    	dbus_message_append_args(query,
    							 DBUS_TYPE_BYTE,&type,
    							 DBUS_TYPE_BYTE,&slot_no,
    							 DBUS_TYPE_BYTE,&port_no,
    							 DBUS_TYPE_UINT32,&value,
    							 DBUS_TYPE_INVALID);
		
    	if(NULL == dbus_connection_dcli[slot_no]->dcli_dbus_connection) 				
    	{
    		if(slot_no == local_slot_id) 
    		{
    		    reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
    		}
    		else 
    		{	
    			vty_out(vty, "Connection to slot%d is not exist.\n", slot_no);
    			return CMD_WARNING;
    		}
    	}else {
    		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_no]->dcli_dbus_connection,\
    				query, -1, &err);
    	}    	
		
    	dbus_message_unref(query);
    	if (NULL == reply) {
    		vty_out(vty,"failed get reply...Please check npd on slot:%d\n",slot_no);
			/*
    		if (dbus_error_is_set(&err)) {
    			printf("%s raised: %s",err.name,err.message);
    			dbus_error_free_for_dcli(&err);
    		}*/
    		return CMD_SUCCESS;
    	}

    	if (dbus_message_get_args ( reply, &err,
    					DBUS_TYPE_UINT32, &op_ret,
    					DBUS_TYPE_INVALID)) {
    		if(ETHPORT_RETURN_CODE_ERR_NONE == op_ret) {
				dbus_message_unref(reply);
    	        return CMD_SUCCESS;	
    			/*vty_out(vty,"clear stat ok!\n");*/
    		}
    		else if (ETHPORT_RETURN_CODE_NO_SUCH_PORT == op_ret){
    			vty_out(vty,"%% Bad parameter :Bad Slot/Port number.\n");
				dbus_message_unref(reply);
    	        return CMD_WARNING;	
    		}
    		else if (ETHPORT_RETURN_CODE_ERR_HW == op_ret){
    			vty_out(vty,"%% Bad parameter :Error occures on HW operation.\n");
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
			dbus_message_unref(reply);
	        return CMD_WARNING;	
    	}
	

    	query = dbus_message_new_method_call(
    								NPD_DBUS_BUSNAME,		\
    								NPD_DBUS_ETHPORTS_OBJPATH,		\
    								NPD_DBUS_ETHPORTS_INTERFACE,		\
    								NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CLEAR_ETHPORT_STAT);
    	

    	dbus_error_init(&err);
    	dbus_message_append_args(query,
    							 DBUS_TYPE_BYTE,&type,
    							 DBUS_TYPE_BYTE,&slot_no,
    							 DBUS_TYPE_BYTE,&port_no,
    							 DBUS_TYPE_UINT32,&value,
    							 DBUS_TYPE_INVALID);
		
    	if(NULL == dbus_connection_dcli[slot_no]->dcli_dbus_connection) 				
    	{
    		if(slot_no == local_slot_id) 
    		{
    		    reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
    		}
    		else 
    		{	
    			vty_out(vty, "Connection to slot%d is not exist.\n", slot_no);
    			return CMD_WARNING;
    		}
    	}else {
    		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_no]->dcli_dbus_connection,\
    				query, -1, &err);
    	}   
		
    	dbus_message_unref(query);
    	if (NULL == reply) {
    		vty_out(vty,"failed get reply...Please check npd on slot:%d\n",slot_no);
			/*
    		if (dbus_error_is_set(&err)) {
    			printf("%s raised: %s",err.name,err.message);
    			dbus_error_free_for_dcli(&err);
    		}*/
    		return CMD_SUCCESS;
    	}

    	if (dbus_message_get_args ( reply, &err,
    					DBUS_TYPE_UINT32, &op_ret,
    					DBUS_TYPE_INVALID)) {
    		if(ETHPORT_RETURN_CODE_ERR_NONE == op_ret) {
    			/*vty_out(vty,"clear stat ok!\n");*/
    		}
    		else if (ETHPORT_RETURN_CODE_NO_SUCH_PORT == op_ret){
    			vty_out(vty,"%% Bad parameter :Bad Slot/Port number.\n");
    		}
    		else if (ETHPORT_RETURN_CODE_ERR_HW == op_ret){
    			vty_out(vty,"%% Bad parameter :Error occures on HW operation.\n");
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
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_struct, iter_array;
	unsigned char slot_no = 0, port_no = 0;
	unsigned int op_ret = 0, ret = 0, i = 0;
	unsigned long long tmp = 0;
	struct eth_port_counter_s *ptr, stat = {{0}};
	ptr = &stat;

	if(0 == type){
		slot_no = (unsigned char)((value>>8)&0xff);
		port_no = (unsigned char)(value & 0xff);
		value = 0xffff; 
	}

	/*DCLI_DEBUG(("445 :: show slot/port %d/%d info\n",slot_no,port_no));*/

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,		\
								NPD_DBUS_ETHPORTS_OBJPATH,		\
								NPD_DBUS_ETHPORTS_INTERFACE,		\
								NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CLEAR_ETHPORT_STAT);
	

	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&type,
							 DBUS_TYPE_BYTE,&slot_no,
							 DBUS_TYPE_BYTE,&port_no,
							 DBUS_TYPE_UINT32,&value,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) {
		if(ETHPORT_RETURN_CODE_ERR_NONE == op_ret) {
			/*vty_out(vty,"clear stat ok!\n");*/
		}
		else if (ETHPORT_RETURN_CODE_NO_SUCH_PORT == op_ret){
			vty_out(vty,"%% Bad parameter :Bad Slot/Port number.\n");
		}
		else if (ETHPORT_RETURN_CODE_ERR_HW == op_ret){
			vty_out(vty,"%% Bad parameter :Error occures on HW operation.\n");
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

#if 0
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&op_ret);

	// NOTICE We used 68 bytes here, we still got 12 bytes of summary info
	if(CMD_SUCCESS == op_ret){
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);

		for(i = 0; i < 29 ; i++)
		{
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&tmp);
			//vty_out(vty,"tmp %d value %lld\n",i,tmp);
			dbus_message_iter_next(&iter_struct);
			*((unsigned long long*)ptr + i) = tmp;
			tmp = 0ULL;
			dbus_message_iter_next(&iter_array);
		}
		

		/*if(0 == type)
			vty_out(vty,"Clear Detail Information of Ethernet Port %d-%d\n",slot_no,port_no);
		else
			vty_out(vty,"Clear Detail Information of Ethernet Port\n");
		*/
		
	}
	else if (NPD_DBUS_ERROR_NO_SUCH_PORT == op_ret) 
	{
    	vty_out(vty,"Error:Illegal %d/%d,No such port.\n",slot_no,port_no);//Stay here,not enter eth_config_node CMD.
	}
	else if (NPD_DBUS_ERROR == op_ret ) 
	{
		vty_out(vty,"execute command failed.\n"); 
	}
#endif
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(clear_ethport_cn_stat_cmd_func,
	clear_ethport_cn_stat_cmd,
	"clear eth-port PORTNO statistics",
	"Clear system information\n"
	"Clear Ethernet Port information\n"
	CONFIG_ETHPORT_STR 
	"Clear Ethernet Port statistics information\n"
	)
{
	int ret = 0;
	unsigned char slot_no = 0, port_no = 0;
	unsigned char type = 0;
	unsigned int value = 0;
	
	ret = parse_slotport_no((char *)argv[0],&slot_no,&port_no);
	if (NPD_FAIL == ret) {
    	vty_out(vty,"Unknow portno format.\n");
		return CMD_WARNING;
	}
	/*DCLI_DEBUG(("563 :: show slot/port %d/%d info\n",slot_no,port_no));*/
	if (is_distributed == DISTRIBUTED_SYSTEM)	
	{
		int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
		if(slot_no > slotNum || slot_no <= 0)
		{
			vty_out(vty,"%% NO SUCH SLOT %d!\n", slot_no);
            return CMD_WARNING;
		}
	}
	
	value = slot_no;
	value =  (value << 8) |port_no;

    if (is_distributed == DISTRIBUTED_SYSTEM)
    {
        value = SLOT_PORT_COMBINATION(slot_no, port_no);
	}

	ret = clear_eth_port_stat(vty,value,type);
	if(ret != CMD_SUCCESS)
		vty_out(vty,"clear eth-port stat error\n");

	return ret;
}

DEFUN(clear_ethport_en_stat_cmd_func,
	clear_ethport_en_stat_cmd,
	"clear eth-port statistics",
	SHOW_STR
	"Display Ethernet Port information\n"
	"Display Ethernet Port statistics information\n"
	)
{
	int ret = 0;
	unsigned char slot_no = 0, port_no = 0;
	unsigned char type = 1;
	unsigned int value = 0;
	
	value = (unsigned int)vty->index;
	/*DCLI_DEBUG(("show port_index %d info\n",value));*/

	ret = clear_eth_port_stat(vty,value,type);
	if(ret != CMD_SUCCESS)
		vty_out(vty,"clear eth-port stat error\n");

	return ret;
}

DEFUN(config_ethernet_port_cmd_func,
	config_ethernet_port_cmd,
	"config eth-port PORTNO",
	CONFIG_STR
	"Config ethernet port information\n"
	CONFIG_ETHPORT_STR	
	)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;

	int ret = 0, op_ret = 0;
	unsigned int port_index = 0;
	unsigned char slot_no = 0, port_no = 0;
	int local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);

	/*vty_out(vty,"before parse_vlan_no %s\n",argv[0]);*/
	ret = parse_slotport_no((char*)argv[0],&slot_no,&port_no);
    if (ret == NPD_FAIL)
	{
		vty_out(vty, "% Bad parameter,slot/port illegal!\n");
		return CMD_WARNING;
	}
	
	if (is_distributed == DISTRIBUTED_SYSTEM)
	{
		int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
		if(slot_no > slotNum || slot_no <= 0)
		{
			vty_out(vty,"%% NO SUCH SLOT %d!\n", slot_no);
            return CMD_WARNING;
		}
		
		query = dbus_message_new_method_call(
							SEM_DBUS_BUSNAME,\
							SEM_DBUS_OBJPATH,\
							SEM_DBUS_INTERFACE, 
							SEM_DBUS_CONFIG_ETHPORT
			                );
		
		dbus_error_init(&err);
	
		dbus_message_append_args(query,
								 DBUS_TYPE_BYTE,&slot_no,
								 DBUS_TYPE_BYTE,&port_no,
								 DBUS_TYPE_INVALID);
		
    	if(NULL == dbus_connection_dcli[slot_no]->dcli_dbus_connection) 				
    	{
    		if(slot_no == local_slot_id) 
    		{
    		    reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
    		}
    		else 
    		{	
    			vty_out(vty, "Connection to slot%d is not exist.\n", slot_no);
    			return CMD_WARNING;
    		}
    	}else {
    		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_no]->dcli_dbus_connection,\
    				query, -1, &err);
    	}

		dbus_message_unref(query);

		if (NULL == reply) {
			vty_out(vty,"failed get reply.\n");
			if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_SUCCESS;
		}

		if (dbus_message_get_args ( reply, &err,
						DBUS_TYPE_UINT32, &op_ret,
						DBUS_TYPE_UINT32, &port_index,
						DBUS_TYPE_INVALID)) 
		{
			if (SEM_WRONG_PARAM == op_ret) 
			{
		    	vty_out(vty,"Error:Illegal %d/%d,No such port.\n",slot_no,port_no);/*Stay here,not enter eth_config_node CMD.*/
				return CMD_FAILURE;
			}
			else if (SEM_COMMAND_FAILED== op_ret) 
			{
				vty_out(vty,"execute command failed.\n"); 
				return CMD_FAILURE;
			}
			else  if(SEM_COMMAND_SUCCESS == op_ret)   
			{
				DCLI_DEBUG(("port_index = %d\n", port_index));
				if(CONFIG_NODE == vty->node) {
					vty->node = ETH_PORT_NODE;
					vty->index = (void*)port_index;/*when not add & before port_index, the Vty enter <config-line> CMD Node.*/
				}
				else{
					vty_out (vty, "Terminal mode change must under configure mode!\n", VTY_NEWLINE);
					dbus_message_unref(reply);
					return CMD_WARNING;
				}
				dbus_message_unref(reply);
				return CMD_SUCCESS;
			}
			else if (SEM_COMMAND_NOT_SUPPORT == op_ret)
			{
				// TODO: call npd dbus to process the command.		
				DCLI_DEBUG(("call npd dbus process\n"));
				dbus_message_unref(reply);
			}
			else
			{
				vty_out(vty, "unkown return code:%d\n", op_ret);
				return CMD_FAILURE;
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
    		dbus_message_unref(reply);
    		return CMD_SUCCESS;
		}

    	query = dbus_message_new_method_call(
    							NPD_DBUS_BUSNAME,		\
    							NPD_DBUS_ETHPORTS_OBJPATH ,	\
    							NPD_DBUS_ETHPORTS_INTERFACE ,		\
    							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT);
    	
    	dbus_error_init(&err);
		
        DCLI_DEBUG(("index = %d, slot_no = %d, port_no = %d\n", port_index, slot_no, port_no));

    	dbus_message_append_args(query,
    							 DBUS_TYPE_BYTE,&slot_no,
    							 DBUS_TYPE_BYTE,&port_no,
    							 DBUS_TYPE_INVALID);
    	
    	if(NULL == dbus_connection_dcli[slot_no]->dcli_dbus_connection) 				
    	{
    		if(slot_no == local_slot_id)
    		{
    		    reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
    		}
    		else 
    		{	
    			vty_out(vty, "Connection to slot%d is not exist.\n", slot_no);
    			return CMD_WARNING;
    		}
    	}else {
    		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_no]->dcli_dbus_connection,\
    				query, -1, &err);
    	}
    	
    	dbus_message_unref(query);
    	
    	if (NULL == reply) {
    		vty_out(vty,"failed get reply.\n");
    		if (dbus_error_is_set(&err)) {
    			vty_out(vty,"%s raised: %s",err.name,err.message);
    			dbus_error_free_for_dcli(&err);
    		}
    		return CMD_SUCCESS;
    	}

    	if (dbus_message_get_args ( reply, &err,
    					DBUS_TYPE_UINT32, &op_ret,
    					DBUS_TYPE_UINT32, &port_index,
    					DBUS_TYPE_INVALID)) 
    	{
    		if (ETHPORT_RETURN_CODE_NO_SUCH_PORT == op_ret) 
    		{
            	vty_out(vty,"Error:Illegal %d/%d,No such port.\n",slot_no,port_no);/*Stay here,not enter eth_config_node CMD.*/
    		}
    		else if (ETHPORT_RETURN_CODE_ERR_GENERAL == op_ret) 
    		{
    			vty_out(vty,"execute command failed.\n"); 
    		}
    		else  if(ETHPORT_RETURN_CODE_ERR_NONE == op_ret)   
    		{
    			DCLI_DEBUG(("return port_index  %d \n",port_index));
    			if(CONFIG_NODE == vty->node) {
    				vty->node = ETH_PORT_NODE;
    				vty->index = (void*)port_index;/*when not add & before port_index, the Vty enter <config-line> CMD Node.*/
    			}
    			else{
    				vty_out (vty, "Terminal mode change must under configure mode!\n", VTY_NEWLINE);
    				dbus_message_unref(reply);
    				return CMD_WARNING;
    			}
    			dbus_message_unref(reply);
    			return CMD_SUCCESS;
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
        	dbus_message_unref(reply);
        	return CMD_SUCCESS;
    	}
    }
 
	if(8 == slot_no) {
		vty_out(vty,"%% Unsupported config eth-port attribute.\n");
		return CMD_WARNING;
	}
	
	if (NPD_FAIL == ret) {
    	vty_out(vty,"% Bad parameter,slot/port illegal!\n");
		return CMD_WARNING;
	}

	query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,		\
							NPD_DBUS_ETHPORTS_OBJPATH ,	\
							NPD_DBUS_ETHPORTS_INTERFACE ,		\
							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&slot_no,
							 DBUS_TYPE_BYTE,&port_no,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_UINT32, &port_index,
					DBUS_TYPE_INVALID)) 
	{
		if (ETHPORT_RETURN_CODE_NO_SUCH_PORT == op_ret) 
		{
        	vty_out(vty,"Error:Illegal %d/%d,No such port.\n",slot_no,port_no);/*Stay here,not enter eth_config_node CMD.*/
		}
		else if (ETHPORT_RETURN_CODE_ERR_GENERAL == op_ret ) 
		{
			vty_out(vty,"execute command failed.\n"); 
		}
		else  if(ETHPORT_RETURN_CODE_ERR_NONE == op_ret)   
		{
			DCLI_DEBUG(("return port_index  %d \n",port_index));
			if(CONFIG_NODE == vty->node) {
				vty->node = ETH_PORT_NODE;
				vty->index = (void*)port_index;/*when not add & before port_index, the Vty enter <config-line> CMD Node.*/
			}
			else{
				vty_out (vty, "Terminal mode change must under configure mode!\n", VTY_NEWLINE);
				dbus_message_unref(reply);
				return CMD_WARNING;
			}
			dbus_message_unref(reply);
			return CMD_SUCCESS;
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
    	dbus_message_unref(reply);
    	return CMD_SUCCESS;
	}
	
}

DEFUN(config_buffer_mode_cmd_func,
	  config_buffer_mode_cmd,
	  "config buffer mode (shared |divided)",
	  CONFIG_STR
	  "Configure buffer mode\n"
	  "Configure buffer mode enable or disable\n"
	  "Enable buffer mode \n"
	  "Disable buffer mode \n"	  
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	boolean isEnable = 0;
	unsigned int op_ret;
	if(strncmp("shared",argv[0],strlen(argv[0]))==0)
	{
		isEnable = 0;
	}
	else if (strncmp("divided",argv[0],strlen(argv[0]))==0)
	{
		isEnable = 1;
	}
	else
	{
		vty_out(vty,"bad command parameter!\n");
		return CMD_WARNING;
	}
		query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,		\
							NPD_DBUS_ETHPORTS_OBJPATH ,	\
							NPD_DBUS_ETHPORTS_INTERFACE ,		\
							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_BUFFER_MODE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&isEnable,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{
			if (NPD_DBUS_SUCCESS == op_ret ) 
			{
				/*vty_out(vty,"Access-list Service is %d\n",isEnable);*/
			}		
		
	} 
	else 
	{
		/*vty_out(vty,"Failed get args.\n");*/
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(config_ethport_admin_cmd_func,
	config_ethport_admin_cmd,
	"config admin state (enable|disable)",
	CONFIG_STR
	"Config eth-port admin information\n"
	"Config eth-port admin state information\n"
	"Specify eth-port admin enable \n"
	"Specify eth-port admin disable\n"
	)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;

	int ret = 0, op_ret = 0;
	unsigned int port_index = 0;
	unsigned int isEnable = 1;
	unsigned int type = ADMIN;
	unsigned int slot_no;
	int local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
	
	if(argc > 1){
		vty_out(vty,"input too many params\n ");
		return CMD_WARNING;
	}
	
	DCLI_DEBUG(("before parse value %s\n",argv[0]));
	if(strncmp(argv[0] , "enable",strlen((char*)argv[0])) == 0){
		isEnable = 1;
	}
	else if (strncmp(argv[0],"disable",strlen((char*)argv[0]))== 0){
		isEnable = 0;
	}
	else {
		vty_out(vty,"admin state config value must be enable or disable\n");
		return CMD_WARNING;
	}
	
	port_index = (unsigned int)vty->index;
	DCLI_DEBUG(("config port_index %d\n",port_index));
    SLOT_PORT_ANALYSIS_SLOT(port_index, slot_no);
	
	if (is_distributed == DISTRIBUTED_SYSTEM)
	{
		query = dbus_message_new_method_call(
							SEM_DBUS_BUSNAME,\
							SEM_DBUS_OBJPATH,\
							SEM_DBUS_INTERFACE, \
							SEM_DBUS_CONFIG_ETHPORT_ATTR \
			                );
		dbus_error_init(&err);

		dbus_message_append_args(query,
		 		  		    DBUS_TYPE_UINT32,&type,
							DBUS_TYPE_UINT32,&port_index,
							DBUS_TYPE_UINT32,&isEnable,
							DBUS_TYPE_INVALID);

    	if(NULL == dbus_connection_dcli[slot_no]->dcli_dbus_connection) 				
    	{
    		if(slot_no == local_slot_id) 
    		{
    		    reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
    		}
    		else 
    		{	
    			vty_out(vty, "Connection to slot%d is not exist.\n", slot_no);
    			return CMD_WARNING;
    		}
    	}else {
    		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_no]->dcli_dbus_connection,\
    				query, -1, &err);
    	}

		dbus_message_unref(query);

		
		if (NULL == reply) {
				vty_out(vty,"failed get reply.\n");
				if (dbus_error_is_set(&err)) {
					vty_out(vty,"%s raised: %s",err.name,err.message);
					dbus_error_free_for_dcli(&err);
				}
				return CMD_SUCCESS;
			}
		if (dbus_message_get_args ( reply, &err,
				DBUS_TYPE_UINT32, &op_ret,
				DBUS_TYPE_UINT32, &port_index,
				DBUS_TYPE_INVALID)) 
		{
			if (SEM_COMMAND_FAILED == op_ret ) 
			{
				vty_out(vty,"execute command failed.\n");
				dbus_message_unref(reply);
				return CMD_FAILURE;
			}
			else if(SEM_COMMAND_SUCCESS == op_ret)
			{
				DCLI_DEBUG(("port %d config admin succeed\n",port_index));
				dbus_message_unref(reply);
				return CMD_SUCCESS;
			}
			else if (SEM_WRONG_PARAM == op_ret ) 
			{
				vty_out(vty,"%% wrong parame\n"); 
				dbus_message_unref(reply);
				return CMD_WARNING;
			}
			else if (SEM_OPERATE_NOT_SUPPORT == op_ret) {
				
				vty_out(vty,"%% The eth-port don't support admin state setting\n");
				dbus_message_unref(reply);
				return CMD_WARNING;
			}
			else if (SEM_COMMAND_NOT_SUPPORT == op_ret) {
				// TODO:call npd dbus to process.
				DCLI_DEBUG(("%% call npd dbus to process\n"));
				dbus_message_unref(reply);		
			}
			else
			{
				vty_out(vty, "unkown return value\n");
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
			dbus_message_unref(reply);
			return CMD_WARNING;
		}
		
    	query = dbus_message_new_method_call(
    							NPD_DBUS_BUSNAME,		\
    							NPD_DBUS_ETHPORTS_OBJPATH ,	\
    							NPD_DBUS_ETHPORTS_INTERFACE ,		\
    							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_ATTR);
    	
    	dbus_error_init(&err);

    	dbus_message_append_args(query,
    							 DBUS_TYPE_UINT32,&type,
    							 DBUS_TYPE_UINT32,&port_index,
    							 DBUS_TYPE_UINT32,&isEnable,
    							 DBUS_TYPE_INVALID);
    	
    	if(NULL == dbus_connection_dcli[slot_no]->dcli_dbus_connection) 				
    	{
    		if(slot_no == local_slot_id) 
    		{
    		    reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
    		}
    		else 
    		{	
    			vty_out(vty, "Connection to slot%d is not exist.\n", slot_no);
    			return CMD_WARNING;
    		}
    	}else {
    		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_no]->dcli_dbus_connection,\
    				query, -1, &err);
    	}
    	
    	dbus_message_unref(query);
    	
    	if (NULL == reply) {
    		vty_out(vty,"failed get reply.\n");
    		if (dbus_error_is_set(&err)) {
    			vty_out(vty,"%s raised: %s",err.name,err.message);
    			dbus_error_free_for_dcli(&err);
    		}
    		return CMD_SUCCESS;
    	}

    	if (dbus_message_get_args ( reply, &err,
    					DBUS_TYPE_UINT32, &op_ret,
    					DBUS_TYPE_UINT32, &port_index,
    					DBUS_TYPE_INVALID)) 
    	{
    		if (ETHPORT_RETURN_CODE_ERR_GENERAL == op_ret ) 
    		{
    			vty_out(vty,"execute command failed.\n"); 
    		}
    		else if(ETHPORT_RETURN_CODE_ERR_NONE == op_ret)
    		{
    			DCLI_DEBUG(("port %d config admin succeed\n",port_index));
    		}
    		else if (ETHPORT_RETURN_CODE_ERR_HW == op_ret ) 
    		{
    			vty_out(vty,"%% Error on the hardware\n"); 
    		}
    		else if (ETHPORT_RETURN_CODE_UNSUPPORT == op_ret) {
    			vty_out(vty,"%% The eth-port don't support admin state setting\n");
    		}
    		else if (ETHPORT_RETURN_CODE_ERR_PRODUCT_TYPE == op_ret) {
    			vty_out(vty,"%% Error product type\n");
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
	
	query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,		\
							NPD_DBUS_ETHPORTS_OBJPATH ,	\
							NPD_DBUS_ETHPORTS_INTERFACE ,		\
							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_ATTR);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&type,
							 DBUS_TYPE_UINT32,&port_index,
							 DBUS_TYPE_UINT32,&isEnable,
							 DBUS_TYPE_INVALID);	

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_UINT32, &port_index,
					DBUS_TYPE_INVALID)) 
	{
		if (ETHPORT_RETURN_CODE_ERR_GENERAL == op_ret ) 
		{
			vty_out(vty,"execute command failed.\n"); 
		}
		else if(ETHPORT_RETURN_CODE_ERR_NONE == op_ret)
		{
			DCLI_DEBUG(("port %d config admin succeed\n",port_index));
		}
		else if (ETHPORT_RETURN_CODE_ERR_HW == op_ret ) 
		{
			vty_out(vty,"%% Error on the hardware\n"); 
		}
		else if (ETHPORT_RETURN_CODE_UNSUPPORT == op_ret) {
			vty_out(vty,"%% The eth-port don't support admin state setting\n");
		}
		else if (ETHPORT_RETURN_CODE_ERR_PRODUCT_TYPE == op_ret) {
			vty_out(vty,"%% Error product type\n");
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


DEFUN(config_ethport_ipg_func,
   config_ethport_ipg_cmd,
   "config minimum-ipg <0-15>",
   CONFIG_STR
   "Config ethernet port minimum inter-packet-gap\n"
   "Config inter-packet-gap value\n"
   )
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;

	unsigned int port_index = 0;
	unsigned short shport_ipg = 0;
	unsigned int ret = 0;
	unsigned char port_ipg = 0;
	int p_ret = 0;
	if(argc > 1){
		vty_out(vty,"input too many params\n ");
		return CMD_WARNING;
	}

	/*vty_out(vty,"before parse value %s\n",argv[0]);*/
	p_ret = parse_short_parse((char*)argv[0],&shport_ipg);
	if(NPD_SUCCESS != p_ret){
		vty_out(vty,"parse param error\n");
		return CMD_WARNING;
	}
 	port_ipg = (unsigned char )shport_ipg;
	if(port_ipg > 15) {
		vty_out(vty,"% Bad parameter.\n");
		return CMD_WARNING;
	}

   	port_index =(unsigned int ) vty->index;

    if (is_distributed == DISTRIBUTED_SYSTEM)
    {
		unsigned int slot_no;
        int local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
		
		SLOT_PORT_ANALYSIS_SLOT(port_index, slot_no);
		
    	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,		\
    										NPD_DBUS_ETHPORTS_OBJPATH ,	\
    										NPD_DBUS_ETHPORTS_INTERFACE ,		\
    										NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_IPG);

    	dbus_error_init(&err);

    	dbus_message_append_args(query,
    								DBUS_TYPE_UINT32, &port_index,
    								DBUS_TYPE_BYTE, &port_ipg,
    								DBUS_TYPE_INVALID);

    	if(NULL == dbus_connection_dcli[slot_no]->dcli_dbus_connection) 				
    	{
    		if(slot_no == local_slot_id) 
    		{
    		    reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
    		}
    		else 
    		{	
    			vty_out(vty, "Connection to slot%d is not exist.\n", slot_no);
    			return CMD_WARNING;
    		}
    	}else {
    		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_no]->dcli_dbus_connection,\
    				query, -1, &err);
    	}
		
    	if (NULL == reply) {
    		vty_out(vty,"failed get reply .\n");
    		if (dbus_error_is_set(&err)) {
    			vty_out(vty,"%s raised: %s",err.name,err.message);
    			dbus_error_free_for_dcli(&err);
    		}
    		return CMD_SUCCESS;
    	}

    	if (dbus_message_get_args ( reply, &err,
    								DBUS_TYPE_UINT32, &ret,
    								DBUS_TYPE_INVALID)) {
    		if (ETHPORT_RETURN_CODE_ERR_NONE == ret) 
    		{
				/* NULL */
     		}
    		else if(ETHPORT_RETURN_CODE_ERR_HW == ret){
    			vty_out(vty,"%% Port IPG configure failure on hardware.\n");
    		}
    		else if(ETHPORT_RETURN_CODE_NO_SUCH_PORT == ret)
    		{
    			vty_out(vty,"%% The eth-port is not asic port, check please!\n");
    		}
			else
			{
			    vty_out(vty,"%% Execute failed !\n"); 				
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
	else
	{

    	/*vty_out(vty,"%% get param: ret %d,port_ipg %d.\n",ret,port_ipg);*/
    	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,		\
    										NPD_DBUS_ETHPORTS_OBJPATH ,	\
    										NPD_DBUS_ETHPORTS_INTERFACE ,		\
    										NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_IPG);

    	dbus_error_init(&err);

    	dbus_message_append_args(query,
    								DBUS_TYPE_UINT32, &port_index,
    								DBUS_TYPE_BYTE, &port_ipg,
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

    	if (dbus_message_get_args ( reply, &err,
    								DBUS_TYPE_UINT32, &ret,
    								DBUS_TYPE_INVALID)) {
    		if (ETHPORT_RETURN_CODE_ERR_GENERAL == ret) 
    		{
    			vty_out(vty,"%% execute command failed.\n"); 
    		}
    		else if(ETHPORT_RETURN_CODE_ERR_HW == ret){
    			vty_out(vty,"%% Port IPG configure failure on hardware.\n");
    		}
    		else if (ETHPORT_RETURN_CODE_BAD_IPG == ret){
    			vty_out(vty,"%% Main board not hold out config ipg\n");
    		}
    		else if(ETHPORT_RETURN_CODE_UNSUPPORT == ret){
    			vty_out(vty,"%% The eth-port don't support ipg setting\n");
    		}
    		else if (ETHPORT_RETURN_CODE_ERR_PRODUCT_TYPE == ret) {
    			vty_out(vty,"%% Error product type\n");
    		}
    		/*else  if(NPD_DBUS_SUCCESS == ret)
    		{
    			DCLI_DEBUG(("port %d config ipg %d succeed\n",port_index,port_ipg));
    		}*/
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

DEFUN(config_ethport_speed_cmd_func,
	config_ethport_speed_cmd,
	"config speed (10|100|1000)",
	CONFIG_STR
	"Config eth-port speed information\n"
	"Specify eth-port speed 10M \n"
	"Specify eth-port speed 100M \n"
	"Specify eth-port speed 1000M \n"
	)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;

	int ret = 0, op_ret = 0;
	unsigned int port_index = 0;
	unsigned short param = 1000;
	unsigned int speed = 0, type = SPEED, slot_no;
    int local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
	
	if(argc > 1){
		vty_out(vty,"input too many params\n ");
		return CMD_WARNING;
	}
	
	DCLI_DEBUG(("before parse value %s\n",argv[0]));
	ret = parse_short_parse((char *)argv[0],&param);
	if(NPD_SUCCESS != ret){
		vty_out(vty,"parse param error\n");
		return CMD_WARNING;
	}
	speed = param;
	DCLI_DEBUG(("after parse param %d\n",speed));
	
	port_index = (unsigned int)vty->index;
	DCLI_DEBUG(("config port_index %d\n",port_index));
   
	
    if (is_distributed == DISTRIBUTED_SYSTEM)
    {
		query = dbus_message_new_method_call(
							SEM_DBUS_BUSNAME,\
							SEM_DBUS_OBJPATH,\
							SEM_DBUS_INTERFACE, \
							SEM_DBUS_CONFIG_ETHPORT_ATTR \
			                );
		dbus_error_init(&err);

		dbus_message_append_args(query,
		 		  		    DBUS_TYPE_UINT32,&type,
							DBUS_TYPE_UINT32,&port_index,
							DBUS_TYPE_UINT32,&speed,
							DBUS_TYPE_INVALID);
		
		SLOT_PORT_ANALYSIS_SLOT(port_index, slot_no);

    	if(NULL == dbus_connection_dcli[slot_no]->dcli_dbus_connection) 				
    	{
    		if(slot_no == local_slot_id) 
    		{
    		    reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
    		}
    		else 
    		{	
    			vty_out(vty, "Connection to slot%d is not exist.\n", slot_no);
    			return CMD_WARNING;
    		}
    	}else {
    		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_no]->dcli_dbus_connection,\
    				query, -1, &err);
    	}

		dbus_message_unref(query);
		
		if (NULL == reply) {
				vty_out(vty,"failed get reply.\n");
				if (dbus_error_is_set(&err)) {
					vty_out(vty,"%s raised: %s",err.name,err.message);
					dbus_error_free_for_dcli(&err);
				}
				return CMD_SUCCESS;
			}
		if (dbus_message_get_args ( reply, &err,
				DBUS_TYPE_UINT32, &op_ret,
				DBUS_TYPE_UINT32, &port_index,
				DBUS_TYPE_INVALID)) 
		{
			if (SEM_COMMAND_FAILED == op_ret) 
			{
				vty_out(vty,"execute command failed.\n");
				dbus_message_unref(reply);
				return CMD_FAILURE;
			}
			else if(SEM_COMMAND_SUCCESS == op_ret)
			{
				DCLI_DEBUG(("port %d config speed succeed\n",port_index));
				dbus_message_unref(reply);
				return CMD_SUCCESS;
			}
			else if (SEM_WRONG_PARAM == op_ret) 
			{
				vty_out(vty,"%% wrong parame\n"); 
				dbus_message_unref(reply);
				return CMD_WARNING;
			}
			else if (SEM_OPERATE_NOT_SUPPORT == op_ret) {
				
				vty_out(vty,"%% The eth-port don't support speed setting\n");
				dbus_message_unref(reply);
				return CMD_WARNING;
			}
			else if (SEM_COMMAND_NOT_SUPPORT == op_ret) {
				// TODO:call npd dbus to process.
				DCLI_DEBUG(("%% call npd dbus to process\n"));
				dbus_message_unref(reply);		
			}
			else
			{
				vty_out(vty, "unkown return value\n");
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
			dbus_message_unref(reply);
			return CMD_WARNING;
		}
		
    	query = dbus_message_new_method_call(
    							NPD_DBUS_BUSNAME,		\
    							NPD_DBUS_ETHPORTS_OBJPATH ,	\
    							NPD_DBUS_ETHPORTS_INTERFACE ,		\
    							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_ATTR);
    	
    	dbus_error_init(&err);

    	dbus_message_append_args(query,
    						   DBUS_TYPE_UINT32,&type,
    						   DBUS_TYPE_UINT32,&port_index,
    						   DBUS_TYPE_UINT32,&speed,
    						   DBUS_TYPE_INVALID);
    	
    	if(NULL == dbus_connection_dcli[slot_no]->dcli_dbus_connection) 				
    	{
    		if(slot_no == local_slot_id) 
    		{
    		    reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
    		}
    		else 
    		{	
    			vty_out(vty, "Connection to slot%d is not exist.\n", slot_no);
    			return CMD_WARNING;
    		}
    	}else {
    		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_no]->dcli_dbus_connection,\
    				query, -1, &err);
    	}
    	
    	
    	dbus_message_unref(query);
    	
    	if (NULL == reply) {
    		vty_out(vty,"failed get reply.\n");
    		if (dbus_error_is_set(&err)) {
    			vty_out(vty,"%s raised: %s",err.name,err.message);
    			dbus_error_free_for_dcli(&err);
    		}
    		return CMD_SUCCESS;
    	}

    	if (dbus_message_get_args ( reply, &err,
    					DBUS_TYPE_UINT32, &op_ret,
    					DBUS_TYPE_UINT32, &port_index,
    					DBUS_TYPE_INVALID)) 
    	{
    		if (ETHPORT_RETURN_CODE_ERR_GENERAL == op_ret ) 
    		{
    			vty_out(vty,"%% Execute command failed.\n"); 
    		}
    		else if (ETHPORT_RETURN_CODE_NO_SUCH_PORT == op_ret){
    			vty_out(vty,"%% Bad parameter :Bad Slot/Port number.\n");
    		}		
    		else if (ETHPORT_RETURN_CODE_ERR_NONE == op_ret)
    		{
    			DCLI_DEBUG(("port %d config speed succeed\n",port_index));
    		}
    		else if (ETHPORT_RETURN_CODE_SPEED_NODE == op_ret) {
    			vty_out(vty,"%% Speed is configed only auto negotiation speed is disable.\n");
    		}
    		else if (ETHPORT_RETURN_CODE_UNSUPPORT == op_ret) {
    			vty_out(vty,"%% The eth-port don't support speed setting\n");
    		}
    		else if (ETHPORT_RETURN_CODE_DUPLEX_MODE == op_ret) {
    			vty_out(vty,"%% The eth-port wasn't supported config 1000 when duplex is half mode \n");
    		}		
    		else if (VLAN_RETURN_CODE_PORT_TRUNK_MBR == op_ret) {
    			vty_out(vty,"%% The eth-port wasn't supported config when the port belonged to a active trunk \n");
    		}		
    		else if (ETHPORT_RETURN_CODE_ETH_GE_SFP == op_ret) {
    			vty_out(vty,"%% Fiber port not support this operation\n"); 
    		}
    		else if (ETHPORT_RETURN_CODE_ERR_HW == op_ret) {
    			vty_out(vty,"%% config speed error on hardware\n"); 
    		}		
    		else if (ETHPORT_RETURN_CODE_ERR_PRODUCT_TYPE == op_ret) {
    			vty_out(vty,"%% Error product type\n");
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

	query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,		\
							NPD_DBUS_ETHPORTS_OBJPATH ,	\
							NPD_DBUS_ETHPORTS_INTERFACE ,		\
							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_ATTR);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
						   DBUS_TYPE_UINT32,&type,
						   DBUS_TYPE_UINT32,&port_index,
						   DBUS_TYPE_UINT32,&speed,
						   DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_UINT32, &port_index,
					DBUS_TYPE_INVALID)) 
	{
		if (ETHPORT_RETURN_CODE_ERR_GENERAL == op_ret ) 
		{
			vty_out(vty,"%% Execute command failed.\n"); 
		}
		else if (ETHPORT_RETURN_CODE_NO_SUCH_PORT == op_ret){
			vty_out(vty,"%% Bad parameter :Bad Slot/Port number.\n");
		}		
		else if (ETHPORT_RETURN_CODE_ERR_NONE == op_ret)
		{
			DCLI_DEBUG(("port %d config speed succeed\n",port_index));
		}
		else if (ETHPORT_RETURN_CODE_SPEED_NODE == op_ret) {
			vty_out(vty,"%% Speed is configed only auto negotiation speed is disable.\n");
		}
		else if (ETHPORT_RETURN_CODE_UNSUPPORT == op_ret) {
			vty_out(vty,"%% The eth-port don't support speed setting\n");
		}
		else if (ETHPORT_RETURN_CODE_DUPLEX_MODE == op_ret) {
			vty_out(vty,"%% The eth-port wasn't supported config 1000 when duplex is half mode \n");
		}		
		else if (VLAN_RETURN_CODE_PORT_TRUNK_MBR == op_ret) {
			vty_out(vty,"%% The eth-port wasn't supported config when the port belonged to a active trunk \n");
		}		
		else if (ETHPORT_RETURN_CODE_ETH_GE_SFP == op_ret) {
			vty_out(vty,"%% Fiber port not support this operation\n"); 
		}
		else if (ETHPORT_RETURN_CODE_ERR_HW == op_ret) {
			vty_out(vty,"%% config speed error on hardware\n"); 
		}		
		else if (ETHPORT_RETURN_CODE_ERR_PRODUCT_TYPE == op_ret) {
			vty_out(vty,"%% Error product type\n");
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

DEFUN(config_ethport_autonegt_cmd_func,
	config_ethport_autonegt_cmd,
	"config auto-negotiation (enable|disable)",
	CONFIG_STR
	"Config eth-port auto-negotiation information\n"
	"Specify eth-port auto-negotiation enable \n"
	"Specify eth-port auto-negotiation disable\n"
	)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;

	int ret = 0, op_ret = 0;
	unsigned int port_index = 0;
	unsigned int isEnable = 1;
	unsigned int type = AUTONEGT;
	
	if(argc > 1){
		vty_out(vty,"input too many params\n ");
		return CMD_WARNING;
	}
	
	DCLI_DEBUG(("before parse value %s\n",argv[0]));
	if(strncmp(argv[0] , "enable",strlen((char*)argv[0])) == 0){
		isEnable = 1;
	}
	else if (strncmp(argv[0],"disable",strlen((char*)argv[0]))== 0){
		isEnable = 0;
	}
	else {
		vty_out(vty,"auto-negotiation config value must be enable or disable\n");
		return CMD_WARNING;
	}
	
	port_index = (unsigned int)vty->index;
	DCLI_DEBUG(("config port_index %d\n",port_index));
	
    if (is_distributed == DISTRIBUTED_SYSTEM)
    {
		unsigned int slot_no;
        int local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
		
		SLOT_PORT_ANALYSIS_SLOT(port_index, slot_no);
		
		query = dbus_message_new_method_call(
							SEM_DBUS_BUSNAME,\
							SEM_DBUS_OBJPATH,\
							SEM_DBUS_INTERFACE, \
							SEM_DBUS_CONFIG_ETHPORT_ATTR \
			                );
		dbus_error_init(&err);

		dbus_message_append_args(query,
		 		  		    DBUS_TYPE_UINT32,&type,
							DBUS_TYPE_UINT32,&port_index,
							DBUS_TYPE_UINT32,&isEnable,
							DBUS_TYPE_INVALID);

    	if(NULL == dbus_connection_dcli[slot_no]->dcli_dbus_connection) 				
    	{
    		if(slot_no == local_slot_id) 
    		{
    		    reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
    		}
    		else 
    		{	
    			vty_out(vty, "Connection to slot%d is not exist.\n", slot_no);
    			return CMD_WARNING;
    		}
    	}else {
    		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_no]->dcli_dbus_connection,\
    				query, -1, &err);
    	}
		
		dbus_message_unref(query);

		
		if (NULL == reply) {
			vty_out(vty,"failed get reply.\n");
			if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_SUCCESS;
		}
		if (dbus_message_get_args ( reply, &err,
				DBUS_TYPE_UINT32, &op_ret,
				DBUS_TYPE_UINT32, &port_index,
				DBUS_TYPE_INVALID)) 
		{
			if (SEM_COMMAND_FAILED == op_ret ) 
			{
				vty_out(vty,"execute command failed.\n");
				dbus_message_unref(reply);
				return CMD_FAILURE;
			}
			else if(SEM_COMMAND_SUCCESS == op_ret)
			{
				DCLI_DEBUG(("port %d config auto-negotiation succeed\n",port_index));
				dbus_message_unref(reply);
				return CMD_SUCCESS;
			}
			else if (SEM_WRONG_PARAM == op_ret ) 
			{
				vty_out(vty,"%% wrong parame\n"); 
				dbus_message_unref(reply);
				return CMD_WARNING;
			}
			else if (SEM_OPERATE_NOT_SUPPORT == op_ret) {
				
				vty_out(vty,"%% The eth-port don't support auto-negotiation setting\n");
				dbus_message_unref(reply);
				return CMD_WARNING;
			}
			else if (SEM_COMMAND_NOT_SUPPORT == op_ret) {
				// TODO:call npd dbus to process.
				DCLI_DEBUG(("%% call npd dbus to process\n"));
				dbus_message_unref(reply);		
			}
			else
			{
				vty_out(vty, "unkown return value\n");
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
			dbus_message_unref(reply);
			return CMD_WARNING;
		}       

    	query = dbus_message_new_method_call(
    							NPD_DBUS_BUSNAME,		\
    							NPD_DBUS_ETHPORTS_OBJPATH ,	\
    							NPD_DBUS_ETHPORTS_INTERFACE ,		\
    							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_ATTR);
    	
    	dbus_error_init(&err);

    	dbus_message_append_args(query,
    							 DBUS_TYPE_UINT32,&type,
    							 DBUS_TYPE_UINT32,&port_index,
    							 DBUS_TYPE_UINT32,&isEnable,
    							 DBUS_TYPE_INVALID);

    	if(NULL == dbus_connection_dcli[slot_no]->dcli_dbus_connection) 				
    	{
    		if(slot_no == local_slot_id)
    		{
    		    reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
    		}
    		else 
    		{	
    			vty_out(vty, "Connection to slot%d is not exist.\n", slot_no);
    			return CMD_WARNING;
    		}
    	}else {
    		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_no]->dcli_dbus_connection,\
    				query, -1, &err);
    	}
    	
    	
    	dbus_message_unref(query);
    	
    	if (NULL == reply) {
    		vty_out(vty,"failed get reply.\n");
    		if (dbus_error_is_set(&err)) {
    			vty_out(vty,"%s raised: %s",err.name,err.message);
    			dbus_error_free_for_dcli(&err);
    		}
    		return CMD_SUCCESS;
    	}

    	if (dbus_message_get_args ( reply, &err,
    					DBUS_TYPE_UINT32, &op_ret,
    					DBUS_TYPE_UINT32, &port_index,
    					DBUS_TYPE_INVALID)) 
    	{
    		if (ETHPORT_RETURN_CODE_ERR_GENERAL == op_ret ) 
    		{
    			vty_out(vty,"execute command failed.\n"); 
    		}
    		else if (ETHPORT_RETURN_CODE_ERR_NONE == op_ret)
    		{
    			DCLI_DEBUG(("port %d config auto-negotiation succeed\n",port_index));
    		}
    		else if (ETHPORT_RETURN_CODE_UNSUPPORT == op_ret) {
    			vty_out(vty,"%% The eth-port don't support auto-negotiation setting\n");
    		}
    		else if (ETHPORT_RETURN_CODE_ETH_GE_SFP == op_ret) {
    			vty_out(vty,"%% Fiber port not support this operation\n"); 
    		}
    		else if (ETHPORT_RETURN_CODE_ERR_HW == op_ret) {
    			vty_out(vty,"%% operation err on hardware\n"); 
    		}
            else if (ETHPORT_RETURN_CODE_ERR_OPERATE == op_ret) {
    			vty_out(vty,"%% Admin disable not support this operation\n"); 
    		}
    		else if (ETHPORT_RETURN_CODE_ERR_PRODUCT_TYPE == op_ret) {
    			vty_out(vty,"%% Error product type\n");
    		}
    		else {
    			vty_out(vty,"%% please config auto negotiation by configure guide.\n");
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

	query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,		\
							NPD_DBUS_ETHPORTS_OBJPATH ,	\
							NPD_DBUS_ETHPORTS_INTERFACE ,		\
							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_ATTR);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&type,
							 DBUS_TYPE_UINT32,&port_index,
							 DBUS_TYPE_UINT32,&isEnable,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_UINT32, &port_index,
					DBUS_TYPE_INVALID)) 
	{
		if (ETHPORT_RETURN_CODE_ERR_GENERAL == op_ret ) 
		{
			vty_out(vty,"execute command failed.\n"); 
		}
		else if (ETHPORT_RETURN_CODE_ERR_NONE == op_ret)
		{
			DCLI_DEBUG(("port %d config auto-negotiation succeed\n",port_index));
		}
		else if (ETHPORT_RETURN_CODE_UNSUPPORT == op_ret) {
			vty_out(vty,"%% The eth-port don't support auto-negotiation setting\n");
		}
		else if (ETHPORT_RETURN_CODE_ETH_GE_SFP == op_ret) {
			vty_out(vty,"%% Fiber port not support this operation\n"); 
		}
		else if (ETHPORT_RETURN_CODE_ERR_HW == op_ret) {
			vty_out(vty,"%% operation err on hardware\n"); 
		}
        else if (ETHPORT_RETURN_CODE_ERR_OPERATE == op_ret) {
			vty_out(vty,"%% Admin disable not support this operation\n"); 
		}
		else if (ETHPORT_RETURN_CODE_ERR_PRODUCT_TYPE == op_ret) {
			vty_out(vty,"%% Error product type\n");
		}
		else {
			vty_out(vty,"%% please config auto negotiation by configure guide.\n");
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

DEFUN(config_ethport_autonegt_speed_cmd_func,
	config_ethport_autonegt_speed_cmd,
	"config auto-negotiation speed (enable|disable)",
	CONFIG_STR
	"Config eth-port auto-negotiation information\n"
	"Config eth-port auto-negotiation speed information\n"
	"Specify eth-port auto-negotiation speed enable \n"
	"Specify eth-port auto-negotiation speed disable\n"
	)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;

	unsigned int port_index = 0;
	int ret = 0, op_ret = 0;
	unsigned int isEnable = 1;
	unsigned int type = AUTONEGTS;
	
	if(argc > 1){
		vty_out(vty,"input too many params\n ");
		return CMD_WARNING;
	}
	
	DCLI_DEBUG(("before parse value %s\n",argv[0]));
	if(strncmp(argv[0] , "enable",strlen((char*)argv[0])) == 0){
		isEnable = 1;
	}
	else if (strncmp(argv[0],"disable",strlen((char*)argv[0]))== 0){
		isEnable = 0;
	}
	else {
		vty_out(vty,"auto-negotiation speed config value must be enable or disable\n");
		return CMD_WARNING;
	}
	
	port_index = (unsigned int)vty->index;
	DCLI_DEBUG(("config port_index %d\n",port_index));

	if (is_distributed == DISTRIBUTED_SYSTEM)
	{
    	unsigned int slot_no;
        int local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);	

		SLOT_PORT_ANALYSIS_SLOT(port_index, slot_no);
		
		query = dbus_message_new_method_call(
							SEM_DBUS_BUSNAME,\
							SEM_DBUS_OBJPATH,\
							SEM_DBUS_INTERFACE, \
							SEM_DBUS_CONFIG_ETHPORT_ATTR \
			                );
		dbus_error_init(&err);

		dbus_message_append_args(query,
		 		  		    DBUS_TYPE_UINT32,&type,
							DBUS_TYPE_UINT32,&port_index,
							DBUS_TYPE_UINT32,&isEnable,
							DBUS_TYPE_INVALID);

    	if(NULL == dbus_connection_dcli[slot_no]->dcli_dbus_connection) 				
    	{
    		if(slot_no == local_slot_id) 
    		{
    		    reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
    		}
    		else 
    		{	
    			vty_out(vty, "Connection to slot%d is not exist.\n", slot_no);
    			return CMD_WARNING;
    		}
    	}else {
    		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_no]->dcli_dbus_connection,\
    				query, -1, &err);
    	}

		dbus_message_unref(query);

		
		if (NULL == reply) {
				vty_out(vty,"failed get reply.\n");
				if (dbus_error_is_set(&err)) {
					vty_out(vty,"%s raised: %s",err.name,err.message);
					dbus_error_free_for_dcli(&err);
				}
				return CMD_SUCCESS;
			}
		if (dbus_message_get_args ( reply, &err,
				DBUS_TYPE_UINT32, &op_ret,
				DBUS_TYPE_UINT32, &port_index,
				DBUS_TYPE_INVALID)) 
		{
			if (SEM_COMMAND_FAILED == op_ret ) 
			{
				vty_out(vty,"execute command failed.\n");
				dbus_message_unref(reply);
				return CMD_FAILURE;
			}
			else if(SEM_COMMAND_SUCCESS == op_ret)
			{
				DCLI_DEBUG(("port %d config auto-negotiation speed succeed\n",port_index));
				dbus_message_unref(reply);
				return CMD_SUCCESS;
			}
			else if (SEM_WRONG_PARAM == op_ret ) 
			{
				vty_out(vty,"%% wrong parame\n"); 
				dbus_message_unref(reply);
				return CMD_WARNING;
			}
			else if (SEM_OPERATE_NOT_SUPPORT == op_ret) {
				
				vty_out(vty,"%% The eth-port don't support auto-negotiation speed setting\n");
				dbus_message_unref(reply);
				return CMD_WARNING;
			}
			else if (SEM_COMMAND_NOT_SUPPORT == op_ret) {
				// TODO:call npd dbus to process.
				DCLI_DEBUG(("%% call npd dbus to process\n"));
				dbus_message_unref(reply);		
			}
			else
			{
				vty_out(vty, "unkown return value\n");
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
			dbus_message_unref(reply);
			return CMD_WARNING;
		}
    	query = dbus_message_new_method_call(
    							NPD_DBUS_BUSNAME,		\
    							NPD_DBUS_ETHPORTS_OBJPATH ,	\
    							NPD_DBUS_ETHPORTS_INTERFACE ,		\
    							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_ATTR);
    	
    	dbus_error_init(&err);

    	dbus_message_append_args(query,
    							DBUS_TYPE_UINT32,&type,
    							 DBUS_TYPE_UINT32,&port_index,
    							 DBUS_TYPE_UINT32,&isEnable,
    							 DBUS_TYPE_INVALID);
    	
    	if(NULL == dbus_connection_dcli[slot_no]->dcli_dbus_connection) 				
    	{
    		if(slot_no == local_slot_id) 
    		{
    		    reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
    		}
    		else 
    		{	
    			vty_out(vty, "Connection to slot%d is not exist.\n", slot_no);
    			return CMD_WARNING;
    		}
    	}else {
    		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_no]->dcli_dbus_connection,\
    				query, -1, &err);
    	}		
    	
    	dbus_message_unref(query);
    	
    	if (NULL == reply) {
    		vty_out(vty,"failed get reply.\n");
    		if (dbus_error_is_set(&err)) {
    			vty_out(vty,"%s raised: %s",err.name,err.message);
    			dbus_error_free_for_dcli(&err);
    		}
    		return CMD_SUCCESS;
    	}

    	if (dbus_message_get_args ( reply, &err,
    					DBUS_TYPE_UINT32, &op_ret,
    					DBUS_TYPE_UINT32, &port_index,
    					DBUS_TYPE_INVALID)) 
    	{
    		if (ETHPORT_RETURN_CODE_ERR_GENERAL == op_ret ) 
    		{
    			vty_out(vty,"execute command failed.\n"); 
    		}
    		else  if(ETHPORT_RETURN_CODE_ERR_NONE == op_ret)
    		{
    			DCLI_DEBUG(("port  %d config auto-negotiation speed succeed\n",port_index));
    		}
    		else if (ETHPORT_RETURN_CODE_UNSUPPORT == op_ret) {
    			vty_out(vty,"%% The eth-port don't support auto-negotiation speed setting\n");
    		}
    		else if (ETHPORT_RETURN_CODE_NOT_SUPPORT == op_ret) {
    			vty_out(vty,"%% The eth-port don't support auto-speed configuration\n");
    		}		
    		else if (ETHPORT_RETURN_CODE_ETH_GE_SFP == op_ret) {
    			vty_out(vty,"%% Fiber port not support this operation\n"); 
    		}
    		else if (ETHPORT_RETURN_CODE_ERR_HW == op_ret) {
    			vty_out(vty,"%% operation err on hardware\n"); 
    		}
            else if (ETHPORT_RETURN_CODE_ERR_OPERATE == op_ret) {
    			vty_out(vty,"%% Admin disable not support this operation\n"); 
    		}
    		else if (ETHPORT_RETURN_CODE_ERR_PRODUCT_TYPE == op_ret) {
    			vty_out(vty,"%% Error product type\n");
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
	
	query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,		\
							NPD_DBUS_ETHPORTS_OBJPATH ,	\
							NPD_DBUS_ETHPORTS_INTERFACE ,		\
							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_ATTR);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&type,
							 DBUS_TYPE_UINT32,&port_index,
							 DBUS_TYPE_UINT32,&isEnable,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_UINT32, &port_index,
					DBUS_TYPE_INVALID)) 
	{
		if (ETHPORT_RETURN_CODE_ERR_GENERAL == op_ret ) 
		{
			vty_out(vty,"execute command failed.\n"); 
		}
		else  if(ETHPORT_RETURN_CODE_ERR_NONE == op_ret)
		{
			DCLI_DEBUG(("port  %d config auto-negotiation speed succeed\n",port_index));
		}
		else if (ETHPORT_RETURN_CODE_UNSUPPORT == op_ret) {
			vty_out(vty,"%% The eth-port don't support auto-negotiation speed setting\n");
		}
		else if (ETHPORT_RETURN_CODE_NOT_SUPPORT == op_ret) {
			vty_out(vty,"%% The eth-port don't support auto-speed configuration\n");
		}		
		else if (ETHPORT_RETURN_CODE_ETH_GE_SFP == op_ret) {
			vty_out(vty,"%% Fiber port not support this operation\n"); 
		}
		else if (ETHPORT_RETURN_CODE_ERR_HW == op_ret) {
			vty_out(vty,"%% operation err on hardware\n"); 
		}
        else if (ETHPORT_RETURN_CODE_ERR_OPERATE == op_ret) {
			vty_out(vty,"%% Admin disable not support this operation\n"); 
		}
		else if (ETHPORT_RETURN_CODE_ERR_PRODUCT_TYPE == op_ret) {
			vty_out(vty,"%% Error product type\n");
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

DEFUN(config_ethport_autonegt_duplex_cmd_func,
	config_ethport_autonegt_duplex_cmd,
	"config auto-negotiation duplex (enable|disable)",
	CONFIG_STR
	"Config ethport auto-negotiation information\n"
	"Config ethport auto-negotiation duplex information\n"
	"Specify ethport auto-negotiation duplex enable \n"
	"Specify ethport auto-negotiation duplex disable\n"
	)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;

	int ret = 0, op_ret = 0;
	unsigned int port_index = 0;
	unsigned int isEnable = 1;
	unsigned int type = AUTONEGTD;
	
	if(argc > 1){
		vty_out(vty,"input too many params\n ");
		return CMD_WARNING;
	}
	
	DCLI_DEBUG(("before parse value %s\n",argv[0]));
	if(strncmp(argv[0] , "enable",strlen((char*)argv[0])) == 0){
		isEnable = 1;
	}
	else if (strncmp(argv[0],"disable",strlen((char*)argv[0]))== 0){
		isEnable = 0;
	}
	else {
		vty_out(vty,"auto-negotiation duplex config value must be enable or disable\n");
		return CMD_WARNING;
	}
	
	port_index = (unsigned int)vty->index;
	DCLI_DEBUG(("config port_index %d\n",port_index));
   

	if (is_distributed == DISTRIBUTED_SYSTEM)
	{
    	unsigned int slot_no;
        int local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
		SLOT_PORT_ANALYSIS_SLOT(port_index, slot_no);
		 
		query = dbus_message_new_method_call(
							SEM_DBUS_BUSNAME,\
							SEM_DBUS_OBJPATH,\
							SEM_DBUS_INTERFACE, \
							SEM_DBUS_CONFIG_ETHPORT_ATTR \
			                );
		dbus_error_init(&err);

		dbus_message_append_args(query,
		 		  		    DBUS_TYPE_UINT32,&type,
							DBUS_TYPE_UINT32,&port_index,
							DBUS_TYPE_UINT32,&isEnable,
							DBUS_TYPE_INVALID);

    	if(NULL == dbus_connection_dcli[slot_no]->dcli_dbus_connection) 				
    	{
    		if(slot_no == local_slot_id) 
    		{
    		    reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
    		}
    		else 
    		{	
    			vty_out(vty, "Connection to slot%d is not exist.\n", slot_no);
    			return CMD_WARNING;
    		}
    	}else {
    		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_no]->dcli_dbus_connection,\
    				query, -1, &err);
    	}
		
		dbus_message_unref(query);

		
		if (NULL == reply) {
				vty_out(vty,"failed get reply.\n");
				if (dbus_error_is_set(&err)) {
					vty_out(vty,"%s raised: %s",err.name,err.message);
					dbus_error_free_for_dcli(&err);
				}
				return CMD_SUCCESS;
			}
		if (dbus_message_get_args ( reply, &err,
				DBUS_TYPE_UINT32, &op_ret,
				DBUS_TYPE_UINT32, &port_index,
				DBUS_TYPE_INVALID)) 
		{
			if (SEM_COMMAND_FAILED == op_ret ) 
			{
				vty_out(vty,"execute command failed.\n");
				dbus_message_unref(reply);
				return CMD_FAILURE;
			}
			else if(SEM_COMMAND_SUCCESS == op_ret)
			{
				DCLI_DEBUG(("port %d config auto-negotiation duplex succeed\n",port_index));
				dbus_message_unref(reply);
				return CMD_SUCCESS;
			}
			else if (SEM_WRONG_PARAM == op_ret ) 
			{
				vty_out(vty,"%% wrong parame\n"); 
				dbus_message_unref(reply);
				return CMD_WARNING;
			}
			else if (SEM_OPERATE_NOT_SUPPORT == op_ret) {
				
				vty_out(vty,"%% The eth-port don't support auto-negotiation duplex setting\n");
				dbus_message_unref(reply);
				return CMD_WARNING;
			}
			else if (SEM_COMMAND_NOT_SUPPORT == op_ret) {
				// TODO:call npd dbus to process.
				DCLI_DEBUG(("%% call npd dbus to process\n"));
				dbus_message_unref(reply);		
			}
			else
			{
				vty_out(vty, "unkown return value\n");
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
			dbus_message_unref(reply);
			return CMD_WARNING;
		}

    	query = dbus_message_new_method_call(
    							NPD_DBUS_BUSNAME,		\
    							NPD_DBUS_ETHPORTS_OBJPATH ,	\
    							NPD_DBUS_ETHPORTS_INTERFACE ,	\
    							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_ATTR);
    	
    	dbus_error_init(&err);

    	dbus_message_append_args(query,
    							DBUS_TYPE_UINT32,&type,
    							 DBUS_TYPE_UINT32,&port_index,
    							 DBUS_TYPE_UINT32,&isEnable,
    							 DBUS_TYPE_INVALID);

    	if(NULL == dbus_connection_dcli[slot_no]->dcli_dbus_connection) 				
    	{
    		if(slot_no == local_slot_id)
    		{
    		    reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
    		}
    		else 
    		{	
    			vty_out(vty, "Connection to slot%d is not exist.\n", slot_no);
    			return CMD_WARNING;
    		}
    	}else {
    		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_no]->dcli_dbus_connection,\
    				query, -1, &err);
    	}		
    	
    	dbus_message_unref(query);
    	
    	if (NULL == reply) {
    		vty_out(vty,"failed get reply.\n");
    		if (dbus_error_is_set(&err)) {
    			vty_out(vty,"%s raised: %s",err.name,err.message);
    			dbus_error_free_for_dcli(&err);
    		}
    		return CMD_SUCCESS;
    	}

    	if (dbus_message_get_args ( reply, &err,
    					DBUS_TYPE_UINT32, &op_ret,
    					DBUS_TYPE_UINT32, &port_index,
    					DBUS_TYPE_INVALID)) 
    	{
    		if (ETHPORT_RETURN_CODE_ERR_GENERAL == op_ret ) 
    		{
    			vty_out(vty,"execute command failed.\n"); 
    		}
    		else  if(ETHPORT_RETURN_CODE_ERR_NONE == op_ret)
    		{
    			DCLI_DEBUG(("port %d config auto-negotiation duplex succeed\n",port_index));
    		}
    		else if (ETHPORT_RETURN_CODE_UNSUPPORT == op_ret) {
    			vty_out(vty,"%% The eth-port don't support auto-negotiation duplex setting\n");
    		}
    		else if (ETHPORT_RETURN_CODE_NOT_SUPPORT == op_ret) {
    			vty_out(vty,"%% The eth-port don't support auto-duplex configuration\n");
    		}		
    		else if (ETHPORT_RETURN_CODE_ETH_GE_SFP == op_ret) {
    			vty_out(vty,"%% Fiber port not support this operation\n"); 
    		}
    		else if (ETHPORT_RETURN_CODE_ERR_HW == op_ret) {
    			vty_out(vty,"%% operation err on hardware\n"); 
    		}
    		else if (ETHPORT_RETURN_CODE_ERR_OPERATE == op_ret) {
    			vty_out(vty,"%% Admin disable not support this operation\n"); 
    		}
    		else if (ETHPORT_RETURN_CODE_ERR_PRODUCT_TYPE == op_ret) {
    			vty_out(vty,"%% Error product type\n");
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
	
	query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,		\
							NPD_DBUS_ETHPORTS_OBJPATH ,	\
							NPD_DBUS_ETHPORTS_INTERFACE ,	\
							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_ATTR);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&type,
							 DBUS_TYPE_UINT32,&port_index,
							 DBUS_TYPE_UINT32,&isEnable,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_UINT32, &port_index,
					DBUS_TYPE_INVALID)) 
	{
		if (ETHPORT_RETURN_CODE_ERR_GENERAL == op_ret ) 
		{
			vty_out(vty,"execute command failed.\n"); 
		}
		else  if(ETHPORT_RETURN_CODE_ERR_NONE == op_ret)
		{
			DCLI_DEBUG(("port %d config auto-negotiation duplex succeed\n",port_index));
		}
		else if (ETHPORT_RETURN_CODE_UNSUPPORT == op_ret) {
			vty_out(vty,"%% The eth-port don't support auto-negotiation duplex setting\n");
		}
		else if (ETHPORT_RETURN_CODE_NOT_SUPPORT == op_ret) {
			vty_out(vty,"%% The eth-port don't support auto-duplex configuration\n");
		}		
		else if (ETHPORT_RETURN_CODE_ETH_GE_SFP == op_ret) {
			vty_out(vty,"%% Fiber port not support this operation\n"); 
		}
		else if (ETHPORT_RETURN_CODE_ERR_HW == op_ret) {
			vty_out(vty,"%% operation err on hardware\n"); 
		}
		else if (ETHPORT_RETURN_CODE_ERR_OPERATE == op_ret) {
			vty_out(vty,"%% Admin disable not support this operation\n"); 
		}
		else if (ETHPORT_RETURN_CODE_ERR_PRODUCT_TYPE == op_ret) {
			vty_out(vty,"%% Error product type\n");
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

DEFUN(config_ethport_autonegt_flowcontrol_cmd_func,
	config_ethport_autonegt_flowcontrol_cmd,
	"config auto-negotiation flow-control (enable|disable)",
	CONFIG_STR
	"Config eth-port auto-negotiation information\n"
	"Config eth-port auto-negotiation flow control information\n"
	"Specify eth-port auto-negotiation flow control enable \n"
	"Specify eth-port auto-negotiation flow control disable\n"
	)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;

	int ret = 0, op_ret = 0;
	unsigned int port_index = 0;
	unsigned int isEnable = 1;
	unsigned int type = AUTONEGTF;
	
	if(argc > 1){
		vty_out(vty,"input too many params\n ");
		return CMD_WARNING;
	}
	
	DCLI_DEBUG(("before parse_vlan_no %s\n",argv[0]));
	if(strncmp(argv[0] , "enable",strlen((char*)argv[0])) == 0){
		isEnable = 1;
	}
	else if (strncmp(argv[0],"disable",strlen((char*)argv[0]))== 0){
		isEnable = 0;
	}
	else {
		vty_out(vty,"auto-negotiation flow control config value must be enable or disable\n");
		return CMD_WARNING;
	}
	
	port_index = (unsigned int)vty->index;
	DCLI_DEBUG(("config port_index %d\n",port_index));

	if (is_distributed == DISTRIBUTED_SYSTEM)
	{
    	unsigned int slot_no;
        int local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
		SLOT_PORT_ANALYSIS_SLOT(port_index, slot_no);
		
		query = dbus_message_new_method_call(
							SEM_DBUS_BUSNAME,\
							SEM_DBUS_OBJPATH,\
							SEM_DBUS_INTERFACE, \
							SEM_DBUS_CONFIG_ETHPORT_ATTR \
			                );
		dbus_error_init(&err);

		dbus_message_append_args(query,
		 		  		    DBUS_TYPE_UINT32,&type,
							DBUS_TYPE_UINT32,&port_index,
							DBUS_TYPE_UINT32,&isEnable,
							DBUS_TYPE_INVALID);

    	if(NULL == dbus_connection_dcli[slot_no]->dcli_dbus_connection) 				
    	{
    		if(slot_no == local_slot_id) 
    		{
    		    reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
    		}
    		else 
    		{	
    			vty_out(vty, "Connection to slot%d is not exist.\n", slot_no);
    			return CMD_WARNING;
    		}
    	}else {
    		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_no]->dcli_dbus_connection,\
    				query, -1, &err);
    	}

		dbus_message_unref(query);

		
		if (NULL == reply) {
				vty_out(vty,"failed get reply.\n");
				if (dbus_error_is_set(&err)) {
					vty_out(vty,"%s raised: %s",err.name,err.message);
					dbus_error_free_for_dcli(&err);
				}
				return CMD_SUCCESS;
			}
		if (dbus_message_get_args ( reply, &err,
				DBUS_TYPE_UINT32, &op_ret,
				DBUS_TYPE_UINT32, &port_index,
				DBUS_TYPE_INVALID)) 
		{
			if (SEM_COMMAND_FAILED == op_ret ) 
			{
				vty_out(vty,"execute command failed.\n");
				dbus_message_unref(reply);
				return CMD_FAILURE;
			}
			else if(SEM_COMMAND_SUCCESS == op_ret)
			{
				DCLI_DEBUG(("port %d config auto-negotiation flow control succeed\n",port_index));
				dbus_message_unref(reply);
				return CMD_SUCCESS;
			}
			else if (SEM_WRONG_PARAM == op_ret ) 
			{
				vty_out(vty,"%% wrong parame\n"); 
				dbus_message_unref(reply);
				return CMD_WARNING;
			}
			else if (SEM_OPERATE_NOT_SUPPORT == op_ret) {
				
				vty_out(vty,"%% The eth-port don't support auto-negotiation flow control setting\n");
				dbus_message_unref(reply);
				return CMD_WARNING;
			}
			else if (SEM_COMMAND_NOT_SUPPORT == op_ret) {
				// TODO:call npd dbus to process.
				DCLI_DEBUG(("%% call npd dbus to process\n"));
				dbus_message_unref(reply);		
			}
			else
			{
				vty_out(vty, "unkown return value\n");
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
			dbus_message_unref(reply);
			return CMD_WARNING;
		}
    	query = dbus_message_new_method_call(
    							NPD_DBUS_BUSNAME,		\
    							NPD_DBUS_ETHPORTS_OBJPATH ,	\
    							NPD_DBUS_ETHPORTS_INTERFACE ,		\
    							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_ATTR);
    	
    	dbus_error_init(&err);

    	dbus_message_append_args(query,
    							DBUS_TYPE_UINT32,&type,
    							 DBUS_TYPE_UINT32,&port_index,
    							 DBUS_TYPE_UINT32,&isEnable,
    							 DBUS_TYPE_INVALID);

    	if(NULL == dbus_connection_dcli[slot_no]->dcli_dbus_connection) 				
    	{
    		if(slot_no == local_slot_id) 
    		{
    		    reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
    		}
    		else 
    		{	
    			vty_out(vty, "Connection to slot%d is not exist.\n", slot_no);
    			return CMD_WARNING;
    		}
    	}else {
    		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_no]->dcli_dbus_connection,\
    				query, -1, &err);
    	}		
    		
    	dbus_message_unref(query);
    	
    	if (NULL == reply) {
    		vty_out(vty,"failed get reply.\n");
    		if (dbus_error_is_set(&err)) {
    			vty_out(vty,"%s raised: %s",err.name,err.message);
    			dbus_error_free_for_dcli(&err);
    		}
    		return CMD_SUCCESS;
    	}

    	if (dbus_message_get_args ( reply, &err,
    					DBUS_TYPE_UINT32, &op_ret,
    					DBUS_TYPE_UINT32, &port_index,
    					DBUS_TYPE_INVALID)) 
    	{
    		if (ETHPORT_RETURN_CODE_ERR_GENERAL == op_ret ) 
    		{
    			vty_out(vty,"execute command failed.\n"); 
    		}
    		else  if(ETHPORT_RETURN_CODE_ERR_NONE == op_ret)
    		{
    			DCLI_DEBUG(("port  %d config auto-negotiation flow-control succeed\n",port_index));
    		}
    		else if (ETHPORT_RETURN_CODE_UNSUPPORT == op_ret) {
    			vty_out(vty,"%% The eth-port don't support auto-negotiation flow-control setting\n");
    		}
    		else if (ETHPORT_RETURN_CODE_NOT_SUPPORT == op_ret) {
    			vty_out(vty,"%% The eth-port don't support auto-flowControl configuration\n");
    		}		
    		else if (ETHPORT_RETURN_CODE_ETH_GE_SFP == op_ret) {
    			vty_out(vty,"%% Fiber port not support this operation\n"); 
    		}
    		else if (ETHPORT_RETURN_CODE_ERR_HW == op_ret) {
    			vty_out(vty,"%% operation err on hardware\n"); 
    		}		
    		else if (ETHPORT_RETURN_CODE_ERR_OPERATE == op_ret) {
    			vty_out(vty,"%% Admin disable not support this operation\n"); 
    		}
    		else if (ETHPORT_RETURN_CODE_ERR_PRODUCT_TYPE == op_ret) {
    			vty_out(vty,"%% Error product type\n");
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
	
	query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,		\
							NPD_DBUS_ETHPORTS_OBJPATH ,	\
							NPD_DBUS_ETHPORTS_INTERFACE ,		\
							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_ATTR);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&type,
							 DBUS_TYPE_UINT32,&port_index,
							 DBUS_TYPE_UINT32,&isEnable,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
		
	dbus_message_unref(query);
	
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_UINT32, &port_index,
					DBUS_TYPE_INVALID)) 
	{
		if (ETHPORT_RETURN_CODE_ERR_GENERAL == op_ret ) 
		{
			vty_out(vty,"execute command failed.\n"); 
		}
		else  if(ETHPORT_RETURN_CODE_ERR_NONE == op_ret)
		{
			DCLI_DEBUG(("port  %d config auto-negotiation flow-control succeed\n",port_index));
		}
		else if (ETHPORT_RETURN_CODE_UNSUPPORT == op_ret) {
			vty_out(vty,"%% The eth-port don't support auto-negotiation flow-control setting\n");
		}
		else if (ETHPORT_RETURN_CODE_NOT_SUPPORT == op_ret) {
			vty_out(vty,"%% The eth-port don't support auto-flowControl configuration\n");
		}		
		else if (ETHPORT_RETURN_CODE_ETH_GE_SFP == op_ret) {
			vty_out(vty,"%% Fiber port not support this operation\n"); 
		}
		else if (ETHPORT_RETURN_CODE_ERR_HW == op_ret) {
			vty_out(vty,"%% operation err on hardware\n"); 
		}		
		else if (ETHPORT_RETURN_CODE_ERR_OPERATE == op_ret) {
			vty_out(vty,"%% Admin disable not support this operation\n"); 
		}
		else if (ETHPORT_RETURN_CODE_ERR_PRODUCT_TYPE == op_ret) {
			vty_out(vty,"%% Error product type\n");
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

DEFUN(config_ethport_duplext_mode_cmd_func,
	config_ethport_duplex_mode_cmd,
	"config duplex mode (half|full)",
	CONFIG_STR
	"Config eth-port duplex information\n"
	"Config eth-port duplex mode information\n"
	"Specify eth-port duplex mode half\n"
	"Specify eth-port duplex mode full\n"
	)
{
	DBusMessage *query = NULL , *reply = NULL;
	DBusError err;

	int op_ret = 0;
	unsigned int port_index = 0;
	unsigned int isEnable = 1;
	unsigned int type = DUMOD;
	
	if(argc > 1){
		vty_out(vty,"input too many params\n ");
		return CMD_WARNING;
	}
	
	DCLI_DEBUG(("before parse value %s\n",argv[0]));
	if(strncmp(argv[0] , "full",strlen((char*)argv[0])) == 0){
		isEnable = 1;
	}
	else if (strncmp(argv[0],"half",strlen((char*)argv[0]))== 0){
		isEnable = 0;
	}
	else {
		vty_out(vty,"dulex mode config value must be half or full\n");
		return CMD_WARNING;
	}
	
	port_index = (unsigned int)vty->index;
	DCLI_DEBUG(("config port_index %d\n",port_index));
    	
    if (is_distributed == DISTRIBUTED_SYSTEM)
    {
		unsigned int slot_no;
	    int local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
		SLOT_PORT_ANALYSIS_SLOT(port_index, slot_no);
		
		query = dbus_message_new_method_call(
							SEM_DBUS_BUSNAME,\
							SEM_DBUS_OBJPATH,\
							SEM_DBUS_INTERFACE, \
							SEM_DBUS_CONFIG_ETHPORT_ATTR \
			                );
		dbus_error_init(&err);

		dbus_message_append_args(query,
		 		  		    DBUS_TYPE_UINT32,&type,
							DBUS_TYPE_UINT32,&port_index,
							DBUS_TYPE_UINT32,&isEnable,
							DBUS_TYPE_INVALID);

    	if(NULL == dbus_connection_dcli[slot_no]->dcli_dbus_connection) 				
    	{
    		if(slot_no == local_slot_id) 
    		{
    		    reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
    		}
    		else 
    		{	
    			vty_out(vty, "Connection to slot%d is not exist.\n", slot_no);
    			return CMD_WARNING;
    		}
    	}else {
    		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_no]->dcli_dbus_connection,\
    				query, -1, &err);
    	}

		dbus_message_unref(query);

		
		if (NULL == reply) {
				vty_out(vty,"failed get reply.\n");
				if (dbus_error_is_set(&err)) {
					vty_out(vty,"%s raised: %s",err.name,err.message);
					dbus_error_free_for_dcli(&err);
				}
				return CMD_SUCCESS;
			}
		if (dbus_message_get_args ( reply, &err,
				DBUS_TYPE_UINT32, &op_ret,
				DBUS_TYPE_UINT32, &port_index,
				DBUS_TYPE_INVALID)) 
		{
			if (SEM_COMMAND_FAILED == op_ret ) 
			{
				vty_out(vty,"execute command failed.\n");
				dbus_message_unref(reply);
				return CMD_FAILURE;
			}
			else if(SEM_COMMAND_SUCCESS == op_ret)
			{
				DCLI_DEBUG(("port %d config duplex mode succeed\n",port_index));
				dbus_message_unref(reply);
				return CMD_SUCCESS;
			}
			else if (SEM_WRONG_PARAM == op_ret ) 
			{
				vty_out(vty,"%% wrong parame\n"); 
				dbus_message_unref(reply);
				return CMD_WARNING;
			}
			else if (SEM_OPERATE_NOT_SUPPORT == op_ret) {
				
				vty_out(vty,"%% The eth-port don't support duplex mode setting\n");
				dbus_message_unref(reply);
				return CMD_WARNING;
			}
			else if (SEM_COMMAND_NOT_SUPPORT == op_ret) {
				// TODO:call npd dbus to process.
				DCLI_DEBUG(("%% call npd dbus to process\n"));
				dbus_message_unref(reply);		
			}
			else
			{
				vty_out(vty, "unkown return value\n");
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
			dbus_message_unref(reply);
			return CMD_WARNING;
		} 
    	query = dbus_message_new_method_call(
    							NPD_DBUS_BUSNAME,		\
    							NPD_DBUS_ETHPORTS_OBJPATH ,	\
    							NPD_DBUS_ETHPORTS_INTERFACE ,		\
    							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_ATTR);
    	
    	dbus_error_init(&err);

    	dbus_message_append_args(query,
    							DBUS_TYPE_UINT32,&type,
    							 DBUS_TYPE_UINT32,&port_index,
    							 DBUS_TYPE_UINT32,&isEnable,
    							 DBUS_TYPE_INVALID);
    	
    	if(NULL == dbus_connection_dcli[slot_no]->dcli_dbus_connection) 				
    	{
    		if(slot_no == local_slot_id)
    		{
    		    reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
    		}
    		else 
    		{	
    			vty_out(vty, "Connection to slot%d is not exist.\n", slot_no);
    			return CMD_WARNING;
    		}
    	}else {
    		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_no]->dcli_dbus_connection,\
    				query, -1, &err);
    	}
    	
    	
    	dbus_message_unref(query);
    	
    	if (NULL == reply) {
    		vty_out(vty,"failed get reply.\n");
    		if (dbus_error_is_set(&err)) {
    			vty_out(vty,"%s raised: %s",err.name,err.message);
    			dbus_error_free_for_dcli(&err);
    		}
    		return CMD_SUCCESS;
    	}

    	if (dbus_message_get_args ( reply, &err,
    					DBUS_TYPE_UINT32, &op_ret,
    					DBUS_TYPE_UINT32, &port_index,
    					DBUS_TYPE_INVALID)) 
    	{
    		if (ETHPORT_RETURN_CODE_ERR_GENERAL == op_ret ) 
    		{
    			vty_out(vty,"execute command failed.\n"); 
    		}
    		else if(ETHPORT_RETURN_CODE_ERR_NONE == op_ret)
    		{
    			DCLI_DEBUG(("port  %d config duplex mode succeed\n",port_index));
    		}
    		else if(ETHPORT_RETURN_CODE_DUPLEX_NODE  == op_ret) {
    			vty_out(vty,"%% duplex is configed only auto negotiation duplex is disable and speed isn't 1000M.\n");
    		}
    		else if(ETHPORT_RETURN_CODE_ERROR_DUPLEX_HALF == op_ret) {
    			vty_out(vty,"%% can not config duplex mode half when flow control is enable.\n");
    		}
    		else if(ETHPORT_RETURN_CODE_ERROR_DUPLEX_FULL == op_ret) {
    			vty_out(vty,"%% can not config duplex mode full when back pressure is enable.\n");
    		}
    		else if (ETHPORT_RETURN_CODE_UNSUPPORT == op_ret) {
    			vty_out(vty,"%% The eth-port don't support duplex setting\n");
    		}
    		else if (ETHPORT_RETURN_CODE_ETH_GE_SFP == op_ret) {
    			vty_out(vty,"%% Fiber port not support this operation\n"); 
    		}
    		else if (ETHPORT_RETURN_CODE_ERR_HW == op_ret) {
    			vty_out(vty,"%% operation err on hardware\n"); 
    		}
    		else if (ETHPORT_RETURN_CODE_ERR_PRODUCT_TYPE == op_ret) {
    			vty_out(vty,"%% Error product type\n");
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
	query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,		\
							NPD_DBUS_ETHPORTS_OBJPATH ,	\
							NPD_DBUS_ETHPORTS_INTERFACE ,		\
							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_ATTR);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&type,
							 DBUS_TYPE_UINT32,&port_index,
							 DBUS_TYPE_UINT32,&isEnable,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_UINT32, &port_index,
					DBUS_TYPE_INVALID)) 
	{
		if (ETHPORT_RETURN_CODE_ERR_GENERAL == op_ret ) 
		{
			vty_out(vty,"execute command failed.\n"); 
		}
		else if(ETHPORT_RETURN_CODE_ERR_NONE == op_ret)
		{
			DCLI_DEBUG(("port  %d config duplex mode succeed\n",port_index));
		}
		else if(ETHPORT_RETURN_CODE_DUPLEX_NODE  == op_ret) {
			vty_out(vty,"%% duplex is configed only auto negotiation duplex is disable and speed isn't 1000M.\n");
		}
		else if(ETHPORT_RETURN_CODE_ERROR_DUPLEX_HALF == op_ret) {
			vty_out(vty,"%% can not config duplex mode half when flow control is enable.\n");
		}
		else if(ETHPORT_RETURN_CODE_ERROR_DUPLEX_FULL == op_ret) {
			vty_out(vty,"%% can not config duplex mode full when back pressure is enable.\n");
		}
		else if (ETHPORT_RETURN_CODE_UNSUPPORT == op_ret) {
			vty_out(vty,"%% The eth-port don't support duplex setting\n");
		}
		else if (ETHPORT_RETURN_CODE_ETH_GE_SFP == op_ret) {
			vty_out(vty,"%% Fiber port not support this operation\n"); 
		}
		else if (ETHPORT_RETURN_CODE_ERR_HW == op_ret) {
			vty_out(vty,"%% operation err on hardware\n"); 
		}
		else if (ETHPORT_RETURN_CODE_ERR_PRODUCT_TYPE == op_ret) {
			vty_out(vty,"%% Error product type\n");
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

DEFUN(config_ethport_flow_control_cmd_func,
	config_ethport_flow_control_cmd,
	"config flow-control (enable|disable)",
	CONFIG_STR
	"Config eth-port flow-control information\n"
	"Specify eth-port flow-control enable \n"
	"Specify eth-port flow-control disable\n"
	)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;

	int ret = 0, op_ret = 0;
	unsigned int port_index = 0;
	unsigned int isEnable = 0;
	unsigned int type = FLOWCTRL;
	
	if(argc > 1){
		vty_out(vty,"input too many params\n ");
		return CMD_WARNING;
	}
	
	DCLI_DEBUG(("before parse value %s\n",argv[0]));
	if(strncmp(argv[0] , "enable",strlen((char*)argv[0])) == 0){
		isEnable = 1;
	}
	else if (strncmp(argv[0],"disable",strlen((char*)argv[0]))== 0){
		isEnable = 0;
	}
	else {
		vty_out(vty,"flow-control config value must be enable or disable\n");
		return CMD_WARNING;
	}
	
	port_index = (unsigned int)vty->index;
	DCLI_DEBUG(("config port_index %d\n",port_index));  

	if (is_distributed == DISTRIBUTED_SYSTEM)
	{
		unsigned int slot_no;
        int local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
	    SLOT_PORT_ANALYSIS_SLOT(port_index, slot_no);
		
		query = dbus_message_new_method_call(
							SEM_DBUS_BUSNAME,\
							SEM_DBUS_OBJPATH,\
							SEM_DBUS_INTERFACE, \
							SEM_DBUS_CONFIG_ETHPORT_ATTR \
			                );
		dbus_error_init(&err);

		dbus_message_append_args(query,
		 		  		    DBUS_TYPE_UINT32,&type,
							DBUS_TYPE_UINT32,&port_index,
							DBUS_TYPE_UINT32,&isEnable,
							DBUS_TYPE_INVALID);

    	if(NULL == dbus_connection_dcli[slot_no]->dcli_dbus_connection) 				
    	{
    		if(slot_no == local_slot_id) 
    		{
    		    reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
    		}
    		else 
    		{	
    			vty_out(vty, "Connection to slot%d is not exist.\n", slot_no);
    			return CMD_WARNING;
    		}
    	}else {
    		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_no]->dcli_dbus_connection,\
    				query, -1, &err);
    	}

		dbus_message_unref(query);

		
		if (NULL == reply) {
				vty_out(vty,"failed get reply.\n");
				if (dbus_error_is_set(&err)) {
					vty_out(vty,"%s raised: %s",err.name,err.message);
					dbus_error_free_for_dcli(&err);
				}
				return CMD_SUCCESS;
			}
		if (dbus_message_get_args ( reply, &err,
				DBUS_TYPE_UINT32, &op_ret,
				DBUS_TYPE_UINT32, &port_index,
				DBUS_TYPE_INVALID)) 
		{
			if (SEM_COMMAND_FAILED == op_ret ) 
			{
				vty_out(vty,"execute command failed.\n");
				dbus_message_unref(reply);
				return CMD_FAILURE;
			}
			else if(SEM_COMMAND_SUCCESS == op_ret)
			{
				DCLI_DEBUG(("port %d config flow control succeed\n",port_index));
				dbus_message_unref(reply);
				return CMD_SUCCESS;
			}
			else if (SEM_WRONG_PARAM == op_ret ) 
			{
				vty_out(vty,"%% wrong parame\n"); 
				dbus_message_unref(reply);
				return CMD_WARNING;
			}
			else if (SEM_OPERATE_NOT_SUPPORT == op_ret) {
				
				vty_out(vty,"%% The eth-port don't support flow control setting\n");
				dbus_message_unref(reply);
				return CMD_WARNING;
			}
			else if (SEM_COMMAND_NOT_SUPPORT == op_ret) {
				// TODO:call npd dbus to process.
				DCLI_DEBUG(("%% call npd dbus to process\n"));
				dbus_message_unref(reply);		
			}
			else
			{
				vty_out(vty, "unkown return value\n");
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
			dbus_message_unref(reply);
			return CMD_WARNING;
		}	
    	query = dbus_message_new_method_call(
    							NPD_DBUS_BUSNAME,		\
    							NPD_DBUS_ETHPORTS_OBJPATH ,	\
    							NPD_DBUS_ETHPORTS_INTERFACE ,		\
    							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_ATTR);
    	
    	dbus_error_init(&err);

    	dbus_message_append_args(query,
    							DBUS_TYPE_UINT32,&type,
    							 DBUS_TYPE_UINT32,&port_index,
    							 DBUS_TYPE_UINT32,&isEnable,
    							 DBUS_TYPE_INVALID);
    	
    	if(NULL == dbus_connection_dcli[slot_no]->dcli_dbus_connection) 				
    	{
    		if(slot_no == local_slot_id)
    		{
    		    reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
    		}
    		else 
    		{	
    			vty_out(vty, "Connection to slot%d is not exist.\n", slot_no);
    			return CMD_WARNING;
    		}
    	}else {
    		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_no]->dcli_dbus_connection,\
    				query, -1, &err);
    	}
        	 
    	dbus_message_unref(query);
    	
    	if (NULL == reply) {
    		vty_out(vty,"failed get reply.\n");
    		if (dbus_error_is_set(&err)) {
    			vty_out(vty,"%s raised: %s",err.name,err.message);
    			dbus_error_free_for_dcli(&err);
    		}
    		return CMD_SUCCESS;
    	}

    	if (dbus_message_get_args ( reply, &err,
    					DBUS_TYPE_UINT32, &op_ret,
    					DBUS_TYPE_UINT32, &port_index,
    					DBUS_TYPE_INVALID)) 
    	{
    		if (ETHPORT_RETURN_CODE_ERR_GENERAL == op_ret ) 
    		{
    			vty_out(vty,"execute command failed.\n"); 
    		}
    		else if(ETHPORT_RETURN_CODE_ERR_NONE == op_ret)
    		{
    			DCLI_DEBUG(("port %d config flow-control succeed\n",port_index));
    		}
    		else if(ETHPORT_RETURN_CODE_NO_SUCH_PORT == op_ret)
    		{
    			DCLI_DEBUG(("%% port is not exist\n",port_index));
    		}
    		else if(ETHPORT_RETURN_CODE_FLOWCTL_NODE == op_ret) {
    			vty_out(vty,"%% Flow control is configed only duplex is full and auto-negotiation flow-control is disable.\n");
    		}
    		else if (ETHPORT_RETURN_CODE_UNSUPPORT == op_ret) {
    			vty_out(vty,"%% The eth-port don't support flow-control setting\n");
    		}
    		else if (VLAN_RETURN_CODE_PORT_TRUNK_MBR == op_ret) {
    			vty_out(vty,"%% The eth-port wasn't supported config when the port belonged to a active trunk \n");
    		}		
    		else if (ETHPORT_RETURN_CODE_NOT_SUPPORT == op_ret) {
    			vty_out(vty,"%% The eth-port wasn't supported the flow-control config\n");
    		}
    		else if (ETHPORT_RETURN_CODE_ERR_HW == op_ret) {
    			vty_out(vty,"%% operation err on hardware\n"); 
    		}
    		else if (ETHPORT_RETURN_CODE_ERR_PRODUCT_TYPE == op_ret) {
    			vty_out(vty,"%% Error product type\n");
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
	
	query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,		\
							NPD_DBUS_ETHPORTS_OBJPATH ,	\
							NPD_DBUS_ETHPORTS_INTERFACE ,		\
							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_ATTR);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&type,
							 DBUS_TYPE_UINT32,&port_index,
							 DBUS_TYPE_UINT32,&isEnable,
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

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_UINT32, &port_index,
					DBUS_TYPE_INVALID)) 
	{
		if (ETHPORT_RETURN_CODE_ERR_GENERAL == op_ret ) 
		{
			vty_out(vty,"execute command failed.\n"); 
		}
		else if(ETHPORT_RETURN_CODE_ERR_NONE == op_ret)
		{
			DCLI_DEBUG(("port %d config flow-control succeed\n",port_index));
		}
		else if(ETHPORT_RETURN_CODE_NO_SUCH_PORT == op_ret)
		{
			DCLI_DEBUG(("%% port is not exist\n",port_index));
		}
		else if(ETHPORT_RETURN_CODE_FLOWCTL_NODE == op_ret) {
			vty_out(vty,"%% Flow control is configed only duplex is full and auto-negotiation flow-control is disable.\n");
		}
		else if (ETHPORT_RETURN_CODE_UNSUPPORT == op_ret) {
			vty_out(vty,"%% The eth-port don't support flow-control setting\n");
		}
		else if (VLAN_RETURN_CODE_PORT_TRUNK_MBR == op_ret) {
			vty_out(vty,"%% The eth-port wasn't supported config when the port belonged to a active trunk \n");
		}		
		else if (ETHPORT_RETURN_CODE_NOT_SUPPORT == op_ret) {
			vty_out(vty,"%% The eth-port wasn't supported the flow-control config\n");
		}
		else if (ETHPORT_RETURN_CODE_ERR_HW == op_ret) {
			vty_out(vty,"%% operation err on hardware\n"); 
		}
		else if (ETHPORT_RETURN_CODE_ERR_PRODUCT_TYPE == op_ret) {
			vty_out(vty,"%% Error product type\n");
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

DEFUN(config_ethport_back_pressure_cmd_func,
	config_ethport_back_pressure_cmd,
	"config back-pressure (enable|disable)",
	CONFIG_STR
	"Config eth-port back-pressure information\n"
	"Specify eth-port back-pressure duplex enable \n"
	"Specify eth-port back-pressure duplex disable\n"
	)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;

	int ret = 0, op_ret = 0;
	unsigned int port_index = 0;
	unsigned int isEnable = 0;
	unsigned int type = BACKPRE;
	unsigned int slot_no;
	int local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
	
	if(argc > 1){
		vty_out(vty,"input too many params\n ");
		return CMD_WARNING;
	}
	
	DCLI_DEBUG(("before parse value %s\n",argv[0]));
	if(strncmp(argv[0] , "enable",strlen((char*)argv[0])) == 0){
		isEnable = 1;
	}
	else if (strncmp(argv[0],"disable",strlen((char*)argv[0]))== 0){
		isEnable = 0;
	}
	else {
		vty_out(vty,"back-pressure config value must be enable or disable\n");
		return CMD_WARNING;
	}
	
	port_index = (unsigned int)vty->index;
	DCLI_DEBUG(("config port_index %d\n",port_index));
	
	/* for config eth3-3 on slot1, AX7605i  bug:AXSSZFI-188 */
	if (is_distributed == DISTRIBUTED_SYSTEM)
	{
	#if 1
		unsigned int slot_no;
        int local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
	    SLOT_PORT_ANALYSIS_SLOT(port_index, slot_no);
		# if 1
		query = dbus_message_new_method_call(
							SEM_DBUS_BUSNAME,\
							SEM_DBUS_OBJPATH,\
							SEM_DBUS_INTERFACE, \
							SEM_DBUS_CONFIG_ETHPORT_ATTR \
			                );
		dbus_error_init(&err);

		dbus_message_append_args(query,
		 		  		    DBUS_TYPE_UINT32,&type,
							DBUS_TYPE_UINT32,&port_index,
							DBUS_TYPE_UINT32,&isEnable,
							DBUS_TYPE_INVALID);

    	if(NULL == dbus_connection_dcli[slot_no]->dcli_dbus_connection) 				
    	{
    		if(slot_no == local_slot_id) 
    		{
    		    reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
    		}
    		else 
    		{	
    			vty_out(vty, "Connection to slot%d is not exist.\n", slot_no);
    			return CMD_WARNING;
    		}
    	}else {
    		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_no]->dcli_dbus_connection,\
    				query, -1, &err);
    	}

		dbus_message_unref(query);
		
		if (NULL == reply) {
				vty_out(vty,"failed get reply.\n");
				if (dbus_error_is_set(&err)) {
					vty_out(vty,"%s raised: %s",err.name,err.message);
					dbus_error_free_for_dcli(&err);
				}
				return CMD_SUCCESS;
			}
		if (dbus_message_get_args ( reply, &err,
				DBUS_TYPE_UINT32, &op_ret,
				DBUS_TYPE_UINT32, &port_index,
				DBUS_TYPE_INVALID)) 
		{
			if (SEM_COMMAND_FAILED == op_ret ) 
			{
				vty_out(vty,"execute command failed.\n");
				dbus_message_unref(reply);
				return CMD_FAILURE;
			}
			else if(SEM_COMMAND_SUCCESS == op_ret)
			{
				DCLI_DEBUG(("port %d config back-pressure succeed\n",port_index));
				dbus_message_unref(reply);
				return CMD_SUCCESS;
			}
			else if (SEM_WRONG_PARAM == op_ret ) 
			{
				vty_out(vty,"%% wrong parame\n"); 
				dbus_message_unref(reply);
				return CMD_WARNING;
			}
			else if (SEM_OPERATE_NOT_SUPPORT == op_ret) {
				
				vty_out(vty,"%% The eth-port don't support back-pressure control setting\n");
				dbus_message_unref(reply);
				return CMD_WARNING;
			}
			else if (SEM_COMMAND_NOT_SUPPORT == op_ret) {
				// TODO:call npd dbus to process.
				DCLI_DEBUG(("%% call npd dbus to process\n"));
				dbus_message_unref(reply);		
			}
			else
			{
				vty_out(vty, "unkown return value\n");
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
			dbus_message_unref(reply);
			return CMD_WARNING;
		}
		# endif
	#endif
    	query = dbus_message_new_method_call(
    							NPD_DBUS_BUSNAME,		\
    							NPD_DBUS_ETHPORTS_OBJPATH ,	\
    							NPD_DBUS_ETHPORTS_INTERFACE ,		\
    							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_ATTR);
    	
    	dbus_error_init(&err);

    	dbus_message_append_args(query,
    							DBUS_TYPE_UINT32,&type,
    							 DBUS_TYPE_UINT32,&port_index,
    							 DBUS_TYPE_UINT32,&isEnable,
    							 DBUS_TYPE_INVALID);
		SLOT_PORT_ANALYSIS_SLOT(port_index, slot_no);
    	if(NULL == dbus_connection_dcli[slot_no]->dcli_dbus_connection) 				
    	{
    		if(slot_no == local_slot_id) 
    		{
    		    reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
    		}
    		else 
    		{	
    			vty_out(vty, "Connection to slot%d is not exist.\n", slot_no);
    			return CMD_WARNING;
    		}
    	}else {
    		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_no]->dcli_dbus_connection,\
    				query, -1, &err);
    	}
    	
    	dbus_message_unref(query);
    	
    	if (NULL == reply) {
    		vty_out(vty,"failed get reply.\n");
    		if (dbus_error_is_set(&err)) {
    			vty_out(vty,"%s raised: %s",err.name,err.message);
    			dbus_error_free_for_dcli(&err);
    		}
    		return CMD_SUCCESS;
    	}

    	if (dbus_message_get_args ( reply, &err,
    					DBUS_TYPE_UINT32, &op_ret,
    					DBUS_TYPE_UINT32, &port_index,
    					DBUS_TYPE_INVALID)) 
    	{
    		if (ETHPORT_RETURN_CODE_ERR_GENERAL == op_ret ) 
    		{
    			vty_out(vty,"execute command failed.\n"); 
    		}
    		else if(ETHPORT_RETURN_CODE_ERR_NONE == op_ret)
    		{
    			DCLI_DEBUG(("port %d config back-pressure duplex succeed\n",port_index));
    		}
    		else if(ETHPORT_RETURN_CODE_BACKPRE_NODE == op_ret) {
    			vty_out(vty,"%% Back-pressure is configed only port duplex is half.\n");
    		}
    		else if (ETHPORT_RETURN_CODE_UNSUPPORT == op_ret) {
    			vty_out(vty,"%% The eth-port don't support back-pressure setting\n");
    		}
    		else if (ETHPORT_RETURN_CODE_ERR_HW == op_ret) {
    			vty_out(vty,"%% operation err on hardware\n"); 
    		}		
    		else if (ETHPORT_RETURN_CODE_ERR_PRODUCT_TYPE == op_ret) {
    			vty_out(vty,"%% Error product type\n");
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
    /* is not distributed  */
	query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,		\
							NPD_DBUS_ETHPORTS_OBJPATH ,	\
							NPD_DBUS_ETHPORTS_INTERFACE ,		\
							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_ATTR);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&type,
							 DBUS_TYPE_UINT32,&port_index,
							 DBUS_TYPE_UINT32,&isEnable,
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

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_UINT32, &port_index,
					DBUS_TYPE_INVALID)) 
	{
		if (ETHPORT_RETURN_CODE_ERR_GENERAL == op_ret ) 
		{
			vty_out(vty,"execute command failed.\n"); 
		}
		else if(ETHPORT_RETURN_CODE_ERR_NONE == op_ret)
		{
			DCLI_DEBUG(("port %d config back-pressure duplex succeed\n",port_index));
		}
		else if(ETHPORT_RETURN_CODE_BACKPRE_NODE == op_ret) {
			vty_out(vty,"%% Back-pressure is configed only port duplex is half.\n");
		}
		else if (ETHPORT_RETURN_CODE_UNSUPPORT == op_ret) {
			vty_out(vty,"%% The eth-port don't support back-pressure setting\n");
		}
		else if (ETHPORT_RETURN_CODE_ERR_HW == op_ret) {
			vty_out(vty,"%% operation err on hardware\n"); 
		}		
		else if (ETHPORT_RETURN_CODE_ERR_PRODUCT_TYPE == op_ret) {
			vty_out(vty,"%% Error product type\n");
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


DEFUN(config_ethport_link_state_cmd_func,
	config_ethport_link_state_cmd,
	"config eth-port PORTNO link state (down|up|auto)",
	CONFIG_STR
	"Config eth-port \n"
	CONFIG_ETHPORT_STR
	"Config eth-port link information\n"
	"Config eth-port link state information\n"
	"Specify eth-port link state down \n"
	"Specify eth-port link state up\n"
	"Specify eth-port link state auto\n"
	)
{
	DBusMessage *query = NULL , *reply = NULL;
	DBusError err;

	int op_ret = 0;
	unsigned int port_index = 0;
	unsigned int isEnable = 0;
	unsigned char *modeStr=NULL;
	unsigned char slot_no,port_no;


	op_ret = parse_slotport_no((char *)argv[0], &slot_no, &port_no);
	if (NPD_FAIL == op_ret) {
		vty_out(vty,"input bad slot/port!\n");
		return CMD_SUCCESS;
	}

	if (is_distributed == DISTRIBUTED_SYSTEM)	
	{
		int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
		if(slot_no > slotNum || slot_no <= 0)
		{
			vty_out(vty,"%% NO SUCH SLOT %d!\n", slot_no);
            return CMD_WARNING;
		}
	}
	
	modeStr = (char*)argv[1];
	if(strncmp(modeStr,"up",strlen(modeStr)) == 0){
		isEnable = 1;
	}
	else if (strncmp(modeStr,"down",strlen(modeStr))== 0){
		isEnable = 0;
	}
	else if (strncmp(modeStr,"auto",strlen(modeStr))== 0){
		isEnable = 2;
	}
	else {
		vty_out(vty,"link state config value must be enable or disable or auto.\n");
		return CMD_WARNING;
	}
	
	query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,		\
							NPD_DBUS_ETHPORTS_OBJPATH ,	\
							NPD_DBUS_ETHPORTS_INTERFACE ,		\
							NPD_DBUS_SYSTEM_CONFIG_PORT_LINK);

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&slot_no,
							 DBUS_TYPE_BYTE,&port_no,
							 DBUS_TYPE_UINT32,&isEnable,
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

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_UINT32, &port_index,
					DBUS_TYPE_INVALID)) 
	{
		if (ETHPORT_RETURN_CODE_ERR_GENERAL == op_ret ) 
		{
			vty_out(vty,"execute command failed.\n"); 
		}
		else if(ETHPORT_RETURN_CODE_NO_SUCH_PORT == op_ret){
				vty_out(vty,"%% Error NO SUCH PORT!\n");
		}
		else  if(ETHPORT_RETURN_CODE_ERR_NONE == op_ret){
			/*vty_out(vty," config link state succeed\n",port_index);*/
		}
		else if (ETHPORT_RETURN_CODE_UNSUPPORT == op_ret) {
			vty_out(vty,"%% The eth-port don't support link state setting\n");
		}
		else if (ETHPORT_RETURN_CODE_ERR_HW == op_ret) {
			vty_out(vty,"%% operation err on hardware\n"); 
		}
		else if (ETHPORT_RETURN_CODE_ERR_PRODUCT_TYPE == op_ret) {
			vty_out(vty,"%% Error product type\n");
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

DEFUN(config_ethport_mtu_cmd_func,
	config_ethport_mtu_cmd,
	"config mtu <64-8192>",
	CONFIG_STR
	"Config eth-port mtu information\n"
	"Specify eth-port mtu value \n"
	)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;

	int ret=0, op_ret=0;
	unsigned int port_index = 0;
	unsigned short param = 1522;
	unsigned int mtu,type = CFGMTU;

	if(argc > 1){
		vty_out(vty,"input too many params\n ");
		return CMD_WARNING;
	}
	
	DCLI_DEBUG(("before parse value %s\n",argv[0]));
	ret = parse_short_parse((char *)argv[0],&param);
	if(NPD_SUCCESS != ret){
		vty_out(vty,"parse param error\n");
		return CMD_WARNING;
	}

	if((param < 64) && (param > 8192)) {
		vty_out(vty,"%% bad param.\n");
	}
	
	mtu = param;
	DCLI_DEBUG(("after parse param %d\n",mtu));

	port_index = (unsigned int)vty->index;
	DCLI_DEBUG(("config port_index %d\n",port_index));

   if (is_distributed == DISTRIBUTED_SYSTEM)
    {
		unsigned int slot_no;
        int local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
		
		SLOT_PORT_ANALYSIS_SLOT(port_index, slot_no);
		
		query = dbus_message_new_method_call(
							SEM_DBUS_BUSNAME,\
							SEM_DBUS_OBJPATH,\
							SEM_DBUS_INTERFACE, \
							SEM_DBUS_CONFIG_ETHPORT_ATTR \
			                );
		dbus_error_init(&err);

		dbus_message_append_args(query,
		 		  		    DBUS_TYPE_UINT32,&type,
							DBUS_TYPE_UINT32,&port_index,
							DBUS_TYPE_UINT32,&mtu,
							DBUS_TYPE_INVALID);
		
		SLOT_PORT_ANALYSIS_SLOT(port_index, slot_no);

    	if(NULL == dbus_connection_dcli[slot_no]->dcli_dbus_connection) 				
    	{
    		if(slot_no == local_slot_id) 
    		{
    		    reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
    		}
    		else 
    		{	
    			vty_out(vty, "Connection to slot%d is not exist.\n", slot_no);
    			return CMD_WARNING;
    		}
    	}else {
    		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_no]->dcli_dbus_connection,\
    				query, -1, &err);
    	}
		
		dbus_message_unref(query);

		
		if (NULL == reply) {
			vty_out(vty,"failed get reply.\n");
			if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_SUCCESS;
		}
		if (dbus_message_get_args ( reply, &err,
				DBUS_TYPE_UINT32, &op_ret,
				DBUS_TYPE_UINT32, &port_index,
				DBUS_TYPE_INVALID)) 
		{
			if (SEM_COMMAND_FAILED == op_ret ) 
			{
				vty_out(vty,"execute command failed.\n");
				dbus_message_unref(reply);
				return CMD_FAILURE;
			}
			else if(SEM_COMMAND_SUCCESS == op_ret)
			{
				DCLI_DEBUG(("port %d config mtu succeed\n",port_index));
				dbus_message_unref(reply);
				return CMD_SUCCESS;
			}
			else if (SEM_WRONG_PARAM == op_ret ) 
			{
				vty_out(vty,"%% wrong parame\n"); 
				dbus_message_unref(reply);
				return CMD_WARNING;
			}
			else if (SEM_OPERATE_NOT_SUPPORT == op_ret) {
				
				vty_out(vty,"%% The eth-port don't support mtu setting\n");
				dbus_message_unref(reply);
				return CMD_WARNING;
			}
			else if (SEM_COMMAND_NOT_SUPPORT == op_ret) {
				// TODO:call npd dbus to process.
				DCLI_DEBUG(("%% call npd dbus to process\n"));
				dbus_message_unref(reply);		
			}
			else
			{
				vty_out(vty, "unkown return value\n");
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
			dbus_message_unref(reply);
			return CMD_WARNING;
		} 
		
    	query = dbus_message_new_method_call(
    							NPD_DBUS_BUSNAME,		\
    							NPD_DBUS_ETHPORTS_OBJPATH ,	\
    							NPD_DBUS_ETHPORTS_INTERFACE ,		\
    							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_ATTR);
    	
    	dbus_error_init(&err);

    	dbus_message_append_args(query,
    							 DBUS_TYPE_UINT32,&type,
    							 DBUS_TYPE_UINT32,&port_index,
    							 DBUS_TYPE_UINT32,&mtu,
    							 DBUS_TYPE_INVALID);

    	if(NULL == dbus_connection_dcli[slot_no]->dcli_dbus_connection) 				
    	{
    		if(slot_no == local_slot_id) 
    		{
    		    reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
    		}
    		else 
    		{	
    			vty_out(vty, "Connection to slot%d is not exist.\n", slot_no);
    			return CMD_WARNING;
    		}
    	}else {
    		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_no]->dcli_dbus_connection,\
    				query, -1, &err);
    	}
    	
    	dbus_message_unref(query);
    	
    	if (NULL == reply) {
    		vty_out(vty,"failed get reply.\n");
    		if (dbus_error_is_set(&err)) {
    			vty_out(vty,"%s raised: %s",err.name,err.message);
    			dbus_error_free_for_dcli(&err);
    		}
    		return CMD_SUCCESS;
    	}

    	if (dbus_message_get_args ( reply, &err,
    					DBUS_TYPE_UINT32, &op_ret,
    					DBUS_TYPE_UINT32, &port_index,
    					DBUS_TYPE_INVALID)) 
    	{
    		if (ETHPORT_RETURN_CODE_ERR_GENERAL == op_ret ) 
    		{
    			vty_out(vty,"%% execute command failed.\n"); 
    		}
    		else if(ETHPORT_RETURN_CODE_ERR_NONE == op_ret)
    		{
    			DCLI_DEBUG(("port  %d config mtu succeed\n",port_index));
    		}
    		else if (ETHPORT_RETURN_CODE_BAD_VALUE == op_ret)
    		{
               vty_out(vty,"%% input only even value.\n"); 
    		}
    		else if (ETHPORT_RETURN_CODE_ERR_HW == op_ret) {
    			vty_out(vty,"%% operation err on hardware\n"); 
    		}
    		else if (ETHPORT_RETURN_CODE_UNSUPPORT == op_ret) {
    			vty_out(vty,"%% The eth-port don't support mtu setting\n");
    		}
    		else if (ETHPORT_RETURN_CODE_ERR_PRODUCT_TYPE == op_ret) {
    			vty_out(vty,"%% Error product type\n");
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
	
	query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,		\
							NPD_DBUS_ETHPORTS_OBJPATH ,	\
							NPD_DBUS_ETHPORTS_INTERFACE ,		\
							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_ATTR);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&type,
							 DBUS_TYPE_UINT32,&port_index,
							 DBUS_TYPE_UINT32,&mtu,
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

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_UINT32, &port_index,
					DBUS_TYPE_INVALID)) 
	{
		if (ETHPORT_RETURN_CODE_ERR_GENERAL == op_ret ) 
		{
			vty_out(vty,"%% execute command failed.\n"); 
		}
		else if(ETHPORT_RETURN_CODE_ERR_NONE == op_ret)
		{
			DCLI_DEBUG(("port  %d config mtu succeed\n",port_index));
		}
		else if (ETHPORT_RETURN_CODE_BAD_VALUE == op_ret)
		{
           vty_out(vty,"%% input only even value.\n"); 
		}
		else if (ETHPORT_RETURN_CODE_ERR_HW == op_ret) {
			vty_out(vty,"%% operation err on hardware\n"); 
		}
		else if (ETHPORT_RETURN_CODE_UNSUPPORT == op_ret) {
			vty_out(vty,"%% The eth-port don't support mtu setting\n");
		}
		else if (ETHPORT_RETURN_CODE_ERR_PRODUCT_TYPE == op_ret) {
			vty_out(vty,"%% Error product type\n");
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


DEFUN(config_ethport_default_cmd_func,
	config_ethport_default_cmd,
	"config default",
	CONFIG_STR
	"Specify eth-port default value \n"
	)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;

	int ret = 0, op_ret = 0;
	unsigned int port_index = 0;
	unsigned int defval = 0;
	unsigned int type = DEFAULT;
	
	port_index = (unsigned int)vty->index;
	DCLI_DEBUG(("config port_index %d\n",port_index));
	
    if (is_distributed == DISTRIBUTED_SYSTEM)
    {
		unsigned int slot_no;
        int local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
		
		SLOT_PORT_ANALYSIS_SLOT(port_index, slot_no);
		
		query = dbus_message_new_method_call(
							SEM_DBUS_BUSNAME,\
							SEM_DBUS_OBJPATH,\
							SEM_DBUS_INTERFACE, \
							SEM_DBUS_CONFIG_ETHPORT_ATTR \
			                );
		dbus_error_init(&err);

		dbus_message_append_args(query,
		 		  		    DBUS_TYPE_UINT32,&type,
							DBUS_TYPE_UINT32,&port_index,
							DBUS_TYPE_UINT32,&defval,
							DBUS_TYPE_INVALID);
		
		SLOT_PORT_ANALYSIS_SLOT(port_index, slot_no);

    	if(NULL == dbus_connection_dcli[slot_no]->dcli_dbus_connection) 				
    	{
    		if(slot_no == local_slot_id) 
    		{
    		    reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
    		}
    		else 
    		{	
    			vty_out(vty, "Connection to slot%d is not exist.\n", slot_no);
    			return CMD_WARNING;
    		}
    	}else {
    		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_no]->dcli_dbus_connection,\
    				query, -1, &err);
    	}
		
		dbus_message_unref(query);

		
		if (NULL == reply) {
			vty_out(vty,"failed get reply.\n");
			if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_SUCCESS;
		}
		if (dbus_message_get_args ( reply, &err,
				DBUS_TYPE_UINT32, &op_ret,
				DBUS_TYPE_UINT32, &port_index,
				DBUS_TYPE_INVALID)) 
		{
			if (SEM_COMMAND_FAILED == op_ret ) 
			{
				vty_out(vty,"execute command failed.\n");
				dbus_message_unref(reply);
				return CMD_FAILURE;
			}
			else if(SEM_COMMAND_SUCCESS == op_ret)
			{
				DCLI_DEBUG(("port %d config defualt value succeed\n",port_index));
				dbus_message_unref(reply);
				return CMD_SUCCESS;
			}
			else if (SEM_WRONG_PARAM == op_ret ) 
			{
				vty_out(vty,"%% wrong parame\n"); 
				dbus_message_unref(reply);
				return CMD_WARNING;
			}
			else if (SEM_OPERATE_NOT_SUPPORT == op_ret) {
				
				vty_out(vty,"%% The eth-port don't support media setting\n");
				dbus_message_unref(reply);
				return CMD_WARNING;
			}
			else if (SEM_COMMAND_NOT_SUPPORT == op_ret) {
				// TODO:call npd dbus to process.
				DCLI_DEBUG(("%% call npd dbus to process\n"));
				dbus_message_unref(reply);		
			}
			else
			{
				vty_out(vty, "unkown return value\n");
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
			dbus_message_unref(reply);
			return CMD_WARNING;
		} 
		
    	query = dbus_message_new_method_call(
    							NPD_DBUS_BUSNAME,		\
    							NPD_DBUS_ETHPORTS_OBJPATH ,	\
    							NPD_DBUS_ETHPORTS_INTERFACE ,		\
    							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_ATTR);
    	
    	dbus_error_init(&err);

    	dbus_message_append_args(query,
    							 DBUS_TYPE_UINT32,&type,
    							 DBUS_TYPE_UINT32,&port_index,
    							 DBUS_TYPE_UINT32,&defval,
    							 DBUS_TYPE_INVALID);

    	if(NULL == dbus_connection_dcli[slot_no]->dcli_dbus_connection) 				
    	{
    		if(slot_no == local_slot_id) 
    		{
    		    reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
    		}
    		else 
    		{	
    			vty_out(vty, "Connection to slot%d is not exist.\n", slot_no);
    			return CMD_WARNING;
    		}
    	}else {
    		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_no]->dcli_dbus_connection,\
    				query, -1, &err);
    	}
    	
    	dbus_message_unref(query);
    	
    	if (NULL == reply) {
    		vty_out(vty,"failed get reply.\n");
    		if (dbus_error_is_set(&err)) {
    			vty_out(vty,"%s raised: %s",err.name,err.message);
    			dbus_error_free_for_dcli(&err);
    		}
    		return CMD_SUCCESS;
    	}

    	if (dbus_message_get_args ( reply, &err,
    					DBUS_TYPE_UINT32, &op_ret,
    					DBUS_TYPE_UINT32, &port_index,
    					DBUS_TYPE_INVALID)) 
    	{
			
    		if (ETHPORT_RETURN_CODE_ERR_GENERAL == op_ret ) 
    		{
    			vty_out(vty,"execute command failed.\n"); 
    		}
    		else  if(ETHPORT_RETURN_CODE_ERR_NONE == op_ret)
    		{
    			DCLI_DEBUG(("port  %d config default values succeed\n",port_index));    
    		}
    		else if (ETHPORT_RETURN_CODE_UNSUPPORT == op_ret) {
    			vty_out(vty,"%% The eth-port wasn't supported media set\n");
    		}
    		else 
    			vty_out(vty,"%% Please config default by configure guide.\n");
			
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
	query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,		\
							NPD_DBUS_ETHPORTS_OBJPATH ,	\
							NPD_DBUS_ETHPORTS_INTERFACE ,		\
							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_ATTR);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&type,
							 DBUS_TYPE_UINT32,&port_index,
							 DBUS_TYPE_UINT32,&defval,
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

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_UINT32, &port_index,
					DBUS_TYPE_INVALID)) 
	{
		if (ETHPORT_RETURN_CODE_ERR_GENERAL == op_ret ) 
		{
			vty_out(vty,"execute command failed.\n"); 
		}
		else  if(ETHPORT_RETURN_CODE_ERR_NONE == op_ret)
		{
			DCLI_DEBUG(("port  %d config default value succeed\n",port_index));
		}
		else if (ETHPORT_RETURN_CODE_UNSUPPORT == op_ret) {
			vty_out(vty,"%% The eth-port wasn't supported media set\n");
		}
		else 
			vty_out(vty,"%% Please config default by configure guide.\n");
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


DEFUN(config_eth_port_media_preferred_cmd_func,
	config_eth_port_media_preferred_cmd,
	"config media preferred (copper|fiber|none)",
	CONFIG_STR
	"Configure eth-port preferred media interface\n"
	"Configure eth-port preferred media interface\n"
	"Copper media as preferred interface\n"
	"Fiber media as preferred interface\n"
	"No interface type has privilege\n"
	)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int media = 0;
	unsigned int op_ret = 0;
	unsigned int port_index = 0;

    if (is_distributed == DISTRIBUTED_SYSTEM)
    {
        vty_out(vty,"%% The eth-port isn't supported media set\n");
	}
	else
	{
    	if(0 == strncmp("copper",argv[0],strlen((char*)argv[0]))) {
    		media = COMBO_PHY_MEDIA_PREFER_COPPER;
    	}
    	else if(0 == strncmp("fiber",argv[0],strlen((char*)argv[0]))) {
    		media = COMBO_PHY_MEDIA_PREFER_FIBER;
    	}
    	else if(0 == strncmp("none",argv[0],strlen((char*)argv[0]))) {
    		media = COMBO_PHY_MEDIA_PREFER_NONE;
    	}
    	else {
        	vty_out(vty,"%% Unknow input param\n");
    		return CMD_SUCCESS;
    	}


    	port_index = (unsigned int)vty->index;

    	query = dbus_message_new_method_call(
    					NPD_DBUS_BUSNAME,				\
    					NPD_DBUS_ETHPORTS_OBJPATH,		\
    					NPD_DBUS_ETHPORTS_INTERFACE,		\
    					NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_ETHPORT_MEDIA);
    	
    	dbus_error_init(&err);

    	dbus_message_append_args(query,
    							 DBUS_TYPE_UINT32,&port_index,
    							 DBUS_TYPE_UINT32,&media,
    							 DBUS_TYPE_INVALID);

    	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
    	
    	dbus_message_unref(query);
    	
    	if (NULL == reply) {
    		vty_out(vty,"query reply null\n");
    		vty_out(vty,"failed get reply.\n");
    		if (dbus_error_is_set(&err)) {
    			vty_out(vty,"%s raised: %s",err.name,err.message);
    			dbus_error_free_for_dcli(&err);
    		}
    		return CMD_SUCCESS;
    	}
    	
    	if (dbus_message_get_args ( reply, &err,
    		DBUS_TYPE_UINT32,&op_ret,
    		DBUS_TYPE_INVALID)) {
    			if (ETHPORT_RETURN_CODE_ERR_NONE == op_ret ) {
    				DCLI_DEBUG(("exe success\n"));
    			}
    			else if (ETHPORT_RETURN_CODE_UNSUPPORT == op_ret) {
    				vty_out(vty,"%% The eth-port isn't supported media set\n");
    			}
    			else if (ETHPORT_RETURN_CODE_NOT_SUPPORT == op_ret) {
    				vty_out(vty,"%% The eth-port isn't supported media set 'none'.\n");
    			}
    			else if (ETHPORT_RETURN_CODE_ERR_GENERAL == op_ret) {
    				vty_out(vty,"%% general error such as device or port out of range and etc.");
    			}
    			else if (ETHPORT_RETURN_CODE_ERR_PRODUCT_TYPE == op_ret) {
    				vty_out(vty,"%% Error product type\n");
    			}
    			else 
    				vty_out(vty,"%% Config media error\n");
    		
    	} else {
    		vty_out(vty,"Failed get args.\n");
    		if (dbus_error_is_set(&err)) {
    			vty_out(vty,"%s raised: %s",err.name,err.message);
    			dbus_error_free_for_dcli(&err);
    		}
    	}
    	
    	dbus_message_unref(reply);
    }
	return CMD_SUCCESS;
}


clear_eth_port_arp
(
	struct vty* vty,
	unsigned int value,
	unsigned int type,
	unsigned int isStatic
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;

	unsigned char slot_no = 0, port_no = 0;
	unsigned int op_ret = 0, ret = 0;


	if(0 == type){
		slot_no = (unsigned char)((value>>8)&0xff);
		port_no = (unsigned char)(value & 0xff);
		value = 0xffff; 
	}

	DCLI_DEBUG(("%s %d :: clear type %d value %d slot/port %d/%d info\n",__func__,__LINE__,type,value,slot_no,port_no));

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,		\
								NPD_DBUS_ETHPORTS_OBJPATH,		\
								NPD_DBUS_ETHPORTS_INTERFACE,		\
								NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CLEAR_ETHPORT_ARP);
	

	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&type,
							 DBUS_TYPE_BYTE,&slot_no,
							 DBUS_TYPE_BYTE,&port_no,
							 DBUS_TYPE_UINT32,&value,
							 DBUS_TYPE_UINT32,&isStatic,
							 DBUS_TYPE_INVALID);
    if (is_distributed) {
		int i;

	   for ( i = 0; i< MAX_SLOT; i++) {
          
	  	  if (NULL != dbus_connection_dcli[i]->dcli_dbus_connection) {
		  	
         	reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[i]->dcli_dbus_connection,query,-1, &err);
         	if (NULL == reply) {
         		vty_out(vty,"failed get reply.\n");
         		if (dbus_error_is_set(&err)) {
         			vty_out(vty,"%s raised: %s",err.name,err.message);
         			dbus_error_free_for_dcli(&err);
         		}
         		return CMD_SUCCESS;
         	}

         	if (dbus_message_get_args ( reply, &err,
         					DBUS_TYPE_UINT32, &op_ret,
         					DBUS_TYPE_INVALID)) 
         	{

         		if (ETHPORT_RETURN_CODE_ERR_GENERAL == op_ret ) 
         		{
         			vty_out(vty,"execute command failed.\n"); 
         		}
         		else  if(ETHPORT_RETURN_CODE_ERR_NONE == op_ret)
         		{
         			/*DCLI_DEBUG(("port  %d clear arp\n",value));*/
         		}
         		else if (ETHPORT_RETURN_CODE_NO_SUCH_PORT == op_ret) 
         		{
         			vty_out(vty,"Error:Illegal %d/%d,No such port.\n",slot_no,port_no);/*Stay here,not enter eth_config_node CMD.*/
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
         
         	if (dbus_message_get_args ( reply, &err,
         					DBUS_TYPE_UINT32, &op_ret,
         					DBUS_TYPE_INVALID)) 
         	{
         		if (ETHPORT_RETURN_CODE_ERR_GENERAL == op_ret ) 
         		{
         			vty_out(vty,"execute command failed.\n"); 
         		}
         		else  if(ETHPORT_RETURN_CODE_ERR_NONE == op_ret)
         		{
         			/*DCLI_DEBUG(("port  %d clear arp\n",value));*/
         		}
         		else if (ETHPORT_RETURN_CODE_NO_SUCH_PORT == op_ret) 
         		{
         			vty_out(vty,"Error:Illegal %d/%d,No such port.\n",slot_no,port_no);/*Stay here,not enter eth_config_node CMD.*/
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

DEFUN(clear_ethport_arp_cn_cmd_func,
	clear_ethport_arp_cn_cmd,
	"clear eth-port PORTNO arp [static]",
	CLEAR_STR
	"Clear ethernet port information\n"
	CONFIG_ETHPORT_STR 
	"ARP information\n"
	"Clear static arp only\n"
	)
{
	unsigned char slot_no,port_no;
	unsigned int type;
	unsigned int value = 0;
	unsigned int isStatic = 0;
	int ret;

	ret = parse_slotport_no((char *)argv[0],&slot_no,&port_no);

	if (NPD_FAIL == ret) {
    	vty_out(vty,"Unknow portno format.\n");
		return CMD_WARNING;
	}

	if (is_distributed == DISTRIBUTED_SYSTEM)	
	{
		int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
		if(slot_no > slotNum || slot_no <= 0)
		{
			vty_out(vty,"%% NO SUCH SLOT %d!\n", slot_no);
            return CMD_WARNING;
		}
	}
	
	DCLI_DEBUG(("%d :: show slot/port %d/%d info\n",__LINE__,slot_no,port_no));
	
	type = 0;
	value = slot_no;
	value =  (value << 8) |port_no;
	if(argc > 1){
		if(!strncmp(argv[1],"static",strlen(argv[1]))){
            isStatic = 1;
		}
		else {
            vty_out(vty,"%% Unknown Command!\n");
			return CMD_WARNING;
		}
	}
	ret = clear_eth_port_arp(vty,value,type,isStatic);
	if(ret != CMD_SUCCESS)
		vty_out(vty,"%% Clear arp error\n");
	
	return ret;
}

DEFUN(clear_ethport_arp_en_cmd_func,
	clear_ethport_arp_en_cmd,
	"clear eth-port arp",
	CLEAR_STR
	"Clear ethernet port information\n"
	"ARP information\n"
	)
{
	unsigned char slot_no = 0,port_no = 0;
	unsigned int type;
	unsigned int value = (unsigned int)(vty->index);
	unsigned int isStatic = 0;
	int ret = 0;

	DCLI_DEBUG(("%d :: show port_index %d\n",__LINE__,value));
	
	type = 1;
	if(argc > 1){
		if(!strncmp(argv[1],"static",strlen(argv[1]))){
            isStatic = 1;
		}
		else {
            vty_out(vty,"%% Unknown Command!\n");
			return CMD_WARNING;
		}
	}

	ret = clear_eth_port_arp(vty,value,type,isStatic);
	if(ret != CMD_SUCCESS)
		vty_out(vty,"%% Clear arp error\n");
	
	return ret;
}

int dcli_eth_port_show_running_config(struct vty* vty) {	
	char *showStr = NULL,*cursor = NULL,ch = 0,tmpBuf[SHOWRUN_PERLINE_SIZE] = {0};
	DBusMessage *query, *reply;
	DBusError err;
	int ret = 1, i = 0;

	if (is_distributed == DISTRIBUTED_SYSTEM)
	{
    	char *showStr = NULL,*cursor = NULL;
    	int totalLen = 0;
    	int i = 0;

    	showStr = (char*)malloc(ETHPORT_SHOWRUN_CFG_SIZE);

    	if(NULL == showStr) {
    		printf("memory malloc error\n");
    		return -1;
    	}
    	memset(showStr, 0, ETHPORT_SHOWRUN_CFG_SIZE);
    	cursor = showStr;

		int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
		for(i = 0; i < BOARD_GLOBAL_ETHPORTS_MAXNUM*slotNum; i++)
		{
            if(global_ethport[i]->isValid == VALID_ETHPORT)
            {
            	unsigned int attr_bitmap = 0, mtu = 0,local_port_ipg = 0;
            	unsigned int ret = 0;
            	unsigned char enter_node = 0; /* need to enter eth-port node first*/
            	char tmpBuf[2048] = {0}, *tmpPtr = NULL;
            	int length = 0;
            	unsigned char an = 0,an_state = 0,an_fc = 0,fc = 0,an_duplex = 0,duplex = 0,an_speed = 0;
            	unsigned char admin_status = 0,bp = 0,media = 0;
            	PORT_SPEED_ENT speed = PORT_SPEED_10_E;
		
            	attr_bitmap	= global_ethport[i]->attr_bitmap;
            	mtu		    = global_ethport[i]->mtu; 
				
            	tmpPtr = tmpBuf;

            	/* MRU*/

            	if(mtu != global_ethport[i]->port_default_attr.mtu) 
        		{		
            		enter_node = 1;
            		length += sprintf(tmpPtr," config mtu %d\n",mtu);
            		tmpPtr = tmpBuf+length;
			    }

            	admin_status = (attr_bitmap & ETH_ATTR_ADMIN_STATUS) >> ETH_ADMIN_STATUS_BIT;
            	bp = (attr_bitmap & ETH_ATTR_BACKPRESSURE) >> ETH_BACKPRESSURE_BIT;
            	an_state = (attr_bitmap & ETH_ATTR_AUTONEG) >> ETH_AUTONEG_BIT;
            	an = (attr_bitmap & ETH_ATTR_AUTONEG_CTRL) >> ETH_AUTONEG_CTRL_BIT;
            	an_speed = (attr_bitmap & ETH_ATTR_AUTONEG_SPEED) >> ETH_AUTONEG_SPEED_BIT;
            	speed = (attr_bitmap & ETH_ATTR_SPEED_MASK) >> ETH_SPEED_BIT;
            	an_duplex = (attr_bitmap & ETH_ATTR_AUTONEG_DUPLEX) >> ETH_AUTONEG_DUPLEX_BIT;
            	duplex = (attr_bitmap & ETH_ATTR_DUPLEX) >> ETH_DUPLEX_BIT;
            	an_fc = (attr_bitmap & ETH_ATTR_AUTONEG_FLOWCTRL) >> ETH_AUTONEG_FLOWCTRL_BIT;
            	fc = (attr_bitmap & ETH_ATTR_FLOWCTRL) >> ETH_FLOWCTRL_BIT;
                media = global_ethport[i]->port_type;

				/*
            	media = (attr_bitmap & ETH_ATTR_MEDIA_MASK) >> ETH_ATTR_MEDIA_BIT; 
            	*/

            	/*media priority*/
            	if(media == ETH_GTX && global_ethport[i]->port_default_attr.mediaPrefer == COMBO_PHY_MEDIA_PREFER_FIBER) {
            		enter_node = 1;
            		length += sprintf(tmpPtr," config media mode copper\n");
            		tmpPtr = tmpBuf + length;				
    			}
            	else if((media == ETH_GE_FIBER || media == ETH_GE_SFP) && global_ethport[i]->port_default_attr.mediaPrefer == COMBO_PHY_MEDIA_PREFER_COPPER) {
            		enter_node = 1;
            		length += sprintf(tmpPtr," config media mode fiber\n");
            		tmpPtr = tmpBuf + length;	
    			}

				/*  Auto-Nego - all AN options disable we need one command control*/
            	if((an != global_ethport[i]->port_default_attr.autoNego) &&
            		!(an_speed != an && an_duplex != an && an_fc != an) /* &&
            		(an == an_speed) && (an == an_duplex) && (an == an_fc) */){
            		enter_node = 1;
            		length += sprintf(tmpPtr," config auto-negotiation %s\n", an ? "enable":"disable");
            		tmpPtr = tmpBuf+length;
            	}

            	/* Auto-Nego speed*/
            	if(((an_speed != global_ethport[i]->port_default_attr.speed_an && 
            		an == global_ethport[i]->port_default_attr.autoNego) ||
            		(an_speed != an && an != global_ethport[i]->port_default_attr.autoNego)) &&
            		!(an_speed != an && an_duplex != an && an_fc != an)) {
            		enter_node = 1;
            		length += sprintf(tmpPtr," config auto-negotiation speed %s\n", an_speed ? "enable":"disable");
            		tmpPtr = tmpBuf+length;
            	}
	   		
            	/* Auto-Nego duplex*/
            	if(((an_duplex != global_ethport[i]->port_default_attr.duplex_an && 
            		an == global_ethport[i]->port_default_attr.autoNego)||
            		(an_duplex != an  && an != global_ethport[i]->port_default_attr.autoNego)) &&
            		!(an_speed != an && an_duplex != an && an_fc != an)) {
            		enter_node = 1;
            		length += sprintf(tmpPtr," config auto-negotiation duplex %s\n", an_duplex ? "enable":"disable");
            		tmpPtr = tmpBuf+length;
            	}
				
            	/* Auto-Nego flow-control*/
            	if(((an_fc != global_ethport[i]->port_default_attr.fc_an && 
            		an == global_ethport[i]->port_default_attr.autoNego)||
            		(an_fc != an && an != global_ethport[i]->port_default_attr.autoNego)) &&
            		!(an_speed != an && an_duplex != an && an_fc != an)) {
            		enter_node = 1;
            		length += sprintf(tmpPtr," config auto-negotiation flow-control %s\n",an_fc ? "enable":"disable");
            		tmpPtr = tmpBuf+length;
            	}

            	/* speed - if not auto-nego speed and speed not default*/

        		if((an_speed != ETH_ATTR_ON) && (speed != global_ethport[i]->port_default_attr.speed)) {
        			enter_node = 1;
        			length += sprintf(tmpPtr," config speed %d\n",(speed == PORT_SPEED_100_E) ? 100:10);
        			tmpPtr = tmpBuf+length;
        		}		    	

            	/* flow-control - if not auto-nego fc and fc mode not default*/
            	if((an_fc != ETH_ATTR_ON) && (fc != global_ethport[i]->port_default_attr.fc)){
            		enter_node = 1;
            		length += sprintf(tmpPtr," config flow-control %s\n", fc ? "enable":"disable");
            		tmpPtr = tmpBuf+length;
            	}
            	/* duplex - if not auto-nego duplex and duplex mode not default*/
            	if((an_duplex != ETH_ATTR_ON) && (duplex != global_ethport[i]->port_default_attr.duplex)){
            		enter_node = 1;
            		length += sprintf(tmpPtr," config duplex mode %s\n", duplex ? "half":"full");
            		tmpPtr = tmpBuf+length;
            	}	
            	/* back-pressure  - back pressure only relevant when port in half-duplex mode*/
            	if((bp != global_ethport[i]->port_default_attr.bp) && (PORT_HALF_DUPLEX_E == duplex)) {
            		enter_node = 1;
            		length += sprintf(tmpPtr," config back-pressure %s\n", bp ? "enable":"disable");
            		tmpPtr = tmpBuf + length;
            	}

            	/* admin status*/
            	if(admin_status != global_ethport[i]->port_default_attr.admin_state) {
            		enter_node = 1;
            		length += sprintf(tmpPtr," config admin state %s\n",admin_status ? "enable":"disable");
            		tmpPtr = tmpBuf+length;
            	}
            	if(enter_node) 
    		{
            		if((totalLen + length + 20 + 5) > ETHPORT_SHOWRUN_CFG_SIZE) { /* 20 for enter node; 5 for exit node*/
            			printf("show ethport buffer full");
            			break;
            		}				

            		totalLen += sprintf(cursor,"config eth-port %d/%d\n",global_ethport[i]->slot_no,global_ethport[i]->local_port_no);
            		cursor = showStr + totalLen;
            		totalLen += sprintf(cursor,"%s",tmpBuf);
            		cursor = showStr + totalLen;
            		totalLen += sprintf(cursor," exit\n");
            		cursor = showStr + totalLen;
            		enter_node = 0;
            	}

        	}	
		}
    			
		char _tmpstr[64];
		memset(_tmpstr,0,64);
		sprintf(_tmpstr,BUILDING_MOUDLE,"ETH PORT");
		vtysh_add_show_string(_tmpstr);

		vtysh_add_show_string(showStr);

		return 0;	
	}

	// TODO:non-distributed product
	query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,		\
							NPD_DBUS_ETHPORTS_OBJPATH , \
							NPD_DBUS_ETHPORTS_INTERFACE ,		\
							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_RUNNING_CONFIG);

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("show eth_port running config failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return ret;
	}
	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_STRING, &showStr,
					DBUS_TYPE_INVALID)) 
	{
	
			char _tmpstr[64];
			memset(_tmpstr,0,64);
			sprintf(_tmpstr,BUILDING_MOUDLE,"ETH PORT");
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
	return ret;
}

int show_ethport_arp_info
(
	struct vty* vty, 
	unsigned int value,
	unsigned int type
)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_struct,iter_array;
	unsigned char slot_no = 0 ,port_no = 0;
	unsigned int op_ret = 0,ret = 0,i = 0,j = 0;
	unsigned int arpCount = 0;
	unsigned int ipaddr = 0;
	unsigned char mac[6] = {0};
	unsigned char isTrunk = 0;
	unsigned char trunkId = 0;
	unsigned short vid = 0,vidx = 0;
	unsigned char isTagged = 0,isStatic = 0, isValid = 0;
	


	if(0 == type){
		slot_no = (unsigned char)((value>>8)&0xff);
		port_no = (unsigned char)(value & 0xff);
		value = 0xffff; 
	}

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,		\
								NPD_DBUS_ETHPORTS_OBJPATH,		\
								NPD_DBUS_ETHPORTS_INTERFACE,		\
								NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_ETHPORT_ARP);
	

	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&type,
							 DBUS_TYPE_BYTE,&slot_no,
							 DBUS_TYPE_BYTE,&port_no,
							 DBUS_TYPE_UINT32,&value,
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



	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&op_ret);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&arpCount);
	/*vty_out(vty,"arp count %d\n",arpCount);*/
	dbus_message_iter_next(&iter);	
	

	if(CMD_SUCCESS == op_ret){
		if(1 == type) {
			dcli_get_slot_port_by_portindex(vty,value, &slot_no,&port_no);
		}
		vty_out(vty,"Detail Information of Ethernet Port %d-%d (%d Items)    * - Invalid item\n",slot_no,port_no, arpCount);
		vty_out(vty,"==============================================================================\n");
		vty_out(vty,"%-2s%-16s%-18s%-10s%-8s%-6s%-6s%-6s%-8s\n"," ","IP","MAC","SLOT/PORT","TRUNKID","VID","VIDX","ISTAG","STATIC");
		vty_out(vty,"==============================================================================\n");

		if(arpCount > 0) {
			dbus_message_iter_recurse(&iter,&iter_array);
			for(i = 0; i<arpCount; i++) {
				dbus_message_iter_recurse(&iter_array,&iter_struct);

                dbus_message_iter_get_basic(&iter_struct,&isValid);
				dbus_message_iter_next(&iter_struct);
				vty_out(vty,"%-2s",isValid ? " " : "*");

				dbus_message_iter_get_basic(&iter_struct,&ipaddr);
				dbus_message_iter_next(&iter_struct);
				/*vty_out(vty,"ipaddr %d\n",ipaddr);*/
				vty_out(vty,"%-3d.%-3d.%-3d.%-3d ",((ipaddr & 0xff000000) >> 24),((ipaddr & 0xff0000) >> 16),	\
					((ipaddr & 0xff00) >> 8),(ipaddr & 0xff));

				for(j = 0; j< 6; j++) {
		 			dbus_message_iter_get_basic(&iter_struct,&mac[j]);
					/*vty_out(vty,"mac[j] %d\n",mac[j]);*/
					dbus_message_iter_next(&iter_struct);
				}
				
				vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x ",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
				vty_out(vty,"%2d/%-2d",slot_no,port_no);

				dbus_message_iter_get_basic(&iter_struct,&isTrunk);
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&trunkId);
				dbus_message_iter_next(&iter_struct);
				vty_out(vty,"%-5s","");
				if(isTrunk) 
					vty_out(vty,"%-8d",trunkId);
				else
					vty_out(vty,"%2s%-6s",""," - ");

				dbus_message_iter_get_basic(&iter_struct,&vid);
				dbus_message_iter_next(&iter_struct);
				vty_out(vty,"%-6d",vid);

				dbus_message_iter_get_basic(&iter_struct,&vidx);
				dbus_message_iter_next(&iter_struct);
				vty_out(vty,"%-6s"," - ");

				dbus_message_iter_get_basic(&iter_struct,&isTagged);
				dbus_message_iter_next(&iter_struct);
				vty_out(vty,"%-6s",isTagged ? "TRUE":"FALSE");

				dbus_message_iter_get_basic(&iter_struct,&isStatic);
				dbus_message_iter_next(&iter_struct);
				vty_out(vty,"%-8s",isStatic ? "TRUE" : "FALSE");

				vty_out(vty,"\n");
				dbus_message_iter_next(&iter_array);
			}
		}

		/*vty_out(vty,"here end\n");*/
		vty_out(vty,"================================ARP COUNT %-4d================================\n",arpCount);
	}
	else if (NPD_DBUS_ERROR_NO_SUCH_PORT == op_ret) 
	{
		vty_out(vty,"%% Illegal port %d/%d\n",slot_no,port_no);/*Stay here,not enter eth_config_node CMD.*/
	}
	else if (NPD_DBUS_ERROR == op_ret ) 
	{
		vty_out(vty,"%% Command execute fail\n"); 
	}
	dbus_message_unref(reply);

	return CMD_SUCCESS;
}

DEFUN(show_ethport_arp_cn_cmd_func,
	show_ethport_arp_cn_cmd,
	"show eth-port PORTNO arp",
	SHOW_STR
	"Show eth-port information\n"
	CONFIG_ETHPORT_STR 
	"ARP information\n"
)
{
	unsigned char slotno = 0,portno = 0;
	unsigned int type = 0;
	unsigned int value = 0;
	int ret = 0;
	
	ret = parse_slotport_no((char *)argv[0], &slotno, &portno);
	if(0 != ret) {
		vty_out(vty,"input bad param\n");
		return CMD_WARNING;
	}

	if (is_distributed == DISTRIBUTED_SYSTEM)	
	{
		int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
		if(slotno > slotNum || slotno <= 0)
		{
			vty_out(vty,"%% NO SUCH SLOT %d!\n", slotno);
            return CMD_WARNING;
		}
	}
		
	type = 0;
	value |= (slotno<<8);
	value |= portno;

	ret = show_ethport_arp_info(vty,value,type);
	if(0 != ret) {
		vty_out(vty,"show arp info error\n");
	}

	return CMD_SUCCESS;
}

DEFUN(show_ethport_staticarp_cn_cmd_func,
	show_ethport_staticarp_cn_cmd,
	"show eth-port PORTNO static arp",
	SHOW_STR
	"Show eth-port information\n"
	CONFIG_ETHPORT_STR 
	"Static arp entry\n"
	"ARP information\n"
)
{
	unsigned char slotno = 0,portno = 0;
	unsigned int type;
	unsigned int value = 0;
	int ret = 0;
	
	ret = parse_slotport_no((char *)argv[0], &slotno, &portno);
	if(0 != ret) {
		vty_out(vty,"input bad param\n");
		return CMD_WARNING;
	}

	if (is_distributed == DISTRIBUTED_SYSTEM)	
	{
		int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
		if(slotno > slotNum || slotno <= 0)
		{
			vty_out(vty,"%% NO SUCH SLOT %d!\n", slotno);
            return CMD_WARNING;
		}
	}
		
	type = 0;
	value |= (slotno<<8);
	value |= portno;

	ret = show_ethport_arp_info(vty,value,type);
	if(0 != ret) {
		vty_out(vty,"show arp info error\n");
	}

	return CMD_SUCCESS;
}


DEFUN(show_ethport_arp_en_cmd_func,
	show_ethport_arp_en_cmd,
	"show eth-port arp",
	SHOW_STR
	"Show eth-port information\n"
	"ARP information\n"
)
{
	unsigned int type;
	unsigned int value = (unsigned int)vty->index;
	int ret = 0;
	
	
	type = 1;
	ret = show_ethport_arp_info(vty,value,type);
	if(0 != ret) {
		vty_out(vty,"show arp info error\n");
	}

	return CMD_SUCCESS;
}


DEFUN(show_ethport_staticarp_en_cmd_func,
	show_ethport_staticarp_en_cmd,
	"show eth-port static arp",
	SHOW_STR
	"Show eth-port information\n"
	"Static arp entry\n"
	"ARP information\n"
)
{
	unsigned int type;
	unsigned int value = (unsigned int)vty->index;
	int ret = 0;
	
	
	type = 1;
	ret = show_ethport_arp_info(vty,value,type);
	if(0 != ret) {
		vty_out(vty,"show arp info error\n");
	}

	return CMD_SUCCESS;
}


int show_ethport_nexthop_info
(
	struct vty* vty, 
	unsigned int value,
	unsigned int type
)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_struct,iter_array;
	unsigned char slot_no = 0 ,port_no = 0;
	unsigned int op_ret = 0,ret = 0,i = 0,j = 0;
	unsigned int arpCount = 0;
	unsigned int ipaddr = 0;
	unsigned char mac[6] = {0};
	unsigned char isTrunk = 0;
	unsigned char trunkId = 0;
	unsigned short vid = 0,vidx = 0;
	unsigned char isTagged = 0,isStatic = 0;
	unsigned int refCnt = 0;

	if(0 == type){
		slot_no = (unsigned char)((value>>8)&0xff);
		port_no = (unsigned char)(value & 0xff);
		value = 0xffff; 
	}

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,		\
								NPD_DBUS_ETHPORTS_OBJPATH,		\
								NPD_DBUS_ETHPORTS_INTERFACE,		\
								NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_ETHPORT_NEXTHOP);
	

	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&type,
							 DBUS_TYPE_BYTE,&slot_no,
							 DBUS_TYPE_BYTE,&port_no,
							 DBUS_TYPE_UINT32,&value,
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



	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&op_ret);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&arpCount);
	/*vty_out(vty,"arp count %d\n",arpCount);*/
	dbus_message_iter_next(&iter);	
	

	if(CMD_SUCCESS == op_ret){
		if(1 == type) {
			dcli_get_slot_port_by_portindex(vty,value, &slot_no,&port_no);
		}
		vty_out(vty,"Detail Information of Ethernet Port %d-%d (%d Items)\n",slot_no,port_no,arpCount);
		vty_out(vty,"================================================================================\n");
		vty_out(vty,"%-16s%-18s%-10s%-8s%-5s%-5s%-6s%-7s%-4s\n",	\
					 "IP","MAC","SLOT/PORT","TRUNKID","VID","VIDX","ISTAG","STATIC","REF");
		vty_out(vty,"================================================================================\n");

		if(arpCount > 0) {
			dbus_message_iter_recurse(&iter,&iter_array);
			for(i = 0; i<arpCount; i++) {
				dbus_message_iter_recurse(&iter_array,&iter_struct);

				dbus_message_iter_get_basic(&iter_struct,&ipaddr);
				dbus_message_iter_next(&iter_struct);
				/*vty_out(vty,"ipaddr %d\n",ipaddr);*/
				vty_out(vty,"%-3d.%-3d.%-3d.%-3d ",((ipaddr & 0xff000000) >> 24),((ipaddr & 0xff0000) >> 16),	\
					((ipaddr & 0xff00) >> 8),(ipaddr & 0xff));

				for(j = 0; j< 6; j++) {
		 			dbus_message_iter_get_basic(&iter_struct,&mac[j]);
					/*vty_out(vty,"mac[j] %d\n",mac[j]);*/
					dbus_message_iter_next(&iter_struct);
				}
				
				vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x ",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
				vty_out(vty,"%-2d/%-2d",slot_no,port_no);

				dbus_message_iter_get_basic(&iter_struct,&isTrunk);
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&trunkId);
				dbus_message_iter_next(&iter_struct);
				vty_out(vty,"%-5s","");
				if(isTrunk) 
					vty_out(vty,"%-8d",trunkId);
				else
					vty_out(vty,"%-8s"," - ");

				dbus_message_iter_get_basic(&iter_struct,&vid);
				dbus_message_iter_next(&iter_struct);
				vty_out(vty,"%-5d",vid);

				dbus_message_iter_get_basic(&iter_struct,&vidx);
				dbus_message_iter_next(&iter_struct);
				vty_out(vty,"%-5s"," - ");

				dbus_message_iter_get_basic(&iter_struct,&isTagged);
				dbus_message_iter_next(&iter_struct);
				vty_out(vty,"%-6s",isTagged ? "TRUE":"FALSE");

				dbus_message_iter_get_basic(&iter_struct,&isStatic);
				dbus_message_iter_next(&iter_struct);
				vty_out(vty,"%-7s",isStatic ? "TRUE" : "FALSE");
				
				dbus_message_iter_get_basic(&iter_struct,&refCnt);
				dbus_message_iter_next(&iter_struct);
				vty_out(vty,"%-d",refCnt);

				vty_out(vty,"\n");
				dbus_message_iter_next(&iter_array);
			}
		}

		/*vty_out(vty,"here end\n");*/
		vty_out(vty,"===============================NEXT-HOP COUNT %-4d==============================\n",arpCount);
	}
	else if (NPD_DBUS_ERROR_NO_SUCH_PORT == op_ret) 
	{
    		vty_out(vty,"%% Illegal port %d/%d\n",slot_no,port_no);/*Stay here,not enter eth_config_node CMD.*/
	}
	else if (NPD_DBUS_ERROR == op_ret ) 
	{
		vty_out(vty,"%% Command execute fail\n"); 
	}
	dbus_message_unref(reply);

	return CMD_SUCCESS;
}

DEFUN(show_ethport_nexthop_en_cmd_func,
	show_ethport_nexthop_en_cmd,
	"show eth-port nexthop",
	SHOW_STR
	"Show eth-port information\n"
	"Nexthop table information\n"
)
{
	unsigned int type;
	unsigned int value = (unsigned int)vty->index;
	int ret = 0;
	
	
	type = 1;
	ret = show_ethport_nexthop_info(vty,value,type);
	if(0 != ret) {
		vty_out(vty,"show arp info error\n");
	}

	return CMD_SUCCESS;
}



DEFUN(show_ethport_nexthop_cn_cmd_func,
	show_ethport_nexthop_cn_cmd,
	"show eth-port PORTNO nexthop",
	SHOW_STR
	"Show eth-port information\n"
	CONFIG_ETHPORT_STR
	"Nexthop table information\n"
)
{
	unsigned char slotno = 0,portno = 0;
	unsigned int type;
	unsigned int value = 0;
	int ret = 0;
	
	ret = parse_slotport_no((char*)argv[0], &slotno, &portno);
	if(0 != ret) {
		vty_out(vty,"input bad param\n");
		return CMD_WARNING;
	}

	if (is_distributed == DISTRIBUTED_SYSTEM)	
	{
		int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
		if(slotno > slotNum || slotno <= 0)
		{
			vty_out(vty,"%% NO SUCH SLOT %d!\n", slotno);
            return CMD_WARNING;
		}
	}
		
	type = 0;
	value |= (slotno<<8);
	value |= portno;

	ret = show_ethport_nexthop_info(vty,value,type);
	if(0 != ret) {
		vty_out(vty,"show arp info error\n");
	}

	return CMD_SUCCESS;
}


DEFUN(show_buffer_mode_cmd_func,
	show_buffer_mode_cmd,
	"show buffer mode",
	SHOW_STR
	"Show buffer mode\n"
	"Show buffer mode information\n"
)
{
	
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = { 0 };
	unsigned int Isable = 0;
	query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,		\
							NPD_DBUS_ETHPORTS_OBJPATH ,	\
							NPD_DBUS_ETHPORTS_INTERFACE ,		\
							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_BUFFER_MODE);
	
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&Isable,
		DBUS_TYPE_INVALID))
	{						
		vty_out(vty,"----------------------------\n");
		if(BUFFER_MODE_SHARED == Isable) {
			vty_out(vty,"Global buffer mode is shared!\n");
		}
		else if(BUFFER_MODE_DIVIDED == Isable) {
			vty_out(vty,"Global buffer mode is divided!\n");		
		}
		else {
			vty_out(vty,"Global buffer mode is uncertain!\n");
		}
		vty_out(vty,"----------------------------\n");
	} 
	else 
	{
		/*vty_out(vty,"Failed get args.\n");*/
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
unsigned int dcli_eth_port_config_sc_common
(
	unsigned char modeType,
    unsigned int g_index,
    unsigned int scMode,
    unsigned int scType,
    unsigned int scValue,
    unsigned int* ret
)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int op_ret = 0;
	unsigned char slotno = 0,portno = 0;

	if(0 == modeType){
		slotno = (unsigned char)((g_index>>8)&0xff);
		portno = (unsigned char)(g_index & 0xff);
		g_index = 0xffff; 
	}
	
	query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,		\
							NPD_DBUS_ETHPORTS_OBJPATH , \
							NPD_DBUS_ETHPORTS_INTERFACE ,		\
							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_STORM_CONTROL);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&modeType,
							 DBUS_TYPE_BYTE,&slotno,
							 DBUS_TYPE_BYTE,&portno,
							 DBUS_TYPE_UINT32,&g_index,
							 DBUS_TYPE_UINT32,&scMode,
							 DBUS_TYPE_UINT32,&scType,
							 DBUS_TYPE_UINT32,&scValue,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {	
		return DCLI_ERROR_DBUS;
	}

	else if (!dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) {
		dbus_message_unref(reply);
	    return DCLI_ERROR_DBUS;
	}
	*ret = op_ret;
	dbus_message_unref(reply);
	return DCLI_ERROR_NONE;

}

unsigned int dcli_eth_port_config_sc
(
	struct vty* vty, 
	unsigned char modeType,
    unsigned int g_index,
    unsigned int scMode,
    unsigned int scType,
    unsigned int scValue
)
{
    unsigned int ret = 0,op_ret = 0;
	
	op_ret = dcli_eth_port_config_sc_common(modeType,g_index,scMode,scType,scValue,&ret);

	if(DCLI_ERROR_DBUS == op_ret){
		vty_out(vty,"Dbus error.\n");			
	}
	else if(DCLI_ERROR_NONE== op_ret){
		if (NPD_DBUS_ERROR == ret ) 
		{
			vty_out(vty,"execute command failed.\n"); 
		}
		else if(NPD_DBUS_ERROR_NOT_SUPPORT == ret) {
			vty_out(vty,"%% value out of range.\n");
		}
		else if (NPD_DBUS_ERROR_OPERATE == ret) {
			vty_out(vty,"%% Hardware error or other storm control type has been set!\n");
		}
	}
    return CMD_SUCCESS;
}

unsigned int dcli_global_config_sc_common
(
    unsigned int scType,
    unsigned int* ret
)
{
    DBusMessage *query, *reply;
	DBusError err;

	unsigned int op_ret = 0;
	
	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,		\
								NPD_DBUS_ETHPORTS_OBJPATH,		\
								NPD_DBUS_ETHPORTS_INTERFACE,		\
								NPD_DBUS_ETHPORTS_INTERFACE_METHOD_STORM_CONTROL_GLOBAL_MODEL);


	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&scType,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
	   *ret = 0;
       return DCLI_ERROR_DBUS;
	}

	else if (! dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) 
	{
		*ret = 0;
		return DCLI_ERROR_DBUS;

	} 
    *ret = op_ret;
	dbus_message_unref(reply);
    return DCLI_ERROR_NONE;
}
unsigned int dcli_global_config_sc
(
	struct vty* vty,
	unsigned int scType
)
{
    unsigned int ret = 0,op_ret = 0;
	ret = dcli_global_config_sc_common(scType,&op_ret);
	if(DCLI_ERROR_DBUS == ret){
		vty_out(vty,"Dbus error.\n");			
	}
    else {
		if (NPD_DBUS_ERROR == op_ret ) 
		{
			vty_out(vty,"%% execute command failed.\n"); 
		}
		else if (NPD_DBUS_ERROR_NOT_SUPPORT == op_ret) 
		{
			vty_out(vty,"%% Not support this global configuration!\n");
		}
		else if(NPD_DBUS_ERROR_OPERATE== op_ret){
            vty_out(vty,"%% There may be ports running on other mode,disable it first please!\n");
		}
	} 
	return CMD_SUCCESS;
}


DEFUN(config_storm_control_global_model_cn_cmd_func ,
	config_storm_control_global_model_cn_cmd,
	"config storm-control (pps|bps)",
	CONFIG_STR
	"Storm control configuration\n"
	"Strom control type: packet per second on the port\n"
	"Strom control type: bit per second on the port\n"
)
{   

	int ret= 0,op_ret = 0;
	unsigned int modeType= DCLI_ETH_PORT_STREAM_MAX_E;


	if(argc > 1){
		vty_out(vty,"input too many params\n ");
		return CMD_WARNING;
	}

	if(strncmp(argv[0] , "pps",strlen((char*)argv[0])) == 0){
		modeType = DCLI_ETH_PORT_STREAM_PPS_E;
	}
	else if (strncmp(argv[0],"bps",strlen((char*)argv[0]))== 0){
		modeType = DCLI_ETH_PORT_STREAM_BPS_E;
	}

	ret = dcli_global_config_sc(vty,modeType);
	if(ret != CMD_SUCCESS)
		vty_out(vty,"%% Config %s error\n",( DCLI_ETH_PORT_STREAM_BPS_E == modeType)?"bps":"pps");
	return ret;
}

DEFUN(config_port_storm_control_pps_cn_cmd_func ,
	config_port_storm_control_pps_cn_cmd,
	"config eth-port PORTNO storm-control (dlf|broadcast|multicast) pps <0-1488100>",
	CONFIG_STR
	"Display Ethernet Port information\n"
	"Please input port not like 3-2 (port 2 on slot 3 of a chassis product) or 5 (port 5 of a box product).\n"
	"Storm control for each port configuration\n"
	"Strom control type: dlf for destination looking up failure packets received on the port\n"
	"Strom control type: broadcast for broadcast packets on the port\n"
	"Strom control type: multicast for multicast packets on the port\n"
	"Storm control rate: pakcets per second\n"
	"Value range <0 - 1488100> for GE port,and value range <0 - 148810> for FE port\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;

	int ret,op_ret;
	unsigned char slotno = 0,portno = 0,type = 0,modeType = 0;
	unsigned int cntype = 0,value = 0,cnvalue = 0;

	if(argc > 3){
		vty_out(vty,"input too many params\n ");
		return CMD_WARNING;
	}
	
	ret = parse_slotport_no((char*)argv[0], &slotno, &portno);
	if(0 != ret) {
		vty_out(vty,"input bad param\n");
		return CMD_WARNING;
	}

	if (is_distributed == DISTRIBUTED_SYSTEM)	
	{
		int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
		if(slotno > slotNum || slotno <= 0)
		{
			vty_out(vty,"%% NO SUCH SLOT %d!\n", slotno);
            return CMD_WARNING;
		}
	}
		
	DCLI_DEBUG(("before parse, type %s,value %d\n",argv[1],argv[2]));
	if(strncmp(argv[1] , "dlf",strlen((char*)argv[1])) == 0){
		cntype = PORT_STORM_CONTROL_STREAM_DLF;
	}
	else if (strncmp(argv[1],"broadcast",strlen((char*)argv[1]))== 0){
		cntype = PORT_STORM_CONTROL_STREAM_BCAST;
	}
	else if (strncmp(argv[1],"multicast",strlen((char*)argv[1]))== 0){
		cntype = PORT_STORM_CONTROL_STREAM_MCAST;
	}
	else {
		vty_out(vty,"storm-control config type should be dlf,broadcast or multicast!\n");
		return CMD_WARNING;
	}
	ret = parse_int_parse((char *)argv[2],&cnvalue);
	if(NPD_SUCCESS != ret){
		vty_out(vty,"parse param error\n");
		return CMD_WARNING;
	}

	if((cnvalue < 0)||(cnvalue > 1488100)) {
		vty_out(vty,"%% bad value input!\n");
		return CMD_WARNING;
	}
	type = 0;
	value = slotno;
	value =  (value << 8) |portno;
    modeType = DCLI_ETH_PORT_STREAM_PPS_E;
	
    ret = dcli_eth_port_config_sc(vty,type,value,modeType,cntype,cnvalue);
	if(ret != CMD_SUCCESS)
		vty_out(vty,"%% Config pps error\n");
	return ret;
}


DEFUN(config_port_storm_control_pps_en_cmd_func ,
	config_port_storm_control_pps_en_cmd,
	"config storm-control (dlf|broadcast|multicast) pps <0-1488100>",
	CONFIG_STR
	"Storm control for each port configuration\n"
	"Strom control type: dlf for destination looking up failure packets received on the port\n"
	"Strom control type: broadcast for broadcast packets on the port\n"
	"Strom control type: multicast for multicast packets on the port\n"
	"Storm control rate: pakcets per second\n"
	"Value range <0 - 1488100> for GE port,and value range <0 - 148810> for FE port\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;

	int ret,op_ret;
	unsigned char slotno = 0,portno = 0,type = 0,modeType = 0;
	unsigned int cntype = 0,value = 0,cnvalue = 0;

	if(argc > 2){
		vty_out(vty,"input too many params\n ");
		return CMD_WARNING;
	}
	
	DCLI_DEBUG(("before parse, type %s,value %d\n",argv[1],argv[2]));
	if(strncmp(argv[0] , "dlf",strlen((char*)argv[0])) == 0){
		cntype = PORT_STORM_CONTROL_STREAM_DLF;
	}
	else if (strncmp(argv[0],"broadcast",strlen((char*)argv[0]))== 0){
		cntype = PORT_STORM_CONTROL_STREAM_BCAST;
	}
	else if (strncmp(argv[0],"multicast",strlen((char*)argv[0]))== 0){
		cntype = PORT_STORM_CONTROL_STREAM_MCAST;
	}
	else {
		vty_out(vty,"storm-control config type should be dlf,broadcast or multicast!\n");
		return CMD_WARNING;
	}
	ret = parse_int_parse((char *)argv[1],&cnvalue);
	if(NPD_SUCCESS != ret){
		vty_out(vty,"parse param error\n");
		return CMD_WARNING;
	}

	if((cnvalue < 0)||(cnvalue > 1488100)) {
		vty_out(vty,"%% bad value input!\n");
		return CMD_WARNING;
	}
	type = 1;
	value = (unsigned int)(vty->index);
	modeType = DCLI_ETH_PORT_STREAM_PPS_E;

    ret = dcli_eth_port_config_sc(vty,type,value,modeType,cntype,cnvalue);
	if(ret != CMD_SUCCESS)
		vty_out(vty,"%% Config pps error\n");
	return ret;
}


DEFUN(config_port_storm_control_bps_cn_cmd_func ,
	config_port_storm_control_bps_cn_cmd,
	"config eth-port PORTNO storm-control (dlf|broadcast|multicast) bps <0-1000000000>",
	CONFIG_STR
	"Display Ethernet Port information\n"
	CONFIG_ETHPORT_STR 
	"Storm control for each port configuration\n"
	"Strom control type: dlf for destination looking up failure bytes received on the port\n"
	"Strom control type: broadcast for broadcast bytes on the port\n"
	"Strom control type: multicast for multicast bytes on the port\n"
	"Storm control rate: bits per second\n"
	"Value range <0 - 1000000000> for GE port,and value range <0 - 100000000> for FE port\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;

	int ret,op_ret;
	unsigned char slotno = 0,portno = 0,type = 0,modeType = 0;
	unsigned int cntype = 0,value = 0,cnvalue = 0;

	if(argc > 3){
		vty_out(vty,"input too many params\n ");
		return CMD_WARNING;
	}
	
	ret = parse_slotport_no((char*)argv[0], &slotno, &portno);
	if(0 != ret) {
		vty_out(vty,"input bad param\n");
		return CMD_WARNING;
	}
	
	if (is_distributed == DISTRIBUTED_SYSTEM)	
	{
		int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
		if(slotno > slotNum || slotno <= 0)
		{
			vty_out(vty,"%% NO SUCH SLOT %d!\n", slotno);
            return CMD_WARNING;
		}
	}
	
	DCLI_DEBUG(("before parse, type %s,value %d\n",argv[1],argv[2]));
	if(strncmp(argv[1] , "dlf",strlen((char*)argv[1])) == 0){
		cntype = PORT_STORM_CONTROL_STREAM_DLF;
	}
	else if (strncmp(argv[1],"broadcast",strlen((char*)argv[1]))== 0){
		cntype = PORT_STORM_CONTROL_STREAM_BCAST;
	}
	else if (strncmp(argv[1],"multicast",strlen((char*)argv[1]))== 0){
		cntype = PORT_STORM_CONTROL_STREAM_MCAST;
	}
	else {
		vty_out(vty,"storm-control config type should be dlf,broadcast or multicast!\n");
		return CMD_WARNING;
	}
	ret = parse_int_parse((char *)argv[2],&cnvalue);
	if(NPD_SUCCESS != ret){
		vty_out(vty,"parse param error\n");
		return CMD_WARNING;
	}

	if((cnvalue < 0)||(cnvalue > 1000000000)) {
		vty_out(vty,"%% bad value input!\n");
		return CMD_WARNING;
	}
	
	type = 0;
	value = slotno;
	value =  (value << 8) |portno;
    modeType = DCLI_ETH_PORT_STREAM_BPS_E;
	
	ret = dcli_eth_port_config_sc(vty,type,value,modeType,cntype,cnvalue);
	if(ret != CMD_SUCCESS)
		vty_out(vty,"%% Config bps error\n");
	return ret;	
}




DEFUN(config_port_storm_control_bps_en_cmd_func ,
	config_port_storm_control_bps_en_cmd,
	"config storm-control (dlf|broadcast|multicast) bps <0-1000000000>",
	CONFIG_STR
	"Storm control for each port configuration\n"
	"Strom control type: dlf for destination looking up failure bytes received on the port\n"
	"Strom control type: broadcast for broadcast bytes on the port\n"
	"Strom control type: multicast for multicast bytes on the port\n"
	"Storm control rate: bit per second\n"
	"Value range <0 - 1000000000> for GE port,and value range <0 - 100000000> for FE port\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;

	int ret,op_ret;
	unsigned char slotno = 0,portno = 0,type = 0,modeType = 0;
	unsigned int cntype = 0,value = 0,cnvalue = 0;

	if(argc > 2){
		vty_out(vty,"input too many params\n ");
		return CMD_WARNING;
	}
	
	DCLI_DEBUG(("before parse, type %s,value %d\n",argv[1],argv[2]));
	if(strncmp(argv[0] , "dlf",strlen((char*)argv[0])) == 0){
		cntype = PORT_STORM_CONTROL_STREAM_DLF;
	}
	else if (strncmp(argv[0],"broadcast",strlen((char*)argv[0]))== 0){
		cntype = PORT_STORM_CONTROL_STREAM_BCAST;
	}
	else if (strncmp(argv[0],"multicast",strlen((char*)argv[0]))== 0){
		cntype = PORT_STORM_CONTROL_STREAM_MCAST;
	}
	else {
		vty_out(vty,"storm-control config type should be dlf,broadcast or multicast!\n");
		return CMD_WARNING;
	}
	ret = parse_int_parse((char *)argv[1],&cnvalue);
	if(NPD_SUCCESS != ret){
		vty_out(vty,"parse param error\n");
		return CMD_WARNING;
	}

	if((cnvalue < 0)||(cnvalue > 1000000000)) {
		vty_out(vty,"%% bad value input!\n");
		return CMD_WARNING;
	}
	type = 1;
	value = (unsigned int)(vty->index);
	modeType = DCLI_ETH_PORT_STREAM_BPS_E;

    ret = dcli_eth_port_config_sc(vty,type,value,modeType,cntype,cnvalue);
	if(ret != CMD_SUCCESS)
		vty_out(vty,"%% Config pps error\n");
	return ret;
}
/*add by yinlm 091012 for vct*/
DEFUN(config_ethport_vct_cmd_func,
	config_ethport_vct_cmd,
	"config vct state enable",
	CONFIG_STR
	"Config eth-port vct information\n"
	"Config eth-port vct state information\n"
	"Specify eth-port vct enable \n"
	)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;

	int ret = 0,op_ret = 0;
	unsigned int port_index = 0;
	unsigned int mode = 0;
	unsigned char slot_no = 0, port_no = 0;

	if (1 == argc) {
		ret = parse_slotport_no((char*)argv[0], &slot_no, &port_no);
		if(0 != ret) {
			vty_out(vty,"input bad param\n");
			return CMD_WARNING;
		}
	}
	else {
		port_index = (unsigned int)vty->index;
		mode = 1;
	}
	DCLI_DEBUG(("config port_index %d\n",port_index));
	query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,		\
							NPD_DBUS_ETHPORTS_OBJPATH ,	\
							NPD_DBUS_ETHPORTS_INTERFACE ,		\
							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_VCT);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&slot_no,
							 DBUS_TYPE_BYTE,&port_no,
							 DBUS_TYPE_UINT32,&port_index,
							 DBUS_TYPE_UINT32,&mode,
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

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_UINT32, &port_index,
					DBUS_TYPE_INVALID)) 
	{
		if (NPD_DBUS_ERROR == op_ret ) 
		{
			vty_out(vty,"execute command failed.\n"); 
		}
		else  if((NPD_DBUS_SUCCESS) == op_ret)
		{
			DCLI_DEBUG(("port %d config admin succeed\n",port_index));
		}
		else if (NPD_DBUS_ERROR_UNSUPPORT == op_ret) {
			vty_out(vty,"%% The eth-port wasn't supported media set\n");
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

ALIAS(config_ethport_vct_cmd_func,
	config_ethport_num_vct_cmd,
	"config eth-port PORTNO vct state enable",
	CONFIG_STR
	"Display Ethernet Port information\n"
	CONFIG_ETHPORT_STR 
	"Config eth-port vct information\n"
	"Config eth-port vct state information\n"
	"Specify eth-port vct enable \n"
);

DEFUN(read_ethport_vct_cmd_func,
	read_ethport_vct_cmd,
	"read vct state",
	CONFIG_STR
	"Read eth-port vct information\n"
	"Read eth-port vct state information\n"
	)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;

	int ret = 0,op_ret = 0;
	unsigned int port_index = 0, len = 0;
	unsigned short state = 0;
	unsigned int mode = 0, lengh = 0;
	unsigned char slot_no = 0, port_no = 0;

	if (1 == argc) {
		ret = parse_slotport_no((char*)argv[0], &slot_no, &port_no);
		if(0 != ret) {
			vty_out(vty,"input bad param\n");
			return CMD_WARNING;
		}
	}
	else {
		port_index = (unsigned int)vty->index;
		mode = 1;
	}
	DCLI_DEBUG(("config port_index %d\n",port_index));
	query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,		\
							NPD_DBUS_ETHPORTS_OBJPATH ,	\
							NPD_DBUS_ETHPORTS_INTERFACE ,		\
							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_READ_PORT_VCT);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_BYTE,&slot_no,
							DBUS_TYPE_BYTE,&port_no,
							DBUS_TYPE_UINT32,&port_index,
							DBUS_TYPE_UINT32,&mode,
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

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_UINT32, &port_index,
					DBUS_TYPE_UINT16, &state,
					DBUS_TYPE_UINT32, &len,
					DBUS_TYPE_INVALID)) 
	{
		if (NPD_DBUS_ERROR == op_ret ) 
		{
			vty_out(vty,"execute command failed.\n"); 
		}
		else  if((NPD_DBUS_SUCCESS) == op_ret)
		{
		
			vty_out(vty,"===========================================\n");
			if (mode) {
				vty_out(vty," VCT Information of Ethernet Port.\n"); 
			}
			else {			
				vty_out(vty," VCT Information of Ethernet Port %d-%d.\n", slot_no, port_no); 
			}
			unsigned int i = 0;
			unsigned int j = 0;
			while (j < 4) {
				if (~(state>>i & 0x000f)) {
					vty_out(vty," PAIR-%d state is : ", j);
					if (state>>i & 0x2) {
						vty_out(vty,"open \n");
					}
					else if (state>>i & 0x1) {
						vty_out(vty,"short \n");
					}
					else {
						vty_out(vty,"normal \n");
					}
					lengh = len>>(i*2) & 0x000000ff;
					vty_out(vty," PAIR-%d lengh is : %d m\n", j, lengh);
					i += 4;
					j ++;
				}
			}
/*			
			if(~(state>>4 & 0x000f)) {			
				vty_out(vty," PAIR-1 state is : ");
				if ((state>>4) & 0x2))) {
					vty_out(vty,"open \n");
				}
				else if ((state>>4) & 0x1)) {
					vty_out(vty,"short \n");
				}
				else {
					vty_out(vty,"normal \n");
				}
				lengh = len>>8 & 0x000000ff;
				vty_out(vty," PAIR-1 lengh is : %d m\n", lengh);
			}
			
			if(~(state>>8 & 0x000f)) {				
				vty_out(vty," PAIR-2 state is : ");
				if ((state>>8) & 0x2))) {
					vty_out(vty,"open \n");
				}
				else if ((state>>8) & 0x1)) {
					vty_out(vty,"short \n");
				}
				else {
					vty_out(vty,"normal \n");
				}
				lengh = len>>16 & 0x000000ff;
				vty_out(vty," PAIR-2 lengh is : %d m\n", lengh);
			}
			
			if(~(state>>12 & 0x000f)) {
				vty_out(vty," PAIR-3 state is : ");
				if ((state>>12 & 0x2))) {
					vty_out(vty,"open \n");
				}
				else if ((state>>12 & 0x1)) {
					vty_out(vty,"short \n");
				}
				else {
					vty_out(vty,"normal \n");
				}
				lengh = len>>24 & 0x000000ff;
				vty_out(vty," PAIR-3 lengh is : %d m\n", lengh);
			}
			*/
/*			
			vty_out(vty, "%-10s:%-4d\r\n", "  port", port_index);
			vty_out(vty, "%-10s:%#-4x\r\n","  state", state);
			vty_out(vty, "%-10s:%#-4x\r\n","  len", len);
*/
		}
		else if (NPD_DBUS_ERROR_UNSUPPORT == op_ret) {
			vty_out(vty,"%% The eth-port wasn't supported media set\n");
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

ALIAS(read_ethport_vct_cmd_func,
	read_ethport_num_vct_cmd,
	"read eth-port PORTNO vct state",
	CONFIG_STR
	"Display Ethernet Port information\n"
	CONFIG_ETHPORT_STR 
	"Read eth-port vct information\n"
	"Read eth-port vct state information\n"
);

/*
 * Now instead of "config media preferred". Jia Lihui on 2011.7.11
 */
DEFUN(config_eth_port_media_mode_cmd_func,
	config_eth_port_media_mode_cmd,
	"config media mode (copper|fiber)",
	CONFIG_STR
    "set media mode\n"	
    "set media to copper or fiber\n"
    "set media to copper\n"
    "set media to fiber\n"
	)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int media = 0;
	unsigned int op_ret = 0;
	unsigned int port_index = 0;

	if(0 == strncmp("copper",argv[0],strlen((char*)argv[0]))) {
		media = PHY_MEDIA_MODE_COPPER;
	}
	else if(0 == strncmp("fiber",argv[0],strlen((char*)argv[0]))) {
		media = PHY_MEDIA_MODE_FIBER;
	}
	else {
    	vty_out(vty,"%% Unknow input param\n");
		return CMD_SUCCESS;
	}

	port_index = (unsigned int)vty->index;

	if (is_distributed == DISTRIBUTED_SYSTEM)
	{
    	int slot_no;
    	int local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
		SLOT_PORT_ANALYSIS_SLOT(port_index, slot_no);
		
		query = dbus_message_new_method_call(
							SEM_DBUS_BUSNAME,\
							SEM_DBUS_ETHPORTS_OBJPATH,\
							SEM_DBUS_ETHPORTS_INTERFACE, 
							SEM_DBUS_CONFIG_MEDIA_MODE
			);

		dbus_error_init(&err);

		dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&port_index,
							 DBUS_TYPE_UINT32,&media,
							 DBUS_TYPE_INVALID); 	
		
    	if(NULL == dbus_connection_dcli[slot_no]->dcli_dbus_connection) 				
    	{
    		if(slot_no == local_slot_id) 
    		{
    		    reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
    		}
    		else 
    		{	
    			vty_out(vty, "Connection to slot%d is not exist.\n", slot_no);
    			return CMD_WARNING;
    		}
    	}else {
    		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_no]->dcli_dbus_connection,\
    				query, -1, &err);
    	}

		dbus_message_unref(query);
	
		if (NULL == reply) {
			vty_out(vty,"query reply null\n");
			vty_out(vty,"failed get reply.\n");
			if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_SUCCESS;
		}
	
		if (dbus_message_get_args ( reply, &err,
			DBUS_TYPE_UINT32,&op_ret,
			DBUS_TYPE_INVALID)) 
		{
			if (SEM_COMMAND_SUCCESS == op_ret ) {
				DCLI_DEBUG(("exe success\n"));
				dbus_message_unref(reply);
				return CMD_SUCCESS;
			}
			else if (SEM_OPERATE_NOT_SUPPORT == op_ret) {
				dbus_message_unref(reply);
				vty_out(vty,"%% The eth-port don't support media set\n");
				return CMD_WARNING;
			}
			else if (SEM_WRONG_PARAM == op_ret)
			{
				dbus_message_unref(reply);
				vty_out(vty, "wrong parameters\n");
				return CMD_WARNING;
			}
			else if (SEM_COMMAND_NOT_SUPPORT == op_ret)
			{
				// TODO: call npd dbus to process the command.		
				DCLI_DEBUG(( "call npd dbus process\n"));
				dbus_message_unref(reply);
			}
			else if (SEM_COMMAND_FAILED == op_ret) {
				dbus_message_unref(reply);
				vty_out(vty,"%% Error product type\n");
				return CMD_FAILURE;
			}
			else 
			{
				dbus_message_unref(reply);
				vty_out(vty,"%% Config media error\n");
				return CMD_WARNING;
			}
			
		} else {
			vty_out(vty,"Failed get args.\n");
			if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
		    dbus_message_unref(reply);
			return CMD_WARNING;
		}
    	query = dbus_message_new_method_call(
    					NPD_DBUS_BUSNAME,				\
    					NPD_DBUS_ETHPORTS_OBJPATH,		\
    					NPD_DBUS_ETHPORTS_INTERFACE,		\
    					NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_ETHPORT_MEDIA);
    	
    	dbus_error_init(&err);

    	dbus_message_append_args(query,
    							 DBUS_TYPE_UINT32,&port_index,
    							 DBUS_TYPE_UINT32,&media,
    							 DBUS_TYPE_INVALID);

    	if(NULL == dbus_connection_dcli[slot_no]->dcli_dbus_connection) 				
    	{
    		if(slot_no == local_slot_id)
    		{
    		    reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
    		}
    		else 
    		{	
    			vty_out(vty, "Connection to slot%d is not exist.\n", slot_no);
    			return CMD_WARNING;
    		}
    	}else {
    		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_no]->dcli_dbus_connection,\
    				query, -1, &err);
    	}
    	
    	dbus_message_unref(query);
    	
    	if (NULL == reply) {
    		vty_out(vty,"query reply null\n");
    		vty_out(vty,"failed get reply.\n");
    		if (dbus_error_is_set(&err)) {
    			vty_out(vty,"%s raised: %s",err.name,err.message);
    			dbus_error_free_for_dcli(&err);
    		}
    		return CMD_SUCCESS;
    	}
    	
    	if (dbus_message_get_args ( reply, &err,
    		DBUS_TYPE_UINT32,&op_ret,
    		DBUS_TYPE_INVALID)) {
    			if (ETHPORT_RETURN_CODE_ERR_NONE == op_ret ) {
    				DCLI_DEBUG(("exe success\n"));
    			}
    			else if (ETHPORT_RETURN_CODE_UNSUPPORT == op_ret) {
    				vty_out(vty,"%% The eth-port don't support media set\n");
    			}
    			else if (ETHPORT_RETURN_CODE_NOT_SUPPORT == op_ret) {
    				vty_out(vty,"%% The eth-port don't support media set 'none'.\n");
    			}
    			else if (ETHPORT_RETURN_CODE_ERR_GENERAL == op_ret) {
    				vty_out(vty,"%% general error such as device or port out of range and etc.");
    			}
    			else if (ETHPORT_RETURN_CODE_ERR_PRODUCT_TYPE == op_ret) {
    				vty_out(vty,"%% Error product type\n");
    			}
    			else 
    				vty_out(vty,"%% Config media error\n");
    		
    	} else {
    		vty_out(vty,"Failed get args.\n");
    		if (dbus_error_is_set(&err)) {
    			vty_out(vty,"%s raised: %s",err.name,err.message);
    			dbus_error_free_for_dcli(&err);
    		}
    	}
    	
    	dbus_message_unref(reply);
    	return CMD_SUCCESS;
	}

	query = dbus_message_new_method_call(
					NPD_DBUS_BUSNAME,				\
					NPD_DBUS_ETHPORTS_OBJPATH,		\
					NPD_DBUS_ETHPORTS_INTERFACE,		\
					NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_ETHPORT_MEDIA);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&port_index,
							 DBUS_TYPE_UINT32,&media,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		vty_out(vty,"query reply null\n");
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)) {
			if (ETHPORT_RETURN_CODE_ERR_NONE == op_ret ) {
				DCLI_DEBUG(("exe success\n"));
			}
			else if (ETHPORT_RETURN_CODE_UNSUPPORT == op_ret) {
				vty_out(vty,"%% The eth-port don't support media set\n");
			}
			else if (ETHPORT_RETURN_CODE_NOT_SUPPORT == op_ret) {
				vty_out(vty,"%% The eth-port don't support media set 'none'.\n");
			}
			else if (ETHPORT_RETURN_CODE_ERR_GENERAL == op_ret) {
				vty_out(vty,"%% general error: Do not support this CMD or (device,port) out of range and etc.");
			}
			else if (ETHPORT_RETURN_CODE_ERR_PRODUCT_TYPE == op_ret) {
				vty_out(vty,"%% Error product type\n");
			}
			else 
				vty_out(vty,"%% Config media error\n");
		
	} else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
/* add end */

DEFUN(sync_master_eth_port_info,
	sync_master_eth_port_info_cmd,
	"sync master eth_port info",
	"sync master eth_port information\n"
	"sync master eth_port information\n"
	"sync master eth_port information\n")
{
	DBusMessage *query, *reply;
	DBusError err;
	int ret = 0;
    int slot_id =0;  
	int master_slot_id[2] = {-1, -1};
	int standby_master_exist = 1;
	int standby_master_slot_id = 0;
	int update_active_or_standby = 0;
	int slot_count = get_product_info(SEM_SLOT_COUNT_PATH);
    int local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
    int master_slot_count = get_product_info(SEM_MASTER_SLOT_COUNT_PATH);
	int active_master_slot_id = get_product_info(SEM_ACTIVE_MASTER_SLOT_ID_PATH);
	
    if((local_slot_id < 0) || (slot_count < 0) || \
	   (master_slot_count < 0) || (active_master_slot_id <0))
    {
        vty_out(vty,"get_product_info() return -1,Please check dbm file !\n");
    	return CMD_WARNING;			
    }
	
	ret = dcli_master_slot_id_get(master_slot_id);
	if(ret !=0 )
	{
		vty_out(vty,"get master_slot_id error !\n");
		return CMD_WARNING;		
   	}

	if(local_slot_id == master_slot_id[0] || local_slot_id == master_slot_id[1])
	{
	}
	else
	{
		vty_out(vty,"Only master board can Execute this command!\n");
		return CMD_SUCCESS;	
	}
	if(active_master_slot_id == master_slot_id[0])
	{
		standby_master_slot_id = master_slot_id[1];
	}
	else
	{
		standby_master_slot_id = master_slot_id[0];
	}
	/*update active master eth-port information*/
	for(slot_id = 1; slot_id <= slot_count; slot_id++)
	{
        query = dbus_message_new_method_call(
							SEM_DBUS_BUSNAME,                \
							SEM_DBUS_ETHPORTS_OBJPATH,       \
							SEM_DBUS_ETHPORTS_INTERFACE,     \
							SEM_DBUS_SYNC_MASTER_ETH_PORT_INFO
		);
		dbus_error_init(&err);
		dbus_message_append_args(query,
			                 DBUS_TYPE_UINT32,&update_active_or_standby,
							 DBUS_TYPE_INVALID); 	
		
        if(NULL == dbus_connection_dcli[slot_id]->dcli_dbus_connection) 				
        {
            if(slot_id == local_slot_id)
            {
                reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
            }
            else 
    	    {	
    		    vty_out(vty,"Can not connect to slot:%d \n",slot_id);
				if(standby_master_slot_id == slot_id)
					standby_master_exist = 0;
    		    continue;
    	    }	
        }
        else
        {
            reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,query,-1, &err);				
        }
		
		dbus_message_unref(query);
	    if (NULL == reply){
		    vty_out(vty,"Dbus reply==NULL, Please check on slot: %d\n",slot_id);
		    return CMD_SUCCESS;
	    }
        if (dbus_message_get_args ( reply, &err,
        					    DBUS_TYPE_UINT32, &ret,
        					    DBUS_TYPE_INVALID)) 
        {
            if(ret == SEM_COMMAND_SUCCESS)
			{
			    vty_out(vty,"Update slot%d eth_port information success.\n", slot_id);
		    }
        } 
        else 
        {
        	vty_out(vty,"% Failed get args.\n");
        	if (dbus_error_is_set(&err)) 
        	{
        		vty_out(vty,"% %s raised: %s",err.name,err.message);
        		dbus_error_free_for_dcli(&err);
        	}
        }
        dbus_message_unref(reply);
    }
	/*update standby master eth-port information form active master*/
	if(standby_master_exist)
	{
	    update_active_or_standby = 1;
	    query = dbus_message_new_method_call(
							SEM_DBUS_BUSNAME,                \
							SEM_DBUS_ETHPORTS_OBJPATH,       \
							SEM_DBUS_ETHPORTS_INTERFACE,     \
							SEM_DBUS_SYNC_MASTER_ETH_PORT_INFO
		);
		dbus_error_init(&err);
		dbus_message_append_args(query,
			                 DBUS_TYPE_UINT32,&update_active_or_standby,
							 DBUS_TYPE_INVALID); 	
		if(local_slot_id == active_master_slot_id)
		{
			reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
		}
		else
		{
			reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[active_master_slot_id]->dcli_dbus_connection,query,-1, &err);
		}
		
		dbus_message_unref(query);
	    if (NULL == reply){
		    vty_out(vty,"Dbus reply==NULL, Please check on slot: %d\n",slot_id);
		    return CMD_SUCCESS;
	    }
        if (dbus_message_get_args ( reply, &err,
        					    DBUS_TYPE_UINT32, &ret,
        					    DBUS_TYPE_INVALID)) 
        {
            if(ret == SEM_COMMAND_SUCCESS)
			{
		    }
        } 
        else 
        {
        	vty_out(vty,"% Failed get args.\n");
        	if (dbus_error_is_set(&err)) 
        	{
        		vty_out(vty,"% %s raised: %s",err.name,err.message);
        		dbus_error_free_for_dcli(&err);
        	}
        }
        dbus_message_unref(reply);
	}
	return CMD_SUCCESS;
}

DEFUN(diagnosis_read_port_rate_cmd_func,
		diagnosis_read_port_rate_cmd,
		"show eth-port PORTNO rate",
		SHOW_STR
		"Display Ethernet Port information\n"
		CONFIG_ETHPORT_STR 
		"Display Ethernet Port rate information\n"
		"Get times\n"
) 
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	DBusMessageIter	 iter;
	
	unsigned char slot_no = 0, port_no = 0;
	unsigned int ret = 0;
	unsigned long long bitsumrecv = 0,packetsumrecv = 0;
	unsigned long long bitsumsent = 0,packetsumsent = 0;
	unsigned char type = 1;	
	unsigned int value = 0;
	
	int local_slot_id = get_product_info("/dbm/local_board/slot_id");	

	ret = parse_slotport_no((char *)argv[0],&slot_no,&port_no);
	if (NPD_FAIL == ret) {
    	vty_out(vty,"Unknow portno format.\n");
		return CMD_WARNING;
	}	
	
	if (is_distributed == DISTRIBUTED_SYSTEM)	
	{
		int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
		if(slot_no > slotNum || slot_no <= 0)
		{
			vty_out(vty,"%% NO SUCH SLOT %d!\n", slot_no);
            return CMD_WARNING;
		}
	}
	
	value = slot_no;
	value =  (value << 8) |port_no;
	value = SLOT_PORT_COMBINATION(slot_no, port_no);
	
	query = dbus_message_new_method_call(
							   NPD_DBUS_BUSNAME,
							   NPD_DBUS_OBJPATH,
							   NPD_DBUS_INTERFACE,
							   NPD_DBUS_ETHPORTS_INTERFACE_METHOD_GET_PORT_RATE);
	dbus_error_init(&err);



	/* opType is no use */
	dbus_message_append_args(	query,
								DBUS_TYPE_BYTE,&slot_no,
								DBUS_TYPE_BYTE,&port_no,	
								DBUS_TYPE_BYTE,&type,	
								DBUS_TYPE_UINT32,&value,
							 	DBUS_TYPE_INVALID);

    if(NULL == dbus_connection_dcli[slot_no]->dcli_dbus_connection) 				
	{
		if(slot_no == local_slot_id)
		{
            reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
		}
		else 
		{	
			vty_out(vty, "Connection to slot%d is not exist.\n", slot_no);
			return CMD_WARNING;
		}
    }
	else
	{
        reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_no]->dcli_dbus_connection,query,-1, &err);				
	}


	dbus_message_unref(query);
	if (NULL == reply) {		
		vty_out(vty,"failed get reply.\n"); 
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}
	
	dbus_message_iter_init(reply,&iter);	
		
	dbus_message_iter_get_basic(&iter,&bitsumrecv);
	dbus_message_iter_next(&iter);	

	
	dbus_message_iter_get_basic(&iter,&packetsumrecv);
	dbus_message_iter_next(&iter);	

	dbus_message_iter_get_basic(&iter,&bitsumsent);
	dbus_message_iter_next(&iter);	

	dbus_message_iter_get_basic(&iter,&packetsumsent);
	dbus_message_iter_next(&iter);	
	
	dbus_message_iter_get_basic(&iter,&ret);
	dbus_message_iter_next(&iter);	
	
	dbus_message_unref(reply);


	if(ETHPORT_RETURN_CODE_UNSUPPORT == ret)
	{
		vty_out(vty,"port no support\n");
		return CMD_WARNING;
	}
	else
	{
		vty_out(vty, "-------------------- RX ------------------\n");	
		vty_out(vty, "%-20s %-15lld\n","bitRcv/s", bitsumrecv/3 * 8);
		vty_out(vty, "%-20s %-15lld\n","byteRcv/s", bitsumrecv/3 );
		vty_out(vty, "%-20s %-15lld\n","packRcv/s", packetsumrecv/3);
		vty_out(vty, "-------------------- TX ------------------\n");		
		vty_out(vty, "%-20s %-15lld\n","bitSent/s", bitsumsent/3 * 8);
		vty_out(vty, "%-20s %-15lld\n","byteSent/s", bitsumsent/3 );
		vty_out(vty, "%-20s %-15lld\n","packSent/s", packetsumsent/3);
			
		/* end */
		return CMD_SUCCESS;  
	}
}


void dcli_eth_port_init(void) {
	install_node (&eth_port_node, dcli_eth_port_show_running_config, "ETH_PORT_NODE");
	install_default(ETH_PORT_NODE);
	
	install_element(VIEW_NODE,&show_ethport_list_cmd);
	install_element(VIEW_NODE,&show_ethport_cn_attr_cmd);
	//install_element(VIEW_NODE,&show_ethport_cn_ipg_cmd);
	install_element(VIEW_NODE,&show_ethport_cn_stat_cmd);
	install_element(VIEW_NODE,&clear_ethport_cn_stat_cmd);
	install_element(VIEW_NODE,&show_ethport_arp_cn_cmd);
	install_element(VIEW_NODE,&show_ethport_nexthop_cn_cmd);	
	install_element(ENABLE_NODE,&show_ethport_list_cmd);
	/* Added by Jia Lihui for show eth-port list on single slot. 2011.7.17 */
    install_element(ENABLE_NODE,&show_ethport_slotx_list_cmd);
	install_element(ENABLE_NODE,&show_ethport_cn_attr_cmd);
	//install_element(ENABLE_NODE,&show_ethport_cn_ipg_cmd);
	install_element(ENABLE_NODE,&show_ethport_cn_stat_cmd);
	install_element(ENABLE_NODE,&clear_ethport_cn_stat_cmd);
	install_element(ENABLE_NODE,&show_ethport_arp_cn_cmd);
	install_element(ENABLE_NODE,&show_ethport_nexthop_cn_cmd);
	install_element(ENABLE_NODE, &sync_master_eth_port_info_cmd);
	
   /*install_element(ENABLE_NODE,&config_interface_ethport_cmd);
	//install_element(ENABLE_NODE,&config_no_interface_ethport_cmd);*/
	install_element(CONFIG_NODE,&show_ethport_list_cmd);
    /* Added by Jia Lihui for show eth-port list on single slot. 2011.7.17 */
    install_element(CONFIG_NODE,&show_ethport_slotx_list_cmd);
	install_element(CONFIG_NODE,&show_ethport_cn_attr_cmd);
	install_element(CONFIG_NODE,&show_ethport_cn_stat_cmd);
	install_element(CONFIG_NODE,&clear_ethport_cn_stat_cmd);
	/*install_element(CONFIG_NODE,&config_port_mode_cmd);*/
	install_element(CONFIG_NODE,&config_ethernet_port_cmd);
	/* add by yinlm@autelan.com for queue wrr and sp*/
	install_element(CONFIG_NODE,&config_buffer_mode_cmd);
	install_element(CONFIG_NODE,&clear_ethport_arp_cn_cmd);
	install_element(CONFIG_NODE,&show_ethport_arp_cn_cmd);
	//install_element(CONFIG_NODE,&show_ethport_cn_ipg_cmd);
	
	install_element(CONFIG_NODE,&show_ethport_nexthop_cn_cmd);
	/* add by yinlm@autelan.com for queue wrr and sp*/
	install_element(CONFIG_NODE,&show_buffer_mode_cmd);
	
	install_element(CONFIG_NODE,&config_storm_control_global_model_cn_cmd);
	install_element(CONFIG_NODE,&config_port_storm_control_pps_cn_cmd);
	install_element(CONFIG_NODE,&config_port_storm_control_bps_cn_cmd);

	install_element(ETH_PORT_NODE,&config_ethport_admin_cmd);
	install_element(ETH_PORT_NODE,&config_ethport_ipg_cmd);
	install_element(ETH_PORT_NODE,&show_ethport_ipg_cmd);
	
	install_element(ETH_PORT_NODE,&config_ethport_speed_cmd);
	install_element(ETH_PORT_NODE,&config_ethport_autonegt_cmd);
	install_element(ETH_PORT_NODE,&config_ethport_autonegt_speed_cmd);
	install_element(ETH_PORT_NODE,&config_ethport_autonegt_duplex_cmd);
	install_element(ETH_PORT_NODE,&config_ethport_autonegt_flowcontrol_cmd);
	install_element(ETH_PORT_NODE,&config_ethport_duplex_mode_cmd);
	install_element(ETH_PORT_NODE,&config_ethport_flow_control_cmd);
	install_element(ETH_PORT_NODE,&config_ethport_back_pressure_cmd);
	/*install_element(ETH_PORT_NODE,&config_ethport_link_state_cmd);*/
	install_element(ETH_PORT_NODE,&config_ethport_mtu_cmd);
	install_element(ETH_PORT_NODE,&config_ethport_default_cmd);
	install_element(ETH_PORT_NODE,&show_ethport_en_attr_cmd);
	install_element(ETH_PORT_NODE,&show_ethport_en_stat_cmd);
	install_element(ETH_PORT_NODE,&clear_ethport_en_stat_cmd);
	/* Added by Jia Lihui for distinguish both of media configurations. 2011.7.17 */
	install_element(ETH_PORT_NODE,&config_eth_port_media_preferred_cmd);
	install_element(ETH_PORT_NODE,&config_eth_port_media_mode_cmd);
	install_element(ETH_PORT_NODE,&clear_ethport_arp_en_cmd);
	install_element(ETH_PORT_NODE,&show_ethport_arp_en_cmd);
	install_element(ETH_PORT_NODE,&show_ethport_nexthop_en_cmd);
	install_element(ETH_PORT_NODE,&config_port_storm_control_pps_en_cmd);
	install_element(ETH_PORT_NODE,&config_port_storm_control_bps_en_cmd);
	/*install_element(CONFIG_NODE,&config_no_interface_ethport_cmd);*/
	install_element(HIDDENDEBUG_NODE,&config_ethport_link_state_cmd);
	install_element(HIDDENDEBUG_NODE,&show_xg_ethport_cn_stat_cmd);
	/*add by yinlm for port oct*/	
	install_element(ETH_PORT_NODE,&config_ethport_vct_cmd);
	install_element(CONFIG_NODE,&config_ethport_num_vct_cmd);
	install_element(ETH_PORT_NODE,&read_ethport_vct_cmd);
	install_element(CONFIG_NODE,&read_ethport_num_vct_cmd);
	install_element(CONFIG_NODE, &diagnosis_read_port_rate_cmd);	
	install_element(ENABLE_NODE, &diagnosis_read_port_rate_cmd);
}

#ifdef __cplusplus
}
#endif
