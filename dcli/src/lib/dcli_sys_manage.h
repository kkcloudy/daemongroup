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
* dcli_sys_manage.h
*
* MODIFY:
*		
*
* CREATOR:
*		shanchx@autelan.com
*
* DESCRIPTION:
*		CLI definition for system manage module.
*
* DATE:
*		02/13/2009
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.7 $	
*  			
*******************************************************************************/


#ifndef __DCLI_SYS_MANAGE_H__
#define __DCLI_SYS_MANAGE_H__


extern DBusConnection *dcli_dbus_connection;
#define SYS_LOCATION_CONFIG_FILE "/var/run/sys_location"
#define SYS_LOCATION_STR_LEN 128
#define SYS_LOCATION_PREFIX  "SYSTEM LOCATION:"

#define NET_ELEMENT_CONFIG_FILE  "/var/run/net_elemnet"
#define NET_ELEMENT_STR_LEN 128
#define NET_ELEMENT_PREFIX  "NET ELEMENT:"
#define TIME_SLOT_FILE_NAME "time-slot"
#define TIME_SLOT_FILE  "/mnt/"TIME_SLOT_FILE_NAME

#define DEMO_DBUS_BUSNAME 				"aw.demo"
#define DEMO_DBUS_OBJPATH 				"/aw/demo"
#define DEMO_DBUS_INTERFACE 			"aw.demo"
#define DEMO_DBUS_METHOD_GET_SLOT_NUM 	"demo_get_slot"

/**gjd : add for pfm**/
#define PFM_DBUS_BUSNAME				"pfm.daemon"
#define PFM_DBUS_OBJPATH				"/pfm/daemon"
#define PFM_DBUS_INTERFACE				"pfm.daemon"
#define PFM_DBUS_METHOD_PFM_TABLE 	"pfm_maintain_table"

#define PFM_SETUP_FILE 			"/var/run/pfm_setup"
#define PFM_SETUP_TMP_FILE 		"/var/run/pfm_setup_tmp"
#define PFM_DNS_FILE			"/var/run/pfm_dns"
#define PFM_DNS_FILE_BK			"/var/run/pfm_dns.bak"
#define PFM_RADIUS_FILE			"/var/run/pfm_radius"
#define PFM_RADIUS_FILE_BK			"/var/run/pfm_radius.bak"
#define PFM_MODULE_SECTION 		"\n!PFM section.\n"

/* sfd */
#define SFD_DBUS_BUSNAME			"sfd.daemon"
#define SFD_DBUS_OBJPATH			"/sfd/daemon"
#define SFD_DBUS_INTERFACE			"sfd.daemon"

#define SFD_DBUS_METHOD_LOG			"log_switcher"
#define SFD_DBUS_METHOD_DEBUG		"debug_switcher"

#define SFD_DBUS_METHOD_SWT			"sfd_switcher"
#define SFD_DBUS_METHOD_SWT_SLOT	"sfd_switcher_slot"

#define SFD_DBUS_METHOD_TMS			"sfd_timespan"
#define SFD_DBUS_METHOD_LMT			"sfd_limitpacket"
#define SFD_DBUS_METHOD_USERADD		"sfd_useradd"
#define SFD_DBUS_METHOD_USERDEL		"sfd_userdel"
#define SFD_DBUS_METHOD_VAR			"sfd_variables"
#define SFD_DBUS_METHOD_RUN			"sfd_running"


#define SFD_DBUS_METHOD_ARP_SWT		 "arp_switcher"
#define SFD_DBUS_METHOD_ARP_LMT		 "arp_limitpacket"
#define SFD_DBUS_METHOD_ARPUSERADD   "arp_useradd"
#define	SFD_DBUS_METHOD_ARPUSERDEL   "arp_userdel"

#define PRODUCT_SLOT_DIR "/dbm/product/slot/"
#define HOSTNAME_FILE	  "/host_name"


extern	void dcli_send_dbus_signal(const char* ,const char* );

/*add by zhaocg for "show memory" subcommand 2011-10-15*/

#define MEM_COMMAND_LEN 256
struct _slabinfo_title
{
	char prompt[4];
	char name[8];
	char active_objs[16];
	char num_objs[16];
	char objsize[16];
	char objperslab[16];
	char pagesperslab[16];
	char colon1[4];
	char tunables[16];
	char limit[8];
	char batchcount[16];
	char sharedfactor[16];
	char colon2[4];
	char slabdata[16];
	char active_slabs[16];
	char num_slabs[16]; 
	char sharedavail[16];
} ;
struct _slabinfo
{
		char name[32];	
		int act_objs;	
		int num_objs;	
		int objsize;	
		int objperslab;
		int pagesperslab;	
		char colon1[4];	
		char tunable[16];	
		int limit;	
		int batchcount;	
		int sharedfactor;	
		char colon2[4];	
		char slabdata[16];	
		int act_slabs;	
		int num_slabs;	
		int sharedatabail;
};
typedef struct _command
{
	char name[MEM_COMMAND_LEN];
	int rss;
	struct _command *next;
}ps_list;

/*end by zhaocg 2011-10-15*/

/*added by zhaocg for sfd 2012-12-4*/
#define FLAG_TCP  1
#define FLAG_ICMP 2
#define FLAG_SNMP 3
#define FLAG_CAPWAP 4


#define ENABLE_SFD 0x00
#define ENABLE_TCP  0x01
#define ENABLE_ICMP 0x02
#define ENABLE_SNMP 0x03
#define ENABLE_DNS 0x04
#define ENABLE_CAPWAP 0x05


#define DISABLE_SFD 0xFF
#define DISABLE_TCP  0x10
#define DISABLE_ICMP 0x20
#define DISABLE_SNMP 0x30
#define DISABLE_DNS 0x40
#define DISABLE_CAPWAP 0x50

#define TIMESPAN_DEF 1000
#define PACKET_DEF 50
#define SNMP_PACKET_DEF 200
#define TCP_PACKET_DEF 5000
#define ICMP_PACKET_DEF 200
#define ARP_PACKET_DEF 200
#define CAPWAP_PACKET_DEF 100



#endif

