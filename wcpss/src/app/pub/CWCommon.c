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
* CWCommon.c
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
 
#include "CWCommon.h"
#include "wcpss/wid/WID.h"

#ifdef DMALLOC
#include "../dmalloc-5.5.0/dmalloc.h"
#endif

int gCWForceMTU = 0;
int gCWRetransmitTimer = CW_RETRANSMIT_INTERVAL_DEFAULT;	//Default value for RetransmitInterval
int gCWNeighborDeadInterval = CW_NEIGHBORDEAD_INTERVAL_DEFAULT; //Default value for NeighbourDeadInterval (no less than 2*EchoInterval and no greater than 240) 
int gCWMaxRetransmit = CW_MAX_RETRANSMIT_DEFAULT;		//Default value for MaxRetransmit 
int wid_memory_trace_switch = 0;
void wid_free(void *ptr, const char* func_name, unsigned int line) {
	free(ptr);
	if (wid_memory_trace_switch) 
		wid_syslog_err("wid_memory_trace:%s %d free addr %p success\n", func_name, line, ptr);
}

void *wid_malloc(unsigned int len, const char* func_name, unsigned int line) {
	void *temp;
	temp = malloc(len);
	if (temp) {
		if (wid_memory_trace_switch) 
			wid_syslog_err("wid_memory_trace:%s %d malloc %d Bytes addr %p success\n", func_name, line, len, temp);
		return temp;
	} else {
		if (wid_memory_trace_switch)
			wid_syslog_err("wid_memory_trace:%s %d malloc %d Bytes failed\n",func_name, line, len);
		return NULL;
	}
}

/*
__inline__ int CWGetFragmentID() {
	static int fragID = 0;
	int r;

	if(!CWThreadMutexLock(&gCreateIDMutex)){CWDebugLog("Error Locking a mutex");}
		r = fragID;
		if (fragID==CW_MAX_FRAGMENT_ID) fragID=0;
		else fragID++;
	CWThreadMutexUnlock(&gCreateIDMutex);
	
	return r;
}
*/

int CWTimevalSubtract(struct timeval *res, const struct timeval *x, const struct timeval *y){
	int nsec;
	struct timeval z=*y;
   
	// Perform the carry for the later subtraction by updating Y
	if (x->tv_usec < z.tv_usec) {
		nsec = (z.tv_usec - x->tv_usec) / 1000000 + 1;
		z.tv_usec -= 1000000 * nsec;
		z.tv_sec += nsec;
	}
	if (x->tv_usec - z.tv_usec > 1000000) {
		nsec = (x->tv_usec - z.tv_usec) / 1000000;
		z.tv_usec += 1000000 * nsec;
		z.tv_sec -= nsec;
	}

	// Compute the time remaining to wait. `tv_usec' is certainly positive
	if ( res != NULL){
		res->tv_sec = x->tv_sec - z.tv_sec;
		res->tv_usec = x->tv_usec - z.tv_usec;
	}

	// Return 1 if result is negative (x < y)
	return ((x->tv_sec < z.tv_sec) || ((x->tv_sec == z.tv_sec) && (x->tv_usec < z.tv_usec)));
}
