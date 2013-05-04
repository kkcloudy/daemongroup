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
* dhcp_dbus.c
*
* MODIFY:
*		by <qinhs@autelan.com> on 05/20/2010 revision <0.1>
*
* CREATOR:
*		qinhs@autelan.com
*
* DESCRIPTION:
*		CLI definition for dhcp module.
*
* DATE:
*		05/20/2010
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.3 $	
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

#include <sys/ioctl.h>
#include <net/if.h>
#include <string.h>
#include <dbus/dbus.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>
#include <omapip/omapip_p.h>
#include <syslog.h>

#include "sysdef/returncode.h"
#include "dhcpd.h"
#include "dhcrelay_dbus.h"
#include "dbus/dhcp/dhcp_dbus_def.h"
#if (defined _D_WCPSS_ || defined _D_CC_)	
#include "wcpss/waw.h"
#include "wcpss/wid/WID.h"
#include "wcpss/asd/asd.h"
#endif

#define DHCRELAY_PID_BUFFER_LEN		128
#define ALIAS_NAME_SIZE             0x15
#define DHCRELAY_PID_PATH_LEN		DHCRELAY_PID_BUFFER_LEN
#define DHCRELAY_INVALID_FD		(-1)
#define DHCRELAY_FD_INIT		DHCRELAY_INVALID_FD
#define RELAY_HASH_SIZE 		65535
#define DHCRELAY_SAVE_CFG_MEM	(1024*1024)

#define DHCRELAY_PID_FILE_PATH "/var/run/"
#define DHCRELAY_PID_FILE_PREFIX "dhcrelay"
#define DHCRELAY_PID_FILE_SUFFIX ".pid"

pthread_t		*dbus_dhcrelay_thread;
pthread_attr_t	dbus_dhcrelay_thread_attr;
DBusConnection *dhcrelay_dbus_connection = NULL;
unsigned int global_current_instance_no = 0;
int g_dhcrelay_pid_fd = DHCRELAY_INVALID_FD;
struct dcli_relay dhcrelay_head;
unsigned int dhcrelay_count;

/*debug level*/
extern unsigned int dhcp_log_level;


extern int del_server_addr_ipv4(unsigned int ia);;
extern void del_relay_interface(	char* ifname);
extern int discover_interfaces_to_server(int state, char *name);


unsigned int
dhcrelay_dbus_four_char_to_int
(
	unsigned char* in_char,
	unsigned int *out_int
)
{
	if (!in_char) {
		return 1;
	}
	unsigned char *tmpchar = in_char;
	
	*out_int = 	(tmpchar[0]<<24) |
				(tmpchar[1]<<16) |
				(tmpchar[2]<<8) |
				(tmpchar[3]);

	return 0;
}

unsigned int 
dhcrelay_dbus_int_to_four_char
(
	unsigned int in_int,
	unsigned char *out_char
)
{
	if (! out_char) {
		return 1;
	}
	out_char[0] = (in_int>>24) & 0xFF;
	out_char[1] = (in_int>>16) & 0xFF;
	out_char[2] = (in_int>>8) & 0xFF;
	out_char[3] = in_int & 0xFF;
		
	return 0;
}

inline int dhcrelay_dbus_check_ipaddr_uint(unsigned int ipaddr) 
{	
	if ((0xffffffff == ipaddr)
		|| ((ipaddr & 0xff000000) == 0)) {
		return -1;
	}
	return 0;
}


#ifdef _D_WCPSS_ 	
int 
parse_hansi_radio_ifname
(
	char* ptr,
	int *vrrid, 
	int *wtpid,
	int *radioid,
	int *wlanid
)
{
	
    radio_ifname_state state = check_vrrid;
	char *str = NULL;
	str = ptr;
   
	while(1){
		switch(state){
		case check_vrrid: 
				
				*vrrid = strtoul(str,&str,10);
				
				if(*vrrid >= 0 && *vrrid < 255){
					state=check_sub;
				}
				else state=check_fail;
				
				break;
		case check_sub: 
		
			if (PARSE_RADIO_IFNAME_SUB == str[0]){
		
				state = check_wtpid;
				}
			else
				state = check_fail;
			break;
			
		case check_wtpid: 
			
			*wtpid = strtoul((char *)&(str[1]),&str,10);
			
			if(*wtpid > 0 && *wtpid < 4095){
        		state=check_sub1;
			}
			else state=check_fail;
			
			break;

		case check_sub1: 
		
			if (PARSE_RADIO_IFNAME_SUB == str[0]){
		
				state = check_radioid;
				}
			else
				state = check_fail;
			break;

		case check_radioid: 
		
			*radioid = strtoul((char *)&(str[1]),&str,10);

			if(*radioid >= 0 && *radioid < 4){/*radioid range 0 1 2 3*/
        		state=check_point;
			}
			else state=check_fail;
			
			break;

		case check_point: 
		
			if (PARSE_RADIO_IFNAME_POINT == str[0]){
			
				state = check_wlanid;
				
				}
			else
				state = check_fail;
			break;
				
		case check_wlanid: 
		
			*wlanid = strtoul((char *)&(str[1]),&str,10);

			if(*wlanid > 0 && *wlanid < WLAN_NUM+1){
        		state=check_end;
			}
			else state=check_fail;
			
			break;
			
		
		
		case check_fail:
	
		
            return -1;
			break;

		case check_end: 
	
			if ('\0' == str[0]) {
				state = check_success;
				
				}
			else
				state = check_fail;
				break;
			
		case check_success: 
		
			return 0;
			break;
			
		default: break;
		}
		
		}
		
}
#endif
#if (defined _D_WCPSS_ || defined _D_CC_)	
int 
parse_hansi_wlan_ebr_ifname
(
	char* ptr,
	int *vrrid, 
	int *wlanid
)
{
	
    radio_ifname_state state = check_vrrid;
	char *str = NULL;
	str = ptr;
   
	while(1){
		switch(state){
		case check_vrrid: 				
				*vrrid = strtoul(str,&str,10);
				
				if(*vrrid >= 0 && *vrrid < 255){
					state=check_sub;
				}
				else state=check_fail;
				
				break;
		case check_sub: 
		
			if (PARSE_RADIO_IFNAME_SUB == str[0]){
		
				state = check_wlanid;
				}
			else
				state = check_fail;
			break;
			
				
		case check_wlanid: 
		
			*wlanid = strtoul((char *)&(str[1]),&str,10);

			if(*wlanid > 0 && *wlanid < WLAN_NUM+1){
        		state=check_end;
			}
			else state=check_fail;
			
			break;
			
		
		
		case check_fail:
	
		
            return -1;
			break;

		case check_end: 
	
			if ('\0' == str[0]) {
				state = check_success;
				
				}
			else
				state = check_fail;
				break;
			
		case check_success: 
		
			return 0;
			break;
			
		default: break;
		}
		
	}
		
}
#endif

