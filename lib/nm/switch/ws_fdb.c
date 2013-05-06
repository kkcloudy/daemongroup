
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
* ws_fdb.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* function for web
*
*
***************************************************************************/
/*dcli_fdb.c 1.54*/
/*author tangsiqi*/
/*update time 08-12-15*/

#include "ws_fdb.h"
/*#include "cgic.h"*/
#include "ws_usrinfo.h"
#if 0
#include "ws_ec.h"
#include "ws_err.h"
#endif
#include "ws_returncode.h"
/*
void dcli_fdb_output(unsigned int type) {
	switch (type){
	case NPD_FDB_ERR_NONE :
		break;
	case NPD_FDB_ERR_GENERAL:
		printf("operate fail!\n");
		
		break;
	case NPD_FDB_ERR_NODE_EXIST :
		printf("fdb item exist!\n");
		
		break;
	case NPD_FDB_ERR_NODE_NOT_EXIST:
		printf("fdb item not exist !\n");
		
		break;
	case NPD_FDB_ERR_PORT_NOTIN_VLAN:
		printf("port not in vlan !\n");
		
		break;
	case NPD_FDB_ERR_VLAN_NONEXIST:
		printf("vlan non-exist !\n");
		
		break;
	case NPD_FDB_ERR_SYSTEM_MAC:
		printf("MAC address conflict with system MAC address!\n");
		
		break;
	default :
		printf("operate error !\n");
		
	}
}
*/
int parse_vlan_no_fdb(char* str,unsigned short* vlanId) 
{
	char *endptr =NULL;
	char c;
	if (NULL == str) return -1;
	c = str[0];
	if (c>='0'&&c<='9')
	{
		*vlanId= strtoul(str,&endptr,10);
		if('\0'== endptr[0])
		{

			return 0;		  
		}
		if(endptr[0]>'9'||endptr[0]<'0')
		{
			//printf("error.\n");
			return -1;
		}	
	}
	else 
	{
		return -1; //not Vlan ID. for Example

	}
	return 0; 
}

inline int parse_int_parse(char* str,unsigned int* shot)
{
	char *endptr = NULL;

	if (NULL == str) return NPD_FAIL;
	*shot= strtoul(str,&endptr,10);
	return NPD_SUCCESS; 
}
int mac_format_check_fdb
(
	char* str,
	int len
) 
{
	int i = 0;
	unsigned int result = NPD_SUCCESS;
	char c = 0;

	if( 17 != len)
	{
		return NPD_FAIL;
	}
	for(;i<len;i++)
	{
		c = str[i];
		if((2 == i)||(5 == i)||(8 == i)||(11 == i)||(14 == i))
		{
			if((':'!=c)&&('-'!=c))
			return NPD_FAIL;
		}
		else if((c>='0'&&c<='9')||
		(c>='A'&&c<='F')||
		(c>='a'&&c<='f'))
		continue;
		else 
		{
			result = NPD_FAIL;
			return result;
		}
	}
	if((str[2] != str[5])||(str[2] != str[8])||(str[2] != str[11])||(str[2] != str[14])||
	(str[5] != str[8])||(str[5] != str[11])||(str[5] != str[14])||
	(str[8] != str[11])||(str[8] != str[14]))
    {

		result = NPD_FAIL;
		return result;
	}

	return result;
}
int parse_vlan_string(char* input)
{
	char c;
	if(NULL == input) {
	 return NPD_FAIL;
	}
	c=input[0];
	 
	if ((c>='A'&&c<='Z')||(c>='a'&&c<='z')||('_'==c)){
	 	return 1;
	}
	return NPD_FAIL;
	
}

/**************************************
* config fdb agingtime on Both Sw & Hw. 
*
* Usage: config fdb agingtime (<10-630>|0) 
*
*input Params: 
*       agingtime value:	<10-630|0>agingtime skip value is 10s.
*       param 0 means fdb table is not aging.
*
*output params: none
*DEFUN(config_fdb_agingtime_cmd_func,
	config_fdb_agingtime_cmd,
	"config fdb agingtime (<10-630>|0)",
	"Config system information\n"
	"FDB table \n"
	"FDB table agingtime Range <10-630s>|0\n"
	"config fdb aging value <10-630>\n"
	"config fdb agingout 0\n"
)
****************************************/
//return -2: agingtime 不符合标准; return -3: agingtime 不能为空
int config_fdb_agingtime(char *agingtime)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int testAgingtime=0;
	unsigned int 	op_ret = 0;

	op_ret = parse_int_parse((char *)agingtime,&testAgingtime);
	if(NPD_FAIL == op_ret) {
    	//vty_out(vty,"FDB agingtime form erro!\n");
		return -3;
	}
	if(testAgingtime ==0){
		;
	}
    else if ((testAgingtime < 10) ||(testAgingtime > 630)){
		//vty_out(vty,"FDB agingtime is outrange!\n");
		//vty_out(vty,"FDB agingtime %d. \n",testAgingtime);
		return -2;
	}
	//vty_out(vty,"FDB agingtime %d. \n",agingtime);

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_AGINGTIME);
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
							 	DBUS_TYPE_UINT32,&testAgingtime,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_UINT32,&testAgingtime,
		DBUS_TYPE_INVALID)) {
			//dcli_fdb_output(op_ret);
			return op_ret;
		}
			
	 else {
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}
	dbus_message_unref(reply);
	return NPD_FDB_ERR_NONE;
}



/**************************************
*show fdb agingtime on Both Sw & Hw. 
*
* Usage: show fdb agingtime 
*
*input Params: none
*
*output params:
*       agingtime value:	<10-630|0>agingtime .
*       param 0 means fdb table is not aging.
*DEFUN(show_fdb_agingtime_cmd_func,
	show_fdb_agingtime_cmd,
	"show fdb agingtime",
	"Show system information\n"
	"FDB table\n"
	"FDB table agingtime range <10-630s>"
)
****************************************/

int show_fdb_agingtime(int *agingtime)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int testAgingtime = 0;
	unsigned int 	op_ret = 0;

	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_SHOW_FDB_AGINGTIME);
	
	dbus_error_init(&err);
    
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
	 	DBUS_TYPE_UINT32,&testAgingtime,
		DBUS_TYPE_INVALID)) {
		/*
    		if (NPD_DBUS_SUCCESS == op_ret ) 
    	    {
    			
    			//vty_out(vty,"FDB Table Agingtime %d.\n",testAgingtime);
    			*agingtime = testAgingtime;
    		}	 
    		else if (NPD_FDB_ERR_OCCUR_HW== op_ret ) 
    		 {
    			// ShowAlert(search(lcontrol,"HW_error"));
				 return 2;
    		 }

   */
		if (NPD_FDB_ERR_OCCUR_HW== op_ret ) {
			//vty_out(vty,"%% Error,failed when config hw.\n ");
			//ShowAlert(search(lcontrol,"HW_error"));
			return -5;
		}
		else{
            //vty_out(vty,"The FDB AGINGTIME is %d\n",testAgingtime);
            *agingtime = testAgingtime;
		}


		}
	dbus_message_unref(reply);
	return NPD_FDB_ERR_NONE;
}


/**************************************
*config fdb agingtime on Both Sw & Hw. 
*
* Usage:config fdb agingtime default 
*
*input Params: none
*
*output params:none
* DEFUN(config_fdb_no_config_agingtime_cmd_func,
	config_fdb_no_agingtime_cmd,
	"config fdb agingtime default ",
	"config system info!\n"
	"config fdb table info!\n"
	"config fdb default value!\n"
	"config fdb default 300s!\n"
)    
****************************************/

int config_fdb_agingtime_default()
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int testAgingtime = 300;
	unsigned int 	op_ret = 0;
	//agingtime = testagingtime;
	//vty_out(vty,"fdb table agingtime %d. \n",testAgingtime);
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_DEFAULT_AGINGTIME);
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
							 	DBUS_TYPE_UINT32,&testAgingtime,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}
	
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_UINT32,&testAgingtime,
		DBUS_TYPE_INVALID)) {
			//dcli_fdb_output(op_ret);
			return op_ret;
		}
			
	 else {
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}
	dbus_message_unref(reply);
	return NPD_FDB_ERR_NONE;
}


/**************************************
*config fdb static fdb table item on Both Sw & Hw. 
*
* Usage:create fdb static mac MAC vlan <1-4094> port PORTNO 
*
*input Params: 
*	MAC: destination mac form 00:00:11:11:aa:aa
*	<1-4094>: valid vlan index  range,which has been configed befor doing.
*   	PORTNO:  destination port ,form slot/port
*
*output params:none
* DEFUN(config_fdb_mac_vlan_port_static_cmd_func,
	config_fdb_mac_vlan_port_cmd,
	"create fdb static mac MAC vlan <1-4094> port PORTNO",
	"Config system information\n"
	"config FDB table\n"
	"find static fdb item \n"
	"config FDB table MAC field.\n"
	"config MAC address in Hexa.Eg 00:00:11:11:aa:aa\n"
	"config FDB table vlan Id\n"
	"config VLAN Id valid range <1-4094>\n"
	"config port \n"
	CONFIG_ETHPORT_STR
)    
****************************************/

int create_fdb_static(char *mac, char *vlanid, char *portno)
{
	DBusMessage *query, *reply;
	ETHERADDR		macAddr;
	unsigned short	vlanId = 0;
	unsigned char	slot_no = 0,port_no = 0; 
	DBusError err;
	unsigned int 	op_ret = 0;	
	
	memset(&macAddr,0,sizeof(ETHERADDR));
	
	op_ret = parse_mac_addr((char *)mac,&macAddr);
	if (NPD_FAIL == op_ret) {
    	//vty_out(vty,"Unknow mac addr format!\n");
		return -2;
	}
   
	op_ret=is_muti_brc_mac(&macAddr);
	if(op_ret==1){
		//vty_out(vty,"erro:input broadcast or multicast mac!\n");
		return -3;
		}
		
	
	op_ret = parse_short_parse((char*)vlanid, &vlanId);
	if (NPD_FAIL == op_ret) {
    	//vty_out(vty,"Unknow vlan id format.\n");
		return -4;
	}
    if ((vlanId<MIN_VLANID)||(vlanId>MAX_VLANID)){
		//vty_out(vty,"FDB vlan outrange!\n");
		return -5;
	}
	//vty_out(vty,"fdb vlan id %d.\n",vlanId);

	
	op_ret = parse_slotport_no((char *)portno, &slot_no, &port_no);
	if (NPD_FAIL == op_ret) {
    	//vty_out(vty,"Unknow portno format!\n");
		return -6;
	}
    if ((slot_no<MIN_SLOT || slot_no>MAX_SLOT)||(port_no<MIN_PORT ||port_no>MAX_PORT)){
		//vty_out(vty,"Unknow portno format!\n");
		return -7;
	}
	

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,	\
										NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_STATIC);
	
	dbus_error_init(&err);
	dbus_message_append_args(	query,
								DBUS_TYPE_UINT16,&vlanId,
							 	DBUS_TYPE_BYTE,&macAddr.arEther[0],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[1],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[2],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[3],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[4],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[5],
							 	DBUS_TYPE_BYTE,&slot_no,
							 	DBUS_TYPE_BYTE,&port_no,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
			//vty_out(vty,"failed get reply.\n");
			if (dbus_error_is_set(&err)) {
					//vty_out(vty,"%s raised: %s",err.name,err.message);
					dbus_error_free(&err);
			}
			
			return CMD_FAILURE;
	}

	if (dbus_message_get_args ( reply, &err,
			DBUS_TYPE_UINT32,&op_ret,
			DBUS_TYPE_INVALID)) {
			//dcli_fdb_output(op_ret);
			return op_ret;
			}

	else {
			//vty_out(vty,"Failed get args.\n");
			if (dbus_error_is_set(&err)) {
					//vty_out(vty,"%s raised: %s",err.name,err.message);
					dbus_error_free(&err);
			}
			return CMD_FAILURE;
	}
	dbus_message_unref(reply);
	return NPD_FDB_ERR_NONE;
}

/**************************************
*config fdb static fdb table item on Both Sw & Hw. 
*
* Usage:create fdb static mac MAC vlan VLANNAME port PORTNO 
*
*input Params: 
*	MAC: destination mac form 00:00:11:11:aa:aa
*	VLANNAME:  before doing, the vlan name which has been configed ,
*                           VLANNAME must be to begin with letter or '_'
*   	PORTNO:  destination port, form slot/port
*
*output params:none
* DEFUN(config_fdb_mac_vlan_port_static_name_cmd_func,
	config_fdb_mac_vlan_port_name_cmd,
	"create fdb static mac MAC vlan VLANNAME port PORTNO",
	"Config system information\n"
	"config FDB table\n"
	"find static fdb item \n"
	"config FDB table MAC field.\n"
	"config MAC address in Hexa.Eg 00:00:11:11:aa:aa\n"
	"config FDB table vlanname\n"
	" VLANNAME must be to begin with letter or '_'\n"
	"config port \n"
	CONFIG_ETHPORT_STR
)    
****************************************/

