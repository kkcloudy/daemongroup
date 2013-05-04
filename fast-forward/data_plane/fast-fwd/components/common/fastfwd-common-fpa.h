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

#ifndef __FASTFWD_COMMON_FPA_H__
#define __FASTFWD_COMMON_FPA_H__

#include "cvmx.h"

#if !defined(__KERNEL__)

#include "cvmx-fau.h"

/* structure to hold fpa buffer information */

#define CVM_COMMON_MAX_FPA_INFO_ENTRIES 5

typedef struct _cvm_common_fpa_info_entry {
    uint64_t start_addr;
    uint64_t end_addr;
    uint64_t element_size;
    uint64_t info_base;
} cvm_common_fpa_info_entry_t;

typedef struct _cvm_common_fpa_info {
    int count;
    int unused;
    cvm_common_fpa_info_entry_t entry[CVM_COMMON_MAX_FPA_INFO_ENTRIES];
} cvm_common_fpa_info_t;

/* fpa information */
extern cvm_common_fpa_info_t cvm_common_fpa_info[CVMX_FPA_NUM_POOLS];


#ifdef FPA_SANITY_BREAKPOINT
#define CVM_COMMON_BREAK asm volatile ("sdbbp 1");
#else
#define CVM_COMMON_BREAK 
#endif



/** Add FPA allocated memory information
 *
 *  @param pool       - FPA pool number
 *  @param buffer     - pointer to the start address of the allocated memory
 *  @param block_size - size of each individual buffer in this pool (in bytes)
 *  @param num_blocks - number of buffers allocated
 *  @param base       - address of extra memory allocated to keep track of FPA buffers
 *
 * This function is used to add the FPA allocated 
 * information to future reference.
 *
 * Note that this function should be called at
 * the initializations time by the boot core.
 *
 */
static inline int cvm_common_fpa_add_pool_info(uint64_t pool, void* buffer, uint64_t block_size, uint64_t num_blocks, uint64_t base)
{
    static char pools_initialized = 0;
    int ptr = 0;

    if (!pools_initialized)
    {
        int k = 0;
	for (k=0; k<CVMX_FPA_NUM_POOLS; k++) cvm_common_fpa_info[k].count = 0;

        pools_initialized = 1;
    }

    if (!buffer)
    {
        printf("ERROR: cvm_fpa_add_pool: NULL buffer pointer!\n");
        return(-1);
    }
    if (pool >= CVMX_FPA_NUM_POOLS)
    {
        printf("ERROR: cvm_fpa_add_pool: Illegal pool!\n");
        return(-1);
    }
    if (block_size < CVMX_FPA_MIN_BLOCK_SIZE)
    {
        printf("ERROR: cvm_fpa_add_pool: Block size too small.\n");
        return(-1);
    }

    if (cvm_common_fpa_info[pool].count == CVM_COMMON_MAX_FPA_INFO_ENTRIES)
    {
        printf("ERROR: cvm_fpa_add_pool: No more space for pool %lld to hold another entry!\n", CAST64(pool));
        return(-1);
    }

    ptr = cvm_common_fpa_info[pool].count;
    cvm_common_fpa_info[pool].entry[ptr].start_addr = CAST64(buffer);
    cvm_common_fpa_info[pool].entry[ptr].end_addr   = (CAST64(buffer) + (block_size*num_blocks))-1;
    cvm_common_fpa_info[pool].entry[ptr].element_size = block_size;
    cvm_common_fpa_info[pool].entry[ptr].info_base = base;
    ptr++;
    cvm_common_fpa_info[pool].count = ptr;

    return (0);
}


static inline int cvm_common_fpa_display_pool_info(uint64_t pool)
{
    int i=0;
    cvm_common_fpa_info_t pool_info = cvm_common_fpa_info[pool];

    printf("POOL %lld (each buffer is %lld bytes):\n", CAST64(pool), CAST64(pool_info.entry[0].element_size));
    for (i=0; i<pool_info.count; i++)
    {
      printf("  [%d] : start = 0x%llX,  end = 0x%llX, entries = %lld\n",
             i, 
             CAST64(pool_info.entry[i].start_addr), 
	     CAST64(pool_info.entry[i].end_addr),
             CAST64((pool_info.entry[i].end_addr- pool_info.entry[i].start_addr+1)/pool_info.entry[0].element_size));
    }

    printf("\n");

    return (0);
}


