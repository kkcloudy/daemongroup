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
* ws_log_conf.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* function for web
*
*
***************************************************************************/
#include "ws_log_conf.h"
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpathInternals.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
/*#include "cgic.h"*/
#include <signal.h>
#include <time.h>


/*判断文件是否存在，不用脚本*/

int is_file_exist( char *filtpath )
{
	return access( filtpath, 0 ); 
}

/*启动服务, 可设定值*/
int start_syslog()
{
	int iRet,status;
	char cmd[128] = {0};
	sprintf(cmd,"%s %s 2&> /dev/null",S_PATH,S_START);
	status = system(cmd);
	iRet = WEXITSTATUS(status);
	return iRet;
}

/*停止服务, 可设定值*/
int stop_syslog()
{
	int iRet,status;
	char cmd[128] = {0};
	sprintf(cmd,"%s %s 2&> /dev/null",S_PATH,S_STOP);
	status = system(cmd);
	iRet = WEXITSTATUS(status);
	return iRet;
}

/*重启syslog服务, 可设定值*/
int restart_syslog()
{
	int iRet,status;
	char cmd[128] = {0};
	sprintf(cmd,"%s %s 2&> /dev/null",S_PATH,S_RESTART);
	status = system(cmd);
	iRet = WEXITSTATUS(status);
	return iRet;
}

/*启动NTP服务, 可设定值*/
int start_ntp()
{
	int iRet,status;
	char cmd[128] = {0};
	sprintf(cmd,"%s %s 2&> /dev/null",N_PATH,N_START);
	status = system(cmd);
	iRet = WEXITSTATUS(status);
	
	return iRet;
}


/*停止NTP服务, 可设定值*/
int stop_ntp()
{
	int iRet,status;
	char cmd[128] = {0};
	sprintf(cmd,"%s %s 2&> /dev/null",N_PATH,N_STOP);
	status = system(cmd);
	iRet = WEXITSTATUS(status);
	//sleep(2);
	return iRet;
}
/*重启ntp服务, 可设定值*/
int restart_ntp()
{
	int iRet,status;
	char cmd[128] = {0};
	sprintf(cmd,"%s %s 2&> /dev/null",N_PATH,N_RESTART);
	status = system(cmd);
	iRet = WEXITSTATUS(status);
	
	return iRet;
}

inline void reset_sigmask() {
sigset_t psmask;
memset(&psmask,0,sizeof(psmask));
sigprocmask(SIG_SETMASK,&psmask,NULL);
}
/***************************************************************
写入信息到conf文件中去
****************************************************************/
int write_config( ST_SYS_ALL *sysall, char *file_path) 
{

	FILE * fp;	
	char File_content[50000],des_name[50],content[1024];
	memset(File_content,0,50000);
	memset(des_name,0,50);

    ST_LOG_KEY keys;  
    memset(&keys,0,sizeof(keys));
  

	int i;
	
	if( NULL == file_path)
	{
		return -1 ;  
	}
	if(( fp = fopen(file_path,"w+"))==NULL)
		return -2;

	 //option	
	 //循环读取结构体中内容来写入到文件中去  option	
	
		for(i=0;i<sysall->opt_num;i++)
		{
			memset(content,0,1024);			
			sprintf(content , " options { \n %s };\n\n",((sysall->optinfo[i]).content));		
			strcat(File_content,content);	

		}
	
		for(i=0;i<(sysall->opt_num);i++)
		{
             free((sysall->optinfo[i]).content);

		}
		
		///////////////////////////////////////////////////////////


		//循环读取结构体中内容来写入到文件中去  source
	
		for(i=0;i<sysall->su_num;i++)
		{
				memset(des_name,0,50);
				sprintf(des_name , "source  %s { \n",sysall->suinfo[i].suname);
			
				strcat(File_content,des_name);	
				
				memset(content,0,1024);			
				sprintf(content , " %s \n };\n\n",sysall->suinfo[i].content);
				strcat(File_content,content);		

		}
		///////////////////////////////////////////////////////////
	//循环读取结构体中内容来写入到文件中去  destination 配置
	
		for(i=0;i<sysall->des_num;i++)
	    {
				memset(des_name,0,50);
				sprintf(des_name , "destination %s",sysall->desinfo[i].rname);
			
				strcat(File_content,des_name);	

				memset(content,0,1024);			
				sprintf(content , " {%s};\n\n",sysall->desinfo[i].content);
				strcat(File_content,content);		

		}
		///////////////////////////////////////////////////////////
		//循环读取结构体中内容来写入到文件中去   filter配置
	
		for(i=0;i<sysall->f_num;i++)
		{
			memset(des_name,0,50);
			sprintf(des_name , "filter %s",sysall->finfo[i].fname);
		
			strcat(File_content,des_name);	

			memset(content,0,1024);			
			sprintf(content , " {%s};\n\n",sysall->finfo[i].content);
			strcat(File_content,content);		

		}
	//////////////////////////////////////////////////////////////////
	 //循环读取结构体中内容来写入到文件中去  log 信息
	
		for(i=0;i<sysall->log_num;i++)
		{
			memset(des_name,0,50);
			sprintf(des_name , "log { \n source(%s); \n",sysall->loginfo[i].source);

			strcat(File_content,des_name);	

	        /* multiple keys */
		    memset(content,0,1024);		
			memset(des_name,0,50);
			
			cut_up(sysall->loginfo[i].dest, &keys,S_DEST);  //调用函数，存储到结构体中  

			sprintf(content , " %s(%s);\n %s\n };\n\n",S_FILT,sysall->loginfo[i].filter, keys.key);		
			strcat(File_content,content);	

		}
		////////////////////////////////////////////////////////
	
	fwrite(File_content,strlen(File_content),1,fp);	
	fclose(fp);
	return 0;
}

/*读取文件信息函数*/
int read_filter(char * name,char * node_name, ST_SYS_ALL *sysall)
{
	 xmlDocPtr pdoc = NULL;

	 xmlNodePtr pcurnode = NULL;
	 char *psfilename;
	 
	 psfilename = name;
	 
	 memset(sysall, 0, sizeof(ST_SYS_ALL) );

	 pdoc = xmlReadFile(psfilename,"utf-8",256); 

	  if(NULL == pdoc)
	  {
	     return 1;
	  }  
	 pcurnode = xmlDocGetRootElement(pdoc);  

	  xmlChar *value;
	  int i=0,j=0,m=0,n=0,p=0;
	  pcurnode=pcurnode->xmlChildrenNode;  

	   while (pcurnode != NULL)   
	  {     
			//options  
		   if ((!xmlStrcmp(pcurnode->name, BAD_CAST  NODE_OPT)))
		  {   
		     value = xmlNodeGetContent(pcurnode);	
			 sysall->optinfo[n].content=(char *)malloc(xmlStrlen(value));	
			 strcpy(sysall->optinfo[n].content,(char *)value);	
			 xmlFree(value);
			 
			 n++;
		   }  
		////////////////////////////////////////////////////////////////////////////////
			//source  的情况
		   if ((!xmlStrcmp(pcurnode->name, BAD_CAST NODE_SOURCE)))
		  {    
			    
			     xmlNodePtr testnode;
				 
				 testnode=pcurnode;
				 
			     testnode=testnode->children;
				 while(testnode !=NULL)
			 	{	 

				    
				     if ((!xmlStrcmp(testnode->name, BAD_CAST  NODE_VALUE)))   
			     	{
						 value = xmlNodeGetContent(testnode);	
						 strcpy(sysall->suinfo[p].suname,(char *)value);	
					     xmlFree(value);
				   	}	
					 
						
				     if ((!xmlStrcmp(testnode->name, BAD_CAST NODE_CONTENT)))   
			     	{
						 value = xmlNodeGetContent(testnode);
						 strcpy(sysall->suinfo[p].content,(char *)value);	
					     xmlFree(value);
				   	}	
					  testnode = testnode->next;	  
		     	}	   
				 p++;
		   }  
		////////////////////////////////////////////////////////////////////////////////
			//des  的情况
		   if ((!xmlStrcmp(pcurnode->name, BAD_CAST NODE_DES)))
		  {    
			      
			     xmlNodePtr testnode;
				 
				 testnode=pcurnode;
				 
			     testnode=testnode->children;
				 while(testnode !=NULL)
			 	{	 
					//des value
				     if ((!xmlStrcmp(testnode->name, BAD_CAST  NODE_VALUE)))  
			     	 {
						 value = xmlNodeGetContent(testnode);	
						 strcpy(sysall->desinfo[i].rname,(char *)value);	
					     xmlFree(value);
				   	 }
					//des content
					 if ((!xmlStrcmp(testnode->name, BAD_CAST  NODE_CONTENT)))
			     	 {
						 value = xmlNodeGetContent(testnode);		 
						 strcpy(sysall->desinfo[i].content,(char *)value);
						
					     xmlFree(value);
				   	 }
					  testnode = testnode->next;
			   	}	    
				 i++;
		   }  
		////////////////////////////////////////////////////////////////////////////////
		  //filter   的情况
		   if ((!xmlStrcmp(pcurnode->name, BAD_CAST NODE_FILTER)))
		  {    
			  	xmlNodePtr testnode;				 
				 testnode=pcurnode;				 
			     testnode=testnode->children;
				 while(testnode !=NULL)
			 	{	 
                        //filter value
					     if ((!xmlStrcmp(testnode->name, BAD_CAST NODE_VALUE)))  
				     	{
							 value = xmlNodeGetContent(testnode);	
							 strcpy(sysall->finfo[j].fname,(char *)value);	
						     xmlFree(value);
					   	}
                        //filter view enable
					     if ((!xmlStrcmp(testnode->name, BAD_CAST NODE_VIEWS)))  
				     	{
							 value = xmlNodeGetContent(testnode);	
							 strcpy(sysall->finfo[j].views,(char *)value);	
						     xmlFree(value);
					   	}
					     //filter content
						 if ((!xmlStrcmp(testnode->name, BAD_CAST NODE_CONTENT)))
				     	{
							 value = xmlNodeGetContent(testnode);		 
							 strcpy(sysall->finfo[j].content,(char *)value);							
						     xmlFree(value);
					   	}
					     //filter infos
						 if ((!xmlStrcmp(testnode->name, BAD_CAST NODE_INFOS)))
				     	{
							 value = xmlNodeGetContent(testnode);		 
							 strcpy(sysall->finfo[j].infos,(char *)value);							
						     xmlFree(value);
					   	}
						  testnode = testnode->next;	  
			   	}	    
				 j++;
		   }  
		  ///////////////////////////////////////////////////////////////////////////////
		  	//log  的情况
		   if ((!xmlStrcmp(pcurnode->name, BAD_CAST NODE_LOG)))
		  {    
			  
			     xmlNodePtr testnode;
				 
				 testnode=pcurnode;
				 
			     testnode=testnode->children;
				 while(testnode !=NULL)
			 	{	 
                     //log keyz
				     if ((!xmlStrcmp(testnode->name, BAD_CAST CH_KEYZ)))  
			     	{
						 value = xmlNodeGetContent(testnode);
						 strcpy(sysall->loginfo[m].keyz,(char *)value);	
					     xmlFree(value);
				   	}
                     //log source
				     if ((!xmlStrcmp(testnode->name, BAD_CAST CH_SOURCE)))  
			     	{
						 value = xmlNodeGetContent(testnode);
						 strcpy(sysall->loginfo[m].source,(char *)value);	
					     xmlFree(value);
				   	}
				    //log filter 
					 if ((!xmlStrcmp(testnode->name, BAD_CAST CH_FILTER)))  
			     	{
						 value = xmlNodeGetContent(testnode);		 
						 strcpy(sysall->loginfo[m].filter,(char *)value);
						
					     xmlFree(value);
				   	}
                    //lot destination
				    if ((!xmlStrcmp(testnode->name, BAD_CAST CH_DEST)))   
			     	{
						 value = xmlNodeGetContent(testnode);		 
						 strcpy(sysall->loginfo[m].dest,(char *)value);
						
					     xmlFree(value);
				   	}
					  testnode = testnode->next;	  
			   	}	    
				 m++;
		   }  
		  ///////////////////////////////////////////////////////////////////////////////
		  
		  pcurnode = pcurnode->next;
   	  } 
	  sysall->des_num=i;
	  sysall->f_num=j;
	  sysall->log_num=m;
	  sysall->opt_num=n;
	  sysall->su_num=p;
	  
	  xmlFreeDoc(pdoc);
	  xmlCleanupParser();
	  return 0;
}

//free link list
void Free_read_syslogall_st(SYSLOGALL_ST *sysall)
{
    if(sysall->optnum>0)
		Free_read_opt_xml(&(sysall->optst));
	
    if(sysall->sournum>0)
		Free_read_source_xml(&(sysall->sourst));
	
	if(sysall->filtnum>0)
		Free_read_filter_xml(&(sysall->filtst));

	if(sysall->destnum>0)
		Free_read_dest_xml(&(sysall->destst));

	if(sysall->lognum>0)
		Free_read_log_xml(&(sysall->logst));	  	 
}


/*读取文件信息函数*/
int read_syslogall_st(char * xmlpath,SYSLOGALL_ST *sysall)
{
    int i,j,m,n,k;
	char newc[20] = {0};
	read_opt_xml(&(sysall->optst), &i);
	sysall->optnum=i;
	read_source_xml(&(sysall->sourst), &j);
	sysall->sournum=j;
	read_filter_xml(&(sysall->filtst), &m);
	sysall->filtnum=m;
	read_dest_xml(&(sysall->destst), &n);	 
	sysall->destnum=n;
	read_log_xml(&(sysall->logst), &k);
	sysall->lognum=k;
	memset(newc,0,sizeof(newc));
	get_first_xmlnode(XML_FPATH, NODE_LSTATUS,&newc);
	if (0 == strcmp(newc,"start"))
	{
		sysall->serv_flag = 1;
	}
	else
	{
		sysall->serv_flag = 0;
	}
	memset(newc,0,sizeof(newc));
	get_first_xmlnode(XML_FPATH, IF_SYNFLOOD,&newc);
	if (0 == strcmp(newc,"start"))
	{
		sysall->synflood= 1;
	}
	else
	{
		sysall->synflood = 0;
	}
	memset(newc,0,sizeof(newc));
	get_first_xmlnode(XML_FPATH, IF_EAGLOG,&newc);
	if (0 == strcmp(newc,"start"))
	{
		sysall->eaglog= 1;
	}
	else
	{
		sysall->eaglog= 0;
	}
	return 0;
}
/***************************************************************
写入信息到conf文件中去
****************************************************************/
int write_config_syslogallst( SYSLOGALL_ST *sysall, char *file_path) 
{

	FILE * fp;	
	char *File_content=(char *)malloc(50000);
	char *content=(char *)malloc(1024);
	char *des_name=(char *)malloc(50);

	memset(File_content,0,50000);	
	memset(des_name,0,50);

	ST_LOG_KEY keys;  
	memset(&keys,0,sizeof(keys));

	struct opt_st    *oq;
	struct source_st *sq;
	struct filter_st *fq;
	struct dest_st   *dq;
	struct log_st    *lq;

	if( NULL == file_path)
	{
		free(File_content);
		free(des_name);
		free(content);
		return -1 ;  
	}
	if(( fp = fopen(file_path,"w+"))==NULL)
	{
		free(File_content);
		free(des_name);
		free(content);
		return -2;
	}

	//option	
	oq=sysall->optst.next;
	while(oq != NULL)
	{
		memset(content,0,1024);			
		sprintf(content , " options { \n %s };\n\n",oq->contentz);		
		strcat(File_content,content);	
		oq = oq->next;
	}
	///////////////////////////////////////////////////////////
	//source 
	sq = sysall->sourst.next;
	while(sq!=NULL)
	{
		memset(des_name,0,50);
		sprintf(des_name , "source  %s { \n",sq->valuez);

		strcat(File_content,des_name);	

		memset(content,0,1024);			
		sprintf(content , " %s \n };\n\n",sq->contentz);
		strcat(File_content,content);		
		sq = sq->next;
	}

	///////////////////////////////////////////////////////////
	//des
	dq=sysall->destst.next;
	while(dq!=NULL)
	{
		memset(des_name,0,50);
		sprintf(des_name , "destination %s",dq->valuez);

		strcat(File_content,des_name);	

		memset(content,0,1024);			
		sprintf(content , " {%s};\n\n",dq->contentz);
		strcat(File_content,content);		
		dq=dq->next;
	}
	///////////////////////////////////////////////////////////
	//filter	
	fq=sysall->filtst.next;
	while(fq!=NULL)
	{
		memset(des_name,0,50);
		sprintf(des_name , "filter %s",fq->valuez);

		strcat(File_content,des_name);	

		memset(content,0,1024);			
		sprintf(content , " {%s};\n\n",fq->contentz);
		strcat(File_content,content);		
		fq=fq->next;
	}

	//////////////////////////////////////////////////////////////////
	//log
	lq=sysall->logst.next;
	while(lq!=NULL)
	{

		if ((0 == strcmp(lq->enablez,""))||(sysall->serv_flag != 0))//three must be add,and for the start flags 
		{
				memset(des_name,0,50);
				sprintf(des_name , "log { \n source(%s); \n",lq->sourcez);
				strcat(File_content,des_name);	
				/* multiple keys */
				memset(content,0,1024);		
				memset(des_name,0,50);
				cut_up(lq->des, &keys,S_DEST);  
				sprintf(content , " %s(%s);\n %s\n };\n\n",S_FILT,lq->filterz, keys.key);		
				strcat(File_content,content);	
		}
		if((sysall->synflood != 0)&&(sysall->serv_flag != 0)&&(0 != strcmp(lq->filterz,F_CLILOG))&&(0 == strcmp(lq->enablez,"1")))/*syslog send to server,and flag for the synflood*/
		{
			/*single for dos flood and sshd,telnetd*/
				/*single for dos flood and sshd,telnetd*/
				memset(des_name,0,50);
				sprintf(des_name , "log { \n source(%s); \n",lq->sourcez);
				strcat(File_content,des_name);
				memset(content,0,1024);		
				sprintf(content , " %s(%s);\n %s\n };\n\n",S_FILT,F_FLOOD, keys.key);		
				strcat(File_content,content);
		}
		lq=lq->next;
	}

	if(sysall->eaglog!= 0)/*syslog send to server,and flag for the eaglog*/
	{
			memset(des_name,0,50);
			sprintf(des_name , "log { \n source(s_all); \n");
			strcat(File_content,des_name);
			memset(content,0,1024);		
			sprintf(content , " %s(%s);\n destination (df_system);\n };\n\n",S_FILT,F_EAGLOG);		
			strcat(File_content,content);
	}

	////////////////////////////////////////////////////////

	fwrite(File_content,strlen(File_content),1,fp);	
	fclose(fp);
	free(File_content);
	free(des_name);
	free(content);
	return 0;
}

