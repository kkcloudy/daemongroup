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
* dot11AcDeviceInfo.c
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
#include <sys/sysinfo.h>
#include "dot11AcDeviceInfo.h"
#include "ws_sysinfo.h"
#include "mibs_public.h"
#include "autelanWtpGroup.h"
#include "ws_log_conf.h"
#include "ws_stp.h"
#include "wcpss/asd/asd.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/dcli_wid_wtp.h"
#include "dbus/wcpss/dcli_wid_wlan.h"
#include "board/board_define.h"
#include "ws_dcli_wlans.h"
#include "ws_dcli_ac.h"
#include "ac_sample_def.h"
#include "ac_sample_err.h"



 CPUS_STU st_cpus_status;
 CPU_TEM st_cpu_temperature;
 MEM_STU st_mem_status;
 SYS_INFO st_sys_info={ &st_cpus_status, 
                              &st_cpu_temperature,  
                              &st_mem_status};
/** Initializes the dot11AcDeviceInfo module */

#define ACDEVNAME 				"2.1.2.1"
#define ACMANUFAUTURES			"2.1.2.2"
#define	ACDEVICESN				"2.1.2.3"
#define	ACDEVICETYR				"2.1.2.4"
#define	ACDEVICEHWVERSION   	"2.1.2.5"
#define	ACMEMORYTYPE			"2.1.2.6"
#define	ACMEMORYCAPACITY		"2.1.2.7"
#define	ACMEMRTUSAGE			"2.1.2.8"
#define ACMEMPEAKUSAGE			"2.1.2.9"		
#define	ACMEMAVGUSAGE			"2.1.2.10"		
#define	ACMEMUSAGETHRESHHD		"2.1.2.11"
#define	ACCPUTYPE				"2.1.2.12"
#define	ACCPURTUSAGE			"2.1.2.13"
#define ACCPUPEAKUSAGE			"2.1.2.14"
#define ACCPUAVGUSAGE			"2.1.2.15"
#define	ACCPUUSAGETHRESHHD		"2.1.2.16"
#define	ACFLASHTYPE				"2.1.2.17"
#define	ACFLASHCAPACITY			"2.1.2.18"
#define	ACFLASHSPACE 			"2.1.2.19"	
#define	ACWORKTEMPETURE		"2.1.2.20"
#define ACWTPCPUTHRESHHOLD  "2.1.2.21"
#define ACWTPMEMTHRESHHOLD  "2.1.2.22"
#define ACCPUFREQUENCY		"2.1.2.24"
		
static unsigned int acMemRTUsage = 0;
static unsigned int acMemPeakUsage = 0;
static unsigned int acMemAvgUsage = 0;
static long update_time_dbus_get_sample_memusage_info = 0;
static void update_data_for_dbus_get_sample_memusage_info();

static unsigned int acCPURTUsage = 0;
static unsigned int acCPUAvgUsage = 0;
static unsigned int acCPUPeakUsage = 0;
static long update_time_dbus_get_sample_cpuusage_info = 0;
static void update_data_for_dbus_get_sample_cpuusage_info();

void
init_dot11AcDeviceInfo(void)
{
    static oid acDeviceName_oid[128] 				= {0};
    static oid acManufacturers_oid[128] 				= {0};
    static oid acDeviceSN_oid[128] 					= {0};
    static oid acDeviceType_oid[128] 				= {0};
    static oid acDeviceHWVersion_oid[128] 			= {0};
    static oid acMemoryType_oid[128]  				= {0};
    static oid acMemoryCapacity_oid[128] 			= {0};
    static oid acMemRTUsage_oid[128] 				= {0};
    static oid acMemPeakUsage_oid[128]				= {0};
    static oid acMemAvgUsage_oid[128] 				= {0};
    static oid acMemUsageThreshhd_oid[128] 		= {0};
    static oid acCPUType_oid[128] 					= {0};
    static oid acCPURTUsage_oid[128] 				= {0};
    static oid acCPUPeakUsage_oid[128]				= {0};
    static oid acCPUAvgUsage_oid[128]				= {0};
    static oid acCPUusageThreshhd_oid[128] 			= {0};
    static oid acFlashType_oid[128] 					= {0};
    static oid acFlashCapacity_oid[128] 				= {0};
    static oid acFlashResidualSpace_oid[128] 			= {0};
    static oid acWorkTemperature_oid[128] 			= {0};
	static oid acWtpCPUusageThreshhd_oid[128]           = {0};
	static oid acWtpMemusageThreshhd_oid[128]           = {0};
	static oid acCPUFrequency_oid[128]				= {0};


	size_t public_oid_len   = 0;
	mad_dev_oid(acDeviceName_oid,ACDEVNAME,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acManufacturers_oid,ACMANUFAUTURES,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acDeviceSN_oid,ACDEVICESN,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acDeviceType_oid,ACDEVICETYR,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acDeviceHWVersion_oid,ACDEVICEHWVERSION,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acMemoryType_oid,ACMEMORYTYPE,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acMemoryCapacity_oid,ACMEMORYCAPACITY,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acMemRTUsage_oid,ACMEMRTUSAGE,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acMemPeakUsage_oid,ACMEMPEAKUSAGE,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acMemAvgUsage_oid,ACMEMAVGUSAGE,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acMemUsageThreshhd_oid,ACMEMUSAGETHRESHHD,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acCPUType_oid,ACCPUTYPE,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acCPURTUsage_oid,ACCPURTUSAGE,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acCPUPeakUsage_oid,ACCPUPEAKUSAGE,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acCPUAvgUsage_oid,ACCPUAVGUSAGE,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acCPUusageThreshhd_oid,ACCPUUSAGETHRESHHD,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acFlashType_oid,ACFLASHTYPE,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acFlashCapacity_oid,ACFLASHCAPACITY,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acFlashResidualSpace_oid,ACFLASHSPACE,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acWorkTemperature_oid,ACWORKTEMPETURE,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acWtpCPUusageThreshhd_oid,ACWTPCPUTHRESHHOLD,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acWtpMemusageThreshhd_oid,ACWTPMEMTHRESHHOLD,&public_oid_len,enterprise_pvivate_oid);
	mad_dev_oid(acCPUFrequency_oid,ACCPUFREQUENCY,&public_oid_len,enterprise_pvivate_oid);

  DEBUGMSGTL(("dot11AcDeviceInfo", "Initializing\n"));

    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acDeviceName", handle_acDeviceName,
                               acDeviceName_oid, public_oid_len,
                               HANDLER_CAN_RWRITE
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acManufacturers", handle_acManufacturers,
                               acManufacturers_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acDeviceSN", handle_acDeviceSN,
                               acDeviceSN_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acDeviceType", handle_acDeviceType,
                               acDeviceType_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acDeviceHWVersion", handle_acDeviceHWVersion,
                               acDeviceHWVersion_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acMemoryType", handle_acMemoryType,
                               acMemoryType_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acMemoryCapacity", handle_acMemoryCapacity,
                               acMemoryCapacity_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acMemRTUsage", handle_acMemRTUsage,
                               acMemRTUsage_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));

    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acMemPeakUsage", handle_acMemPeakUsage,
                               acMemPeakUsage_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acMemAvgUsage", handle_acMemAvgUsage,
                               acMemAvgUsage_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acMemUsageThreshhd", handle_acMemUsageThreshhd,
                               acMemUsageThreshhd_oid, public_oid_len,
                               HANDLER_CAN_RWRITE
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acCPUType", handle_acCPUType,
                               acCPUType_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
	
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acCPURTUsage", handle_acCPURTUsage,
                               acCPURTUsage_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acCPUPeakUsage", handle_acCPUPeakUsage,
                               acCPUPeakUsage_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acCPUAvgUsage", handle_acCPUAvgUsage,
                               acCPUAvgUsage_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acCPUusageThreshhd", handle_acCPUusageThreshhd,
                               acCPUusageThreshhd_oid, public_oid_len,
                               HANDLER_CAN_RWRITE
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acFlashType", handle_acFlashType,
                               acFlashType_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acFlashCapacity", handle_acFlashCapacity,
                               acFlashCapacity_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acFlashResidualSpace", handle_acFlashResidualSpace,
                               acFlashResidualSpace_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("acWorkTemperature", handle_acWorkTemperature,
                               acWorkTemperature_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
	netsnmp_register_scalar(
        netsnmp_create_handler_registration("acWtpCPUusageThreshhd", handle_acWtpCPUusageThreshhd,
                               acWtpCPUusageThreshhd_oid, public_oid_len,
                               HANDLER_CAN_RWRITE
        ));
	 netsnmp_register_scalar(
        netsnmp_create_handler_registration("acWtpMemusageThreshhd", handle_acWtpMemusageThreshhd,
                               acWtpMemusageThreshhd_oid, public_oid_len,
                               HANDLER_CAN_RWRITE
        ));
	 netsnmp_register_scalar(
        netsnmp_create_handler_registration("acCPUFrequency", handle_acCPUFrequency,
                               acCPUFrequency_oid, public_oid_len,
                               HANDLER_CAN_RONLY
        ));
}

