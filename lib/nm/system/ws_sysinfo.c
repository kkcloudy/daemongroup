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
* ws_sysinfo.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* function for web
*
*
***************************************************************************/
#ifdef __cplusplus
	extern "C"
	{
#endif

#include "ws_sysinfo.h"
/*#include "cgic.h"*/
#include <sys/ioctl.h>
#include <stdio.h>
#include "ws_usrinfo.h"
//#include "ws_ec.h"
#include <string.h>
#include <fcntl.h> 
#include <sys/stat.h> 
#include <stdlib.h>
#include "ws_public.h"
#include <ctype.h>
#include "bsd/bsdpub.h"
#include "dbus/bsd/BsdDbusPath.h"
#include "ws_dbus_def.h"
#include "ws_dbus_list_interface.h"


int show_file_string(char *FILENAME,char *zstr);


//extern char *slot_status_str[MODULE_STAT_MAX];

#if 0
char *slot_status_str[MODULE_STAT_MAX] = {
	"NONE",
	"INITING",
	"RUNNING",
	"DISABLED"
};
#endif


int show_sys_ver(struct sys_ver *SysV) /*显示系统信息，成功返回0，失败返回-1*/
{
	DBusMessage *query, *reply;
	DBusError err;

    int retu=0;
	unsigned int product_id;
	unsigned int sw_version;
	char *product_name = NULL;
	char *base_mac = NULL;
	char *serial_no = NULL;
	char *swname = NULL;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_OBJPATH,NPD_DBUS_INTERFACE,NPD_DBUS_INTERFACE_METHOD_VER);

	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return -1;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&product_id,
		DBUS_TYPE_UINT32,&sw_version,
		DBUS_TYPE_STRING,&product_name,
		DBUS_TYPE_STRING,&base_mac,
		DBUS_TYPE_STRING,&serial_no,
		DBUS_TYPE_STRING,&swname,
		DBUS_TYPE_INVALID)) {	
		SysV->swname=(char *)malloc(50);
		if(SysV->swname)
		{
			memset(SysV->swname,0,50);
			strcpy(SysV->swname,swname);
		}
		SysV->sw_version=sw_version;
		//add 2009.2.27
		SysV->product_id=product_id;
		//changed 2008.6.26.pm
		//if (SW_COMPATIBLE_VER(sw_version) > 0 ) {}

		SysV->product_name=(char *)malloc(50);
		if(SysV->product_name)
		{
			memset(SysV->product_name,0,50);
			strcpy(SysV->product_name,product_name);
		}
		SysV->base_mac=(char *)malloc(50);
		if(SysV->base_mac)
		{
			memset(SysV->base_mac,0,50);
			strcpy(SysV->base_mac,base_mac);
		}
		SysV->serial_no=(char *)malloc(50);
		if(SysV->serial_no)
		{
			memset(SysV->serial_no,0,50);
			strcpy(SysV->serial_no,serial_no);
		}
		//software name
		{
        	char software_name_file[256]="";

        	if(access("/devinfo/software_name",0)==0)
            {
            	sprintf( software_name_file, "if [ -f /devinfo/software_name ];then cat /devinfo/software_name 2>/dev/null;fi" );
            	GET_CMD_STDOUT(SysV->sw_name,sizeof(SysV->sw_name),software_name_file);
            }
			
        }

		

		//product name
		{
        	char product_name_file[256]="";

        	if(access("/devinfo/product_name",0)==0)
    		{
            	sprintf( product_name_file, "if [ -f /devinfo/product_name ];then cat /devinfo/product_name 2>/dev/null;fi" );
            	GET_CMD_STDOUT(SysV->sw_product_name,sizeof(SysV->sw_product_name),product_name_file);
            }
		}

		
			
		//version
        {
        	char get_sw_ver_cmd[256]="";

        	if(access("/etc/version/verstring",0)==0)
            {
            	//sprintf( get_sw_ver_cmd, "if [ -f /etc/version/verstring ];then cat /etc/version/verstring 2>/dev/null;fi" );
            	sprintf( get_sw_ver_cmd, "if [ -f /etc/version/rversion ];then cat /etc/version/rversion 2>/dev/null;fi" );
            	GET_CMD_STDOUT(SysV->sw_version_str,sizeof(SysV->sw_version_str),get_sw_ver_cmd);
            }
        }


		
			
		//mac address
		{
        	char mac_address_file[256]="";
			
        	if(access("/devinfo/mac",0)==0)
            {
            	sprintf( mac_address_file, "if [ -f /devinfo/mac ];then cat /devinfo/mac 2>/dev/null;fi" );
            	GET_CMD_STDOUT(SysV->sw_mac,sizeof(SysV->sw_mac),mac_address_file);
            }
        }

		
			
		//serial number
		{
        	char serial_num_file[256]="";
			
        	if(access("/devinfo/sn",0)==0)
            {
            	sprintf( serial_num_file, "if [ -f /devinfo/sn ];then cat /devinfo/sn 2>/dev/null;fi" );
            	GET_CMD_STDOUT(SysV->sw_serial_num,sizeof(SysV->sw_serial_num),serial_num_file);
            }
        }
		
		
	} else {
		if (dbus_error_is_set(&err)) {
				dbus_error_free(&err);
		}
		retu=-1;
	}
		
	dbus_message_unref(reply);
	return retu;
}