int create_fdb_static_vlanname(char *mac, char *vlan_name, char *portno)
{
	DBusMessage *query, *reply;
	DBusError err;
	char* vlanname = NULL;
	unsigned char	slot_no = 0,port_no = 0; 
	int 	op_ret = 0;
	ETHERADDR		macAddr;
	memset(&macAddr,0,sizeof(ETHERADDR));	
	op_ret = parse_mac_addr((char *)mac,&macAddr);
	if (NPD_FAIL == op_ret) {
    	//vty_out(vty,"Unknow mac addr format!\n");
		return -2;
	}
	
	op_ret=is_muti_brc_mac(&macAddr);
	if(op_ret==1){
		//vty_out(vty,"erro:input broadcast or multicast mac!\n");
		return -3;
	}
		
	
	op_ret =parse_vlan_string((char*)vlan_name);

    if (NPD_FAIL == op_ret) {
    	//vty_out(vty,"vlanname form erro!\n");
		return -4;
	}

    vlanname= (char *)vlan_name;
	op_ret = parse_slotport_no((char *)portno, &slot_no, &port_no);
	if (NPD_FAIL == op_ret) {
    	//vty_out(vty,"Unknow portno format!\n");
		return -6;
	}
    if ((slot_no<MIN_SLOT || slot_no>MAX_SLOT)||(port_no<MIN_PORT ||port_no>MAX_PORT)){
		//vty_out(vty,"Unknow portno format!\n");
		return -7;
	}
	

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_STATIC_WITH_NAME);
	
	dbus_error_init(&err);
	dbus_message_append_args(	query,
								DBUS_TYPE_STRING,&vlanname,
							 	DBUS_TYPE_BYTE,&macAddr.arEther[0],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[1],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[2],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[3],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[4],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[5],
							 	DBUS_TYPE_BYTE,&slot_no,
							 	DBUS_TYPE_BYTE,&port_no,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)) {
		//dcli_fdb_output(op_ret);
		return op_ret;
		} 
	else {
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}
	dbus_message_unref(reply);
	return NPD_FDB_ERR_NONE;
}



/**************************************
*config fdb static fdb trunk table item on Both Sw & Hw. 
*
* Usage:create fdb static mac MAC vlan VLANNAME trunk <1-127>
*
*input Params: 
*	MAC: destination mac form 00:00:11:11:aa:aa
*	VLANNAME:  before doing, the vlan name which has been configed ,
*                           VLANNAME must be to begin with letter or '_'
*   	<1-127>:  destination trunk
*
*output params:none
*   DEFUN(config_fdb_mac_vlan_trunk_static_name_cmd_func,
	config_fdb_mac_vlan_trunk_name_cmd,
	"create fdb static mac MAC vlan VLANNAME trunk <1-127>",
	"Config system information\n"
	"config FDB table\n"
	"find static fdb item \n"
	"config FDB table MAC field.\n"
	"config MAC address in Hexa.Eg 00:00:11:11:aa:aa\n"
	"config FDB table vlanname\n"
	" VLANNAME must be to begin with letter or '_'\n"
	"config trunk entry \n"
	"Trunk ID range <1-127>\n"
)  
****************************************/

int create_fdb_static_vlanname_trunk(char *mac, char *vlan_name, char *trunkid)
{
	DBusMessage *query, *reply;
	DBusError err;
	char* vlanname = NULL;
	unsigned short  trunkId = 0;
	int 	op_ret = 0;
	ETHERADDR		macAddr;
	memset(&macAddr,0,sizeof(ETHERADDR));	
	op_ret = parse_mac_addr((char *)mac,&macAddr);
	if (NPD_FAIL == op_ret) {
    	//vty_out(vty,"Unknow mac addr format!\n");
		return -2;
	}
	
	op_ret=is_muti_brc_mac(&macAddr);
	if(op_ret==1){
		//vty_out(vty,"erro:input broadcast or multicast mac!\n");
		return -3;
	}
		
	
	 op_ret =parse_vlan_string((char*)vlan_name);

   if (NPD_FAIL == op_ret) {
    	//vty_out(vty,"vlanname form erro!\n");
		return -4;
	}

    vlanname= (char *)vlan_name;

    op_ret = parse_short_parse((char*)trunkid, &trunkId);
	if (NPD_FAIL == op_ret) {
    	//vty_out(vty,"Unknow trunk id format.\n");
		return -6;
	}
    if ((trunkId<1)||(trunkId>127)){
		//vty_out(vty,"FDB vlan outrange!\n");
		return -7;
	}

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_STATIC_TRUNK_WITH_NAME);
	
	dbus_error_init(&err);
	dbus_message_append_args(	query,
								DBUS_TYPE_STRING,&vlanname,
								DBUS_TYPE_UINT16,&trunkId,
							 	DBUS_TYPE_BYTE,&macAddr.arEther[0],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[1],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[2],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[3],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[4],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[5],
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		
		DBUS_TYPE_INVALID)) {
		//dcli_fdb_output(op_ret);
		return op_ret;
		} 
	else {
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}
	dbus_message_unref(reply);
	return NPD_FDB_ERR_NONE;
}



/**************************************
*config fdb static fdb table item on Both Sw & Hw. 
*
* Usage:create fdb static mac MAC vlan <1-4094> trunk <1-127> 
*
*input Params: 
*	MAC: destination mac form 00:00:11:11:aa:aa
*	<1-4094>: valid vlan index  range,which has been configed befor doing.
*   	<1-127>:  destination trunk number
*
*output params:none
*   DEFUN(config_fdb_mac_vlan_trunk_static_cmd_func,
	config_fdb_mac_vlan_trunk_cmd,
	"create fdb static mac MAC vlan <1-4094> trunk <1-127>",
	"Config system information\n"
	"config FDB table\n"
	"find static fdb item \n"
	"config FDB table MAC field.\n"
	"config MAC address in Hexa.Eg 00:00:11:11:aa:aa\n"
	"config FDB table vlan Id\n"
	"config VLAN Id valid range <1-4094>\n"
	"config trunk entry \n"
	"Trunk ID range <1-127>\n"
)  
****************************************/

int create_fdb_static_vlanid_trunk(char * mac,char * vlan_id,char * trunkid)
{
	DBusMessage *query, *reply;

	unsigned short	vlanId = 0;
	unsigned short  trunkId = 0;
	DBusError err;
	unsigned int 	op_ret = 0;	
	
	ETHERADDR		macAddr;
	memset(&macAddr,0,sizeof(ETHERADDR));	
	op_ret = parse_mac_addr((char *)mac,&macAddr);

	if (NPD_FAIL == op_ret) {
    	//vty_out(vty,"Unknow mac addr format!\n");
		return -2;
	}
   
	op_ret=is_muti_brc_mac(&macAddr);
	if(op_ret==1){
		//vty_out(vty,"erro:input broadcast or multicast mac!\n");
		return -3;
		}
		
	
	op_ret = parse_short_parse((char*)vlan_id, &vlanId);
	if (NPD_FAIL == op_ret) {
    	//vty_out(vty,"Unknow vlan id format.\n");
		return -4;
	}
    if ((vlanId<MIN_VLANID)||(vlanId>MAX_VLANID)){
		//vty_out(vty,"FDB vlan outrange!\n");
		return -5;
		}
	//vty_out(vty,"fdb vlan id %d.\n",vlanId);

	
	op_ret = parse_short_parse((char*)trunkid, &trunkId);
	if (NPD_FAIL == op_ret) {
    	//vty_out(vty,"Unknow trunk id format.\n");
		return -6;
	}
    if ((trunkId<1)||(trunkId>127)){
		//vty_out(vty,"FDB vlan outrange!\n");
		return -7;
	}

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,	\
										NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_TRUNK_STATIC);
	
	dbus_error_init(&err);
	dbus_message_append_args(	query,
								DBUS_TYPE_UINT16,&vlanId,
					            DBUS_TYPE_UINT16,&trunkId,
							 	DBUS_TYPE_BYTE,&macAddr.arEther[0],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[1],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[2],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[3],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[4],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[5],
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
			//vty_out(vty,"failed get reply.\n");
			if (dbus_error_is_set(&err)) {
					//vty_out(vty,"%s raised: %s",err.name,err.message);
					dbus_error_free(&err);
			}
			return CMD_FAILURE;
	}

	if (dbus_message_get_args ( reply, &err,
			DBUS_TYPE_UINT32,&op_ret,
			DBUS_TYPE_INVALID)) {
			//dcli_fdb_output(op_ret);
			return op_ret;
			}

	else {
			//vty_out(vty,"Failed get args.\n");
			if (dbus_error_is_set(&err)) {
					//vty_out(vty,"%s raised: %s",err.name,err.message);
					dbus_error_free(&err);
			}
			return CMD_FAILURE;
	}
	dbus_message_unref(reply);
	return NPD_FDB_ERR_NONE;
}


/**************************************
*config specify fdb item deny to forward base on (dmac|smac) on Both Sw & Hw. 
*
* Usage:delete fdb  blacklist (dmac|smac) MAC vlan <1-4094> 
*
*input Params: 
*	(dmac|smac): two chose one param( destination mar or source mac) mac form 00:00:11:11:aa:aa
*	<1-4094>: valid vlan index  range, before doing the vlan index which has been configed 
*   
*
*output params:none
*   DEFUN(config_fdb_mac_vlan_match_no_drop_cmd_func,
	config_fdb_mac_vlan_match_no_drop_cmd,
	"delete fdb  blacklist (dmac|smac) MAC vlan <1-4094>",
	"Config system information\n"
	"delete FDB table\n"
	"config FDB table make match forward command!\n"
	"config FDB table dmac match out of blacklist \n"
	"config FDB table smac match out of blacklist \n"
	"config MAC address in Hexa.Eg 00:00:11:11:aa:aa\n"
	"config FDB table vlan Id\n"
	"config VLAN Id valid range <1-4094>\n"
	"config FDB table make match forward command!\n"
	"config forwarding!\n"
)  
****************************************/
/* modify 08-12-15 */

int delete_fdb_blacklist_by_vid(char *mactype, char *mac, char *vlanid)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	ETHERADDR		macAddr;
	unsigned short	vlanId = 0;
	int 	op_ret = 0,len = 0;
    unsigned int   flag = 0;
	char*  string=  "dmac";
	memset(&macAddr,0,sizeof(ETHERADDR));
	
	len = strlen((char*)mactype);
	if((len>4)||(0 == len)){
		return -6;
	}
	else{
		op_ret=strncmp(string,(char*)mactype,len);
		if(op_ret==0){
			flag =1;
		}
		else if(op_ret<0){
			flag= 0;
		}
		else{
           return -6;
		}
	}

		
	op_ret = parse_mac_addr((char *)mac,&macAddr);
	if (NPD_FAIL == op_ret) {
    	//vty_out(vty,"Unknow mac addr format.\n");
		return -2;
	}
	
	op_ret=is_muti_brc_mac(&macAddr);
	if(op_ret==1){
		//vty_out(vty,"erro:input broadcast or multicast mac!\n");
		return -3;
		}
		

	op_ret = parse_short_parse((char*)vlanid, &vlanId);
	if (NPD_FAIL == op_ret) {
    	//vty_out(vty,"Unknow vlan id format.\n");
		return -4;
	}
    if (vlanId<MIN_VLANID||vlanId>MAX_VLANID){
		//vty_out(vty,"FDB vlan outrang!\n");
		return -5;
		}
	
 	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,	\
						NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_NO_DROP);
	
	dbus_error_init(&err);
	dbus_message_append_args(	query,
		                        DBUS_TYPE_UINT32,&flag,								
							 	DBUS_TYPE_UINT16,&vlanId,
							 	DBUS_TYPE_BYTE,&macAddr.arEther[0],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[1],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[2],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[3],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[4],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[5],
							 	DBUS_TYPE_INVALID);
	
		
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}
	

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)) {
		//dcli_fdb_output(op_ret);
		return op_ret;
		} 
	else {
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
	}
	
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

