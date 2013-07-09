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
* ws_dcli_vlan.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* tangsq@autelan.com
*
* DESCRIPTION:
* function for web
*
*
***************************************************************************/
/*dcli_vlan.h V1.16*/
/*dcli_vlan.c V1.94*/
/*author tangsiqi*/
/*update time 09-11-11*/


#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <dbus/dbus.h>
#include "ws_dcli_vlan.h"
#include <unistd.h>
#include "ws_returncode.h"
#include "ws_init_dbus.h"
#include "ws_dbus_list.h"
#include "sysdef/returncode.h"
#include <fcntl.h>
#include <sys/mman.h>  
#include <sys/stat.h>
#include "nm/app/manage/ac_manage_def.h"
#include "board/board_define.h"

/*statements*/
int get_vlan_member_slot_port
(
	struct vlan_info *vlan_info_byID,
    unsigned int product_id_param,
    PORT_MEMBER_BMP untagBmp_param,
    PORT_MEMBER_BMP tagBmp_param,
    unsigned int * promisPortBmp_param,
    unsigned int * untagPortNum,
    unsigned int * tagPortNum
);



int parse_trunk_no1(char* str,unsigned short* trunkId)
{
    char *endptr = NULL;
    char c;
    if (NULL == str) return NPD_FAIL;
    c = str[0];
    if (c>'0'&&c<='9'){
        *trunkId= strtoul(str,&endptr,10);
        if('\0' != endptr[0]){
            return NPD_FAIL;
    	}
        return NPD_SUCCESS;	
    }
    else {
        return NPD_FAIL; //not Vlan ID. for Example ,enter '.',and so on ,some special char.
    }
}


int parse_vlan_no(char* str,unsigned short* vlanId) {
    char *endptr =NULL;
    char c;
    if (NULL == str) return -1;
    c = str[0];
    if (c>'0'&&c<='9'){
        *vlanId= strtoul(str,&endptr,10);
        if('\0'== endptr[0]){
    		
           return 0;          
        }
        if(endptr[0]>'9'||endptr[0]<'0'){
           //printf("error.\n");
           return -1;
        }   
    }
    else {
        return -1; //not Vlan ID. for Example

    	}
    return 0; 
    }

int parse_single_param_no (char* str,unsigned short* sig_param) {
    char *endptr = NULL;

    if (NULL == str) return NPD_FAIL;
    *sig_param= strtoul(str,&endptr,10);
    return NPD_SUCCESS;	
}


int vlan_name_legal_check(char* str,unsigned int len)
    {	
    int i;  int ret = NPD_FAIL;	
    char c = 0;	
    if((NULL == str)||(len==0))
        {		
        return ret;	
    	}	
    if(len >= ALIAS_NAME_SIZE)
        {		
        ret = ALIAS_NAME_LEN_ERROR;		
        return ret;
    	}	
    c = str[0];
    if( (c=='_')||      (c<='z'&&c>='a')||      (c<='Z'&&c>='A')	  )
        {       ret =NPD_SUCCESS;	}	
    else {      return ALIAS_NAME_HEAD_ERROR;	}	
    for (i=1;i<=len-1;i++)
        {		
        c = str[i];		
        if( (c>='0' && c<='9')||			
            (c<='z'&&c>='a')||
            (c<='Z'&&c>='A')||
            (c=='_')
            )
            {           continue;		}		
        else {          ret =ALIAS_NAME_BODY_ERROR;			break;		}	
    	}	
    return ret;
    }


int param_first_char_check(char* str,unsigned int cmdtip)
    {	
    int ret = NPD_FAIL;
    char c = 0;
    if(NULL == str)
        {       return ret;	}	
    c = str[0]; //printf("param String %s,first char %c\n",str,c);	
    switch (cmdtip)
        {		
        case 0://add/delete     //printf("check add Or delete\n");		
        if('a' == c){ret = 1;}			
        else if('d' == c){ret = 0;}			
        else {ret = NPD_FAIL;}			
        break;		
        case 1://untag/tag      //printf("check tag Or untag\n");
        if(c =='t'){ret = 1;}			
        else if('u' == c){ret = 0;}			
        else {ret = NPD_FAIL;}		
        break;		
        default:			
            break;	
        	}	
    return ret;	
    }

/*return 1:succ;0:fail;other:error*/
inline int create_vlan_intf(unsigned short vid,unsigned int advanced)
{

    DBusMessage *query, *reply;
    DBusError err;
    unsigned int op_ret = INTERFACE_RETURN_CODE_ERROR,ifIndex = 0,vId = 0;
    char *pname = NULL;
	int retu = 0;


        query = dbus_message_new_method_call(
                            NPD_DBUS_BUSNAME,			
                            NPD_DBUS_INTF_OBJPATH,		
                            NPD_DBUS_INTF_INTERFACE,					
                            NPD_DBUS_INTF_METHOD_CREATE_VID_INTF);
    	
        dbus_error_init(&err);

        dbus_message_append_args(query,
                             DBUS_TYPE_UINT16,&vid,
                             DBUS_TYPE_UINT32,&advanced,
                             DBUS_TYPE_INVALID);

        reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);

        dbus_message_unref(query);
    	
        if (NULL == reply)
    	{
		    //ShowAlert(search(lpublic,"contact_adm"));
            if (dbus_error_is_set(&err)) 
    		{
                dbus_error_free(&err);
    		}
            dbus_message_unref(reply);
            return 0;
    	}

        if (dbus_message_get_args ( reply, &err,
        DBUS_TYPE_UINT32,&op_ret,
        DBUS_TYPE_UINT32, &ifIndex,
        DBUS_TYPE_STRING,&pname,
        DBUS_TYPE_INVALID)) 
    	{
            if (INTERFACE_RETURN_CODE_SUCCESS != op_ret)
        	{
				//vty_out(vty,dcli_error_info_intf(op_ret));
				//ret = CMD_WARNING;
				//dcli_error_info_intf(op_ret);
				if(0 == op_ret)
				{
					retu = 1;
				}
				else
				{
					retu = op_ret;
				}
        	}
    	}
        else
		{
            //ShowAlert(search(lcontrol,"Get_args_error"));
            if (dbus_error_is_set(&err))
    		{
                dbus_error_free(&err);
    		}
			return 0;
		}
        dbus_message_unref(reply);
        return retu;
}

inline int create_vlan_intf_advanced_routing(unsigned short vid,unsigned int enable)
{
	DBusMessage *query, *reply;
	DBusError err;
	int ret = 0;
    unsigned int op_ret = INTERFACE_RETURN_CODE_ERROR,ifIndex = 0,vId = 0;
	
	char *pname = NULL;


	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,			\
								NPD_DBUS_INTF_OBJPATH,			\
								NPD_DBUS_INTF_INTERFACE,					\
								NPD_DBUS_INTF_METHOD_VLAN_INTERFACE_ADVANCED_ROUTING_ENABLE);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT16,&vid,
							 DBUS_TYPE_UINT32,&enable,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		//vty_out(vty,"query reply null\n");
		//vty_out(vty,"failed get reply.\n");
		return -1;
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return -1;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_UINT32,&ifIndex,
		DBUS_TYPE_STRING,&pname,
		DBUS_TYPE_INVALID)) {
		if (INTERFACE_RETURN_CODE_SUCCESS != op_ret)
		{
			if (INTERFACE_RETURN_CODE_INTERFACE_NOTEXIST== op_ret)
			{
				//vty_out(vty,"%% Advanced-routing already disabled!\n");
				return -2;
			}
			else if (INTERFACE_RETURN_CODE_ALREADY_ADVANCED== op_ret)
			{
				//vty_out(vty,"%% Advanced-routing already enabled!\n");
				return -3;
			}
			else
			{
				//vty_out(vty,dcli_error_info_intf(op_ret));
				if(op_ret == 0)
				{
					ret = 0;
				}
				else
				{
					ret = op_ret;
				}
			}
		}
		dbus_message_unref(reply);
		return ret;
	}
	else{
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return ret;
}
int show_current_vlan_port_member(struct vlanlist_info_c *head)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	unsigned int vlan_Cont = 0;
	unsigned short  vlanId = 0;
	char*			vlanName = NULL;	
	unsigned int	slot =0, port =0, tmpVal =0;
	unsigned int 	product_id = PRODUCT_ID_NONE, count = 0, i = 0, j = 0,k = 0, ret = 0;
	unsigned int    promisPortBmp[2] = {0};
	unsigned int vlanStat = 0;
	PORT_MEMBER_BMP untagPortBmp, tagPortBmp;
	memset(&untagPortBmp,0,sizeof(PORT_MEMBER_BMP));
	memset(&tagPortBmp,0,sizeof(PORT_MEMBER_BMP));
	vlanName = (char*)malloc(ALIAS_NAME_SIZE);
	memset(vlanName,0,ALIAS_NAME_SIZE);
	int vflag = 0;
	
	int	count_tag =0;
	int	count_untag =0;		
	int count_bond =0;
	
	int fd = -1;
	struct stat sb;
	vlan_list_t * mem_vlan_list =NULL;
	char* file_path = "/dbm/shm/vlan/shm_vlan";
	
	int local_slot_id = get_product_info(LOCAL_SLOTID_FILE);
    int slotNum = get_product_info(SLOT_COUNT_FILE);
    if((local_slot_id<0)||(slotNum<0))
    {
    	//vty_out(vty,"get_product_info() return -1,Please check dbm file !\n");
		return -2;			
    }
		
	/* only read file */
    fd = open(file_path, O_RDONLY);
	if(fd < 0)
    {
        //vty_out(vty,"Failed to open! %s\n", strerror(errno));
        return -2;
    }
	fstat(fd,&sb);
	/* map not share */	
    mem_vlan_list = (vlan_list_t *)mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0 );
    if(MAP_FAILED == mem_vlan_list)
    {
        //vty_out(vty,"Failed to mmap for g_vlanlist[]! %s\n", strerror(errno));
		close(fd);
        return -2;
    }	
	vflag = 0;
	for (vlanId=1;vlanId<=4093;vlanId++)
    {
		head->mem_vlan_list[vlanId-1].vlanStat = mem_vlan_list[vlanId-1].vlanStat;
		head->mem_vlan_list[vlanId-1].vlanid = vlanId;
		if(mem_vlan_list[vlanId-1].vlanStat == 1)
		{
			vflag ++;
			//vty_out(vty,"%-4d(%s)  ",vlanId,mem_vlan_list[vlanId-1].updown ? "U":"D");			
			//vty_out(vty,"%-20s	",mem_vlan_list[vlanId-1].vlanName);
			
			//fprintf(stderr,"1111111           %-4d(%s)  \n",vlanId,mem_vlan_list[vlanId-1].updown ? "U":"D");			
			//fprintf(stderr,"2222222           %-20s	\n",mem_vlan_list[vlanId-1].vlanName);

			strncpy(head->mem_vlan_list[vlanId-1].updown,mem_vlan_list[vlanId-1].updown ? "U":"D",sizeof(head->mem_vlan_list[vlanId-1].updown)-1);
			strncpy(head->mem_vlan_list[vlanId-1].vlanName,mem_vlan_list[vlanId-1].vlanName,sizeof(head->mem_vlan_list[vlanId-1].vlanName)-1);

			count_tag =0;
			for(i = 0; i < 10; i++ )
			{
    			for(j = 0; j < 64; j++ )
    			{
					if(j<32)
					{
                        if((mem_vlan_list[vlanId-1].untagPortBmp[i].low_bmp)&(1<<j))
                    	{
    						slot = i+1;
    						port = j+1;
                			if((count_tag!=0) && (0 == count_tag % 4)) 
                			{
                				//vty_out(vty,"\n%-32s"," ");
                				//fprintf(stderr,"333333333   %-32s\n"," ");
								
                			}
                			//vty_out(vty,"%s%d/%d(u)",(count_tag%4) ? ",":"",slot,port);
                			//fprintf(stderr,"44444444  %s%d/%d(u)\n",(count_tag%4) ? ",":"",slot,port);
                			count_tag++;						
                    	}
    					else
    					{
    						continue;
    					}
					}
					else
					{
                        if((mem_vlan_list[vlanId-1].untagPortBmp[i].high_bmp)&(1<<(j-32)))
                    	{
    						slot = i+1;
    						port = j+1;
                			if((count_tag!=0) && (0 == count_tag % 4)) 
                			{
                				//fprintf(stderr,"555555 %-32s\n"," ");
                			}
                			//fprintf(stderr,"666666666   %s%d/%d(u)\n",(count_tag%4) ? ",":"",slot,port);
                			count_tag++;						
                    	}
    					else
    					{
    						continue;
    					}								
					}
    			}				
			}

			count_untag = 0;
			for(i = 0; i < 10; i++ )
			{
    			for(j = 0; j < 64; j++ )
    			{
					if(j<32)
					{
                        if((mem_vlan_list[vlanId-1].tagPortBmp[i].low_bmp)&(1<<j))
                    	{
    						slot = i+1;
    						port = j+1;
                            /* add '\n' between tag and untag list  */
            				if(count_tag != 0)
            				{
            					//fprintf(stderr,"777777         %-32s\n"," ");
            					count_tag = 0;     /* change count_temp back to 0 for next use */
            				}								
                			if((count_untag != 0)&&(0 == (count_untag % 4)))    /* tag port start at new line */
                			{
                				//fprintf(stderr,"888888    %-32s\n"," ");
                			}
                			//fprintf(stderr,"99999999999999        %s%d/%d(t)\n",(count_untag%4) ? ",":"",slot,port);
                			count_untag++;													
                    	}
    					else
    					{
    						continue;
    					}													
					}
					else
					{
                        if((mem_vlan_list[vlanId-1].tagPortBmp[i].high_bmp)&(1<<(j-32)))
                    	{
    						slot = i+1;
    						port = j+1;
                            /* add '\n' between tag and untag list  */
            				if(count_tag != 0)
            				{
            					//fprintf(stderr,"aaaaaaaaaa          %-32s\n"," ");
            					count_tag = 0;     /* change count_temp back to 0 */
            				}								
                			if((count_untag != 0)&&(0 == (count_untag % 4)))    /* tag port start at new line */
                			{
                				//fprintf(stderr,"bbbbbbbbbbbb        %-32s\n"," ");
                			}
                			//fprintf(stderr,"cccccccccc           %s%d/%d(t)\n",(count_untag%4) ? ",":"",slot,port);
                			count_untag++;													
                    	}
    					else
    					{
    						continue;
    					}									
					}
    			}				
			}
            /* print slot this vlan bonded to */
			count_bond = 0;
			for(k = 0; k < 10; k++ )
			{   
				if(mem_vlan_list[vlanId-1].bond_slot[k] != 0)
				{
					if(count_bond == 0)   /* print one times */
					{
        				//fprintf(stderr,"dddddddddddd     Bonded to slot : \n",vlanId);	
						head->mem_vlan_list[vlanId-1].bond_slot[k] = vlanId;
					}
					//fprintf(stderr,"eeeeeeeee            %-2d \n",k+1);
					count_bond++;
				}
			}
			if(count_bond != 0)
			{
			   // vty_out(vty,"\n");	
			}
			/* continue for next vlan list */
		}
	    else
		{
            /* do nothing */
            continue;
		}

	}
	head->vlannum = vflag;
    /*
	vty_out(vty,"sb.st_size: %d\n",sb.st_size );   
	vty_out(vty,"mem_vlan_list: 0x%x\n",mem_vlan_list);   
	vty_out(vty,"mem_vlan_list[0]: 0x%x\n",&mem_vlan_list[0]);   
	vty_out(vty,"mem_vlan_list[1]: 0x%x\n",&mem_vlan_list[1]);   
    */
	/* munmap and close fd */
    ret = munmap(mem_vlan_list,sb.st_size);
    if( ret != 0 )
    {
        //vty_out(vty,"Failed to munmap for g_vlanlist[]! %s\n", strerror(errno));			
    }	
	ret = close(fd);
	if( ret != 0 )
    {
        //vty_out(vty,"close shm_vlan failed \n" );   
    }
    return 0;
}

