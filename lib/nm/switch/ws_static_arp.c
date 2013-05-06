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
* ws_static_arp.c
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
#ifdef __cplusplus
extern "C"
{
#endif
#include "ws_static_arp.h"
#include "ws_returncode.h"
#include "ws_dcli_interface.h"

/* for dcli_intf.c  v1.103 zhouym*/

int  ccgi_error_info_intf(int errorCode)
    {
        switch (errorCode)
        {
            case COMMON_SUCCESS :  /*0*/
                return 0;
            case ARP_RETURN_CODE_ERROR:
				return -1;
                //return "%% Arp operation Failed!\n";
            case INTERFACE_RETURN_CODE_NO_SUCH_PORT :       /*0x90019*/
            case ARP_RETURN_CODE_NO_SUCH_PORT:              /*0x2001D*/
				return WS_ERR_PORT_NUM;
                //return " %% No such port!\n";
            case ARP_RETURN_CODE_CHECK_IP_ERROR:         /*0x20014*/
				return -5;
                //return "%% Check ip address FAILED!\n";
            case ARP_RETURN_CODE_STATIC_EXIST :
				return DCLI_ARP_SNOOPING_ERR_STATIC_EXIST;
                //return "%% Static arp already exist!\n";
            case ARP_RETURN_CODE_PORT_NOTMATCH :
				return -11;
                //return "%% The static arp not for this port!\n";
            case ARP_RETURN_CODE_BADPARAM :
			case COMMON_RETURN_CODE_BADPARAM:
				return DCLI_VLAN_BADPARAM;
                //return "%% Input bad parameter!\n";
            case ARP_RETURN_CODE_STATIC_ARP_FULL:
				return -13;
                //return "%% Static arp items already FULL!\n";
            case ARP_RETURN_CODE_VLAN_NOTEXISTS :
				return DCLI_VLAN_NOTEXISTS;
                //return "%% Vlan not exists!\n";
            case INTERFACE_RETURN_CODE_SUBIF_CREATE_FAILED:
                return NPD_DBUS_ERROR;
            case ARP_RETURN_CODE_PORT_NOT_IN_VLAN:
			case INTERFACE_RETURN_CODE_PORT_NOT_IN_VLAN :
				return DCLI_DBUS_PORT_NOT_IN_VLAN;
                //return "%% The port isn't in the vlan!\n";            
			case ARP_RETURN_CODE_HAVE_THE_IP :
                return DCLI_INTF_HAVE_THE_IP;
            case ARP_RETURN_CODE_MAC_MATCHED_BASE_MAC :
				return -10;
                //return "%% Can't add static-arp with system mac address!\n";
            case ARP_RETURN_CODE_NO_HAVE_ANY_IP :
				return DCLI_INTF_NO_HAVE_ANY_IP;
               // return "%% The layer 3 interface not config ip address!\n";
            case ARP_RETURN_CODE_NOT_SAME_SUB_NET :
				return DCLI_INTF_NOT_SAME_SUB_NET;
                //return "%% The ip is not the same subnet with the layer 3 interface!\n";
            case ARP_RETURN_CODE_MAC_MATCHED_INTERFACE_MAC:
				return -12;
                //return "%% Can't add static-arp with interface mac address!\n";
            case INTERFACE_RETURN_CODE_CHECK_MAC_ERROR:
				return -3;
                //return "%% Check interface's mac address FAILED!\n";
            case INTERFACE_RETURN_CODE_VLAN_NOTEXIST:
				return DCLI_VLAN_NOTEXISTS;
                //return "%% The vlan does not exist!\n";

            default:
                return -1;
        }
    }

int config_ip_static_arp( char *portnum, char *mac, char *ipaddr, char *vlanID_str )/*返回0表示成功，返回-1表示失败，返回-2表示Bad parameter:Unknown port format，返回-3表示Bad parameter:bad slot/port number，返回-4表示Input broadcast or multicast mac，返回-5表示Bad parameter ipaddr，返回-6表示Ip address mask must be 32*/
                  /*返回-7表示Unknow vlan id format，返回-8表示Vlan out of range*/
{
        DBusMessage *query, *reply;
        DBusError err;
        unsigned int ret = 0;
        unsigned int int_slot_no = 0,int_port_no = 0;
        unsigned char slot_no = 0,port_no = 0;
        unsigned long ipno = 0;
        unsigned int ipmaskLen = 0;
        unsigned short vlanId = 0;
        ETHERADDR macAddr;
		int retu=0;

        memset(&macAddr,0,sizeof(ETHERADDR));
        ret = parse_slotno_localport_include_slot0((char *)portnum,&int_slot_no,&int_port_no);
        if (NPD_FAIL == ret)
        {
            //vty_out(vty,"%% Bad parameter:Unknown port format.\n");
            return -2;
        }
        else if (1 == ret)
        {
            //vty_out(vty,"%% Bad parameter:bad slot/port number.\n");
            return -3;
        }
        slot_no = (unsigned char)int_slot_no;
        port_no = (unsigned char)int_port_no;

        ret = parse_mac_addr((char *)mac,&macAddr);
        if (NPD_FAIL == ret)
        {
            //vty_out(vty,"%% Unknow mac addr format!\n");
            return -9;

        }

        ret = is_muti_brc_mac(&macAddr);
        if (ret==1)
        {
            //vty_out(vty,"%% Input broadcast or multicast mac!\n");
            return -4;
        }
        ret=ip_address_format2ulong((char **)&ipaddr,&ipno,&ipmaskLen);
        if (1 == ret)
        {
            //vty_out(vty, "%% Bad parameter ipaddr%s\n", argv[2]);
            return -5;
        }
        IP_MASK_CHECK(ipmaskLen);
        if (32 != ipmaskLen)
        {
            //vty_out(vty,"%% Ip address mask must be 32!\n");
            return -6;
        }
        ret = parse_short_parse((char*)vlanID_str, &vlanId);
        if (NPD_FAIL == ret)
        {
            //vty_out(vty,"%% Unknow vlan id format!\n");
            return -7;
        }
        if (vlanId <MIN_VLANID||vlanId>MAX_L3INTF_VLANID)
        {
           // vty_out(vty,"%% Vlan out of range!\n");
            return -8;
        }

        /*appends and gets*/
        query = dbus_message_new_method_call(\
                                             NPD_DBUS_BUSNAME,                   \
                                             NPD_DBUS_INTF_OBJPATH,      \
                                             NPD_DBUS_INTF_INTERFACE,        \
                                             NPD_DBUS_INTERFACE_IP_STATIC_ARP);

        dbus_error_init(&err);

        dbus_message_append_args(query,
                                 DBUS_TYPE_BYTE,&slot_no,
                                 DBUS_TYPE_BYTE,&port_no,
                                 DBUS_TYPE_BYTE,&macAddr.arEther[0],
                                 DBUS_TYPE_BYTE,&macAddr.arEther[1],
                                 DBUS_TYPE_BYTE,&macAddr.arEther[2],
                                 DBUS_TYPE_BYTE,&macAddr.arEther[3],
                                 DBUS_TYPE_BYTE,&macAddr.arEther[4],
                                 DBUS_TYPE_BYTE,&macAddr.arEther[5],
                                 DBUS_TYPE_UINT32,&ipno,
                                 DBUS_TYPE_UINT32,&ipmaskLen,
                                 DBUS_TYPE_UINT32,&vlanId,
                                 DBUS_TYPE_INVALID);

        reply = dbus_connection_send_with_reply_and_block(ccgi_dbus_connection,query,-1, &err);

        dbus_message_unref(query);

        if (NULL == reply)
        {
            if (dbus_error_is_set(&err))
            {
                dbus_error_free(&err);
            }
            return -1;
        }

        if (dbus_message_get_args(reply, &err,
                                  DBUS_TYPE_UINT32,&ret,
                                  DBUS_TYPE_INVALID))
        {
            if ((INTERFACE_RETURN_CODE_SUCCESS != ret)&&\
                (ARP_RETURN_CODE_SUCCESS != ret))
            {
                //vty_out(vty,dcli_error_info_intf(ret));
                if(ret==0)
					retu=0;
				else
					retu=ccgi_error_info_intf(ret);
            }

        }
        else
        {
            if (dbus_error_is_set(&err))
            {
                dbus_error_free(&err);
            }
        }
        dbus_message_unref(reply);

        return retu;
    }

int delete_ip_static_arp( char *portnum, char *mac, char *ipaddr, char *vlanID_str )/*返回0表示成功，返回-1表示失败，返回-2表示Bad parameter:Unknown port format，返回-3表示Bad parameter:bad slot/port number，返回-4表示Input broadcast or multicast mac，返回-5表示Bad parameter ipaddr，返回-6表示Ip address mask must be 32*/
                  /*返回-7表示Unknow vlan id format，返回-8表示Vlan out of range*/	
{
        DBusMessage *query, *reply;
        DBusError err;
        unsigned int ret = 0;
        unsigned int int_slot_no = 0,int_port_no = 0;
        unsigned char slot_no = 0,port_no = 0;
        unsigned long ipno = 0;
        unsigned int ipmaskLen = 0;
        unsigned short vlanId = 0;
        ETHERADDR macAddr;
		int retu=0;

        memset(&macAddr,0,sizeof(ETHERADDR));
        ret = parse_slotno_localport_include_slot0((char *)portnum,&int_slot_no,&int_port_no);
        if (NPD_FAIL == ret)
        {
            //vty_out(vty,"%% Bad parameter:unknow portno format!\n");
            return -2;
        }
        else if (1 == ret)
        {
            //vty_out(vty,"%% Bad parameter: bad slot/port %s number!\n",argv[0]);
            return -3;
        }
        slot_no = (unsigned char)int_slot_no;
        port_no = (unsigned char)int_port_no;

        ret = parse_mac_addr((char *)mac,&macAddr);
        if (NPD_FAIL == ret)
        {
            //vty_out(vty,"%% Unknown mac addr format!\n");
			return -4;

        }
        ret = is_muti_brc_mac(&macAddr);
        if (ret==1)
        {
            //vty_out(vty,"%% Input broadcast or multicast mac!\n");
            return -5;
        }

        ret=ip_address_format2ulong((char**)&ipaddr,&ipno,&ipmaskLen);
        if (1 == ret)
        {
           // vty_out(vty, "%% Bad parameter %s\n", argv[2]);
            return -6;
        }
        IP_MASK_CHECK(ipmaskLen);
        if (32 != ipmaskLen)
        {
            //vty_out(vty,"%% Ip address mask must be 32!\n");
            return -7;
        }

        ret = parse_short_parse((char*)vlanID_str, &vlanId);
        if (NPD_FAIL == ret)
        {
            //vty_out(vty,"%% Unknown vlan id format!\n");
            return -8;
        }
        if (vlanId <MIN_VLANID||vlanId>MAX_L3INTF_VLANID)
        {
           // vty_out(vty,"%% Vlan out of range!\n");
            return -9;
        }
        /*appends and gets*/
        query = dbus_message_new_method_call(\
                                             NPD_DBUS_BUSNAME,                   \
                                             NPD_DBUS_INTF_OBJPATH,      \
                                             NPD_DBUS_INTF_INTERFACE,        \
                                             NPD_DBUS_INTERFACE_NO_IP_STATIC_ARP);

        dbus_error_init(&err);

        dbus_message_append_args(query,
                                 DBUS_TYPE_BYTE,&slot_no,
                                 DBUS_TYPE_BYTE,&port_no,
                                 DBUS_TYPE_BYTE,&macAddr.arEther[0],
                                 DBUS_TYPE_BYTE,&macAddr.arEther[1],
                                 DBUS_TYPE_BYTE,&macAddr.arEther[2],
                                 DBUS_TYPE_BYTE,&macAddr.arEther[3],
                                 DBUS_TYPE_BYTE,&macAddr.arEther[4],
                                 DBUS_TYPE_BYTE,&macAddr.arEther[5],
                                 DBUS_TYPE_UINT32,&ipno,
                                 DBUS_TYPE_UINT32,&ipmaskLen,
                                 DBUS_TYPE_UINT32,&vlanId,
                                 DBUS_TYPE_INVALID);

        reply = dbus_connection_send_with_reply_and_block(ccgi_dbus_connection,query,-1, &err);

        dbus_message_unref(query);

        if (NULL == reply)
        {
            if (dbus_error_is_set(&err))
            {
                dbus_error_free(&err);
            }
            return -1;
        }

        if (dbus_message_get_args(reply, &err,
                                  DBUS_TYPE_UINT32,&ret,
                                  DBUS_TYPE_INVALID))
        {
            if ((INTERFACE_RETURN_CODE_SUCCESS != ret)&&\
                (ARP_RETURN_CODE_SUCCESS != ret))
            {
                //vty_out(vty,dcli_error_info_intf(ret));
                if(ret==0)
					retu = 0;
				else
					retu = -1;
            }
        }
        else
        {
            if (dbus_error_is_set(&err))
            {
                dbus_error_free(&err);
            }
        }
        dbus_message_unref(reply);

        return retu;
    }

int config_ip_static_arp_for_slot0(char *portnum, char *mac, char *ipaddr)
{
    DBusMessage *query, *reply;
	DBusError err;
    /*variables*/
    unsigned int ret = 0;
	unsigned int int_slot_no = 0,int_port_no = 0;
	unsigned char slot_no = 0,port_no = 0;
	unsigned long ipno = 0;
    unsigned int ipmaskLen = 0;
	unsigned short vlanId = 0;
	ETHERADDR macAddr;

    memset(&macAddr,0,sizeof(ETHERADDR));
	/*parse argcs*/
	ret = parse_slotno_localport_include_slot0((char *)portnum,&int_slot_no,&int_port_no);
	if (NPD_FAIL == ret) {
    	//vty_out(vty,"%% Bad parameter:Unknown port format.\n");
		return COMMON_ERROR;
	}
	else if (1 == ret){
		//vty_out(vty,"%% Bad parameter:bad slot/port number.\n");
		return COMMON_ERROR;
	}
	slot_no = (unsigned char)int_slot_no;
	port_no = (unsigned char)int_port_no;

	ret = parse_mac_addr((char *)mac,&macAddr);	
	if (NPD_FAIL == ret) {
    	//vty_out(vty,"%% Unknow mac addr format!\n");
		return COMMON_ERROR;

	}
	 
	ret = is_muti_brc_mac(&macAddr);
	if(ret==1){
		//vty_out(vty,"%% Input broadcast or multicast mac!\n");
		return COMMON_ERROR;
	}
	ret=ip_address_format2ulong((char **)&ipaddr,&ipno,&ipmaskLen);		
	if(COMMON_ERROR == ret) {
		// vty_out(vty, "%% Bad parameter %s\n", argv[2]);
		 return COMMON_ERROR;
	}
	IP_MASK_CHECK(ipmaskLen);
	if(32 != ipmaskLen) { 
		//vty_out(vty,"%% Ip address mask must be 32!\n");
		return COMMON_ERROR;
	}

	/*appends and gets*/
	query = dbus_message_new_method_call(			\
							NPD_DBUS_BUSNAME,					\
							NPD_DBUS_INTF_OBJPATH,		\
							NPD_DBUS_INTF_INTERFACE,		\
							NPD_DBUS_INTERFACE_IP_STATIC_ARP);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&slot_no,
							 DBUS_TYPE_BYTE,&port_no,
							 DBUS_TYPE_BYTE,&macAddr.arEther[0],
							 DBUS_TYPE_BYTE,&macAddr.arEther[1],
							 DBUS_TYPE_BYTE,&macAddr.arEther[2],
							 DBUS_TYPE_BYTE,&macAddr.arEther[3],
							 DBUS_TYPE_BYTE,&macAddr.arEther[4],
							 DBUS_TYPE_BYTE,&macAddr.arEther[5],
							 DBUS_TYPE_UINT32,&ipno,
							 DBUS_TYPE_UINT32,&ipmaskLen,
							 DBUS_TYPE_UINT32,&vlanId,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return COMMON_SUCCESS;
	}
	
	//DCLI_DEBUG(("query reply not null\n"));
	if (dbus_message_get_args ( reply, &err,
	    DBUS_TYPE_UINT32,&ret,
		DBUS_TYPE_INVALID)) {
		if(NPD_DBUS_SUCCESS != ret){
             //vty_out(vty,dcli_error_info_intf(ret));
             return ret;
		}

	} 
	else {
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);

	return COMMON_SUCCESS;
}

