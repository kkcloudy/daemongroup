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
* dot11AcVersionAndConfigTable.c
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


#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "dot11AcVersionAndConfigTable.h"
#include "autelanWtpGroup.h"
#include "ws_version_param.h"
#include "ws_init_dbus.h"

#define	ACVERSIONANDCONFIGTABLE "2.15.1"

struct dot11AcVersionAndConfigTable_entry {
    /* Index values */
    long LoadFlag;

    /* Column values */
    char *FileName;
    //char *old_FileName;
    long FileType;
    //long old_FileType;
    long EstimateFileSize;
    //long old_EstimateFileSize;
    long TransProtocol;
    //long old_TransProtocol;
    char *ServerAddr;
    //char *old_ServerAddr;
    char *ServerPort;
    //char *old_ServerPort;
    char *ServerUsername;
    //char *old_ServerUsername;
    char *ServerPasswd;
    //char *old_ServerPasswd;
    long TransStatus;
    long TotalBytes;
    //long old_TotalBytes;
    long TransferBytes;
    char *FailReason;
    //char *old_FailReason;

    /* Illustrate using a simple linked list */
    int   valid;
    struct dot11AcVersionAndConfigTable_entry *next;
};

void dot11AcVersionAndConfigTable_load();
void
dot11AcVersionAndConfigTable_removeEntry( struct dot11AcVersionAndConfigTable_entry *entry ) ;

/** Initializes the dot11AcVersionAndConfigTable module */
void
init_dot11AcVersionAndConfigTable(void)
{
  /* here we initialize all the tables we're planning on supporting */
    initialize_table_dot11AcVersionAndConfigTable();
}

/** Initialize the dot11AcVersionAndConfigTable table by defining its contents and how it's structured */
void
initialize_table_dot11AcVersionAndConfigTable(void)
{
    oid dot11AcVersionAndConfigTable_oid[128] = {0};
    size_t dot11AcVersionAndConfigTable_oid_len   = 0;
	mad_dev_oid(dot11AcVersionAndConfigTable_oid,ACVERSIONANDCONFIGTABLE,&dot11AcVersionAndConfigTable_oid_len,enterprise_pvivate_oid);
    netsnmp_handler_registration    *reg;
    netsnmp_iterator_info           *iinfo;
    netsnmp_table_registration_info *table_info;

    reg = netsnmp_create_handler_registration(
              "dot11AcVersionAndConfigTable",     dot11AcVersionAndConfigTable_handler,
              dot11AcVersionAndConfigTable_oid, dot11AcVersionAndConfigTable_oid_len,
              HANDLER_CAN_RWRITE
              );

    table_info = SNMP_MALLOC_TYPEDEF( netsnmp_table_registration_info );
    netsnmp_table_helper_add_indexes(table_info,
                           ASN_INTEGER,  /* index: LoadFlag */
                           0);
    table_info->min_column = COLUMN_VERSIONMIN;
    table_info->max_column = COLUMN_VERSIONMAX;
    
    iinfo = SNMP_MALLOC_TYPEDEF( netsnmp_iterator_info );
    iinfo->get_first_data_point = dot11AcVersionAndConfigTable_get_first_data_point;
    iinfo->get_next_data_point  = dot11AcVersionAndConfigTable_get_next_data_point;
    iinfo->table_reginfo        = table_info;
    
    netsnmp_register_table_iterator( reg, iinfo );
	netsnmp_inject_handler(reg,netsnmp_get_cache_handler(DOT1DTPFDBTABLE_CACHE_TIMEOUT,dot11AcVersionAndConfigTable_load, dot11AcVersionAndConfigTable_removeEntry,
							dot11AcVersionAndConfigTable_oid, dot11AcVersionAndConfigTable_oid_len));

    /* Initialise the contents of the table here */
}

    /* Typical data structure for a row entry */

struct dot11AcVersionAndConfigTable_entry  *dot11AcVersionAndConfigTable_head;

