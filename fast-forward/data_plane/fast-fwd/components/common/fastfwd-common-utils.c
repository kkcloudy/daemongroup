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
#elif defined(linux)
#include <malloc.h>
#include "cvmx.h"
#include "cvmx-wqe.h"
#else
#include <stdio.h>
#include <string.h>
#include "cvmx.h"
#include "cvmx-wqe.h"
#endif


#include "cvmx-config.h"
#if !defined(__KERNEL__)

#include "cvmx-spinlock.h"
#include "cvmx-malloc.h"
#include "cvmx-pow.h"
#include "cvmx-fau.h"
#include "cvmx-bootmem.h"
#include "cvmx-coremask.h"
#include "cvmx-scratch.h"

#endif

#include "cvmx-helper.h"
#include "cvmx-helper-util.h"
#include "cvmx-fpa.h"
#include "cvmx-fau.h"
#include "fastfwd-common-defs.h"
#include "fastfwd-common-misc.h"
#include "fastfwd-common-fpa.h"

/*
 * returns the version number based on CVS tag
 */
inline const char *cvm_common_get_version(void)
{
    static char version[80];
    const char *cavium_parse = "$Name: FastFWD_1_1_0_build_001 ";

    if (cavium_parse[7] == ' ')
    {
        snprintf(version, sizeof(version), "Internal %s", __DATE__);
    }
    else
    {
        char *major = NULL;
        char *minor1 = NULL;
        char *minor2 = NULL;
        char *build = NULL;
        char *buildnum = NULL;
        char *end = NULL;
        char buf[80];

        strncpy(buf, cavium_parse, sizeof(buf));
        buf[sizeof(buf)-1] = 0;

        major = strchr(buf, '_');
        if (major)
        {
            major++;
            minor1 = strchr(major, '_');
            if (minor1)
            {
                *minor1 = 0;
                minor1++;
                minor2 = strchr(minor1, '_');
                if (minor2)
                {
                    *minor2 = 0;
                    minor2++;
                    build = strchr(minor2, '_');
                    if (build)
                    {
                        *build = 0;
                        build++;
                        buildnum = strchr(build, '_');
                        if (buildnum)
                        {
                            *buildnum = 0;
                            buildnum++;
                            end = strchr(buildnum, ' ');
                            if (end)
                                *end = 0;
                        }
                    }
                }
            }
        }

        if (major && minor1 && minor2 && build && buildnum && (strcmp(build, "build") == 0))
            snprintf(version, sizeof(version), "%s.%s.%s, build %s", major, minor1, minor2, buildnum);
        else
            snprintf(version, sizeof(version), "%s", cavium_parse);
    }

    return version;
}


int cvm_common_global_init(void)
{
#if !defined(__KERNEL__)
    int common_malloc_arena_size = CVM_COMMON_ARENA_SIZE;
    int num_common_arenas = CVM_COMMON_ARENA_COUNT;
	
    cvmx_spinlock_init(&cvm_common_malloc_lock);
#endif

#if !defined(__KERNEL__)
    /* allocate arenas for components */
    if ( (cvm_common_allocate_arenas(common_malloc_arena_size, num_common_arenas)) < 0 )
    {
        return (-1);
    }
#endif

    return (RETURN_OK);
}

int cvm_common_local_init(void)
{
    return (RETURN_OK);
}

/*
 * Common Memory allocation routines
 */
int cvm_common_allocate_arenas(uint64_t arena_size, uint64_t arena_count)
{
#if defined(__KERNEL__) 
   return 0;
#elif defined(linux)
   return 0;
#else

    uint32_t i;
    uint8_t *memptr=NULL;

    for(i=0;i<arena_count;i++)
    {
        memptr = (uint8_t*)cvmx_bootmem_alloc(arena_size, arena_size);

        if (!memptr || cvmx_add_arena(&cvm_common_arenas, memptr, arena_size) < 0)
        {
	    printf("%s : error adding arena: %u!\n", __FUNCTION__, (unsigned int)i);
            return (-1);
        }
    }
#endif

    return (0);
}

void
cvm_common_dump_data(void *d, unsigned int size)
{
	unsigned int    i;
	uint8_t        *data = (uint8_t *)d;

    //printf("Printing %d bytes @ 0x%p\n",size, data);
    for(i = 0; i < size; i++) {
        printf(" %02x", data[i]);
        if(!((i+1) & 0x7))
            printf("\n");
    }
    printf( "\n");
}


/**
 * Debug routine to dump the packet structure to the console
 * 
 * @param work   Work queue entry containing the packet to dump
 */
void cvm_common_dump_special(cvmx_wqe_t *work)
{
    FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN,FASTFWD_COMMON_DBG_LVL_DEBUG, 
		"Packet Type:   %u\n", CVM_WQE_GET_UNUSED(work));
    FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN,FASTFWD_COMMON_DBG_LVL_DEBUG, 
		"Packet Length:   %u\n", CVM_WQE_GET_LEN(work));
    FASTFWD_COMMON_DBG_DUMP(FASTFWD_COMMON_MOUDLE_MAIN,FASTFWD_COMMON_DBG_LVL_DEBUG, 
		(uint8_t *)work->packet_data, CVM_WQE_GET_LEN(work));
}

