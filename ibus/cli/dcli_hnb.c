#include <string.h>
#include <dbus/dbus.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <zebra.h>
#include "../../quagga/debsrc/quagga-0.99.5/lib/command.h"
#include "../../ibus/app/ranapproxy/iuh/Iuh_log.h"
#include "../../dcli/src/lib/dcli_main.h"
#include "../../accapi/iuh/Iuh.h"
#include "../../accapi/iuh/mapi_hnb.h"
#include "../mapi/mapi_hnb.h"
#include "../../accapi/dbus/iuh/IuhDBusDef.h"
#include <dirent.h>
#include "dcli_hnb.h"
#include "../app/ranapproxy/iuh/Iuh_DBus_handler.h"
#include "../../accapi/dbus/hmd/HmdDbusDef.h"

#define MAX_STR_LEN			(1024*1024)
#define SHOW_STR_LEN    32          //book add, 2011-10-09
struct cmd_node iuh_node = 
{
	IUH_NODE,
	"%s(config-iuh)# ",
	1
};
struct cmd_node hansi_iuh_node =
{
	HANSI_IUH_NODE,
	"%s(hansi-iuh %d-%d)# ",
};
struct cmd_node local_hansi_iuh_node =
{
	LOCAL_HANSI_IUH_NODE,
	"%s(local_hansi-iuh %d-%d)# ",
};

void ReInitFemtoDbusConnection(DBusConnection **dcli_dbus_connection,int slot_id,int distributFag)
{
	if(distributFag && dbus_connection_dcli[slot_id] && dbus_connection_dcli[slot_id]->dcli_dbus_connection)
	{
		*dcli_dbus_connection = dbus_connection_dcli[slot_id]->dcli_dbus_connection;
	}
	else
	{
		*dcli_dbus_connection = dcli_dbus_connection_local;
	}
}

int parse_int_ID(char* str,unsigned int* ID){
	char *endptr = NULL;
	char c;
	c = str[0];
	if (c>='0'&&c<='9'){
		*ID= strtoul(str,&endptr,10);
		if((c=='0')&&(str[1]!='\0')){
			 return -1;
		}
		else if((endptr[0] == '\0')||(endptr[0] == '\n')){
			return 0;
		}
		else{
			return -1;
		}
	}
	else{
		return -1;
	}
}


void FREEHNBLIST(HNBLIST *hnblist)
{
    if(hnblist == NULL) return;
    int i;
    int num = hnblist->hnbcount;
    for(i = 0; i < num; i++){
        IUH_FREE_OBJECT(hnblist->HNB_LIST[i]);
    }
    IUH_FREE_OBJECT(hnblist);
    return;
}

/*****************************************************
** DISCRIPTION:
**          set char[] to int
** INPUT:
**          char[]
** OUTPUT:
**          null
** RETURN:
**          int value
*****************************************************/

int dcli_string_to_int(char *my_str){
    int my_id = *((int*)my_str);
    return my_id;
}


/*****************************************************
** DISCRIPTION:
**          change the sequence of the str
** INPUT:
**          char[]
** OUTPUT:
**          NULL
** RETURN:
**          char *
** book add
*****************************************************/
char * dcli_hnb_change_str(unsigned char *my_str, int len){
    int i = 0;
    
    if(my_str == NULL) return "Error";

    if(len == 0) return "Error";

    for(i = 0; i < len; i++){
		my_str[i] = ((my_str[i] & 0x0f) << 4) | (my_str[i] >> 4);
    }

    return my_str;
}



/*****************************************************
** DISCRIPTION:
**          show str as hexdecimal
** INPUT:
**          char[]
** OUTPUT:
**          show string
** RETURN:
**          void
** book add
*****************************************************/
void dcli_hnb_show_str(const char * strname, unsigned char * my_str, int flag, int len){

    unsigned char *tmp;
    if(flag)
        tmp = dcli_hnb_change_str(my_str,len);
    else
        tmp = my_str;

    if((tmp == NULL) || (strcmp(tmp, "Error") == 0)){
        printf("Error\n");
        return;
    }
    
    int i = 0;
    printf("%s",strname);
    for(i = 0; i < len; i++){
        printf("%.2x",tmp[i]);
    }
    printf("\n");
    
    return;
}



/*****************************************************
** DISCRIPTION:
**          set char[] to long long
** INPUT:
**          char[]
** OUTPUT:
**          null
** RETURN:
**          int value
*****************************************************/

unsigned long long dcli_string_to_ull(char *my_str){
    unsigned long long my_id = *((unsigned long long*)my_str);
    return my_id;
}



/*****************************************************
** DISCRIPTION:
**          free rab memory
** INPUT:
**          rab
** OUTPUT:
**          null
** RETURN:
**          void
*****************************************************/
void dcli_free_rab(Iu_RAB *myRab)
{
    if(myRab == NULL) return;

    if(myRab->pdp_type_list == NULL) return;

    int i = 0;
    struct pdpTypeInfo *tmp_info;
    tmp_info = myRab->pdp_type_list->pdp_type_info;
    for(i = 0; i < myRab->pdp_type_list->count; i++){
        if(tmp_info != NULL){
            myRab->pdp_type_list->pdp_type_info = tmp_info->next;
            tmp_info->next = NULL;
            IUH_FREE_OBJECT(tmp_info);
        }
    }

    IUH_FREE_OBJECT(myRab->pdp_type_list);
    IUH_FREE_OBJECT(myRab);
    return;
}
int dcli_check_imsi(unsigned char* imsi)
{
	int imsi_len = strlen(imsi);
	if(imsi_len != IMSI_DIGIT_LEN)
		return WRONG_LEN;
	int i = 0;
	for(i=0; i<IMSI_DIGIT_LEN; i++)
	{
		if(imsi[i]<'0' || imsi[i]>'9')
			return WRONG_CHARACTER;
	}
	return CORRECT_FORMAT;
}

