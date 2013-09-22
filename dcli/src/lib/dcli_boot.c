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
* dcli_boot.c
*
* MODIFY:
*		
*
* CREATOR:
*		shancx@autelan.com
*
* DESCRIPTION:
*		CLI definition for bootfile .
*
* DATE:
*		10/27/2008	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.62 $	
*******************************************************************************/
//#include <dirent.h>
//#include <unistd.h>
#include <ctype.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h> 
#include <sys/ioctl.h>
#include <ctype.h>
#include <zebra.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/wait.h>	
#include "command.h"
#include "dcli_boot.h"
#include "dcli_user.h"
#include "image.h"
#include "dcli_apache.h"
#include "dcli_wireless/dcli_bsd.h"
#include "bsd/bsdpub.h"
#include <pwd.h>
#include <grp.h>
#include "dcli_main.h"
#include <dbus/sem/sem_dbus_def.h>
#include "bsd_bsd.h"

#include <time.h>
#include "memory.h"

extern int is_distributed;
extern int is_active_master;

extern	void dcli_send_dbus_signal(const char* ,const char* );

int get_boot_version_name(char* vername)
{
	int fd; 
	int retval; 
	boot_env_t	env_args={0};	
	char *name = "version";
	
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
		sprintf(vername,env_args.value); 
	}
	close(fd);
	return 0;


}

int get_boot_env_var_name(char* vername)
{
	int fd; 
	int retval; 
	boot_env_t	env_args={0};	
	char *name = vername;
	
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
		sprintf(vername,env_args.value); 
	}
	close(fd);
	return 0;


}
int set_boot_env_var_name(char* vername,char* value)
{
	int fd; 
	int retval; 
	boot_env_t	env_args={0};	
	char *name = vername;
	char *env_value = value;
	
	sprintf(env_args.name,name);
	sprintf(env_args.value,env_value);
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

/*added by zhaocg for fast forward 2012-11-23*/
int get_fast_forward_name(char* fastfwdname)
{
	int fd; 
	int retval; 
	boot_env_t	env_args={0};	
	char *name = "sefile";
	
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
		sprintf(fastfwdname,env_args.value); 
	}
	close(fd);
	return 0;


}

int check_img(char* filename)
{
	int read_len;
	image_header_t *ptr = NULL;
	char *buf = NULL;
	int buff_len = sizeof(image_header_t);
	FILE* fd=fopen(filename,"r");

	if(!fd)
	{
		return -1;
	}
	buf=malloc(buff_len);
	if(!buf)
	{
		fclose(fd);
		return -1;
	}
	
	

	read_len = fread(buf,1,buff_len,fd);
	if(read_len!=buff_len)
	{
		fclose(fd);
		free(buf);
		return -1;
	}
	ptr=(image_header_t *)buf;
	if(ptr->ih_magic != IH_MAGIC)
	{
		fclose(fd);
		free(buf);
		return 1;
	}
	
	fclose(fd);
	free(buf);
	return 0;
}
char* get_img_result_file(char *filename)
{
	struct dirent* ent = NULL;
	DIR *pDir;
	char filenameprix[256] ;
	pid_t pid=0;
	pDir=opendir(SOR_VAR_PATH);
	if(pDir == NULL)
	{
		return NULL;
	}
	sprintf(filenameprix,"%s_%d_",SOR_RESULT_FILE_PRIX,getpid());
	while (NULL != (ent=readdir(pDir)))
	{
		if (ent->d_type==8 && !strncmp(filenameprix,ent->d_name,strlen(filenameprix)))
		{
			int pid_t=atoi(ent->d_name+strlen(filenameprix));
			if(pid<=pid_t)
			{
				pid=pid_t;
				sprintf(filename,"%s%s",SOR_VAR_PATH,ent->d_name);
			}
		}
	}
	closedir(pDir);
	if(pid == 0)
		return NULL;
	return filename;

		
}

int write_bootrom(struct vty *vty,char *path)
{
	int fd;
	bootrom_file bootrom;
	int retval;
	memset(bootrom.path_name,0,sizeof(bootrom.path_name));
	strcpy(bootrom.path_name,path);
#if 1
	fd = open("/dev/bm0",0);
	if(fd < 0){
		return -1;
	}
	retval = ioctl(fd,BM_IOC_BOOTROM_EXCH,&bootrom);
	if(retval < 0){
		vty_out(vty,"update bootrom failed return [%d]\n",retval);
		close(fd);
		return retval;
	}	
	close(fd);
#endif
	return 0;

}
int sor_exec(struct vty* vty,char* cmd,char* filename,int time)
	{	char cmdstr[512];
		int ret;
		sprintf(cmdstr,"sor.sh %s %s %d ",cmd,filename,time);
		ret = system(cmdstr);	
		switch (WEXITSTATUS(ret)) { 	
			case 0: 		
				return CMD_SUCCESS; 	
			case 1: 		
				vty_out(vty,"Sysetm internal error (1).\n");			
				break;		
			case 2: 		
				vty_out(vty,"Sysetm internal error (2).\n");			
				break;		
			case 3: 		
				vty_out(vty,"Storage media is busy.\n");			
				break;		
			case 4: 		
				vty_out(vty,"Storage operation time out.\n");			
				break;		
			case 5: 		
				vty_out(vty,"No left space on storage media.\n");			
				break;		
			default:			
				vty_out(vty,"Sysetm internal error (3).\n");			
				break;		
				}	
		return CMD_WARNING;
		}

int sor_ap_lsimg(struct vty* vty,int time)
{
	char result_file[64][128];
	char *cmdstr = "sor.sh ap_imgls /mnt/wtp 180";
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
				vty_out(vty,"There is %d AP IMG file\n",imgnum);
				for(i=0;i<imgnum;i++)
				{
					vty_out(vty,"%s",result_file[i]);
				}
				return CMD_SUCCESS;
			case 1:
				vty_out(vty,"Sysetm internal error (1).\n");
				break;
			case 2:
				vty_out(vty,"Sysetm internal error (2).\n");
				break;
			case 3:
				vty_out(vty,"Storage media is busy.\n");
				break;
			case 4:
				vty_out(vty,"Storage operation time out.\n");
				break;
			case 5:
				vty_out(vty,"No left space on storage media.\n");
				break;
			default:
				vty_out(vty,"Sysetm internal error (3).\n");
				break;
			}
	}
	else
	{
		vty_out(vty,"Cant't get IMG file list\n");
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}

int sor_lsimg(struct vty* vty,int time)
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
				vty_out(vty,"There is %d IMG file\n",imgnum);
				for(i=0;i<imgnum;i++)
				{
					vty_out(vty,"%s",result_file[i]);
				}
				return CMD_SUCCESS;
			case 1:
				vty_out(vty,"Sysetm internal error (1).\n");
				break;
			case 2:
				vty_out(vty,"Sysetm internal error (2).\n");
				break;
			case 3:
				vty_out(vty,"Storage media is busy.\n");
				break;
			case 4:
				vty_out(vty,"Storage operation time out.\n");
				break;
			case 5:
				vty_out(vty,"No left space on storage media.\n");
				break;
			default:
				vty_out(vty,"Sysetm internal error (3).\n");
				break;
			}
	}
	else
	{
		vty_out(vty,"Cant't get IMG file list\n");
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}
/*added by zhaocg for fastfwd command 2012-11-20*/
int sor_fastfwd(struct vty* vty,int time)
{
	char result_file[64][128];
	char *cmdstr = "sor.sh fastfwdls fastfwdls 180";
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
				vty_out(vty,"There is %d fast-forward file\n",imgnum);
				for(i=0;i<imgnum;i++)
				{
					vty_out(vty,"%s",result_file[i]);
				}
				return CMD_SUCCESS;
			case 1:
				vty_out(vty,"Sysetm internal error (1).\n");
				break;
			case 2:
				vty_out(vty,"Sysetm internal error (2).\n");
				break;
			case 3:
				vty_out(vty,"Storage media is busy.\n");
				break;
			case 4:
				vty_out(vty,"Storage operation time out.\n");
				break;
			case 5:
				vty_out(vty,"No left space on storage media.\n");
				break;
			default:
				vty_out(vty,"Sysetm internal error (3).\n");
				break;
			}
	}
	else
	{
		vty_out(vty,"Cant't get fast-forward file list\n");
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}

/*ended by zhaocg for fastfwd command 2012-11-20*/



int sor_md5img(struct vty* vty,const char* filename)
{
	char result_str[512];
	char cmdstr[512] = {0};
	int ret,i,imgnum;
	FILE *fp = 	NULL;

	if(strlen(filename)>480)
	{
		vty_out(vty,"arguments too long\n");
		return CMD_WARNING;
	}
	
	sprintf(cmdstr,"sor.sh imgmd5 %s 120",filename);
	fp = popen( cmdstr, "r" );
	if(fp)
	{
		fgets( result_str, 512, fp );
		ret = pclose(fp);
		
		switch (WEXITSTATUS(ret)) {
			case 0:
				vty_out(vty,"%s\n",result_str);
				return CMD_SUCCESS;
			case 1:
				vty_out(vty,"Sysetm internal error (1).\n");
				break;
			case 2:
				vty_out(vty,"Sysetm internal error (2).\n");
				break;
			case 3:
				vty_out(vty,"Storage media is busy.\n");
				break;
			case 4:
				vty_out(vty,"Storage operation time out.\n");
				break;
			case 5:
				vty_out(vty,"No left space on storage media.\n");
				break;
			default:
				vty_out(vty,"Sysetm internal error (3).\n");
				break;
			}
	}
	else
	{
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}


/*added by zhaocg for md5 subcommand*/
int sor_md5patch(struct vty* vty,const char* filename)
{
	char result_str[512] = {0};
	char cmdstr[512] = {0};
	char tmp[512] = {0};
	char *token = NULL;
	int ret,i,imgnum;
	FILE *fp = 	NULL;

	if(strlen(filename)>475)
	{
		vty_out(vty,"arguments too long\n");
		return CMD_WARNING;
	}
	sprintf(cmdstr,"sor.sh imgmd5 patch/%s 120",filename);
	
	fp = popen( cmdstr, "r" );
	
	if(fp)
	{
		fgets( result_str, 512, fp );
		strcpy(tmp,result_str);
		token= strtok(result_str,"=");
		token = strtok(NULL,"\r");
		
		if(NULL==token)
		{
			token= strtok(tmp,":");
			token = strtok(NULL,"\r");
		}
		
		ret = pclose(fp);
		
		switch (WEXITSTATUS(ret)) {
			case 0:
				vty_out(vty,"MD5(%s)=%s\n",filename,token);
				return CMD_SUCCESS;
			case 1:
				vty_out(vty,"Sysetm internal error (1).\n");
				break;
			case 2:
				vty_out(vty,"Sysetm internal error (2).\n");
				break;
			case 3:
				vty_out(vty,"Storage media is busy.\n");
				break;
			case 4:
				vty_out(vty,"Storage operation time out.\n");
				break;
			case 5:
				vty_out(vty,"No left space on storage media.\n");
				break;
			default:
				vty_out(vty,"Sysetm internal error (3).\n");
				break;
			}
	}
	else
	{
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}

/*ended by zhaocg for md5 subcommand*/

/*added by zhaocg for show fastfwd command 2012-11-20*/
DEFUN(show_fast_forward_func,
	show_fast_forward_func_cmd,
	"show fastfwd",
	SHOW_STR
	"Show fast-forward which can be used infomation\n"
	)
{
	sor_fastfwd(vty,120);

	return CMD_SUCCESS;
}

DEFUN(show_fast_forward_slot_func,
	show_fast_forward_slot_func_cmd,
	"show fastfwd <1-16>",
	SHOW_STR
	"Show fast-forward which can be used infomation\n"
	"Show fast-forward which can be used infomation on slot board\n"
	)
{
	DBusMessage *query=NULL; 
	DBusMessage *reply=NULL;
	DBusMessageIter  iter;
	DBusError err;
	char *ret = NULL;
	char **tem = NULL;
	char *cmdstr = "sor.sh fastfwdls fastfwdls 120";
	char *cmd = cmdstr;
	int fastfwdnum = 0;
	int i;
	int slot_id = 0;
	/*
	int slot_count = get_product_info(SEM_SLOT_COUNT_PATH);
	slot_id = strtol(argv[0], NULL, 10);
	*/
	slot_id = atoi(argv[0]);
	
	#if 0
	if(slot_id > slot_count || slot_id <= 0)
	{
		vty_out(vty, "error slot number : %s\n", argv[0]);
		vty_out(vty, "correct slot number option : 1 ~ %d\n", slot_count);
        return CMD_WARNING;
	}
	#endif

	if(NULL != (dbus_connection_dcli[slot_id] -> dcli_dbus_connection))
	{
	
		query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
					 SEM_DBUS_INTERFACE,SEM_DBUS_IMG_OR_FASTFWD_SLOT);

		dbus_error_init(&err);

		dbus_message_append_args(query,
				DBUS_TYPE_STRING, &cmd,
						DBUS_TYPE_INVALID);
		reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[slot_id] -> dcli_dbus_connection, query, 60000, &err);
		dbus_message_unref(query);
		if (NULL == reply)
		{
			vty_out(vty,"<error> failed get reply.\n");
			
			if (dbus_error_is_set(&err)) 
			{
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			
			return CMD_WARNING;
		}

		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&fastfwdnum);

		if(fastfwdnum != -1)
		{
			vty_out(vty,"There is %d fast-forward file\n",fastfwdnum);
		
			for(i=0;i < fastfwdnum;i++)
			{	
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&ret);
				vty_out(vty,"%s", ret);
				
			}
		}
		else
		{
			vty_out(vty,"Can't get fast-forward file info\n");
		}
		
		dbus_message_unref(reply);
	}
	else
	{
		vty_out(vty,"The slot doesn't exist");
	}
	

	return CMD_SUCCESS;

}
/*ended by zhaocg for show fastfwd command 2012-11-20*/

/*added by zhaocg for delet fast_forward command 2012-11-19*/
#if 0
	DEFUN(del_fast_fwd_func,
	del_fast_fwd_func_cmd,
	"delete fastfwd (all|self) WORD",
	"Delete configuration"
	"Delete fast-forward\n"
	"The name of .fastfwd.bin file"
	)
{
	char cmdstr[SOR_COMMAND_SIZE];
	int ret;
	int ret2,i,imgnum;
	char fastfwd_name[SOR_OPFILENAME_SIZE] ;

	
	FILE *fp =	NULL;
	DBusMessage *query, *reply;
	DBusError err;
	
	int slot_id;
	FILE *fd = NULL;
	int is_active_master = 0;
	char *cmd = NULL;
	
	if(strlen(argv[1])>SOR_OPFILENAME_SIZE-1)
	{
		vty_out(vty,"file name too long\n");
		return CMD_WARNING;
	}	
	if(strncasecmp((argv[1]+strlen(argv[1])-12),".fastfwd.bin",12))
	{
		vty_out(vty,"The fast forward file should be fastfwd.bin file\n");
		return CMD_WARNING;
	}

	#if 0
	get_fast_forward_name(fastfwd_name);

	if(!(strncasecmp(fastfwd_name,argv[1],strlen(argv[1]))))	
	{
		vty_out(vty,"Delete error!Delete the file is in use! \n");
		return CMD_WARNING;
	}
	#endif
	
	if(0 == strncmp(argv[0],"self",strlen(argv[0])))
	{
		memset(fastfwd_name,0,SOR_OPFILENAME_SIZE);
		sprintf(cmdstr,"sor.sh ls %s 120",argv[1]);
		fp = popen( cmdstr, "r" );
		if(fp)
		{	
			fgets(fastfwd_name, sizeof(fastfwd_name), fp );
			ret2 = pclose(fp);	
			switch (WEXITSTATUS(ret2))
			{			
				case 0: 
					if(fastfwd_name[0]!= NULL)	
					{							
					 	ret=sor_exec(vty,"rm",argv[1],100);
					  	if(ret==CMD_SUCCESS)
						vty_out(vty,"Delete successfully! \n");
					}
					else
					{
						vty_out(vty,"Delete error! Cant't get fastfwd file\n");
					}
					break;					
				default :
					vty_out(vty,"Delete error!\n");
					return CMD_WARNING;
					break;									
			}	
		}	
		else	
		{
			vty_out(vty,"Delete error!\n");		
			return CMD_WARNING; 
		}	
	}
	else if (0 == strncmp(argv[0],"all",strlen(argv[0])))
	{
		memset(fastfwd_name,0,SOR_OPFILENAME_SIZE);
		sprintf(cmdstr,"sor.sh ls %s 120",argv[1]);
		fp = popen( cmdstr, "r" );
		if(fp)
		{	
			fgets(fastfwd_name, sizeof(fastfwd_name), fp );		
			ret2 = pclose(fp);	
			switch (WEXITSTATUS(ret2))
			{			
				case 0: 
					if(*fastfwd_name!= 0)	
					{						
						fd = fopen("/dbm/local_board/is_active_master", "r");
							
						if (fd == NULL)
		        		{
			
							vty_out(vty,"Get production information [1] error\n");
							return -1;
						}
		
						fscanf(fd, "%d", &is_active_master);
						fclose(fd);
						if (is_active_master != 1)
						{
							vty_out(vty, "This command is only surpported by distributed system and only on active master board\n");
							return CMD_SUCCESS;
						}
						cmd = malloc(SOR_COMMAND_SIZE);
						slot_id = 0;
						sprintf(cmd,"sor.sh rm %s 10",argv[1]);
						query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
											 SEM_DBUS_INTERFACE, SEM_DBUS_EXECUTE_SYSTEM_COMMAND);
			
						dbus_error_init(&err);
						dbus_message_append_args(query,
										DBUS_TYPE_UINT32, &slot_id,
										DBUS_TYPE_STRING, &cmd,
										DBUS_TYPE_INVALID);
						reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);
						dbus_message_unref(query);

						if (NULL == reply)
						{
							free(cmd);
							vty_out(vty,"<error> failed get reply.\n");
							if (dbus_error_is_set(&err))
							{
								vty_out(vty,"%s raised: %s",err.name,err.message);
								dbus_error_free(&err);
							}
							return CMD_SUCCESS;
						}
	
						if (dbus_message_get_args (reply, &err,
								DBUS_TYPE_UINT32, &ret,
								DBUS_TYPE_INVALID)) 
						{
							if(ret == 0)
							{
								syslog(LOG_NOTICE,"Successful\n");
								vty_out(vty,"Delete successfully! \n");
							}
							else if (ret == 1)
							{
								vty_out(vty,"Sem send exec message failed\n");
							}
							else
							{
								free(cmd);
								dbus_message_unref(reply);
								vty_out(vty,"Failed\n", time);
								return CMD_WARNING;
							}
		
							free(cmd);
							dbus_message_unref(reply);	
									
						}
					}
					else
					{
					 vty_out(vty,"Delete error! Cant't get IMG file\n");
					}
					break;
				default :
						vty_out(vty,"Delete error! \n");
						return CMD_WARNING;	
			}/*end switch*/
			
			return CMD_SUCCESS;
	  	}	

	}
	else 
	{
		vty_out(vty,"% Bad parameter!\n");
		return CMD_WARNING;
	}
	return CMD_SUCCESS;


}
#else
DEFUN(del_fast_fwd_func,
	del_fast_fwd_func_cmd,
	"delete fastfwd self WORD",
	"Delete configuration"
	"Delete fast-forward\n"
	"Delete fast-forward on local board\n"
	"The name of .fastfwd.bin file"
	)
{
	char cmdstr[SOR_COMMAND_SIZE] = {0};
	int ret = 0;
	int ret2 = 0;
	char fastfwd_name[SOR_OPFILENAME_SIZE] = {0};
	FILE *fp =	NULL;	
	if(strlen(argv[0])>SOR_OPFILENAME_SIZE-1)
	{
		vty_out(vty,"file name too long\n");
		return CMD_WARNING;
	}	
	if(strncasecmp((argv[0]+strlen(argv[0])-12),".fastfwd.bin",12))
	{
		vty_out(vty,"The fast forward file should be fastfwd.bin file\n");
		return CMD_WARNING;
	}

	get_fast_forward_name(fastfwd_name);

	if(!(strncasecmp(fastfwd_name,argv[0],strlen(argv[0]))))	
	{
		vty_out(vty,"Delete error!Delete the file is in use! \n");
		return CMD_WARNING;
	}

	memset(fastfwd_name,0,SOR_OPFILENAME_SIZE);
	sprintf(cmdstr,"sor.sh ls %s 120",argv[0]);
	fp = popen( cmdstr, "r" );
	if(fp)
	{	
		fgets(fastfwd_name, sizeof(fastfwd_name), fp );
		ret2 = pclose(fp);	
		switch (WEXITSTATUS(ret2))
		{			
			case 0: 
				if(fastfwd_name[0]!= NULL)	
				{							
				 	ret=sor_exec(vty,"rm",argv[0],100);
				  	if(ret==CMD_SUCCESS)
					vty_out(vty,"Delete successfully! \n");
				}
				else
				{
					vty_out(vty,"Delete error! Can't get fastfwd file\n");
				}
				break;					
			default :
				vty_out(vty,"Delete error!\n");
				return CMD_WARNING;								
		}	
	}	
	else	
	{
		vty_out(vty,"Delete error!\n");		
		return CMD_WARNING; 
	}	
	
	return CMD_SUCCESS;
}
DEFUN(del_fast_fwd_all_func,
	del_fast_fwd_all_func_cmd,
	"delete fastfwd all WORD",
	"Delete configuration"
	"Delete fast-forward\n"
	"Delete fast-forward on all board\n"
	"The name of .fastfwd.bin file"
	)
{
	int ret;
	int ret2;
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;
	DBusMessageIter  iter;
	char fastfwd_name[SOR_OPFILENAME_SIZE] = {0};
	int slot_id = 0;
	FILE *fd = NULL;
	int is_active_master = 0;
	char *cmd = argv[0];
	int slot_count = get_product_info(SEM_SLOT_COUNT_PATH);

	if(strlen(argv[0])>SOR_OPFILENAME_SIZE-1)
	{
		vty_out(vty,"file name too long\n");
		return CMD_WARNING;
	}
	if(strncasecmp((argv[0]+strlen(argv[0])-12),".fastfwd.bin",12))
	{
		vty_out(vty,"The fast forward file should be .bin file\n");
		return CMD_WARNING;
	}
	get_fast_forward_name(fastfwd_name);

	if(!(strncasecmp(fastfwd_name,argv[0],strlen(argv[0]))))	
	{
		vty_out(vty,"Delete error!Delete the file is in use! \n");
		return CMD_WARNING;
	}
	fd = fopen("/dbm/local_board/is_active_master", "r");
						
	if (fd == NULL)
	{

		vty_out(vty,"Get production information [1] error\n");
		return -1;
	}

	fscanf(fd, "%d", &is_active_master);
	fclose(fd);
	if (is_active_master != 1)
	{
		vty_out(vty, "This command is only surpported by distributed system and only on active master board\n");
		return CMD_SUCCESS;
	}
	for(slot_id = 1;slot_id <=slot_count;slot_id++)
	{
		if(NULL != (dbus_connection_dcli[slot_id] -> dcli_dbus_connection))
		{	
			query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
											 SEM_DBUS_INTERFACE, SEM_DBUS_DEL_IMG_OR_FASTFWD_SLOT);
			dbus_error_init(&err);
			dbus_message_append_args(query,
									DBUS_TYPE_STRING, &cmd,
									DBUS_TYPE_INVALID);
			reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[slot_id] -> dcli_dbus_connection, query, 60000, &err);
			dbus_message_unref(query);
			
			if (NULL == reply)
			{
				vty_out(vty,"<error> failed get reply.\n");
				if (dbus_error_is_set(&err))
				{
					vty_out(vty,"%s raised: %s",err.name,err.message);
					dbus_error_free(&err);
				}
				return CMD_WARNING;
			}
	
			dbus_message_iter_init(reply,&iter);
			dbus_message_iter_get_basic(&iter,&ret);
			if(ret == 0)
			{
				vty_out(vty,"Slot %2d : Delete successfully! \n",slot_id);
			}
			else if (ret == 1)
			{
				vty_out(vty,"Slot %2d : Delete error! Cant't get IMG file\n",slot_id);
			}
			else
			{
				vty_out(vty,"Slot %2d : Delete failed\n",slot_id);
			}
	
			dbus_message_unref(reply);
		}
		else	
		{		
			vty_out(vty,"Slot %2d : Slot don't exist.\n",slot_id);
		}
	}
	
	return CMD_SUCCESS;
}

#endif
DEFUN(del_fast_fwd_slot_func,
	del_fast_fwd_slot_func_cmd,
	"delete fastfwd <1-16> WORD",
	"Delete configuration\n"
	"Delete fast-forward\n"
	"Slot of board\n"
	"The name of .fastfwd.bin file\n"
	)
{
	int ret;
	int ret2;
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter  iter;
	char fastfwd_name[SOR_OPFILENAME_SIZE] = {0};
	int slot_id;
	
	char *cmd = argv[1];
#if 0
	int slot_count = get_product_info(SEM_SLOT_COUNT_PATH);

    slot_id = strtol(argv[0], NULL, 10);
	if(slot_id > slot_count || slot_id <= 0)
	{
		vty_out(vty, "error slot number : %s\n", argv[0]);
		vty_out(vty, "correct slot number option : 1 ~ %d\n", slot_count);
        return CMD_WARNING;
	}
#endif
	slot_id = atoi(argv[0]);
	if(strlen(argv[1])>SOR_OPFILENAME_SIZE-1)
	{
		vty_out(vty,"file name too long\n");
		return CMD_WARNING;
	}
	if(strncasecmp((argv[1]+strlen(argv[1])-12),".fastfwd.bin",12))
	{
		vty_out(vty,"The fast forward file should be .bin file\n");
		return CMD_WARNING;
	}
	#if 0
	get_fast_forward_name(fastfwd_name);

	if(!(strncasecmp(fastfwd_name,argv[1],strlen(argv[1]))))	
	{
		vty_out(vty,"Delete error!Delete the file is in use! \n");
		return CMD_WARNING;
	}
	#endif
	
	if(NULL != (dbus_connection_dcli[slot_id] -> dcli_dbus_connection))
	{	
		query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
										 SEM_DBUS_INTERFACE, SEM_DBUS_DEL_IMG_OR_FASTFWD_SLOT);
		dbus_error_init(&err);
		dbus_message_append_args(query,
								DBUS_TYPE_STRING, &cmd,
								DBUS_TYPE_INVALID);
		reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[slot_id] -> dcli_dbus_connection, query, 60000, &err);
		dbus_message_unref(query);
		
		if (NULL == reply)
		{
			vty_out(vty,"<error> failed get reply.\n");
			if (dbus_error_is_set(&err))
			{
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free(&err);
			}
			return CMD_WARNING;
		}

		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&ret);
		if(ret == 0)
		{
		vty_out(vty,"Delete successfully! \n");
		}
		else if (ret == 1)
		{
			vty_out(vty,"Delete error! Cant't get fastfwd file\n\n");
		}
		else
		{
			vty_out(vty,"Failed\n");
		}

		dbus_message_unref(reply);
	}
	else	
	{		
		vty_out(vty,"The slot doesn't exist");
	}

	
	return CMD_SUCCESS;
}

/*ended by zhaocg for delet fast_forward command 2012-11-19*/



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
					if(!strncasecmp(result_file[i],imgname,strlen(imgname)))
						return CMD_SUCCESS;
				}
				return -1;
			default:
				return WEXITSTATUS(ret);
			}
	}
	else
		return -2;
}

#if 0
struct apache_conf {
        int Listen_http;
        int Listen_https;
        char ServerName[32];
} ;


struct apache_conf apache_default_conf = {
        DEFAULE_APACHE_HTTP_PORT,
         443,
	 "127.0.0.1"
};


int start_apache()
{
	int iRet,status;
       if( if_apache_enable() == APACHE_IS_ENABLE )
       {
	   	 return 0;
	}
	   
       status = system ("sudo "APACHE_PATH" start");
	iRet = WEXITSTATUS(status);
       
	return (0==iRet)?APACHE_OK:APACHE_ERR;
}


int stop_apache()
{
	int iRet,status;
	
	status = system("sudo "APACHE_PATH" stop");
	iRet = WEXITSTATUS(status);
	return (0==iRet)?APACHE_OK:APACHE_ERR;
}

int if_apache_enable() 
{
	char tmpz[128];
	int   sflag=-1;
	FILE *pipe=NULL;
		
	pipe=popen("ps -ef|grep \"/usr/sbin/apache2 -k start\" |grep -v grep|wc -l","r");
	if (pipe != NULL)
	{
        	memset(tmpz,0,sizeof(tmpz));
		fgets( tmpz, sizeof(tmpz), pipe );	
		sflag = strtoul( tmpz, 0, 10);
        	pclose( pipe );
	}

	return (sflag>0)?APACHE_IS_ENABLE:APACHE_IS_DISABLE;
}

 int apache_load_port_conf()
{
        FILE *fp = NULL;
        char line[512];
	int iret = APACHE_ERR;

	fp = fopen( CONF_APACHE_PATH, "r" );
       if( NULL != fp )
       {
	   	  memset( line, 0, sizeof( line ) );
                fgets( line, sizeof(line), fp );
		  sscanf( line, "Listen %d", &apache_default_conf.Listen_http );

	   	  memset( line, 0, sizeof( line ) );
                fgets( line, sizeof(line), fp );
		  sscanf( line, "Listen %d", &apache_default_conf.Listen_https );

	   	  memset( line, 0, sizeof( line ) );
                fgets( line, sizeof(line), fp );
		  sscanf( line, "ServerName %s", apache_default_conf.ServerName );	
		  fclose( fp );
		  fp = NULL;
		  iret = APACHE_OK;
	}
	else
	{
            iret = APACHE_ERR_FILEOPEN_ERR;
	}

	return iret;
}

static int apache_write_port_conf(  )
{
        FILE *fp = NULL;
        mode_t oldmask;
	 int Ret,status;
	 int iret = APACHE_ERR;
	 char cmd[128];
	 char content[512];
        if(access(CONF_APACHE_DIR,0)!=0)
	{
		memset(cmd,0,128);
		sprintf(cmd,"sudo chmod 777 %s",CONF_APACHE_DIR);
		status=system(cmd);
	}

	 Ret = WEXITSTATUS(status);
	 if(Ret)
	 {
            iret=APACHE_ERR_FILEOPEN_ERR-1;
	 }
        fp = fopen( CONF_APACHE_PATH, "w+" );

	if( NULL != fp )
        {       
		  memset(content,0,512);
                sprintf( content,"Listen %d \n Listen %d \n  ServerName %s \n",  apache_default_conf.Listen_http, apache_default_conf.Listen_https, apache_default_conf.ServerName);	
                fwrite(content,strlen(content),1,fp);	
                fclose(fp);
                iret =  APACHE_OK;
        }
	else
	{
		  iret = APACHE_ERR_FILEOPEN_ERR;
        }
        
        return iret;
}

int apache_set_port( int port )
{
        int iret = APACHE_ERR;

        if( port > 65535 || port < 0  )
        {
            return APACHE_ERR_PORT_ERR;
	}

        iret = apache_load_port_conf();

	apache_default_conf.Listen_http = port;
	
        if( APACHE_OK != iret  )
        {
                return iret;
	}
		
        return apache_write_port_conf(  );
}



int apache_write_default_conf(  )
{
        FILE *fp = NULL;
        mode_t oldmask;
	
        apache_default_conf.Listen_http = DEFAULE_APACHE_HTTP_PORT;
	 apache_default_conf.Listen_https = 443;
	 strcpy( apache_default_conf.ServerName, "127.0.0.1" );

        return apache_write_port_conf(  );
}

int apache_get_port()
{
        apache_load_port_conf();

	return apache_default_conf.Listen_http;
}
 
int pfm_download_config(int opt)
{
	int i;
	int result = 1;
	
	for(i = 1;i < MAX_SLOT ; i++)
		if(NULL != (dbus_connection_dcli[i] -> dcli_dbus_connection))
			if (i != HostSlotId)
			{
				result = dcli_communicate_pfm_by_dbus(opt, 0, 6, "all", 0,0, HostSlotId, "all", "all",i);
				if(0 != result)
				{					
					return -1;
				}
			}
	return 0;
}
#endif

int get_ipaddr_and_slot_by_url(char *url,char *ipaddr,char *if_name,int *slot)
{	
	char url_str[512] = {0};
	char *p_str = NULL;
	char *host = NULL;
	char *inf_str = NULL;

	int vid =0;
	FILE *fp=NULL;
	char cmd_str[64]={0};
	char buf[256] = {0};

	p_str = strstr(url,"://");
	if(NULL==p_str)
	{
		return -1;
	}
	p_str += 3;/*get ip address pointer*/
	
	memset(url_str,0,512);
	strncpy(url_str,p_str,512);
	host = strtok(url_str,":/");
	if(NULL==host)
	{
		return -1;
	}
	 
	strncpy(ipaddr,host,strlen(host)+1);
	strcat(ipaddr,"/32");
	memset(cmd_str,0,64);
	memset(buf,0,256);
	

	
	sprintf(cmd_str,"ip route get %s",host);
	fp = popen(cmd_str,"r");
	if(NULL == fp)	
	{		
   		return -1;	
	} 
	
	fgets(buf, sizeof(buf), fp ); 
	p_str = strstr(buf,"dev");
	if(NULL==p_str)
	{
		pclose(fp);
		return -1;
	}
	p_str +=4;
	inf_str = strtok(p_str," ");
	if(NULL==inf_str)
	{
		pclose(fp);
		return -1;
	}

	strncpy(if_name,inf_str,strlen(inf_str)+1);
	pclose(fp);
	
	if(*if_name==0)
		return -1;
		
	if(strncmp(if_name,"ve",2)==0)
	{
		
		if(2!=sscanf(if_name,"ve0%df1.%d",slot,&vid))
		{
			return -1;
		}
		
	}
	else if(strncmp(if_name,"eth",3)==0)
	{
		if(2!=sscanf(if_name,"eth%d-%d",slot,&vid))
		{
			return -1;
		}
	}
	else if(strncmp(if_name,"mng",3)==0)
	{
		if(2!=sscanf(if_name,"mng%d-%d",slot,&vid))
		{
			return -1;
		}
	}
	else
		return -1;
	

	return 0;
}
int get_ipaddr_and_slot_by_server(char *server_addr,char *ipaddr,char *if_name,int *slot)
{	

	int vid =0;
	FILE *fp=NULL;
	char *inf_str = NULL;
	char *p_str = NULL;
	char cmd_str[64]={0};
	char buf[256] = {0};
	 
	strcpy(ipaddr,server_addr);
	strcat(ipaddr,"/32");
	memset(cmd_str,0,64);
	memset(buf,0,256);
	
	sprintf(cmd_str,"ip route get %s",server_addr);
	fp = popen(cmd_str,"r");
	if(NULL == fp)	
	{		
   		return -1;	
	} 

	fgets(buf, sizeof(buf), fp ); 
	p_str = strstr(buf,"dev");
	if(NULL==p_str)
	{
		pclose(fp);
		return -1;
	}
	p_str +=4;
	inf_str = strtok(p_str," ");
	if(NULL==inf_str)
	{
	
		pclose(fp);
		return -1;
	}

	strncpy(if_name,inf_str,strlen(inf_str)+1);
	pclose(fp);
	
	if(*if_name==0)
		return -1;
		
	if(strncmp(if_name,"ve",2)==0)
	{
		
		if(2!=sscanf(if_name,"ve0%df1.%d",slot,&vid))
		{
			return -1;
		}
		
	}
	else if(strncmp(if_name,"eth",3)==0)
	{
		if(2!=sscanf(if_name,"eth%d-%d",slot,&vid))
		{
			return -1;
		}
	}
	else if(strncmp(if_name,"mng",3)==0)
	{
		if(2!=sscanf(if_name,"mng%d-%d",slot,&vid))
		{
			return -1;
		}
	}
	else
		return -1;
	

	return 0;
}

int pfm_download_config(int opt,int slot,char *src_ipaddr,char *if_name)
{
	int i;
	int result = 1;

	if(NULL != (dbus_connection_dcli[slot] -> dcli_dbus_connection))
		if (slot != HostSlotId)
		{
			result = dcli_communicate_pfm_by_dbus(opt, 0, 6, if_name, 0,0, HostSlotId, src_ipaddr, "all",slot);
			if(0 != result)
			{					
				return -1;
			}
			return 0;
		}
		else
			return 0;
	
	return -1;
}

