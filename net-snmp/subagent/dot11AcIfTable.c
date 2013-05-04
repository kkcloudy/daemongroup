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
* dot11AcIfTable.c
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
#include "dot11AcIfTable.h"
#include "autelanWtpGroup.h"
#include "wcpss/asd/asd.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/dcli_wid_wtp.h"
#include "dbus/wcpss/dcli_wid_wlan.h"
#include "ws_dcli_wlans.h"
#include "ws_sysinfo.h"
#include "ws_dcli_portconf.h"
#include "mibs_public.h"
#include "ws_sta.h"


#define ACIFTABLE 	"2.4.2"

    /* Typical data structure for a row entry */
struct dot11AcIfTable_entry {
    /* Index values */
    long ifIndex;

    /* Column values */
	u_int32_t value;
    char *ifDescr;
    long ifType;
    long ifMTU;
    u_long ifSpeed;
    unsigned char ifPhysAddress[6];
    long ifAdminStatus;
    //long old_ifAdminStatus;
    long ifOperStatus;
    u_long ifLastChange;

    /* Illustrate using a simple linked list */
    int   valid;
    struct dot11AcIfTable_entry *next;
};

void dot11AcIfTable_head_load();
void
dot11AcIfTable_removeEntry( struct dot11AcIfTable_entry *entry );

/** Initializes the dot11AcIfTable module */
void
init_dot11AcIfTable(void)
{
  /* here we initialize all the tables we're planning on supporting */
    initialize_table_dot11AcIfTable();
}

/** Initialize the dot11AcIfTable table by defining its contents and how it's structured */
void
initialize_table_dot11AcIfTable(void)
{
    static oid dot11AcIfTable_oid[128] = {0};
    size_t dot11AcIfTable_oid_len   = 0;
	mad_dev_oid(dot11AcIfTable_oid,ACIFTABLE,&dot11AcIfTable_oid_len,enterprise_pvivate_oid);
    netsnmp_handler_registration    *reg;
    netsnmp_iterator_info           *iinfo;
    netsnmp_table_registration_info *table_info;

    reg = netsnmp_create_handler_registration(
              "dot11AcIfTable",     dot11AcIfTable_handler,
              dot11AcIfTable_oid, dot11AcIfTable_oid_len,
              HANDLER_CAN_RWRITE
              );

    table_info = SNMP_MALLOC_TYPEDEF( netsnmp_table_registration_info );
    netsnmp_table_helper_add_indexes(table_info,
                           ASN_INTEGER,  /* index: ifIndex */
                           0);
    table_info->min_column = COLUMN_ACIFMIN;
    table_info->max_column = COLUMN_ACIFMAX;
    
    iinfo = SNMP_MALLOC_TYPEDEF( netsnmp_iterator_info );
    iinfo->get_first_data_point = dot11AcIfTable_get_first_data_point;
    iinfo->get_next_data_point  = dot11AcIfTable_get_next_data_point;
    iinfo->table_reginfo        = table_info;
    
    netsnmp_register_table_iterator( reg, iinfo );
	netsnmp_inject_handler(reg,netsnmp_get_cache_handler(DOT1DTPFDBTABLE_CACHE_TIMEOUT,dot11AcIfTable_head_load, dot11AcIfTable_removeEntry,
							dot11AcIfTable_oid, dot11AcIfTable_oid_len));

    /* Initialise the contents of the table here */
}



struct dot11AcIfTable_entry  *dot11AcIfTable_head;

/* create a new row in the (unsorted) table */
struct dot11AcIfTable_entry *
dot11AcIfTable_createEntry(
					long  ifIndex,
					u_int32_t value,
					char *ifDescr,
					long ifType,
					long ifMTU,
					u_long ifSpeed,
					unsigned char ifPhysAddress[],
					long ifAdminStatus,
					long ifOperStatus,
					u_long ifLastChange) 
{
    struct dot11AcIfTable_entry *entry;

    entry = SNMP_MALLOC_TYPEDEF(struct dot11AcIfTable_entry);
    if (!entry)
        return NULL;

	entry->ifIndex = ifIndex;
	entry->value = value;
	entry->ifDescr = strdup(ifDescr);
	entry->ifType = ifType;
	entry->ifMTU = ifMTU;
	entry->ifSpeed = ifSpeed;
	memcpy(entry->ifPhysAddress,ifPhysAddress,6);
	entry->ifAdminStatus = ifAdminStatus;
	entry->ifOperStatus = ifOperStatus;
	entry->ifLastChange = ifLastChange;
    entry->next = dot11AcIfTable_head;
    dot11AcIfTable_head = entry;
    return entry;
}

