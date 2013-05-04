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
* ACDbusPathInit.c
*
*
* CREATOR:
* autelan.software.wireless-control. team
*
* DESCRIPTION:
* wid module
*
*
*******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "CWAC.h"
#include "wcpss/waw.h"
#include "wcpss/wid/WID.h"
#include "CWCommon.h"
#include "ACDbus.h"
#include "dbus/wcpss/ACDbusDef1.h"
#include "dbus/asd/ASDDbusDef1.h"

char MSGQ_PATH[PATH_LEN] = "/var/run/wcpss/wmsgq";



void InitPath(unsigned int vrrid,char *buf){
	int len = strlen(buf);
#ifndef _DISTRIBUTION_
	sprintf(buf+len,"%d",vrrid);
#else
	sprintf(buf+len,"%d_%d",local,vrrid);
#endif
	if(buf == WID_DBUS_WLAN_OBJPATH){
			len = strlen(buf);
			sprintf(buf+len,"/%s","wlan");
	}
	else if(buf == WID_DBUS_WLAN_INTERFACE)
	{	
			len = strlen(buf);
			sprintf(buf+len,".%s","wlan");			
	}
	else if(buf == WID_DBUS_WTP_OBJPATH){
			len = strlen(buf);
			sprintf(buf+len,"/%s","wtp");
	}
	else if(buf == WID_DBUS_WTP_INTERFACE)
	{	
			len = strlen(buf);
			sprintf(buf+len,".%s","wtp");			
	}
	else if(buf == WID_DBUS_RADIO_OBJPATH){
			len = strlen(buf);
			sprintf(buf+len,"/%s","radio");
	}
	else if(buf == WID_DBUS_RADIO_INTERFACE)
	{	
			len = strlen(buf);
			sprintf(buf+len,".%s","radio");			
	}
	else if(buf == WID_DBUS_QOS_OBJPATH){
			len = strlen(buf);
			sprintf(buf+len,"/%s","qos");
	}
	else if(buf == WID_DBUS_QOS_INTERFACE)
	{	
			len = strlen(buf);
			sprintf(buf+len,".%s","qos");			
	}
	else if(buf == WID_DBUS_EBR_OBJPATH){
			len = strlen(buf);
			sprintf(buf+len,"/%s","ebr");
	}
	else if(buf == WID_DBUS_EBR_INTERFACE)
	{	
			len = strlen(buf);
			sprintf(buf+len,".%s","ebr");			
	}	
	else if(buf == WID_BAK_OBJPATH){
			len = strlen(buf);
			sprintf(buf+len,"/%s","bak");
	}
	else if(buf == WID_BAK_INTERFACE)
	{	
			len = strlen(buf);
			sprintf(buf+len,".%s","bak");			
	}
	else if(buf == WID_DBUS_ACIPLIST_OBJPATH){
			len = strlen(buf);
			sprintf(buf+len,"/%s","aciplist");
	}
	else if(buf == WID_DBUS_ACIPLIST_INTERFACE)
	{	
			len = strlen(buf);
			sprintf(buf+len,".%s","aciplist");			
	}
	else if(buf == WID_DBUS_AP_GROUP_OBJPATH){
			len = strlen(buf);
			sprintf(buf+len,"/%s","apgroup");
	}
	else if(buf == WID_DBUS_AP_GROUP_INTERFACE)
	{	
			len = strlen(buf);
			sprintf(buf+len,".%s","apgroup");			
	}
}

void CWWIDDbusPathInit()
{
	InitPath(vrrid,WID_DBUS_BUSNAME);
	InitPath(vrrid,WID_DBUS_OBJPATH);
	InitPath(vrrid,WID_DBUS_INTERFACE);
	InitPath(vrrid,WID_DBUS_WLAN_OBJPATH);
	InitPath(vrrid,WID_DBUS_WLAN_INTERFACE);
	InitPath(vrrid,WID_DBUS_WTP_OBJPATH);
	InitPath(vrrid,WID_DBUS_WTP_INTERFACE);
	InitPath(vrrid,WID_DBUS_RADIO_OBJPATH);
	InitPath(vrrid,WID_DBUS_RADIO_INTERFACE);
	InitPath(vrrid,WID_DBUS_QOS_OBJPATH);
	InitPath(vrrid,WID_DBUS_QOS_INTERFACE);
	InitPath(vrrid,WID_DBUS_EBR_OBJPATH);
	InitPath(vrrid,WID_DBUS_EBR_INTERFACE);
	InitPath(vrrid,WID_BAK_OBJPATH);
	InitPath(vrrid,WID_BAK_INTERFACE);
	InitPath(vrrid,WID_DBUS_ACIPLIST_OBJPATH);
	InitPath(vrrid,WID_DBUS_ACIPLIST_INTERFACE);	
	InitPath(vrrid,WID_DBUS_AP_GROUP_OBJPATH);
	InitPath(vrrid,WID_DBUS_AP_GROUP_INTERFACE);
}

