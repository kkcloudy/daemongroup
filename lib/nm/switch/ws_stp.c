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
* ws_stp.c
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
//ws_stp.c
/*dcli_stp.h 1.10*/
/*dcli_stp.c 1.62*/
/*dcli_common_stp.h 1.10*/
/*dcli_common_stp.c 1.42*/
/*author qiandawei*/
/*modify zhouyanmei*/
/*update time 09-11-10*/

#ifdef __cplusplus
extern "C"
{
#endif

#include "ws_stp.h"
/*#include "cgic.h"*/
//#include "ws_ec.h"
#include "ws_returncode.h"

//int dcli_debug_out = 1;

char* stp_port_role[7] = {
	
			  	"Dis",
				  "Alt",
				  "Bkp",
				  "Root",
				  "Desi",
				  "Mst",
				  "NStp"
	};

char* stp_port_state[5] = {
		"DISABLED",
		"DISCARDING",
		"LEARNING",
		"FORWARDING",
		"NON-STP"
	};



int config_mode = 0;          //保存配置模式，0表示，1表示MST_MODE   全局变量

static char g_regionName[32] = {'\0'};

static unsigned int productid = 0;//产品	ID

unsigned int  GetSlotPortFromPortIndex
(
    unsigned int  product_id,
    unsigned int eth_g_index,
    unsigned char *slot_no,
    unsigned char *port_no
)
{
	unsigned char slot_index = 0,port_index = 0;
	
	slot_index = ((eth_g_index & 0x000007C0) >> 6);
	port_index = (eth_g_index & 0x0000003F);
    //printf("Welcome to the GetSlotPortFromEthIndex func!\n");
	//printf("port_index: %d,slot_index %d,port_index %d,productId %d\n",eth_g_index,slot_index,port_index,productId);
	if(PRODUCT_ID_AX7K == product_id){
      *slot_no = slot_index;//AU7K,AU3K,slot_start_no is 0
      *port_no = port_index + 1;//AU7K port start no is 1
      if((*slot_no > 5)||(*port_no > 6)){
         return DCLI_STP_OK;
	  }
     // printf("slot_no %d,port_no %d\n",*slot_no,*port_no);
	  
	}
	else if(PRODUCT_ID_AX5K == product_id){
       *slot_no = slot_index + 1;//AX5K,4K.slot start no is 1
       *port_no = port_index;//AX5K port start no is 0
       if((*slot_no > 1)||(*port_no > 28)){
         return DCLI_STP_OK;
	   }
     // printf("slot_no %d,port_no %d\n", *slot_no,*port_no);
	}
	else if(PRODUCT_ID_AU4K == product_id){//AX5K,4K.slot start no is 1
       *slot_no= slot_index + 1;
	   *port_no = port_index + 1;//AU4K port start no is 1
	   if((*slot_no > 1)||(*port_no > 28)){
         return DCLI_STP_OK;
	   }
	  //printf("slot_no %d,port_no %d\n", *slot_no,*port_no);
	}
	else if(PRODUCT_ID_AU3K == product_id){
       *slot_no = slot_index + 1;//AU3K,slot_start_no is 0
	   *port_no = port_index + 1;//AU3K port start no is 0
	   //::TODO　restrict the slot,port !!!
	   
	 // printf("slot_no %d,port_no %d\n", *slot_no,*port_no);
	}
	return DCLI_STP_OK;
   /**********************
	*enum product_id_e {
	*	PRODUCT_ID_NONE,
	*	PRODUCT_ID_AX7K,
	**	PRODUCT_ID_AX5K,
	*	PRODUCT_ID_AU4K,
	*	PRODUCT_ID_AU3K,
	*	PRODUCT_ID_MAX
	*};
       **********************/
	
	/*************************************  
	*chassis_slot_count, chassis_slot_start_no
	*{0,0},  // PRODUCT_ID_NONE
	*{5,0},	// PRODUCT_ID_AX7K
	*{1,1},	// PRODUCT_ID_AX5K
	*{1,1},	// PRODUCT_ID_AU4K
	*{0,0}	// PRODUCT_ID_AU3K
	*************************************/
	/* ***********************************
	*ext_slot_count,  eth_port_1no, eth_port_count	
	*{0,0,0},   	//  MODULE_ID_NONE
       * {1,1,4},   	//  MODULE_ID_AX7_CRSMU
	*{0,1,6},	// MODULE_ID_AX7_6GTX
	*{0,1,6},    // MODULE_ID_AX7_6GE_SFP
	*{0,1,1},	// MODULE_ID_AX7_XFP
	*{0,1,6},	// MODULE_ID_AX7_6GTX_POE
	*{0,0,0},	// MODULE_ID_AX5
       ****************************************/
}


unsigned int  GetSlotPortFromPortIndex_new
(
    unsigned int eth_g_index,
    unsigned char *slot_no,
    unsigned char *port_no
)
{
	DBusMessage *query, *reply;
	DBusError err; 
	//int op_ret = 0;
	unsigned char slotno = 0,portno = 0;

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,			\
								NPD_DBUS_ETHPORTS_OBJPATH,		\
								NPD_DBUS_ETHPORTS_INTERFACE,		\
								STP_DBUS_METHOD_GET_SLOTS_PORTS_FROM_INDEX);


    dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&eth_g_index,
									DBUS_TYPE_INVALID);

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return CMD_SUCCESS;
	}


	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_BYTE,&slotno,
		DBUS_TYPE_BYTE,&portno,
		DBUS_TYPE_INVALID)) {
		*slot_no = slotno;
		*port_no = portno;
		//DCLI_DEBUG(("return op_ret %d and product_id %d\n",op_ret,*id));
		dbus_message_unref(reply);
		return CMD_SUCCESS;

	} else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
	    return CMD_FAILURE;
	}

	dbus_message_unref(reply);
	return CMD_SUCCESS;



}

//选择STP的配置模式
//0:成功，-1:失败，1:现在运行的是RSTP 2:现在运行的是MSTP
int config_spanning_tree_mode(char *mode)
{
	int state = 1;
	int stp_mode = 0; 
	//int node = DCLI_STP_M; 


	ccgi_get_broad_product_id(&productid);
	state = ccgi_get_brg_g_state(&stp_mode);
	//Testing the state of the brdige , that geted.
	//DCLI_DEBUG(("Now the stp state is %d and stp mode is %d.\n",state, stp_mode));
	if(0 == state){
		if(2 == stp_mode) {


			if(strncmp("mst",mode,strlen(mode))==0)
			{
				config_mode = CGI_MST_MODE;
			}
			else if (strncmp("stp",mode,strlen(mode))==0)
			{
				config_mode = CGI_STP_MODE;
			}
			#if 0
			if(strcmp(mode,"mst")==0)
			{
				config_mode = CGI_MST_MODE;
			}
			else if (strcmp(mode,"stp")==0)
			{
				config_mode = CGI_STP_MODE;
			}
			#endif
			else
			{
				//vty_out(vty,"command parameters error!\n");
				return CMD_FAILURE;
			}

		}
		else{
			//vty_out(vty,"Critical error in stp!\n");
			return CMD_FAILURE;
		}
	}
	else if(1 == state)	
		{
			if(0 == stp_mode) //Bridge is running stp
				{
					if((strcmp(mode, "stp") == 0) && (CGI_STP_MODE == config_mode))
						config_mode = CGI_STP_MODE;
					else if((strcmp(mode, "mst") == 0) && (CGI_STP_MODE == config_mode))
						{
							//vty_out(vty, "Please disable stp first!\n");
							config_mode = CGI_STP_MODE;
							return 1;//现在运行的是RSTP，请先关掉
						}
					else
						{
							//vty_out(vty, "Bad parameters error!\n");
							return CMD_FAILURE;
						}
				}
			else if(1 == stp_mode) //Bridge is running mstp
				{
					if((strcmp(mode, "stp") == 0) && (CGI_STP_MODE == config_mode))
						{
							//vty_out(vty, "Please disable mstp first!\n");
							config_mode = CGI_MST_MODE;
							return 2;//现在运行的是MSTP，请先关掉
						}
					else if((strcmp(mode, "mst") == 0) && (CGI_STP_MODE == config_mode))
						config_mode = CGI_MST_MODE;
					else
						{
							//vty_out(vty, "Bad parameters error!\n");
							return CMD_FAILURE;
						}
				}		
			else
				{
					//vty_out(vty, "Critical error in stp!\n");
					return CMD_FAILURE;
				}
		}
	else
		{
			//vty_out(vty, "Critical error in stp!\n");
			return CMD_FAILURE;
		}

	
	return CMD_SUCCESS;

}
//打开或者关闭桥配置
int config_spanning_tree(char *status,DBusConnection *connection)

{
	//DBusMessage *query, *reply;
	//DBusError err;
	int isEnable = 0;//unsigned
	//int ret,op_ret;
	//VLAN_PORTS_BMP ports_bmp = {0};
	//int count = 0;
	
	if(strncmp("enable",status,strlen(status))==0)
	{
		isEnable = 1;
	}
	else if (strncmp("disable",status,strlen(status))==0)
	{
		isEnable = 0;
	}
	else
	{
		//vty_out(vty,"bad command parameter!\n");
		return CMD_FAILURE;
	}		

	if(CGI_STP_MODE == config_mode) {
		//init stp protocol
		ccgi_enable_g_stp_to_protocol(DCLI_STP_M,isEnable,ccgi_dbus_connection);
		//set stp version
		//DCLI_DEBUG(("isenable %s\n",isEnable ? "enable" : "disable"));
		if(isEnable)
			ccgi_set_bridge_force_version(STP_FORCE_VERS,ccgi_dbus_connection);
		//DCLI_DEBUG(("isenable %s finish\n",isEnable ? "enable" : "disable"));
	}
	else if(CGI_MST_MODE == config_mode) {
		ccgi_enable_g_stp_to_protocol(DCLI_MST_M,isEnable,ccgi_dbus_connection);
		if(isEnable)
			ccgi_set_bridge_force_version(MST_FORCE_VERS,ccgi_dbus_connection);
	}
	
	return CMD_SUCCESS;
}


//打开或者关闭端口配置
//0:成功 -1:失败 -2:端口未连接
//命令 config spanning-tree eth-port portno enable/disable

/*follow-up 09-02-23*/
int config_spanning_tree_ethport(char *port, char *status)
{
//	DBusMessage *query, *reply;
//	DBusError err;
//	struct interface * ethIntf = NULL;
	unsigned char slot_no = 0,port_no = 0;
	unsigned int isEnable = 0;
	//unsigned int port_index;
	int port_index = 0;
	int lk = 0;
	unsigned int speed = 0;
	unsigned int duplex_mode = 0;
	int ret = 0;
	//op_ret = 0;
	unsigned int mode = DCLI_STP_M;

	//DCLI_DEBUG(("It has been here: 250!!!! and port_index %d\n",port_index));
	if(strncmp("enable",status,strlen(status))==0)
	{
		isEnable = 1;
	}
	else if (strncmp("disable",status,strlen(status)) ==0)
	{
		isEnable = 0;
	}
	else
	{
		//vty_out(vty,"bad command parameter!\n");
		return CMD_FAILURE;
	}		

	//DCLI_DEBUG(("It has been here: 244!!!!\n"));
	ret = parse_slotport_no((unsigned char *)port,&slot_no,&port_no);//(unsigned char*)port
	if (NPD_FAIL == ret) {
    	//vty_out(vty,"Unknow port number format.\n");
		return CMD_FAILURE;
		
	}
	//DCLI_DEBUG(("It has been here: 250!!!! and slot %d,port %d\n",slot_no,port_no));
	
	if(ccgi_get_one_port_index(slot_no,port_no,&port_index) != 0)
	{
		//DCLI_DEBUG(("execute command failed\n"));
		return CMD_FAILURE;
		
	}
	
	//get the link state of the port
	if ( ccgi_get_port_index_link_state(port_index,&lk)!= 0)
	{
		//vty_out(vty, "Can't get the port link state!\n");
		return CMD_FAILURE;
		
	}
	
	
	//else 
	//{
		//DCLI_DEBUG(("port[%d],link state %s\n",port_index,lk ? "up" : "down"));
		//lk ? ShowAlert("up") : ShowAlert("down");
		
	//	if(lk == 0)
	//		return -2;
	//}

	//不能进行的操作
	/*
    else
    {
    	if(lk==0)
			return -1;
    }
	*/
	if(  ccgi_get_port_index_speed(port_index,&speed)!= 0){
	    //vty_out(vty, "Can't get the port speed!\n");
	    return CMD_FAILURE;
	}
	
	//get the port duplex mode
	if(isEnable)
	{
	if(  ccgi_get_port_duplex_mode(port_index,&duplex_mode) != 0){
       // vty_out(vty,"Can't get the port duplex mode!\n");
       return CMD_FAILURE;
	}
	else{//set duplex_mode value to stp
       if( ccgi_set_port_duplex_mode_to_stp(port_index,duplex_mode)!= 0){
	   	 // vty_out(vty,"dcli_set_port_duplex_mode_to_stp error!\n");
	   	 return CMD_FAILURE;
       	}
	}
}
	

	//vty_out(vty,"parse slot %d,port %d,port_index %d\n",slot_no,port_no,port_index);

	if(CGI_STP_MODE == config_mode) 
		mode = DCLI_STP_M;
	else if(CGI_MST_MODE == config_mode)
		mode = DCLI_MST_M;

	//DCLI_DEBUG(("It has been here: !!!! and mode %d\n",mode));

//++++++++++++++++++++++++检查是否开启
	if(CMD_SUCCESS == ccgi_enable_stp_on_one_port_to_protocol(port_index,isEnable,lk,speed)) {
		ret=ccgi_enable_stp_on_one_port_to_npd(mode,port_index,isEnable);
		if(ret!=0)
			//DCLI_DEBUG((" execute command failed\n"));
			return CMD_FAILURE;
	}

	return CMD_SUCCESS;

}



