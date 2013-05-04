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
* dcli_user.h
*
* MODIFY:
*
* CREATOR:
*		shancx@autelan.com
*
* DESCRIPTION:
*		CLI definition for user module.
*
* DATE:
*		04/22/2008	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.13 $	
*******************************************************************************/



#ifndef _DCLI_USER_H_
#include "zebra.h"


#define _DCLI_USER_H_

#define VIEWGROUP "vtyview"
#define ADMINGROUP "vtyadmin"
struct dcli_user
{
  char *name;
  char role;
  char *passwd;
};
#define CONSOLEPWDFILE "/etc/ttyS0pwd"
#define CONSOLEUSRFILE "/etc/ttyS0usr"




#define CONPWDSETTINGFILE "/etc/passwdsetting"
#define CONLOGINSETTINGFILE "/etc/login.defs"
#define CONLOGINSETTINGFILEBK "/etc/login.defs.bak"
#define CONPWDERRSETTINGFILE "/etc/pam.d/common-auth"
#define CONPWDERRSETTINGFILEBK "/etc/pam.d/common-auth.bak"
#if 1
#define CONPWDUNREPLYFILE "/var/run/passwdunreply"
#define CONPWDUNREPLYFILEBK "/var/run/passwdunreply.bak"
#else
#define CONPWDUNREPLYFILE "/etc/pam.d/common-password"
#define CONPWDUNREPLYFILEBK "/etc/pam.d/common-passwordbk"

#endif

#define CONPWDSYSSETTING "/etc/pam.d/common-password"
#define CONPWDSYSSETTINGBK "/etc/pam.d/common-password.bak"

#if 1
#define CONLOGINSETTING "/etc/pam.d/login"
#define CONLOCALLOGINSETTIN "/etc/pam.d/local-login"
#define CONREMOTELOGINSETTIN "/etc/pam.d/remote-login"
#define CONSSHSETTING "/etc/pam.d/sshd"
#define	CONPAMRADIUSAUTH "/etc/pam_radius_auth.conf"
#define	CONPAMRADIUSAUTHBK "/etc/pam_radius_auth.conf.bak"
#endif 

/*gujd : 2013-03-06, pm  2:46. Add code for user authentication file sync to  other boards.*/
#define USER_NAME_AUTHEN_FILE	"/etc/passwd"
#define USER_PASSWD_AUTHEN_FILE 	"/etc/shadow"
#define USER_AUTHEN_FILE_SYNC_FLAG	"/var/run/user_auth_sync"
#define HOME_DIR					"/home"

extern 	int get_user_role(char* );
#endif
