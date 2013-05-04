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
* dcli_spacial.c
*
*
* CREATOR:
*		dingkang@autelan.com
*
* DESCRIPTION:
*		CLI definition for spacial config module.
*
* DATE:
*		05/26/2010	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.5 $	
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif
#include <zebra.h>
#include <stdlib.h>
#include <dbus/dbus.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "command.h"
#define SPACIAL_CONFIG_FILE_PATH "/var/run/setup_file"
#define SPACIAL_CONFIG_COMMAND_MAX 64

//spacial node//
struct cmd_node spacial_config_node =
{
  SPACIAL_CONFIG_NODE,
  "%s(spacial-config)# ",
  1
};
#if 0
void insert(char *dtr, char *str, int locat)
{
	dtr+=locat;
	locat=0;
	while(*str)
		{
		while(*str)
			{
			*dtr^=*str;
			*str^=*dtr;
			*dtr^=*str;
			str++;
			dtr++;
			locat++;
			}
	str-=locat;
	locat=0;
	}
}

#endif
//special config

DEFUN (spacial_config,
       spacial_config_cmd,
       "spacial config",
       "Enable a spacial config node\n"
       "Start spacial config\n")
{
  vty->node = SPACIAL_CONFIG_NODE;
  return CMD_SUCCESS;
}

DEFUN (spacial_config_setup,
       spacial_config_setup_cmd,
       "config .LINE",
       "Config a command\n"
       "Config command\n")
{
	int ret = CMD_SUCCESS;
	char cmd[256];
	char* temp[64];
	char* location;
	location = argv_concat(argv, argc, 0);
	if(strlen(location)>SPACIAL_CONFIG_COMMAND_MAX)
		{
		vty_out(vty,"command is too long,please do not more than %d\n",SPACIAL_CONFIG_COMMAND_MAX);
		return CMD_WARNING;
		}
	memset(temp,0,64);
	memset(cmd,0,256);
	strcpy(temp,"config ");
	sprintf(cmd,"touch %s >/dev/null 2>/dev/null;echo \'%s\' >>%s",SPACIAL_CONFIG_FILE_PATH,strcat(temp,location),SPACIAL_CONFIG_FILE_PATH);
	ret = system (cmd);
	ret = WEXITSTATUS(ret);
	if(0!=ret)
		return CMD_WARNING;
	return ret;
  
}
DEFUN (spacial_config_no_setup,
       spacial_config_no_setup_cmd,
       "no config",
       "Delete a setup command\n")
{
	int ret = CMD_SUCCESS;
	char cmd[256];
	memset(cmd,0,256);
#if 0
	FILE* fp;
	void* data;
	struct stat sb;
	char* temp[64];
	char* temp_data;
	if(0==argv)
		{
		ret=system("rm /var/run/setup_file -f>/dev/null 2>/dev/null");
		if(0!=ret)
			return CMD_WARNING;
		return ret;
		}
	if(strlen(argv[0])>32)
		{
		vty_out(vty,"command is too long,please do not more than 32\n");
		return CMD_WARNING;
		}
	memset(temp,0,64);
	strcpy(temp,"setup ");
	fp=open("/var/run/setup_file",O_RDWR);
	if(NULL==fp)
		{
		vty_out(vty,"open setup file error\n");
		return CMD_WARNING;
		}
	fstat(fp,&sb);
	data=mmap(NULL,sb.st_size,PROT_READ|PROT_WRITE,MAP_SHARED,fp,0);
	if(MAP_FAILED==data)
		{
		printf("can't mmap \n");
		return CMD_WARNING;
		}
	temp_data=strstr((char*)data,strcat(temp,argv[0]));
	if(NULL==temp_data && '\n'!=*(temp_data+strlen(argv[0])+strlen("setup ")))
		{
		vty_out(vty,"can not find command\n");
		return CMD_WARNING;
		}
	memcpy(temp_data,temp_data+strlen(argv[0])+strlen("setup "),sb.st_size-(((int)data-(int)temp_data)+strlen(argv[0])+strlen("setup ")+1));
	munmap(data,sb.st_size);
	close(fp);
	return ret;
#else
	sprintf(cmd,"rm %s -f>/dev/null 2>/dev/null",SPACIAL_CONFIG_FILE_PATH);
	ret=system(cmd);
	ret = WEXITSTATUS(ret);
	if(0!=ret)
		return CMD_WARNING;
	return ret;

 #endif
}


int dcli_spacial_config_show_running_config(struct vty* vty) {	
	int i,m;
	char _tmpstr[64];
	char *temp;
	char *temp_char;
	FILE* fp;
	void* data;
	char* data_temp;
	char* p_temp;
	struct stat sb;
	
	p_temp=data_temp=malloc(1024);
	memset(data_temp,0,1024);
	memset(_tmpstr,0,64);
	sprintf(_tmpstr,BUILDING_MOUDLE,"SPACIAL CONFIG");
	vtysh_add_show_string(_tmpstr);
	fp=open(SPACIAL_CONFIG_FILE_PATH,O_RDWR);
	if(NULL==fp)
		{
		
		free(data_temp);
		return 0;
		}
	fstat(fp,&sb);
	data=mmap(NULL,sb.st_size,PROT_READ,MAP_PRIVATE,fp,0);
	if(MAP_FAILED==data)
		{
		
		free(data_temp);
		close(fp);
		return 0;
		}
	vtysh_add_show_string("spacial config");
	strcpy(data_temp," ");
	p_temp++;
	for(i=m=0;i<sb.st_size;i++)
		{
		if((*((char*)data+i))=='\n')
			{
			memcpy(p_temp,data+m,(i-m)+1);
			strcat(data_temp," ");
			p_temp+=(i-m)+2;
			m=i+1;
			}
		}
	
	temp_char=strrchr(data_temp,'\n');
	*temp_char='\0';
#if 0
	for(i=0;i<sb.st_size;i++)
		{
		if((*((char*)data+i))=='\n')
			{
			vty_out(vty,"insert a space %d\n",i+1);
			insert(data,temp,i+1);
			}
		}
	temp_char=strrchr(data,'\n');
	*temp_char='\0';
#endif
	vtysh_add_show_string(data_temp);
	vtysh_add_show_string(" exit");
	close(fp);
	free(data_temp);
	munmap(data,sb.st_size);
	return 0;
}

void dcli_spacial_config_init(){
	install_node (&spacial_config_node,dcli_spacial_config_show_running_config,"SPACIAL_CONFIG_NODE");
	install_default (SPACIAL_CONFIG_NODE);
	install_element (SPACIAL_CONFIG_NODE,&spacial_config_setup_cmd);
	install_element (SPACIAL_CONFIG_NODE,&spacial_config_no_setup_cmd);	
	install_element (CONFIG_NODE,&spacial_config_cmd);
}
#ifdef __cplusplus
}
#endif