//config spanning tree
//配置桥的优先级
//0:成功 -1:失败 1:RSTP未开启 2:MSTP未开启 3:small 4: large
int config_spanning_tree_pri(char *priority )
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int ret;//unsigned
	unsigned int value,mstid = 0;//unsigned
	
	value = strtoul (priority,0,10);
	//Display the value that get from the command
	//printf("The parameter's value value = %ud\n", value);
	if(value < MIN_BR_PRIO || value > MAX_BR_PRIO ){
		//vty_out(vty,"input priority value out range <0-61440>\n");
		return  CMD_FAILURE;
	}
	else{
		if(0 != (value%4096)){
			//vty_out (vty,"spanning-tree priority value must be times of 4096!\n");
			return CMD_FAILURE;
		}
	}
	
	query = dbus_message_new_method_call(
									RSTP_DBUS_NAME,    \
									RSTP_DBUS_OBJPATH,    \
									RSTP_DBUS_INTERFACE,    \
									RSTP_DBUS_METHOD_CONFIG_PRIO);

    dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&mstid,
									DBUS_TYPE_UINT32,&value,
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


	if (dbus_message_get_args ( reply, &err,
									DBUS_TYPE_UINT32,&ret,
									DBUS_TYPE_INVALID)) {

		if(STP_DISABLE == ret){
			if(CGI_STP_MODE == config_mode)
				//vty_out(vty,PRINTF_RSTP_NOT_ENABLED)
				return 1;
			else if(CGI_MST_MODE == config_mode)
				//vty_out(vty,PRINTF_MSTP_NOT_ENABLED)
				return 2;

		//* * * 新增	
			}
			else if(STP_Small_Bridge_Priority== ret){
				//vty_out(vty,PRINTF_STP_Small_Bridge_Priority);
				return 3;
			}
			else if(STP_Large_Bridge_Priority== ret){
				//vty_out(vty,PRINTF_STP_Large_Bridge_Priority);
				return 4;
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
		
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

//配置报文的最大存活时间
//0:成功 -1:失败 1:RSTP未开启 2:MSTP未开启 3:small 4: large
int config_spanning_tree_max_age(char *max_age)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int ret;//unsigned
	unsigned int value;//unsigned
	
	value = strtoul (max_age,0,10);
	if(value < MIN_BR_MAXAGE || value > MAX_BR_MAXAGE ){
		//vty_out(vty,"input max-age value out range <6-40>\n");
		return  CMD_FAILURE;
	}

	query = dbus_message_new_method_call( 				 \
										RSTP_DBUS_NAME,		 \
										RSTP_DBUS_OBJPATH,	 \
										RSTP_DBUS_INTERFACE, \
										RSTP_DBUS_METHOD_CONFIG_MAXAGE);

    dbus_message_append_args(query,
								DBUS_TYPE_UINT32,&value,
								DBUS_TYPE_INVALID);

	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);


	dbus_message_unref(query);

	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",&err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}
    
	if (dbus_message_get_args ( reply, &err,
								DBUS_TYPE_UINT32,&ret,
								DBUS_TYPE_INVALID)) {

		if(STP_DISABLE == ret){
			if(CGI_STP_MODE == config_mode)
				//vty_out(vty,PRINTF_RSTP_NOT_ENABLED)
				return 1;
			else if(CGI_MST_MODE == config_mode)
				//vty_out(vty,PRINTF_MSTP_NOT_ENABLED)
				return 2;
			
			else if(STP_Small_Max_Age== ret )
				//vty_out(vty,PRINTF_STP_Small_Max_Age);
				return 3;
			
			else if(STP_Large_Max_Age== ret)
				//vty_out(vty,PRINTF_STP_Large_Max_Age);
				return 4;
			
			else if(STP_Forward_Delay_And_Max_Age_Are_Inconsistent == ret){
			//	vty_out(vty,PRINTF_STP_Forward_Delay_And_Max_Age_Are_Inconsistent);
			return CMD_FAILURE;
			}
			else if(STP_Hello_Time_And_Max_Age_Are_Inconsistent == ret){
			//	vty_out(vty,PRINTF_STP_Hello_Time_And_Max_Age_Are_Inconsistent);
			return CMD_FAILURE;
			}
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
		
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

//配置hello报文的间隔时间
//0:成功 -1:失败 1:RSTP未开启 2:MSTP未开启 3,small 4,large
int config_spanning_tree_hello_time(char *hellotime)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int  ret;//unsigned
	unsigned int value=0;//unsigned
	
	value = strtoul (hellotime,0,10);
	if(value < MIN_BR_HELLOT || value > MAX_BR_HELLOT )
	{
		//vty_out(vty,"input hello-time value out range <1-10>\n");
		return CMD_FAILURE;
	}
	
	query = dbus_message_new_method_call(		\
									RSTP_DBUS_NAME,	\
									RSTP_DBUS_OBJPATH,	\
									RSTP_DBUS_INTERFACE,	\
									RSTP_DBUS_METHOD_CONFIG_HELTIME);

	dbus_error_init(&err);
	dbus_message_append_args(query,
								DBUS_TYPE_UINT32,&value,
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
									DBUS_TYPE_UINT32,&ret,
									DBUS_TYPE_INVALID)) {
		if(STP_DISABLE == ret){
			if(CGI_STP_MODE == config_mode)
				//vty_out(vty,PRINTF_RSTP_NOT_ENABLED)
				return 1;
			else if(CGI_MST_MODE == config_mode)
				//vty_out(vty,PRINTF_MSTP_NOT_ENABLED)
				return 2;
			
			}
			else if(STP_Small_Hello_Time== ret )
				//vty_out(vty,PRINTF_STP_Small_Hello_Time);
				return 3;
			
			else if(STP_Large_Hello_Time== ret)
				//vty_out(vty,PRINTF_STP_Large_Hello_Time);
				return 4;
			
			else if(STP_Hello_Time_And_Max_Age_Are_Inconsistent == ret){
				//vty_out(vty,PRINTF_STP_Hello_Time_And_Max_Age_Are_Inconsistent);
				return CMD_FAILURE;
			}
			else if(STP_Hello_Time_And_Forward_Delay_Are_Inconsistent == ret){
				//vty_out(vty,PRINTF_STP_Hello_Time_And_Forward_Delay_Are_Inconsistent);
				return CMD_FAILURE;
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
		
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

//配置转发延迟时间
//0:成功 -1:失败 1:RSTP未开启 2:MSTP未开启 3,small 4,large
int config_spanning_tree_forward_delay(char *delaytime)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int  ret;//unsigned
	unsigned int value;//unsigned
	
	value = strtoul (delaytime,0,10);
	if(value < MIN_BR_FWDELAY || value > MAX_BR_FWDELAY )
	{
		//vty_out(vty,"input forward delay value out range <4-30>\n");
		return CMD_FAILURE;
	}

	query = dbus_message_new_method_call(					\
										RSTP_DBUS_NAME,			\
										RSTP_DBUS_OBJPATH,		\
										RSTP_DBUS_INTERFACE,	\
										RSTP_DBUS_METHOD_CONFIG_FORDELAY);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&value,							
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
    
	if (dbus_message_get_args ( reply, &err,
								DBUS_TYPE_UINT32,&ret,
								DBUS_TYPE_INVALID)) {

		if(STP_DISABLE == ret){
			if(CGI_STP_MODE == config_mode)
				//vty_out(vty,PRINTF_RSTP_NOT_ENABLED)
				return 1;
			else if(CGI_MST_MODE == config_mode)
				//vty_out(vty,PRINTF_MSTP_NOT_ENABLED)
				return 2;
			
			else if(STP_Small_Forward_Delay== ret ){
				//vty_out(vty,PRINTF_STP_Small_Forward_Delay);
				return 3;
			}
			else if(STP_Large_Forward_Delay== ret){
				//vty_out(vty,PRINTF_STP_Large_Forward_Delay);
				return 4;
			}
			else if(STP_Forward_Delay_And_Max_Age_Are_Inconsistent == ret){
				//vty_out(vty,PRINTF_STP_Forward_Delay_And_Max_Age_Are_Inconsistent);
				return CMD_FAILURE;
			}
			else if(STP_Hello_Time_And_Forward_Delay_Are_Inconsistent == ret){
				//vty_out(vty,PRINTF_STP_Hello_Time_And_Forward_Delay_Are_Inconsistent);
				return CMD_FAILURE;
			}
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
		
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

//配置stp的版本
//0:成功 -1:失败 1:RSTP未运行 2:MSTP未运行
int config_spanning_tree_version(char *version)
{
	//DBusMessage *query, *reply;
	//DBusError err;
	
	unsigned int ret;//unsigned
	unsigned int value;//unsigned
	
	value = strtoul (version,0,10);
	if (!((value == 0)||(value == 2)) ) {
		//vty_out (vty,"%%spanning tree version must be 0 or 2 !\n");
		return -1;
	}
	if(CGI_STP_MODE == config_mode) {
		ret = ccgi_set_bridge_force_version(value,ccgi_dbus_connection);
	}
	
	return ret;
}

//恢复默认配置
//0:成功 -1:失败 1:RSTP未开启 2:MSTP未开启
int config_spanning_tree_default()
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int ret;//unsigned
	unsigned int mstid = 0;//unsigned
	
	query = dbus_message_new_method_call(				\
									RSTP_DBUS_NAME,			\
									RSTP_DBUS_OBJPATH,		\
									RSTP_DBUS_INTERFACE,	\
									RSTP_DBUS_METHOD_CONFIG_NOCONFIG);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32&mstid,						
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
	if (dbus_message_get_args ( reply, &err,
								DBUS_TYPE_UINT32,&ret,
								DBUS_TYPE_INVALID)) {

		if(STP_DISABLE == ret){
			if(CGI_STP_MODE == config_mode)
				//vty_out(vty,PRINTF_RSTP_NOT_ENABLED)
				return 1;
			else if(CGI_MST_MODE == config_mode)
				//vty_out(vty,PRINTF_MSTP_NOT_ENABLED)
				return 2;
		}
	}	
	else {
		
		//vty_out(vty," Failed get args.\n");
		if (dbus_error_is_set(&err)) {
				//vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}
		
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;
}



//config spanning tree port
//配置端口的路径开销
//0:成功 -1:失败 1:RSTP未开启 3:端口未开启 4: 路径开销默认值

/*follow-up 08-12-31 */
int config_spanning_tree_port_cost(char *port, char *cost)
{
	DBusMessage *query, *reply;
	DBusError err;
	
//	unsigned int re_slot_port = 0;
	//unsigned int re_value;
	unsigned int value,mstid = 0;
	unsigned char slot_no=0;
	unsigned char port_no=0;
	unsigned int ret;
	unsigned int port_index;


	ret = parse_slotport_no((unsigned char *)port,&slot_no,&port_no);//(unsigned char*)port
	if (NPD_FAIL == ret) {
    	//vty_out(vty,"Unknow port number format.\n");
		return CMD_FAILURE;
	}
	
	if (strcmp(cost, "auto")== 0){
			value = ADMIN_PORT_PATH_COST_AUTO;
	}
	else {
		value = strtoul (cost,0,10);
		if (!((value > 0)&&(value % 20 ==0)&&(value<=200000000))){
			//vty_out (vty,"path cost must be times of 20!\n");
			return CMD_FAILURE;
			}
		/* 还没有对返回值做处理 */
		else if(ADMIN_PORT_PATH_COST_AUTO == value){
            //vty_out(vty,"The path-cost 200000000 is auto value!");
            return 4;
		}

		}
	if( 0!= ccgi_get_one_port_index(slot_no,port_no,&port_index) )	{
		//DCLI_DEBUG(("execute command failed\n"));
		return CMD_FAILURE;
	}

	
	dbus_error_init(&err);
	query = dbus_message_new_method_call(				\
										RSTP_DBUS_NAME,		\
										RSTP_DBUS_OBJPATH,	\
										RSTP_DBUS_INTERFACE, \
										RSTP_DBUS_METHOD_CONFIG_PORT_PATHCOST);

	
	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&mstid,
									DBUS_TYPE_UINT32,&port_index,							 								 		
									DBUS_TYPE_UINT32,&value,									
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
								DBUS_TYPE_UINT32,&ret,
								DBUS_TYPE_INVALID)) {

		if(STP_DISABLE == ret){
			//vty_out(vty,PRINTF_RSTP_NOT_ENABLED);
			return 1;
		}
		else if(STP_PORT_NOT_ENABLED == ret) {
			//vty_out(vty,PRINTF_PORT_NOT_ENABLED);
			return 3;
		}
		//else if(0 == ret)
		//	ccgi_stp_set_port_pathcost_to_npd(mstid,port_index,value);
		
	} else {
		//vty_out(vty,"Failed get args from rstp!.\n");	
		if (dbus_error_is_set(&err)) {
				//vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}
		
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;
}

//配置端口的优先级
//0:成功 -1:失败 1:RSTP未开启 3:端口未开启

/*follow-up 08-12-31 */
int config_spanning_tree_port_pri(char *port, char *pri)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int port_index;
	unsigned int value,mstid = 0;
	unsigned char slot_no,port_no;
	unsigned int ret;

	ret = parse_slotport_no((unsigned char *)port,&slot_no,&port_no);//(unsigned char*)port
	if (NPD_FAIL == ret) {
    	//vty_out(vty,"Unknow port number format.\n");
		return CMD_FAILURE;
	}
	
	value = strtoul (pri,0,10);
	if(value < MIN_PORT_PRIO || value > MAX_PORT_PRIO ){
		//vty_out(vty,"input port-priority value out range <0-240>\n");
		return CMD_FAILURE;
	}
	else{
		if(0 != (value%16)){
			//vty_out (vty,"spanning-tree port-priority value must be times of 16!\n");
			return CMD_FAILURE;
		}
	}

	if( 0!= ccgi_get_one_port_index(slot_no,port_no,&port_index) )	{
		//DCLI_DEBUG(("execute command failed\n"));
		return CMD_FAILURE;
	}
	
	dbus_error_init(&err);	
	query = dbus_message_new_method_call(				\
										RSTP_DBUS_NAME,		\
										RSTP_DBUS_OBJPATH,	\
										RSTP_DBUS_INTERFACE, \
										RSTP_DBUS_METHOD_CONFIG_PORTPRIO);
	
	dbus_message_append_args(query,
						DBUS_TYPE_UINT32,&mstid,
						DBUS_TYPE_UINT32,&port_index,
						DBUS_TYPE_UINT32,&value,
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
						DBUS_TYPE_UINT32,&ret,
						DBUS_TYPE_INVALID)) {
		if(STP_DISABLE == ret){
			//vty_out(vty,PRINTF_RSTP_NOT_ENABLED);
			return 1;
		}
		else if(STP_PORT_NOT_ENABLED == ret) {
			//vty_out(vty,PRINTF_PORT_NOT_ENABLED);
			return 3;
		}
	//	else if(0 == ret)
		//	ccgi_stp_set_port_prio_to_npd(mstid,port_index,value);
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

//配置端口是否参加RSTP计算
//0:成功 -1:失败 1:RSTP未开启 3:端口未开启

/* follow-up 08-12-31 */
int config_spanning_tree_port_nonstp(char *port, char *mode)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int value,mstid = 0;//unsigned
	unsigned char slot_no,port_no;//unsigned char
	unsigned int port_index;//unsigned
	unsigned int ret;//unsigned

	ret = parse_slotport_no((unsigned char *)port,&slot_no,&port_no);//(unsigned char*)port
	if (NPD_FAIL == ret) {
    	//vty_out(vty,"Unknow port number format.\n");
		return CMD_FAILURE ;
	}

	if( 0!= ccgi_get_one_port_index(slot_no,port_no,&port_index))	{
		//DCLI_DEBUG(("execute command failed\n"));
		return CMD_FAILURE;
	}

		
	if(strncmp("yes",mode,strlen(mode))==0)
	{
		value = 1;
	}
	else if (strncmp("no",mode,strlen(mode))==0)
	{
		value = 0;
	}
	else
	{
		//vty_out(vty,"bad command parameter!\n");
		return CMD_FAILURE;
	}		
	
	dbus_error_init(&err);
	

	query = dbus_message_new_method_call(				\
									RSTP_DBUS_NAME,			\
									RSTP_DBUS_OBJPATH,		\
									RSTP_DBUS_INTERFACE,	\
									RSTP_DBUS_METHOD_CONFIG_NONSTP);
	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&mstid,
									DBUS_TYPE_UINT32,&port_index,
									DBUS_TYPE_UINT32,&value,
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
								DBUS_TYPE_UINT32,&ret,
								DBUS_TYPE_INVALID)) {
		if(STP_DISABLE == ret){
			//vty_out(vty,PRINTF_RSTP_NOT_ENABLED);
			return 1;
		}
		else if(STP_PORT_NOT_ENABLED == ret) {
			//vty_out(vty,PRINTF_PORT_NOT_ENABLED);
			return 3;
		}
	//	else if(0 == ret)
		//	ccgi_stp_set_port_nonstp_to_npd(mstid,port_index,value);
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

//配置端口的链路类型
//0:成功 -1:失败 1:RSTP未开启 3:端口未开启


/*follow-up 08-12-31 */
int config_spanning_tree_port_p2p(char *port, char *mode)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned char slot_no,port_no;//unsigned char
	unsigned int input,mstid = 0;//unsigned
	unsigned int port_index;//unsigned
	unsigned int ret;//unsigned

	ret = parse_slotport_no((unsigned char *)port,&slot_no,&port_no);//(unsigned char*)port
	if (NPD_FAIL == ret) {
    	//vty_out(vty,"Unknow port number format.\n");
		return CMD_FAILURE;
	}
	
	if (strncmp("no",mode , strlen(mode))== 0){
		input = 0;
	}
	else if(strcmp(mode,"yes")== 0){
		input = 1;
	}
	else if(strncmp("auto",mode,strlen(mode))== 0){
		input = 2;
	}
	else {
		//vty_out(vty,"p2p config value must be yes or no or auto!\n");
		return CMD_FAILURE;
	}

	if( 0!= ccgi_get_one_port_index(slot_no,port_no,&port_index))	{
		//DCLI_DEBUG(("execute command failed\n"));
		return CMD_FAILURE;
	}
	
	dbus_error_init(&err);
	

	query = dbus_message_new_method_call(						\
									RSTP_DBUS_NAME,					\
									RSTP_DBUS_OBJPATH,				\
									RSTP_DBUS_INTERFACE,			\
									RSTP_DBUS_METHOD_CONFIG_P2P);

	dbus_message_append_args(query,
								DBUS_TYPE_UINT32,&mstid,
								DBUS_TYPE_UINT32,&port_index,
								DBUS_TYPE_UINT32,&input,
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
							DBUS_TYPE_UINT32,&ret,
							DBUS_TYPE_INVALID)) {
		if(STP_DISABLE == ret){
			//vty_out(vty,PRINTF_RSTP_NOT_ENABLED);
			return 1;
		}
		else if(STP_PORT_NOT_ENABLED == ret) {
			//vty_out(vty,PRINTF_PORT_NOT_ENABLED);
			return 3;
		}
	//	else if(0 == ret) {
	//		ccgi_stp_set_port_p2p_to_npd(mstid,port_index,input);
	//	}
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

//配置端口是否为边缘端口
//0:成功 -1:失败 1:RSTP未开启 3:端口未开启

/*follow-up 08-12-31 */
int config_spanning_tree_port_edge(char *port, char *mode)
{
	DBusMessage *query, *reply;
	DBusError err;
	//unsigned int re_slot_port;//,re_value;
	unsigned int port_index;//unsigned
	
	unsigned char slot_no,port_no;//unsigned char
	unsigned int input,mstid = 0;//unsigned
	
	unsigned int ret;//unsigned

	ret = parse_slotport_no((unsigned char *)port,&slot_no,&port_no);//(unsigned char*)port
	if (NPD_FAIL == ret) {
    	//vty_out(vty,"Unknow port number format.\n");
		return CMD_FAILURE;
	}	
	
	if(strncmp("no",mode, strlen(mode)) == 0){
		input =0;
	}
	else if (strncmp("yes",mode,strlen(mode))== 0){
		input = 1;
	}
	else {
		//vty_out(vty,"edge config value must be yes or no\n");
		return CMD_FAILURE;
	}

	if( 0!= ccgi_get_one_port_index(slot_no,port_no,&port_index))	{
		//DCLI_DEBUG(("execute command failed\n"));
		return CMD_FAILURE;
	}


	dbus_error_init(&err);
	query = dbus_message_new_method_call(				\
									RSTP_DBUS_NAME,			\
									RSTP_DBUS_OBJPATH,		\
									RSTP_DBUS_INTERFACE,	\
									RSTP_DBUS_METHOD_CONFIG_EDGE);
	dbus_message_append_args(query,
								DBUS_TYPE_UINT32,&mstid,
								DBUS_TYPE_UINT32,&port_index,
								DBUS_TYPE_UINT32,&input,
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
									DBUS_TYPE_UINT32,&ret,
									DBUS_TYPE_INVALID)) {
		if(STP_DISABLE == ret){
			//vty_out(vty,PRINTF_RSTP_NOT_ENABLED);
			return 1;
		}
		else if(STP_PORT_NOT_ENABLED == ret) {
			//vty_out(vty,PRINTF_PORT_NOT_ENABLED);
			return 3;
		}
	//	else if(0 == ret)
	//		ccgi_stp_set_port_edge_to_npd(mstid,port_index,input);
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

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//配置端口是否执行端口协议类型协商
//0:成功 -1:失败 1:RSTP未开启 3:端口未开启

/*follow-up 08-12-31 */
int config_spanning_tree_port_mcheck(char *port, char *mode)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int ret;//re_slot_port,,re_value;//unsigned
	unsigned int port_index,mstid = 0;//unsigned
	unsigned char slot_no,port_no;//unsigned char
	
	int test;

	test = parse_slotport_no((unsigned char *)port,&slot_no,&port_no);//(unsigned char*)port
	if (NPD_FAIL == test) {
    	//vty_out(vty,"Unknow port number format.\n");
		return CMD_FAILURE;
	}
	
	if( 0!= ccgi_get_one_port_index(slot_no,port_no,&port_index))	{
		//DCLI_DEBUG(("execute command failed\n"));
		return CMD_FAILURE;
	}
	
	#ifdef STP_FULL_DEBUG
	//vty_out(vty,"SLOT_NO %d, LOCAL_PORT_NO %d, port_index %d \n",slot_no,port_no,port_index);
	#endif
	dbus_error_init(&err);
	query = dbus_message_new_method_call(					\
									RSTP_DBUS_NAME,				\
									RSTP_DBUS_OBJPATH,			\
									RSTP_DBUS_INTERFACE,		\
									RSTP_DBUS_METHOD_CONFIG_MCHECK);

	dbus_message_append_args(query,
								DBUS_TYPE_UINT32,&mstid,
								DBUS_TYPE_UINT32,&port_index,
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
								DBUS_TYPE_UINT32,&ret,
								DBUS_TYPE_INVALID)) {
		if(STP_DISABLE == ret){
			//vty_out(vty,PRINTF_RSTP_NOT_ENABLED);
			return 1;
		}
		else if(STP_PORT_NOT_ENABLED == ret) {
			//vty_out(vty,PRINTF_PORT_NOT_ENABLED);
			return 3;
		}
	}
	else {		
		//vty_out(vty," Failed get args.\n");		
		if (dbus_error_is_set(&err)) {
				//vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}
		
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

//恢复端口配置的默认值
//0:成功 -1:失败 1:RSTP未开启 3:端口未开启

/*follow-up 08-12-31 */
int config_spanning_tree_port_default(char *port)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int ret;//re_slot_port,;//unsigned
	unsigned char slot_no,port_no;//unsigned char
	unsigned int port_index,mstid = 0;//unsigned


	ret = parse_slotport_no((unsigned char *)port,&slot_no,&port_no);//(unsigned char*)port
	if (NPD_FAIL == ret) {
    	//vty_out(vty,"Unknown port number format!\n");
		return CMD_FAILURE;
	}
	
	dbus_error_init(&err);

	if( 0!= ccgi_get_one_port_index(slot_no,port_no,&port_index))	{
		//DCLI_DEBUG(("execute command failed\n"));
		return CMD_FAILURE;
	}
	query = dbus_message_new_method_call(			\
									RSTP_DBUS_NAME,		\
									RSTP_DBUS_OBJPATH,	\
									RSTP_DBUS_INTERFACE, \
									RSTP_DBUS_METHOD_CONFIG_PORT_NOCONFIG);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&mstid,
									DBUS_TYPE_UINT32,&port_index,							 							 		
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
							DBUS_TYPE_UINT32,&ret,
							DBUS_TYPE_INVALID)) {
		if(STP_DISABLE == ret){
			//vty_out(vty,PRINTF_RSTP_NOT_ENABLED);
			return 1;
		}
		else if(STP_PORT_NOT_ENABLED == ret) {
			//vty_out(vty,PRINTF_PORT_NOT_ENABLED);
			return 3;
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
		
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}



//show spanning tree information  ---dcli_stp_show_port_running_config       未使用
//显示桥信息

/*follow-up 09-02-1=23 */
int show_spanning_tree(bridge_info *ptbridge_info)
{

	port_info *ptport_info;
	
	//unsigned char product_flag = 0;
	unsigned int tmpval[2] = {0};
    PORT_MEMBER_BMP portbmp;
  	int product_id = 0;

    memset(&portbmp,0,sizeof(PORT_MEMBER_BMP));

	
	//SLOT_INFO slot_info[6];
	unsigned port_index = 0;//unsigned
	int i,ret = 0;
	unsigned char slot = 0,port = 0;//unsigned char

	//get slot count
	if( 0!= (ret = ccgi_get_all_ports_index(&portbmp)))
	{
		//vty_out(vty,"execute command failed \n");
		return CMD_FAILURE;
	}
	//DCLI_DEBUG(("portbmp %02x , productid %d \n",portbmp,productid));
	//Get bridge info  
	 	
	if((ret = ccgi_get_br_info(ptbridge_info)))
	{
		if(STP_DISABLE == ret)
		{
				//vty_out(vty,PRINTF_RSTP_NOT_ENABLED);
				return 1;
		}
		else 
		{
			//vty_out(vty,"execute command failed \n");
			return CMD_FAILURE;
		}
		
		
	
	}

	
	//vty_out(vty,"\n----------------------All ports information of STP domain 0-----------------------------------\n");	
	
	//vty_out(vty,"%-16s%-4s%-9s%-5s%-12s%-3s%-4s%-5s%-20s%-10s%-10s\n","Name","pri","cost","role","span-state","lk","p2p","edge","Desi-bridge-id","Dcost","D-port");

	for (i = 0; i < 64; i++) 
	{	
		if(PRODUCT_ID_AX7K == productid) {
			slot = i/8 + 1;
			port = i%8;
			
		}
			else if((PRODUCT_ID_AX5K == productid) ||
				(PRODUCT_ID_AX5K_I == productid) ||
				(PRODUCT_ID_AU4K == productid) ||
				(PRODUCT_ID_AU3K == productid) ||
				(PRODUCT_ID_AU3K_BCM == productid)||
				(PRODUCT_ID_AU3K_BCAT == productid)||
				(PRODUCT_ID_AU2K_TCAT == productid)
				)
			{


			slot = 1;
			port = i;
		}

		tmpval[i/32] = (1<<(i%32));
		if(portbmp.portMbr[i/32]& tmpval[i/32]) {
			//vty_out(vty,"%-d/%-14d",slot,port);
			if( 0!= ccgi_get_one_port_index(slot,port,&port_index))	{
				//DCLI_DEBUG(("execute command failed\n"));
				continue;
			}
			//DCLI_DEBUG(("port -index %d\n",port_index));
			if(ccgi_get_one_port_info(port_index,productid,ptport_info))
			{
				//vty_out(vty,"execute command failed \n");
				return CMD_FAILURE;
			}
		}	
	}			
	return CMD_SUCCESS;
}


/* get one port admin state*/

int get_port_admin_state
(
	char *port_num,
	int *s_type
)
{
	DBusMessage *stpQuery,*stpReply;
	DBusError err;
	DBusMessageIter	 stpIter;
	
	unsigned char slot_no,port_no;
    unsigned int port_index;
		
	unsigned int ret = 0;
	
    parse_slotport_no((unsigned char *)port_num,&slot_no,&port_no);
	ccgi_get_one_port_index(slot_no, port_no, &port_index);
	
    unsigned int enable = 0;
	/*printf("port index %d\n",port_index);*/
	stpQuery = dbus_message_new_method_call(			\
								RSTP_DBUS_NAME,		\
								RSTP_DBUS_OBJPATH,	\
								RSTP_DBUS_INTERFACE, \
								RSTP_DBUS_METHOD_SHOW_SPANTREE_ADMIN_STATE);

	dbus_message_append_args(stpQuery,
					 DBUS_TYPE_UINT32,&port_index,
					 DBUS_TYPE_INVALID);
	dbus_error_init(&err);
	stpReply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,stpQuery,-1, &err);


	dbus_message_unref(stpQuery);
	if (NULL == stpReply) {
		//vty_out(vty,"failed get stp stpReply.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(stpReply);
		//return CMD_WARNING;
		return -1;
	}
	dbus_message_iter_init(stpReply,&stpIter);			
	dbus_message_iter_get_basic(&stpIter, &ret);
	if(DCLI_STP_OK == ret )
	{
		dbus_message_iter_next(&stpIter);
		dbus_message_iter_get_basic(&stpIter,&enable);
        //vty_out(vty,"%-11s",enable ? "Y":"N");
        *s_type=enable;
		
		dbus_message_unref(stpReply);
		return CMD_SUCCESS;
		
	}
	else if(STP_DISABLE == ret)
	{
		dbus_message_unref(stpReply);
		return ret;
	}
	else
	{
		dbus_message_unref(stpReply);
		return ret;
	}
}


/*Dcli_common_stp*/
/**********************************************************************************
 *  ccgi_get_broad_product_id
 *
 *	DESCRIPTION:
 * 		get broad type
 *
 *	INPUT:
 *		NONE		
 *	
 * RETURN:
 *		0   -   succ
 *		-1   -   fail
 *
 **********************************************************************************/
//$$$$$$$$$$$$$$$$$$op_ret  ??????????????????????????????????????????
int ccgi_get_broad_product_id(unsigned int *id)
{
	DBusMessage *query, *reply;
	DBusError err; 
	int op_ret = 0;
	unsigned int product_id = 0;//unsigned

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,			\
								NPD_DBUS_ETHPORTS_OBJPATH,		\
								NPD_DBUS_ETHPORTS_INTERFACE,		\
								STP_DBUS_METHOD_GET_BROAD_TYPE);

	
	dbus_error_init(&err);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_UINT32,&product_id,
		DBUS_TYPE_INVALID)) {
		*id = product_id;
		//DCLI_DEBUG(("return op_ret %d and product_id %d\n",op_ret,*id));

	} else {
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


/**********************************************************************************
 *  ccgi_get_brg_g_state
 *
 *	DESCRIPTION:
 * 		get stp current state
 *
 *	INPUT:
 *		NONE		
 *	
 * RETURN:
 *		0   -   disable
 *		1   -   enable
 *
 **********************************************************************************/
//Get the global state and stp mode of the bridge

int ccgi_get_brg_g_state(int *stpmode)
{
	DBusMessage *query, *reply;
	DBusError err; 
	int op_ret = 0;
	int mode = 0;

	query = dbus_message_new_method_call(
								RSTP_DBUS_NAME,    \
								RSTP_DBUS_OBJPATH,    \
								RSTP_DBUS_INTERFACE,    \
								RSTP_DBUS_METHOD_GET_PROTOCOL_STATE);
	
	dbus_error_init(&err);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return 0;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_UINT32,&mode,
		DBUS_TYPE_INVALID)) {
		*stpmode = mode;
		//DCLI_DEBUG(("return op_ret %d and stpmode %d\n",op_ret,*stpmode));

	} else {
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return 0;
	}

	//DCLI_DEBUG(("return op_ret %d and stpmode %d\n",op_ret,*stpmode));
	dbus_message_unref(reply);
	return op_ret;
}


/**********************************************************************************
 *  ccgi_get_brg_g_state
 *
 *	DESCRIPTION:
 * 		get stp current state
 *
 *	INPUT:
 *		NONE		
 *	
 * RETURN:
 *		0   -   disable
 *		1   -   enable
 *
 **********************************************************************************/
//Get the global state and stp mode of the bridge

int ccgi_get_brg_g_state_slot(int *stpmode,DBusConnection *connection)
{
	DBusMessage *query, *reply;
	DBusError err; 
	int op_ret = 0;
	int mode = 0;

	query = dbus_message_new_method_call(
								RSTP_DBUS_NAME,    \
								RSTP_DBUS_OBJPATH,    \
								RSTP_DBUS_INTERFACE,    \
								RSTP_DBUS_METHOD_GET_PROTOCOL_STATE);
	
	dbus_error_init(&err);
	
	reply = dbus_connection_send_with_reply_and_block (connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return 0;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_UINT32,&mode,
		DBUS_TYPE_INVALID)) {
		*stpmode = mode;
		//DCLI_DEBUG(("return op_ret %d and stpmode %d\n",op_ret,*stpmode));

	} else {
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return 0;
	}

	//DCLI_DEBUG(("return op_ret %d and stpmode %d\n",op_ret,*stpmode));
	dbus_message_unref(reply);
	return op_ret;
}

/**********************************************************************************
 *  ccgi_enable_g_stp_to_protocol
 *
 *	DESCRIPTION:
 * 		enable or disable stp protocol
 *
 *	INPUT:
 *		type 			- stop/mstp
 *		isEnable	- en/disable
 *	
 * RETURN:
 *		CMD_FAILURE   
 *		CMD_SUCCESS   
 *
 **********************************************************************************/
//0:成功 -1:失败 1:RSTP未开启 4:RSTP已经开启

/*follow-up 08-12-31 */
int ccgi_enable_g_stp_to_protocol
(
	DCLI_STP_RUNNING_MODE mode,
	unsigned int isEnable,
	DBusConnection *connection

)
{
	DBusMessage *query, *reply;
	DBusError err;
	int ret,state=0,stp_mode=0;
	VLAN_PORTS_BMP ports_bmp = {0	};
	VLAN_PORTS_BMP *bmpPtr = NULL,*tmp = NULL;
	unsigned int count = 0;
	int i;


	if(1 == isEnable) 
		{   	    
		state=ccgi_get_brg_g_state_slot(&stp_mode,connection);
		if(1 == state){
          // vty_out(vty,"STP has already enabled!\n");
		  // return CMD_SUCCESS;
		  return 4;
		}
		ret = 0;
		ret = ccgi_change_all_ports_to_bmp(&ports_bmp,&count);//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$2008.7.9pm$$$$$$$$$$$$$$$$$$$
		//DCLI_DEBUG(("%s%d>>ports_bmp %02x\n",__FILE__,__LINE__,ports_bmp.untagbmp));
		if(CMD_SUCCESS == ret) {
			//DCLI_DEBUG(("%s%d>>ports_bmp vid[%d]\n",__FILE__,__LINE__,ports_bmp.vid));
			ret = ccgi_send_vlanbmp_to_mstp(&ports_bmp);
			if(CMD_FAILURE == ret) {
				//vty_out(vty," init mstp error\n");
				return CMD_FAILURE;
			}
		}
		else
			return CMD_FAILURE;	

		if(DCLI_MST_M == mode) {
			ret = ccgi_get_vlan_portmap(&bmpPtr,&count);
			tmp = bmpPtr;
			if(CMD_SUCCESS == ret) {
				for(i = 0; i<count; i++) {
					if(tmp) {
						ret = ccgi_send_vlanbmp_to_mstp(tmp);
						if(CMD_FAILURE == ret) {
							//vty_out(vty,"init mstp error\n");
							return CMD_FAILURE;
						}
						tmp++;
					}
				}
			}
			if(bmpPtr)
				free(bmpPtr);
		}
	}


	query = dbus_message_new_method_call(
									RSTP_DBUS_NAME,    \
									RSTP_DBUS_OBJPATH,    \
									RSTP_DBUS_INTERFACE,    \
									RSTP_DBUS_METHOD_STPM_ENABLE);

    dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&isEnable,
									DBUS_TYPE_UINT32,&mode,
									DBUS_TYPE_INVALID);

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (connection,query,-1, &err);

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
									DBUS_TYPE_UINT32,&ret,
									DBUS_TYPE_INVALID)) {
		if(0 == ret) {
			ccgi_enable_g_stp_to_npd(isEnable,connection);
		}
		if(STP_DISABLE == ret)
		{
			//vty_out(vty,PRINTF_RSTP_NOT_ENABLED)
			return 1;
		}
		else if(STP_HAVE_ENABLED == ret)
		{
			//vty_out(vty,PRINTF_RSTP_HAVE_ENABLED)
			return 4;
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
		
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

//0:成功 -1:失败 1:rstp未开启 2:mstp未开启
int ccgi_set_bridge_force_version
(
	unsigned int fversion,
	DBusConnection *connection
)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int ret;//unsigned
	unsigned int value = fversion;
	
	query = dbus_message_new_method_call(
										RSTP_DBUS_NAME,				\
										RSTP_DBUS_OBJPATH,			\
										RSTP_DBUS_INTERFACE,		\
										RSTP_DBUS_METHOD_CONFIG_FORVERSION);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&value,
							DBUS_TYPE_INVALID);
	dbus_error_init(&err);
 
	reply = dbus_connection_send_with_reply_and_block (connection,query,-1, &err);

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
								DBUS_TYPE_UINT32,&ret,
								DBUS_TYPE_INVALID)) {

		//DCLI_DEBUG(("%s, ret %d\n",__FILE__,ret));
		if(STP_DISABLE == ret){
			if(CGI_STP_MODE == config_mode)
				//vty_out(vty,PRINTF_RSTP_NOT_ENABLED);
				return 1;
			else if(CGI_MST_MODE == config_mode)
				//vty_out(vty,PRINTF_MSTP_NOT_ENABLED);
				return 2;
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
		
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}


int ccgi_get_one_port_index   //dcli_get_one_port_bmp 函数
(
	unsigned char slot,
	unsigned char local_port,
	unsigned int *port_index
) 
{
	DBusMessage *query, *reply;
	DBusError err;
	int op_ret = 0;
	unsigned int eth_g_index = 0;//unsigned
	unsigned char slot_no = slot,port_no = local_port;//unsigned char

	dbus_error_init(&err);
	query = dbus_message_new_method_call(				\
											NPD_DBUS_BUSNAME, 	\
											NPD_DBUS_ETHPORTS_OBJPATH,	\
											NPD_DBUS_ETHPORTS_INTERFACE, \
											STP_DBUS_METHOD_GET_PORT_INDEX
											);
	
		
		//DCLI_DEBUG(("the slot_no: %d	port_no: %d\n",slot_no,port_no);
		
		//tran_value=ETH_GLOBAL_INDEX_FROM_SLOT_PORT_LOCAL_NO(slot_no,port_no);
		//DCLI_DEBUG(("changed value : slot %d,port %d\n",slot_no,port_no));
		dbus_message_append_args(query,
										DBUS_TYPE_BYTE,&slot_no,										
										DBUS_TYPE_BYTE,&port_no,									
										DBUS_TYPE_INVALID);


	 reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"failed get re_slot_port reply.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return CMD_FAILURE;
	}
	
	if (dbus_message_get_args ( reply, &err,
								DBUS_TYPE_UINT32,&op_ret,
								DBUS_TYPE_UINT32,&eth_g_index,
								DBUS_TYPE_INVALID)) {
		if(NPD_DBUS_SUCCESS == op_ret){
			*port_index = eth_g_index;
			dbus_message_unref(reply);
			return CMD_SUCCESS;
		}
		else if(NPD_DBUS_ERROR == op_ret){
			//DCLI_DEBUG(("execute command failed\n");
			dbus_message_unref(reply);
			return CMD_FAILURE;
		}
		else if(NPD_DBUS_ERROR_NO_SUCH_PORT == op_ret){
			//vty_out(vty,"NO SUCH PORT\n");
			dbus_message_unref(reply);
			return CMD_FAILURE;
		}		
	} else {
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
				//vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return CMD_FAILURE;
	}
	return CMD_SUCCESS;	
}
//*******新增

