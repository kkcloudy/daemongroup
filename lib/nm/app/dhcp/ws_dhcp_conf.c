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
* ws_dhcp_conf.c
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
#include "ws_dhcp_conf.h"
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpathInternals.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
/*#include "cgic.h"*/

#define BAD_CAST (const xmlChar *)

int start_dhcpvs()
{
	int iRet,status;
	
	status = system( DHCPVS_INITZ" "DHCPVS_START);
	iRet = WEXITSTATUS(status);
	
	return iRet;
}


int stop_dhcpvs()
{
	int iRet,status;
	
	status = system( DHCPVS_INITZ" "DHCPVS_STOP);
	iRet = WEXITSTATUS(status);
	
	return iRet;
}



int restart_dhcpvs()
{
	int iRet,status;	
	status = system( DHCPVS_INITZ" "DHCPVS_RESTART);
	iRet = WEXITSTATUS(status);
	
	return iRet;
}


/*/add xml node , property  one level*/
int add_dhcp_node_attr(char *fpath,char * node_name,char *attribute,char *ruler)
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
								flag = 1;
						}
					}
					xmlFree(value);
			  }        
			 pcurnode = pcurnode->next; 
									 
	  }

//root下为空节点的时候，就创建一个大的节点，并带上属性
    if(flag==0)
	{
	    xmlNodePtr node = xmlNewNode(NULL, BAD_CAST node_name);
		xmlSetProp(node, BAD_CAST attribute, BAD_CAST ruler);
		xmlAddChild(pcurnode_f, node);
	}
    xmlSaveFile(fpath,pdoc); 
	return flag;
}


//mod xml node
int mod_dhcp_node(char * fpath,char * node_name,char *attribute,char *ruler,char * content,char *newc)
{
		xmlDocPtr pdoc = NULL;

		xmlNodePtr pcurnode = NULL;

		xmlNodePtr design_node = NULL;

		pdoc = xmlReadFile(fpath,"utf-8",256);  

		if(NULL == pdoc)
		{
			return 1;
		}

        int if_second_flag=0,if_second_null=0;
		
		pcurnode = xmlDocGetRootElement(pdoc); 

		pcurnode = pcurnode->xmlChildrenNode; 

		while (NULL != pcurnode)
	    {			

			if (!xmlStrcmp(pcurnode->name, BAD_CAST node_name))   //找到 des 这个 root 下一级子节点             
			{
					
					xmlChar* szAttr = xmlGetProp(pcurnode,BAD_CAST attribute);  //取出属性值     

					if(!xmlStrcmp(szAttr,BAD_CAST ruler))
					{		
						//此处来处理下一级的节点,如果没有下一级的节点
						xmlNodePtr childPtr = pcurnode; 
						childPtr=childPtr->children;
					
						while(childPtr !=NULL)
						{	 
						    if_second_null=1;
							//有这个节点就修改，没有就创建
							if ((!xmlStrcmp(childPtr->name, BAD_CAST content)))
							{
 								xmlNodeSetContent(childPtr, BAD_CAST  newc); 
								if_second_flag=0;
								break;
								 
							}
							else
							{
							    if_second_flag=1;
								design_node=pcurnode;
								
							}

							childPtr = childPtr->next;
						}
						//如果二级节点是空的话
						if(if_second_null == 0)
						{
                            if_second_flag=1;
							design_node=pcurnode;
						}
					}

					xmlFree(szAttr);
			}        
			pcurnode = pcurnode->next; 

	    }	  
	    /*如果为空或者是没有此二级节点，都要进行创建,现在是创建不成功
	     pcurnode 子节点的集合*/
		if(if_second_flag==1)
		{

			   xmlNodePtr node = xmlNewNode(NULL,BAD_CAST content);    

               xmlAddChild(design_node,node);
                     
               xmlNodePtr content1 = xmlNewText(BAD_CAST newc);
                     
               xmlAddChild(node,content1);
			   design_node=NULL;
	
	    }
		 
		xmlSaveFile(fpath,pdoc);  
		return if_second_flag;
}

//if this file is xml_file
int if_xml_file(char * fpath)
{
	xmlDocPtr pdoc = NULL;
	
	char *psfilename;
	psfilename = fpath;

	pdoc = xmlReadFile(psfilename,"utf-8",256); 

	if(NULL == pdoc)
	{
		return -1;
	}
	
	return 0;
}

//create xml for mibs
int create_dhcp_xml(char *xmlpath)
{
	xmlDocPtr doc = NULL;			/* document pointer */	
	xmlNodePtr root_node = NULL;
	
	doc = xmlNewDoc(BAD_CAST "1.0");
	root_node = xmlNewNode(NULL, BAD_CAST "root" );
	xmlDocSetRootElement(doc, root_node);

	xmlSaveFormatFileEnc(xmlpath, doc, "UTF-8", 1);
	/*free the document */
	xmlFreeDoc(doc);
	xmlCleanupParser();
	xmlMemoryDump();	  
	
	return 0;	
}


//get xml node signle
int get_dhcp_node_attr(char * fpath,char * node_name,char *attribute,char *ruler,char * content,char *logkey)
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
		    propNodePtr = pcurnode;	

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
						xmlFree(value);
					}

					childPtr = childPtr->next;
				}
			}
			xmlFree(szAttr);
		}        
		pcurnode = pcurnode->next; 
	}	  
	xmlSaveFile(fpath,pdoc);  
	return 0;
}
//get node under root has not attributes 
int get_dhcp_node(char * fpath,char * node_name,char *content)
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

	xmlChar *value;

	while (NULL != pcurnode)
	{			
		if (!xmlStrcmp(pcurnode->name, BAD_CAST node_name))           
		{
		    
			value = xmlNodeGetContent(pcurnode);        	
			strcpy(content,(char *)value);			 
			xmlFree(value);
		}        
		pcurnode = pcurnode->next; 
	}	  
	xmlSaveFile(fpath,pdoc);  
	return 0;
}