/**************************************
*config specify fdb item desterilize to forward base on (dmac|smac) on Both Sw & Hw. 
*
* Usage:delete fdb  blacklist (dmac|smac) MAC vlan <1-4094> 
*
*input Params: 
*	(dmac|smac): two chose one param( destination mar or source mac) mac form 00:00:11:11:aa:aa
*	VLANNAME:  before doing, the vlan name which has been configed ,
*                           VLANNAME must be to begin with letter or '_' 
*
*output params:none
* DEFUN(config_fdb_mac_vlan_match_no_drop_name_cmd_func,
	config_fdb_mac_vlan_match_no_drop_name_cmd,
	"delete fdb  blacklist (dmac|smac) MAC vlan VLANNAME",
	"Config system information\n"
	"delete FDB table\n"
	"config FDB table make match forward command!\n"
	"config FDB table dmac match out of blacklist \n"
	"config FDB table smac match out of blacklist \n"
	"config MAC address in Hexa.Eg 00:00:11:11:aa:aa\n"
	"config FDB table vlanname\n"
	"vlanname must be to begin with letter or '_'\n"
	"config FDB table make match forward command!\n"
	"config forwarding!\n"
)    
****************************************/
/* checked 08-12-15 */
int delete_fdb_blacklist_by_vlanname(char *mactype, char *mac, char *vlan_name)
{
	DBusMessage *query, *reply;
	DBusError err;
	char *vlanname = NULL;
	ETHERADDR		macAddr;
	
	int 	op_ret = 0;
	unsigned int    flag = 0;
	unsigned char   len = 0;
	char* string="dmac";
	memset(&macAddr,0,sizeof(ETHERADDR));
	
	len = strlen((char*)mactype);
		if((len>4)||(0 == len)){
			return -6;
		}
		else{
			op_ret=strncmp(string,(char*)mactype,len);
			if(op_ret==0){
				flag =1;
			}
			else if(op_ret<0){
				flag= 0;
			}
			else{
			   return -6;
			}
		}

	op_ret = parse_mac_addr((char *)mac,&macAddr);
	if (NPD_FAIL == op_ret) {
    	//vty_out(vty,"Unknow mac addr format.\n");
		return -2;
	}

	op_ret=is_muti_brc_mac(&macAddr);
	if(op_ret==1){
		//vty_out(vty,"erro:input broadcast or multicast mac!\n");
		return -3;
		}

	 op_ret =parse_vlan_string((char*)vlan_name);

   if (NPD_FAIL == op_ret) {
    	//vty_out(vty,"vlanname form erro!\n");
		return -4;
	}

    vlanname= (char *)vlan_name;
    
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,	\
						NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_NO_DROP_WITH_NAME);
	
	dbus_error_init(&err);
	dbus_error_init(&err);
	dbus_message_append_args(	query,								
	                            DBUS_TYPE_UINT32,&flag,
							 	DBUS_TYPE_STRING,&vlanname,
							 	DBUS_TYPE_BYTE,&macAddr.arEther[0],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[1],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[2],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[3],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[4],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[5],
							 	DBUS_TYPE_INVALID);
	
		
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}
	

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)) {
		//dcli_fdb_output(op_ret);
		return op_ret;
		} 
	else {
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
	}
	
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}


/**************************************
*delete fdb item which belong to the vlan on Both Sw & Hw. 
*
* Usage:delete fdb static vlan <1-4094> 
*
*input Params: 
*	
*	<1-4094>:  valid vlan index  range, before done the vlan index which has been configed
*   
*
*output params:none
*    DEFUN(config_fdb_static_delete_vlan_cmd_func,
	config_fdb_static_delete_vlan_cmd,
	"delete fdb static vlan <1-4094>",
	"Config system information\n"
	"delete FDB table \n"
	"delete FDB static table\n"
	"FDB table vlan Range <1-4094>\n"
	"config Fdb vlan value\n"
) 
****************************************/

int delete_fdb_static_by_vlanid(char * vlan_id)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned short vlanid = 0;
	unsigned int 	op_ret = 0;

	op_ret = parse_short_parse((char *)vlan_id,&vlanid);
	if (NPD_FAIL == op_ret) {
    	//vty_out(vty,"FDB agingtime form erro!\n");
		return -2;
	}
	 if (vlanid <MIN_VLANID||vlanid>MAX_VLANID){
		//vty_out(vty,"FDB vlan outrang!\n");
		return -3;
		}
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_STATIC_DELETE_WITH_VLAN );
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
							 	DBUS_TYPE_UINT16,&vlanid,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}

	if (!(dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))) {
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_FAILURE;
			
	} 
	else {
		//dcli_fdb_output(op_ret);
		return op_ret;
		}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

/**************************************
*delete fdb item which belong to the port on Both Sw & Hw. 
*
* Usage:delete fdb static port PORTNO 
*
*input Params: 
*	
*		PORTNO:  destination port, form slot/port
*   
*
*output params:none
*    DEFUN(config_fdb_static_delete_port_cmd_func,
	config_fdb_static_delete_port_cmd,
	"delete fdb static port PORTNO",
	"Config system information\n"
	"delete FDB table \n"
	"delete FDB static table \n"
	"FDB table port\n"
	CONFIG_ETHPORT_STR
) 
****************************************/

int delete_fdb_static_by_port(char *portno)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned char slotNum = 0,portNum = 0;
	unsigned int 	op_ret = 0;
	op_ret =  parse_slotport_no((char*)portno,&slotNum,&portNum);
	if (NPD_FAIL == op_ret) {
		//vty_out(vty,"Unknow portno format!\n");
		return -2;
	}
	if((slotNum <MIN_SLOT ||slotNum>MAX_SLOT)||(portNum <MIN_PORT||portNum>MAX_PORT)){
		//vty_out(vty,"err port outrange!\n");
		return -3;
		}
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_STATIC_DELETE_WITH_PORT);
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
							 	DBUS_TYPE_BYTE,&slotNum,
							 	DBUS_TYPE_BYTE,&portNum,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}

	if (!(dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))) {
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_FAILURE;
			
	} 
	else {
		//dcli_fdb_output(op_ret);
		return op_ret;
		}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}




/**************************************
*config specify fdb item deny to forward base on (dmac|smac) on Both Sw & Hw. 
*
* Usage:create fdb blacklist (dmac|smac) MAC vlan VLANNAME" 
*
*input Params: 
*	(dmac|smac): two chose one param( destination mar or source mac) mac form 00:00:11:11:aa:aa
*	VLANNAME:  before done the vlan name which has been configed ,
*                           VLANNAME must be to begin with letter or '_'
*  
*
*output params:none
*   DEFUN(config_fdb_mac_vlanname_match_drop_name_cmd_func,
	config_fdb_mac_vlan_name_match_cmd,
	"create fdb blacklist (dmac|smac) MAC vlan VLANNAME",
	"Config system information\n"
	"config FDB table make match drop command!\n"
	"config FDB table\n"
	"config FDB table dmac match into blacklist \n"
	"config FDB table smac match into blacklist \n"
	"config MAC address in Hexa.Eg 00:00:11:11:aa:aa\n"
	"config FDB table vlanname\n"
	"config VLANNAME must be to begin with letter or '_'\n"
)  
****************************************/
/* checked 08-12-15 */
int create_fdb_blacklist_by_vlanname(char *mactype, char *mac, char *vlan_name)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	int ret = 0;
	unsigned char   len = 0;
	unsigned int    flag = 0;

	char* vlanname= NULL;
	ETHERADDR		macAddr;
	char* string="dmac";
	memset(&macAddr,0,sizeof(ETHERADDR));

	len = strlen((char*)mactype);
	if((len>4)||(0 == len)){
		return -6;
	}
	else{
		ret=strncmp(string,(char*)mactype,len);
		if(ret==0){
			flag =1;
		}
		else if(ret<0){
			flag= 0;
		}
		else{
           return -6;
		}
	}

	ret = parse_mac_addr((char *)mac,&macAddr);
	if (NPD_FAIL == ret) {
    	//vty_out(vty,"Unknow mac addr format!\n");
		return -2;
	}

	ret=is_muti_brc_mac(&macAddr);
	if(ret==1){
		//vty_out(vty,"erro:input broadcast or multicast mac!\n");
		return -3;
		}
		

   ret =parse_vlan_string((char*)vlan_name);

   if (NPD_FAIL == ret) {
    	//vty_out(vty,"vlanname form erro!\n");
		return -4;
	}

   vlanname= (char *)vlan_name;
	
 	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_DROP_WITH_NAME);
	
	dbus_error_init(&err);
	
	dbus_message_append_args(	query,								
	                            DBUS_TYPE_UINT32,&flag,								
							 	DBUS_TYPE_STRING,&vlanname,
							 	DBUS_TYPE_BYTE,&macAddr.arEther[0],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[1],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[2],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[3],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[4],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[5],
							 	DBUS_TYPE_INVALID);
	
		
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
			//vty_out(vty,"failed get reply.\n");
			if (dbus_error_is_set(&err)) {
				//vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free(&err);
			}
			return CMD_FAILURE;
	}
	
	else{
			if (!(dbus_message_get_args ( reply, &err,
											DBUS_TYPE_UINT32,&ret,
											DBUS_TYPE_INVALID))) {
				
					//vty_out(vty,"Failed get args.\n");
					if (dbus_error_is_set(&err)) {
							//vty_out(vty,"%s raised: %s",err.name,err.message);
							dbus_error_free(&err);
					}
					return CMD_FAILURE;
			}
			else{
				//dcli_fdb_output(ret);
				return ret;
				}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

/**************************************
*config specify fdb item deny to forward base on (dmac|smac) on Both Sw & Hw. 
*
* Usage:create fdb blacklist (dmac|smac) MAC vlan <1-4094> 
*
*input Params: 
*	(dmac|smac): two chose one param( destination mar or source mac) mac form 00:00:11:11:aa:aa
*	<1-4094>: valid vlan index  range, before done the vlan index which has been configed 
*   
*
*output params:none
* DEFUN(config_fdb_mac_vlan_match_drop_cmd_func,
	config_fdb_mac_vlan_match_cmd,
	"create fdb blacklist (dmac|smac) MAC vlan <1-4094>",
	"Config system information\n"
	"config FDB table make match  drop command!\n"
	"config FDB table\n"
	"config FDB table match dMAC into blacklist \n"
	"config FDB table match sMAC into blacklist \n"
	"config MAC address in Hexa.Eg 00:00:11:11:aa:aa\n"
	"config FDB table vlan Id\n"
	"config VLAN Id valid range <1-4094>\n"
)    
****************************************/
/* checked 08-12-15 */
int create_fdb_blacklist_by_vid(char * mactype,char * mac,char * vlanid)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	ETHERADDR		macAddr;
	
	unsigned short	vlanId = 0;
	int 	op_ret = 0;
	unsigned char   len = 0;
	unsigned int    flag = 0;
	
	char*  string ="dmac";
	memset(&macAddr,0,sizeof(ETHERADDR));
	
	len = strlen((char*)mactype);
	if((len>4)||(0 == len)){
		return -6;
	}
	else{
		op_ret=strncmp(string,(char*)mactype,len);
		if(op_ret==0){
			flag =1;
		}
		else if(op_ret < 0){
			flag= 0;
		}
		else{
           return -6;
		}
	}

	op_ret = parse_mac_addr((char *)mac,&macAddr);
	if (NPD_FAIL == op_ret) {
    	//vty_out(vty,"Unknow mac addr format!\n");
		return -2;
	}
	
	op_ret=is_muti_brc_mac(&macAddr);
	if(op_ret==1){
		//vty_out(vty,"erro:input broadcast or multicast mac!\n");
		return -3;
		}
		
	
	op_ret = parse_short_parse((char*)vlanid, &vlanId);
	if (NPD_FAIL == op_ret) {
    	//vty_out(vty,"Unknow vlan id format!\n");
		return -4;
	}
    if (vlanId<MIN_VLANID || vlanId>MAX_VLANID){
	//vty_out(vty,"FDB vlan outrang!\n");
		return -5;
	}

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_DROP);
	
	
	dbus_error_init(&err);
	dbus_message_append_args(	query,								
	                            DBUS_TYPE_UINT32,&flag,								
							 	DBUS_TYPE_UINT16,&vlanId,
							 	DBUS_TYPE_BYTE,&macAddr.arEther[0],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[1],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[2],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[3],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[4],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[5],
							 	DBUS_TYPE_INVALID);
	
		
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}
	

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)) {
		//dcli_fdb_output(op_ret);
		return op_ret;
		}
	else {
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}
	
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