/* create a new row in the (unsorted) table */
struct dot11AcVersionAndConfigTable_entry *
dot11AcVersionAndConfigTable_createEntry(
								long  LoadFlag,
								char *FileName,
								long FileType,
								long EstimateFileSize,
								long TransProtocol,
								char *ServerAddr,
								char *ServerPort,
								char *ServerUsername,
								char *ServerPasswd,
								long TransStatus,
								long TotalBytes,
								long TransferBytes,
								char *FailReason
                ) {
    struct dot11AcVersionAndConfigTable_entry *entry;

    entry = SNMP_MALLOC_TYPEDEF(struct dot11AcVersionAndConfigTable_entry);
    if (!entry)
        return NULL;

	entry->LoadFlag = LoadFlag;
	entry->FileName = strdup(FileName);
	entry->FileType = FileType;
	entry->EstimateFileSize = EstimateFileSize;
	entry->TransProtocol = TransProtocol;
	entry->ServerAddr = strdup(ServerAddr);
	entry->ServerPort = strdup(ServerPort);
	entry->ServerUsername = strdup(ServerUsername);
	entry->ServerPasswd = strdup(ServerPasswd);
	entry->TransStatus = TransStatus;
	entry->TotalBytes = TotalBytes;
	entry->TransferBytes = TransferBytes;
	entry->FailReason = strdup(FailReason);
    entry->next = dot11AcVersionAndConfigTable_head;
    dot11AcVersionAndConfigTable_head = entry;
    return entry;
}

/* remove a row from the table */
void
dot11AcVersionAndConfigTable_removeEntry( struct dot11AcVersionAndConfigTable_entry *entry ) {
    struct dot11AcVersionAndConfigTable_entry *ptr, *prev;

    if (!entry)
        return;    /* Nothing to remove */

    for ( ptr  = dot11AcVersionAndConfigTable_head, prev = NULL;
          ptr != NULL;
          prev = ptr, ptr = ptr->next ) {
        if ( ptr == entry )
            break;
    }
    if ( !ptr )
        return;    /* Can't find it */

    if ( prev == NULL )
        dot11AcVersionAndConfigTable_head = ptr->next;
    else
        prev->next = ptr->next;
	FREE_OBJECT(entry->FileName);
	FREE_OBJECT(entry->ServerAddr);
	FREE_OBJECT(entry->ServerUsername);
	FREE_OBJECT(entry->ServerPasswd);
	FREE_OBJECT(entry->FailReason);
	FREE_OBJECT(entry->ServerPort);
    SNMP_FREE( entry );   /* XXX - release any other internal resources */
}