/*---------------------------------------------------------
DEFUN(show_sys_hwconf_cmd_func,
	show_sys_hwconf_cmd,
	"show system hardware-configuration",
	SHOW_STR
	"Display system information\n"
	"Display system hardware-configuration information\n"
	)
  ---------------------------------------------------------
*/
#if 0

int show_hardware_configuration()
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	unsigned char slot_count = 0;
	int i;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_OBJPATH,NPD_DBUS_INTERFACE,NPD_DBUS_INTERFACE_METHOD_HWCONF);
	

	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return -1;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&slot_count);
	
	fprintf(cgiOut,"<tr height=25 bgcolor=#eaeff9 style=font-size:14px>");
	if (1 == slot_count) {
		//vty_out(vty,"%-21s","PRODCTNAME");
		fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","PRODCTNAME");
	} else {
    	//vty_out(vty,"%-5s%-10s%-20s","SLOT","STATUS","MODULE NAME");
		fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","SLOT");
		fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","STATUS");
		fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","MODULE NAME");
	}
	//vty_out(vty,"%-22s%-6s%-11s\n","SN","HWVER","EXT SLOTNUM");
	//fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","SN");
	//fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","HWVER");
	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","ICM");
	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","EXT SLOTNUM");
	fprintf(cgiOut,"</tr>");

	
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
		fprintf(cgiOut,"<tr height=25 >");
		if (1 == slot_count) {
			//vty_out(vty," %-20s",modname);
			char key[128];
			memset(key,0,128);
			show_file_string(DEVINFO_PRODUCT_NAME,key); 
			fprintf(cgiOut,"<td align=center>%s</td>",key);
						
			

		} else {
			//vty_out(vty," %-4d%-10s%-20s",slotno,slot_status_str[module_status],(MODULE_STAT_NONE != module_status) ? modname:"-");
			fprintf(cgiOut,"<td align=center>%d</td>",slotno);
			fprintf(cgiOut,"<td align=center>%s</td>",slot_status_str[module_status]);
			fprintf(cgiOut,"<td align=center>%s</td>",(MODULE_STAT_NONE != module_status) ? modname:"-");
		}
		if(MODULE_STAT_NONE == module_status) {
			//vty_out(vty,"%-22s%-6s%-11s\n","-","-","-");
			fprintf(cgiOut,"<td align=center>%s</td>","-");
			fprintf(cgiOut,"<td align=center>%s</td>","-");
			//fprintf(cgiOut,"<td align=center>%s</td>","-");
		}
		else {
			//vty_out(vty,"%20s##%02d.%02d %-11d\n",(MODULE_STAT_NONE != module_status) ? sn:"-",HW_PCB_VER(hw_ver),HW_CPLD_VER(hw_ver),ext_slot_num);

			fprintf(cgiOut,"<td align=center>%20s##%02d.%02d</td>",(MODULE_STAT_NONE != module_status) ? sn:"-",HW_PCB_VER(hw_ver),HW_CPLD_VER(hw_ver));
			fprintf(cgiOut,"<td align=center>%d</td>",ext_slot_num);
		}
	}
		

	dbus_message_unref(reply);
	
	
	return 0;
}
#endif

