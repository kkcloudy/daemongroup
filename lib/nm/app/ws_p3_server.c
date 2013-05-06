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
* capture.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
*
* DESCRIPTION:
*
*
*
*******************************************************************************/

#include "ws_p3_server.h"
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpathInternals.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include "cgic.h"




/*读取pppoe的xml文件信息*/
int read_p3_xml(char * name, ST_P3_ALL *sysall)
{
	xmlDocPtr pdoc = NULL;

	xmlNodePtr pcurnode = NULL;
	char *psfilename;

	psfilename = name;

	memset(sysall, 0, sizeof(ST_P3_ALL) );

	pdoc = xmlReadFile(psfilename,"utf-8",256);  //解析文件

	if(NULL == pdoc)
	{

		//fprintf(cgiOut,"error: open file %s" , psfilename);
		return 1;

	}  
	pcurnode = xmlDocGetRootElement(pdoc);  //得到根节点   

	xmlChar *value;
	xmlChar *content;
	int i=0,n=0,p=0,j=0;
	pcurnode=pcurnode->xmlChildrenNode;  //得到子节点集合,函数中有了

	while (pcurnode != NULL)   //遍历子节点集合，找出所需的，
	{     


		//radius 
		if ((!xmlStrcmp(pcurnode->name, BAD_CAST  P3_RADIUS)))
		{    

			xmlNodePtr testnode;

			testnode=pcurnode;	 
			testnode=testnode->children;

			while(testnode !=NULL)
			{	 

				//radius下有serverip属性的        
				if ((!xmlStrcmp(testnode->name, BAD_CAST  P3_SERIP)))   
				{
					value = xmlNodeGetContent(testnode);	
					strcpy(sysall->s_radius.serip,(char *)value);	
					xmlFree(value);
				}	    	 

				//radius下有password属性的    		
				if ((!xmlStrcmp(testnode->name, BAD_CAST P3_PWD)))   
				{
					value = xmlNodeGetContent(testnode);
					strcpy(sysall->s_radius.passwd,(char *)value);	
					xmlFree(value);
				}	

				//radius下有auth属性的    		
				if ((!xmlStrcmp(testnode->name, BAD_CAST P3_AUTH)))   
				{
					value = xmlNodeGetContent(testnode);
					strcpy(sysall->s_radius.auth,(char *)value);	
					xmlFree(value);
				}	

				//radius下有auport 属性的  
				if ((!xmlStrcmp(testnode->name, BAD_CAST P3_AUTH_PORT)))   
				{
					value = xmlNodeGetContent(testnode);
					strcpy(sysall->s_radius.auport,(char *)value);	
					xmlFree(value);
				}	

				//radius下有acct属性的    		
				if ((!xmlStrcmp(testnode->name, BAD_CAST P3_ACCT)))   
				{
					value = xmlNodeGetContent(testnode);
					strcpy(sysall->s_radius.acct,(char *)value);	
					xmlFree(value);
				}
				//radius下有acport属性的    		
				if ((!xmlStrcmp(testnode->name, BAD_CAST P3_ACCT_PORT)))   
				{
					value = xmlNodeGetContent(testnode);
					strcpy(sysall->s_radius.acport,(char *)value);	
					xmlFree(value);
				}	


				//radius下有default属性的    	剩余的
				if ((!xmlStrcmp(testnode->name, BAD_CAST P3_DEF)))   
				{
					value = xmlNodeGetContent(testnode);
					strcpy(sysall->s_radius.def,(char *)value);	
					xmlFree(value);
				}	

				//radius下有log属性的    	
				if ((!xmlStrcmp(testnode->name, BAD_CAST P3_LOGIN)))   
				{
					value = xmlNodeGetContent(testnode);
					strcpy(sysall->s_radius.log,(char *)value);	
					xmlFree(value);
				}	

				//radius下有dict属性的    	
				if ((!xmlStrcmp(testnode->name, BAD_CAST P3_DIC)))   
				{
					value = xmlNodeGetContent(testnode);
					strcpy(sysall->s_radius.dict,(char *)value);	
					xmlFree(value);
				}	

				//radius下有ser属性的    	
				if ((!xmlStrcmp(testnode->name, BAD_CAST P3_SER)))   
				{
					value = xmlNodeGetContent(testnode);
					strcpy(sysall->s_radius.ser,(char *)value);	
					xmlFree(value);
				}	


				testnode = testnode->next;	  
			}	   
			n++;
		}  

		////////////////////////////////////////////////////////////////////////////////


		//interface  的情况
		if ((!xmlStrcmp(pcurnode->name, BAD_CAST P3_INF)))
		{    

			xmlNodePtr testnode;

			testnode=pcurnode;

			testnode=testnode->children;
			while(testnode !=NULL)
			{	 

				//interface下有max属性的

				if ((!xmlStrcmp(testnode->name, BAD_CAST  P3_MAX)))   
				{
					value = xmlNodeGetContent(testnode);	
					strcpy(sysall->s_inf.max,(char *)value);	
					xmlFree(value);
				}	

				//interface下有base属性的

				if ((!xmlStrcmp(testnode->name, BAD_CAST P3_BASE)))  
				{
					value = xmlNodeGetContent(testnode);
					strcpy(sysall->s_inf.base,(char *)value);	
					xmlFree(value);
				}	

				//interface下有myip属性的

				if ((!xmlStrcmp(testnode->name, BAD_CAST P3_MYIP)))  
				{
					value = xmlNodeGetContent(testnode);
					strcpy(sysall->s_inf.myip,(char *)value);	
					xmlFree(value);
				}	

				//interface下有p3if(port)属性的

				if ((!xmlStrcmp(testnode->name, BAD_CAST P3_PORT)))  
				{
					value = xmlNodeGetContent(testnode);
					strcpy(sysall->s_inf.port,(char *)value);	
					xmlFree(value);
				}	
				testnode = testnode->next;	  
			}	   
			p++;
		}  


		////////////////////////////////////////////////////////////////////////////////

		//dns  的情况
		if ((!xmlStrcmp(pcurnode->name, BAD_CAST P3_DNS)))
		{    

			xmlNodePtr testnode;

			testnode=pcurnode;

			testnode=testnode->children;
			while(testnode !=NULL)
			{	 

				//dns下有hostip属性的
				if ((!xmlStrcmp(testnode->name, BAD_CAST  P3_HIP)))   
				{
					value = xmlNodeGetContent(testnode);	
					strcpy(sysall->s_dns.hostip,(char *)value);	
					xmlFree(value);
				}

				//dns下有backip属性的
				if ((!xmlStrcmp(testnode->name, BAD_CAST  P3_BIP))) 
				{
					content = xmlNodeGetContent(testnode);		 
					strcpy(sysall->s_dns.backip,(char *)value);

					xmlFree(content);
				}

				//dns下有def属性的
				if ((!xmlStrcmp(testnode->name, BAD_CAST  P3_DEFAULT))) 
				{
					content = xmlNodeGetContent(testnode);		 
					strcpy(sysall->s_dns.defu,(char *)value);

					xmlFree(content);
				}

				//dns下有mask属性的
				if ((!xmlStrcmp(testnode->name, BAD_CAST  P3_MASK))) 
				{
					content = xmlNodeGetContent(testnode);		 
					strcpy(sysall->s_dns.mask,(char *)value);

					xmlFree(content);
				}

				//dns下有lcp属性的
				if ((!xmlStrcmp(testnode->name, BAD_CAST  P3_LCP))) 
				{
					content = xmlNodeGetContent(testnode);		 
					strcpy(sysall->s_dns.lcp,(char *)value);

					xmlFree(content);
				}

				//dns下有logfile属性的
				if ((!xmlStrcmp(testnode->name, BAD_CAST  P3_LFILE))) 
				{
					content = xmlNodeGetContent(testnode);		 
					strcpy(sysall->s_dns.logfile,(char *)value);

					xmlFree(content);
				}

				//dns下有plugin属性的
				if ((!xmlStrcmp(testnode->name, BAD_CAST  P3_PLUG))) 
				{
					content = xmlNodeGetContent(testnode);		 
					strcpy(sysall->s_dns.plugin,(char *)value);

					xmlFree(content);
				}

				testnode = testnode->next;

			}	    
			i++;
		}  


		////////////////////////////////////////////////////////////////////////////////
		//host 
		if ((!xmlStrcmp(pcurnode->name, BAD_CAST  P3_HOST)))
		{    

			xmlNodePtr testnode;

			testnode=pcurnode;	 
			testnode=testnode->children;

			while(testnode !=NULL)
			{	 

				//host下有hostdef属性的        
				if ((!xmlStrcmp(testnode->name, BAD_CAST  P3_HOSTDEF)))   
				{
					value = xmlNodeGetContent(testnode);	
					strcpy(sysall->s_host.hostdef,(char *)value);	
					xmlFree(value);
				}	    	 

				//host下有hostip属性的    		
				if ((!xmlStrcmp(testnode->name, BAD_CAST P3_HOSTIP)))   
				{
					value = xmlNodeGetContent(testnode);
					strcpy(sysall->s_host.hostip,(char *)value);	
					xmlFree(value);
				}	

				//host下有hostname属性的    		
				if ((!xmlStrcmp(testnode->name, BAD_CAST P3_HOSTNAME)))   
				{
					value = xmlNodeGetContent(testnode);
					strcpy(sysall->s_host.hostname,(char *)value);	
					xmlFree(value);
				}	

				testnode = testnode->next;	  
			}	  
			j++;
		}  

		////////////////////////////////////////////////////////////////////////////////

		pcurnode = pcurnode->next;
	} 
	sysall->dnum=i;
	sysall->ifnum=p;
	sysall->rnum=n;
	sysall->hnum=j;
	xmlFreeDoc(pdoc);
	xmlCleanupParser();
	return 0;
}



