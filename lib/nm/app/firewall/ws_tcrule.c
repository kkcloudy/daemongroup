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
* ws_tcrule.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* shaojw@autelan.com
*
* DESCRIPTION:
* function for web
*
*
***************************************************************************/
#include "ws_tcrule.h"

#include <syslog.h>

#if 1
//#define SYSLOG( flag, formats... ) syslog( flag, formats )
#define SYSLOG( flag, formats... ) \
                {\
                    char buff[1024]="";\
                    char cmd[1024]="";\
                    snprintf( buff, sizeof(buff)-1, formats );\
                    snprintf( cmd, sizeof(cmd)-1, "/usr/bin/logger -u /dev/log -p local0.notice -t \"traffic:\" \"%s\"", buff );\
                    system( cmd );\
                }
#define LIMITE512K_INFO	"Interface %s traffic down to 512k"
#define LIMITE1M_INFO		"Interface %s traffic down to 1M"                
static int str2int( char *str )
{
	int ret;

	sscanf( str, "%d", &ret );

	return ret;
}


#else
#define SYSLOG( flag, formats... )
#endif







#if 0
static void showRule( PTCRule ptcRule )
{
	if( NULL != ptcRule )
	{
		printf( ptcRule->name );
		printf( "\n" );
		printf( ptcRule->comment );
		printf( "\n" );
		printf( ptcRule->interface );
		printf( "\n" );
		printf( ptcRule->protocol );
		printf( "\n" );
		printf( ptcRule->p2p_detail );
		printf( "\n" );
		printf( ptcRule->limit_speed );
		printf( "\n" );		

		printf( ptcRule->uplink_speed );
		printf( ptcRule->downlink_speed );
		printf( ptcRule->p2p_uplink_speed );
		printf( ptcRule->p2p_downlink_speed );
	
		printf( "time begin %d\n", ptcRule->time_begin );
		printf( "time end %d\n", ptcRule->time_end );		
	}
}

int cgiMain()
{
	PTCRule ptcRule;
	PTCRule ptcRule2;
	printf("xxxxxxxxxxxxxxxxxxxxxxxxx\n\n");
	
	ptcRule = tcNewRule();
	
	if( NULL == ptcRule )
	{
		printf( "create new rule err!\n\n" );
		return -1;
	}

	ptcRule2 = tcNewRule();
	
	if( NULL == ptcRule2 )
	{
		printf( "create new rule err!\n\n" );
		free( ptcRule2 );
		return -1;
	}
	
	ptcRule->name="test";
	ptcRule->enable=1;
	ptcRule->comment="abc123";
	ptcRule->interface="eth0";
	ptcRule->protocol="tcp";
	ptcRule->useP2P = 1;
	ptcRule->p2p_detail = "xunlei";
	ptcRule->limit_speed = "1234";
	
	ptcRule->uplink_speed = "123";
	ptcRule->downlink_speed = "234";
	ptcRule->p2p_uplink_speed = "123";
	ptcRule->p2p_downlink_speed = "234";
	
	ptcRule->time_begin = 4;
	ptcRule->time_end = 8;
	
	ptcRule2->name="test22222";
	ptcRule2->enable=1;
	ptcRule2->comment="abc1232222";
	ptcRule2->interface="eth02222";
	ptcRule2->protocol="tcp2222";
	ptcRule2->useP2P = 1;
	ptcRule2->p2p_detail = "xunle2222i";
	ptcRule2->limit_speed = "12222234";
	       
	ptcRule2->uplink_speed = "12222222223";
	ptcRule2->downlink_speed = "23222224";
	ptcRule2->p2p_uplink_speed = "12222223";
	ptcRule2->p2p_downlink_speed = "23222224";
	       
	ptcRule2->time_begin = 22;
	ptcRule2->time_end = 32;	
	
	ptcRule->next = ptcRule2;
	tcSaveDoc( "abc.xml", ptcRule );
	
	free( ptcRule );
	free( ptcRule2 );
	
	ptcRule = tcParseDoc( TCRULES_XML_FILE );
	
	showRule( ptcRule );
	printf("xxxxxxxxxxxxxxxxxxxxxxxxx\n\n");
	showRule( ptcRule->next );
	printf("xxxxxxxxxxxxxxxxxxxxxxxxx\n\n");
	return 0;
}
#endif


int if_tcrule_file(char * fpath)
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


//创建一条规则
PTCRule	tcNewRule()
{
	PTCRule ptcRule;
	ptcRule = (PTCRule)malloc( sizeof(TCRule) );
	
	if( NULL == ptcRule )
		return NULL;
	
	memset( ptcRule, 0, sizeof(TCRule) );
	
	return ptcRule;
}
 
//释放一条规则,要释放下面所有的指针
void tcFreeRule(PTCRule ptcRule)
{
    
	if( NULL != ptcRule->name )
		free( ptcRule->name );
	
	if( NULL != ptcRule->comment )
		free( ptcRule->comment );
	
	if( NULL != ptcRule->interface )
		free( ptcRule->interface );
		
	if( NULL != ptcRule->up_interface )
		free( ptcRule->up_interface );	
		
	if( NULL != ptcRule->protocol )
		free( ptcRule->protocol );
	
	if( NULL != ptcRule->p2p_detail )
		free( ptcRule->p2p_detail );

	if( NULL != ptcRule->limit_speed )
		free( ptcRule->limit_speed );
		
	if( NULL != ptcRule->uplink_speed )
		free( ptcRule->uplink_speed );

	if( NULL != ptcRule->downlink_speed )
		free( ptcRule->downlink_speed );

	if( NULL != ptcRule->p2p_uplink_speed )
		free( ptcRule->p2p_uplink_speed );

	if( NULL != ptcRule->p2p_downlink_speed )
		free( ptcRule->p2p_downlink_speed );

	if( NULL != ptcRule->addr_begin )
		free( ptcRule->addr_begin );
		
	if( NULL != ptcRule->addr_end )
		free( ptcRule->addr_end );
	
	if( NULL != ptcRule->mode )
		free( ptcRule->mode );
	
	if( NULL != ptcRule->addrtype )
		free( ptcRule->addrtype );	

#if TRANS_TO_CMD		//
	if( NULL != ptcRule->cmd_mark )
		free( ptcRule->cmd_mark );
		
	if( NULL != ptcRule->cmd_class )
		free( ptcRule->cmd_class );
			
	if( NULL != ptcRule->cmd_filter )
		free( ptcRule->cmd_filter );			
#endif
	
	free( ptcRule );
	
	return;
}

