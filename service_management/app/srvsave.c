#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>

#define xml_path "/mnt/conf_xml.conf"   		//define xml file's path
#define err_log "/mnt/err_log.txt"  		//define log message output's path
#define conf_path "/opt/services/conf/"  	//define configure file's path
//#define init_path "/opt/services/init/" 		//define shell's path
#define option_path "/opt/services/option/"	
#define status_path "/opt/services/status/"

long get_file_size( char* filename )
{
    FILE* fp = fopen( filename, "r" );
     if (fp==NULL) return -1;
	    fseek( fp, 0L, SEEK_END );
       return ftell(fp); 
}


int main(int argc,char **argv)
{

	xmlDocPtr doc;
	FILE *fp;
	xmlNodePtr root_node = NULL;
	xmlNodePtr cur = NULL,node_conf = NULL,node_option = NULL,node_status = NULL,node_tmp = NULL;
	doc = xmlNewDoc(BAD_CAST"1.0");
	root_node = xmlNewNode(NULL,BAD_CAST"root");
	xmlDocSetRootElement(doc,root_node);
	fp = popen("ls /opt/services/status","r");
	if(fp == NULL)
	{
		printf("can not save configure files!\n");
		return 0;
	}
	char name_buf[100];
	char name[40];
	memset(name_buf,0,100);
	int fd;
	long int len = 0;
	int i = 0;
	char * filename = (char *)malloc(140);
	char *confile = NULL;
	while(fgets(name_buf,100,fp) != NULL)
	{
/*save configure file*/
		i = 0;
		printf("file name:%s\n",name_buf);
		memset(name,0,40);
		while( name_buf+i )
		{
			if(*(name_buf+i) == '_')
				break;
			else
				i++;
		}
		strncpy(name,name_buf,i);
		printf("name:%s\n",name);
		
		if((strcmp(name,"dhcp") == 0)||(strcmp(name,"dxml") == 0)||(strcmp(name,"eag") == 0)
			||(strcmp(name,"eagmt") == 0)||(strcmp(name,"iptables") == 0)||(strcmp(name,"firewall") == 0)
			||(strcmp(name,"syslog") == 0)||(strcmp(name,"multiportal") == 0)||(strcmp(name,"multiradius") == 0)
			||(strcmp(name,"nasidvlan") == 0)||(strcmp(name,"nasmt") == 0)||(strcmp(name,"portal") == 0)
			||(strcmp(name,"portalmt") == 0)||(strcmp(name,"radiusmt") == 0)||(strcmp(name,"wwvmt") == 0)
			||(strcmp(name,"snmpd") == 0))
			{
				printf("%s can not need to exec!",name);
				continue;
			}
		printf("save the configure file:%s ...................................................\n",name);
		cur = xmlNewNode(NULL,(const xmlChar*)name);
		node_conf = xmlNewNode(NULL,(const xmlChar*)"conf");
		memset(filename,0,140);
		sprintf(filename,"%s%s_conf.conf",conf_path,name);
		//printf("%d\n",tt);
		printf("%s\n",filename);
		fd = open(filename,O_RDONLY);
		if(fd == -1)
		{
			printf("can not save configure file :%s\n",filename);
		}
		else
		{	
			len = get_file_size(filename);
			printf("len===========%ld\n",len);
			confile = (char*)malloc(len+1);
			memset(confile,0,len+1);
			if( read(fd,confile,len) == -1)
			{
				printf("read file %s failed!\n",filename);
			
			}
			node_tmp = xmlNewText(BAD_CAST(const xmlChar*)confile);
			xmlAddChild(node_conf,node_tmp);
			xmlAddChild(cur,node_conf);
			memset(name_buf,0,100);
			memset(filename,0,140);
			free(confile);
			close(fd);
			printf("complete save configure!\n");
		}
//save option file
		
		printf("name:%s\n",name);
		printf("save the option file:%s ...................................................\n",name);
	//	cur = xmlNewNode(NULL,(const xmlChar*)name);
		node_option = xmlNewNode(NULL,(const xmlChar*)"option");
		sprintf(filename,"%s%s_option",option_path,name);
		printf("%s\n",filename);
		fd = open(filename,O_RDONLY);
		if(fd == -1)
		{
			printf("can not save configure file :%s\n",filename);
		}
		else
		{
			len = 0;
			len = get_file_size(filename);
			printf("len==============%ld\n",len);
			confile  = (char*)malloc(len+1);
			memset(confile,0,len+1);
			if( read(fd,confile,len) == -1)
			    {
                             printf("read file %s failed!\n",filename);
			    }
			node_tmp = xmlNewText(BAD_CAST(const xmlChar*)confile);
			//free(confile);
			xmlAddChild(node_option,node_tmp);
			xmlAddChild(cur,node_option);
	//		xmlAddChild(root_node,cur);
			close(fd);
			memset(name_buf,0,100);
			memset(filename,0,140);
			free(confile);
			printf("complete save option!\n");
		}
//save status file
		
		printf("save the file status:%s ...................................................\n",name);
			printf("name:%s\n",name);
	//	cur = xmlNewNode(NULL,(const xmlChar*)name);
			node_status = xmlNewNode(NULL,(const xmlChar*)"status");
			sprintf(filename,"%s%s_status.status",status_path,name);
			printf("%s\n",filename);
		fd = open(filename,O_RDONLY);
		if(fd == -1)
		{
			printf("can not save configure file :%s\n",filename);
		}
		else
		{
			len = 0;
			len = get_file_size(filename);
			 printf("len==============%ld\n",len);
			 confile = (char*)malloc(len+1);
			 memset(confile,0,len+1);
			if( read(fd,confile,len) == -1)
			{
				printf("read file %s failed!\n",filename);
			}		
			node_tmp = xmlNewText(BAD_CAST(const xmlChar*)confile);
		//free(confile);
			xmlAddChild(node_status,node_tmp);
			xmlAddChild(cur,node_status);
			xmlAddChild(root_node,cur);
			close(fd);
			memset(name_buf,0,100);
			memset(filename,0,140);
			free(confile);
			printf("complete save status!\n");
		}

	}
	free(filename);
	xmlSaveFormatFileEnc(xml_path,doc,"utf-8",1);
	xmlFreeDoc(doc);
	xmlCleanupParser();	
	xmlMemoryDump();
	return 0;
}