DEFUN(show_boot_img_func,
	show_boot_img_func_cmd,
	"show boot_img",
	SHOW_STR
	"Show boot img which can be used infomation\n"
	)
{
	sor_lsimg(vty,120);

	return CMD_SUCCESS;
}
/*added by zhaocg for show boot_img command 2012-11-19*/

DEFUN(show_ap_boot_img_func,
	show_ap_boot_img_func_cmd,
	"show ap_boot_img",
	SHOW_STR
	"Show ap boot img which can be used information\n")
{
	sor_ap_lsimg(vty, 120);
	return CMD_SUCCESS;
}

DEFUN(show_boot_img_slot_func,
	show_boot_img_slot_func_cmd,
	"show boot_img <1-16>",
	SHOW_STR
	"Show boot img which can be used infomation\n"
	"Show boot img which can be used infomation on slot board\n"
	)
{
	DBusMessage *query=NULL; 
	DBusMessage *reply=NULL;
	DBusMessageIter  iter;
	DBusError err;
	char *ret = NULL;
	char **tem = NULL;
	char *cmdstr = "sor.sh imgls imgls 120";
	char *cmd = cmdstr;
	int fastfwdnum = 0;
	int i;
	int slot_id = 0;
#if 0
	int slot_count = get_product_info(SEM_SLOT_COUNT_PATH);
	slot_id = strtol(argv[0], NULL, 10);
	if(slot_id > slot_count || slot_id <= 0)
	{
		vty_out(vty, "error slot number : %s\n", argv[0]);
		vty_out(vty, "correct slot number option : 1 ~ %d\n", slot_count);
        return CMD_WARNING;
	}
#endif
	slot_id = atoi(argv[0]);
	

	if(NULL != (dbus_connection_dcli[slot_id] -> dcli_dbus_connection))
	{
		query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
					 SEM_DBUS_INTERFACE,SEM_DBUS_IMG_OR_FASTFWD_SLOT);

		dbus_error_init(&err);

		dbus_message_append_args(query,
					DBUS_TYPE_STRING, &cmd,
							DBUS_TYPE_INVALID);
		reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[slot_id] -> dcli_dbus_connection, query, 60000, &err);
		
		dbus_message_unref(query);

		if (NULL == reply)
		{
			vty_out(vty,"<error> failed get reply.\n");
			
			if (dbus_error_is_set(&err)) 
			{
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			
			return CMD_WARNING;
		}

		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&fastfwdnum);
		
		if(fastfwdnum != -1)
		{
			vty_out(vty,"There is %d img file\n",fastfwdnum);
		
			for(i=0;i < fastfwdnum;i++)
			{	
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&ret);
				vty_out(vty,"%s", ret);
				
			}
		}
		else
		{
			vty_out(vty,"Can't get img file info\n");
		}
		dbus_message_unref(reply);
	}
	else
	{
		vty_out(vty,"The slot doesn't exist");
	}
	
	return CMD_SUCCESS;

}
#if 0
DEFUN(del_boot_img_func,
	del_boot_img_func_cmd,
	"delete boot_img all WORD",
	"Delete configuration\n"
	"Delete boot_img file\n"
	"Delete all board boot_img file\n"
	"The name of .img file\n"
	)
{
	char cmdstr[SOR_COMMAND_SIZE];
	int ret;
	
	char result_file[64][128];
	char *cmdstr1 = "sor.sh imgls imgls 180";
	int ret2,i,imgnum,ret3;
	char imgname[SOR_OPFILENAME_SIZE] ;

	
	FILE *fp =	NULL;
#if 1 //add by houxx 20120807
	DBusMessage *query, *reply;
	DBusError err;
	
	int slot_id;
	int is_active_master = 0;
	FILE *fd;
	
	char *cmd = NULL;
#endif

	if(strncasecmp((argv[0]+strlen(argv[0])-4),".IMG",4))
	{
		vty_out(vty,"The boot file should be .img file\n");
		return CMD_WARNING;
	}
#if 0	
	sprintf(cmdstr,"sor.sh rm  %s 10",argv[0]);
	switch(system(cmdstr))
		case 0:
			return CMD_SUCCESS;
		case 1:
			vty_out(vty,"Delete bootimg error for input args wrong\n");
			break;
		case 2:
			vty_out(vty,"SAD deosn't start\n");
			break;
		case 3:
			vty_out(vty,"SAD busy\n");
			break;
		case 4:
			vty_out(vty,"SAD time out\n");
			break;
	return CMD_WARNING;
#else



	
	memset(imgname,0,SOR_OPFILENAME_SIZE);
		
	ret3 = get_boot_img_name(imgname);

	if(!(strncasecmp(imgname,argv[0],strlen(argv[0]))))	
	{
		vty_out(vty,"Delete error!Delete the file is in use! \n");
		return CMD_WARNING;
	}
	
	fp = popen( cmdstr1, "r" );

	if(fp)
	{		
		i=0;		
		while(i<64 && fgets( result_file[i], sizeof(result_file[i]), fp ))
				i++;		
		imgnum=i;		
		ret2 = pclose(fp);
			
		switch (WEXITSTATUS(ret2)) 
		{			
			case 0: 			
				for(i=0;i<imgnum;i++)
				{					
						
					if(!(strncasecmp(result_file[i],argv[0],strlen(argv[0]))))	
					{							
						ret=sor_exec(vty,"rm",argv[0],10);
						if(ret==CMD_SUCCESS)
						vty_out(vty,"Delete successfully! \n");	

#if 1	//add by houxx 20120807
						cmd = malloc(SOR_COMMAND_SIZE);
							
						fd = fopen("/dbm/local_board/is_active_master", "r");
							
						if (fd == NULL)
						{
							free(cmd);
							fprintf(stderr,"Get production information [1] error\n");
							return -1;
						}
						fscanf(fd, "%d", &is_active_master);
						fclose(fd);
						if (is_active_master != 1)
						{
					/*		vty_out(vty, "This command is only surpported by distributed system and only on active master board\n");*/
							free(cmd);
							return CMD_SUCCESS;
						}
						slot_id = 0;
						sprintf(cmd,"sor.sh rm %s 10",argv[0]);
						query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
																 SEM_DBUS_INTERFACE, SEM_DBUS_EXECUTE_SYSTEM_COMMAND);
						dbus_error_init(&err);
						dbus_message_append_args(query,
													DBUS_TYPE_UINT32, &slot_id,
													DBUS_TYPE_STRING, &cmd,
													DBUS_TYPE_INVALID);
						reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);
						dbus_message_unref(query);
					
						if (NULL == reply)
						{
							free(cmd);
							vty_out(vty,"<error> failed get reply.\n");
							if (dbus_error_is_set(&err))
							{
								vty_out(vty,"%s raised: %s",err.name,err.message);
								dbus_error_free_for_dcli(&err);
							}
							return CMD_SUCCESS;
						}
						
						if (dbus_message_get_args (reply, &err,
											DBUS_TYPE_UINT32, &ret,
											DBUS_TYPE_INVALID))
						{
							if(ret == 0)
							{
								/*vty_out(vty,"Successful\n", time);*/
								syslog(LOG_NOTICE,"Successful\n");
							}
							else if (ret == 1)
							{
								vty_out(vty,"Sem send exec message failed\n");
							}
							else
							{
								free(cmd);
								vty_out(vty,"Failed\n", time);
								dbus_message_unref(reply);
								return CMD_WARNING;
							}
						}
							
#endif 	
						dbus_message_unref(reply);
						free(cmd);	
						return CMD_SUCCESS;		
					}
				}	
											
/*						vty_out(vty,"Delete error! Cant't get IMG file	\n");
					
					break;
					*/
				default :
						vty_out(vty,"Delete error! Cant't get IMG file\n");
						return CMD_WARNING;
						break;									
		}	
	}	
	else	
	{
		vty_out(vty,"Delete error!\n");		
		return CMD_WARNING; 
	}	

#endif

}
#else
DEFUN(del_boot_img_func,
	del_boot_img_func_cmd,
	"delete boot_img all WORD",
	"Delete configuration\n"
	"Delete boot_img file\n"
	"Delete all board boot_img file\n"
	"The name of .img file\n"
	)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;
	DBusMessageIter  iter;
	int slot_id = 0;	
	int ret = 0;
	FILE *fd = NULL;
	int is_active_master = 0;
	char imgname[SOR_OPFILENAME_SIZE] = {0};
	char *cmd = argv[0];
	int slot_count = get_product_info(SEM_SLOT_COUNT_PATH);	
	if(strlen(argv[0])>SOR_OPFILENAME_SIZE-1)
	{
		vty_out(vty,"file name too long\n");
		return CMD_WARNING;
	}
	if(strncasecmp((argv[0]+strlen(argv[0])-4),".IMG",4))
	{
		vty_out(vty,"The img file should be .img file\n");
		return CMD_WARNING;
	}
	
	get_boot_img_name(imgname);

	if(!(strncasecmp(imgname,argv[0],strlen(argv[0])))) 
	{
		vty_out(vty,"Delete error!Delete the file is in use! \n");
		return CMD_WARNING;
	}
	fd = fopen("/dbm/local_board/is_active_master", "r");
							
	if (fd == NULL)
	{

		vty_out(vty,"Get production information [1] error\n");
		return -1;
	}

	fscanf(fd, "%d", &is_active_master);
	fclose(fd);
	if (is_active_master != 1)
	{
		vty_out(vty, "This command is only surpported by distributed system and only on active master board\n");
		return CMD_SUCCESS;
	}

	for(slot_id = 1;slot_id <= slot_count;slot_id++)
	{
		if(NULL != (dbus_connection_dcli[slot_id] -> dcli_dbus_connection))
		{
			query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
											 SEM_DBUS_INTERFACE, SEM_DBUS_DEL_IMG_OR_FASTFWD_SLOT);
			dbus_error_init(&err);
			dbus_message_append_args(query,
								DBUS_TYPE_STRING, &cmd,
								DBUS_TYPE_INVALID);
			reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[slot_id] -> dcli_dbus_connection, query, 60000, &err);
			dbus_message_unref(query);
	
			if (NULL == reply)
			{
				vty_out(vty,"<error> failed get reply.\n");
				if (dbus_error_is_set(&err))
				{
					vty_out(vty,"%s raised: %s",err.name,err.message);
					dbus_error_free_for_dcli(&err);
				}
				return CMD_WARNING;
			}
	
			dbus_message_iter_init(reply,&iter);
			dbus_message_iter_get_basic(&iter,&ret);
			if(ret == 0)
			{
				vty_out(vty,"Slot %2d : Delete successfully! \n",slot_id);
			}
			else if (ret == 1)
			{
				vty_out(vty,"Slot %2d : Delete error! Cant't get IMG file\n",slot_id);
			}
			else
			{
				vty_out(vty,"Slot %2d : Delete failed\n",slot_id);
			}
	
			dbus_message_unref(reply);
		}
		else	
		{		
			vty_out(vty,"Slot %2d : Slot don't exist.\n",slot_id);
		}	
	}
	return CMD_SUCCESS;
}

#endif
/*added by zhaocg for delet boot img command 2012-11-19*/
DEFUN(del_boot_img_self_func,
	del_boot_img_self_func_cmd,
	"delete boot_img self WORD",
	"Delete configuration\n"
	"Delete boot_img file\n"
	"Delete local boot_img file\n"
	"The name of .img file\n"
	)
{
	char imgname[SOR_OPFILENAME_SIZE] = {0};
	char cmdstr[SOR_COMMAND_SIZE] = {0};

	FILE*fp = NULL;
	int ret;
	int ret2;
	
	if(strlen(argv[0])>SOR_OPFILENAME_SIZE-1)
	{
		vty_out(vty,"file name too long\n");
		return CMD_WARNING;
	}	
	
	if(strncasecmp((argv[0]+strlen(argv[0])-4),".IMG",4))
	{
		vty_out(vty,"The img file should be .img file\n");
		return CMD_WARNING;
	}

	get_boot_img_name(imgname);

	if(!(strncasecmp(imgname,argv[0],strlen(argv[0]))))	
	{
		vty_out(vty,"Delete error!Delete the file is in use! \n");
		return CMD_WARNING;
	}
	
	memset(cmdstr,0,SOR_COMMAND_SIZE);
	memset(imgname,0,SOR_OPFILENAME_SIZE);
	sprintf(cmdstr,"sor.sh ls %s 120",argv[0]);
	fp = popen( cmdstr, "r" );
	if(fp)
	{	
		fgets(imgname, sizeof(imgname), fp ); 	
		ret2 = pclose(fp);	
		switch (WEXITSTATUS(ret2))
		{			
			case 0: 
				if(*imgname != 0)	
				{							
					ret=sor_exec(vty,"rm",argv[0],120);
					if(ret==CMD_SUCCESS)
					vty_out(vty,"Delete successfully! \n");
				}
				else
				{
					vty_out(vty,"Delete error! Cant't get IMG file\n");
				}
				break;					
			default :
					vty_out(vty,"Delete error! Cant't get img file\n");
					return CMD_WARNING;
					break;									
		}	
	}	
	else	
	{
		vty_out(vty,"Delete error!\n"); 	
		return CMD_WARNING; 
	}	

	return CMD_SUCCESS;
		

}

DEFUN(del_boot_img_slot_func,
	del_boot_img_slot_func_cmd,
	"delete boot_img <1-16> WORD",
	"Delete configuration"
	"Delete boot_img\n"
	"Delete SLOT_ID\n"
	"The name of .img file"
	)
{
	int ret;
	int ret2,i;
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter  iter;
	int slot_id;
	char imgname[SOR_OPFILENAME_SIZE] = {0};
	char *cmd = argv[1];
#if 0
	int slot_count = get_product_info(SEM_SLOT_COUNT_PATH);

    slot_id = strtol(argv[0], NULL, 10);
	if(slot_id > slot_count || slot_id <= 0)
	{
		vty_out(vty, "error slot number : %s\n", argv[0]);
		vty_out(vty, "correct slot number option : 1 ~ %d\n", slot_count);
        return CMD_WARNING;
	}
#endif
	slot_id = atoi(argv[0]);
	if(strlen(argv[1])>SOR_OPFILENAME_SIZE-1)
	{
		vty_out(vty,"file name too long\n");
		return CMD_WARNING;
	}
	if(strncasecmp((argv[1]+strlen(argv[1])-4),".IMG",4))
	{
		vty_out(vty,"The img file should be .img file\n");
		return CMD_WARNING;
	}
	
	get_boot_img_name(imgname);

	if(!(strncasecmp(imgname,argv[1],strlen(argv[1]))))	
	{
		vty_out(vty,"Delete error!Delete the file is in use! \n");
		return CMD_WARNING;
	}

	
	if(NULL != (dbus_connection_dcli[slot_id] -> dcli_dbus_connection))
	{
		query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
										 SEM_DBUS_INTERFACE, SEM_DBUS_DEL_IMG_OR_FASTFWD_SLOT);
		dbus_error_init(&err);
		dbus_message_append_args(query,
							DBUS_TYPE_STRING, &cmd,
							DBUS_TYPE_INVALID);
		reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[slot_id] -> dcli_dbus_connection, query, 60000, &err);
		dbus_message_unref(query);

		if (NULL == reply)
		{
			vty_out(vty,"<error> failed get reply.\n");
			if (dbus_error_is_set(&err))
			{
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free(&err);
			}
			return CMD_WARNING;
		}

		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&ret);
		if(ret == 0)
		{
		vty_out(vty,"Delete successfully! \n");
		}
		else if (ret == 1)
		{
			vty_out(vty,"Delete error! Cant't get IMG file\n");
		}
		else
		{
			vty_out(vty,"Failed\n");
		}

		dbus_message_unref(reply);
	}
	else	
	{		
		vty_out(vty,"The slot doesn't exist");
	}
	return CMD_SUCCESS;

}


DEFUN(show_running_boot_img_func,
	show_running_boot_img_func_cmd,
	"show system boot_img",
	SHOW_STR
	"Show system infomation\n"
	"Show running boot img infomation\n"
	)
{
	char imgname[128] ;
	int ret;

	memset(imgname,0,128);
	
	ret = get_boot_img_name(imgname);
	if(0==ret)
	{
		vty_out(vty,"The current boot file is %s\n",imgname);
		ret= CMD_SUCCESS;
	}
	else if(1==ret)
	{
		vty_out(vty,"Can't open the file\n");
		ret = CMD_WARNING;
	}
	else
	{
		vty_out(vty,"Can't get the file name\n");
		ret = CMD_WARNING;
	}
	
	return ret;

}
DEFUN(config_boot_img_func,
	config_boot_img_func_cmd,
	"set boot_img WORD",
	"Set system configuration"
	"Set boot img infomation\n"
	"Set boot img name\n"
	)
{
	int ret=0;
	char old_bootimg_name[64] = {0};
	char dbus_str_pre[128] = {0};
	char dbus_str[256]={0};
	
	get_boot_img_name(old_bootimg_name);

	sprintf(dbus_str_pre,"Old boot img file %s,new boot img file %s",old_bootimg_name,argv[0]);
	if(strncasecmp((argv[0]+strlen(argv[0])-4),".IMG",4))
	{
		vty_out(vty,"The boot file should be .img file\n");
		sprintf(dbus_str,"%s set boot img file format failure\n",dbus_str_pre);
		dcli_send_dbus_signal("set_boot_img_failed",dbus_str);

		return CMD_WARNING;
	}
	
	ret = sor_checkimg(argv[0]);
	if( ret ==  CMD_SUCCESS ){
		ret = set_boot_img_name(argv[0]);
		if( 0 == ret)
		{
			vty_out(vty,"\nSet boot img success\n");
			sprintf(dbus_str,"%s set boot img file success\n",dbus_str_pre);
			dcli_send_dbus_signal("set_boot_img_success",dbus_str);
			ret=CMD_SUCCESS;
		}
		else
		{
			ret= CMD_WARNING;
			vty_out(vty,"Config boot_img failure ret is %d\n",ret);

			
			sprintf(dbus_str,"%s set boot img failure\n",dbus_str_pre);
			dcli_send_dbus_signal("set_boot_img_failed",dbus_str);
			ret= CMD_WARNING;
		}
	}
	else
	{
		switch(ret)
		{
		
			case -1:
				vty_out(vty,"The boot img %s doesn't exist.\n",argv[0]);
				
				sprintf(dbus_str,"%s The boot img doesn't exist.\n",dbus_str_pre);
				dcli_send_dbus_signal("set_boot_img_failed",dbus_str);
				break;
			case 1:
				vty_out(vty,"Sysetm internal error (1).\n");
				
				sprintf(dbus_str,"%s:Sysetm internal error (1).\n",dbus_str_pre);
				dcli_send_dbus_signal("set_boot_img_failed",dbus_str);
				break;
			case 2:
				vty_out(vty,"Sysetm internal error (2).\n");
				
				sprintf(dbus_str,"%s:Sysetm internal error (2).\n",dbus_str_pre);
				dcli_send_dbus_signal("set_boot_img_failed",dbus_str);
				break;
			case 3:
				vty_out(vty,"Storage media is busy.\n");
				
				sprintf(dbus_str,"%s:Storage media is busy.\n",dbus_str_pre);
				dcli_send_dbus_signal("set_boot_img_failed",dbus_str);
				break;
			case 4:
				vty_out(vty,"Storage operation time out.\n");
				sprintf(dbus_str,"%s:Storage operation time out.\n",dbus_str_pre);
				dcli_send_dbus_signal("set_boot_img_failed",dbus_str);
				
				break;
			case 5:
				vty_out(vty,"No left space on storage media.\n");
				
				sprintf(dbus_str,"%s:No left space on storage media.\n",dbus_str_pre);
				dcli_send_dbus_signal("set_boot_img_failed",dbus_str);
				break;
			default:
				vty_out(vty,"Sysetm internal error (3).\n");
				
				sprintf(dbus_str,"%s:Sysetm internal error (3).\n",dbus_str_pre);
				dcli_send_dbus_signal("set_boot_img_failed",dbus_str);
				break;
				}
	
		sprintf(dbus_str,"%s:check boot img error\n",dbus_str_pre);
		dcli_send_dbus_signal("set_boot_img_failed",dbus_str);
	}

	return ret;

}

/*
 * set boot img on every board in the whole system 
 * caojia add, 20111027
 */
DEFUN(config_system_img_func,
	config_system_img_func_cmd,
	"set system_img WORD",
	"Set system configuration"
	"Set system boot img infomation\n"
	"Set system boot img name\n"
	)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned long long ret = 0x0ull;
	int board_on_mask = 0;
	int slot_count = 10;
	char old_bootimg_name[64] = {0};
	char dbus_str_pre[128] = {0};
	char dbus_str[256]={0};
	char version_file_temp[64];
	char *version_file = version_file_temp;
	FILE *fd;
	int retval;
	int i;

	/* after mcb active standby switched, the is_active_master need get value again , caojia added*/
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
		fprintf(stderr,"Get production information [2] error\n");
		return -1;
	}
	fscanf(fd, "%d", &slot_count);
	fclose(fd);
	//vty_out(vty, "Slot Count : %d\n", slot_count);
	
	//get_boot_img_name(old_bootimg_name);

	sprintf(dbus_str_pre,"Old boot img file %s,new boot img file %s",old_bootimg_name,argv[0]);
	if(strncasecmp((argv[0]+strlen(argv[0])-4),".IMG",4))
	{
		vty_out(vty,"The boot file should be .img file\n");
		//sprintf(dbus_str,"%s set boot img file format failure\n",dbus_str_pre);
		//dcli_send_dbus_signal("set_boot_img_failed",dbus_str);

		return CMD_WARNING;
	}

	memset(version_file_temp, 0, 64);
	strcpy(version_file_temp, argv[0]);

	query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
												 SEM_DBUS_INTERFACE, SEM_DBUS_SET_SYSTEM_IMG);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_STRING,&version_file,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, 60000, &err);
	
	dbus_message_unref(query);

	if (NULL == reply){
		vty_out(vty,"<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args (reply, &err,
					DBUS_TYPE_UINT64, &ret,
					DBUS_TYPE_INVALID)){
		//vty_out(vty, "ret : %llx\n", ret);
		for (i = 0; i < slot_count; i++)
		{
			retval = (ret >> (i * 4)) & 0xf;
			retval -= 1;
			
			vty_out(vty, "Slot %2d : ", i + 1);
			switch(retval)
			{
				case -1:
					vty_out(vty,"Slot Empty.\n");
					
					break;
				case 0:
					vty_out(vty,"Set system img success.\n");
					
					break;
				case 1:
					vty_out(vty,"Sysetm internal error (1).\n");
					
					//sprintf(dbus_str,"%s:Sysetm internal error (1).\n",dbus_str_pre);
					//dcli_send_dbus_signal("set_boot_img_failed",dbus_str);
					break;
				case 2:
					vty_out(vty,"Sysetm internal error (2).\n");
					
					//sprintf(dbus_str,"%s:Sysetm internal error (2).\n",dbus_str_pre);
					//dcli_send_dbus_signal("set_boot_img_failed",dbus_str);
					break;
				case 3:
					vty_out(vty,"Storage media is busy.\n");
					
					//sprintf(dbus_str,"%s:Storage media is busy.\n",dbus_str_pre);
					//dcli_send_dbus_signal("set_boot_img_failed",dbus_str);
					break;
				case 4:
					vty_out(vty,"Storage operation time out.\n");
					
					//sprintf(dbus_str,"%s:Storage operation time out.\n",dbus_str_pre);
					//dcli_send_dbus_signal("set_boot_img_failed",dbus_str);
					break;
				case 5:
					vty_out(vty,"No left space on storage media.\n");
					
					//sprintf(dbus_str,"%s:No left space on storage media.\n",dbus_str_pre);
					//dcli_send_dbus_signal("set_boot_img_failed",dbus_str);
					break;
				case 6:
					vty_out(vty,"The boot img %s doesn't exist.\n",argv[0]);
					
					//sprintf(dbus_str,"%s The boot img doesn't exist.\n",dbus_str_pre);
					//dcli_send_dbus_signal("set_boot_img_failed",dbus_str);
					break;
				case 0xd:
					vty_out(vty,"Sem send or receive set system_img msg error.\n",ret);

					break;
				case 0xe:
					vty_out(vty,"Config boot_img failure.\n");

					break;
				default:
					vty_out(vty,"Sysetm internal error (3).\n");
					
					//sprintf(dbus_str,"%s:Sysetm internal error (3).\n",dbus_str_pre);
					//dcli_send_dbus_signal("set_boot_img_failed",dbus_str);
					break;
					}
		
			//sprintf(dbus_str,"%s:check boot img error\n",dbus_str_pre);
			//dcli_send_dbus_signal("set_boot_img_failed",dbus_str);
		}
	}

	return ret;

}

/*
 * Display designated board environment variable
 * niehy@autelan.com add, 2012-09-18
 */

DEFUN(show_boot_env_var_func,
	show_boot_env_var_func_cmd,
	"show boot_env_var <1-16> NAME",
	SHOW_STR
	"Show bootrom environment variable infomation\n"
	"the slot id number\n"
	"Show bootrom environment variable name\n"
	)
{
	DBusMessage *query, *reply;
	DBusError err;
	int ret,res;
	unsigned short slot_id;
	int slot_count = get_product_info(SEM_SLOT_COUNT_PATH);
	char env_name[256] ;
	char *envname = env_name;

    //slot_id = strtoul(argv[0], NULL, 10);
    res = parse_single_param_no((char*)argv[0],&slot_id);
	if(res != 0)
	{
	 	vty_out(vty,"%% parse param failed!\n");
		return CMD_WARNING;        
	}
	if(slot_id > slot_count || slot_id <= 0)
	{
		vty_out(vty, "error slot number : %s\n", argv[0]);
		vty_out(vty, "correct slot number option : 1 ~ %d\n", slot_count);
        return CMD_WARNING;
	}
	memset(env_name, 0, 256);
	strcpy(env_name, argv[1]);

	query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
												 SEM_DBUS_INTERFACE, SEM_DBUS_SHOW_BOOTROM_ENVIRONMENT_VARIABLE);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_STRING,&envname,
							DBUS_TYPE_INVALID);

	if (dbus_connection_dcli[slot_id]->dcli_dbus_connection)
	{
        reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,query,-1, &err);
		dbus_message_unref(query);
        if (NULL == reply)
		{
			vty_out(vty,"<error> failed get reply.\n");
			if (dbus_error_is_set(&err)) 
			{
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_WARNING;
		}
		if(dbus_message_get_args (reply, &err,
					DBUS_TYPE_UINT32, &ret,
					DBUS_TYPE_STRING,&envname,
					DBUS_TYPE_INVALID)){
					
    		if(ret == 0)
    		{
                vty_out(vty,"%s = %s\n",env_name,envname);
    			dbus_message_unref(reply);
                return CMD_SUCCESS;
    		}
    		else if(2 == ret)
    		{
                vty_out(vty, "Bootrom environment variable %s does not exis!\n",env_name);
    			dbus_message_unref(reply);
                return CMD_WARNING;
			}
			else
    		{
				vty_out(vty,"show bootrom environment variable command fail\n");
				vty_out(vty,"ret = %d\n",ret);
    			dbus_message_unref(reply);
                return CMD_WARNING;
    		}
		}
		else
		{
            vty_out(vty,"Failed get args.\n");
            if (dbus_error_is_set(&err))
            {
                printf("%s raised: %s",err.name,err.message);
            	dbus_error_free_for_dcli(&err);
            }
			dbus_message_unref(reply);
            return CMD_WARNING;
         }
	}
	else
	{
		vty_out(vty, "no connection to slot %d\n", slot_id);
		return CMD_WARNING;
	}
}

DEFUN(set_boot_env_bootcmd_func,
	set_boot_env_bootcmd_func_cmd,
	"set bootcmd <1-16> (1|2|3|4|5|6|7|8|9|10|11|12|13|14)",
    SETT_STR
    "set bootcmd configuration\n"
    "the slot id number must be in <1-16>\n"
    " AX71_SMU board (don't take fast forward)\n"    
    " AX71_SMU board (take fast forward for 2G memory)\n" 
    " AX71_2x12g12s board\n"
    " AX71_1x12g12s board(Don't take fast forward)\n"    
    " AX71_1x12g12s board(take fast forward for 4G memory)\n"
    " AX81_SMU board\n"  
    " AX81_AC8C/AX81_AC12C board(Don't take fast forward)\n"
    " AX81_AC8C/AX81_AC12C board(take fast forward for 4G memory)\n"
    " AX81_AC4X board(Don't take fast forward)\n"
    " AX81_AC4X board(take fast forward for 4G memory)\n"
    " AX81_12x board\n"
    " AX81_1x12g12s board(Don't take fast forward)\n"
    " AX81_1x12g12s board(take fast forward for 4G memory)\n"
    " AX81_2x12g12s board\n"  
    
)

{
	DBusMessage *query, *reply;
	DBusError err;
	int ret,res;
	int retval;
	unsigned short slot_id;
	int slot_count = get_product_info(SEM_SLOT_COUNT_PATH);
	char env_name[256] ;
	char *name = "bootcmd";	
	char *input_num_temp = argv[1];	

    /*slot_id = strtoul(argv[0], NULL, 10);*/
    res = parse_single_param_no((char*)argv[0],&slot_id);
	if(res != 0)
	{
	 	vty_out(vty,"%% parse param failed!\n");
		return CMD_WARNING;        
	}
	if(slot_id > slot_count || slot_id < 1 )
	{
		vty_out(vty, "error slot number : %s\n", argv[0]);
		vty_out(vty, "correct slot number option : 1 ~ %d\n", slot_count);
        return CMD_WARNING;
	}
	memset(env_name, 0, 256);	

	    strcpy(env_name, name);
	    query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
												 SEM_DBUS_INTERFACE, SEM_DBUS_SET_BOOTCMD);
		dbus_error_init(&err);
		dbus_message_append_args(query,
							DBUS_TYPE_STRING,&name,
							DBUS_TYPE_STRING, &input_num_temp,
							DBUS_TYPE_INVALID);
		if (dbus_connection_dcli[slot_id]->dcli_dbus_connection)
		{
			reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,query,-1, &err);
			dbus_message_unref(query);
			if (NULL == reply)
			{
    			vty_out(vty,"<error> failed get reply.\n");
    			if (dbus_error_is_set(&err)) 
    			{
    				vty_out(vty,"%s raised: %s",err.name,err.message);
    				dbus_error_free_for_dcli(&err);
    			}
    			return CMD_WARNING;
			}
    		if(dbus_message_get_args (reply, &err,
    					DBUS_TYPE_UINT32, &ret,
    					DBUS_TYPE_INVALID))
    		{
        		if(ret == 0)
        		{
    				vty_out(vty, "set environment variable bootcmd  command success!\n");
        			dbus_message_unref(reply);
                    return CMD_SUCCESS;
        		}
        		else if (ret == -1)
        		{
                   vty_out(vty, "input parameter error,slot%d board type doesn't correct \n",slot_id);
				   dbus_message_unref(reply);
                    return CMD_WARNING;
				}
				else if (ret == -2)
				{
                    vty_out(vty,"Error! Not find the item: %d\n",argv[1]);
					dbus_message_unref(reply);
                    return CMD_WARNING;
				}
				else
        		{
                    vty_out(vty, "set environment variable bootcmd  command fail!\n");
    				vty_out(vty,"ret = %d\n",ret);
        			dbus_message_unref(reply);
                    return CMD_WARNING;
        		}
			}
			else
    		{
                vty_out(vty,"Failed get args.\n");
                if (dbus_error_is_set(&err))
                {
                    printf("%s raised: %s",err.name,err.message);
                	dbus_error_free_for_dcli(&err);
                }
    			dbus_message_unref(reply);
                return CMD_WARNING;
            }
	    }
    	else
    	{
        	vty_out(vty, "no connection to slot %d\n", slot_id);
        	return CMD_WARNING;
    	}
		
}

DEFUN(set_boot_env_var_func,
	set_boot_env_var_func_cmd,
	"set boot_env_var <1-16> NAME .LINE",
	SETT_STR
	"set bootrom environment variable\n"
	"the slot id number\n"
	"set bootrom environment variable name\n"
	"Set bootrom environment variable value\n"
	)
{
	DBusMessage *query, *reply;
	DBusError err;
	int ret,res;
	unsigned short slot_id;	
	int slot_count = get_product_info(SEM_SLOT_COUNT_PATH);
	char env_name[256] ;	    
	char *envname = env_name;		
    char *input_value = NULL; 	 
	
	//slot_id = strtoul(argv[0], NULL, 10);
	res = parse_single_param_no((char*)argv[0],&slot_id);
	if(res != 0)
	{
	 	vty_out(vty,"%% parse param failed!\n");
		return CMD_WARNING;        
	}
	if(slot_id > slot_count || slot_id <= 0)
	{
		vty_out(vty, "error slot number : %s\n", argv[0]);
		vty_out(vty, "correct slot number option : 1 ~ %d\n", slot_count);
        return CMD_WARNING;
	}
	
	memset(env_name, 0, 256);
	strcpy(env_name, argv[1]);
	char sel[64] = {0};
	
    input_value = argv_concat(&argv[2], argc - 2, 0);
    if( !input_value )
	{
        vty_out(vty,"get new value is NULL\n");
        return CMD_WARNING;
    }
	
	if(strlen(input_value) > BOOT_ENV_VALUE_LEN)
  	{
  		vty_out(vty,"Environment variable vlaue was too long!(must be less than %d)\n",BOOT_ENV_VALUE_LEN);
		XFREE(MTYPE_TMP,input_value);
		return CMD_WARNING;
				
  	}


   if(strcmp(envname,"sefile") == 0)
   {
        if(!strncasecmp((argv[2]+strlen(argv[2])-4),".BIN",4))
        {			
			goto DBUS;   		
    	}
    	else
    	{
    		 vty_out(vty,"the sefile may not be the .bin file,you want to contnue? [yes/no]:\n");
    		 fscanf(stdin, "%s", sel);
    		 while(1)
    		 {
    		 	    if(!strncasecmp("yes", sel, strlen(sel)))
    		 	    {
						goto DBUS;
    				}
    				else if(!strncasecmp("no", sel, strlen(sel)))
    				{
    				
						XFREE(MTYPE_TMP,input_value);
                       	return CMD_WARNING;
                    }
    				else
    				{
                        vty_out(vty,"% Please input 'yes' or 'no'.\n");
                        vty_out(vty,"are you going on to set sefile [yes/no]:\n");
                        memset(sel, 0, 64);
                        fscanf(stdin, "%s", sel);
                    }
			}
    	}
    }

DBUS:
	query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
												 SEM_DBUS_INTERFACE, SEM_DBUS_SET_ENVIRONMENT_VARIABLE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
						DBUS_TYPE_STRING,&envname,
						DBUS_TYPE_STRING, &input_value,
						DBUS_TYPE_INVALID);
	if (dbus_connection_dcli[slot_id]->dcli_dbus_connection)
	{
		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,query,-1, &err);


		dbus_message_unref(query);
		if (NULL == reply)
		{
    		vty_out(vty,"<error> failed get reply.\n");
    		if (dbus_error_is_set(&err)) 
    		{
    			vty_out(vty,"%s raised: %s",err.name,err.message);
    			dbus_error_free_for_dcli(&err);
    		}
			
			XFREE(MTYPE_TMP,input_value);
    		return CMD_WARNING;
		}
    	if(dbus_message_get_args (reply, &err,
    				DBUS_TYPE_UINT32, &ret,
    				DBUS_TYPE_INVALID))
    	{
               
			
       		if(ret == 0)
       		{
    			vty_out(vty, "set environment variable %s success!\n",env_name);
       			dbus_message_unref(reply);
				
				XFREE(MTYPE_TMP,input_value);
                return CMD_SUCCESS;
       		}
			else if(ret == -1)
			{

                vty_out(vty,"this  board  type  doesn't support fastfwd\n");
				dbus_message_unref(reply);
				
				XFREE(MTYPE_TMP,input_value);
				return CMD_WARNING;

			}			
			else if(ret == -2)
			{
                 vty_out(vty,"The fastfwd file %s doesn't exist.\n",argv[2]); 
				 dbus_message_unref(reply);
				 
				 XFREE(MTYPE_TMP,input_value);
				 return CMD_WARNING;
			}
       		else  
       		{
				vty_out(vty, "set environment variable %s fail!\n",env_name);
    			vty_out(vty,"ret = %d\n",ret);
        		dbus_message_unref(reply);
				
				XFREE(MTYPE_TMP,input_value);
                return CMD_WARNING;
        	}
		}
		else
    	{
			vty_out(vty,"Failed get args.\n");
            if (dbus_error_is_set(&err))
            {
				printf("%s raised: %s",err.name,err.message);
                dbus_error_free_for_dcli(&err);
            }
    		dbus_message_unref(reply);
			
			XFREE(MTYPE_TMP,input_value);
            return CMD_WARNING;
        }
	}
    else
    {
       	vty_out(vty, "no connection to slot %d\n", slot_id);
		
		XFREE(MTYPE_TMP,input_value);
       	return CMD_WARNING;
    }
 

}