void 
dhcp_dbus_config_interface_save
(
	char **showif,
	char **ifcursor,
	struct dcli_relay *node,
	int *ifLen
)
{
	char ifname[25];
#if (defined _D_WCPSS_ || defined _D_CC_)	
	int vrrid = 0;
	int ebrid = 0;
	int wlanid = 0;
	int radioid = 0;
	int wtpid = 0;
	int ret = 0;
#endif
	while (node) {
		memset(ifname , 0, 25);
		memcpy(ifname, node->downifname, strlen(node->downifname));

		if ((!strncmp(node->downifname, "wlan", 4) || !strncmp(node->downifname, "ebr", 3))
			|| (!strncmp(node->upifname, "wlan", 4) || !strncmp(node->upifname, "ebr", 3))) {
			node = node->next;
			continue;
		}
		
#if 0		
#if (defined _D_WCPSS_ || defined _D_CC_)	
        if (!strncasecmp(ifname,"wlan",4)){
			ret = parse_hansi_wlan_ebr_ifname(ifname+4,&vrrid, &wlanid);
			if(ret == 0){
				memset(ifname, 0, 25);
				sprintf(ifname,"wlan%d",wlanid);				
				if(vrrid > 0){
					*ifLen += sprintf(*ifcursor, "config hansi-profile %d\n", vrrid);
					*ifcursor = *showif + *ifLen;		
				}		
			}else
				continue;

		}
        else if (!strncasecmp(ifname,"ebr",3)){
			ret = parse_hansi_wlan_ebr_ifname(ifname+3,&vrrid, &ebrid);
			if(ret == 0){
				memset(ifname, 0, 25);
				sprintf(ifname,"ebr%d",ebrid);	
				if(vrrid > 0){
					*ifLen += sprintf(*ifcursor, "config hansi-profile %d\n", vrrid);
					*ifcursor = *showif + *ifLen;
				}
			}else
				continue;
		}
#endif		
#ifdef _D_WCPSS_
        else if (!strncasecmp(ifname,"radio",5)){
			ret = parse_hansi_radio_ifname(ifname+5,&vrrid, &wtpid,&radioid,&wlanid);
			if(ret == 0){
				memset(ifname, 0, 25);
				sprintf(ifname,"radio%d-%d.%d",wtpid,radioid,wlanid);				
				if(vrrid > 0){
					*ifLen += sprintf(*ifcursor, "config hansi-profile %d\n", vrrid);
					*ifcursor = *showif + *ifLen;
				}
			}else
				continue;
		}
#endif
#endif
		if ((node->ipv4addr) & 0xff000000) {
			*ifLen += sprintf(*ifcursor, "interface %s\n", ifname);
			*ifcursor = *showif + *ifLen;
			
			*ifLen += sprintf(*ifcursor, " ip relay %s %u.%u.%u.%u\n", node->upifname,
				(((node->ipv4addr) & 0xff000000) >> 24), (((node->ipv4addr) & 0xff0000) >> 16),
				(((node->ipv4addr) & 0xff00) >> 8), ((node->ipv4addr) & 0xff));
			*ifcursor = *showif + *ifLen;	
			
			*ifLen += sprintf(*ifcursor, " exit\n");
			*ifcursor = *showif + *ifLen;
		}
#if 0		
#if (defined _D_WCPSS_ || defined _D_CC_)	
		if ((vrrid > 0)&&((!strncasecmp(ifname,"wlan",4))||(!strncasecmp(ifname,"radio",5))||(!strncasecmp(ifname,"ebr",3)))){
			*ifLen += sprintf(*ifcursor, " exit\n");
			*ifcursor = *showif + *ifLen;
		}
#endif	
		*ifLen += sprintf(*ifcursor, "\n");
		*ifcursor = *showif + *ifLen;
#endif
		node = node->next;
	}

}




