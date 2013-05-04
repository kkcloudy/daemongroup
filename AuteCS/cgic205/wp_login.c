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
* wp_login.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* shaojw@autelan.com
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
#include <sys/wait.h>
#include "ws_err.h"
#include "ws_usrinfo.h"
#include "ws_ec.h"

void get_title( char *title, int size );

int cgiMain()
{
	char title[64]="";
	struct list *lpublic;
	
	lpublic=get_chain_head("../htdocs/text/public.txt");
	
	cgiHeaderContentType("text/html");
	
	get_title( title, sizeof(title) );
	
	fprintf( cgiOut,"<html xmlns:v>\n"\
					"	<head>\n"\
					"	<meta http-equiv='content-type' content='text/html; charset=gb2312'>\n"\
  					"	<META   HTTP-EQUIV=\"pragma\"   CONTENT=\"no-cache\"> \n"\
  					"	<META   HTTP-EQUIV=\"Cache-Control\"   CONTENT=\"no-cache,   must-revalidate\"> \n"\
  					"	<META   HTTP-EQUIV=\"expires\"   CONTENT=\"Wed,   26   Feb   1997   08:21:57   GMT\">	\n"\
					"	<title>log in</title>\n"\
					"	<style>\n"\
					"	v\\:*{behavior: url(#default#VML);}\n"\
					"	</style>\n"\
					"	<link rel='stylesheet' href='style.css' type='text/css' />\n"\
					"	<style type='text/css'>\n"\
					"<!--\n"\
					".STYLE2 {\n"\
					"	font-size: 34px;\n"\
					"	font-family: 'Times New Roman', Times, serif;\n"\
					"	font-weight: bold;\n"\
					"	font-style: italic;\n"\
					"	color: #FFFFFF;\n"\
					"}\n"\
					"-->\n"\
					"</style>\n"\
					"</head>\n"\
					"<script>\n"\
					"function x()\n"\
					"{\n"\
  					"	var x = navigator.browserLanguage; \n"\
  					"	var y = document.getElementById('langu');\n"\
  					"	y.value=x;\n"\
					"}\n"\
					"function setTD()\n"\
					"{\n"\
  					"	var x = navigator.browserLanguage; \n"\
  					"	var usrname = document.getElementById('prompt1');\n"\
  					"	var usrpass = document.getElementById('prompt2');\n"\
  					"	var login = document.getElementById('prompt3');\n"\
  					"	if(x=='zh-cn')\n"\
  					"	{\n"\
    				"		usrname.src='/images/a1.png'; \n"\
    				"		usrpass.src='/images/a2.png';\n"\
    				"		login.src='/images/log1.png';\n"\
  					"	}\n"\
  					"	else\n"\
  					"	{\n"\
    				"		usrname.src='/images/b1.png';\n"\
    				"		usrpass.src='/images/b2.png'; \n"\
    				"		login.src='/images/log2.png'; \n"\
  					"	}\n"\
					"}\n"\
					"</script>\n\n\n\n" );
					
fprintf( cgiOut, "<body id='main' style='margin-top:130px' onload='' onkeydown=\"if(event.keyCode=='13'&&event.srcElement.type!='textarea') return false;\">\n"\
				"<form name='indexform' method='post' id='log_in' encType='multipart/form-data' action='/cgi-bin/wp_Frameset.cgi'>\n"\
				"	<div align='center'>\n"\
				"	<table width='659' height='372' border='0' cellspacing='0' cellpadding='0' background='/images/indexbg.jpg'>\n"\
  				"		<tr> <td height='110px' align='center' valign='bottom'>\n" );


  				
fprintf( cgiOut,"<span class='STYLE2'>%s</span>\n", title );


fprintf( cgiOut,"		<td></tr>\n"\
  				"		<tr>\n"\
    			"			<td align='right' style='padding-right:90px; padding-top:0px'>\n"\
      			"				<table width='260' border='0' cellspacing='0' cellpadding='0'>\n"\
        		"					<tr height='40'>\n" );
fprintf( cgiOut,"						<td width='100'><img id='prompt1' src='/images/%s' width='94' height='20'></td>\n",search(lpublic,"user_name"));
fprintf( cgiOut,"						<td width='160'><input type='text' name='user_name' size='20' style='height:25; border:1px solid #000000; width:140;'/></td>\n"\
        		"					</tr>\n"\
        		"					<tr height='40'>\n" );
        		
fprintf( cgiOut,"						<td><img id='prompt2' src='/images/%s' width='94' height='20'></td>\n", search(lpublic,"pass_word") );
          		
fprintf( cgiOut,"						<td><input type='password' name='user_pass' size='20' style='height:25; border:1px solid #000000;width:140;'/></td>\n"\
        		"					</tr>\n"\
        		"					<tr height='40'>\n"\
          		"						<td>&nbsp;</td>\n");
fprintf( cgiOut,"						<td><input id='prompt3' align=left type='image' src='/images/%s' onClick='x()'/></td>\n", search(lpublic,"login_btn") );
fprintf( cgiOut,"					</tr>\n"\
        		"					<tr>\n"\
          		"						<td colspan='2'><input type=hidden id=langu name=LAN></td>\n"\
        		"					</tr>\n"\
      			"				</table>\n"\
    			"			</td>\n"\
  				"		</tr>\n"\
				"	</table>\n"\
				"</div>\n"\
				"</form>\n"\
				"</body>\n"\
				"</html> \n" );
return 0;
}


void get_title( char *title, int size )
{
	//根据实际情况从sysinfo中读取！
	strncpy( title, " ", size );
	return;
}