void Free_show_vlan_list_slot(struct vlan_info_detail *head)
{
	if(head == NULL)
	{
		return;
	}
	
    struct vlan_info_detail *f1 = NULL, *f2 = NULL;
	f1=head->next;	
	if(f1 == NULL)
	{
		return;
	}		
	f2=f1->next;
	while(f2!=NULL)
	{
	  free(f1);
	  f1=f2;
	  f2=f2->next;
	}
	free(f1);
}
/*返回1时调用Free_show_vlan_list_slot()释放空间*/
int show_vlan_list_slot(struct vlan_info_detail *head,int *vlannum)/*返回0表示失败，返回1表示成功*/
															   		 /*返回-1表示get_product_info() return -1,Please check dbm file*/
															   		 /*返回-2表示Failed to open*/
															   		 /*返回-3表示Failed to mmap for g_vlanlist[]*/
																	 /*返回-4表示malloc error*/
{
	if(NULL == head)
	{
		return 0;
	}
	
	unsigned short  vlanId = 0;
	unsigned int 	i = 0, j = 0,k = 0, ret = 0;
	int count = 0;

	int	count_tag =0;
	int	count_untag =0;		
	int count_bond =0;
	*vlannum = 0;

	struct vlan_info_detail *tail = NULL,*q = NULL;
	head->next=NULL;
	tail=head;
	int portnum = 0;

	
	int fd = -1;
	struct stat sb;
	vlan_list_t * mem_vlan_list =NULL;
	char* file_path = "/dbm/shm/vlan/shm_vlan";
	
	int local_slot_id = get_product_info(LOCAL_SLOTID_FILE);
    int slotNum = get_product_info(SLOT_COUNT_FILE);
	int slot_count = slotNum;
    if((local_slot_id<0)||(slotNum<0))
    {
    	//vty_out(vty,"get_product_info() return -1,Please check dbm file !\n");
		return -1;			
    }
	/* only read file */
    fd = open(file_path, O_RDONLY);
	if(fd < 0)
    {
       // vty_out(vty,"Failed to open! %s\n", strerror(errno));
        return -2;
    }
	fstat(fd,&sb);
	/* map not share */	
    mem_vlan_list = (vlan_list_t *)mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0 );
    if(MAP_FAILED == mem_vlan_list)
    {
        //vty_out(vty,"Failed to mmap for g_vlanlist[]! %s\n", strerror(errno));
		close(fd);
        return -3;
    }	
	//vty_out(vty,"Codes:  U - vlan link status is up,     D - vlan link status is down, \n");
	//vty_out(vty,"        u - untagged port member,       t - tagged port member, \n");
	//vty_out(vty,"        * - promiscuous mode port member\n");
	//vty_out(vty,"========================================================================\n");
	//vty_out(vty,"%-7s  %-20s  %-40s\n","VLAN ID","VLAN NAME","PORT MEMBER LIST");
	//vty_out(vty,"=======  ====================  =========================================\n");				
	for (vlanId=1;vlanId<=4093;vlanId++)
    {
    	portnum = 0;
	    if(mem_vlan_list[vlanId-1].vlanStat == 1)
		{
			q = (struct vlan_info_detail *)malloc(sizeof(struct vlan_info_detail)+1);
			if(NULL == q)
			{
				return -4;
			}
			memset(q,0,sizeof(struct vlan_info_detail)+1);
			q->vlanId = vlanId;
			q->vlanStat = mem_vlan_list[vlanId-1].vlanStat;
			count++;
			//vty_out(vty,"%-4d(%s)  ",vlanId,mem_vlan_list[vlanId-1].updown ? "U":"D");			
			//vty_out(vty,"%-20s	",mem_vlan_list[vlanId-1].vlanName);
			q->updown = mem_vlan_list[vlanId-1].updown;
			strcpy(q->vlanName,mem_vlan_list[vlanId-1].vlanName);

			count_tag =0;
			for(i = 0; i < slot_count; i++ )
			{
    			for(j = 0; j < 64; j++ )
    			{
					if(j<32)
					{
						q->untagPortBmp[i].low_bmp = mem_vlan_list[vlanId-1].untagPortBmp[i].low_bmp;
						if((mem_vlan_list[vlanId-1].untagPortBmp[i].low_bmp)&(1<<j))
                    	{
                			portnum++;
                			count_tag++;						
                    	}
    					else
    					{
    						continue;
    					}
					}
					else
					{
						q->untagPortBmp[i].high_bmp = mem_vlan_list[vlanId-1].untagPortBmp[i].high_bmp;
						if((mem_vlan_list[vlanId-1].untagPortBmp[i].high_bmp)&(1<<(j-32)))
                    	{
                			portnum++;
                			count_tag++;						
                    	}
    					else
    					{
    						continue;
    					}								
					}
    			}				
			}
			q->untagnum= count_tag;
			count_untag = 0;
			for(i = 0; i < slot_count; i++ )
			{
    			for(j = 0; j < 64; j++ )
    			{
					if(j<32)
					{
						q->tagPortBmp[i].low_bmp = mem_vlan_list[vlanId-1].tagPortBmp[i].low_bmp;
						if((mem_vlan_list[vlanId-1].tagPortBmp[i].low_bmp)&(1<<j))
                    	{
                			portnum++;
                			count_untag++;													
                    	}
    					else
    					{
    						continue;
    					}													
					}
					else
					{
						q->tagPortBmp[i].high_bmp = mem_vlan_list[vlanId-1].tagPortBmp[i].high_bmp;
                        if((mem_vlan_list[vlanId-1].tagPortBmp[i].high_bmp)&(1<<(j-32)))
                    	{
    						//slot = i+1;
    						//port = j+1;
                            /* add '\n' between tag and untag list  */
            				//if(count_tag != 0)
            				//{
            					//vty_out(vty,"\n%-32s"," ");
            					//count_tag = 0;     /* change count_temp back to 0 */
            				//}								
                			//if((count_untag != 0)&&(0 == (count_untag % 4)))    /* tag port start at new line */
                			//{
                				//vty_out(vty,"\n%-32s"," ");
                			//}
                			//vty_out(vty,"%s%d/%d(t)",(count_untag%4) ? ",":"",slot,port);
                			portnum++;
                			count_untag++;													
                    	}
    					else
    					{
    						continue;
    					}									
					}
    			}				
			}
			q->tagnum= count_untag;
            /* print slot this vlan bonded to */
			count_bond = 0;
			for(k = 0; k < slot_count; k++ )
			{   
				q->bond_slot[k] = mem_vlan_list[vlanId-1].bond_slot[k];
				if(mem_vlan_list[vlanId-1].bond_slot[k] != 0)
				{
					if(count_bond == 0)   /* print one times */
					{
        				//vty_out(vty,"Bonded to slot : ",vlanId);	
					}
					//vty_out(vty,"%-2d ",k+1);
					count_bond++;
				}
			}
			if(count_bond != 0)
			{
			   // vty_out(vty,"\n");	
			}
			/* continue for next vlan list */
			q->portnum = portnum;
			q->next = NULL;
			tail->next = q;
			tail = q;
		}
	    else
		{
            /* do nothing */
            continue;
		}
	}
	*vlannum = count;
	/* munmap and close fd */
    ret = munmap(mem_vlan_list,sb.st_size);
    if( ret != 0 )
    {
        //vty_out(vty,"Failed to munmap for g_vlanlist[]! %s\n", strerror(errno));			
    }	
	ret = close(fd);
	if( ret != 0 )
    {
       // vty_out(vty,"close shm_vlan failed \n" );   
    }
    return 1;

}

int show_vlan_list(struct vlan_info_simple  vlan_all[], int vlan_port_num[],int *vNum)
{
    DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	unsigned int vlan_Cont = 0;
	unsigned short  vlanId = 0;
	char*			vlanName = NULL;	
	unsigned int	slot =0, port =0, tmpVal =0;
	unsigned int 	product_id = PRODUCT_ID_NONE, count = 0, i = 0, j = 0, ret = 0;
	unsigned int    promisPortBmp[2] = {0};
	unsigned int vlanStat = 0;

    ///////////////   
    PORT_MEMBER_BMP untagPortBmp,tagPortBmp;
    memset(&untagPortBmp,0,sizeof(PORT_MEMBER_BMP));
    memset(&tagPortBmp,0,sizeof(PORT_MEMBER_BMP));
    int port_num=0;	

    vlanName = (char*)malloc(ALIAS_NAME_SIZE);
    memset(vlanName,0,ALIAS_NAME_SIZE);
    
    query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
                                    	NPD_DBUS_VLAN_OBJPATH ,	\
                                        NPD_DBUS_VLAN_INTERFACE ,	\
                                        NPD_DBUS_VLAN_METHOD_SHOW_VLANLIST_PORT_MEMBERS_V1 );
    

    dbus_error_init(&err);
    reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
    
    dbus_message_unref(query);
    if (NULL == reply)
	{
	    if (dbus_error_is_set(&err)) 
		{
	        dbus_error_free(&err);
		}
	    return CMD_FAILURE;
	}

    dbus_message_iter_init(reply,&iter);
    dbus_message_iter_get_basic(&iter,&ret);	
    
	if(VLAN_RETURN_CODE_ERR_NONE == ret){
        dbus_message_iter_next(&iter);			
        dbus_message_iter_get_basic(&iter,&vlan_Cont);		
        dbus_message_iter_next(&iter);
        dbus_message_iter_get_basic(&iter,&product_id);
    	
        dbus_message_iter_next(&iter);	
        dbus_message_iter_get_basic(&iter,&promisPortBmp[0]);
        dbus_message_iter_next(&iter);	
        dbus_message_iter_get_basic(&iter,&promisPortBmp[1]);

    
        *vNum=vlan_Cont;
        dbus_message_iter_next(&iter);	
        dbus_message_iter_recurse(&iter,&iter_array);
        if((vlan_Cont>0)&&(vlan_Cont<4096)){
        for (j = 0; j < vlan_Cont; j++)
    		{
                port_num=0;
				vlanStat = 0;
                memset(&untagPortBmp,0,sizeof(untagPortBmp));
                memset(&tagPortBmp,0,sizeof(tagPortBmp));

                DBusMessageIter iter_struct;	
                dbus_message_iter_recurse(&iter_array,&iter_struct);			

                dbus_message_iter_get_basic(&iter_struct,&vlanId);		
    
                dbus_message_iter_next(&iter_struct);			
                dbus_message_iter_get_basic(&iter_struct,&vlanName);		
    
                dbus_message_iter_next(&iter_struct);
                dbus_message_iter_get_basic(&iter_struct,&untagPortBmp.portMbr[0]);

                dbus_message_iter_next(&iter_struct);
                dbus_message_iter_get_basic(&iter_struct,&untagPortBmp.portMbr[1]);

                dbus_message_iter_next(&iter_struct);
                dbus_message_iter_get_basic(&iter_struct,&tagPortBmp.portMbr[0]);
    			
                dbus_message_iter_next(&iter_struct);
                dbus_message_iter_get_basic(&iter_struct,&tagPortBmp.portMbr[1]);  

    			
                dbus_message_iter_next(&iter_struct);
                dbus_message_iter_get_basic(&iter_struct,&vlanStat);

                dbus_message_iter_next(&iter_array);

                
	            //port_num = show_vlan_member_slot_port(product_id,untagPortBmp,tagPortBmp,promisPortBmp);
        				
	            
				////////////

                if(NPD_PORT_L3INTF_VLAN_ID != vlanId)
				{
					vlan_all[j].vlanId=vlanId;
					vlan_all[j].vlanStat=vlanStat;
					
					vlan_all[j].vlanName = (char *)malloc(ALIAS_NAME_SIZE);
					memset(vlan_all[j].vlanName, 0, ALIAS_NAME_SIZE);
					strcpy(vlan_all[j].vlanName,vlanName);

					if(0 != untagPortBmp.portMbr[0] || 0 != tagPortBmp.portMbr[0] || 0 != untagPortBmp.portMbr[1] || 0 != tagPortBmp.portMbr[1])
					{
						port_num=show_vlan_member_slot_port(product_id,untagPortBmp,tagPortBmp,promisPortBmp);
					}
					else 
					{						
						//vty_out(vty,"  %-40s","No Port member.");
						ret=10;						
					}
					vlan_port_num[j]=port_num;
				}                
				////////////
        	}		
    	}
    }
	else if(VLAN_RETURN_CODE_VLAN_NOT_EXISTS == ret) {
		//vty_out(vty,"%% Error,no vaild vlan exist.\n");
		return COMMON_FAIL;
	}
	else if (VLAN_RETURN_CODE_ERR_GENERAL == ret) {
		//vty_out(vty,"%% Error: operation on software fail.\n");	
		return COMMON_FAIL;
	}	
    else 
	{
		return COMMON_FAIL;
	}


        dbus_message_unref(reply);	


        return COMMON_SUCCESS;

}