void new_xml_file(const char *xmlpath)
{
    if(NULL == xmlpath) {
        return ;
    }
    
	xmlDocPtr doc = NULL;			/* document pointer */	
	xmlNodePtr root_node = NULL;
	
	doc = xmlNewDoc(BAD_CAST "1.0");
	root_node = xmlNewNode(NULL, BAD_CAST "root" );
	xmlDocSetRootElement(doc, root_node);

	xmlSaveFormatFileEnc(xmlpath, doc, "utf-8", 1);
	/*free the document */
	xmlFreeDoc(doc);
	xmlCleanupParser();
	xmlMemoryDump();	  
	
	return ;	
}



/*更新一个节点内容 处理下级节点的内容*/
int mod_log_node(char * fpath,char * node_name,char *attribute,char *ruler,char * content,char *newc)
{
	xmlDocPtr pdoc = NULL;

	xmlNodePtr pcurnode = NULL;
	char *psfilename;

	psfilename = fpath;

	pdoc = xmlReadFile(psfilename,"utf-8",XML_PARSE_RECOVER);  

	if(NULL == pdoc)
	{
		return 1;
	}

	pcurnode = xmlDocGetRootElement(pdoc);  

	pcurnode = pcurnode->xmlChildrenNode;  

	xmlNodePtr propNodePtr = pcurnode;

	while (NULL != pcurnode)
	{			

		if (!xmlStrcmp(pcurnode->name, BAD_CAST node_name))   
		{

			if (xmlHasProp(pcurnode,BAD_CAST attribute))
			{     
				propNodePtr = pcurnode;	

				xmlAttrPtr attrPtr = propNodePtr->properties;	

				while (attrPtr != NULL)
				{      

					if (!xmlStrcmp(attrPtr->name, BAD_CAST attribute))
					{      		

						xmlChar* szAttr = xmlGetProp(propNodePtr,BAD_CAST attribute);  

						if(!xmlStrcmp(szAttr,BAD_CAST ruler))
						{		
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

						xmlFree(szAttr);

					}
					attrPtr = attrPtr->next;
				}
			}                  
		}        
		pcurnode = pcurnode->next; 

	}	  
	xmlSaveFile(fpath,pdoc);  
	xmlFreeDoc(pdoc);
	return 0;
}

//free link list
void Free_read_dest_xml(struct dest_st *head)
{
    struct dest_st *f1,*f2;
	f1=head->next;
	if(NULL != f1)
	{
		f2=f1->next;
		while(f2!=NULL)
		{
		  free(f1->valuez);
		  free(f1->contentz);
		  free(f1->sysipz);
		  free(f1->sysport);
		  free(f1->proz);
		  free(f1->timeflag);
		  free(f1->indexz);
		  free(f1->flevel);
		  free(f1);
		  f1=f2;
		  f2=f2->next;
		}
		free(f1);
	}
}

int read_dest_xml(struct dest_st *chead,int *confnum)
{
	xmlDocPtr doc;	 
	xmlNodePtr cur,tmp;
    int conflag=0;	
	doc = xmlReadFile(XML_FPATH, "utf-8", 256); 
	if (doc == NULL ) 
	{
		return -1;
	}
	cur = xmlDocGetRootElement(doc); 
	if (cur == NULL)
	{
		xmlFreeDoc(doc);
		return -1;
	}

	if (xmlStrcmp(cur->name, (const xmlChar *) "root")) 
	{
		xmlFreeDoc(doc);
		return -1;
	}

	struct dest_st *ctail=NULL;
	chead->next=NULL;
	ctail=chead;

	cur = cur->xmlChildrenNode;	
	while(cur !=NULL)
	{

	    tmp = cur->xmlChildrenNode;
		if (!xmlStrcmp(cur->name, BAD_CAST NODE_DES))           
		{
		        /////////////conf informations
		        struct dest_st  *cq=NULL;
				cq=(struct dest_st *)malloc(sizeof(struct dest_st)+1);
				memset(cq,0,sizeof(struct dest_st)+1);
				if(NULL == cq)
				{
					return  -1;
				}
				
				cq->valuez=(char *)malloc(50);
				memset(cq->valuez,0,50);
				
				cq->contentz=(char *)malloc(256);
				memset(cq->contentz,0,256);     

				cq->timeflag=(char *)malloc(20);
				memset(cq->timeflag,0,20);

				cq->sysipz=(char *)malloc(50);
				memset(cq->sysipz,0,50);

				cq->sysport=(char *)malloc(10);
				memset(cq->sysport,0,10);

				cq->proz=(char *)malloc(10);
				memset(cq->proz,0,10);
				
				cq->flevel=(char *)malloc(10);
				memset(cq->flevel,0,10);    

				cq->indexz=(char *)malloc(20);
				memset(cq->indexz,0,20);

				/////////////  
                conflag++;
				xmlChar *value=NULL;
				
				while(tmp !=NULL)
				{	 
					//dest value
					if ((!xmlStrcmp(tmp->name, BAD_CAST NODE_VALUE)))
					{
					    memset(cq->valuez,0,50);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->valuez,(char *)value);	
						xmlFree(value);
					}
					
					//dest content
					else if ((!xmlStrcmp(tmp->name, BAD_CAST NODE_CONTENT)))
					{
					    memset(cq->contentz,0,256);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->contentz,(char *)value);	
						xmlFree(value);
					}
					//dest mark
					else if ((!xmlStrcmp(tmp->name, BAD_CAST NODE_MARKZ)))
					{
					    memset(cq->timeflag,0,20);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->timeflag,(char *)value);	
						xmlFree(value);
					}
					//dest sysip
					else if ((!xmlStrcmp(tmp->name, BAD_CAST NODE_SYSIP)))
					{
					    memset(cq->sysipz,0,50);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->sysipz,(char *)value);	
						xmlFree(value);
					}
					//dest sysport
					else if ((!xmlStrcmp(tmp->name, BAD_CAST NODE_SYSPORT)))
					{
					    memset(cq->sysport,0,10);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->sysport,(char *)value);	
						xmlFree(value);
					}
					//dest sysport
					else if ((!xmlStrcmp(tmp->name, BAD_CAST NODE_PROTOCOLZ)))
					{
					    memset(cq->proz,0,10);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->proz,(char *)value);	
						xmlFree(value);
					}		
					//dest flevel
					else if ((!xmlStrcmp(tmp->name, BAD_CAST NODE_FLEVEL)))
					{
					    memset(cq->flevel,0,10);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->flevel,(char *)value);	
						xmlFree(value);
					}
					else if ((!xmlStrcmp(tmp->name, BAD_CAST NODE_INDEXZ)))
					{
					    memset(cq->indexz,0,20);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->indexz,(char *)value);	
						xmlFree(value);
					}
					tmp = tmp->next;
				}            
				 cq->next = NULL;
			     ctail->next = cq;
			     ctail = cq;
		}      
	   cur = cur->next;
	}
    *confnum=conflag;
	xmlFreeDoc(doc);
	return 0;
}

//free link list
void Free_read_log_xml(struct log_st *head)
{
    struct log_st *f1,*f2;
	f1=head->next;		 
	f2=f1->next;
	while(f2!=NULL)
	{
	  free(f1->keyz);
	  free(f1->sourcez);
	  free(f1->filterz);
	  free(f1->des);
	  free(f1->enablez);
	  free(f1->timeflag);
	  free(f1->sysipz);
	  free(f1->sysport);
	  free(f1->flag);
	  free(f1->indexz);
	  free(f1);
	  f1=f2;
	  f2=f2->next;
	}
	free(f1);
}
int read_log_xml(struct log_st *chead,int *confnum)
{
	xmlDocPtr doc;	 
	xmlNodePtr cur,tmp;
    int conflag=0;	
	doc = xmlReadFile(XML_FPATH, "utf-8", 256); 
	if (doc == NULL ) 
	{
		return -1;
	}
	cur = xmlDocGetRootElement(doc); 
	if (cur == NULL)
	{
		xmlFreeDoc(doc);
		return -1;
	}

	if (xmlStrcmp(cur->name, (const xmlChar *) "root")) 
	{
		xmlFreeDoc(doc);
		return -1;
	}

	struct log_st *ctail=NULL;
	chead->next=NULL;
	ctail=chead;

	cur = cur->xmlChildrenNode;	
	while(cur !=NULL)
	{

	    tmp = cur->xmlChildrenNode;
		if (!xmlStrcmp(cur->name, BAD_CAST NODE_LOG))           
		{
		        /////////////conf informations
		        struct log_st  *cq=NULL;
				cq=(struct log_st *)malloc(sizeof(struct log_st)+1);
				memset(cq,0,sizeof(struct log_st)+1);
				if(NULL == cq)
				{
					return  -1;
				}
				
				cq->keyz=(char *)malloc(50);
				memset(cq->keyz,0,50);
				
				cq->sourcez=(char *)malloc(50);
				memset(cq->sourcez,0,50);   

				cq->filterz=(char *)malloc(128);
				memset(cq->filterz,0,128);  
				
				cq->des=(char *)malloc(128);
				memset(cq->des,0,128);    

				cq->enablez=(char *)malloc(10);
				memset(cq->enablez,0,10);    
				
				cq->timeflag=(char *)malloc(20);
				memset(cq->timeflag,0,20);    

				cq->sysipz=(char *)malloc(50);
				memset(cq->sysipz,0,50);    

				cq->sysport=(char *)malloc(10);
				memset(cq->sysport,0,10);    
                cq->indexz=(char *)malloc(20);
				memset(cq->indexz,0,20);
				
/////////////////////////////////////
                conflag++;
				xmlChar *value=NULL;
				
				while(tmp !=NULL)
				{	 
					//log keyz
					if ((!xmlStrcmp(tmp->name, BAD_CAST CH_KEYZ)))
					{
					    memset(cq->keyz,0,50);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->keyz,(char *)value);	
						xmlFree(value);
					}
					//log enables
					else if ((!xmlStrcmp(tmp->name, BAD_CAST NODE_ENABLES)))
					{
					    memset(cq->enablez,0,10);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->enablez,(char *)value);	
						xmlFree(value);
					}
					//log source
					else if ((!xmlStrcmp(tmp->name, BAD_CAST CH_SOURCE)))
					{
					    memset(cq->sourcez,0,50);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->sourcez,(char *)value);	
						xmlFree(value);
					}
					//log filter
					else if ((!xmlStrcmp(tmp->name, BAD_CAST CH_FILTER)))
					{
					    memset(cq->filterz,0,128);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->filterz,(char *)value);	
						xmlFree(value);
					}
					//log des
					else if ((!xmlStrcmp(tmp->name, BAD_CAST CH_DEST)))
					{
					    memset(cq->des,0,128);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->des,(char *)value);	
						xmlFree(value);
					}
					//log timeflag
					else if ((!xmlStrcmp(tmp->name, BAD_CAST NODE_MARKZ)))
					{
					    memset(cq->timeflag,0,20);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->timeflag,(char *)value);	
						xmlFree(value);
					}
					//log sysip
					else if ((!xmlStrcmp(tmp->name, BAD_CAST NODE_SYSIP)))
					{
					    memset(cq->sysipz,0,50);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->sysipz,(char *)value);	
						xmlFree(value);
					}
					//log sysport
					else if ((!xmlStrcmp(tmp->name, BAD_CAST NODE_SYSPORT)))
					{
					    memset(cq->sysport,0,10);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->sysport,(char *)value);	
						xmlFree(value);
					}
					else if ((!xmlStrcmp(tmp->name, BAD_CAST NODE_INDEXZ)))
					{
					    memset(cq->indexz,0,20);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->indexz,(char *)value);	
						xmlFree(value);
					}
					tmp = tmp->next;
				}            
				 cq->next = NULL;
			     ctail->next = cq;
			     ctail = cq;
		}      
	   cur = cur->next;
	}
    *confnum=conflag;
	xmlFreeDoc(doc);
	return 0;
}

//free link list
void Free_read_filter_xml(struct filter_st *head)
{
    struct filter_st *f1,*f2;
	f1=head->next;		 
	f2=f1->next;
	while(f2!=NULL)
	{
	  free(f1->valuez);
	  free(f1->viewz);
	  free(f1->contentz);
	  free(f1->infos);
	  free(f1);
	  f1=f2;
	  f2=f2->next;
	}
	free(f1);
}

int read_filter_xml(struct filter_st *chead,int *confnum)
{
	xmlDocPtr doc;	 
	xmlNodePtr cur,tmp;
    int conflag=0;	
	doc = xmlReadFile(XML_FPATH, "utf-8", 256); 
	if (doc == NULL ) 
	{
		return -1;
	}
	cur = xmlDocGetRootElement(doc); 
	if (cur == NULL)
	{
		xmlFreeDoc(doc);
		return -1;
	}

	if (xmlStrcmp(cur->name, (const xmlChar *) "root")) 
	{
		xmlFreeDoc(doc);
		return -1;
	}

	struct filter_st *ctail=NULL;
	chead->next=NULL;
	ctail=chead;

	cur = cur->xmlChildrenNode;	
	while(cur !=NULL)
	{

	    tmp = cur->xmlChildrenNode;
		if (!xmlStrcmp(cur->name, BAD_CAST NODE_FILTER))           
		{
		        /////////////conf informations
		        struct filter_st  *cq=NULL;
				cq=(struct filter_st *)malloc(sizeof(struct filter_st)+1);
				memset(cq,0,sizeof(struct filter_st)+1);
				if(NULL == cq)
				{
					return  -1;
				}
				
				cq->valuez=(char *)malloc(50);
				memset(cq->valuez,0,50);
				
				cq->viewz=(char *)malloc(10);
				memset(cq->viewz,0,10);   

				cq->contentz=(char *)malloc(128);
				memset(cq->contentz,0,128);  
				
				cq->infos=(char *)malloc(128);
				memset(cq->infos,0,128);     
				/////////////  
                conflag++;
				xmlChar *value=NULL;
				
				while(tmp !=NULL)
				{	 
					//filter value key
					if ((!xmlStrcmp(tmp->name, BAD_CAST NODE_VALUE)))
					{
					    memset(cq->valuez,0,50);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->valuez,(char *)value);	
						xmlFree(value);
					}
					
					//filter views en or dis
					else if ((!xmlStrcmp(tmp->name, BAD_CAST NODE_VIEWS)))
					{
					    memset(cq->viewz,0,10);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->viewz,(char *)value);	
						xmlFree(value);
					}
					//filter content
					else if ((!xmlStrcmp(tmp->name, BAD_CAST NODE_CONTENT)))
					{
					    memset(cq->contentz,0,128);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->contentz,(char *)value);	
						xmlFree(value);
					}
					//filter views
					else if ((!xmlStrcmp(tmp->name, BAD_CAST NODE_INFOS)))
					{
					    memset(cq->infos,0,128);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->infos,(char *)value);	
						xmlFree(value);
					}
					tmp = tmp->next;
				}            
				 cq->next = NULL;
			     ctail->next = cq;
			     ctail = cq;
		}      
	   cur = cur->next;
	}
    *confnum=conflag;
	xmlFreeDoc(doc);
	return 0;
}

//free link list
void Free_read_opt_xml(struct opt_st *head)
{
    struct opt_st *f1,*f2;
	f1=head->next;		 
	f2=f1->next;
	while(f2!=NULL)
	{
	  free(f1->contentz);
	  free(f1);
	  f1=f2;
	  f2=f2->next;
	}
	free(f1);
}

int read_opt_xml(struct opt_st *chead,int *confnum)
{
	xmlDocPtr doc;	 
	xmlNodePtr cur,tmp;
    int conflag=0;	
	doc = xmlReadFile(XML_FPATH, "utf-8", 256); 
	if (doc == NULL ) 
	{
		return -1;
	}
	cur = xmlDocGetRootElement(doc); 
	if (cur == NULL)
	{
		xmlFreeDoc(doc);
		return -1;
	}

	if (xmlStrcmp(cur->name, (const xmlChar *) "root")) 
	{
		xmlFreeDoc(doc);
		return -1;
	}

	struct opt_st *ctail=NULL;
	chead->next=NULL;
	ctail=chead;

	cur = cur->xmlChildrenNode;	
	while(cur !=NULL)
	{

	    tmp = cur->xmlChildrenNode;
		if (!xmlStrcmp(cur->name, BAD_CAST NODE_OPT))           
		{
		        /////////////conf informations
		        struct opt_st  *cq=NULL;
				cq=(struct opt_st *)malloc(sizeof(struct opt_st)+1);
				memset(cq,0,sizeof(struct opt_st)+1);
				if(NULL == cq)
				{
					return  -1;
				}			
				cq->contentz=(char *)malloc(256);
				memset(cq->contentz,0,256); 
				/////////////  
                conflag++;
				xmlChar *value=NULL;
				value=xmlNodeGetContent(cur);
				strcpy(cq->contentz,(char *)value);	
				xmlFree(value);
				
				cq->next = NULL;
				ctail->next = cq;
				ctail = cq;
		}      
	   cur = cur->next;
	}
    *confnum=conflag;
	xmlFreeDoc(doc);
	return 0;
}