/*add 0:success -1: fail  v1.41*/
int ccgi_get_all_ports_index
(		
  PORT_MEMBER_BMP* portBmp

)
{
	DBusMessage *query, *reply;
	DBusError err;
	int op_ret = 0;
	unsigned int portbmp[2] = {0};


	dbus_error_init(&err);
	query = dbus_message_new_method_call(  \
												NPD_DBUS_BUSNAME, 	\
												NPD_DBUS_ETHPORTS_OBJPATH,	\
												NPD_DBUS_ETHPORTS_INTERFACE, \
												STP_DBUS_METHOD_GET_ALL_PORTS_INDEX_V1
												);

	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
	//	vty_out(vty,"failed get slot count npdReply.\n");
		if (dbus_error_is_set(&err)) {
		//	vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return CMD_FAILURE;
	}

	if (dbus_message_get_args ( reply, &err,
								DBUS_TYPE_UINT32,&op_ret,
								DBUS_TYPE_UINT32,&portbmp[0],
								DBUS_TYPE_UINT32,&portbmp[1],
								DBUS_TYPE_INVALID)) {
		portBmp->portMbr[0]= portbmp[0];
		portBmp->portMbr[1]= portbmp[1];
		dbus_message_unref(reply);
		return CMD_SUCCESS;	
	}
	else {
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
				//vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return CMD_FAILURE;
	}

}


