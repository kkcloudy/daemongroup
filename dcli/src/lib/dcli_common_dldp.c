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
* dcli_common_dldp.c
*
*
* CREATOR:
* 		jinpc@autelan.com
*
* DESCRIPTION:
* 		dcli to handle dldp orders comunicate with dldp module and npd.
*
* DATE:
*		6/1/2009
*
* FILE REVISION NUMBER:
*  		$Revision: 1.8 $
*
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif
/*********************************************************
*	head files														*
**********************************************************/
#include "dcli_common_dldp.h"
#include "sysdef/returncode.h"


/*********************************************************
*	global variable define											*
**********************************************************/


/*********************************************************
*	extern variable												*
**********************************************************/


/*********************************************************
*	functions define												*
**********************************************************/

/**********************************************************************************
 * dcli_dldp_check_global_status()
 *	DESCRIPTION:
 *		check  dldp enable/disable global status 
 *
 *	INPUTS:
 *		NULL
 *
 *	OUTPUTS:
 *		unsigned char* stats		- check result
 *
 *	RETURN VALUE:
 *		DLDP_RETURN_CODE_OK	- success
 *		DLDP_RETURN_CODE_ERROR	- fail
***********************************************************************************/
int dcli_dldp_check_global_status
(
	unsigned char* stats,
	unsigned char* devchk
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned char status = DCLI_DLDP_INIT_0, devok = 0;
	int  ret = DCLI_DLDP_RETURN_CODE_OK;

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,
								NPD_DBUS_DLDP_OBJPATH,
								NPD_DBUS_DLDP_INTERFACE,
								NPD_DBUS_DLDP_METHOD_CHECK_GLOBAL_STATUS);
	
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args(reply, &err,
								DBUS_TYPE_UINT32, &ret,
								DBUS_TYPE_BYTE, &status,
								DBUS_TYPE_BYTE, &devok,
								DBUS_TYPE_INVALID)) 
	{
		if (DCLI_DLDP_RETURN_CODE_OK == ret) {
			*stats = status;
			*devchk = devok;
		}else{
			ret = DCLI_DLDP_RETURN_CODE_ERROR;
		}
	} else {
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);

	return ret;
}

/**********************************************************************************
 * dcli_dldp_enable_disable_one_vlan()
 *	DESCRIPTION:
 *		enable/disable DLDP on vlan and its ports by vlanid, base vlan
 *
 *	INPUTS:
 *		unsigned short vlanid
 *		unsigned char enable
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		DCLI_NPD_DLDP_RETURN_CODE_OK				- success
 *		DCLI_NPD_DLDP_RETURN_CODE_ERROR			- fail
 * 		DCLI_NPD_DLDP_RETURN_CODE_VLAN_NOT_EXIST	- L2 vlan not exist
 *		DCLI_NPD_DLDP_RETURN_CODE_NOT_ENABLE_GBL	- not enable DLDP global
 *		DCLI_NPD_DLDP_RETURN_CODE_NOTENABLE_VLAN	- L2 vlan not enable DLDP
 *		DCLI_NPD_DLDP_RETURN_CODE_HASENABLE_VLAN	- L2 vlan has enabled DLDP
***********************************************************************************/
unsigned int dcli_dldp_enable_disable_one_vlan
(
	unsigned short vlanid,
	unsigned char enable
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned char isEnable = DCLI_DLDP_INIT_0;
	unsigned int ret = DCLI_DLDP_RETURN_CODE_OK;

	isEnable = enable;

	query = dbus_message_new_method_call(
										NPD_DBUS_BUSNAME,
										NPD_DBUS_DLDP_OBJPATH,
										NPD_DBUS_DLDP_INTERFACE,
										NPD_DBUS_DLDP_METHOD_CONFIG_VLAN);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT16, &vlanid,
							 DBUS_TYPE_BYTE, &isEnable,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args(reply, &err,
								DBUS_TYPE_UINT32, &ret,
								DBUS_TYPE_INVALID))
	{
		dbus_message_unref(reply);
		return ret;
	}else {
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
	}

	return CMD_SUCCESS;
}

/**********************************************************************************
 * dcli_dldp_get_vlan_count()
 *	DESCRIPTION:
 *		get count of vlan which enable DLDP, base vlan
 *
 *	INPUTS:
 *		struct vty	 *vty
 *
 *	OUTPUTS:
 *		unsigned short *vlan_count
 *
 *	RETURN VALUE:
 *		DCLI_NPD_DLDP_RETURN_CODE_OK				- success
 *		DCLI_NPD_DLDP_RETURN_CODE_ERROR			- fail
 *		DCLI_DLDP_RETURN_CODE_NOT_ENABLE_GBL		- not enable DLDP global
 ***********************************************************************************/