/* remove a row from the table */
void
dot11AcIfTable_removeEntry( struct dot11AcIfTable_entry *entry ) {
    struct dot11AcIfTable_entry *ptr, *prev;

    if (!entry)
        return;    /* Nothing to remove */

    for ( ptr  = dot11AcIfTable_head, prev = NULL;
          ptr != NULL;
          prev = ptr, ptr = ptr->next ) {
        if ( ptr == entry )
            break;
    }
    if ( !ptr )
        return;    /* Can't find it */

    if ( prev == NULL )
        dot11AcIfTable_head = ptr->next;
    else
        prev->next = ptr->next;
	free(entry->ifDescr);
	//free(entry->ifPhysAddress);
    SNMP_FREE( entry );   /* XXX - release any other internal resources */
}

void dot11AcIfTable_head_load()
{
		snmp_log(LOG_DEBUG, "enter dot11AcIfTable_head_load\n");

		struct dot11AcIfTable_entry *temp;

		while( dot11AcIfTable_head )
		{
			temp=dot11AcIfTable_head->next;
			dot11AcIfTable_removeEntry(dot11AcIfTable_head);
			dot11AcIfTable_head=temp;
		}
		{
			FILE *get_mac = NULL;
			char teammac[6][3] = { 0 };
			char mac[30] = {0};
			memset(mac,0,30);
			int speed = 0;
			unsigned char ac_mac[6] = { 0 };
			int i = 0;
			get_mac = fopen("/devinfo/mac","r");
			if(get_mac != NULL)
			{
				memset(mac,0,30);
				fgets(mac,30,get_mac);
				sscanf(mac,"%2s%2s%2s%2s%2s%2s",teammac[0],teammac[1],teammac[2],teammac[3],teammac[4],teammac[5]);
				for(i=0;i<6;i++)
				{
						ac_mac[i] = (unsigned char) strtoul(teammac[i], NULL, 16);
				}
			}
			int if_index = 1;
			int number = 0;
			unsigned int value = 0;
			char port_desc[10] = {0};
			memset(port_desc,0,10);
	  		ETH_SLOT_LIST  head,*p;
			memset(&head,0,sizeof(ETH_SLOT_LIST));
	  		ETH_PORT_LIST *pp;
			int ret = 1;

			snmp_log(LOG_DEBUG, "enter show_ethport_list\n");
		  	ret =show_ethport_list(&head,&number);
			snmp_log(LOG_DEBUG, "exit show_ethport_list,ret=%d\n", ret);
			
			p = head.next;
			while(p!=NULL)
			{
				pp = p->port.next;
				while(pp!=NULL)
				{
					memset(port_desc,0,10);
					if(1 ==number )
					{
						snprintf(port_desc,sizeof(port_desc)-1,"1-%d",pp->port_no);
					}
					else
					{
						snprintf(port_desc,sizeof(port_desc)-1,"%d-%d",p->slot_no,pp->port_no);
					}
					value = p->slot_no;				  
					value =  (value << 8) |pp->port_no;
					if(ETH_ATTR_LINKUP == ((pp->attr_map & ETH_ATTR_LINK_STATUS)>>ETH_LINK_STATUS_BIT)) 
					{
					    speed = atoi(p_eth_speed_str[(pp->attr_map & ETH_ATTR_SPEED_MASK) >> ETH_SPEED_BIT]);
						dot11AcIfTable_createEntry(if_index,
												   value,
												   port_desc,
												   pp->porttype,
												   pp->mtu,
												   speed*1000,
												   ac_mac,
												   0,
												   0,
												   pp->link_keep_time*100);
					}
					else
					{
						dot11AcIfTable_createEntry(if_index,
												   value,
												   port_desc,
												   pp->porttype,
												   pp->mtu,
												   0,
												   ac_mac,
												   0,
												   0,
												   pp->link_keep_time*100);
					}
					if_index++;
					pp = pp->next;
				}
				p=p->next;
			}
			if((ret==0)&&(number>0))
			{
				Free_ethslot_head(&head);
			}
			
			if(get_mac != NULL)
				fclose(get_mac);
		}

	snmp_log(LOG_DEBUG, "exit dot11AcIfTable_head_load\n");

}

/* Example iterator hook routines - using 'get_next' to do most of the work */
netsnmp_variable_list *
dot11AcIfTable_get_first_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{
	if(dot11AcIfTable_head==NULL)
		return NULL;
	*my_data_context = dot11AcIfTable_head;
	*my_loop_context = dot11AcIfTable_head;
	return dot11AcIfTable_get_next_data_point(my_loop_context, my_data_context,put_index_data,  mydata );
}

netsnmp_variable_list *
dot11AcIfTable_get_next_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{
    struct dot11AcIfTable_entry *entry = (struct dot11AcIfTable_entry *)*my_loop_context;
    netsnmp_variable_list *idx = put_index_data;

    if ( entry ) {
        snmp_set_var_value( idx, (u_char*)&entry->ifIndex, sizeof(long));
        idx = idx->next_variable;
        *my_data_context = (void *)entry;
        *my_loop_context = (void *)entry->next;
    } else {
        return NULL;
    }
	return put_index_data;
}