/**
 * Debug routine to dump the packet structure to the console
 * 
 * @param work   Work queue entry containing the packet to dump
 */
void cvm_common_dump_packet(cvmx_wqe_t *work, int dir)
{
    cvmx_buf_ptr_t  buffer_ptr;
    uint32_t sadr, dadr;
    uint16_t sprt, dprt, tflg;
    buffer_ptr = work->packet_ptr;
    int iplen;
    int             len;
    uint64_t        remaining_bytes;
    uint64_t        start_of_buffer;
    uint64_t virt_ptr = CAST64(cvmx_phys_to_ptr(buffer_ptr.s.addr));

    if (*( CASTPTR(uint16_t,(virt_ptr+12)) ) == 0x800) 
      {
	sadr = ( CASTPTR(cvm_common_ip_hdr_t,(virt_ptr+14)))->ip_src;
	dadr = ( CASTPTR(cvm_common_ip_hdr_t,(virt_ptr+14)))->ip_dst;
	iplen = (( CASTPTR(cvm_common_ip_hdr_t,(virt_ptr+14)) )->ip_hl)<<2;
	sprt = ( CASTPTR(cvm_common_tcp_hdr_t,(virt_ptr+14+iplen)))->th_sport;
	dprt = ( CASTPTR(cvm_common_tcp_hdr_t,(virt_ptr+14+iplen)))->th_dport;
	tflg = ( CASTPTR(cvm_common_tcp_hdr_t,(virt_ptr+14+iplen)))->th_flags;
	
	if (dir == 0) /* rx */
	  {
	    FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN,FASTFWD_COMMON_DBG_LVL_DEBUG,
			"RX len=%u sadr=%llx dadr=%llx sprt=%x dprt=%x tflg=%x\n",
			       CVM_WQE_GET_LEN(work), CVM_COMMON_UCAST64(sadr), CVM_COMMON_UCAST64(dadr), sprt, dprt, tflg);
	  }
	else /* dir == 1 , tx */
	  {
	    FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN,FASTFWD_COMMON_DBG_LVL_DEBUG,  
			"TX len=%u sadr=%llx dadr=%llx sprt=%x dprt=%x tflg=%x\n",
			       CVM_WQE_GET_LEN(work), CVM_COMMON_UCAST64(sadr), CVM_COMMON_UCAST64(dadr), sprt, dprt, tflg);
	  }
      }
    else if (CVM_WQE_GET_PORT(work) != 32) /* not IP */
      {
	if (dir == 0) /* rx */
	  {
	    FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN,FASTFWD_COMMON_DBG_LVL_DEBUG, 
			"RX len=%u NOT IP\n", CVM_WQE_GET_LEN(work));
	  }
	else
	  {
	    FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN,FASTFWD_COMMON_DBG_LVL_DEBUG,  
			"TX len=%u NOT IP\n", CVM_WQE_GET_LEN(work));
	  }
      }

    FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN,FASTFWD_COMMON_DBG_LVL_DEBUG,  
		"Packet Length:   %u\n", CVM_WQE_GET_LEN(work));
    FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN,FASTFWD_COMMON_DBG_LVL_DEBUG, 
		"    Input Port:  %u\n", CVM_WQE_GET_PORT(work));
    FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN,FASTFWD_COMMON_DBG_LVL_DEBUG,  
		"    QoS:         %u\n", CVM_WQE_GET_QOS(work));
    buffer_ptr = work->packet_ptr;
    remaining_bytes = CVM_WQE_GET_LEN(work);

    while (remaining_bytes)
    {
        virt_ptr = CAST64(cvmx_phys_to_ptr(buffer_ptr.s.addr));

        start_of_buffer = ((virt_ptr >> 7) - buffer_ptr.s.back) << 7;
        FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN,FASTFWD_COMMON_DBG_LVL_DEBUG,  
			"    Buffer Start:%llx\n", CAST64(start_of_buffer));
        FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN,FASTFWD_COMMON_DBG_LVL_DEBUG, 
			"    Buffer I   : %u\n", buffer_ptr.s.i);
        FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN,FASTFWD_COMMON_DBG_LVL_DEBUG, 
			"    Buffer Back: %u\n", buffer_ptr.s.back);
        FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN,FASTFWD_COMMON_DBG_LVL_DEBUG, 
			"    Buffer Pool: %u\n", buffer_ptr.s.pool);
        FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN,FASTFWD_COMMON_DBG_LVL_DEBUG, 
			"    Buffer Data: %llx\n", CAST64(virt_ptr));
        FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN,FASTFWD_COMMON_DBG_LVL_DEBUG,  
			"    Buffer Size: %u\n", buffer_ptr.s.size);

        len = (remaining_bytes < buffer_ptr.s.size) ? remaining_bytes : buffer_ptr.s.size;
        FASTFWD_COMMON_DBG_DUMP(FASTFWD_COMMON_MOUDLE_MAIN,FASTFWD_COMMON_DBG_LVL_DEBUG,  CASTPTR(uint8_t,virt_ptr), len)
	remaining_bytes -= len;

        buffer_ptr = *CASTPTR(cvmx_buf_ptr_t,(virt_ptr - 8));
    }
}

