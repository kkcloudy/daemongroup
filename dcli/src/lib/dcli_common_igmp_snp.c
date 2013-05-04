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
* dcli_common_igmp_snp.c
*
*
* CREATOR:
* 		wujh@autelan.com
*
* DESCRIPTION:
* 		dcli igmp common file to handle some orders .
*
* DATE:
*		6/19/2008
*
* FILE REVISION NUMBER:
*  		$Revision: 1.36 $
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
/* add for distributed igmp */
#include "dcli_main.h"



/*include header files begin */

/*MACRO definition begin */
/*MACRO definition end */

/*local variables definition begin */
extern DBusConnection *dcli_dbus_connection;
extern DBusConnection *dcli_dbus_connection_igmp;

/* add for distributed igmp */
extern int is_distributed;


/**************************************
*show_group_member_slot_port
*Params:
*		vty - vty terminal
*		product_id - product id definition as enum product_id_e
*		portmbrBmp - member bitmap
*		
*
*Usage: show group membership within
*		slot&port number.
**************************************/
int show_group_member_slot_port
(
	struct vty *vty,
	unsigned int product_id,
	PORT_MEMBER_BMP mbrBmp
)
{
	unsigned int i,count = 0;
	unsigned int slot = 0,port = 0;
    unsigned int tmpVal[2];
	
	memset(tmpVal,0,sizeof(tmpVal));
	if(is_distributed == DISTRIBUTED_SYSTEM)
	{
		slot = igmp_dist_slot;

		
    	for (i=0;i<64;i++)
		{
#if 0			
			port = i+1;
#endif			
    		tmpVal[i/32] = (1<<(i%32));
    		if((mbrBmp.portMbr[i/32]) & tmpVal[i/32]) 
			{				
    			if(count && (0 == count % 4)) 
				{
					vty_out(vty,"\n%-32s"," ");

    			}
    			vty_out(vty,"%s%d/%d",(count%4) ? ",":"",slot,i);
    			count++;
    		}
    	}	
		return 0;
	}
	else
	{
		for (i=0;i<64;i++){
			if((PRODUCT_ID_AX7K_I == product_id) ||
				(PRODUCT_ID_AX7K == product_id)) {
				slot = i/8 + 1;
				port = i%8;
			}
			else if((PRODUCT_ID_AX5K == product_id) ||
					(PRODUCT_ID_AX5K_I == product_id) ||
					(PRODUCT_ID_AU4K == product_id) ||
					(PRODUCT_ID_AU3K == product_id) ||
					(PRODUCT_ID_AU3K_BCM == product_id)||
					(PRODUCT_ID_AU3K_BCAT == product_id) || 
					(PRODUCT_ID_AU2K_TCAT == product_id)) {
				slot = 1;
				port = i;
			}

			tmpVal[i/32] = (1<<(i%32));
			if((mbrBmp.portMbr[i/32]) & tmpVal[i/32]) {				
				if(count && (0 == count % 4)) {
					vty_out(vty,"\n%-32s"," ");
				}
				if(PRODUCT_ID_AX7K_I == product_id) {
					vty_out(vty,"%scscd%d",(count%4) ? ",":"",port-1);
				}
				else {
					vty_out(vty,"%s%d/%d",(count%4) ? ",":"",slot,port);
				}
				count++;
			}
			memset(tmpVal,0,sizeof(tmpVal));
		}
		return 0;
	}
}

/*****************************************
 *
 *
 *
 *
 ****************************************/
unsigned int ltochararry
(
	unsigned int Num,
	unsigned char *c0,
	unsigned char *c1,
	unsigned char *c2,
	unsigned char *c3
)
{
	unsigned char cstr0 = 0,cstr1= 0,cstr2 = 0,cstr3 = 0;

/*	cstr0= ((unsigned char *)&Num)[0]; 
	cstr1= ((unsigned char *)&Num)[1]; 
	cstr2= ((unsigned char *)&Num)[2]; 
	cstr3= ((unsigned char *)&Num)[3]; 
*/
/*change for xcat*/
	cstr0= (unsigned char)(Num>>24 & 0xff);
	cstr1= (unsigned char)(Num>>16 & 0xff);
	cstr2= (unsigned char)(Num>>8  & 0xff);
	cstr3= (unsigned char)(Num	   & 0xff);

	*c0 = cstr0;
	*c1 = cstr1;
	*c2 = cstr2;
	*c3 = cstr3;
	return 0;
}
/* 	
 *  by slot and local_port get global port index
 */