/** Display FPA allocated memory information
 *
 * This function displays various FPA allocated
 * pools and their corresponding memory usage
 *
 */
static inline int cvm_common_fpa_display_all_pool_info(void)
{
    int i = 0;
    for (i=0; i<CVMX_FPA_NUM_POOLS; i++)
    {
        if (cvm_common_fpa_info[i].count)
	{
            cvm_common_fpa_display_pool_info(i);
	}
    }

    return (0);
}


static inline int cvm_common_fpa_find_pool (uint64_t ptr, uint64_t *size)
{
    int pool = -1;
    cvm_common_fpa_info_t pool_info; 
    
    for (pool=0; pool<CVMX_FPA_NUM_POOLS; pool++)
    {
        pool_info = cvm_common_fpa_info[pool];
	
	if ( (uint64_t)CAST64(cvmx_phys_to_ptr(ptr)) >= pool_info.entry[pool].start_addr &&
	     (uint64_t)CAST64(cvmx_phys_to_ptr(ptr)) <= pool_info.entry[pool].end_addr) 
        {
            *size = pool_info.entry[pool].element_size; 

	    return (pool);
	}
    }

    return (pool);
}

#define CVM_COMMON_FPA_AVAIL_COUNT(pool) cvmx_read_csr(CVMX_FPA_QUEX_AVAILABLE(pool))

#define CVM_COMMON_GET_FPA_USE_COUNT(pool) \
({ \
    uint32_t count = 0; \
    switch(pool) \
    { \
        case 0:  \
            count = cvmx_fau_fetch_and_add32(CVM_FAU_REG_POOL_0_USE_COUNT, 0); \
            break; \
        case 1:  \
            count = cvmx_fau_fetch_and_add32(CVM_FAU_REG_POOL_1_USE_COUNT, 0); \
            break; \
        case 2:  \
            count = cvmx_fau_fetch_and_add32(CVM_FAU_REG_POOL_2_USE_COUNT, 0); \
            break; \
        case 3:  \
            count = cvmx_fau_fetch_and_add32(CVM_FAU_REG_POOL_3_USE_COUNT, 0); \
            break; \
        case 4:  \
            count = cvmx_fau_fetch_and_add32(CVM_FAU_REG_POOL_4_USE_COUNT, 0); \
            break; \
        case 5:  \
            count = cvmx_fau_fetch_and_add32(CVM_FAU_REG_POOL_5_USE_COUNT, 0); \
            break; \
        case 6:  \
            count = cvmx_fau_fetch_and_add32(CVM_FAU_REG_POOL_6_USE_COUNT, 0); \
            break; \
        case 7:  \
            count = cvmx_fau_fetch_and_add32(CVM_FAU_REG_POOL_7_USE_COUNT, 0); \
            break; \
    } \
    count;\
})

#define CVM_COMMON_FPA_USE_COUNT_ADD(pool, val) \
{ \
    switch(pool) \
    { \
        case 0:  \
            cvmx_fau_atomic_add32(CVM_FAU_REG_POOL_0_USE_COUNT, val); \
            break; \
        case 1:  \
            cvmx_fau_atomic_add32(CVM_FAU_REG_POOL_1_USE_COUNT, val); \
            break; \
        case 2:  \
            cvmx_fau_atomic_add32(CVM_FAU_REG_POOL_2_USE_COUNT, val); \
            break; \
        case 3:  \
            cvmx_fau_atomic_add32(CVM_FAU_REG_POOL_3_USE_COUNT, val); \
            break; \
        case 4:  \
            cvmx_fau_atomic_add32(CVM_FAU_REG_POOL_4_USE_COUNT, val); \
            break; \
        case 5:  \
            cvmx_fau_atomic_add32(CVM_FAU_REG_POOL_5_USE_COUNT, val); \
            break; \
        case 6:  \
            cvmx_fau_atomic_add32(CVM_FAU_REG_POOL_6_USE_COUNT, val); \
            break; \
        case 7:  \
            cvmx_fau_atomic_add32(CVM_FAU_REG_POOL_7_USE_COUNT, val); \
            break; \
    } \
}