/*写入到不同的配置文件中去*/
int write_p3_conf( ST_P3_ALL *sysall) 
{

	FILE * fp;	
	char content[2000], temp[256];
	//int i;

	// radius的配置文件

	if(( fp = fopen(P3_SERVER,"w+"))==NULL)
	{
		return -1;
	}
	else
	{

		memset(content,0,1024);			
		sprintf(content , "%s  %s\n",sysall->s_radius.serip,sysall->s_radius.passwd);			
		fwrite(content,strlen(content),1,fp);	
		fclose(fp);
	}	
	///////////////////////////////////////////////////////////
	if(( fp = fopen(P3_RCONF,"w+"))==NULL)
	{
		return -2;
	}
	else
	{

		memset(content,0,1024);	

		memset(temp,0,256);
		sprintf(temp , "%s",sysall->s_radius.log);			
		strcat(content,temp);

		memset(temp,0,256);
		sprintf(temp , "authserver %s:%s\n",sysall->s_radius.auth,sysall->s_radius.auport);			
		strcat(content,temp);

		memset(temp,0,256);
		sprintf(temp , "acctserver %s:%s\n",sysall->s_radius.acct,sysall->s_radius.acport);	
		strcat(content,temp);

		memset(temp,0,256);		
		sprintf(temp , "%s\n",sysall->s_radius.dict);			
		strcat(content,temp);

		memset(temp,0,256);		
		sprintf(temp , "%s\n",sysall->s_radius.ser);		
		strcat(content,temp);

		memset(temp,0,256);
		sprintf(temp , "%s",sysall->s_radius.def);	
		strcat(content,temp);

		fwrite(content,strlen(content),1,fp);	
		fclose(fp);
	}	
	///////////////////////////////////////////////////////////


	// interface and ip 配置  .sh文件

	system("sudo chmod 666 /etc/init.d/pppoe-server.sh");
	if(( fp = fopen(P3_SERSH,"w+"))==NULL)
	{
		return -3;
	}
	else
	{

		memset(content,0,1024);	

		memset(temp,0,256);
		sprintf(temp , "%s\n","#!/bin/bash");
		strcat(content,temp);

		memset(temp,0,256);
		sprintf(temp , "MAX=%s \n",sysall->s_inf.max);	
		strcat(content,temp);

		memset(temp,0,256);
		sprintf(temp , "BASE=%s \n",sysall->s_inf.base);
		strcat(content,temp);


		memset(temp,0,256);
		sprintf(temp , "MYIP=%s \n",sysall->s_inf.myip);
		strcat(content,temp);

		memset(temp,0,256);
		sprintf(temp , "PPPOEIF=%s \n",sysall->s_inf.port);	
		strcat(content,temp);

		memset(temp,0,256);
		sprintf(temp , "%s \n","/usr/sbin/pppoe-server -T 60 -I $PPPOEIF -N $MAX -S ax7 -L $MYIP -R $BASE");
		strcat(content,temp);

		fwrite(content,strlen(content),1,fp);	
		fclose(fp);

		system("sudo chmod 755 /etc/init.d/pppoe-server.sh");  //修改为可执行的权限
	}	
	///////////////////////////////////////////////////////////    

	//读取结构体中内容来写入到文件中去  dns 的配置文件	

	if(( fp = fopen(P3_OPTION,"w+"))==NULL)
	{
		return -4;
	}
	else
	{

		memset(content,0,1024);	
		memset(temp,0,256);
		sprintf(temp , "ms-dns %s\n",sysall->s_dns.hostip);	
		strcat(content,temp);

		memset(temp,0,256);
		sprintf(temp , "ms-dns %s\n",sysall->s_dns.backip);	
		strcat(content,temp);

		memset(temp,0,256);
		sprintf(temp , "%s\n",sysall->s_dns.defu);	
		strcat(content,temp);

		memset(temp,0,256);
		sprintf(temp , "%s\n",sysall->s_dns.mask);	
		strcat(content,temp);

		memset(temp,0,256);
		sprintf(temp , "%s\n",sysall->s_dns.lcp);	
		strcat(content,temp);

		memset(temp,0,256);
		sprintf(temp , "logfile  %s\n",sysall->s_dns.logfile);	
		strcat(content,temp);

		memset(temp,0,256);
		sprintf(temp , "%s\n",sysall->s_dns.plugin);	
		strcat(content,temp);


		fwrite(content,strlen(content),1,fp);	
		fclose(fp);
	}	

	///////////////////////////////////////////////////////////    

	//读取结构体中内容来写入到文件中去  host 的配置文件	

	if(( fp = fopen(P3_HP,"w+"))==NULL)
	{
		return -5;
	}
	else
	{

		memset(content,0,1024);	
		memset(temp,0,256);
		sprintf(temp , "%s\n",sysall->s_host.hostdef);	
		strcat(content,temp);

		memset(temp,0,256);
		sprintf(temp , "%s %s\n",sysall->s_host.hostip,sysall->s_host.hostname);	
		strcat(content,temp);

		fwrite(content,strlen(content),1,fp);	
		fclose(fp);
	}	
	return 0;
}