//释放规则链表
void tcFreeList( TCRule *root )
{
	PTCRule temp;
	
	while( NULL != root )
	{
		temp = root;
		root = root->next;
		tcFreeRule( temp );
	}
	
	return;
}

static void 
tcFreeRuleData(PTCRule ptcRule) {

	if( NULL != ptcRule->name )
		free( ptcRule->name );
	
	if( NULL != ptcRule->comment )
		free( ptcRule->comment );
	
	if( NULL != ptcRule->interface )
		free( ptcRule->interface );
		
	if( NULL != ptcRule->up_interface )
		free( ptcRule->up_interface );	
		
	if( NULL != ptcRule->protocol )
		free( ptcRule->protocol );
	
	if( NULL != ptcRule->p2p_detail )
		free( ptcRule->p2p_detail );

	if( NULL != ptcRule->limit_speed )
		free( ptcRule->limit_speed );
		
	if( NULL != ptcRule->uplink_speed )
		free( ptcRule->uplink_speed );

	if( NULL != ptcRule->downlink_speed )
		free( ptcRule->downlink_speed );

	if( NULL != ptcRule->p2p_uplink_speed )
		free( ptcRule->p2p_uplink_speed );

	if( NULL != ptcRule->p2p_downlink_speed )
		free( ptcRule->p2p_downlink_speed );
		
	if( NULL != ptcRule->addr_begin )
		free( ptcRule->addr_begin );
		
	if( NULL != ptcRule->addr_end )
		free( ptcRule->addr_end );
	
	if( NULL != ptcRule->mode )
		free( ptcRule->mode );
	
	if( NULL != ptcRule->addrtype )
		free( ptcRule->addrtype );	

#if TRANS_TO_CMD
	if( NULL != ptcRule->cmd_mark )
		free( ptcRule->cmd_mark );
		
	if( NULL != ptcRule->cmd_class )
		free( ptcRule->cmd_class );
			
	if( NULL != ptcRule->cmd_filter )
		free( ptcRule->cmd_filter );			
#endif

	return;
}

void 
tcFreeArray(TCRule **array, unsigned int count) {
    if(NULL == array || NULL == *array || 0 == count) {
        return ;
    }

    TCRule *temp_array = *array;
    int i = 0;
    for(i = 0; i < count; i++) {
        tcFreeRuleData(&temp_array[i]);
    }
    free(temp_array);
    *array = NULL;
    
    return ;
}

//添加一个规则到xml节点
static int addTCRuleToNode( xmlNodePtr root_node, PTCRule ptcRule )
{
	xmlNodePtr rule_node = NULL;
	char content[1024];
	
	if(!root_node)
		return -1;
	
	rule_node = xmlNewNode(NULL, BAD_CAST "tcRule");
	xmlAddChild( root_node, rule_node );

	xmlNewChild( rule_node, NULL, BAD_CAST "interface", BAD_CAST ptcRule->interface );
	xmlNewChild( rule_node, NULL, BAD_CAST "up_interface", BAD_CAST ptcRule->up_interface );
	//sprintf(content,"%d" ,ptcRule->name);
	xmlNewChild( rule_node, NULL, BAD_CAST "name", BAD_CAST ptcRule->name );

	sprintf(content,"%d" ,ptcRule->ruleIndex);
	xmlNewChild( rule_node, NULL, BAD_CAST "ruleIndex", BAD_CAST content );	
	
	sprintf(content,"%d" ,ptcRule->enable);
	xmlNewChild( rule_node, NULL, BAD_CAST "enable", BAD_CAST content );
	xmlNewChild( rule_node, NULL, BAD_CAST "comment", BAD_CAST ptcRule->comment );
	
	
	xmlNewChild( rule_node, NULL, BAD_CAST "protocol", BAD_CAST ptcRule->protocol );
	xmlNewChild( rule_node, NULL, BAD_CAST "p2p_detail", BAD_CAST ptcRule->p2p_detail );

	sprintf(content,"%d" ,ptcRule->useP2P);
	xmlNewChild( rule_node, NULL, BAD_CAST "useP2P", BAD_CAST content );
	
	
	
	xmlNewChild( rule_node, NULL, BAD_CAST "limit_speed", BAD_CAST ptcRule->limit_speed );
	
	xmlNewChild( rule_node, NULL, BAD_CAST "uplink_speed", BAD_CAST ptcRule->uplink_speed );
	xmlNewChild( rule_node, NULL, BAD_CAST "downlink_speed", BAD_CAST ptcRule->downlink_speed );
	xmlNewChild( rule_node, NULL, BAD_CAST "p2p_uplink_speed", BAD_CAST ptcRule->p2p_uplink_speed );
	xmlNewChild( rule_node, NULL, BAD_CAST "p2p_downlink_speed", BAD_CAST ptcRule->p2p_downlink_speed );
	
	sprintf(content,"%d" ,ptcRule->time_begin);
	xmlNewChild( rule_node, NULL, BAD_CAST "time_begin", BAD_CAST content );
	sprintf(content,"%d" ,ptcRule->time_end);
	xmlNewChild( rule_node, NULL, BAD_CAST "time_end", BAD_CAST content );
	

	xmlNewChild( rule_node, NULL, BAD_CAST "addrtype", BAD_CAST ptcRule->addrtype );
	xmlNewChild( rule_node, NULL, BAD_CAST "addr_begin", BAD_CAST ptcRule->addr_begin );
	xmlNewChild( rule_node, NULL, BAD_CAST "addr_end", BAD_CAST ptcRule->addr_end );
	xmlNewChild( rule_node, NULL, BAD_CAST "mode", BAD_CAST ptcRule->mode );
	

#if TRANS_TO_CMD	
	xmlNewChild( rule_node, NULL, BAD_CAST "cmd_mark", BAD_CAST ptcRule->cmd_mark );
	xmlNewChild( rule_node, NULL, BAD_CAST "cmd_class", BAD_CAST ptcRule->cmd_class );
	xmlNewChild( rule_node, NULL, BAD_CAST "cmd_filter", BAD_CAST ptcRule->cmd_filter );	
#endif	
	return 0;
}



