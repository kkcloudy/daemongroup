#include "ws_portal_conf.h"
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpathInternals.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
/*#include "cgic.h"*/
#include <signal.h>
#include <time.h>


//free link list
void Free_read_portal_xml(struct portal_st *head)
{
    struct portal_st *f1,*f2;
	f1=head->next;		 
	f2=f1->next;
	while(f2!=NULL)
	{
	  free(f1->pssid);
	  free(f1->pip);
	  free(f1->pport);
	  free(f1->pweb);
	  free(f1->pdom);
	  free(f1->purl);
	  free(f1);
	  f1=f2;
	  f2=f2->next;
	}
	free(f1);
}

int read_portal_xmlz(char *xml_fpath,struct portal_st *chead,int *confnum)
{
	xmlDocPtr doc;	 
	xmlNodePtr cur,tmp;
    int conflag=0;	
	doc = xmlReadFile(xml_fpath, "utf-8", 256); 
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

	struct portal_st *ctail=NULL;
	chead->next=NULL;
	ctail=chead;

	cur = cur->xmlChildrenNode;	
	while(cur !=NULL)
	{

	    tmp = cur->xmlChildrenNode;
		if (!xmlStrcmp(cur->name, BAD_CAST NODE_PORTAL_SECD))           
		{
		        /////////////conf informations
		        struct portal_st  *cq=NULL;
				cq=(struct portal_st *)malloc(sizeof(struct portal_st)+1);
				memset(cq,0,sizeof(struct portal_st)+1);
				if(NULL == cq)
				{
					return  -1;
				}
				
				cq->pssid=(char *)malloc(20);
				memset(cq->pssid,0,20);
				
				cq->pip=(char *)malloc(50);
				memset(cq->pip,0,50);   
				
				cq->pport=(char *)malloc(20);
				memset(cq->pport,0,20);
				
				cq->pweb=(char *)malloc(128);
				memset(cq->pweb,0,128);
				
				cq->pdom=(char *)malloc(128);
				memset(cq->pdom,0,128);
				
				cq->purl=(char *)malloc(256);
				memset(cq->purl,0,256);
				
                conflag++;
				xmlChar *value=NULL;
				
				while(tmp !=NULL)
				{	 
					//portal ssid
					if ((!xmlStrcmp(tmp->name, BAD_CAST NODE_PORTAL_SSID)))
					{
					    memset(cq->pssid,0,20);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->pssid,(char *)value);	
						xmlFree(value);
					}
					
					//portal ip
					else if ((!xmlStrcmp(tmp->name, BAD_CAST NODE_PORTAL_IP)))
					{
					    memset(cq->pip,0,50);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->pip,(char *)value);	
						xmlFree(value);
					}
					//portal port
					else if ((!xmlStrcmp(tmp->name, BAD_CAST NODE_PORTAL_PORT)))
					{
					    memset(cq->pport,0,20);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->pport,(char *)value);	
						xmlFree(value);
					}
					//portal web
					else if ((!xmlStrcmp(tmp->name, BAD_CAST NODE_PORTAL_WEB)))
					{
					    memset(cq->pweb,0,128);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->pweb,(char *)value);	
						xmlFree(value);
					}
					//portal domain
					else if ((!xmlStrcmp(tmp->name, BAD_CAST NODE_PORTAL_DOMAIN)))
					{
					    memset(cq->pdom,0,128);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->pdom,(char *)value);	
						xmlFree(value);
					}
					//portal url 
					else if ((!xmlStrcmp(tmp->name, BAD_CAST NODE_PORTAL_URL)))
					{
					    memset(cq->purl,0,256);
						value=xmlNodeGetContent(tmp);
						strcpy(cq->purl,(char *)value);	
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

int get_second_xmlnode_portal(char * fpath,char * node_name,char * content,char *gets,int flagz)
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
	return 0;
}

/*xml的内容，节点无属性，fpath:xml文件路径，nodename，二级节点名称，content，三级节点名称，newc，更改的新内容*/
int mod_second_xmlnode_portal(char * fpath,char * node_name,char * content,char *newc,int flagz)
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
	return 0;
}

/*查找指定节点，fpath，xml文件路径，nodename，二级xml节点名，content，三级xml节点名，keyz，三级xml节点名下内容，flagz，标记xml*/
int find_second_xmlnode_portal(char * fpath,char * node_name,char * content,char *keyz,int *flagz)
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
	return 0;
}

