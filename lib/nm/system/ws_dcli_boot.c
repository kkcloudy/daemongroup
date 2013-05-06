/* cgicTempDir is the only setting you are likely to need
	to change in this file. */

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
* ws_dcli_boot.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* qiaojie@autelan.com
*
* DESCRIPTION:
*
*
*
*******************************************************************************/

#ifdef __cplusplus
	extern "C"
	{
#endif

#include "ws_dcli_boot.h"
#include "ws_sysinfo.h"

int get_boot_img_name(char* imgname)
{
	int fd; 
	int retval; 
	boot_env_t	env_args={0};	
	char *name = "bootfile";
	
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

int set_boot_img_name(char* imgname)
{
	int fd; 
	int retval; 
	boot_env_t	env_args={0};	
	char *name = "bootfile";
	
	sprintf(env_args.name,name);
	sprintf(env_args.value,imgname);
	env_args.operation = SAVE_BOOT_ENV;

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
	close(fd);
	return 0;	
}

int sor_checkimg(char* imgname)
{
	char result_file[64][128];
	char *cmdstr = "sor.sh imgls imgls 180";
	int ret,i,imgnum;
	FILE *fp = 	NULL;
	

	fp = popen( cmdstr, "r" );
	if(fp)
	{
		i=0;
		while(i<64 && fgets( result_file[i], sizeof(result_file[i]), fp ))
			i++;
		imgnum=i;

		ret = pclose(fp);
		
		switch (WEXITSTATUS(ret)) {
			case 0:
				for(i=0;i<imgnum;i++)
				{
					if(!strncasecmp(result_file[i],imgname,sizeof(imgname)))
						return 0;
				}
				return -1;
			default:
				return WEXITSTATUS(ret);
			}
	}
	else
		return -2;
}

void dcli_send_dbus_signal(const char* name,const char* str)
{
	DBusMessage *query, *reply;
	DBusMessageIter	 iter;
	DBusError err;
	char exec=1;
	unsigned char* config_cmd = malloc(256); 

	memset(config_cmd,0,256);
	strcpy(config_cmd,str);
	query = dbus_message_new_signal(RTDRV_DBUS_OBJPATH,\
						"aw.trap",name);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		DBUS_TYPE_BYTE,&exec,								
		DBUS_TYPE_STRING,&(config_cmd),
		DBUS_TYPE_INVALID);
	dbus_connection_send (ccgi_dbus_connection,query,NULL);
	dbus_connection_flush(ccgi_dbus_connection);
	dbus_message_unref(query);	
	free(config_cmd);
	

}

int config_boot_img_func_cmd(char *img_name)/*返回0表示失败，返回1表示成功，返回-1表示The boot file should be .img file*/
												  /*返回-2表示The boot img doesn't exist，返回-3表示Sysetm internal error (1)*/
												  /*返回-4表示Sysetm internal error (2)，返回-5表示Storage media is busy*/
												  /*返回-6表示Storage operation time out，返回-7表示No left space on storage media*/
												  /*返回-8表示Sysetm internal error (3)*/
{
	int ret=0;
	char old_bootimg_name[64] = {0};
	char dbus_str_pre[128] = {0};
	char dbus_str[256]={0};
	int retu = 0;
	
	get_boot_img_name(old_bootimg_name);

	sprintf(dbus_str_pre,"Old boot img file %s,new boot img file %s",old_bootimg_name,img_name);
	if(strncasecmp((img_name+strlen(img_name)-4),".IMG",4))
	{
		sprintf(dbus_str,"%s set boot img file format failure\n",dbus_str_pre);
		dcli_send_dbus_signal("set_boot_img_failed",dbus_str);
		return -1;
	}
	
	ret = sor_checkimg(img_name);
	if( ret ==  0 ){
		ret = set_boot_img_name(img_name);
		if( 0 == ret)
		{
			sprintf(dbus_str,"%s set boot img file success\n",dbus_str_pre);
			dcli_send_dbus_signal("set_boot_img_success",dbus_str);
			retu = 1;
			ret=0;
		}
		else
		{
			sprintf(dbus_str,"%s set boot img failure\n",dbus_str_pre);
			dcli_send_dbus_signal("set_boot_img_failed",dbus_str);
			retu = 0;
			ret= 1;
		}
	}
	else
	{
		switch(ret)
		{
		
			case -1:
				sprintf(dbus_str,"%s The boot img doesn't exist.\n",dbus_str_pre);
				dcli_send_dbus_signal("set_boot_img_failed",dbus_str);
				retu = -2;
				break;
			case 1:
				sprintf(dbus_str,"%s:Sysetm internal error (1).\n",dbus_str_pre);
				dcli_send_dbus_signal("set_boot_img_failed",dbus_str);
				retu = -3;
				break;
			case 2:
				sprintf(dbus_str,"%s:Sysetm internal error (2).\n",dbus_str_pre);
				dcli_send_dbus_signal("set_boot_img_failed",dbus_str);
				retu = -4;
				break;
			case 3:
				sprintf(dbus_str,"%s:Storage media is busy.\n",dbus_str_pre);
				dcli_send_dbus_signal("set_boot_img_failed",dbus_str);
				retu = -5;
				break;
			case 4:
				sprintf(dbus_str,"%s:Storage operation time out.\n",dbus_str_pre);
				dcli_send_dbus_signal("set_boot_img_failed",dbus_str);
				retu = -6;
				break;
			case 5:
				sprintf(dbus_str,"%s:No left space on storage media.\n",dbus_str_pre);
				dcli_send_dbus_signal("set_boot_img_failed",dbus_str);
				retu = -7;
				break;
			default:
				sprintf(dbus_str,"%s:Sysetm internal error (3).\n",dbus_str_pre);
				dcli_send_dbus_signal("set_boot_img_failed",dbus_str);
				retu = -8;
				break;
				}
		sprintf(dbus_str,"%s:check boot img error\n",dbus_str_pre);
		dcli_send_dbus_signal("set_boot_img_failed",dbus_str);	
	}

	return retu;

}


#ifdef __cplusplus
}
#endif