void 
dhcrelay_dbus_hansi_config
(
	char *showStr,
	unsigned int slot,
	unsigned int vrrp,
	unsigned int local_flag
)
{
#define IFNAME_SIZE		(20)
	char downlink[IFNAME_SIZE];
	char uplink[IFNAME_SIZE];
	char *cursor = NULL;
	char *ifname = NULL;
	int totalLen = 0;
	int value1 = 0, value2 = 0, value3 = 0;
	int ret = 0;
	int valid_flag = 0;
	struct dcli_relay *node = NULL;

	if (!showStr) {
		return;
	}

	cursor = showStr;	
	node = dhcrelay_head.next;

	/* local hansi */
	if (local_flag) {
		if (vrrp) {
			totalLen += sprintf(cursor, "config local-hansi %d-%d\n", slot, vrrp);
			cursor = showStr + totalLen;
		}
		
		while (node) {			
			if ((!strncmp(node->downifname, "wlan", 4) || !strncmp(node->downifname, "ebr", 3))
				|| (!strncmp(node->upifname, "wlan", 4) || !strncmp(node->upifname, "ebr", 3))) {

				valid_flag = 0;

				/* get downlink name */
				ifname = node->downifname;	
				memset(downlink, 0, sizeof(downlink));				

				if (!strncmp(ifname, "wlanl", 5)) {
					ret = sscanf(ifname, "wlanl%d-%d-%d", &value1, &value2, &value3);
					if ((3 == ret) && (slot == value1) && (vrrp == value2)) {
						sprintf(downlink, "wlan%d", value3);
						valid_flag = 1;
					} 
				} else if (!strncmp(ifname, "ebrl", 4)) {
					ret = sscanf(ifname, "ebrl%d-%d-%d", &value1, &value2, &value3);
					if ((3 == ret) && (slot == value1) && (vrrp == value2)) {
						sprintf(downlink, "ebr%d", value3);
						valid_flag = 1;
					}
				} else {
					sprintf(downlink, "%s", ifname);
				}
				
				/* get uplink name */
				ifname = node->upifname;
				memset(uplink, 0, sizeof(uplink));
				
				if (!strncmp(ifname, "wlanl", 5)) {
					ret = sscanf(ifname, "wlanl%d-%d-%d", &value1, &value2, &value3);
					if ((3 == ret) && (slot == value1) && (vrrp == value2)) { 
						sprintf(uplink, "wlan%d", value3);
						valid_flag = 1;
					} 
				} else if (!strncmp(ifname, "ebrl", 4)) {
					ret = sscanf(ifname, "ebrl%d-%d-%d", &value1, &value2, &value3);
					if ((3 == ret) && (slot == value1) && (vrrp == value2)) { 
						sprintf(uplink, "ebr%d", value3);
						valid_flag = 1;
					}
				} else {
					sprintf(uplink, "%s", ifname);
				}

				if (valid_flag) {
					if ((node->ipv4addr) & 0xff000000) {
						totalLen += sprintf(cursor, "interface %s\n", downlink);
						cursor = showStr + totalLen;
						
						totalLen += sprintf(cursor, " ip relay %s %u.%u.%u.%u\n", uplink,
							(((node->ipv4addr) & 0xff000000) >> 24), (((node->ipv4addr) & 0xff0000) >> 16),
							(((node->ipv4addr) & 0xff00) >> 8), ((node->ipv4addr) & 0xff));
						cursor = showStr + totalLen;
						
						totalLen += sprintf(cursor, " exit\n");
						cursor = showStr + totalLen;
					}	
				}		
			}

			node = node->next;
		}

		if (vrrp) {
			totalLen += sprintf(cursor, " exit\n");
			cursor = showStr + totalLen;
		}
	} 

	/* hansi config */
	else {
		if (vrrp) {
			totalLen += sprintf(cursor, "config hansi-profile %d-%d\n", slot, vrrp);
			cursor = showStr + totalLen;
		}
		
		while (node) {			
			if ((!strncmp(node->downifname, "wlan", 4) || !strncmp(node->downifname, "ebr", 3))
				|| (!strncmp(node->upifname, "wlan", 4) || !strncmp(node->upifname, "ebr", 3))) {

				valid_flag = 0;

				/* get downlink name */
				ifname = node->downifname;	
				memset(downlink, 0, sizeof(downlink));
				
				if (!strncmp(ifname, "wlan", 4) && strncmp(ifname, "wlanl", 5)) {
					ret = sscanf(ifname, "wlan%d-%d-%d", &value1, &value2, &value3);
					if ((3 == ret) && (slot == value1) && (vrrp == value2)) {
						sprintf(downlink, "wlan%d", value3);
						valid_flag = 1;
					} 
				} else if (!strncmp(ifname, "ebr", 3) && strncmp(ifname, "ebrl", 4)) {
					ret = sscanf(ifname, "ebr%d-%d-%d", &value1, &value2, &value3);
					if ((3 == ret) && (slot == value1) && (vrrp == value2)) {
						sprintf(downlink, "ebr%d", value3);
						valid_flag = 1;
					}
				} else {
					sprintf(downlink, "%s", ifname);
				}
				
				/* get uplink name */
				ifname = node->upifname;
				memset(uplink, 0, sizeof(uplink));
				
				if (!strncmp(ifname, "wlan", 4) && strncmp(ifname, "wlanl", 5)) {
					ret = sscanf(ifname, "wlan%d-%d-%d", &value1, &value2, &value3);
					if ((3 == ret) && (slot == value1) && (vrrp == value2)) { 
						sprintf(uplink, "wlan%d", value3);
						valid_flag = 1;
					} 
				} else if (!strncmp(ifname, "ebr", 3) && strncmp(ifname, "ebrl", 4)) {
					ret = sscanf(ifname, "ebrl%d-%d-%d", &value1, &value2, &value3);
					if ((3 == ret) && (slot == value1) && (vrrp == value2)) { 
						sprintf(uplink, "ebr%d", value3);
						valid_flag = 1;
					}
				} else {
					sprintf(uplink, "%s", ifname);
				}

				if (valid_flag) {
					if ((node->ipv4addr) & 0xff000000) {
						totalLen += sprintf(cursor, "interface %s\n", downlink);
						cursor = showStr + totalLen;
						
						totalLen += sprintf(cursor, " ip relay %s %u.%u.%u.%u\n", uplink,
							(((node->ipv4addr) & 0xff000000) >> 24), (((node->ipv4addr) & 0xff0000) >> 16),
							(((node->ipv4addr) & 0xff00) >> 8), ((node->ipv4addr) & 0xff));
						cursor = showStr + totalLen;
						
						totalLen += sprintf(cursor, " exit\n");
						cursor = showStr + totalLen;
					}	
				}		
			}
			node = node->next;			
		}

		if (vrrp) {
			totalLen += sprintf(cursor, " exit\n");
			cursor = showStr + totalLen;
		}

	}
	return;
}