unsigned int dcli_dldp_get_vlan_count
(
	struct vty	 *vty,
	unsigned short *vlan_count
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int ret = DCLI_DLDP_RETURN_CODE_OK;
	unsigned int count = DCLI_DLDP_INIT_0;

	query = dbus_message_new_method_call(
										NPD_DBUS_BUSNAME,
										NPD_DBUS_DLDP_OBJPATH,
										NPD_DBUS_DLDP_INTERFACE,
										NPD_DBUS_DLDP_METHOD_GET_VLAN_COUNT);
	dbus_error_init(&err);
	
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "Failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return DCLI_DLDP_RETURN_CODE_ERROR;
	}

	if (dbus_message_get_args(reply, &err,
							  DBUS_TYPE_UINT32, &ret,
							  DBUS_TYPE_UINT32, &count,
							  DBUS_TYPE_INVALID))
	{
		if ((DCLI_DLDP_RETURN_CODE_OK == ret) &&
			(DCLI_DLDP_INIT_0 != count)) {
			*vlan_count = count;
			ret = DCLI_DLDP_RETURN_CODE_OK;
		}
		else if ((DCLI_DLDP_RETURN_CODE_OK == ret) &&
				(DCLI_DLDP_INIT_0 == count)) {
			vty_out(vty, "%% Error:DLDP not enabled any vlan!\n");
			ret = DCLI_DLDP_RETURN_CODE_ERROR;
		}
		else if (DCLI_DLDP_RETURN_CODE_NOT_ENABLE_GBL == ret){
			vty_out(vty, "%% Error:DLDP not enabled global!\n");
			ret = DCLI_DLDP_RETURN_CODE_ERROR;
		}else {
			ret = DCLI_DLDP_RETURN_CODE_ERROR;
		}
	}else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		ret = DCLI_DLDP_RETURN_CODE_ERROR;
	}
	dbus_message_unref(reply);

	return ret;
}

/**********************************************************************************
 * dcli_dldp_get_product_id()
 *	DESCRIPTION:
 *		get product_id
 *
 *	INPUTS:
 *		struct vty	 *vty
 *
 *	OUTPUTS:
 *		unsigned int *product_id
 *
 *	RETURN VALUE:
 *		DCLI_NPD_DLDP_RETURN_CODE_OK				- success
 *		DCLI_NPD_DLDP_RETURN_CODE_ERROR			- fail
 *		DCLI_DLDP_RETURN_CODE_NOT_ENABLE_GBL		- not enable DLDP global
 ***********************************************************************************/
unsigned int dcli_dldp_get_product_id
(
	struct vty	 *vty,
	unsigned int *product_id
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int ret = DCLI_DLDP_RETURN_CODE_OK;
	unsigned int productid = DCLI_DLDP_INIT_0;

	query = dbus_message_new_method_call(
										NPD_DBUS_BUSNAME,
										NPD_DBUS_DLDP_OBJPATH,
										NPD_DBUS_DLDP_INTERFACE,
										NPD_DBUS_DLDP_METHOD_GET_PRODUCT_ID);

	dbus_error_init(&err);
	
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "Failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return DCLI_DLDP_RETURN_CODE_ERROR;
	}

	if (dbus_message_get_args(reply, &err,
							  DBUS_TYPE_UINT32, &ret,
							  DBUS_TYPE_UINT32, &productid,
							  DBUS_TYPE_INVALID))
	{
		if (DCLI_DLDP_RETURN_CODE_OK == ret) {
			*product_id = productid;
			ret = DCLI_DLDP_RETURN_CODE_OK;
		}else {
			ret = DCLI_DLDP_RETURN_CODE_ERROR;
		}
	}else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		ret = DCLI_DLDP_RETURN_CODE_ERROR;
	}

	dbus_message_unref(reply);

	return ret;
}

/*************************************************************************** 
 * function : show dldp vlan <1-4095> status
 *	name : dcli_dldp_exchange_ifindex_to_slotno_portno
 *	input :
 *		struct vty  	*vty
 *		unsigned int   ifindex		-count of ifindex array  which need to exchange
 *	 	long			*routeArray	-ifindex array which need to exchange
 *
 *	output :
 *		unsigned char *slot_no
 *		unsigned char *port_no
 *
 *	re_value: ret
 *			CMD_WARNING	-dbus err
 *			CMD_SUCCESS	-success
 **************************************************************************/
