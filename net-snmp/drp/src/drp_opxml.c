/*****************************************************************************
  Copyright (C) Autelan Technology

  This software file is owned and distributed by Autelan Technology 
 ******************************************************************************

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
 ****************************************************************************
 *
 *
 * CREATOR:
 * autelan.software.xxx. team
 *
 * DESCRIPTION:
 * xxx module main routine
 *
 *
 ***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpathInternals.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dbus/dbus.h>
#include <signal.h>
#include <time.h>

#include "drp_def.h"
#include "drp_log.h"
#include "drp_mem.h"
#include "drp_opxml.h"

//free link list
void free_read_domain_xml(struct domain_st *head,int *free_flag)
{
	struct domain_st *f1,*f2;
	f1=head->next;		 
	f2=f1->next;
	*free_flag=0;
	while(f2!=NULL)
	{
		(*free_flag)++;
		drp_free(f1->domain_ip);
		drp_free(f1->domain_name);
		drp_free(f1);
		f1=f2;
		f2=f2->next;
	}
	drp_free(f1);
	head=NULL;
}

int check_and_get_domainname_xml(char * filepath,struct domain_st *chead,
		char * second_node_name,char *second_node_attribute,int *domain_num)
{
	xmlDocPtr doc;	 
	xmlNodePtr cur,tmp;
	int conflag=0;	
	char *psfilename;
	psfilename = filepath;
	doc = xmlReadFile(psfilename, "utf-8", 256); 
	if (doc == NULL ) 
	{
		xmlFreeDoc(doc);
		drp_log_err("check_and_get_domainname_xml file %s not exist !",psfilename);
		return DRP_ERR_OPXML_FILE_NOT_EXIST;
	}
	cur = xmlDocGetRootElement(doc); 
	if (cur == NULL)
	{
		xmlFreeDoc(doc);	
		drp_log_err("check_and_get_domainname_xml file %s xmlDocGetRootElement error !",psfilename);
		return DRP_ERR_OPXML_FILE_FORMAT_WRONG;
	}

	if (xmlStrcmp(cur->name, (const xmlChar *) "root")) 
	{
		xmlFreeDoc(doc);
		drp_log_err("check_and_get_domainname_xml file %s format is wrong !",psfilename);
		return DRP_ERR_OPXML_FILE_FORMAT_WRONG;
	}

	//printf("before chead =%p\n",chead);
	struct domain_st *ctail=NULL;
	chead->next=NULL;
	ctail=chead;

	//printf("after chead =%p\n",chead);
	cur = cur->xmlChildrenNode; 
	while(cur !=NULL)
	{			 
		//printf("get the second node name is %s\n",second_node_name);
		if (!xmlStrcmp(cur->name, BAD_CAST second_node_name))			 /*find the node "http" or "https" node in xml file*/ 
		{							
			xmlChar *value;
			value = xmlGetProp(cur, BAD_CAST "attribute");	/*get the attribute of second node"domain"*/
			if(value != NULL)
			{	
				//printf("get the second node attr(domain name)  is %s\n",value);
				if(strcmp(second_node_attribute,(char*)value)==0)
				{
#if 0
					conflag++; 
					struct domain_st  *cq;
					//printf("aaa conflag=%d\n",conflag);
					cq=(struct domain_st *)malloc(sizeof(struct domain_st)+1);
					memset(cq,0,sizeof(struct domain_st)+1);
					if(NULL == cq)
					{
						xmlFree(value);
						return	-1; 					
					}

					//printf("every  new cq=%p\n",cq);

					cq->domain_ip=(char *)malloc(50);			
					memset(cq->domain_ip,0,50); 
					cq->domain_name=(char *)malloc(128);			
					memset(cq->domain_name,0,128); 
					strcpy(cq->domain_name,second_node_attribute);
					//printf("get the cq->domain_name is %s\n",cq->domain_name);
#endif					
					xmlChar *value_ip=NULL; 
					tmp = cur->xmlChildrenNode;
					while(tmp !=NULL)
					{	 
						conflag++; 
						struct domain_st  *cq;
						//printf("aaa conflag=%d\n",conflag);
						cq=(struct domain_st *)malloc(sizeof(struct domain_st)+1);
						memset(cq,0,sizeof(struct domain_st)+1);
						if(NULL == cq)
						{
							xmlFree(value);
							drp_log_err("check_and_get_domainname_xml malloc error !");
							return DRP_ERR_MALLOC_FAILED; 					
						}

						//printf("every  new cq=%p\n",cq);

						cq->domain_ip=(char *)malloc(50);			
						memset(cq->domain_ip,0,50); 
						cq->domain_name=(char *)malloc(128);			
						memset(cq->domain_name,0,128); 
						strcpy(cq->domain_name,second_node_attribute);
						//printf("get the cq->domain_name is %s\n",cq->domain_name);
						//get	 IP  address corresponding to the domain name,don't need to compare the node name					
						{
							memset(cq->domain_ip,0,50);
							value_ip=xmlNodeGetContent(tmp);
							strcpy(cq->domain_ip,(char *)value_ip); 
							//printf("get the cq->domain_ip is %s\n",cq->domain_ip);		
							xmlFree(value_ip);
						}	
						cq->ip_index=conflag;
						//printf("get the cq->ip_index is %d\n",cq->ip_index);	
						//	tmp = tmp->next;  
						//printf("bbb conflag=%d\n",conflag);
						cq->next = NULL;
						ctail->next = cq;
						ctail = cq;
						//printf("xxx the point for ctail is %p\n",ctail);
						tmp = tmp->next;  
					}  
#if 0
					//printf("bbb conflag=%d\n",conflag);
					cq->next = NULL;
					ctail->next = cq;
					ctail = cq;
					//printf("xxx the point for ctail is %p\n",ctail);
#endif
				}
			}
			//printf("ccc conflag=%d\n",conflag);
			xmlFree(value);
			//printf("ddd conflag=%d\n",conflag);
		}	
		cur = cur->next;
		//printf("eee conflag=%d\n",conflag);
	}
	//printf("fff conflag=%d the end chead=%p\n",conflag,chead);
	*domain_num=conflag;
	xmlFreeDoc(doc);
	return DRP_RETURN_OK;
}
#if 0
int get_domainname_ip_xml(char * filepath,struct domain_st *chead,
		char * second_node_name,int *domain_num)
{
	xmlDocPtr doc;	 
	xmlNodePtr cur,tmp;
	int conflag=0;	
	char *psfilename;
	psfilename = filepath;
	doc = xmlReadFile(psfilename, "utf-8", 256); 
	if (doc == NULL ) 
	{
		xmlFreeDoc(doc);
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

	struct domain_st *ctail=NULL;
	chead->next=NULL;
	ctail=chead;

	cur = cur->xmlChildrenNode; 
	while(cur !=NULL)
	{								
		if (!xmlStrcmp(cur->name, BAD_CAST second_node_name))			 /*find the node "http" or "https" node in xml file*/ 
		{							
			xmlChar *value;
			value = xmlGetProp(cur, BAD_CAST "attribute");	/*get the attribute of second node"domain"*/
			if(value != NULL)
			{
				/*if(strcmp(second_node_attribute,(char*)value)==0)*/
				{
					conflag++; 
					struct domain_st  *cq;
					cq=(struct domain_st *)malloc(sizeof(struct domain_st)+1);
					memset(cq,0,sizeof(struct domain_st)+1);
					if(NULL == cq)
					{
						xmlFree(value);
						return	-1;
					}

					cq->domain_ip=(char *)malloc(50);			
					memset(cq->domain_ip,0,50); 
					cq->domain_name=(char *)malloc(128);			
					memset(cq->domain_name,0,128); 
					strcpy(cq->domain_name,value);

					xmlChar *value_ip=NULL; 
					tmp = cur->xmlChildrenNode;
					while(tmp !=NULL)
					{	 
						//get	 IP  address corresponding to the domain name
						//if ((!xmlStrcmp(tmp->name, BAD_CAST "ip")))					//test if the child node is what i want
						{
							memset(cq->domain_ip,0,50);
							value_ip=xmlNodeGetContent(tmp);
							strcpy(cq->domain_ip,(char *)value_ip); 
							xmlFree(value_ip);
						}

						tmp = tmp->next;												  
					}  

					cq->next = NULL;
					ctail->next = cq;
					ctail = cq;	
				}
			}
			xmlFree(value);

		}  

		cur = cur->next;
	}
	*domain_num=conflag;
	xmlFreeDoc(doc);
	return 0;
}
#endif


int get_domainname_ip_xml(char * filepath,struct domain_st *chead,
		char * second_node_name,int *domain_num, int *ip_count)
{
	xmlDocPtr doc;	 
	xmlNodePtr cur,tmp;
	int third_node=0,new_second_node=0; 
	int third_node_count = 0;
	char *psfilename;
	char *index_str = NULL;
	psfilename = filepath;
	doc = xmlReadFile(psfilename, "utf-8", 256); 
	if (doc == NULL ) 
	{
		xmlFreeDoc(doc);
		drp_log_err("get_domainname_ip_xml file %s xmlReadFile error !",psfilename);
		return DRP_ERR_OPXML_FILE_NOT_EXIST;
	}
	cur = xmlDocGetRootElement(doc); 
	if (cur == NULL)
	{
		xmlFreeDoc(doc);
		drp_log_err("get_domainname_ip_xml file %s xmlDocGetRootElement error !",psfilename);
		return DRP_ERR_OPXML_FILE_FORMAT_WRONG;
	}

	if (xmlStrcmp(cur->name, (const xmlChar *) "root")) 
	{
		xmlFreeDoc(doc);		
		drp_log_err("get_domainname_ip_xml file %s error !",psfilename);
		return DRP_ERR_OPXML_FILE_FORMAT_WRONG;
	}

	struct domain_st *ctail=NULL;
	chead->next=NULL;
	ctail=chead;

	cur = cur->xmlChildrenNode; 
	while(cur !=NULL)
	{								
		if (!xmlStrcmp(cur->name, BAD_CAST second_node_name))			 
		{							
			xmlChar *value;
			value = xmlGetProp(cur, BAD_CAST "attribute");	/*get the attribute of second node"domain"*/
			if(value != NULL)
			{
				/*if(strcmp(second_node_attribute,(char*)value)==0)*/
				new_second_node++;
				//printf("aaa second_node=%d",new_second_node);
				xmlChar *value_ip=NULL; 
				tmp = cur->xmlChildrenNode;
				third_node=0;
				while(tmp !=NULL)
				{	
#if 0
					//printf("bbb second_node=%d",new_second_node);
					if( new_second_node > 0)
					{ 
						third_node++;
					}
					else
					{
						third_node=1;
					}
#endif
					third_node++;
					struct domain_st  *cq;
					cq=(struct domain_st *)malloc(sizeof(struct domain_st)+1);
					memset(cq,0,sizeof(struct domain_st)+1);
					if(NULL == cq)
					{
						xmlFree(value);
						drp_log_err("get_domainname_ip_xml malloc error !");
						return DRP_ERR_MALLOC_FAILED;
					}

					cq->domain_ip=(char *)malloc(50);			
					memset(cq->domain_ip,0,50); 
					cq->domain_name=(char *)malloc(128);			
					memset(cq->domain_name,0,128); 
					strcpy(cq->domain_name,(char *)value);
					//get	 IP  address corresponding to the domain name
					//if ((!xmlStrcmp(tmp->name, BAD_CAST "ip")))					//test if the child node is what i want
					{
						memset(cq->domain_ip,0,50);
						value_ip=xmlNodeGetContent(tmp);
						strcpy(cq->domain_ip,(char *)value_ip); 
						xmlFree(value_ip);
					}
					//cq->ip_index=third_node;
					index_str = (char *)tmp->name + 2;
					cq->ip_index = atoi(index_str);
					//printf("index=%d third_node=%s index_str=%s",cq->ip_index, tmp->name,index_str);
					cq->next = NULL;
					ctail->next = cq;
					ctail = cq; 
					tmp = tmp->next;	
					third_node_count +=1;
				}  
				//printf("ccc second_node=%d",new_second_node);
			}

			xmlFree(value); 
		}  
		cur = cur->next;
	}
	*domain_num=new_second_node;
	*ip_count = third_node_count;
	xmlFreeDoc(doc);
	return DRP_RETURN_OK;
}

#if 0
int get_first_domain_xmlnode(char *xmlpath,char *node_name,char *newc)
{
	xmlDocPtr pdoc = NULL;
	xmlNodePtr pcurnode = NULL;
	pdoc = xmlReadFile(xmlpath,"utf-8",256);  
	if(NULL == pdoc)
	{
		xmlFreeDoc(pdoc);
		return 1;
	}
	pcurnode = xmlDocGetRootElement(pdoc); 
	if(pcurnode==NULL)
	{
		xmlFreeDoc(pdoc);
		return -1;
	}
	pcurnode = pcurnode->xmlChildrenNode; 
	while (NULL != pcurnode)
	{		
		xmlChar *value;
		if (!xmlStrcmp(pcurnode->name, BAD_CAST node_name)) 			 //找到 lstatus 这个 root 下一级子节点			   
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

int mod_first_domain_xmlnode(char *xmlpath,char *node_name,char *newc)
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

/*get second level xml node(with property) ,node_name:second node name;attribute:attribute name;ruler:the vlaue of attribute */
int get_domain_second_node_attr(char *fpath,char * node_name,char *attribute,char *ruler)
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
		xmlFreeDoc(pdoc);
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
					flag = 1;							// if the node (with attribute) has already exists
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
#endif

/*add second level xml node(with attribute) ,node_name:second node name;attribute:attribute name;ruler:the vlaue of attribute */
int add_domain_second_node_attr(char *fpath,char * node_name,char *attribute,char *ruler)
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
		xmlFreeDoc(pdoc);
		drp_log_err("add_domain_second_node_attr file %s xmlReadFile error !",psfilename);
		return DRP_ERR_OPXML_FILE_NOT_EXIST;
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
					flag = DRP_ERR_DNS_RESOLVE_DOMAIN_EXIST;	// if the node (with attribute) has already exists
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


/*node_name:second level node;	attribute:the attribute name; ruler:the valude of attribute ;content:third node name*/
int add_domain_third_node(char * fpath,char * node_name,char *attribute,char *ruler,char * third_node_name,char *third_node_content)
{
	xmlDocPtr pdoc = NULL;
	xmlNodePtr pcurnode = NULL;
	char *psfilename;
	psfilename = fpath;
	pdoc = xmlReadFile(psfilename,"utf-8",256); 
	if(NULL == pdoc)
	{
		xmlFreeDoc(pdoc);
		drp_log_err("add_domain_third_node file %s xmlReadFile error !",psfilename);
		return DRP_ERR_OPXML_FILE_NOT_EXIST;
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
							xmlNodePtr node1 = xmlNewNode(NULL,BAD_CAST third_node_name);			  //create a third level childnode 
							xmlNodePtr content1 = xmlNewText(BAD_CAST third_node_content);				//set the third level childnode's name
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
	return DRP_RETURN_OK;
}

/*delete second level xml node(with attribute)*/
/*node_name:second node name; attribure:the attribute name;ruler:the content of attribute in second node*/
int delete_domain_second_xmlnode(char *fpath,char *node_name,char *attribute,char *ruler)
{
	xmlDocPtr pdoc = NULL;
	xmlNodePtr pcurnode = NULL;
	char *psfilename;
	int flag=DRP_ERR_DNS_DOMAIN_NOT_EXIST;
	psfilename = fpath;

	if (NULL == fpath
			|| NULL == node_name
			|| NULL == attribute
			|| NULL == ruler){

		drp_log_err("delete_domain_second_xmlnode input param error!");
		return DRP_ERR_INPUT_PARAM_ERR;
	}

	pdoc = xmlReadFile(psfilename,"utf-8",256);  
	if(NULL == pdoc)
	{
		xmlFreeDoc(pdoc);
		drp_log_err("delete_domain_second_xmlnode file %s xmlReadFile error !",psfilename);
		return DRP_ERR_OPXML_FILE_NOT_EXIST;
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



//NOTICE: this function have bugs do really not usefull  !!!
/*delete third level xml node(with attribute)*/
int delete_domain_third_xmlnodel(char *fpath,char *node_name,char *attribute,char *ruler,char *secnode)
{
	xmlDocPtr pdoc = NULL;
	xmlNodePtr pcurnode = NULL;
	char *psfilename;
	int flag = DRP_ERR_DNS_DOMAIN_IP_NOT_EXIST;
	int total_third_node = 1;
	
	psfilename = fpath;
	pdoc = xmlReadFile(psfilename,"utf-8",256);  
	if(NULL == pdoc)
	{
		xmlFreeDoc(pdoc);
		drp_log_err("delete_domain_third_xmlnodel file %s xmlReadFile error !",psfilename);
		return DRP_ERR_OPXML_FILE_NOT_EXIST;
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
				//flag = DRP_ERR_DNS_DOMAIN_IP_NOT_EXIST;
				xmlNodePtr childPtr = propNodePtr; 
				childPtr=childPtr->children;
				while(childPtr !=NULL)
				{
					total_third_node += 1;
					if ((!xmlStrcmp(childPtr->name, BAD_CAST secnode)))
					{
						xmlNodePtr tempNode;
						tempNode = childPtr->next;
						xmlUnlinkNode(childPtr);
						xmlFreeNode(childPtr);
						childPtr= tempNode;
						total_third_node -= 1;
						flag = DRP_RETURN_OK;
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
	if (DRP_RETURN_OK == flag ){
		return total_third_node;
	}
	return flag;
}


#if 0
int  add_domain_ip_into_cache_xml(const char *domain)
{
	FILE *fp = NULL;
	char line[256], *p = NULL;
	int ip_num=0,ipval=-1;
	memset(line, 0, sizeof(line));
	char ip_name[128]={0};	

	if ( (fp = fopen(DOMAIN_NAME_IP_TEMP_PATH, "r")) == NULL)
		return 0;

	while (fgets(line, sizeof(line)-1, fp) != NULL)
	{
		for (p = line; *p; p++)
			if ('\r' == *p || '\n' == *p)
			{
				*p = '\0';
				ip_num++;
				break;
			}
		//printf("the ip num=%d ,line for ip address=%s\n",ip_num,line);
		sprintf(ip_name,"ip%d",ip_num);
		//printf("the ip in xml is %s\n",ip_name);
		/*check the line ,check whether it is a legal  IP address*/
		ipval=parse_ip_check(line);
		if(COMMON_SUCCESS != ipval)
		{
			break;
		}

		add_domain_third_node(XML_DOMAIN_PATH,"domain","attribute",domain,ip_name,line);
	}

	fclose(fp); 	
	return 0;
}
#endif



#ifdef drp_opxml_test


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpathInternals.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#include "drp_interface.h"
#include "drp_opxml.h"


void drp_test_show_conf()
{

	printf("================================================================================\n");
	struct	 domain_st	domain_head,*domain_cq=NULL;
	int  domain_num=0;
	int ip_num = 0;
	int count = 0;
	memset (&domain_head, 0, sizeof(domain_head));
	get_domainname_ip_xml(XML_DOMAIN_PATH,&domain_head,"domain",&domain_num,&ip_num);

	for( count = 0,domain_cq=domain_head.next; \
					(domain_cq != NULL) && (count < ip_num); 
					(domain_cq=domain_cq->next, count++))
	{
			printf("domain name: ");
			printf("%s\t",domain_cq->domain_name);
			printf("domain IP : ");
			printf("%s\t",domain_cq->domain_ip); 						
			printf("IP index for the domain name: ");
			printf("%d\n",domain_cq->ip_index);
	}

	if(domain_num>0)
	{	
		//vty_out(vty,"debug message!free link list\n");
		int free_domain_flag=0;
		free_read_domain_xml(&domain_head,&free_domain_flag);
		//vty_out(vty,"debug message!free_domain_flag=%d \n",free_domain_flag);
	}
	printf("===============================================================================\n");

	return ;
}

int main (int argc, char **argv)
{
	char *domain_name = "www.test.com";
	char *ip_index1 = "ip3";
	char *ip_index2 = "ip4";
	char *ip_addr_str = "192.168.7.1";
	int ret = 0;


	drp_test_show_conf();
	printf("++++1\n");
	add_domain_second_node_attr( XML_DOMAIN_PATH,"domain","attribute",domain_name);
	
	printf("++++2\n");
	add_domain_third_node(XML_DOMAIN_PATH,"domain","attribute",domain_name,ip_index1,ip_addr_str);
	
	printf("++++3\n");
	add_domain_third_node(XML_DOMAIN_PATH,"domain","attribute",domain_name,ip_index2,ip_addr_str);
	printf("++++4\n");

	drp_test_show_conf();
	printf("++++5\n");
	ret = delete_domain_third_xmlnodel(XML_DOMAIN_PATH,"domain","attribute",domain_name,ip_index2);
	printf("++++delete_domain_third_xmlnodel ret = %d\n",ret);
	drp_test_show_conf();
	delete_domain_second_xmlnode( XML_DOMAIN_PATH,"domain","attribute",domain_name);

	drp_test_show_conf();

	return 0;
}
#endif


