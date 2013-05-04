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
 *
 *
 * CREATOR:
 * autelan.software.xxx. team
 *
 * DESCRIPTION:
 * xxx module main routine
 *
 *
 *******************************************************************************/

#ifndef _MEM_CHECK_DRP_C_
#define _MEM_CHECK_DRP_C_

#ifdef _MEM_TRACE_	//for test memory

#include "log.h"

#define drp_malloc(size) drp_malloc_trace(size,__FILE__,__FUNCTION__,__LINE__)
#define drp_free(point) do {\
	if (point) {	\
		drp_trace_log("free: address is %p\n",point);	\
		free((void *)point);	\
		point=NULL;		\
	}else{	\
		drp_trace_log("free: error ptr is NULL\n");	\
	}	\
} while(0)
#else
#define drp_malloc(size) malloc(size)
#define drp_free(point) free(point);
#endif


#endif
