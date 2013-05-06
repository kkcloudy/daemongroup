
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "file_transe.h"
#include <sys/stat.h>
#include <unistd.h>


static const char *protocal_key[] = {
	"",
	"tftp",
	"ftp",
	"sftp",
	"http",
	"https"
};

struct file_trans_t{
	LOAD_FLAG 				loadFlag;
	TRANS_FLAG				transFlag;
	TRANS_PROTOCAL			protocal; 
	TRANS_STATUS			status;
	ON_TRANS_FINISH			onFinish;	
	unsigned int			port;
	char					*ipAddr;
	char					*srvPath;
	char					*userPath;
	char					*userName;
	char					*passWord;	
	char					*failedReason;
	
};

/*create file_trans*/
file_trans_t *file_trans_create()
{
	file_trans_t	*file_trans_tp = NULL;
	file_trans_tp = (file_trans_t *)calloc(1, sizeof(file_trans_t));
	file_trans_tp->failedReason=(char*)calloc(MAX_LEN, 1);

	return file_trans_tp;
}

/*destroy  file_trans*/
int file_trans_destroy( file_trans_t *me )
{
	if (NULL == me || NULL == me->failedReason)
		return FT_POINTER_NULL;
	free(me->failedReason);
	free(me);
	return FT_OK;
}
/*other function*/
int ft_set_loadflag( file_trans_t *me, LOAD_FLAG flag)
{
	if (NULL == me || NULL == me->failedReason)
		return FT_POINTER_NULL;
	
	me->loadFlag = flag;
	return FT_OK;
}
	
int ft_set_transflag (file_trans_t *me, TRANS_FLAG transflag)
{
	if (NULL == me || NULL == me->failedReason)
		return FT_POINTER_NULL;

	me->transFlag=transflag;
	return FT_OK;
}

int ft_set_protocal( file_trans_t *me, TRANS_PROTOCAL portal)
{
	if (NULL == me || NULL == me->failedReason)
		return FT_POINTER_NULL;
	
	me->protocal = portal;
	return FT_OK;
}
int ft_set_ipaddr  ( file_trans_t *me, char *ipaddr )

{
	if (NULL == me || NULL == me->failedReason)
		return FT_POINTER_NULL;
	
	me->ipAddr = ipaddr;
	return FT_OK;
}
int ft_set_port    ( file_trans_t *me, unsigned int port )
{
	if (NULL == me || NULL == me->failedReason)
		return FT_POINTER_NULL;
		
	me->port = port;
	return FT_OK;
}


int ft_set_srvpath( file_trans_t *me, char *srvpath)
{
	if (NULL == me || NULL == me->failedReason)
		return FT_POINTER_NULL;
	
	me->srvPath = srvpath;
	return FT_OK;
}

int ft_set_userpath(file_trans_t *me, char *userpath)
{
	if (NULL == me || NULL == me->failedReason)
		return FT_POINTER_NULL;

	me->userPath = userpath;
	return FT_OK;
}

int ft_set_username( file_trans_t *me, char *username )
{
	if (NULL == me || NULL == me->failedReason)
		return FT_POINTER_NULL;
		
	me->userName = username;
	return FT_OK;
}

int ft_set_password( file_trans_t *me, char *password )
{
	if (NULL == me || NULL == me->failedReason)
		return FT_POINTER_NULL;
			
	me->passWord= password;
	return FT_OK;
}

int ft_set_onfinish( file_trans_t *me, ON_TRANS_FINISH on_finish )
{
	if (NULL == me || NULL == me->failedReason)
		return FT_POINTER_NULL;
			
	me->onFinish = on_finish;
	return FT_OK;
}


