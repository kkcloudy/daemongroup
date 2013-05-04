#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define xml_path "/mnt/conf_xml.conf"                   //define xml file's path
#define err_log "/var/run/apache2/err_log.txt"  		//define log message output's path
#define conf_path "/opt/services/conf/"         //define configure file's path
#define init_path "/opt/services/init/"               //define shell's path
//#define option_path "/opt/services/option/"
#define status_path "/opt/services/status/"
//

int main(int argc,char **argv)
{
	FILE *fp;
	FILE *fd;
	int ret = 0;
	char *name_buf = (char*)malloc(128);
	char *name=(char*)malloc(30);
	char *para = (char*)malloc(20);
	char * cmd = (char*)malloc(128);
	 int i = 0;
	 char * stat_name=(char*)malloc(128);
	 memset(name_buf,0,128);

	// Redirect errout	 
	 int fd_errout;
	 int newfd;
	 fd_errout = open(err_log,O_RDWR|O_CREAT|O_TRUNC);
	 if(fd_errout == -1)
	 {
		 fprintf(stderr,"can not open %s!\n",err_log);
		 return 0;
	 }
	 newfd = dup2(fd_errout,2);
	 if( newfd != 2 )
		 {
			 fprintf(stderr,"could not duplicate fd to 2\n");
			 exit(1);
		 }


	 
	 fp = popen("ls /opt/services/status","r");
         if(fp == NULL)
         {
	     fprintf(stderr,"can not save configure files!\n");
	     return 0;
         }
	 while(fgets(name_buf,128,fp))
		{ 
                   
		    i = 0;
		    ret = 0;
			fprintf(stderr,"file name:%s\n",name_buf);
			fprintf(stderr,"len=%d\n",strlen(name_buf));
		    while( name_buf+i != NULL )
		         {
		              if(*(name_buf+i) == '_')
		                          break;
		             else
		                    i++;
		           }
		    fprintf(stderr,"name_buf[0]=%c\n",*(name_buf+0));
            fprintf(stderr,"i=%d\n",i);
		    memset(name,0,30);
		    strncpy(name,name_buf,i);
			fprintf(stderr,"%s\n",name);

			if((strcmp(name,"dhcp") == 0) ||(strcmp(name,"eag") == 0)||(strcmp(name,"iptables") == 0)||(strcmp(name,"snmpd") == 0)||(strcmp(name,"syslog") == 0))
				{
					fprintf(stderr,"%s can not need to exec!",name);
					continue;
				}
			memset(stat_name,0,128);
			strcpy(stat_name,status_path);
			strcat(stat_name,name);
			strcat(stat_name,"_status.status");
			fprintf(stderr,"%s",stat_name);
			fd = fopen(stat_name,"r");
			if(fd == NULL)
			{
				fprintf(stderr,"can not open the file %s",stat_name);
				continue;
			}
			 memset(cmd,0,128);
			 strcpy(cmd,init_path);
			 strcat(cmd,name);
			 strcat(cmd,"_init");
		     fprintf(stderr,"cmd===%s\n",cmd);			
       		if (access(cmd, F_OK) == -1) 
			{ 
		          fprintf(stderr,"cmd not exists!"); 
		          continue; 
		     } 
            else if (access(cmd, X_OK) == -1) 
			{ 
		          fprintf(stderr,"cmd can not  exec!"); 
		          continue; 
			}
			else
			{
					
				memset(para,0,20);
				fgets(para,20,fd);
				strcat(cmd," ");
				strcat(cmd,para);
				int n = strlen(para);
				if( para[n-1] == '\n' )
					para[n-1]='\0';
				printf("%s %s ....\n",para,name);
				fprintf(stderr,"%s\n",cmd);
				ret = system(cmd);
				if(ret == 0)
				 {
				   fprintf(stderr,"command %s sucessfull!\n",cmd);
				 }
				else
				 {
				   fprintf(stderr,"command %s failed!\n",cmd);
				  }
			}
			
		memset(name_buf,0,128);
		fclose(fd);
		}
free(cmd);
free(para);
free(stat_name);
pclose(fp);
close(fd_errout);
return 0;
}