int dcli_igmp_snp_check_status(unsigned char* stats, unsigned char ismldq, unsigned char* devchk)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned char status = 0, devok = 0;
	unsigned int  op_ret = 0;

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,			\
								NPD_DBUS_VLAN_OBJPATH,		\
								NPD_DBUS_VLAN_INTERFACE,		\
								NPD_DBUS_VLAN_METHOD_CHECK_IGMP_SNP_STATUS);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&ismldq,
							 DBUS_TYPE_INVALID);


	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_igmp,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return IGMPSNP_RETURN_CODE_OK;
	}

	if (dbus_message_get_args ( reply, &err,
								DBUS_TYPE_UINT32,&op_ret,
								DBUS_TYPE_BYTE,&status,
								DBUS_TYPE_BYTE,&devok,
								DBUS_TYPE_INVALID)) 
	{
		if (IGMPSNP_RETURN_CODE_OK == op_ret ) {
			/*DCLI_DEBUG(("success\n"));*/
			*stats = status;
			*devchk = devok;
		}
		else
			op_ret = IGMPSNP_RETURN_CODE_ERROR;
	} 
	else {
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return op_ret;
}

/* 	dcli_vlan_igmp_snp_status_get
 *  by slot and local_port get global port index
 */
int dcli_vlan_igmp_snp_status_get(unsigned short vid,unsigned char* stats)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned char status = 0;
	unsigned int  op_ret = IGMPSNP_RETURN_CODE_OK;
	/*DCLI_DEBUG(("Enter :dcli_igmp_snp_check_status.\n"));*/
	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,			\
								NPD_DBUS_VLAN_OBJPATH,		\
								NPD_DBUS_VLAN_INTERFACE,	\
								NPD_DBUS_VLAN_METHOD_CHECK_VLAN_IGMP_SNP_STATUS);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT16,&vid,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_igmp,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
								DBUS_TYPE_UINT32,&op_ret,
								DBUS_TYPE_BYTE,&status,
								DBUS_TYPE_INVALID)) 
	{
		if (IGMPSNP_RETURN_CODE_OK == op_ret ) {
			/*DCLI_DEBUG(("success\n"));*/
			*stats = status;
		}
		else 
			op_ret = IGMPSNP_RETURN_CODE_ERROR;
	} 
	else {
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return op_ret;
}

int dcli_igmp_snp_vlan_portmbr_status_check
(
	unsigned short vlanId,
	unsigned int slotno,
	unsigned int portno,
	unsigned int* stats
)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int status = 0,op_ret = 0;
	/*DCLI_DEBUG(("Enter :dcli_igmp_snp_check_status.\n"));*/
	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,			\
								NPD_DBUS_VLAN_OBJPATH,		\
								NPD_DBUS_VLAN_INTERFACE,		\
								NPD_DBUS_VLAN_METHOD_CHECK_IGMP_SNP_VLANMBR_STATUS);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT16,&vlanId,
							DBUS_TYPE_UINT32,&slotno,
							DBUS_TYPE_UINT32,&portno,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_igmp,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
								DBUS_TYPE_UINT32,&op_ret,
								DBUS_TYPE_UINT32,&status,
								DBUS_TYPE_INVALID)) 
	{
		if (NPD_DBUS_SUCCESS == op_ret ) {
			/*DCLI_DEBUG(("success\n"));*/
			*stats = status;
		}
	} 
	else {
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return op_ret;
}

