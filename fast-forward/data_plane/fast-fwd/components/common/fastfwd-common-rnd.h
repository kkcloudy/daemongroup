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

#ifndef __FASTFWD_COMMON_RNDC_H__
#define __FASTFWD_COMMON_RNDC_H__

#include <cvmx-rng.h>


typedef struct _cvm_common_rand_state {
    uint64_t rand_state_key0;
    uint64_t rand_state_key1;
    uint64_t rand_state_key2;
    uint64_t rand_state_v;
    uint64_t rand_state_i;
} cvm_common_rand_state_t;




 
/* must be executed by a single core during global init */
static inline void cvm_common_hw_rand_init(void)
{
#if defined(__KERNEL__) && defined(linux)
    set_c0_status(ST0_CU2);
#endif

    cvmx_rng_enable();
} 


/* do not call before cvm_common_hw_rand_init() */
static inline void cvm_common_hw_rand( uint8_t *output, int length )
{
    uint64_t out;

#if defined(__KERNEL__) && defined(linux)
    set_c0_status(ST0_CU2);
#endif

    while (length >= 8) 
    { 
        *(uint64_t*)output = cvmx_rng_get_random64();
    	length-=8 ;
    	output+=8;
    }
    /* check remaining non-8-byte boundary */
    if (length) 
    {
        out = cvmx_rng_get_random64();
    	*(uint64_t*)output = out >> (64 - (length<<3));
    }

    return;    
}


/* random externs */
extern void cvm_common_hw_rand_init(void);
extern void cvm_common_hw_rand( uint8_t *output, int length );


static inline void cvm_common_rand8_init(cvm_common_rand_state_t *rstate)
{	
    uint64_t dt;
    uint64_t id = cvmx_get_core_num();
    cvm_common_hw_rand((uint8_t *)&dt, 8);
    dt += id;
    cvm_common_hw_rand((uint8_t *)&(rstate->rand_state_key0), 8);
    rstate->rand_state_key0 += id;
    cvm_common_hw_rand((uint8_t *)&(rstate->rand_state_key1), 8);
    rstate->rand_state_key1 += id;
    cvm_common_hw_rand((uint8_t *)&(rstate->rand_state_key2), 8);
    rstate->rand_state_key2 += id;
    cvm_common_hw_rand((uint8_t *)&(rstate->rand_state_v), 8);
    rstate->rand_state_v += id;

#ifndef OCTEON_EXP
    
#if defined(__KERNEL__) && defined(linux)
    set_c0_status(ST0_CU2);
#endif

    CVMX_MT_3DES_IV(0);
    CVMX_MT_3DES_KEY(rstate->rand_state_key0, 0);
    CVMX_MT_3DES_KEY(rstate->rand_state_key1, 1);
    CVMX_MT_3DES_KEY(rstate->rand_state_key2, 2);
    CVMX_MT_3DES_ENC_CBC(dt);
    CVMX_MF_3DES_RESULT(rstate->rand_state_i);
#endif
}

/* returns 8 bytes of random data -- based on ANSI X9.31 */
static inline uint64_t cvm_common_rand8(cvm_common_rand_state_t *rstate)
{
    uint64_t r;

#ifdef OCTEON_EXP
    r = cvmx_rng_get_random64();
    return (r);
#else

#if defined(__KERNEL__) && defined(linux)
    set_c0_status(ST0_CU2);
#endif

    CVMX_MT_3DES_KEY(rstate->rand_state_key0, 0);
    CVMX_MT_3DES_KEY(rstate->rand_state_key1, 1);
    CVMX_MT_3DES_KEY(rstate->rand_state_key2, 2);
 try_again:
    CVMX_MT_3DES_IV(rstate->rand_state_v);
    CVMX_MT_3DES_ENC_CBC(rstate->rand_state_i);
    CVMX_MF_3DES_RESULT(r);
    CVMX_MT_3DES_ENC_CBC(rstate->rand_state_i);
    CVMX_MF_3DES_RESULT(rstate->rand_state_v);
    if (r != 0)
    {
        return (r);
    }
    else
    {
        goto try_again;
    }
#endif
}

#endif /* __CVM_COMMON_RND_H__ */
