#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <fcntl.h>
#include <net/if_arp.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/uio.h>
#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <pthread.h>
#include <dirent.h>
#include "iuh/Iuh.h"
#include "Iuh_License.h"

LICENSE_TYPE **g_hnb_count;


int parse_int_ID(char* str,unsigned int* ID){
	char *endptr = NULL;
	char c;
	c = str[0];
	if (c>='0'&&c<='9'){
		*ID= strtoul(str,&endptr,10);
		if((c=='0')&&(str[1]!='\0')){
			 return -1;
		}
		else if((endptr[0] == '\0')||(endptr[0] == '\n')){
			return 0;
		}
		else{
			return -1;
		}
	}
	else{
		return -1;
	}
}


int read_ac_info(char *FILENAME,char *buff)
{
	int len,fd;
	
	fd = open(FILENAME,O_RDONLY);
	if (fd < 0) {
		return 1;
	}	
	len = read(fd,buff,DEFAULT_LEN);	
	if (len < 0) {
		close(fd);
		return 1;
	}	
	close(fd);
	return 0;
}

int get_dir_wild_file_count(char *dir, char *wildfile)
{
	DIR *dp = NULL;
	struct dirent *dirp;
	int wildfilecount = 0;
	dp = opendir(dir);
	if(dp == NULL)
	{
		return wildfilecount;
	}
	while((dirp = readdir(dp)) != NULL)
	{
		if((memcmp(dirp->d_name,wildfile,strlen(wildfile))) ==  0)
		{
			wildfilecount++;
		}
	}
	closedir(dp);
	return wildfilecount;
}

int read_file_info(char *FILENAME,char *buff)
{
	int len,fd;
	
	fd = open(FILENAME,O_RDONLY);
	if (fd < 0) {
		return 1;
	}	
	len = read(fd,buff,DEFAULT_LEN);	
	
	if (len < 0) {
		close(fd);
		return 1;
	}
	if(len != 0)
	{
		if(buff[len-1] == '\n')
		{
			buff[len-1] = '\0';
		}
	}
	close(fd);
	return 0;
}

void Iuh_License_Init(){
	//int licensecount = 0;
	int i = 0;
	char HNB_COUNT_PATH_BASE[] = "/devinfo/maxhnbcount";
	char buf_base[DEFAULT_LEN];
	char strdir[DEFAULT_LEN];
	char Board_Slot_Path[] = "/dbm/local_board/slot_id";
	int slot_id = 0;
	char tmpbuf[128] = {0}; 
	memset(tmpbuf, 0, 128);
	if(read_file_info(Board_Slot_Path,tmpbuf) == 0){
		if(parse_int_ID(tmpbuf,&slot_id) == 0){
			slotid = slot_id;
		}	
	}

	glicensecount = get_dir_wild_file_count("/devinfo","maxhnbcount");

	if(glicensecount == 0)
	{
		/*xiaodawei modify, 20101029*/
		g_hnb_count = malloc(sizeof(LICENSE_TYPE *));
		g_hnb_count[0] = malloc(sizeof(LICENSE_TYPE));
		g_hnb_count[0]->gcurrent_hnb_count = 0;
		g_hnb_count[0]->gmax_hnb_count = HNB_DEFAULT_NUM_AUTELAN;
		g_hnb_count[0]->flag = 0;
		glicensecount = 1;
	}
	else
	{
		/*xiaodawei modify, 20101029*/
		g_hnb_count = malloc(glicensecount*(sizeof(LICENSE_TYPE *)));
		
		for(i=0; i<glicensecount; i++)
		{
			g_hnb_count[i] = malloc(sizeof(LICENSE_TYPE));
			g_hnb_count[i]->gcurrent_hnb_count = 0;
			g_hnb_count[i]->flag = 0;
			
			memset(strdir,0,DEFAULT_LEN);
			memset(buf_base,0,DEFAULT_LEN);	

			if(i == 0)
			{
				if(read_ac_info(HNB_COUNT_PATH_BASE,buf_base) == 0)
				{
					if(parse_int_ID(buf_base, &g_hnb_count[i]->gmax_hnb_count)==-1)
					g_hnb_count[i]->gmax_hnb_count = HNB_DEFAULT_NUM_AUTELAN;
				}
				else
				{

					g_hnb_count[i]->gmax_hnb_count = HNB_DEFAULT_NUM_AUTELAN;
				}
	
			}
			else
			{
				sprintf(strdir,"/devinfo/maxwtpcount%d",i+1);
				if(read_ac_info(strdir,buf_base) == 0)
				{
					if(parse_int_ID(buf_base, &g_hnb_count[i]->gmax_hnb_count)==-1)
					g_hnb_count[i]->gmax_hnb_count = HNB_DEFAULT_NUM_OEM;
				}
				else
				{
					g_hnb_count[i]->gmax_hnb_count = HNB_DEFAULT_NUM_OEM;
				}
			}

			printf("################ max HNB[%d] = %d\n",i,g_hnb_count[i]->gmax_hnb_count);
		}
		
	}

	for(i=0; i<glicensecount; i++)
	{
		HNB_NUM += g_hnb_count[i]->gmax_hnb_count;
	}
	
	printf("################ max HNB = %d\n",HNB_NUM);
	HNB_NUM += 1;
    UE_NUM = HNB_NUM * HNB_MAX_UE_NUM;
	IUH_HNB = malloc(HNB_NUM*(sizeof(Iuh_HNB*)));
	memset(IUH_HNB,0,HNB_NUM*(sizeof(Iuh_HNB *)));
	IUH_HNB_UE = malloc(UE_NUM*(sizeof(Iuh_HNB_UE*)));
	memset(IUH_HNB_UE,0,UE_NUM*(sizeof(Iuh_HNB_UE *)));
	
}

