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
* dcli_prot_vlan.c
*
*
* CREATOR:
* 		hanhui@autelan.com
*
* DESCRIPTION:
* 		dcli to handle protocol vlan command to communication with npd.
*
* DATE:
*		15/4/2010
*
* FILE REVISION NUMBER:
*  		$Revision: 1.3 $
*
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

#include "dcli_prot_vlan.h"


DEFUN(prot_vlan_port_enable_config_func,
	prot_vlan_port_enable_config_cmd,
	"config protocol-vlan (enable|disable)",
	CONFIG_STR
	"Config protocol-based vlan\n"
	"Config protocol-based vlan port Enable\n"
	"Config protocol-based vlan port Disable\n"
)
{
    DBusMessage 		*query, *reply;
	DBusError 			err;
	unsigned int 		ret;
    unsigned int eth_g_index = 0;
    unsigned int isEnable = 0;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
                                         NPD_DBUS_PROT_VLAN_OBJPATH,
                                         NPD_DBUS_PROT_VLAN_INTERFACE,
                                         NPD_DBUS_PROT_VLAN_METHOD_PORT_ENABLE_CONFIG);
	
	dbus_error_init(&err);
    if(1 != argc){
        
        vty_out(vty,"%% Bad parameter number!\n");
        
        return CMD_WARNING;
    }
    if(!strncmp((char *)argv[0],"enable",strlen((char *)argv[0]))){
        isEnable = 1;
    }
    else if(!strncmp((char *)argv[0],"disable",strlen((char *)argv[0]))){
        isEnable = 0;
    }
    else{
        vty_out(vty,"%% Bad parameter %s!\n",(char *)argv[0]);
        return CMD_WARNING;
    }
    if(vty){
        eth_g_index = (unsigned int)(vty->index);
    }
    else{
        vty_out(vty,"%% Error : Get port global index failed!\n");
        return CMD_WARNING;
    }

	dbus_message_append_args(	query,
							 	DBUS_TYPE_UINT32,&eth_g_index,
							 	DBUS_TYPE_UINT32,&isEnable,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&ret,
		DBUS_TYPE_INVALID)) {
			if(PROTOCOL_VLAN_RETURN_CODE_SUCCESS != ret){
                vty_out(vty,dcli_prot_vlan_error_info(ret));
                            
            	dbus_message_unref(reply);
            	return CMD_WARNING;
			}
	} else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}