int
handle_acDeviceName(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
	snmp_log(LOG_DEBUG, "enter handle_acDeviceName\n");

    switch(reqinfo->mode) {

    case MODE_GET:
	{
		FILE *hostname = NULL;
		char code[128] = {0};
		memset(code,0,128);

		snmp_log(LOG_DEBUG, "enter popen\n");
		hostname = popen("hostname","r");
		snmp_log(LOG_DEBUG, "exit popen,hostname=%p\n", hostname);
		
		if(hostname != NULL)
		{
			fgets(code,128,hostname);
			delete_enter(code);
			pclose(hostname);
		}

		snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
									(u_char *)code,
									strlen(code));
		
	}
	break;
	case MODE_SET_RESERVE1:
	//   if (/* XXX: check incoming data in requests->requestvb->val.XXX for failures, like an incorrect type or an illegal value or ... */) {
	//      netsnmp_set_request_error(reqinfo, requests, /* XXX: set error code depending on problem (like SNMP_ERR_WRONGTYPE or SNMP_ERR_WRONGVALUE or ... */);
	//  }
	break;
	case MODE_SET_RESERVE2:
	/* XXX malloc "undo" storage buffer */
	//  if (/* XXX if malloc, or whatever, failed: */) {

	//       netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_RESOURCEUNAVAILABLE);

	//  }
	break;

	case MODE_SET_FREE:
	/* XXX: free resources allocated in RESERVE1 and/or
	RESERVE2.  Something failed somewhere, and the states
	below won't be called. */
	break;

	case MODE_SET_ACTION:
	{
		int status;
		char command[256] = {0};
		char * input_string = (char *)malloc(requests->requestvb->val_len+1);
		if(input_string)
		{
			memset(input_string,0,requests->requestvb->val_len+1);
			strncpy(input_string,requests->requestvb->val.string,requests->requestvb->val_len);
		}
		
		memset(command,0,256);		
		strncpy(command,"hostname -v ",sizeof(command)-1);
		if(input_string)
		{
			strncat(command,input_string,sizeof(command)-strlen(command)-1);
		}
		status = system(command);
	
        if(status!=0)
        {
            netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
        }

		FREE_OBJECT(input_string);
	}   
	break;

	case MODE_SET_COMMIT:
	/* XXX: delete temporary storage */
	// if (/* XXX: error? */) {
	/* try _really_really_ hard to never get to this point */
	//    netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_COMMITFAILED);
	// }
	break;

	case MODE_SET_UNDO:
	/* XXX: UNDO and return to previous value for the object */
	// if (/* XXX: error? */) {
	/* try _really_really_ hard to never get to this point */
	//     netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_UNDOFAILED);
	// }
	break;


    default:
        /* we should never get here, so this is a really bad error */
        snmp_log(LOG_ERR, "unknown mode (%d) in handle_acDeviceName\n", reqinfo->mode );
        return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acDeviceName\n");
    return SNMP_ERR_NOERROR;
}
int
handle_acManufacturers(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    snmp_log(LOG_DEBUG, "enter handle_acManufacturers\n");

    switch(reqinfo->mode) {

	case MODE_GET:
	{
		FILE * get_logo = NULL;
		char logo[100] = {0};
		memset(logo,0,100);

		snmp_log(LOG_DEBUG, "enter popen\n");
		get_logo = popen("cat /devinfo/enterprise_name | sed \"2,200d\"","r");
		snmp_log(LOG_DEBUG, "exit popen,get_logo=%p\n", get_logo);
		
		if(get_logo != NULL)
		{
			fgets(logo,100,get_logo);
			delete_enter(logo);
			pclose(get_logo);
		}
		snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
								(u_char *)logo,
								strlen(logo));
	}
	break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acManufacturers\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acManufacturers\n");
    return SNMP_ERR_NOERROR;
}
int
handle_acDeviceSN(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
	/* We are never called for a GETNEXT if it's registered as a
	"instance", as it's "magically" handled for us.  */

	/* a instance handler also only hands us one request at a time, so
	we don't need to loop over a list of requests; we'll only get one. */
	snmp_log(LOG_DEBUG, "enter handle_acDeviceSN\n");

	switch(reqinfo->mode) 
	{

		case MODE_GET:
		{
			int cllection_mode = 0;/*0表示关闭分板采集，1表示开启分板采集*/  
			int sn_type = 0;/*0表示sn+slotid，1表示local_board_sn*/  
			FILE *fp = NULL;
			char acDeviceSn[256] = {0};
			char slot_id[10] = { 0 };

			if(snmp_cllection_mode(ccgi_dbus_connection))
			{
				cllection_mode = 1;
			}

			sn_type = ac_trap_get_flag("/var/run/ac_sn_type");
			if((1 == cllection_mode)&&(1 == sn_type))/*开启分板采集*/
			{
				fp = NULL;
				memset(acDeviceSn,0,256);			

				if(VALID_DBM_FLAG == get_dbm_effective_flag())
				{
					snmp_log(LOG_DEBUG, "enter fopen /dbm/local_board/sn\n");
					fp=fopen("/dbm/local_board/sn","r");
					snmp_log(LOG_DEBUG, "exit fopen /dbm/local_board/sn,fp=%p\n", fp);				
					if(fp != NULL)
					{
						fgets(acDeviceSn,256,fp);
						delete_enter(acDeviceSn);
						if(strcmp(acDeviceSn,"") == 0)
						{
							memset(acDeviceSn,0,256);			
							strncpy(acDeviceSn,"01010106C14009900001",sizeof(acDeviceSn)-1);
						}
						fclose(fp);
					}	
				}
				else
				{
					strncpy(acDeviceSn,"01010106C14009900001",sizeof(acDeviceSn)-1);
				}
				
			}
			else
			{
				fp = NULL;
				memset(acDeviceSn,0,256);			
				
				snmp_log(LOG_DEBUG, "enter fopen /devinfo/sn\n");
				fp=fopen("/devinfo/sn","r");
				snmp_log(LOG_DEBUG, "exit fopen /devinfo/sn,fp=%p\n", fp);
				
				if(fp != NULL)
				{
					fgets(acDeviceSn,256,fp);
					delete_enter(acDeviceSn);
					if(strcmp(acDeviceSn,"") == 0)
					{
						memset(acDeviceSn,0,256);			
						strncpy(acDeviceSn,"01010106C14009900001",sizeof(acDeviceSn)-1);
					}
					fclose(fp);
				}
				if((1 == cllection_mode)&&(0 == sn_type))
				{
					if(VALID_DBM_FLAG == get_dbm_effective_flag())
					{
						fp = NULL;	
						snmp_log(LOG_DEBUG, "enter fopen /dbm/local_board/slot_id\n");
						fp=fopen("/dbm/local_board/slot_id","r");
						snmp_log(LOG_DEBUG, "exit fopen /dbm/local_board/slot_id,fp=%p\n", fp);
						if(fp != NULL)
						{
							memset(slot_id,0,sizeof(slot_id));
							fgets(slot_id,10,fp);
							delete_enter(slot_id);
							fclose(fp);
							strncat(acDeviceSn, "_", sizeof(acDeviceSn)-strlen(acDeviceSn)-1);
							strncat(acDeviceSn, slot_id, sizeof(acDeviceSn)-strlen(acDeviceSn)-1);
						}
					}
				}
			}			
			
			snmp_set_var_typed_value(requests->requestvb,ASN_OCTET_STR,
										(u_char *)acDeviceSn,
										strlen(acDeviceSn));

		}
		break;


		default:
		/* we should never get here, so this is a really bad error */
		snmp_log(LOG_ERR, "unknown mode (%d) in handle_acDeviceSN\n",reqinfo->mode );
		return SNMP_ERR_GENERR;
	}

	snmp_log(LOG_DEBUG, "exit handle_acDeviceSN\n");
	return SNMP_ERR_NOERROR;
}
int
handle_acDeviceType(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    snmp_log(LOG_DEBUG, "enter handle_acDeviceType\n");

    switch(reqinfo->mode) {

        case MODE_GET:
		{
			struct sys_ver sys_version;
			memset(&sys_version,0,sizeof(struct sys_ver));
			#if 0
			sys_version.base_mac = (char *)malloc(50);
			if(sys_version.base_mac)
			{
				memset(sys_version.base_mac,0,50);
			}
			sys_version.product_name = (char *)malloc(50);
			if(sys_version.product_name)
			{
				memset(sys_version.product_name,0,50);
			}
			sys_version.serial_no =  (char *)malloc(50);
			if(sys_version.serial_no)
			{
				memset(sys_version.serial_no,0,50);
			}
			sys_version.swname = (char *)malloc(50);
			if(sys_version.swname)
			{
				memset(sys_version.swname,0,50);
			}
			memset(sys_version.sw_product_name,0,128);
			#endif
			show_sys_ver(&sys_version);
			delete_enter(sys_version.sw_product_name);
			
			snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
										(u_char *)sys_version.sw_product_name,
										strlen(sys_version.sw_product_name));
			
			FREE_OBJECT(sys_version.base_mac);
			FREE_OBJECT(sys_version.product_name);
			FREE_OBJECT(sys_version.serial_no);
			FREE_OBJECT(sys_version.swname);
		}
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acDeviceType\n",reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acDeviceType\n");
    return SNMP_ERR_NOERROR;
}
int
handle_acDeviceHWVersion(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
	/* We are never called for a GETNEXT if it's registered as a
	"instance", as it's "magically" handled for us.  */

	/* a instance handler also only hands us one request at a time, so
	we don't need to loop over a list of requests; we'll only get one. */
	snmp_log(LOG_DEBUG, "enter handle_acDeviceHWVersion\n");

	switch(reqinfo->mode) 
	{
		case MODE_GET:
		{
			char hw_ver[50];
			memset(hw_ver,0,50);
			FILE *fp =NULL;

			fp = fopen("/devinfo/hardware_version","r");
			snmp_log(LOG_DEBUG, "exit fopen,fp=%p\n", fp);
			
			if (fp !=NULL)
			{
				fgets(hw_ver,50,fp);
				delete_enter(hw_ver);
				fclose(fp);
			}

			snmp_set_var_typed_value(requests->requestvb,ASN_OCTET_STR,
									(u_char *)hw_ver,
									strlen(hw_ver));

		}
		break;


		default:
		/* we should never get here, so this is a really bad error */
		snmp_log(LOG_ERR, "unknown mode (%d) inhandle_acDeviceHWVersion\n", reqinfo->mode );
		return SNMP_ERR_GENERR;
	}

	snmp_log(LOG_DEBUG, "exit handle_acDeviceHWVersion\n");
	return SNMP_ERR_NOERROR;
}
int
handle_acMemoryType(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
		snmp_log(LOG_DEBUG, "enter handle_acMemoryType\n");

		switch(reqinfo->mode) 
		{
	
			case MODE_GET:
			{
				snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
										(u_char *)"SDRAM",
										strlen("SDRAM"));
			}
			break;
	
	
			default:
			/* we should never get here, so this is a really bad error */
			snmp_log(LOG_ERR, "unknown mode (%d) inhandle_acMemoryType\n", reqinfo->mode );
			return SNMP_ERR_GENERR;
		}

		snmp_log(LOG_DEBUG, "exit handle_acMemoryType\n");
		return SNMP_ERR_NOERROR;


}
int
handle_acMemoryCapacity(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
	/* We are never called for a GETNEXT if it's registered as a
	"instance", as it's "magically" handled for us.  */

	/* a instance handler also only hands us one request at a time, so
	we don't need to loop over a list of requests; we'll only get one. */
	snmp_log(LOG_DEBUG, "enter handle_acMemoryCapacity\n");

	switch(reqinfo->mode) 
	{

		case MODE_GET:
		{
			get_mem_usage(&st_sys_info);
			unsigned int mem_total = st_sys_info.pst_mem_status->un_mem_total;//M_SIZE;
			snmp_set_var_typed_value(requests->requestvb, ASN_GAUGE,
									(u_char *)&mem_total,
									sizeof(mem_total));
		}
		break;


		default:
		/* we should never get here, so this is a really bad error */
		snmp_log(LOG_ERR, "unknown mode (%d) inhandle_acMemoryCapacity\n", reqinfo->mode );
		return SNMP_ERR_GENERR;
	}

	snmp_log(LOG_DEBUG, "exit handle_acMemoryCapacity\n");
	return SNMP_ERR_NOERROR;
}


