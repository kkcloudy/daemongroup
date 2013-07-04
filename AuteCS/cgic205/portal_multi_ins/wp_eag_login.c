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
* wp_eag_login.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
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
#include "ws_eag_login.h"
#include "ws_user_manage.h"

/*redirection*/
int locate(FILE* fp,char *web)
{
	if( fp == NULL || web == NULL )
		return -1;

	fprintf( fp, "<script type='text/javascript'>\n" );
	fprintf( fp, "window.location.href='%s';\n", web );
	fprintf( fp, "</script>\n" );
	return 0;
}


char * replaceStrPart(char *Src, const char * sReplace)
{
	if( Src == NULL || sReplace == NULL )
	{
		return -1;
	}
	char * replace = NULL;
	replace = strrchr(Src, '//');
	//fprintf(stderr,"before replace=%s\n" ,replace );
	int partLen = strlen(replace);
	memset(replace, "\0", partLen);
	//fprintf(stderr,"inner Src=%s\n" ,Src );
	memcpy(replace, sReplace, strlen(sReplace)+1);
	//fprintf(stderr,"later Src=%s\n" ,Src );
	return Src;
}

/*function : send a request pkg to AC auth and wish to get response from that --tangsiqi 2010-1-18*/
int CgiInformAc(char * clientIp, char * serverIp, PKG_TYPE Type, STAuthProcess * pAuthProc,UINT32 pro)
{
		int retErr=0;
		pAuthProc->pSendPkg= createPortalPkg(Type);
		/*malloc STPortalPkg ready to rev data*/
		fprintf(stderr,"CgiInformAc createPortalPkg suc Type is %d\n",Type );
		pAuthProc->pRevPkg = (STPortalPkg * )malloc(sizeof(STPortalPkg));
		memset(pAuthProc->pRevPkg, 0, sizeof(STPortalPkg));
		

		setAuthType(pAuthProc->pSendPkg, pro);
		
		setPkgUserIP( pAuthProc->pSendPkg, htonl(inet_addr(clientIp)) );

		
		if(sendPortalPkg(pAuthProc->fd, 3, 2000, serverIp, pAuthProc->pSendPkg) < 0 )
		{
			fprintf(stderr,"CgiInformAc sendPortalPkg failed\n" );
			retErr = -1;
		}
		else
		{
			fprintf(stderr,"CgiInformAc sendPortalPkg suc\n" );
		}
		
		if(getPortalPkg(pAuthProc->fd, 3, &(pAuthProc->pRevPkg))<0)
		{
			fprintf(stderr,"CgiInformAc getPortalPkg failed\n" );
			retErr = -1;
		}
		else
		{
			fprintf(stderr,"CgiInformAc getPortalPkg suc\n" );
			retErr = getErrCode(pAuthProc->pRevPkg);
		}
		
		
		fprintf(stderr,"CgiInformAc getErrCode(stAuth.pRevPkg)=%d\n", retErr );
		
		
		
	return retErr;
}