DEFUN(config_iuh_mode_cmd_func,
		config_iuh_mode_cmd,
		"config iuh",
		CONFIG_STR
		"iuh config node\n"
     )
{
	int ret = 0;
	int islocal = 1;
	int slot_id = HostSlotId;
	int insid = 0;
	if(vty->node == CONFIG_NODE){
		insid = 0;
	}else if(vty->node == HANSI_NODE){
		insid = vty->index;
		islocal = vty->local;
		slot_id = vty->slotindex;
	}else if(vty->node == LOCAL_HANSI_NODE){
		insid = vty->index;
		islocal = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dbus_connection = NULL;
	ReInitFemtoDbusConnection(&dbus_connection, slot_id, distributFag);
	ret = mapi_femto_service_state_check(insid, islocal, slot_id, 0, dbus_connection, HMD_DBUS_CONF_METHOD_FEMTO_SERVICE_CHECK);
	if(ret == HMD_DBUS_SUCCESS)
	{
		if(vty->node == CONFIG_NODE)
		{
			vty->node = IUH_NODE;
		}
		else if(vty->node == HANSI_NODE)
		{
			vty->node = HANSI_IUH_NODE;
		}
		else if(vty->node == LOCAL_HANSI_NODE)
		{
			vty->node = LOCAL_HANSI_IUH_NODE;
		}
	}
	else
		vty_out(vty, "<error> you should enable iuh first!!\n");

	return CMD_SUCCESS;
}


DEFUN(set_auto_hnb_binding_interface_func,
	  set_auto_hnb_binding_interface_cmd,
	  "set auto_hnb_login interface (add|del) IFNAME",
	  "auto_hnb_login_binding_interface config\n"
	  "auto_hnb_login_binding_interface\n"
	  "auto_hnb_login_binding_interface add/del\n"
	  "auto_hnb_login_binding_interface IFNAME\n"
	 )
{
	int ret = 0;
	int len = 0;
	char *name;
	unsigned char policy = 2;

	len = strnlen(argv[1],IF_NAME_LEN);

	if(len > IF_NAME_LEN)
	{		
		vty_out(vty,"<error> interface name is too long,should be no more than 15\n");
		return CMD_SUCCESS;
	}
	if (!strcmp(argv[0],"add"))
	{
		policy = 1;	
	}
	else if (!strcmp(argv[0],"del"))
	{
		policy = 0;	
	}
	else
	{
		vty_out(vty,"<error> input patameter only with 'add'or 'del'\n");
		return CMD_SUCCESS;
	}	
	name = (char*)malloc(len+1);
	memset(name, 0, len+1);
	memcpy(name, argv[1], len); 

	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if(vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dbus_connection = NULL;
	ReInitFemtoDbusConnection(&dbus_connection, slot_id, distributFag);
    ret = mapi_iuh_auto_hnb_login(index, localid, name, policy, dbus_connection, IUH_DBUS_CONF_METHOD_SET_IUH_DYNAMIC_HNB_LOGIN_INTERFACE);

    if(ret == 0)
	{
		vty_out(vty,"set iuh_dynamic_hnb_login_interface %s %s successfully\n",argv[0],argv[1]);
	}
	else if(ret == SWITCH_IS_DISABLE)
		vty_out(vty,"<error> auto hnb login switch is enable,you should disable it first\n");
	else if(ret == APPLY_IF_FAIL)
		vty_out(vty,"<error> interface %s error, no index or interface down\n",argv[1]);
	else
		vty_out(vty,"<error>  interface %s error\n",argv[1]);
	
	IUH_FREE_OBJECT(name);
	return CMD_SUCCESS;			
}


DEFUN(set_iuh_daemonlog_debug_open_func,
	  set_iuh_daemonlog_debug_open_cmd,
	  "set iuh daemonlog (default|dbus|hnbinfo|mb|all) debug (open|close)",
	  "iuh config\n"
	  "iuh daemonlog config\n"
	  "iuh daemonlog debug open|close\n"
	 )
{
	int ret;
    unsigned int daemonlogtype;
    unsigned int daemonloglevel;

	if (!strcmp(argv[0],"default")){
		daemonlogtype = IUH_DEFAULT;	
	}
	else if (!strcmp(argv[0],"dbus")){
		daemonlogtype = IUH_DBUS;	
	}
	else if (!strcmp(argv[0],"hnbinfo")){
		daemonlogtype = IUH_HNBINFO;	
	}
	else if (!strcmp(argv[0],"mb")){
		daemonlogtype = IUH_MB;	
	}
	else if (!strcmp(argv[0],"all")){
		daemonlogtype = IUH_ALL;	
	}
	else{
		vty_out(vty,"<error> input patameter should only be default|dbus|80211|1x|wpa|wapi|all\n");
		return CMD_SUCCESS;
	}
	
	if (!strcmp(argv[1],"open")){
		daemonloglevel = 1;	
	}else if (!strcmp(argv[1],"close")){
		daemonloglevel = 0;	
	}else{
		vty_out(vty,"<error> input patameter should only be 'open' or 'close'\n");
		return CMD_SUCCESS;
	}
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	if(vty->node == IUH_NODE){
		index = 0;
	}else if(vty->node == HANSI_IUH_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if(vty->node == LOCAL_HANSI_IUH_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dbus_connection = dcli_dbus_connection;
	ReInitFemtoDbusConnection(&dbus_connection, slot_id, distributFag);
	ret = mapi_set_iuh_daemonlog(index, localid, daemonlogtype, daemonloglevel, dbus_connection, IUH_DBUS_SECURITY_METHOD_SET_IUH_DAEMONLOG_DEBUG);

	if(ret == 0){
		vty_out(vty,"iuh set daemonlog debug %s successfully\n",argv[0]);
	}				
	else{
		vty_out(vty,"<error>  %d\n",ret);
	}
	
	return CMD_SUCCESS;			
}


DEFUN(delete_hnb_by_hnbid_func,
	  delete_hnb_by_hnbid_cmd,
	  "delete hnb HNBID",
	  CONFIG_STR
	  "delete hnb hnbid\n"
	  "delete hnb by id\n"
	 )
{
	int ret;
	unsigned int hnb_id;
    ret = parse_int_ID((char*)argv[0], &hnb_id);
	if(ret != 0){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}
	if(hnb_id >= HNB_DEFAULT_NUM_AUTELAN || hnb_id == 0){
		vty_out(vty,"<error> hnb id should be 1 to %d\n",HNB_DEFAULT_NUM_AUTELAN);
		return CMD_SUCCESS;
	}
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	if(vty->node == IUH_NODE){
		index = 0;
	}else if(vty->node == HANSI_IUH_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if(vty->node == LOCAL_HANSI_IUH_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dbus_connection = dcli_dbus_connection;
	ReInitFemtoDbusConnection(&dbus_connection, slot_id, distributFag);
	ret = mapi_delete_hnb_id(index, localid, hnb_id, dbus_connection, IUH_DBUS_SECURITY_METHOD_DELETE_HNB_BY_HNBID);

	if(ret == 0){
		vty_out(vty,"iuh delete hnb %d successfully\n",hnb_id);
	}		
	else if(ret == HNB_ID_INVALID){
		vty_out(vty,"<error> hnb id does not exist\n");
	}
	else if(ret == HNB_ID_LARGE_THAN_MAX)
	{
		vty_out(vty,"<error> input hnb id should be 1 to %d\n",HNB_DEFAULT_NUM_AUTELAN);
	}
	else if(ret == HNB_ID_BE_REGISTERED)
	{
		vty_out(vty,"<error> hnb is registed, can not delete\n");
	}
	else if(ret == HNB_ID_UESD_BY_UE)
	{
		vty_out(vty,"<error> hnb is used by users, can not delete\n");
	}
	else{
		vty_out(vty,"<error>  %d\n",ret);
	}
	
	return CMD_SUCCESS;			
}


DEFUN(show_hnb_info_by_hnbid_func,
	  show_hnb_info_by_hnbid_cmd,
	  "show hnb HNBID",
	  "HNB information\n"
	  "HNB id that you want to show\n"
	 )
{	int ret;
	unsigned int hnb_id = 0;
    Iuh_HNB *HNBINFO = NULL;

	ret = parse_int_ID((char*)argv[0], &hnb_id);
	if(ret != 0){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(hnb_id >= HNB_DEFAULT_NUM_AUTELAN || hnb_id == 0){
		vty_out(vty,"<error> hnb id should be 1 to %d\n",HNB_DEFAULT_NUM_AUTELAN);
		return CMD_SUCCESS;
	}

	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	if(vty->node == IUH_NODE){
		index = 0;
	}else if(vty->node == HANSI_IUH_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if(vty->node == LOCAL_HANSI_IUH_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dbus_connection = dcli_dbus_connection;
	ReInitFemtoDbusConnection(&dbus_connection, slot_id, distributFag);
    HNBINFO = mapi_get_hnb_info_by_hnbid(index, localid, hnb_id, &ret, dbus_connection, IUH_DBUS_HNB_METHOD_SHOW_HNBINFO_BY_HNBID);

    if((ret == 0) && (HNBINFO != NULL)){
        vty_out(vty,"==============================================================================\n");
        
        vty_out(vty,"HNB NAME : %s\n",(char*)HNBINFO->HNBNAME);
        vty_out(vty,"HNB ID : %d\n",HNBINFO->HNBID);
        vty_out(vty,"HNB IP : %s\n",(char*)HNBINFO->HNBIP);
        vty_out(vty,"HNB BindingIFName : %s\n",(char*)HNBINFO->BindingIFName);
        vty_out(vty,"HNB BindingSystemIndex : %d\n",HNBINFO->BindingSystemIndex);
        vty_out(vty,"HNB state : %s\n",(HNBINFO->state == UNREGISTED)?"UNREGISTED":"REGISTED");
        vty_out(vty,"HNB socket : %d\n",HNBINFO->socket);
        vty_out(vty,"HNB Accessmode : %d\n",HNBINFO->accessmode);
        vty_out(vty,"HNB HnbIdentity : %s\n",(char*)HNBINFO->HnbIdentity);
        if(HNBINFO->HnbLocationInfo.macroCoverageInfo != NULL){
            switch(HNBINFO->HnbLocationInfo.macroCoverageInfo->present){
                case 1:
                {
                    vty_out(vty,"HNB MacroCoverageInfo Type : %s\n","UTRANCellID");
	                //vty_out(vty,"UTRANCellID LAC : ");
	                dcli_hnb_show_str("UTRANCellID LAC : ", HNBINFO->HnbLocationInfo.macroCoverageInfo->choice.uTRANCellID.lAC, 0, LAC_LEN); 
	                //vty_out(vty,"UTRANCellID RAC : ");
	                dcli_hnb_show_str("UTRANCellID RAC : ", HNBINFO->HnbLocationInfo.macroCoverageInfo->choice.uTRANCellID.rAC, 0, RAC_LEN); 
	                //vty_out(vty,"UTRANCellID PLMNidentity : "); 
	                dcli_hnb_show_str("UTRANCellID PLMNidentity : ", HNBINFO->HnbLocationInfo.macroCoverageInfo->choice.uTRANCellID.pLMNidentity, 1, PLMN_LEN); 
	                //vty_out(vty,"UTRANCellID CellIdentity : "); 
	                dcli_hnb_show_str("UTRANCellID CellIdentity : ", HNBINFO->HnbLocationInfo.macroCoverageInfo->choice.uTRANCellID.cellIdentity, 0, CELLID_LEN);
                    break;
                }
	            case 2:
	            {
	                vty_out(vty,"HNB MacroCoverageInfo Type : %s\n","GERANCellID");
	                //vty_out(vty,"GERANCellID PLMNidentity : ");
	                dcli_hnb_show_str("GERANCellID PLMNidentity : ", HNBINFO->HnbLocationInfo.macroCoverageInfo->choice.gERANCellID.pLMNidentity, 0, PLMN_LEN); 
	                //vty_out(vty,"GERANCellID LAC : ");
	                dcli_hnb_show_str("GERANCellID LAC : ", HNBINFO->HnbLocationInfo.macroCoverageInfo->choice.gERANCellID.lAC, 0, LAC_LEN);
	                //vty_out(vty,"GERANCellID CI : ");
	                dcli_hnb_show_str("GERANCellID CI : ", HNBINFO->HnbLocationInfo.macroCoverageInfo->choice.gERANCellID.cI, 0, CI_LEN);
	                break;
	            }
	            default:
	                break;
            }
        }
        if(HNBINFO->HnbLocationInfo.gographicalLocation != NULL){
            vty_out(vty,"HNB GographicalLocation :\n");
            vty_out(vty,"GographicalLocation LatitudeSign : %s\n", \
                (HNBINFO->HnbLocationInfo.gographicalLocation->GeographicalCoordinates.LatitudeSign == 0)?"north":"south");
            vty_out(vty,"GographicalLocation latitude : %d\n",HNBINFO->HnbLocationInfo.gographicalLocation->GeographicalCoordinates.latitude);
            vty_out(vty,"GographicalLocation longitude : %d\n",HNBINFO->HnbLocationInfo.gographicalLocation->GeographicalCoordinates.longitude);
            vty_out(vty,"GographicalLocation DirectionOfAltitude : %s\n", \
                (HNBINFO->HnbLocationInfo.gographicalLocation->AltitudeAndDirection.DirectionOfAltitude == 0)?"height":"depth");
            vty_out(vty,"GographicalLocation altitude : %d\n",HNBINFO->HnbLocationInfo.gographicalLocation->AltitudeAndDirection.altitude);
        }
        usleep(1000);
        
        //vty_out(vty,"HNB LAC : ");
        dcli_hnb_show_str("HNB LAC : ", HNBINFO->lac, 0, LAC_LEN);
        //vty_out(vty,"HNB RAC : ");
        dcli_hnb_show_str("HNB RAC : ", HNBINFO->rac, 0, RAC_LEN);
        //vty_out(vty,"HNB SAC : ");
        dcli_hnb_show_str("HNB SAC : ", HNBINFO->sac, 0, SAC_LEN);
        //vty_out(vty,"HNB CellId : ");
        dcli_hnb_show_str("HNB CellId : ", HNBINFO->cellId, 0, CELLID_LEN);
        //vty_out(vty,"HNB PLMNId : ");
        dcli_hnb_show_str("HNB PLMNId : ", HNBINFO->plmnId, 1, PLMN_LEN);
        vty_out(vty,"HNB RncId : %d\n",HNBINFO->rncId);

        vty_out(vty,"HNB current_UE_number : %d\n",HNBINFO->current_UE_number);

		vty_out(vty,"IMSI White List :");
		IMSIWHITELIST *head = HNBINFO->imsi_white_list;
		if(head == NULL)
			vty_out(vty,"NONE\n");
		else
		{			
			vty_out(vty,"\n");
			unsigned char *tmp;
			int i = 0;
			while(head != NULL)
			{
				//vty_out(vty,"%s\n",head->imsi);
				tmp = dcli_hnb_change_str(head->imsi, IMSI_LEN);
			    for(i = 0; i < IMSI_LEN; i++){
			        vty_out(vty,"%.2x",tmp[i]);
			    }
				vty_out(vty,"\n");
				head = head->next;
			}
		}
        
        vty_out(vty,"==============================================================================\n");

		dcli_free_hnb_info(HNBINFO);
    }
    else if(ret == HNB_ID_INVALID){
		vty_out(vty,"<error> hnb id does not exist\n");
	}
	else if(ret == HNB_ID_LARGE_THAN_MAX)
	{
		vty_out(vty,"<error> input hnb id should be 1 to %d\n",HNB_DEFAULT_NUM_AUTELAN);
	}
	else 
		vty_out(vty,"<error>  %d\n",ret);
	
	return CMD_SUCCESS;
}


DEFUN(show_hnb_list_func,
	  show_hnb_list_cmd,
	  "show hnb list",
	  CONFIG_STR
	  "show hnb list\n"
	  "show hnb list\n"
	 )
{	
    int i,ret;
    HNBLIST *hnblist = NULL;
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	if(vty->node == IUH_NODE){
		index = 0;
	}else if(vty->node == HANSI_IUH_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if(vty->node == LOCAL_HANSI_IUH_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dbus_connection = dcli_dbus_connection;
	ReInitFemtoDbusConnection(&dbus_connection, slot_id, distributFag);
    hnblist = mapi_get_hnb_list(index, localid, dbus_connection, IUH_DBUS_HNB_METHOD_SHOW_HNB_LIST);

    if(hnblist != NULL){
        vty_out(vty,"hnb list summary:\n");	
        vty_out(vty,"%d HNBs\n",hnblist->hnbcount);
        vty_out(vty,"==============================================================================\n");
    	vty_out(vty,"%-5s %-17s %-17s %-17s\n","HNBID","HNBNAME","WTPIP","HNBSTATE");

        for(i = 0; i < hnblist->hnbcount; i++){
            vty_out(vty,"%-5d %-17s %-17s %-17s \n",\
            hnblist->HNB_LIST[i]->hnb_id, hnblist->HNB_LIST[i]->hnb_name, hnblist->HNB_LIST[i]->hnb_ip, \
            (hnblist->HNB_LIST[i]->hnb_state == 0) ? "UNREGISTED" : "REGISTED");
        }
        FREEHNBLIST(hnblist);
        vty_out(vty,"==============================================================================\n");
    }
	else 
		vty_out(vty,"<error>  %d\n",ret);

	return CMD_SUCCESS;
}


DEFUN(show_ue_info_by_hnbid_func,
	  show_ue_info_by_ueid_cmd,
	  "show ue UEID",
	  "UE information\n"
	  "UE id that you want to show\n"
	 )
{	int ret;
	unsigned int ue_id = 0;
    Iuh_HNB_UE *UEINFO = NULL;
    Iu_RAB *p1 = NULL;
    Iu_RAB *RABINFO = NULL;
    int i = 0;
	
	ret = parse_int_ID((char*)argv[0], &ue_id);
	if(ret != 0){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(ue_id >= UE_MAX_NUM_AUTELAN || ue_id == 0){
		vty_out(vty,"<error> ue id should be 1 to %d\n",UE_MAX_NUM_AUTELAN);
		return CMD_SUCCESS;
	}
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	if(vty->node == IUH_NODE){
		index = 0;
	}else if(vty->node == HANSI_IUH_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if(vty->node == LOCAL_HANSI_IUH_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dbus_connection = dcli_dbus_connection;
	ReInitFemtoDbusConnection(&dbus_connection, slot_id, distributFag);
    UEINFO = mapi_get_ue_info_by_ueid(index, localid, ue_id, &ret, dbus_connection, IUH_DBUS_HNB_METHOD_SHOW_UEINFO_BY_UEID);

    if((ret == 0) && (UEINFO != NULL)){
        vty_out(vty,"==============================================================================\n");
        
        vty_out(vty,"UE ID : %d\n",UEINFO->UEID);
        vty_out(vty,"HNB ID : %d\n",UEINFO->HNBID);
        vty_out(vty,"UE Identity TYPE: %d\n",UEINFO->UE_Identity.present);
        usleep(1000);
        switch(UEINFO->UE_Identity.present){
            case UE_Identity_iMSI:
    	    {
    	        //vty_out(vty,"IMSI: ");
    	        dcli_hnb_show_str("IMSI: ", UEINFO->UE_Identity.choice.imsi, 1, IMSI_LEN);
    	        break;
    	    }
    	    case UE_Identity_tMSILAI:
    	    {
    	        //vty_out(vty,"TMSI: ");
    	        dcli_hnb_show_str("TMSI: ", UEINFO->UE_Identity.choice.tmsilai.tmsi, 0, TMSI_LEN);
    	        //vty_out(vty,"PLMNID: ");
    	        dcli_hnb_show_str("PLMNID: ", UEINFO->UE_Identity.choice.tmsilai.lai.plmnid, 0, PLMN_LEN);
    	        //vty_out(vty,"LAC: "); 
    	        dcli_hnb_show_str("LAC: ", UEINFO->UE_Identity.choice.tmsilai.lai.lac, 0, LAC_LEN);
    	        break;
    	    }
    	    case UE_Identity_pTMSIRAI:
    	    {
    	        //vty_out(vty,"PTMSI: ");
    	        dcli_hnb_show_str("PTMSI: ", UEINFO->UE_Identity.choice.ptmsirai.ptmsi, 0, TMSI_LEN);
    	        //vty_out(vty,"RAC: ");
    	        dcli_hnb_show_str("RAC: ", UEINFO->UE_Identity.choice.ptmsirai.rai.rac, 0, RAC_LEN);
    	        //vty_out(vty,"PLMNID: ");
    	        dcli_hnb_show_str("PLMNID: ", UEINFO->UE_Identity.choice.ptmsirai.rai.lai.plmnid, 0, PLMN_LEN);
    	        //vty_out(vty,"LAC: ");
    	        dcli_hnb_show_str("LAC: ", UEINFO->UE_Identity.choice.ptmsirai.rai.lai.lac, 0, LAC_LEN);
    	        break;
    	    }
    	    case UE_Identity_iMEI:
    	    {
    	        //vty_out(vty,"IMEI: ");
    	        dcli_hnb_show_str("IMEI: ", UEINFO->UE_Identity.choice.imei, 0, IMEI_LEN);
    	        break;
    	    }
    	    case UE_Identity_eSN:
    	    {
    	        //vty_out(vty,"ESN: ");
    	        dcli_hnb_show_str("ESN: ", UEINFO->UE_Identity.choice.esn, 0, ESN_LEN);
    	        break;
    	    }
    	    case UE_Identity_iMSIDS41:
    	    {
    	        vty_out(vty,"IMSIDS41: %s\n",UEINFO->UE_Identity.choice.imsids41);
    	        IUH_FREE_OBJECT(UEINFO->UE_Identity.choice.imsids41);
    	        break;
    	    }
    	    case UE_Identity_iMSIESN:
    	    {
    	        vty_out(vty,"IMSIDS41: %s\n",UEINFO->UE_Identity.choice.imsiesn.imsids41);
    	        //vty_out(vty,"ESN: ");
    	        dcli_hnb_show_str("ESN: ", UEINFO->UE_Identity.choice.imsiesn.esn, 0, ESN_LEN);
    	        IUH_FREE_OBJECT(UEINFO->UE_Identity.choice.imsiesn.imsids41);
    	        break;
    	    }
    	    case UE_Identity_tMSIDS41:
    	    {
    	        vty_out(vty,"TMSIDS41: %s\n",UEINFO->UE_Identity.choice.imsiesn.imsids41);
    	        IUH_FREE_OBJECT(UEINFO->UE_Identity.choice.tmsids41);
    	        break;
    	    }
    	    default:
    	        break;
        }
        vty_out(vty,"UE Context ID : %d\n",UEINFO->context_id);
        vty_out(vty,"UE Capabilities :\n");
        switch(UEINFO->Capabilities.accStrRelIndicator){
            case R99:
            {
                vty_out(vty,"AccStrRelIndicator :%s\n","R99");
                break;
            }
            case REL4:
            {
                vty_out(vty,"AccStrRelIndicator :%s\n","REL4");
                break;
            }
            case REL5:
            {
                vty_out(vty,"AccStrRelIndicator :%s\n","REL5");
                break;
            }
            case REL6:
            {
                vty_out(vty,"AccStrRelIndicator :%s\n","REL6");
                break;
            }
            case REL7:
            {
                vty_out(vty,"AccStrRelIndicator :%s\n","REL7");
                break;
            }
            case REL8_AND_BEYOND:
            {
                vty_out(vty,"AccStrRelIndicator :%s\n","REL8_AND_BEYOND");
                break;
            }
            default:
                break;
        }
        
        vty_out(vty,"CsgIndicator :%s\n",(UEINFO->Capabilities.csgIndicator == 0)?"CSG_CAPABLE":"NOT_CSG_CAPABLE");
        vty_out(vty,"CsgMemStatus :%s\n",(UEINFO->csgMemStatus == 0)?"MEMBER":"NOT_MEMBER");
        vty_out(vty,"UE state : %s\n",(UEINFO->state == UNREGISTED)?"UNREGISTED":"REGISTED");
        vty_out(vty,"Registration Cause : %s\n",(UEINFO->registrationCause == 0)?"EMERGENCY":"NORMAL" );

        vty_out(vty,"RAB Counts: %d\n", UEINFO->rab_count);
	#ifdef RAB_INFO
        if(UEINFO->rab_count != 0){
            vty_out(vty,"----------------------------------------\n");
        }
        RABINFO = UEINFO->UE_RAB;
        p1 = UEINFO->UE_RAB;
        for(i = 0; i < UEINFO->rab_count; i++){
            if(RABINFO != NULL){
                vty_out(vty,"RAB ID: %d\n", RABINFO->RABID[0]);
                vty_out(vty,"Nas Scr Indicator: %d\n", RABINFO->nAS_ScrIndicator[0]);
                vty_out(vty,"RAB Parameters List:\n");
                vty_out(vty,"   Delivery Order: %s\n", (RABINFO->rab_para_list.deliveryOrder == Requested)?"Request":"NotRequest");
                vty_out(vty,"   Max SDU Size: %d\n", RABINFO->rab_para_list.maxSDUSize);
                if(RABINFO->rab_para_list.rab_ass_indicator == Symmetric_bidirectional)
                    vty_out(vty,"   RAB Assoc Indicator: %s\n", "Symmetric bidirectional");
                else if(RABINFO->rab_para_list.rab_ass_indicator == Asymmetric_unidirectional_downlink)
                    vty_out(vty,"   RAB Assoc Indicator: %s\n", "Asymmetric Unidirectional Downlink");
                else if(RABINFO->rab_para_list.rab_ass_indicator == Asymmetric_unidirectional_uplink)
                    vty_out(vty,"   RAB Assoc Indicator: %s\n", "Asymmetric Unidirectional Uplink");
                else if(RABINFO->rab_para_list.rab_ass_indicator == Asymmetric_bidirectional)
                    vty_out(vty,"   RAB Assoc Indicator: %s\n", "Asymmetric Bidirectional");

                if(RABINFO->rab_para_list.traffic_class == Tfc_conversational)
                    vty_out(vty,"   Traffic Class: %s\n", "Traffic Conversational");
                else if(RABINFO->rab_para_list.traffic_class == Tfc_streaming)
                    vty_out(vty,"   Traffic Class: %s\n", "Traffic Streaming");
                else if(RABINFO->rab_para_list.traffic_class == Tfc_interactive)
                    vty_out(vty,"   Traffic Class: %s\n", "Traffic Interactive");
                else if(RABINFO->rab_para_list.traffic_class == Tfc_background)
                    vty_out(vty,"   Traffic Class: %s\n", "Traffic Background");
                
                vty_out(vty,"User Plane Mode: %s\n", (RABINFO->user_plane_info.user_plane_mode == Transparent_mode)?"Transparent Mode":"Support Mode For Predefined SDU Sizes");

                vty_out(vty,"Transport Layer Info: %s\n",(RABINFO->trans_layer_info.iu_trans_assoc.present == IuTransportAssociation_gTP_TEI)?"GTP_TEI":"BINDINGID");
                if(RABINFO->service_handover == Handover_to_GSM_should_be_performed)
                    vty_out(vty,"Service Handover: %s\n","Handover to GSM should be performed");
                else if(RABINFO->service_handover == Handover_to_GSM_should_not_be_performed)
                    vty_out(vty,"Service Handover: %s\n","Handover to GSM should not be performed");
                else if(RABINFO->service_handover == Handover_to_GSM_shall_not_be_performed)
                    vty_out(vty,"Service Handover: %s\n","Handover to GSM shall not be performed");

                vty_out(vty,"Is PS Domain:  %d\n",RABINFO->isPsDomain);

                if(RABINFO->isPsDomain == 1){
                    vty_out(vty,"Data Volume Report: %s\n", (RABINFO->data_vol_rpt == DataVolume_do_report)?"Data Volume do report":"Data Volume do not report");    
                    vty_out(vty,"DL GTP PDU Sequence Number:    %d\n", RABINFO->dl_GTP_PDU_SeqNum);
                    vty_out(vty,"UL GTP PDU Sequence Number:    %d\n", RABINFO->ul_GTP_PDU_seqNum);
                    vty_out(vty,"DL N PDU Sequence Number:    %d\n", RABINFO->dl_N_PDU_SeqNum);
                    vty_out(vty,"UL N PDU Sequence Number:    %d\n", RABINFO->ul_N_PDU_SeqNum);
                }
                p1->rab_next = RABINFO->rab_next;
                RABINFO->rab_next = NULL;
                dcli_free_rab(RABINFO);
                RABINFO = p1->rab_next;
            }
            else
                break;
        }
    #endif
        vty_out(vty,"==============================================================================\n");
    	IUH_FREE_OBJECT(UEINFO);
    }
    else if(ret == UE_ID_INVALID){
		vty_out(vty,"<error> ue id does not exist\n");
	}
	else if(ret == UE_ID_LARGE_THAN_MAX)
	{
		vty_out(vty,"<error> input ue id should be 1 to %d\n",UE_MAX_NUM_AUTELAN);
	}
	else 
		vty_out(vty,"<error>  %d\n",ret);
	
	return CMD_SUCCESS;
}


DEFUN(show_ue_list_func,
	  show_ue_list_cmd,
	  "show ue list",
	  CONFIG_STR
	  "show ue list\n"
	  "show ue list\n"
	 )
{	
    int i,ret;
    UELIST *uelist = NULL;
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	if(vty->node == IUH_NODE){
		index = 0;
	}else if(vty->node == HANSI_IUH_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if(vty->node == LOCAL_HANSI_IUH_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dbus_connection = dcli_dbus_connection;
	ReInitFemtoDbusConnection(&dbus_connection, slot_id, distributFag);
    uelist = mapi_get_ue_list(index, localid, dbus_connection, IUH_DBUS_HNB_METHOD_SHOW_UE_LIST);

    if(uelist != NULL){
        vty_out(vty,"ue list summary:\n");	
        vty_out(vty,"%d UEs\n",uelist->uecount);
        vty_out(vty,"==============================================================================\n");
    	vty_out(vty,"%-10s %-10s %-20s %-17s\n","UEID","HNBID","REGISTRATION_CAUSE","UESTATE");

        for(i = 0; i < uelist->uecount; i++){
            vty_out(vty,"%-10d %-10d %-20s %-17s \n",\
            uelist->UE_LIST[i]->ue_id, uelist->UE_LIST[i]->hnb_id, (uelist->UE_LIST[i]->reg_cause == 0) ? "EMERGENCY" : "NORMAL", \
            (uelist->UE_LIST[i]->ue_state== 0) ? "UNREGISTED" : "REGISTED");
        }
		dcli_free_ue_list(uelist);
        vty_out(vty,"==============================================================================\n");
    }
	else 
		vty_out(vty,"<error>  %d\n",ret);

	return CMD_SUCCESS;
}


DEFUN(delete_ue_by_ueid_func,
	  delete_ue_by_ueid_cmd,
	  "delete ue UEID",
	  CONFIG_STR
	  "delete ue ueid\n"
	  "delete ue by id\n"
	 )
{
	int ret;
	unsigned int ue_id;
    ret = parse_int_ID((char*)argv[0], &ue_id);
	if(ret != 0){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(ue_id >= UE_MAX_NUM_AUTELAN || ue_id == 0){
		vty_out(vty,"<error> ue id should be 1 to %d\n",UE_MAX_NUM_AUTELAN);
		return CMD_SUCCESS;
	}
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	if(vty->node == IUH_NODE){
		index = 0;
	}else if(vty->node == HANSI_IUH_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if(vty->node == LOCAL_HANSI_IUH_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dbus_connection = dcli_dbus_connection;
	ReInitFemtoDbusConnection(&dbus_connection, slot_id, distributFag);
	ret = mapi_delete_ue_id(index, localid, ue_id, dbus_connection, IUH_DBUS_SECURITY_METHOD_DELETE_UE_BY_UEID);

	if(ret == 0){
		vty_out(vty,"iuh delete ue %d successfully\n",ue_id);
	}		
	else if(ret == UE_ID_INVALID){
		vty_out(vty,"<error> ue id does not exist\n");
	}
	else if(ret == UE_ID_LARGE_THAN_MAX){
		vty_out(vty,"<error> input ue id should be 1 to %d\n",UE_MAX_NUM_AUTELAN);
	}
	else if(ret == UE_ID_BE_REGISTERED){
		vty_out(vty,"<error> ue is registed, can not delete\n");
	}
	else{
		vty_out(vty,"<error>  %d\n",ret);
	}
	
	return CMD_SUCCESS;			
}

DEFUN(set_asn_debug_switch_func,
	  set_asn_debug_switch_cmd,
	  "set asn1 debug switch (open|close)",
	  "set asn1 debug switch\n"
	  "set asn1 debug switch\n"
	  "open|close\n"
	 )
{
	int ret;
    unsigned int debug_switch;

	if (!strcmp(argv[0],"open")){
		debug_switch = 1;	
	}
	else if (!strcmp(argv[0],"close")){
		debug_switch = 0;	
	}
	else{
		vty_out(vty,"<error> input patameter should only be 'open' or 'close'\n");
		return CMD_SUCCESS;
	}
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	if(vty->node == IUH_NODE){
		index = 0;
	}else if(vty->node == HANSI_IUH_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if(vty->node == LOCAL_HANSI_IUH_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dbus_connection = dcli_dbus_connection;
	ReInitFemtoDbusConnection(&dbus_connection, slot_id, distributFag);
	ret = mapi_set_asn_debug_switch(index, localid, debug_switch, dbus_connection, IUH_DBUS_IUH_SET_ASN_DEBUG_SWITCH);

	if(ret == 0){
		vty_out(vty,"set asn1 debug switch %s successfully\n",argv[0]);
	}				
	else{
		vty_out(vty,"<error>  %d\n",ret);
	}
	
	return CMD_SUCCESS;			
}



DEFUN(set_rncid_func,
	  set_rncid_cmd,
	  "set rncid <1-65535>",
	  "set rncid 1-65535\n"
	  "1-65535\n"
	 )
{
	int ret;
    unsigned short int rncid;
    rncid = atoi(argv[0]);

	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	if(vty->node == IUH_NODE){
		index = 0;
	}else if(vty->node == HANSI_IUH_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if(vty->node == LOCAL_HANSI_IUH_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dbus_connection = dcli_dbus_connection;
	ReInitFemtoDbusConnection(&dbus_connection, slot_id, distributFag);
	ret = mapi_set_rncid(index, localid, rncid, dbus_connection, IUH_DBUS_IUH_SET_RNCID);

	if(ret == 0){
		vty_out(vty,"set rncid %d successfully\n",rncid);
	}				
	else{
		vty_out(vty,"<error>  %d\n",ret);
	}
	
	return CMD_SUCCESS;			
}

DEFUN(set_paging_optimize_switch_func,
	  set_paging_optimize_switch_cmd,
	  "set paging optimize by (imsi|lai) (open|close)",
	  "set paging optimize by IMSI or LAI\n"
	  "set paging optimize switch\n"
	  "only one option should be chosen\n"
	 )
{
	int ret;

    unsigned int optimize_switch = 0;
    unsigned int optimize_type = 0;     //0---imsi,  1---lai

	if (!strcmp(argv[0],"imsi")){
		optimize_type = 0;	
	}
	else if (!strcmp(argv[0],"lai")){
		optimize_type = 1;	
	}
	else{
		vty_out(vty,"<error> input patameter should only be 'imsi' or 'lai'\n");
		return CMD_SUCCESS;
	}

	if (!strcmp(argv[1],"open")){
		optimize_switch = 1;	
	}
	else if (!strcmp(argv[1],"close")){
		optimize_switch = 0;	
	}
	else{
		vty_out(vty,"<error> input patameter should only be 'open' or 'close'\n");
		return CMD_SUCCESS;
	}
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	if(vty->node == IUH_NODE){
		index = 0;
	}else if(vty->node == HANSI_IUH_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if(vty->node == LOCAL_HANSI_IUH_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dbus_connection = dcli_dbus_connection;
	ReInitFemtoDbusConnection(&dbus_connection, slot_id, distributFag);
	ret = mapi_set_paging_optimize_switch(index, localid, optimize_type, optimize_switch, dbus_connection, IUH_SET_PAGING_OPTIMIZE_SWITCH);

	if(ret == 0){
		vty_out(vty,"set paging optimize switch %s  %s successfully\n",argv[0],argv[1]);
	}				
	else{
		vty_out(vty,"<error>  %d\n",ret);
	}
	
	return CMD_SUCCESS;			
}
DEFUN(femto_acl_whitelist_func,
	  femto_acl_whitelist_cmd,
	  "(add|del) femto acl white_list hnb HNBID IMSI",
	  "(add|del) femto acl white_list hnb HNBID IMSI\n"
	  "femto acl white_list hnb HNBID IMSI\n"
	  "acl white_list hnb HNBID IMSI\n"
	  "white_list hnb HNBID IMSI\n"
	  "hnb HNBID IMSI\n"
	 )
{
	int ret;
	unsigned int op_type = 0;
	unsigned hnb_id = 0;
	unsigned char *imsi = NULL;

	if (!strcmp(argv[0],"add")){
		op_type = 0;	
	}
	else if (!strcmp(argv[0],"del")){
		op_type = 1;	
	}
	else{
		vty_out(vty,"<error> input patameter should only be 'add' or 'del'\n");
		return CMD_SUCCESS;
	}
	ret = parse_int_ID((char*)argv[1], &hnb_id);
	if(ret != 0){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(hnb_id >= HNB_DEFAULT_NUM_AUTELAN || hnb_id == 0){
		vty_out(vty,"<error> hnb id should be 1 to %d\n",HNB_DEFAULT_NUM_AUTELAN);
		return CMD_SUCCESS;
	}
	ret = dcli_check_imsi((char*)argv[2]);
	if(ret == WRONG_LEN){
		vty_out(vty,"IMSI's length must be %d!!\n",IMSI_DIGIT_LEN);
		return CMD_SUCCESS;
	}
	if(ret == WRONG_CHARACTER){
		vty_out(vty,"IMSI contains only digit!!\n");
		return CMD_SUCCESS;
	}
	imsi = (char*)malloc(strlen(argv[2])+1);
	memset(imsi, 0, strlen(argv[2])+1);
	memcpy(imsi, argv[2], strlen(argv[2])); 

	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	if(vty->node == IUH_NODE){
		index = 0;
	}else if(vty->node == HANSI_IUH_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if(vty->node == LOCAL_HANSI_IUH_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dbus_connection = dcli_dbus_connection;
	ReInitFemtoDbusConnection(&dbus_connection, slot_id, distributFag);
	ret = mapi_femto_acl_white_list(index, localid, op_type, hnb_id, imsi, dbus_connection, IUH_FEMTO_ACL_WHITE_LIST);

	if(ret == FEMTO_ACL_SUCCESS){
		vty_out(vty,"%s femto acl white_list hnb %d %s successfully\n",argv[0], hnb_id, argv[2]);
	}		
	else if(ret == HNB_ID_INVALID){
		vty_out(vty,"<error> hnb id does not exist\n");
	}
	else if(ret == HNB_ID_LARGE_THAN_MAX)
	{
		vty_out(vty,"<error> input hnb id should be 1 to %d\n",HNB_DEFAULT_NUM_AUTELAN);
	}
	else if(ret == MALLOC_ERROR)
	{
		vty_out(vty,"<error> ACL WHITE LIST memory allocation ERROR!!\n");
	}
	else if(ret == IMSI_EXIST_IN_LIST)
	{
		vty_out(vty,"<error> %s already in ACL WHITE LIST!!\n",argv[2]);
	}
	else if(ret == IMSI_NOT_EXIST_IN_LIST)
	{
		vty_out(vty,"<error> %s not in ACL WHITE LIST!!\n",argv[2]);
	}	
	IUH_FREE_OBJECT(imsi);
	return CMD_SUCCESS;			
}
DEFUN(femto_service_switch_cmd_func,
	  femto_service_switch_cmd,
	  "(iu|iuh) service (enable|disable)",
	  "iuh service\n"
	  "iuh service\n"
	  "iuh service switch\n"
	 )
{
	unsigned int service_type = 0;	//0-- iuh, 1-- iu
	unsigned int service_switch = 0;	//0-- disable, 1-- enable
	if(!strcmp(argv[0], "iu"))
	{
		service_type = 1;
	}
	else if(!strcmp(argv[0], "iuh"))
	{
		service_type = 0;
	}
	else
	{
		vty_out(vty,"<error> input parameter should only be 'iu' or 'iuh'\n");
		return CMD_SUCCESS;
	}
	if(!strcmp(argv[1], "enable"))
	{
		service_switch = 1;
	}
	else if(!strcmp(argv[1], "disable"))
	{
		service_switch = 0;
	}
	else
	{
		vty_out(vty,"<error> input parameter should only be 'enable' or 'disable'\n");
		return CMD_SUCCESS;
	}
	int ret = 0;	
	int islocal = 1;	//1--local hansi, 0--remote hansi
	int slot_id = HostSlotId;
	unsigned int index = 0;
	if(vty->node == HANSI_NODE){
		index = vty->index;
		islocal = vty->local;
		slot_id = vty->slotindex;
	}else if(vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		islocal = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dbus_connection = dcli_dbus_connection;
	ReInitFemtoDbusConnection(&dbus_connection, slot_id, distributFag);
	ret = mapi_femto_service_switch(index, islocal, slot_id, service_type, service_switch, dbus_connection, HMD_DBUS_CONF_METHOD_FEMTO_SERVICE_SWITCH);

	if(ret == HMD_DBUS_SUCCESS)
	{
		vty_out(vty,"%s service %s successfully\n",argv[0], argv[1]);
	}				
	else if(ret == HMD_DBUS_FEMTO_SERVICE_CONFLICT)
	{
		vty_out(vty,"%s service already %s\n",argv[0], argv[1]);
	}
	else if(ret == HMD_DBUS_HANSI_ID_NOT_EXIST)
	{
		vty_out(vty,"<error> Remote hansi is not EXIST!\n");
	}
	else if(ret == HMD_DBUS_LOCAL_HANSI_ID_NOT_EXIST)
	{
		vty_out(vty,"<error> Local hansi is not EXIST!\n");
	}
	else if(ret == HMD_DBUS_FEMTO_SERVICE_ERROR)
	{
		vty_out(vty,"<error>can't %s %s process!\n", argv[1], argv[0]);
	}
	else if(ret == HMD_DBUS_SLOT_ID_NOT_EXIST)
	{
		vty_out(vty,"<error>The Slot is not EXIST!\n");
	}
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
	return CMD_SUCCESS;			
}
DEFUN(iuh_specify_iu_cmd_func,
	  iuh_specify_iu_cmd,
	  "iuh_specify_iu slotid SLOTID insid INSID",
	  "iuh specify iu\n"
	  "iu slotid\n"
	  "iu slotid <1-16>\n"
	  "iu instance id\n"
	  "iu instance id <1-4>\n"
	 )
{
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	int ret = 0;
	unsigned int iu_slotid;
	unsigned int iu_insid;
	unsigned int type = 1;	//iu
	ret = parse_int_ID((char*)argv[0], &iu_slotid);
	if(ret != 0)
	{
		vty_out(vty,"<error> unknown slotid format\n");
		return CMD_SUCCESS;
	}
	//check slotid
	if(iu_slotid >= MAX_SLOT_NUM || iu_slotid < 0)
	{
		vty_out(vty,"<error> iu slotid should be 1 to %d\n",MAX_SLOT_NUM);
		return CMD_SUCCESS;
	}
	ret = parse_int_ID((char*)argv[1], &iu_insid);
	if(ret != 0)
	{
		vty_out(vty,"<error> unknown insid format\n");
		return CMD_SUCCESS;
	}
	//check insid
	if(iu_insid > IUH_MAX_INS_NUM || iu_insid < 0)
	{
		vty_out(vty,"<error> iu insid should be 1 to %d\n",IUH_MAX_INS_NUM);
		return CMD_SUCCESS;
	}
	
	int islocal = 1;	//1--local hansi, 0--remote hansi
	int slot_id = HostSlotId;
	unsigned int index = 0;
	if(vty->node == HANSI_IUH_NODE){
		index = vty->index;
		islocal = vty->local;
		slot_id = vty->slotindex;
	}else if(vty->node == LOCAL_HANSI_IUH_NODE){
		index = vty->index;
		islocal = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dbus_connection = dcli_dbus_connection;
	ReInitFemtoDbusConnection(&dbus_connection, iu_slotid, distributFag);
	//check iu
	query = dbus_message_new_method_call(HMD_DBUS_BUSNAME,HMD_DBUS_OBJPATH,HMD_DBUS_INTERFACE,HMD_DBUS_CONF_METHOD_FEMTO_SERVICE_CHECK);
	dbus_error_init(&err);
	dbus_message_append_args(query,
                            DBUS_TYPE_UINT32,&type,
                            DBUS_TYPE_UINT32,&iu_slotid,
                            DBUS_TYPE_UINT32,&iu_insid,
                            DBUS_TYPE_UINT32,&islocal,
                            DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_SUCCESS;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	dbus_message_unref(reply);
	if(ret == HMD_DBUS_SUCCESS)	//specify iuh
	{
		ReInitFemtoDbusConnection(&dbus_connection, slot_id, distributFag);
		char BUSNAME[PATH_LEN];
		char OBJPATH[PATH_LEN];
		char INTERFACE[PATH_LEN];
		ReInitFemtoDbusPath(islocal,index,IUH_DBUS_BUSNAME,BUSNAME);
		ReInitFemtoDbusPath(islocal,index,IUH_DBUS_OBJPATH,OBJPATH);
		ReInitFemtoDbusPath(islocal,index,IUH_DBUS_INTERFACE,INTERFACE);
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,IUH_DBUS_IUH_TIPC_INIT);
		
		dbus_error_init(&err);
		dbus_message_append_args(query,
								 DBUS_TYPE_UINT32,&islocal,
								 DBUS_TYPE_UINT32,&iu_slotid,
								 DBUS_TYPE_UINT32,&iu_insid,
								 DBUS_TYPE_INVALID);
		reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);	
		dbus_message_unref(query);
	
		if (NULL == reply) {
			printf("<error> failed get reply.\n");
			if (dbus_error_is_set(&err)) {
				printf("%s raised: %s",err.name,err.message);
				dbus_error_free(&err);
			}
			return CMD_SUCCESS;
		}	
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&ret);
	}
	if(ret == IUH_DBUS_SUCCESS)
	{
		vty_out(vty,"iuh specify iu %s %s successfully\n", argv[0], argv[1]);
	}				
	else if(ret == IUH_TIPC_SOCK_INIT_ERROR)
	{
		vty_out(vty,"IUH TIPC Socket Init Error!!\n");
	}
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
	return CMD_SUCCESS;			
}

int mapi_get_running_cfg_lib(int index)
{	
    DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	char *tmp_str = NULL;
	tmp_str = NULL;
	index = 0;
	int localid = 0;
    char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath(localid,index,IUH_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(localid,index,IUH_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath(localid,index,IUH_DBUS_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,IUH_DBUS_METHOD_SHOW_RUNNING_CFG);

	dbus_error_init(&err);
							 
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);

    if (NULL == reply){
		printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)){
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return -1;
	}

	if(dbus_message_get_args(reply, &err, DBUS_TYPE_STRING, &tmp_str, DBUS_TYPE_INVALID))
	{
		char _tmpstr[64];
		memset(_tmpstr, 0, 64);
		sprintf(_tmpstr, BUILDING_MOUDLE, "IUH");
		vtysh_add_show_string(_tmpstr);
		vtysh_add_show_string(tmp_str);
		dbus_message_unref(reply);
		return 0;
	}
	else
	{
		if(dbus_error_is_set(&err))
		{
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return 1;
	}
	
	return 0;	
}


void dcli_hnb_init(void) {
	install_node(&hansi_iuh_node,NULL,"HANSI_IUH_NODE");
	install_default(HANSI_IUH_NODE);
	install_node(&local_hansi_iuh_node,NULL,"LOCAL_HANSI_IUH_NODE");
	install_default(LOCAL_HANSI_IUH_NODE);
	install_element(HANSI_NODE,&config_iuh_mode_cmd);
	install_element(HANSI_NODE,&set_auto_hnb_binding_interface_cmd);
	install_element(LOCAL_HANSI_NODE,&config_iuh_mode_cmd);
	install_element(LOCAL_HANSI_NODE,&set_auto_hnb_binding_interface_cmd);
	install_element(HANSI_NODE,&femto_service_switch_cmd);
	install_element(LOCAL_HANSI_NODE,&femto_service_switch_cmd);
	/****************HANSI IUH NODE**********************************/
	install_element(HANSI_IUH_NODE,&set_iuh_daemonlog_debug_open_cmd);
	install_element(HANSI_IUH_NODE,&show_hnb_info_by_hnbid_cmd);
	install_element(HANSI_IUH_NODE,&show_hnb_list_cmd);
	install_element(HANSI_IUH_NODE,&delete_hnb_by_hnbid_cmd);
	install_element(HANSI_IUH_NODE,&show_ue_info_by_ueid_cmd);
	install_element(HANSI_IUH_NODE,&show_ue_list_cmd);
	install_element(HANSI_IUH_NODE,&delete_ue_by_ueid_cmd);
	install_element(HANSI_IUH_NODE,&set_asn_debug_switch_cmd);
	install_element(HANSI_IUH_NODE,&set_rncid_cmd);
	install_element(HANSI_IUH_NODE,&set_paging_optimize_switch_cmd);
	install_element(HANSI_IUH_NODE,&femto_acl_whitelist_cmd);
	install_element(HANSI_IUH_NODE,&iuh_specify_iu_cmd);
	/****************LOCAL HANSI IUH NODE****************************/
	install_element(LOCAL_HANSI_IUH_NODE,&set_iuh_daemonlog_debug_open_cmd);
	install_element(LOCAL_HANSI_IUH_NODE,&show_hnb_info_by_hnbid_cmd);
	install_element(LOCAL_HANSI_IUH_NODE,&show_hnb_list_cmd);
	install_element(LOCAL_HANSI_IUH_NODE,&delete_hnb_by_hnbid_cmd);
	install_element(LOCAL_HANSI_IUH_NODE,&show_ue_info_by_ueid_cmd);
	install_element(LOCAL_HANSI_IUH_NODE,&show_ue_list_cmd);
	install_element(LOCAL_HANSI_IUH_NODE,&delete_ue_by_ueid_cmd);
	install_element(LOCAL_HANSI_IUH_NODE,&set_asn_debug_switch_cmd);
	install_element(LOCAL_HANSI_IUH_NODE,&set_rncid_cmd);
	install_element(LOCAL_HANSI_IUH_NODE,&set_paging_optimize_switch_cmd);
	install_element(LOCAL_HANSI_IUH_NODE,&femto_acl_whitelist_cmd);
	install_element(LOCAL_HANSI_IUH_NODE,&iuh_specify_iu_cmd);
	
	return;
}