#if 0
int get_sample_time()
{

	int instance_id = 0;
	int ret = -1;
	int time = 0;
	DCLI_AC_API_GROUP_FIVE *sample_info;

	
	instance_id = get_main_instance_id();
	ret=show_sample_info(instance_id,&sample_info);
	if(ret == 1)
	{
		time = sample_info->sample_info->sample_time;
	}
	
	if(ret == 1)
	{
		Free_sample_info(sample_info);
	}

	return time;
}
#endif

int
handle_acMemRTUsage(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
	/* We are never called for a GETNEXT if it's registered as a
	"instance", as it's "magically" handled for us.  */

	/* a instance handler also only hands us one request at a time, so
	we don't need to loop over a list of requests; we'll only get one. */
	snmp_log(LOG_DEBUG, "enter handle_acMemRTUsage\n");

	switch(reqinfo->mode) 
	{

		case MODE_GET:
			#if 0
		{
			int acSample = get_sample_time();	
		
			static int prv_load_time = 0;
			
			int timenow = time(0);			
			static int acMemRTUsage = 0;

			if( timenow - prv_load_time > acSample )
			{
				prv_load_time = timenow;
		    	get_mem_usage(&st_sys_info);
			
				float mem_used = st_sys_info.pst_mem_status->un_mem_used/M_SIZE;
	    		float mem_total = st_sys_info.pst_mem_status->un_mem_total/M_SIZE;
				if(mem_total!=0)
				{
					acMemRTUsage = (mem_used/mem_total)*100;
				}
				else
				{
					acMemRTUsage = 0;
				}
			}
			snmp_set_var_typed_value(requests->requestvb,ASN_INTEGER,
									(u_char *)&acMemRTUsage,
									sizeof(acMemRTUsage));
		}
		#endif			
		#if 0
		{
			int acMemAvgUsage = 0;
			int retu = 0;

			retu = trap_read(&acMemAvgUsage,"memory_average");
			if(retu!=0)
			{
				acMemAvgUsage = 0;
			}

			snmp_set_var_typed_value(requests->requestvb,ASN_INTEGER,
										(u_char *)&acMemAvgUsage,
										sizeof(acMemAvgUsage));
		}
		#endif
		{
			#if 0
			unsigned int acMemRTUsage = 0;
			unsigned int acMemPeakUsage = 0;
			unsigned int acMemAvgUsage = 0;
			dbus_get_sample_info(ccgi_dbus_connection,SAMPLE_NAME_MEMUSAGE,&acMemRTUsage,&acMemAvgUsage, &acMemPeakUsage);
			#endif

			update_data_for_dbus_get_sample_memusage_info();
			snmp_set_var_typed_value(requests->requestvb,ASN_INTEGER,
										(u_char *)&acMemRTUsage,
										sizeof(acMemRTUsage));
		}
		break;


		default:
		/* we should never get here, so this is a really bad error */
		snmp_log(LOG_ERR, "unknown mode (%d) inhandle_acMemRTUsage\n", reqinfo->mode );
		return SNMP_ERR_GENERR;
	}

	snmp_log(LOG_DEBUG, "exit handle_acMemRTUsage\n");
	return SNMP_ERR_NOERROR;
}
int
handle_acMemPeakUsage(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
	/* We are never called for a GETNEXT if it's registered as a
	"instance", as it's "magically" handled for us.  */

	/* a instance handler also only hands us one request at a time, so
	we don't need to loop over a list of requests; we'll only get one. */
	snmp_log(LOG_DEBUG, "enter handle_acMemPeakUsage\n");

 	switch(reqinfo->mode) 
	{

		case MODE_GET:
		{
			#if 0
			int acMemPeakUsage = 0;
			int retu = 0;
			//char *useage = (char *)malloc(10);
			//memset(useage,0,10);
			retu = trap_read(&acMemPeakUsage,"memory_peak");
			if(retu!=0)
			{
				acMemPeakUsage = 0;
			}
			//sprintf(useage,"%d",acMemPeakUsage);
			//strcat(useage,"%");
			
			snmp_set_var_typed_value(requests->requestvb,ASN_INTEGER,
										(u_char *)&acMemPeakUsage,
										sizeof(acMemPeakUsage));
			
			//free(useage);
			unsigned int acMemRTUsage = 0;
			unsigned int acMemPeakUsage = 0;
			unsigned int acMemAvgUsage = 0;
			dbus_get_sample_info(ccgi_dbus_connection,SAMPLE_NAME_MEMUSAGE,&acMemRTUsage,&acMemAvgUsage, &acMemPeakUsage);
			#endif			

			update_data_for_dbus_get_sample_memusage_info();
			snmp_set_var_typed_value(requests->requestvb,ASN_INTEGER,
										(u_char *)&acMemPeakUsage,
										sizeof(acMemPeakUsage));
		}
		break;


		default:
		/* we should never get here, so this is a really bad error */
		snmp_log(LOG_ERR, "unknown mode (%d) inhandle_acMemRTUsage\n", reqinfo->mode );
		return SNMP_ERR_GENERR;
	}

	snmp_log(LOG_DEBUG, "exit handle_acMemPeakUsage\n");
	return SNMP_ERR_NOERROR;
}
int
handle_acMemAvgUsage(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
	/* We are never called for a GETNEXT if it's registered as a
	"instance", as it's "magically" handled for us.  */

	/* a instance handler also only hands us one request at a time, so
	we don't need to loop over a list of requests; we'll only get one. */
	
	snmp_log(LOG_DEBUG, "enter handle_acMemAvgUsage\n");
	
	switch(reqinfo->mode) 
	{

		case MODE_GET:
		{
			#if 0
			int acMemAvgUsage = 0;
			int retu = 0;

			retu = trap_read(&acMemAvgUsage,"memory_average");
			if(retu!=0)
			{
				acMemAvgUsage = 0;
			}

			snmp_set_var_typed_value(requests->requestvb,ASN_INTEGER,
										(u_char *)&acMemAvgUsage,
										sizeof(acMemAvgUsage));
			unsigned int acMemRTUsage = 0;
			unsigned int acMemPeakUsage = 0;
			unsigned int acMemAvgUsage = 0;
			dbus_get_sample_info(ccgi_dbus_connection,SAMPLE_NAME_MEMUSAGE,&acMemRTUsage,&acMemAvgUsage, &acMemPeakUsage);
			#endif

			update_data_for_dbus_get_sample_memusage_info();
			snmp_set_var_typed_value(requests->requestvb,ASN_INTEGER,
										(u_char *)&acMemAvgUsage,
										sizeof(acMemAvgUsage));
		}
		break;


		default:
		/* we should never get here, so this is a really bad error */
		snmp_log(LOG_ERR, "unknown mode (%d) inhandle_acMemRTUsage\n", reqinfo->mode );
		return SNMP_ERR_GENERR;
	}

	snmp_log(LOG_DEBUG, "exit handle_acMemAvgUsage\n");
	return SNMP_ERR_NOERROR;
}
int
handle_acMemUsageThreshhd(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
	snmp_log(LOG_DEBUG, "enter handle_acMemUsageThreshhd\n");

	switch(reqinfo->mode) 
	{

		case MODE_GET:
		{
			int acMemUsageThreshhd = 0;
			int retu = 0;

			snmp_log(LOG_DEBUG, "enter acMemUsageThreshhd_dbus_acsample\n");
			/*lixiang add at 2010-12-13*/
			retu = dbus_get_signal_threshold( ccgi_dbus_connection, SAMPLE_NAME_MEMUSAGE, &acMemUsageThreshhd);
	//		retu = trap_read_conf(&acMemUsageThreshhd,"memory_threshold");
			snmp_log(LOG_DEBUG, "exit acMemUsageThreshhd_dbus_acsample, retu=%d\n", retu);
			
			if(retu != 0)
			{
				acMemUsageThreshhd = 90;//GMCC set default ac mem threshhd
			}
			
			snmp_set_var_typed_value(requests->requestvb,ASN_INTEGER,
									(u_char *)&acMemUsageThreshhd,
									sizeof(acMemUsageThreshhd));
		}
		break;
		/*
		* SET REQUEST
		*
		* multiple states in the transaction.  See:
		* http://www.net-snmp.org/tutorial-5/toolkit/mib_module/set-actions.jpg
		*/

		case MODE_SET_RESERVE1:
		//   if (/* XXX: check incoming data in requests->requestvb->val.XXX for failures, like an incorrect type or an illegal value or ... */) {
		//      netsnmp_set_request_error(reqinfo, requests, /* XXX: set error code depending on problem (like SNMP_ERR_WRONGTYPE or SNMP_ERR_WRONGVALUE or ... */);
		//  }
		break;

		case MODE_SET_RESERVE2:
		/* XXX malloc "undo" storage buffer */
		//  if (/* XXX if malloc, or whatever, failed: */) {

		//       netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_RESOURCEUNAVAILABLE);

		//  }
		break;

		case MODE_SET_FREE:
		/* XXX: free resources allocated in RESERVE1 and/or
		RESERVE2.  Something failed somewhere, and the states
		below won't be called. */
		break;

		case MODE_SET_ACTION:
		/* XXX: perform the value change here */
		//  if (/* XXX: error? */) {
		//      netsnmp_set_request_error(reqinfo, requests, /* some error */);
		//  }
		{
			int retu = 0;
			if((*requests->requestvb->val.integer>0)&&(*requests->requestvb->val.integer<=100))
			{
				instance_parameter *slotConnect_head = NULL, *slotConnect_node = NULL;		
				list_instance_parameter(&slotConnect_head, SNMPD_SLOT_CONNECT);
				for(slotConnect_node = slotConnect_head; NULL != slotConnect_node; slotConnect_node = slotConnect_node->next)
				{
					/*lixiang change at 2010-12-13*/
					retu = dbus_set_signal_threshold( slotConnect_node->connection, SAMPLE_NAME_MEMUSAGE, *requests->requestvb->val.integer);
					if(0 != retu)	
						netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_NOACCESS);
					//trap_mod_conf(*requests->requestvb->val.integer,"memory_threshold");
				}
				free_instance_parameter_list(&slotConnect_head);
			}
			else
			{
				netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
			}
		}   
		break;
		case MODE_SET_COMMIT:
		/* XXX: delete temporary storage */
		// if (/* XXX: error? */) {
		/* try _really_really_ hard to never get to this point */
		//    netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_COMMITFAILED);
		// }
		break;

		case MODE_SET_UNDO:
		/* XXX: UNDO and return to previous value for the object */
		// if (/* XXX: error? */) {
		/* try _really_really_ hard to never get to this point */
		//     netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_UNDOFAILED);
		// }
		break;
		default:
		/* we should never get here, so this is a really bad error */
		snmp_log(LOG_ERR, "unknown mode (%d) inhandle_acMemUsageThreshhd\n", reqinfo->mode );
		return SNMP_ERR_GENERR;
	}

	snmp_log(LOG_DEBUG, "exit handle_acMemUsageThreshhd\n");
	return SNMP_ERR_NOERROR;


}
int
handle_acCPUType(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
	snmp_log(LOG_DEBUG, "enter handle_acCPUType\n");

	switch(reqinfo->mode) 
	{

		case MODE_GET:
		{
			FILE *get_cpu = NULL;
			char cpuType[50] = {0};
			memset(cpuType,0,50);
			
			snmp_log(LOG_DEBUG, "enter popen\n");
			get_cpu = popen("cat /proc/cpuinfo | grep \"cpu model\" | sed \"2,200d\" | awk -v FS=\":\" '{print $2}'","r");
			snmp_log(LOG_DEBUG, "exit popen,get_cpu=%p\n", get_cpu);
			
			if(get_cpu != NULL)
			{
				fgets(cpuType,50,get_cpu);
				delete_enter(cpuType);
				pclose(get_cpu);
			}
			
			snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
										(u_char *)cpuType,
										strlen(cpuType));
			
		}
		break;


		default:
		/* we should never get here, so this is a really bad error */
		snmp_log(LOG_ERR, "unknown mode (%d) inhandle_acCPUType\n", reqinfo->mode );
		return SNMP_ERR_GENERR;
	}

	snmp_log(LOG_DEBUG, "exit handle_acCPUType\n");
	return SNMP_ERR_NOERROR;


}
int
handle_acCPURTUsage(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
	/* We are never called for a GETNEXT if it's registered as a
	"instance", as it's "magically" handled for us.  */

	/* a instance handler also only hands us one request at a time, so
	we don't need to loop over a list of requests; we'll only get one. */
 	
	snmp_log(LOG_DEBUG, "enter handle_acCPURTUsage\n");
	
	switch(reqinfo->mode) 
	{

		case MODE_GET:
			#if 0
		{
			int acSample = get_sample_time();
			
			//trap_read(&acSample,"sample");
			
			static int prv_load_time = 0;
			
			int timenow = time(0);
			unsigned int cpu_seq = 0;
			unsigned int cpu_usage = 0;
			unsigned int total_cpu_usage = 0;				
			static int userate = 0;

			if( timenow - prv_load_time > acSample )
			{
				prv_load_time = timenow;
		    	get_cpu_usage(&st_sys_info, DEFAULT_REFRESH);
			
				for(;cpu_seq<st_sys_info.pst_cpus_status->cpu_no;cpu_seq++)
				{
			        cpu_usage = (unsigned int)(100 - st_sys_info.pst_cpus_status->ar_cpu_usage[cpu_seq][CPU_USAGE_IDEL]);
					total_cpu_usage+=cpu_usage;
				}

				userate = total_cpu_usage/st_sys_info.pst_cpus_status->cpu_no;
			}
			snmp_set_var_typed_value(requests->requestvb,ASN_INTEGER,
									(u_char*)&userate,
									sizeof(userate));
		}
			#endif
			#if 0
		{
			int acCPUAvgUsage = 0;
			int retu = 0;

			retu = trap_read(&acCPUAvgUsage,"cpu_average");
			if(retu!=0)
			{
				acCPUAvgUsage = 0;
			}

			snmp_set_var_typed_value(requests->requestvb,ASN_INTEGER,
									(u_char *)&acCPUAvgUsage,
									sizeof(acCPUAvgUsage));
		}
			#endif			
			{
				#if 0
				unsigned int acCPURTUsage = 0;
				unsigned int acCPUAvgUsage = 0;
				unsigned int acCPUPeakUsage = 0;
				dbus_get_sample_info(ccgi_dbus_connection,SAMPLE_NAME_CPU,&acCPURTUsage,&acCPUAvgUsage, &acCPUPeakUsage);
				#endif

				update_data_for_dbus_get_sample_cpuusage_info();
				snmp_set_var_typed_value(requests->requestvb,ASN_INTEGER,
											(u_char *)&acCPURTUsage,
											sizeof(acCPURTUsage));
			}
		break;


		default:
		/* we should never get here, so this is a really bad error */
		snmp_log(LOG_ERR, "unknown mode (%d) in handle_acCPURTUsage\n",reqinfo->mode );
		return SNMP_ERR_GENERR;
	}

	snmp_log(LOG_DEBUG, "exit handle_acCPURTUsage\n");
	return SNMP_ERR_NOERROR;
}
int
handle_acCPUPeakUsage(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
	/* We are never called for a GETNEXT if it's registered as a
	"instance", as it's "magically" handled for us.  */

	/* a instance handler also only hands us one request at a time, so
	we don't need to loop over a list of requests; we'll only get one. */
	
	snmp_log(LOG_DEBUG, "enter handle_acCPUPeakUsage\n");
	
	switch(reqinfo->mode) 
	{

		case MODE_GET:
		{
			#if 0
			int acCPUPeakUsage = 0;
			int retu = 0;
			char *useage = (char *)malloc(10);
			memset(useage,0,10);
			retu = trap_read(&acCPUPeakUsage,"cpu_peak");
			if(retu!=0)
			{
				acCPUPeakUsage = 0;
			}
			sprintf(useage,"%d",acCPUPeakUsage);
			
			snmp_set_var_typed_value(requests->requestvb,ASN_OCTET_STR,
									(u_char *)useage,
									strlen(useage));
			
			free(useage);
			unsigned int acCPURTUsage = 0;
			unsigned int acCPUAvgUsage = 0;
			unsigned int acCPUPeakUsage = 0;
			dbus_get_sample_info(ccgi_dbus_connection,SAMPLE_NAME_CPU,&acCPURTUsage,&acCPUAvgUsage, &acCPUPeakUsage);
			#endif

			update_data_for_dbus_get_sample_cpuusage_info();
			snmp_set_var_typed_value(requests->requestvb,ASN_INTEGER,
										(u_char *)&acCPUPeakUsage,
										sizeof(acCPUPeakUsage));
		}
		break;


		default:
		/* we should never get here, so this is a really bad error */
		snmp_log(LOG_ERR, "unknown mode (%d) inhandle_acMemRTUsage\n", reqinfo->mode );
		return SNMP_ERR_GENERR;
	}

	snmp_log(LOG_DEBUG, "exit handle_acCPUPeakUsage\n");
	return SNMP_ERR_NOERROR;
}
int
handle_acCPUAvgUsage(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
	/* We are never called for a GETNEXT if it's registered as a
	"instance", as it's "magically" handled for us.  */

	/* a instance handler also only hands us one request at a time, so
	we don't need to loop over a list of requests; we'll only get one. */
	snmp_log(LOG_DEBUG, "enter handle_acCPUAvgUsage\n");
	
	switch(reqinfo->mode) 
	{

		case MODE_GET:
		{
			#if 0
			int acCPUAvgUsage = 0;
			int retu = 0;

			retu = trap_read(&acCPUAvgUsage,"cpu_average");
			if(retu!=0)
			{
				acCPUAvgUsage = 0;
			}

			snmp_set_var_typed_value(requests->requestvb,ASN_INTEGER,
									(u_char *)&acCPUAvgUsage,
									sizeof(acCPUAvgUsage));
			unsigned int acCPURTUsage = 0;
			unsigned int acCPUAvgUsage = 0;
			unsigned int acCPUPeakUsage = 0;
			dbus_get_sample_info(ccgi_dbus_connection,SAMPLE_NAME_CPU,&acCPURTUsage,&acCPUAvgUsage, &acCPUPeakUsage);
			#endif

			update_data_for_dbus_get_sample_cpuusage_info();
			snmp_set_var_typed_value(requests->requestvb,ASN_INTEGER,
										(u_char *)&acCPUAvgUsage,
										sizeof(acCPUAvgUsage));
		}
		break;


		default:
		/* we should never get here, so this is a really bad error */
		snmp_log(LOG_ERR, "unknown mode (%d) inhandle_acMemRTUsage\n", reqinfo->mode );
		return SNMP_ERR_GENERR;
	}

	snmp_log(LOG_DEBUG, "exit handle_acCPUAvgUsage\n");
	return SNMP_ERR_NOERROR;
}
int
handle_acCPUusageThreshhd(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
	
	snmp_log(LOG_DEBUG, "enter handle_acCPUusageThreshhd\n");
	
	switch(reqinfo->mode) 
	{

		case MODE_GET:
		{
			int acCPUusageThreshhd = 0;
			int retu = 0;

			snmp_log(LOG_DEBUG, "enter trap_read_conf\n");
			/*lixiang add at 2010-12-13*/
			retu = dbus_get_signal_threshold(ccgi_dbus_connection, SAMPLE_NAME_CPU, &acCPUusageThreshhd);
			//retu = trap_read_conf(&acCPUusageThreshhd,"cpu_threshold");
			snmp_log(LOG_DEBUG, "exit trap_read_conf,retu=%d\n", retu);
			
			if(retu != 0)
			{
				acCPUusageThreshhd = 80;//GMCC set default ac mem threshhd
			}

			snmp_set_var_typed_value(requests->requestvb,ASN_INTEGER,
									(u_char *)&acCPUusageThreshhd,
									sizeof(acCPUusageThreshhd));
		}
		break;

		/*
		* SET REQUEST
		*
		* multiple states in the transaction.  See:
		* http://www.net-snmp.org/tutorial-5/toolkit/mib_module/set-actions.jpg
		*/

		case MODE_SET_RESERVE1:
		//   if (/* XXX: check incoming data in requests->requestvb->val.XXX for failures, like an incorrect type or an illegal value or ... */) {
		//      netsnmp_set_request_error(reqinfo, requests, /* XXX: set error code depending on problem (like SNMP_ERR_WRONGTYPE or SNMP_ERR_WRONGVALUE or ... */);
		//  }
		break;

		case MODE_SET_RESERVE2:
		/* XXX malloc "undo" storage buffer */
		//  if (/* XXX if malloc, or whatever, failed: */) {

		//       netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_RESOURCEUNAVAILABLE);

		//  }
		break;

		case MODE_SET_FREE:
		/* XXX: free resources allocated in RESERVE1 and/or
		RESERVE2.  Something failed somewhere, and the states
		below won't be called. */
		break;

		case MODE_SET_ACTION:
		/* XXX: perform the value change here */
		//  if (/* XXX: error? */) {
		//      netsnmp_set_request_error(reqinfo, requests, /* some error */);
		//  }
		{
			int retu = 0;
			if((*requests->requestvb->val.integer>0)&&(*requests->requestvb->val.integer<=100))
			{
				instance_parameter *slotConnect_head = NULL, *slotConnect_node = NULL;
				list_instance_parameter(&slotConnect_head, SNMPD_SLOT_CONNECT);
				for(slotConnect_node = slotConnect_head; NULL != slotConnect_node; slotConnect_node = slotConnect_node->next)
				{
					/*lixiang change at 2010-12-13*/
					retu = dbus_set_signal_threshold( slotConnect_node->connection, SAMPLE_NAME_CPU, *requests->requestvb->val.integer);
					if(0 != retu)
					{
						netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_NOACCESS);
					}
					//retu = trap_mod_conf(*requests->requestvb->val.integer,"cpu_threshold");
				}
				free_instance_parameter_list(&slotConnect_head);
			}
			else
			{
				netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGTYPE);
			}
		}   
		break;
		case MODE_SET_COMMIT:
		/* XXX: delete temporary storage */
		// if (/* XXX: error? */) {
		/* try _really_really_ hard to never get to this point */
		//    netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_COMMITFAILED);
		// }
		break;

		case MODE_SET_UNDO:
		/* XXX: UNDO and return to previous value for the object */
		// if (/* XXX: error? */) {
		/* try _really_really_ hard to never get to this point */
		//     netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_UNDOFAILED);
		// }
		break;		default:
		/* we should never get here, so this is a really bad error */
		snmp_log(LOG_ERR, "unknown mode (%d) inhandle_acCPUusageThreshhd\n", reqinfo->mode );
		return SNMP_ERR_GENERR;
	}

	snmp_log(LOG_DEBUG, "exit handle_acCPUusageThreshhd\n");
	return SNMP_ERR_NOERROR;



}
int
handle_acFlashType(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    snmp_log(LOG_DEBUG, "enter handle_acFlashType\n");

    switch(reqinfo->mode) {
        case MODE_GET:
            snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
                                     (u_char *)"SDRAM",
                                     strlen("SDRAM"));
            break;
        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_acFlashType\n",reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acFlashType\n");
    return SNMP_ERR_NOERROR;
}int
handle_acFlashCapacity(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    //float acFlashCapacity = 0;
	snmp_log(LOG_DEBUG, "enter handle_acFlashCapacity\n");
	
	switch(reqinfo->mode) 
	{
		case MODE_GET:
		{
			FILE *get_flash = NULL;
			char flash[50] = {0};
			memset(flash,0,50);
			char *endptr = NULL; 
			u_long acFlashCapacity = 0;

			snmp_log(LOG_DEBUG, "enter fopen\n");
			get_flash = fopen("/var/run/sad/totalspace","r");
			snmp_log(LOG_DEBUG, "exit fopen,get_flash=%p\n", get_flash);
			
			if(get_flash != NULL)
			{
				fgets(flash,50,get_flash);
				acFlashCapacity=strtoul(flash,&endptr,10)/1024;	/*char转成int，10代表十进制*/
				fclose(get_flash);
			}
			
			snmp_set_var_typed_value(requests->requestvb, ASN_GAUGE,
								(u_char *)&acFlashCapacity,
								sizeof(acFlashCapacity));
			
		}
		break;
		default:
		/* we should never get here, so this is a really bad error */
		snmp_log(LOG_ERR, "unknown mode (%d) in handle_acFlashCapacity\n",reqinfo->mode );
		return SNMP_ERR_GENERR;
	}
	
		snmp_log(LOG_DEBUG, "exit handle_acFlashCapacity\n");
	    return SNMP_ERR_NOERROR;
}
int
handle_acFlashResidualSpace(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    snmp_log(LOG_DEBUG, "enter handle_acFlashResidualSpace\n");

    switch(reqinfo->mode) {

        case MODE_GET:
		{
			char buff[128] = {0};
			char *endptr = NULL;
			GET_CMD_STDOUT(buff,sizeof(buff),"cat /var/run/sad/freespace");
			delete_enter(buff);
			snprintf(buff,sizeof(buff)-1,"%lu", strtoul(buff, &endptr, 10)/1024);
        	
            snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
                                     (u_char *)buff,
                                     strlen(buff));
        }
            break;
    

        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) inhandle_acFlashResidualSpace\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

	snmp_log(LOG_DEBUG, "exit handle_acFlashResidualSpace\n");
    return SNMP_ERR_NOERROR;
}
int
handle_acWorkTemperature(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
	/* We are never called for a GETNEXT if it's registered as a
	"instance", as it's "magically" handled for us.  */

	/* a instance handler also only hands us one request at a time, so
	we don't need to loop over a list of requests; we'll only get one. */
	snmp_log(LOG_DEBUG, "enter handle_acWorkTemperature\n");

	switch(reqinfo->mode) 
	{

		case MODE_GET:
		{
			char tempu[10] = { 0 };
			struct sys_envir envir;
			memset(&envir,0,sizeof(struct sys_envir));

			
			snmp_log(LOG_DEBUG, "enter show_sys_envir\n");
			show_sys_envir(&envir);
			snmp_log(LOG_DEBUG, "exit show_sys_envir,envir.core_tmprt=%d\n", envir.core_tmprt);
			
			memset(tempu,0,10);
			snprintf(tempu,sizeof(tempu)-1,"%d",envir.core_tmprt);
			strncat(tempu," C",sizeof(tempu)-strlen(tempu)-1);
			
			snmp_set_var_typed_value(requests->requestvb,ASN_OCTET_STR,
									(u_char *)tempu,
									strlen(tempu));
		}
		break;


		default:
		/* we should never get here, so this is a really bad error */
		snmp_log(LOG_ERR, "unknown mode (%d) inhandle_acWorkTemperature\n", reqinfo->mode );
		return SNMP_ERR_GENERR;
	}

	snmp_log(LOG_DEBUG, "exit handle_acWorkTemperature\n");
	return SNMP_ERR_NOERROR;
}

