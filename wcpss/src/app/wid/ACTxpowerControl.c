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
* ACTxpowerControl.c
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
/*
	added by weiay for transmit power control
	2008/12/16
*/

#include "CWAC.h"
#include "wcpss/wid/WID.h"
#include "ACDbus_handler.h"
#include "ACDbus.h"
#ifdef DMALLOC
#include "../dmalloc-5.5.0/dmalloc.h"
#endif

CW_THREAD_RETURN_TYPE CWTransmitPowerControl(void * arg)
{
	wid_pid_write_v2("CWTransmitPowerControl",0,vrrid);
	int retflag = 0; 
	int i = 0;
	while(1)
	{		
		retflag = 0;
		CWThreadMutexLock(&(gACTxpowerMutex));
		CWWaitThreadCondition(&gACTxpowerWait, &gACTxpowerMutex);
		CWThreadMutexUnlock(&gACTxpowerMutex);
		CWThreadMutexLock(&(gACTxpowerMutex));

		//CWThreadMutexLock(&(gACChannelMutex));

		if(txpower_state)
		{
			i = tx_wtpid;
			
			if((AC_WTP[i] != NULL)&&(AC_WTP[i]->NeighborAPInfos != NULL))
			{
				CWThreadMutexLock(&(gWTPs[i].RRMThreadMutex));
				calc_transmit_power_control(i, AC_WTP[i]->NeighborAPInfos,&retflag);
				CWThreadMutexUnlock(&(gWTPs[i].RRMThreadMutex));
			}

		}
		
		//CWThreadMutexUnlock(&gACChannelMutex);
		
		CWThreadMutexUnlock(&gACTxpowerMutex);

		
		if(retflag == 1)
		{
			if(gtrapflag>=4)
				wid_dbus_trap_wtp_ac_discovery_cover_hole_clear(i);
			int radioid = AC_WTP[i]->WTP_Radio[0]->Radio_G_ID;
			int itxpower = AC_WTP[i]->WTP_Radio[0]->Radio_TXP;
			WID_RADIO_SET_TXP(radioid,(itxpower-2),CW_FALSE);
		}
		else if(retflag == 2)
		{
			if(gtrapflag>=4)
				wid_dbus_trap_wtp_ac_discovery_cover_hole(i);
			
			int radioid = AC_WTP[i]->WTP_Radio[0]->Radio_G_ID;
			int itxpower = AC_WTP[i]->WTP_Radio[0]->Radio_TXP;
			WID_RADIO_SET_TXP(radioid,(itxpower+2),CW_FALSE);
		}
	}

}

