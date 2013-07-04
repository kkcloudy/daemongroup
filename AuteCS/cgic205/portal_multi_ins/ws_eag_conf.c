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
* ws_eag_conf.c
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

#include "ws_eag_conf.h"
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpathInternals.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>


#define BAD_CAST (const xmlChar *)

int if_design_node(char *fpath,char *node_name,char *attribute,char *ruler)
{
	 xmlDocPtr pdoc = NULL;
	 xmlNodePtr pcurnode_f = NULL; 
	 xmlNodePtr pcurnode = NULL; 
	 char *psfilename;
     psfilename = fpath;
	 int flag=-1;

	 pdoc = xmlReadFile(psfilename,"utf-8",256); 
	 
	  if(NULL == pdoc)
	  {
	     return 1;
	  }

	  pcurnode_f = xmlDocGetRootElement(pdoc);  

	  pcurnode = pcurnode_f->xmlChildrenNode;  

	  while (NULL != pcurnode)
	  {			
	                        
	          if (!xmlStrcmp(pcurnode->name, BAD_CAST node_name))                
	          {
					xmlChar *value;
					value = xmlGetProp(pcurnode, (const xmlChar *)attribute);
					if(strcmp(ruler,(char*)value)==0)
					{
							flag = 0;
					}
					xmlFree(value);
			  }        
			 pcurnode = pcurnode->next; 
	                    
	  }
	
    //xmlSaveFile(fpath,pdoc);
    xmlFreeDoc(pdoc);
	return flag;
}


int if_design_node_second(char *fpath,char *node_name,char *attribute,char *ruler,char *content)
{
		xmlDocPtr pdoc = NULL;

		xmlNodePtr pcurnode = NULL;

		xmlNodePtr design_node = NULL;

		pdoc = xmlReadFile(fpath,"utf-8",256);  

		int flag=-1;

		if(NULL == pdoc)
		{
			return 1;
		}

        int if_second_flag=0,if_second_null=0;
		
		pcurnode = xmlDocGetRootElement(pdoc); 

		pcurnode = pcurnode->xmlChildrenNode; 

		while (NULL != pcurnode)
	    {			

			if (!xmlStrcmp(pcurnode->name, BAD_CAST node_name))           
			{
					
					xmlChar* szAttr = xmlGetProp(pcurnode,BAD_CAST attribute);  

					if(!xmlStrcmp(szAttr,BAD_CAST ruler))
					{		
						xmlNodePtr childPtr = pcurnode; 
						childPtr=childPtr->children;
					
						while(childPtr !=NULL)
						{	 
							if ((!xmlStrcmp(childPtr->name, BAD_CAST content)))
							{
								flag=0;
								break;
							}

							childPtr = childPtr->next;
						}
						
					}

					xmlFree(szAttr);
			}        
			pcurnode = pcurnode->next; 

	    }	  
		//xmlSaveFile(fpath,pdoc);  
		xmlFreeDoc(pdoc);
		return flag;
}

/*/add xml node , property  one level*/
int add_eag_node_attr(char *fpath,char * node_name,char *attribute,char *ruler)
{
	 xmlDocPtr pdoc = NULL;

	 xmlNodePtr pcurnode_f = NULL; 
	 xmlNodePtr pcurnode = NULL; 

	 char *psfilename;
     psfilename = fpath;

	 int flag=0;

	 pdoc = xmlReadFile(psfilename,"utf-8",256); 
	 
	  if(NULL == pdoc)
	  {
	     return 1;
	  }

	  pcurnode_f = xmlDocGetRootElement(pdoc);  

	  pcurnode = pcurnode_f->xmlChildrenNode;  

	  while (NULL != pcurnode)
	  {			
	                        
	          if (!xmlStrcmp(pcurnode->name, BAD_CAST node_name))                
	          {
					xmlChar *value;
					value = xmlGetProp(pcurnode, (const xmlChar *)attribute);
					if(strcmp(ruler,(char*)value)==0)
					{
							flag = 1;
					}
					xmlFree(value);
			  }        
			 pcurnode = pcurnode->next; 
	                    
	  }

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

/*mod xml node attr 
check the eag nas xml attribute if default
if return 1,means the attribute is default
add by liuyu 2011-5-8 
*/

int if_eag_arrt_default(char * fpath,char * node_name,char *attribute,char *rule)
{
	xmlDocPtr pdoc = NULL;
	xmlNodePtr pcurnode = NULL;
	pdoc = xmlReadFile(fpath,"utf-8",256);	
	int flag = 1;
	if(NULL == pdoc)
	{
		return -1;
	}

	pcurnode = xmlDocGetRootElement(pdoc); 
	pcurnode = pcurnode->xmlChildrenNode; 

	while (NULL != pcurnode)
	{			

		if (!xmlStrcmp(pcurnode->name, BAD_CAST node_name)) 		  
		{				
			xmlChar* szAttr = xmlGetProp(pcurnode,BAD_CAST attribute);	
			if(xmlStrcmp(szAttr, BAD_CAST rule) == 0)
			{
				xmlFree(szAttr);
				flag = 0;
				break;
			}
			xmlFree(szAttr);			
			
		}		 
		pcurnode = pcurnode->next; 

	}	
	 
	xmlSaveFile(fpath,pdoc); 
	xmlFreeDoc(pdoc);
	return flag;
}

/*mod xml node attr 
Check the keyword whether  existence
if return 1 means can't find
add by liuyu 2011-5-8 
*/

int check_key_word(char * fpath,char * node_name,char *sec_node_name,char *rule)
{
	xmlDocPtr pdoc = NULL;
	xmlNodePtr pcurnode = NULL;
	xmlNodePtr sec_pcurnode = NULL;
	xmlChar *content;
	pdoc = xmlReadFile(fpath,"utf-8",256);	
	int flag = 1;
	if(NULL == pdoc)
	{
		return -1;
	}

	pcurnode = xmlDocGetRootElement(pdoc); 
	pcurnode = pcurnode->xmlChildrenNode; 

	while (NULL != pcurnode)
	{			

		if (!xmlStrcmp(pcurnode->name, BAD_CAST node_name)) 		  
		{
			//fprintf(stderr,"test_2_node_name=%s\n",node_name);
			sec_pcurnode = pcurnode->xmlChildrenNode;
			if (!xmlStrcmp(sec_pcurnode->name, BAD_CAST sec_node_name)) 	
			{				
				content = xmlNodeGetContent(sec_pcurnode);
				//fprintf(stderr,"test_2_sec_node_name=%s,content=%s,rule=%s\n",sec_node_name,content,rule);
				if(!xmlStrcmp(content, BAD_CAST rule))
				{
					flag = 0;
					xmlFree(content);
					break;
				}
				xmlFree(content); 
			}			
		}		 
		pcurnode = pcurnode->next; 

	}	
	 
	xmlSaveFile(fpath,pdoc); 
	xmlFreeDoc(pdoc);
	return flag;
}


/*mod xml node attr 
add by liuyu 2011-5-8 
*/

int mod_eag_node_arrt(char * fpath,char * node_name,char *attribute,char *att_old,char *att_new)
{
	xmlDocPtr pdoc = NULL;
	xmlNodePtr pcurnode = NULL;
	pdoc = xmlReadFile(fpath,"utf-8",256);	
	int flag = 1;
	if(NULL == pdoc)
	{
		return -1;
	}

	pcurnode = xmlDocGetRootElement(pdoc); 
	pcurnode = pcurnode->xmlChildrenNode; 

	while (NULL != pcurnode)
	{			

		if (!xmlStrcmp(pcurnode->name, BAD_CAST node_name)) 		  
		{				
			xmlChar* szAttr = xmlGetProp(pcurnode,BAD_CAST attribute);	
			if(xmlStrcmp(szAttr, BAD_CAST att_old) == 0)
			{
				xmlSetProp(pcurnode, BAD_CAST attribute, BAD_CAST att_new);	
				xmlFree(szAttr);
				flag = 0;
				break;
			}
			xmlFree(szAttr);			
			
		}		 
		pcurnode = pcurnode->next; 

	}	
	 
	xmlSaveFile(fpath,pdoc); 
	xmlFreeDoc(pdoc);
	return flag;
}


//mod xml node
int mod_eag_node(char * fpath,char * node_name,char *attribute,char *ruler,char * content,char *newc)
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

			if (!xmlStrcmp(pcurnode->name, BAD_CAST node_name))           
			{
					
					xmlChar* szAttr = xmlGetProp(pcurnode,BAD_CAST attribute);  

					if(!xmlStrcmp(szAttr,BAD_CAST ruler))
					{		
						xmlNodePtr childPtr = pcurnode; 
						childPtr=childPtr->children;
					
						while(childPtr !=NULL)
						{	 
						    if_second_null=1;
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
		if(if_second_flag==1)
		{

			   xmlNodePtr node = xmlNewNode(NULL,BAD_CAST content);    

               xmlAddChild(design_node,node);
                     
               xmlNodePtr content1 = xmlNewText(BAD_CAST newc);
                     
               xmlAddChild(node,content1);
			   design_node=NULL;
	
	    }
		 
		xmlSaveFile(fpath,pdoc);
		xmlFreeDoc(pdoc);
		return if_second_flag;
}

//if this file is xml_file
int if_xml_file_z(char * fpath)
{
	xmlDocPtr pdoc = NULL;
	
	char *psfilename;
	psfilename = fpath;

	pdoc = xmlReadFile(psfilename,"utf-8",256); 

	if(NULL == pdoc)
	{
		return -1;
	}
	xmlFreeDoc(pdoc);
	return 0;
}

//create xml for mibs
int create_eag_xml(char *xmlpath)
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

	do {
		char command[56];
		sprintf(command, "sudo chmod a+rw %s", xmlpath);
		if(strchr(command,';'))
			return -1;
		system(command);
	} while(0);
	
	return 0;	
}




//get node content ,for struct conf 
int get_nas_struct(char * fpath,char * node_name,char *attribute,char *ruler,struct st_nasz *cq)
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
					//ntype
					if ((!xmlStrcmp(tmp->name, BAD_CAST NTYPE)))
					{
					    memset(cq->ntype,0,32);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->ntype,(char *)value);	
						xmlFree(value);
					}
					
					//  nstart
					else if ((!xmlStrcmp(tmp->name, BAD_CAST NSTART)))
					{
					    memset(cq->nstart,0,32);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->nstart,(char *)value);	
						xmlFree(value);
					}

					//nend
					else if ((!xmlStrcmp(tmp->name, BAD_CAST NEND)))
					{
					    memset(cq->nend,0,32);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->nend,(char *)value);	
						xmlFree(value);
					}

				     //nnasid
					else if ((!xmlStrcmp(tmp->name, BAD_CAST NNASID)))
					{
					    memset(cq->nnasid,0,32);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->nnasid,(char *)value);	
						xmlFree(value);
					}
					//ncover
					else if ((!xmlStrcmp(tmp->name, BAD_CAST NCOVER)))
					{
					    memset(cq->ncover,0,32);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->ncover,(char *)value);	
						xmlFree(value);
					}
					tmp = tmp->next;
				}
			}
			xmlFree(szAttr);
		}        
		pcurnode = pcurnode->next; 
	}	  
	//xmlSaveFile(fpath,pdoc);  
	xmlFreeDoc(pdoc);
	return 0;
}