DEFUN(prot_vlan_config_vid_for_port_entry_func,
	prot_vlan_config_vid_for_port_entry_cmd,
	"config protocol-vlan (ipv4|ipv6|pppoe|udf) <1-4095>",
	CONFIG_STR
	"Config protocol-based vlan\n"
	"Config for IPv4 protocol\n"
	"Config for IPv6 protocol\n"
	"Config for PPPOE protocol\n"
	"Config for UDF protocol\n"
	"Config protocol-based vlan entry vid\n"
	)
{
    DBusMessage 		*query, *reply;
	DBusError 			err;
	unsigned int 		ret;
    unsigned int eth_g_index = 0;
    unsigned int entryNum = 0;
    unsigned short vlanId = 0;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
                                         NPD_DBUS_PROT_VLAN_OBJPATH,
                                         NPD_DBUS_PROT_VLAN_INTERFACE,
                                         NPD_DBUS_PROT_VLAN_METHOD_CONFIG_VID_BY_PORT_ENTRY);
	
	dbus_error_init(&err);
    if(2 != argc){
        
        vty_out(vty,"%% Bad parameter number!\n");
        
        return CMD_WARNING;
    }
    if(!strcmp((char *)argv[0],"ipv4")){
        entryNum = PROT_TYPE_IPV4;
    }
    else if(!strcmp((char *)argv[0],"ipv6")){
        entryNum = PROT_TYPE_IPV6;
    }
    else if(!strncmp((char *)argv[0],"pppoe",strlen((char *)argv[0]))){
        entryNum = PROT_TYPE_PPPOE_D;
    }
    else if(!strncmp((char *)argv[0],"udf",strlen((char *)argv[0]))){
        entryNum = PROT_TYPE_UDF;
    }
    else{
        vty_out(vty,"%% Bad parameter %s!\n",(char *)argv[0]);
        return CMD_WARNING;
    }
    vlanId = strtoul((char *)argv[1],NULL,NULL);
    if((0 == vlanId)||(NPD_PORT_L3INTF_VLAN_ID < vlanId)){
        vty_out(vty,"%% Bad parameter %s!\n",(char *)argv[1]);
        return CMD_WARNING;
    }
    if(vty){
        eth_g_index = (unsigned int)(vty->index);
    }
    else{
        vty_out(vty,"%% Error : Get port global index failed!\n");
        return CMD_WARNING;
    }

	dbus_message_append_args(	query,
							 	DBUS_TYPE_UINT32,&eth_g_index,
							 	DBUS_TYPE_UINT32,&entryNum,
							 	DBUS_TYPE_UINT16,&vlanId,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&ret,
		DBUS_TYPE_INVALID)) {
			if(PROTOCOL_VLAN_RETURN_CODE_SUCCESS != ret){
                vty_out(vty,dcli_prot_vlan_error_info(ret));
                            
            	dbus_message_unref(reply);
            	return CMD_WARNING;
			}
	} else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(prot_vlan_no_vid_for_port_entry_func,
	prot_vlan_no_vid_for_port_entry_cmd,
	"no protocol-vlan (ipv4|ipv6|pppoe|udf) <1-4095>",
	NO_STR
	"No config protocol-based vlan\n"
	"No config for IPv4 protocol\n"
	"No config for IPv6 protocol\n"
	"No config for PPPOE protocol\n"
	"No config for UDF protocol\n"
	"No config protocol-based vlan entry vid\n"
	)
{
    DBusMessage 		*query, *reply;
	DBusError 			err;
	unsigned int 		ret;
    unsigned int eth_g_index = 0;
    unsigned int entryNum = 0;
    unsigned short vlanId = 0;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
                                         NPD_DBUS_PROT_VLAN_OBJPATH,
                                         NPD_DBUS_PROT_VLAN_INTERFACE,
                                         NPD_DBUS_PROT_VLAN_METHOD_NO_VID_BY_PORT_ENTRY);
	
	dbus_error_init(&err);
    if(2 != argc){
        
        vty_out(vty,"%% Bad parameter number!\n");
        
        return CMD_WARNING;
    }
    if(!strcmp((char *)argv[0],"ipv4")){
        entryNum = PROT_TYPE_IPV4;
    }
    else if(!strcmp((char *)argv[0],"ipv6")){
        entryNum = PROT_TYPE_IPV6;
    }
    else if(!strncmp((char *)argv[0],"pppoe",strlen((char *)argv[0]))){
        entryNum = PROT_TYPE_PPPOE_D;
    }
    else if(!strncmp((char *)argv[0],"udf",strlen((char *)argv[0]))){
        entryNum = PROT_TYPE_UDF;
    }
    else{
        vty_out(vty,"%% Bad parameter %s!\n",(char *)argv[0]);
        return CMD_WARNING;
    }
    vlanId = strtoul((char *)argv[1],NULL,NULL);
    if((0 == vlanId)||(NPD_PORT_L3INTF_VLAN_ID < vlanId)){
        vty_out(vty,"%% Bad parameter %s!\n",(char *)argv[1]);
        return CMD_WARNING;
    }
    if(vty){
        eth_g_index = (unsigned int)(vty->index);
    }
    else{
        vty_out(vty,"%% Error : Get port global index failed!\n");
        return CMD_WARNING;
    }

	dbus_message_append_args(	query,
							 	DBUS_TYPE_UINT32,&eth_g_index,
							 	DBUS_TYPE_UINT32,&entryNum,
							 	DBUS_TYPE_UINT16,&vlanId,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&ret,
		DBUS_TYPE_INVALID)) {
			if(PROTOCOL_VLAN_RETURN_CODE_SUCCESS != ret){
                vty_out(vty,dcli_prot_vlan_error_info(ret));
                            
            	dbus_message_unref(reply);
            	return CMD_WARNING;
			}
	} else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(prot_vlan_show_port_prot_vlan_func,
	prot_vlan_show_port_prot_vlan_cmd,
	"show PORTNO protocol-vlan",
	SHOW_STR
	CONFIG_ETHPORT_STR
	"Show prot-vlan information of this port\n"
	)
{
    DBusMessage 		*query, *reply;
	DBusMessageIter	 iter;
	DBusError 			err;
	unsigned int 		ret;
    unsigned int portValue = 0;
    unsigned char slotNo = 0,portNo = 0;
    unsigned int entryNum = 0;
    unsigned short vlanId = 0;
    unsigned short protValue = 0;
    unsigned char * protName = NULL;
    unsigned char isEnable = 0;
    unsigned int isValid = 0;
    unsigned int isGlobalIndex = 0;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
                                         NPD_DBUS_PROT_VLAN_OBJPATH,
                                         NPD_DBUS_PROT_VLAN_INTERFACE,
                                         NPD_DBUS_PROT_VLAN_METHOD_SHOW_PORT_PROT_VLAN);
	
	dbus_error_init(&err);
    if(1 < argc){
        
        vty_out(vty,"%% Bad parameter number!\n");
        
        return CMD_WARNING;
    }
    if(1 == argc){
        isGlobalIndex = 0;
        ret = parse_slotport_no((char *)argv[0], &slotNo, &portNo);
        if(NPD_SUCCESS != ret){
            vty_out(vty,"%% Bad parameter input: %s!\n",(char *)argv[0]);
            return CMD_WARNING;
        }
        portValue = (slotNo << 8)|portNo;
    }
    else if(vty){
        isGlobalIndex = 1;
        portValue = (unsigned int)(vty->index);
    }
    else{
        vty_out(vty,"%% Error : Get port global index failed!\n");
        return CMD_WARNING;
    }
    dbus_message_append_args(   query,
                                    DBUS_TYPE_UINT32,&isGlobalIndex,
    							 	DBUS_TYPE_UINT32,&portValue,
    							 	DBUS_TYPE_INVALID);
 
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

    dbus_message_iter_init(reply,&iter);
    dbus_message_iter_get_basic(&iter,&ret);
	dbus_message_iter_next(&iter);	
    if(PROTOCOL_VLAN_RETURN_CODE_SUCCESS != ret){
        vty_out(vty,dcli_prot_vlan_error_info(ret));
        dbus_message_unref(reply);
	    return CMD_SUCCESS;
    }
    
    dbus_message_iter_get_basic(&iter,&protValue);
	dbus_message_iter_next(&iter);	
    vty_out(vty,"=============================\n");
    vty_out(vty," udf : ");
    if(protValue){
        vty_out(vty,"%#x\n",protValue);
    }
    else{
        vty_out(vty,"not configed\n");
    }
    vty_out(vty,"=============================\n");
    dbus_message_iter_get_basic(&iter,&slotNo);
	dbus_message_iter_next(&iter);
    dbus_message_iter_get_basic(&iter,&portNo);
	dbus_message_iter_next(&iter);
    dbus_message_iter_get_basic(&iter,&isEnable);
	dbus_message_iter_next(&iter);
    vty_out(vty," PORT %d/%d : %s\n",slotNo,portNo,isEnable?"ENABLE":"DISABLE");
    vty_out(vty,"-----------------------------\n");
    vty_out(vty," %-12s%5s%3s%8s\n","PROTOCOL","VID"," ","ISVALID");
    vty_out(vty,"-----------------------------\n");

	for(entryNum = 0;entryNum <= PROT_TYPE_UDF;entryNum++){        
        dbus_message_iter_get_basic(&iter,&protName);
	    dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter,&vlanId);
	    dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter,&isValid);
	    dbus_message_iter_next(&iter);
        vty_out(vty," %-12s%5d%5s%-6s\n",protName,vlanId," ",isValid?"TRUE":"FALSE");
        if(PROT_TYPE_PPPOE_D == entryNum){
            entryNum++;
        }
    }
    vty_out(vty,"=============================\n");
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