//free link list
void Free_read_source_xml(struct source_st *head)
{
    struct source_st *f1,*f2;
	f1=head->next;		 
	f2=f1->next;
	while(f2!=NULL)
	{
	  free(f1->contentz);
	  free(f1->valuez);
	  free(f1);
	  f1=f2;
	  f2=f2->next;
	}
	free(f1);
}

int read_source_xml(struct source_st *chead,int *confnum)
{
	xmlDocPtr doc;	 
	xmlNodePtr cur,tmp;
    int conflag=0;	
	doc = xmlReadFile(XML_FPATH, "utf-8", 256); 
	if (doc == NULL ) 
	{
		return -1;
	}
	cur = xmlDocGetRootElement(doc); 
	if (cur == NULL)
	{
		xmlFreeDoc(doc);
		return -1;
	}

	if (xmlStrcmp(cur->name, (const xmlChar *) "root")) 
	{
		xmlFreeDoc(doc);
		return -1;
	}

	struct source_st *ctail=NULL;
	chead->next=NULL;
	ctail=chead;

	cur = cur->xmlChildrenNode;	
	while(cur !=NULL)
	{

	    tmp = cur->xmlChildrenNode;
		if (!xmlStrcmp(cur->name, BAD_CAST NODE_SOURCE))           
		{
		        /////////////conf informations
		        struct source_st  *cq=NULL;
				cq=(struct source_st *)malloc(sizeof(struct source_st)+1);
				memset(cq,0,sizeof(struct source_st)+1);
				if(NULL == cq)
				{
					return  -1;
				}
				cq->valuez=(char *)malloc(50);
				memset(cq->valuez,0,50);  				

				cq->contentz=(char *)malloc(256);
				memset(cq->contentz,0,256); 				
				
				/////////////  
                conflag++;
				xmlChar *value=NULL;
				
				while(tmp !=NULL)
				{	 
					//source value
					if ((!xmlStrcmp(tmp->name, BAD_CAST NODE_VALUE)))
					{
					    memset(cq->valuez,0,50);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->valuez,(char *)value);	
						xmlFree(value);
					}		
					//source content
					else if ((!xmlStrcmp(tmp->name, BAD_CAST NODE_CONTENT)))
					{
					    memset(cq->contentz,0,256);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->contentz,(char *)value);	
						xmlFree(value);
					}					
					tmp = tmp->next;
				}            
				 cq->next = NULL;
			     ctail->next = cq;
			     ctail = cq;
		}      
	   cur = cur->next;
	}
    *confnum=conflag;
	xmlFreeDoc(doc);
	return 0;
}
/*xml的内容，节点无属性，fpath:xml文件路径，nodename，二级节点名称，content，三级节点名称，newc，更改的新内容*/
int get_second_xmlnode(char * fpath,char * node_name,char * content,char *gets,int flagz)
{
	xmlDocPtr pdoc = NULL;
	xmlNodePtr pcurnode = NULL;
	char *psfilename;
	psfilename = fpath;
	pdoc = xmlReadFile(psfilename,"utf-8",XML_PARSE_RECOVER);  

	int tagz = 0;

	if(NULL == pdoc) {
		return 1;
	}

	pcurnode = xmlDocGetRootElement(pdoc);  	
    if(NULL == pcurnode) {
        new_xml_file(psfilename);
        return 1;
    }

	pcurnode = pcurnode->xmlChildrenNode;  

	while (NULL != pcurnode)
	{			
		if (!xmlStrcmp(pcurnode->name, BAD_CAST node_name))   
		{
		    tagz ++ ;
			xmlNodePtr childPtr = pcurnode; 
			childPtr=childPtr->children;
			xmlChar *value;
			
			while(childPtr !=NULL)
			{	 
				if ((!xmlStrcmp(childPtr->name, BAD_CAST content)))
				{
					value = xmlNodeGetContent(childPtr);   
					if( tagz == flagz )
					{
						strcpy(gets,(char *)value);
					}
					xmlFree(value);
				}
				childPtr = childPtr->next;
			}
		}        
		pcurnode = pcurnode->next; 
	}	  
	xmlSaveFile(fpath,pdoc);  
	xmlFreeDoc(pdoc);
	return 0;
}