/**************************************
*delete specify static fdb item on Both Sw & Hw. 
*
* Usage:delete fdb static mac MAC vlan VLANNAME 
*
*input Params: 
*	(dmac|smac): two chose one param( destination mar or source mac) mac form 00:00:11:11:aa:aa
*	VLANNAME:  before done the vlan name which has been configed ,
*                           VLANNAME must be to begin with letter or '_'
*   
*
*output params:none
*  DEFUN(config_fdb_mac_vlan_no_static_port_name_cmd_func,
	config_fdb_mac_vlan_no_static_port_name_cmd,
	"delete fdb static mac MAC vlan VLANNAME ",
	"Config system information\n"
	"delete FDB table\n"
	"find static fdb item\n"
	"config FDB table MAC field.\n"
	"config MAC address in Hexa.Eg 00:00:11:11:aa:aa\n"
	"config FDB table vlanname\n"
	"config VLANNAME must be to begin with letter or '_'\n"
)   
****************************************/

int delete_fdb_static_by_vlanname(char *mac, char *vlan_name)
{
	DBusMessage *query, *reply;
	ETHERADDR		macAddr;
	char*	vlanname=NULL; 
	DBusError err;
	unsigned int 	ret = 0;
	memset(&macAddr,0,sizeof(ETHERADDR));
	
	ret = parse_mac_addr((char *)mac,&macAddr);
	if (NPD_FAIL == ret) {
    	//vty_out(vty,"Unknow mac addr format!\n");
		return -2;
	}
	
	ret=is_muti_brc_mac(&macAddr);
	if(ret==1){
		//vty_out(vty,"erro:input broadcast or multicast mac!\n");
		return -3;
		}
		
	
	ret =parse_vlan_string((char*)vlan_name);
	
	   if (NPD_FAIL == ret) {
			//vty_out(vty,"vlanname form erro!\n");
			return -4;
		}
	
	   vlanname= (char *)vlan_name;

	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_NO_STATIC_WITH_NAME);
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
								//DBUS_TYPE_STRING,&vlanname,
							 	DBUS_TYPE_BYTE,&macAddr.arEther[0],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[1],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[2],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[3],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[4],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[5],
							 	DBUS_TYPE_STRING,&vlanname,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}
	else{
			if (dbus_message_get_args ( reply, &err,
				DBUS_TYPE_UINT32,&ret,
				
				DBUS_TYPE_INVALID)) {
				//dcli_fdb_output(ret);
				return ret;
				} else {
				//vty_out(vty,"Failed get args.\n");
				if (dbus_error_is_set(&err)) {
					//vty_out(vty,"%s raised: %s",err.name,err.message);
					dbus_error_free(&err);
				}
				return CMD_FAILURE;
			}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

/**************************************
*delete specify static fdb item on Both Sw & Hw. 
*
* Usage:delete fdb static mac MAC vlan <1-4094> 
*
*input Params: 
*	(dmac|smac): two chose one param( destination mar or source mac) mac form 00:00:11:11:aa:aa
*	<1-4094>:  valid vlan index  range, before done the vlan index which has been configed
*   
*
*output params:none
*  DEFUN(config_fdb_mac_vlan_no_static_port_cmd_func,
	config_fdb_mac_vlan_no_static_port_cmd,
	"delete fdb static mac MAC vlan <1-4094> ",
	"Config system information\n"
	"delete FDB table\n"
	"find static fdb item\n"
	"config FDB table MAC field.\n"
	"config MAC address in Hexa.Eg 00:00:11:11:aa:aa\n"
	"config FDB table vlan Id\n"
	"config VLAN Id valid range <1-4094>\n"
)   
****************************************/

int delete_fdb_static_by_vid(char *mac, char *vlanid)
{
	DBusMessage *query, *reply;
	ETHERADDR		macAddr;
	unsigned short	vlanId = 0;
	
	DBusError err;
	unsigned int 	op_ret = 0;

	memset(&macAddr,0,sizeof(ETHERADDR));
	
	op_ret = parse_mac_addr((char *)mac,&macAddr);
	if (NPD_FAIL == op_ret) {
    	//vty_out(vty,"Unknow mac addr format!\n");
		return -2;

	}
	

	op_ret=is_muti_brc_mac(&macAddr);
	if(op_ret==1){
		//vty_out(vty,"erro:input broadcast or multicast mac!\n");
		return -3;
		}
		

	op_ret = parse_short_parse((char*)vlanid, &vlanId);
	if (NPD_FAIL == op_ret) {
    	//vty_out(vty,"Unknow vlan id format!\n");
		return -4;
	}
    if (vlanId <MIN_VLANID||vlanId>MAX_VLANID){
		//vty_out(vty,"FDB vlan outrang!\n");
		return -5;
		}

	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_NO_STATIC);
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
								DBUS_TYPE_UINT16,&vlanId,
							 	DBUS_TYPE_BYTE,&macAddr.arEther[0],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[1],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[2],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[3],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[4],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[5],
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}
	else{
			if (dbus_message_get_args ( reply, &err,
				DBUS_TYPE_UINT32,&op_ret,
				DBUS_TYPE_INVALID)) {
				//dcli_fdb_output(op_ret);
				return op_ret;
				} else {
				//vty_out(vty,"Failed get args.\n");
				if (dbus_error_is_set(&err)) {
					//vty_out(vty,"%s raised: %s",err.name,err.message);
					dbus_error_free(&err);
				}
				return CMD_FAILURE;
			}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

/**************************************
*delete fdb item which belong to the vlan on Both Sw & Hw. 
*
* Usage:delete fdb vlan <1-4094> 
*
*input Params: 
*	
*	<1-4094>:  valid vlan index  range, before done the vlan index which has been configed
*   
*
*output params:none
*  DEFUN(config_fdb_delete_vlan_cmd_func,
	config_fdb_delete_vlan_cmd,
	"delete fdb vlan <1-4094>",
	"Config system information\n"
	"delete FDB table \n"
	"FDB table vlan Range <1-4094s>\n"
	"config Fdb vlan value\n"
)   
****************************************/

int delete_fdb_by_vid(char *vlan_id)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned short vlanid = 0;
	unsigned int 	op_ret = 0;

	op_ret = parse_short_parse((char *)vlan_id,&vlanid);
	if (NPD_FAIL == op_ret) {
    	//vty_out(vty,"FDB agingtime form erro!\n");
		return CMD_FAILURE;
	}
	 if (vlanid <MIN_VLANID||vlanid>MAX_VLANID){
		//vty_out(vty,"FDB vlan outrang!\n");
		return -5;
		}
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_DELETE_WITH_VLAN );
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
							 	DBUS_TYPE_UINT16,&vlanid,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}

	if (!(dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))) {
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_FAILURE;
			
			
	} 
	else {
		   if(NPD_FDB_ERR_VLAN_NONEXIST == op_ret){
				//vty_out(vty,"%% Error,the vlan input not exist!\n");
				return NPD_FDB_ERR_VLAN_NONEXIST;
			}
			else if(NPD_FDB_ERR_OCCUR_HW == op_ret){
				//vty_out(vty,"%% Error,there is something wrong when deleting.\n");
				return CMD_FAILURE;
			}
		}

	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

/**************************************
*delete fdb item which belong to the port on Both Sw & Hw. 
*
* Usage:delete fdb port PORTNO 
*
*input Params: 
*	
*		PORTNO:  destination port, form slot/port
*   
*
*output params:none
*   DEFUN(config_fdb_delete_port_cmd_func,
	config_fdb_delete_port_cmd,
	"delete fdb port PORTNO",
	"Config system information\n"
	"delete FDB table \n"
	"FDB table port\n"
	CONFIG_ETHPORT_STR
)  
****************************************/

int delete_fdb_by_port(char *portno)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned char devNum = 0,portNum = 0;
	unsigned int 	op_ret = 0;
	op_ret =  parse_slotport_no((char*)portno,&devNum,&portNum);
	if (NPD_FAIL == op_ret) {
		//vty_out(vty,"Unknow portno format!\n");
		return CMD_FAILURE;
	}
	/*if((devNum <MIN_SLOT ||devNum>MAX_SLOT)||(portNum <MIN_PORT||portNum>MAX_PORT)){
		//vty_out(vty,"err port outrange!\n");
		return CMD_FAILURE;
		}
	*/
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_DELETE_WITH_PORT);
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
							 	DBUS_TYPE_BYTE,&devNum,
							 	DBUS_TYPE_BYTE,&portNum,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}

	if (!(dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))) {
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_FAILURE;
			
	} 
	else {
           if(NPD_FDB_ERR_BADPARA == op_ret){
				//vty_out(vty,"%% Error,the parameter input is error!\n");
				return -3;
            }
			else if(NPD_FDB_ERR_OCCUR_HW == op_ret){
				//vty_out(vty,"%% Error,there is something wrong when configuration hw.\n");
				return CMD_FAILURE;
			}
		}

	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

//add 2009-07-02 ** delete fdb by trunk id
int delete_fdb_by_trunk(char *trunk_id)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned short trunk_no;
	unsigned int 	op_ret;

	op_ret = parse_short_parse((char *)trunk_id,&trunk_no);
	
	if (NPD_FAIL == op_ret) {
		//vty_out(vty,"% Bad Parameters,Unknow portno format!\n");
		//return CMD_SUCCESS;
		return CMD_FAILURE;
	}
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_DELETE_WITH_TRUNK);
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
							 	DBUS_TYPE_UINT16,&trunk_no,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}

	if (!(dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))) {
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
			
			
	} 
	else {
		   if( NPD_FDB_ERR_GENERAL== op_ret){
                //vty_out(vty,"%% Error,operations error when configuration!\n");
                return CMD_FAILURE;
		    }

			else if(NPD_FDB_ERR_OCCUR_HW == op_ret){
				//vty_out(vty,"%% Error,there is something wrong when configuration hw.\n");
				return CMD_FAILURE;
			}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

//show fdb
int show_fdb_by_mac_vid
(
char *mac,
char *vlan_id,
struct fdb_mac_vid *macvid
)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	unsigned int dnumber = 0;
	unsigned int dcli_flag = 0;
	unsigned char  show_mac[6] ={0};
	unsigned short vlanid = 0;
	unsigned int trans_value1 = 0;
	unsigned int trans_value2 = 0;
	ETHERADDR		macAddr;
	unsigned short	vlanId = 0;
	unsigned int	op_ret = 0;

    memset(&macAddr,0,sizeof(ETHERADDR));
	
	/*fetch the 1st param : MAC addr*/
	op_ret = parse_mac_addr((char *)mac,&macAddr);
	
	
	if (NPD_FAIL == op_ret) {
		//vty_out(vty,"Unknow mac addr format.\n");
		return -3;
	}
	
	op_ret = parse_short_parse((char*)vlan_id, &vlanId);
	if (NPD_FAIL == op_ret) {
		//vty_out(vty,"Unknow vlan id format.\n");
		return -4;
	}
	if(vlanId <MIN_VLANID ||vlanId>MAX_VLANID){
   		//vty_out(vty,"FDB vlan outrang!\n");
		return -5;
	}


	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,	\
								NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_SHOW_FDB_TABLE_ONE);

	dbus_error_init(&err);

	dbus_message_append_args(	query,
							DBUS_TYPE_UINT16,&vlanId,
						 	DBUS_TYPE_BYTE,&macAddr.arEther[0],
						 	DBUS_TYPE_BYTE,&macAddr.arEther[1],
						 	DBUS_TYPE_BYTE,&macAddr.arEther[2],
						 	DBUS_TYPE_BYTE,&macAddr.arEther[3],
						 	DBUS_TYPE_BYTE,&macAddr.arEther[4],
						 	DBUS_TYPE_BYTE,&macAddr.arEther[5],						 	
						 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&op_ret);
		
		 if(NPD_FDB_ERR_NONE != op_ret){
			if( NPD_FDB_ERR_GENERAL== op_ret){
				//ShowAlert(search(lcontrol,"oper_fail"));
				return 1;
			}
			else if(NPD_FDB_ERR_VLAN_NONEXIST == op_ret){
				//ShowAlert(search(lcontrol,"vlan_not_exist"));
				return 2;
			}
			else if(NPD_FDB_ERR_BADPARA == op_ret){
				//ShowAlert(search(lcontrol,"illegal_input"));
				return 3;
			}
			else if(NPD_FDB_ERR_OCCUR_HW == op_ret){
				//ShowAlert(search(lcontrol,"HW_error"));
				return 4;
			}
			else if(NPD_FDB_ERR_SYSTEM_MAC == op_ret){
				//ShowAlert(search(lcontrol,"mac_confilt"));
				return 5;
			}
		    dbus_message_unref(reply);
	        return CMD_SUCCESS;
		}
		
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&dnumber);
		if (dnumber == 0){
			//vty_out(vty,"THE ITEM IS NONE IN FDB TABLE !\n");
			return -2;//THE ITEM IS NONE IN FDB TABLE !
		}
		
		//vty_out(vty,"%-17s  %-8s%-11s%-8s%-8s%-6s\n","=================",	"======", "=========","======","======","======");
		//vty_out(vty,"%-17s  %-8s%-11s%-8s%-8s%-6s\n","MAC","VLAN","SLOT/PORT","TRUNK","VID","VIDX");
		//vty_out(vty,"%-17s  %-8s%-11s%-8s%-8s%-6s\n","=================",	"======", "=========","======","======","======");
		
		int cl = 1;
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&dcli_flag);

		macvid->dcli_flag=dcli_flag;
		
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&vlanid);

		macvid->vlanid=vlanid;
		
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&trans_value1);

		macvid->trans_value1=trans_value1;
		
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&trans_value2);

		macvid->trans_value2=trans_value2;		

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&show_mac[0]);

		macvid->show_mac[0]=show_mac[0];
		
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&show_mac[1]);

		macvid->show_mac[1]=show_mac[1];

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&show_mac[2]);

		macvid->show_mac[2]=show_mac[2];
		
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&show_mac[3]);

		macvid->show_mac[3]=show_mac[3];
		
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&show_mac[4]);

		macvid->show_mac[4]=show_mac[4];
		
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&show_mac[5]);

		macvid->show_mac[5]=show_mac[5];
		
		
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