int ccgi_get_port_index_speed
(
	unsigned int port_index,
	unsigned int* speed
)
{
    DBusMessage *query, *reply;
	DBusError err; 
	//int ret = 0,
	int op_ret = 0;
	int value = 0;

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,			\
								NPD_DBUS_ETHPORTS_OBJPATH,		\
								NPD_DBUS_ETHPORTS_INTERFACE,		\
								NPD_DBUS_ETHPORTS_METHOD_STP_GET_PORT_SPEED);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&port_index,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return CMD_FAILURE;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INT32,&value,
		DBUS_TYPE_INVALID)) {
			if (NPD_DBUS_SUCCESS == op_ret ) {
				//DCLI_DEBUG(("success\n"));
				//DCLI_DEBUG(("success,value [%d]\n",value));
				*speed = value;
				//DCLI_DEBUG(("success,lkstate [%d]\n",*lkState);

				dbus_message_unref(reply);
				return CMD_SUCCESS;
			}
			else{
				
				dbus_message_unref(reply);
				return CMD_FAILURE;
			}
	} else {
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return CMD_FAILURE;
	}
}

//*******新增

int ccgi_set_port_duplex_mode_to_stp
(
	unsigned int port_index,
	unsigned int mode
)
{
    DBusMessage *query, *reply;
	DBusError err; 
//	int ret = 0,
	int op_ret = 0;
	//int value = 0;

	query = dbus_message_new_method_call(
								RSTP_DBUS_NAME,    \
								RSTP_DBUS_OBJPATH,	  \
								RSTP_DBUS_INTERFACE,	\
								MSTP_DBUS_METHOD_SET_STP_DUPLEX_MODE);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&port_index,
							 DBUS_TYPE_UINT32,&mode,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return CMD_FAILURE;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)) {
			if (NPD_DBUS_SUCCESS == op_ret ) {
				dbus_message_unref(reply);
				return CMD_SUCCESS;
			}
			else{
				//vty_out(vty,"Set duplex mode %d to port %d failed! op_ret is %d\n",mode,port_index,op_ret);
				dbus_message_unref(reply);
				return CMD_FAILURE;
			}
	} else {
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return CMD_FAILURE;
	}
}


//*******新增

int ccgi_get_port_duplex_mode
(
	unsigned int port_index,
	unsigned int* mode
)
{
    DBusMessage *query, *reply;
	DBusError err; 
	//int ret = 0,
	int op_ret = 0;
	int value = 0;

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,			\
								NPD_DBUS_ETHPORTS_OBJPATH,		\
								NPD_DBUS_ETHPORTS_INTERFACE,		\
								NPD_DBUS_ETHPORTS_METHOD_STP_GET_PORT_DUPLEX_MODE);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&port_index,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return CMD_FAILURE;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INT32,&value,
		DBUS_TYPE_INVALID)) {
			if (NPD_DBUS_SUCCESS == op_ret ) {
				*mode = value;
				dbus_message_unref(reply);
				return CMD_SUCCESS;
			}
			else{
				
				dbus_message_unref(reply);
				return CMD_FAILURE;
			}
	} else {
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return CMD_FAILURE;
	}
}


//*******
int ccgi_get_port_index_link_state
(
	
	unsigned int port_index,
	int* lkState
)
{
	DBusMessage *query, *reply;
	DBusError err; 	
	int op_ret = 0;
	unsigned int value = 0;

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,			\
								NPD_DBUS_ETHPORTS_OBJPATH,		\
								NPD_DBUS_ETHPORTS_INTERFACE,		\
								NPD_DBUS_ETHPORTS_METHOD_STP_GET_PORT_LINK_STATE);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&port_index,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
		//	vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return CMD_FAILURE;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_UINT32,&value,
		DBUS_TYPE_INVALID)) {
			if (NPD_DBUS_SUCCESS == op_ret ) {
				//DCLI_DEBUG(("success\n"));
				//DCLI_DEBUG(("success,value [%d]\n",value));
				*lkState = value;
				//DCLI_DEBUG(("success,lkstate [%d]\n",*lkState);

				dbus_message_unref(reply);
				return CMD_SUCCESS;
			}
			else{
				
				dbus_message_unref(reply);
				return CMD_FAILURE;
			}
	} else {
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return CMD_FAILURE;
	}

}

/*
int ccgi_get_port_index_link_state   //获得端口的连接状态
(
	unsigned int port_index,
	int* lkState
)
{
	DBusMessage *query, *reply;
	DBusError err; 
	int op_ret = 0;
	int value = 0;

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,			\
								NPD_DBUS_ETHPORTS_OBJPATH,		\
								NPD_DBUS_ETHPORTS_INTERFACE,		\
								NPD_DBUS_ETHPORTS_METHOD_STP_GET_PORT_LINK_STATE);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&port_index,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return CMD_FAILURE;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INT32,&value,
		DBUS_TYPE_INVALID)) {
			if (NPD_DBUS_SUCCESS == op_ret ) {
				//DCLI_DEBUG(("success\n"));
				//DCLI_DEBUG(("success,value [%d]\n",value);
				*lkState = value;
				//DCLI_DEBUG(("success,lkstate [%d]\n",*lkState);

				dbus_message_unref(reply);
				return CMD_SUCCESS;
			}
			else{
				
				dbus_message_unref(reply);
				return CMD_FAILURE;
			}
	} else {
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return CMD_FAILURE;
	}

}



//修改
//1: RSTP not enable  3:port not enable 5: port already enabled 6:this port not linked;
int ccgi_enable_stp_on_one_port_to_protocol
(
	unsigned int port_index,
	unsigned int enable,
	int lkState
)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int isEnable = enable;//unsigned
	int op_ret = 0;
	int value = lkState;

	query = dbus_message_new_method_call(
								RSTP_DBUS_NAME,    \
								RSTP_DBUS_OBJPATH,    \
								RSTP_DBUS_INTERFACE,    \
								RSTP_DBUS_METHOD_PORT_STP_ENABLE);
	
	dbus_error_init(&err);

	//DCLI_DEBUG(("ccgi_enable_stp_on_one_port_to_protocol: enable %d,  port_index %d, lkstate %d\n",enable,port_index,lkState));
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&port_index,
							 DBUS_TYPE_UINT32,&isEnable,
							 DBUS_TYPE_INT32,&value,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return CMD_FAILURE;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_INT32,&op_ret,
		DBUS_TYPE_INVALID)) {
			if (NPD_DBUS_SUCCESS == op_ret ) {
				//DCLI_DEBUG(("success\n"));
				dbus_message_unref(reply);
				return CMD_SUCCESS;
			}
			else{
				//DCLI_DEBUG(("enable port %d failed ,ret %02x.\n",port_index,op_ret));
				if(STP_DISABLE== op_ret){
					//vty_out(vty,"RSTP not enable\n");
					return 1;
				}
				else if(STP_PORT_HAVE_ENABLED == op_ret) {
					//vty_out(vty,"this port already enabled\n");
					return 5;//已经打开了此端口
				}
				else if(STP_PORT_NOT_ENABLED == op_ret) {
					//vty_out(vty,"this port not enable\n");
					return 3;
				}// changed on 2008.7.8pm
				else if(STP_PORT_NOT_LINK == op_ret){
					//vty_out(vty, "this port not linked!\n");
					return 6;//此端口未链接 
				}
				else{
					//vty_out(vty,"en/disable error\n");
					return CMD_FAILURE;
				}
				
				dbus_message_unref(reply);
				return CMD_FAILURE;
			
			}
	} 
	else {
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return CMD_FAILURE;
	}

}

*/

//****修改

int ccgi_enable_stp_on_one_port_to_protocol
(
	unsigned int port_index,
	unsigned int enable,
	int lkState,
	unsigned int speed
)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int isEnable = enable;
	//int ret = 0,
	int op_ret = 0;
	int port_link = lkState;
	int port_speed = speed;
	

	query = dbus_message_new_method_call(
								RSTP_DBUS_NAME,    \
								RSTP_DBUS_OBJPATH,    \
								RSTP_DBUS_INTERFACE,    \
								RSTP_DBUS_METHOD_PORT_STP_ENABLE);
	
	dbus_error_init(&err);

	//vty_out(vty,"dcli_enable_stp_on_one_port_to_protocol: enable %d,  port_index %d, lkstate %d\n",enable,port_index,lkState);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&port_index,
							 DBUS_TYPE_UINT32,&isEnable,
							 DBUS_TYPE_UINT32,&port_link,
							 DBUS_TYPE_UINT32,&port_speed,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		/*dbus_message_unref(reply);*/
		return CMD_FAILURE;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_INT32,&op_ret,
		DBUS_TYPE_INVALID)) {
			if (NPD_DBUS_SUCCESS == op_ret ) {
				//DCLI_DEBUG(("success\n"));
				dbus_message_unref(reply);
				return CMD_SUCCESS;
			}
			else{
				//DCLI_DEBUG(("enable port %d failed ,ret %02x.\n",port_index,op_ret));
				if(STP_DISABLE== op_ret){
					//vty_out(vty,"RSTP not enable\n");
					return 1;
				}
				else if(STP_PORT_HAVE_ENABLED == op_ret) {
					//vty_out(vty,"this port already enabled\n");
					return 5;
				}
				else if(STP_PORT_NOT_ENABLED == op_ret) {
					//vty_out(vty,"this port not enable\n");
					return 3;
				}
				else if(STP_PORT_NOT_LINK == op_ret){
					//vty_out(vty, "this port not linked!\n");	
					return 6;
				}
				else{
					//vty_out(vty,"en/disable error\n");
					return CMD_FAILURE;
				}
				
				dbus_message_unref(reply);
				return CMD_FAILURE;
			}
	} 
	else {
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return CMD_FAILURE;
	}

}


//*****

int ccgi_enable_stp_on_one_port_to_npd
(	
	unsigned int mode,
	unsigned int port_index,
	unsigned int enable
)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int isEnable = enable;//unsigned
	int op_ret=0;

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,			\
								NPD_DBUS_ETHPORTS_OBJPATH,		\
								NPD_DBUS_ETHPORTS_INTERFACE,		\
								NPD_DBUS_ETHPORTS_METHOD_CONFIG_STP);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&port_index,
							 DBUS_TYPE_UINT32,&mode,
							 DBUS_TYPE_UINT32,&isEnable,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return CMD_FAILURE;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_INT32,&op_ret,
		DBUS_TYPE_INVALID)) {
			if (NPD_DBUS_SUCCESS == op_ret ) {
				//DCLI_DEBUG(("success\n"));
				dbus_message_unref(reply);
				return CMD_SUCCESS;
			}
			else{
				dbus_message_unref(reply);
				return CMD_FAILURE;
			}
	} else {
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return CMD_FAILURE;
	}

}

