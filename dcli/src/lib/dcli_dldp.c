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
* dcli_dldp.c
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
*  		$Revision: 1.11 $
*
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif
/*********************************************************
*	head files														*
**********************************************************/
#include "dcli_dldp.h"
#include "sysdef/returncode.h"


/*********************************************************
*	global variable define											*
**********************************************************/

/*********************************************************
*	extern variable												*
**********************************************************/
extern DBusConnection *dcli_dbus_connection;


/*********************************************************
*	functions define												*
**********************************************************/

/**********************************************************************************
 *dcli_dldp_parse_timeout()
 *
 *	DESCRIPTION:
 *		string to unsigned long
 *
 *	INPUTS:
 *		char* str					-time, is string type
 *	
 *	OUTPUTS:
 *		unsigned int* timeout		- time, is unsigned int type

 *
 *	RETURN VALUE:
 *		DLDP_RETURN_CODE_OK	- success
 *		DLDP_RETURN_CODE_ERROR	- error
 *
 ***********************************************************************************/
int dcli_dldp_parse_timeout
(
	char* str,
	unsigned int* timeout
)
{
	char *endptr = NULL;
	char c = DCLI_DLDP_INIT_0;
	
	if (NULL == str || NULL == timeout) {
		return DCLI_DLDP_RETURN_CODE_ERROR;
	}
	
	c = str[0];
	if (c >= '0' && c<='9') {
		*timeout= strtoul(str, &endptr, 10);
		if ('\0' != endptr[0]) {
			/* timeout value be end by invalid char. */
			return DCLI_DLDP_RETURN_CODE_ERROR;
		}

		return DCLI_DLDP_RETURN_CODE_OK;	
	}else {
		/*not timeout; for Example ,enter '.',and so on ,some special char.	*/
		return DCLI_DLDP_RETURN_CODE_ERROR;
	}
}

/**********************************************************************************
 *dldp_show_dldp_timer()
 *
 *	DESCRIPTION:
 *		show DLDP timer
 *
 *	INPUTS:
 *		struct vty  *vty
 *		unsigned int detection_interval			- detection interval
 *		unsigned int redetection_interval			- redetection interval
 *	
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		NULL
 *
 ***********************************************************************************/
void dldp_show_dldp_timer
(
	struct vty  *vty,
	unsigned int detection_interval,
	unsigned int redetection_interval
)
{
	vty_out(vty, "\n");
	vty_out(vty, "==================  ====================\r\n");
	vty_out(vty, "DETECTION-INTERVAL  REDETECTION-INTERVAL\n");
	vty_out(vty, "%-18d  %-20d", detection_interval, redetection_interval);
	vty_out(vty,"\n==================  ====================\r\n");

	return ;
}