int
handle_acWtpCPUusageThreshhd(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
	snmp_log(LOG_DEBUG, "enter handle_acWtpCPUusageThreshhd\n");
	
	switch(reqinfo->mode) 
	{

		case MODE_GET:
        {				
            unsigned int cpu_usage_threshold = 90;
            int ret = 0;
            
            instance_parameter *paraHead = NULL;
            if(SNMPD_DBUS_SUCCESS == get_slot_dbus_connection(LOCAL_SLOT_NUM, &paraHead, SNMPD_INSTANCE_MASTER_V2))
            {                
            	WID_TRAP_THRESHOLD *threshold_info = NULL;
            	snmp_log(LOG_DEBUG, "enter show_ap_threshold_func\n");
            	ret = show_ap_trap_rogue_ap_ter_cpu_mem_threshold_cmd(paraHead->parameter, paraHead->connection, 0, &threshold_info);
            	snmp_log(LOG_DEBUG, "exit show_ap_threshold_func,ret=%d\n", ret);
            	
            	if(ret == 1)
            	{
            		cpu_usage_threshold = threshold_info->cpu;            		
            	}	
            	else if(SNMPD_CONNECTION_ERROR == ret) {
                    close_slot_dbus_connection(paraHead->parameter.slot_id);
                }
				Free_show_ap_trap_threshold(threshold_info);
            }
            free_instance_parameter_list(&paraHead);
            
            snmp_set_var_typed_value(requests->requestvb,ASN_INTEGER,
            							(u_char *)&cpu_usage_threshold,
            							sizeof(cpu_usage_threshold));
        }
		break;

		/*
		* SET REQUEST
		*
		* multiple states in the transaction.  See:
		* http://www.net-snmp.org/tutorial-5/toolkit/mib_module/set-actions.jpg
		*/

		case MODE_SET_RESERVE1:
		//   if (/* XXX: check incoming data in requests->requestvb->val.XXX for failures, like an incorrect type or an illegal value or ... */) {
		//      netsnmp_set_request_error(reqinfo, requests, /* XXX: set error code depending on problem (like SNMP_ERR_WRONGTYPE or SNMP_ERR_WRONGVALUE or ... */);
		//  }
		break;

		case MODE_SET_RESERVE2:
		/* XXX malloc "undo" storage buffer */
		//  if (/* XXX if malloc, or whatever, failed: */) {

		//       netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_RESOURCEUNAVAILABLE);

		//  }
		break;

		case MODE_SET_FREE:
		/* XXX: free resources allocated in RESERVE1 and/or
		RESERVE2.  Something failed somewhere, and the states
		below won't be called. */
		break;

		case MODE_SET_ACTION:
		/* XXX: perform the value change here */
		//  if (/* XXX: error? */) {
		//      netsnmp_set_request_error(reqinfo, requests, /* some error */);
		//  }
		{
    		int ret=0;
    		char thr_value[10] = {0};
    		
    		memset(thr_value,0,10);
    		snprintf(thr_value,sizeof(thr_value)-1,"%d",*requests->requestvb->val.integer);
    		
            instance_parameter *paraHead = NULL, *paraNode = NULL;
            list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER);
            
            for(paraNode = paraHead; NULL != paraNode; paraNode = paraNode->next) {
            
                snmp_log(LOG_DEBUG, "enter set_ap_cm_threshold_func, slot %d, local_id = %d, instanec_id = %d\n", 
                                     paraNode->parameter.slot_id, paraNode->parameter.local_id, paraNode->parameter.instance_id);

                ret = set_wtp_trap_threshold_cmd(paraNode->parameter, paraNode->connection, 0, "cpu", thr_value);
        		snmp_log(LOG_DEBUG, "exit set_ap_cm_threshold_func,ret=%d\n", ret);
        		
        		if(ret != 1)
        		{	
        		    if(SNMPD_CONNECTION_ERROR == ret) {
                        close_slot_dbus_connection(paraNode->parameter.slot_id);
            	    }
        			netsnmp_set_request_error(reqinfo,requests,SNMP_ERR_WRONGTYPE);
        		}
            }		
            free_instance_parameter_list(&paraHead);        		
		}   
		break;
		case MODE_SET_COMMIT:
		/* XXX: delete temporary storage */
		// if (/* XXX: error? */) {
		/* try _really_really_ hard to never get to this point */
		//    netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_COMMITFAILED);
		// }
		break;

		case MODE_SET_UNDO:
		/* XXX: UNDO and return to previous value for the object */
		// if (/* XXX: error? */) {
		/* try _really_really_ hard to never get to this point */
		//     netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_UNDOFAILED);
		// }
		break;		default:
		/* we should never get here, so this is a really bad error */
		snmp_log(LOG_ERR, "unknown mode (%d) inhandle_acCPUusageThreshhd\n", reqinfo->mode );
		return SNMP_ERR_GENERR;
	}

	snmp_log(LOG_DEBUG, "exit handle_acWtpCPUusageThreshhd\n");
	return SNMP_ERR_NOERROR;



}

