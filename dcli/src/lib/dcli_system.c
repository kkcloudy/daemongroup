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
* dcli_system.c
*
*
* CREATOR:
*		qinhs@autelan.com
*
* DESCRIPTION:
*		CLI definition for system configuration.
*
* DATE:
*		02/21/2008	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.71 $	
*******************************************************************************/
#ifdef __cplusplus
	extern "C"
	{
#endif
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/mman.h> /*wangchao add for mmap*/
#include <fcntl.h>
#include <unistd.h>
#include <zebra.h>
#include <dbus/dbus.h>

#include "sysdef/npd_sysdef.h"
#include "sysdef/returncode.h"
#include "dbus/npd/npd_dbus_def.h"
#include "dbus/sem/sem_dbus_def.h"

#include "command.h"
#include "if.h"
#include "vtysh/vtysh.h"//huangjing

#include "util/npd_list.h"
#include "npd/nam/npd_amapi.h"
#include "board/board_define.h"
#include "dcli_system.h"
#include "dcli_vlan.h"
#include "dcli_main.h"

extern DBusConnection *dcli_dbus_connection;
extern char *slot_status_str[MODULE_STAT_MAX];
extern int is_distributed;

#define DISPLAY_VERSTR_FILE "/etc/version/verstring"
#define DEVINFO_SN "/devinfo/sn"
#define NULL_SN "000000000000000000000"
#define DEVINFO_SW_NAME "/devinfo/software_name"
#define DEVINFO_PRODUCT_NAME "/devinfo/product_name"
#define DEVINFO_MAC "/devinfo/mac"
#define DEVINFO_BASE_MAC "/devinfo/base_mac"

#define SW_VER_FILE "/etc/version/version"
#define SW_BUILDNO_FILE "/etc/version/buildno"
#define SW_PRODUCTS_FILE "/etc/version/products"
#define SW_SHOWCODE_FILE "/etc/version/showcode"
#define MAX_NAME_LEN 256

char *power_state_str[4][3] = {
	{"NONE","OFF","ON"},
	{"NONE","OFF","ON"},
	{"NONE","OFF","ON"},
    {"NONE","OFF","ON"}
};

char *fan_rpm_str[] = {
	"%12.5","%25","%37.5","%50",
	"%62.5","%75","%87.5","%100",
};

int show_file_string(char *FILENAME,struct vty* vty)
{
	char buff[MAX_NAME_LEN];
	int len,i,fd;
	
	fd = open(FILENAME,O_RDONLY);
	if (fd < 0) {
		//vty_out(vty,"\r\nFailed to open file.\r\n");
		return 1;
	}	
	len = read(fd,buff,MAX_NAME_LEN);	
	if (len < 0) {
		vty_out(vty,"\r\nFailed to read file.\r\n");
		return 1;
	}	
	for (i=0;i<len; i++) {
		if (buff[i] != '\n') vty_out(vty,"%c",buff[i]);
	}	
	close(fd);
	return 0;

}


int show_sn_string(struct vty* vty)
{
	char buff[MAX_NAME_LEN];
	int len,i,fd;

	
	fd = open(DEVINFO_SN,O_RDONLY);
	if (fd < 0) {
		return 1;
	}	
	len = read(fd,buff,MAX_NAME_LEN);	
	if (len < 0) {
		return 1;
	}	
	if (strncmp(buff,NULL_SN,21)) {
		vty_out(vty,"Serial No:");
		for (i=0;i<len; i++) {
			if (buff[i] != '\n') vty_out(vty,"%c",buff[i]);
		}	
		vty_out(vty,"\r\n");
	}
	close(fd);

	return 0;

}


DEFUN(show_sys_version_cmd_func,
	show_sys_version_cmd,
	"show system version",
	SHOW_STR
	"Display system information\n"
	"Display system version information\n"
	)
{
  if (show_file_string(DEVINFO_SW_NAME,vty)) return CMD_WARNING;
  vty_out(vty," ");

  if (show_file_string(DISPLAY_VERSTR_FILE,vty)) return CMD_WARNING;
  
  vty_out(vty," running on ");

  if (show_file_string(DEVINFO_PRODUCT_NAME,vty)) return CMD_WARNING;

  vty_out(vty,"\r\nProudct Base MAC:");

  if (is_distributed) {
  	if (show_file_string(DEVINFO_BASE_MAC,vty)) return CMD_WARNING;
  }
  else {
  	if (show_file_string(DEVINFO_MAC,vty)) return CMD_WARNING;
  }

  vty_out(vty,"\r\n");

 // show_sn_string(vty);

  return CMD_SUCCESS;
}

void show_internal_code(struct vty* vty)
{
	DBusMessage *query, *reply;
	DBusError err;


	unsigned int product_id;
	unsigned int sw_version;
	char *product_name = NULL;
	char *base_mac = NULL;
	char *serial_no = NULL;
	char *swname = NULL;

	vty_out(vty,"\r\nIC: ");

	if (!show_file_string(SW_SHOWCODE_FILE,vty)) {
		vty_out(vty,"\r\n");
		return ;
	} else {
		
	}
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_OBJPATH,NPD_DBUS_INTERFACE,NPD_DBUS_INTERFACE_METHOD_VER);

	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return ;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&product_id,
		DBUS_TYPE_UINT32,&sw_version,
		DBUS_TYPE_STRING,&product_name,
		DBUS_TYPE_STRING,&base_mac,
		DBUS_TYPE_STRING,&serial_no,
		DBUS_TYPE_STRING,&swname,
		DBUS_TYPE_INVALID)) {
		/* vty_out(vty,"product id [%d]\n",product_id);
            Product Id could be used to identify product when there is many products and OEM products
		*/
/*		vty_out(vty,"\n%s v%d.%d.%d(Build%0.3d)",
				swname,
				SW_MAJOR_VER(sw_version),
				SW_MINOR_VER(sw_version),
				SW_COMPATIBLE_VER(sw_version),
				SW_BUILD_VER(sw_version));
		vty_out(vty," running on %s\n\n",product_name);
		vty_out(vty,"Product Base MAC Addrress: %s\n",base_mac);
	*/	
		vty_out(vty,"%s",serial_no);
	} else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
				printf("%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
		}
	}
		
	dbus_message_unref(reply);

	vty_out(vty,"#");
	if (show_file_string(SW_VER_FILE,vty)) return ;
	vty_out(vty,".");
	if (show_file_string(SW_BUILDNO_FILE,vty)) return ;
	vty_out(vty,".");
	if (show_file_string(SW_PRODUCTS_FILE,vty)) return ;
	vty_out(vty,"\r\n");


	return ;

}

DEFUN(show_sys_hwconf_cmd_func,
	show_sys_hwconf_cmd,
	"show system hardware-configuration",
	SHOW_STR
	"Display system information\n"
	"Display system hardware-configuration information\n"
	)
{
    if(is_distributed == DISTRIBUTED_SYSTEM)
    {
        int i = 0;
        int ret = -1;
        unsigned char slot_count = 0;
        unsigned char slot_num   = 0;
        unsigned char board_state = 0;
        unsigned char is_active_master = 0;
        char board_name[16] = {'\0'};
        char path_buf[100];
        
        slot_count = get_int_from_file("/dbm/product/slotcount");        
        vty_out(vty,"SLOT\t");
        vty_out(vty,"STATUS\t\t");
        vty_out(vty,"IS_MASTER\t");
        vty_out(vty,"BOARD_NAME\t");
        vty_out(vty,"\n----------------------------------------------------\n");
        
        for (i = 0; i < slot_count; i++) 
        {
            slot_num = i+1;
            /* slot */
            vty_out(vty," %d\t", slot_num);
            
            /* status */
            sprintf(path_buf, "/dbm/product/slot/slot%d/board_state", slot_num);
            ret = get_int_from_file(path_buf);
            if(-1 == ret)
            {
                vty_out(vty," %s  read error! ", path_buf);
                return CMD_SUCCESS;
            }
            board_state = ret;
            switch(board_state)
            {
				case BOARD_INSERTED_AND_REMOVED:
					vty_out(vty, "REMOVED\t\t");
					break;
                case BOARD_EMPTY:
                    vty_out(vty,"EMPTY\t\t");
                    break;
				case BOARD_REMOVED:
					vty_out(vty, "REMOVED\t\t");
					break;
                case BOARD_INSERTED:
                    vty_out(vty,"INSERTED\t\t");
                    break;
                case BOARD_INITIALIZING:
                    vty_out(vty,"INITIALIZING\t\t");
                    break;
                case BOARD_READY:
                    vty_out(vty,"READY\t\t");
                    break;
                case BOARD_RUNNING:
                    vty_out(vty,"RUNNING\t\t");
                    break;
            }

            /* master */
            sprintf(path_buf, "/dbm/product/slot/slot%d/is_active_master", slot_num);
            ret = get_int_from_file(path_buf);
            if(-1 == ret)
            {
                vty_out(vty," %s  read error! ", path_buf);
                return CMD_SUCCESS;
            }
            is_active_master = ret;
            if(is_active_master)
                vty_out(vty,"YES\t\t");
            else
                vty_out(vty,"NO\t\t");

            /* name */
            sprintf(path_buf, "/dbm/product/slot/slot%d/name", slot_num);
            ret = get_str_from_file(path_buf, board_name);
            if(-1 == ret)
            {
                vty_out(vty," %s  read error! ", path_buf);
                return CMD_SUCCESS;
            }
            vty_out(vty,"%s\t", board_name);

            /* Next line */
            vty_out(vty,"\n");
        }

        show_internal_code(vty);
        return CMD_SUCCESS;
    }
    
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	unsigned char slot_count = 0;
	int i;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_OBJPATH,NPD_DBUS_INTERFACE,NPD_DBUS_INTERFACE_METHOD_HWCONF);
	

	dbus_error_init(&err);
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

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&slot_count);

	if (1 == slot_count) {
		vty_out(vty,"%-21s","PRODCT");
	} else {
    	vty_out(vty,"%-5s%-10s%-20s","SLOT","STATUS","MODULE TYPE");
	}
	vty_out(vty,"%27s %-12s\n","ICM","EXT SLOTNUM");
	if (1 == slot_count) {
		vty_out(vty,"%-21s%27s %-12s\n","--------------------","---------------------------","-----------");
	}
	else
		vty_out(vty,"%-5s%-10s%-20s%27s %-12s\n","----","---------","-------------------","---------------------------","-----------");
	
	dbus_message_iter_next(&iter);
	
	dbus_message_iter_recurse(&iter,&iter_array);

	for (i = 0; i < slot_count; i++) {
		DBusMessageIter iter_struct;
		unsigned char slotno;
		unsigned int module_id;
		unsigned char module_status;
		unsigned char hw_ver;
		unsigned char ext_slot_num;
		char *sn;
		char *modname;
		
		
		dbus_message_iter_recurse(&iter_array,&iter_struct);
		
		dbus_message_iter_get_basic(&iter_struct,&slotno);
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&module_id);
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&module_status);
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&hw_ver);
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&ext_slot_num);
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&sn);
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&modname);
		
		dbus_message_iter_next(&iter_array);

		if(MODULE_ID_AX7I_XG_CONNECT == module_id){
				 continue;/*when the module is MODULE_ID_AX7I_XG_CON	 NECT,don't show this hardware --yangxs 10/05/26*/
		 }

		if (1 == slot_count) {
			/*			vty_out(vty," %-20s",modname); */
			if (show_file_string(DEVINFO_PRODUCT_NAME,vty)) {
				dbus_message_unref(reply);
				return CMD_WARNING;
			}
		} else {
			vty_out(vty," %-4d%-10s%-20s",slotno,slot_status_str[module_status],(MODULE_STAT_NONE != module_status) ? modname:"-");
		}
		if(MODULE_STAT_NONE == module_status) {
			vty_out(vty,"%27s %-11s\n","-","-","-");
		}
		else {
			vty_out(vty,"%20s##%02d.%02d %-11d\n",(MODULE_STAT_NONE != module_status) ? sn:"-",HW_PCB_VER(hw_ver),HW_CPLD_VER(hw_ver),ext_slot_num);
		}
	}
		

	dbus_message_unref(reply);
	
	show_internal_code(vty);
	return CMD_SUCCESS;
}

DEFUN(show_sys_environment_cmd_func,
	show_sys_environment_cmd,
	"show system environment",
	SHOW_STR
	"show system  \n"
	"environment include fan power and temperature\n"
	)
{
	DBusMessage *query, *reply;
	DBusError err;
    unsigned int i;
	unsigned char fan_power=0;
	unsigned short core_tmprt=0;
	unsigned short surface_tmprt=0;
	unsigned int ok;
	int ret = NPD_SUCCESS;
	
	if(is_distributed == DISTRIBUTED_SYSTEM)
	{
	       unsigned int fan_state=0, fan_rpm = 0;
		   unsigned int power_state = 0;
	       int master_remote_temp = 0;
	       int master_inter_temp = 0;
	       int slave_remote_temp = 0;
	       int slave_inter_temp = 0;
		   unsigned int is_active_master, cpu_num, board_id;
			
	       query = dbus_message_new_method_call(
	            		SEM_DBUS_BUSNAME,
	            		SEM_DBUS_OBJPATH,
	            		SEM_DBUS_INTERFACE,
	            		SEM_DBUS_SHOW_SYSTEM_ENVIRONMENT);
	    	
	       dbus_error_init(&err);


		   for (i = 0;i < MAX_SLOT; i++ ) 
		   {
				if (NULL != dbus_connection_dcli[i]->dcli_dbus_connection)
				{
					reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[i]->dcli_dbus_connection,query,-1, &err);
	
                  	if (NULL == reply) {
                  		vty_out(vty,"failed get reply.\n");
                  		if (dbus_error_is_set(&err)) {
                  			printf("%s raised: %s",err.name,err.message);
                  			dbus_error_free_for_dcli(&err);
                  		}
                  		return CMD_SUCCESS;
                  	}
              
              
              
                  	if (dbus_message_get_args ( reply, &err,
              			                 DBUS_TYPE_UINT32,&is_active_master,
              			                 DBUS_TYPE_UINT32,&cpu_num,
              			                 DBUS_TYPE_UINT32,&board_id,
              							 DBUS_TYPE_UINT32,&fan_state,
              							 DBUS_TYPE_UINT32,&fan_rpm,
              							 DBUS_TYPE_UINT32,&power_state,
              							 DBUS_TYPE_INT32,&master_inter_temp,
              							 DBUS_TYPE_INT32,&master_remote_temp,
              							 DBUS_TYPE_INT32,&slave_inter_temp,
              							 DBUS_TYPE_INT32,&slave_remote_temp,
                  		                 DBUS_TYPE_INVALID)) {
                  			/* show detailed message below*/
                  	}else {
                  		vty_out(vty,"Failed get args.\n");
                  		if (dbus_error_is_set(&err)) {
                  				printf("%s raised: %s",err.name,err.message);
                  				dbus_error_free_for_dcli(&err);
                  		}
                  		return CMD_WARNING;
                  	}
              
              
              		if(is_active_master)
              		{	
              		
                      	vty_out(vty,"+---------------+-----------------------------------+\n");
                      	vty_out(vty,"|     %-4s      |   %-4s |   %-4s |   %-4s |   %-4s |\n"," ","1","2","3","4" );
                      	vty_out(vty,"|  %-11s  |--------+--------+--------+--------|\n","POWER STATE");
                      	vty_out(vty,"|     %-4s      |  %-4s  |  %-4s  |  %-4s  |  %-4s  |\n"," ",power_state_str[0][power_state&0xf],
                  			                                                                     power_state_str[1][(power_state>>4)&0xf],
                  			                                                                     power_state_str[2][(power_state>>8)&0xf],
                  			                                                                     power_state_str[3][(power_state>>12)&0xf]);
                      	vty_out(vty,"+---------------+-----------------------------------|\n");
                      	vty_out(vty,"|     %-4s      |      %-5s      |       %-4s      |\n"," ","STATE","RPM" );
                      	vty_out(vty,"|  %-9s    |-----------------+-----------------|\n","FAN STATE");
                      	vty_out(vty,"|     %-4s      |      %-6s     |       %-5s     |\n"," ",fan_state>0 ? "NORMAL":"ALARM", fan_rpm_str[fan_rpm]);
						vty_out(vty,"+---------------+-----------------------------------+\n");
              		}                  		
                  	dbus_message_unref(reply);
                  	      
           		}
			}
	
		   for (i = 0;i < MAX_SLOT; i++ ) 
		   {
				if (NULL != dbus_connection_dcli[i]->dcli_dbus_connection)
				{
					reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[i]->dcli_dbus_connection,query,-1, &err);
	
                  	if (NULL == reply) {
                  		vty_out(vty,"failed get reply.\n");
                  		if (dbus_error_is_set(&err)) {
                  			printf("%s raised: %s",err.name,err.message);
                  			dbus_error_free_for_dcli(&err);
                  		}
                  		return CMD_SUCCESS;
                  	}
              
              
              
                  	if (dbus_message_get_args ( reply, &err,
              			                 DBUS_TYPE_UINT32,&is_active_master,
              			                 DBUS_TYPE_UINT32,&cpu_num,
              			                 DBUS_TYPE_UINT32,&board_id,
              							 DBUS_TYPE_UINT32,&fan_state,
              							 DBUS_TYPE_UINT32,&fan_rpm,
              							 DBUS_TYPE_UINT32,&power_state,
              							 DBUS_TYPE_INT32,&master_inter_temp,
              							 DBUS_TYPE_INT32,&master_remote_temp,
              							 DBUS_TYPE_INT32,&slave_inter_temp,
              							 DBUS_TYPE_INT32,&slave_remote_temp,
                  		                 DBUS_TYPE_INVALID)) {
                  			/* show detailed message below*/
                  	}else {
                  		vty_out(vty,"Failed get args.\n");
                  		if (dbus_error_is_set(&err)) {
                  				printf("%s raised: %s",err.name,err.message);
                  				dbus_error_free_for_dcli(&err);
                  		}
                  		return CMD_WARNING;
                  	}
               #if 0  
              		if(is_master)
              		{	
              		
                      	vty_out(vty,"+---------------+-----------------------------------+\n");
                      	vty_out(vty,"|     %-4s      |   %-4s |   %-4s |   %-4s |   %-4s |\n"," ","1","2","3","4" );
                      	vty_out(vty,"|  %-11s  |--------+--------+--------+--------|\n","POWER STATE");
                      	vty_out(vty,"|     %-4s      |  %-4s  |  %-4s  |  %-4s  |  %-4s  |\n"," ",power_state_str[0][power_state&0xf],
                  			                                                                     power_state_str[1][(power_state>>4)&0xf],
                  			                                                                     power_state_str[2][(power_state>>8)&0xf],
                  			                                                                     power_state_str[3][(power_state>>12)&0xf]);
                      	vty_out(vty,"+---------------+-----------------------------------|\n");
                      	vty_out(vty,"|     %-4s      |      %-5s      |       %-4s      |\n"," ","STATE","RPM" );
                      	vty_out(vty,"|  %-9s    |-----------------+-----------------|\n","FAN STATE");
                      	vty_out(vty,"|     %-4s      |      %-6s     |       %-5s     |\n"," ",fan_state>0 ? "NORMAL":"ALARM", fan_rpm_str[fan_rpm]);
						vty_out(vty,"+---------------+-----------------------------------+\n");
              		}
				#endif	
                  	vty_out(vty,"+---------------+-----------------------------------+\n");
				
    				if (board_id == BOARD_SOFT_ID_AX81_SMUE) {
                      	vty_out(vty,"|     SLOT%d     |      %-6s     |   %-11s   |\n",i,"CPU","ENVIRONMENT" );		
                      	vty_out(vty,"|     %-4s      |-----------------+-----------------|\n"," ");
                      	vty_out(vty,"|  %-11s  |  %-4s  | %-7s|  %-4s | %-7s|\n","TEMPERATURE", "CORE", "SURFACE", "LOCAL", "REMOTE");
                      	vty_out(vty,"|     %-4s      |--------+--------+--------+--------|\n"," ");
                            
	    			} else {				
                    	vty_out(vty,"|     SLOT%d     |      %-6s     |      %-5s      |\n",i,"MASTER","SLAVE" );		
                    	vty_out(vty,"|     %-4s      |-----------------+-----------------|\n"," ");
                  	    vty_out(vty,"|  %-11s  |  %-4s  | %-7s|  %-4s  | %-7s|\n","TEMPERATURE", "CORE", "SURFACE", "CORE", "SURFACE");
                  	    vty_out(vty,"|     %-4s      |--------+--------+--------+--------|\n"," ");
	    			}
              		if(cpu_num == DUAL_CPU || board_id == BOARD_SOFT_ID_AX81_SMUE) {
                  	    vty_out(vty,"|     %-4s      |   %-4d |   %-4d |   %-4d |   %-4d |\n"," ",master_inter_temp,master_remote_temp,
              			                                                                     slave_inter_temp, slave_remote_temp);
              		}else {
                  	    vty_out(vty,"|     %-4s      |   %-4d |   %-4d |   %-4s |   %-4s |\n"," ",master_inter_temp,master_remote_temp,
              			                                                                     "NONE", "NONE");
              		}
                  	vty_out(vty,"+---------------+-----------------------------------+\n");
                  		
                  	dbus_message_unref(reply);
                  	      
           		}
			}
		   dbus_message_unref(query);
		   return CMD_SUCCESS;
	}		
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_OBJPATH,NPD_DBUS_INTERFACE,NPD_DBUS_SYSTEM_SHOW_STATE);
	
	dbus_error_init(&err);
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
		DBUS_TYPE_BYTE,&fan_power,
		DBUS_TYPE_UINT16,&core_tmprt,
		DBUS_TYPE_UINT16,&surface_tmprt,
		DBUS_TYPE_INVALID)) {
			/* show detailed message below*/
	} 
	else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
				printf("%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}
	
	/*printf("char = %-20x, inter = %d, sur = %d\n",fan_power,core_tmprt,surface_tmprt);*/
	vty_out(vty,"+---------+---------+-------------------+\n");
	vty_out(vty,"|  %-5s  |  %-5s  |    %-11s    |\n"," "," ","TEMPERATURE" );
	vty_out(vty,"|  %-5s  |  %-5s  +---------+---------+\n","FAN","POWER");
	vty_out(vty,"|  %-5s  |  %-5s  |  %-5s  | %-7s |\n"," "," ","CORE","SURFACE");
	vty_out(vty,"+---------+---------+---------+---------+\n");
	for(i=0;i<2;i++) {
		if(fan_power & ( 1<<i )) {
			vty_out(vty,"|  %-6s ","normal");
		}
		else {		
			vty_out(vty,"|  %-6s ","alarm");
		}
	}
	vty_out(vty,"|  %-5d  | %-7d |\n",(short)core_tmprt,(short)surface_tmprt);
	vty_out(vty,"+---------+---------+---------+---------+\n");
		
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(config_sys_shut_down_state_cmd_func,
	config_sys_shut_down_state_cmd,
	"config system extreme-temperature shutdown (enable|disable)",
	CONFIG_STR
	"config system  alarm information\n"
	"config system  maxnum temperation operation\n"
	"config system  meet with the maxnum shutdown is done\n"
	"config system  shutdown enable \n"
	"config system  shutdown disable \n"
	)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned short isenable;
	int ret = NPD_SUCCESS;
	char* str = NULL;

	str = (char*) argv[0];

	if(0==strncmp(str,"enable",strlen(str))) {
		isenable = 1;
	}
	else if(0==strncmp(str,"disable",strlen(str))) {
		isenable = 0;
	}
	else {
		vty_out(vty,"%% CLI get bad parameter %s!\n",str);
		return CMD_SUCCESS;
	}	
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_OBJPATH,	\
								NPD_DBUS_INTERFACE,NPD_DBUS_SYSTEM_SHUTDOWN_STATE);
	dbus_message_append_args(query,
									DBUS_TYPE_UINT16,&isenable,
									DBUS_TYPE_INVALID);

	dbus_error_init(&err);
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
		DBUS_TYPE_UINT32,&ret,
		DBUS_TYPE_INVALID)) {
		if(ret != NPD_SUCCESS){
			vty_out(vty,"%% Set system shutdown %s fail\n",str);
			}
		
		} else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
				printf("%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
		}
	}
		
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}