//get node content ,for struct conf 
int get_conf_struct(char * fpath,char * node_name,char *attribute,char *ruler,struct confinfo *confs)
{
	xmlDocPtr pdoc = NULL;

	xmlNodePtr pcurnode = NULL,tmp;
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

        tmp = pcurnode->xmlChildrenNode;
		if (!xmlStrcmp(pcurnode->name, BAD_CAST node_name))           
		{
		    propNodePtr = pcurnode;	

			xmlChar* szAttr = xmlGetProp(propNodePtr,BAD_CAST attribute);  

			if(!xmlStrcmp(szAttr,BAD_CAST ruler))  
			{	
                
				//xmlNodePtr childPtr = propNodePtr; 
				//childPtr=childPtr->children;
				xmlChar *value;

				while(tmp !=NULL)
				{	 
					//netmask
					if ((!xmlStrcmp(tmp->name, BAD_CAST DHCP_NET_MASK)))
					{
					    memset(confs->netmask,0,30);
						//value = xmlNodeListGetString(doc, tmp->xmlChildrenNode, 1);
						value=xmlNodeGetContent(tmp);
						strcpy(confs->netmask,(char *)value);	
						xmlFree(value);
					}
					//confname
					if ((!xmlStrcmp(tmp->name, BAD_CAST DHCP_CONFNAME_Z)))
					{
					    memset(confs->confname,0,30);
						//value = xmlNodeListGetString(doc, tmp->xmlChildrenNode, 1);
						value=xmlNodeGetContent(tmp);
						strcpy(confs->confname,(char *)value);	
						xmlFree(value);
					}
					//subnet
					if ((!xmlStrcmp(tmp->name, BAD_CAST DHCP_SUBNET_Z)))
					{
					    memset(confs->subnetz,0,30);
						//value = xmlNodeListGetString(doc, tmp->xmlChildrenNode, 1);
						value=xmlNodeGetContent(tmp);
						strcpy(confs->subnetz,(char *)value);	
						xmlFree(value);
					}
					//soureip
					if ((!xmlStrcmp(tmp->name, BAD_CAST DHCP_OPT_VENDER)))
					{
					    memset(confs->sip,0,128);
						//value = xmlNodeListGetString(doc, tmp->xmlChildrenNode, 1);
						value=xmlNodeGetContent(tmp);
						strcpy(confs->sip,(char *)value);	
						xmlFree(value);
					}
					//hexip
					if ((!xmlStrcmp(tmp->name, BAD_CAST DHCP_OPT_HEX)))
					{
					    memset(confs->hexip,0,128);
						//value = xmlNodeListGetString(doc, tmp->xmlChildrenNode, 1);
						value=xmlNodeGetContent(tmp);
						strcpy(confs->hexip,(char *)value);	
						xmlFree(value);
					}
					//optype
					if ((!xmlStrcmp(tmp->name, BAD_CAST DHCP_OPTYPEZ)))
					{
					    memset(confs->optype,0,10);
						//value = xmlNodeListGetString(doc, tmp->xmlChildrenNode, 1);
						value=xmlNodeGetContent(tmp);
						strcpy(confs->optype,(char *)value);	
						xmlFree(value);
					}
					//lease
					else if ((!xmlStrcmp(tmp->name, BAD_CAST DHCP_LEASE_TIME)))
					{
					    memset(confs->lease,0,30);
						//value = xmlNodeListGetString(doc, tmp->xmlChildrenNode, 1);
						value=xmlNodeGetContent(tmp);
						strcpy(confs->lease,(char *)value);	
						xmlFree(value);
					}
					#if 1
					//gateway
					else if ((!xmlStrcmp(tmp->name, BAD_CAST DHCP_DEF_GW)))
					{
					    memset(confs->gateway,0,30);
						//value = xmlNodeListGetString(doc, tmp->xmlChildrenNode, 1);
						value=xmlNodeGetContent(tmp);
						strcpy(confs->gateway,(char *)value);	
						xmlFree(value);
					}
					#endif
					#if 1
					//dns server name
					if ((!xmlStrcmp(tmp->name, BAD_CAST DHCP_SERVER_N)))
					{
					    memset(confs->dnsname,0,30);
						//value = xmlNodeListGetString(doc, tmp->xmlChildrenNode, 1);
						value=xmlNodeGetContent(tmp);
						strcpy(confs->dnsname,(char *)value);	
						xmlFree(value);
					}
					#endif
					#if 1
					//dns server
					if ((!xmlStrcmp(tmp->name, BAD_CAST DHCP_SERVER)))
					{
					    memset(confs->dnserver,0,128);
						//value = xmlNodeListGetString(doc, tmp->xmlChildrenNode, 1);
						value=xmlNodeGetContent(tmp);
						strcpy(confs->dnserver,(char *)value);	
						xmlFree(value);
					}
					#endif
					//address pools
					if ((!xmlStrcmp(tmp->name, BAD_CAST DHCP_ADDR_POOL)))
					{
					    memset(confs->pools,0,8192);
						//value = xmlNodeListGetString(doc, tmp->xmlChildrenNode, 1);
						value=xmlNodeGetContent(tmp);
						strcpy(confs->pools,(char *)value);	
						xmlFree(value);
					}
					//ifactive
					if ((!xmlStrcmp(tmp->name, BAD_CAST DHCP_IFACTIVE)))
					{
					    memset(confs->ifactive,0,10);
						//value = xmlNodeListGetString(doc, tmp->xmlChildrenNode, 1);
						value=xmlNodeGetContent(tmp);
						strcpy(confs->ifactive,(char *)value);	
						xmlFree(value);
					}
					
					tmp = tmp->next;
				}
			}
			xmlFree(szAttr);
		}        
		pcurnode = pcurnode->next; 
	}	  
	xmlSaveFile(fpath,pdoc);  
	return 0;
}


//get node content ,for struct conf 
int get_dvsconf_struct(char * fpath,char * node_name,char *attribute,char *ruler,struct dvsconfz *confs)
{
	xmlDocPtr pdoc = NULL;

	xmlNodePtr pcurnode = NULL,tmp;
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

        tmp = pcurnode->xmlChildrenNode;
		if (!xmlStrcmp(pcurnode->name, BAD_CAST node_name))           
		{
		    propNodePtr = pcurnode;	

			xmlChar* szAttr = xmlGetProp(propNodePtr,BAD_CAST attribute);  

			if(!xmlStrcmp(szAttr,BAD_CAST ruler))  
			{	
                
				xmlChar *value;

				while(tmp !=NULL)
				{	 
					//netmask  1
					if ((!xmlStrcmp(tmp->name, BAD_CAST DHCPVS_TC_NETMASK)))
					{
					    memset(confs->snetmask,0,50);
						value=xmlNodeGetContent(tmp);
						strcpy(confs->snetmask,(char *)value);	
						xmlFree(value);
					}
					//sauth 2
					if ((!xmlStrcmp(tmp->name, BAD_CAST DHCPVS_TC_AUTH)))
					{
					    memset(confs->sauth,0,10);
						value=xmlNodeGetContent(tmp);
						strcpy(confs->sauth,(char *)value);	
						xmlFree(value);
					}
					//prefix 3
					if ((!xmlStrcmp(tmp->name, BAD_CAST DHCPVS_TC_PREFIX)))
					{
					    memset(confs->sprefix,0,10);
						value=xmlNodeGetContent(tmp);
						strcpy(confs->sprefix,(char *)value);	
						xmlFree(value);
					}
					//snameserver 4
					if ((!xmlStrcmp(tmp->name, BAD_CAST DHCPVS_TC_NSERVER)))
					{
					    memset(confs->snserver,0,50);
						value=xmlNodeGetContent(tmp);
						strcpy(confs->snserver,(char *)value);	
						xmlFree(value);
					}
					//search  5
					if ((!xmlStrcmp(tmp->name, BAD_CAST DHCPVS_TC_DSEARCH)))
					{
					    memset(confs->ssearch,0,50);
						value=xmlNodeGetContent(tmp);
						strcpy(confs->ssearch,(char *)value);	
						xmlFree(value);
					}
					//spool  6
					if ((!xmlStrcmp(tmp->name, BAD_CAST DHCPVS_TC_POOL)))
					{
					    memset(confs->spool,0,1024);
						value=xmlNodeGetContent(tmp);
						strcpy(confs->spool,(char *)value);	
						xmlFree(value);
					}
					//lease  7
					else if ((!xmlStrcmp(tmp->name, BAD_CAST DHCPVS_TC_LEASE)))
					{
					    memset(confs->slease,0,50);
						value=xmlNodeGetContent(tmp);
						strcpy(confs->slease,(char *)value);	
						xmlFree(value);
					}
					//subnet 8
					else if ((!xmlStrcmp(tmp->name, BAD_CAST DHCPVS_TC_SUBNET)))
					{
					    memset(confs->ssubnet,0,50);
						value=xmlNodeGetContent(tmp);
						strcpy(confs->ssubnet,(char *)value);	
						xmlFree(value);
					}		
					tmp = tmp->next;
				}
			}
			xmlFree(szAttr);
		}        
		pcurnode = pcurnode->next; 
	}	  
	xmlSaveFile(fpath,pdoc);  
	return 0;
}