int
handle_acWtpMemusageThreshhd(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
	snmp_log(LOG_DEBUG, "enter handle_acWtpMemusageThreshhd\n");
	
	switch(reqinfo->mode) 
	{

		case MODE_GET:
		{
			int ret = 0;
			unsigned int memoryuse = 90;
			
            instance_parameter *paraHead = NULL;
            if(SNMPD_DBUS_SUCCESS == get_slot_dbus_connection(LOCAL_SLOT_NUM, &paraHead, SNMPD_INSTANCE_MASTER_V2))
            {
                
            	WID_TRAP_THRESHOLD *threshold_info = NULL;
            	snmp_log(LOG_DEBUG, "enter show_ap_threshold_func\n");
            	ret = show_ap_trap_rogue_ap_ter_cpu_mem_threshold_cmd(paraHead->parameter, paraHead->connection, 0, &threshold_info);
            	snmp_log(LOG_DEBUG, "exit show_ap_threshold_func,ret=%d\n", ret);

    			if(ret == 1)
    			{
    				memoryuse = threshold_info->memoryuse;
    				Free_show_ap_trap_threshold(threshold_info);
    			}
    			else if(SNMPD_CONNECTION_ERROR == ret) {
                    close_slot_dbus_connection(paraHead->parameter.slot_id);
        	    }
			}
            free_instance_parameter_list(&paraHead);
            
            snmp_set_var_typed_value(requests->requestvb,ASN_INTEGER,
									(u_char *)&memoryuse,
									sizeof(memoryuse));
        }
		break;

		/*
		* SET REQUEST
		*
		* multiple states in the transaction.  See:
		* http://www.net-snmp.org/tutorial-5/toolkit/mib_module/set-actions.jpg
		*/

		case MODE_SET_RESERVE1:
		//   if (/* XXX: check incoming data in requests->requestvb->val.XXX for failures, like an incorrect type or an illegal value or ... */) {
		//      netsnmp_set_request_error(reqinfo, requests, /* XXX: set error code depending on problem (like SNMP_ERR_WRONGTYPE or SNMP_ERR_WRONGVALUE or ... */);
		//  }
		break;

		case MODE_SET_RESERVE2:
		/* XXX malloc "undo" storage buffer */
		//  if (/* XXX if malloc, or whatever, failed: */) {

		//       netsnmp_set_request_error(reqinfo, requests,SNMP_ERR_RESOURCEUNAVAILABLE);

		//  }
		break;

		case MODE_SET_FREE:
		/* XXX: free resources allocated in RESERVE1 and/or
		RESERVE2.  Something failed somewhere, and the states
		below won't be called. */
		break;

		case MODE_SET_ACTION:
		/* XXX: perform the value change here */
		//  if (/* XXX: error? */) {
		//      netsnmp_set_request_error(reqinfo, requests, /* some error */);
		//  }
		{
	        int ret=0;
    		char thr_value[10] = {0};		
    		memset(thr_value,0,10);
    		snprintf(thr_value,sizeof(thr_value)-1,"%d",*requests->requestvb->val.integer);
    		
            instance_parameter *paraHead = NULL, *paraNode = NULL;
            list_instance_parameter(&paraHead, SNMPD_INSTANCE_MASTER);
            for(paraNode = paraHead; NULL != paraNode; paraNode = paraNode->next) {
            
                snmp_log(LOG_DEBUG, "enter set_ap_cm_threshold_func, slot %d, local_id = %d, instanec_id = %d\n", 
                                     paraNode->parameter.slot_id, paraNode->parameter.local_id, paraNode->parameter.instance_id);
                ret = set_wtp_trap_threshold_cmd(paraNode->parameter, paraNode->connection, 0, "memory", thr_value);
        		snmp_log(LOG_DEBUG, "exit set_ap_cm_threshold_func,ret=%d\n", ret);
        		
        		if(ret != 1)
        		{		
        		    if(SNMPD_CONNECTION_ERROR == ret) {
                        close_slot_dbus_connection(paraNode->parameter.slot_id);
            	    }
        			netsnmp_set_request_error(reqinfo,requests,SNMP_ERR_WRONGTYPE);
        		}
        	}	
    		free_instance_parameter_list(&paraHead);
		}   
		break;
		case MODE_SET_COMMIT:
		/* XXX: delete temporary storage */
		// if (/* XXX: error? */) {
		/* try _really_really_ hard to never get to this point */
		//    netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_COMMITFAILED);
		// }
		break;

		case MODE_SET_UNDO:
		/* XXX: UNDO and return to previous value for the object */
		// if (/* XXX: error? */) {
		/* try _really_really_ hard to never get to this point */
		//     netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_UNDOFAILED);
		// }
		break;		default:
		/* we should never get here, so this is a really bad error */
		snmp_log(LOG_ERR, "unknown mode (%d) inhandle_acCPUusageThreshhd\n", reqinfo->mode );
		return SNMP_ERR_GENERR;
	}

	snmp_log(LOG_DEBUG, "exit handle_acWtpMemusageThreshhd\n");
	return SNMP_ERR_NOERROR;



}