/** handles requests for the dot11AcIfTable table */
int
dot11AcIfTable_handler(
    netsnmp_mib_handler               *handler,
    netsnmp_handler_registration      *reginfo,
    netsnmp_agent_request_info        *reqinfo,
    netsnmp_request_info              *requests) {

    netsnmp_request_info       *request;
    netsnmp_table_request_info *table_info;
    struct dot11AcIfTable_entry          *table_entry;

    switch (reqinfo->mode) {
        /*
         * Read-support (also covers GetNext requests)
         */
    case MODE_GET:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11AcIfTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    
	   		if( !table_entry ){
        		netsnmp_set_request_error(reqinfo,request,SNMP_NOSUCHINSTANCE);
				continue;
	    	}  
            switch (table_info->colnum) {
            case COLUMN_IFINDEX:
			{
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->ifIndex,
                                          sizeof(long));
            }
                break;
            case COLUMN_IFDESCR:
			{
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          (u_char*)table_entry->ifDescr,
                                          strlen(table_entry->ifDescr));
            }
                break;
            case COLUMN_IFTYPE:
			{
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->ifType,
                                          sizeof(table_entry->ifType));
            }
                break;
            case COLUMN_IFMTU:
			{
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&table_entry->ifMTU,
                                          sizeof(long));
            }
                break;
            case COLUMN_IFSPEED:
			{
                snmp_set_var_typed_value( request->requestvb, ASN_GAUGE,
                                          (u_char*)&table_entry->ifSpeed,
                                          sizeof(table_entry->ifSpeed));
            }
                break;
            case COLUMN_IFPHYSADDRESS:
			{
                snmp_set_var_typed_value( request->requestvb,ASN_OCTET_STR,
                                          (u_char*)table_entry->ifPhysAddress,
                                          6);
            }
                break;
            case COLUMN_IFADMINSTATUS:
			{
				struct eth_port_s pr;
				pr.attr_bitmap=0;
				int ret = CCGI_FAIL;
				int adstate=0;
				
				ret=show_eth_port_atrr(table_entry->value,0,&pr);
				if(ret==CCGI_SUCCESS)
				{
					adstate=(pr.attr_bitmap & ETH_ATTR_ADMIN_STATUS) >> ETH_ADMIN_STATUS_BIT;
				}
				
                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&adstate,
                                          sizeof(adstate));
            }
                break;
            case COLUMN_IFOPERSTATUS:
			{
				struct eth_port_s pr;
				pr.attr_bitmap=0;
				int ret = CCGI_FAIL;
				int opstate=0;
				
				ret=show_eth_port_atrr(table_entry->value,0,&pr);
				if(ret==CCGI_SUCCESS)
				{
					opstate=(pr.attr_bitmap & ETH_ATTR_LINK_STATUS) >> ETH_LINK_STATUS_BIT;
				}

                snmp_set_var_typed_value( request->requestvb, ASN_INTEGER,
                                          (u_char*)&opstate,
                                          sizeof(opstate));
            }
                break;
            case COLUMN_IFLASTCHANGE:
			{
                snmp_set_var_typed_value( request->requestvb, ASN_TIMETICKS,
                                          (u_char*)&table_entry->ifLastChange,
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
            table_entry = (struct dot11AcIfTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    
            switch (table_info->colnum) {
            case COLUMN_IFADMINSTATUS:
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
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11AcIfTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
			
			if( !table_entry ){
        		netsnmp_set_request_error(reqinfo,request,SNMP_NOSUCHINSTANCE);
				continue;
	    	}

            switch (table_info->colnum) {
            case COLUMN_IFADMINSTATUS:
			{
				int ret = WS_FAIL;
				char admin_state[10] = {0};
				memset(admin_state,0,10);

				if(*request->requestvb->val.integer==0)
				{
					memset(admin_state,0,10);
					strncpy(admin_state,"OFF",sizeof(admin_state)-1);
				}
				else
				{
					memset(admin_state,0,10);
					strncpy(admin_state,"ON",sizeof(admin_state)-1);
				}
				
				ret=ccgi_port_admin_state(table_entry->ifDescr, admin_state);
				if(ret == WS_SUCCESS)
				{
					table_entry->ifAdminStatus = *request->requestvb->val.integer;
				}
				else
				{
					netsnmp_set_request_error(reqinfo,request,SNMP_ERR_WRONGTYPE);
				}

            }
                break;
            }
        }
        break;

    case MODE_SET_UNDO:
        for (request=requests; request; request=request->next) {
            table_entry = (struct dot11AcIfTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    
            switch (table_info->colnum) {
            case COLUMN_IFADMINSTATUS:
                /* Need to restore old 'table_entry->ifAdminStatus' value.
                   May need to use 'memcpy' */
                //table_entry->ifAdminStatus = table_entry->old_ifAdminStatus;
                break;
            }
        }
        break;

    case MODE_SET_COMMIT:
        break;
    }
    return SNMP_ERR_NOERROR;
}