unsigned int 
dhcrelay_dbus_add_relay_node
(
	char *downifname,
	char *upifname,
	unsigned int ipaddr
)
{
	struct dcli_relay *relay_node = NULL;

	relay_node = malloc(sizeof(struct dcli_relay));
	memset(relay_node, 0, sizeof(struct dcli_relay));

	log_debug("add downifname %s len %u, upifname %s len %u\n", 
		downifname, strlen(downifname), upifname, strlen(upifname));	
		
	strncpy(relay_node->downifname, downifname, strlen(downifname));
	strncpy(relay_node->upifname, upifname, strlen(upifname));
	relay_node->ipv4addr = ipaddr;
	
log_debug("add downifname %s len %u, upifname %s len %u\n", 
	relay_node->downifname, strlen(relay_node->downifname), relay_node->upifname, strlen(relay_node->upifname));	

	relay_node->next = dhcrelay_head.next;
	dhcrelay_head.next = relay_node;
	dhcrelay_count ++;

	return 0;
}

unsigned int 
dhcrelay_dbus_del_relay_node
(
	char *downifname,
	char *upifname,
	unsigned int ipaddr
)
{	
	struct dcli_relay *relay_priv = NULL, *relay_head = NULL;
	unsigned int ret = 0, upif = 0, svip = 0;
	
	relay_head = dhcrelay_head.next;
	while (relay_head) {
		log_debug("downifname is %s , upifname is %s, ipaddr is %x \n",
					relay_head->downifname, relay_head->upifname, relay_head->ipv4addr);
		log_debug("downifname is %s , upifname is %s, ipaddr is %x \n",
					downifname, upifname, ipaddr);

		/*if more down interface use one up interface*/
		if (!strncmp(relay_head->upifname, upifname, strlen(upifname))) {
			upif ++;
		}
		
		/*if more down interface use one serverip*/
		if (relay_head->ipv4addr == ipaddr) {
			svip ++;
		}
		
		if (!strncmp(relay_head->downifname, downifname, strlen(downifname)) &&
			!strncmp(relay_head->upifname, upifname, strlen(upifname)) &&
			relay_head->ipv4addr == ipaddr) {

			if (relay_priv) {
				relay_priv->next = relay_head->next;
			}
			else {
				dhcrelay_head.next = relay_head->next;
			}
			ret |= 1;
			dhcrelay_count --;
		}
		
		relay_priv = relay_head;
		relay_head = relay_head->next;
	}

	if (upif > 1) ret |=4;
	if (svip > 1) ret |=2;
	
	return ret;
}

unsigned int 
dhcrelay_dbus_check_relay_node
(
	char *downifname,
	char *upifname,
	unsigned int ipaddr
)
{
	struct dcli_relay  *relay_head = NULL;
	unsigned int ret = 0;
	
	relay_head = dhcrelay_head.next;
	while (relay_head) {
		if (downifname) {
			if (!strncmp(relay_head->downifname, downifname, strlen(downifname))) {
				log_info("check_relay_node downifname interface %s is in used \n", downifname);
				ret |= 1;
			}
		}

		if (upifname) {
			if (!strncmp(relay_head->upifname, upifname, strlen(upifname))) {
				log_info("check_relay_node upifname interface %s is in used \n", upifname);
				ret |= 2;
			}
		}
		
		relay_head = relay_head->next;
	}

	return ret;
}

unsigned int 
dhcrelay_dbus_check_relay_interface
(
	char *ifname
)
{
	struct dcli_relay *relay_head = NULL;
	unsigned int ret = 0;
	
	relay_head = dhcrelay_head.next;
	while (relay_head) {
		if (ifname) {
			if (!strncmp(relay_head->upifname, ifname, strlen(ifname))) {
				log_info("dhcrelay_dbus_check_relay_interface upifname interface %s is in used \n", ifname);
				ret |= 1;
			}
			
			if (!strncmp(relay_head->downifname, ifname, strlen(ifname))) {
				log_info("dhcrelay_dbus_check_relay_interface downifname interface %s is in used \n", ifname);
				ret |= 2;
			}
		}

		relay_head = relay_head->next;
	}

	return ret;
}

/*****************************************************
 * dhcp_dbus_save_lease
 *		save dhcp whole times
 			 dhcp segment times
 			 dhcp requested times 
 			 dhcp response times
 			 dhcp leases in the file /var/run/apache2/dhcp_*
 * INPUT:
 *		uint32 - profilee
 *		uint32 - detect
 *		
 * OUTPUT:
 *		uint32 - return code
 *				DHCP_SERVER_RETURN_CODE_SUCCESS  -set success
 * RETURN:
 *		NULL - get args failed
 *		reply - set success
 *		
 *****************************************************/