DEFUN(show_boot_version_func,
	show_boot_version_func_cmd,
	"show boot version",
	SHOW_STR
	"Show boot version infomation\n"
	)
{
	char vername[128] ;
	int ret;
	char *strl = vername;
	char *tmp = vername;

	memset(vername,0,128);
	ret = get_boot_version_name(vername);
	if(0==ret)
	{
		while(*tmp)
			{
				if(*tmp=='(')
					{
						*(tmp-1)='\0';
						vty_out(vty,"%s\n",strl);
						strl=tmp;
					}
				tmp++;
			}
		vty_out(vty,"%s\0",strl);
		return CMD_SUCCESS;
	}
	else if(1==ret)
	{
		vty_out(vty,"Can't open the file\n");
	}
	else
	{
		vty_out(vty,"Can't get the boot version information,please reboot!\n");
	}
	
	return CMD_WARNING;

}

DEFUN (download_fastforward_func,
	   	download_fastforward_cmd,
	   	"download fastforward URL USERNAME PASSWORD",
			"Download system infomation\n"
			"Download system fastforward\n"
	   	"The fastforward file location&name\n"
	   	"User name\n"
	   	"Password\n")

{
	char cmd[256]={0},path[256]={0},oldimgfilename[128]={0};
	int ret;
	char dbus_str_pre[128] = {0};
	char dbus_str[256]={0};
	char ipaddr[32] = {0};
	char if_name[32]= {0};
	int send_slot = 0;
	ret = get_ipaddr_and_slot_by_url(argv[0],ipaddr,if_name,&send_slot);/*zhaocg add for pfm*/
	if(ret != 0)
	{
		vty_out(vty,"The URL is wrong,please check it\n");
		return CMD_WARNING;
	}
	char* filename=strrchr(argv[0],'/');
	if(!filename)
	{
		vty_out(vty,"The URL is wrong,pls check it\n");
		return CMD_WARNING;
	}
	filename++;
	memset(cmd,0,256);
	memset(path,0,256);
	if(filename)
	{
		int i=0;
		char* upfilename = 	NULL;
		int filenamelen = strlen(filename);
		char *p = NULL, *q = NULL;		
		int result = 1;
#if 1 //added by houxx		
		if(((p = strstr(argv[0],"http:")) != NULL) || (q = strstr(argv[0],"ftp:") != NULL))
		{
			result = pfm_download_config(0,send_slot,ipaddr,if_name);
			if(result != 0)
			{
				vty_out(vty,"config_pfm_failed");
				return CMD_WARNING;
			}
		}
#endif

		
		
		i=snprintf(cmd,256,"downimg.sh %s %s %s %s \n",argv[0],argv[1],argv[2],filename);
		if(i>256)
		{
			vty_out(vty,"patch too long\n");
			
			if(((p = strstr(argv[0],"http:")) != NULL) || (q = strstr(argv[0],"ftp") != NULL))
			{
				
				result = pfm_download_config(1,send_slot,ipaddr,if_name);
				if(result != 0)
				{
					vty_out(vty,"config_pfm_failed");
					return CMD_WARNING;
				}
			}
			return CMD_WARNING;
		}
		if(system(cmd))
		{
			vty_out(vty,"Can't download fastforward file successly\n");
			
#if 1 //added by houxx		
			if(((p = strstr(argv[0],"http:")) != NULL) || (q = strstr(argv[0],"ftp") != NULL))
			{
				
				result = pfm_download_config(1,send_slot,ipaddr,if_name);
				if(result != 0)
				{
					vty_out(vty,"config_pfm_failed");
					return CMD_WARNING;
				}
			}
#endif
			
			return CMD_WARNING;

		}
		
#if 1 //added by houxx		
		if(((p = strstr(argv[0],"http:")) != NULL) || (q = strstr(argv[0],"ftp") != NULL))
		{
			
			result = pfm_download_config(1,send_slot,ipaddr,if_name);
			if(result != 0)
			{
				vty_out(vty,"config_pfm_failed");
				return CMD_WARNING;
			}
		}
#endif

		upfilename=malloc(filenamelen+1);
		if(NULL==upfilename)
		{
			vty_out(vty,"System Error,doesn't download success\n");

			return CMD_WARNING;

		}
		memset(upfilename,0,filenamelen+1);
		i = 0;
		while(filename[i]&& i <= filenamelen)
		{
			upfilename[i] = (filename[i]);
			i++;			
		}
		sprintf(path,"/mnt/%s",filename);
		{
			if(strcmp(filename,upfilename)){
				sprintf(cmd,"mv %s /mnt/%s > ~/down.log 2>&1\n ",path,upfilename);
				if(system(cmd))
				{
					vty_out(vty,"Download fastforward file wrong\n");
					free(upfilename);
					
					return CMD_WARNING;
				}
			}
			vty_out(vty,"System is write file,must not power down\n");
			memset(cmd,0,sizeof(cmd));
#if 0			
			sprintf(cmd,"sor.sh cp %s 300",upfilename);
			if(system(cmd))
			{
				vty_out(vty,"Download fastforward file wrong\n");
				free(upfilename);
				return CMD_WARNING;
			}
#else
			if(sor_exec(vty,"cp",upfilename,500)!=CMD_SUCCESS)
			{
				vty_out(vty,"Download fastforward file wrong\n");
				free(upfilename);
				
				return CMD_WARNING;
			}								

#endif
			
			/* book add for synchronize img file, 2011-09-15 */
			char sel[PATH_LEN] = {0};
			int board_count = 0;
			int ID[MAX_SLOT_NUM] = {0};
			char a_returnString[BSD_COMMAND_BUF_LEN] = {0};
			//fflush(stdin);
			vty_out(vty,"Finish downloading system fastfwd file.\n");
			vty_out(vty,"Synchronize fastfwd file to other boards? This may cost a few seconds. [yes/no]:\n");
			fscanf(stdin, "%s", sel);
			while(1){
				if(!strncasecmp("yes", sel, strlen(sel))){
					vty_out(vty,"Start synchronizing, please wait...\n");
					char src_path[PATH_LEN] = {0};
					char des_path[PATH_LEN] = {0};
					char resMd5[PATH_LEN] = {0};
					sprintf(src_path, "/blk/%s", filename);
					sprintf(des_path, "/blk/%s", filename);
	        #if 0
					ret = dcli_bsd_copy_files_to_boards(dcli_dbus_connection,src_path, des_path, BSD_OP_BOOT_IMG);
	        #else
					board_count = dcli_bsd_get_slot_ids(dcli_dbus_connection,ID,BSD_TYPE_BOOT_IMG);
					//printf("count = %d\n",board_count);
					for(i = 0; i < board_count; i++)
					{
					    memset(resMd5, 0, PATH_LEN);
						vty_out(vty,"start synchronizing to slot_%d...\n",ID[i]);
						ret = dcli_bsd_copy_file_to_board(dcli_dbus_connection,ID[i],src_path,des_path,0,BSD_TYPE_BLK, resMd5);
						vty_out(vty,"File md5 value on dest board is %s\n", (char*)resMd5);
						vty_out(vty,"%s", dcli_bsd_get_return_string(ret, a_returnString));
					}
	        #endif
					break;
				}
				else if(!strncasecmp("no", sel, strlen(sel))){
					break;
				}
				else{
					vty_out(vty,"% Please answer 'yes' or 'no'.\n");
					vty_out(vty,"Synchronize fastforward file to other boards? [yes/no]:\n");
					memset(sel, 0, PATH_LEN);
					fscanf(stdin, "%s", sel);
				}
			}
			/* book add end */

			vty_out(vty,"System fastforward file has been download.\n");
			free(upfilename);
		}
		
	}
	

	return CMD_SUCCESS;
}
DEFUN (download_fastforward_slot_func,
	   	download_fastforward_slot_cmd,
	   	"download fastforward slot <1-15> URL USERNAME PASSWORD",
		"Download system infomation\n"
		"Download system fastforward\n"
		"Board slot for interface local mode\n"
		"Slot id\n"
	   	"The fastforward file location&name\n"
	   	"User name\n"
	   	"Password\n")
{
	DBusMessage *query = NULL; 
	DBusMessage *reply = NULL;
	DBusMessageIter  iter;
	DBusError err; 
	
	char* upfilename = 	NULL;
	char path[256]={0};
	char cmd[512] = {0};
	char *cmd_str = cmd;
	char *err_str = NULL;
	char *urlstr = argv[1];
	char *username = argv[2];
	char *password = argv[3];
	char* filename = NULL;
	char *md5_str = NULL;
	char dbus_str_pre[128] = {0};
	char dbus_str[256]={0};
	int ret = 0;
	int opt = 0;
	int slot_id = 0;
	int i=0;
	int filenamelen = 0;
	
	slot_id = atoi(argv[0]);
	if(slot_id == HostSlotId)
	{
		vty_out(vty,"Can't config the active master board,please config other board\n");
		return CMD_WARNING;
	}
	
	filename=strrchr(argv[1],'/');
	if(!filename)
	{
		vty_out(vty,"The URL is wrong,pls check it\n");
		
		sprintf(dbus_str,"%s: The URL is wrong,pls check it\n",dbus_str_pre);
		dcli_send_dbus_signal("download_boot_img_failed",dbus_str);
		return CMD_WARNING;
	}
	
	filename++;
	filenamelen = strlen(filename);
	if(filenamelen>255||(strlen(argv[1])+strlen(argv[2])+strlen(argv[3])+strlen(filename))>448)
	{
		vty_out(vty,"arguments is too long\n");
		return CMD_WARNING;
	}
	if(NULL != (dbus_connection_dcli[slot_id] -> dcli_dbus_connection))
	{
	
		sprintf(cmd,"downimg.sh %s %s %s %s",urlstr,username,password,filename);
		query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
					 SEM_DBUS_INTERFACE,SEM_DBUS_DOWNLOAD_IMG_SLOT);

		dbus_error_init(&err);

		dbus_message_append_args(query,
								DBUS_TYPE_STRING, &cmd_str,
								DBUS_TYPE_INVALID);
		reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[slot_id] -> dcli_dbus_connection, query, 200000, &err);
		
		dbus_message_unref(query);

		if (NULL == reply)
		{
			vty_out(vty,"<error> failed get reply.\n");
			
			if (dbus_error_is_set(&err)) 
			{
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			
			return CMD_WARNING;
		}

		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&ret);
		
		dbus_message_unref(reply);
		if (-1 == ret)	
		{  
		   vty_out(vty,"system error!"); 
		   return CMD_WARNING;
		}  
		else  
		{  
		   if (WIFEXITED(ret))	
		   {  
			   switch (WEXITSTATUS(ret))
			   {	
					case 0: 		 
						   break;
					default:			
						vty_out(vty,"download error\n");
						return CMD_WARNING; 	
			   }	
		   }  
		   else  
		   {  
			   vty_out(vty,"exit status = [%d]\n", WEXITSTATUS(ret)); 
			   return CMD_WARNING;
		   }  
		} 
		
		vty_out(vty,"System is write file,must not power down\n"); 
		memset(cmd,0,sizeof(cmd));
		sprintf(cmd,"/mnt/%s",filename);

		query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
					 SEM_DBUS_INTERFACE,SEM_DBUS_DOWNLOAD_CPY_CONFIG_SLOT);

		dbus_error_init(&err);

		dbus_message_append_args(query,
								DBUS_TYPE_UINT32, &slot_id,
								DBUS_TYPE_UINT32, &HostSlotId,					
								DBUS_TYPE_STRING, &cmd_str,
								DBUS_TYPE_INVALID);
		reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[slot_id] -> dcli_dbus_connection, query, 200000, &err);
		
		dbus_message_unref(query);

		if (NULL == reply)
		{
			vty_out(vty,"<error> failed get reply.\n");
			
			if (dbus_error_is_set(&err)) 
			{
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			
			return CMD_WARNING;
		}

		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&ret);
		if(ret==1)
		{
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&err_str);
			vty_out(vty,"%s",err_str);
			dbus_message_unref(reply);
			return CMD_WARNING;
		}
		else
		{
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&opt);
			dbus_message_unref(reply);
		
			if (-1 == opt)	
			{  
			   vty_out(vty,"system error!"); 
			   return CMD_WARNING;
			}  
			else  
			{  
			   if (WIFEXITED(opt))	
			   {  
				   switch (WEXITSTATUS(opt))
				   {	
						case 0: 		 
						   break;
						default:			
							vty_out(vty,"download error\n");		
							return CMD_WARNING;
					}	
			   }  
			   else  
			   {  
				   vty_out(vty,"exit status = [%d]\n", WEXITSTATUS(opt));
				   return CMD_WARNING;
			   }  
			} 
		}
	   
	}
	else
	{
		vty_out(vty,"The slot doesn't exist");
		
		return CMD_WARNING;
	}
	
	upfilename=malloc(filenamelen+1);
	if(NULL==upfilename)
	{
		vty_out(vty,"System Error,doesn't download success\n");
		return CMD_WARNING;
	}
	memset(upfilename,0,filenamelen+1);
	while(filename[i]&& i <= filenamelen)
	{
		upfilename[i] = (filename[i]);
		i++;			
	}
		
	sprintf(path,"/mnt/%s",filename);
	
	if(strcmp(filename,upfilename))
	{
		sprintf(cmd,"mv %s /mnt/%s > ~/down.log 2>&1\n ",path,upfilename);
		if(system(cmd))
		{
			vty_out(vty,"Download fastforward file wrong\n");
			free(upfilename);	
			return CMD_WARNING;
		}
	}
	memset(cmd,0,sizeof(cmd));

	if(sor_exec(vty,"cp",upfilename,500)!=CMD_SUCCESS)
	{
		vty_out(vty,"Download fastforward file wrong\n");
		free(upfilename);
		return CMD_WARNING;
	}								

    sor_md5img(vty, upfilename);

	/* book add for synchronize img file, 2011-09-15 */
	char sel[PATH_LEN] = {0};
	int board_count = 0;
	int ID[MAX_SLOT_NUM] = {0};
	char a_returnString[BSD_COMMAND_BUF_LEN] = {0};
	vty_out(vty,"Finish downloading system fastfwd file.\n");
	vty_out(vty,"Synchronize fastfwd file to other boards? This may cost a few seconds. [yes/no]:\n");
	fscanf(stdin, "%s", sel);
	while(1)
	{
		if(!strncasecmp("yes", sel, strlen(sel)))
		{
			vty_out(vty,"Start synchronizing, please wait...\n");
			char src_path[PATH_LEN] = {0};
			char des_path[PATH_LEN] = {0};
			char resMd5[PATH_LEN] = {0};
			sprintf(src_path, "/blk/%s", filename);
			sprintf(des_path, "/blk/%s", filename);
   			board_count = dcli_bsd_get_slot_ids(dcli_dbus_connection,ID,BSD_TYPE_BOOT_IMG);
			for(i = 0; i < board_count; i++)
			{
			    memset(resMd5, 0, PATH_LEN);
				vty_out(vty,"start synchronizing to slot_%d...\n",ID[i]);
				ret = dcli_bsd_copy_file_to_board(dcli_dbus_connection,ID[i],src_path,des_path,0,BSD_TYPE_BLK, resMd5);
				vty_out(vty,"File md5 value on dest board is %s\n", (char*)resMd5);
				vty_out(vty,"%s", dcli_bsd_get_return_string(ret, a_returnString));
			}
			break;
		}
		else if(!strncasecmp("no", sel, strlen(sel)))
		{
			break;
		}
		else
		{
			vty_out(vty,"% Please answer 'yes' or 'no'.\n");
			vty_out(vty,"Synchronize fastforward file to other boards? [yes/no]:\n");
			memset(sel, 0, PATH_LEN);
			fscanf(stdin, "%s", sel);
		}
	}
	/* book add end */
	vty_out(vty,"System fastforward file has been download.\n");
	free(upfilename);
	return CMD_SUCCESS;
}

	   	

DEFUN (download_system_func,
	   	download_system_cmd,
	   	"download img URL [USERNAME] [PASSWORD]",
			"Download system infomation\n"
			"Download system img\n"
	   	"The img file location&name\n"
	   	"User name\n"
	   	"Password\n")
{
	char cmd[256]={0},path[256]={0},oldimgfilename[128]={0};
	char* filename=strrchr(argv[0],'/');
	int ret;
	
	char old_bootimg_name[64] = {0};
	char dbus_str_pre[128] = {0};
	char dbus_str[256]={0};
	char ipaddr[32] = {0};
	char if_name[32]= {0};
	int send_slot = 0;
    /************************/
	int i=0;
	char sel[PATH_LEN] = {0};
	int board_count = 0;
	int ID[MAX_SLOT_NUM] = {0};
	char a_returnString[BSD_COMMAND_BUF_LEN] = {0};
	//fflush(stdin);

	/************************/	
	char *tm = NULL,*usbp = NULL;
	char *imgname = NULL;
	int status,status1;
	char temp_buf[100];
	int temp;
	char src[100]={0};
	int profuct_type = get_product_info(BOARD_81SMU_PATH);
	if((usbp = strstr(argv[0],"usb:")) != NULL){
		if(argc !=1){
			vty_out(vty,"please cheack input parameter \n");
			return CMD_WARNING;
		}
	}
	else{
		if(argc != 3){
			vty_out(vty,"please cheack input parameter \n");
			return CMD_WARNING;
		}
	}
	
    if((tm = strstr(argv[0],"usb:")) != NULL)
    {				
        if(profuct_type == AX81SMU)
    	{
            imgname  = tm + 4;					
            if(strncasecmp((imgname + strlen(imgname)-4),".IMG",4))
            {
                vty_out(vty,"The boot file should be .img file\n");
                return CMD_WARNING;
            }
            memset(temp_buf,0,100);
            sprintf(temp_buf,"/usr/bin/cp_img_to_card.sh %s",imgname);
            vty_out(vty,"start copying,please waite for few minutes \n");
            status = system(temp_buf);
            if ( 0 == WEXITSTATUS(status))
    		{ 	
                memset(cmd,0,256);	
                sprintf(cmd,"sor.sh cp %s 300",imgname);
                status1 = system(cmd);
                if (status1 != -1)
    			{
                    if (WIFEXITED(status1))
        			{
                        if (0 != WEXITSTATUS(status1))
    					{
                        	vty_out(vty, "cp file failed.\n");
                        	return CMD_WARNING;
                        }				
                    }
                }
                memset(src,0,100);
                sprintf(src,"rm -r  /mnt/%s",imgname);
                system(src);
                ret = sor_checkimg(imgname);
                if( ret ==  CMD_SUCCESS )
    			{
                    ret = set_boot_img_name(imgname);
                    if( 0 == ret)
                    {
                        vty_out(vty,"\nSet boot img success\n");
                        //return CMD_SUCCESS;
                    }
                    else
                    {
                        vty_out(vty,"Config boot_img failure ret is %d\n",ret);
                        return CMD_WARNING;
                    }
                }
                else
                {
                    switch(ret)
                    {
                        case -1:
                        vty_out(vty,"The boot img %s doesn't exist.\n",argv[0]);
                        break;
                        case -2:
                        vty_out(vty,"Sysetm internal error (1).\n");
                        break;

                        default:
                        vty_out(vty,"Sysetm internal error (3).\n");
                        break;
                    }
                    return CMD_WARNING;
                }		
            }	
            else if( 0 != WEXITSTATUS(status))
			{
                vty_out(vty,"cp file failed..\n");
                return  CMD_WARNING;
            }

            filename = imgname;
            vty_out(vty,"Finish downloading system img file.\n");
            vty_out(vty,"Synchronize img file to other boards? This may cost a few seconds. [yes/no]:\n");
            fscanf(stdin, "%s", sel);
            while(1)
			{
                if(!strncasecmp("yes", sel, strlen(sel)))
				{
                    vty_out(vty,"Start synchronizing, please wait...\n");
                    char src_path[PATH_LEN] = {0};
                    char des_path[PATH_LEN] = {0};
                    char res_md5[PATH_LEN] = {0};
                    sprintf(src_path, "/blk/%s", filename);
                    sprintf(des_path, "/blk/%s", filename);
#if 0
                    ret = dcli_bsd_copy_files_to_boards(dcli_dbus_connection,src_path, des_path, BSD_OP_BOOT_IMG);
#else
                    board_count = dcli_bsd_get_slot_ids(dcli_dbus_connection,ID,BSD_TYPE_BOOT_IMG);
                    //printf("count = %d\n",board_count);
                    for(i = 0; i < board_count; i++)
                    {
                        memset(res_md5, 0, PATH_LEN);
                        vty_out(vty,"start synchronizing to slot_%d...\n",ID[i]);
                        ret = dcli_bsd_copy_file_to_board(dcli_dbus_connection,ID[i],src_path,des_path,0,BSD_TYPE_BOOT_IMG, res_md5);
                        vty_out(vty,"File md5 value on dest board is %s\n", (char*)res_md5);
                        vty_out(vty,"%s", dcli_bsd_get_return_string(ret, a_returnString));
                    }
#endif
                    break;
                }
                else if(!strncasecmp("no", sel, strlen(sel)))
				{
                    break;
                }
                else
				{
                    vty_out(vty,"% Please answer 'yes' or 'no'.\n");
                    vty_out(vty,"Synchronize img file to other boards? [yes/no]:\n");
                    memset(sel, 0, PATH_LEN);
                    fscanf(stdin, "%s", sel);
                }
            }
            return CMD_SUCCESS;
        }
        else
		{ 
            vty_out(vty,"board_type nonsupport usb device\n");	
            return CMD_WARNING;
        }
    }
	/***************************/

	ret = get_ipaddr_and_slot_by_url(argv[0],ipaddr,if_name,&send_slot);/*zhaocg add for pfm*/
	/*
	vty_out(vty,"The URL is ipaddr=%s,if_name=%s,send_slot=%d\n",ipaddr,if_name,send_slot);
	*/
	if(ret != 0)
	{
		vty_out(vty,"The URL is wrong,please check it\n");
		return CMD_WARNING;
	}
	get_boot_img_name(old_bootimg_name);

	sprintf(dbus_str_pre,"Old boot img file %s,new boot img file %s",old_bootimg_name,filename);

	sprintf(dbus_str,"%s: download img begin\n",dbus_str_pre);

	syslog(LOG_NOTICE,"dbus_str: %s", dbus_str);

	dcli_send_dbus_signal("download_boot_img_begin",dbus_str);
	if(!filename)
	{
		vty_out(vty,"The URL is wrong,pls check it\n");
		
		sprintf(dbus_str,"%s: The URL is wrong,pls check it\n",dbus_str_pre);
		dcli_send_dbus_signal("download_boot_img_failed",dbus_str);
		return CMD_WARNING;
	}
	filename++;
	memset(cmd,0,256);
	memset(path,0,256);
	if(filename)
	{
		int i=0;
		char* upfilename = 	NULL;
		int filenamelen = strlen(filename);
		char *p = NULL, *q = NULL;		
		int result = 1;
#if 1 //added by houxx		
		if(((p = strstr(argv[0],"http:")) != NULL) || (q = strstr(argv[0],"ftp:") != NULL))
		{
			result = pfm_download_config(0,send_slot,ipaddr,if_name);
			if(result != 0)
			{
				vty_out(vty,"config_pfm_failed");
				return CMD_WARNING;
			}
		}
#endif

		
		
		sprintf(cmd,"downimg.sh %s %s %s %s \n",argv[0],argv[1],argv[2],filename);
		if(system(cmd))
		{
			vty_out(vty,"Can't download img successly\n");
			
			sprintf(dbus_str,"%s: Can't download img successly",dbus_str_pre);
			dcli_send_dbus_signal("download_boot_img_failed",dbus_str);

#if 1 //added by houxx		
			if(((p = strstr(argv[0],"http:")) != NULL) || (q = strstr(argv[0],"ftp") != NULL))
			{
				
				result = pfm_download_config(1,send_slot,ipaddr,if_name);
				if(result != 0)
				{
					vty_out(vty,"config_pfm_failed");
					return CMD_WARNING;
				}
			}
#endif
			
			return CMD_WARNING;

		}
		
#if 1 //added by houxx		
		if(((p = strstr(argv[0],"http:")) != NULL) || (q = strstr(argv[0],"ftp") != NULL))
		{
			
			result = pfm_download_config(1,send_slot,ipaddr,if_name);
			if(result != 0)
			{
				vty_out(vty,"config_pfm_failed");
				return CMD_WARNING;
			}
		}
#endif

		upfilename=malloc(filenamelen+1);
		if(NULL==upfilename)
		{
			vty_out(vty,"System Error,doesn't download success\n");

			
			sprintf(dbus_str,"%s: System Error,doesn't download success\n",dbus_str_pre);
			dcli_send_dbus_signal("download_boot_img_failed",dbus_str);
			return CMD_WARNING;

		}
		memset(upfilename,0,filenamelen+1);
		i = 0;
		while(filename[i]&& i <= filenamelen)
		{
			upfilename[i] = toupper(filename[i]);
			i++;			
		}
		sprintf(path,"/mnt/%s",filename);
		if(!check_img(path)){
			if(strcmp(filename,upfilename)){
				sprintf(cmd,"mv %s /mnt/%s > ~/down.log 2>&1\n ",path,upfilename);
				if(system(cmd))
				{
					vty_out(vty,"Download img file wrong\n");
					free(upfilename);
					
					sprintf(dbus_str,"%s: Download img file wrong\n",dbus_str_pre);
					dcli_send_dbus_signal("download_boot_img_failed",dbus_str);
					return CMD_WARNING;
				}
			}
			vty_out(vty,"System is write file,must not power down\n");
			memset(cmd,0,sizeof(cmd));
#if 0			
			sprintf(cmd,"sor.sh cp %s 300",upfilename);
			if(system(cmd))
			{
				vty_out(vty,"Download img file wrong\n");
				free(upfilename);
				return CMD_WARNING;
			}
#else
			if(sor_exec(vty,"cp",upfilename,500)!=CMD_SUCCESS)
			{
				vty_out(vty,"Download img file wrong\n");
				free(upfilename);
				
				sprintf(dbus_str,"%s: Save img file wrong\n",dbus_str_pre);
				dcli_send_dbus_signal("download_boot_img_failed",dbus_str);
				return CMD_WARNING;
			}								

#endif
			i = 0;
			while(filename[i]&& i <= filenamelen)
			{
				filename[i] = toupper(filename[i]);
				i++;	
			}

			set_boot_img_name(filename);

            /* show md5 on screen, zhangshu add */
            sor_md5img(vty, filename);
            #if 0//huangjing
			/* book add for synchronize img file, 2011-09-15 */
			char sel[PATH_LEN] = {0};
			int board_count = 0;
			int ID[MAX_SLOT_NUM] = {0};
			char a_returnString[BSD_COMMAND_BUF_LEN] = {0};
			#endif
			//fflush(stdin);
			vty_out(vty,"Finish downloading system img file.\n");
			vty_out(vty,"Synchronize img file to other boards? This may cost a few seconds. [yes/no]:\n");
			fscanf(stdin, "%s", sel);
			while(1){
				if(!strncasecmp("yes", sel, strlen(sel))){
					vty_out(vty,"Start synchronizing, please wait...\n");
					char src_path[PATH_LEN] = {0};
			        char des_path[PATH_LEN] = {0};
			        char res_md5[PATH_LEN] = {0};
			        sprintf(src_path, "/blk/%s", filename);
			        sprintf(des_path, "/blk/%s", filename);
			        #if 0
			        ret = dcli_bsd_copy_files_to_boards(dcli_dbus_connection,src_path, des_path, BSD_OP_BOOT_IMG);
			        #else
			        board_count = dcli_bsd_get_slot_ids(dcli_dbus_connection,ID,BSD_TYPE_BOOT_IMG);
			        //printf("count = %d\n",board_count);
			        for(i = 0; i < board_count; i++)
			        {
			            memset(res_md5, 0, PATH_LEN);
			            vty_out(vty,"start synchronizing to slot_%d...\n",ID[i]);
                        ret = dcli_bsd_copy_file_to_board(dcli_dbus_connection,ID[i],src_path,des_path,0,BSD_TYPE_BOOT_IMG, res_md5);
                        vty_out(vty,"File md5 value on dest board is %s\n", (char*)res_md5);
			            vty_out(vty,"%s", dcli_bsd_get_return_string(ret, a_returnString));
			        }
			        #endif
					break;
				}
				else if(!strncasecmp("no", sel, strlen(sel))){
					break;
				}
				else{
					vty_out(vty,"% Please answer 'yes' or 'no'.\n");
					vty_out(vty,"Synchronize img file to other boards? [yes/no]:\n");
					memset(sel, 0, PATH_LEN);
					fscanf(stdin, "%s", sel);
				}
			}
			/* book add end */

			vty_out(vty,"System img file has been download ,please reset system\n");
			free(upfilename);
		}
		else
		{
			vty_out(vty,"The img file is error\n");
			free(upfilename);
			
			sprintf(dbus_str,"%s: The img file is error\n",dbus_str_pre);
			dcli_send_dbus_signal("download_boot_img_failed",dbus_str);
			return CMD_WARNING;
		}
	}
	
	sprintf(dbus_str,"%s: download img success\n",dbus_str_pre);
	dcli_send_dbus_signal("download_boot_img_success",dbus_str);
	return CMD_SUCCESS;
}
/*******************zhaocg add ******/
DEFUN (download_system_slot_func,
	   	download_system_slot_cmd,
	   	"download img slot <1-15> URL [USERNAME] [PASSWORD]",
		"Download system infomation\n"
		"Download system img\n"
		"Board slot for interface local mode\n"
	   	"Slot id\n"
	   	"The img file location&name\n"
	   	"User name\n"
	   	"Password\n")
{
	DBusMessage *query = NULL; 
	DBusMessage *reply = NULL;
	DBusMessageIter  iter;
	DBusError err; 
	
	/************************/
	int i=0;
	char sel[PATH_LEN] = {0};
	int board_count = 0;
	int ID[MAX_SLOT_NUM] = {0};
	char a_returnString[BSD_COMMAND_BUF_LEN] = {0};

	/************************/	

	FILE *fp = NULL;
	char *upfilename = NULL;
	char cmd[512] = {0};
	char *cmd_str = cmd;
	char path[256] = {0};
	char *err_str = NULL;
	char *urlstr = argv[1];
	char *username = argv[2];
	char *password = argv[3];
	char* filename = NULL;
	char *md5_str = NULL;
	char dbus_str_pre[128] = {0};
	char dbus_str[256]={0};
	int ret = 0;
	int opt = 0;
	int slot_id = 0;
	int download_img_flag = 0;
	int timeout = 0;
	int filenamelen = 0;
	
	slot_id = atoi(argv[0]);
	if(slot_id == HostSlotId)
	{
		vty_out(vty,"Can't config the active master board,please config other board\n");
		return CMD_WARNING;
	}
	
	filename=strrchr(argv[1],'/');
	if(!filename)
	{
		vty_out(vty,"The URL is wrong,pls check it\n");
		
		sprintf(dbus_str,"%s: The URL is wrong,pls check it\n",dbus_str_pre);
		dcli_send_dbus_signal("download_boot_img_failed",dbus_str);
		return CMD_WARNING;
	}
	filename++;
	filenamelen = strlen(filename);
	if(filenamelen>255||(strlen(argv[0])+strlen(argv[1])+strlen(argv[2])+strlen(filename))>448)
	{
		vty_out(vty,"arguments is too long\n");
		return CMD_WARNING;
	}
	if(NULL != (dbus_connection_dcli[slot_id] -> dcli_dbus_connection))
	{
		
		sprintf(cmd,"downimg.sh %s %s %s %s",urlstr,username,password,filename);
		query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
					 SEM_DBUS_INTERFACE,SEM_DBUS_DOWNLOAD_IMG_SLOT);

		dbus_error_init(&err);
		dbus_message_append_args(query,
										DBUS_TYPE_STRING, &cmd_str,
										DBUS_TYPE_INVALID);
/*
		dbus_message_append_args(query,
								DBUS_TYPE_STRING, &urlstr,					
								DBUS_TYPE_STRING, &username,
								DBUS_TYPE_STRING, &password,
								DBUS_TYPE_STRING, &filename,
								DBUS_TYPE_INVALID);
*/
		reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[slot_id] -> dcli_dbus_connection, query, 300000, &err);
		
		dbus_message_unref(query);

		if (NULL == reply)
		{
			vty_out(vty,"<error> failed get reply.\n");
			
			if (dbus_error_is_set(&err)) 
			{
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			
			return CMD_WARNING;
		}

		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&ret);
		
		dbus_message_unref(reply);
		if (-1 == ret)	
		{  
		   vty_out(vty,"system error!"); 
		   return CMD_WARNING;
		}  
		else  
		{ 
		   if (WIFEXITED(ret))	
		   {  
			   switch (WEXITSTATUS(ret))
			   {	
					case 0: 		 
						   break;
					default:			
						vty_out(vty,"download error\n");
						return CMD_WARNING;		
				}	
		   }  
		   else  
		   {  
			   vty_out(vty,"exit status = [%d]\n", WEXITSTATUS(ret)); 
			   return CMD_WARNING;
		   }  
	    }
	    
		vty_out(vty,"System is write file,must not power down\n"); 
		memset(cmd,0,512);
		sprintf(cmd,"/mnt/%s",filename);

		query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
					 SEM_DBUS_INTERFACE,SEM_DBUS_DOWNLOAD_CPY_CONFIG_SLOT);

		dbus_error_init(&err);

		dbus_message_append_args(query,
								DBUS_TYPE_UINT32, &slot_id,
								DBUS_TYPE_UINT32, &HostSlotId,					
								DBUS_TYPE_STRING, &cmd_str,
								DBUS_TYPE_INVALID);
		reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[slot_id] -> dcli_dbus_connection, query, 200000, &err);
		
		dbus_message_unref(query);

		if (NULL == reply)
		{
			vty_out(vty,"<error> failed get reply.\n");
			
			if (dbus_error_is_set(&err)) 
			{
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			
			return CMD_WARNING;
		}

		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&ret);
		if(ret==1)
		{
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&err_str);
			vty_out(vty,"%s",err_str);
			dbus_message_unref(reply);
			return CMD_WARNING;
		}
		else
		{
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&opt);
			/*
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&md5_str);
			
			vty_out(vty,"%s",md5_str);
			*/
			dbus_message_unref(reply);
		
			if (-1 == opt)	
			{  
			   vty_out(vty,"system error!"); 
			   return CMD_WARNING;
			}  
			else  
			{  
			   if (WIFEXITED(opt))	
			   {  
				   switch (WEXITSTATUS(opt))
				   {	
						case 0: 		 		   
							   break;
						default:			
							vty_out(vty,"copy img error\n"); 		
							return CMD_WARNING;
					}	
			   }  
			   else  
			   {  
				   vty_out(vty,"exit status = [%d]\n", WEXITSTATUS(opt));
				   return CMD_WARNING;
			   }  
		    } 
		}
	   
	}
	else
	{
		vty_out(vty,"The slot doesn't exist");
		
		return CMD_WARNING;
	}

	upfilename=malloc(filenamelen+1);
	if(NULL==upfilename)
	{
		vty_out(vty,"System Error,doesn't download success\n");
		sprintf(dbus_str,"%s: System Error,doesn't download success\n",dbus_str_pre);
		dcli_send_dbus_signal("download_boot_img_failed",dbus_str);
		return CMD_WARNING;

	}
	memset(upfilename,0,filenamelen+1);
	i = 0;
	while(filename[i]&& i <= filenamelen)
	{
		upfilename[i] = toupper(filename[i]);
		i++;			
	}
	sprintf(path,"/mnt/%s",filename);
	if(!check_img(path))
	{
		if(strcmp(filename,upfilename))
		{
			sprintf(cmd,"mv %s /mnt/%s > ~/down.log 2>&1\n ",path,upfilename);
			if(system(cmd))
			{
				vty_out(vty,"Download img file wrong\n");
				free(upfilename);
				sprintf(dbus_str,"%s: Download img file wrong\n",dbus_str_pre);
				dcli_send_dbus_signal("download_boot_img_failed",dbus_str);
				return CMD_WARNING;
			}
		}
		
		memset(cmd,0,sizeof(cmd));
		if(sor_exec(vty,"cp",upfilename,500)!=CMD_SUCCESS)
		{
			vty_out(vty,"Download img file wrong\n");
			free(upfilename);
			sprintf(dbus_str,"%s: Save img file wrong\n",dbus_str_pre);
			dcli_send_dbus_signal("download_boot_img_failed",dbus_str);
			return CMD_WARNING;
		}								

		i = 0;
		while(filename[i]&& i <= filenamelen)
		{
			filename[i] = toupper(filename[i]);
			i++;	
		}

		set_boot_img_name(filename);

        /* show md5 on screen, zhangshu add */
        sor_md5img(vty, filename);
		vty_out(vty,"Finish downloading system img file.\n");
		vty_out(vty,"Synchronize img file to other boards? This may cost a few seconds. [yes/no]:\n");
		fscanf(stdin, "%s", sel);
		while(1)
		{
			if(!strncasecmp("yes", sel, strlen(sel)))
			{
				vty_out(vty,"Start synchronizing, please wait...\n");
				char src_path[PATH_LEN] = {0};
		        char des_path[PATH_LEN] = {0};
		        char res_md5[PATH_LEN] = {0};
		        sprintf(src_path, "/blk/%s", filename);
		        sprintf(des_path, "/blk/%s", filename);
		        
		        board_count = dcli_bsd_get_slot_ids(dcli_dbus_connection,ID,BSD_TYPE_BOOT_IMG);
		        //printf("count = %d\n",board_count);
		        for(i = 0; i < board_count; i++)
		        {
		            memset(res_md5, 0, PATH_LEN);
		            vty_out(vty,"start synchronizing to slot_%d...\n",ID[i]);
                    ret = dcli_bsd_copy_file_to_board(dcli_dbus_connection,ID[i],src_path,des_path,0,BSD_TYPE_BOOT_IMG, res_md5);
                    vty_out(vty,"File md5 value on dest board is %s\n", (char*)res_md5);
		            vty_out(vty,"%s", dcli_bsd_get_return_string(ret, a_returnString));
		        }
				break;
			}
			else if(!strncasecmp("no", sel, strlen(sel)))
			{
				break;
			}
			else
			{
				vty_out(vty,"% Please answer 'yes' or 'no'.\n");
				vty_out(vty,"Synchronize img file to other boards? [yes/no]:\n");
				memset(sel, 0, PATH_LEN);
				fscanf(stdin, "%s", sel);
			}
		}
		/* book add end */

		vty_out(vty,"System img file has been download ,please reset system\n");
		free(upfilename);
	}
		
	sprintf(dbus_str,"%s: download img success\n",dbus_str_pre);
	dcli_send_dbus_signal("download_boot_img_success",dbus_str);

	return CMD_SUCCESS;
}


