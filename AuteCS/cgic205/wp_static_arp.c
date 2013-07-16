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
* wp_static_arp.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system contrl for port static arp
*
*
*******************************************************************************/
#include <stdio.h>
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include "ws_err.h"
#include "ws_usrinfo.h"

//add by kehao 03-17-2011   
#include "ws_sysinfo.h"
////////////////////////////////////////////

#include "ws_ec.h"
#include "ws_module_container.h"
#include "ws_static_arp.h"
#include "ws_dcli_portconf.h"
#include "ws_dcli_interface.h"

#define _DEBUG	0







typedef struct{
	char port_num[PORT_NUM_STR_LEN];
	char mac[MAC_STR_LEN];
	char ipaddr[IPADDR_STR_LEN];
	char vlanID[VLANID_STR_LEN];
	char sz_edit_type[EDIT_TYPE_STR_LEN];
}STUserInput;

typedef struct{
	STModuleContainer *pstModuleContainer;
	struct list *lpublic;/*解析public.txt文件的链表头*/
	struct list *lcon;/*解析control.txt文件的链表头*/
	char encry[BUF_LEN];
	char product[PORDUCT_STR_LEN];
	char *username_encry;	         /*存储解密后的当前登陆用户名*/
	int iUserGroup;	//为0时表示是管理员。
	FILE *cgiOut;
	STUserInput stUserInput;
	
	int formProcessError;
} STPageInfo;

static int s_arp_prefix_of_page( STPageInfo *pstPageInfo );
static int s_arp_content_of_page( STPageInfo *pstPageInfo );
static int getUserInput( STUserInput *pstUserInput );
static int doUserCommand( STUserInput *pstUserInput );
static int showPortSelect( char *selected_portno, char *name, char *onchange );