/*return 0:fail; reutrn 1:succ; others*/
int create_vlan(DBusConnection *connection,unsigned short vID,char *vName) /*vid  (2-4094)*/
{

	if(NULL==connection)
		return 0;

		DBusMessage *query, *reply;
        DBusError err;

        unsigned short  vlanId;	
        char *vlanName = NULL;	
        unsigned int nameSize = 0;	
        int ret;	
        unsigned int op_ret;

        vlanName = (char*)malloc(ALIAS_NAME_SIZE);	
        memset(vlanName,0,ALIAS_NAME_SIZE);

        if(vID<1 || vID>4094)
		{
                //ShowAlert(search(lcontrol,"illegal_vID"));
                free(vlanName);
                return -1;
		}
    	
        if(strcmp((char*)vName,"list")==0){
            //ShowAlert(search(lcontrol,"change_vlanName"));
            //vty_out(vty,"Please enter another name for this vlan.");
            free(vlanName);
            return -2;
    	}
        char IDtemp[4] = {0};
        sprintf(IDtemp,"%d",(int)vID);   /*int转成char*/
        ret = parse_vlan_no(IDtemp,&vlanId);
        if (NPD_FAIL == ret) 
		{

            //ShowAlert(search(lcontrol,"illegal_vID"));
            return -2;
		}
	        ret = vlan_name_legal_check(vName,strlen(vName));
	        if(ALIAS_NAME_LEN_ERROR == ret)
	        	{		
	            //ShowAlert(search(lcontrol,"name_long"));			
	            free(vlanName);
	            return -3;
	    		}
	        else if(ALIAS_NAME_HEAD_ERROR == ret)
	        	{		
	            //ShowAlert(search(lcontrol,"name_begin_illegal"));			
	            free(vlanName);
	            return -4;
	    		}
	        else if(ALIAS_NAME_BODY_ERROR == ret)
	        	{		
	            //ShowAlert(search(lcontrol,"name_body_illegal"));
	            free(vlanName);
	            return -5;	
	    		}
	        else{
	            nameSize = strlen(vName);	
	            memcpy(vlanName,vName,nameSize);
	            query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,			
	                    NPD_DBUS_VLAN_OBJPATH ,				
	                    NPD_DBUS_VLAN_INTERFACE ,					
	                    NPD_DBUS_VLAN_METHOD_CREATE_VLAN_ONE );

	                dbus_error_init(&err);	
	                dbus_message_append_args(query,		
	                    DBUS_TYPE_UINT16,&vlanId,		
	                    DBUS_TYPE_STRING,&vlanName,	
	                    DBUS_TYPE_INVALID);
	    			
	                reply = dbus_connection_send_with_reply_and_block (connection,query,-1, &err);	
	    		}
	                dbus_message_unref(query);
	    			
	                if (NULL == reply) 
	        			{	
	                        //ShowAlert(search(lpublic,"contact_adm"));
	                    if (dbus_error_is_set(&err))
	            			{			
	                            dbus_error_free(&err);		
	        				}
							free(vlanName);
	                        return 0;	
	        			}

	                if (dbus_message_get_args ( reply, &err,	
	                    DBUS_TYPE_UINT32, &op_ret,		
	                    DBUS_TYPE_INVALID)) 
	        			{
	        			    ////////////
						if(VLAN_RETURN_CODE_ERR_NONE == op_ret) 
						{
							/*vty_out(vty,"% Create vlan success,query reply op_ret %d .\n",op_ret);*/
							strcpy(vName,vlanName);
	                        //ShowAlert(search(lcontrol,"create_vlan_success"));
	                        free(vlanName);
	                        return 1;
						}
						else if (ETHPORT_RETURN_CODE_NO_SUCH_VLAN == op_ret) 
						{
							//vty_out(vty,"%% ERROR:Illegal Vlan ID %d.\n",vlanId);
							//ShowAlert(search(lcontrol,"illegal_vID"));
							free(vlanName);
							return -2;
						}
						else if(VLAN_RETURN_CODE_VLAN_EXISTS == op_ret)
						{
							//vty_out(vty,"% Vlan %d Already Exists.\n",vlanId);
							//ShowAlert(search(lcontrol,"vID_Exist"));
							free(vlanName);
							return -6;
						}
						else if(VLAN_RETURN_CODE_NAME_CONFLICT == op_ret) 
						{
							//vty_out(vty,"% Vlan name %s conflict.\n",vlanName);
							//ShowAlert(search(lcontrol,"vname_conflict"));
							free(vlanName);
							return -7;
						}
						else if(VLAN_RETURN_CODE_ERR_HW == op_ret)
						{
							//vty_out(vty,"% Error occurs in Config vlan %d on HW.\n",vlanId);
							//ShowAlert(search(lcontrol,"HW_error"));
							free(vlanName);
							return -8;
						}
						else if(VLAN_RETURN_CODE_ERR_GENERAL == op_ret) 
						{
							//vty_out(vty,"& Error occurs in config vlan %d on SW.\n",vlanId);
							//ShowAlert(search(lcontrol,"SW_error"));
							free(vlanName);
							return -9;
						}
						#if 0 
			    		else if(VLAN_RETURN_CODE_VLAN_CREATE_NOT_ALLOWED == op_ret) {
			    			//vty_out(vty,"% Vlan %d could not be created.\n",vlanId);
			    			return -1
			    		}
			    		else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret){
			    			//vty_out(vty,"%% Product not support this function!\n");
			    			return -1;
			    		}
						#endif
							////////////        			  
	        			}
	                else 
					{
						//ShowAlert(search(lcontrol,"Get_args_error"));
			    		//vty_out(vty,"% Failed get args.\n");
			    		if (dbus_error_is_set(&err)) 
			    		{
			    			//vty_out(vty,"% %s raised: %s",err.name,err.message);
			    			dbus_error_free(&err);
			    		}
					}
	                dbus_message_unref(reply);
        free(vlanName);
        return COMMON_SUCCESS;
}

/*return 0: fail; reutrn 1:succ; other*/
int delete_vlan(DBusConnection *connection,unsigned short vID)
{

	if(NULL==connection)
		return 0;

	DBusMessage *query, *reply;
    DBusError err;	
    
    int ret;
    unsigned int op_ret;
    unsigned short  vlanId =0;

    if(vID==4095)
    {
       // ShowAlert(search(lcontrol,"illegal_vID"));
        return -2;
    }
	char IDtemp[4] = {0};
	sprintf(IDtemp,"%d",(int)vID);   /*int转成char*/
	ret = parse_vlan_no(IDtemp,&vlanId);

	if (NPD_FAIL == ret)
	{ 
		// ShowAlert(search(lcontrol,"illegal_vID"));
		return -2;
	}
	else 
	{
	    query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	
	    NPD_DBUS_VLAN_OBJPATH ,				
	    NPD_DBUS_VLAN_INTERFACE ,				
	    NPD_DBUS_VLAN_METHOD_CONFIG_DELETE_VLAN_ENTRY );	

	    dbus_error_init(&err);	

	    dbus_message_append_args(query,								
	    DBUS_TYPE_UINT16,&vlanId,	
	    DBUS_TYPE_INVALID);     /*printf("build query for vlanId %d.\n",vlanId);*/			
	    reply = dbus_connection_send_with_reply_and_block (connection,query,-1, &err);	
		}
    
        dbus_message_unref(query);

        if (NULL == reply) 
    		{
                //ShowAlert(search(lpublic,"contact_adm"));
            if (dbus_error_is_set(&err))
        		{			
                dbus_error_free(&err);		
        		}
            return 0;	
    		}

            if (dbus_message_get_args ( reply, &err,
                DBUS_TYPE_UINT32, &op_ret,	
                DBUS_TYPE_INVALID))
				{

					if (ETHPORT_RETURN_CODE_NO_SUCH_VLAN == op_ret) 
					{
						//vty_out(vty,"%% Bad Parameter:vlan id illegal.\n");
						//ShowAlert(search(lcontrol,"illegal_vID"));
						return -2;

					}
					else if (VLAN_RETURN_CODE_VLAN_NOT_EXISTS == op_ret) 
					{
						//vty_out(vty,"%% Bad Parameter:vlan %d NOT exists.\n",vlanId);
						//ShowAlert(search(lcontrol,"vID_NotExist"));
						return -4;

					}
					else if (VLAN_RETURN_CODE_BADPARAM == op_ret)
					{
						//vty_out(vty,"%% Bad Parameter:Can NOT delete Default vlan.\n");
						//ShowAlert(search(lcontrol,"Default_delete_error"));
						return -5;

					}
					else if (VLAN_RETURN_CODE_ERR_HW == op_ret) 
					{
						//vty_out(vty,"%% Error occurs in config on hardware.\n");
						//ShowAlert(search(lcontrol,"HW_error"));
						return -6;

					}
					else if (VLAN_RETURN_CODE_L3_INTF == op_ret) 
					{
						//vty_out(vty,"%% Bad Parameter:Can NOT delete Layer3 Interface vlan.\n");
						//ShowAlert(search(lcontrol,"opt_fail"));
						return -7;

					}
					else if (VLAN_RETURN_CODE_SUBINTF_EXISTS == op_ret)
					{
						#if 0  /* Advanced-routing not use in distributed system */
	                    vty_out(vty,"%% Bad Parameter:Can NOT delete Advanced-routing Interface vlan.\n");
						#endif
						/* vlan is bonded, can not delete */
	                    //vty_out(vty,"slot%d: vlan%d has been bonded, Please unbond it first !\n",slot_id,vlanId);
						return -9;

					}
					else if (VLAN_RETURN_CODE_PORT_SUBINTF_EXISTS == op_ret)
					{
						//vty_out(vty,"%% Can NOT delete vlan with port sub interface.\n");
						//ShowAlert(search(lcontrol,"opt_fail"));
						return -1;

					}
					else if(VLAN_RETURN_CODE_ERR_NONE == op_ret) 
					{
						/*vty_out(vty,"Delete vlan OK.\n");*/
						//ShowAlert(search(lcontrol,"delete_vlan_success"));
						return 1;

					}
					else
					{
						//vty_out(vty,"%% Unknown error occured when delete vlan.\n");
						//ShowAlert(search(lcontrol,"s_arp_unknow_err"));
						return -8;
					}

					//////////////
				}
            else{;}
    		
        dbus_message_unref(reply);
        return 0;
}
/* update the gvlanlist[] on 2 MCBs */
int ccgi_vlanlist_add_del_port
(
DBusConnection *connection,
char * addordel,
char * slot_port_no,
char * Tagornot,
unsigned short vID
)
{
	if(NULL==connection)
		return 0;
	
	DBusError err;
	unsigned int 	op_ret = 0;
	DBusMessage *query = NULL, *reply = NULL;

	int i = 0,ret=0;	
	//////////////////////

    unsigned char slot_no = 0,local_port_no = 0;
    unsigned int t_slotno = 0,t_portno = 0;
	int retu = 0;
    unsigned short  vlanId=0;	
    unsigned char isAdd     = 0;
    unsigned char isTagged  =0;
    if(0 == strncmp(addordel,"add",strlen(addordel)))
	{
        isAdd = 1;
    }
    else if (0 == strncmp(addordel,"delete",strlen(addordel))) 
	{
        isAdd= 0;
    }
    else 
	{    	
        return 0;
	}    

    /*fetch the 2nd param : slotNo/portNo*/
    op_ret = parse_slotno_localport(slot_port_no,&t_slotno,&t_portno);

    if (NPD_FAIL == op_ret)
    {    	
       // ShowAlert(search(lcontrol,"unknown_portno_format"));
        return -2;
	}
    else if (1 == op_ret){
        //ShowAlert(search(lcontrol,"error_slotno_format"));
        return -3;
    }
    slot_no = (unsigned char)t_slotno;
    local_port_no = (unsigned char)t_portno;
    /*not support config port on slot0*/
    if(0  == slot_no)
       {		
        //ShowAlert(search(lcontrol,"error_slotno_format"));
        return -3;
    }

    ret  = param_first_char_check((char *)Tagornot,1);
    if(NPD_FAIL == ret) 
       {		
        //ShowAlert(search(lcontrol,"error_tag_param"));
        return -4;
    }
    else if(1 == ret)
    {
	    isTagged = 1;	
	}	
    else if(0 == ret)
    {   
	    isTagged = 0;	
	}
    vlanId = vID;
	//////////////////////


	query = NULL;
	reply = NULL;
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_VLAN_OBJPATH,	\
										NPD_DBUS_VLAN_INTERFACE,	\
										NPD_DBUS_VLAN_METHOD_CONFIG_VLANLIST_PORT_MEMBER_ADD_DEL);
    	
    	dbus_error_init(&err);

    	dbus_message_append_args(	query,
    							 	DBUS_TYPE_BYTE,&isAdd,
    								DBUS_TYPE_BYTE,&slot_no,
    								DBUS_TYPE_BYTE,&local_port_no,
    							 	DBUS_TYPE_BYTE,&isTagged,
    							 	DBUS_TYPE_UINT16,&vlanId,
    							 	DBUS_TYPE_INVALID);
    	
        reply = dbus_connection_send_with_reply_and_block (connection,query,-1, &err);
    	
    	dbus_message_unref(query);
    	if (NULL == reply) {
    		//vty_out(vty,"Please check npd on MCB slot %d\n",slot_id);
    		return -3;
    	}

    	if (dbus_message_get_args ( reply, &err,
    		DBUS_TYPE_UINT32,&op_ret,
    		DBUS_TYPE_INVALID)) {
    		if (VLAN_RETURN_CODE_ERR_NONE != op_ret){
    			//vty_out(vty,"%% vlan_list add/delete port Error! return %d \n",op_ret);
    			return -5;
    		}
    	} 
    	else {
    		//vty_out(vty,"Failed get args,Please check npd on MCB slot %d\n",slot_id);
    		return -6;
    	}
    	dbus_message_unref(reply);
    	if(VLAN_RETURN_CODE_ERR_NONE == op_ret)
    	{
    		return 1;
    	}
	return 0;	
}

static int ccgi_master_slot_id_get(int *master_slot_id)
{
    FILE *fd;
	char buff[8][8];
	int i;
	fd = fopen("/dbm/product/master_slot_id", "r");
	if (fd == NULL)
	{
		return -1;
	}
	
	for(i = 0; i < 2; i++)
	{
		if(!fgets(buff[i], 8, fd))
			printf("read error no value\n");
		master_slot_id[i] = strtoul(buff[i], NULL, 10);
	}
	
	fclose(fd);
    
	return 0;	
}

