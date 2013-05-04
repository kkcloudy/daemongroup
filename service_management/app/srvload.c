#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>

#define xml_path 		"/mnt/conf_xml.conf"   		//define xml file's path
#define cli_path 		"/mnt/cli.conf"
#define err_log 		"/var/log/start_err.log"  		//define log message output's path
#define conf_path 		"/opt/services/conf/"  	//define configure file's path
#define init_path 		"/opt/services/init/" 		//define shell's path
#define option_path 	"/opt/services/option/"	
#define status_path 	"/opt/services/status/"

#define XML_TYPE 	0
#define TXT_TYPE	1

int config_type();
int txt_load_type();
int xml_load_type();



int main(int argc, char**argv)
{
		int ret = 0;
		int fd;
		int newfd;
		int ret_load = 0;
		fd = open(err_log,O_RDWR|O_CREAT|O_TRUNC);
		if(fd == -1){
			//fprintf(stderr,"can not open %s!\n",err_log);
			return -1;
		}
		newfd = dup2(fd,2);
		if( -1 == newfd  ){
				//fprintf(stderr,"could not duplicate fd to 2\n");
				close(fd);
				return -2;
		}
		newfd = dup2(fd,1);
		if( -1 == newfd ){
				//fprintf(stderr,"could not duplicate fd to 2\n");
				close(fd);
				return -3;
		}

		ret = config_type();
		if(XML_TYPE == ret){
			ret_load = xml_load_type();
		}else if(TXT_TYPE == ret){
			ret_load = txt_load_type();
		}else{
			printf("config_type return err!\n");

		}
		
		close(fd);
		return 0;
}

