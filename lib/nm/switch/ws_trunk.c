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
* ws_trunk.c
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
/*dcli_trunk.c V1.34-V1.59*/
/*dcli_trunk.h V1.18*/
/*author tangsiqi*/
/*update 10-02-02*/




#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "ws_init_dbus.h"
#include <dbus/dbus.h>
#include "sysdef/npd_sysdef.h"
#include "dbus/npd/npd_dbus_def.h"
#include "util/npd_list.h"
#include "npd/nam/npd_amapi.h"
#include "ws_dcli_vlan.h"
#include "ws_trunk.h"
#include "ws_returncode.h"

typedef enum {
	LOAD_BANLC_SRC_DEST_MAC = 0 ,
	LOAD_BANLC_SOURCE_DEV_PORT ,
	LOAD_BANLC_SRC_DEST_IP,
	LOAD_BANLC_TCP_UDP_RC_DEST_PORT ,
	LOAD_BANLC_MAC_IP,
	LOAD_BANLC_MAC_L4,
	LOAD_BANLC_IP_L4 ,
	LOAD_BANLC_MAC_IP_L4,
	LOAD_BANLC_MAX
}trkLoadBanlcMode;

char *trunkLoadBalanc[LOAD_BANLC_MAX] = {
	 "mac",
	 "ingress port",
	 "ip",
	 "tcp/udp",
	 "mac+ip",
	 "mac+l4",
	 "ip+l4",
	 "mac+ip+l4"
};


int parse_trunk_no(char* str,unsigned short* trunkId)
{
	char *endptr = NULL;
	char c = 0;
	if (NULL == str) return NPD_FAIL;
	c = str[0];
	if (c>'0'&&c<='9'){
		*trunkId= (unsigned short)strtoul(str,&endptr,10);
		if('\0' != endptr[0]){
			return NPD_FAIL;
		}
		return NPD_SUCCESS;	
	}
	else {
		return NPD_FAIL; //not Vlan ID. for Example ,enter '.',and so on ,some special char.
	}
}


/*dcli_trunk.c V1.29*/
/*author qiaojie*/
/*update time 08-10-7*/

int show_trunk_member_slot_port(unsigned int product_id,PORT_MEMBER_BMP mbrBmp_sp,PORT_MEMBER_BMP disMbrBmp_sp,struct port_profile *phead,unsigned int *port_num)
{
	unsigned int i = 0;
	unsigned int  tmpVal[2];
	memset(&tmpVal,0,sizeof(tmpVal));
	unsigned int count = 0;
	unsigned int slot = 0,port = 0;    
	struct port_profile *pq,*ptail;

	ptail=phead;	
	for (i=0;i<64;i++)
	{
		if(PRODUCT_ID_AX7K == product_id) 
		{
			slot = i/8 + 1;
			port = i%8;
		}
		else if((PRODUCT_ID_AX5K == product_id) ||
				(PRODUCT_ID_AX5K_I == product_id) ||
				(PRODUCT_ID_AU4K == product_id) ||
				(PRODUCT_ID_AU3K == product_id) ||
				(PRODUCT_ID_AU3K_BCM == product_id) ||
				(PRODUCT_ID_AU3K_BCAT == product_id) || 
				(PRODUCT_ID_AU2K_TCAT == product_id)){
			slot = 1;
			port = i;
		}
		tmpVal[i/32] = (1<<(i%32));
		if((mbrBmp_sp.portMbr[i/32]) & tmpVal[i/32]) 
		//if((mbrBmp_sp) & tmpVal) 
		{
			pq=(struct port_profile*)malloc(sizeof(struct port_profile));
			pq->slot=slot;
			pq->port=port;
			count++;
			pq->next=NULL;
			ptail->next=pq;
			ptail=pq; 
		}
	}
	
	slot = 0;
	port = 0;
	memset(&tmpVal,0,sizeof(tmpVal));
	for (i=0;i<64;i++)
	{
		if(PRODUCT_ID_AX7K == product_id) 
		{
			slot = i/8 + 1;
			port = i%8;
		}
		else if((PRODUCT_ID_AX5K == product_id) ||
				(PRODUCT_ID_AX5K_I == product_id) ||
				(PRODUCT_ID_AU4K == product_id) ||
				(PRODUCT_ID_AU3K == product_id) ||
				(PRODUCT_ID_AU3K_BCM == product_id) ||
				(PRODUCT_ID_AU3K_BCAT == product_id) || 
				(PRODUCT_ID_AU2K_TCAT == product_id)){
			slot = 1;
			port = i;
		}
		tmpVal[i/32] = (1<<(i%32));
		if((disMbrBmp_sp.portMbr[i/32]) & tmpVal[i/32]) 
		{				
		    pq=(struct port_profile*)malloc(sizeof(struct port_profile));
			pq->slot=slot;
			pq->port=port;
			count++;
			pq->next=NULL;
			ptail->next=pq;
			ptail=pq; 
		}
	}
	*port_num=count;	
	return 0;
}


int trunk_name_legal_check(char* str,unsigned int len)
{
	int i = 0;
	int ret = NPD_FAIL;
	char c = 0;
	if((NULL == str)||(len==0)){
		return ret;
	}
	if(len >= ALIAS_NAME_SIZE){
		ret = ALIAS_NAME_LEN_ERROR;
		return ret;
	}

	c = str[0];
	if(	(c=='_')||
		(c<='z'&&c>='a')||
		(c<='Z'&&c>='A')
	  ){
		ret =NPD_SUCCESS;
	}
	else {
		return ALIAS_NAME_HEAD_ERROR;
	}
	for (i=1;i<=len-1;i++){
		c = str[i];
		if( (c>='0' && c<='9')||
			(c<='z'&&c>='a')||
		    (c<='Z'&&c>='A')||
		    (c=='_')){
			continue;
		}
		else {
			ret =ALIAS_NAME_BODY_ERROR;
			break;
		}
	}
	return ret;
}

/*由于编译报重复定义，故将函数名由parse_vlan_no改为parse_vlan_no_trunk*/
int parse_vlan_no_trunk(char* str,unsigned short* vlanId) {
	char *endptr = NULL;
	char c;
	if (NULL == str) return NPD_FAIL;
	c = str[0];
	if (c>='0'&&c<='9'){
		*vlanId= strtoul(str,&endptr,10);
		if('\0' != endptr[0]){
				return NPD_FAIL;
		}
		return NPD_SUCCESS;	
	}
	else {
		return NPD_FAIL; //not Vlan ID. for Example ,enter '.',and so on ,some special char.
	}
}