/** Checks whether the address belongs to correct FPA pool or not
 *
 *  @param pool     - FPA pool number
 *  @param ptr      - address to verify
 *
 * This macro checks that the address 'ptr' belongs
 * to the 'pool' or not.
 *
 */
#define CVM_COMMON_CHECK_FPA_POOL(pool, ptr) \
({ \
    char correct_pool = 0; \
    cvm_common_fpa_info_t pool_info = cvm_common_fpa_info[pool]; \
    int i=0; \
    for (i=0; i<pool_info.count; i++) \
    { \
      if (( (uint64_t)(CAST64(cvmx_phys_to_ptr(CAST64(ptr)))) >= pool_info.entry[i].start_addr) && ( (uint64_t)(CAST64(cvmx_phys_to_ptr(CAST64(ptr)))) <= pool_info.entry[i].end_addr)) \
        { \
            correct_pool = 1; \
            break; \
        } \
    } \
    correct_pool; \
})


/** Macro to return FPA pool number for a memory address
 *
 *  @param ptr      - address pointer
 *
 * This macro goes through the stored information to
 * find out which pool the pointer 'ptr' belongs to
 *
 */
#define CVM_COMMON_WHICH_FPA_POOL(ptr) \
({ \
    int pool = -1; \
    int i = 0; \
    for (i=0; i<CVMX_FPA_NUM_POOLS; i++) \
    { \
        cvm_common_fpa_info_t pool_info = cvm_common_fpa_info[i]; \
        int j=0; \
        for (j=0; j<pool_info.count; j++) \
        { \
	  if (( (uint64_t)(CAST64(cvmx_phys_to_ptr(CAST64(ptr)))) >= pool_info.entry[j].start_addr) && ((uint64_t)(CAST64(cvmx_phys_to_ptr(CAST64(ptr)))) <= pool_info.entry[j].end_addr)) \
            { \
                pool = i; \
                break; \
            } \
        } \
        if (pool != -1) break; \
    } \
    pool;\
})

#define CVM_COMMON_WHICH_FPA_POOL_SEGMENT(ptr, pool) \
({ \
    int segment = -1; \
    cvm_common_fpa_info_t pool_info = cvm_common_fpa_info[pool]; \
    int j = 0; \
    for (j=0; j<pool_info.count; j++) \
    { \
      if (((uint64_t)(CAST64(cvmx_phys_to_ptr(CAST64(ptr)))) >= pool_info.entry[j].start_addr) && ((uint64_t)(CAST64(cvmx_phys_to_ptr(CAST64(ptr)))) <= pool_info.entry[j].end_addr)) \
        { \
            segment = j; \
            break; \
        } \
    } \
    segment;\
})

#endif /* __KERNEL__*/


#ifdef FPA_CHECKS

/** Macro to initialize various FPA checks including double alloc and free
 *
 *  @param pool_mem_base      - memory base address for an FPA pool
 *  @param pool_count         - number of buffer entires in a specific pool
 *  @param single_buffer_size - size of individual buffer in pool (in bytes)
 *
 * This macro allocates extra memory to keep track of FPA 
 * double alloc and double free errors
 *
 * This function should be called at
 * the initializations time by the boot core.
 *
 * Note that this feature is only available when both the
 * SANITY_CHECKS and FPA_CHECKS flags are defined
 *
 */
#define CVM_COMMON_INIT_FPA_CHECKS(pool_mem_base, pool_count, single_buffer_size) \
({ \
    void *base = NULL; \
    int k = 0; \
    uint64_t ptr_addr = 0; \
    uint64_t *u64_ptr = NULL; \
    base = cvmx_bootmem_alloc(8 * pool_count, CVMX_CACHE_LINE_SIZE); \
    if (base == NULL) \
    { \
        printf("CVM_COMMON_INIT_FPA_CHECKS : Out of memory initializing fpa pool [buffe size = %d bytes]\n", single_buffer_size); \
    } \
    else \
    { \
        memset(base, 0, 8 * pool_count); \
        u64_ptr = CASTPTR(uint64_t,base); \
        ptr_addr= CAST64(pool_mem_base);  \
        for (k = 0;  k < pool_count; k++) \
        { \
	    *u64_ptr = (uint64_t)( (ptr_addr & (0xffffull) ) << 32); \
	    u64_ptr++; \
            ptr_addr += (uint64_t)single_buffer_size; \
        } \
    } \
    base; \
})