/*xml的内容，节点无属性，fpath:xml文件路径，nodename，二级节点名称，content，三级节点名称，newc，更改的新内容*/
int mod_second_xmlnode(char * fpath,char * node_name,char * content,char *newc,int flagz)
{
	xmlDocPtr pdoc = NULL;
	xmlNodePtr pcurnode = NULL;
	char *psfilename;
	psfilename = fpath;
	pdoc = xmlReadFile(psfilename,"utf-8",XML_PARSE_RECOVER);  

	int tagz = 0;

	if(NULL == pdoc)
	{
		return 1;
	}

	pcurnode = xmlDocGetRootElement(pdoc);  
	pcurnode = pcurnode->xmlChildrenNode;  

	while (NULL != pcurnode)
	{			
		if (!xmlStrcmp(pcurnode->name, BAD_CAST node_name))   
		{
            tagz ++ ;
			xmlNodePtr childPtr = pcurnode; 
			childPtr=childPtr->children;
			xmlChar *value;
			
			while(childPtr !=NULL)
			{	 
				if ((!xmlStrcmp(childPtr->name, BAD_CAST content)))
				{
				    
					value = xmlNodeGetContent(childPtr);   
					if( tagz == flagz )
					{
						xmlNodeSetContent(childPtr, BAD_CAST  newc); 
					}
					xmlFree(value);
				}
				childPtr = childPtr->next;
			}
		}        
		pcurnode = pcurnode->next; 
	}	  
	xmlSaveFile(fpath,pdoc);  
	xmlFreeDoc(pdoc);
	return 0;
}
/*modify log enables xml的内容，节点无属性，fpath:xml文件路径，nodename，二级节点名称，content，三级节点名称，newc，更改的新内容*/
int mod_log_enables(char * fpath,char * node_name,char * content,char *newc)
{
	xmlDocPtr pdoc = NULL;
	xmlNodePtr pcurnode = NULL;
	char *psfilename;
	psfilename = fpath;
	pdoc = xmlReadFile(psfilename,"utf-8",256);  

	int tagz = 0;

	if(NULL == pdoc)
	{
		return 1;
	}

	pcurnode = xmlDocGetRootElement(pdoc);  
	pcurnode = pcurnode->xmlChildrenNode;  

	while (NULL != pcurnode)
	{			
		if (!xmlStrcmp(pcurnode->name, BAD_CAST node_name))   
		{
            tagz ++ ;
			xmlNodePtr childPtr = pcurnode; 
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
	xmlSaveFile(fpath,pdoc);  
	xmlFreeDoc(pdoc);
	return 0;
}


/*查找指定节点，fpath，xml文件路径，nodename，二级xml节点名，content，三级xml节点名，keyz，三级xml节点名下内容，flagz，标记xml*/
int find_second_xmlnode(char * fpath,char * node_name,char * content,char *keyz,int *flagz)
{
	xmlDocPtr pdoc = NULL;
	xmlNodePtr pcurnode = NULL;
	char *psfilename;
	psfilename = fpath;
	*flagz = 0;
	int tagz = 0,freetag = -1;
	pdoc = xmlReadFile(psfilename,"utf-8",256); 

	if(NULL == pdoc)
	{
		return 1;
	}

	pcurnode = xmlDocGetRootElement(pdoc);   
	pcurnode = pcurnode->xmlChildrenNode;  //root---->des

	while (NULL != pcurnode)
	{			
		if (!xmlStrcmp(pcurnode->name, BAD_CAST node_name))       //root--->des      
		{

            tagz ++;
			xmlNodePtr childPtr = pcurnode; 
			childPtr=childPtr->children;
			xmlChar *value;

			while(childPtr !=NULL)
			{	 
				if(!xmlStrcmp(childPtr->name, BAD_CAST content))
				{
				    
					value = xmlNodeGetContent(childPtr);      
					if(strcmp(keyz,(char *)value)==0)
					{
						freetag = 0;
						break;
					}
					xmlFree(value);
				}
				childPtr = childPtr->next;
			}
			if(freetag==0)
			{
				xmlFree(value);
			}
		}    
		
		if(freetag==0)
			break;
		
		pcurnode = pcurnode->next; 
	}	  
	if(freetag==0)//find
		*flagz = tagz;
	else
		*flagz = 0;
	xmlSaveFile(fpath,pdoc);  
	xmlFreeDoc(pdoc);
	return 0;
}

/*添加二级节点并内容fpath:xml file path,node_name:second level node,content:sub second node name,keyz:node content*/
int add_second_xmlnode(char * fpath,char * node_name,ST_SYS_ALL *sysall)
{
    int numz=0,i=0;
	numz=sizeof(sysall->suinfo)/sizeof(sysall->suinfo[0]);
	
	xmlDocPtr pdoc = NULL;
	xmlNodePtr pcurnode = NULL;
	char *psfilename;
	psfilename = fpath;
	pdoc = xmlReadFile(psfilename,"utf-8",256); 

	if(NULL == pdoc)
	{
		return 1;
	}

	pcurnode = xmlDocGetRootElement(pdoc);   

    xmlNodePtr node = xmlNewNode(NULL,BAD_CAST node_name);  
	xmlAddChild(pcurnode,node);
	
	for(i=0;i<numz;i++)
	{
	    if(strcmp(sysall->suinfo[i].suname,"")!=0)
		{
			xmlNodePtr node1 = xmlNewNode(NULL,BAD_CAST sysall->suinfo[i].suname);
			xmlNodePtr content1 = xmlNewText(BAD_CAST sysall->suinfo[i].content);
			xmlAddChild(node,node1);
			xmlAddChild(node1,content1);
		}
	}	
	xmlSaveFile(fpath,pdoc); 
	xmlFreeDoc(pdoc);
	return 0;
}

/*删除二级节点并内容fpath:xml file path,node_name:second level node,content:sub second node name,keyz:node content*/
int del_second_xmlnode(char * fpath,char * node_name,int flagz)
{
	xmlDocPtr pdoc = NULL;
	xmlNodePtr pcurnode = NULL;
	char *psfilename;
	psfilename = fpath;
	pdoc = xmlReadFile(psfilename,"utf-8",XML_PARSE_RECOVER);  

	int tagz = 0;

	if(NULL == pdoc)
	{
		return 1;
	}

	pcurnode = xmlDocGetRootElement(pdoc);  
	pcurnode = pcurnode->xmlChildrenNode;  

	while (NULL != pcurnode)
	{			
		if (!xmlStrcmp(pcurnode->name, BAD_CAST node_name))   
		{
            tagz ++;
			if( tagz == flagz )
			{
				xmlNodePtr tempNode;				  		   

				tempNode = pcurnode->next;  

				xmlUnlinkNode(pcurnode);

				xmlFreeNode(pcurnode);

				pcurnode= tempNode;			   

				continue;
			}
		}        
		pcurnode = pcurnode->next; 
	}	  
	xmlSaveFile(fpath,pdoc);  
	xmlFreeDoc(pdoc);
	return 0;
}

/*find designed node 一个节点的属性如果不存在这个节点，做法 */

int find_log_node(char * fpath,char * node_name,char *attribute,char *ruler,char * content,ST_LOG_KEY *logkey)
{
	xmlDocPtr pdoc = NULL;

	xmlNodePtr pcurnode = NULL;
	char *psfilename;

	psfilename = fpath;

	pdoc = xmlReadFile(psfilename,"utf-8",256); 

	if(NULL == pdoc)
	{
		return 1;
	}

	pcurnode = xmlDocGetRootElement(pdoc);   

	pcurnode = pcurnode->xmlChildrenNode;  

	xmlNodePtr propNodePtr = pcurnode;

	while (NULL != pcurnode)
	{			

		if (!xmlStrcmp(pcurnode->name, BAD_CAST node_name))             
		{

			if (xmlHasProp(pcurnode,BAD_CAST attribute))
			{     
				propNodePtr = pcurnode;	

				xmlAttrPtr attrPtr = propNodePtr->properties;	

				while (attrPtr != NULL)  
				{      

					if (!xmlStrcmp(attrPtr->name, BAD_CAST attribute))
					{      		

						xmlChar* szAttr = xmlGetProp(propNodePtr,BAD_CAST attribute);  

						if(!xmlStrcmp(szAttr,BAD_CAST ruler))  
						{	

							xmlNodePtr childPtr = propNodePtr; 
							childPtr=childPtr->children;
							xmlChar *value;

							while(childPtr !=NULL)
							{	 
								if ((!xmlStrcmp(childPtr->name, BAD_CAST content)))
								{
									value = xmlNodeGetContent(childPtr);       
									strcpy(logkey->key,(char *)value);			 
									xmlFree(value);
								}

								childPtr = childPtr->next;
							}
						}
						xmlFree(szAttr);
					}
					attrPtr = attrPtr->next;
				}
			}                  
		}        
		pcurnode = pcurnode->next; 
	}	  
	xmlSaveFile(fpath,pdoc);  
	xmlFreeDoc(pdoc);
	return 0;
}

/*切割；*/
int  cut_up(char *dstring,ST_LOG_KEY *keys,char *ruler)
{
	char input[256];
	memset(input,0,256);

	strcpy(input,dstring);

	char *p;

	char des[128];

	char output[512];
	memset(output,0,512);

	p=strtok(input,";");

	while( p != NULL ) 
	{   
		memset(des,0,128);   
		sprintf(des,"%s (%s);",ruler,p);
		strcat(output,des);
		p = strtok( NULL, ";"); 
	} 
	strcpy(keys->key,output);
	return 0;	
}

char *first_ip(char *dstring)
{
	char input[256];
	memset(input,0,256);

	strcpy(input,dstring);

	char *p;

	p=strtok(input,";"); 
	if(p)
		return p;
	else
		return NULL;
}


char *first_port(char *dstring)
{
	char input[256];
	memset(input,0,256);
	char *output;
	strcpy(input,dstring);

	output=first_ip(dstring);
	char *p;

	p=strtok(output,"("); 
	if(p)

	p=strtok(NULL,";");

	return p;
}

/*syslog的ip中进行切割；*/
int  cut_up_ip(char *dstring,char *ip)
{
	char *input;
	char *p;
	char temp[60];
	memset(temp,0,60);

	input=first_ip(dstring);  

	p=strtok(input,"\"");
	if(p)
		
	p=strtok(NULL,"\"");

	strcpy(ip,p);

	return 0;	
}


/*syslog的port中进行切割；*/
int  cut_up_port(char *dstring,char *port)
{
	char *input;
	char *p;

	input=first_port(dstring);  

	p=strtok(input,"(");
	if(p)
		
	p=strtok(NULL,")");

	strcpy(port,p);

	return 0;	
}

//ip的替换函数,输入源加上端口，新的ip，返回的是修改后的udp
char *replace_ip(char *strbuf, char *sstr, char *dstr)
{      
	char *p,*p1;
	int len;

	if ((strbuf == NULL)||(sstr == NULL)||(dstr == NULL))
	return NULL;

	p = strstr(strbuf, sstr);      
	if (p == NULL)  /*not found*/
	return NULL;

	len = strlen(strbuf) + strlen(dstr) - strlen(sstr);
	p1 = malloc(len);
	bzero(p1, len);
	strncpy(p1, strbuf, p-strbuf);
	strcat(p1, dstr);
	p += strlen(sstr);
	strcat(p1, p);
	return p1;
} 

/*存储多个ip的函数*/
int ip_save(char *fpath,char *value)
{
	FILE *fp;
	char buff[128];
	memset(buff,0,128);
	strcpy(buff,value);
	int ret,op_ret;

	ret=is_file_exist(fpath);
	if(ret==0)
	{
		op_ret=line_num(fpath,"wc -l");
		if(op_ret>4)
		return 2;
		else
		{
			fp=fopen(fpath,"at");
			if(fp==NULL)
			return 1;     
			else
			{
				fwrite(buff,strlen(buff),1,fp);	
				fclose(fp);
			}  
		}

	}else
	{
		fp=fopen(fpath,"wt");
		if(fp==NULL)
		return 1;    
		else
		{
			fwrite(buff,strlen(buff),1,fp);	
			fclose(fp);
		}
	}
	return 0;
}

 //关于查看行数
int  line_num(char *fpath,char *cmd)  
{

	FILE *fp;
	char buff[128];
	char temp[30];
	memset(temp,0,30);
	sprintf(temp,"%s %s",cmd,fpath);

	fp=popen(temp,"r");
	if(fp != NULL)
	{
		fgets( buff, sizeof(buff), fp );  
	}


	char input[128];
	memset(input,0,128);
	strcpy(input,buff);

	char *p;
	int i;

	p=strtok(input," ");
	if(p)
	i=atoi(p);

	if(fp != NULL)
	{
		pclose(fp); 
	}

	return i;	

}

/*关于获取ip 内容*/
int get_ip(char *fpath,ST_IP_CONTENT * ip)
{
	FILE *fp;
	char buff[128];
	fp=fopen(fpath,"r");

	if(fp==NULL)  
	return 1; 

	else
	{
		fgets( buff, sizeof(buff), fp );						 
		do
		{	
			strcat(ip->content,buff);
			fgets( buff, sizeof(buff), fp ); 									   
		}while( !feof(fp) ); 					   

		fclose(fp);
	}
	return 0;
}


/*测试是否含有字母*/
int check_abc_d(char *ff)
{
 int pf,i,j;
 pf = strtoul(ff,0,10);	
 char *df;
 df=(char *)malloc(sizeof(ff));
 
 sprintf(df,"%d",pf);
 
 i=strlen(ff);
 j=strlen(df);
 free(df);
 
 if(i!=j)
 	return -1;  
 else
     return 0;

}

/*判断一个字符串是否是另一个的子串函数*/
int if_subs(char *zstring,char *subs)
{
char input[256];
memset(input,0,256);

strcpy(input,zstring);

char *p;

p=strtok(input,";");

while( p != NULL ) 
{   
	   if(strcmp(subs,p)==0)
   	   {
		   	 return 2;
		      break;
	   	}
	   p = strtok( NULL, ";"); 
} 
return 0;
}

///////////////////////////////////////////////////////////////

/*读取ntp的xml文件信息 not use*/
int read_ntp(char * name, ST_SYS_ALL *sysall)
{
	xmlDocPtr pdoc = NULL;

	xmlNodePtr pcurnode = NULL;
	char *psfilename;

	psfilename = name;

	memset(sysall, 0, sizeof(ST_SYS_ALL) );

	pdoc = xmlReadFile(psfilename,"utf-8",256); 

	if(NULL == pdoc)
	{

		return 1;

	}  
	pcurnode = xmlDocGetRootElement(pdoc);  

	//xmlChar *key;
	xmlChar *value;
	xmlChar *content;
	int i=0,j=0,n=0,p=0;
	pcurnode=pcurnode->xmlChildrenNode;  

	while (pcurnode != NULL)   
	{     


		//drift 
		if ((!xmlStrcmp(pcurnode->name, BAD_CAST  NTP_DRIFT)))
		{   
			value = xmlNodeGetContent(pcurnode);	
			sysall->optinfo[n].content=(char *)malloc(xmlStrlen(value));	
			strcpy(sysall->optinfo[n].content,(char *)value);	
			xmlFree(value);

			n++;
		}  

		////////////////////////////////////////////////////////////////////////////////


		//server 
		if ((!xmlStrcmp(pcurnode->name, BAD_CAST NTP_SERV)))
		{    

			xmlNodePtr testnode;

			testnode=pcurnode;

			testnode=testnode->children;
			while(testnode !=NULL)
			{	 

				//server

				if ((!xmlStrcmp(testnode->name, BAD_CAST  NODE_VALUE)))   
				{
					value = xmlNodeGetContent(testnode);	
					strcpy(sysall->suinfo[p].suname,(char *)value);	
					xmlFree(value);
				}	

				//server

				if ((!xmlStrcmp(testnode->name, BAD_CAST NODE_CONTENT)))  
				{
					value = xmlNodeGetContent(testnode);
					strcpy(sysall->suinfo[p].content,(char *)value);	
					xmlFree(value);
				}	
				testnode = testnode->next;	  
			}	   
			p++;
		}  


		////////////////////////////////////////////////////////////////////////////////

		//rest  
		if ((!xmlStrcmp(pcurnode->name, BAD_CAST NTP_REST)))
		{    

			xmlNodePtr testnode;

			testnode=pcurnode;

			testnode=testnode->children;
			while(testnode !=NULL)
			{	 

				//rest
				if ((!xmlStrcmp(testnode->name, BAD_CAST  NODE_VALUE)))   
				{
					value = xmlNodeGetContent(testnode);	
					strcpy(sysall->desinfo[i].rname,(char *)value);	
					xmlFree(value);
				}

				if ((!xmlStrcmp(testnode->name, BAD_CAST  NODE_CONTENT))) 
				{
					content = xmlNodeGetContent(testnode);		 
					strcpy(sysall->desinfo[i].content,(char *)value);

					xmlFree(content);
				}


				testnode = testnode->next;

			}	    
			i++;
		}  


		////////////////////////////////////////////////////////////////////////////////

		//broadcast   
		if ((!xmlStrcmp(pcurnode->name, BAD_CAST NTP_BROD)))
		{    
			xmlNodePtr testnode;

			testnode=pcurnode;

			testnode=testnode->children;
			while(testnode !=NULL)
			{	 

				//broadcast
				if ((!xmlStrcmp(testnode->name, BAD_CAST NODE_VALUE)))   
				{
					value = xmlNodeGetContent(testnode);	
					strcpy(sysall->finfo[j].fname,(char *)value);	
					xmlFree(value);
				}

				if ((!xmlStrcmp(testnode->name, BAD_CAST NODE_CONTENT))) 
				{
					content = xmlNodeGetContent(testnode);		 
					strcpy(sysall->finfo[j].content,(char *)value);

					xmlFree(content);
				}
				testnode = testnode->next;	  
			}	    
			j++;
		}  

		pcurnode = pcurnode->next;
	} 
	sysall->des_num=i;
	sysall->f_num=j;
	sysall->su_num=p;
	sysall->opt_num=n;

	xmlFreeDoc(pdoc);
	xmlCleanupParser();
	return 0;
}

/*写入到ntp的conf中函数 not use*/
int write_ntp( ST_SYS_ALL *sysall, char *file_path) 
{

	FILE * fp;	
	char File_content[10000],des_name[50],content[1024];
	memset(File_content,0,10000);
	memset(des_name,0,50);

	ST_LOG_KEY keys;  
	memset(&keys,0,sizeof(keys));


	int i;

	if( NULL == file_path)
	{
		return -1 ;  
	}
	if(( fp = fopen(file_path,"w+"))==NULL)
	return -2;

	
	///////////////////////////////////////////////////////////

	//restrict 0.0.0.0 mask  0.0.0.0  parameter
	//循环读取结构体中内容来写入到文件中去  rest 配置

	for(i=0;i<sysall->des_num;i++)
	{
		memset(des_name,0,50);
		sprintf(des_name , "restrict %s",sysall->desinfo[i].rname);

		strcat(File_content,des_name);	


		memset(content,0,1024);			
		sprintf(content , " %s\n",sysall->desinfo[i].content);
		strcat(File_content,content);		

	}
	///////////////////////////////////////////////////////////
	//server 0.0.0.0 perferd

	//循环读取结构体中内容来写入到文件中去  server
	for(i=0;i<sysall->su_num;i++)
	{
		memset(des_name,0,50);
		sprintf(des_name , "server  %s",sysall->suinfo[i].suname);

		strcat(File_content,des_name);	

		memset(content,0,1024);			
		sprintf(content , " %s\n",sysall->suinfo[i].content);
		strcat(File_content,content);		

	}

	///////////////////////////////////////////////////////////

	//restrict 0.0.0.0
	//循环读取结构体中内容来写入到文件中去   broad配置

	for(i=0;i<sysall->f_num;i++)
	{
		
		#if 0
		memset(des_name,0,50);
		sprintf(des_name , "fudge %s stratum 1\n",sysall->finfo[i].fname);
		strcat(File_content,des_name);	
        #endif
        
		memset(des_name,0,50);
		sprintf(des_name , "restrict %s\n",sysall->finfo[i].fname);
		strcat(File_content,des_name);	
        

	}
	#if 0
	//////////////////////////////////////////////////////////////////
	for(i=0;i<sysall->opt_num;i++)
	{
		memset(content,0,1024);			
		sprintf(content , "driftfile %s \n",((sysall->optinfo[i]).content));		
		strcat(File_content,content);	

	}

	for(i=0;i<(sysall->opt_num);i++)
	{
		free((sysall->optinfo[i]).content);

	}
    #endif
	fwrite(File_content,strlen(File_content),1,fp);	
	fclose(fp);
	return 0;
}

/*增加ntp的xml文件中一个节点,不带节点属性的 not use*/
int add_ntp_node(char *fpath,char * node_name,char * value,char *content)
{
	xmlDocPtr pdoc = NULL;

	xmlNodePtr pcurnode = NULL; 

	char *psfilename;

	psfilename = fpath;

	pdoc = xmlReadFile(psfilename,"utf-8",256); 

	if(NULL == pdoc)
	{
		return 1;
	}

	pcurnode = xmlDocGetRootElement(pdoc);  

	xmlNodePtr node = xmlNewNode(NULL,BAD_CAST node_name);    

	xmlAddChild(pcurnode,node);

	xmlNodePtr node1 = xmlNewNode(NULL,BAD_CAST NODE_VALUE);

	xmlNodePtr content1 = xmlNewText(BAD_CAST value);

	xmlNodePtr node2 = xmlNewNode(NULL,BAD_CAST NODE_CONTENT);

	xmlNodePtr content2 = xmlNewText(BAD_CAST  content);


	xmlAddChild(node,node1);

	xmlAddChild(node,node2);

	xmlAddChild(node1,content1);

	xmlAddChild(node2,content2);

	xmlSaveFile(fpath,pdoc);  
	xmlFreeDoc(pdoc);
	
	return 0;
}


/*增加ntp的xml文件中一个节点，带节点属性的  not use*/
int add_ntp_node_new(char *fpath,char * node_name,char * value,char *content,char *attribute)
{
	xmlDocPtr pdoc = NULL;

	xmlNodePtr pcurnode = NULL; 

	char *psfilename;

	psfilename = fpath;

	pdoc = xmlReadFile(psfilename,"utf-8",256);  

	if(NULL == pdoc)
	{
		return 1;
	}

	pcurnode = xmlDocGetRootElement(pdoc); 


	xmlNodePtr node = xmlNewNode(NULL,BAD_CAST node_name);    

	xmlAddChild(pcurnode,node);

	xmlNodePtr node1 = xmlNewNode(NULL,BAD_CAST NODE_VALUE);

	xmlNodePtr content1 = xmlNewText(BAD_CAST value);

	xmlNodePtr node2 = xmlNewNode(NULL,BAD_CAST NODE_CONTENT);

	xmlNodePtr content2 = xmlNewText(BAD_CAST  content);


	xmlAddChild(node,node1);

	xmlAddChild(node,node2);

	xmlAddChild(node1,content1);

	xmlAddChild(node2,content2);

	xmlSetProp(node,NODE_ATT, attribute);

	xmlSaveFile(fpath,pdoc); 
	xmlFreeDoc(pdoc);

	return 0;
}

/*删除指定节点，指定属性的节点 not use*/
int ntp_del(char *fpath,char *node_name,char *attribute,char *key)
{
	xmlDocPtr pdoc = NULL;

	xmlNodePtr pcurnode = NULL;
	char *psfilename;

	psfilename = fpath;

	pdoc = xmlReadFile(psfilename,"utf-8",256); 

	if(NULL == pdoc)
	{
		return 1;
	}

	pcurnode = xmlDocGetRootElement(pdoc);  

	pcurnode = pcurnode->xmlChildrenNode;  

	while (NULL != pcurnode)
	{			

		if (!xmlStrcmp(pcurnode->name, BAD_CAST node_name))  
		{

			if (xmlHasProp(pcurnode,BAD_CAST attribute))
			{     
				//删除newNode1 ok 

				if (!xmlStrcmp(pcurnode->name, BAD_CAST node_name))  
				{            

					xmlNodePtr tempNode;				  		   

					tempNode = pcurnode->next;  

					xmlUnlinkNode(pcurnode);

					xmlFreeNode(pcurnode);

					pcurnode= tempNode;			   

					continue;

				} 	 

			}
		}        
		pcurnode = pcurnode->next; 

	}	  
	xmlSaveFile(fpath,pdoc); 
	xmlFreeDoc(pdoc);
	return 0;
}


//查看指定节点的值
/*find designed node 一个节点的属性如果不存在这个节点，做法 not use*/

int find_ntp_node(char * fpath,char * node_name,char *attribute,char *ruler,char * content,char *logkey)
{
	xmlDocPtr pdoc = NULL;

	xmlNodePtr pcurnode = NULL;
	char *psfilename;

	int flag=-1;

	psfilename = fpath;

	pdoc = xmlReadFile(psfilename,"utf-8",256); 

	if(NULL == pdoc)
	{
		return 1;
	}

	pcurnode = xmlDocGetRootElement(pdoc); 

	pcurnode = pcurnode->xmlChildrenNode;

	xmlNodePtr propNodePtr = pcurnode;

	while (NULL != pcurnode)
	{			

		if (!xmlStrcmp(pcurnode->name, BAD_CAST node_name))   
		{

			if (xmlHasProp(pcurnode,BAD_CAST attribute))
			{     
				propNodePtr = pcurnode;	

				xmlAttrPtr attrPtr = propNodePtr->properties;	

				while (attrPtr != NULL)  
				{      

					if (!xmlStrcmp(attrPtr->name, BAD_CAST attribute))
					{      		

						xmlChar* szAttr = xmlGetProp(propNodePtr,BAD_CAST attribute); 

						if(!xmlStrcmp(szAttr,BAD_CAST ruler))  
						{	

							xmlNodePtr childPtr = propNodePtr; 
							childPtr=childPtr->children;
							xmlChar *value;

							while(childPtr !=NULL)
							{	 
								if ((!xmlStrcmp(childPtr->name, BAD_CAST content)))
								{
									value = xmlNodeGetContent(childPtr);      
									strcpy(logkey,(char *)value);	
									flag=0;
									xmlFree(value);
								}

								childPtr = childPtr->next;
							}
						}

						xmlFree(szAttr);

					}
					attrPtr = attrPtr->next;
				}
			} 

		}  
		pcurnode = pcurnode->next; 
	}	  
	xmlSaveFile(fpath,pdoc); 
	xmlFreeDoc(pdoc);
	//return 0;
	return flag;
}


//修改指定节点的值 not use
int mod_ntp_node(char * fpath,char * node_name,char *attribute,char *ruler,char * content,char *newc)
{
	xmlDocPtr pdoc = NULL;

	xmlNodePtr pcurnode = NULL;
	char *psfilename;

	psfilename = fpath;

	pdoc = xmlReadFile(psfilename,"utf-8",XML_PARSE_RECOVER);  

	if(NULL == pdoc)
	{
		return 1;
	}

	pcurnode = xmlDocGetRootElement(pdoc); 

	pcurnode = pcurnode->xmlChildrenNode;  

	xmlNodePtr propNodePtr = pcurnode;

	while (NULL != pcurnode)
	{			

		if (!xmlStrcmp(pcurnode->name, BAD_CAST node_name))   
		{

			if (xmlHasProp(pcurnode,BAD_CAST attribute))
			{     
				propNodePtr = pcurnode;	

				xmlAttrPtr attrPtr = propNodePtr->properties;	

				while (attrPtr != NULL)
				{      

					if (!xmlStrcmp(attrPtr->name, BAD_CAST attribute))
					{      		

						xmlChar* szAttr = xmlGetProp(propNodePtr,BAD_CAST attribute); 

						if(!xmlStrcmp(szAttr,BAD_CAST ruler))
						{		
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

						xmlFree(szAttr);

					}
					attrPtr = attrPtr->next;
				}
			}                  
		}        
		pcurnode = pcurnode->next; 

	}	  
	xmlSaveFile(fpath,pdoc); 
	xmlFreeDoc(pdoc);
	return 0;
}
//get cli-log on|off

int get_cli_syslog_state()
{
	FILE *fp=NULL;
	char *ptr=NULL;
	int ret=0;

	fp = fopen(VTYSH_CLI_LOG_CONFIG_FILE,"r");
	
	if(!fp)
		return 0;
	ptr=malloc(8);
	memset(ptr,0,8);
	if(!ptr)
	{
		fclose(fp);
		return 0;
	}
	fgets(ptr,8,fp);
	ret = atoi(ptr);
	free(ptr);
	fclose(fp);
	return ret;
	
}

int add_syslog_server( char *xml_fpath, int enable, char *ip, int port, int filter ,char *utcp,int indexz)
{
	ST_SYS_ALL sysall,logall;	
    time_t now;
	unsigned int timez = (unsigned int)time(&now);
	memset( &sysall, 0, sizeof(ST_SYS_ALL) );
	sprintf( sysall.suinfo[0].suname, "%s",NODE_ENABLES );
	sprintf( sysall.suinfo[0].content, "%d",enable );
	
	sprintf( sysall.suinfo[1].suname,  "%s",CH_SOURCE);
	sprintf( sysall.suinfo[1].content,  "%s","s_all" );
	
	sprintf( sysall.suinfo[2].suname,  "%s",CH_FILTER);
	sprintf( sysall.suinfo[2].content,  "%s",filterlist[filter] );
	
	sprintf( sysall.suinfo[3].suname, "%s", CH_DEST);
	sprintf( sysall.suinfo[3].content, "df_server_%d", timez);
	
	sprintf( sysall.suinfo[4].suname,  "%s",NODE_MARKZ);
	sprintf( sysall.suinfo[4].content, "%d", timez);
	
	sprintf( sysall.suinfo[5].suname,  "%s",NODE_SYSIP);
	sprintf( sysall.suinfo[5].content, "%s", ip);
	
	sprintf( sysall.suinfo[6].suname,  "%s",NODE_SYSPORT);
	sprintf( sysall.suinfo[6].content, "%d", port);	
	
	sprintf( sysall.suinfo[7].suname,  "%s",NODE_INDEXZ);
	sprintf( sysall.suinfo[7].content, "%d", indexz);	
	
	add_second_xmlnode( xml_fpath, NODE_LOG, &sysall);

	
	memset( &logall, 0, sizeof(ST_SYS_ALL) );
	sprintf( logall.suinfo[0].suname,  "%s",NODE_VALUE );
	sprintf( logall.suinfo[0].content, "df_server_%d", timez );
	
	sprintf( logall.suinfo[1].suname,  "%s",NODE_CONTENT );
	sprintf( logall.suinfo[1].content, "udp(\"%s\" port(%d));", ip, port );
	
	sprintf( logall.suinfo[2].suname,  "%s",NODE_MARKZ);
	sprintf( logall.suinfo[2].content, "%d", timez);
	
	sprintf( logall.suinfo[3].suname,  "%s",NODE_SYSIP);
	sprintf( logall.suinfo[3].content, "%s", ip);
	
	sprintf( logall.suinfo[4].suname,  "%s",NODE_SYSPORT);
	sprintf( logall.suinfo[4].content, "%d", port);

	sprintf( logall.suinfo[5].suname,  "%s",NODE_PROTOCOLZ);
	sprintf( logall.suinfo[5].content, "%s", utcp);

	sprintf( logall.suinfo[6].suname,  "%s",NODE_INDEXZ);
	sprintf( logall.suinfo[6].content, "%d", indexz);

	char newc[20] = {0};
	int levelz = 0;
	get_first_xmlnode(XML_FPATH, NODE_FLEVEL, &newc);
	levelz = strtoul(newc,0,10);
	if( levelz == 0 )
	{
		levelz  = 1;
	}	
	sprintf( logall.suinfo[6].suname,  "%s",NODE_FLEVEL);
	sprintf( logall.suinfo[6].content, "%d", levelz);	

	sprintf( logall.suinfo[7].suname,  "%s",NODE_INDEXZ);
	sprintf( logall.suinfo[7].content, "%s", indexz);
	
	add_second_xmlnode( xml_fpath, NODE_DES, &logall);
	return 0;
}

int add_syslog_serve_web( char *xml_fpath, char *enable, char *ip,char *port, char *filter ,char *utcp,char *flevel,char *indexz)
{
	ST_SYS_ALL sysall,logall;	
    time_t now;
	int fid = 0;
	char destmp[128] = {0};
	sprintf(destmp,"d%s",filter);
	find_second_xmlnode(XML_FPATH, NODE_DES, NODE_VALUE, destmp, &fid);	

	unsigned int timez = (unsigned int)time(&now);
	memset( &sysall, 0, sizeof(ST_SYS_ALL) );
	sprintf( sysall.suinfo[0].suname, "%s",NODE_ENABLES );
	sprintf( sysall.suinfo[0].content, "%s",enable );
	
	sprintf( sysall.suinfo[1].suname,  "%s",CH_SOURCE);
	sprintf( sysall.suinfo[1].content,  "%s","s_all" );
	
	sprintf( sysall.suinfo[2].suname,  "%s",CH_FILTER);
	sprintf( sysall.suinfo[2].content,  "%s",filter);

	if(fid == 0)
	{
		sprintf( sysall.suinfo[3].suname, "%s", CH_DEST);
		sprintf( sysall.suinfo[3].content, "df_server_%d", timez);
	}
	else
	{
		sprintf( sysall.suinfo[3].suname, "%s", CH_DEST);
		sprintf( sysall.suinfo[3].content, "df_server_%d;%s", timez,destmp);
	}
	
	sprintf( sysall.suinfo[4].suname,  "%s",NODE_MARKZ);
	sprintf( sysall.suinfo[4].content, "%d", timez);
	
	sprintf( sysall.suinfo[5].suname,  "%s",NODE_SYSIP);
	sprintf( sysall.suinfo[5].content, "%s", ip);
	
	sprintf( sysall.suinfo[6].suname,  "%s",NODE_SYSPORT);
	sprintf( sysall.suinfo[6].content, "%s", port);	
	
	sprintf( sysall.suinfo[7].suname,  "%s",NODE_INDEXZ);
	sprintf( sysall.suinfo[7].content, "%s", indexz);	
	
	add_second_xmlnode( xml_fpath, NODE_LOG, &sysall);
	
	memset( &sysall, 0, sizeof(ST_SYS_ALL) );
	sprintf( sysall.suinfo[0].suname, "%s",NODE_ENABLES );
	sprintf( sysall.suinfo[0].content, "%s",enable );
	
	sprintf( sysall.suinfo[1].suname,  "%s",CH_SOURCE);
	sprintf( sysall.suinfo[1].content,  "%s","s_all" );
	
	sprintf( sysall.suinfo[2].suname,  "%s",CH_FILTER);
	sprintf( sysall.suinfo[2].content,  "%s",F_CLILOG);

	sprintf( sysall.suinfo[3].suname, "%s", CH_DEST);
	sprintf( sysall.suinfo[3].content, "df_server_%d", timez);
	
	sprintf( sysall.suinfo[4].suname,  "%s",NODE_MARKZ);
	sprintf( sysall.suinfo[4].content, "%d", timez);
	
	sprintf( sysall.suinfo[5].suname,  "%s",NODE_SYSIP);
	sprintf( sysall.suinfo[5].content, "%s", ip);
	
	sprintf( sysall.suinfo[6].suname,  "%s",NODE_SYSPORT);
	sprintf( sysall.suinfo[6].content, "%s", port);	
	
	sprintf( sysall.suinfo[7].suname,  "%s",NODE_INDEXZ);
	sprintf( sysall.suinfo[7].content, "%s", indexz);	
	
	add_second_xmlnode( xml_fpath, NODE_LOG, &sysall);

	memset( &logall, 0, sizeof(ST_SYS_ALL) );
	sprintf( logall.suinfo[0].suname,  "%s",NODE_VALUE );
	sprintf( logall.suinfo[0].content, "df_server_%d", timez );
	
	sprintf( logall.suinfo[1].suname,  "%s",NODE_CONTENT );
	sprintf( logall.suinfo[1].content, "udp(\"%s\" port(%s));", ip, port );
	
	sprintf( logall.suinfo[2].suname,  "%s",NODE_MARKZ);
	sprintf( logall.suinfo[2].content, "%d", timez);
	
	sprintf( logall.suinfo[3].suname,  "%s",NODE_SYSIP);
	sprintf( logall.suinfo[3].content, "%s", ip);
	
	sprintf( logall.suinfo[4].suname,  "%s",NODE_SYSPORT);
	sprintf( logall.suinfo[4].content, "%s", port);

	sprintf( logall.suinfo[5].suname,  "%s",NODE_PROTOCOLZ);
	sprintf( logall.suinfo[5].content, "%s", utcp);

	sprintf( logall.suinfo[6].suname,  "%s",NODE_FLEVEL);
	sprintf( logall.suinfo[6].content, "%s", flevel);	

	sprintf( logall.suinfo[7].suname,  "%s",NODE_INDEXZ);
	sprintf( logall.suinfo[7].content, "%s", indexz);
	
	add_second_xmlnode( xml_fpath, NODE_DES, &logall);
	return 0;
}
int del_des_node( char *xml_fpath, char *ip,char *port,char *utcp,char *flevel)
{
	xmlDocPtr pdoc = NULL;
	xmlNodePtr pcurnode = NULL;
	char *psfilename;
	psfilename = xml_fpath;
	int flag1 = 0,flag2 = 0,flag3 = 0,flag4 = 0,flag5 = 0;
	pdoc = xmlReadFile(psfilename,"utf-8",256); 

	if(NULL == pdoc)
	{
		return 1;
	}
	xmlNodePtr tempNode = NULL;
	xmlNodePtr childPtr = NULL;
	xmlChar *value;
	pcurnode = xmlDocGetRootElement(pdoc);
	pcurnode = pcurnode->xmlChildrenNode;

	while (NULL != pcurnode)
	{			
		tempNode = NULL;
		//tempNode = pcurnode->next; 
		if(NULL == pcurnode->next)
		{
			tempNode = pcurnode;
		}
		else
		{
			tempNode = pcurnode->next;  
		}
		flag5 = 0;
		if (!xmlStrcmp(pcurnode->name, BAD_CAST NODE_DES))
		{
			childPtr = NULL;
			childPtr = pcurnode; 
			childPtr=childPtr->children;

		    flag1 = 0;
			flag2 = 0;
			flag3 = 0;
			flag4 = 0;
			while(childPtr !=NULL)
			{	 
				if(!xmlStrcmp(childPtr->name, BAD_CAST NODE_SYSIP))
				{
				    
					value = xmlNodeGetContent(childPtr);
					if(strcmp(ip,(char *)value)==0)
					{
						flag1 = 1;
					}
					else
					{
						break;
					}
					xmlFree(value);
				}
				if(!xmlStrcmp(childPtr->name, BAD_CAST "sysport"))
				{
				    
					value = xmlNodeGetContent(childPtr);  
					if(strcmp(port,(char *)value)==0)
					{
						flag2 = 1;
					}
					else
					{
						break;
					}
					xmlFree(value);
				}
				if(!xmlStrcmp(childPtr->name, BAD_CAST "proz"))
				{
				    
					value = xmlNodeGetContent(childPtr);
					if(strcmp(utcp,(char *)value)==0)
					{
						flag3 = 1;
					}
					else
					{
						break;
					}
					xmlFree(value);
				}
				if(!xmlStrcmp(childPtr->name, BAD_CAST "flevel"))
				{
				    
					value = xmlNodeGetContent(childPtr);
					if(strcmp(flevel,(char *)value)==0)
					{
						flag4 = 1;
					}
					else
					{
						break;
					}
					xmlFree(value);
				}
				if((1 == flag1) && (1 == flag2) && (1 == flag3) && (1 == flag4))
				{
					flag5 = 1;
					break;
					
				}
				childPtr = childPtr->next;
			}
		}   
		if(1 == flag5)
		{
			/*this last node*/
			if(NULL == pcurnode->next)
			{
				xmlUnlinkNode(pcurnode);
				xmlFreeNode(pcurnode);
				pcurnode= tempNode;
			}
			else
			{
				tempNode = pcurnode->next;  
				xmlUnlinkNode(pcurnode);
				xmlFreeNode(pcurnode);
				pcurnode= tempNode;			   
			}
			break;
		}
		else
		{
			pcurnode = pcurnode->next; 
		}
	}	  
	xmlSaveFile(xml_fpath,pdoc);  
	xmlFreeDoc(pdoc);
	return 0;
}
int del_log_node( char *xml_fpath, char *ip,char *port, char *filter ,char *utcp)
{
	xmlDocPtr pdoc = NULL;
	xmlNodePtr pcurnode = NULL;
	char *psfilename;
	psfilename = xml_fpath;
	int flag1 = 0,flag2 = 0,flag3 = 0,flag4 = 0,flag5 = 0;
	pdoc = xmlReadFile(psfilename,"utf-8",256); 

	if(NULL == pdoc)
	{
		return 1;
	}
	xmlNodePtr tempNode = NULL;
	xmlNodePtr childPtr = NULL;
	xmlChar *value;
	pcurnode = xmlDocGetRootElement(pdoc);
	pcurnode = pcurnode->xmlChildrenNode;

	while (NULL != pcurnode)
	{			
		tempNode = NULL;
		if(NULL == pcurnode->next)
		{
			tempNode = pcurnode;
		}
		else
		{
			tempNode = pcurnode->next;  
		}
		flag5 = 0;
		if (!xmlStrcmp(pcurnode->name, BAD_CAST NODE_LOG))
		{
			childPtr = NULL;
			childPtr = pcurnode; 
			childPtr=childPtr->children;

		    flag1 = 0;
			flag2 = 0;
			flag3 = 0;
			while(childPtr !=NULL)
			{	 
				if(!xmlStrcmp(childPtr->name, BAD_CAST "filter"))
				{
				    
					value = xmlNodeGetContent(childPtr);
					if(strcmp(filter,(char *)value)==0)
					{
						flag1 = 1;
					}
					else
					{
						break;
					}
					xmlFree(value);
				}
				if(!xmlStrcmp(childPtr->name, BAD_CAST "sysipz"))
				{
				    
					value = xmlNodeGetContent(childPtr);  
					if(strcmp(ip,(char *)value)==0)
					{
						flag2 = 1;
					}
					else
					{
						break;
					}
					xmlFree(value);
				}
				if(!xmlStrcmp(childPtr->name, BAD_CAST "sysport"))
				{
				    
					value = xmlNodeGetContent(childPtr);
					if(strcmp(port,(char *)value)==0)
					{
						flag3 = 1;
					}
					else
					{
						break;
					}
					xmlFree(value);
				}
				if((1 == flag1) && (1 == flag2) && (1 == flag3))
				{
					flag5 = 1;
					break;
					
				}
				childPtr = childPtr->next;
			}
		}   
		if(1 == flag5)
		{
			/*this last node*/
			if(NULL == pcurnode->next)
			{
				xmlUnlinkNode(pcurnode);
				xmlFreeNode(pcurnode);
				pcurnode= tempNode;
			}
			else
			{
				tempNode = pcurnode->next;  
				xmlUnlinkNode(pcurnode);
				xmlFreeNode(pcurnode);
				pcurnode= tempNode;			   
			}
			break;
		}
		else
		{
			pcurnode = pcurnode->next; 
		}
	}	  
	xmlSaveFile(xml_fpath,pdoc);  
	xmlFreeDoc(pdoc);
	return 0;
}
int del_syslog_serve_web( char *xml_fpath, char *ip,char *port, char *filter ,char *utcp,char *flevel)
{
	del_des_node(xml_fpath, ip, port, utcp, flevel);
	del_log_node(xml_fpath, ip, port, filter, utcp);
	del_clilog_node(xml_fpath, ip, port, F_CLILOG, utcp,flevel);
	return 0;
}

int mod_syslog_server( char *xml_fpath, char *timeflag, int enable, char *ipaddr, int port, int filter )
{
     int flagz=0,idindex=0;
	 char *tempz=(char *)malloc(128);
	 char *strport=(char *)malloc(20);
     //log idindex
	 find_second_xmlnode(xml_fpath, NODE_LOG, NODE_MARKZ, timeflag, &idindex);
	 //des flagz
	 find_second_xmlnode(xml_fpath, NODE_DES, NODE_MARKZ, timeflag,&flagz);

	 if(flagz != 0)
	 {
		memset(tempz,0,128);
		sprintf(tempz,"udp(\"%s\" port(%d))",ipaddr,port);
		mod_second_xmlnode(xml_fpath, NODE_DES, NODE_CONTENT, tempz, flagz);		
		mod_second_xmlnode(xml_fpath, NODE_DES, NODE_SYSIP, ipaddr, flagz);
		memset(strport,0,20);
	 	sprintf(strport,"%d",port);
		mod_second_xmlnode(xml_fpath, NODE_DES, NODE_SYSPORT, strport, flagz);
		
	    if(idindex != 0)
	    {
			memset(tempz,0,128);
			sprintf(tempz,"%d",enable);
			mod_second_xmlnode(xml_fpath, NODE_LOG, NODE_ENABLES, tempz, idindex);
			mod_second_xmlnode(xml_fpath, NODE_LOG, NODE_SYSIP, ipaddr, idindex);
			mod_second_xmlnode(xml_fpath, NODE_LOG, NODE_SYSPORT, strport, idindex);
			mod_second_xmlnode(xml_fpath, NODE_LOG, CH_FILTER, filterlist[filter], idindex);
	    }
				
	 }
	 free(tempz);
	 free(strport);
	 return 0;
}

int del_syslog_server( char *xml_fpath, int idindex )
{
    int flagz=0;
	char gets[50];
	memset(gets,0,50);	
	get_second_xmlnode(xml_fpath, NODE_LOG, NODE_MARKZ,&gets,idindex);
	del_second_xmlnode(xml_fpath, NODE_LOG, idindex);
	if(strcmp(gets,"")!=0)
	{
		find_second_xmlnode(xml_fpath, NODE_DES, NODE_MARKZ, gets,&flagz);
		del_second_xmlnode(xml_fpath, NODE_DES, flagz);
	}
	return 0;
}
void save_syslog_file()
{
	if_filter_flood();
	if_filter_eaglog();
	add_log_portaldebug();
	if_filter_clilog();
	if_destination_clilog();
	if_log_clilog();
	SYSLOGALL_ST sysall; 
	memset(&sysall,0,sizeof(sysall));
    read_syslogall_st(XML_FPATH, &sysall);
	write_config_syslogallst(&sysall, CONF_FPATH);
	Free_read_syslogall_st(&sysall);
}
void if_filter_flood()
{
	int flag = 0;
	ST_SYS_ALL sysall,logall;
	memset( &sysall, 0, sizeof(ST_SYS_ALL) );
	int fid = 0;
	char destmp[128] = {0};
	
	find_second_xmlnode(XML_FPATH, NODE_FILTER, NODE_VALUE, F_FLOOD, &flag);
	if (0 == flag)
	{
		sprintf( sysall.suinfo[0].suname, "%s",NODE_VALUE);
		sprintf( sysall.suinfo[0].content, "%s",F_FLOOD);
		
		sprintf( sysall.suinfo[1].suname,  "%s",NODE_VIEWS);
		sprintf( sysall.suinfo[1].content,  "%d",0 );
		
		sprintf( sysall.suinfo[2].suname,  "%s",NODE_CONTENT);
		sprintf( sysall.suinfo[2].content,  "%s","match(\"syn_flood\") or match(\"sshd\") or match(\"telnetd\");");

		sprintf( sysall.suinfo[3].suname,  "%s",NODE_INFOS);
		sprintf( sysall.suinfo[3].content, "%s", "flood");

		add_second_xmlnode( XML_FPATH, NODE_FILTER, &sysall);
		
	}
}
void if_filter_clilog()
{
	int flag = 0;
	ST_SYS_ALL sysall,logall;
	memset( &sysall, 0, sizeof(ST_SYS_ALL) );
	int fid = 0;
	char destmp[128] = {0};
	
	find_second_xmlnode(XML_FPATH, NODE_FILTER, NODE_VALUE, F_CLILOG, &flag);
	if (0 == flag)
	{
		sprintf( sysall.suinfo[0].suname, "%s",NODE_VALUE);
		sprintf( sysall.suinfo[0].content, "%s",F_CLILOG);
		
		sprintf( sysall.suinfo[1].suname,  "%s",NODE_VIEWS);
		sprintf( sysall.suinfo[1].content,  "%d",0 );
		
		sprintf( sysall.suinfo[2].suname,  "%s",NODE_CONTENT);
		sprintf( sysall.suinfo[2].content,  "%s","facility(local5);");

		sprintf( sysall.suinfo[3].suname,  "%s",NODE_INFOS);
		sprintf( sysall.suinfo[3].content, "%s", "clilog");

		add_second_xmlnode( XML_FPATH, NODE_FILTER, &sysall);
		
	}
}
void if_log_clilog()
{
	int flag = 0;
	ST_SYS_ALL sysall,logall;	
	memset( &sysall, 0, sizeof(ST_SYS_ALL) );
	
	find_second_xmlnode(XML_FPATH, NODE_LOG, CH_FILTER, F_CLILOG, &flag);
	if (0 == flag)
	{
		sprintf( sysall.suinfo[0].suname,  "%s",CH_SOURCE);
		sprintf( sysall.suinfo[0].content,  "%s","s_all" );
		
		sprintf( sysall.suinfo[1].suname,  "%s",CH_FILTER);
		sprintf( sysall.suinfo[1].content,  "%s",F_CLILOG );
		
		sprintf( sysall.suinfo[2].suname, "%s", CH_DEST);
		sprintf( sysall.suinfo[2].content, "%s", DF_CLILOG);
		add_second_xmlnode( XML_FPATH, NODE_LOG, &sysall);
	}
}
void if_destination_clilog()
{
	int flag = 0;
	ST_SYS_ALL sysall,logall;
	memset( &sysall, 0, sizeof(ST_SYS_ALL) );
	int fid = 0;
	char destmp[128] = {0};
	
	find_second_xmlnode(XML_FPATH, NODE_DES, NODE_VALUE, DF_CLILOG, &flag);
	if (0 == flag)
	{
		sprintf( sysall.suinfo[0].suname, "%s",NODE_VALUE);
		sprintf( sysall.suinfo[0].content, "%s",DF_CLILOG);
		
		sprintf( sysall.suinfo[1].suname,  "%s",NODE_CONTENT);
		sprintf( sysall.suinfo[1].content,  "%s","file( \"/var/log/cli.log\" template(\"$DATE $PRIORITY $MSG\\n\") );");

		add_second_xmlnode( XML_FPATH, NODE_DES, &sysall);
		
	}
}

void if_filter_eaglog()
{
	int flag = 0;
	ST_SYS_ALL sysall,logall;
	memset( &sysall, 0, sizeof(ST_SYS_ALL) );
	int fid = 0;
	char destmp[128] = {0};
	
	find_second_xmlnode(XML_FPATH, NODE_FILTER, NODE_VALUE, F_EAGLOG, &flag);
	if (0 == flag)
	{
		sprintf( sysall.suinfo[0].suname, "%s",NODE_VALUE);
		sprintf( sysall.suinfo[0].content, "%s",F_EAGLOG);
		
		sprintf( sysall.suinfo[1].suname,  "%s",NODE_VIEWS);
		sprintf( sysall.suinfo[1].content,  "%d",0 );
		
		sprintf( sysall.suinfo[2].suname,  "%s",NODE_CONTENT);
		sprintf( sysall.suinfo[2].content,  "%s","match(\"eag\") or match(\"trap\");");

		sprintf( sysall.suinfo[3].suname,  "%s",NODE_INFOS);
		sprintf( sysall.suinfo[3].content, "%s", "eaglog");

		add_second_xmlnode( XML_FPATH, NODE_FILTER, &sysall);
		
	}
}

void if_filter_portaldebug()
{
	int flag = 0;
	ST_SYS_ALL sysall,logall;
	memset( &sysall, 0, sizeof(ST_SYS_ALL) );
	int fid = 0;
	char destmp[128] = {0};
	
	find_second_xmlnode(XML_FPATH, NODE_FILTER, NODE_VALUE, F_PDEBUG, &flag);
	if (0 == flag)
	{
		sprintf( sysall.suinfo[0].suname, "%s",NODE_VALUE);
		sprintf( sysall.suinfo[0].content, "%s",F_PDEBUG);
		
		sprintf( sysall.suinfo[1].suname,  "%s",NODE_VIEWS);
		sprintf( sysall.suinfo[1].content,  "%d",0 );
		
		sprintf( sysall.suinfo[2].suname,  "%s",NODE_CONTENT);
		sprintf( sysall.suinfo[2].content,  "%s","facility(local6);");

		sprintf( sysall.suinfo[3].suname,  "%s",NODE_INFOS);
		sprintf( sysall.suinfo[3].content, "%s", "local6");

		add_second_xmlnode( XML_FPATH, NODE_FILTER, &sysall);
		
	}
}
void if_destination_portaldebug()
{
	int flag = 0;
	ST_SYS_ALL sysall,logall;
	memset( &sysall, 0, sizeof(ST_SYS_ALL) );
	int fid = 0;
	char destmp[128] = {0};
	
	find_second_xmlnode(XML_FPATH, NODE_DES, NODE_VALUE, DF_PDEBUG, &flag);
	if (0 == flag)
	{
		sprintf( sysall.suinfo[0].suname, "%s",NODE_VALUE);
		sprintf( sysall.suinfo[0].content, "%s",DF_PDEBUG);
		
		sprintf( sysall.suinfo[1].suname,  "%s",NODE_CONTENT);
		sprintf( sysall.suinfo[1].content,  "%s","usertty(\"*\");");

		add_second_xmlnode( XML_FPATH, NODE_DES, &sysall);
		
	}
}
void if_log_portaldebug()
{
	int flag = 0;
	ST_SYS_ALL sysall,logall;
	memset( &sysall, 0, sizeof(ST_SYS_ALL) );
	int fid = 0;
	char destmp[128] = {0};
	char gets[128] = {0};

	find_second_xmlnode(XML_FPATH, NODE_LOG, CH_FILTER, F_PDEBUG, &flag);
	if (0 == flag)
	{
		sprintf( sysall.suinfo[0].suname, "%s",CH_SOURCE);
		sprintf( sysall.suinfo[0].content, "%s","s_all");
		
		sprintf( sysall.suinfo[1].suname,  "%s",CH_FILTER);
		sprintf( sysall.suinfo[1].content,  "%s",F_PDEBUG);

		sprintf( sysall.suinfo[2].suname,  "%s",CH_DEST);
		sprintf( sysall.suinfo[2].content,  "%s","df_system");

		add_second_xmlnode( XML_FPATH, NODE_LOG, &sysall);
		
	}
	else
	{
		get_second_xmlnode(XML_FPATH, NODE_LOG, CH_DEST, &gets, flag);
		if(0 != strcmp(gets,"df_system"))
		{
			mod_second_xmlnode(XML_FPATH, NODE_LOG, CH_DEST, "df_system", flag);
		}
	}
}
void add_log_portaldebug()
{
	if_filter_portaldebug();
	if_log_portaldebug();
}
void if_syslog_exist()
{
	char cmd[128] = {0};
	xmlDocPtr doc;	 
	if(access(XML_FPATH,0)!=0)
	{		
		memset(cmd,0,sizeof(cmd));
		sprintf(cmd,"sudo cp %s  %s",XML_SYS_D,XML_FPATH);
		system(cmd);
		memset(cmd,0,sizeof(cmd));
		sprintf(cmd,"sudo chmod 666 %s",XML_FPATH);
		system(cmd);
	}
	else
	{
		doc = xmlReadFile(XML_FPATH, "utf-8", 256); 
		if (doc == NULL ) 
		{
			memset(cmd,0,sizeof(cmd));
			sprintf(cmd,"sudo cp %s  %s",XML_SYS_D,XML_FPATH);
			system(cmd);
			memset(cmd,0,sizeof(cmd));
			sprintf(cmd,"sudo rm %s",XML_FPATH);
			system(cmd);
		}
	}
	save_syslog_file();
}
void if_ntp_exist()
{
	FILE *fp = NULL;
	if(access(NTP_XML_FPATH,0)!=0)
	{
		new_xml_file(NTP_XML_FPATH);
	}
	if(access(NTP_CONF_BK,0)!=0)
	{
		if((fp=fopen(NTP_CONF_BK,"w"))==NULL); 
		{ 
			fclose(fp);
			return ;
		}
		fclose(fp);
	}
	system("sudo chmod 666 "NTP_XML_FPATH"\n");
}

int if_syslog_enable()
{
	char *cmd=(char *)malloc(128);
	memset(cmd,0,128);
	char *tmpz=(char *)malloc(20);
	memset(tmpz,0,20);
	int sflag=-1;
	FILE *pp;
	pp=popen("ps -ef|grep syslog |grep -v grep|wc -l","r");
	if(pp==NULL)
	{
		sflag=0;//disable
	}
	else
	{
		fgets( tmpz, sizeof(20), pp );	
		sflag=strtoul(tmpz,0,10);						   
		pclose(pp);
	}
	free(tmpz);
	free(cmd);
	return sflag;
}

int if_ntp_enable()
{
	char *cmd=(char *)malloc(128);
	memset(cmd,0,128);

    
	char *tmpz=(char *)malloc(20);
	memset(tmpz,0,20);

    
    
	int sflag=-1;
    
	FILE *pp;
	pp=popen("ps -ef|grep ntpd |grep -v grep|wc -l","r");

    
	if(pp==NULL)
	{
		sflag=0;//disable
	}

    
	else
	{
		fgets( tmpz, sizeof(20), pp );	
		sflag=strtoul(tmpz,0,10);						   
		pclose(pp);
	}

    
	free(tmpz);
	free(cmd);

    
	return sflag;
}

///////////////////////////////////////////////////////////

int if_dup_info(char *sysudp,char *sysip,char *port,char *flevel,char *timeflag)/*返回0表示不重复，返回1表示重复*/
{
	struct dest_st dst,*dq;
	memset(&dst,0,sizeof(dst));
	int dnum=0,dip=0,dudp=0,dport=0,ftime=0,flagz=0;
	char *gets=(char *)malloc(50);
	memset(gets,0,50);
	
	read_dest_xml(&dst,&dnum);
	dq=dst.next;
	while(dq!=NULL)
	{
		if(strcmp(dq->sysipz,sysip)==0)
		{
			dip=1;
		}
		if(strcmp(dq->sysport,port)==0)
		{
			dport=1;
		}
		if(strcmp(dq->proz,sysudp)==0)
		{
			dudp=1;
		}
		if((dip==1)&&(dport==1)&&(dudp==1))
		{
		    find_second_xmlnode(XML_FPATH, NODE_LOG, NODE_MARKZ, dq->timeflag, &flagz);
			get_second_xmlnode(XML_FPATH, NODE_LOG, CH_FILTER, gets, flagz);
			if(strcmp(gets,flevel)==0)
			{
				ftime=1;
				strcpy(timeflag,dq->timeflag);
				break;
			}
		}
		dq=dq->next;
	}
	if(dnum>0)
		Free_read_dest_xml(&dst);
	free(gets);	
	return ftime;
}
int mod_first_xmlnode(char *xmlpath,char *node_name,char *newc)
{
	xmlDocPtr pdoc = NULL;
	xmlNodePtr pcurnode = NULL;
	xmlNodePtr design_node = NULL;
	pdoc = xmlReadFile(xmlpath,"utf-8",256);  

	if(NULL == pdoc)
	{
		return 1;
	}

	int if_second_flag=0;
	pcurnode = xmlDocGetRootElement(pdoc); 
	design_node=pcurnode;
	pcurnode = pcurnode->xmlChildrenNode; 
	if(NULL == pcurnode)
	{
		if_second_flag = 1;
	}
	while (NULL != pcurnode)
	{		
		if (!xmlStrcmp(pcurnode->name, BAD_CAST node_name))   //找到 status 这个 root 下一级子节点             
		{
			xmlNodeSetContent(pcurnode, BAD_CAST  newc); 
			if_second_flag=0;
			break;
		}  
		else
		{
		    if_second_flag=1;
		}
		pcurnode = pcurnode->next; 
	}	  
	/*如果为空或者是没有此二级节点，都要进行创建,现在是创建不成功
	pcurnode 子节点的集合*/
	if(if_second_flag==1)
	{
		xmlNodePtr node = xmlNewNode(NULL,BAD_CAST node_name);    
		xmlAddChild(design_node,node);
		xmlNodePtr content1 = xmlNewText(BAD_CAST newc);
		xmlAddChild(node,content1);
		design_node=NULL;
	}
	xmlSaveFile(xmlpath,pdoc);  
	xmlFreeDoc(pdoc);
	return if_second_flag;
}


int get_first_xmlnode(char *xmlpath,char *node_name,char *newc)
{
	xmlDocPtr pdoc = NULL;
	xmlNodePtr pcurnode = NULL;
	pdoc = xmlReadFile(xmlpath,"utf-8",256);  
	if(NULL == pdoc)
	{
		return 1;
	}
	pcurnode = xmlDocGetRootElement(pdoc); 
	pcurnode = pcurnode->xmlChildrenNode; 
	while (NULL != pcurnode)
	{		
		xmlChar *value;
		if (!xmlStrcmp(pcurnode->name, BAD_CAST node_name))   //找到 status 这个 root 下一级子节点             
		{
			value = xmlNodeGetContent(pcurnode);         	
			strcpy(newc,(char *)value);
			xmlFree(value);		
		}  
		pcurnode = pcurnode->next; 
	}	  
	xmlSaveFile(xmlpath,pdoc);  
	xmlFreeDoc(pdoc);
	return 0;
}


int add_ntp_server( char *xml_fpath, char *ipz, char *maskz)
{
	ST_SYS_ALL sysall;	
    time_t now;
	int retu = 0;
	unsigned int timez = (unsigned int)time(&now);
	memset( &sysall, 0, sizeof(ST_SYS_ALL) );
	
	sprintf( sysall.suinfo[0].suname, "%s",NTP_SIPZ);
	sprintf( sysall.suinfo[0].content, "%s",ipz );
	
	sprintf( sysall.suinfo[1].suname,  "%s",NTP_SMASKZ);
	sprintf( sysall.suinfo[1].content,  "%s",maskz );	
	
	sprintf( sysall.suinfo[2].suname, "%s", NTP_TIMEF);
	sprintf( sysall.suinfo[2].content, "%d", timez);
	
	retu = add_second_xmlnode( xml_fpath,NTP_SERVZ, &sysall);
	return retu ;
}



int add_ntp_server_default( char *xml_fpath, char *ipz, char *maskz)
{
	ST_SYS_ALL sysall;	
    time_t now;
	unsigned int timez = (unsigned int)time(&now);
	memset( &sysall, 0, sizeof(ST_SYS_ALL) );
	
	sprintf( sysall.suinfo[0].suname, "%s",NTP_SIPZ);
	sprintf( sysall.suinfo[0].content, "%s",ipz );
	
	sprintf( sysall.suinfo[1].suname,  "%s",NTP_SMASKZ);
	sprintf( sysall.suinfo[1].content,  "%s",maskz );	
	
	sprintf( sysall.suinfo[2].suname, "%s", NTP_TIMEF);
	sprintf( sysall.suinfo[2].content, "%s", "def");
	
	add_second_xmlnode( xml_fpath,NTP_SERVZ, &sysall);
	return 0;
}

int add_ntp_server_slotid( char *xml_fpath, char *ipz, char *maskz,int slotid)
{
	ST_SYS_ALL sysall;	
    time_t now;
	unsigned int timez = (unsigned int)time(&now);
	memset( &sysall, 0, sizeof(ST_SYS_ALL) );
	
	sprintf( sysall.suinfo[0].suname, "%s",NTP_SIPZ);
	sprintf( sysall.suinfo[0].content, "%s",ipz );
	
	sprintf( sysall.suinfo[1].suname,  "%s",NTP_SMASKZ);
	sprintf( sysall.suinfo[1].content,  "%s",maskz );	
	
	sprintf( sysall.suinfo[2].suname, "%s", NTP_TIMEF);
	sprintf( sysall.suinfo[2].content, "%d", timez);

	sprintf( sysall.suinfo[3].suname, "%s", NTP_SLOTID);
	sprintf( sysall.suinfo[3].content, "%d", slotid);	
	
	add_second_xmlnode( xml_fpath,NTP_SERVZ, &sysall);
	return 0;
}

int add_ntp_client( char *xml_fpath, char *ipz, char *perz)
{
	ST_SYS_ALL sysall;	
    time_t now;
	int ret = 0;
	unsigned int timez = (unsigned int)time(&now);
	memset( &sysall, 0, sizeof(ST_SYS_ALL) );
	
	sprintf( sysall.suinfo[0].suname, "%s",NTP_CIPZ);
	sprintf( sysall.suinfo[0].content, "%s",ipz );
	
	sprintf( sysall.suinfo[1].suname,  "%s",NTP_CPERZ);
	sprintf( sysall.suinfo[1].content,  "%s",perz );	
	
	sprintf( sysall.suinfo[2].suname, "%s", NTP_TIMEF);
	sprintf( sysall.suinfo[2].content, "%d", timez);
	
	ret = add_second_xmlnode( xml_fpath,NTP_CLIZ, &sysall);
	return ret;
}


int add_ntp_client_default( char *xml_fpath, char *ipz, char *perz)
{
	ST_SYS_ALL sysall;	
    time_t now;
	unsigned int timez = (unsigned int)time(&now);
	memset( &sysall, 0, sizeof(ST_SYS_ALL) );
	
	sprintf( sysall.suinfo[0].suname, "%s",NTP_CIPZ);
	sprintf( sysall.suinfo[0].content, "%s",ipz );
	
	sprintf( sysall.suinfo[1].suname,  "%s",NTP_CPERZ);
	sprintf( sysall.suinfo[1].content,  "%s",perz );	
	
	sprintf( sysall.suinfo[2].suname, "%s", NTP_TIMEF);
	sprintf( sysall.suinfo[2].content, "%s", "def");
	
	add_second_xmlnode( xml_fpath,NTP_CLIZ, &sysall);
	return 0;
}

int add_ntp_client_slotid( char *xml_fpath, char *ipz, char *perz,int slotid)
{
	ST_SYS_ALL sysall;	
    time_t now;
	unsigned int timez = (unsigned int)time(&now);
	memset( &sysall, 0, sizeof(ST_SYS_ALL) );
	
	sprintf( sysall.suinfo[0].suname, "%s",NTP_CIPZ);
	sprintf( sysall.suinfo[0].content, "%s",ipz );
	
	sprintf( sysall.suinfo[1].suname,  "%s",NTP_CPERZ);
	sprintf( sysall.suinfo[1].content,  "%s",perz );	
	
	sprintf( sysall.suinfo[2].suname, "%s", NTP_TIMEF);
	sprintf( sysall.suinfo[2].content, "%d", timez);

	sprintf( sysall.suinfo[3].suname, "%s", NTP_SLOTID);
	sprintf( sysall.suinfo[3].content, "%d", slotid);
	
	add_second_xmlnode( xml_fpath,NTP_CLIZ, &sysall);
	return 0;
}

//free link list
void Free_read_ntp_server(struct serverz_st *head)
{
    struct serverz_st *f1,*f2;
	f1=head->next;
	if(NULL != f1)
	{
		f2=f1->next;
		while(f2!=NULL)
		{
		  free(f1);
		  f1=f2;
		  f2=f2->next;
		}
		free(f1);
	}
}

int read_ntp_server(char *xmlpath,struct serverz_st *chead,int *confnum)
{
	xmlDocPtr doc;	 
	xmlNodePtr cur,tmp;
    int conflag=0;	
	doc = xmlReadFile(xmlpath, "utf-8", 256); 
	if (doc == NULL ) 
	{
		return -1;
	}
	cur = xmlDocGetRootElement(doc); 
	if (cur == NULL)
	{
		xmlFreeDoc(doc);
		return -1;
	}

	if (xmlStrcmp(cur->name, (const xmlChar *) "root")) 
	{
		xmlFreeDoc(doc);
		return -1;
	}

	struct serverz_st *ctail=NULL;
	chead->next=NULL;
	ctail=chead;

	cur = cur->xmlChildrenNode;	
	while(cur !=NULL)
	{

	    tmp = cur->xmlChildrenNode;
		if (!xmlStrcmp(cur->name, BAD_CAST NTP_SERVZ))           
		{
		        /////////////conf informations
		        struct serverz_st  *cq=NULL;
				cq=(struct serverz_st *)malloc(sizeof(struct serverz_st)+1);
				memset(cq,0,sizeof(struct serverz_st)+1);
				if(NULL == cq)
				{
					return  -1;
				}
				/////////////  
                conflag++;
				xmlChar *value=NULL;
				
				while(tmp !=NULL)
				{	 
					//server ip
					if ((!xmlStrcmp(tmp->name, BAD_CAST NTP_SIPZ)))
					{
					    memset(cq->servipz,0,IPMASK_LEN);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->servipz,(char *)value);	
						xmlFree(value);
					}		
					//server mask
					else if ((!xmlStrcmp(tmp->name, BAD_CAST NTP_SMASKZ)))
					{
					    memset(cq->maskz,0,IPMASK_LEN);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->maskz,(char *)value);	
						xmlFree(value);
					}	
					//server timeflag
					else if ((!xmlStrcmp(tmp->name, BAD_CAST NTP_TIMEF)))
					{
					    memset(cq->timeflag,0,TIMESTR_LEN);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->timeflag,(char *)value);	
						xmlFree(value);
					}	
					//server slotid
					else if ((!xmlStrcmp(tmp->name, BAD_CAST NTP_SLOTID)))
					{
					    memset(cq->slotid,0,SID_LEN);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->slotid,(char *)value);	
						xmlFree(value);
					}	
					tmp = tmp->next;
				}            
				 cq->next = NULL;
			     ctail->next = cq;
			     ctail = cq;
		}      
	   cur = cur->next;
	}
    *confnum=conflag;
	xmlFreeDoc(doc);
	return 0;
}

