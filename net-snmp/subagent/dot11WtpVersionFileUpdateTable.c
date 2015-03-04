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
* dot11WtpVersionFileUpdateTable.c
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

#include <libxml/parser.h>
#include <libxml/tree.h>

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "dot11WtpVersionFileUpdateTable.h"
#include "wcpss/asd/asd.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/dcli_wid_wtp.h"
#include "dbus/wcpss/dcli_wid_wlan.h"
#include "ws_dcli_wlans.h"
#include "autelanWtpGroup.h"
#include "ws_init_dbus.h"
#include "ws_dbus_list_interface.h"


#define VEROID	"1.1.4"


#define MIBS_XML_FPAHT "/opt/www/htdocs/mibs_xml"

#define MIBS_XML_URL  "url"
#define MIBS_XML_UNAME  "uname"
#define MIBS_XML_PWD  "pwd"
#define MIBS_XML_VNAME  "vname"
#define MIBS_XML_APID  "apid"
#define MIBS_XML_APM  "apmodel"
#define MIBS_XML_ROOT  "root"

//create xml for mibs
static int create_mibs_xml()
{
	xmlDocPtr doc = NULL;			/* document pointer */
	//char content[256];	
	
	xmlNodePtr root_node = NULL;
	
	doc = xmlNewDoc(BAD_CAST "1.0");
	root_node = xmlNewNode(NULL, BAD_CAST MIBS_XML_ROOT );
	xmlDocSetRootElement(doc, root_node);

	xmlSaveFormatFileEnc(MIBS_XML_FPAHT, doc, "UTF-8", 1);
	/*free the document */
	xmlFreeDoc(doc);
	xmlCleanupParser();
	xmlMemoryDump();	  //debug memory for regression tests
	
	return 0;	
}


static int clear_mibs_xml()
{
    char temp[128];
    memset(temp,0,128);
    int flag=-1;
    sprintf(temp,"rm  %s",MIBS_XML_FPAHT);
    if(access(MIBS_XML_FPAHT,0)==0)
    	system(temp);
    flag=create_mibs_xml();
    return flag;
}

static int read_version(struct ver_wtp *vrrp)
{
	xmlDocPtr doc;	 //定义解析文档指针
	
	xmlNodePtr cur,tmp; //定义结点指针(你需要它为了在各个结点间移动)
	

	
	doc = xmlReadFile(MIBS_XML_FPAHT, "utf-8", 256); //解析文件
	
	if (doc == NULL ) {
	
	   fprintf(stderr,"Document not parsed successfully. \n");
	
		return 0;
	
	}
	cur = xmlDocGetRootElement(doc); 
	
	if (cur == NULL) {
	
		   fprintf(stderr,"empty document\n");
	
		   xmlFreeDoc(doc);
	
		   return 0;
	
	}
	
	if (xmlStrcmp(cur->name, (const xmlChar *) "root")) {
	
	   fprintf(stderr,"document of the wrong type, root node != root");
	
		   xmlFreeDoc(doc);
	
		   return 0;
	
	}
	
	struct ver_wtp *tail = NULL;
	xmlChar *value;
	vrrp->next = NULL;
	tail = vrrp;
	cur = cur->xmlChildrenNode;
	while(cur !=NULL)
	{
		xmlChar *key=NULL;
		struct ver_wtp *q = NULL;
		q = (struct tem_vrrp*)malloc(sizeof(struct ver_wtp)+1);
		memset(q,0,sizeof(struct ver_wtp)+1);
		q->id = (char*)malloc(10);
		memset(q->id,0,10);
		q->faddr = (char*)malloc(256);
		memset(q->faddr,0,256);
		q->usrname = (char*)malloc(128);
		memset(q->usrname,0,128);
		q->passwd = (char*)malloc(128);
		memset(q->passwd,0,128);	
		q->fname = (char*)malloc(128);
		memset(q->fname,0,128);
		q->apmode= (char*)malloc(128);
		memset(q->apmode,0,128);
		q->next=NULL;
		value = xmlGetProp(cur, (const xmlChar *)"id");
		strcpy(q->id,(char*)value);
		tmp = cur->xmlChildrenNode;
		 xmlChar *value;
        // key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
        value = xmlGetProp(cur, (const xmlChar *)"id");
        strcpy(q->id,(char*)value);
        xmlFree(value);
		while(tmp!=NULL) 
		{
			//fprintf(stderr,"tmp->name===========%s\n",tmp->name);
			if ((!xmlStrcmp(tmp->name, (const xmlChar *)"url"))) 
			{
				key = xmlNodeListGetString(doc, tmp->xmlChildrenNode, 1);
				strcpy(q->faddr,(char*)key);
			}
			else if(!xmlStrcmp(tmp->name,(const xmlChar *)"pwd"))
			{
				key = xmlNodeListGetString(doc, tmp->xmlChildrenNode, 1);
				strcpy(q->passwd,(char*)key);
			}
			else if ((!xmlStrcmp(tmp->name, (const xmlChar *)"uname"))) 
			{
				key = xmlNodeListGetString(doc, tmp->xmlChildrenNode, 1);
				strcpy(q->usrname,(char*)key);
			}
			else if ((!xmlStrcmp(tmp->name, (const xmlChar *)MIBS_XML_VNAME))) 
			{
				key = xmlNodeListGetString(doc, tmp->xmlChildrenNode, 1);
				strcpy(q->fname,(char*)key);
			}
			else if ((!xmlStrcmp(tmp->name, (const xmlChar *)MIBS_XML_APM))) 
			{
				key = xmlNodeListGetString(doc, tmp->xmlChildrenNode, 1);
				strcpy(q->apmode,(char*)key);
			}
			tmp = tmp->next;

		}
		//fprintf(stderr,"q->inter==========%s\n",q->inter);
		//fprintf(stderr,"q->vrid==========%s\n",q->vrrpid);
		//fprintf(stderr,"q->priority==========%s\n",q->priority);
		//fprintf(stderr,"q->vritual==========%s\n",q->virtual);
		q->next = NULL;
		tail->next = q;
		tail = q;
		xmlFree(key);
		
		cur = cur->next;
	}
	xmlFreeDoc(doc);
	
    return 1;
}