void cvm_common_display_fpa_alloc_info(void)
{
    printf("\n");
    cvm_common_fpa_display_all_pool_info();
}

void cvm_common_display_fpa_buffer_count(void)
{
    int i=0;

    printf("\n");
    for (i=0; i<8; i++)
    {
        printf("Pool %2d :%10lld\n", i, CAST64(CVM_COMMON_FPA_AVAIL_COUNT(i)) );
    }
}

#ifdef OCTEON_DEBUG_LEVEL
void cvm_common_get_debug_level(void)
{
    printf("\n");
    printf("Current debug level is %d\n", fastfwd_common_debug_level);
}

void cvm_common_set_debug_level(int64_t dbg_lvl)
{
    if ((dbg_lvl >= FASTFWD_COMMON_DBG_LVL_MUST_PRINT) &&
	(dbg_lvl < FASTFWD_COMMON_MAX_DBG_LVL))
    {
        printf("\nDebug level changed from %d to %ld\n", fastfwd_common_debug_level, dbg_lvl);
        fastfwd_common_debug_level = dbg_lvl;
    }
    else
    {
        printf("\nInvalied debug level (%ld) passed\n", dbg_lvl);
    }
}

#endif

#define CVM_COMMON_GET_NPI_ADDR_HI(pool)    (0x2807 + (pool*2))
#define CVM_COMMON_GET_NPI_ADDR_LOW(pool)   (0x2807 + (pool*2) + 1)


uint64_t cvm_common_get_top_fpa_mem(int pool)
{
    uint64_t low_addr = 0;
    uint64_t high_addr = 0;
    uint64_t final_addr = 0;

    cvmx_write_csr(CVMX_NPI_DBG_SELECT, CVM_COMMON_GET_NPI_ADDR_HI(pool));
    high_addr = cvmx_read_csr(CVMX_DBG_DATA);
    high_addr = (high_addr & 0xFFF) << 16;

    cvmx_write_csr(CVMX_NPI_DBG_SELECT, CVM_COMMON_GET_NPI_ADDR_LOW(pool));
    low_addr = cvmx_read_csr(CVMX_DBG_DATA);
    low_addr = (low_addr & 0xFFFF);

    final_addr = (high_addr) | (low_addr);
    final_addr = final_addr << 7;

    return (final_addr);
}


/*
 * In fpa page, two pointers are stored in a 64-bit fashion.
 * The details of each 64-bit word is:
 *
 *   [63]   :  should always be 1
 *  [62:61] :  these 2 bits from the 16, 64-bit words 
 *             written to the L2C is used to build a predictable 
 *             value. High order bits [31:30] are sent first then 
 *             the next 2 MSBits
 *
 *  [60:32] :  first pointer 
 *  [31:29] :  should be 0x7 
 *  [28:0]  :  second pointer
 *
 *
 * Format of 32 bit predictable word is:
 *
 *  [31:29] : should be 0x7 
 *  [28:26] : FPA pool number 
 *    [25]  : should be 0x0 
 *  [24:0]  : Page count number 
 *
 */
int cvm_common_analyze_single_fpa_page(uint64_t page_addr, uint32_t pool)
{
    int i = 0;
    uint32_t  verify_word = 0;
    uint64_t *ptr = CASTPTR(uint64_t, page_addr);

    for (i=0; i<16; i++)
    {
        /* bit [63] should be 1 */
        if (!(*ptr >> 63))
	{
	    printf("%s: pool=%d, page addr=0x%llx, free word=0x%llx  [bit 63 is not set]\n",
		   __FUNCTION__, pool, CAST64(page_addr), CAST64(*ptr));
	    return (1);
	}

	/* bit [31:29] should be 0x7 */
        if ( ((*ptr >> 29) & 0x7) != 0x7 )
	{
	    printf("%s: pool=%d, page addr=0x%llx, free word=0x%llx  [bit 31:29 is not 0x7]\n",
		   __FUNCTION__, pool, CAST64(page_addr), CAST64(*ptr));
	    return (1);
	}

	verify_word <<=2;
	verify_word |= (( (*ptr) >> 61) & 0x3);

	ptr++;
    }

    /* verify word: bit [31:29] should be 0x7 */
    if ( ((verify_word >> 29) & 0x7) != 0x7 )
    {
        printf("%s: pool=%d, page addr=0x%llx, verify word=0x%llx  [bit 31:29 is not 0x7]\n",
	       __FUNCTION__, pool, CAST64(page_addr), CAST64(verify_word));
        return (1);
    }

    /* verify word: bit 25 should be 0x0 */
    if ( ((verify_word >> 25) & 0x1) != 0x0 )
    {
        printf("%s: pool=%d, page addr=0x%llx, verify word=0x%llx  [bit 25 is not 0x0]\n",
	       __FUNCTION__, pool, CAST64(page_addr), CAST64(verify_word));
        return (1);
    }

    /* verify word: pool should match */
    if ( ((verify_word >> 26) & 0x7) != pool )
    {
        printf("%s: pool=%d, page addr=0x%llx, verify word=0x%llx  [pool number %lld is invalid]\n",
	       __FUNCTION__, pool, CAST64(page_addr), CAST64(verify_word), CAST64((verify_word >> 26) & 0x7) );
        return (1);
    }

    /*
    printf("VerifyWord = 0x%x\n", verify_word);
    printf("[31:29]    = 0x%x\n", (verify_word >> 29) & 0x3);
    printf("pool       = %d\n", (verify_word >> 26) & 0x7);
    printf("[25]       = 0x%x\n", (verify_word >> 25) & 0x1);
    printf("Page count = %d\n", (verify_word >> 0) & 0x1ffffff);
    */

    return (0);
}

