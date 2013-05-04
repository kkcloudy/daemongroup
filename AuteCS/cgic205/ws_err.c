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
* ws_err.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* qiaojie@autelan.com
* tangsq@autelan.com
*
* DESCRIPTION:
*
*
*
*******************************************************************************/


#include "cgic.h"
#include "ws_err.h"
#include "ws_ec.h"

#define SCRIPT_NAME "/cgi-bin/wp_login.cgi"
void ShowErrorPage(char *message)
{
   struct list *lpublic;   /*解析public.txt文件的链表头*/
   lpublic=get_chain_head("../htdocs/text/public.txt");
   cgiHeaderCookieSetString("AuteLAN","",-3600,SCRIPT_NAME,cgiServerName);
   cgiHeaderContentType("text/html");
   fprintf(cgiOut,"<html><head>");
   fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
   fprintf(cgiOut,"<title>%s</title>",search(lpublic,"returnlogonpage"));
   fprintf(cgiOut,"</head><body topmargin=200 leftmargin=350>"\
/*   "<form method=post action=/index.html target=_top>"\
   "<table width=200 border=0 cellspacing=0 cellpadding=0>"\
   "<tr>"\
    "<td>"\
	"<div align=center>"\
	  "<table border=0 cellspacing=0 cellpadding=0>"\
        "<tr>"\
          "<td width=306 height=53><img src=/images/error_1.gif width=306 height=53/></td>"\
        "</tr>"\
        "<tr>"\
          "<td><table width=306 height=43 border=0 cellspacing=0 cellpadding=0>"\
            "<tr>"\
              "<td width=83><img src=/images/error_2_1.gif width=83 height=43/></td>"\
              "<td width=190 bgcolor=#ece9d8 nowrap><div align=left><span class=STYLE3>%s</span></div></td>",message);
              fprintf(cgiOut,"<td width=33><img src=/images/error_2_2.gif width=33 height=43/></td>"\
            "</tr>"\
          "</table></td>"\
        "</tr>"\
        "<tr>"\
          "<td><table width=306 height=31 border=0 cellspacing=0 cellpadding=0>"\
            "<tr>"\
              "<td width=42 style='background:url(/images/error_3_1.gif) repeat-y';><!--<img src=/images/error_3_1.gif width=42 height=31/>--></td>"\
              "<td width=221 bgcolor=#ece9d8><div align=center><input type=submit value=%s name=log_on/></div></td>",search(lpublic,"return"));
              fprintf(cgiOut,"<td width=43 style='background:url(/images/error_3_2.gif) repeat-y';><!--<img src=/images/error_3_2.gif width=43 height=31 />--></td>"\
            "</tr>"\
          "</table></td>"\
        "</tr>"\
        "<tr>"\
          "<td width=306 height=50><img src=/images/error_4.gif width=306 height=50/></td>"\
        "</tr>"\
      "</table>"\
	"</div>"\
	"</td>"\
  "</tr>"\
"</table>"\
"</form>"\*/
		"<script type=text/javascript>\n"\
		"if( parent.topFrame )\n"\
   		"{\n"
   		"	window.parent.frames.topFrame.document.body.innerHTML='<p/>';\n"\
   		"}\n"
		"alert( '%s' );\n"\
		"window.parent.location.href='wp_login.cgi';\n"\
		"</script>\n"\
"</body>"\
"</html>", message );
release(lpublic); 
}

void LogoffPage(char *username)
{
	
	struct list *lpublic;
	lpublic=get_chain_head("../htdocs/text/public.txt");
	cgiHeaderContentType("text/html");
	fprintf(cgiOut,"<html><head>");
	fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
	fprintf(cgiOut,"<title>%s</title>",search(lpublic,"returnlogonpage"));
	fprintf(cgiOut,"<style type=text/css></style></head><body topmargin=200 leftmargin=350>");
	fprintf(cgiOut,"<script type='text/javascript'>");	
	fprintf(cgiOut,"if(confirm(\"%s\")){",search(lpublic,"usr_logoff"));
	fprintf(cgiOut,"window.location.href='wp_delsession.cgi?UNAME=%s';\n", username); 	
	fprintf(cgiOut,"}else{"\
	"window.parent.location.href='wp_login.cgi';\n"\
	"}");
	fprintf(cgiOut,"</script>");	
	fprintf(cgiOut,"</body>"\
	"</html>" );
	release(lpublic); 
}

void ShowAlert(char *message)
{
  fprintf(cgiOut,"<SCRIPT  LANGUAGE=JavaScript>"\
  "alert(\"%s\")",message);
  fprintf(cgiOut,"</SCRIPT>");
}
void ShowAlertTwoMsg(char *message1, char *message2)
{
  fprintf(cgiOut,"<SCRIPT  LANGUAGE=JavaScript>"\
  "alert(\"%s\"+\"\\n\"+\"%s\");",message1,message2);
  fprintf(cgiOut,"</SCRIPT>");
}