int cgiMain()
{
	STPageInfo stPageInfo;
	STLabel *pstLabel;
	char url[MAX_URL_LEN]="";
	
//初始化常用公共变量
	memset( &stPageInfo, 0,sizeof(STPageInfo) );
	stPageInfo.lpublic=get_chain_head("../htdocs/text/public.txt");
	stPageInfo.lcon=get_chain_head("../htdocs/text/control.txt");

	cgiFormStringNoNewlines("UN", stPageInfo.encry, BUF_LEN);
	
	stPageInfo.username_encry=dcryption(stPageInfo.encry);
    if( NULL == stPageInfo.username_encry )
    {
	    ShowErrorPage(search(stPageInfo.lpublic,"ill_user")); 	  /*用户非法*/
		return 0;
	}
	stPageInfo.iUserGroup = checkuser_group( stPageInfo.username_encry );

	stPageInfo.pstModuleContainer = MC_create_module_container();
	if( NULL == stPageInfo.pstModuleContainer )
	{
		return 0;
	}
	stPageInfo.cgiOut = cgiOut;
//初始化完毕
	
//处理表单
    if( cgiFormSubmitClicked("add_ruler") == cgiFormSuccess  )
    {
         
    	stPageInfo.formProcessError = getUserInput( &(stPageInfo.stUserInput) );
    	if( 0 == stPageInfo.formProcessError )
    	{
    		ccgi_dbus_init();
    		stPageInfo.formProcessError = doUserCommand( &(stPageInfo.stUserInput) );

		}
		
    }

	//属性检测
	pstLabel = LB_create_label();
	if( NULL != pstLabel )
	{
		LB_setLabelName( pstLabel, search(stPageInfo.lcon,"prt_sur") );
		
		snprintf( url,sizeof(url), "wp_prtsur.cgi?UN=%s", stPageInfo.encry );
		LB_setLabelUrl( pstLabel, url ); 
		MC_addLabel( stPageInfo.pstModuleContainer, pstLabel );
	}	
	
	//ARP检测
	pstLabel = LB_create_label();
	if( NULL != pstLabel )
	{
		char temp[64]="ARP ";
		
		strcat( temp, search(stPageInfo.lpublic,"survey") );
		LB_setLabelName( pstLabel, temp );
		
		snprintf( url, sizeof(url), "wp_prtarp.cgi?UN=%s", stPageInfo.encry );
		LB_setLabelUrl( pstLabel, url ); 
		MC_addLabel( stPageInfo.pstModuleContainer, pstLabel );
	}	

	if( 0 == stPageInfo.iUserGroup )
	{
		//静态arp
		pstLabel = LB_create_label();
		if( NULL != pstLabel )
		{
			LB_setLabelName( pstLabel, search(stPageInfo.lcon,"prt_static_arp") );
			
			snprintf( url, sizeof(url),"wp_static_arp.cgi?UN=%s", stPageInfo.encry );
			LB_setLabelUrl( pstLabel, url ); 
			MC_addLabel( stPageInfo.pstModuleContainer, pstLabel );
		}	
		
		//属性配置
		pstLabel = LB_create_label();
		if( NULL != pstLabel )
		{
			LB_setLabelName( pstLabel, search(stPageInfo.lcon,"prt_cfg") );
			
			snprintf( url, sizeof(url), "wp_prtcfg.cgi?UN=%s", stPageInfo.encry );
			LB_setLabelUrl( pstLabel, url ); 
			MC_addLabel( stPageInfo.pstModuleContainer, pstLabel );
		}	
		//功能配置
		pstLabel = LB_create_label();
		if( NULL != pstLabel )
		{
			LB_setLabelName( pstLabel, search(stPageInfo.lcon,"func_cfg") );
			
			snprintf( url, sizeof(url),"wp_prtfuncfg.cgi?UN=%s", stPageInfo.encry );
			LB_setLabelUrl( pstLabel, url ); 
			MC_addLabel( stPageInfo.pstModuleContainer, pstLabel );
		}
	}		
	//子接口
	pstLabel = LB_create_label();
	if( NULL != pstLabel )
	{
		LB_setLabelName( pstLabel, search(stPageInfo.lcon,"title_subintf") );
		
		snprintf( url, sizeof(url),"wp_subintf.cgi?UN=%s", stPageInfo.encry );
		LB_setLabelUrl( pstLabel, url ); 
		MC_addLabel( stPageInfo.pstModuleContainer, pstLabel );
	}

	//配置IP
	pstLabel = LB_create_label();
	if( NULL != pstLabel )
	{
		LB_setLabelName( pstLabel, search(stPageInfo.lpublic,"config_interface") );
		
		snprintf( url, sizeof(url),"wp_interface_bindip.cgi?UN=%s", stPageInfo.encry );
		LB_setLabelUrl( pstLabel, url ); 
		MC_addLabel( stPageInfo.pstModuleContainer, pstLabel );
	}
	//INTF
	pstLabel = LB_create_label();
	if( NULL != pstLabel )
	{
		LB_setLabelName( pstLabel, search(stPageInfo.lcon,"interface") );
		
		snprintf( url, sizeof(url),"wp_all_interface.cgi?UN=%s", stPageInfo.encry );
		LB_setLabelUrl( pstLabel, url ); 
		MC_addLabel( stPageInfo.pstModuleContainer, pstLabel );
	}
				
	//当前页面是static arp的页面，所以设置活动label为2
	MC_setActiveLabel( stPageInfo.pstModuleContainer, 2 );

	MC_setPrefixOfPageCallBack( stPageInfo.pstModuleContainer, (MC_CALLBACK)s_arp_prefix_of_page, &stPageInfo );
	MC_setContentOfPageCallBack( stPageInfo.pstModuleContainer, (MC_CALLBACK)s_arp_content_of_page, &stPageInfo );

	
	MC_setOutPutFileHandle( stPageInfo.pstModuleContainer, cgiOut );
	
	MC_setModuleContainerDomainValue( stPageInfo.pstModuleContainer, PAGE_TITLE, "static_arp_setting" );
	MC_setModuleContainerDomainValue( stPageInfo.pstModuleContainer, MODULE_TITLE, search(stPageInfo.lcon,"prt_manage") );
	MC_setModuleContainerDomainValue( stPageInfo.pstModuleContainer, FORM_ENCTYPE, "multipart/form-data" );
	MC_setModuleContainerDomainValue( stPageInfo.pstModuleContainer, FORM_ONSIBMIT, "return true;" );//可以设置为一个javascript函数
	MC_setModuleContainerDomainValue( stPageInfo.pstModuleContainer, FORM_METHOD, "post" );
	//MC_setModuleContainerDomainValue( stPageInfo.pstModuleContainer, FORM_ACTION, "wp_static_arp.cgi" );
	MC_setModuleContainerDomainValue( stPageInfo.pstModuleContainer, PUBLIC_INPUT_ENCRY, stPageInfo.encry );
	
	
	MC_setModuleContainerDomainValue( stPageInfo.pstModuleContainer, BTN_OK_IMG, search(stPageInfo.lpublic,"img_ok") );
#if 0
	MC_setModuleContainerDomainValue( stPageInfo.pstModuleContainer, BTN_OK_URL, "wp_static_arp.cgi?label=0" );
#else
	//MC_setModuleContainerDomainValue( stPageInfo.pstModuleContainer, BTN_OK_SUBMIT_NAME, "submit_add_static_arp" );
	snprintf( url, sizeof(url), "wp_static_arp.cgi?UN=%s", stPageInfo.encry );
	MC_setModuleContainerDomainValue( stPageInfo.pstModuleContainer, BTN_OK_URL, url );


#endif	
	MC_setModuleContainerDomainValue( stPageInfo.pstModuleContainer, BTN_CANCEL_IMG, search(stPageInfo.lpublic,"img_cancel") );
	snprintf( url, sizeof(url), "wp_contrl.cgi?UN=%s", stPageInfo.encry );
	MC_setModuleContainerDomainValue( stPageInfo.pstModuleContainer, BTN_CANCEL_URL, url );
	
	MC_setModuleContainerDomainValue( stPageInfo.pstModuleContainer, LABLE_TOP_HIGHT, "25" );
#if 0	
	MC_setModuleContainerDomainValue( stPageInfo.pstModuleContainer, LABLE_BOTTOM_HIGHT, "300" );//如果你不设置，高度自动调整
#endif
	
	MC_writeHtml( stPageInfo.pstModuleContainer );
	release(stPageInfo.lcon);
	release(stPageInfo.lpublic);
	
	MC_destroy_module_container( &(stPageInfo.pstModuleContainer) );
	
	
	return 0;
}