/*
 * parses the FPA free list
 */
int cvm_common_parsefpa_free_list(uint32_t pool)
{
    uint64_t start_address = 0;
    uint64_t total_free_buffers = 0;
    uint64_t total_free_pages = 0;
    uint64_t mem_rd_addr = 0;
    uint64_t mem_rd_data = 0;
    uint64_t mem_rd_low = 0;
    uint64_t mem_rd_high = 0;

    total_free_buffers = cvmx_read_csr(CVMX_FPA_QUEX_AVAILABLE(pool));
    start_address = cvm_common_get_top_fpa_mem(pool);

    mem_rd_addr = start_address;

    /* check if we have at least a single page buffer   */
    /* Note: the pointer read from the NPI register has */
    /*       28 bits while the one in the page buffer   */
    /*       has 29 bits                                */

    if ( ((mem_rd_addr >> 7) & (0xFFFFFFF)) == 0xFFFFFFF) goto done;

    /* loop through the free list */
    while ( mem_rd_addr !=  (0x1FFFFFFFULL << 7) )
    {
        cvm_common_analyze_single_fpa_page(mem_rd_addr, pool);

        total_free_pages++;

        mem_rd_data  = *(CASTPTR(uint64_t,mem_rd_addr));

        mem_rd_low  = mem_rd_data & 0x1FFFFFFF;
        mem_rd_high = (mem_rd_data >> 32) & 0x1FFFFFFF;

        mem_rd_addr = mem_rd_low << 7;
    }


done:
    /*
    printf("Pool %d: Total pages found   = %lld\n", pool, CAST64(total_free_pages));
    printf("Pool %d: Total buffers found = %lld\n", pool, CAST64(total_free_pages*31 + total_free_pages));
    */

    return (0);
}