/*-------------------------------------
DEFUN(show_running_boot_img_func,
	show_running_boot_img_func_cmd,
	"show system boot_img",
	SHOW_STR
	"Show system infomation\n"
	"Show running boot img infomation\n"
	)
  -------------------------------------
	*/
int show_system_bootimg(struct sys_img *bootimg)
{
	char imgname[128] ;
	int ret;

	memset(imgname,0,128);
	ret = get_bootimg_name(imgname);
	if(0==ret)
	{
		//vty_out(vty,"The current boot file is %s\n",imgname);
	
		//bootimg->boot_img=imgname;	

		memset(bootimg->boot_img,0,50);
	    strcpy(bootimg->boot_img,imgname);
		
		return 0;
	}
	else if(1==ret)
	{
		//vty_out(vty,"Can't open the file\n");
		return 1;
	
	}
	else
	{
		//vty_out(vty,"Can't get the file name\n");
		return 2;
	}

}

    

//重写新的函数来获取产品ID

 int get_product_id()
{
	DBusMessage *query, *reply;
	DBusError err;

   // int retu=0;
	unsigned int product_id;
	unsigned int sw_version;
	char *product_name = NULL;
	char *base_mac = NULL;
	char *serial_no = NULL;
	char *swname = NULL;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_OBJPATH,NPD_DBUS_INTERFACE,NPD_DBUS_INTERFACE_METHOD_VER);

	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		//return -1;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&product_id,
		DBUS_TYPE_UINT32,&sw_version,
		DBUS_TYPE_STRING,&product_name,
		DBUS_TYPE_STRING,&base_mac,
		DBUS_TYPE_STRING,&serial_no,
		DBUS_TYPE_STRING,&swname,
		DBUS_TYPE_INVALID)) {	
		
		//pro->product_id=product_id;
		
		
	} else {
		if (dbus_error_is_set(&err)) {
				dbus_error_free(&err);
		}
		//retu=-1;
	}
		
	dbus_message_unref(reply);
	//return retu;
	return product_id;
}


int get_bootimg_name(char* imgname)
{
	int fd; 
	//FILE *fd;
	int retval; 
	boot_env_t	env_args;
	char *name = "bootfile";

	memset( &env_args, 0, sizeof(env_args) );
	sprintf(env_args.name,name);
	env_args.operation = GET_BOOT_ENV;

	fd = open("/dev/bm0",0);	
	if(fd < 0)
	{ 	
		return 1;	
	}		
	retval = ioctl(fd,BM_IOC_ENV_EXCH,&env_args);

	if(retval == -1)
		{		
			close(fd);
			return 2;	
		}
	else
		{		
			sprintf(imgname,env_args.value); 
		}
		close(fd);
		return 0;


}