static int s_arp_err_proc( STPageInfo *pstPageInfo )
{
    char *error_message=NULL;

	if( NULL == pstPageInfo )
	{
		return -1;
	}
 	switch( pstPageInfo->formProcessError )
 	{
 	    case 333:
		   error_message = search( pstPageInfo->lpublic, "oper_succ" );
 			break;
 	    case -5:
 	        error_message = search( pstPageInfo->lcon, "s_arp_ip_err" );
 	        break;
 	    case -3:
 	        error_message = search( pstPageInfo->lcon, "s_arp_mac_err_format" );
 	        break;
 	    case -4:
 	        error_message = search( pstPageInfo->lcon, "s_arp_mac_broadcast" );
 	        break;
 	    case -8:
 	        error_message = search( pstPageInfo->lcon, "s_arp_vlan_out_range" );
 	        break;
		case -9:
			error_message = search( pstPageInfo->lcon, "s_arp_mac_err_format" );
 	        break;
		case -10:
			error_message = search( pstPageInfo->lcon, "s_arp_arp_system_mac_error" );
 	        break;
		case -11:
			error_message = search( pstPageInfo->lcon, "s_arp_arp_not_for_port" );
 	        break;
		case -12:
			error_message = search( pstPageInfo->lcon, "s_arp_arp_interface_mac_error" );
 	        break;
		case -13:
			error_message = search( pstPageInfo->lcon, "s_arp_arp_items_already_full" );
 	        break;
		case -1:
 	        error_message = search( pstPageInfo->lpublic, "oper_fail" );  
 	        break;
			
 
 	    case NPD_DBUS_ERROR:
 	        error_message = search( pstPageInfo->lcon, "s_arp_intf_create_err" );
 	        break;
 	    case DCLI_VLAN_BADPARAM:
 	        error_message = search( pstPageInfo->lcon, "s_arp_bad_param" );
 	        break;
 	    case DCLI_VLAN_NOTEXISTS:
 	        error_message = search( pstPageInfo->lcon, "s_arp_vlan_notexist" );
 	        break;
 	    case DCLI_DBUS_PORT_NOT_IN_VLAN:
 	        error_message = search( pstPageInfo->lcon, "s_arp_port_not_in_vlan" );
 	        break;
 	    case DCLI_INTF_NOTEXISTED:
 	        error_message = search( pstPageInfo->lcon, "s_arp_vlan_not_l3" );
 	        break;
 	    case DCLI_L3_INTF_NOT_ACTIVE:
 	        error_message = search( pstPageInfo->lcon, "s_arp_l3_not_active" );
 	        break;
 	    case DCLI_INTF_NO_HAVE_ANY_IP:
 	        error_message = search( pstPageInfo->lcon, "s_arp_l3_no_ip" );
 	        break;
 	    case DCLI_INTF_STATUS_CHECK_ERR:
 	        error_message = search( pstPageInfo->lcon, "s_arp_intf_status_failed" );
 	        break;
 	    case DCLI_INTF_HAVE_THE_IP:
 	        error_message = search( pstPageInfo->lcon, "s_arp_intf_hasthe_ip" );
 	        break;
 	    case DCLI_INTF_NOT_SAME_SUB_NET:
 	        error_message = search( pstPageInfo->lcon, "s_arp_not_same_subnet" );
 	        break;
 	    case DCLI_ARP_SNOOPING_ERR_STATIC_EXIST:
 	        error_message = search( pstPageInfo->lcon, "s_arp_have_exist" );
 	        break;       
 	    case DCLI_ARP_SNOOPING_ERR_KERN_CREATE_FAILED:
 	        error_message = search( pstPageInfo->lcon, "s_arp_keneral_failed" );
 	        break;
 	    case WS_ERR_PORT_NUM:
 	        error_message = search( pstPageInfo->lcon, "port_num_err" );
 	        break;
 	    case NPD_DBUS_ERROR_NO_SUCH_PORT:
			break;			
 	    default:			
 	        //error_message = search( pstPageInfo->lcon, "s_arp_unknow_err" );
 	        break;
 	}
 	if( NULL != error_message )
 	{
 	    ShowAlert( error_message );
 	}

	return 0;	
}


