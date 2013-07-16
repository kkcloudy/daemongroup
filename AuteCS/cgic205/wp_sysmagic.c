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
* wp_sysmagic.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system function for product panel 
*
*
*******************************************************************************/
#include <stdio.h>
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include "ws_usrinfo.h"
#include "ws_err.h"
#include "ws_ec.h"
#include "ws_nm_status.h"
#include "ws_sysinfo.h"
#include "ws_sndr_cfg.h"
#include "ws_list_container.h"
#include "ws_secondary_container.h"
#include "ws_dbus_list.h"



void ShowSysmagPage(char *m,char *n,struct list *lpublic,struct list *lsys,struct list *lcon,unsigned int Pid);     /*m代表用户名，n代表加密后的字符串*/

void SetColor(int wth,char *q);                /* wth代表柱状图宽度*/

void select_id(unsigned int id,unsigned int value,struct eth_port_s pt,char *prt_no,int retu,char *n,struct list *lcon,char *name);

void select_5k_id(struct sys_ver ptrsysver,unsigned int value,struct eth_port_s pt,int retu,struct list *lcon,char *n,char *m,unsigned int Mid);

void select_panel_id(unsigned int id,unsigned int value,struct slot sl,struct sys_ver ptrsysver,struct eth_port_s pt,char *prt_no,int retu,char *n,struct list *lcon,char * m,unsigned int Mid);

void module_6GTX(unsigned int value,struct eth_port_s pt,char *prt_no,int retu,char *n,struct list *lcon,char *name);

void module_XFP(unsigned int value,struct eth_port_s pt,char *prt_no,int retu,char *n,struct list *lcon,char *name);

void module_SFP(unsigned int value,struct eth_port_s pt,char *prt_no,int retu,char *n,struct list *lcon,char *name);

int port_info(int i,unsigned int value,char *prt_no,struct eth_port_s pt,int retu,char *n);

int light_port(int i,unsigned int value,char *prt_no,struct eth_port_s pt,int retu,char *n);

int elec_port(int i,unsigned int value,char *prt_no,struct eth_port_s pt,int retu,char *n);

int panel_7k(struct sys_ver ptrsysver,unsigned int value,struct slot sl,struct eth_port_s pt,char *prt_no,int retu,char *n,struct list *lcon);

int panel_5k(struct sys_ver ptrsysver,unsigned int value,struct eth_port_s pt,int retu,struct list *lcon,char *n,char *m);

int panel_5k_4624(struct sys_ver ptrsysver,unsigned int value,struct eth_port_s pt,int retu,struct list *lcon,char *n,char *m);

int panel_5k_3052(struct sys_ver ptrsysver,unsigned int value,struct eth_port_s pt,int retu,struct list *lcon,char *n,char *m);

int light_flag(int begin,int end,struct eth_port_s pt,int type);

int light_spe(int key,struct eth_port_s pt,char *content);

int light_spe_new(int key,struct eth_port_s pt,char *content);

int light_flag_new(int begin,int end,struct eth_port_s pt,int type);

int light_flag_3052(int begin,int end,struct eth_port_s pt,int type);

int light_spe_3052(int key,struct eth_port_s pt,char *content);

int light_flag_5612i(int begin,int end,struct eth_port_s pt,int type,char *flight,char *fsep,char *slight,char *ssep);

int panel_5K_5612i(unsigned int value,struct eth_port_s pt,int retu,char *n,char *m);
int panel_5K_5612E(unsigned int value,struct eth_port_s pt,int retu,char *n,char *m);

void  panel_7605i();
void panel_86();
void  select_86_slotinfo(int slotid);


////////////////////////////////// begin new

static int ShowPage(struct secondary_module_container *p );


/////////////////////////////////////end new



#define sec 60   /*刷新的间隔时间，单位: 秒*/

int panel_fill_summary( STPubInfoForItem *p_pubinfo, STSndrItem *p_item ) //panel,has callback
{
   
	p_item->fp=cgiOut;
	if( NULL == p_item )
	{
		return -1;	
	}
	char temp_url[128];
	memset(temp_url,0,128);
	SI_set_label_name( p_item, search(p_pubinfo->local,"user_manage")); ///images/SysFun.jpg
	SI_set_label_img( p_item,"/images/UserMan.jpg");

	///////////////
	struct sys_ver ptrsysver;/*产品系统信息*/
    ptrsysver.product_name=(char *)malloc(50);
    ptrsysver.base_mac=(char *)malloc(50);
    ptrsysver.serial_no=(char *)malloc(50);
    ptrsysver.swname=(char *)malloc(50);
    unsigned int propid;
	unsigned int Pid;
	
    char *m;
	ccgi_dbus_init();	  /*初始化dbus*/			
	show_sys_ver(&ptrsysver);
    propid=get_product_id();
    Pid=propid;

	m=dcryption(p_pubinfo->encry);
    int ret = 2;
	ret = checkuser_group(m);
	if(ret==-1)
	{
		  ShowErrorPage(search(p_pubinfo->public,"ill_user")); 		   /*用户非法*/
 	}
	else
	{
          if(ret==1)
 		  {
	 		  sprintf(temp_url,"wp_usrlis.cgi?UN=%s",p_pubinfo->encry);
		      SI_set_label_url(p_item,temp_url);
	 		  //fprintf(cgiOut,"<td width=101 align=left ><a href=wp_usrlis.cgi?UN=%s target=mainFrame class=qu><font id=%s>%s</font></a></td>",n,search(lpublic,"menu_er"),search(lsys,"user_manage"));
          }
		  if(ret==0)
		  {
			  sprintf(temp_url,"wp_usrlis.cgi?UN=%s",p_pubinfo->encry);
		      SI_set_label_url(p_item,temp_url);
	          //fprintf(cgiOut,"<td width=101 align=left ><a href=wp_usrlis.cgi?UN=%s target=mainFrame class=qu><font id=%s>%s</font></a></td>",n,search(lpublic,"menu_er"),search(lsys,"user_manage"));
		  }
	}


	//////////////
	
	SI_set_label_font(p_item, search(p_pubinfo->public,"menu_er"));  
	
	free(ptrsysver.product_name);
    free(ptrsysver.base_mac);
    free(ptrsysver.serial_no);
    free(ptrsysver.swname);
	return 0;
}

int system_fill_summary( STPubInfoForItem *p_pubinfo, STSndrItem *p_item ) //system,no callback
{
   
	p_item->fp=cgiOut;
	if( NULL == p_item )
	{
		return -1;	
	}
	char temp_url[128];
	memset(temp_url,0,128);
	SI_set_label_name( p_item, search(p_pubinfo->local,"sys_function")); ///images/SysFun.jpg
	SI_set_label_img( p_item,"/images/SysFun.jpg");
	sprintf(temp_url,"wp_sysinfo.cgi?UN=%s",p_pubinfo->encry);
	SI_set_label_url(p_item,temp_url);
	SI_set_label_font(p_item, search(p_pubinfo->public,"menu_er"));  
	
	return 0;
}
int snmp_fill_summary( STPubInfoForItem *p_pubinfo, STSndrItem *p_item ) //snmp,no callback
{
   
	p_item->fp=cgiOut;
	if( NULL == p_item )
	{
		return -1;	
	}
	char temp_url[128];
	memset(temp_url,0,128);
	SI_set_label_name( p_item, "SNMP"); ///images/SysFun.jpg
	SI_set_label_img( p_item,"/images/SNMP.jpg");
	sprintf(temp_url,"wp_snmp.cgi?UN=%s",p_pubinfo->encry);
	SI_set_label_url(p_item,temp_url);
	SI_set_label_font(p_item, search(p_pubinfo->public,"menu_er"));  
	
	//不出现摘要
	
	return 0;
}

int license_fill_summary( STPubInfoForItem *p_pubinfo, STSndrItem *p_item ) //snmp,no callback
{
   
	p_item->fp=cgiOut;
	if( NULL == p_item )
	{
		return -1;	
	}
	char temp_url[128];
	memset(temp_url,0,128);
	SI_set_label_name( p_item, search(p_pubinfo->local,"auth_title")); ///images/SysFun.jpg
	SI_set_label_img( p_item,"/images/license.jpg");
	sprintf(temp_url,"wp_auth_license.cgi?UN=%s",p_pubinfo->encry);
	SI_set_label_url(p_item,temp_url);
	SI_set_label_font(p_item, search(p_pubinfo->public,"menu_er"));  
	
	//不出现摘要
	
	return 0;
}

STSCCreateHelper pstControlSCCreateHelper[] = {

	#if SNDR_USER_ITEM
	{panel_fill_summary},
	#endif 

	#if SNDR_SYSTEM_ITEM
	{system_fill_summary},
	#endif 	

	#if SNDR_SNMP_ITEM
	{snmp_fill_summary},
	#endif 
	{license_fill_summary}
};

#define HELPER_ITEM_NUM  sizeof(pstControlSCCreateHelper)/sizeof(pstControlSCCreateHelper[0])

int cgiMain()
{
    char *encry=(char *)malloc(BUF_LEN);              
    char *str;    
    char *proid=(char *)malloc(20);
    memset(proid,0,20);
	unsigned int Pid;

	STPubInfoForItem stPubInfoForItem;
	memset( &stPubInfoForItem, 0, sizeof(stPubInfoForItem));  
    struct list *lpublic;  
    lpublic=get_chain_head("../htdocs/text/public.txt");
    struct list *local;   
    local=get_chain_head("../htdocs/text/system.txt"); 

  
	stPubInfoForItem.public = lpublic;
	stPubInfoForItem.local = local;
	
    memset(encry,0,BUF_LEN);
    cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
    cgiFormStringNoNewlines("ID", proid, 20); 
    Pid=strtoul(proid,0,10);

	strcpy( stPubInfoForItem.encry, encry );
	
    str=dcryption(encry);
    if(str==NULL)
    ShowErrorPage(search(lpublic,"ill_user"));		 /*用户非法*/
    else
    {
    	STSndrContainer *pstControlSndrContainer = NULL;	
    	pstControlSndrContainer = create_sndr_module_container_helper( &stPubInfoForItem, pstControlSCCreateHelper, HELPER_ITEM_NUM );
    	if( NULL != pstControlSndrContainer )
    	{   
    	    pstControlSndrContainer->fp=cgiOut;
			pstControlSndrContainer->lpublic=lpublic;
			pstControlSndrContainer->local=local;

			strcpy(pstControlSndrContainer->encry,encry);
		    pstControlSndrContainer->pid=Pid;
			
            MC_setPageCallBack(pstControlSndrContainer, (EX_SHOW_CALL_BACK_N) ShowPage,&pstControlSndrContainer);

			pstControlSndrContainer->flag=1;
			SC_writeHtml(pstControlSndrContainer);  
    		release_sndr_module_container( pstControlSndrContainer );  
    	}
    }
	free(proid);
    free(encry);
	release(lpublic);  
	release(local);	
	return 0;
}

static int ShowPage(struct secondary_module_container *p )
{   
  
  struct list *lcon;      /*解析control.txt文件的链表头*/  
  lcon=get_chain_head("../htdocs/text/control.txt");
  
  int height=0;
  int retu;
  char *prt_no=(char *)malloc(10);  
  struct slot sl;        //声明存储slot 信息的结构体变量
  struct eth_port_s pt;        //声明存储port 信息的结构体变量  
  struct sys_envir sys;  
  int p_getid = 0;
  p_getid = get_product_info("/dbm/product/product_serial");
  int devicetype = 0;
  devicetype = get_product_info("/dbm/product/product_type");
  unsigned int propid;
  unsigned int Mid=0;
  
  struct sys_ver ptrsysver;/*产品系统信息*/
  ptrsysver.product_name=(char *)malloc(50);
  ptrsysver.base_mac=(char *)malloc(50);
  ptrsysver.serial_no=(char *)malloc(50);
  ptrsysver.swname=(char *)malloc(50); 
  
  sl.module_status=0;     
  sl.modname=(char *)malloc(20);     //为结构体成员申请空间，假设该字段的最大长度为20
  sl.sn=(char *)malloc(20);          //为结构体成员申请空间，假设该字段的最大长度为20
  sl.hw_ver=0;
  sl.ext_slot_num=0;
  pt.attr_bitmap=0;
  pt.port_type=0;
  pt.mtu=0;  
  sys.fan_power=0;
  sys.core_tmprt=0;
  sys.surface_tmprt=0;
  unsigned int value = 0; 
  
  //////////////
  unsigned int Pid;
  Pid=p->pid;
  
  ccgi_dbus_init();	  /*初始化dbus*/			
  show_sys_ver(&ptrsysver);
  propid=get_product_id();
  Pid=propid;
  
  char *m;
  m=dcryption(p->encry);

  FILE *ffp=p->fp;
 
  fprintf(ffp,"<table width=766 height=200 align=center border=0 bgcolor=#ffffff cellpadding=0 cellspacing=0>"\
			    "<tr>"\
			      "<td width=766 align=center>"\
		"<table width=722 border=0 cellspacing=0 cellpadding=0 style=padding-bottom:10px>"\
          "<tr height=30>");
         {			
 			fprintf( ffp, "<td><p id=refresh>\n" );
 			fprintf( ffp, search(p->local,"auto_refresh"),"<input type=text name=DateTime size=1 id=retext>" );
 			fprintf(ffp,"</p></td>");
         }

       if(Pid==PRODUCT_ID_AX7K)
	   	 height=286;
	   if(Pid==PRODUCT_ID_AU4K)
	   	 height=200;
	   if(Pid==3)
	   	 height=200;
	   else
	   	 height=200;
	   
          fprintf(ffp,"</tr>"\
          "<tr height=%d>"\
          "<td align=center>",height);
		  retu=checkuser_group(m);
		  
		if(devicetype == 4)
		{
			panel_86();
		}		
		else if((p_getid == 7)||(devicetype == 6))
		{
			panel_7605i();
		}
		else
		{
		/*********************************7605 panel***********************************************/
			if(nm_show_hw_config(0,&sl)==CCGI_SUCCESS)     /*如果读取信息成功*/
			{
        		Mid=sl.module_id;
	    		select_panel_id(Pid,value,sl,ptrsysver,pt,prt_no,retu,p->encry,lcon,m,Mid);
			}
       /***************************************end 7605 panel**************************************/
		}

     fprintf(cgiOut,"</td>"\
     "</tr>"\
     "</table>"\
	 "</td>"\
	 "</tr>"\
	 "</table>");

release(lcon);
free(prt_no);		  
free(sl.modname);
free(sl.sn);  
free(ptrsysver.product_name);
free(ptrsysver.base_mac);
free(ptrsysver.serial_no);
free(ptrsysver.swname);
return 0;
}

/////////////////////////////////////////////////////////////end new

/*选择模块名*/
void select_id(unsigned int id,unsigned int value,struct eth_port_s pt,char *prt_no,int retu,char *n,struct list *lcon,char *name)
{
 switch( id )
 	{
 	case MODULE_ID_AX7_6GTX:
 	module_6GTX(value,pt,prt_no,retu,n,lcon,name);
	break;

    case MODULE_ID_AX7_XFP:
 	module_XFP(value,pt,prt_no,retu,n,lcon,name);
    break;

	case MODULE_ID_AX7_6GE_SFP:
 	module_SFP(value,pt,prt_no,retu,n,lcon,name);
	break;

	default:
     fprintf(cgiOut,"<img src=/images/panel/7000news_23.jpg width=344 height=55 >");
		break;
 	}
}



/*选择模块名*/
void select_5k_id(struct sys_ver ptrsysver,unsigned int value,struct eth_port_s pt,int retu,struct list *lcon,char *n,char *m,unsigned int Mid)
{

 switch( Mid )
 	{

	
 	case MODULE_ID_AU4_4626:
	case MODULE_ID_AU3_3524:
 	panel_5k(ptrsysver,value,pt,retu,lcon,n, m);
	break;
  
    case MODULE_ID_AX5_5612:
 	panel_5k_4624(ptrsysver,value,pt,retu,lcon,n,m);
    break;		

    case MODULE_ID_AX5_5612I:
	panel_5K_5612i(value,pt,retu,n,m);
    break;		

	case MODULE_ID_AX5_5612E:
 	panel_5K_5612E(value, pt, retu, n, m);
    break;	

	case MODULE_ID_AU3_3052:
 	panel_5k_3052(ptrsysver,value,pt,retu,lcon,n,m);
    break;	
	
	default:
    fprintf(cgiOut,"<font size=4>%s</font>",search(lcon,"no_support"));
		break;
 	}
}

/*选择产品面板名字*/
void select_panel_id(unsigned int id,unsigned int value,struct slot sl,struct sys_ver ptrsysver,struct eth_port_s pt,char *prt_no,int retu,char *n,struct list *lcon,char * m,unsigned int Mid)
{

 	switch( id )
 	{
		case PRODUCT_ID_AX7K:
			panel_7k(ptrsysver,value,sl,pt,prt_no,retu,n,lcon);
			break;
		case PRODUCT_ID_AX7K_I:
			panel_7605i();
			break;
		case PRODUCT_ID_AU4K:
		case PRODUCT_ID_AX5K:
		case PRODUCT_ID_AU3K:
		case PRODUCT_ID_AU3K_BCM:
		case PRODUCT_ID_AX5K_I:
		case PRODUCT_ID_AX5K_E:
		case 3://1.0版本得到的是这个值为5000的设备
			select_5k_id(ptrsysver,value,pt,retu,lcon,n,m,Mid); //在5000的类型中再选
			break;
		default:
			fprintf(cgiOut,"<font size=4>%s</font>",search(lcon,"no_support"));
			break;
	}

}



/*模块名对应的函数*/
void module_6GTX(unsigned int value,struct eth_port_s pt,char *prt_no,int retu,char *n,struct list *lcon,char * name)
{
       
 int ret;  //判断状态
 int i;
 
 fprintf(cgiOut,"<table  width=345 height=55 border=0 cellpadding=0 cellspacing=0>"\
	"<tr>"\
		"<td colspan=19>"\
			"<img src=/images/GTX/GTX_01.jpg width=344 height=4 ></td>"\
		"<td>"\
			"<img src=/images/GTX/sep.gif width=1 height=4 ></td>"\
	"</tr>"\
	"<tr>"\
		"<td rowspan=6>"\
			"<img src=/images/GTX/GTX_02.jpg width=8 height=51 ></td>"\
		"<td colspan=5 bgcolor=#D0D0D0 height=15>");       
		//	fprintf(cgiOut,"<img src=/images/GTX/GTX_03.jpg width=75 height=15 >");
		fprintf(cgiOut,"</td>");
		fprintf(cgiOut,"<td colspan=13 rowspan=2>"\
			"<img src=/images/GTX/GTX_04.jpg width=261 height=17 ></td>"\
		"<td>"\
			"<img src=/images/GTX/sep.gif width=1 height=15 ></td>"\
	"</tr>"\
	"<tr>"\
		"<td colspan=5 rowspan=2>"\
			"<img src=/images/GTX/GTX_05.jpg width=75 height=17 ></td>"\
		"<td>"\
			"<img src=/images/GTX/sep.gif width=1 height=2 ></td>"\
	"</tr>"\
	"<tr>");

 fprintf(cgiOut,"<td rowspan=4>"\
			"<img src=/images/GTX/GTX_06.jpg width=57 height=34 ></td>");
		
        /*端口信息*/		
      //ret=port_info(i,value, prt_no, pt,retu,n,pro);

      for(i=1;i<6;i++)
  	  {
	      ret=port_info(i,value, prt_no, pt,retu,n);
		  
		  if(ret==22)
		  fprintf(cgiOut,"<td rowspan=2><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame><img src=%s width=23 height=19 border=0 alt=\"%s\"></td>",n,prt_no,"/images/GTX/GTX_07.jpg",search(lcon,"prt_cfg"));
		  if(ret==33)
		  fprintf(cgiOut,"<td rowspan=2><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame><img src=%s width=23 height=19 border=0 alt=\"%s\"></td>",n,prt_no,"/images/GTX/GTX_09.jpg",search(lcon,"prt_cfg"));
	      if(ret==11)
		  fprintf(cgiOut,"<td rowspan=2><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame><img src=%s width=23 height=19 border=0 alt=\"%s\"></td>",n,prt_no,"/images/GTX/GTX_11.jpg",search(lcon,"prt_cfg"));
	      if(ret==1)
		  fprintf(cgiOut,"<td rowspan=2><img src=/images/GTX/GTX_11.jpg width=23 height=19></td>");
		  if(ret==2)
		  fprintf(cgiOut,"<td rowspan=2><img src=/images/GTX/GTX_07.jpg width=23 height=19></td>");
		  if(ret==3)
		  fprintf(cgiOut,"<td rowspan=2><img src=/images/GTX/GTX_09.jpg width=23 height=19></td>");
		  
		  fprintf(cgiOut,"<td rowspan=4>"\
				"<img src=/images/GTX/GTX_08.jpg width=10 height=34 ></td>"); 
      	}		

	  ret=port_info(6,value, prt_no, pt,retu,n);	
	  if(ret==22)
	  fprintf(cgiOut,"<td rowspan=2><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame><img src=%s width=23 height=19 border=0 alt=\"%s\"></td>",n,prt_no,"/images/GTX/GTX_07.jpg",search(lcon,"prt_cfg"));
	  if(ret==33)
	  fprintf(cgiOut,"<td rowspan=2><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame><img src=%s width=23 height=19 border=0 alt=\"%s\"></td>",n,prt_no,"/images/GTX/GTX_09.jpg",search(lcon,"prt_cfg"));
      if(ret==11)
	  fprintf(cgiOut,"<td rowspan=2><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame><img src=%s width=23 height=19 border=0 alt=\"%s\"></td>",n,prt_no,"/images/GTX/GTX_11.jpg",search(lcon,"prt_cfg"));
      if(ret==1)
	  fprintf(cgiOut,"<td rowspan=2><img src=/images/GTX/GTX_11.jpg width=23 height=19></td>");
	  if(ret==2)
	  fprintf(cgiOut,"<td rowspan=2><img src=/images/GTX/GTX_07.jpg width=23 height=19></td>");
	  if(ret==3)
	  fprintf(cgiOut,"<td rowspan=2><img src=/images/GTX/GTX_09.jpg width=23 height=19></td>");
			
		fprintf(cgiOut,"<td rowspan=4>"\
			"<img src=/images/GTX/GTX_18.jpg width=16 height=34 ></td>"\
		"<td>"\
			"<img src=/images/GTX/sep.gif width=1 height=15 ></td>");		
			
	fprintf(cgiOut,"</tr>");
			
	fprintf(cgiOut,"<tr>"\
		"<td rowspan=3>"\
			"<img src=/images/GTX/GTX_19.jpg width=19 height=19 ></td>"\
		"<td rowspan=2>"\
			"<img src=/images/GTX/GTX_20.jpg width=10 height=9 ></td>"\
		"<td rowspan=3>"\
			"<img src=/images/GTX/GTX_21.jpg width=17 height=19 ></td>"\
		"<td rowspan=2>"\
			"<img src=/images/GTX/GTX_22.jpg width=10 height=9 ></td>"\
		"<td rowspan=3>"\
			"<img src=/images/GTX/GTX_23.jpg width=19 height=19 ></td>"\
		"<td>"\
			"<img src=/images/GTX/sep.gif width=1 height=4 ></td>"\
	"</tr>"\
	"<tr>"\
		"<td rowspan=2>"\
			"<img src=/images/GTX/GTX_24.jpg width=23 height=15 ></td>"\
		"<td rowspan=2>"\
			"<img src=/images/GTX/GTX_25.jpg width=23 height=15 ></td>"\
		"<td rowspan=2>"\
			"<img src=/images/GTX/GTX_26.jpg width=23 height=15 ></td>"\
		"<td rowspan=2>"\
			"<img src=/images/GTX/GTX_27.jpg width=23 height=15 ></td>"\
		"<td rowspan=2>"\
			"<img src=/images/GTX/GTX_28.jpg width=23 height=15 ></td>"\
		"<td rowspan=2>"\
			"<img src=/images/GTX/GTX_29.jpg width=23 height=15 ></td>"\
		"<td>"\
			"<img src=/images/GTX/sep.gif width=1 height=5 ></td>"\
	"</tr>"\
	"<tr>"\
		"<td>"\
			"<img src=/images/GTX/GTX_30.jpg width=10 height=10 ></td>"\
		"<td>"\
			"<img src=/images/GTX/GTX_31.jpg width=10 height=10 ></td>"\
		"<td>"\
			"<img src=/images/GTX/sep.gif width=1 height=10 ></td>"\
	"</tr>"\
"</table>");

       
}
/*XFP 模块函数 */
void module_XFP(unsigned int value,struct eth_port_s pt,char *prt_no,int retu,char *n,struct list *lcon,char * name)
{
int ret;
int i;
fprintf(cgiOut,"<table  width=345 height=55 border=0 cellpadding=0 cellspacing=0>"\
	"<tr>"\
		"<td colspan=11>"\
			"<img src=/images/XFP/xfp_01.jpg width=344 height=5 ></td>"\
		"<td>"\
			"<img src=/images/XFP/sep.gif width=1 height=5 ></td>"\
	"</tr>"\
	"<tr>"\
		"<td rowspan=10>"\
			"<img src=/images/XFP/xfp_02.jpg width=8 height=50 ></td>");
		fprintf(cgiOut,"<td colspan=5 rowspan=2 bgcolor=#D0D0D0 height=15>");
			//fprintf(cgiOut,"<img src=/images/XFP/xfp_03.jpg width=75 height=15 >");
		fprintf(cgiOut,"</td>");
		
		fprintf(cgiOut,"<td colspan=5>"\
			"<img src=/images/XFP/xfp_04.jpg width=261 height=11 ></td>"\
		"<td>"\
			"<img src=/images/XFP/sep.gif width=1 height=11 ></td>"\
	"</tr>"\
	"<tr>"\
		"<td rowspan=9>"\
			"<img src=/images/XFP/xfp_05.jpg width=81 height=39 ></td>"\
		"<td rowspan=3>"\
			"<img src=/images/XFP/xfp_06.jpg width=10 height=9 ></td>"\
		"<td colspan=3 rowspan=2>"\
			"<img src=/images/XFP/xfp_07.jpg width=170 height=7 ></td>"\
		"<td>"\
			"<img src=/images/XFP/sep.gif width=1 height=4 ></td>"\
	"</tr>"\
	"<tr>"\
		"<td colspan=5 rowspan=4>"\
			"<img src=/images/XFP/xfp_08.jpg width=75 height=17 ></td>"\
		"<td>"\
			"<img src=/images/XFP/sep.gif width=1 height=3 ></td>"\
	"</tr>"\
	"<tr>"\
		"<td rowspan=7>"\
			"<img src=/images/XFP/xfp_09.jpg width=31 height=32 ></td>");		
		
		    i=1;
		    ret=port_info(i,value, prt_no, pt,retu,n);
			if(ret==22)
    	    fprintf(cgiOut,"<td rowspan=4><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame><img src=%s width=46 height=16 border=0 alt=\"%s\"></td>",n,prt_no,"/images/XFP/xfp_10.jpg",search(lcon,"prt_cfg"));
    	    if(ret==33)
    	    fprintf(cgiOut,"<td rowspan=4><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame><img src=%s width=46 height=16 border=0 alt=\"%s\"></td>",n,prt_no,"/images/XFP/xfp_10.jpg",search(lcon,"prt_cfg"));
            if(ret==11)
    	    fprintf(cgiOut,"<td rowspan=4><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame><img src=%s width=46 height=16 border=0 alt=\"%s\"></td>",n,prt_no,"/images/XFP/xfp_10.jpg",search(lcon,"prt_cfg"));
            if(ret==1)
    	    fprintf(cgiOut,"<td rowspan=4><img src=/images/XFP/xfp_10.jpg width=46 height=16></td>");
    	    if(ret==2)
    	    fprintf(cgiOut,"<td rowspan=4><img src=/images/XFP/xfp_10.jpg width=46 height=16></td>");
    	    if(ret==3)
    	    fprintf(cgiOut,"<td rowspan=4><img src=/images/XFP/xfp_10.jpg width=46 height=16></td>");
			
			//fprintf(cgiOut,"<img src=/images/XFP/xfp_10.jpg width=46 height=16 >");
			
			fprintf(cgiOut,"<td rowspan=7>");
			fprintf(cgiOut,"<img src=/images/XFP/xfp_11.jpg width=93 height=32 >");
			fprintf(cgiOut,"</td>");
		fprintf(cgiOut,"<td>");
			fprintf(cgiOut,"<img src=/images/XFP/sep.gif width=1 height=2 >");
			fprintf(cgiOut,"</td>"\
	"</tr>"\
	"<tr>"\
		"<td>"\
			"<img src=/images/XFP/xfp_12.jpg width=10 height=11 ></td>"\
		"<td>"\
			"<img src=/images/XFP/sep.gif width=1 height=11 ></td>"\
	"</tr>"\
	"<tr>"\
		"<td rowspan=3>"\
			"<img src=/images/XFP/xfp_06.jpg width=10 height=9 ></td>"\
		"<td>"\
			"<img src=/images/XFP/sep.gif width=1 height=1 ></td>"\
	"</tr>"\
	"<tr>"\
		"<td rowspan=4>"\
			"<img src=/images/XFP/xfp_14.jpg width=19 height=18 ></td>"\
		"<td rowspan=3>"\
			"<img src=/images/XFP/xfp_15.jpg width=10 height=9 ></td>"\
		"<td rowspan=4>"\
			"<img src=/images/XFP/xfp_16.jpg width=17 height=18 ></td>"\
		"<td rowspan=3>"\
			"<img src=/images/XFP/xfp_06.jpg width=10 height=9 ></td>"\
		"<td rowspan=4>"\
			"<img src=/images/XFP/xfp_18.jpg width=19 height=18 ></td>"\
		"<td>"\
			"<img src=/images/XFP/sep.gif width=1 height=2 ></td>"\
	"</tr>"\
	"<tr>"\
		"<td rowspan=3>"\
			"<img src=/images/XFP/xfp_19.jpg width=46 height=16 ></td>"\
		"<td>"\
			"<img src=/images/XFP/sep.gif width=1 height=6 ></td>"\
	"</tr>"\
	"<tr>"\
		"<td rowspan=2>"\
			"<img src=/images/XFP/xfp_20.jpg width=10 height=10 ></td>"\
		"<td>"\
			"<img src=/images/XFP/sep.gif width=1 height=1 ></td>"\
	"</tr>"\
	"<tr>"\
		"<td>"\
			"<img src=/images/XFP/xfp_21.jpg width=10 height=9 ></td>"\
		"<td>"\
			"<img src=/images/XFP/xfp_22.jpg width=10 height=9 ></td>"\
		"<td>"\
			"<img src=/images/XFP/sep.gif width=1 height=9 ></td>"\
	"</tr>"\
"</table>");



}

void module_SFP(unsigned int value,struct eth_port_s pt,char *prt_no,int retu,char *n,struct list *lcon,char *name)
{
   int i;
   int ret;
   fprintf(cgiOut,"<table  width=345 height=55 border=0 cellpadding=0 cellspacing=0>"\
	"<tr>"\
		"<td colspan=19>"\
			"<img src=/images/SFP/sfp_01.jpg width=344 height=5 ></td>"\
		"<td>"\
			"<img src=/images/SFP/sep.gif width=1 height=5 ></td>"\
	"</tr>"\
	"<tr>"\
		"<td rowspan=5>"\
			"<img src=/images/SFP/sfp_02.jpg width=8 height=50 ></td>"\
		"<td colspan=5 bgcolor=#D0D0D0 height=15>");
			//fprintf(cgiOut,"<img src=/images/SFP/sfp_03.jpg width=75 height=15 >");
			fprintf(cgiOut,"</td>"\
		"<td colspan=13 rowspan=2>"\
			"<img src=/images/SFP/sfp_04.jpg width=261 height=18 ></td>"\
		"<td>"\
			"<img src=/images/SFP/sep.gif width=1 height=15 ></td>"\
	"</tr>"\
	"<tr>"\
		"<td colspan=5 rowspan=2>"\
			"<img src=/images/SFP/sfp_05.jpg width=75 height=17 ></td>"\
		"<td>"\
			"<img src=/images/SFP/sep.gif width=1 height=3 ></td>"\
	"</tr>"\
	"<tr>");
		fprintf(cgiOut,"<td rowspan=3>");
			fprintf(cgiOut,"<img src=/images/SFP/sfp_06.jpg width=57 height=32 >");
			fprintf(cgiOut,"</td>");

		for(i=1;i<6;i++)
		{
			ret=port_info(i,value, prt_no, pt,retu,n);
			if(i==1 || i==2 ||i==4)	
			{
				if(ret==22) //红色
				fprintf(cgiOut,"<td><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame><img src=%s width=21 height=14 border=0 alt=\"%s\"></td>",n,prt_no,"/images/SFP/sfp_07.jpg",search(lcon,"prt_cfg"));
				if(ret==33) //绿色
				fprintf(cgiOut,"<td><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame><img src=%s width=21 height=14 border=0 alt=\"%s\"></td>",n,prt_no,"/images/SFP/sfp_09.jpg",search(lcon,"prt_cfg"));
				if(ret==11) //黑色
				fprintf(cgiOut,"<td><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame><img src=%s width=21 height=14 border=0 alt=\"%s\"></td>",n,prt_no,"/images/SFP/sfp_11.jpg",search(lcon,"prt_cfg"));
				if(ret==1)
				fprintf(cgiOut,"<td><img src=/images/SFP/sfp_11.jpg width=21 height=14></td>");
				if(ret==2)
				fprintf(cgiOut,"<td><img src=/images/SFP/sfp_07.jpg width=21 height=14></td>");
				if(ret==3)
				fprintf(cgiOut,"<td><img src=/images/SFP/sfp_09.jpg width=21 height=14></td>");

				//fprintf(cgiOut,"<td>");
				//fprintf(cgiOut,"<img src=/images/SFP/sfp_07.jpg width=21 height=14 >");
				//fprintf(cgiOut,"</td>");

				fprintf(cgiOut,"<td rowspan=3>");
				fprintf(cgiOut,"<img src=/images/SFP/sfp_08.jpg width=12 height=32 >");
				fprintf(cgiOut,"</td>");
			}
			if(i==3 || i==5)
			{
				if(ret==22) //红色
				fprintf(cgiOut,"<td><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame><img src=%s width=21 height=14 border=0 alt=\"%s\"></td>",n,prt_no,"/images/SFP/sfp_07.jpg",search(lcon,"prt_cfg"));
				if(ret==33)  //绿色
				fprintf(cgiOut,"<td><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame><img src=%s width=21 height=14 border=0 alt=\"%s\"></td>",n,prt_no,"/images/SFP/sfp_09.jpg",search(lcon,"prt_cfg"));
				if(ret==11) //黑色
				fprintf(cgiOut,"<td><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame><img src=%s width=21 height=14 border=0 alt=\"%s\"></td>",n,prt_no,"/images/SFP/sfp_11.jpg",search(lcon,"prt_cfg"));
				if(ret==1)
				fprintf(cgiOut,"<td><img src=/images/SFP/sfp_11.jpg width=21 height=14></td>");
				if(ret==2)
				fprintf(cgiOut,"<td><img src=/images/SFP/sfp_07.jpg width=21 height=14></td>");
				if(ret==3)
				fprintf(cgiOut,"<td><img src=/images/SFP/sfp_09.jpg width=21 height=14></td>");

				//	fprintf(cgiOut,"<td>");
				//	fprintf(cgiOut,"<img src=/images/SFP/sfp_11.jpg width=21 height=14 >");
				//	fprintf(cgiOut,"</td>");

				fprintf(cgiOut,"<td rowspan=3>");
				fprintf(cgiOut,"<img src=/images/SFP/sfp_12.jpg width=11 height=32 >");
				fprintf(cgiOut,"</td>");
			}

		}
      ret=port_info(6,value, prt_no, pt,retu,n);
	  if(ret==22) //红色
	  fprintf(cgiOut,"<td><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame><img src=%s width=21 height=14 border=0 alt=\"%s\"></td>",n,prt_no,"/images/SFP/sfp_07.jpg",search(lcon,"prt_cfg"));
	  if(ret==33) //绿色
	  fprintf(cgiOut,"<td><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame><img src=%s width=21 height=14 border=0 alt=\"%s\"></td>",n,prt_no,"/images/SFP/sfp_09.jpg",search(lcon,"prt_cfg"));
      if(ret==11) //黑色
	  fprintf(cgiOut,"<td><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame><img src=%s width=21 height=14 border=0 alt=\"%s\"></td>",n,prt_no,"/images/SFP/sfp_11.jpg",search(lcon,"prt_cfg"));
      if(ret==1)
	  fprintf(cgiOut,"<td><img src=/images/SFP/sfp_11.jpg width=21 height=14></td>");
	  if(ret==2)
	  fprintf(cgiOut,"<td><img src=/images/SFP/sfp_07.jpg width=21 height=14></td>");
	  if(ret==3)
	  fprintf(cgiOut,"<td><img src=/images/SFP/sfp_09.jpg width=21 height=14></td>");
		//fprintf(cgiOut,"<td>");
			//fprintf(cgiOut,"<img src=/images/SFP/sfp_17.jpg width=21 height=14 >");
			//fprintf(cgiOut,"</td>");
			
		fprintf(cgiOut,"<td rowspan=3>");			
			fprintf(cgiOut,"<img src=/images/SFP/sfp_18.jpg width=20 height=32 ></td>"\
		"<td>"\
			"<img src=/images/SFP/sep.gif width=1 height=14 ></td>"\
	"</tr>"\
	"<tr>"\
		"<td rowspan=2>"\
			"<img src=/images/SFP/sfp_19.jpg width=19 height=18 ></td>"\
		"<td>"\
			"<img src=/images/SFP/sfp_20.jpg width=10 height=9 ></td>"\
		"<td rowspan=2>"\
			"<img src=/images/SFP/sfp_21.jpg width=17 height=18 ></td>"\
		"<td>"\
			"<img src=/images/SFP/sfp_22.jpg width=10 height=9 ></td>"\
		"<td rowspan=2>"\
			"<img src=/images/SFP/sfp_23.jpg width=19 height=18 ></td>"\
		"<td rowspan=2>"\
			"<img src=/images/SFP/sfp_24.jpg width=21 height=18 ></td>"\
		"<td rowspan=2>"\
			"<img src=/images/SFP/sfp_25.jpg width=21 height=18 ></td>"\
		"<td rowspan=2>"\
			"<img src=/images/SFP/sfp_26.jpg width=21 height=18 ></td>"\
		"<td rowspan=2>"\
			"<img src=/images/SFP/sfp_27.jpg width=21 height=18 ></td>"\
		"<td rowspan=2>"\
			"<img src=/images/SFP/sfp_28.jpg width=21 height=18 ></td>"\
		"<td rowspan=2>"\
			"<img src=/images/SFP/sfp_29.jpg width=21 height=18 ></td>"\
		"<td>"\
			"<img src=/images/SFP/sep.gif width=1 height=9 ></td>"\
	"</tr>"\
	"<tr>"\
		"<td>"\
			"<img src=/images/SFP/sfp_30.jpg width=10 height=9 ></td>"\
		"<td>"\
			"<img src=/images/SFP/sfp_31.jpg width=10 height=9 ></td>"\
		"<td>"\
			"<img src=/images/SFP/sep.gif width=1 height=9 ></td>"\
	    "</tr>"\
        "</table>");

}

/*显示端口信息*/
int port_info(int i,unsigned int value,char *prt_no,struct eth_port_s pt,int retu,char *n)
{
	  unsigned int dig;
	  dig=value;
	  value = dig;                 /*slot_no=2*/
	  value =  (value << 8) |i;    /*port_no=i*/
	  memset(prt_no,0,10);
	  sprintf(prt_no,"%d-%d",dig,i);     /*int转成char*/
      if(show_eth_port_atrr(value,0,&pt)==CCGI_SUCCESS)   /*如果读取port 信息成功*/
      {
        if(strcmp(onoff_status_str[(pt.attr_bitmap & ETH_ATTR_ADMIN_STATUS) >> ETH_ADMIN_STATUS_BIT],"OFF")==0)  /*admin_status=disable*/
        {
          if(retu==0)
		  	return 11; //黑色
		  else
		  	return 1; //黑色
        }
		else  /*admin_status=able*/
		{
          if(strcmp(link_status_str[(pt.attr_bitmap & ETH_ATTR_LINK_STATUS) >> ETH_LINK_STATUS_BIT],"UP")==0)   /*link_state=UP*/
          {
            if(retu==0)  /*管理员*/
              return 33; //绿色 管理
			else
				return 3; //绿色 
          	}
		  else    /*link_state=DOWN*/
 		  {
 		    if(retu==0)  /*管理员*/
 		    	return 22; //红色图片 管理
			else
				return 2; //红色图片
		  	}
		}
      }
	  else     /*读取port 信息失败,显示灰色 图片*/
	  	  {
	  	    if(retu==0)  /*管理员*/
	  	    	return 11; //黑色图片  管理
		else
			return 1; //黑色图片
	  	}

  //return 0; //没有要求
}

/*光口状态*/
int light_port(int i,unsigned int value,char *prt_no,struct eth_port_s pt,int retu,char *n)
{
	  
    value = 0;                   /*slot_no=0*/
    value =  (value << 8) |i;    /*port_no=i*/
    memset(prt_no,0,10);
    sprintf(prt_no,"%d-%d",0,i);     /*int转成char*/
    if(show_eth_port_atrr(value,0,&pt)==CCGI_SUCCESS)   /*如果读取port 信息成功*/
    {
    if(strcmp(eth_port_type_str[pt.port_type],"ETH_GE_SFP")==0)   /*type为光口*/
		
      return 1;  //黑色的
      
    else  /*type为电口*/
    {
      if(strcmp(onoff_status_str[(pt.attr_bitmap & ETH_ATTR_ADMIN_STATUS) >> ETH_ADMIN_STATUS_BIT],"OFF")==0)  /*admin_status=disable*/
      {
        if(retu==0)  /*管理员*/
           return 11; //黑色  管理
    	else
    	 return 1; //黑色
      }
      else  /*admin_status=able*/
      {
        if(strcmp(link_status_str[(pt.attr_bitmap & ETH_ATTR_LINK_STATUS) >> ETH_LINK_STATUS_BIT],"UP")==0)   /*link_state=UP*/
       	{
       	  if(retu==0)  /*管理员*/
          return 33; //绿色  管理
    	  else
    	   return 3; //绿色
        	}
    	else    /*link_state=DOWN*/
    	{
    	  if(retu==0)  /*管理员*/
          return 22;  //红色，管理
		  else
          return 2; //红色
    	}
      }
    }
    }
    else     /*读取port 信息失败,显示灰色图片*/
    {
    if(retu==0)  /*管理员*/
		return 11; //黑色 管理
    else
     return 1; //黑色
    }
	
    
    }

/*电口状态 方形*/
int elec_port(int i,unsigned int value,char *prt_no,struct eth_port_s pt,int retu,char *n)
{

    
      value = 0;                   /*slot_no=0*/
	  value =  (value << 8) |i;    /*port_no=i*/
	  memset(prt_no,0,10);
	  sprintf(prt_no,"%d-%d",0,i);     /*int转成char*/
	  if(show_eth_port_atrr(value,0,&pt)==CCGI_SUCCESS)   /*如果读取port 信息成功*/
	  {
        if(strcmp(eth_port_type_str[pt.port_type],"ETH_GE_SFP")!=0)   /*type为电口*/
        return 1 ; //黑色

		else    /*type为光口*/
		{
          if(strcmp(onoff_status_str[(pt.attr_bitmap & ETH_ATTR_ADMIN_STATUS) >> ETH_ADMIN_STATUS_BIT],"OFF")==0)  /*admin_status=disable*/
          {
            if(retu==0)  /*管理员*/
              return 11; //黑色 管理
			else
              return 1;  //黑色
		  }
		  else  /*admin_status=able*/
		  {
            if(strcmp(link_status_str[(pt.attr_bitmap & ETH_ATTR_LINK_STATUS) >> ETH_LINK_STATUS_BIT],"UP")==0)   /*link_state=UP*/
            {
              if(retu==0)  /*管理员*/
               return 33; //绿色 管理
 			  else
               return 3; //绿色
            }
			else    /*link_state=DOWN*/
			{
			  if(retu==0)  /*管理员*/			  	
              return 22;	  	
			  else
              return 2; //红色

			}
		  }
		}
	  }
	  else      /*读取port 信息失败,显示灰色 图片*/
	  {
	    if(retu==0)  /*管理员*/
          return 11; //黑色 管理
		else
        return 1; //黑色
	  }	 

}


/*7000的面板函数，根据产品id来判断*/
int panel_7k(struct sys_ver ptrsysver,unsigned int value,struct slot sl,struct eth_port_s pt,char *prt_no,int retu,char *n,struct list *lcon)
{
//ccgi_dbus_init();   //读取数据的函数初始化
//show_sys_ver(&ptrsysver);
int i;
int op_ret;

		
		fprintf(cgiOut,"<table  width=722 bgcolor=#555251 border=0 cellpadding=0 cellspacing=0>"\
	"<tr>"\
		"<td rowspan=13>");
			fprintf(cgiOut,"<img src=/images/panel/7000news_01.jpg width=17 height=267 >");
			fprintf(cgiOut,"</td>"\
		"<td colspan=7>"
			"<img src=/images/panel/7000news_02.jpg width=292 height=14 ></td>"\
		"<td colspan=2 rowspan=8>"
			"<img src=/images/panel/7000news_03.jpg width=398 height=60 ></td>"\
		"<td rowspan=13>");
			fprintf(cgiOut,"<img src=/images/panel/7000news_04.jpg width=17 height=267 >");
			fprintf(cgiOut,"</td>"\
		"<td>"\
			"<img src=/images/panel/sep.gif width=1 height=14 ></td>"\
	"</tr>"\
	"<tr>"\
		"<td rowspan=8>"
			"<img src=/images/panel/7000news_05.jpg width=3 height=47 ></td>"\
		"<td rowspan=6>"
			"<img src=/images/panel/7000news_06.jpg width=71 height=44 ></td>"\
		"<td colspan=5>"
			"<img src=/images/panel/7000news_07.jpg width=218 height=9 ></td>"\
		"<td>"\
			"<img src=/images/panel/sep.gif width=1 height=9 ></td>"\
	"</tr>"\
	"<tr>"\
		"<td colspan=4 rowspan=2>"
			"<img src=/images/panel/7000news_08.jpg width=207 height=17 ></td>"\
		"<td bgcolor=#ACACAC>"\
			"<img src=/images/panel/7000news_09.jpg width=10 height=9 ></td>"\
		"<td>"\
			"<img src=/images/panel/sep.gif width=1 height=9 ></td>"\
	"</tr>"\
	"<tr>"\
		"<td bgcolor=#ACACAC>"\
			"<img src=/images/panel/7000news_10.jpg width=10 height=8 ></td>"\
		"<td>"\
			"<img src=/images/panel/sep.gif width=1 height=8 ></td>"\
	"</tr>"\
	"<tr>"\
		"<td colspan=2>"
			"<img src=/images/panel/7000news_11.jpg width=190 height=2 ></td>"\
		"<td rowspan=2>"
			"<img src=/images/panel/7000news_12.jpg width=10 height=9 ></td>"\
		"<td rowspan=5>"
			"<img src=/images/panel/7000news_13.jpg width=7 height=21 ></td>"\
		"<td rowspan=2 bgcolor=#ACACAC>"
			"<img src=/images/panel/7000news_14.jpg width=10 height=9 ></td>"\
		"<td>"\
			"<img src=/images/panel/sep.gif width=1 height=2 ></td>"\
	"</tr>"\
	"<tr>");
			
	   
		
		fprintf(cgiOut,"<td rowspan=2 bgcolor=#ACACAC ></td>");
			//ptrsysver.product_name
			//fprintf(cgiOut,"<img src=/images/panel/7000news_15.jpg width=87 height=16 >");
			
		fprintf(cgiOut,"<td rowspan=4>"
			"<img src=/images/panel/7000news_16.jpg width=103 height=19 alt=\"%s\"></td>",ptrsysver.sw_product_name);
		fprintf(cgiOut,"<td>"\
			"<img src=/images/panel/sep.gif width=1 height=7 ></td>"\
	"</tr>"\
	"<tr>"\
		"<td rowspan=3>"
			"<img src=/images/panel/7000news_17.jpg width=10 height=12 ></td>"\
		"<td rowspan=3>"
			"<img src=/images/panel/7000news_18.jpg width=11 height=12 ></td>"\
		"<td>"\
			"<img src=/images/panel/sep.gif width=1 height=9 ></td>"\
	"</tr>"\
	"<tr>"\
		"<td colspan=2 rowspan=2>"
			"<img src=/images/panel/7000news_19.jpg width=158 height=3 ></td>"\
		"<td>"\
			"<img src=/images/panel/sep.gif width=1 height=2 ></td>"\
	"</tr>"\
	"<tr>"\
		"<td colspan=2>"\
			"<img src=/images/panel/7000news_20.jpg width=397 height=1 ></td>"\
		"<td>"\
			"<img src=/images/panel/sep.gif width=1 height=1 ></td>"\
	"</tr>"\
	"<tr>"\
		"<td colspan=8>");
		/*-------------------------begin-slot-3---------------------------------*/
    	
	if(nm_show_hw_config(3,&sl)==CCGI_SUCCESS)     /*如果读取信息成功*/
	{
	  if(sl.module_status==2)   /*slot_status=RUNNING*/
	  select_id(sl.module_id,3,pt,prt_no,retu,n,lcon,sl.modname);	
      else        /*slot_status=NONE or INITING or DISABLED*/
	   fprintf(cgiOut,"<img src=/images/panel/7000news_23.jpg width=344 height=55 >");
	}
	else              /*读取信息失败,显示空slot*/
	  fprintf(cgiOut,"<img src=/images/panel/7000news_23.jpg width=344 height=55 >");

		/*-------------------------end-slot-3-----------------------------------*/
		//	fprintf(cgiOut,"<img src=/images/panel/7000news_21.jpg width=344 height=55 ="">");
			fprintf(cgiOut,"</td>"\
		"<td>");



		

		/*--------------------------begin slot-4-------------------------------*/

		    	
	if(nm_show_hw_config(4,&sl)==CCGI_SUCCESS)     /*如果读取信息成功*/
	{
	  if(sl.module_status==2)   /*slot_status=RUNNING*/
	  select_id(sl.module_id,4,pt,prt_no,retu,n,lcon,sl.modname);	
	  //fprintf(cgiOut,"moduleid: %d",sl.module_id);
      else        /*slot_status=NONE or INITING or DISABLED*/
	   fprintf(cgiOut,"<img src=/images/panel/7000news_23.jpg width=344 height=55 >");
	}
	else              /*读取信息失败,显示空slot*/
	  fprintf(cgiOut,"<img src=/images/panel/7000news_23.jpg width=344 height=55 >");
		/*--------------------------end slot-4----------------------------------*/
		//	fprintf(cgiOut,"<img src=/images/panel/7000news_22.jpg width=344 height=55 ="">");
			fprintf(cgiOut,"</td>"\
		"<td>"\
			"<img src=/images/panel/sep.gif width=1 height=55 ></td>"\
	"</tr>"\
	"<tr>"\
		"<td colspan=8>");


		/*-------------------------begin slot-1-------------------------------*/
       	
	if(nm_show_hw_config(1,&sl)==CCGI_SUCCESS)     /*如果读取信息成功*/
	{
	  if(sl.module_status==2)   /*slot_status=RUNNING*/
	  select_id(sl.module_id,1,pt,prt_no,retu,n,lcon,sl.modname);	
	  //fprintf(cgiOut,"moduleid: %d   modify: %d",sl.module_id,MODULE_ID_AX7_6GTX);
      else        /*slot_status=NONE or INITING or DISABLED*/
	   fprintf(cgiOut,"<img src=/images/panel/7000news_23.jpg width=344 height=55 >");
	}
	else              /*读取信息失败,显示空slot*/
	  fprintf(cgiOut,"<img src=/images/panel/7000news_23.jpg width=344 height=55 >");

		/*-------------------------end slot-1----------------------------------*/
		
		
			//fprintf(cgiOut,"<img src=/images/panel/7000news_23.jpg width=344 height=55 >");
			fprintf(cgiOut,"</td>");
		fprintf(cgiOut,"<td >");
		




		/*-------------------------begin slot-2--------------------------------*/
		
       if(nm_show_hw_config(2,&sl)==CCGI_SUCCESS)     /*如果读取信息成功*/
	{

	 
	  
	  if(sl.module_status==2)   /*slot_status=RUNNING*/   	
	  	{
	   select_id(sl.module_id,2,pt,prt_no,retu,n,lcon,sl.modname);	 
	   //fprintf(cgiOut,"module_id %d",sl.module_id);
	  	}
	   else        /*slot_status=NONE or INITING or DISABLED*/
	   fprintf(cgiOut,"<img src=/images/panel/7000news_24.jpg width=344 height=55 >");
	}
	else              /*读取信息失败,显示空slot*/
	 fprintf(cgiOut,"<img src=/images/panel/7000news_24.jpg width=344 height=55 >");
	
		/*-------------------------end slot-2----------------------------------*/

		//	fprintf(cgiOut,"<img src=/images/panel/7000news_24.jpg width=344 height=55 >");
			fprintf(cgiOut,"</td>"\
		"<td>"\
			"<img src=/images/panel/sep.gif width=1 height=55 ></td>"\
	"</tr>"\
	"<tr>"\
		"<td colspan=9>");

	

			/*--------------------------begin   CRSMU -----------------------------------*/

 
  fprintf(cgiOut,"<table width=689  height=67 border=0 cellpadding=0 cellspacing=0>"
	"<tr>"\
		"<td colspan=8>"\
			"<img src=/images/crmsu/crmsu_01.jpg width=123 height=3 ></td>"\
		"<td rowspan=11>"\
			"<img src=/images/crmsu/crmsu_02.jpg width=261 height=67 ></td>"\
		"<td colspan=7 rowspan=4>"\
			"<img src=/images/crmsu/crmsu_03.jpg width=122 height=31 ></td>"\
		"<td rowspan=11>"\
			"<img src=/images/crmsu/crmsu_04.jpg width=38 height=67 ></td>"\
		"<td colspan=8 rowspan=3>"\
			"<img src=/images/crmsu/crmsu_05.jpg width=144 height=30 ></td>"\
		"<td>"\
			"<img src=/images/crmsu/sep.gif width=1 height=3 ></td>"\
	"</tr>"\
	"<tr>"\
		"<td rowspan=10>"
			"<img src=/images/crmsu/crmsu_06.jpg width=13 height=64 ></td>");
			
   if(nm_show_hw_config(0,&sl)==CCGI_SUCCESS)     /*如果读取信息成功*/
	{
	  if(sl.module_status==2)   /*slot_status=RUNNING*/   	
	  {
	  	fprintf(cgiOut,"<td colspan=5 bgcolor=#C3C3C3 height=14>");
	    fprintf(cgiOut,"</td>");
	  }
	  else
	  {
	  	fprintf(cgiOut,"<td colspan=5 bgcolor=#C3C3C3 height=14>");
	    fprintf(cgiOut,"</td>");
	  }	
	}else
    {
	  	fprintf(cgiOut,"<td colspan=5 bgcolor=#C3C3C3 height=14>");
	    fprintf(cgiOut,"</td>");
	}	
		
		fprintf(cgiOut,"<td colspan=2 rowspan=4>"\
			"<img src=/images/crmsu/crmsu_08.jpg width=37 height=29 ></td>"\
		"<td>"\
			"<img src=/images/crmsu/sep.gif width=1 height=14 ></td>"\
	"</tr>"\
	"<tr>"\
		"<td colspan=5 rowspan=4>"\
			"<img src=/images/crmsu/crmsu_09.jpg width=73 height=22 ></td>"\
		"<td>"\
			"<img src=/images/crmsu/sep.gif width=1 height=13 ></td>"\
	"</tr>"\
	"<tr>");
			/*电口 方形*/
		for(i=1;i<4;i++)
			{
		op_ret=elec_port(i,0, prt_no,pt,retu,n);
		if(i==1 || i==3)
		{
		if(op_ret==11)
		fprintf(cgiOut,"<td rowspan=4><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame><img src=%s width=21 height=14 border=0 alt=\"%s\"></td>",n,prt_no,"/images/crmsu/crmsu_14.jpg",search(lcon,"prt_cfg"));
		if(op_ret==22)
		fprintf(cgiOut,"<td rowspan=4><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame><img src=%s width=21 height=14 border=0 alt=\"%s\"></td>",n,prt_no,"/images/crmsu/crmsu_10.jpg",search(lcon,"prt_cfg"));
		if(op_ret==33)
		fprintf(cgiOut,"<td rowspan=4><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame><img src=%s width=21 height=14 border=0 alt=\"%s\"></td>",n,prt_no,"/images/crmsu/crmsu_12.jpg",search(lcon,"prt_cfg"));
		if(op_ret==1)
		fprintf(cgiOut,"<td rowspan=4><img src=/images/crmsu/crmsu_14.jpg width=21 height=14 ></td>");
		if(op_ret==2)
		fprintf(cgiOut,"<td rowspan=4><img src=/images/crmsu/crmsu_10.jpg width=21 height=14 ></td>");
		if(op_ret==3)
		fprintf(cgiOut,"<td rowspan=4><img src=/images/crmsu/crmsu_12.jpg width=21 height=14 ></td>");
				
		fprintf(cgiOut,"<td rowspan=8>"\
			"<img src=/images/crmsu/crmsu_11.jpg width=11 height=37 ></td>");
		}
		if(i==2)
		{
        if(op_ret==11)
		fprintf(cgiOut,"<td rowspan=4><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame><img src=%s width=21 height=14 border=0 alt=\"%s\"></td>",n,prt_no,"/images/crmsu/crmsu_14.jpg",search(lcon,"prt_cfg"));
		if(op_ret==22)
		fprintf(cgiOut,"<td rowspan=4><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame><img src=%s width=21 height=14 border=0 alt=\"%s\"></td>",n,prt_no,"/images/crmsu/crmsu_10.jpg",search(lcon,"prt_cfg"));
		if(op_ret==33)
		fprintf(cgiOut,"<td rowspan=4><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame><img src=%s width=21 height=14 border=0 alt=\"%s\"></td>",n,prt_no,"/images/crmsu/crmsu_12.jpg",search(lcon,"prt_cfg"));
		if(op_ret==1)
		fprintf(cgiOut,"<td rowspan=4><img src=/images/crmsu/crmsu_14.jpg width=21 height=14 ></td>");
		if(op_ret==2)
		fprintf(cgiOut,"<td rowspan=4><img src=/images/crmsu/crmsu_10.jpg width=21 height=14 ></td>");
		if(op_ret==3)
		fprintf(cgiOut,"<td rowspan=4><img src=/images/crmsu/crmsu_12.jpg width=21 height=14 ></td>");
				
		fprintf(cgiOut,"<td rowspan=8>"\
			"<img src=/images/crmsu/crmsu_11.jpg width=12 height=37 ></td>");

		}
       }

		

		op_ret=elec_port(4,0,prt_no,pt,retu,n);
		
		if(op_ret==11)
		fprintf(cgiOut,"<td rowspan=4><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame><img src=%s width=21 height=14 border=0 alt=\"%s\"></td>",n,prt_no,"/images/crmsu/crmsu_14.jpg",search(lcon,"prt_cfg"));
		if(op_ret==22)
		fprintf(cgiOut,"<td rowspan=4><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame><img src=%s width=21 height=14 border=0 alt=\"%s\"></td>",n,prt_no,"/images/crmsu/crmsu_10.jpg",search(lcon,"prt_cfg"));
		if(op_ret==33)
		fprintf(cgiOut,"<td rowspan=4><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame><img src=%s width=21 height=14 border=0 alt=\"%s\"></td>",n,prt_no,"/images/crmsu/crmsu_12.jpg",search(lcon,"prt_cfg"));
		if(op_ret==1)
		fprintf(cgiOut,"<td rowspan=4><img src=/images/crmsu/crmsu_14.jpg width=21 height=14 ></td>");
		if(op_ret==2)
		fprintf(cgiOut,"<td rowspan=4><img src=/images/crmsu/crmsu_10.jpg width=21 height=14 ></td>");
		if(op_ret==3)
		fprintf(cgiOut,"<td rowspan=4><img src=/images/crmsu/crmsu_12.jpg width=21 height=14 ></td>");
			
		fprintf(cgiOut,"<td rowspan=8>"\
			"<img src=/images/crmsu/crmsu_17.jpg width=26 height=37 ></td>"\
		"<td>"\
			"<img src=/images/crmsu/sep.gif width=1 height=1 ></td>"\
	"</tr>"\
	"<tr>");

		/*光口*/
		
		for(i=1;i<4;i++){
		op_ret=light_port(i,0,prt_no,pt,retu,n);
		if(op_ret==11)
		fprintf(cgiOut,"<td rowspan=5><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame><img src=%s width=23 height=19 border=0 alt=\"%s\"></td>",n,prt_no,"/images/crmsu/crmsu_22.jpg",search(lcon,"prt_cfg"));
		if(op_ret==22)
		fprintf(cgiOut,"<td rowspan=5><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame><img src=%s width=23 height=19 border=0 alt=\"%s\"></td>",n,prt_no,"/images/crmsu/crmsu_18.jpg",search(lcon,"prt_cfg"));
		if(op_ret==33)
		fprintf(cgiOut,"<td rowspan=5><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame><img src=%s width=23 height=19 border=0 alt=\"%s\"></td>",n,prt_no,"/images/crmsu/crmsu_20.jpg",search(lcon,"prt_cfg"));
		if(op_ret==1)
		fprintf(cgiOut,"<td rowspan=5><img src=/images/crmsu/crmsu_22.jpg width=23 height=19 ></td>");
		if(op_ret==2)
		fprintf(cgiOut,"<td rowspan=5><img src=/images/crmsu/crmsu_18.jpg width=23 height=19 ></td>");
		if(op_ret==3)
		fprintf(cgiOut,"<td rowspan=5><img src=/images/crmsu/crmsu_20.jpg width=23 height=19 ></td>");
		
		
		fprintf(cgiOut,"<td rowspan=7>"
			"<img src=/images/crmsu/crmsu_19.jpg width=10 height=36 ></td>");
			}

        op_ret=light_port(4,0,prt_no,pt,retu,n);
		if(op_ret==11)
		fprintf(cgiOut,"<td rowspan=5><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame><img src=%s width=23 height=19 border=0 alt=\"%s\"></td>",n,prt_no,"/images/crmsu/crmsu_22.jpg",search(lcon,"prt_cfg"));
		if(op_ret==22)
		fprintf(cgiOut,"<td rowspan=5><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame><img src=%s width=23 height=19 border=0 alt=\"%s\"></td>",n,prt_no,"/images/crmsu/crmsu_18.jpg",search(lcon,"prt_cfg"));
		if(op_ret==33)
		fprintf(cgiOut,"<td rowspan=5><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame><img src=%s width=23 height=19 border=0 alt=\"%s\"></td>",n,prt_no,"/images/crmsu/crmsu_20.jpg",search(lcon,"prt_cfg"));
		if(op_ret==1)
		fprintf(cgiOut,"<td rowspan=5><img src=/images/crmsu/crmsu_22.jpg width=23 height=19 ></td>");
		if(op_ret==2)
		fprintf(cgiOut,"<td rowspan=5><img src=/images/crmsu/crmsu_18.jpg width=23 height=19 ></td>");
		if(op_ret==3)
		fprintf(cgiOut,"<td rowspan=5><img src=/images/crmsu/crmsu_20.jpg width=23 height=19 ></td>");
				
		fprintf(cgiOut,"<td>");
			fprintf(cgiOut,"<img src=/images/crmsu/sep.gif width=1 height=1 ></td>");
		
		
	fprintf(cgiOut,"</tr>"\
	"<tr>"\
		"<td rowspan=6>"
			"<img src=/images/crmsu/crmsu_25.jpg width=14 height=35 ></td>"\
		"<td rowspan=5>"\
			"<img src=/images/crmsu/crmsu_26.jpg width=23 height=19 ></td>"\
		"<td>"\
			"<img src=/images/crmsu/sep.gif width=1 height=7 ></td>"\
	"</tr>"\
	"<tr>"\
		"<td rowspan=5>"\
			"<img src=/images/crmsu/crmsu_27.jpg width=14 height=28 ></td>"\
		"<td rowspan=2>"
			"<img src=/images/crmsu/crmsu_28.jpg width=10 height=9 ></td>"\
		"<td rowspan=5>"\
			"<img src=/images/crmsu/crmsu_29.jpg width=17 height=28 ></td>"\
		"<td rowspan=2>"
			"<img src=/images/crmsu/crmsu_30.jpg width=10 height=9 ></td>"\
		"<td rowspan=5>"\
			"<img src=/images/crmsu/crmsu_31.jpg width=22 height=28 ></td>"\
		"<td>"\
			"<img src=/images/crmsu/sep.gif width=1 height=5 ></td>"\
	"</tr>"\
	"<tr>"\
		"<td rowspan=4>"\
			"<img src=/images/crmsu/crmsu_32.jpg width=21 height=23 ></td>"\
		"<td rowspan=4>"\
			"<img src=/images/crmsu/crmsu_33.jpg width=21 height=23 ></td>"\
		"<td rowspan=4>"\
			"<img src=/images/crmsu/crmsu_34.jpg width=21 height=23 ></td>"\
		"<td rowspan=4>"\
			"<img src=/images/crmsu/crmsu_35.jpg width=21 height=23 ></td>"\
		"<td>"\
			"<img src=/images/crmsu/sep.gif width=1 height=4 ></td>"\
	"</tr>"\
	"<tr>"\
		"<td rowspan=3>"\
			"<img src=/images/crmsu/crmsu_36.jpg width=10 height=19 ></td>"\
		"<td rowspan=3>"\
			"<img src=/images/crmsu/crmsu_37.jpg width=10 height=19 ></td>"\
		"<td>"\
			"<img src=/images/crmsu/sep.gif width=1 height=2 ></td>"\
	"</tr>"\
	"<tr>"\
		"<td rowspan=2>"
			"<img src=/images/crmsu/crmsu_38.jpg width=23 height=17 ></td>"\
		"<td rowspan=2>"
			"<img src=/images/crmsu/crmsu_39.jpg width=23 height=17 ></td>"\
		"<td rowspan=2>"
			"<img src=/images/crmsu/crmsu_40.jpg width=23 height=17 ></td>"\
		"<td rowspan=2>"
			"<img src=/images/crmsu/crmsu_41.jpg width=23 height=17 ></td>"\
		"<td>"\
			"<img src=/images/crmsu/sep.gif width=1 height=1 ></td>"\
	"</tr>"\
	"<tr>"\
		"<td>"\
			"<img src=/images/crmsu/crmsu_42.jpg width=23 height=16 ></td>"\
		"<td>"\
			"<img src=/images/crmsu/sep.gif width=1 height=16 ></td>"\
	"</tr>"\
"</table>");
		/*------------------------------end  CRSMU-----------------------------------*/

		
			//fprintf(cgiOut,"<img src=/images/panel/7000news_25.jpg width=688 height=67 >");
			
			fprintf(cgiOut,"</td>"\
		"<td>"\
			"<img src=/images/panel/sep.gif width=1 height=67 ></td>"\
	"</tr>"\
	"<tr>"\
		"<td colspan=9 >"\
			"<img src=/images/panel/7000news_26.jpg width=690 height=29 ></td>"\
		"<td>"\
			"<img src=/images/panel/sep.gif width=1 height=29 ></td>"\
	"</tr>"\
	"<tr>"\
		"<td>"\
			"<img src=/images/panel/sep.gif width=16 height=1 ></td>"\
		"<td>"\
			"<img src=/images/panel/sep.gif width=3 height=1 ></td>"\
		"<td>"\
			"<img src=/images/panel/sep.gif width=71 height=1 ></td>"\
		"<td>"\
			"<img src=/images/panel/sep.gif width=87 height=1 ></td>"\
		"<td>"\
			"<img src=/images/panel/sep.gif width=103 height=1 ></td>"\
		"<td>"\
			"<img src=/images/panel/sep.gif width=10 height=1 ></td>"\
		"<td>"\
			"<img src=/images/panel/sep.gif width=7 height=1 ></td>"\
		"<td>"\
			"<img src=/images/panel/sep.gif width=10 height=1 ></td>"\
		"<td>"\
			"<img src=/images/panel/sep.gif width=53 height=1 ></td>"\
		"<td>"\
			"<img src=/images/panel/sep.gif width=344 height=1 ></td>"\
		"<td>"\
			"<img src=/images/panel/sep.gif width=17 height=1 ></td>"\
		"<td></td>"\
	"</tr>"\
"</table>");


return 0;
}

void  panel_7605i()
	{
		int ret = 0;
		char path_buf[128] = {0};	
		
		fprintf(cgiOut,"<table id=\"Table_01\" width=\"719\" height=\"251\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\">"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/7605i/7605i_01.gif\" width=\"719\" height=\"67\" alt=\"\"></td>"\
			"</tr>"\
			"<tr>");
		
		memset(path_buf,0,sizeof(path_buf));
		sprintf(path_buf, "/dbm/product/slot/slot%d/board_state", 3);
		ret = get_int_from_file(path_buf);
		if(0 == ret)
		{
			fprintf(cgiOut,"<td><img src=\"/images/7605i/bk7605i_02.gif\" width=\"719\" height=\"59\" alt=\"\"></td>");
		}
		else	
		{
			fprintf(cgiOut,"<td><table width=\"719\" height=\"59\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\">"\
		"<tr>"\
			"<td colspan=\"5\">"\
				"<img src=\"/images/7605i/7605i_02_01.gif\" width=\"719\" height=\"3\" ></td>"\
		"</tr>"\
		"<tr>"\
			"<td rowspan=\"2\">"\
				"<img src=\"/images/7605i/7605i_02_02.gif\" width=\"310\" height=\"56\" ></td>"\
			"<td width=\"167\" height=\"48\" alt=\"\">");
		//ports
		//fprintf(cgiOut,"<img src=\"/images/7605i/7605i_02_03.gif\" width=\"167\" height=\"48\" >");
		fprintf(cgiOut,"<table table id=__01 width=\"167\" height=\"48\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\">"\
		"<tr>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_6_4.gif\" width=\"6\" height=\"4\" alt=\"\"></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_20_4.gif\" width=\"20\" height=\"4\" alt=\"\"></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_4.gif\" width=\"7\" height=\"4\" alt=\"\"></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_20_4.gif\" width=\"20\" height=\"4\" alt=\"\"></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_4.gif\" width=\"7\" height=\"4\" alt=\"\"></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_20_4.gif\" width=\"20\" height=\"4\" alt=\"\"></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_4.gif\" width=\"7\" height=\"4\" alt=\"\"></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_20_4.gif\" width=\"20\" height=\"4\" alt=\"\"></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_4.gif\" width=\"7\" height=\"4\" alt=\"\"></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_20_4.gif\" width=\"20\" height=\"4\" alt=\"\"></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_4.gif\" width=\"7\" height=\"4\" alt=\"\"></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_20_4.gif\" width=\"20\" height=\"4\" alt=\"\"></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_6_4.gif\" width=\"6\" height=\"4\" alt=\"\"></td>"\
		"</tr>"\
		"<tr>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_6_18.gif\" width=\"6\" height=\"18\" alt=\"\"></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_lup_black.gif\" width=\"20\" height=\"18\" alt=\"\"></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_18.gif\" width=\"7\" height=\"18\" alt=\"\"></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_lup_black.gif\" width=\"20\" height=\"18\" alt=\"\"></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_18.gif\" width=\"7\" height=\"18\" alt=\"\"></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_lup_black.gif\" width=\"20\" height=\"18\" alt=\"\"></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_18.gif\" width=\"7\" height=\"18\" alt=\"\"></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_lup_black.gif\" width=\"20\" height=\"18\" alt=\"\"></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_18.gif\" width=\"7\" height=\"18\" alt=\"\"></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_lup_black.gif\" width=\"20\" height=\"18\" alt=\"\"></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_18.gif\" width=\"7\" height=\"18\" alt=\"\"></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_lup_black.gif\" width=\"20\" height=\"18\" alt=\"\"></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_6_18_r.gif\" width=\"6\" height=\"18\" alt=\"\"></td>"\
		"</tr>"\
		"<tr>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_6_3.gif\" width=\"6\" height=\"3\" alt=\"\"></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_20_3.gif\" width=\"20\" height=\"3\" alt=\"\"></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_3.gif\" width=\"7\" height=\"3\" alt=\"\"></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_20_3.gif\" width=\"20\" height=\"3\" alt=\"\"></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_3.gif\" width=\"7\" height=\"3\" alt=\"\"></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_20_3.gif\" width=\"20\" height=\"3\" alt=\"\"></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_3.gif\" width=\"7\" height=\"3\" alt=\"\"></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_20_3.gif\" width=\"20\" height=\"3\" alt=\"\"></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_3.gif\" width=\"7\" height=\"3\" alt=\"\"></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_20_3.gif\" width=\"20\" height=\"3\" alt=\"\"></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_3.gif\" width=\"7\" height=\"3\" alt=\"\"></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_20_3.gif\" width=\"20\" height=\"3\" alt=\"\"></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_6_3.gif\" width=\"6\" height=\"3\" alt=\"\"></td>"\
		"</tr>"\
		"<tr>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_6_18.gif\" width=\"6\" height=\"18\" alt=\"\"></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_ldw_black.gif\" width=\"20\" height=\"18\" alt=\"\"></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_18.gif\" width=\"7\" height=\"18\" alt=\"\"></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_ldw_black.gif\" width=\"20\" height=\"18\" alt=\"\"></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_18.gif\" width=\"7\" height=\"18\" alt=\"\"></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_ldw_black.gif\" width=\"20\" height=\"18\" alt=\"\"></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_18.gif\" width=\"7\" height=\"18\" alt=\"\"></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_ldw_black.gif\" width=\"20\" height=\"18\" alt=\"\"></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_18.gif\" width=\"7\" height=\"18\" alt=\"\"></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_ldw_black.gif\" width=\"20\" height=\"18\" alt=\"\"></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_18.gif\" width=\"7\" height=\"18\" alt=\"\"></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_ldw_black.gif\" width=\"20\" height=\"18\" alt=\"\"></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_6_18_r.gif\" width=\"6\" height=\"18\" alt=\"\"></td>"\
		"</tr>"\
		"<tr>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_6_5.gif\" width=\"6\" height=\"5\" alt=\"\"></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_20_5.gif\" width=\"20\" height=\"5\" alt=\"\"></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_5.gif\" width=\"7\" height=\"5\" alt=\"\"></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_20_5.gif\" width=\"20\" height=\"5\" alt=\"\"></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_5.gif\" width=\"7\" height=\"5\" alt=\"\"></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_20_5.gif\" width=\"20\" height=\"5\" alt=\"\"></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_5.gif\" width=\"7\" height=\"5\" alt=\"\"></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_20_5.gif\" width=\"20\" height=\"5\" alt=\"\"></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_5.gif\" width=\"7\" height=\"5\" alt=\"\"></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_20_5.gif\" width=\"20\" height=\"5\" alt=\"\"></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_5.gif\" width=\"7\" height=\"5\" alt=\"\"></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_20_5.gif\" width=\"20\" height=\"5\" alt=\"\"></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_6_5_r.gif\" width=\"6\" height=\"5\" alt=\"\"></td>"\
		"</tr>"\
	"</table>");
		fprintf(cgiOut,"</td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_02_04.gif\" width=\"31\" height=\"48\" ></td>"\
			"<td>");
		//ports
		//fprintf(cgiOut,"<img src=\"/images/7605i/7605i_02_05.gif\" width=\"167\" height=\"48\" >");
		fprintf(cgiOut,"<table id=Table_01 width=167 height=48 border=0 cellpadding=0 cellspacing=0>"\
		"<tr>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_6_10.gif\" width=6 height=10 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_20_10.gif\" width=20 height=10 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_10.gif\" width=7 height=10 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_20_10.gif\" width=20 height=10 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_10.gif\" width=7 height=10 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_20_10.gif\" width=20 height=10 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_10.gif\" width=7 height=10 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_20_10.gif\" width=20 height=10 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_10.gif\" width=7 height=10 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_20_10.gif\" width=20 height=10 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_10.gif\" width=7 height=10 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_20_10.gif\" width=20 height=10 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_6_10_r.gif\" width=6 height=10 ></td>"\
		"</tr>"\
		"<tr>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_6_12.gif\" width=6 height=12 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_e_black.gif\" width=20 height=12 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_12.gif\" width=7 height=12 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_e_black.gif\" width=20 height=12 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_12.gif\" width=7 height=12 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_e_black.gif\" width=20 height=12 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_12.gif\" width=7 height=12 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_e_black.gif\" width=20 height=12 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_12.gif\" width=7 height=12 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_e_black.gif\" width=20 height=12 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_12.gif\" width=7 height=12 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_e_black.gif\" width=20 height=12 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_6_12_r.gif\" width=6 height=12 ></td>"\
		"</tr>"\
		"<tr>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_6_3.gif\" width=6 height=3 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_20_3.gif\" width=20 height=3 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_3.gif\" width=7 height=3 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_20_3.gif\" width=20 height=3 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_3.gif\" width=7 height=3 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_20_3.gif\" width=20 height=3 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_3.gif\" width=7 height=3 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_20_3.gif\" width=20 height=3 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_3.gif\" width=7 height=3 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_20_3.gif\" width=20 height=3 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_3.gif\" width=7 height=3 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_20_3.gif\" width=20 height=3 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_6_3_r.gif\" width=6 height=3 ></td>"\
		"</tr>"\
		"<tr>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_6_12.gif\" width=6 height=12 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_e_black.gif\" width=20 height=12 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_12.gif\" width=7 height=12 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_e_black.gif\" width=20 height=12 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_12.gif\" width=7 height=12 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_e_black.gif\" width=20 height=12 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_12.gif\" width=7 height=12 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_e_black.gif\" width=20 height=12 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_12.gif\" width=7 height=12 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_e_black.gif\" width=20 height=12 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_12.gif\" width=7 height=12 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_e_black.gif\" width=20 height=12 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_6_12_r.gif\" width=6 height=12 ></td>"\
		"</tr>"\
		"<tr>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_6_11.gif\" width=6 height=11 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_20_11.gif\" width=20 height=11 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_11.gif\" width=7 height=11 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_20_11.gif\" width=20 height=11 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_11.gif\" width=7 height=11 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_20_11.gif\" width=20 height=11 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_11.gif\" width=7 height=11 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_20_11.gif\" width=20 height=11 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_11.gif\" width=7 height=11 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_20_11.gif\" width=20 height=11 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_11.gif\" width=7 height=11 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_20_11.gif\" width=20 height=11 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_6_11_r.gif\" width=6 height=11 ></td>"\
		"</tr>"\
	"</table>");
		fprintf(cgiOut,"</td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_02_06.gif\" width=\"44\" height=\"48\" ></td>"\
		"</tr>"\
		"<tr>"\
			"<td colspan=\"4\">"\
				"<img src=\"/images/7605i/7605i_02_07.gif\" width=\"409\" height=\"8\" ></td>"\
		"</tr>"\
	"</table></td>");
		}
		fprintf(cgiOut,"</tr>"\
			"<tr>");
	
		memset(path_buf,0,sizeof(path_buf));
		sprintf(path_buf, "/dbm/product/slot/slot%d/board_state", 2);
		ret = get_int_from_file(path_buf);
		if(0 == ret)
		{
			fprintf(cgiOut,"<td>"\
				"<img src=\"/images/7605i/bk7605i_03.gif\" width=\"719\" height=\"62\" alt=\"\"></td>");
		}
		else	
		{
			fprintf(cgiOut,"<td>");
			
			//fprintf(cgiOut,"<img src=\"/images/7605i/7605i_03.gif\" width=\"719\" height=\"62\" alt=\"\">");
			
			fprintf(cgiOut,"<table id=Table_01 width=719 height=62 border=0 cellpadding=0 cellspacing=0>"\
				"<tr>"\
					"<td rowspan=3>"\
						"<img src=\"/images/7605i/7605i_03_01.gif\" width=364 height=62 ></td>"\
					"<td colspan=4>"\
						"<img src=\"/images/7605i/7605i_03_02.gif\" width=355 height=16 ></td>"\
				"</tr>"\
				"<tr>"\
					"<td>");
			//ports
				//fprintf(cgiOut,"<img src=\"/images/7605i/7605i_03_03.gif\" width=113 height=25 >");
			fprintf(cgiOut,"<table id=Table_01 width=113 height=25 border=0 cellpadding=0 cellspacing=0>"\
			"<tr>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_6_3.gif\" width=6 height=3 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_20_3.gif\" width=20 height=3 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_3.gif\" width=7 height=3 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_20_3.gif\" width=20 height=3 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_3.gif\" width=7 height=3 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_20_3.gif\" width=20 height=3 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_3.gif\" width=7 height=3 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_20_3.gif\" width=20 height=3 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_6_3_r.gif\" width=6 height=3 ></td>"\
		"</tr>"\
		"<tr>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_6_18.gif\" width=6 height=18 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_lup_black.gif\" width=20 height=18 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_18.gif\" width=7 height=18 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_lup_black.gif\" width=20 height=18 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_18.gif\" width=7 height=18 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_lup_black.gif\" width=20 height=18 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_18.gif\" width=7 height=18 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_lup_black.gif\" width=20 height=18 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_6_18_r.gif\" width=6 height=18 ></td>"\
		"</tr>"\
		"<tr>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_6_4.gif\" width=6 height=4 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_20_4.gif\" width=20 height=4 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_4.gif\" width=7 height=4 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_20_4.gif\" width=20 height=4 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_4.gif\" width=7 height=4 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_20_4.gif\" width=20 height=4 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_4.gif\" width=7 height=4 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_20_4.gif\" width=20 height=4 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_6_4_r.gif\" width=6 height=4 ></td>"\
			"</tr>"\
		"</table>");
				fprintf(cgiOut,"</td>"\
					"<td>"\
						"<img src=\"/images/7605i/7605i_03_04.gif\" width=85 height=25 ></td>"\
					"<td>");
			//ports
				//fprintf(cgiOut,"<img src=\"/images/7605i/7605i_03_05.gif\" width=113 height=25 >");
			fprintf(cgiOut,"<table id=Table_01 width=113 height=25 border=0 cellpadding=0 cellspacing=0>"\
		"<tr>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_6_6.gif\" width=6 height=6 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_20_6.gif\" width=20 height=6 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_6.gif\" width=7 height=6 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_20_6.gif\" width=20 height=6 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_6.gif\" width=7 height=6 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_20_6.gif\" width=20 height=6 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_6.gif\" width=7 height=6 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_20_6.gif\" width=20 height=6 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_6_6_r.gif\" width=6 height=6 ></td>"\
		"</tr>"\
		"<tr>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_6_12.gif\" width=6 height=12 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_e_black.gif\" width=20 height=12 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_12.gif\" width=7 height=12 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_e_black.gif\" width=20 height=12 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_12.gif\" width=7 height=12 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_e_black.gif\" width=20 height=12 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_12.gif\" width=7 height=12 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_e_black.gif\" width=20 height=12 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_6_12_r.gif\" width=6 height=12 ></td>"\
		"</tr>"\
		"<tr>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_6_7.gif\" width=6 height=7 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_20_7.gif\" width=20 height=7 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_7.gif\" width=7 height=7 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_20_7.gif\" width=20 height=7 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_7.gif\" width=7 height=7 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_20_7.gif\" width=20 height=7 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_7.gif\" width=7 height=7 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_20_7.gif\" width=20 height=7 ></td>"\
			"<td>"\
					"<img src=\"/images/7605i/7605i_6_7_r.gif\" width=6 height=7 ></td>"\
			"</tr>"\
		"</table>");
				fprintf(cgiOut,"</td>"\
					"<td>"\
						"<img src=\"/images/7605i/7605i_03_06.gif\" width=44 height=25 ></td>"\
				"</tr>"\
				"<tr>"\
					"<td colspan=4>"\
						"<img src=\"/images/7605i/7605i_03_07.gif\" width=355 height=21 ></td>"\
				"</tr>"\
			"</table>");
	
			fprintf(cgiOut,"</td>");
		}
		fprintf(cgiOut,"</tr>"\
			"<tr>");
		memset(path_buf,0,sizeof(path_buf));
		sprintf(path_buf, "/dbm/product/slot/slot%d/board_state", 1);
		ret = get_int_from_file(path_buf);
		if(0 == ret)
		{
			fprintf(cgiOut,"<td>"\
						"<img src=\"/images/7605i/bk7605i_04.gif\" width=\"719\" height=\"63\" alt=\"\"></td>");
		}
		else
		{
			fprintf(cgiOut,"<td>"); 	
			//fprintf(cgiOut,"<img src=\"/images/7605i/7605i_04.gif\" width=\"719\" height=\"63\" alt=\"\">");
			fprintf(cgiOut,"<table id=Table_01 width=719 height=63 border=0 cellpadding=0 cellspacing=0>"\
		"<tr>"\
			"<td rowspan=3>"\
				"<img src=\"/images/7605i/7605i_04_01.gif\" width=364 height=63 ></td>"\
			"<td colspan=4>"\
				"<img src=\"/images/7605i/7605i_04_02.gif\" width=355 height=14 ></td>"\
		"</tr>"\
		"<tr>"\
			"<td>");
		//ports 
		//fprintf(cgiOut,"<img src=\"/images/7605i/7605i_04_03.gif\" width=113 height=26 >");
		fprintf(cgiOut,"<table id=Table_01 width=113 height=26 border=0 cellpadding=0 cellspacing=0>"\
		"<tr>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_6_4.gif\" width=6 height=4 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_20_4.gif\" width=20 height=4 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_4.gif\" width=7 height=4 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_20_4.gif\" width=20 height=4 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_4.gif\" width=7 height=4 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_20_4.gif\" width=20 height=4 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_4.gif\" width=7 height=4 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_20_4.gif\" width=20 height=4 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_6_4_r.gif\" width=6 height=4 ></td>"\
		"</tr>"\
		"<tr>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_6_18.gif\" width=6 height=18 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_lup_black.gif\" width=20 height=18 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_18.gif\" width=7 height=18 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_lup_black.gif\" width=20 height=18 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_18.gif\" width=7 height=18 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_lup_black.gif\" width=20 height=18 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_18.gif\" width=7 height=18 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_lup_black.gif\" width=20 height=18 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_6_18_r.gif\" width=6 height=18 ></td>"\
		"</tr>"\
		"<tr>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_6_4.gif\" width=6 height=4 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_20_4.gif\" width=20 height=4 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_4.gif\" width=7 height=4 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_20_4.gif\" width=20 height=4 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_4.gif\" width=7 height=4 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_20_4.gif\" width=20 height=4 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_4.gif\" width=7 height=4 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_20_4.gif\" width=20 height=4 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_6_4_r.gif\" width=6 height=4 ></td>"\
			"</tr>"\
		"</table>");
		fprintf(cgiOut,"</td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_04_04.gif\" width=85 height=26 ></td>"\
			"<td>");
		//ports
		//fprintf(cgiOut,	"<img src=\"/images/7605i/7605i_04_05.gif\" width=113 height=26 >");
		fprintf(cgiOut,"<table id=Table_01 width=113 height=26 border=0 cellpadding=0 cellspacing=0>"\
		"<tr>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_6_7_t.gif\" width=6 height=7 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_20_7_t.gif\" width=20 height=7 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_7_t.gif\" width=7 height=7 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_20_7_t.gif\" width=20 height=7 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_7_t.gif\" width=7 height=7 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_20_7_t.gif\" width=20 height=7 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_7_t.gif\" width=7 height=7 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_20_7_t.gif\" width=20 height=7 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_6_7_r.gif\" width=6 height=7 ></td>"\
		"</tr>"\
		"<tr>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_6_12.gif\" width=6 height=12 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_e_black.gif\" width=20 height=12 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_12.gif\" width=7 height=12 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_e_black.gif\" width=20 height=12 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_12.gif\" width=7 height=12 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_e_black.gif\" width=20 height=12 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_12.gif\" width=7 height=12 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_e_black.gif\" width=20 height=12 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_6_12_r.gif\" width=6 height=12 ></td>"\
		"</tr>"\
		"<tr>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_6_7.gif\" width=6 height=7 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_20_7.gif\" width=20 height=7 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_7.gif\" width=7 height=7 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_20_7.gif\" width=20 height=7 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_7.gif\" width=7 height=7 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_20_7.gif\" width=20 height=7 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_7_7.gif\" width=7 height=7 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_20_7.gif\" width=20 height=7 ></td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_6_7_r.gif\" width=6 height=7 ></td>"\
			"</tr>"\
		"</table>");
		fprintf(cgiOut,"</td>"\
			"<td>"\
				"<img src=\"/images/7605i/7605i_04_06.gif\" width=44 height=26 ></td>"\
		"</tr>"\
		"<tr>"\
			"<td colspan=4>"\
					"<img src=\"/images/7605i/7605i_04_07.gif\" width=355 height=23 ></td>"\
			"</tr>"\
		"</table>");
			fprintf(cgiOut,"</td>");
		}
		fprintf(cgiOut,"</tr>"\
		"</table>");
	}

/*5000的面板函数，根据产品id来判断*/
int panel_5k(struct sys_ver ptrsysver,unsigned int value,struct eth_port_s pt,int retu,struct list *lcon,char *n,char *m)
{
  char cstring[128];  //字符串
  memset(cstring,0,128);
  int i;
 ////////////////////// panel 5000 //////////////////////////////////////////////////////

     //ccgi_dbus_init();
     //show_sys_ver(&ptrsysver);
	 
fprintf(cgiOut,"<table id=__01 width=730 height=139 border=0 cellpadding=0 cellspacing=0>"\
	"<tr>"\
		"<td rowspan=9>"\
			"<img src=/images/new5000/5624_01.jpg width=48 height=139 ></td>"\
		"<td colspan=56>"\
			"<img src=/images/new5000/5624_02.jpg width=534 height=20 ></td>"\
		"<td colspan=5 rowspan=5>"\
			"<img src=/images/new5000/5624_03.jpg width=117 height=50 ></td>"\
		"<td rowspan=9>"\
			"<img src=/images/new5000/5624_04.jpg width=30 height=139 ></td>"\
		"<td>"\
			"<img src=/images/new5000/sep.gif width=1 height=20 ></td>"\
	"</tr>"\
	"<tr>"\
		"<td>"\
			"<img src=/images/new5000/5624_05.jpg width=9 height=9 ></td>"\
		"<td rowspan=2>"\
			"<img src=/images/new5000/5624_06.jpg width=4 height=17 ></td>"\
		"<td>"\
			"<img src=/images/new5000/5624_05.jpg width=9 height=9 ></td>"\
		"<td colspan=53 rowspan=2>"\
			"<img src=/images/new5000/5624_08.jpg width=512 height=17 ></td>"\
		"<td>"\
			"<img src=/images/new5000/sep.gif width=1 height=9 ></td>"\
	"</tr>"\
	"<tr>"\
		"<td>"\
			"<img src=/images/new5000/5624_09.jpg width=9 height=8 ></td>"\
		"<td>"\
			"<img src=/images/new5000/5624_10.jpg width=9 height=8 ></td>"\
		"<td>"\
			"<img src=/images/new5000/sep.gif width=1 height=8 ></td>"\
	"</tr>"\
	"<tr>");
		


         ///////////////1 3 5 7 9  普通 /////////////////

		 light_flag(1,6,pt,1);
		 memset(cstring,0,128);
		 strcpy(cstring,"<td rowspan=5><img src=/images/new5000/5624_34.jpg width=38 height=65 ></td>");		
		 light_spe(11,pt,cstring);

      ////////////////// 11 口结束  ///////////////
			
		light_flag(7,10,pt,1);
	  	memset(cstring,0,128);
		strcpy(cstring,"<td><img src=/images/new5000/5624_50.jpg width=12 height=9 ></td>");		
        light_spe(19,pt,cstring);
			
	

		////////////  到第二十个端口  光电互斥口////////////////////////////////////		
	  		for(i=1;i<3;i++)
           	{
           		value=1;
           		value=(value << 8) |(2*i-1+20);
           		if(show_eth_port_atrr(value,0,&pt)==CCGI_SUCCESS)
		      	{
     		        if(strcmp(eth_port_type_str[pt.port_type],"ETH_GE_SFP")==0)
         		      fprintf(cgiOut,"<td><img src=/images/new5000/5624_05.jpg width=9 height=9 ></td>"); //黑色
      				else  /*type为电口*/
      				{
                        if(strcmp(onoff_status_str[(pt.attr_bitmap & ETH_ATTR_ADMIN_STATUS) >> ETH_ADMIN_STATUS_BIT],"OFF")==0)
         		          fprintf(cgiOut,"<td><img src=/images/new5000/5624_05.jpg width=9 height=9 ></td>"); //黑色
      			    	else
      			    	{
      		          		if(strcmp(link_status_str[(pt.attr_bitmap & ETH_ATTR_LINK_STATUS) >> ETH_LINK_STATUS_BIT],"UP")==0 && 0==strcmp(eth_speed_str[(pt.attr_bitmap & ETH_ATTR_SPEED_MASK) >> ETH_SPEED_BIT],"1000M"))
	       			            fprintf(cgiOut,"<td><img src=/images/new5000/5624_11.jpg width=9 height=9 ></td>"); //绿色
      				  		else
			                    fprintf(cgiOut,"<td><img src=/images/new5000/5624_13.jpg width=9 height=9 ></td>"); //红色
      			    	}
      				}
		      	}
			  else
         		fprintf(cgiOut,"<td><img src=/images/new5000/5624_05.jpg width=9 height=9 ></td>"); //黑色

				fprintf(cgiOut,"<td><img src=/images/new5000/5624_12.jpg width=4 height=9 ""></td>");

        ///////////////////////////////////////////////////light2///////////////////////////////////////				
				if(show_eth_port_atrr(value,0,&pt)==CCGI_SUCCESS)
     			{
     				if(strcmp(eth_port_type_str[pt.port_type],"ETH_GE_SFP")==0)
         		       fprintf(cgiOut,"<td><img src=/images/new5000/5624_05.jpg width=9 height=9 ></td>"); //黑色
     				else  /*type为电口*/
     				{
     					if(strcmp(onoff_status_str[(pt.attr_bitmap & ETH_ATTR_ADMIN_STATUS) >> ETH_ADMIN_STATUS_BIT],"OFF")==0)
         		            fprintf(cgiOut,"<td><img src=/images/new5000/5624_05.jpg width=9 height=9 ></td>"); //黑色
     					else
     					{
     						if(strcmp(link_status_str[(pt.attr_bitmap & ETH_ATTR_LINK_STATUS) >> ETH_LINK_STATUS_BIT],"UP")==0)
	       			           fprintf(cgiOut,"<td><img src=/images/new5000/5624_11.jpg width=9 height=9 ></td>"); //绿色
     						else
			                  fprintf(cgiOut,"<td><img src=/images/new5000/5624_13.jpg width=9 height=9 ></td>"); //红色
     					}
     				}
     			}
     		  else
         		fprintf(cgiOut,"<td><img src=/images/new5000/5624_05.jpg width=9 height=9 ></td>"); //黑色

     			switch(i)
				{
					case 1: fprintf(cgiOut,"<td><img src=/images/new5000/5624_14.jpg width=11 height=9 ></td>");break;
					case 2: fprintf(cgiOut,"<td rowspan=5><img src=/images/new5000/5624_58.jpg width=40 height=65 ></td>");break;
				}
     
			    
           	}


      ////////////////////////////////////////////////////////光口 指示灯 21 /////////////////////////////////////////////  
            for(i=1;i<3;i++)
			{
				value=1;
           		value=(value << 8) |(2*i+20-1);
           		if(show_eth_port_atrr(value,0,&pt)==CCGI_SUCCESS)
		      	{
		       	 	if(strcmp(eth_port_type_str[pt.port_type],"ETH_GE_SFP")!=0)
		       	 		{
		       	//两个黑灯 		
		       	if(i==1){
         		fprintf(cgiOut,"<td><img src=/images/new5000/5624_05.jpg width=9 height=9 ></td>"); //黑色
		        fprintf(cgiOut,"<td><img src=/images/new5000/5624_12.jpg width=4 height=9 ></td>");
         		fprintf(cgiOut,"<td><img src=/images/new5000/5624_05.jpg width=9 height=9 ></td>"); //黑色
		        fprintf(cgiOut,"<td rowspan=5><img src=/images/new5000/5624_62.jpg width=16 height=65 ></td>");
		       		}
				if(i==2){
         		fprintf(cgiOut,"<td><img src=/images/new5000/5624_05.jpg width=9 height=9 ></td>"); //黑色
		        fprintf(cgiOut,"<td><img src=/images/new5000/5624_64.jpg width=5 height=9 ></td>");
         		fprintf(cgiOut,"<td><img src=/images/new5000/5624_05.jpg width=9 height=9 ></td>"); //黑色
		        fprintf(cgiOut,"<td rowspan=6><img src=/images/new5000/5624_66.jpg width=20 height=102 ></td>");
		        fprintf(cgiOut,"<td><img src=/images/new5000/sep.gif width=1 height=9 ></td>");
					}
		       	 		}

			/*--------------------------  ----------------------------------------*/
		       	 	else
		       	 		{
		       	 			if(strcmp(onoff_status_str[(pt.attr_bitmap & ETH_ATTR_ADMIN_STATUS) >> ETH_ADMIN_STATUS_BIT],"OFF")==0)
		       	 				{
		       	 				        if(i==1){
        		       	 				fprintf(cgiOut,"<td><img src=/images/new5000/5624_05.jpg width=9 height=9 ></td>"); //黑色
                        		        fprintf(cgiOut,"<td><img src=/images/new5000/5624_12.jpg width=4 height=9 ></td>");
                                 		fprintf(cgiOut,"<td><img src=/images/new5000/5624_05.jpg width=9 height=9 ></td>"); //黑色
                        		        fprintf(cgiOut,"<td rowspan=5><img src=/images/new5000/5624_62.jpg width=16 height=65 ></td>");
		       	 				        	}
										if(i==2){
                                 		fprintf(cgiOut,"<td><img src=/images/new5000/5624_05.jpg width=9 height=9 ></td>"); //黑色
                        		        fprintf(cgiOut,"<td><img src=/images/new5000/5624_64.jpg width=5 height=9 ></td>");
                                 		fprintf(cgiOut,"<td><img src=/images/new5000/5624_05.jpg width=9 height=9 ></td>"); //黑色
                        		        fprintf(cgiOut,"<td rowspan=6><img src=/images/new5000/5624_66.jpg width=20 height=102 ></td>");
                        		        fprintf(cgiOut,"<td><img src=/images/new5000/sep.gif width=1 height=9 ></td>");
                        		       	 }
										}
		       	 			  else
		       	 				{
								if(strcmp(link_status_str[(pt.attr_bitmap & ETH_ATTR_LINK_STATUS) >> ETH_LINK_STATUS_BIT],"UP")==0)
		       	 				{
		       	 				//全绿色
		       	 				 if(i==1){
										fprintf(cgiOut,"<td><img src=/images/new5000/5624_11.jpg width=9 height=9 ></td>"); //绿色
                        		        fprintf(cgiOut,"<td><img src=/images/new5000/5624_12.jpg width=4 height=9 ></td>");
                                 		fprintf(cgiOut,"<td><img src=/images/new5000/5624_11.jpg width=9 height=9 ></td>"); //绿色
                        		        fprintf(cgiOut,"<td rowspan=5><img src=/images/new5000/5624_62.jpg width=16 height=65 ></td>");
		       	 					 }
								 if(i==2){
                                 		fprintf(cgiOut,"<td><img src=/images/new5000/5624_11.jpg width=9 height=9 ></td>"); //绿色
                        		        fprintf(cgiOut,"<td><img src=/images/new5000/5624_64.jpg width=5 height=9 ></td>");
                                 		fprintf(cgiOut,"<td><img src=/images/new5000/5624_11.jpg width=9 height=9 ></td>"); //绿色
                        		        fprintf(cgiOut,"<td rowspan=6><img src=/images/new5000/5624_66.jpg width=20 height=102 ></td>");
                        		        fprintf(cgiOut,"<td><img src=/images/new5000/sep.gif width=1 height=9 ></td>");
								 	}
								 }
		       	 			else
		       	 				{
		       	 				//全红色
		       	 				         if(i==1){
		       	 						fprintf(cgiOut,"<td><img src=/images/new5000/5624_13.jpg width=9 height=9 ></td>"); //红色
                        		        fprintf(cgiOut,"<td><img src=/images/new5000/5624_12.jpg width=4 height=9 ></td>");
                                 		fprintf(cgiOut,"<td><img src=/images/new5000/5624_13.jpg width=9 height=9 ></td>"); //红色
                        		        fprintf(cgiOut,"<td rowspan=5><img src=/images/new5000/5624_62.jpg width=16 height=65 ></td>");
		       	 				         	}
										 if(i==2)
										 	{
                                 		fprintf(cgiOut,"<td><img src=/images/new5000/5624_13.jpg width=9 height=9 ></td>"); //红色
                        		        fprintf(cgiOut,"<td><img src=/images/new5000/5624_64.jpg width=5 height=9 ></td>");
                                 		fprintf(cgiOut,"<td><img src=/images/new5000/5624_13.jpg width=9 height=9 ></td>"); //红色
                        		        fprintf(cgiOut,"<td rowspan=6><img src=/images/new5000/5624_66.jpg width=20 height=102 ></td>");
                        		        fprintf(cgiOut,"<td><img src=/images/new5000/sep.gif width=1 height=9 ></td>");
										 	}
		       	 				}
		       	 		    }
		       	 		}

					/*----------------------------  end  ----------------------------------------*/

		       	}
			}
	  //////////////////////////// 21 等结束 /////////////////////////////////////////////
	fprintf(cgiOut,"</tr>");
 ///////////////////////分开截图 端口//////////////////////////

         fprintf(cgiOut,"<tr>");

 retu=checkuser_group(m);
 if(retu==0)  /*管理员*/
 	{
		fprintf(cgiOut,"<td colspan=23 rowspan=3>");
		//fprintf(cgiOut,"<img src=/images/new5000/5624_67.jpg width=187 height=47 >");
		fprintf(cgiOut,"<table id=__01 width=187 height=47 border=0 cellpadding=0 cellspacing=0>"\
	"<tr>"\
		"<td colspan=11>"\
			"<img src=/images/new5000/5624_67_01.jpg width=187 height=4 ></td>"\
	"</tr>"\
	"<tr>");
		fprintf(cgiOut,"<td><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame>"\
			"<img src=/images/new5000/5624_68_03.jpg width=23 height=19 border=0 alt=\"%s\"></td>",n,"1-1","1");
		
		fprintf(cgiOut,"<td rowspan=4>"\
			"<img src=/images/new5000/5624_67_03.jpg width=10 height=43 ></td>");
		
		fprintf(cgiOut,"<td><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame>"\
			"<img src=/images/new5000/5624_68_03.jpg width=23 height=19 border=0 alt=\"%s\"></td>",n,"1-3","3");
		
		
		fprintf(cgiOut,"<td rowspan=4>"\
			"<img src=/images/new5000/5624_67_03.jpg width=10 height=43 ></td>");
		fprintf(cgiOut,"<td><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame>"\
			"<img src=/images/new5000/5624_68_03.jpg width=23 height=19 border=0 alt=\"%s\"></td>",n,"1-5","5");
		
		
		fprintf(cgiOut,"<td rowspan=4>"\
			"<img src=/images/new5000/5624_67_03.jpg width=10 height=43 ></td>");
		fprintf(cgiOut,"<td><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame>"\
			"<img src=/images/new5000/5624_68_03.jpg width=23 height=19 border=0 alt=\"%s\"></td>",n,"1-7","7");
		
		fprintf(cgiOut,"<td rowspan=4>"\
			"<img src=/images/new5000/5624_67_03.jpg width=10 height=43 ></td>");
		fprintf(cgiOut,"<td><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame>"\
			"<img src=/images/new5000/5624_68_03.jpg width=23 height=19 border=0 alt=\"%s\"></td>",n,"1-9","9");
		
		fprintf(cgiOut,"<td rowspan=4>"\
			"<img src=/images/new5000/5624_67_11.jpg width=9 height=43 ></td>");
		fprintf(cgiOut,"<td><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame>"\
			"<img src=/images/new5000/5624_68_03.jpg width=23 height=19 border=0 alt=\"%s\"></td>",n,"1-11","11");
		
	fprintf(cgiOut,"</tr>"\
	"<tr>"\
		"<td>"\
			"<img src=/images/new5000/5624_67_13.jpg width=23 height=3 ></td>"\
		"<td>"\
			"<img src=/images/new5000/5624_67_14.jpg width=23 height=3 ></td>"\
		"<td>"\
			"<img src=/images/new5000/5624_67_14.jpg width=23 height=3 ></td>"\
		"<td>"\
			"<img src=/images/new5000/5624_67_14.jpg width=23 height=3 ></td>"\
		"<td>"\
			"<img src=/images/new5000/5624_67_14.jpg width=23 height=3 ></td>"\
		"<td>"\
			"<img src=/images/new5000/5624_67_14.jpg width=23 height=3 ></td>"\
	"</tr>"\
	"<tr>");
		fprintf(cgiOut,"<td><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame>"\
			"<img src=/images/new5000/5624_67_19.jpg width=23 height=19 border=0 alt=\"%s\"></td>",n,"1-2","2");
		
		fprintf(cgiOut,"<td><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame>"\
			"<img src=/images/new5000/5624_67_19.jpg width=23 height=19 border=0 alt=\"%s\"></td>",n,"1-4","4");
		
		fprintf(cgiOut,"<td><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame>"\
			"<img src=/images/new5000/5624_67_19.jpg width=23 height=19 border=0 alt=\"%s\"></td>",n,"1-6","6");
		
		fprintf(cgiOut,"<td><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame>"\
			"<img src=/images/new5000/5624_67_19.jpg width=23 height=19 border=0 alt=\"%s\"></td>",n,"1-8","8");
		
		fprintf(cgiOut,"<td><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame>"\
			"<img src=/images/new5000/5624_67_19.jpg width=23 height=19 border=0 alt=\"%s\"></td>",n,"1-10","10");
		
		fprintf(cgiOut,"<td><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame>"\
			"<img src=/images/new5000/5624_67_19.jpg width=23 height=19 border=0 alt=\"%s\"></td>",n,"1-12","12");
		
	fprintf(cgiOut,"</tr>"\
	"<tr>"\
		"<td bgcolor=#000000>"\
			"<img src=/images/new5000/5624_67_25.jpg width=23 height=2 ></td>"\
		"<td>"\
			"<img src=/images/new5000/5624_67_26.jpg width=23 height=2 ></td>"\
		"<td>"\
			"<img src=/images/new5000/5624_67_26.jpg width=23 height=2 ></td>"\
		"<td>"\
			"<img src=/images/new5000/5624_67_26.jpg width=23 height=2 ></td>"\
		"<td>"\
			"<img src=/images/new5000/5624_67_26.jpg width=23 height=2 ></td>"\
		"<td>"\
			"<img src=/images/new5000/5624_67_26.jpg width=23 height=2 ></td>"\
	"</tr>"\
"</table>");
		fprintf(cgiOut,"</td>");
		

		fprintf(cgiOut,"<td colspan=23 rowspan=3>");
		//fprintf(cgiOut,"<img src=/images/new5000/5624_68.jpg width=188 height=47 >");

		
fprintf(cgiOut,"<table id=__01 width=188 height=47 border=0 cellpadding=0 cellspacing=0>"\
	"<tr>"\
		"<td colspan=13>"\
			"<img src=/images/new5000/5624_68_01.jpg width=188 height=3 ></td>"\
	"</tr>"\
	"<tr>");
		fprintf(cgiOut,"<td colspan=3 rowspan=2 bgcolor=#F4F4F4><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame>"\
			"<img src=/images/new5000/5624_68_02.jpg width=34 height=22 border=0 alt=\"%s\">",n,"1-13","13");
			fprintf(cgiOut,"</td>");
			fprintf(cgiOut,"<td><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame>"\
			"<img src=/images/new5000/5624_68_03.jpg width=23 height=19 border=0 alt=\"%s\"></td>",n,"1-15","15");
		
		fprintf(cgiOut,"<td rowspan=4>"\
			"<img src=/images/new5000/5624_68_04.jpg width=10 height=44 ></td>");
		fprintf(cgiOut,"<td><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame>"\
			"<img src=/images/new5000/5624_68_03.jpg width=23 height=19 border=0 alt=\"%s\"></td>",n,"1-17","17");
		
		fprintf(cgiOut,"<td rowspan=4>"\
			"<img src=/images/new5000/5624_68_06.jpg width=10 height=44 ></td>");
		fprintf(cgiOut,"<td><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame>"\
			"<img src=/images/new5000/5624_68_03.jpg width=23 height=19 border=0 alt=\"%s\"></td>",n,"1-19","19");
		
		fprintf(cgiOut,"<td rowspan=4>"\
			"<img src=/images/new5000/5624_68_08.jpg width=10 height=44 ></td>");
		fprintf(cgiOut,"<td><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame>"\
			"<img src=/images/new5000/5624_68_03.jpg width=23 height=19 border=0 alt=\"%s\"></td>",n,"1-21","21");
		
		fprintf(cgiOut,"<td rowspan=4>"\
			"<img src=/images/new5000/5624_68_10.jpg width=8 height=44 ></td>");
		fprintf(cgiOut,"<td><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame>"\
			"<img src=/images/new5000/5624_68_03.jpg width=23 height=19 border=0 alt=\"%s\"></td>",n,"1-23","23");
		
		fprintf(cgiOut,"<td rowspan=4>"\
			"<img src=/images/new5000/5624_68_12.jpg width=1 height=44 ></td>"\
	"</tr>"\
	"<tr>"\
		"<td>"\
			"<img src=/images/new5000/5624_67_14.jpg width=23 height=3 ></td>"\
		"<td>"\
			"<img src=/images/new5000/5624_67_14.jpg width=23 height=3 ></td>"\
		"<td>"\
			"<img src=/images/new5000/5624_67_14.jpg width=23 height=3 ></td>"\
		"<td>"\
			"<img src=/images/new5000/5624_67_14.jpg width=23 height=3 ></td>"\
		"<td>"\
			"<img src=/images/new5000/5624_67_14.jpg width=23 height=3 ></td>"\
	"</tr>"\
	"<tr>"\
		"<td rowspan=2>"\
			"<img src=/images/new5000/5624_68_18.jpg width=1 height=22 ></td>");
		fprintf(cgiOut,"<td><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame>"\
			"<img src=/images/new5000/5624_67_19.jpg width=23 height=19 border=0 alt=\"%s\"></td>",n,"1-14","14");
		
		fprintf(cgiOut,"<td rowspan=2>"\
			"<img src=/images/new5000/5624_68_20.jpg width=10 height=22 ></td>");
		fprintf(cgiOut,"<td><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame>"\
			"<img src=/images/new5000/5624_67_19.jpg width=23 height=19 border=0 alt=\"%s\"></td>",n,"1-16","16");
		
		fprintf(cgiOut,"<td><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame>"\
			"<img src=/images/new5000/5624_67_19.jpg width=23 height=19 border=0 alt=\"%s\"></td>",n,"1-18","18");
		
		fprintf(cgiOut,"<td><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame>"\
			"<img src=/images/new5000/5624_67_19.jpg width=23 height=19 border=0 alt=\"%s\"></td>",n,"1-20","20");
		
		fprintf(cgiOut,"<td><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame>"\
			"<img src=/images/new5000/5624_67_19.jpg width=23 height=19 border=0 alt=\"%s\"></td>",n,"1-22","22");
		
		fprintf(cgiOut,"<td><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame>"\
			"<img src=/images/new5000/5624_67_19.jpg width=23 height=19 border=0 alt=\"%s\"></td>",n,"1-24","24");
		
	fprintf(cgiOut,"</tr>"\
	"<tr>"\
		"<td>"\
			"<img src=/images/new5000/5624_68_26.jpg width=23 height=3 ></td>"\
		"<td>"\
			"<img src=/images/new5000/5624_67_14.jpg width=23 height=3 ></td>"\
		"<td>"\
			"<img src=/images/new5000/5624_67_14.jpg width=23 height=3 ></td>"\
		"<td>"\
			"<img src=/images/new5000/5624_67_14.jpg width=23 height=3 ></td>"\
		"<td>"\
			"<img src=/images/new5000/5624_67_14.jpg width=23 height=3 ></td>"\
		"<td>"\
			"<img src=/images/new5000/5624_67_14.jpg width=23 height=3 ></td>"\
	"</tr>"\
"</table>");

		
		fprintf(cgiOut,"</td>");

 	}
 else
  {
		fprintf(cgiOut,"<td colspan=23 rowspan=3>");
		//fprintf(cgiOut,"<img src=/images/new5000/5624_67.jpg width=187 height=47 >");
		fprintf(cgiOut,"<table id=__01 width=187 height=47 border=0 cellpadding=0 cellspacing=0>"\
	"<tr>"\
		"<td colspan=11>"\
			"<img src=/images/new5000/5624_67_01.jpg width=187 height=4 ></td>"\
	"</tr>"\
	"<tr>");
		fprintf(cgiOut,"<td>"\
			"<img src=/images/new5000/5624_68_03.jpg width=23 height=19 border=0 alt=\"%s\"></td>","1");
		
		fprintf(cgiOut,"<td rowspan=4>"\
			"<img src=/images/new5000/5624_67_03.jpg width=10 height=43 ></td>");
		
		fprintf(cgiOut,"<td>"\
			"<img src=/images/new5000/5624_68_03.jpg width=23 height=19 border=0 alt=\"%s\"></td>","3");
		
		
		fprintf(cgiOut,"<td rowspan=4>"\
			"<img src=/images/new5000/5624_67_03.jpg width=10 height=43 ></td>");
		fprintf(cgiOut,"<td>"\
			"<img src=/images/new5000/5624_68_03.jpg width=23 height=19 border=0 alt=\"%s\"></td>","5");
		
		
		fprintf(cgiOut,"<td rowspan=4>"\
			"<img src=/images/new5000/5624_67_03.jpg width=10 height=43 ></td>");
		fprintf(cgiOut,"<td>"\
			"<img src=/images/new5000/5624_68_03.jpg width=23 height=19 border=0 alt=\"%s\"></td>","7");
		
		fprintf(cgiOut,"<td rowspan=4>"\
			"<img src=/images/new5000/5624_67_03.jpg width=10 height=43 ></td>");
		fprintf(cgiOut,"<td>"\
			"<img src=/images/new5000/5624_68_03.jpg width=23 height=19 border=0 alt=\"%s\"></td>","9");
		
		fprintf(cgiOut,"<td rowspan=4>"\
			"<img src=/images/new5000/5624_67_11.jpg width=9 height=43 ></td>");
		fprintf(cgiOut,"<td>"\
			"<img src=/images/new5000/5624_68_03.jpg width=23 height=19 border=0 alt=\"%s\"></td>","11");
		
	fprintf(cgiOut,"</tr>"\
	"<tr>"\
		"<td>"\
			"<img src=/images/new5000/5624_67_13.jpg width=23 height=3 ></td>"\
		"<td>"\
			"<img src=/images/new5000/5624_67_14.jpg width=23 height=3 ></td>"\
		"<td>"\
			"<img src=/images/new5000/5624_67_14.jpg width=23 height=3 ></td>"\
		"<td>"\
			"<img src=/images/new5000/5624_67_14.jpg width=23 height=3 ></td>"\
		"<td>"\
			"<img src=/images/new5000/5624_67_14.jpg width=23 height=3 ></td>"\
		"<td>"\
			"<img src=/images/new5000/5624_67_14.jpg width=23 height=3 ></td>"\
	"</tr>"\
	"<tr>");
		fprintf(cgiOut,"<td>"\
			"<img src=/images/new5000/5624_67_19.jpg width=23 height=19 border=0 alt=\"%s\"></td>","2");
		
		fprintf(cgiOut,"<td>"\
			"<img src=/images/new5000/5624_67_19.jpg width=23 height=19 border=0 alt=\"%s\"></td>","4");
		
		fprintf(cgiOut,"<td>"\
			"<img src=/images/new5000/5624_67_19.jpg width=23 height=19 border=0 alt=\"%s\"></td>","6");
		
		fprintf(cgiOut,"<td>"\
			"<img src=/images/new5000/5624_67_19.jpg width=23 height=19 border=0 alt=\"%s\"></td>","8");
		
		fprintf(cgiOut,"<td>"\
			"<img src=/images/new5000/5624_67_19.jpg width=23 height=19 border=0 alt=\"%s\"></td>","10");
		
		fprintf(cgiOut,"<td>"\
			"<img src=/images/new5000/5624_67_19.jpg width=23 height=19 border=0 alt=\"%s\"></td>","12");
		
	fprintf(cgiOut,"</tr>"\
	"<tr>"\
		"<td bgcolor=#000000>"\
			"<img src=/images/new5000/5624_67_25.jpg width=23 height=2 ></td>"\
		"<td>"\
			"<img src=/images/new5000/5624_67_26.jpg width=23 height=2 ></td>"\
		"<td>"\
			"<img src=/images/new5000/5624_67_26.jpg width=23 height=2 ></td>"\
		"<td>"\
			"<img src=/images/new5000/5624_67_26.jpg width=23 height=2 ></td>"\
		"<td>"\
			"<img src=/images/new5000/5624_67_26.jpg width=23 height=2 ></td>"\
		"<td>"\
			"<img src=/images/new5000/5624_67_26.jpg width=23 height=2 ></td>"\
	"</tr>"\
"</table>");
		fprintf(cgiOut,"</td>");
		

		fprintf(cgiOut,"<td colspan=23 rowspan=3>");
		//fprintf(cgiOut,"<img src=/images/new5000/5624_68.jpg width=188 height=47 >");

		
fprintf(cgiOut,"<table id=__01 width=188 height=47 border=0 cellpadding=0 cellspacing=0>"\
	"<tr>"\
		"<td colspan=13>"\
			"<img src=/images/new5000/5624_68_01.jpg width=188 height=3 ></td>"\
	"</tr>"\
	"<tr>");
		fprintf(cgiOut,"<td colspan=3 rowspan=2 bgcolor=#F4F4F4>"\
			"<img src=/images/new5000/5624_68_02.jpg width=34 height=22 border=0 alt=\"%s\">","13");
			fprintf(cgiOut,"</td>");
			fprintf(cgiOut,"<td>"\
			"<img src=/images/new5000/5624_68_03.jpg width=23 height=19 border=0 alt=\"%s\"></td>","15");
		
		fprintf(cgiOut,"<td rowspan=4>"\
			"<img src=/images/new5000/5624_68_04.jpg width=10 height=44 ></td>");
		fprintf(cgiOut,"<td>"\
			"<img src=/images/new5000/5624_68_03.jpg width=23 height=19 border=0 alt=\"%s\"></td>","17");
		
		fprintf(cgiOut,"<td rowspan=4>"\
			"<img src=/images/new5000/5624_68_06.jpg width=10 height=44 ></td>");
		fprintf(cgiOut,"<td>"\
			"<img src=/images/new5000/5624_68_03.jpg width=23 height=19 border=0 alt=\"%s\"></td>","19");
		
		fprintf(cgiOut,"<td rowspan=4>"\
			"<img src=/images/new5000/5624_68_08.jpg width=10 height=44 ></td>");
		fprintf(cgiOut,"<td>"\
			"<img src=/images/new5000/5624_68_03.jpg width=23 height=19 border=0 alt=\"%s\"></td>","21");
		
		fprintf(cgiOut,"<td rowspan=4>"\
			"<img src=/images/new5000/5624_68_10.jpg width=8 height=44 ></td>");
		fprintf(cgiOut,"<td>"\
			"<img src=/images/new5000/5624_68_03.jpg width=23 height=19 border=0 alt=\"%s\"></td>","23");
		
		fprintf(cgiOut,"<td rowspan=4>"\
			"<img src=/images/new5000/5624_68_12.jpg width=1 height=44 ></td>"\
	"</tr>"\
	"<tr>"\
		"<td>"\
			"<img src=/images/new5000/5624_67_14.jpg width=23 height=3 ></td>"\
		"<td>"\
			"<img src=/images/new5000/5624_67_14.jpg width=23 height=3 ></td>"\
		"<td>"\
			"<img src=/images/new5000/5624_67_14.jpg width=23 height=3 ></td>"\
		"<td>"\
			"<img src=/images/new5000/5624_67_14.jpg width=23 height=3 ></td>"\
		"<td>"\
			"<img src=/images/new5000/5624_67_14.jpg width=23 height=3 ></td>"\
	"</tr>"\
	"<tr>"\
		"<td rowspan=2>"\
			"<img src=/images/new5000/5624_68_18.jpg width=1 height=22 ></td>");
		fprintf(cgiOut,"<td>"\
			"<img src=/images/new5000/5624_67_19.jpg width=23 height=19 border=0 alt=\"%s\"></td>","14");
		
		fprintf(cgiOut,"<td rowspan=2>"\
			"<img src=/images/new5000/5624_68_20.jpg width=10 height=22 ></td>");
		fprintf(cgiOut,"<td>"\
			"<img src=/images/new5000/5624_67_19.jpg width=23 height=19 border=0 alt=\"%s\"></td>","16");
		
		fprintf(cgiOut,"<td>"\
			"<img src=/images/new5000/5624_67_19.jpg width=23 height=19 border=0 alt=\"%s\"></td>","18");
		
		fprintf(cgiOut,"<td>"\
			"<img src=/images/new5000/5624_67_19.jpg width=23 height=19 border=0 alt=\"%s\"></td>","20");
		
		fprintf(cgiOut,"<td>"\
			"<img src=/images/new5000/5624_67_19.jpg width=23 height=19 border=0 alt=\"%s\"></td>","22");
		
		fprintf(cgiOut,"<td>"\
			"<img src=/images/new5000/5624_67_19.jpg width=23 height=19 border=0 alt=\"%s\"></td>","24");
		
	fprintf(cgiOut,"</tr>"\
	"<tr>"\
		"<td>"\
			"<img src=/images/new5000/5624_68_26.jpg width=23 height=3 ></td>"\
		"<td>"\
			"<img src=/images/new5000/5624_67_14.jpg width=23 height=3 ></td>"\
		"<td>"\
			"<img src=/images/new5000/5624_67_14.jpg width=23 height=3 ></td>"\
		"<td>"\
			"<img src=/images/new5000/5624_67_14.jpg width=23 height=3 ></td>"\
		"<td>"\
			"<img src=/images/new5000/5624_67_14.jpg width=23 height=3 ></td>"\
		"<td>"\
			"<img src=/images/new5000/5624_67_14.jpg width=23 height=3 ></td>"\
	"</tr>"\
"</table>");

		
		fprintf(cgiOut,"</td>");

 	}

			
////////////////// end 端口////////////////////////////////////
			
		fprintf(cgiOut,"<td colspan=3 rowspan=3>");
			fprintf(cgiOut,"<img src=/images/new5000/5624_69.jpg width=22 height=47 >");
			fprintf(cgiOut,"</td>"\
		"<td colspan=3 rowspan=3>"\
			"<img src=/images/new5000/5624_70.jpg width=23 height=47 ></td>"\
		"<td>"\
			"<img src=/images/new5000/sep.gif width=1 height=4 ></td>"\
	"</tr>"\
	"<tr>");
		
		fprintf(cgiOut,"<td align=center colspan=5 bgcolor=#C7C7C7>");
			//fprintf(cgiOut,"<img src=/images/new5000/5624_71.jpg width=117 height=19 >");
			fprintf(cgiOut,"</td>"\
		"<td>"\
			"<img src=/images/new5000/sep.gif width=1 height=19 ></td>"\
	"</tr>"\
	"<tr>"\
		"<td colspan=5>"\
			"<img src=/images/new5000/5624_72.jpg width=117 height=24 ></td>"\
		"<td>"\
			"<img src=/images/new5000/sep.gif width=1 height=24 ></td>"\
	"</tr>"\
	"<tr>");

     //////  2 4 6 8 10 指示灯 ////////////////
     light_flag(1,6,pt,2);
     memset(cstring,0,128);
	 strcpy(cstring,"");		
     light_spe(12,pt,cstring);	 

     light_flag(7,10,pt,2);
	 memset(cstring,0,128);
	 strcpy(cstring,"<td><img src=/images/new5000/5624_111.jpg width=12 height=9 ></td>");		
     light_spe(20,pt,cstring);	 

    ///////// 20   光电互斥口//////////////////////
  
       for(i=1;i<3;i++)
           	{
           		value=1;
           		value=(value << 8) |(2*i+20);
           		if(show_eth_port_atrr(value,0,&pt)==CCGI_SUCCESS)
		      	{
     		        if(strcmp(eth_port_type_str[pt.port_type],"ETH_GE_SFP")==0)
         		      fprintf(cgiOut,"<td><img src=/images/new5000/5624_05.jpg width=9 height=9 ></td>"); //黑色
      				else  /*type为电口*/
      				{
                        if(strcmp(onoff_status_str[(pt.attr_bitmap & ETH_ATTR_ADMIN_STATUS) >> ETH_ADMIN_STATUS_BIT],"OFF")==0)
         		          fprintf(cgiOut,"<td><img src=/images/new5000/5624_05.jpg width=9 height=9 ></td>"); //黑色
      			    	else
      			    	{
      		          		if(strcmp(link_status_str[(pt.attr_bitmap & ETH_ATTR_LINK_STATUS) >> ETH_LINK_STATUS_BIT],"UP")==0 && 0==strcmp(eth_speed_str[(pt.attr_bitmap & ETH_ATTR_SPEED_MASK) >> ETH_SPEED_BIT],"1000M"))
	       			            fprintf(cgiOut,"<td><img src=/images/new5000/5624_11.jpg width=9 height=9 ></td>"); //绿色
      				  		else
			                    fprintf(cgiOut,"<td><img src=/images/new5000/5624_13.jpg width=9 height=9 ></td>"); //红色
      			    	}
      				}
		      	}
			  else
         		fprintf(cgiOut,"<td><img src=/images/new5000/5624_05.jpg width=9 height=9 ></td>"); //黑色

				fprintf(cgiOut,"<td><img src=/images/new5000/5624_12.jpg width=4 height=9 ""></td>");

        ///////////////////////////////////////////////////light2///////////////////////////////////////				
				if(show_eth_port_atrr(value,0,&pt)==CCGI_SUCCESS)
     			{
     				if(strcmp(eth_port_type_str[pt.port_type],"ETH_GE_SFP")==0)
         		       fprintf(cgiOut,"<td><img src=/images/new5000/5624_05.jpg width=9 height=9 ></td>"); //黑色
     				else  /*type为电口*/
     				{
     					if(strcmp(onoff_status_str[(pt.attr_bitmap & ETH_ATTR_ADMIN_STATUS) >> ETH_ADMIN_STATUS_BIT],"OFF")==0)
         		            fprintf(cgiOut,"<td><img src=/images/new5000/5624_05.jpg width=9 height=9 ></td>"); //黑色
     					else
     					{
     						if(strcmp(link_status_str[(pt.attr_bitmap & ETH_ATTR_LINK_STATUS) >> ETH_LINK_STATUS_BIT],"UP")==0)
	       			           fprintf(cgiOut,"<td><img src=/images/new5000/5624_11.jpg width=9 height=9 ></td>"); //绿色
     						else
			                  fprintf(cgiOut,"<td><img src=/images/new5000/5624_13.jpg width=9 height=9 ></td>"); //红色
     					}
     				}
     			}
     		  else
         		fprintf(cgiOut,"<td><img src=/images/new5000/5624_05.jpg width=9 height=9 ></td>"); //黑色

     			switch(i)
				{
					case 1: fprintf(cgiOut,"<td><img src=/images/new5000/5624_14.jpg width=11 height=9 ></td>");break;
                    case 2: break;
				}
     
			    
           	}

	///////////////////////  光口指示灯   //////////////////////////////

	  for(i=1;i<3;i++)
			{
				value=1;
           		value=(value << 8) |(2*i+20);
           		if(show_eth_port_atrr(value,0,&pt)==CCGI_SUCCESS)
		      	{
		       	 	if(strcmp(eth_port_type_str[pt.port_type],"ETH_GE_SFP")!=0)
		       	 		{
		       	//两个黑灯 		
		       	if(i==1){
         		fprintf(cgiOut,"<td><img src=/images/new5000/5624_05.jpg width=9 height=9 ></td>"); //黑色
		        fprintf(cgiOut,"<td><img src=/images/new5000/5624_12.jpg width=4 height=9 ></td>");
         		fprintf(cgiOut,"<td><img src=/images/new5000/5624_05.jpg width=9 height=9 ></td>"); //黑色
 		       		}
				if(i==2){
         		fprintf(cgiOut,"<td><img src=/images/new5000/5624_05.jpg width=9 height=9 ></td>"); //黑色
		        fprintf(cgiOut,"<td><img src=/images/new5000/5624_64.jpg width=5 height=9 ></td>");
         		fprintf(cgiOut,"<td><img src=/images/new5000/5624_05.jpg width=9 height=9 ></td>"); //黑色
		        fprintf(cgiOut,"<td><img src=/images/new5000/5624_125.jpg width=68 height=9 ></td>");

					}
		       	 		}

			/*--------------------------  ----------------------------------------*/
		       	 	else
		       	 		{
		       	 			if(strcmp(onoff_status_str[(pt.attr_bitmap & ETH_ATTR_ADMIN_STATUS) >> ETH_ADMIN_STATUS_BIT],"OFF")==0)
		       	 				{
		       	 				        if(i==1){
        		       	 				fprintf(cgiOut,"<td><img src=/images/new5000/5624_05.jpg width=9 height=9 ></td>"); //黑色
                        		        fprintf(cgiOut,"<td><img src=/images/new5000/5624_12.jpg width=4 height=9 ></td>");
                                 		fprintf(cgiOut,"<td><img src=/images/new5000/5624_05.jpg width=9 height=9 ></td>"); //黑色
		       	 				        	}
										if(i==2){
                                 		fprintf(cgiOut,"<td><img src=/images/new5000/5624_05.jpg width=9 height=9 ></td>"); //黑色
                        		        fprintf(cgiOut,"<td><img src=/images/new5000/5624_64.jpg width=5 height=9 ></td>");
                                 		fprintf(cgiOut,"<td><img src=/images/new5000/5624_05.jpg width=9 height=9 ></td>"); //黑色
                        		        fprintf(cgiOut,"<td><img src=/images/new5000/5624_125.jpg width=68 height=9 ></td>");

                        		       	 }
										}
		       	 			  else
		       	 				{
								if(strcmp(link_status_str[(pt.attr_bitmap & ETH_ATTR_LINK_STATUS) >> ETH_LINK_STATUS_BIT],"UP")==0)
		       	 				{
		       	 				//全绿色
		       	 				 if(i==1){
										fprintf(cgiOut,"<td><img src=/images/new5000/5624_11.jpg width=9 height=9 ></td>"); //绿色
                        		        fprintf(cgiOut,"<td><img src=/images/new5000/5624_12.jpg width=4 height=9 ></td>");
                                 		fprintf(cgiOut,"<td><img src=/images/new5000/5624_11.jpg width=9 height=9 ></td>"); //绿色
 		       	 					 }
								 if(i==2){
                                 		fprintf(cgiOut,"<td><img src=/images/new5000/5624_11.jpg width=9 height=9 ></td>"); //绿色
                        		        fprintf(cgiOut,"<td><img src=/images/new5000/5624_64.jpg width=5 height=9 ></td>");
                                 		fprintf(cgiOut,"<td><img src=/images/new5000/5624_11.jpg width=9 height=9 ></td>"); //绿色
                        		      	fprintf(cgiOut,"<td><img src=/images/new5000/5624_125.jpg width=68 height=9 ></td>");

								 	}
								 }
		       	 			else
		       	 				{
		       	 				//全红色
		       	 				         if(i==1){
		       	 						fprintf(cgiOut,"<td><img src=/images/new5000/5624_13.jpg width=9 height=9 ></td>"); //红色
                        		        fprintf(cgiOut,"<td><img src=/images/new5000/5624_12.jpg width=4 height=9 ></td>");
                                 		fprintf(cgiOut,"<td><img src=/images/new5000/5624_13.jpg width=9 height=9 ></td>"); //红色
 		       	 				         	}
										 if(i==2)
										 	{
                                 		fprintf(cgiOut,"<td><img src=/images/new5000/5624_13.jpg width=9 height=9 ></td>"); //红色
                        		        fprintf(cgiOut,"<td><img src=/images/new5000/5624_64.jpg width=5 height=9 ></td>");
                                 		fprintf(cgiOut,"<td><img src=/images/new5000/5624_13.jpg width=9 height=9 ></td>"); //红色
                        		       	fprintf(cgiOut,"<td><img src=/images/new5000/5624_125.jpg width=68 height=9 ></td>");

										 	}
		       	 				}
		       	 		    }
		       	 		}

					/*----------------------------  end  ----------------------------------------*/

		       	}
			}
 
		////////////////  绿色运行 ，注意边框信息  ////////////////////
		fprintf(cgiOut,"<td bgcolor=#C7C7C7>"\
			"<img src=/images/new5000/5624_126.jpg width=9 height=9 ></td>"\
		"<td bgcolor=#C7C7C7>"\
			"<img src=/images/new5000/5624_127.jpg width=19 height=9 ></td>"\
		"<td bgcolor=#C7C7C7>"\
			"<img src=/images/new5000/5624_126.jpg width=9 height=9 ></td>"\
		"<td bgcolor=#C7C7C7>"\
			"<img src=/images/new5000/5624_129.jpg width=12 height=9 ></td>");
		
		fprintf(cgiOut,"<td>");
		fprintf(cgiOut,"<img src=/images/new5000/sep.gif width=1 height=9 >");
	    fprintf(cgiOut,"</td>");
		
	fprintf(cgiOut,"</tr>"\
	"<tr>"\
		"<td colspan=55>"\
			"<img src=/images/new5000/5624_130.jpg width=514 height=37 ></td>"\
		"<td colspan=5>"\
			"<img src=/images/new5000/5624_131.jpg width=117 height=37 ></td>"\
		"<td>"\
			"<img src=/images/new5000/sep.gif width=1 height=37 ></td>"\
	"</tr>"\
"</table>");



return 0;


 /////////////////////// end panel 5000  ////////////////////////////////////////////////////


}

/*5612i panel */
int panel_5K_5612i(unsigned int value,struct eth_port_s pt,int retu,char *n,char *m)
{
  char *flight=(char *)malloc(30);
  char *fsep=(char *)malloc(128);
  char *slight=(char *)malloc(30);
  char *ssep=(char *)malloc(128);
  
  int i;
   fprintf(cgiOut,"<table id=__01 width=726 height=135 border=0 cellpadding=0 cellspacing=0>"\
	"<tr>"\
		"<td rowspan=17>"\
			"<img src=/images/5612i/a_01.jpg width=43 height=134 ></td>"\
		"<td colspan=20>"\
			"<img src=/images/5612i/a_02.jpg width=124 height=18 ></td>"\
		"<td rowspan=17>"\
			"<img src=/images/5612i/a_03.jpg width=75 height=134 ></td>"\
		"<td rowspan=6>"\
			"<img src=/images/5612i/a_04.jpg width=9 height=55 ></td>"
		"<td rowspan=17>"\
			"<img src=/images/5612i/a_05.jpg width=144 height=134 ></td>"\
		"<td colspan=8 rowspan=3>"\
			"<img src=/images/5612i/a_06.jpg width=58 height=38 ></td>"\
		"<td rowspan=17>"\
			"<img src=/images/5612i/a_07.jpg width=44 height=134 ></td>"\
		"<td colspan=7 rowspan=3>"\
			"<img src=/images/5612i/a_08.jpg width=64 height=38 ></td>"\
		"<td rowspan=17>"\
			"<img src=/images/5612i/a_09.jpg width=19 height=134 ></td>"\
		"<td colspan=7 rowspan=5>"\
			"<img src=/images/5612i/a_10.jpg width=117 height=48 ></td>"\
		"<td rowspan=17>"\
			"<img src=/images/5612i/a_11.jpg width=28 height=134 ></td>"\
		"<td>"\
			"<img src=/images/5612i/sep.gif width=1 height=18 ></td>"\
	"</tr>"\
	"<tr>"\
		"<td rowspan=2>"\
			"<img src=/images/5612i/a_12.jpg width=3 height=20 ></td>"\
		"<td colspan=2>"\
			"<img src=/images/5612i/a_15.jpg width=9 height=9 ></td>"\
		"<td colspan=2 rowspan=2>"\
			"<img src=/images/5612i/a_14.jpg width=4 height=20 ></td>"\
		"<td colspan=2>"\
			"<img src=/images/5612i/a_15.jpg width=9 height=9 ></td>"\
		"<td colspan=13 rowspan=2>"\
			"<img src=/images/5612i/a_16.jpg width=99 height=20 ></td>"\
		"<td>"\
			"<img src=/images/5612i/sep.gif width=1 height=9 ></td>"\
	"</tr>"\
	"<tr>"\
		"<td colspan=2>"\
			"<img src=/images/5612i/a_17.jpg width=9 height=11 ></td>"\
		"<td colspan=2>"\
			"<img src=/images/5612i/a_18.jpg width=9 height=11 ></td>"\
		"<td>"\
			"<img src=/images/5612i/sep.gif width=1 height=11 ></td>"\
	"</tr>"\
	"<tr>");

        memset(flight,0,30);
        memset(fsep,0,128);
		memset(slight,0,30);
        memset(ssep,0,128);

        strcpy(flight,"<td colspan=2>");
		strcpy(fsep,"<td colspan=2><img src=/images/5612i/a_20.jpg width=4 height=9 ></td>");
		strcpy(slight,"<td colspan=2>");
		strcpy(ssep,"<td colspan=2><img src=/images/5612i/a_22.jpg width=11 height=9 ></td>");

		 //1
		 light_flag_5612i(1, 1, pt, 1, flight, fsep, slight, ssep);

        memset(flight,0,30);
        memset(fsep,0,128);
		memset(slight,0,30);
        memset(ssep,0,128);


        strcpy(flight,"<td>");
		strcpy(fsep,"<td><img src=/images/5612i/a_20.jpg width=4 height=9 ></td>");
		strcpy(slight,"<td>");
		strcpy(ssep,"<td><img src=/images/5612i/a_22.jpg width=11 height=9 ></td>");

		//3,5
		light_flag_5612i(2, 3, pt, 1, flight, fsep, slight, ssep);

        memset(flight,0,30);
        memset(fsep,0,128);
		memset(slight,0,30);
        memset(ssep,0,128);

        strcpy(flight,"<td>");
		strcpy(fsep,"<td><img src=/images/5612i/a_20.jpg width=4 height=9 ></td>");
		strcpy(slight,"<td>");
		strcpy(ssep,"<td><img src=/images/5612i/a_34.jpg width=3 height=9 ></td>");
        //7
	    light_flag_5612i(4, 4, pt, 1, flight, fsep, slight, ssep);


       for(i=1;i<3;i++)
       {
           		value=1;
           		value=(value << 8) |(2*i-1+8);
           		if(show_eth_port_atrr(value,0,&pt)==CCGI_SUCCESS)
		      	{
     		        if(strcmp(eth_port_type_str[pt.port_type],"ETH_GE_SFP")==0)
         		      fprintf(cgiOut,"<td><img src=/images/5612i/a_15.jpg width=9 height=9 ></td>"); //黑色
      				else  /*type为电口*/
      				{
                        if(strcmp(onoff_status_str[(pt.attr_bitmap & ETH_ATTR_ADMIN_STATUS) >> ETH_ADMIN_STATUS_BIT],"OFF")==0)
         		          fprintf(cgiOut,"<td><img src=/images/5612i/a_15.jpg width=9 height=9 ></td>"); //黑色
      			    	else
      			    	{
      		          		if(strcmp(link_status_str[(pt.attr_bitmap & ETH_ATTR_LINK_STATUS) >> ETH_LINK_STATUS_BIT],"UP")==0 && 0==strcmp(eth_speed_str[(pt.attr_bitmap & ETH_ATTR_SPEED_MASK) >> ETH_SPEED_BIT],"1000M"))
	       			            fprintf(cgiOut,"<td><img src=/images/5612i/a_19.jpg width=9 height=9 ></td>"); //绿色
      				  		else
			                    fprintf(cgiOut,"<td><img src=/images/5612i/a_21.jpg width=9 height=9 ></td>"); //红色
      			    	}
      				}
		      	}
			  else
         		fprintf(cgiOut,"<td><img src=/images/5612i/a_15.jpg width=9 height=9 ></td>"); //黑色

               
				fprintf(cgiOut,"<td><img src=/images/5612i/a_40.jpg width=4 height=9 ></td>");
				
        ///////////////////////////////////////////////////light2///////////////////////////////////////				
				if(show_eth_port_atrr(value,0,&pt)==CCGI_SUCCESS)
     			{
     				if(strcmp(eth_port_type_str[pt.port_type],"ETH_GE_SFP")==0)
         		       fprintf(cgiOut,"<td><img src=/images/5612i/a_15.jpg width=9 height=9 ></td>"); //黑色
     				else  /*type为电口*/
     				{
     					if(strcmp(onoff_status_str[(pt.attr_bitmap & ETH_ATTR_ADMIN_STATUS) >> ETH_ADMIN_STATUS_BIT],"OFF")==0)
         		            fprintf(cgiOut,"<td><img src=/images/5612i/a_15.jpg width=9 height=9 ></td>"); //黑色
     					else
     					{
     						if(strcmp(link_status_str[(pt.attr_bitmap & ETH_ATTR_LINK_STATUS) >> ETH_LINK_STATUS_BIT],"UP")==0)
	       			           fprintf(cgiOut,"<td><img src=/images/5612i/a_19.jpg width=9 height=9 ></td>"); //绿色
     						else
			                  fprintf(cgiOut,"<td><img src=/images/5612i/a_21.jpg width=9 height=9 ></td>"); //红色
     					}
     				}
     			}
     		  else
         		fprintf(cgiOut,"<td><img src=/images/5612i/a_15.jpg width=9 height=9 ></td>"); //黑色

     			switch(i)
				{
					case 1: fprintf(cgiOut,"<td><img src=/images/5612i/a_38.jpg width=12 height=9 ></td>");break;
					case 2: fprintf(cgiOut,"<td><img src=/images/5612i/a_42.jpg width=2 height=9 ></td>");break;
				}
     
			    
           	}

   

        for(i=1;i<3;i++)
		{
			value=1;
			value=(value << 8) |(2*i+8-1);
			if(show_eth_port_atrr(value,0,&pt)==CCGI_SUCCESS)
			{
				if(strcmp(eth_port_type_str[pt.port_type],"ETH_GE_SFP")!=0)
				{
					//两个黑灯 		
					if(i==1)
					{
						fprintf(cgiOut,"<td><img src=/images/5612i/a_15.jpg width=9 height=9 ></td>");
						fprintf(cgiOut,"<td><img src=/images/5612i/a_44.jpg width=8 height=9 ></td>");
						fprintf(cgiOut,"<td><img src=/images/5612i/a_15.jpg width=9 height=9 ></td>");
						fprintf(cgiOut,"<td><img src=/images/5612i/a_46.jpg width=13 height=9 ></td>");
					}
					if(i==2)
					{
						fprintf(cgiOut,"<td><img src=/images/5612i/a_15.jpg width=9 height=9 ></td>");		
						fprintf(cgiOut,"<td><img src=/images/5612i/a_48.jpg width=7 height=9 ></td>");
						fprintf(cgiOut,"<td><img src=/images/5612i/a_15.jpg width=9 height=9 ></td>");
						fprintf(cgiOut,"<td><img src=/images/5612i/sep.gif width=1 height=9 ></td>");
					}
				}

				/*--------------------------  ----------------------------------------*/
				else
				{
					if(strcmp(onoff_status_str[(pt.attr_bitmap & ETH_ATTR_ADMIN_STATUS) >> ETH_ADMIN_STATUS_BIT],"OFF")==0)
					{
						if(i==1)
						{
							fprintf(cgiOut,"<td><img src=/images/5612i/a_15.jpg width=9 height=9 ></td>");
							fprintf(cgiOut,"<td><img src=/images/5612i/a_44.jpg width=8 height=9 ></td>");
							fprintf(cgiOut,"<td><img src=/images/5612i/a_15.jpg width=9 height=9 ></td>");
							fprintf(cgiOut,"<td><img src=/images/5612i/a_46.jpg width=13 height=9 ></td>");
						}
						if(i==2)
						{
							fprintf(cgiOut,"<td><img src=/images/5612i/a_15.jpg width=9 height=9 ></td>");		
							fprintf(cgiOut,"<td><img src=/images/5612i/a_48.jpg width=7 height=9 ></td>");
							fprintf(cgiOut,"<td><img src=/images/5612i/a_15.jpg width=9 height=9 ></td>");
							fprintf(cgiOut,"<td><img src=/images/5612i/sep.gif width=1 height=9 ></td>");
						}
					}
					else
					{
						if(strcmp(link_status_str[(pt.attr_bitmap & ETH_ATTR_LINK_STATUS) >> ETH_LINK_STATUS_BIT],"UP")==0)
						{
							//全绿色
							if(i==1)
							{
								fprintf(cgiOut,"<td><img src=/images/5612i/a_19.jpg width=9 height=9 ></td>");
								fprintf(cgiOut,"<td><img src=/images/5612i/a_44.jpg width=8 height=9 ></td>");
								fprintf(cgiOut,"<td><img src=/images/5612i/a_19.jpg width=9 height=9 ></td>");
								fprintf(cgiOut,"<td><img src=/images/5612i/a_46.jpg width=13 height=9 ></td>");
							}
							if(i==2)
							{
								fprintf(cgiOut,"<td><img src=/images/5612i/a_19.jpg width=9 height=9 ></td>");		
								fprintf(cgiOut,"<td><img src=/images/5612i/a_48.jpg width=7 height=9 ></td>");
								fprintf(cgiOut,"<td><img src=/images/5612i/a_19.jpg width=9 height=9 ></td>");
								fprintf(cgiOut,"<td><img src=/images/5612i/sep.gif width=1 height=9 ></td>");
							}
						}
						else
						{
							//全红色
							if(i==1)
							{
								fprintf(cgiOut,"<td><img src=/images/5612i/a_21.jpg width=9 height=9 ></td>");
								fprintf(cgiOut,"<td><img src=/images/5612i/a_44.jpg width=8 height=9 ></td>");
								fprintf(cgiOut,"<td><img src=/images/5612i/a_21.jpg width=9 height=9 ></td>");
								fprintf(cgiOut,"<td><img src=/images/5612i/a_46.jpg width=13 height=9 ></td>");
							}
							if(i==2)
							{
								fprintf(cgiOut,"<td><img src=/images/5612i/a_21.jpg width=9 height=9 ></td>");		
								fprintf(cgiOut,"<td><img src=/images/5612i/a_48.jpg width=7 height=9 ></td>");
								fprintf(cgiOut,"<td><img src=/images/5612i/a_21.jpg width=9 height=9 ></td>");
								fprintf(cgiOut,"<td><img src=/images/5612i/sep.gif width=1 height=9 ></td>");
							}
						}
					}
				}

			}
		}


	fprintf(cgiOut,"</tr>"\
	"<tr>"\
		"<td colspan=20 rowspan=9>");

    retu=checkuser_group(m);
    if(retu==0)	  
	{
	    fprintf(cgiOut,"<table id=__01 width=124 height=47 border=0 cellpadding=0 cellspacing=0>"\
		"<tr>"\
			"<td rowspan=5>"\
				"<img src=/images/5612i/a_50_01.jpg width=1 height=47 ></td>"\
			"<td colspan=7>"\
				"<img src=/images/5612i/a_50_02.jpg width=122 height=3 ></td>"\
			"<td rowspan=5>"\
				"<img src=/images/5612i/a_50_03.jpg width=1 height=47 ></td>"\
		"</tr>"\
		"<tr>"\
			"<td>");	
		    fprintf(cgiOut,"<a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame>"\
			"<img src=/images/5612i/a_50_04.jpg width=23 height=19 border=0 alt=\"%s\">",n,"1-1","1");
			fprintf(cgiOut,"</td>"\
			"<td>"\
				"<img src=/images/5612i/a_50_05.jpg width=10 height=19 ></td>"\
			"<td>");	
		    fprintf(cgiOut,"<a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame>"\
			"<img src=/images/5612i/a_50_04.jpg width=23 height=19 border=0 alt=\"%s\">",n,"1-3","3");
			fprintf(cgiOut,"</td>"\
			"<td>"\
				"<img src=/images/5612i/a_50_05.jpg width=10 height=19 ></td>"\
			"<td>");	
		    fprintf(cgiOut,"<a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame>"\
			"<img src=/images/5612i/a_50_04.jpg width=23 height=19 border=0 alt=\"%s\">",n,"1-5","5");
			fprintf(cgiOut,"</td>"\
			"<td>"\
				"<img src=/images/5612i/a_50_05.jpg width=10 height=19 ></td>"\
			"<td>");	
		    fprintf(cgiOut,"<a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame>"\
			"<img src=/images/5612i/a_50_04.jpg width=23 height=19 border=0 alt=\"%s\">",n,"1-7","7");
			fprintf(cgiOut,"</td>"\
		"</tr>"\
		"<tr>"\
			"<td colspan=7>"\
				"<img src=/images/5612i/a_50_11.jpg width=122 height=3 ></td>"\
		"</tr>"\
		"<tr>"\
			"<td>");	
		    fprintf(cgiOut,"<a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame>"\
			"<img src=/images/5612i/a_50_12.jpg width=23 height=19 border=0 alt=\"%s\">",n,"1-2","2");
			fprintf(cgiOut,"</td>"\
			"<td rowspan=2>"\
				"<img src=/images/5612i/a_50_13.jpg width=10 height=22 ></td>"\
			"<td>");	
		    fprintf(cgiOut,"<a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame>"\
			"<img src=/images/5612i/a_50_12.jpg width=23 height=19 border=0 alt=\"%s\">",n,"1-4","4");
			fprintf(cgiOut,"</td>"\
			"<td rowspan=2>"\
				"<img src=/images/5612i/a_50_13.jpg width=10 height=22 ></td>"\
			"<td>");	
		    fprintf(cgiOut,"<a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame>"\
			"<img src=/images/5612i/a_50_12.jpg width=23 height=19 border=0 alt=\"%s\">",n,"1-6","6");
			fprintf(cgiOut,"</td>"\
			"<td rowspan=2>"\
				"<img src=/images/5612i/a_50_13.jpg width=10 height=22 ></td>"\
			"<td>");	
		    fprintf(cgiOut,"<a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame>"\
			"<img src=/images/5612i/a_50_12.jpg width=23 height=19 border=0 alt=\"%s\">",n,"1-8","8");
			fprintf(cgiOut,"</td>"\
		"</tr>"\
		"<tr>"\
			"<td>"\
				"<img src=/images/5612i/a_50_19.jpg width=23 height=3 ></td>"\
			"<td>"\
				"<img src=/images/5612i/a_50_19.jpg width=23 height=3 ></td>"\
			"<td>"\
				"<img src=/images/5612i/a_50_19.jpg width=23 height=3 ></td>"\
			"<td>"\
				"<img src=/images/5612i/a_50_19.jpg width=23 height=3 ></td>"\
		"</tr>"\
	"</table>");
	}
    else
	{
	    fprintf(cgiOut,"<table id=__01 width=124 height=47 border=0 cellpadding=0 cellspacing=0>"\
		"<tr>"\
			"<td rowspan=5>"\
				"<img src=/images/5612i/a_50_01.jpg width=1 height=47 ></td>"\
			"<td colspan=7>"\
				"<img src=/images/5612i/a_50_02.jpg width=122 height=3 ></td>"\
			"<td rowspan=5>"\
				"<img src=/images/5612i/a_50_03.jpg width=1 height=47 ></td>"\
		"</tr>"\
		"<tr>"\
			"<td>");	
		    fprintf(cgiOut,"<img src=/images/5612i/a_50_04.jpg width=23 height=19 border=0 alt=\"%s\">","1");
			fprintf(cgiOut,"</td>"\
			"<td>"\
				"<img src=/images/5612i/a_50_05.jpg width=10 height=19 ></td>"\
			"<td>");	
		    fprintf(cgiOut,"<img src=/images/5612i/a_50_04.jpg width=23 height=19 border=0 alt=\"%s\">","3");
			fprintf(cgiOut,"</td>"\
			"<td>"\
				"<img src=/images/5612i/a_50_05.jpg width=10 height=19 ></td>"\
			"<td>");	
		    fprintf(cgiOut,"<img src=/images/5612i/a_50_04.jpg width=23 height=19 border=0 alt=\"%s\">","5");
			fprintf(cgiOut,"</td>"\
			"<td>"\
				"<img src=/images/5612i/a_50_05.jpg width=10 height=19 ></td>"\
			"<td>");	
		    fprintf(cgiOut,"<img src=/images/5612i/a_50_04.jpg width=23 height=19 border=0 alt=\"%s\">","7");
			fprintf(cgiOut,"</td>"\
		"</tr>"\
		"<tr>"\
			"<td colspan=7>"\
				"<img src=/images/5612i/a_50_11.jpg width=122 height=3 ></td>"\
		"</tr>"\
		"<tr>"\
			"<td>");	
		    fprintf(cgiOut,"<img src=/images/5612i/a_50_12.jpg width=23 height=19 border=0 alt=\"%s\">","2");
			fprintf(cgiOut,"</td>"\
			"<td rowspan=2>"\
				"<img src=/images/5612i/a_50_13.jpg width=10 height=22 ></td>"\
			"<td>");	
		    fprintf(cgiOut,"<img src=/images/5612i/a_50_12.jpg width=23 height=19 border=0 alt=\"%s\">","4");
			fprintf(cgiOut,"</td>"\
			"<td rowspan=2>"\
				"<img src=/images/5612i/a_50_13.jpg width=10 height=22 ></td>"\
			"<td>");	
		    fprintf(cgiOut,"<img src=/images/5612i/a_50_12.jpg width=23 height=19 border=0 alt=\"%s\">","6");
			fprintf(cgiOut,"</td>"\
			"<td rowspan=2>"\
				"<img src=/images/5612i/a_50_13.jpg width=10 height=22 ></td>"\
			"<td>");	
		    fprintf(cgiOut,"<img src=/images/5612i/a_50_12.jpg width=23 height=19 border=0 alt=\"%s\">","8");
			fprintf(cgiOut,"</td>"\
		"</tr>"\
		"<tr>"\
			"<td>"\
				"<img src=/images/5612i/a_50_19.jpg width=23 height=3 ></td>"\
			"<td>"\
				"<img src=/images/5612i/a_50_19.jpg width=23 height=3 ></td>"\
			"<td>"\
				"<img src=/images/5612i/a_50_19.jpg width=23 height=3 ></td>"\
			"<td>"\
				"<img src=/images/5612i/a_50_19.jpg width=23 height=3 ></td>"\
		"</tr>"\
	"</table>");
	}
		//////////////
		//fprintf(cgiOut,"<img src=/images/5612i/a_50.jpg width=124 height=47 >");
		
		fprintf(cgiOut,"</td>"\
		"<td colspan=8 rowspan=9>");
         /////////////////
       retu=checkuser_group(m);
       if(retu==0)	  
		{
		    fprintf(cgiOut,"<table id=__01 width=58 height=47 border=0 cellpadding=0 cellspacing=0>"\
			"<tr>"\
				"<td rowspan=5>"\
					"<img src=/images/5612i/a_51_01.jpg width=1 height=47 ></td>"\
				"<td colspan=3>"\
					"<img src=/images/5612i/a_51_02.jpg width=56 height=3 ></td>"\
				"<td rowspan=5>"\
					"<img src=/images/5612i/a_51_01.jpg width=1 height=47 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>");	
			    fprintf(cgiOut,"<a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame>"\
				"<img src=/images/5612i/a_51_04.jpg width=23 height=19 border=0 alt=\"%s\">",n,"1-9","9");
				fprintf(cgiOut,"</td>"\
				"<td rowspan=4>"\
					"<img src=/images/5612i/a_51_05.jpg width=10 height=44 ></td>"\
				"<td>");	
			    fprintf(cgiOut,"<a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame>"\
				"<img src=/images/5612i/a_51_04.jpg width=23 height=19 border=0 alt=\"%s\">",n,"1-11","11");
				fprintf(cgiOut,"</td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=/images/5612i/a_51_07.jpg width=23 height=3 ></td>"\
				"<td>"\
					"<img src=/images/5612i/a_51_07.jpg width=23 height=3 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>");	
			    fprintf(cgiOut,"<a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame>"\
				"<img src=/images/5612i/a_51_09.jpg width=23 height=19 border=0 alt=\"%s\">",n,"1-10","10");
				fprintf(cgiOut,"</td>"\
				"<td>");	
			    fprintf(cgiOut,"<a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame>"\
				"<img src=/images/5612i/a_51_09.jpg width=23 height=19 border=0 alt=\"%s\">",n,"1-12","12");
				fprintf(cgiOut,"</td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=/images/5612i/a_51_07.jpg width=23 height=3 ></td>"\
				"<td>"\
					"<img src=/images/5612i/a_51_07.jpg width=23 height=3 ></td>"\
			"</tr>"\
		"</table>");
		}
        else
		{
		    fprintf(cgiOut,"<table id=__01 width=58 height=47 border=0 cellpadding=0 cellspacing=0>"\
			"<tr>"\
				"<td rowspan=5>"\
					"<img src=/images/5612i/a_51_01.jpg width=1 height=47 ></td>"\
				"<td colspan=3>"\
					"<img src=/images/5612i/a_51_02.jpg width=56 height=3 ></td>"\
				"<td rowspan=5>"\
					"<img src=/images/5612i/a_51_01.jpg width=1 height=47 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>");	
			    fprintf(cgiOut,"<img src=/images/5612i/a_51_04.jpg width=23 height=19 border=0 alt=\"%s\">","9");
				fprintf(cgiOut,"</td>"\
				"<td rowspan=4>"\
					"<img src=/images/5612i/a_51_05.jpg width=10 height=44 ></td>"\
				"<td>");	
			    fprintf(cgiOut,"<img src=/images/5612i/a_51_04.jpg width=23 height=19 border=0 alt=\"%s\">","11");
				fprintf(cgiOut,"</td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=/images/5612i/a_51_07.jpg width=23 height=3 ></td>"\
				"<td>"\
					"<img src=/images/5612i/a_51_07.jpg width=23 height=3 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>");	
			    fprintf(cgiOut,"<img src=/images/5612i/a_51_09.jpg width=23 height=19 border=0 alt=\"%s\">","10");
				fprintf(cgiOut,"</td>"\
				"<td>");	
			    fprintf(cgiOut,"<img src=/images/5612i/a_51_09.jpg width=23 height=19 border=0 alt=\"%s\">","12");
				fprintf(cgiOut,"</td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=/images/5612i/a_51_07.jpg width=23 height=3 ></td>"\
				"<td>"\
					"<img src=/images/5612i/a_51_07.jpg width=23 height=3 ></td>"\
			"</tr>"\
		"</table>");
		}
		  ///////////////
			//fprintf(cgiOut,"<img src=/images/5612i/a_51.jpg width=58 height=47 >");			
			fprintf(cgiOut,"</td>"\
		"<td colspan=7 rowspan=9>"\
			"<img src=/images/5612i/a_52.jpg width=64 height=47 ></td>"\
		"<td>"\
			"<img src=/images/5612i/sep.gif width=1 height=1 ></td>"\
	"</tr>");

	//under
	fprintf(cgiOut,"<tr>"\
		"<td colspan=7 rowspan=3>"\
			"<img src=/images/5612i/a_53.jpg width=117 height=20 ></td>"\
		"<td>"\
			"<img src=/images/5612i/sep.gif width=1 height=7 ></td>"\
	"</tr>"\
	"<tr>");

	    fprintf(cgiOut,"<td>"\
			"<img src=/images/5612i/a_54.jpg width=9 height=9 ></td>"\
		"<td>"\
			"<img src=/images/5612i/sep.gif width=1 height=9 ></td>"\
	"</tr>"\
	"<tr>"\
		"<td rowspan=2>"\
			"<img src=/images/5612i/a_55.jpg width=9 height=11 ></td>"\
		"<td>"\
			"<img src=/images/5612i/sep.gif width=1 height=4 ></td>"\
	"</tr>"\
	"<tr>"\
		"<td colspan=7 rowspan=2>"\
			"<img src=/images/5612i/a_56.jpg width=117 height=14 ></td>"\
		"<td>"\
			"<img src=/images/5612i/sep.gif width=1 height=7 ></td>"\
	"</tr>"\
	"<tr>"\
		"<td rowspan=2>"\
			"<img src=/images/5612i/a_54.jpg width=9 height=9 ></td>"\
		"<td>"\
			"<img src=/images/5612i/sep.gif width=1 height=7 ></td>"\
	"</tr>"\
	"<tr>"\
		"<td rowspan=7>"\
			"<img src=/images/5612i/a_58.jpg width=13 height=52 ></td>"\
		"<td rowspan=5>"\
			"<img src=/images/5612i/a_59.jpg width=23 height=19 ></td>"\
		"<td colspan=5 rowspan=2>"\
			"<img src=/images/5612i/a_60.jpg width=81 height=9 ></td>"\
		"<td>"\
			"<img src=/images/5612i/sep.gif width=1 height=2 ></td>"\
	"</tr>"\
	"<tr>"\
		"<td rowspan=6>"\
			"<img src=/images/5612i/a_61.jpg width=9 height=50 ></td>"\
		"<td>"\
			"<img src=/images/5612i/sep.gif width=1 height=7 ></td>"\
	"</tr>"\
	"<tr>"\
		"<td rowspan=5>"\
			"<img src=/images/5612i/a_62.jpg width=32 height=43 ></td>"\
		"<td rowspan=2>"\
			"<img src=/images/5612i/a_63.jpg width=9 height=9 ></td>"\
		"<td rowspan=5>"\
			"<img src=/images/5612i/a_64.jpg width=19 height=43 ></td>"\
		"<td rowspan=2>"\
			"<img src=/images/5612i/a_65.jpg width=9 height=9 ></td>"\
		"<td rowspan=5>"\
			"<img src=/images/5612i/a_66.jpg width=12 height=43 ></td>"\
		"<td>"\
			"<img src=/images/5612i/sep.gif width=1 height=3 ></td>"\
	"</tr>"\
	"<tr>");

	    memset(flight,0,30);
        memset(fsep,0,128);
		memset(slight,0,30);
        memset(ssep,0,128);

        strcpy(flight,"<td colspan=2 rowspan=3>");
		strcpy(fsep,"<td colspan=2 rowspan=4><img src=/images/5612i/a_68.jpg width=4 height=40 ></td>");
		strcpy(slight,"<td colspan=2 rowspan=3>");
		strcpy(ssep,"<td colspan=2 rowspan=4><img src=/images/5612i/a_70.jpg width=11 height=40 ></td>");

		 //2
		 light_flag_5612i(1, 1, pt, 2, flight, fsep, slight, ssep);


        memset(flight,0,30);
        memset(fsep,0,128);
		memset(slight,0,30);
        memset(ssep,0,128);

        strcpy(flight,"<td rowspan=3>");
		strcpy(fsep,"<td rowspan=4><img src=/images/5612i/a_72.jpg width=4 height=40 ></td>");
		strcpy(slight,"<td rowspan=3>");
		strcpy(ssep,"<td rowspan=4><img src=/images/5612i/a_74.jpg width=11 height=40 ></td>");

		 light_flag_5612i(2, 3, pt, 2, flight, fsep, slight, ssep);

		memset(ssep,0,128);
		strcpy(ssep,"<td rowspan=4><img src=/images/5612i/a_82.jpg width=3 height=40 ></td>");

		light_flag_5612i(4, 4, pt, 2, flight, fsep, slight, ssep);

        for(i=1;i<3;i++)
		{
           		value=1;
           		value=(value << 8) |(2*i+8);
           		if(show_eth_port_atrr(value,0,&pt)==CCGI_SUCCESS)
		      	{
     		        if(strcmp(eth_port_type_str[pt.port_type],"ETH_GE_SFP")==0)
         		      fprintf(cgiOut,"<td rowspan=3><img src=/images/5612i/a_15.jpg width=9 height=9 ></td>"); //黑色
      				else  /*type为电口*/
      				{
                        if(strcmp(onoff_status_str[(pt.attr_bitmap & ETH_ATTR_ADMIN_STATUS) >> ETH_ADMIN_STATUS_BIT],"OFF")==0)
         		          fprintf(cgiOut,"<td rowspan=3><img src=/images/5612i/a_15.jpg width=9 height=9 ></td>"); //黑色
      			    	else
      			    	{
      		          		if(strcmp(link_status_str[(pt.attr_bitmap & ETH_ATTR_LINK_STATUS) >> ETH_LINK_STATUS_BIT],"UP")==0 && 0==strcmp(eth_speed_str[(pt.attr_bitmap & ETH_ATTR_SPEED_MASK) >> ETH_SPEED_BIT],"1000M"))
	       			            fprintf(cgiOut,"<td rowspan=3><img src=/images/5612i/a_19.jpg width=9 height=9 ></td>"); //绿色
      				  		else
			                    fprintf(cgiOut,"<td rowspan=3><img src=/images/5612i/a_21.jpg width=9 height=9 ></td>"); //红色
      			    	}
      				}
		      	}
			  else
         		fprintf(cgiOut,"<td rowspan=3><img src=/images/5612i/a_15.jpg width=9 height=9 ></td>"); //黑色

			 fprintf(cgiOut,"<td rowspan=4><img src=/images/5612i/a_84.jpg width=4 height=40 ></td>");
        ///////////////////////////////////////////////////light2///////////////////////////////////////				
				if(show_eth_port_atrr(value,0,&pt)==CCGI_SUCCESS)
     			{
     				if(strcmp(eth_port_type_str[pt.port_type],"ETH_GE_SFP")==0)
         		       fprintf(cgiOut,"<td rowspan=3><img src=/images/5612i/a_15.jpg width=9 height=9 ></td>"); //黑色
     				else  /*type为电口*/
     				{
     					if(strcmp(onoff_status_str[(pt.attr_bitmap & ETH_ATTR_ADMIN_STATUS) >> ETH_ADMIN_STATUS_BIT],"OFF")==0)
         		            fprintf(cgiOut,"<td rowspan=3><img src=/images/5612i/a_15.jpg width=9 height=9 ></td>"); //黑色
     					else
     					{
     						if(strcmp(link_status_str[(pt.attr_bitmap & ETH_ATTR_LINK_STATUS) >> ETH_LINK_STATUS_BIT],"UP")==0)
	       			           fprintf(cgiOut,"<td rowspan=3><img src=/images/5612i/a_19.jpg width=9 height=9 ></td>"); //绿色
     						else
			                  fprintf(cgiOut,"<td rowspan=3><img src=/images/5612i/a_21.jpg width=9 height=9 ></td>"); //红色
     					}
     				}
     			}
     		  else
         		fprintf(cgiOut,"<td rowspan=3><img src=/images/5612i/a_15.jpg width=9 height=9 ></td>"); //黑色

     			switch(i)
				{
					case 1: fprintf(cgiOut,"<td rowspan=4><img src=/images/5612i/a_86.jpg width=12 height=40 ></td>");break;
					case 2: fprintf(cgiOut,"<td rowspan=4><img src=/images/5612i/a_90.jpg width=2 height=40 ></td>");break;
				}
     
			    
           	}

			for(i=1;i<3;i++)
			{
				value=1;
				value=(value << 8) |(2*i+8);
				if(show_eth_port_atrr(value,0,&pt)==CCGI_SUCCESS)
				{
					if(strcmp(eth_port_type_str[pt.port_type],"ETH_GE_SFP")!=0)
					{
						//两个黑灯 		
						if(i==1)
						{
							fprintf(cgiOut,"<td rowspan=3><img src=/images/5612i/a_15.jpg width=9 height=9 ></td>");		
							fprintf(cgiOut,"<td rowspan=4><img src=/images/5612i/a_92.jpg width=8 height=40 ></td>");
							fprintf(cgiOut,"<td rowspan=3><img src=/images/5612i/a_15.jpg width=9 height=9 ></td>");
							fprintf(cgiOut,"<td rowspan=4><img src=/images/5612i/a_94.jpg width=13 height=40 ></td>");
						}
						if(i==2)
						{
							fprintf(cgiOut,"<td rowspan=3><img src=/images/5612i/a_15.jpg width=9 height=9 ></td>");
							fprintf(cgiOut,"<td rowspan=4><img src=/images/5612i/a_96.jpg width=7 height=40 ></td>");
							fprintf(cgiOut,"<td rowspan=3><img src=/images/5612i/a_15.jpg width=9 height=9 ></td>");
							fprintf(cgiOut,"<td><img src=/images/5612i/sep.gif width=1 height=6 ></td>");
						}
					}

					/*--------------------------  ----------------------------------------*/
					else
					{
						if(strcmp(onoff_status_str[(pt.attr_bitmap & ETH_ATTR_ADMIN_STATUS) >> ETH_ADMIN_STATUS_BIT],"OFF")==0)
						{
							if(i==1)
							{
								fprintf(cgiOut,"<td rowspan=3><img src=/images/5612i/a_15.jpg width=9 height=9 ></td>");		
								fprintf(cgiOut,"<td rowspan=4><img src=/images/5612i/a_92.jpg width=8 height=40 ></td>");
								fprintf(cgiOut,"<td rowspan=3><img src=/images/5612i/a_15.jpg width=9 height=9 ></td>");
								fprintf(cgiOut,"<td rowspan=4><img src=/images/5612i/a_94.jpg width=13 height=40 ></td>");
							}
							if(i==2)
							{
								fprintf(cgiOut,"<td rowspan=3><img src=/images/5612i/a_15.jpg width=9 height=9 ></td>");
								fprintf(cgiOut,"<td rowspan=4><img src=/images/5612i/a_96.jpg width=7 height=40 ></td>");
								fprintf(cgiOut,"<td rowspan=3><img src=/images/5612i/a_15.jpg width=9 height=9 ></td>");
								fprintf(cgiOut,"<td><img src=/images/5612i/sep.gif width=1 height=6 ></td>");
							}
						}
						else
						{
							if(strcmp(link_status_str[(pt.attr_bitmap & ETH_ATTR_LINK_STATUS) >> ETH_LINK_STATUS_BIT],"UP")==0)
							{
								//全绿色
								if(i==1)
								{
									fprintf(cgiOut,"<td rowspan=3><img src=/images/5612i/a_19.jpg width=9 height=9 ></td>");		
									fprintf(cgiOut,"<td rowspan=4><img src=/images/5612i/a_92.jpg width=8 height=40 ></td>");
									fprintf(cgiOut,"<td rowspan=3><img src=/images/5612i/a_19.jpg width=9 height=9 ></td>");
									fprintf(cgiOut,"<td rowspan=4><img src=/images/5612i/a_94.jpg width=13 height=40 ></td>");
								}
								if(i==2)
								{
									fprintf(cgiOut,"<td rowspan=3><img src=/images/5612i/a_19.jpg width=9 height=9 ></td>");
									fprintf(cgiOut,"<td rowspan=4><img src=/images/5612i/a_96.jpg width=7 height=40 ></td>");
									fprintf(cgiOut,"<td rowspan=3><img src=/images/5612i/a_19.jpg width=9 height=9 ></td>");
									fprintf(cgiOut,"<td><img src=/images/5612i/sep.gif width=1 height=6 ></td>");
								}
							}
							else
							{
								//全红色
								if(i==1)
								{
									fprintf(cgiOut,"<td rowspan=3><img src=/images/5612i/a_21.jpg width=9 height=9 ></td>");		
									fprintf(cgiOut,"<td rowspan=4><img src=/images/5612i/a_92.jpg width=8 height=40 ></td>");
									fprintf(cgiOut,"<td rowspan=3><img src=/images/5612i/a_21.jpg width=9 height=9 ></td>");
									fprintf(cgiOut,"<td rowspan=4><img src=/images/5612i/a_94.jpg width=13 height=40 ></td>");
								}
								if(i==2)
								{
									fprintf(cgiOut,"<td rowspan=3><img src=/images/5612i/a_21.jpg width=9 height=9 ></td>");
									fprintf(cgiOut,"<td rowspan=4><img src=/images/5612i/a_96.jpg width=7 height=40 ></td>");
									fprintf(cgiOut,"<td rowspan=3><img src=/images/5612i/a_21.jpg width=9 height=9 ></td>");
									fprintf(cgiOut,"<td><img src=/images/5612i/sep.gif width=1 height=6 ></td>");
								}
							}
						}
					}
				}
			}
		
	fprintf(cgiOut,"</tr>"
	"<tr>"
		"<td rowspan=3>"
			"<img src=/images/5612i/a_98.jpg width=9 height=34 ></td>"
		"<td rowspan=3>"
			"<img src=/images/5612i/a_99.jpg width=9 height=34 ></td>"
		"<td>"
			"<img src=/images/5612i/sep.gif width=1 height=1 ></td>"
	"</tr>"
	"<tr>"
		"<td rowspan=2>"
			"<img src=/images/5612i/a_100.jpg width=23 height=33 ></td>"
		"<td>"
			"<img src=/images/5612i/sep.gif width=1 height=2 ></td>"
	"</tr>"
	"<tr>"
		"<td colspan=2>"
			"<img src=/images/5612i/a_101.jpg width=9 height=31 ></td>"
		"<td colspan=2>"
			"<img src=/images/5612i/a_102.jpg width=9 height=31 ></td>"
		"<td>"
			"<img src=/images/5612i/a_103.jpg width=9 height=31 ></td>"
		"<td>"
			"<img src=/images/5612i/a_104.jpg width=9 height=31 ></td>"
		"<td>"
			"<img src=/images/5612i/a_105.jpg width=9 height=31 ></td>"
		"<td>"
			"<img src=/images/5612i/a_106.jpg width=9 height=31 ></td>"
		"<td>"
			"<img src=/images/5612i/a_107.jpg width=9 height=31 ></td>"
		"<td>"
			"<img src=/images/5612i/a_108.jpg width=9 height=31 ></td>"
		"<td>"
			"<img src=/images/5612i/a_109.jpg width=9 height=31 ></td>"
		"<td>"
			"<img src=/images/5612i/a_110.jpg width=9 height=31 ></td>"
		"<td>"
			"<img src=/images/5612i/a_111.jpg width=9 height=31 ></td>"
		"<td>"
			"<img src=/images/5612i/a_112.jpg width=9 height=31 ></td>"
		"<td>"
			"<img src=/images/5612i/a_113.jpg width=9 height=31 ></td>"
		"<td>"
			"<img src=/images/5612i/a_114.jpg width=9 height=31 ></td>"
		"<td>"
			"<img src=/images/5612i/a_115.jpg width=9 height=31 ></td>"
		"<td>"
			"<img src=/images/5612i/a_116.jpg width=9 height=31 ></td>"
		"<td>"
			"<img src=/images/5612i/sep.gif width=1 height=31 ></td>"
	"</tr>"
	"<tr>"
		"<td>"
			"<img src=/images/5612i/sep.gif width=43 height=1 ></td>"
		"<td>"
			"<img src=/images/5612i/sep.gif width=3 height=1 ></td>"
		"<td>"
			"<img src=/images/5612i/sep.gif width=6 height=1 ></td>"
		"<td>"
			"<img src=/images/5612i/sep.gif width=3 height=1 ></td>"
		"<td>"
			"<img src=/images/5612i/sep.gif width=1 height=1 ></td>"
		"<td>"
			"<img src=/images/5612i/sep.gif width=3 height=1 ></td>"
		"<td>"
			"<img src=/images/5612i/sep.gif width=6 height=1 ></td>"
		"<td>"
			"<img src=/images/5612i/sep.gif width=3 height=1 ></td>"
		"<td>"
			"<img src=/images/5612i/sep.gif width=8 height=1 ></td>"
		"<td>"
			"<img src=/images/5612i/sep.gif width=9 height=1 ></td>"
		"<td>"
			"<img src=/images/5612i/sep.gif width=4 height=1 ></td>"
		"<td>"
			"<img src=/images/5612i/sep.gif width=9 height=1 ></td>"
		"<td>"
			"<img src=/images/5612i/sep.gif width=11 height=1 ></td>"
		"<td>"
			"<img src=/images/5612i/sep.gif width=9 height=1 ></td>"
		"<td>"
			"<img src=/images/5612i/sep.gif width=4 height=1 ></td>"
		"<td>"
			"<img src=/images/5612i/sep.gif width=9 height=1 ></td>"
		"<td>"
			"<img src=/images/5612i/sep.gif width=11 height=1 ></td>"
		"<td>"
			"<img src=/images/5612i/sep.gif width=9 height=1 ></td>"
		"<td>"
			"<img src=/images/5612i/sep.gif width=4 height=1 ></td>"
		"<td>"
			"<img src=/images/5612i/sep.gif width=9 height=1 ></td>"
		"<td>"
			"<img src=/images/5612i/sep.gif width=3 height=1 ></td>"
		"<td>"
			"<img src=/images/5612i/sep.gif width=75 height=1 ></td>"
		"<td>"
			"<img src=/images/5612i/sep.gif width=9 height=1 ></td>"
		"<td>"
			"<img src=/images/5612i/sep.gif width=144 height=1 ></td>"
		"<td>"
			"<img src=/images/5612i/sep.gif width=9 height=1 ></td>"
		"<td>"
			"<img src=/images/5612i/sep.gif width=4 height=1 ></td>"
		"<td>"
			"<img src=/images/5612i/sep.gif width=9 height=1 ></td>"
		"<td>"
			"<img src=/images/5612i/sep.gif width=12 height=1 ></td>"
		"<td>"
			"<img src=/images/5612i/sep.gif width=9 height=1 ></td>"
		"<td>"
			"<img src=/images/5612i/sep.gif width=4 height=1 ></td>"
		"<td>"
			"<img src=/images/5612i/sep.gif width=9 height=1 ></td>"
		"<td>"
			"<img src=/images/5612i/sep.gif width=2 height=1 ></td>"
		"<td>"
			"<img src=/images/5612i/sep.gif width=44 height=1 ></td>"
		"<td>"
			"<img src=/images/5612i/sep.gif width=9 height=1 ></td>"
		"<td>"
			"<img src=/images/5612i/sep.gif width=8 height=1 ></td>"
		"<td>"
			"<img src=/images/5612i/sep.gif width=9 height=1 ></td>"
		"<td>"
			"<img src=/images/5612i/sep.gif width=13 height=1 ></td>"
		"<td>"
			"<img src=/images/5612i/sep.gif width=9 height=1 ></td>"
		"<td>"
			"<img src=/images/5612i/sep.gif width=7 height=1 ></td>"
		"<td>"
			"<img src=/images/5612i/sep.gif width=9 height=1 ></td>"
		"<td>"
			"<img src=/images/5612i/sep.gif width=19 height=1 ></td>"
		"<td>"
			"<img src=/images/5612i/sep.gif width=13 height=1 ></td>"
		"<td>"
			"<img src=/images/5612i/sep.gif width=23 height=1 ></td>"
		"<td>"
			"<img src=/images/5612i/sep.gif width=32 height=1 ></td>"
		"<td>"
			"<img src=/images/5612i/sep.gif width=9 height=1 ></td>"
		"<td>"
			"<img src=/images/5612i/sep.gif width=19 height=1 ></td>"
		"<td>"
			"<img src=/images/5612i/sep.gif width=9 height=1 ></td>"
		"<td>"
			"<img src=/images/5612i/sep.gif width=12 height=1 ></td>"
		"<td>"
			"<img src=/images/5612i/sep.gif width=28 height=1 ></td>"
		"<td></td>"
	"</tr>"
"</table>");

   free(flight);
   free(fsep);
   free(slight);
   free(ssep);
   
   return 0;
}

int panel_5K_5612E(unsigned int value, struct eth_port_s pt, int retu, char * n, char * m)
{
	int i = 0;

	fprintf(cgiOut,"<table width=\"719\" border=\"0\" cellspacing=\"0\" cellpadding=\"0\">\n"\
	"<tr>\n"\
	"<td height=\"103\" align=\"left\" valign=\"top\" background=\"/images/5612e/5608.jpg\"><table width=\"719\" border=\"0\" cellspacing=\"0\" cellpadding=\"0\">\n"\
	"<tr>\n"\
	"<td height=\"43\" colspan=\"2\" align=\"right\" valign=\"bottom\"><table width=\"240\" border=\"0\" cellspacing=\"0\" cellpadding=\"0\">\n"\
	  "<tr>\n"\
	    "<td height=\"8\" colspan=\"3\"></td>\n"\
	  "</tr>\n"\
	  "<tr>\n"\
	    "<td width=\"80\"><img src=\"/images/5612e/c_05.jpg\" width=\"79\" height=\"19\" /></td>\n"\
	    "<td width=\"76\" align=\"center\" class=\"STYLE2\">10/100/1000BASE-T</td>\n"\
	    "<td width=\"84\"><img src=\"/images/5612e/c_06.jpg\" width=\"79\" height=\"19\" /></td>\n"\
	  "</tr>\n"\
	  "<tr>\n"\
	    "<td colspan=\"3\"><table width=\"100%\" border=\"0\" cellspacing=\"0\" cellpadding=\"0\" >\n"\
	      "<tr>\n"\
	        "<td align=\"left\"><span class=\"STYLE2\">LINK/ACT</span></td>\n"\
	        "<td align=\"center\"><span class=\"STYLE2\">SPD LINK/ACT</span></td>\n"\
	        "<td align=\"right\"><span class=\"STYLE2\">SPD</span></td>\n"\
	      "</tr>\n"\
	    "</table></td>\n"\
	  "</tr>\n"\
	"</table></td>\n");
	fprintf(cgiOut,"<td align=\"right\" valign=\"bottom\"><table width=\"181\" border=\"0\" cellspacing=\"0\" cellpadding=\"0\">\n"\
	  "<tr>\n"\
	    "<td height=\"8\" colspan=\"3\"></td>\n"\
	  "</tr>\n"\
	  "<tr>\n"\
	    "<td width=\"65\"><img src=\"/images/5612e/c_07.jpg\" width=\"60\" height=\"19\" /></td>\n"\
	    "<td width=\"59\" align=\"center\" class=\"STYLE2\">1000BASE-X</td>\n"\
	    "<td width=\"62\"><img src=\"/images/5612e/c_08.jpg\" width=\"60\" height=\"19\" /></td>\n"\
	  "</tr>\n"\
	  "<tr>\n"\
	    "<td colspan=\"3\" style=\"overflow:hidden; height:8px; line-height:8px; text-align:left\"><table width=\"100%\" border=\"0\" cellspacing=\"0\" >\n"\
	      "<tr>\n"\
	        "<td><span class=\"STYLE4\">ACT</span>/<span class=\"STYLE4\">LINK</span></td>\n"\
	      "</tr>\n"\
	    "</table></td>\n"\
	  "</tr>\n"\
	"</table></td>\n"\
	"<td align=\"right\" valign=\"bottom\"><table width=\"60\" border=\"0\" cellspacing=\"0\" cellpadding=\"0\">\n"\
	  "<tr>\n"\
	    "<td width=\"42\" align=\"right\" valign=\"bottom\" class=\"STYLE2 STYLE4\">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class=\"STYLE5\">RUN</span>&nbsp;<span class=\"STYLE5\">PWR</span></td>\n"\
	  "</tr>\n"\
	"</table></td>\n"\
	"<td width=\"155\" colspan=\"2\" rowspan=\"3\" align=\"left\" valign=\"top\"><table width=\"106\" border=\"0\" cellspacing=\"0\" cellpadding=\"0\">\n"\
	  "<tr>\n"\
	    "<td height=\"35\" colspan=\"2\">&nbsp;</td>\n"\
	  "</tr>\n"\
	  "<tr>\n"\
	    "<td colspan=\"2\" align=\"right\" valign=\"top\"><img src=\"/images/5612e/logo.jpg\" width=\"74\" height=\"9\" /></td>\n"\
	  "</tr>\n"\
	  "<tr>\n"\
	    "<td width=\"30\" height=\"27\" align=\"left\">&nbsp;</td>\n"\
	    "<td width=\"76\" align=\"right\" valign=\"bottom\"><span class=\"STYLE1\">auteX 5612E</span></td>\n"\
	  "</tr>\n"\
	"</table></td>\n"\
	"</tr>\n");
	fprintf(cgiOut,"<tr>\n"\
	"<td width=\"146\" height=\"24\" align=\"right\" valign=\"top\"><table width=\"102\" border=\"0\" cellspacing=\"0\" cellpadding=\"0\">\n"\
	"<tr>\n");
	/*port 1,2,3,4 only gtx*/
	///////////
	for (i = 1;i < 5;i++)
	{
		value=1;
		value=(value << 8) |(i);
		fprintf(cgiOut,"<td width=\"26\">\n");
		if(show_eth_port_atrr(value,0,&pt)==CCGI_SUCCESS)
		{
			if(strcmp(eth_port_type_str[pt.port_type],"ETH_GE_SFP")==0)
			{
				fprintf(cgiOut,"<img src=\"/images/5612e/q.jpg\" alt=\"\" width=\"8\" height=\"8\" />"); //黑色
			}
			else  /*type为电口*/
			{
				if(strcmp(onoff_status_str[(pt.attr_bitmap & ETH_ATTR_ADMIN_STATUS) >> ETH_ADMIN_STATUS_BIT],"OFF")==0)
				{
					fprintf(cgiOut,"<img src=\"/images/5612e/q.jpg\" alt=\"\" width=\"8\" height=\"8\" />"); //黑色
				}
				else
				{
					if(strcmp(link_status_str[(pt.attr_bitmap & ETH_ATTR_LINK_STATUS) >> ETH_LINK_STATUS_BIT],"UP")==0)
					{
						fprintf(cgiOut,"<img src=\"/images/5612e/h.jpg\" alt=\"\" width=\"8\" height=\"8\" />"); //绿色
					}
					else
					{
						fprintf(cgiOut,"<img src=\"/images/5612e/h_1.jpg\" alt=\"\" width=\"8\" height=\"8\" />"); //红色
					}
				}
			}
		}
		else
		{
			fprintf(cgiOut,"<img src=\"/images/5612e/q.jpg\" alt=\"\" width=\"8\" height=\"8\" />"); //黑色
		}
		fprintf(cgiOut,"&nbsp;");
		///////////////////////////////////////////////////light2///////////////////////////////////////		
		if(show_eth_port_atrr(value,0,&pt)==CCGI_SUCCESS)
		{
			if(strcmp(eth_port_type_str[pt.port_type],"ETH_GE_SFP")==0)
			{
				fprintf(cgiOut,"<img src=\"/images/5612e/q.jpg\" alt=\"\" width=\"8\" height=\"8\" />"); //黑色
			}
			else  /*type为电口*/
			{
				if(strcmp(onoff_status_str[(pt.attr_bitmap & ETH_ATTR_ADMIN_STATUS) >> ETH_ADMIN_STATUS_BIT],"OFF")==0)
				{
					fprintf(cgiOut,"<img src=\"/images/5612e/q.jpg\" alt=\"\" width=\"8\" height=\"8\" />"); //黑色
				}
				else
				{
					if(strcmp(link_status_str[(pt.attr_bitmap & ETH_ATTR_LINK_STATUS) >> ETH_LINK_STATUS_BIT],"UP")==0 && 0==strcmp(eth_speed_str[(pt.attr_bitmap & ETH_ATTR_SPEED_MASK) >> ETH_SPEED_BIT],"1000M"))
					{
						fprintf(cgiOut,"<img src=\"/images/5612e/h.jpg\" alt=\"\" width=\"8\" height=\"8\" />"); //绿色
					}
					else
					{
						fprintf(cgiOut,"<img src=\"/images/5612e/h_1.jpg\" alt=\"\" width=\"8\" height=\"8\" />"); //红色
					}
				}
			}
		}
		else
		{
			fprintf(cgiOut,"<img src=/images/5612e/r.jpg width=8 height=8 >"); //黑色
		}
		fprintf(cgiOut,"</td>\n");

	}
	//////////
	fprintf(cgiOut,"</tr>\n");
	  fprintf(cgiOut,"<tr>\n");
	if (0 == retu)
	{
		for (i = 1; i < 5; i++)
		{
				fprintf(cgiOut,"<td><a href=wp_prtcfg.cgi?UN=%s&ID=1-%d target=mainFrame>\n"\
				"<img src=\"/images/5612e/c_02.jpg\" alt=\"%d\" width=\"23\" height=\"21\" border=0/></td>\n",n,i,i);
		}
	}
	else
	{
		for (i = 1; i < 5; i++)
		{
				fprintf(cgiOut,"<td><img src=\"/images/5612e/c_02.jpg\" alt=\"%d\" width=\"23\" height=\"21\" border=0/></td>\n",i);
		}
	}
	fprintf(cgiOut,"</tr>\n");
	fprintf(cgiOut,"</table></td>\n"\
	"<td width=\"138\" align=\"right\" valign=\"top\"><table width=\"102\" border=\"0\" cellspacing=\"0\" cellpadding=\"0\">\n"\
	  "<tr>\n");
	for (i = 5;i < 9;i++)
	{
		value=1;
		value=(value << 8) |(i);
		fprintf(cgiOut,"<td width=\"26\">\n");
		if(show_eth_port_atrr(value,0,&pt)==CCGI_SUCCESS)
		{
			if(strcmp(eth_port_type_str[pt.port_type],"ETH_GE_SFP")==0)
			{
				fprintf(cgiOut,"<img src=\"/images/5612e/q.jpg\" alt=\"\" width=\"8\" height=\"8\" />"); //黑色
			}
			else  /*type为电口*/
			{
				if(strcmp(onoff_status_str[(pt.attr_bitmap & ETH_ATTR_ADMIN_STATUS) >> ETH_ADMIN_STATUS_BIT],"OFF")==0)
				{
					fprintf(cgiOut,"<img src=\"/images/5612e/q.jpg\" alt=\"\" width=\"8\" height=\"8\" />"); //黑色
				}
				else
				{
					if(strcmp(link_status_str[(pt.attr_bitmap & ETH_ATTR_LINK_STATUS) >> ETH_LINK_STATUS_BIT],"UP")==0 && 0==strcmp(eth_speed_str[(pt.attr_bitmap & ETH_ATTR_SPEED_MASK) >> ETH_SPEED_BIT],"1000M"))
					{
						fprintf(cgiOut,"<img src=\"/images/5612e/h.jpg\" alt=\"\" width=\"8\" height=\"8\" />"); //绿色
					}
					else
					{
						fprintf(cgiOut,"<img src=\"/images/5612e/h_1.jpg\" alt=\"\" width=\"8\" height=\"8\" />"); //红色
					}
				}
			}
		}
		else
		{
			fprintf(cgiOut,"<img src=\"/images/5612e/q.jpg\" alt=\"\" width=\"8\" height=\"8\" />"); //黑色
		}
		fprintf(cgiOut,"&nbsp;");
		///////////////////////////////////////////////////light2///////////////////////////////////////				
		if(show_eth_port_atrr(value,0,&pt)==CCGI_SUCCESS)
		{
			if(strcmp(eth_port_type_str[pt.port_type],"ETH_GE_SFP")==0)
			{
				fprintf(cgiOut,"<img src=\"/images/5612e/q.jpg\" alt=\"\" width=\"8\" height=\"8\" />"); //黑色
			}
			else  /*type为电口*/
			{
				if(strcmp(onoff_status_str[(pt.attr_bitmap & ETH_ATTR_ADMIN_STATUS) >> ETH_ADMIN_STATUS_BIT],"OFF")==0)
				{
					fprintf(cgiOut,"<img src=\"/images/5612e/q.jpg\" alt=\"\" width=\"8\" height=\"8\" />"); //黑色
				}
				else
				{
					if(strcmp(link_status_str[(pt.attr_bitmap & ETH_ATTR_LINK_STATUS) >> ETH_LINK_STATUS_BIT],"UP")==0 && 0==strcmp(eth_speed_str[(pt.attr_bitmap & ETH_ATTR_SPEED_MASK) >> ETH_SPEED_BIT],"1000M"))
					{
						fprintf(cgiOut,"<img src=\"/images/5612e/h.jpg\" alt=\"\" width=\"8\" height=\"8\" />"); //绿色
					}
					else
					{
						fprintf(cgiOut,"<img src=\"/images/5612e/h_1.jpg\" alt=\"\" width=\"8\" height=\"8\" />"); //红色
					}
				}
			}
		}
		else
		{
			fprintf(cgiOut,"<img src=/images/5612e/h_1.jpg width=8 height=8 >"); //黑色
		}
		fprintf(cgiOut,"</td>\n");

	}

	fprintf(cgiOut,"</tr>\n"\
	  "<tr>\n");
	if (0 == retu)
	{
		for (i=5;i<9;i++)
		{
				fprintf(cgiOut,"<td><a href=wp_prtcfg.cgi?UN=%s&ID=1-%d target=mainFrame>\n"
				"<img src=\"/images/5612e/c_02.jpg\" alt=\"%d\" width=\"23\" height=\"21\" border=0/></td>\n",n,i,i);
		}
	}
	else
	{
		for (i=5;i<9;i++)
		{
				fprintf(cgiOut,"<td><img src=\"/images/5612e/c_02.jpg\" alt=\"%d\" width=\"23\" height=\"21\" border=0/></td>\n",i);
		}
	}
	fprintf(cgiOut,"</tr>\n"\
	"</table></td>\n");
	fprintf(cgiOut,"<td width=\"203\" align=\"right\" valign=\"top\"><table width=\"200\" border=\"0\" cellspacing=\"0\" cellpadding=\"0\">\n"\
	  "<tr>\n");
    fprintf(cgiOut,"<td width=\"30\" rowspan=\"2\" align=\"right\" valign=\"bottom\"></td>\n");
	////////////////////
	for (i = 5;i < 9;i++)
	{
		value=1;
		value=(value << 8) |i;
		if(show_eth_port_atrr(value,0,&pt)==CCGI_SUCCESS)
		{
			if(strcmp(eth_port_type_str[pt.port_type],"ETH_GE_SFP")!=0)
			{
				//两个黑灯 		
				if(i == 8)
				{
					fprintf(cgiOut,"<td width=\"30\" rowspan=\"2\" align=\"right\" valign=\"bottom\"><img src=\"/images/5612e/r.jpg\" alt=\"\" width=\"8\" height=\"8\" /> <img src=\"/images/5612e/r.jpg\" alt=\"\" width=\"8\" height=\"8\" /></td>\n");
				}
				else 
				{
					fprintf(cgiOut,"<td width=\"30\" rowspan=\"2\" align=\"right\" valign=\"bottom\"><img src=\"/images/5612e/r.jpg\" alt=\"\" width=\"8\" height=\"8\" /> <img src=\"/images/5612e/r.jpg\" alt=\"\" width=\"8\" height=\"8\" /></td>\n");
					fprintf(cgiOut,"<td width=\"17\" rowspan=\"2\" align=\"right\" valign=\"bottom\"></td>\n");
				}
			}

			/*--------------------------  ----------------------------------------*/
			else
			{
				if(strcmp(onoff_status_str[(pt.attr_bitmap & ETH_ATTR_ADMIN_STATUS) >> ETH_ADMIN_STATUS_BIT],"OFF")==0)
				{
					if(i == 8)
					{
						fprintf(cgiOut,"<td width=\"30\" rowspan=\"2\" align=\"right\" valign=\"bottom\"><img src=\"/images/5612e/r.jpg\" alt=\"\" width=\"8\" height=\"8\" /> <img src=\"/images/5612e/r.jpg\" alt=\"\" width=\"8\" height=\"8\" /></td>\n");
					}
					else
					{
						fprintf(cgiOut,"<td width=\"30\" rowspan=\"2\" align=\"right\" valign=\"bottom\"><img src=\"/images/5612e/r.jpg\" alt=\"\" width=\"8\" height=\"8\" /> <img src=\"/images/5612e/r.jpg\" alt=\"\" width=\"8\" height=\"8\" /></td>\n");
						fprintf(cgiOut,"<td width=\"17\" rowspan=\"2\" align=\"right\" valign=\"bottom\"></td>\n");
					}
				}
				else
				{
					if(strcmp(link_status_str[(pt.attr_bitmap & ETH_ATTR_LINK_STATUS) >> ETH_LINK_STATUS_BIT],"UP")==0)
					{
						//全绿色
						if(i == 8)
						{
							fprintf(cgiOut,"<td width=\"30\" rowspan=\"2\" align=\"right\" valign=\"bottom\"><img src=\"/images/5612e/h.jpg\" alt=\"\" width=\"8\" height=\"8\" /> <img src=\"/images/5612e/h.jpg\" alt=\"\" width=\"8\" height=\"8\" /></td>\n");
						}
						else 
						{
							fprintf(cgiOut,"<td width=\"30\" rowspan=\"2\" align=\"right\" valign=\"bottom\"><img src=\"/images/5612e/h.jpg\" alt=\"\" width=\"8\" height=\"8\" /> <img src=\"/images/5612e/h.jpg\" alt=\"\" width=\"8\" height=\"8\" /></td>\n");
							fprintf(cgiOut,"<td width=\"17\" rowspan=\"2\" align=\"right\" valign=\"bottom\"></td>\n");
						}
					}
					else
					{
						//全红色
						if(i == 8)
						{
							fprintf(cgiOut,"<td width=\"30\" rowspan=\"2\" align=\"right\" valign=\"bottom\"><img src=\"/images/5612e/h_1.jpg\" alt=\"\" width=\"8\" height=\"8\" /> <img src=\"/images/5612e/h_1.jpg\" alt=\"\" width=\"8\" height=\"8\" /></td>\n");
						}
						else
						{
							fprintf(cgiOut,"<td width=\"30\" rowspan=\"2\" align=\"right\" valign=\"bottom\"><img src=\"/images/5612e/h_1.jpg\" alt=\"\" width=\"8\" height=\"8\" /> <img src=\"/images/5612e/h_1.jpg\" alt=\"\" width=\"8\" height=\"8\" /></td>\n");
							fprintf(cgiOut,"<td width=\"17\" rowspan=\"2\" align=\"right\" valign=\"bottom\"></td>\n");
						}
					}
				}
			}

		}
	}
	/////////////////////
	fprintf(cgiOut,"</tr>\n"\
	  "<tr></tr>\n"\
	  "<tr>\n"\
	    "<td height=\"20\" align=\"right\" valign=\"bottom\">&nbsp;</td>\n"\
	    "<td align=\"right\" valign=\"bottom\"><img src=\"/images/5612e/c_04.jpg\" alt=\"\" width=\"23\" height=\"19\" /></td>\n"\
	    "<td align=\"right\" valign=\"bottom\">&nbsp;</td>\n"\
	    "<td align=\"right\" valign=\"bottom\"><img src=\"/images/5612e/c_04.jpg\" alt=\"\" width=\"23\" height=\"19\" /></td>\n"\
	    "<td align=\"right\" valign=\"bottom\">&nbsp;</td>\n"\
	    "<td align=\"right\" valign=\"bottom\"><img src=\"/images/5612e/c_04.jpg\" alt=\"\" width=\"23\" height=\"19\" /></td>\n"\
	    "<td align=\"right\" valign=\"bottom\">&nbsp;</td>\n"\
	    "<td align=\"right\" valign=\"bottom\"><img src=\"/images/5612e/c_04.jpg\" alt=\"\" width=\"23\" height=\"19\" /></td>\n"\
	  "</tr>\n"\
	"</table></td>\n");
	fprintf(cgiOut,"<td width=\"77\" align=\"right\" valign=\"bottom\"><table width=\"60\" border=\"0\" cellspacing=\"0\" cellpadding=\"0\">\n"\
	  "<tr>\n"\
	    "<td width=\"46\" rowspan=\"3\" align=\"right\" valign=\"bottom\"><img src=\"/images/5612e/r.jpg\" alt=\"\" width=\"8\" height=\"8\" />&nbsp;<img src=\"/images/5612e/r.jpg\" alt=\"\" width=\"8\" height=\"8\" /></td>\n"\
	    "<td width=\"1\" rowspan=\"3\"></td>\n"\
	    "<td width=\"13\" align=\"left\"></td>\n"\
	  "</tr>\n"\
	  "<tr>\n"\
	    "<td align=\"left\"></td>\n"\
	  "</tr>\n"\
	  "<tr>\n"\
	    "<td height=\"8\" align=\"left\"></td>\n"\
	  "</tr>\n"\
	  "<tr>\n"\
	    "<td align=\"right\" valign=\"bottom\"><img src=\"/images/5612e/c_03.jpg\" alt=\"\" width=\"23\" height=\"21\" /></td>\n"\
	    "<td></td>\n"\
	    "<td height=\"12\" align=\"left\">&nbsp;</td>\n"\
	  "</tr>\n"\
	"</table></td>\n"\
	"</tr>\n");
	fprintf(cgiOut,"<tr>\n"\
	"<td align=\"right\" valign=\"bottom\">&nbsp;</td>\n"\
	"<td align=\"right\" valign=\"bottom\">&nbsp;</td>\n"\
	"<td align=\"right\" valign=\"top\">&nbsp;</td>\n"\
	"<td>&nbsp;</td>\n"\
	"</tr>\n"\
	"</table></td>\n"\
	"</tr>\n"\
	"</table>\n");

	return 0;
}
/*5000的面板函数，根据产品id来判断*/
int panel_5k_4624(struct sys_ver ptrsysver,unsigned int value,struct eth_port_s pt,int retu,struct list *lcon,char *n,char *m)
{
  char cstring[128];  //字符串
  memset(cstring,0,128);
  int i;
 ////////////////////// panel 5000 //////////////////////////////////////////////////////

     //ccgi_dbus_init();
     //show_sys_ver(&ptrsysver);
	 
fprintf(cgiOut,"<table id=__01 width=730 height=139 border=0 cellpadding=0 cellspacing=0>"\
	"<tr>"\
		"<td rowspan=9>"\
			"<img src=/images/new5000/new_5624_01.jpg width=48 height=139 ></td>"\
		"<td colspan=56>"\
			"<img src=/images/new5000/5624_02.jpg width=534 height=20 ></td>"\
		"<td colspan=5 rowspan=5>"\
			"<img src=/images/new5000/5624_03.jpg width=117 height=50 ></td>"\
		"<td rowspan=9>"\
			"<img src=/images/new5000/5624_04.jpg width=30 height=139 ></td>"\
		"<td>"\
			"<img src=/images/new5000/sep.gif width=1 height=20 ></td>"\
	"</tr>"\
	"<tr>"\
		"<td>"\
			"<img src=/images/new5000/5624_05.jpg width=9 height=9 ></td>"\
		"<td rowspan=2>"\
			"<img src=/images/new5000/5624_06.jpg width=4 height=17 ></td>"\
		"<td>"\
			"<img src=/images/new5000/5624_05.jpg width=9 height=9 ></td>"\
		"<td colspan=53 rowspan=2>"\
			"<img src=/images/new5000/5624_08.jpg width=512 height=17 ></td>"\
		"<td>"\
			"<img src=/images/new5000/sep.gif width=1 height=9 ></td>"\
	"</tr>"\
	"<tr>"\
		"<td>"\
			"<img src=/images/new5000/5624_09.jpg width=9 height=8 ></td>"\
		"<td>"\
			"<img src=/images/new5000/5624_10.jpg width=9 height=8 ></td>"\
		"<td>"\
			"<img src=/images/new5000/sep.gif width=1 height=8 ></td>"\
	"</tr>"\
	"<tr>");
		


         ///////////////1 3 5 7 9  普通 /////////////////

		 light_flag_new(1,6,pt,1);
		 memset(cstring,0,128);
		 strcpy(cstring,"<td rowspan=5><img src=/images/new5000/5624_34.jpg width=38 height=65 ></td>");		
		 light_spe_new(11,pt,cstring);

      ////////////////// 11 口结束  ///////////////
			
		light_flag(1,4,pt,1);
	  	memset(cstring,0,128);
		strcpy(cstring,"<td><img src=/images/new5000/5624_50.jpg width=12 height=9 ></td>");		
        light_spe(7,pt,cstring);
			
	

		////////////  到九，十一个端口  光电互斥口////////////////////////////////////		
	  		for(i=1;i<3;i++)
           	{
           		value=1;
           		value=(value << 8) |(2*i-1+8);
           		if(show_eth_port_atrr(value,0,&pt)==CCGI_SUCCESS)
		      	{
     		        if(strcmp(eth_port_type_str[pt.port_type],"ETH_GE_SFP")==0)
         		      fprintf(cgiOut,"<td><img src=/images/new5000/5624_05.jpg width=9 height=9 ></td>"); //黑色
      				else  /*type为电口*/
      				{
                        if(strcmp(onoff_status_str[(pt.attr_bitmap & ETH_ATTR_ADMIN_STATUS) >> ETH_ADMIN_STATUS_BIT],"OFF")==0)
         		          fprintf(cgiOut,"<td><img src=/images/new5000/5624_05.jpg width=9 height=9 ></td>"); //黑色
      			    	else
      			    	{
      		          		if(strcmp(link_status_str[(pt.attr_bitmap & ETH_ATTR_LINK_STATUS) >> ETH_LINK_STATUS_BIT],"UP")==0 && 0==strcmp(eth_speed_str[(pt.attr_bitmap & ETH_ATTR_SPEED_MASK) >> ETH_SPEED_BIT],"1000M"))
	       			            fprintf(cgiOut,"<td><img src=/images/new5000/5624_11.jpg width=9 height=9 ></td>"); //绿色
      				  		else
			                    fprintf(cgiOut,"<td><img src=/images/new5000/5624_13.jpg width=9 height=9 ></td>"); //红色
      			    	}
      				}
		      	}
			  else
         		fprintf(cgiOut,"<td><img src=/images/new5000/5624_05.jpg width=9 height=9 ></td>"); //黑色

				fprintf(cgiOut,"<td><img src=/images/new5000/5624_12.jpg width=4 height=9 ""></td>");

        ///////////////////////////////////////////////////light2///////////////////////////////////////				
				if(show_eth_port_atrr(value,0,&pt)==CCGI_SUCCESS)
     			{
     				if(strcmp(eth_port_type_str[pt.port_type],"ETH_GE_SFP")==0)
         		       fprintf(cgiOut,"<td><img src=/images/new5000/5624_05.jpg width=9 height=9 ></td>"); //黑色
     				else  /*type为电口*/
     				{
     					if(strcmp(onoff_status_str[(pt.attr_bitmap & ETH_ATTR_ADMIN_STATUS) >> ETH_ADMIN_STATUS_BIT],"OFF")==0)
         		            fprintf(cgiOut,"<td><img src=/images/new5000/5624_05.jpg width=9 height=9 ></td>"); //黑色
     					else
     					{
     						if(strcmp(link_status_str[(pt.attr_bitmap & ETH_ATTR_LINK_STATUS) >> ETH_LINK_STATUS_BIT],"UP")==0)
	       			           fprintf(cgiOut,"<td><img src=/images/new5000/5624_11.jpg width=9 height=9 ></td>"); //绿色
     						else
			                  fprintf(cgiOut,"<td><img src=/images/new5000/5624_13.jpg width=9 height=9 ></td>"); //红色
     					}
     				}
     			}
     		  else
         		fprintf(cgiOut,"<td><img src=/images/new5000/5624_05.jpg width=9 height=9 ></td>"); //黑色

     			switch(i)
				{
					case 1: fprintf(cgiOut,"<td><img src=/images/new5000/5624_14.jpg width=11 height=9 ></td>");break;
					case 2: fprintf(cgiOut,"<td rowspan=5><img src=/images/new5000/5624_58.jpg width=40 height=65 ></td>");break;
				}
     
			    
           	}


      ////////////////////////////////////////////////////////光口 指示灯 9， 11 /////////////////////////////////////////////  
            for(i=1;i<3;i++)
			{
				value=1;
           		value=(value << 8) |(2*i+8-1);
           		if(show_eth_port_atrr(value,0,&pt)==CCGI_SUCCESS)
		      	{
		       	 	if(strcmp(eth_port_type_str[pt.port_type],"ETH_GE_SFP")!=0)
		       	 		{
		       	//两个黑灯 		
		       	if(i==1){
         		fprintf(cgiOut,"<td><img src=/images/new5000/5624_05.jpg width=9 height=9 ></td>"); //黑色
		        fprintf(cgiOut,"<td><img src=/images/new5000/5624_12.jpg width=4 height=9 ></td>");
         		fprintf(cgiOut,"<td><img src=/images/new5000/5624_05.jpg width=9 height=9 ></td>"); //黑色
		        fprintf(cgiOut,"<td rowspan=5><img src=/images/new5000/5624_62.jpg width=16 height=65 ></td>");
		       		}
				if(i==2){
         		fprintf(cgiOut,"<td><img src=/images/new5000/5624_05.jpg width=9 height=9 ></td>"); //黑色
		        fprintf(cgiOut,"<td><img src=/images/new5000/5624_64.jpg width=5 height=9 ></td>");
         		fprintf(cgiOut,"<td><img src=/images/new5000/5624_05.jpg width=9 height=9 ></td>"); //黑色
		        fprintf(cgiOut,"<td rowspan=6><img src=/images/new5000/5624_66.jpg width=20 height=102 ></td>");
		        fprintf(cgiOut,"<td><img src=/images/new5000/sep.gif width=1 height=9 ></td>");
					}
		       	 		}

			/*--------------------------  ----------------------------------------*/
		       	 	else
		       	 		{
		       	 			if(strcmp(onoff_status_str[(pt.attr_bitmap & ETH_ATTR_ADMIN_STATUS) >> ETH_ADMIN_STATUS_BIT],"OFF")==0)
		       	 				{
		       	 				        if(i==1){
        		       	 				fprintf(cgiOut,"<td><img src=/images/new5000/5624_05.jpg width=9 height=9 ></td>"); //黑色
                        		        fprintf(cgiOut,"<td><img src=/images/new5000/5624_12.jpg width=4 height=9 ></td>");
                                 		fprintf(cgiOut,"<td><img src=/images/new5000/5624_05.jpg width=9 height=9 ></td>"); //黑色
                        		        fprintf(cgiOut,"<td rowspan=5><img src=/images/new5000/5624_62.jpg width=16 height=65 ></td>");
		       	 				        	}
										if(i==2){
                                 		fprintf(cgiOut,"<td><img src=/images/new5000/5624_05.jpg width=9 height=9 ></td>"); //黑色
                        		        fprintf(cgiOut,"<td><img src=/images/new5000/5624_64.jpg width=5 height=9 ></td>");
                                 		fprintf(cgiOut,"<td><img src=/images/new5000/5624_05.jpg width=9 height=9 ></td>"); //黑色
                        		        fprintf(cgiOut,"<td rowspan=6><img src=/images/new5000/5624_66.jpg width=20 height=102 ></td>");
                        		        fprintf(cgiOut,"<td><img src=/images/new5000/sep.gif width=1 height=9 ></td>");
                        		       	 }
										}
		       	 			  else
		       	 				{
								if(strcmp(link_status_str[(pt.attr_bitmap & ETH_ATTR_LINK_STATUS) >> ETH_LINK_STATUS_BIT],"UP")==0)
		       	 				{
		       	 				//全绿色
		       	 				 if(i==1){
										fprintf(cgiOut,"<td><img src=/images/new5000/5624_11.jpg width=9 height=9 ></td>"); //绿色
                        		        fprintf(cgiOut,"<td><img src=/images/new5000/5624_12.jpg width=4 height=9 ></td>");
                                 		fprintf(cgiOut,"<td><img src=/images/new5000/5624_11.jpg width=9 height=9 ></td>"); //绿色
                        		        fprintf(cgiOut,"<td rowspan=5><img src=/images/new5000/5624_62.jpg width=16 height=65 ></td>");
		       	 					 }
								 if(i==2){
                                 		fprintf(cgiOut,"<td><img src=/images/new5000/5624_11.jpg width=9 height=9 ></td>"); //绿色
                        		        fprintf(cgiOut,"<td><img src=/images/new5000/5624_64.jpg width=5 height=9 ></td>");
                                 		fprintf(cgiOut,"<td><img src=/images/new5000/5624_11.jpg width=9 height=9 ></td>"); //绿色
                        		        fprintf(cgiOut,"<td rowspan=6><img src=/images/new5000/5624_66.jpg width=20 height=102 ></td>");
                        		        fprintf(cgiOut,"<td><img src=/images/new5000/sep.gif width=1 height=9 ></td>");
								 	}
								 }
		       	 			else
		       	 				{
		       	 				//全红色
		       	 				         if(i==1){
		       	 						fprintf(cgiOut,"<td><img src=/images/new5000/5624_13.jpg width=9 height=9 ></td>"); //红色
                        		        fprintf(cgiOut,"<td><img src=/images/new5000/5624_12.jpg width=4 height=9 ></td>");
                                 		fprintf(cgiOut,"<td><img src=/images/new5000/5624_13.jpg width=9 height=9 ></td>"); //红色
                        		        fprintf(cgiOut,"<td rowspan=5><img src=/images/new5000/5624_62.jpg width=16 height=65 ></td>");
		       	 				         	}
										 if(i==2)
										 	{
                                 		fprintf(cgiOut,"<td><img src=/images/new5000/5624_13.jpg width=9 height=9 ></td>"); //红色
                        		        fprintf(cgiOut,"<td><img src=/images/new5000/5624_64.jpg width=5 height=9 ></td>");
                                 		fprintf(cgiOut,"<td><img src=/images/new5000/5624_13.jpg width=9 height=9 ></td>"); //红色
                        		        fprintf(cgiOut,"<td rowspan=6><img src=/images/new5000/5624_66.jpg width=20 height=102 ></td>");
                        		        fprintf(cgiOut,"<td><img src=/images/new5000/sep.gif width=1 height=9 ></td>");
										 	}
		       	 				}
		       	 		    }
		       	 		}

					/*----------------------------  end  ----------------------------------------*/

		       	}
			}
	  //////////////////////////// 21 等结束 /////////////////////////////////////////////
	fprintf(cgiOut,"</tr>");
 ///////////////////////分开截图 端口//////////////////////////

         fprintf(cgiOut,"<tr>");

 retu=checkuser_group(m);
 if(retu==0)  /*管理员*/
 	{
		fprintf(cgiOut,"<td colspan=23 rowspan=3 bgcolor=#F5F5F5>");
		//fprintf(cgiOut,"<img src=/images/new5000/5624_67.jpg width=187 height=47 >");
		fprintf(cgiOut,"</td>");		

		fprintf(cgiOut,"<td colspan=23 rowspan=3>");
		//fprintf(cgiOut,"<img src=/images/new5000/5624_68.jpg width=188 height=47 >");

		
fprintf(cgiOut,"<table id=__01 width=188 height=47 border=0 cellpadding=0 cellspacing=0>"\
	"<tr>"\
		"<td colspan=13>"\
			"<img src=/images/new5000/5624_68_01.jpg width=188 height=3 ></td>"\
	"</tr>"\
	"<tr>");
		fprintf(cgiOut,"<td colspan=3 rowspan=2 bgcolor=#F4F4F4><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame>"\
			"<img src=/images/new5000/5624_68_02.jpg width=34 height=22 border=0 alt=\"%s\">",n,"1-1","1");
			fprintf(cgiOut,"</td>");
			fprintf(cgiOut,"<td><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame>"\
			"<img src=/images/new5000/5624_68_03.jpg width=23 height=19 border=0 alt=\"%s\"></td>",n,"1-3","3");
		
		fprintf(cgiOut,"<td rowspan=4>"\
			"<img src=/images/new5000/5624_68_04.jpg width=10 height=44 ></td>");
		fprintf(cgiOut,"<td><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame>"\
			"<img src=/images/new5000/5624_68_03.jpg width=23 height=19 border=0 alt=\"%s\"></td>",n,"1-5","5");
		
		fprintf(cgiOut,"<td rowspan=4>"\
			"<img src=/images/new5000/5624_68_06.jpg width=10 height=44 ></td>");
		fprintf(cgiOut,"<td><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame>"\
			"<img src=/images/new5000/5624_68_03.jpg width=23 height=19 border=0 alt=\"%s\"></td>",n,"1-7","7");
		
		fprintf(cgiOut,"<td rowspan=4>"\
			"<img src=/images/new5000/5624_68_08.jpg width=10 height=44 ></td>");
		fprintf(cgiOut,"<td><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame>"\
			"<img src=/images/new5000/5624_68_03.jpg width=23 height=19 border=0 alt=\"%s\"></td>",n,"1-9","9");
		
		fprintf(cgiOut,"<td rowspan=4>"\
			"<img src=/images/new5000/5624_68_10.jpg width=8 height=44 ></td>");
		fprintf(cgiOut,"<td><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame>"\
			"<img src=/images/new5000/5624_68_03.jpg width=23 height=19 border=0 alt=\"%s\"></td>",n,"1-11","11");
		
		fprintf(cgiOut,"<td rowspan=4>"\
			"<img src=/images/new5000/5624_68_12.jpg width=1 height=44 ></td>"\
	"</tr>"\
	"<tr>"\
		"<td>"\
			"<img src=/images/new5000/5624_67_14.jpg width=23 height=3 ></td>"\
		"<td>"\
			"<img src=/images/new5000/5624_67_14.jpg width=23 height=3 ></td>"\
		"<td>"\
			"<img src=/images/new5000/5624_67_14.jpg width=23 height=3 ></td>"\
		"<td>"\
			"<img src=/images/new5000/5624_67_14.jpg width=23 height=3 ></td>"\
		"<td>"\
			"<img src=/images/new5000/5624_67_14.jpg width=23 height=3 ></td>"\
	"</tr>"\
	"<tr>"\
		"<td rowspan=2>"\
			"<img src=/images/new5000/5624_68_18.jpg width=1 height=22 ></td>");
		fprintf(cgiOut,"<td><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame>"\
			"<img src=/images/new5000/5624_67_19.jpg width=23 height=19 border=0 alt=\"%s\"></td>",n,"1-2","2");
		
		fprintf(cgiOut,"<td rowspan=2>"\
			"<img src=/images/new5000/5624_68_20.jpg width=10 height=22 ></td>");
		fprintf(cgiOut,"<td><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame>"\
			"<img src=/images/new5000/5624_67_19.jpg width=23 height=19 border=0 alt=\"%s\"></td>",n,"1-4","4");
		
		fprintf(cgiOut,"<td><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame>"\
			"<img src=/images/new5000/5624_67_19.jpg width=23 height=19 border=0 alt=\"%s\"></td>",n,"1-6","6");
		
		fprintf(cgiOut,"<td><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame>"\
			"<img src=/images/new5000/5624_67_19.jpg width=23 height=19 border=0 alt=\"%s\"></td>",n,"1-8","8");
		
		fprintf(cgiOut,"<td><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame>"\
			"<img src=/images/new5000/5624_67_19.jpg width=23 height=19 border=0 alt=\"%s\"></td>",n,"1-10","10");
		
		fprintf(cgiOut,"<td><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame>"\
			"<img src=/images/new5000/5624_67_19.jpg width=23 height=19 border=0 alt=\"%s\"></td>",n,"1-12","12");
		
	fprintf(cgiOut,"</tr>"\
	"<tr>"\
		"<td>"\
			"<img src=/images/new5000/5624_68_26.jpg width=23 height=3 ></td>"\
		"<td>"\
			"<img src=/images/new5000/5624_67_14.jpg width=23 height=3 ></td>"\
		"<td>"\
			"<img src=/images/new5000/5624_67_14.jpg width=23 height=3 ></td>"\
		"<td>"\
			"<img src=/images/new5000/5624_67_14.jpg width=23 height=3 ></td>"\
		"<td>"\
			"<img src=/images/new5000/5624_67_14.jpg width=23 height=3 ></td>"\
		"<td>"\
			"<img src=/images/new5000/5624_67_14.jpg width=23 height=3 ></td>"\
	"</tr>"\
"</table>");

		
		fprintf(cgiOut,"</td>");

 	}
 else
  {
		fprintf(cgiOut,"<td colspan=23 rowspan=3 bgcolor=#F5F5F5>");
		//fprintf(cgiOut,"<img src=/images/new5000/5624_67.jpg width=187 height=47 >");	
		fprintf(cgiOut,"</td>");
		
		fprintf(cgiOut,"<td colspan=23 rowspan=3>");
		//fprintf(cgiOut,"<img src=/images/new5000/5624_68.jpg width=188 height=47 >");

		
fprintf(cgiOut,"<table id=__01 width=188 height=47 border=0 cellpadding=0 cellspacing=0>"\
	"<tr>"\
		"<td colspan=13>"\
			"<img src=/images/new5000/5624_68_01.jpg width=188 height=3 ></td>"\
	"</tr>"\
	"<tr>");
		fprintf(cgiOut,"<td colspan=3 rowspan=2 bgcolor=#F4F4F4>"\
			"<img src=/images/new5000/5624_68_02.jpg width=34 height=22 border=0 alt=\"%s\">","1");
			fprintf(cgiOut,"</td>");
			fprintf(cgiOut,"<td>"\
			"<img src=/images/new5000/5624_68_03.jpg width=23 height=19 border=0 alt=\"%s\"></td>","3");
		
		fprintf(cgiOut,"<td rowspan=4>"\
			"<img src=/images/new5000/5624_68_04.jpg width=10 height=44 ></td>");
		fprintf(cgiOut,"<td>"\
			"<img src=/images/new5000/5624_68_03.jpg width=23 height=19 border=0 alt=\"%s\"></td>","5");
		
		fprintf(cgiOut,"<td rowspan=4>"\
			"<img src=/images/new5000/5624_68_06.jpg width=10 height=44 ></td>");
		fprintf(cgiOut,"<td>"\
			"<img src=/images/new5000/5624_68_03.jpg width=23 height=19 border=0 alt=\"%s\"></td>","7");
		
		fprintf(cgiOut,"<td rowspan=4>"\
			"<img src=/images/new5000/5624_68_08.jpg width=10 height=44 ></td>");
		fprintf(cgiOut,"<td>"\
			"<img src=/images/new5000/5624_68_03.jpg width=23 height=19 border=0 alt=\"%s\"></td>","9");
		
		fprintf(cgiOut,"<td rowspan=4>"\
			"<img src=/images/new5000/5624_68_10.jpg width=8 height=44 ></td>");
		fprintf(cgiOut,"<td>"\
			"<img src=/images/new5000/5624_68_03.jpg width=23 height=19 border=0 alt=\"%s\"></td>","11");
		
		fprintf(cgiOut,"<td rowspan=4>"\
			"<img src=/images/new5000/5624_68_12.jpg width=1 height=44 ></td>"\
	"</tr>"\
	"<tr>"\
		"<td>"\
			"<img src=/images/new5000/5624_67_14.jpg width=23 height=3 ></td>"\
		"<td>"\
			"<img src=/images/new5000/5624_67_14.jpg width=23 height=3 ></td>"\
		"<td>"\
			"<img src=/images/new5000/5624_67_14.jpg width=23 height=3 ></td>"\
		"<td>"\
			"<img src=/images/new5000/5624_67_14.jpg width=23 height=3 ></td>"\
		"<td>"\
			"<img src=/images/new5000/5624_67_14.jpg width=23 height=3 ></td>"\
	"</tr>"\
	"<tr>"\
		"<td rowspan=2>"\
			"<img src=/images/new5000/5624_68_18.jpg width=1 height=22 ></td>");
		fprintf(cgiOut,"<td>"\
			"<img src=/images/new5000/5624_67_19.jpg width=23 height=19 border=0 alt=\"%s\"></td>","2");
		
		fprintf(cgiOut,"<td rowspan=2>"\
			"<img src=/images/new5000/5624_68_20.jpg width=10 height=22 ></td>");
		fprintf(cgiOut,"<td>"\
			"<img src=/images/new5000/5624_67_19.jpg width=23 height=19 border=0 alt=\"%s\"></td>","4");
		
		fprintf(cgiOut,"<td>"\
			"<img src=/images/new5000/5624_67_19.jpg width=23 height=19 border=0 alt=\"%s\"></td>","6");
		
		fprintf(cgiOut,"<td>"\
			"<img src=/images/new5000/5624_67_19.jpg width=23 height=19 border=0 alt=\"%s\"></td>","8");
		
		fprintf(cgiOut,"<td>"\
			"<img src=/images/new5000/5624_67_19.jpg width=23 height=19 border=0 alt=\"%s\"></td>","10");
		
		fprintf(cgiOut,"<td>"\
			"<img src=/images/new5000/5624_67_19.jpg width=23 height=19 border=0 alt=\"%s\"></td>","12");
		
	fprintf(cgiOut,"</tr>"\
	"<tr>"\
		"<td>"\
			"<img src=/images/new5000/5624_68_26.jpg width=23 height=3 ></td>"\
		"<td>"\
			"<img src=/images/new5000/5624_67_14.jpg width=23 height=3 ></td>"\
		"<td>"\
			"<img src=/images/new5000/5624_67_14.jpg width=23 height=3 ></td>"\
		"<td>"\
			"<img src=/images/new5000/5624_67_14.jpg width=23 height=3 ></td>"\
		"<td>"\
			"<img src=/images/new5000/5624_67_14.jpg width=23 height=3 ></td>"\
		"<td>"\
			"<img src=/images/new5000/5624_67_14.jpg width=23 height=3 ></td>"\
	"</tr>"\
"</table>");

		
		fprintf(cgiOut,"</td>");

 	}

			
////////////////// end 端口////////////////////////////////////
			
		fprintf(cgiOut,"<td colspan=3 rowspan=3>");
			fprintf(cgiOut,"<img src=/images/new5000/new_5624_69.jpg width=22 height=47 >");
			fprintf(cgiOut,"</td>"\
		"<td colspan=3 rowspan=3>"\
			"<img src=/images/new5000/new_5624_70.jpg width=23 height=47 ></td>"\
		"<td>"\
			"<img src=/images/new5000/sep.gif width=1 height=4 ></td>"\
	"</tr>"\
	"<tr>");
			
		fprintf(cgiOut,"<td align=center colspan=5 bgcolor=#C7C7C7>");
			//fprintf(cgiOut,"<img src=/images/new5000/5624_71.jpg width=117 height=19 >");
			fprintf(cgiOut,"</td>"\
		"<td>"\
			"<img src=/images/new5000/sep.gif width=1 height=19 ></td>"\
	"</tr>"\
	"<tr>"\
		"<td colspan=5>"\
			"<img src=/images/new5000/5624_72.jpg width=117 height=24 ></td>"\
		"<td>"\
			"<img src=/images/new5000/sep.gif width=1 height=24 ></td>"\
	"</tr>"\
	"<tr>");

     //////  2 4 6 8 10 指示灯 左边端口不显示////////////////
     light_flag_new(1,6,pt,2);
     memset(cstring,0,128);
	 strcpy(cstring,"");		
     light_spe_new(12,pt,cstring);	 

     //右边端口
     light_flag(1,4,pt,2);
	 memset(cstring,0,128);
	 strcpy(cstring,"<td><img src=/images/new5000/5624_111.jpg width=12 height=9 ></td>");		
     light_spe(8,pt,cstring);	 

    ///////// 20   光电互斥口//////////////////////
  
       for(i=1;i<3;i++)
	   {
			value=1;
			value=(value << 8) |(2*i+8);
			if(show_eth_port_atrr(value,0,&pt)==CCGI_SUCCESS)
			{
				if(strcmp(eth_port_type_str[pt.port_type],"ETH_GE_SFP")==0)
				fprintf(cgiOut,"<td><img src=/images/new5000/5624_05.jpg width=9 height=9 ></td>"); //黑色
				else  /*type为电口*/
				{
					if(strcmp(onoff_status_str[(pt.attr_bitmap & ETH_ATTR_ADMIN_STATUS) >> ETH_ADMIN_STATUS_BIT],"OFF")==0)
					fprintf(cgiOut,"<td><img src=/images/new5000/5624_05.jpg width=9 height=9 ></td>"); //黑色
					else
					{
						if(strcmp(link_status_str[(pt.attr_bitmap & ETH_ATTR_LINK_STATUS) >> ETH_LINK_STATUS_BIT],"UP")==0 && 0==strcmp(eth_speed_str[(pt.attr_bitmap & ETH_ATTR_SPEED_MASK) >> ETH_SPEED_BIT],"1000M"))
						fprintf(cgiOut,"<td><img src=/images/new5000/5624_11.jpg width=9 height=9 ></td>"); //绿色
						else
						fprintf(cgiOut,"<td><img src=/images/new5000/5624_13.jpg width=9 height=9 ></td>"); //红色
					}
				}
			}
			else
			fprintf(cgiOut,"<td><img src=/images/new5000/5624_05.jpg width=9 height=9 ></td>"); //黑色

			fprintf(cgiOut,"<td><img src=/images/new5000/5624_12.jpg width=4 height=9 ""></td>");

			///////////////////////////////////////////////////light2///////////////////////////////////////				
			if(show_eth_port_atrr(value,0,&pt)==CCGI_SUCCESS)
			{
				if(strcmp(eth_port_type_str[pt.port_type],"ETH_GE_SFP")==0)
				fprintf(cgiOut,"<td><img src=/images/new5000/5624_05.jpg width=9 height=9 ></td>"); //黑色
				else  /*type为电口*/
				{
					if(strcmp(onoff_status_str[(pt.attr_bitmap & ETH_ATTR_ADMIN_STATUS) >> ETH_ADMIN_STATUS_BIT],"OFF")==0)
					fprintf(cgiOut,"<td><img src=/images/new5000/5624_05.jpg width=9 height=9 ></td>"); //黑色
					else
					{
						if(strcmp(link_status_str[(pt.attr_bitmap & ETH_ATTR_LINK_STATUS) >> ETH_LINK_STATUS_BIT],"UP")==0)
						fprintf(cgiOut,"<td><img src=/images/new5000/5624_11.jpg width=9 height=9 ></td>"); //绿色
						else
						fprintf(cgiOut,"<td><img src=/images/new5000/5624_13.jpg width=9 height=9 ></td>"); //红色
					}
				}
			}
			else
			fprintf(cgiOut,"<td><img src=/images/new5000/5624_05.jpg width=9 height=9 ></td>"); //黑色

			switch(i)
			{
				case 1: fprintf(cgiOut,"<td><img src=/images/new5000/5624_14.jpg width=11 height=9 ></td>");break;
				case 2: break;
			}


		}

	///////////////////////  光口指示灯   //////////////////////////////

	  for(i=1;i<3;i++)
	  {
		value=1;
		value=(value << 8) |(2*i+8);
		if(show_eth_port_atrr(value,0,&pt)==CCGI_SUCCESS)
		{
			if(strcmp(eth_port_type_str[pt.port_type],"ETH_GE_SFP")!=0)
			{
				//两个黑灯 		
				if(i==1)
				{
					fprintf(cgiOut,"<td><img src=/images/new5000/5624_05.jpg width=9 height=9 ></td>"); //黑色
					fprintf(cgiOut,"<td><img src=/images/new5000/5624_12.jpg width=4 height=9 ></td>");
					fprintf(cgiOut,"<td><img src=/images/new5000/5624_05.jpg width=9 height=9 ></td>"); //黑色
				}
				if(i==2)
				{
					fprintf(cgiOut,"<td><img src=/images/new5000/5624_05.jpg width=9 height=9 ></td>"); //黑色
					fprintf(cgiOut,"<td><img src=/images/new5000/5624_64.jpg width=5 height=9 ></td>");
					fprintf(cgiOut,"<td><img src=/images/new5000/5624_05.jpg width=9 height=9 ></td>"); //黑色
					fprintf(cgiOut,"<td><img src=/images/new5000/5624_125.jpg width=68 height=9 ></td>");

				}
			}

			/*--------------------------  ----------------------------------------*/
			else
			{
				if(strcmp(onoff_status_str[(pt.attr_bitmap & ETH_ATTR_ADMIN_STATUS) >> ETH_ADMIN_STATUS_BIT],"OFF")==0)
				{
					if(i==1)
					{
						fprintf(cgiOut,"<td><img src=/images/new5000/5624_05.jpg width=9 height=9 ></td>"); //黑色
						fprintf(cgiOut,"<td><img src=/images/new5000/5624_12.jpg width=4 height=9 ></td>");
						fprintf(cgiOut,"<td><img src=/images/new5000/5624_05.jpg width=9 height=9 ></td>"); //黑色
					}
					if(i==2)
					{
						fprintf(cgiOut,"<td><img src=/images/new5000/5624_05.jpg width=9 height=9 ></td>"); //黑色
						fprintf(cgiOut,"<td><img src=/images/new5000/5624_64.jpg width=5 height=9 ></td>");
						fprintf(cgiOut,"<td><img src=/images/new5000/5624_05.jpg width=9 height=9 ></td>"); //黑色
						fprintf(cgiOut,"<td><img src=/images/new5000/5624_125.jpg width=68 height=9 ></td>");

					}
				}
				else
				{
					if(strcmp(link_status_str[(pt.attr_bitmap & ETH_ATTR_LINK_STATUS) >> ETH_LINK_STATUS_BIT],"UP")==0)
					{
						//全绿色
						if(i==1)
						{
							fprintf(cgiOut,"<td><img src=/images/new5000/5624_11.jpg width=9 height=9 ></td>"); //绿色
							fprintf(cgiOut,"<td><img src=/images/new5000/5624_12.jpg width=4 height=9 ></td>");
							fprintf(cgiOut,"<td><img src=/images/new5000/5624_11.jpg width=9 height=9 ></td>"); //绿色
						}
						if(i==2)
						{
							fprintf(cgiOut,"<td><img src=/images/new5000/5624_11.jpg width=9 height=9 ></td>"); //绿色
							fprintf(cgiOut,"<td><img src=/images/new5000/5624_64.jpg width=5 height=9 ></td>");
							fprintf(cgiOut,"<td><img src=/images/new5000/5624_11.jpg width=9 height=9 ></td>"); //绿色
							fprintf(cgiOut,"<td><img src=/images/new5000/5624_125.jpg width=68 height=9 ></td>");

						}
					}
					else
					{
						//全红色
						if(i==1)
						{
							fprintf(cgiOut,"<td><img src=/images/new5000/5624_13.jpg width=9 height=9 ></td>"); //红色
							fprintf(cgiOut,"<td><img src=/images/new5000/5624_12.jpg width=4 height=9 ></td>");
							fprintf(cgiOut,"<td><img src=/images/new5000/5624_13.jpg width=9 height=9 ></td>"); //红色
						}
						if(i==2)
						{
							fprintf(cgiOut,"<td><img src=/images/new5000/5624_13.jpg width=9 height=9 ></td>"); //红色
							fprintf(cgiOut,"<td><img src=/images/new5000/5624_64.jpg width=5 height=9 ></td>");
							fprintf(cgiOut,"<td><img src=/images/new5000/5624_13.jpg width=9 height=9 ></td>"); //红色
							fprintf(cgiOut,"<td><img src=/images/new5000/5624_125.jpg width=68 height=9 ></td>");

						}
					}
				}
			}

			/*----------------------------  end  ----------------------------------------*/

		}
	}
 
		////////////////  绿色运行 ，注意边框信息  ////////////////////
		fprintf(cgiOut,"<td bgcolor=#C7C7C7>"\
			"<img src=/images/new5000/5624_126.jpg width=9 height=9 ></td>"\
		"<td bgcolor=#C7C7C7>"\
			"<img src=/images/new5000/5624_127.jpg width=19 height=9 ></td>"\
		"<td bgcolor=#C7C7C7>"\
			"<img src=/images/new5000/5624_126.jpg width=9 height=9 ></td>"\
		"<td bgcolor=#C7C7C7>"\
			"<img src=/images/new5000/5624_129.jpg width=12 height=9 ></td>");
		
		fprintf(cgiOut,"<td>");
		fprintf(cgiOut,"<img src=/images/new5000/sep.gif width=1 height=9 >");
	    fprintf(cgiOut,"</td>");
		
	fprintf(cgiOut,"</tr>"\
	"<tr>"\
		"<td colspan=55>"\
			"<img src=/images/new5000/5624_130.jpg width=514 height=37 ></td>"\
		"<td colspan=5>"\
			"<img src=/images/new5000/5624_131.jpg width=117 height=37 ></td>"\
		"<td>"\
			"<img src=/images/new5000/sep.gif width=1 height=37 ></td>"\
	"</tr>"\
"</table>");



return 0;


 /////////////////////// end panel 5000  ////////////////////////////////////////////////////


}



/*5000的面板函数，根据产品id来判断*/
int panel_5k_3052(struct sys_ver ptrsysver,unsigned int value,struct eth_port_s pt,int retu,struct list *lcon,char *n,char *m)
{

  char cstring[128];  //字符串
  memset(cstring,0,128);
  int i;
  char id[10];
  memset(id,0,10);

fprintf(cgiOut,"<table width=719 height=127 border=0 cellpadding=0 cellspacing=0 background=/images/3052/3002.jpg>"\
  "<tr>"\
    "<td align=center><table width=719 height=116 border=0 cellpadding=0 cellspacing=0>"\
      "<tr>"\
        "<td width=85%% align=left valign=top><table width=610 border=0 align=center cellpadding=0 cellspacing=0>"\
          "<tr>"\
            "<td height=6 colspan=8><table width=200 height=24 border=0 cellpadding=0 cellspacing=0>"\
              "<tr>"\
                "<td width=42>&nbsp;</td>"\
                "<td width=43><div align=center style=font-weight:bold><span class=STYLE4>SPEED</span></div></td>"\
                "<td width=13 align=center><img src=/images/3052/h.jpg width=9 height=9 /></td>"\
                "<td width=12 align=center><img src=/images/3052/h.jpg width=9 height=9 /></td>"\
                "<td width=38 align=center><strong class=STYLE4>LINK/ACT</strong></td>"\
                "<td width=29>&nbsp;</td>"\
                "<td width=23>&nbsp;</td>"\
              "</tr>"\
            "</table></td>"\
          "</tr>"\
          "<tr>"\
            "<td height=4 ></td>"\
          "</tr>");

//上面部分不用动


          fprintf(cgiOut,"<tr>"\
            "<td width=3%% height=60>&nbsp;</td>"\
            "<td width=29%% align=center><table width=166 border=0 cellpadding=0 cellspacing=0>"\
              "<tr>"\
                "<td height=9><table id=__01 width=166 height=9 border=0 cellpadding=0 cellspacing=0>");

// 1 区灯
                    fprintf(cgiOut,"<tr>");

                
				 light_flag_3052(1,8,pt,1);				 
        		 memset(cstring,0,128);
        		 strcpy(cstring,"");		
        		 light_spe_3052(15,pt,cstring);
                   
/// 1 区灯结束，16 个灯，八个口的 1，3，5，7，9，11，13，15(特殊)

                fprintf(cgiOut,"</tr>");
                fprintf(cgiOut,"</table></td>"\
              "</tr>"\
              "<tr>"\
                "<td height=4></td>"\
              "</tr>"\
              "<tr>"\
                "<td height=15><table id=__01 width=166 height=15 border=0 cellpadding=0 cellspacing=0>"\
                    "<tr>");
//1 区奇数端口		

 retu=checkuser_group(m);
 if(retu==0)
 	{
                      fprintf(cgiOut,"<td width=1></td>");

					  for(i=0;i<7;i++)
					  	{
					  memset(id,0,10);
					  sprintf(id,"1-%d",2*i+1);
					  fprintf(cgiOut,"<td><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame>"\
                      "<img src=/images/3052/c_02.jpg width=17 height=15 border=0 alt=\"%d\" ></td>"\
                      "<td width=4></td>",n,id,2*i+1);
					  	}					  
                      fprintf(cgiOut,"<td><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame>"\
					  	"<img src=/images/3052/c_02.jpg width=17 height=15 border=0  alt=\"%s\" ></td>"\
                      "<td width=1></td>",n,"1-15","15");
 	}
 else{
                      fprintf(cgiOut,"<td width=1></td>");
					  for(i=0;i<7;i++)
					  	{
                      fprintf(cgiOut,"<td>"\
                      "<img src=/images/3052/c_02.jpg width=17 height=15 border=0  alt=\"%d\" ></td>"\
                      "<td width=4></td>",2*i+1);
					  	}		 
					  
                      fprintf(cgiOut,"<td><img src=/images/3052/c_02.jpg width=17 height=15 border=0  alt=\"%s\" ></td>"\
                      "<td width=1></td>","15");
 	}
// 1 区奇数端口结束 1,3,5,7,9,11,13,15

                    fprintf(cgiOut,"</tr>"\
                "</table></td>"\
              "</tr>"\
              "<tr>"\
                "<td height=4></td>"\
              "</tr>"\
              "<tr>"\
                "<td height=15><table id=__01 width=166 height=15 border=0 cellpadding=0 cellspacing=0>"\
                    "<tr>");
// 一区偶数端口
retu=checkuser_group(m);
 if(retu==0)
 	{
                      fprintf(cgiOut,"<td width=1></td>");

					  for(i=0;i<7;i++)
					  	{
					  memset(id,0,10);
					  sprintf(id,"1-%d",2*(i+1));
                      fprintf(cgiOut,"<td><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame>"\
                      "<img src=/images/3052/c_03.jpg width=17 height=15 border=0  alt=\"%d\"></td>"\
                      "<td width=4></td>",n,id,2*(i+1));
					  	}					  
                      fprintf(cgiOut,"<td><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame>"\
					  	"<img src=/images/3052/c_03.jpg width=17 height=15 border=0  alt=\"%s\" ></td>"\
                      "<td width=1></td>",n,"1-16","16");
 	}
 else
 	{
                      fprintf(cgiOut,"<td width=1></td>");

					  for(i=0;i<7;i++)
					  	{
                      fprintf(cgiOut,"<td><img src=/images/3052/c_03.jpg width=17 height=15 border=0  alt=\"%d\"></td>"\
                      "<td width=4></td>",2*(i+1));
					  	}
					  
                      fprintf(cgiOut,"<td><img src=/images/3052/c_03.jpg width=17 height=15 border=0  alt=\"%s\" ></td>"\
                      "<td width=1></td>","16");
 	}

//1区偶数端口结束  2,4,6,8,10,12,14,16
                    fprintf(cgiOut,"</tr>"\
                "</table></td>"\
              "</tr>"\
              "<tr>"\
                "<td height=4></td>"\
              "</tr>"\
              "<tr>"\
                "<td height=9><table id=__01 width=166 height=9 border=0 cellpadding=0 cellspacing=0>"\
                    "<tr>");
//1区偶数灯

                 light_flag_3052(1,8,pt,2);				 
        		 memset(cstring,0,128);
        		 strcpy(cstring,"");		
        		 light_spe_3052(16,pt,cstring);
				
//1区偶数灯结束					  
                    fprintf(cgiOut,"</tr>"\
                "</table></td>"\
              "</tr>"\
            "</table></td>"\
            "<td width=1%%>&nbsp;</td>"\
            "<td width=27%% align=center><table width=166 border=0 cellpadding=0 cellspacing=0>"\
              "<tr>"\
                "<td height=9><table id=__01 width=166 height=9 border=0 cellpadding=0 cellspacing=0>"\
                    "<tr>");
//2区奇数灯

                 light_flag_3052(9,16,pt,1);				 
        		 memset(cstring,0,128);
        		 strcpy(cstring,"");		
        		 light_spe_3052(31,pt,cstring);
 
//2区奇数灯结束
                    fprintf(cgiOut,"</tr>"\
                "</table></td>"\
              "</tr>"\
              "<tr>"\
                "<td height=4></td>"\
              "</tr>"\
              "<tr>"\
                "<td height=15><table id=__01 width=166 height=15 border=0 cellpadding=0 cellspacing=0>"\
                    "<tr>");
                 
//2区奇数端口 17，19，21，23，25，27，29，31
                      
                 fprintf(cgiOut,"<td width=1></td>");
    retu=checkuser_group(m);
    if(retu==0)
    {
                 for(i=1;i<8;i++){
				 memset(id,0,10);
				 sprintf(id,"1-%d",2*i+15);
                 fprintf(cgiOut,"<td><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame>"\
					  	"<img src=/images/3052/c_02.jpg width=17 height=15 border=0  alt=\"%d\" /></td>"\
                      "<td width=4></td>",n,id,2*i+15);  
                 }
                      
                 fprintf(cgiOut,"<td><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame>"\
				 	"<img src=/images/3052/c_02.jpg width=17 height=15 border=0  alt=\"%s\" /></td>"\
                      "<td width=1></td>",n,"1-31","31");
     }
	else
		{
                 for(i=1;i<8;i++){
				 memset(id,0,10);
				 sprintf(id,"1-%d",2*i+15);
                 fprintf(cgiOut,"<td>"\
					  	"<img src=/images/3052/c_02.jpg width=17 height=15 border=0  alt=\"%d\" /></td>"\
                      "<td width=4></td>",2*i+15);  
                 }
                      
                 fprintf(cgiOut,"<td><img src=/images/3052/c_02.jpg width=17 height=15 border=0  alt=\"%s\" /></td>"\
                      "<td width=1></td>","31");
       }

                    fprintf(cgiOut,"</tr>"\
                "</table></td>"\
              "</tr>"\
              "<tr>"\
                "<td height=4></td>"\
              "</tr>"\
              "<tr>"\
                "<td height=15><table id=__01 width=166 height=15 border=0 cellpadding=0 cellspacing=0>"\
                    "<tr>");
//2区偶数端口  18，20，22，24，26，28，30，32
                   fprintf(cgiOut,"<td width=1></td>");
    retu=checkuser_group(m);
    if(retu==0)
    	{
                      for(i=1;i<8;i++)
                      	{
                      memset(id,0,10);
					  sprintf(id,"1-%d",2*i+16);
                      fprintf(cgiOut,"<td><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame>"\
					  	"<img src=/images/3052/c_03.jpg width=17 height=15 border=0  alt=\"%d\" /></td>"\
                      "<td width=4></td>",n,id,2*i+16);
                      	}
                      fprintf(cgiOut,"<td><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame>"
					  	"<img src=/images/3052/c_03.jpg width=17 height=15 border=0  alt=\"%s\" /></td>"\
                      "<td width=1></td>",n,"1-32","32");
    	}
	   else
	   	{
                      for(i=1;i<8;i++)
                      	{                     
                      fprintf(cgiOut,"<td>"\
					  	"<img src=/images/3052/c_03.jpg width=17 height=15 border=0  alt=\"%d\" /></td>"\
                      "<td width=4></td>",2*i+16);
                      	}
                      fprintf(cgiOut,"<td>"
					  	"<img src=/images/3052/c_03.jpg width=17 height=15 border=0  alt=\"%s\" /></td>"\
                      "<td width=1></td>","32");
    	}
//2区偶数端口结束
                   fprintf(cgiOut,"</tr>"\
                "</table></td>"\
              "</tr>"\
              "<tr>"\
                "<td height=4></td>"\
              "</tr>"\
              "<tr>"\
                "<td height=9><table id=__01 width=166 height=9 border=0 cellpadding=0 cellspacing=0>"\
                    "<tr>");
//2区偶数灯 

                 light_flag_3052(9,16,pt,2);				 
        		 memset(cstring,0,128);
        		 strcpy(cstring,"");		
        		 light_spe_3052(32,pt,cstring);                      
//2区偶数灯结束
                 fprintf(cgiOut,"</tr>"\
                "</table></td>"\
              "</tr>"\
            "</table></td>"\
            "<td width=1%%>&nbsp;</td>"\
            "<td width=28%% align=center><table width=166 border=0 cellpadding=0 cellspacing=0>"\
              "<tr>"\
                "<td height=9><table id=__01 width=166 height=9 border=0 cellpadding=0 cellspacing=0>"\
                    "<tr>");
//3区奇数灯

                 light_flag_3052(17,24,pt,1);				 
        		 memset(cstring,0,128);
        		 strcpy(cstring,"");		
        		 light_spe_3052(47,pt,cstring);  				 
                 
//3区奇数灯结束
                   fprintf(cgiOut,"</tr>"\
                "</table></td>"\
              "</tr>"\
              "<tr>"\
                "<td height=4></td>"\
              "</tr>"\
              "<tr>"\
                "<td height=15><table id=__01 width=166 height=15 border=0 cellpadding=0 cellspacing=0>"\
                    "<tr>");
//3区奇数端口
          fprintf(cgiOut,"<td width=1></td>");
          retu=checkuser_group(m);
		  if(retu==0)
		  	{
                      for(i=1;i<8;i++)
                      {
                      memset(id,0,10);
					  sprintf(id,"1-%d",2*i+31);
                      fprintf(cgiOut,"<td><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame>"
					  	"<img src=/images/3052/c_02.jpg width=17 height=15 border=0  alt=\"%d\" /></td>"\
                      "<td width=4></td>",n,id,2*i+31);    
                      }
                      fprintf(cgiOut,"<td><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame>"
					  	"<img src=/images/3052/c_02.jpg width=17 height=15 border=0  alt=\"%s\" /></td>"\
                      "<td width=1></td>",n,"1-47","47");
		  	}
		    else
				{
                      for(i=1;i<8;i++)
                      {                      
                      fprintf(cgiOut,"<td>"
					  	"<img src=/images/3052/c_02.jpg width=17 height=15 border=0  alt=\"%d\" /></td>"\
                      "<td width=4></td>",2*i+31);    
                      }
                      fprintf(cgiOut,"<td>"
					  	"<img src=/images/3052/c_02.jpg width=17 height=15 border=0  alt=\"%s\" /></td>"\
                      "<td width=1></td>","47");
		  	    }
                      
                    fprintf(cgiOut,"</tr>"\
                "</table></td>"\
              "</tr>"\
              "<tr>"\
                "<td height=4></td>"\
              "</tr>"\
              "<tr>"\
                "<td height=15><table id=__01 width=166 height=15 border=0 cellpadding=0 cellspacing=0>"\
                    "<tr>");
//3区偶数端口

           fprintf(cgiOut,"<td width=1></td>");
           retu=checkuser_group(m);
		    if(retu==0)
		  	{
                      for(i=1;i<8;i++)
                      {
	                      memset(id,0,10);
						  sprintf(id,"1-%d",2*i+32);
	                      fprintf(cgiOut,"<td><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame>"
						  	"<img src=/images/3052/c_03.jpg width=17 height=15 border=0  alt=\"%d\" /></td>"\
	                      "<td width=4></td>",n,id,2*i+32);    
                      }
                      fprintf(cgiOut,"<td><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame>"
					  	"<img src=/images/3052/c_03.jpg width=17 height=15 border=0  alt=\"%s\" /></td>"\
                      "<td width=1></td>",n,"1-48","48");
		  	}
		    else
			{
                      for(i=1;i<8;i++)
                      {                      
	                      fprintf(cgiOut,"<td>"
						  	"<img src=/images/3052/c_03.jpg width=17 height=15 border=0  alt=\"%d\" /></td>"\
	                      "<td width=4></td>",2*i+32);    
                      }
                      fprintf(cgiOut,"<td>"
					  	"<img src=/images/3052/c_03.jpg width=17 height=15 border=0  alt=\"%s\" /></td>"\
                      "<td width=1></td>","48");
	  	    }
                      
 //3区偶数端口结束
                    fprintf(cgiOut,"</tr>"\
                "</table></td>"\
              "</tr>"\
              "<tr>"\
                "<td height=4></td>"\
              "</tr>"\
              "<tr>"\
                "<td height=9><table id=__01 width=166 height=9 border=0 cellpadding=0 cellspacing=0>"\
                    "<tr>");
 //3区偶数灯

                 light_flag_3052(17,24,pt,2);				 
        		 memset(cstring,0,128);
        		 strcpy(cstring,"");		
        		 light_spe_3052(48,pt,cstring); 
//3区偶数灯
                    fprintf(cgiOut,"</tr>"\
                "</table></td>"\
              "</tr>"\
            "</table></td>"\
            "<td width=1%%>&nbsp;</td>"\
            "<td width=10%%><table width=47 border=0 align=center cellpadding=0 cellspacing=0>"\
                "<tr>"\
                  "<td><table id=__01 width=40 height=9 border=0 cellpadding=0 cellspacing=0>"\
                      "<tr>");
//49光电互斥口   两个口当成一个口来用

        value=1;
        value=(value << 8) |(49);
        if(show_eth_port_atrr(value,0,&pt)==CCGI_SUCCESS)
		{
		    //光口的时候
			if(strcmp(eth_port_type_str[pt.port_type],"ETH_GE_SFP")==0)
			{
				if(strcmp(onoff_status_str[(pt.attr_bitmap & ETH_ATTR_ADMIN_STATUS) >> ETH_ADMIN_STATUS_BIT],"OFF")==0)
				{
					fprintf(cgiOut,"<td><img src=/images/3052/h.jpg width=9 height=9  /></td>"); //黑色
				}
				else
				{
					if(strcmp(link_status_str[(pt.attr_bitmap & ETH_ATTR_LINK_STATUS) >> ETH_LINK_STATUS_BIT],"UP")==0)
					{
						//全绿色
						fprintf(cgiOut,"<td><img src=/images/3052/t_02.jpg width=9 height=9  /></td>"); //黑色
					}
					else
					{
						//全红色
						fprintf(cgiOut,"<td><img src=/images/3052/t_01.jpg width=9 height=9  /></td>"); //黑色
					}
				}
			}
			else  /*type为电口*/
			{
				if(strcmp(onoff_status_str[(pt.attr_bitmap & ETH_ATTR_ADMIN_STATUS) >> ETH_ADMIN_STATUS_BIT],"OFF")==0)
				fprintf(cgiOut,"<td><img src=/images/3052/h.jpg width=9 height=9  /></td>"); //黑色
				else
				{
					if(strcmp(link_status_str[(pt.attr_bitmap & ETH_ATTR_LINK_STATUS) >> ETH_LINK_STATUS_BIT],"UP")==0 && 0==strcmp(eth_speed_str[(pt.attr_bitmap & ETH_ATTR_SPEED_MASK) >> ETH_SPEED_BIT],"1000M"))
					fprintf(cgiOut,"<td><img src=/images/3052/t_02.jpg width=9 height=9 ></td>"); //绿色
					else
					fprintf(cgiOut,"<td><img src=/images/3052/t_01.jpg width=9 height=9 ></td>"); //红色
				}
			}
		}
			  else
         		fprintf(cgiOut,"<td><img src=/images/3052/h.jpg width=9 height=9  /></td>"); //黑色
				fprintf(cgiOut,"<td width=1></td>");

        ///////////////////////////////////////////////////light2///////////////////////////////////////				
				if(show_eth_port_atrr(value,0,&pt)==CCGI_SUCCESS)
     			{
     				if(strcmp(eth_port_type_str[pt.port_type],"ETH_GE_SFP")==0)
     				{
						if(strcmp(onoff_status_str[(pt.attr_bitmap & ETH_ATTR_ADMIN_STATUS) >> ETH_ADMIN_STATUS_BIT],"OFF")==0)
						{
							fprintf(cgiOut,"<td><img src=/images/3052/h.jpg width=9 height=9  /></td>"); //黑色
						}
						else
						{
							if(strcmp(link_status_str[(pt.attr_bitmap & ETH_ATTR_LINK_STATUS) >> ETH_LINK_STATUS_BIT],"UP")==0)
							{
								//全绿色
								fprintf(cgiOut,"<td><img src=/images/3052/t_02.jpg width=9 height=9  /></td>"); //黑色
							}
							else
							{
								//全红色
								fprintf(cgiOut,"<td><img src=/images/3052/t_01.jpg width=9 height=9  /></td>"); //黑色
							}
						}
					}
					else  /*type为电口*/
     				{
     					if(strcmp(onoff_status_str[(pt.attr_bitmap & ETH_ATTR_ADMIN_STATUS) >> ETH_ADMIN_STATUS_BIT],"OFF")==0)
         		            fprintf(cgiOut,"<td><img src=/images/3052/h.jpg width=9 height=9  /></td>"); //黑色
     					else
     					{
     						if(strcmp(link_status_str[(pt.attr_bitmap & ETH_ATTR_LINK_STATUS) >> ETH_LINK_STATUS_BIT],"UP")==0)
	       			           fprintf(cgiOut,"<td><img src=/images/3052/t_02.jpg width=9 height=9 ></td>"); //绿色
     						else
			                  fprintf(cgiOut,"<td><img src=/images/3052/t_01.jpg width=9 height=9 ></td>"); //红色
     					}
     				}
     			}
     		  else
         		fprintf(cgiOut,"<td><img src=/images/3052/h.jpg width=9 height=9  /></td>"); //黑色
                fprintf(cgiOut,"<td width=2></td>");
                    
//51 光口
				value=1;
                value=(value << 8) |(51);
           		if(show_eth_port_atrr(value,0,&pt)==CCGI_SUCCESS)
				{
					if(strcmp(eth_port_type_str[pt.port_type],"ETH_GE_FIBER")!=0)
					{

						fprintf(cgiOut,"<td><img src=/images/3052/h.jpg width=9 height=9  /></td>"\
						"<td width=1></td>"\
						"<td><img src=/images/3052/h.jpg width=9 height=9  /></td>");
					}

					/*--------------------------  ----------------------------------------*/
					else
					{
						if(strcmp(onoff_status_str[(pt.attr_bitmap & ETH_ATTR_ADMIN_STATUS) >> ETH_ADMIN_STATUS_BIT],"OFF")==0)
						{
							fprintf(cgiOut,"<td><img src=/images/3052/h.jpg width=9 height=9  /></td>"\
							"<td width=1></td>"\
							"<td><img src=/images/3052/h.jpg width=9 height=9  /></td>");
						}
						else
						{
							if(strcmp(link_status_str[(pt.attr_bitmap & ETH_ATTR_LINK_STATUS) >> ETH_LINK_STATUS_BIT],"UP")==0)
							{
								//全绿色
								fprintf(cgiOut,"<td><img src=/images/3052/t_02.jpg width=9 height=9  /></td>"\
								"<td width=1></td>"\
								"<td><img src=/images/3052/t_02.jpg width=9 height=9  /></td>");


							}
							else
							{
								//全红色
								fprintf(cgiOut,"<td><img src=/images/3052/t_01.jpg width=9 height=9  /></td>"\
								"<td width=1></td>"\
								"<td><img src=/images/3052/t_01.jpg width=9 height=9  /></td>");

							}
						}
					}

					/*----------------------------  end  ----------------------------------------*/

				}
			
                     
                        
                      fprintf(cgiOut,"</tr>"\
                  "</table></td>"\
                "</tr>"\
                "<tr>"\
                  "<td height=4></td>"\
                "</tr>"\
                "<tr>"\
                  "<td><table id=__01 width=40 height=15 border=0 cellpadding=0 cellspacing=0>"\
                      "<tr>");

					    retu=checkuser_group(m);
						if(retu==0)
						{
						   fprintf(cgiOut,"<td width=1></td>");

						     //49
    				        value=1;
                            value=(value << 8) |(49);
                            if(show_eth_port_atrr(value,0,&pt)==CCGI_SUCCESS)
                        	{
                        	   	if(strcmp(eth_port_type_str[pt.port_type],"ETH_GE_SFP")==0)
                        	   	{
							        fprintf(cgiOut,"<td><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame>"
							        "<img src=/images/3052/c_04.jpg width=17 height=12 border=0 alt=\"%s\" /></td>",n,"1-49","49");

                                }
								else
								{
							       fprintf(cgiOut,"<td><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame>"
							       "<img src=/images/3052/c_02.jpg width=17 height=12 border=0 alt=\"%s\" /></td>",n,"1-49","49");
								}
							}
                            //51
							fprintf(cgiOut,"<td width=4></td>"\
							"<td><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame>"\
							"<img src=/images/3052/c_04.jpg width=17 height=12   border=0 alt=\"%s\" /></td>",n,"1-51","51");
						}
						else
						{
							fprintf(cgiOut,"<td width=1></td>");

						    //49
    				        value=1;
                            value=(value << 8) |(49);
                            if(show_eth_port_atrr(value,0,&pt)==CCGI_SUCCESS)
                        	{
                        	   	if(strcmp(eth_port_type_str[pt.port_type],"ETH_GE_SFP")==0)
                        	   	{
									fprintf(cgiOut,"<td>"
									"<img src=/images/3052/c_04.jpg width=17 height=12 border=0 alt=\"%s\" /></td>","49");

                                }
								else
								{
									fprintf(cgiOut,"<td>"
									"<img src=/images/3052/c_02.jpg width=17 height=12 border=0 alt=\"%s\" /></td>","49");
								}
							}
                            //51
							fprintf(cgiOut,"<td width=4></td>"\
							"<td>"\
							"<img src=/images/3052/c_04.jpg width=17 height=12   border=0 alt=\"%s\" /></td>","51");
						}
							
                      fprintf(cgiOut,"</tr>"\
                  "</table></td>"\
                "</tr>"\
                "<tr>"\
                  "<td height=4></td>"\
                "</tr>"\
                "<tr>"\
                  "<td><table id=__01 width=40 height=15 border=0 cellpadding=0 cellspacing=0>"\
                      "<tr>");

					  retu=checkuser_group(m);
					  if(retu==0)
    				  {
    				       fprintf(cgiOut,"<td width=1></td>");

    				        //50
    				        value=1;
                            value=(value << 8) |(50);
                            if(show_eth_port_atrr(value,0,&pt)==CCGI_SUCCESS)
                        	{
                        	   	if(strcmp(eth_port_type_str[pt.port_type],"ETH_GE_SFP")==0)
                        	   	{
		                            fprintf(cgiOut,"<td><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame>"\
		                            "<img src=/images/3052/c_04.jpg width=17 height=12  border=0 alt=\"%s\"  /></td>",n,"1-50","50");
                                }
								else
								{
		                            fprintf(cgiOut,"<td><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame>"\
		                            "<img src=/images/3052/c_03.jpg width=17 height=12  border=0 alt=\"%s\"  /></td>",n,"1-50","50");
								}
							}

							//52
                            fprintf(cgiOut,"<td width=4></td>"\
                            "<td><a href=wp_prtcfg.cgi?UN=%s&ID=%s target=mainFrame>"\
                            "<img src=/images/3052/c_04.jpg width=17 height=12  border=0 alt=\"%s\"  /></td>",n,"1-52","52");
    				  }
					  else
					  {

					        //50
    				        value=1;
                            value=(value << 8) |(50);
                            if(show_eth_port_atrr(value,0,&pt)==CCGI_SUCCESS)
                        	{
                        	   	if(strcmp(eth_port_type_str[pt.port_type],"ETH_GE_SFP")==0)
                        	   	{
		                            fprintf(cgiOut,"<td width=1></td>"\
		                            "<td>"\
		                            "<img src=/images/3052/c_04.jpg width=17 height=12  border=0 alt=\"%s\"  /></td>","50");
                                }
								else
								{
		                            fprintf(cgiOut,"<td width=1></td>"\
		                            "<td>"\
		                            "<img src=/images/3052/c_03.jpg width=17 height=12  border=0 alt=\"%s\"  /></td>","50");
							    }
							}
    						//52
                            fprintf(cgiOut,"<td width=4></td>"\
                            "<td>"\
                            "<img src=/images/3052/c_04.jpg width=17 height=12  border=0 alt=\"%s\"  /></td>","52");
    				  }
							
                      fprintf(cgiOut,"</tr>"\
                  "</table></td>"\
                "</tr>"\
                "<tr>"\
                  "<td height=4></td>"\
                "</tr>"\
                "<tr>"\
                  "<td><table id=__01 width=40 height=9 border=0 cellpadding=0 cellspacing=0>"\
                      "<tr>");
//50 光电互斥口		

        value=1;
        value=(value << 8) |(50);
        if(show_eth_port_atrr(value,0,&pt)==CCGI_SUCCESS)
		{
			if(strcmp(eth_port_type_str[pt.port_type],"ETH_GE_SFP")==0)
            {
				if(strcmp(onoff_status_str[(pt.attr_bitmap & ETH_ATTR_ADMIN_STATUS) >> ETH_ADMIN_STATUS_BIT],"OFF")==0)
				{
					fprintf(cgiOut,"<td><img src=/images/3052/h.jpg width=9 height=9  /></td>"); //黑色
				}
				else
				{
					if(strcmp(link_status_str[(pt.attr_bitmap & ETH_ATTR_LINK_STATUS) >> ETH_LINK_STATUS_BIT],"UP")==0)
					{
						//全绿色
						fprintf(cgiOut,"<td><img src=/images/3052/t_02.jpg width=9 height=9  /></td>"); //黑色
					}
					else
					{
						//全红色
						fprintf(cgiOut,"<td><img src=/images/3052/t_01.jpg width=9 height=9  /></td>"); //黑色
					}
				}
			}			
			else  /*type为电口*/
			{
				if(strcmp(onoff_status_str[(pt.attr_bitmap & ETH_ATTR_ADMIN_STATUS) >> ETH_ADMIN_STATUS_BIT],"OFF")==0)
				fprintf(cgiOut,"<td><img src=/images/3052/h.jpg width=9 height=9  /></td>"); //黑色
				else
				{
					if(strcmp(link_status_str[(pt.attr_bitmap & ETH_ATTR_LINK_STATUS) >> ETH_LINK_STATUS_BIT],"UP")==0 && 0==strcmp(eth_speed_str[(pt.attr_bitmap & ETH_ATTR_SPEED_MASK) >> ETH_SPEED_BIT],"1000M"))
					fprintf(cgiOut,"<td><img src=/images/3052/t_02.jpg width=9 height=9 ></td>"); //绿色
					else
					fprintf(cgiOut,"<td><img src=/images/3052/t_01.jpg width=9 height=9 ></td>"); //红色
				}
			}
		}
			  else
         		fprintf(cgiOut,"<td><img src=/images/3052/h.jpg width=9 height=9  /></td>"); //黑色

				fprintf(cgiOut,"<td width=1></td>");

        ///////////////////////////////////////////////////light2///////////////////////////////////////				
		if(show_eth_port_atrr(value,0,&pt)==CCGI_SUCCESS)
		{
			if(strcmp(eth_port_type_str[pt.port_type],"ETH_GE_SFP")==0)
            {
				if(strcmp(onoff_status_str[(pt.attr_bitmap & ETH_ATTR_ADMIN_STATUS) >> ETH_ADMIN_STATUS_BIT],"OFF")==0)
				{
					fprintf(cgiOut,"<td><img src=/images/3052/h.jpg width=9 height=9  /></td>"); //黑色
				}
				else
				{
					if(strcmp(link_status_str[(pt.attr_bitmap & ETH_ATTR_LINK_STATUS) >> ETH_LINK_STATUS_BIT],"UP")==0)
					{
						//全绿色
						fprintf(cgiOut,"<td><img src=/images/3052/t_02.jpg width=9 height=9  /></td>"); //黑色
					}
					else
					{
						//全红色
						fprintf(cgiOut,"<td><img src=/images/3052/t_01.jpg width=9 height=9  /></td>"); //黑色
					}
				}
			}			
			else  /*type为电口*/
			{
				if(strcmp(onoff_status_str[(pt.attr_bitmap & ETH_ATTR_ADMIN_STATUS) >> ETH_ADMIN_STATUS_BIT],"OFF")==0)
				fprintf(cgiOut,"<td><img src=/images/3052/h.jpg width=9 height=9  /></td>"); //黑色
				else
				{
					if(strcmp(link_status_str[(pt.attr_bitmap & ETH_ATTR_LINK_STATUS) >> ETH_LINK_STATUS_BIT],"UP")==0)
					fprintf(cgiOut,"<td><img src=/images/3052/t_02.jpg width=9 height=9 ></td>"); //绿色
					else
					fprintf(cgiOut,"<td><img src=/images/3052/t_01.jpg width=9 height=9 ></td>"); //红色
				}
			}
		}
     		  else
         		fprintf(cgiOut,"<td><img src=/images/3052/h.jpg width=9 height=9  /></td>"); //黑色
                fprintf(cgiOut,"<td width=2></td>");
                
//52 光口		                 

                value=1;
                value=(value << 8) |(52);
           		if(show_eth_port_atrr(value,0,&pt)==CCGI_SUCCESS)
				{
					if(strcmp(eth_port_type_str[pt.port_type],"ETH_GE_FIBER")!=0)
					{
						fprintf(cgiOut,"<td><img src=/images/3052/h.jpg width=9 height=9  /></td>"\
						"<td width=1></td>"\
						"<td><img src=/images/3052/h.jpg width=9 height=9  /></td>");
					}
					/*--------------------------  ----------------------------------------*/
					else
					{
						if(strcmp(onoff_status_str[(pt.attr_bitmap & ETH_ATTR_ADMIN_STATUS) >> ETH_ADMIN_STATUS_BIT],"OFF")==0)
						{


							fprintf(cgiOut,"<td><img src=/images/3052/h.jpg width=9 height=9  /></td>"\
							"<td width=1></td>"\
							"<td><img src=/images/3052/h.jpg width=9 height=9  /></td>");

						}
						else
						{
							if(strcmp(link_status_str[(pt.attr_bitmap & ETH_ATTR_LINK_STATUS) >> ETH_LINK_STATUS_BIT],"UP")==0)
							{
								//全绿色
								fprintf(cgiOut,"<td><img src=/images/3052/t_02.jpg width=9 height=9  /></td>"\
								"<td width=1></td>"\
								"<td><img src=/images/3052/t_02.jpg width=9 height=9  /></td>");

							}
							else
							{
								//全红色

								fprintf(cgiOut,"<td><img src=/images/3052/t_01.jpg width=9 height=9  /></td>"\
								"<td width=1></td>"\
								"<td><img src=/images/3052/t_01.jpg width=9 height=9  /></td>");


							}
						}
					}

					/*----------------------------  end  ----------------------------------------*/

				}
		                 
						
                      fprintf(cgiOut,"</tr>"\
                  "</table></td>"\
                "</tr>"\
            "</table></td>"\
          "</tr>"\
          "<tr>"\
            "<td height=13 colspan=8>&nbsp;</td>"\
          "</tr>"\
        "</table></td>"\
        "<td width=15%% valign=top><table width=89 height=77 border=0 cellpadding=0 cellspacing=0>"\
          "<tr>"\
            "<td width=89 height=8></td>"\
          "</tr>"\
          "<tr>");					  
            //fprintf(cgiOut,"<td><img src=/images/3052/logojpg.jpg width=75 height=38 /></td>");
            fprintf(cgiOut,"<td bgcolor=#C7C7C7></td>");
          fprintf(cgiOut,"</tr>"\
          "<tr>");
		 //获取产品名字  
		
            fprintf(cgiOut,"<td align=center ><span class=STYLE1></span></td>");
			
          fprintf(cgiOut,"</tr>"\
          "<tr>"\
            "<td height=10></td>"\
          "</tr>"\
          "<tr>"\
            "<td align=center style=font-weight:bold class=STYLE3>RUN SYS </td>"\
          "</tr>"\
          "<tr>"\
            "<td align=center><table id=__01 width=35 height=9 border=0 cellpadding=0 cellspacing=0>"\
              "<tr>"\
                "<td width=2></td>"\
                "<td width=18><img src=/images/3052/t_02.jpg width=9 height=9  /></td>"\
                "<td width=1></td>"\
                "<td width=14><img src=/images/3052/t_02.jpg width=9 height=9  /></td>"\
              "</tr>"\
            "</table></td>"\
          "</tr>"\
        "</table></td>"\
      "</tr>"\
    "</table></td>"\
  "</tr>"\
"</table>");



return 0;
}




/*指示灯的显示*/
int light_flag_3052(int begin,int end,struct eth_port_s pt,int type)
{
	int i;
	unsigned int value = 0;
	for(i=begin;i<end;i++)
	{
		if(type==1)
		{
			value=1;
			value=(value << 8) |(2*i-1);
		}
		else if(type==2)
		{
			value=1;
			value=(value << 8) |(2*i);
		}

		if(show_eth_port_atrr(value,0,&pt)==CCGI_SUCCESS)
		{
			if(strcmp(onoff_status_str[(pt.attr_bitmap & ETH_ATTR_ADMIN_STATUS) >> ETH_ADMIN_STATUS_BIT],"OFF")==0)
			{
				fprintf(cgiOut,"<td><img src=/images/3052/h.jpg width=9 height=9  /></td>"); //黑色
			}
			else
			{
				if(strcmp(link_status_str[(pt.attr_bitmap & ETH_ATTR_LINK_STATUS) >> ETH_LINK_STATUS_BIT],"UP")==0 && 0==strcmp(eth_speed_str[(pt.attr_bitmap & ETH_ATTR_SPEED_MASK) >> ETH_SPEED_BIT],"1000M"))
				fprintf(cgiOut,"<td><img src=/images/3052/t_02.jpg width=9 height=9  /></td>"); //绿色
				else
				fprintf(cgiOut,"<td><img src=/images/3052/t_01.jpg width=9 height=9  /></td>"); //红色
			}
		}   
		else
		fprintf(cgiOut,"<td><img src=/images/3052/h.jpg width=9 height=9  /></td>"); //黑色
		fprintf(cgiOut,"<td width=1></td>");		
		///////第一个灯	

		if(show_eth_port_atrr(value,0,&pt)==CCGI_SUCCESS)
		{
			if(strcmp(onoff_status_str[(pt.attr_bitmap & ETH_ATTR_ADMIN_STATUS) >> ETH_ADMIN_STATUS_BIT],"OFF")==0)
			fprintf(cgiOut,"<td><img src=/images/3052/t_02.jpg width=9 height=9  /></td>"); //绿色
			else
			{
				if(strcmp(link_status_str[(pt.attr_bitmap & ETH_ATTR_LINK_STATUS) >> ETH_LINK_STATUS_BIT],"UP")==0)
				fprintf(cgiOut,"<td><img src=/images/3052/t_02.jpg width=9 height=9  /></td>"); //绿色
				else
				fprintf(cgiOut,"<td><img src=/images/3052/t_01.jpg width=9 height=9  /></td>"); //红色
			}
		}
		else
		fprintf(cgiOut,"<td><img src=/images/3052/h.jpg width=9 height=9  /></td>"); //黑色
		fprintf(cgiOut,"<td width=2></td>");
	}    	

	return 0;
}


/*特殊的3052*/
int light_spe_3052(int key,struct eth_port_s pt,char *content)
{
	unsigned int value=0;
	value=1;
	value=(value << 8) | key;
	if(show_eth_port_atrr(value,0,&pt)==CCGI_SUCCESS)
	{
		if(strcmp(onoff_status_str[(pt.attr_bitmap & ETH_ATTR_ADMIN_STATUS) >> ETH_ADMIN_STATUS_BIT],"OFF")==0)
		{
			fprintf(cgiOut,"<td><img src=/images/3052/h.jpg width=9 height=9  /></td>"); //黑色
		}
		else
		{
			if(strcmp(link_status_str[(pt.attr_bitmap & ETH_ATTR_LINK_STATUS) >> ETH_LINK_STATUS_BIT],"UP")==0 && 0==strcmp(eth_speed_str[(pt.attr_bitmap & ETH_ATTR_SPEED_MASK) >> ETH_SPEED_BIT],"1000M"))
			fprintf(cgiOut,"<td><img src=/images/3052/t_02.jpg width=9 height=9  /></td>"); //绿色
			else
			fprintf(cgiOut,"<td><img src=/images/3052/t_01.jpg width=9 height=9  /></td>"); //红色
		}
	}   
	else
	fprintf(cgiOut,"<td><img src=/images/3052/h.jpg width=9 height=9  /></td>"); //黑色
	fprintf(cgiOut,"<td width=1></td>");	
	///////第一个灯	

	if(show_eth_port_atrr(value,0,&pt)==CCGI_SUCCESS)
	{
		if(strcmp(onoff_status_str[(pt.attr_bitmap & ETH_ATTR_ADMIN_STATUS) >> ETH_ADMIN_STATUS_BIT],"OFF")==0)
		fprintf(cgiOut,"<td><img src=/images/3052/t_02.jpg width=9 height=9  /></td>"); //绿色
		else
		{
			if(strcmp(link_status_str[(pt.attr_bitmap & ETH_ATTR_LINK_STATUS) >> ETH_LINK_STATUS_BIT],"UP")==0)
			fprintf(cgiOut,"<td><img src=/images/3052/t_02.jpg width=9 height=9  /></td>"); //绿色
			else
			fprintf(cgiOut,"<td><img src=/images/3052/t_01.jpg width=9 height=9  /></td>"); //红色
		}
	}
	else
	fprintf(cgiOut,"<td><img src=/images/3052/h.jpg width=9 height=9  /></td>"); //黑色
	fprintf(cgiOut,"%s",content);
	return 0;
}

/*指示灯的显示*/
int light_flag_5612i(int begin,int end,struct eth_port_s pt,int type,char *flight,char *fsep,char *slight,char *ssep)
{

	int i;
	unsigned int value = 0;
	for(i=begin;i<=end;i++)
	{
		if(type==1)
		{
			value=1;
			value=(value << 8) |(2*i-1);
		}
		else if(type==2)
		{
			value=1;
			value=(value << 8) |(2*i);
		}

		if(show_eth_port_atrr(value,0,&pt)==CCGI_SUCCESS)
		{
			if(strcmp(onoff_status_str[(pt.attr_bitmap & ETH_ATTR_ADMIN_STATUS) >> ETH_ADMIN_STATUS_BIT],"OFF")==0)
			{
				fprintf(cgiOut,"%s<img src=/images/5612i/a_15.jpg width=9 height=9 ></td>",flight); //黑色
			}
			else
			{
				if(strcmp(link_status_str[(pt.attr_bitmap & ETH_ATTR_LINK_STATUS) >> ETH_LINK_STATUS_BIT],"UP")==0 && 0==strcmp(eth_speed_str[(pt.attr_bitmap & ETH_ATTR_SPEED_MASK) >> ETH_SPEED_BIT],"1000M"))
				fprintf(cgiOut,"%s<img src=/images/5612i/a_19.jpg width=9 height=9 ></td>",flight); //绿色
				else
				fprintf(cgiOut,"%s<img src=/images/5612i/a_21.jpg width=9 height=9 ></td>",flight); //红色
			}
		}   
		else
		fprintf(cgiOut,"%s<img src=/images/5612i/a_15.jpg width=9 height=9 ></td>",flight); //黑色

		fprintf(cgiOut,"%s",fsep);		
		///////第一个灯	

		if(show_eth_port_atrr(value,0,&pt)==CCGI_SUCCESS)
		{
			if(strcmp(onoff_status_str[(pt.attr_bitmap & ETH_ATTR_ADMIN_STATUS) >> ETH_ADMIN_STATUS_BIT],"OFF")==0)
			fprintf(cgiOut,"%s<img src=/images/5612i/a_19.jpg width=9 height=9 ></td>",slight); //绿色
			else
			{
				if(strcmp(link_status_str[(pt.attr_bitmap & ETH_ATTR_LINK_STATUS) >> ETH_LINK_STATUS_BIT],"UP")==0)
				fprintf(cgiOut,"%s<img src=/images/5612i/a_19.jpg width=9 height=9 ></td>",slight); //绿色
				else
				fprintf(cgiOut,"%s<img src=/images/5612i/a_21.jpg width=9 height=9 ></td>",slight); //红色
			}
		}
		else
		fprintf(cgiOut,"%s<img src=/images/5612i/a_15.jpg width=9 height=9 ></td>",slight); //黑色
		fprintf(cgiOut,"%s",ssep);
	}    	

	return 0;
}

/*指示灯的显示*/
int light_flag(int begin,int end,struct eth_port_s pt,int type)
{
	int i;
	unsigned int value = 0;
	for(i=begin;i<end;i++)
	{
		if(type==1)
		{
			value=1;
			value=(value << 8) |(2*i-1);
		}
		else if(type==2)
		{
			value=1;
			value=(value << 8) |(2*i);
		}

		if(show_eth_port_atrr(value,0,&pt)==CCGI_SUCCESS)
		{
			if(strcmp(onoff_status_str[(pt.attr_bitmap & ETH_ATTR_ADMIN_STATUS) >> ETH_ADMIN_STATUS_BIT],"OFF")==0)
			{
				fprintf(cgiOut,"<td><img src=/images/new5000/5624_05.jpg width=9 height=9 ></td>"); //黑色
			}
			else
			{
				if(strcmp(link_status_str[(pt.attr_bitmap & ETH_ATTR_LINK_STATUS) >> ETH_LINK_STATUS_BIT],"UP")==0 && 0==strcmp(eth_speed_str[(pt.attr_bitmap & ETH_ATTR_SPEED_MASK) >> ETH_SPEED_BIT],"1000M"))
				fprintf(cgiOut,"<td><img src=/images/new5000/5624_11.jpg width=9 height=9 ></td>"); //绿色
				else
				fprintf(cgiOut,"<td><img src=/images/new5000/5624_13.jpg width=9 height=9 ></td>"); //红色
			}
		}   
		else
		fprintf(cgiOut,"<td><img src=/images/new5000/5624_05.jpg width=9 height=9 ></td>"); //黑色
		fprintf(cgiOut,"<td>"\
		"<img src=/images/new5000/5624_12.jpg width=4 height=9 ></td>");		
		///////第一个灯	

		if(show_eth_port_atrr(value,0,&pt)==CCGI_SUCCESS)
		{
			if(strcmp(onoff_status_str[(pt.attr_bitmap & ETH_ATTR_ADMIN_STATUS) >> ETH_ADMIN_STATUS_BIT],"OFF")==0)
			fprintf(cgiOut,"<td><img src=/images/new5000/5624_11.jpg width=9 height=9 ></td>"); //绿色
			else
			{
				if(strcmp(link_status_str[(pt.attr_bitmap & ETH_ATTR_LINK_STATUS) >> ETH_LINK_STATUS_BIT],"UP")==0)
				fprintf(cgiOut,"<td><img src=/images/new5000/5624_11.jpg width=9 height=9 ></td>"); //绿色
				else
				fprintf(cgiOut,"<td><img src=/images/new5000/5624_13.jpg width=9 height=9 ></td>"); //红色
			}
		}
		else
		fprintf(cgiOut,"<td><img src=/images/new5000/5624_05.jpg width=9 height=9 ></td>"); //黑色
		fprintf(cgiOut,"<td><img src=/images/new5000/5624_14.jpg width=11 height=9 ></td>");
	}    	

	return 0;
}

/*特殊*/
int light_spe(int key,struct eth_port_s pt,char *content)
{
	unsigned int value=0;
	value=1;
	value=(value << 8) | key;
	if(show_eth_port_atrr(value,0,&pt)==CCGI_SUCCESS)
	{
		if(strcmp(onoff_status_str[(pt.attr_bitmap & ETH_ATTR_ADMIN_STATUS) >> ETH_ADMIN_STATUS_BIT],"OFF")==0)
		{
			fprintf(cgiOut,"<td><img src=/images/new5000/5624_05.jpg width=9 height=9 ></td>"); //黑色
		}
		else
		{
			if(strcmp(link_status_str[(pt.attr_bitmap & ETH_ATTR_LINK_STATUS) >> ETH_LINK_STATUS_BIT],"UP")==0 && 0==strcmp(eth_speed_str[(pt.attr_bitmap & ETH_ATTR_SPEED_MASK) >> ETH_SPEED_BIT],"1000M"))
			fprintf(cgiOut,"<td><img src=/images/new5000/5624_11.jpg width=9 height=9 ></td>"); //绿色
			else
			fprintf(cgiOut,"<td><img src=/images/new5000/5624_13.jpg width=9 height=9 ></td>"); //红色
		}
	}   
	else
	fprintf(cgiOut,"<td><img src=/images/new5000/5624_05.jpg width=9 height=9 ></td>"); //黑色
	fprintf(cgiOut,"<td>"\
	"<img src=/images/new5000/5624_12.jpg width=4 height=9 ></td>");		
	///////第一个灯	

	if(show_eth_port_atrr(value,0,&pt)==CCGI_SUCCESS)
	{
		if(strcmp(onoff_status_str[(pt.attr_bitmap & ETH_ATTR_ADMIN_STATUS) >> ETH_ADMIN_STATUS_BIT],"OFF")==0)
		fprintf(cgiOut,"<td><img src=/images/new5000/5624_11.jpg width=9 height=9 ></td>"); //绿色
		else
		{
			if(strcmp(link_status_str[(pt.attr_bitmap & ETH_ATTR_LINK_STATUS) >> ETH_LINK_STATUS_BIT],"UP")==0)
			fprintf(cgiOut,"<td><img src=/images/new5000/5624_11.jpg width=9 height=9 ></td>"); //绿色
			else
			fprintf(cgiOut,"<td><img src=/images/new5000/5624_13.jpg width=9 height=9 ></td>"); //红色
		}
	}
	else
	fprintf(cgiOut,"<td><img src=/images/new5000/5624_05.jpg width=9 height=9 ></td>"); //黑色
	fprintf(cgiOut,"%s",content);
	return 0;
}





/*指示灯的显示,不显示指示灯*/
int light_flag_new(int begin,int end,struct eth_port_s pt,int type)
{
	int i;
	unsigned int value = 0;
	for(i=begin;i<end;i++)
	{
		if(type==1)
		{
			value=1;
			value=(value << 8) |(2*i-1);
		}
		else if(type==2)
		{
			value=1;
			value=(value << 8) |(2*i);
		}


		fprintf(cgiOut,"<td><img src=/images/new5000/new.jpg width=9 height=9 ></td>"); //黑色
		fprintf(cgiOut,"<td>"\
		"<img src=/images/new5000/5624_12.jpg width=4 height=9 ></td>");		
		///////第一个灯	
		fprintf(cgiOut,"<td><img src=/images/new5000/new.jpg width=9 height=9 ></td>"); //黑色
		fprintf(cgiOut,"<td><img src=/images/new5000/5624_14.jpg width=11 height=9 ></td>");
	}    	

	return 0;
}


/*特殊，不显示指示灯*/
int light_spe_new(int key,struct eth_port_s pt,char *content)
{
	unsigned int value=0;
	value=1;
	value=(value << 8) | key;

	fprintf(cgiOut,"<td><img src=/images/new5000/new.jpg width=9 height=9 ></td>"); //黑色
	fprintf(cgiOut,"<td>"\
	"<img src=/images/new5000/5624_12.jpg width=4 height=9 ></td>");		
	///////第一个灯	
	fprintf(cgiOut,"<td><img src=/images/new5000/new.jpg width=9 height=9 ></td>"); //黑色
	fprintf(cgiOut,"%s",content);
	return 0;
}
void panel_86()
{
	fprintf(cgiOut,"<table id=Table_01 width=719 height=842 border=0 cellpadding=0 cellspacing=0>"\
	"<tr>"\
		"<td>"\
			"<img src=\"/images/8610/8610_01.gif\" width=13 height=65 ></td>"\
		"<td>"\
			"<img src=\"/images/8610/8610_02.gif\" width=63 height=65 ></td>"\
		"<td>"\
			"<img src=\"/images/8610/8610_03.gif\" width=6 height=65 ></td>"\
		"<td>"\
			"<img src=\"/images/8610/8610_04.gif\" width=63 height=65 ></td>"\
		"<td>"\
			"<img src=\"/images/8610/8610_05.gif\" width=6 height=65 ></td>"\
		"<td>"\
			"<img src=\"/images/8610/8610_06.gif\" width=63 height=65 ></td>"\
		"<td>"\
			"<img src=\"/images/8610/8610_07.gif\" width=6 height=65 ></td>"\
		"<td>"\
			"<img src=\"/images/8610/8610_08.gif\" width=63 height=65 ></td>"\
		"<td>"\
			"<img src=\"/images/8610/8610_09.gif\" width=8 height=65 ></td>"\
		"<td>"\
			"<img src=\"/images/8610/8610_10.gif\" width=63 height=65 ></td>"\
		"<td>"\
			"<img src=\"/images/8610/8610_11.gif\" width=6 height=65 ></td>"\
		"<td>"\
			"<img src=\"/images/8610/8610_12.gif\" width=63 height=65 ></td>"\
		"<td>"\
			"<img src=\"/images/8610/8610_13.gif\" width=6 height=65 ></td>"\
		"<td>"\
			"<img src=\"/images/8610/8610_14.gif\" width=63 height=65 ></td>"\
		"<td>"\
			"<img src=\"/images/8610/8610_15.gif\" width=7 height=65 ></td>"\
		"<td>"\
			"<img src=\"/images/8610/8610_16.gif\" width=63 height=65 ></td>"\
		"<td>"\
			"<img src=\"/images/8610/8610_17.gif\" width=9 height=65 ></td>"\
		"<td>"\
			"<img src=\"/images/8610/8610_18.gif\" width=63 height=65 ></td>"\
		"<td>"\
			"<img src=\"/images/8610/8610_19.gif\" width=8 height=65 ></td>"\
		"<td>"\
			"<img src=\"/images/8610/8610_20.gif\" width=63 height=65 ></td>"\
		"<td>"\
			"<img src=\"/images/8610/8610_21.gif\" width=14 height=65 ></td>"\
	"</tr>"\
	"<tr>"\
		"<td>"\
			"<img src=\"/images/8610/8610_22.gif\" width=13 height=708 ></td>"\
		"<td>");
	//slot 1
	//fprintf(cgiOut,"<img src=\"/images/8610/8610_23.gif\" width=63 height=708 >");
	select_86_slotinfo(1);
	fprintf(cgiOut,"</td>"\
		"<td>"\
			"<img src=\"/images/8610/8610_24.gif\" width=6 height=708 ></td>"\
		"<td>");
	//slot 2
	//fprintf(cgiOut,"<img src=\"/images/8610/8610_25.gif\" width=63 height=708 >");
	select_86_slotinfo(2);
	fprintf(cgiOut,"</td>"\
		"<td>"\
			"<img src=\"/images/8610/8610_26.gif\" width=6 height=708 ></td>"\
		"<td>");
	//slot 3
	//fprintf(cgiOut,"<img src=\"/images/8610/8610_27.gif\" width=63 height=708 >");
	select_86_slotinfo(3);
	fprintf(cgiOut,"</td>"\
		"<td>"\
			"<img src=\"/images/8610/8610_28.gif\" width=6 height=708 ></td>"\
		"<td>");
	//slot 4
	//fprintf(cgiOut,"<img src=\"/images/8610/8610_29.gif\" width=63 height=708 >");
	select_86_slotinfo(4);
	fprintf(cgiOut,"</td>"\
		"<td>"\
			"<img src=\"/images/8610/8610_30.gif\" width=8 height=708 ></td>"\
		"<td>");
	//slot 5
	//fprintf(cgiOut,"<img src=\"/images/8610/8610_31.gif\" width=63 height=708 >");
	select_86_slotinfo(5);
	fprintf(cgiOut,"</td>"\
		"<td>"\
			"<img src=\"/images/8610/8610_32.gif\" width=6 height=708 ></td>"\
		"<td>");
	//slot 6
	//fprintf(cgiOut,"<img src=\"/images/8610/8610_33.gif\" width=63 height=708 >");
	select_86_slotinfo(6);
	fprintf(cgiOut,"</td>"\
		"<td>"\
			"<img src=\"/images/8610/8610_34.gif\" width=6 height=708 ></td>"\
		"<td>");
	//slot 7
	//fprintf(cgiOut,"<img src=\"/images/8610/8610_35.gif\" width=63 height=708 >");
	select_86_slotinfo(7);
	fprintf(cgiOut,"</td>"\
		"<td>"\
			"<img src=\"/images/8610/8610_36.gif\" width=7 height=708 ></td>"\
		"<td>");
	//slot 8	
	//fprintf(cgiOut,"<img src=\"/images/8610/8610_37.gif\" width=63 height=708 >");
	select_86_slotinfo(8);
	fprintf(cgiOut,"</td>"\
		"<td>"\
			"<img src=\"/images/8610/8610_38.gif\" width=9 height=708 ></td>"\
		"<td bgcolor=#A3A3A3>");
	//slot 9	
	//fprintf(cgiOut,"<img src=\"/images/8610/8610_39.gif\" width=63 height=708 >");
	select_86_slotinfo(9);
	fprintf(cgiOut,"</td>"\
		"<td>"\
			"<img src=\"/images/8610/8610_40.gif\" width=8 height=708 ></td>"\
		"<td bgcolor=#A3A3A3>");
	//slot 10
	//fprintf(cgiOut,"<img src=\"/images/8610/8610_41.gif\" width=63 height=708 >");
	select_86_slotinfo(10);
	fprintf(cgiOut,"</td>"\
		"<td>"\
			"<img src=\"/images/8610/8610_42.gif\" width=14 height=708 ></td>"\
	"</tr>"\
	"<tr>"\
		"<td>"\
			"<img src=\"/images/8610/8610_43.gif\" width=13 height=69 ></td>"\
		"<td>"\
			"<img src=\"/images/8610/8610_44.gif\" width=63 height=69 ></td>"\
		"<td>"\
			"<img src=\"/images/8610/8610_45.gif\" width=6 height=69 ></td>"\
		"<td>"\
			"<img src=\"/images/8610/8610_46.gif\" width=63 height=69 ></td>"\
		"<td>"\
			"<img src=\"/images/8610/8610_47.gif\" width=6 height=69 ></td>"\
		"<td>"\
			"<img src=\"/images/8610/8610_48.gif\" width=63 height=69 ></td>"\
		"<td>"\
			"<img src=\"/images/8610/8610_49.gif\" width=6 height=69 ></td>"\
		"<td>"\
			"<img src=\"/images/8610/8610_50.gif\" width=63 height=69 ></td>"\
		"<td>"\
			"<img src=\"/images/8610/8610_51.gif\" width=8 height=69 ></td>"\
		"<td>"\
			"<img src=\"/images/8610/8610_52.gif\" width=63 height=69 ></td>"\
		"<td>"\
			"<img src=\"/images/8610/8610_53.gif\" width=6 height=69 ></td>"\
		"<td>"\
			"<img src=\"/images/8610/8610_54.gif\" width=63 height=69 ></td>"\
		"<td>"\
			"<img src=\"/images/8610/8610_55.gif\" width=6 height=69 ></td>"\
		"<td>"\
			"<img src=\"/images/8610/8610_56.gif\" width=63 height=69 ></td>"\
		"<td>"\
			"<img src=\"/images/8610/8610_57.gif\" width=7 height=69 ></td>"\
		"<td>"\
			"<img src=\"/images/8610/8610_58.gif\" width=63 height=69 ></td>"\
		"<td>"\
			"<img src=\"/images/8610/8610_59.gif\" width=9 height=69 ></td>"\
		"<td>"\
			"<img src=\"/images/8610/8610_60.gif\" width=63 height=69 ></td>"\
		"<td>"\
			"<img src=\"/images/8610/8610_61.gif\" width=8 height=69 ></td>"\
		"<td>"\
			"<img src=\"/images/8610/8610_62.gif\" width=63 height=69 ></td>"\
		"<td>"\
			"<img src=\"/images/8610/8610_63.gif\" width=14 height=69 ></td>"\
	"</tr>"\
"</table>");
}
void  select_86_slotinfo(int slotid)
{
	int ret = 0;
	char path_buf[128] = {0};
	char board_name[30] = {0};
	
	memset(path_buf,0,sizeof(path_buf));
	sprintf(path_buf, "/dbm/product/slot/slot%d/name", slotid);
	memset(board_name,0,sizeof(board_name));
	ret = get_str_from_file(path_buf, &board_name);
	if(ret == -1)
	{
		fprintf(cgiOut,"<img src=\"/images/8610/8610_gray.gif\" width=63 height=708 >");
	}
	else
	{
		if(0 == strcmp(board_name,"unkown"))
		{
			fprintf(cgiOut,"<img src=\"/images/8610/8610_gray.gif\" width=63 height=708 >");
		}
		if(0 == strcmp(board_name,"AX81_AC8C"))
		{
			//fprintf(cgiOut,"<img src=\"/images/8610/8610_8ports.gif\" width=63 height=708 >");
			fprintf(cgiOut,"<table id=Table_01 width=63 height=708 border=0 cellpadding=0 cellspacing=0>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_8ports_01.gif\" width=63 height=36 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>");
			//ports
			//fprintf(cgiOut,"<img src=\"/images/8610/8610_8ports_02.gif\" width=63 height=113 >");
			fprintf(cgiOut,"<table id=Table_01 width=63 height=113 border=0 cellpadding=0 cellspacing=0>"\
				"<tr>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_02_01.gif\" width=18 height=6 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_12_6.gif\" width=12 height=6 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_3_6.gif\" width=3 height=6 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_12_6.gif\" width=12 height=6 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_02_05.gif\" width=18 height=6 ></td>"\
				"</tr>"\
				"<tr>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_02_06.gif\" width=18 height=20 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_eblack.gif\" width=12 height=20 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_3_20.gif\" width=3 height=20 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_eblack.gif\" width=12 height=20 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_02_10.gif\" width=18 height=20 ></td>"\
				"</tr>"\
				"<tr>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_02_11.gif\" width=18 height=7 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_12_7.gif\" width=12 height=7 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_3_7.gif\" width=3 height=7 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_12_7.gif\" width=12 height=7 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_02_15.gif\" width=18 height=7 ></td>"\
				"</tr>"\
				"<tr>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_02_16.gif\" width=18 height=20 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_eblack.gif\" width=12 height=20 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_3_20.gif\" width=3 height=20 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_eblack.gif\" width=12 height=20 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_02_20.gif\" width=18 height=20 ></td>"\
				"</tr>"\
				"<tr>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_02_21.gif\" width=18 height=7 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_12_7.gif\" width=12 height=7 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_3_7.gif\" width=3 height=7 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_12_7.gif\" width=12 height=7 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_02_25.gif\" width=18 height=7 ></td>"\
				"</tr>"\
				"<tr>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_02_26.gif\" width=18 height=20 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_eblack.gif\" width=12 height=20 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_3_20.gif\" width=3 height=20 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_eblack.gif\" width=12 height=20 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_02_30.gif\" width=18 height=20 ></td>"\
				"</tr>"\
				"<tr>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_02_31.gif\" width=18 height=7 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_12_7.gif\" width=12 height=7 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_3_7.gif\" width=3 height=7 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_12_7.gif\" width=12 height=7 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_02_35.gif\" width=18 height=7 ></td>"\
				"</tr>"\
				"<tr>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_02_36.gif\" width=18 height=20 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_eblack.gif\" width=12 height=20 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_02_38.gif\" width=3 height=20 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_eblack.gif\" width=12 height=20 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_02_40.gif\" width=18 height=20 ></td>"\
				"</tr>"\
				"<tr>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_02_41.gif\" width=18 height=6 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_12_6.gif\" width=12 height=6 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_3_6.gif\" width=3 height=6 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_12_6.gif\" width=12 height=6 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_02_45.gif\" width=18 height=6 ></td>"\
				"</tr>"\
			"</table>");
			fprintf(cgiOut,"</td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_8ports_03.gif\" width=63 height=85 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>");
			//ports
			//fprintf(cgiOut,"<img src=\"/images/8610/8610_8ports_04.gif\" width=63 height=113 >");
			fprintf(cgiOut,"<table id=Table_01 width=63 height=113 border=0 cellpadding=0 cellspacing=0>"\
				"<tr>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_04_01.gif\" width=12 height=6 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_18_6.gif\" width=18 height=6 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_3_6.gif\" width=3 height=6 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_18_6.gif\" width=18 height=6 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_04_05.gif\" width=12 height=6 ></td>"\
				"</tr>"\
				"<tr>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_04_06.gif\" width=12 height=20 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_lblack.gif\" width=18 height=20 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_3_20.gif\" width=3 height=20 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_rblack.gif\" width=18 height=20 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_04_10.gif\" width=12 height=20 ></td>"\
				"</tr>"\
				"<tr>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_04_11.gif\" width=12 height=7 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_18_7.gif\" width=18 height=7 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_3_7.gif\" width=3 height=7 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_18_7.gif\" width=18 height=7 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_04_15.gif\" width=12 height=7 ></td>"\
				"</tr>"\
				"<tr>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_04_16.gif\" width=12 height=20 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_lblack.gif\" width=18 height=20 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_3_20.gif\" width=3 height=20 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_rblack.gif\" width=18 height=20 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_04_20.gif\" width=12 height=20 ></td>"\
				"</tr>"\
				"<tr>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_04_21.gif\" width=12 height=7 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_18_7.gif\" width=18 height=7 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_3_7.gif\" width=3 height=7 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_18_7.gif\" width=18 height=7 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_04_25.gif\" width=12 height=7 ></td>"\
				"</tr>"\
				"<tr>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_04_26.gif\" width=12 height=20 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_lblack.gif\" width=18 height=20 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_3_20.gif\" width=3 height=20 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_rblack.gif\" width=18 height=20 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_04_30.gif\" width=12 height=20 ></td>"\
				"</tr>"\
				"<tr>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_04_31.gif\" width=12 height=7 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_18_7.gif\" width=18 height=7 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_3_7.gif\" width=3 height=7 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_18_7.gif\" width=18 height=7 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_04_35.gif\" width=12 height=7 ></td>"\
				"</tr>"\
				"<tr>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_04_36.gif\" width=12 height=20 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_lblack.gif\" width=18 height=20 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_3_20.gif\" width=3 height=20 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_rblack.gif\" width=18 height=20 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_04_40.gif\" width=12 height=20 ></td>"\
				"</tr>"\
				"<tr>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_04_41.gif\" width=12 height=6 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_18_6.gif\" width=18 height=6 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_3_6.gif\" width=3 height=6 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_18_6.gif\" width=18 height=6 ></td>"\
					"<td>"\
						"<img src=\"/images/8610/8610_8ports_04_45.gif\" width=12 height=6 ></td>"\
				"</tr>"\
			"</table>");
			fprintf(cgiOut,"</td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_8ports_05.gif\" width=63 height=361 ></td>"\
			"</tr>"\
		"</table>");
		}
		if(0 == strcmp(board_name,"AX81_2X12G12S"))
		{
			//fprintf(cgiOut,"<img src=\"/images/8610/8610_12ports.gif\" width=63 height=708 >");
			fprintf(cgiOut,"<table id=Table_01 width=63 height=708 border=0 cellpadding=0 cellspacing=0>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_01.gif\" width=63 height=35 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>");
			//ports
			//fprintf(cgiOut,"<img src=\"/images/8610/8610_12ports_02.gif\" width=63 height=168 >");
			fprintf(cgiOut,"<table id=Table_01 width=63 height=168 border=0 cellpadding=0 cellspacing=0>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_01.gif\" width=17 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_13_6.gif\" width=13 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_6.gif\" width=2 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_13_6.gif\" width=13 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_05.gif\" width=18 height=6 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_06.gif\" width=17 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_eblack.gif\" width=13 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_21.gif\" width=2 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_eblack.gif\" width=13 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_10.gif\" width=18 height=21 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_11.gif\" width=17 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_13_6.gif\" width=13 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_6.gif\" width=2 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_13_6.gif\" width=13 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_15.gif\" width=18 height=6 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_16.gif\" width=17 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_eblack.gif\" width=13 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_21.gif\" width=2 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_eblack.gif\" width=13 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_20.gif\" width=18 height=21 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_21.gif\" width=17 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_13_6.gif\" width=13 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_6.gif\" width=2 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_13_6.gif\" width=13 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_25.gif\" width=18 height=6 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_26.gif\" width=17 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_eblack.gif\" width=13 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_21.gif\" width=2 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_eblack.gif\" width=13 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_30.gif\" width=18 height=21 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_31.gif\" width=17 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_13_6.gif\" width=13 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_6.gif\" width=2 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_13_6.gif\" width=13 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_35.gif\" width=18 height=6 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_36.gif\" width=17 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_eblack.gif\" width=13 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_21.gif\" width=2 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_eblack.gif\" width=13 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_40.gif\" width=18 height=21 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_41.gif\" width=17 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_13_6.gif\" width=13 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_6.gif\" width=2 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_13_6.gif\" width=13 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_45.gif\" width=18 height=6 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_46.gif\" width=17 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_eblack.gif\" width=13 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_21.gif\" width=2 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_eblack.gif\" width=13 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_50.gif\" width=18 height=21 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_51.gif\" width=17 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_13_6.gif\" width=13 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_6.gif\" width=2 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_13_6.gif\" width=13 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_55.gif\" width=18 height=6 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_56.gif\" width=17 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_eblack.gif\" width=13 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_21.gif\" width=2 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_eblack.gif\" width=13 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_60.gif\" width=18 height=21 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_61.gif\" width=17 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_13_6.gif\" width=13 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_6.gif\" width=2 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_13_6.gif\" width=13 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_65.gif\" width=18 height=6 ></td>"\
			"</tr>"\
		"</table>");
			fprintf(cgiOut,"</td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_03.gif\" width=63 height=30 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>");
			//ports
			//fprintf(cgiOut,"<img src=\"/images/8610/8610_12ports_04.gif\" width=63 height=168 >");
			fprintf(cgiOut,"<table id=Table_01 width=63 height=168 border=0 cellpadding=0 cellspacing=0>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_01.gif\" width=11 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_19_6.gif\" width=19 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_6.gif\" width=2 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_19_6.gif\" width=19 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_05.gif\" width=12 height=6 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_06.gif\" width=11 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_lblack.gif\" width=19 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_21.gif\" width=2 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_rblack.gif\" width=19 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_10.gif\" width=12 height=21 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_11.gif\" width=11 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_19_6.gif\" width=19 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_6.gif\" width=2 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_19_6.gif\" width=19 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_15.gif\" width=12 height=6 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_16.gif\" width=11 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_lblack.gif\" width=19 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_21.gif\" width=2 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_rblack.gif\" width=19 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_20.gif\" width=12 height=21 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_21.gif\" width=11 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_19_6.gif\" width=19 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_6.gif\" width=2 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_19_6.gif\" width=19 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_25.gif\" width=12 height=6 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_26.gif\" width=11 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_lblack.gif\" width=19 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_21.gif\" width=2 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_rblack.gif\" width=19 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_30.gif\" width=12 height=21 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_31.gif\" width=11 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_19_6.gif\" width=19 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_6.gif\" width=2 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_19_6.gif\" width=19 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_35.gif\" width=12 height=6 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_36.gif\" width=11 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_lblack.gif\" width=19 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_21.gif\" width=2 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_rblack.gif\" width=19 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_40.gif\" width=12 height=21 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_41.gif\" width=11 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_19_6.gif\" width=19 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_6.gif\" width=2 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_19_6.gif\" width=19 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_45.gif\" width=12 height=6 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_46.gif\" width=11 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_lblack.gif\" width=19 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_21.gif\" width=2 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_rblack.gif\" width=19 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_50.gif\" width=12 height=21 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_51.gif\" width=11 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_19_6.gif\" width=19 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_6.gif\" width=2 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_19_6.gif\" width=19 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_55.gif\" width=12 height=6 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_56.gif\" width=11 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_lblack.gif\" width=19 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_21.gif\" width=2 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_rblack.gif\" width=19 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_60.gif\" width=12 height=21 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_61.gif\" width=11 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_19_6.gif\" width=19 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_6.gif\" width=2 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_19_6.gif\" width=19 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_65.gif\" width=12 height=6 ></td>"\
			"</tr>"\
		"</table>");
			fprintf(cgiOut,"</td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_05.gif\" width=63 height=307 ></td>"\
			"</tr>"\
		"</table>");
		}
		if(0 == strcmp(board_name,"AX81_SMU"))
		{
			fprintf(cgiOut,"<img src=\"/images/8610/8610_mcontrol.gif\" width=63 height=708 >");
		}
		if(0 == strcmp(board_name,"AX81_1X12G12S"))
		{
			//fprintf(cgiOut,"<img src=\"/images/8610/8610_12ports.gif\" width=63 height=708 >");
			fprintf(cgiOut,"<table id=Table_01 width=63 height=708 border=0 cellpadding=0 cellspacing=0>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_01.gif\" width=63 height=35 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>");
			//ports
			//fprintf(cgiOut,"<img src=\"/images/8610/8610_12ports_02.gif\" width=63 height=168 >");
			fprintf(cgiOut,"<table id=Table_01 width=63 height=168 border=0 cellpadding=0 cellspacing=0>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_01.gif\" width=17 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_13_6.gif\" width=13 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_6.gif\" width=2 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_13_6.gif\" width=13 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_05.gif\" width=18 height=6 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_06.gif\" width=17 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_eblack.gif\" width=13 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_21.gif\" width=2 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_eblack.gif\" width=13 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_10.gif\" width=18 height=21 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_11.gif\" width=17 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_13_6.gif\" width=13 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_6.gif\" width=2 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_13_6.gif\" width=13 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_15.gif\" width=18 height=6 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_16.gif\" width=17 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_eblack.gif\" width=13 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_21.gif\" width=2 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_eblack.gif\" width=13 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_20.gif\" width=18 height=21 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_21.gif\" width=17 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_13_6.gif\" width=13 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_6.gif\" width=2 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_13_6.gif\" width=13 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_25.gif\" width=18 height=6 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_26.gif\" width=17 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_eblack.gif\" width=13 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_21.gif\" width=2 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_eblack.gif\" width=13 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_30.gif\" width=18 height=21 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_31.gif\" width=17 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_13_6.gif\" width=13 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_6.gif\" width=2 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_13_6.gif\" width=13 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_35.gif\" width=18 height=6 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_36.gif\" width=17 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_eblack.gif\" width=13 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_21.gif\" width=2 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_eblack.gif\" width=13 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_40.gif\" width=18 height=21 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_41.gif\" width=17 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_13_6.gif\" width=13 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_6.gif\" width=2 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_13_6.gif\" width=13 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_45.gif\" width=18 height=6 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_46.gif\" width=17 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_eblack.gif\" width=13 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_21.gif\" width=2 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_eblack.gif\" width=13 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_50.gif\" width=18 height=21 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_51.gif\" width=17 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_13_6.gif\" width=13 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_6.gif\" width=2 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_13_6.gif\" width=13 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_55.gif\" width=18 height=6 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_56.gif\" width=17 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_eblack.gif\" width=13 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_21.gif\" width=2 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_eblack.gif\" width=13 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_60.gif\" width=18 height=21 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_61.gif\" width=17 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_13_6.gif\" width=13 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_6.gif\" width=2 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_13_6.gif\" width=13 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_65.gif\" width=18 height=6 ></td>"\
			"</tr>"\
		"</table>");
			fprintf(cgiOut,"</td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_03.gif\" width=63 height=30 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>");
			//ports
			//fprintf(cgiOut,"<img src=\"/images/8610/8610_12ports_04.gif\" width=63 height=168 >");
			fprintf(cgiOut,"<table id=Table_01 width=63 height=168 border=0 cellpadding=0 cellspacing=0>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_01.gif\" width=11 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_19_6.gif\" width=19 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_6.gif\" width=2 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_19_6.gif\" width=19 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_05.gif\" width=12 height=6 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_06.gif\" width=11 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_lblack.gif\" width=19 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_21.gif\" width=2 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_rblack.gif\" width=19 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_10.gif\" width=12 height=21 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_11.gif\" width=11 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_19_6.gif\" width=19 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_6.gif\" width=2 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_19_6.gif\" width=19 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_15.gif\" width=12 height=6 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_16.gif\" width=11 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_lblack.gif\" width=19 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_21.gif\" width=2 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_rblack.gif\" width=19 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_20.gif\" width=12 height=21 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_21.gif\" width=11 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_19_6.gif\" width=19 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_6.gif\" width=2 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_19_6.gif\" width=19 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_25.gif\" width=12 height=6 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_26.gif\" width=11 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_lblack.gif\" width=19 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_21.gif\" width=2 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_rblack.gif\" width=19 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_30.gif\" width=12 height=21 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_31.gif\" width=11 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_19_6.gif\" width=19 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_6.gif\" width=2 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_19_6.gif\" width=19 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_35.gif\" width=12 height=6 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_36.gif\" width=11 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_lblack.gif\" width=19 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_21.gif\" width=2 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_rblack.gif\" width=19 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_40.gif\" width=12 height=21 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_41.gif\" width=11 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_19_6.gif\" width=19 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_6.gif\" width=2 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_19_6.gif\" width=19 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_45.gif\" width=12 height=6 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_46.gif\" width=11 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_lblack.gif\" width=19 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_21.gif\" width=2 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_rblack.gif\" width=19 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_50.gif\" width=12 height=21 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_51.gif\" width=11 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_19_6.gif\" width=19 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_6.gif\" width=2 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_19_6.gif\" width=19 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_55.gif\" width=12 height=6 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_56.gif\" width=11 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_lblack.gif\" width=19 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_21.gif\" width=2 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_rblack.gif\" width=19 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_60.gif\" width=12 height=21 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_61.gif\" width=11 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_19_6.gif\" width=19 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_6.gif\" width=2 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_19_6.gif\" width=19 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_65.gif\" width=12 height=6 ></td>"\
			"</tr>"\
		"</table>");
			fprintf(cgiOut,"</td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_05_02.gif\" width=63 height=307 ></td>"\
			"</tr>"\
		"</table>");
		}
		if(0 == strcmp(board_name,"AX81_AC12C"))
		{
			//fprintf(cgiOut,"<img src=\"/images/8610/8610_12ports.gif\" width=63 height=708 >");
			fprintf(cgiOut,"<table id=Table_01 width=63 height=708 border=0 cellpadding=0 cellspacing=0>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_01.gif\" width=63 height=35 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>");
			//ports
			//fprintf(cgiOut,"<img src=\"/images/8610/8610_12ports_02.gif\" width=63 height=168 >");
			fprintf(cgiOut,"<table id=Table_01 width=63 height=168 border=0 cellpadding=0 cellspacing=0>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_01.gif\" width=17 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_13_6.gif\" width=13 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_6.gif\" width=2 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_13_6.gif\" width=13 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_05.gif\" width=18 height=6 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_06.gif\" width=17 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_eblack.gif\" width=13 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_21.gif\" width=2 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_eblack.gif\" width=13 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_10.gif\" width=18 height=21 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_11.gif\" width=17 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_13_6.gif\" width=13 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_6.gif\" width=2 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_13_6.gif\" width=13 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_15.gif\" width=18 height=6 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_16.gif\" width=17 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_eblack.gif\" width=13 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_21.gif\" width=2 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_eblack.gif\" width=13 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_20.gif\" width=18 height=21 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_21.gif\" width=17 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_13_6.gif\" width=13 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_6.gif\" width=2 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_13_6.gif\" width=13 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_25.gif\" width=18 height=6 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_26.gif\" width=17 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_eblack.gif\" width=13 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_21.gif\" width=2 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_eblack.gif\" width=13 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_30.gif\" width=18 height=21 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_31.gif\" width=17 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_13_6.gif\" width=13 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_6.gif\" width=2 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_13_6.gif\" width=13 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_35.gif\" width=18 height=6 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_36.gif\" width=17 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_eblack.gif\" width=13 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_21.gif\" width=2 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_eblack.gif\" width=13 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_40.gif\" width=18 height=21 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_41.gif\" width=17 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_13_6.gif\" width=13 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_6.gif\" width=2 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_13_6.gif\" width=13 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_45.gif\" width=18 height=6 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_46.gif\" width=17 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_eblack.gif\" width=13 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_21.gif\" width=2 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_eblack.gif\" width=13 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_50.gif\" width=18 height=21 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_51.gif\" width=17 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_13_6.gif\" width=13 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_6.gif\" width=2 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_13_6.gif\" width=13 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_55.gif\" width=18 height=6 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_56.gif\" width=17 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_eblack.gif\" width=13 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_21.gif\" width=2 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_eblack.gif\" width=13 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_60.gif\" width=18 height=21 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_61.gif\" width=17 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_13_6.gif\" width=13 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_6.gif\" width=2 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_13_6.gif\" width=13 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_02_65.gif\" width=18 height=6 ></td>"\
			"</tr>"\
		"</table>");
			fprintf(cgiOut,"</td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_03.gif\" width=63 height=30 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>");
			//ports
			//fprintf(cgiOut,"<img src=\"/images/8610/8610_12ports_04.gif\" width=63 height=168 >");
			fprintf(cgiOut,"<table id=Table_01 width=63 height=168 border=0 cellpadding=0 cellspacing=0>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_01.gif\" width=11 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_19_6.gif\" width=19 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_6.gif\" width=2 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_19_6.gif\" width=19 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_05.gif\" width=12 height=6 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_06.gif\" width=11 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_lblack.gif\" width=19 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_21.gif\" width=2 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_rblack.gif\" width=19 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_10.gif\" width=12 height=21 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_11.gif\" width=11 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_19_6.gif\" width=19 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_6.gif\" width=2 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_19_6.gif\" width=19 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_15.gif\" width=12 height=6 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_16.gif\" width=11 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_lblack.gif\" width=19 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_21.gif\" width=2 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_rblack.gif\" width=19 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_20.gif\" width=12 height=21 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_21.gif\" width=11 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_19_6.gif\" width=19 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_6.gif\" width=2 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_19_6.gif\" width=19 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_25.gif\" width=12 height=6 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_26.gif\" width=11 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_lblack.gif\" width=19 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_21.gif\" width=2 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_rblack.gif\" width=19 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_30.gif\" width=12 height=21 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_31.gif\" width=11 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_19_6.gif\" width=19 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_6.gif\" width=2 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_19_6.gif\" width=19 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_35.gif\" width=12 height=6 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_36.gif\" width=11 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_lblack.gif\" width=19 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_21.gif\" width=2 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_rblack.gif\" width=19 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_40.gif\" width=12 height=21 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_41.gif\" width=11 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_19_6.gif\" width=19 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_6.gif\" width=2 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_19_6.gif\" width=19 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_45.gif\" width=12 height=6 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_46.gif\" width=11 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_lblack.gif\" width=19 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_21.gif\" width=2 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_rblack.gif\" width=19 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_50.gif\" width=12 height=21 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_51.gif\" width=11 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_19_6.gif\" width=19 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_6.gif\" width=2 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_19_6.gif\" width=19 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_55.gif\" width=12 height=6 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_56.gif\" width=11 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_lblack.gif\" width=19 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_21.gif\" width=2 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_rblack.gif\" width=19 height=21 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_60.gif\" width=12 height=21 ></td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_61.gif\" width=11 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_19_6.gif\" width=19 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_2_6.gif\" width=2 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_19_6.gif\" width=19 height=6 ></td>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_04_65.gif\" width=12 height=6 ></td>"\
			"</tr>"\
		"</table>");
			fprintf(cgiOut,"</td>"\
			"</tr>"\
			"<tr>"\
				"<td>"\
					"<img src=\"/images/8610/8610_12ports_05_03.gif\" width=63 height=307 ></td>"\
			"</tr>"\
		"</table>");
		}
	}
}