DEFUN(config_arp_smac_check_cmd_func,
	config_arp_smac_check_cmd,
	"config arp smac-check (enable|disable)",
	CONFIG_STR
	"Config arp operation\n"
	"Config arp smac-check\n"
	"Config arp smac-check enable\n"
	"Config arp smac-check disable\n"
	)
{
	DBusMessage *query, *reply;
	DBusError err;


    int ret;	
	unsigned int isenable = 2;
	char* str = NULL;
	str = (char*) argv[0];
	int arglen=strlen(argv[0]);
	
	if(0==strncmp(str,"enable",arglen)){
		isenable = 1;
	}else if(0==strncmp(str,"disable",arglen)){
	    isenable = 0;
	}else{
	    isenable = 2;
	}
	if(isenable== 2){
		vty_out(vty,"%% CLI get bad parameter %s!\n",str);
		return CMD_SUCCESS;
	}	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_OBJPATH,NPD_DBUS_INTERFACE,NPD_DBUS_SYSTEM_ARP_CHECK_STATE);
	dbus_message_append_args(	query,
									DBUS_TYPE_UINT32,&isenable,
									DBUS_TYPE_INVALID);
	dbus_error_init(&err);
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
		DBUS_TYPE_UINT32,&ret,
		DBUS_TYPE_INVALID)) {
		if(ret != NPD_SUCCESS){
		    vty_out(vty,"%% Config arp SMAC-check err %d!\n",ret);			
		}
	} else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
				printf("%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
		}
	}
		
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(config_arp_strict_check_cmd_func,
	config_arp_strict_check_cmd,
	"config arp strict-check (enable|disable)",
	CONFIG_STR
	"Config arp operation\n"
	"Config arp strict-check\n"
	"Config arp strict-check enable\n"
	"Config arp strict-check disable\n"
	)
{
	DBusMessage *query, *reply;
	DBusError err;


    int ret;	
	unsigned int isenable = 2;
	char* str = NULL;
	str = (char*) argv[0];
	int arglen=strlen(argv[0]);
	
	if(0==strncmp(str,"enable",arglen)){
		isenable = 1;
	}else if(0==strncmp(str,"disable",arglen)){
	    isenable = 0;
	}else{
	    isenable = 2;
	}
	if(isenable== 2){
		vty_out(vty,"%% CLI get bad parameter %s!\n",str);
		return CMD_SUCCESS;
	}	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_OBJPATH,NPD_DBUS_INTERFACE,NPD_DBUS_SYSTEM_ARP_STRICT_CHECK_STATE);
	dbus_message_append_args(	query,
									DBUS_TYPE_UINT32,&isenable,
									DBUS_TYPE_INVALID);
	dbus_error_init(&err);
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
		DBUS_TYPE_UINT32,&ret,
		DBUS_TYPE_INVALID)) {
		if(ret != ARP_RETURN_CODE_SUCCESS){
		    vty_out(vty,"%% Config arp strict-check err %#x!\n",ret);			
		}
	} else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
				printf("%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
		}
	}
		
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(config_arp_aging_broadcast_cmd_func,
	config_arp_aging_broadcast_cmd,
	"config arp aging dest-mac (broadcast|unicast)",
	CONFIG_STR
	"Config ARP configuration\n"
	"Config ARP aging configuration\n"
	"Config ARP aging destination mac\n"
	"Config ARP aging destination mac broadcast\n"
	"Config ARP aging destination mac unicast\n"
	)
{
	DBusMessage *query, *reply;
	DBusError err;


    int ret;	
	unsigned int isBroadCast = 2;
	char* str = NULL;
	str = (char*) argv[0];
	int arglen=strlen(argv[0]);
	
	if(0==strncmp(str,"broadcast",arglen)){
		isBroadCast = 1;
	}else if(0==strncmp(str,"unicast",arglen)){
	    isBroadCast = 0;
	}else{
	    isBroadCast = 2;
	}
	if(isBroadCast== 2){
		vty_out(vty,"%% CLI get bad parameter %s!\n",str);
		return CMD_SUCCESS;
	}	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_OBJPATH,NPD_DBUS_INTERFACE,NPD_DBUS_SYSTEM_ARP_AGING_DEST_MAC);
	dbus_message_append_args(	query,
									DBUS_TYPE_UINT32,&isBroadCast,
									DBUS_TYPE_INVALID);
	dbus_error_init(&err);
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
		DBUS_TYPE_UINT32,&ret,
		DBUS_TYPE_INVALID)) {
		if(ret != NPD_SUCCESS){
		    vty_out(vty,"%% Config arp aging dest mac err %d!\n",ret);			
		}
	} else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
				printf("%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
		}
	}
		
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}