//存储规则文件
int tcSaveDoc( char *path, TCRule *root )
{
	char command[256];
	xmlDocPtr doc = NULL;			/* document pointer */
	xmlNodePtr root_node = NULL;/* node pointers */
	PTCRule	ptcRuleTemp;
	int i = 0;
	
	doc = xmlNewDoc(BAD_CAST "1.0");
	root_node = xmlNewNode(NULL, BAD_CAST "root");
	xmlDocSetRootElement(doc, root_node);
	
	for( ptcRuleTemp = root; ptcRuleTemp != NULL; ptcRuleTemp = ptcRuleTemp->next )
	{
		ptcRuleTemp->ruleIndex = i;
		i++;
		addTCRuleToNode( root_node, ptcRuleTemp );
	}
	
	xmlSaveFormatFileEnc(path, doc, "UTF-8", 1);
	/*free the document */
	xmlFreeDoc(doc);
	xmlCleanupParser();
	xmlMemoryDump();	  //debug memory for regression tests

	memset(command,0,sizeof(command));
	sprintf(command,"sudo chmod 777 %s",path);
	system(command);
	
	return 0;
}




//将xml节点解析为规则
static PTCRule tcParseXmlNode( xmlNodePtr rule_node )
{
	PTCRule ptcRule;
	xmlNode *node = NULL;
	xmlChar *content = NULL;
	
	if( NULL == rule_node )
	{
		return NULL;	
	}
	
	ptcRule = tcNewRule( );
	if( NULL == ptcRule  )
		return NULL;
#if 1
	for(node = rule_node->children; node; node = node->next)
	{
		if (node->type != XML_ELEMENT_NODE) 
			continue;
		
		content = xmlNodeGetContent(node);
		if(strlen((char *)content) == 0)
		{
			xmlFree(content);
			content = NULL;
			continue;
		}
		else if( content == NULL )
			continue;
		
		//printf( "content = %s<br />", content );
		//printf( "node->name = %s<br />", (char *)node->name );
		if( strcmp((char *)node->name, "name" ) ==0)
		{
			if( NULL != ptcRule->name )
				free( ptcRule->name );
			ptcRule->name = (char *)malloc( strlen( (char*)content )+2 );
			if( NULL == ptcRule->name )
				continue;
			strcpy( ptcRule->name, (char *)content );
		}
		else if( strcmp((char *)node->name, "enable" ) ==0)
			sscanf( (char *)content, "%d", &(ptcRule->enable) );
		else if( strcmp((char *)node->name, "comment" ) == 0 )
		{
			if( NULL != ptcRule->comment )
				free( ptcRule->comment );
			ptcRule->comment = (char *)malloc( strlen( (char*)content )+2 );
			if( NULL == ptcRule->comment )
				continue;
			strcpy( ptcRule->comment, (char *)content );
		}
		else if( strcmp((char *)node->name, "interface" ) ==0 )
		{
			if( NULL != ptcRule->interface )
				free( ptcRule->interface );
			ptcRule->interface = (char *)malloc( strlen( (char*)content )+2 );
			if( NULL == ptcRule->interface )
				continue;			
			strcpy( ptcRule->interface, (char *)content );
		}
		else if( strcmp((char *)node->name, "up_interface" ) ==0 )
		{
			if( NULL != ptcRule->up_interface )
				free( ptcRule->up_interface );
			ptcRule->up_interface = (char *)malloc( strlen( (char*)content )+2 );
			if( NULL == ptcRule->up_interface )
				continue;			
			strcpy( ptcRule->up_interface, (char *)content );
		}
		else if( strcmp((char *)node->name, "protocol" ) ==0 )
		{
			if( NULL != ptcRule->protocol )
				free( ptcRule->protocol );
			ptcRule->protocol = (char *)malloc( strlen( (char*)content )+2 );
			if( NULL == ptcRule->protocol )
				continue;				
			strcpy( ptcRule->protocol, (char *)content );
		}
		else if( strcmp((char *)node->name, "useP2P" ) ==0)
			sscanf( (char *)content, "%d", &(ptcRule->useP2P) );
		else if( strcmp((char *)node->name, "p2p_detail" ) ==0)
		{
			if( NULL != ptcRule->p2p_detail )
				free( ptcRule->p2p_detail );
			ptcRule->p2p_detail = (char *)malloc( strlen( (char*)content )+2 );
			if( NULL == ptcRule->p2p_detail )
				continue;				
			strcpy( ptcRule->p2p_detail, (char *)content );
		}
		else if( strcmp((char *)node->name, "limit_speed" ) ==0)
		{
			if( NULL != ptcRule->limit_speed )
				free( ptcRule->limit_speed );
			ptcRule->limit_speed = (char *)malloc( strlen( (char*)content )+2 );
			if( NULL == ptcRule->limit_speed )
				continue;				
			strcpy( ptcRule->limit_speed, (char *)content );
		}		
		else if( strcmp( (char *)node->name, "uplink_speed" ) == 0 )
		{
			if( NULL != ptcRule->uplink_speed )
				free( ptcRule->uplink_speed );
			ptcRule->uplink_speed = (char *)malloc( strlen( (char*)content )+2 );
			if( NULL == ptcRule->uplink_speed )
				continue;				
			strcpy( ptcRule->uplink_speed, (char *)content );	
		}
		else if( strcmp( (char *)node->name, "downlink_speed" ) == 0 )
		{
			if( NULL != ptcRule->downlink_speed )
				free( ptcRule->downlink_speed );
			ptcRule->downlink_speed = (char *)malloc( strlen( (char*)content )+2 );
			if( NULL == ptcRule->downlink_speed )
				continue;				
			strcpy( ptcRule->downlink_speed, (char *)content );	
		}
		else if( strcmp( (char *)node->name, "p2p_uplink_speed" ) == 0 )
		{
			if( NULL != ptcRule->p2p_uplink_speed )
				free( ptcRule->p2p_uplink_speed );
			ptcRule->p2p_uplink_speed = (char *)malloc( strlen( (char*)content )+2 );
			if( NULL == ptcRule->p2p_uplink_speed )
				continue;				
			strcpy( ptcRule->p2p_uplink_speed, (char *)content );				
		}
		else if( strcmp( (char *)node->name, "p2p_downlink_speed" ) == 0 )
		{
			if( NULL != ptcRule->p2p_downlink_speed )
				free( ptcRule->p2p_downlink_speed );
			ptcRule->p2p_downlink_speed = (char *)malloc( strlen( (char*)content )+2 );
			if( NULL == ptcRule->p2p_downlink_speed )
				continue;				
			strcpy( ptcRule->p2p_downlink_speed, (char *)content );				
		}
		else if( strcmp((char *)node->name, "time_begin" ) ==0)
			sscanf( (char *)content, "%d", &(ptcRule->time_begin) );
		else if( strcmp((char *)node->name, "time_end" ) ==0)
			sscanf( (char *)content, "%d", &(ptcRule->time_end) );
		else if( strcmp( (char *)node->name, "addrtype" ) == 0 )
		{
			if( NULL != ptcRule->addrtype )
				free( ptcRule->addrtype );
			ptcRule->addrtype = (char *)malloc( strlen( (char*)content )+2 );
			if( NULL == ptcRule->addrtype )
				continue;				
			strcpy( ptcRule->addrtype, (char *)content );				
		}
		else if( strcmp( (char *)node->name, "addr_begin" ) == 0 )
		{
			if( NULL != ptcRule->addr_begin )
				free( ptcRule->addr_begin );
			ptcRule->addr_begin = (char *)malloc( strlen( (char*)content )+2 );
			if( NULL == ptcRule->addr_begin )
				continue;				
			strcpy( ptcRule->addr_begin, (char *)content );				
		}
		else if( strcmp( (char *)node->name, "addr_end" ) == 0 )
		{
			if( NULL != ptcRule->addr_end )
				free( ptcRule->addr_end );
			ptcRule->addr_end = (char *)malloc( strlen( (char*)content )+2 );
			if( NULL == ptcRule->addr_end )
				continue;				
			strcpy( ptcRule->addr_end, (char *)content );
		}
		else if( strcmp( (char *)node->name, "mode" ) == 0 )
		{
			if( NULL != ptcRule->mode )
				free( ptcRule->mode );
			ptcRule->mode = (char *)malloc( strlen( (char*)content )+2 );
			if( NULL == ptcRule->mode )
				continue;				
			strcpy( ptcRule->mode, (char *)content );
		}		
	
#if TRANS_TO_CMD				
		else if( strcmp( (char *)node->name, "cmd_mark" ) == 0 )
		{
			if( NULL != ptcRule->cmd_mark )
				free( ptcRule->cmd_mark );
			ptcRule->cmd_mark = (char *)malloc( strlen( (char*)content )+2 );
			if( NULL == ptcRule->cmd_mark )
				continue;				
			strcpy( ptcRule->cmd_mark, (char *)content );				
		}
		else if( strcmp( (char *)node->name, "cmd_class" ) == 0 )
		{
			if( NULL != ptcRule->cmd_class )
				free( ptcRule->cmd_class );
			ptcRule->cmd_class = (char *)malloc( strlen( (char*)content )+2 );
			if( NULL == ptcRule->cmd_class )
				continue;				
			strcpy( ptcRule->cmd_class, (char *)content );				
		}
		else if( strcmp( (char *)node->name, "cmd_filter" ) == 0 )
		{
			if( NULL != ptcRule->cmd_filter )
				free( ptcRule->cmd_filter );
			ptcRule->cmd_filter = (char *)malloc( strlen( (char*)content )+2 );
			if( NULL == ptcRule->cmd_filter )
				continue;				
			strcpy( ptcRule->cmd_filter, (char *)content );				
		}
#endif		
		xmlFree(content);
		content = NULL;
	}
#else//测试用的
	
	ptcRule->interface = (char*)malloc(8);
	strcpy( ptcRule->interface, "eth1" );
	
	ptcRule->addrtype = (char*)malloc(32);
	strcpy( ptcRule->addrtype, "address" );
	
	ptcRule->addr_begin = (char*)malloc(32);
	strcpy( ptcRule->addr_begin, "192.168.1.3" );
	
	ptcRule->uplink_speed = (char*)malloc(32);
	strcpy( ptcRule->uplink_speed,"12" );
	
	ptcRule->downlink_speed = (char*)malloc(32);
	strcpy( ptcRule->downlink_speed,"34" );
	
#endif
	
	return ptcRule;
}