/*当show_trunk_list==1且trunk_num>0时*/
void Free_trunk_head( struct trunk_profile *head)
{
  struct trunk_profile *f1,*f2;
  struct port_profile *pf1,*pf2;
  int i;
  unsigned int  tmpVal[2];
  memset(&tmpVal,0,sizeof(tmpVal));
  f1=head->next;
  f2=f1->next;

  while(f2!=NULL)
  {

    if(0 != f1->masterFlag)
	{
		
	  pf1=f1->portHead->next;
	  for(i=0;i<64;i++)
	  {
	    tmpVal[i/32] = (1<<(i%32));
		if((f1->mbrBmp_sp.portMbr[i/32]) & tmpVal[i/32])
		//if((f1->mbrBmp_sp) & tmpVal) 
		{
	      pf2=pf1->next;

          free(pf1);

	      pf1=pf2;
		}
	  }

	  memset(&tmpVal,0,sizeof(tmpVal));
	  for(i=0;i<64;i++)
	  {
	    tmpVal[i/32] = (1<<(i%32));
		if((f1->disMbrBmp_sp.portMbr[i/32]) & tmpVal[i/32])
	    //if((f1->disMbrBmp_sp) & tmpVal) 
	    {
	      pf2=pf1->next;

          free(pf1);

	      pf1=pf2;
	    }
	  }
	  free(f1->portHead);
	}

	//free(f1->trunkName);

	free(f1);

	f1=f2;
	f2=f2->next;
  }
  if(0 != f1->masterFlag)
  {
    pf1=f1->portHead->next;
	 for(i=0;i<64;i++)
	  {
	    tmpVal[i/32] = (1<<(i%32));
	 if((f1->mbrBmp_sp.portMbr[i/32]) & tmpVal[i/32])
	  {
	    pf2=pf1->next;
        free(pf1);
	    pf1=pf2;
	  }
	}

	memset(&tmpVal,0,sizeof(tmpVal));
	for(i=0;i<64;i++)
	  {
	    tmpVal[i/32] = (1<<(i%32));
		if((f1->disMbrBmp_sp.portMbr[i/32]) & tmpVal[i/32])
	  {
	    pf2=pf1->next;
        free(pf1);
	    pf1=pf2;
	  }
	}
	free(f1->portHead);
  }

  //free(f1->trunkName);

  free(f1);
}



/*当show_trunk_byid==1*/
void Free_trunk_one( struct trunk_profile *trunk)
{
  struct port_profile *pf1,*pf2;
  int i;
  unsigned int  tmpVal[2];
  memset(&tmpVal,0,sizeof(tmpVal));
  if(0 != trunk->masterFlag)
  {
    pf1=trunk->portHead->next;
	for(i=0;i<64;i++)
	  {
	   tmpVal[i/32] = (1<<(i%32));
	 if((trunk->mbrBmp_sp.portMbr[i/32]) & tmpVal[i/32])
	  {
	    pf2=pf1->next;
        free(pf1);
	    pf1=pf2;
	  }
	}

    memset(&tmpVal,0,sizeof(tmpVal));
	for(i=0;i<64;i++)
	  {
	    tmpVal[i/32] = (1<<(i%32));
		if((trunk->disMbrBmp_sp.portMbr[i/32]) & tmpVal[i/32])
	  {
	    pf2=pf1->next;
        free(pf1);
	    pf1=pf2;
	  }
	}
	free(trunk->portHead);
  }
}


/*当show_trunk_vlanlist==1*/
void Free_vlanlist_trunk_head( struct trunk_profile *head)
{
  struct vlan_profile *f1,*f2;
  if(head->vlan_Cont>0)
  {
    f1=head->vlanHead;
	f2=f1->next;
	while(f2!=NULL)
	{
      free(f1);
	  f1=f2;
	  f2=f2->next;
	}
	free(f1);
  }
  //free(head->vlanHead);
}