void Free_get_portal_struct(struct st_portalz *f1)
{
	  free(f1->pacname);
	  free(f1->pdomain);
	  free(f1->pip);
	  free(f1->pjom);
	  free(f1->pnport);
	  free(f1->pport);
	  free(f1->pweb);
	  free(f1->p_keyword);
	  free(f1->p_type);
	  free(f1->pmip);
	  free(f1->pmport);
	  free(f1->pacip);
	  free(f1->pnasid);
	  free(f1->pnas_port_type);	  
	  free(f1->purl_suffix);
}

//get node content ,for struct conf  Free_get_portal_struct freeflag=0
int get_portal_struct(char * fpath,char * node_name,char *attribute,char *ruler,struct st_portalz *cq)
{
	xmlDocPtr pdoc = NULL;

	xmlNodePtr pcurnode = NULL,tmp;
	char *psfilename;
	psfilename = fpath;
	int freeflag = -1;

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
                
				cq->pacname=(char *)malloc(32);
				memset(cq->pacname,0,32);
				cq->pdomain=(char *)malloc(32);
				memset(cq->pdomain,0,32);
				cq->pip=(char *)malloc(256);
				memset(cq->pip,0,256);
				cq->pjom=(char *)malloc(32);
				memset(cq->pjom,0,32);
				cq->pnport=(char *)malloc(32);
				memset(cq->pnport,0,32);
				cq->pport=(char *)malloc(32);
				memset(cq->pport,0,32);
				cq->pweb=(char *)malloc(128);
				memset(cq->pweb,0,128);
				cq->p_keyword=(char *)malloc(32);
				memset(cq->p_keyword,0,32);
				cq->p_type=(char *)malloc(32);
				memset(cq->p_type,0,32);
				cq->advertise_url=(char *)malloc(256);
				memset(cq->advertise_url,0,256);
				cq->purl_suffix=(char *)malloc(64);
				memset(cq->purl_suffix,0,64);
				cq->pmip=(char *)malloc(16);
				memset(cq->pmip,0,16);
				cq->pmport=(char *)malloc(16);
				memset(cq->pmport,0,16);
				cq->pacip=(char *)malloc(10);
				memset(cq->pacip,0,10);
				cq->pnasid=(char *)malloc(10);
				memset(cq->pnasid,0,10);
				cq->papmac=(char *)malloc(10);
				memset(cq->papmac,0,10);
				cq->pusermac=(char *)malloc(10);
				memset(cq->pusermac,0,10);
				cq->pwlanapmac=(char *)malloc(10);
				memset(cq->pwlanapmac,0,10);
				cq->pwlanusermac=(char *)malloc(10);
				memset(cq->pwlanusermac,0,10);
				cq->pwlandeskey=(char *)malloc(10);
				memset(cq->pwlandeskey,0,10);
				cq->pfirsturl=(char *)malloc(10);
				memset(cq->pfirsturl,0,10);
				cq->pwlanparameter=(char *)malloc(10);
				memset(cq->pwlanparameter,0,10);
				cq->pdeskey=(char *)malloc(10);
				memset(cq->pdeskey,0,10);
				cq->pnas_port_type=(char *)malloc(10);
				memset(cq->pnas_port_type,0,10);
				freeflag = 0;

				xmlChar *value;
				while(tmp !=NULL)
				{	 
					//pssid
					if ((!xmlStrcmp(tmp->name, BAD_CAST P_KEYWORD)))
					{
					    memset(cq->p_keyword,0,32);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->p_keyword,(char *)value);	
						xmlFree(value);
					}
					
					//  pip
					else if ((!xmlStrcmp(tmp->name, BAD_CAST PIP)))
					{
					    memset(cq->pip,0,256);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->pip,(char *)value);	
						xmlFree(value);
					}

					//pport
					else if ((!xmlStrcmp(tmp->name, BAD_CAST PPORT)))
					{
					    memset(cq->pport,0,32);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->pport,(char *)value);	
						xmlFree(value);
					}

				     //pweb
					else if ((!xmlStrcmp(tmp->name, BAD_CAST PWEB)))
					{
					    memset(cq->pweb,0,128);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->pweb,(char *)value);	
						xmlFree(value);
					}
					  //pacname
					else if ((!xmlStrcmp(tmp->name, BAD_CAST PACN)))
					{
					    memset(cq->pacname,0,32);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->pacname,(char *)value);	
						xmlFree(value);
					}
					 //pnport
					else if ((!xmlStrcmp(tmp->name, BAD_CAST PNPORT)))
					{
					    memset(cq->pnport,0,32);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->pnport,(char *)value);	
						xmlFree(value);
					}
					 //pjom
					else if ((!xmlStrcmp(tmp->name, BAD_CAST PJOM)))
					{
					    memset(cq->pjom,0,32);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->pjom,(char *)value);	
						xmlFree(value);
					}
					 //pdomain
					else if ((!xmlStrcmp(tmp->name, BAD_CAST PDOMAIN)))
					{
					    memset(cq->pdomain,0,32);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->pdomain,(char *)value);	
						xmlFree(value);
					}
					//type
					if ((!xmlStrcmp(tmp->name, BAD_CAST P_TYPE)))
					{
					    memset(cq->p_type,0,32);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->p_type,(char *)value);	
						xmlFree(value);
					}
					//type
					if ((!xmlStrcmp(tmp->name, BAD_CAST P_ADVERTISE_URL)))
					{
					    memset(cq->advertise_url,0,256);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->advertise_url,(char *)value);	
						xmlFree(value);
					}
					//pmip
					if ((!xmlStrcmp(tmp->name, BAD_CAST P_MAC_IP)))
					{
					    memset(cq->pmip,0,16);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->pmip,(char *)value);	
						xmlFree(value);
					}
					//pmport
					if ((!xmlStrcmp(tmp->name, BAD_CAST P_MAC_PORT)))
					{
					    memset(cq->pmport,0,16);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->pmport,(char *)value);	
						xmlFree(value);
					}
					//pacip
					if ((!xmlStrcmp(tmp->name, BAD_CAST P_ACIP)))
					{
					    memset(cq->pacip,0,10);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->pacip,(char *)value);	
						xmlFree(value);
					}	
					//pnasid
					if ((!xmlStrcmp(tmp->name, BAD_CAST P_NASID)))
					{
					    memset(cq->pnasid,0,10);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->pnasid,(char *)value);	
						xmlFree(value);
					}
					//papmac
					if ((!xmlStrcmp(tmp->name, BAD_CAST P_APMAC)))
					{
					    memset(cq->papmac,0,10);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->papmac,(char *)value);	
						xmlFree(value);
					}
					//pusermac
					if ((!xmlStrcmp(tmp->name, BAD_CAST P_USERMAC)))
					{
					    memset(cq->pusermac,0,10);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->pusermac,(char *)value);	
						xmlFree(value);
					}
					//pwlanapmac
					if ((!xmlStrcmp(tmp->name, BAD_CAST P_WLANAPMAC)))
					{
					    memset(cq->pwlanapmac,0,10);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->pwlanapmac,(char *)value);	
						xmlFree(value);
					}
					//pwlanusermac
					if ((!xmlStrcmp(tmp->name, BAD_CAST P_WLANUSERMAC)))
					{
					    memset(cq->pwlanusermac,0,10);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->pwlanusermac,(char *)value);	
						xmlFree(value);
					}
					//wlandeskey
					if ((!xmlStrcmp(tmp->name, BAD_CAST P_WLANDESKEY)))
					{
					    memset(cq->pwlandeskey,0,10);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->pwlandeskey,(char *)value);	
						xmlFree(value);
					}
					// add firsturl to url
					if ((!xmlStrcmp(tmp->name, BAD_CAST P_FIRSTURL)))
					{
					    memset(cq->pfirsturl,0,10);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->pfirsturl,(char *)value);	
						xmlFree(value);
					}
					//pwlanparameter
					if ((!xmlStrcmp(tmp->name, BAD_CAST P_PARAMETER)))
					{
					    memset(cq->pwlanparameter,0,10);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->pwlanparameter,(char *)value);	
						xmlFree(value);
					}
					//deskey for wlanparameter
					if ((!xmlStrcmp(tmp->name, BAD_CAST P_DESKEY)))
					{
					    memset(cq->pdeskey,0,10);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->pdeskey,(char *)value);	
						xmlFree(value);
					}
					//nas port type
					if ((!xmlStrcmp(tmp->name, BAD_CAST PNAS_PORT_TYPE)))
					{
					    memset(cq->pnas_port_type,0,10);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->pnas_port_type,(char *)value);	
						xmlFree(value);
					}	
					
					tmp = tmp->next;
				}
			}
			xmlFree(szAttr);
		}        
		pcurnode = pcurnode->next; 
	}	  
	//xmlSaveFile(fpath,pdoc);  
	xmlFreeDoc(pdoc);
	return freeflag;
}