static int s_arp_prefix_of_page( STPageInfo *pstPageInfo )
{
	
	fprintf( cgiOut, "<style type=text/css>.a3{width:30px;border:0px; text-align:center}</style>" );
  		
 	fprintf( cgiOut, "<script language=javascript src=/ip.js></script>\n" );
	   

	return s_arp_err_proc( pstPageInfo );
}



static int s_arp_content_of_page( STPageInfo *pstPageInfo )
{
	//4个参数
	//fprintf( cgiOut, "<input type=hidden name=PRODUCT value='%s'>", pstPageInfo->product);
	fprintf( cgiOut, "<table>\n"\
			"	<tr><td>%s:</td><td>\n", search( pstPageInfo->lcon, "port_no" ) );
			
	showPortSelect( NULL, "static_arp_portnum", NULL );
			
			
	fprintf( cgiOut, "</td><td></td></tr>\n" );
	fprintf( cgiOut, "	<tr><td>MAC:</td><td><input type=text name=static_arp_mac style='width:143px;'/></td><td><font color=red>%s</font></td></tr>\n"\
			"	<tr><td>IP:</td><td><div style='border-width:1px;border-color:#a5acb2;border-style:solid;width:147px;font-size:9pt'>",search(pstPageInfo->lpublic,"mac_format") );
	fprintf( cgiOut, "<input type=text name='port_ip1' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() />.",search(pstPageInfo->lpublic,"ip_error"));
	fprintf( cgiOut, "<input type=text name='port_ip2' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() />.",search(pstPageInfo->lpublic,"ip_error"));
	fprintf( cgiOut, "<input type=text name='port_ip3' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() />.",search(pstPageInfo->lpublic,"ip_error"));
	fprintf( cgiOut, "<input type=text name='port_ip4' maxlength=3 class=a3 onKeyUp=\"mask(this,%s)\" onbeforepaste=mask_c() />",search(pstPageInfo->lpublic,"ip_error"));
	fprintf( cgiOut, "</div></td><td></td></tr>\n" );

	//kehao modified  
	//vlan id ,if the product ID  of device is 5608 or 5612e,it can not support vlan,so forbid it

	int product_id  = 0;
	product_id = get_product_id();

	if(product_id != PRODUCT_ID_AX5K_E  &&  product_id != PRODUCT_ID_AX5608)
	{
	  fprintf( cgiOut, "<tr><td>VLAN ID:</td><td><input type=text name='static_arp_vlanid' style='width:143px;' /></td><td><font color=red>%s</font></td></tr>\n",search(pstPageInfo->lpublic,"vlan_id_range"));
	  ///////////////////////////////////////////
	}
	fprintf( cgiOut, "<tr><td><input type=submit name='add_ruler' style='width:40px;' value=\"%s\" /></td><td></td></tr>\n",search(pstPageInfo->lcon,"add"));
  	fprintf( cgiOut, "</table>");

	return 0;
}