/**************************************
*display valid fdb item count on hw . 
*
* Usage:show fdb count 
*
*input Params: none
*
*output params:
*	fdb valid count.
*  DEFUN(show_fdb_count_cmd_func,
	show_fdb_count_cmd,
	"show fdb count" ,
	"Show system information\n"
	"FDB table content!\n"
	"show fdb valid count!\n"
)   
****************************************/

int get_fdb_count()
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int dnumber = 0;
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,	\
									NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_SHOW_FDB_TABLE_COUNT);
	
	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&dnumber,
		DBUS_TYPE_INVALID)) {
		       if(dnumber==0){
				//vty_out(vty,"FDB TABLE IS NONE!\n");
				return 0;
				}
				//vty_out(vty,"FDB Table count %d.\n",dnumber);
				return dnumber;
				
			} else {
			
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}
	
	dbus_message_unref(reply);
	return CMD_FAILURE;
}




/**************************************
单一函数，可直接修改 链表形式
****************************************/

int show_fdb
(
struct fdb_all *head,
int *fdb_num
)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	unsigned int dnumber = 0;
	unsigned int dcli_flag =0;
	unsigned int type_flag = 0;
	char* type = NULL;//unsigned 
	
	unsigned char  show_mac[6] = {0};
	unsigned short vlanid=0;
	
	unsigned int i=0;	
	
	struct fdb_all *q,*tail;
   

	unsigned int trans_value1 = 0;
	unsigned int trans_value2 = 0;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,	\
								NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_SHOW_FDB_TABLE);
	
	dbus_error_init(&err);

	
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&dnumber);
	if (dnumber == 0){
		//vty_out(vty,"FDB TABLE IS NONE!\n");
		return -2;
		}
	// NOTICE We used 68 bytes here, we still got 12 bytes of summary info
	//vty_out(vty,"%-17s  %-8s%-11s%-8s%-8s%-6s%-8s\n","=================",	"======", "=========","======","======","======"," ======");
	//vty_out(vty,"%-17s  %-8s%-11s%-8s%-8s%-6s%-8s\n","MAC","VLAN","SLOT/PORT","TRUNK","VID","VIDX","TYPE");
	//vty_out(vty,"%-17s  %-8s%-11s%-8s%-8s%-6s%-8s\n","=================",	"======", "=========","======","======","======"," ======");

	int cl = 1;
	dbus_message_iter_next(&iter);
	
	dbus_message_iter_recurse(&iter,&iter_array);
	
    /*---------- b b -------------------------*/

    head->next=NULL;
	tail=head;
	*fdb_num=dnumber;
    
	
  	for (i=0 ; dnumber > i; i++){

	    //分配空间
	    q=(struct fdb_all *)malloc(sizeof(struct fdb_all));
		if ( NULL == q)
			{
			return 0;
			}
		
		/*获得所有的信息*/
		DBusMessageIter iter_struct;
		
		dbus_message_iter_recurse(&iter_array,&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&dcli_flag);
		dbus_message_iter_next(&iter_struct);

		q->dcli_flag=dcli_flag;
		
		dbus_message_iter_get_basic(&iter_struct,&vlanid);
		dbus_message_iter_next(&iter_struct);

		q->vlanid=vlanid;
		
		dbus_message_iter_get_basic(&iter_struct,&trans_value1);
		dbus_message_iter_next(&iter_struct);

		q->trans_value1=trans_value1;
		
		dbus_message_iter_get_basic(&iter_struct,&trans_value2);
		dbus_message_iter_next(&iter_struct);

		q->trans_value2=trans_value2;
		
		dbus_message_iter_get_basic(&iter_struct,&type_flag);
		dbus_message_iter_next(&iter_struct);

		q->type_flag=type_flag;
		
		dbus_message_iter_get_basic(&iter_struct,&show_mac[0]);
		dbus_message_iter_next(&iter_struct);

		q->show_mac[0]=show_mac[0];
		
		dbus_message_iter_get_basic(&iter_struct,&show_mac[1]);
		dbus_message_iter_next(&iter_struct);

		q->show_mac[1]=show_mac[1];
		
		dbus_message_iter_get_basic(&iter_struct,&show_mac[2]);
		dbus_message_iter_next(&iter_struct);

		q->show_mac[2]=show_mac[2];
		
		dbus_message_iter_get_basic(&iter_struct,&show_mac[3]);
		dbus_message_iter_next(&iter_struct);

		q->show_mac[3]=show_mac[3];
		
		dbus_message_iter_get_basic(&iter_struct,&show_mac[4]);
		dbus_message_iter_next(&iter_struct);

		q->show_mac[4]=show_mac[4];
		
		dbus_message_iter_get_basic(&iter_struct,&show_mac[5]);

		q->show_mac[5]=show_mac[5];

		if( 0 ==type_flag){
            type = "DYNAMIC";
			q->type=type;
		}
		else if(1 ==type_flag){
			type = "STATIC";
			q->type=type;

		}
		else if(2 ==type_flag){
			type = "BLACKLIST";
			q->type=type;

		}
		else {
			//vty_out (vty,"sorry fdb item type wrong !\n");
			return -1;
		}
        
		
		
	


/////////////////////////////////

		//else {
			//vty_out (vty,"sorry interface type wrong !\n");
		//	return -1;
		//}


			
		/*---------------  end display  --------------------------*/

		
		dbus_message_iter_next(&iter_array);
		cl = !cl;

		q->next=NULL;
		tail->next=q;
		tail=q;
		
		}
		
/*-----------    end for  ----------------------------------*/


	dbus_message_unref(reply);

	return CMD_SUCCESS;	
	
}


//////释放链表


void Free_fdb_all(struct fdb_all *head)
{
    struct fdb_all *f1,*f2;
	f1=head->next;		 
	f2=f1->next;
	while(f2!=NULL)
	{
	  free(f1);
	  f1=f2;
	  f2=f2->next;
	}
	free(f1);
}



//////释放链表


void Free_fdb_blacklist(struct fdb_blacklist *head)
{
    struct fdb_blacklist *f1,*f2;
	f1=head->next;		 
	f2=f1->next;
	while(f2!=NULL)
	{
	  free(f1);
	  f1=f2;
	  f2=f2->next;
	}
	free(f1);
}

void Free_fdb_limit(struct fdb_limit *head)
{
    struct fdb_limit *f1,*f2;
	f1=head->next;		 
	f2=f1->next;
	while(f2!=NULL)
	{
	  free(f1);
	  f1=f2;
	  f2=f2->next;
	}
	free(f1);
}

//重新写 链表存储函数
int show_fdb_dynamic
(
struct fdb_all *head,
int *fdb_num
)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	unsigned int dnumber = 0;
	unsigned int dcli_flag= 0;
	
	unsigned char  show_mac[6];
	unsigned short vlanid = 0;
	unsigned int i=0;
	unsigned int trans_value1 = 0;
	unsigned int trans_value2 = 0;

	struct fdb_all *q,*tail;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,	\
								NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_SHOW_FDB_DYNAMIC_TABLE);
	
	dbus_error_init(&err);

	
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&dnumber);
	if (dnumber == 0){
		//vty_out(vty,"FDB TABLE IS NONE!\n");
		return -2;
		}
	// NOTICE We used 68 bytes here, we still got 12 bytes of summary info
	//vty_out(vty,"%-17s  %-8s%-11s%-8s%-8s%-6s\n","=================",	"======", "=========","======","======","======");
	//vty_out(vty,"%-17s  %-8s%-11s%-8s%-8s%-6s\n","MAC","VLAN","SLOT/PORT","TRUNK","VID","VIDX");
	//vty_out(vty,"%-17s  %-8s%-11s%-8s%-8s%-6s\n","=================",	"======", "=========","======","======","======");
	
	int cl = 1;
	dbus_message_iter_next(&iter);
	
	dbus_message_iter_recurse(&iter,&iter_array);

	 head->next=NULL;
     tail=head;
	 *fdb_num=dnumber;
	 
	for ( ; dnumber > i; i++){

		//分配空间
		q=(struct fdb_all *)malloc(sizeof(struct fdb_all));
		if( NULL ==q )
			{
			return 0;
			}
		
		DBusMessageIter iter_struct;
		
		dbus_message_iter_recurse(&iter_array,&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&dcli_flag);
		dbus_message_iter_next(&iter_struct);

		q->dcli_flag=dcli_flag;
		
		dbus_message_iter_get_basic(&iter_struct,&vlanid);
		dbus_message_iter_next(&iter_struct);

		q->vlanid=vlanid;
		
		dbus_message_iter_get_basic(&iter_struct,&trans_value1);
		dbus_message_iter_next(&iter_struct);

		q->trans_value1=trans_value1;
		
		dbus_message_iter_get_basic(&iter_struct,&trans_value2);
		dbus_message_iter_next(&iter_struct);

		q->trans_value2=trans_value2;
		
		dbus_message_iter_get_basic(&iter_struct,&show_mac[0]);
		dbus_message_iter_next(&iter_struct);

		q->show_mac[0]=show_mac[0];

		dbus_message_iter_get_basic(&iter_struct,&show_mac[1]);
		dbus_message_iter_next(&iter_struct);

		q->show_mac[1]=show_mac[1];
		
		dbus_message_iter_get_basic(&iter_struct,&show_mac[2]);
		dbus_message_iter_next(&iter_struct);

		q->show_mac[2]=show_mac[2];
		
		dbus_message_iter_get_basic(&iter_struct,&show_mac[3]);
		dbus_message_iter_next(&iter_struct);

		q->show_mac[3]=show_mac[3];
		
		dbus_message_iter_get_basic(&iter_struct,&show_mac[4]);
		dbus_message_iter_next(&iter_struct);

		q->show_mac[4]=show_mac[4];
		
		dbus_message_iter_get_basic(&iter_struct,&show_mac[5]);

		q->show_mac[5]=show_mac[5];

		q->next=NULL;
		tail->next=q;
		tail=q;
		
	
		
		dbus_message_iter_next(&iter_array);
		cl = !cl;
		}
	
	dbus_message_unref(reply);

	return CMD_SUCCESS;	
}




/**************************************
新的改写函数，用链表存储，
****************************************/