//free link list
void Free_read_ntp_client(struct clientz_st *head)
{
    struct clientz_st *f1,*f2;
	f1=head->next;	
	if (NULL != f1)
	{
		f2=f1->next;
		while(f2!=NULL)
		{
		  free(f1);
		  f1=f2;
		  f2=f2->next;
		}
		free(f1);
	}
}

int read_ntp_client(char *xmlpath,struct clientz_st *chead,int *confnum)
{
	xmlDocPtr doc;	 
	xmlNodePtr cur,tmp;
    int conflag=0;	
	doc = xmlReadFile(xmlpath, "utf-8", 256); 
	if (doc == NULL ) 
	{
		return -1;
	}
	cur = xmlDocGetRootElement(doc); 
	if (cur == NULL)
	{
		xmlFreeDoc(doc);
		return -1;
	}

	if (xmlStrcmp(cur->name, (const xmlChar *) "root")) 
	{
		xmlFreeDoc(doc);
		return -1;
	}

	struct clientz_st *ctail=NULL;
	chead->next=NULL;
	ctail=chead;

	cur = cur->xmlChildrenNode;	
	while(cur !=NULL)
	{

	    tmp = cur->xmlChildrenNode;
		if (!xmlStrcmp(cur->name, BAD_CAST NTP_CLIZ))           
		{
		        /////////////conf informations
		        struct clientz_st  *cq=NULL;
				cq=(struct clientz_st *)malloc(sizeof(struct clientz_st)+1);
				memset(cq,0,sizeof(struct clientz_st)+1);
				if(NULL == cq)
				{
					return  -1;
				}
                conflag++;
				xmlChar *value=NULL;
				
				while(tmp !=NULL)
				{	 
					//client ip
					if ((!xmlStrcmp(tmp->name, BAD_CAST NTP_CIPZ)))
					{
					    memset(cq->clitipz,0,IPMASK_LEN);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->clitipz,(char *)value);	
						xmlFree(value);
					}		
					//client ifper
					else if ((!xmlStrcmp(tmp->name, BAD_CAST NTP_CPERZ)))
					{
					    memset(cq->ifper,0,NTPOSTR_LEN);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->ifper,(char *)value);	
						xmlFree(value);
					}	
					//client timeflag
					else if ((!xmlStrcmp(tmp->name, BAD_CAST NTP_TIMEF)))
					{
					    memset(cq->timeflag,0,TIMESTR_LEN);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->timeflag,(char *)value);	
						xmlFree(value);
					}	
					//client slotid
					else if ((!xmlStrcmp(tmp->name, BAD_CAST NTP_SLOTID)))
					{
					    memset(cq->slotid,0,SID_LEN);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->slotid,(char *)value);	
						xmlFree(value);
					}	
					tmp = tmp->next;
				}            
				 cq->next = NULL;
			     ctail->next = cq;
			     ctail = cq;
		}      
	   cur = cur->next;
	}
    *confnum=conflag;
	xmlFreeDoc(doc);
	return 0;
}
int count_xml_node(char *xmlpath,int *nodenum)
{
	xmlDocPtr pdoc = NULL;
	xmlNodePtr pcurnode = NULL;
	*nodenum = 0;
	int count=0;
	pdoc = xmlReadFile(xmlpath,"utf-8",256); 

	if(NULL == pdoc)
	{
		return 1;
	}

	pcurnode = xmlDocGetRootElement(pdoc);   
	pcurnode = pcurnode->xmlChildrenNode; 

	while (NULL != pcurnode)
	{	
	    count++;
		pcurnode = pcurnode->next; 
	}	  
	*nodenum=count;
	xmlSaveFile(xmlpath,pdoc);  
	xmlFreeDoc(pdoc);
	return 0;
}
int count_secondxml_node(char *xmlpath,char *node_name,int *nodenum)
{
	xmlDocPtr pdoc = NULL;
	xmlNodePtr pcurnode = NULL;
	*nodenum = 0;
	int count=0;
	pdoc = xmlReadFile(xmlpath,"utf-8",256); 

	if(NULL == pdoc)
	{
		return 1;
	}

	pcurnode = xmlDocGetRootElement(pdoc);   
	pcurnode = pcurnode->xmlChildrenNode; 

	while (NULL != pcurnode)
	{	
		if (!xmlStrcmp(pcurnode->name, BAD_CAST node_name))
		{
			count++;
		}  
		pcurnode = pcurnode->next; 
	}	  
	*nodenum=count;
	xmlSaveFile(xmlpath,pdoc);  
	xmlFreeDoc(pdoc);
	return 0;
}
/*写入到ntp的conf中函数*/
int write_ntp_conf( NTPALL_ST *ntpall, char *file_path) 
{

	FILE * fp;	
	char des_name[50];
	char *File_content=(char *)malloc(10000);
	memset(File_content,0,10000);
	memset(des_name,0,50);
	int i=0;

    struct clientz_st *cq,*upcq;
	struct serverz_st *sq;
    
	if( NULL == file_path)
	{
		free(File_content);
		return -1 ;  
	}
	if(( fp = fopen(file_path,"w+"))==NULL)
	{
		free(File_content);
		return -2;
	}

    sq=ntpall->serst.next;
	while(sq!=NULL)
	{

		memset(des_name,0,50);
		sprintf(des_name , "restrict %s mask %s %s\n",sq->servipz,sq->maskz,NTP_NM);

		strcat(File_content,des_name);	

	  sq=sq->next;
	}
	upcq=ntpall->upclist.next;
	while(upcq!=NULL)
	{
		memset(des_name,0,50);
		if(0 == strncmp(upcq->ifper,"on",2))
		{
			sprintf(des_name , "server %s %s\n",upcq->clitipz,"prefer");
		}
		else
		{
			sprintf(des_name , "server %s\n",upcq->clitipz);
		}
		strcat(File_content,des_name);	

		memset(des_name,0,50);
		sprintf(des_name , "restrict %s\n",upcq->clitipz);
		strcat(File_content,des_name);	

		upcq=upcq->next;
	}

	cq=ntpall->clist.next;
	while(cq!=NULL)
	{
		memset(des_name,0,50);
		if(0 == strncmp(cq->ifper,"on",2))
		{
			sprintf(des_name , "server %s %s\n",cq->clitipz,"prefer");
		}
		else
		{
			sprintf(des_name , "server %s\n",cq->clitipz);
		}
		strcat(File_content,des_name);	

		memset(des_name,0,50);
		sprintf(des_name , "restrict %s\n",cq->clitipz);
		strcat(File_content,des_name);	

		cq=cq->next;
	}
	if (0 == strncmp(ntpall->sring,"enable",6))
	{
		memset(des_name,0,50);
		sprintf(des_name , "server %s\n",NTP_RING);
		strcat(File_content,des_name);	

		memset(des_name,0,50);
		sprintf(des_name , "restrict %s\n",NTP_RING);
		strcat(File_content,des_name);	

	}
	if (0 != strcmp(ntpall->ipv4,""))
	{
		memset(des_name,0,50);
		sprintf(des_name , "interface %s ipv4\n",ntpall->ipv4);
		strcat(File_content,des_name);	
	}
	if (0 != strcmp(ntpall->ipv6,""))
	{
		memset(des_name,0,50);
		sprintf(des_name , "interface %s ipv6\n",ntpall->ipv6);
		strcat(File_content,des_name);	
	}
	
	fwrite(File_content,strlen(File_content),1,fp);	
	fclose(fp);
	free(File_content);
	return 0;
}
void save_ntp_conf()
{
	NTPALL_ST ntpall; 
	memset(&ntpall,0,sizeof(ntpall));
    int cnum=0,snum=0,unum = 0;
	
	read_upper_ntp(NTP_XML_FPATH, &(ntpall.upclist),&unum);
	read_ntp_client(NTP_XML_FPATH, &(ntpall.clist),&cnum);	
	read_ntp_server(NTP_XML_FPATH, &(ntpall.serst), &snum);
	
	get_first_xmlnode(NTP_XML_FPATH, NODE_IPV4, &(ntpall.ipv4));
	get_first_xmlnode(NTP_XML_FPATH, NODE_IPV6, &(ntpall.ipv6));
	get_first_xmlnode(NTP_XML_FPATH, "cront", &(ntpall.cront));
	get_first_xmlnode(NTP_XML_FPATH, "ntpv", &(ntpall.ntpv));
	get_first_xmlnode(NTP_XML_FPATH, NODE_RING, &(ntpall.sring));
	
	write_ntp_conf(&ntpall, NTP_CONF_BK);

	if(cnum>0)
	{
		Free_read_ntp_client(&(ntpall.clist));
	}
	if(snum>0)
	{
		Free_read_ntp_server(&(ntpall.serst));
	}
	if(unum>0)
	{
		Free_read_upper_ntp(&(ntpall.upclist));
	}

}

