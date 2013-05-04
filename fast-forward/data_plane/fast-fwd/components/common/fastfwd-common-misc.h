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

#ifndef __FASTFWD_COMMON_MISC_H__
#define __FASTFWD_COMMON_MISC_H__


#if !(defined(__KERNEL__) && defined(linux))
#include <string.h>
#endif

#if !(defined(__KERNEL__) && defined(linux))

#include "cvmx-scratch.h"

static inline void cvm_common_set_cycle(uint64_t cycle)
{
    uint32_t val_low  = cycle & 0xffffffff;
    uint32_t val_high = cycle  >> 32;
    
    uint32_t tmp; /* temp register */

    asm volatile (
        "  .set push\n"
        "  .set mips64                       \n"
        "  .set noreorder                    \n"
        /* Standard twin 32 bit -> 64 bit construction */
        "  dsll  %[valh], 32                 \n"
        "  dla   %[tmp], 0xffffffff          \n"
        "  and   %[vall], %[tmp], %[vall]    \n"
        "  daddu %[valh], %[valh], %[vall]   \n"
        /* Combined value is in valh */
        "  dmtc0 %[valh],$9, 6               \n"
        "  .set pop\n"                                             \
         :[tmp] "=&r" (tmp) : [valh] "r" (val_high), [vall] "r" (val_low) );

}


#endif /* KERNEL */

/* check the coremask for the last available core */
static inline uint8_t cvm_common_get_last_core(unsigned int mask)
{
   uint8_t i;
   uint8_t buf = 0;
                                                                                                                
   for(i=0;i<16;i++) {
        if(mask & (1 << i)) {
                buf = i;
        }
   }
   return buf;
}

static inline void cvm_common_buffer_init_fast(void *p, uint64_t size)
{
   uint64_t num_cache_lines = ((size + (CVMX_CACHE_LINE_SIZE - 1)) / CVMX_CACHE_LINE_SIZE) << 1;
   uint64_t *ptr = (uint64_t *) p;
   while(num_cache_lines-- > 0) {
      *ptr++ = 0x0L;
      *ptr++ = 0x0L;
      *ptr++ = 0x0L;
      *ptr++ = 0x0L;
      *ptr++ = 0x0L;
      *ptr++ = 0x0L;
      *ptr++ = 0x0L;
      *ptr++ = 0x0L;
   };
}


inline int cvm_common_packet_copy (cvmx_buf_ptr_t *ptr_to, cvmx_buf_ptr_t *ptr_from, int num_bufs, int total_len);

#endif /* __CVM_COMMON_MISC_H__ */