//get node content ,for struct conf 
int get_radius_struct(char * fpath,char * node_name,char *attribute,char *ruler,struct st_radiusz *cq)
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
					//domain_name
					if ((!xmlStrcmp(tmp->name, BAD_CAST RDOMAIN)))
					{
					    memset(cq->domain_name,0,128);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->domain_name,(char *)value);	
						xmlFree(value);
					}
					//radius_server_type
					else if ((!xmlStrcmp(tmp->name, BAD_CAST RRADST)))
					{
					    memset(cq->radius_server_type,0,32);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->radius_server_type,(char *)value);	
						xmlFree(value);
					}
					
					//  radius_server_ip
					else if ((!xmlStrcmp(tmp->name, BAD_CAST RRIP)))
					{
					    memset(cq->radius_server_ip,0,50);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->radius_server_ip,(char *)value);	
						xmlFree(value);
					}

					//radius_server_port
					else if ((!xmlStrcmp(tmp->name, BAD_CAST RRPORT)))
					{
					    memset(cq->radius_server_port,0,32);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->radius_server_port,(char *)value);	
						xmlFree(value);
					}

				     //radius_server_key
					else if ((!xmlStrcmp(tmp->name, BAD_CAST RRKEY)))
					{
					    memset(cq->radius_server_key,0,128);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->radius_server_key,(char *)value);	
						xmlFree(value);
					}

					
				     //radius_server_portal
					else if ((!xmlStrcmp(tmp->name, BAD_CAST RRPORTAL)))
					{
					    memset(cq->radius_server_portal,0,32);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->radius_server_portal,(char *)value);	
						xmlFree(value);
					}


				     //charging_server_ip
					else if ((!xmlStrcmp(tmp->name, BAD_CAST RCIP)))
					{
					    memset(cq->charging_server_ip,0,50);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->charging_server_ip,(char *)value);	
						xmlFree(value);
					}

				     //charging_server_port
					else if ((!xmlStrcmp(tmp->name, BAD_CAST RCPORT)))
					{
					    memset(cq->charging_server_port,0,32);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->charging_server_port,(char *)value);	
						xmlFree(value);
					}

				     //charging_server_key
					else if ((!xmlStrcmp(tmp->name, BAD_CAST RCKEY)))
					{
					    memset(cq->charging_server_key,0,128);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->charging_server_key,(char *)value);	
						xmlFree(value);
					}

				     //backup_radius_server_ip
					else if ((!xmlStrcmp(tmp->name, BAD_CAST RBIP)))
					{
					    memset(cq->backup_radius_server_ip,0,50);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->backup_radius_server_ip,(char *)value);	
						xmlFree(value);
					}

				     //backup_radius_server_port
					else if ((!xmlStrcmp(tmp->name, BAD_CAST RBPORT)))
					{
					    memset(cq->backup_radius_server_port,0,32);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->backup_radius_server_port,(char *)value);	
						xmlFree(value);
					}

				     //backup_radius_server_key
					else if ((!xmlStrcmp(tmp->name, BAD_CAST RBKEY)))
					{
					    memset(cq->backup_radius_server_key,0,128);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->backup_radius_server_key,(char *)value);	
						xmlFree(value);
					}
				 
				     //backup_radius_server_portal
					else if ((!xmlStrcmp(tmp->name, BAD_CAST RBPORTAL)))
					{
					    memset(cq->backup_radius_server_portal,0,32);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->backup_radius_server_portal,(char *)value);	
						xmlFree(value);
					}

                    //backup_charging_server_ip
					else if ((!xmlStrcmp(tmp->name, BAD_CAST RBCIP)))
					{
					    memset(cq->backup_charging_server_ip,0,50);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->backup_charging_server_ip,(char *)value);	
						xmlFree(value);
					}

				     //backup_charging_server_port
					else if ((!xmlStrcmp(tmp->name, BAD_CAST RBCPORT)))
					{
					    memset(cq->backup_charging_server_port,0,32);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->backup_charging_server_port,(char *)value);	
						xmlFree(value);
					}
				     //backup_charging_server_key
					else if ((!xmlStrcmp(tmp->name, BAD_CAST RBCKEY)))
					{
					    memset(cq->backup_charging_server_key,0,128);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->backup_charging_server_key,(char *)value);	
						xmlFree(value);
					}
				     //swap octets
					else if ((!xmlStrcmp(tmp->name, BAD_CAST R_SWAP_OCTETS)))
					{
					    memset(cq->swap_octets,0,32);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->swap_octets,(char *)value);	
						xmlFree(value);
					}
				     /*strip domain name to radius server*/
					else if ((!xmlStrcmp(tmp->name, BAD_CAST R_STRIP_DOMAIN)))
					{
						memset(cq->strip_domain_name,0,32);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->strip_domain_name,(char *)value);	
						xmlFree(value);
					}
					
					tmp = tmp->next;
				}
			}
			xmlFree(szAttr);
		}        
		pcurnode = pcurnode->next; 
	}	  
	//xmlSaveFile(fpath,pdoc);  
	xmlFreeDoc(pdoc);
	return 0;
}


