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
* wp_igmp_port.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system infos 
*
*
*******************************************************************************/
#include <stdio.h>
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include "ws_usrinfo.h"
#include "ws_init_dbus.h"
#include "ws_nm_status.h"
#include "ws_igmp_snp.h"
#include "ws_trunk.h"
#include "ws_dcli_portconf.h"  //端口的处理

int showPortConfig();
int cgiMain()
{
	showPortConfig();
	return 0;
}

int showPortConfig()
{
  struct list *lpublic;	/*解析public.txt文件的链表头*/
  struct list *lcontrol; 	/*解析help.txt文件的链表头*/
  lpublic=get_chain_head("../htdocs/text/public.txt");
  lcontrol=get_chain_head("../htdocs/text/control.txt"); 

	//定义并初始化
  ETH_SLOT_LIST  port_head,*p;
  ETH_PORT_LIST *pp;
  int num,show_ret=-1;

  struct igmp_port igmp_head,*ip,igp_head,*p_q,route_head,*rq;
  int tag_num,port_num,route_num;

  char *port=(char *)malloc(N);  //单个取端口的值
  
  
  int op_ret;
  char *encry=(char *)malloc(BUF_LEN);	
  
  char *IDForSearch=(char *)malloc(10);
  char *str;
  int i,k;
  k=0;
  char port_encry[BUF_LEN];
  unsigned short IDTemp;
  
  char *vlan_id=(char *)malloc(10);  //字符串型
  
  //struct vlan_info 	v_infoByVID;
  int ableNum=0;
  
  
  int result,result1=-1,result2=-1,result3=-1;  //多选的操作  
  //int invalid; 
  int untagNum,tagNum; //tag和untag的条目

  
  char VID[10]="";
  unsigned short IDSub=0;   //vid的格式
	
 
  char TagTemp[10]="";
  char TagTempLater[10]="";
  char TagTempBuf[10]="";


  //char *flavors[52];
	
  char numTempLater[4]="";
  char numTemp[4]="";
  //int ableNumLater=0;

	////刚进来的时候，没进行任何的操作
	  if((cgiFormSubmitClicked("add_port") != cgiFormSuccess) && (cgiFormSubmitClicked("delete_port") != cgiFormSuccess)&& (cgiFormSubmitClicked("submit_igmp") != cgiFormSuccess) && (cgiFormSubmitClicked("submit_igmp_disable") != cgiFormSuccess)&& (cgiFormSubmitClicked("add_route") != cgiFormSuccess) && (cgiFormSubmitClicked("del_route") != cgiFormSuccess))
	  {
		memset(encry,0,BUF_LEN);
		memset(IDForSearch,0,10);
		memset(TagTemp,0,10);
		cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
		cgiFormStringNoNewlines("VID", IDForSearch, 10); 
		cgiFormStringNoNewlines("Tag", TagTemp, 10);  
	
		str=dcryption(encry);
		if(str==NULL)
		{
		  ShowErrorPage(search(lpublic,"ill_user"));		   /*用户非法*/
		  return 0;
		}
		memset(port_encry,0,BUF_LEN);					/*清空临时变量*/
		memset(VID,0,10);
		memset(TagTempLater,0,10);
		memset(numTempLater,0,10);
	  }

  cgiFormStringNoNewlines("encry_port",port_encry,BUF_LEN);
  cgiFormStringNoNewlines("VId",VID,10); 
  
  cgiFormStringNoNewlines("encry_Num",numTempLater,10);
  cgiFormStringNoNewlines("Tag_Temp",TagTempLater,10);
  
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>IGMP SNOOPING</title>");
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  "</head>"\
  "<body>");


   //如此拷贝有什么用呢
	if((cgiFormSubmitClicked("submit_igmp") == cgiFormSuccess) || (cgiFormSubmitClicked("submit_igmp_disable") == cgiFormSuccess) || (cgiFormSubmitClicked("add_route") == cgiFormSuccess) || (cgiFormSubmitClicked("del_route") == cgiFormSuccess))
	{
		strcpy(TagTempBuf,TagTempLater);
	}
	if((cgiFormSubmitClicked("add_port") != cgiFormSuccess) && (cgiFormSubmitClicked("delete_port") != cgiFormSuccess)&& (cgiFormSubmitClicked("submit_igmp") != cgiFormSuccess) && (cgiFormSubmitClicked("submit_igmp_disable") != cgiFormSuccess)&& (cgiFormSubmitClicked("add_route") != cgiFormSuccess) && (cgiFormSubmitClicked("del_route") != cgiFormSuccess))
	{
		  strcpy(TagTempBuf,TagTemp);
	}
	else if(cgiFormSubmitClicked("add_port") == cgiFormSuccess) 	 /*右移添加端口操作*/
	  { 
		  strcpy(TagTempBuf,TagTempLater);		  
		  IDSub=atoi(VID);
		  
		  ccgi_dbus_init(); 
          result = cgiFormStringNoNewlines("can_operation",port,N);


		  if (result == cgiFormNotFound) 
		  {
			  	ShowAlert(search(lcontrol,"Not_Select"));
		  }
		  else  		 
    	  { 		
    
    		  ////////////函数没有做处理
    		   
    		   if(0==strcmp(TagTempBuf,"Untag"))
    		      op_ret = add_delete("add",port,"untag",IDSub);
    		   else
    			   op_ret = add_delete("add",port,"tag",IDSub);
    		   
    		   switch(op_ret)
    		   	{
    
    			 case CMD_SUCCESS:
    			 	ShowAlert(search(lpublic,"oper_succ"));
    				break;
                 case IGMP_UNKNOW_PORT: //未知端口形式
    			 ShowAlert(search(lcontrol,"unknown_portno_format"));
    				break;
    			 case IGMP_ERR_PORT: //错误的端口形式
    			 ShowAlert(search(lcontrol,"error_slotno_format"));
                    break;
    			 case IGMP_ERR_PORT_TAG: //错误的tag模式
                 ShowAlert(search(lcontrol,"error_tag_param"));
    			    break;
                     case IGMP_CON_ADMIN: //联系管理员
                 ShowAlert(search(lpublic,"contact_adm"));
    			    break;
    		     case IGMP_HW:
    			 ShowAlert(search(lcontrol,"HW_error"));	
    			    break;
    			 case IGMP_PORT_EXISTS:
    			 ShowAlert(search(lcontrol,"port_AlreadyExist"));
    			    break;
    			 case IGMP_PORT_NOTEXISTS:
    			 ShowAlert(search(lcontrol,"port_NotExist"));
    			    break;
                 case IGMP_BADPARAM:
    			 	ShowAlert(search(lcontrol,"parse_index_error"));
    				break;
    			 case IGMP_PORT_NOTMEMBER:
    			 	ShowAlert(search(lcontrol,"tag_only"));		
    				break;
    			 case IGMP_TAG_CONFLICT:
    			 	ShowAlert(search(lcontrol,"tagmode_not_match"));
    				break;
    			 case IGMP_NOT_TRUNK:
    			 	ShowAlert(search(lcontrol,"port_member_trunk"));
    				break;
    			 case IGMP_HAS_ARP:
    			 	ShowAlert(search(lcontrol,"port_static_arp"));
    				break;
    			 case IGMP_PVLAN:
    			 	ShowAlert(search(lcontrol,"prot_member_pvlan"));
    				break;
    			
    			 default :
    			 	ShowAlert(search(lpublic,"oper_fail"));
    				break;
    
    		   	}
    		   
    		 }
			 
	  }
	else if(cgiFormSubmitClicked("delete_port") == cgiFormSuccess)	   /*左移删除端口操作*/
	  {
		  strcpy(TagTempBuf,TagTempLater);
		  IDSub=atoi(VID);
		  untagNum=0;tagNum=0;ableNum=0;k=0;

		  ccgi_dbus_init(); 
		  result = cgiFormStringNoNewlines("havebeen_operation",port,N);

		 	
		  if (result == cgiFormNotFound) 
			  {
			  ShowAlert(search(lcontrol,"Not_Select"));
			  }
		  else
		  {
		  if(0==strcmp(TagTempBuf,"Untag"))
		      op_ret = add_delete("delete",port,"untag",IDSub);
		   else
              op_ret = add_delete("delete",port,"tag",IDSub);
		   switch(op_ret)
		   	{

			 case CMD_SUCCESS:
			 	ShowAlert(search(lpublic,"oper_succ"));
				break;
             case IGMP_UNKNOW_PORT: //未知端口形式
			 ShowAlert(search(lcontrol,"unknown_portno_format"));
				break;
			 case IGMP_ERR_PORT: //错误的端口形式
			 ShowAlert(search(lcontrol,"error_slotno_format"));
                break;
			 case IGMP_ERR_PORT_TAG: //错误的tag模式
             ShowAlert(search(lcontrol,"error_tag_param"));
			    break;
                 case IGMP_CON_ADMIN: //联系管理员
             ShowAlert(search(lpublic,"contact_adm"));
			    break;
		     case IGMP_HW:
			 ShowAlert(search(lcontrol,"HW_error"));	
			    break;
			 case IGMP_PORT_EXISTS:
			 ShowAlert(search(lcontrol,"port_AlreadyExist"));
			    break;
			 case IGMP_PORT_NOTEXISTS:
			 ShowAlert(search(lcontrol,"port_NotExist"));
			    break;
             case IGMP_BADPARAM:
			 	ShowAlert(search(lcontrol,"parse_index_error"));
				break;
			 case IGMP_PORT_NOTMEMBER:
			 	ShowAlert(search(lcontrol,"tag_only"));		
				break;
			 case IGMP_TAG_CONFLICT:
			 	ShowAlert(search(lcontrol,"tagmode_not_match"));
				break;
			 case IGMP_NOT_TRUNK:
			 	ShowAlert(search(lcontrol,"port_member_trunk"));
				break;
			 case IGMP_HAS_ARP:
			 	ShowAlert(search(lcontrol,"port_static_arp"));
				break;
			 case IGMP_PVLAN:
			 	ShowAlert(search(lcontrol,"prot_member_pvlan"));
				break;
			
			 default :
			 	ShowAlert(search(lpublic,"oper_fail"));
				break;

		   	}
		  }
			 
		  
	  }

  fprintf(cgiOut,"<form method=post>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom background=/images/di22.jpg><font id=titleen>IGMP SNOOPING</font></td>"\
    "<td width=690 align=right valign=bottom background=/images/di22.jpg>");

	  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
      "<tr>");

	  
	   if((cgiFormSubmitClicked("add_port") != cgiFormSuccess) && (cgiFormSubmitClicked("delete_port") != cgiFormSuccess)&& (cgiFormSubmitClicked("submit_igmp") != cgiFormSuccess) && (cgiFormSubmitClicked("submit_igmp_disable") != cgiFormSuccess)&& (cgiFormSubmitClicked("add_route") != cgiFormSuccess) && (cgiFormSubmitClicked("del_route") != cgiFormSuccess))
	   	{
        	fprintf(cgiOut,"<td width=62 align=left><a href=wp_igmp_vlan.cgi?UN=%s&VID=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,IDForSearch,search(lpublic,"img_ok"));
        	fprintf(cgiOut,"<td width=62 align=left><a href=wp_igmp_vlan.cgi?UN=%s&VID=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,IDForSearch,search(lpublic,"img_cancel"));
       	}
	  else
	  	{
	  		fprintf(cgiOut,"<td width=62 align=left><a href=wp_igmp_vlan.cgi?UN=%s&VID=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",port_encry,VID,search(lpublic,"img_ok"));
 			fprintf(cgiOut,"<td width=62 align=left><a href=wp_igmp_vlan.cgi?UN=%s&VID=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",port_encry,VID,search(lpublic,"img_cancel"));
 		}
	  fprintf(cgiOut,"</tr>"\
      "</table>");

	fprintf(cgiOut,"</td>"\
    "<td width=74 align=right valign=top background=/images/di22.jpg><img src=/images/youce3.jpg width=31 height=30/></td>"\
  "</tr>"\
  "<tr>"\
    "<td colspan=5 align=center valign=middle><table width=976 border=0 cellpadding=0 cellspacing=0 bgcolor=#f0eff0>"\
      "<tr>"\
        "<td width=12 align=left valign=top background=/images/di888.jpg>&nbsp;</td>"\
        "<td width=948><table width=947 border=0 cellspacing=0 cellpadding=0>"\
            "<tr height=4 valign=bottom>"\
              "<td width=120>&nbsp;</td>"\
              "<td width=827 valign=bottom><img src=/images/bottom_05.gif width=827 height=4/></td>"\
            "</tr>"\
            "<tr>"\
              "<td><table width=120 border=0 cellspacing=0 cellpadding=0>"\
                   "<tr height=25>"\
                    "<td id=tdleft>&nbsp;</td>"\
                  "</tr>");
            		fprintf(cgiOut,"<tr height=26>"\
            		   "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s<font></td>",search(lpublic,"menu_san"),search(lcontrol,"port_configure"));   /*突出显示*/
            		 fprintf(cgiOut,"</tr>");
					  for(i=0;i<12;i++)
					  {
						fprintf(cgiOut,"<tr height=25>"\
						  "<td id=tdleft>&nbsp;</td>"\
						"</tr>");
					  }


				  fprintf(cgiOut,"</table>"\
                  "</td>");
				  
                  fprintf(cgiOut,"<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">"\
					  	"<table width=350 height=150 border=0 cellpadding=0 cellspacing=0>"\
						"<tr>"\
						"<td align=\"center\" width=140>"\
						"<table height=120 width=140 border=1 cellspacing=0 cellpadding=0>");
						if((cgiFormSubmitClicked("add_port") != cgiFormSuccess) && 
							(cgiFormSubmitClicked("delete_port") != cgiFormSuccess)&& (cgiFormSubmitClicked("submit_igmp") != cgiFormSuccess) && 
							(cgiFormSubmitClicked("submit_igmp_disable") != cgiFormSuccess)&& (cgiFormSubmitClicked("add_route") != cgiFormSuccess) && 
							(cgiFormSubmitClicked("del_route") != cgiFormSuccess))
							{
							IDTemp= atoi(IDForSearch);
							strcpy(vlan_id,IDForSearch);
							}
						else 
							{
							IDTemp=atoi(VID);
							strcpy(vlan_id,VID);
							}
						

					/*------------------------ left begin---------------------------------------------------*/

						if(0==strcmp(TagTempBuf,"Untag"))	   /*因为可用untag端口直接查找default vlan就行，对应配置default vlan可用和已用都有,所以应排除*/
							{							
							fprintf(cgiOut,"<tr height=30>"\
							"<td style=\"font-size:12px\" bgcolor=#eaeff9 align=\"center\">%s(Slot/Port)</td>",search(lcontrol,"can_allocat_untag"));
							fprintf(cgiOut,"</tr>");
							}
						else if(0==strcmp(TagTempBuf,"Tag"))
							{
							fprintf(cgiOut,"<tr height=30>");			
							fprintf(cgiOut,"<td style=\"font-size:12px\" bgcolor=#eaeff9 align=\"center\">%s(Slot/Port)</td>",search(lcontrol,"can_allocat_tag"));
							fprintf(cgiOut,"</tr>");
							}


							{														
							fprintf(cgiOut,"<tr>"\
							"<td align=\"center\">");							
						   fprintf(cgiOut, "<select style=\"width:140\" style=\"height:160;background-color:#ebfff9\"  name=\"can_operation\" multiple>\n");
                           /////////////////////////////

                            ccgi_dbus_init();		 //初始化dbus
                            show_ret=show_ethport_list(&port_head,&num);
							if(show_ret==0)
       						{

                                   p=port_head.next;
                                   if(p!=NULL)
                                   {
                                       while(p!=NULL)
                                       {
                                       pp=p->port.next;
                                       while(pp!=NULL)
                                       {
                                       if(p->slot_no!=0)
                                       {                           
           	                        fprintf(cgiOut,"<option value=\"%d/%d\">%d/%d</option>\n",p->slot_no,pp->port_no,p->slot_no,pp->port_no);
                                       
                                       }
                                       pp=pp->next;
                                       }
                                       p=p->next;
                                       }
                                  }	
       						}
                          //////////////////////////////
						   fprintf(cgiOut, "</select>"\
						     "</td>"\
						     "</tr>");

							}


			/*--------------------------------left end ----------------------------------------*/
				
						fprintf(cgiOut,"</table>"\
						"</td>"\
						"<td align=\"center\">"\
						"<table width=70 border=0 cellspacing=0 cellpadding=0>"\
						"<tr>"\
						"<td align=center>");			
						fprintf(cgiOut,"<input type=submit name=add_port  onclick=\"return confirm('%s')\"	style=\"width:49px;height:49px;background:url(/images/right.gif)  left top no-repeat; border=0 \" value="">",search(lcontrol,"add_port_confirm"));
                		fprintf(cgiOut,"</td>"\
						"</tr>"\
						"<tr>"\
						"<td align=center>");						
						fprintf(cgiOut,"<input type=submit name=delete_port onclick=\"return confirm('%s')\"  style=\"width:49px;height:49px;background:url(/images/left.gif)  left top no-repeat; border=0\"	value="">",search(lcontrol,"delete_port_confirm"));
						fprintf(cgiOut,"</td>"\
						"</tr>"\
						"</table>"\
						"</td>"\
						
						"<td align=\"center\" width=140 >"\
						"<table width=140 height=170 border=1 cellspacing=0 cellpadding=0>"\
						"<tr height=30>");


						fprintf(cgiOut,"<td style=\"font-size:12px\"  bgcolor=#eaeff9 align=\"center\">%s(Slot/Port)</td>",search(lcontrol,"have_allocat_untag"));
						fprintf(cgiOut,"</tr>"\
						"<tr>"\
						"<td align=\"center\" colspan=4>");
						fprintf(cgiOut, "<select style=\"width:140\" style=\"height:160;background-color:#ebfff9\" name=\"havebeen_operation\" multiple>\n");

						/*-----------right begin------------------------------------------*/

						if(0==strcmp(TagTempBuf,"Untag"))
						{
						
						//////////////////
						
                            ccgi_dbus_init();		 //初始化dbus
                            result1=show_vlan_port_member(vlan_id,&igmp_head, &tag_num);	
							
							
							
                            if(result1 ==0)
    						{


    						  for(ip=igmp_head.next;ip!=NULL;ip=ip->next)
                               { 
                                    if(ip->un_slot!=0)
                                    {                           
        	                        fprintf(cgiOut,"<option value=\"%d/%d\">%d/%d</option>\n",ip->un_slot,ip->un_port,ip->un_slot,ip->un_port);
                                    }
                               }
                            }
							else
							{
                                fprintf(cgiOut,"<option></option>\n");
							}
                           	
						//////////////////
						}
						else
						{						
						//////////////////

                            ccgi_dbus_init();		 //初始化dbus
                            result1=show_vlan_port_member(vlan_id,&igmp_head, &tag_num);
														
							

							if(result1 ==0)
     					    {


     							for(ip=igmp_head.next;ip!=NULL;ip=ip->next)
     							{
                                     if(ip->slot!=0)
                                     {                           
         	                        fprintf(cgiOut,"<option value=\"%d/%d\">%d/%d</option>\n",ip->slot,ip->port,ip->slot,ip->port);
                                                        
                                     }
     							}
     					    }
							else
							{
                                fprintf(cgiOut,"<option></option>\n");
							}
    
						//////////////////
						}

	
						/*---------------------------------right end -------------------------------------*/
						
						fprintf(cgiOut, "</select>"\
						"</td>"\
						"</tr>");
						
						fprintf(cgiOut,"</table>"\
						"</td>"\
					  "</tr>");

						
					
						//Add by qiandawei 2008.07.25
                        ///////////////  igmp port  ,selected 
						fprintf(cgiOut,"<tr style=\"padding-left:0px; padding-top:30px\"><td colspan=3 align =left>"\
						"<table width=350 border=0 cellspacing=0 cellpadding=0>");
						fprintf(cgiOut,"<tr height=30>");
						fprintf(cgiOut,"<td width=100>%s:</td>","IGMP SNOOPING");
						fprintf(cgiOut,"<td width=70><select name=portno style=width:138px>"); 
            

							if(0==strcmp(TagTempBuf,"Untag"))
      						{
      							  ccgi_dbus_init();		 //初始化dbus
                                  result2=show_vlan_port_member(vlan_id,&igp_head, &port_num);							
                                  if(result2 ==0 )
      						      {
      						       for(p_q=igp_head.next;p_q!=NULL;p_q=p_q->next)
                                         { 
                                             if(p_q->un_slot!=0)
                                             {                           
                 	                        fprintf(cgiOut,"<option value=\"%d/%d\">%d/%d</option>\n",p_q->un_slot,p_q->un_port,p_q->un_slot,p_q->un_port);
                                             }
                                         }
                                  }
      								
      						}
							else
     						{
     								
     							ccgi_dbus_init();		 //初始化dbus
                                result2=show_vlan_port_member(vlan_id,&igp_head, &port_num);
								if(result2 == 0)
    							{
         							for(p_q=igp_head.next;p_q!=NULL;p_q=p_q->next)
         							{
                                         if(p_q->slot!=0)
                                         {                           
             	                           fprintf(cgiOut,"<option value=\"%d/%d\">%d/%d</option>\n",p_q->slot,p_q->port,p_q->slot,p_q->port);
                                         }
         							}
    							}
     						}	

						  	fprintf(cgiOut,"</select></td>"\
						  	"<td align=left style=padding-left:10px><input type=submit style=width:50px; height:36px  border=0 name=submit_igmp style=background-image:url(/images/SubBackGif.gif) value=\"%s\"></td>"\
							"<td align=left style=padding-left:10px><input type=submit style=width:50px; height:36px  border=0 name=submit_igmp_disable style=background-image:url(/images/SubBackGif.gif) value=\"%s\"></td>",search(lcontrol,"start"),search(lcontrol,"stop"));
                      ///////////////// end 

						fprintf(cgiOut,"</tr></table></td></tr>");
						char portno[N];
						memset(portno,0,N);
						cgiFormStringNoNewlines("portno",portno,N);
						int vlanid;
						int ret = -1;
						vlanid = strtoul(VID,0,10);
						if(cgiFormSubmitClicked("submit_igmp") == cgiFormSuccess)
						{
							strcpy(TagTempBuf,TagTempLater);//add 14:10
							ret = config_igmp_snp_npd_port(vlanid,"enable",portno);//开启vlan端口的igmp功能

							switch(ret)
							{
								case 0:
									ShowAlert(search(lpublic,"oper_succ"));//开启成功
									break;
								case 4:
									ShowAlert(search(lcontrol,"igmp_no_sta_g"));//全局未开启igmp
									break;
								case 8:
									ShowAlert(search(lcontrol,"igmp_port_sta"));//此端口已开启igmp
									break;
								case 70:
									ShowAlert(search(lcontrol,"igmp_no_sta_vlan"));//此VLAN不支持IGMP功能
									break;								
								default:
									ShowAlert(search(lpublic,"oper_fail"));//开启失败
							}
						}
						if(cgiFormSubmitClicked("submit_igmp_disable") == cgiFormSuccess)
						{
							strcpy(TagTempBuf,TagTempLater);//add 14:10
							ret = config_igmp_snp_npd_port(vlanid,"disable",portno);//关闭vlan端口的igmp功能
							switch(ret)
							{
								case 0:
									ShowAlert(search(lpublic,"oper_succ"));//关闭成功
									break;	
								case 4:
									ShowAlert(search(lcontrol,"igmp_no_sta_g"));//全局未开启igmp
									break;
								case 7:
									ShowAlert(search(lcontrol,"igmp_port_no_sta"));//此端口未开启igmp
									break;
								case 70:
									ShowAlert(search(lcontrol,"igmp_no_sta_vlan"));//此VLAN不支持IGMP功能
									break;								
								default:
									ShowAlert(search(lpublic,"oper_fail"));//开启失败
							}
						}

                       /////////////////// route port  selected 
						fprintf(cgiOut,"<tr style=\"padding-left:0px; padding-top:30px\"><td colspan=3 align =left>"\
						"<table width=350 border=0 cellspacing=0 cellpadding=0>");
						fprintf(cgiOut,"<tr height=30>");
						  fprintf(cgiOut,"<td width=100>%s:</td>",search(lcontrol,"route_port"));
						  fprintf(cgiOut,"<td width=70><select name=routeport style=width:138px>");

						  
						  if(0==strcmp(TagTempBuf,"Untag"))
						  {
      							  ccgi_dbus_init();		 //初始化dbus
                                  result3=show_vlan_port_member(vlan_id,&route_head, &route_num);							
                                  if(result3 ==0)
                                  {
          						      for(rq=route_head.next;rq!=NULL;rq=rq->next)
                                      { 
                                          if(rq->un_slot!=0)
                                          {                           
              	                        fprintf(cgiOut,"<option value=\"%d/%d\">%d/%d</option>\n",rq->un_slot,rq->un_port,rq->un_slot,rq->un_port);
                                          }
                                      }
                                   }
      								
      						}
						  else
						  {
     								
     							ccgi_dbus_init();		 //初始化dbus
                                result3=show_vlan_port_member(vlan_id,&route_head, &route_num);	
								if(result3 == 0)
    							{
         							for(rq=route_head.next;rq!=NULL;rq=rq->next)
         							{
                                         if(rq->slot!=0)
                                         {                           
             	                           fprintf(cgiOut,"<option value=\"%d/%d\">%d/%d</option>\n",rq->slot,rq->port,rq->slot,rq->port);
                                         }
         							}
    							}
     						}   

							  fprintf(cgiOut,"</select></td>"\
							  "<td align=left style=padding-left:10px><input type=submit style=width:50px; height:36px	border=0 name=add_route style=background-image:url(/images/SubBackGif.gif) value=\"%s\"></td>"\
							  "<td align=left style=padding-left:10px><input type=submit style=width:50px; height:36px	border=0 name=del_route style=background-image:url(/images/SubBackGif.gif) value=\"%s\"></td>",search(lcontrol,"add_route_port"),search(lcontrol,"del_route_port"));
						  fprintf(cgiOut,"</tr></table></td></tr>");

 /////////////////// end route ports 
 
						char routeport[N];
						memset(routeport,0,N);
						cgiFormStringNoNewlines("routeport",routeport,N);
						//int vlanid;
						ret = -1;
						//vlanid = strtoul(VID,0,10);
						if(cgiFormSubmitClicked("add_route") == cgiFormSuccess)
						{
							strcpy(TagTempBuf,TagTempLater);//add 14:10
							ret = igmp_snp_mcroute_port_add_del(vlanid,1,routeport);//添加vlan的路由端口

							switch(ret)
							{
								case 0:
									ShowAlert(search(lpublic,"oper_succ"));
									break;
								case 3:
									ShowAlert(search(lcontrol,"igmp_port_no_sta"));
									break;	
								case 4:
									ShowAlert(search(lcontrol,"route_port_exist"));
									break;					
								default:
									ShowAlert(search(lpublic,"oper_fail"));
							}
						}
						if(cgiFormSubmitClicked("del_route") == cgiFormSuccess)
						{
							strcpy(TagTempBuf,TagTempLater);//add 14:10
							ret = igmp_snp_mcroute_port_add_del(vlanid,0,routeport);//删除vlan的路由端口
							switch(ret)
							{
								case 0:
									ShowAlert(search(lpublic,"oper_succ"));
									break;	
								case 3:
									ShowAlert(search(lcontrol,"igmp_port_no_sta"));
									break;	
								case 6:
									ShowAlert(search(lcontrol,"route_port_no"));
									break;	
								case 5:
									ShowAlert(search(lcontrol,"no_route_port"));
									break;	
								default:
									ShowAlert(search(lpublic,"oper_fail"));
							}
						}						
						
						//end
						fprintf(cgiOut,"<tr>");
										sprintf(numTemp,"%d",ableNum);
										if(((cgiFormSubmitClicked("add_port") != cgiFormSuccess) && 
											(cgiFormSubmitClicked("delete_port") != cgiFormSuccess)&& (cgiFormSubmitClicked("submit_igmp") != cgiFormSuccess) && 
											(cgiFormSubmitClicked("submit_igmp_disable") != cgiFormSuccess)&& (cgiFormSubmitClicked("add_route") != cgiFormSuccess) && 
											(cgiFormSubmitClicked("del_route") != cgiFormSuccess)))
									  {
									  
										fprintf(cgiOut,"<td ><input type=hidden name=encry_port value=%s></td>",encry);
										fprintf(cgiOut,"<td ><input type=hidden name=encry_Num value=%s></td>",numTemp);
										fprintf(cgiOut,"<td ><input type=hidden name=Tag_Temp value=%s></td>",TagTemp);
										fprintf(cgiOut,"<td ><input type=hidden name=VId value=%s></td>",IDForSearch);
									  }
									  else
									  {
										fprintf(cgiOut,"<td ><input type=hidden name=encry_port value=%s></td>",port_encry);
										fprintf(cgiOut,"<td ><input type=hidden name=encry_Num value=%s></td>",numTempLater);
										fprintf(cgiOut,"<td ><input type=hidden name=Tag_Temp value=%s></td>",TagTempLater);
										fprintf(cgiOut,"<td ><input type=hidden name=VId value=%s></td>",VID);
									  }
									  fprintf(cgiOut,"</tr>"\
					"</table>"\
              "</td>"\
            "</tr>"\
            "<tr height=4 valign=top>"\
              "<td width=120 height=4 align=right valign=top><img src=/images/bottom_07.gif width=1 height=10/></td>"\
              "<td width=827 height=4 valign=top bgcolor=#FFFFFF><img src=/images/bottom_06.gif width=827 height=15/></td>"\
            "</tr>"\
          "</table>"\
        "</td>"\
        "<td width=15 background=/images/di999.jpg>&nbsp;</td>"\
      "</tr>"\
    "</table></td>"\
  "</tr>"\
  "<tr>"\
    "<td colspan=3 align=left valign=top background=/images/di777.jpg><img src=/images/di555.jpg width=61 height=62/></td>"\
    "<td align=left valign=top background=/images/di777.jpg>&nbsp;</td>"\
    "<td align=left valign=top background=/images/di777.jpg><img src=/images/di666.jpg width=74 height=62/></td>"\
  "</tr>"\
"</table>"\
"</div>"\
"</form>"\
"</body>"\
"</html>");  
	
free(IDForSearch);
free(encry);
free(vlan_id);
free(port);
release(lpublic);  
release(lcontrol);

/////释放
if((show_ret==0)&&(num>0))
{
	Free_ethslot_head(&port_head);
}	

//释放链表  right
if((result1==0)&&(tag_num>0))
  Free_igmp_port(&igmp_head);

//释放链表  port
if((result2==0)&&(port_num>0))
  Free_igmp_port(&igp_head);

//释放链表  route
if((result3==0)&&(route_num>0))
  Free_igmp_port(&route_head);


return 0;
}
													