//因为嵌入页面代码，show_hardware_configuration无法再次调用，因此重写
int show_sys_hwconf( hw_info * sys_head,int *slot_number)//if return SN equals with "-",ex_slotnum print "-",return 0 means successful,-1 means fail
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	unsigned char slot_count = 0;
	int i;
	hw_info *q,*tail;
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_OBJPATH,NPD_DBUS_INTERFACE,NPD_DBUS_INTERFACE_METHOD_HWCONF);
	

	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return -1;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&slot_count);
	#if 0
	if (1 == slot_count) 
	{
		vty_out(vty,"%-21s","PRODCTNAME");
	} 
	else 
	{
    		vty_out(vty,"%-5s%-10s%-20s","SLOT","STATUS","MODULE NAME");
	}
	vty_out(vty,"%-22s%-6s%-12s\n","SN","HWVER","EXT SLOTNUM");
	if (1 == slot_count) {
		vty_out(vty,"%-21s%-22s%-6s%-12s\n","--------------------","---------------------","-----","-----------");
	}
	else
		vty_out(vty,"%-5s%-10s%-20s%-22s%-6s%-12s\n","----","---------","-------------------","---------------------","-----","-----------");
	#endif
	dbus_message_iter_next(&iter);
	
	dbus_message_iter_recurse(&iter,&iter_array);
	*slot_number = slot_count;
	sys_head->next = NULL;
	tail = sys_head;
	
	for (i = 0; i < slot_count; i++) 
	{
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
		q=(hw_info *)malloc(sizeof(hw_info));
		memset(q,0,sizeof(hw_info));
		if (1 == slot_count) 
		{
			//vty_out(vty," %-20s",modname);
			q->slot = 0;
			strcpy(q->moduleName,modname);
			
		} 
		else 
		{
			q->slot = slotno;
			strcpy(q->hw_status,slot_status_str[module_status]);
			strcpy(q->moduleName,(MODULE_STAT_NONE != module_status) ? modname:"-");
			//vty_out(vty," %-4d%-10s%-20s",slotno,slot_status_str[module_status],(MODULE_STAT_NONE != module_status) ? modname:"-");
		}
		
		if(MODULE_STAT_NONE == module_status) 
		{
			strcpy(q->SN,"-");
			strcpy(q->hw_ver,"-");
			q->ext_slotnum = 0;
			//vty_out(vty,"%-22s%-6s%-11s\n","-","-","-");
		}
		else 
		{
			strcpy(q->SN,sn);
			sprintf(q->hw_ver,"%d.%d",HW_PCB_VER(hw_ver),HW_CPLD_VER(hw_ver));
			q->ext_slotnum = ext_slot_num;
			//vty_out(vty,"%-22s% 2d.%-2d %-11d\n",(MODULE_STAT_NONE != module_status) ? sn:"-",HW_PCB_VER(hw_ver),HW_CPLD_VER(hw_ver),ext_slot_num);
		}

		q->next = NULL;
		tail->next = q;
		tail = q;
	}
		

	dbus_message_unref(reply);
	
	
	return 0;
}
void free_hw_head(hw_info * hw_head)
{
	hw_info *f1,*f2;
	f1=hw_head->next;		 /*释放hw信息链表的空间*/
	f2=f1->next;
	while(f2!=NULL)
	{
		free(f1);
		f1=f2;
		f2=f2->next;
	}
	free(f1);
}


//add new function show system performace


char* get_token(char *str_tok, unsigned int *pos)
{
    unsigned int temp = *pos;
    while(isspace(*(str_tok+temp)) && (*(str_tok+temp) != '\0'))
    {
        temp++;
    }
    while(!isspace(*(str_tok+temp)) && (*(str_tok+temp) != '\0'))
    {
        temp++;
    }
    while(isspace(*(str_tok+temp)) && (*(str_tok+temp) != '\0'))
    {
        temp++;
    }
    *pos = temp;
    return (char *)(str_tok+temp);
}

int get_cpu_status(unsigned int *cpu_times, unsigned int *count)
{
	FILE *fp = NULL;
	char buffer[BUF_LEN];
	char *iter = NULL;
	unsigned int begin = 0;
	unsigned int cpu_no = 0;
	unsigned int cpu_status = 0;
	(*count) = 0;

	memset(buffer, 0, sizeof(buffer));
	memset(cpu_times, 0, sizeof(unsigned int)*MAX_CPU*CPU_STATUS);

	if(NULL != (fp = fopen(cp_cpu_info,"r")))
	{  
		cpu_no = 0;
		cpu_status = 0;
		while( (NULL != fgets(buffer, BUF_LEN, fp)) && cpu_no < MAX_CPU )
		{    
			iter = buffer;
			begin = 0;
			cpu_status = 0;
			if(buffer[begin]=='c' && buffer[begin+1]=='p' && buffer[begin+2]=='u' )
			{
				if( buffer[begin+3]!=' ')
				{   
					(*count)++;
					get_token(buffer, &begin);
					iter += begin;
					while(cpu_status < CPU_STATUS)
					{
						*(cpu_times+cpu_no*CPU_STATUS+(cpu_status++)) = strtoul(iter, &iter, 0);
					}
					cpu_no ++;
				}
			}
			else
			{
				break;
			}
		}
		fclose(fp);
		return SUCCESS;
	}
	return SYS_ERR;
}