struct dot11WtpVersionFileUpdateTable_entry {
    /* Index values */
	dbus_parameter parameter;
    long wtpCurrID;
	char *wtpMacAddr;

    /* Column values */
    char *wtpVersionFileName;
  //  char *old_wtpVersionFileName;
    char *wtpMode;
  //  char *old_wtpModel;
    char *wtpFtpAddr;
   // char *old_wtpFtpAddr;
    char *wtpUsrName;
  //  char *old_wtpUsrName;
    char *wtpPasswd;
  //  char *old_wtpPasswd;
    long wtpUpdateAction;
  //  long old_wtpUpdateAction;

    /* Illustrate using a simple linked list */
    int   valid;
    struct dot11WtpVersionFileUpdateTable_entry *next;
};

void dot11WtpVersionFileUpdateTable_load();

void
dot11WtpVersionFileUpdateTable_removeEntry( struct dot11WtpVersionFileUpdateTable_entry *entry );

/** Initializes the dot11WtpVersionFileUpdateTable module */
void
init_dot11WtpVersionFileUpdateTable(void)
{
  /* here we initialize all the tables we're planning on supporting */
    initialize_table_dot11WtpVersionFileUpdateTable();
  	/*here add create xml file*/
	clear_mibs_xml();
}

/** Initialize the dot11WtpVersionFileUpdateTable table by defining its contents and how it's structured */
void
initialize_table_dot11WtpVersionFileUpdateTable(void)
{
    static oid dot11WtpVersionFileUpdateTable_oid[128] = {0};
    size_t dot11WtpVersionFileUpdateTable_oid_len   = 0;
	mad_dev_oid(dot11WtpVersionFileUpdateTable_oid,VEROID,&dot11WtpVersionFileUpdateTable_oid_len,enterprise_pvivate_oid);
    netsnmp_handler_registration    *reg;
    netsnmp_iterator_info           *iinfo;
    netsnmp_table_registration_info *table_info;

    reg = netsnmp_create_handler_registration(
              "dot11WtpVersionFileUpdateTable",     dot11WtpVersionFileUpdateTable_handler,
              dot11WtpVersionFileUpdateTable_oid, dot11WtpVersionFileUpdateTable_oid_len,
              HANDLER_CAN_RWRITE
              );

    table_info = SNMP_MALLOC_TYPEDEF( netsnmp_table_registration_info );
    netsnmp_table_helper_add_indexes(table_info,
                           ASN_OCTET_STR,  /* index: wtpCurrID */
                           0);
    table_info->min_column = VERMIN;
    table_info->max_column = VERMAX;
    
    iinfo = SNMP_MALLOC_TYPEDEF( netsnmp_iterator_info );
    iinfo->get_first_data_point = dot11WtpVersionFileUpdateTable_get_first_data_point;
    iinfo->get_next_data_point  = dot11WtpVersionFileUpdateTable_get_next_data_point;
    iinfo->table_reginfo        = table_info;
    
    netsnmp_register_table_iterator( reg, iinfo );
	netsnmp_inject_handler(reg,netsnmp_get_cache_handler(DOT1DTPFDBTABLE_CACHE_TIMEOUT,dot11WtpVersionFileUpdateTable_load, dot11WtpVersionFileUpdateTable_removeEntry,
							dot11WtpVersionFileUpdateTable_oid, dot11WtpVersionFileUpdateTable_oid_len));

    /* Initialise the contents of the table here */
}

    /* Typical data structure for a row entry */