/*******************zhaocg end ******/
void update_conf_flag_write()
{
	FILE* fp;
	fp=fopen("/mnt/newconf", "w");
	if(!fp)
		return;
	fprintf(fp,"1");
	fclose(fp);
	sor_exec(NULL,"cp","newconf",5);
	
	return;
}
DEFUN (download_system_config_func,
	   download_system_config_cmd,
	   "download configure URL USERNAME PASSWORD [CONFIGFILENAME]",
	   "Download system infomation\n"
	   "Download system configure\n"
	   "The system configure file location&name\n"
	   "User name\n"
	   "Password\n"
	   "Config file name you want save\n")
{
	char cmd[256];
	char filename_temp[64];
	char* filename=strrchr(argv[0],'/');
	int ret;
	char* temp;
	char ipaddr[32] = {0};
	char if_name[32]= {0};
	int send_slot = 0;
	ret = get_ipaddr_and_slot_by_url(argv[0],ipaddr,if_name,&send_slot);/*zhaocg add for pfm*/
	if(ret != 0)
	{
		vty_out(vty,"The URL is wrong,please check it\n");
		return CMD_WARNING;
	}
	memset(filename_temp,0,64);
	if(!filename)
	{
		vty_out(vty,"The URL is wrong,pls check it\n");
		
		dcli_send_dbus_signal("download_config_failure","The URL is wrong");
		return CMD_WARNING;
	}
	filename++;
	if(4==argc)
	{
		temp=argv[3];
		if(strlen(argv[3])>VTYSH_MAX_FILE_LEN)
  	{
  			vty_out(vty,"The config file name you saved was too long!(must be less than %d)\n",VTYSH_MAX_FILE_LEN);
			dcli_send_dbus_signal("download_config_failure","The file name was too long");
			return CMD_WARNING;
				
  	}
	if(!((temp=strstr(temp,".conf"))&&!(*(temp+5))))//if temp does not has ".conf" and ".conf" in the end then..
	{
		vty_out(vty,"The config file name your seved does not end with \".conf\".Please end with \".conf\".\n");
		dcli_send_dbus_signal("download_config_failure","The config file name without \".conf\"!");

		return CMD_WARNING;
	}
	}
	temp=filename;
	if(strlen(filename)>VTYSH_MAX_FILE_LEN)
  	{
  			vty_out(vty,"The config file name on the server was too long!(must be less than %d)\n",VTYSH_MAX_FILE_LEN);
			dcli_send_dbus_signal("download_config_failure","The config file name on the server was too long!");
			return CMD_WARNING;
				
  	}
	if(!((temp=strstr(temp,".conf"))&&!(*(temp+5))))//if temp does not has ".conf" and ".conf" in the end then..
	{
		vty_out(vty,"The config file name on the server does not end with \".conf\".Please end with \".conf\".\n");
		dcli_send_dbus_signal("download_config_failure","The config file name on the server without \".conf\"!");

		return CMD_WARNING;
	}
		
	sprintf(filename_temp,"/mnt/%s",filename);
	if(4==argc&&(0==access(filename_temp,F_OK)))
	{
		memset(cmd,0,256);
		sprintf(cmd,"mv /mnt/%s /mnt/backup_config.conf >/dev/null",filename);
		fprintf(stdout,"%s\n",cmd);
		ret = system(cmd);
		ret = WEXITSTATUS(ret);
		if(0!=ret)
		{
			vty_out(vty,"COPY ERROR\n");
			dcli_send_dbus_signal("download_config_failure","Copy to flash ERROR!");
			
			return CMD_WARNING;
		}
	}
/*	
	if(filename)
*/
	{
		int i=0;
		int result = 1;
		char *p = NULL, *q = NULL;
		memset(cmd,0,256);
			
		
#if 1 //added by houxx		
		if(((p = strstr(argv[0],"http:")) != NULL) || (q = strstr(argv[0],"ftp:") != NULL))
		{
			result = pfm_download_config(0,send_slot,ipaddr,if_name);
			if(result != 0)
			{
				vty_out(vty,"config_pfm_failed");
				return CMD_WARNING;
			}
		}
#endif
		
		sprintf(cmd,"downimg.sh %s %s %s %s \n",argv[0],argv[1],argv[2],filename);
		ret = system(cmd);
		
#if 1 //added by houxx		
		if(((p = strstr(argv[0],"http:")) != NULL) || (q = strstr(argv[0],"ftp:") != NULL))
		{
			result = pfm_download_config(1,send_slot,ipaddr,if_name);
			if(result != 0)
			{
				vty_out(vty,"config_pfm_failed");
				return CMD_WARNING;
			}
		}
#endif		
		if(!WEXITSTATUS(ret))
		{
			vty_out(vty,"System is writing configure file,must not power down\n");
			
			dcli_send_dbus_signal("download_config_success","download config success");
			memset(filename_temp,0,64);
			if(4==argc)
			{
				strcpy(filename_temp,argv[3]);
				if(0!=strcmp(filename,filename_temp))
				{
					sprintf(cmd,"mv /mnt/%s /mnt/%s >/dev/null\n",filename,filename_temp);
			
					ret = system(cmd);
					ret = WEXITSTATUS(ret);
					if(0!=ret)
					{
						vty_out(vty,"COPY ERROR\n");
						dcli_send_dbus_signal("update_config_failure","Copy to flash ERROR!");
						
						return CMD_WARNING;
						}
					}
				}else{
				strcpy(filename_temp,filename);
					}
				
#if 0
			if(system("sor.sh cp conf_xml.conf 10"))
#else
			if(sor_exec(vty,"cp",filename_temp,60)!=CMD_SUCCESS)

#endif
			{
				vty_out(vty,"Download configure file wrong\n");
				dcli_send_dbus_signal("update_config_failure","Copy to flash ERROR!");
				
				return CMD_WARNING;
			}
			vty_out(vty,"System has writen configure file successly,please reboot system\n");
			}
			else
			{
				vty_out(vty,"Download configure file failure\n");
				dcli_send_dbus_signal("download_config_failure","Download configure file failure");
				return CMD_WARNING;

			}
	}

	if(4==argc&&(0==access("/mnt/backup_config.conf",F_OK)))
	{
		memset(cmd,0,256);
		sprintf(cmd,"mv /mnt/backup_config.conf /mnt/%s >/dev/null",filename);
		ret = system(cmd);
		ret = WEXITSTATUS(ret);
		if(0!=ret)
		{
			vty_out(vty,"COPY ERROR\n");
			return CMD_WARNING;
		}
	}
	update_conf_flag_write();
	dcli_send_dbus_signal("update_config_success","update config success");
	return CMD_SUCCESS;
}
DEFUN (download_system_config_slot_func,
	   download_system_config_slot_cmd,
	   "download configure slot <1-15> URL USERNAME PASSWORD [CONFIGFILENAME]",
	   "Download system infomation\n"
	   "Download system configure\n"
	   "Board slot for interface local mode\n"
	   "Slot id\n"
	   "The system configure file location&name\n"
	   "User name\n"
	   "Password\n"
	   "Config file name you want save\n")
{
	DBusMessage *query = NULL; 
	DBusMessage *reply = NULL;
	DBusMessageIter  iter;
	DBusError err; 
	char *urlstr = argv[1];
	char *username = argv[2];
	char *password = argv[3];
	char *err_str = NULL;
	int slot_id = 0;
	char cmd[512]={0};
	char *cmd_str = cmd;
	char filename_temp[128] = {0};
	char *filename_str = filename_temp;
	char* filename=NULL;
	int ret =0;
	int opt =0;
	char* temp = NULL;
	char ipaddr[32] = {0};
	char if_name[32]= {0};
	
	slot_id = atoi(argv[0]);
	if(slot_id == HostSlotId)
	{
		vty_out(vty,"Can't config the active master board,please config other board\n");
		return CMD_WARNING;
	}
	
	filename=strrchr(argv[1],'/');
	if(!filename)
	{
		vty_out(vty,"The URL is wrong,pls check it\n");
		
		dcli_send_dbus_signal("download_config_failure","The URL is wrong");
		return CMD_WARNING;
	}
	filename++;
	if((strlen(argv[1])+strlen(argv[2])+strlen(argv[3])+strlen(argv[4]))>448)
	{
		vty_out(vty,"arguments is too long\n");
		return CMD_WARNING;
	}
	if(5==argc)
	{
		temp=argv[4];
		if(strlen(argv[4])>VTYSH_MAX_FILE_LEN)
	  	{
	  		vty_out(vty,"The config file name you saved was too long!(must be less than %d)\n",VTYSH_MAX_FILE_LEN);
			dcli_send_dbus_signal("download_config_failure","The file name was too long");
			return CMD_WARNING;
					
	  	}
		if(!((temp=strstr(temp,".conf"))&&!(*(temp+5))))//if temp does not has ".conf" and ".conf" in the end then..
		{
			vty_out(vty,"The config file name your seved does not end with \".conf\".Please end with \".conf\".\n");
			dcli_send_dbus_signal("download_config_failure","The config file name without \".conf\"!");

			return CMD_WARNING;
		}
	}
	
	temp=filename;
	if(strlen(filename)>VTYSH_MAX_FILE_LEN)
  	{
  		vty_out(vty,"The config file name on the server was too long!(must be less than %d)\n",VTYSH_MAX_FILE_LEN);
		dcli_send_dbus_signal("download_config_failure","The config file name on the server was too long!");
		return CMD_WARNING;
				
  	}
	if(!((temp=strstr(temp,".conf"))&&!(*(temp+5))))//if temp does not has ".conf" and ".conf" in the end then..
	{
		vty_out(vty,"The config file name on the server does not end with \".conf\".Please end with \".conf\".\n");
		dcli_send_dbus_signal("download_config_failure","The config file name on the server without \".conf\"!");

		return CMD_WARNING;
	}
		
	memset(filename_temp,0,sizeof(filename_temp));
	sprintf(filename_temp,"/mnt/%s",filename);
	if(5==argc&&(0==access(filename_temp,F_OK)))
	{
		memset(cmd,0,512);
		sprintf(cmd,"mv /mnt/%s /mnt/backup_config.conf >/dev/null",filename);
		fprintf(stdout,"%s\n",cmd);
		ret = system(cmd);
		ret = WEXITSTATUS(ret);
		if(0!=ret)
		{
			vty_out(vty,"COPY ERROR\n");
			dcli_send_dbus_signal("download_config_failure","Copy to flash ERROR!");
			
			return CMD_WARNING;
		}
	}
	if((strlen(argv[0])+strlen(argv[1])+strlen(argv[2])+strlen(filename))>512)
	{
		vty_out(vty,"arguments is too long\n");
		return CMD_WARNING;
	}

	if(NULL != (dbus_connection_dcli[slot_id] -> dcli_dbus_connection))
	{
	
		memset(cmd,0,sizeof(cmd));
		sprintf(cmd,"downimg.sh %s %s %s %s",urlstr,username,password,filename);
		query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
					 SEM_DBUS_INTERFACE,SEM_DBUS_DOWNLOAD_IMG_SLOT);

		dbus_error_init(&err);

		dbus_message_append_args(query,
								DBUS_TYPE_STRING, &cmd_str,
								DBUS_TYPE_INVALID);
		reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[slot_id] -> dcli_dbus_connection, query, 200000, &err);
		
		dbus_message_unref(query);

		if (NULL == reply)
		{
			vty_out(vty,"<error> failed get reply.\n");
			
			if (dbus_error_is_set(&err)) 
			{
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			
			return CMD_WARNING;
		}

		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&ret);
		
		dbus_message_unref(reply);
		if (-1 == ret)	
		{  
		   vty_out(vty,"system error!");  
		   return CMD_WARNING;	   
		}  
		else  
		{  
		   if (WIFEXITED(ret))	
		   {  
			   switch (WEXITSTATUS(ret))
			   {	
					case 0: 		 
						   break;
					default:			
						vty_out(vty,"download error\n");
						return CMD_WARNING; 	
			   }	
		   }  
		   else  
		   {  
			   vty_out(vty,"exit status = [%d]\n", WEXITSTATUS(ret)); 
			   return CMD_WARNING;
		   }  
		} 
		
		vty_out(vty,"System is writing configure file,must not power down\n");
		query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
					 SEM_DBUS_INTERFACE,SEM_DBUS_DOWNLOAD_CPY_CONFIG_SLOT);

		dbus_error_init(&err);

		dbus_message_append_args(query,
								DBUS_TYPE_UINT32, &slot_id,
								DBUS_TYPE_UINT32, &HostSlotId,					
								DBUS_TYPE_STRING, &filename_str,
								DBUS_TYPE_INVALID);
		reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[slot_id] -> dcli_dbus_connection, query, 200000, &err);
		
		dbus_message_unref(query);

		if (NULL == reply)
		{
			vty_out(vty,"<error> failed get reply.\n");
			
			if (dbus_error_is_set(&err)) 
			{
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			
			return CMD_WARNING;
		}

		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&ret);
		if(ret==1)
		{
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&err_str);
			vty_out(vty,"%s",err_str);
			dbus_message_unref(reply);
			return CMD_WARNING;
		}
		else
		{
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&opt);
			dbus_message_iter_next(&iter);	
			dbus_message_unref(reply);
		
			if (-1 == opt)	
			{  
			   vty_out(vty,"system error!"); 
			   return CMD_WARNING;
			}  
			else  
			{  
			   if (WIFEXITED(opt))	
			   {  
				   switch (WEXITSTATUS(opt))
				   {	
						case 0: 		 	   
							   break;
						default:			
							vty_out(vty,"copy configure file error\n");		
							return CMD_WARNING;
					}	
			   }  
			   else  
			   {  
				   vty_out(vty,"exit status = [%d]\n", WEXITSTATUS(opt));
				   return CMD_WARNING;
			   }  
			} 
		}
	   
	}
	else
	{
		vty_out(vty,"The slot doesn't exist");	
		return CMD_WARNING;
	}
		
	
	dcli_send_dbus_signal("download_config_success","download config success");
	memset(filename_temp,0,sizeof(filename_temp));
	if(5==argc)
	{
		strcpy(filename_temp,argv[4]);
		if(0!=strcmp(filename,filename_temp))
		{
			sprintf(cmd,"mv /mnt/%s /mnt/%s >/dev/null\n",filename,filename_temp);
			ret = system(cmd);
			ret = WEXITSTATUS(ret);
			if(0!=ret)
			{
				vty_out(vty,"COPY ERROR\n");
				dcli_send_dbus_signal("update_config_failure","Copy to flash ERROR!");
				return CMD_WARNING;
			}
		}
	}
	else
	{
		strcpy(filename_temp,filename);
	}
		
	if(sor_exec(vty,"cp",filename_temp,60)!=CMD_SUCCESS)
	{
		vty_out(vty,"Download configure file wrong\n");
		dcli_send_dbus_signal("update_config_failure","Copy to flash ERROR!");
		
		return CMD_WARNING;
	}

	if(4==argc&&(0==access("/mnt/backup_config.conf",F_OK)))
	{
		memset(cmd,0,256);
		sprintf(cmd,"mv /mnt/backup_config.conf /mnt/%s >/dev/null",filename);
		ret = system(cmd);
		ret = WEXITSTATUS(ret);
		if(0!=ret)
		{
			vty_out(vty,"COPY ERROR\n");
			return CMD_WARNING;
		}
	}
	update_conf_flag_write();
	dcli_send_dbus_signal("update_config_success","update config success");
	
	vty_out(vty,"System has writen configure file successly,please reboot system\n");
	return CMD_SUCCESS;
}

DEFUN (upload_system_config_func,
	   upload_system_config_cmd,
	   "upload configure ftp SERVER USERNAME PASSWORD FILENAME [FILENAME]",
	   "Upload system infomation\n"
	   "Upload system configure\n"
	   "Use ftp protocol\n" 
	   "Ftp server\n"
	   "User name\n"
	   "Password\n"
	   "The file name on server\n"
	   "The file name you want upload\n")
{
	char cmd[512];
	char* filename=argv[3];
	char* temp_file=argv[4];
	char temp[512];
	int ret=CMD_SUCCESS;
	int result = 1;
	memset(cmd,0,512);
	memset(temp,0,512);
	
	char ipaddr[32] = {0};
	char if_name[32]= {0};
	int send_slot = 0;
	ret = get_ipaddr_and_slot_by_server(argv[0],ipaddr,if_name,&send_slot);/*zhaocg add for pfm*/
	if(ret != 0)
	{
		vty_out(vty,"The URL is wrong,please check it\n");
		return CMD_WARNING;
	}
	if(strlen(argv[3])>VTYSH_MAX_FILE_LEN)
	{
			vty_out(vty,"The config file name you saved was too long!(must be less than %d)\n",VTYSH_MAX_FILE_LEN);
			return CMD_WARNING;
		
	}
	if(5==argc)
	{
		if(strlen(argv[4])>VTYSH_MAX_FILE_LEN)
		{
				vty_out(vty,"The config file name was too long!(must be less than %d)\n",VTYSH_MAX_FILE_LEN);
				return CMD_WARNING;
			
		}
		if(!((temp_file=strstr(temp_file,".conf"))&&!(*(temp_file+5))))//if temp does not has ".conf" and ".conf" in the end then..
		{
			vty_out(vty,"The config file name does not end with \".conf\".Please end with \".conf\".\n");
			return CMD_WARNING;
		}
		sprintf(temp,"/mnt/%s",argv[4]);
		if(0!=access(temp,F_OK)||0==strcmp(argv[4],"conf_xml.conf")||0==strcmp(argv[4],"cli.conf"))
		{
			vty_out(vty,"WARNING: FILE NAME ERROR\n");
			return CMD_WARNING;
		}
		sprintf(cmd,"ftpupload.sh %s %s %s %s %s\n",argv[0],argv[1],argv[2],temp,filename);
	}else{
		sprintf(cmd,"ftpupload.sh %s %s %s %s %s\n",argv[0],argv[1],argv[2],"/mnt/conf_xml.conf",filename);
		}

#if 1 //added by houxx		
		result = pfm_download_config(0,send_slot,ipaddr,if_name);
		if(result != 0)
		{
			vty_out(vty,"config_pfm_failed");
			return CMD_WARNING;
		}
#endif	
		ret = system (cmd);

#if 1 //added by houxx		
		result = pfm_download_config(1,send_slot,ipaddr,if_name);
		if(result != 0)
		{
			vty_out(vty,"config_pfm_failed");
			return CMD_WARNING;
		}
#endif 		
		ret = WEXITSTATUS(ret);
		if(!ret)
			vty_out(vty,"DONE(%d)\n",ret);
		else
			return CMD_WARNING;
	return CMD_SUCCESS;
}
DEFUN (upload_system_config_slot_func,
	   upload_system_config_slot_cmd,
	   "upload configure ftp slot <1-15> SERVER USERNAME PASSWORD FILENAME [FILENAME]",
	   "Upload system infomation\n"
	   "Upload system configure\n"
	   "Use ftp protocol\n"
	   "Board slot for interface local mode\n"
	   "Slot id\n"
	   "Ftp server\n"
	   "User name\n"
	   "Password\n"
	   "The file name on server\n"
	   "The file name you want upload\n")
{
	DBusMessage *query = NULL; 
	DBusMessage *reply = NULL;
	DBusMessageIter  iter;
	DBusError err;
	int slot_id;
	char cmd[512]={0};
	char *cmd_str = cmd;
	char* filename=argv[4];
	char* temp_file=argv[5];
	char temp[512] = {0};
	int ret = 0;
	int opt = 0;
	int result = 1;
	memset(cmd,0,512);
	memset(temp,0,512);
	
	slot_id = atoi(argv[0]);
	if(slot_id == HostSlotId)
	{
		vty_out(vty,"Can't config the active master board,please config other board\n");
		return CMD_WARNING;
	}
	
	if(strlen(argv[4])>VTYSH_MAX_FILE_LEN)
	{
			vty_out(vty,"The config file name you saved was too long!(must be less than %d)\n",VTYSH_MAX_FILE_LEN);
			return CMD_WARNING;
		
	}
	if((strlen(argv[1])+strlen(argv[2])+strlen(argv[3])+strlen(argv[4])+strlen(argv[5]))>512)
	{
		vty_out(vty,"arguments is too long\n");
		return CMD_WARNING;
	}
	if(6==argc)
	{
		if(strlen(argv[5])>VTYSH_MAX_FILE_LEN)
		{
				vty_out(vty,"The config file name was too long!(must be less than %d)\n",VTYSH_MAX_FILE_LEN);
				return CMD_WARNING;
			
		}
		if(!((temp_file=strstr(temp_file,".conf"))&&!(*(temp_file+5))))//if temp does not has ".conf" and ".conf" in the end then..
		{
			vty_out(vty,"The config file name does not end with \".conf\".Please end with \".conf\".\n");
			return CMD_WARNING;
		}
		sprintf(temp,"/mnt/%s",argv[5]);
		if(0!=access(temp,F_OK)||0==strcmp(argv[5],"conf_xml.conf")||0==strcmp(argv[5],"cli.conf"))
		{
			vty_out(vty,"WARNING: FILE NAME ERROR\n");
			return CMD_WARNING;
		}
							   	
		sprintf(cmd,"sudo /opt/bin/vtysh -c \'configure terminal\ncopy %d %s to %d %s\'",HostSlotId,temp,slot_id,temp);
	}
	else
	{
		sprintf(cmd,"sudo /opt/bin/vtysh -c \'configure terminal\ncopy %d %s to %d %s\'",HostSlotId,"/mnt/conf_xml.conf",slot_id,"/mnt/conf_xml.conf");
	}
	
	ret = system(cmd);
	if (-1 == ret)	
	{  
	   vty_out(vty,"system error!"); 
	   return CMD_WARNING;
	}  
	else  
	{  
	   if (WIFEXITED(ret))	
	   {  
		   switch (WEXITSTATUS(ret))
		   {	
				case 0: 		 	   
					   break;
				default:			
					vty_out(vty,"copy file error\n");		
					return CMD_WARNING;
			}	
	   }  
	   else  
	   {  
		   vty_out(vty,"exit status = [%d]\n", WEXITSTATUS(opt));
		   return CMD_WARNING;
	   }  
	}
	
	if(NULL != (dbus_connection_dcli[slot_id] -> dcli_dbus_connection))
	{
		memset(cmd,0,512);
		if(argc==6)
			sprintf(cmd,"ftpupload.sh %s %s %s %s %s\n",argv[1],argv[2],argv[3],temp,filename);
		else
			sprintf(cmd,"ftpupload.sh %s %s %s %s %s\n",argv[1],argv[2],argv[3],"/mnt/conf_xml.conf",filename);
		query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
					 SEM_DBUS_INTERFACE,SEM_DBUS_DOWNLOAD_IMG_SLOT);

		dbus_error_init(&err);

		dbus_message_append_args(query,
								DBUS_TYPE_STRING, &cmd_str,
								DBUS_TYPE_INVALID);
		reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[slot_id] -> dcli_dbus_connection, query, 200000, &err);
		
		dbus_message_unref(query);

		if (NULL == reply)
		{
			vty_out(vty,"<error> failed get reply.\n");
			
			if (dbus_error_is_set(&err)) 
			{
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			
			return CMD_WARNING;
		}

		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&ret);
		
		dbus_message_unref(reply);
		if (-1 == ret)	
		{  
		   vty_out(vty,"system error!"); 
		   return CMD_WARNING; 
		}  
		else  
		{  
	  
		   if (WIFEXITED(ret))	
		   {  
			   switch (WEXITSTATUS(ret))
			   {	
					case 0: 		 
						   break;
					default:			
						vty_out(vty,"upload error\n");
						return CMD_WARNING; 	
			   }	
		   }  
		   else  
		   {  
			   vty_out(vty,"exit status = [%d]\n", WEXITSTATUS(ret)); 
			   return CMD_WARNING;
		   }  
		} 
		
		vty_out(vty,"System write file ok\n"); 
	   
	}
	else
	{
		vty_out(vty,"The slot doesn't exist");
		
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
}

/* start - added by zhengbo */
#define SNAPSHOT_PATH "/mnt/snapshot.zip"