int get_cpu_usage(P_SYS_INFO pst_sys_info, unsigned int refresh )
{
	unsigned int cpu_no = 0;
	unsigned int cpu_stu = 0;
	int total_time = 0;
	unsigned int *cpu_time_old = NULL;
	unsigned int *cpu_time_now = NULL; 
	float *cpu_usage = NULL;
	int *diffs_temp = NULL;
	unsigned int time_old[MAX_CPU][CPU_STATUS];
	unsigned int time_cur[MAX_CPU][CPU_STATUS];
	int time_diffs[CPU_STATUS];
	unsigned int count_1 = 0;
	unsigned int count_2 = 0;

	memset((void *)time_old, 0, MAX_CPU*CPU_STATUS*sizeof(unsigned int));
	memset((void *)time_cur, 0, MAX_CPU*CPU_STATUS*sizeof(unsigned int));
	memset((void *)time_diffs, 0, CPU_STATUS*sizeof(unsigned int));

	if(SUCCESS == get_cpu_status((unsigned int *)time_old, &count_1))
	{   
		sleep(refresh);
		if(SUCCESS == get_cpu_status((unsigned int *)time_cur, &count_2))
		{
			pst_sys_info->pst_cpus_status->cpu_no = MIN(count_1, count_2);
			for(;cpu_no<(pst_sys_info->pst_cpus_status->cpu_no);cpu_no++)
			{
				cpu_stu = 0;
				total_time = 0;
				diffs_temp = time_diffs;
				cpu_time_old = (unsigned int *)time_old + CPU_STATUS*cpu_no;
				cpu_time_now = (unsigned int *)time_cur + CPU_STATUS*cpu_no;
				cpu_usage = (float *)pst_sys_info->pst_cpus_status->ar_cpu_usage + CPU_STATUS*cpu_no;
				memset(diffs_temp, 0, sizeof(int)*CPU_STATUS);
				for(;cpu_stu<CPU_STATUS;cpu_stu++)
				{
					*diffs_temp = (int)((*cpu_time_now++)-(*cpu_time_old++));
					total_time += *diffs_temp++;
				}
				total_time = (total_time == 0)?1:total_time;
				cpu_stu = 0;
				diffs_temp = time_diffs;
				for(;cpu_stu<CPU_STATUS;cpu_stu++)
				{
					*cpu_usage++ = (*diffs_temp++)*100.0/total_time;
				}
			}
		}
		return SYS_ERR;
	}
	return SYS_ERR;
}

int get_mem_usage( P_SYS_INFO pst_sys_info )
{
    FILE *fp = NULL;
    char buffer[BUF_LEN];
    char *iter = NULL;
    unsigned int begin = 0;
    
    memset(buffer, 0, sizeof(buffer));
    if(NULL != (fp = fopen(cp_mem_info,"r")))
    {  
        if(NULL == fgets(buffer, BUF_LEN, fp))
        {
        	fclose(fp);
            return SYS_ERR;
        }
        get_token(buffer, &begin);
        iter = buffer + begin;
        pst_sys_info->pst_mem_status->un_mem_total = strtoul(iter, &iter, 0);

        if(NULL == fgets(buffer, BUF_LEN, fp))
        {
        	fclose(fp);
            return SYS_ERR;
        }
        begin = 0;
        get_token(buffer, &begin);
        iter = buffer + begin;
        pst_sys_info->pst_mem_status->un_mem_free = strtoul(iter, &iter, 0);

        if(NULL == fgets(buffer, BUF_LEN, fp))
        {
        	fclose(fp);
            return SYS_ERR;
        }
        begin = 0;
        get_token(buffer, &begin);
        iter = buffer + begin;
        pst_sys_info->pst_mem_status->un_mem_buffers = strtoul(iter, &iter, 0);

        pst_sys_info->pst_mem_status->un_mem_used = pst_sys_info->pst_mem_status->un_mem_total - pst_sys_info->pst_mem_status->un_mem_free;
        
        fclose(fp);
        return SUCCESS;
    }
    return SYS_ERR;

}

int get_cpu_temperature( P_SYS_INFO pst_sys_info )
{
    struct sys_envir sys;
    memset((struct sys_envir *)&sys, 0, sizeof(struct sys_envir));
    
    ccgi_dbus_init();  //用此函数了，从底层取值了啊 
	if( show_sys_envir(&sys) == CCGI_SUCCESS)
	{
	      pst_sys_info->pst_cpu_temperature->un_tem_core = (unsigned int)sys.core_tmprt;
		pst_sys_info->pst_cpu_temperature->un_tem_sface = (unsigned int)sys.surface_tmprt;

	    return SUCCESS;
	}
    return SYS_ERR;
}


