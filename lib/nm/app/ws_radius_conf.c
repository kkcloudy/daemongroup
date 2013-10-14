#include "ws_log_conf.h"
#include "ws_radius_conf.h"
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
#if 0
int is_file_exist( char *filtpath )
{
	return access( filtpath, 0 ); 
}
#endif
/*启动服务, 可设定值*/
int start_radius()
{
	int iRet,status;
	
//	status = system( RAD_PATH" "S_START);
       status = system ("sudo /opt/services/init/radius_init start");
	iRet = WEXITSTATUS(status);
	if(iRet==0)
	{
		mod_first_xmlnode(XML_RADIUS_PATH, "lstatus", "enable");
	}
	return iRet;
}

/*停止服务, 可设定值*/
int stop_radius()
{
	int iRet,status;
	
	status = system("sudo /opt/services/init/radius_init stop");
	iRet = WEXITSTATUS(status);
	if(iRet==0)
	{
		mod_first_xmlnode(XML_RADIUS_PATH,"lstatus", "disable");
	}
	return iRet;
}

/*重启radius服务, 可设定值*/
int restart_radius()
{
	int iRet,status;	
	status = system( "sudo /opt/services/init/radius_init restart");
	iRet = WEXITSTATUS(status);
	if(iRet==0)
	{
		mod_first_xmlnode(XML_RADIUS_PATH, "lstatus", "enable");
	}
	else
	{
		mod_first_xmlnode(XML_RADIUS_PATH, "lstatus", "disable");
	}	

	return iRet;
}

int if_radius_enable()
{
	char *tmpz=(char *)malloc(128);
	memset(tmpz,0,128);
	int sflag=-1;
	FILE *pipe;
	pipe=popen("ps -ef|grep /opt/bin/radiusd |grep -v grep|wc -l","r");
	if(pipe==NULL)
	{
		sflag=0;                         //disable
	}
	else
	{
		fgets( tmpz, 128, pipe );	
		sflag=strtoul(tmpz,0,10);						   
	}
	pclose(pipe);
	free(tmpz);
	return sflag;
}

void if_radius_exist()
{
	char *cmd=(char *)malloc(128);
	memset(cmd,0,128);
	if(access(XML_RADIUS_PATH,0)!=0)
	{
		memset(cmd,0,128);
		sprintf(cmd,"sudo cp %s  %s",XML_RADIUS_D,XML_RADIUS_PATH);
		system(cmd);
		memset(cmd,0,128);
		sprintf(cmd,"sudo chmod 666 %s",XML_RADIUS_PATH);
		system(cmd);
	}
	if(access(CONF_RADIUS_PATH,0)!=0)
	{
		memset(cmd,0,128);
		sprintf(cmd,"sudo cp %s %s",CONF_RADIUS_D,CONF_RADIUS_PATH);
		system(cmd);
		memset(cmd,0,128);
		sprintf(cmd,"sudo chmod 666 %s",CONF_RADIUS_PATH);
		system(cmd);
	}
	free(cmd);
}

void if_radius_client_exist()
{
	char *cmd=(char *)malloc(128);
	memset(cmd,0,128);
	if(access(XML_RADIUS_CLIENT_PATH,0)!=0)
	{
		memset(cmd,0,128);
		sprintf(cmd,"sudo cp %s  %s",XML_RADIUS_CLIENT_D,XML_RADIUS_CLIENT_PATH);
		system(cmd);
		memset(cmd,0,128);
		sprintf(cmd,"sudo chmod 666 %s",XML_RADIUS_CLIENT_PATH);
		system(cmd);
	}
	if(access(CONF_RADIUS_CLIENT_PATH,0)!=0)
	{
		memset(cmd,0,128);
		sprintf(cmd,"sudo cp %s %s",CONF_RADIUS_CLIENT_D,CONF_RADIUS_CLIENT_PATH);
		system(cmd);
		memset(cmd,0,128);
		sprintf(cmd,"sudo chmod 666 %s",CONF_RADIUS_CLIENT_PATH);
		system(cmd);
	}
	free(cmd);
}

int mod_first_radius_xmlnode(char *xmlpath,char *node_name,char *newc)
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
	if(pcurnode==NULL)
	{
		xmlFreeDoc(pdoc);
		return -1;
	}
	if (xmlStrcmp(pcurnode->name,  BAD_CAST "root")) 
	{
		xmlFreeDoc(pdoc);
		return -1;
	}
	design_node=pcurnode;
	pcurnode = pcurnode->xmlChildrenNode; 
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

int get_first_radius_xmlnode(char *xmlpath,char *node_name,char *newc)
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