int dcli_enable_disable_igmp_one_vlan
(
	struct vty*	vty,
	unsigned short vlanId,
	unsigned int enable
)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned short vid = vlanId;
	unsigned int isEnable = enable;
	int op_ret =0;

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,			\
								NPD_DBUS_VLAN_OBJPATH,		\
								NPD_DBUS_VLAN_INTERFACE,		\
								NPD_DBUS_VLAN_METHOD_CONFIG_IGMP_SNP_VLAN);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT16,&vid,
							 DBUS_TYPE_UINT32,&isEnable,
							 DBUS_TYPE_INVALID);

	/*vty_out(vty,"get params: vid = %d, isEnable = %d.\r\n",vid,isEnable);*/
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_igmp,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return CMD_WARNING;
	}

	if (dbus_message_get_args ( reply, &err,
								DBUS_TYPE_UINT32,&op_ret,
								DBUS_TYPE_INVALID)) {
		/*
		if (NPD_DBUS_SUCCESS == op_ret ) {
			//DCLI_DEBUG(("success\n"));
		}
		if (NPD_IGMP_SNP_NOTENABLE == op_ret){
			vty_out(vty,"IGMP Snooping NOT Enabled Global.\n");
		}
		if (NPD_IGMP_SNP_VLAN_NOTEXIST == op_ret){
			vty_out(vty,"% Bad perameter : vlan %d NOT exists.\n",vid);
		}
		if (NPD_IGMP_SNP_NOTENABLE_VLAN == op_ret){
			vty_out(vty,"% Bad perameter : vlan %d NOT Enabled.\n",vid);
		}
		if (NPD_IGMP_SNP_HASENABLE_VLAN == op_ret){
			vty_out(vty,"% Bad perameter : vlan %d Already Enabled.\n",vid);
		}
		*/
		dbus_message_unref(reply);
		return op_ret ;
	} 
	else {
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
	}
	return CMD_WARNING;
}
/*
 *  by slot and local_port get global port index
 */