/*注意MASTER PORT的结果
   if(0 != masterFlag) 
      vty_out(vty,"%d/%-11d  ",mSlotNo,mPortNo);
    else 
      vty_out(vty,"%-13s	","No masterPort");
*/
int show_trunk_list(struct trunk_profile *trunk_head, int *trunk_num)      /*失败返回0，成功返回1，返回-1表示no trunk*/
                                                                             /*返回-2表示error.，返回-3表示getting portlist Fail.*/
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	struct trunk_profile *q,*tail;	
	unsigned int trunk_Cont = 0;
	unsigned short  trunkId = 0;
	char *			trunkName;
	unsigned char 	masterFlag = 0;	
	unsigned int   mSlotNo = 0, mPortNo = 0,loadBalanc = 0;
	unsigned int	product_id = 0; 
	unsigned int 	j,ret;
	int retu=1;
	unsigned int     trunkStat = 0;
    PORT_MEMBER_BMP mbrBmp_sp, disMbrBmp_sp;
	memset(&mbrBmp_sp,0,sizeof(PORT_MEMBER_BMP));
	memset(&disMbrBmp_sp,0,sizeof(PORT_MEMBER_BMP));


	trunkName = (char*)malloc(ALIAS_NAME_SIZE);
	memset(trunkName,0,ALIAS_NAME_SIZE);

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_TRUNK_OBJPATH ,	\
										NPD_DBUS_TRUNK_INTERFACE ,	\
										NPD_DBUS_TRUNK_METHOD_SHOW_TRUNKLIST_PORT_MEMBERS_V1 );
	

	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		if(NULL!=trunkName){free(trunkName);trunkName=NULL;}
		return 0;
	}

	dbus_message_iter_init(reply,&iter);

	dbus_message_iter_get_basic(&iter,&ret);
	if(TRUNK_RETURN_CODE_ERR_NONE == ret){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&trunk_Cont);

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&product_id);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);

        trunk_head->next = NULL;
	    tail=trunk_head;
	    *trunk_num = trunk_Cont;
		for (j = 0; j < trunk_Cont; j++) {
			DBusMessageIter iter_struct;
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&trunkId);
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&trunkName);
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&masterFlag);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&mSlotNo);
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&mPortNo);
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&mbrBmp_sp.portMbr[0]);

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&mbrBmp_sp.portMbr[1]);

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&disMbrBmp_sp.portMbr[0]);

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&disMbrBmp_sp.portMbr[1]);

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&loadBalanc);

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&trunkStat);

			dbus_message_iter_next(&iter_array);  
 
			
            q=(struct trunk_profile*)malloc(sizeof(struct trunk_profile));
			
			q->trunkId=trunkId;
			
			memset(q->trunkName,0,NPD_TRUNK_IFNAME_SIZE);
			memcpy(q->trunkName,trunkName,NPD_TRUNK_IFNAME_SIZE);

			memset(q->loadBalanc,0,20);
			memcpy(q->loadBalanc,trunkLoadBalanc[loadBalanc],20);

			q->masterFlag=masterFlag;
			if(0 != masterFlag)
			{
			  q->mSlotNo=mSlotNo;
              q->mPortNo=mPortNo;
			  q->mbrBmp_sp=mbrBmp_sp;
			  q->disMbrBmp_sp=disMbrBmp_sp;
			  q->portHead=(struct port_profile*)malloc(sizeof(struct port_profile));
			  q->portHead->next=NULL;
			  show_trunk_member_slot_port(product_id,mbrBmp_sp,disMbrBmp_sp,q->portHead,&q->port_Cont);
			}
            q->next=NULL;
		    tail->next=q;
		    tail=q; 
		}
	}	
	else if(TRUNK_RETURN_CODE_TRUNK_NOTEXISTS == ret) {
		//vty_out(vty,"%% There is no vaild trunk exist.\n");
		retu=-1;
	}
	else if(TRUNK_RETURN_CODE_ERR_HW == ret) {
		//vty_out(vty,"%% Error occurs in read trunk entry in hardware.\n");	
		retu=-2;
	}
	else if(TRUNK_RETURN_CODE_ERR_GENERAL == ret) {
		//vty_out(vty,"%% Error,operation on trunk portlist fail.\n");
		retu=-3;
	}

	dbus_message_unref(reply);
	return retu;
}


