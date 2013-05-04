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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <dbus/dbus.h>

#include "drp_log.h"
#include "drp_mem.h"

void *drp_malloc_trace (unsigned int malloc_size,
		const char * malloc_file_name,
		const char * malloc_func_name,
		int 	malloc_line_num )
{
	void *ptr=NULL;
	if (NULL != (ptr=malloc(malloc_size))){ 
		drp_log_debug("%s[%d]@%s: malloc: address is %p size is %ul\n",malloc_file_name,malloc_line_num,malloc_func_name,ptr,malloc_size);
		return ptr;
	}else
	{
		drp_log_debug("%s[%d]@%s: malloc: malloc error!\n",malloc_file_name,malloc_line_num,malloc_func_name);
		return NULL;
	}
}

