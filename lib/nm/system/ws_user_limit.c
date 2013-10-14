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
* ws_user_limit.c
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
#include "ws_user_limit.h"
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpathInternals.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
/*#include "cgic.h"*/


/***************************************************************
写入信息到conf文件中去
****************************************************************/
int write_limit_config( ST_LIMIT_ALL *sysall, char *file_path) 
{

	FILE * fp;	
	char File_content[10000],des_name[50];
	memset(File_content,0,10000);
	memset(des_name,0,50);  

	char state[50];
	memset(state,0,50);

	int i;

	if( NULL == file_path)
	{
		return -1 ;  //文件名为空
	}
	////////////打开文件//////////////////////////////
	if(( fp = fopen(file_path,"w+"))==NULL)
	return -2;	
	///////////////////////////////////////////////////////////
	//循环读取结构体中内容来写入到文件中去  	

	for(i=0;i<sysall->l_num;i++)
	{
		memset(des_name,0,50);

		//对on/off状态做处理
		memset(state,0,50);
		if(strcmp(sysall->limit_info[i].state,"on")==0)
		strcpy(state,"");
		if(strcmp(sysall->limit_info[i].state,"off")==0)
		strcpy(state,"#");

		sprintf(des_name , "%s",state);

		strcat(File_content,des_name);	
		strcat(File_content," ");

		memset(des_name,0,50);
		sprintf(des_name , "%s",sysall->limit_info[i].name);

		strcat(File_content,des_name);	
		strcat(File_content," ");

		memset(des_name,0,50);
		sprintf(des_name , "%s",sysall->limit_info[i].ruler);

		strcat(File_content,des_name);	
		strcat(File_content," ");

		memset(des_name,0,50);
		sprintf(des_name , "%s",sysall->limit_info[i].device);

		strcat(File_content,des_name);	
		strcat(File_content," ");

		memset(des_name,0,50);
		sprintf(des_name , "%s\n",sysall->limit_info[i].number);

		strcat(File_content,des_name);	


	}
	///////////////////////////////////////////////////////////		

	fwrite(File_content,strlen(File_content),1,fp);	
	fclose(fp);
	return 0;
}

/*读取文件信息函数,name是xml文件的路径，结构体中存放的是xml中的数据*/
int read_limit_xml(char * name, ST_LIMIT_ALL *sysall)
{
	xmlDocPtr pdoc = NULL;

	xmlNodePtr pcurnode = NULL;
	char *psfilename;

	psfilename = name;

	memset(sysall, 0, sizeof(ST_LIMIT_ALL) );

	pdoc = xmlReadFile(psfilename,"utf-8",256);  //解析文件

	if(NULL == pdoc)
	{
		return 1;
	}  
	pcurnode = xmlDocGetRootElement(pdoc);  //得到根节点   

	xmlChar *value = NULL;
	xmlChar *content;
	int i=0;
	pcurnode=pcurnode->xmlChildrenNode;  //得到子节点集合,函数中有了

	while (pcurnode != NULL)   //遍历子节点集合，找出所需的，
	{     



		////////////////////////////////////////////////////////////////////////////////

		//des  的情况
		if ((!xmlStrcmp(pcurnode->name, BAD_CAST LIMIT_NODE)))
		{    

			xmlNodePtr testnode;

			testnode=pcurnode;

			testnode=testnode->children;
			while(testnode !=NULL)
			{	 

				//node下有state节点的
				if ((!xmlStrcmp(testnode->name, BAD_CAST  LIMIT_NODE_STATE)))   //有state节点的
				{
					value = xmlNodeGetContent(testnode);	
					strcpy(sysall->limit_info[i].state,(char *)value);	
					xmlFree(value);
				}

				//node下有name节点的
				if ((!xmlStrcmp(testnode->name, BAD_CAST  LIMIT_NODE_NAME)))
				{
					content = xmlNodeGetContent(testnode);		 
					strcpy(sysall->limit_info[i].name,(char *)value);	
					xmlFree(content);
				}
				//node下有ruler节点的
				if ((!xmlStrcmp(testnode->name, BAD_CAST  LIMIT_NODE_RULER)))
				{
					content = xmlNodeGetContent(testnode);		 
					strcpy(sysall->limit_info[i].ruler,(char *)value);	
					xmlFree(content);
				}
				//node下有device节点的
				if ((!xmlStrcmp(testnode->name, BAD_CAST  LIMIT_NODE_DEVICE)))
				{
					content = xmlNodeGetContent(testnode);		 
					strcpy(sysall->limit_info[i].device,(char *)value);	
					xmlFree(content);
				}
				//node下有number节点的
				if ((!xmlStrcmp(testnode->name, BAD_CAST  LIMIT_NODE_NUMBER)))
				{
					content = xmlNodeGetContent(testnode);		 
					strcpy(sysall->limit_info[i].number,(char *)value);	
					xmlFree(content);
				}


				testnode = testnode->next;

			}	    
			i++;
		}  



		///////////////////////////////////////////////////////////////////////////////

		pcurnode = pcurnode->next;
	} 
	sysall->l_num=i;
	xmlFreeDoc(pdoc);
	xmlCleanupParser();
	return 0;
}