#define CVM_COMMON_CHECK_FPA_BUFFER_STATE(ptr, pool) \
({ \
    int segment = CVM_COMMON_WHICH_FPA_POOL_SEGMENT(ptr, pool); \
    cvm_common_fpa_info_entry_t entry = cvm_common_fpa_info[pool].entry[segment]; \
    uint32_t index = (uint32_t)(ptr - entry.start_addr) / (uint32_t)entry.element_size; \
    if (segment == -1) \
        printf("check_buffer_state: can't find segment (ptr=%lX pool=%d)\n",ptr,pool); \
    (((uint64_t *)(entry.info_base))[index]) & 0x1; \
})

#define CVM_COMMON_GET_FPA_BUFFER_STATE(ptr, pool) \
({ \
    int segment = CVM_COMMON_WHICH_FPA_POOL_SEGMENT(ptr, pool); \
    cvm_common_fpa_info_entry_t entry = cvm_common_fpa_info[pool].entry[segment]; \
    uint32_t index = (uint32_t)(ptr - entry.start_addr) / (uint32_t)entry.element_size; \
    if (segment == -1) \
        printf("check_buffer_state: can't find segment (ptr=%lX pool=%d)\n",ptr,pool); \
    (((uint64_t *)(entry.info_base))[index]); \
})


/** Macro to mark an FPA buffer as allocated
 *
 *  @param ptr      - FPA memory address
 *  @param pool     - FPA pool to which the 'ptr' belongs to
 *  @param label    - a specific name to this mark call
 *
 * Note that this feature is only available when both the
 * SANITY_CHECKS and FPA_CHECKS flags are defined
 *
 */
#define CVM_COMMON_MARK_FPA_BUFFER_ALLOC(ptr, pool, label) \
({ \
    int ret=0; \
    ret = cvm_common_mark_fpa_buffer_alloc(__FILE__, __LINE__, ptr, pool, label); \
    ret; \
})

static inline int cvm_common_mark_fpa_buffer_alloc(char* file, int line, uint64_t ptr, uint32_t pool, int label)
{
   uint64_t state, temp;
   int segment = CVM_COMMON_WHICH_FPA_POOL_SEGMENT((uint64_t)ptr, (uint32_t)pool);
   cvm_common_fpa_info_entry_t entry = cvm_common_fpa_info[pool].entry[segment];
   uint32_t index = (uint32_t)(ptr - entry.start_addr) / (uint32_t)entry.element_size;
   uint8_t count;
   int ret = 0;

   if (segment == -1)
   {
       printf("[%d]: can't find segment (ptr=%llX pool=%lld)\n",label,CAST64(ptr),CAST64(pool));
   }

   /*CVM_COMMON_TRACE_P2 (CVM_COMMON_TRACE_BODY, CVM_COMMON_TRACE_NOT_FNPTR, CVM_COMMON_TRACE_MACRO_MARK_BUFFER_ALLOC, ptr, pool);*/

 fpa_alloc_do_again:

   CVMX_LLD(state, &((CASTPTR(uint64_t,(entry.info_base)))[index]), 0);

   if ((state & 0x1) == 0)
   {
       state += 1;
       count = (state >> 8) & 0xff;
       count++;
       state &= 0xffffffffffff00ffull;
       state += (((uint64_t)count) << 8);
       state &= 0xffffffff00ffffffull;
       CVMX_SCD(state, &((CASTPTR(uint64_t,entry.info_base))[index]), 0);
       if (state == 0) goto fpa_alloc_do_again;
   }
   else
   {
       count = (state >> 8) & 0xff;
       count++;
       state &= 0xffffffffffff00ffull;
       state += (((uint64_t)count) << 8);
       temp=state;
       CVMX_SCD(state, &((CASTPTR(uint64_t,entry.info_base))[index]), 0);
       if (state == 0) goto fpa_alloc_do_again;
       printf("[%d : 0x%llX]: buffer is already allocated (file %s, line=%d, buffer=%llx pool=%llx ac=%llx fc=%llx)\n",
	      label,CAST64(cvmx_get_cycle()), file, line, CAST64(ptr), CAST64(pool), CAST64((uint32_t)((temp>>8)&0xff)),
              CAST64((uint32_t)((temp>>16)&0xff)));
       CVM_COMMON_BREAK;
       ret = 1;
     }

   return (ret);
}