DEFUN(debug_npd_info ,
		debug_npd_info_cmd,
		"debug npd (all|error|warning|debug|event)",
		DEBUG_STR
		MODULE_DEBUG_STR(npd)
		MODULE_DEBUG_LEVEL_STR(npd,all)
		MODULE_DEBUG_LEVEL_STR(npd,error)
		MODULE_DEBUG_LEVEL_STR(npd,warning)
		MODULE_DEBUG_LEVEL_STR(npd,debug)
		MODULE_DEBUG_LEVEL_STR(npd,event)
)
{	
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int re_value = 0;
	unsigned int ret = 0;
	unsigned int flag = 0;
	
	if(argc > 1) {
		vty_out(vty,"%% Command parameters number error!\n");
		return CMD_WARNING;
	}
	
	if(0 == strncmp(argv[0],"all",strlen(argv[0]))) {
		flag = DCLI_DEBUG_FLAG_ALL;
	}		
	else if(0 == strncmp(argv[0],"error",strlen(argv[0]))) {
		flag = DCLI_DEBUG_FLAG_ERR;
	}
	else if (0 == strncmp(argv[0],"warning",strlen(argv[0]))){
	    flag = DCLI_DEBUG_FLAG_WAR;
	}
	else if (0 == strncmp(argv[0],"debug",strlen(argv[0]))){
	    flag = DCLI_DEBUG_FLAG_DBG;
	}
	else if (0 == strncmp(argv[0],"event",strlen(argv[0]))){
	    flag = DCLI_DEBUG_FLAG_EVT;
	}
	else {
		vty_out(vty,"%% Command parameter %s error!\n",argv[0]);
		return CMD_WARNING;
	}
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
							NPD_DBUS_OBJPATH,NPD_DBUS_INTERFACE,	\
							NPD_DBUS_INTERFACE_METHOD_SYSTEM_DEBUG_STATE);
    dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&flag,
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
									DBUS_TYPE_UINT32,&re_value,
									DBUS_TYPE_UINT32,&ret,
									DBUS_TYPE_INVALID)) {
		/*vty_out(vty," re_value:%d",re_value);				
		//vty_out(vty," return-value:%d\n",ret);*/
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


/*The no debug npd function*/

DEFUN(no_debug_npd_info ,
		no_debug_npd_info_cmd,
		"no debug npd (all|error|warning|debug|event)",
		NO_STR
		NODEBUG_STR
		MODULE_DEBUG_STR(module)
		MODULE_DEBUG_LEVEL_STR(npd,all)
		MODULE_DEBUG_LEVEL_STR(npd,error)
		MODULE_DEBUG_LEVEL_STR(npd,warning)
		MODULE_DEBUG_LEVEL_STR(npd,debug)
		MODULE_DEBUG_LEVEL_STR(npd,event)
)
{	
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int re_value = 0;
	unsigned int ret = 0;
	unsigned int flag = 0;
	
	if(argc > 1) {
		vty_out(vty,"%% Command parameters number error!\n");
		return CMD_WARNING;
	}
	
	if(0 == strncmp(argv[0],"all",strlen(argv[0]))) {
		flag = DCLI_DEBUG_FLAG_ALL;
	}		
	else if(0 == strncmp(argv[0],"error",strlen(argv[0]))) {
		flag = DCLI_DEBUG_FLAG_ERR;
	}
	else if (0 == strncmp(argv[0],"warning",strlen(argv[0]))){
	    flag = DCLI_DEBUG_FLAG_WAR;
	}
	else if (0 == strncmp(argv[0],"debug",strlen(argv[0]))){
	    flag = DCLI_DEBUG_FLAG_DBG;
	}
	else if (0 == strncmp(argv[0],"event",strlen(argv[0]))){
	    flag = DCLI_DEBUG_FLAG_EVT;
	}
	else {
		vty_out(vty,"%% Command parameter %s error!\n",argv[0]);
		return CMD_WARNING;
	}
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_OBJPATH,NPD_DBUS_INTERFACE,NPD_DBUS_INTERFACE_METHOD_SYSTEM_UNDEBUG_STATE);
    dbus_error_init(&err);
	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&flag,
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
									DBUS_TYPE_UINT32,&re_value,
									DBUS_TYPE_UINT32,&ret,
									DBUS_TYPE_INVALID)) {

		/*vty_out(vty," re_value:%d",re_value);				
		//vty_out(vty," return-value:%d\n",ret);*/
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

DEFUN(debug_npd_pkt_info ,
		debug_npd_pkt_info_cmd,
		"debug npd packet (all|receive|send)",
		DEBUG_STR
		MODULE_DEBUG_STR(npd)
		MODULE_DEBUG_STR(packet)
		MODULE_DEBUG_LEVEL_STR(packet,all)
		MODULE_DEBUG_LEVEL_STR(packet,receive)
		MODULE_DEBUG_LEVEL_STR(packet,send)
)
{	
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int re_value = 0;
	unsigned int ret = 0;
	unsigned int flag = 0;
	
	if(argc > 1) {
		vty_out(vty,"%% Command parameters number error!\n");
		return CMD_WARNING;
	}
	
	if(0 == strncmp(argv[0],"all",strlen(argv[0]))) {
		flag = DCLI_DEBUG_FLAG_PKT_ALL;
	}		
	else if(0 == strncmp(argv[0],"receive",strlen(argv[0]))) {
		flag = DCLI_DEBUG_FLAG_PKT_REV;
	}
	else if(0 == strncmp(argv[0],"send",strlen(argv[0]))) {
		flag = DCLI_DEBUG_FLAG_PKT_SED;
	}
	else {
		vty_out(vty,"%% Command parameter %s error!\n",argv[0]);
		return CMD_WARNING;
	}
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
							NPD_DBUS_OBJPATH,NPD_DBUS_INTERFACE,	\
							NPD_DBUS_INTERFACE_METHOD_SYSTEM_DEBUG_STATE);
    dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&flag,
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
									DBUS_TYPE_UINT32,&re_value,
									DBUS_TYPE_UINT32,&ret,
									DBUS_TYPE_INVALID)) {
		/*vty_out(vty," re_value:%d",re_value);				
		//vty_out(vty," return-value:%d\n",ret);*/
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


/*The no debug npd function*/

DEFUN(no_debug_npd_pkt_info ,
		no_debug_npd_pkt_info_cmd,
		"no debug npd packet (all|receive|send)",
		NO_STR
		NODEBUG_STR
		MODULE_DEBUG_STR(npd)
		MODULE_DEBUG_STR(packet)
		MODULE_DEBUG_LEVEL_STR(packet,all)
		MODULE_DEBUG_LEVEL_STR(packet,receive)
		MODULE_DEBUG_LEVEL_STR(npd,send)
)
{	
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int re_value = 0;
	unsigned int ret = 0;
	unsigned int flag = 0;
	
	if(argc > 1) {
		vty_out(vty,"%% Command parameters number error!\n");
		return CMD_WARNING;
	}
	
	if(0 == strncmp(argv[0],"all",strlen(argv[0]))) {
		flag = DCLI_DEBUG_FLAG_PKT_ALL;
	}		
	else if(0 == strncmp(argv[0],"receive",strlen(argv[0]))) {
		flag = DCLI_DEBUG_FLAG_PKT_REV;
	}
	else if(0 == strncmp(argv[0],"send",strlen(argv[0]))) {
		flag = DCLI_DEBUG_FLAG_PKT_SED;
	}
	else {
		vty_out(vty,"%% Command parameter %s error!\n",argv[0]);
		return CMD_WARNING;
	}

	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_OBJPATH,NPD_DBUS_INTERFACE,NPD_DBUS_INTERFACE_METHOD_SYSTEM_UNDEBUG_STATE);
    dbus_error_init(&err);
	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&flag,
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
									DBUS_TYPE_UINT32,&re_value,
									DBUS_TYPE_UINT32,&ret,
									DBUS_TYPE_INVALID)) {

		/*vty_out(vty," re_value:%d",re_value);				
		//vty_out(vty," return-value:%d\n",ret);*/
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


DEFUN(debug_nam_info ,
		debug_nam_info_cmd,
		"debug nam (all|error|warning|debug|event)",
		DEBUG_STR
		MODULE_DEBUG_STR(nam)
		MODULE_DEBUG_LEVEL_STR(nam,all)
		MODULE_DEBUG_LEVEL_STR(nam,error)
		MODULE_DEBUG_LEVEL_STR(nam,warning)
		MODULE_DEBUG_LEVEL_STR(nam,debug)
		MODULE_DEBUG_LEVEL_STR(nam,event)
)
{	
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int re_value = 0;
	unsigned int ret = 0;
	unsigned int flag = 0;
	
	if(argc > 1) {
		vty_out(vty,"%% Command parameters number error!\n");
		return CMD_WARNING;
	}
	
	if(0 == strncmp(argv[0],"all",strlen(argv[0]))) {
		flag = DCLI_DEBUG_FLAG_ALL;
	}		
	else if(0 == strncmp(argv[0],"error",strlen(argv[0]))) {
		flag = DCLI_DEBUG_FLAG_ERR;
	}
	else if (0 == strncmp(argv[0],"warning",strlen(argv[0]))){
	    flag = DCLI_DEBUG_FLAG_WAR;
	}
	else if (0 == strncmp(argv[0],"debug",strlen(argv[0]))){
	    flag = DCLI_DEBUG_FLAG_DBG;
	}
	else if (0 == strncmp(argv[0],"event",strlen(argv[0]))){
	    flag = DCLI_DEBUG_FLAG_EVT;
	}
	else {
		vty_out(vty,"%% Command parameter %s error!\n",argv[0]);
		return CMD_WARNING;
	}
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_OBJPATH,NPD_DBUS_INTERFACE,NAM_DBUS_METHOD_SYSLOG_DEBUG);
    dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&flag,
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
									DBUS_TYPE_UINT32,&re_value,
									DBUS_TYPE_UINT32,&ret,
									DBUS_TYPE_INVALID)) {
		/*vty_out(vty," re_value:%d",re_value);				
		//vty_out(vty," return-value:%d\n",ret);*/
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


/*The no debug npd function*/

DEFUN(no_debug_nam_info ,
		no_debug_nam_info_cmd,
		"no debug nam (all|error|warning|debug|event)",
		NO_STR
		NODEBUG_STR
		MODULE_DEBUG_STR(nam)
		MODULE_DEBUG_LEVEL_STR(nam,all)
		MODULE_DEBUG_LEVEL_STR(nam,error)
		MODULE_DEBUG_LEVEL_STR(nam,warning)
		MODULE_DEBUG_LEVEL_STR(nam,debug)
		MODULE_DEBUG_LEVEL_STR(nam,event)
)
{	
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int re_value = 0;
	unsigned int ret = 0;
	unsigned int flag = 0;
	
	if(argc > 1) {
		vty_out(vty,"%% Command parameters number error!\n");
		return CMD_WARNING;
	}
	
	if(0 == strncmp(argv[0],"all",strlen(argv[0]))) {
		flag = DCLI_DEBUG_FLAG_ALL;
	}		
	else if(0 == strncmp(argv[0],"error",strlen(argv[0]))) {
		flag = DCLI_DEBUG_FLAG_ERR;
	}
	else if (0 == strncmp(argv[0],"warning",strlen(argv[0]))){
	    flag = DCLI_DEBUG_FLAG_WAR;
	}
	else if (0 == strncmp(argv[0],"debug",strlen(argv[0]))){
	    flag = DCLI_DEBUG_FLAG_DBG;
	}
	else if (0 == strncmp(argv[0],"event",strlen(argv[0]))){
	    flag = DCLI_DEBUG_FLAG_EVT;
	}
	else {
		vty_out(vty,"%% Command parameter %s error!\n",argv[0]);
		return CMD_WARNING;
	}
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_OBJPATH,NPD_DBUS_INTERFACE,NAM_DBUS_METHOD_SYSLOG_NO_DEBUG);
    dbus_error_init(&err);
	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&flag,
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
									DBUS_TYPE_UINT32,&re_value,
									DBUS_TYPE_UINT32,&ret,
									DBUS_TYPE_INVALID)) {

		/*vty_out(vty," re_value:%d",re_value);				
		//vty_out(vty," return-value:%d\n",ret);*/
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


DEFUN(debug_nbm_info ,
		debug_nbm_info_cmd,
		"debug nbm (all|error|warning|debug|event)",
		DEBUG_STR
		MODULE_DEBUG_STR(nbm)
		MODULE_DEBUG_LEVEL_STR(nbm,all)
		MODULE_DEBUG_LEVEL_STR(nbm,error)
		MODULE_DEBUG_LEVEL_STR(nbm,warning)
		MODULE_DEBUG_LEVEL_STR(nbm,debug)
		MODULE_DEBUG_LEVEL_STR(nbm,event)
)
{	
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int re_value = 0;
	unsigned int ret = 0;
	unsigned int flag = 0;
	
	if(argc > 1) {
		vty_out(vty,"%% Command parameters number error!\n");
		return CMD_WARNING;
	}
	
	if(0 == strncmp(argv[0],"all",strlen(argv[0]))) {
		flag = DCLI_DEBUG_FLAG_ALL;
	}		
	else if(0 == strncmp(argv[0],"error",strlen(argv[0]))) {
		flag = DCLI_DEBUG_FLAG_ERR;
	}
	else if (0 == strncmp(argv[0],"warning",strlen(argv[0]))){
	    flag = DCLI_DEBUG_FLAG_WAR;
	}
	else if (0 == strncmp(argv[0],"debug",strlen(argv[0]))){
	    flag = DCLI_DEBUG_FLAG_DBG;
	}
	else if (0 == strncmp(argv[0],"event",strlen(argv[0]))){
	    flag = DCLI_DEBUG_FLAG_EVT;
	}
	else {
		vty_out(vty,"%% Command parameter %s error!\n",argv[0]);
		return CMD_WARNING;
	}
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_OBJPATH,NPD_DBUS_INTERFACE,NBM_DBUS_METHOD_SYSLOG_DEBUG);
    dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&flag,
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
									DBUS_TYPE_UINT32,&re_value,
									DBUS_TYPE_UINT32,&ret,
									DBUS_TYPE_INVALID)) {
		/*vty_out(vty," re_value:%d",re_value);				
		//vty_out(vty," return-value:%d\n",ret);*/
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


/*The no debug npd function*/

DEFUN(no_debug_nbm_info,
		no_debug_nbm_info_cmd,
		"no debug nbm (all|error|warning|debug|event)",
		NO_STR
		NODEBUG_STR
		MODULE_DEBUG_STR(nbm)
		MODULE_DEBUG_LEVEL_STR(nbm,all)
		MODULE_DEBUG_LEVEL_STR(nbm,error)
		MODULE_DEBUG_LEVEL_STR(nbm,warning)
		MODULE_DEBUG_LEVEL_STR(nbm,debug)
		MODULE_DEBUG_LEVEL_STR(nbm,event)
)
{	
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int re_value = 0;
	unsigned int ret = 0;
	unsigned int flag = 0;
	
	if(argc > 1) {
		vty_out(vty,"%% Command parameters number error!\n");
		return CMD_WARNING;
	}
	
	if(0 == strncmp(argv[0],"all",strlen(argv[0]))) {
		flag = DCLI_DEBUG_FLAG_ALL;
	}		
	else if(0 == strncmp(argv[0],"error",strlen(argv[0]))) {
		flag = DCLI_DEBUG_FLAG_ERR;
	}
	else if (0 == strncmp(argv[0],"warning",strlen(argv[0]))){
	    flag = DCLI_DEBUG_FLAG_WAR;
	}
	else if (0 == strncmp(argv[0],"debug",strlen(argv[0]))){
	    flag = DCLI_DEBUG_FLAG_DBG;
	}
	else if (0 == strncmp(argv[0],"event",strlen(argv[0]))){
	    flag = DCLI_DEBUG_FLAG_EVT;
	}
	else {
		vty_out(vty,"%% Command parameter %s error!\n",argv[0]);
		return CMD_WARNING;
	}
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_OBJPATH,NPD_DBUS_INTERFACE,NBM_DBUS_METHOD_SYSLOG_NO_DEBUG);
    dbus_error_init(&err);
	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&flag,
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
									DBUS_TYPE_UINT32,&re_value,
									DBUS_TYPE_UINT32,&ret,
									DBUS_TYPE_INVALID)) {

		/*vty_out(vty," re_value:%d",re_value);				
		//vty_out(vty," return-value:%d\n",ret);*/
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



DEFUN(debug_asic_on_func ,
		debug_asic_on_cmd,
		"debug asic (all|error|warning|debug|event)",
		DEBUG_STR
		MODULE_DEBUG_STR(asic)
		MODULE_DEBUG_LEVEL_STR(asic,all)
		MODULE_DEBUG_LEVEL_STR(asic,error)
		MODULE_DEBUG_LEVEL_STR(asic,warning)
		MODULE_DEBUG_LEVEL_STR(asic,debug)
		MODULE_DEBUG_LEVEL_STR(asic,event)
)
{	
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int re_value = 0;
	unsigned int ret = 0;
	unsigned int flag = 0;
	
	if(argc > 1) {
		vty_out(vty,"%% Command parameters number error!\n");
		return CMD_WARNING;
	}
	
	if(0 == strncmp(argv[0],"all",strlen(argv[0]))) {
		flag = DCLI_DEBUG_FLAG_ALL;
	}		
	else if(0 == strncmp(argv[0],"error",strlen(argv[0]))) {
		flag = DCLI_DEBUG_FLAG_ERR;
	}
	else if (0 == strncmp(argv[0],"warning",strlen(argv[0]))){
	    flag = DCLI_DEBUG_FLAG_WAR;
	}
	else if (0 == strncmp(argv[0],"debug",strlen(argv[0]))){
	    flag = DCLI_DEBUG_FLAG_DBG;
	}
	else if (0 == strncmp(argv[0],"event",strlen(argv[0]))){
	    flag = DCLI_DEBUG_FLAG_EVT;
	}
	else {
		vty_out(vty,"%% Command parameter %s error!\n",argv[0]);
		return CMD_WARNING;
	}
	
	query = dbus_message_new_method_call(		\
							NPD_DBUS_BUSNAME,		\
							NPD_DBUS_OBJPATH,			\
							NPD_DBUS_INTERFACE,			\
							NPD_DBUS_METHOD_ASIC_SYSLOG_DEBUG);
	
    dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&flag,
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
	if (!dbus_message_get_args ( reply, &err,
									DBUS_TYPE_UINT32,&ret,
									DBUS_TYPE_INVALID)) {		
		vty_out(vty,"Failed get args.\n");		
		if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
		}
	}
		
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(debug_asic_off_func ,
		debug_asic_off_cmd,
		"no debug asic (all|error|warning|debug|event)",
		NO_STR
		NODEBUG_STR	
		MODULE_DEBUG_STR(asic)
		MODULE_DEBUG_LEVEL_STR(asic,all)
		MODULE_DEBUG_LEVEL_STR(asic,error)
		MODULE_DEBUG_LEVEL_STR(asic,warning)
		MODULE_DEBUG_LEVEL_STR(asic,debug)
		MODULE_DEBUG_LEVEL_STR(asic,event)
)
{	
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int re_value = 0;
	unsigned int ret = 0;
	unsigned int flag = 0;
	
	if(argc > 1) {
		vty_out(vty,"%% Command parameters number error!\n");
		return CMD_WARNING;
	}
	
	if(0 == strncmp(argv[0],"all",strlen(argv[0]))) {
		flag = DCLI_DEBUG_FLAG_ALL;
	}		
	else if(0 == strncmp(argv[0],"error",strlen(argv[0]))) {
		flag = DCLI_DEBUG_FLAG_ERR;
	}
	else if (0 == strncmp(argv[0],"warning",strlen(argv[0]))){
	    flag = DCLI_DEBUG_FLAG_WAR;
	}
	else if (0 == strncmp(argv[0],"debug",strlen(argv[0]))){
	    flag = DCLI_DEBUG_FLAG_DBG;
	}
	else if (0 == strncmp(argv[0],"event",strlen(argv[0]))){
	    flag = DCLI_DEBUG_FLAG_EVT;
	}
	else {
		vty_out(vty,"%% Command parameter %s error!\n",argv[0]);
		return CMD_WARNING;
	}
	
	query = dbus_message_new_method_call(		\
							NPD_DBUS_BUSNAME,		\
							NPD_DBUS_OBJPATH,			\
							NPD_DBUS_INTERFACE,			\
							NPD_DBUS_METHOD_ASIC_SYSLOG_NO_DEBUG);
	
    dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&flag,
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
	if (!dbus_message_get_args ( reply, &err,
									DBUS_TYPE_UINT32,&ret,
									DBUS_TYPE_INVALID)) {		
		vty_out(vty,"Failed get args.\n");		
		if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
		}
	}
		
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}



DEFUN(debug_nam_pkt_on_func ,
		debug_nam_pkt_on_cmd,
		"debug nam packet (all|receive|send)",
		DEBUG_STR
		MODULE_DEBUG_STR(nam)
		MODULE_DEBUG_STR(packet)
		MODULE_DEBUG_LEVEL_STR(packet,all)
		MODULE_DEBUG_LEVEL_STR(packet,receive)
		MODULE_DEBUG_LEVEL_STR(packet,send)
)
{	
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int re_value = 0;
	unsigned int ret = 0;
	unsigned int flag = 0;
	
	if(argc > 1) {
		vty_out(vty,"%% Command parameters number error!\n");
		return CMD_WARNING;
	}
	
	if(0 == strncmp(argv[0],"all",strlen(argv[0]))) {
		flag = DCLI_DEBUG_FLAG_PKT_ALL;
	}		
	else if(0 == strncmp(argv[0],"send",strlen(argv[0]))) {
		flag = DCLI_DEBUG_FLAG_PKT_SED;
	}
	else if (0 == strncmp(argv[0],"receive",strlen(argv[0]))){
	    flag = DCLI_DEBUG_FLAG_PKT_REV;
	}
	else {
		vty_out(vty,"%% Command parameter %s error!\n",argv[0]);
		return CMD_WARNING;
	}
	
	query = dbus_message_new_method_call(		\
							NPD_DBUS_BUSNAME,		\
							NPD_DBUS_OBJPATH,			\
							NPD_DBUS_INTERFACE,			\
							NAM_DBUS_METHOD_SYSLOG_DEBUG);
	
    dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&flag,
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
	if (!dbus_message_get_args ( reply, &err,
									DBUS_TYPE_UINT32,&ret,
									DBUS_TYPE_INVALID)) {		
		vty_out(vty,"Failed get args.\n");		
		if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
		}
	}
		
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(debug_nam_pkt_off_func ,
		debug_nam_pkt_off_cmd,
		"no debug nam packet (all|receive|send)",
		NO_STR
		NODEBUG_STR	
		MODULE_DEBUG_STR(nam)
		MODULE_DEBUG_STR(packet)
		MODULE_DEBUG_LEVEL_STR(packet,all)
		MODULE_DEBUG_LEVEL_STR(packet,receive)
		MODULE_DEBUG_LEVEL_STR(packet,send)
)
{	
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int re_value = 0;
	unsigned int ret = 0;
	unsigned int flag = 0;
	
	if(argc > 1) {
		vty_out(vty,"%% Command parameters number error!\n");
		return CMD_WARNING;
	}
	
	if(0 == strncmp(argv[0],"all",strlen(argv[0]))) {
		flag = DCLI_DEBUG_FLAG_PKT_ALL;
	}		
	else if(0 == strncmp(argv[0],"send",strlen(argv[0]))) {
		flag = DCLI_DEBUG_FLAG_PKT_SED;
	}
	else if (0 == strncmp(argv[0],"receive",strlen(argv[0]))){
	    flag = DCLI_DEBUG_FLAG_PKT_REV;
	}
	else {
		vty_out(vty,"%% Command parameter %s error!\n",argv[0]);
		return CMD_WARNING;
	}

	
	query = dbus_message_new_method_call(		\
							NPD_DBUS_BUSNAME,		\
							NPD_DBUS_OBJPATH,			\
							NPD_DBUS_INTERFACE,			\
							NAM_DBUS_METHOD_SYSLOG_NO_DEBUG);
	
    dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&flag,
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
	if (!dbus_message_get_args ( reply, &err,
									DBUS_TYPE_UINT32,&ret,
									DBUS_TYPE_INVALID)) {		
		vty_out(vty,"Failed get args.\n");		
		if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
		}
	}
		
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

/*****************************************
 *
 * broadcast rate-limiter
 * it'll bind several cpu_codes as a group
 * cpu codes in group including :
 * 			ARP Broadcast
 *			...
 *			...
 *
 *****************************************/