ALIAS(prot_vlan_show_port_prot_vlan_func,
	prot_vlan_show_this_port_prot_vlan_cmd,
	"show protocol-vlan",
	SHOW_STR
	"Show prot-vlan information of this port\n"
);

DEFUN(prot_vlan_show_port_prot_vlan_list_func,
	prot_vlan_show_port_prot_vlan_list_cmd,
	"show protocol-vlan",
	SHOW_STR
	"Show prot-vlan information list\n"
	)
{
    DBusMessage 		*query, *reply;
	DBusMessageIter	 iter;
    DBusMessageIter  iter_array, iter_struct;
	DBusError 			err;
	unsigned int 		ret;
    unsigned char slotCount = 0, portCount = 0;
    unsigned char slotNo = 0,portNo = 0;
    unsigned int entryNum = 0;
    unsigned short vlanId = 0;
    unsigned short protValue = 0;
    unsigned char * protName = NULL;
    unsigned char isEnable = 0;
    unsigned int isValid = 0;
    unsigned int isRgmii = 0;
    int i = 0,j = 0;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
                                         NPD_DBUS_PROT_VLAN_OBJPATH,
                                         NPD_DBUS_PROT_VLAN_INTERFACE,
                                         NPD_DBUS_PROT_VLAN_METHOD_SHOW_PORT_PROT_VLAN_LIST);
	
	dbus_error_init(&err);
    if(0 < argc){
        
        vty_out(vty,"%% Bad parameter number!\n");
        
        return CMD_WARNING;
    }
    
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

    
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
    
    dbus_message_iter_init(reply,&iter);
    dbus_message_iter_get_basic(&iter,&protValue);
	dbus_message_iter_next(&iter);
    vty_out(vty," Protocol-based vlan info list\n"
                      "\t Y  --  protocol-vlan enable for the port\n"
                      "\t N  --  protocol-vlan disable for the port\n"
                      "\t *  --  vid valid for the port & entry\n"
                      "\t -  --  protocol-vlan info not config or get failed\n");
    vty_out(vty,"========================================================\n");
    vty_out(vty,"  udf : ");
    if(protValue){
        vty_out(vty,"%#.4x\n",protValue);
    }
    else{
        vty_out(vty,"not configed\n");
    }
    
    vty_out(vty,"========================================================\n");
    vty_out(vty,"%6s%3s%-10s%-10s%-10s%-10s%-10s\n",
                "  PORT","","ENABLE", "IPV4",  "IPV6",  "PPPOE", "UDF");
    vty_out(vty,"--------------------------------------------------------\n");

    dbus_message_iter_get_basic(&iter,&slotCount);
    for(i = 0;i < slotCount;i++){
	    dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter,&slotNo);
    	dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter,&portCount);
        for(j = 0;j < portCount;j++){            
    	    dbus_message_iter_next(&iter);
            dbus_message_iter_get_basic(&iter,&portNo);
    	    dbus_message_iter_next(&iter);
            dbus_message_iter_get_basic(&iter,&isRgmii);
            if(!isRgmii){
                
    	        dbus_message_iter_next(&iter);
                dbus_message_iter_get_basic(&iter,&ret);
                if(PROTOCOL_VLAN_RETURN_CODE_SUCCESS != ret){
                    vty_out(vty,"%s%d/%-2d%6s%6s%4s%3s%3s%4s%3s%3s%4s%3s%3s%4s%3s%3s\n",
                        "",slotNo,portNo,"-","","-","","","-","","","-","","","-","","");
                    vty_out(vty,dcli_prot_vlan_error_info(ret));
                    continue;
                }
                else{             
                    
        	        dbus_message_iter_next(&iter);
                    dbus_message_iter_get_basic(&iter,&isEnable);
            	    dbus_message_iter_next(&iter);
                    vty_out(vty,"%2s%d/%-2d%6s%5s","",slotNo,portNo,isEnable?"Y":"N","");                    

                    dbus_message_iter_recurse(&iter,&iter_array);

                	for(entryNum = 0;entryNum <= PROT_TYPE_UDF;entryNum++){
                        if(PROT_TYPE_PPPOE_S == entryNum){
                            continue;
                        }
                        dbus_message_iter_recurse(&iter_array,&iter_struct);
                        dbus_message_iter_get_basic(&iter_struct,&vlanId);
                	    dbus_message_iter_next(&iter_struct);
                        dbus_message_iter_get_basic(&iter_struct,&isValid);
                	    dbus_message_iter_next(&iter_struct);
                        if(vlanId){
                            vty_out(vty,"%4d%3s%3s",vlanId,isValid?"(*)":"","");
                        }
                        else{
                            vty_out(vty,"%4s%3s%3s","-","","");
                        }
                                            
                        dbus_message_iter_next(&iter_array);
                    }
                    vty_out(vty,"\n");
                }
            }
        }
    }
    vty_out(vty,"========================================================\n");
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}