int del_first_radius_xmlnode(char *xmlpath,char *node_name,char *newc)
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
		if (!xmlStrcmp(pcurnode->name, BAD_CAST node_name))   //find the first level node            
		{
			xmlNodePtr tempNode;				  		   
		       tempNode = pcurnode->next;  
		       xmlUnlinkNode(pcurnode);
			xmlFreeNode(pcurnode);
			pcurnode= tempNode;			   
			continue;
		}  
		pcurnode = pcurnode->next; 
	}	  
	xmlSaveFile(xmlpath,pdoc);
	xmlFreeDoc(pdoc);
	return 0;
}


int read_module_ldap_xml(struct ldap_st *chead,int *ldapnum)
{
	xmlDocPtr doc;	 
	xmlNodePtr cur,tmp;
    	int conflag=0;	
	doc = xmlReadFile(XML_RADIUS_PATH, "utf-8", 256); 
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

	struct ldap_st *ctail=NULL;
	chead->next=NULL;
	ctail=chead;
	
	cur = cur->xmlChildrenNode;	
	while(cur !=NULL)
	{              		
	    	tmp = cur->xmlChildrenNode;
		if (!xmlStrcmp(cur->name, BAD_CAST "ldap"))            /*find the node "ldap" in xml*/ 
		{ 
               		        conflag++;                                               /*set the flag=1 if find the "ldap" node in xml file */
		               //ldap module  information		       
		        	struct ldap_st  *cq;
				cq=(struct ldap_st *)malloc(sizeof(struct ldap_st)+1);
				memset(cq,0,sizeof(struct ldap_st)+1);
				if(NULL == cq)
				{
					return  -1;
				}

				cq->server=(char *)malloc(50);
				//memset(cq->server,0,sizeof(cq->server));
				memset(cq->server,0,50);
				cq->identify=(char *)malloc(128);
				//memset(cq->identify,0,sizeof(cq->identify));     
				memset(cq->identify,0,128); 
				cq->password=(char *)malloc(50);
				//memset(cq->password,0,sizeof(cq->password));
				memset(cq->password,0,50);
				cq->basedn=(char *)malloc(128);
				memset(cq->basedn,0,128);
				cq->filter_ldap=(char *)malloc(256);
				memset(cq->filter_ldap,0,256);
				cq->access_attr=(char *)malloc(128);
				memset(cq->access_attr,0,128);				
 				cq->connect=(char *)malloc(50);
				memset(cq->connect,0,50);
				cq->timeout=(char *)malloc(50);
				memset(cq->timeout,0,50);
				cq->timelimit=(char *)malloc(50);
			       memset(cq->timelimit,0,50);
				cq->net_time=(char *)malloc(50);
				memset(cq->net_time,0,50);	
				cq->tls=(char *)malloc(128);
				memset(cq->tls,0,128);
				cq->content=(char *)malloc(256);
				memset(cq->content,0,256);
				
				xmlChar *value=NULL;				
				while(tmp !=NULL)
				{	 
					//ldap  server
					if ((!xmlStrcmp(tmp->name, BAD_CAST NODE_SERVER)))     //test if the child node is what i want
					{
					    	memset(cq->server,0,50);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->server,(char *)value);	
						xmlFree(value);
					}
					
					//ldap identify
					else if ((!xmlStrcmp(tmp->name, BAD_CAST NODE_IDEN)))
					{
					         memset(cq->identify,0,128); 
						value=xmlNodeGetContent(tmp);
						strcpy(cq->identify,(char *)value);	
						xmlFree(value);
					}
					//ldap password
					else if ((!xmlStrcmp(tmp->name, BAD_CAST NODE_PASSWD)))
					{
					    	memset(cq->password,0,50);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->password,(char *)value);	
						xmlFree(value);
					}
					//ldap  basedn
					else if ((!xmlStrcmp(tmp->name, BAD_CAST NODE_BASEDN)))
					{
					    	memset(cq->basedn,0,128);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->basedn,(char *)value);	
						xmlFree(value);
					}
					//ldap filter
					else if ((!xmlStrcmp(tmp->name, BAD_CAST NODE_FILTER)))
					{
					         memset(cq->filter_ldap,0,256);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->filter_ldap,(char *)value);	
						xmlFree(value);
					}
					//ldap access_attr
					else if ((!xmlStrcmp(tmp->name, BAD_CAST NODE_ACCESS_ATTR)))
					{
					        memset(cq->access_attr,0,128);			
						value=xmlNodeGetContent(tmp);
						strcpy(cq->access_attr,(char *)value);	
						xmlFree(value);
					}		
					//ldap connect
					else if ((!xmlStrcmp(tmp->name, BAD_CAST NODE_CONNECT)))
					{
					       memset(cq->connect,0,50);  
						value=xmlNodeGetContent(tmp);
						strcpy(cq->connect,(char *)value);	
						xmlFree(value);
					}//ldap timeout
					else if ((!xmlStrcmp(tmp->name, BAD_CAST NODE_TIMEOUT)))
					{
					       memset(cq->timeout,0,50);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->timeout,(char *)value);	
						xmlFree(value);
					}//ldap timelimit
					else if ((!xmlStrcmp(tmp->name, BAD_CAST NODE_TIMELIMIT)))
					{
					       memset(cq->timelimit,0,50);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->timelimit,(char *)value);	
						xmlFree(value);
					}//ldap net_time
					else if ((!xmlStrcmp(tmp->name, BAD_CAST NODE_NETTIME)))
					{
					       memset(cq->net_time,0,50);	
						value=xmlNodeGetContent(tmp);
						strcpy(cq->net_time,(char *)value);	
						xmlFree(value);
					}//ldap tls
					else if ((!xmlStrcmp(tmp->name, BAD_CAST NODE_TLS)))
					{
					       memset(cq->tls,0,128);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->tls,(char *)value);	
						xmlFree(value);
					}//ldap content
					else if ((!xmlStrcmp(tmp->name, BAD_CAST NODE_CONTENT)))
					{
					       memset(cq->content,0,256);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->content,(char *)value);	
						xmlFree(value);
					}
					tmp = tmp->next;                                                  //tmp=tmp->next  <=>   cur=cur->next
				}            
				 cq->next = NULL;
			     	 ctail->next = cq;
			     	 ctail = cq;
		}  
	
	   cur = cur->next;
	}
   	 *ldapnum=conflag;
	xmlFreeDoc(doc);
	return 0;
}