DEFUN(config_cpu_protection_cmd_func,
	config_cpu_protection_cmd,
	"config broadcast rate-limit (enable|disable) ",
	"Config system feather\n"
	"Broadcast packet to cpu\n"
	"Rate limiter of Broadcast\n"
	"Enable broadcast rate-limiter \n"
	"Disable broadcast rate-limiter\n"
)
{	
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int ret,retVal;
	unsigned int flag;
	
	if(argc > 1) {
		vty_out(vty,"command parameters error!\n");
		return CMD_WARNING;
	}
	
	
	if(0 == strncmp("enable",argv[0],strlen(argv[0]))) {
		flag = 1;
	}
	else if(0 == strncmp("disable",argv[0],strlen(argv[0]))) {
		flag = 0;
	}
	else {
		vty_out(vty,"command parameter error!\n");
		return CMD_WARNING;
	}
	
	query = dbus_message_new_method_call(\
							NPD_DBUS_BUSNAME,\
							NPD_DBUS_OBJPATH,\
							NPD_DBUS_INTERFACE,	\
							NPD_DBUS_METHOD_CPU_PROTECTION_CONFIG);
	
    dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&flag,
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
									DBUS_TYPE_UINT32,&ret,
									DBUS_TYPE_UINT32,&retVal,
									DBUS_TYPE_INVALID)) {
		if(0 == ret){
			if(retVal == flag)
			 vty_out(vty,"operation Ok.");
		}
		else {
			vty_out(vty,"%Error : Config on Hw Fail.\n");
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

DEFUN(config_set_cpu_queue_fc_func,
	config_set_cpu_queue_fc_cmd,
	"config cpufc (arp|bpdu|dhcp|icmp|igmp|ospf|ripv1|ripv2|oam|telnet|ssh|http|https|snmp|wireless|capwapc|capwapd|all) <0-4095> <0-65535>",
	CONFIG_STR
	"Config cpu rate limiter\n"
	"Config cpu rate limiter of arp packet\n"
	"Config cpu rate limiter of bpdu packet\n"
	"Config cpu rate limiter of dhcp packet\n"
	"Config cpu rate limiter of icmp packet\n"
	"Config cpu rate limiter of igmp packet\n"
	"Config cpu rate limiter of ospf packet\n"
	"Config cpu rate limiter of rip version 1 packet\n"
	"Config cpu rate limiter of rip version 2 packet\n"
	"Config cpu rate limiter of OAM packet include telnet/ssh/http/https/snmp\n"
	"Config cpu rate limiter of telnet packet\n"
	"Config cpu rate limiter of ssh packet\n"
	"Config cpu rate limiter of http packet\n"
	"Config cpu rate limiter of https packet\n"
	"Config cpu rate limiter of snmp packet\n"
	"Config cpu rate limiter of wireless packet include capwapc/capwapd\n"
	"Config cpu rate limiter of wireless control packet\n"
	"Config cpu rate limiter of wireless service packet\n"
	"Config cpu rate limiter of all supported packet\n"
	"Config cpu rate limiter interval in step of 200us\n"
	"Config packet limit in one check cycle\n"
)
{	
	DBusMessage *query, *reply;
	DBusError err;
	int ret = 0;
	CPU_FC_PACKET_TYPE cpuFc = CPU_FC_TYPE_MAX_E;
	unsigned int time = 0,rate = 0;
	
	if(argc > 3) {
		vty_out(vty,"%% command parameters error!\n");
		return CMD_WARNING;
	}
	
	if(0 == strncmp("arp",argv[0],strlen(argv[0]))) {
		cpuFc  = CPU_FC_TYPE_ARP_E;
	}
	else if(0 == strncmp("bpdu",argv[0],strlen(argv[0]))) {
		cpuFc  = CPU_FC_TYPE_BPDU_E;
	}
	else if(0 == strncmp("dhcp",argv[0],strlen(argv[0]))) {
		cpuFc  = CPU_FC_TYPE_DHCP_E;
	}
	else if(0 == strncmp("icmp",argv[0],strlen(argv[0]))) {
		cpuFc  = CPU_FC_TYPE_ICMP_E;
	}
	else if(0 == strncmp("igmp",argv[0],strlen(argv[0]))) {
		cpuFc  = CPU_FC_TYPE_IGMP_E;
	}
	else if(0 == strncmp("ospf",argv[0],strlen(argv[0]))) {
		cpuFc  = CPU_FC_TYPE_OSPF_E;
	}
	else if(0 == strncmp("ripv1",argv[0],strlen(argv[0]))) {
		cpuFc  = CPU_FC_TYPE_RIPv1_E;
	}
	else if(0 == strncmp("ripv2",argv[0],strlen(argv[0]))) {
		cpuFc  = CPU_FC_TYPE_RIPv2_E;
	}
	else if(0 == strncmp("oam",argv[0],strlen(argv[0]))) {
		cpuFc  = CPU_FC_TYPE_OAM_E;
	}	
	else if(0 == strncmp("telnet",argv[0],strlen(argv[0]))) {
		cpuFc  = CPU_FC_TYPE_TELNET_E;
	}
	else if(0 == strncmp("ssh",argv[0],strlen(argv[0]))) {
		cpuFc  = CPU_FC_TYPE_SSH_E;
	}
	else if(0 == strncmp("http",argv[0],strlen(argv[0]))) {
		cpuFc  = CPU_FC_TYPE_HTTP_E;
	}
	else if(0 == strncmp("https",argv[0],strlen(argv[0]))) {
		cpuFc  = CPU_FC_TYPE_HTTPS_E;
	}
	else if(0 == strncmp("snmp",argv[0],strlen(argv[0]))) {
		cpuFc  = CPU_FC_TYPE_SNMP_E;
	}
	else if(0 == strncmp("wireless",argv[0],strlen(argv[0]))) {
		cpuFc  = CPU_FC_TYPE_WIRELESS_E;
	}
	else if(0 == strncmp("capwapc",argv[0],strlen(argv[0]))) {
		cpuFc  = CPU_FC_TYPE_CAPWAP_CTRL_E;
	}
	else if(0 == strncmp("capwapd",argv[0],strlen(argv[0]))) {
		cpuFc  = CPU_FC_TYPE_CAPWAP_DATA_E;
	}
	else if(0 == strncmp("all",argv[0],strlen(argv[0]))) {
		cpuFc  = CPU_FC_TYPE_ALL_E;
	}
	else {
		vty_out(vty,"%% Command parameter %s error\n",argv[0]);
		return CMD_WARNING;
	}

	ret = parse_int_parse(argv[1],&time);
	if(0 != ret) {
		vty_out(vty,"%% Command parameter %s error\n",argv[1]);
		return CMD_WARNING;
	}

	ret = parse_int_parse(argv[2],&rate);
	if(0 != ret) {
		vty_out(vty,"%% Command parameter %s error\n",argv[2]);
		return CMD_WARNING;
	}
	
	query = dbus_message_new_method_call(\
							NPD_DBUS_BUSNAME,\
							NPD_DBUS_OBJPATH,\
							NPD_DBUS_INTERFACE,	\
							NPD_DBUS_METHOD_CPU_FC_CONFIG);
	
    dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&cpuFc,
							DBUS_TYPE_UINT32,&time,
							DBUS_TYPE_UINT32,&rate,
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
									DBUS_TYPE_UINT32,&ret,
									DBUS_TYPE_INVALID)) {
		if(0 == ret){
			/* vty_out(vty,"operation Ok.");*/
		}
		else {
			vty_out(vty,"%% Command execute error %d\n",ret);
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

DEFUN(config_set_cpufc_quota_func,
	config_set_cpufc_quota_cmd,
	"config cpufc quota <0-7> <0-65535>",
	CONFIG_STR
	"Config cpu flow control\n"
	"Config cpu flow control queue quota\n"
	"Config cpu flow control queue number\n"
	"Config cpu flow control quota for the queue\n"
)
{	
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	int ret = 0;
	unsigned int queue = 0;
	unsigned int quota = 0;
	
	if(argc >= 3) {
		vty_out(vty,"%% Command parameters error!\n");
		return CMD_WARNING;
	}

	ret = parse_int_parse(argv[0],&queue);
	if(0 != ret) {
		vty_out(vty,"%% Bad parameter %s\n",argv[0]);
		return CMD_WARNING;
	}

	ret = parse_int_parse(argv[1],&quota);
	if(0 != ret) {
		vty_out(vty,"%% Bad parameter %s\n",argv[1]);
		return CMD_WARNING;
	}
	
	query = dbus_message_new_method_call(\
							NPD_DBUS_BUSNAME,\
							NPD_DBUS_OBJPATH,\
							NPD_DBUS_INTERFACE, \
							NPD_DBUS_METHOD_CPUFC_SET_QUEUE_QUOTA);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&queue,
							DBUS_TYPE_UINT32,&quota,
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
							DBUS_TYPE_UINT32,&ret,
							DBUS_TYPE_INVALID)) {
		if(ret) {
			vty_out(vty,"%% command execute error %d.\n",ret);
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

DEFUN(config_get_cpufc_quota_func,
	config_get_cpufc_quota_cmd,
	"show cpufc quota",
	SHOW_STR
	"Show cpu flow control configuration\n"
	"Show cpu flow control queue quota\n"
)
{	
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	int ret = 0;
	unsigned int quota[8] = {0};
	unsigned int i = 0;
		
	query = dbus_message_new_method_call(\
							NPD_DBUS_BUSNAME,\
							NPD_DBUS_OBJPATH,\
							NPD_DBUS_INTERFACE, \
							NPD_DBUS_METHOD_CPUFC_GET_QUEUE_QUOTA);
	
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
	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&ret,
					DBUS_TYPE_UINT32,&quota[0],
					DBUS_TYPE_UINT32,&quota[1],
					DBUS_TYPE_UINT32,&quota[2],
					DBUS_TYPE_UINT32,&quota[3],
					DBUS_TYPE_UINT32,&quota[4],
					DBUS_TYPE_UINT32,&quota[5],
					DBUS_TYPE_UINT32,&quota[6],
					DBUS_TYPE_UINT32,&quota[7],
					DBUS_TYPE_INVALID)) {
		if(ret) {
			vty_out(vty,"%% Command execute error %d.\n",ret);
		}
		else {
			vty_out(vty,"----------------------------------------\n");
			vty_out(vty,"%-4s%-6s%-4s%-6s\n","","Queue","","Quota");
			for(i = 0; i < 8; i++) {
				vty_out(vty,"%-4s%-6d%-4s%-6d\n","",i,"",quota[i]);
			}
			vty_out(vty,"----------------------------------------\n");			
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

DEFUN(config_set_cpufc_shaper_func,
	config_set_cpufc_shaper_cmd,
	"config cpufc queue-shaper <0-7> <0-65535>",
	CONFIG_STR
	"Config cpu port flow control\n"
	"Config cpu port queue shaper\n"
	"Config cpu port queue number\n"
	"Config shaper value(in unit of PPS) for cpu port queue\n"
)
{	
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	int ret = 0;
	unsigned int queue = 0;
	unsigned int shape = 0;
	
	if(argc >= 3) {
		vty_out(vty,"%% Command parameters error!\n");
		return CMD_WARNING;
	}

	ret = parse_int_parse(argv[0],&queue);
	if(0 != ret) {
		vty_out(vty,"%% Bad parameter %s\n",argv[0]);
		return CMD_WARNING;
	}

	ret = parse_int_parse(argv[1],&shape);
	if(0 != ret) {
		vty_out(vty,"%% Bad parameter %s\n",argv[1]);
		return CMD_WARNING;
	}
	
	query = dbus_message_new_method_call(\
							NPD_DBUS_BUSNAME,\
							NPD_DBUS_OBJPATH,\
							NPD_DBUS_INTERFACE, \
							NPD_DBUS_METHOD_CPUFC_SET_QUEUE_SHAPER);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&queue,
							DBUS_TYPE_UINT32,&shape,
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
							DBUS_TYPE_UINT32,&ret,
							DBUS_TYPE_INVALID)) {
		if(ret) {
			vty_out(vty,"%% Command execute error %d.\n",ret);
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

DEFUN(config_set_port_cpufc_shaper_func,
	config_set_port_cpufc_shaper_cmd,
	"config cpufc port-shaper <0-65535>",
	CONFIG_STR
	"Config cpu port flow control\n"
	"Config cpu port shaper\n"
	"Config shaper value(in unit of PPS) for cpu port queue\n"
)
{	
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	int ret = 0;
	unsigned int shape = 0;
		
	ret = parse_int_parse(argv[0],&shape);
	if(0 != ret) {
		vty_out(vty,"%% Bad parameter %s\n",argv[0]);
		return CMD_WARNING;
	}
	
	query = dbus_message_new_method_call(\
							NPD_DBUS_BUSNAME,\
							NPD_DBUS_OBJPATH,\
							NPD_DBUS_INTERFACE, \
							NPD_DBUS_METHOD_CPUFC_SET_PORT_SHAPER);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&shape,
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
							DBUS_TYPE_UINT32,&ret,
							DBUS_TYPE_INVALID)) {
		if(ret) {
			vty_out(vty,"%% Command execute error %d.\n",ret);
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

void dcli_system_show_cpu_channel_configuration
(
	struct vty *vty,
	unsigned char queue,
	unsigned char direction
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned char *showStr = NULL;
	
	query = dbus_message_new_method_call(\
							NPD_DBUS_BUSNAME,\
							NPD_DBUS_OBJPATH,\
							NPD_DBUS_INTERFACE, \
							NPD_DBUS_METHOD_CPU_SHOW_QUEUE_DESC
							);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_BYTE, &queue,
							DBUS_TYPE_BYTE, &direction,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {		
		vty_out(vty,"failed get reply.\n"); 
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return;
	}
	
	if (dbus_message_get_args ( reply, &err,
							DBUS_TYPE_STRING, &showStr,
							DBUS_TYPE_INVALID)) {
		vty_out(vty,"%s",showStr);
	}
	else {		
		vty_out(vty,"Failed get args.\n");		
		if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
		}
	}
		
	dbus_message_unref(reply);

	return;
}

DEFUN(show_cpu_all_queue_desc_func,
		show_cpu_all_queue_desc_cmd,
		"show dma-channel",
		SHOW_STR
		"Show cpu port dma channel info\n"
)
{
	unsigned int arguments = 0;
	unsigned char queue = 0, i = 0, direction = 0;
	
	direction = 2;
	for( i = 0; i < DCLI_SYSTEM_CPU_ALL_QUEUE; i++) {
		queue = i;
		dcli_system_show_cpu_channel_configuration(vty, queue,direction);
	}

	return CMD_SUCCESS;
}

DEFUN(show_cpu_single_queue_desc_func,
		show_cpu_single_queue_desc_cmd,
		"show dma-channel <0-7> (receive|transmit|bidirection)",
		SHOW_STR
		"Show cpu port dma channel info\n"
		"Select one dma channel number\n"
		"Show dma channel receive direction configuration\n"
		"Show dma channel transmit direction configuration\n"
		"Show dma channel bi-directional configuration\n"
)
{	
	unsigned int arguments = 0;
	unsigned char queue = 0, i = 0, direction = 0;
	int ret = 0;
	
	ret = parse_int_parse(argv[0],&arguments);
	if(0 != ret) {
		vty_out(vty,"%% Bad parameter %s\n",argv[0]);
		return CMD_WARNING;
	}
	queue = (unsigned char)arguments;
	vty_out(vty, "show cpu queue %d %s configuration.\n",queue,argv[1]);
	
	if(0 == strncmp(argv[1],"receive",strlen(argv[1]))) {
		direction = 0;
	}		
	else if(0 == strncmp(argv[1],"transmit",strlen(argv[1]))) {
		direction = 1;
	}
	else if(0 == strncmp(argv[1],"bidirection",strlen(argv[1]))) {
		direction = 2;
	}
	
	dcli_system_show_cpu_channel_configuration(vty, queue,direction);
	
	return CMD_SUCCESS;
}

void dcli_system_show_cpu_statistic
(
	struct vty *vty,
	unsigned int cpuIntfType,
	unsigned int *portStat
)
{
	unsigned int dmaNum = 0;
	if(!portStat) return;

	vty_out(vty, "----------------------------------------\n");
	/* cpu SDMA channel mib  */
	if(0x0 == cpuIntfType){
		vty_out(vty, "%-4s%-12s%-12s%-12s\n","DMA","RxPktCnt","RxByteCnt","RxErrCnt");
		vty_out(vty, "%-3s %-11s %-11s %-12s\n","---","-----------","-----------","------------");
		for(dmaNum = 0; dmaNum < 8; dmaNum++) {
			vty_out(vty, "%-3u %-11u %-11u %-11u\n", dmaNum, portStat[dmaNum*3], \
					portStat[dmaNum*3 + 1],portStat[dmaNum*3 + 2]);
		}
	}
	/* cpu ethernet mac mib*/
	else {
		vty_out(vty, "%-15s %-10s %-10s\n","DESCRIPTION","32BitH","32BitL");
		vty_out(vty, "%-15s %-10s %-10s\n","---------------","----------","----------");
		vty_out(vty, "%-15s %-10u %-10u\n","RxGoodPkts", portStat[1],portStat[0]);
		vty_out(vty, "%-15s %-10u %-10u\n","RxBadPkts", portStat[3],portStat[2]);
		vty_out(vty, "%-15s %-10u %-10u\n","RxGoodBytes", portStat[5],portStat[4]);
		vty_out(vty, "%-15s %-10u %-10u\n","RxBadBytes", portStat[7],portStat[6]);
		vty_out(vty, "%-15s %-10u %-10u\n","RxInternalDrop", portStat[9],portStat[8]);
		vty_out(vty, "%-15s %-10u %-10u\n","TxGoodPkts", portStat[11],portStat[10]);
		vty_out(vty, "%-15s %-10u %-10u\n","TxGoodBytes", portStat[13],portStat[12]);
		vty_out(vty, "%-15s %-10u %-10u\n","TxMacError", portStat[15],portStat[14]);
	}
	
	vty_out(vty, "----------------------------------------\n");
	return;
}

DEFUN(show_cpu_port_statistic_func,
		show_cpu_port_statistic_cmd,
		"show cpu statistics",
		SHOW_STR
		"Show cpu port configuration or statistics\n"
		"Show cpu port statistics\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter	 iter, iterArray, iterStruct;
	unsigned int cpuStat[24] = {0}, cpuIntfType = 0, instance = 0;
	int i = 0, j = 0;
	
	query = dbus_message_new_method_call(\
							NPD_DBUS_BUSNAME,\
							NPD_DBUS_OBJPATH,\
							NPD_DBUS_INTERFACE, \
							NPD_DBUS_METHOD_CPU_SHOW_PORT_MIB
							);
	
	dbus_error_init(&err);
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
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&instance);
	dbus_message_iter_next(&iter);	
	
	vty_out(vty,"Sytem has %d asic instance:\n", instance);
	for(i = 0; i < instance ; i++) {
		dbus_message_iter_get_basic(&iter,&cpuIntfType);
		dbus_message_iter_next(&iter);	
		
		for(j = 0; j < 24; j++) {
			dbus_message_iter_get_basic(&iter,&(cpuStat[j]));
			dbus_message_iter_next(&iter);	
		}
		vty_out(vty, "INSTANCE %d detailed info as follow\n", i);
		dcli_system_show_cpu_statistic(vty,cpuIntfType,cpuStat);
	}
			
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(clear_cpu_port_statistic_func,
		clear_cpu_port_statistic_cmd,
		"clear cpu statistics",
		CLEAR_STR
		"Clear cpu port configuration or statistics\n"
		"Clear cpu port statistics\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter  iter;
	unsigned int result = 0;
	
	query = dbus_message_new_method_call(\
							NPD_DBUS_BUSNAME,\
							NPD_DBUS_OBJPATH,\
							NPD_DBUS_INTERFACE, \
							NPD_DBUS_METHOD_CPU_CLEAR_PORT_MIB
							);
	
	dbus_error_init(&err);
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
	
	dbus_message_iter_init(reply,&iter);
	if (dbus_message_get_args ( reply, &err,
							DBUS_TYPE_UINT32,&result,
							DBUS_TYPE_INVALID)) {
		if(result) {
			vty_out(vty,"%% Command execute error %d\n",result);
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


int dcli_sys_global_show_running_cfg
(
	void
)
{
	char *showStr = NULL,*cursor = NULL,ch = 0,tmpBuf[SHOWRUN_PERLINE_SIZE] = {0};
	DBusMessage *query, *reply;
	DBusError err;
	int ret = 0;
	
query = dbus_message_new_method_call(		\
							NPD_DBUS_BUSNAME,		\
							NPD_DBUS_OBJPATH,			\
							NPD_DBUS_INTERFACE,			\
							NPD_DBUS_METHOD_SYS_GLOBAL_CFG_SAVE);

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("show system grobal running config failed get reply.\n");
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
		sprintf(_tmpstr,BUILDING_MOUDLE,"SYSTEM GLOBAL");
		vtysh_add_show_string(_tmpstr);
		vtysh_add_show_string(showStr);
		ret = 1;
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


#define CONFIG_L2_INGRESS_COUNTER_DROP_STR "Config bridge ingress drop counter\n"
#define CONFIG_L2_EGRESS_COUNTER_STR "Config bridge egress counter\n"

DEFUN(config_system_counter_drop_cmd_func,
     config_system_counter_drop_cmd,
    "config l2-ingress-drop (all|blacklist|unknown-smac|invalid-smac|invalid-vlan|membership|\
     vlan-range|station-move|arp-smac|rate-limit|vlan-mru|stp-state)",
    CONFIG_STR
    CONFIG_L2_INGRESS_COUNTER_DROP_STR
	" Config counter with all drop\n"
	" Config counter with blacklist\n"
	" Config counter with unknown source mac\n"
	" Config counter with invalid vlan\n"
	" Config counter with port and vlan membership mismatch\n"
	" Config counter with vlan out of range\n"
	" Config counter with station movement\n"
	" Config counter with arp source mac mismatch\n"
	" Config counter with rate limit\n"
	" Config counter with blacklist\n"
	" Config counter with vlan mru\n"
	" Config counter with stp state\n"
)
{
    DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;
	int ret = 0;
	BRIDGE_DROP_CNTR_MODE_ENT  reason = BRG_DROP_CNTR_COUNT_ALL_E;

	if(0 == strncmp("all",argv[0],strlen(argv[0]))) {
		reason  = BRG_DROP_CNTR_COUNT_ALL_E;
	}
	else if(0 == strncmp("blacklist",argv[0],strlen(argv[0]))) {
		reason  =  BRG_DROP_CNTR_FDB_ENTRY_CMD_E;
	}
	else if(0 == strncmp("invalid-smac",argv[0],strlen(argv[0]))) {
		reason  =  BRG_DROP_CNTR_INVALID_SA_E;
	}
	else if(0 == strncmp("invalid-vlan",argv[0],strlen(argv[0]))) {
		reason  =BRG_DROP_CNTR_INVALID_VLAN_E;
	}
	else if(0 == strncmp("membership",argv[0],strlen(argv[0]))) {
		reason  = BRG_DROP_CNTR_PORT_NOT_IN_VLAN_E;
	}
	else if(0 == strncmp("vlan-range",argv[0],strlen(argv[0]))) {
		reason  = BRG_DROP_CNTR_VLAN_RANGE_E;
	}
	else if(0 == strncmp("station-move",argv[0],strlen(argv[0]))) {
		reason  =  BRG_DROP_CNTR_MOVED_STATIC_ADDR_E;
	}

    else if(0 == strncmp("arp-smac",argv[0],strlen(argv[0]))) {
		reason  = BRG_DROP_CNTR_ARP_SA_MISMATCH_E;
	}
	else if(0 == strncmp("unknown-smac",argv[0],strlen(argv[0]))) {
		reason  =  BRG_DROP_CNTR_UNKNOWN_MAC_SA_E;
	}
	else if(0 == strncmp("rate-limit",argv[0],strlen(argv[0]))) {
		reason  =  BRG_DROP_CNTR_RATE_LIMIT_E;
	}
	else if(0 == strncmp("vlan-mru",argv[0],strlen(argv[0]))) {
		reason  =  BRG_DROP_CNTR_VLAN_MRU_E;
	}
	else if(0 == strncmp("stp-state",argv[0],strlen(argv[0]))) {
		reason  = BRG_DROP_CNTR_SPAN_TREE_PORT_ST_E;
	}

	else {
		vty_out(vty,"%% Command parameter %s error!\n", argv[0]);
		return CMD_WARNING;
	}

	query = dbus_message_new_method_call(
								   NPD_DBUS_BUSNAME,	   \
								   NPD_DBUS_OBJPATH,		   \
								   NPD_DBUS_INTERFACE,		   \
								   NPD_DBUS_SYSTEM_CONFIG_COUNTER_DROP_STATISTIC);
   
   
	dbus_error_init(&err);
   
   
	dbus_message_append_args(query,
							   DBUS_TYPE_UINT32,&reason,
							   DBUS_TYPE_INVALID);
	   
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
	   vty_out(vty,"failed get reply.\n");
		  if (dbus_error_is_set(&err)) {
			   vty_out(vty,"%s raised: %s",err.name,err.message);
			   dbus_error_free_for_dcli(&err);
		   }
		   return ;
	   }
   
	   if (dbus_message_get_args ( reply, &err,
					   DBUS_TYPE_UINT32, &op_ret,
					   DBUS_TYPE_INVALID)) 
	   {  
		   /*vty_out(vty,"Drop reason: %s,%s ,%d\n",DropReason[op_ret][0], DropReason[op_ret][1], op_rem);*/
		     
		  if(0!=op_ret)
   
			  vty_out(vty,"set is fail\n");
			  
	   }	 
	   else 
	   {
		   if (dbus_error_is_set(&err)) 
		   {
			   vty_out(vty,"%s raised: %s\n",err.name,err.message);
			   dbus_error_free_for_dcli(&err);
		   }
	   }
	   dbus_message_unref(reply);
	   return CMD_SUCCESS; 
   }
DEFUN(show_system_counter_drop_cmd_func,
	       show_system_counter_drop_cmd,
	       "show l2-ingress-drop statistic",
	       SHOW_STR
           "Show bridge ingress drop counter\n"
           "Show bridge ingress drop counter\n"
 )   
{
	
    DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;
	unsigned int op_rem=0;
    char *DropReason[] = { 	\
		"all", 				\
  	    "blacklist", 		\
  	    "unknown source mac", 		\
        "invalid source mac",		\
        "invalid vlan", 		\
        "port and vlan membership mismatch", 	\
        "vlan out of range", 	\
        "station movement", \
        "arp source mac mismatch",\
        "rate limit",\
        "vlan mru",\
        "stp state" \
   };

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,		\
							    NPD_DBUS_OBJPATH,			\
							    NPD_DBUS_INTERFACE,			\
								NPD_DBUS_SYSTEM_SHOW_COUNTER_DROP_STATISTIC);


	dbus_error_init(&err);
    
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return ;
	}
	vty_out(vty,"Detailed bridge drop counter\n");
	vty_out(vty,"========================================\n");
	vty_out(vty," %-18s %-20s\n","REASON","COUNT");
	vty_out(vty,"%-20s%-20s\n","-------------------","--------------------");
	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_UINT32, &op_rem,
					DBUS_TYPE_INVALID)) 
	{  
		switch(op_ret) {
			default:
				vty_out(vty," %-18s ","unkown");
				break;
			case BRG_DROP_CNTR_COUNT_ALL_E:					  /* 0 */
			case BRG_DROP_CNTR_FDB_ENTRY_CMD_E:				  /* 1 */
			case BRG_DROP_CNTR_UNKNOWN_MAC_SA_E: 			  /* 2 */
			case BRG_DROP_CNTR_INVALID_SA_E: 				  /* 3 */
			case BRG_DROP_CNTR_INVALID_VLAN_E:				  /* 4 */
			case BRG_DROP_CNTR_PORT_NOT_IN_VLAN_E:			  /* 5 */
			case BRG_DROP_CNTR_VLAN_RANGE_E: 				  /* 6 */
			case BRG_DROP_CNTR_MOVED_STATIC_ADDR_E:			  /* 7 */
			case BRG_DROP_CNTR_ARP_SA_MISMATCH_E:			  /* 8 */
				vty_out(vty, " %-18s ",DropReason[op_ret]);
				break;
			case BRG_DROP_CNTR_VLAN_MRU_E: 	/* 20*/
				vty_out(vty, " %-18s ",DropReason[BRG_DROP_CNTR_ARP_SA_MISMATCH_E + 1]);
				break;
			case BRG_DROP_CNTR_RATE_LIMIT_E: /* 21 */
				vty_out(vty, " %-18s ",DropReason[BRG_DROP_CNTR_ARP_SA_MISMATCH_E + 2]);
				break;
			case BRG_DROP_CNTR_SPAN_TREE_PORT_ST_E:	/* 23 */
				vty_out(vty, " %-18s ",DropReason[BRG_DROP_CNTR_ARP_SA_MISMATCH_E + 3]);
				break;
		}
		vty_out(vty,"%-d\n",op_rem);
	} 
	else 
	{
		if (dbus_error_is_set(&err)) 
		{
			vty_out(vty,"%s raised: %s\n",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	vty_out(vty,"========================================\n");
    dbus_message_unref(reply);
	return CMD_SUCCESS;	
}
DEFUN(config_bridge_port_counter_egress_cmd_func,
   config_brg_pt_cnt_egress_cmd,
   "config l2-egress-counter port PORTNO",
   CONFIG_STR
   CONFIG_L2_EGRESS_COUNTER_STR
   "Config bridge egress counter with port\n"
	CONFIG_ETHPORT_STR
)
{
  DBusMessage *query = NULL, *reply = NULL;
  DBusError err;
  unsigned char slot_no = 0,port_no = 0;
  unsigned int op_ret = 0;
  unsigned int ret = 0 ;

  
  op_ret = parse_slotport_no((char *)argv[0], &slot_no, &port_no);
  if (NPD_FAIL == op_ret) {
	  vty_out(vty,"%% Input bad slot/port!\n");
	  return CMD_WARNING;
  }
   
  query = dbus_message_new_method_call(
						NPD_DBUS_BUSNAME,		\
						NPD_DBUS_OBJPATH,			\
						NPD_DBUS_INTERFACE, 		\
						NPD_DBUS_SYSTEM_CONFIG_EGRESS_COUNTER);

  dbus_error_init(&err);
  dbus_message_append_args(query,
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
									DBUS_TYPE_UINT32,&ret,
									DBUS_TYPE_INVALID)) {
	 

		if(0 == ret){
		   /*vty_out(vty,"%%operation Ok.");*/
		}

		else if( NPD_DBUS_ERROR_NO_SUCH_PORT == ret){

		  vty_out(vty,"Error:Illegal %d/%d,No such port.\n",slot_no,port_no);

		}
      	else {
		   vty_out(vty,"%% Command execute error.\n");
		   return CMD_WARNING;
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

DEFUN(config_bridge_vlan_counter_egress_cmd_func,
   config_brg_vln_cnt_egress_cmd,
   "config l2-egress-counter vlan <1-4095> [port] [PORTNO]",
   CONFIG_STR
   CONFIG_L2_EGRESS_COUNTER_STR
   "Config bridge egress counter with vlan\n"
   "Specify vlan id for l2 egress counter\n"
   "Setting ethernet port for l2 egress counter\n"
   "Config bridge egress counter with port\n"
	CONFIG_ETHPORT_STR
)

{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned char   slot_no = 0,port_no = 0;
	unsigned int op_ret = 0;
	unsigned int ret = 0;
    unsigned short vlanId = 0;
	unsigned char port=0;
    
    ret = parse_vlan_no((char*)argv[0],&vlanId);
	if (NPD_FAIL == ret) {
    	vty_out(vty,"%% Bad parameter,vlan id illegal!\n");
		return CMD_WARNING;
	}
    if ((vlanId<MIN_VLAN_ID)||(vlanId>MAX_VLAN_ID)){
		vty_out(vty,"% Bad Parameters,vlan outrange!\n");
		return CMD_WARNING;
		}
	
	#if 0
	if (4095 == vlanId){
		vty_out(vty,"%% Bad parameter,reserved vlan for l3 interface!\n");
		return CMD_WARNING;
	}
	#endif

    if((argc>1)&&(argc < 4)){
   	    if(0 == (strncmp("port",argv[1],strlen(argv[1])))){
			port = 1;
		    op_ret = parse_slotport_no((char *)argv[2], &slot_no, &port_no);
		    if (NPD_FAIL == op_ret) {
				    vty_out(vty,"%% Input bad slot/port!\n");
				    return CMD_WARNING;
		    }
	    }
    }
  
    query = dbus_message_new_method_call(
						 NPD_DBUS_BUSNAME,		 \
						 NPD_DBUS_OBJPATH,			 \
						 NPD_DBUS_INTERFACE,		 \
						 NPD_DBUS_CONFIG_VLAN_EGRESS_COUNTER);



   dbus_error_init(&err);
   dbus_message_append_args(query,
  	                   DBUS_TYPE_UINT16,&vlanId,
  	                   DBUS_TYPE_BYTE,&port,
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
								   DBUS_TYPE_UINT32,&ret,
								   DBUS_TYPE_INVALID)) {
	   if(0 == ret){
			/*vty_out(vty,"%%operation Ok.");*/
	   }
	   	else if( NPD_DBUS_ERROR_NO_SUCH_PORT == ret){

		    vty_out(vty,"Error:Illegal %d/%d,No such port.\n",slot_no,port_no);
		}
		else if(NPD_VLAN_NOTEXISTS == ret){
			vty_out(vty,"%% Error,The vlan %d does not exist\n",vlanId);
		}
		else if(NPD_VLAN_PORT_NOTEXISTS== ret){
		    vty_out(vty,"%% Error,The port number is not in vlan %d\n",vlanId);
		}
	   else {
		    vty_out(vty,"%% command execute error.\n");
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

DEFUN( config_bridge_host_dmac_counter_ingress_cmd_func,
		config_brg_host_dmac_cnt_ingress_cmd,
		"config l2-ingress-counter dmac DMAC",
		CONFIG_STR
		"Config l2-ingress-counter information\n"
		"Config host dmac information\n"
		"Config DMAC addressin Hexa.Eg 00:00:11:11:aa:aa\n"
)


{
		DBusMessage *query, *reply;
        DBusError err;
		unsigned int   op_ret = 0; 
		ETHERADDR     macAddr ;
		memset(&macAddr,0,sizeof(ETHERADDR));

		op_ret = parse_mac_addr((char *) argv[0],&macAddr);

		if(NPD_FAIL == op_ret){
		   vty_out(vty,"Bad Parameters,Unknow mac addr format");
		return CMD_WARNING;
		} 
		query = dbus_message_new_method_call(
												NPD_DBUS_BUSNAME, 	  \
												NPD_DBUS_OBJPATH, 		  \
												NPD_DBUS_INTERFACE,		  \
												NPD_DBUS_CONFIG_HOST_DMAC_INGRESS_COUNTER);



		dbus_error_init(&err);
		dbus_message_append_args(query,
								DBUS_TYPE_BYTE,&macAddr.arEther[0],
								DBUS_TYPE_BYTE,&macAddr.arEther[1],
								DBUS_TYPE_BYTE,&macAddr.arEther[2],
								DBUS_TYPE_BYTE,&macAddr.arEther[3],
								DBUS_TYPE_BYTE,&macAddr.arEther[4],
								DBUS_TYPE_BYTE,&macAddr.arEther[5],
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
								   DBUS_TYPE_INVALID)) {
			if(0 == op_ret){
			/*vty_out(vty,"%%operation Ok.");*/
			}

			else if(NPD_ERR_SYSTEM_MAC == op_ret){
				vty_out(vty,"%% Error,the mac input conflict with system mac,please chang!\n");
			}
			else {
			vty_out(vty,"%% command execute error.\n");
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

DEFUN( config_bridge_host_smac_counter_ingress_cmd_func,
		config_brg_host_smac_cnt_ingress_cmd,
		"config l2-ingress-counter smac SMAC",
		CONFIG_STR
		"Config l2-ingress-counter information.\n"
		"Config host dmac information\n"
		"Config SMAC addressin Hexa.Eg 00:00:11:11:aa:aa\n"
)


{
		DBusMessage *query, *reply;
        DBusError err;
		unsigned int   op_ret = 0; 
		ETHERADDR     macAddr ;
		memset(&macAddr,0,sizeof(ETHERADDR));

		op_ret = parse_mac_addr((char *) argv[0],&macAddr);

		if(NPD_FAIL == op_ret){
			vty_out(vty,"Bad Parameters,Unknow mac addr format");
			return CMD_WARNING;
		} 
		query = dbus_message_new_method_call(
												NPD_DBUS_BUSNAME, 	  \
												NPD_DBUS_OBJPATH, 		  \
												NPD_DBUS_INTERFACE,		  \
												NPD_DBUS_CONFIG_HOST_SMAC_INGRESS_COUNTER);



		dbus_error_init(&err);
		dbus_message_append_args(query,
								DBUS_TYPE_BYTE,&macAddr.arEther[0],
								DBUS_TYPE_BYTE,&macAddr.arEther[1],
								DBUS_TYPE_BYTE,&macAddr.arEther[2],
								DBUS_TYPE_BYTE,&macAddr.arEther[3],
								DBUS_TYPE_BYTE,&macAddr.arEther[4],
								DBUS_TYPE_BYTE,&macAddr.arEther[5],
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
								   DBUS_TYPE_INVALID)) {
			if(0 == op_ret){
			/*vty_out(vty,"%%operation Ok.");*/
			}
			else {
			vty_out(vty,"%% command execute error.\n");
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
DEFUN( config_bridge_host_counter_ingress_cmd_func,
		config_brg_host_cnt_ingress_cmd,
		"config l2-ingress-counter dmac DMAC smac SMAC",
		CONFIG_STR
		"Config l2-ingress-counter information.\n"
		"Config host dmac information\n"
		"Config SMAC addressin Hexa.Eg 00:00:11:11:aa:aa\n"
		"Config host smac information\n"
		"Config SMAC addressin Hexa.Eg 00:00:11:11:aa:aa\n"
)


{
	DBusMessage *query, *reply;
    DBusError err;
	unsigned int   op_ret = 0; 
	ETHERADDR     dmacAddr ;
	ETHERADDR     smacAddr ;
	
	memset(&dmacAddr,0,sizeof(ETHERADDR));
	memset(&smacAddr,0,sizeof(ETHERADDR));

	op_ret = parse_mac_addr((char *) argv[0],&dmacAddr);

	if(NPD_FAIL == op_ret){
		vty_out(vty,"Bad Parameters,Unknow mac addr format");
		return CMD_WARNING;
	} 

	op_ret = parse_mac_addr((char *)argv[1],&smacAddr);
    if(NPD_FAIL == op_ret){
       vty_out(vty,"Bad Parameters,Unknow mac addr format");
	   return CMD_WARNING;

	}
	
	query = dbus_message_new_method_call(
											NPD_DBUS_BUSNAME, 	  \
											NPD_DBUS_OBJPATH, 		  \
											NPD_DBUS_INTERFACE,		  \
											NPD_DBUS_CONFIG_HOST_INGRESS_COUNTER);

	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_BYTE,&dmacAddr.arEther[0],
							DBUS_TYPE_BYTE,&dmacAddr.arEther[1],
							DBUS_TYPE_BYTE,&dmacAddr.arEther[2],
							DBUS_TYPE_BYTE,&dmacAddr.arEther[3],
							DBUS_TYPE_BYTE,&dmacAddr.arEther[4],
							DBUS_TYPE_BYTE,&dmacAddr.arEther[5],
							DBUS_TYPE_BYTE,&smacAddr.arEther[0],
							DBUS_TYPE_BYTE,&smacAddr.arEther[1],
							DBUS_TYPE_BYTE,&smacAddr.arEther[2],
							DBUS_TYPE_BYTE,&smacAddr.arEther[3],
							DBUS_TYPE_BYTE,&smacAddr.arEther[4],
							DBUS_TYPE_BYTE,&smacAddr.arEther[5],
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
							   DBUS_TYPE_INVALID)) {
		if(0 == op_ret){
		/*vty_out(vty,"%%operation Ok.");*/
		}
		else {
		vty_out(vty,"%% command execute error.\n");
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

DEFUN(  show_sys_cnt_egress_cmd_func,
		show_sys_cnt_egress_cmd,
		"show l2-egress-counter",
		SHOW_STR
		"Show bridge egress counter\n"
) 
{
   DBusMessage *query = NULL, *reply = NULL;
   DBusError err;
   unsigned int op_ret = 0;
   unsigned int ret=0;
   PORT_EGRESS_CNTR_STC	egrCntrPtr;
#define CTRL_PACKET_DESCRIPTOR \
"* control packet means:\n\
     packets send to cpu or\n\
     packets send from cpu or\n\
     packets send to analyzer port\n"
     
#define FILTERED_PACKET_DESCRIPTOR \
"* filtered packet means:\n\
     packets filtered due to vlan egress filter or\n\
     packets filtered due to spanning tree egress filter\n"

#define TAILDROP_PACKET_DESCRIPTOR \
"* congestion drop packet means:\n\
     packets filtered due to egress queue congestion(i.e. tail drop)\n"

   memset(&egrCntrPtr,0,sizeof(PORT_EGRESS_CNTR_STC));
   query = dbus_message_new_method_call(
							   NPD_DBUS_BUSNAME,	   \
							   NPD_DBUS_OBJPATH,		   \
							   NPD_DBUS_INTERFACE,		   \
							   NPD_DBUS_SYSTEM_SHOW_COUNTER_EGRESS);
   dbus_error_init(&err);
   
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
								  DBUS_TYPE_UINT32,&egrCntrPtr.brgEgrFilterDisc,
								  DBUS_TYPE_UINT32,&egrCntrPtr.outBcFrames,
								  DBUS_TYPE_UINT32,&egrCntrPtr.outCtrlFrames,
								  DBUS_TYPE_UINT32,&egrCntrPtr.outMcFrames,
								  DBUS_TYPE_UINT32,&egrCntrPtr.outUcFrames,
								  DBUS_TYPE_UINT32,&egrCntrPtr.txqFilterDisc,
								  DBUS_TYPE_INVALID));
	   if (0==op_ret) {
		   vty_out(vty,"========================================\n");
		   vty_out(vty," %-18s %-20s\n","PACKET TYPE","COUNT");
		   vty_out(vty,"%-20s%-20s\n","-------------------","--------------------");
		   vty_out(vty," %-18s %-d\n","filtered",egrCntrPtr.brgEgrFilterDisc);
		   vty_out(vty," %-18s %-d\n","control",egrCntrPtr.outCtrlFrames);
		   vty_out(vty," %-18s %-d\n","congestion drop",egrCntrPtr.txqFilterDisc);
		   vty_out(vty," %-18s %-d\n","broadcast",egrCntrPtr.outBcFrames);
		   vty_out(vty," %-18s %-d\n","multicast",egrCntrPtr.outMcFrames);
		   vty_out(vty," %-18s %-d\n","unicast",egrCntrPtr.outUcFrames);
		   vty_out(vty,"========================================\n");
		   vty_out(vty,"%s",CTRL_PACKET_DESCRIPTOR);
		   vty_out(vty,"%s",FILTERED_PACKET_DESCRIPTOR);
		   vty_out(vty,"%s",TAILDROP_PACKET_DESCRIPTOR);
	   } 
	   else 
	   {
		   if (dbus_error_is_set(&err)) 
		   {
			   vty_out(vty,"%s raised: %s\n",err.name,err.message);
			   dbus_error_free_for_dcli(&err);
		   }
	   }
   
		  dbus_message_unref(reply);
		  return CMD_SUCCESS;  
	   
 }

#define HOST_INT_PACKET_DESCRIPTOR \
"* HIP - number of good packets with a MAC DA\n\
         matching the CPU-configured MAC DA.\n"
     
#define HOST_OUT_PACKET_DESCRIPTOR \
"* HOP - number of good packets with a MAC SA\n\
         matching the CPU-configured MAC SA.\n"

#define HOST_Broadcast_PACKET_DESCRIPTOR \
"* HOBP - number of good Broadcast packets with a MAC SA\n\
          matching the configured MAC SA.\n"

#define HOST_Multicast_PACKET_DESCRIPTOR \
"* HOMP - number of good Multicast packets with a MAC SA\n\
          matching the configured MAC SA.\n"
#define HOST_Matrix_PACKET_DESCRIPTOR \
"* HIOP - number of good  packets with a MAC SA/DA\n\
          matching the configured MAC SA/DA.\n"

DEFUN( show_sys_cnt_ingress_cmd_func,
		show_sys_cnt_ingress_cmd,
		"show l2-ingress-counter",
		SHOW_STR
		"Show bridge host ingress counter\n"


)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;
    BRIDGE_HOST_CNTR_STC hostGroupCntPtr ;
	ETHERADDR dmacAddr;
	ETHERADDR smacAddr;
	unsigned int matrixCntSaDaPkts = 0;

	unsigned char arEther[6]={0}, smacStr[18] = {0},dmacStr[18] = {0};/* alloc 1B more to support sprintf append null at the end of the buffer */
	

	memset(&dmacAddr,0,sizeof(ETHERADDR));
	memset(&smacAddr,0,sizeof(ETHERADDR));
	memset(&hostGroupCntPtr,0,sizeof(BRIDGE_HOST_CNTR_STC));

	query = dbus_message_new_method_call(
											NPD_DBUS_BUSNAME, 	  \
											NPD_DBUS_OBJPATH, 		  \
											NPD_DBUS_INTERFACE,		  \
											NPD_DBUS_SYSTEM_SHOW_COUNTER_INGRESS);
	dbus_error_init(&err);

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
									 DBUS_TYPE_BYTE,&dmacAddr.arEther[0],
									 DBUS_TYPE_BYTE,&dmacAddr.arEther[1],
									 DBUS_TYPE_BYTE,&dmacAddr.arEther[2],
									 DBUS_TYPE_BYTE,&dmacAddr.arEther[3],
									 DBUS_TYPE_BYTE,&dmacAddr.arEther[4],
									 DBUS_TYPE_BYTE,&dmacAddr.arEther[5],
									 DBUS_TYPE_BYTE,&smacAddr.arEther[0],
									 DBUS_TYPE_BYTE,&smacAddr.arEther[1],
									 DBUS_TYPE_BYTE,&smacAddr.arEther[2],
									 DBUS_TYPE_BYTE,&smacAddr.arEther[3],
									 DBUS_TYPE_BYTE,&smacAddr.arEther[4],
									 DBUS_TYPE_BYTE,&smacAddr.arEther[5],
									 DBUS_TYPE_UINT32,&hostGroupCntPtr.gtHostInPkts,
									 DBUS_TYPE_UINT32,&hostGroupCntPtr.gtHostOutBroadcastPkts,
									 DBUS_TYPE_UINT32,&hostGroupCntPtr.gtHostOutMulticastPkts,
									 DBUS_TYPE_UINT32,&hostGroupCntPtr.gtHostOutPkts,
									 DBUS_TYPE_UINT32,&matrixCntSaDaPkts,
									 DBUS_TYPE_INVALID));
	     
		  if (0==op_ret) {
			  vty_out(vty,"==========================================================================\n");
			  vty_out(vty,"%-20s %-20s %-10s %-20s\n","SOURCE MAC","DESTINATION MAC","COUNT",   "DESCRIPTION");
			  vty_out(vty,"%-20s %-20s %-10s %-20s\n","----------","---------------","-------", "-----------");

			  /* format mac string as xx:xx:xx:xx:xx:xx */
			  sprintf(dmacStr,"%02x:%02x:%02x:%02x:%02x:%02x",
			  			dmacAddr.arEther[0],dmacAddr.arEther[1],dmacAddr.arEther[2],
			  			dmacAddr.arEther[3],dmacAddr.arEther[4],dmacAddr.arEther[5]);
			  sprintf(smacStr,"%02x:%02x:%02x:%02x:%02x:%02x",
			  			smacAddr.arEther[0],smacAddr.arEther[1],smacAddr.arEther[2],
			  			smacAddr.arEther[3],smacAddr.arEther[4],smacAddr.arEther[5]);
			  
			  /* show host incoming packet count */
			  vty_out(vty,"%-20s %-20s %-10d %-20s\n", "any",
			  			memcmp(dmacAddr.arEther,arEther,6) ? dmacStr : "any",
			  			hostGroupCntPtr.gtHostInPkts,"HIP");

			  /* show host outgoing packet count */
			  /* all packets */
			  vty_out(vty,"%-20s %-20s %-10d %-20s\n",memcmp(smacAddr.arEther,arEther,6) ? smacStr : "any",
			  			"any",hostGroupCntPtr.gtHostOutPkts,"HOP");
			  /* all broadcast packet */
			  vty_out(vty,"%-20s %-20s %-10d %-20s\n",memcmp(smacAddr.arEther,arEther,6) ? smacStr : "any",
			  			"broadcast",hostGroupCntPtr.gtHostOutBroadcastPkts,"HOBP");
			  /* all multicast packet */
			  vty_out(vty,"%-20s %-20s %-10d %-20s\n",memcmp(smacAddr.arEther,arEther,6) ? smacStr : "any",
			  			"multicast",hostGroupCntPtr.gtHostOutMulticastPkts,"HOMP");
			  vty_out(vty,"%-20s %-20s %-10d %-20s\n",memcmp(smacAddr.arEther,arEther,6) ? smacStr : "any",
			  			memcmp(dmacAddr.arEther,arEther,6) ? dmacStr : "any",matrixCntSaDaPkts,"HIOP");
			  #if 0
			  if((memcmp(smacAddr.arEther,arEther,6)==0)&&(memcmp(dmacAddr.arEther,arEther,6)==0)){ 	
                    vty_out(vty,"%-20s %-20s %-10d %-20s\n","any","any",hostGroupCntPtr.gtHostInPkts,"hostInPkts");
					vty_out(vty,"%-21s %-20s %-10d %-20s\n","any","any",hostGroupCntPtr.gtHostOutPkts,"hostOutPkts");
					vty_out(vty,"%-21s %-20s %-10d %-20s\n","any","broadcast",hostGroupCntPtr.gtHostOutBroadcastPkts,"hostOutBroadcastPkts");
					vty_out(vty,"%-21s %-20s %-10d %-20s\n","any","multicast",hostGroupCntPtr.gtHostOutMulticastPkts,"hostOutMulticastPkts");
			   }
			 else if((memcmp(smacAddr.arEther,arEther,6)==0)&&(memcmp(dmacAddr.arEther,arEther,6)!=0)){ 	
					vty_out(vty,"%-21s%02x:%02x:%02x:%02x:%02x:%02x    %-10d %-20s\n","any",\
						dmacAddr.arEther[0],dmacAddr.arEther[1],dmacAddr.arEther[2],dmacAddr.arEther[3]
						,dmacAddr.arEther[4],dmacAddr.arEther[5],hostGroupCntPtr.gtHostInPkts,"hostInPkts");
					vty_out(vty,"%-21s%-17s    %-10d %-20s\n","any","any",hostGroupCntPtr.gtHostOutPkts,"hostOutPkts");
					vty_out(vty,"%-21s%-17s    %-10d %-20s\n","any","broadcast",hostGroupCntPtr.gtHostOutBroadcastPkts,"hostOutBroadcastPkts");
					vty_out(vty,"%-21s%-17s    %-10d %-20s\n","any","multicast",hostGroupCntPtr.gtHostOutMulticastPkts,"hostOutMulticastPkts");
			   }
			 else if((memcmp(smacAddr.arEther,arEther,6)!=0)&&(memcmp(dmacAddr.arEther,arEther,6)==0)){
			     	vty_out(vty,"%-21s%-17s    %-10d %-20s\n","any","any",hostGroupCntPtr.gtHostInPkts,"hostInPkts");
			  		vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x    %-20s %-10d %-20s\n",\
						smacAddr.arEther[0],smacAddr.arEther[1],smacAddr.arEther[2],smacAddr.arEther[3]
						,smacAddr.arEther[4],smacAddr.arEther[5],"any",hostGroupCntPtr.gtHostOutPkts,"hostOutPkts");
					vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x    %-20s %-10d %-20s\n",\
						smacAddr.arEther[0],smacAddr.arEther[1],smacAddr.arEther[2],smacAddr.arEther[3]
						,smacAddr.arEther[4],smacAddr.arEther[5],"broadcast",hostGroupCntPtr.gtHostOutBroadcastPkts,"hostOutBroadcastPkts");
					vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x    %-20s %-10d %-20s\n",\
						smacAddr.arEther[0],smacAddr.arEther[1],smacAddr.arEther[2],smacAddr.arEther[3]
						,smacAddr.arEther[4],smacAddr.arEther[5],"multicast",hostGroupCntPtr.gtHostOutMulticastPkts,"hostOutMulticastPkts");
			   }
			 else{
			  		vty_out(vty,"%-21s%02x:%02x:%02x:%02x:%02x:%02x    %-10d %-20s\n","any",\
						dmacAddr.arEther[0],dmacAddr.arEther[1],dmacAddr.arEther[2],dmacAddr.arEther[3]
						,dmacAddr.arEther[4],dmacAddr.arEther[5],hostGroupCntPtr.gtHostInPkts,"hostInPkts");
					vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x    %-20s %-10d %-20s\n",\
						smacAddr.arEther[0],smacAddr.arEther[1],smacAddr.arEther[2],smacAddr.arEther[3]
						,smacAddr.arEther[4],smacAddr.arEther[5],"any",hostGroupCntPtr.gtHostOutPkts,"hostOutPkts");
					vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x    %-20s %-10d %-20s\n",\
						smacAddr.arEther[0],smacAddr.arEther[1],smacAddr.arEther[2],smacAddr.arEther[3]
						,smacAddr.arEther[4],smacAddr.arEther[5],"broadcast",hostGroupCntPtr.gtHostOutBroadcastPkts,"hostOutBroadcastPkts");
					vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x    %-20s %-10d %-20s\n",\
						smacAddr.arEther[0],smacAddr.arEther[1],smacAddr.arEther[2],smacAddr.arEther[3]
						,smacAddr.arEther[4],smacAddr.arEther[5],"multicast",hostGroupCntPtr.gtHostOutMulticastPkts,"hostOutMulticastPkts");
			  	}
			 #endif
			  vty_out(vty,"==========================================================================\n");
	   		  vty_out(vty,"%s",HOST_INT_PACKET_DESCRIPTOR);
	   		  vty_out(vty,"%s",HOST_OUT_PACKET_DESCRIPTOR);
	   		  vty_out(vty,"%s",HOST_Broadcast_PACKET_DESCRIPTOR);
			  vty_out(vty,"%s",HOST_Multicast_PACKET_DESCRIPTOR);
			  vty_out(vty,"%s",HOST_Matrix_PACKET_DESCRIPTOR);
		  } 
		  else 
		  {
			  if (dbus_error_is_set(&err)) 
			  {
				  vty_out(vty,"%s raised: %s\n",err.name,err.message);
				  dbus_error_free_for_dcli(&err);
			  }
		  }
	  
			 dbus_message_unref(reply);
			 return CMD_SUCCESS;  
		  
	}

DEFUN(  config_promis_port_add_to_vlan_interface_cmd_func,
		config_promis_port_add_to_vlan_interface_cmd,
		"config promis-port add-to-interface (permit|deny)",
		CONFIG_STR
		"Config promiscuous mode port\n"
		"Config add to vlan interface\n"
		"Permit promiscuous mode port to add to vlan interface\n"
		"Deny promiscuous mode port to add vlan interface\n"
	 ) 
	 
  {

   DBusMessage *query = NULL, *reply = NULL;
   DBusError err = {0};
   unsigned int op_ret = 0;
   unsigned int permit = 1;

   if(argc != 1){
   	    vty_out(vty,"Bab parameter,parameter number is not correct!\n");
		return CMD_WARNING;
   }
   else {
   	   if(0 == strncmp("permit",argv[0],strlen(argv[0]))){
           permit = 1;
	   }
	   else if(0 == strncmp("deny",argv[0],strlen(argv[0]))){
	   	   permit = 0;
	   }
	   else{
	   	   vty_out(vty,"Bab parameter,it is not 'permit' or 'deny'!\n");
		   return CMD_WARNING;
	   }
   }

   
   query = dbus_message_new_method_call(
							   NPD_DBUS_BUSNAME,	   \
							   NPD_DBUS_OBJPATH,		   \
							   NPD_DBUS_INTERFACE,		   \
							   NPD_DBUS_SYSTEM_CONFIG_PROMIS_PORT_ADD2_INTERFACE);


   dbus_error_init(&err);

   dbus_message_append_args(query,
  	                   DBUS_TYPE_UINT32,&permit,
					   DBUS_TYPE_INVALID);

   
   reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
   
   dbus_message_unref(query);
   if (NULL == reply) {
	   vty_out(vty,"failed get reply.\n");
	   if (dbus_error_is_set(&err)) {
		   vty_out(vty,"%s raised: %s",err.name,err.message);
		   dbus_error_free_for_dcli(&err);
	   }
	   return -1 ;
   }
   
	   if ((!(dbus_message_get_args ( reply, &err,
									  DBUS_TYPE_UINT32,&op_ret,
									  DBUS_TYPE_INVALID)))||\
									  (0 != op_ret)){
									  
		   if (dbus_error_is_set(&err)) 
		   {
			   vty_out(vty,"%s raised: %s\n",err.name,err.message);
			   dbus_error_free_for_dcli(&err);
		   }
		}

		  dbus_message_unref(reply);
		  return CMD_SUCCESS;  

	}

DEFUN(show_and_get_tech_support_func ,
	show_and_get_tech_support_cmd,
	"show tech-support",
	SHOW_STR
	"Show all technical support needed information\n"
)
{
	int fd = 0;

	/* test read the script file to assure it exists*/
	fd = open(SYS_TECH_SUPPORT_SCRIPT, O_RDONLY);
	if(fd < 0) {
		vty_out(vty, "%% No tech support script found\r\n");
		return CMD_WARNING;
	}
	else {
		close(fd);
	}

	system(SYS_TECH_SUPPORT_SCRIPT);

	return CMD_SUCCESS;
}

DEFUN(config_arp_muti_network_cmd_func ,
	config_arp_muti_network_cmd,
	"config arp ip-check (enable|disable)",
	CONFIG_STR
	"Config ARP configuration\n"
	"Config ARP ip-check enable or disable\n"
	"Config ARP ip-check enable\n"
	"Config ARP ip-check disable\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int isEnable = 2;
	unsigned int ret;


    if(!strncmp("disable",(char *)argv[0],strlen(argv[0]))){
		isEnable = 0;
    }
	else if(!strncmp("enable",(char *)argv[0],strlen(argv[0]))){
		isEnable = 1;
	}
	else {
		isEnable = 2;
		vty_out(vty,"%% Bad parameter : %s !\n",argv[0]);
		return CMD_WARNING;
	}
	query = dbus_message_new_method_call(
							   NPD_DBUS_BUSNAME,	   \
							   NPD_DBUS_OBJPATH,		   \
							   NPD_DBUS_INTERFACE,		   \
							   NPD_DBUS_CONFIG_ARP_MUTI_NETWORK);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &isEnable,
							DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
   	dbus_message_unref(query);
   
	if (NULL == reply) {
	   vty_out(vty, "failed get reply.\n");
	   if (dbus_error_is_set(&err)) {
		   vty_out(vty, "%s raised: %s", err.name, err.message);
		   dbus_error_free_for_dcli(&err);
	   }
	   return CMD_WARNING;
	}
   
   if ((!(dbus_message_get_args ( reply, &err,
								   DBUS_TYPE_UINT32, &ret,
								   DBUS_TYPE_INVALID)))
		|| (0 != ret)) {
		if (dbus_error_is_set(&err)) 
		{
			vty_out(vty, "%s raised: %s\n", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	dbus_message_unref(reply);

	return CMD_SUCCESS;
}

DEFUN(config_arp_inspection_cmd_func ,
	config_arp_inspection_cmd,
	"config arp inspection (enable|disable)",
	CONFIG_STR
	"Config ARP configuration\n"
	"Config ARP inspection enable or disable\n"
	"Config ARP inspection enable\n"
	"Config ARP inspection disable\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int isEnable = 2;
	unsigned int ret = 0;
	unsigned int vlanId = 0;


    if(!strncmp("disable",(char *)argv[0],strlen(argv[0]))){
		isEnable = 0;
    }
	else if(!strncmp("enable",(char *)argv[0],strlen(argv[0]))){
		isEnable = 1;
	}
	else {
		isEnable = 2;
		vty_out(vty,"%% Bad parameter : %s !\n",argv[0]);
		return CMD_WARNING;
	}	
	vlanId = (unsigned int)(vty->index);
	query = dbus_message_new_method_call(
							   NPD_DBUS_BUSNAME,	   \
							   NPD_DBUS_OBJPATH,		   \
							   NPD_DBUS_INTERFACE,		   \
							   NPD_DBUS_CONFIG_ARP_INSPECTION);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &isEnable,
							DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
   	dbus_message_unref(query);
   
	if (NULL == reply) {
	   vty_out(vty, "failed get reply.\n");
	   if (dbus_error_is_set(&err)) {
		   vty_out(vty, "%s raised: %s", err.name, err.message);
		   dbus_error_free_for_dcli(&err);
	   }
	   return CMD_WARNING;
	}
   
   if ((!(dbus_message_get_args ( reply, &err,
								   DBUS_TYPE_UINT32, &ret,
								   DBUS_TYPE_INVALID)))) {
		if (dbus_error_is_set(&err)) 
		{
			vty_out(vty, "%s raised: %s\n", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	else{
        if(0 != ret){
            vty_out(vty,"%% Config Failed!\n");
		}
	}
   

	dbus_message_unref(reply);

	return CMD_SUCCESS;
}

DEFUN(config_arp_proxy_cmd_func ,
	config_arp_proxy_cmd,
	"config arp proxy (enable|disable)",
	CONFIG_STR
	"Config ARP configuration\n"
	"Config ARP proxy enable or disable\n"
	"Config ARP proxy enable\n"
	"Config ARP proxy disable\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int isEnable = 2;
	unsigned int ret = 0;
	unsigned int vlanId = 0;


    if(!strncmp("disable",(char *)argv[0],strlen(argv[0]))){
		isEnable = 0;
    }
	else if(!strncmp("enable",(char *)argv[0],strlen(argv[0]))){
		isEnable = 1;
	}
	else {
		isEnable = 2;
		vty_out(vty,"%% Bad parameter : %s !\n",argv[0]);
		return CMD_WARNING;
	}
	vlanId = (unsigned int)(vty->index);
	query = dbus_message_new_method_call(
							   NPD_DBUS_BUSNAME,	   \
							   NPD_DBUS_OBJPATH,		   \
							   NPD_DBUS_INTERFACE,		   \
							   NPD_DBUS_CONFIG_ARP_PROXY);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &isEnable,
							DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
   	dbus_message_unref(query);
   
	if (NULL == reply) {
	   vty_out(vty, "failed get reply.\n");
	   if (dbus_error_is_set(&err)) {
		   vty_out(vty, "%s raised: %s", err.name, err.message);
		   dbus_error_free_for_dcli(&err);
	   }
	   return CMD_WARNING;
	}
   
   if ((!(dbus_message_get_args ( reply, &err,
								   DBUS_TYPE_UINT32, &ret,
								   DBUS_TYPE_INVALID)))) {
		if (dbus_error_is_set(&err)) 
		{
			vty_out(vty, "%s raised: %s\n", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	else{
        if(0 != ret){
            vty_out(vty,"%% Config Failed!\n");
		}
	}

	dbus_message_unref(reply);

	return CMD_SUCCESS;
}

#define STATIC_ARP_CONFIG_FILE_PATH "/var/run/static_arp_file"

int static_arp_entry_count(char *key)
{
	char *start_pos;
	char *end_pos;
	void* data;
	int fd = -1;
	int count = 0;
	
	struct stat sb;

	fd=open(STATIC_ARP_CONFIG_FILE_PATH,O_RDWR);
	if(fd < 0)
	{	
		return 0;
	}
	
	fstat(fd,&sb);
	data=mmap(NULL,sb.st_size,PROT_READ,MAP_PRIVATE,fd,0);
	if(MAP_FAILED==data)
	{
		close(fd);
		return -1;
	}	

	start_pos = data;
	end_pos = strstr(start_pos, key);
	
	while(end_pos)
	{
		count ++;		
		start_pos = end_pos;
		end_pos = strstr(start_pos+1, key);
	}
	
	munmap(data,sb.st_size);
	close(fd);
	return count;
}


  DEFUN(system_show_ip_neigh_func,
  		  system_show_ip_neigh_cmd,
  		  "show arp list",
  		  SHOW_STR
  		  "Show ARP information of the system\n"
  		  "sShow ARP list of the system\n")
  {	
  	unsigned int ret = 0;
	if(0 < argc){
		vty_out(vty, "%% Bad parameter!\n");
		return CMD_WARNING;
	}
  	ret = dcli_system_show_ip_neigh(vty, 0, 0);
  	if(ret){
  		return CMD_WARNING;
  	}
  	return CMD_SUCCESS;
  }
  
    DEFUN(system_show_ip_neigh_by_ip_func,
  		  system_show_ip_neigh_by_ip_cmd,
  		  "show arp by ip A.B.C.D",
  		  SHOW_STR
  		  "Show ARP information of the system\n"
  		  "Show ARP list by filter\n"
  		  "Show ARP list by ip address\n"
  		  "Show ARP list by this ip address\n")
  {	
  	unsigned int ret = 0;
	unsigned int ipaddr = 0;
	if(1 != argc){
		vty_out(vty, "%% Bad parameter!\n");
		return CMD_WARNING;
	}
	ipaddr = (unsigned int)dcli_ip2ulong((char *)argv[0]);
  	ret = dcli_system_show_ip_neigh(vty, ARP_FILTER_IP, ipaddr);
  	if(ret){
  		return CMD_WARNING;
  	}
  	return CMD_SUCCESS;
  }

	  DEFUN(system_show_ip_neigh_by_ifname_func,
			system_show_ip_neigh_by_ifname_cmd,
			"show arp by interface IFNAME",
			SHOW_STR
			"Show ARP information of the system\n"
			"Show ARP list by filter\n"
			"Show ARP list by interface\n"
			"Show ARP list by this interface\n")
	{ 
	  unsigned int ret = 0;
	  unsigned int ifindex = 0;
	  if(1 != argc){
		  vty_out(vty, "%% Bad parameter!\n");
		  return CMD_WARNING;
	  }
	  ifindex = if_nametoindex((char *)argv[0]);
	  if(!ifindex){
	  	vty_out(vty, "%% Interface %s not exists!\n", (char *)argv[0]);
		return CMD_WARNING;
	  }
	  ret = dcli_system_show_ip_neigh(vty, ARP_FILTER_IFINDEX, ifindex);
	  if(ret){
		  return CMD_WARNING;
	  }
	  return CMD_SUCCESS;
	}

	  DEFUN(system_show_ip_neigh_by_state_func,
			system_show_ip_neigh_by_state_cmd,
			"show arp by state (reachable|stale|delay|probe|noarp|permanent)",
			SHOW_STR
			"Show ARP information of the system\n"
			"Show ARP list by filter\n"
			"Show ARP list by state\n"
			"Show ARP list by rachable state\n"
			"Show ARP list by stale state\n"
			"Show ARP list by delay state\n"
			"Show ARP list by probe state\n"
			"Show ARP list by noarp state\n"
			"Show ARP list by permanent state\n")
	{ 
	  unsigned int ret = 0;
	  unsigned int state = 0;
	  if(1 != argc){
		  vty_out(vty, "%% Bad parameter!\n");
		  return CMD_WARNING;
	  }
	  if(!strncmp("reachable", (char *)argv[0], strlen((char *)argv[0]))){
		  state = 0x02;
	  }
	  else if(!strncmp("stale", (char *)argv[0], strlen((char *)argv[0]))){
	  	  state = 0x04;
	  }
	  else if(!strncmp("delay", (char *)argv[0], strlen((char *)argv[0]))){
	  	  state = 0x08;
	  }
	  else if(!strncmp("probe", (char *)argv[0], strlen((char *)argv[0]))){
	  	  state = 0x10;
	  }
	  else if(!strncmp("noarp", (char *)argv[0], strlen((char *)argv[0]))){
	  	  state = 0x40;
	  }
	  else if(!strncmp("permanent", (char *)argv[0], strlen((char *)argv[0]))){
	  	  state = 0x80;
	  }
	  else{
	  	  vty_out(vty, "%% Bad parameter: %s!\n", (char *)argv[0]);
		  return CMD_WARNING;
	  }
	  ret = dcli_system_show_ip_neigh(vty, ARP_FILTER_STATE, state);
	  if(ret){
		  return CMD_WARNING;
	  }
	  return CMD_SUCCESS;
	}
	DEFUN(system_show_ip_neigh_by_mac_func,
			  system_show_ip_neigh_by_mac_cmd,
			  "show arp by mac MAC",
			  SHOW_STR
			  "Show ARP information of the system\n"
			  "Show ARP list by filter\n"
			  "Show ARP list by mac address\n"
			  "Show ARP list by this mac,format as 00:01:02:03:04:05\n")
	  { 
		unsigned int ret = 0;
		ETHERADDR macAddr;
		if(1 != argc){
			vty_out(vty, "%% Bad parameter!\n");
			return CMD_WARNING;
		}
		memset(macAddr.arEther, 0, sizeof(ETHERADDR));
		if(NPD_FAIL == parse_mac_addr((char *)argv[0], &macAddr)){
		  vty_out(vty, "%% Parse mac address failed,or Bad parameter: %s!\n", (char *)argv[0]);
		  return CMD_WARNING;
		}
		ret = dcli_system_show_ip_neigh(vty, ARP_FILTER_MAC, macAddr.arEther);
		if(ret){
			return CMD_WARNING;
		}
		return CMD_SUCCESS;
	  }


DEFUN(show_sys_sensitive_info_cmd_func,
	show_sys_sensitive_info_cmd,
	"show system sensitive_info",
	SHOW_STR
	"Display system information\n"
	"Display system sensitive information\n"
	)
{	
	printf("the slab usage for current system:\n");
    system("slabtop -o -s -c > /mnt/slabtop.log");
	system("cat /mnt/slabtop.log | tr -d \"\015\" | tr -d \"\033\" > /mnt/slabtop");
	system("cat /mnt/slabtop");
	system("rm /mnt/slabtop.log");
	system("rm /mnt/slabtop");
		
	printf("the cache(M) for current system:\n");
	system("free -m |awk '{print $7}'");
	
	printf("the processes number for current system:\n");
	system("ps -ef|wc -l");
	printf("\n");

	system("du -sh /var");

	system("du -sh /mnt");

	system("du -sh /opt");
	printf("\n");
	printf("the main processes state for current system:\n");
	printf("PID %%CPU %%MEM  PROCESS\n");
    system("ps aux f |grep wid |grep -v _ |awk '{print $2,$3,$4,$11}'");

    system("ps aux f |grep npd |grep -v _ |grep -v dhcpsnpd |awk '{print $2,$3,$4,$11}'");

    system("ps aux f |grep eag |grep -v _ |awk '{print $2,$3,$4,$11}'");

    system("ps aux f |grep sem |grep -v _ |awk '{print $2,$3,$4,$11}'");

    system("ps aux f |grep asd |grep -v _ |awk '{print $2,$3,$4,$11}'");

    system("ps aux f |grep wsm |grep -v _ |awk '{print $2,$3,$4,$11}'");

    //printf("the wtp num for current system:\n");
	//system("sudo /opt/bin/vtysh -c \"show wtp list\" |grep \"WTPs Online\"");
	//printf("\n");

	//printf("the sta num for current system:\n");
	//system("sudo /opt/bin/vtysh -c \"show sta summary\" |grep \"Total accessed sta now\"");
	//printf("\n");
	
    return CMD_SUCCESS;
}

	

	int dcli_system_show_ip_neigh
		(
			struct vty* vty,
			unsigned int filterType,
			unsigned int filter
		)
		{
		   
		
		DBusMessage *query, *reply;
		DBusMessageIter  iter;
		DBusMessageIter	 iter_array;
		DBusMessageIter iter_struct;
		DBusError err;
		int i;
		int count = -1;
		unsigned int haveMore = 0;
		unsigned int ipAddr = 0;
	unsigned char * ifName = NULL,*dst_ifname = NULL;
		unsigned char macAddr[MAC_ADDRESS_LEN] = {0};
		unsigned int state = 0;
		unsigned int ret = 0;
		int function_type = -1;
		char file_path[64] = {0};
		unsigned char f_macAddr[MAC_ADDRESS_LEN] = {0};
		unsigned char stateStr[128] = {0};
		unsigned char * tableTitle = "IP NEIGHBOUR TABLE:\n";
		unsigned char * startLine  = "===================================================================\n";
		unsigned char * septalLine = "-------------------------------------------------------------------\n";
#define HEADER_TITLES "%-3s%13s%-5s%14s%-8s%10s%-7s\n", " IP","","  MAC","","  ifname","","  state"		
		unsigned char * endLine = startLine;
		if(ARP_FILTER_MAC == filterType){
			if(filter){
				memcpy(f_macAddr, (char *)filter, MAC_ADDRESS_LEN);
			}
			else{
				vty_out(vty, "%% Bad parameter: null pointer!\n");
				return CMD_WARNING;
			}
		}

		query = dbus_message_new_method_call(\												 
											 NPD_DBUS_BUSNAME,	   \
											 NPD_DBUS_OBJPATH,		   \
											 NPD_DBUS_INTERFACE,			 \
											 NPD_DBUS_SYSTEM_SHOW_IP_NEIGH);

		dbus_error_init(&err);

		dbus_message_iter_init_append(query, &iter);
		
		dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &filterType);
		if(ARP_FILTER_MAC == filterType){
			dbus_message_iter_append_basic(&iter,
										DBUS_TYPE_BYTE, &f_macAddr[0]);
			dbus_message_iter_append_basic(&iter,
										DBUS_TYPE_BYTE, &f_macAddr[1]);
			dbus_message_iter_append_basic(&iter,
										DBUS_TYPE_BYTE, &f_macAddr[2]);
			dbus_message_iter_append_basic(&iter,
										DBUS_TYPE_BYTE, &f_macAddr[3]);
			dbus_message_iter_append_basic(&iter,
										DBUS_TYPE_BYTE, &f_macAddr[4]);
			dbus_message_iter_append_basic(&iter,
										DBUS_TYPE_BYTE, &f_macAddr[5]);
		}
	else if(ARP_FILTER_NONE != filterType)
	{			
		dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &filter);
		if(ARP_FILTER_IFINDEX == filterType)
		{
			dst_ifname = (char *)malloc(MAX_IFNAME_LEN+1);
			memset(dst_ifname,0,MAX_IFNAME_LEN+1);
			if_indextoname(filter,dst_ifname);
			dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_STRING, &dst_ifname);
			free(dst_ifname);
			dst_ifname = NULL;
		}
	}
		
     	if (is_distributed) 
		{	
			
        	for (i = 0;i < MAX_SLOT; i++ ) 
			{
				sprintf(file_path,"/dbm/product/slot/slot%d/function_type", i);
				function_type = get_product_info(file_path);
				
				if (function_type == SWITCH_BOARD)
					continue;
				
         	    if (NULL != dbus_connection_dcli[i]->dcli_dbus_connection)
           		   {
           		   
                    	reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[i]->dcli_dbus_connection,query,-1, &err);
                 //   	dbus_message_unref(query);
       
                    	if (NULL == reply)
                    	{
                    		printf("failed get reply.\n");
                    		if (dbus_error_is_set(&err))
                    		{
                    			printf("%s raised: %s",err.name,err.message);
                    			dbus_error_free_for_dcli(&err);
                    		}
                    		ret = 1;
                    		return ret;
                    	}	
                    	dbus_message_iter_init(reply,&iter);
                    	dbus_message_iter_get_basic(&iter, &count);
						dbus_message_iter_next(&iter);
                    	
                    	dbus_message_iter_recurse(&iter,&iter_array);
                    	
                    	dbus_message_iter_recurse(&iter_array,&iter_struct);
                    	
                    	dbus_message_iter_get_basic(&iter_struct,&haveMore);
                    	dbus_message_iter_next(&iter_struct);	
                    	
                    	dbus_message_iter_get_basic(&iter_struct,&ipAddr);
                    	dbus_message_iter_next(&iter_struct);
                    	
                    	dbus_message_iter_get_basic(&iter_struct,&ifName);
                    	dbus_message_iter_next(&iter_struct);
                    	
                    	dbus_message_iter_get_basic(&iter_struct,&macAddr[0]);				
                    	dbus_message_iter_next(&iter_struct);
                    	
                    	dbus_message_iter_get_basic(&iter_struct,&macAddr[1]);
                    	dbus_message_iter_next(&iter_struct);
                    	
                    	dbus_message_iter_get_basic(&iter_struct,&macAddr[2]);
                    	dbus_message_iter_next(&iter_struct);
                    
                    	dbus_message_iter_get_basic(&iter_struct,&macAddr[3]);
                    	dbus_message_iter_next(&iter_struct);
                    
                    	dbus_message_iter_get_basic(&iter_struct,&macAddr[4]);				
                    	dbus_message_iter_next(&iter_struct);
                    
                    	dbus_message_iter_get_basic(&iter_struct,&macAddr[5]);				
                    	dbus_message_iter_next(&iter_struct);
                 
                    	dbus_message_iter_get_basic(&iter_struct,&state);
                    	dbus_message_iter_next(&iter_struct);
                    
                    	dbus_message_iter_next(&iter_array);
                    	
                    	if(!haveMore){
                    		dbus_message_iter_next(&iter);
                    		dbus_message_iter_get_basic(&iter,&ret);
                    		dbus_message_iter_next(&iter);	
                    		if(ARP_RETURN_CODE_SUCCESS != ret){
                    			vty_out(vty, "%% Failed to get ip neighbour table!\n");
                    			ret = 1;
                    		}
                    		else{/* output empty tables*/
								vty_out(vty, "\n\nIn slot %d, ", i);
                    			vty_out(vty, tableTitle);
								/*f (filterType == ARP_FILTER_NONE)*/
								vty_out(vty, "arp entry count:%d\n",count);
                    			vty_out(vty, startLine);
                    			vty_out(vty, HEADER_TITLES);
                    			vty_out(vty, septalLine);
                    			vty_out(vty, endLine);	
                    		}
                    	}
                    	else{
							vty_out(vty, "\n\nIn slot %d, ", i);
                    		vty_out(vty, tableTitle);
							/*if (filterType == ARP_FILTER_NONE)*/
							vty_out(vty, "arp entry count:%d\n",count);	
                    		vty_out(vty, startLine);
                    		vty_out(vty, HEADER_TITLES);
                    		vty_out(vty, septalLine);
                    		while(haveMore){
								if (!(macAddr[0]==0 && macAddr[1] == 0 && macAddr[2] == 0 && macAddr[3] == 0 && macAddr[4] == 0&& macAddr[5] == 0)) {
                            			if(ipAddr && ((!state) || (state & (~0x21)))){ /*state is none, or not only incomplete and failed*/
                            				vty_out(vty, " %3d.%3d.%3d.%3d", (ipAddr>>24)&0xFF,(ipAddr>>16)&0xFF,(ipAddr>>8)&0xFF,ipAddr&0xFF);
                            				vty_out(vty, "  %.2x:%.2x:%.2x:%.2x:%.2x:%.2x", macAddr[0],macAddr[1],macAddr[2],macAddr[3],macAddr[4],macAddr[5]);
                            				vty_out(vty, "  %-16s", ifName ? ifName : "null");
                            				vty_out(vty, "  %s\n", arp_state_str(state,stateStr));	
                            			}
								}
                    			ipAddr = 0;
                    			ifName = NULL;
                    			memset(macAddr, 0, 6);
                    			state = 0;
                    			memset(stateStr, 0, 128);
                    			haveMore = 0;
                    			
                    			dbus_message_iter_recurse(&iter_array,&iter_struct);
                    			
                    			dbus_message_iter_get_basic(&iter_struct,&haveMore);
                    			dbus_message_iter_next(&iter_struct);	
                    			
                    			dbus_message_iter_get_basic(&iter_struct,&ipAddr);
                    			dbus_message_iter_next(&iter_struct);
                    			
                    			dbus_message_iter_get_basic(&iter_struct,&ifName);
                    			dbus_message_iter_next(&iter_struct);
                    			
                    			dbus_message_iter_get_basic(&iter_struct,&macAddr[0]);				
                    			dbus_message_iter_next(&iter_struct);
                    			
                    			dbus_message_iter_get_basic(&iter_struct,&macAddr[1]);
                    			dbus_message_iter_next(&iter_struct);
                    			
                    			dbus_message_iter_get_basic(&iter_struct,&macAddr[2]);
                    			dbus_message_iter_next(&iter_struct);
                    			
                    			dbus_message_iter_get_basic(&iter_struct,&macAddr[3]);
                    			dbus_message_iter_next(&iter_struct);
                    			
                    			dbus_message_iter_get_basic(&iter_struct,&macAddr[4]);				
                    			dbus_message_iter_next(&iter_struct);
                    			
                    			dbus_message_iter_get_basic(&iter_struct,&macAddr[5]);				
                    			dbus_message_iter_next(&iter_struct);
                    			
                    			dbus_message_iter_get_basic(&iter_struct,&state);
                    			dbus_message_iter_next(&iter_struct);
                    
                    			dbus_message_iter_next(&iter_array);
                    			
                    		}
                    		
                    		vty_out(vty, endLine);
                    		
                    		dbus_message_iter_next(&iter);
                    		dbus_message_iter_get_basic(&iter,&ret);
                    		dbus_message_iter_next(&iter);
                    		if(ARP_RETURN_CODE_SUCCESS != ret){
                    			vty_out(vty,"%% Some error occurred while getting ip neighbour on slot %d!\n", i);
                    			//ret = 1;
                    		}
                    	}
                    //	dbus_message_unref(reply);
                    			
                 }
        	}			
			dbus_message_unref(query);
			dbus_message_unref(reply);
	  }

	  else 
    	{
    		
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
			ret = 1;
			return ret;
		}
				
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter, &count);
		dbus_message_iter_next(&iter);
		
		dbus_message_iter_recurse(&iter,&iter_array);
		
		dbus_message_iter_recurse(&iter_array,&iter_struct);
		
		dbus_message_iter_get_basic(&iter_struct,&haveMore);
		dbus_message_iter_next(&iter_struct);	
		
		dbus_message_iter_get_basic(&iter_struct,&ipAddr);
		dbus_message_iter_next(&iter_struct);
		
		dbus_message_iter_get_basic(&iter_struct,&ifName);
		dbus_message_iter_next(&iter_struct);
		
		dbus_message_iter_get_basic(&iter_struct,&macAddr[0]);				
		dbus_message_iter_next(&iter_struct);
		
		dbus_message_iter_get_basic(&iter_struct,&macAddr[1]);
		dbus_message_iter_next(&iter_struct);
		
		dbus_message_iter_get_basic(&iter_struct,&macAddr[2]);
		dbus_message_iter_next(&iter_struct);
		
		dbus_message_iter_get_basic(&iter_struct,&macAddr[3]);
		dbus_message_iter_next(&iter_struct);
		
		dbus_message_iter_get_basic(&iter_struct,&macAddr[4]);				
		dbus_message_iter_next(&iter_struct);
		
		dbus_message_iter_get_basic(&iter_struct,&macAddr[5]);				
		dbus_message_iter_next(&iter_struct);
		
		dbus_message_iter_get_basic(&iter_struct,&state);
		dbus_message_iter_next(&iter_struct);

		dbus_message_iter_next(&iter_array);
		
		if(!haveMore){
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&ret);
			dbus_message_iter_next(&iter);	
			if(ARP_RETURN_CODE_SUCCESS != ret){
				vty_out(vty, "%% Failed to get ip neighbour table!\n");
				ret = 1;
			}
			else{/* output empty tables*/
				vty_out(vty, tableTitle);
				if (filterType == ARP_FILTER_NONE)
					vty_out(vty, "arp entry count:%d\n",count);
				vty_out(vty, startLine);
				vty_out(vty, HEADER_TITLES);
				vty_out(vty, septalLine);
				vty_out(vty, endLine);	
			}
		}
		else{
			vty_out(vty, tableTitle);
			if (filterType == ARP_FILTER_NONE)
					vty_out(vty, "arp entry count:%d\n",count);
			vty_out(vty, startLine);
			vty_out(vty, HEADER_TITLES);
			vty_out(vty, septalLine);
			while(haveMore){					
				if(ipAddr && ((!state) || (state & (~0x21)))){ /*state is none, or not only incomplete and failed*/
					vty_out(vty, " %3d.%3d.%3d.%3d", (ipAddr>>24)&0xFF,(ipAddr>>16)&0xFF,(ipAddr>>8)&0xFF,ipAddr&0xFF);
					vty_out(vty, "  %.2x:%.2x:%.2x:%.2x:%.2x:%.2x", macAddr[0],macAddr[1],macAddr[2],macAddr[3],macAddr[4],macAddr[5]);
					vty_out(vty, "  %-16s", ifName ? ifName : "null");
					vty_out(vty, "  %s\n", arp_state_str(state,stateStr));	
				}
				
				ipAddr = 0;
				ifName = NULL;
				memset(macAddr, 0, 6);
				state = 0;
				memset(stateStr, 0, 128);
				haveMore = 0;
				
				dbus_message_iter_recurse(&iter_array,&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct,&haveMore);
				dbus_message_iter_next(&iter_struct);	
				
				dbus_message_iter_get_basic(&iter_struct,&ipAddr);
				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct,&ifName);
				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct,&macAddr[0]);				
				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct,&macAddr[1]);
				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct,&macAddr[2]);
				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct,&macAddr[3]);
				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct,&macAddr[4]);				
				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct,&macAddr[5]);				
				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct,&state);
				dbus_message_iter_next(&iter_struct);

				dbus_message_iter_next(&iter_array);
				
			}
			
			vty_out(vty, endLine);
			
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&ret);
			dbus_message_iter_next(&iter);
			if(ARP_RETURN_CODE_SUCCESS != ret){
				vty_out(vty,"%% Some error occurred while getting ip neighbour!\n");
				ret = 1;
			}
		}
		dbus_message_unref(reply);
		return ret;
	}
}
static unsigned char * arp_state_str(unsigned int state, unsigned char * outStr){
	
	unsigned char * ptr = outStr;
	int flag = 0;
	if(!outStr){
		return "null pointer";
	}
	if(!state){
		sprintf(outStr, "none");
		return outStr;
	}
	else{
		if(state & 0x01){
			ptr += sprintf(ptr, "%sincomplete", flag ? ",":"");
			flag = 1;
		}
		if(state & 0x02){
			ptr += sprintf(ptr, "%sreachable", flag ? ",":"");
			flag = 1;
		}
		if(state & 0x04){
			ptr += sprintf(ptr, "%sstale", flag ? ",":"");
			flag = 1;
		}
		if(state & 0x08){
			ptr += sprintf(ptr, "%sdelay", flag ? ",":"");
			flag = 1;
		}
		if(state & 0x10){
			ptr += sprintf(ptr, "%sprobe", flag ? ",":"");
			flag = 1;
		}
		if(state & 0x20){
			ptr += sprintf(ptr, "%sfailed", flag ? ",":"");
			flag = 1;
		}
		if(state & 0x40){
			ptr += sprintf(ptr, "%snoarp", flag ? ",":"");
			flag = 1;
		}
		if(state & 0x80){
			ptr += sprintf(ptr, "%spermanent", flag ? ",":"");
			flag = 1;
		}
	}
	return outStr;
}

