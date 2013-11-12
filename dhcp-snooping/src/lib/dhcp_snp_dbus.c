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
* dhcp_snp_dbus.c
*
* CREATOR:
*		qinhs@autelan.com
*
* DESCRIPTION:
*		dbus message main routine for DHCP snooping module.
*
* DATE:
*		04/16/2010	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.1.1.1 $	
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif
#include <string.h>
#include <sysdef/returncode.h>
#include <dbus/dhcp/dhcpsnp_dbus_def.h>

#include "dhcp_snp_log.h"
#include "dhcp_snp_com.h"
#include "dhcp_snp_dbus.h"

#define DHCPSNP_RD_DEBUG	1
extern pthread_mutex_t mutexDhcpsnptbl;

DBusConnection *dhcp_snp_dbus_connection = NULL;

/**********************************************************************************
 * dhcp_snp_dbus_message_handler
 *	DHCP snooping dbus message handler
 *
 *	INPUT:
 *		connection - D-BUS connection
 *		message - dbus message
 *		user_data - user data along within dbus message
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		dbus reply message
 **********************************************************************************/
static DBusHandlerResult dhcp_snp_dbus_message_handler 
(
	DBusConnection *connection, 
	DBusMessage *message, 
	void *user_data
)
{
	DBusMessage 	*reply = NULL;

	#if DHCPSNP_RD_DEBUG
	syslog_ax_dhcp_snp_dbg("dhcp snoop dbus handler!\n");
	#endif
 
	if(strcmp(dbus_message_get_path(message), DHCPSNP_DBUS_OBJPATH) == 0) {	/* DHCP_Snooping */
		#if DHCPSNP_RD_DEBUG
		syslog_ax_dhcp_snp_dbg("dbus message objpath is %s!\n", DHCPSNP_DBUS_OBJPATH);
		#endif
		if (dbus_message_is_method_call(message,
										DHCPSNP_DBUS_INTERFACE,
										DHCPSNP_DBUS_METHOD_CHECK_GLOBAL_STATUS))
		{
			reply = dhcp_snp_dbus_check_global_status(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,
										DHCPSNP_DBUS_INTERFACE,
										DHCPSNP_DBUS_METHOD_GLOBAL_ENABLE))
		{
			reply = dhcp_snp_dbus_enable_global_status(connection, message, user_data);
		}
		#if 0
		else if (dbus_message_is_method_call(message,
										DHCPSNP_DBUS_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_VLAN_ENABLE))
		{
			reply = dhcp_snp_dbus_enable_vlan_status(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,
										DHCPSNP_DBUS_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_SET_ETHPORT_TRUST_MODE))
		{
			reply = dhcp_snp_dbus_config_port(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,
										DHCPSNP_DBUS_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_SHOW_BIND_TABLE))
		{
			reply = dhcp_snp_dbus_show_bind_table(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,
										DHCPSNP_DBUS_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_SHOW_STATIC_BIND_TABLE))
		{
			reply = dhcp_snp_dbus_show_static_bind_table(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,
										DHCPSNP_DBUS_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_SHOW_STATIC_BIND_TABLE_BY_VLAN))
		{
			reply = dhcp_snp_dbus_show_static_bind_table_by_vlan(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,
										DHCPSNP_DBUS_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_SHOW_STATIC_BIND_TABLE_BY_ETHPORT))
		{
			reply = dhcp_snp_dbus_show_static_bind_table_by_ethport(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,
										DHCPSNP_DBUS_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_SHOW_TRUST_PORTS))
		{
			reply = dhcp_snp_dbus_show_trust_ports(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,
										DHCPSNP_DBUS_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_OPT82_ENABLE))
		{
			reply = dhcp_snp_dbus_enable_opt82(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,
										DHCPSNP_DBUS_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_SET_OPT82_FORMAT_TYPE))
		{
			reply = dhcp_snp_dbus_set_opt82_format_type(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,
										DHCPSNP_DBUS_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_SET_OPT82_FILL_FORMAT_TYPE))
		{
			reply = dhcp_snp_dbus_set_opt82_fill_format_type(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,
										DHCPSNP_DBUS_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_SET_OPT82_REMOTEID_CONTENT))
		{
			reply = dhcp_snp_dbus_set_opt82_remoteid_content(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,
										DHCPSNP_DBUS_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_SET_OPT82_PORT_STRATEGY))
		{
			reply = dhcp_snp_dbus_set_opt82_port_strategy(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,
										DHCPSNP_DBUS_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_SET_OPT82_PORT_CIRCUITID_CONTENT))
		{
			reply = dhcp_snp_dbus_set_opt82_port_circuitid_content(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,
										DHCPSNP_DBUS_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_SET_OPT82_PORT_REMOTEID_CONTENT))
		{
			reply = dhcp_snp_dbus_set_opt82_port_remoteid_content(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,
										DHCPSNP_DBUS_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_ADD_DEL_STATIC_BINDING))
		{
			reply = dhcp_snp_dbus_add_del_static_binding(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,
										DHCPSNP_DBUS_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_ADD_BINDING))
		{
			reply = dhcp_snp_dbus_add_binding(connection, message, user_data);
		}
		#endif
		else if (dbus_message_is_method_call(message,
										DHCPSNP_DBUS_INTERFACE,
										DHCPSNP_DBUS_METHOD_SHOW_RUNNING_GLOBAL_CONFIG))
		{
			reply = dhcp_snp_dbus_show_running_global_config(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,
										DHCPSNP_DBUS_INTERFACE,
										DHCPSNP_DBUS_METHOD_SHOW_RUNNING_GLOBAL_HANSI_CONFIG))
		{
			reply = dhcp_snp_dbus_show_running_global_hansi_config(connection, message, user_data);
		}
		
		#if 0
		else if (dbus_message_is_method_call(message,
										DHCPSNP_DBUS_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_SHOW_RUNNING_VLAN_CONFIG))
		{
			reply = dhcp_snp_dbus_show_running_vlan_config(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,
										DHCPSNP_DBUS_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_SHOW_RUNNING_SAVE_BIND_TABLE))
		{
			reply = dhcp_snp_dbus_show_running_save_bind_table(connection, message, user_data);
		}
		#endif
		else if(dbus_message_is_method_call(message,
										DHCPSNP_DBUS_INTERFACE,
										DHCPSNP_DBUS_METHOD_INTERFACE_ENABLE)) 
		{
			reply = dhcp_snp_dbus_config_intf(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message,
										DHCPSNP_DBUS_INTERFACE,
										DHCPSNP_DBUS_METHOD_INTERFACE_ARP_ENABLE)) 
		{
			reply = dhcp_snp_dbus_config_intf_arp(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message,
										DHCPSNP_DBUS_INTERFACE,
										DHCPSNP_DBUS_METHOD_INTERFACE_ADD_ROUTER_ENABLE)) 
		{
			reply = dhcp_snp_dbus_config_intf_add_router(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message,
										DHCPSNP_DBUS_INTERFACE,
										DHCPSNP_DBUS_METHOD_CHECK_SNP_INTERFACE_VE)) 
		{
			reply = dhcp_snp_dbus_check_snp_interface_ve(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message,
										DHCPSNP_DBUS_INTERFACE,
										DHCPSNP_DBUS_METHOD_DEBUG_STATE)) 
		{
			reply = dhcp_snp_dbus_config_debug_level(connection, message, user_data);
		}
		else if(dbus_message_is_method_call(message,
										DHCPSNP_DBUS_INTERFACE,
										DHCPSNP_DBUS_METHOD_INTERFACE_ONOFF_STA_STATIC_ARP)) 
		{
			reply = dhcp_snp_dbus_config_ipneigh(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,
										DHCPSNP_DBUS_INTERFACE,
										DHCPSNP_DBUS_METHOD_SHOW_WAN_BIND_TABLE))
		{
			reply = dhcp_snp_dbus_show_wan_bind_table(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,
										DHCPSNP_DBUS_INTERFACE,
										DHCPSNP_DBUS_METHOD_SHOW_WAN_IPV6_BIND_TABLE))
		{
			reply = dhcp_snp_dbus_show_wan_ipv6_bind_table(connection, message, user_data);
		}
		else if (dbus_message_is_method_call(message,
										DHCPSNP_DBUS_INTERFACE,
										DHCPSNP_DBUS_METHOD_DELETE_WAN_TABLE))
		{
			pthread_mutex_lock(&mutexDhcpsnptbl);
			reply = dhcp_snp_dbus_del_wan_bind_table(connection, message, user_data);
			pthread_mutex_unlock(&mutexDhcpsnptbl);			
		}
		else if (dbus_message_is_method_call(message,
										DHCPSNP_DBUS_INTERFACE,
										DHCPSNP_DBUS_METHOD_DELETE_HOST_ROUTER))
		{
			//pthread_mutex_lock(&mutexDhcpsnptbl);
			reply = dhcp_snp_dbus_del_host_router(connection, message, user_data);
			//pthread_mutex_unlock(&mutexDhcpsnptbl);			
		}
		else if (dbus_message_is_method_call(message,
										DHCPSNP_DBUS_INTERFACE,
										DHCPSNP_DBUS_METHOD_ADD_WAN_TABLE))
		{
			pthread_mutex_lock(&mutexDhcpsnptbl);
			reply = dhcp_snp_dbus_add_wan_bind_table(connection, message, user_data);
			pthread_mutex_unlock(&mutexDhcpsnptbl);			
		}
		#if 0
		else if (dbus_message_is_method_call(message,
										DHCPSNP_DBUS_INTERFACE,
										DHCPSNP_DBUS_METHOD_CONFIG_AGING_TIME))
		{
			reply = dhcp_snp_dbus_config_ageing_time(connection, message, user_data);
		}
		#endif
		else if (dbus_message_is_method_call(message,
										DHCPSNP_DBUS_INTERFACE,
										DHCPSNP_DBUS_METHOD_ENABLE_ARP_PROXY))
		{
			reply = dhcp_snp_dbus_enable_arp_proxy(connection, message, user_data);
		}
		
	}
	
	if (reply) {
		dbus_connection_send (connection, reply, NULL);
		dbus_connection_flush(connection); /* TODO	  Maybe we should let main loop process the flush*/
		dbus_message_unref (reply);
	}

/*	dbus_message_unref(message); //TODO who should unref the incoming message? */
	return DBUS_HANDLER_RESULT_HANDLED ;
}


