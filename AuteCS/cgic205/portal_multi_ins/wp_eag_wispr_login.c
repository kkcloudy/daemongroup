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
#include "ws_eag_wispr.h"

//#define MAX_EAG_WISPR_VERSION_LEN 3

#if 0
int encodeURIComponent(char *src,char *dst, unsigned int dstsize)
{
	char transet[]="+:/;?&";
	int i;
#define isinset(a) \
	for(i=0;i<sizeof(transet)/sizeof(char);i++){\
		

}
#endif

#define urlprefix (strcmp(cgiServerPort,"8081")==0)?"http":"https"

int cgiMain()
{
	int retLogin=0,ResponseCode= 0;
	unsigned short reqID = 0;
	FILE * fpOut = cgiOut;
	char  WISPrVersion[MAX_EAG_WISPR_VERSION_LEN+1] = {0};
	STUserInfo userInfo;
	STAuthProcess stAuth;
	UINT32 userip = 0;
	char userip_str[32]="";
	int ret;
	UINT32 timeout = DEFAULT_PORTAL_REQ_TIMEOUT;
	char timeout_str[32];
	char *ReplyMessage = NULL;
	char *RedirectionURL = NULL;
	UINT32 MaxSessionTime = 0;
	
	memset(&stAuth, 0, sizeof(STAuthProcess));

	cgi_auth_init(&stAuth, 2000);
	memset(&userInfo, 0 ,sizeof(STUserInfo));
	
	ret = cgiFormStringNoNewlines("WISPrVersion", WISPrVersion, MAX_EAG_WISPR_VERSION_LEN);
	if (cgiFormNotFound == ret) {
		strcpy(WISPrVersion,"1");
	}
	cgiFormStringNoNewlines("UserName", userInfo.usrName, MAX_EAG_LOGIN_NAME_LEN);
	cgiFormStringNoNewlines("Password", userInfo.usrPass, MAX_EAG_LOGIN_PASS_LEN);

	memset (userip_str, 0, sizeof(userip_str) );
	ret = cgiFormStringNoNewlines("UserIP", userip_str, sizeof(userip_str));
	if( cgiFormNotFound == ret ){
		fprintf(stderr,"UserIP not find! use cgiRemoteAddr=%s\n",cgiRemoteAddr);
		strncpy (userip_str, cgiRemoteAddr, sizeof(userip_str)-1);		
	}
	userip = htonl(inet_addr(userip_str));

	if (0 == userip) {
		fprintf(stderr, "user ip is 0!!\n" );
	}
	
	ret = cgiFormStringNoNewlines("TimeOut", timeout_str, sizeof(timeout_str)-1);
	if (cgiFormSuccess == ret) {
		timeout = atoi(timeout_str);
		if (timeout<MIN_PORTAL_REQ_TIMEOUT || timeout>MAX_PORTAL_REQ_TIMEOUT) {
			timeout = DEFAULT_PORTAL_REQ_TIMEOUT;
		}
	}
	fprintf(stderr,"login WISPrVersion:%s,UserName:%s,Password=%s.\n",WISPrVersion,userInfo.usrName,userInfo.usrPass);
		
	/*after challege exchange*/
	stAuth.pSendPkg = createPortalPkg(REQ_AUTH);
	fprintf(stderr,"login createPortalPkg suc\n" );
	
	/*malloc STPortalPkg ready to rev data*/
	stAuth.pRevPkg = (STPortalPkg * )malloc(sizeof(STPortalPkg));
	memset(stAuth.pRevPkg, 0, sizeof(STPortalPkg));

	stAuth.protocal = AUTH_PAP;
	setAuthType(stAuth.pSendPkg, stAuth.protocal);
	setRequireID(stAuth.pSendPkg, reqID );
	setPkgUserIP( stAuth.pSendPkg, userip );
	
	addAttr( &stAuth.pSendPkg, ATTR_USERNAME, userInfo.usrName, strlen(userInfo.usrName) );
	
	fprintf(stderr,"userInfo.usrPass=%s",userInfo.usrPass);
	addAttr( &stAuth.pSendPkg, ATTR_PASSWORD, userInfo.usrPass, strlen(userInfo.usrPass) );
	
	if(sendPortalPkg(stAuth.fd, (timeout+1)/2, 2000, cgiServerName, stAuth.pSendPkg) < 0 )
	{
		fprintf(stderr,"login sendPortalPkg failed\n" );
		retLogin = -1;
	}
	else
	{
		fprintf(stderr,"login sendPortalPkg suc\n" );
	}
	
	if(getPortalPkg(stAuth.fd, timeout/2, &(stAuth.pRevPkg))<0)
	{
		fprintf(stderr,"login getPortalPkg failed\n" );
		retLogin = -1;
	}
	else
	{
		fprintf(stderr,"login getPortalPkg suc\n" );
		retLogin = getErrCode(stAuth.pRevPkg);
	}
	
	fprintf(stderr,"login getErrCode(stAuth.pRevPkg)=%d\n", retLogin );

//		WISPr response
//		<MessageType>120</MessageType>	//Required
//		<ResponseCode>{Response Code}</ResponseCode>	//Required
//		<ReplyMessage>{Reply Message Text}</ReplyMessage>	//Conditional Required
//		<LogoffURL>http[s]:/*{site specific logoffURL}*/</LogoffURL>	//Conditional Required
//		<RedirectionURL>http[s]:/*{redirection URL}*/</RedirectionURL>	//Optional
//		<StatusURL>http[s]:/*{status URL}*/</StatusURL>	//Conditional Required
//		<MaxSessionTime>{Maximum Session Time}</MaxSessionTime>
	switch(retLogin){
		case 0:
			ResponseCode=50;	//Login succeeded (Access ACCEPT)
			break;
		case 1:
			ResponseCode=100;	//Login failed (Access REJECT)
			break;
#if 0				
		case:
			ResponseCode=102;	//Authentication server error/timeout
			break;
		case:
			ResponseCode=105;	//Network Administrator Error: No authentication server enabled
			break;
		case:
			ResponseCode=252;	//Invalid state for WISPr request
			break;
		case:
			ResponseCode=253;	//MTU of AAA message is too big
			break;
		case:
			ResponseCode=254;	//Protocol error
			break;
		case:
			ResponseCode=255;	//Access gateway internal error
			break;
#endif				
		default:
			ResponseCode=100;
			fprintf(stderr,"login err retcode(%d) not found!",retLogin);
			break;
	}
/*html pre*/
	cgiHeaderContentType("text/html");
	fprintf( fpOut, "<html xmlns=\"http://www.w3.org/1999/xhtml\"> \n" );
	fprintf( fpOut, "<head> \n" );
#if 0	
	fprintf( fpOut, "<meta http-equiv=\"Content-Type\" content=\"text/html;\" charset=\"gb2312\"> \n" );


  	fprintf( fpOut, "<META   HTTP-EQUIV=\"pragma\"   CONTENT=\"no-cache\"> \n");
  	fprintf( fpOut, "<META   HTTP-EQUIV=\"Cache-Control\"   CONTENT=\"no-cache\"> \n" );
  	fprintf( fpOut, "<META   HTTP-EQUIV=\"expires\"   CONTENT=\"0\">\n");
#endif

	fprintf( fpOut, "<title>login_proc</title> \n");
	fprintf( fpOut, "</head> \n" );
	fprintf( fpOut, "<body> \n");
/*xml */	
	fprintf( fpOut, "<!-- \n" );
	fprintf( fpOut, "<?xml version=\"1.0\" encoding=\"UTF-8\"?> \n" );
	fprintf( fpOut, "<WISPAccessGatewayParam  \n"
					"xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" \n"
					"xsi:noNamespaceSchemaLocation=\"http://www.acmewisp.com/WISPAccessGatewayParam.xsd\"> \n" );
	fprintf( fpOut, "  <AuthenticationReply> \n");
	fprintf( fpOut, "    <MessageType>120</MessageType> \n    <ResponseCode>%d</ResponseCode> \n",ResponseCode);
	if (ReplyMessage && strlen(ReplyMessage)>0){
		fprintf( fpOut, "    <ReplyMessage>%s</ReplyMessage> \n",ReplyMessage);
	}else if (0 != retLogin) {
		fprintf( fpOut, "    <ReplyMessage>Login Failed.Please check your username and password!</ReplyMessage> \n" );
	}
	fprintf( fpOut, "    <LogoffURL>%s://%s:%s/wispr/logout?UserIP=%s</LogoffURL> \n",
						urlprefix,cgiServerName, cgiServerPort, userip_str );
	if (RedirectionURL && strlen(RedirectionURL)>0){
		fprintf( fpOut,	"    <RedirectionURL>%s</RedirectionURL> \n",RedirectionURL);
	}
#if 0	
	fprintf( fpOut, "    <StatusURL>%s://%s:%s/wispr/status?UserIP=%s</StatusURL> \n",
						urlprefix,cgiServerName, cgiServerPort, userip_str );
#endif
	if (MaxSessionTime) {
		fprintf( fpOut,	"    <MaxSessionTime>%u</MaxSessionTime> \n",MaxSessionTime);
	}
	fprintf( fpOut, "  </AuthenticationReply>\n</WISPAccessGatewayParam> \n" );
	
	destroyPortalPkg(stAuth.pSendPkg);
	destroyPortalPkg(stAuth.pRevPkg);
	fprintf( fpOut, "--> \n" );	
/*xml  end*/
#if 0
	if ( 0 == retLogin ){
		fprintf( fpOut, "login successful!\n" );
	}
	if (ReplyMessage && strlen(ReplyMessage)>0){
		fprintf( fpOut, "ReplyMessage=%s\n",ReplyMessage );
	}

#endif	           
/**/
	  
	fprintf( fpOut, "</body>\n" );
	fprintf( fpOut, "</html>\n" );

	/* adding aff_ack_auth package	*/
	// err_code=0 means ack-auth normal succeed
	if (0 == retLogin ) { 
		/*after auth exchange*/
		stAuth.pSendPkg = createPortalPkg(AFF_ACK_AUTH);
		
		/*malloc STPortalPkg ready to rev data*/
		stAuth.pRevPkg = (STPortalPkg * )malloc(sizeof(STPortalPkg));
		memset(stAuth.pRevPkg, 0, sizeof(STPortalPkg));
		
		setAuthType(stAuth.pSendPkg, stAuth.protocal);
		setRequireID(stAuth.pSendPkg, reqID );
		setPkgUserIP( stAuth.pSendPkg, htonl(inet_addr(cgiRemoteAddr)) );
	
		if(sendPortalPkg(stAuth.fd, 3, 2000, cgiServerName, stAuth.pSendPkg) < 0 )
		{
			fprintf(stderr,"auth sendPortalPkg failed\n" );
			retLogin = -1;
		}
		else
		{
			fprintf(stderr,"auth sendPortalPkg suc\n" );
		}
	
		destroyPortalPkg(stAuth.pSendPkg);
		destroyPortalPkg(stAuth.pRevPkg);
	}


	return 0;
}