DEFUN (upload_system_snapshot_func,
		upload_system_snapshot_ex_cmd,
		"upload snapshot ftp SERVER USERNAME PASSWORD FILENAME slot SLOTID",
		"Upload system infomation\n"
		"Upload system snapshot\n"
		"Use ftp protocol\n" 
		"Ftp server\n"
		"User name\n"
		"Password\n"
		"The file name on server\n"
		"The slot id\n"
		"The slot id num\n")
{
	DBusConnection *slot_dcli_dbus_connection = NULL;
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	char cmd[1024];
	int op_ret, func_ret, i, slot_id;
	char ipaddr[32] = {0};
	char if_name[32]= {0};
	int send_slot = 0;
	op_ret = get_ipaddr_and_slot_by_server(argv[0],ipaddr,if_name,&send_slot);/*zhaocg add for pfm*/
	if(op_ret != 0)
	{
		vty_out(vty,"The URL is wrong,please check it\n");
		return CMD_WARNING;
	}

	if(argc == 4) {
		slot_id = HostSlotId;
		slot_dcli_dbus_connection = dcli_dbus_connection;
	}
	else {
		slot_id = atoi(argv[4]);
		for(i = 1; i < MAX_SLOT; ++i) {
			if(dbus_connection_dcli[i] &&
				(dbus_connection_dcli[i]->dcli_dbus_connection)) {
				if(slot_id == dbus_connection_dcli[i]->slot_id) {
					slot_dcli_dbus_connection = dbus_connection_dcli[i]->dcli_dbus_connection;
					break;
				}
			}
			else {
				continue;
			}
		}

		if(!slot_dcli_dbus_connection) {
			vty_out(vty, "%% no slot %u dbus connection\n", slot_id);
			return CMD_FAILURE;
		}
	}

	if(strlen(argv[3]) > VTYSH_MAX_FILE_LEN) {
		vty_out(vty,
				"%% The config file name you saved was too long! "
				"(must be less than %d)\n", VTYSH_MAX_FILE_LEN);
		return CMD_FAILURE;
	}

	query = dbus_message_new_method_call(SEM_DBUS_BUSNAME,
											SEM_DBUS_OBJPATH,
											SEM_DBUS_INTERFACE,
											SEM_DBUS_UPLOAD_SNAPSHOT);
	dbus_error_init(&err);
	dbus_message_append_args(query, DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(slot_dcli_dbus_connection, query, 200000, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "%% dbus failed get reply in slot %u.\n", slot_id);
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%% %s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}
	if (dbus_message_get_args(reply, &err,
										DBUS_TYPE_INT32, &op_ret,
										DBUS_TYPE_INVALID)) {
		dbus_message_unref(reply);
		if(op_ret == -1) {
			vty_out(vty, "%% command execute failed in slot %d\n", slot_id);
			return CMD_FAILURE;
		}
		else if(op_ret == 1) {
			vty_out(vty, "%% no snapshot in slot %d\n", slot_id);
			return CMD_FAILURE;
		}
		else if(op_ret == 2) {
			vty_out(vty, "%% compress snapshot failed in slot %d\n", slot_id);
			return CMD_FAILURE;
		}
	}
	else {
		vty_out(vty, "%% dbus failed get arg in slot %d.\n", slot_id);
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%% %s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return CMD_FAILURE;
	}

	if(slot_id != HostSlotId) {
	    char res_md5[PATH_LEN] = {0};
		op_ret = dcli_bsd_copy_file_to_board(slot_dcli_dbus_connection,
												HostSlotId, SNAPSHOT_PATH,
												SNAPSHOT_PATH, 0,
												BSD_TYPE_SINGLE, res_md5);
		vty_out(vty,"File md5 value on dest board is %s\n", (char*)res_md5);
		if(op_ret) {
			vty_out(vty, "%% copy snapshot from slot %d failed: %d.\n", slot_id, op_ret);
			return CMD_FAILURE;
		}
	}

	sprintf(cmd, "ftpupload.sh %s %s %s %s %s\n",
			argv[0], argv[1], argv[2], SNAPSHOT_PATH, argv[3]);

#if 1
	if(pfm_download_config(0,send_slot,ipaddr,if_name) != 0) {
		vty_out(vty, "%% config pfm failed.");
		goto failed;
	}
#endif
	system(cmd);
#if 1
	if(pfm_download_config(1,send_slot,ipaddr,if_name) != 0) {
		vty_out(vty, "%% config pfm failed.");
		goto failed;
	}
#endif

	func_ret = CMD_SUCCESS;

cleanup:
	sprintf(cmd, "rm -rf %s", SNAPSHOT_PATH);
	system(cmd);
	return func_ret;

failed:
	func_ret = CMD_FAILURE;
	goto cleanup;
}
DEFUN (upload_system_snapshot_slot_func,
		upload_system_snapshot_slot_cmd,
		"upload snapshot ftp slot <1-15> SERVER USERNAME PASSWORD FILENAME slot SLOTID",
		"Upload system infomation\n"
		"Upload system snapshot\n"
		"Use ftp protocol\n" 
		"Board slot for interface local mode\n"
	   	"Slot id\n"
		"Ftp server\n"
		"User name\n"
		"Password\n"
		"The file name on server\n"
		"The slot id\n"
		"The slot id num\n")
{
	DBusConnection *slot_dcli_dbus_connection = NULL;
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusMessageIter  iter;
	DBusError err ;
	char cmd[1024]={0};
	char *cmd_str = cmd;
	int op_ret=0;
	int  func_ret = 0;
	int opt = 0;
	int ret = 0;
	int i = 0;
	int slot_id = 0;
	int inf_local_id = 0;
	inf_local_id = atoi(argv[0]);
	if(inf_local_id == HostSlotId)
	{
		vty_out(vty,"Can't config the active master board,please config other board\n");
		return CMD_WARNING;
	}
	
	if(argc == 5) {
		slot_id = HostSlotId;
		slot_dcli_dbus_connection = dcli_dbus_connection;
	}
	else 
	{
		slot_id = atoi(argv[5]);
		for(i = 1; i < MAX_SLOT; ++i)
		{
			if(dbus_connection_dcli[i] &&(dbus_connection_dcli[i]->dcli_dbus_connection)) 
			{
				if(slot_id == dbus_connection_dcli[i]->slot_id) 
				{
				slot_dcli_dbus_connection = dbus_connection_dcli[i]->dcli_dbus_connection;
				break;
				}
			}
			else 
			{
				continue;
			}
		}

		if(!slot_dcli_dbus_connection)
		{
			vty_out(vty, "%% no slot %u dbus connection\n", slot_id);
			return CMD_FAILURE;
		}
	}

	if(strlen(argv[4]) > VTYSH_MAX_FILE_LEN) 
	{
		vty_out(vty,
				"%% The config file name you saved was too long! "
				"(must be less than %d)\n", VTYSH_MAX_FILE_LEN);
		return CMD_FAILURE;
	}
	if((strlen(argv[1])+strlen(argv[2])+strlen(argv[3])+strlen(argv[4]))>1024)
	{
		vty_out(vty,"arguments is too long\n");
		return CMD_WARNING;
	}

	query = dbus_message_new_method_call(SEM_DBUS_BUSNAME,
											SEM_DBUS_OBJPATH,
											SEM_DBUS_INTERFACE,
											SEM_DBUS_UPLOAD_SNAPSHOT);
	dbus_error_init(&err);
	dbus_message_append_args(query, DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(slot_dcli_dbus_connection, query, 200000, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		vty_out(vty, "%% dbus failed get reply in slot %u.\n", slot_id);
		if (dbus_error_is_set(&err))
		{
			vty_out(vty, "%% %s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}
	if (dbus_message_get_args(reply, &err,
										DBUS_TYPE_INT32, &op_ret,
										DBUS_TYPE_INVALID)) 
	{
		dbus_message_unref(reply);
		if(op_ret == -1) 
		{
			vty_out(vty, "%% command execute failed in slot %d\n", slot_id);
			return CMD_FAILURE;
		}
		else if(op_ret == 1) 
		{
			vty_out(vty, "%% no snapshot in slot %d\n", slot_id);
			return CMD_FAILURE;
		}
		else if(op_ret == 2) 
		{
			vty_out(vty, "%% compress snapshot failed in slot %d\n", slot_id);
			return CMD_FAILURE;
		}
	}
	else 
	{
		vty_out(vty, "%% dbus failed get arg in slot %d.\n", slot_id);
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%% %s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return CMD_FAILURE;
	}

	if(slot_id != HostSlotId) 
	{
	    char res_md5[PATH_LEN] = {0};
		op_ret = dcli_bsd_copy_file_to_board(slot_dcli_dbus_connection,
												HostSlotId, SNAPSHOT_PATH,
												SNAPSHOT_PATH, 0,
												BSD_TYPE_SINGLE, res_md5);
		vty_out(vty,"File md5 value on dest board is %s\n", (char*)res_md5);
		if(op_ret) {
			vty_out(vty, "%% copy snapshot from slot %d failed: %d.\n", slot_id, op_ret);
			return CMD_FAILURE;
		}
	}


	
	memset(cmd,0,sizeof(cmd));
	sprintf(cmd,"sudo /opt/bin/vtysh -c \'configure terminal\ncopy %d %s to %d %s\'",HostSlotId,SNAPSHOT_PATH,inf_local_id,SNAPSHOT_PATH);
	ret = system(cmd);
	if (-1 == ret)	
	{  
	   vty_out(vty,"system error!"); 
	   return CMD_WARNING;
	}  
	else  
	{  
	   if (WIFEXITED(ret))	
	   {  
		   switch (WEXITSTATUS(ret))
		   {	
				case 0: 		 	   
					   break;
				default:			
					vty_out(vty,"copy file error\n");		
					return CMD_WARNING;
			}	
	   }  
	   else  
	   {  
		   vty_out(vty,"exit status = [%d]\n", WEXITSTATUS(opt));
		   return CMD_WARNING;
	   }  
	}
	if(NULL != (dbus_connection_dcli[inf_local_id] -> dcli_dbus_connection))
	{
		memset(cmd,0,sizeof(cmd));
		sprintf(cmd, "ftpupload.sh %s %s %s %s %s\n",argv[1], argv[2], argv[3], SNAPSHOT_PATH, argv[4]);
	
		query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
					 SEM_DBUS_INTERFACE,SEM_DBUS_DOWNLOAD_IMG_SLOT);

		dbus_error_init(&err);

		dbus_message_append_args(query,
								DBUS_TYPE_STRING, &cmd_str,
								DBUS_TYPE_INVALID);
		reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[inf_local_id] -> dcli_dbus_connection, query, 200000, &err);
		
		dbus_message_unref(query);

		if (NULL == reply)
		{
			vty_out(vty,"<error> failed get reply.\n");
			
			if (dbus_error_is_set(&err)) 
			{
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			
			return CMD_WARNING;
		}

		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&ret);
		
		dbus_message_unref(reply);
		if (-1 == ret)	
		{  
		   vty_out(vty,"system error!"); 
		   return CMD_WARNING; 
		}  
		else  
		{  
	  
		   if (WIFEXITED(ret))	
		   {  
			   switch (WEXITSTATUS(ret))
			   {	
					case 0: 		 
						   break;
					default:			
						vty_out(vty,"upload error\n");
						return CMD_WARNING; 	
			   }	
		   }  
		   else  
		   {  
			   vty_out(vty,"exit status = [%d]\n", WEXITSTATUS(ret)); 
			   return CMD_WARNING;
		   }  
		} 
		
		vty_out(vty,"System write file ok\n"); 
	   
	}

	func_ret = CMD_SUCCESS;

cleanup:
	sprintf(cmd, "rm -rf %s", SNAPSHOT_PATH);
	system(cmd);
	return func_ret;

failed:
	func_ret = CMD_FAILURE;
	goto cleanup;
}

#if 0
DEFUN (upload_system_snapshot_func,
		upload_system_snapshot_cmd,
		"upload snapshot ftp SERVER USERNAME PASSWORD FILENAME",
		"Upload system infomation\n"
		"Upload system snapshot\n"
		"Use ftp protocol\n" 
		"Ftp server\n"
		"User name\n"
		"Password\n"
		"The file name on server\n")
{
	DBusConnection *slot_dcli_dbus_connection = NULL;
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	char cmd[1024];
	int op_ret, func_ret, i, slot_id;

	slot_id = HostSlotId;
	slot_dcli_dbus_connection = dcli_dbus_connection;

	if(strlen(argv[3]) > VTYSH_MAX_FILE_LEN) {
		vty_out(vty,
				"%% The config file name you saved was too long! "
				"(must be less than %d)\n", VTYSH_MAX_FILE_LEN);
		return CMD_FAILURE;
	}

	query = dbus_message_new_method_call(SEM_DBUS_BUSNAME,
											SEM_DBUS_OBJPATH,
											SEM_DBUS_INTERFACE,
											SEM_DBUS_UPLOAD_SNAPSHOT);
	dbus_error_init(&err);
	dbus_message_append_args(query, DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(slot_dcli_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "%% dbus failed get reply in slot %u.\n", slot_id);
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%% %s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}
	if (dbus_message_get_args(reply, &err,
										DBUS_TYPE_INT32, &op_ret,
										DBUS_TYPE_INVALID)) {
		dbus_message_unref(reply);
		if(op_ret == -1) {
			vty_out(vty, "%% command execute failed in slot %d\n", slot_id);
			return CMD_FAILURE;
		}
		else if(op_ret == 1) {
			vty_out(vty, "%% no snapshot in slot %d\n", slot_id);
			return CMD_FAILURE;
		}
		else if(op_ret == 2) {
			vty_out(vty, "%% compress snapshot failed in slot %d\n", slot_id);
			return CMD_FAILURE;
		}
	}
	else {
		vty_out(vty, "%% dbus failed get arg in slot %d.\n", slot_id);
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%% %s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return CMD_FAILURE;
	}

	sprintf(cmd, "ftpupload.sh %s %s %s %s %s\n",
			argv[0], argv[1], argv[2], SNAPSHOT_PATH, argv[3]);

#if 1
	if(pfm_download_config(0) != 0) {
		vty_out(vty, "%% config pfm failed.");
		goto failed;
	}
#endif
	system(cmd);
#if 1
	if(pfm_download_config(1) != 0) {
		vty_out(vty, "%% config pfm failed.");
		goto failed;
	}
#endif

	func_ret = CMD_SUCCESS;

cleanup:
	sprintf(cmd, "rm -rf %s", SNAPSHOT_PATH);
	system(cmd);
	return func_ret;

failed:
	func_ret = CMD_FAILURE;
	goto cleanup;
}
#else
ALIAS(upload_system_snapshot_func,
		upload_system_snapshot_cmd,
		"upload snapshot ftp SERVER USERNAME PASSWORD FILENAME",
		"Upload system infomation\n"
		"Upload system snapshot\n"
		"Use ftp protocol\n" 
		"Ftp server\n"
		"User name\n"
		"Password\n"
		"The file name on server\n"
);
ALIAS(upload_system_snapshot_slot_func,
		upload_system_snapshot_slot_ex_cmd,
		"upload snapshot ftp slot <1-15> SERVER USERNAME PASSWORD FILENAME",
		"Upload system infomation\n"
		"Upload system snapshot\n"
		"Use ftp protocol\n" 
		"Board slot for interface local mode\n"
	   	"Slot id\n"
		"Ftp server\n"
		"User name\n"
		"Password\n"
		"The file name on server\n"
);

#endif

/* end - added by zhengbo */

DEFUN (download_dev_info_func,
	   download_dev_info_cmd,
	   "download devinfo ULR USERNAME PASSWORD",
	   "Download system infomation\n"
	   "Download device infomation\n"
	   "The device infomation file location&name\n"
	   "User name\n"
	   "Password\n")
{
	char cmd[256];
	int ret;
	char ipaddr[32] = {0};
	char if_name[32]= {0};
	int send_slot = 0;
	ret = get_ipaddr_and_slot_by_url(argv[0],ipaddr,if_name,&send_slot);/*zhaocg add for pfm*/
	if(ret != 0)
	{
		vty_out(vty,"The URL is wrong,please check it\n");
		return CMD_WARNING;
	}
	
	char* filename=strrchr(argv[0],'/');
	
	if(!filename)
	{
		vty_out(vty,"The ULR is wrong,pls check it\n");
		return CMD_WARNING;
	}
	filename++;
	if(filename)
	{
		int i=0;
		int result = 1;
		char *p = NULL, *q = NULL;
		
		memset(cmd,0,256);
#if 1 //added by houxx	
		if(((p = strstr(argv[0],"http:")) != NULL) || (q = strstr(argv[0],"ftp:") != NULL))
		{
			result = pfm_download_config(0,send_slot,ipaddr,if_name);
			if(result != 0)
			{
				vty_out(vty,"config_pfm_failed");
				return CMD_WARNING;
			}
		}
#endif

		
		sprintf(cmd,"downimg.sh %s %s %s %s \n",argv[0],argv[1],argv[2],filename);
		ret = system(cmd);
#if 1 //added by houxx				
		if(((p = strstr(argv[0],"http:")) != NULL) || (q = strstr(argv[0],"ftp:") != NULL))
		{
			result = pfm_download_config(1,send_slot,ipaddr,if_name);
			if(result != 0)
			{
				vty_out(vty,"config_pfm_failed");
				return CMD_WARNING;
			}
		}
#endif		
		
		if(!WEXITSTATUS(ret))
		{
			
			if(strcmp(filename,"devinfo"))
			{
				sprintf(cmd,"mv /mnt/%s /mnt/devinfo > ~/down.log 2>&1\n",filename);
				system(cmd);
			}
			vty_out(vty,"System is writing devinfo file,must not power down\n");
#if 0
			if(system("sor.sh cp devinfo 10"))
#else
			if(sor_exec(vty,"cp","devinfo",60)!=CMD_SUCCESS)

#endif
			{
				vty_out(vty,"Download devinfo file wrong\n");
				return CMD_WARNING;
			}
			vty_out(vty,"System has writen devinfo file successly,please reboot system\n");
		}
		else
		{
			vty_out(vty,"Can't download devinfo file successly\n");
			return CMD_WARNING;
		
		}
	}
	
	return CMD_SUCCESS;
}
DEFUN (download_dev_info_slot_func,
	   download_dev_info_slot_cmd,
	   "download devinfo slot <1-15> ULR USERNAME PASSWORD",
	   "Download system infomation\n"
	   "Download device infomation\n"
	   "Board slot for interface local mode\n"
	   "Slot id\n"
	   "The device infomation file location&name\n"
	   "User name\n"
	   "Password\n")
{	
	DBusMessage *query = NULL; 
	DBusMessage *reply = NULL;
	DBusMessageIter  iter;
	DBusError err; 
	char *urlstr = argv[1];
	char *username = argv[2];
	char *password = argv[3];
	char *err_str = NULL;
	char* filename = NULL;
	int slot_id = 0;
	char cmd[512]={0};
	char *cmd_str = cmd;
	int ret = 0;
	int opt =0;
	
	slot_id = atoi(argv[0]);
	if(slot_id == HostSlotId)
	{
		vty_out(vty,"Can't config the active master board,please config other board\n");
		return CMD_WARNING;
	}
	
	filename=strrchr(argv[1],'/');
	if(!filename)
	{
		vty_out(vty,"The ULR is wrong,pls check it\n");
		return CMD_WARNING;
	}
	filename++;
	if(strlen(filename)>VTYSH_MAX_FILE_LEN)
	{
			vty_out(vty,"The file name you saved was too long!(must be less than %d)\n",VTYSH_MAX_FILE_LEN);
			return CMD_WARNING;
		
	}
	if((strlen(argv[1])+strlen(argv[2])+strlen(argv[3])+strlen(filename))>448)
	{
		vty_out(vty,"arguments is too long\n");
		return CMD_WARNING;
	}

	if(NULL != (dbus_connection_dcli[slot_id] -> dcli_dbus_connection))
	{
	
		memset(cmd,0,sizeof(cmd));
		sprintf(cmd,"downimg.sh %s %s %s %s",urlstr,username,password,filename);
		query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
					 SEM_DBUS_INTERFACE,SEM_DBUS_DOWNLOAD_IMG_SLOT);

		dbus_error_init(&err);

		dbus_message_append_args(query,
								DBUS_TYPE_STRING, &cmd_str,
								DBUS_TYPE_INVALID);
		reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[slot_id] -> dcli_dbus_connection, query, 200000, &err);
		
		dbus_message_unref(query);

		if (NULL == reply)
		{
			vty_out(vty,"<error> failed get reply.\n");
			
			if (dbus_error_is_set(&err)) 
			{
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			
			return CMD_WARNING;
		}

		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&ret);
		
		dbus_message_unref(reply);
		if (-1 == ret)	
		{  
		   vty_out(vty,"system error!"); 
		   return CMD_WARNING;
		}  
		else  
		{  
		   if (WIFEXITED(ret))	
		   {  
			   switch (WEXITSTATUS(ret))
			   {	
					case 0: 		 
						   break;
					default:			
						vty_out(vty,"download error\n");
						return CMD_WARNING; 	
			   }	
		   }  
		   else  
		   {  
			   vty_out(vty,"exit status = [%d]\n", WEXITSTATUS(ret)); 
			   return CMD_WARNING;
		   }  
		} 
		
		vty_out(vty,"System is writing devinfo file,must not power down\n");
		memset(cmd,0,512);
		sprintf(cmd,"/mnt/%s",filename);

		query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
					 SEM_DBUS_INTERFACE,SEM_DBUS_DOWNLOAD_CPY_CONFIG_SLOT);

		dbus_error_init(&err);

		dbus_message_append_args(query,
								DBUS_TYPE_UINT32, &slot_id,
								DBUS_TYPE_UINT32, &HostSlotId,					
								DBUS_TYPE_STRING, &cmd_str,
								DBUS_TYPE_INVALID);
		reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[slot_id] -> dcli_dbus_connection, query, 200000, &err);
		
		dbus_message_unref(query);

		if (NULL == reply)
		{
			vty_out(vty,"<error> failed get reply.\n");
			
			if (dbus_error_is_set(&err)) 
			{
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			
			return CMD_WARNING;
		}

		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&ret);
		if(ret==1)
		{
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&err_str);
			vty_out(vty,"%s",err_str);
			dbus_message_unref(reply);
			return CMD_WARNING;
		}
		else
		{
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&opt);
			dbus_message_iter_next(&iter);	
			dbus_message_unref(reply);
		
			if (-1 == opt)	
			{  
			   vty_out(vty,"system error!"); 
			   return CMD_WARNING;
			}  
			else  
			{  
		  
			   if (WIFEXITED(opt))	
			   {  
				   switch (WEXITSTATUS(opt))
				   {	
						case 0: 		   
							   break;
						default:			
							vty_out(vty,"Can't download devinfo file successly\n");			
							return CMD_WARNING;
					}	
			   }  
			   else  
			   {  
				   vty_out(vty,"exit status = [%d]\n", WEXITSTATUS(opt));
				   return CMD_WARNING;
			   }  
			} 
		}
	   
	}
	else
	{
		vty_out(vty,"The slot doesn't exist");
		
		return CMD_WARNING;
	}

	memset(cmd,0,sizeof(cmd));
	if(strcmp(filename,"devinfo"))
	{
		sprintf(cmd,"mv /mnt/%s /mnt/devinfo > ~/down.log 2>&1\n",filename);
		system(cmd);
	}

	if(sor_exec(vty,"cp","devinfo",60)!=CMD_SUCCESS)
	{
		vty_out(vty,"Download devinfo file wrong\n");
		return CMD_WARNING;
	}
	vty_out(vty,"System has writen devinfo file successly,please reboot system\n");

	return CMD_SUCCESS;
}

DEFUN (download_cvm_rate_config_func,
	   download_cvm_rate_config_cmd,
	   "download traffic-policer-config ULR USERNAME PASSWORD",
	   "Download system infomation\n"
	   "Download traffic-policer config file\n"
	   "The traffic-policer config file location&name\n"
	   "User name\n"
	   "Password\n")
{

#define CMD_LINE_LEN 256
#define FILE_NAME_LEN 256
#define CVM_RATE_CONFIG_FILE "traffic_policer_rule.conf"
	unsigned char cmd[CMD_LINE_LEN] = {0};
	unsigned char* filename = NULL;
	unsigned char  oldfilename[FILE_NAME_LEN] = {0};
	int ret = 0;
	struct timeval timer_now = {0};
	char *p = NULL, *q = NULL;
	int result = 1;
	char ipaddr[32] = {0};
	char if_name[32]= {0};
	int send_slot = 0;
	if(3 != argc)
	{
		vty_out(vty, "Bad parameter number!\n");
		return CMD_WARNING;
	}
	
	ret = get_ipaddr_and_slot_by_url(argv[0],ipaddr,if_name,&send_slot);/*zhaocg add for pfm*/
	if(ret != 0)
	{
		vty_out(vty,"The URL is wrong,please check it\n");
		return CMD_WARNING;
	}
	filename = strrchr(argv[0],'/');
	
	if(!filename)
	{
		vty_out(vty,"The ULR is wrong,please check it\n");
		return CMD_WARNING;
	}
	
	filename = filename + 1;
	
	gettimeofday(&timer_now, 0);
	memset(cmd,0,CMD_LINE_LEN);
	sprintf(oldfilename, "%s_%d", CVM_RATE_CONFIG_FILE, timer_now.tv_sec);
	sprintf(cmd,"mv /mnt/"CVM_RATE_CONFIG_FILE" /mnt/%s 2>/dev/null \n", oldfilename);
	ret = system(cmd);	
	if(!WEXITSTATUS(ret)){
		if(sor_exec(vty,"cp",oldfilename,30)!=CMD_SUCCESS)
		{
			vty_out(vty,"%% Save old config file failed !\n");
		}
	}
	
#if 1		
	if(((p = strstr(argv[0],"http:")) != NULL) || (q = strstr(argv[0],"ftp:") != NULL))
	{
		result = pfm_download_config(0,send_slot,ipaddr,if_name);
		if(result != 0)
		{
			vty_out(vty,"config_pfm_failed");
			return CMD_WARNING;
		}
	}
#endif
	memset(cmd,0,CMD_LINE_LEN);
	sprintf(cmd,"downimg.sh %s %s %s %s \n",argv[0],argv[1],argv[2],filename);
	ret = system(cmd);
	
#if 1 //added by houxx				
	if(((p = strstr(argv[0],"http:")) != NULL) || (q = strstr(argv[0],"ftp:") != NULL))
	{
		result = pfm_download_config(1,send_slot,ipaddr,if_name);
		if(result != 0)
		{
			vty_out(vty,"config_pfm_failed");
			return CMD_WARNING;
		}
	}
#endif		
	if(!WEXITSTATUS(ret)){
		if(strcmp(filename,CVM_RATE_CONFIG_FILE)){
			memset(cmd,0,CMD_LINE_LEN);
			sprintf(cmd,"mv /mnt/%s /mnt/%s > ~/down.log 2>&1\n",filename, CVM_RATE_CONFIG_FILE);
			system(cmd);
		}
		vty_out(vty,"System is writing traffic policer config file,must not power down\n");
			
		if(sor_exec(vty,"cp",CVM_RATE_CONFIG_FILE,30)!=CMD_SUCCESS)
		{
			vty_out(vty,"%% Download traffic policer config file wrong !\n");
			return CMD_WARNING;
		}
		vty_out(vty,"System has writen traffic policer config file successly, you can load the config now.\n");
	}
	else
	{
		vty_out(vty,"%% Can't download traffic policer config file !\n");
		return CMD_WARNING;
	
	}	
	
	return CMD_SUCCESS;
}
DEFUN (download_cvm_rate_config_slot_func,
	   download_cvm_rate_config_slot_cmd,
	   "download traffic-policer-config slot <1-15> ULR USERNAME PASSWORD",
	   "Download system infomation\n"
	   "Download traffic-policer config file\n"
	   "Board slot for interface local mode\n"
	   "Slot id\n"
	   "The traffic-policer config file location&name\n"
	   "User name\n"
	   "Password\n")
{

#define CMD_LINE_LEN 512
#define FILE_NAME_LEN 256
#define CVM_RATE_CONFIG_FILE "traffic_policer_rule.conf"
	DBusMessage *query = NULL; 
	DBusMessage *reply = NULL;
	DBusMessageIter  iter;
	DBusError err; 
	char *urlstr = argv[1];
	char *username = argv[2];
	char *password = argv[3];
	char *err_str = NULL;
	int slot_id = 0;

	char cmd[CMD_LINE_LEN] = {0};
	char *cmd_str = cmd;
	char* filename = NULL;
	char  oldfilename[FILE_NAME_LEN] = {0};
	int ret = 0;
	int opt =0;
	struct timeval timer_now = {0};
	
	if(4 != argc)
	{
		vty_out(vty, "Bad parameter number!\n");
		return CMD_WARNING;
	}
	
	slot_id = atoi(argv[0]);
	if(slot_id == HostSlotId)
	{
		vty_out(vty,"Can't config the active master board,please config other board\n");
		return CMD_WARNING;
	}
	
	filename = strrchr(argv[1],'/');
	if(!filename)
	{
		vty_out(vty,"The ULR is wrong,please check it\n");
		return CMD_WARNING;
	}
	
	filename = filename + 1;
	if(strlen(filename)>VTYSH_MAX_FILE_LEN)
	{
			vty_out(vty,"The file name you saved was too long!(must be less than %d)\n",VTYSH_MAX_FILE_LEN);
			return CMD_WARNING;
		
	}
	if((strlen(argv[1])+strlen(argv[2])+strlen(argv[3])+strlen(filename))>448)
	{
		vty_out(vty,"arguments is too long\n");
		return CMD_WARNING;
	}
	gettimeofday(&timer_now, 0);
	memset(cmd,0,CMD_LINE_LEN);
	sprintf(oldfilename, "%s_%d", CVM_RATE_CONFIG_FILE, timer_now.tv_sec);
	sprintf(cmd,"mv /mnt/"CVM_RATE_CONFIG_FILE" /mnt/%s 2>/dev/null \n", oldfilename);
	ret = system(cmd);	
	if(!WEXITSTATUS(ret))
	{
		if(sor_exec(vty,"cp",oldfilename,30)!=CMD_SUCCESS)
		{
			vty_out(vty,"%% Save old config file failed !\n");
		}
	}
	
	if(NULL != (dbus_connection_dcli[slot_id] -> dcli_dbus_connection))
	{
	
		memset(cmd,0,CMD_LINE_LEN);
		sprintf(cmd,"downimg.sh %s %s %s %s",urlstr,username,password,filename);
		query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
					 SEM_DBUS_INTERFACE,SEM_DBUS_DOWNLOAD_IMG_SLOT);

		dbus_error_init(&err);

		dbus_message_append_args(query,
								DBUS_TYPE_STRING, &cmd_str,
								DBUS_TYPE_INVALID);
		reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[slot_id] -> dcli_dbus_connection, query, 200000, &err);
		
		dbus_message_unref(query);

		if (NULL == reply)
		{
			vty_out(vty,"<error> failed get reply.\n");
			
			if (dbus_error_is_set(&err)) 
			{
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			
			return CMD_WARNING;
		}

		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&ret);
		
		dbus_message_unref(reply);
		if (-1 == ret)	
		{  
		   vty_out(vty,"system error!");
		   return CMD_WARNING;
		}  
		else  
		{  
	  
		   if (WIFEXITED(ret))	
		   {  
			   switch (WEXITSTATUS(ret))
			   {	
					case 0: 		 
						   break;
					default:			
						vty_out(vty,"download error\n");
						return CMD_WARNING; 	
			   }	
		   }  
		   else  
		   {  
			   vty_out(vty,"exit status = [%d]\n", WEXITSTATUS(ret)); 
			   return CMD_WARNING;
		   }  
		} 
		
		vty_out(vty,"System is writing traffic policer config file,must not power down\n");
		memset(cmd,0,CMD_LINE_LEN);
		sprintf(cmd,"/mnt/%s",filename);

		query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
					 SEM_DBUS_INTERFACE,SEM_DBUS_DOWNLOAD_CPY_CONFIG_SLOT);

		dbus_error_init(&err);

		dbus_message_append_args(query,
								DBUS_TYPE_UINT32, &slot_id,
								DBUS_TYPE_UINT32, &HostSlotId,					
								DBUS_TYPE_STRING, &cmd_str,
								DBUS_TYPE_INVALID);
		reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[slot_id] -> dcli_dbus_connection, query, 200000, &err);
		
		dbus_message_unref(query);

		if (NULL == reply)
		{
			vty_out(vty,"<error> failed get reply.\n");
			
			if (dbus_error_is_set(&err)) 
			{
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			
			return CMD_WARNING;
		}

		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&ret);
		if(ret==1)
		{
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&err_str);
			vty_out(vty,"%s",err_str);
			dbus_message_unref(reply);
			return CMD_WARNING;
		}
		else
		{
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&opt);
			dbus_message_iter_next(&iter);	
			dbus_message_unref(reply);
		
			if (-1 == opt)	
			{  
			   vty_out(vty,"system error!"); 
			   return CMD_WARNING;
			}  
			else  
			{  
		  
			   if (WIFEXITED(opt))	
			   {  
				   switch (WEXITSTATUS(opt))
				   {	
						case 0: 		 	   
							   break;
						default:			
							vty_out(vty,"%% Can't download traffic policer config file !\n");		
							return CMD_WARNING;
					}	
			   }  
			   else  
			   {  
				   vty_out(vty,"exit status = [%d]\n", WEXITSTATUS(opt));
				   return CMD_WARNING;
			   }  
			} 
		}
	   
	}
	else
	{
		vty_out(vty,"The slot doesn't exist");
		
		return CMD_WARNING;
	}

	if(strcmp(filename,CVM_RATE_CONFIG_FILE))
	{
		memset(cmd,0,CMD_LINE_LEN);
		sprintf(cmd,"mv /mnt/%s /mnt/%s > ~/down.log 2>&1\n",filename, CVM_RATE_CONFIG_FILE);
		system(cmd);
	}
		
	if(sor_exec(vty,"cp",CVM_RATE_CONFIG_FILE,30)!=CMD_SUCCESS)
	{
		vty_out(vty,"%% Download traffic policer config file wrong !\n");
		return CMD_WARNING;
	}
	vty_out(vty,"System has writen traffic policer config file successly, you can load the config now.\n");
	
	return CMD_SUCCESS;
}

DEFUN (download_web_logo_func,
	   download_web_logo_cmd,
	   "download weblogo URL USERNAME PASSWORD",
	   "Download system infomation\n"
	   "Download web logo file\n"
	   "The web logo file location&name\n"
	   "User name\n"
	   "Password\n")
{
	char cmd[256];
	int ret;
	char ipaddr[32] = {0};
	char if_name[32]= {0};
	int send_slot = 0;
	ret = get_ipaddr_and_slot_by_url(argv[0],ipaddr,if_name,&send_slot);/*zhaocg add for pfm*/
	if(ret != 0)
	{
		vty_out(vty,"The URL is wrong,please check it\n");
		return CMD_WARNING;
	}
	char* filename=strrchr(argv[0],'/');
	if(!filename)
	{
		vty_out(vty,"The URL is wrong,pls check it\n");
		return CMD_WARNING;
	}
	filename++;
	if(filename)
	{
		int i=0;
		int result = 1;
		char *p = NULL, *q = NULL;

		memset(cmd,0,256);

#if 1 //added by houxx				
		if(((p = strstr(argv[0],"http:")) != NULL) || (q = strstr(argv[0],"ftp:") != NULL))
		{
			result = pfm_download_config(0,send_slot,ipaddr,if_name);
			if(result != 0)
			{
				vty_out(vty,"config_pfm_failed");
				return CMD_WARNING;
			}
		}
#endif		
		
		sprintf(cmd,"downimg.sh %s %s %s %s \n",argv[0],argv[1],argv[2],filename);
		ret = system(cmd);
#if 1 //added by houxx				
		if(((p = strstr(argv[0],"http:")) != NULL) || (q = strstr(argv[0],"ftp:") != NULL))
		{
			result = pfm_download_config(1,send_slot,ipaddr,if_name);
			if(result != 0)
			{
				vty_out(vty,"config_pfm_failed");
				return CMD_WARNING;
			}
		}
#endif		
		
		if(!WEXITSTATUS(ret)){
				sprintf(cmd,"[ -d /mnt/logo ]|| mkdir /mnt/logo \n mv /mnt/%s /mnt/logo/logo.jpg > ~/down.log 2>&1\n",filename);
				system(cmd);
				vty_out(vty,"System is writing web log file,must not power down\n");
#if 0
				if(system("sor.sh cp logo/logo.jpg 10"))
#else
				if(sor_exec(vty,"cp","logo/logo.jpg",60)!=CMD_SUCCESS)

#endif
				{
					vty_out(vty,"Download web log file wrong\n");
					return CMD_WARNING;
				}

				vty_out(vty,"System has writen web log file successly,please reboot system\n");
			}
		else
		{
			vty_out(vty,"Can't download weblogo file successly\n");
			return CMD_WARNING;
		
		}
	}
	return CMD_SUCCESS;
}
DEFUN (download_web_logo_slot_func,
	   download_web_logo_slot_cmd,
	   "download weblogo slot <1-15> URL USERNAME PASSWORD",
	   "Download system infomation\n"
	   "Download web logo file\n"
	   "Board slot for interface local mode\n"
	   "Slot id\n"
	   "The web logo file location&name\n"
	   "User name\n"
	   "Password\n")
{
	DBusMessage *query = NULL; 
	DBusMessage *reply = NULL;
	DBusMessageIter  iter;
	DBusError err; 
	char *urlstr = argv[1];
	char *username = argv[2];
	char *password = argv[3];
	char *err_str = NULL;
	char* filename = NULL;
	char cmd[512] = {0};
	char *cmd_str = cmd;
	int slot_id = 0;
	int ret = 0;
	int opt =0;
	
	slot_id = atoi(argv[0]);
	if(slot_id == HostSlotId)
	{
		vty_out(vty,"Can't config the active master board,please config other board\n");
		return CMD_WARNING;
	}
	
 	filename=strrchr(argv[1],'/');
	if(!filename)
	{
		vty_out(vty,"The URL is wrong,pls check it\n");
		return CMD_WARNING;
	}
	filename++;
	if(strlen(filename)>VTYSH_MAX_FILE_LEN)
	{
			vty_out(vty,"The file name you saved was too long!(must be less than %d)\n",VTYSH_MAX_FILE_LEN);
			return CMD_WARNING;
		
	}
	if((strlen(argv[1])+strlen(argv[2])+strlen(argv[3])+strlen(filename))>448)
	{
		vty_out(vty,"arguments is too long\n");
		return CMD_WARNING;
	}
	if(NULL != (dbus_connection_dcli[slot_id] -> dcli_dbus_connection))
	{
	
		memset(cmd,0,sizeof(cmd));
		sprintf(cmd,"downimg.sh %s %s %s %s",urlstr,username,password,filename);
		query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
					 SEM_DBUS_INTERFACE,SEM_DBUS_DOWNLOAD_IMG_SLOT);

		dbus_error_init(&err);

		dbus_message_append_args(query,
								DBUS_TYPE_STRING, &cmd_str,
								DBUS_TYPE_INVALID);
		reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[slot_id] -> dcli_dbus_connection, query, 200000, &err);
		
		dbus_message_unref(query);

		if (NULL == reply)
		{
			vty_out(vty,"<error> failed get reply.\n");
			
			if (dbus_error_is_set(&err)) 
			{
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_WARNING;
		}

		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&ret);
		dbus_message_unref(reply);
		if (-1 == ret)	
		{  
		   vty_out(vty,"system error!");
		   return CMD_WARNING;
		}  
		else  
		{  
	  
		   if (WIFEXITED(ret))	
		   {  
			   switch (WEXITSTATUS(ret))
			   {	
					case 0: 		 
						   break;
					default:			
						vty_out(vty,"download error\n");
						return CMD_WARNING; 	
			   }	
		   }  
		   else  
		   {  
			   vty_out(vty,"exit status = [%d]\n", WEXITSTATUS(ret)); 
			   return CMD_WARNING;
		   }  
		} 
		
		vty_out(vty,"System is writing web log file,must not power down\n");
		memset(cmd,0,sizeof(cmd));
		sprintf(cmd,"/mnt/%s",filename);

		query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
					 SEM_DBUS_INTERFACE,SEM_DBUS_DOWNLOAD_CPY_CONFIG_SLOT);

		dbus_error_init(&err);

		dbus_message_append_args(query,
								DBUS_TYPE_UINT32, &slot_id,
								DBUS_TYPE_UINT32, &HostSlotId,					
								DBUS_TYPE_STRING, &cmd_str,
								DBUS_TYPE_INVALID);
		reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[slot_id] -> dcli_dbus_connection, query, 200000, &err);
		
		dbus_message_unref(query);

		if (NULL == reply)
		{
			vty_out(vty,"<error> failed get reply.\n");
			
			if (dbus_error_is_set(&err)) 
			{
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			
			return CMD_WARNING;
		}

		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&ret);
		if(ret==1)
		{
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&err_str);
			vty_out(vty,"%s",err_str);
			dbus_message_unref(reply);
			return CMD_WARNING;
		}
		else
		{
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&opt);
			dbus_message_iter_next(&iter);	
			dbus_message_unref(reply);
	
			if (-1 == opt)	
			{  
			   vty_out(vty,"system error!"); 
			   return CMD_WARNING;
			}  
			else  
			{  
		  
			   if (WIFEXITED(opt))	
			   {  
				   switch (WEXITSTATUS(opt))
				   {	
						case 0: 		 	   
							   break;
						default:			
							vty_out(vty,"Can't download weblogo file successly\n");		
							return CMD_WARNING;
					}	
			   }  
			   else  
			   {  
				   vty_out(vty,"exit status = [%d]\n", WEXITSTATUS(opt));
				   return CMD_WARNING;
			   }  
			} 
		}
	   
	}
	else
	{
		vty_out(vty,"The slot doesn't exist");
		
		return CMD_WARNING;
	}
	
	memset(cmd,0,sizeof(cmd));
	sprintf(cmd,"[ -d /mnt/logo ]|| mkdir /mnt/logo \n mv /mnt/%s /mnt/logo/logo.jpg > ~/down.log 2>&1\n",filename);
	if(system(cmd))
	{
		vty_out(vty,"Modify web log file wrong\n");
		return CMD_WARNING;
	}
	if(sor_exec(vty,"cp","logo/logo.jpg",60)!=CMD_SUCCESS)
	{
		vty_out(vty,"Download web log file wrong\n");
		return CMD_WARNING;
	}

	vty_out(vty,"System has writen web log file successly,please reboot system\n");

	

	return CMD_SUCCESS;
}

