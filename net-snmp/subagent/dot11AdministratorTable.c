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
* dot11AdministratorTable.c
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
#include "dot11AdministratorTable.h"
#include "autelanWtpGroup.h"
#include "ws_usrinfo.h"
#include <grp.h>

/** Initializes the dot11AdministratorTable module */

#define DOT11ADMINISTRATORTABLE "2.6.2"

    /* Typical data structure for a row entry */
struct dot11AdministratorTable_entry {
    /* Index values */
    long administratorID;

    /* Column values */
    char *administratorName;
    char *administratorPassword;
 
    long administratorPrivilege;
    long administratorRowStatus;

    /* Illustrate using a simple linked list */
    int   valid;
    struct dot11AdministratorTable_entry *next;
};

void dot11AdministratorTable_load();
void
dot11AdministratorTable_removeEntry( struct dot11AdministratorTable_entry *entry );

void
init_dot11AdministratorTable(void)
{
  /* here we initialize all the tables we're planning on supporting */
    initialize_table_dot11AdministratorTable();
}

/** Initialize the dot11AdministratorTable table by defining its contents and how it's structured */
void
initialize_table_dot11AdministratorTable(void)
{
    static oid dot11AdministratorTable_oid[128] = {0};
    size_t dot11AdministratorTable_oid_len   = 0;
	
	mad_dev_oid(dot11AdministratorTable_oid,DOT11ADMINISTRATORTABLE,&dot11AdministratorTable_oid_len,enterprise_pvivate_oid);
    netsnmp_handler_registration    *reg;
    netsnmp_iterator_info           *iinfo;
    netsnmp_table_registration_info *table_info;

    reg = netsnmp_create_handler_registration(
              "dot11AdministratorTable",     dot11AdministratorTable_handler,
              dot11AdministratorTable_oid, dot11AdministratorTable_oid_len,
              HANDLER_CAN_RWRITE
              );

    table_info = SNMP_MALLOC_TYPEDEF( netsnmp_table_registration_info );
    netsnmp_table_helper_add_indexes(table_info,
                           ASN_INTEGER,  /* index: administratorID */
                           0);
    table_info->min_column = ADMIN_MIN;
    table_info->max_column = ADMIN_MAX;
    
    iinfo = SNMP_MALLOC_TYPEDEF( netsnmp_iterator_info );
    iinfo->get_first_data_point = dot11AdministratorTable_get_first_data_point;
    iinfo->get_next_data_point  = dot11AdministratorTable_get_next_data_point;
    iinfo->table_reginfo        = table_info;
    
    netsnmp_register_table_iterator( reg, iinfo );
	netsnmp_inject_handler(reg,netsnmp_get_cache_handler(DOT1DTPFDBTABLE_CACHE_TIMEOUT,dot11AdministratorTable_load, dot11AdministratorTable_removeEntry,
							dot11AdministratorTable_oid, dot11AdministratorTable_oid_len));

    /* Initialise the contents of the table here */
}


struct dot11AdministratorTable_entry  *dot11AdministratorTable_head;

/* create a new row in the (unsorted) table */
struct dot11AdministratorTable_entry *
dot11AdministratorTable_createEntry(
                 long  administratorID,
                 char *administratorName,
                 char *administratorPassword,
			     long administratorPrivilege,
			     long administratorRowStatus
                ) {
    struct dot11AdministratorTable_entry *entry;

    entry = SNMP_MALLOC_TYPEDEF(struct dot11AdministratorTable_entry);
    if (!entry)
        return NULL;

    entry->administratorID = administratorID;
	entry->administratorName = strdup(administratorName);
    entry->administratorPassword = strdup(administratorPassword);
	entry->administratorPrivilege = administratorPrivilege;
	entry->administratorRowStatus = administratorRowStatus;
    entry->next = dot11AdministratorTable_head;
    dot11AdministratorTable_head = entry;
    return entry;
}