int show_fdb_static
(
struct fdb_all *head,
int *fdb_num
)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	unsigned int dnumber = 0;
	unsigned int dcli_flag = 0;

	struct fdb_all *q,*tail;

	
	unsigned char  show_mac[6];
	unsigned short vlanid;
	unsigned int i=0;
	unsigned int trans_value1 = 0;
	unsigned int trans_value2 = 0;
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_SHOW_FDB_STATIC_TABLE);
	
	dbus_error_init(&err);

	
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&dnumber);
	if (dnumber == 0){
		//vty_out(vty,"FDB ITEM IS NONE!\n");
		return -2;
		}
	// NOTICE We used 68 bytes here, we still got 12 bytes of summary info
	//vty_out(vty,"%-17s  %-8s%-11s%-8s%-8s%-6s\n","=================",	"======", "=========","======","======","======");
	//vty_out(vty,"%-17s  %-8s%-11s%-8s%-8s%-6s\n","MAC","VLAN","SLOT/PORT","TRUNK","VID","VIDX");
	//vty_out(vty,"%-17s  %-8s%-11s%-8s%-8s%-6s\n","=================",	"======", "=========","======","======","======");

	dbus_message_iter_next(&iter);
	
	dbus_message_iter_recurse(&iter,&iter_array);


   /*---------- b b -------------------------*/
     head->next=NULL;
     tail=head;
	 *fdb_num=dnumber;
    	
	
	for ( ; dnumber > i; i++){

		//分配空间
		q=(struct fdb_all *)malloc(sizeof(struct fdb_all));
		if( NULL ==q )
			{
			return 0;
			}
		
		DBusMessageIter iter_struct;
		
		dbus_message_iter_recurse(&iter_array,&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&dcli_flag);
		dbus_message_iter_next(&iter_struct);

		q->dcli_flag=dcli_flag;
		
		dbus_message_iter_get_basic(&iter_struct,&vlanid);
		dbus_message_iter_next(&iter_struct);

		q->vlanid=vlanid;
		
		dbus_message_iter_get_basic(&iter_struct,&trans_value1);
		dbus_message_iter_next(&iter_struct);

		q->trans_value1=trans_value1;
		
		dbus_message_iter_get_basic(&iter_struct,&trans_value2);
		dbus_message_iter_next(&iter_struct);

		q->trans_value2=trans_value2;
		
		dbus_message_iter_get_basic(&iter_struct,&show_mac[0]);
		dbus_message_iter_next(&iter_struct);

		q->show_mac[0]=show_mac[0];
		
		dbus_message_iter_get_basic(&iter_struct,&show_mac[1]);
		dbus_message_iter_next(&iter_struct);

		q->show_mac[1]=show_mac[1];
		
		dbus_message_iter_get_basic(&iter_struct,&show_mac[2]);
		dbus_message_iter_next(&iter_struct);

        q->show_mac[2]=show_mac[2];
		
		dbus_message_iter_get_basic(&iter_struct,&show_mac[3]);
		dbus_message_iter_next(&iter_struct);

		q->show_mac[3]=show_mac[3];
		
		dbus_message_iter_get_basic(&iter_struct,&show_mac[4]);
		dbus_message_iter_next(&iter_struct);

		q->show_mac[4]=show_mac[4];
		
		dbus_message_iter_get_basic(&iter_struct,&show_mac[5]);
		q->show_mac[5]=show_mac[5];
		

		

		q->next=NULL;
		tail->next=q;
		tail=q;
		
		dbus_message_iter_next(&iter_array);
		
			}
	
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}	

//重写函数 并在页面实行分页
int show_fdb_blacklist
(
struct fdb_blacklist *head,
int *bl_num
)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	unsigned int dnumber = 0;
    unsigned char dmac = 0;
	unsigned char smac = 0;
	
	unsigned char  show_mac[6];
	unsigned short vlanid = 0;
	unsigned int i=0;

	struct fdb_blacklist *q,*tail;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_SHOW_FDB_BLACKLIST_TABLE);
	
	dbus_error_init(&err);

	
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&dnumber);
	if (dnumber == 0){
		//vty_out(vty,"FDB ITEM IS NONE!\n");
		return -2;
	}
	
	// NOTICE We used 68 bytes here, we still got 12 bytes of summary info
	//vty_out(vty,"%-17s  %-8s%-8s%-8s\n","=================",	"======", "=========", "=========");
	//vty_out(vty,"%-17s  %-8s%-8s%-8s\n","MAC","VLAN","DMAC","SMAC");
	//vty_out(vty,"%-17s  %-8s%-8s%-8s\n","=================",	"======", "=========", "=========");
	
	dbus_message_iter_next(&iter);
	
	dbus_message_iter_recurse(&iter,&iter_array);

	 head->next=NULL;
     tail=head;
	 *bl_num=dnumber;
	 
	for ( ; dnumber > i; i++){

		//分配空间
		q=(struct fdb_blacklist *)malloc(sizeof(struct fdb_blacklist));
		if( NULL ==q )
			{
			return 0;
			}
		DBusMessageIter iter_struct;
		
		dbus_message_iter_recurse(&iter_array,&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&dmac);
		dbus_message_iter_next(&iter_struct);

		q->dmac=dmac;
		
		dbus_message_iter_get_basic(&iter_struct,&smac);
		dbus_message_iter_next(&iter_struct);

		q->smac=smac;
		
		dbus_message_iter_get_basic(&iter_struct,&vlanid);
		dbus_message_iter_next(&iter_struct);

		q->vlanid=vlanid;
		
		dbus_message_iter_get_basic(&iter_struct,&show_mac[0]);
		dbus_message_iter_next(&iter_struct);

		q->show_mac[0]=show_mac[0];
		
		dbus_message_iter_get_basic(&iter_struct,&show_mac[1]);
		dbus_message_iter_next(&iter_struct);

		q->show_mac[1]=show_mac[1];
			
		dbus_message_iter_get_basic(&iter_struct,&show_mac[2]);
		dbus_message_iter_next(&iter_struct);
		
		q->show_mac[2]=show_mac[2];
		
		dbus_message_iter_get_basic(&iter_struct,&show_mac[3]);
		dbus_message_iter_next(&iter_struct);
		
		q->show_mac[3]=show_mac[3];
		
		dbus_message_iter_get_basic(&iter_struct,&show_mac[4]);
		dbus_message_iter_next(&iter_struct);

		q->show_mac[4]=show_mac[4];
		
		dbus_message_iter_get_basic(&iter_struct,&show_mac[5]);

		q->show_mac[5]=show_mac[5];	


	    q->next=NULL;
		tail->next=q;
		tail=q;
		
		dbus_message_iter_next(&iter_array);
		
	}
	
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}	


/**************************************
重写函数
****************************************/

int show_fdb_by_port
(
struct fdb_all *head,
int *p_num,
char *portno
)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	unsigned int dnumber = 0;
	unsigned int dcli_flag = 0;
	
	unsigned char  show_mac[6] ={0};
	unsigned short vlanid = 0;
	unsigned int i=0;
	unsigned char slotNum = 0;
	unsigned char portNum = 0;
	unsigned int trans_value1 = 0;
	unsigned int trans_value2 = 0;
	int op_ret = 0;

	struct fdb_all *q,*tail;
	
	op_ret =  parse_slotport_no((char*)portno,&slotNum,&portNum);
	if (NPD_FAIL == op_ret) {
		//vty_out(vty,"Unknow portno format!\n");
		return -3;
	}
	/*if((slotNum <MIN_SLOT ||slotNum>MAX_SLOT)||(portNum <MIN_PORT||portNum>MAX_PORT)){
		//vty_out(vty,"err port outrange!\n");
		return -4;
	}*/
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,	\
									NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_SHOW_FDB_TABLE_PORT);
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
							DBUS_TYPE_BYTE,&slotNum,
						 	DBUS_TYPE_BYTE,&portNum,
						 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&dnumber);

	head->next=NULL;
	tail=head;
	*p_num=dnumber;
	
	if (dnumber == 0){
		//vty_out(vty,"THE ITEM IS NONE !\n");
		return -2;
	}
	// NOTICE We used 68 bytes here, we still got 12 bytes of summary info
	//vty_out(vty,"%-17s  %-8s%-11s%-8s%-8s%-6s\n","=================",	"======", "=========","======","======","======");
	//vty_out(vty,"%-17s  %-8s%-11s%-8s%-8s%-6s\n","MAC","VLAN","SLOT/PORT","TRUNK","VID","VIDX");
	//vty_out(vty,"%-17s  %-8s%-11s%-8s%-8s%-6s\n","=================",	"======", "=========","======","======","======");
	

	dbus_message_iter_next(&iter);
	
	dbus_message_iter_recurse(&iter,&iter_array);
	for ( ; dnumber > i; i++){

		//分配空间
		q=(struct fdb_all *)malloc(sizeof(struct fdb_all));
		if( NULL ==q )
			{
			return 0;
			}
		
		DBusMessageIter iter_struct;
		
		dbus_message_iter_recurse(&iter_array,&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&dcli_flag);
		dbus_message_iter_next(&iter_struct);

		q->dcli_flag=dcli_flag;
		
		dbus_message_iter_get_basic(&iter_struct,&vlanid);
		dbus_message_iter_next(&iter_struct);

		q->vlanid=vlanid;
		
		dbus_message_iter_get_basic(&iter_struct,&trans_value1);
		dbus_message_iter_next(&iter_struct);

		q->trans_value1=trans_value1;
		
		dbus_message_iter_get_basic(&iter_struct,&trans_value2);
		dbus_message_iter_next(&iter_struct);

		q->trans_value2=trans_value2;
		
		dbus_message_iter_get_basic(&iter_struct,&show_mac[0]);
		dbus_message_iter_next(&iter_struct);

		q->show_mac[0]=show_mac[0];
		
		dbus_message_iter_get_basic(&iter_struct,&show_mac[1]);
		dbus_message_iter_next(&iter_struct);

		q->show_mac[1]=show_mac[1];
		
		dbus_message_iter_get_basic(&iter_struct,&show_mac[2]);
		dbus_message_iter_next(&iter_struct);

		q->show_mac[2]=show_mac[2];
		
		dbus_message_iter_get_basic(&iter_struct,&show_mac[3]);
		dbus_message_iter_next(&iter_struct);

		q->show_mac[3]=show_mac[3];
		
		dbus_message_iter_get_basic(&iter_struct,&show_mac[4]);
		dbus_message_iter_next(&iter_struct);

		q->show_mac[4]=show_mac[4];
		
		dbus_message_iter_get_basic(&iter_struct,&show_mac[5]);

		q->show_mac[5]=show_mac[5];

		q->next=NULL;
		tail->next=q;
		tail=q;
		
		dbus_message_iter_next(&iter_array);
			}
	
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}	

//重新写函数

int show_fdb_by_vid
(
char *vlan_id,
struct fdb_all *head,
int *vid_num
)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	unsigned int dnumber = 0;
	unsigned int dcli_flag = 0;
	unsigned int ret=0;
	unsigned char  show_mac[6];
	unsigned short vlanid=0;
	unsigned int i=0;
	unsigned int trans_value1 = 0;
	unsigned int trans_value2 = 0;
	int op_ret = 0;

	struct fdb_all *q,*tail;
	
	op_ret =  parse_short_parse((char*)vlan_id,&vlanid);
	if (NPD_FAIL == op_ret) {
		//vty_out(vty,"FDB vlan outrang!\n");
		return -3;
	}
	 if (vlanid<MIN_VLANID||vlanid>MAX_VLANID){
		//vty_out(vty,"FDB vlan outrang!\n");
		return -4;
		}
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,	\
									NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_SHOW_FDB_TABLE_VLAN);
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
							 	DBUS_TYPE_UINT16,&vlanid,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);	
		if(ret==NPD_FDB_ERR_VLAN_NONEXIST){
			//vty_out(vty,"the vlanid is not register !\n");
			return -5;
			}
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&dnumber);
	if (dnumber == 0){
		//vty_out(vty,"THE ITEM IS NONE !\n");
		return -2;
		}
	// NOTICE We used 68 bytes here, we still got 12 bytes of summary info
	//vty_out(vty,"%-17s  %-8s%-11s%-8s%-8s%-6s\n","=================",	"======", "=========","======","======","======");
	//vty_out(vty,"%-17s  %-8s%-11s%-8s%-8s%-6s\n","MAC","VLAN","SLOT/PORT","TRUNK","VID","VIDX");
	//vty_out(vty,"%-17s  %-8s%-11s%-8s%-8s%-6s\n","=================",	"======", "=========","======","======","======");
	

	dbus_message_iter_next(&iter);
	
	dbus_message_iter_recurse(&iter,&iter_array);

	 head->next=NULL;
     tail=head;
	 *vid_num=dnumber;

	for ( ; dnumber > i; i++){

				//分配空间
		q=(struct fdb_all *)malloc(sizeof(struct fdb_all));
		if( NULL ==q )
			{
			return 0;
			}

		DBusMessageIter iter_struct;
		
		dbus_message_iter_recurse(&iter_array,&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&dcli_flag);
		dbus_message_iter_next(&iter_struct);

		q->dcli_flag=dcli_flag;

		dbus_message_iter_get_basic(&iter_struct,&vlanid);
		dbus_message_iter_next(&iter_struct);
		
		q->vlanid=vlanid;

		dbus_message_iter_get_basic(&iter_struct,&trans_value1);
		dbus_message_iter_next(&iter_struct);

		q->trans_value1=trans_value1;

		dbus_message_iter_get_basic(&iter_struct,&trans_value2);
		dbus_message_iter_next(&iter_struct);

		q->trans_value2=trans_value2;

		dbus_message_iter_get_basic(&iter_struct,&show_mac[0]);
		dbus_message_iter_next(&iter_struct);

		q->show_mac[0]=show_mac[0];

		dbus_message_iter_get_basic(&iter_struct,&show_mac[1]);
		dbus_message_iter_next(&iter_struct);

		q->show_mac[1]=show_mac[1];

		dbus_message_iter_get_basic(&iter_struct,&show_mac[2]);
		dbus_message_iter_next(&iter_struct);

		q->show_mac[2]=show_mac[2];

		dbus_message_iter_get_basic(&iter_struct,&show_mac[3]);
		dbus_message_iter_next(&iter_struct);

		q->show_mac[3]=show_mac[3];

		dbus_message_iter_get_basic(&iter_struct,&show_mac[4]);
		dbus_message_iter_next(&iter_struct);

		q->show_mac[4]=show_mac[4];

		dbus_message_iter_get_basic(&iter_struct,&show_mac[5]);

		q->show_mac[5]=show_mac[5];

		q->next=NULL;
		tail->next=q;
		tail=q;

		dbus_message_iter_next(&iter_array);
			}
	
	
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}	