//get node content ,for struct conf 
int get_eag_struct(char * fpath,char * node_name,char *attribute,char *ruler,struct st_eagz *cq)
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
					//eag_start
					if ((!xmlStrcmp(tmp->name, BAD_CAST HS_STATUS)))
					{
					    memset(cq->eag_start,0,20);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->eag_start,(char *)value);	
						xmlFree(value);
					}
					
					//  space_start
					else if ((!xmlStrcmp(tmp->name, BAD_CAST HS_STATUS_KICK)))
					{
					    memset(cq->space_start,0,20);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->space_start,(char *)value);	
						xmlFree(value);
					}

					//debug_log
					else if ((!xmlStrcmp(tmp->name, BAD_CAST HS_DEBUG_LOG)))
					{
					    memset(cq->debug_log,0,20);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->debug_log,(char *)value);	
						xmlFree(value);
					}
					//force_wireless
					else if ((!xmlStrcmp(tmp->name, BAD_CAST HS_FORCE_WIRELESS)))
					{
					    memset(cq->force_wireless,0,20);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->force_wireless,(char *)value);	
						xmlFree(value);
					}
					//flow_from
					else if ((!xmlStrcmp(tmp->name, BAD_CAST HS_FLOW_FROM_TYPE)))
					{
					    memset(cq->flow_from,0,20);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->flow_from,(char *)value);	
						xmlFree(value);
					}
				     //db_listen
					else if ((!xmlStrcmp(tmp->name, BAD_CAST HS_UAMLISTEN)))
					{
					    memset(cq->db_listen,0,32);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->db_listen,(char *)value);	
						xmlFree(value);
					}
					//listen_port
					else if ((!xmlStrcmp(tmp->name, BAD_CAST HS_UAMPORT)))
					{
					    memset(cq->listen_port,0,20);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->listen_port,(char *)value);	
						xmlFree(value);
					}
				     //nasid
					else if ((!xmlStrcmp(tmp->name, BAD_CAST HS_NAS_PT)))
					{
					    memset(cq->nasid,0,10);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->nasid,(char *)value);	
						xmlFree(value);
					}
				     //radiusid
					else if ((!xmlStrcmp(tmp->name, BAD_CAST HS_RADIUS_PT)))
					{
					    memset(cq->radiusid,0,10);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->radiusid,(char *)value);	
						xmlFree(value);
					}
				     //portalid
					else if ((!xmlStrcmp(tmp->name, BAD_CAST HS_PORTAL_PT)))
					{
					    memset(cq->portalid,0,10);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->portalid,(char *)value);	
						xmlFree(value);
					}
					else if ((!xmlStrcmp(tmp->name, BAD_CAST HS_DEFIDLETIMEOUT)))
					{
					    memset(cq->timeout,0,10);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->timeout,(char *)value);	
						xmlFree(value);
					}
					else if ((!xmlStrcmp(tmp->name, BAD_CAST HS_VRRPID)))
					{
					    memset(cq->vrrpid,0,10);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->vrrpid,(char *)value);	
						xmlFree(value);
					}
                    else if ((!xmlStrcmp(tmp->name, BAD_CAST HS_PPI_PORT)))
					{
					    memset(cq->ppi_port,0,10);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->ppi_port,(char *)value);	
						xmlFree(value);
					}
				     //wwvid, strategy id for wlan & wtp mapping vlan
					else if ((!xmlStrcmp(tmp->name, BAD_CAST HS_WWV_PT)))
					{
					    memset(cq->wwvid,0,10);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->wwvid,(char *)value);	
						xmlFree(value);
					}
				     //max_httprsp
					else if ((!xmlStrcmp(tmp->name, BAD_CAST HS_MAX_HTTPRSP)))
					{
					    memset(cq->max_httprsp,0,10);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->max_httprsp,(char *)value);	
						xmlFree(value);
					}
					// def acct interval
					else if ((!xmlStrcmp(tmp->name, BAD_CAST HS_RADACCTINTERVAL)))
					{
						memset(cq->def_acct_interval,0,10);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->def_acct_interval,(char *)value);	
						xmlFree(value);
					}
					// radius retry count
					else if ((!xmlStrcmp(tmp->name, BAD_CAST HS_RADRETRYCOUNT)))
					{
						memset(cq->radius_retry_count,0,10);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->radius_retry_count,(char *)value);	
						xmlFree(value);
					}
					// radius retry interval
					else if ((!xmlStrcmp(tmp->name, BAD_CAST HS_RADRETRYINTERVAL)))
					{
						memset(cq->radius_retry_interval,0,10);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->radius_retry_interval,(char *)value);	
						xmlFree(value);
					}
					// max idle flow
					else if ((!xmlStrcmp(tmp->name, BAD_CAST HS_IDLEFLOW)))
					{
						memset(cq->max_idle_flow,0,10);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->max_idle_flow,(char *)value);
						xmlFree(value);
					}
					//auto session
					if ((!xmlStrcmp(tmp->name, BAD_CAST HS_AUTO_SESSION)))
					{
					    memset(cq->auto_session,0,20);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->auto_session,(char *)value);	
						xmlFree(value);
					}
					//class to bandwidth
					if ((!xmlStrcmp(tmp->name, BAD_CAST HS_CLASS_TO_BANDWIDTH)))
					{
					    memset(cq->class_to_bandwidth,0,20);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->class_to_bandwidth,(char *)value);	
						xmlFree(value);
					}
					//force dhcplease
					if ((!xmlStrcmp(tmp->name, BAD_CAST HS_FORCE_DHCPLEASE)))
					{
					    memset(cq->force_dhcplease,0,20);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->force_dhcplease,(char *)value);	
						xmlFree(value);
					}
					//check errid
					if ((!xmlStrcmp(tmp->name, BAD_CAST HS_CHECK_ERRID)))
					{
					    memset(cq->check_errid,0,20);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->check_errid,(char *)value);	
						xmlFree(value);
					}
					//notice to asd
					if ((!xmlStrcmp(tmp->name, BAD_CAST HS_NOTICE_TO_ASD)))
					{
					    memset(cq->notice_to_asd,0,20);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->notice_to_asd,(char *)value);	
						xmlFree(value);
					}

					//notice to bindserver
					if ((!xmlStrcmp(tmp->name, BAD_CAST HS_NOTICE_TO_BINDSERVER)))
					{
					    memset(cq->notice_to_bindserver,0,20);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->notice_to_bindserver,(char *)value);	
						xmlFree(value);
					}

					//mac_preauth_start
					if ((!xmlStrcmp(tmp->name, BAD_CAST HS_MAC_PREAUTH)))
					{
					    memset(cq->mac_preauth_start,0,20);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->mac_preauth_start,(char *)value);	
						xmlFree(value);
					}

					//mac_preauth_time
					if ((!xmlStrcmp(tmp->name, BAD_CAST HS_PREAUTH_TIME)))
					{
					    memset(cq->mac_preauth_time,0,20);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->mac_preauth_time,(char *)value);	
						xmlFree(value);
					}

					//mac_preauth_flow
					if ((!xmlStrcmp(tmp->name, BAD_CAST HS_PREAUTH_FLOW)))
					{
					    memset(cq->mac_preauth_flow,0,20);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->mac_preauth_flow,(char *)value);	
						xmlFree(value);
					}

					//mac_preauth_interval
					if ((!xmlStrcmp(tmp->name, BAD_CAST HS_PREAUTH_INTERVAL)))
					{
					    memset(cq->mac_preauth_interval,0,20);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->mac_preauth_interval,(char *)value);	
						xmlFree(value);
					}
					
					//radius_reauth
					if ((!xmlStrcmp(tmp->name, BAD_CAST HS_RADIUS_REAUTH)))
					{
					    memset(cq->radius_reauth,0,20);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->radius_reauth,(char *)value);	
						xmlFree(value);
					}

					//check-nasportid
					if ((!xmlStrcmp(tmp->name, BAD_CAST HS_CHECK_NASPORTID)))
					{
					    memset(cq->check_nasportid,0,20);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->check_nasportid,(char *)value);	
						xmlFree(value);
					}
					
					// fastfwd flow interval
					else if ((!xmlStrcmp(tmp->name, BAD_CAST HS_FFDFLOW_INTERVAL)))
					{
						memset(cq->fastfwd_flow_interval,0,10);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->fastfwd_flow_interval,(char *)value);	
						xmlFree(value);
					}
					
					//get flow from wireless
					#if 0
					if ((!xmlStrcmp(tmp->name, BAD_CAST HS_GET_FLOW_FROM_WIRELESS)))
					{
					    memset(cq->get_flow_from_wireless,0,20);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->get_flow_from_wireless,(char *)value);	
						xmlFree(value);
					}
					#endif
					tmp = tmp->next;
				}
			}
			xmlFree(szAttr);
		}        
		pcurnode = pcurnode->next; 
	}	  
	//xmlSaveFile(fpath,pdoc);  
	xmlFreeDoc(pdoc);
	return 0;
}

//get xml node signle
int get_eag_node_attr(char * fpath,char * node_name,char *attribute,char *ruler,char * content,char *logkey)
{
	xmlDocPtr pdoc = NULL;

	xmlNodePtr pcurnode = NULL;
	char *psfilename;
	psfilename = fpath;

	int flag=-1;

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
						flag=0;
					}

					childPtr = childPtr->next;
				}
			}
			xmlFree(szAttr);
		}        
		pcurnode = pcurnode->next; 
	}	  
	//xmlSaveFile(fpath,pdoc);  
	xmlFreeDoc(pdoc);
	return flag;
}
//get node under root has not attributes 
int get_eag_node(char * fpath,char * node_name,char *content)
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
	//xmlSaveFile(fpath,pdoc);  
	xmlFreeDoc(pdoc);
	return 0;
}

/*delete one level xml node*/
int delete_eag_onelevel(char *fpath,char *node_name,char *attribute,char *ruler)
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

/*delete second level xml node*/
int delete_eag_seclevel(char *fpath,char *node_name,char *attribute,char *ruler,char *secnode)
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


