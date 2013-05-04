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
* dcli_common_mld_snp.c
*
*
* CREATOR:
* 		yangxs@autelan.com
*
* DESCRIPTION:
* 		dcli mld common file to handle some orders .
*
* DATE:
*		4/20/2010
*
* FILE REVISION NUMBER:
*  		$Revision: 1.1 $
*
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

/*include header files begin */
/*kernel part */
#include <stdio.h>
#include <string.h>
/*user part */
#include <zebra.h>
#include <dbus/dbus.h>
#include <sysdef/npd_sysdef.h>
#include <dbus/npd/npd_dbus_def.h>
#include "vty.h"
#include "dcli_vlan.h"
#include "command.h"
#include "dcli_common_igmp_snp.h"
#include "dcli_igmp_snp.h"
#include "sysdef/returncode.h"
#include "dcli_mld_snp.h"


/*include header files begin */

/*MACRO definition begin */
/*MACRO definition end */

/*local variables definition begin */
extern DBusConnection *dcli_dbus_connection;


void dcli_show_mld_snp_mcgroup_list
(
	struct vty		*vty,
	unsigned short vlanId
)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	unsigned int 	group_Cont = 0,groupIp=0;
	unsigned short  vidx= 0;
	unsigned int	i,j,product_id = 0;
	unsigned int 	ret = 0;
	unsigned char ip0=0,ip1=0,ip2=0,ip3=0;
    PORT_MEMBER_BMP mbrBmp;
	unsigned short groupIpv6[SIZE_OF_IPV6_ADDR];

	memset(&mbrBmp,0,sizeof(PORT_MEMBER_BMP));
	query = dbus_message_new_method_call(\
						NPD_DBUS_BUSNAME,	\
						NPD_DBUS_VLAN_OBJPATH ,	\
						NPD_DBUS_VLAN_INTERFACE ,	\
						NPD_DBUS_VLAN_METHOD_SHOW_VLAN_MCGROUP_LIST_PORT_MEMBERS_V1 );

	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT16,&vlanId,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_igmp,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return;
	}

	dbus_message_iter_init(reply,&iter);

	dbus_message_iter_get_basic(&iter,&ret);
	if(IGMPSNP_RETURN_CODE_OK == ret){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&group_Cont);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&product_id);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
		vty_out(vty,"================================================================================\n");
		vty_out(vty,"%-7s  %-4s  %-39s  %-40s\n","VLAN ID","VIDX","GROUP_IP","PORT MEMBER LIST");
		vty_out(vty,"=======  ====  =======================================  ========================\n");

		for (j = 0; j < group_Cont; j++) {
			DBusMessageIter iter_struct;
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&vidx);
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&groupIp);
			dbus_message_iter_next(&iter_struct);

			for(i=0; i < SIZE_OF_IPV6_ADDR; i++){
				dbus_message_iter_get_basic(&iter_struct,&(groupIpv6[i]));
				dbus_message_iter_next(&iter_struct);
			}			
			
			dbus_message_iter_get_basic(&iter_struct,&(mbrBmp.portMbr[0]));
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(mbrBmp.portMbr[1]));
			dbus_message_iter_next(&iter_array);

			if(0 == groupIp){
				vty_out(vty,"%-7d  ",vlanId);
				vty_out(vty,"%-4d  ",vidx);
				vty_out(vty,"%-4x:%-4x:%-4x:%-4x:%-4x:%-4x:%-4x:%-4x  ",\
						groupIpv6[0],groupIpv6[1],groupIpv6[2],groupIpv6[3],groupIpv6[4],groupIpv6[5],groupIpv6[6],groupIpv6[7]);
				show_group_member_slot_port(vty,product_id,mbrBmp);
				vty_out(vty,"\n");
			}
		}
		vty_out(vty,"================================================================================\n");
	}
	else if(IGMPSNP_RETURN_CODE_GROUP_NOTEXIST == ret) {
		vty_out(vty,"%% Error:layer 2 multicast entry not exist!\n");
	}
	else if (IGMPSNP_RETURN_CODE_ERROR_HW == ret) {
		vty_out(vty,"%% Read layer 2 multicast entry from hardware error!\n");				
	}
	else if(IGMPSNP_RETURN_CODE_NOT_ENABLE_GBL == ret){
		vty_out(vty,"%% Error:mld snooping not enabled global!\n");
	}
	else if(IGMPSNP_RETURN_CODE_NOTENABLE_VLAN == ret){
		vty_out(vty,"%% Error:vlan %d not enabled mld snooping!\n",vlanId);
	}
	else if(IGMPSNP_RETURN_CODE_MC_VLAN_NOT_EXIST == ret){
		vty_out(vty,"%% Error:layer 2 multicast vlan %d not exist!\n",vlanId);
	}
	else {
		vty_out(vty,"%% Error %d\n",ret);
	}
		
	dbus_message_unref(reply);
	return;

}
/*************************************************************************** 
 * function : Show all layer 2 multicast group's count in the vlan
 *     name : dcli_show_mld_snp_mcgroup_count
 *     input :
 *			struct vty       *vty
 *			unsigned short vlanId
 *   output :
 *			null
 *  re_value: void
 ***************************************************************************/