/*添加二级节点并内容fpath:xml file path,node_name:second level node,content:sub second node name,keyz:node content*/
int add_second_xmlnode_portal(char * fpath,char * node_name,ST_PORTAL_ALL *sysall)
{
    int numz=0,i=0;
	numz=sizeof(sysall->porinfo)/sizeof(sysall->porinfo[0]);
	
	
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
	    if(strcmp(sysall->porinfo[i].valuez,"")!=0)
		{
			xmlNodePtr node1 = xmlNewNode(NULL,BAD_CAST sysall->porinfo[i].valuez);
			xmlNodePtr content1 = xmlNewText(BAD_CAST sysall->porinfo[i].contentz);
			xmlAddChild(node,node1);
			xmlAddChild(node1,content1);
		}
	}	
	xmlSaveFile(fpath,pdoc); 
	return 0;
}

/*删除二级节点并内容fpath:xml file path,node_name:second level node,content:sub second node name,keyz:node content*/
int del_second_xmlnode_portal(char * fpath,char * node_name,int flagz)
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
	return 0;
}

int add_portal_server( char *xml_fpath, char *ssid, char *ip, int port, char *webz,char *domainz)
{
    #if 1
	ST_PORTAL_ALL sysall;	
	memset( &sysall, 0, sizeof(ST_PORTAL_ALL) );
	
	sprintf( sysall.porinfo[0].valuez, "%s",NODE_PORTAL_SSID);
	sprintf( sysall.porinfo[0].contentz, "%s",ssid );
	
	sprintf( sysall.porinfo[1].valuez, "%s",NODE_PORTAL_IP);
	sprintf( sysall.porinfo[1].contentz, "%s",ip );
	
	sprintf( sysall.porinfo[2].valuez, "%s",NODE_PORTAL_PORT);
	sprintf( sysall.porinfo[2].contentz, "%d",port );
	
	sprintf( sysall.porinfo[3].valuez, "%s",NODE_PORTAL_WEB);
	sprintf( sysall.porinfo[3].contentz, "%s",webz );
	
	sprintf( sysall.porinfo[4].valuez, "%s",NODE_PORTAL_DOMAIN);
	sprintf( sysall.porinfo[4].contentz, "%s",domainz);
	
	//sprintf( sysall.porinfo[5].valuez, "%s",NODE_PORTAL_URL);
	//sprintf( sysall.porinfo[5].contentz, "http://%s:%d/%s",ip,port,webz );

	add_second_xmlnode_portal( xml_fpath, NODE_PORTAL_SECD, &sysall);
	#endif
	return 0;
}

int add_portal_server_url( char *xml_fpath, char *ssid, char *purl)
{
    #if 1
	ST_PORTAL_ALL sysall;	
	memset( &sysall, 0, sizeof(ST_PORTAL_ALL) );
	
	sprintf( sysall.porinfo[0].valuez, "%s",NODE_PORTAL_SSID);
	sprintf( sysall.porinfo[0].contentz, "%s",ssid );
	
	sprintf( sysall.porinfo[1].valuez, "%s",NODE_PORTAL_URL);
	sprintf( sysall.porinfo[1].contentz, "%s",purl );

	add_second_xmlnode_portal( xml_fpath, NODE_PORTAL_SECD, &sysall);
	#endif
	return 0;
}

void if_portal_exist()
{
	if(access(PORTAL_XMLFZ,0)!=0)
	{
		xmlDocPtr doc = NULL;			/* document pointer */	
		xmlNodePtr root_node = NULL;

		doc = xmlNewDoc(BAD_CAST "1.0");
		root_node = xmlNewNode(NULL, BAD_CAST "root" );
		xmlDocSetRootElement(doc, root_node);

		xmlSaveFormatFileEnc(PORTAL_XMLFZ, doc, "UTF-8", 1);
		/*free the document */
		xmlFreeDoc(doc);
		xmlCleanupParser();
		xmlMemoryDump();
	}
}

void if_wtpxml_exist(char *xmlpath)
{
	if(access(xmlpath,0)!=0)
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
	}
}
int add_wtp_psk( char *xml_fpath, int index, char *psk)
{
	ST_PORTAL_ALL sysall;	
	memset( &sysall, 0, sizeof(ST_PORTAL_ALL) );
	
	sprintf( sysall.porinfo[0].valuez, "%s",NODE_WTP_INDEXZ);
	sprintf( sysall.porinfo[0].contentz, "%d",index );
	
	sprintf( sysall.porinfo[1].valuez, "%s",NODE_WTP_PSKZ);
	sprintf( sysall.porinfo[1].contentz, "%s",psk );

	add_second_xmlnode_portal( xml_fpath, NODE_WTP_F, &sysall);
	return 0;
}

