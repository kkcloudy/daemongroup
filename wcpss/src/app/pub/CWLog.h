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
* CWLog.h
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


#ifndef __CAPWAP_CWLog_HEADER__
#define __CAPWAP_CWLog_HEADER__

extern char gLogFileName[];



#define WID_SYSLOG_EMERG	0
#define WID_SYSLOG_ALERT	1
#define WID_SYSLOG_CRIT		2
#define WID_SYSLOG_ERR		3
#define WID_SYSLOG_WARNING	4
#define WID_SYSLOG_NOTICE	5
#define WID_SYSLOG_INFO		6
#define WID_SYSLOG_DEBUG	7
#define WID_SYSLOG_DEFAULT	0


#define WID_SYSLOG_DEBUG_NONE		0
#define WID_SYSLOG_DEBUG_INFO		1
#define WID_SYSLOG_DEBUG_DEBUG		8
#define WID_SYSLOG_DEBUG_ALL		15
#define WID_SYSLOG_DEBUG_DEFAULT	15
//if the syslog system forbidden showing the debug_info,we should change the default value to 0
extern int gWIDLogdebugLevel;

extern int gWIDLOGLEVEL;

__inline__ void CWVLog(const char *format, va_list args);
__inline__ void CWLog(const char *format, ...);
__inline__ void CWDebugLog(const char *format, ...);
void CWLogInitFile(char *fileName);
__inline__ void WID_Log(int level,const char *format, ...);
__inline__ void WIDVLog(int level,const char *format,va_list args);
void wid_syslog_emerg(char *format,...);
void wid_syslog_alert(char *format,...);
void wid_syslog_crit(char *format,...);
void wid_syslog_err(char *format,...);
void wid_syslog_warning(char *format,...);
void wid_syslog_notice(char *format,...);
void wid_syslog_info(char *format,...);
void wid_syslog_debug(char *format,...);
void wid_syslog_debug_info(char *format,...);
void wid_syslog_debug_debug(int type,char *format,...);
void wid_pid_write(unsigned int vrrid);
void wid_syslog_notice_local7(char *format,...);
void wid_pid_write_v2(char *name,int id,unsigned int vrrid);
void wid_syslog_hn(int level,char *iden,char *fmt,...);
void wid_syslog_auteview(int level,int type,void *parameter,int Rcode);



#endif