int get_logo(char * logo)
{
	FILE * get_info;
	get_info = popen("cat /devinfo/enterprise_name | sed \"2,200d\"","r");
	if(get_info != NULL)
	{
		fgets(logo,50,get_info);
		pclose(get_info);
	}
	return 0;
}

//write to boorom

int write_to_boorom(char *path)
{
	int fd;
	bootrom_file bootrom;
	int retval;
	memset(bootrom.path_name,0,sizeof(bootrom.path_name));
	strcpy(bootrom.path_name,path);
#if 1
	fd = open("/dev/bm0",0);
	retval = ioctl(fd,BM_IOC_BOOTROM_EXCH,&bootrom);
	if(retval < 0){
		//vty_out(vty,"update bootrom failed return [%d]\n",retval);
		close(fd);
		return retval;
	}	
	close(fd);
#endif
	return 0;

}

int show_file_string(char *FILENAME,char *zstr)
{
	char buff[256];
	int len,i,fd;
	char temp[10];
	char zstring[128];
	memset(zstring,0,128);
	
	fd = open(FILENAME,O_RDONLY);
	if (fd < 0) {
		//vty_out(vty,"\r\nFailed to open file.\r\n");
		return 1;
	}	
	len = read(fd,buff,256);	
	for (i=0;i<len; i++) {
		if (buff[i] != '\n') 
        memset(temp,0,10);
		sprintf(temp,"%c",buff[i]);
		strcat(zstring,temp);
	}	
	memcpy(zstr,zstring,strlen(zstring));
	
	close(fd);
	return 0;

}

int get_hostname(char *hostname)/*返回0表示失败，返回1表示成功*/
{
	if(NULL == hostname)
		return 0;
	
	FILE *fp = NULL;
	memset(hostname, 0, 128);

	fp = popen("hostname","r");	
	if(fp != NULL)
	{
		fgets(hostname,128,fp);
		pclose(fp);

		/*begin delete_enter*/
		int len = 0;
		len = strlen(hostname);
	    int len_l = 0;
		char * tmp = hostname;
		while(*tmp != '\n')
		{
			len_l++;
			if(len_l >= len)
				return 0;
			tmp++;
		}
		*tmp = '\0';
		/*end delete_enter*/		

		return 1;
	}
	else
	{
		return 0;
	}
}

int set_hostname(char *hostname)/*返回0表示失败，返回1表示成功*/
								   /*返回-1表示hostname is too long,should be 1 to 64*/
								   /*返回-2表示hostname should be start with a letter*/
								   /*返回-3表示hostname should be use letters,numbers,"-","."*/
{
	if(NULL == hostname)
		return 0;
	
	char tmp;
	int i=0,len = 0;
	int status = -1;
	char command[256] = {0};

	len = strlen(hostname);
	if(len>64)
	{
		return -1;
	}
	
 	if (!isalpha(hostname[0]))
    {
      	return -2;
    }

  	tmp = *hostname;
	for(i = 0;i<len ;i++)
  	{
	  	tmp = *(hostname+i);
	  	if( (tmp>='0' && tmp<='9')||(tmp>='a' &&tmp<='z')
			||(tmp>='A' &&tmp<='Z')||(tmp=='-')||(tmp=='.'))
	  	{
		  	continue;
	  	}
	  	else
	  	{
		  	return -3;
	  	}
  	}

	memset(command,0,sizeof(command));		
	sprintf(command,"hostname.sh %s",hostname);
	
	status = system(command);
    if(0 == status)
    {
    	return 1;
    }
	else
	{
    	return 0;
    }	
}

static int get_dns_str(char **name)
{
	FILE *fp=NULL;
	char *ptr=NULL;
	int i=0;

	fp = fopen("/etc/resolv.conf","r");
	
	if(!fp)
		return -1;
	ptr=malloc(128);
	if(!ptr)
		{
		fclose(fp);
		return -1;
	}
	while(fgets(ptr,128,fp))
	{
		if(!strncmp(ptr,"nameserver ",11))
		{
			sprintf(*(name+i),ptr+11);
			i++;			
			if (i>=3)
				break;
		}
	}
	free(ptr);
	fclose(fp);
	return i;
	
}