int dcli_enable_disable_igmp_one_port
(
	struct vty*	vty,
	unsigned short vlanId,
	unsigned char slot_no,
	unsigned char port_no,
	unsigned char enable
)
{
	DBusMessage *query, *reply;
	DBusError err;
	/*DCLI_DEBUG(("Enter::: dcli_enable_disable_igmp_one_port...\n"));*/
	DCLI_DEBUG(("vid %d,slot_no %d,port_no %d,enable %d.\n",vlanId,slot_no,port_no,enable));
	unsigned char isEnable = enable;
	int op_ret;

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,	\
								NPD_DBUS_VLAN_OBJPATH,	\
								NPD_DBUS_VLAN_INTERFACE,	\
								NPD_DBUS_VLAN_METHOD_CONFIG_IGMP_SNP_ETHPORT);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT16,&vlanId,
							 DBUS_TYPE_BYTE,&slot_no,
							 DBUS_TYPE_BYTE,&port_no,
							 DBUS_TYPE_BYTE,&isEnable,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_igmp,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args( reply, &err,
								DBUS_TYPE_UINT32,&op_ret,
								DBUS_TYPE_INVALID)) {
			if (NPD_DBUS_SUCCESS == op_ret ) {
				/*vty_out(vty,"success\n");*/
				dbus_message_unref(reply);
				return CMD_SUCCESS;
			}
			else{
				/*
				if(NPD_IGMP_SNP_NOTENABLE == op_ret){
					vty_out(vty,"%% Error: IGMP Snooping NOT Enabled Global!\n");
				}
				else if(NPD_IGMP_SNP_PORT_NOTEXIST == op_ret){
					vty_out(vty,"% Bad Parameter :Bad Slot/Port Number!\n");
				}
				else if(NPD_IGMP_SNP_NOTENABLE_PORT == op_ret){
					vty_out(vty,"% Bad parameter :Port %d/%d NOT Enable IGMP Snooping.\n",slot_no,port_no);
				}
				else if(NPD_IGMP_SNP_HASENABLE_PORT == op_ret){
					vty_out(vty,"% Bad parameter :Port %d/%d Already Enable IGMP Snooping.\n",slot_no,port_no);
				}
				else if(NPD_VLAN_NOTEXISTS == op_ret){
					vty_out(vty,"% Bad Parameter :vlan %d Not exist.\n",vlanId);
				}
				else if(NPD_VLAN_NOT_SUPPORT_IGMP_SNP == op_ret){
					vty_out(vty,"% Bad Parameter :vlan %d Not support IGMP snooping.\n",vlanId);
				}
				else if(NPD_DBUS_ERROR_NO_SUCH_PORT == op_ret){
					vty_out(vty,"% Bad Parameter :Illegal Slot/Port Number!\n");
				}
				else if (NPD_VLAN_PORT_NOTEXISTS == op_ret){
					vty_out(vty,"% Bad parameter :Port %d/%d NOT vlan %d's member.\n",slot_no,port_no,vlanId);
				}
				else if (NPD_VLAN_ERR_HW == op_ret){
					vty_out(vty,"%% Error :IGMP config error occurs on HW.\n");
				}
				*/
				dbus_message_unref(reply);
				return op_ret;
			}
	} else {
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
	}
	return CMD_SUCCESS;
}
int dcli_enable_disable_igmp_mcrouter_port
(
	unsigned short vlanId,
	unsigned char slot_no,
	unsigned char port_no,
	unsigned char enDis
)
{
	DBusMessage *query, *reply;
	DBusError err;
	/*printf("Enter::: dcli_enable_disable_igmp_mcrouter_port...\n");
	//printf("vid %d,slot_no %d,port_no %d,enable %d.\n",vlanId,slot_no,port_no,enDis);*/
	unsigned short vid = vlanId;
	unsigned char isEnable = enDis,slotNo =slot_no,localPortNo = port_no;
	int ret = CMD_SUCCESS;
	
	query = dbus_message_new_method_call(
										NPD_DBUS_BUSNAME,			\
										NPD_DBUS_VLAN_OBJPATH,		\
										NPD_DBUS_VLAN_INTERFACE,	\
										NPD_DBUS_VLAN_METHOD_IGMP_SNP_ADD_DEL_MCROUTE_PORT);
	dbus_error_init(&err);

    dbus_message_append_args(query,
							DBUS_TYPE_UINT16,&vid,
							DBUS_TYPE_BYTE,	&slotNo,
							DBUS_TYPE_BYTE,	&localPortNo,
							DBUS_TYPE_BYTE,	&isEnable,
							DBUS_TYPE_INVALID);


	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_igmp,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return CMD_WARNING;
	}

	if (dbus_message_get_args ( reply, &err,
								DBUS_TYPE_UINT32,&ret,				
								DBUS_TYPE_INVALID)) {
		/*error code proccess in dcli_igmp_snp.c*/
		if (NPD_DBUS_SUCCESS == ret ) {
			dbus_message_unref(reply);
			return CMD_SUCCESS;
		}
		else{
			dbus_message_unref(reply);
			return ret;
		}
	} 
	else {
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_WARNING;
}
#if 0
int dcli_show_igmp_snp_one_mcgroup
(
	struct vty*		vty,
	unsigned short vid,
	unsigned short vidx
)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	
	DBusMessageIter	 iter_array;
	unsigned int	i,count,ret;
	unsigned short	vlanId = vid,vIdx = vidx;
	unsigned int 	productId = 0;
	/***********************************/
	unsigned int mcMbrBmp = 0,groupIp =0;
	unsigned char ip0=0,ip1=0,ip2=0,ip3=0;
	//DCLI_DEBUG(("Enter: dcli_show_igmp_snp_one_mcgroup..\n"));
	//DCLI_DEBUG(("get params: vid %d, vidx %d.\n",vid,vidx));
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,\
										NPD_DBUS_VLAN_OBJPATH ,	\
										NPD_DBUS_VLAN_INTERFACE ,\
										NPD_DBUS_VLAN_METHOD_SHOW_ONE_MCGROUP_PORT_MEMBERS );
	

	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT16,&vlanId,
							 DBUS_TYPE_UINT16,&vIdx,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args( reply, &err,
					DBUS_TYPE_UINT32, &ret,
					//DBUS_TYPE_UINT16, &vlanId,
					DBUS_TYPE_UINT32, &groupIp,
					DBUS_TYPE_UINT32, &mcMbrBmp,
					DBUS_TYPE_INVALID)) {
		vty_out(vty,"vlanId %d, vidx %d, groupIp 0x%x, groupMbrBmp %#0X,\n",vlanId,vidx,groupIp,mcMbrBmp);
		if(0==ret) {
			vty_out(vty,"========================================================================\n");
			vty_out(vty,"%-7s  %-4s  %-15s  %-40s\n","VLAN ID","VIDX","GROUP_IP","PORT MEMBER LIST");
			vty_out(vty,"=======  ====  ===============  ========================================\n");

			ltochararry(groupIp,&ip0,&ip1,&ip2,&ip3);
			vty_out(vty,"%-7d  ",vlanId);
			vty_out(vty,"%-4d  ",vidx);
			vty_out(vty,"%u.%u.%u.%u  ",ip0,ip1,ip2,ip3);
			show_group_member_slot_port(vty,productId,mcMbrBmp);
			vty_out(vty,"\n");
			vty_out(vty,"=====================================================================\n");
		}
		/*
		else if(NPD_DBUS_ERROR_NO_SUCH_GROUP == ret) {
			vty_out(vty,"% Bad parameter: vidx illegal.\n");
		}
		*/
		else if(NPD_GROUP_NOTEXIST == ret) {
			vty_out(vty, "% Bad parameter: l2mc entry %d NOT Exists.\n",vidx);
		}
		else if(NPD_VLAN_ERR_HW == ret) {
			vty_out(vty,"Error occurs in Read vidx Entry in HW.\n");
		}
	}
	else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
