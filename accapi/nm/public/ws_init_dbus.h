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
* ws_init_dbus.h
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

#ifndef _WS_INIT_DBUS_H
#define _WS_INIT_DBUS_H

#include <stdio.h>
#include <string.h>
#include <dbus/dbus.h>


#define CCGI_SUCCESS  0
#define CCGI_FAIL	-1
#define INSTANCE_NUM	16
#define INSTANCE_STATE_WEB	SNMPD_INSTANCE_ALL

#define CWErr(arg)				((arg) || _CWErrorHandleLast(__FILE__, __LINE__))
#define		FREE_OBJECT(obj_name)				{if(obj_name){free((obj_name)); (obj_name) = NULL;}}

#define DISTRIBUTFAG 0

extern DBusConnection *ccgi_dbus_connection;
extern struct sample_rtmd_info intf_flow_info;
extern int HAD_ID;
extern int IS_HAD_SWITCH;
extern void *ccgi_dl_handle;

extern int ccgi_dbus_init(void);
extern void destroy_ccgi_dbus(void);
extern int ccgi_local_dbus_init(void);
extern int snmpd_dbus_init(void);
extern void ccgi_ReInitDbusPath(int index, char * path, char * newpath);
extern void ccgi_ReInitDbusPath_v2(int local,int index, char * path, char * newpath);
extern void ccgi_ReInitDbusConnection(DBusConnection **dcli_dbus_connection,int slot_id,int distributFag);
extern int get_is_distributed();
extern int get_dbm_effective_flag();
#endif