int read_radius_client_xml(struct radius_client_st *chead,int *client_num)
{
	xmlDocPtr doc;	 
	xmlNodePtr cur,tmp;
    	int conflag=0;	
	doc = xmlReadFile(XML_RADIUS_CLIENT_PATH, "utf-8", 256); 
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

	struct radius_client_st *ctail=NULL;
	chead->next=NULL;
	ctail=chead;
	
	cur = cur->xmlChildrenNode;	
	while(cur !=NULL)
	{              		
	    	tmp = cur->xmlChildrenNode;
		if (!xmlStrcmp(cur->name, BAD_CAST "client"))            /*find the node "client" in clients.xml*/ 
		{ 
               		 conflag++;                                               /*set the flag=1 if find the "ldap" node in xml file */
		               //ldap module  information		       
		        	struct radius_client_st  *cq;
				cq=(struct radius_client_st *)malloc(sizeof(struct radius_client_st)+1);
				memset(cq,0,sizeof(struct radius_client_st)+1);
				if(NULL == cq)
				{
					return  -1;
				}

				cq->client_ip=(char *)malloc(50);			
				memset(cq->client_ip,0,50);
				cq->secret=(char *)malloc(128);				
				memset(cq->secret,0,128); 
				cq->nastype=(char *)malloc(50);			
				memset(cq->nastype,0,50);
				
				xmlChar *value=NULL;				
				while(tmp !=NULL)
				{	 
					//client IP 
					if ((!xmlStrcmp(tmp->name, BAD_CAST NODE_IP)))     //test if the child node is what i want
					{
					    	memset(cq->client_ip,0,50);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->client_ip,(char *)value);	
						xmlFree(value);
					}
					
					//radius secret
					else if ((!xmlStrcmp(tmp->name, BAD_CAST NODE_SECRET)))
					{
					         memset(cq->secret,0,128); 
						value=xmlNodeGetContent(tmp);
						strcpy(cq->secret,(char *)value);	
						xmlFree(value);
					}
					//client NASTYPE
					else if ((!xmlStrcmp(tmp->name, BAD_CAST NODE_NASTYPE)))
					{
					    	memset(cq->nastype,0,50);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->nastype,(char *)value);	
						xmlFree(value);
					}
					
					tmp = tmp->next;                                                  //tmp=tmp->next  <=>   cur=cur->next
				}            
				 cq->next = NULL;
			     	 ctail->next = cq;
			     	 ctail = cq;
		}  
	
	   cur = cur->next;
	}
      *client_num=conflag;
	xmlFreeDoc(doc);
	return 0;
}

//free link list
void free_read_radius_client_xml(struct radius_client_st *head,int *free_flag)
{
       struct radius_client_st *f1,*f2;
	f1=head->next;		 
	f2=f1->next;
	*free_flag=0;
	while(f2!=NULL)
	{
	  (*free_flag)++;
	  free(f1->client_ip);
	  free(f1->secret);
	  free(f1->nastype);
	  free(f1);
	  f1=f2;
	  f2=f2->next;
	}
	free(f1);
	head=NULL;
}