/*delete one level xml node*/
int delete_dhcp_onelevel(char *fpath,char *node_name,char *attribute,char *ruler)
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
	return flag;
}

/*delete second level xml node*/
int delete_dhcp_seclevel(char *fpath,char *node_name,char *attribute,char *ruler,char *secnode)
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
				//此处来处理下一级的节点
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
	return 0;
}

/*read xml content ,save it as link list(test ok)*/
int string_link_list(struct substringz *head,int *subnum,char *source)
{
	struct substringz *q,*tail;
    int num=0;
	head->next=NULL;
	tail=head;

    if(source == NULL)
		return -2;
	
	char *temp=(char *)malloc(128);
	memset(temp,0,128);

	temp=strtok(source,"^");
	while(temp != NULL)
	{
	    num++;
		q=(struct substringz *)malloc(sizeof(struct substringz));
		if ( NULL == q)
		{
			return -1;
		}
		q->substr=(char *)malloc(128);

		strcpy(q->substr,temp);

		q->next=NULL;
		tail->next=q;
		tail=q;

        temp=strtok(NULL,"^");
	}
    *subnum=num;
	free(temp);
	
	return 0;
}

/*read xml content ,save it as link list(test ok)*/
int string_linksep_list(struct substringz *head,int *subnum,char *source,char *sep)
{
	struct substringz *q,*tail;
    int num=0;
	head->next=NULL;
	tail=head;

    if(source == NULL)
		return -2;
	
	char *temp=(char *)malloc(128);
	memset(temp,0,128);

	temp=strtok(source,sep);
	while(temp != NULL)
	{
	    num++;
		q=(struct substringz *)malloc(sizeof(struct substringz));
		if ( NULL == q)
		{
			return -1;
		}
		q->substr=(char *)malloc(128);

		strcpy(q->substr,temp);

		q->next=NULL;
		tail->next=q;
		tail=q;

        temp=strtok(NULL,sep);
	}
    *subnum=num;
	free(temp);
	
	return 0;
}

//free link list
void Free_substringz_all(struct substringz *head)
{
    struct substringz *f1,*f2;
	f1=head->next;		 
	f2=f1->next;
	while(f2!=NULL)
	{
	  free(f1->substr);
	  free(f1);
	  f1=f2;
	  f2=f2->next;
	}
	free(f1);
}

//free link list
void Free_bindipmac_info(struct bindipmac *head)
{
    struct bindipmac *f1,*f2;
	f1=head->next;		 
	f2=f1->next;
	while(f2!=NULL)
	{
	  free(f1->bindinfo);	  
	  free(f1);
	  f1=f2;
	  f2=f2->next;
	}
	free(f1);
}

//free link list
void Free_confinfo_info(struct confinfo *head)
{
    struct confinfo *f1,*f2;
	f1=head->next;		 
	f2=f1->next;
	while(f2!=NULL)
	{
	  free(f1->dnserver);
	  free(f1->pools);
	  free(f1->hexip);
	  free(f1->sip);
	  free(f1);
	  f1=f2;
	  f2=f2->next;
	}
	free(f1);
}


//free link list
void Free_dvsconfz_info(struct dvsconfz*head)
{
    struct dvsconfz *f1,*f2;
	f1=head->next;		 
	f2=f1->next;
	while(f2!=NULL)
	{	  
	  free(f1->spool);	 
	  free(f1);
	  f1=f2;
	  f2=f2->next;
	}
	free(f1);
}
//free link list
void Free_gbparam_info(struct gbparam *head)
{
    struct gbparam *f1,*f2;
	f1=head->next;		 
	f2=f1->next;
	while(f2!=NULL)
	{
	  free(f1);
	  f1=f2;
	  f2=f2->next;
	}
	free(f1);
}

//free link list
void Free_optionsix_info(struct optionsix *head)
{
    struct optionsix *f1,*f2;
	f1=head->next;		 
	f2=f1->next;
	while(f2!=NULL)
	{
	  free(f1);
	  f1=f2;
	  f2=f2->next;
	}
	free(f1);
}

int read_option_info(struct optionsix *ohead,int *optnum)
{
	xmlDocPtr doc;	 //定义解析文档指针
	xmlNodePtr cur,tmp; //定义结点指针

    int optflag=0;

	doc = xmlReadFile(DHCP_XML, "utf-8", 256); //解析文件

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


	struct optionsix *otail=NULL;
	ohead->next=NULL;
	otail=ohead;


	cur = cur->xmlChildrenNode;
	
	while(cur !=NULL)
	{

	    
		tmp = cur->xmlChildrenNode;
		/////////////  optionsix
		if (!xmlStrcmp(cur->name, BAD_CAST DHCP_OPT_NAME))           
		{
		        /////////////optionsixzero
		        struct optionsix *oq=NULL;
				oq=(struct optionsix *)malloc(sizeof(struct optionsix)+1);
				memset(oq,0,sizeof(struct optionsix)+1);
				if(NULL == oq)
				{
					return  -1;
				}
                optflag++;
				xmlChar *value;
				

				while(tmp !=NULL)
				{	 
				    
                    
					if ((!xmlStrcmp(tmp->name, BAD_CAST DHCP_OPT_IDEN)))
					{
					    memset(oq->iden,0,30);
						//value = xmlNodeListGetString(doc, tmp->xmlChildrenNode, 1);
						value=xmlNodeGetContent(tmp);
						strcpy(oq->iden,(char *)value);	
						xmlFree(value);
					}
					if ((!xmlStrcmp(tmp->name, BAD_CAST DHCP_OPT_USRN)))
					{
					    memset(oq->classname,0,30);
						//value = xmlNodeListGetString(doc, tmp->xmlChildrenNode, 1);
						value=xmlNodeGetContent(tmp);
						strcpy(oq->classname,(char *)value);	
						xmlFree(value);
					}
					if ((!xmlStrcmp(tmp->name, BAD_CAST DHCP_OPT_VENDER)))
					{
					    memset(oq->vendor,0,30);
						//value = xmlNodeListGetString(doc, tmp->xmlChildrenNode, 1);
						value=xmlNodeGetContent(tmp);
						strcpy(oq->vendor,(char *)value);	
						xmlFree(value);
					}
					if ((!xmlStrcmp(tmp->name, BAD_CAST DHCP_OPT_HEX)))
					{
					    memset(oq->opthex,0,30);
						//value = xmlNodeListGetString(doc, tmp->xmlChildrenNode, 1);
						value=xmlNodeGetContent(tmp);
						strcpy(oq->opthex,(char *)value);	
						xmlFree(value);
					}
					tmp = tmp->next;
				}
             oq->next = NULL;
		     otail->next = oq;
		     otail = oq;

		}        

		 
		 

		 
	     cur = cur->next;
	}
   *optnum=optflag;

   xmlFreeDoc(doc);

	return 0;
}
int get_show_xml_nodenum(char * fpath,char * node_name,int *nodenum)
{
	xmlDocPtr doc;	 
	xmlNodePtr cur; 
	int index_flag=0;
	
	doc = xmlReadFile(fpath, "utf-8", 256);
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
	
	cur = cur->xmlChildrenNode;
	while(cur !=NULL)
	{

		if (!xmlStrcmp(cur->name, BAD_CAST DHCP_NODE_CONF))           
		{
		     index_flag++;				            
		}  		
	     cur = cur->next;		 
	}
    *nodenum=index_flag;
	xmlFreeDoc(doc);
	return 0;
}