int ccgi_stp_set_port_pathcost_to_npd
(
	unsigned int mstid,
	unsigned int port_index,
	unsigned int value
)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int ret;//unsigned
	
	dbus_error_init(&err);
	query = dbus_message_new_method_call(				\
								NPD_DBUS_BUSNAME,			\
								NPD_DBUS_ETHPORTS_OBJPATH,		\
								NPD_DBUS_ETHPORTS_INTERFACE,		\
								RSTP_DBUS_METHOD_CONFIG_PORT_PATHCOST);

	
	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&mstid,
									DBUS_TYPE_UINT32,&port_index,							 								 		
									DBUS_TYPE_UINT32,&value,									
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
								DBUS_TYPE_UINT32,&ret,
								DBUS_TYPE_INVALID)) {

		if(0 != ret) {
			//vty_out(vty,"execute command error\n");
			return CMD_FAILURE;
		}
		
	} else {
		//vty_out(vty,"Failed get args from rstp!.\n");	
		if (dbus_error_is_set(&err)) {
				//vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}
		
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}


int ccgi_stp_set_port_prio_to_npd
(
	unsigned int mstid,
	unsigned int port_index,
	unsigned int value
)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int ret;//unsigned
	
	dbus_error_init(&err);
	query = dbus_message_new_method_call(				\
								NPD_DBUS_BUSNAME,			\
								NPD_DBUS_ETHPORTS_OBJPATH,		\
								NPD_DBUS_ETHPORTS_INTERFACE,		\
								RSTP_DBUS_METHOD_CONFIG_PORTPRIO);

	
	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&mstid,
									DBUS_TYPE_UINT32,&port_index,							 								 		
									DBUS_TYPE_UINT32,&value,									
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
								DBUS_TYPE_UINT32,&ret,
								DBUS_TYPE_INVALID)) {

		if(0 != ret) {
			//vty_out(vty,"execute command error\n");
			return CMD_FAILURE;
		}
		
	} else {
		//vty_out(vty,"Failed get args from rstp!.\n");	
		if (dbus_error_is_set(&err)) {
				//vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}
		
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

int ccgi_stp_set_port_nonstp_to_npd
(
	unsigned int mstid,
	unsigned int port_index,
	unsigned int value
)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int ret;//unsigned
	
	dbus_error_init(&err);
	query = dbus_message_new_method_call(				\
								NPD_DBUS_BUSNAME,			\
								NPD_DBUS_ETHPORTS_OBJPATH,		\
								NPD_DBUS_ETHPORTS_INTERFACE,		\
								RSTP_DBUS_METHOD_CONFIG_NONSTP);

	
	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&port_index,							 								 		
									DBUS_TYPE_UINT32,&value,									
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
								DBUS_TYPE_UINT32,&ret,
								DBUS_TYPE_INVALID)) {

		if(0 != ret) {
			//vty_out(vty,"execute command error\n");
			return CMD_FAILURE;
		}
		
	} else {
		//vty_out(vty,"Failed get args from rstp!.\n");	
		if (dbus_error_is_set(&err)) {
				//vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}
		
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

int ccgi_stp_set_port_p2p_to_npd
(
	unsigned int mstid,
	unsigned int port_index,
	unsigned int value
)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int ret;//unsigned
	
	dbus_error_init(&err);
	query = dbus_message_new_method_call(				\
								NPD_DBUS_BUSNAME,			\
								NPD_DBUS_ETHPORTS_OBJPATH,		\
								NPD_DBUS_ETHPORTS_INTERFACE,		\
								RSTP_DBUS_METHOD_CONFIG_P2P);

	
	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&port_index,							 								 		
									DBUS_TYPE_UINT32,&value,									
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
								DBUS_TYPE_UINT32,&ret,
								DBUS_TYPE_INVALID)) {

		if(0 != ret) {
			//vty_out(vty,"execute command error\n");
			return CMD_FAILURE;
		}
		
	} else {
		//vty_out(vty,"Failed get args from rstp!.\n");	
		if (dbus_error_is_set(&err)) {
				//vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}
		
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

int ccgi_stp_set_port_edge_to_npd
(
	unsigned int mstid,
	unsigned int port_index,
	unsigned int value
)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int ret;//unsigned
	
	dbus_error_init(&err);
	query = dbus_message_new_method_call(				\
								NPD_DBUS_BUSNAME,			\
								NPD_DBUS_ETHPORTS_OBJPATH,		\
								NPD_DBUS_ETHPORTS_INTERFACE,		\
								RSTP_DBUS_METHOD_CONFIG_EDGE);

	
	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&port_index,							 								 		
									DBUS_TYPE_UINT32,&value,									
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
								DBUS_TYPE_UINT32,&ret,
								DBUS_TYPE_INVALID)) {

		if(0 != ret) {
			//vty_out(vty,"execute command error\n");
			return CMD_FAILURE;
		}
		
	} else {
		//vty_out(vty,"Failed get args from rstp!.\n");	
		if (dbus_error_is_set(&err)) {
				//vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}
		
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}


int ccgi_get_br_info
(
	bridge_info *ptbridge_info
)
{
	DBusMessage *brQuery,*brReply;
	DBusError err;
	DBusMessageIter	 brIter,brIter_array,brIter_struct;

	//char buf[10] = {0};

	unsigned  char   root_br_mac[6] ={'\0','\0','\0','\0','\0','\0'};//unsigned char
	unsigned  char   design_br_mac[6] ={'\0','\0','\0','\0','\0','\0'};//unsigned char
	unsigned  int      root_path_cost,design_br_version;//unsigned
	unsigned  short  root_br_prio,design_br_prio;//unsigned
	unsigned  short  root_br_portId;//unsigned
	unsigned  short  root_br_maxAge,design_br_maxAge;//unsigned
	unsigned  short  root_br_hTime,design_br_hTime;//unsigned
	unsigned  short  root_br_fdelay,design_br_fdelay;//unsigned
	
	unsigned int    eth_g_index = 0;
	unsigned char   slot = 0,port = 0;
	int i = 0,ret = 1;
	unsigned int id = 0;


	brQuery = dbus_message_new_method_call(			\
						RSTP_DBUS_NAME,		\
						RSTP_DBUS_OBJPATH,	\
						RSTP_DBUS_INTERFACE, \
						RSTP_DBUS_METHOD_SHOW_BRIDGE_INFO);

	dbus_error_init(&err);
	brReply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,brQuery,-1, &err);
	dbus_message_unref(brQuery);
	if (NULL == brReply) {
		//vty_out(vty,"failed get slot count npdReply.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		//dbus_message_unref(brReply);
		return CMD_FAILURE;
	}

	/*get product id*/
	ccgi_get_broad_product_id(&id);
	dbus_message_iter_init(brReply,&brIter);
	dbus_message_iter_get_basic(&brIter, &ret);
	dbus_message_iter_next(&brIter);
	if(DCLI_STP_OK == ret )
	{

	dbus_message_iter_recurse(&brIter,&brIter_array);
	dbus_message_iter_recurse(&brIter_array,&brIter_struct);

	//display root bridge info
		//vty_out(vty,"-----------------------SPANNING TREE information of STP domain 0------------------------------\n");

		//vty_out(vty,"Designated Root\t\t\t:  ");

		for(i = 0; i< 5; i++)
		{
			dbus_message_iter_get_basic(&brIter_struct,&root_br_mac[i]);
			//vty_out(vty,"%02x:",root_br_mac[i]);
			ptbridge_info->root_br_mac[i] = root_br_mac[i];
			dbus_message_iter_next(&brIter_struct);
		}

		dbus_message_iter_get_basic(&brIter_struct,&design_br_mac[i]);
		//vty_out(vty,"%02x",design_br_mac[5]);
		ptbridge_info->root_br_mac[5] = design_br_mac[5];
		dbus_message_iter_next(&brIter_struct);

		//vty_out(vty,"\nDesignated Root Priority\t:  ");
		dbus_message_iter_get_basic(&brIter_struct,&root_br_prio);
		//vty_out(vty,"%d\n",root_br_prio);
		ptbridge_info->root_br_prio = root_br_prio;
		dbus_message_iter_next(&brIter_struct);

		//vty_out(vty,"Designated Root Path Cost\t:  ");
		dbus_message_iter_get_basic(&brIter_struct,&root_path_cost);
		//vty_out(vty,"%d\n",root_path_cost);
		ptbridge_info->root_path_cost = root_path_cost;
		dbus_message_iter_next(&brIter_struct);


		//vty_out(vty,"Root Port\t\t\t:  ");
		dbus_message_iter_get_basic(&brIter_struct,&root_br_portId);
		
	if(root_br_portId){
			
			eth_g_index = root_br_portId&0xfff;
			//GetSlotPortFromPortIndex(id,eth_g_index,&slot,&port);
			GetSlotPortFromPortIndex_new(eth_g_index,&slot,&port);

			//vty_out(vty,"%d/%d\n",slot,port);
			ptbridge_info->slot=slot;
			ptbridge_info->port=port;
			//sprintf(ptbridge_info->sp,"%d/%d",slot,port);
			ptbridge_info->root_br_portId = root_br_portId;
		}
//		else{
			//vty_out(vty,"%s\n","none");
			
//		}
		dbus_message_iter_next(&brIter_struct);


		//vty_out(vty,"Root Max Age");
		dbus_message_iter_get_basic(&brIter_struct,&root_br_maxAge);
		//vty_out(vty,"%6d\t",root_br_maxAge);
		ptbridge_info->root_br_maxAge = root_br_maxAge;
		dbus_message_iter_next(&brIter_struct);

		//vty_out(vty,"Hello Time");
		dbus_message_iter_get_basic(&brIter_struct,&root_br_hTime);
		//vty_out(vty,"%5d\t\t",root_br_hTime);
		ptbridge_info->root_br_hTime = root_br_hTime;
		dbus_message_iter_next(&brIter_struct);

		//vty_out(vty,"Forward Delay");
		dbus_message_iter_get_basic(&brIter_struct,&root_br_fdelay);
		//vty_out(vty,"%5d\n",root_br_fdelay);
		ptbridge_info->root_br_fdelay = root_br_fdelay;
		dbus_message_iter_next(&brIter_struct);

		//display self-bridge info*/

		//vty_out(vty,"\nBridge ID Mac Address\t\t:  ");

		for(i = 0; i< 5; i++)
		{
			dbus_message_iter_get_basic(&brIter_struct,&design_br_mac[i]);
			//vty_out(vty,"%02x:",design_br_mac[i]);
			ptbridge_info->design_br_mac[i] = design_br_mac[i];
			dbus_message_iter_next(&brIter_struct);
		}

		dbus_message_iter_get_basic(&brIter_struct,&design_br_mac[i]);
		//vty_out(vty,"%02x",design_br_mac[5]);
		ptbridge_info->design_br_mac[5] = design_br_mac[5];
		dbus_message_iter_next(&brIter_struct);

		//vty_out(vty,"\nBridge ID Priority\t\t:  ");
		dbus_message_iter_get_basic(&brIter_struct,&design_br_prio);
		//vty_out(vty,"%d\n",design_br_prio);
		ptbridge_info->design_br_prio = design_br_prio;
		dbus_message_iter_next(&brIter_struct);

		//vty_out(vty,"Bridge ID ForceVersion\t\t:  ");
		dbus_message_iter_get_basic(&brIter_struct,&design_br_version);
		//vty_out(vty,"%d\n",design_br_version);
		ptbridge_info->design_br_version = design_br_version;
		dbus_message_iter_next(&brIter_struct);

		//vty_out(vty,"Bridge Max Age");
		dbus_message_iter_get_basic(&brIter_struct,&design_br_maxAge);
		//vty_out(vty,"%5d\t",design_br_maxAge);
		ptbridge_info->design_br_maxAge = design_br_maxAge;
		dbus_message_iter_next(&brIter_struct);

		//(vty,"Hello Time");
		dbus_message_iter_get_basic(&brIter_struct,&design_br_hTime);
		//vty_out(vty,"%5d\t\t",design_br_hTime);
		ptbridge_info->design_br_hTime = design_br_hTime;
		dbus_message_iter_next(&brIter_struct);

		//vty_out(vty,"Forward Delay");
		dbus_message_iter_get_basic(&brIter_struct,&design_br_fdelay);
		//vty_out(vty,"%5d\n",design_br_fdelay);
		ptbridge_info->design_br_fdelay = design_br_fdelay;
		dbus_message_iter_next(&brIter_struct);
		dbus_message_unref(brReply);
		return CMD_SUCCESS;
	}
	else if(STP_DISABLE == ret)
	{
		dbus_message_unref(brReply);
		return ret;
	}
	else
	{
		dbus_message_unref(brReply);
		return ret;
	}
	
}

/*follow-up  09-03-24 */

int ccgi_get_one_port_info
(
	unsigned int port_index,
	unsigned int portductid,
	port_info *ptport_info
)
{
	DBusMessage *stpQuery,*stpReply;
	DBusError err;
	DBusMessageIter	 stpIter;
	DBusMessageIter  stpIter_array;
	DBusMessageIter	 stpIter_struct;
	DBusMessageIter	 stpIter_sub_struct;

	char buf[10] = {0};
	int n,ret;

	unsigned char  port_prio;
	unsigned int 	port_cost;
	int 					port_role;
	int 					port_state;
	int 					port_lk;
	int 					port_p2p;
	int 					port_edge;
	unsigned short br_prio;
	unsigned char   mac[6] ={'\0','\0','\0','\0','\0','\0'};
	unsigned int     br_cost;
	unsigned short br_dPort;
	//fprintf(stderr,"####port_prio%-4d port_cost%-9d port_role%-5s port_state%-13s port_lk%-3s",port_prio,port_cost,stp_port_role[port_role],stp_port_state[port_state],port_lk ? "Y" : "N");

	stpQuery = dbus_message_new_method_call(			\
								RSTP_DBUS_NAME,		\
								RSTP_DBUS_OBJPATH,	\
								RSTP_DBUS_INTERFACE, \
								RSTP_DBUS_METHOD_SHOW_SPANTREE);

			dbus_message_append_args(stpQuery,
							 DBUS_TYPE_UINT32,&(port_index),
							 DBUS_TYPE_INVALID);
			dbus_error_init(&err);
			stpReply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,stpQuery,-1, &err);


			dbus_message_unref(stpQuery);
			if (NULL == stpReply) {
				//vty_out(vty,"failed get stp stpReply.\n");
				if (dbus_error_is_set(&err)) {
					//vty_out(vty,"%s raised: %s",err.name,err.message);
					dbus_error_free(&err);
				}
				dbus_message_unref(stpReply);
				return CMD_FAILURE;
			}

	dbus_message_iter_init(stpReply,&stpIter);				
	dbus_message_iter_get_basic(&stpIter, &ret);
	dbus_message_iter_next(&stpIter);
	if(DCLI_STP_OK == ret )
	{
		/*		
			Array of Port Infos.
			port no
			port prio
			port role
			port State
			port link
			port p2p
			port edge
			port Desi bridge
			port Dcost
			port D-port
		*/

		dbus_message_iter_recurse(&stpIter,&stpIter_array);
		dbus_message_iter_recurse(&stpIter_array,&stpIter_struct);	
		dbus_message_iter_get_basic(&stpIter_struct,&port_prio);
		
		dbus_message_iter_next(&stpIter_struct);
		dbus_message_iter_get_basic(&stpIter_struct,&port_cost);

		dbus_message_iter_next(&stpIter_struct);
		dbus_message_iter_get_basic(&stpIter_struct,&port_role);

		dbus_message_iter_next(&stpIter_struct);
		dbus_message_iter_get_basic(&stpIter_struct,&port_state);

		dbus_message_iter_next(&stpIter_struct);
		dbus_message_iter_get_basic(&stpIter_struct,&port_lk);

		dbus_message_iter_next(&stpIter_struct);
		dbus_message_iter_get_basic(&stpIter_struct,&port_p2p);

		dbus_message_iter_next(&stpIter_struct);
		dbus_message_iter_get_basic(&stpIter_struct,&port_edge);

		dbus_message_iter_next(&stpIter_struct);
		dbus_message_iter_get_basic(&stpIter_struct,&br_cost);			

		dbus_message_iter_next(&stpIter_struct);			
		dbus_message_iter_get_basic(&stpIter_struct,&br_dPort);

		dbus_message_iter_next(&stpIter_struct);
		dbus_message_iter_recurse(&stpIter_struct,&stpIter_sub_struct);


		dbus_message_iter_get_basic(&stpIter_sub_struct,&br_prio);
		dbus_message_iter_next(&stpIter_sub_struct);

		for(n = 0; n < 6; n++)
		{
			dbus_message_iter_get_basic(&stpIter_sub_struct,&mac[n]);
			dbus_message_iter_next(&stpIter_sub_struct);
		}						


		ptport_info->port_prio = port_prio;
		ptport_info->port_cost = port_cost;
		ptport_info->port_role = port_role;
		ptport_info->port_state = port_state;
		ptport_info->port_lk = port_lk;
		ptport_info->port_p2p = port_p2p;
		ptport_info->port_edge = port_edge;
		ptport_info->br_prio = br_prio;
		ptport_info->br_cost = br_cost;
		ptport_info->br_dPort = br_dPort;
		for(n = 0; n < 6; n++)
		{
			ptport_info->mac[n] = mac[n];
		}

				if(strcmp("NStp",stp_port_role[port_role]) == 0) {
		
					//vty_out(vty,"%-4s","-");
					//vty_out(vty,"%-9s","-");
				   // vty_out(vty,"%-5s",stp_port_role[port_role]);
					//vty_out(vty,"%-13s","-");
					//vty_out(vty,"%-3s", "-");
		
					if(0 == port_p2p)
						//vty_out(vty,"%-4s","-");
						;
					else if(1 == port_p2p)
						//vty_out(vty,"%-4s","-");
						;
					else if(2 == port_p2p)
						//vty_out(vty,"%-4s","-");
						;
					
					//vty_out(vty,"%-5s",port_edge ? "-" : "-");
		
					memset(buf,0,sizeof(buf));
					sprintf(buf,"%s","-");
		
				   memset(buf,0,sizeof(buf));
				   sprintf(buf,"%s","-");
				   //vty_out(vty,"%s","");
						
				  // vty_out(vty,"\n");
		
				}
				else
				{

		//vty_out(vty,"%-4d",port_prio);
		//vty_out(vty,"%-9d",port_cost);
		//vty_out(vty,"%-5s",stp_port_role[port_role]);
		//vty_out(vty,"%-13s",stp_port_state[port_state]);
		//vty_out(vty,"%-3s", port_lk ? "Y" : "N");		
		if(0 == port_p2p)
			//vty_out(vty,"%-4s","Y")
			//vty_out(vty,"%-4s","N")   new 
			;
		else if(1 == port_p2p)
			//vty_out(vty,"%-4s","N")
			//vty_out(vty,"%-4s","Y")   new 
			;
		else if(2 == port_p2p)
			//vty_out(vty,"%-4s","A")
			;
		
		//vty_out(vty,"%-5s",port_edge ? "Y" : "N");

		memset(buf,0,sizeof(buf));
		sprintf(buf,"%d",br_prio);
		if(port_state)
		{				
			//vty_out(vty,"%-5s:",port_state ? buf : "");
			for(n = 0; n < 6; n++)
			{
				//vty_out(vty,"%02x",mac[n]);
			}
			//vty_out(vty,"  ");
		}
		
		memset(buf,0,sizeof(buf));
		sprintf(buf,"%d",br_cost);		
		//vty_out(vty,"%-10s",port_state ? buf : "");
		//只是输出格式有改变，(命令行下)
		//if((strcmp("NStp",stp_port_role[port_role]))){
		  // vty_out(vty,"%-10s",port_state ? buf : "");
		//}
		//else {
			//vty_out(vty,"%-10s","     -");
		//}

		memset(buf,0,sizeof(buf));
		sprintf(buf,"%#0x",br_dPort);
		//vty_out(vty,"%s",port_state ? buf : "");
				
		//vty_out(vty,"\n");
		//fprintf(stderr,"port_prio%-4d port_cost%-9d port_role%-5s port_state%-13s port_lk%-3s",port_prio,port_cost,stp_port_role[port_role],stp_port_state[port_state],port_lk ? "Y" : "N");
		}
		dbus_message_unref(stpReply);
		return CMD_SUCCESS;
		
	}
	else if(STP_DISABLE == ret)
	{
		dbus_message_unref(stpReply);
		return ret;
	}
	else
	{
		dbus_message_unref(stpReply);
		return ret;
	}
}




