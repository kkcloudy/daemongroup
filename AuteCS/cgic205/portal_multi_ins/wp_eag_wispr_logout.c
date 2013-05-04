
#include <stdio.h>
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include "ws_eag_login.h"
#include "ws_user_manage.h"
#include "ws_eag_wispr.h"

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


int cgiMain()
{
	int retLogout=0,ResponseCode= 0;
	unsigned short reqID = 0;
	FILE * fpOut = cgiOut;
	char  WISPrVersion[MAX_EAG_WISPR_VERSION_LEN] = {0};
	STUserInfo userInfo;
	STAuthProcess stAuth;
	char userip_str[32]="";
	int ret;

	fprintf (stderr, "cgiContentType = %s    xxx\n", cgiContentType );
	
	memset(&stAuth, 0, sizeof(STAuthProcess));

	cgi_auth_init(&stAuth, 2000);
	STUserManagePkg * pstReq = NULL;
	STUserManagePkg * pstRsp = NULL;
	memset(&userInfo, 0 ,sizeof(STUserInfo));
	
	memset (userip_str, 0, sizeof(userip_str) );
	ret = cgiFormStringNoNewlines("UserIP", userip_str, sizeof(userip_str));
	if( cgiFormNotFound == ret ){
		fprintf(stderr,"UserIP not find! use cgiRemoteAddr=%s\n",cgiRemoteAddr);
		strncpy (userip_str, cgiRemoteAddr, sizeof(userip_str)-1);		
	}
	
	retLogout = CgiInformAc(userip_str, cgiServerName, REQ_LOGOUT, &stAuth, stAuth.protocal);
	destroyPortalPkg(stAuth.pSendPkg);
	destroyPortalPkg(stAuth.pRevPkg);
	closePkgSock(&stAuth);
	
//		WISPr response
//		<MessageType>130</MessageType>Required
//		<ResponseCode>{Response Code}</ResponseCode>Required

//logoput response
//150 Logoff succeeded
//152 Invalid session
//254 Protocol error
//255 Access gateway internal error
	cgiHeaderContentType("text/html");
	fprintf( fpOut, "<html xmlns=\"http://www.w3.org/1999/xhtml\"> \n" );
	fprintf( fpOut, "<head> \n" );
#if 0	
	fprintf( fpOut, "<meta http-equiv=\"Content-Type\" ontent=\"text/html;\" charset=\"gb2312\"> \n" );


  	fprintf( fpOut, "<META   HTTP-EQUIV=\"pragma\"   CONTENT=\"no-cache\">\n");
  	fprintf( fpOut, "<META   HTTP-EQUIV=\"Cache-Control\"   CONTENT=\"no-cache\"> \n" );
  	fprintf( fpOut, "<META   HTTP-EQUIV=\"expires\"   CONTENT=\"0\">\n");
#endif

	fprintf( fpOut, "<title>login_proc</title> \n");
	fprintf( fpOut, "</head> \n" );
	fprintf( fpOut, "<body> \n");          

	switch(retLogout){
		case 0:
			ResponseCode=150;	//Logoff succeeded (Access ACCEPT)
			break;
		case 1:
		case 2:
		case 3:
		default:
			ResponseCode=255;	//Access gateway internal error
			fprintf(stderr,"login err retcode(%d) not found!",retLogout);
			break;
	}
	fprintf( fpOut, "<!-- \n" );
	fprintf( fpOut, "<?xml version=\"1.0\" encoding=\"UTF-8\"?> \n" );
	fprintf( fpOut, "<WISPAccessGatewayParam  \n"
					"xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" \n"
					"xsi:noNamespaceSchemaLocation=\"http://www.acmewisp.com/WISPAccessGatewayParam.xsd\"> \n" );
	fprintf( fpOut, "	<AuthenticationReply> \n");
	
	fprintf( fpOut, "	<MessageType>130</MessageType><ResponseCode>%d</ResponseCode>\n",ResponseCode);
	fprintf( fpOut, "</AuthenticationReply>\n</WISPAccessGatewayParam> \n" );
	
	fprintf( fpOut, "--> \n" );
		
	fprintf( fpOut, "</body>\n" );
	fprintf( fpOut, "</html>\n" );
	
	return 0;
}



