/* cgicTempDir is the only setting you are likely to need
	to change in this file. */

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
* ws_pppoe_snp.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* qiaojie@autelan.com
*
* DESCRIPTION:
*
*
*
*******************************************************************************/


#ifdef __cplusplus
extern "C"
{
#endif

#include "ws_pppoe_snp.h"
#include "dbus/dhcp/dhcp_dbus_def.h"
#include "ws_dbus_list.h"
#include "ac_manage_def.h"

/*state为"enable"或"disable"*/
int config_pppoe_snp_server_cmd(char *state)/*返回0表示失败，返回1表示成功*/
													/*返回-1表示bad command parameter*/
													/*返回-2表示error*/
{
	if(NULL == state)
		return 0;
	
	unsigned int ret = 0, isEnable = 0;
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;

	if(!strncmp("enable",state,strlen(state))) {
		isEnable = 1;
	} else if (!strncmp("disable",state,strlen(state))) {
		isEnable = 0;
	} else {
		return -1;
	}

	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_PPPOE_SNP_ENABLE);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &isEnable,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
				
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return 0;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &ret,
					DBUS_TYPE_INVALID)) {
		if(!ret) {
			dbus_message_unref(reply);
			return 1;
		}
	} 
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
	}

	
	if (!ret) {		
		return 1;
	} else {
		return -2;
	}
}

static int ve_ifname_transfer_conversion(char *new_ifname, const char *original_ifname)
{
	int slotNum, vlanNum;
	char tmpStr[32];

	memcpy(tmpStr, original_ifname, strlen(original_ifname));

	if (!strncmp(tmpStr, "ve", 2)) {
		if (tmpStr[4] == 'f' || tmpStr[4] == 's') {
			memcpy(new_ifname, original_ifname, strlen(original_ifname));
			return 0;
		}
		else {
			sscanf(original_ifname,"ve%d.%d", &slotNum, &vlanNum);
			sprintf(new_ifname, "ve%02df%d.%d", slotNum, 1, vlanNum);
			return 0;
		}
	}
	else {
		//vty_out(vty, "[%s] is not a ve-interface name.\n", original_ifname);
		return -1;
	}

	return 0;
}

/*state为"enable"或"disable"*/
int config_pppoe_snooping_enable_cmd(char *if_name,char *state)/*返回0表示失败，返回1表示成功*/
																		/*返回-1表示bad command parameter*/
																		/*返回-2表示Interface name is too long*/
																		/*返回-3表示if_name is not a ve-interface name*/
																		/*返回-4表示get local_slot_id error*/
																		/*返回-5表示error*/
{
	if((NULL == if_name) && (NULL == state))
		return 0;
	
	int ret = 0;
	unsigned short mru = 0;
	unsigned int enable_flag = 0;
	char ifname[IFNAMESIZE];
	char tmp_ifname[IFNAMESIZE];
	//char *ptr = NULL;
	int slotid = 0, vrrpid = 0, num = 0, local_flag = 0;
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;

	if(!strncmp("enable", state, strlen(state))) {
		enable_flag = 1;
	} else if (!strncmp("disable", state, strlen(state))) {
		enable_flag = 0;
	} else {
		return -1;
	}

	if(strlen(if_name) > IFNAMESIZE){
		return -2;
	}

	memset(ifname, 0, sizeof(ifname));
	memcpy(ifname, if_name, strlen(if_name));

	memset(tmp_ifname, 0, sizeof(tmp_ifname));
	if(-1 == ve_ifname_transfer_conversion(tmp_ifname, ifname))
	{
		return -3;
	}
	memcpy(ifname, tmp_ifname, strlen(tmp_ifname));

	/************dcli_pppoe_snp_convert_ifname*************/
	if(!ifname) {
		return 0;
	}

	//if (CONFIG_NODE == vty->node) {
		if ((slotid = get_product_info(PRODUCT_LOCAL_SLOTID)) < 0) {
			return -4;
		}
		vrrpid = 0;
		local_flag = 1;
	//}

	/* hansi node or config node */
	if (!local_flag) {
		if (0 == strncmp(ifname, "wlan", 4)) {
			sscanf(ifname, "wlan%d", &num);	
			memset(ifname, 0, strlen(ifname));
			sprintf(ifname, "wlan%d-%d-%d", slotid, vrrpid, num);
		} else if (0 == strncmp(ifname, "ebr", 3)) {
			sscanf(ifname, "ebr%d", &num);	
			memset(ifname, 0, strlen(ifname));	
			sprintf(ifname, "ebr%d-%d-%d", slotid, vrrpid, num);
		} else {
			sprintf(ifname, "%s", ifname);
		}

	/* local-hansi node */
	} else {
		if (!strncmp(ifname, "wlan", 4)) {
			sscanf(ifname, "wlan%d", &num);	
			memset(ifname, 0, strlen(ifname));
			sprintf(ifname, "wlanl%d-%d-%d", slotid, vrrpid, num);
		} else if (!strncmp(ifname, "ebr", 3)) {
			sscanf(ifname, "ebr%d", &num);			
			memset(ifname, 0, strlen(ifname));
			sprintf(ifname, "ebrl%d-%d-%d", slotid, vrrpid, num);
		} else {
			sprintf(ifname, "%s", ifname);
		}
	}

	#if 0
	if (argc == 3) {
		mru = (unsigned short)strtol((char*)argv[2], &ptr, 10); 
		if (*ptr) {
			vty_out (vty, "%% Invalid MRU %s\n", (char*)argv[2]);
			return CMD_WARNING;
		}

		if ((mru < MRU_MIN) || (mru > MRU_MAX)) {
			vty_out (vty, "%% MRU range: %d - %d\n", MRU_MIN, MRU_MAX);
			return CMD_WARNING;
		}
	}
	#endif
	
	/************dcli_pppoe_snp_iface_enable*************/
	char *temp_ifname = (char *)malloc(sizeof(ifname)+1);
	if(NULL == temp_ifname)
		return 0;

	memset(temp_ifname, 0, sizeof(temp_ifname)+1);
	strcpy(temp_ifname, ifname);
	
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_PPPOE_SNP_IFACE_ENABLE);

	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_STRING, &temp_ifname,
							DBUS_TYPE_UINT32, &enable_flag,
							DBUS_TYPE_UINT16, &mru,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection, query, -1, &err);

	FREE_OBJECT(temp_ifname);

	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return 0;
	}

	if (dbus_message_get_args (reply, &err,
					DBUS_TYPE_UINT32, &ret,
					DBUS_TYPE_INVALID)) {
		dbus_message_unref(reply);
	} 
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
	}	

	if (!ret) {
		return 1;
	} else {	
		return -5;
	}
}


#ifdef __cplusplus
}
#endif