int addordelete_port(DBusConnection *connection,char * addordel,char * slot_port_no,char * Tagornot,unsigned short vID,char * lan)
																/*返回0表示失败，返回1表示成功*/
																/* 返回-1表示operation failed，返回-2表示Unknown portno format*/
																/*返回-3表示Bad slot/port number，返回-4表示Bad tag parameter*/
																/*返回-5表示Error occurs in Parse eth-port Index or devNO.& logicPortNo*/
																/*返回-6表示Port already member of the vlan*/
																/*返回-7表示Port is not member of the vlan*/
																/*返回-8表示Port already untagged member of other active vlan*/
																/*返回-9表示Port Tag-Mode can NOT match*/
																/*返回-10表示Occcurs error,port is member of a active trunk*/
																/*返回-11表示Occcurs error,port has attribute of static arp*/	
																/*返回-12表示Occcurs error,port is member of a pvlan*/
																/*返回-13表示There are protocol-vlan config on this vlan and port*/
																/*返回-14表示Can't del an advanced-routing interface port from this vlan*/
																/*返回-15表示get master_slot_id error，返回-16表示get get_product_info return -1*/
																/*返回-17表示Please check npd on MCB slot，返回-18表示vlan_list add/delete port Error*/
{

	if((NULL==connection)||(NULL==addordel)||(NULL==slot_port_no)||(NULL==Tagornot))
		return 0;


	DBusMessage *query = NULL, *reply = NULL;
    DBusError err;
    
    unsigned char slot_no = 0,local_port_no = 0;
    unsigned int t_slotno = 0,t_portno = 0;
	int retu = 0;
    
    int     ret 		= 0;
    unsigned short  vlanId=0;	
    unsigned int    op_ret=0;	
    unsigned char isAdd     = 0;
    unsigned char isTagged  =0;
    char  message[100];
    memset(message,0,100);

    if(0 == strncmp(addordel,"add",strlen(addordel))) {
        isAdd = 1;
    }
    else if (0 == strncmp(addordel,"delete",strlen(addordel))) {
        isAdd= 0;
    }
    else {
    	
        return 0;
    }
    

    /*fetch the 2nd param : slotNo/portNo*/

    op_ret = parse_slotno_localport(slot_port_no,&t_slotno,&t_portno);


    if (NPD_FAIL == op_ret)
    {    	
       // ShowAlert(search(lcontrol,"unknown_portno_format"));
        return -2;
	}
    else if (1 == op_ret){
        //ShowAlert(search(lcontrol,"error_slotno_format"));
        return -3;
    }
    slot_no = (unsigned char)t_slotno;
    local_port_no = (unsigned char)t_portno;
    /*not support config port on slot0*/
    if(0  == slot_no)
        {		
        //ShowAlert(search(lcontrol,"error_slotno_format"));
        return -3;
    	}

    ret  = param_first_char_check((char *)Tagornot,1);
    if(NPD_FAIL == ret) 
        {		
        //ShowAlert(search(lcontrol,"error_tag_param"));
        return -4;
    	}
    else if(1 == ret)
        {       isTagged = 1;	}	
    else if(0 == ret)
        {       isTagged = 0;	}

    //fprintf(stderr,"&&%d-%d-%s-%s&&",isAdd,isTagged,&slot_no,&local_port_no);
    vlanId = vID;

    query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	
        NPD_DBUS_VLAN_OBJPATH,					
        NPD_DBUS_VLAN_INTERFACE,						
        NPD_DBUS_VLAN_METHOD_CONFIG_PORT_MEMBER_ADD_DEL);
    
    dbus_error_init(&err);	

    dbus_message_append_args(   query,			
        DBUS_TYPE_BYTE,&isAdd,					
        DBUS_TYPE_BYTE,&slot_no,	
        DBUS_TYPE_BYTE,&local_port_no,			
        DBUS_TYPE_BYTE,&isTagged,						
        DBUS_TYPE_UINT16,&vlanId,   						 
        DBUS_TYPE_INVALID);

    reply = dbus_connection_send_with_reply_and_block (connection,query,-1, &err);	
    
    dbus_message_unref(query);
    if (NULL == reply)
    	{	
        //ShowAlert(search(lpublic,"contact_adm"));
        if (dbus_error_is_set(&err)) 
        	{			
            dbus_error_free(&err);	
    		}
            return 0;	
    	}

    if (dbus_message_get_args ( reply,
        &err,	
        DBUS_TYPE_UINT32,&op_ret,		
        DBUS_TYPE_INVALID)) 
    	{
    	    /////////////////////////////
			if (ETHPORT_RETURN_CODE_NO_SUCH_PORT == op_ret) 
			{
				//vty_out(vty,"%% Bad parameter: Bad slot/port number.\n");
				//ShowAlert(search(lcontrol,"error_slotno_format"));
				retu = -3;
			}/*either slot or local port No err,return NPD_DBUS_ERROR_NO_SUCH_PORT */
			else if (VLAN_RETURN_CODE_ERR_NONE == op_ret )
			{
				/*vty_out(vty,"Ethernet Port %d-%d %s success.\n",slot_no,local_port_no,isAdd?"Add":"Delete");*/
				if(isAdd==1)
				{
					//sprintf(message,"%s%s%s%s",search(lcontrol,"add"),Tagornot,search(lcontrol,"_port"),slot_port_no);
					//ShowAlert(message);
					//return 1;
				}
				else if(isAdd ==0)
				{
					//sprintf(message,"%s%s%s%s",search(lcontrol,"del"),Tagornot,search(lcontrol,"_port"),slot_port_no);
					//ShowAlert(message);
					//return 1;
				}
				retu = 1;

			} 
			else if (VLAN_RETURN_CODE_BADPARAM == op_ret) 
			{
				//vty_out(vty,"%% Error occurs in Parse eth-port Index or devNO.& logicPortNo.\n");
				//ShowAlert(search(lcontrol,"parse_index_error"));
				retu = -5;
			}
			else if (VLAN_RETURN_CODE_VLAN_NOT_EXISTS == op_ret)
			{
				//vty_out(vty,"%% Error: Checkout-vlan to be configured NOT exists on SW.\n");
				//return COMMON_FAIL;
				//ShowAlert(search(lcontrol,"opt_fail"));
				retu = -1;
			}
			else if (VLAN_RETURN_CODE_PORT_EXISTS == op_ret)
			{
				//vty_out(vty,"%% Bad parameter: port Already member of the vlan.\n");
				//ShowAlert(search(lcontrol,"port_AlreadyExist"));
				retu = -6;
			}
			else if (VLAN_RETURN_CODE_PORT_NOTEXISTS == op_ret)
			{
				//vty_out(vty,"%% Bad parameter: port NOT member of the vlan.\n");
				//ShowAlert(search(lcontrol,"port_NotExist"));
				retu = -7;
			}
			else if (VLAN_RETURN_CODE_PORT_MBRSHIP_CONFLICT == op_ret) 
			{
				//vty_out(vty,"%% Bad parameter: port Already untagged member of other active vlan.\n");
				//ShowAlert(search(lcontrol,"tag_only"));		
				retu = -8;
				
			}			
			else if (VLAN_RETURN_CODE_PORT_PROMIS_PORT_CANNOT_ADD2_VLAN == op_ret) 
			{
				//vty_out(vty,"%% Promiscous port %d/%d can't be added to any vlan!\n",slotno,portno);
				//return COMMON_FAIL;
				//ShowAlert(search(lcontrol,"opt_fail"));
				retu = -1;
			}
			else if (VLAN_RETURN_CODE_PORT_PROMISCUOUS_MODE_ADD2_L3INTF == op_ret)
			{
				//vty_out(vty,"%% Bad parameter: promiscuous mode port can't add to l3 interface!\n");
				//return COMMON_FAIL;
				//ShowAlert(search(lcontrol,"opt_fail"));
				retu = -1;
			}
			else if (VLAN_RETURN_CODE_PORT_TAG_CONFLICT == op_ret) 
			{
				//vty_out(vty,"%% Bad parameter: port Tag-Mode can NOT match!\n");
				//ShowAlert(search(lcontrol,"tagmode_not_match"));
				retu = -9;
			}
			else if (VLAN_RETURN_CODE_PORT_SUBINTF_EXISTS == op_ret)
			{
				//vty_out(vty,"%% Can't delete tag port with sub interface!\n");
				//return COMMON_FAIL;
				//ShowAlert(search(lcontrol,"opt_fail"));
				retu = -1;
			}
			else if (VLAN_RETURN_CODE_PORT_DEL_PROMIS_PORT_TO_DFLT_VLAN_INTF == op_ret)
			{
				//vty_out(vty,"%% Promiscuous mode port can't delete!\n");
				//return COMMON_FAIL;
				//ShowAlert(search(lcontrol,"opt_fail"));
				retu = -1;
			}
			else if (VLAN_RETURN_CODE_PORT_TRUNK_MBR == op_ret) 
			{
				//vty_out(vty,"%% Bad parameter: port is member of a active trunk!\n");
				//ShowAlert(search(lcontrol,"port_member_trunk"));
				retu = -10;
								
			}
			else if (VLAN_RETURN_CODE_ARP_STATIC_CONFLICT == op_ret) 
			{
				//vty_out(vty,"%% Bad parameter: port has attribute of static arp!\n");
				//ShowAlert(search(lcontrol,"port_static_arp"));
				retu = -11;
			}
			else if(PVLAN_RETURN_CODE_THIS_PORT_HAVE_PVE == op_ret)
			{
				//vty_out(vty,"%% Bad parameter: port is pvlan port!please delete pvlan first!\n")
				//ShowAlert(search(lcontrol,"prot_member_pvlan"));;
				retu = -12;
			}
            else if(VLAN_RETURN_CODE_HAVE_PROT_VLAN_CONFIG == op_ret)
			{
    			//vty_out(vty,"%% There are protocol-vlan config on this vlan and port!\n");
    			return -13;
    		}
			else if (VLAN_RETURN_CODE_L3_INTF == op_ret) 
			{
				if(isAdd) 
				{
					//vty_out(vty,"%% Port adds to L3 interface vlan %d.\n",vlanid);
					//ShowAlert(search(lcontrol,"opt_fail"));
					//return -1;
				}
				else 
				{
					//vty_out(vty,"%% L3 interface vlan member can NOT delete here.\n");
					//ShowAlert(search(lcontrol,"opt_fail"));
					//return -1;
				}
				retu = -1;
			}
			else if (VLAN_RETURN_CODE_PORT_L3_INTF == op_ret) 
			{
				if(isAdd) 
				{
					//vty_out(vty,"%% Port can NOT add to vlan as port is L3 interface.\n");
					//return COMMON_FAIL;
					//ShowAlert(search(lcontrol,"opt_fail"));
					//return -1;
				}
				else 
				{
					//vty_out(vty,"%% Port can NOT delete from vlan as port is L3 interface.\n");
					//return COMMON_FAIL;
					//ShowAlert(search(lcontrol,"opt_fail"));
					//return -1;
				}
				retu = -1;
			}
			else if (VLAN_RETURN_CODE_ERR_HW == op_ret) 
			{
				//vty_out(vty,"%% Error occurs in Config on HW.\n");	
				//ShowAlert(search(lcontrol,"HW_error"));	
				retu = -1;
			}
			else if (ETHPORT_RETURN_CODE_UNSUPPORT == op_ret)
			{
				//vty_out(vty,"%% This operation is unsupported!\n");
				//return COMMON_FAIL;
				//ShowAlert(search(lcontrol,"opt_fail"));
				retu = -1;
			}
    		else if(VLAN_RETURN_CODE_PROMIS_PORT_CANNOT_DEL == op_ret)
			{
    			//vty_out(vty,"%% Can't del an advanced-routing interface port from this vlan\n");
				retu = -14;
    		}
			else if (VLAN_RETURN_CODE_ERR_NONE != op_ret)
			{
				//vty_out(vty,"%% Unknown Error! return %d \n",op_ret);
				retu = -1;
			}		

            //////////////////////
    	}
    else
	{
        //ShowAlert(search(lcontrol,"Get_args_error"));
        if (dbus_error_is_set(&err))
    	{			
            dbus_error_free(&err);		
    	}	
		retu = -1;
	}
    dbus_message_unref(reply);

	if( 1 != retu){
		return retu;
	}


	int i = 0,slot_id =0;	
   	int master_slot_id[2] = {-1, -1};	
	void *connection_master = NULL;
    int master_slot_count = get_product_info("/dbm/product/master_slot_count");
	int local_slot_id = get_product_info("/dbm/local_board/slot_id");

   	ret = ccgi_master_slot_id_get(master_slot_id);
	if(ret !=0 )
	{
		//vty_out(vty,"get master_slot_id error !\n");
		return -15;		
   	}
	if((local_slot_id<0)||(master_slot_count<0))
	{
		//vty_out(vty,"get get_product_info return -1 !\n");
		return -16;		
   	}

    for(i=0;i<master_slot_count;i++)
    {

		slot_id = master_slot_id[i];
    	query = NULL;
    	reply = NULL;
    	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
    										NPD_DBUS_VLAN_OBJPATH,	\
    										NPD_DBUS_VLAN_INTERFACE,	\
    										NPD_DBUS_VLAN_METHOD_CONFIG_VLANLIST_PORT_MEMBER_ADD_DEL);
    	
    	dbus_error_init(&err);

    	dbus_message_append_args(	query,
    							 	DBUS_TYPE_BYTE,&isAdd,
    								DBUS_TYPE_BYTE,&slot_no,
    								DBUS_TYPE_BYTE,&local_port_no,
    							 	DBUS_TYPE_BYTE,&isTagged,
    							 	DBUS_TYPE_UINT16,&vlanId,
    							 	DBUS_TYPE_INVALID);

		if(SNMPD_DBUS_SUCCESS != get_slot_dbus_connection(slot_id, &connection_master, SNMPD_INSTANCE_MASTER_V3))
		{
			reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
		}
		else
		{
			reply = dbus_connection_send_with_reply_and_block (connection_master,query,-1, &err);				
		}
			    	
    	dbus_message_unref(query);
    	if (NULL == reply) {
    		//vty_out(vty,"Please check npd on MCB slot %d\n",slot_id);
    		return -17;
    	}

    	if (dbus_message_get_args ( reply, &err,
    		DBUS_TYPE_UINT32,&op_ret,
    		DBUS_TYPE_INVALID)) {
    		if (VLAN_RETURN_CODE_ERR_NONE != op_ret){
    			//vty_out(vty,"%% vlan_list add/delete port Error! return %d \n",op_ret);
				retu = -18;
    		}		
    	} 
    	else {
    		//vty_out(vty,"Failed get args,Please check npd on MCB slot %d\n",slot_id);
			retu = -17;
    	}
    	dbus_message_unref(reply);
    	if(VLAN_RETURN_CODE_ERR_NONE != op_ret)
    	{
    		return retu;
    	}
    }
	return 1;	
}