void cvm_common_analyze_fpa(uint32_t pool)
{
    uint16_t *buf_array = NULL;
    uint64_t *saved_buffers = NULL;
    uint32_t no_of_buffers = 0;
    int i = 0,k = 0;
    uint32_t  j = 0;
    int cur_index = 0;
    uint64_t buffer_addr;
    int offset[CVM_COMMON_MAX_FPA_INFO_ENTRIES];
    int cur_offset = 0;
    cvm_common_fpa_info_t pool_info;
    int segment;
    cvm_common_fpa_info_entry_t entry;
    uint32_t index;
    uint16_t state=0;
    cvmx_wqe_t *swp=NULL;
    cvmx_fpa_int_sum_t int_sum;

    /* stats */
    int buffer_analyzed = 0;
    int duplicate_buffers = 0;
    int invalid_buffers = 0;
    int unaligned_buffers = 0;

    if(pool > CVMX_FPA_NUM_POOLS)
    {
        printf("Invalid pool number [%d]\n", pool);
        return;
    }

    /*
     * Check FPA interrupts (may show signs of FPA corruption)
     */
    int_sum.u64 = cvmx_read_csr(CVMX_FPA_INT_SUM);

    if (int_sum.s.fed0_sbe) printf("INT: Single Bit Error detected in FPF0\n");
    if (int_sum.s.fed0_dbe) printf("INT: Double Bit Error detected in FPF0\n");
    if (int_sum.s.fed1_sbe) printf("INT: Single Bit Error detected in FPF1\n");
    if (int_sum.s.fed1_dbe) printf("INT: Double Bit Error detected in FPF1\n");

    if ( (int_sum.u64 >> ((pool*3)+4)) & 0x1 ) printf("INT: Page count available goes negative\n");
    if ( (int_sum.u64 >> ((pool*3)+4)) & 0x2 ) printf("INT: Count available is greater than pointers present in the FPA\n");
    if ( (int_sum.u64 >> ((pool*3)+4)) & 0x4 ) printf("INT: Pointer read from the stack in the L2C does not have the FPA owner ship bit set\n");

    /* 
     * parse the FPA free list and
     * check for issues
     */
    //cvm_common_parsefpa_free_list(pool);


    /* populate the offet array */
    cur_offset = 0;
    pool_info = cvm_common_fpa_info[pool];

    for (i=0; i<pool_info.count; i++)
    {
        offset[i] = cur_offset;
        cur_offset += ( (pool_info.entry[i].end_addr - pool_info.entry[i].start_addr+1)/pool_info.entry[0].element_size);
    }

    no_of_buffers = cur_offset;

    buf_array = (uint16_t*)CVM_COMMON_MALLOC_TSAFE(cvm_common_arenas, no_of_buffers*sizeof(uint16_t) );
    saved_buffers = (uint64_t*)CVM_COMMON_MALLOC_TSAFE(cvm_common_arenas, no_of_buffers*8);
    if ( (buf_array == NULL) || (saved_buffers == NULL) )
    {
        printf("Unable to allocate memory to hold %lld entries\n", CVM_COMMON_UCAST64(no_of_buffers));

	if (buf_array)       CVM_COMMON_FREE_TSAFE(buf_array);
	if (saved_buffers)   CVM_COMMON_FREE_TSAFE(saved_buffers);

	return;
    }

    memset( (void*)buf_array, 0x0, no_of_buffers*sizeof(uint16_t));

    /* setup the array to check buffer boundries */
    k = 0;
    for (i=0; i<pool_info.count; i++)
    {
       uint32_t buf_count_in_this_seg = ( (pool_info.entry[i].end_addr - pool_info.entry[i].start_addr+1)/pool_info.entry[0].element_size );
	uint64_t ptr_addr = (uint64_t)pool_info.entry[i].start_addr;

	for (j=0; j<buf_count_in_this_seg; j++)
	{
	    buf_array[k] = (ptr_addr & (0x7fffull) );
	    k++;
            ptr_addr += (uint64_t)(pool_info.entry[i].element_size);

	}
    }


    /* analyze this pool */
    printf("Analyzing %lld buffers. Wait...\n", CAST64(CVM_COMMON_FPA_AVAIL_COUNT(pool)) );

    /* Get all the buffers from POW */
    while(1)
    {
        cvmx_buf_ptr_t ptr;

        swp = (cvmx_wqe_t *)cvmx_pow_work_request_sync(0);
        if (swp == NULL)
        {
	    break;
	}

        if(swp->word2.s.bufs)
	{
	    ptr = swp->packet_ptr;
            cvmx_fpa_free((void *)(cvmx_phys_to_ptr((ptr.s.addr & ~127) - 128*ptr.s.back)), ptr.s.pool, 0);
	}

        cvmx_fpa_free( (void*)swp, CVMX_FPA_WQE_POOL, 0);
    }

    cur_index = 0;
    while(1)
    {
        buffer_addr = CAST64(cvmx_fpa_alloc(pool));
	if (buffer_addr == 0x0)
	{
	    if ( (CVM_COMMON_FPA_AVAIL_COUNT(pool)) != 0x0)
	    {
	        invalid_buffers++;
	        printf("analyze fpa : Invalid buffer address (buffer=0x%llX, index=%d, no. of buffers still in pool=%lld)\n", 
		       CAST64(buffer_addr), cur_index, CAST64(CVM_COMMON_FPA_AVAIL_COUNT(pool)) );
	        continue;
	    }
	    else
	    {
	        break;
	    }
	}

	buffer_analyzed++;

        if (!(CVM_COMMON_CHECK_FPA_POOL(pool, buffer_addr)))
        {
	    invalid_buffers++;
	    printf("analyze fpa : Invalid buffer address (buffer=0x%llX, index=%d)\n", CAST64(buffer_addr), cur_index);
	    continue;
        }

	/* find duplicate entries */
        segment = CVM_COMMON_WHICH_FPA_POOL_SEGMENT((uint64_t)buffer_addr, (uint32_t)pool);
        entry = cvm_common_fpa_info[pool].entry[segment];
        index = (uint32_t)(buffer_addr - entry.start_addr) / (uint32_t)entry.element_size;
	state = buf_array[index + offset[segment] ];

	if ( state & 0x8000 )
	{
	    duplicate_buffers++;
            printf("analyze fpa : Duplicate entries for buffer 0x%llX (previous entry is at %d)\n", CAST64(buffer_addr), cur_index);
	    continue;
	}
	else
	{
	    state |= 0x8000;
	}

	/* remember the state */
	buf_array[index + offset[segment] ] = state;


	/* check the buffer boundry */
	if (  (state & 0x7fff) != (buffer_addr & 0x7fff) )
	{
	    printf("analyze fpa : Un-aligned buffer 0x%llX at index %d\n", CAST64(buffer_addr), cur_index);
	    unaligned_buffers++;
	    //continue;
	}

	saved_buffers[cur_index] = buffer_addr;
	cur_index++;
    }

    for (i=0; i<cur_index; i++)
    {
        cvmx_fpa_free( CASTPTR(void,saved_buffers[i]), pool, 0);
    }

    CVM_COMMON_FREE_TSAFE(buf_array);
    CVM_COMMON_FREE_TSAFE(saved_buffers);

    printf("Buffer analyzation completed.\n");
    printf("    buffers analyzed          = %d\n", buffer_analyzed);
    printf("    duplicate entries         = %d\n", duplicate_buffers);
    printf("    invalid address entreis   = %d\n", invalid_buffers);
    printf("    unaligned address entries = %d\n", unaligned_buffers);
    printf("\n");
}
void cvm_common_dump_fpa(uint32_t pool)
{
    uint16_t *buf_array = NULL;
    uint64_t *saved_buffers = NULL;
    uint32_t no_of_buffers = 0;
    int i = 0,k = 0;
    uint32_t  j = 0;
    int cur_index = 0;
    uint64_t buffer_addr;
    int offset[CVM_COMMON_MAX_FPA_INFO_ENTRIES];
    int cur_offset = 0;
    cvm_common_fpa_info_t pool_info;
    int segment;
    cvm_common_fpa_info_entry_t entry;
    uint32_t index;
    uint16_t state=0;
    cvmx_wqe_t *swp=NULL;
    cvmx_fpa_int_sum_t int_sum;

    /* stats */
    int buffer_analyzed = 0;
    int duplicate_buffers = 0;
    int invalid_buffers = 0;
    int unaligned_buffers = 0;

    if(pool > CVMX_FPA_NUM_POOLS)
    {
        printf("Invalid pool number [%d]\n", pool);
        return;
    }

    /*
     * Check FPA interrupts (may show signs of FPA corruption)
     */
    int_sum.u64 = cvmx_read_csr(CVMX_FPA_INT_SUM);

    if (int_sum.s.fed0_sbe) printf("INT: Single Bit Error detected in FPF0\n");
    if (int_sum.s.fed0_dbe) printf("INT: Double Bit Error detected in FPF0\n");
    if (int_sum.s.fed1_sbe) printf("INT: Single Bit Error detected in FPF1\n");
    if (int_sum.s.fed1_dbe) printf("INT: Double Bit Error detected in FPF1\n");

    if ( (int_sum.u64 >> ((pool*3)+4)) & 0x1 ) printf("INT: Page count available goes negative\n");
    if ( (int_sum.u64 >> ((pool*3)+4)) & 0x2 ) printf("INT: Count available is greater than pointers present in the FPA\n");
    if ( (int_sum.u64 >> ((pool*3)+4)) & 0x4 ) printf("INT: Pointer read from the stack in the L2C does not have the FPA owner ship bit set\n");

    /* 
     * parse the FPA free list and
     * check for issues
     */
    //cvm_common_parsefpa_free_list(pool);


    /* populate the offet array */
    cur_offset = 0;
    pool_info = cvm_common_fpa_info[pool];

    for (i=0; i<pool_info.count; i++)
    {
        offset[i] = cur_offset;
        cur_offset += ( (pool_info.entry[i].end_addr - pool_info.entry[i].start_addr+1)/pool_info.entry[0].element_size);
    }

    no_of_buffers = cur_offset;

    buf_array = (uint16_t*)CVM_COMMON_MALLOC_TSAFE(cvm_common_arenas, no_of_buffers*sizeof(uint16_t) );
    saved_buffers = (uint64_t*)CVM_COMMON_MALLOC_TSAFE(cvm_common_arenas, no_of_buffers*8);
    if ( (buf_array == NULL) || (saved_buffers == NULL) )
    {
        printf("Unable to allocate memory to hold %lld entries\n", CVM_COMMON_UCAST64(no_of_buffers));

	if (buf_array)       CVM_COMMON_FREE_TSAFE(buf_array);
	if (saved_buffers)   CVM_COMMON_FREE_TSAFE(saved_buffers);

	return;
    }

    memset( (void*)buf_array, 0x0, no_of_buffers*sizeof(uint16_t));

    /* setup the array to check buffer boundries */
    k = 0;
    for (i=0; i<pool_info.count; i++)
    {
       uint32_t buf_count_in_this_seg = ( (pool_info.entry[i].end_addr - pool_info.entry[i].start_addr+1)/pool_info.entry[0].element_size );
	uint64_t ptr_addr = (uint64_t)pool_info.entry[i].start_addr;

	for (j=0; j<buf_count_in_this_seg; j++)
	{
	    buf_array[k] = (ptr_addr & (0x7fffull) );
	    k++;
            ptr_addr += (uint64_t)(pool_info.entry[i].element_size);

	}
    }


    /* analyze this pool */
    printf("Analyzing %lld buffers. Wait...\n", CAST64(CVM_COMMON_FPA_AVAIL_COUNT(pool)) );

    /* Get all the buffers from POW */
    while(1)
    {
        cvmx_buf_ptr_t ptr;

        swp = (cvmx_wqe_t *)cvmx_pow_work_request_sync(0);
        if (swp == NULL)
        {
	    break;
	}

        if(swp->word2.s.bufs)
	{
	    ptr = swp->packet_ptr;
            cvmx_fpa_free((void *)(cvmx_phys_to_ptr((ptr.s.addr & ~127) - 128*ptr.s.back)), ptr.s.pool, 0);
	}

        cvmx_fpa_free( (void*)swp, CVMX_FPA_WQE_POOL, 0);
    }

    cur_index = 0;
    while(1)
    {
        buffer_addr = CAST64(cvmx_fpa_alloc(pool));
        printf("fpa buffer vaddress = 0x%llX, paddress = 0x%lX\n", CAST64(buffer_addr), (uint64_t)cvmx_ptr_to_phys((void *)buffer_addr));
        
	if (buffer_addr == 0x0)
	{
	    if ( (CVM_COMMON_FPA_AVAIL_COUNT(pool)) != 0x0)
	    {
	        invalid_buffers++;
	        printf("analyze fpa : Invalid buffer address (buffer=0x%llX, index=%d, no. of buffers still in pool=%lld)\n", 
		       CAST64(buffer_addr), cur_index, CAST64(CVM_COMMON_FPA_AVAIL_COUNT(pool)) );
	        continue;
	    }
	    else
	    {
	        break;
	    }
	}

	buffer_analyzed++;

        if (!(CVM_COMMON_CHECK_FPA_POOL(pool, buffer_addr)))
        {
	    invalid_buffers++;
	    printf("analyze fpa : Invalid buffer address (buffer=0x%llX, index=%d)\n", CAST64(buffer_addr), cur_index);
	    continue;
        }

	/* find duplicate entries */
        segment = CVM_COMMON_WHICH_FPA_POOL_SEGMENT((uint64_t)buffer_addr, (uint32_t)pool);
        entry = cvm_common_fpa_info[pool].entry[segment];
        index = (uint32_t)(buffer_addr - entry.start_addr) / (uint32_t)entry.element_size;
	state = buf_array[index + offset[segment] ];

	if ( state & 0x8000 )
	{
	    duplicate_buffers++;
            printf("analyze fpa : Duplicate entries for buffer 0x%llX (previous entry is at %d)\n", CAST64(buffer_addr), cur_index);
	    continue;
	}
	else
	{
	    state |= 0x8000;
	}

	/* remember the state */
	buf_array[index + offset[segment] ] = state;


	/* check the buffer boundry */
	if (  (state & 0x7fff) != (buffer_addr & 0x7fff) )
	{
	    printf("analyze fpa : Un-aligned buffer 0x%llX at index %d\n", CAST64(buffer_addr), cur_index);
	    unaligned_buffers++;
	    //continue;
	}

	saved_buffers[cur_index] = buffer_addr;
	cur_index++;
    }

    for (i=0; i<cur_index; i++)
    {
        cvmx_fpa_free( CASTPTR(void,saved_buffers[i]), pool, 0);
    }

    CVM_COMMON_FREE_TSAFE(buf_array);
    CVM_COMMON_FREE_TSAFE(saved_buffers);

    printf("Buffer analyzation completed.\n");
    printf("    buffers analyzed          = %d\n", buffer_analyzed);
    printf("    duplicate entries         = %d\n", duplicate_buffers);
    printf("    invalid address entreis   = %d\n", invalid_buffers);
    printf("    unaligned address entries = %d\n", unaligned_buffers);
    printf("\n");
}