void dot11AcVersionAndConfigTable_load()
{

		snmp_log(LOG_DEBUG, "enter dot11AcVersionAndConfigTable_load\n");

		struct dot11AcVersionAndConfigTable_entry *temp;
		while( dot11AcVersionAndConfigTable_head )
		{
			temp=dot11AcVersionAndConfigTable_head->next;
			dot11AcVersionAndConfigTable_removeEntry(dot11AcVersionAndConfigTable_head);
			dot11AcVersionAndConfigTable_head=temp;
		}
		{
			int i =0;
			int ver_filetype = 0;
			int iss_filetype = 0;
			int ver_protocal = 0;
			int iss_protocal = 0;
			int running = 0;
			ST_VERSION_PARAM ver_param,iss_param;
			memset(&ver_param,0,sizeof(ST_VERSION_PARAM));  //存储取到的值
			for(i=0;i<2;i++)
			{
				if(i==0)
				{
					//download add here
					if(access(V_XML,0)!=-1)
					{	
						snmp_log(LOG_DEBUG, "enter get_version_param\n");
						get_version_param(&ver_param,V_XML);  //从xml文件中获得消息
						snmp_log(LOG_DEBUG, "exit get_version_param\n");
						
						if(strcmp(ver_param.filetype,T_IMG)==0)
						{
							ver_filetype = 1;
						}
						else if(strcmp(ver_param.filetype,T_LOG)==0)
						{
							ver_filetype = 3;
						}
						else if(strcmp(ver_param.filetype,T_CONF)==0)
						{
							ver_filetype = 2;
						}
							

						if(strcmp(ver_param.protocal,T_HTTP)==0)
						{
							ver_protocal = 2;
						}
						else if(strcmp(ver_param.protocal,T_FTP)==0)
						{
							ver_protocal = 1;
						}
						dot11AcVersionAndConfigTable_createEntry(
																1,
																((strcmp(ver_param.filename,"\n")==0)||(strcmp(ver_param.filename,"")==0))?"UNKNOWN":ver_param.filename,
																ver_filetype,
																0,
																ver_protocal,
																((strcmp(ver_param.routeip,"\n")==0)||(strcmp(ver_param.routeip,"")==0))?"UNKNOWN":ver_param.routeip,
																((strcmp(ver_param.routeport,"\n")==0)||(strcmp(ver_param.routeport,"")==0))?"UNKNOWN":ver_param.routeport,
																((strcmp(ver_param.username,"\n")==0)||(strcmp(ver_param.username,"")==0))?"UNKNOWN":ver_param.username,
																((strcmp(ver_param.password,"\n")==0)||(strcmp(ver_param.password,"")==0))?"UNKNOWN":ver_param.password,
																0,
																0,
																0,
																((strcmp(ver_param.failcause,"\n")==0)||(strcmp(ver_param.failcause,"")==0))?"UNKNOWN":ver_param.failcause);
					}
				}
				else if(i==1)
				{
					//download add here
					if(access(ISS_XML,0)!=-1)
					{
						snmp_log(LOG_DEBUG, "enter get_version_param\n");
						get_version_param(&iss_param,ISS_XML);  //从xml文件中获得消息
						snmp_log(LOG_DEBUG, "exit get_version_param\n");
						
						if(strcmp(iss_param.filetype,T_IMG)==0)
						{
							iss_filetype = 1;
						}
						else if(strcmp(iss_param.filetype,T_LOG)==0)
						{
							iss_filetype = 3;
						}
						else if(strcmp(iss_param.filetype,T_CONF)==0)
						{
							iss_filetype = 2;
						}

						if(strcmp(iss_param.protocal,T_HTTP)==0)
						{
							iss_protocal = 2;
						}
						else if(strcmp(iss_param.protocal,T_FTP)==0)
						{
							iss_protocal = 1;
						}

						dot11AcVersionAndConfigTable_createEntry(
																2,
																((strcmp(iss_param.filename,"\n")==0)||(strcmp(iss_param.filename,"")==0))?"UNKNOWN":iss_param.filename,
																iss_filetype,
																0,//while upload ,it must be set 0
																iss_protocal,
																((strcmp(iss_param.routeip,"\n")==0)||(strcmp(iss_param.routeip,"")==0))?"UNKNOWN":iss_param.routeip,
																((strcmp(iss_param.routeport,"\n")==0)||(strcmp(iss_param.routeport,"")==0))?"UNKNOWN":iss_param.routeport,
																((strcmp(iss_param.username,"\n")==0)||(strcmp(iss_param.username,"")==0))?"UNKNOWN":iss_param.username,
																((strcmp(iss_param.password,"\n")==0)||(strcmp(iss_param.password,"")==0))?"UNKNOWN":iss_param.password,
																0,
																0,
																0,
																((strcmp(iss_param.failcause,"\n")==0)||(strcmp(iss_param.failcause,"")==0))?"UNKNOWN":iss_param.failcause);
					}
				}
			}
		}
		
	snmp_log(LOG_DEBUG, "exit dot11AcVersionAndConfigTable_load\n");
}

/* Example iterator hook routines - using 'get_next' to do most of the work */
netsnmp_variable_list *
dot11AcVersionAndConfigTable_get_first_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{
	if(dot11AcVersionAndConfigTable_head==NULL)
		return NULL;
    *my_loop_context = dot11AcVersionAndConfigTable_head;
    *my_data_context = dot11AcVersionAndConfigTable_head;
    return dot11AcVersionAndConfigTable_get_next_data_point(my_loop_context, my_data_context,
                                    put_index_data,  mydata );
}

netsnmp_variable_list *
dot11AcVersionAndConfigTable_get_next_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{
    struct dot11AcVersionAndConfigTable_entry *entry = (struct dot11AcVersionAndConfigTable_entry *)*my_loop_context;
    netsnmp_variable_list *idx = put_index_data;

    if ( entry ) {
        snmp_set_var_value( idx, (u_char *)&entry->LoadFlag, sizeof(long) );
        idx = idx->next_variable;
        *my_data_context = (void *)entry;
        *my_loop_context = (void *)entry->next;
    } else {
        return NULL;
    }
	 return put_index_data; 
}