DBusMessage *dhcrelay_dbus_set_interface_relay
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage *reply = NULL;
	DBusMessageIter iter;
	DBusError		err;
	char *upifname = NULL, *downifname = NULL;
	unsigned int ret = 0, ipaddr = 0, add_info = 0, check_ret = 0;
	
	dbus_error_init( &err );

	if( !(dbus_message_get_args( msg ,&err,
					DBUS_TYPE_STRING, &downifname,
					DBUS_TYPE_STRING, &upifname,
					DBUS_TYPE_UINT32, &ipaddr,
					DBUS_TYPE_UINT32, &add_info,
					DBUS_TYPE_INVALID))) {
		if(dbus_error_is_set( &err )) {
			dbus_error_free( &err );
		}
		return NULL;
	}
	if (dhcrelay_dbus_check_ipaddr_uint(ipaddr)) {
		ret = DHCP_SERVER_RETURN_CODE_FAIL;
		goto out;
	}
	
	check_ret = dhcrelay_dbus_check_relay_node(downifname, upifname, ipaddr);
	if (add_info) {	
		log_debug("add ip relay check_ret is %d\n", check_ret);
		if (!(1&check_ret)) {
			ret = discover_interfaces_test(DISCOVER_RELAY, downifname);
			if (!ret) {
				if (!(2&check_ret)) {
					log_debug("add new upifname \n");
					ret = discover_interfaces_to_server(DISCOVER_RELAY, upifname);
				}
				if (!ret) {
					add_server_addr_ipv4(ipaddr);
					dhcrelay_dbus_add_relay_node(downifname, upifname, ipaddr);
					log_debug("add relay node is ok server ip is %x , downifname %s, upifname %s\n", 
								ipaddr, downifname, upifname);
				}
				else {
					log_info("free the new downifname beause set upifname fail\n");
					del_relay_interface(downifname);
				}
			}
			else {
				
			}
		}
		else {
			log_info("the relay down interface %s is in used \n", downifname);
			ret = DHCP_INTERFACE_IN_USR;
		}
	}
	else {
		ret = dhcrelay_dbus_del_relay_node(downifname, upifname, ipaddr);
		log_debug("delete ip relay ret is %d \n", ret);
		if (1&ret) {
			del_relay_interface(downifname);
			if (!(4&ret)) {
				del_relay_interface(upifname);	
			}
			if (!(2&ret)) {
				del_server_addr_ipv4(ipaddr);
			}
			ret = 0;
		}
		else {
			ret = DHCP_SERVER_RETURN_CODE_FAIL;
			log_error("delete ip relay node fail \n");
		}
	}
	
	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		return reply;
	}
out:	
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &ret);
	
	return reply;
}

DBusMessage * 
dhcp_dbus_set_relay_enable
(	
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
	unsigned int enable = 0;
	unsigned int op_ret = 0;
	DBusError err;

	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_UINT32, &enable,
		DBUS_TYPE_INVALID))) {
		 log_error("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 log_error("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	dhcp_server_enable = enable;
	log_debug("globle dhcp server is %s \n", (dhcp_server_enable)?"enalbe":"disable");

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, 
									 &op_ret);


	return reply;	
}
int parse_int_ve(char* str, unsigned int* slotid, unsigned int *vlanid, char *cid, unsigned int *port){
	char c;
	char *tmp = NULL;
	char *endptr = NULL;
	c = str[0];
	if (c>='0'&&c<='9'){
		*slotid= strtoul(str,&endptr,10);
		
		if(endptr[0] == '.'){
			tmp = endptr+1;
			*vlanid= strtoul(tmp,&endptr,10);//regard the ve interface name as right
			if((endptr[0] != '\0')&&(endptr[0] != '\n'))
				return -1;
			return 1;
		}
		else if((endptr[0] == 'f')||(endptr[0] == 's')){
			*cid = endptr[0];
			tmp = endptr+1;
			*port = strtoul(tmp,&endptr,10);
			if(endptr[0] == '.'){
				tmp = endptr+1;
				*vlanid= strtoul(tmp,&endptr,10);
				if((endptr[0] != '\0')&&(endptr[0] != '\n'))
					return -1;
				return 2;
			}
			else if((endptr[0] == '\0')||(endptr[0] == '\n'))
				return 2;
			
			return -1;
			}
	}
	
		return -1;
}
	

int check_ve_interface(char *ifname ,char *name){
	int sockfd;
	unsigned int slotid = 0;
	unsigned int vlanid = 0;
	unsigned int port = 0;
	char cpu = 'f';
	//char *cpu_id = &cpu;
	struct ifreq ifr;
	int ret = 0;
	
	if (0 != strncmp(ifname, "ve", 2)) {
		log_debug("It's not ve interface\n");
		sprintf(name,"%s",ifname);
		return 0;
	}
	else{
		sockfd = socket(AF_INET, SOCK_DGRAM, 0);
		if(sockfd<0){
			log_debug("sockfd failed!\n");
			return -1;
		}
		strncpy(ifr.ifr_name,ifname, sizeof(ifr.ifr_name)); 	
		ret = parse_int_ve(ifname+2,&slotid,&vlanid,&cpu,&port);
		
		if(1 == ret)
		{
			log_debug("slotid = %d\n",slotid);
			log_debug("vlanid = %d\n",vlanid);

			if (ioctl(sockfd, SIOCGIFINDEX, &ifr) == -1){//bind to a interface 
				log_debug("SIOCGIFINDEX error\n");

				//convert to new ve name
				if(slotid < 10)
					sprintf(name,"ve0%df1.%d",slotid,vlanid);
				else if(slotid >= 10)
					sprintf(name,"ve%df1.%d",slotid,vlanid);
				log_debug("ve name is %s\n",name);

				memset(ifr.ifr_name, 0, sizeof(ifr.ifr_name));
				strncpy(ifr.ifr_name,name, sizeof(ifr.ifr_name));		
				if (ioctl(sockfd, SIOCGIFINDEX, &ifr) == -1){//bind to a interface 
					log_debug("SIOCGIFINDEX error\n");		
					close(sockfd);
					return -1;	//the new ve interface doesn't exist
				}	
				else{
					close(sockfd);
				    return 0;	//the new ve interface exists
				}
			}
			else{
				sprintf(name,"%s",ifname);
				close(sockfd);
				return 0;//the old ve interface exists
			}
		}
		else if(2 == ret)
		{
			log_debug("slotid = %d\n",slotid);
			log_debug("vlanid=%d\n",vlanid);
			log_debug("ifname=%s\n",ifname);
			if(vlanid == 0){
				if(slotid < 10)
					sprintf(name,"ve0%d%c%d",slotid,cpu,port);
				else if(slotid >= 10)
					sprintf(name,"ve%d%c%d",slotid,cpu,port);
			}
			else if(vlanid > 0){
				if(slotid < 10)
					sprintf(name,"ve0%d%c%d.%d",slotid,cpu,port,vlanid);
				else if(slotid >= 10)
					sprintf(name,"ve%d%c%d.%d",slotid,cpu,port,vlanid);
			}
			
			log_debug("name=%s\n",name);
			memset(ifr.ifr_name, 0, sizeof(ifr.ifr_name));
			strncpy(ifr.ifr_name,name, sizeof(ifr.ifr_name));
			if (ioctl(sockfd, SIOCGIFINDEX, &ifr) == -1){//bind to a interface 
				log_debug("SIOCGIFINDEX error\n");
				close(sockfd);
				return -1;	//the new ve interface doesn't exist
			}		
			else{
				//sprintf(name,"%s",ifname);
			    close(sockfd);
			    return 0;	//the new ve interface exists
		 	}
		}
		else{
			log_debug("the ve name is wrong\n");
			sprintf(name,"%s",ifname);
			close(sockfd);
			return -1;
		}
	}
}