DEFUN (download_wtpcompatible_func,
	   download_wtpcompatible_cmd,
	   "download wtpcompatible URL USERNAME PASSWORD",
	   "Download system infomation\n"
	   "Download wtpcompatible infomation\n"
	   "The wtpcompatible file location&name\n"
	   "User name\n"
	   "Password\n")
{
	char cmd[256];
	int ret;
	char ipaddr[32] = {0};
	char if_name[32]= {0};
	int send_slot = 0;
	ret = get_ipaddr_and_slot_by_url(argv[0],ipaddr,if_name,&send_slot);/*zhaocg add for pfm*/
	if(ret != 0)
	{
		vty_out(vty,"The URL is wrong,please check it\n");
		return CMD_WARNING;
	}
	char* filename=strrchr(argv[0],'/');
	if(!filename)
	{
		vty_out(vty,"The URL is wrong,pls check it\n");
		return CMD_WARNING;
	}
	filename++;
	if(filename)
	{
		int i=0;
		int result = 1;
		char *p = NULL, *q = NULL;
		
		memset(cmd,0,256);

#if 1 //added by houxx				
		if(((p = strstr(argv[0],"http:")) != NULL) || (q = strstr(argv[0],"ftp:") != NULL))
		{
			result = pfm_download_config(0,send_slot,ipaddr,if_name);
			if(result != 0)
			{
				vty_out(vty,"config_pfm_failed");
				return CMD_WARNING;
			}
		}
#endif		
		
		sprintf(cmd,"downimg.sh %s %s %s %s \n",argv[0],argv[1],argv[2],filename);
		ret = system(cmd);

#if 1 //added by houxx				
		if(((p = strstr(argv[0],"http:")) != NULL) || (q = strstr(argv[0],"ftp:") != NULL))
		{
			result = pfm_download_config(1,send_slot,ipaddr,if_name);
			if(result != 0)
			{
				vty_out(vty,"config_pfm_failed");
				return CMD_WARNING;
			}
		}
#endif		
		
		if(!WEXITSTATUS(ret)){
				sprintf(cmd,"[ -d /mnt/wtp ] || mkdir /mnt/wtp\n mv /mnt/%s /mnt/wtp/wtpcompatible.xml > ~/down.log 2>&1\n",filename);
				system(cmd);
				vty_out(vty,"System is writing wtpcompatible file,must not power down\n");
#if 0
				if(system("sor.sh cp wtp/wtpcompatible.xml 10"))
#else
				if(sor_exec(vty,"cp","wtp/wtpcompatible.xml",60)!=CMD_SUCCESS)

#endif
				{
					vty_out(vty,"Download web log file wrong\n");
					return CMD_WARNING;
				}
				vty_out(vty,"System has writen wtpcompatible file successly,please reboot system\n");
			}
		else
		{
			vty_out(vty,"Can't download wtpcompatible file successly\n");
			return CMD_WARNING;
		
		}
	}
	
	return CMD_SUCCESS;
}
DEFUN (download_wtpcompatible_slot_func,
	   download_wtpcompatible_slot_cmd,
	   "download wtpcompatible slot <1-15> URL USERNAME PASSWORD",
	   "Download system infomation\n"
	   "Download wtpcompatible infomation\n"
	   "Board slot for interface local mode\n"
	   "Slot id\n"
	   "The wtpcompatible file location&name\n"
	   "User name\n"
	   "Password\n")
{
	DBusMessage *query = NULL; 
	DBusMessage *reply = NULL;
	DBusMessageIter  iter;
	DBusError err; 
	char *urlstr = argv[1];
	char *username = argv[2];
	char *password = argv[3];
	char *err_str = NULL;
	char* filename = NULL;
	int slot_id = 0;
	char cmd[512]={0};
	char *cmd_str = cmd;
	int ret = 0;
	int opt = 0;
	
	slot_id = atoi(argv[0]);
	if(slot_id == HostSlotId)
	{
		vty_out(vty,"Can't config the active master board,please config other board\n");
		return CMD_WARNING;
	}
	filename=strrchr(argv[1],'/');
	if(!filename)
	{
		vty_out(vty,"The URL is wrong,pls check it\n");
		return CMD_WARNING;
	}
	filename++;
	if(strlen(filename)>VTYSH_MAX_FILE_LEN)
	{
			vty_out(vty,"The file name you saved was too long!(must be less than %d)\n",VTYSH_MAX_FILE_LEN);
			return CMD_WARNING;
		
	}
	if((strlen(argv[1])+strlen(argv[2])+strlen(argv[3])+strlen(filename))>448)
	{
		vty_out(vty,"arguments is too long\n");
		return CMD_WARNING;
	}

	if(NULL != (dbus_connection_dcli[slot_id] -> dcli_dbus_connection))
	{
	
		memset(cmd,0,sizeof(cmd));
		sprintf(cmd,"downimg.sh %s %s %s %s",urlstr,username,password,filename);
		query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
					 SEM_DBUS_INTERFACE,SEM_DBUS_DOWNLOAD_IMG_SLOT);

		dbus_error_init(&err);

		dbus_message_append_args(query,
								DBUS_TYPE_STRING, &cmd_str,
								DBUS_TYPE_INVALID);
		reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[slot_id] -> dcli_dbus_connection, query, 200000, &err);
		
		dbus_message_unref(query);

		if (NULL == reply)
		{
			vty_out(vty,"<error> failed get reply.\n");
			
			if (dbus_error_is_set(&err)) 
			{
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			
			return CMD_WARNING;
		}

		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&ret);
		
		dbus_message_unref(reply);
		if (-1 == ret)	
		{  
		   vty_out(vty,"system error!");
		   return CMD_WARNING;
		}  
		else  
		{  
		   if (WIFEXITED(ret))	
		   {  
			   switch (WEXITSTATUS(ret))
			   {	
					case 0: 		 
						   break;
					default:			
						vty_out(vty,"download error\n");
						return CMD_WARNING; 	
			   }	
		   }  
		   else  
		   {  
			   vty_out(vty,"exit status = [%d]\n", WEXITSTATUS(ret)); 
			   return CMD_WARNING;
		   }  
		} 
		
		
		vty_out(vty,"System is writing wtpcompatible file,must not power down\n");		
		memset(cmd,0,sizeof(cmd));
		sprintf(cmd,"/mnt/%s",filename);

		query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
					 SEM_DBUS_INTERFACE,SEM_DBUS_DOWNLOAD_CPY_CONFIG_SLOT);

		dbus_error_init(&err);

		dbus_message_append_args(query,
								DBUS_TYPE_UINT32, &slot_id,
								DBUS_TYPE_UINT32, &HostSlotId,					
								DBUS_TYPE_STRING, &cmd_str,
								DBUS_TYPE_INVALID);
		reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[slot_id] -> dcli_dbus_connection, query, 200000, &err);
		
		dbus_message_unref(query);
		if (NULL == reply)
		{
			vty_out(vty,"<error> failed get reply.\n");
			
			if (dbus_error_is_set(&err)) 
			{
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			
			return CMD_WARNING;
		}

		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&ret);
		if(ret==1)
		{
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&err_str);
			vty_out(vty,"%s",err_str);
			dbus_message_unref(reply);
			return CMD_WARNING;
		}
		else
		{
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&opt);
			dbus_message_iter_next(&iter);	
			dbus_message_unref(reply);
		
			if (-1 == opt)	
			{  
			   vty_out(vty,"system error!"); 
			   return CMD_WARNING;
			}  
			else  
			{  
			   if (WIFEXITED(opt))	
			   {  
				   switch (WEXITSTATUS(opt))
				   {	
						case 0: 		 
							   break;
						default:			
							vty_out(vty,"Can't download wtpcompatible file successly\n");	
							return CMD_WARNING;
					}	
			   }  
			   else  
			   {  
				   vty_out(vty,"exit status = [%d]\n", WEXITSTATUS(opt));
				   return CMD_WARNING;
			   }  
			} 
		}
	   
	}
	else
	{
		vty_out(vty,"The slot doesn't exist");
		
		return CMD_WARNING;
	}

	
	sprintf(cmd,"[ -d /mnt/wtp ] || mkdir /mnt/wtp\n mv /mnt/%s /mnt/wtp/wtpcompatible.xml > ~/down.log 2>&1\n",filename);
	if(system(cmd))
	{
		vty_out(vty,"Modify wtpcompatible file wrong\n");
		return CMD_WARNING;	
	}

	if(sor_exec(vty,"cp","wtp/wtpcompatible.xml",60)!=CMD_SUCCESS)
	{
		vty_out(vty,"Download wtpcompatible file wrong\n");
		return CMD_WARNING;
	}
	vty_out(vty,"System has writen wtpcompatible file successly,please reboot system\n");
	
	return CMD_SUCCESS;
}

DEFUN (download_bootrom_func,
	   download_bootrom_cmd,
	   "download bootrom  URL USERNAME PASSWORD  SLOT_ID",
	   "Download system infomation\n"
	   "Download bootrom infomation\n"
	   "The bootrom file location&name\n"
	   "User name\n"
	   "Password\n"
	   "the slot id number\n")
{
	DBusMessage *query, *reply;
	DBusError err;
	char cmd[256] = {0};	
	int slot_id;
	int slot_count = get_product_info(SEM_SLOT_COUNT_PATH);
	char* filename=strrchr(argv[0],'/');
	int ret;
	char ipaddr[32] = {0};
	char if_name[32]= {0};
	int send_slot = 0;
	ret = get_ipaddr_and_slot_by_url(argv[0],ipaddr,if_name,&send_slot);/*zhaocg add for pfm*/
	if(ret != 0)
	{
		vty_out(vty,"The URL is wrong,please check it\n");
		return CMD_WARNING;
	}

	slot_id = strtoul(argv[3], NULL, 10);
	if(slot_id > slot_count || slot_id <= 0)
	{
        vty_out(vty,"error slot number : %s\n", argv[0]);
		vty_out(vty,"correct slot numbet option: 1~%d\n",slot_count);
		return CMD_WARNING;
	}
	
	if(!filename)
	{
		vty_out(vty,"The URL is wrong,pls check it\n");
		return CMD_WARNING;
	}
	filename++;
    vty_out(vty,"filename=%s\n",filename);
    if(filename)
	{
		int i=0;
		int result = 1;
		
		memset(cmd,0,256);
		
#if 1 //added by houxx				
		if((( strstr(argv[0],"http:")) != NULL) || (strstr(argv[0],"ftp:") != NULL))
		{
			result = pfm_download_config(0,send_slot,ipaddr,if_name);
			if(result != 0)
			{
				vty_out(vty,"config_pfm_failed");
				return CMD_WARNING;
			}
		}
#endif	
		
		sprintf(cmd,"sudo downimg.sh %s %s %s %s \n",argv[0],argv[1],argv[2],filename);
		ret = system(cmd);
#if 1 //added by houxx				
		if((( strstr(argv[0],"http:")) != NULL) || ( strstr(argv[0],"ftp:") != NULL))
		{
			result = pfm_download_config(1,send_slot,ipaddr,if_name);
			if(result != 0)
			{
				vty_out(vty,"config_pfm_failed");
				return CMD_WARNING;
			}
		}
#endif		
		
		if(!WEXITSTATUS(ret)){					
			vty_out(vty,"filename=%s\n",filename);
			
			if(sor_exec(vty,"cp",filename,500)!= CMD_SUCCESS)
			{ 
				vty_out(vty,"Download img file wrong\n");
				return CMD_WARNING;
			}
			vty_out(vty,"Finishing downloading system img file\n");
            char src_path[PATH_LEN] = {0};
			char des_path[PATH_LEN] = {0};
			char res_md5[PATH_LEN] = {0};
			char a_returnString[BSD_COMMAND_BUF_LEN] = {0};
			sprintf(src_path,"/blk/%s",filename);
			sprintf(des_path,"/blk/%s",filename);

          	vty_out(vty, "start writing bootrom to flash SLOT %d *****Just a minute, please\n",slot_id);			
            ret = dcli_bsd_copy_file_to_board(dcli_dbus_connection,slot_id,src_path,des_path,0,BSD_TYPE_BLK, res_md5);
            vty_out(vty,"File md5 value on dest board is %s\n", (char*)res_md5);
	        vty_out(vty,"%s",dcli_bsd_get_return_string(ret,a_returnString));        

			
	        query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
												 SEM_DBUS_INTERFACE, WRITE_BOOT_TO_FLASH);
			
             dbus_error_init(&err);
	         dbus_message_append_args(query,
						DBUS_TYPE_STRING,&filename,								
						DBUS_TYPE_INVALID);
             if (dbus_connection_dcli[slot_id]->dcli_dbus_connection)
             {
                  reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,query,-1, &err);

		          dbus_message_unref(query);
		          if (NULL == reply)
            	 {
			 
                	vty_out(vty,"<error> failed get reply.\n");
                	if (dbus_error_is_set(&err)) 
                	{
                		vty_out(vty,"%s raised: %s",err.name,err.message);
                		dbus_error_free(&err);
                	}
                	return CMD_WARNING;
            	 }
				  if(dbus_message_get_args (reply, &err,
    				DBUS_TYPE_INT32, &ret,
    				DBUS_TYPE_INVALID))
				  {
                        vty_out(vty,"remote %d :",slot_id);
						
						switch ((ret)) {	
							case 0: 
								
								vty_out(vty,"write successfully,please reset the board\n"); 		
								return CMD_SUCCESS; 	
							case 1: 		
								vty_out(vty,"writing uboot fialed\n");			
								break;		
							case 2: 		
								vty_out(vty,"open file failed,Please check the filename, write BOOTROM again\n");			
								break;		
							case 3: 		
								vty_out(vty,"unable to get MTD device\n");			
								break;		
							case 4: 		
								vty_out(vty,"read fiel failed\n");			
								break;		
							case 5: 		
								vty_out(vty,"write file failed\n"); 		
								break;
							case -1:
								vty_out(vty," file check failed\n");
								break;
							default:			
								vty_out(vty,"writing uboot failed\n");			
								break;		
								}	
						    return CMD_WARNING;   					

				  }
				  else
    		     {
                    vty_out(vty,"Failed get args.\n");
                    if (dbus_error_is_set(&err))
                    {
                        printf("%s raised: %s",err.name,err.message);
                    	dbus_error_free_for_dcli(&err);
                   }
        			dbus_message_unref(reply);
                    return CMD_WARNING;
                 }

             }
			 else
			 {
              	vty_out(vty, "no connection to slot %d\n", slot_id);
                return CMD_WARNING;
			 }

         }
		  else
	     {
             vty_out(vty,"Can't download bootrom file successly\n");
			 return CMD_WARNING;

	     }
	}
	else
	{
		vty_out(vty,"The URL is wrong,pls check it\n");
		return CMD_WARNING;
		
	}
	

	return CMD_SUCCESS;
}
DEFUN (download_bootrom_slot_func,
	   download_bootrom_slot_cmd,
	   "download bootrom slot <1-15> URL USERNAME PASSWORD  SLOT_ID",
	   "Download system infomation\n"
	   "Download bootrom infomation\n"
	   "Board slot for interface local mode\n"	  
	   "Slot id\n"
	   "The bootrom file location&name\n"
	   "User name\n"
	   "Password\n"
	   "the slot id number\n")
{
	DBusMessage *query = NULL; 
	DBusMessage *reply = NULL;
	DBusMessageIter  iter;
	DBusError err; 
	char *urlstr = argv[1];
	char *username = argv[2];
	char *password = argv[3];
	char *err_str = NULL;
	char* filename =NULL;
	int inf_local_id = 0;
	char cmd[512]={0};
	char *cmd_str = cmd;
	int ret = 0;
	int opt = 0;
	int slot_id = 0;
	int slot_count = get_product_info(SEM_SLOT_COUNT_PATH);
	
	inf_local_id = atoi(argv[0]);
	if(inf_local_id == HostSlotId)
	{
		vty_out(vty,"Can't config the active master board,please config other board\n");
		return CMD_WARNING;
	}
	
	slot_id = strtoul(argv[4], NULL, 10);
	if(slot_id > slot_count || slot_id <= 0)
	{
        vty_out(vty,"error slot number : %s\n", argv[1]);
		vty_out(vty,"correct slot numbet option: 1~%d\n",slot_count);
		return CMD_WARNING;
	}
	
	filename=strrchr(argv[1],'/');
	if(!filename)
	{
		vty_out(vty,"The URL is wrong,pls check it\n");
		return CMD_WARNING;
	}
	filename++;
	if(strlen(filename)>VTYSH_MAX_FILE_LEN)
	{
			vty_out(vty,"The file name you saved was too long!(must be less than %d)\n",VTYSH_MAX_FILE_LEN);
			return CMD_WARNING;
		
	}
	if((strlen(argv[1])+strlen(argv[2])+strlen(argv[3])+strlen(argv[4])+strlen(filename))>448)
	{
		vty_out(vty,"arguments is too long\n");
		return CMD_WARNING;
	}
	if(NULL != (dbus_connection_dcli[inf_local_id] -> dcli_dbus_connection))
	{
	
		memset(cmd,0,sizeof(cmd));
		sprintf(cmd,"downimg.sh %s %s %s %s",urlstr,username,password,filename);
		query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
					 SEM_DBUS_INTERFACE,SEM_DBUS_DOWNLOAD_IMG_SLOT);

		dbus_error_init(&err);

		dbus_message_append_args(query,
								DBUS_TYPE_STRING, &cmd_str,
								DBUS_TYPE_INVALID);
		reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[inf_local_id] -> dcli_dbus_connection, query, 200000, &err);
		
		dbus_message_unref(query);

		if (NULL == reply)
		{
			vty_out(vty,"<error> failed get reply.\n");
			
			if (dbus_error_is_set(&err)) 
			{
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			
			return CMD_WARNING;
		}

		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&ret);
		
		dbus_message_unref(reply);
		if (-1 == ret)	
		{  
		   vty_out(vty,"system error!");
		   return CMD_WARNING;
		}  
		else  
		{ 
		   if (WIFEXITED(ret))	
		   {  
			   switch (WEXITSTATUS(ret))
			   {	
					case 0: 		 
						   break;
					default:			
						vty_out(vty,"download error\n");
						return CMD_WARNING; 	
			   }	
		   }  
		   else  
		   {  
			   vty_out(vty,"exit status = [%d]\n", WEXITSTATUS(ret)); 
			   return CMD_WARNING;
		   }  
		} 
		
		vty_out(vty,"System is write file,must not power down\n"); 
		memset(cmd,0,512);
		sprintf(cmd,"/mnt/%s",filename);

		query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
					 SEM_DBUS_INTERFACE,SEM_DBUS_DOWNLOAD_CPY_CONFIG_SLOT);

		dbus_error_init(&err);

		dbus_message_append_args(query,
								DBUS_TYPE_UINT32, &inf_local_id,
								DBUS_TYPE_UINT32, &HostSlotId,					
								DBUS_TYPE_STRING, &cmd_str,
								DBUS_TYPE_INVALID);
		reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[slot_id] -> dcli_dbus_connection, query, 200000, &err);
		
		dbus_message_unref(query);

		if (NULL == reply)
		{
			vty_out(vty,"<error> failed get reply.\n");
			
			if (dbus_error_is_set(&err)) 
			{
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			
			return CMD_WARNING;
		}

		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&ret);
		if(ret==1)
		{
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&err_str);
			vty_out(vty,"%s",err_str);
			dbus_message_unref(reply);
			return CMD_WARNING;
		}
		else
		{
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&opt);
			dbus_message_iter_next(&iter);	
			dbus_message_unref(reply);
		
			if (-1 == opt)	
			{  
			   vty_out(vty,"system error!"); 
			   return CMD_WARNING;
			}  
			else  
			{  
			   if (WIFEXITED(opt))	
			   {  
				   switch (WEXITSTATUS(opt))
				   {	
						case 0: 		 	   
							   break;
						default:			
							vty_out(vty,"Can't download bootrom file successly\n");
							return CMD_WARNING;
					}	
			   }  
			   else  
			   {  
				   vty_out(vty,"exit status = [%d]\n", WEXITSTATUS(opt));
				   return CMD_WARNING;
			   }  
			} 
		}
	   
	}
	else
	{
		vty_out(vty,"The slot doesn't exist");
		
		return CMD_WARNING;
	}
					
	vty_out(vty,"filename=%s\n",filename);
	
	if(sor_exec(vty,"cp",filename,500)!= CMD_SUCCESS)
	{ 
		vty_out(vty,"Download img file wrong\n");
		return CMD_WARNING;
	}
	vty_out(vty,"Finishing downloading system img file\n");
    char src_path[PATH_LEN] = {0};
	char des_path[PATH_LEN] = {0};
	char res_md5[PATH_LEN] = {0};
	char a_returnString[BSD_COMMAND_BUF_LEN] = {0};
	sprintf(src_path,"/blk/%s",filename);
	sprintf(des_path,"/blk/%s",filename);

  	vty_out(vty, "start writing bootrom to flash SLOT %d *****Just a minute, please\n",slot_id);			
    ret = dcli_bsd_copy_file_to_board(dcli_dbus_connection,slot_id,src_path,des_path,0,BSD_TYPE_BLK, res_md5);
    vty_out(vty,"File md5 value on dest board is %s\n", (char*)res_md5);
    vty_out(vty,"%s",dcli_bsd_get_return_string(ret,a_returnString));        

	
    query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
										 SEM_DBUS_INTERFACE, WRITE_BOOT_TO_FLASH);
	
     dbus_error_init(&err);
     dbus_message_append_args(query,
				DBUS_TYPE_STRING,&filename,								
				DBUS_TYPE_INVALID);
     if (dbus_connection_dcli[slot_id]->dcli_dbus_connection)
     {
          reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,query,-1, &err);

          dbus_message_unref(query);
          if (NULL == reply)
    	 {
	 
        	vty_out(vty,"<error> failed get reply.\n");
        	if (dbus_error_is_set(&err)) 
        	{
        		vty_out(vty,"%s raised: %s",err.name,err.message);
        		dbus_error_free(&err);
        	}
        	return CMD_WARNING;
    	 }
		  if(dbus_message_get_args (reply, &err,
			DBUS_TYPE_INT32, &ret,
			DBUS_TYPE_INVALID))
		  {
                vty_out(vty,"remote %d :",slot_id);
				
				switch ((ret)) {	
					case 0: 
						
						vty_out(vty,"write successfully,please reset the board\n"); 		
						return CMD_SUCCESS; 	
					case 1: 		
						vty_out(vty,"writing uboot fialed\n");			
						break;		
					case 2: 		
						vty_out(vty,"open file failed,Please check the filename, write BOOTROM again\n");			
						break;		
					case 3: 		
						vty_out(vty,"unable to get MTD device\n");			
						break;		
					case 4: 		
						vty_out(vty,"read fiel failed\n");			
						break;		
					case 5: 		
						vty_out(vty,"write file failed\n"); 		
						break;
					case -1:
						vty_out(vty," file check failed\n");
						break;
					default:			
						vty_out(vty,"writing uboot failed\n");			
						break;		
						}	
				    return CMD_WARNING;   					

		  }
		  else
	     {
            vty_out(vty,"Failed get args.\n");
            if (dbus_error_is_set(&err))
            {
                printf("%s raised: %s",err.name,err.message);
            	dbus_error_free_for_dcli(&err);
           }
			dbus_message_unref(reply);
            return CMD_WARNING;
         }

     }
	 else
	 {
      	vty_out(vty, "no connection to slot %d\n", slot_id);
        return CMD_WARNING;
	 }
	
	return CMD_SUCCESS;
}

DEFUN (reload_startup_cfg_func,
	   reload_startup_cfg_cmd,
	   "reload startup configure",
	   "Reload system infomation\n"
	   "Startup system infotion\n"
	   "Startup system configure\n")
{
	system("srvload.sh");
	return CMD_SUCCESS;
}

DEFUN (md5_img_func, 
       md5_img_func_cmd,
       "md5 WORD",
       "Md5\n"
       "filename\n")
{

	if(strlen(argv[0])>255)
	{
		vty_out(vty,"file name too long\n");
		return CMD_WARNING;
	}
	
	int ret = sor_md5img(vty,argv[0]);
	
	return ret;
}
ALIAS(md5_img_func, 
       md5_config_slot_func_cmd,
       "md5 config CONFIGFILENAME",
       "Md5\n"
       "Md5 config file\n"
       "Config file name\n");


/*added by zhaocg for md5 subcommand*/
ALIAS(md5_img_func, 
      md5_img_default_func_cmd,
      "md5 img IMGFILENAME",
      "Md5\n"
      "Md5 img\n"
      "IMG file name\n");

DEFUN(md5_img_slot_func,
	md5_img_slot_func_cmd,
	"md5 img <1-16> IMGFILENAME",
    "Md5\n"
    "Md5 img\n"
    "Slot number:1-16\n"
	"IMG file name\n"
	)
{
	DBusMessage *query=NULL; 
	DBusMessage *reply=NULL;
	DBusError err;
	char *ret=NULL;
	char *stopstring=NULL;
	char img_name[256]={0};
	char *imgname=img_name;

	
	long i= atoi( argv[0]);

	if(i<1|| i>15)
	{
		vty_out(vty,"slot number <1 - 15> : invalid number\n");
		
		return CMD_WARNING;
	}
	/*Prevent array subscript beyond the bounds */
	if(strlen(argv[1])>255)
	{
		vty_out(vty,"file name too long\n");
		return CMD_WARNING;
	}

	strcpy(img_name,argv[1]);

	query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
			 SEM_DBUS_INTERFACE, SEM_DBUS_MD5_IMG_SLOT);

	dbus_error_init(&err);

	dbus_message_append_args(query,
		DBUS_TYPE_STRING, &imgname,
				DBUS_TYPE_INVALID);

	if(NULL != (dbus_connection_dcli[i] -> dcli_dbus_connection))
	{
		reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[i] -> dcli_dbus_connection, query, 60000, &err);

		if (NULL == reply)
		{
			vty_out(vty,"<error> failed get reply.\n");
			
			if (dbus_error_is_set(&err)) 
			{
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			
			return CMD_WARNING;
		}

		if (dbus_message_get_args (reply, &err,
					DBUS_TYPE_STRING, &ret,DBUS_TYPE_INVALID))
		{
			vty_out(vty,"%s\n", ret);
		}

		dbus_message_unref(reply);
	}

	dbus_message_unref(query);

	return CMD_SUCCESS;
	
}


DEFUN (md5_patch_func, 
	   md5_patch_func_cmd,
	   "md5 patch PATCHNAME",
	   "Md5\n"
	   "Md5 patch\n"
	   "Patch file name\n")
{
	if(strlen(argv[0])>255)
	{
		vty_out(vty,"file name too long\n");
		return CMD_WARNING;
	}

	int ret = sor_md5patch(vty,argv[0]);
	return ret;
}


DEFUN (md5_patch_slot_func, 
	   md5_patch_slot_func_cmd,
	   "md5 patch <1-16> PATCHNAME",
	   "Md5\n"
	   "Md5 patch\n"
       "Slot number:1-16\n"
	   "Patch file name\n")
{

	DBusMessage *query=NULL; 
	DBusMessage *reply=NULL;
	DBusError err;
	char *ret = NULL;
	char *stopstring = NULL;
	char patch_name[256] = {0};
	char *patchname = patch_name;

	long i= atoi( argv[0]);

	if(i<1|| i>15)
	{
		vty_out(vty,"slot number <1 - 15> : invalid number\n");
		return CMD_WARNING;
	}
	
	/*Prevent array subscript beyond the bounds */
	if(strlen(argv[1])>255)
	{
		vty_out(vty,"file name too long\n");
		return CMD_WARNING;
	}

	strcpy(patch_name,argv[1]);

	query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
				 SEM_DBUS_INTERFACE, SEM_DBUS_MD5_PATCH_SLOT);

	dbus_error_init(&err);

	dbus_message_append_args(query,
				DBUS_TYPE_STRING, &patchname,
						DBUS_TYPE_INVALID);

	if(NULL != (dbus_connection_dcli[i] -> dcli_dbus_connection))
	{
		reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[i] -> dcli_dbus_connection, query, -1, &err);

		if (NULL == reply)
		{
			vty_out(vty,"<error> failed get reply.\n");
			
			if (dbus_error_is_set(&err)) 
			{
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			
			return CMD_WARNING;
		}

		if (dbus_message_get_args (reply, &err,
						DBUS_TYPE_STRING, &ret,
							DBUS_TYPE_INVALID))
		{
			vty_out(vty,"%s\n", ret);
		}
		
		dbus_message_unref(reply);
	}
	
	dbus_message_unref(query);

	return CMD_SUCCESS;

}

/*ended by zhaocg for md5 subcommand*/

#if 0
DEFUN (tcpdump_func, 
       tcpdump_func_cmd,
       "tcpdump WORD",
       "Tcpdump\n"
       "Dev name(eth1-1)\n")
{
	if ((strlen(argv[0])) > INTERFACE_NAMSIZ)
	{
		vty_out(vty, "%% Interface name %s is invalid: length exceeds "
				"%d characters%s",
				argv[0], INTERFACE_NAMSIZ, VTY_NEWLINE);
		return CMD_WARNING;
	}
	if(!strncasecmp(argv[0],"eth",3))
	{
		char cmd[64];
		int ret;
		memset(cmd,0,64);
		sprintf(cmd,"sudo tcpdump -i %s",argv[0]);
		ret=system(cmd);
		if(0!=ret)
		{
			vty_out(vty,"Tcpdump error%s",VTY_NEWLINE);
		}
		return CMD_SUCCESS;
	}

}
#endif
DEFUN (tcpdump_moniter_func, 
       tcpdump_moniter_func_cmd,
       "tcpdump monitor .LINE",
       TCPDUMP_STR
       "Tcpdump and print to the screen\n"
       "The options of tcpdump: [ -AbdDefIKlLNOpqRStuUvxX ] [ -B buffer_size ] [ -c count ] \
       [ -C file_size ] [ -G rotate_seconds ] [ -F file ] [ -i interface ] [ -m module ] \
       [ -M secret ] [ -r file ] [ -s snaplen ] [ -T type ] [ -w file ] [ -W filecount ]\
       [ -E spi@ipaddr algo:secret,...	] [ -y datalinktype ] [ -z postrotate-command ] [ -Z user ][ expression ]\n"
	 )
{
	char cmd[256] = {0};
	int ret;
	char* location = NULL;
	location = argv_concat(argv, argc, 0);
	memset(cmd,0,256);

	if(!location)
	{
		vty_out(vty,"Can't get the options please check input!%s",VTY_NEWLINE);
		return CMD_WARNING;
	}	
	if(strlen(location)>239)
	{
		vty_out(vty,"The options is to long!%s",VTY_NEWLINE);
		XFREE(MTYPE_TMP,location);
		return CMD_WARNING;
	}
	
	sprintf(cmd,"sudo tcpdump %s -n",location);
	ret=system(cmd);
	
	if(0!=ret)
	{
		vty_out(vty,SYS_ERROR);
	}
	
	XFREE(MTYPE_TMP,location);
	return CMD_SUCCESS;

}

DEFUN (tcpdump_interface_func, 
       tcpdump_interface_func_cmd,
       "tcpdump monitor interface INTERFACE",
       TCPDUMP_STR
       "Tcpdump and print to the screen\n"
       "Packet capture from an interface\n"
	 	"The interface you want tcpdump\n")
{
	if ((strlen(argv[0])) > INTERFACE_NAMSIZ)
	{
		vty_out(vty, "%% Interface name %s is invalid: length exceeds "
				"%d characters%s",
				argv[0], INTERFACE_NAMSIZ, VTY_NEWLINE);
		return CMD_WARNING;
	}
	if(1)
	{
		char cmd[64];
		int ret;
		memset(cmd,0,64);
		sprintf(cmd,"sudo tcpdump -i %s -n",argv[0]);
		ret=system(cmd);
		if(0!=ret)
		{
			vty_out(vty,SYS_ERROR);
		}
		return CMD_SUCCESS;
	}

}
/*
DEFUN (moniter_port_func, 
       moniter_port_func_cmd,
       "port moniter WORD",
       "Moniter the packet of port\n"
       "Port\n")
{
	if ((strlen(argv[0])) > INTERFACE_NAMSIZ)
	{
		vty_out(vty, "%% Interface name %s is invalid: length exceeds "
				"%d characters%s",
				argv[0], INTERFACE_NAMSIZ, VTY_NEWLINE);
		return CMD_WARNING;
	}
	if(1)
	{
		char cmd[64];
		int ret;
		memset(cmd,0,64);
		sprintf(cmd,"sudo tcpdump -vv -x -e -i %s",argv[0]);
		ret=system(cmd);
		if(0!=ret)
		{
			vty_out(vty,SYS_ERROR);
		}
		return CMD_SUCCESS;
	}

}

*/
DEFUN (tcpdump_save_func, 
       tcpdump_save_func_cmd,
       "tcpdump save INTERFACE .[LINE]",
	   TCPDUMP_STR
		"Save data into file\n"
		"The interface you want \n"
        "The options of tcpdump: [ -AbdDefIKlLNOpqRStuUvxX ] [ -B buffer_size ] [ -c count ][ -C file_size ] \
        [ -G rotate_seconds ] [ -F file ] [ -m module ] [ -M secret ][ -r file ] [ -s snaplen ] [ -T type ] \
        [ -w file ][ -E spi@ipaddr algo:secret,...	][ -y datalinktype ] [ -z postrotate-command ] [ -Z user ][ expression ]\n")
{
	char cmd[256] = {0};
	char file_name[64] = {0};
	time_t timep;
	struct tm *p = NULL;
	int ret;
	char* location = NULL;
	
	if ((strlen(argv[0])) > INTERFACE_NAMSIZ)
	{
		vty_out(vty, "%% Interface name %s is invalid: length exceeds "
				"%d characters%s",
				argv[0], INTERFACE_NAMSIZ, VTY_NEWLINE);
		return CMD_WARNING;
	}
	
	location = argv_concat(argv,argc, 1);
	
	memset(cmd,0,256);

	if(location != NULL&&strlen(location)>232)
	{
		vty_out(vty,"The optons is too long!%s",VTY_NEWLINE);
		
		XFREE(MTYPE_TMP,location);
		return CMD_WARNING;
	}

	time(&timep);
	p=gmtime(&timep);
	sprintf(file_name,"%d%d%d%d%d%d",(1900+p->tm_year),(1+p->tm_mon),p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec);
	
	if(NULL==location)
	{
		sprintf(cmd,"sudo tcpdump -w /opt/debugdown/%s -i %s -n",file_name,argv[0]);
		//vty_out(vty,"%s\n",cmd);
	}
	else
	{
		sprintf(cmd,"sudo tcpdump %s -w /opt/debugdown/%s -i %s -n",location,file_name,argv[0]);
		//vty_out(vty,"%s\n",cmd);
	}
	
	ret=system(cmd);
	if(0!=ret)
	{
		vty_out(vty,SYS_ERROR);
	}
	if(location)
		XFREE(MTYPE_TMP,location);
	return CMD_SUCCESS;

}

ALIAS(tcpdump_save_func,
	tcpdump_save2_func_cmd,
	"tcpdump save INTERFACE",
	TCPDUMP_STR
	"Save data into file\n"
	"The interface you want \n"
	);


DEFUN (show_packet_file_func, 
       show_packet_file_func_cmd,
       "show packet file [FILENAME]",
       SHOW_STR
       "Show packet information\n"
       "show packet file\n"
       "show the data of packet file\n")
{
	
	char cmd[128];
	int ret;
	if(argc==1)
	{
		if(strlen(argv[0])>FILE_NAMESIZ)
		{
			vty_out(vty,"The file name you press too long%s");
			return CMD_SUCCESS;
		}
		memset(cmd,0,128);
		sprintf(cmd,"sudo tcpdump -r /opt/debugdown/%s -xxx -n 2> /dev/null",argv[0]);
		ret=system(cmd);
		if(0!=ret)
		{
			vty_out(vty,"please press right file name%s",VTY_NEWLINE);
		}
		return CMD_SUCCESS;
	}else{
		
		memset(cmd,0,128);
		sprintf(cmd,"cd /opt/debugdown;ls | more");
		ret=system(cmd);  
		return CMD_SUCCESS;
		}
		
}

DEFUN (delete_packet_file_func, 
       delete_packet_file_func_cmd,
       "delete packet file (all|[FILENAME])",
       "Delete system file\n"
       "Delete packet information\n"
       "Delete packet file\n"
	   "Delete all packet file\n"
       "Delete packet file of FILENAME\n")
{
	char cmd[256];
	if(strncmp(argv[0],"all",3) == 0)
	{
		system("rm /opt/debugdown/* > /dev/null 2> /dev/null");
	}else{
		int ret = -1;
		memset(cmd,0,256);
		sprintf(cmd,"rm /opt/debugdown/%s > /dev/null 2> /dev/null",argv[0]);
		ret = system(cmd);
		ret = WEXITSTATUS(ret);
		if(ret == 1)
		{
			vty_out(vty,"not found\n");
			return CMD_WARNING;
		}
	}
	return CMD_SUCCESS;	
}

#define PACKET_FILE "/opt/debugdown/temp_packet_file"
DEFUN (upload_packet_file_func,
	   upload_packet_file_ex_cmd,
	   "upload packet file ftp SERVER USERNAME PASSWORD FILENAME FILENAME slot SLOTID",
	   "Upload system infomation\n"
	   "Upload packet file\n"
	   "Upload packet file\n"
	   "Use ftp serves\n"
	   "Ftp server\n"
	   "User name\n"
	   "Password\n"
	   "The file name on server\n"
	   "The file name you want upload\n")
{
	DBusConnection *slot_dcli_dbus_connection = NULL;
	char cmd[512], spath[512];
	char *path;
	int slot_id, op_ret, func_ret, i;
	char ipaddr[32] = {0};
	char if_name[32]= {0};
	int send_slot = 0;
	op_ret = get_ipaddr_and_slot_by_server(argv[0],ipaddr,if_name,&send_slot);/*zhaocg add for pfm*/
	if(op_ret != 0)
	{
		vty_out(vty,"The URL is wrong,please check it\n");
		return CMD_WARNING;
	}

	if(argc == 5) {
		slot_id = HostSlotId;
		slot_dcli_dbus_connection = dcli_dbus_connection;
	}
	else {
		slot_id = atoi(argv[5]);
		for(i = 1; i < MAX_SLOT; ++i) {
			if(dbus_connection_dcli[i] &&
				(dbus_connection_dcli[i]->dcli_dbus_connection)) {
				if(slot_id == dbus_connection_dcli[i]->slot_id) {
					slot_dcli_dbus_connection = dbus_connection_dcli[i]->dcli_dbus_connection;
					break;
				}
			}
			else {
				continue;
			}
		}

		if(!slot_dcli_dbus_connection) {
			vty_out(vty, "%% no slot %u dbus connection\n", slot_id);
			return CMD_FAILURE;
		}
	}

	if(strlen(argv[3]) > VTYSH_MAX_FILE_LEN) {
		vty_out(vty,
				"%% The config file name you saved was too long!(must be less than %d)\n",
				VTYSH_MAX_FILE_LEN);
		return CMD_FAILURE;
	}

	if(strlen(argv[4]) > VTYSH_MAX_FILE_LEN) {
		vty_out(vty,
				"%% The config file name was too long!(must be less than %d)\n",
				VTYSH_MAX_FILE_LEN);
		return CMD_FAILURE;
	}

	sprintf(spath, "/opt/debugdown/%s", argv[4]);

	if(slot_id != HostSlotId) {
	    char res_md5[PATH_LEN] = {0};
		op_ret = dcli_bsd_copy_file_to_board(slot_dcli_dbus_connection,
												HostSlotId, spath,
												PACKET_FILE, 0,
												BSD_TYPE_SINGLE, res_md5);
		vty_out(vty,"File md5 value on dest board is %s\n", (char*)res_md5);										
		if(op_ret) {
			vty_out(vty, "%% copy snapshot from slot %d failed: %d.\n", slot_id, op_ret);
			return CMD_FAILURE;
		}
		path = PACKET_FILE;
	}
	else {
		path = spath;
	}

	if(access(path, F_OK)) {
		vty_out(vty, "%% %s not exist\n", path);
		return CMD_FAILURE;
	}

	sprintf(cmd, "ftpupload.sh %s %s %s %s %s\n",
				argv[0], argv[1], argv[2], path, argv[3]);
#if 1
	if(pfm_download_config(0,send_slot,ipaddr,if_name) != 0) {
		vty_out(vty, "%% config pfm failed.");
		goto failed;
	}
#endif
	system(cmd);
#if 1
	if(pfm_download_config(1,send_slot,ipaddr,if_name) != 0) {
		vty_out(vty, "%% config pfm failed.");
		goto failed;
	}
#endif

	func_ret = CMD_SUCCESS;
cleanup:
	if(slot_id != HostSlotId) {
		unlink(path);
	}
	return func_ret;

failed:
	func_ret = CMD_FAILURE;
	goto cleanup;
}

