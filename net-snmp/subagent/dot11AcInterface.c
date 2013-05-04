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
* dot11AcInterface.c
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
#include "dot11AcInterface.h"
#include "wcpss/asd/asd.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/dcli_wid_wtp.h"
#include "dbus/wcpss/dcli_wid_wlan.h"
#include "ws_dcli_wlans.h"
#include "ws_sysinfo.h"
#include "ws_init_dbus.h"
#include "ws_dcli_portconf.h"
#include "mibs_public.h"
#include "ws_sta.h"

#include "autelanWtpGroup.h"
#include "ws_intf.h"

#define ACIFPORTNUM	"2.4.1"
#define ACPHYPORTNUM	"2.4.5"

/** Initializes the dot11AcInterface module */
void
init_dot11AcInterface(void)
{
	static oid acIfPortNum_oid[128] = { 0 };
	static oid acPhyPortNum_oid[128] = { 0 };
	
	size_t public_oid_len = 0;
	mad_dev_oid(acIfPortNum_oid,ACIFPORTNUM,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acPhyPortNum_oid,ACPHYPORTNUM,&public_oid_len,enterprise_pvivate_oid);
	
	DEBUGMSGTL(("dot11AcInterface", "Initializing\n"));
	
	netsnmp_register_scalar(
		netsnmp_create_handler_registration("acIfPortNum", handle_acIfPortNum,
							acIfPortNum_oid, public_oid_len,
							HANDLER_CAN_RONLY
		));
	netsnmp_register_scalar(
		netsnmp_create_handler_registration("acPhyPortNum", handle_acPhyPortNum,
							acPhyPortNum_oid, public_oid_len,
							HANDLER_CAN_RONLY
		));
}

int
handle_acIfPortNum(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
	snmp_log(LOG_DEBUG, "enter handle_acIfPortNum\n");

	switch(reqinfo->mode) {

	case MODE_GET:
	{
		#if 0
		int number = 0,ret1=0,port_count = 0;
		ETH_SLOT_LIST  head,*p;
		memset(&head,0,sizeof(ETH_SLOT_LIST));
		ETH_PORT_LIST *pp;
		ret1=show_ethport_list(&head,&number);
		p=head.next;
		if(p!=NULL)
		{
			while(p!=NULL)
			{
				port_count +=p->port_num;
				pp=p->port.next;
				p=p->next;
			}
		#endif
		unsigned int ret = 0, num = 0;
		struct flow_data_list *alldata = NULL;
		struct flow_data_list *temp = NULL;

		
		snmp_log(LOG_DEBUG, "enter intflib_getdata\n");
		ret = intflib_getdata(&alldata);
		snmp_log(LOG_DEBUG, "exit intflib_getdata,ret=%d\n", ret);
		
		if(ret < 0)
		{
			printf("intflib get data error! ret %d.<br>",ret);
		}
		temp = alldata;
		for(temp; NULL != temp; temp = temp->next)
		{
			
			if(strncmp(temp->ltdata.ifname,"r",1)==0 
			|| strncmp(temp->ltdata.ifname,"pimreg",6)==0 
			|| strncmp(temp->ltdata.ifname,"sit0",4)==0)
			{
				continue;
			}
	
			num++;
		}
		snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
								(u_char *)&num,
								sizeof(num));
		intflib_memfree(&alldata);
		#if 0
		if((ret1==0)&&(number>0))
		{
			Free_ethslot_head(&head);
		}
		#endif
	}
	break;


	default:
	/* we should never get here, so this is a really bad error */
	snmp_log(LOG_ERR, "unknown mode (%d) in handle_acIfPortNum\n", reqinfo->mode );

	return SNMP_ERR_GENERR;
	}

	snmp_log(LOG_DEBUG, "exit handle_acIfPortNum\n");
	return SNMP_ERR_NOERROR;
}

int
handle_acPhyPortNum(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
	snmp_log(LOG_DEBUG, "enter handle_acPhyPortNum\n");

	switch(reqinfo->mode) {

	case MODE_GET:
	{
		int acPhyPortNum = 0;

		snmp_log(LOG_DEBUG, "enter count_eth_port_num\n");
		acPhyPortNum=count_eth_port_num();
		snmp_log(LOG_DEBUG, "exit count_eth_port_num,acPhyPortNum=%d\n", acPhyPortNum);
		
		snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
								(u_char *)&acPhyPortNum,
								sizeof(acPhyPortNum));
	}
	break;


	default:
	/* we should never get here, so this is a really bad error */
	snmp_log(LOG_ERR, "unknown mode (%d) in handle_acPhyPortNum\n", reqinfo->mode );

	return SNMP_ERR_GENERR;
	}

	snmp_log(LOG_DEBUG, "exit handle_acPhyPortNum\n");
	return SNMP_ERR_NOERROR;
}