TRANS_STATUS ft_get_status	( file_trans_t *me )
{
	int size_then=0;
	int size_now=0;
	char sor_ret[MAX_LEN]="";
	char command[MAX_LEN]="";
	char *file_name=NULL;
	FILE *fp_sor=NULL;
	
	if (NULL==me || NULL==me->failedReason)
		return FAILED;

	if(TRANSFERRING == me->status){
		size_then = ft_get_curent_trans_size(me);
		if(size_then < 0)
			return FAILED;
		usleep(100000);																					//间隔0.1秒
		size_now = ft_get_curent_trans_size(me);
		if(size_now < 0)
			return FAILED;		
		if(size_then == size_now){
			if(0 == strncmp(me->userPath, "/blk/", 5)){													//SD卡的文件上传下载通过/mnt/目录中转
				if(DOWNLOAD == me->loadFlag){//DOWNLOAD
					file_name = strrchr(me->srvPath, '/')+1;
					if(NULL+1 == file_name)
						file_name = me->srvPath;
					snprintf(command, MAX_LEN-1,
							"sor.sh cp %s 15 >/dev/null\n"
							"echo -n $? >/var/run/sor_log",
							file_name);

					system(command);
				
					fp_sor = fopen("/var/run/sor_log", "r");
					if(NULL == fp_sor)
						return FAILED;
					fread(sor_ret, 1, MAX_LEN-1, fp_sor);
					fclose(fp_sor);	
					if(0 != strcmp(sor_ret, "0")){														//sor.sh返回非0，从mnt到blk转移文件失败
						snprintf(me->failedReason, MAX_LEN-1, "Download: sor.sh failed.(%s)\n", sor_ret);
						return FAILED;
					}
					
					snprintf(command, MAX_LEN-1, "rm /mnt/%s", file_name);
					system(command);

				}
				
				else{//UPLOAD，删除中转的临时文件
					file_name = strrchr(me->userPath, '/')+1;
					snprintf(command, MAX_LEN-1, "rm /mnt/%s", file_name);
					system(command);
				}
				
			}
			
			me->status=COMPLETED;
			
		}		
	}
	
	return me->status;
	
}

char *ft_get_failedreason( file_trans_t *me )
{
	if (NULL == me || NULL == me->failedReason)
		return NULL;
	return me->failedReason;
}


int  ft_get_curent_trans_size( file_trans_t *me )
{	
	struct stat st;
	FILE *fp_size=NULL;
	char *file_name=NULL;
	char trans_path[MAX_LEN]="";
	char command[MAX_LEN]="";
	char up_size[MAX_LEN]="";

	if(FAILED == me->status)
		return FT_ERR;
	
	if (DOWNLOAD == me->loadFlag){
		
		system("sudo mount /blk >/dev/null 2>&1");												//若文件已经从/mnt移动到/blk 则去/blk中读文件大小
		
		file_name=strrchr(me->srvPath,'/')+1;
		if(NULL+1 == file_name)
			file_name=me->srvPath;		

		if(0==strncmp(me->userPath, "/blk/", 5) && TRANSFERRING==me->status)
				snprintf(trans_path, MAX_LEN-1, "/mnt/%s", file_name);	
		else
			snprintf(trans_path, MAX_LEN-1, "%s%s", me->userPath, file_name);
		
		stat(trans_path, &st);		
		system("sudo umount /blk >/dev/null 2>&1");
		return st.st_size;
	}
	
	else{
		file_name=strrchr(me->userPath,'/')+1;
		if(NULL+1 == file_name)
			file_name=me->userPath;
		snprintf(trans_path, MAX_LEN-1, "%s%s", me->srvPath, file_name);
		snprintf(command, MAX_LEN-1,
				"ftp -q 3 ftp://%s:%s@%s:%d/ <<! |grep -v EPSV |awk '{printf $5}' >/var/run/ftp_size_log\n"				
				"ls %s\n"
				"bye\n"
				"!",
				me->userName, me->passWord, me->ipAddr, me->port, trans_path);		
		system(command);   																		//取得大小字段输出到ftp_size_log 中
		
		fp_size=fopen("/var/run/ftp_size_log", "r");
		if(NULL == fp_size)
			return FT_ERR;
		fread(up_size, 1, MAX_LEN-1, fp_size);
		fclose(fp_size);		
		return atoi(up_size);
	}
	
}

/*	上传时本地路径必须为文件路径,目标路径必须为目录路径;				*
*	本地目录可用相对或绝对路径且不可以"/"结尾或为空;					*
*	远程目录为相对于/home/somebody/的路径,需要以"/"结尾且不为空;			*
*************************************************************************************************
*	下载时本地路径须为目录,服务器路径为文件路径;							*
*	本地目录可用相对或绝对路径且以"/"结尾;									*
*	远程目录为相对于/home/somebody/的路径且不可以"/"开头或结尾或为空	*
*	最好用./开头,代表/home/somebody/;												      */
																								