/**********************************************************************************
 * dhcp_snp_dbus_filter_function
 *	DHCP snooping dbus message filter
 *
 *	INPUT:
 *		connection - dbus connection
 *		message - dbus message
 *		user_data - user data along within dbus message
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		dbus reply message
 **********************************************************************************/
DBusHandlerResult dhcp_snp_dbus_filter_function 
(
	DBusConnection * connection,
	DBusMessage * message, 
	void *user_data
)
{
	if (dbus_message_is_signal (message, DBUS_INTERFACE_LOCAL, "Disconnected") &&
		   strcmp (dbus_message_get_path (message), DBUS_PATH_LOCAL) == 0) {

		/* this is a local message; e.g. from libdbus in this process */

		syslog_ax_dbus_dbg ("Got disconnected from the system message bus; "
				"retrying to reconnect every 3000 ms");
		dbus_connection_unref (dhcp_snp_dbus_connection);
		dhcp_snp_dbus_connection = NULL;

		/*g_timeout_add (3000, reinit_dbus, NULL);*/

	} else if (dbus_message_is_signal (message,
			      DBUS_INTERFACE_DBUS,
			      "NameOwnerChanged")) {
		;
	} else
		return TRUE;

	return DBUS_HANDLER_RESULT_HANDLED;
}