int read_radius_xml(char *fpath,struct st_radiusz *chead,int *confnum,char *node_name)
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

	struct st_radiusz *ctail=NULL;
	chead->next=NULL;
	ctail=chead;

	cur = cur->xmlChildrenNode;
	
	while(cur !=NULL)
	{

	    tmp = cur->xmlChildrenNode;
		if (!xmlStrcmp(cur->name, BAD_CAST node_name))           
		{
		        struct st_radiusz  *cq=NULL;
				cq=(struct st_radiusz *)malloc(sizeof(struct st_radiusz)+1);
				memset(cq,0,sizeof(struct st_radiusz)+1);
				if(NULL == cq)
				{
					return  -1;
				}
				
                conflag++;
				xmlChar *value=NULL;				
				while(tmp !=NULL)
				{	 
					//domain_name
					if ((!xmlStrcmp(tmp->name, BAD_CAST RDOMAIN)))
					{
					    memset(cq->domain_name,0,128);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->domain_name,(char *)value);	
						xmlFree(value);
					}
					//radius server type
					else if ((!xmlStrcmp(tmp->name, BAD_CAST RRADST)))
					{
					    memset(cq->radius_server_type,0,32);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->radius_server_type,(char *)value);	
						xmlFree(value);
					}
					//  radius_server_ip
					else if ((!xmlStrcmp(tmp->name, BAD_CAST RRIP)))
					{
					    memset(cq->radius_server_ip,0,50);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->radius_server_ip,(char *)value);	
						xmlFree(value);
					}

					//radius_server_port
					else if ((!xmlStrcmp(tmp->name, BAD_CAST RRPORT)))
					{
					    memset(cq->radius_server_port,0,32);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->radius_server_port,(char *)value);	
						xmlFree(value);
					}

				     //radius_server_key
					else if ((!xmlStrcmp(tmp->name, BAD_CAST RRKEY)))
					{
					    memset(cq->radius_server_key,0,128);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->radius_server_key,(char *)value);	
						xmlFree(value);
					}

					
				     //radius_server_portal
					else if ((!xmlStrcmp(tmp->name, BAD_CAST RRPORTAL)))
					{
					    memset(cq->radius_server_portal,0,32);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->radius_server_portal,(char *)value);	
						xmlFree(value);
					}


				     //charging_server_ip
					else if ((!xmlStrcmp(tmp->name, BAD_CAST RCIP)))
					{
					    memset(cq->charging_server_ip,0,50);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->charging_server_ip,(char *)value);	
						xmlFree(value);
					}

				     //charging_server_port
					else if ((!xmlStrcmp(tmp->name, BAD_CAST RCPORT)))
					{
					    memset(cq->charging_server_port,0,32);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->charging_server_port,(char *)value);	
						xmlFree(value);
					}

				     //charging_server_key
					else if ((!xmlStrcmp(tmp->name, BAD_CAST RCKEY)))
					{
					    memset(cq->charging_server_key,0,128);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->charging_server_key,(char *)value);	
						xmlFree(value);
					}

				     //backup_radius_server_ip
					else if ((!xmlStrcmp(tmp->name, BAD_CAST RBIP)))
					{
					    memset(cq->backup_radius_server_ip,0,50);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->backup_radius_server_ip,(char *)value);	
						xmlFree(value);
					}

				     //backup_radius_server_port
					else if ((!xmlStrcmp(tmp->name, BAD_CAST RBPORT)))
					{
					    memset(cq->backup_radius_server_port,0,32);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->backup_radius_server_port,(char *)value);	
						xmlFree(value);
					}

				     //backup_radius_server_key
					else if ((!xmlStrcmp(tmp->name, BAD_CAST RBKEY)))
					{
					    memset(cq->backup_radius_server_key,0,128);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->backup_radius_server_key,(char *)value);	
						xmlFree(value);
					}
				 
				     //backup_radius_server_portal
					else if ((!xmlStrcmp(tmp->name, BAD_CAST RBPORTAL)))
					{
					    memset(cq->backup_radius_server_portal,0,32);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->backup_radius_server_portal,(char *)value);	
						xmlFree(value);
					}

                    //backup_charging_server_ip
					else if ((!xmlStrcmp(tmp->name, BAD_CAST RBCIP)))
					{
					    memset(cq->backup_charging_server_ip,0,50);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->backup_charging_server_ip,(char *)value);	
						xmlFree(value);
					}

				     //backup_charging_server_port
					else if ((!xmlStrcmp(tmp->name, BAD_CAST RBCPORT)))
					{
					    memset(cq->backup_charging_server_port,0,32);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->backup_charging_server_port,(char *)value);	
						xmlFree(value);
					}
				     //backup_charging_server_key
					else if ((!xmlStrcmp(tmp->name, BAD_CAST RBCKEY)))
					{
					    memset(cq->backup_charging_server_key,0,128);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->backup_charging_server_key,(char *)value);	
						xmlFree(value);
					}
					// swap octets
					else if ((!xmlStrcmp(tmp->name, BAD_CAST R_SWAP_OCTETS)))
					{
					    memset(cq->swap_octets,0,32);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->swap_octets,(char *)value);	
						xmlFree(value);
					}
					
				     /*strip domain name to radius server*/
					else if ((!xmlStrcmp(tmp->name, BAD_CAST R_STRIP_DOMAIN)))
					{
						memset(cq->strip_domain_name,0,32);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->strip_domain_name,(char *)value);	
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

int read_optionz_xml(char *fpath,struct optionz *chead,int *confnum,char *node_sep)
{
	xmlDocPtr doc;	 
	xmlNodePtr cur,tmp; 
    int conflag=0;	
	char tmp0[10];
	memset(tmp0,0,10);
	sprintf(tmp0,"%s%s",node_sep,PLOTID_ZEAO);
	char tmp1[10];
	memset(tmp1,0,10);
	sprintf(tmp1,"%s%s",node_sep,PLOTID_ONE);
	char tmp2[10];
	memset(tmp2,0,10);
	sprintf(tmp2,"%s%s",node_sep,PLOTID_TWO);
	char tmp3[10];
	memset(tmp3,0,10);
	sprintf(tmp3,"%s%s",node_sep,PLOTID_THREE);
	char tmp4[10];
	memset(tmp4,0,10);
	sprintf(tmp4,"%s%s",node_sep,PLOTID_FOUR);
	char tmp5[10];
	memset(tmp5,0,10);
	sprintf(tmp5,"%s%s",node_sep,PLOTID_FIVE);

	
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

		tmp = cur->xmlChildrenNode;

	    if (!xmlStrcmp(cur->name, BAD_CAST tmp0))           
		{
			conflag++;
			memset(chead->content0,0,20);
			strcpy(chead->content0,PLOTID_ZEAO);
			

		}    	

		
		if (!xmlStrcmp(cur->name, BAD_CAST tmp1))           
		{
			conflag++;
			memset(chead->content1,0,20);
			strcpy(chead->content1,PLOTID_ONE);
			

		}    	
		if (!xmlStrcmp(cur->name, BAD_CAST tmp2))           
		{
			conflag++;
			memset(chead->content2,0,20);
			strcpy(chead->content2,PLOTID_TWO);
			

		}    	
		if (!xmlStrcmp(cur->name, BAD_CAST tmp3))           
		{
			conflag++;
			memset(chead->content3,0,20);
			strcpy(chead->content3,PLOTID_THREE);
			

		}    	
		if (!xmlStrcmp(cur->name, BAD_CAST tmp4))           
		{
			conflag++;
			memset(chead->content4,0,20);
			strcpy(chead->content4,PLOTID_FOUR);
			

		}    	
		if (!xmlStrcmp(cur->name, BAD_CAST tmp5))           
		{
			conflag++;
			memset(chead->content5,0,20);
			strcpy(chead->content5,PLOTID_FIVE);			

		}    	
		cur = cur->next;

	}
	
	
	 
    *confnum=conflag;
	xmlFreeDoc(doc);
	return 0;
}


int read_nas_xml(char *fpath,struct st_nasz *chead,int *confnum,char *node_name)
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

	struct st_nasz *ctail=NULL;
	chead->next=NULL;
	ctail=chead;

	cur = cur->xmlChildrenNode;
	
	while(cur !=NULL)
	{

	    tmp = cur->xmlChildrenNode;
		if (!xmlStrcmp(cur->name, BAD_CAST node_name))           
		{
		        struct st_nasz  *cq=NULL;
				cq=(struct st_nasz *)malloc(sizeof(struct st_nasz)+1);
				memset(cq,0,sizeof(struct st_nasz)+1);
				if(NULL == cq)
				{
					return  -1;
				}
				
                conflag++;
				xmlChar *value=NULL;				
				while(tmp !=NULL)
				{	 
					//ntype
					value = xmlGetProp(cur, (const xmlChar *)NATTR);
					if(value)
					{
						memset(cq->nattr, 0, sizeof(cq->nattr));
						strcpy(cq->nattr,(char *)value);
						xmlFree(value);
					}
					if ((!xmlStrcmp(tmp->name, BAD_CAST NTYPE)))
					{
					    memset(cq->ntype,0,32);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->ntype,(char *)value);	
						xmlFree(value);
					}
					
					//  nstart
					else if ((!xmlStrcmp(tmp->name, BAD_CAST NSTART)))
					{
					    memset(cq->nstart,0,32);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->nstart,(char *)value);	
						xmlFree(value);
					}

					//nend
					else if ((!xmlStrcmp(tmp->name, BAD_CAST NEND)))
					{
					    memset(cq->nend,0,32);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->nend,(char *)value);	
						xmlFree(value);
					}

				     //nnasid
					else if ((!xmlStrcmp(tmp->name, BAD_CAST NNASID)))
					{
					    memset(cq->nnasid,0,32);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->nnasid,(char *)value);	
						xmlFree(value);
					}
					//ncover
					else if ((!xmlStrcmp(tmp->name, BAD_CAST NCOVER)))
					{
					    memset(cq->ncover,0,32);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->ncover,(char *)value);	
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