#endif

void dcli_show_igmp_snp_mcgroup_list
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
		vty_out(vty,"========================================================================\n");
		vty_out(vty,"%-7s  %-4s  %-15s  %-40s\n","VLAN ID","VIDX","GROUP_IP","PORT MEMBER LIST");
		vty_out(vty,"=======  ====  ===============  ========================================\n");

		for (j = 0; j < group_Cont; j++) {
			DBusMessageIter iter_struct;
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&vidx);
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&groupIp);
			dbus_message_iter_next(&iter_struct);
			for(i=0; i<SIZE_OF_IPV6_ADDR; i++){
				dbus_message_iter_get_basic(&iter_struct,&(groupIpv6[i]));
				dbus_message_iter_next(&iter_struct);
			}			
			
			dbus_message_iter_get_basic(&iter_struct,&(mbrBmp.portMbr[0]));
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(mbrBmp.portMbr[1]));
			dbus_message_iter_next(&iter_array);

			ltochararry(groupIp,&ip0,&ip1,&ip2,&ip3);
			vty_out(vty,"%-7d  ",vlanId);
			vty_out(vty,"%-4d  ",vidx);
			vty_out(vty,"%-3u.%-3u.%-3u.%-3u  ",ip0,ip1,ip2,ip3);
			show_group_member_slot_port(vty,product_id,mbrBmp);
			vty_out(vty,"\n");
		}
		vty_out(vty,"========================================================================\n");
	}
	else if(IGMPSNP_RETURN_CODE_GROUP_NOTEXIST == ret) {
		vty_out(vty,"%% Error:layer 2 multicast entry not exist!\n");
	}
	else if (IGMPSNP_RETURN_CODE_ERROR_HW == ret) {
		vty_out(vty,"%% Read layer 2 multicast entry from hardware error!\n");				
	}
	else if(IGMPSNP_RETURN_CODE_NOT_ENABLE_GBL == ret){
		vty_out(vty,"%% Error:igmp snooping not enabled global!\n");
	}
	else if(IGMPSNP_RETURN_CODE_NOTENABLE_VLAN == ret){
		vty_out(vty,"%% Error:vlan %d not enabled igmp snooping!\n",vlanId);
	}
	else if(IGMPSNP_RETURN_CODE_MC_VLAN_NOT_EXIST == ret){
		vty_out(vty,"%% Error:layer 2 multicast vlan %d not exist!\n",vlanId);
	}
	else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == ret)
		vty_out(vty,"%% Product not support this function!\n");
	else {
		vty_out(vty,"%% Error %d\n",ret);
	}
		
	dbus_message_unref(reply);
	return;

}
/*************************************************************************** 
 * function : Show all layer 2 multicast group's count in the vlan
 *     name : dcli_show_igmp_snp_mcgroup_count
 *     input :
 *			struct vty       *vty
 *			unsigned short vlanId
 *   output :
 *			null
 *  re_value: void
 ***************************************************************************/