void dcli_show_mld_snp_mcgroup_count
(
	struct vty		*vty,
	unsigned short vlanId
)
{
	DBusMessage     *query, *reply;
	DBusError        err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	unsigned int 	 group_Cont = 0, groupIp = 0;
	unsigned short   vidx = 0;
	unsigned int	 i,j,product_id = 0;
	PORT_MEMBER_BMP  mbrBmp;
	unsigned int 	 ret = 0;
	unsigned char    ip0 = 0, ip1 = 0, ip2 = 0, ip3 = 0;
	unsigned short groupIpv6[SIZE_OF_IPV6_ADDR];

    memset(&mbrBmp,0,sizeof(PORT_MEMBER_BMP));
	query = dbus_message_new_method_call(\
						NPD_DBUS_BUSNAME,	\
						NPD_DBUS_VLAN_OBJPATH ,	\
						NPD_DBUS_VLAN_INTERFACE ,	\
						NPD_DBUS_VLAN_METHOD_SHOW_VLAN_MCGROUP_LIST_PORT_MEMBERS_V1 );

	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT16, &vlanId,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_igmp, query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return;
	}

	dbus_message_iter_init(reply, &iter);

	dbus_message_iter_get_basic(&iter, &ret);
	if(IGMPSNP_RETURN_CODE_OK == ret){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter, &group_Cont);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter, &product_id);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter, &iter_array);

		vty_out(vty,"====================\n");
		vty_out(vty,"%-7s  %-11s\n", "VLAN ID", "GROUP_COUNT");
		vty_out(vty,"=======  ===========\n");
		vty_out(vty,"%-7d  ", vlanId);
		vty_out(vty,"%-4d  ", group_Cont);
		vty_out(vty,"\n====================\n");

		/* this "for" loop isn't use in fact, but existing for the dbus function  */
		for (j = 0; j < group_Cont; j++) {
			DBusMessageIter iter_struct;
			dbus_message_iter_recurse(&iter_array, &iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct, &vidx);
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct, &groupIp);
			dbus_message_iter_next(&iter_struct);

			for(i=0; i<SIZE_OF_IPV6_ADDR; i++){
				dbus_message_iter_get_basic(&iter_struct,&(groupIpv6[i]));
				dbus_message_iter_next(&iter_struct);
			}			
			
			dbus_message_iter_get_basic(&iter_struct, &(mbrBmp.portMbr[0]));
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct, &(mbrBmp.portMbr[1]));
			dbus_message_iter_next(&iter_array);			
		}
	}
	else if(IGMPSNP_RETURN_CODE_GROUP_NOTEXIST == ret) {
		vty_out(vty,"%% Error:layer 2 multicast group not exist!\n");
	}
	else if (IGMPSNP_RETURN_CODE_ERROR_HW == ret) {
		vty_out(vty,"%% Read layer 2 multicast entry from hardware error!\n");				
	}
	else if(IGMPSNP_RETURN_CODE_NOT_ENABLE_GBL == ret){
		vty_out(vty,"%% Error:mld snooping not enabled global!\n");
	}
	else if(IGMPSNP_RETURN_CODE_NOTENABLE_VLAN == ret){
		vty_out(vty,"%% Error:vlan %d not enabled mld snooping!\n", vlanId);
	}
	else if(IGMPSNP_RETURN_CODE_MC_VLAN_NOT_EXIST == ret){
		vty_out(vty,"%% Error:layer 2 multicast vlan %d not exist!\n",vlanId);
	}
	else {
		vty_out(vty,"%% Error %x\n", ret);
	}
		
	dbus_message_unref(reply);
	return;
}


#ifdef __cplusplus
}
#endif