/*删除指定节点，指定属性的节点*/
int p3_del(char *fpath,char *node_name,char *attribute,char *key)
{
	xmlDocPtr pdoc = NULL;

	xmlNodePtr pcurnode = NULL;
	char *psfilename;

	psfilename = fpath;

	pdoc = xmlReadFile(psfilename,"utf-8",256);  //解析文件

	if(NULL == pdoc)
	{
		//fprintf(cgiOut,"error: open file %s" , psfilename);
		return 1;
	}

	pcurnode = xmlDocGetRootElement(pdoc);  //得到根节点    

	pcurnode = pcurnode->xmlChildrenNode;  //得到子节点集合

	while (NULL != pcurnode)
	{			

		if (!xmlStrcmp(pcurnode->name, BAD_CAST node_name))   //找到 rest 这个 root 下一级子节点             
		{

			if (xmlHasProp(pcurnode,BAD_CAST attribute))
			{     



				if (!xmlStrcmp(pcurnode->name, BAD_CAST node_name))   //找到 des 这个 root 下一级子节点des

				{            

					xmlNodePtr tempNode;				  		   

					tempNode = pcurnode->next;  // 同层的节点

					xmlUnlinkNode(pcurnode);

					xmlFreeNode(pcurnode);

					pcurnode= tempNode;			   

					continue;

				} 	 

			}
		}        
		pcurnode = pcurnode->next; 

	}	  
	xmlSaveFile(fpath,pdoc);  //保存xml文件
	return 0;
}