/** Macro to mark an FPA buffer as free
 *
 *  @param ptr      - FPA memory address
 *  @param pool     - FPA pool to which the 'ptr' belongs to
 *  @param label    - a specific name to this mark call
 *
 * Note that this feature is only available when both the
 * SANITY_CHECKS and FPA_CHECKS flags are defined
 *
 */
#define CVM_COMMON_MARK_FPA_BUFFER_FREE(ptr, pool, label) \
({ \
    int ret=0; \
    ret = cvm_common_mark_fpa_buffer_free(__FILE__, __LINE__, ptr, pool, label); \
    ret; \
})


static inline int cvm_common_mark_fpa_buffer_free(char* file, int line, uint64_t ptr, uint32_t pool, int label)
{
   uint64_t state, temp;
   int segment = CVM_COMMON_WHICH_FPA_POOL_SEGMENT((uint64_t)ptr, (uint32_t)pool);
   cvm_common_fpa_info_entry_t entry = cvm_common_fpa_info[pool].entry[segment];
   uint32_t index = (uint32_t)(ptr - entry.start_addr) / (uint32_t)entry.element_size;
   uint8_t count;
   int ret = 0;

   if (segment == -1)
   {
       printf("[%d]: can't find segment (ptr=%llX pool=%lld)\n",label,CAST64(ptr),CAST64(pool));
   }

   CVM_COMMON_TRACE_P2 (CVM_COMMON_TRACE_BODY, CVM_COMMON_TRACE_NOT_FNPTR, CVM_COMMON_TRACE_MACRO_MARK_BUFFER_FREE, ptr, pool);

fpa_free_do_again:

   CVMX_LLD(state, &((CASTPTR(uint64_t,entry.info_base))[index]), 0);
   if ((state & 0x1) == 1)
   {
       state -= 1;
       count = (state >> 16) & 0xff;
       count++;
       state &= 0xffffffffff00ffffull;
       state += (((uint64_t)count) << 16);
       CVMX_SCD(state, &((CASTPTR(uint64_t,entry.info_base))[index]), 0);
       if (state == 0) goto fpa_free_do_again;
   }
   else
   {
       count = (state >> 16) & 0xff;
       count++;
       state &= 0xffffffffff00ffffull;
       state += (((uint64_t)count) << 16);
       temp = state;
       CVMX_SCD(state, &((CASTPTR(uint64_t,entry.info_base))[index]), 0);
       if (state == 0) goto fpa_free_do_again;

       printf("[%d : 0x%llX]: buffer is already freed (file %s, line=%d, buffer=%llx pool=%llx ac=%llx fc=%llx)\n",
	      label,CAST64(cvmx_get_cycle()), file, line, CAST64(ptr),CAST64((uint32_t)pool),CAST64((uint32_t)((temp>>8)&0xff)),
	      CAST64((uint32_t)((temp>>16)&0xff)));
       CVM_COMMON_BREAK;
       ret = 1;
   }

   return (ret);
}



/*
 * Check FPA buffer boundries
 * Make sure that we don't free a buffer which
 * may result in over-writing some other fpa
 * buffer partially
 */

#define IS_PTR_WITHIN_BUFFER(ptr_in_buffer, ptr_to_test) \
({ \
    int ret=0; \
    ret = is_ptr_within_buffer(__FILE__, __LINE__, (uint64_t)ptr_in_buffer, (uint64_t)ptr_to_test); \
    ret; \
})