ALIAS(upload_packet_file_func,
		upload_packet_file_cmd,
		"upload packet file ftp SERVER USERNAME PASSWORD FILENAME FILENAME",
		"Upload system infomation\n"
		"Upload packet file\n"
		"Upload packet file\n"
		"Use ftp serves\n"
		"Ftp server\n"
		"User name\n"
		"Password\n"
		"The file name on server\n"
		"The file name you want upload\n"
);
DEFUN (upload_packet_file_slot_func,
	   upload_packet_file_slot_cmd,
	   "upload packet file ftp slot <1-15> SERVER USERNAME PASSWORD FILENAME FILENAME slot SLOTID",
	   "Upload system infomation\n"
	   "Upload packet file\n"
	   "Upload packet file\n"
	   "Use ftp serves\n"
	   "Board slot for interface local mode\n"	  
	   "Slot id\n"
	   "Ftp server\n"
	   "User name\n"
	   "Password\n"
	   "The file name on server\n"
	   "The file name you want upload\n")
{
	DBusConnection *slot_dcli_dbus_connection = NULL;
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusMessageIter  iter;
	DBusError err ;
	char cmd[512]={0};
	char *cmd_str = cmd;
	char spath[512] = {0};
	char *path;
	int slot_id = 0;
	int op_ret = 0;
	int func_ret = 0;
	int i =0;
	int ret = 0;
	int opt = 0;
	int inf_local_id = 0;
	inf_local_id = atoi(argv[0]);
	if(inf_local_id == HostSlotId)
	{
		vty_out(vty,"Can't config the active master board,please config other board\n");
		return CMD_WARNING;
	}

	if(argc == 6) {
		slot_id = HostSlotId;
		slot_dcli_dbus_connection = dcli_dbus_connection;
	}
	else {
		slot_id = atoi(argv[6]);
		for(i = 1; i < MAX_SLOT; ++i) {
			if(dbus_connection_dcli[i] &&
				(dbus_connection_dcli[i]->dcli_dbus_connection)) {
				if(slot_id == dbus_connection_dcli[i]->slot_id) {
					slot_dcli_dbus_connection = dbus_connection_dcli[i]->dcli_dbus_connection;
					break;
				}
			}
			else {
				continue;
			}
		}

		if(!slot_dcli_dbus_connection) {
			vty_out(vty, "%% no slot %u dbus connection\n", slot_id);
			return CMD_FAILURE;
		}
	}

	if(strlen(argv[4]) > VTYSH_MAX_FILE_LEN) {
		vty_out(vty,
				"%% The config file name you saved was too long!(must be less than %d)\n",
				VTYSH_MAX_FILE_LEN);
		return CMD_FAILURE;
	}

	if(strlen(argv[5]) > VTYSH_MAX_FILE_LEN) {
		vty_out(vty,
				"%% The config file name was too long!(must be less than %d)\n",
				VTYSH_MAX_FILE_LEN);
		return CMD_FAILURE;
	}
	if((strlen(argv[1])+strlen(argv[2])+strlen(argv[3])+strlen(argv[4])+strlen(argv[5]))>448)
	{
		vty_out(vty,"arguments is too long\n");
		return CMD_WARNING;
	}

	sprintf(spath, "/opt/debugdown/%s", argv[5]);

	if(slot_id != HostSlotId) {
	    char res_md5[PATH_LEN] = {0};
		op_ret = dcli_bsd_copy_file_to_board(slot_dcli_dbus_connection,
												HostSlotId, spath,
												PACKET_FILE, 0,
												BSD_TYPE_SINGLE, res_md5);
		vty_out(vty,"File md5 value on dest board is %s\n", (char*)res_md5);										
		if(op_ret) {
			vty_out(vty, "%% copy snapshot from slot %d failed: %d.\n", slot_id, op_ret);
			return CMD_FAILURE;
		}
		path = PACKET_FILE;
	}
	else {
		path = spath;
	}

	if(access(path, F_OK)) {
		vty_out(vty, "%% %s not exist\n", path);
		return CMD_FAILURE;
	}
	memset(cmd,0,sizeof(cmd));
	sprintf(cmd,"sudo /opt/bin/vtysh -c \'configure terminal\ncopy %d %s to %d %s\'",HostSlotId,path,inf_local_id,path);
	ret = system(cmd);
	if (-1 == ret)	
	{  
	   vty_out(vty,"system error!"); 
	   return CMD_WARNING;
	}  
	else  
	{  
	   if (WIFEXITED(ret))	
	   {  
		   switch (WEXITSTATUS(ret))
		   {	
				case 0: 			   
					   break;
				default:			
					vty_out(vty,"copy file error\n");		
					return CMD_WARNING;
			}	
	   }  
	   else  
	   {  
		   vty_out(vty,"exit status = [%d]\n", WEXITSTATUS(opt));
		   return CMD_WARNING;
	   }  
	}
	if(NULL != (dbus_connection_dcli[inf_local_id] -> dcli_dbus_connection))
	{
		memset(cmd,0,sizeof(cmd));
		sprintf(cmd, "ftpupload.sh %s %s %s %s %s\n",argv[1], argv[2], argv[3], path, argv[4]);
		query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
					 SEM_DBUS_INTERFACE,SEM_DBUS_DOWNLOAD_IMG_SLOT);

		dbus_error_init(&err);

		dbus_message_append_args(query,
								DBUS_TYPE_STRING, &cmd_str,
								DBUS_TYPE_INVALID);
		reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[inf_local_id] -> dcli_dbus_connection, query, 200000, &err);
		
		dbus_message_unref(query);

		if (NULL == reply)
		{
			vty_out(vty,"<error> failed get reply.\n");
			
			if (dbus_error_is_set(&err)) 
			{
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			
			return CMD_WARNING;
		}

		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&ret);
		
		dbus_message_unref(reply);
		if (-1 == ret)	
		{  
		   vty_out(vty,"system error!"); 
		   return CMD_WARNING; 
		}  
		else  
		{  
	  
		   if (WIFEXITED(ret))	
		   {  
			   switch (WEXITSTATUS(ret))
			   {	
					case 0: 		 
						   break;
					default:			
						vty_out(vty,"upload error\n");
						return CMD_WARNING; 	
			   }	
		   }  
		   else  
		   {  
			   vty_out(vty,"exit status = [%d]\n", WEXITSTATUS(ret)); 
			   return CMD_WARNING;
		   }  
		} 
		
		vty_out(vty,"System write file ok\n"); 
	   
	}

	func_ret = CMD_SUCCESS;
cleanup:
	if(slot_id != HostSlotId) {
		unlink(path);
	}
	return func_ret;

failed:
	func_ret = CMD_FAILURE;
	goto cleanup;
}
ALIAS(upload_packet_file_slot_func,
		upload_packet_file_slot_ex_cmd,
		"upload packet file ftp slot <1-15> SERVER USERNAME PASSWORD FILENAME FILENAME",
		"Upload system infomation\n"
		"Upload packet file\n"
		"Upload packet file\n"
		"Use ftp serves\n"
		"Board slot for interface local mode\n"	  
	   	"Slot id\n"
		"Ftp server\n"
		"User name\n"
		"Password\n"
		"The file name on server\n"
		"The file name you want upload\n"
);

#define DK_OF_SSH_PATCH
#ifdef DK_OF_SSH_PATCH

DEFUN(show_patch_func,
	show_patch_func_cmd,
	"show patch",
	SHOW_STR
	"Show patch file which can be used infomation\n"
	)
{
	char cmd[128];
	int ret;
	memset(cmd,0,128);
	//vty_out(vty,"show local board patch:");
	sprintf(cmd,"cd /mnt/patch/;ls -l *.sp 2> /dev/NULL| grep ^[^d] | awk '{print $9}'");
	ret = system (cmd);
	memset(cmd,0,128);
	sprintf(cmd,"cd /mnt/patch/;ls -l *.sps 2> /dev/NULL| grep ^[^d] | awk '{print $9}'");
	ret = system (cmd);
	return CMD_SUCCESS;
}
DEFUN(show_patch_slot_func,
	show_patch_slot_func_cmd,
	"show patch <1-15>",
	SHOW_STR
	"Show patch files which can be used infomation\n"
	"Show patch files which can be used infomation on slot board\n"
	)
{
	DBusMessage *query=NULL; 
	DBusMessage *reply=NULL;
	DBusMessageIter  iter;
	DBusError err;
	char *ret = NULL;
	char **tem = NULL;
	char *cmdstr = "cd /mnt/patch/;ls *[.sp,.sps] 2> /dev/NULL | more 2> /dev/NULL";
	char *cmd = cmdstr;
	int fastfwdnum = 0;
	int i;
	int slot_id = 0;

	slot_id = atoi(argv[0]);
	

	if(NULL != (dbus_connection_dcli[slot_id] -> dcli_dbus_connection))
	{
		query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
					 SEM_DBUS_INTERFACE,SEM_DBUS_IMG_OR_FASTFWD_SLOT);

		dbus_error_init(&err);

		dbus_message_append_args(query,
					DBUS_TYPE_STRING, &cmd,
							DBUS_TYPE_INVALID);
		reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[slot_id] -> dcli_dbus_connection, query, -1, &err);
		
		dbus_message_unref(query);

		if (NULL == reply)
		{
			vty_out(vty,"<error> failed get reply.\n");
			
			if (dbus_error_is_set(&err)) 
			{
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			
			return CMD_WARNING;
		}

		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&fastfwdnum);
		
		if(fastfwdnum != -1)
		{
			for(i=0;i < fastfwdnum;i++)
			{	
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&ret);
				vty_out(vty,"%s", ret);
				
			}
		}
		dbus_message_unref(reply);
	}
	else
	{
		vty_out(vty,"The slot doesn't exist");
	}
	
	return CMD_SUCCESS;

}
	

DEFUN(delete_patch_func,
	delete_patch_func_cmd,
	"delete patch PATCH_NAME",
	"delete system file\n"
	"Show patch file which can be used infomation\n"
	)
{
	char cmd[128];
	int ret;
	memset(cmd,0,128);
	sprintf(cmd,"cd /mnt/patch/;ls *[.sp,.sps] | more 2> /dev/NULL");
	ret = system (cmd);
	return CMD_SUCCESS;
}

DEFUN(ssh_up_func,
	ssh_up_func_cmd,
	"service ssh enable [<1-65535>]",
	SER_STR
	SER_SSH_STR
	"Make ssh service enable\n"
	"Port number\n"
	)
{
	if((is_distributed == 1) && (is_active_master == 0))
	{
		vty_out(vty,"only active master can enable ssh\n");
		return CMD_WARNING;
	}
	FILE *fp;
	struct stat sb;
	u_int32_t port;
	char port_str[10];
	char *temp_data;
	char *data;
	fp=open(SERVICES_PATH,O_RDWR);	
	if(NULL==fp)
	{
		vty_out(vty,SYS_ERROR);
		return CMD_WARNING;
	}
	fstat(fp,&sb);
	data=mmap(NULL,sb.st_size,PROT_READ|PROT_WRITE,MAP_SHARED,fp,0);
	if(MAP_FAILED==data)
	{
		vty_out(vty,SYS_ERROR);
		close(fp);
		return CMD_WARNING;
	}
	if(argc==1)
	{
	//Check the correctness of port
		int i;
		memset(port_str,0,10);
		strncpy(port_str,argv[0],strlen(argv[0]));
		port = atoi(argv[0]);
		for(i=0;i<strlen(argv[0]);i++)
		{
			if((*(argv[0]+i))>'9' || (*(argv[0]+i))<'0')
			{
				vty_out(vty,"Please press right port number%s",VTY_NEWLINE);
				close(fp);
				return CMD_WARNING;
			}
		}
		if(port>65535 || port<0)
		{
			vty_out(vty,"Port number is incorrect%s",VTY_NEWLINE);
			munmap(data,sb.st_size);
			close(fp);
			return CMD_WARNING;
		}
	}else{
		memset(port_str,0,10);
		sprintf(port_str,"%d",22);
		port=22;
		}


	if(1){
	temp_data=strstr(data,port_str);
	if(temp_data && (*(temp_data+(strlen(port_str))))=='/' && (*(temp_data-1))=='\t')//the port was used
	{
		if(strncmp(temp_data-(strlen("ssh")+2),"ssh",strlen("ssh")))
		{
		
			vty_out(vty,"Port is already in use%s",VTY_NEWLINE);
			munmap(data,sb.st_size);
			close(fp);
			return CMD_WARNING;
		}
		
	}
	munmap(data,sb.st_size);
	close(fp);
	}
	if(1) 
	{
		//find port for argv[0] in "/etc/ssh/sshd_config
		fp=open(SSHD_CONFIG_PATCH,O_RDWR);
		if(NULL==fp)
		{
			vty_out(vty,SYS_ERROR);
			return CMD_WARNING;
		}
		fstat(fp,&sb);
		data=mmap(NULL,sb.st_size+10,PROT_READ|PROT_WRITE,MAP_SHARED,fp,0);
		if(MAP_FAILED==data)
		{
			vty_out(vty,SYS_ERROR);
			close(fp);
			return CMD_WARNING;
		}		
		
		//change the ssh port in sshd_config
		for(temp_data=data;temp_data<=(data+sb.st_size);temp_data=strstr(temp_data,"Port"))
		{
			if(temp_data==NULL)
			{
				break;
			}
			if((*(temp_data-1))==10 && (*(temp_data+4))==' ') //is real "Port"
			{
				char cmd[64];
				char* temp;
				temp=strchr(temp_data,'\n');
				
				if(!strstr(data,"used_for_mmap"))
				{
					system("echo \"#used_for_mmap           \" >>/etc/ssh/sshd_config");
				}
				memset(cmd,0,64);
				sprintf(cmd,"Port %d",port);
			//	sprintf(temp_data+strlen(cmd),"%s",temp);
				memmove(temp_data+strlen(cmd),temp,strlen(temp));//change the port
				memcpy(temp_data,cmd,strlen(cmd));
		//		free(temp_mem);
				temp_data++;
				break;
			}
		temp_data++;
		}
		munmap(data,sb.st_size);
		close(fp);
		
		fp=open(SERVICES_PATH,O_RDWR);
		if(NULL==fp)
		{
			vty_out(vty,SYS_ERROR);
			return CMD_WARNING;
		}
		fstat(fp,&sb);
		data=mmap(NULL,sb.st_size+10,PROT_READ|PROT_WRITE,MAP_SHARED,fp,0);
		if(MAP_FAILED==data)
		{
			vty_out(vty,SYS_ERROR);
			close(fp);
			return CMD_WARNING;
		}		
		
		//change the ssh port on services
		for(temp_data=data;temp_data<=(data+sb.st_size);temp_data=strstr(temp_data,"ssh"))
		{
			if(temp_data==NULL)
			{
				break;
			}
			if((*(temp_data-1))==10 && (*(temp_data+3))=='\t') //is real "Port"
			{
				char cmd[64];
				char* temp;
				temp=strchr(temp_data,'/');
				memset(cmd,0,64);
				sprintf(cmd,"ssh\t\t%d",port);
			//	sprintf(temp_data+strlen(cmd),"%s",temp);
				memmove(temp_data+strlen(cmd),temp,strlen(temp));//change the port
				memcpy(temp_data,cmd,strlen(cmd));
		//		free(temp_mem);
				temp_data++;
				continue;
			}
		temp_data++;
		}
	}
	
	munmap(data,sb.st_size);
	close(fp);
	#if 0
	else
	{
		
		fp=open(SSHD_CONFIG_PATCH,O_RDWR);
		if(NULL==fp)
		{
			vty_out(vty,SYS_ERROR);
			return CMD_WARNING;
		}
		fstat(fp,&sb);
		data=mmap(NULL,sb.st_size+10,PROT_READ|PROT_WRITE,MAP_SHARED,fp,0);
		if(MAP_FAILED==data)
		{
			vty_out(vty,SYS_ERROR);
			close(fp);
			return CMD_WARNING;
		}		
		for(temp_data=data;temp_data<=(data+sb.st_size);temp_data=strstr(temp_data,"Port"))
		{
			if(NULL==temp_data)
			{
				break;
			}
			if((*(temp_data-1))==10 && (*(temp_data+4))==' ') //is real "Port"
			{
				char cmd[64];
				char* temp;
				temp=strchr(temp_data,'\n');
				memset(cmd,0,64);
				sprintf(cmd,"Port %s",22);
			//	sprintf(temp_data+strlen(cmd),"%s",temp);
			memcpy(temp_mem,temp,strlen(temp));
				memcpy(temp_data+strlen(cmd),temp_mem,strlen(temp_mem));//change the port
				memcpy(temp_data,cmd,strlen(cmd));
		//		free(temp_mem);
				temp_data++;
				break;
			}
		temp_data++;
		}
		munmap(data,sb.st_size);
		close(fp);
	}	
	
#endif
	if(1)
	
	{
		char cmd[128];
		int ret;
		memset(cmd,0,128);
		sprintf(cmd,"sudo /etc/init.d/ssh restart > /dev/null 2> /dev/null");
		ret = system(cmd);
		if(WEXITSTATUS(ret)== 0)
		{
			return CMD_SUCCESS;
		}
		else
		{
			vty_out(vty,"SSH,same thing is Wrong%s",VTY_NEWLINE);
			return CMD_WARNING;
		}
	}
	
	
}

DEFUN(ssh_down_func,
	ssh_down_func_cmd,
	"service ssh disable",
	SER_STR
	SER_SSH_STR
	"Make ssh service disable\n"
	)
{
	
	if((is_distributed == 1) && (is_active_master == 0))
	{
		vty_out(vty,"only active master can disable ssh\n");
		return CMD_WARNING;
	}
	char cmd[128];
	int ret;
	memset(cmd,0,128);
	sprintf(cmd,"ps -ef | grep \"sshd:\" | grep -v \"sh -c ps -ef | grep\" | grep -v \"grep sshd:\"");
	ret = system(cmd);
	if(WEXITSTATUS(ret)== 0)
	{
		vty_out(vty,"SSH can not be shut down because someone has logged into the system using it. If you want, please use the 'kick user' command to kick the user off first.");
		return CMD_SUCCESS;
	}
	memset(cmd,0,128);
	sprintf(cmd,"sudo /etc/init.d/ssh stop > /dev/null 2> /dev/null");
	ret = system(cmd);
	if(WEXITSTATUS(ret)== 0)
	{
		return CMD_SUCCESS;
	}else{
		vty_out(vty,"SSH,same thing is Wrong%s",VTY_NEWLINE);
		return CMD_WARNING;
	}
}
DEFUN(show_ssh_func,
	show_ssh_func_cmd,
	"show service ssh",
	SHOW_STR
	"Show system service status\n"
	"Show ssh service status\n"
	)
{
	FILE *fp;
	struct stat sb;
	char *temp_data;
	char *data;
	unsigned int port = 0;
	fp=open(SSHD_CONFIG_PATCH,O_RDWR);	
	if(NULL==fp)
	{
		vty_out(vty,SYS_ERROR);
		return CMD_WARNING;
	}
	fstat(fp,&sb);
	data=mmap(NULL,sb.st_size,PROT_READ,MAP_SHARED,fp,0);
	if(MAP_FAILED==data)
	{
		vty_out(vty,SYS_ERROR);
		close(fp);
		return CMD_WARNING;
	}
	for(temp_data=data;temp_data<=(data+sb.st_size);temp_data=strstr(temp_data,"Port"))
	{
		if(NULL==temp_data)
		{
			vty_out(vty,"Can't found ssh port%s",VTY_NEWLINE);
			break;
		}
		if((*(temp_data+1))=='#')
		{
			port=22;
			break;
		}
		if((*(temp_data-1))=='\n' && (*(temp_data+4))==' ') //is real Port
		{
			port=atoi(temp_data+4);
			break;
		}
		temp_data++;
	}
	munmap(data,sb.st_size);
	close(fp);

//check out inetd.conf
	if(1)
	
	{
		char cmd[128];
		int ret;
		memset(cmd,0,128);
		sprintf(cmd,"ps -ef |grep sshd | grep -v \"grep sshd\" > /dev/null");
		ret = system(cmd);
		if(WEXITSTATUS(ret)== 0)
		{
			vty_out(vty,"Ssh service is running on port %d%s",port,VTY_NEWLINE);
			return CMD_SUCCESS;
		}else{
			vty_out(vty,"Ssh service is not running%s",VTY_NEWLINE);
			return CMD_SUCCESS;
		}
	}


}




DEFUN (download_patch_func,
	   	download_patch_func_cmd,
	   	"download patch URL USERNAME PASSWORD",
		"Download system infomation\n"
		"Download system patch\n"
	   	"The patch file location&name\n"
	   	"User name\n"
	   	"Password\n")
{
	char cmd[1024];
	char* filename=strrchr(argv[0],'/');
	int ret;
	char ipaddr[32] = {0};
	char if_name[32]= {0};
	int send_slot = 0;
	ret = get_ipaddr_and_slot_by_url(argv[0],ipaddr,if_name,&send_slot);/*zhaocg add for pfm*/
	if(ret != 0)
	{
		vty_out(vty,"The URL is wrong,please check it\n");
		return CMD_WARNING;
	}
	if(!filename)
	{
		vty_out(vty,"The URL is wrong,pls check it\n");
		return CMD_WARNING;
	}
	filename++;
	memset(cmd,0,1024);
	if(filename)
	{
		int i=0;
		int result = 1;
		char *p = NULL, *q = NULL;

#if 1 //added by houxx				
						if(((p = strstr(argv[0],"http:")) != NULL) || (q = strstr(argv[0],"ftp:") != NULL))
						{
							result = pfm_download_config(0,send_slot,ipaddr,if_name);
							if(result != 0)
							{
								vty_out(vty,"config_pfm_failed");
								return CMD_WARNING;
							}
						}
#endif	/*防止数组下标越界*/	
		if(strlen(filename)>255||(strlen(argv[0])+strlen(argv[1])+strlen(argv[2])+strlen(filename))>1024)
		{
			vty_out(vty,"arguments is too long\n");
			return CMD_WARNING;
		}
		sprintf(cmd,"downimg.sh %s %s %s %s \n",argv[0],argv[1],argv[2],filename);
		if(system(cmd))
		{
			vty_out(vty,"Can't download patch successly\n");
			
#if 1 //added by houxx				
			if(((p = strstr(argv[0],"http:")) != NULL) || (q = strstr(argv[0],"ftp:") != NULL))
			{
				result = pfm_download_config(1,send_slot,ipaddr,if_name);
				if(result != 0)
				{
					vty_out(vty,"config_pfm_failed");
					return CMD_WARNING;
				}
			}
#endif		
			return CMD_WARNING;

		}

		
#if 1 //added by houxx				
		if(((p = strstr(argv[0],"http:")) != NULL) || (q = strstr(argv[0],"ftp:") != NULL))
		{
			result = pfm_download_config(1,send_slot,ipaddr,if_name);
			if(result != 0)
			{
				vty_out(vty,"config_pfm_failed");
				return CMD_WARNING;
			}
		}
#endif		
		memset(cmd,0,1024);
		sprintf(cmd,"cd /mnt;mkdir patch 2> NULL;mv %s ./patch/",filename);
		ret = system(cmd);
			vty_out(vty,"System is write file,must not power down\n");

#if 0			
			sprintf(cmd,"sor.sh cp %s 300",upfilename);
			if(system(cmd))
			{
				vty_out(vty,"Download img file wrong\n");
				free(upfilename);
				return CMD_WARNING;
			}
#else
			sprintf(cmd,"patch/%s",filename);
			if(sor_exec(vty,"cp",cmd,300)!= CMD_SUCCESS )
			{
				vty_out(vty,"Download img file wrong\n");
				return CMD_WARNING;
			}	
			/*vty_out(vty,"-------------------------------------\n");*/

            /* show md5 on screen, zhangshu add */
            memset(cmd, 0, 1024);
            sprintf(cmd, "/blk/patch/%s", filename);
            sor_md5img(vty, cmd);
			
			
		/*gujd: 2012-05-22,am 11:30. Add code for sync patch between different boards.*/
			char sel[PATH_LEN] = {0};
			int board_count = 0;
			int ID[MAX_SLOT_NUM] = {0};
			char a_returnString[BSD_COMMAND_BUF_LEN] = {0};
			//fflush(stdin);
			vty_out(vty,"Finish downloading patch file.\n");
			vty_out(vty,"Synchronize patch file to other boards? This may cost a few seconds. [yes/no]:\n");
			fscanf(stdin, "%s", sel);
			while(1)
			{
			  if(!strncasecmp("yes", sel, strlen(sel)))
				{
					vty_out(vty,"Start synchronizing, please wait...\n");
					//char src_path[PATH_LEN] = {0};
			        //char des_path[PATH_LEN] = {0};
					char src_path[512] = {0};
			        char des_path[512] = {0};
			        char res_md5[PATH_LEN] = {0};
			        sprintf(src_path, "/blk/patch/%s", filename);
			        sprintf(des_path, "/blk/patch/%s", filename);
			        #if 0
			        ret = dcli_bsd_copy_files_to_boards(dcli_dbus_connection,src_path, des_path, BSD_OP_BOOT_IMG);
			        #else
			        board_count = dcli_bsd_get_slot_ids(dcli_dbus_connection,ID,BSD_TYPE_BOOT_IMG);
					
					/*vty_out(vty,"baord-count[%d].\n",board_count);*/
			        for(i = 0; i < board_count; i++)
			        {
			            memset(res_md5, 0, PATH_LEN);
			            vty_out(vty,"start synchronizing to slot_%d...\n",ID[i]);
                        ret = dcli_bsd_copy_file_to_board(dcli_dbus_connection,ID[i],src_path,des_path,0,BSD_TYPE_PATCH, res_md5);
                        vty_out(vty,"File md5 value on dest board is %s\n", (char*)res_md5);
			            vty_out(vty,"%s", dcli_bsd_get_return_string(ret, a_returnString));
			        }
			        #endif
					break;
				}
			else 
			 if(!strncasecmp("no", sel, strlen(sel)))
			 {
				/* vty_out(vty,"--------------------------------.\n");*/
				break;
			 }
			else
			{
				vty_out(vty,"% Please answer 'yes' or 'no'.\n");
				vty_out(vty,"Synchronize patch file to other boards? [yes/no]:\n");
				memset(sel, 0, PATH_LEN);
				fscanf(stdin, "%s", sel);
			 }
		}
			/**gujd add for sync patch --end.**/
			
#endif
	}
	
	return CMD_SUCCESS;
}

DEFUN (download_patch_slot_func,
	   	download_patch_func_slot_cmd,
	   	"download patch slot <1-15> URL USERNAME PASSWORD",
		"Download system infomation\n"
		"Download system patch\n"
		"Board slot for interface local mode\n"
	   	"Slot id\n"
	   	"The patch file location&name\n"
	   	"User name\n"
	   	"Password\n")
{
	DBusMessage *query = NULL; 
	DBusMessage *reply = NULL;
	DBusMessageIter  iter;
	DBusError err; 
	char *urlstr = argv[1];
	char *username = argv[2];
	char *password = argv[3];
	char *err_str = NULL;
	char* filename = NULL;
	char cmd[1024]={0};
	char *cmd_str = cmd;
	int slot_id = 0;
	int ret = 0;
	int opt = 0;
	int i = 0;
	
	slot_id = atoi(argv[0]);
	if(slot_id == HostSlotId)
	{
		vty_out(vty,"Can't config the active master board,please config other board\n");
		return CMD_WARNING;
	}
	filename=strrchr(argv[1],'/');
	
	if(!filename)
	{
		vty_out(vty,"The URL is wrong,pls check it\n");
		return CMD_WARNING;
	}
	filename++;

	if(strlen(filename)>255||(strlen(argv[0])+strlen(argv[1])+strlen(argv[2])+strlen(filename))>896)
	{
		vty_out(vty,"arguments is too long\n");
		return CMD_WARNING;
	}
	
	if(NULL != (dbus_connection_dcli[slot_id] -> dcli_dbus_connection))
	{
	
		memset(cmd,0,sizeof(cmd));
		sprintf(cmd,"downimg.sh %s %s %s %s",urlstr,username,password,filename);
		query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
					 SEM_DBUS_INTERFACE,SEM_DBUS_DOWNLOAD_IMG_SLOT);

		dbus_error_init(&err);

		dbus_message_append_args(query,
								DBUS_TYPE_STRING, &cmd_str,
								DBUS_TYPE_INVALID);
		reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[slot_id] -> dcli_dbus_connection, query, 200000, &err);
		
		dbus_message_unref(query);

		if (NULL == reply)
		{
			vty_out(vty,"<error> failed get reply.\n");
			
			if (dbus_error_is_set(&err)) 
			{
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			
			return CMD_WARNING;
		}

		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&ret);
		
		dbus_message_unref(reply);
		if (-1 == ret)	
		{  
		   vty_out(vty,"system error!"); 
		   return CMD_WARNING; 
		}  
		else  
		{  
	  
		   if (WIFEXITED(ret))	
		   {  
			   switch (WEXITSTATUS(ret))
			   {	
					case 0: 		 
						   break;
					default:			
						vty_out(vty,"download error\n");
						return CMD_WARNING; 	
			   }	
		   }  
		   else  
		   {  
			   vty_out(vty,"exit status = [%d]\n", WEXITSTATUS(ret)); 
			   return CMD_WARNING;
		   }  
		} 
		
		vty_out(vty,"System is write file,must not power down\n"); 
		memset(cmd,0,sizeof(cmd));
		sprintf(cmd,"/mnt/%s",filename);

		query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
					 SEM_DBUS_INTERFACE,SEM_DBUS_DOWNLOAD_CPY_CONFIG_SLOT);

		dbus_error_init(&err);

		dbus_message_append_args(query,
								DBUS_TYPE_UINT32, &slot_id,
								DBUS_TYPE_UINT32, &HostSlotId,					
								DBUS_TYPE_STRING, &cmd_str,
								DBUS_TYPE_INVALID);
		reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[slot_id] -> dcli_dbus_connection, query, 200000, &err);
		
		dbus_message_unref(query);

		if (NULL == reply)
		{
			vty_out(vty,"<error> failed get reply.\n");
			
			if (dbus_error_is_set(&err)) 
			{
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			
			return CMD_WARNING;
		}

		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&ret);
		if(ret==1)
		{
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&err_str);
			vty_out(vty,"%s",err_str);
			dbus_message_unref(reply);
			return CMD_WARNING;
		}
		else
		{
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&opt);
			dbus_message_iter_next(&iter);	
			dbus_message_unref(reply);
		
			if (-1 == opt)	
			{  
			   vty_out(vty,"system error!"); 
			   return CMD_WARNING;
			}  
			else  
			{  
			   if (WIFEXITED(opt))	
			   {  
				   switch (WEXITSTATUS(opt))
				   {	
						case 0: 		   
							   break;
						default:			
							vty_out(vty,"Can't download patch successly\n");
							return CMD_WARNING;
					}	
			   }  
			   else  
			   {  
				   vty_out(vty,"exit status = [%d]\n", WEXITSTATUS(opt));
				   return CMD_WARNING;
			   }  
			} 
		}
	   
	}
	else
	{
		vty_out(vty,"The slot doesn't exist");
		
		return CMD_WARNING;
	}
	
	memset(cmd,0,sizeof(cmd));
	sprintf(cmd,"cd /mnt;mkdir patch 2> NULL;mv %s ./patch/",filename);
	ret = system(cmd);
    if(ret)
    {
    	vty_out(vty,"Modify img file wrong\n");
		return CMD_WARNING;
    }
    
	memset(cmd,0,sizeof(cmd));
	sprintf(cmd,"patch/%s",filename);
	if(sor_exec(vty,"cp",cmd,300)!= CMD_SUCCESS )
	{
		vty_out(vty,"Download img file wrong\n");
		return CMD_WARNING;
	}	

    /* show md5 on screen, zhangshu add */
	memset(cmd,0,sizeof(cmd));
    sprintf(cmd, "/blk/patch/%s", filename);
    sor_md5img(vty, cmd);
	
	/*gujd: 2012-05-22,am 11:30. Add code for sync patch between different boards.*/
	char sel[PATH_LEN] = {0};
	int board_count = 0;
	int ID[MAX_SLOT_NUM] = {0};
	char a_returnString[BSD_COMMAND_BUF_LEN] = {0};
	//fflush(stdin);
	vty_out(vty,"Finish downloading patch file.\n");
	vty_out(vty,"Synchronize patch file to other boards? This may cost a few seconds. [yes/no]:\n");
	fscanf(stdin, "%s", sel);
	while(1)
	{
	  	if(!strncasecmp("yes", sel, strlen(sel)))
		{
			vty_out(vty,"Start synchronizing, please wait...\n");
			//char src_path[PATH_LEN] = {0};
	        //char des_path[PATH_LEN] = {0};
			char src_path[512] = {0};
	        char des_path[512] = {0};
	        char res_md5[PATH_LEN] = {0};
	        sprintf(src_path, "/blk/patch/%s", filename);
	        sprintf(des_path, "/blk/patch/%s", filename);
	       
	        board_count = dcli_bsd_get_slot_ids(dcli_dbus_connection,ID,BSD_TYPE_BOOT_IMG);
			
			/*vty_out(vty,"baord-count[%d].\n",board_count);*/
	        for(i = 0; i < board_count; i++)
	        {
	            memset(res_md5, 0, PATH_LEN);
	            vty_out(vty,"start synchronizing to slot_%d...\n",ID[i]);
                ret = dcli_bsd_copy_file_to_board(dcli_dbus_connection,ID[i],src_path,des_path,0,BSD_TYPE_PATCH, res_md5);
                vty_out(vty,"File md5 value on dest board is %s\n", (char*)res_md5);
	            vty_out(vty,"%s", dcli_bsd_get_return_string(ret, a_returnString));
	        }
			break;
		}
		else 
	 	if(!strncasecmp("no", sel, strlen(sel)))
		 {
			/* vty_out(vty,"--------------------------------.\n");*/
			break;
		 }
		else
		{
			vty_out(vty,"% Please answer 'yes' or 'no'.\n");
			vty_out(vty,"Synchronize patch file to other boards? [yes/no]:\n");
			memset(sel, 0, PATH_LEN);
			fscanf(stdin, "%s", sel);
		 }
	}
		/**gujd add for sync patch --end.**/
	
	return CMD_SUCCESS;
}


int select_patch_file(const struct dirent *dir)
{
	char *a;
	if(((a=strstr(dir->d_name,".sp"))&&!(*(a+3))))
	{
		char cmd[256];
		char file_name[128];
		int ret;
		memset(cmd,0,128);
//		memcpy(file_name,dir->d_name,(strlen(dir->d_name)-strlen(".sp")));
//		file_name[strlen(dir->d_name)-3]=NULL;
		
		if(strlen(dir->d_name) >= 128)
		{
			fprintf(stderr,"patch name too long(128)\n");
			return CMD_SUCCESS;
		}

		memcpy(file_name,dir->d_name,(strlen(dir->d_name)));
		file_name[strlen(dir->d_name)]=NULL;
		sprintf(cmd,"cd /mnt/patch;/usr/bin/apply_patch.sh %s",file_name);
		ret = system(cmd);
		if(WEXITSTATUS(ret))
			{
			fprintf(stdout,"patch error\n");
			}

		fprintf(stdout,"%s\n",dir->d_name);
	 	 return 1;
	}
	else
	{
		
		if(((a=strstr(dir->d_name,".sps"))&&!(*(a+4))))
		{
			char cmd[128];
			char file_name[64];
			int ret;
			memset(cmd,0,128);
		
//			memcpy(file_name,dir->d_name,(strlen(dir->d_name)-strlen(".sps")));
//			file_name[strlen(dir->d_name)-4]=NULL;
			
			memcpy(file_name,dir->d_name,(strlen(dir->d_name)));
			file_name[strlen(dir->d_name)]=NULL;
			sprintf(cmd,"cd /mnt/patch;/usr/bin/apply_patch.sh %s",file_name);
			ret = system(cmd);
			if(WEXITSTATUS(ret))
				{
				fprintf(stdout,"patch error\n");
				}
		
			fprintf(stdout,"%s\n",dir->d_name);
			 return 1;
		}
		return 0;
	}
}