DBusMessage *
dhcp_dbus_check_relay_interface_ve
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage *reply = NULL;
	DBusMessageIter iter;
	DBusError		err;
	unsigned int	ret = 1;
	char *ifname = NULL;
	char *name=NULL;

	dbus_error_init( &err );

	if( !(dbus_message_get_args( msg ,&err,
					DBUS_TYPE_STRING, &ifname,
					DBUS_TYPE_INVALID))) {
		if(dbus_error_is_set( &err )) {
			dbus_error_free( &err );
		}
		return NULL;
	}
	name=malloc(ALIAS_NAME_SIZE);
	if(!name){
		log_error("application of name fails ");
		return NULL;	
	}
    memset(name,0,ALIAS_NAME_SIZE);
    ret = check_ve_interface(ifname,name);
	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		if(name){
			free(name);
			name=NULL;
	}
		return reply;
	}
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_STRING, &name);
	if(name){
		free(name);
		name=NULL;
	}
	return reply;
}

DBusMessage *
dhcp_dbus_check_relay_interface
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage *reply = NULL;
	DBusMessageIter iter;
	DBusError		err;
	unsigned int	ret = 1;
	unsigned int detect = 0;
	char *ifname = NULL;

	dbus_error_init( &err );

	if( !(dbus_message_get_args( msg ,&err,
		            DBUS_TYPE_STRING, &ifname,
					DBUS_TYPE_UINT32,&detect,
					DBUS_TYPE_INVALID))) {
		if(dbus_error_is_set( &err )) {
			dbus_error_free( &err );
		}
		return NULL;
	}
	
	ret = dhcrelay_dbus_check_relay_interface(ifname);
	
	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		return reply;
	}
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &ret);
	
	return reply;
}

DBusMessage*
dhcp_dbus_set_debug_relay_state
(	
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
	unsigned int debug_type = 0;
	unsigned int enable = 0;
	unsigned int op_ret = 0;
	DBusError err;

	dbus_error_init(&err);

	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_UINT32, &debug_type,
		DBUS_TYPE_UINT32, &enable,
		DBUS_TYPE_INVALID))) {
		 log_error("while set_debug_state,unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 log_error("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}		

	if(debug_type == DEBUG_TYPE_ALL){
        log_debug("dhcp debug_type is %s \n", "all");
	}
	else if(debug_type == DEBUG_TYPE_INFO){
        log_debug("dhcp debug_type is %s \n", "info");
	}
	else if(debug_type == DEBUG_TYPE_ERROR){
        log_debug("dhcp debug_type is %s \n", "error");
	}
	else if(debug_type == DEBUG_TYPE_DEBUG){
        log_debug("dhcp debug_type is %s \n", "debug");
	}

	if(enable){
		dhcp_log_level |= debug_type;
	}else{
		dhcp_log_level &= ~debug_type;
	}
	
	log_debug("globle dhcp relay dhcp_log_level is %d \n", dhcp_log_level);

	reply = dbus_message_new_method_return(msg);

	dbus_message_iter_init_append (reply, &iter);

	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &op_ret);

	return reply;
}