/*注意判断id的合法性<1-127>*/
/*注意MASTER PORT的结果
   if(0 != masterFlag) 
      vty_out(vty,"%d/%-11d  ",mSlotNo,mPortNo);
    else 
      vty_out(vty,"%-13s	","No masterPort");
*/
int show_trunk_byid(int id,struct trunk_profile *trunk) /*传递要显示的trunk的id号，返回trunk的所有信息*/
                                                            /*返回0表示失败，,返回1表示成功，返回-1表示trunk ID非法*/
                                                            /*返回-2表示trunk not Exists，返回-3表示error*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	//DBusMessageIter	 iter;
	
	int retu=1;
	unsigned short	trunkId = 0;
	char*			trunkName = NULL;
	unsigned int	ret = 0;
	unsigned char   masterFlag = 0;
	unsigned int    mSlotNo = 0, mPortNo = 0;
	unsigned int 	product_id = 0,loadBalanc = 0;
	unsigned int 	trunkStat = 0;
	PORT_MEMBER_BMP mbrBmp_sp, disMbrBmp_sp;
	memset(&mbrBmp_sp,0,sizeof(PORT_MEMBER_BMP));
	memset(&disMbrBmp_sp,0,sizeof(PORT_MEMBER_BMP));

	trunkId=(unsigned short)id;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_TRUNK_OBJPATH ,	\
										NPD_DBUS_TRUNK_INTERFACE ,	\
										NPD_DBUS_TRUNK_METHOD_SHOW_TRUNK_PORT_MEMBERS_V1 );
	

	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT16,&trunkId,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		retu = 0;
	}
	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &ret,
     				DBUS_TYPE_STRING, &trunkName,
					DBUS_TYPE_UINT32, &product_id,
					DBUS_TYPE_BYTE,	  &masterFlag,
     				DBUS_TYPE_UINT32, &mSlotNo,
     				DBUS_TYPE_UINT32, &mPortNo,
					DBUS_TYPE_UINT32, &mbrBmp_sp.portMbr[0],
					DBUS_TYPE_UINT32, &mbrBmp_sp.portMbr[1],
					DBUS_TYPE_UINT32, &disMbrBmp_sp.portMbr[0],
					DBUS_TYPE_UINT32, &disMbrBmp_sp.portMbr[1],
					DBUS_TYPE_UINT32, &loadBalanc,
					DBUS_TYPE_UINT32, &trunkStat,
					DBUS_TYPE_INVALID)) {
		if(TRUNK_RETURN_CODE_ERR_NONE == ret) {
			retu = 1;
			trunk->trunkId=trunkId;
			
			memset(trunk->trunkName,0,NPD_TRUNK_IFNAME_SIZE);
			memcpy(trunk->trunkName,trunkName,NPD_TRUNK_IFNAME_SIZE);

			memset(trunk->loadBalanc,0,20);
			memcpy(trunk->loadBalanc,trunkLoadBalanc[loadBalanc],20);

			trunk->masterFlag=masterFlag;
			if(0 != masterFlag)
			{
				trunk->mSlotNo=mSlotNo;
				trunk->mPortNo=mPortNo;
			}
			trunk->mbrBmp_sp=mbrBmp_sp;
			trunk->disMbrBmp_sp=disMbrBmp_sp;
			trunk->portHead=(struct port_profile*)malloc(sizeof(struct port_profile));
			trunk->portHead->next=NULL;
			show_trunk_member_slot_port(product_id,mbrBmp_sp,disMbrBmp_sp,trunk->portHead,&trunk->port_Cont);
		}
		else if(ETHPORT_RETURN_CODE_NO_SUCH_TRUNK == ret) {
			//vty_out(vty,"% Bad parameter,illegal trunk Id.\n");
			retu = -1;
		}
		else if(TRUNK_RETURN_CODE_TRUNK_NOTEXISTS == ret) {
			//vty_out(vty,"% Bad parameter,trunk %d not exist.\n",trunkId);
			retu = -2;
		}
		else if(TRUNK_RETURN_CODE_ERR_GENERAL == ret) {
			//vty_out(vty,"%% Error,operation on trunk portlist fail.\n");
			retu = -3;
		}		
	}
	else {
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return retu;
}

int show_trunk_vlanlist(int trunk_id,struct trunk_profile *trunk_head)	 /*失败返回0，成功返回1，返回-1表示Illegal trunk Id，返回-2表示trunk not exists*/
											               					 /*返回-3表示error，返回-4表示Getting trunk allow vlanlist fail*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter	 iter;	
	DBusMessageIter	 iter_array;	
	struct vlan_profile *q,*tail;
	unsigned short	trunkId =0,vlanId = 0;//vlanId[NPD_MAX_VLAN_ID] = {0};
	char			*trunkName = "No name",*vlanName = NULL;
	unsigned int	i = 0,vlan_Cont,ret;
	int retu=1;	
	unsigned char	tagMode =0;
	
	trunkId = (unsigned short)trunk_id;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_TRUNK_OBJPATH ,\
										NPD_DBUS_TRUNK_INTERFACE ,	\
										NPD_DBUS_TRUNK_METHOD_SHOW_TRUNK_VLAN_AGGREGATION );
	

	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT16,&trunkId,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return 0;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(VLAN_RETURN_CODE_ERR_NONE == ret){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&trunkName);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&vlan_Cont);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);

		trunk_head->trunkId=trunkId;
		memset(trunk_head->trunkName,0,NPD_TRUNK_IFNAME_SIZE);
		memcpy(trunk_head->trunkName,trunkName,NPD_TRUNK_IFNAME_SIZE);
		trunk_head->vlanHead=(struct vlan_profile*)malloc(sizeof(struct vlan_profile));
		trunk_head->vlanHead->next=NULL;
		tail=trunk_head->vlanHead;
		trunk_head->vlan_Cont=vlan_Cont;
		for(i=0;i<vlan_Cont;i++){
			DBusMessageIter iter_struct;
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&vlanId);

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&vlanName);
			
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&tagMode);
				
			dbus_message_iter_next(&iter_array);
			q=(struct vlan_profile*)malloc(sizeof(struct vlan_profile));
			
			q->vlanId=vlanId;
			memset(q->vlanName,0,NPD_TRUNK_IFNAME_SIZE);
			memcpy(q->vlanName,vlanName,NPD_TRUNK_IFNAME_SIZE);
			q->tagMode=tagMode;
			
			q->next=NULL;
			tail->next=q;
			tail=q; 	
		}
	}
	#if 0
	else if(NPD_DBUS_ERROR_NO_SUCH_TRUNK+1== ret) {
		retu=-1;
	}
	else if(NPD_TRUNK_NOTEXISTS == ret) {
		retu=-2;
	}
	else if(TRUNK_CONFIG_FAIL == ret) {
		retu=-3;
	}
	else if(NPD_TRUNK_GET_ALLOWVLAN_ERR == ret) {
		retu=-4;
	}
	#endif
	///////////new return 
	else if(ETHPORT_RETURN_CODE_NO_SUCH_TRUNK == ret) {
		//vty_out(vty,"%% Bad parameter,illegal trunk Id.\n");
		retu=-1;
	}
	else if(TRUNK_RETURN_CODE_TRUNK_NOTEXISTS == ret) {
		//vty_out(vty,"%% Bad parameter,trunk %d not exist.\n",trunkId);
		retu=-2;
	}
	else if(TRUNK_RETURN_CODE_ERR_HW == ret) {
		//vty_out(vty,"%% Error,operation on hardware fail.\n");
		retu=-3;
	}
	else if(TRUNK_RETURN_CODE_GET_ALLOWVLAN_ERR == ret) {
		//vty_out(vty,"%% Error,operation on getting trunk allow vlanlist fail.\n");
		retu=-4;
	}

	//////////
	dbus_message_unref(reply);
	return retu;
}


/*注意判断id合法性<1-127>，name长度0x15，以_或字母开头......*/
int create_trunk(char *id, char *trunk_name)    /*返回0表示失败，返回1表示成功，返回-1表示trunk name can  not be list*/
                                                /*返回-2表示trunk name too long，返回-3表示trunk name begins with an illegal char*/
                                                /*返回-4表示trunk name contains illegal char，返回-5表示trunk id illeagal*/
                                                /*返回-6表示trunkID Already Exists，返回-7表示trunkName Already Exists*/
                                                /*返回-8表示error*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;

	unsigned short 	trunkId = 0;
	char*  trunkName =NULL;
	int ret = 0,retu=1;
	unsigned int op_ret = 0;
	
	//trunkId = (unsigned short) id;
	ret = parse_trunk_no((char*)id,&trunkId);
	if (NPD_FAIL == ret) {
		return -5; 
	}

	
	trunkName = (char*)malloc(ALIAS_NAME_SIZE);
	memset(trunkName,0,ALIAS_NAME_SIZE);

	if(strcmp((char*)trunk_name,"list")==0){
		free(trunkName);
		return -1;
	}
	ret = trunk_name_legal_check((char*)trunk_name,strlen(trunk_name));
	if(ALIAS_NAME_LEN_ERROR == ret) {
		free(trunkName);
		return -2;
	}
	else if(ALIAS_NAME_HEAD_ERROR == ret) {
		free(trunkName);
		return -3;
	}
	else if(ALIAS_NAME_BODY_ERROR == ret) {
		free(trunkName);
		return -4;
	}
	else {
		memcpy(trunkName,trunk_name,strlen(trunk_name));
		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
											NPD_DBUS_TRUNK_OBJPATH ,	\
											NPD_DBUS_TRUNK_INTERFACE ,	\
											NPD_DBUS_TRUNK_METHOD_CREATE_TRUNK_ONE );
		
		dbus_error_init(&err);
		dbus_message_append_args(query,
								 DBUS_TYPE_UINT16,&trunkId,
								 DBUS_TYPE_STRING,&trunkName,
								 DBUS_TYPE_INVALID);

		reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	}
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		free(trunkName);
		return 0;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) 
	{
	   	
		if(TRUNK_RETURN_CODE_TRUNK_EXISTS == op_ret) 
		{  
			//vty_out(vty,"%% Trunk %d already exists.\n",trunkId);
			retu=-6;
		}
		else if(TRUNK_RETURN_CODE_NAME_CONFLICT == op_ret)
		{
			//vty_out(vty,"%% Trunk name %s conflict.\n",trunkName);
			retu=-7;
		}
		else if(TRUNK_RETURN_CODE_ERR_HW == op_ret) 
		{
			//vty_out(vty,"%% Create trunk %d hardware fail.\n",trunkId);
			retu=-8;
		}
		else if(TRUNK_RETURN_CODE_ERR_GENERAL == op_ret) 
		{
			//vty_out(vty,"%% Create trunk %d general failure.\n",trunkId);
			retu=-8;
		}		
	} 
	else 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		retu = 0;
	}
	dbus_message_unref(reply);
	free(trunkName);
	return retu;
}

/*注意判断id合法性<1-127>*/
int delete_trunk_byid(int id)    /*返回0表示 删除失败，返回1表示删除成功，返回-1表示Trunk id Illegal，返回-2表示trunk not exists，返回-3表示出错*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;

	unsigned short 	trunkId =0;
	int retu = 1;
	unsigned int op_ret = 0;

    trunkId = (unsigned short) id;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_TRUNK_OBJPATH ,	\
										NPD_DBUS_TRUNK_INTERFACE ,	\
										NPD_DBUS_TRUNK_METHOD_DELETE_TRUNK_ENTRY );
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT16,&trunkId,
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
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) 
	{
	   
        if (ETHPORT_RETURN_CODE_NO_SUCH_TRUNK == op_ret) 
					{
		//	vty_out(vty,"% Bad Parameter,trunk %d Invalid.\n",trunkId);
			retu=-1;
		}
		if (TRUNK_RETURN_CODE_TRUNK_NOTEXISTS == op_ret) 
		{
			//vty_out(vty,"% Bad Parameter,trunk %d NOT exists.\n",trunkId);
			retu=-2;
		}
		else if (TRUNK_RETURN_CODE_BADPARAM == op_ret) 
		{
			//vty_out(vty,"%% Error occurs in Parse portNo or deviceNo.\n");
			retu=-3;
		}		
		else if (TRUNK_RETURN_CODE_ERR_HW == op_ret) 
		{
			//vty_out(vty,"%% Error occurs in Config on HW.\n");
			retu=-3;
		}
		else if(TRUNK_RETURN_CODE_ERR_NONE == op_ret) 
		{
			/*vty_out(vty,"query reply op %d ,Delete trunk %d OK.\n",op_ret,trunkId);*/
			retu=1;
		}

	} 
	else 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		retu=0;
	}
	dbus_message_unref(reply);
	
	return retu;
}