//解析规则文件
PTCRule tcParseDoc( char *path )
{
	xmlDoc *doc = NULL;
    xmlNode *root_element = NULL;
	xmlNode *rule_node = NULL;
	PTCRule ptcRuleRoot,ptcRuleLast;
	
	ptcRuleRoot = NULL;
	ptcRuleLast = NULL;

	
	/*parse the file and get the DOM */
	doc = xmlReadFile(path, NULL, 0);	
	if (doc == NULL) 
	{
		return NULL;
	}
	
	/*Get the root element node */
	
	root_element = xmlDocGetRootElement(doc);	
	
	for(rule_node = root_element->children; rule_node; rule_node = rule_node->next)
	{
		if(rule_node->type != XML_ELEMENT_NODE)
			continue;
		
		if( NULL == ptcRuleRoot )
		{
			ptcRuleRoot = tcParseXmlNode( rule_node );
			ptcRuleLast = ptcRuleRoot;
		}	
		else
		{
			ptcRuleLast->next =  tcParseXmlNode( rule_node );
			if( NULL != ptcRuleLast->next )
				ptcRuleLast = ptcRuleLast->next;
		}
	
	}
	
	xmlFreeDoc(doc);
	
	return ptcRuleRoot;
}

void tcrule_status_exist()
{
	char cmd[128] = {0};
	if(access(TCRULE_STATUS_FILE,0)!=0)
	{
		memset(cmd,0,128);
		sprintf(cmd,"sudo touch %s\n sudo chmod 666 %s\n echo \"start\" > %s",TCRULE_STATUS_FILE,TCRULE_STATUS_FILE,TCRULE_STATUS_FILE);
		system(cmd);
	}
	else
	{
		memset(cmd,0,128);
		sprintf(cmd,"sudo chmod 666 %s\n echo \"start\" > %s",TCRULE_STATUS_FILE,TCRULE_STATUS_FILE);
		system(cmd);
	}
}