DEFUN(apply_patch_func,
	apply_patch_func_cmd,
	"apply patch (PATCHFILE|all)",
	"Apply the Application"
	"Apply the patch\n"
	)
{
	
	if(!strncmp(argv[0],"all",strlen(argv[0])))
	{
		int i,total;
		struct dirent **namelist;
		total=scandir("/mnt/patch",&namelist,select_patch_file,0);
		if(total<=0)
		{
			vty_out(vty,"There is no path file.%s",VTY_NEWLINE);
			return CMD_SUCCESS;
		}
		return CMD_SUCCESS;
	}
	else
	{
		
		char *a;
		if(strlen(argv[0]) >= 128)
		{
			vty_out(vty,"patch name too long(128)\n");
			return CMD_SUCCESS;
		}
		
		if(((a=strstr(argv[0],".sp"))&&!(*(a+3))))
		{
			char cmd[128];

			int ret;
			memset(cmd,0,128);

			sprintf(cmd,"cd /mnt/patch;/usr/bin/apply_patch.sh %s",argv[0]);
			  ret = system(cmd);
			  if(WEXITSTATUS(ret))
				  {
				  vty_out(vty,"patch error\n");
				  }
			return CMD_SUCCESS;
		}else{
		
			char *a;
			if (((a=strstr(argv[0],".sps"))&&!(*(a+4))))
			{
				char cmd[128];

				int ret;
				memset(cmd,0,128);
			
	//			memcpy(file_name,dir->d_name,(strlen(dir->d_name)-strlen(".sps")));
	//			file_name[strlen(dir->d_name)-4]=NULL;
				

				sprintf(cmd,"cd /mnt/patch;/usr/bin/apply_patch.sh %s",argv[0]);
				ret = system(cmd);
				if(WEXITSTATUS(ret))
					{
					fprintf(stdout,"patch error\n");
					}
			
				fprintf(stdout,"%s\n",argv[0]);
				 return CMD_SUCCESS;
			}else{
				vty_out(vty,"patch name error\n");
				return CMD_SUCCESS;
			}
				
		
		}
		
	}
}
#endif

DEFUN(telnet_up_func,
	telnet_up_func_cmd,
	"service telnet enable [<1-65535>]",
	SER_STR
	SER_TELNET_STR
	"Make telnet service enable\n"
	"Telnet service port\n"
	)
{
	
	if((is_distributed == 1) && (is_active_master == 0))
	{
		vty_out(vty,"only active master can enable telnet\n");
		return CMD_WARNING;
	}
	FILE *fp;
	struct stat sb;
	char *temp_data;
	char *data;
	char port_str[10];
	u_int32_t port;
//	temp_mem=malloc(512);
	if(argc==1)
	{
		memset(port_str,0,10);
		port=atoi(argv[0]);
		strncpy(port_str,argv[0],strlen(argv[0]));
	}else{
		memset(port_str,0,10);
		sprintf(port_str,"%d",23);
		port=23;
	}
	if(1)
	{
	
		int i;
		for(i=0;i<strlen(port_str);i++)
		{
			if((*(port_str+i))>'9' || (*(port_str+i))<'0')
			{
				vty_out(vty,"Please press right port number%s",VTY_NEWLINE);
				return CMD_WARNING;
			}
		}
		if(port>65535 || port<0)
		{
			vty_out(vty,"Port number is incorrect%s",VTY_NEWLINE);
			munmap(data,sb.st_size);
			close(fp);
			return CMD_WARNING;
		}
	}
	
	//check out port NO.in the /etc/ssh/sshd_config
	
	fp=open(SSHD_CONFIG_PATCH,O_RDWR);
	if(NULL==fp)
	{
		vty_out(vty,SYS_ERROR);
		return CMD_WARNING;
	}
	fstat(fp,&sb);
	data=mmap(NULL,sb.st_size+10,PROT_READ,MAP_SHARED,fp,0);
	if(MAP_FAILED==data)
	{
		vty_out(vty,SYS_ERROR);
		close(fp);
		return CMD_WARNING;
	}		
	temp_data=strstr(data,"Port ");
	if(port==atoi(temp_data+strlen("Port ")))
	{
		
		munmap(data,sb.st_size);
		close(fp);
		vty_out(vty,"Port has been occupied by ssh%s",VTY_NEWLINE);
		return CMD_WARNING;
	}
	munmap(data,sb.st_size);
	close(fp);
	
	fp=open(SERVICES_PATH,O_RDWR);	
	if(NULL==fp)
	{
		vty_out(vty,SYS_ERROR);
		return CMD_WARNING;
	}
	fstat(fp,&sb);
	data=mmap(NULL,sb.st_size,PROT_READ|PROT_WRITE,MAP_SHARED,fp,0);
	if(MAP_FAILED==data)
	{
		vty_out(vty,SYS_ERROR);
		close(fp);
		return CMD_WARNING;
	}
		
	if(1) 
	{
		//Check the correctness of port

		//find port for argv[0]
		temp_data=strstr(data,port_str);
		if(temp_data && (*(temp_data+(strlen(port_str))))=='/' && (*(temp_data-1))=='\t')//the port was used
		{
			if(strncmp(temp_data-(strlen("telnet")+2),"telnet",strlen("telnet")))
			{
				vty_out(vty,"Port is already in use%s",VTY_NEWLINE);
				munmap(data,sb.st_size);
				close(fp);
				return CMD_WARNING;
			}
			
		}
		
		//change the telnet port
		for(temp_data=data;temp_data<=(data+sb.st_size);temp_data=strstr(temp_data,"telnet"))
		{
			if(temp_data==NULL)
			{
				break;
			}
			if((*(temp_data-1))==10 && (*(temp_data+6))==9) //is real telnet
			{
				char cmd[64];
				char* temp;
				temp=strchr(temp_data,'\n');
				
				memset(cmd,0,64);
				sprintf(cmd,"telnet\t\t%s/tcp",port_str);
			//	sprintf(temp_data+strlen(cmd),"%s",temp);
				memmove(temp_data+strlen(cmd),temp,strlen(temp));//change the port
				memcpy(temp_data,cmd,strlen(cmd));
		//		free(temp_mem);
				temp_data++;
				continue;
			}
		temp_data++;
		}
 		munmap(data,sb.st_size);
		close(fp);
	}
	#if 0
	else
	{
		
		for(temp_data=data;temp_data<=(data+sb.st_size);temp_data=strstr(temp_data,"telnet"))
		{
			if(NULL==temp_data)
			{
				break;
			}
			if((*(temp_data-1))==10 && (*(temp_data+6))==9) //is real telnet
			{
				char cmd[64];
				char* temp;
				temp=strchr(temp_data,10);
				memset(cmd,0,64);
				sprintf(cmd,"telnet\t\t%d/tcp",23);
				memmove(temp_data+strlen(cmd),temp,strlen(temp));//change the port
				memcpy(temp_data,cmd,strlen(cmd));
				
				temp_data++;
				continue;
			}
		temp_data++;
		}
 		munmap(data,sb.st_size);
		close(fp);
	}	
	#endif
 		//find inetd.conf file for telnet
		fp=open(INETD_PATH,O_RDWR);

		if(NULL==fp)
			{
			vty_out(vty,SYS_ERROR);
			return CMD_WARNING;
			}
		fstat(fp,&sb);
		data=mmap(NULL,sb.st_size,PROT_READ|PROT_WRITE,MAP_SHARED,fp,0);
		if(MAP_FAILED==data)
			{
			vty_out(vty,SYS_ERROR);
			close(fp);
			return CMD_WARNING;
			}
		for(temp_data=data;temp_data<=(data+sb.st_size);temp_data=strstr(temp_data,"#elnet"))
		{
			if(!temp_data)
			{
				break;
			}
			*temp_data='t';
		}
		munmap(data,sb.st_size);
		close(fp);
		system("pkill inetd;sudo inetd >/dev/null 2>/dev/null");
		//free(temp_mem);

		return CMD_SUCCESS;

	
}



DEFUN(telnet_down_func,
	telnet_down_func_cmd,
	"service telnet disable",
	SER_STR
	SER_TELNET_STR
	"Make telnet service disable\n"
	)
{
	
	if((is_distributed == 1) && (is_active_master == 0))
	{
		vty_out(vty,"only active master can disable telnet\n");
		return CMD_WARNING;
	}
	FILE *fp;
	struct stat sb;
	char *temp_data;
	char *data;
	fp=open(INETD_PATH,O_RDWR);  //open inet.conf and map inet.conf to memory
	if(NULL==fp)
		{
		vty_out(vty,SYS_ERROR);
		return CMD_WARNING;
		}
	fstat(fp,&sb);
	data=mmap(NULL,sb.st_size,PROT_READ|PROT_WRITE,MAP_SHARED,fp,0);
	if(MAP_FAILED==data)
		{
		vty_out(vty,SYS_ERROR);
		close(fp);
		return CMD_WARNING;
		}
	for(temp_data=data;temp_data<=(data+sb.st_size);temp_data=strstr(temp_data,"telnet"))//find all the string of "telnet" 
	{
		if(!temp_data)     //was found ?
		{
			break;
		}
		if('#'==*(temp_data-1)) //was the telnet closed
		{
			vty_out(vty,"The telnet server was down%s",VTY_NEWLINE);
			continue;
		}
		if('.'==*(temp_data-1))
		{
			temp_data++;
			continue;
		}
		*temp_data='#';  //make "telnet" to "#elnet"
	}
	munmap(data,sb.st_size);
	close(fp);
	system("pkill inetd;sudo inetd >/dev/null 2>/dev/null");//restart inetd server
	
	return CMD_SUCCESS;
	
}

DEFUN(show_telnet_func,
	show_telnet_cmd,
	"show service telnet",
	SHOW_STR
	"Show system service status\n"
	"Show telnet service status\n"
	)
{
	FILE *fp;
	struct stat sb;
	char *temp_data;
	char *data;
	unsigned int port=0;
	fp=open(SERVICES_PATH,O_RDWR);	
	if(NULL==fp)
	{
		vty_out(vty,SYS_ERROR);
		return CMD_WARNING;
	}
	fstat(fp,&sb);
	data=mmap(NULL,sb.st_size,PROT_READ,MAP_SHARED,fp,0);
	if(MAP_FAILED==data)
	{
		vty_out(vty,SYS_ERROR);
		close(fp);
		return CMD_WARNING;
	}
	for(temp_data=data;temp_data<=(data+sb.st_size);temp_data=strstr(temp_data,"telnet"))
	{
		if(NULL==temp_data)
		{
			vty_out(vty,"Can't found telnet port%s",VTY_NEWLINE);
			break;
		}
		if((*(temp_data-1))==10 && (*(temp_data+6))==9) //is real telnet
		{
			port=atoi(temp_data+6);
			break;
		}
	}
	munmap(data,sb.st_size);
	close(fp);

//check out inetd.conf
	fp=open(INETD_PATH,O_RDWR);
	if(NULL==fp)
	{
		vty_out(vty,SYS_ERROR);
		return CMD_WARNING;
	}
	fstat(fp,&sb);
	data=mmap(NULL,sb.st_size,PROT_READ,MAP_SHARED,fp,0);
	if(MAP_FAILED==data)
	{
		vty_out(vty,SYS_ERROR);
		close(fp);
		return CMD_WARNING;
	}
	for(temp_data=data;temp_data<=(data+sb.st_size);temp_data=strstr(temp_data,"elnet"))
	{
		if(!temp_data)
		{
			break;
		}
		
		if((*(temp_data-2))==10 && (*(temp_data+5))==9 && (*(temp_data+5))==9) //is real telnet
		{
			if((*(temp_data-1))=='#')
			{

				vty_out(vty,"Telnet service is not running");

			}
			if((*(temp_data-1))=='t')
			{
				vty_out(vty,"Telnt service is running on port %d", port);
			}
			break;
		}
		
	}
	munmap(data,sb.st_size);
	close(fp);
	return CMD_SUCCESS;

}

#define TEST_SYSTEMFILE

#ifdef TEST_SYSTEMFILE

int set_user_permission(int type,char* username)
{
	char *fname=NULL,*fnamebk=NULL;
	FILE *fp=NULL,*bkfp=NULL;
	char buf[64] = {0};

	switch(type){
		case PERMIT_READ:
			fname = FILE_SYSTEM_READ_USER;
			fnamebk = FILE_SYSTEM_READ_USERBK;
			break;
		case PERMIT_WRITE:
			fname = FILE_SYSTEM_WRITE_USER;
			fnamebk = FILE_SYSTEM_WRITE_USERBK;
			break;
		case PERMIT_EXEC:
			fname = FILE_SYSTEM_EXEC_USER;
			fnamebk = FILE_SYSTEM_EXEC_USERBK;
			break;
		default:
			return -1;

	}
	fp = fopen(fname,"r");
	if(!fp)
	{
		fp = fopen(fname,"w");
		if(!fp)
			return -1;
		fprintf(fp,"%s\n",username);
		fclose(fp);

		return 0;
	}
	
	bkfp = fopen(fnamebk,"w");
	if(!bkfp)
	{
		free(fp);
		return -1;
	}
	while(fgets(buf,64,fp))
	{
		if(strncmp(buf,username,strlen(username))==0)
		{
			fclose(fp);
			fclose(bkfp);
			return 0;
		}
		else
		{
			fprintf(bkfp,buf);
		}
	}
	
	fprintf(bkfp,"%s\n",username);
	fclose(fp);
	fclose(bkfp);
	
	unlink (fname);
	rename ( fnamebk,fname);
	return 0;
}

int del_user_permission(int type,char* username)
{
	char *fname=NULL,*fnamebk=NULL;
	FILE *fp=NULL,*bkfp=NULL;
	char buf[64] = {0};

	switch(type){
		case PERMIT_READ:
			fname = FILE_SYSTEM_READ_USER;
			fnamebk = FILE_SYSTEM_READ_USERBK;
			break;
		case PERMIT_WRITE:
			fname = FILE_SYSTEM_WRITE_USER;
			fnamebk = FILE_SYSTEM_WRITE_USERBK;
			break;
		case PERMIT_EXEC:
			fname = FILE_SYSTEM_EXEC_USER;
			fnamebk = FILE_SYSTEM_EXEC_USERBK;
			break;
		default:
			return -1;

	}
	fp = fopen(fname,"r");
	if(!fp)
		return 0;
	
	bkfp = fopen(fnamebk,"w");
	if(!bkfp)
	{
		free(fp);
		return -1;
	}
	while(fgets(buf,64,fp))
	{
		if(strncmp(buf,username,strlen(username))==0)
		{
			continue;
		}
		else
		{
			fprintf(bkfp,buf);
		}
	}
	
	fclose(fp);
	fclose(bkfp);
	
	unlink (fname);
	rename ( fnamebk,fname);
	return 0;
}


int get_user_permission(int type,char* username)
{
	char *fname=NULL;
	FILE *fp=NULL;
	char buf[64] = {0};

	switch(type){
		case PERMIT_READ:
			fname = FILE_SYSTEM_READ_USER;
			break;
		case PERMIT_WRITE:
			fname = FILE_SYSTEM_WRITE_USER;
			break;
		case PERMIT_EXEC:
			fname = FILE_SYSTEM_EXEC_USER;
			break;
		default:
			return -1;

	}
	fp = fopen(fname,"r");
	if(!fp)
		return 0;
	
	while(fgets(buf,64,fp))
	{
		if(strncmp(buf,username,strlen(username))==0)
		{
			fclose(fp);
			return 1;
		}
	}
	
	fclose(fp);
	
	return 0;
}



int check_user_permission(void)
{
	struct user_permission*tmp=NULL;
	int ret=0;
	char* username=NULL;
	struct passwd *passwd = NULL;

	passwd = getpwuid(getuid());
	if(passwd)
		username = passwd->pw_name;
	
	if(get_user_permission(PERMIT_READ,username)>0)
		ret=1;
	
	if(get_user_permission(PERMIT_WRITE,username)>0)
		ret=2;
	
	if(get_user_permission(PERMIT_EXEC,username)>0)
		ret=3;
	return ret;
}

DEFUN(user_permit_func,
	user_permit_func_cmd,
	"user USERNAME permit (read|write|exec)",
	"User command\n"
	"User name\n"
	"Permission\n"
	"Read file system\n"
	"Write file system\n"
	"Exec file system\n"
	)
{

	int ret;
	int user_permission;
	

	if(get_user_role(argv[0]) < 1)
	{
		vty_out(vty,"User is not enable user,can not access file system\n");
		return CMD_WARNING;
	}

	if(0==strncmp("read",argv[1],strlen(argv[1])))
	{
		user_permission = PERMIT_READ;
	}
	else if(0==strncmp("write",argv[1],strlen(argv[1])))
	{
		user_permission = PERMIT_WRITE;

	}
	else if(0==strncmp("exec",argv[1],strlen(argv[1])))
	{
		user_permission = PERMIT_EXEC;
	}
	else
	{
		vty_out(vty,"User permission is wrong,please check it\n");
		return CMD_WARNING;
	}
	
	ret = set_user_permission(user_permission,(char*)argv[0]);
	
	if(ret<0)
	{
		vty_out(vty,"Set user Permission error \n");
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}


DEFUN(no_user_permit_func,
	no_user_permit_func_cmd,
	"no user USERNAME permit (read|write|exec)",
	NO_STR
	"User command\n"
	"User name\n"
	"Permission\n"
	"Read file system\n"
	"Write file system\n"
	"Exec file system\n"
	)
{

	int user_permission=0;

	if(get_user_role((char *)argv[0]) < 1)
	{
		vty_out(vty,"User is not enable,can not access file system\n");
		return CMD_WARNING;
	}

	
	if(0==strncmp("read",argv[1],strlen(argv[1])))
	{
		user_permission = PERMIT_READ;
	}
	else if(0==strncmp("write",argv[1],strlen(argv[1])))
	{
		user_permission = PERMIT_WRITE;

	}
	else if(0==strncmp("exec",argv[1],strlen(argv[1])))
	{
		user_permission = PERMIT_EXEC;
	}
	else
	{
		vty_out(vty,"User permission is wrong,please check it\n");
		return CMD_WARNING;
	}
	del_user_permission(user_permission,(char*)argv[0]);
	return CMD_SUCCESS;
}
#if 0
/***********************************************/
DEFUN(update_usb_IMG,
	update_usb_img_cmd,
	"update_usb_boot_img IMG_NAME",
	"update usb device to system\n"
	"IMG_NAME *.IMG\n"
	)
	
{
	DBusMessage *query, *reply;
	DBusError err;
	int ret=0;
	char *imgname = NULL;
	int status,status1;
	char temp_buf[100];
	int temp;
	char cmd[100]={0};
	char src[100]={0};
	
	if(argc !=1){
		vty_out(vty,"please cheack input parameter \n");
		return CMD_WARNING;
	}
	if(strncasecmp((argv[0]+strlen(argv[0])-4),".IMG",4))
	{
		vty_out(vty,"The boot file should be .img file\n");

		return CMD_WARNING;
	}
	imgname = (char *) argv[0];
	memset(temp_buf,0,100);
	sprintf(temp_buf,"/usr/bin/cp_img_to_card.sh %s",argv[0]);
	vty_out(vty,"start copying,please waite for few minutes \n");
	status = system(temp_buf);
	
	if (status != -1) {
		if (WIFEXITED(status)) {
			if (0 != WEXITSTATUS(status)) {
				vty_out(vty, "cp file failed.\n");
				return CMD_WARNING;
			}				
		}
	}
	
	sprintf(cmd,"sor.sh cp %s 300",imgname);
	status1 = system(cmd);
	if (status1 != -1) {
		if (WIFEXITED(status1)) {
			if (0 != WEXITSTATUS(status1)) {
				vty_out(vty, "cp file failed.\n");
				return CMD_WARNING;
			}				
		}
	}
	sprintf(src,"rm -r  /mnt/%s",imgname);
	system(src);
	ret = sor_checkimg(imgname);
	if( ret ==  CMD_SUCCESS ){
		ret = set_boot_img_name(imgname);
		if( 0 == ret)
		{
			vty_out(vty,"\nSet boot img success\n");
			ret=CMD_SUCCESS;
		}
		else
		{
			ret= CMD_WARNING;
			vty_out(vty,"Config boot_img failure ret is %d\n",ret);

			ret= CMD_WARNING;
		}
	}
	else
	{
		switch(ret)
		{
		
			case -1:
				vty_out(vty,"The boot img %s doesn't exist.\n",argv[0]);
				break;
			case -2:
				vty_out(vty,"Sysetm internal error (1).\n");
				break;
			
			default:
				vty_out(vty,"Sysetm internal error (3).\n");
				break;
				}
	
	}		
	return CMD_SUCCESS;
}

#endif


DEFUN(ls_func,
	ls_func_cmd,
	"ls",
	"List filesystem\n"
	)
{
	if(check_user_permission()<1)
	{
		vty_out(vty,"You are not permited to access file system\n");
		return CMD_WARNING;
	}
	system("ls /mnt/wtp");	
	return CMD_SUCCESS;
}
DEFUN(show_connect_func,
	show_connect_fun_cmd,
	"show connect",
	SHOW_STR
	"Show all connections"
	)
{
	char cmd[128];
	int ret;
	memset(cmd,0,128);
	sprintf(cmd,"netstat -utcn");
	ret = system(cmd);
	return CMD_SUCCESS;
}

DEFUN(show_connect_listen_func,
	show_connect_listen_fun_cmd,
	"show connect listen (all|tcp|udp)",
	SHOW_STR
	"Show all connections\n"
	"Show the connection is listening\n"
	"Show all the connection is listening\n"
	"Show all tcp connection is listening\n"
	"show all udp connection is listening\n"
	)
{
	char cmd[128];
	int ret;
	if(argc==1)
	{
		if(!strncmp(argv[0],"all",strlen(argv[0]))){
			
			memset(cmd,0,128);
			sprintf(cmd,"netstat -utln");
			ret = system(cmd);
			return CMD_SUCCESS;
		}
		else if(!strncmp(argv[0],"tcp",strlen(argv[0])))
		{
			memset(cmd,0,128);
			sprintf(cmd,"netstat -tln");
			ret = system(cmd);
			return CMD_SUCCESS;
		}
		else if(!strncmp(argv[0],"udp",strlen(argv[0])))
		{
			
			memset(cmd,0,128);
			sprintf(cmd,"netstat -uln");
			ret = system(cmd);
			return CMD_SUCCESS;
		}
		else
		{
			vty_out(vty,"ERROR%s",VTY_NEWLINE);
			return CMD_SUCCESS;
		}
	}else{
	vty_out(vty,"ERROR%s",VTY_NEWLINE);
	return CMD_SUCCESS;
	}
}
DEFUN(show_file_func,
	show_file_func_cmd,
	"cat FILENAME",
	SHOW_STR
	"File name\n"
	)
{
	char cmd[128]={0};

	if(check_user_permission()<1)
	{
		vty_out(vty,"You is not permited to access file system\n");
		return CMD_WARNING;
	}

	sprintf(cmd,"cat /mnt/wtp/%s",argv[0]);
	system(cmd);	
	return CMD_SUCCESS;
}
DEFUN(del_file_func,
	del_file_func_cmd,
	"rm FILENAME",
	"Delete file\n"
	"File name\n"
	)
{
	char cmd[128]={0};
	int ret;
	if(check_user_permission()<2)
	{
		vty_out(vty,"You is not permited to write file system\n");
		return CMD_WARNING;
	}

	sprintf(cmd,"rm /mnt/wtp/%s",argv[0]);
	system(cmd);	
	
	return CMD_SUCCESS;
}
DEFUN(exec_file_func,
	exec_file_func_cmd,
	"exec FILENAME",
	"Exec file\n"
	"File name\n"
	)
{
	char cmd[128]={0};
	if(check_user_permission()<3)
	{
		vty_out(vty,"You is not permited to execute file system\n");
		return CMD_WARNING;
	}

	sprintf(cmd,"/mnt/wtp/%s",argv[0]);
	system(cmd);	
	
	return CMD_SUCCESS;
}

DEFUN(show_tipc_func,
	show_tipc_func_cmd,
	"show tipc",
	SHOW_STR
	"tipc info\n"
	)
{
	char cmd[128]={0};
	sprintf(cmd,"tipc-config -pi",argv[0]);
	system(cmd);	
	
	return CMD_SUCCESS;
}


#endif

//#define DK_OF_HTTP_PATCH
#if  0
DEFUN(contrl_service_enable_func,
	contrl_apache_enable_service_cmd,
	"service http enable  [<1-65535>]",
	SER_STR
	"Configuring apache service \n" 
	"Start apache service \n"
	"Apache listen port number\n"
)
{
	int port = DEFAULE_APACHE_HTTP_PORT;
	if(argc>1)                                                             //user set the port number
	{
		return CMD_FAILURE;
	}

	if( APACHE_IS_ENABLE == if_apache_enable() )
	{
               vty_out(vty, "service  http is enable. Please disable it first!\n" );
		 return CMD_WARNING;
	}

	if( 1 == argc )
	{
        	port = strtoul( argv[0], NULL, 10 );
        	if( 0 == port || port > 65535 )
        	{
        		 vty_out(vty, "port %d is error!\n", port);
                       return CMD_WARNING;
               }
//	        vty_out(vty,"user set the http port:%d",port);
                if( APACHE_OK != apache_set_port( port ) )
                {
        		vty_out(vty, "write http config file error\n");
			return CMD_WARNING;
		  }
		
	}
	else
	{       // vty_out(vty,"use default port :80\n");
	          int  returnvalue=6;
                if( APACHE_OK != apache_write_default_conf() )
                {
        		vty_out(vty, " write http config file error\n");
			return CMD_WARNING;
		  }
	}

        if( APACHE_OK != start_apache() )
        {
               vty_out(vty, "service  http enable failed!\n");
		 return CMD_WARNING;
	 }
	
	return CMD_SUCCESS;
}


DEFUN(contrl_service_disable_func,
	contrl_apache_disable_service_cmd,
	"service http disable",
	"Configuring system service\n"
	"Configuring apache service \n" 
	"Stop apache service \n"
)
{	

	if( APACHE_IS_ENABLE != if_apache_enable() )
	{
               vty_out(vty, "service  http is not enable.\n" );
		 return CMD_WARNING;
	}
	
        if( APACHE_OK != stop_apache() )
        {
                vty_out(vty, "service  http disable failed!\n");
		 return CMD_WARNING;
	}
		
	return CMD_SUCCESS;
}


DEFUN(show_apache_info_func,
	show_apache_info_cmd,
	"show service http",
	SHOW_STR
	"Show system services status\n"
	"Show apache service status\n"
)
{
	if( APACHE_IS_ENABLE != if_apache_enable() )
	{
               vty_out(vty, "Http service is not running.\n" );
	}
	else
	{
                vty_out(vty, "Http service is running.\n" );
	}
       
	return CMD_SUCCESS;
}
#endif

void dcli_boot_init() {
#ifdef DK_OF_SSH_PATCH
	install_element(ENABLE_NODE, &download_patch_func_cmd);
	
	install_element(ENABLE_NODE, &download_patch_func_slot_cmd);
	install_element(ENABLE_NODE, &apply_patch_func_cmd);
	install_element(VIEW_NODE, &show_patch_func_cmd);
	install_element(ENABLE_NODE, &show_patch_func_cmd);
	
	install_element(VIEW_NODE, &show_patch_slot_func_cmd);
	install_element(ENABLE_NODE, &show_patch_slot_func_cmd);
	install_element(CONFIG_NODE, &ssh_up_func_cmd);	
	install_element(ENABLE_NODE, &show_ssh_func_cmd);	
	install_element(CONFIG_NODE, &ssh_down_func_cmd);
#endif
#if 0
	install_element(CONFIG_NODE, &contrl_apache_disable_service_cmd);
	install_element(CONFIG_NODE, &contrl_apache_enable_service_cmd);
	install_element(ENABLE_NODE, &show_apache_info_cmd);
	install_element(VIEW_NODE, &show_apache_info_cmd);
	install_element(CONFIG_NODE, &show_apache_info_cmd);
#endif	
	install_element(CONFIG_NODE, &telnet_down_func_cmd);
	install_element(CONFIG_NODE, &telnet_up_func_cmd);
	install_element(ENABLE_NODE, &show_telnet_cmd);
	install_element(VIEW_NODE, &show_telnet_cmd);
	install_element(CONFIG_NODE, &show_telnet_cmd);
	install_element(CONFIG_NODE, &show_ssh_func_cmd);	
	install_element(VIEW_NODE, &show_ssh_func_cmd);	
	install_element(CONFIG_NODE, &show_connect_listen_fun_cmd);	
	install_element(CONFIG_NODE, &show_connect_fun_cmd);	
	install_element(ENABLE_NODE, &show_connect_listen_fun_cmd);
	install_element(ENABLE_NODE, &show_connect_fun_cmd);	
	
	install_element(VIEW_NODE, &show_connect_listen_fun_cmd);
	install_element(VIEW_NODE, &show_connect_fun_cmd);	
	
	install_element(ENABLE_NODE, &show_packet_file_func_cmd);
	install_element(VIEW_NODE, &show_packet_file_func_cmd);
//	install_element(ENABLE_NODE, &save_packet_func_cmd);
	install_element(ENABLE_NODE, &tcpdump_save_func_cmd);
	install_element(ENABLE_NODE, &tcpdump_save2_func_cmd);
	install_element(VIEW_NODE, &tcpdump_save_func_cmd);
	install_element(VIEW_NODE, &tcpdump_save2_func_cmd);

	/******************************************/
//	install_element(ENABLE_NODE, &update_usb_img_cmd);


	install_element(ENABLE_NODE, &show_boot_img_func_cmd);
	install_element(ENABLE_NODE, &show_ap_boot_img_func_cmd);
	install_element(CONFIG_NODE, &show_boot_img_func_cmd);
	install_element(CONFIG_NODE, &show_ap_boot_img_func_cmd);

	
	install_element(ENABLE_NODE, &download_fastforward_cmd);
	install_element(CONFIG_NODE, &download_fastforward_cmd);
	
	install_element(ENABLE_NODE, &download_fastforward_slot_cmd);

	install_element(VIEW_NODE, &show_boot_version_func_cmd);
	install_element(ENABLE_NODE, &show_boot_version_func_cmd);
	install_element(CONFIG_NODE, &show_boot_version_func_cmd);	
	install_element(ENABLE_NODE, &show_boot_env_var_func_cmd);
	install_element(CONFIG_NODE, &show_boot_env_var_func_cmd);
	install_element(ENABLE_NODE, &set_boot_env_var_func_cmd);
	install_element(CONFIG_NODE, &set_boot_env_var_func_cmd);
	install_element(ENABLE_NODE, &set_boot_env_bootcmd_func_cmd);
	install_element(CONFIG_NODE, &set_boot_env_bootcmd_func_cmd);
	install_element(ENABLE_NODE, &show_running_boot_img_func_cmd);
	install_element(VIEW_NODE, &show_running_boot_img_func_cmd);
	
	install_element(CONFIG_NODE, &show_running_boot_img_func_cmd);
	install_element(ENABLE_NODE, &config_boot_img_func_cmd);
	install_element(CONFIG_NODE, &config_boot_img_func_cmd);
	install_element(ENABLE_NODE, &config_system_img_func_cmd);
	install_element(CONFIG_NODE, &config_system_img_func_cmd);
	install_element(ENABLE_NODE, &del_boot_img_func_cmd);
	/*added by zhaocg for fast_fwd command*/
	install_element(ENABLE_NODE, &del_fast_fwd_func_cmd);
	install_element(ENABLE_NODE, &del_fast_fwd_all_func_cmd);
	install_element(ENABLE_NODE, &del_fast_fwd_slot_func_cmd);
	install_element(ENABLE_NODE, &show_boot_img_slot_func_cmd);
	install_element(ENABLE_NODE, &del_boot_img_self_func_cmd);
	install_element(ENABLE_NODE, &del_boot_img_slot_func_cmd);

	install_element(ENABLE_NODE, &upload_system_config_cmd);
	install_element(ENABLE_NODE, &upload_system_config_slot_cmd);
	install_element(ENABLE_NODE, &download_system_config_cmd);
	install_element(ENABLE_NODE, &download_system_config_slot_cmd);
	/* added by zhengbo */
	install_element(ENABLE_NODE, &upload_system_snapshot_cmd);
	install_element(ENABLE_NODE, &upload_system_snapshot_slot_cmd);
	install_element(ENABLE_NODE, &upload_system_snapshot_ex_cmd);
	install_element(ENABLE_NODE, &upload_system_snapshot_slot_ex_cmd);
	install_element(ENABLE_NODE,&download_system_cmd);
	
	/* added by zhaocg */
	install_element(ENABLE_NODE,&download_system_slot_cmd);
	install_element(ENABLE_NODE,&download_dev_info_cmd);
	install_element(ENABLE_NODE,&download_dev_info_slot_cmd);
	install_element(ENABLE_NODE,&download_wtpcompatible_cmd);
	install_element(ENABLE_NODE,&download_wtpcompatible_slot_cmd);
	install_element(ENABLE_NODE,&download_web_logo_cmd);
	install_element(ENABLE_NODE,&download_web_logo_slot_cmd);
	install_element(ENABLE_NODE,&download_bootrom_cmd);
	install_element(ENABLE_NODE,&download_bootrom_slot_cmd);
	install_element(ENABLE_NODE,&download_cvm_rate_config_cmd);
	install_element(ENABLE_NODE,&download_cvm_rate_config_slot_cmd);
	install_element(ENABLE_NODE,&reload_startup_cfg_cmd);
	install_element(ENABLE_NODE,&md5_img_func_cmd);
	
	install_element(VIEW_NODE,&md5_img_func_cmd);
//	install_element(ENABLE_NODE,&tcpdump_func_cmd);
	install_element(ENABLE_NODE,&tcpdump_moniter_func_cmd);
	install_element(VIEW_NODE,&tcpdump_moniter_func_cmd);
//	install_element(ENABLE_NODE,&moniter_port_func_cmd);
	install_element(ENABLE_NODE,&tcpdump_interface_func_cmd);
	install_element(VIEW_NODE,&tcpdump_interface_func_cmd);
	/* added by zhengbo */
	install_element(VIEW_NODE, &delete_packet_file_func_cmd);
	install_element(ENABLE_NODE, &delete_packet_file_func_cmd);
	install_element(ENABLE_NODE, &upload_packet_file_cmd);
	install_element(ENABLE_NODE, &upload_packet_file_slot_cmd);
	install_element(ENABLE_NODE, &upload_packet_file_slot_ex_cmd);
	install_element(ENABLE_NODE, &upload_packet_file_ex_cmd);
#ifdef TEST_SYSTEMFILE

	install_element(ENABLE_NODE,&user_permit_func_cmd);
	install_element(CONFIG_NODE,&user_permit_func_cmd);
	install_element(ENABLE_NODE,&no_user_permit_func_cmd);
	install_element(CONFIG_NODE,&no_user_permit_func_cmd);
	
	install_element(ENABLE_NODE,&ls_func_cmd);
	install_element(CONFIG_NODE,&ls_func_cmd);
	
	install_element(ENABLE_NODE,&show_file_func_cmd);
	install_element(CONFIG_NODE,&show_file_func_cmd);
	
	install_element(ENABLE_NODE,&del_file_func_cmd);
	install_element(CONFIG_NODE,&del_file_func_cmd);
	install_element(ENABLE_NODE,&exec_file_func_cmd);
	install_element(CONFIG_NODE,&exec_file_func_cmd);
#endif
	/*added by zhaocg for md5 subcommand 2012-10-30*/
	install_element(ENABLE_NODE,&md5_img_default_func_cmd);
	install_element(VIEW_NODE,&md5_img_default_func_cmd);

	install_element(ENABLE_NODE,&md5_img_slot_func_cmd);
	install_element(VIEW_NODE,&md5_img_slot_func_cmd);
	
	install_element(ENABLE_NODE,&md5_patch_func_cmd);
	install_element(VIEW_NODE,&md5_patch_func_cmd);
	
	install_element(ENABLE_NODE,&md5_patch_slot_func_cmd);
	install_element(VIEW_NODE,&md5_patch_slot_func_cmd);

	install_element(ENABLE_NODE,&md5_config_slot_func_cmd);
	install_element(VIEW_NODE,&md5_config_slot_func_cmd);
	
	/*ended by zhaocg for md5 subcommand*/
	/*added by zhaocg for fastfwd command 2012-11-20*/
	install_element(ENABLE_NODE,&show_fast_forward_slot_func_cmd);
	install_element(ENABLE_NODE,&show_fast_forward_func_cmd);
	/*ended by zhaocg for fastfwd command 2012-11-20*/
install_element(HIDDENDEBUG_NODE,&show_tipc_func_cmd);
#if 0
	install_element(CONFIG_NODE, &show_route_mvdrv_entry1_cmd);
	install_element(CONFIG_NODE, &config_rpf_cmd);
	install_element(CONFIG_NODE, &show_rpf_cmd);
	install_element(CONFIG_NODE, &show_statues_cmd);
	install_element(CONFIG_NODE, &config_arp_aging_cmd);
#endif
}