/*注意add端口的admin state必须为on*/
 /*flag="add"或"delete"，slot_port格式"1-1,1-2"，slot 范围1-4，port范围1-6，每个trunk最多add 8个port*/
                                                                  /*返回0表示失败，返回1表示成功，返回-1表示Unknow portno format.，返回-2表示port not exists*/
                                                                  /*返回-3表示trunk not exists，返回-4表示 port was Already the member of this trunk*/
                                                                  /*返回-5表示port was not member of this trunk，返回-6表示 trunk port member id FULL*/
                                                                  /*返回-7表示This port is a member of other trunk，返回-8表示 This port is L3 interface*/
int add_delete_trunk_port(int id,char *flag,char *slot_port)                                                                   /*返回-9表示This is the master port of trunk，返回-10表示error*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned char slot_no = 0,local_port_no = 0;
	unsigned int t_slotno = 0,t_portno = 0;
	boolean isAdd 		= FALSE;
	unsigned short	trunkId = 0;
	unsigned int 	op_ret = 0;
	int retu = 1;

    
	if(strncmp(flag,"add",strlen(flag))==0) {
		isAdd = TRUE;
	}	
	else if (strncmp(flag,"delete",strlen(flag))==0) {
		isAdd= FALSE;
	}

	op_ret = parse_slotno_localport((char *)slot_port,&t_slotno,&t_portno);
	
	slot_no = (unsigned char)t_slotno;
	local_port_no = (unsigned char)t_portno;
	
	if (NPD_FAIL == op_ret) {
		return -1;
	}

	trunkId = (unsigned short) id;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_TRUNK_OBJPATH,	\
										NPD_DBUS_TRUNK_INTERFACE,	\
										NPD_DBUS_TRUNK_METHOD_PORT_MEMBER_ADD_DEL);
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
							 	DBUS_TYPE_BYTE,&isAdd,
								DBUS_TYPE_BYTE,&slot_no,
								DBUS_TYPE_BYTE,&local_port_no,
							 	DBUS_TYPE_UINT16,&trunkId,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		retu = 0;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)) {			
			if (ETHPORT_RETURN_CODE_NO_SUCH_PORT == op_ret) 
			{
				//vty_out(vty,"%% Bad parameter,port %d/%d not exist.\n",slot_no,local_port_no);
				retu=-2;
			}
			else if (TRUNK_RETURN_CODE_BADPARAM == op_ret)
			{
				//vty_out(vty,"%% Error occurs in Parse portNo or deviceNo.\n");
				retu=-10;
			}
			else if (TRUNK_RETURN_CODE_TRUNK_NOTEXISTS == op_ret) 
			{
				//vty_out(vty,"%% Trunk to be configured not exists on SW.\n");
				retu=-3;
			}
			else if (TRUNK_RETURN_CODE_PORT_EXISTS == op_ret)
			{
				//vty_out(vty,"%% Port was already the member of this trunk.\n ");
				retu=-4;
			}
			else if (TRUNK_RETURN_CODE_PORT_NOTEXISTS == op_ret)
			{
				//vty_out(vty,"%% Port was not member of this trunk ever.\n");
				retu=-5;
			}
			else if (TRUNK_RETURN_CODE_PORT_MBRS_FULL == op_ret) 
			{
				//vty_out(vty,"%% Trunk port member full on hardware.\n"); 
				retu=-6;
			}
			else if (TRUNK_RETURN_CODE_ERR_HW == op_ret) 
			{
				//vty_out(vty,"%% Config trunk on hardware general failure.\n");	
				retu=-10;
			}
			else if (TRUNK_RETURN_CODE_ERR_GENERAL == op_ret) 
			{
				//vty_out(vty,"%% Error occurs in config on software.\n"); 
				retu=-10;
			}
			else if (TRUNK_RETURN_CODE_MEMBERSHIP_CONFICT == op_ret) 
			{
				//vty_out(vty,"%% Error,this port is a member of other trunk.\n");
				retu=-7;
			}
			else if (TRUNK_RETURN_CODE_PORT_CONFIG_DIFFER == op_ret) 
			{
				//vty_out(vty,"%% Error,this port has not the same configuration as master port.\n");
				retu = -9;
			}			
			else if (TRUNK_RETURN_CODE_PORT_L3_INTFG == op_ret) 
			{
				//vty_out(vty,"%% Error,this port is L3 interface.\n");
				retu=-8;
			}
			else if (TRUNK_RETURN_CODE_MEMBER_ADD_ERR == op_ret) 
			{
				//vty_out(vty,"%% Error on adding this port to default or allowed vlans.\n");
				retu=-10;
			}
			else if (TRUNK_RETURN_CODE_MEMBER_DEL_ERR == op_ret) 
			{
				//vty_out(vty,"%% Error on deleting this port from allowed vlan.\n");
				retu=-10;
			}
			else if (TRUNK_RETURN_CODE_SET_TRUNKID_ERR == op_ret)
			{
				//vty_out(vty,"%% Error on setting port trunkId.\n");
				retu=-10;
			}
			else if (TRUNK_RETURN_CODE_DEL_MASTER_PORT == op_ret)
			{
				//vty_out(vty,"%% Can not delete master port.\n");
				retu=-9;
			}	
			else if (TRUNK_RETURN_CODE_UNSUPPORT == op_ret) 
			{
				//vty_out(vty,"%% This operation is unsupported!\n");
				retu=-10;				
			}	
	}
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		retu=0;
	}
	dbus_message_unref(reply);
	return retu;
}

int set_master_port(int id,char *slot_port)  /*slot_port格式"1-1,1-2"，slot 范围1-4，port范围1-6*/
                                            	/*返回0表示失败，返回1表示成功，返回-1表示Unknow portno format.，返回-2表示port not exists*/
                                            	/*返回-3表示trunk not exists，返回-4表示 port was not member of this trunk*/
                                            	/*返回-5表示This port is L3 interface，返回-6表示 error*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned char slot_no = 0,local_port_no = 0;
	unsigned int t_slotno = 0,t_portno = 0;
	unsigned short	trunkId;	
	unsigned int op_ret = 0;
	int retu = 1;

	/*fetch the 1st param : slotNo/portNo*/
	if(4 < strlen(slot_port)){
		return -1;
	}

    op_ret = parse_slotno_localport((char *)slot_port,&t_slotno,&t_portno);
	
	slot_no = (unsigned char)t_slotno;
	local_port_no = (unsigned char)t_portno;
	
	if (NPD_FAIL == op_ret) {
		return -1;
	}

	trunkId = (unsigned short) id;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_TRUNK_OBJPATH,	\
										NPD_DBUS_TRUNK_INTERFACE,	\
										NPD_DBUS_TRUNK_METHOD_PORT_MEMBER_MASTERSHIP_CONFIG);

	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_BYTE,&slot_no,
							DBUS_TYPE_BYTE,&local_port_no,
						 	DBUS_TYPE_UINT16,&trunkId,
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
							DBUS_TYPE_UINT32,&op_ret,
							DBUS_TYPE_INVALID)) {			
			//////////new return 
			if (ETHPORT_RETURN_CODE_NO_SUCH_PORT == op_ret) 
			{
				//vty_out(vty,"% Bad parameter,bad slot/port number.\n");
				retu=-1;
			}
			else if (TRUNK_RETURN_CODE_ERR_NONE == op_ret ) 
			{
				/*vty_out(vty,"Ethernet Port %d-%d %s success.\n",slot_no,local_port_no,isAdd?"Add":"Delete");*/
				retu=1;
			}/*either slot or local port No err,return NPD_DBUS_ERROR_NO_SUCH_PORT */
			else if (TRUNK_RETURN_CODE_BADPARAM == op_ret) 
			{
				//vty_out(vty,"%% Error occurs in Parse portNo or deviceNo.\n");
				retu = 0;
			}
			else if (TRUNK_RETURN_CODE_TRUNK_NOTEXISTS == op_ret) 
			{
				//vty_out(vty,"%% Error,trunk to be configured not exist on software.\n");
				retu = 0;
			}
			else if (TRUNK_RETURN_CODE_PORT_NOTEXISTS == op_ret)
			{
				//vty_out(vty,"%% Error,port was not member of this trunk ever.\n");
				retu=-4;
			}
			else if (TRUNK_RETURN_CODE_ERR_HW == op_ret) 
			{
				//vty_out(vty,"%% Error occurs in config on hardware.\n");	
				retu=-6;
			}
			else if (TRUNK_RETURN_CODE_ERR_GENERAL == op_ret) 
			{				
				//vty_out(vty,"%% Error occurs in config on software.\n"); /*such as add port to trunkNode struct.*/
				retu=-6;
			}
			else if (TRUNK_RETURN_CODE_PORT_L3_INTFG == op_ret) 
			{				
				//vty_out(vty,"%% Error,this port is L3 interface.\n");
				retu=-5;
			}
			///////////
	}
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		retu=0;
	}
	dbus_message_unref(reply);
	return retu;
}