//查看指定节点的值
/*find designed node 一个节点的属性如果不存在这个节点，做法 */

int find_p3_node(char * fpath,char * node_name,char * content,char *logkey)
{
	xmlDocPtr pdoc = NULL;

	xmlNodePtr pcurnode = NULL;
	char *psfilename;

	int flag=-1;

	psfilename = fpath;

	pdoc = xmlReadFile(psfilename,"utf-8",256);  //解析文件

	if(NULL == pdoc)
	{
		//fprintf(cgiOut,"error: open file %s" , psfilename);
		return 1;
	}

	pcurnode = xmlDocGetRootElement(pdoc);  //得到根节点    

	pcurnode = pcurnode->xmlChildrenNode;  //得到子节点集合

	xmlNodePtr propNodePtr = pcurnode;


	while (NULL != pcurnode)
	{		                        
		if (!xmlStrcmp(pcurnode->name, BAD_CAST node_name))   //root 下一级子节点             
		{          
			propNodePtr = pcurnode;		

			//此处来处理下一级的节点
			xmlNodePtr childPtr = propNodePtr; 
			childPtr=childPtr->children;
			xmlChar *value;

			while(childPtr !=NULL)
			{	 
				if ((!xmlStrcmp(childPtr->name, BAD_CAST content)))
				{
					value = xmlNodeGetContent(childPtr);        //取出 孙子节点的值  	
					strcpy(logkey,(char *)value);	
					flag=0;
					xmlFree(value);
				}         	 
				childPtr = childPtr->next;
			}		
		} 	 
		pcurnode = pcurnode->next;                     
	}	  
	xmlSaveFile(fpath,pdoc);  //保存xml文件
	return flag;
}


