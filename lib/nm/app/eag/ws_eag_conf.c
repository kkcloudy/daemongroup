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
// #include "cgic.h"

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
					// max idle flow
					else if ((!xmlStrcmp(tmp->name, BAD_CAST HS_IDLEFLOW)))
					{
						memset(cq->max_idle_flow,0,10);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->max_idle_flow,(char *)value);
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
					    memset(cq->wlansidz,0,10);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->wlansidz,(char *)value);	
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
					 //vlan id 
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
	f2=f1->next;
	while(f2!=NULL)
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
	  free(f1);
	  f1=f2;
	  f2=f2->next;
	}
	free(f1);
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