static int set_dns_str(const char **name,int num)
{
	FILE * fp=NULL;
	char *ptr=NULL;
	int i=0;

	system("sudo chmod 777 /etc/resolv.conf");
	fp = fopen("/etc/resolv.conf","w");
	if(!fp)
	{
		system("sudo chmod 644 /etc/resolv.conf");
		return -1;
	}

	ptr=malloc(128);
	if(!ptr)
	{
		fclose(fp);
		system("sudo chmod 644 /etc/resolv.conf");
		return -1;
	}
	for(i=0;i<num;i++)
	{
		sprintf(ptr,"nameserver %s\n",*(name+i));
		fputs(ptr,fp);
	}
	
	free(ptr);
	fclose(fp);
	system("sudo chmod 644 /etc/resolv.conf");
	return 0;

}

int set_ip_dns_func_cmd(char *ip_dns)  /*返回0表示失败，返回1表示成功*/
											/*返回-1表示Can't get system dns seting*/
											/*返回-2表示The system has 3 dns,can't set again*/
											/*返回-3表示The dns server is exist,can't set again*/
											/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == ip_dns)
		return 0;
	
	char *dnsstr[3];
	int ret,i;
	int retu = 0;

	for(i=0;i<3;i++)
	{
		dnsstr[i] = malloc(128);
		if(!dnsstr[i])
			goto ret_err;
		memset(dnsstr[i],0,128);
	}	

	ret = get_dns_str(&dnsstr);
	if(ret<0)
	{
		retu = -1;
		goto ret_err;
	}
	else if(ret >= 3)
	{
		retu = -2;
		goto ret_err;
	}
	else
	{
		int i;
		DBusMessage *query = NULL;		
		DBusMessage *reply = NULL;	
		DBusError err = {0};
		//unsigned int result = 0;				
		unsigned int src_slotid = 0;
		unsigned int des_slotid = 0;
		//unsigned int product_serial = 0;		
		char file_path_temp[64] = "/etc/resolv.conf";
		char *src_path_temp = file_path_temp;
		char *des_path_temp = file_path_temp;
		int fd;		
		int tar_switch = 0;
		int op = BSD_TYPE_NORMAL;
		int length = 0;
		char dns[128];
		int ID[MAX_SLOT_NUM] = {0};
		int count = 0;

		fd = fopen("/dbm/product/master_slot_id", "r");
		fscanf(fd, "%d", &src_slotid);
		fclose(fd);

		count = dcli_bsd_get_slot_ids(ccgi_dbus_connection,ID,1);

		for(i=0;i<ret;i++)
		{
			memset(dns,0,128);
			length = strlen(dnsstr[i]);
			strncpy(dns,dnsstr[i],length-1);
			if(!strcmp(ip_dns,dns))
				break;
		}
		if(i==ret)
			sprintf(dnsstr[ret],(char*)ip_dns);
		else
		{
			retu = -3;
			goto ret_err;
		}		
		if(set_dns_str(&dnsstr,ret+1))
		{
			retu = 0;
			goto ret_err;

		}
		if(count != 0)
		{
			for(i = 0; i < count; i++)
			{
				des_slotid = ID[i];
				ccgi_ReInitDbusConnection(&ccgi_dbus_connection,src_slotid,DISTRIBUTFAG);
			    
			    query = dbus_message_new_method_call(BSD_DBUS_BUSNAME,BSD_DBUS_OBJPATH,\
									BSD_DBUS_INTERFACE,BSD_COPY_FILES_BETEWEEN_BORADS);
				dbus_error_init(&err);
				dbus_message_append_args(query,
										 DBUS_TYPE_UINT32,&des_slotid,
										 DBUS_TYPE_STRING,&src_path_temp,
										 DBUS_TYPE_STRING,&des_path_temp,
										 DBUS_TYPE_UINT32,&tar_switch,
										 DBUS_TYPE_UINT32,&op,
										 DBUS_TYPE_INVALID);
			
				reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,60000, &err);
				dbus_message_unref(query);
				if (NULL == reply) {
					if (dbus_error_is_set(&err)) {
						dbus_error_free(&err);
					}
					return SNMPD_CONNECTION_ERROR;
				}
				dbus_message_unref(reply);
			}
		}
	}
	for(i=0;i<3;i++)
	{
		if(dnsstr[i])
			free(dnsstr[i]);
	}	

	return 1;

ret_err:
	
	for(i=0;i<3;i++)
	{
		if(dnsstr[i])
			free(dnsstr[i]);
	}	
	return retu;
}

