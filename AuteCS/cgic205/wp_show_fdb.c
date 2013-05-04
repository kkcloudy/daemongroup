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
* wp_show_fdb.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* system contrl for fdb list
*
*
*******************************************************************************/
#include <stdio.h>
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include "ws_usrinfo.h"
#include "ws_ec.h"
#include "ws_err.h"
#include "ws_init_dbus.h"
#include "ws_fdb.h"

int ShowFDBPage(struct list *lpublic,struct list *lcon);

int cgiMain()
{
	struct list *lpublic;	/*解析public.txt文件的链表头*/
	struct list *lcon;	   /*解析control.txt文件的链表头*/	
	lpublic=get_chain_head("../htdocs/text/public.txt");
	lcon=get_chain_head("../htdocs/text/control.txt");
	ccgi_dbus_init();
    ShowFDBPage(lpublic,lcon);
	release(lpublic);  
	release(lcon);

 	return 0;
}

int ShowFDBPage(struct list *lpublic,struct list *lcon)
{
	char *encry=(char *)malloc(BUF_LEN);				/*存储从wp_usrmag.cgi带入的加密字符串*/
	char *str=NULL;

    /*----------------- b b -----------------*/
	char *PNtemp=(char *)malloc(10);
    char *SNtemp=(char *)malloc(10);
	int pageNum=0;   
	char * pageNumCA=(char *)malloc(10);
	memset(pageNumCA,0,10);
	char * pageNumCD=(char *)malloc(10);
	memset(pageNumCD,0,10);
	char * pageNumCF=(char *)malloc(10);//first page
	memset(pageNumCF,0,10);
	char * pageNumCL=(char *)malloc(10);//last page
	memset(pageNumCL,0,10);

    char ID[N];
	memset(ID,0,N); //那个type

	char VID[N];
	memset(VID,0,N); //那个type

	char TY[N];
	memset(TY,0,N);  //vlan的type

	char VN[N];
	memset(VN,0,N);  //vlan的name

	char MAC[N];
	memset(MAC,0,N);  //vlan的mac

	char PT[N];
	memset(PT,0,N);  //vlan的port

	struct fdb_all f_head,*fq,s_head,*sq,d_head,*dq;  //all,static,dynamic
	int all_num=0,s_num=0,d_num=0;

	struct fdb_all vid_head,*vq; // vlanid
	int vid_num=0;

	struct fdb_all pt_head,*pq;  //port
	int p_num=0;

    struct fdb_blacklist bk_head,*bkq;  //blacklist
	int bk_num=0;

	struct fdb_all m_head,*mq;
	int m_num=0;

    struct fdb_limit lm_head,*lq;
	int l_num=0;

	struct fdb_mac_vid macvid;
	memset(&macvid,0,sizeof(macvid));

	
	int FirstPage=0,LastPage=0;
	int cl=1;
	int xnt=0,head=0,tail=0;

    int result1 = 0;
    int result2 = 0;
    int result3 = 0;
	int result4 = 0;
	int result5 = 0;
	int result6 = 0;
    int result7 = 0;
    int result8 = 0;
	int result9 = 0;
	/*---------------- e e -------------------*/
	
	char fdb_encry[BUF_LEN]; 
	char showtype[N],vlantype[N],vlanid[N],vlanname[N],mac[N],port[N];
	int i,ret;
	int fdbcount = 0;
	int retu=0;
	char * CheckUsr=(char *)malloc(10);
	memset(CheckUsr,0,10);
	
	if(cgiFormSubmitClicked("submit_fdb") != cgiFormSuccess)
	{
			 memset(encry,0,BUF_LEN);
			 cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
			 str=dcryption(encry);
			 if(str==NULL)
			 {
				   ShowErrorPage(search(lpublic,"ill_user"));	 /*用户非法*/
				   return 0;
			 }
			 memset(fdb_encry,0,BUF_LEN); 				  /*清空临时变量*/
	}
	memset(fdb_encry,0,BUF_LEN); 				  /*清空临时变量*/
    cgiFormStringNoNewlines("fdb_encry",fdb_encry,BUF_LEN);
	cgiFormStringNoNewlines("CheckUsr",CheckUsr,10);
   /*----------- b b ------------------*/
	memset(PNtemp,0,10);
	cgiFormStringNoNewlines("PN",PNtemp,10);
	pageNum=atoi(PNtemp);
	memset(SNtemp,0,10);
	cgiFormStringNoNewlines("SN",SNtemp,10);
   /*----------- e e ------------------*/
		
	

	if(strcmp(CheckUsr,"")!=0)
		retu=atoi(CheckUsr);
	cgiHeaderContentType("text/html");
	fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
	fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
	fprintf(cgiOut,"<title>%s</title>",search(lcon,"fdb_man"));
	fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
	  "<style type=text/css>"\
	  ".a3{width:30;border:0; text-align:center}"\
	  "</style>"\
	"</head>"\
	"<script src=/ip.js>"\
	"</script>"\
		"<script type='text/javascript'>"\
		"function changestate(){"\
			"var all = document.getElementsByName('showtype')[0];"\
			"var sta = document.getElementsByName('showtype')[1];"\
			"var dyna = document.getElementsByName('showtype')[2];"\
			"var black = document.getElementsByName('showtype')[3];"\
			"var vlan = document.getElementsByName('showtype')[4];"\
			"var vlanmac = document.getElementsByName('showtype')[5];"\
			"var showmac = document.getElementsByName('showtype')[6];"\
			"var showport = document.getElementsByName('showtype')[7];"\
			"var showlimit = document.getElementsByName('showtype')[8];"\
			"var vlanid = document.getElementsByName('vlanid')[0];"\
			"var vlanname = document.getElementsByName('vlanname')[0];"\
			"var port = document.getElementsByName('port')[0];"\
			"var mac = document.getElementsByName('mac')[0];"\
			"var id = document.getElementsByName('vlantype')[0];"\
			"var name = document.getElementsByName('vlantype')[1];"\
			"if( all.checked == true || sta.checked == true || black.checked == true || dyna.checked == true || showlimit.checked == true)"\
			"{"\
				"vlanid.disabled = true;"\
				"vlanid.style.backgroundColor = '#ccc';"\
				"vlanname.disabled = true;"\
				"vlanname.style.backgroundColor = '#ccc';"\
				"mac.disabled = true;"\
				"mac.style.backgroundColor = '#ccc';"\
				"port.disabled = true;"\
				"port.style.backgroundColor = '#ccc';"\
			"}"\
			"else if( vlan.checked == true )"\
			"{"\
				"if( id.checked == true )"\
				"{"\
					"vlanname.disabled = true;"\
					"vlanname.style.backgroundColor = '#ccc';"\
					"vlanid.disabled = false;"\
					"vlanid.style.backgroundColor = '#fff';"\
				"}"\
				"else"\
				"{"\
					"vlanid.disabled = true;"\
					"vlanid.style.backgroundColor = '#ccc';"\
					"vlanname.disabled = false;"\
					"vlanname.style.backgroundColor = '#fff';"\
				"}"\
				"mac.disabled = true;"\
				"mac.style.backgroundColor = '#ccc';"\
				"port.disabled = true;"\
				"port.style.backgroundColor = '#ccc';"\
			"}"\
			"else if( showmac.checked == true )"\
			"{"\
				"vlanid.disabled = true;"\
				"vlanid.style.backgroundColor = '#ccc';"\
				"vlanname.disabled = true;"\
				"vlanname.style.backgroundColor = '#ccc';"\
				"port.disabled = true;"\
				"port.style.backgroundColor = '#ccc';"\
				"mac.disabled = false;"\
				"mac.style.backgroundColor = '#fff';"\
			"}"\
			"else if( vlanmac.checked == true )"\
			"{"\
				"if( id.checked == true )"\
				"{"\
					"vlanname.disabled = true;"\
					"vlanname.style.backgroundColor = '#ccc';"\
					"vlanid.disabled = false;"\
					"vlanid.style.backgroundColor = '#fff';"\
				"}"\
				"else"\
				"{"\
					"vlanid.disabled = true;"\
					"vlanid.style.backgroundColor = '#ccc';"\
				"}"\
				"mac.disabled = false;"\
				"mac.style.backgroundColor = '#fff';"\
				"port.disabled = true;"\
				"port.style.backgroundColor = '#ccc';"\
			"}"\
			"else if( showport.checked == true )"\
			"{"\
				"vlanid.disabled = true;"\
				"vlanid.style.backgroundColor = '#ccc';"\
				"vlanname.disabled = true;"\
				"vlanname.style.backgroundColor = '#ccc';"\
				"mac.disabled = true;"\
				"mac.style.backgroundColor = '#ccc';"\
				"port.disabled = false;"\
				"port.style.backgroundColor = '#fff';"\
			"}"\
		"}"\
		"</script>"\
	"<body>");

	
	if(cgiFormSubmitClicked("submit_fdb") != cgiFormSuccess)
	{
		retu=checkuser_group(str);
	}
	fprintf(cgiOut,"<form method=post>");
    fprintf(cgiOut,"<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0 style=overflow:auto>"\
  "<tr>"\
	"<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
	"<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
	"<td width=153 align=left valign=bottom background=/images/di22.jpg><font id=titleen>FDB</font><font id=%s> %s</font></td>",search(lpublic,"title_style"),search(lpublic,"management"));
	fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");

		//fprintf(cgiOut,"<input type=hidden name=UN  value=%s />",encry);
			
		  fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
		  "<tr>"\
		  "<td width=62 align=center><input id=but type=submit name=submit_fdb style=background-image:url(/images/%s) value=""></td>",search(lpublic,"img_ok"));		  
		  if(cgiFormSubmitClicked("submit_fdb") != cgiFormSuccess)
			fprintf(cgiOut,"<td width=62 align=left><a href=wp_contrl.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_cancel"));
		  else										   
			fprintf(cgiOut,"<td width=62 align=left><a href=wp_contrl.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",fdb_encry,search(lpublic,"img_cancel"));
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

					if(cgiFormSubmitClicked("submit_fdb") != cgiFormSuccess)
						{
							if(retu==0)  /*管理员*/
							{
    							fprintf(cgiOut,"<tr height=26>"\
    							  "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=yingwen_san>FDB </font><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcon,"show_fdb"));   /*突出显示*/
    							fprintf(cgiOut,"</tr>");
    							fprintf(cgiOut,"<tr height=25>"\
    							  "<td align=left id=tdleft><a href=wp_config_agingtime.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>FDB </font><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcon,"fdb_age"));
    							fprintf(cgiOut,"</tr>"\
    							"<tr height=25>"\
    							  "<td align=left id=tdleft><a href=wp_add_static_fdb.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san> FDB</font></a></td>",encry,search(lpublic,"menu_san"),search(lcon,"fdb_sta"));
    							fprintf(cgiOut,"</tr>"\
    							"<tr height=25>"\
    							  "<td align=left id=tdleft><a href=wp_add_blacklist.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcon,"fdb_bla"));
    							fprintf(cgiOut,"</tr>");
    							fprintf(cgiOut,"<tr height=25>"\
    							  "<td align=left id=tdleft><a href=wp_config_fdb_limit.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>FDB </font><font id=%s>%s</font></a></td>",encry,search(lpublic,"menu_san"),search(lcon,"fdb_limit"));
    							fprintf(cgiOut,"</tr>");

								 //add new web page for delete fdb
							fprintf(cgiOut,"<tr height=25>"\
    						"<td align=left id=tdleft><a href=wp_delete_fdb.cgi?UN=%s target=mainFrame class=top><font id=%s>%s FDB</font></a></td>",encry,search(lpublic,"menu_san"),search(lcon,"del"));  
    						fprintf(cgiOut,"</tr>");
							}
							else
							{
								fprintf(cgiOut,"<tr height=26>"\
    							  "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=yingwen_san>FDB </font><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcon,"show_fdb"));   /*突出显示*/
    							fprintf(cgiOut,"</tr>");
							}

						}
						else if(cgiFormSubmitClicked("submit_fdb") == cgiFormSuccess)
						{
							if(retu==0)  /*管理员*/
							{
    							fprintf(cgiOut,"<tr height=26>"\
    							  "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=yingwen_san>FDB </font><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcon,"show_fdb"));   /*突出显示*/
    							fprintf(cgiOut,"</tr>");
    							fprintf(cgiOut,"<tr height=25>"\
    							  "<td align=left id=tdleft><a href=wp_config_agingtime.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>FDB </font><font id=%s>%s</font></a></td>",fdb_encry,search(lpublic,"menu_san"),search(lcon,"fdb_age"));
    							fprintf(cgiOut,"</tr>"\
    							"<tr height=25>"\
    							  "<td align=left id=tdleft><a href=wp_add_static_fdb.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font><font id=yingwen_san> FDB</font></a></td>",fdb_encry,search(lpublic,"menu_san"),search(lcon,"fdb_sta"));
    							fprintf(cgiOut,"</tr>"\
    							"<tr height=25>"\
    							  "<td align=left id=tdleft><a href=wp_add_blacklist.cgi?UN=%s target=mainFrame class=top><font id=%s>%s</font></a></td>",fdb_encry,search(lpublic,"menu_san"),search(lcon,"fdb_bla"));
    							fprintf(cgiOut,"</tr>");
    							fprintf(cgiOut,"<tr height=25>"\
    							  "<td align=left id=tdleft><a href=wp_config_fdb_limit.cgi?UN=%s target=mainFrame class=top><font id=yingwen_san>FDB </font><font id=%s>%s</font></a></td>",fdb_encry,search(lpublic,"menu_san"),search(lcon,"fdb_limit"));
    							fprintf(cgiOut,"</tr>");

								 //add new web page for delete fdb
							fprintf(cgiOut,"<tr height=25>"\
    						"<td align=left id=tdleft><a href=wp_delete_fdb.cgi?UN=%s target=mainFrame class=top><font id=%s>%s FDB</font></a></td>",fdb_encry,search(lpublic,"menu_san"),search(lcon,"del"));  
    						fprintf(cgiOut,"</tr>");
							}
							else
							{
								fprintf(cgiOut,"<tr height=26>"\
    							"<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=yingwen_san>FDB </font><font id=%s>%s</font></td>",search(lpublic,"menu_san"),search(lcon,"show_fdb"));   /*突出显示*/
    							fprintf(cgiOut,"</tr>");
							}

						}

                  
						
						int rowscount=0;
						
						if(retu==0)  /*管理员*/
							rowscount=5;
						else rowscount=8;
						for(i=0;i<PageNumber + rowscount+1;i++)
						{
							fprintf(cgiOut,"<tr height=25>"\
							  "<td id=tdleft>&nbsp;</td>"\
							"</tr>");
						}
	
					  fprintf(cgiOut,"</table>"\
				  "</td>"\
				  "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">"\
						  "<table width=750 border=0 cellspacing=0 cellpadding=0>"\
								"<tr height=35>");
								int agetime=0;
								ret = show_fdb_agingtime(&agetime);
								
								if(retu==0)  /*管理员*/
								{
									
         								fprintf(cgiOut,"<td align=left id=sec1 style=\"border-bottom:2px solid #53868b;font-size:14px;padding-left:0px;padding-top:0px\">%s</td>",search(lcon,"sel_view"));
         							
								}
								else
								{
    								if(ret == 0)
    								{
         								
         									fprintf(cgiOut,"<td align=left id=sec1 style=\"border-bottom:2px solid #53868b;font-size:14px;padding-left:0px;padding-top:0px\">%s(%s: %ds)</td>",search(lcon,"sel_view"),search(lcon,"sel_age"),agetime);
         								
    								}
    								else if(ret == -1)
    								{
    									
         									fprintf(cgiOut,"<td align=left id=sec1 style=\"border-bottom:2px solid #53868b;font-size:14px;padding-left:0px;padding-top:0px\">%s(%s: %s)</td>",search(lcon,"sel_view"),search(lcon,"sel_age"),search(lcon,"age_err"));
         								
    								}
									else if(ret==-5)
									  ShowAlert(search(lcon,"HW_error"));

								}
										  fprintf(cgiOut,"</tr>"\
						  "</table>"\
						  "<table width=750 border=0 cellspacing=0 cellpadding=0>"\
								"<tr height=35>");

                          //////////////////

							   	  memset(ID,0,N);
								  cgiFormStringNoNewlines("ID", ID, N);   //先抓页面参数
								  
								    if(strcmp(ID,"")!=0)
										strcpy(showtype,ID);
                                    else{
									memset(showtype,0,N);
									cgiFormStringNoNewlines("showtype", showtype, N);  //抓隐藏表单
                                    	}

								                                   
							////////////////

                           							
									if(strcmp(showtype,"1")==0)
									{
		  								fprintf(cgiOut,"<td width=80><input type=\"radio\" name=\"showtype\" value=\"1\" onclick=\"changestate()\" checked>%s:</td>",search(lcon,"all"));
										fprintf(cgiOut,"<td width=80><input type=\"radio\" name=\"showtype\" value=\"2\" onclick=\"changestate()\">%s:</td>",search(lcon,"static"));
										fprintf(cgiOut,"<td width=80><input type=\"radio\" name=\"showtype\" value=\"3\" onclick=\"changestate()\">%s:</td>",search(lcon,"dynamic"));
										fprintf(cgiOut,"<td width=75><input type=\"radio\" name=\"showtype\" value=\"4\" onclick=\"changestate()\">%s:</td>",search(lcon,"blacklist"));
										fprintf(cgiOut,"<td width=70><input type=\"radio\" name=\"showtype\" value=\"5\" onclick=\"changestate()\">%s:</td>","VLAN");
										fprintf(cgiOut,"<td width=130><input type=\"radio\" name=\"showtype\" value=\"6\" onclick=\"changestate()\">%s:</td>","VLAN ID AND MAC");
										fprintf(cgiOut,"<td width=70><input type=\"radio\" name=\"showtype\" value=\"7\" onclick=\"changestate()\">%s:</td>","MAC");
										fprintf(cgiOut,"<td width=75><input type=\"radio\" name=\"showtype\" value=\"8\" onclick=\"changestate()\">%s:</td>",search(lcon,"_port"));	
										fprintf(cgiOut,"<td width=80><input type=\"radio\" name=\"showtype\" value=\"9\" onclick=\"changestate()\">%s:</td>",search(lcon,"fdb_limit"));
									}
									else if(strcmp(showtype,"2")==0)
									{
		  								fprintf(cgiOut,"<td width=80><input type=\"radio\" name=\"showtype\" value=\"1\" onclick=\"changestate()\">%s:</td>",search(lcon,"all"));
										fprintf(cgiOut,"<td width=80><input type=\"radio\" name=\"showtype\" value=\"2\" onclick=\"changestate()\" checked>%s:</td>",search(lcon,"static"));
										fprintf(cgiOut,"<td width=80><input type=\"radio\" name=\"showtype\" value=\"3\" onclick=\"changestate()\">%s:</td>",search(lcon,"dynamic"));
										fprintf(cgiOut,"<td width=75><input type=\"radio\" name=\"showtype\" value=\"4\" onclick=\"changestate()\">%s:</td>",search(lcon,"blacklist"));
										fprintf(cgiOut,"<td width=70><input type=\"radio\" name=\"showtype\" value=\"5\" onclick=\"changestate()\">%s:</td>","VLAN");
										fprintf(cgiOut,"<td width=130><input type=\"radio\" name=\"showtype\" value=\"6\" onclick=\"changestate()\">%s:</td>","VLAN ID AND MAC");
										fprintf(cgiOut,"<td width=70><input type=\"radio\" name=\"showtype\" value=\"7\" onclick=\"changestate()\">%s:</td>","MAC");
										fprintf(cgiOut,"<td width=75><input type=\"radio\" name=\"showtype\" value=\"8\" onclick=\"changestate()\">%s:</td>",search(lcon,"_port"));
										fprintf(cgiOut,"<td width=80><input type=\"radio\" name=\"showtype\" value=\"9\" onclick=\"changestate()\">%s:</td>",search(lcon,"fdb_limit"));
									}
									else if(strcmp(showtype,"3")==0)
									{
		  								fprintf(cgiOut,"<td width=80><input type=\"radio\" name=\"showtype\" value=\"1\" onclick=\"changestate()\">%s:</td>",search(lcon,"all"));
										fprintf(cgiOut,"<td width=80><input type=\"radio\" name=\"showtype\" value=\"2\" onclick=\"changestate()\">%s:</td>",search(lcon,"static"));
										fprintf(cgiOut,"<td width=80><input type=\"radio\" name=\"showtype\" value=\"3\" onclick=\"changestate()\" checked>%s:</td>",search(lcon,"dynamic"));
										fprintf(cgiOut,"<td width=75><input type=\"radio\" name=\"showtype\" value=\"4\" onclick=\"changestate()\">%s:</td>",search(lcon,"blacklist"));
										fprintf(cgiOut,"<td width=70><input type=\"radio\" name=\"showtype\" value=\"5\" onclick=\"changestate()\">%s:</td>","VLAN");
										fprintf(cgiOut,"<td width=130><input type=\"radio\" name=\"showtype\" value=\"6\" onclick=\"changestate()\">%s:</td>","VLAN ID AND MAC");
										fprintf(cgiOut,"<td width=70><input type=\"radio\" name=\"showtype\" value=\"7\" onclick=\"changestate()\">%s:</td>","MAC");
										fprintf(cgiOut,"<td width=75><input type=\"radio\" name=\"showtype\" value=\"8\" onclick=\"changestate()\">%s:</td>",search(lcon,"_port"));
										fprintf(cgiOut,"<td width=80><input type=\"radio\" name=\"showtype\" value=\"9\" onclick=\"changestate()\">%s:</td>",search(lcon,"fdb_limit"));
									}
									else if(strcmp(showtype,"4")==0)
									{
		  								fprintf(cgiOut,"<td width=80><input type=\"radio\" name=\"showtype\" value=\"1\" onclick=\"changestate()\">%s:</td>",search(lcon,"all"));
										fprintf(cgiOut,"<td width=80><input type=\"radio\" name=\"showtype\" value=\"2\" onclick=\"changestate()\">%s:</td>",search(lcon,"static"));
										fprintf(cgiOut,"<td width=80><input type=\"radio\" name=\"showtype\" value=\"3\" onclick=\"changestate()\">%s:</td>",search(lcon,"dynamic"));
										fprintf(cgiOut,"<td width=75><input type=\"radio\" name=\"showtype\" value=\"4\" onclick=\"changestate()\" checked>%s:</td>",search(lcon,"blacklist"));
										fprintf(cgiOut,"<td width=70><input type=\"radio\" name=\"showtype\" value=\"5\" onclick=\"changestate()\">%s:</td>","VLAN");
										fprintf(cgiOut,"<td width=130><input type=\"radio\" name=\"showtype\" value=\"6\" onclick=\"changestate()\">%s:</td>","VLAN ID AND MAC");
										fprintf(cgiOut,"<td width=70><input type=\"radio\" name=\"showtype\" value=\"7\" onclick=\"changestate()\">%s:</td>","MAC");
										fprintf(cgiOut,"<td width=75><input type=\"radio\" name=\"showtype\" value=\"8\" onclick=\"changestate()\">%s:</td>",search(lcon,"_port"));
										fprintf(cgiOut,"<td width=80><input type=\"radio\" name=\"showtype\" value=\"9\" onclick=\"changestate()\">%s:</td>",search(lcon,"fdb_limit"));
									}
									else if(strcmp(showtype,"5")==0)
									{
		  								fprintf(cgiOut,"<td width=80><input type=\"radio\" name=\"showtype\" value=\"1\" onclick=\"changestate()\">%s:</td>",search(lcon,"all"));
										fprintf(cgiOut,"<td width=80><input type=\"radio\" name=\"showtype\" value=\"2\" onclick=\"changestate()\">%s:</td>",search(lcon,"static"));
										fprintf(cgiOut,"<td width=80><input type=\"radio\" name=\"showtype\" value=\"3\" onclick=\"changestate()\">%s:</td>",search(lcon,"dynamic"));
										fprintf(cgiOut,"<td width=75><input type=\"radio\" name=\"showtype\" value=\"4\" onclick=\"changestate()\">%s:</td>",search(lcon,"blacklist"));
										fprintf(cgiOut,"<td width=70><input type=\"radio\" name=\"showtype\" value=\"5\" onclick=\"changestate()\" checked>%s:</td>","VLAN");
										fprintf(cgiOut,"<td width=130><input type=\"radio\" name=\"showtype\" value=\"6\" onclick=\"changestate()\">%s:</td>","VLAN ID AND MAC");
										fprintf(cgiOut,"<td width=70><input type=\"radio\" name=\"showtype\" value=\"7\" onclick=\"changestate()\">%s:</td>","MAC");
										fprintf(cgiOut,"<td width=75><input type=\"radio\" name=\"showtype\" value=\"8\" onclick=\"changestate()\">%s:</td>",search(lcon,"_port"));
										fprintf(cgiOut,"<td width=80><input type=\"radio\" name=\"showtype\" value=\"9\" onclick=\"changestate()\">%s:</td>",search(lcon,"fdb_limit"));
									}
									else if(strcmp(showtype,"6")==0)
									{
		  								fprintf(cgiOut,"<td width=80><input type=\"radio\" name=\"showtype\" value=\"1\" onclick=\"changestate()\">%s:</td>",search(lcon,"all"));
										fprintf(cgiOut,"<td width=80><input type=\"radio\" name=\"showtype\" value=\"2\" onclick=\"changestate()\">%s:</td>",search(lcon,"static"));
										fprintf(cgiOut,"<td width=80><input type=\"radio\" name=\"showtype\" value=\"3\" onclick=\"changestate()\">%s:</td>",search(lcon,"dynamic"));
										fprintf(cgiOut,"<td width=75><input type=\"radio\" name=\"showtype\" value=\"4\" onclick=\"changestate()\">%s:</td>",search(lcon,"blacklist"));
										fprintf(cgiOut,"<td width=70><input type=\"radio\" name=\"showtype\" value=\"5\" onclick=\"changestate()\">%s:</td>","VLAN");
										fprintf(cgiOut,"<td width=130><input type=\"radio\" name=\"showtype\" value=\"6\" onclick=\"changestate()\" checked>%s:</td>","VLAN ID AND MAC");
										fprintf(cgiOut,"<td width=70><input type=\"radio\" name=\"showtype\" value=\"7\" onclick=\"changestate()\">%s:</td>","MAC");
										fprintf(cgiOut,"<td width=75><input type=\"radio\" name=\"showtype\" value=\"8\" onclick=\"changestate()\">%s:</td>",search(lcon,"_port"));
										fprintf(cgiOut,"<td width=80><input type=\"radio\" name=\"showtype\" value=\"9\" onclick=\"changestate()\">%s:</td>",search(lcon,"fdb_limit"));
									}
									else if(strcmp(showtype,"7")==0)
									{
		  								fprintf(cgiOut,"<td width=80><input type=\"radio\" name=\"showtype\" value=\"1\" onclick=\"changestate()\">%s:</td>",search(lcon,"all"));
										fprintf(cgiOut,"<td width=80><input type=\"radio\" name=\"showtype\" value=\"2\" onclick=\"changestate()\">%s:</td>",search(lcon,"static"));
										fprintf(cgiOut,"<td width=80><input type=\"radio\" name=\"showtype\" value=\"3\" onclick=\"changestate()\">%s:</td>",search(lcon,"dynamic"));
										fprintf(cgiOut,"<td width=75><input type=\"radio\" name=\"showtype\" value=\"4\" onclick=\"changestate()\">%s:</td>",search(lcon,"blacklist"));
										fprintf(cgiOut,"<td width=70><input type=\"radio\" name=\"showtype\" value=\"5\" onclick=\"changestate()\">%s:</td>","VLAN");
										fprintf(cgiOut,"<td width=130><input type=\"radio\" name=\"showtype\" value=\"6\" onclick=\"changestate()\">%s:</td>","VLAN ID AND MAC");
										fprintf(cgiOut,"<td width=70><input type=\"radio\" name=\"showtype\" value=\"7\" onclick=\"changestate()\" checked>%s:</td>","MAC");
										fprintf(cgiOut,"<td width=75><input type=\"radio\" name=\"showtype\" value=\"8\" onclick=\"changestate()\">%s:</td>",search(lcon,"_port"));
										fprintf(cgiOut,"<td width=80><input type=\"radio\" name=\"showtype\" value=\"9\" onclick=\"changestate()\">%s:</td>",search(lcon,"fdb_limit"));
									}
									else if(strcmp(showtype,"8")==0)
									{
		  								fprintf(cgiOut,"<td width=80><input type=\"radio\" name=\"showtype\" value=\"1\" onclick=\"changestate()\">%s:</td>",search(lcon,"all"));
										fprintf(cgiOut,"<td width=80><input type=\"radio\" name=\"showtype\" value=\"2\" onclick=\"changestate()\">%s:</td>",search(lcon,"static"));
										fprintf(cgiOut,"<td width=80><input type=\"radio\" name=\"showtype\" value=\"3\" onclick=\"changestate()\">%s:</td>",search(lcon,"dynamic"));
										fprintf(cgiOut,"<td width=75><input type=\"radio\" name=\"showtype\" value=\"4\" onclick=\"changestate()\">%s:</td>",search(lcon,"blacklist"));
										fprintf(cgiOut,"<td width=70><input type=\"radio\" name=\"showtype\" value=\"5\" onclick=\"changestate()\">%s:</td>","VLAN");
										fprintf(cgiOut,"<td width=130><input type=\"radio\" name=\"showtype\" value=\"6\" onclick=\"changestate()\">%s:</td>","VLAN ID AND MAC");
										fprintf(cgiOut,"<td width=70><input type=\"radio\" name=\"showtype\" value=\"7\" onclick=\"changestate()\">%s:</td>","MAC");
										fprintf(cgiOut,"<td width=75><input type=\"radio\" name=\"showtype\" value=\"8\" onclick=\"changestate()\" checked>%s:</td>",search(lcon,"_port"));
										fprintf(cgiOut,"<td width=80><input type=\"radio\" name=\"showtype\" value=\"9\" onclick=\"changestate()\">%s:</td>",search(lcon,"fdb_limit"));
									}
									else if(strcmp(showtype,"9")==0)
									{
										fprintf(cgiOut,"<td width=80><input type=\"radio\" name=\"showtype\" value=\"1\" onclick=\"changestate()\">%s:</td>",search(lcon,"all"));
										fprintf(cgiOut,"<td width=80><input type=\"radio\" name=\"showtype\" value=\"2\" onclick=\"changestate()\">%s:</td>",search(lcon,"static"));
										fprintf(cgiOut,"<td width=80><input type=\"radio\" name=\"showtype\" value=\"3\" onclick=\"changestate()\">%s:</td>",search(lcon,"dynamic"));
										fprintf(cgiOut,"<td width=75><input type=\"radio\" name=\"showtype\" value=\"4\" onclick=\"changestate()\">%s:</td>",search(lcon,"blacklist"));
										fprintf(cgiOut,"<td width=70><input type=\"radio\" name=\"showtype\" value=\"5\" onclick=\"changestate()\">%s:</td>","VLAN");
										fprintf(cgiOut,"<td width=130><input type=\"radio\" name=\"showtype\" value=\"6\" onclick=\"changestate()\">%s:</td>","VLAN ID AND MAC");
										fprintf(cgiOut,"<td width=70><input type=\"radio\" name=\"showtype\" value=\"7\" onclick=\"changestate()\">%s:</td>","MAC");
										fprintf(cgiOut,"<td width=75><input type=\"radio\" name=\"showtype\" value=\"8\" onclick=\"changestate()\">%s:</td>",search(lcon,"_port"));	
										fprintf(cgiOut,"<td width=80><input type=\"radio\" name=\"showtype\" value=\"9\" onclick=\"changestate()\" checked>%s:</td>",search(lcon,"fdb_limit"));
									}
									else
									{
		  								fprintf(cgiOut,"<td width=80><input type=\"radio\" name=\"showtype\" value=\"1\" onclick=\"changestate()\" checked>%s:</td>",search(lcon,"all"));
										fprintf(cgiOut,"<td width=80><input type=\"radio\" name=\"showtype\" value=\"2\" onclick=\"changestate()\">%s:</td>",search(lcon,"static"));
										fprintf(cgiOut,"<td width=80><input type=\"radio\" name=\"showtype\" value=\"3\" onclick=\"changestate()\">%s:</td>",search(lcon,"dynamic"));
										fprintf(cgiOut,"<td width=75><input type=\"radio\" name=\"showtype\" value=\"4\" onclick=\"changestate()\">%s:</td>",search(lcon,"blacklist"));
										fprintf(cgiOut,"<td width=70><input type=\"radio\" name=\"showtype\" value=\"5\" onclick=\"changestate()\">%s:</td>","VLAN");
										fprintf(cgiOut,"<td width=130><input type=\"radio\" name=\"showtype\" value=\"6\" onclick=\"changestate()\">%s:</td>","VLAN ID AND MAC");
										fprintf(cgiOut,"<td width=70><input type=\"radio\" name=\"showtype\" value=\"7\" onclick=\"changestate()\">%s:</td>","MAC");
										fprintf(cgiOut,"<td width=75><input type=\"radio\" name=\"showtype\" value=\"8\" onclick=\"changestate()\">%s:</td>",search(lcon,"_port"));	
										fprintf(cgiOut,"<td width=80><input type=\"radio\" name=\"showtype\" value=\"9\" onclick=\"changestate()\">%s:</td>",search(lcon,"fdb_limit"));
									}

								   	///////////// 找出vid的参数，可设置成为隐藏的
           /////vlan id               
             cgiFormStringNoNewlines("VID", VID, N);  //页面传参
	         if(strcmp(VID,"")==0)
		     cgiFormStringNoNewlines("old_vid", VID, N);  //本地参数

           /////vlan name
			 cgiFormStringNoNewlines("VN", VN, N);  //页面传参
	         if(strcmp(VN,"")==0)
		     cgiFormStringNoNewlines("old_vname", VN, N);  //本地参数

			  /////vlan port
			 cgiFormStringNoNewlines("PT", PT, N);  //页面传参
	         if(strcmp(PT,"")==0)
		     cgiFormStringNoNewlines("old_port", PT, N);  //本地参数

			  /////vlan mac
			 cgiFormStringNoNewlines("MAC", MAC, N);  //页面传参
	         if(strcmp(MAC,"")==0)
		     cgiFormStringNoNewlines("old_mac", MAC, N);  //本地参数

            ////// vlan 的单选按钮
			 memset(TY,0,N);
			 cgiFormStringNoNewlines("TY", TY, N);   //先抓页面参数
			  
			 if(strcmp(TY,"")!=0)
					strcpy(vlantype,TY);
             else{
				memset(vlantype,0,N);
				cgiFormStringNoNewlines("vlantype", vlantype, N);  //抓表单
                 }
			



									/////////////

								fprintf(cgiOut,"</tr>"\
						  "</table>"\
						  "<table border=0 cellspacing=0 cellpadding=0>");


						if(strcmp(vlantype,"1")==0)	
							{
								fprintf(cgiOut,"<tr height=35>");
									fprintf(cgiOut,"<td><input type=\"radio\" name=\"vlantype\" value=\"1\" onclick=\"changestate()\" checked>%s:</td><td width=140><input type=text name=vlanid size=21 value=\"%s\" maxLength=4></td><td><font color=red>(1--4094)</font></td>",search(lcon,"vID"),VID);
									fprintf(cgiOut,"<td>%s:</td><td width=140><input type=text name=mac size=21 value=\"%s\"></td><td><font color=red>%s</font></td>","MAC",MAC,"(00:00:11:22:33:44)");
								fprintf(cgiOut,"</tr>");
								fprintf(cgiOut,"<tr height=35>");
									fprintf(cgiOut,"<td><input type=\"radio\" name=\"vlantype\" value=\"2\" onclick=\"changestate()\">%s:</td><td width=140><input type=text name=vlanname size=21 value=\"\"></td><td><font color=red>(%s)</font></td>",search(lcon,"vlan_name"),search(lcon,"vlanname_form"));
									fprintf(cgiOut,"<td>%s:</td><td width=140><input type=text name=port size=21 value=\"%s\"></td><td><font color=red>(%s)</font></td>",search(lcon,"_port"),PT,search(lcon,"port_form"));
								fprintf(cgiOut,"</tr>");
							}
						else if(strcmp(vlantype,"2")==0)	
							{
								fprintf(cgiOut,"<tr height=35>");
									fprintf(cgiOut,"<td><input type=\"radio\" name=\"vlantype\" value=\"1\" onclick=\"changestate()\" >%s:</td><td width=140><input type=text name=vlanid size=21 value=\"\" maxLength=4></td><td><font color=red>(1--4094)</font></td>",search(lcon,"vID"));
									fprintf(cgiOut,"<td>%s:</td><td width=140><input type=text name=mac size=21 value=\"%s\"></td><td><font color=red>%s</font></td>","MAC",MAC,"(00:00:11:22:33:44)");
								fprintf(cgiOut,"</tr>");
								fprintf(cgiOut,"<tr height=35>");
									fprintf(cgiOut,"<td><input type=\"radio\" name=\"vlantype\" value=\"2\" onclick=\"changestate()\" checked>%s:</td><td width=140><input type=text name=vlanname size=21 value=\"%s\"></td><td><font color=red>(%s)</font></td>",search(lcon,"vlan_name"),VN,search(lcon,"vlanname_form"));
									fprintf(cgiOut,"<td>%s:</td><td width=140><input type=text name=port size=21 value=\"%s\"></td><td><font color=red>(%s)</font></td>",search(lcon,"_port"),PT,search(lcon,"port_form"));
								fprintf(cgiOut,"</tr>");
							}
						else 
							{
								fprintf(cgiOut,"<tr height=35>");
									fprintf(cgiOut,"<td><input type=\"radio\" name=\"vlantype\" value=\"1\" onclick=\"changestate()\" checked>%s:</td><td width=140><input type=text name=vlanid size=21 value=\"%s\" maxLength=4></td><td><font color=red>(1--4094)</font></td>",search(lcon,"vID"),VID);
									fprintf(cgiOut,"<td>%s:</td><td width=140><input type=text name=mac size=21 value=\"%s\"></td><td><font color=red>%s</font></td>","MAC",MAC,"(00:00:11:22:33:44)");
								fprintf(cgiOut,"</tr>");
								fprintf(cgiOut,"<tr height=35>");
									fprintf(cgiOut,"<td><input type=\"radio\" name=\"vlantype\" value=\"2\" onclick=\"changestate()\">%s:</td><td width=140><input type=text name=vlanname size=21 value=\"\"></td><td><font color=red>(%s)</font></td>",search(lcon,"vlan_name"),search(lcon,"vlanname_form"));
									fprintf(cgiOut,"<td>%s:</td><td width=140><input type=text name=port size=21 value=\"%s\"></td><td><font color=red>(%s)</font></td>",search(lcon,"_port"),PT,search(lcon,"port_form"));
								fprintf(cgiOut,"</tr>");
							}
								
						 fprintf(cgiOut,"</table>");
			//改变显示状态
			fprintf(cgiOut,"<script type='text/javascript'>"\
							"changestate();"\
							"</script>");					
			fprintf(cgiOut,"<table width=750 border=0 cellspacing=0 cellpadding=0>"\
								"<tr height=35>");
					
						fprintf(cgiOut,"<td align=center id=sec1 style=\"border-bottom:2px solid #53868b;font-size:14px;padding-left:0px;padding-top:0px\">%s</td>","FDB");
					    
							fprintf(cgiOut,"</tr>");
							fprintf(cgiOut,"<tr>"\
							  "<td align=left valign=top  style=\"padding-top:0px\">");
							   fprintf(cgiOut,"<div id=\"list\"  style=overflow:auto><table  width=750 border=1 frame=below rules=rows bordercolor=#cccccc cellspacing=0 cellpadding=0>");



                                 /*--------------------- begin ------------------------------*/

 ///////////////////  列出了总的条目，不恰当，莫若用链表来取得的恰当
	    if(strcmp(showtype,"1")==0)
		{
		fdbcount = get_fdb_count();	
	    }
		if(strcmp(showtype,"2")==0)
		{
		 show_fdb_static(&s_head,&s_num);
		 fdbcount=s_num;		 

		}
		if(strcmp(showtype,"3")==0)
		{
		  show_fdb_dynamic(&d_head,&d_num);
		  fdbcount=d_num;

		}
		if(strcmp(showtype,"4")==0)
		{
		  show_fdb_blacklist(&bk_head,&bk_num);
		  fdbcount=bk_num;

		}
		if(strcmp(showtype,"5")==0)
		{
	
		memset(vlanid,0,N);
		cgiFormStringNoNewlines("vlanid", vlanid, N); 
		if(strcmp(vlanid,"")==0)
			strcpy(vlanid,VID);
		
		if(strcmp(vlantype,"1")==0)
			{
             show_fdb_by_vid(vlanid,&vid_head,&vid_num);		
     		 fdbcount=vid_num;
			}

		memset(vlanname,0,N);
		cgiFormStringNoNewlines("vlanname", vlanname, N); 
		if(strcmp(vlanname,"")==0)
			strcpy(vlanname,VN);
		
		if(strcmp(vlantype,"2")==0)
			{
             show_fdb_by_vlanname(vlanname,&vid_head,&vid_num);                                     	
     		 fdbcount=vid_num;
		    }
		
		}
		if(strcmp(showtype,"6")==0)
		{

	    memset(mac,0,N);
		cgiFormStringNoNewlines("mac", mac, N); 
		if(strcmp(mac,"")==0)
			strcpy(mac,MAC);	
		
		memset(vlanid,0,N);
		cgiFormStringNoNewlines("vlanid", vlanid, N); 
		if(strcmp(vlanid,"")==0)
			strcpy(vlanid,VID);	
		
		}
		if(strcmp(showtype,"7")==0)
		{
	
		memset(mac,0,N);
		cgiFormStringNoNewlines("mac", mac, N); 
		if(strcmp(mac,"")==0)
		strcpy(mac,MAC);

		show_fdb_by_mac(mac,&m_head,&m_num);
		fdbcount=m_num;
		
		
		}
		if(strcmp(showtype,"8")==0)
		{
	
		memset(port,0,N);
		cgiFormStringNoNewlines("port", port, N); 
		if(strcmp(port,"")==0)
			strcpy(port,PT);
		
		show_fdb_by_port(&pt_head,&p_num,port);
		fdbcount=p_num;
		}
		if(strcmp(showtype,"9")==0)
		{
	
		show_fdb_number_limit(&lm_head,&l_num);
		fdbcount=l_num;
		}
		
///////////////////////切出了对于页面的处理逻辑关系


   {
	 if((fdbcount%PageNumber)==0)
		   LastPage=(fdbcount/PageNumber); //计算出最大页数
	   else	
		   LastPage=(fdbcount/PageNumber)+1; //计算出最大页数


			  if(0==strcmp(SNtemp,"PageFirst"))   //回到首页
			 	 	{
    			 	 					
			 	 		if(fdbcount-pageNum*PageNumber<PageNumber)
								xnt=fdbcount;
						else	  xnt=(pageNum+1)*PageNumber;

						head=0;
						tail=xnt;
			 	 	}
			  else if(0==strcmp(SNtemp,"PageDown") || 0==strcmp(SNtemp,""))  //往下翻页
				  {
    				  					  
					  if(fdbcount-pageNum*PageNumber<0)
					  {
						  pageNum=pageNum-1;
						  ShowAlert(search(lcon,"Page_end")); 
					  }
					  if(fdbcount-pageNum*PageNumber<PageNumber)
							  xnt=fdbcount;
					  else	  xnt=(pageNum+1)*PageNumber;

					  head=pageNum*PageNumber;
					  tail=xnt;
				  }
			  else if(0==strcmp(SNtemp,"PageUp"))  //往上翻页
				  {
					  if(pageNum<0)
					  {
						  pageNum=pageNum+1;
						  ShowAlert(search(lcon,"Page_Begin"));
					  }
					  if(fdbcount-pageNum*PageNumber<PageNumber)
							  xnt=fdbcount;
					  else	  xnt=(pageNum+1)*PageNumber;
					  head=pageNum*PageNumber;
					  tail=xnt;
				  }
			  else if(0==strcmp(SNtemp,"PageLast"))  //回到末页
					{
					    if(pageNum==0)
						  {
						   head = fdbcount;
					       tail = fdbcount;
						   pageNum--;
				    	  }
					   else
						  {
					   	   head = (pageNum-1)*PageNumber;
						   tail = fdbcount;
						   pageNum--;
						  }
					}
	  
	 sprintf(pageNumCA,"%d",pageNum);
  }
//////////////////////
#if 0
/*---------- b b -------------------------*/
		//int FirstPage=0,LastPage=0;	
				

		 if((fdbcount%PageNumber)==0)
			   LastPage=(fdbcount/PageNumber); //计算出最大页数
		   else	
			   LastPage=(fdbcount/PageNumber)+1; //计算出最大页数

	    

			   /*首页，就是跳转到第一页*/
			    if(0==strcmp(SNtemp,"PageFirst"))
		 	 	{	
		 	 	    FirstPage=1;
		 	 		pageNum=FirstPage;
		 	 	}
				
			/*向后翻，就是当前页加一页*/
		  else if(0==strcmp(SNtemp,"PageDown") || 0==strcmp(SNtemp,""))
			  {
				  if(pageNum != LastPage)				  
					  pageNum=pageNum+1;
				  else				 	
					  ShowAlert(search(lcon,"Page_end")); 
				
				 
			  }
		  
		  /*向前翻，就是当前页减一页*/
		  else if(0==strcmp(SNtemp,"PageUp"))
			  {
				  if(pageNum!=1)				
					  pageNum=pageNum-1;
				  else if(pageNum==1)				
			    	  ShowAlert(search(lcon,"Page_Begin"));
				
				 
			  }
		  
		  /*末页，就是跳转到最后一页*/
		  else if(0==strcmp(SNtemp,"PageLast"))
				{
					pageNum=LastPage;
				}
 
  
                  sprintf(pageNumCA,"%d",pageNum);

 			      
 /*-----------  end -----------------------------*/
#endif
	   
				   		
						if(strcmp(showtype,"1")==0)  //全部
						{
						    strcpy(ID,"1");
							strcpy(TY,"1");
							result1 = -1;
							
							
          							fprintf(cgiOut,"<tr height=25 bgcolor=#eaeff9 style=font-size:14px align=left>");
                                  	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","MAC");
                                  	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","VLAN");
                                  	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","SLOT/PORT");
                                  	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","TRUNK");
                                  	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","VID");
                                  	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","VIDX");
                                  	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","TYPE");
                                  	fprintf(cgiOut,"</tr>");
									

							result1=show_fdb(&f_head,&all_num);
							fq=f_head.next;
							
							for(i=0;i<head;i++)  //这样就可以去掉前面的数据了，像是挪到指定位置。
							fq=fq->next;
						    							   
                           	   for(i=head;i<tail;i++)  //应该输出链表的指定项       
                          		  {
                                
                                			if (fq->dcli_flag == port_type){
                                			if(CPU_PORT_VIRTUAL_SLOT == fq->trans_value1) { //static FDB to CPU
                                				if(CPU_PORT_VIRTUAL_PORT == fq->trans_value2)
                                					{
                                        				fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));		
                                        				fprintf(cgiOut,"<td align=left>%02x:%02x:%02x:%02x:%02x:%02x</td>",fq->show_mac[0],fq->show_mac[1],fq->show_mac[2],fq->show_mac[3],fq->show_mac[4],fq->show_mac[5]);
                                				        fprintf(cgiOut,"<td align=left>%d</td>",fq->vlanid);
														fprintf(cgiOut,"<td align=left>%s</td>","CPU");
                                        				fprintf(cgiOut,"<td align=left>%s</td>","-");
                                        				fprintf(cgiOut,"<td align=left>%s</td>","-");
                                        				fprintf(cgiOut,"<td align=left>%s</td>","-");
                                						fprintf(cgiOut,"<td align=left>%s</td>","-");
                                        				fprintf(cgiOut,"</tr>");	
                                					}
                                				else
                                				{
                                                    if(SPI_PORT_VIRTUAL_PORT == fq->trans_value2)
													{
                                						fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));		
                                        				fprintf(cgiOut,"<td align=left>%02x:%02x:%02x:%02x:%02x:%02x</td>",fq->show_mac[0],fq->show_mac[1],fq->show_mac[2],fq->show_mac[3],fq->show_mac[4],fq->show_mac[5]);
                                        				fprintf(cgiOut,"<td align=left>%d</td>",fq->vlanid);
                                        				fprintf(cgiOut,"<td align=left>%s</td>","HSC");
                                        				fprintf(cgiOut,"<td align=left>%s</td>","-");
                                        				fprintf(cgiOut,"<td align=left>%s</td>","-");
                                        				fprintf(cgiOut,"<td align=left>%s</td>","-");
                                						fprintf(cgiOut,"<td align=left>%s</td>","-");
                                        				fprintf(cgiOut,"</tr>");
                                					}
													else
													{
                                						fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));		
                                        				fprintf(cgiOut,"<td align=left>%02x:%02x:%02x:%02x:%02x:%02x</td>",fq->show_mac[0],fq->show_mac[1],fq->show_mac[2],fq->show_mac[3],fq->show_mac[4],fq->show_mac[5]);
                                        				fprintf(cgiOut,"<td align=left>%d</td>",fq->vlanid);
                                        				fprintf(cgiOut,"<td align=left>%s</td>","ERR");
                                        				fprintf(cgiOut,"<td align=left>%s</td>","-");
                                        				fprintf(cgiOut,"<td align=left>%s</td>","-");
                                        				fprintf(cgiOut,"<td align=left>%s</td>","-");
                                						fprintf(cgiOut,"<td align=left>%s</td>","-");
                                        				fprintf(cgiOut,"</tr>");
                                					}
                                				}
                                			}
                                			else {
                                				fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));			
                                				fprintf(cgiOut,"<td align=left>%02x:%02x:%02x:%02x:%02x:%02x</td>",fq->show_mac[0],fq->show_mac[1],fq->show_mac[2],fq->show_mac[3],fq->show_mac[4],fq->show_mac[5]);
                                				fprintf(cgiOut,"<td align=left>%d</td>",fq->vlanid);
                                				fprintf(cgiOut,"<td align=left>%d/%d</td>",fq->trans_value1,fq->trans_value2);
                                				fprintf(cgiOut,"<td align=left>%s</td>","-");
                                				fprintf(cgiOut,"<td align=left>%s</td>","-");
                                				fprintf(cgiOut,"<td align=left>%s</td>","-");
                                				fprintf(cgiOut,"<td align=left>%s</td>",fq->type);
                                				fprintf(cgiOut,"</tr>");				
                                			}
                                		}
                                		else if (fq->dcli_flag == trunk_type){ 				
                                			fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));		
                                			fprintf(cgiOut,"<td align=left>%02x:%02x:%02x:%02x:%02x:%02x</td>",fq->show_mac[0],fq->show_mac[1],fq->show_mac[2],fq->show_mac[3],fq->show_mac[4],fq->show_mac[5]);
                                			fprintf(cgiOut,"<td align=left>%d</td>",fq->vlanid);
                                			fprintf(cgiOut,"<td align=left>%s</td>","-");
                                			fprintf(cgiOut,"<td align=left>%d</td>",fq->trans_value1);
                                			fprintf(cgiOut,"<td align=left>%s</td>","-");
                                			fprintf(cgiOut,"<td align=left>%s</td>","-");
                                			fprintf(cgiOut,"<td align=left>%s</td>",fq->type);
                                			fprintf(cgiOut,"</tr>");		        
                                		}
                                		
                                		else if (fq->dcli_flag == vid_type){				
                                			fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));		
                                			fprintf(cgiOut,"<td align=left>%02x:%02x:%02x:%02x:%02x:%02x</td>",fq->show_mac[0],fq->show_mac[1],fq->show_mac[2],fq->show_mac[3],fq->show_mac[4],fq->show_mac[5]);
                                			fprintf(cgiOut,"<td align=left>%d</td>",fq->vlanid);
                                			fprintf(cgiOut,"<td align=left>%s</td>","-");
                                			fprintf(cgiOut,"<td align=left>%s</td>","-");
                                			fprintf(cgiOut,"<td align=left>%d</td>",fq->trans_value1);
                                			fprintf(cgiOut,"<td align=left>%s</td>","-");
                                			fprintf(cgiOut,"<td align=left>%s</td>",fq->type);
                                			fprintf(cgiOut,"</tr>");    
                                		}			
                                		else if (fq->dcli_flag == vidx_type){				
                                			fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));	
                                			fprintf(cgiOut,"<td align=left>%02x:%02x:%02x:%02x:%02x:%02x</td>",fq->show_mac[0],fq->show_mac[1],fq->show_mac[2],fq->show_mac[3],fq->show_mac[4],fq->show_mac[5]);
                                			fprintf(cgiOut,"<td align=left>%d</td>",fq->vlanid);
                                			fprintf(cgiOut,"<td align=left>%s</td>","-");
                                			fprintf(cgiOut,"<td align=left>%s</td>","-");
                                			fprintf(cgiOut,"<td align=left>%s</td>","-");
                                			fprintf(cgiOut,"<td align=left>%d</td>",fq->trans_value1);
                                			fprintf(cgiOut,"<td align=left>%s</td>",fq->type);
                                			fprintf(cgiOut,"</tr>");  		       
                                		}
                                		else 
										{
									          fprintf(cgiOut,"<div><tr height = 30><td><font color=red>%s</font></td></tr></div>",search(lcon,"show_err"));
                                		}

                                         cl = !cl;
									     fq=fq->next;
                                		  }		
							
							switch(result1)
							{	
								case -1:
									fprintf(cgiOut,"<div><tr height = 30><td><font color=red>%s</font></td></tr></div>",search(lcon,"show_err"));
									break;
								case -2:
									fprintf(cgiOut,"<div><tr height = 30><td><font color=red>%s</font></td></tr></div>",search(lcon,"fdb_none"));
									break;	
							}
						}
						else if(strcmp(showtype,"2")==0)  //静态
						{
						   strcpy(ID,"2");	
						   strcpy(TY,"1");
							
                            fprintf(cgiOut,"<tr height=25 bgcolor=#eaeff9 style=font-size:14px align=left>");
                        	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","MAC");
                        	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","VLAN");
                        	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","SLOT/PORT");
                        	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","TRUNK");
                        	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","VID");
                        	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","VIDX");
                        	fprintf(cgiOut,"</tr>");
							
                            result2 = -1;
							
							result2 = show_fdb_static(&s_head,&s_num);
                            sq=s_head.next;

							for(i=0;i<head;i++)
							sq=sq->next;
							
							for(i=head;i<tail;i++)
								{


                      		if (sq->dcli_flag == port_type){
                      			if(CPU_PORT_VIRTUAL_SLOT == sq->trans_value1) { //static FDB to CPU
                      				if(CPU_PORT_VIRTUAL_PORT == sq->trans_value2)
                      					{
                              				fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));		
                              				fprintf(cgiOut,"<td align=left>%02x:%02x:%02x:%02x:%02x:%02x</td>",sq->show_mac[0],sq->show_mac[1],sq->show_mac[2],sq->show_mac[3],sq->show_mac[4],sq->show_mac[5]);
                              				fprintf(cgiOut,"<td align=left>%d</td>",sq->vlanid);
                              				fprintf(cgiOut,"<td align=left>%s</td>","CPU");
                              				fprintf(cgiOut,"<td align=left>%s</td>","-");
                              				fprintf(cgiOut,"<td align=left>%s</td>","-");
                              				fprintf(cgiOut,"<td align=left>%s</td>","-");
                              				fprintf(cgiOut,"</tr>");	
                      					}
                      				else
                      				{
                                        if(SPI_PORT_VIRTUAL_PORT == sq->trans_value2)
										{
                      						fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));		
                              				fprintf(cgiOut,"<td align=left>%02x:%02x:%02x:%02x:%02x:%02x</td>",sq->show_mac[0],sq->show_mac[1],sq->show_mac[2],sq->show_mac[3],sq->show_mac[4],sq->show_mac[5]);
                              				fprintf(cgiOut,"<td align=left>%d</td>",sq->vlanid);
                              				fprintf(cgiOut,"<td align=left>%s</td>","HSC");
                              				fprintf(cgiOut,"<td align=left>%s</td>","-");
                              				fprintf(cgiOut,"<td align=left>%s</td>","-");
                              				fprintf(cgiOut,"<td align=left>%s</td>","-");
                              				fprintf(cgiOut,"</tr>");
                      					}
										else
										{
                      						fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));		
                              				fprintf(cgiOut,"<td align=left>%02x:%02x:%02x:%02x:%02x:%02x</td>",sq->show_mac[0],sq->show_mac[1],sq->show_mac[2],sq->show_mac[3],sq->show_mac[4],sq->show_mac[5]);
                              				fprintf(cgiOut,"<td align=left>%d</td>",sq->vlanid);
                              				fprintf(cgiOut,"<td align=left>%s</td>","ERR");
                              				fprintf(cgiOut,"<td align=left>%s</td>","-");
                              				fprintf(cgiOut,"<td align=left>%s</td>","-");
                              				fprintf(cgiOut,"<td align=left>%s</td>","-");
                              				fprintf(cgiOut,"</tr>");
                      					}	
									}
                      			}
                      			else {
                      				fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));			
                      				fprintf(cgiOut,"<td align=left>%02x:%02x:%02x:%02x:%02x:%02x</td>",sq->show_mac[0],sq->show_mac[1],sq->show_mac[2],sq->show_mac[3],sq->show_mac[4],sq->show_mac[5]);
                      				fprintf(cgiOut,"<td align=left>%d</td>",sq->vlanid);
                      				fprintf(cgiOut,"<td align=left>%d/%d</td>",sq->trans_value1,sq->trans_value2);
                      				fprintf(cgiOut,"<td align=left>%s</td>","-");
                      				fprintf(cgiOut,"<td align=left>%s</td>","-");
                      				fprintf(cgiOut,"<td align=left>%s</td>","-");
                      				fprintf(cgiOut,"</tr>");					
                      			}
                      		}
                      		else if (sq->dcli_flag == trunk_type){ 				
                      			fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));		
                      			fprintf(cgiOut,"<td align=left>%02x:%02x:%02x:%02x:%02x:%02x</td>",sq->show_mac[0],sq->show_mac[1],sq->show_mac[2],sq->show_mac[3],sq->show_mac[4],sq->show_mac[5]);
                      			fprintf(cgiOut,"<td align=left>%d</td>",sq->vlanid);
                      			fprintf(cgiOut,"<td align=left>%s</td>","-");
                      			fprintf(cgiOut,"<td align=left>%d</td>",sq->trans_value1);
                      			fprintf(cgiOut,"<td align=left>%s</td>","-");
                      			fprintf(cgiOut,"<td align=left>%s</td>","-");
                      			fprintf(cgiOut,"</tr>");					      
                      		}
                      		else if (sq->dcli_flag == vid_type){				
                      			fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));		
                      			fprintf(cgiOut,"<td align=left>%02x:%02x:%02x:%02x:%02x:%02x</td>",sq->show_mac[0],sq->show_mac[1],sq->show_mac[2],sq->show_mac[3],sq->show_mac[4],sq->show_mac[5]);
                      			fprintf(cgiOut,"<td align=left>%d</td>",sq->vlanid);
                      			fprintf(cgiOut,"<td align=left>%s</td>","-");
                      			fprintf(cgiOut,"<td align=left>%s</td>","-");
                      			fprintf(cgiOut,"<td align=left>%d</td>",sq->trans_value1);
                      			fprintf(cgiOut,"<td align=left>%s</td>","-");
                      			fprintf(cgiOut,"</tr>");  				       
                      		}			
                      		else if (sq->dcli_flag == vidx_type){				
                      			fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));	
                      			fprintf(cgiOut,"<td align=left>%02x:%02x:%02x:%02x:%02x:%02x</td>",sq->show_mac[0],sq->show_mac[1],sq->show_mac[2],sq->show_mac[3],sq->show_mac[4],sq->show_mac[5]);
                      			fprintf(cgiOut,"<td align=left>%d</td>",sq->vlanid);
                      			fprintf(cgiOut,"<td align=left>%s</td>","-");
                      			fprintf(cgiOut,"<td align=left>%s</td>","-");
                      			fprintf(cgiOut,"<td align=left>%s</td>","-");
                      			fprintf(cgiOut,"<td align=left>%d</td>",sq->trans_value1);
                      			fprintf(cgiOut,"</tr>");  				
                      		}
                      		else {
							fprintf(cgiOut,"<div><tr height = 30><td><font color=red>%s</font></td></tr></div>",search(lcon,"show_err"));

                      		}
			
								cl = !cl;
								sq=sq->next;
								}
							
							switch(result2)
							{	
								case -1:
									fprintf(cgiOut,"<div><tr height = 30><td><font color=red>%s</font></td></tr></div>",search(lcon,"show_err"));
									break;
								case -2:
									fprintf(cgiOut,"<div><tr height = 30><td><font color=red>%s</font></td></tr></div>",search(lcon,"fdb_none"));
									break;	
							}
						}
						else if(strcmp(showtype,"3")==0)  //动态
						{ 
						    strcpy(ID,"3");
							strcpy(TY,"1");
							result3 = -1;							

                                fprintf(cgiOut,"<tr height=25 bgcolor=#eaeff9 style=font-size:14px align=left>");
                            	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","MAC");
                            	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","VLAN");
                            	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","SLOT/PORT");
                            	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","TRUNK");
                            	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","VID");
                            	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","VIDX");
                            	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","TYPE");
                            	fprintf(cgiOut,"</tr>");

								
							//ret = show_fdb_dynamic();//show dynamic
							result3=show_fdb_dynamic(&d_head,&d_num);
                            dq=d_head.next;

							for(i=0;i<head;i++)
							dq=dq->next;
							for(i=head;i<tail;i++)								
								{

                           	if (dq->dcli_flag == port_type){
                         			if(CPU_PORT_VIRTUAL_SLOT == dq->trans_value1) { //static FDB to CPU
                         				if(CPU_PORT_VIRTUAL_PORT == dq->trans_value2)
                         					{
                                 				fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));		
                                 				fprintf(cgiOut,"<td align=left>%02x:%02x:%02x:%02x:%02x:%02x</td>",dq->show_mac[0],dq->show_mac[1],dq->show_mac[2],dq->show_mac[3],dq->show_mac[4],dq->show_mac[5]);
                                 				fprintf(cgiOut,"<td align=left>%d</td>",dq->vlanid);
                                 				fprintf(cgiOut,"<td align=left>%s</td>","CPU");
                                 				fprintf(cgiOut,"<td align=left>%s</td>","-");
                                 				fprintf(cgiOut,"<td align=left>%s</td>","-");
                                 				fprintf(cgiOut,"<td align=left>%s</td>","-");
                         						fprintf(cgiOut,"<td align=left>%s</td>","dynamic");
                                 				fprintf(cgiOut,"</tr>");	
                         					}
                         				else
                         				{
                                           if(SPI_PORT_VIRTUAL_PORT == dq->trans_value2)
											{
                         						fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));		
                                 				fprintf(cgiOut,"<td align=left>%02x:%02x:%02x:%02x:%02x:%02x</td>",dq->show_mac[0],dq->show_mac[1],dq->show_mac[2],dq->show_mac[3],dq->show_mac[4],dq->show_mac[5]);
                                 				fprintf(cgiOut,"<td align=left>%d</td>",dq->vlanid);
                                 				fprintf(cgiOut,"<td align=left>%s</td>","HSC");
                                 				fprintf(cgiOut,"<td align=left>%s</td>","-");
                                 				fprintf(cgiOut,"<td align=left>%s</td>","-");
                                 				fprintf(cgiOut,"<td align=left>%s</td>","-");
                         						fprintf(cgiOut,"<td align=left>%s</td>","dynamic");
                                 				fprintf(cgiOut,"</tr>");
                         					}
                         					else
											{
                         						fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));		
                                 				fprintf(cgiOut,"<td align=left>%02x:%02x:%02x:%02x:%02x:%02x</td>",dq->show_mac[0],dq->show_mac[1],dq->show_mac[2],dq->show_mac[3],dq->show_mac[4],dq->show_mac[5]);
                                 				fprintf(cgiOut,"<td align=left>%d</td>",dq->vlanid);
                                 				fprintf(cgiOut,"<td align=left>%s</td>","ERR");
                                 				fprintf(cgiOut,"<td align=left>%s</td>","-");
                                 				fprintf(cgiOut,"<td align=left>%s</td>","-");
                                 				fprintf(cgiOut,"<td align=left>%s</td>","-");
                         						fprintf(cgiOut,"<td align=left>%s</td>","dynamic");
                                 				fprintf(cgiOut,"</tr>");
                         					}
												
                         				}
                         			}
                         			else {
                         				fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));		
                         				fprintf(cgiOut,"<td align=left>%02x:%02x:%02x:%02x:%02x:%02x</td>",dq->show_mac[0],dq->show_mac[1],dq->show_mac[2],dq->show_mac[3],dq->show_mac[4],dq->show_mac[5]);
                         				fprintf(cgiOut,"<td align=left>%d</td>",dq->vlanid);
                         				fprintf(cgiOut,"<td align=left>%d/%d</td>",dq->trans_value1,dq->trans_value2);
                         				fprintf(cgiOut,"<td align=left>%s</td>","-");
                         				fprintf(cgiOut,"<td align=left>%s</td>","-");
                         				fprintf(cgiOut,"<td align=left>%s</td>","-");
                         				fprintf(cgiOut,"<td align=left>%s</td>","dynamic");
                         				fprintf(cgiOut,"</tr>");
                         
                         			}
                         		}
                         		else if (dq->dcli_flag == trunk_type){ 				
                         				fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));		
                         				fprintf(cgiOut,"<td align=left>%02x:%02x:%02x:%02x:%02x:%02x</td>",dq->show_mac[0],dq->show_mac[1],dq->show_mac[2],dq->show_mac[3],dq->show_mac[4],dq->show_mac[5]);
                         				fprintf(cgiOut,"<td align=left>%d</td>",dq->vlanid);
                         				fprintf(cgiOut,"<td align=left>%s</td>","-");
                         				fprintf(cgiOut,"<td align=left>%d</td>",dq->trans_value1);
                         				fprintf(cgiOut,"<td align=left>%s</td>","-");
                         				fprintf(cgiOut,"<td align=left>%s</td>","-");
                         				fprintf(cgiOut,"<td align=left>%s</td>","dynamic");
                         				fprintf(cgiOut,"</tr>");	        
                         		}
                         		
                         		else {
                         		fprintf(cgiOut,"<div><tr height = 30><td><font color=red>%s</font></td></tr></div>",search(lcon,"show_err"));
                         		}
                         

								 cl =!cl;
								 dq=dq->next;
								}
							switch(result3)
							{	
								case -1:
									fprintf(cgiOut,"<div><tr height = 30><td><font color=red>%s</font></td></tr></div>",search(lcon,"show_err"));
									break;
								case -2:
									fprintf(cgiOut,"<div><tr height = 30><td><font color=red>%s</font></td></tr></div>",search(lcon,"fdb_none"));
									break;	
							}
						}
						else if(strcmp(showtype,"4")==0)
						{
						    strcpy(ID,"4");
							strcpy(TY,"1");

                            fprintf(cgiOut,"<tr height=25 bgcolor=#eaeff9 style=font-size:14px align=left>");
                         	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","MAC");
                         	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","VLAN");
                         	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","DMAC");
                         	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","SMAC");
                         	fprintf(cgiOut,"</tr>");

							
							result4= -1;
							//result4 = show_fdb_blacklist();//show blacklist
							result4 = show_fdb_blacklist(&bk_head,&bk_num);
							bkq=bk_head.next;

							for(i=0;i<head;i++)
							bkq=bkq->next;
							for(i=head;i<tail;i++)
								{

                     		fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));	
                     		fprintf(cgiOut,"<td align=left>%02x:%02x:%02x:%02x:%02x:%02x</td>",bkq->show_mac[0],bkq->show_mac[1],bkq->show_mac[2],bkq->show_mac[3],bkq->show_mac[4],bkq->show_mac[5]);
                     		fprintf(cgiOut,"<td align=left>%d</td>",bkq->vlanid);
                     		fprintf(cgiOut,"<td align=left>%d</td>",bkq->dmac);
                     		fprintf(cgiOut,"<td align=left>%d</td>",bkq->smac);
                     		fprintf(cgiOut,"</tr>"); 
							
                                cl = !cl;
								bkq=bkq->next;
								}

							switch(result4)
							{	
								case -1:
									fprintf(cgiOut,"<div><tr height = 30><td><font color=red>%s</font></td></tr></div>",search(lcon,"show_err"));
									break;
								case -2:
									fprintf(cgiOut,"<div><tr height = 30><td><font color=red>%s</font></td></tr></div>",search(lcon,"fdb_none"));
									break;	
							}
						}
						else if(strcmp(showtype,"5")==0)
						{	
						    strcpy(ID,"5");
							result5 = -1;
							if(strcmp(vlantype,"1")==0)
							{
							 strcpy(TY,"1");
							 
    						 fprintf(cgiOut,"<tr height=25 bgcolor=#eaeff9 style=font-size:14px align=left>");
                         	 fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","MAC");
                         	 fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","VLAN");
                         	 fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","SLOT/PORT");
                         	 fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","TRUNK");
                         	 fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","VID");
                         	 fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","VIDX");
                         	 fprintf(cgiOut,"</tr>");
							
								if(strcmp(vlanid,"")!=0)
									//result5 = show_fdb_by_vid(vlanid);//show by vlanid
                                    {
                                       
                                     	strcpy(VID,vlanid);
										
                                     	result5 = show_fdb_by_vid(vlanid,&vid_head,&vid_num);
                                         vq=vid_head.next;
                                     	
                                     	for(i=0;i<head;i++)
                                     		vq=vq->next;
                                     	
                                     	for(i=head;i<tail;i++)
                                     		{
                                     
                                     	if (vq->dcli_flag == port_type){
                                     			if(CPU_PORT_VIRTUAL_SLOT == vq->trans_value1) { //static FDB to CPU
                                     				if(CPU_PORT_VIRTUAL_PORT == vq->trans_value2)
                                     					{
                                             				fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));		
                                             				fprintf(cgiOut,"<td align=left>%02x:%02x:%02x:%02x:%02x:%02x</td>",vq->show_mac[0],vq->show_mac[1],vq->show_mac[2],vq->show_mac[3],vq->show_mac[4],vq->show_mac[5]);
                                             				fprintf(cgiOut,"<td align=left>%d</td>",vq->vlanid);
                                             				fprintf(cgiOut,"<td align=left>%s</td>","CPU");
                                             				fprintf(cgiOut,"<td align=left>%s</td>","-");
                                             				fprintf(cgiOut,"<td align=left>%s</td>","-");
                                             				fprintf(cgiOut,"<td align=left>%s</td>","-");
                                             				fprintf(cgiOut,"</tr>");	
                                     					}
                                     				else
                                     				{
                                     				    if(SPI_PORT_VIRTUAL_PORT == vq->trans_value2)
                                     					{
                                     						fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));		
                                             				fprintf(cgiOut,"<td align=left>%02x:%02x:%02x:%02x:%02x:%02x</td>",vq->show_mac[0],vq->show_mac[1],vq->show_mac[2],vq->show_mac[3],vq->show_mac[4],vq->show_mac[5]);
                                             				fprintf(cgiOut,"<td align=left>%d</td>",vq->vlanid);
                                             				fprintf(cgiOut,"<td align=left>%s</td>","HSC");
                                             				fprintf(cgiOut,"<td align=left>%s</td>","-");
                                             				fprintf(cgiOut,"<td align=left>%s</td>","-");
                                             				fprintf(cgiOut,"<td align=left>%s</td>","-");
                                             				fprintf(cgiOut,"</tr>");
                                     					}
														else
														{
                                     						fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));		
                                             				fprintf(cgiOut,"<td align=left>%02x:%02x:%02x:%02x:%02x:%02x</td>",vq->show_mac[0],vq->show_mac[1],vq->show_mac[2],vq->show_mac[3],vq->show_mac[4],vq->show_mac[5]);
                                             				fprintf(cgiOut,"<td align=left>%d</td>",vq->vlanid);
                                             				fprintf(cgiOut,"<td align=left>%s</td>","ERR");
                                             				fprintf(cgiOut,"<td align=left>%s</td>","-");
                                             				fprintf(cgiOut,"<td align=left>%s</td>","-");
                                             				fprintf(cgiOut,"<td align=left>%s</td>","-");
                                             				fprintf(cgiOut,"</tr>");
                                     					}
                                     				}
                                     			}
                                     
                                     			else {
                                     				fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));			
                                     				fprintf(cgiOut,"<td align=left>%02x:%02x:%02x:%02x:%02x:%02x</td>",vq->show_mac[0],vq->show_mac[1],vq->show_mac[2],vq->show_mac[3],vq->show_mac[4],vq->show_mac[5]);
                                     				fprintf(cgiOut,"<td align=left>%d</td>",vq->vlanid);
                                     				fprintf(cgiOut,"<td align=left>%d/%d</td>",vq->trans_value1,vq->trans_value2);
                                     				fprintf(cgiOut,"<td align=left>%s</td>","-");
                                     				fprintf(cgiOut,"<td align=left>%s</td>","-");
                                     				fprintf(cgiOut,"<td align=left>%s</td>","-");
                                     				fprintf(cgiOut,"</tr>");				
                                     			}
                                     		}
                                     		else if (vq->dcli_flag == trunk_type){ 				
                                     			fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));		
                                     			fprintf(cgiOut,"<td align=left>%02x:%02x:%02x:%02x:%02x:%02x</td>",vq->show_mac[0],vq->show_mac[1],vq->show_mac[2],vq->show_mac[3],vq->show_mac[4],vq->show_mac[5]);
                                     			fprintf(cgiOut,"<td align=left>%d</td>",vq->vlanid);
                                     			fprintf(cgiOut,"<td align=left>%s</td>","-");
                                     			fprintf(cgiOut,"<td align=left>%d</td>",vq->trans_value1);
                                     			fprintf(cgiOut,"<td align=left>%s</td>","-");
                                     			fprintf(cgiOut,"<td align=left>%s</td>","-");
                                     			fprintf(cgiOut,"</tr>");					        
                                     		}
                                     		else if (vq->dcli_flag == vid_type){				
                                     			fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));		
                                     			fprintf(cgiOut,"<td align=left>%02x:%02x:%02x:%02x:%02x:%02x</td>",vq->show_mac[0],vq->show_mac[1],vq->show_mac[2],vq->show_mac[3],vq->show_mac[4],vq->show_mac[5]);
                                     			fprintf(cgiOut,"<td align=left>%d</td>",vq->vlanid);
                                     			fprintf(cgiOut,"<td align=left>%s</td>","-");
                                     			fprintf(cgiOut,"<td align=left>%s</td>","-");
                                     			fprintf(cgiOut,"<td align=left>%d</td>",vq->trans_value1);
                                     			fprintf(cgiOut,"<td align=left>%s</td>","-");
                                     			fprintf(cgiOut,"</tr>");   
                                     
                                     		}			
                                     		else if (vq->dcli_flag == vidx_type){				
                                     			fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));	
                                     			fprintf(cgiOut,"<td align=left>%02x:%02x:%02x:%02x:%02x:%02x</td>",vq->show_mac[0],vq->show_mac[1],vq->show_mac[2],vq->show_mac[3],vq->show_mac[4],vq->show_mac[5]);
                                     			fprintf(cgiOut,"<td align=left>%d</td>",vq->vlanid);
                                     			fprintf(cgiOut,"<td align=left>%s</td>","-");
                                     			fprintf(cgiOut,"<td align=left>%s</td>","-");
                                     			fprintf(cgiOut,"<td align=left>%s</td>","-");
                                     			fprintf(cgiOut,"<td align=left>%d</td>",vq->trans_value1);
                                     			fprintf(cgiOut,"</tr>"); 				       
                                     		}
                                     		else {
                                     		fprintf(cgiOut,"<div><tr height = 30><td><font color=red>%s</font></td></tr></div>",search(lcon,"show_err"));
                                     			//return -1;;
                                     		}
                                     		
                                             cl = !cl;
                                     		vq=vq->next;
                                     		}
                                     }
								else
									ShowAlert(search(lcon,"vid_not_null"));
								switch(result5)
								{	
									case -1:
										fprintf(cgiOut,"<div><tr height = 30><td><font color=red>%s</font></td></tr></div>",search(lcon,"show_err"));
										break;
									case -2:
										fprintf(cgiOut,"<div><tr height = 30><td><font color=red>%s</font></td></tr></div>",search(lcon,"fdb_none"));
										break;	
									case -3:
										fprintf(cgiOut,"<div><tr height = 30><td><font color=red>%s</font></td></tr></div>",search(lcon,"vlan_form"));
										break;
									case -4:
										fprintf(cgiOut,"<div><tr height = 30><td><font color=red>%s</font></td></tr></div>",search(lcon,"vid_std"));
										break;
									case -5:
										fprintf(cgiOut,"<div><tr height = 30><td><font color=red>%s</font></td></tr></div>",search(lcon,"vlan_not_exist"));
										break;
								}
							}
							else if(strcmp(vlantype,"2")==0)
							{
							    strcpy(TY,"2");
								fprintf(cgiOut,"<tr height=25 bgcolor=#eaeff9 style=font-size:14px align=left>");
                             	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","MAC");
                             	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","VLAN-NAME");
                             	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","SLOT/PORT");
                             	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","TRUNK");
                             	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","VID");
                             	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","VIDX");
                             	fprintf(cgiOut,"</tr>");

										
								if(strcmp(vlanname,"")!=0)
									//result5 = show_fdb_by_vlanname(vlanname);//show by vlanname
									
									{
                                        strcpy(VN,vlanname);
										
                                     	result5 = show_fdb_by_vlanname(vlanname,&vid_head,&vid_num);                                     	
                                        vq=vid_head.next;
                                     	
                                     	for(i=0;i<head;i++)
                                     		vq=vq->next;
                                     	
                                     	for(i=head;i<tail;i++)
                                     		{
                                     
                                     	if (vq->dcli_flag == port_type){
                                     			if(CPU_PORT_VIRTUAL_SLOT == vq->trans_value1) { //static FDB to CPU
                                     				if(CPU_PORT_VIRTUAL_PORT == vq->trans_value2)
                                     					{
                                             				fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));		
                                             				fprintf(cgiOut,"<td align=left>%02x:%02x:%02x:%02x:%02x:%02x</td>",vq->show_mac[0],vq->show_mac[1],vq->show_mac[2],vq->show_mac[3],vq->show_mac[4],vq->show_mac[5]);
                                             				fprintf(cgiOut,"<td align=left>%s</td>",vq->vname);
                                             				fprintf(cgiOut,"<td align=left>%s</td>","CPU");
                                             				fprintf(cgiOut,"<td align=left>%s</td>","-");
                                             				fprintf(cgiOut,"<td align=left>%s</td>","-");
                                             				fprintf(cgiOut,"<td align=left>%s</td>","-");
                                             				fprintf(cgiOut,"</tr>");	
                                     					}
                                     				else
                                     					{
                                     						fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));		
                                             				fprintf(cgiOut,"<td align=left>%02x:%02x:%02x:%02x:%02x:%02x</td>",vq->show_mac[0],vq->show_mac[1],vq->show_mac[2],vq->show_mac[3],vq->show_mac[4],vq->show_mac[5]);
                                             				fprintf(cgiOut,"<td align=left>%s</td>",vq->vname);
                                             				fprintf(cgiOut,"<td align=left>%s</td>","SPI:ERR");
                                             				fprintf(cgiOut,"<td align=left>%s</td>","-");
                                             				fprintf(cgiOut,"<td align=left>%s</td>","-");
                                             				fprintf(cgiOut,"<td align=left>%s</td>","-");
                                             				fprintf(cgiOut,"</tr>");
                                     					}
                                     			}
                                     
                                     			else {
                                     				fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));			
                                     				fprintf(cgiOut,"<td align=left>%02x:%02x:%02x:%02x:%02x:%02x</td>",vq->show_mac[0],vq->show_mac[1],vq->show_mac[2],vq->show_mac[3],vq->show_mac[4],vq->show_mac[5]);
                                     				fprintf(cgiOut,"<td align=left>%s</td>",vq->vname);
                                     				fprintf(cgiOut,"<td align=left>%d/%d</td>",vq->trans_value1,vq->trans_value2);
                                     				fprintf(cgiOut,"<td align=left>%s</td>","-");
                                     				fprintf(cgiOut,"<td align=left>%s</td>","-");
                                     				fprintf(cgiOut,"<td align=left>%s</td>","-");
                                     				fprintf(cgiOut,"</tr>");				
                                     			}
                                     		}
                                     		else if (vq->dcli_flag == trunk_type){ 				
                                     			fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));		
                                     			fprintf(cgiOut,"<td align=left>%02x:%02x:%02x:%02x:%02x:%02x</td>",vq->show_mac[0],vq->show_mac[1],vq->show_mac[2],vq->show_mac[3],vq->show_mac[4],vq->show_mac[5]);
                                     			fprintf(cgiOut,"<td align=left>%s</td>",vq->vname);
                                     			fprintf(cgiOut,"<td align=left>%s</td>","-");
                                     			fprintf(cgiOut,"<td align=left>%d</td>",vq->trans_value1);
                                     			fprintf(cgiOut,"<td align=left>%s</td>","-");
                                     			fprintf(cgiOut,"<td align=left>%s</td>","-");
                                     			fprintf(cgiOut,"</tr>");					        
                                     		}
                                     		else if (vq->dcli_flag == vid_type){				
                                     			fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));		
                                     			fprintf(cgiOut,"<td align=left>%02x:%02x:%02x:%02x:%02x:%02x</td>",vq->show_mac[0],vq->show_mac[1],vq->show_mac[2],vq->show_mac[3],vq->show_mac[4],vq->show_mac[5]);
                                     			fprintf(cgiOut,"<td align=left>%s</td>",vq->vname);
                                     			fprintf(cgiOut,"<td align=left>%s</td>","-");
                                     			fprintf(cgiOut,"<td align=left>%s</td>","-");
                                     			fprintf(cgiOut,"<td align=left>%d</td>",vq->trans_value1);
                                     			fprintf(cgiOut,"<td align=left>%s</td>","-");
                                     			fprintf(cgiOut,"</tr>");   
                                     
                                     		}			
                                     		else if (vq->dcli_flag == vidx_type){				
                                     			fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));	
                                     			fprintf(cgiOut,"<td align=left>%02x:%02x:%02x:%02x:%02x:%02x</td>",vq->show_mac[0],vq->show_mac[1],vq->show_mac[2],vq->show_mac[3],vq->show_mac[4],vq->show_mac[5]);
                                     			fprintf(cgiOut,"<td align=left>%s</td>",vq->vname);
                                     			fprintf(cgiOut,"<td align=left>%s</td>","-");
                                     			fprintf(cgiOut,"<td align=left>%s</td>","-");
                                     			fprintf(cgiOut,"<td align=left>%s</td>","-");
                                     			fprintf(cgiOut,"<td align=left>%d</td>",vq->trans_value1);
                                     			fprintf(cgiOut,"</tr>"); 				       
                                     		}
                                     		else {
										fprintf(cgiOut,"<div><tr height = 30><td><font color=red>%s</font></td></tr></div>",search(lcon,"show_err"));
                                     			//return -1;;
                                     		}
                                     		
                                             cl = !cl;
                                     		vq=vq->next;
                                     		}
                                     }
								else
									ShowAlert(search(lcon,"vlanname_null"));
								switch(result5)
								{	
									case -1:
										fprintf(cgiOut,"<div><tr height = 30><td><font color=red>%s</font></td></tr></div>",search(lcon,"show_err"));
										break;
									case -2:
										fprintf(cgiOut,"<div><tr height = 30><td><font color=red>%s</font></td></tr></div>",search(lcon,"fdb_none"));
										break;	
									case -5:
										fprintf(cgiOut,"<div><tr height = 30><td><font color=red>%s</font></td></tr></div>",search(lcon,"vlan_not_exist"));
										break;
								}
							}	
						}
						else if(strcmp(showtype,"6")==0)  //条例很少，不用着分页的。唯一
						{ 
						    strcpy(ID,"6");
							result6 = -1;
							if(strcmp(vlanid,"")!=0 && strcmp(mac,"")!=0)
								{
								strcpy(VID,vlanid);
								strcpy(MAC,mac);

								fprintf(cgiOut,"<tr height=25 bgcolor=#eaeff9 style=font-size:14px>");
                        		fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","MAC");
                        		fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","VLAN");
                        		fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","SLOT/PORT");
                        		fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","TRUNK");
                        		fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","VID");
                        		fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","VIDX");
                        		fprintf(cgiOut,"</tr>");
								
								result6 = show_fdb_by_mac_vid(mac, vlanid,&macvid);//show by vlan and mac

								if (macvid.dcli_flag == port_type){
                                    			if(CPU_PORT_VIRTUAL_SLOT == macvid.trans_value1) { //static FDB to CPU
                                    				if(CPU_PORT_VIRTUAL_PORT == macvid.trans_value2)
                                    					{
                                            				fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));		
                                            				fprintf(cgiOut,"<td align=left>%02x:%02x:%02x:%02x:%02x:%02x</td>",macvid.show_mac[0],macvid.show_mac[1],macvid.show_mac[2],macvid.show_mac[3],macvid.show_mac[4],macvid.show_mac[5]);
                                            				fprintf(cgiOut,"<td align=left>%d</td>",macvid.vlanid);
                                            				fprintf(cgiOut,"<td align=left>%s</td>","CPU");
                                            				fprintf(cgiOut,"<td align=left>%s</td>","-");
                                            				fprintf(cgiOut,"<td align=left>%s</td>","-");
                                            				fprintf(cgiOut,"<td align=left>%s</td>","-");
                                            				fprintf(cgiOut,"</tr>");	
                                    					}
                                    				else
                                    					{
                                    						fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));		
                                            				fprintf(cgiOut,"<td align=left>%02x:%02x:%02x:%02x:%02x:%02x</td>",macvid.show_mac[0],macvid.show_mac[1],macvid.show_mac[2],macvid.show_mac[3],macvid.show_mac[4],macvid.show_mac[5]);
                                            				fprintf(cgiOut,"<td align=left>%d</td>",macvid.vlanid);
                                            				fprintf(cgiOut,"<td align=left>%s</td>","SPI:ERR");
                                            				fprintf(cgiOut,"<td align=left>%s</td>","-");
                                            				fprintf(cgiOut,"<td align=left>%s</td>","-");
                                            				fprintf(cgiOut,"<td align=left>%s</td>","-");
                                            				fprintf(cgiOut,"</tr>");
                                    					}
                                    			}
                                    			else {
                                    				//vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-8d%5d/%-6d%-8s%-8s%-6s\n",   show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],	vlanid,trans_value1,trans_value2," - "," - "," - ");
                                    				fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));		
                                    				fprintf(cgiOut,"<td align=left>%02x:%02x:%02x:%02x:%02x:%02x</td>",macvid.show_mac[0],macvid.show_mac[1],macvid.show_mac[2],macvid.show_mac[3],macvid.show_mac[4],macvid.show_mac[5]);
                                    				fprintf(cgiOut,"<td align=left>%d</td>",macvid.vlanid);
                                    				fprintf(cgiOut,"<td align=left>%d/%d</td>",macvid.trans_value1,macvid.trans_value2);
                                    				fprintf(cgiOut,"<td align=left>%s</td>","-");
                                    				fprintf(cgiOut,"<td align=left>%s</td>","-");
                                    				fprintf(cgiOut,"<td align=left>%s</td>","-");
                                    				fprintf(cgiOut,"</tr>");							
                                    			}
                                    		}
                                    
                                    		else if (macvid.dcli_flag == trunk_type){ 				
                                    			//vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-8d%-11s%-8d%-8s%-6s\n",    show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],	vlanid," - ",trans_value1," - "," - ");
                                    			fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));		
                                    			fprintf(cgiOut,"<td align=left>%02x:%02x:%02x:%02x:%02x:%02x</td>",macvid.show_mac[0],macvid.show_mac[1],macvid.show_mac[2],macvid.show_mac[3],macvid.show_mac[4],macvid.show_mac[5]);
                                    			fprintf(cgiOut,"<td align=left>%d</td>",macvid.vlanid);
                                    			fprintf(cgiOut,"<td align=left>%s</td>","-");
                                    			fprintf(cgiOut,"<td align=left>%d</td>",macvid.trans_value1);
                                    			fprintf(cgiOut,"<td align=left>%s</td>","-");
                                    			fprintf(cgiOut,"<td align=left>%s</td>","-");
                                    			fprintf(cgiOut,"</tr>");				        
                                    		}
                                    		else if (macvid.dcli_flag == vidx_type){				
                                    			//vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-8d%-11s%-8s%-8d%-6s\n",     show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],		vlanid," - "," - ",trans_value1," - ");
                                    			fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));		
                                    			fprintf(cgiOut,"<td align=left>%02x:%02x:%02x:%02x:%02x:%02x</td>",macvid.show_mac[0],macvid.show_mac[1],macvid.show_mac[2],macvid.show_mac[3],macvid.show_mac[4],macvid.show_mac[5]);
                                    			fprintf(cgiOut,"<td align=left>%d</td>",macvid.vlanid);
                                    			fprintf(cgiOut,"<td align=left>%s</td>","-");
                                    			fprintf(cgiOut,"<td align=left>%s</td>","-");
                                    			fprintf(cgiOut,"<td align=left>%d</td>",macvid.trans_value1);
                                    			fprintf(cgiOut,"<td align=left>%s</td>","-");
                                    			fprintf(cgiOut,"</tr>");  					       
                                    		}			
                                    		else if (macvid.dcli_flag == vid_type){				
                                    			//vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-8d%-11s%-8s%-8s%-8d\n",     show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],		vlanid," - "," - "," - ",trans_value1);
                                    			fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));	
                                    			fprintf(cgiOut,"<td align=left>%02x:%02x:%02x:%02x:%02x:%02x</td>",macvid.show_mac[0],macvid.show_mac[1],macvid.show_mac[2],macvid.show_mac[3],macvid.show_mac[4],macvid.show_mac[5]);
                                    			fprintf(cgiOut,"<td align=left>%d</td>",macvid.vlanid);
                                    			fprintf(cgiOut,"<td align=left>%s</td>","-");
                                    			fprintf(cgiOut,"<td align=left>%s</td>","-");
                                    			fprintf(cgiOut,"<td align=left>%s</td>","-");
                                    			fprintf(cgiOut,"<td align=left>%d</td>",macvid.trans_value1);
                                    			fprintf(cgiOut,"</tr>"); 				       
                                    		}
                                    		else 
											{
									             fprintf(cgiOut,"<div><tr height = 30><td><font color=red>%s</font></td></tr></div>",search(lcon,"show_err"));
                                    		}
							}
							else if(strcmp(vlanid,"")==0)
								ShowAlert(search(lcon,"vid_not_null"));
							else
								ShowAlert(search(lcon,"mac_null"));
							switch(result6)
							{	
								case -1:
									fprintf(cgiOut,"<div><tr height = 30><td><font color=red>%s</font></td></tr></div>",search(lcon,"show_err"));
									break;
								case -2:
									fprintf(cgiOut,"<div><tr height = 30><td><font color=red>%s</font></td></tr></div>",search(lcon,"fdb_none"));
									break;	
								case -3:
									fprintf(cgiOut,"<div><tr height = 30><td><font color=red>%s</font></td></tr></div>",search(lcon,"mac_form"));
									break;
								case -4:
									fprintf(cgiOut,"<div><tr height = 30><td><font color=red>%s</font></td></tr></div>",search(lcon,"vlan_form"));
									break;
								case -5:
									fprintf(cgiOut,"<div><tr height = 30><td><font color=red>%s</font></td></tr></div>",search(lcon,"vlan_out_range"));
									break;	
								case 1:
									ShowAlert(search(lcon,"oper_fail"));
									break;
								case 2:
								   ShowAlert(search(lcon,"vlan_not_exist"));
									break;
								case 3:
									ShowAlert(search(lcon,"illegal_input"));
									break;
								case 4:
									ShowAlert(search(lcon,"HW_error"));
									break;
								case 5:
									ShowAlert(search(lcon,"mac_confilt"));
									break;

							}

						}
						else if(strcmp(showtype,"7")==0)   //分页，传值 MAC
						{
						    strcpy(ID,"7");

							fprintf(cgiOut,"<tr height=25 bgcolor=#eaeff9 style=font-size:14px align=left>");
                         	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","MAC");
                         	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","VLAN");
                         	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","SLOT/PORT");
                         	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","TRUNK");
                         	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","VID");
                         	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","VIDX");
                         	fprintf(cgiOut,"</tr>");

							
							result7 = -1;
							if(strcmp(mac,"")!=0)
								{
							    strcpy(MAC,mac);
							
								//result7 = show_fdb_by_mac(mac);//show by mac
								result7 = show_fdb_by_mac(mac,&m_head,&m_num);
								mq=m_head.next;

								for(i=0;i<head;i++)
								 mq=mq->next;

								for(i=head;i<tail;i++)
                               	{
                               
                               		if (mq->dcli_flag == port_type){
                               			if(CPU_PORT_VIRTUAL_SLOT == mq->trans_value1) { //static FDB to CPU
                               				if(CPU_PORT_VIRTUAL_PORT == mq->trans_value2)
                               					{
                               						fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));		
                               						fprintf(cgiOut,"<td align=left>%02x:%02x:%02x:%02x:%02x:%02x</td>",mq->show_mac[0],mq->show_mac[1],mq->show_mac[2],mq->show_mac[3],mq->show_mac[4],mq->show_mac[5]);
                               						fprintf(cgiOut,"<td align=left>%d</td>",mq->vlanid);
                               						fprintf(cgiOut,"<td align=left>%s</td>","CPU");
                               						fprintf(cgiOut,"<td align=left>%s</td>","-");
                               						fprintf(cgiOut,"<td align=left>%s</td>","-");
                               						fprintf(cgiOut,"<td align=left>%s</td>","-");
                               						fprintf(cgiOut,"</tr>");	
                               					}
                               				else 
                               				{
												if(SPI_PORT_VIRTUAL_PORT == mq->trans_value2)
                               					{
                               						fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));		
                               						fprintf(cgiOut,"<td align=left>%02x:%02x:%02x:%02x:%02x:%02x</td>",mq->show_mac[0],mq->show_mac[1],mq->show_mac[2],mq->show_mac[3],mq->show_mac[4],mq->show_mac[5]);
                               						fprintf(cgiOut,"<td align=left>%d</td>",mq->vlanid);
                               						fprintf(cgiOut,"<td align=left>%s</td>","HSC");
                               						fprintf(cgiOut,"<td align=left>%s</td>","-");
                               						fprintf(cgiOut,"<td align=left>%s</td>","-");
                               						fprintf(cgiOut,"<td align=left>%s</td>","-");
                               						fprintf(cgiOut,"</tr>");
                               					}
												else
												{
                               						fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));		
                               						fprintf(cgiOut,"<td align=left>%02x:%02x:%02x:%02x:%02x:%02x</td>",mq->show_mac[0],mq->show_mac[1],mq->show_mac[2],mq->show_mac[3],mq->show_mac[4],mq->show_mac[5]);
                               						fprintf(cgiOut,"<td align=left>%d</td>",mq->vlanid);
                               						fprintf(cgiOut,"<td align=left>%s</td>","ERR");
                               						fprintf(cgiOut,"<td align=left>%s</td>","-");
                               						fprintf(cgiOut,"<td align=left>%s</td>","-");
                               						fprintf(cgiOut,"<td align=left>%s</td>","-");
                               						fprintf(cgiOut,"</tr>");
                               					}
                               				}
                               			}
                               			else {
                               				fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));			
                               				fprintf(cgiOut,"<td align=left>%02x:%02x:%02x:%02x:%02x:%02x</td>",mq->show_mac[0],mq->show_mac[1],mq->show_mac[2],mq->show_mac[3],mq->show_mac[4],mq->show_mac[5]);
                               				fprintf(cgiOut,"<td align=left>%d</td>",mq->vlanid);
                               				fprintf(cgiOut,"<td align=left>%d/%d</td>",mq->trans_value1,mq->trans_value2);
                               				fprintf(cgiOut,"<td align=left>%s</td>","-");
                               				fprintf(cgiOut,"<td align=left>%s</td>","-");
                               				fprintf(cgiOut,"<td align=left>%s</td>","-");
                               				fprintf(cgiOut,"</tr>");								
                               			}
                               		}
                               		else if (mq->dcli_flag == trunk_type){ 				
                               			fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));		
                               			fprintf(cgiOut,"<td align=left>%02x:%02x:%02x:%02x:%02x:%02x</td>",mq->show_mac[0],mq->show_mac[1],mq->show_mac[2],mq->show_mac[3],mq->show_mac[4],mq->show_mac[5]);
                               			fprintf(cgiOut,"<td align=left>%d</td>",mq->vlanid);
										fprintf(cgiOut,"<td align=left>%s</td>","-");
                               			fprintf(cgiOut,"<td align=left>%d</td>",mq->trans_value1);
                               			fprintf(cgiOut,"<td align=left>%s</td>","-");
                               			fprintf(cgiOut,"<td align=left>%s</td>","-");
                               			fprintf(cgiOut,"</tr>");					        
                               		}
                               		else if (mq->dcli_flag == vid_type){				
                               			fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));		
                               			fprintf(cgiOut,"<td align=left>%02x:%02x:%02x:%02x:%02x:%02x</td>",mq->show_mac[0],mq->show_mac[1],mq->show_mac[2],mq->show_mac[3],mq->show_mac[4],mq->show_mac[5]);
                               			fprintf(cgiOut,"<td align=left>%d</td>",mq->vlanid);
                               			fprintf(cgiOut,"<td align=left>%s</td>","-");
                               			fprintf(cgiOut,"<td align=left>%s</td>","-");
                               			fprintf(cgiOut,"<td align=left>%s</td>","-");
                               			fprintf(cgiOut,"<td align=left>%d</td>",mq->trans_value1);
                               			fprintf(cgiOut,"</tr>");  				       
                               		}			
                               		else if (mq->dcli_flag == vidx_type)
										{				
                               			fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));	
                               			fprintf(cgiOut,"<td align=left>%02x:%02x:%02x:%02x:%02x:%02x</td>",mq->show_mac[0],mq->show_mac[1],mq->show_mac[2],mq->show_mac[3],mq->show_mac[4],mq->show_mac[5]);
                               			fprintf(cgiOut,"<td align=left>%d</td>",mq->vlanid);
                               			fprintf(cgiOut,"<td align=left>%s</td>","-");
                               			fprintf(cgiOut,"<td align=left>%s</td>","-");
                               			fprintf(cgiOut,"<td align=left>%d</td>",mq->trans_value1);
                               			fprintf(cgiOut,"<td align=left>%s</td>","-");										
                               			fprintf(cgiOut,"</tr>"); 					       
                               		     }
                               		else {
                               		    fprintf(cgiOut,"<div><tr height = 30><td><font color=red>%s</font></td></tr></div>",search(lcon,"show_err"));
                               			//return -1;;
                               		     }
                               
       									cl =!cl;
       									mq=mq->next;
                                  }
								}
							else
								ShowAlert(search(lcon,"mac_null"));
							switch(result7)
							{	
								case -1:
									fprintf(cgiOut,"<div><tr height = 30><td><font color=red>%s</font></td></tr></div>",search(lcon,"show_err"));
									break;
								case -2:
									fprintf(cgiOut,"<div><tr height = 30><td><font color=red>%s</font></td></tr></div>",search(lcon,"fdb_none"));
									break;	
								case -3:
									fprintf(cgiOut,"<div><tr height = 30><td><font color=red>%s</font></td></tr></div>",search(lcon,"mac_form"));
									break;
							}

						}
						else if(strcmp(showtype,"8")==0)
						{
						    strcpy(ID,"8");

							fprintf(cgiOut,"<tr height=25 bgcolor=#eaeff9 style=font-size:14px align=left>");
                         	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","MAC");
                         	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","VLAN");
                         	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","SLOT/PORT");
                         	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","TRUNK");
                         	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","VID");
                         	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","VIDX");
                         	fprintf(cgiOut,"</tr>");
							
							result8 = -1;
							if(strcmp(port,"")!=0)
								{
								strcpy(PT,port);
								
								//result8 = show_fdb_by_port(port);//show by port
							    result8 = show_fdb_by_port(&pt_head,&p_num,port);
							    pq=pt_head.next;

								for(i=0;i<head;i++)  
									pq=pq->next;

								for(i=head;i<tail;i++)
                              		{
                              
                              
                              		if (pq->dcli_flag == port_type){
                              			if(CPU_PORT_VIRTUAL_SLOT == pq->trans_value1) { //static FDB to CPU
                              				if(CPU_PORT_VIRTUAL_PORT == pq->trans_value2)
                              					{
                                      				fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));		
                                      				fprintf(cgiOut,"<td align=left>%02x:%02x:%02x:%02x:%02x:%02x</td>",pq->show_mac[0],pq->show_mac[1],pq->show_mac[2],pq->show_mac[3],pq->show_mac[4],pq->show_mac[5]);
                                      				fprintf(cgiOut,"<td align=left>%d</td>",pq->vlanid);
                                      				fprintf(cgiOut,"<td align=left>%s</td>","CPU");
                                      				fprintf(cgiOut,"<td align=left>%s</td>","-");
                                      				fprintf(cgiOut,"<td align=left>%s</td>","-");
                                      				fprintf(cgiOut,"<td align=left>%s</td>","-");
                                      				fprintf(cgiOut,"</tr>");	
                              					}
                              				else
                              				{
                              					if(SPI_PORT_VIRTUAL_PORT == pq->trans_value2)
												{
                              						fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));		
                                      				fprintf(cgiOut,"<td align=left>%02x:%02x:%02x:%02x:%02x:%02x</td>",pq->show_mac[0],pq->show_mac[1],pq->show_mac[2],pq->show_mac[3],pq->show_mac[4],pq->show_mac[5]);
                                      				fprintf(cgiOut,"<td align=left>%d</td>",pq->vlanid);
                                      				fprintf(cgiOut,"<td align=left>%s</td>","HSC");
                                      				fprintf(cgiOut,"<td align=left>%s</td>","-");
                                      				fprintf(cgiOut,"<td align=left>%s</td>","-");
                                      				fprintf(cgiOut,"<td align=left>%s</td>","-");
                                      				fprintf(cgiOut,"</tr>");
                              					}
												else
												{
                              						fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));		
                                      				fprintf(cgiOut,"<td align=left>%02x:%02x:%02x:%02x:%02x:%02x</td>",pq->show_mac[0],pq->show_mac[1],pq->show_mac[2],pq->show_mac[3],pq->show_mac[4],pq->show_mac[5]);
                                      				fprintf(cgiOut,"<td align=left>%d</td>",pq->vlanid);
                                      				fprintf(cgiOut,"<td align=left>%s</td>","ERR");
                                      				fprintf(cgiOut,"<td align=left>%s</td>","-");
                                      				fprintf(cgiOut,"<td align=left>%s</td>","-");
                                      				fprintf(cgiOut,"<td align=left>%s</td>","-");
                                      				fprintf(cgiOut,"</tr>");
                              					}
							
                              				}
                              			}
                              		  else {
                              				fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));			
                              				fprintf(cgiOut,"<td align=left>%02x:%02x:%02x:%02x:%02x:%02x</td>",pq->show_mac[0],pq->show_mac[1],pq->show_mac[2],pq->show_mac[3],pq->show_mac[4],pq->show_mac[5]);
                              				fprintf(cgiOut,"<td align=left>%d</td>",pq->vlanid);
                              				fprintf(cgiOut,"<td align=left>%d/%d</td>",pq->trans_value1,pq->trans_value2);
                              				fprintf(cgiOut,"<td align=left>%s</td>","-");
                              				fprintf(cgiOut,"<td align=left>%s</td>","-");
                              				fprintf(cgiOut,"<td align=left>%s</td>","-");
                              				fprintf(cgiOut,"</tr>");			
                              			}
                              		}
                              		else if (pq->dcli_flag == trunk_type){ 				
                              			fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));		
                              			fprintf(cgiOut,"<td align=left>%02x:%02x:%02x:%02x:%02x:%02x</td>",pq->show_mac[0],pq->show_mac[1],pq->show_mac[2],pq->show_mac[3],pq->show_mac[4],pq->show_mac[5]);
                              			fprintf(cgiOut,"<td align=left>%s</td>","-");
                              			fprintf(cgiOut,"<td align=left>%d</td>",pq->trans_value1);
                              			fprintf(cgiOut,"<td align=left>%s</td>","-");
                              			fprintf(cgiOut,"<td align=left>%s</td>","-");
                              			fprintf(cgiOut,"</tr>");				        
                              		}
                              		else if (pq->dcli_flag == vid_type){				
                              			fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));		
                              			fprintf(cgiOut,"<td align=left>%02x:%02x:%02x:%02x:%02x:%02x</td>",pq->show_mac[0],pq->show_mac[1],pq->show_mac[2],pq->show_mac[3],pq->show_mac[4],pq->show_mac[5]);
                              			fprintf(cgiOut,"<td align=left>%d</td>",pq->vlanid);
                              			fprintf(cgiOut,"<td align=left>%s</td>","-");
                              			fprintf(cgiOut,"<td align=left>%s</td>","-");
                              			fprintf(cgiOut,"<td align=left>%d</td>",pq->trans_value1);
                              			fprintf(cgiOut,"<td align=left>%s</td>","-");
                              			fprintf(cgiOut,"</tr>");   				       
                              		}			
                              		else if (pq->dcli_flag == vidx_type){				
                              			fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));	
                              			fprintf(cgiOut,"<td align=left>%02x:%02x:%02x:%02x:%02x:%02x</td>",pq->show_mac[0],pq->show_mac[1],pq->show_mac[2],pq->show_mac[3],pq->show_mac[4],pq->show_mac[5]);
                              			fprintf(cgiOut,"<td align=left>%d</td>",pq->vlanid);
                              			fprintf(cgiOut,"<td align=left>%s</td>","-");
                              			fprintf(cgiOut,"<td align=left>%s</td>","-");
                              			fprintf(cgiOut,"<td align=left>%s</td>","-");
                              			fprintf(cgiOut,"<td align=left>%d</td>",pq->trans_value1);
                              			fprintf(cgiOut,"</tr>"); 				       
                              		}
                              		else {
                              	     fprintf(cgiOut,"<div><tr height = 30><td><font color=red>%s</font></td></tr></div>",search(lcon,"show_err"));
                              
                              		}
                              		
                              
                                                                  cl = !cl;
                              									pq=pq->next;
                              	  }


								}

								
							else
								ShowAlert(search(lcon,"port_not_null"));
							switch(result8)
							{	
								case -1:
									fprintf(cgiOut,"<div><tr height = 30><td><font color=red>%s</font></td></tr></div>",search(lcon,"show_err"));
									break;
								case -2:
									fprintf(cgiOut,"<div><tr height = 30><td><font color=red>%s</font></td></tr></div>",search(lcon,"fdb_none"));
									break;	
								case -3:
									fprintf(cgiOut,"<div><tr height = 30><td><font color=red>%s</font></td></tr></div>",search(lcon,"port_format"));
									break;
								case -4:
									fprintf(cgiOut,"<div><tr height = 30><td><font color=red>%s</font></td></tr></div>",search(lcon,"port_out_range"));
									break;
							}

						}
						else if(strcmp(showtype,"9")==0)
						{	
						    strcpy(ID,"9");

							fprintf(cgiOut,"<tr height=25 bgcolor=#eaeff9 style=font-size:14px align=left>");
                         	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","VLAN");
                         	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","SLOT/PORT");
                         	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","TRUNK");
                         	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","NUMBER");
                         	fprintf(cgiOut,"</tr>");
							
							result9 = -1;
							//result9 = show_fdb_number_limit();//show all
							result9 = show_fdb_number_limit(&lm_head,&l_num);
							lq=lm_head.next;

							for(i=0;i<head;i++)
								lq=lq->next;
							
							for(i=head;i<tail;i++)
                            {
                                 if(0 == lq->vlanid){
                        		   fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));	   
                        		   fprintf(cgiOut,"<td align=left>%s</td>","-");
                        		   fprintf(cgiOut,"<td align=left>%d/%d</td>",lq->slotNum,lq->portNum);
                        		   fprintf(cgiOut,"<td align=left>%s</td>","-");
                        		   fprintf(cgiOut,"<td align=left>%d</td>",lq->fdbnumber);
                        		   fprintf(cgiOut,"</tr>");    
                                }
                        		else if((0 == lq->slotNum)&&( 0 == lq->portNum)){
                        		   fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));	   
                        		   fprintf(cgiOut,"<td align=left>%d</td>",lq->vlanid);
                        		   fprintf(cgiOut,"<td align=left>%s/%s</td>","-","-");
                        		   fprintf(cgiOut,"<td align=left>%s</td>","-");
                        		   fprintf(cgiOut,"<td align=left>%d</td>",lq->fdbnumber);
                        		   fprintf(cgiOut,"</tr>"); 
                        		}
                        		else{
                        			fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));		
                        			fprintf(cgiOut,"<td align=left>%d</td>",lq->vlanid);
                        			fprintf(cgiOut,"<td align=left>%d/%d</td>",lq->slotNum,lq->portNum);
                        			fprintf(cgiOut,"<td align=left>%s</td>","-");
                        			fprintf(cgiOut,"<td align=left>%d</td>",lq->fdbnumber);
                        			fprintf(cgiOut,"</tr>"); 
                        		}
                                                        cl = !cl;
                        								lq=lq->next;
                        	}

							
							switch(result9)
							{	
								case -1:
									fprintf(cgiOut,"<div><tr height = 30><td><font color=red>%s</font></td></tr></div>",search(lcon,"show_err"));
									break;
								case -2:
									fprintf(cgiOut,"<div><tr height = 30><td><font color=red>%s</font></td></tr></div>",search(lcon,"no_limit"));
									break;	
							}
						}
						else
						{
						    strcpy(ID,"1");
							strcpy(TY,"1");
							result1 = -1;
							
							
          							fprintf(cgiOut,"<tr height=25 bgcolor=#eaeff9 style=font-size:14px align=left>");
                                  	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","MAC");
                                  	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","VLAN");
                                  	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","SLOT/PORT");
                                  	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","TRUNK");
                                  	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","VID");
                                  	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","VIDX");
                                  	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","TYPE");
                                  	fprintf(cgiOut,"</tr>");
									

							result1=show_fdb(&f_head,&all_num);
							fq=f_head.next;
							
							for(i=0;i<head;i++)  //这样就可以去掉前面的数据了，像是挪到指定位置。
							fq=fq->next;
						    							   
                           	   for(i=head;i<tail;i++)  //应该输出链表的指定项       
                          		  {
                                
                                			if (fq->dcli_flag == port_type){
                                			if(CPU_PORT_VIRTUAL_SLOT == fq->trans_value1) { //static FDB to CPU
                                				if(CPU_PORT_VIRTUAL_PORT == fq->trans_value2)
                                					{
                                        				fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));		
                                        				fprintf(cgiOut,"<td align=left>%02x:%02x:%02x:%02x:%02x:%02x</td>",fq->show_mac[0],fq->show_mac[1],fq->show_mac[2],fq->show_mac[3],fq->show_mac[4],fq->show_mac[5]);
                                				        fprintf(cgiOut,"<td align=left>%d</td>",fq->vlanid);
														fprintf(cgiOut,"<td align=left>%s</td>","CPU");
                                        				fprintf(cgiOut,"<td align=left>%s</td>","-");
                                        				fprintf(cgiOut,"<td align=left>%s</td>","-");
                                        				fprintf(cgiOut,"<td align=left>%s</td>","-");
                                						fprintf(cgiOut,"<td align=left>%s</td>","-");
                                        				fprintf(cgiOut,"</tr>");	
                                					}
                                				else
                                					{
                                						fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));		
                                        				fprintf(cgiOut,"<td align=left>%02x:%02x:%02x:%02x:%02x:%02x</td>",fq->show_mac[0],fq->show_mac[1],fq->show_mac[2],fq->show_mac[3],fq->show_mac[4],fq->show_mac[5]);
                                				        fprintf(cgiOut,"<td align=left>%d</td>",fq->vlanid);
                                        				fprintf(cgiOut,"<td align=left>%d</td>",fq->vlanid);
                                        				fprintf(cgiOut,"<td align=left>%s</td>","SPI:ERR");
                                        				fprintf(cgiOut,"<td align=left>%s</td>","-");
                                        				fprintf(cgiOut,"<td align=left>%s</td>","-");
                                        				fprintf(cgiOut,"<td align=left>%s</td>","-");
                                						fprintf(cgiOut,"<td align=left>%s</td>","-");
                                        				fprintf(cgiOut,"</tr>");
                                					}
                                			}
                                			else {
                                				fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));			
                                				fprintf(cgiOut,"<td align=left>%02x:%02x:%02x:%02x:%02x:%02x</td>",fq->show_mac[0],fq->show_mac[1],fq->show_mac[2],fq->show_mac[3],fq->show_mac[4],fq->show_mac[5]);
                                				fprintf(cgiOut,"<td align=left>%d</td>",fq->vlanid);
                                				fprintf(cgiOut,"<td align=left>%d/%d</td>",fq->trans_value1,fq->trans_value2);
                                				fprintf(cgiOut,"<td align=left>%s</td>","-");
                                				fprintf(cgiOut,"<td align=left>%s</td>","-");
                                				fprintf(cgiOut,"<td align=left>%s</td>","-");
                                				fprintf(cgiOut,"<td align=left>%s</td>",fq->type);
                                				fprintf(cgiOut,"</tr>");				
                                			}
                                		}
                                		else if (fq->dcli_flag == trunk_type){ 				
                                			fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));		
                                			fprintf(cgiOut,"<td align=left>%02x:%02x:%02x:%02x:%02x:%02x</td>",fq->show_mac[0],fq->show_mac[1],fq->show_mac[2],fq->show_mac[3],fq->show_mac[4],fq->show_mac[5]);
                                			fprintf(cgiOut,"<td align=left>%d</td>",fq->vlanid);
                                			fprintf(cgiOut,"<td align=left>%s</td>","-");
                                			fprintf(cgiOut,"<td align=left>%d</td>",fq->trans_value1);
                                			fprintf(cgiOut,"<td align=left>%s</td>","-");
                                			fprintf(cgiOut,"<td align=left>%s</td>","-");
                                			fprintf(cgiOut,"<td align=left>%s</td>",fq->type);
                                			fprintf(cgiOut,"</tr>");		        
                                		}
                                		
                                		else if (fq->dcli_flag == vid_type){				
                                			fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));		
                                			fprintf(cgiOut,"<td align=left>%02x:%02x:%02x:%02x:%02x:%02x</td>",fq->show_mac[0],fq->show_mac[1],fq->show_mac[2],fq->show_mac[3],fq->show_mac[4],fq->show_mac[5]);
                                			fprintf(cgiOut,"<td align=left>%d</td>",fq->vlanid);
                                			fprintf(cgiOut,"<td align=left>%s</td>","-");
                                			fprintf(cgiOut,"<td align=left>%s</td>","-");
                                			fprintf(cgiOut,"<td align=left>%d</td>",fq->trans_value1);
                                			fprintf(cgiOut,"<td align=left>%s</td>","-");
                                			fprintf(cgiOut,"<td align=left>%s</td>",fq->type);
                                			fprintf(cgiOut,"</tr>");    
                                		}			
                                		else if (fq->dcli_flag == vidx_type){				
                                			fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));	
                                			fprintf(cgiOut,"<td align=left>%02x:%02x:%02x:%02x:%02x:%02x</td>",fq->show_mac[0],fq->show_mac[1],fq->show_mac[2],fq->show_mac[3],fq->show_mac[4],fq->show_mac[5]);
                                			fprintf(cgiOut,"<td align=left>%d</td>",fq->vlanid);
                                			fprintf(cgiOut,"<td align=left>%s</td>","-");
                                			fprintf(cgiOut,"<td align=left>%s</td>","-");
                                			fprintf(cgiOut,"<td align=left>%s</td>","-");
                                			fprintf(cgiOut,"<td align=left>%d</td>",fq->trans_value1);
                                			fprintf(cgiOut,"<td align=left>%s</td>",fq->type);
                                			fprintf(cgiOut,"</tr>");  		       
                                		}
                                		else {
									fprintf(cgiOut,"<div><tr height = 30><td><font color=red>%s</font></td></tr></div>",search(lcon,"show_err"));
                                		     }

                                         cl = !cl;
									     fq=fq->next;
                                		  }		
							
							switch(result1)
							{	
								case -1:
									fprintf(cgiOut,"<div><tr height = 30><td><font color=red>%s</font></td></tr></div>",search(lcon,"show_err"));
									break;
								case -2:
									fprintf(cgiOut,"<div><tr height = 30><td><font color=red>%s</font></td></tr></div>",search(lcon,"fdb_none"));
									break;	
							}
						}
  
  fprintf(cgiOut,"</table></div>");



					fprintf(cgiOut,"</td>"\
							  "</tr>");


 
                        //分割开一段距离
                          fprintf(cgiOut,"<tr height=15><td></td></tr>");
					      fprintf(cgiOut,"<tr>");
						  fprintf(cgiOut,"<td colspan=4>"\
							  "<table width=430 style=padding-top:2px>"\
							  "<tr>");

						
						  sprintf(pageNumCF,"%d",FirstPage);
						  sprintf(pageNumCA,"%d",pageNum+1);
						  sprintf(pageNumCD,"%d",pageNum-1);
						  sprintf(pageNumCL,"%d",LastPage);

				         if(cgiFormSubmitClicked("submit_fdb") != cgiFormSuccess)
								  {  
									 fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_show_fdb.cgi?UN=%s&PN=%s&SN=%s&ID=%s&VID=%s&TY=%s&VN=%s&MAC=%s&PT=%s >%s</td>",encry,pageNumCF,"PageFirst",ID,VID,TY,VN,MAC,PT,search(lcon,"Page_First"));
									 fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_show_fdb.cgi?UN=%s&PN=%s&SN=%s&ID=%s&VID=%s&TY=%s&VN=%s&MAC=%s&PT=%s >%s</td>",encry,pageNumCD,"PageUp",ID,VID,TY,VN,MAC,PT,search(lcon,"page_up"));
									 fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_show_fdb.cgi?UN=%s&PN=%s&SN=%s&ID=%s&VID=%s&TY=%s&VN=%s&MAC=%s&PT=%s >%s</td>",encry,pageNumCA,"PageDown",ID,VID,TY,VN,MAC,PT,search(lcon,"page_down"));
									 fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_show_fdb.cgi?UN=%s&PN=%s&SN=%s&ID=%s&VID=%s&TY=%s&VN=%s&MAC=%s&PT=%s >%s</td>",encry,pageNumCL,"PageLast",ID,VID,TY,VN,MAC,PT,search(lcon,"Page_Last"));
							      }
						else
								  {

									  fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_show_fdb.cgi?UN=%s&PN=%s&SN=%s&ID=%s&VID=%s&TY=%s&VN=%s&MAC=%s&PT=%s >%s</td>",fdb_encry,pageNumCF,"PageFirst",ID,VID,TY,VN,MAC,PT,search(lcon,"Page_First"));
									  fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_show_fdb.cgi?UN=%s&PN=%s&SN=%s&ID=%s&VID=%s&TY=%s&VN=%s&MAC=%s&PT=%s >%s</td>",fdb_encry,pageNumCD,"PageUp",ID,VID,TY,VN,MAC,PT,search(lcon,"page_up"));  
									  fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_show_fdb.cgi?UN=%s&PN=%s&SN=%s&ID=%s&VID=%s&TY=%s&VN=%s&MAC=%s&PT=%s >%s</td>",fdb_encry,pageNumCA,"PageDown",ID,VID,TY,VN,MAC,PT,search(lcon,"page_down"));
									  fprintf(cgiOut,"<td align=center style=padding-top:2px><a href=wp_show_fdb.cgi?UN=%s&PN=%s&SN=%s&ID=%s&VID=%s&TY=%s&VN=%s&MAC=%s&PT=%s >%s</td>",fdb_encry,pageNumCL,"PageLast",ID,VID,TY,VN,MAC,PT,search(lcon,"Page_Last"));
								  }
                           fprintf(cgiOut,"</tr>"\
							  "<tr height=30  align=center valign=bottom>");

     if(LastPage==0)
     	{
     	}
     else
     	{
	 fprintf(cgiOut,"<td colspan=4>%s%d%s(%s%d%s)</td>",search(lpublic,"current_sort"),pageNum+1,search(lpublic,"page"),search(lpublic,"total"),LastPage,search(lpublic,"page"));
     	}
						   fprintf(cgiOut,"</tr>"\
							  	"</table></td>"\
								  "</tr>");

	/*---------- e e ------------------------*/		
	
                         //点击按钮
                           fprintf(cgiOut,"<tr>");							  
								  if(cgiFormSubmitClicked("submit_fdb") != cgiFormSuccess)
								  {
									fprintf(cgiOut,"<td><input type=hidden name=fdb_encry value=%s></td>",encry);
									fprintf(cgiOut,"<td><input type=hidden name=CheckUsr value=%d></td>",retu);

									fprintf(cgiOut,"<td><input type=hidden name=old_vid value=\"%s\"></td>",VID);
									fprintf(cgiOut,"<td><input type=hidden name=old_vname value=\"%s\"></td>",VN);
									fprintf(cgiOut,"<td><input type=hidden name=old_mac value=\"%s\"></td>",MAC);
									fprintf(cgiOut,"<td><input type=hidden name=old_port value=\"%s\"></td>",PT);

									
								  }
								  else if(cgiFormSubmitClicked("submit_fdb") == cgiFormSuccess)
									  { 			 
										fprintf(cgiOut,"<td><input type=hidden name=fdb_encry value=%s></td>",fdb_encry);
										fprintf(cgiOut,"<td><input type=hidden name=CheckUsr value=%d></td>",retu);

										fprintf(cgiOut,"<td><input type=hidden name=old_vid value=\"%s\"></td>",VID);
										fprintf(cgiOut,"<td><input type=hidden name=old_vname value=\"%s\"></td>",VN);
										fprintf(cgiOut,"<td><input type=hidden name=old_mac value=\"%s\"></td>",MAC);
									    fprintf(cgiOut,"<td><input type=hidden name=old_port value=\"%s\"></td>",PT);

								  }
				fprintf(cgiOut,"</tr>");
							fprintf(cgiOut,"</table>"\


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

	free(encry);
	free(CheckUsr);		
	
	if((result1==1)&&(all_num>0))   //释放总的fdb数目
	Free_fdb_all(&f_head);

	if((result2==1)&&(s_num>0)) //释放静态fdb
	Free_fdb_all(&s_head);

	if((result3==1)&&(d_num>0)) //释放动态fdb
	Free_fdb_all(&d_head);

	if((result4==1)&&(bk_num>0)) //释放黑名单
	Free_fdb_blacklist(&bk_head);

	if((result5==1)&&(vid_num>0)) //释放vlan vid,vname信息
	Free_fdb_all(&vid_head);

	if((result7==1)&&(m_num>0)) //释放vlan mac信息
	Free_fdb_all(&m_head);


	if((result8==1)&&(p_num>0)) //释放vlan port信息
	Free_fdb_all(&pt_head);

	if((result9==1)&&(l_num>0)) //释放vlan limit信息
	Free_fdb_limit(&lm_head);
	
	return 0;

}