int setVID(unsigned short vidOld,char * vidnew)   /*设定返回值，0为成功，-1为执行失败*/
{
    DBusMessage *query=NULL, *reply=NULL;
    DBusError err;
    unsigned short  vid_corrent,vid_new;
    unsigned int    ret,op_ret;
	int retu = 0;

    vid_corrent=vidOld;
    ret = parse_vlan_no(vidnew,&vid_new);
    if(vid_new<1 || vid_new>4094)
    	{
            //ShowAlert(search(lcontrol,"illegal_vID"));
            return -2;
    	}
    if (NPD_FAIL == ret) 
    	{
            //ShowAlert(search(lcontrol,"illegal_vID"));	
            return -2;
    	}
    else if(vid_corrent == vid_new)
    	{
        //ShowAlert(search(lcontrol,"same_vID"));	 
        return -3; //no return lead to segmentation fault.
    	}
    else 
    	{
        query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
                                    		NPD_DBUS_VLAN_OBJPATH,	\
                                        	NPD_DBUS_VLAN_INTERFACE,	\
                                            NPD_DBUS_VLAN_METHOD_UPDATE_VID);
    	
        dbus_error_init(&err);
        dbus_message_append_args(query,
                                DBUS_TYPE_UINT16,&vid_corrent,	
                                DBUS_TYPE_UINT16,&vid_new,
                            	DBUS_TYPE_INVALID);
        reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
    	}
    dbus_message_unref(query);
    if (NULL == reply)
    	{
        if (dbus_error_is_set(&err)) 
    		{
            dbus_error_free(&err);
    		}
            return 0;
    	}
    if (dbus_message_get_args ( reply, &err,
        DBUS_TYPE_UINT32,&op_ret,
        DBUS_TYPE_INVALID)) 
    	{
			/////////////////
			if (VLAN_RETURN_CODE_VLAN_EXISTS == op_ret ) 
			{
				//vty_out(vty,"% Bad param:new vid points to a vlan has Already exists!\n");				
				 //ShowAlert(search(lcontrol,"exist_vID"));
				 return -4;
			} 
			else if (VLAN_RETURN_CODE_TRUNK_EXISTS == op_ret) 
			{
				//vty_out(vty,"% Bad param:vlan has trunk member.\n");
				//ShowAlert(search(lpublic,"oper_fail"));
				return -1;
				
			}
			else if (VLAN_RETURN_CODE_ERR_HW == op_ret) 
			{
				//vty_out(vty,"Error occurs on HW.\n");
				//ShowAlert(search(lcontrol,"HW_error"));	
				return -5;
			}
			else if (VLAN_RETURN_CODE_ERR_NONE == op_ret) 
			{
				/*vty_out(vty,"vlan %d Changed vid to %d.\n",vid_corrent,vid_new);*/
				//ShowAlert(search(lcontrol,"modify_success"));
				retu = 1;
				return 1;
			}
			else if (VLAN_RETURN_CODE_L3_INTF == op_ret) 
			{
				//vty_out(vty,"vlan L3 interface NOT support modify vid.\n");
				//ShowAlert(search(lcontrol,"L3_modify_fail"));
			    return -6;
			}
			/////////////////
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

/*modified show_vlan_ByID by  tangsq 2010-1-19*/
int show_vlan_ByID(struct vlan_info *vlan_info_byID, unsigned short vID, int* untagport_num,int* tagport_num)
{
        DBusMessage *query=NULL, *reply=NULL;
        DBusError err;
        unsigned int    vlan_Cont = 0;
        unsigned int    slot =0,port =0, tmpVal =0;
	    char*			vlanName = NULL;	
        unsigned short  vlanId = vID;
        int port_num=0;
        unsigned int vlanStat = 0;
	    unsigned int 	product_id = PRODUCT_ID_NONE,count = 0,i = 0,j = 0,ret = 0;
        unsigned int    promisPortBmp[2] = {0};
        PORT_MEMBER_BMP untagPortBmp,tagPortBmp;
        memset(&untagPortBmp,0,sizeof(PORT_MEMBER_BMP));
        memset(&tagPortBmp,0,sizeof(PORT_MEMBER_BMP));


        //vlanName = (char*)malloc(ALIAS_NAME_SIZE);
        //memset(vlanName,0,ALIAS_NAME_SIZE);
    	
        query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
                                    		NPD_DBUS_VLAN_OBJPATH , \
                                        	NPD_DBUS_VLAN_INTERFACE ,	\
                                            NPD_DBUS_VLAN_METHOD_SHOW_VLAN_PORT_MEMBERS_V1 );
    	
    
       dbus_error_init(&err);
	   dbus_message_append_args(query,
							 DBUS_TYPE_UINT16,&vlanId,
							 DBUS_TYPE_INVALID);
        reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
    	
        dbus_message_unref(query);
        if (NULL == reply)
    		{
            if (dbus_error_is_set(&err)) 
        		{
                    dbus_error_free(&err);
        		}
            return 0;
    		}
		
    
       if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &ret,
					DBUS_TYPE_UINT32, &product_id,
					DBUS_TYPE_UINT32, &promisPortBmp[0],
					DBUS_TYPE_UINT32, &promisPortBmp[1],
					DBUS_TYPE_STRING, &vlanName,
					DBUS_TYPE_UINT32, &untagPortBmp.portMbr[0],
					DBUS_TYPE_UINT32, &untagPortBmp.portMbr[1],
					DBUS_TYPE_UINT32, &tagPortBmp.portMbr[0],
					DBUS_TYPE_UINT32, &tagPortBmp.portMbr[1],
					DBUS_TYPE_UINT32, &vlanStat,
					DBUS_TYPE_INVALID)) {
		fprintf(stderr,"ret %d,vlanName %s\n",ret,vlanName);
		if(VLAN_RETURN_CODE_ERR_NONE == ret) {
			strcpy(vlan_info_byID->vlanName , vlanName);
			if(0 != untagPortBmp.portMbr[0] || 0 != tagPortBmp.portMbr[0] || 0 != untagPortBmp.portMbr[1] || 0 != tagPortBmp.portMbr[1]){
				get_vlan_member_slot_port(vlan_info_byID,product_id,untagPortBmp,tagPortBmp,promisPortBmp,untagport_num,tagport_num);
			}
			

		}
		else if(ETHPORT_RETURN_CODE_NO_SUCH_VLAN == ret) {
			return -2;
		}
		else if(VLAN_RETURN_CODE_VLAN_NOT_EXISTS == ret) {
			return -3;
		}
		else if(VLAN_RETURN_CODE_ERR_GENERAL == ret) {
			return -4;
		}
	}
	else {
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	
    dbus_message_unref(reply);	
    return CMD_SUCCESS;
}



int createIntfForVlan(unsigned int vID)
{

    //int ret;
    unsigned int Advanced = 0;
	return create_vlan_intf(vID,0);
}


int execute_dcli_command (const char *command, int argc,char* argv1,char* argv2,char* argv3,char* argv4)
{
    
     int ret;
//   int paranum=0;
     pid_t pid;
     int status;
//   char *tmp=NULL;
//   char *cmdstr[8];
     pid = fork ();
    ret=-100;
     if (pid < 0)
    	{
          /* Failure of fork(). */
//        fprintf (stderr, "Can't fork: %s\n", safe_strerror (errno));
          exit (1);
    	}
       else if (pid == 0)
    	{
         switch (argc)
      {
      case 0:
       ret = execlp (command, command,(const char *)NULL);
       break;
      case 1:
       ret = execlp (command, command, argv1, (const char *)NULL);
       break;
      case 2:
      	 {
            ret = execlp ("/sbin/ifconfig", command, argv1, argv2, (const char *)NULL);
             break;
       }
      case 3:
       ret = execlp (command, command, argv1, argv2, argv3,(const char *)NULL);
       break;
      case 4:
       ret = execlp (command, command, argv1, argv2,argv3,argv4, (const char *)NULL);
       break;
      }
           /* When execlp suceed, this part is not executed. */
//        fprintf (stderr, "Can't execute %s: %s\n", command, safe_strerror (errno));
          exit (1);
    	}
       else
    	{
          /* This is parent. */
          ret = wait4 (pid, &status, 0, NULL);
    	}
      return 0;

    
}

int deleteIntfForVlan(unsigned short vID)
{
    DBusMessage *query, *reply;
    DBusError err;
    unsigned short vId = 0;
    unsigned int op_ret = 0,ifIndex = 0;
    vId=vID;
	int retu = 0;
    
    query = dbus_message_new_method_call(
                                NPD_DBUS_BUSNAME,			\
                                NPD_DBUS_INTF_OBJPATH,		\
                                NPD_DBUS_INTF_INTERFACE,			\
                                NPD_DBUS_INTF_METHOD_DEL_VID_INTF);
    
    dbus_error_init(&err);

    dbus_message_append_args(query,
                             DBUS_TYPE_UINT16,&vId,
                             DBUS_TYPE_INVALID);

    reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
    
    dbus_message_unref(query);
    
    if (NULL == reply)
    	{
            if (dbus_error_is_set(&err)) 
        		{
                    dbus_error_free(&err);
        		}
            dbus_message_unref(reply);
            return 0;
    	}
    
    if (dbus_message_get_args ( reply, &err,
        DBUS_TYPE_UINT32,&op_ret,
        DBUS_TYPE_UINT32, &ifIndex,
        DBUS_TYPE_INVALID)) {

	            if (INTERFACE_RETURN_CODE_SUCCESS == op_ret)
	            {
	                dbus_message_unref(reply);
	                sleep(1);
					retu = 1;
	                return 1;
	            }
	            else
	            {
		                //vty_out(vty,dcli_error_info_intf(op_ret));
					if(op_ret == 0)
					{
						retu = 1;
					}
					else
					{
						return op_ret;
					}
					dbus_message_unref(reply);
	            }		
    }
    else{
            if (dbus_error_is_set(&err))
    		{
                dbus_error_free(&err);
    		}
    	}
        dbus_message_unref(reply);
    return retu;
}

int deleteIntfForVlanNoShow(unsigned short vID)
{

    DBusMessage *query, *reply;
    DBusError err;
    unsigned short vId = 0;

    vId=vID;
    
    query = dbus_message_new_method_call(
                                NPD_DBUS_BUSNAME,			\
                                NPD_DBUS_INTF_OBJPATH,		\
                                NPD_DBUS_INTF_INTERFACE,			\
                                NPD_DBUS_INTF_METHOD_DEL_VID_INTF);
    
    dbus_error_init(&err);

    dbus_message_append_args(query,
                             DBUS_TYPE_UINT16,&vId,
                             DBUS_TYPE_INVALID);

    reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
    
    dbus_message_unref(query);
    
    if (NULL == reply)
    	{
            if (dbus_error_is_set(&err)) 
        		{
                    dbus_error_free(&err);
        		}
            dbus_message_unref(reply);
            return 0;
    	}
        dbus_message_unref(reply);
    return 1;
}

int show_vlan_member_slot_port
(
    unsigned int product_id_param,
    PORT_MEMBER_BMP untagBmp_param,
    PORT_MEMBER_BMP tagBmp_param,
    unsigned int * promisPortBmp_param
)
{
	//if (is_distributed == DISTRIBUTED_SYSTEM)
	{
	    /* add for show vlan on AX71-2X12G12S,zhangdi 2011-05-30 */
		unsigned int i,count = 0;
		unsigned int port = 0;
	    unsigned int tmpVal = 0;
		int slot = get_product_info(LOCAL_SLOTID_FILE);
	    if(slot<0)
	    {
	    	//vty_out(vty,"get_product_info() return -1,Please check dbm file !\n");
			return -1;			
	    }
		
		for (i=0;i<64;i++)
		{
			port = i;
			tmpVal = (1<<((i>31) ? (i-32) : i));
			if(((i>31) ? untagBmp_param.portMbr[1] : untagBmp_param.portMbr[0]) & tmpVal)
			{	
				if(count && (0 == count % 4)) 
				{
					//vty_out(vty,"\n%-32s"," ");
				}
				//vty_out(vty,"%s%d/%d(u%s)",(count%4) ? ",":"",slot,port,((promisPortBmp_param[i/32] & tmpVal)?"*":""));
				count++;
			}
		}
		port = 0;
		tmpVal = 0;
		for (i=0;i<64;i++)
		{
			port = i;
			tmpVal = (1<<((i>31) ? (i-32) : i));
			if(((i>31) ? tagBmp_param.portMbr[1] : tagBmp_param.portMbr[0]) & tmpVal) \
			{				
				if(count && (0 == count % 4))
				{
					//vty_out(vty,"\n%-32s"," ");
				}
				//vty_out(vty,"%s%d/%d(t%s)",(count%4) ? ",":"",slot,port,((promisPortBmp_param[i/32] & tmpVal)?"*":""));
				count++;
			}
		}	
		return 0;
	}
#if 0
	else
	{
		unsigned int i,count = 0;
		unsigned int slot = 0,port = 0;
	    unsigned int  tmpVal = 0;
		for (i=0;i<64;i++){
			if((PRODUCT_ID_AX7K_I == product_id) ||
				(PRODUCT_ID_AX7K == product_id)) {
				slot = i/8 + 1;
				port = i%8;
			}
			else if((PRODUCT_ID_AX5K == product_id) ||
					(PRODUCT_ID_AU4K == product_id) ||
					(PRODUCT_ID_AU3K == product_id) ||
					(PRODUCT_ID_AU3K_BCM == product_id) ||
					(PRODUCT_ID_AU3K_BCAT == product_id) || 
					(PRODUCT_ID_AU2K_TCAT == product_id)) {
				if(PRODUCT_ID_AU3K_BCAT == product_id){
					slot = 2;
				}
				else
				{
				    slot = 1;
				}
				port = i;
			}
			else if((PRODUCT_ID_AX5K_I == product_id)){
				slot = i/8;
				port = i%8 + 1;
			}

			tmpVal = (1<<((i>31) ? (i-32) : i));
			if(((i>31) ? untagBmp.portMbr[1] : untagBmp.portMbr[0]) & tmpVal) {	
				if(count && (0 == count % 4)) {
					vty_out(vty,"\n%-32s"," ");
				}
				if(PRODUCT_ID_AX7K_I == product_id) {
					vty_out(vty,"%scscd%d(u%s)",(count%4) ? ",":"", port-1 ,((promisPortBmp[i/32] & tmpVal)?"*":""));
				}
				else {
					vty_out(vty,"%s%d/%d(u%s)",(count%4) ? ",":"",slot,port,((promisPortBmp[i/32] & tmpVal)?"*":""));
				}
				count++;
			}
		}
		slot = 0;
		port = 0;
		tmpVal = 0;
		for (i=0;i<64;i++){
			if((PRODUCT_ID_AX7K_I == product_id) || 
				(PRODUCT_ID_AX7K == product_id)) {
				slot = i/8 + 1;
				port = i%8;
			}
			else if((PRODUCT_ID_AX5K == product_id) ||
					(PRODUCT_ID_AU4K == product_id) ||
					(PRODUCT_ID_AU3K == product_id) ||
					(PRODUCT_ID_AU3K_BCM == product_id) ||
					(PRODUCT_ID_AU3K_BCAT == product_id) || 
					(PRODUCT_ID_AU2K_TCAT == product_id)){
				if(PRODUCT_ID_AU3K_BCAT == product_id){
					slot = 2;
				}
				else
				{
				    slot = 1;
				}
				port = i;
			}
			else if((PRODUCT_ID_AX5K_I == product_id)){
				slot = i/8;
				port = i%8 + 1;
			}

			tmpVal = (1<<((i>31) ? (i-32) : i));
			if(((i>31) ? tagBmp.portMbr[1] : tagBmp.portMbr[0]) & tmpVal) {				
				if(count && (0 == count % 4)) {
					vty_out(vty,"\n%-32s"," ");
				}
				if(PRODUCT_ID_AX7K_I == product_id) {
					vty_out(vty,"%scscd%d(t%s)",(count%4) ? ",":"", port-1 ,((promisPortBmp[i/32] & tmpVal)?"*":""));
				}
				else {
					vty_out(vty,"%s%d/%d(t%s)",(count%4) ? ",":"",slot,port,((promisPortBmp[i/32] & tmpVal)?"*":""));
				}
				count++;
			}
		}	
		return 0;	
	}
#endif
}

int get_vlan_member_slot_port
(
	struct vlan_info *vlan_info_byID,
    unsigned int product_id_param,
    PORT_MEMBER_BMP untagBmp_param,
    PORT_MEMBER_BMP tagBmp_param,
    unsigned int * promisPortBmp_param,
    unsigned int * untagPortNum,
    unsigned int * tagPortNum
)
{
    unsigned int i,tmpVal = 0,count = 0;
    unsigned int slot = 0,port = 0;
	int k = 0;
    int uk = 0;
	int  port_num=0;
    for (i=0;i<64;i++)
    {
        if(PRODUCT_ID_AX7K == product_id_param) {
            slot = i/8 + 1;
            port = i%8;
    	}
		else if((PRODUCT_ID_AX5K == product_id_param) ||
				(PRODUCT_ID_AU4K == product_id_param) ||
				(PRODUCT_ID_AU3K == product_id_param) ||
				(PRODUCT_ID_AU3K_BCM == product_id_param) ||
				(PRODUCT_ID_AU3K_BCAT == product_id_param) || 
				(PRODUCT_ID_AU2K_TCAT == product_id_param)) {

            slot = 1;
            port = i;
    	}
		else if((PRODUCT_ID_AX5K_I == product_id_param)){
			slot = i/8;
			port = i%8 + 1;
		}
        tmpVal = (1<<((i>31) ? (i-32) : i));
		//fprintf(stderr,"33333333tmpVal=%u--i=%d",tmpVal,i );
        if(((i>31) ? untagBmp_param.portMbr[1] : untagBmp_param.portMbr[0]) & tmpVal) 
    		{
    			sprintf(vlan_info_byID->slot_port_untag[uk],"%d/%d",slot,port);
                vlan_info_byID->tagornot=1;
				
                if ( (promisPortBmp_param[i/32] & tmpVal) != 0 )
					
                    vlan_info_byID->promisProt_untag[uk] = 1;

				//fprintf(stderr,"vlan_info_byID.slot_port_untag[%d]=%s",uk,vlan_info_byID->slot_port_untag[uk]);
				port_num++;
				uk++;
                
            	count++;
    		}
    }


	*untagPortNum = port_num;
    slot = 0;
    port = 0;
    tmpVal = 0;
	port_num = 0;
    for (i=0;i<64;i++)
    {
        if(PRODUCT_ID_AX7K == product_id_param) 
    	{
            slot = i/8 + 1;
            port = i%8;
    	}
		else if((PRODUCT_ID_AX5K == product_id_param) ||
				(PRODUCT_ID_AU4K == product_id_param) ||
				(PRODUCT_ID_AU3K == product_id_param) ||
				(PRODUCT_ID_AU3K_BCM == product_id_param) ||
				(PRODUCT_ID_AU3K_BCAT == product_id_param) || 
				(PRODUCT_ID_AU2K_TCAT == product_id_param)){
            slot = 1;
            port = i;
    	}
		else if((PRODUCT_ID_AX5K_I == product_id_param)){
			slot = i/8;
			port = i%8 + 1;
		}
        tmpVal = (1<<((i>31) ? (i-32) : i));
		
        if(((i>31) ? tagBmp_param.portMbr[1] : tagBmp_param.portMbr[0]) & tmpVal)
        {
        	sprintf(vlan_info_byID->slot_port_tag[k],"%d/%d",slot,port);
            vlan_info_byID->tagornot=2;	
            if ( (promisPortBmp_param[i/32] & tmpVal) != 0 )
            vlan_info_byID->promisProt_tag[k] = 1;


			//fprintf(stderr,"vlan_info_byID.slot_port_tag[%d]=%s",k,vlan_info_byID->slot_port_untag[k]);
			port_num++;
			k++;
            count++;
			
    	}
    }
	*tagPortNum = port_num;
    return count;
}

