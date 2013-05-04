/******************************************************************************
Copyright (C) Autelan Technology

This software file is owned and distributed by Autelan Technology 
*******************************************************************************

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
*******************************************************************************
* dcli_wsm.h
*
*
* DESCRIPTION:
*  SEM module Dbus implement.
*
* DATE:
*  2011-04-07
*
* CREATOR:
*  zhangdx@autelan.com
*
*
* CHANGE LOG:
*  2011-04-07 <zhangdx> Create file.
*
******************************************************************************/


#ifndef _DCLI_SEM_H
#define _DCLI_SEM_H

#define PATH_MAX_LEN 128
#define PATH_LEN 64

#define SPACIAL_CONFIG_COMMAND_MAX 64

/*#define DEBUG_STR "Config system debugging\n"*/
#define NODEBUG_STR	"Cancel system debugging\n"
#define MODULE_DEBUG_STR(module)	"Config "#module" debugging\n"
#define MODULE_DEBUG_LEVEL_STR(module,level) 	"Open "#module" debug level "#level"\n"


/* add by caojia */
enum
{
	XXX_YYY_AX7605I = 6,
	XXX_YYY_AX8610 = 0x10
};

void dcli_sem_init(void);

#define SEM_IS_DISTRIBUTED_PATH   "/dbm/product/is_distributed"
#define SEM_LOCAL_SLOT_ID_PATH    "/dbm/local_board/slot_id"
#define SEM_SLOT_COUNT_PATH       "/dbm/product/slotcount"
#define SEM_PRODUCT_TYPE_PATH     "/dbm/product/product_type"
#define SEM_MASTER_SLOT_COUNT_PATH "/dbm/product/master_slot_count"
#define SEM_ACTIVE_MASTER_SLOT_ID_PATH "/dbm/product/active_master_slot_id"

#endif