struct dot11WtpVersionFileUpdateTable_entry  *dot11WtpVersionFileUpdateTable_head;

/* create a new row in the (unsorted) table */
struct dot11WtpVersionFileUpdateTable_entry *
dot11WtpVersionFileUpdateTable_createEntry(
						dbus_parameter parameter,
					    long  wtpCurrID,
					    char *wtpMacAddr,
					    char *wtpVersionFileName,
					    char *wtpMode,
					    char *wtpFtpAddr,
					    char *wtpUsrName,
					    char *wtpPasswd,
					    long wtpUpdateAction
                ) {
    struct dot11WtpVersionFileUpdateTable_entry *entry;

    entry = SNMP_MALLOC_TYPEDEF(struct dot11WtpVersionFileUpdateTable_entry);
    if (!entry)
        return NULL;

	memcpy(&(entry->parameter), &parameter, sizeof(dbus_parameter));
    entry->wtpCurrID = wtpCurrID;
	entry->wtpMacAddr	= strdup(wtpMacAddr);    	
	entry->wtpVersionFileName = strdup(wtpVersionFileName);
	entry->wtpMode = strdup(wtpMode);
	entry->wtpFtpAddr= strdup(wtpFtpAddr);
	entry->wtpUsrName= strdup(wtpUsrName);
	entry->wtpPasswd= strdup(wtpPasswd);
	entry->wtpUpdateAction= wtpUpdateAction;
    entry->next = dot11WtpVersionFileUpdateTable_head;
    dot11WtpVersionFileUpdateTable_head = entry;
    return entry;
}

/* remove a row from the table */
void
dot11WtpVersionFileUpdateTable_removeEntry( struct dot11WtpVersionFileUpdateTable_entry *entry ) {
    struct dot11WtpVersionFileUpdateTable_entry *ptr, *prev;

    if (!entry)
        return;    /* Nothing to remove */

    for ( ptr  = dot11WtpVersionFileUpdateTable_head, prev = NULL;
          ptr != NULL;
          prev = ptr, ptr = ptr->next ) {
        if ( ptr == entry )
            break;
    }
    if ( !ptr )
        return;    /* Can't find it */

    if ( prev == NULL )
        dot11WtpVersionFileUpdateTable_head = ptr->next;
    else
        prev->next = ptr->next;
	FREE_OBJECT(entry->wtpMacAddr);
	FREE_OBJECT(entry->wtpVersionFileName);
	FREE_OBJECT(entry->wtpMode);
	FREE_OBJECT(entry->wtpFtpAddr);
	FREE_OBJECT(entry->wtpUsrName);
	FREE_OBJECT(entry->wtpPasswd);
    SNMP_FREE( entry );   /* XXX - release any other internal resources */
}
void dot11WtpVersionFileUpdateTable_load()
{	
	snmp_log(LOG_DEBUG, "enter dot11WtpVersionFileUpdateTable_load\n");

	struct dot11WtpVersionFileUpdateTable_entry *temp;
	while( dot11WtpVersionFileUpdateTable_head )
	{
		temp=dot11WtpVersionFileUpdateTable_head->next;
		dot11WtpConfigFileUpdateTable_removeEntry(dot11WtpVersionFileUpdateTable_head);
		dot11WtpVersionFileUpdateTable_head=temp;
	}  
	
	char temp_mac[20] = { 0 };
	char temp_model[50] = { 0 };
    snmpd_dbus_message *messageHead = NULL, *messageNode = NULL;
    snmp_log(LOG_DEBUG, "enter list_connection_call_dbus_method:show_wtp_list_by_mac_cmd_func\n");
    messageHead = list_connection_call_dbus_method(show_wtp_list_by_mac_cmd_func, SHOW_ALL_WTP_TABLE_METHOD);
    snmp_log(LOG_DEBUG, "exit list_connection_call_dbus_method:show_wtp_list_by_mac_cmd_func,messageHead=%p\n", messageHead);
	
	if(messageHead)
	{
	    for(messageNode = messageHead; NULL != messageNode; messageNode = messageNode->next)
        {
            DCLI_WTP_API_GROUP_ONE *head = messageNode->message;

    		if((head)&&(head->WTP_INFO)&&(head->WTP_INFO->WTP_LIST))
    		{
    		    int i = 0;
	            WID_WTP *q = NULL;
    			for(i = 0,q = head->WTP_INFO->WTP_LIST; (i < head->WTP_INFO->list_len)&&(NULL != q); i++,q=q->next)
    			{
    				memset(temp_mac,0,20);
					memset(temp_model,0,20);
					if(q->WTPMAC)
					{
						snprintf(temp_mac,sizeof(temp_mac)-1,"%02X:%02X:%02X:%02X:%02X:%02X",q->WTPMAC[0],q->WTPMAC[1],q->WTPMAC[2],q->WTPMAC[3],q->WTPMAC[4],q->WTPMAC[5]);
					}
					if(q->WTPModel)
					{
						strncpy(temp_model,q->WTPModel,sizeof(temp_model)-1);
					}
    				dot11WtpVersionFileUpdateTable_createEntry(messageNode->parameter,
    															q->WTPID,
    															temp_mac,
    															"",
    															temp_model,
    															"",
    															"",
    															"",
    															1);		
					FREE_OBJECT(q->WTPMAC);
					FREE_OBJECT(q->WTPNAME); //lilong add 2015.1.19
					FREE_OBJECT(q->WTPModel); //lilong add 2015.1.19
    			}
    		}
    	}	
        free_dbus_message_list(&messageHead, Free_wtp_list_by_mac_head);
	}
	
	snmp_log(LOG_DEBUG, "exit dot11WtpVersionFileUpdateTable_load\n");
}