int get_show_pool_list(int indexz,struct snmp_single *ssingle)
{
	xmlDocPtr doc;	 
	xmlNodePtr cur,tmp; 
	int index_flag=0,designz=-1;
	int stflag=0;
	doc = xmlReadFile(DHCP_XML, "utf-8", 256);
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


	char *temp=(char *)malloc(8192);
	memset(temp,0,8192);
	
	cur = cur->xmlChildrenNode;
	while(cur !=NULL)
	{
	    tmp = cur->xmlChildrenNode;

		if (!xmlStrcmp(cur->name, BAD_CAST DHCP_NODE_CONF))           
		{
		       index_flag++;
		        /////////////conf informations
				xmlChar *value=NULL;				
				while(tmp !=NULL)
				{	 
					
					//address pools
				    if ((!xmlStrcmp(tmp->name, BAD_CAST DHCP_ADDR_POOL)))
					{
                        memset(ssingle->startz,0,50);
					    memset(ssingle->endz,0,50);						
					   	value=xmlNodeGetContent(tmp);
						/////////////////////////
						temp=strtok((char*)value,"^");
						while(temp != NULL)
						{
					    	
							if(indexz == index_flag)
							{
                                stflag++;
							    designz=0;	
								char *qpt = NULL;	
								qpt = strchr(temp,' ');
								if(stflag==1)
								{									
									strncpy(ssingle->startz, temp, qpt - temp );
								}	
								strcpy(ssingle->endz, qpt+1 );
							}
					        temp=strtok(NULL,"^");													
						}
						////////////////////////
						stflag=0;
						xmlFree(value);
					}	
					//subnet
				    else if ((!xmlStrcmp(tmp->name, BAD_CAST DHCP_SUBNET_Z)))
					{
					    memset(ssingle->subnetz,0,50);
						value=xmlNodeGetContent(tmp);						
						if(designz==0)
						{
						    designz=2;
							strcpy(ssingle->subnetz,value);
						}
						xmlFree(value);
					}
					if(designz==2)
						break;
					
					tmp = tmp->next;
				}            
		}  		
	     cur = cur->next;		 
	}
    free(temp);
	xmlFreeDoc(doc);
	return 0;
}
/*返回0且poolnumz>0时，调用Free_poollist_head()释放空间*/
int show_pool_list(SNMP_LIST *head,int * poolnumz)
{
	xmlDocPtr doc;	 
	xmlNodePtr cur,tmp; 
    int conflag=0;	
	int poolnum=0;
	doc = xmlReadFile(DHCP_XML, "utf-8", 256);
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
	SNMP_LIST *q,*tail;
	POOL_LIST *p_q,*p_tail;
	int i;	
	int stflag=0;
	
	char *temp=(char *)malloc(8192);
	memset(temp,0,8192);
	
	head->next = NULL;
	tail = head;

	cur = cur->xmlChildrenNode;
	while(cur !=NULL)
	{
        conflag++;
	    tmp = cur->xmlChildrenNode;
		if (!xmlStrcmp(cur->name, BAD_CAST DHCP_NODE_CONF))           
		{
		        /////////////conf informations
		        
				q=(SNMP_LIST *)malloc(sizeof(SNMP_LIST));				
			    if(NULL == q)
				{
					return  -1;
				}
				
				xmlChar *value=NULL;
				
				while(tmp !=NULL)
				{	 
	
					//gateway
				    if(!xmlStrcmp(tmp->name, BAD_CAST DHCP_DEF_GW))
					{
					    memset(q->gatewayz,0,50);
						value=xmlNodeGetContent(tmp);
						strcpy(q->gatewayz,(char *)value);	
						xmlFree(value);
					}			
					//dns
					else if(!xmlStrcmp(tmp->name, BAD_CAST DHCP_SERVER))
					{
					    memset(q->dnsz,0,256);
						value=xmlNodeGetContent(tmp);
						strcpy(q->dnsz,(char *)value);	
						xmlFree(value);
					}		
					//lease
					else if ((!xmlStrcmp(tmp->name, BAD_CAST DHCP_LEASE_TIME)))
					{
					    memset(q->leasez,0,50);
						value=xmlNodeGetContent(tmp);
						strcpy(q->leasez,(char *)value);	
						xmlFree(value);
					}		
					//lease timetick
					else if ((!xmlStrcmp(tmp->name, BAD_CAST DHCP_TIMETICK)))
					{
					    memset(q->timetickz,0,50);
						value=xmlNodeGetContent(tmp);
						strcpy(q->timetickz,(char *)value);	
						xmlFree(value);
					}		

					//netmask
					else if ((!xmlStrcmp(tmp->name, BAD_CAST DHCP_NET_MASK)))
					{
					    memset(q->netmask,0,50);
						value=xmlNodeGetContent(tmp);
						strcpy(q->netmask,(char *)value);	
						xmlFree(value);
					}	
					
					//address pools
					if ((!xmlStrcmp(tmp->name, BAD_CAST DHCP_ADDR_POOL)))
					{
					   	value=xmlNodeGetContent(tmp);
						/////////////////////////
						p_tail = &(q->pool);
						temp=strtok((char*)value,"^");
						while(temp != NULL)
						{
						    
						    poolnum++;
							stflag++;
							p_q = (POOL_LIST *)malloc(sizeof(POOL_LIST));	
							memset(p_q->startipz,0,50);
							memset(p_q->endipz,0,50);							
							
							char *qpt = NULL;	
							qpt = strchr(temp,' ');
							strncpy(p_q->startipz, temp, qpt - temp );
							strcpy(p_q->endipz, qpt+1 );
							if(stflag==1)
							{
							
								strcpy(q->stipz,p_q->startipz);
							}
								
							p_q->next = NULL;
							p_tail->next = p_q;
							p_tail = p_q;
					        temp=strtok(NULL,"^");
						}
						if((p_q != NULL)&&(p_q->next == NULL))
						{
							strcpy(q->eipz,p_q->endipz);
						}
						////////////////////////
						xmlFree(value);
						stflag=0;
					}		
					//subnet
					else if ((!xmlStrcmp(tmp->name, BAD_CAST DHCP_SUBNET_Z)))
					{
					    memset(q->subnetz,0,50);
						value=xmlNodeGetContent(tmp);
						strcpy(q->subnetz,(char *)value);	
						xmlFree(value);
					}	
					tmp = tmp->next;
				}
				q->snmp_num=conflag;
	            q->pool.pool_num=poolnum;
				q->next = NULL;
				tail->next = q;
				tail = q;
		}  		
	     cur = cur->next;
		 
	}
	
	*poolnumz=poolnum;
    free(temp);
	xmlFreeDoc(doc);
	return 0;
}


void Free_poollist_head(SNMP_LIST * head)
{
	SNMP_LIST *f1,*f2;
	POOL_LIST *pf1,*pf2;
	int i;
	f1=head->next;
	f2=f1->next;
	while(f2!=NULL)
	{
		pf1=f1->pool.next;
		if(pf1!=NULL)
		{
			pf2=pf1->next;
			while(pf2!=NULL)
			{
				free(pf1);
				pf1=pf2;
				pf2=pf2->next;
			}
			free(pf1);
		}		
		free(f1);
		f1=f2;
		f2=f2->next;
	}
	pf1=f1->pool.next;
	if(pf1!=NULL)
	{
		pf2=pf1->next;
		while(pf2!=NULL)
		{
			free(pf1);
			pf1=pf2;
			pf2=pf2->next;
		}
		free(pf1);
	}
	free(f1);
}