/**********************************************************************************
 * dhcp_snp_dbus_init
 *	Initialize DHCP snooping dbus connection
 *
 *	INPUT:
 *		NULL
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		TRUE		- success
 *	 	FALSE		- fail
 **********************************************************************************/
int dhcp_snp_dbus_init(void)
{
	DBusError dbus_error;
	DBusObjectPathVTable	dhcp_snp_vtable = {NULL, &dhcp_snp_dbus_message_handler, NULL, NULL, NULL, NULL};
	
	dbus_connection_set_change_sigpipe (TRUE);

	dbus_error_init (&dbus_error);
	dhcp_snp_dbus_connection = dbus_bus_get (DBUS_BUS_SYSTEM, &dbus_error);
	if (dhcp_snp_dbus_connection == NULL) {
		syslog_ax_dbus_err ("dhcp snooping init dbus dbus_bus_get(): %s", dbus_error.message);
		return FALSE;
	}

	/* Use npd to handle subsection of NPD_DBUS_OBJPATH including slots*/
	if (!dbus_connection_register_fallback (dhcp_snp_dbus_connection, DHCPSNP_DBUS_OBJPATH, &dhcp_snp_vtable, NULL)) {
		syslog_ax_dbus_err("can't register D-BUS handlers (fallback dhcpsnp). cannot continue.\n");
		return FALSE;
		
	}
		
	dbus_bus_request_name (dhcp_snp_dbus_connection, DHCPSNP_DBUS_BUSNAME,
			       0, &dbus_error);
	
	
	if (dbus_error_is_set (&dbus_error)) {
		syslog_ax_dbus_err ("dhcp snooping dbus request bus name error: %s\n",
			    dbus_error.message);
		return FALSE;
	}

	dbus_connection_add_filter (dhcp_snp_dbus_connection, dhcp_snp_dbus_filter_function, NULL, NULL);

	dbus_bus_add_match (dhcp_snp_dbus_connection,
			    "type='signal'"
			    ",interface='"DBUS_INTERFACE_DBUS"'"
			    ",sender='"DBUS_SERVICE_DBUS"'"
			    ",member='NameOwnerChanged'",
			    NULL);
	return TRUE;
}

/**********************************************************************************
 * dhcp_snp_dbus_thread_main
 *	Main routine for dbus message handle thread
 *
 *	INPUT:
 *		NULL
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		NULL		
 **********************************************************************************/
void * dhcp_snp_dbus_thread_main
(
	void *arg
)
{
	syslog_ax_dbus_dbg("dhcp snooping dbus handler thread start!\n");

	/* tell my thread pid*/
	dhcp_snp_tell_whoami("dhcpsnp dbus", 0);
	/*
	For all OAM method call, synchronous is necessary.
	Only signal/event could be asynchronous, it could be sent in other thread.
	*/	
	while (dbus_connection_read_write_dispatch(dhcp_snp_dbus_connection,-1)) {
		;
	}
	return NULL;
}

#ifdef __cplusplus
}
#endif