DEFUN(prot_vlan_udf_ethtype_config_func,
	prot_vlan_udf_ethtype_config_cmd,
	"config protocol-vlan udf PROTOCOL",
	CONFIG_STR
	"Config protocol-based vlan\n"
	"Config protocol-based vlan UDF\n"
	"Config UDF value, eg. 0, 0x8888,...\n"
	)
{
    DBusMessage 		*query, *reply;
	DBusMessageIter	 iter;
	DBusError 			err;
	unsigned int 		ret;
    unsigned int protValue = 0;
    int i = 0;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
                                         NPD_DBUS_PROT_VLAN_OBJPATH,
                                         NPD_DBUS_PROT_VLAN_INTERFACE,
                                         NPD_DBUS_PROT_VLAN_METHOD_CONFIG_UDF_ETHTYPE_VALUE);
	
	dbus_error_init(&err);
    if(1 != argc){
        
        vty_out(vty,"%% Bad parameter number!\n");
        
        return CMD_WARNING;
    }
    if(!(((1 == strlen((char *)argv[0]))&&\
            ('0' == (char)argv[0][0]))||\
            ((1 < strlen((char *)argv[0]))&&\
            (('x' == (char)argv[0][1])||\
            ('X' == (char)argv[0][1]))))){
        vty_out(vty,"%% Bad parameter input: %s!\n",(char *)argv[0]);
        return CMD_WARNING;
        
    }
    for(i = 2;i < strlen((char *)argv[0]);i++){
        if(!((('0' <= ((char)argv[0][i]))&&('9' >= ((char)argv[0][i])))||\
            (('a' <= ((char)argv[0][i]))&&('f' >= ((char)argv[0][i])))||\
            (('A' <= ((char)argv[0][i]))&&('F' >= ((char)argv[0][i]))))){
            
            vty_out(vty,"%% Bad parameter input: %s!\n",(char *)argv[0]);
            return CMD_WARNING;
         }
    }
    protValue = strtoul((char *)argv[0],NULL,NULL);
	dbus_message_append_args(	query,
						 	DBUS_TYPE_UINT32,&protValue,
						 	DBUS_TYPE_INVALID);
    reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
    
    dbus_message_iter_init(reply,&iter);
    dbus_message_iter_get_basic(&iter,&ret);
	dbus_message_iter_next(&iter);
    if(PROTOCOL_VLAN_RETURN_CODE_SUCCESS != ret){
        vty_out(vty,dcli_prot_vlan_error_info(ret));
    }
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}


