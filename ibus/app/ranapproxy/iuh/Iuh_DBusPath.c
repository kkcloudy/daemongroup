#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "../../../../accapi/iuh/Iuh.h"
#include "dbus/iuh/IuhDBusDef.h"

void InitFemtoPath(unsigned int vrrid,char *buf){
	int len = strnlen(buf,PATH_LEN);
	#ifndef _DISTRIBUTION_
		sprintf(buf+len,"%d",vrrid);
	#else
		sprintf(buf+len,"%d_%d",local,vrrid);
	#endif
	if(buf == IUH_DBUS_OBJPATH){
		len = strnlen(buf,PATH_LEN);
		sprintf(buf+len,"/%s","iuh");
	}
	else if(buf == IUH_DBUS_INTERFACE){
		len = strnlen(buf,PATH_LEN);
		sprintf(buf+len,".%s","iuh");
	}
}

void Iuh_DbusPath_Init(){
	InitFemtoPath(vrrid,IUH_DBUS_BUSNAME);
	InitFemtoPath(vrrid,IUH_DBUS_OBJPATH);
	InitFemtoPath(vrrid,IUH_DBUS_INTERFACE);
}