static inline int is_ptr_within_buffer(char* file, int line, uint64_t ptr_in_buffer, uint64_t ptr_to_test)
{
    int pool = CVM_COMMON_WHICH_FPA_POOL(ptr_in_buffer);
    int segment;
    cvm_common_fpa_info_entry_t entry;
    uint32_t index;
    uint64_t state;
    uint64_t buf_start, buf_end;
    int ret = 0;

    if (pool == -1)
    {
        printf("%s: Failed to find the pool for buffer 0x%llX\n", __FUNCTION__, CAST64(ptr_in_buffer)); \
        ret = -1;
    }
    else
    {
        segment = CVM_COMMON_WHICH_FPA_POOL_SEGMENT((uint64_t)ptr_in_buffer, (uint32_t)pool);
        entry  = cvm_common_fpa_info[pool].entry[segment];
        index = (uint32_t)((uint64_t)ptr_in_buffer - entry.start_addr) / (uint32_t)entry.element_size;
        state = (CASTPTR(uint64_t,entry.info_base))[index];
        buf_start = ptr_in_buffer & 0xffffffffffff0000ull;
        buf_start |= ((state >> 32) & 0xffff);
        buf_end = buf_start + entry.element_size;

        if ( (ptr_to_test < buf_start) || (ptr_to_test > buf_end) )
        {
            printf("%s: Invalid pointer 0x%llX (buf start=0x%llX, buf end=0x%llX, ptr=0x%llX, file=%s, line=%d)\n",
	           __FUNCTION__, CAST64(ptr_to_test), CAST64(buf_start), CAST64(buf_end), CAST64(ptr_in_buffer),
		   file, line);

            ret = -1;
            CVM_COMMON_BREAK;
        }
        else
        {
            ret = 0;
        }
    }

    return (ret);
}


/** Macro to verify FPA buffer boundries
 *
 *  @param ptr      - FPA memory address
 *  @param pool     - FPA pool number
 *  @param back     - back field in cvmx_buf_ptr_t
 *  @param calculate_ptr_using_back - if set, the back field is used to verify the boundries
 *
 * This macro checks whether the 'ptr' address lies within
 * the single buffer belonging to the FPA 'pool'. This
 * is helpful in a case where the user wants to check
 * that after changing the 'ptr' value, it still stays
 * with the same FPA buffer.
 *
 * Note that this feature is only available when the
 * SANITY_CHECKS flag is defined
 *
 */
#define CVM_COMMON_CHECK_BUFFER_BOUNDRIES(ptr, pool, back, calculate_ptr_using_back) \
({ \
    int ret=0; \
    ret = cvm_common_check_buffer_boundries(__FILE__, __LINE__, ptr, pool, back, calculate_ptr_using_back); \
    ret; \
})


static inline int cvm_common_check_buffer_boundries(char* file, int line, uint64_t ptr, int pool, int back, int calculate_ptr_using_back)
{
    int ret = 0;
    uint64_t buffer_start_addr = (uint64_t)ptr;
    int segment = CVM_COMMON_WHICH_FPA_POOL_SEGMENT((uint64_t)ptr, (uint32_t)pool);
    cvm_common_fpa_info_entry_t entry = cvm_common_fpa_info[pool].entry[segment];
    uint32_t index = (uint32_t)((uint64_t)ptr - entry.start_addr) / (uint32_t)entry.element_size;
    uint64_t state = (CASTPTR(uint64_t,entry.info_base))[index];

    if (calculate_ptr_using_back)
    {
        buffer_start_addr = CAST64(cvmx_phys_to_ptr(( (uint64_t)ptr & ~127) - 128*back));
    }
    if ( ( (state >> 32) & 0xffff) != ( buffer_start_addr & 0xffff) )
    {
        printf("Buffer boundry check: Trying to free buffer using incorrect address (ptr=0x%llx, pool=0x%x, back=0x%x, file=%s, line=%d)\n", 
               CAST64(ptr), pool, back, file, line);
        ret = 1;
    }

    return (ret);
}


#else

#define CVM_COMMON_MARK_FPA_BUFFER_ALLOC(ptr, pool, label);
#define CVM_COMMON_MARK_FPA_BUFFER_FREE(ptr, pool, label);

static inline int cvm_common_mark_fpa_buffer_alloc(char* file, int line, uint64_t ptr, uint32_t pool, int label)
{
    return (0);
}