/**********************************************************************************
 *dcli_dldp_show_running_config()
 *
 *	DESCRIPTION:
 *		show DLDP running config
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
int dcli_dldp_show_running_config
(
struct vty * vty
)
{
	char _tmpstr[64];
	memset(_tmpstr,0,64);
	sprintf(_tmpstr,BUILDING_MOUDLE,"DLDP");
	vtysh_add_show_string(_tmpstr);

	dcli_dldp_vlan_show_running_config();
	dcli_dldp_time_show_running_config();

	return 0;
}

/**************************************************
 *	command: debug  dldp (all|error|warning|debug|event)
 *	
 *	cmd Node: config Node
 *
***************************************************/
DEFUN(config_dldp_debug_cmd_func,
	config_dldp_debug_cmd,
	"debug dldp (all|error|warning|debug|event)",
	DCLI_DEBUG_STR
	MODULE_DEBUG_STR(dldp)
	MODULE_DEBUG_LEVEL_STR(dldp, all)
	MODULE_DEBUG_LEVEL_STR(dldp, error)
	MODULE_DEBUG_LEVEL_STR(dldp, warning)
	MODULE_DEBUG_LEVEL_STR(dldp, debug)
	MODULE_DEBUG_LEVEL_STR(dldp, event)
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	int ret = DCLI_DLDP_INIT_0;
	unsigned int flag = DCLI_DLDP_INIT_0;
	unsigned char is_support = 0;
	
	if(argc > 1) {
		vty_out(vty, "%% Command parameters number error!\n");
		return CMD_WARNING;
	}
	
	dcli_dldp_get_product_support_function(vty,&is_support);
	if(1 == is_support){
		vty_out(vty,"%% Product not support this function!\n");
		return CMD_WARNING;
	}
	
	if(0 == strncmp(argv[0], "all", strlen(argv[0]))) {
		flag = DCLI_DEBUG_FLAG_ALL;
	}		
	else if(0 == strncmp(argv[0],"error", strlen(argv[0]))) {
		flag = DCLI_DEBUG_FLAG_ERR;
	}
	else if (0 == strncmp(argv[0],"warning", strlen(argv[0]))) {
		flag = DCLI_DEBUG_FLAG_WAR;
	}
	else if (0 == strncmp(argv[0],"debug", strlen(argv[0]))) {
		flag = DCLI_DEBUG_FLAG_DBG;
	}
	else if (0 == strncmp(argv[0],"event", strlen(argv[0]))) {
		flag = DCLI_DEBUG_FLAG_EVT;
	}
	else {
		vty_out(vty, "%% Command parameter %s error!\n", argv[0]);
		return CMD_WARNING;
	}

	query = dbus_message_new_method_call(
										DLDP_DBUS_BUSNAME,
										DLDP_DBUS_OBJPATH,
										DLDP_DBUS_INTERFACE,
										DLDP_DBUS_METHOD_DLDP_DEBUG_ON);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &flag,
							DBUS_TYPE_INVALID);


	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args(reply, &err,
								DBUS_TYPE_UINT32, &ret,
								DBUS_TYPE_INVALID)) {
		if (DCLI_DLDP_RETURN_CODE_NOT_ENABLE_GBL == ret) {
			vty_out(vty,"%% Error:DLDP not enabled global!\n");
		}
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

/**************************************************
 *	command: no dldp debug (all|error|warning|debug|event)
 *	
 *	cmd Node: config Node
 *
***************************************************/
DEFUN(config_dldp_no_debug_cmd_func,
	config_dldp_no_debug_cmd,
	"no debug dldp (all|error|warning|debug|event)",
	"Disable specific function \n"
	NODEBUG_STR
	DCLI_DEBUG_STR
	MODULE_DEBUG_STR(dldp)
	MODULE_DEBUG_LEVEL_STR(dldp, all)
	MODULE_DEBUG_LEVEL_STR(dldp, error)
	MODULE_DEBUG_LEVEL_STR(dldp, warning)
	MODULE_DEBUG_LEVEL_STR(dldp, debug)
	MODULE_DEBUG_LEVEL_STR(dldp, event)
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	int ret = DCLI_DLDP_INIT_0;
	unsigned int flag = DCLI_DLDP_INIT_0;
	unsigned char is_support = 0;
	
	if (argc > 1) {
		vty_out(vty, "%% Command parameters number error!\n");
		return CMD_WARNING;
	}

	dcli_dldp_get_product_support_function(vty,&is_support);
	if(1 == is_support){
		vty_out(vty,"%% Product not support this function!\n");
		return CMD_WARNING;
	}
	
	if (0 == strncmp(argv[0], "all", strlen(argv[0]))) {
		flag = DCLI_DEBUG_FLAG_ALL;
	}		
	else if (0 == strncmp(argv[0], "error", strlen(argv[0]))) {
		flag = DCLI_DEBUG_FLAG_ERR;
	}
	else if (0 == strncmp(argv[0], "warning", strlen(argv[0]))) {
	    flag = DCLI_DEBUG_FLAG_WAR;
	}
	else if (0 == strncmp(argv[0], "debug", strlen(argv[0]))) {
	    flag = DCLI_DEBUG_FLAG_DBG;
	}
	else if (0 == strncmp(argv[0], "event", strlen(argv[0]))) {
	    flag = DCLI_DEBUG_FLAG_EVT;
	}
	else {
		vty_out(vty,"%% Command parameter %s error!\n", argv[0]);
		return CMD_WARNING;
	}
	
	query = dbus_message_new_method_call(
										DLDP_DBUS_BUSNAME,
										DLDP_DBUS_OBJPATH,
										DLDP_DBUS_INTERFACE,
										DLDP_DBUS_METHOD_DLDP_DEBUG_OFF);
	dbus_error_init(&err);
    dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &flag,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args(reply, &err,
								DBUS_TYPE_UINT32, &ret,
								DBUS_TYPE_INVALID))
	{
		if (DCLI_DLDP_RETURN_CODE_NOT_ENABLE_GBL == ret) {
			vty_out(vty,"%% Error:DLDP not enabled global!\n");
		}
	}else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

/**************************************************
 *	command: debug dldp packet (all|receive|send)
 *	
 *	cmd Node: config Node
 *
***************************************************/
DEFUN(debug_dldp_pkt_info,
	config_dldp_debug_pkt_info_cmd,
	"debug dldp packet (all|receive|send)",
	DCLI_DEBUG_STR
	MODULE_DEBUG_STR(dldp)
	MODULE_DEBUG_STR(packet)
	MODULE_DEBUG_LEVEL_STR(packet, all)
	MODULE_DEBUG_LEVEL_STR(packet, receive)
	MODULE_DEBUG_LEVEL_STR(packet, send)
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int ret = DCLI_DLDP_INIT_0;
	unsigned int flag = DCLI_DLDP_INIT_0;
	unsigned char is_support = 0;
	
	if (argc > 1) {
		vty_out(vty, "%% Command parameters number error!\n");
		return CMD_WARNING;
	}
	
	dcli_dldp_get_product_support_function(vty,&is_support);
	if(1 == is_support){
		vty_out(vty,"%% Product not support this function!\n");
		return CMD_WARNING;
	}
	
	if (0 == strncmp(argv[0], "all", strlen(argv[0])))	{
		flag = DCLI_DEBUG_FLAG_PKT_ALL;
	}else if (0 == strncmp(argv[0], "receive", strlen(argv[0]))) {
		flag = DCLI_DEBUG_FLAG_PKT_REV;
	}else if (0 == strncmp(argv[0], "send", strlen(argv[0]))) {
		flag = DCLI_DEBUG_FLAG_PKT_SED;
	}else {
		vty_out(vty,"%% Command parameter %s error!\n", argv[0]);
		return CMD_WARNING;
	}

	query = dbus_message_new_method_call(
										DLDP_DBUS_BUSNAME,
										DLDP_DBUS_OBJPATH,
										DLDP_DBUS_INTERFACE,
										DLDP_DBUS_METHOD_DLDP_DEBUG_ON);
    dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &flag,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply)
	{
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			vty_out(vty,"%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	if (dbus_message_get_args(reply, &err,
								DBUS_TYPE_UINT32, &ret,
								DBUS_TYPE_INVALID))
	{
		if (DCLI_DLDP_RETURN_CODE_NOT_ENABLE_GBL == ret) {
			vty_out(vty,"%% Error:DLDP not enabled global!\n");
		}
	}else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

/**************************************************
 *	command: no debug dldp packet (all|receive|send)
 *	
 *	cmd Node: config Node
 *
***************************************************/
DEFUN(no_debug_dldp_pkt_info,
	config_dldp_no_debug_pkt_info_cmd,
	"no debug dldp packet (all|receive|send)",
	NODEBUG_STR
	DCLI_DEBUG_STR
	MODULE_DEBUG_STR(dldp)
	MODULE_DEBUG_STR(packet)
	MODULE_DEBUG_LEVEL_STR(packet, all)
	MODULE_DEBUG_LEVEL_STR(packet, receive)
	MODULE_DEBUG_LEVEL_STR(packet, send)
)
{	
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int ret  = DCLI_DLDP_INIT_0;
	unsigned int flag  = DCLI_DLDP_INIT_0;
	unsigned char is_support = 0;
	
	if (argc > 1) {
		vty_out(vty,"%% Command parameters number error!\n");
		return CMD_WARNING;
	}

	dcli_dldp_get_product_support_function(vty,&is_support);
	if(1 == is_support){
		vty_out(vty,"%% Product not support this function!\n");
		return CMD_WARNING;
	}
	
	if (0 == strncmp(argv[0], "all", strlen(argv[0]))) {
		flag = DCLI_DEBUG_FLAG_PKT_ALL;
	}else if (0 == strncmp(argv[0], "receive", strlen(argv[0]))) {
		flag = DCLI_DEBUG_FLAG_PKT_REV;
	}else if (0 == strncmp(argv[0], "send", strlen(argv[0]))) {
		flag = DCLI_DEBUG_FLAG_PKT_SED;
	}else {
		vty_out(vty,"%% Command parameter %s error!\n", argv[0]);
		return CMD_WARNING;
	}

	query = dbus_message_new_method_call(
										DLDP_DBUS_BUSNAME,
										DLDP_DBUS_OBJPATH,
										DLDP_DBUS_INTERFACE,
										DLDP_DBUS_METHOD_DLDP_DEBUG_OFF);	
    dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &flag,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply)
	{
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args(reply, &err,
								DBUS_TYPE_UINT32, &ret,
								DBUS_TYPE_INVALID))
	{
		if (DCLI_DLDP_RETURN_CODE_NOT_ENABLE_GBL == ret) {
			vty_out(vty,"%% Error:DLDP not enabled global!\n");
		}
	}else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
/**************************************************
 *	command: config dldp (enable|disable)
 *
 *	note      : complete ONLY by dldp Protocol
 *
 *	cmd Node: config Node
 *
***************************************************/
DEFUN(config_dldp_enable_cmd_func,
	config_dldp_enable_cmd,
	"config dldp (enable|disable)",
	CONFIG_STR
	DCLI_DLDP_STR
	"Enable DLDP functionality\n"
	"Disable DLDP functionality\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned char status = DCLI_DLDP_INIT_0, devchk = 0;
	unsigned char isEnable = DCLI_DLDP_INIT_0;
	int ret = DCLI_DLDP_INIT_0;
	unsigned char is_support = 0;
	
	if (argc > 1) {
		vty_out(vty,"%% Command without so many parameters!\n");
		return CMD_WARNING;
	}
	
	dcli_dldp_get_product_support_function(vty,&is_support);
	if(1 == is_support){
		vty_out(vty,"%% Product not support this function!\n");
		return CMD_WARNING;
	}
	
	if (strncmp(argv[0], "enable", strlen(argv[0])) == 0) {
		isEnable = 1;
	}
	else if (strncmp(argv[0], "disable", strlen(argv[0])) == 0) {
		isEnable = 0;
	}else {
		vty_out(vty,"%% Bad parameter %s!\n", argv[0]);
		return CMD_WARNING;
	}

	ret = dcli_dldp_check_global_status(&status, &devchk);
	if (DCLI_DLDP_RETURN_CODE_OK == ret) {
		if(TURE_5612E == devchk){
			vty_out(vty, "%% Error device 5612e not support DLDP!\n");
			return CMD_WARNING;
		}
		
		if(isEnable == status) {
			vty_out(vty, "%% Error DLDP already %s!\n",
					isEnable ? "enabled" : "disabled");
			return CMD_WARNING;
		}
	}else {
		vty_out(vty, "%% Check DLDP state error %x!\n", ret);
		return CMD_SUCCESS;
	}

	query = dbus_message_new_method_call(
										DLDP_DBUS_BUSNAME,
										DLDP_DBUS_OBJPATH,
										DLDP_DBUS_INTERFACE,
										DLDP_DBUS_METHOD_DLDP_ENABLE);
	dbus_error_init(&err);
    dbus_message_append_args(query,
							DBUS_TYPE_BYTE, &isEnable,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID)) {
		if (DCLI_DLDP_RETURN_CODE_OK != ret) {
			vty_out(vty, "%% %s DLDP error %x!", isEnable ? "Enable":"Disable", ret);
			return CMD_WARNING;
		}
	} else {
		vty_out(vty, "Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;
}

/*********************************************************
 *	command: config dldp (detection-interval|re-detection-interval) TIMEOUT
 *
 *	cmd Node: config Node
 *
**********************************************************/
DEFUN(config_dldp_timer_cmd_func,
	config_dldp_timer_cmd,
	"config dldp (detection-interval|re-detection-interval) TIMEOUT",
	CONFIG_STR
	DCLI_DLDP_STR
	"Config DLDP detection interval: valid range [2-30]\n"
	"Config DLDP re-detection interval: valid range [60-180]\n"
	"Timeout value\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;

	unsigned int timer_type = DCLI_DLDP_TYPE_INVALID;
	unsigned int timeout = DCLI_DLDP_INIT_0;
	unsigned int ret = DCLI_DLDP_RETURN_CODE_OK;
	unsigned char is_support = 0;

	dcli_dldp_get_product_support_function(vty,&is_support);
	if(1 == is_support){
		vty_out(vty,"%% Product not support this function!\n");
		return CMD_WARNING;
	}
	
	if (!strncmp(argv[0], "detection-interval", strlen(argv[0]))) {
		timer_type = DCLI_DLDP_TYPE_DETECTION;
	}else if (!strncmp(argv[0], "re-detection-interval", strlen(argv[0]))) {
		timer_type = DCLI_DLDP_TYPE_REDETECTION;
	}else {
		vty_out(vty, "%% Unknown command format!\n");
		return CMD_WARNING;
	}

	ret = dcli_dldp_parse_timeout((char *)argv[1], &timeout);
	if (NPD_FAIL == ret) {
    	vty_out(vty,"%% Error:malformed TIMEOUT value!\n");
		return CMD_WARNING;
	}
	
	switch(timer_type){
		default:
			break;
		case DCLI_DLDP_TYPE_DETECTION:
			if(timeout > 30 || timeout <2){
	    		vty_out(vty,"%% Valid detection-interval value must be in range [2-30] second\n");
				return CMD_WARNING;
			}
			break;
		case DCLI_DLDP_TYPE_REDETECTION:
			if(timeout > 180 || timeout <60){
	    		vty_out(vty,"%% Valid re-detection-interval value must be in range [60-180] second\n");
				return CMD_WARNING;
			}
			break;
	}

	query = dbus_message_new_method_call(
										DLDP_DBUS_BUSNAME,
										DLDP_DBUS_OBJPATH,
										DLDP_DBUS_INTERFACE,
										DLDP_DBUS_METHOD_DLDP_SET_TIMER);

	dbus_error_init(&err);
    dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &timer_type,
							DBUS_TYPE_UINT32, &timeout,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args(reply, &err,
								DBUS_TYPE_UINT32, &ret,
								DBUS_TYPE_INVALID))
	{
		if (DCLI_DLDP_RETURN_CODE_NOT_ENABLE_GBL == ret){
			vty_out(vty, "%% Error: DLDP not enabled global!\n");
		}else if (DCLI_DLDP_RETURN_CODE_OUT_RANGE == ret) {
			vty_out(vty, "%% Error: parameter value %d invalid!\n", timeout);
		}else if (DCLI_DLDP_RETURN_CODE_SAME_VALUE == ret) {
			vty_out(vty, "%% Error: value %d is same as the original, or has configured!\n", timeout);
		}else if (DCLI_DLDP_RETURN_CODE_ERROR == ret) {
			vty_out(vty, "%% Error: failed set timer value %d!\n", timeout);
		}
	}else {
		vty_out(vty, "Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

/*********************************************************
 *	command: show dldp time-interval
 *		 including:	
 * 				detection-interval
 *				re-detection-interval
 *
 *	cmd Node: config Node
 *
**********************************************************/
DEFUN(show_dldp_timers_cmd_func,
	show_dldp_timers_cmd,
	"show dldp time-interval",
	SHOW_STR
	DCLI_DLDP_STR
	"Show DLDP time interval configuration\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int detection_interval = DCLI_DLDP_INIT_0;
	unsigned int redetection_interval = DCLI_DLDP_INIT_0;
	unsigned int ret = DCLI_DLDP_RETURN_CODE_OK;
	unsigned char is_support = 0;

	dcli_dldp_get_product_support_function(vty,&is_support);
	if(1 == is_support){
		vty_out(vty,"%% Product not support this function!\n");
		return CMD_WARNING;
	}
	
	query = dbus_message_new_method_call(
										DLDP_DBUS_BUSNAME,
										DLDP_DBUS_OBJPATH,
										DLDP_DBUS_INTERFACE,
										DLDP_DBUS_METHOD_DLDP_SHOW_TIMER);
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
	dbus_message_unref(query);

	if (NULL == reply) {
		vty_out(vty, "Failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args(reply, &err,
								DBUS_TYPE_UINT32, &ret,
								DBUS_TYPE_UINT32, &detection_interval,
								DBUS_TYPE_UINT32, &redetection_interval,
								DBUS_TYPE_INVALID))
	{
		if (DCLI_DLDP_RETURN_CODE_OK == ret) {
			dldp_show_dldp_timer(vty, detection_interval, redetection_interval);
		}else if (DCLI_DLDP_RETURN_CODE_NOT_ENABLE_GBL == ret) {
			vty_out(vty, "%% Error:DLDP not enabled global!\n");
		}
		else if (DCLI_DLDP_RETURN_CODE_GET_DETECT_E == ret) {
			vty_out(vty, "%% Get detection-interval error!\n");
		}
		else if (DCLI_DLDP_RETURN_CODE_GET_REDETECT_E == ret) {
			vty_out(vty, "%% Get re-detection-interval error!\n");
		}
	}
	else {
		vty_out(vty, "Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	dbus_message_unref(reply);
	return CMD_SUCCESS;
}


#if 0
// base port
/*********************************************************
 *	command: config dldp PORTNO (enable|disable)
 *		enable|disable port to(from)  DLDP, add port to npd first, and then 
 *		pass param to DLDP Protocol
 *
 *	cmd Node: config Node
 *
**********************************************************/
DEFUN(config_dldp_npd_port_cmd_func,
	config_dldp_npd_port_cmd,
	"config dldp port PORTNO (enable|disable)",
	CONFIG_STR
	DCLI_DLDP_STR
	CONFIG_ETHPORT_STR
	"Enable DLDP functionality on port\n"
	"Disable DLDP functionality on port\n"
)
{
	unsigned char	enDis = DCLI_DLDP_PORT_ENABLE;
	unsigned char	slot_no = DCLI_DLDP_INIT_0;
	unsigned char	port_no = DCLI_DLDP_INIT_0;
	unsigned int	eth_g_index = DCLI_DLDP_INIT_0;	
	unsigned int	ret	= DCLI_DLDP_RETURN_CODE_OK;

	if (argc > 2) {
		vty_out(vty, "%% Command without so many parameters!\n");;
		return CMD_WARNING;
	}
	
	/*get first param*/
	ret = parse_slotport_no((char*)argv[0], &slot_no, &port_no);
	if (NPD_FAIL == ret) {
    	vty_out(vty,"%% Error:malformed port number format!\n");
		return CMD_SUCCESS;
	}	

	/*get 2nd param : slot port number*/
	if (!strncmp(argv[1], "enable", strlen(argv[1]))) {
		enDis = DCLI_DLDP_PORT_ENABLE;
	}else if (!strncmp(argv[1], "disable", strlen(argv[1]))) {
		enDis = DCLI_DLDP_PORT_DISABLE;
	}else {
		vty_out(vty, "%% Command parameters error!\n");
		return CMD_WARNING;
	}

	ret = dcli_enable_disable_dldp_one_port(slot_no, port_no, enDis);
	if (CMD_SUCCESS == ret) {
		return CMD_SUCCESS;
	}else {
		if(NPD_IGMP_SNP_NOTENABLE == ret){
			vty_out(vty,"%% Error:igmp snooping service not enabled!\n");
		}
		else if(NPD_IGMP_SNP_PORT_NOTEXIST == ret){
			vty_out(vty,"%% Error:bad slot/port number!\n");
		}
		else if(NPD_IGMP_SNP_NOTENABLE_PORT == ret){
			vty_out(vty,"%% Error:port %d/%d not enable igmp snooping!\n",slot_no,port_no);
		}
		else if(NPD_IGMP_SNP_HASENABLE_PORT == ret){
			vty_out(vty,"%% Error:port %d/%d already enable igmp snooping!\n",slot_no,port_no);
		}
		else if(NPD_VLAN_NOTEXISTS == ret){
			vty_out(vty,"%% Error:vlan %d not exist!\n",vlanId);
		}
		else if(NPD_VLAN_NOT_SUPPORT_IGMP_SNP == ret){
			vty_out(vty,"%% Error:vlan %d not support igmp snooping!\n",vlanId);
		}
		else if (NPD_VLAN_PORT_NOTEXISTS == ret){
			vty_out(vty,"%% Error:port %d/%d not vlan %d's member!\n",slot_no,port_no,vlanId);
		}
		else if (NPD_VLAN_ERR_HW == ret){
			vty_out(vty,"%% Error:igmp configured error occurs on hardware!\n");
		}
		else if (NPD_VLAN_PORT_TRUNK_MBR == ret){
			vty_out(vty,"%% Error:port is trunk member!\n");
		}
	}

	return CMD_SUCCESS;
}
#endif


/*********************************************************
 *	command: config dldp PORTNO (enable|disable)
 *		enable/disable DLDP on vlan and its ports, and then 	pass param to
 *		DLDP Protocol
 *
 *	cmd Node: config Node
 *
**********************************************************/
DEFUN(config_dldp_npd_vlan_cmd_func,
	config_dldp_npd_vlan_cmd,
	"config dldp vlan <1-4095> (enable|disable)",
	CONFIG_STR
	DCLI_DLDP_STR
	"Config layer 2 vlan entity\n"
	"Specify vlan id for vlan entity\n"
	"Enable DLDP functionality on vlan\n"
	"Disable DLDP functionality on vlan\n"
)
{
	unsigned short  vlanId = DCLI_DLDP_INIT_0;
	unsigned char	enDis = DCLI_DLDP_PORT_ENABLE;
	unsigned int	ret	= DCLI_DLDP_RETURN_CODE_OK;
	unsigned char is_support = 0;

	dcli_dldp_get_product_support_function(vty,&is_support);
	if(1 == is_support){
		vty_out(vty,"%% Product not support this function!\n");
		return CMD_WARNING;
	}
	
	if (argc > 2) {
		vty_out(vty, "%% Command without so many parameters!\n");;
		return CMD_WARNING;
	}

	/*get first param, vlan id	*/
	ret = parse_vlan_no((char*)argv[0], &vlanId);
	if (NPD_FAIL == ret) {
    	vty_out(vty,"%% Bad parameter, vlan id illegal!\n");
		return CMD_WARNING;
	}	

	/*get 2nd param : slot port number*/
	if (!strncmp(argv[1], "enable", strlen(argv[1]))) {
		enDis = DCLI_DLDP_VLAN_ENABLE;
	}else if (!strncmp(argv[1], "disable", strlen(argv[1]))) {
		enDis = DCLI_DLDP_VLAN_DISABLE;
	}else {
		vty_out(vty, "%% Command parameters error!\n");
		return CMD_WARNING;
	}

	ret = dcli_dldp_enable_disable_one_vlan(vlanId, enDis);
	if (DCLI_DLDP_RETURN_CODE_OK == ret) {
		return CMD_SUCCESS;
	}else {
		if (DCLI_DLDP_RETURN_CODE_NOT_ENABLE_GBL == ret) {
			vty_out(vty, "%% Error:DLDP not enabled global!\n");
		}
		else if(DCLI_DLDP_RETURN_CODE_NOTENABLE_VLAN == ret) {
			vty_out(vty, "%% Error:vlan not enabled DLDP!\n");
		}
		else if(DCLI_DLDP_RETURN_CODE_HASENABLE_VLAN == ret) {
			vty_out(vty, "%% Error:vlan already enabled DLDP!\n");
		}
		else if(DCLI_DLDP_RETURN_CODE_VLAN_NOT_EXIST == ret) {
			vty_out(vty, "%% Error:vlan %d not exist!\n", vlanId);
		}
		else {
			vty_out(vty, "%% Error: %x!\n", ret);
		}
	}

	return CMD_SUCCESS;
}

/*********************************************************
 *	command: show dldp vlan status
 *		show DLDP enabled vlans list and its ports status(at protocol side)
 *
 *	cmd Node: config Node
 *
**********************************************************/
DEFUN(show_dldp_vlan_status_cmd_func,
	show_dldp_vlan_status_cmd,
	"show dldp vlan <1-4095> status",
	SHOW_STR
	DCLI_DLDP_STR
	"Config layer 2 vlan entity\n"
	"Specify vlan id for vlan entity\n"
	"Show DLDP enabled vlan list and its ports status\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;

	unsigned int ret = DCLI_DLDP_RETURN_CODE_OK;
	unsigned int i = DCLI_DLDP_INIT_0;
	unsigned int port_count = DCLI_DLDP_INIT_0;
	unsigned short vlanId = DCLI_DLDP_INIT_0;
	unsigned char slot_no = DCLI_DLDP_INIT_0;
	unsigned char port_no = DCLI_DLDP_INIT_0,is_support = 0;
	/* eth_g_index_status[slot_no][port_no][port status][loop port index] */
	unsigned long port_status[DCLI_DLDP_MAX_ETHPORT_PER_BOARD * DCLI_DLDP_MAX_CHASSIS_SLOT_COUNT][4];
	memset(port_status, -1,
			sizeof(unsigned long) * DCLI_DLDP_MAX_ETHPORT_PER_BOARD * DCLI_DLDP_MAX_CHASSIS_SLOT_COUNT * 4);
	
	dcli_dldp_get_product_support_function(vty,&is_support);
	if(1 == is_support){
		vty_out(vty,"%% Product not support this function!\n");
		return CMD_WARNING;
	}
	
	if (argc > 1) {
		vty_out(vty, "%% Command without so many parameters!\n");;
		return CMD_WARNING;
	}

	/*get first param, vlan id	*/
	ret = parse_vlan_no((char*)argv[0], &vlanId);
	if (NPD_FAIL == ret) {
    	vty_out(vty,"%% Bad parameter, vlan id illegal!\n");
		return CMD_WARNING;
	}

	dbus_error_init(&err);

	query = dbus_message_new_method_call(
										DLDP_DBUS_BUSNAME,
										DLDP_DBUS_OBJPATH,
										DLDP_DBUS_INTERFACE,
										DLDP_DBUS_METHOD_DLDP_SHOW_VLAN_MEMBER_STATUS);
	dbus_error_init(&err);
    dbus_message_append_args(query,
							DBUS_TYPE_UINT16, &vlanId,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "Failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);

	if (DCLI_DLDP_RETURN_CODE_OK == ret)
	{
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter, &port_count);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter, &iter_array);

		for (i = 0; i < port_count; i++)
		{
			DBusMessageIter iter_struct;
			dbus_message_iter_recurse(&iter_array, &iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &port_status[i][0]);	/* slot_no*/
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &port_status[i][1]);	/* port_no*/
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &port_status[i][2]);	/* status*/
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &port_status[i][3]);	/* loop port index*/
			dbus_message_iter_next(&iter_array);
		}
	}
	else if (DCLI_DLDP_RETURN_CODE_NOT_ENABLE_GBL == ret){
		vty_out(vty, "%% Error:DLDP not enabled global!\n");
		dbus_message_unref(reply);
		return CMD_SUCCESS;
	}

	#if 0
	// for debug
	vty_out(vty, "%% port_count %d\n", port_count);
	for (i = 0; i < port_count; i++) {
		vty_out(vty, "i %d port_status[0] %d, port_status[1] %d, port_status[2] %d, port_status[3] %d\n",
					i, port_status[i][0], port_status[i][1], port_status[i][2], port_status[i][3]);
	}
	#endif
	
	vty_out(vty, "=================================================\n");
	vty_out(vty, "%-7s  %-4s  %-9s  %-9s\n","VLAN ID","PORT","STATUS","LOOP-PORT" );
	vty_out(vty, "=======  ====  =========  =======================\n");
	if (port_count != 0)
	{
		for (i = 0; i < port_count; i++)
		{
			vty_out(vty, "%-7d	", vlanId);
			vty_out(vty, "%2d/%2d  ", port_status[i][0], port_status[i][1]);
			vty_out(vty, "%-9s  ", (port_status[i][2] == 1) ? "link-loop":"normal");
			if (port_status[i][2] == 1) {
				/* change portindex to soltno and portno*/
				ret = dcli_dldp_exchange_ifindex_to_slotno_portno(vty, port_status[i][3], &slot_no, &port_no);
				vty_out(vty, "%d/%d", slot_no, port_no);
			}
			vty_out(vty, "\n");
		}
	}else {
		vty_out(vty, "%% %-45s","no port enabled DLDP!\n");
		vty_out(vty, "\n");
	}
	vty_out(vty, "=================================================\n");

	dbus_message_unref(reply);

	return CMD_SUCCESS;
}

#if 0

/*********************************************************
 *	command: show dldp vlan status
 *		show DLDP enabled vlans list and its ports status(at protocol side)
 *
 *	cmd Node: config Node
 *
**********************************************************/
DEFUN(show_dldp_vlan_status_cmd_func,
	show_dldp_vlan_status_cmd,
	"show dldp vlan status",
	SHOW_STR
	DCLI_DLDP_STR
	"Show DLDP enabled vlan list and its ports status\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;

	unsigned int ret = DCLI_DLDP_RETURN_CODE_OK;
	unsigned int j = DCLI_DLDP_INIT_0;
	unsigned int product_id = DCLI_DLDP_INIT_0;
	unsigned int vlan_count = DCLI_DLDP_INIT_0;
	unsigned short vlanId = DCLI_DLDP_INIT_0;
	PORT_MEMBER_BMP	untagBmp ,tagBmp;

    memset(&untagBmp, 0, sizeof(PORT_MEMBER_BMP));
	memset(&tagBmp, 0, sizeof(PORT_MEMBER_BMP));

	ret = dcli_dldp_get_vlan_count(&vlan_count);
	if (DCLI_DLDP_RETURN_CODE_OK != ret) {
		return CMD_WARNING;
	}

	ret = dcli_dldp_get_product_id(&product_id);
	if (DCLI_DLDP_RETURN_CODE_OK != ret) {
		return CMD_WARNING;
	}

	dbus_error_init(&err);

	query = dbus_message_new_method_call(
										DLDP_DBUS_BUSNAME,
										DLDP_DBUS_OBJPATH,
										DLDP_DBUS_INTERFACE,
										DLDP_DBUS_METHOD_DLDP_SHOW_VLAN_MEMBER_STATUS);
	dbus_error_init(&err);
    dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &vlan_count,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "Failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);
	if (0 == ret){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&vlan_count);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&product_id);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
		vty_out(vty,"=================================================\n");
		vty_out(vty,"%-7s  %-40s\n","VLAN ID","PORT MEMBER LIST");
		vty_out(vty,"=======  ========================================\n");

		for (j = 0; j < vlan_count; j++) {
			DBusMessageIter iter_struct;
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&vlanId);

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&untagBmp.portMbr[0]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&untagBmp.portMbr[1]);

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&tagBmp.portMbr[0]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&tagBmp.portMbr[1]);

			dbus_message_iter_next(&iter_array); 
			if(NPD_PORT_L3INTF_VLAN_ID != vlanId) {
				vty_out(vty,"%-7d  ",vlanId);
				if ((0 != untagBmp.portMbr[0]) ||
					(0 != untagBmp.portMbr[1]) ||
					(0 != tagBmp.portMbr[0]) ||
					(0 != tagBmp.portMbr[1]))
					show_igmpsnp_vlan_member_info(vty,product_id,untagBmp,tagBmp);
				else 
					vty_out(vty,"%% %-45s","no port enabled igmp snooping!\n");
				vty_out(vty,"\n");
			}
		}
		vty_out(vty,"=================================================\n");
	}
	else if (NPD_IGMP_SNP_NOTENABLE == ret){
			vty_out(vty,"%% Error:igmp snooping service not enabled!\n");
	}
	else if(NPD_VLAN_NOTEXISTS == ret) {
		vty_out(vty,"%% Error:no vaild vlan exist.\n");
	}
	else if (NPD_VLAN_ERR_HW == ret) {
		vty_out(vty,"%% Read vlan entry from hardware error!\n");	
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
	
}


#endif
struct cmd_node dldp_node = 
{
	DLDP_NODE,
	" ",
	1
};


void dcli_dldp_element_init
(
	void
)
{
	install_node(&dldp_node,dcli_dldp_show_running_config,"DLDP_NODE");
	install_element(CONFIG_NODE, &config_dldp_debug_cmd);
	install_element(CONFIG_NODE, &config_dldp_no_debug_cmd);
	install_element(CONFIG_NODE, &config_dldp_debug_pkt_info_cmd);
	install_element(CONFIG_NODE, &config_dldp_no_debug_pkt_info_cmd);	
	install_element(CONFIG_NODE, &config_dldp_enable_cmd);
	install_element(CONFIG_NODE, &config_dldp_timer_cmd);
	install_element(CONFIG_NODE, &show_dldp_timers_cmd);
	#if 0
	// base port
	install_element(CONFIG_NODE, &config_dldp_npd_port_cmd);
	#endif
	install_element(CONFIG_NODE, &config_dldp_npd_vlan_cmd);
	install_element(CONFIG_NODE, &show_dldp_vlan_status_cmd);

}

#ifdef __cplusplus
}
#endif

