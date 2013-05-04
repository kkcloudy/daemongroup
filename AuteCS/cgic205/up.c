
/*2010-08-18 liujk@autelan.com --for local version upgrade */
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/wait.h>
#include<sys/stat.h>
//#include "dcli_boot.h"
//#include "dcli_main.h"
//add by wangyan
#include "bsd_dbus.h"

#include "ws_init_dbus.h"
#include <dbus/dbus.h>
//#include "BsdDbusPath.h"
#define CMD_SUCCESS 0
#define PATH_LEN (64)

#define PATH_LEN 64

char BSD_DBUS_BUSNAME[PATH_LEN]=	"aw.bsd";
char BSD_DBUS_OBJPATH[PATH_LEN]=	"/aw/bsd";
char BSD_DBUS_INTERFACE[PATH_LEN]=  "aw.bsd";
char BSD_COPY_FILES_BETEWEEN_BORADS[PATH_LEN]= "bsd_copy_files_between_boards";
char BSD_GET_ALIVE_SLOT_IDS[PATH_LEN] = "bsd_get_alive_slot_ids";

DBusConnection *ccgi_dbus_connection = NULL;

#define MAX_SLOT_NUM 16
//DBusConnection *ccgi_dbus_connection= NULL;
typedef enum bsd_file_type {
    BSD_TYPE_NORMAL = 0,
    BSD_TYPE_BOOT_IMG = 1,	  	//ac img file
    BSD_TYPE_WTP = 2,			//ap img or configuration files
    BSD_TYPE_PATCH = 3,			//patch
    BSD_TYPE_CORE = 4,			//core file in /proc
	BSD_TYPE_FOLDER = 5,		//normal folder
	BSD_TYPE_SINGLE = 6,		//normal single file
	BSD_TYPE_CMD = 7,			//command
	BSD_TYPE_BLK = 8,
	BSD_TYPE_COMPRESS = 9
}bsd_file_type_t;

typedef enum{
     CK_OK,
     CK_INVALID,
     CK_NOSPACE,
     CK_CORRUPT,
     CK_FAIL,
}CHECK_STATUS;
//add by wangyan

void jmp_back(void)
{
	char *UN=getenv("QUERY_STRING");
	printf("<script type='text/javascript'>\n" );
	printf("window.location.href='wp_version_upgrade.cgi?%s';\n", UN);
	printf("</script>\n" );	

}


void show_alert(char *message_en, char *message_ch)
{	

	char *LAN=getenv("HTTP_ACCEPT_LANGUAGE");
	char *message;
	if(LAN == strstr(LAN, "zh-cn"))
		message=message_ch;
	else
		message=message_en;
	printf("<script language=JavaScript>"
			"alert(\"%s\");\n"		
			"</script>\n",
			message);
}



int check_mnt(char *fname, int fsize)/*check the version file uploaded to mnt directory, if valid then mv it to blk*/
{	
	char sys_cmd[128];
	int status;
	int ret;
	struct stat st;
	char mnt_path[128];
	CHECK_STATUS toret=CK_FAIL;

	sprintf(mnt_path, "/mnt/%s", fname);
	stat(mnt_path, &st);
	
	if(st.st_size == fsize)
	{	
//		fprintf(stderr, "file in mnt size check OK\n");
		memset(sys_cmd,0,128);
		sprintf(sys_cmd,"sudo sor.sh cp %s %d > /dev/null",fname,600);
		status = system(sys_cmd);
		ret = WEXITSTATUS(status);

		if(ret==0)
		{
//			fprintf(stderr, "file in mnt -> blk\n");
			memset(sys_cmd,0,128);
			sprintf(sys_cmd,	
					"source vtysh_start.sh >/dev/null 2>&1\n"
					"vtysh -c \"show boot_img\" |grep '^%s$' >/dev/null"
					,fname);
			
			status = system(sys_cmd);
			ret = WEXITSTATUS(status);
		
			if (ret == 0)
			{		
				// 注意SD卡是否写保护了 在保护状态下不能页面上传的
//				fprintf(stderr, "file valid and set as boot img!\n");
				//	DBusConnection *ccgi_dbus_connection= NULL;	
				//add by wangyan
				fprintf(stderr, "fname======cccccccccc================%s\n",fname);
					char src_path[PATH_LEN] = {0};
					char des_path[PATH_LEN] = {0};
					sprintf(src_path, "/blk/%s", fname);
					sprintf(des_path, "/blk/%s", fname);
	       			int ID[MAX_SLOT_NUM] = {0};
					int i =0;
					//printf("count = %d\n",board_count);
					int board_count = -1;
					 board_count =ccgi_dcli_bsd_get_slot_ids(ccgi_dbus_connection,ID,BSD_TYPE_BOOT_IMG);
					 fprintf(stderr, "board_count======================%d\n",board_count);
					for(i = 0; i < board_count; i++)
					{
						
					 ccgi_dcli_bsd_copy_file_to_board(ccgi_dbus_connection,ID[i],src_path,des_path,0,BSD_TYPE_BLK);
						
					}
				//add by wangyan
				memset(sys_cmd,0,128);
				sprintf(sys_cmd,"sudo boot.sh %s >/dev/null 2>&1",fname);
				system(sys_cmd);
				toret=CK_OK;
			}
			else
			{
//				fprintf(stderr, "file invalid and removed from blk!\n");
				memset(sys_cmd,0,128);
				sprintf(sys_cmd,"sudo sor.sh rm %s %d > /dev/null 2>&1",fname,600);
				system(sys_cmd);
				toret=CK_INVALID;

			}
		}
		else if(ret==5)
		{
			toret=CK_NOSPACE;
		}
		else
		{
//			fprintf(stderr, "sor.sh error ret=%d!\n", ret);
			toret=CK_FAIL;
		}
	}
	else
	{
		toret=CK_CORRUPT;
	}
//	fprintf(stderr, "file in mnt is removed! \n");
	sprintf(sys_cmd, "sudo rm /mnt/%s >/dev/null 2>&1", fname);
	system(sys_cmd);
	return toret;

}
//add by wangyan