int ccgi_get_vlan_portmap
(
	VLAN_PORTS_BMP** ports_bmp,
	unsigned int* count
)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;

	unsigned int vlan_Cont = 0;
	unsigned short  vlanId = 0;
	char*			vlanName = NULL;
	//unsigned int	untagBmp = 0,tagBmp = 0;
	PORT_MEMBER_BMP	untagBmp,tagBmp;
	unsigned int j,ret;
	//unsigned int vlanStat = 0;
	VLAN_PORTS_BMP* tmp = NULL;
	unsigned int 	product_id = PRODUCT_ID_NONE;
	unsigned char trkTagMode;

	unsigned int    promisPortBmp[2] = {0};
    memset(&untagBmp,0,sizeof(PORT_MEMBER_BMP));
	memset(&tagBmp,0,sizeof(PORT_MEMBER_BMP));


	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_VLAN_OBJPATH ,	\
										NPD_DBUS_VLAN_INTERFACE ,	\
										NPD_DBUS_VLAN_METHOD_SHOW_VLANLIST_PORT_MEMBERS_V1 );
	

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

	dbus_message_iter_get_basic(&iter,&ret);
	if(0 == ret){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&vlan_Cont);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&product_id);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&promisPortBmp[0]);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&promisPortBmp[1]);

		//DCLI_DEBUG(("get basic return value actVlanCont =%d\n",vlan_Cont);
		tmp = (VLAN_PORTS_BMP*)malloc(vlan_Cont * sizeof(VLAN_PORTS_BMP));
		if(!tmp)
			return CMD_FAILURE;
		
		memset(tmp,0,vlan_Cont * sizeof(VLAN_PORTS_BMP));
		*ports_bmp = tmp ;
		*count = vlan_Cont;
		//DCLI_DEBUG(("tmp %p,count %d\n",tmp,*count));
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);

		for (j = 0; j < vlan_Cont; j++) {

			DBusMessageIter iter_struct;
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&vlanId);
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&vlanName);
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&untagBmp.portMbr[0]);
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&untagBmp.portMbr[1]);
			dbus_message_iter_next(&iter_struct);	
			
			dbus_message_iter_get_basic(&iter_struct,&tagBmp.portMbr[0]);
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&tagBmp.portMbr[1]);


			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&trkTagMode);
			
			dbus_message_iter_next(&iter_array);
			//DCLI_DEBUG(("untagbmp %02x,tagbmp %02x\n",untagBmp,tagBmp));
			tmp->vid = vlanId;
			tmp->untagbmp.portMbr[0]= untagBmp.portMbr[0];
			tmp->untagbmp.portMbr[1]= untagBmp.portMbr[1];			
			tmp->tagbmp.portMbr[0]= tagBmp.portMbr[0];
			tmp->tagbmp.portMbr[1]= tagBmp.portMbr[1];

			//DCLI_DEBUG(("1318::tmp ++ %02x,tagbmp %02x\n",untagBmp,tagBmp));
			tmp++;
			
		}
	}
	else if (NPD_VLAN_ERR_HW_STP == ret) {
		//DCLI_DEBUG(("Error occurs in initing mstp\n"));
		return CMD_FAILURE;
	}
	dbus_message_unref(reply);
	//DCLI_DEBUG(("715 :: dcli_get_vlan_portsbmp end\n"));
	return CMD_SUCCESS;
	
}


int ccgi_enable_g_stp_to_npd(unsigned int enable,DBusConnection *connection)
{
	DBusMessage *query, *reply;
	DBusError err; 
	int op_ret;

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,			\
								NPD_DBUS_ETHPORTS_OBJPATH,		\
								NPD_DBUS_ETHPORTS_INTERFACE,		\
								NPD_DBUS_ETHPORTS_METHOD_CONFIG_G_ALL_STP);
    dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&enable,
							DBUS_TYPE_INVALID);
	dbus_error_init(&err);
	
	reply = dbus_connection_send_with_reply_and_block (connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return CMD_FAILURE;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)) {
			if (NPD_DBUS_SUCCESS == op_ret ) {
				//DCLI_DEBUG(("success\n"));
				dbus_message_unref(reply);
				return CMD_SUCCESS;
			}
			else{
				dbus_message_unref(reply);
				return CMD_FAILURE;
			}
	} else {
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return CMD_FAILURE;
	}
}



int ccgi_send_vlanbmp_to_mstp
(
	VLAN_PORTS_BMP* ports_bmp
)
{
	DBusMessage *query, *reply;
	DBusError err;
	int op_ret;

	query = dbus_message_new_method_call(
									RSTP_DBUS_NAME,    \
									RSTP_DBUS_OBJPATH,    \
									RSTP_DBUS_INTERFACE,    \
									STP_DBUS_METHOD_INIT_MSTP_V1);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT16,&(ports_bmp->vid),
							 DBUS_TYPE_UINT32,&(ports_bmp->untagbmp.portMbr[0]),
							 DBUS_TYPE_UINT32,&(ports_bmp->untagbmp.portMbr[1]),
							 DBUS_TYPE_UINT32,&(ports_bmp->tagbmp.portMbr[0]),
							 DBUS_TYPE_UINT32,&(ports_bmp->tagbmp.portMbr[1]),
							 DBUS_TYPE_UINT32,&productid,
							 DBUS_TYPE_INVALID);


	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return CMD_FAILURE;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)) {
			if (NPD_DBUS_SUCCESS == op_ret ) {
				//DCLI_DEBUG(("757 :: success\n"));
				dbus_message_unref(reply);
				return CMD_SUCCESS;
			}
			else{
				if(0xff == op_ret){
					//vty_out(vty,"MSTP hasn't enable\n");
					return 2;
				}
				else{
					//vty_out(vty,"MSTP en/disable error\n");
				}
				dbus_message_unref(reply);
				return CMD_FAILURE;
			}
	} else {
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




//检查端口状态
int check_port_state(char *port)//0 :enable  -1:disable
{
	unsigned int ret;//unsigned
	int state = -1;
	unsigned char slot_no,port_no;//unsigned char
	unsigned int port_index;//unsigned


	DBusMessage *stpQuery,*stpReply;
	DBusError err;
	DBusMessageIter	 stpIter;
//	DBusMessageIter	 stpIter_struct;
//	DBusMessageIter	 stpIter_sub_struct;

	//	SLOT_INFO slot_info[6];

	ret = parse_slotport_no((unsigned char *)port,&slot_no,&port_no);//(unsigned char*)port
	if (NPD_FAIL == ret) {
		return -1;
	}

	if(ccgi_get_one_port_index(slot_no,port_no,&port_index) < 0)	{
		return -1;
	}


	stpQuery = dbus_message_new_method_call(			\
								RSTP_DBUS_NAME,		\
								RSTP_DBUS_OBJPATH,	\
								RSTP_DBUS_INTERFACE, \
								RSTP_DBUS_METHOD_SHOW_SPANTREE);

			dbus_message_append_args(stpQuery,
							 DBUS_TYPE_UINT32,&(port_index),
							 DBUS_TYPE_INVALID);
			dbus_error_init(&err);
			stpReply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,stpQuery,-1, &err);


			dbus_message_unref(stpQuery);
			if (NULL == stpReply) {
				//vty_out(vty,"failed get stp stpReply.\n");
				if (dbus_error_is_set(&err)) {
					//vty_out(vty,"%s raised: %s",err.name,err.message);
					dbus_error_free(&err);
				}
				dbus_message_unref(stpReply);
				return CMD_FAILURE;
			}

	dbus_message_iter_init(stpReply,&stpIter);				
	dbus_message_iter_get_basic(&stpIter, &state);
	dbus_message_iter_next(&stpIter);

	if(DCLI_STP_OK == state)
	{
		state = 0;
	}

	return state;

}


int ccgi_change_all_ports_to_bmp
(
	VLAN_PORTS_BMP* ports_bmp,
	unsigned int *num
)
{
	PORT_MEMBER_BMP portBmp;
	int ret = 0;
	//SLOT_INFO slot_info[6] = {{0}};
    memset(&portBmp,0,sizeof(PORT_MEMBER_BMP));

	*num = 0;
	ports_bmp->vid = 0;  //init stp ports,create port struct,vid 0 is flag
	//ret = dcli_get_all_ports_index(vty,&portBmp);
	ret = ccgi_get_all_ports_index(&portBmp);
	if(0 != ret)
		return CMD_FAILURE;
	else {  //init cist ports
		//ports_bmp->untagbmp = portBmp;
		//ports_bmp->tagbmp = 0;

		memcpy(&(ports_bmp->untagbmp),&portBmp,sizeof(PORT_MEMBER_BMP));
		memset(&(ports_bmp->tagbmp),0,sizeof(PORT_MEMBER_BMP));

		ports_bmp->vid = 0;
	}
	return 0;
}



//MSTP
//设置桥域的名称,0:succ,-1:fail,-2:MSTP not enable

/*checked 08-12-31*/
int config_spanning_tree_region_name(char *region_name)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int ret;
	unsigned int value;
	char* name = g_regionName;
	
	value = strlen (region_name);
	if(value > 31){
		//vty_out(vty,"input name too many characters\n");
		return  CMD_FAILURE;
	}
	memset(name,'\0',32);
	strcpy(name,region_name);