int allow_refuse_vlan(int id,char *flag,char *vlan_id,char *mod)  /*flag="allow"或"refuse"，vlan_id范围1-4094*，mod="tag"或"untag"*/
                                                                     /*返回0表示失败，返回1表示成功，返回-1表示Unknow vlan format.，返回-2表示vlan not exists*/
                                                                     /*返回-3表示vlan is L3 interface，返回-4表示 vlan Already allow in trunk*/
                                                                     /*返回-5表示vlan Already allow in other trunk，返回-6表示 There exists no member in trunk*/
                                                                     /*返回-7表示Vlan NOT allow in trunk，返回-8表示tagMode error in vlan，返回-9表示error*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	boolean isAllow 	= FALSE;	
	boolean isTag	= FALSE;	
	unsigned short	trunkId =0,vid = 0;//vid[VID_MAX_NUM] = {0};
	unsigned int 	count = 1,ret = 0,op_ret = 0;
	int retu = 1;

	if(strncmp(flag,"allow",strlen((char*)flag))==0) {
		isAllow = TRUE;
	}
	else if (strncmp(flag,"refuse",strlen((char*)flag))==0) {
		isAllow = FALSE;
	}

	ret = parse_vlan_no_trunk((char*)vlan_id,&vid);
	if(0 != ret ){
		return -1;
	}

    
	if(strncmp(mod,"tag",strlen((char*)mod))==0) {
		isTag= TRUE;
	}
	else if (strncmp(mod,"untag",strlen((char*)mod))==0) {
		isTag = FALSE;
	}
	
	trunkId = (unsigned short) id;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_TRUNK_OBJPATH,	\
										NPD_DBUS_TRUNK_INTERFACE,	\
										NPD_DBUS_TRUNK_METHOD_ALLOW_REFUSE_VLAN_LIST);

	dbus_error_init(&err);

	dbus_message_append_args(	query,
							 	DBUS_TYPE_BYTE,&isAllow,
								DBUS_TYPE_BYTE,&isTag,
							 	DBUS_TYPE_UINT32,&count,
								DBUS_TYPE_UINT16,&vid,
							 	DBUS_TYPE_UINT16,&trunkId,
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
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)) {		 
			if (TRUNK_RETURN_CODE_ERR_NONE == op_ret )
			{
				/*vty_out(vty,"trunk %d %s vlan %d success.\n",trunkId,isAllow?"Allow":"Refuse",vid);*/
				retu=1;
			}
			else if (VLAN_RETURN_CODE_VLAN_NOT_EXISTS == op_ret ) 
			{
				//vty_out(vty,"%% Bad Parameter,vlan not exist.\n");
				retu=-2;
			}
			else if (VLAN_RETURN_CODE_L3_INTF == op_ret ) 
			{
				//vty_out(vty,"%% Bad Parameter,vlan is L3 interface .\n");
				retu=-3;
			}
			else if (TRUNK_RETURN_CODE_BADPARAM == op_ret) 
			{
				//vty_out(vty,"%% Error occurs in parse portNo or deviceNO.\n");
				retu=-9;
			}
			else if (VLAN_RETURN_CODE_TRUNK_EXISTS== op_ret) 
			{
				//vty_out(vty,"%% Bad Parameter,vlan Already allow in trunk %d.\n",trunkId);
				retu=-4;
			}
			else if (VLAN_RETURN_CODE_TRUNK_CONFLICT == op_ret) 
			{
				//vty_out(vty,"%% Bad Parameter,trunk %d already untagged member of other active vlan.\n",trunkId);
				retu=-5;
			}
			else if (TRUNK_RETURN_CODE_ALLOW_ERR == op_ret) 
			{
				//vty_out(vty,"%% Error occurs in trunk port add to allowed vlans.\n");
				retu=-9;
			}
			else if (TRUNK_RETURN_CODE_REFUSE_ERR == op_ret) 
			{
				//vty_out(vty,"%% Error occurs in trunk port delete from refused vlans.\n");
				retu=-9;
			}			
			else if (VLAN_RETURN_CODE_PORT_L3_INTF == op_ret) 
			{
				/*never happen*/
				//vty_out(vty,"%% Bad Parameter,there exists L3 interface port.\n");
				retu=-9;
			}
			else if (TRUNK_RETURN_CODE_ERR_HW == op_ret) 
			{
				//vty_out(vty,"%% Error occurs in config on HW.\n");	
				retu=-9;
			}
			else if (TRUNK_RETURN_CODE_ERR_GENERAL == op_ret) 
			{
				//vty_out(vty,"%% Error occurs in config on SW.\n"); /*such as add port to trunkNode struct.*/
				retu=-9;
			}
			else if (TRUNK_RETURN_CODE_MEMBERSHIP_CONFICT == op_ret) 
			{
				//vty_out(vty,"%% Error,this port is a member of other trunk.\n"); /*such as add port to trunkNode struct.*/
				retu=-9;
			}
			else if (TRUNK_RETURN_CODE_NO_MEMBER == op_ret) 
			{
				//vty_out(vty,"%% Bad parameter,there exists no member in trunk %d.\n",trunkId); /*such as add port to trunkNode struct.*/
				retu=-6;
			}
			else if (TRUNK_RETURN_CODE_ALLOW_VLAN == op_ret) 
			{
				//vty_out(vty,"%% Bad parameter,vlan already allow in trunk %d.\n",trunkId); /*such as add port to trunkNode struct.*/
				retu=-4;
			}
			else if (TRUNK_RETURN_CODE_NOTALLOW_VLAN == op_ret) 
			{
				//vty_out(vty,"%% Bad parameter,vlan not allow in trunk %d.\n",trunkId); /*such as add port to trunkNode struct.*/
				retu=-7;
			}
			else if (TRUNK_RETURN_CODE_VLAN_TAGMODE_ERR == op_ret) 
			{
				//vty_out(vty,"%% Bad parameter,trunk %d tagMode error in vlan.\n",trunkId); 
				retu=-8;
			}
			//////////////////
	} 
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		retu=0;
	}
	dbus_message_unref(reply);
	return retu;
}

