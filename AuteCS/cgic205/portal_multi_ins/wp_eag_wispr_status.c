#include <stdio.h>
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include "ws_eag_login.h"
#include "ws_user_manage.h"
#include "ws_init_dbus.h"

#include "eag/eag_conf.h"
#include "eag/eag_interface.h"
#include "nm_list.h"
#include "eag/eag_errcode.h"
#include "ws_eag_wispr.h"

//#define MAX_EAG_WISPR_VERSION_LEN 3

int cgiMain()
{
	int ret=0, ResponseCode=0,Status=0;
	unsigned long userip=0;
	struct eag_userdb userdb = {0};
	struct eag_user *user = NULL;
	struct in_addr client_addr;
	FILE * fpOut = cgiOut;

	ret = inet_aton(cgiRemoteAddr,&client_addr);
	if ( !ret ){
		fprintf(stderr,"inet_aton error!\n");
	}
	userip = ntohl(client_addr.s_addr);
	ccgi_dbus_init();
	//eag_userdb_init(&userdb);
	//ret = eag_show_user_by_userip(ccgi_dbus_connection,
	//						hansitype, insid,
	//						&userdb,
	//						userip);
	if (EAG_RETURN_OK == ret) {

		fprintf(stderr, "user num : %d\n", userdb.num);
		if (userdb.num>0){
			Status = 1;
		}
	}
	else if (EAG_ERR_DBUS_FAILED == ret) {
		ResponseCode = 255;
		fprintf(stderr, "%% dbus error\n");
	}
	else {
		ResponseCode = 254;
		fprintf(stderr, "%% unknown error: %d\n", ret);
	}
	//eag_userdb_destroy(&userdb);

//	<MessageType>160</MessageType>Required
//	<ResponseCode>{Response Code}</ResponseCode>Required
//	<Status>	{Status code}</Status>	Required

//response code
//0 No error
//254 Protocol error
//255 Access gateway internal error

	fprintf( fpOut, "<MessageType>160</MessageType>"\
			"<ResponseCode>%d</ResponseCode><Status>%d</Status>",ResponseCode,Status);
	
	return 0;
}





