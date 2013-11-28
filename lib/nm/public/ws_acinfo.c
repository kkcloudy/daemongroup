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
* ws_acinfo.c
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
#include "ws_acinfo.h"
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpathInternals.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include "ws_snmpd_engine.h"


//free link list
void Free_read_acinfo_xml(struct acbackup_st *head)
{
    struct netipbk_st *f1,*f2;
	if(head->netipst.next != NULL)
	{
		f1=head->netipst.next;
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
}

int read_acinfo_xml(struct acbackup_st *chead,int *confnum)
{
	xmlDocPtr doc;	 
	xmlNodePtr cur;
    int conflag = 0;	
	if(access(ACBACKUPFILE,0) != 0)
	{
		return -1;
	}
	doc = xmlReadFile(ACBACKUPFILE, "utf-8", 256); 
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

	struct netipbk_st *nq,*ntail;
	struct netipbk_st *nq_ipv6,*ntail_ipv6;
	ntail = &(chead->netipst);
	ntail_ipv6 = &(chead->netipst_ipv6);
	cur = cur->xmlChildrenNode;	
	while(cur !=NULL)
	{
	    /////////////conf informations
		
	    conflag++;
		xmlChar *value=NULL;
		if ((!xmlStrcmp(cur->name, BAD_CAST AC_IDENTITY)))
		{

			memset(chead->identity,0,sizeof(chead->identity));
			value=xmlNodeGetContent(cur);
			strcpy(chead->identity,(char *)value);
			xmlFree(value);
		}
		else if ((!xmlStrcmp(cur->name, BAD_CAST AC_MODE)))
		{

			memset(chead->mode,0,sizeof(chead->mode));
			value=xmlNodeGetContent(cur);
			strcpy(chead->mode,(char *)value);
			xmlFree(value);
		}
		else if ((!xmlStrcmp(cur->name, BAD_CAST AC_STATUS)))
		{

			memset(chead->status,0,sizeof(chead->status));
			value=xmlNodeGetContent(cur);
			strcpy(chead->status,(char *)value);
			xmlFree(value);
		}
		//acinfo for net ip
		if ((!xmlStrcmp(cur->name, BAD_CAST AC_NETIP)))
		{
			nq=(struct netipbk_st *)malloc(sizeof(struct netipbk_st)+1);
			memset(nq,0,sizeof(struct netipbk_st)+1);
			if(NULL == nq)
			{
				return  -1;
			}

			memset(nq->netip,0,sizeof(nq->netip));
			value=xmlNodeGetContent(cur);
			strcpy(nq->netip,(char *)value);
			xmlFree(value);
			
			nq->next = NULL;
			ntail->next = nq;
			ntail = nq;
		}
		if ((!xmlStrcmp(cur->name, BAD_CAST AC_NETIP_IPV6)))
		{	
			nq_ipv6=(struct netipbk_st *)malloc(sizeof(struct netipbk_st)+1);
			memset(nq_ipv6,0,sizeof(struct netipbk_st)+1);
			if(NULL == nq_ipv6)
			{
				return	-1;
			}
			memset(nq_ipv6->netip,0,sizeof(nq_ipv6->netip));
			value=xmlNodeGetContent(cur);
			strcpy(nq_ipv6->netip,(char *)value);
			xmlFree(value);
			
			nq_ipv6->next = NULL;
			ntail_ipv6->next = nq_ipv6;
			ntail_ipv6 = nq_ipv6;
		}
		cur = cur->next;
	}
    *confnum=conflag;
	xmlFreeDoc(doc);
	return 0;
}
int get_ip_by_active_instance(char *instance,unsigned long *ip)
{
	char *ins_id  = NULL;
	int retu = 0;
	char *p = NULL;
	int confnum = 0;
	unsigned long ip_address = 0;
	ins_id = instance;
	
	struct acbackup_st ahead;
	struct netipbk_st *aq = NULL;
	memset(&ahead,0,sizeof(struct acbackup_st));
	retu = read_acinfo_xml(&ahead,&confnum);
	if(0 == retu)
	{
		aq = ahead.netipst.next;
		while(aq != NULL)
		{
			p = strtok(aq->netip, " ");
			if(strcmp(p,ins_id) == 0)
			{
				p = strtok(NULL, " "); 
				if(p)
				INET_ATON(ip_address, p);
				break;
			}
			else
			{
				aq = aq->next;
				ip_address = 0;
				p = NULL;
			}
		}
	}
	else
	{
		ip_address = 0;
	}
	*ip = ip_address;
	Free_read_acinfo_xml(&ahead);
	return 0;
}

int get_ipv6_by_active_instance(char *instance,char  *ipv6)
{
	char  ip_address[128] = {0};
	memset(ip_address,0,sizeof(ip_address));
	char *ins_id  = NULL;
	ins_id = instance;
	struct acbackup_st ahead;
	struct netipbk_st *aq = NULL;
	int confnum = 0;
	int retu = 0;
	char *p = NULL;
	memset(&ahead,0,sizeof(struct acbackup_st));
	retu = read_acinfo_xml(&ahead,&confnum);
	if(0 == retu)
	{
		aq = ahead.netipst_ipv6.next;
		while(aq != NULL)
		{
			p = strtok(aq->netip, " ");
			if(strcmp(p,ins_id) == 0)
			{
				p = strtok(NULL, " "); 
				if(p)
				{
					memset(ip_address,0,sizeof(ip_address));
					strncpy(ip_address,p,sizeof(ip_address)-1);
				}
				break;
			}
			else
			{
				aq = aq->next;
				p = NULL;
			}
		}
	}
	else
	{
		memset(ip_address,0,sizeof(ip_address));
	}
	strcpy(ipv6,ip_address);
	//*ipv6=ip_address;
	Free_read_acinfo_xml(&ahead);
	return 0;
}



int mod_insbk_xmlnode(char *xmlpath,char *node_name,char *slotid,char *netip)
{
	xmlDocPtr pdoc = NULL;
	xmlNodePtr pcurnode = NULL;
	xmlNodePtr design_node = NULL;
	pdoc = xmlReadFile(xmlpath,"utf-8",256);  

	if(NULL == pdoc)
	{
		return 1;
	}

	int if_second_flag = 0;
	char sidstr[10] = {0};
	char nipstr[32] = {0};
	char newc[50] = {0};
	pcurnode = xmlDocGetRootElement(pdoc); 
	design_node=pcurnode;
	pcurnode = pcurnode->xmlChildrenNode; 
	
	while (NULL != pcurnode)
	{		
		xmlChar *value=NULL;
		if (!xmlStrcmp(pcurnode->name, BAD_CAST node_name))   //找到 status 这个 root 下一级子节点             
		{
			memset(sidstr,0,sizeof(sidstr));
			memset(nipstr,0,sizeof(nipstr));
			memset(newc,0,sizeof(newc));
			snprintf(newc,sizeof(newc)-1,"%s %s",slotid,netip);
			value=xmlNodeGetContent(pcurnode);
			sscanf(value,"%s %s",sidstr,nipstr);
			if(0 == strcmp(sidstr,slotid))
			{
				xmlNodeSetContent(pcurnode, BAD_CAST  newc); 
				if_second_flag = 1;
				xmlFree(value);
				break;
			}
			else
			{
				if_second_flag = 0;
			}
			xmlFree(value);
		}  
		pcurnode = pcurnode->next; 
	}	  
	/*如果为空或者是没有此二级节点，都要进行创建,现在是创建不成功
	pcurnode 子节点的集合*/
	if(if_second_flag != 1)
	{
		xmlNodePtr node = xmlNewNode(NULL,BAD_CAST node_name);    
		xmlAddChild(design_node,node);
		memset(newc,0,sizeof(newc));
		snprintf(newc,sizeof(newc)-1,"%s %s",slotid,netip);
		xmlNodePtr content1 = xmlNewText(BAD_CAST newc);
		xmlAddChild(node,content1);
		design_node=NULL;
	}
	xmlSaveFile(xmlpath,pdoc);  
	xmlFreeDoc(pdoc);
	return if_second_flag;
}