int config_type()
{

	xmlDocPtr doc;
	
	doc = xmlReadFile(xml_path,"utf-8",256);
	if(doc == NULL)
	{
		  printf("It is not xml\n");
		  return TXT_TYPE;
	 }else{
		  printf("It is xml\n");
		  return XML_TYPE;
	 }
	return XML_TYPE;
}
int xml_load_type()
	{
	//	int fe;
		xmlDocPtr doc;
		xmlNodePtr cur;
		xmlNodePtr tmp;
		xmlChar *key;
	//	char tmp[20];
	//	char cmd_name[80]; 
		
		doc = xmlReadFile(xml_path,"utf-8",256);
		if(doc == NULL)
		{
			fprintf(stderr,"Does not open the file %s,",xml_path);
			fprintf(stderr,"may be no such file!\n");
			return 0;
		}
		cur = xmlDocGetRootElement(doc);
		if(cur == NULL)
		{
			fprintf(stderr,"empty document\n");
			xmlFreeDoc(doc);
			return 0;
			
		}
		cur = cur->xmlChildrenNode;
	
		char *buf = (char*)malloc(90);
		char *filename = (char*)malloc(40);
		char *chmod = (char*)malloc(100);
		while(cur != NULL)
		{
			if((strcmp((char*)cur->name,"dhcp") != 0)&&(strcmp((char*)cur->name,"dxml") != 0)&&(strcmp((char*)cur->name,"eag") != 0)
				&&(strcmp((char*)cur->name,"eagmt") != 0)&&(strcmp((char*)cur->name,"iptables") != 0)&&(strcmp((char*)cur->name,"firewall") != 0)
				&&(strcmp((char*)cur->name,"syslog") != 0)&&(strcmp((char*)cur->name,"multiportal") != 0)&&(strcmp((char*)cur->name,"multiradius") != 0)
				&&(strcmp((char*)cur->name,"nasidvlan") != 0)&&(strcmp((char*)cur->name,"nasmt") != 0)&&(strcmp((char*)cur->name,"portal") != 0)
				&&(strcmp((char*)cur->name,"portalmt") != 0)&&(strcmp((char*)cur->name,"radiusmt") != 0)&&(strcmp((char*)cur->name,"wwvmt") != 0)
				&&(strcmp((char*)cur->name,"snmpd") != 0))
			{
				tmp = cur->xmlChildrenNode;
				while(tmp != NULL)
				{
					fprintf(stderr,"%s\n",tmp->name);
					int fd_tmp;
					if(!strcmp((char*)tmp->name,"conf"))
					{
						memset(filename,0,40);
						memset(buf,0,90);
						sprintf(filename,"%s_conf.conf",cur->name);
						printf("Load %s Configure File.....\n",cur->name);
						strcat(buf,conf_path);
						strcat(buf,filename);
					//	printf("%s\n",buf);
						fd_tmp = open(buf,O_RDWR|O_CREAT);
						if(fd_tmp == -1)
						{
							fprintf(stderr,"can not create %s configure file %s.conf.\n",cur->name,cur->name);
							return 0;
						}	
						key = xmlNodeListGetString(doc,tmp->xmlChildrenNode,1);
						if(key != NULL)
						{
							int flag = write(fd_tmp,(char*)key,strlen((char*)key));
							if(flag == -1)
							{
								fprintf(stderr,"Create the configure %s failed!!\n",filename);
							}
							else
							{	
								fprintf(stderr,"Create the configure %s sucessfull!!\n",filename);
							}
						//	free(buf);
						//	free(filename);
							close(fd_tmp);	
							xmlFree(key);
						}
	
							sprintf(chmod,"chmod 666 %s",buf);
							fprintf(stderr,"%s\n",chmod);
							int tt = system(chmod);
							fprintf(stderr,"%d\n",tt);
							memset(chmod,0,100);
					}
					else if(!strcmp((char*)tmp->name,"option"))
					{
						
					//	char *buf = (char*)malloc(90);
					//	char *filename = (char*)malloc(20);
						memset(filename,0,40);
						memset(buf,0,90);
						sprintf(filename,"%s_option",cur->name);
						strcat(buf,option_path);
						strcat(buf,filename);
						fprintf(stderr,"%s\n",buf);
						fd_tmp = open(buf,O_RDWR|O_CREAT);
						if(fd_tmp == -1)
						{
							fprintf(stderr,"can not create %s configure file %s.conf.\n",cur->name,cur->name);
							return 0;
						}	
						key = xmlNodeListGetString(doc,tmp->xmlChildrenNode,1);
						if(key != NULL)
						{	
							int flag = write(fd_tmp,(char*)key,strlen((char*)key));
							if(flag == -1)
							{
								fprintf(stderr,"Create the configure %s failed!!\n",filename);
							}	
							else
							{	
								fprintf(stderr,"Create the configure %s sucessfull!!\n",filename);
							}
							//	free(buf);
						//	free(filename);
							close(fd_tmp);
							xmlFree(key);	
						}
						
							sprintf(chmod,"chmod 777 %s",buf);
							fprintf(stderr,"%s\n",chmod);
							int tt = system(chmod);
							fprintf(stderr,"%d\n",tt);
							memset(chmod,0,100);
					}
					else
					{
					//	char *buf = (char*)malloc(90);
					//	char *filename = (char*)malloc(20);
						memset(filename,0,40);
						memset(buf,0,90);
						sprintf(filename,"%s_status.status",cur->name);
						strcat(buf,status_path);
						strcat(buf,filename);
						fprintf(stderr,"%s\n",buf);
						fd_tmp = open(buf,O_RDWR|O_CREAT);
						if(fd_tmp == -1)
						{
							fprintf(stderr,"can not create %s configure file %s.conf.\n",cur->name,cur->name);
							return 0;
						}	
						key = xmlNodeListGetString(doc,tmp->xmlChildrenNode,1);
						if(key != NULL)
						{
							int flag = write(fd_tmp,(char*)key,strlen((char*)key));
							if(flag == -1)
							{
								fprintf(stderr,"Create the configure %s failed!!\n",filename);
							}
							else
							{	
								fprintf(stderr,"Create the configure %s sucessfull!!\n",filename);
							}
					//		printf("%s\n",key);
					//		sprintf(cmd_name,"%s%s_init",init_path,cur->name);
					//		printf("%s\n",cmd_name);
					//		fe = access(cmd_name,0);
					//		if(fe==0)
					//		{	
					//		memset(cmd_name,0,80);
					//		sprintf(cmd_name,"%s%s_init %s",init_path,cur->name,key);
					//		printf("%s\n",cmd_name);
					//		int ret = system(cmd_name);
					//		if(ret == 0)
					//			{
					//				printf("command %s sucessfull!\n",cmd_name);
					//			}					
					//		else
					//			{
					//				printf("command %s failed!\n",cmd_name);
					//			}
							xmlFree(key);	
				//			}
						}
					//	free(buf);
					//	free(filename);
						close(fd_tmp);
						sprintf(chmod,"chmod 666 %s",buf);
						fprintf(stderr,"%s\n",chmod);
						int tt = system(chmod);
						fprintf(stderr,"%d\n",tt);
						memset(chmod,0,100);
					}
						tmp = tmp->next;
				}
			}
		cur = cur->next;
		}
		free(buf);
		free(filename);
		free(chmod);
		return 0;
	}

int txt_load_type()
{
	char cp[255] = { 0 };
	sprintf(cp,"sudo cp %s %s",xml_path,cli_path);
	int ret = 0;
	ret = system(cp);
	return ret;
}