int
handle_acCPUFrequency(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
	/* We are never called for a GETNEXT if it's registered as a
	"instance", as it's "magically" handled for us.  */

	/* a instance handler also only hands us one request at a time, so
	we don't need to loop over a list of requests; we'll only get one. */
	snmp_log(LOG_DEBUG, "enter handle_acCPUFrequency\n");

	switch(reqinfo->mode) 
	{

		case MODE_GET:
		{
			char temp[255] = { 0 };
			memset(temp,0,255);
			char *p1=NULL,*p2=NULL;
			int i = 0,j = 0; 
			char acCPUFrequency[20] = { 0 };
			memset(acCPUFrequency,0,20);
			strncpy(acCPUFrequency,"500MHz",sizeof(acCPUFrequency)-1);
			FILE *fp = NULL;

			snmp_log(LOG_DEBUG, "enter fopen\n");
			fp = fopen("/proc/cpuinfo","r");
			snmp_log(LOG_DEBUG, "exit fopen,fp=%p\n", fp);
			
			if(fp != NULL)
			{
				fgets(temp,sizeof(temp),fp);
				p1=strtok(temp,"-");
				if(p1 != NULL)
				{
					p2=strtok(NULL,"-");
					if(p2 != NULL)
					{
						memset(acCPUFrequency,0,20);
						strncpy(acCPUFrequency,p2,sizeof(acCPUFrequency)-1);
						strncat(acCPUFrequency,"MHz",sizeof(acCPUFrequency)-strlen(acCPUFrequency)-1);
					}
				}
				fclose(fp);
			}
			
			snmp_set_var_typed_value(requests->requestvb,ASN_OCTET_STR,
									(u_char *)acCPUFrequency,
									strlen(acCPUFrequency));
		}
		break;


		default:
		/* we should never get here, so this is a really bad error */
		snmp_log(LOG_ERR, "unknown mode (%d) inhandle_acCPUFrequency\n", reqinfo->mode );
		return SNMP_ERR_GENERR;
	}

	snmp_log(LOG_DEBUG, "exit handle_acCPUFrequency\n");
	return SNMP_ERR_NOERROR;
}