/* remove a row from the table */
void
dot11AdministratorTable_removeEntry( struct dot11AdministratorTable_entry *entry ) {
    struct dot11AdministratorTable_entry *ptr, *prev;

    if (!entry)
        return;    /* Nothing to remove */

    for ( ptr  = dot11AdministratorTable_head, prev = NULL;
          ptr != NULL;
          prev = ptr, ptr = ptr->next ) {
        if ( ptr == entry )
            break;
    }
    if ( !ptr )
        return;    /* Can't find it */

    if ( prev == NULL )
        dot11AdministratorTable_head = ptr->next;
    else
        prev->next = ptr->next;
 	FREE_OBJECT(entry->administratorName);
	FREE_OBJECT(entry->administratorPassword);
    SNMP_FREE( entry );   /* XXX - release any other internal resources */
}

void dot11AdministratorTable_load()
{
	snmp_log(LOG_DEBUG, "enter dot11AdministratorTable_load\n");

#if 1
// get all user
{
		int cur_index=0;
		
#define add_group_all_user_to_entry( groupname, Privilege )\
		{\
			struct group *grentry = NULL;\
			char *ptr=NULL;\
			int i;\
			grentry = getgrnam(groupname);\
			if (grentry)\
			{\
				for(i=0;(ptr=grentry->gr_mem[i])!=NULL;i++)\
				{\
					dot11AdministratorTable_createEntry( cur_index + 1,\
														 ptr,\
														 "******",\
														 Privilege,\
														 1);\
					cur_index++;\
				}\
			}\
		}
		
		
		add_group_all_user_to_entry(ADMINGROUP, 1);
		add_group_all_user_to_entry(VIEWGROUP, 2);
}
#else
		dot11AdministratorTable_createEntry(1,"2","3");
#endif		

		snmp_log(LOG_DEBUG, "exit dot11AdministratorTable_load\n");
}

/* Example iterator hook routines - using 'get_next' to do most of the work */
netsnmp_variable_list *
dot11AdministratorTable_get_first_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{
	if(dot11AdministratorTable_head==NULL)
			return NULL;
	*my_data_context = dot11AdministratorTable_head;
    *my_loop_context = dot11AdministratorTable_head;
    return dot11AdministratorTable_get_next_data_point(my_loop_context, my_data_context,
                                    put_index_data,  mydata );
}

netsnmp_variable_list *
dot11AdministratorTable_get_next_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{
    struct dot11AdministratorTable_entry *entry = (struct dot11AdministratorTable_entry *)*my_loop_context;
    netsnmp_variable_list *idx = put_index_data;

    if ( entry ) {
        snmp_set_var_value( idx,(u_char*)&entry->administratorID, sizeof(entry->administratorID) );
        idx = idx->next_variable;
        *my_data_context = (void *)entry;
        *my_loop_context = (void *)entry->next;
    } else {
        return NULL;
    }
	return put_index_data;
}