int read_portal_xml(char *fpath,struct st_portalz *chead,int *confnum,char *node_name)
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
    *confnum=0;
	struct st_portalz *ctail=NULL;
	chead->next=NULL;
	ctail=chead;

	cur = cur->xmlChildrenNode;
	
	while(cur !=NULL)
	{
	    tmp = cur->xmlChildrenNode;
		if (!xmlStrcmp(cur->name, BAD_CAST node_name))           
		{

		        struct st_portalz  *cq=NULL;
				cq=(struct st_portalz *)malloc(sizeof(struct st_portalz)+1);
				memset(cq,0,sizeof(struct st_portalz)+1);
				if(NULL == cq)
				{
					return  -1;
				}
				cq->pacname=(char *)malloc(32);
				memset(cq->pacname,0,32);

				cq->pdomain=(char *)malloc(32);
				memset(cq->pdomain,0,32);

				cq->pip=(char *)malloc(256);
				memset(cq->pip,0,256);

				cq->pjom=(char *)malloc(32);
				memset(cq->pjom,0,32);

				cq->pnport=(char *)malloc(32);
				memset(cq->pnport,0,32);

				cq->pport=(char *)malloc(32);
				memset(cq->pport,0,32);

				cq->pweb=(char *)malloc(128);
				memset(cq->pweb,0,128);

				cq->p_keyword=(char *)malloc(32);
				memset(cq->p_keyword,0,32);

				cq->p_type=(char *)malloc(32);
				memset(cq->p_type,0,32);

				cq->advertise_url=(char *)malloc(256);
				memset(cq->advertise_url,0,(256));
	
				cq->purl_suffix=(char *)malloc(64);
				memset(cq->purl_suffix,0,(64));

				cq->pmip=(char *)malloc(16);
				memset(cq->pmip,0,(16));
				
				cq->pmport=(char *)malloc(16);
				memset(cq->pmport,0,(16));

				cq->pacip=(char *)malloc(10);
				memset(cq->pacip,0,10);

				cq->pnasid=(char *)malloc(10);
				memset(cq->pnasid,0,10);

				cq->papmac=(char *)malloc(10);
				memset(cq->papmac,0,10);

				cq->pusermac=(char *)malloc(10);
				memset(cq->pusermac,0,10);

				cq->pwlanapmac=(char *)malloc(10);
				memset(cq->pwlanapmac,0,10);

				cq->pwlanusermac=(char *)malloc(10);
				memset(cq->pwlanusermac,0,10);

				cq->pwlandeskey=(char *)malloc(10);
				memset(cq->pwlandeskey,0,10);

				cq->pfirsturl=(char *)malloc(10);
				memset(cq->pfirsturl,0,10);
				
				cq->pwlanparameter=(char *)malloc(10);
				memset(cq->pwlanparameter,0,10);

				cq->pdeskey=(char *)malloc(10);
				memset(cq->pdeskey,0,10);

				cq->pnas_port_type=(char *)malloc(10);
				memset(cq->pnas_port_type,0,10);
				
                conflag++;
				xmlChar *value=NULL;				
				while(tmp !=NULL)
				{	 
					//pssid
					if ((!xmlStrcmp(tmp->name, BAD_CAST P_KEYWORD)))
					{
					    memset(cq->p_keyword,0,32);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->p_keyword,(char *)value);	
						xmlFree(value);
					}
					
					//  pip
					else if ((!xmlStrcmp(tmp->name, BAD_CAST PIP)))
					{
					    memset(cq->pip,0,256);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->pip,(char *)value);	
						xmlFree(value);
					}

					//pport
					else if ((!xmlStrcmp(tmp->name, BAD_CAST PPORT)))
					{
					    memset(cq->pport,0,32);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->pport,(char *)value);	
						xmlFree(value);
					}

				     //pweb
					else if ((!xmlStrcmp(tmp->name, BAD_CAST PWEB)))
					{
					    memset(cq->pweb,0,128);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->pweb,(char *)value);	
						xmlFree(value);
					}
					 //pacname
					else if ((!xmlStrcmp(tmp->name, BAD_CAST PACN)))
					{
					    memset(cq->pacname,0,32);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->pacname,(char *)value);	
						xmlFree(value);
					}
					 //pnport
					else if ((!xmlStrcmp(tmp->name, BAD_CAST PNPORT)))
					{
					    memset(cq->pnport,0,32);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->pnport,(char *)value);	
						xmlFree(value);
					}
					 //pjom
					else if ((!xmlStrcmp(tmp->name, BAD_CAST PJOM)))
					{
					    memset(cq->pjom,0,32);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->pjom,(char *)value);	
						xmlFree(value);
					}
					//pdomain -- wk
					else if ((!xmlStrcmp(tmp->name, BAD_CAST PDOMAIN)))
					{
					    memset(cq->pdomain,0,32);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->pdomain,(char *)value);	
						xmlFree(value);
					}
					//type
					else if ((!xmlStrcmp(tmp->name, BAD_CAST P_TYPE)))
					{
					    memset(cq->p_type,0,32);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->p_type,(char *)value);	
						xmlFree(value);
					}
					//advertise url
					else if ((!xmlStrcmp(tmp->name, BAD_CAST P_ADVERTISE_URL)))
					{
					    memset(cq->advertise_url,0,256);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->advertise_url,(char *)value);	
						xmlFree(value);
					}
					 //url-suffix
					else if ((!xmlStrcmp(tmp->name, BAD_CAST P_URLSUFFIX)))
					{
					    memset(cq->purl_suffix,0,64);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->purl_suffix,(char *)value);	
						xmlFree(value);
					}
					//portal-mac-auth-ip
					else if ((!xmlStrcmp(tmp->name, BAD_CAST P_MAC_IP)))
					{
					    memset(cq->pmip,0,16);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->pmip,(char *)value);	
						xmlFree(value);
					}
					//portal-mac-auth-port
					else if ((!xmlStrcmp(tmp->name, BAD_CAST P_MAC_PORT)))
					{
					    memset(cq->pmport,0,16);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->pmport,(char *)value);
						xmlFree(value);
					}
					//nacip
					else if ((!xmlStrcmp(tmp->name, BAD_CAST P_ACIP)))
					{
					    memset(cq->pacip,0,10);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->pacip,(char *)value);
						xmlFree(value);
					}
					//nasid
					else if ((!xmlStrcmp(tmp->name, BAD_CAST P_NASID)))
					{
					    memset(cq->pnasid,0,10);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->pnasid,(char *)value);
						xmlFree(value);
					}
					//apmac
					else if ((!xmlStrcmp(tmp->name, BAD_CAST P_APMAC)))
					{
					    memset(cq->papmac,0,10);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->papmac,(char *)value);
						xmlFree(value);
					}
					//pusermac
					else if ((!xmlStrcmp(tmp->name, BAD_CAST P_USERMAC)))
					{
					    memset(cq->pusermac,0,10);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->pusermac,(char *)value);
						xmlFree(value);
					}
					//wlanapmac
					else if ((!xmlStrcmp(tmp->name, BAD_CAST P_WLANAPMAC)))
					{
					    memset(cq->pwlanapmac,0,10);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->pwlanapmac,(char *)value);
						xmlFree(value);
					}
					//wlanusermac
					else if ((!xmlStrcmp(tmp->name, BAD_CAST P_WLANUSERMAC)))
					{
					    memset(cq->pwlanusermac,0,10);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->pwlanusermac,(char *)value);
						xmlFree(value);
					}
					//wlandeskey
					else if ((!xmlStrcmp(tmp->name, BAD_CAST P_WLANDESKEY)))
					{
					    memset(cq->pwlandeskey,0,10);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->pwlandeskey,(char *)value);
						xmlFree(value);
					}
					//add firsturl to url
					else if ((!xmlStrcmp(tmp->name, BAD_CAST P_FIRSTURL)))
					{
					    memset(cq->pfirsturl,0,10);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->pfirsturl,(char *)value);
						xmlFree(value);
					}
					//add wlanparameter to url
					else if ((!xmlStrcmp(tmp->name, BAD_CAST P_PARAMETER)))
					{
					    memset(cq->pwlanparameter,0,10);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->pwlanparameter,(char *)value);
						xmlFree(value);
					}
					//add deskey for wlanparameter
					else if ((!xmlStrcmp(tmp->name, BAD_CAST P_DESKEY)))
					{
					    memset(cq->pdeskey,0,10);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->pdeskey,(char *)value);
						xmlFree(value);
					}
					//nas port type
					else if ((!xmlStrcmp(tmp->name, BAD_CAST PNAS_PORT_TYPE)))
					{
					    memset(cq->pnas_port_type,0,10);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->pnas_port_type,(char *)value);
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