int ccgi_dcli_bsd_get_slot_ids(DBusConnection *connection, int *ID, const int op)
{
	fprintf(stderr, "**********ccgi_dcli_bsd_get_slot_ids**************");
    int ret = 0;
    DBusMessageIter  iter;
    DBusMessage *query = NULL;
    DBusMessage *reply = NULL;
    DBusError err = {0};
	int i = 0;
	int slot_id = 0;
   

    query = dbus_message_new_method_call(BSD_DBUS_BUSNAME, BSD_DBUS_OBJPATH, \
        BSD_DBUS_INTERFACE, BSD_GET_ALIVE_SLOT_IDS);

    dbus_message_append_args(query,
    					DBUS_TYPE_UINT32,&op,
    					DBUS_TYPE_INVALID);

    reply = dbus_connection_send_with_reply_and_block (connection,query,-1, &err);

    dbus_message_unref(query);

    if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return -1;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	//printf("ret = %d\n",ret);

	if(ret != 0){
		for(i = 0; i < ret; i++)
		{
		    dbus_message_iter_next(&iter);
	        dbus_message_iter_get_basic(&iter,&(ID[i]));
		}
	}

    return ret;
}
//add by wangyan

int ccgi_dcli_bsd_copy_file_to_board(DBusConnection *connection, const int slot_id, const char *src_path, const char *des_path, const int flag, const int op)
{   
    int ret = 0;
    DBusMessage *query = NULL;
    DBusMessage *reply = NULL;
    DBusError err = {0};
	int retu = 0;
    fprintf(stderr, "**********ccgi_dcli_bsd_copy_file_to_board**************");
    char *tmp_src_path = src_path;
    char *tmp_des_path = des_path;
    
    //printf("dbus_name = %s\ndbus_objpath = %s\ndbus_interface = %s\n",BSD_DBUS_BUSNAME,BSD_DBUS_OBJPATH,BSD_DBUS_INTERFACE);
    query = dbus_message_new_method_call(BSD_DBUS_BUSNAME,BSD_DBUS_OBJPATH,\
						BSD_DBUS_INTERFACE,BSD_COPY_FILES_BETEWEEN_BORADS);
    
	dbus_error_init(&err);
    //printf("11111\n");
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&slot_id,
							 DBUS_TYPE_STRING,&tmp_src_path,
							 DBUS_TYPE_STRING,&tmp_des_path,
							 DBUS_TYPE_UINT32,&flag,
							 DBUS_TYPE_UINT32,&op,
							 DBUS_TYPE_INVALID);
    
   // printf("Copying file, please wait ...\n");
    //printf("0000\n");
	reply = dbus_connection_send_with_reply_and_block (connection,query,120000, &err);
	//printf("0001\n");
	dbus_message_unref(query);
	//printf("2222\n");
	if (NULL == reply) {
		printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_SUCCESS;
	}
	//printf("3333\n");
	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&ret,
					DBUS_TYPE_INVALID)) {
		
	}
	//printf("4444\n");
	dbus_message_unref(reply);
    
    return ret;
}
//add by wangyan