int config_vlan_advanced_routing(unsigned int vID)
{
    //int ret;
    unsigned int Advanced = 1;
    
    
    return create_vlan_intf_advanced_routing(vID,Advanced);
}

int config_vlan_filter(unsigned short VID,char * status,char * filterParam)
{
    DBusMessage *query, *reply;
    DBusError err;
	int retu = 0;

    VLAN_FLITER_ENT vlanfilterType = VLAN_FILTER_TYPE_MAX;
    unsigned int en_dis = 0, op_ret = 0;
    unsigned short vlanId = 0;
    if(0 == strncmp(status,"enable",strlen(status))) {
        en_dis = 1;
    }
    else if (0 == strncmp(status,"disable",strlen(status))) {
        en_dis = 0;
    }
    else {
    	
        return -1;
    }

    if(0 == strncmp(filterParam,"unkown-unicast",strlen((char*)filterParam))){
        vlanfilterType = VLAN_FILTER_UNKOWN_UC;
    }
    else if(0 == strncmp(filterParam,"unreg-ipv4-multicast",strlen((char*)filterParam))){
        vlanfilterType = VLAN_FILTER_UNREG_IPV4_MC;
    }
    else if(0 == strncmp(filterParam,"unreg-ipv6-multicast",strlen((char*)filterParam))){
        vlanfilterType = VLAN_FILTER_UNREG_IPV6_MC;
    }
    else if(0 == strncmp(filterParam,"unreg-nonIp-multicast",strlen((char*)filterParam))){
        vlanfilterType = VLAN_FILTER_UNREG_NONIP_MC;
    }
    else if(0 == strncmp(filterParam,"unreg-ipv4-broadcast",strlen((char*)filterParam))){
        vlanfilterType = VLAN_FILTER_UNREG_IPV4_BC;
    }
    else if(0 == strncmp(filterParam,"unreg-nonIpv4-broadcast",strlen((char*)filterParam))){
        vlanfilterType = VLAN_FILTER_UNREG_NONIPV4_BC;
    }
    else {
        return -1;
    }

    vlanId = VID;
    query = dbus_message_new_method_call(
                                		NPD_DBUS_BUSNAME,	\
                                    	NPD_DBUS_VLAN_OBJPATH ,	\
                                        NPD_DBUS_VLAN_INTERFACE ,	\
                                        NPD_DBUS_VLAN_METHOD_CONFIG_FILTER );
    
    dbus_error_init(&err);

    dbus_message_append_args(query,
                            DBUS_TYPE_UINT16,&vlanId,
                            DBUS_TYPE_UINT32,&vlanfilterType,
                            DBUS_TYPE_UINT32,&en_dis,
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
		if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret){
            //ShowAlert(search(lcontrol,"Set_filter_fail"));
            return -2;
    	}
        else if(NPD_DBUS_SUCCESS == op_ret) {
            //ShowAlert(search(lcontrol,"Set_filter_suc"));
            retu = 1;
            return 1;
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

int addordel_trunk(char * addordel,unsigned short VID,char * trunk_id,char * tagornot ,struct list * lcontrol)
{
    
    DBusMessage *query=NULL, *reply=NULL;
    DBusError err;
    unsigned short vlanId,trunk_no;
    char  isAdd     	= 0;
    char isTagged       = 0;
    unsigned int    ret,op_ret;
	int retu = 0;
    ret = param_first_char_check((char*)addordel,0);
    if (1 ==ret){
        isAdd = 1;
    }
    else if(0 == ret){
        isAdd = 0;
    }
    /*fetch the 2nd param : slotNo/portNo*/
    op_ret = parse_trunk_no1((char *)trunk_id,&trunk_no);
    if (NPD_FAIL == op_ret) {
        //ShowAlert(search(lcontrol,"illegal_input"));
        return -2;
    }
    #if 1
    if(strncmp(tagornot,"tagged",strlen(tagornot))==0)
    {
        isTagged = 1;
    }
    else if (strncmp(tagornot,"untagged",strlen(tagornot))==0)
    {
        isTagged = 0;
    }
    else
    {
        //ShowAlert(search(lcontrol,"illegal_input"));
        return -2;
    }
    #endif 
    vlanId = VID;
    query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	
                                    	NPD_DBUS_VLAN_OBJPATH,	
                                    	NPD_DBUS_VLAN_INTERFACE,	
                                        NPD_DBUS_VLAN_METHOD_CONFIG_TRUNK_MEMBER_UNTAG_TAG_ADD_DEL);
    
    dbus_error_init(&err);

    dbus_message_append_args(   query,
                                DBUS_TYPE_BYTE,&isAdd,
                                DBUS_TYPE_UINT16,&trunk_no,
                                DBUS_TYPE_BYTE,&isTagged,
                                DBUS_TYPE_UINT16,&vlanId,
                             	DBUS_TYPE_INVALID);
    
    reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
    
    dbus_message_unref(query);
    if (NULL == reply) {
        if (dbus_error_is_set(&err)) {
            dbus_error_free(&err);
    	}
        return CMD_SUCCESS;
    }
    if (dbus_message_get_args ( reply, &err,
        DBUS_TYPE_UINT32,&op_ret,
        DBUS_TYPE_INVALID)) {      
		if (VLAN_RETURN_CODE_VLAN_NOT_EXISTS == op_ret) 
		{
			//vty_out(vty,"% Bad parameter,vlan %d not exist.\n",vlanId);
			//ShowAlert(search(lcontrol,"VLAN_NOT_EXITSTS"));
			return -3;
		}
		else if (TRUNK_RETURN_CODE_TRUNK_NOTEXISTS == op_ret)
		{
			//vty_out(vty,"% Bad parameter,trunk %d not exsit.\n",trunk_no);
			//ShowAlert(search(lcontrol,"trunk_not_exist"));
			return -4;
		}		
		else if (VLAN_RETURN_CODE_TRUNK_EXISTS == op_ret)
		{
			//vty_out(vty,"% Bad parameter,trunk %d already vlan %d member.\n",trunk_no,vlanId);
			//ShowAlert(search(lcontrol,"vlan_Already_trunk"));
			return -5;
		}
		else if (VLAN_RETURN_CODE_TRUNK_NOTEXISTS == op_ret)
		{
			//vty_out(vty,"% Bad parameter,trunk %d was NOT member vlan %d.\n",trunk_no,vlanId);
			//ShowAlert(search(lcontrol,"port_NotExist"));
			return -6;
		}
		else if (VLAN_RETURN_CODE_TRUNK_CONFLICT == op_ret)
		{
			//vty_out(vty,"% Bad parameter,trunk %d already untagged member of other active vlan.\n",trunk_no);
			//ShowAlert(search(lcontrol,"vlan_had_other_trunk"));
			return -7;
		}
		else if (VLAN_RETURN_CODE_TRUNK_MEMBER_NONE == op_ret)
		{
			//vty_out(vty,"% Bad parameter,there exists no member in trunk %d.\n",trunk_no);
			//ShowAlert(search(lcontrol,"trunk_no_member"));
			return -8;
		}		
		else if (VLAN_RETURN_CODE_ERR_HW == op_ret)
		{
			//vty_out(vty,"%% Error occurs in config on hardware.\n");
			//ShowAlert(search(lcontrol,"HW_error"));	
			return -9;
		}	
		else if (VLAN_RETURN_CODE_ERR_NONE == op_ret ) 
		{
			/*vty_out(vty,"%s trunk %d vlan %d ok.\n", isAdd ? "add":"delete",trunk_no,vlanId);*/
			if(isAdd==1)
			{
				//ShowAlert(search(lcontrol,"add_trunk_suc"));
				return 1;
			}
			else if(isAdd==0)
			{
				//ShowAlert(search(lcontrol,"delete_trunk_succ"));
				return 1;
			}
			retu = 1;
		}
		else if (VLAN_RETURN_CODE_TRUNK_MBRSHIP_CONFLICT == op_ret )
		{
			//vty_out(vty,"% Bad parameter,trunk %d membership conflict.\n", trunk_no);
			//ShowAlert(search(lcontrol,"trunk_conflict"));	
			return -10;
		} 
    } 
    else {
        if (dbus_error_is_set(&err)) {
            dbus_error_free(&err);
    	}
    }
    dbus_message_unref(reply);
    return retu;
}


int show_vlan_member_slot_port_link
(
	unsigned int product_id,
	PORT_MEMBER_BMP untagBmp,
	PORT_MEMBER_BMP tagBmp,
	unsigned int * promisPortBmp,
    struct Vlanid_info *head
)
{
	unsigned int i,count = 0;
	unsigned int slot = 0,port = 0;
    unsigned int  tmpVal = 0;
	int retu = 0;

	struct Vlanid_info *tail = NULL,*q = NULL;
	head->next = NULL;
	tail = head;
	for (i=0;i<64;i++){
		if((PRODUCT_ID_AX7K_I == product_id) ||
			(PRODUCT_ID_AX7K == product_id)) {
			slot = i/8 + 1;
			port = i%8;
		}
		else if((PRODUCT_ID_AX5K == product_id) ||
				(PRODUCT_ID_AU4K == product_id) ||
				(PRODUCT_ID_AU3K == product_id) ||
				(PRODUCT_ID_AU3K_BCM == product_id) ||
				(PRODUCT_ID_AU3K_BCAT == product_id) || 
				(PRODUCT_ID_AU2K_TCAT == product_id)) {
			slot = 1;
			port = i;
		}
		else if((PRODUCT_ID_AX5K_I == product_id)){
			slot = i/8;
			port = i%8 + 1;
		}
		tmpVal = (1<<((i>31) ? (i-32) : i));
		if(((i>31) ? untagBmp.portMbr[1] : untagBmp.portMbr[0]) & tmpVal) {	

			q = (struct Vlanid_info*)malloc(sizeof(struct Vlanid_info));
			if (NULL == q)
			{
				return -1;
			}
			memset(q,0,sizeof(struct Vlanid_info));
			retu = 1;
			if(count && (0 == count % 4)) {
				//vty_out(vty,"\n%-32s"," ");
			}
			if(PRODUCT_ID_AX7K_I == product_id) {
				sprintf(q->untagport,"cscd%d(u%s)", port-1 ,((promisPortBmp[i/32] & tmpVal)?"*":""));
				q->untagflag = ((promisPortBmp[i/32] & tmpVal)?1:0);
				q->iftag = 0;
			}
			else {
				sprintf(q->untagport,"%d/%d(u%s)",slot,port,((promisPortBmp[i/32] & tmpVal)?"*":""));
				q->untagflag = ((promisPortBmp[i/32] & tmpVal)?1:0);
				q->iftag = 0;
			}
			count++;
    		q->next=NULL;
            tail->next=q;
            tail=q;
		}
	}
	slot = 0;
	port = 0;
	tmpVal = 0;
	for (i=0;i<64;i++){
		if((PRODUCT_ID_AX7K_I == product_id) || 
			(PRODUCT_ID_AX7K == product_id)) {
			slot = i/8 + 1;
			port = i%8;
		}
		else if((PRODUCT_ID_AX5K == product_id) ||
				(PRODUCT_ID_AU4K == product_id) ||
				(PRODUCT_ID_AU3K == product_id) ||
				(PRODUCT_ID_AU3K_BCM == product_id) ||
				(PRODUCT_ID_AU3K_BCAT == product_id) || 
				(PRODUCT_ID_AU2K_TCAT == product_id)){
			slot = 1;
			port = i;
		}
		else if((PRODUCT_ID_AX5K_I == product_id)){
			slot = i/8;
			port = i%8 + 1;
		}
		tmpVal = (1<<((i>31) ? (i-32) : i));
		if(((i>31) ? tagBmp.portMbr[1] : tagBmp.portMbr[0]) & tmpVal) {		

			q = (struct Vlanid_info*)malloc(sizeof(struct Vlanid_info));
			if (NULL == q)
			{
				return -1;
			}

			memset(q,0,sizeof(struct Vlanid_info));
			retu = 1;
			if(count && (0 == count % 4)) {
				//vty_out(vty,"\n%-32s"," ");
			}
			if(PRODUCT_ID_AX7K_I == product_id) {
				sprintf(q->tagport,"cscd%d(t%s)", port-1 ,((promisPortBmp[i/32] & tmpVal)?"*":""));
				q->tagflag = ((promisPortBmp[i/32] & tmpVal)?1:0);
				q->iftag = 1;
			}
			else {
				sprintf(q->tagport,"%d/%d(t%s)",slot,port,((promisPortBmp[i/32] & tmpVal)?"*":""));
				q->tagflag = ((promisPortBmp[i/32] & tmpVal)?1:0);
				q->iftag = 1;
			}

			count++;
			q->next=NULL;
            tail->next=q;
            tail=q;
		}
	}	

	return retu;
}

void Free_vlanid_info(struct Vlanid_info * head)
{
    struct Vlanid_info *f1,*f2;
	f1=head->next;		 
	if (NULL == f1)
	{
		return ;
	}
	f2=f1->next;
	while(f2!=NULL)
	{
	  free(f1);
	  f1=f2;
	  f2=f2->next;
	}
	free(f1);
}

int show_vlanid_portlist(char *vid,struct Vlanid_info *head)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter	 iter;
	
	DBusMessageIter	 iter_array;
	unsigned short	vlanId = 0;
	char*			vlanName = NULL;
	unsigned int	count = 0, ret = 0;
	unsigned int 	i = 0, row = 0;
	unsigned int slot = 0, port = 0, tmpVal = 0;
	unsigned int product_id = 0; 
	unsigned int vlanStat = 0;
	unsigned int bitOffset = 0, mbrCount = 0;
	unsigned int promisPortBmp[2] = {0};
	int retu = 0;
	int op_ret = 0;

	PORT_MEMBER_BMP untagPortBmp,tagPortBmp;
	memset(&untagPortBmp,0,sizeof(PORT_MEMBER_BMP));
	memset(&tagPortBmp,0,sizeof(PORT_MEMBER_BMP));
	ret = parse_vlan_no((char*)vid,&vlanId);


	if (NPD_FAIL == ret) {
		//vty_out(vty,"% Bad parameter :vlan ID illegal!\n");
		return -2;
	}

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_VLAN_OBJPATH ,	\
										NPD_DBUS_VLAN_INTERFACE ,\
										NPD_DBUS_VLAN_METHOD_SHOW_VLAN_PORT_MEMBERS_V1 );
	

	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT16,&vlanId,
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
					DBUS_TYPE_UINT32, &ret,
					DBUS_TYPE_UINT32, &product_id,
					DBUS_TYPE_UINT32, &promisPortBmp[0],
					DBUS_TYPE_UINT32, &promisPortBmp[1],
					DBUS_TYPE_STRING, &vlanName,
					DBUS_TYPE_UINT32, &untagPortBmp.portMbr[0],
					DBUS_TYPE_UINT32, &untagPortBmp.portMbr[1],
					DBUS_TYPE_UINT32, &tagPortBmp.portMbr[0],
					DBUS_TYPE_UINT32, &tagPortBmp.portMbr[1],
					DBUS_TYPE_UINT32, &vlanStat,
					DBUS_TYPE_INVALID)) {
		if(VLAN_RETURN_CODE_ERR_NONE == ret) {
			retu = 1;
			if(0 != untagPortBmp.portMbr[0] || 0 != tagPortBmp.portMbr[0] || 0 != untagPortBmp.portMbr[1] || 0 != tagPortBmp.portMbr[1]){
				op_ret = show_vlan_member_slot_port_link(product_id,untagPortBmp,tagPortBmp,promisPortBmp,head);
				if (1 == op_ret)
				{
					retu = 1;
				}
				else
				{
					retu = -1;
				}

			}
			else 
			{
				//vty_out(vty,"  %-40s","No Port member.");
				retu = -3;
			}
		}
		else if(ETHPORT_RETURN_CODE_NO_SUCH_VLAN == ret) {
			//vty_out(vty,"% Bad parameter,vlan id illegal.\n");
			retu = -4;
		}
		else if(VLAN_RETURN_CODE_VLAN_NOT_EXISTS == ret) {
			//vty_out(vty,"% Bad parameter,vlan %d not exist.\n",vlanId);
			retu = -5;
		}
		else if(VLAN_RETURN_CODE_ERR_GENERAL == ret) {
			//vty_out(vty,"%% Error,operation on software fail.\n");
			retu = -6;
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

int show_trunklis_by_vlanId(char * vlanID,struct Trunklist * trunkdta,int * num)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter	 iter;
	
	DBusMessageIter	 iter_array;
	unsigned short	vlanId = 0;
	char*			vlanName = NULL;
	unsigned int vlan_Cont = 0;
	unsigned int count = 0, ret = 0;
	unsigned int i = 0, j = 0;
	unsigned int slot = 0, port = 0, tmpVal = 0;
	unsigned int product_id = 0;
	unsigned int bitOffset = 0, mbrCount = 0;
	unsigned int untagTrkBmp[4] = {0}, tagTrkBmp[4] = {0};
	unsigned int vlanStat = 0;

	int retu = 0;
    struct Trunklist *tail = NULL,*q = NULL; 
	trunkdta->next= NULL ;
	tail=trunkdta;

    ret = parse_vlan_no((char*)vlanID,&vlanId);

    if (NPD_FAIL == ret) {
        //vty_out(vty,"% Bad parameter :vlan ID illegal!\n");
        return -2;
    }

    query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
                                    	NPD_DBUS_VLAN_OBJPATH ,	\
                                    	NPD_DBUS_VLAN_INTERFACE ,\
                                        NPD_DBUS_VLAN_METHOD_SHOW_VLAN_TRUNK_MEMBERS );
    
    dbus_error_init(&err);
    dbus_message_append_args(query,
                             DBUS_TYPE_UINT16,&vlanId,
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
                    DBUS_TYPE_UINT32, &ret,
                    DBUS_TYPE_UINT32, &product_id,
                    DBUS_TYPE_STRING, &vlanName,
                    DBUS_TYPE_UINT32, &untagTrkBmp[0],
                    DBUS_TYPE_UINT32, &untagTrkBmp[1],
                    DBUS_TYPE_UINT32, &untagTrkBmp[2],
                    DBUS_TYPE_UINT32, &untagTrkBmp[3],
                    DBUS_TYPE_UINT32, &tagTrkBmp[0],
                    DBUS_TYPE_UINT32, &tagTrkBmp[1],
                    DBUS_TYPE_UINT32, &tagTrkBmp[2],
                    DBUS_TYPE_UINT32, &tagTrkBmp[3],					
                    DBUS_TYPE_UINT32, &vlanStat,
                    DBUS_TYPE_INVALID)){	

		if(VLAN_RETURN_CODE_ERR_NONE == ret){    
			retu = 1;
            for(i =0; i<4; i++) {
                if(0 != untagTrkBmp[i] || 0 != tagTrkBmp[i]) {
                    for(bitOffset = 0;bitOffset < 32;bitOffset++) {
        				
                        if(untagTrkBmp[i] & (1<<bitOffset)) {							
                            q = (struct Trunklist*)malloc(sizeof(struct Trunklist)+1);
							if (NULL == q)
							{
								return -1;
							}
							memset(q,0,sizeof(struct Trunklist)+1);
                    		mbrCount++;
                            //vty_out(vty,"%s%s%d(u)",((mbrCount-1)%4)?",":"","trunk",(i*32 + bitOffset+1));
                            q->TrunkId=(i*32 + bitOffset+1);
                    		q->tagMode=1;
                    		q->next=NULL;
                            tail->next=q;
                            tail=q;
							///////////////////////////
							//if(untagTrkBmp[i] & (1<<bitOffset)) {
							//if(0 == mbrCount % 4 && 0 != mbrCount){
							//vty_out(vty,"\n%-32s"," ");
							//
							//}	
							//mbrCount++;
							//vty_out(vty,"%s%s%d(u)",((mbrCount-1)%4)?",":"","trunk",(i*32 + bitOffset+1));
							//////////////////////////
        				}
            			else 
        				{
                			continue;
        				}
        				
        			}
                    for(bitOffset = 0;bitOffset < 32;bitOffset++) {
                        if(tagTrkBmp[i] & (1<<bitOffset)) {
                            q=(struct Trunklist*)malloc(sizeof(struct Trunklist));
							if (NULL == q)
							{
								return -1;
							}
							memset(q,0,sizeof(struct Trunklist)+1);
                    		mbrCount++;
                            //vty_out(vty,"%s%s%d(t)",((mbrCount-1)%4)?",":"","trunk",(i*32 + bitOffset+1));
                            q->TrunkId=(i*32 + bitOffset+1);
                    		q->tagMode=2;
                    		q->next=NULL;
                            tail->next=q;
                            tail=q;
        				}
            			else
        				{
                			continue;
        				}
        			}
        		}
            	else 
        		{
                	continue;
        		}
    		}
            *num=mbrCount;
            if(0 == mbrCount){ 
                //vty_out(vty,"  %-40s","No Trunk member.");
                retu = -3;
        	}	
    	}
		else if(ETHPORT_RETURN_CODE_NO_SUCH_VLAN == ret)
		{
			//vty_out(vty,"% Bad parameter,vlan id illegal.\n");
			//ShowAlert(search(lcontrol,"vlan_id_illegal"));
			retu  = -4;
		}		
		else if(VLAN_RETURN_CODE_VLAN_NOT_EXISTS == ret)
		{
			//vty_out(vty,"% Bad parameter: vlan %d not exist.\n",vlanId);
			//ShowAlert(search(lcontrol,"VLAN_NOT_EXITSTS"));
			retu = -5;
		}
		//////////////
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

void Free_vlan_trunk(struct Trunklist * head)
{
    struct Trunklist *f1,*f2;
	f1=head->next;		 
	if (NULL == f1)
	{
		return ;
	}
	f2=f1->next;
	while(f2!=NULL)
	{
	  free(f1);
	  f1=f2;
	  f2=f2->next;
	}
	free(f1);
}

#if PRODUCT_SELECTOR_SWITCH
int Product_Adapter_for_page(PRODUCT_PAGE_INFO * pstProductInfo,char * product_name)
{
	if( pstProductInfo == NULL )
		return -1;
	if( !strcmp(product_name, "Switch7000"))
	{
		pstProductInfo->port_total_num = 24;
		pstProductInfo->slot_num = 4;
		pstProductInfo->port_num = 6;
	}
	else if( !strcmp(product_name, "Switch3052"))
	{
		pstProductInfo->port_total_num = 52;
		pstProductInfo->slot_num = 1;
		pstProductInfo->port_num = 52;
	}
	else if( !strcmp(product_name, "Switch5612"))
	{
		pstProductInfo->port_total_num = 12;
		pstProductInfo->slot_num = 1;
		pstProductInfo->port_num = 12;
	}
	else if( !strcmp(product_name, "Switch5612i"))
	{
		pstProductInfo->port_total_num = 8;
		pstProductInfo->slot_num = 1;
		pstProductInfo->port_num = 8;
	}

	else//默认做4624处理
	{
		pstProductInfo->port_total_num = 24;
		pstProductInfo->slot_num = 1;
		pstProductInfo->port_num = 24;
	}
	return 0;
}

/*isenable的范围是"enable"或"disable"*/
int config_vlan_egress_filter(char *isenable)/*返回0表示失败，返回1表示成功*/
												/*返回-1表示bad command parameter!*/
												/*返回-2表示Product not support this function*/
												/*返回-3表示Config vlan mtu failed*/
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int isable = 0;
	unsigned int op_ret = 0;
	int retu = 0;
	
	if(strncmp("enable",isenable,strlen(isenable))==0)
	{
		isable = 1;
	}
	else if (strncmp("disable",isenable,strlen(isenable))==0)
	{
		isable = 0;
	}
	else
	{
		return -1;
	}

	query = dbus_message_new_method_call(
										NPD_DBUS_BUSNAME,	\
										NPD_DBUS_VLAN_OBJPATH , \
										NPD_DBUS_VLAN_INTERFACE ,	\
										NPD_DBUS_VLAN_METHOD_CONFIG_EGRESS_FILTER );
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&isable,
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
		if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret){
			retu = -2;
		}
		else if (NPD_DBUS_ERROR == op_ret) {
			retu = -3;
		}
		else if(NPD_DBUS_SUCCESS == op_ret) {
			retu = 1;
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
	return retu;
}