//free link list (ldap_st)  test ok!!!
void free_read_ldap_xml(struct ldap_st *head,int *free_flag)
{
	*free_flag=0;
        struct ldap_st *f1;
//    struct ldap_st *f2;
	f1=head->next;		 
//	f2=f1->next;
	if(f1!=NULL)
	{
	  *free_flag=1;
//	  f2=f1->next;
	  free(f1->server);
	  free(f1->basedn);
	  free(f1->connect);
	  free(f1->content);
	  free(f1->filter_ldap);
	  free(f1->identify);
	  free(f1->net_time);
	  free(f1->password);
	  free(f1->access_attr);
	  free(f1->timelimit);
	  free(f1->timeout);
	  free(f1->tls);
	  free(f1);
//	  f1=f2;
//	  f2=f2->next;
	}
//	free(f1);
	head=NULL;
}


/*xml的内容，节点无属性，fpath:xml文件路径，nodename，二级节点名称，content，三级节点名称，gets 三级节点的内容*/
int get_third_radius_xmlnode(char * fpath,char * node_name,char * content,char *gets)
{
	xmlDocPtr pdoc = NULL;
	xmlNodePtr pcurnode = NULL;
	char *psfilename;
	psfilename = fpath;
	pdoc = xmlReadFile(psfilename,"utf-8",XML_PARSE_RECOVER);  
	//int tagz = 0;

	if(NULL == pdoc)
	{
		return 1;
	}

	pcurnode = xmlDocGetRootElement(pdoc);  
	pcurnode = pcurnode->xmlChildrenNode;  

	while (NULL != pcurnode)                               //get the second level childnode
	{			
		if (!xmlStrcmp(pcurnode->name, BAD_CAST node_name))   
		{
		      //tagz ++ ;
			xmlNodePtr childPtr = pcurnode; 
			childPtr=childPtr->children;
			xmlChar *value;
			
			while(childPtr !=NULL)
			{	 
				if ((!xmlStrcmp(childPtr->name, BAD_CAST content)))    //get the third level childnode
				{
					value = xmlNodeGetContent(childPtr);   
				//	if( tagz == flagz )
				//	{
						strcpy(gets,(char *)value);
				//	}
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

/*查找指定节点，fpath，xml文件路径，nodename，二级xml节点名，content，三级xml节点名，getc，三级xml节点名下内容，flagz，标记xml*/
int find_third_radius_xmlnode(char * fpath,char * node_name,char * content,char *getc,int *flagz)
{
	xmlDocPtr pdoc = NULL;
	xmlNodePtr pcurnode = NULL;
	char *psfilename;
	psfilename = fpath;
	*flagz = 0;
	int  tagz=0,flag = 0;
	pdoc = xmlReadFile(psfilename,"utf-8",XML_PARSE_RECOVER); 

	if(NULL == pdoc)
	{		
		return 1;
	}

	pcurnode = xmlDocGetRootElement(pdoc); 
	if(pcurnode==NULL)
	{
		xmlFreeDoc(pdoc);
		return -1;
	}
	if (xmlStrcmp(pcurnode->name,  BAD_CAST "root")==0) 
	{      
	        //tagz ++; 
		xmlFreeDoc(pdoc);
		return -1;
	}
	pcurnode = pcurnode->xmlChildrenNode;                                 

	while (NULL != pcurnode)
	{			
		if (!xmlStrcmp(pcurnode->name, BAD_CAST node_name))           
		{

      		                 tagz ++;                                                                         //if find the second  level childnode, tagz++
 //      		        *flagz=0;                                                                         //debug
			xmlNodePtr childPtr = pcurnode; 
			childPtr=childPtr->children;
			xmlChar *value;

			while(NULL!=childPtr)
			{	 
				if(!xmlStrcmp(childPtr->name, BAD_CAST content))     //find the third level childnode
				{
				    
					value = xmlNodeGetContent(childPtr);      
					if(strcmp(getc,(char *)value)==0)                         //find the third level childnode which we want
					{
						flag = 1;                                                   	  //set flag=1 
						*flagz=1;
						break;
					}
					xmlFree(value);
				}
				childPtr = childPtr->next;
			}
			if(flag==0)
			{
				xmlFree(value);                                                           //?????????????
			}
		}    
		
		if(flag==0)
		break;
		
		pcurnode = pcurnode->next; 
	}	  
//	if(tagz!=0)                                      
//		*flagz = tagz;                                                                      
//	else
//		*flagz = 0;
	xmlSaveFile(fpath,pdoc);
	xmlFreeDoc(pdoc);
	return 0;
}


/*xml的内容，节点无属性，fpath:xml文件路径，nodename，二级节点名称，content，三级节点名称，newc，更改的新内容*/
int mod_third_radius_xmlnode(char * fpath,char * node_name,char * content,char *newc)
{
	xmlDocPtr pdoc = NULL;
	xmlNodePtr pcurnode = NULL;
	char *psfilename;
	psfilename = fpath;
	pdoc = xmlReadFile(psfilename,"utf-8",XML_PARSE_RECOVER);  

//	int tagz = 0;

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
 //           		tagz ++ ;
			xmlNodePtr childPtr = pcurnode; 
			childPtr=childPtr->children;
			xmlChar *value;
			
			while(childPtr !=NULL)
			{	 
				if ((!xmlStrcmp(childPtr->name, BAD_CAST content)))
				{
				    
					value = xmlNodeGetContent(childPtr);   
//					if( tagz == flagz )
//					{
						xmlNodeSetContent(childPtr, BAD_CAST  newc); 
//					}
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

/*add the third level node (second level node already exist)fpath:xml file path,node_name:second level node,content:sub second node name,keyz:node content*/
int add_radius_third_xmlnode(char * fpath,char * second_node_name,char * third_node_name,char *third_node_content)
{
	int third_node_flag=0;
	xmlDocPtr pdoc = NULL;
	xmlNodePtr pcurnode = NULL;
//	xmlNodePtr tempnode=NULL;
	char *psfilename;
	psfilename = fpath;
	pdoc = xmlReadFile(psfilename,"utf-8",256); 

	if(NULL == pdoc)
	{
		return 1;
	}	
	find_third_radius_xmlnode(psfilename, second_node_name, third_node_name, third_node_content,&third_node_flag);
	if( third_node_flag != 0 )
	{
	      //the same third level node already exist
		return 1;		
	}
	else
	{
	  
//	       xmlNodePtr node = xmlNewNode(NULL,BAD_CAST second_node_name);           //node_name:the second level childnode
//	       xmlAddChild(pcurnode,node);  
	 	 pcurnode = xmlDocGetRootElement(pdoc); 
		 pcurnode = pcurnode->xmlChildrenNode; 
		 while (NULL != pcurnode)
		{		
			//xmlChar *value;
			if (!xmlStrcmp(pcurnode->name, BAD_CAST second_node_name))                   //find the  second node          
			{
			 	 xmlNodePtr node1 = xmlNewNode(NULL,BAD_CAST third_node_name);             //create a third level childnode 
	 			 xmlNodePtr content1 = xmlNewText(BAD_CAST third_node_content);              //set the third level childnode's name
				 xmlAddChild(pcurnode,node1);											//add the third level childnode
				 xmlAddChild(node1,content1);		
			}  
			pcurnode = pcurnode->next; 
		}		
	
	}	
	xmlSaveFile(fpath,pdoc);
	xmlFreeDoc(pdoc);
	return 0;
}

int write_xml2config_radiusd(char *filepath,struct ldap_st *lst)
{
     FILE *fp=NULL;
     char  file_config[90000],content[10000];
     memset(file_config,0,90000);
     memset(content,0,10000);
     if(NULL == filepath)
     	{
     		return -1;
     	}
     if((fp=fopen(filepath,"w+"))==NULL)
     	{
     		return -2;
     	}
	 char locatedir[512]="prefix = /opt/radius \n"
                                     "exec_prefix = ${prefix} \n"
                                     "sysconfdir = ${prefix} \n"
                                     "localstatedir=/var \n"
                                	 "sbindir = ${exec_prefix}/sbin \n"
                                 	 "logdir = ${localstatedir}/log/ \n"
                                     "raddbdir = ${sysconfdir}/raddb \n"
                                     "radacctdir = ${logdir}/radacct \n"
                                     "confdir = ${raddbdir} \n"
                                 	 "run_dir = ${localstatedir}/run/radius \n"
                                	 "db_dir = $(raddbdir)\n"
                                	 "libdir = /opt/lib/radius\n"
                                	 "pidfile = /var/run/radius.pid\n"
                              	 "max_request_time = 30\n"
                               	 "cleanup_delay = 5\n"
                               	 "max_requests = 1024\n";
	char listen[128]="listen  {\n type = auth\n ipaddr = *\n port = 0\n"
                    		   "}\n\n  listen  {\n ipaddr = *\n port = 0\n  type = acct\n }\n\n";
	char host[128]="hostname_lookups = no\n allow_core_dumps = no\n regular_expressions= yes\n extended_expressions = yes\n";
       char log[256]="log  {\n destination=files\n file = ${logdir}/radius.log\n syslog_facility = daemon\n stripped_names = no\n"	
		     		 "auth= no\n auth_badpass = no\n auth_goodpass = no\n }\n\n";
      char checkrad_security[256]="checkrad = ${sbindir}/checkrad\n\n"
			                             "security  {\n max_attributes = 200\n reject_delay = 1\n status_server = yes\n }\n\n";
	char proxy_include[256]="proxy_requests  = yes\n $INCLUDE proxy.conf\n $INCLUDE clients.conf\n snmp= no\n $INCLUDE snmp.conf\n\n";
       char thread_pool[256]="thread pool  {\n  start_servers = 5 \n max_servers = 32 \n min_spare_servers = 3\n max_spare_servers = 10\n"
			       		"max_requests_per_server = 0\n }\n\n";
	char module[128]="modules   { \n\n $INCLUDE eap.conf \n mschap   {\n  }\n\n";
#if 0
	char ldap[512]="ldap {\n" 
			"server = \"192.168.7.251\"\n"
			"identity = \"cn=Manager,dc=test,dc=com\"\n"
			"password = \"example\"\n" 
	               "basedn = \"dc=test,dc=com\" \n  filter = \"(uid=%{Stripped-User-Name:-%{User-Name}})\"\n "
			"#base_filter = \"(objectclass=radiusprofile)\" \n"
            		"access_attr=\"uid\"\n  ldap_connections_number = 5\n  timeout = 4\n "	
	              "timelimit = 3\n net_timeout = 1\n	 tls\n {\n start_tls = no\n  }\n\n"
			"dictionary_mapping = ${confdir}/ldap.attrmap\n"  
			"password_attribute = userPassword\n  edir_account_policy_check = yes\n"
			" set_auth_type = yes \n  }\n\n";
#endif
	
	char realm_IPASS[512]="realm IPASS {\n format = prefix\n"
							"delimiter = \"/\" \n}\n\n";
	char realm_suffix[128]="realm suffix  {\n format = suffix\n"
						 "delimiter = \"@\"\n}\n\n";
	char realm_realmpercent[128]="realm realmpercent {\n"
							"format = suffix\n"
							"delimiter = \"%\"\n}\n\n";
	char realm_ntdomain[128]="realm ntdomain {\n"
                            		"format = prefix\n"
                            		"delimiter = \"";
      char realm_ntdomain_end[20]="\"\n}\n\n";   
    sprintf(realm_ntdomain,"%s%s%s%s",realm_ntdomain,"\\","\\",realm_ntdomain_end);
//      printf("%s",realm_ntdomain);
      char checkval[128]="checkval {\n "
						"item-name = Calling-Station-Id\n"
						"check-name = Calling-Station-Id\n"
						"data-type = string\n"
						"}\n\n";
	char preprocess[256]="preprocess  {\n"
		                    "huntgroups = ${confdir}/huntgroups \n"
				      "hints = ${confdir}/hints\n"
				      "with_ascend_hack = no \n"
				      "ascend_channels_per_line = 23 \n"
				      "with_ntdomain_hack = no \n"
				      "with_specialix_jetstream_hack = no\n"		
				      "with_cisco_vsa_hack = no\n"
					"}\n\n";
	char files[256]="files  {\n"
					"usersfile = ${confdir}/users \n"
					"acctusersfile = ${confdir}/acct_users \n"
					"preproxy_usersfile = ${confdir}/preproxy_users \n"	
					"compat = no \n"
					"}\n\n";
	
	char detail[128]="detail  {\n"		
					"detailfile = ${radacctdir}/%{Client-IP-Address}/detail-%Y%m%d\n"		
					"detailperm = 0600\n"	
					"header = \"%t\"\n"	
					"}\n\n";
	
	char acct_unique[128]="acct_unique  {\n"
					"key = \"User-Name, Acct-Session-Id, NAS-IP-Address, Client-IP-Address, NAS-Port\"\n"
						"}\n\n";
	char sql_conf[128]="$INCLUDE  sql.conf \n  $INCLUDE sql/mysql/counter.conf\n $INCLUDE  ${confdir}/sqlcounter.conf\n";                                                  /*for mysql */                                                                                                                                                              
                                                                                                                    /*for mysql */
	char always[512]="always fail  {\n"
					"rcode = fail \n"
					"}\n"
					"always reject  {\n"
					"rcode = reject \n"
					"}\n"
					"always noop  {\n"
					"rcode = noop \n"
					"}\n"
					"always handled {\n"
					"rcode = handled \n"
						"}\n"
					"always updated {\n"
					"rcode = updated \n"
					"}\n"
					"always notfound {\n"
					"rcode = notfound \n"
					"}\n"
					"always ok {\n"
					"rcode = ok\n"
					"simulcount = 0\n"
					"mpp = no\n"
					"}\n";
	char expr[256]="expr   {\n"
						"}\n"
					"digest   {\n"
						"}\n"
					"expiration  {\n"	
					"reply-message = \"Password Has Expired\" \n"
					"}\n"	
					"logintime   {\n"	
					"reply-message = \"You are calling outside your allowed timespan\" \n"		
					"minimum-timeout = 60\n"
					"}\n";
	char exec[256]="exec {\n"
					"wait = yes \n"
					"input_pairs = request \n"
					"shell_escape = yes \n"
					"output = none \n"
					"}\n"
					"exec echo {\n"		
					"wait = yes \n"		
					"program = \"/bin/echo %{User-Name}\" \n"		
					"input_pairs = request \n"	
					"output_pairs = reply \n"
					"shell_escape = yes \n"
					"} \n ";
	char ip_pool[512]="ippool main_pool { \n"		
					"range-start = 192.168.1.1 \n"
					"range-stop = 192.168.3.254	\n"
					"netmask = 255.255.255.0 \n"		
					"cache-size = 800 \n"		
					"session-db = ${db_dir}/db.ippool \n"	
					"ip-index = ${db_dir}/db.ipindex \n"		
					"override = no \n "		
					"maximum-timeout = 0 \n"		
					"}\n"	
					"policy {	\n"      
					"filename = ${confdir}/policy.txt \n"
					"} \n"
					"} \n\n";
	char instantiate[256]="instantiate { \n"	
			   		"exec \n"	
	 		   		"expr \n"
			   		"expiration \n"
			   		"logintime  \n"
		           		"}\n\n"
					"$INCLUDE policy.conf \n"
					"$INCLUDE sites-enabled/ \n";
    sprintf(file_config,"%s %s %s %s %s %s %s %s  ",locatedir,listen,host,log,checkrad_security,proxy_include,thread_pool,module);
    memset(content,0,10000);
    sprintf(content,"ldap {\n %s \n %s \n  %s \n %s \n %s \n ",lst->server,lst->identify,lst->password,lst->basedn,lst->filter_ldap);
    strcat(file_config,content);
	  
    memset(content,0,10000);
    sprintf(content,"\n %s \n  %s \n  %s \n  %s \n  %s \n  tls { \n %s \n } %s \n }\n ",lst->access_attr,lst->connect,lst->timeout,lst->timelimit,lst->net_time,lst->tls,lst->content);
    strcat(file_config,content);
	
    memset(content,0,10000);
    sprintf(content,"%s %s %s %s %s %s %s %s %s  %s ",realm_IPASS,realm_suffix,realm_realmpercent,realm_ntdomain,checkval,preprocess,files,detail,acct_unique,always);
    strcat(file_config,content);

    memset(content,0,10000);
    sprintf(content," %s %s %s %s %s %s ",preprocess,files,detail,acct_unique,sql_conf,always);
    strcat(file_config,content);
	
    memset(content,0,10000);
    sprintf(content,"%s %s %s %s",expr,exec,ip_pool,instantiate);
    strcat(file_config,content);

    fwrite(file_config,strlen(file_config),1,fp);	
    fclose(fp);
    return 0;
}

int write_xml2config_radiusd_client(char *filepath,struct radius_client_st * clist)
{
     FILE *fp=NULL;
     struct radius_client_st *cq;
     char  file_config[10000];
     memset(file_config,0,10000); 
     char * content=(char *)malloc(128);
     char * secret=(char *)malloc(128);
     char * nas=(char *)malloc(128);
    
     if(NULL == filepath)
     	{
     		free(content);
			free(secret);
			free(nas);
     		return -1;
     	}
     if((fp=fopen(filepath,"w+"))==NULL)
     	{
     		free(content);
			free(secret);
			free(nas);
     		return -2;
     	}
       char localhost[256]="client localhost  {  \n"
					"ipaddr = 127.0.0.1  \n"
					"secret = testing123  \n"
 					"require_message_authenticator = no \n"
					"nastype  = other \n"	
					"} \n";
	strcpy(file_config,localhost);
	cq=clist->next;
 	while(NULL !=cq)
 	{
		memset(content,0,128);
		sprintf(content," %s { \n",cq->client_ip);
		strcat(file_config,content);

		memset(secret,0,128);
		sprintf(secret,"%s \n",cq->secret);
		strcat(file_config,secret);

		memset(nas,0,128);
		sprintf(nas,"%s \n }\n\n",cq->nastype);
		strcat(file_config,nas);

		cq=cq->next;
	}
    	fwrite(file_config,strlen(file_config),1,fp);	
    	fclose(fp);
	free(content);
	free(secret);
	free(nas);
    	return 0;
}

/*get second level xml node(with property) ,node_name:second node name;attribute:attribute name;ruler:the vlaue of attribute */
int get_radius_second_node_attr(char *fpath,char * node_name,char *attribute,char *ruler)
{
	 xmlDocPtr pdoc = NULL;
	 xmlNodePtr pcurnode = NULL; 
        xmlNodePtr pcurnode_f = NULL;
	 char *psfilename;
     	 psfilename = fpath;
	 int flag=0;
	 pdoc = xmlReadFile(psfilename,"utf-8",256); 	 
	  if(NULL == pdoc)
	  {
	     return 1;
	  }
	  pcurnode = xmlDocGetRootElement(pdoc);  
	  pcurnode_f = pcurnode;
	  pcurnode = pcurnode->xmlChildrenNode;  

	  while (NULL != pcurnode)
	  {			
	          if (!xmlStrcmp(pcurnode->name, BAD_CAST node_name))                
	          {
				xmlChar *value;
				value = xmlGetProp(pcurnode, (const xmlChar *)attribute);
				if(value != NULL)
				{
					if(strcmp(ruler,(char*)value)==0)
					{
						flag = 1;                           // if the node (with attribute) has already exists
					}
				}
				xmlFree(value);
		  }        
		  pcurnode = pcurnode->next; 									 
	  }
	   xmlSaveFile(fpath,pdoc);
	   xmlFreeDoc(pdoc);
	  return flag;
}

/*add second level xml node(with property) ,node_name:second node name;attribute:attribute name;ruler:the vlaue of attribute */
int add_second_node_attr(char *fpath,char * node_name,char *attribute,char *ruler)
{
	 xmlDocPtr pdoc = NULL;
	 xmlNodePtr pcurnode = NULL; 
        xmlNodePtr pcurnode_f = NULL;
	 char *psfilename;
     	 psfilename = fpath;
	 int flag=0;
	 pdoc = xmlReadFile(psfilename,"utf-8",256); 	 
	  if(NULL == pdoc)
	  {
	     return 1;
	  }
	  pcurnode = xmlDocGetRootElement(pdoc);  
	  pcurnode_f = pcurnode;
	  pcurnode = pcurnode->xmlChildrenNode;  

	  while (NULL != pcurnode)
	  {			
	          if (!xmlStrcmp(pcurnode->name, BAD_CAST node_name))                
	          {
				xmlChar *value;
				value = xmlGetProp(pcurnode, (const xmlChar *)attribute);
				if(value != NULL)
				{
					if(strcmp(ruler,(char*)value)==0)
					{
						flag = 1;                           // if the node (with attribute) has already exists
					}
				}
				xmlFree(value);
		  }        
		  pcurnode = pcurnode->next; 									 
	  }
    //if the node (with attribute) didn't exist,create a new one 
    if(flag==0)
	{
	       xmlNodePtr node = xmlNewNode(NULL, BAD_CAST node_name);
		xmlSetProp(node, BAD_CAST attribute, BAD_CAST ruler);
		xmlAddChild(pcurnode_f, node);
	}
    xmlSaveFile(fpath,pdoc);
	xmlFreeDoc(pdoc);
	return flag;
}


/*node_name:second level node;  attribute:the attribute name; ruler:the valude of attribute ;content:third node name*/
int add_radius_client_third_node(char * fpath,char * node_name,char *attribute,char *ruler,char * third_node_name,char *third_node_content)
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
							xmlNodePtr node1 = xmlNewNode(NULL,BAD_CAST third_node_name);             //create a third level childnode 
	 						xmlNodePtr content1 = xmlNewText(BAD_CAST third_node_content);              //set the third level childnode's name
							xmlAddChild(pcurnode,node1);											//add the third level childnode
				 			xmlAddChild(node1,content1);
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

/*delete second level xml node(with attribute)*/
int delete_radius_client_second_xmlnode(char *fpath,char *node_name,char *attribute,char *ruler)
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
                      propNodePtr = pcurnode;	
		        xmlChar* szAttr = xmlGetProp(propNodePtr,BAD_CAST attribute);  
		        if(!xmlStrcmp(szAttr,BAD_CAST ruler))  
		        {     		            
				xmlNodePtr tempNode;				  		   
				tempNode = pcurnode->next;  
				xmlUnlinkNode(pcurnode);
				xmlFreeNode(pcurnode);
				pcurnode = tempNode;			 
				flag=0;
				continue;
			}
		        xmlFree(szAttr);
		}        
		pcurnode = pcurnode->next; 
	}	  
	xmlSaveFile(fpath,pdoc); 
	xmlFreeDoc(pdoc);
	return flag;
}

/*delete third level xml node(with attribute)*/
int delete_radius_client_third_xmlnodel(char *fpath,char *node_name,char *attribute,char *ruler,char *secnode)
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
			xmlChar* szAttr = xmlGetProp(propNodePtr,BAD_CAST attribute);  
			if(!xmlStrcmp(szAttr,BAD_CAST ruler))  
			{     
				//deal with the next level node(third level node)
				xmlNodePtr childPtr = propNodePtr; 
				childPtr=childPtr->children;
				while(childPtr !=NULL)
				{	 
					if ((!xmlStrcmp(childPtr->name, BAD_CAST secnode)))
					{
						xmlNodePtr tempNode;				  		   
						tempNode = childPtr->next;  
						xmlUnlinkNode(childPtr);
						xmlFreeNode(childPtr);
						childPtr= tempNode;			   
						continue;
					}
					childPtr = childPtr->next;
				}
			}
			xmlFree(szAttr);
		}        
		pcurnode = pcurnode->next; 
	}	  
	xmlSaveFile(fpath,pdoc); 
	xmlFreeDoc(pdoc);
	return 0;
}