int main(int argc,char* argv[])
{
     printf("ContentType:text/html\r\n");
     printf("Cache-Control:private,max-age=0;\r\n");
     printf("\r\n");
	ccgi_dbus_init();

    char *p;
	int ret;
	int fSize=0;
    char updir[256]="/mnt/";
	FILE *f = NULL;
    if(getenv("CONTENT_TYPE"))
	{
        if(!strncmp(getenv("CONTENT_TYPE"),"multipart/form-data",19))
		{
            //get boundary
            char boundary[64]="--";
             p=strstr(getenv("CONTENT_TYPE"),"boundary=");
             strcpy(boundary+2,p+9);

            //get content length
            int len=atoi(getenv("CONTENT_LENGTH"));
            //read down to last file field
            char str[512];
            char fname[256];
            int pos=0;
            while(1)
			{
                if(fgets(str,512,stdin))
				{
                     pos+=strlen(str);
                    if(strstr(str,"filename="))
					{
                        //get filename
                         p=strstr(str,"filename=");
                         strcpy(fname,p+9);
                        if(0 == strcmp(fname,"\"\"\r\n"))
						{
							goto nofile;
						}
						
						p=strrchr(fname, '\\')+1;
						if(NULL+1 != p)//for ie6's full path.
						{
                        	memmove(fname, p, strlen(p));
                         	p=strchr(fname,'\"');
                        	*p='\0';
							fprintf(stderr, "base fname in fullpath=%s", fname);
						}
						else//for ie8's fake path.
						{
							memmove(fname, fname+1, strlen(fname+1));
                         	p=strchr(fname,'\"');
                        	*p='\0';
							fprintf(stderr, "base fname in fakepath=%s", fname);

						}
							
                        break;
                     }
                 }
             }

             fgets(str,512,stdin);
             pos+=strlen(str);
             fgets(str,512,stdin);
             pos+=strlen(str);
            fSize=len-pos-strlen(boundary)-6;
            int bSize=65536;
            //open file for writing
            char fpath[256]="";
             strcat(fpath,updir);
             strcat(fpath,fname);

            f=fopen(fpath,"wb");
            if(!f)
			{
				goto fail;
			}
            //chunk read
            int chunk=fSize/bSize;
            int lbSize=fSize%bSize;
            void* buf=malloc(bSize);
            int i;
            for(i=0;i<chunk;i++)
			{
                if(fread(buf,bSize,1,stdin))
				{
                    if(!fwrite(buf,bSize,1,f))
					{
						goto fail;
					}
                 }
				else
				{
                  goto fail;
                }
             }

            free(buf);
            if(lbSize)
			{
                 buf=malloc(lbSize);
                if(fread(buf,lbSize,1,stdin))
				{
                    if(!fwrite(buf,lbSize,1,f))
					{
						goto fail;
					}
                 }
				else
				{
                   goto fail;
                }
                 free(buf);
             }
			
            if(fflush(f)){goto fail;}
			
            fclose(f);
			f = NULL;
			fprintf(stderr,"fname================%s\n",fname);
            ret=check_mnt(fname, fSize);
			if (CK_OK == ret)
				goto done;
			else if(CK_INVALID == ret)
				goto invalid;
			else if(CK_NOSPACE == ret)
				goto nospace;
			else if (CK_CORRUPT == ret)
				goto corrupt;
			else
				goto fail;
         }
     }
    else    
		goto login;
    

//show alerts and exit.

	
	done:
		show_alert("Local upload succeed!", 
					"本地上传成功!");
		jmp_back();
    	return 0;
		
	fail:
		show_alert( "Error: Internal error, upload failed!",
					"错误: 内部错误, 上传失败!");
		if(f != NULL)
		{
			fclose(f);
		}
		jmp_back();
    	return -1;
		
	nofile:
		show_alert( "Error: No file to upload, please check the file path!",
					"错误: 没有文件上传, 请检查文件路径!");
		jmp_back();
		return -2;
		
	nospace:
		show_alert( "Error: No space left on device, please remove useless files and try again!",
					"错误: 存储空间不足, 请删除无用文件后重试!");
		jmp_back();
		return -3;
		
	invalid:
		show_alert( "Error: Invalid version file, please make sure the file to upload is valid!",
					"错误: 无效的版本文件, 请确保要上传的版本的可用性!");
		jmp_back();
		return -4;
		
	corrupt:
		show_alert( "Error: File corrupted during transfer, and was rejected by device for protection!",
					"错误: 文件在传输过程中损坏, 为了安全起见已被设备拒绝!");
		jmp_back();
		return -5;
		
	login://防止直接在地址栏引用up.cgi
		printf("<SCRIPT language=javascript>if(top==self)top.location=\"wp_login.cgi\"</SCRIPT> ");
		return -6;

}
		