DBusMessage* 
dhcrelay_dbus_show_relay_cfg
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
) 
{
	DBusMessage* reply = NULL;
	DBusMessageIter  iter,iter_array;	
	DBusError err;
	unsigned int i = 0, ret = 0, count = 0 ,value = 0, isenable = 0;
	struct dcli_relay *node = NULL;
	char *upifname = NULL, *downifname = NULL;
	
	dbus_error_init(&err);	
	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_UINT32,&value,
		DBUS_TYPE_INVALID))) {
		log_error("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			log_error("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	/* Total count*/
	count = dhcrelay_count;
	node = dhcrelay_head.next;
	isenable = dhcp_server_enable;
	log_debug("dhcrelay_dbus_show_interface_relay count is %d \n", count);	
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, 
									&ret);
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32, 
									&isenable);
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32, 
									&count);
	
	if (!ret) {
		if (count > 0) {
			dbus_message_iter_open_container (&iter,
												DBUS_TYPE_ARRAY,
												DBUS_STRUCT_BEGIN_CHAR_AS_STRING
												DBUS_TYPE_UINT32_AS_STRING   /* ip addr*/														
												DBUS_TYPE_STRING_AS_STRING    /*upifname*/
												DBUS_TYPE_STRING_AS_STRING	/*downifname*/
												DBUS_STRUCT_END_CHAR_AS_STRING,
												&iter_array);
						
			for (i = 0; i < count; i++ ) {
				DBusMessageIter iter_struct;
				dbus_message_iter_open_container (&iter_array,
												   DBUS_TYPE_STRUCT,
												   NULL,
												   &iter_struct);
				
								
				dbus_message_iter_append_basic
						(&iter_struct,
						  DBUS_TYPE_UINT32,
						  &(node->ipv4addr));  /* ip addr*/
				
				upifname = node->upifname;
				dbus_message_iter_append_basic
							(&iter_struct,
							  DBUS_TYPE_STRING,
							  &(upifname));/*upifname*/
				
				downifname = node->downifname;
				dbus_message_iter_append_basic
							(&iter_struct,
							  DBUS_TYPE_STRING,
							  &(downifname));/*downifname*/
				
				dbus_message_iter_close_container (&iter_array, &iter_struct);
				node = node->next;
			}	
			dbus_message_iter_close_container (&iter, &iter_array);
		}	
	}
	
	return reply;
}

DBusMessage* 
dhcrelay_dbus_show_running_cfg
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage*		reply;	  
	DBusMessageIter 	iter= {0};
	DBusError			err;   		
	char *showStr = NULL, *cursor = NULL;	
	char *showif = NULL, *ifcursor = NULL;
	struct dcli_relay *node = NULL;
	int totalLen = 0, iflen = 0;

	showStr = (char*)malloc(128*1024);
	memset(showStr, 0, 128*1024);
	
	showif = (char*)malloc(100*1024);
	memset(showif, 0, 100*1024);
	
	ifcursor = showif;
	cursor = showStr;

	dbus_error_init(&err);
	
	totalLen += sprintf(cursor, "\n!%s section.\n\n", "DHCP RELAY");
	cursor = showStr + totalLen;

	/*dhcp server enable*/
	if (dhcp_server_enable) {
		totalLen += sprintf(cursor, "ip dhcp relay enable\n");
		cursor = showStr + totalLen;
	}
	
	node = dhcrelay_head.next;
	if (node) {
		dhcp_dbus_config_interface_save(&showif, &ifcursor, node, &iflen);
	}

	totalLen += sprintf(cursor, showif);
	cursor = showStr + totalLen;

	log_debug("dhcrelay show run: %s\n", showStr);
		
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);	
	dbus_message_iter_append_basic (&iter,
								   DBUS_TYPE_STRING,
								   &showStr);

	free(showStr);
	showStr = NULL;
	free(showif);
	showif = NULL;
	
	return reply;
}


DBusMessage* 
dhcrelay_dbus_show_running_hansi_cfg
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage *reply = NULL;	  
	DBusMessageIter iter;
	DBusError err;   		
	char *strShow = NULL;
	unsigned int slot_id = 0;
	unsigned int vrrp_id = 0;
	unsigned int local_flag = 0;

	dbus_error_init(&err);

	if(!(dbus_message_get_args( msg ,&err,
					DBUS_TYPE_UINT32, &slot_id,
					DBUS_TYPE_UINT32, &vrrp_id,	
					DBUS_TYPE_UINT32, &local_flag,					
					DBUS_TYPE_INVALID))) {
		if(dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}

	strShow = (char *)malloc(DHCRELAY_SAVE_CFG_MEM);	
	if(!strShow) {
		log_debug("%s:%d malloc memory failed\n", MDL);
		return NULL;
	}
	memset(strShow, 0, DHCRELAY_SAVE_CFG_MEM);

	dbus_error_init(&err);

	dhcrelay_dbus_hansi_config(strShow, slot_id, vrrp_id, local_flag);

	log_debug("%s\n", __func__);
	log_debug("%s\n", strShow);	

	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);	
	dbus_message_iter_append_basic (&iter,
								   DBUS_TYPE_STRING,
								   &strShow);

	free(strShow);
	strShow = NULL;
	return reply;
}



/* init dhcrelay dbus */
static DBusHandlerResult dhcrelay_dbus_message_handler 
(
	DBusConnection *connection, 
	DBusMessage *message, 
	void *user_data
)
{
	DBusMessage		*reply = NULL;

	if (dbus_message_is_method_call(message, DHCRELAY_DBUS_INTERFACE, DHCRELAY_DBUS_METHOD_SET_DHCP_RELAY)) {
		reply = dhcrelay_dbus_set_interface_relay(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCRELAY_DBUS_INTERFACE, DHCRELAY_DBUS_METHOD_SHOW_DHCP_RELAY)) {
		reply = dhcrelay_dbus_show_relay_cfg(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCRELAY_DBUS_INTERFACE, DHCRELAY_DBUS_METHOD_SHOW_RUNNING_CFG)) {
		reply = dhcrelay_dbus_show_running_cfg(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCRELAY_DBUS_INTERFACE, DHCRELAY_DBUS_METHOD_SHOW_RUNNING_HANSI_CFG)) {
		reply = dhcrelay_dbus_show_running_hansi_cfg(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCRELAY_DBUS_INTERFACE, DHCRELAY_DBUS_METHOD_SET_RELAY_ENABLE)) {
		reply = dhcp_dbus_set_relay_enable(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCRELAY_DBUS_INTERFACE, DHCRELAY_DBUS_METHOD_CHECK_RELAY_INTERFACE)) {
		reply = dhcp_dbus_check_relay_interface(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCRELAY_DBUS_INTERFACE, DHCRELAY_DBUS_METHOD_CHECK_RELAY_INTERFACE_VE)) {
		reply = dhcp_dbus_check_relay_interface_ve(connection, message, user_data);
	}
	if (dbus_message_is_method_call(message, DHCRELAY_DBUS_INTERFACE, DHCP_DBUS_METHOD_SET_DEBUG_RELAY_STATE)) {
		reply = dhcp_dbus_set_debug_relay_state(connection, message, user_data);
	}

	if (reply) {
		dbus_connection_send (connection, reply, NULL);
		dbus_connection_flush(connection); /* TODO Maybe we should let main loop process the flush*/
		dbus_message_unref (reply);
	}

	return DBUS_HANDLER_RESULT_HANDLED;
}