void cvm_common_read_csr(uint64_t addr)
{
    uint64_t val = cvmx_read_csr(CVMX_ADD_IO_SEG(addr));

    printf("\n\tRegister = 0x%llX\n", CAST64(addr));
    printf("\tValue = %02llX %02llX %02llX %02llX - %02llX %02llX %02llX %02llX\n\n",
	   CAST64((uint32_t)( (val >> 56) & 0xff)),
	   CAST64((uint32_t)( (val >> 48) & 0xff)),
	   CAST64((uint32_t)( (val >> 40) & 0xff)),
	   CAST64((uint32_t)( (val >> 32) & 0xff)),
	   CAST64((uint32_t)( (val >> 24) & 0xff)),
	   CAST64((uint32_t)( (val >> 16) & 0xff)),
	   CAST64((uint32_t)( (val >> 8) & 0xff)),
	   CAST64((uint32_t)( (val >> 0) & 0xff))
	   );
}

void cvm_common_write_csr(uint64_t addr, uint64_t val)
{
    cvmx_write_csr(CVMX_ADD_IO_SEG(addr), val);
    printf("\n\tValue 0x%llX written to register = 0x%llX\n\n", CAST64(val), CAST64(addr));
}

void cvm_common_dump_pko(void)
{
    int i = 0;

    printf("\n");

    for (i=0; i<36; i++)
    {
        /* get the PKO stats for port 0 and PCI ports */
        cvmx_write_csr(CVMX_PKO_REG_READ_IDX, i);

        printf("\tPort %d : packets = %llu  ", i, CAST64(cvmx_read_csr(CVMX_PKO_MEM_COUNT0)) );
        printf("bytes   = %llu  ", CAST64(cvmx_read_csr(CVMX_PKO_MEM_COUNT1)) );
        printf("PKO_MEM_DBG9 = 0x%llx\n", CAST64(cvmx_read_csr(CVMX_PKO_MEM_DEBUG9)) );
    }

    printf("PKO_REG_ERROR  = 0x%llX\n", CAST64(cvmx_read_csr(CVMX_PKO_REG_ERROR)) );
    printf("PKO_REG_DEBUG0 = 0x%llX\n", CAST64(cvmx_read_csr(CVMX_PKO_REG_DEBUG0)));

    return;
}