static inline int cvm_common_mark_fpa_buffer_free(char* file, int line, uint64_t ptr, uint32_t pool, int label)
{
    return (0);
}

#define CVM_COMMON_CHECK_BUFFER_BOUNDRIES(ptr, pool, back, calculate_ptr_using_back) \
({ \
  int ret = 0; \
  ret; \
})

static inline int cvm_common_check_buffer_boundries(char* file, int line, uint64_t ptr, int pool, int back, int calculate_ptr_using_back)
{
    return (0);
}

#define CVM_COMMON_INIT_FPA_CHECKS(pool_mem_base, pool_count, single_buffer_size) \
({ \
    void* base = NULL; \
    base; \
})

#define IS_PTR_WITHIN_BUFFER(ptr_in_buffer, ptr_to_test) \
({ \
  int ret = 0; \
  ret; \
})

#endif /* FPA_CHECKS */


static inline int cvm_common_is_fpa_buffer_available(uint64_t scratch_ptr)
{
#if defined(__KERNEL__) && defined (linux)
	/* FIXME: */
	return 1;
#else
    uint64_t  so = 0; 
    so = cvmx_scratch_read64(scratch_ptr);
    if (so == CVM_COMMON_ALLOC_INIT)
    {
        CVMX_SYNCIOBDMA;
        so = cvmx_scratch_read64(scratch_ptr);
    }

    if (so == 0x0)
    {
        return (0);
    }
    return (1);
#endif
}


#define CVM_COMMON_INC_OOB_COUNT(pool) \
{ \
    int offset = pool*4 + CVM_FAU_REG_FPA_OOB_COUNT; \
    cvmx_fau_atomic_add32( offset, 1); \
}


/** Macro to allocate an FPA buffer
 *
 *  @param pool     - FPA pool from which the buffer should be allocated
 *
 * This macro is a wraper around the cvmx_fpa_alloc()
 * executive function.
 *
 * If SANITY_CHECK and FPA_CHECKS are defined, this 
 * macro performs extra checks to verify the allocated
 * FPA buffer integrity.
 *
 */
#define cvm_common_alloc_fpa_buffer_sync(pool) _alloc_fpa_buffer_sync(__FILE__, __LINE__, pool)

static inline void* _alloc_fpa_buffer_sync(char* file, int line, uint64_t pool)
{
    void *so = NULL;
#if defined(__KERNEL__)
    so = (void *)cvmx_fpa_alloc(pool);
    if (so == NULL)
    {
        printf("cvm_common_alloc_fpa_buffer_sync : OUT OF BUFFERS (file %s, line %d, pool %d)\n", file, line, (int)pool);
	return NULL;
    }
    return so;

#else
  
    so = (void *)cvmx_fpa_alloc(pool);
    if (so == 0)
    {
        /* out of buffers */
        CVM_COMMON_INC_OOB_COUNT(pool);
        return (NULL);
    }

#ifdef SANITY_CHECKS
    if (!(CVM_COMMON_CHECK_FPA_POOL(pool, so)))
    {
        printf("cvm_common_alloc_fpa_buffer_sync : INVALID POOL (pool 0x%llX, buffer 0x%llX, file %s, line %d)\n", CAST64(pool), CAST64(so), file, line);
        CVM_COMMON_BREAK;
    }

    CVM_COMMON_FPA_USE_COUNT_ADD(pool,1);
    cvm_common_mark_fpa_buffer_alloc(file, line, CAST64(so), (uint32_t)pool, 1);
#endif

    return (so);
#endif /*__KERNEL__*/
}



/** Macro to allocate an FPA buffer asynchronously
 *
 *  @param scratch_ptr - Scratch pad register address
 *  @param pool        - FPA pool from which the buffer should be allocated
 *
 * This macro is a wraper around the cvmx_fpa_alloc()
 * executive function. It tries to read an already
 * allocated FPA buffer address from the scratch pad
 * register. Once that is done, another buffer allocation
 * request is initiated to work in the background and
 * results are stored in the same scratch pad register.
 * This register value is used for the next allocation.
 *
 * If SANITY_CHECK and FPA_CHECKS are defined, this 
 * macro performs extra checks to verify the allocated
 * FPA buffer integrity.
 *
 */