#if 0
//read xml for wlan wtp vlan
int read_wwvz_xml(char *fpath,struct st_wwvz *chead,int *confnum,char *node_name)
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

	struct st_wwvz *ctail=NULL;
	chead->next=NULL;
	ctail=chead;

	cur = cur->xmlChildrenNode;
	
	while(cur !=NULL)
	{

	    tmp = cur->xmlChildrenNode;
		if (!xmlStrcmp(cur->name, BAD_CAST node_name))           
		{
		        struct st_wwvz  *cq=NULL;
				cq=(struct st_wwvz *)malloc(sizeof(struct st_wwvz)+1);
				memset(cq,0,sizeof(struct st_wwvz)+1);
				if(NULL == cq)
				{
					return  -1;
				}
				
                conflag++;
				xmlChar *value=NULL;				
				while(tmp !=NULL)
				{	 
					//wlan start id
					if ((!xmlStrcmp(tmp->name, BAD_CAST WLANSIDZ)))
					{
					    	memset(cq->wlan_vlan_sid,0,10);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->wlan_vlan_sid,(char *)value);	
						xmlFree(value);
					}
					
					//  wlan end id 
					else if ((!xmlStrcmp(tmp->name, BAD_CAST WLANEIDZ)))
					{
					    memset(cq->wlaneidz,0,10);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->wlaneidz,(char *)value);	
						xmlFree(value);
					}

					//wtp start id
					else if ((!xmlStrcmp(tmp->name, BAD_CAST WTPSIDZ)))
					{
					    memset(cq->wtpsidz,0,10);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->wtpsidz,(char *)value);	
						xmlFree(value);
					}

				     //wtp end id 
					else if ((!xmlStrcmp(tmp->name, BAD_CAST WTPEIDZ)))
					{
					    memset(cq->wtpeidz,0,10);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->wtpeidz,(char *)value);	
						xmlFree(value);
					}
					 //nasportid  
					else if ((!xmlStrcmp(tmp->name, BAD_CAST VLANIDZ)))
					{
					    memset(cq->vlanidz,0,10);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->vlanidz,(char *)value);	
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
#endif

//read xml for wlan wtp --->nasportid ,  vlan--->nasportid
int read_wwvz_xml(char *fpath,struct st_wwvz *chead,int *confnum,char *node_name)
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

	struct st_wwvz *ctail=NULL;
	chead->next=NULL;
	ctail=chead;

	cur = cur->xmlChildrenNode;
	
	while(cur !=NULL)
	{

	    tmp = cur->xmlChildrenNode;
		if (!xmlStrcmp(cur->name, BAD_CAST node_name))           
		{
		        struct st_wwvz  *cq=NULL;
				cq=(struct st_wwvz *)malloc(sizeof(struct st_wwvz)+1);
				memset(cq,0,sizeof(struct st_wwvz)+1);
				if(NULL == cq)
				{
					return  -1;
				}
				
                		conflag++;
				xmlChar *value=NULL;				
				while(tmp !=NULL)
				{	 
					//get nasportid type
					if ((!xmlStrcmp(tmp->name, BAD_CAST NASPORTID_TYPE)))
					{
						memset(cq->nasportid_type,0,10);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->nasportid_type,(char *)value);	
						xmlFree(value);
					}
					//wlan start id ,vlan start id
					if ((!xmlStrcmp(tmp->name, BAD_CAST WLAN_VLAN_SIDZ)))
					{
						memset(cq->wlan_vlan_sid,0,10);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->wlan_vlan_sid,(char *)value);	
						xmlFree(value);
					}
					
					//  wlan end id ,vlan end id ,
					else if ((!xmlStrcmp(tmp->name, BAD_CAST WLAN_VLAN_EIDZ)))
					{
						memset(cq->wlan_vlan_eid,0,10);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->wlan_vlan_eid,(char *)value);	
						xmlFree(value);
					}

					//wtp start id
					else if ((!xmlStrcmp(tmp->name, BAD_CAST WTPSIDZ)))
					{
					    	memset(cq->wtpsidz,0,10);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->wtpsidz,(char *)value);	
						xmlFree(value);
					}

				     //wtp end id 
					else if ((!xmlStrcmp(tmp->name, BAD_CAST WTPEIDZ)))
					{
						memset(cq->wtpeidz,0,10);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->wtpeidz,(char *)value);	
						xmlFree(value);
					}
					 //nasportid 
					else if ((!xmlStrcmp(tmp->name, BAD_CAST NASPORTIDZ)))
					{
						memset(cq->nasportid,0,10);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->nasportid,(char *)value);	
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

//read xml for  vlan-nasportid map
int read_vvnasportid_xml(char *fpath,struct st_vnz *chead,int *confnum,char *node_name)
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

	struct st_vnz *ctail=NULL;
	chead->next=NULL;
	ctail=chead;

	cur = cur->xmlChildrenNode;
	
	while(cur !=NULL)
	{

	    tmp = cur->xmlChildrenNode;
		if (!xmlStrcmp(cur->name, BAD_CAST node_name))           
		{
		        struct st_vnz  *cq=NULL;
				cq=(struct st_vnz *)malloc(sizeof(struct st_vnz)+1);
				memset(cq,0,sizeof(struct st_vnz)+1);
				if(NULL == cq)
				{
					return  -1;
				}
				
                		conflag++;
				xmlChar *value=NULL;				
				while(tmp !=NULL)
				{	 
					//vlan start id
					if ((!xmlStrcmp(tmp->name, BAD_CAST VLANSIDZ)))
					{
						memset(cq->vlansidz,0,10);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->vlansidz,(char *)value);	
						xmlFree(value);
					}
					
					//  Vlan end id 
					else if ((!xmlStrcmp(tmp->name, BAD_CAST VLANEIDZ)))
					{
						memset(cq->vlaneidz,0,10);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->vlaneidz,(char *)value);	
						xmlFree(value);
					}

					 //nasportid  
					else if ((!xmlStrcmp(tmp->name, BAD_CAST NASPORTIDZ)))
					{
						memset(cq->nasportid,0,10);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->nasportid,(char *)value);	
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

int get_nasportid_map_node_num(char *fpath,int *confnum,char *node_name)
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

	cur = cur->xmlChildrenNode;
	
	while(cur !=NULL)
	{
		if (!xmlStrcmp(cur->name, BAD_CAST node_name))           
		{
                		conflag++;
		}
		 cur = cur->next;
	}
	*confnum=conflag;
	xmlFreeDoc(doc);
	return 0;
}

int read_eag_xml(char *fpath,struct st_eagz *chead,int *confnum,char *node_name)
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

	struct st_eagz *ctail=NULL;
	chead->next=NULL;
	ctail=chead;

	cur = cur->xmlChildrenNode;
	
	while(cur !=NULL)
	{

	    tmp = cur->xmlChildrenNode;
		if (!xmlStrcmp(cur->name, BAD_CAST node_name))           
		{
		        struct st_eagz  *cq=NULL;
				cq=(struct st_eagz *)malloc(sizeof(struct st_eagz)+1);
				memset(cq,0,sizeof(struct st_eagz)+1);
				if(NULL == cq)
				{
					return  -1;
				}
				
                conflag++;
				xmlChar *value=NULL;				
				while(tmp !=NULL)
				{	 
					//eag_start
					if ((!xmlStrcmp(tmp->name, BAD_CAST HS_STATUS)))
					{
					    memset(cq->eag_start,0,20);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->eag_start,(char *)value);	
						xmlFree(value);
					}
					
					//  space_start
					else if ((!xmlStrcmp(tmp->name, BAD_CAST HS_STATUS_KICK)))
					{
					    memset(cq->space_start,0,20);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->space_start,(char *)value);	
						xmlFree(value);
					}

					//debug_log
					else if ((!xmlStrcmp(tmp->name, BAD_CAST HS_DEBUG_LOG)))
					{
					    memset(cq->debug_log,0,20);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->debug_log,(char *)value);	
						xmlFree(value);
					}

				     //db_listen
					else if ((!xmlStrcmp(tmp->name, BAD_CAST HS_UAMLISTEN)))
					{
					    memset(cq->db_listen,0,32);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->db_listen,(char *)value);	
						xmlFree(value);
					}
					//listen_port
					else if ((!xmlStrcmp(tmp->name, BAD_CAST HS_UAMPORT)))
					{
					    memset(cq->listen_port,0,20);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->listen_port,(char *)value);	
						xmlFree(value);
					}
				     //nasid
					else if ((!xmlStrcmp(tmp->name, BAD_CAST HS_NAS_PT)))
					{
					    memset(cq->nasid,0,10);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->nasid,(char *)value);	
						xmlFree(value);
					}
				     //radiusid
					else if ((!xmlStrcmp(tmp->name, BAD_CAST HS_RADIUS_PT)))
					{
					    memset(cq->radiusid,0,10);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->radiusid,(char *)value);	
						xmlFree(value);
					}
				     //portalid
					else if ((!xmlStrcmp(tmp->name, BAD_CAST HS_PORTAL_PT)))
					{
					    memset(cq->portalid,0,10);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->portalid,(char *)value);	
						xmlFree(value);
					}
					else if ((!xmlStrcmp(tmp->name, BAD_CAST HS_DEFIDLETIMEOUT)))
					{
					    memset(cq->timeout,0,10);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->timeout,(char *)value);	
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