int show_vlan_egress_filter()/*返回1表示Vlan egress filter is enabled*/
								/*返回2表示Vlan egress filter is disabled*/
								/*返回0表示失败，返回-1表示Product not support this function!*/
{	
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = { 0 };
	unsigned int Isable = NPD_TRUE, ret = 0;
	int retu = 0;
	
	query = dbus_message_new_method_call(
					NPD_DBUS_BUSNAME,  \
					NPD_DBUS_VLAN_OBJPATH ,  \
					NPD_DBUS_VLAN_INTERFACE ,  \
					NPD_DBUS_VLAN_METHOD_SHOW_EGRESS_FILTER );
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return 0;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&Isable,
		DBUS_TYPE_UINT32,&ret,
		DBUS_TYPE_INVALID)) {						
		if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == ret){
			retu = -1;
		}
		else{
			if(NPD_TRUE == Isable) {
				retu = 1;
			}
			if(NPD_FALSE == Isable) {
				retu = 2;
			}
		}
	} 
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		retu = 0;
	}
	dbus_message_unref(reply);
	return retu;
}

#endif 

void get_vlan_ports_collection(struct vlan_ports_collection *ports)
{
	int ret = 0, vlanNum = 0, i = 0, j = 0;
	struct vlan_info_detail  vhead,*vq=NULL;
	int slot_count = get_product_info(SLOT_COUNT_FILE);	

	for(i = 0; i < slot_count; i++ )
	{
		ports[i+1].have_port = 0;
		ports[i+1].port_min = 0;
		ports[i+1].port_max = 0;
	}
	
	memset(&vhead,0,sizeof(struct vlan_info_detail));
	ret = show_vlan_list_slot(&vhead, &vlanNum);
	if(1 == ret)
	{
		for(vq = vhead.next; vq; vq = vq->next)
		{
			for(i = 0; i < slot_count; i++ )
			{
				for(j = 0; j < 64; j++ )
				{
					if(j<32)
					{
						if((vq->untagPortBmp[i].low_bmp)&(1<<j))
						{
							if(0 == ports[i+1].have_port)
							{
								ports[i+1].port_min = j+1;
								ports[i+1].port_max = j+1;
								ports[i+1].have_port = 1;
							}
							else
							{
								ports[i+1].port_min = ((ports[i+1].port_min < j+1)?ports[i+1].port_min:j+1);
								ports[i+1].port_max = ((ports[i+1].port_max > j+1)?ports[i+1].port_max:j+1);
							}
						}
						else
						{
							continue;
						}
					}
					else
					{
						if((vq->untagPortBmp[i].high_bmp)&(1<<(j-32)))
						{
							if(0 == ports[i+1].have_port)
							{
								ports[i+1].port_min = j+1;
								ports[i+1].port_max = j+1;
								ports[i+1].have_port = 1;
							}
							else
							{
								ports[i+1].port_min = ((ports[i+1].port_min < j+1)?ports[i+1].port_min:j+1);
								ports[i+1].port_max = ((ports[i+1].port_max > j+1)?ports[i+1].port_max:j+1);
							}
						}
						else
						{
							continue;
						}								
					}
				}				
			}

			for(i = 0; i < slot_count; i++ )
			{
    			for(j = 0; j < 64; j++ )
    			{    				
					if(j<32)
					{
						if((vq->tagPortBmp[i].low_bmp)&(1<<j))
                    	{
    						if(0 == ports[i+1].have_port)
							{
								ports[i+1].port_min = j+1;
								ports[i+1].port_max = j+1;
								ports[i+1].have_port = 1;
							}
							else
							{
								ports[i+1].port_min = ((ports[i+1].port_min < j+1)?ports[i+1].port_min:j+1);
								ports[i+1].port_max = ((ports[i+1].port_max > j+1)?ports[i+1].port_max:j+1);
							}
                    	}
    					else
    					{
    						continue;
    					}													
					}
					else
					{
                        if((vq->tagPortBmp[i].high_bmp)&(1<<(j-32)))
                    	{
    						if(0 == ports[i+1].have_port)
							{
								ports[i+1].port_min = j+1;
								ports[i+1].port_max = j+1;
								ports[i+1].have_port = 1;
							}
							else
							{
								ports[i+1].port_min = ((ports[i+1].port_min < j+1)?ports[i+1].port_min:j+1);
								ports[i+1].port_max = ((ports[i+1].port_max > j+1)?ports[i+1].port_max:j+1);
							}
                    	}
    					else
    					{
    						continue;
    					}									
					}
    			}				
			}
		}
	}
	Free_show_vlan_list_slot(&vhead);
}