//	DCLI_DEBUG(("bridge name %s\n",name));
	query = dbus_message_new_method_call(
									RSTP_DBUS_NAME,    \
									RSTP_DBUS_OBJPATH,    \
									RSTP_DBUS_INTERFACE,    \
									MSTP_DBUS_METHOD_CONFIG_REG_NAME);

    dbus_message_append_args(query,
									DBUS_TYPE_STRING,&name,
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

	//#ifdef STP_FULL_DEBUG
	//vty_out(vty,"test recv message\n");
	//#endif
	if (dbus_message_get_args ( reply, &err,
									DBUS_TYPE_UINT32,&ret,
									DBUS_TYPE_INVALID)) {
		//#ifdef STP_FULL_DEBUG
		//vty_out(vty," region name: %s	return-value %d\n",name,ret);
		//#endif
		if(STP_DISABLE == ret){
			//vty_out(vty,"MSTP not enable\n");
			return -2;//MSTP not enable
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
		
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}


//设置桥域的版本号 0:succ ,-1:fail, -2:MSTP not enable

/*follow-up 08-12-31 */
int config_spanning_tree_revision(char *br_revision)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int ret;
	unsigned int value = 0;
	
	//value = strtoul (br_revision,0,10);
	//if(value < MIN_BR_REVISION || value > MAX_BR_REVISION ){
		//vty_out(vty,"input revision value out range <0-65535>\n");
	//	return  CMD_FAILURE;
		
	//}

	if((strtoul (br_revision,0,10) < MIN_BR_REVISION) || (strtoul (br_revision,0,10) > MAX_BR_REVISION )){
		//vty_out(vty,"input revision value out range <0-65535>\n");
		return  CMD_FAILURE;
	}
	else {
		value = strtoul (br_revision,0,10);
	}

	//DCLI_DEBUG(("input revision %d\n",value));
	
	query = dbus_message_new_method_call(
									RSTP_DBUS_NAME,    \
									RSTP_DBUS_OBJPATH,    \
									RSTP_DBUS_INTERFACE,    \
									MSTP_DBUS_METHOD_CONFIG_BR_REVISION);

    dbus_message_append_args(query,
									DBUS_TYPE_UINT16,&value,
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

	if (dbus_message_get_args ( reply, &err,
									DBUS_TYPE_UINT32,&ret,
									DBUS_TYPE_INVALID)) {
		//DCLI_DEBUG(("input value %d, return value %d\n",value,ret));
		if(STP_DISABLE == ret){
			//vty_out(vty,"MSTP not enable\n");
			return -2;
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
		
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}


//配置指定域的优先级 0:succ , -1:fail ,-2: MSTP not enable

/*follow-up 08-12-31 */
int config_spanning_tree_mstp_prio(char *instanceID, char *prio)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int ret;
	unsigned int mstid = 0,value = 0;

	mstid = strtoul(instanceID,0,10);
	value = strtoul(prio,0,10);
	if(value < MIN_BR_PRIO || value > MAX_BR_PRIO  || 
		mstid < MIN_MST_ID || mstid > MAX_MST_ID){
		//vty_out(vty,"input priority value out range \n");
		return  CMD_FAILURE;
	}
	else{
		if(0 != (value%4096)){
			//vty_out (vty,"spanning-tree priority value must be times of 4096!\n");
			return CMD_FAILURE;
		}
	}
	
	query = dbus_message_new_method_call(
									RSTP_DBUS_NAME,    \
									RSTP_DBUS_OBJPATH,    \
									RSTP_DBUS_INTERFACE,    \
									RSTP_DBUS_METHOD_CONFIG_PRIO);

    dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&mstid,
									DBUS_TYPE_UINT32,&value,
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

	if (dbus_message_get_args ( reply, &err,
									DBUS_TYPE_UINT32,&ret,
									DBUS_TYPE_INVALID)) {
		if(STP_DISABLE == ret){
			//vty_out(vty,"MSTP hasn't enabled\n");
			//return -2;
			
			dbus_message_unref(reply);
			return -2;

		}
		else if(STP_Vlan_Had_Not_Yet_Been_Created == ret){
			//vty_out(vty,"STP does not exist yet!\n");
			dbus_message_unref(reply);
			//return CMD_WARNING;
			return -1;

		}
	} 
	else {
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
	    //return CMD_WARNING;
		return CMD_FAILURE;
	}
		
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}


//配置报文的最大跳数 0:succ , -1:fail ,-2: MSTP not enable 3,small 4,large

/*checked 08-12-31 */
int config_spanning_tree_max_hops(char *max_hops)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int ret;
	unsigned int value=0;
	
	value = strtoul (max_hops,0,10);
	if(value < MIN_REMAINING_HOPS || value > MAX_REMAINING_HOPS )
	{
		//vty_out(vty,"input max-hops value out range <6-40>\n");
		return CMD_FAILURE;
	}
	
	query = dbus_message_new_method_call(		\
									RSTP_DBUS_NAME,	\
									RSTP_DBUS_OBJPATH,	\
									RSTP_DBUS_INTERFACE,	\
									MSTP_DBUS_METHOD_CONFIG_MAXHOPS);

	dbus_error_init(&err);
	dbus_message_append_args(query,
								DBUS_TYPE_UINT32,&value,
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
									DBUS_TYPE_UINT32,&ret,
									DBUS_TYPE_INVALID)) {
		if(STP_DISABLE == ret){
			//vty_out(vty,"MSTP hasn't enabled\n");
			return -2;
		}
		
		else if(STP_Small_Max_Hops== ret){
           // vty_out(vty,PRINTF_STP_Small_Max_Hops);
           return 3;
		}
		else if(STP_Large_Max_Hops== ret){
            //vty_out(vty,PRINTF_STP_Large_Max_Hops);
            return 4;
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
		
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}


//配置端口路径开销 0:succ , -1:fail ,-2: MSTP not enable -3:port not enable -4:已经是auto值了
/*follow-up 08-12-31 */
int config_spanning_tree_path_cost_mstp(char *mstID, char *port, char *path_cast)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int value,mstid = 0;
	unsigned char slot_no=0;
	unsigned char port_no=0;
	unsigned int ret;
	unsigned int port_index;
	
	mstid =  strtoul (mstID,0,10);
	if(mstid < MIN_MST_ID || mstid > MAX_MST_ID) {
		//vty_out(vty,"input para out range.\n");
		return CMD_FAILURE;
	}
	ret = parse_slotport_no((unsigned char *)port,&slot_no,&port_no);
	if (NPD_FAIL == ret) {
    	//vty_out(vty,"Unknow port number format.\n");
		return CMD_SUCCESS;
	}
	
	if (strncmp("auto",path_cast , strlen(path_cast))== 0){
		value = 200000000;
	}
	else {
		value = strtoul (path_cast,0,10);
		if (!((value > 0)&&(value % 20 ==0)&& (value<=200000000))){
			//vty_out (vty,"path cost must be times of 20!\n");
			return CMD_FAILURE;
			}
		//已经是auto，但是没有处理
		else if(ADMIN_PORT_PATH_COST_AUTO == value){
           // vty_out(vty,"The path-cost 200000000 is auto value!");
           return -4;
		}		

		}
	if( 0!= ccgi_get_one_port_index(slot_no,port_no,&port_index))	{
		//DCLI_DEBUG(("execute command failed\n"));
		return CMD_FAILURE;
	}

	//DCLI_DEBUG(("mstid %d, slot/port %d/%d,value %d\n",mstid,slot_no,port_no,value));
	dbus_error_init(&err);
	query = dbus_message_new_method_call(				\
										RSTP_DBUS_NAME,		\
										RSTP_DBUS_OBJPATH,	\
										RSTP_DBUS_INTERFACE, \
										RSTP_DBUS_METHOD_CONFIG_PORT_PATHCOST);

	
	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&mstid,
									DBUS_TYPE_UINT32,&port_index,							 	
									DBUS_TYPE_UINT32,&value,									
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
								DBUS_TYPE_UINT32,&ret,
								DBUS_TYPE_INVALID)) {
		if(STP_DISABLE == ret){
			//vty_out(vty,PRINTF_RSTP_NOT_ENABLED);
			return -2;//stp not enable
		}
		else if(STP_PORT_NOT_ENABLED == ret) {
			//vty_out(vty,PRINTF_PORT_NOT_ENABLED);
			return -3;//stp port not enable
					}
		else if(STP_Cannot_Find_Vlan == ret){
            //vty_out(vty,"Port not in the mst!\n");
            return -1;
		}
	//	else if(0 == ret)
	//		ccgi_stp_set_port_pathcost_to_npd(mstid,port_index,value);
		
	} else {
		//vty_out(vty,"Failed get args from rstp!.\n");	
		if (dbus_error_is_set(&err)) {
				//vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}
		
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;
}




//配置端口优先级 0:succ , -1:fail ,-2: MSTP not enable -3:port not enable
/*follow-up 08-12-31 */
int config_spanning_tree_port_prio_mstp(char *mstID, char *port, char *prio)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int port_index;
	unsigned int value,mstid = 0;
	unsigned char slot_no,port_no;
	unsigned int ret;


	mstid =  strtoul (mstID,0,10);
	if(mstid < MIN_MST_ID || mstid > MAX_MST_ID) {
		//vty_out(vty,"input para out range.\n");
		return CMD_FAILURE;
	}
	ret = parse_slotport_no((unsigned char *)port,&slot_no,&port_no);
	if (NPD_FAIL == ret) {
    	//vty_out(vty,"Unknow port number format.\n");
		return CMD_FAILURE;
	}
	
	value = strtoul (prio,0,10);
	if(value < MIN_PORT_PRIO || value > MAX_PORT_PRIO ){
		//vty_out(vty,"input port-priority value out range <0-240>\n");
		return CMD_FAILURE;
	}
	else{
		if(0 != (value%16)){
			//vty_out (vty,"spanning-tree port-priority value must be times of 16!\n");
			return CMD_FAILURE;
		}
	}

	if( 0!= ccgi_get_one_port_index(slot_no,port_no,&port_index))	{
		//DCLI_DEBUG(("execute command failed\n"));
		return CMD_FAILURE;
	}

	//DCLI_DEBUG(("mstid %d, slot/port %d/%d,value %d\n",mstid,slot_no,port_no,value));
	dbus_error_init(&err);	
	query = dbus_message_new_method_call(				\
										RSTP_DBUS_NAME,		\
										RSTP_DBUS_OBJPATH,	\
										RSTP_DBUS_INTERFACE, \
										RSTP_DBUS_METHOD_CONFIG_PORTPRIO);
	
	dbus_message_append_args(query,
						DBUS_TYPE_UINT32,&mstid,
						DBUS_TYPE_UINT32,&port_index,
						DBUS_TYPE_UINT32,&value,
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
						DBUS_TYPE_UINT32,&ret,
						DBUS_TYPE_INVALID)) {
		if(STP_DISABLE == ret){
			//vty_out(vty,PRINTF_RSTP_NOT_ENABLED);
			return -2;
		}
		else if(STP_PORT_NOT_ENABLED == ret) {
			//vty_out(vty,PRINTF_PORT_NOT_ENABLED);
			return -3;
			}
		else if(STP_Cannot_Find_Vlan == ret){
           // vty_out(vty,"Port not in the mst!\n");
           return -1;
		}

		
		//else if(0 == ret)
		//	ccgi_stp_set_port_prio_to_npd(mstid,port_index,value);
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


//配置桥恢复默认 0:succ , -1:fail ,-2: MSTP not enable -3:port not enable????????????????????????????????????????
int config_spanning_tree_default_mstp(char *mstID)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int value = 0 ,ret;

	value = strtoul (mstID,0,10);
	if (value < MIN_MST_ID || value > MAX_MST_ID ) {
		//vty_out (vty,"input param out range!\n");
		return CMD_FAILURE;
	}
	
	query = dbus_message_new_method_call(				\
									RSTP_DBUS_NAME,			\
									RSTP_DBUS_OBJPATH,		\
									RSTP_DBUS_INTERFACE,	\
									RSTP_DBUS_METHOD_CONFIG_NOCONFIG);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&value,
							DBUS_TYPE_INVALID);

	//DCLI_DEBUG(("mstid %d\n",value));
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
								DBUS_TYPE_UINT32,&ret,
								DBUS_TYPE_INVALID)) {
		if(STP_DISABLE == ret){
			//vty_out(vty,PRINTF_RSTP_NOT_ENABLED);
			return -2;
		}
		else if(STP_PORT_NOT_ENABLED == ret) {
			//vty_out(vty,PRINTF_PORT_NOT_ENABLED);
			return -3;
		}
	}	
	else {
		
		//vty_out(vty," Failed get args.\n");
		if (dbus_error_is_set(&err)) {
				//vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}
		
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;
}



//配置端口恢复默认 0:succ , -1:fail ,-2: MSTP not enable -3:port not enable
/*follow-up 08-12-31 */
int config_spanning_tree_port_default_mstp(char *mstID, char *port)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int ret;
	unsigned char slot_no,port_no;
	unsigned int port_index,mstid = 0;


	mstid =  strtoul (mstID,0,10);
	if(mstid < MIN_MST_ID || mstid > MAX_MST_ID) {
		//vty_out(vty,"input para out range.\n");
		return CMD_FAILURE;
	}
	
	ret = parse_slotport_no((unsigned char *)port,&slot_no,&port_no);
	if (NPD_FAIL == ret) {
    	//vty_out(vty,"Unknown port number format!\n");
		return CMD_SUCCESS;
	}
	
	dbus_error_init(&err);

if( 0!= ccgi_get_one_port_index(slot_no,port_no,&port_index))	{
		//DCLI_DEBUG(("execute command failed\n"));
		return CMD_FAILURE;
	}

	//DCLI_DEBUG(("mstid %d, slot/port %d/%d\n",mstid,slot_no,port_no));
	query = dbus_message_new_method_call(			\
									RSTP_DBUS_NAME,		\
									RSTP_DBUS_OBJPATH,	\
									RSTP_DBUS_INTERFACE, \
									RSTP_DBUS_METHOD_CONFIG_PORT_NOCONFIG);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&mstid,
									DBUS_TYPE_UINT32,&port_index,				 		
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
							DBUS_TYPE_UINT32,&ret,
							DBUS_TYPE_INVALID)) {
		if(STP_DISABLE == ret){
			//vty_out(vty,PRINTF_MSTP_NOT_ENABLED);
			return -2;
		}
		else if(STP_PORT_NOT_ENABLED == ret) {
			//vty_out(vty,PRINTF_PORT_NOT_ENABLED);
			return -3;
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
		
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}


//把指定vlan添加到指定实例中  0:succ , -1:fail ,-2: MSTP not enable
/* follow-up 08-12-31 */
int config_spanning_tree_map(char *vlanID, char *mstID)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int ret = -1;
	unsigned short vid = 0;
	unsigned int mstid = 0;
	VLAN_PORTS_BMP ports_bmp = {0}; 

	

	vid =  strtoul (vlanID,0,10);
	/*
	if(vid < 1 || vid > 4094) {
		//vty_out(vty,"input para out range.\n");
		return CMD_FAILURE;
	}
	*/
	mstid =  strtoul (mstID,0,10);
	if(mstid < MIN_MST_ID || mstid > MAX_MST_ID) {
		//vty_out(vty,"input param out range.\n");
		
		return CMD_FAILURE;
	}

	ret = ccgi_get_one_vlan_portmap(vid,&ports_bmp);
	if(CMD_SUCCESS != ret)
	{
		fprintf(stderr,"2222vid=%d22222",vid);
		return CMD_FAILURE;
	}
	dbus_error_init(&err);
	//DCLI_DEBUG(("set vid %d on mstid %d\n",vid,mstid));
	query = dbus_message_new_method_call(			\
									RSTP_DBUS_NAME,		\
									RSTP_DBUS_OBJPATH,	\
									RSTP_DBUS_INTERFACE, \
									MSTP_DBUS_METHOD_CFG_VLAN_ON_MST);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT16,&vid,
									DBUS_TYPE_UINT32,&mstid,							 									 		
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
							DBUS_TYPE_UINT32,&ret,
							DBUS_TYPE_INVALID)) {

		if(STP_DISABLE == ret){
			//vty_out(vty,PRINTF_MSTP_NOT_ENABLED);
			return -2;
					}
		//不另外处理
		else if(STP_Cannot_Find_Vlan == ret){
			//vty_out(vty,"The vlan %d not exist!\n ",vid);
			return -1; 

		}
		else if(0 == ret) {
			ccgi_stp_set_stpid_to_npd(vid,mstid);
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
		
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}


//显示指定域中所有实例的状态信息  0:succ , -1:fail ,-2: MSTP not enable
//未用函数
int config_spanning_tree_all_instance(char *mstID)
{	
	//unsigned int ret;
	//unsigned short vid = 0;
	//unsigned int mstid = 0;

	


	/*get portbmp*/
	/*get bridge info*/
	/*get port info*/
	return CMD_SUCCESS;
}

//********************************************************************

int ccgi_get_br_self_info(
	int mstid,
	unsigned short ** pvid,
	unsigned int* 						num,
	int flag,
	br_self_info *test    //关于结构体 
)
{
	DBusMessage *query,*reply;
	DBusError err;
	DBusMessageIter	iter,iter_struct,iter_sub_array,iter_sub_struct;    
    
   	//char buf[10] = {0};

	char* pname = NULL;
	unsigned short revision = 0;
	unsigned char   mac[6] ={'\0','\0','\0','\0','\0','\0'};
	unsigned int      br_version;
	unsigned int		count = 0;
	unsigned short vid = 0,*tmp = NULL,*bufs=NULL,oldvid = 0;
	unsigned short  br_prio;
	unsigned short  br_maxAge;
	unsigned short  br_hTime;
	unsigned short  br_fdelay;
	unsigned char 	 br_hops;
	int i=0,j=0,ret=0;
    char temp[10];
	memset(temp,0,10);
	
	query = dbus_message_new_method_call(
									RSTP_DBUS_NAME,    \
									RSTP_DBUS_OBJPATH,    \
									RSTP_DBUS_INTERFACE,    \
									MSTP_DBUS_METHOD_SHOW_SELF_INFO);

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&mstid,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return CMD_FAILURE;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	dbus_message_iter_next(&iter);

	//DCLI_DEBUG(("dcli_common_stp:: 1053 ret %d\n",ret));
	if(DCLI_STP_OK == ret) {

		dbus_message_iter_recurse(&iter,&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&pname);

		//vty_out(vty,"Region name\t\t:  %s",pname);

		//分配空间并清空
        test->pname=(char *)malloc(35);
		memset(test->pname,0,35);
		
        strcpy(test->pname,pname);
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&revision);
		//vty_out(vty,"\nBridge revision\t\t:  %d",revision);
		
		test->revision=revision;
		
		dbus_message_iter_next(&iter_struct);

		dbus_message_iter_get_basic(&iter_struct,&count);
		test->count=count;  //计数的		
		dbus_message_iter_next(&iter_struct);

		
		//DCLI_DEBUG(("dcli_common_stp:: 1131 count %d\n",count);
		*num = count;

  
		if(0 != count) {


			tmp = (unsigned short*)malloc(sizeof(unsigned short)*count);
			if(NULL == tmp)
				return CMD_FAILURE;
			else{
				
					memset(tmp,0,sizeof(unsigned short)*count);
				}
				bufs = (unsigned short*)malloc(sizeof(unsigned short)*count);
				if(NULL == bufs){
					free(tmp);
					tmp = NULL;
					return CMD_FAILURE;
				}
				else{
					memset(bufs,0,sizeof(unsigned short)*count);
					*pvid = bufs;
				}
				
		}
		
		dbus_message_iter_recurse(&iter_struct,&iter_sub_array);
		//vty_out(vty,"\nVlan map\t\t:  ");/////////////////////////////////////?????????????????????????????????????????????
	
		
//新的代码
		if((0 == count))
		{			
				//	vty_out(vty,"0");
				memset(test->map,0,10);
		        strcpy(test->map,"0");
		}
		else 
		{

			//char oldvid[10] = {0}, vid[10] = {0};
			for(i=0; i < count; i++)
			{
			    memset(test->map,0,10);  //应该是在for循环中清空
				dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&vid);
                 bufs[i] = vid; 
				 if(0 == i)
				 {
				 	 j = 0;
                     oldvid=tmp[j]=vid;
					 if(1 == count)
					 {
                         //vty_out(vty,"%d",oldvid);
                         memset(temp,0,10);
		                 sprintf(temp,"%d",oldvid);
						 strcat(test->map,temp);
					 }
                  }  //这样是一个
		          else
				  {
				      //memset(test->map,0,10);
		              if(1 != (vid-oldvid))
					  {
		                       if(0 == j)
    						   {
    		                       //vty_out(vty,"%d,",oldvid);
    							   memset(temp,0,10);							   
    		                       sprintf(temp,"%d,",oldvid);
								   strcat(test->map,temp);

								   memset(test->t1,0,10);
								   strcat(test->t1,temp);
    		                      
    		                       oldvid=tmp[j]=vid;
    							      if( i == (count - 1))
         							   {
         		                           //vty_out(vty,"%d",oldvid);
         		                           
         		                           memset(temp,0,10);
         		                           sprintf(temp,"%d",oldvid);
         								   strcat(test->map,temp);									   
         		                           break;
         		                       }
    		                   }
		                      else
    						   {
    		                      // vty_out(vty,"%d-%d,",tmp[0],tmp[j]);
    							   memset(temp,0,10);
    		                       sprintf(temp,"%d-%d,",tmp[0],tmp[j]);
                                   strcat(test->map,temp);						   
								  
    		                       j=0;
    		                       oldvid=tmp[j]=vid;
    		                           if( i == (count - 1))
        							   {
        		                          // vty_out(vty,"%d",oldvid);
        		                           memset(temp,0,10);
        		                           sprintf(temp,"%d",oldvid);
										   strcat(test->map,temp);
        		                           break;
        		                       }
    		                   }
		                 }
		                 else
					      {
		                        j++;
		                        oldvid=tmp[j]=vid;
		                       if( i == (count - 1))
    							{
    		                        // vty_out(vty,"%d-%d",tmp[0],tmp[j]);
    								 memset(temp,0,10);
    		                         sprintf(temp,"%d-%d",tmp[0],tmp[j]);
									 strcat(test->map,temp);  
    		                    }
		                   }
		              
		          }  //这样是第二个

				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_next(&iter_sub_array);
			}
					free(tmp);
					tmp = NULL;
		}	

		dbus_message_iter_next(&iter_struct);
		//vty_out(vty,"\nBridge ID Mac Address\t:  ");
		for(i = 0; i< 5; i++)
		{
			dbus_message_iter_get_basic(&iter_struct,&mac[i]);
			//vty_out(vty,"%02x:",mac[i]);
			dbus_message_iter_next(&iter_struct);
		}

		dbus_message_iter_get_basic(&iter_struct,&mac[i]);
		//vty_out(vty,"%02x",mac[5]);
		
		dbus_message_iter_next(&iter_struct);
		if(flag == 1)
		{
			
    	   for(i=0;i<5;i++){
    	   test->mac[i]=mac[i];
    	   	}
		   test->mac[5]=mac[5];
		}
		//vty_out(vty,"\nBridge Priority\t\t:  ");
		dbus_message_iter_get_basic(&iter_struct,&br_prio);
		//vty_out(vty,"%d\n",br_prio);
		if(flag == 1)
		{
			/*fprintf(cgiOut,"<tr>"\
							   "<td id=td1>Bridge Priority\t\t:</td>"\
							   "<td id=td2>%d</td>"\
						   "</tr>",br_prio);*/
			test->br_prio=br_prio;
		}
		dbus_message_iter_next(&iter_struct);

		//vty_out(vty,"Bridge Force Version\t:  ");
		dbus_message_iter_get_basic(&iter_struct,&br_version);
		//vty_out(vty,"%d\n",br_version);
		if(flag == 1)
			/*fprintf(cgiOut,"<tr>"\
							   "<td id=td1>Bridge Force Version\t:</td>"\
							   "<td id=td2>%d</td>"\
						   "</tr>",br_version);*/
		test->br_version=br_version;
		dbus_message_iter_next(&iter_struct);

		//vty_out(vty,"Root Max Age");
		dbus_message_iter_get_basic(&iter_struct,&br_maxAge);
		//vty_out(vty,"%6d\t",br_maxAge);
		if(flag == 1)
			/*fprintf(cgiOut,"<tr>"\
							   "<td id=td1>Root Max Age:</td>"\
							   "<td id=td2>%6d</td>"\
						   "</tr>",br_maxAge);*/
        test->br_maxAge=br_maxAge;
		dbus_message_iter_next(&iter_struct);

		//vty_out(vty,"Hello Time");
		dbus_message_iter_get_basic(&iter_struct,&br_hTime);
		//vty_out(vty,"%5d\t\t",br_hTime);
		if(flag == 1)
			/*fprintf(cgiOut,"<tr>"\
							   "<td id=td1>Hello Time:</td>"\
							   "<td id=td2>%5d</td>"\
						   "</tr>",br_hTime);*/
        test->br_hTime=br_hTime;
		dbus_message_iter_next(&iter_struct);

		//vty_out(vty,"Forward Delay");
		dbus_message_iter_get_basic(&iter_struct,&br_fdelay);
		//vty_out(vty,"%5d\t",br_fdelay);
		if(flag == 1)
			/*fprintf(cgiOut,"<tr>"\
							   "<td id=td1>Forward Delay:</td>"\
							   "<td id=td2>%5d</td>"\
						   "</tr>",br_fdelay);*/
         test->br_fdelay=br_fdelay;
		dbus_message_iter_next(&iter_struct);

		//vty_out(vty,"MaxHops");
		dbus_message_iter_get_basic(&iter_struct,&br_hops);
		//vty_out(vty,"%5d\n",br_hops);
		if(flag == 1)
			/*fprintf(cgiOut,"<tr>"\
							   "<td id=td1>MaxHops:</td>"\
							   "<td id=td2>%5d</td>"\
						   "</tr>",br_hops);*/
        test->br_hops=br_hops;			
		dbus_message_iter_next(&iter_struct);

		//DCLI_DEBUG(("dcli_common1134:: END success dcli_get_br_self_info\n"));
		dbus_message_unref(reply);
		return CMD_SUCCESS;

	}
	else if(DCLI_STP_NO_SUCH_MSTID == ret)
	{
		//DCLI_DEBUG(("dcli_common879 :: END no ins dcli_get_cist_info\n"));
		dbus_message_unref(reply);
		return DCLI_STP_NO_SUCH_MSTID;
	}
	else if(STP_DISABLE == ret)
	{
		dbus_message_unref(reply);
		return ret;
	}
	else
	{
		dbus_message_unref(reply);
		return ret;
	}	
	return ret;

}