static int showPortSelect( char *selected_portno, char *name, char *onchange )
{
	char *device;
	char slot_port[PORT_NUM_STR_LEN];
	int num,ret;
  	struct slot sr;
	ETH_SLOT_LIST  head,*p;
    ETH_PORT_LIST *pp;
  	
	sr.module_status=0;     
	sr.modname=(char *)malloc(20);     //为结构体成员申请空间，假设该字段的最大长度为20
	sr.sn=(char *)malloc(20);          //为结构体成员申请空间，假设该字段的最大长度为20
	sr.hw_ver=0;
	sr.ext_slot_num=0;
  	
  	
	device=readproductID();	
	if( NULL == onchange )
	{
		fprintf( cgiOut, "<select name='%s_select_in' onchange='%s_selchange_infunc()'>\n", name, name );
	}
	else
	{
		fprintf( cgiOut, "<select name='%s_select' onchange='%s'>\n", name, onchange );
	}


	if( NULL == selected_portno || strlen(selected_portno) == 0 )
	{
		selected_portno = "1-1";
	}
	
	ccgi_dbus_init();		 //初始化dbus
	ret=show_ethport_list(&head,&num);
	p=head.next;
	if(p!=NULL)
	{
		while(p!=NULL)
		{
			pp=p->port.next;
			while(pp!=NULL)
			{
				if(p->slot_no!=0)
				{
					memset(slot_port,0,sizeof(slot_port));							
					sprintf(slot_port,"%d-%d",p->slot_no,pp->port_no);		 /*int转成char*/
					if(strcmp(slot_port,selected_portno)==0)
					{
						fprintf(cgiOut,"<option value='%s' selected=selected>%s</option>",slot_port,slot_port);
					}
					else
					{
						fprintf(cgiOut,"<option value='%s'>%s</option>",slot_port,slot_port);
					}
				}
				pp=pp->next;
			}
			p=p->next;
		}
	}									

	fprintf( cgiOut, "</select>" );
	
	fprintf( cgiOut, "<input type=hidden name='%s' value='' />\n", name );
	fprintf( cgiOut, "<script type=text/javascript>\n" );
	fprintf( cgiOut, "	function %s_selchange_infunc(){\n"\
					 "		var sel=document.getElementsByName( '%s_select_in' )[0];\n"\
					 "		var inp=document.getElementsByName( '%s' )[0];\n"\
					 "		inp.value = sel.options[sel.selectedIndex].value;\n"\
					 "		//alert( inp.value );\n"\
					 "	}\n"\
					 "	%s_selchange_infunc();\n"\
					 "</script>\n", name, name, name, name );
	if((ret==0)&&(num>0))
	{
		Free_ethslot_head(&head);
	}
	return 0;
}