/*tag_flag:1:Untagged 2:Tagged 3:non-members*/
int get_vlan_port_member_tagflag(int vlan_id,char *port,int *tag_flag)/*返回0表示失败，返回1表示成功*/
																			/*返回-1表示Failed to open file，返回-2表示Failed to mmap*/
																			/*返回-3表示vlan is NOT exists，返回-4表示Failed to munmap*/
																			/*返回-5表示close shm_vlan failed*/
{
	if(NULL == port)
		return 0;

	*tag_flag = 3;
	
	unsigned short	vlanId = 0;
	unsigned int	ret = 0;
	unsigned int 	i = 0;
	char temp_port[10] = { 0 };
	int retu = 0;
	
	/*ret = parse_vlan_no((char*)argv[0],&vlanId);

	if (NPD_FAIL == ret) {
		vty_out(vty,"% Bad parameter :vlan ID illegal!\n");
		return CMD_SUCCESS;
	}*/
	vlanId = vlan_id;

    /* read vlan info from file: shm_vlan ,zhangdi@autelan.com */		
	int	count_tag =0;
	int	count_untag =0;		
    int slot_count = get_product_info(SLOT_COUNT_FILE);
	
	int fd = -1, j=0;
	struct stat sb;

	vlan_list_t * mem_vlan_list =NULL;
	char* file_path = "/dbm/shm/vlan/shm_vlan";
	/* only read file */
    fd = open(file_path, O_RDONLY);
	if(fd < 0)
    {
        return -1;
    }
	fstat(fd,&sb);
	/* map not share */	
    mem_vlan_list = (vlan_list_t *)mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, fd, 0 );
    if(MAP_FAILED == mem_vlan_list)
    {
		close(fd);
        return -2;
    }	

    if(mem_vlan_list[vlanId-1].vlanStat == 1)
	{		
		count_untag =0;
		for(i = 0; i < slot_count; i++ )
		{
			for(j = 0; j < 64; j++ )
			{
				if(j<32)
				{
                    if((mem_vlan_list[vlanId-1].untagPortBmp[i].low_bmp)&(1<<j))
                	{
                		memset(temp_port, 0, sizeof(temp_port));
						snprintf(temp_port, sizeof(temp_port)-1, "%d/%d", i+1, j+1);

						if(0 == strcmp(temp_port,port))
						{
							*tag_flag = 1;
							retu = 1;
							goto munmap_close;
						}
            			count_untag++;						
                	}
					else
					{
						continue;
					}
				}
				else
				{
                    if((mem_vlan_list[vlanId-1].untagPortBmp[i].high_bmp)&(1<<(j-32)))
                	{
						memset(temp_port, 0, sizeof(temp_port));
						snprintf(temp_port, sizeof(temp_port)-1, "%d/%d", i+1, j+1);

						if(0 == strcmp(temp_port,port))
						{
							*tag_flag = 1;
							retu = 1;
							goto munmap_close;
						}
            			count_untag++;						
                	}
					else
					{
						continue;
					}								
				}
			}				
		}

		count_tag = 0;
		for(i = 0; i < slot_count; i++ )
		{
			for(j = 0; j < 64; j++ )
			{
				if(j<32)
				{
                    if((mem_vlan_list[vlanId-1].tagPortBmp[i].low_bmp)&(1<<j))
                	{
						memset(temp_port, 0, sizeof(temp_port));
						snprintf(temp_port, sizeof(temp_port)-1, "%d/%d", i+1, j+1);

						if(0 == strcmp(temp_port,port))
						{
							*tag_flag = 2;
							retu = 1;
							goto munmap_close;
						}
            			count_tag++;													
                	}
					else
					{
						continue;
					}													
				}
				else
				{
                    if((mem_vlan_list[vlanId-1].tagPortBmp[i].high_bmp)&(1<<(j-32)))
                	{
						memset(temp_port, 0, sizeof(temp_port));
						snprintf(temp_port, sizeof(temp_port)-1, "%d/%d", i+1, j+1);

						if(0 == strcmp(temp_port,port))
						{
							*tag_flag = 2;
							retu = 1;
							goto munmap_close;
						}
            			count_tag++;													
                	}
					else
					{
						continue;
					}									
				}
			}				
		}
	}
    else
	{
        /* do nothing */
		retu = -3;
	}

munmap_close:
	/* munmap and close fd */
    ret = munmap(mem_vlan_list,sb.st_size);
    if( ret != 0 )
    {
		retu = -4;
    }	
	ret = close(fd);
	if( ret != 0 )
    {
		retu = -5;
    }

    return retu;				
}	

/*oper为"bond"或"unbond"*/
/*vlanid的范围是2-4093*/
/*slotid的范围是1-16*/
/*cpu的范围是1-2*/
/*port的范围是1-8*/
int bond_vlan_to_ms_cpu_port_cmd(char *oper,char *vlanid,char *slotid,char *cpu,char *port)
																/*返回0表示失败，返回1表示成功*/
																/*返回-1表示get_product_info() return -1,Please check dbm file*/
																/*返回-2表示Bad parameter，返回-3表示vlan id illegal*/
																/*返回-4表示parse param failed ，返回-5表示slot id illegal*/
																/*返回-6表示cpu no illegal，返回-7表示cpu port illegal*/
																/*返回-8表示the dist slot is not AC board，返回-9表示ve if is exist,no interface first*/
																/*返回-10表示vlan is no bond to slot, need not unbond*/
																/*返回-11表示vlan do not exists on slot*/
																/*返回-12表示vlan is already bond to slot, can not bond*/
																/*返回-13表示add port err，返回-14表示delete port err*/
																/*返回-15表示slot sync vlan info err，返回-16表示slot have not such cpu or port*/
																/*返回-17表示error*/
{
	if((NULL == oper) || (NULL == vlanid) || (NULL == slotid) || (NULL == cpu) || (NULL == port))
		return 0;
	
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	int ret = 0;
	unsigned int op_ret = 0;	
	boolean isbond = FALSE;
	unsigned short 	vlanId = 0;
	unsigned short 	cpu_no = 0, cpu_port_no = 0;
	unsigned short slot_id =0, slot = 0;
	int function_type = -1;
	char file_path[64] = {0};	
	int local_slot_id = get_product_info(PRODUCT_LOCAL_SLOTID);
    int slot_count = get_product_info(SLOT_COUNT_FILE);
	void *connection = NULL;
	int retu = 0;

    if((local_slot_id<0)||(slot_count<0))
    {
		return -1;			
    }
	
    /* bond or unbond */
	if(0 == strncmp(oper,"bond",strlen(oper))) {
		isbond = TRUE;
	}
	else if (0 == strncmp(oper,"unbond",strlen(oper))) {
		isbond= FALSE;
	}
	else {
		return -2;
	}

	/*get vlan ID*/
	ret = parse_vlan_no((char*)vlanid,&vlanId);
	if (NPD_FAIL == ret) 
	{
		return -3;
	}
	if((vlanId < 2) || (vlanId > 4093))
	{
		return -3;
	}	
    /*get dist slot */
	ret = parse_single_param_no((char*)slotid,&slot_id);
	if(NPD_SUCCESS != ret) {
		return -4;
	}
    if(slot_id > slot_count)
    {
		return -5;		
    }
	if((slot_id < 1) || (slot_id > SLOT_MAX_NUM))
	{
		return -5;
	}

	/* get cpu no */
	ret = parse_single_param_no((char*)cpu,&cpu_no);
	if(NPD_SUCCESS != ret) {
		return -4;
	}		
	if((cpu_no < 1) || (cpu_no > 2))
	{
		return -6;
	}

	/* get cpu prot number */
	ret = parse_single_param_no((char*)port,&cpu_port_no);
	if(NPD_SUCCESS != ret) {
		return -4;
	}
	if((cpu_port_no < 1) || (cpu_port_no > 8))
	{
		return -7;
	}
	/* change no to index, for hardware use */
    cpu_no = cpu_no -1;
    cpu_port_no = cpu_port_no -1;
	
    /* Check if the dist slot is exist */
   	/*if(slot_id != local_slot_id)
   	{
        if(NULL == dbus_connection_dcli[slot_id]->dcli_dbus_connection)
        {
		    vty_out(vty,"Can not connect to slot %d, please check! \n",slot_id);	
		    return CMD_SUCCESS;			
        }		
   	}*/
    
    /* Check if the dist slot is AC board */
	sprintf(file_path,"/dbm/product/slot/slot%d/function_type", slot_id);
	function_type = get_product_info(file_path);	
	if((function_type&AC_BOARD)!=AC_BOARD)
	{
		return -8;		
	}

   	/* if unbond, send to dist slot before unbond, check interface ve exist or not. */	
    if(isbond == 0)
    {
        query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
        									NPD_DBUS_VLAN_OBJPATH ,	\
        									NPD_DBUS_VLAN_INTERFACE ,	\
        									NPD_DBUS_VLAN_EXIST_INTERFACE_UNDER_VLAN_TO_SLOT_CPU);
        	
        dbus_error_init(&err);

        dbus_message_append_args(query,
        						DBUS_TYPE_UINT16,&vlanId,
        						DBUS_TYPE_UINT16,&slot_id,
						        DBUS_TYPE_UINT16,&cpu_no,
        						DBUS_TYPE_UINT16,&cpu_port_no,
        						DBUS_TYPE_INVALID);

        reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);				
        		
        dbus_message_unref(query);
        	
        if (NULL == reply) {
        	return 0;
        }

        if (dbus_message_get_args ( reply, &err,DBUS_TYPE_UINT32, &op_ret,DBUS_TYPE_INVALID)) 
        {
			/* ve sub-intf is exist */
    		if(VLAN_RETURN_CODE_ERR_NONE == op_ret) 
            {
                //vty_out(vty,"ve%02d%c%d.%d is exist,no interface first!\n",slot_id,(cpu_no == 0)?'f':'s',(cpu_port_no+1),vlanId);			
        		return -9;        	
			}
			else if(VLAN_RETURN_CODE_ERR_HW == op_ret)  /* ve sub-intf is not exist */
			{
                /*NULL;*/
			}			
			else if(VLAN_RETURN_CODE_VLAN_NOT_BONDED == op_ret)
			{
        		return -10;				
			}
			else
			{
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
    }

	/* send to dist slot first, bond or unbond */
	{
		slot = slot_id;    /* dist slot */
     	/*once bad param,it'll NOT sed message to NPD*/
    	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
    										NPD_DBUS_VLAN_OBJPATH ,	\
    										NPD_DBUS_VLAN_INTERFACE ,	\
    										NPD_DBUS_VLAN_METHOD_BOND_VLAN_TO_SLOT_CPU);
    	
    	dbus_error_init(&err);

    	dbus_message_append_args(query,
     							 DBUS_TYPE_BYTE,&isbond,
    							 DBUS_TYPE_UINT16,&vlanId,
    							 DBUS_TYPE_UINT16,&slot_id,
        						 DBUS_TYPE_UINT16,&cpu_no,
        						 DBUS_TYPE_UINT16,&cpu_port_no,    							 
    							 DBUS_TYPE_INVALID);

		connection = NULL;
		if(SNMPD_DBUS_SUCCESS != get_slot_dbus_connection(slot, &connection, SNMPD_INSTANCE_MASTER_V3))
    	{
    		if(slot == local_slot_id)
    		{
                reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
    		}
    		else 
			{	
			   	//vty_out(vty,"Can not connect to slot:%d \n",slot);
        		return 0;				
			}	
        }
    	else
    	{
            reply = dbus_connection_send_with_reply_and_block (connection,query,-1, &err);				
    	}
    		
    	dbus_message_unref(query);
    	
    	if (NULL == reply) {
    		return 0;
    	}

    	if (dbus_message_get_args ( reply, &err,DBUS_TYPE_UINT32, &op_ret,DBUS_TYPE_INVALID)) 
    	{
			if(VLAN_RETURN_CODE_ERR_NONE == op_ret) 
        	{
				/*if(isbond == 1)
				{
           		    vty_out(vty,"Bond vlan %d to slot %d OK\n",vlanId,slot);
				}
				else
				{
					vty_out(vty,"Unbond vlan %d to slot %d OK\n",vlanId,slot);

				}*/		
				retu = 1;
            	/*return CMD_SUCCESS;		*/
				/* can not return, for next slot update bondinfo */
            }
        	else 
        	{					
				if(VLAN_RETURN_CODE_VLAN_NOT_EXISTS == op_ret)
				{
					return -11;		
				}
				else if(VLAN_RETURN_CODE_VLAN_NOT_BONDED == op_ret)
				{
					return -10;								
				}
				else if(VLAN_RETURN_CODE_VLAN_ALREADY_BOND == op_ret)
				{
					return -12;								
				}
				else if(VLAN_RETURN_CODE_VLAN_BOND_ERR == op_ret)
				{
					return -13;								
				}
				else if(VLAN_RETURN_CODE_VLAN_UNBOND_ERR == op_ret)
				{
					return -14;								
				}
				else if(VLAN_RETURN_CODE_VLAN_SYNC_ERR == op_ret)
				{
					return -15;								
				}
				else if(VLAN_RETURN_CODE_ERR_HW == op_ret)
				{
					return -16;								
				}				
				else
				{
					return -17;														
				}
        	}
		}
    	else 
    	{
    		if (dbus_error_is_set(&err)) 
    		{
    			dbus_error_free(&err);
    		}
    		return 0;					
    	}
    	dbus_message_unref(reply);
	}
	/* if dist slot is ok, send to every other slot, update the g_vlanlist[] */
    for(slot=1; slot<=slot_count; slot++)
    {
		if(slot == slot_id)     /* jump the dist slot */
		{
			continue;
		}
     	/*once bad param,it'll NOT sed message to NPD*/
    	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
    										NPD_DBUS_VLAN_OBJPATH ,	\
    										NPD_DBUS_VLAN_INTERFACE ,	\
    										NPD_DBUS_VLAN_METHOD_BOND_VLAN_TO_SLOT_CPU);
    	
    	dbus_error_init(&err);

    	dbus_message_append_args(query,
     							 DBUS_TYPE_BYTE,&isbond,
    							 DBUS_TYPE_UINT16,&vlanId,
    							 DBUS_TYPE_UINT16,&slot_id,
        						 DBUS_TYPE_UINT16,&cpu_no,
        						 DBUS_TYPE_UINT16,&cpu_port_no,    							 
    							 DBUS_TYPE_INVALID);

		connection = NULL;
		if(SNMPD_DBUS_SUCCESS != get_slot_dbus_connection(slot, &connection, SNMPD_INSTANCE_MASTER_V3))
    	{
    		if(slot == local_slot_id)
    		{
                reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
    		}
    		else 
			{	
			   	//vty_out(vty,"Can not connect to slot:%d \n",slot);	
				continue;
			}	
        }
    	else
    	{
            reply = dbus_connection_send_with_reply_and_block (connection,query,-1, &err);				
    	}
    		
    	dbus_message_unref(query);
    	
    	if (NULL == reply) {
    		return 0;
    	}

    	if (dbus_message_get_args ( reply, &err,DBUS_TYPE_UINT32, &op_ret,DBUS_TYPE_INVALID)) 
    	{
	
    		/* (VLAN_RETURN_CODE_ERR_NONE == op_ret) */
    		if(VLAN_RETURN_CODE_ERR_NONE == op_ret) 
			{
    			/* update the bond_slot in g_vlanlist[v_index] */
				retu = 1;
			}
    		else
    		{
           		//vty_out(vty,"update bondinfo of vlan %d on slot %d error\n",vlanId,slot);
				if(VLAN_RETURN_CODE_VLAN_NOT_EXISTS == op_ret)
				{
					retu = -11;
				}
				else if(VLAN_RETURN_CODE_VLAN_SYNC_ERR == op_ret)
				{
            		retu = -15;
				}
				else
				{
            		retu = -17;
				}
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
    }
    return retu;
}


#ifdef __cplusplus
}
#endif