DEFUN(show_system_process_status_cmd_func,
	show_system_process_status_cmd,
	"show system process-status (PROCESS_NAME |all) [number]",
	"Show-command\n"
	"Show system infomation\n"
	"Display process-status information\n"
	"Name of process which you want check out\n"
	"All processes\n"
	"Display the number of target process or all process\n")
{
	int all_processes;
	int process_number;
	char cmdstr[256] = {0};
	
	if (strncmp(argv[0], "all", (strlen(argv[0]))) == 0)
		all_processes = 1;
	else
		all_processes = 0;

	if (argc == 1) 
		process_number = 0;
	else
		process_number = 1;

	if (!process_number) {
		if (all_processes) {
			memset(cmdstr, 0, 256);
			sprintf(cmdstr, "ps -ef");
			system(cmdstr);
		}
		else {
			memset(cmdstr, 0, 256);
			sprintf(cmdstr, "ps -ef | grep \'%s\' | grep -v \'grep\'", argv[0]);
			vty_out(vty, "UID        PID  PPID  C STIME TTY          TIME CMD\n");
			system(cmdstr);
		}
	}
	else {
		if (all_processes) {
			memset(cmdstr, 0, 256);
			sprintf(cmdstr, "ps -ef | wc -l");
			system(cmdstr);
		}
		else {
			memset(cmdstr, 0, 256);
			sprintf(cmdstr, "ps -ef | grep \'%s\' | grep -v \'grep\' | wc -l", argv[0]);
			system(cmdstr);
		}
	}

	return CMD_SUCCESS;
}

