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
* ws_dcli_license.h
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* function for web
*
*
***************************************************************************/
#ifndef _WS_LICENSE_H
#define _WS_LICENSE_H

#define LICENSE_FILE "/mnt/license.txt"   //license default path and filename
#define LICENSE_TEMP "/var/run/apache2/license.temp"  //temp file path

enum license_item {
	SYSTEM_ITEM,  //0
	CONTRL_ITEM,
	WLAN_ITEM,
	AUTH_ITEM,
	FIREWALL_ITEM,
	SYS_ITEM,
	HELP_ITEM
};                     

extern int get_license_state( int module );//if display the system contrl module

extern int get_license_state_new( int module ,char *Src);//if display the system contrl module

extern char * get_decrypt_content(); //first dencrpt mc txt

extern int encry_oid(); //encryt src mc

extern char *get_machine_code(); //get machine code

extern int get_max_size_file(char *fpath);

#endif