static int getUserInput( STUserInput *pstUserInput )
{
	char port_ip1[5],port_ip2[5],port_ip3[5],port_ip4[5];
	
	if( NULL == pstUserInput )
	{
		return WP_ERR_OUTOF_MEMERY;	
	}
//编辑类型，如果没有该参数，编辑类型为add，如果为delete,表示删除一个静态arp
//delete是从wp_prtarp.cgi连接过来的，处理完了要再连接回去
	cgiFormStringNoNewlines( "editType", pstUserInput->sz_edit_type, sizeof(pstUserInput->sz_edit_type) );
	
	
	//端口号
	cgiFormStringNoNewlines( "static_arp_portnum", pstUserInput->port_num, sizeof(pstUserInput->port_num) );
	
	//mac
	cgiFormStringNoNewlines( "static_arp_mac", pstUserInput->mac, sizeof(pstUserInput->mac) );

#if 0
	if( strcmp(pstUserInput->sz_edit_type, "delete" ) == 0 )//如果是delete，ip地址是从url上得到的，没有分开
	{
		cgiFormStringNoNewlines( "ipaddr", pstUserInput->ipaddr, sizeof(pstUserInput->ipaddr) );
		
	}
	else
#endif		
	{
		//ip地址
		memset( port_ip1, 0, sizeof(port_ip1) );
		cgiFormStringNoNewlines( "port_ip1", port_ip1, sizeof(port_ip1) );
		memset( port_ip2, 0, sizeof(port_ip2) );
		cgiFormStringNoNewlines( "port_ip2", port_ip2, sizeof(port_ip2) );
		memset( port_ip3, 0, sizeof(port_ip3) );
		cgiFormStringNoNewlines( "port_ip3", port_ip3, sizeof(port_ip3) );
		memset( port_ip4, 0, sizeof(port_ip4) );
		cgiFormStringNoNewlines( "port_ip4", port_ip4, sizeof(port_ip4) );
	
	
	//fprintf( cgiOut, "port_ip1 = %s  <br /> port_ip1 = %s  <br />port_ip1 = %s  <br />port_ip1 = %s  <br />\n", port_ip1, port_ip2, port_ip3, port_ip4 );
		if( strlen(port_ip1) == 0 || strlen(port_ip2) == 0 || strlen(port_ip3) == 0 || strlen(port_ip4) == 0  )
		{
			return -2;
		}	
		sprintf( pstUserInput->ipaddr,"%s.%s.%s.%s", port_ip1, port_ip2, port_ip3, port_ip4 );
	}	
	
	
	//vlanid
	cgiFormStringNoNewlines( "static_arp_vlanid", pstUserInput->vlanID, sizeof(pstUserInput->vlanID) );
	
	return 0;
}


static int doUserCommand( STUserInput *pstUserInput )
{
	char ipaddr[32];
	int iRet=0;
	
	if( NULL == pstUserInput )
	{
		return WP_ERR_NULL_POINTER;
	}
	
	if( strlen(pstUserInput->ipaddr) > 16 )
	{
	    return WP_ERR_IPADDR_LEN;    
	}
	
	sprintf( ipaddr, "%s/32", pstUserInput->ipaddr );//命令要求输入的ip地址带一个32bit的掩码

	
	iRet = config_ip_static_arp( pstUserInput->port_num, pstUserInput->mac, ipaddr, pstUserInput->vlanID );

	if(iRet==0)
		return 333;

	return iRet;
}