int ft_do_transfers( file_trans_t *me)
{

	int	 file_size=0;
	char command[MAX_LEN]="";
	char trans_path[MAX_LEN]="";																//传输后的文件的路径
	char user_dir[MAX_LEN]="";
	char srv_dir[MAX_LEN]="";
	char blk_tmp[MAX_LEN]="";
	char *file_name=NULL;
	char *epsv=NULL;
	FILE *fp_err=NULL;

	if (NULL==me)
		return FT_POINTER_NULL;	
	if(NULL!=strchr(me->userPath, '~') || NULL!=strchr(me->srvPath, '~')){					//fopen不能识别~字符
		strcpy( me->failedReason, "Invalid charactor \"~\" in file path.\n");
		me->status=FAILED;
		return FT_ERR;
	}
	
	switch (me->protocal){
	case PROTOCAL_FTP:		
		if(DOWNLOAD == me->loadFlag){/*FTP下载操作*/
			if(0 == strcmp(me->userPath, "/blk/"))											//下载到SD卡的文件通过/mnt/目录中转
				strcpy(me->userPath,"/mnt/");
			else if(0 < strcmp(me->userPath, "/blk/")){
				strcpy(me->failedReason, "Download: Only /blk/ directory supported.\n");
				me->status=FAILED;
				return FT_ERR;
			}

			if(	0 == strlen(me->userPath) ||
				0 == strlen(me->srvPath) || 
				'/' == me->srvPath[0] ||
				'/' != me->userPath[strlen(me->userPath)-1] ||					
				'/' == me->srvPath[strlen(me->srvPath)-1]){
				
				strcpy(me->failedReason,"Download: Local path must end with \"/\"; "
										"remote path can not end or start with \"/\" or be null.\n");
				me->status=FAILED;
				return FT_ERR;
			}			

			snprintf(command, MAX_LEN-1, "cd %s >/dev/null 2>&1", me->userPath);	
			if (256 == system(command)){													//system()返回值256代表cd命令出错
				strcpy(me->failedReason, "No such local directory or not a directory.\n");	//防止文件错下载到当前目录
				me->status=FAILED;
				return FT_ERR;
			}

			file_name=strrchr(me->srvPath,'/')+1;
			if(NULL+1 == file_name)															//strrchr返回NULL.
				file_name=me->srvPath;
			snprintf(trans_path, MAX_LEN-1, "%s%s", me->userPath, file_name);
			snprintf(command, MAX_LEN-1, "ls %s >/dev/null 2>&1", trans_path);
			snprintf(srv_dir, file_name - me->srvPath, "%s", me->srvPath);
			if (0 == system(command)){//本地文件已存在,续传
				me->status = TRANSFERRING;
				if (TRANS_FLAG_BACKGROUND == me->transFlag)
					snprintf(command, MAX_LEN-1,
							"cd %s\n"
							"ftp -q 3 ftp://%s:%s@%s:%d/ <<! >/var/run/ftp_err_log 2>&1 &\n"
							"cd %s\n"
							"reget %s\n"
							"bye\n"
							"!",
							me->userPath, me->userName, me->passWord, me->ipAddr, me->port, srv_dir, file_name);

				else 
					snprintf(command, MAX_LEN-1,
							"cd %s\n"
							"ftp -q 3 ftp://%s:%s@%s:%d/ <<! >/var/run/ftp_err_log 2>&1\n"
							"cd %s\n"
							"reget %s\n"
							"bye\n"
							"!",
							me->userPath, me->userName, me->passWord, me->ipAddr, me->port, srv_dir, file_name);

			}				

			else{//文件不存在,初次下载.
				snprintf(command, MAX_LEN-1, "cd %s", me->userPath); 
				if (256 == system(command)){												//若不判断目录是否存在则会将文件下载到当前目录
					strcpy(me->failedReason, "No such local directory or not a directory!\n");				
					me->status=FAILED;
					return FT_ERR;
				}
				
				snprintf(command, MAX_LEN-1,						
						"cd %s\n"
						"ftp -q 3 ftp://%s:%s@%s:%d/%s >/var/run/ftp_err_log 2>&1",
						me->userPath, me->userName, me->passWord, me->ipAddr, me->port, me->srvPath);
				
				if (TRANS_FLAG_BACKGROUND == me->transFlag)
					strncat(command, " &", MAX_LEN-1);
			}
			
			if(0==strcmp(me->userPath, "/mnt/"))
				strcpy(me->userPath, "/blk/");
			
			me->status = TRANSFERRING;
			system(command);																//执行传输命令
			
			if (TRANS_FLAG_BACKGROUND == me->transFlag) 									//若连接超时,得超时后ftp_err_log才有数据,这个数字要大于超时数字
				sleep(4);
			fp_err = fopen("/var/run/ftp_err_log", "r"); 									//读取ftp错误日志中的信息.
			if(NULL == fp_err)
			{
				strcpy(me->failedReason, "Can not open ftp_err_log.\n");
				me->status = FAILED;
				return FT_ERR;
			}
			fread(me->failedReason, 1, MAX_LEN-1, fp_err);
			fclose(fp_err);

			if(0 != strcmp(me->failedReason, "'EPSV': command not understood.\n")){			//ftp_err_log有且仅有此串说明没有错误					
				epsv=strstr(me->failedReason, "'EPSV': command not understood.\n");
				if(NULL!=epsv){																//去掉错误信息中的"'EPSV': command not understood."	使错误信息更清晰
					if(me->failedReason == epsv)
						memmove(epsv, epsv+strlen("'EPSV': command not understood.\n"),
								1+strlen(me->failedReason)-strlen("'EPSV': command not understood.\n"));
						
					else
						*epsv='\0';
					
				}
				me->status=FAILED;
				return FT_ERR;
			}
			
			strcpy(me->failedReason, "\n");
			return FT_OK;

		}
			
		
		else{/*FTP上传操作*/

			if (0==strlen(me->userPath)||
				0==strlen(me->srvPath)||										
				'/'==me->srvPath[0]||					
				'/'!=me->srvPath[strlen(me->srvPath)-1]||
				'/'==me->userPath[strlen(me->userPath)-1]){
				strcpy( me->failedReason,
						"Upload: Local path can not end with \"/\" or be null; "
						"remote path must end with \"/\", and can not start with \"/\" or be null.\n");
				me->status=FAILED;
				return FT_ERR;
			}

			file_name=strrchr(me->userPath,'/')+1;											//本地文件只写文件名则默认为当前目录下.
 			if (NULL+1 == file_name){
				file_name=me->userPath;
				strcpy(user_dir, "./");
			}	
			snprintf(user_dir, file_name - me->userPath, "%s", me->userPath);

			if(0 == strncmp(me->userPath, "/blk/", 5)){										//从SD卡上传的文件通过/mnt/目录中转
				snprintf(blk_tmp, MAX_LEN-1, "%s", me->userPath);					
				snprintf(command, MAX_LEN-1,
						"sudo mount /blk >/dev/null 2>&1\n"
						"cp %s /mnt/ >/dev/null 2>&1\n"
						"sudo umount /blk >/dev/null 2>&1",
						me->userPath);
				system(command);
				snprintf(me->userPath, MAX_LEN-1, "/mnt/%s", file_name);
				strcpy(user_dir, "/mnt/");
			}

			snprintf(command, MAX_LEN-1, "ls %s >/dev/null 2>&1", me->userPath);	
			if (512 == system(command)){													//system()返回值512代表ls命令出错
				strcpy(me->failedReason, "No such local file or directory.\n");	
				me->status=FAILED;
				return FT_ERR;
			}

			file_size=ft_get_curent_trans_size(me);
			if(file_size>0){																//远程文件已存在,续传

				if (TRANS_FLAG_BACKGROUND == me->transFlag)
					snprintf(command, MAX_LEN-1,
							"cd %s\n"
							"ftp -q 3 ftp://%s:%s@%s:%d/ <<! >/var/run/ftp_err_log 2>&1 &\n"
							"cd %s\n"
							"restart %d\n"
							"put %s\n"
							"bye\n"
							"!",
							user_dir, me->userName, me->passWord, me->ipAddr, me->port, me->srvPath, file_size, file_name);
				else
					snprintf(command, MAX_LEN-1,
							"cd %s\n"
							"ftp -q 3 ftp://%s:%s@%s:%d/ <<! >/var/run/ftp_err_log 2>&1\n"
							"cd %s\n"
							"restart %d\n"
							"put %s\n"
							"bye\n"
							"!",
							user_dir, me->userName, me->passWord, me->ipAddr, me->port, me->srvPath, file_size, file_name);

			}		
			
			else{ 																			//初次上传新文件
				snprintf(command, MAX_LEN-1,
						"cd %s\n"
						"ftp -q 3 -u ftp://%s:%s@%s:%d/%s %s >/var/run/ftp_err_log 2>&1",
						user_dir, me->userName, me->passWord, me->ipAddr, me->port, me->srvPath, file_name);
				if (TRANS_FLAG_BACKGROUND == me->transFlag)
					strncat(command, " &", MAX_LEN-1);
			}

			if(0 != strlen(blk_tmp))
				snprintf(me->userPath, MAX_LEN-1, "%s", blk_tmp);

			me->status = TRANSFERRING;	
			system(command);																//执行输入命令
			
			if (TRANS_FLAG_BACKGROUND == me->transFlag) 									//若连接超时,得超时后ftp_err_log才有数据,这个数字要大于超时数字
				sleep(4);
			fp_err = fopen("/var/run/ftp_err_log", "r"); 									//读取ftp错误日志中的信息.
			if(NULL == fp_err){
				strcpy(me->failedReason, "Can not open ftp_err_log.\n");
				me->status=FAILED;
				return FT_ERR;
			}
			fread(me->failedReason, 1, MAX_LEN-1, fp_err);
			fclose(fp_err);

			epsv=strstr(me->failedReason, "'EPSV': command not understood.\n");				
			if(0 != strcmp(me->failedReason, "'EPSV': command not understood.\n") &&		//ftp_err_log有且仅有此串说明没有错误		
				(NULL==strstr(me->failedReason, "Restarting") || NULL==epsv)){				//同时有这两个串也没有传输错误
								
				me->status=FAILED;					

				if(NULL!=epsv){																//去掉错误信息中的"'EPSV': command not understood."	使错误信息更清晰
					if(me->failedReason == epsv)
						memmove(epsv, epsv+strlen("'EPSV': command not understood.\n"),
								1+strlen(me->failedReason)-strlen("'EPSV': command not understood.\n"));						
					else
						*epsv='\0';					
				}
				return FT_ERR;
			}

			strcpy(me->failedReason, "\n");													//没有出现传输错误,错误信息为空
			return FT_OK;

		}
		break;
		
		
	/*other switch cases*/
	default:
		return FT_ERR;
	}
	
}