//**********************************************************************************


int ccgi_get_one_vlan_portmap
(
	unsigned short vid,
	VLAN_PORTS_BMP* ports_bmp
)
{
	DBusMessage *query, *reply;
	DBusError err;
	char*			vlanName;
	//unsigned int untagBmp = 0,tagBmp =0;
	unsigned int ret = 0;

	unsigned int product_id = 0;
	//unsigned short trunk_id = 0;
	unsigned int vlanStat = 0;
	//unsigned char trkTagMode;
	
	unsigned int promisPortBmp[2] = {0};
	PORT_MEMBER_BMP untagBmp,tagBmp;
	memset(&untagBmp,0,sizeof(PORT_MEMBER_BMP));
	memset(&tagBmp,0,sizeof(PORT_MEMBER_BMP));



	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_VLAN_OBJPATH , \
										NPD_DBUS_VLAN_INTERFACE ,\
										NPD_DBUS_VLAN_METHOD_SHOW_VLAN_PORT_MEMBERS_V1 );
	

	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT16,&vid,
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
	if(dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &ret,
					DBUS_TYPE_UINT32, &product_id,
					DBUS_TYPE_UINT32, &promisPortBmp[0],					
					DBUS_TYPE_UINT32, &promisPortBmp[1],
					DBUS_TYPE_STRING, &vlanName,
					DBUS_TYPE_UINT32, &untagBmp.portMbr[0],
					DBUS_TYPE_UINT32, &untagBmp.portMbr[1],
					DBUS_TYPE_UINT32, &tagBmp.portMbr[0],
					DBUS_TYPE_UINT32, &tagBmp.portMbr[1],
					DBUS_TYPE_UINT32, &vlanStat,
					DBUS_TYPE_INVALID)) {


		/*vty_out(vty,"vlanName %s , untagBmp %#0X , tagBmp %#0X\n",vlanName,untagBmp,tagBmp);*/
		if(0==ret|| 64 == ret) {
			ports_bmp->tagbmp.portMbr[0]= tagBmp.portMbr[0];
			ports_bmp->tagbmp.portMbr[1]= tagBmp.portMbr[1];
			ports_bmp->untagbmp.portMbr[0]= untagBmp.portMbr[0];
			ports_bmp->untagbmp.portMbr[1]= untagBmp.portMbr[1];
			return CMD_SUCCESS;
		}
		else if(NPD_DBUS_ERROR_NO_SUCH_VLAN_CCGI +1== ret) {
			//DCLI_DEBUG(("% Bad parameter: vlan id illegal.\n"));
			dbus_message_unref(reply);
			return ret;
		}
		/*
		else if(NPD_VLAN_NOTEXISTS_STP == ret) {
			//DCLI_DEBUG(( "% Bad parameter: vlan %d NOT Exists.\n",vid));
			fprintf(stderr,"3333333");
			dbus_message_unref(reply);
			return ret;
		}
		*/
		else if(0xff  == ret) {
			//DCLI_DEBUG(("% Error: Op on Hw Fail.\n"));
			fprintf(stderr,"4444444");
			dbus_message_unref(reply);
			return ret;
		}
	}
	else {
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
	}

	return CMD_SUCCESS;
}


int ccgi_stp_set_stpid_to_npd
(
	unsigned short vid,
	unsigned int mstid
)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int ret;
	
	dbus_error_init(&err);
	query = dbus_message_new_method_call(				\
								NPD_DBUS_BUSNAME,			\
								NPD_DBUS_ETHPORTS_OBJPATH,		\
								NPD_DBUS_ETHPORTS_INTERFACE,		\
								MSTP_DBUS_METHOD_CFG_VLAN_ON_MST);

	
	dbus_message_append_args(query,
									DBUS_TYPE_UINT16,&vid,							 								 		
									DBUS_TYPE_UINT32,&mstid,									
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
								DBUS_TYPE_UINT32,&ret,
								DBUS_TYPE_INVALID)) {

		if(0 != ret) {
			//vty_out(vty,"execute command error\n");
			return CMD_FAILURE;
		}
		else
		{
			return CMD_SUCCESS;
		}
		
	} else {
		//vty_out(vty,"Failed get args from rstp!.\n");	
		if (dbus_error_is_set(&err)) {
				//vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}
		
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
//**************************************************************************************************



//根桥信息

/*follow-up 08-12-31 */
int ccgi_get_cist_info_new(

 int mstid,
 cist_info *test
)
{
	DBusMessage *query,*reply;
	DBusError err;
	DBusMessageIter	iter,iter_struct;

  	//char buf[10] = {0};

	//char* pname = NULL;
	//unsigned short revision = 0;
	unsigned char   mac[6] ={'\0','\0','\0','\0','\0','\0'};
	unsigned int		path_cost;
	unsigned short root_portId;
	//unsigned short vid = 0;
	unsigned short  br_prio;
	//unsigned short  br_maxAge;
	//unsigned short  br_hTime;
	//unsigned short  br_fdelay;
	//unsigned char 	 br_hops;
	unsigned int    eth_g_index = 0;
	unsigned char   slot = 0,port = 0;
	unsigned  int   id = 0;

	int i,ret;

	query = dbus_message_new_method_call(
									RSTP_DBUS_NAME,    \
									RSTP_DBUS_OBJPATH,    \
									RSTP_DBUS_INTERFACE,    \
									MSTP_DBUS_METHOD_SHOW_CIST_INFO);
	
	dbus_error_init(&err);	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return CMD_FAILURE;
	}
	
	/*get product id*/
	ccgi_get_broad_product_id(&id);
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	dbus_message_iter_next(&iter);

	//vty_out(vty,"STP>> slot_count = %d\n",slot_count);
	if(DCLI_STP_OK == ret) {

	dbus_message_iter_recurse(&iter,&iter_struct);

	//vty_out(vty,"\nRoot Bridge\t\t:  ");
	for(i = 0; i< 5; i++)
	{
		dbus_message_iter_get_basic(&iter_struct,&mac[i]);
		//vty_out(vty,"%02x:",mac[i]);
		dbus_message_iter_next(&iter_struct);
	}

		dbus_message_iter_get_basic(&iter_struct,&mac[i]);
		//vty_out(vty,"%02x",mac[5]);
		dbus_message_iter_next(&iter_struct);
		/*fprintf(cgiOut,"<tr>"\
						   "<td id=td1>Root Bridge\t\t:</td>"\
						   "<td id=td2>%02x:%02x:%02x:%02x:%02x:%02x</td>"\
					   "</tr>",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);	*/
					   for(i=0;i<5;i++){
					   test->mac[i]=mac[i];
		}
					   test->mac[5]=mac[5];
		//vty_out(vty,"\nRoot Priority\t\t:  ");
		dbus_message_iter_get_basic(&iter_struct,&br_prio);
		//vty_out(vty,"%d\n",br_prio);
		/*fprintf(cgiOut,"<tr>"\
						   "<td id=td1>Root Priority\t\t:</td>"\
						   "<td id=td2>%d</td>"\
					   "</tr>",br_prio);	*/
		test->br_prio=br_prio;
		dbus_message_iter_next(&iter_struct);

		//vty_out(vty,"Root Path Cost\t\t:  ");
		dbus_message_iter_get_basic(&iter_struct,&path_cost);
		//vty_out(vty,"%d\n",path_cost);
		/*fprintf(cgiOut,"<tr>"\
						   "<td id=td1>Root Path Cost\t\t:</td>"\
						   "<td id=td2>%d</td>"\
					   "</tr>",path_cost);*/
        test->path_cost=path_cost;
		dbus_message_iter_next(&iter_struct);

		//vty_out(vty,"Root Port\t\t:  ");
		dbus_message_iter_get_basic(&iter_struct,&root_portId);

		//char buf[10]={0};
		if(root_portId){
			//vty_out(vty,"%d\n",root_portId);
		/*fprintf(cgiOut,"<tr>"\
						   "<td id=td1>Root Port\t\t:</td>"\
						   "<td id=td2>%d</td>"\
					   "</tr>",root_portId);*/
			eth_g_index = root_portId& 0xfff;
			//GetSlotPortFromPortIndex(id,eth_g_index,&slot,&port);  
			
			GetSlotPortFromPortIndex_new(eth_g_index,&slot,&port);  
			
			test->root_portId=root_portId;
		}
		else{
			//vty_out(vty,"%s\n","none");
			/*fprintf(cgiOut,"<tr>"\
							   "<td id=td1>Root Port\t\t:</td>"\
							   "<td id=td2>%s</td>"\
						   "</tr>","none");*/
		 //  memset(buf,0,sizeof(buf));
		 //  sprintf(buf,"%s","none");	
		 
             test->root_portId=0;
			 
		}
		dbus_message_iter_next(&iter_struct);

		//DCLI_DEBUG(("dcli_common872 :: END suc dcli_get_cist_info\n"));
		dbus_message_unref(reply);
		return CMD_SUCCESS;
	}
	else if(DCLI_STP_NO_SUCH_MSTID == ret)
	{
		//DCLI_DEBUG(("dcli_common879 :: END no ins dcli_get_cist_info\n"));
		dbus_message_unref(reply);
		return DCLI_STP_NO_SUCH_MSTID;
	}
	else if(STP_DISABLE == ret)
	{
		dbus_message_unref(reply);
		return ret;
	}
	else
	{
		dbus_message_unref(reply);
		return ret;
	}
	
}



//***************************************************************************************************

//域信息

int ccgi_get_msti_info_new(
	int mstid,
	msti_info *test 
)
{
	DBusMessage *query,*reply;
	DBusError err;
	DBusMessageIter	iter,iter_struct;//,iter_sub_array,iter_sub_struct;

	unsigned char   mac[6] ={'\0','\0','\0','\0','\0','\0'};
	unsigned int		path_cost;
	unsigned short root_portId;
	unsigned short  br_prio;
	unsigned short  br_maxAge;
	unsigned short  br_hTime;
	unsigned short  br_fdelay;
	unsigned char 	 br_hops;
	unsigned int    eth_g_index = 0;
	unsigned char   slot = 0,port = 0;
	unsigned int    id = 0;
	int i,ret;

	query = dbus_message_new_method_call(
									RSTP_DBUS_NAME,    \
									RSTP_DBUS_OBJPATH,    \
									RSTP_DBUS_INTERFACE,    \
									MSTP_DBUS_METHOD_SHOW_MSTI_INFO);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&mstid,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return CMD_FAILURE;
	}
	
    	/*get product id*/
	ccgi_get_broad_product_id(&id);
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	dbus_message_iter_next(&iter);
	//vty_out(vty,"STP>> slot_count = %d\n",slot_count);
	if(DCLI_STP_OK == ret) {

		dbus_message_iter_recurse(&iter,&iter_struct);

		//vty_out(vty,"\n Root\t\t:  ");
		for(i = 0; i< 5; i++)
		{
			dbus_message_iter_get_basic(&iter_struct,&mac[i]);
			//vty_out(vty,"%02x:",mac[i]);
			dbus_message_iter_next(&iter_struct);
		}

		dbus_message_iter_get_basic(&iter_struct,&mac[i]);
		//vty_out(vty,"%02x",mac[5]);
		test->mac[5]=mac[5];

		 for(i=0;i<5;i++){
					   test->mac[i]=mac[i];
		}
		dbus_message_iter_next(&iter_struct);

		//vty_out(vty,"\nRegion Root Priority\t:  ");
		dbus_message_iter_get_basic(&iter_struct,&br_prio);
		//vty_out(vty,"%d\n",br_prio);

		test->br_prio=br_prio;
		dbus_message_iter_next(&iter_struct);

		//vty_out(vty,"Region Root Path Cost\t:  ");
		dbus_message_iter_get_basic(&iter_struct,&path_cost);
		//vty_out(vty,"%d\n",path_cost);
		
		test->path_cost=path_cost;
		dbus_message_iter_next(&iter_struct);

		//vty_out(vty,"Region Root Port\t:  ");
		dbus_message_iter_get_basic(&iter_struct,&root_portId);
		if(root_portId){
			//vty_out(vty,"%d\n",root_portId);
		/*fprintf(cgiOut,"<tr>"\
						   "<td id=td1>Region Root Port\t:</td>"\
						   "<td id=td2>%d</td>"\
					   "</tr>",root_portId);	*/
					   
		eth_g_index = root_portId&0xfff;
		//GetSlotPortFromPortIndex(id,eth_g_index,&slot,&port);
		GetSlotPortFromPortIndex_new(eth_g_index,&slot,&port);
		test->root_portId=root_portId;
		}
		else{
			//vty_out(vty,"%s\n","none");
			/*fprintf(cgiOut,"<tr>"\
							   "<td id=td1>Region Root Port\t:</td>"\
							   "<td id=td2>%s</td>"\
						   "</tr>","none");	*/
			
			test->root_portId=0;
		}
		dbus_message_iter_next(&iter_struct);

		//vty_out(vty,"Root Max Age");
		dbus_message_iter_get_basic(&iter_struct,&br_maxAge);
		//vty_out(vty,"%6d\t",br_maxAge);
		/*fprintf(cgiOut,"<tr>"\
						   "<td id=td1>Root Max Age:</td>"\
						   "<td id=td2>%6d</td>"\
					   "</tr>",br_maxAge);*/
					   
		test->br_maxAge=br_maxAge;
		dbus_message_iter_next(&iter_struct);

		//vty_out(vty,"Hello Time");
		dbus_message_iter_get_basic(&iter_struct,&br_hTime);
		//vty_out(vty,"%5d\t\t",br_hTime);
		/*fprintf(cgiOut,"<tr>"\
						   "<td id=td1>Hello Time:</td>"\
						   "<td id=td2>%5d</td>"\
					   "</tr>",br_hTime);*/
        test->br_hTime=br_hTime;
		dbus_message_iter_next(&iter_struct);

		//vty_out(vty,"Forward Delay");
		dbus_message_iter_get_basic(&iter_struct,&br_fdelay);
		//vty_out(vty,"%5d\t",br_fdelay);
		/*fprintf(cgiOut,"<tr>"\
						   "<td id=td1>Forward Delay:</td>"\
						   "<td id=td2>%5d</td>"\
					   "</tr>",br_fdelay);*/
					   
        test->br_fdelay=br_fdelay;
		dbus_message_iter_next(&iter_struct);

		//vty_out(vty,"MaxHops");
		dbus_message_iter_get_basic(&iter_struct,&br_hops);
		//vty_out(vty,"%5d",br_hops);
		/*fprintf(cgiOut,"<tr>"\
						   "<td id=td1>MaxHops:</td>"\
						   "<td id=td2>%5d</td>"\
					   "</tr>",br_hops);*/

		test->br_hops=br_hops;
		dbus_message_iter_next(&iter_struct);
		
		//DCLI_DEBUG(("dcli_common995:: END suc dcli_get_msti_info\n"));
		dbus_message_unref(reply);
		return CMD_SUCCESS;
	}
	else if(DCLI_STP_NO_SUCH_MSTID == ret)
	{
		///DCLI_DEBUG(("dcli_common879 :: END no ins dcli_get_cist_info\n"));
		dbus_message_unref(reply);
		return DCLI_STP_NO_SUCH_MSTID;
	}
	else if(STP_DISABLE == ret)
	{
		dbus_message_unref(reply);
		return ret;
	}
	else
	{
		dbus_message_unref(reply);
		return ret;
	}
}

/*测试是否含有字母*/
int check_abc(char *ff)
{
 int pf,i,j;
 pf = strtoul(ff,0,10);//参数合法性检查，如果是字符串，就返回了零	不确切			
 char *df;
 df=(char *)malloc(sizeof(ff));
 
 sprintf(df,"%d",pf);
 
 i=strlen(ff);
 j=strlen(df);
 free(df);
 
 if(i!=j)
 	return -1;  //有错
 else
     return 0;

}

//********************************************************************************
#ifdef __cplusplus
}
#endif