static void update_data_for_dbus_get_sample_memusage_info()
{
	struct sysinfo info;
	
	if(0 != update_time_dbus_get_sample_memusage_info)
	{
		sysinfo(&info); 	
		if(info.uptime - update_time_dbus_get_sample_memusage_info < cache_time)
		{
			return;
		}
	}
		
	snmp_log(LOG_DEBUG, "enter update data for dbus_get_sample_memusage_info\n");
	
	/*update cache data*/
	int cllection_mode = 0;/*0表示关闭分板采集，1表示开启分板采集*/  
	acMemRTUsage = 0;
	acMemPeakUsage = 0;
	acMemAvgUsage = 0;
	
	if(snmp_cllection_mode(ccgi_dbus_connection))
	{
		cllection_mode = 1;
	}

	if(1 == cllection_mode)/*开启分板采集*/
	{
		snmp_log(LOG_DEBUG, "enter dbus_get_sample_memusage_info\n");
		dbus_get_sample_info(ccgi_dbus_connection,SAMPLE_NAME_MEMUSAGE,&acMemRTUsage,&acMemAvgUsage, &acMemPeakUsage);
		snmp_log(LOG_DEBUG, "exit dbus_get_sample_memusage_info\n");
	}	
	else
	{
		int slot_num = 0;
		unsigned int temp_acMemRTUsage = 0;
		unsigned int temp_acMemPeakUsage = 0;
		unsigned int temp_acMemAvgUsage = 0;
		instance_parameter *slotConnect_head = NULL, *slotConnect_node = NULL;

		list_instance_parameter(&slotConnect_head, SNMPD_SLOT_CONNECT);
		for(slotConnect_node = slotConnect_head; NULL != slotConnect_node; slotConnect_node = slotConnect_node->next)
		{
			snmp_log(LOG_DEBUG, "enter dbus_get_sample_memusage_info\n");
			dbus_get_sample_info(slotConnect_node->connection,SAMPLE_NAME_MEMUSAGE,&temp_acMemRTUsage,&temp_acMemAvgUsage, &temp_acMemPeakUsage);
			snmp_log(LOG_DEBUG, "exit dbus_get_sample_memusage_info\n");
			acMemRTUsage += temp_acMemRTUsage;
			acMemPeakUsage = (temp_acMemPeakUsage > acMemPeakUsage)? temp_acMemPeakUsage : acMemPeakUsage;
			acMemAvgUsage += temp_acMemAvgUsage;
			slot_num++;
		}
		free_instance_parameter_list(&slotConnect_head);				

		acMemRTUsage = acMemRTUsage / slot_num;
		acMemAvgUsage = acMemAvgUsage / slot_num;
	}
	
	sysinfo(&info); 		
	update_time_dbus_get_sample_memusage_info = info.uptime;

	snmp_log(LOG_DEBUG, "exit update data for dbus_get_sample_memusage_info\n");
}