/*main--tangsiqi 2010-1-18*/
int cgiMain()
{

	int retLogin=-1,retLogout=-1,ret_challege=0,fd=0;/*0--suc,100--timeout, 減方---fail*/
	unsigned short reqID = 0;
	char opt[10]="";
	FILE * fpOut = cgiOut;
	unsigned char chap_password[MD5LEN + 2] = {0};
	unsigned char chap_challenge[MD5LEN] = {0};
	MD5_CTX context;
	unsigned char chap_ident=0;
	STPkgAttr *tlvPkgAttr;
	UINT8  tmp[MD5LEN+1];
	
	STUserInfo userInfo;
	memset(&userInfo, 0 ,sizeof(STUserInfo));

	cgiHeaderContentType("text/html");

	/*rev user info from login.html*/
	cgiFormStringNoNewlines("op_auth", opt, 10);
	fprintf(stderr,"opt=%s",opt);
	if( strlen(opt)>0 && (!strcmp(opt,"login")) )
	{
		cgiFormStringNoNewlines("a_name", userInfo.usrName, MAX_EAG_LOGIN_NAME_LEN);
		cgiFormStringNoNewlines("a_pass", userInfo.usrPass, MAX_EAG_LOGIN_PASS_LEN);

		userInfo.usrOperation = 1;
	}
	else
	{
		userInfo.usrOperation = 2;/*logout*/
	}
		
	


	fprintf( fpOut, "<html xmlns=\"http://www.w3.org/1999/xhtml\"> \n" );
	fprintf( fpOut, "<head> \n" );
	fprintf( fpOut, "<meta http-equiv=Content-Type content=text/html; charset=gb2312> \n" );


  	fprintf( fpOut, "<META   HTTP-EQUIV=\"pragma\"   CONTENT=\"no-cache\"> \n");
  	fprintf( fpOut, "<META   HTTP-EQUIV=\"Cache-Control\"   CONTENT=\"no-cache,   must-revalidate\"> \n" );
  	fprintf( fpOut, "<META   HTTP-EQUIV=\"expires\"   CONTENT=\"Wed,   26   Feb   1997   08:21:57   GMT\">	\n");


	fprintf( fpOut, "<title>login_proc</title>\n");
	fprintf( fpOut, "</head> \n" );
	fprintf( fpOut, "<boby>\n");

	
	/*process http req and require a auth request with AC*/
	STAuthProcess stAuth;
	memset(&stAuth, 0, sizeof(STAuthProcess));

	cgi_auth_init(&stAuth, 2000);
	STUserManagePkg * pstReq = NULL;
	STUserManagePkg * pstRsp = NULL;
	char urlPost[4096]={0};
	char *urlNew = NULL;
	char *replace = NULL;
	
	fprintf(stderr,"f_name=%s--f_pass=%s--op_auth=%s--cgiRemoteAddr =%s--cgiServerName=%s\n", userInfo.usrName, userInfo.usrPass, opt, cgiRemoteAddr,cgiServerName  );
	#if 1

	fprintf(stderr,"cgiReferrer=%s\n", cgiReferrer  );
	#endif
	strncpy(urlPost, cgiReferrer, strlen(cgiReferrer));
	#if 0
	fprintf(stderr,"before urlPost=%s\n" ,urlPost );
	replace = strrchr(urlPost, '//');
	fprintf(stderr,"before replace=%s\n" ,replace );
	int partLen = strlen(replace);
	memset(replace, "\0", partLen);
	fprintf(stderr,"inner urlPost=%s\n" ,urlPost );
	memcpy(replace, "/auth_suc.html", strlen("/auth_suc.html")+1);
	
	fprintf(stderr,"last urlPost=%s--partLen=%d\n" ,urlPost, partLen );
	#endif
	
	fprintf(stderr,"userInfo.usrOperation=%d\n" ,userInfo.usrOperation );
	switch(userInfo.usrOperation)
 	{
 		case 1:/*login*/
 			//stAuth.protocal = AUTH_CHAP;
 			
 			
 	
 			pstReq =  createRequirePkg(REQ_GET_AUTH_TYPE,NULL,NULL);
 			
 			/*connect unix sock to get auth type*/
 			//fd = suc_connect_unix_sock();
 			//if(fd <= -1)
 			//{
 			//	fprintf(stderr,"suc_connect_unix_sock: error\n");
 			//	break;
 			//}
 			
 			//fprintf(stderr,"fd=%d",fd);
			stAuth.protocal = AUTH_CHAP;//get_authType_from_eag( pstReq, fd, 5, &(pstRsp));
 			//fprintf(stderr,"stAuth.protocal=%d",stAuth.protocal);
 			//close( fd );
 			
 			if( stAuth.protocal == AUTH_CHAP )				/*chap md5 simulation----------*/
 			{
 				ret_challege = CgiInformAc(cgiRemoteAddr, cgiServerName, REQ_CHALLENGE, &stAuth, stAuth.protocal);
 				fprintf(stderr,"ret_challege=%d", ret_challege);
 				if( CHALLENGE_SUCCESS == ret_challege || CHALLENGE_CONNECTED == ret_challege )/*if ret is success ,then can get attr from rev pkg*/
 				{
 					if((tlvPkgAttr = getAttrByAttrType(stAuth.pRevPkg, ATTR_CHALLENGE)) == NULL && CHALLENGE_CONNECTED == ret_challege)
 					{
 						retLogin = 0;/*容僕suc.html*/
 						break;
 					}
 				}
 				else
 				{
 					retLogin = -1;/*容僕fail.html*/
 					break;
 				}
 				memcpy(chap_challenge, tlvPkgAttr->attr_value, tlvPkgAttr->attr_len);
 				fprintf(stderr,"chap_challenge() value %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n", \
 						 chap_challenge[0],chap_challenge[1],chap_challenge[2],
 						chap_challenge[3],chap_challenge[4],chap_challenge[5],chap_challenge[6],chap_challenge[7],chap_challenge[8],chap_challenge[9],
 						chap_challenge[10],chap_challenge[11],chap_challenge[12],chap_challenge[13],chap_challenge[14],chap_challenge[15] );
 				reqID = getRequireID(stAuth.pRevPkg);
 				fprintf(stderr,"CHAP: reqID=%d\n",reqID);
 				unsigned char chap_id = (unsigned char)reqID ;
 				fprintf(stderr, "chap_id=%d\n",chap_id);
 
 				/*simulate MD5 encoded at portal server add by tangsiqi 2010-1-5*/
 
 				MD5Init(&context);
 				MD5Update(&context, (UINT8 *)&chap_id, 1);
 				MD5Update(&context, (UINT8 *)userInfo.usrPass, strlen(userInfo.usrPass));/*now the password is get through by redir url */
 			    MD5Update(&context, chap_challenge, MD5LEN);
 				MD5Final(tmp, &context);
 				tmp[MD5LEN] = 0;/*add 0 at end of char[]*/
 				fprintf(stderr,"CHAP: tmp=%s",tmp);
 
 				memcpy(chap_password, tmp, MD5LEN );
 				chap_password[MD5LEN+1] = 0;
 				fprintf(stderr,"...add attr CHAP_PASSWORD() value %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n", \
 						 chap_password[0],chap_password[1],chap_password[2],
 						chap_password[3],chap_password[4],chap_password[5],chap_password[6],chap_password[7],chap_password[8],chap_password[9],
 						chap_password[10],chap_password[11],chap_password[12],chap_password[13],chap_password[14],chap_password[15] );
 
 				destroyPortalPkg(stAuth.pSendPkg);
 				destroyPortalPkg(stAuth.pRevPkg);
 
 			}
 
 			/*after challege exchange*/
 			stAuth.pSendPkg = createPortalPkg(REQ_AUTH);
 			fprintf(stderr,"login createPortalPkg suc\n" );
 			
 			/*malloc STPortalPkg ready to rev data*/
 			stAuth.pRevPkg = (STPortalPkg * )malloc(sizeof(STPortalPkg));
 			memset(stAuth.pRevPkg, 0, sizeof(STPortalPkg));
 			
 			setAuthType(stAuth.pSendPkg, stAuth.protocal);
 			setRequireID(stAuth.pSendPkg, reqID );
 			setPkgUserIP( stAuth.pSendPkg, htonl(inet_addr(cgiRemoteAddr)) );
 			
 			addAttr( &stAuth.pSendPkg, ATTR_USERNAME, userInfo.usrName, strlen(userInfo.usrName) );
 		
 			if( stAuth.protocal == AUTH_CHAP )
 			{
 				/*challenge exchange*/
 				addAttr( &stAuth.pSendPkg, ATTR_CHAPPASSWORD, chap_password, MD5LEN );
 				
 			}
 			else/*PAP authentication*/
 			{
 				fprintf(stderr,"userInfo.usrPass=%s",userInfo.usrPass);
 				addAttr( &stAuth.pSendPkg, ATTR_PASSWORD, userInfo.usrPass, strlen(userInfo.usrPass) );
 			}
 			
 			
 			
 			
 			if(sendPortalPkg(stAuth.fd, 6, 2000, cgiServerName, stAuth.pSendPkg) < 0 )
 			{
 				fprintf(stderr,"login sendPortalPkg failed\n" );
 				retLogin = -1;
 			}
 			else
 			{
 				fprintf(stderr,"login sendPortalPkg suc\n" );
 			}
 			
 			if(getPortalPkg(stAuth.fd, 12, &(stAuth.pRevPkg))<0)
 			{
 				fprintf(stderr,"login getPortalPkg failed\n" );
 				retLogin = -1;
 			}
 			else
 			{
 				fprintf(stderr,"login getPortalPkg suc\n" );
 			}
 			retLogin = getErrCode(stAuth.pRevPkg);
 			fprintf(stderr,"login getErrCode(stAuth.pRevPkg)=%d\n", retLogin );
 			
 			
 			destroyPortalPkg(stAuth.pSendPkg);
 			destroyPortalPkg(stAuth.pRevPkg);
 			break;
 		case 2:/*logout*/
 			retLogout = CgiInformAc(cgiRemoteAddr, cgiServerName, REQ_LOGOUT, &stAuth, stAuth.protocal);
 			destroyPortalPkg(stAuth.pSendPkg);
 			destroyPortalPkg(stAuth.pRevPkg);
 			break;
 		default: break;
 	}
	
	
	fprintf(stderr,"retLogin=%d---retLogout=%d\n" ,retLogin,retLogout );
	closePkgSock(&stAuth);
	if( retLogin==100 ||  retLogout==100 )/*time out will retry,reserve*/
	{
		fprintf( fpOut, "<table border=0 cellspacing=0 cellpadding=0><tr><td colspan=2>time out!please retry or return</td></tr>\n");
		fprintf( fpOut, "<tr><td><input type='submit' name='retry' value='retry'></td><td><input type='submit' name='return' value='return'></td></tr>\n");
		fprintf( fpOut, "</table>\n");
		goto html_end;
	}

	
	if( userInfo.usrOperation == 1 )/*login*/
	{
		switch(retLogin)
		{
			case PORTAL_AUTH_SUCCESS: 	urlNew = replaceStrPart(urlPost, "/auth_suc.html"); locate(fpOut, urlNew);break;
			case PORTAL_AUTH_REJECT: 	urlNew = replaceStrPart(urlPost, "/auth_fail.html"); locate(fpOut, urlNew);break;
			case PORTAL_AUTH_CONNECTED: urlNew = replaceStrPart(urlPost, "/auth_suc.html"); locate(fpOut, urlNew);break;
			case PORTAL_AUTH_ONAUTH: 	urlNew = replaceStrPart(urlPost, "/auth_fail.html"); locate(fpOut, urlNew);break;
			case PORTAL_AUTH_FAILED: 	urlNew = replaceStrPart(urlPost, "/auth_fail.html"); locate(fpOut, urlNew);break;
			case -1:					urlNew = replaceStrPart(urlPost, "/login.html"); locate(fpOut, urlNew);break;
		}
	}
	else if( userInfo.usrOperation == 2 )/*logout*/
	{
		switch(retLogout)
		{
			case EC_ACK_LOGOUT_SUCCESS: urlNew = replaceStrPart(urlPost, "/login.html"); locate(fpOut, urlNew);break;
			case -1:
			case EC_ACK_LOGOUT_REJECT: 	urlNew = replaceStrPart(urlPost, "/auth_suc.html"); locate(fpOut, urlNew);break;
			case EC_ACK_LOGOUT_FAILED: 	urlNew = replaceStrPart(urlPost, "/auth_suc.html"); locate(fpOut, urlNew);break;
		}
	}
html_end:

	fprintf( fpOut, "</body>\n" );
	fprintf( fpOut, "</html>\n" );

		
}

