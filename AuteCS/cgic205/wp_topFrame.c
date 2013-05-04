/* cgicTempDir is the only setting you are likely to need
	to change in this file. */

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
* wp_topFrame.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* qiaojie@autelan.com
*
* DESCRIPTION:
*
*
*
*******************************************************************************/


#include <stdio.h>
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include "ws_ec.h"
#include "ws_usrinfo.h"
#include "ws_err.h"
#include <sys/stat.h>
#include <sys/wait.h>
#include "ws_list_container.h"
#include "ws_secondary_container.h"   //引用的新的
#include "ws_license.h"   //控制条目是否出现

#ifndef LT
#define LT get_decrypt_content()
#endif

void ShowtopFramePage(char *m,char *n,char *t,struct list *lpublic,char *Src);   /*m代表用户名，n代表加密后的字符串*/

int cgiMain()
{
  char *encry=(char *)malloc(BUF_LEN);               /*存储从wp_Frameset.cgi带入的加密字符串*/
  
  char *str;        
  char lan[10];
  struct list *lpublic;   /*解析public.txt文件的链表头*/
  lpublic=get_chain_head("../htdocs/text/public.txt");
  memset(lan,0,10);
  memset(encry,0,BUF_LEN);
  char Src[128];
  memset(Src,0,128);
  
  if(cgiFormStringNoNewlines("UN", encry, BUF_LEN)!=cgiFormNotFound )  /*首次进入该页，lan为浏览器默认语言*/
  {
    cgiFormStringNoNewlines("LAN", lan, 10); 	
	
    str=dcryption(encry);
    if(str==NULL)
     ShowErrorPage(search(lpublic,"ill_user"));		 /*用户非法*/
    else
    {
     if(LT !=NULL)
     strcpy(Src,LT);
	 else
	 strcpy(Src,"");

     ShowtopFramePage(str,encry,lan,lpublic,Src); 
    }
  }
  else                       /*修改select之后,lan为上次选中的语言*/
  {
    cgiFormStringNoNewlines("en_ch",lan,10);
    cgiFormStringNoNewlines("encry_top", encry, BUF_LEN);	

	str=dcryption(encry);
    if(str==NULL)
     ShowErrorPage(search(lpublic,"ill_user"));		 /*用户非法*/
    else
    {
     if(LT !=NULL)
     strcpy(Src,LT);
	 else
	 strcpy(Src,"");
	 
     ShowtopFramePage(str,encry,lan,lpublic,Src); 
    }
  }
  free(encry);
  release(lpublic);  

  return 0;
}