/* Example iterator hook routines - using 'get_next' to do most of the work */
netsnmp_variable_list *
dot11WtpVersionFileUpdateTable_get_first_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{

		if(dot11WtpVersionFileUpdateTable_head==NULL)
		{
			return NULL;
		}
	*my_data_context =   dot11WtpVersionFileUpdateTable_head;
    *my_loop_context = dot11WtpVersionFileUpdateTable_head;
    return dot11WtpVersionFileUpdateTable_get_next_data_point(my_loop_context, my_data_context,
                                    put_index_data,  mydata );
}

netsnmp_variable_list *
dot11WtpVersionFileUpdateTable_get_next_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{
    struct dot11WtpVersionFileUpdateTable_entry *entry = (struct dot11WtpVersionFileUpdateTable_entry *)*my_loop_context;
    netsnmp_variable_list *idx = put_index_data;

    if ( entry ) {
        snmp_set_var_value( idx,(u_char *)entry->wtpMacAddr, strlen(entry->wtpMacAddr));
        idx = idx->next_variable;
        *my_data_context = (void *)entry;
        *my_loop_context = (void *)entry->next;
    } else {
        return NULL;
    }
	return put_index_data;
}


/** handles requests for the dot11WtpVersionFileUpdateTable table */
int
dot11WtpVersionFileUpdateTable_handler(
    netsnmp_mib_handler               *handler,
    netsnmp_handler_registration      *reginfo,
    netsnmp_agent_request_info        *reqinfo,
    netsnmp_request_info              *requests) {

    netsnmp_request_info       *request;
    netsnmp_table_request_info *table_info;
    struct dot11WtpVersionFileUpdateTable_entry          *table_entry;
    switch (reqinfo->mode) {
        /*
         * Read-support (also covers GetNext requests)
         */
    case MODE_GET:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11WtpVersionFileUpdateTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    
		 if( !table_entry ){
		       	netsnmp_set_request_error(reqinfo,request,SNMP_NOSUCHINSTANCE);
				continue;
		    }    
            switch (table_info->colnum) {
            case COLUMN_WTPVERSIONFILENAME:
			{
				int ret = 0,i=0;
                void *connection = NULL;
                if(SNMPD_DBUS_ERROR == get_slot_dbus_connection(table_entry->parameter.slot_id, &connection, SNMPD_INSTANCE_MASTER_V3)) {
                    break;
                }
                
                char ap_version[50] = { 0 };
				memset(ap_version,0,50);
				#if 0
				DCLI_WTP_API_GROUP_ONE *WTPINFO;

				ret = show_version(table_entry->parameter, connection,&WTPINFO);
				if(ret == 1)
				{
  					if((WTPINFO)&&(WTPINFO->AP_VERSION))
					{
						for (i = 0; i < WTPINFO->num; i++) 
						{
							if(strcmp(table_entry->wtpMode,WTPINFO->AP_VERSION[i]->apmodel)==0)
							{
								strncpy(ap_version,WTPINFO->AP_VERSION[i]->versionname,sizeof(ap_version)-1);
								break;
							}
						}
					}						
				}
				#endif
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char*)(table_entry->wtpVersionFileName),
                                          strlen(table_entry->wtpVersionFileName));
                #if 0
				if(ret==1)
				{
					Free_wtp_model(WTPINFO);
				}
				else if (SNMPD_CONNECTION_ERROR == ret){
                    close_slot_dbus_connection(table_entry->parameter.slot_id);
				}
				#endif
            }
                break;
            case COLUMN_WTPMODE:
			{
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char*)table_entry->wtpMode,
                                          strlen(table_entry->wtpMode));
            }
                break;
            case COLUMN_WTPFTPADDR:
			{
				int ret=0;
				struct ver_wtp vrrphead,*p;
				memset(&vrrphead,0,sizeof(struct ver_wtp));
				char wtpID[10] = { 0 };
				memset(wtpID,0,10);
				char ftpaddr[256];
				memset(ftpaddr,0,256);

				ret = read_version(&vrrphead);
				if(ret==1)
				{
					p=vrrphead.next;
					if(p !=NULL)
					{
						while(p)
						{
							snprintf(wtpID,sizeof(wtpID)-1,"%d",table_entry->wtpCurrID);
							if(strcmp(p->id,wtpID)==0)
							{
								strncpy(ftpaddr,p->faddr,sizeof(ftpaddr)-1);
							}
							p = p->next;
						}
					}
				}
				
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char*)ftpaddr,
                                          strlen(ftpaddr));

				if(ret==1)
				{
					struct ver_wtp *f1=NULL,*f2=NULL;
					f1 = vrrphead.next;
					if(f1 !=NULL)
					{
						if((f1->next)!=NULL)
						{
							f2 = f1->next;
							while(f2!=NULL)
							{
								FREE_OBJECT(f1->id);
								FREE_OBJECT(f1->faddr);
								FREE_OBJECT(f1->passwd);
								FREE_OBJECT(f1->usrname);
								FREE_OBJECT(f1->fname);
								FREE_OBJECT(f1->apmode);
								FREE_OBJECT(f1);
								f1 = f2;
								f2=f2->next;
							}
						}
						FREE_OBJECT(f1->id);
						FREE_OBJECT(f1->faddr);
						FREE_OBJECT(f1->passwd);
						FREE_OBJECT(f1->usrname);
						FREE_OBJECT(f1->fname);
						FREE_OBJECT(f1->apmode);
						FREE_OBJECT(f1);
					}
				}
            }
                break;
            case COLUMN_WTPUSRNAME:
			{
				int ret=0;
				struct ver_wtp vrrphead,*p;
				memset(&vrrphead,0,sizeof(struct ver_wtp));
				char wtpID[10] = { 0 };
				memset(wtpID,0,10);
				char usrname[128];
				memset(usrname,0,128);

				ret = read_version(&vrrphead);

				if(ret==1)
				{
					p=vrrphead.next;
					if(p !=NULL)
					{
						while(p)
						{
							snprintf(wtpID,sizeof(wtpID)-1,"%d",table_entry->wtpCurrID);
							if(strcmp(p->id,wtpID)==0)
							{
								strncpy(usrname,p->usrname,sizeof(usrname)-1);
							}
							p = p->next;
						}
					}
				}
				
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char*)usrname,
                                          strlen(usrname));

				if(ret==1)
				{
					struct ver_wtp *f1=NULL,*f2=NULL;
					f1 = vrrphead.next;
					if(f1 !=NULL)
					{
						if((f1->next)!=NULL)
						{
							f2 = f1->next;
							while(f2!=NULL)
							{
								FREE_OBJECT(f1->id);
								FREE_OBJECT(f1->faddr);
								FREE_OBJECT(f1->passwd);
								FREE_OBJECT(f1->usrname);
								FREE_OBJECT(f1->fname);
								FREE_OBJECT(f1->apmode);
								FREE_OBJECT(f1);
								f1 = f2;
								f2=f2->next;
							}
						}
						FREE_OBJECT(f1->id);
						FREE_OBJECT(f1->faddr);
						FREE_OBJECT(f1->passwd);
						FREE_OBJECT(f1->usrname);
						FREE_OBJECT(f1->fname);
						FREE_OBJECT(f1->apmode);
						FREE_OBJECT(f1);
					}
				}
            }
                break;
            case COLUMN_WTPPASSWD:
			{
				int ret=0;
				struct ver_wtp vrrphead,*p;
				memset(&vrrphead,0,sizeof(struct ver_wtp));
				char wtpID[10] = { 0 };
				memset(wtpID,0,10);
				char passwd[128];
				memset(passwd,0,128);

				ret = read_version(&vrrphead);

				if(ret==1)
				{
					p=vrrphead.next;
					if(p !=NULL)
					{
						while(p)
						{
							snprintf(wtpID,sizeof(wtpID)-1,"%d",table_entry->wtpCurrID);
							if(strcmp(p->id,wtpID)==0)
							{
								strncpy(passwd,p->passwd,sizeof(passwd)-1);
							}
							p = p->next;
						}
					}
				}
				
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char*)passwd,
                                          strlen(passwd));

				if(ret==1)
				{
					struct ver_wtp *f1=NULL,*f2=NULL;
					f1 = vrrphead.next;
					if(f1 !=NULL)
					{
						if((f1->next)!=NULL)
						{
							f2 = f1->next;
							while(f2!=NULL)
							{
								FREE_OBJECT(f1->id);
								FREE_OBJECT(f1->faddr);
								FREE_OBJECT(f1->passwd);
								FREE_OBJECT(f1->usrname);
								FREE_OBJECT(f1->fname);
								FREE_OBJECT(f1->apmode);
								FREE_OBJECT(f1);
								f1 = f2;
								f2=f2->next;
							}
						}
						FREE_OBJECT(f1->id);
						FREE_OBJECT(f1->faddr);
						FREE_OBJECT(f1->passwd);
						FREE_OBJECT(f1->usrname);
						FREE_OBJECT(f1->fname);
						FREE_OBJECT(f1->apmode);
						FREE_OBJECT(f1);
					}
				}
            }
                break;
            case COLUMN_WTPUPDATEACTION:
			{
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->wtpUpdateAction,
                                          sizeof(long));
            }
                break;
            }
        }
        break;

        /*
         * Write-support
         */
    case MODE_SET_RESERVE1:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11WtpVersionFileUpdateTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    
            switch (table_info->colnum) {
            case COLUMN_WTPVERSIONFILENAME:
                if ( request->requestvb->type != ASN_OCTET_STR ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_WTPMODE:
                if ( request->requestvb->type != ASN_OCTET_STR ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_WTPFTPADDR:
                if ( request->requestvb->type != ASN_OCTET_STR ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_WTPUSRNAME:
                if ( request->requestvb->type != ASN_OCTET_STR ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_WTPPASSWD:
                if ( request->requestvb->type != ASN_OCTET_STR ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_WTPUPDATEACTION:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            default:
                netsnmp_set_request_error( reqinfo, request,
                                           SNMP_ERR_NOTWRITABLE );
                return SNMP_ERR_NOERROR;
            }
        }
        break;

    case MODE_SET_RESERVE2:
        break;

    case MODE_SET_FREE:
        break;

	case MODE_SET_ACTION:
	{
		for (request=requests; request; request=request->next) 
		{
			table_entry = (struct dot11WtpVersionFileUpdateTable_entry *)
			netsnmp_extract_iterator_context(request);
			table_info  =     netsnmp_extract_table_info(      request);
			
			 if( !table_entry ){
			       	netsnmp_set_request_error(reqinfo,request,SNMP_NOSUCHINSTANCE);
					continue;
			    	}    
			switch (table_info->colnum) 
			{
				
				case COLUMN_WTPVERSIONFILENAME:
				{	
					char input_string[256] = { 0 };
					memset(input_string,0,256);
					char wtpID[10] = { 0 };
					memset(wtpID,0,10);
					int ret= 0;
					snprintf(wtpID,sizeof(wtpID)-1,"%d",table_entry->wtpCurrID);
					ret =add_mibs_node_attr_z(MIBS_XML_FPAHT,"pid","id",wtpID);
					FREE_OBJECT(table_entry->wtpVersionFileName);
					strncpy(input_string,request->requestvb->val.string,request->requestvb->val_len);
					
					table_entry->wtpVersionFileName     = strdup(input_string);
					mod_mibs_node_z(MIBS_XML_FPAHT,"pid","id",wtpID,MIBS_XML_VNAME,table_entry->wtpVersionFileName);
				}
				break;
				case COLUMN_WTPMODE:
				{
					char wtpID[10] = { 0 };
					char input_string[256] = { 0 };
					memset(wtpID,0,10);
					memset(input_string,0,256);
					snprintf(wtpID,sizeof(wtpID)-1,"%d",table_entry->wtpCurrID);
					add_mibs_node_attr_z(MIBS_XML_FPAHT,"pid","id",wtpID);
					/* Need to save old 'table_entry->wtpModel' value.
					May need to use 'memcpy' */
					FREE_OBJECT(table_entry->wtpMode);
					strncpy(input_string,request->requestvb->val.string,request->requestvb->val_len);
					table_entry->wtpMode= strdup(input_string);
					mod_mibs_node_z(MIBS_XML_FPAHT,"pid","id",wtpID,MIBS_XML_APM,table_entry->wtpMode);
				}
				break;
				case COLUMN_WTPFTPADDR:
				{
					char wtpID[10] = { 0 };
					char input_string[256] = { 0 };
					memset(wtpID,0,10);
					memset(input_string,0,256);
					snprintf(wtpID,sizeof(wtpID)-1,"%d",table_entry->wtpCurrID);
					add_mibs_node_attr_z(MIBS_XML_FPAHT,"pid","id",wtpID);
					/* Need to save old 'table_entry->wtpFtpAddr' value.
					May need to use 'memcpy' */
					FREE_OBJECT(table_entry->wtpFtpAddr);
					strncpy(input_string,request->requestvb->val.string,request->requestvb->val_len);
					table_entry->wtpFtpAddr     = strdup(input_string);
					mod_mibs_node_z(MIBS_XML_FPAHT,"pid","id",wtpID,MIBS_XML_URL,table_entry->wtpFtpAddr);
				}
				break;
				case COLUMN_WTPUSRNAME:
				{
					char wtpID[10] = { 0 };
					char input_string[256] = { 0 };
					memset(wtpID,0,10);
					memset(input_string,0,256);
					snprintf(wtpID,sizeof(wtpID)-1,"%d",table_entry->wtpCurrID);
					add_mibs_node_attr_z(MIBS_XML_FPAHT,"pid","id",wtpID);
					/* Need to save old 'table_entry->wtpUsrName' value.
					May need to use 'memcpy' */
					FREE_OBJECT(table_entry->wtpUsrName);
					strncpy(input_string,request->requestvb->val.string,request->requestvb->val_len);
					table_entry->wtpUsrName     = strdup(input_string);
					mod_mibs_node_z(MIBS_XML_FPAHT,"pid","id",wtpID,MIBS_XML_UNAME,table_entry->wtpUsrName);
				}
				break;
				case COLUMN_WTPPASSWD:
				{
					char wtpID[10] = { 0 };
					char input_string[256] = { 0 };
					memset(wtpID,0,10);
					memset(input_string,0,256);
					snprintf(wtpID,sizeof(wtpID)-1,"%d",table_entry->wtpCurrID);
					add_mibs_node_attr_z(MIBS_XML_FPAHT,"pid","id",wtpID);
					/* Need to save old 'table_entry->wtpPasswd' value.
					May need to use 'memcpy' */
					FREE_OBJECT(table_entry->wtpPasswd);
					strncpy(input_string,request->requestvb->val.string,request->requestvb->val_len);
					table_entry->wtpPasswd     = strdup(input_string);
					mod_mibs_node_z(MIBS_XML_FPAHT,"pid","id",wtpID,MIBS_XML_PWD,table_entry->wtpPasswd);
				}
				break;
				
				case COLUMN_WTPUPDATEACTION:
				{
                    void *connection = NULL;
                    if(SNMPD_DBUS_ERROR == get_instance_dbus_connection(table_entry->parameter, &connection, SNMPD_INSTANCE_MASTER_V3)) {
                        netsnmp_set_request_error(reqinfo,request,SNMP_ERR_WRONGTYPE);
                        break;
                    }
					int ret_1=0;
				 	struct ver_wtp vrrphead,*p;
					char wtpID[10] = { 0 };
					memset(wtpID,0,10);
					//sprintf(wtpID,"%d",table_entry->wtpCurrID);
					//add_mibs_node_attr(MIBS_XML_FPAHT,"pid","id",wtpID);
					int ret =  -1;
					char url[256];
					char uname[128];
					char pwd[128];
					char ap_mode[128];
					char ap_filename[128];
					memset(url,0,256);
					memset(uname,0,128);
					memset(pwd,0,128);
					memset(ap_mode,0,128);
					memset(ap_filename,0,128);
					ret_1 = read_version(&vrrphead);
					if(ret_1==1)
					{
						p=vrrphead.next;
						if(p !=NULL)
						{
							while(p)
							{
								snprintf(wtpID,sizeof(wtpID)-1,"%d",table_entry->wtpCurrID);
								if(strcmp(p->id,wtpID)==0)
								{
									strncpy(url,p->faddr,sizeof(url)-1);
									strncpy(uname,p->usrname,sizeof(uname)-1);
									strncpy(pwd,p->passwd,sizeof(pwd)-1);
									strncpy(ap_mode,p->apmode,sizeof(ap_mode)-1);
									strncpy(ap_filename,p->fname,sizeof(ap_filename)-1);
								}
								p = p->next;
							}
						}
					}
					else
					{						
						netsnmp_set_request_error(reqinfo,request,SNMP_ERR_WRONGTYPE);
					}

					if(ret_1==1)
					{
						struct ver_wtp*f1=NULL,*f2=NULL;
						f1 = vrrphead.next;
						if(f1 !=NULL)
						{
							if((f1->next)!=NULL)
							{
								f2 = f1->next;
								while(f2!=NULL)
								{
									FREE_OBJECT(f1->id);
									FREE_OBJECT(f1->faddr);
									FREE_OBJECT(f1->passwd);
									FREE_OBJECT(f1->usrname);
									FREE_OBJECT(f1->fname);
									FREE_OBJECT(f1->apmode);
									FREE_OBJECT(f1);
									f1 = f2;
									f2=f2->next;
								}
							}
							FREE_OBJECT(f1->id);
							FREE_OBJECT(f1->faddr);
							FREE_OBJECT(f1->passwd);
							FREE_OBJECT(f1->usrname);
							FREE_OBJECT(f1->fname);
							FREE_OBJECT(f1->apmode);
							FREE_OBJECT(f1);
						}		
					}
					if(*requests->requestvb->val.integer == 1)
					{
						if((strcmp(url,"")!=0)&&(strcmp(uname,"")!=0)&&(strcmp(pwd,"")!=0))
						{
							ret = download_ap_version(url,uname,pwd);
							if(ret ==-1)
							{						
								netsnmp_set_request_error(reqinfo,request,SNMP_ERR_WRONGTYPE);
							}
							else
							{
								int ret_mo = 0;
								if((strcmp(ap_mode,"")!=0)&&(strcmp(ap_filename,"")!=0))
								{
									ret_mo = set_ap_model(table_entry->parameter, connection, ap_mode,ap_filename,ap_filename, "1","8");
									if(ret_mo !=1)
									{
									    if(SNMPD_CONNECTION_ERROR == ret) {
                                            close_slot_dbus_connection(table_entry->parameter.slot_id);
                                	    }
										netsnmp_set_request_error(reqinfo,request,SNMP_ERR_WRONGTYPE);
									}
								}
							}
						}
						else
						{						
							netsnmp_set_request_error(reqinfo,request,SNMP_ERR_WRONGTYPE);
						}
					}
				}
				break;
			}
		}
	}
	break;

    case MODE_SET_UNDO:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11WtpVersionFileUpdateTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    
            switch (table_info->colnum) {
            case COLUMN_WTPVERSIONFILENAME:
                /* Need to restore old 'table_entry->wtpVersionFileName' value.
                   May need to use 'memcpy' */
             //   table_entry->wtpVersionFileName = strdup(table_entry->old_wtpVersionFileName);
                break;
            case COLUMN_WTPMODE:
                /* Need to restore old 'table_entry->wtpVersionFileName' value.
                   May need to use 'memcpy' */
               // table_entry->wtpModel= strdup(table_entry->old_wtpModel);
                break;
            case COLUMN_WTPFTPADDR:
                /* Need to restore old 'table_entry->wtpFtpAddr' value.
                   May need to use 'memcpy' */
              //  table_entry->wtpFtpAddr = strdup(table_entry->old_wtpFtpAddr);
                break;
            case COLUMN_WTPUSRNAME:
                /* Need to restore old 'table_entry->wtpUsrName' value.
                   May need to use 'memcpy' */
              //  table_entry->wtpUsrName = strdup(table_entry->old_wtpUsrName);
                break;
            case COLUMN_WTPPASSWD:
                /* Need to restore old 'table_entry->wtpPasswd' value.
                   May need to use 'memcpy' */
             //   table_entry->wtpPasswd = strdup(table_entry->old_wtpPasswd);
                break;
            case COLUMN_WTPUPDATEACTION:
                /* Need to restore old 'table_entry->wtpUpdateAction' value.
                   May need to use 'memcpy' */
             //   table_entry->wtpUpdateAction = table_entry->old_wtpUpdateAction;
                break;
            }
        }
        break;

    case MODE_SET_COMMIT:
        break;
    }
    return SNMP_ERR_NOERROR;
}