int set_port_state(int id,char *slot_port,char *state)  /*slot_port格式"1-1,1-2"，slot 范围1-4，port范围1-6，每个trunk最多add 8个port，state="enable"或"disable"*/
                                                         /*返回0表示失败，返回1表示成功，返回-1表示Unknow portno format.，返回-2表示port was not member of this trunk*/
                                                         /*返回-3表示port already enable，返回-4表示 port not enable，返回-5表示error*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned char en_dis = 0;
	unsigned char slot_no = 0, local_port_no = 0;
	unsigned int t_slotno = 0, t_portno = 0;
	unsigned short	trunkId = 0;
	unsigned int 	ret = 0, op_ret = 0;
	int retu;

	ret = parse_slotno_localport((char *)slot_port,&t_slotno,&t_portno);

	slot_no = (unsigned char)t_slotno;
	local_port_no = (unsigned char)t_portno;

	if(strncmp(state,"enable",strlen((char*)state))==0) {
		en_dis = TRUE;
	}
	else if (strncmp(state,"disable",strlen((char*)state))==0) {
		en_dis = FALSE;
	}
	trunkId = (unsigned short)id;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_TRUNK_OBJPATH,	\
										NPD_DBUS_TRUNK_INTERFACE,	\
										NPD_DBUS_TRUNK_METHOD_PORT_MEMBER_ENALBE_DISABLE);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_BYTE,&slot_no,
							DBUS_TYPE_BYTE,&local_port_no,
						 	DBUS_TYPE_UINT16,&trunkId,
							DBUS_TYPE_BYTE,&en_dis,
						 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		retu = 0;
	}

	if (dbus_message_get_args ( reply, &err,
							DBUS_TYPE_UINT32,&op_ret,
							DBUS_TYPE_INVALID)) {		
			if (ETHPORT_RETURN_CODE_NO_SUCH_PORT == op_ret) 
			{
				//vty_out(vty,"% Bad parameter,bad slot/port number.\n");
				retu=-1;
			}
			else if (TRUNK_RETURN_CODE_ERR_NONE == op_ret ) 
			{
				/*vty_out(vty,"Ethernet Port %d-%d %s success.\n",slot_no,local_port_no,isAdd?"Add":"Delete");*/
				retu = 1;
			}
			else if (TRUNK_RETURN_CODE_BADPARAM == op_ret) 
			{
				//vty_out(vty,"%% Error occurs in Parse portNo or deviceNo.\n");
				retu=-5;
			}
			else if (TRUNK_RETURN_CODE_TRUNK_NOTEXISTS == op_ret) 
			{
				//vty_out(vty,"%% Error,trunk not exist on software.\n");
				retu=-5;
			}
			else if (TRUNK_RETURN_CODE_PORT_NOTEXISTS == op_ret)
			{
				//vty_out(vty,"%% Error,port was not member of this trunk ever.\n");
				retu=-2;
			}
			else if (TRUNK_RETURN_CODE_PORT_ENABLE == op_ret) 
			{
				//vty_out(vty,"%% Error port already enable.\n");	
				retu=-3;
			}
			else if (TRUNK_RETURN_CODE_PORT_NOTENABLE == op_ret) 
			{
				//vty_out(vty,"%% Error port not enable.\n"); /*such as add port to trunkNode struct.*/
				retu=-4;
			}
			else if (TRUNK_RETURN_CODE_ERR_HW == op_ret) 
			{
				//vty_out(vty,"%% Error occurs on hardware.\n"); /*such as add port to trunkNode struct.*/
				retu=-5;
			}
			else if (TRUNK_RETURN_CODE_PORT_LINK_DOWN == op_ret) 
			{
				//vty_out(vty,"%% The port must be link first.\n");
				retu=-5;
			}
			//////////
	}
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		retu=0;
	}
	dbus_message_unref(reply);
	return retu;
}