static void update_data_for_dbus_get_sample_cpuusage_info()
{
	struct sysinfo info;
	
	if(0 != update_time_dbus_get_sample_cpuusage_info)
	{
		sysinfo(&info); 	
		if(info.uptime - update_time_dbus_get_sample_cpuusage_info < cache_time)
		{
			return;
		}
	}
		
	snmp_log(LOG_DEBUG, "enter update data for dbus_get_sample_cpuusage_info\n");
	
	/*update cache data*/
	int cllection_mode = 0;/*0表示关闭分板采集，1表示开启分板采集*/  
	acCPURTUsage = 0;
	acCPUAvgUsage = 0;
	acCPUPeakUsage = 0;

	if(snmp_cllection_mode(ccgi_dbus_connection))
	{
		cllection_mode = 1;
	}

	if(1 == cllection_mode)/*开启分板采集*/
	{
		snmp_log(LOG_DEBUG, "enter dbus_get_sample_cpuusage_info\n");
		dbus_get_sample_info(ccgi_dbus_connection,SAMPLE_NAME_CPU,&acCPURTUsage,&acCPUAvgUsage, &acCPUPeakUsage);
		snmp_log(LOG_DEBUG, "exit dbus_get_sample_cpuusage_info\n");
	}
	else
	{
		int slot_num = 0;
		unsigned int temp_acCPURTUsage = 0;
		unsigned int temp_acCPUAvgUsage = 0;
		unsigned int temp_acCPUPeakUsage = 0;
		instance_parameter *slotConnect_head = NULL, *slotConnect_node = NULL;

		list_instance_parameter(&slotConnect_head, SNMPD_SLOT_CONNECT);
		for(slotConnect_node = slotConnect_head; NULL != slotConnect_node; slotConnect_node = slotConnect_node->next)
		{
			snmp_log(LOG_DEBUG, "enter dbus_get_sample_cpuusage_info\n");
			dbus_get_sample_info(slotConnect_node->connection,SAMPLE_NAME_CPU,&temp_acCPURTUsage,&temp_acCPUAvgUsage, &temp_acCPUPeakUsage);
			snmp_log(LOG_DEBUG, "exit dbus_get_sample_cpuusage_info\n");
			acCPURTUsage += temp_acCPURTUsage;
			acCPUPeakUsage = (temp_acCPUPeakUsage > acCPUPeakUsage)? temp_acCPUPeakUsage : acCPUPeakUsage;
			acCPUAvgUsage += temp_acCPUAvgUsage;
			slot_num++;
		}
		free_instance_parameter_list(&slotConnect_head);

		acCPURTUsage = acCPURTUsage / slot_num;
		acCPUAvgUsage = acCPUAvgUsage / slot_num;
	}
	
	sysinfo(&info); 		
	update_time_dbus_get_sample_cpuusage_info = info.uptime;

	snmp_log(LOG_DEBUG, "exit update data for dbus_get_sample_cpuusage_info\n");
}