/** handles requests for the dot11AdministratorTable table */
int
dot11AdministratorTable_handler(
    netsnmp_mib_handler               *handler,
    netsnmp_handler_registration      *reginfo,
    netsnmp_agent_request_info        *reqinfo,
    netsnmp_request_info              *requests) {

    netsnmp_request_info       *request;
    netsnmp_table_request_info *table_info;
    struct dot11AdministratorTable_entry          *table_entry;

    switch (reqinfo->mode) {
        /*
         * Read-support (also covers GetNext requests)
         */
    case MODE_GET:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11AdministratorTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
			
   			 if( !table_entry ){
		       	netsnmp_set_request_error(reqinfo,request,SNMP_NOSUCHINSTANCE);
				continue;
		    }
			 
            switch (table_info->colnum) {
            case COLUMN_ADMINISTRATORID:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->administratorID,
                                          sizeof(table_entry->administratorID));
                break;
            case COLUMN_ADMINISTRATORNAME:
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char*)table_entry->administratorName,
                                          strlen(table_entry->administratorName));
                break;
            case COLUMN_ADMINISTRATORPASSWORD:
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char*)table_entry->administratorPassword,
                                          strlen(table_entry->administratorPassword));
                break;
            case COLUMN_ADMINISTRATORPRIVILEGE:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->administratorPrivilege,
                                          sizeof(long));
                break;
            case COLUMN_ADMINISTRATORROWSTATUS:
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->administratorRowStatus,
                                          sizeof(long));
                break;
			 default:	
             	netsnmp_set_request_error( reqinfo, request,
                                           SNMP_ERR_NOTWRITABLE );
                return SNMP_ERR_NOERROR;
            }
        }
        break;

        /*
         * Write-support
         */
    case MODE_SET_RESERVE1:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11AdministratorTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    
            switch (table_info->colnum) {
            case COLUMN_ADMINISTRATORNAME:
                if ( request->requestvb->type != ASN_OCTET_STR ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_ADMINISTRATORPASSWORD:
                if ( request->requestvb->type != ASN_OCTET_STR ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_ADMINISTRATORPRIVILEGE:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                break;
            case COLUMN_ADMINISTRATORROWSTATUS:
                if ( request->requestvb->type != ASN_INTEGER ) {
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGTYPE );
                    return SNMP_ERR_NOERROR;
                }
                /* Also may need to check size/value */
                switch (*request->requestvb->val.integer) {
                case RS_ACTIVE:
                case RS_NOTINSERVICE:
                    if (!table_entry) {
                        netsnmp_set_request_error( reqinfo, request,
                                                   SNMP_ERR_INCONSISTENTVALUE );
                        return SNMP_ERR_NOERROR;
                    }
                    break;
                case RS_CREATEANDGO:
                case RS_CREATEANDWAIT:
                    if (table_entry) {
                        netsnmp_set_request_error( reqinfo, request,
                                                   SNMP_ERR_INCONSISTENTVALUE );
                        return SNMP_ERR_NOERROR;
                    }
                    break;
                case RS_DESTROY:
                    /* Valid in all circumstances */
                    break;
                case RS_NOTREADY:
                default:
                    netsnmp_set_request_error( reqinfo, request,
                                               SNMP_ERR_WRONGVALUE );
                    return SNMP_ERR_NOERROR;
                    break;
                }
                break;
            default:
                netsnmp_set_request_error( reqinfo, request,
                                           SNMP_ERR_NOTWRITABLE );
                return SNMP_ERR_NOERROR;
            }
        }
        break;

    case MODE_SET_RESERVE2:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11AdministratorTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    
            switch (table_info->colnum) {
            case COLUMN_ADMINISTRATORROWSTATUS:
                switch (*request->requestvb->val.integer) {
                case RS_CREATEANDGO:
                case RS_CREATEANDWAIT:
					dot11AdministratorTable_createEntry(*table_info->indexes->val.integer,
														"",
														"",
														0,
														3);
					break;
                }
			break;
            }
        }
        break;

    case MODE_SET_FREE:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11AdministratorTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    
            switch (table_info->colnum) {
            case COLUMN_ADMINISTRATORROWSTATUS:
                switch (*request->requestvb->val.integer) {
                case RS_CREATEANDGO:
                case RS_CREATEANDWAIT:
					break;
                }
            }
        }
        break;

    case MODE_SET_ACTION:
{
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11AdministratorTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    		
            switch (table_info->colnum) {
			case COLUMN_ADMINISTRATORID:
			{
                /* Need to save old 'table_entry->wtpID' value.
                   May need to use 'memcpy' */
                //table_entry->old_wtpID = table_entry->wtpID;
                if(table_entry)
                {
					table_entry->administratorID= *request->requestvb->val.integer;
                }
				else
				{
					netsnmp_set_request_error(reqinfo,request,SNMP_NOSUCHINSTANCE);
				}
			}
                break;
            case COLUMN_ADMINISTRATORNAME:
			{
                /* Need to save old 'table_entry->administratorName' value.
                   May need to use 'memcpy' */
				char input_string[256] = { 0 };
				memset(input_string,0,sizeof(input_string));
				strncpy(input_string,request->requestvb->val.string,request->requestvb->val_len);

				if(table_entry)
                {
					if(table_entry->administratorRowStatus == 1)
					{						
						int ret = 1,status = 0;
						char usrnamez[32] = { 0 };
						memset(usrnamez,0,sizeof(usrnamez));
						char priv[32] = { 0 };
						memset(priv,0,sizeof(priv));
						char cmd[128] = { 0 };
						memset(cmd,0,sizeof(cmd));

						if(table_entry->administratorName!= NULL)
						{
							strncpy(usrnamez,table_entry->administratorName,sizeof(usrnamez)-1);
						}

						//删除用户先修改权限，再进行删除					
						if(table_entry->administratorPrivilege == 1)
						{			
						    memset(cmd,0,sizeof(cmd));
							memset(priv,0,sizeof(priv));
							strncpy(priv,"view",sizeof(priv)-1);
							strncat(cmd,"userrole.sh",sizeof(cmd)-strlen(cmd)-1);
							strncat(cmd," ",sizeof(cmd)-strlen(cmd)-1);
							strncat(cmd,usrnamez,sizeof(cmd)-strlen(cmd)-1);
							strncat(cmd," ",sizeof(cmd)-strlen(cmd)-1);
							strncat(cmd,priv,sizeof(cmd)-strlen(cmd)-1);
							system(cmd);
							//ret = WEXITSTATUS(status);
						}
						memset(cmd,0,sizeof(cmd));
						strncat(cmd,"userdel.sh",sizeof(cmd)-strlen(cmd)-1);		
						strncat(cmd," ",sizeof(cmd)-strlen(cmd)-1);
						strncat(cmd,usrnamez,sizeof(cmd)-strlen(cmd)-1);						
						system(cmd);		 
						//ret = WEXITSTATUS(status);						

						if(strcmp(input_string,""))
						{
							//添加用户
							memset(cmd,0,sizeof(cmd));
							strncat(cmd,"useradd.sh",sizeof(cmd)-strlen(cmd)-1);
							strncat(cmd," ",sizeof(cmd)-strlen(cmd)-1);
							strncat(cmd,input_string,sizeof(cmd)-strlen(cmd)-1);
							strncat(cmd," ",sizeof(cmd)-strlen(cmd)-1);
							strncat(cmd,"123456",sizeof(cmd)-strlen(cmd)-1);
							strncat(cmd," ",sizeof(cmd)-strlen(cmd)-1);
							if(table_entry->administratorPrivilege == 1)
							{
								strncat(cmd,"enable",sizeof(cmd)-strlen(cmd)-1);
							}
							else
							{
								strncat(cmd,"view",sizeof(cmd)-strlen(cmd)-1);
							}
							strncat(cmd," ",sizeof(cmd)-strlen(cmd)-1);
							strncat(cmd,"normal",sizeof(cmd)-strlen(cmd)-1);

							status = system(cmd);		 
							ret = WEXITSTATUS(status);
							if(ret == 0)
							{
								if(table_entry->administratorName!= NULL)
								{
									FREE_OBJECT(table_entry->administratorName);
								}
								table_entry->administratorName = strdup(input_string);
							}						   
						 }
					}
					else
					{
						if(table_entry->administratorName!= NULL)
						{
							free(table_entry->administratorName);
						}
						table_entry->administratorName	   = strdup(input_string);
					}
                }
				else
				{
					netsnmp_set_request_error(reqinfo,request,SNMP_NOSUCHINSTANCE);
				}
            }
                break;
            case COLUMN_ADMINISTRATORPASSWORD:
			{
				/* Need to save old 'table_entry->administratorPassword' value.
				   May need to use 'memcpy' */
			 //   table_entry->old_administratorPassword = table_entry->administratorPassword;
			//	  table_entry->administratorPassword	 = request->requestvb->val.YYY;
				char input_string[256] = { 0 };
				memset(input_string,0,sizeof(input_string));
				strncpy(input_string,request->requestvb->val.string,request->requestvb->val_len);

				if(table_entry)
                {
					if(table_entry->administratorRowStatus == 1)
					{			
						#define CMD_MODIFY_PASSWD	"sudo /usr/bin/chpass.sh %s %s normal"
						char cmd_modify[256] = { 0 };
						int ret = 1,status = 0;
								
						if( strlen(input_string) > sizeof(cmd_modify) - strlen(CMD_MODIFY_PASSWD) - strlen(table_entry->administratorName))
						{
							netsnmp_set_request_error(reqinfo,request,SNMP_ERR_WRONGTYPE);
							break;//超过了最大长度，
						}
						snprintf( cmd_modify,sizeof(cmd_modify)-1, CMD_MODIFY_PASSWD, table_entry->administratorName, input_string);
						status = system( cmd_modify );
						ret = WEXITSTATUS(status);				
						if(ret == 0)
						{
							if(table_entry->administratorPassword!= NULL)
							{
								FREE_OBJECT(table_entry->administratorPassword);
							}
							table_entry->administratorPassword = strdup(input_string);
						}						   
					}
					else
					{
						if(table_entry->administratorPassword!= NULL)
						{
							free(table_entry->administratorPassword);
						}
						table_entry->administratorPassword	   = strdup(input_string);
					}
                }
				else
				{
					netsnmp_set_request_error(reqinfo,request,SNMP_NOSUCHINSTANCE);
				}
			}
                break;
            case COLUMN_ADMINISTRATORPRIVILEGE:
			{
                /* Need to save old 'table_entry->administratorPrivilege' value.
                   May need to use 'memcpy' */
				if(table_entry)
                {
					if(table_entry->administratorRowStatus == 1)
					{			
						/*修改权限*/	
						int ret = 1,status = 0;
						char usrnamez[32] = { 0 };
						memset(usrnamez,0,sizeof(usrnamez));
	                    char priv[32] = { 0 };
						memset(priv,0,sizeof(priv));
						char cmd[128] = { 0 };
						memset(cmd,0,sizeof(cmd));
						
						if(*request->requestvb->val.integer == 1)
							strncpy(priv,"enable",sizeof(priv)-1);
						else if(*request->requestvb->val.integer == 2)
							strncpy(priv,"view",sizeof(priv)-1);
						
						strncat(cmd,"userrole.sh",sizeof(cmd)-strlen(cmd)-1);
						strncat(cmd," ",sizeof(cmd)-strlen(cmd)-1);
						strncat(cmd,table_entry->administratorName,sizeof(cmd)-strlen(cmd)-1);
						strncat(cmd," ",sizeof(cmd)-strlen(cmd)-1);
						strncat(cmd,priv,sizeof(cmd)-strlen(cmd)-1);
						status = system(cmd);
					    ret = WEXITSTATUS(status);
						if(ret == 0)
						{
							table_entry->administratorPrivilege = *request->requestvb->val.integer;
						}						   
					}
					else
					{
						table_entry->administratorPrivilege = *request->requestvb->val.integer;
					}
                }
				else
				{
					netsnmp_set_request_error(reqinfo,request,SNMP_NOSUCHINSTANCE);
				}
            }
                break;
            }
        }
        /* Check the internal consistency of an active row */
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11AdministratorTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    
            switch (table_info->colnum) {
            case COLUMN_ADMINISTRATORROWSTATUS:
                switch (*request->requestvb->val.integer) {
                case RS_ACTIVE:
                case RS_CREATEANDGO:
                    //if (/* XXX */) {
                      //  netsnmp_set_request_error( reqinfo, request,
                        //                           SNMP_ERR_INCONSISTENTVALUE );
                        //return SNMP_ERR_NOERROR;
                    //}
                    break;
                }
            }
        }
}
        break;

    case MODE_SET_UNDO:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11AdministratorTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    
            switch (table_info->colnum) {
            case COLUMN_ADMINISTRATORNAME:
                /* Need to restore old 'table_entry->administratorName' value.
                   May need to use 'memcpy' */
                //table_entry->administratorName = table_entry->old_administratorName;
                break;
            case COLUMN_ADMINISTRATORPASSWORD:
                /* Need to restore old 'table_entry->administratorPassword' value.
                   May need to use 'memcpy' */
                //table_entry->administratorPassword = table_entry->old_administratorPassword;
                break;
            case COLUMN_ADMINISTRATORPRIVILEGE:
                /* Need to restore old 'table_entry->administratorPrivilege' value.
                   May need to use 'memcpy' */
                //table_entry->administratorPrivilege = table_entry->old_administratorPrivilege;
                break;
            case COLUMN_ADMINISTRATORROWSTATUS:
                switch (*request->requestvb->val.integer) {
                case RS_CREATEANDGO:
                case RS_CREATEANDWAIT:
                    //if (table_entry && !table_entry->valid) {
                      //  dot11AdministratorTable_removeEntry(table_data, table_row );
                    //}
                    break;
                }
                break;
            }
        }
        break;

    case MODE_SET_COMMIT:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11AdministratorTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
			
			if( !table_entry ){
					continue;
				} 
			    
            switch (table_info->colnum) {
            case COLUMN_ADMINISTRATORROWSTATUS:
                switch (*request->requestvb->val.integer) {
                case RS_CREATEANDGO:
                    table_entry->valid = 1;
                    /* Fall-through */
					break;
                case RS_ACTIVE:                    
				{
					if((strcmp(table_entry->administratorName,"")!=0)&&(strcmp(table_entry->administratorPassword,"")!=0)&&(table_entry->administratorPrivilege != 0))
					{
						int ret = 1,status = 0;
						char priv[32] = { 0 };
						memset(priv,0,sizeof(priv));
						char cmd[128] = { 0 };
						memset(cmd,0,sizeof(cmd));

					   //添加用户
						if(table_entry->administratorPrivilege == 1)
						  strncpy(priv,"enable",sizeof(priv)-1);
						else if(table_entry->administratorPrivilege == 2)
						  strncpy(priv,"view",sizeof(priv)-1);

						strncat(cmd,"useradd.sh",sizeof(cmd)-strlen(cmd)-1);
						strncat(cmd," ",sizeof(cmd)-strlen(cmd)-1);
						strncat(cmd,table_entry->administratorName,sizeof(cmd)-strlen(cmd)-1);
						strncat(cmd," ",sizeof(cmd)-strlen(cmd)-1);
						strncat(cmd,table_entry->administratorPassword,sizeof(cmd)-strlen(cmd)-1);
						strncat(cmd," ",sizeof(cmd)-strlen(cmd)-1);
						strncat(cmd,priv,sizeof(cmd)-strlen(cmd)-1);
						strncat(cmd," ",sizeof(cmd)-strlen(cmd)-1);
						strncat(cmd,"normal",sizeof(cmd)-strlen(cmd)-1);

						status = system(cmd);		 
						ret = WEXITSTATUS(status);	
						
						{
						   	struct dot11AdministratorTable_entry *temp; 
							while( dot11AdministratorTable_head )
							{
								temp=dot11AdministratorTable_head->next;
								dot11AdministratorTable_removeEntry(dot11AdministratorTable_head);
								dot11AdministratorTable_head=temp;
							}						
						}
						
						if(ret == 0)
						{
							table_entry->administratorRowStatus= RS_ACTIVE;
						}
						else
						{
							table_entry->administratorRowStatus = RS_NOTREADY;
						}
					}
					else
					{
						table_entry->administratorRowStatus = RS_NOTREADY;
					}
				}                    
                    break;

                case RS_CREATEANDWAIT:
                    table_entry->valid = 1;
                    /* Fall-through */
					break;
                case RS_NOTINSERVICE:
                    table_entry->administratorRowStatus = RS_NOTINSERVICE;
                    break;

                case RS_DESTROY:                
				{
					if(strcmp(table_entry->administratorName,""))
					{
						int ret = 1,status = 0;
						char cmd[128] = { 0 };
						memset(cmd,0,sizeof(cmd));

						
						/*删除用户*/
						if(table_entry->administratorPrivilege == 1)
						{			
							strncat(cmd,"userrole.sh",sizeof(cmd)-strlen(cmd)-1);
							strncat(cmd," ",sizeof(cmd)-strlen(cmd)-1);
							strncat(cmd,table_entry->administratorName,sizeof(cmd)-strlen(cmd)-1);
							strncat(cmd," ",sizeof(cmd)-strlen(cmd)-1);
							strncat(cmd,"view",sizeof(cmd)-strlen(cmd)-1);
							system(cmd);
							//ret = WEXITSTATUS(status);
						}
						memset(cmd,0,sizeof(cmd));
						strncat(cmd,"userdel.sh",sizeof(cmd)-strlen(cmd)-1);
						strncat(cmd," ",sizeof(cmd)-strlen(cmd)-1);
						strncat(cmd,table_entry->administratorName,sizeof(cmd)-strlen(cmd)-1);
						status = system(cmd);		 
						ret = WEXITSTATUS(status);						
						if(ret == 0)
						{
							struct dot11AdministratorTable_entry *temp; 
							 while( dot11AdministratorTable_head )
							 {
								 temp=dot11AdministratorTable_head->next;
								 dot11AdministratorTable_removeEntry(dot11AdministratorTable_head);
								 dot11AdministratorTable_head=temp;
							 }
						}
					}
				}
                break;
                }
            }
        }
        break;
    }
    return SNMP_ERR_NOERROR;
}