/**************************************
重写函数，重新封装
****************************************/


int show_fdb_by_vlanname
(
char *vlan_name,
struct fdb_all *head,
int *vid_num
)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	unsigned int dnumber = 0;
	unsigned int dcli_flag = 0;
	unsigned short vlanid=0;
	unsigned char  show_mac[6];
	char* vlanname = NULL;
	unsigned int i=0;
	unsigned int trans_value1 = 0;
	unsigned int trans_value2 = 0;
	int op_ret=0;
	

	struct fdb_all *q,*tail;

	op_ret =parse_vlan_string((char*)vlan_name);
	
	if (NPD_FAIL == op_ret) {
	   //vty_out(vty,"vlanname form erro!\n");
	   return -3;
	}

	vlanname= (char *)vlan_name;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,	\
									NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_SHOW_FDB_TABLE_VLAN_WITH_NAME);
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
							 	DBUS_TYPE_STRING,&vlanname,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&op_ret);
	if(op_ret==NPD_FDB_ERR_VLAN_NONEXIST){
			//vty_out(vty,"the vlanid is not register !\n");
			return -5;
			}
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&dnumber);
	if (0== dnumber){
		//vty_out(vty,"THE ITEM IS NONE !\n");
		return -2;
		}
	//vty_out(vty,"THE ITEM IS %d\n",dnumber);
	// NOTICE We used 68 bytes here, we still got 12 bytes of summary info
	//vty_out(vty,"%-17s  %-8s%-11s%-8s%-8s%-6s\n","=================",	"======", "=========","======","======","======");
	//vty_out(vty,"%-17s  %-8s%-11s%-8s%-8s%-6s\n","MAC","VLAN","SLOT/PORT","TRUNK","VID","VIDX");
	//vty_out(vty,"%-17s  %-8s%-11s%-8s%-8s%-6s\n","=================",	"======", "=========","======","======","======");


	dbus_message_iter_next(&iter);
	
	dbus_message_iter_recurse(&iter,&iter_array);

	 head->next=NULL;
     tail=head;
	 *vid_num=dnumber;
	

	for ( ; dnumber > i; i++){

		//分配空间
		q=(struct fdb_all *)malloc(sizeof(struct fdb_all));
		if( NULL ==q )
			{
			return 0;
			}
        q->vname=vlanname;
      
		DBusMessageIter iter_struct;
		
		dbus_message_iter_recurse(&iter_array,&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&dcli_flag);
		dbus_message_iter_next(&iter_struct);

		q->dcli_flag=dcli_flag;

		dbus_message_iter_get_basic(&iter_struct,&vlanid);
		dbus_message_iter_next(&iter_struct);
		
		q->vlanid=vlanid;

		dbus_message_iter_get_basic(&iter_struct,&trans_value1);
		dbus_message_iter_next(&iter_struct);

		q->trans_value1=trans_value1;

		dbus_message_iter_get_basic(&iter_struct,&trans_value2);
		dbus_message_iter_next(&iter_struct);

		q->trans_value2=trans_value2;

		dbus_message_iter_get_basic(&iter_struct,&show_mac[0]);
		dbus_message_iter_next(&iter_struct);

		q->show_mac[0]=show_mac[0];

		dbus_message_iter_get_basic(&iter_struct,&show_mac[1]);
		dbus_message_iter_next(&iter_struct);

		q->show_mac[1]=show_mac[1];

		dbus_message_iter_get_basic(&iter_struct,&show_mac[2]);
		dbus_message_iter_next(&iter_struct);

		q->show_mac[2]=show_mac[2];

		dbus_message_iter_get_basic(&iter_struct,&show_mac[3]);
		dbus_message_iter_next(&iter_struct);

		q->show_mac[3]=show_mac[3];

		dbus_message_iter_get_basic(&iter_struct,&show_mac[4]);
		dbus_message_iter_next(&iter_struct);

		q->show_mac[4]=show_mac[4];

		dbus_message_iter_get_basic(&iter_struct,&show_mac[5]);

		q->show_mac[5]=show_mac[5];

		q->next=NULL;
		tail->next=q;
		tail=q;

		dbus_message_iter_next(&iter_array);
	
			}
	
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}


/**************************************
重写函数
)   
****************************************/

int show_fdb_by_mac
(
char *mac,
struct fdb_all *head,
int *m_num
)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	unsigned int dnumber = 0;
	unsigned int dcli_flag = 0;
	ETHERADDR		macAddr;
	unsigned char  show_mac[6];
	unsigned short vlanid = 0;
	unsigned int i=0;
	unsigned int trans_value1 = 0;
	unsigned int trans_value2 = 0;
	int op_ret = 0;
	memset(&macAddr,0,sizeof(ETHERADDR));

	struct fdb_all *q,*tail;
	
	op_ret = parse_mac_addr((char *)mac,&macAddr);
	if (NPD_FAIL == op_ret) {
		//vty_out(vty,"Unknow mac addr format!\n");
		return -3;
	}

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,	\
									NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_SHOW_FDB_TABLE_MAC);
		

	dbus_message_append_args(	query,
							 	DBUS_TYPE_BYTE,&macAddr.arEther[0],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[1],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[2],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[3],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[4],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[5],
							 	DBUS_TYPE_INVALID);
	
	dbus_error_init(&err);

	

	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&dnumber);
	if (dnumber == 0){
		//vty_out(vty,"THE ITEM IS NONE !\n");
		return -2;
		}
	
	//vty_out(vty,"%-17s  %-8s%-11s%-8s%-8s%-6s\n","=================",	"======", "=========","======","======","======");
	//vty_out(vty,"%-17s  %-8s%-11s%-8s%-8s%-6s\n","MAC","VLAN","SLOT/PORT","TRUNK","VID","VIDX");
	//vty_out(vty,"%-17s  %-8s%-11s%-8s%-8s%-6s\n","=================",	"======", "=========","======","======","======");
	

	dbus_message_iter_next(&iter);
	
	dbus_message_iter_recurse(&iter,&iter_array);

    head->next=NULL;
	tail=head;
	*m_num=dnumber;
	
	for ( ; dnumber > i; i++){

		//分配空间
		q=(struct fdb_all *)malloc(sizeof(struct fdb_all));
		if ( NULL ==q )
			{
			return 0;
			}
		DBusMessageIter iter_struct;
		
		dbus_message_iter_recurse(&iter_array,&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&dcli_flag);
		dbus_message_iter_next(&iter_struct);

        q->dcli_flag=dcli_flag;
		
		dbus_message_iter_get_basic(&iter_struct,&vlanid);
		dbus_message_iter_next(&iter_struct);

		q->vlanid=vlanid;
		
		dbus_message_iter_get_basic(&iter_struct,&trans_value1);
		dbus_message_iter_next(&iter_struct);

		q->trans_value1=trans_value1;
		
		dbus_message_iter_get_basic(&iter_struct,&trans_value2);
		dbus_message_iter_next(&iter_struct);

		q->trans_value2=trans_value2;
		
		dbus_message_iter_get_basic(&iter_struct,&show_mac[0]);
		dbus_message_iter_next(&iter_struct);

		q->show_mac[0]=show_mac[0];

		dbus_message_iter_get_basic(&iter_struct,&show_mac[1]);
		dbus_message_iter_next(&iter_struct);

		q->show_mac[1]=show_mac[1];

		dbus_message_iter_get_basic(&iter_struct,&show_mac[2]);
		dbus_message_iter_next(&iter_struct);

		q->show_mac[2]=show_mac[2];

		dbus_message_iter_get_basic(&iter_struct,&show_mac[3]);
		dbus_message_iter_next(&iter_struct);

		q->show_mac[3]=show_mac[3];

		dbus_message_iter_get_basic(&iter_struct,&show_mac[4]);
		dbus_message_iter_next(&iter_struct);

		q->show_mac[4]=show_mac[4];

		dbus_message_iter_get_basic(&iter_struct,&show_mac[5]);

		q->show_mac[5]=show_mac[5];

		q->next=NULL;
		tail->next=q;
		tail=q;

		dbus_message_iter_next(&iter_array);
			}

	
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}


