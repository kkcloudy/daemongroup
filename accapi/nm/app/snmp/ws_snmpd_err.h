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
* ws_snmpd_err.h
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

/*
* Copyright (c) 2008,Autelan - AuteCS
* All rights reserved.
*
*$Source: /rdoc/accapi/nm/app/snmp/ws_snmpd_err.h,v $
*$Author: shaojunwu $
*$Date: 2010/05/31 11:51:50 $
*$Revision: 1.1 $
*$State: Exp $
*
*$Log: ws_snmpd_err.h,v $
*Revision 1.1  2010/05/31 11:51:50  shaojunwu
*modify for some ws has  html output for can't contain in libnm
*
*Revision 1.2  2010/02/22 06:46:02  chensheng
*代码整理,在文件头部加上傲天的文件头
*
*Revision 1.1  2008/12/16 02:32:41  tangsiqi
*snmp module
*
*
*/

#ifndef _WS_SNMPD_ERR_H
#define _WS_SNMPD_ERR_H

#include "ws_sysinfo.h"
/*#include "ws_ec.h"
#include "ws_err.h"*/


#define WS_ERR_SNMPD_MEMORY			-1
#define WS_ERR_SNMPD_ILLEGAL_USER	-2
#define WS_ERR_SNMPD_PORINTER		-3



typedef struct {
	int err_num;
	char *err_msg_key;
}st_error_map;



void show_user_input_err( int err_num, struct list *p_list );
void show_command_err( int err_num, struct list *p_list );
void show_err_page( int err_num, struct list *p_list );//当错误导致页面不能正常输出的时候，就用这个。

#endif