void dcli_show_igmp_snp_mcgroup_count
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
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query,-1, &err);
	
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
		vty_out(vty,"%% Error:igmp snooping not enabled global!\n");
	}
	else if(IGMPSNP_RETURN_CODE_NOTENABLE_VLAN == ret){
		vty_out(vty,"%% Error:vlan %d not enabled igmp snooping!\n", vlanId);
	}
	else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == ret){
		vty_out(vty,"%% Product not support this function!\n");
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


int dcli_get_slot_port_by_gindex
(
	unsigned int port_index,
	unsigned char *slot_no,
	unsigned char *local_port_no
) 
{
	DBusMessage *query, *reply;
	DBusError err;
	int op_ret;
	unsigned int eth_g_index = port_index;
	unsigned char slot = 0,port = 0;

	dbus_error_init(&err);
	query = dbus_message_new_method_call(				\
										NPD_DBUS_BUSNAME,	\
										NPD_DBUS_VLAN_OBJPATH ,	\
										NPD_DBUS_VLAN_INTERFACE ,	\
										NPD_DBUS_VLAN_METHOD_GET_SLOTPORT_BY_INDEX
										);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&eth_g_index,										
							DBUS_TYPE_INVALID);


	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_igmp,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get re_slot_port reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return CMD_WARNING;
	}
	
	if (dbus_message_get_args ( reply, &err,
								DBUS_TYPE_UINT32,&op_ret,
								DBUS_TYPE_BYTE,&slot,
								DBUS_TYPE_BYTE,&port,
								DBUS_TYPE_INVALID)) {
		if(NPD_DBUS_SUCCESS == op_ret){
			*slot_no = slot;
			*local_port_no = port;
			/*DCLI_DEBUG(("eth port index %d -->slot %d,port %d.",eth_g_index,slot,port));*/
			dbus_message_unref(reply);
			return CMD_SUCCESS;
		}
		else if(NPD_DBUS_ERROR_NO_SUCH_PORT == op_ret){
			printf("Bad slot/port Number.\n");
			dbus_message_unref(reply);
			return CMD_WARNING;
		}		
	} 
	else {
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) {
				printf("%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return CMD_WARNING;
	}
		
}

/*************************************************************************** 
 * function : show the user profile route-port in the vlan
 *	name : igmp_snp_show_running_routeport
 *	input :
 *		struct vty 	 *vty
 *		unsigned short vlanId		-lan id
 *
 *	output :
 *		PORT_MEMBER_BMP *routeBmp	-route Bmp
 *
 *  re_value: ret
 *			CMD_WARNING	-dbus err
 *			CMD_SUCCESS	-success
 ***************************************************************************/
int dcli_igmp_snp_show_running_routeport
(
	struct vty		*vty,
	unsigned short	vlanId,
	PORT_MEMBER_BMP	*routeBmp
)
{
	DBusMessage	*query;
	DBusMessage	*reply;
	DBusError	err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;

	unsigned int ret = 0;
	unsigned int i = 0;
	unsigned int count = 0;
	PORT_MEMBER_BMP tmp_routeBmp = {0};
	long		 routeArray[MAX_ETHPORT_PER_BOARD * DCLI_IGMP_MAX_CHASSIS_SLOT_COUNT];

	/* init routeArray */
	for (i = 0; i < (MAX_ETHPORT_PER_BOARD * DCLI_IGMP_MAX_CHASSIS_SLOT_COUNT); i++)
	{
		routeArray[i] = -1;
	}

	/* show the user profile route-port */
	query = dbus_message_new_method_call(
										IGMP_DBUS_BUSNAME,    \
										IGMP_DBUS_OBJPATH,    \
										IGMP_DBUS_INTERFACE,  \
										IGMP_SNP_DBUS_METHOD_IGMP_SNP_SHOW_ROUTE_PORT);

	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT16, &vlanId,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_igmp, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
	{
		vty_out(vty, "Failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}

	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);

	if (IGMPSNP_RETURN_CODE_OK == ret)
	{
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter, &count);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter, &iter_array);

		for (i = 0; i < count; i++)
		{
			DBusMessageIter iter_struct;
			dbus_message_iter_recurse(&iter_array, &iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &routeArray[i]);
			dbus_message_iter_next(&iter_array);
		}
	}

	if (IGMPSNP_RETURN_CODE_OK == ret)
	{
		/* exchange ifindex into slotport  */
		ret = dcli_igmp_snp_exchange_ifindex_to_slotport(vty, count, routeArray, &tmp_routeBmp);
		if (ret != CMD_SUCCESS)
		{
			(*routeBmp).portMbr[0] = 0;
			(*routeBmp).portMbr[1] = 0;
			return CMD_WARNING;
		}
	}

	(*routeBmp).portMbr[0] = tmp_routeBmp.portMbr[0];
	(*routeBmp).portMbr[1] = tmp_routeBmp.portMbr[1];

	return CMD_SUCCESS;	
}

