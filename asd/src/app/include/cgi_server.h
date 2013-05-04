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
* cgi_server.h
*
*
* CREATOR:
* autelan.software.WirelessControl. team
*
* DESCRIPTION:
* asd module
*
*
*******************************************************************************/

#ifndef _CGI_SERVER_H_
#define _CGI_SERVER_H_
#include "common.h"
//#include "typedef.h"

#define SNMPD_PORT			9003
#define SNMPD_CMD_GET_WAI_STATS		0x0312
#define SNMPD_CMD_SET_WAI_STATS		0x0313

#define CGI_PORT			9002
#define CGI_CMD_RELOAD		0x0212
#define CGI_RELOAD_RESPONSE	0x0213
#define CGI_CMD_CHECKCERT	0x0214
#define CGI_CHECK_RESPONSE	0x0215
#define CGI_PRE_AUTHENTICATION	0x0216
#define CGI_CMD_ADD_DEV	0x0217
#define CGI_CMD_DEL_DEV	0x0218

#define SNMPD_UPDATE_PORT							9004
#if 0
#define SNMPD_UPDATE_CMD_SET_DL_FAILED			0x0212
#define SNMPD_UPDATE_CMD_SET_FILE_ERROR			0x0213
#define SNMPD_UPDATE_CMD_SET_START				0x0214
#define SNMPD_UPDATE_CMD_SET_UPDATE_FAILED		0x0215
#define SNMPD_UPDATE_CMD_SET_UPDATE_OK			0x0216
#define SNMPD_UPDATE_CMD_SET_REBOOT			0x0217
#endif
#define SNMPD_ATTACK_INVALID_CERT				0x0220
#define SNMPD_ATTACK_CHALLENGE_REPLAY			0x0221
#define SNMPD_ATTACK_MIC_JUGGLE					0x0222

/*****************************************************************************
	Description  : create a udp socket.
 *****************************************************************************/
int open_socket_for_cgi();
int Process_CGI_checkcert(u8 *mesg, int msglen);
int Process_CTRL_message(u8 *recv_buf, int readlen, int *cgicmd);
int Process_CGI_del_dev(u8 *mesg, int msglen);
int  del_ap_by_dev_and_ssid(const char *dev_in,const char *ssid_in);
int  add_ap_by_cgi(char *line);
int Process_SNMPD_get_wai_stats(u8 *recv_buf);
int  del_ap_by_dev(const char *dev_in);
int open_socket_for_snmpd();
int Process_SNMPD_message(u8 *recv_buf, int readlen);
int trap2snmp(u8 *mac,  unsigned short type);

#endif