//修改指定节点的值
int mod_p3_node(char * fpath,char * node_name,char * content,char *newc)
{
	xmlDocPtr pdoc = NULL;

	xmlNodePtr pcurnode = NULL;
	char *psfilename;

	psfilename = fpath;

	pdoc = xmlReadFile(psfilename,"utf-8",256);  //解析文件

	if(NULL == pdoc)
	{
		//fprintf(cgiOut,"error: open file %s" , psfilename);
		return 1;
	}

	pcurnode = xmlDocGetRootElement(pdoc);  //得到根节点    

	pcurnode = pcurnode->xmlChildrenNode;  //得到子节点集合

	xmlNodePtr propNodePtr = pcurnode;

	while (NULL != pcurnode)
	{			

		if (!xmlStrcmp(pcurnode->name, BAD_CAST node_name))   //找到 des 这个 root 下一级子节点             
		{

			propNodePtr = pcurnode;		
			//此处来处理下一级的节点
			xmlNodePtr childPtr = propNodePtr; 
			childPtr=childPtr->children;
			xmlChar *value;

			while(childPtr !=NULL)
			{	 
				if ((!xmlStrcmp(childPtr->name, BAD_CAST content)))
				{
					value = xmlNodeGetContent(childPtr);         	
					xmlNodeSetContent(childPtr, BAD_CAST  newc); 
					xmlFree(value);
				}             	 
				childPtr = childPtr->next;
			}                    
		}        
		pcurnode = pcurnode->next; 

	}	  
	xmlSaveFile(fpath,pdoc);  //保存xml文件
	return 0;
}

/* check whether ppppoe service is enable*/
int ser_p3_enbale(){

	char status[10] = {0};
	find_p3_node(P3_XML_FPATH, P3_INF, P3_STATUS,status);
	if(!strcmp(status,"1")){
		return 1;
	}
	else{
		return 0;
	}	
}

/* check whether /opt/option/pppoe_option is exsited*/
void file_p3_exsit(){
	char cmd[128] = {0};
	if(access(P3_XML_FPATH,0) != 0){
		memset(cmd,0,128);
		sprintf(cmd,"sudo cp %s %s",P3_CONFXMLT,P3_XML_FPATH);
		system(cmd);
	}
	else{
		memset(cmd,0,128);
		sprintf(cmd,"sudo chmod 777 %s",P3_XML_FPATH);
		system(cmd);	
	 }
}