//read dhcpv6 
int read_dvsconf_xml(char *fpath,struct dvsconfz *chead,int *confnum)
{
	xmlDocPtr doc;	 
	xmlNodePtr cur,tmp; 
    int conflag=0;	
	doc = xmlReadFile(fpath, "utf-8", 256); 
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

	struct dvsconfz *ctail=NULL;
	chead->next=NULL;
	ctail=chead;

	cur = cur->xmlChildrenNode;
	
	while(cur !=NULL)
	{

	    tmp = cur->xmlChildrenNode;
		if (!xmlStrcmp(cur->name, BAD_CAST DHCPVS_SC_CONF))           
		{
		        /////////////conf informations
		        struct dvsconfz  *cq=NULL;
				cq=(struct dvsconfz *)malloc(sizeof(struct dvsconfz)+1);
				memset(cq,0,sizeof(struct dvsconfz)+1);
				if(NULL == cq)
				{
					return  -1;
				}				
				
				cq->spool=(char *)malloc(8192);
				memset(cq->spool,0,8192);
				
				/////////////  
                conflag++;
				xmlChar *value=NULL;
				
				while(tmp !=NULL)
				{	 
					//name server  1
					if ((!xmlStrcmp(tmp->name, BAD_CAST DHCPVS_TC_NSERVER)))
					{
					    memset(cq->snserver,0,50);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->snserver,(char *)value);	
						xmlFree(value);
					}
					
					//lease  2
					if ((!xmlStrcmp(tmp->name, BAD_CAST DHCPVS_TC_LEASE)))
					{
					    memset(cq->slease,0,50);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->slease,(char *)value);	
						xmlFree(value);
					}

					//subnet  3
					if ((!xmlStrcmp(tmp->name, BAD_CAST DHCPVS_TC_SUBNET)))
					{
					    memset(cq->ssubnet,0,50);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->ssubnet,(char *)value);	
						xmlFree(value);
					}

				     //auth 4
					if ((!xmlStrcmp(tmp->name, BAD_CAST DHCPVS_TC_AUTH)))
					{
					    memset(cq->sauth,0,10);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->sauth,(char *)value);	
						xmlFree(value);
					}
					//prefix 5
					if ((!xmlStrcmp(tmp->name, BAD_CAST DHCPVS_TC_PREFIX)))
					{
					    memset(cq->sprefix,0,10);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->sprefix,(char *)value);	
						xmlFree(value);
					}
					
					//search 6
					else if ((!xmlStrcmp(tmp->name, BAD_CAST DHCPVS_TC_DSEARCH)))
					{
					    memset(cq->ssearch,0,50);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->ssearch,(char *)value);	
						xmlFree(value);
					}									
					//address pools 7
					if ((!xmlStrcmp(tmp->name, BAD_CAST DHCPVS_TC_POOL)))
					{
					    memset(cq->spool,0,8192);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->spool,(char *)value);	
						xmlFree(value);
					}			
					//netmask 8
					if ((!xmlStrcmp(tmp->name, BAD_CAST DHCPVS_TC_NETMASK)))
					{
					    memset(cq->snetmask,0,50);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->snetmask,(char *)value);	
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

//write conf dhcpv6
int write_dhcpvs_conf(char *fpath,struct dvsconfz *c_head)
{
    FILE * fp;	
    if( NULL == fpath)
		return -1 ;  
	
	if(( fp = fopen(fpath,"w+"))==NULL)
		return -2;
	
    
	char *File_content=(char *)malloc(50000);
	char *content=(char *)malloc(1024);

	char firstp[30];
	memset(firstp,0,30);
	
	char secondp[30];
	memset(secondp,0,30);
	
	memset(File_content,0,50000);
	memset(content,0,1024);

	struct substringz s_head,*sq;
    int subnum=0,retflag=-1;

    struct dvsconfz *cq;
	cq=c_head->next;

	 while(cq !=NULL)
	{
      //subnet and netmask
      memset(content,0,1024);
	  if((strcmp(cq->ssubnet,"") !=0) &&(strcmp(cq->snetmask,"") !=0))
	  sprintf(content , "subnet %s/%s\n {\n",cq->ssubnet,cq->snetmask);   
	  strcat(File_content,content);	

	  //auth
	  memset(content,0,1024);
	  if(strcmp(cq->sauth,"") !=0)
	  {
	    if(strcmp(cq->sauth,"yes") ==0)
	    sprintf(content , "authoritative ;\n");
	  }
	  strcat(File_content,content);	

	  //name server
	  memset(content,0,1024);		
	  if(strcmp(cq->snserver,"") !=0)
	  sprintf(content , "option dhcp6.name-servers %s ;\n",cq->snserver);	
	  strcat(File_content,content);	

      //domain search
	  memset(content,0,1024);
	  if(strcmp(cq->ssearch,"") !=0)
	  sprintf(content , "option dhcp6.domain-search \"%s\" ;\n",cq->ssearch);   
	  strcat(File_content,content);	

      //leases
	  memset(content,0,1024);
	  if(strcmp(cq->slease,"") !=0)
	  sprintf(content , "default-lease-time %s ;\n",cq->slease);   
	  strcat(File_content,content);	

     //max lease time
	  memset(content,0,1024);
	  if(strcmp(cq->slease,"") !=0)
	  sprintf(content , "max-lease-time %s ;\n",cq->slease);   
	  strcat(File_content,content);	

	  retflag=string_link_list(&s_head, &subnum, cq->spool);  
	  if(retflag==0)
	  {
	      sq=s_head.next;
	   
	       while(sq != NULL)
		   {
		      memset(content,0,1024);		 
		      if(strcmp(sq->substr,"") !=0)
			  sprintf(content,"range %s ;\n",sq->substr);
		      strcat(File_content,content);	
			  
			  sq=sq->next;
		   }
	  }

	  //prefix
	  memset(content,0,1024);
	  if(strcmp(cq->sprefix,"") !=0)
	  {
	    if(strcmp(cq->sprefix,"yes") ==0)
	    sprintf(content , "prefix %s %s /%s ;\n",cq->ssubnet,cq->ssubnet,cq->snetmask);
	  }
	  strcat(File_content,content);	
	  
	  strcat(File_content,"}\n");

	  if((retflag==0 )&& (subnum > 0))
	     Free_substringz_all(&s_head);
	  
	  cq=cq->next;
	}

	fwrite(File_content,strlen(File_content),1,fp);	
	fclose(fp);

    free(File_content);
	free(content);

	return 0;
}