DBusHandlerResult dhcrelay_dbus_filter_function 
(
	DBusConnection * connection,
	DBusMessage * message, 
	void *user_data
)
{
	if (dbus_message_is_signal (message, DBUS_INTERFACE_LOCAL, "Disconnected") &&
		   strcmp (dbus_message_get_path (message), DBUS_PATH_LOCAL) == 0) {

		/* this is a local message; e.g. from libdbus in this process */
		dbus_connection_unref (dhcrelay_dbus_connection);
		dhcrelay_dbus_connection = NULL;

		/*g_timeout_add (3000, reinit_dbus, NULL);*/

	} 
	else if (dbus_message_is_signal (message,
			      DBUS_INTERFACE_DBUS,
			      "NameOwnerChanged")) {

		/*if (services_with_locks != NULL)  service_deleted (message);*/
	}
	else {
		return 1;
	}
		/*return hald_dbus_filter_handle_methods (connection, message, user_data, FALSE);*/

	return DBUS_HANDLER_RESULT_HANDLED;
}

void dhcrelay_tell_whoami
(
	char * myName,
	int isLast
)
{
	char pidBuf[DHCRELAY_PID_BUFFER_LEN] = {0}, pidPath[DHCRELAY_PID_BUFFER_LEN] = {0};
	pid_t myPid = 0;
	
	if(!myName) {
		return;
	}

	sprintf(pidPath,"%s%s%d%s", DHCRELAY_PID_FILE_PATH, DHCRELAY_PID_FILE_PREFIX, \
				global_current_instance_no, DHCRELAY_PID_FILE_SUFFIX);
		
	if(DHCRELAY_FD_INIT == g_dhcrelay_pid_fd) {	
		g_dhcrelay_pid_fd = open(pidPath, O_RDWR|O_CREAT);
		if(DHCRELAY_FD_INIT == g_dhcrelay_pid_fd) {
			return;
		}
	}

	myPid = getpid();
	
	sprintf(pidBuf,"instance %d %s has pid %d\n", global_current_instance_no, myName, myPid);
	write(g_dhcrelay_pid_fd, pidBuf, strlen(pidBuf));

	/* close pid file by last teller */
	if(isLast) {
		close(g_dhcrelay_pid_fd);
		g_dhcrelay_pid_fd = DHCRELAY_FD_INIT;
	}

	return;
}

int dhcrelay_dbus_init(void)
{
	DBusError dbus_error;
	DBusObjectPathVTable	dhcrelay_vtable = {NULL, &dhcrelay_dbus_message_handler, NULL, NULL, NULL, NULL};
	
	dbus_connection_set_change_sigpipe (1);

	dbus_error_init (&dbus_error);
	dhcrelay_dbus_connection = dbus_bus_get (DBUS_BUS_SYSTEM, &dbus_error);
	if (dhcrelay_dbus_connection == NULL) {
		log_error ("dbus_relay_get(): %s", dbus_error.message);
		return 1;
	}

	/* Use dhcp to handle subsection of DHCP_DBUS_OBJPATH including slots*/
	if (!dbus_connection_register_fallback (dhcrelay_dbus_connection, DHCRELAY_DBUS_OBJPATH, &dhcrelay_vtable, NULL)) {
		log_error("can't register D-BUS handlers (fallback DHCP). cannot continue.");
		return 1;	
	}
		
	dbus_bus_request_name (dhcrelay_dbus_connection, DHCRELAY_DBUS_BUSNAME,
			       0, &dbus_error);
		
	if (dbus_error_is_set (&dbus_error)) {
		log_error ("dbus_bus_request_name(): %s",
			    dbus_error.message);
		return 1;
	}

	dbus_connection_add_filter (dhcrelay_dbus_connection, dhcrelay_dbus_filter_function, NULL, NULL);

	dbus_bus_add_match (dhcrelay_dbus_connection,
			    		"type='signal'"
					    ",interface='"DBUS_INTERFACE_DBUS"'"
					    ",sender='"DBUS_SERVICE_DBUS"'"
					    ",member='NameOwnerChanged'",
			    NULL);

	return 0;
}

void * dhcrelay_dbus_thread_main(void *arg)
{

	dhcrelay_tell_whoami("dbusDhcrelay", 0);

	/* tell about my initialization process done
	npd_init_tell_stage_end();
*/	
	/*
	For all OAM method call, synchronous is necessary.
	Only signal/event could be asynchronous, it could be sent in other thread.
	*/	
	while (dbus_connection_read_write_dispatch(dhcrelay_dbus_connection,-1)) {
		;
	}
	
	return NULL;
}

void dhcrelay_dbus_start(void)
{
	int ret = 0;
	dhcrelay_dbus_init();
	
	dbus_dhcrelay_thread = (pthread_t *)malloc(sizeof(pthread_t));
	pthread_attr_init(&dbus_dhcrelay_thread_attr);
	ret = pthread_create(dbus_dhcrelay_thread, &dbus_dhcrelay_thread_attr, dhcrelay_dbus_thread_main, NULL);
    if (0 != ret) {
	   log_error ("start dhcrelay dbus pthread fail\n");
	}
	
}
#ifdef __cplusplus
}
#endif