//这个函数现在没有用
#if 0
int processRuleCmd( PTCRule ptcRule )
{
#if TRANS_TO_CMD	
	char *cmd;
	char ipRange[256]="";
//根据用户填写的信息，转换成命令，存放在xml文件中
	//命令的长度为CMD_LEN，超过后需要从新malloc,最多为CMD_LEN×2
	cmd = (char *)malloc(CMD_LEN);
	if( NULL == cmd )
	{
		goto error;	
	}
	memset( cmd, 0, CMD_LEN );
	
	if( NULL != ptcRule->cmd_mark )
	{
		free( ptcRule->cmd_mark );
	}
	ptcRule->cmd_mark = malloc( CMD_LEN );
	if( NULL == ptcRule->cmd_mark )
	{
		goto error;
	}
	memset( ptcRule->cmd_mark, 0, CMD_LEN );

	if( NULL != ptcRule->cmd_class )
	{
		free( ptcRule->cmd_class );
	}
	ptcRule->cmd_class = malloc( CMD_LEN );
	if( NULL == ptcRule->cmd_class )
	{
		goto error;
	}
	memset( ptcRule->cmd_class, 0, CMD_LEN );

	if( NULL != ptcRule->cmd_filter )
	{
		free( ptcRule->cmd_filter );
	}
	ptcRule->cmd_filter = malloc( CMD_LEN );
	if( NULL == ptcRule->cmd_filter )
	{
		goto error;
	}
	memset( ptcRule->cmd_filter, 0, CMD_LEN );
	//标记号的计算方法：应为一跳规则中，可能有普通上行，普通下行，p2p上行，p2p下行。共有4中，所以在标记的时候
	//使用以下规则：
	//n表示当前规则的index号，从0开始,  PER_RULE_RATE_NUM为  4，这样主要是为了防止以后有可能在一个规则中添加新的流量分类,前面空出一部分
	//第n条规则的普通上行包标记为:(n+1)*PER_RULE_RATE_NUM为
	//第n条规则的普通下行包标记为:(n+1)*PER_RULE_RATE_NUM为+1
	//第n条规则的普通下行包标记为:(n+1)*PER_RULE_RATE_NUM为+2
	//第n条规则的普通下行包标记为:(n+1)*PER_RULE_RATE_NUM为+3
	//........如果一跳规则中可能还有其他流量的话，标记编号可以调整  PER_RULE_RATE_NUM 的值，一直增加都   PER_RULE_RATE_NUM-1
	
	
	//根据填写的ip地址，将数据标记,标记号即为规则的编号，
#define IPTABLES_PATH	"/opt/bin/iptables"
#define TC_PATH		"/sbin/tc"
#define RULE_RATE_PACKAGE_MARK_UPLOAD_FORMAT	IPTABLES_PATH" -t mangle -A PREROUTING -s %s -j MARK --set-mark %d;"
#define RULE_RATE_PACKAGE_MARK_DOWNLOAD_FORMAT	IPTABLES_PATH" -t mangle -A PREROUTING -d %s -j MARK --set-mark %d;"
#define RULE_RATE_P2P_PACKAGE_MARK_UPLOAD_FORMAT	IPTABLES_PATH" -t mangle -A PREROUTING -p tcp -s %s -m --ipp2p -j MARK --set-mark %d;"\
IPTABLES_PATH" -t mangle -A PREROUING -s %s -p tcp -m mark --mark %d -j CONNMARK --save-mark;"

#define RULE_RATE_P2P_PACKAGE_MARK_DOWNLOAD_FORMAT	IPTABLES_PATH" -t mangle -A PREROUTING -p tcp -d %s -m --ipp2p -j MARK --set-mark %d;"\
IPTABLES_PATH" -t mangle -A PREROUING -d %s -p tcp -m mark --mark %d -j CONNMARK --save-mark;"


#define WAN_DEVICE	"eth0"//这个值需要确定确定
	//创建class，下面的classid后的%d也是用(n+1)*PER_RULE_RATE_NUM+x来标记的
#define RULE_RATE_CLASS_FORMAT	TC_PATH" class add dev %s parent 1:1 classid 1:1%d htb rate %skbit;"
	//创建filter
#define RULE_RATE_FILTER_FORMAT	TC_PATH" filter add dev %s parent 1:0 protocol ip prio 1 handle 1 fw classid 1:%d;"	

	
#define PER_RULE_RATE_NUM	4
	//上行
		//标记
	strcpy( ipRange, ptcRule->addr_begin );
	if( strcmp( ptcRule->addrtype, "addrtype" ) == 0 )
	{
		strcat( ipRange, "/" );
		strcpy( ipRange, ptcRule->addr_end );
	}
	sprintf( cmd, RULE_RATE_PACKAGE_MARK_UPLOAD_FORMAT, ipRange, (ptcRule->ruleIndex+1)*PER_RULE_RATE_NUM );
	strcat( ptcRule->cmd_mark, cmd );
		//创建class
	sprintf( cmd, RULE_RATE_CLASS_FORMAT, WAN_DEVICE, (ptcRule->ruleIndex+1)*PER_RULE_RATE_NUM, ptcRule->uplink_speed );
	strcat( ptcRule->cmd_class, cmd );
		//创建filter
	sprintf( cmd, RULE_RATE_FILTER_FORMAT, WAN_DEVICE, (ptcRule->ruleIndex+1)*PER_RULE_RATE_NUM );
	strcat( ptcRule->cmd_filter, cmd );	
	
	//下行
		//标记
	sprintf( cmd, RULE_RATE_PACKAGE_MARK_DOWNLOAD_FORMAT, ipRange, (ptcRule->ruleIndex+1)*PER_RULE_RATE_NUM+1 );
	strcat( ptcRule->cmd_mark, cmd );
		//创建class
	sprintf( cmd, RULE_RATE_CLASS_FORMAT, WAN_DEVICE, (ptcRule->ruleIndex+1)*PER_RULE_RATE_NUM+1, ptcRule->downlink_speed );
	strcat( ptcRule->cmd_class, cmd );
		//创建filter
	sprintf( cmd, RULE_RATE_FILTER_FORMAT, WAN_DEVICE, (ptcRule->ruleIndex+1)*PER_RULE_RATE_NUM+1 );
	strcat( ptcRule->cmd_filter, cmd );	
	
	
	if( 1 == ptcRule->useP2P )
	{
	//p2p上行
		//标记
		sprintf( cmd, RULE_RATE_P2P_PACKAGE_MARK_UPLOAD_FORMAT, ipRange, (ptcRule->ruleIndex+1)*PER_RULE_RATE_NUM+2, ipRange, (ptcRule->ruleIndex+1)*PER_RULE_RATE_NUM+2 );
		strcat( ptcRule->cmd_mark, cmd );
			//创建class
		sprintf( cmd, RULE_RATE_CLASS_FORMAT, WAN_DEVICE, (ptcRule->ruleIndex+1)*PER_RULE_RATE_NUM+2, ptcRule->p2p_uplink_speed );
		strcat( ptcRule->cmd_class, cmd );
			//创建filter
		sprintf( cmd, RULE_RATE_FILTER_FORMAT, WAN_DEVICE, (ptcRule->ruleIndex+1)*PER_RULE_RATE_NUM+2 );
		strcat( ptcRule->cmd_filter, cmd );	
		
		//p2p下行
			//标记
		sprintf( cmd, RULE_RATE_P2P_PACKAGE_MARK_DOWNLOAD_FORMAT, ipRange, (ptcRule->ruleIndex+1)*PER_RULE_RATE_NUM+3, ipRange, (ptcRule->ruleIndex+1)*PER_RULE_RATE_NUM+3 );
		strcat( ptcRule->cmd_mark, cmd );
			//创建class
		sprintf( cmd, RULE_RATE_CLASS_FORMAT, WAN_DEVICE, (ptcRule->ruleIndex+1)*PER_RULE_RATE_NUM+3, ptcRule->p2p_downlink_speed );
		strcat( ptcRule->cmd_class, cmd );
			//创建filter
		sprintf( cmd, RULE_RATE_FILTER_FORMAT, WAN_DEVICE, (ptcRule->ruleIndex+1)*PER_RULE_RATE_NUM+3 );
		strcat( ptcRule->cmd_filter, cmd );			
	}
#endif
	return 0;
#if TRANS_TO_CMD	
error:
	if( NULL != cmd )
		free( cmd );
#endif		
	return -1;
}
#endif