int set_load_balance(char *mode)  /*mode列表:based-port|based-mac|based-ip|based-L4|mac+ip|mac+L4|ip+L4|mac+ip+L4*/
                                     /*返回0表示失败，返回1表示成功，返回-1表示there no trunk exist*/
                                     /*返回-2表示load-balance Mode same to corrent mode*/
                                     /*返回-3表示device isn't supported this mode set*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;

	unsigned int loadBalanMode = LOAD_BANLC_MAX;
	//kehao modify 2011-04-26
	///////////////////////////////////
	//unsigned int op_ret;
    unsigned int op_ret = 0;
	//////////////////////////////////
	int retu = -4;

	if(strncmp(mode,"based-port",strlen(mode))==0) {
		loadBalanMode = LOAD_BANLC_SOURCE_DEV_PORT;
	}
	else if (strncmp(mode,"based-mac",strlen(mode))==0) {
		loadBalanMode= LOAD_BANLC_SRC_DEST_MAC;
	}
	else if (strncmp(mode,"based-ip",strlen(mode))==0) {
		loadBalanMode= LOAD_BANLC_SRC_DEST_IP;
	}
	else if (strncmp(mode,"based-L4",strlen(mode))==0) {
		loadBalanMode= LOAD_BANLC_TCP_UDP_RC_DEST_PORT;
	}
	else if (strncmp(mode,"mac+ip",strlen(mode))==0) {
		loadBalanMode= LOAD_BANLC_MAC_IP;
	}
	else if (strncmp(mode,"mac+L4",strlen(mode))==0) {
		loadBalanMode= LOAD_BANLC_MAC_L4;
	}
	else if (strncmp(mode,"ip+L4",strlen(mode))==0) {
		loadBalanMode= LOAD_BANLC_IP_L4;
	}
	else if (strncmp(mode,"mac+ip+L4",strlen(mode))==0) {
		loadBalanMode= LOAD_BANLC_MAC_IP_L4;
	}
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_TRUNK_OBJPATH ,	\
										NPD_DBUS_TRUNK_INTERFACE ,	\
										NPD_DBUS_TRUNK_METHOD_CONFIG_LOAD_BANLC_MODE);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 //DBUS_TYPE_UINT16,&trunkId,
							 DBUS_TYPE_UINT32,&loadBalanMode,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return 0;
	}

	if (dbus_message_get_args( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) 
	{		  
        if(ETHPORT_RETURN_CODE_NO_SUCH_TRUNK == op_ret ||
			TRUNK_RETURN_CODE_TRUNK_NOTEXISTS == op_ret)
		{
		//	//vty_out(vty,"%% Bad parameter,there no trunk exist.");
			retu=-1;
		}
		else if(TRUNK_RETURN_CODE_ERR_NONE == op_ret)   /*0+3,log exist,then enter trunk_config_node CMD.*/
		{/*nothing to do*/
		  retu = 1;
		}
		else if(TRUNK_RETURN_CODE_LOAD_BANLC_CONFLIT == op_ret) {
			//vty_out(vty,"%% Bad parameter,trunk load-balance mode same to corrent mode.");
			retu=-2;
		}
		else if(TRUNK_RETURN_CODE_UNSUPPORT == op_ret) {
			//vty_out(vty,"%% The device isn't supported this mode set,it can only support base_mac and base_ip set.");
            //retu=0;
              retu = -3;
			//
		}

	} 
	else 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		retu=0;
	}
	dbus_message_unref(reply);
	return retu;
}


#ifdef __cplusplus
}
#endif