DEFUN(show_system_process_psr_cmd_func,
	show_system_process_psr_cmd,
	"show system process-psr PROCESS_NAME",
	"Show-command\n"
	"Show system infomation\n"
	"Display processor that process is currently assigned to\n"
	"Name of process which you want check out\n")
{
	char cmdstr[256] = {0};

	sprintf(cmdstr, "ps -eo cmd,pid,ppid,psr | grep \'%s\' | grep -v \'grep\'", argv[0]);
	vty_out(vty, "CMD                           PID  PPID PSR\n");
	system(cmdstr);

	return CMD_SUCCESS;
}

DEFUN(set_process_affinity_mask_cmd_func,
	set_process_affinity_mask_cmd,
	"set process_affinity_mask PID MASK",
	"Set system configuration\n"
	"Set process affinity mask function\n"
	"PID of process\n"
	"Affinity mask that you want to set\n")
{
	char cmdstr[128] = {0};
	unsigned int mask = 0x0;
	unsigned int pid = 0x0;

	if (!strncmp(argv[1], "0x", 2)) {
		mask = str2hex((unsigned char *)(argv[1] + 2));
	}
	else {
		sscanf(argv[1], "%d", &mask);
	}

	pid = atoi(argv[0]);
	if (pid > 0x8000) {
		vty_out(vty, "PID out of range\n");
		return CMD_WARNING;
	}

	sprintf(cmdstr, "taskset -p %#x %s", mask, argv[0]);
	system(cmdstr);

	return CMD_SUCCESS;
}