int getTcRuleNum( )
{
	int iRet = 0;
	PTCRule ptcRuleRoot = NULL;
	PTCRule ptcRuleTemp = NULL;
	
	ptcRuleRoot = tcParseDoc( TCRULES_XML_FILE );
	for( ptcRuleTemp = ptcRuleRoot; NULL != ptcRuleTemp; ptcRuleTemp = ptcRuleTemp->next, iRet++ );
	
	if( NULL != ptcRuleRoot )
		tcFreeList( ptcRuleRoot );
	
	return iRet;
}



/***************************************************************
*NAME:replaceRule
*USEAGE:替换一条规则，对应的上层操作是修改了该条规则
*Param: ptcRuleRoot -> 根节点
		ptcRuleNew  -> 要替换层的节点数据
		index		-> 被替换节点的索引号
*Return: 返回根节点
*Auther:shao jun wu
*Date:2008-11-17 14:04:17
*Modify:(include modifyer,for what reason, date)
****************************************************************/
PTCRule replaceRule( PTCRule ptcRuleRoot, PTCRule ptcRuleNew, int index )
{
	if( index < 0 || NULL == ptcRuleRoot )
	{
		if( NULL == ptcRuleRoot )
		{
		//说明当前还没有数据。
			ptcRuleRoot = ptcRuleNew;
		}
		else
		{
			PTCRule ptcRuleLast;
			for( ptcRuleLast = ptcRuleRoot; ptcRuleLast->next != NULL; ptcRuleLast = ptcRuleLast->next );
			ptcRuleLast->next = ptcRuleNew;
		}
	}
	else if( 0 == index )
	{
		ptcRuleNew->next = ptcRuleRoot->next;
		tcFreeRule( ptcRuleRoot );
		ptcRuleRoot = ptcRuleNew;
	}
	else
	{
		PTCRule ptcReplacePre;
		
		for(ptcReplacePre = ptcRuleRoot, index--; ((ptcReplacePre->next != NULL) && (index > 0)); ptcReplacePre = ptcReplacePre->next, index-- );
		if( ptcReplacePre->next != NULL )
		{
			ptcRuleNew->next = ptcReplacePre->next->next;
			tcFreeRule( ptcReplacePre->next );
		}
		
		ptcReplacePre->next = ptcRuleNew;
	}

#if 0	
	SYSLOG( LOG_USER|LOG_INFO, "Limit ", 
	                            ptcRuleNew->ruleIndex, 
	                            ptcRuleNew->addr_begin, 
	                            (NULL==ptcRuleNew->addr_end)?"":(ptcRuleNew->addr_end), 
	                            ptcRuleNew->uplink_speed, 
	                            ptcRuleNew->downlink_speed 
	                            );
//#else
	if( str2int(ptcRuleNew->downlink_speed) > 512 && str2int(ptcRuleNew->downlink_speed) <= 1024 )
	{
		SYSLOG( 0, LIMITE1M_INFO, ptcRuleNew->interface );
	}
	else if( str2int(ptcRuleNew->downlink_speed) <= 512 )
	{
		SYSLOG( 0, LIMITE512K_INFO, ptcRuleNew->interface );
	}
#endif	                            
	return ptcRuleRoot;

}

