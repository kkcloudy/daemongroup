#include <stdio.h>
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>  
#include <sys/wait.h>
#include "wcpss/asd/asd.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/dcli_wid_wtp.h"
#include "dbus/wcpss/dcli_wid_wlan.h"
#include "ws_dcli_wlans.h"
#include "ws_usrinfo.h"
#include "ws_err.h"
#include "ws_ec.h"
#include "ws_dcli_vrrp.h"
#include "ws_init_dbus.h"
#include "ws_dbus_list_interface.h"
#include "ws_version_param.h"
#include "bsd/bsdpub.h"
#include "dbus/bsd/BsdDbusPath.h"


#define UPLOAD_FILE_PATH	"/var/run/upload.txt"

int cgiMain(void)
{
	FILE *fp = NULL;
	unsigned int flag=0;
	char res[5]={0};
	
	if(access(UPLOAD_FILE_PATH,0) != 0)
	{
		fprintf(stderr,"(acc  ess(UPLOAD_FILE_PATH,0) != 0)\n"); 
		printf("Content type: text/html\n\n");
		printf("%d", 0);
		return; 
	}
	
	fp = fopen(UPLOAD_FILE_PATH ,"r");
	if(NULL == fp)
	{
		fprintf(stderr,"f  open(UPLOAD_FILE_PATH )=null\n"); 
		printf("Content type: text/html\n\n");
		printf("%d", 0);
		return; 
	}

	fscanf(fp, "%ud", &flag);
	fclose( fp );
	
	fprintf(stderr,"flag ==%d\n",flag); 
	printf("Content type: text/html\n\n");
	printf("%d", flag);
	return; 
}