//read dhcpv4
int read_confinfo_xml(struct confinfo *chead,int *confnum)
{
	xmlDocPtr doc;	 //定义解析文档指针
	xmlNodePtr cur,tmp; //定义结点指针

    int conflag=0;

	
	doc = xmlReadFile(DHCP_XML, "utf-8", 256); //解析文件

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

	struct confinfo *ctail=NULL;
	chead->next=NULL;
	ctail=chead;

	cur = cur->xmlChildrenNode;
	
	while(cur !=NULL)
	{

	    tmp = cur->xmlChildrenNode;
		if (!xmlStrcmp(cur->name, BAD_CAST DHCP_NODE_CONF))           
		{
		        /////////////conf informations
		        struct confinfo  *cq=NULL;
				cq=(struct confinfo *)malloc(sizeof(struct confinfo)+1);
				memset(cq,0,sizeof(struct confinfo)+1);
				if(NULL == cq)
				{
					return  -1;
				}
				
				cq->dnserver=(char *)malloc(128);
				memset(cq->dnserver,0,128);
				
				cq->pools=(char *)malloc(8192);
				memset(cq->pools,0,8192);

                cq->hexip=(char *)malloc(128);
				memset(cq->hexip,0,128);

				cq->sip=(char *)malloc(128);
				memset(cq->sip,0,128);
                
				
				/////////////  
                conflag++;
				xmlChar *value=NULL;
				
				while(tmp !=NULL)
				{	 
					//netmask
					if ((!xmlStrcmp(tmp->name, BAD_CAST DHCP_NET_MASK)))
					{
					    memset(cq->netmask,0,30);
						//value = xmlNodeListGetString(doc, tmp->xmlChildrenNode, 1);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->netmask,(char *)value);	
						xmlFree(value);
					}
					
					//  sourceip
					if ((!xmlStrcmp(tmp->name, BAD_CAST DHCP_OPT_VENDER)))
					{
					    memset(cq->sip,0,128);
						//value = xmlNodeListGetString(doc, tmp->xmlChildrenNode, 1);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->sip,(char *)value);	
						xmlFree(value);
					}

					//hexip
					if ((!xmlStrcmp(tmp->name, BAD_CAST DHCP_OPT_HEX)))
					{
					    memset(cq->hexip,0,128);
						//value = xmlNodeListGetString(doc, tmp->xmlChildrenNode, 1);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->hexip,(char *)value);	
						xmlFree(value);
					}

				     //optype
					if ((!xmlStrcmp(tmp->name, BAD_CAST DHCP_OPTYPEZ)))
					{
					    memset(cq->optype,0,10);
						//value = xmlNodeListGetString(doc, tmp->xmlChildrenNode, 1);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->optype,(char *)value);	
						xmlFree(value);
					}
					//subnet
					if ((!xmlStrcmp(tmp->name, BAD_CAST DHCP_SUBNET_Z)))
					{
					    memset(cq->subnetz,0,30);
						//value = xmlNodeListGetString(doc, tmp->xmlChildrenNode, 1);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->subnetz,(char *)value);	
						xmlFree(value);
					}
					
					//lease
					else if ((!xmlStrcmp(tmp->name, BAD_CAST DHCP_LEASE_TIME)))
					{
					    memset(cq->lease,0,30);
						//value = xmlNodeListGetString(doc, tmp->xmlChildrenNode, 1);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->lease,(char *)value);	
						xmlFree(value);
					}					
					//gateway
					else if ((!xmlStrcmp(tmp->name, BAD_CAST DHCP_DEF_GW)))
					{
					    memset(cq->gateway,0,30);
						//value = xmlNodeListGetString(doc, tmp->xmlChildrenNode, 1);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->gateway,(char *)value);	
						xmlFree(value);
					}					
					//dns server name
					if ((!xmlStrcmp(tmp->name, BAD_CAST DHCP_SERVER_N)))
					{
					    memset(cq->dnsname,0,30);
						//value = xmlNodeListGetString(doc, tmp->xmlChildrenNode, 1);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->dnsname,(char *)value);	
						xmlFree(value);
					}
					//dns server
					if ((!xmlStrcmp(tmp->name, BAD_CAST DHCP_SERVER)))
					{
					    memset(cq->dnserver,0,128);
						//value = xmlNodeListGetString(doc, tmp->xmlChildrenNode, 1);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->dnserver,(char *)value);	
						xmlFree(value);
					}
					
					//address pools
					if ((!xmlStrcmp(tmp->name, BAD_CAST DHCP_ADDR_POOL)))
					{
					    memset(cq->pools,0,8192);
						//value = xmlNodeListGetString(doc, tmp->xmlChildrenNode, 1);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->pools,(char *)value);	
						xmlFree(value);
					}
					//ifactive  
					if ((!xmlStrcmp(tmp->name, BAD_CAST DHCP_IFACTIVE)))
					{
					    memset(cq->ifactive,0,10);
						//value = xmlNodeListGetString(doc, tmp->xmlChildrenNode, 1);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->ifactive,(char *)value);	
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

//read xml informastion
int read_bindinfo_xml(struct bindipmac *bhead,int *bnum)
{
	xmlDocPtr doc;	 //定义解析文档指针
	xmlNodePtr cur,tmp; //定义结点指针

    int bindflag=0;

	
	doc = xmlReadFile(DHCP_XML, "utf-8", 256); //解析文件

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


	struct bindipmac *btail=NULL;
	bhead->next=NULL;
	btail=bhead;

	

	cur = cur->xmlChildrenNode;
	
	while(cur !=NULL)
	{

	   
		
	    tmp = cur->xmlChildrenNode;
		#if 1
	    /////////////  bindipmac
		if (!xmlStrcmp(cur->name, BAD_CAST DHCP_BIND_NODE))           
		{
		         /////////////bind ip and mac
			    struct bindipmac *bq=NULL;
				bq=(struct bindipmac *)malloc(sizeof(struct bindipmac)+1);
				memset(bq,0,sizeof(struct bindipmac)+1);
				if(NULL == bq)
				{
					return  -1;
				}
				bq->bindinfo=(char *)malloc(512);
				memset(bq->bindinfo,0,512);
                bindflag++;
				xmlChar *value;
				

				while(tmp !=NULL)
				{	 
				   
                    
					if ((!xmlStrcmp(tmp->name, BAD_CAST DHCP_BIND_IPMAC)))
					{
					    memset(bq->bindinfo,0,512);
						//value = xmlNodeListGetString(doc, tmp->xmlChildrenNode, 1);
						value=xmlNodeGetContent(tmp);
						strcpy(bq->bindinfo,(char *)value);	
						xmlFree(value);
					}
					tmp = tmp->next;
				}
            
				 bq->next = NULL;
			     btail->next = bq;
			     btail = bq;
		}     
		#endif

         

	     cur = cur->next;
	}

	*bnum=bindflag;
	xmlFreeDoc(doc);

	return 0;
}


//read xml informastion
int read_gbparam_xml(struct gbparam *gbhead,int *gbnum)
{
	xmlDocPtr doc;	 //定义解析文档指针
	xmlNodePtr cur,tmp; //定义结点指针

    int gbflag=0;

	
	doc = xmlReadFile(DHCP_XML, "utf-8", 256); //解析文件

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


	struct gbparam *gbtail=NULL;
	gbhead->next=NULL;
	gbtail=gbhead;

	

	cur = cur->xmlChildrenNode;
	
	while(cur !=NULL)
	{

	   
		
	    tmp = cur->xmlChildrenNode;
		#if 1
	    /////////////  pingcheck
		if (!xmlStrcmp(cur->name, BAD_CAST DHCP_GB_PARAM))           
		{
		         /////////////ping 
			    struct gbparam *gb=NULL;
				gb=(struct gbparam *)malloc(sizeof(struct gbparam)+1);
				memset(gb,0,sizeof(struct gbparam)+1);
				if(NULL == gb)
				{
					return  -1;
				}
				
                gbflag++;
				xmlChar *value;
				
				while(tmp !=NULL)
				{	 
					if ((!xmlStrcmp(tmp->name, BAD_CAST DHCP_GB_PINGC)))
					{
					    memset(gb->pingcheck,0,10);
						//value = xmlNodeListGetString(doc, tmp->xmlChildrenNode, 1);
						value=xmlNodeGetContent(tmp);
						strcpy(gb->pingcheck,(char *)value);	
						xmlFree(value);
					}					
					tmp = tmp->next;
				}
            
				 gb->next = NULL;
			     gbtail->next = gb;
			     gbtail = gb;
		}     
		#endif

         

	     cur = cur->next;
	}

	*gbnum=gbflag;
	xmlFreeDoc(doc);

	return 0;
}

//write conf
int write_dhcp_conf(char *fpath,struct optionsix *o_head,struct confinfo *c_head,struct bindipmac *b_head,struct gbparam *g_head)
{
    FILE * fp;	
	char *File_content=(char *)malloc(50000);
	char *content=(char *)malloc(1024);

    char *p;
	char firstp[30];
	memset(firstp,0,30);
	
	char secondp[30];
	memset(secondp,0,30);
	
	memset(File_content,0,50000);
	memset(content,0,1024);

	struct substringz s_head,*sq;
    int subnum=0,retflag=-1;

	struct substringz d_head,*dq;
    int dsubnum=0,dretflag=-1;

   if( NULL == fpath)
	{
		free(File_content);
		free(content);
		return -1 ;  
	}
	if(( fp = fopen(fpath,"w+"))==NULL){
		free(File_content);
		free(content);
		return -2;
	}

	struct bindipmac *bq;
    int bnum=0;
  
    struct optionsix *oq;
  
    struct confinfo *cq;

	struct gbparam *gq;

	bq=b_head->next;
	oq=o_head->next;
	cq=c_head->next;
	gq=g_head->next;

	 while(gq !=NULL)
	{
	  memset(content,0,1024);
	  if(strcmp(gq->pingcheck,"") !=0)
	  {
	    sprintf(content , "ping-check %s;\n",gq->pingcheck);
	  }
	  strcat(File_content,content);	
	  
	  gq=gq->next;
	}
	 
     while(bq !=NULL)
	{
	  
	  /////////////////////

      dretflag=string_link_list(&d_head, &dsubnum, bq->bindinfo);  
	  if(dretflag==0)
	  {
	      dq=d_head.next;
	   
	       while(dq != NULL)
		   {
		      bnum++;
			  if(strcmp(dq->substr,"") !=0)
			  {
				p=strtok(dq->substr,"-");
				memset(firstp,0,30);
				strcpy(firstp,p);
				p=strtok(NULL,"-");
				memset(secondp,0,30);
				strcpy(secondp,p);

				memset(content,0,1024);
				sprintf(content , "host %d \n { \n hardware ethernet %s;\n",bnum,secondp);		
				strcat(File_content,content);	

				memset(content,0,1024);
				sprintf(content , "fixed-address %s;\n }\n",firstp);		
				strcat(File_content,content);	
			  }
			  dq=dq->next;
		   }

	  }
	  //////////////////////
	  if((dretflag==0 )&& (dsubnum > 0))
	     Free_substringz_all(&d_head);
	  
	  bq=bq->next;
	}

    int optflag;
	 while(cq !=NULL)
	{
  
      memset(content,0,1024);
	  if((strcmp(cq->subnetz,"") !=0) &&(strcmp(cq->netmask,"") !=0))
	  sprintf(content , "subnet %s netmask %s\n {\n",cq->subnetz,cq->netmask);   
	  strcat(File_content,content);	
	  
	  memset(content,0,1024);
	  if(strcmp(cq->gateway,"") !=0)
	  sprintf(content , "option routers %s ;\n",cq->gateway);
	  strcat(File_content,content);	

	  memset(content,0,1024);
	  if(strcmp(cq->ifactive,"") !=0)
	  {
	    if(strcmp(cq->ifactive,"yes") ==0)
	    sprintf(content , "authoritative ;\n");
	  }
	  strcat(File_content,content);	
	  
	  memset(content,0,1024);		
	  if(strcmp(cq->dnserver,"") !=0)
	  sprintf(content , "option domain-name-servers %s ;\n",cq->dnserver);	
	  strcat(File_content,content);	

	  memset(content,0,1024);
	  if(strcmp(cq->dnsname,"") !=0)
	  sprintf(content , "option domain-name \"%s\" ;\n",cq->dnsname);   
	  strcat(File_content,content);	

	  memset(content,0,1024);
	  if(strcmp(cq->lease,"") !=0)
	  sprintf(content , "default-lease-time %s ;\n",cq->lease);   
	  strcat(File_content,content);	

	  memset(content,0,1024);
	  if(strcmp(cq->lease,"") !=0)
	  sprintf(content , "max-lease-time %s ;\n",cq->lease);   
	  strcat(File_content,content);	
  

	  memset(content,0,1024);
	  if((strcmp(cq->hexip,"") !=0) && (strcmp(cq->optype,"") !=0))
	  {
		optflag=strtoul(cq->optype,0,10);
	    sprintf(content , "option vendor-encapsulated-options  %X:%s ;\n",optflag,cq->hexip);
	  }
	  strcat(File_content,content);	


	  retflag=string_link_list(&s_head, &subnum, cq->pools);  
	  if(retflag==0)
	  {
	      sq=s_head.next;
	   
	       while(sq != NULL)
		   {
		      memset(content,0,1024);		 
		      if(strcmp(sq->substr,"") !=0)
			  sprintf(content,"pool \n  { \n  range %s ;\n }\n",sq->substr);
		      strcat(File_content,content);	
			  
			  sq=sq->next;
		   }
	  }
	  
	  strcat(File_content,"}\n");

	  if((retflag==0 )&& (subnum > 0))
	     Free_substringz_all(&s_head);
	  
	  cq=cq->next;
	}

	
	while(oq !=NULL)
	{
	  memset(content,0,1024);		
	  if(strcmp(oq->classname,"") !=0)
	  sprintf(content , "class \"%s\" \n {\n",oq->classname);	
	  strcat(File_content,content);	

	  memset(content,0,1024);	
	  if(strcmp(oq->iden,"") !=0)
	  sprintf(content , "match if option vendor-class-identifier = \"%s\";\n",oq->iden);	
	  strcat(File_content,content);	

	  memset(content,0,1024);	
	  if(strcmp(oq->opthex,"") !=0)
	  sprintf(content , "option vendor-encapsulated-options  11:04:%s ;\n}\n",oq->opthex);	
	  strcat(File_content,content);	
	  
	  oq=oq->next;
	}

	

	fwrite(File_content,strlen(File_content),1,fp);	
	fclose(fp);

    free(File_content);
	free(content);

	return 0;
}

int save_dhcp_lease()
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;
	unsigned int profile = 0;
	int detect = 0;
	
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_SAVE_DHCP_LEASE);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &profile,	
							 DBUS_TYPE_UINT32, &detect,					 
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block(ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
		//	printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return 0;
	}
	if (dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, &op_ret,
		DBUS_TYPE_INVALID)) {
        if(op_ret){
          //  vty_out(vty,"saved dhcp lease success\n");
		}
	} 
	else {		
		if (dbus_error_is_set(&err)) {
		//	printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	
	return 0;
}


int get_pool_info()
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;
	unsigned int profile = 0;
	int detect = 0;

	
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_GET_POOL_INFO);

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &profile,	
							 DBUS_TYPE_UINT32, &detect,					 
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block(ccgi_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
		//	printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return 0;
	}
	if (dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, &op_ret,
		DBUS_TYPE_INVALID)) {
        if(op_ret){
          //  vty_out(vty,"saved dhcp lease success\n");
		}
	} 
	else {		
		if (dbus_error_is_set(&err)) {
		//	printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return 0;
}

void get_sub_net(char *soureip,char *maskbit,char *subz) 
{
  int i;
  char *tmp;
  char sip1[4];
  char sip2[4];
  char sip3[4];
  char sip4[4];
  memset(sip1,0,4);
  memset(sip2,0,4);
  memset(sip3,0,4);
  memset(sip4,0,4);

  char mk1[4];
  char mk2[4];
  char mk3[4];
  char mk4[4];
  memset(mk1,0,4);
  memset(mk2,0,4);
  memset(mk3,0,4);
  memset(mk4,0,4);
  int s1,s2,s3,s4,m1,m2,m3,m4,b1,b2,b3,b4;
  char *temp=(char *)malloc(50);
  memset(temp,0,50);
  char *start_ip=(char *)malloc(50);
  memset(start_ip,0,50);
  strcpy(start_ip,soureip);
  char *netmask=(char *)malloc(50);
  memset(netmask,0,50);
  strcpy(netmask,maskbit);
  i = 0;
  tmp = strtok(start_ip,".");
  while(tmp != NULL)
   {
   i++;
if(i==1)
strcpy(sip1,tmp);
else if(i ==2 )
strcpy(sip2,tmp);
else if(i==3)
strcpy(sip3,tmp);
else if(i==4)
strcpy(sip4,tmp);
tmp = strtok(NULL,"."); 
   }
 i = 0;
  tmp = strtok(netmask,".");
  while(tmp != NULL)
   {
   i++;
if(i==1)
strcpy(mk1,tmp);
else if(i ==2 )
strcpy(mk2,tmp);
else if(i==3)
strcpy(mk3,tmp);
else if(i==4)
strcpy(mk4,tmp);
tmp = strtok(NULL,"."); 
   }
  s1=strtoul(sip1,0,10);
  s2=strtoul(sip2,0,10);
  s3=strtoul(sip3,0,10);
  s4=strtoul(sip4,0,10);
  m1=strtoul(mk1,0,10);
  m2=strtoul(mk2,0,10);
  m3=strtoul(mk3,0,10);
  m4=strtoul(mk4,0,10);
  b1= s1 & m1;
  b2= s2 & m2;
  b3= s3 & m3;
  b4= s4 & m4;
  memset(temp,0,50);
  sprintf(temp,"%d.%d.%d.%d",b1,b2,b3,b4);
  strcpy(subz,temp);
  free(temp);
  free(start_ip);
  free(netmask);
}
int mod_dnsz_service(char *subnetz,char *newdns,int indexz)
{
	struct substringz s_head,*sq;
	int subnum=0,count=0,retflag=-1;

	char *pingc=(char *)malloc(256);	
	char *newc=(char *)malloc(256);
	memset(pingc,0,256);
	get_dhcp_node_attr(DHCP_XML, DHCP_NODE_CONF, DHCP_NODE_ATTR, subnetz, DHCP_SERVER, pingc);
	string_linksep_list(&s_head, &subnum,pingc,","); 
	sq=s_head.next;	
	memset(newc,0,256);
	while(sq != NULL)
	{
		count++;
		if(count==indexz)
		{
			strcat(newc,newdns);
		}
		else
		{
			strcat(newc,sq->substr);
		}
		if((strcmp(newc,"")!=0)&&(subnum!=count))
			strcat(newc,",");
		sq=sq->next;				
	}
	if((retflag==0 )&& (subnum > 0))
		Free_substringz_all(&s_head);	
	if((count==0)&&(strcmp(newdns,"")!=0))
	{
		
		strcat(newc,newdns);
	}
	if((count==1)&&(strcmp(newdns,"")!=0)&&(indexz==2))
	{		
		strcat(newc,",");
		strcat(newc,newdns);
	}
	mod_dhcp_node(DHCP_XML, DHCP_NODE_CONF, DHCP_NODE_ATTR, subnetz,DHCP_SERVER, newc);
	free(pingc);
	free(newc);
	return 0;
}
//read
int save_dhcpconf()
{
	int bflag=-1,cflag=-1,oflag=-1,gflag=-1,ret;
	struct bindipmac b_head;
	int bnum=0;
	struct optionsix o_head;
	int onum=0;
	struct confinfo c_head;
	int cnum=0;
	struct gbparam g_head;
	int gnum=0;
	bflag=read_bindinfo_xml(&b_head, &bnum);
	oflag=read_option_info(&o_head, &onum);
	cflag=read_confinfo_xml(&c_head, &cnum);
	gflag=read_gbparam_xml(&g_head,&gnum);

	if((bflag==0)&&(oflag==0)&&(cflag==0)&&(gflag==0))
	{
		ret=write_dhcp_conf(DHCP_CONF_PATH, &o_head, &c_head, &b_head,&g_head);
	}
	if((bflag==0 )&& (bnum > 0))
	Free_bindipmac_info(&b_head);

	if((cflag==0 )&& (cnum > 0))
	Free_confinfo_info(&c_head);

	if((oflag==0 )&& (onum > 0))
	Free_optionsix_info(&o_head);

	if((gflag==0 )&& (gnum > 0))
	Free_gbparam_info(&g_head);
	
	return ret;

}


void Free_dhcpleasestat_info(struct dhcpleasestat *head)
{
    struct dhcpleasestat *f1,*f2;
	f1=head->next;	
	if(f1 == NULL)
	{
		return;
	}		
	f2=f1->next;
	while(f2!=NULL)
	{
	  free(f1);
	  f1=f2;
	  f2=f2->next;
	}
	free(f1);
}
int get_dhcp_pool_info(struct dhcpleasestat *head)
{
	struct dhcpleasestat *q,*tail;
	int num=0;
	head->next=NULL;
	tail=head;

	FILE *fp ;
	
	char temp[128];
	memset(temp,0,128);
	char zstring[128];
	memset(zstring,0,128);
	char *p = NULL;
	int i = 0;
	int dlcount = 0;
	int dlfree = 0;
	int dluse = 0;
	int dreqnum = 0;
	int drespnum = 0;
	unsigned long  discovery_times = 0;
	unsigned long  offer_times = 0;
	
	fp = fopen("/var/run/apache2/dhcp_leasestat","r");
	if(fp==NULL)
	{
		return -1;
	}
	
	while((fgets(temp,128,fp)) != NULL)
	{
		num++;
		q=(struct dhcpleasestat *)malloc(sizeof(struct dhcpleasestat));		
		if ( NULL == q)
		{
			fclose(fp);
			return -1;
		}
		i = 0;
		memset(zstring,0,sizeof(zstring));
		strcpy(zstring,temp);
		

		dlcount = 0;
		dlfree = 0;
		dluse = 0;
		dreqnum = 0;
		drespnum = 0;
		discovery_times = 0;
		offer_times = 0;
		
		sscanf(zstring, "%29s %d %d %d %d %d %d", q->dlsubnet, &dlcount, &dlfree ,&dreqnum,&drespnum,&discovery_times,&offer_times);//29 = sizeof(q->dlsubnet)-1

		if( 0 == dlcount )
		{
			dluse = 0;
		}
		else
		{
			dluse = (dlcount - dlfree)*100/dlcount;
		}
		memset(q->dluseage, 0 , DHCPCOMLEN);
		sprintf(q->dluseage,"%d",dluse);
		q->dhcpreqnum = dreqnum;
		q->dhcpresponse = drespnum;
	    q->discovery_num = discovery_times;
        q->offer_num = offer_times;
		
		q->next=NULL;
		tail->next=q;
		tail=q;
	}
	fclose(fp);

	return 0;
}