int check_ip_format_func(char *str)
{
	char *endptr = NULL;
	int endptr1 = 0;
	char c;
	int IP,i;
	c = str[0];
	if (c>='0'&&c<='9')
	{
		IP= strtoul(str,&endptr,10);
		if(IP < 0||IP > 255)
		{
			return -1;
		}
		else if(((IP < 10)&&((endptr - str) > 1))||((IP < 100)&&((endptr - str) > 2))||((IP < 256)&&((endptr - str) > 3)))
		{
			return -1;
		}
		for(i = 0; i < 3; i++)
		{
			if(endptr[0] == '\0'||endptr[0] != '.')
			{
				return -1;
			}
			else
			{
				endptr1 = &endptr[1];
				IP= strtoul(&endptr[1],&endptr,10);				
				if(IP < 0||IP > 255)
				{
					return -1;	
				}
				else if(((IP < 10)&&((endptr - endptr1) > 1))||((IP < 100)&&((endptr - endptr1) > 2))||((IP < 256)&&((endptr - endptr1) > 3)))
				{
					return -1;
				}
			}
		}
		if(endptr[0] == '\0' && IP > 0)
		{
			return 0;
		}
		else
		{
			return -1;
		}
	}
	else
	{
		return -1;
	}

}
int set_default_inside_client_func(int p_masterid)
{

	int cflag = 0;

	if_ntp_exist();
	char topsip[32] = {0};	
	//server add top server ip
	snprintf(topsip,sizeof(topsip)-1,"169.254.1.%d",p_masterid);
	cflag = ntp_clientip_duplication(topsip);
	if(0 == cflag)
	{
		add_ntp_client_default(NTP_XML_FPATH, topsip, "on");
	}	
	//add ntpv4
	mod_first_xmlnode(NTP_XML_FPATH, "ntpv", "4");
	//add crontab 
	mod_first_xmlnode(NTP_XML_FPATH, "cront", "10^mins");
	//add ipv4 listen 
	mod_first_xmlnode(NTP_XML_FPATH, "nipv4", "listen");
	//add ipv6 ingore
	mod_first_xmlnode(NTP_XML_FPATH, "nipv6", "ignore");	
	save_ntp_conf();
	//restart_ntp();
	return 0;
}