/*for test only!!!!!!*/
/*
int main()
{	
	file_trans_t *pft = NULL;
	char ipaddr[MAX_LEN]="192.168.1.18";
	unsigned int	 port=21;
	TRANS_PROTOCAL	protocal=PROTOCAL_FTP;
	char username[MAX_LEN]="liujikun";
	char password[MAX_LEN]="123456";
	char userpath[MAX_LEN]="/blk/";
	char srvpath[MAX_LEN]="ljk/MIB5.exe";
	LOAD_FLAG 		loadFlag=DOWNLOAD;
	TRANS_FLAG		transFlag=TRANS_FLAG_BACKGROUND;
	
	pft = file_trans_create();
	
	if( NULL == pft )	
		return -1;	
	
	ft_set_ipaddr(pft, ipaddr);
	ft_set_loadflag(pft, loadFlag);
	ft_set_password(pft, password);
	ft_set_port(pft, port);
	ft_set_protocal(pft, protocal);
	ft_set_srvpath(pft, srvpath);
	ft_set_transflag(pft, transFlag);
	ft_set_username(pft, username);
	ft_set_userpath(pft, userpath);
	ft_do_transfers(pft);
	printf("*****wait 4s if Background******\n");	
	printf("CurrentSize=%d\n", ft_get_curent_trans_size(pft));
	printf("CurrentStatus:%d\n", ft_get_status(pft));
	printf("FailedReason:%s",ft_get_failedreason(pft));
	
	while(ft_get_status(pft)==TRANSFERRING);
	
	printf("EndSize:%d\n",ft_get_curent_trans_size(pft));
	printf("EndStatus:%d\n", ft_get_status(pft));
	printf("FailedReason:%s",ft_get_failedreason(pft));

	file_trans_destroy( pft );
	
	return 0;
}
*/

