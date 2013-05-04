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
* dot11ConfigurationFile.c
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
#include "dot11ConfigurationFile.h"
#include "autelanWtpGroup.h"


/** Initializes the dot11ConfigurationFile module */
#define     CFSERVERADDR 			"2.15.3.1"
#define		CFSERVERUSERNAME   		"2.15.3.2"
#define		CFSERVERPASSWD			"2.15.3.3"
#define		CFTRANSFERTYPE			"2.15.3.4"

#define		CONFIGURATIONFILE_XMLPATH 	"/var/run/configurationfile.xml"
#define 	CFSERADDR_LEN				256
#define 	CFSERUSRNAME_LEN			50
#define		CFSERPASSWD_LEN				50


void
init_dot11ConfigurationFile(void)
{
    static oid CFServerAddr_oid[128] = {0};
    static oid CFServerUsername_oid[128] = {0};
    static oid CFServerPasswd_oid[128] = {0};
    static oid CFTransferType_oid[128] = {0};
	size_t public_oid_len   = 0;
	mad_dev_oid(CFServerAddr_oid,CFSERVERADDR,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(CFServerUsername_oid,CFSERVERUSERNAME,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(CFServerPasswd_oid,CFSERVERPASSWD,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(CFTransferType_oid,CFTRANSFERTYPE,&public_oid_len,enterprise_pvivate_oid);

  DEBUGMSGTL(("dot11ConfigurationFile", "Initializing\n"));

    netsnmp_register_scalar(
        netsnmp_create_handler_registration("CFServerAddr", handle_CFServerAddr,
                               CFServerAddr_oid, public_oid_len,
                               HANDLER_CAN_RWRITE
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("CFServerUsername", handle_CFServerUsername,
                               CFServerUsername_oid, public_oid_len,
                               HANDLER_CAN_RWRITE
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("CFServerPasswd", handle_CFServerPasswd,
                               CFServerPasswd_oid, public_oid_len,
                               HANDLER_CAN_RWRITE
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("CFTransferType", handle_CFTransferType,
                               CFTransferType_oid, public_oid_len,
                               HANDLER_CAN_RWRITE
        ));

	if(access(CONFIGURATIONFILE_XMLPATH,0)!=0)/*文件不存在*/
	{
		create_dhcp_xml(CONFIGURATIONFILE_XMLPATH);
	}
}

int
handle_CFServerAddr(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    snmp_log(LOG_DEBUG, "enter handle_CFServerAddr\n");

    switch(reqinfo->mode) {

        case MODE_GET:
		{
			char CFServerAddr[CFSERADDR_LEN];
			memset(CFServerAddr,0,sizeof(CFServerAddr));
			
			snmp_log(LOG_DEBUG, "enter get_dhcp_node_attr\n");
			get_dhcp_node_attr(CONFIGURATIONFILE_XMLPATH,"ConfigurationFile","CF","1","CFServerAddr",CFServerAddr);
			snmp_log(LOG_DEBUG, "exit get_dhcp_node_attr, CFServerAddr=%s\n",CFServerAddr);
			
            snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
                                     (u_char *)CFServerAddr,
                                     strlen(CFServerAddr));

        }
             break;

        /*
         * SET REQUEST
         *
         * multiple states in the transaction.  See:
         * http://www.net-snmp.org/tutorial-5/toolkit/mib_module/set-actions.jpg
         */
        case MODE_SET_RESERVE1:
			#if 0
            if (/* XXX: check incoming data in requests->requestvb->val.XXX for failures, like an incorrect type or an illegal value or ... */) {
                netsnmp_set_request_error(reqinfo, requests, /* XXX: set error code depending on problem (like SNMP_ERR_WRONGTYPE or SNMP_ERR_WRONGVALUE or ... */);
            }
			#endif
            break;

        case MODE_SET_RESERVE2:
			#if 0
            /* XXX malloc "undo" storage buffer */
            if (/* XXX if malloc, or whatever, failed: */) {
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_RESOURCEUNAVAILABLE);
            }
			#endif
            break;

        case MODE_SET_FREE:
            /* XXX: free resources allocated in RESERVE1 and/or
               RESERVE2.  Something failed somewhere, and the states
               below won't be called. */
            break;

        case MODE_SET_ACTION:
		{
			char * input_string = (char *)malloc(requests->requestvb->val_len+1);
			if(NULL == input_string)
			    break;
			    
			memset(input_string,0,requests->requestvb->val_len+1);
			strncpy(input_string,requests->requestvb->val.string,requests->requestvb->val_len);
			
			add_dhcp_node_attr(CONFIGURATIONFILE_XMLPATH,"ConfigurationFile","CF","1");
			mod_dhcp_node(CONFIGURATIONFILE_XMLPATH,"ConfigurationFile","CF","1","CFServerAddr",input_string);

			FREE_OBJECT(input_string);
        }
             break;

        case MODE_SET_COMMIT:
			#if 0
            /* XXX: delete temporary storage */
            if (/* XXX: error? */) {
                /* try _really_really_ hard to never get to this point */
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_COMMITFAILED);
            }
			#endif
            break;

        case MODE_SET_UNDO:
			#if 0
            /* XXX: UNDO and return to previous value for the object */
            if (/* XXX: error? */) {
                /* try _really_really_ hard to never get to this point */
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_UNDOFAILED);
            }
			#endif
            break;

        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_CFServerAddr\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_CFServerAddr\n");
    return SNMP_ERR_NOERROR;
}
int
handle_CFServerUsername(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    snmp_log(LOG_DEBUG, "enter handle_CFServerUsername\n");

    switch(reqinfo->mode) {

        case MODE_GET:
		{
			char CFServerUsername[CFSERUSRNAME_LEN];
			memset(CFServerUsername,0,sizeof(CFServerUsername));
			
			snmp_log(LOG_DEBUG, "enter get_dhcp_node_attr\n");
			get_dhcp_node_attr(CONFIGURATIONFILE_XMLPATH,"ConfigurationFile","CF","1","CFServerUsername",CFServerUsername);
			snmp_log(LOG_DEBUG, "exit get_dhcp_node_attr,CFServerUsername=%s\n",CFServerUsername);
			
            snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
                                     (u_char *)CFServerUsername,
                                     strlen(CFServerUsername));

        }
             break;

        /*
         * SET REQUEST
         *
         * multiple states in the transaction.  See:
         * http://www.net-snmp.org/tutorial-5/toolkit/mib_module/set-actions.jpg
         */
        case MODE_SET_RESERVE1:
			#if 0
            if (/* XXX: check incoming data in requests->requestvb->val.XXX for failures, like an incorrect type or an illegal value or ... */) {
                netsnmp_set_request_error(reqinfo, requests, /* XXX: set error code depending on problem (like SNMP_ERR_WRONGTYPE or SNMP_ERR_WRONGVALUE or ... */);
            }
			#endif
            break;

        case MODE_SET_RESERVE2:
			#if 0
            /* XXX malloc "undo" storage buffer */
            if (/* XXX if malloc, or whatever, failed: */) {
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_RESOURCEUNAVAILABLE);
            }
			#endif
            break;

        case MODE_SET_FREE:
            /* XXX: free resources allocated in RESERVE1 and/or
               RESERVE2.  Something failed somewhere, and the states
               below won't be called. */
            break;

        case MODE_SET_ACTION:
		{
			char * input_string = (char *)malloc(requests->requestvb->val_len+1);			
			if(NULL == input_string)
			    break;
			    
			memset(input_string,0,requests->requestvb->val_len+1);
			strncpy(input_string,requests->requestvb->val.string,requests->requestvb->val_len);
			
			add_dhcp_node_attr(CONFIGURATIONFILE_XMLPATH,"ConfigurationFile","CF","1");
			mod_dhcp_node(CONFIGURATIONFILE_XMLPATH,"ConfigurationFile","CF","1","CFServerUsername",input_string);

			FREE_OBJECT(input_string);
        }
             break;

        case MODE_SET_COMMIT:
			#if 0
            /* XXX: delete temporary storage */
            if (/* XXX: error? */) {
                /* try _really_really_ hard to never get to this point */
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_COMMITFAILED);
            }
			#endif
            break;

        case MODE_SET_UNDO:
			#if 0
            /* XXX: UNDO and return to previous value for the object */
            if (/* XXX: error? */) {
                /* try _really_really_ hard to never get to this point */
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_UNDOFAILED);
            }
			#endif
            break;

        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_CFServerUsername\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_CFServerUsername\n");
    return SNMP_ERR_NOERROR;
}
int
handle_CFServerPasswd(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    snmp_log(LOG_DEBUG, "enter handle_CFServerPasswd\n");

    switch(reqinfo->mode) {

        case MODE_GET:
		{
			char CFServerPasswd[CFSERPASSWD_LEN];
			memset(CFServerPasswd,0,sizeof(CFServerPasswd));

			snmp_log(LOG_DEBUG, "enter get_dhcp_node_attr\n");
			get_dhcp_node_attr(CONFIGURATIONFILE_XMLPATH,"ConfigurationFile","CF","1","CFServerPasswd",CFServerPasswd);
			snmp_log(LOG_DEBUG, "exit get_dhcp_node_attr,CFServerPasswd = %s\n",CFServerPasswd);
			
            snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
                                     (u_char *) CFServerPasswd,
                                     strlen(CFServerPasswd));

		}
             break;

        /*
         * SET REQUEST
         *
         * multiple states in the transaction.  See:
         * http://www.net-snmp.org/tutorial-5/toolkit/mib_module/set-actions.jpg
         */
        case MODE_SET_RESERVE1:
			#if 0
            if (/* XXX: check incoming data in requests->requestvb->val.XXX for failures, like an incorrect type or an illegal value or ... */) {
                netsnmp_set_request_error(reqinfo, requests, /* XXX: set error code depending on problem (like SNMP_ERR_WRONGTYPE or SNMP_ERR_WRONGVALUE or ... */);
            }
			#endif
            break;

        case MODE_SET_RESERVE2:
			#if 0
            /* XXX malloc "undo" storage buffer */
            if (/* XXX if malloc, or whatever, failed: */) {
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_RESOURCEUNAVAILABLE);
            }
			#endif
            break;

        case MODE_SET_FREE:
            /* XXX: free resources allocated in RESERVE1 and/or
               RESERVE2.  Something failed somewhere, and the states
               below won't be called. */
            break;

        case MODE_SET_ACTION:
		{
			char * input_string = (char *)malloc(requests->requestvb->val_len+1);			
			if(NULL == input_string)
			    break;

			memset(input_string,0,requests->requestvb->val_len+1);
			strncpy(input_string,requests->requestvb->val.string,requests->requestvb->val_len);
			
			add_dhcp_node_attr(CONFIGURATIONFILE_XMLPATH,"ConfigurationFile","CF","1");
			mod_dhcp_node(CONFIGURATIONFILE_XMLPATH,"ConfigurationFile","CF","1","CFServerPasswd",input_string);

			FREE_OBJECT(input_string);
        }
             break;

        case MODE_SET_COMMIT:
			#if 0
            /* XXX: delete temporary storage */
            if (/* XXX: error? */) {
                /* try _really_really_ hard to never get to this point */
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_COMMITFAILED);
            }
			#endif
            break;

        case MODE_SET_UNDO:
			#if 0
            /* XXX: UNDO and return to previous value for the object */
            if (/* XXX: error? */) {
                /* try _really_really_ hard to never get to this point */
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_UNDOFAILED);
            }
			#endif
            break;

        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_CFServerPasswd\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_CFServerPasswd\n");
    return SNMP_ERR_NOERROR;
}
int
handle_CFTransferType(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    snmp_log(LOG_DEBUG, "enter handle_CFTransferType\n");

    switch(reqinfo->mode) {

        case MODE_GET:
		{
			int VFTransferType = 0;
			
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
                                     (u_char *)&VFTransferType,
                                     sizeof(VFTransferType));
        }
             break;

        /*
         * SET REQUEST
         *
         * multiple states in the transaction.  See:
         * http://www.net-snmp.org/tutorial-5/toolkit/mib_module/set-actions.jpg
         */
        case MODE_SET_RESERVE1:
			#if 0
            if (/* XXX: check incoming data in requests->requestvb->val.XXX for failures, like an incorrect type or an illegal value or ... */) {
                netsnmp_set_request_error(reqinfo, requests, /* XXX: set error code depending on problem (like SNMP_ERR_WRONGTYPE or SNMP_ERR_WRONGVALUE or ... */);
            }
			#endif
            break;

        case MODE_SET_RESERVE2:
			#if 0
            /* XXX malloc "undo" storage buffer */
            if (/* XXX if malloc, or whatever, failed: */) {
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_RESOURCEUNAVAILABLE);
            }
			#endif
            break;

        case MODE_SET_FREE:
            /* XXX: free resources allocated in RESERVE1 and/or
               RESERVE2.  Something failed somewhere, and the states
               below won't be called. */
            break;

        case MODE_SET_ACTION:
		{
			int status;
			char command[256];
			memset(command,0,sizeof(command));
			char url[CFSERADDR_LEN];
			memset(url,0,sizeof(url));
			char username[CFSERUSRNAME_LEN];
			memset(username,0,sizeof(username));
			char passwd[CFSERPASSWD_LEN];
			memset(passwd,0,sizeof(passwd));
			char server[256];
			memset(server,0,sizeof(server));
		    char *dfile = NULL;

			snmp_log(LOG_DEBUG, "enter get_dhcp_node_attr\n");
			get_dhcp_node_attr(CONFIGURATIONFILE_XMLPATH,"ConfigurationFile","CF","1","CFServerAddr",url);
			get_dhcp_node_attr(CONFIGURATIONFILE_XMLPATH,"ConfigurationFile","CF","1","CFServerUsername",username);
			get_dhcp_node_attr(CONFIGURATIONFILE_XMLPATH,"ConfigurationFile","CF","1","CFServerPasswd",passwd);
			snmp_log(LOG_DEBUG, "exit get_dhcp_node_attr,url=%s,username=%s,passwd=%s\n", url,username,passwd);
			
			if(*requests->requestvb->val.integer==1)	/*download */
			{				
				memset(command,0,sizeof(command));
				snprintf(command, sizeof(command) - 1,"sudo download_conf.sh %s %s %s >/dev/null",url,username,passwd);		
				status=system(command);		
			
		        if(status!=0)
		        {
		            netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
		        }
			}
			else if(*requests->requestvb->val.integer==2)	/*upload*/
			{				
				dfile=strchr(url,'/');				
				strncpy(server,url,dfile-url);				
			
				memset(command,0,sizeof(command));
				snprintf(command, sizeof(command) - 1,"sudo upload_conf.sh  %s %s %s %s>/dev/null",server,username,passwd,dfile+1);			
				status=system(command);		
			
		        if(status!=0)
		        {
		            netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
		        }
			}
			else
			{
				netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE); 
			}	
			
        }
             break;

        case MODE_SET_COMMIT:
			#if 0
            /* XXX: delete temporary storage */
            if (/* XXX: error? */) {
                /* try _really_really_ hard to never get to this point */
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_COMMITFAILED);
            }
			#endif
            break;

        case MODE_SET_UNDO:
			#if 0
            /* XXX: UNDO and return to previous value for the object */
            if (/* XXX: error? */) {
                /* try _really_really_ hard to never get to this point */
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_UNDOFAILED);
            }
			#endif
            break;

        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_CFTransferType\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_CFTransferType\n");
    return SNMP_ERR_NOERROR;
}