void ShowtopFramePage(char *m,char *n,char *t,struct list *lpublic,char *Src)
{ 
  cgiHeaderContentType("text/html");
 fprintf(cgiOut,"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">"\
 "<html xmlns=\"http://www.w3.org/1999/xhtml\">"\
"<head>"\
"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=gb2312\"/>"\
"<title>top_bar</title>"\
"<link rel=stylesheet href=/style.css type=text/css>"\
"<style type=text/css>"\
  "#save {width:22px; height:20px; border:0;}"\
"</style>"\
"</head>"\
  "<script type=\"text/JavaScript\">"\
  "function d()"\
  "{"\
  "var dc=document.getElementById(\"top\");"\
  "dc.submit();"\
  "window.parent.frames.mainFrame.location.reload();"\
  "}"\
  "function MM_preloadImages()"\
  "{"\
    "var d=document; "\
    "if(d.images)"\
    "{ "\
      "if(!d.MM_p) d.MM_p=new Array();"\
        "var i,j=d.MM_p.length,a=MM_preloadImages.arguments;"\
  	  "for(i=0; i<a.length; i++)"\
        "if (a[i].indexOf(\"#\")!=0)"\
   	    "{"
   	      "d.MM_p[j]=new Image;"\
   		  "d.MM_p[j++].src=a[i];"\
   	    "}"\
     "}"\
   "}"\
   "function MM_swapImgRestore()"\
   "{"\
     "var i,x,a=document.MM_sr;"\
     "for(i=0;a&&i<a.length&&(x=a[i])&&x.oSrc;i++)"\
       "x.src=x.oSrc;"\
   "}"\
   "function MM_findObj(n, d)"\
   "{"\
     "var p,i,x;"\
     "if(!d)"\
       "d=document;"\
     "if((p=n.indexOf(\"?\"))>0&&parent.frames.length)"\
     "{"\
       "d=parent.frames[n.substring(p+1)].document;"\
       "n=n.substring(0,p);"\
     "}"\
     "if(!(x=d[n])&&d.all)"\
       "x=d.all[n];"\
     "for (i=0;!x&&i<d.forms.length;i++)"\
       "x=d.forms[i][n];"\
     "for(i=0;!x&&d.layers&&i<d.layers.length;i++)"\
       "x=MM_findObj(n,d.layers[i].document);"\
     "if(!x && d.getElementById)"\
       "x=d.getElementById(n);"\
     "return x;"\
   "}"\
   "function MM_swapImage()"\
   "{"\
      "var i,j=0,x,a=MM_swapImage.arguments;"\
      "document.MM_sr=new Array;"\
      "for(i=0;i<(a.length-2);i+=3)"\
       "if ((x=MM_findObj(a[i]))!=null)"\
   	   "{"\
   	      "document.MM_sr[j++]=x;"\
   	      "if(!x.oSrc) x.oSrc=x.src;"\
   	        "x.src=a[i+2];"\
   	   "}"\
   "}"\
	"function reflesh_url(url)"\
	"{"
		"var splitstr = url.substring(0,url.indexOf(\"&\"));"\
		"if (splitstr == \"\")"\
		"{"\
			"return url;"\
		"}"\
		"else"\
		"{"\
			"return splitstr;"\
		"}"\
	"}"
	"function refresh()"\
	"{"\
	   "var a=window.parent.frames.mainFrame.location.href;"\
	   "var flag_pos=a.indexOf(\"&SubmitFlag=\");"\
	   "if(flag_pos!=-1)"\
	   "{"\
	       "var url=a.substring(0,flag_pos);"\
	   	   "window.parent.frames.mainFrame.location.href=url;"\
	   "}"\
	   "else"\
	   "{"\
	   	   "window.parent.frames.mainFrame.location.href=a;"\
	   "}"\
	"}" );

fprintf( cgiOut, "function window.onbeforeunload(){"\
				"if((event.clientX>document.body.clientWidth-12)&&event.clientY<0 || event.altKey){"\
						   "window.location='wp_delsession.cgi?UN=%s';", n );
                     fprintf( cgiOut,"}"\
	            "else if(event.clientY>parent.document.body.clientHeight || event.altKey){"\
	            "window.location='wp_delsession.cgi?UN=%s';", n );	            
	                  fprintf( cgiOut,"}"\
	            "else{"\
						  ";"\
					"}"\
			   " } \n\n\n" );
/*add by shaojunwu*/

//如果这里设置为１，则正常操作的用户不会超时，
fprintf( cgiOut, "function refresh_session()\n" );
fprintf( cgiOut, "	{\n" );
fprintf( cgiOut, "		var sessionid='%s';\n", n );
fprintf( cgiOut, "		var url='wp_help.cgi?UN='+sessionid;\n" );
fprintf( cgiOut, "		var headobj = document.getElementsByTagName('head')[0];\n" );
fprintf( cgiOut, "		var name='test_iframe';\n"); 
fprintf( cgiOut, "		var noblock_element = document.getElementsByName(name)[0];\n"\
				"		if( null != noblock_element )\n"\
				"		{\n"\
				"			headobj.removeChild(noblock_element);\n"
				"		}\n"\
				"		var noblock_element=document.createElement('iFrame');\n"\
				"		noblock_element.src=url;\n"\
				"		noblock_element.type = 'text/html';\n"\
				"		noblock_element.visibility = 'hidden';\n"\
				"		noblock_element.name = name;\n"\
				"		headobj.appendChild(noblock_element);\n"\
				"	}\n");
				//"	setInterval('refresh_session();', %d);\n\n\n\n",((unsigned int)DIFF_TIME-5)*1000 );//DIFF_TIME定义的是超时的秒数
/*这里使用了wp_help.cgi去请求server的方式来定时跟新session的last time,这样当ie开着的时候，就不会出现超时*/
/*最早定义超时的一个主要目的就是当用户非法退出时，没有删除其session
通过超时检查来删除sessionid,否则该用户就没有办法登录了。
导致的问题就是，调试很不方便。
加入这段代码后，自动去刷新，只需要将刷新的时间设置的比超时短就可以了。

*/

/*end of add by shaojunwu*/
fprintf( cgiOut,  "</script>"\
  "<body onload=\"MM_preloadImages('/images/xitong-da.jpg','/images/anquan-da.jpg','/images/kongzhi-da.jpg','/images/xingneng-da.jpg','/images/wuxian-da.jpg','/images/bangzhu-da.jpg','/images/firewall-da.jpg')\">"\
  "<form method=post id=top >"\
  "<div align=center style=\"margin-right:15px\">"\
  "<table width=976 border=0 cellspacing=0>"\
  "<tr>"\
    "<td width=303 align=left><img src=/images/logo.jpg height=44/></td>"\
    "<td width=700 align=right valign=bottom><table width=420 border=0 align=right cellpadding=0 cellspacing=0>"\
      "<tr>"\
      "<td width=125 align=center class=duq2></td>"\
        "<td width=37 align=center><img src=/images/yonghu.jpg width=22 height=21/></td>");
        if(checkuser_group(m)==0)
		  fprintf(cgiOut,"<td width=40 align=left>Admin</td>");
		else if(checkuser_group(m)==1)
			   fprintf(cgiOut,"<td width=40 align=left>User</td>");	
		//add by qiandawei 2008.08.28
        if(checkuser_group(m)==0)
        	{
		fprintf(cgiOut,"<td width=22 align=center><input id=save type=submit name=save_config style=\"background-image:url(/images/save.jpg); cursor:hand\" value=""></td>");
        
		fprintf(cgiOut,"<td width=40 align=center><input type=submit name=save_config value=%s id=%s style='border:0;background-color:#fff;color:#0a3a75;cursor:hand; font-size: 12px;'/></td>",
					search(lpublic,"save"), search(lpublic,"top_font"));
         }
		char *cgiRequestMethod = NULL;
		cgiGetenv_func(&cgiRequestMethod, "REQUEST_METHOD");
		if((cgiFormSubmitClicked("save_config") == cgiFormSuccess)&&(0 == strcmp(cgiRequestMethod,"POST")))
		{
			//int status_m =system("srvsave.sh");
			int status = system("save_config.sh > /dev/null");

			int ret = WEXITSTATUS(status);

			if(ret==0)
			{
				ShowAlert(search(lpublic,"oper_succ"));
			}
			else
			{
				ShowAlert(search(lpublic,"oper_fail"));
			}
		}

		//end
		fprintf(cgiOut,"<td width=22 align=center><img src=/images/gengxin.jpg width=22 height=20 onclick=window.parent.frames.mainFrame.location.reload() style=\"cursor:hand\"/></td>"\
		  "<td width=55 align=center><a href=# onclick='refresh()' style=\"cursor:hand\"><font id=%s>%s</font></a></td>"\
		  "<td width=22 align=center><a href=wp_delsession.cgi?UN=%s target=_top><img src=/images/tuichu.jpg width=22 height=20 border=0/></td>"\
		  "<td width=57 align=center><a href=wp_delsession.cgi?UN=%s target=_top><font id=%s>%s</font></a></td>",
		  search(lpublic,"top_font"), search(lpublic,"refresh"), n, n, search(lpublic,"top_font"), search(lpublic,"log_out") );	

		fprintf(cgiOut,"</tr>"\
    "</table></td>"\
  "</tr>\n"\
  "<tr>\n"\
  "	<td colspan=2>\n"
  "		<table width=976 border=0 cellpadding=0 cellspacing=0>\n"\
  "			<tr>\n");  

		{
			char *img_exten;
			char loc_url[128];

			img_exten = search( lpublic, "img_language_ex" );//将语言的相关后缀写在配置文件中
			if( NULL== img_exten )
			{
				img_exten = ".jpg";
			}
			
			sprintf( loc_url, "wp_topFrame.cgi?UN=%s", n );

	


////////////////////////
            int flag,set_f=-1;
            flag=if_show_sndr_icon();
           
			if(flag==1)
    		{
                //中间来处理函数
				set_f=-1;
				set_f=get_license_state_new(SYSTEM_ITEM,Src);
				if( set_f == 0 )
				{
                  fprintf(cgiOut,"		<td width=95 align=left valign=top ><a href=wp_sysmagic.cgi?UN=%s target=mainFrame onmouseout=\"MM_swapImgRestore()\" onmouseover=\"MM_swapImage('xitong','','/images/xitong-da%s',1);\"><img src=/images/xitong-xi%s name=xitong width=95 height=56 border=0 id=xitong /></a></td>\n",n,img_exten, img_exten);
       			}
				else
				{
                  fprintf(cgiOut,"		<td width=14 align=left valign=center background=/images/di-xiao.jpg><img src=/images/left_top.jpg width=20 height=56 border=0 id=xitong /></td>\n");
				}
				////////////////
				set_f=-1;
				set_f=get_license_state_new(CONTRL_ITEM,Src);
				if( set_f == 0 )
				   fprintf(cgiOut,"		<td width=95 align=left valign=top background=/images/di-xiao.jpg><a href=wp_contrl.cgi?UN=%s target=mainFrame onmouseout=\"MM_swapImgRestore()\" onmouseover=\"MM_swapImage('kongzhi','','/images/kongzhi-da%s',1)\"><img src=/images/kongzhi-xi%s name=kongzhi width=95 height=56 border=0 id=kongzhi /></a></td>\n",n, img_exten, img_exten);
				//////////////////
				set_f=-1;				
				set_f=get_license_state_new(WLAN_ITEM,Src);
				if( set_f == 0 )
				   fprintf(cgiOut,"		<td width=95 align=left valign=top background=/images/di-xiao.jpg><a href=wp_wlan.cgi?UN=%s target=mainFrame onmouseout=\"MM_swapImgRestore()\" onmouseover=\"MM_swapImage('wuxian','','/images/wuxian-da%s',1)\"><img src=/images/wuxian-xi%s name=wuxian width=95 height=56 border=0 id=wuxian /></a></td>\n",n, img_exten, img_exten);
    			///////////////////	
				set_f=-1;
				set_f=get_license_state_new(AUTH_ITEM,Src);
				if( set_f == 0 )
				   fprintf(cgiOut,"		<td width=95 align=left valign=top background=/images/di-xiao.jpg><a href=wp_authentication.cgi?UN=%s target=mainFrame onmouseout=\"MM_swapImgRestore()\" onmouseover=\"MM_swapImage('anquan','','/images/anquan-da%s',1)\"><img src=/images/anquan-xi%s name=anquan width=95 height=56 border=0 id=anquan /></a></td>\n",n, img_exten, img_exten);
    			///////////////////////	
				set_f=-1;
				set_f=get_license_state_new(FIREWALL_ITEM,Src);
				if( set_f == 0 )
				   fprintf(cgiOut,"		<td width=95 align=left valign=top background=/images/di-xiao.jpg><a href=wp_firewall.cgi?UN=%s target=mainFrame onmouseout=\"MM_swapImgRestore()\" onmouseover=\"MM_swapImage('firewall','','/images/firewall-da%s',1)\"><img src=/images/firewall-xi%s name=firewall width=95 height=56 border=0 id=firewall /></a></td>\n",n, img_exten, img_exten);
				//////////////////////////////
				set_f=-1;
				set_f=get_license_state_new(SYS_ITEM,Src);
				if( set_f == 0 )
				   fprintf(cgiOut,"		<td width=95 align=left valign=top background=/images/di-xiao.jpg><a href=wp_per.cgi?UN=%s target=mainFrame onmouseout=\"MM_swapImgRestore()\" onmouseover=\"MM_swapImage('xingneng','','/images/xingneng-da%s',1)\"><img src=/images/xingneng-xi%s name=xingneng width=95 height=56 border=0 id=xingneng /></a></td>\n",n, img_exten, img_exten);
				///////////////////////		
				set_f=-1;
				set_f=get_license_state_new(HELP_ITEM,Src);
				if( set_f == 0 )
				   fprintf(cgiOut,"		<td width=95 align=left valign=top background=/images/di-xiao.jpg><a href=wp_help.cgi?UN=%s target=mainFrame onmouseout=\"MM_swapImgRestore()\" onmouseover=\"MM_swapImage('bangzhu','','/images/bangzhu-da%s',1)\"><img src=/images/bangzhu-xi%s name=bangzhu width=95 height=56 border=0 id=bangzhu /></a></td>\n",n, img_exten, img_exten);
				/////////////////////////////
			}
		
			else
    		{       	
    		    //中间来处理函数
    		    set_f=get_license_state_new(SYSTEM_ITEM,Src);
                if( set_f == 0 )
				{
				fprintf(cgiOut,"		<td width=14 align=left valign=center background=/images/di-xiao.jpg><img src=/images/left_top.jpg width=20 height=56 border=0 id=xitong /></td>\n");
				fprintf(cgiOut,"		<td width=81 align=left valign=center background=/images/di-xiao.jpg ><a href=wp_sysmagic.cgi?UN=%s target=mainFrame><font id=%s size=3><b>%s</b></font></a></td>\n",n,search(lpublic,"memu_er"),search(lpublic,"top_system"));
				}
				else
				{
				  fprintf(cgiOut,"		<td width=14 align=left valign=center background=/images/di-xiao.jpg><img src=/images/left_top.jpg width=20 height=56 border=0 id=xitong /></td>\n");
				}
				set_f=get_license_state_new(CONTRL_ITEM,Src);
                if( set_f == 0 )
				fprintf(cgiOut,"		<td width=95 align=left valign=center background=/images/di-xiao.jpg ><a href=wp_contrl.cgi?UN=%s target=mainFrame><font id=%s size=3><b>%s</b></font></a></td>\n",n,search(lpublic,"memu_er"),search(lpublic,"top_contrl"));

                set_f=get_license_state_new(WLAN_ITEM,Src);
                if( set_f == 0 )
				fprintf(cgiOut,"		<td width=95 align=left valign=center background=/images/di-xiao.jpg ><a href=wp_wlan.cgi?UN=%s target=mainFrame><font id=%s size=3><b>%s</b></font></a></td>\n",n,search(lpublic,"memu_er"),search(lpublic,"top_wlan"));

                set_f=get_license_state_new(AUTH_ITEM,Src);
                if( set_f == 0 )
				fprintf(cgiOut,"		<td width=95 align=left valign=center background=/images/di-xiao.jpg ><a href=wp_authentication.cgi?UN=%s target=mainFrame><font id=%s size=3><b>%s</b></font></a></td>\n",n,search(lpublic,"memu_er"),search(lpublic,"top_auth"));

                set_f=get_license_state_new(FIREWALL_ITEM,Src);
                if( set_f == 0 )
				fprintf(cgiOut,"		<td width=95 align=left valign=center background=/images/di-xiao.jpg ><a href=wp_firewall.cgi?UN=%s target=mainFrame><font id=%s size=3><b>&nbsp;&nbsp;%s</b></font></a></td>\n",n,search(lpublic,"memu_er"),search(lpublic,"top_firewall"));

                set_f=get_license_state_new(SYS_ITEM,Src);
                if( set_f == 0 )
				fprintf(cgiOut,"		<td width=95 align=left valign=center background=/images/di-xiao.jpg ><a href=wp_per.cgi?UN=%s target=mainFrame><font id=%s size=3><b>&nbsp;%s</b></font></a></td>\n",n,search(lpublic,"memu_er"),search(lpublic,"top_sys"));

                set_f=get_license_state_new(HELP_ITEM,Src);
                if( set_f == 0 )
				fprintf(cgiOut,"		<td width=95 align=left valign=center background=/images/di-xiao.jpg ><a href=wp_help.cgi?UN=%s target=mainFrame><font id=%s size=3><b>&nbsp;&nbsp;%s</b></font></a></td>\n",n,search(lpublic,"memu_er"),search(lpublic,"top_help"));
       		}


		}
	
		fprintf(cgiOut,"		<td align=left valign=top background=/images/di-xiao.jpg>&nbsp;</td>\n"\
        "		<td align=right valign=top background=/images/di-xia1.jpg><img src=/images/di.jpg width=49 height=56 /></td>\n"\
      "	</tr>\n");
  fprintf(cgiOut,"</table></td>"\
  "</tr>"\
  "<tr>"\
  "<td colspan=2><input type=hidden name=encry_top value=%s></td>",n);
  fprintf(cgiOut,"</tr>"\
"</table>"\
"</div>"\
"</form>"\
"</body>"\
"</html>");
}