/*************************************************************************** 
 * function : show the user profile route-port in the vlan
 *	name : dcli_igmp_snp_exchange_ifindex_to_slotport
 *	input :
 *		struct vty  	*vty
 *		unsigned int   count		-count of ifindex array  which need to exchange
 *	 	long			*routeArray	-ifindex array which need to exchange
 *
 *	output :
 *		unsigned int *routeBmp		-exchange result
 *
 *  re_value: ret
 *			CMD_WARNING	-dbus err
 *			CMD_SUCCESS	-success
 **************************************************************************/
int dcli_igmp_snp_exchange_ifindex_to_slotport
(
	struct vty	  *vty,
	unsigned int  count,
	long		  *routeArray,
	PORT_MEMBER_BMP *routeBmp
)
{
	DBusMessage		*query;
	DBusMessage		*reply;
	DBusError		err;
	DBusMessageIter	iter;
	DBusMessageIter	iter_array;

	unsigned int 	i = 0;
	unsigned int 	ret = 0;
	PORT_MEMBER_BMP	temp_routeBmp = {0};

	query = dbus_message_new_method_call(
										NPD_DBUS_BUSNAME,
										NPD_DBUS_VLAN_OBJPATH,
										NPD_DBUS_VLAN_INTERFACE,
										NPD_DBUS_VLAN_METHOD_IGMP_SNP_EXCHANGE_IFINDEX_TO_SLOTPORT);

	dbus_error_init(&err);

	dbus_message_iter_init_append(query, &iter);

	dbus_message_iter_append_basic(&iter,
								DBUS_TYPE_UINT32, &count);
	if (count > 0) {
		dbus_message_iter_open_container(&iter,
									   DBUS_TYPE_ARRAY,
									   DBUS_STRUCT_BEGIN_CHAR_AS_STRING
											DBUS_TYPE_UINT32_AS_STRING	 /* port index*/
									   DBUS_STRUCT_END_CHAR_AS_STRING,
									   &iter_array);

		for (i = 0; i < count; i++) {
			DBusMessageIter iter_struct;
			dbus_message_iter_open_container(&iter_array,
										   DBUS_TYPE_STRUCT, NULL, &iter_struct);
			dbus_message_iter_append_basic(&iter_struct,
										  DBUS_TYPE_UINT32, &(routeArray[i]));  /* port index*/
			dbus_message_iter_close_container(&iter_array, &iter_struct);
		}
		dbus_message_iter_close_container(&iter, &iter_array);
	}

	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection_igmp, query, -1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply)
	{
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}

	if (dbus_message_get_args(reply, &err,
								DBUS_TYPE_UINT32, &ret,
								DBUS_TYPE_UINT32, &temp_routeBmp.portMbr[0],
								DBUS_TYPE_UINT32, &temp_routeBmp.portMbr[1],
								DBUS_TYPE_INVALID))
	{
		(*routeBmp).portMbr[0] = temp_routeBmp.portMbr[0];
		(*routeBmp).portMbr[1] = temp_routeBmp.portMbr[1];
		dbus_message_unref(reply);
		return CMD_SUCCESS;
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
		return CMD_WARNING;
	}
}

/**********************************************************************************
 *  dcli_get_product_igmp_function
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

int dcli_get_product_igmp_function(struct vty* vty,unsigned char *IsSupport)
{
	DBusMessage *query, *reply;
	DBusError err; 
	int op_ret = 0;
	unsigned char is_support = 0;

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,			\
								NPD_DBUS_VLAN_OBJPATH,		\
								NPD_DBUS_VLAN_INTERFACE,		\
								IGMP_DBUS_METHOD_GET_SUPPORT_FUNCTION);

	
	dbus_error_init(&err);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_igmp,query,-1, &err);
	
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