// check whether server IPv4 address is duplication
int ntp_clientip_duplication(char *ntp_clientip)
{

	struct clientz_st clitst ,*cq;
	memset(&clitst,0,sizeof(clitst));
	int clinum=0;
	int flag  = 0;
	int retu = 0;
                     
    retu = read_ntp_client(NTP_XML_FPATH, &clitst, &clinum);
	if(0 == retu)
	{
		cq=clitst.next;

		while(cq!=NULL)
		{
		   if(!strcmp(cq->clitipz,ntp_clientip)){
				//return 1;
				flag = 1;
				break;
		   }
	       cq = cq->next;
		}
	}
	if(clinum>0)
    {   
		Free_read_ntp_client(&clitst);        
    }
    
	return flag;

}


// check whether client IPv4 address is duplication
int ntp_serverip_duplication(char *ntp_serverip)
{

    struct serverz_st servst ,*sq;                        
	memset(&servst,0,sizeof(servst));
	int servnum  =  0;
	int flag = 0;
	int retu = 0;

	retu = read_ntp_server(  NTP_XML_FPATH,   &servst,   &servnum  );
	if(0 == retu)
	{
	    sq = servst.next;
	    while(sq!=NULL)
	     {
	        if(!strcmp(sq->servipz,ntp_serverip)){
				flag = 1;
				break;
				//return 1;
		   }	                                                
	        sq = sq->next;	                                        
	     }  
	}         
      if(servnum>0)
      {                  
         Free_read_ntp_server(&servst);                                            
      }                                                                                                             

	return flag;
	
}
int add_upper_ntp( char *xml_fpath, char *ipz, char *perz,char* slotid)
{
	ST_SYS_ALL sysall;	
    time_t now;
	unsigned int timez = (unsigned int)time(&now);
	memset( &sysall, 0, sizeof(ST_SYS_ALL) );
	
	sprintf( sysall.suinfo[0].suname, "%s",NTP_CIPZ);
	sprintf( sysall.suinfo[0].content, "%s",ipz );
	
	sprintf( sysall.suinfo[1].suname,  "%s",NTP_CPERZ);
	sprintf( sysall.suinfo[1].content,  "%s",perz );	
	
	sprintf( sysall.suinfo[2].suname, "%s", NTP_TIMEF);
	sprintf( sysall.suinfo[2].content, "%d", timez);

	sprintf( sysall.suinfo[3].suname, "%s", NTP_SLOTID);
	sprintf( sysall.suinfo[3].content, "%s", slotid);
	
	add_second_xmlnode( xml_fpath,NTP_UPCLIZ, &sysall);
	return 0;
}

