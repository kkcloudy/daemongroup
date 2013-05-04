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
* ACSTARoaming.c
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
#include "CWAC.h"
#include "wcpss/waw.h"
#include "wcpss/wid/WID.h"
#include "ACDbus_handler.h"
#include "ACDbus.h"

CW_THREAD_RETURN_TYPE CWSTARoamingOP(void * arg){
		wid_pid_write_v2("CWSTARoamingOP",0,vrrid);
		int nIndex = 0;
		unsigned char WLANID;
		char *command = NULL;
		command = (char*)malloc(128);
		//=============================================
		//printf("created roaming thread\n");
		while(1){
			CWThreadMutexLock(&(gSTARoamingMutex));
			CWWaitThreadCondition(&gSTARoamingWait, &gSTARoamingMutex);
			CWThreadMutexUnlock(&gSTARoamingMutex);
			CWThreadMutexLock(&(gSTARoamingMutex));			
			memset(command, 0, 128);
			if(STA_ROAM.STAOP == 0){
				sprintf(command,"iwpriv ath%d addmac %02X:%02X:%02X:%02X:%02X:%02X\n",STA_ROAM.WLANDomain,STA_ROAM.STAMAC[0],STA_ROAM.STAMAC[1],STA_ROAM.STAMAC[2],STA_ROAM.STAMAC[3],STA_ROAM.STAMAC[4],STA_ROAM.STAMAC[5]);
			}else{
				sprintf(command,"iwpriv ath%d delmac %02X:%02X:%02X:%02X:%02X:%02X\n",STA_ROAM.WLANDomain,STA_ROAM.STAMAC[0],STA_ROAM.STAMAC[1],STA_ROAM.STAMAC[2],STA_ROAM.STAMAC[3],STA_ROAM.STAMAC[4],STA_ROAM.STAMAC[5]);
			}
//			printf("CWSTARoamingOP :%s\n",command);
			WLANID = STA_ROAM.WLANDomain;
			for(nIndex=0;nIndex<WTP_NUM;nIndex++){
				if((AC_WTP[nIndex] != NULL)&&(AC_WLAN[WLANID] != NULL)&&(AC_WLAN[WLANID]->S_WTP_BSS_List[nIndex][0] > 0))
					wid_radio_set_extension_command(nIndex,command);
			}
			
			CWThreadMutexUnlock(&gSTARoamingMutex);		
		}

}