char * dcli_prot_vlan_error_info(unsigned int retCode){
    switch(retCode){
        case PROTOCOL_VLAN_RETURN_CODE_SUCCESS:
            return "";
        case COMMON_RETURN_CODE_NULL_PTR:
            return "%% Unexpected null pointer found!\n";
        case COMMON_RETURN_CODE_BADPARAM:
            return "%% Bad parameter input!\n";
        case PROTOCOL_VLAN_RETURN_CODE_ERROR:
            return "%% Command execute Failed!\n";
        case PROTOCOL_VLAN_RETURN_CODE_NAM_ERR:
            return "%% Asic config FAILED!\n";
        case PROTOCOL_VLAN_RETURN_CODE_ETH_TYPE_RANG_ERR:
            return "%% The PROTOCOL value should greater than 0x0600 or be 0!\n";
        case PROTOCOL_VLAN_RETURN_CODE_ETH_TYPE_EXISTS:
            return "%% This PROTOCOL already exists in other entry!\n";
        case PROTOCOL_VLAN_RETURN_CODE_VID_NOT_SET:
            return "%% The vid is not set yet for the port and entry!\n";
        case PROTOCOL_VLAN_RETURN_CODE_VID_NOT_MATCH:
            return "%% The input vid is not match with configuration!\n";
        case PROTOCOL_VLAN_RETURN_CODE_UNSUPPORT:
            return "%% This port is not support this command!\n";
        case PROTOCOL_VLAN_RETURN_CODE_ALREADY_TAG_MEMBER:
            return "%% This port already tag member of the vlan,delete from the vlan of change another vid!\n";
        case PROTOCOL_VLAN_RETURN_CODE_VLAN_CREATE_FAILED:
            return "%% Active vlan for protocol-vlan failed!\n";
        case PROTOCOL_VLAN_RETURN_CODE_ADD_PORT_TO_VLAN_FAILED:
            return "%% Add port to vlan for protocol-vlan failed!\n";
        default:
            return "%% Unknown error!\n";
    }    
}

void dcli_prot_vlan_element_init(void)
{
        install_element(ETH_PORT_NODE,  &prot_vlan_port_enable_config_cmd);
        install_element(ETH_PORT_NODE,  &prot_vlan_config_vid_for_port_entry_cmd);
        install_element(ETH_PORT_NODE,  &prot_vlan_no_vid_for_port_entry_cmd);
        install_element(ETH_PORT_NODE,  &prot_vlan_show_this_port_prot_vlan_cmd);
        install_element(CONFIG_NODE,  &prot_vlan_show_port_prot_vlan_cmd);
        install_element(CONFIG_NODE,  &prot_vlan_show_port_prot_vlan_list_cmd);
        install_element(CONFIG_NODE,  &prot_vlan_udf_ethtype_config_cmd);        
     
}

#ifdef __cplusplus
}
#endif