unsigned int dcli_dldp_exchange_ifindex_to_slotno_portno
(
	struct vty	  *vty,
	unsigned int  ifindex,
	unsigned char *slot_no,
	unsigned char *port_no
)
{
	DBusMessage		*query;
	DBusMessage		*reply;
	DBusError		err;
	DBusMessageIter	iter;
	DBusMessageIter	iter_array;

	unsigned int ret = DCLI_DLDP_RETURN_CODE_OK;
	unsigned char slotno = DCLI_DLDP_INIT_0;
	unsigned char portno = DCLI_DLDP_INIT_0;

	query = dbus_message_new_method_call(
										NPD_DBUS_BUSNAME,
										NPD_DBUS_DLDP_OBJPATH,
										NPD_DBUS_DLDP_INTERFACE,
										NPD_DBUS_DLDP_METHOD_EXCHANGE_IFINDEX_TO_SLOTPORT);

	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &ifindex,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply)
	{
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return DCLI_DLDP_RETURN_CODE_ERROR;
	}

	if (dbus_message_get_args(reply, &err,
								DBUS_TYPE_UINT32, &ret,
								DBUS_TYPE_BYTE, &slotno,
								DBUS_TYPE_BYTE, &portno,
								DBUS_TYPE_INVALID))
	{
		*slot_no = slotno;
		*port_no = portno;
		dbus_message_unref(reply);
		ret = DCLI_DLDP_RETURN_CODE_OK;
	}
	else
	{
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err))
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		ret = DCLI_DLDP_RETURN_CODE_ERROR;
	}

	return ret;
}

/**********************************************************************************
 *dcli_dldp_vlan_show_running_config()
 *
 *	DESCRIPTION:
 *		show DLDP vlan running config
 *
 *	INPUTS:
 *		NULL
 *	
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		NULL
 *
 ***********************************************************************************/
int dcli_dldp_vlan_show_running_config
(
	void
)
{
	unsigned char *showStr = NULL;
	int ret = 0;
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;
	
	query = dbus_message_new_method_call(
										NPD_DBUS_BUSNAME,
										NPD_DBUS_DLDP_OBJPATH,
										NPD_DBUS_DLDP_INTERFACE,
										NPD_DBUS_DLDP_METHOD_VLAN_SHOW_RUNNING_CONFIG);

	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("show dldp_vlan running config failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return ret;
	}

	if (dbus_message_get_args(reply, &err,
							DBUS_TYPE_STRING, &showStr,
							DBUS_TYPE_INVALID)) 
	{
	
		char _tmpstr[64];
		memset(_tmpstr,0,64);
		sprintf(_tmpstr,BUILDING_MOUDLE,"DLDP VLAN");
		vtysh_add_show_string(_tmpstr);
		vtysh_add_show_string(showStr);
		ret = 1;
	} 
	else 
	{
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	dbus_message_unref(reply);

	return ret;
}

/**********************************************************************************
 *dcli_dldp_time_show_running_config()
 *
 *	DESCRIPTION:
 *		show DLDP time running config
 *
 *	INPUTS:
 *		NULL
 *	
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		NULL
 *
 ***********************************************************************************/
int dcli_dldp_time_show_running_config
(
	void
)
{
	unsigned char *showStr = NULL;
	int ret = 0;
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;
	
	query = dbus_message_new_method_call(
										DLDP_DBUS_BUSNAME,
										DLDP_DBUS_OBJPATH,
										DLDP_DBUS_INTERFACE,
										DLDP_DBUS_METHOD_DLDP_TIME_PARAMETER_GET);

	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("show dldp_time running config failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return ret;
	}

	if (dbus_message_get_args(reply, &err,
							DBUS_TYPE_STRING, &showStr,
							DBUS_TYPE_INVALID)) 
	{
	
		char _tmpstr[64];
		memset(_tmpstr,0,64);
		sprintf(_tmpstr,BUILDING_MOUDLE,"DLDP TIME");
		vtysh_add_show_string(_tmpstr);
		vtysh_add_show_string(showStr);
		ret = 1;
	} 
	else 
	{
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	dbus_message_unref(reply);

	return ret;
}

/**********************************************************************************
 *  dcli_get_product_support_function
 *
 *	DESCRIPTION:
 * 		get broad type
 *
 *	INPUT:
 *		NONE		
 *	
 * RETURN:
 *		0   -   disable
 *		1   -   enable
 *
 **********************************************************************************/

int dcli_dldp_get_product_support_function(struct vty* vty,unsigned char *IsSupport)
{
	DBusMessage *query, *reply;
	DBusError err; 
	int op_ret = 0;
	unsigned char is_support = 0;

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,			\
								NPD_DBUS_DLDP_OBJPATH,		\
								NPD_DBUS_DLDP_INTERFACE,		\
								DLDP_DBUS_METHOD_GET_SUPPORT_FUNCTION);

	
	dbus_error_init(&err);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_BYTE,&is_support,
		DBUS_TYPE_INVALID)) {
		*IsSupport = is_support;

	} 
	
	else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

#ifdef __cplusplus
}
#endif