/***************************************************************
*NAME:insertRule
*USEAGE:插入一条规则，对应的上层操作是添加了一条规则。
		该函数和上面函数的逻辑运算很想，但为了保证函数的内聚性，并没有把这些相似的代码整合到一起。
		保证模块的  内聚性  和  相似代码的泛化   两点是矛盾的。关键是看哪个更复合你的需要～～，有什么原则么？保证设计的清晰、简单。
*Param: ptcRuleRoot -> 根节点
		ptcRuleNew  -> 要插入的节点数据
		index		-> 插入到的位置，如果<0,则表示插入到最后
*Return: 返回当前的根节点
*Auther:shao jun wu
*Date:2008-11-17 14:04:17
*Modify:(include modifyer,for what reason, date)
****************************************************************/
PTCRule insertRule( PTCRule ptcRuleRoot, PTCRule ptcRuleNew, int index )
{
	if( index < 0 || NULL == ptcRuleRoot )
	{
		if( NULL == ptcRuleRoot )
		{
		//说明当前还没有数据。
			ptcRuleRoot = ptcRuleNew;
		}
		else
		{
			PTCRule ptcRuleLast;
			
			for( ptcRuleLast = ptcRuleRoot; ptcRuleLast->next != NULL; ptcRuleLast = ptcRuleLast->next );
			
			ptcRuleLast->next = ptcRuleNew;
		}
	}
	else if( 0 == index )
	{
		ptcRuleNew->next = ptcRuleRoot;
		ptcRuleRoot = ptcRuleNew;
	}
	else
	{
		PTCRule ptcInsertPre;
		
		for(ptcInsertPre = ptcRuleRoot, index--; ((ptcInsertPre->next!=NULL) && (index > 0)); ptcInsertPre = ptcInsertPre->next, index-- );
		
		ptcRuleNew->next = ptcInsertPre->next;
		ptcInsertPre->next = ptcRuleNew;
	}
	
	if( str2int(ptcRuleNew->downlink_speed) > 512 && str2int(ptcRuleNew->downlink_speed) <= 1024 )
	{
		SYSLOG( 0, LIMITE1M_INFO, ptcRuleNew->interface );
	}
	else if( str2int(ptcRuleNew->downlink_speed) <= 512 )
	{
		SYSLOG( 0, LIMITE512K_INFO, ptcRuleNew->interface );
	}

	                            	
	return ptcRuleRoot;	
}


/***************************************************************
*NAME:deleteRule
*USEAGE:删除一条规则，对应的上层操作是删除该条规则
*Param: ptcRuleRoot -> 根节点
		index		-> 被删除节点的索引号
*Return: 返回根节点,因为如果要删除的节点是第一个，
         之前保存的root指针就不对了，所以需要返回根节点，
         调用函数的时候将其赋值给根节点的变量。
*Auther:shao jun wu
*Date:2008-11-17 14:05:19
*Modify:(include modifyer,for what reason, date)
****************************************************************/
PTCRule deleteRule( PTCRule ptcRuleRoot, int index )
{
	PTCRule ptcDeletePre = NULL,temp = NULL;
	
	if( index == 0 )
	{
		temp = ptcRuleRoot;
		ptcRuleRoot = ptcRuleRoot->next;
	}
	else
	{
		for(ptcDeletePre = ptcRuleRoot, index--; ((ptcDeletePre->next!=NULL) && (index > 0)); ptcDeletePre = ptcDeletePre->next, index-- );
		
		if( NULL != ptcDeletePre->next )
		{
			temp = ptcDeletePre->next;
			ptcDeletePre->next = ptcDeletePre->next->next;
		}
	}
	
	if(temp != NULL)
	tcFreeRule( temp );
	
	return ptcRuleRoot;
}

/***************************************************************
*NAME:changeRuleIndex
*USEAGE:删除一条规则，对应的上层操作是删除该条规则
*Param: ptcRuleRoot -> 根节点
		index		-> 被删除节点的索引号
*Return: 返回根节点
*Auther:shao jun wu
*Date:2008-11-17 14:05:19
*Modify:(include modifyer,for what reason, date)
****************************************************************/
PTCRule changeRuleIndex( PTCRule ptcRuleRoot, int iOldIndex, int iNewIndex )
{
    PTCRule ptcRuleChangeIndex;
    PTCRule ptcRuleChangeIndexPrev;
    
    if( NULL == ptcRuleRoot || iOldIndex == iNewIndex )
    {
        goto error;
    }
    
    if( 0 == iOldIndex )
    {
        ptcRuleChangeIndex = ptcRuleRoot;
        ptcRuleRoot = ptcRuleChangeIndex->next;
        
        //将抽出来的规则插入到链表中。
        for( ptcRuleChangeIndexPrev=ptcRuleRoot; (NULL!=ptcRuleChangeIndexPrev)&&(iNewIndex>1); ptcRuleChangeIndexPrev=ptcRuleChangeIndexPrev->next,iNewIndex-- );
        
        ptcRuleChangeIndex->next = ptcRuleChangeIndexPrev->next;
        ptcRuleChangeIndexPrev->next = ptcRuleChangeIndex;        
    }
    else if( 0 == iNewIndex )
    {
         //将要改变位置的规则从链表中取出来。
        for( ptcRuleChangeIndexPrev=ptcRuleRoot; (NULL!=ptcRuleChangeIndexPrev)&&(iOldIndex>1); ptcRuleChangeIndexPrev=ptcRuleChangeIndexPrev->next,iOldIndex-- );
                        
        ptcRuleChangeIndex = ptcRuleChangeIndexPrev->next;
        ptcRuleChangeIndexPrev->next = ptcRuleChangeIndex->next;
        
        ptcRuleChangeIndex->next=ptcRuleRoot;
        ptcRuleRoot = ptcRuleChangeIndex;
    }
    else
    {   
        //将要改变位置的规则从链表中取出来。
        for( ptcRuleChangeIndexPrev=ptcRuleRoot; (NULL!=ptcRuleChangeIndexPrev)&&(iOldIndex>1); ptcRuleChangeIndexPrev=ptcRuleChangeIndexPrev->next,iOldIndex-- );
                        
        ptcRuleChangeIndex = ptcRuleChangeIndexPrev->next;
        ptcRuleChangeIndexPrev->next = ptcRuleChangeIndex->next;
        
        //将抽出来的规则插入到链表中。
        for( ptcRuleChangeIndexPrev=ptcRuleRoot; (NULL!=ptcRuleChangeIndexPrev)&&(iNewIndex>1); ptcRuleChangeIndexPrev=ptcRuleChangeIndexPrev->next,iNewIndex-- );
        
        ptcRuleChangeIndex->next = ptcRuleChangeIndexPrev->next;
        ptcRuleChangeIndexPrev->next = ptcRuleChangeIndex;
    }   
    
error:
        
    return ptcRuleRoot;
}