#define cvm_common_alloc_fpa_buffer(scratch_ptr, pool) _alloc_fpa_buffer(__FILE__, __LINE__, scratch_ptr, pool)

static inline void* _alloc_fpa_buffer(char* file, int line, uint64_t scratch_ptr, uint64_t pool)
{
#if defined(USE_SYNC_ALLOC) || defined (__KERNEL__)
  return ( _alloc_fpa_buffer_sync(file, line, pool) );
#else

    void *so_ptr = NULL;
    uint64_t  so64 = 0;

    so64 = cvmx_scratch_read64(scratch_ptr);
    if (so64 == CVM_COMMON_ALLOC_INIT)
    {
        CVMX_SYNCIOBDMA;
        so64 = cvmx_scratch_read64(scratch_ptr);
    }

    if (so64 == 0)
    {
        /* out of buffers */
        CVM_COMMON_INC_OOB_COUNT(pool);
    }
    else
    {
        so_ptr = cvmx_phys_to_ptr(so64);
#ifdef SANITY_CHECKS
        if (!(CVM_COMMON_CHECK_FPA_POOL(pool, so64)))
        {
	    printf("cvm_common_alloc_fpa_buffer: INVALID POOL (pool 0x%llX, buffer 0x%llX, file %s, line %d)\n", CAST64(pool), CAST64(so64), file, line);
            CVM_COMMON_BREAK;
        }
        CVM_COMMON_FPA_USE_COUNT_ADD(pool, 1);
        cvm_common_mark_fpa_buffer_alloc(file, line, so64, (uint32_t)pool, 1);
#endif
    }

    /* set scratch reg to CVM_COMMON_ALLOC_INIT so that we can verify a new ptr is written */
    cvmx_scratch_write64(scratch_ptr, CVM_COMMON_ALLOC_INIT);
    CVMX_SYNCWS;
    cvmx_fpa_async_alloc(scratch_ptr, pool);

    return (so_ptr);
#endif /* USE_SYNC_ALLOC */
}



/** Macro to free an allocated FPA buffer
 *
 *  @param ptr       - FPA address to be freed
 *  @param pool      - FPA pool to which the buffer should be freed
 *  @num_cache_lines - number of cache lines to be invalidated
 *
 * This macro is a wraper around the cvmx_fpa_free()
 * executive function. 
 *
 * If SANITY_CHECK and FPA_CHECKS are defined, this 
 * macro performs extra checks to verify the FPA buffer 
 * integrity before freeing it.
 *
 */
#define cvm_common_free_fpa_buffer(ptr, pool, num_cache_lines) _free_fpa_buffer(__FILE__, __LINE__, ptr, pool, num_cache_lines)

static inline void _free_fpa_buffer(char* file, int line, void *ptr, uint64_t pool, uint64_t num_cache_lines)
{
#if defined(__KERNEL__)
  CVMX_SYNCW;
  cvmx_fpa_free(ptr, pool, 0);
#else

#ifdef SANITY_CHECKS
    if (!(CVM_COMMON_CHECK_FPA_POOL(pool,ptr)))
    {
        printf("cvm_common_free_fpa_buffer: trying to free a buffer to incorrect pool (buffer 0x%llX, pool 0x%llX, file %s, line %d)\n", 
	       CAST64(ptr), CAST64(pool), file, line);
    }

    cvm_common_check_buffer_boundries( file, line, CAST64(ptr), (int)pool, 0, 0);
    CVM_COMMON_FPA_USE_COUNT_ADD(pool, -1);
    cvm_common_mark_fpa_buffer_free( file, line, CAST64(ptr), (uint32_t)pool, 1);
#endif

    CVMX_SYNCWS;

    if (pool == CVMX_FPA_WQE_POOL)
    {
       cvmx_fpa_free(ptr, pool, CVMX_FPA_WQE_POOL_SIZE / CVMX_CACHE_LINE_SIZE);
    }
    else
    {
       cvmx_fpa_free(ptr, pool, 0);
       num_cache_lines = 0; /* keep the compiler happy */
    }

#endif /* __KERNEL __*/
}


#endif /* _CVM_COMMON_FPA_H__ */
