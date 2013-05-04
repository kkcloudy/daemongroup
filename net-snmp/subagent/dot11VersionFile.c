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
* dot11VersionFile.c
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
#include "dot11VersionFile.h"
#include "autelanWtpGroup.h"


/** Initializes the dot11VersionFile module */

#define     VFSERVERADDR 			"2.15.2.1"
#define		VFSERVERUSERNAME   		"2.15.2.2"
#define		VFSERVERPASSWD			"2.15.2.3"
#define		VFTRANSFERTYPE			"2.15.2.4"

#define		VERSIONFILE_XMLPATH 	"/var/run/versionfile.xml"
#define 	SERADDR_LEN				256
#define 	SERUSRNAME_LEN			50
#define		SERPASSWD_LEN			50

static char *get_img_name_from_path( char *path )
{
	char *temp = NULL;
	static char img_name[64];

	memset( img_name, 0, sizeof(img_name) );
	temp = strstr( path, "AW");
	if( NULL != temp && strlen(temp) < sizeof(img_name) )
	{
		strcpy( img_name, temp );
	}

	return img_name;
}

void
init_dot11VersionFile(void)
{
    static oid VFServerAddr_oid[128] = {0};
    static oid VFServerUsername_oid[128] = {0};
    static oid VFServerPasswd_oid[128] = {0};
    static oid VFTransferType_oid[128] = {0};
	
	size_t public_oid_len   = 0;
	mad_dev_oid(VFServerAddr_oid,VFSERVERADDR,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(VFServerUsername_oid,VFSERVERUSERNAME,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(VFServerPasswd_oid,VFSERVERPASSWD,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(VFTransferType_oid,VFTRANSFERTYPE,&public_oid_len,enterprise_pvivate_oid);

  DEBUGMSGTL(("dot11VersionFile", "Initializing\n"));

    netsnmp_register_scalar(
        netsnmp_create_handler_registration("VFServerAddr", handle_VFServerAddr,
                               VFServerAddr_oid, public_oid_len,
                               HANDLER_CAN_RWRITE
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("VFServerUsername", handle_VFServerUsername,
                               VFServerUsername_oid, public_oid_len,
                               HANDLER_CAN_RWRITE
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("VFServerPasswd", handle_VFServerPasswd,
                               VFServerPasswd_oid, public_oid_len,
                               HANDLER_CAN_RWRITE
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("VFTransferType", handle_VFTransferType,
                               VFTransferType_oid, public_oid_len,
                               HANDLER_CAN_RWRITE
        ));

	if(access(VERSIONFILE_XMLPATH,0)!=0)/*文件不存在*/
	{
		create_dhcp_xml(VERSIONFILE_XMLPATH);
	}
}

int
handle_VFServerAddr(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    snmp_log(LOG_DEBUG, "enter handle_VFServerAddr\n");

    switch(reqinfo->mode) {

        case MODE_GET:
		{
			char VFServerAddr[SERADDR_LEN];
			memset(VFServerAddr,0,sizeof(VFServerAddr));
			
			snmp_log(LOG_DEBUG, "enter get_dhcp_node_attr\n");
			get_dhcp_node_attr(VERSIONFILE_XMLPATH,"VersionFile","VF","1","VFServerAddr",VFServerAddr);
			snmp_log(LOG_DEBUG, "exit get_dhcp_node_attr\n");
			
            snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
                                     (u_char *)VFServerAddr,
                                     strlen(VFServerAddr));

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
			
			add_dhcp_node_attr(VERSIONFILE_XMLPATH,"VersionFile","VF","1");
			mod_dhcp_node(VERSIONFILE_XMLPATH,"VersionFile","VF","1","VFServerAddr",input_string);

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
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_VFServerAddr\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_VFServerAddr\n");
    return SNMP_ERR_NOERROR;
}
int
handle_VFServerUsername(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    
	snmp_log(LOG_DEBUG, "enter handle_VFServerUsername\n");
	
    switch(reqinfo->mode) {

        case MODE_GET:
		{
			char VFServerUsername[SERUSRNAME_LEN];
			memset(VFServerUsername, 0, sizeof(VFServerUsername));

			get_dhcp_node_attr(VERSIONFILE_XMLPATH,"VersionFile","VF","1","VFServerUsername",VFServerUsername);
			
            snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
                                     (u_char *)VFServerUsername,
                                     strlen(VFServerUsername));

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

			add_dhcp_node_attr(VERSIONFILE_XMLPATH,"VersionFile","VF","1");
			mod_dhcp_node(VERSIONFILE_XMLPATH,"VersionFile","VF","1","VFServerUsername",input_string);

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
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_VFServerUsername\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_VFServerUsername\n");
    return SNMP_ERR_NOERROR;
}
int
handle_VFServerPasswd(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    snmp_log(LOG_DEBUG, "enter handle_VFServerPasswd\n");

    switch(reqinfo->mode) {

        case MODE_GET:
		{
			char VFServerPasswd[SERPASSWD_LEN];
			memset(VFServerPasswd, 0, sizeof(VFServerPasswd));

			snmp_log(LOG_DEBUG, "enter get_dhcp_node_attr\n");
			get_dhcp_node_attr(VERSIONFILE_XMLPATH,"VersionFile","VF","1","VFServerPasswd",VFServerPasswd);
			snmp_log(LOG_DEBUG, "exit get_dhcp_node_attr\n");
			
            snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
                                     (u_char *) VFServerPasswd,
                                     strlen(VFServerPasswd));

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
			
			add_dhcp_node_attr(VERSIONFILE_XMLPATH,"VersionFile","VF","1");
			mod_dhcp_node(VERSIONFILE_XMLPATH,"VersionFile","VF","1","VFServerPasswd",input_string);

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
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_VFServerPasswd\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_VFServerPasswd\n");
    return SNMP_ERR_NOERROR;
}
int
handle_VFTransferType(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    
	snmp_log(LOG_DEBUG, "enter handle_VFTransferType\n");
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
			memset(command, 0, sizeof(command));
			char url[SERADDR_LEN];
			memset(url,0,sizeof(url));
			char username[SERUSRNAME_LEN];
			memset(username,0,sizeof(username));
			char passwd[SERPASSWD_LEN];
			memset(passwd,0,sizeof(passwd));
			char server[256];
			memset(server,0,sizeof(server));
		    char *dfile,*file;

			get_dhcp_node_attr(VERSIONFILE_XMLPATH,"VersionFile","VF","1","VFServerAddr",url);
			get_dhcp_node_attr(VERSIONFILE_XMLPATH,"VersionFile","VF","1","VFServerUsername",username);
			get_dhcp_node_attr(VERSIONFILE_XMLPATH,"VersionFile","VF","1","VFServerPasswd",passwd);

			if(*requests->requestvb->val.integer==1)	/*download */
			{			
				char *img_name = get_img_name_from_path( url );
				snmp_log( LOG_DEBUG, "dot11VersionFile img_name = %s\n", img_name );
				if( strlen(img_name) > 0 )
				{
					memset(command,0,sizeof(command));
					snprintf(command, sizeof(command) - 1,"sudo update_ac_version.sh %s %s %s %s>/dev/null",url,username,passwd,img_name);		
					snmp_log( LOG_DEBUG, "dot11VersionFile command = %s\n", command );
					status=system(command);		
				
			        if(status!=0)
			        {
			            netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
			        }
				}
			}
			else if(*requests->requestvb->val.integer==2)	/*upload*/
			{				
				dfile=strchr(url,'/');				
				strncpy(server,url,dfile-url);				
				file=strrchr(url,'/');
			
				memset(command,0,sizeof(command));
				snprintf(command, sizeof(command) - 1, "sudo ftpupload.sh  %s %s %s /blk/%s %s>/dev/null",server,username,passwd,file+1,dfile+1);			
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
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_VFTransferType\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_VFTransferType\n");
    return SNMP_ERR_NOERROR;
}