/***************************************************************
*NAME:tc_doAllRules
*USEAGE:将所有规则下达到内核
*Param: ptcRuleRoot -> 根节点
*Return: 	0 -> 成功
			others -> 失败
*Auther:shao jun wu
*Date:2008-11-17 14:06:27
*Modify:(include modifyer,for what reason, date)
****************************************************************/
int tc_doAllRules( PTCRule ptcRuleRoot )
{
	PTCRule temp;
	FILE *fp;
	int iRet=0;
	int i=0;
	
	//输出到文件
#define RULEINFO_PATH "/var/run/apache2/filter_con2.tmp"
	fp = fopen( RULEINFO_PATH, "w+" );
	if( NULL == fp )
	{
		return -4;
	}
//输出规则， interface，up_down_flag, ipaddr， mask， bandwidth， p2pstate, begintime,endtime
//每条规则对应底层4条规则。分别为普通下载，普通上传，p2p下载，p2p上传。
	for( temp = ptcRuleRoot; temp!=NULL; temp=temp->next )
	{
		unsigned int mask = 32;
		unsigned int downlink_speed=0;
		unsigned int uplink_speed=0;
		unsigned int p2p_downlink_speed=0;
		unsigned int p2p_uplink_speed=0;
		
		if( 0 == temp->enable )
			continue;
		//marsk
		if( NULL != temp->addr_end )// addr_end  实际是掩码
		{
			unsigned int a,b,c,d,e;
			
			sscanf( temp->addr_end, "%d.%d.%d.%d", &a, &b, &c, &d );
			e = a*256*256*256 + b*256*256 + c*256 + d;
			while( (e/2*2 == e) && (e > 0) )
			{
				mask--;
				e = e/2;
			}
		}

        if(temp->uplink_speed && temp->downlink_speed) {
    		sscanf( temp->uplink_speed, "%d", &uplink_speed );
    		sscanf( temp->downlink_speed, "%d", &downlink_speed );
        }
        
        if(temp->ruleIndex == temp->offset.ruleIndex) {
            int uplink_speed_revise = uplink_speed * 8 + temp->offset.uplink_offset;
            int downlink_speed_revise = downlink_speed * 8 + temp->offset.downlink_offset;

            if(uplink_speed_revise < 0) {
                uplink_speed = 0;
            }
            else if(uplink_speed_revise > (9999 * 8)) {
                uplink_speed = 9999 * 8;
            }
            else {
                uplink_speed = uplink_speed_revise;
            }
            
            if(downlink_speed_revise < 0) {
                downlink_speed = 0;
            }
            else if(downlink_speed_revise > (9999 * 8)) {
                downlink_speed = 9999 * 8;
            }
            else {
                downlink_speed = downlink_speed_revise;
            }
        }
        else {
            uplink_speed *= 8; 
            downlink_speed *= 8; 
        }
		
		if( NULL != temp->p2p_uplink_speed )
		{
			sscanf( temp->p2p_uplink_speed, "%d", &p2p_uplink_speed );
		}
		if( NULL != temp->p2p_downlink_speed )
		{
			sscanf( temp->p2p_downlink_speed, "%d", &p2p_downlink_speed );
		}			

        syslog(LOG_DEBUG, "index = %d, up interface %s uplink %d, down interface %s downlink %d\n", 
                            temp->ruleIndex, temp->up_interface, uplink_speed, temp->interface, downlink_speed);
	    
		//down
		fprintf( fp, "%20s/%4d/dst/%s/%u/%u/%d/%d/%d/%s\n",temp->interface,i, temp->addr_begin, mask, downlink_speed, 0, 0, 0,temp->mode?temp->mode:"share" );
		//up
		fprintf( fp, "%20s/%4d/src/%s/%u/%u/%d/%d/%d/%s\n",temp->up_interface,i, temp->addr_begin, mask, uplink_speed, 0, 0, 0, temp->mode?temp->mode:"share" );
		//p2p down
		fprintf( fp, "%20s/%4d/dst/%s/%u/%u/%d/%d/%d/%s\n",temp->interface, i,temp->addr_begin, mask, p2p_downlink_speed*8, temp->useP2P, 0, 0, temp->mode?temp->mode:"share" );
		//p2p up
		fprintf( fp, "%20s/%4d/src/%s/%u/%u/%d/%d/%d/%s\n",temp->up_interface,i, temp->addr_begin, mask, p2p_uplink_speed*8, temp->useP2P, 0, 0, temp->mode?temp->mode:"share" );
		
		i++;		
	}
	
	fclose( fp );
	
	//执行awk，生成shell
#if 1
#define	AWK_PATH "/opt/awk/filter_con.awk"
#define MODIFYSHELL_PERMITE "sudo chmod 777 "TCSHELL_PATH
//#define TCSHELL_PATH "/mnt/ip_addr/tc_con.sh"
//把option方在这个目录是因为这个目录下的文件在开机copy完后可以改为可执行的这样，iptables_init就可以去调用这个脚本，完成开机对流量控制的初始化了
#define TCSHELL_PATH "/opt/services/option/traffic_option"
//按照interfac进行排序，因为interface在开头，排序是因为awk中要求将if排序了来写。
#define GET_SHELL_CMD	"cat "RULEINFO_PATH" | sort | awk -f "AWK_PATH" > "TCSHELL_PATH
//#define SAVE_TRAFFIC_STATUS "echo \"start\" > /opt/services/status/traffic_status.status;echo \"start\" > /opt/services/status/iptables_status.status;"
{
	int status = system(GET_SHELL_CMD" 2>/dev/null");
	int ret = WEXITSTATUS(status);

	if(0==ret)
	{
		//执行shell
		status = system( MODIFYSHELL_PERMITE" >/dev/null 2>&1;"TCSHELL_PATH" >/dev/null 2>&1;" ); 	 
		ret = WEXITSTATUS(status);

		if( 0 != ret )
		{
			//ShowAlert( "Do Shell Error!" );
			iRet = -1;
		}
	}
	else
	{
	//	ShowAlert( "Get Shell Error!" );
	    iRet = -2;
	}
}	
#else
	int status = system("/opt/bin/filter_con.sh  >/dev/null 2>&1");
	int ret = WEXITSTATUS(status);

	if(0!=ret)
	{
		//ShowAlert( "Do Shell Error!" );
		iRet = -3;
	}	
#endif	
	return iRet;
}


/**/
PTCRule getTCRuleByIndex( PTCRule ptcRuleRoot, int iIndex )
{
	int i;
	PTCRule pReturn=NULL;
	
	if( NULL == ptcRuleRoot )
	{
		return NULL;
	}
	
	for( i=0,pReturn=ptcRuleRoot; (i<iIndex&&pReturn);i++, pReturn=pReturn->next);

	if( i<iIndex )
	{
		return NULL;
	}

	return pReturn;
}