int config_noip_static_arp_for_slot0(char *portnum, char *mac, char *ipaddr)
{
    DBusMessage *query, *reply;
	DBusError err;
    /*variables*/
    unsigned int ret = 0;
	unsigned int int_slot_no = 0,int_port_no = 0;
	unsigned char slot_no = 0,port_no = 0;
	unsigned long ipno = 0;
	unsigned int ipmaskLen = 0;
	unsigned short vlanId = 0;
	ETHERADDR macAddr;

    memset(&macAddr,0,sizeof(ETHERADDR));
	/*parse argcs*/
	ret = parse_slotno_localport_include_slot0((char *)portnum,&int_slot_no,&int_port_no);
	if (NPD_FAIL == ret) {
    	//vty_out(vty,"%% Bad parameter:unknow portno format!\n");
		return COMMON_ERROR;
	}
	else if (1 == ret){
		//vty_out(vty,"%% Bad parameter: bad slot/port %s number or bad vlan id!\n",argv[0]);
		return COMMON_ERROR;
	}
	slot_no = (unsigned char)int_slot_no;
	port_no = (unsigned char)int_port_no;

	ret = parse_mac_addr((char *)mac,&macAddr);
	if (NPD_FAIL == ret) {
    	//vty_out(vty,"%% Unknown mac addr format!\n");
		return COMMON_SUCCESS;

	}
	ret = is_muti_brc_mac(&macAddr);
	if(ret==1){
		//vty_out(vty,"%% Input broadcast or multicast mac!\n");
		return COMMON_ERROR;
	}

	ret=ip_address_format2ulong((char**)&ipaddr,&ipno,&ipmaskLen);			
	if(COMMON_ERROR == ret) {
		 //vty_out(vty, "%% Bad parameter %s\n", argv[2]);
		 return COMMON_ERROR;
	}
	IP_MASK_CHECK(ipmaskLen);
	if(32 != ipmaskLen) { 
		//vty_out(vty,"%% Ip address mask must be 32!\n");
		return COMMON_ERROR;
	}


	/*appends and gets*/
	query = dbus_message_new_method_call(			\
							NPD_DBUS_BUSNAME,					\
							NPD_DBUS_INTF_OBJPATH,		\
							NPD_DBUS_INTF_INTERFACE,		\
							NPD_DBUS_INTERFACE_NO_IP_STATIC_ARP);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&slot_no,
							 DBUS_TYPE_BYTE,&port_no,
							 DBUS_TYPE_BYTE,&macAddr.arEther[0],
							 DBUS_TYPE_BYTE,&macAddr.arEther[1],
							 DBUS_TYPE_BYTE,&macAddr.arEther[2],
							 DBUS_TYPE_BYTE,&macAddr.arEther[3],
							 DBUS_TYPE_BYTE,&macAddr.arEther[4],
							 DBUS_TYPE_BYTE,&macAddr.arEther[5],
							 DBUS_TYPE_UINT32,&ipno,
							 DBUS_TYPE_UINT32,&ipmaskLen,
							 DBUS_TYPE_UINT32,&vlanId,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return COMMON_SUCCESS;
	}
	
	//DCLI_DEBUG(("query reply not null\n"));
	if (dbus_message_get_args ( reply, &err,
	    DBUS_TYPE_UINT32,&ret,
		DBUS_TYPE_INVALID)) {
			if (NPD_DBUS_SUCCESS != ret){
               // vty_out(vty,dcli_error_info_intf(ret));
			}
	} 
	else {
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);

	return COMMON_SUCCESS;
}
int config_ip_static_arp_trunk(char *trunkidz,char *macz,char *ipaddrz,char *vidz)/*返回0表示成功，返回-1表示失败，返回-2表示Unknow mac addr format，返回-3表示Input broadcast or multicast mac，返回-4表示Bad parameter ipaddr，返回-5表示Ip address mask must be 32*/
	                        /*返回-6表示Unknow vlan id format!，返回-7表示Vlan out of rang*/
{
        DBusMessage *query, *reply;
        DBusError err;
        unsigned int ret = 0;
        unsigned int trunkId = 0;
        unsigned long ipno = 0;
        unsigned int ipmaskLen = 0;
        unsigned short vlanId = 0;
        ETHERADDR macAddr;
		int retu=0;

        memset(&macAddr,0,sizeof(ETHERADDR));
        trunkId = strtoul(trunkidz,NULL,NULL);

        ret = parse_mac_addr((char *)macz,&macAddr);
        if (NPD_FAIL == ret)
        {
            //vty_out(vty,"%% Unknow mac addr format!\n");
            return -2;

        }

        ret = is_muti_brc_mac(&macAddr);
        if (ret == 1)
        {
            //vty_out(vty,"%% Input broadcast or multicast mac!\n");
            return -3;
        }
        ret = ip_address_format2ulong((char **)&ipaddrz,&ipno,&ipmaskLen);
        if (1 == ret)
        {
            //vty_out(vty, "%% Bad parameter ipaddr%s\n", argv[2]);
            return -4;
        }
        IP_MASK_CHECK(ipmaskLen);
        if (32 != ipmaskLen)
        {
            //vty_out(vty,"%% Ip address mask must be 32!\n");
            return -5;
        }
        ret = parse_short_parse((char*)vidz, &vlanId);
        if (NPD_FAIL == ret)
        {
            //vty_out(vty,"%% Unknow vlan id format!\n");
            return -6;
        }
        if (vlanId <MIN_VLANID||vlanId>MAX_L3INTF_VLANID)
        {
            //vty_out(vty,"%% Vlan out of rang!\n");
            return -7;
        }

        /*appends and gets*/
        query = dbus_message_new_method_call(\
                                             NPD_DBUS_BUSNAME,                   \
                                             NPD_DBUS_INTF_OBJPATH,      \
                                             NPD_DBUS_INTF_INTERFACE,        \
                                             NPD_DBUS_INTERFACE_IP_STATIC_ARP_FOR_TRUNK);

        dbus_error_init(&err);

        dbus_message_append_args(query,
                                 DBUS_TYPE_UINT32,&trunkId,
                                 DBUS_TYPE_BYTE,&macAddr.arEther[0],
                                 DBUS_TYPE_BYTE,&macAddr.arEther[1],
                                 DBUS_TYPE_BYTE,&macAddr.arEther[2],
                                 DBUS_TYPE_BYTE,&macAddr.arEther[3],
                                 DBUS_TYPE_BYTE,&macAddr.arEther[4],
                                 DBUS_TYPE_BYTE,&macAddr.arEther[5],
                                 DBUS_TYPE_UINT32,&ipno,
                                 DBUS_TYPE_UINT32,&ipmaskLen,
                                 DBUS_TYPE_UINT32,&vlanId,
                                 DBUS_TYPE_INVALID);

        reply = dbus_connection_send_with_reply_and_block(ccgi_dbus_connection,query,-1, &err);

        dbus_message_unref(query);

        if (NULL == reply)
        {
            if (dbus_error_is_set(&err))
            {
                dbus_error_free(&err);
            }
            return -1;
        }

        if (dbus_message_get_args(reply, &err,
                                  DBUS_TYPE_UINT32,&ret,
                                  DBUS_TYPE_INVALID))
        {
            if ((INTERFACE_RETURN_CODE_SUCCESS != ret)&&\
                (ARP_RETURN_CODE_SUCCESS != ret))
            {
                //vty_out(vty,dcli_error_info_intf(ret));
                if(ret==0)
					retu=0;
				else 
					retu=-1;
            }

        }
        else
        {
            if (dbus_error_is_set(&err))
            {
                dbus_error_free(&err);
            }
        }
        dbus_message_unref(reply);

        return retu;
    }