/** handles requests for the dot11AcVersionAndConfigTable table */
int
dot11AcVersionAndConfigTable_handler(
    netsnmp_mib_handler               *handler,
    netsnmp_handler_registration      *reginfo,
    netsnmp_agent_request_info        *reqinfo,
    netsnmp_request_info              *requests) {

    netsnmp_request_info       *request;
    netsnmp_table_request_info *table_info;
    struct dot11AcVersionAndConfigTable_entry          *table_entry;

    switch (reqinfo->mode) {
        /*
         * Read-support (also covers GetNext requests)
         */
    case MODE_GET:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11AcVersionAndConfigTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    if( !table_entry ){
        		netsnmp_set_request_error(reqinfo,request,SNMP_NOSUCHINSTANCE);
				continue;
	    	}     
            switch (table_info->colnum) {
            case COLUMN_LOADFLAG:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                           (u_char *)&table_entry->LoadFlag,
                                          sizeof(long));
                break;
            case COLUMN_FILENAME:
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                           (u_char *)table_entry->FileName,
                                          strlen(table_entry->FileName));
                break;
            case COLUMN_FILETYPE:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                           (u_char *)&table_entry->FileType,
                                          sizeof(long));
                break;
            case COLUMN_ESTIMATEFILESIZE:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                           (u_char *)&table_entry->EstimateFileSize,
                                          sizeof(long));
                break;
            case COLUMN_TRANSPROTOCOL:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                           (u_char *)&table_entry->TransProtocol,
                                          sizeof(long));
                break;
            case COLUMN_SERVERADDR:
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                           (u_char *)table_entry->ServerAddr,
                                          strlen(table_entry->ServerAddr));
                break;
            case COLUMN_SERVERPORT:
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                           (u_char *)table_entry->ServerPort,
                                          strlen(table_entry->ServerPort));
                break;
            case COLUMN_SERVERUSERNAME:
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                           (u_char *)table_entry->ServerUsername,
                                          strlen(table_entry->ServerUsername));
                break;
            case COLUMN_SERVERPASSWD:
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                           (u_char *)table_entry->ServerPasswd,
                                          strlen(table_entry->ServerPasswd));
                break;
            case COLUMN_TRANSSTATUS:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                           (u_char *)&table_entry->TransStatus,
                                          sizeof(long));
                break;
            case COLUMN_TOTALBYTES:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                           (u_char *)&table_entry->TotalBytes,
                                          sizeof(long));
                break;
            case COLUMN_TRANSFERBYTES:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                           (u_char *)&table_entry->TransferBytes,
                                          sizeof(long));
                break;
            case COLUMN_FAILREASON:
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                           (u_char *)table_entry->FailReason,
                                          strlen(table_entry->FailReason));
                break;
            }
        }
        break;

        /*
         * Write-support
         */
    case MODE_SET_RESERVE1:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11AcVersionAndConfigTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    
            switch (table_info->colnum) {
            case COLUMN_LOADFLAG:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_FILENAME:
                if ( request->requestvb->type != ASN_OCTET_STR ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_FILETYPE:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_ESTIMATEFILESIZE:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_TRANSPROTOCOL:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_SERVERADDR:
                if ( request->requestvb->type != ASN_OCTET_STR ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_SERVERPORT:
                if ( request->requestvb->type != ASN_OCTET_STR) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_SERVERUSERNAME:
                if ( request->requestvb->type != ASN_OCTET_STR ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_SERVERPASSWD:
                if ( request->requestvb->type != ASN_OCTET_STR ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_TOTALBYTES:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_FAILREASON:
                if ( request->requestvb->type != ASN_OCTET_STR ) {
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
				table_entry = (struct dot11AcVersionAndConfigTable_entry *)
				netsnmp_extract_iterator_context(request);
				table_info  =     netsnmp_extract_table_info(      request);

				if( !table_entry ){
	        		netsnmp_set_request_error(reqinfo,request,SNMP_NOSUCHINSTANCE);
					continue;
		    	}   

				switch (table_info->colnum) 
				{
					case COLUMN_FILENAME:
					{
						int ret=1;						
						char *input_string = (char *)malloc(request->requestvb->val_len+1);
						if(NULL == input_string) {
							netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
							break;
						}
						memset(input_string,0,request->requestvb->val_len+1);
						strncpy(input_string,request->requestvb->val.string,request->requestvb->val_len);

						if(table_entry->LoadFlag == 1)
						{
							ret=set_one_node(V_XML,R_FN,input_string);
						}
						else if(table_entry->LoadFlag == 2)
						{
							ret=set_one_node(ISS_XML,R_FN,input_string);
						}
						if(ret != 0)
						{
							netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
						}

						FREE_OBJECT(input_string);
					}
					break;
					case COLUMN_FILETYPE:
					{
						int ret=1;
						if(table_entry->LoadFlag==1)
						{
							if(*request->requestvb->val.integer == 1)
							{
								ret=set_one_node(V_XML,R_FT,T_IMG);
							}
							else if(*request->requestvb->val.integer==2)
							{
								ret=set_one_node(V_XML,R_FT,T_CONF);
							}
							else if(*request->requestvb->val.integer==3)
							{
								ret=set_one_node(V_XML,R_FT,T_LOG);
							}
						}
						else if(table_entry->LoadFlag == 2)
						{
							if(*request->requestvb->val.integer==1)
							{
								ret=set_one_node(ISS_XML,R_FT,T_IMG);
							}
							else if(*request->requestvb->val.integer==2)
							{
								ret=set_one_node(ISS_XML,R_FT,T_CONF);
							}
							else if(*request->requestvb->val.integer==3)
							{
								ret=set_one_node(ISS_XML,R_FT,T_LOG);
							}
						}
						if(ret != 0)
						{
							netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
						}
					}
					break;
					case COLUMN_ESTIMATEFILESIZE:
					{
					}
					break;
					case COLUMN_TRANSPROTOCOL:
					{
						int ret=1;
						if(table_entry->LoadFlag==1)
						{
							if(*request->requestvb->val.integer ==1 )
							{
								ret=set_one_node(V_XML,R_PRO,T_FTP);
							}
							else if(*request->requestvb->val.integer ==2)
							{
								ret=set_one_node(V_XML,R_PRO,T_HTTP);
							}
						}
						else if(table_entry->LoadFlag ==2)
						{
							if(*request->requestvb->val.integer ==1 )
							{
								ret=set_one_node(ISS_XML,R_PRO,T_FTP);
							}
							else if(*request->requestvb->val.integer ==2)
							{
								ret=set_one_node(ISS_XML,R_PRO,T_HTTP);
							}
						}
						if(ret != 0)
						{
							netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
						}
					}
					break;
					case COLUMN_SERVERADDR:
					{
						int ret=1;
						char * input_string = (char *)malloc(request->requestvb->val_len+1);						
						if(NULL == input_string) {
							netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
							break;
						}
						
						memset(input_string,0,request->requestvb->val_len+1);
						strncpy(input_string,request->requestvb->val.string,request->requestvb->val_len);
						
						if(table_entry->LoadFlag==1)
						{
							ret=set_one_node(V_XML,R_RI,input_string);
						}
						else if(table_entry->LoadFlag ==2)
						{
							ret=set_one_node(ISS_XML,R_RI,input_string);
						}
						if(ret != 0)
						{
							netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
						}

						FREE_OBJECT(input_string);
					}
					break;
					case COLUMN_SERVERPORT:
					{
						int ret=1;
						char * input_string = (char *)malloc(request->requestvb->val_len+1);						
						if(NULL == input_string) {
							netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
							break;
						}
						
						memset(input_string,0,request->requestvb->val_len+1);
						strncpy(input_string,request->requestvb->val.string,request->requestvb->val_len);
						
						if(table_entry->LoadFlag==1)
						{
							ret=set_one_node(V_XML,R_RP,input_string);
						}
						else if(table_entry->LoadFlag ==2)
						{
							ret=set_one_node(ISS_XML,R_RP,input_string);
						}
						if(ret != 0)
						{
							netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
						}

						FREE_OBJECT(input_string);
					}
					break;
					case COLUMN_SERVERUSERNAME:
					{
						int ret=1;
						char * input_string = (char *)malloc(request->requestvb->val_len+1);						
						if(NULL == input_string) {
							netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
							break;
						}
						
						memset(input_string,0,request->requestvb->val_len+1);
						strncpy(input_string,request->requestvb->val.string,request->requestvb->val_len);
						
						if(table_entry->LoadFlag==1)
						{
							ret=set_one_node(V_XML,R_UN,input_string);
						}
						else if(table_entry->LoadFlag ==2)
						{
							ret=set_one_node(ISS_XML,R_UN,input_string);
						}
						if(ret != 0)
						{
							netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
						}

						FREE_OBJECT(input_string);
					}
					break;
					case COLUMN_SERVERPASSWD:
					{
						int ret=1;
						char * input_string = (char *)malloc(request->requestvb->val_len+1);
						if(NULL == input_string) {
							netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
							break;
						}

						memset(input_string,0,request->requestvb->val_len+1);
						strncpy(input_string,request->requestvb->val.string,request->requestvb->val_len);
						
						if(table_entry->LoadFlag==1)
						{
							ret=set_one_node(V_XML,R_PS,input_string);
						}
						else if(table_entry->LoadFlag ==2)
						{
							ret=set_one_node(ISS_XML,R_PS,input_string);
						}
						if(ret != 0)
						{
							netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
						}

						FREE_OBJECT(input_string);
					}
					break;
					case COLUMN_TOTALBYTES:
					{
					}
					break;
					case COLUMN_FAILREASON:
					{
						int ret=1;
						char * input_string = (char *)malloc(request->requestvb->val_len+1);
						if(NULL == input_string) {
							netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
							break;
						}
						
						memset(input_string,0,request->requestvb->val_len+1);
						strncpy(input_string,request->requestvb->val.string,request->requestvb->val_len);
						
						if(table_entry->LoadFlag==1)
						{
							ret=set_one_node(V_XML,R_FC,input_string);
						}
						else if(table_entry->LoadFlag ==2)
						{
							ret=set_one_node(ISS_XML,R_FC,input_string);
						}
						if(ret != 0)
						{
							netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_WRONGTYPE);
						}

						FREE_OBJECT(input_string);
					}
					break;
				}
			}
		}
		break;

    case MODE_SET_UNDO:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11AcVersionAndConfigTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    
            switch (table_info->colnum) {
            case COLUMN_FILENAME:
                /* Need to restore old 'table_entry->FileName' value.
                   May need to use 'memcpy' */
                //table_entry->FileName = table_entry->old_FileName;
                break;
            case COLUMN_FILETYPE:
                /* Need to restore old 'table_entry->FileType' value.
                   May need to use 'memcpy' */
                //table_entry->FileType = table_entry->old_FileType;
                break;
            case COLUMN_ESTIMATEFILESIZE:
                /* Need to restore old 'table_entry->EstimateFileSize' value.
                   May need to use 'memcpy' */
                //table_entry->EstimateFileSize = table_entry->old_EstimateFileSize;
                break;
            case COLUMN_TRANSPROTOCOL:
                /* Need to restore old 'table_entry->TransProtocol' value.
                   May need to use 'memcpy' */
                //table_entry->TransProtocol = table_entry->old_TransProtocol;
                break;
            case COLUMN_SERVERADDR:
                /* Need to restore old 'table_entry->ServerAddr' value.
                   May need to use 'memcpy' */
                //table_entry->ServerAddr = table_entry->old_ServerAddr;
                break;
            case COLUMN_SERVERPORT:
                /* Need to restore old 'table_entry->ServerPort' value.
                   May need to use 'memcpy' */
                //table_entry->ServerPort = table_entry->old_ServerPort;
                break;
            case COLUMN_SERVERUSERNAME:
                /* Need to restore old 'table_entry->ServerUsername' value.
                   May need to use 'memcpy' */
                //table_entry->ServerUsername = table_entry->old_ServerUsername;
                break;
            case COLUMN_SERVERPASSWD:
                /* Need to restore old 'table_entry->ServerPasswd' value.
                   May need to use 'memcpy' */
                //table_entry->ServerPasswd = table_entry->old_ServerPasswd;
                break;
            case COLUMN_TOTALBYTES:
                /* Need to restore old 'table_entry->TotalBytes' value.
                   May need to use 'memcpy' */
                //table_entry->TotalBytes = table_entry->old_TotalBytes;
                break;
            case COLUMN_FAILREASON:
                /* Need to restore old 'table_entry->FailReason' value.
                   May need to use 'memcpy' */
                //table_entry->FailReason = table_entry->old_FailReason;
                break;
            }
        }
        break;

    case MODE_SET_COMMIT:
        break;
    }
    return SNMP_ERR_NOERROR;
}