/*read xml content ,save it as link list(test ok)*/
int string_sep_list(struct substringz *head,int *subnum,char *source,char *sep)
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
void Free_sepstringz_all(struct substringz *head)
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
void Free_nas_info(struct st_nasz *head)
{
    struct st_nasz *f1,*f2;
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
void Free_portal_info(struct st_portalz *head)
{
    struct st_portalz *f1,*f2;
	f1=head->next;		 
	f2=f1;
	while(f2!=NULL)
	{
	  f2=f1->next;
	  free(f1->pacname);
	  free(f1->pdomain);
	  free(f1->pip);
	  free(f1->pjom);
	  free(f1->pnport);
	  free(f1->pport);
	  free(f1->pweb);
	  free(f1->p_keyword);
	  free(f1->p_type);
	  free(f1->advertise_url);
	  free(f1->purl_suffix);
	  free(f1->pmip);
	  free(f1->pmport);
	  free(f1->pacip);
	  free(f1->pnasid);
	  free(f1->papmac);
	  free(f1->pusermac);
	  free(f1->pwlanapmac);
	  free(f1->pwlanusermac);
	  free(f1->pwlandeskey);
	  free(f1->pnas_port_type);	 
	  free(f1);
	  f1=f2;
	}
	
	return ;
}
//free link list
void Free_wwvz_info(struct st_wwvz *head)
{
    struct st_wwvz *f1,*f2;
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
void Free_vnz_info(struct st_vnz *head)
{
    struct st_vnz *f1,*f2;
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
void Free_eag_info(struct st_eagz *head)
{
    struct st_eagz *f1,*f2;
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
void Free_radius_info(struct st_radiusz *head)
{
    struct st_radiusz *f1,*f2;
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


void write_status_file( char *file, char *state )
{
	FILE *fp = NULL;
	
	fp = fopen( file, "wb+" );
	if( NULL == fp )
	{
		fprintf( stderr, "write status file failed!file=%s  state=%s\n", file, state );
		return;	
	}
	fprintf( fp, "%s", state );
	fclose( fp );
	return;
}

int add_portal_server_mib(char *xmlpath,char *nodename,char *pssid,char *purl,char *acname,char *portal_server_portal,char *noteport,char *domain,char *type)
{

	add_eag_node_attr(xmlpath, nodename, ATT_Z, pssid);
	mod_eag_node(xmlpath, nodename, ATT_Z, pssid, P_KEYWORD, pssid);
	mod_eag_node(xmlpath, nodename, ATT_Z, pssid, PIP, purl);
	mod_eag_node(xmlpath, nodename, ATT_Z, pssid, PACN, acname);
	mod_eag_node(xmlpath, nodename, ATT_Z, pssid, PJOM, portal_server_portal);
	mod_eag_node(xmlpath, nodename, ATT_Z, pssid, PPORT, "0");
    mod_eag_node(xmlpath, nodename, ATT_Z, pssid, PWEB, "");
	if(strcmp(noteport,"")!=0)
	mod_eag_node(xmlpath, nodename, ATT_Z, pssid, PNPORT, noteport);		
	mod_eag_node(xmlpath, nodename, ATT_Z, pssid, PDOMAIN, domain);
	mod_eag_node(xmlpath, nodename, ATT_Z, pssid, P_TYPE, type);

	return 0;
}
int ccgi_eag_ins_exist(int ins_id)
{
	char tmpz[10];
	int flag = 1;
	
	memset(tmpz, 0, 10);
	sprintf(tmpz, "%d", ins_id);

	if (access(MULTI_EAG_F, 0) != 0)
		return 0;
	flag = if_design_node(MULTI_EAG_F, MTC_N, ATT_Z, tmpz);

	return 0 == flag;
}
int ccgi_eag_ins_get(int ins_id, struct st_eagz *cq)
{
	int flag = -1;
	char tmpz[10];
	
	memset(tmpz, 0, 10);
	sprintf(tmpz, "%d", ins_id);
	memset(cq, 0, sizeof(*cq));

	if (access(MULTI_EAG_F, 0) != 0)
		return flag;
	flag = get_eag_struct(MULTI_EAG_F, MTC_N, ATT_Z, tmpz, cq);
	
	return flag;
}
int ccgi_eag_ins_is_running(int ins_id)
{
	struct st_eagz cq;
	memset(&cq, 0, sizeof(cq));

	if (ins_id < 1 || ins_id > 5){
		return 0;
	}
	
	ccgi_eag_ins_get(ins_id, &cq);
	if (strcmp(cq.eag_start, "start") == 0){		/* A.B.C.D */
		return 1;
	}
	
	return 0;
}

int xmlencode(char *src, int srclen, char *dst, int dstsize) {
	char *x;
	int n;
	int i = 0;

	for (n=0; n<srclen; n++) {
		x=0;
		switch(src[n]) {
			case '&':  x = "&amp;";  break;
				   case '\"': x = "&quot;"; break;
			case '<':  x = "&lt;";   break;
			case '>':  x = "&gt;";   break;
			default:
				   if (i < dstsize - 1) dst[i++] = src[n];
				   break;
		}
		if (x) {
			if (i < dstsize - strlen(x)) {
				strncpy(dst + i, x, strlen(x));
				i += strlen(x);
			}
		}
	}
	dst[i] = 0;
	return 0;
}
char *ccgi_ip2str(unsigned long ip, char *str, int size)
{
	if (NULL == str) {
		return NULL;
	}

	memset(str, 0, size);
	snprintf(str, size-1, "%u.%u.%u.%u",
		(ip>>24)&0xff, (ip>>16)&0xff, (ip>>8)&0xff, ip&0xff);

	return str;
}



int  ccgi_inet_atoi (const char *cp, unsigned int  *inaddr)
{
  int dots = 0;
  register u_long addr = 0;
  register u_long val = 0, base = 10;

  do
    {
      register char c = *cp;

      switch (c)
	{
	case '0': case '1': case '2': case '3': case '4': case '5':
	case '6': case '7': case '8': case '9':
	  val = (val * base) + (c - '0');
	  break;
	case '.':
	  if (++dots > 3)
	    return 0;
	case '\0':
	  if (val > 255)
	    return 0;
	  addr = addr << 8 | val;
	  val = 0;
	  break;
	default:
	  return 0;
	}
    } while (*cp++) ;

  if (dots < 3)
    addr <<= 8 * (3 - dots);
  if (inaddr)
    *inaddr = htonl (addr);
  return 1;
}

/* This function is copyed from WID_Check_IP_Format() in wid_wtp.h . */
int ccgi_eag_check_ip_format(const char *str)
{
	char *endptr = NULL;
	char* endptr1 = NULL;
	char c;
	int IP,i;
	
	c = str[0];
	if (c>='0'&&c<='9'){
		IP= strtoul(str,&endptr,10);
		if(IP < 0||IP > 255)
			return 1;
		else if(((IP < 10)&&((endptr - str) > 1))||((IP < 100)&&((endptr - str) > 2))||((IP < 256)&&((endptr - str) > 3)))
			return 1;
		for(i = 0; i < 3; i++){
			if(endptr[0] == '\0'||endptr[0] != '.')
				return 1;
			else{
				endptr1 = &endptr[1];
				IP= strtoul(&endptr[1],&endptr,10); 			
				if(IP < 0||IP > 255)
					return 1;				
				else if(((IP < 10)&&((endptr - endptr1) > 1))||((IP < 100)&&((endptr - endptr1) > 2))||((IP < 256)&&((endptr - endptr1) > 3)))
					return 1;
			}
		}
		if(endptr[0] == '\0' && IP >= 0)
			return 0;
		else
			return 1;
	}
	else
		return 1;		
}

char *ccgi_mac2str(char  mac[6], char *str, int  size, char separator)
{
	if (NULL == mac || NULL == str || size <= 0) {
		return NULL;
	}
	if (':' != separator && '-' != separator && '_' != separator) {
		separator = ':';
	}

	memset(str, 0, size);
	snprintf(str, size-1, "%02X%c%02X%c%02X%c%02X%c%02X%c%02X",
		mac[0], separator, mac[1], separator, mac[2], separator,
		mac[3], separator, mac[4], separator, mac[5]);

	return str;
}

int ccgi_str2mac(const char *str, char mac[6])
{
	char separator = ':';
	int num = 0;
	
	if (NULL == str || NULL == mac || strlen(str) < 17) {
		return -1;
	}

	separator = str[2];
	switch (separator) {
	case ':':
		num = sscanf(str, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
			&mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
		break;
	case '-':
		num = sscanf(str, "%hhx-%hhx-%hhx-%hhx-%hhx-%hhx",
			&mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
		break;
	case '_':
		num = sscanf(str, "%hhx_%hhx_%hhx_%hhx_%hhx_%hhx",
			&mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
		break;
	default:
		break;
	}
	
	return (6 == num) ? 0 : -1;
}