int config_fdb_by_port(char *portno, char *count)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int 	ret = 0;
	unsigned char slotNum = 0;
	unsigned char portNum = 0;
	unsigned int number = 0;
	unsigned int fdblimit = 0;


	// parse the slot/port number
	ret =  parse_slotport_no((char*)portno,&slotNum,&portNum);
	if (NPD_FAIL == ret) {
		//vty_out(vty,"%% Error,Unknow portno format!\n");
		//return CMD_SUCCESS;
		return -2;
	}
	/*
	if((slotNum <MIN_SLOT ||slotNum>MAX_SLOT)||(portNum <MIN_PORT||portNum>MAX_PORT)){
		vty_out(vty,"err port outrange!\n");
		return CMD_SUCCESS;
		}
	*/
	
    //parse the number

	ret = parse_int_parse((char *)count,&number);
	if(NPD_FAIL == ret){
		//vty_out(vty,"%% Error,Unknow number format!\n");
		//return CMD_SUCCESS;
		return -4;
	}
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_NUMBER_WITH_PORT);
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
								DBUS_TYPE_BYTE,&slotNum,
							 	DBUS_TYPE_BYTE,&portNum,
							 	DBUS_TYPE_UINT32,&number,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {		
		if (dbus_error_is_set(&err)) {			
			dbus_error_free(&err);
		}
	}
	else{
			if (dbus_message_get_args ( reply, &err,
				DBUS_TYPE_UINT32,&ret,
				DBUS_TYPE_UINT32,&fdblimit,
				DBUS_TYPE_INVALID)) {
				//dcli_fdb_output(ret);
				if(NPD_FDB_ERR_BADPARA == ret){
					//vty_out(vty,"%% Error,bad parameters input!\n");
					return -3;
				}
				else if(NPD_FDB_ERR_GENERAL == ret){
                   // vty_out(vty,"%% Error occured in configuration\n");
                   return -1;
				}
				else if(NPD_FDB_ERR_OCCUR_HW == ret){
					//vty_out(vty,"%% Error occured in hw when configuration\n");
					return -1;
				}
				else{
				   // vty_out(vty,"The number has been set is: %d\n",fdblimit);
					return CMD_SUCCESS;
				}
			} 
			else {				
				if (dbus_error_is_set(&err)) {					
					dbus_error_free(&err);
				}
				return CMD_FAILURE;
			}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

/**************************************
config fdb item with vlan Both Sw & Hw. 
*
* Usage:config fdb number Number on the port Port
*
*input Params: 
*	vlan:       The port wanted to be stricted for fdb count
*	Number:  The fdb count wanted to be learned
*   
*
*output params:none
*   DEFUN(config_fdb_vlan_number_cmd_func,
	config_fdb_vlan_number_cmd,
	"config fdb vlan <1-4094> (<1-16384>|0)",
	"Config system information\n"
	"config FDB table\n"
	"config vlan-based \n"
	"config the vlan number \n"
	"config the number of the fdb entry 1-16384\n"
	"cancel the configuration\n"
)  
****************************************/

int config_fdb_by_vlan(char *vlan_id, char *count)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int 	ret = 0;

	unsigned short vlanid = 0;
	unsigned int number = 0;


	// parse the vlan number
	ret = parse_vlan_no_fdb((char*)vlan_id,&vlanid);
	
	if (NPD_FAIL == ret) {
    	//vty_out(vty,"Illegal vlanId .\n");
		return -2;
	}
	if (4095 == vlanid){
		//vty_out(vty,"% Bad parameter,Reserved vlan for Layer3 interface of EthPort!\n");
		return -2;
	}
	
    //parse the number

	ret = parse_int_parse((char *)count,&number);
	if(NPD_FAIL == ret){
		//vty_out(vty,"Unknow number format!\n");
		return -3;
	}
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_NUMBER_WITH_VLAN);
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
								DBUS_TYPE_UINT16,&vlanid,
							 	DBUS_TYPE_UINT32,&number,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}
	else{
			if (dbus_message_get_args ( reply, &err,
				DBUS_TYPE_UINT32,&ret,
				DBUS_TYPE_UINT32,&number,
				DBUS_TYPE_INVALID)) {
				//dcli_fdb_output(ret);
				if( -1 == ret){
				     //vty_out(vty,"Error in the process\n");
				     return CMD_FAILURE;
				}
				else if(NPD_FDB_ERR_VLAN_NONEXIST == ret){
					 //vty_out(vty,"The vlan %d does not exist\n",vlanid);
					 return -4;
				}
				else{
					 //vty_out(vty,"The number set is %d\n",number);
					 return CMD_SUCCESS;
				}
			}
			 else {
				//vty_out(vty,"Failed get args.\n");
				if (dbus_error_is_set(&err)) {
					//vty_out(vty,"%s raised: %s",err.name,err.message);
					dbus_error_free(&err);
				}
				return CMD_FAILURE;
			}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

//新的关于配置全的方法 

/* -1: 失败 0 成功，-2 vlanid错误 -3 端口格式错误 -4 端口超出范围 -5 保护个数错误
  -7 vlan不存在  -8 vlan中没有此port

*/
int config_fdb_by_vlan_port(char *vlan_id, char *port, char *count)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int 	ret = 0;

	unsigned short vlanid = 0;
	unsigned int number = 0;
	unsigned char slotNum = 0;
	unsigned char portNum = 0;


	// parse the vlan number
	ret = parse_vlan_no_fdb((char*)vlan_id,&vlanid);
	
	if (NPD_FAIL == ret) {
    	//vty_out(vty,"Illegal vlanId .\n");
		//return CMD_SUCCESS;
		return -2;
	}
	if (4095 == vlanid){
		//vty_out(vty,"% Bad parameter,Reserved vlan for Layer3 interface of EthPort!\n");
		//return CMD_WARNING;
		return -2;
	}

	//parse the slot/port number
	ret =  parse_slotport_no((char*)port,&slotNum,&portNum);
	if (NPD_FAIL == ret) {
		//vty_out(vty,"Unknow portno format!\n");
		//return CMD_SUCCESS;
		return -3;
	}
	if((slotNum <MIN_SLOT ||slotNum>MAX_SLOT)||(portNum <MIN_PORT||portNum>MAX_PORT)){
		//vty_out(vty,"err port outrange!\n");
		//return CMD_SUCCESS;
		return -4;
		}
	
    //parse the number

	ret = parse_int_parse((char *)count,&number);
	if(NPD_FAIL == ret){
		//vty_out(vty,"Unknow number format!\n");
		//return CMD_SUCCESS;
		return -5;
	}
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_NUMBER_WITH_VLAN_PORT);
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
								DBUS_TYPE_UINT16,&vlanid,
								DBUS_TYPE_BYTE,&slotNum,
							 	DBUS_TYPE_BYTE,&portNum,
							 	DBUS_TYPE_UINT32,&number,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}
	else{
			if (dbus_message_get_args ( reply, &err,
				DBUS_TYPE_UINT32,&ret,
				DBUS_TYPE_INVALID)) {
				//dcli_fdb_output(ret);
				if( -1 == ret){
				    // vty_out(vty,"%% Error in the process\n");
				    return -9;
				}
				else if(NPD_FDB_ERR_BADPARA == ret){
					//vty_out(vty,"%% Error,Bad parameters input!\n");
					return -6;
				}
				else if(NPD_FDB_ERR_VLAN_NONEXIST == ret){
					// vty_out(vty,"%% Error,The vlan %d does not exist\n",vlanid);
					return -7;
				}
				else if(NPD_FDB_ERR_PORT_NOTIN_VLAN== ret){
					//vty_out(vty,"%% Error,The port number is not in vlan %d\n",vlanid);
					return -8;
				}
				else if(NPD_FDB_ERR_GENERAL == ret){
                   // vty_out(vty,"%% Error occured in configuration\n");
                  return CMD_FAILURE;
				}
				else if(NPD_FDB_ERR_OCCUR_HW == ret){
					//vty_out(vty,"%% Error occured in hw when configuration\n");
					return CMD_FAILURE;
				}
			    else{
					// vty_out(vty,"The number set is %d\n",number);
					return CMD_SUCCESS;
				}
			} 
			else {				
				if (dbus_error_is_set(&err)) {					
					dbus_error_free(&err);
				}
				return CMD_FAILURE;
			}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}



/**************************************
重写函数
)
****************************************/




int show_fdb_number_limit
(
struct fdb_limit *head,
int *l_num
)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	

	unsigned char slotNum = 0;
	unsigned char portNum = 0;
	unsigned short vlanId = 0;
	unsigned int number = 0;
	unsigned int fdbnumber = 0;
	unsigned int i = 0;

	struct fdb_limit *q,*tail;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_SHOW_FDB_NUMBER_LIMIT_ITEM);
	
	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&number);
	if (number == 0){
		//vty_out(vty,"There is no fdb limit!\n");
		return -2;
	}
	// NOTICE We used 68 bytes here, we still got 12 bytes of summary info
	//vty_out(vty,"%-8s%-11s%-8s%-8s\n","======", "=========","======","======");
	//vty_out(vty,"%-8s%-11s%-8s%-8s\n","VLAN","SLOT/PORT","TRUNK","number");
	//vty_out(vty,"%-8s%-11s%-8s%-8s\n","======", "=========","======","======");
	
	dbus_message_iter_next(&iter);
	
	dbus_message_iter_recurse(&iter,&iter_array);

	head->next=NULL;
	tail=head;
	*l_num=number;
	
	for ( ; i < number; i++){

		//分配空间
		q=(struct fdb_limit *)malloc(sizeof(struct fdb_limit));
		if ( NULL ==q)
			{
			return 0;
			}
		DBusMessageIter iter_struct;
		
		dbus_message_iter_recurse(&iter_array,&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&vlanId);
		dbus_message_iter_next(&iter_struct);

		q->vlanid=vlanId;
		
		dbus_message_iter_get_basic(&iter_struct,&slotNum);
		dbus_message_iter_next(&iter_struct);

		q->slotNum=slotNum;
		
		dbus_message_iter_get_basic(&iter_struct,&portNum);
		dbus_message_iter_next(&iter_struct);

		q->portNum=portNum;
		
		dbus_message_iter_get_basic(&iter_struct,&fdbnumber);

		q->fdbnumber=fdbnumber;

		q->next=NULL;
		tail->next=q;
		tail=q;
		

		dbus_message_iter_next(&iter_array);
		
		}
	
	dbus_message_unref(reply);

	return CMD_SUCCESS;	
}


/*
void dcli_fdb_show_running_config() {	
	char *showStr = NULL;
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,		\
							NPD_DBUS_FDB_OBJPATH , \
							NPD_DBUS_FDB_INTERFACE ,		\
							NPD_DBUS_STATIC_FDB_METHOD_SHOW_RUNNING_CONFIG);

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_STRING, &showStr,
					DBUS_TYPE_INVALID)) 
	{
		vtysh_add_show_string(showStr);
	} 
	else 
	{
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
	}

	dbus_message_unref(reply);
	//printf("%s",showStr);
	return CMD_SUCCESS;	
}


void dcli_fdb_init() {
	install_element(CONFIG_NODE, &config_fdb_agingtime_cmd);
	install_element(CONFIG_NODE, &show_fdb_agingtime_cmd);
	install_element(VIEW_NODE,   &show_fdb_agingtime_cmd);
	install_element(ENABLE_NODE, &show_fdb_agingtime_cmd);
	install_element(CONFIG_NODE, &config_fdb_no_agingtime_cmd);
	install_element(CONFIG_NODE, &config_fdb_mac_vlan_port_cmd);
	install_element(CONFIG_NODE, &config_fdb_mac_vlan_port_name_cmd);
	install_element(CONFIG_NODE, &config_fdb_delete_vlan_cmd);
	install_element(CONFIG_NODE, &config_fdb_delete_port_cmd);
	install_element(CONFIG_NODE, &config_fdb_delete_trunk_cmd);
	install_element(CONFIG_NODE, &config_fdb_static_delete_vlan_cmd);
	install_element(CONFIG_NODE, &config_fdb_static_delete_port_cmd);
	install_element(CONFIG_NODE, &config_fdb_mac_vlan_trunk_cmd);
	install_element(CONFIG_NODE, &config_fdb_mac_vlan_trunk_name_cmd);
	install_element(CONFIG_NODE, &show_fdb_one_cmd);
	install_element(VIEW_NODE,   &show_fdb_one_cmd);
	install_element(ENABLE_NODE, &show_fdb_one_cmd);
	install_element(CONFIG_NODE, &show_fdb_mac_cmd);
	install_element(VIEW_NODE,   &show_fdb_mac_cmd);
	install_element(ENABLE_NODE, &show_fdb_mac_cmd);	
	install_element(CONFIG_NODE, &show_fdb_port_cmd);
	install_element(VIEW_NODE,   &show_fdb_port_cmd);
	install_element(ENABLE_NODE, &show_fdb_port_cmd);
	install_element(CONFIG_NODE, &show_fdb_vlan_cmd);
	install_element(VIEW_NODE,   &show_fdb_vlan_cmd);
	install_element(ENABLE_NODE, &show_fdb_vlan_cmd);
	install_element(CONFIG_NODE, &show_fdb_vlan_name_cmd);
	install_element(VIEW_NODE,   &show_fdb_vlan_name_cmd);
	install_element(ENABLE_NODE, &show_fdb_vlan_name_cmd);
	install_element(CONFIG_NODE, &config_fdb_mac_vlan_match_no_drop_cmd);
	install_element(CONFIG_NODE, &config_fdb_mac_vlan_match_no_drop_name_cmd);
	install_element(CONFIG_NODE, &config_fdb_mac_vlan_match_cmd);
	install_element(CONFIG_NODE, &config_fdb_mac_vlan_name_match_cmd);
	install_element(CONFIG_NODE, &config_fdb_mac_vlan_no_static_port_cmd);
	install_element(CONFIG_NODE, &config_fdb_mac_vlan_no_static_port_name_cmd);
	install_element(CONFIG_NODE, &show_fdb_cmd);
	install_element(VIEW_NODE,   &show_fdb_cmd);
	install_element(ENABLE_NODE, &show_fdb_cmd);
	install_element(CONFIG_NODE, &show_fdb_dynamic_cmd);
	install_element(VIEW_NODE,   &show_fdb_dynamic_cmd);
	install_element(ENABLE_NODE, &show_fdb_dynamic_cmd);
	install_element(CONFIG_NODE, &show_fdb_count_cmd);
	install_element(VIEW_NODE,   &show_fdb_count_cmd);
	install_element(ENABLE_NODE, &show_fdb_count_cmd);
	install_element(CONFIG_NODE, &show_fdb_static_cmd);
	install_element(VIEW_NODE,   &show_fdb_static_cmd);
	install_element(ENABLE_NODE, &show_fdb_static_cmd);
	install_element(CONFIG_NODE, &show_fdb_blacklist_cmd);
	install_element(VIEW_NODE,   &show_fdb_blacklist_cmd);
	install_element(ENABLE_NODE, &show_fdb_blacklist_cmd);
	install_element(CONFIG_NODE, &config_fdb_system_mac_cmd);
	install_element(CONFIG_NODE, &config_fdb_port_number_cmd);
	install_element(CONFIG_NODE, &config_fdb_vlan_number_cmd);
	install_element(CONFIG_NODE, &show_fdb_number_limit_cmd);
	install_element(VIEW_NODE,   &show_fdb_number_limit_cmd);
	install_element(ENABLE_NODE, &show_fdb_number_limit_cmd);
	install_element(CONFIG_NODE, &config_fdb_vlan_port_number_cmd);
}

*/