int config_noip_static_arp_trunk(char *trunkidz,char *macz,char *ipaddrz,char *vidz)/*返回0表示成功，返回-1表示失败，返回-2表示Unknow mac addr format，返回-3表示Input broadcast or multicast mac，返回-4表示Bad parameter ipaddr，返回-5表示Ip address mask must be 32*/
	                        /*返回-6表示Unknow vlan id format!，返回-7表示Vlan out of rang*/
{
        DBusMessage *query, *reply;
        DBusError err;
        /*variables*/
        unsigned int ret = 0;
        unsigned int trunkId = 0;
        unsigned long ipno = 0;
        unsigned int ipmaskLen = 0;
        unsigned short vlanId = 0;
        ETHERADDR macAddr;
		int retu;

        memset(&macAddr,0,sizeof(ETHERADDR));
        trunkId = strtoul(trunkidz,NULL,NULL);
        if ((trunkId > 127)||(trunkId < 1))
        {
            //vty_out("%% Bad parameter : trunk id %d \n",trunkId);
            return -2;
        }
        ret = parse_mac_addr((char *)macz,&macAddr);
        if (NPD_FAIL == ret)
        {
            //vty_out(vty,"%% Unknow mac addr format!\n");
            return -3;

        }

        ret = is_muti_brc_mac(&macAddr);
        if (ret == 1)
        {
            //vty_out(vty,"%% Input broadcast or multicast mac!\n");
            return -4;
        }
        ret = ip_address_format2ulong((char **)&ipaddrz,&ipno,&ipmaskLen);
        if (1 == ret)
        {
            //vty_out(vty, "%% Bad parameter %s\n", argv[2]);
            return -5;
        }
        IP_MASK_CHECK(ipmaskLen);
        if (32 != ipmaskLen)
        {
           // vty_out(vty,"%% Ip address mask must be 32!\n");
            return -6;
        }
        ret = parse_short_parse((char*)vidz, &vlanId);
        if (NPD_FAIL == ret)
        {
            //vty_out(vty,"%% Unknow vlan id format!\n");
            return -7;
        }
        if (vlanId <MIN_VLANID||vlanId>MAX_L3INTF_VLANID)
        {
            //vty_out(vty,"%% Vlan out of rang!\n");
            return -8;
        }

        /*appends and gets*/
        query = dbus_message_new_method_call(\
                                             NPD_DBUS_BUSNAME,                   \
                                             NPD_DBUS_INTF_OBJPATH,      \
                                             NPD_DBUS_INTF_INTERFACE,        \
                                             NPD_DBUS_INTERFACE_NO_IP_STATIC_ARP_FOR_TRUNK);

        dbus_error_init(&err);

        dbus_message_append_args(query,
                                 DBUS_TYPE_UINT32,&trunkId,
                                 DBUS_TYPE_BYTE,&macAddr.arEther[0],
                                 DBUS_TYPE_BYTE,&macAddr.arEther[1],
                                 DBUS_TYPE_BYTE,&macAddr.arEther[2],
                                 DBUS_TYPE_BYTE,&macAddr.arEther[3],
                                 DBUS_TYPE_BYTE,&macAddr.arEther[4],
                                 DBUS_TYPE_BYTE,&macAddr.arEther[5],
                                 DBUS_TYPE_UINT32,&ipno,
                                 DBUS_TYPE_UINT32,&ipmaskLen,
                                 DBUS_TYPE_UINT32,&vlanId,
                                 DBUS_TYPE_INVALID);

        reply = dbus_connection_send_with_reply_and_block(ccgi_dbus_connection,query,-1, &err);

        dbus_message_unref(query);

        if (NULL == reply)
        {
            if (dbus_error_is_set(&err))
            {
                dbus_error_free(&err);
            }
            return -1;
        }

        if (dbus_message_get_args(reply, &err,
                                  DBUS_TYPE_UINT32,&ret,
                                  DBUS_TYPE_INVALID))
        {
            if ((INTERFACE_RETURN_CODE_SUCCESS != ret)&&\
                (ARP_RETURN_CODE_SUCCESS != ret))
            {
                //vty_out(vty,dcli_error_info_intf(ret));
                if(ret==0)
					retu=0;
				else
					retu=-1;
            }

        }
        else
        {
            if (dbus_error_is_set(&err))
            {
                dbus_error_free(&err);
            }
        }
        dbus_message_unref(reply);

        return retu;
    }

#ifdef __cplusplus
}
#endif