int read_upper_ntp(char *xmlpath,struct clientz_st *chead,int *confnum)
{
	xmlDocPtr doc;	 
	xmlNodePtr cur,tmp;
    int conflag=0;	
	doc = xmlReadFile(xmlpath, "utf-8", 256); 
	if (doc == NULL ) 
	{
		return -1;
	}
	cur = xmlDocGetRootElement(doc); 
	if (cur == NULL)
	{
		xmlFreeDoc(doc);
		return -1;
	}

	if (xmlStrcmp(cur->name, (const xmlChar *) "root")) 
	{
		xmlFreeDoc(doc);
		return -1;
	}

	struct clientz_st *ctail=NULL;
	chead->next=NULL;
	ctail=chead;

	cur = cur->xmlChildrenNode;	
	while(cur !=NULL)
	{

	    tmp = cur->xmlChildrenNode;
		if (!xmlStrcmp(cur->name, BAD_CAST NTP_UPCLIZ))           
		{
		        /////////////conf informations
		        struct clientz_st  *cq=NULL;
				cq=(struct clientz_st *)malloc(sizeof(struct clientz_st)+1);
				memset(cq,0,sizeof(struct clientz_st)+1);
				if(NULL == cq)
				{
					return  -1;
				}
                conflag++;
				xmlChar *value=NULL;
				
				while(tmp !=NULL)
				{	 
					//client ip
					if ((!xmlStrcmp(tmp->name, BAD_CAST NTP_CIPZ)))
					{
					    memset(cq->clitipz,0,IPMASK_LEN);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->clitipz,(char *)value);	
						xmlFree(value);
					}		
					//client ifper
					else if ((!xmlStrcmp(tmp->name, BAD_CAST NTP_CPERZ)))
					{
					    memset(cq->ifper,0,NTPOSTR_LEN);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->ifper,(char *)value);	
						xmlFree(value);
					}	
					//client timeflag
					else if ((!xmlStrcmp(tmp->name, BAD_CAST NTP_TIMEF)))
					{
					    memset(cq->timeflag,0,TIMESTR_LEN);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->timeflag,(char *)value);	
						xmlFree(value);
					}	
					//client slotid
					else if ((!xmlStrcmp(tmp->name, BAD_CAST NTP_SLOTID)))
					{
					    memset(cq->slotid,0,SID_LEN);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->slotid,(char *)value);	
						xmlFree(value);
					}	
					tmp = tmp->next;
				}            
				 cq->next = NULL;
			     ctail->next = cq;
			     ctail = cq;
		}      
	   cur = cur->next;
	}
    *confnum=conflag;
	xmlFreeDoc(doc);
	return 0;
}

//free link list
void Free_read_upper_ntp(struct clientz_st *head)
{
    struct clientz_st *f1,*f2;
	f1=head->next;	
	if (NULL != f1)
	{
		f2=f1->next;
		while(f2!=NULL)
		{
		  free(f1);
		  f1=f2;
		  f2=f2->next;
		}
		free(f1);
	}
}

// check whether server IPv4 address is duplication
int ntp_uppertip_duplication(char *ntp_clientip)
{

	struct clientz_st clitst ,*cq;
	memset(&clitst,0,sizeof(clitst));
	int clinum=0;
	int flag  = 0;
	int retu = 0;
                     
    retu = read_upper_ntp(NTP_XML_FPATH, &clitst, &clinum);
	if(0 == retu)
	{
		cq=clitst.next;

		while(cq!=NULL)
		{
		   if(!strcmp(cq->clitipz,ntp_clientip)){
				//return 1;
				flag = 1;
				break;
		   }
	       cq = cq->next;
		}
	}
	if(clinum>0)
    {   
		Free_read_upper_ntp(&clitst);        
    }
    
	return flag;

}
/***************************************add new function********************************************************************/

int set_master_default_server_func()
{
	int sflag = 0;
	if_ntp_exist();
	//open 169.254.1.1 subnet 24	
	sflag = ntp_serverip_duplication("169.254.1.0");
	if(0 == sflag)
	{
		add_ntp_server_default(NTP_XML_FPATH, "169.254.1.0", "255.255.255.0");
	}
	//add ntpv4
	mod_first_xmlnode(NTP_XML_FPATH, "ntpv", "4");
	//add crontab 
	mod_first_xmlnode(NTP_XML_FPATH, "cront", "10^mins");
	//add ipv4 listen 
	mod_first_xmlnode(NTP_XML_FPATH, "nipv4", "listen");
	//add ipv6 ingore
	mod_first_xmlnode(NTP_XML_FPATH, "nipv6", "ignore");
	//server 127.127.1.0
	mod_first_xmlnode(NTP_XML_FPATH, NODE_RING, "enable");
	save_ntp_conf();
	//restart_ntp();
	return 0;
}

//Check whether  IPv4 address is valid.It returns nonzero if the address is valid,zero if not.
int ip_address_check(char *str)
{

	char *address = (char *)malloc(16);
	char *tmp;
	tmp = address;
	memset(tmp,0,16);
	strncpy(tmp,str,15);
	int k = 0,j;
	int flag = 1;	
	j = strtoul(tmp,&tmp,0);
	if(j == 0){
		flag = 0;
	}
	while(*tmp){
		j = strtoul(tmp,&tmp,0);
		if(*tmp == '.'){
			k++;
		}
		tmp++;
	}
	if(k != 3 || inet_addr(str) == EOF){
		flag = 0;
	}

	free(address);
	address = NULL;
  	tmp = NULL;
	return flag;
}
//Check whether  IPv4 address is valid.It returns nonzero if the address is valid,zero if not.
//if(j != 255 && j != 0)
int submask_address_check(char *str)
{
	char *address = (char *)malloc(16);
	char *tmp;
	tmp = address;
	memset(tmp,0,16);
	strncpy(tmp,str,15);
	int k = 0,j;
	int flag = 1;
	j = strtoul(tmp,&tmp,0);
	if(j == 0){
		flag = 0;
	}
	while(*tmp){
		j = strtoul(tmp,&tmp,0);
		if(*tmp == '.'){
			k++;
		}
		if(j != 255 && j != 0){
			flag = 0;
		}
		tmp++;
	}
	if(k != 3 || inet_addr(str) == EOF){
		flag = 0;
	}
	free(address);
	address = NULL;
  	tmp = NULL;
	return flag;
}

// check whether client IPv4 address is duplication
int ntp_upperserverip_duplication(char *ntp_serverip)
{

    struct serverz_st servst ,*sq;                        
	memset(&servst,0,sizeof(servst));
	int servnum  =  0;
	int flag = 0;
	int retu = 0;

	retu = read_upper_ntp(  NTP_XML_FPATH,   &servst,   &servnum  );
	if(0 == retu)
	{
	    sq = servst.next;
	    while(sq!=NULL)
	     {
	        if(!strcmp(sq->servipz,ntp_serverip)){
				flag = 1;
				break;
				//return 1;
		   }	                                                
	        sq = sq->next;	                                        
	     }  
	}         
      if(servnum>0)
      {                  
         Free_read_upper_ntp(&servst);                                            
      }                                                                                                             

	return flag;
}
int add_filter(char *rulename,char *content)
{
	int flagz=0;
	char matchstr[128] = {0};
	#if 0 
	char tmp[512] = {0};	
	char *p=NULL;
	p=strtok(content,"/");
	while( NULL != p )
	{
		memset(matchstr,0,sizeof(matchstr));
		snprintf(matchstr,sizeof(matchstr)-1,"match(\"%s\")",p);
		strcat(tmp,matchstr);
		p = strtok( NULL, "/" );
		if(p)
		{
			strcat(tmp," and ");
		}
	}
	#endif
	memset(matchstr,0,sizeof(matchstr));
	snprintf(matchstr,sizeof(matchstr)-1,"match(\"%s\") and not match(\"www-data\") and not match(\"root\");",content);
	
	printf("#ccgi# ---------- tmp is: %s\n",matchstr);
	find_second_xmlnode(XML_FPATH, NODE_FILTER, NODE_VALUE, rulename, &flagz);

	if(flagz == 0)
	{
		ST_SYS_ALL sysall,logall;	
		memset( &sysall, 0, sizeof(ST_SYS_ALL) );
		
		sprintf( sysall.suinfo[0].suname, "%s",NODE_VALUE);
		sprintf( sysall.suinfo[0].content, "%s",rulename );

		sprintf( sysall.suinfo[1].suname,  "%s",NODE_VIEWS);
		sprintf( sysall.suinfo[1].content,  "%s","2" );

		sprintf( sysall.suinfo[2].suname,  "%s",NODE_CONTENT);
		sprintf( sysall.suinfo[2].content,  "%s",matchstr);

		sprintf( sysall.suinfo[3].suname, "%s", NODE_INFOS);
		sprintf( sysall.suinfo[3].content, "%s", content);

		add_second_xmlnode( XML_FPATH, NODE_FILTER, &sysall);

	}
	else
	{
		mod_second_xmlnode(XML_FPATH, NODE_FILTER, NODE_CONTENT, content, flagz);
	}
	return 0;
}
int del_syslogruler(char *xml_fpath,char *node_name,char *subnode,char *rulename)
{
	xmlDocPtr pdoc = NULL;
	xmlNodePtr pcurnode = NULL;
	char *psfilename;
	psfilename = xml_fpath;
	int flag1 = 0;
	pdoc = xmlReadFile(psfilename,"utf-8",256);

	if(NULL == pdoc)
	{
		return 1;
	}
	xmlNodePtr tempNode = NULL;
	xmlNodePtr childPtr = NULL;
	xmlChar *value;
	pcurnode = xmlDocGetRootElement(pdoc);
	pcurnode = pcurnode->xmlChildrenNode;

	while (NULL != pcurnode)
	{
		tempNode = NULL;
		if(NULL == pcurnode->next)
		{
			tempNode = pcurnode;
		}
		else
		{
			tempNode = pcurnode->next;  
		}
		flag1 = 0;
		if (!xmlStrcmp(pcurnode->name, BAD_CAST node_name))
		{
			childPtr = NULL;
			childPtr = pcurnode; 
			childPtr=childPtr->children;

		    flag1 = 0;
			while(childPtr !=NULL)
			{
				if(!xmlStrcmp(childPtr->name, BAD_CAST subnode))
				{
				    
					value = xmlNodeGetContent(childPtr);
					if(strchr(value,';'))
					{
						if(strncmp(rulename,(char *)value,strlen(rulename))==0)
						{
							flag1 = 1;
						}
						else
						{
							break;
						}
					}
					else
					{
						if(strcmp(rulename,(char *)value)==0)
						{
							flag1 = 1;
						}
						else
						{
							break;
						}
					}
					xmlFree(value);
				}
				childPtr = childPtr->next;
			}
		}   
		if(1 == flag1)
		{
			/*this last node*/
			if(NULL == pcurnode->next)
			{
				xmlUnlinkNode(pcurnode);
				xmlFreeNode(pcurnode);
				pcurnode= tempNode;
				break;
			}
			else
			{
				tempNode = pcurnode->next;  
				xmlUnlinkNode(pcurnode);
				xmlFreeNode(pcurnode);
				pcurnode= tempNode;
				continue;
			}
			//break;/*delete signle node*/
		}
		else
		{
			pcurnode = pcurnode->next;
		}
	}	  
	xmlSaveFile(xml_fpath,pdoc);
	xmlFreeDoc(pdoc);
	return 0;
}

int del_clilog_node( char *xml_fpath, char *ip,char *port, char *filter ,char *utcp,char *flevel)
{
	xmlDocPtr pdoc = NULL;
	xmlNodePtr pcurnode = NULL;
	char *psfilename;
	psfilename = xml_fpath;
	int flag1 = 0,flag2 = 0,flag3 = 0,flag4 = 0,flag5 = 0;
	pdoc = xmlReadFile(psfilename,"utf-8",256); 

	if(NULL == pdoc)
	{
		return 1;
	}
	xmlNodePtr tempNode = NULL;
	xmlNodePtr childPtr = NULL;
	xmlChar *value;
	pcurnode = xmlDocGetRootElement(pdoc);
	pcurnode = pcurnode->xmlChildrenNode;

	while (NULL != pcurnode)
	{			
		tempNode = NULL;
		if(NULL == pcurnode->next)
		{
			tempNode = pcurnode;
		}
		else
		{
			tempNode = pcurnode->next;  
		}
		flag5 = 0;
		if (!xmlStrcmp(pcurnode->name, BAD_CAST NODE_LOG))
		{
			childPtr = NULL;
			childPtr = pcurnode; 
			childPtr=childPtr->children;

		    flag1 = 0;
			flag2 = 0;
			flag3 = 0;
			flag4 = 0;
			while(childPtr !=NULL)
			{	 
				if(!xmlStrcmp(childPtr->name, BAD_CAST S_FILT))
				{
				    
					value = xmlNodeGetContent(childPtr);
					if(strcmp(filter,(char *)value)==0)
					{
						flag1 = 1;
					}
					else
					{
						break;
					}
					xmlFree(value);
				}
				if(!xmlStrcmp(childPtr->name, BAD_CAST NODE_SYSIP))
				{
				    
					value = xmlNodeGetContent(childPtr);  
					if(strcmp(ip,(char *)value)==0)
					{
						flag2 = 1;
					}
					else
					{
						break;
					}
					xmlFree(value);
				}
				if(!xmlStrcmp(childPtr->name, BAD_CAST NODE_SYSPORT))
				{
				    
					value = xmlNodeGetContent(childPtr);
					if(strcmp(port,(char *)value)==0)
					{
						flag3 = 1;
					}
					else
					{
						break;
					}
					xmlFree(value);
				}
				if(!xmlStrcmp(childPtr->name, BAD_CAST NODE_INDEXZ))
				{
				    
					value = xmlNodeGetContent(childPtr);
					if(strcmp(flevel,(char *)value)==0)
					{
						flag4 = 1;
					}
					else
					{
						break;
					}
					xmlFree(value);
				}

				if((1 == flag1) && (1 == flag2) && (1 == flag3)&& (1 == flag4))
				{
					flag5 = 1;
					break;
					
				}
				childPtr = childPtr->next;
			}
		}   
		if(1 == flag5)
		{
			/*this last node*/
			if(NULL == pcurnode->next)
			{
				xmlUnlinkNode(pcurnode);
				xmlFreeNode(pcurnode);
				pcurnode= tempNode;
			}
			else
			{
				tempNode = pcurnode->next;  
				xmlUnlinkNode(pcurnode);
				xmlFreeNode(pcurnode);
				pcurnode= tempNode;			   
			}
			break;
		}
		else
		{
			pcurnode = pcurnode->next; 
		}
	}	  
	xmlSaveFile(xml_fpath,pdoc);  
	xmlFreeDoc(pdoc);
	return 0;
}
int add_destination_rule(char *rulename,char *ip ,char *port)
{
	ST_SYS_ALL sysall,logall;
	memset( &sysall, 0, sizeof(ST_SYS_ALL) );
	
   // time_t now;
	//unsigned int timez = (unsigned int)time(&now);
	
	memset( &logall, 0, sizeof(ST_SYS_ALL) );
	sprintf( logall.suinfo[0].suname,  "%s",NODE_VALUE );
	sprintf( logall.suinfo[0].content, "%s", rulename );
	
	sprintf( logall.suinfo[1].suname,  "%s",NODE_CONTENT );
	sprintf( logall.suinfo[1].content, "udp(\"%s\" port(%s));", ip, port );
	
	sprintf( logall.suinfo[2].suname,  "%s",NODE_MARKZ);
	sprintf( logall.suinfo[2].content, "%d", 1);
	
	sprintf( logall.suinfo[3].suname,  "%s",NODE_SYSIP);
	sprintf( logall.suinfo[3].content, "%s", ip);
	
	sprintf( logall.suinfo[4].suname,  "%s",NODE_SYSPORT);
	sprintf( logall.suinfo[4].content, "%s", port);

	sprintf( logall.suinfo[5].suname,  "%s",NODE_PROTOCOLZ);
	sprintf( logall.suinfo[5].content, "%s", "udp");

	add_second_xmlnode(XML_FPATH, NODE_DES, &logall);
	return 0;
}
int add_log_rule(char *filter,char *desname)
{
	ST_SYS_ALL sysall,logall;	
    time_t now;
	int fid = 0;

	unsigned int timez = (unsigned int)time(&now);
	memset( &sysall, 0, sizeof(ST_SYS_ALL) );
	
	sprintf( sysall.suinfo[0].suname, "%s",NODE_ENABLES );
	sprintf( sysall.suinfo[0].content, "%s","1" );
	
	sprintf( sysall.suinfo[1].suname,  "%s",CH_SOURCE);
	sprintf( sysall.suinfo[1].content,  "%s","s_all" );
	
	sprintf( sysall.suinfo[2].suname,  "%s",CH_FILTER);
	sprintf( sysall.suinfo[2].content,  "%s",filter);

	sprintf( sysall.suinfo[3].suname, "%s", CH_DEST);
	sprintf( sysall.suinfo[3].content, "%s", desname);
	
	sprintf( sysall.suinfo[4].suname,  "%s",NODE_MARKZ);
	sprintf( sysall.suinfo[4].content, "%d", timez);
	
	add_second_xmlnode( XML_FPATH, NODE_LOG, &sysall);
	
	return 0;
}