/**
 * cvm_common_packet_copy
 * - Makes a copy of the packet_buffer 
 *   (including chained list of packet buffers)
 * - allocates memory for packet_buffers
 *
 * Return value:
 * - ( 0) - success
 * - (-1) - failure (memory cannot be allocated)
 *        - frees all the allocated memory for packet buffers
 *        - sets ptr_to to NULL
 */
inline int cvm_common_packet_copy(cvmx_buf_ptr_t *ptr_to, cvmx_buf_ptr_t *ptr_from, int num_bufs, int total_len)
{
    int retval = 0;
    uint64_t ptr_from_start_of_buffer;
    uint64_t bytes_delta;

    if (cvmx_unlikely(num_bufs > 1))
    {
        int             nn;
        uint64_t        addr_new;
        cvmx_buf_ptr_t  ptr_new;
        cvmx_buf_ptr_t  ptr_prev;
        int             remaining_bytes;

        ptr_new.u64     = 0;
        ptr_prev.u64    = 0;
        remaining_bytes = total_len;
        nn              = 1;

        while (nn <= num_bufs)
        {
            addr_new = CAST64(cvmx_ptr_to_phys(cvm_common_alloc_fpa_buffer_sync(ptr_from->s.pool)));
            if (addr_new == 0x0)
            {
                FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN,FASTFWD_COMMON_DBG_LVL_WARNING, 
                        "%s: addr_new 0x%llx, cvm_common_alloc_fpa_buffer_sync FAILED, pool %d\n",
                        __FUNCTION__, CAST64(addr_new), ptr_from->s.pool);

                if (nn > 0)
                {
                    cvmx_wqe_t swp_tmp;
                    cvmx_wqe_t *swp_tmp_ptr = &swp_tmp;

                    swp_tmp_ptr->packet_ptr   = *ptr_to;
                    swp_tmp_ptr->word2.s.bufs = (nn - 1);

                    cvmx_helper_free_packet_data(swp_tmp_ptr);
                }

                ptr_to = NULL;
                return -1;
            }

            ptr_new.u64    = 0;
            ptr_new.s.pool = ptr_from->s.pool;
            ptr_new.s.back = ptr_from->s.back;
            ptr_new.s.i    = ptr_from->s.i;

            if (nn == num_bufs)  // last buffer
            {
                ptr_new.s.size = remaining_bytes;
            }
            else
            {
                ptr_new.s.size = ptr_from->s.size;
                remaining_bytes = remaining_bytes - ptr_from->s.size;
            }

            ptr_from_start_of_buffer = ((ptr_from->s.addr >> 7) - ptr_from->s.back) << 7;
            bytes_delta              = ptr_from->s.addr - ptr_from_start_of_buffer;

            memcpy(cvmx_phys_to_ptr(addr_new), cvmx_phys_to_ptr(ptr_from_start_of_buffer), ptr_new.s.size + bytes_delta);
            ptr_new.s.addr = addr_new + bytes_delta;
            ptr_new.s.back = CVM_COMMON_CALC_BACK(ptr_new);

            if (nn == 1)
                *ptr_to = ptr_new;
            else
                *((cvmx_buf_ptr_t *)(cvmx_phys_to_ptr(ptr_prev.s.addr - 8))) = ptr_new;

            ptr_prev = ptr_new;
            ptr_from = (cvmx_buf_ptr_t *)(cvmx_phys_to_ptr(ptr_from->s.addr - 8));
            ++nn;
        }

        CVMX_SYNCWS;
    }
    else
    {
        cvmx_buf_ptr_t ptr_new;
        uint64_t       addr_new;

        addr_new = CAST64(cvmx_ptr_to_phys(cvm_common_alloc_fpa_buffer_sync(ptr_from->s.pool)));
        if (addr_new == 0x0)
        {
            FASTFWD_COMMON_DBG_MSG(FASTFWD_COMMON_MOUDLE_MAIN,FASTFWD_COMMON_DBG_LVL_WARNING, 
                    "%s: addr_new 0x%llx, cvm_common_alloc_fpa_buffer_sync FAILED, pool %d\n",
                    __FUNCTION__, CAST64(addr_new), ptr_from->s.pool);
            return -1;
        }

        ptr_new.u64    = 0;
        ptr_new.s.pool = ptr_from->s.pool;
        ptr_new.s.back = ptr_from->s.back;
        ptr_new.s.i    = ptr_from->s.i;

        ptr_new.s.size           = total_len;
        ptr_from_start_of_buffer = ((ptr_from->s.addr >> 7) - ptr_from->s.back) << 7;
        bytes_delta              = ptr_from->s.addr - ptr_from_start_of_buffer;

        memcpy(cvmx_phys_to_ptr(addr_new), cvmx_phys_to_ptr(ptr_from_start_of_buffer), ptr_new.s.size + bytes_delta);
        ptr_new.s.addr = addr_new + bytes_delta;
        ptr_new.s.back = CVM_COMMON_CALC_BACK(ptr_new);

        *ptr_to = ptr_new;
    }

    return retval;
}


#define CVM_COMMON_PACKET_PTR_PRINT(packet_ptr) \
    printf("packet_ptr u64 0x%016llx, i %d, back %d, pool %d, size %4d, addr 0x%08llx\n", \
            CAST64(packet_ptr->u64), \
            packet_ptr->s.i,     \
            packet_ptr->s.back,  \
            packet_ptr->s.pool,  \
            packet_ptr->s.size,  \
            CAST64(packet_ptr->s.addr)); \