void dcli_system_init(void) {
	install_element(VIEW_NODE,&show_sys_version_cmd);
	install_element(VIEW_NODE,&show_sys_hwconf_cmd);
	
	install_element(VIEW_NODE,&show_sys_environment_cmd);	
	install_element(VIEW_NODE,&system_show_ip_neigh_cmd);
	install_element(VIEW_NODE,&system_show_ip_neigh_by_ip_cmd);
	install_element(VIEW_NODE,&system_show_ip_neigh_by_ifname_cmd);
	install_element(VIEW_NODE,&system_show_ip_neigh_by_state_cmd);
	install_element(VIEW_NODE,&system_show_ip_neigh_by_mac_cmd);
	install_element(ENABLE_NODE,&show_sys_version_cmd);
	install_element(ENABLE_NODE,&show_sys_hwconf_cmd);
	
	install_element(ENABLE_NODE,&show_sys_environment_cmd);
	install_element(ENABLE_NODE,&system_show_ip_neigh_cmd);
	install_element(ENABLE_NODE,&system_show_ip_neigh_by_ip_cmd);
	install_element(ENABLE_NODE,&system_show_ip_neigh_by_ifname_cmd);
	install_element(ENABLE_NODE,&system_show_ip_neigh_by_state_cmd);
	install_element(ENABLE_NODE,&system_show_ip_neigh_by_mac_cmd);
    install_element(CONFIG_NODE,&debug_npd_info_cmd);
	install_element(CONFIG_NODE,&no_debug_npd_info_cmd);
	install_element(CONFIG_NODE,&debug_npd_pkt_info_cmd);
	install_element(CONFIG_NODE,&no_debug_npd_pkt_info_cmd);
	install_element(CONFIG_NODE,&show_system_counter_drop_cmd);
	install_element(CONFIG_NODE,&show_sys_cnt_egress_cmd);
	
	install_element(CONFIG_NODE,&show_sys_version_cmd);
	install_element(CONFIG_NODE,&show_sys_hwconf_cmd);
	
	install_element(CONFIG_NODE,&show_sys_environment_cmd);
	install_element(CONFIG_NODE,&system_show_ip_neigh_cmd);
	install_element(CONFIG_NODE,&system_show_ip_neigh_by_ip_cmd);
	install_element(CONFIG_NODE,&system_show_ip_neigh_by_ifname_cmd);
	install_element(CONFIG_NODE,&system_show_ip_neigh_by_state_cmd);
	install_element(CONFIG_NODE,&system_show_ip_neigh_by_mac_cmd);
	install_element(CONFIG_NODE,&config_sys_shut_down_state_cmd);
/*	all controled by debug npd commands
	install_element(CONFIG_NODE,&debug_nam_info_cmd);
	install_element(CONFIG_NODE,&no_debug_nam_info_cmd);

	install_element(CONFIG_NODE,&debug_nbm_info_cmd);
	install_element(CONFIG_NODE,&no_debug_nbm_info_cmd);

	install_element(CONFIG_NODE,&debug_asic_on_cmd);
	install_element(CONFIG_NODE,&debug_asic_off_cmd);
	install_element(CONFIG_NODE,&debug_nam_pkt_on_cmd);
	install_element(CONFIG_NODE,&debug_nam_pkt_off_cmd);*/
	install_element(CONFIG_NODE,&config_arp_smac_check_cmd);
	install_element(CONFIG_NODE,&config_arp_strict_check_cmd);
	/*install_element(CONFIG_NODE,&config_cpu_protection_cmd);*/
	install_element(CONFIG_NODE,&config_set_cpu_queue_fc_cmd);
	install_element(CONFIG_NODE,&config_set_cpufc_quota_cmd);
	install_element(CONFIG_NODE,&config_get_cpufc_quota_cmd);
	install_element(CONFIG_NODE,&config_set_cpufc_shaper_cmd);
	install_element(CONFIG_NODE,&config_set_port_cpufc_shaper_cmd);
	install_element(CONFIG_NODE,&config_system_counter_drop_cmd);
	install_element(CONFIG_NODE,&config_brg_pt_cnt_egress_cmd);
	install_element(CONFIG_NODE,&config_brg_host_dmac_cnt_ingress_cmd);
	install_element(CONFIG_NODE,&config_brg_host_smac_cnt_ingress_cmd);
	install_element(CONFIG_NODE,&config_brg_host_cnt_ingress_cmd);
	install_element(CONFIG_NODE,&show_sys_cnt_ingress_cmd);
	install_element(CONFIG_NODE,&config_brg_vln_cnt_egress_cmd);	
	install_element(CONFIG_NODE,&config_arp_inspection_cmd);
	install_element(CONFIG_NODE,&config_arp_proxy_cmd);
	install_element(CONFIG_NODE,&show_sys_sensitive_info_cmd);
	install_element(HIDDENDEBUG_NODE,&config_set_cpu_queue_fc_cmd);
	install_element(HIDDENDEBUG_NODE,&config_set_cpufc_quota_cmd);
	install_element(HIDDENDEBUG_NODE,&config_get_cpufc_quota_cmd);
	install_element(HIDDENDEBUG_NODE,&config_set_cpufc_shaper_cmd);
	install_element(HIDDENDEBUG_NODE,&config_set_port_cpufc_shaper_cmd);
	install_element(HIDDENDEBUG_NODE,&show_cpu_all_queue_desc_cmd);
	install_element(HIDDENDEBUG_NODE,&show_cpu_single_queue_desc_cmd);
	install_element(HIDDENDEBUG_NODE,&show_cpu_port_statistic_cmd);
	install_element(HIDDENDEBUG_NODE,&clear_cpu_port_statistic_cmd);
	install_element(HIDDENDEBUG_NODE,&config_arp_aging_broadcast_cmd);
	install_element(HIDDENDEBUG_NODE,&config_promis_port_add_to_vlan_interface_cmd);
	install_element(HIDDENDEBUG_NODE,&show_and_get_tech_support_cmd);	
	install_element(HIDDENDEBUG_NODE,&config_arp_muti_network_cmd);
	install_element(HIDDENDEBUG_NODE,&show_system_process_status_cmd);
	install_element(HIDDENDEBUG_NODE,&show_system_process_psr_cmd);
	install_element(HIDDENDEBUG_NODE,&set_process_affinity_mask_cmd);
}

#ifdef __cplusplus
}
#endif