int delete_ip_dns_func_cmd(char *ip_dns)  /*返回0表示失败，返回1表示成功*/
												/*返回-1表示malloc*/
												/*返回-2表示Can't get system dns seting*/
												/*返回-3表示error*/
												/*返回SNMPD_CONNECTION_ERROR表示connection error*/
{
	if(NULL == ip_dns)
		return 0;
	
	char *dnsstr[3];
	int ret,i;
	int retu = 0;

	for(i=0;i<3;i++)
	{
		dnsstr[i] = malloc(128);
		if(!dnsstr[i])
		{
			retu = -1;
			goto ret_err;
		}
		memset(dnsstr[i],0,128);
	}
	
	ret = get_dns_str(&dnsstr);

	if(ret<0 || ret > 3)
	{
		retu = -2;
		goto ret_err;
	}
	else
	{
		int i=0;
		
		DBusMessage *query = NULL;		
		DBusMessage *reply = NULL;	
		DBusError err = {0};
		unsigned int result = 0;
				
		unsigned int src_slotid = 0;
		unsigned int des_slotid = 0;
		unsigned int product_serial = 0;		
		char file_path_temp[64] = "/etc/resolv.conf";
		char *src_path_temp = file_path_temp;
		char *des_path_temp = file_path_temp;
		int fd;		
		int tar_switch = 0;
		int op = BSD_TYPE_NORMAL;
		int ID[MAX_SLOT_NUM] = {0};
		int count = 0;
		void *connection = NULL;

		fd = fopen("/dbm/product/master_slot_id", "r");
		fscanf(fd, "%d", &src_slotid);
		fclose(fd);

		count = dcli_bsd_get_slot_ids(ccgi_dbus_connection,ID,1);

		int(*dcli_init_func)(DBusConnection *, int *, const int);

		dcli_init_func = dlsym(ccgi_dl_handle,"dcli_bsd_get_slot_ids");
		if(NULL != dcli_init_func)
		{
			count =(*dcli_init_func)(ccgi_dbus_connection,ID,1);
		}
		else
		{
			retu = 0;
			goto ret_err;
		}
		
		for(i=0;i<ret;i++)
		{
			if(!strncmp(ip_dns,dnsstr[i],strlen(ip_dns)))
			{
				int j ;
				if(i<ret-1)
				{	
					for(j=i;j<ret-1;j++)
					{
						sprintf(dnsstr[j],dnsstr[j+1]);
					}
				}
				break;
			}
		}
		if(i >= ret)
		{
			retu = -2;
			goto ret_err;

		}
		if(set_dns_str(&dnsstr,ret-1))
		{
			retu = -3;
			goto ret_err;

		}
		
		if(count != 0)
		{
			for(i = 0; i < count; i++)
			{
				des_slotid = ID[i];

				connection = NULL;
				if(SNMPD_DBUS_SUCCESS != get_slot_dbus_connection(src_slotid, &connection, SNMPD_INSTANCE_MASTER_V3))
				{
					retu = 0;
				}
   
			    query = dbus_message_new_method_call(BSD_DBUS_BUSNAME,BSD_DBUS_OBJPATH,\
									BSD_DBUS_INTERFACE,BSD_COPY_FILES_BETEWEEN_BORADS);
				dbus_error_init(&err);
				dbus_message_append_args(query,
										 DBUS_TYPE_UINT32,&des_slotid,
										 DBUS_TYPE_STRING,&src_path_temp,
										 DBUS_TYPE_STRING,&des_path_temp,
										 DBUS_TYPE_UINT32,&tar_switch,
										 DBUS_TYPE_UINT32,&op,
										 DBUS_TYPE_INVALID);
				reply = dbus_connection_send_with_reply_and_block (connection,query,60000, &err);
				dbus_message_unref(query);
				if (NULL == reply) {
					if (dbus_error_is_set(&err)) {
						dbus_error_free(&err);
					}
					return SNMPD_CONNECTION_ERROR;
				}
				dbus_message_unref(reply);
			}
		}
		
	}

	return 1;
ret_err:
	
	for(i=0;i<3;i++)
	{
		if(dnsstr[i])
			free(dnsstr[i]);
	}	
	return retu;	
}


#ifdef __cplusplus
}
#endif