/*更新一个节点内容 处理下级节点的内容*/
int mod_limit_node(char * fpath,char * node_name,char *attribute,char *ruler,ST_LIMIT_INFO  newc)
{
	xmlDocPtr pdoc = NULL;

	xmlNodePtr pcurnode = NULL;
	char *psfilename;

	psfilename = fpath;

	pdoc = xmlReadFile(psfilename,"utf-8",256);  //解析文件

	if(NULL == pdoc)
	{
		return 1;
	}

	pcurnode = xmlDocGetRootElement(pdoc);  //得到根节点    

	pcurnode = pcurnode->xmlChildrenNode;  //得到子节点集合

	xmlNodePtr propNodePtr = pcurnode;

	while (NULL != pcurnode)
	{			

		if (!xmlStrcmp(pcurnode->name, BAD_CAST node_name))   //找到 des 这个 root 下一级子节点             
		{

			if (xmlHasProp(pcurnode,BAD_CAST attribute))
			{     
				propNodePtr = pcurnode;	

				//查找属性
				xmlAttrPtr attrPtr = propNodePtr->properties;	

				while (attrPtr != NULL)

				{      

					if (!xmlStrcmp(attrPtr->name, BAD_CAST attribute))

					{      		

						xmlChar* szAttr = xmlGetProp(propNodePtr,BAD_CAST attribute);  //取出属性值     

						if(!xmlStrcmp(szAttr,BAD_CAST ruler))

						{		
							//此处来处理下一级的节点
							xmlNodePtr childPtr = propNodePtr; 
							childPtr=childPtr->children;
							xmlChar *value;

							while(childPtr !=NULL)
							{	 

								///////////////////	   
								if ((!xmlStrcmp(childPtr->name, BAD_CAST  LIMIT_NODE_STATE)))
								{
									value = xmlNodeGetContent(childPtr);         	
									xmlNodeSetContent(childPtr, BAD_CAST  newc.state); 
									xmlFree(value);
								}
								///////////////////
								if ((!xmlStrcmp(childPtr->name, BAD_CAST  LIMIT_NODE_STATE)))
								{
									value = xmlNodeGetContent(childPtr);         	
									xmlNodeSetContent(childPtr, BAD_CAST  newc.state); 
									xmlFree(value);
								}
								///////////////////
								if ((!xmlStrcmp(childPtr->name, BAD_CAST  LIMIT_NODE_NAME)))
								{
									value = xmlNodeGetContent(childPtr);         	
									xmlNodeSetContent(childPtr, BAD_CAST  newc.name); 
									xmlFree(value);
								}
								///////////////////
								if ((!xmlStrcmp(childPtr->name, BAD_CAST  LIMIT_NODE_RULER)))
								{
									value = xmlNodeGetContent(childPtr);         	
									xmlNodeSetContent(childPtr, BAD_CAST  newc.ruler); 
									xmlFree(value);
								}
								///////////////////
								if ((!xmlStrcmp(childPtr->name, BAD_CAST  LIMIT_NODE_DEVICE)))
								{
									value = xmlNodeGetContent(childPtr);         	
									xmlNodeSetContent(childPtr, BAD_CAST  newc.device); 
									xmlFree(value);
								}
								///////////////////
								if ((!xmlStrcmp(childPtr->name, BAD_CAST  LIMIT_NODE_NUMBER)))
								{
									value = xmlNodeGetContent(childPtr);         	
									xmlNodeSetContent(childPtr, BAD_CAST  newc.number); 
									xmlFree(value);
								}
								///////////////////


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
	xmlSaveFile(fpath,pdoc);  //保存xml文件
	return 0;
}

/*指定属性指定节点，存储值到结构体中去 */

int find_limit_node(char * fpath,char * node_name,char *attribute,char *ruler, ST_LIMIT_INFO *logkey)
{
	xmlDocPtr pdoc = NULL;

	xmlNodePtr pcurnode = NULL;
	char *psfilename;

	int flag=0;

	psfilename = fpath;

	pdoc = xmlReadFile(psfilename,"utf-8",XML_PARSE_RECOVER);  //解析文件

	if(NULL == pdoc)
	{
		return 1;
	}

	pcurnode = xmlDocGetRootElement(pdoc);  //得到根节点    

	pcurnode = pcurnode->xmlChildrenNode;  //得到子节点集合

	xmlNodePtr propNodePtr = pcurnode;

	while (NULL != pcurnode)
	{			

		if (!xmlStrcmp(pcurnode->name, BAD_CAST node_name))   //找到 des 这个 root 下一级子节点             
		{

			if (xmlHasProp(pcurnode,BAD_CAST attribute))
			{     
				propNodePtr = pcurnode;	

				//查找属性
				xmlAttrPtr attrPtr = propNodePtr->properties;	

				while (attrPtr != NULL)  //如果属性值不为空

				{      

					if (!xmlStrcmp(attrPtr->name, BAD_CAST attribute))

					{      		

						xmlChar* szAttr = xmlGetProp(propNodePtr,BAD_CAST attribute);  //取出属性值     

						if(!xmlStrcmp(szAttr,BAD_CAST ruler))  //有此属性才开始执行	   	
						{	

							//此处来处理下一级的节点
							xmlNodePtr childPtr = propNodePtr; 
							childPtr=childPtr->children;
							xmlChar *value;

							while(childPtr !=NULL)
							{	 
								if ((!xmlStrcmp(childPtr->name, BAD_CAST LIMIT_NODE_STATE)))
								{
									value = xmlNodeGetContent(childPtr);        //取出 孙子节点的值  	
									strcpy(logkey->state,(char *)value);			 
									xmlFree(value);
									flag=4;
								}

								if ((!xmlStrcmp(childPtr->name, BAD_CAST LIMIT_NODE_NAME)))
								{
									value = xmlNodeGetContent(childPtr);        //取出 孙子节点的值  	
									strcpy(logkey->name,(char *)value);			 
									xmlFree(value);
								}


								if ((!xmlStrcmp(childPtr->name, BAD_CAST LIMIT_NODE_RULER)))
								{
									value = xmlNodeGetContent(childPtr);        //取出 孙子节点的值  	
									strcpy(logkey->ruler,(char *)value);			 
									xmlFree(value);
								}


								if ((!xmlStrcmp(childPtr->name, BAD_CAST LIMIT_NODE_DEVICE)))
								{
									value = xmlNodeGetContent(childPtr);        //取出 孙子节点的值  	
									strcpy(logkey->device,(char *)value);			 
									xmlFree(value);
								}

								if ((!xmlStrcmp(childPtr->name, BAD_CAST LIMIT_NODE_NUMBER)))
								{
									value = xmlNodeGetContent(childPtr);        //取出 孙子节点的值  	
									strcpy(logkey->number,(char *)value);			 
									xmlFree(value);
								}

								childPtr = childPtr->next;
							}
						}


						xmlFree(szAttr);
						flag=3;
					}
					attrPtr = attrPtr->next;
				}
				flag=2;
			}         
			flag=1;
		}        
		pcurnode = pcurnode->next; 

	}	  
	xmlSaveFile(fpath,pdoc);  //保存xml文件
	return flag;
}

