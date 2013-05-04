/***********************************************************************

  OCTEON TOOLKITS                                                         
  Copyright (c) 2007 Cavium Networks. All rights reserved.

  This file, which is part of the OCTEON TOOLKIT from Cavium Networks,
  contains proprietary and confidential information of Cavium Networks
  and in some cases its suppliers.

  Any licensed reproduction, distribution, modification, or other use of
  this file or confidential information embodied in this file is subject
  to your license agreement with Cavium Networks. The applicable license
  terms can be found by contacting Cavium Networks or the appropriate
  representative within your company.

  All other use and disclosure is prohibited.

  Contact Cavium Networks at info@caviumnetworks.com for more information.

 ************************************************************************/ 

#if defined(__KERNEL__) && defined(linux)
#include <linux/module.h>
#include <linux/kernel.h>
#include "cvmx.h"
#include "cvmx-wqe.h"

#define printf printk

#else
#include <stdio.h>
#include <string.h>
#include "cvmx.h"
#include "cvmx-wqe.h"
#include "cvmx-malloc.h"
#include "cvmx-spinlock.h"
#include "cvmx-scratch.h"

#endif

#include "cvmx-pip.h"
#include "cvmx-config.h"
#include "cvmx-fpa.h"
#include "fastfwd-common-defs.h"
#include "fastfwd-common-misc.h"
#include "fastfwd-common-fpa.h"

#include "autelan_product_info.h"

CVMX_SHARED product_info_t product_info;
CVMX_SHARED port_info_t pip_port_info[CVMX_PIP_NUM_INPUT_PORTS]; /*IPD/PKO port to index*/


#ifdef OCTEON_DEBUG_LEVEL
/* debug level */
CVMX_SHARED  int fastfwd_common_debug_level = FASTFWD_COMMON_DBG_LVL_WARNING;
CVMX_SHARED  uint8_t module_print[FASTFWD_COMMON_MAX_TYPE];
CVMX_SHARED  uint64_t core_mask;

void fastfwd_common_set_debug_level(uint64_t dbg_lvl)
{
    if ((dbg_lvl >= FASTFWD_COMMON_DBG_LVL_MUST_PRINT) &&
	(dbg_lvl < FASTFWD_COMMON_MAX_DBG_LVL))
    {
        //printf("\nDebug level changed from %ld to %ld\n", *fastfwd_common_debug_level, dbg_lvl);
        fastfwd_common_debug_level = dbg_lvl;
    }
    else
    {
        printf("\nInvalied debug level (%ld) passed\n", dbg_lvl);
    }
}

#endif

#if !defined(__KERNEL__)
/* shared arena for components */
//CVMX_SHARED cvmx_arena_list_t  cvm_common_arenas;

/* fpa information */
//CVMX_SHARED cvm_common_fpa_info_t cvm_common_fpa_info[CVMX_FPA_NUM_POOLS];

/* threadsafe functionality of cvmx_malloc */
//CVMX_SHARED cvmx_spinlock_t cvm_common_malloc_lock;

/* per core shared information */
/*CVMX_SHARED cvm_common_history_t *cvm_common_history = NULL;*/
#endif

