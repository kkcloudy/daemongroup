/***********************license start***************
 * Copyright (c) 2003-2010  Cavium Networks (support@cavium.com). All rights
 * reserved.
 *
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.

 *   * Neither the name of Cavium Networks nor the names of
 *     its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written
 *     permission.

 * This Software, including technical data, may be subject to U.S. export  control
 * laws, including the U.S. Export Administration Act and its  associated
 * regulations, and may be subject to export or import  regulations in other
 * countries.

 * TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND CAVIUM  NETWORKS MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE, INCLUDING ITS CONDITION, ITS CONFORMITY TO ANY REPRESENTATION OR
 * DESCRIPTION, OR THE EXISTENCE OF ANY LATENT OR PATENT DEFECTS, AND CAVIUM
 * SPECIFICALLY DISCLAIMS ALL IMPLIED (IF ANY) WARRANTIES OF TITLE,
 * MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF
 * VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. THE ENTIRE  RISK ARISING OUT OF USE OR
 * PERFORMANCE OF THE SOFTWARE LIES WITH YOU.
 ***********************license end**************************************/







/**
 * @file
 *
 * Helper Functions for the Configuration Framework
 *
 * <hr>$Revision: 0 $<hr>
 */
#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
#include <linux/module.h>
#include <asm/octeon/cvmx.h>
#include <asm/octeon/cvmx-helper.h>
#include <asm/octeon/cvmx-helper-util.h>
#include <asm/octeon/cvmx-helper-cfg.h>
#include <asm/octeon/cvmx-helper-ilk.h>
#include <asm/octeon/cvmx-ilk.h>
#include <asm/octeon/cvmx-config.h>
#else
#include "cvmx.h"
#include "cvmx-bootmem.h"
#include "cvmx-helper.h"
#include "cvmx-helper-util.h"
#include "cvmx-helper-cfg.h"
#include "cvmx-ilk.h"
#include "cvmx-helper-ilk.h"
#include "cvmx-power-throttle.h"
#include "cvmx-config.h"
#include "executive-config.h"
#endif

#if defined(min)
#else
#define min( a, b ) ( ( a ) < ( b ) ) ? ( a ) : ( b )
#endif

/* QLM index */
#define CVMX_HELPER_CFG_QLM0		0
#define CVMX_HELPER_CFG_QLM1		1
#define CVMX_HELPER_CFG_QLM2		2
#define CVMX_HELPER_CFG_QLM3		3
#define CVMX_HELPER_CFG_QLM4		4

/* MIO_QLM(0..4)_CFG[QLM_CFG] */
#define CVMX_HELPER_CFG_QLM_CFG_PCIE	0
#define CVMX_HELPER_CFG_QLM_CFG_ILK	1
#define CVMX_HELPER_CFG_QLM_CFG_SGMII	2
#define CVMX_HELPER_CFG_QLM_CFG_XAUI	3
#define CVMX_HELPER_CFG_QLM_CFG_RXAUI	7
#define CVMX_HELPER_CFG_QLM_CFG_LOOP	8   /* fictitious from here on */
#define CVMX_HELPER_CFG_QLM_CFG_INVALID 9

/* MIO_QLM(0..4)_CFG[QLM_SPD] */
#define CVMX_HELPER_CFG_QLM_SPD_DISABLED	15

/* #define CVMX_HELPER_CFG_DEBUG */

/*
 * QLM constants
 */
struct cvmx_cfg_qlm_param {
	int ccqp_mode;
	int ccqp_nports;
};

/*
 * Per interface parameters
 */
struct cvmx_cfg_iface_param {
	int ccip_nports;
};

/*
 * Per physical port
 */
struct cvmx_cfg_port_param {
	int8_t	ccpp_pknd;
	int8_t	ccpp_bpid;
	int8_t	ccpp_pko_port_base;
	int8_t	ccpp_pko_num_ports;
	uint8_t	ccpp_pko_nqueues;	/*
					 * When the user explicitly
					 * assigns queues,
					 * cvmx_cfg_pko_nqueue_pool[
					 *     ccpp_pko_nqueues ... 
					 *     ccpp_pko_nqueues + 
					 *     ccpp_pko_num_ports - 1]
					 * are the numbers of PKO queues
					 * assigned to the PKO ports for
					 * this physical port.
					 */
};

/*
 * Per pko_port
 */
struct cvmx_cfg_pko_port_param {
	int16_t	ccppp_queue_base;
	int16_t	ccppp_num_queues;
};

/*
 * A map from pko_port to
 *     interface,
 *     index, and
 *     pko engine id
 */
struct cvmx_cfg_pko_port_map {
	int16_t ccppl_interface;
	int16_t ccppl_index;
	int16_t ccppl_eid;
};

/*
 * This is for looking up pko_base_port and pko_nport for ipd_port
 */
struct cvmx_cfg_pko_port_pair {
	int8_t ccppp_base_port;
	int8_t ccppp_nports;
};

/*
 * Indexed by CVMX_HELPER_CFG_QLM_CFG_XXX
 */
static CVMX_SHARED struct cvmx_cfg_qlm_param cvmx_cfg_qlm_params[] = {
    /* 0 */	{CVMX_HELPER_INTERFACE_MODE_NPI, 32},
    /* 1 */	{CVMX_HELPER_INTERFACE_MODE_ILK, 256},
    /* 2 */	{CVMX_HELPER_INTERFACE_MODE_SGMII, 4},
    /* 3 */	{CVMX_HELPER_INTERFACE_MODE_XAUI, 1},
    /* 4 */	{CVMX_HELPER_INTERFACE_MODE_DISABLED, -1},
    /* 5 */	{CVMX_HELPER_INTERFACE_MODE_DISABLED, -1},
    /* 6 */	{CVMX_HELPER_INTERFACE_MODE_DISABLED, -1},
    /* 7 */	{CVMX_HELPER_INTERFACE_MODE_RXAUI, 1},
    /* 8 loop--not to be mixed up with the loopback interface number in o68. */
    		{CVMX_HELPER_INTERFACE_MODE_LOOP, 8},
    /* 9 invalid */
    		{CVMX_HELPER_INTERFACE_MODE_DISABLED, -1}};

/*
 * Indexed by the interface number
 */
static CVMX_SHARED struct cvmx_cfg_iface_param cvmx_cfg_iface_params
    [CVMX_HELPER_CFG_MAX_IFACE] =
    {[0 ... CVMX_HELPER_CFG_MAX_IFACE -1] =
        {0}};

static CVMX_SHARED struct cvmx_cfg_port_param cvmx_cfg_port
    [CVMX_HELPER_CFG_MAX_IFACE][CVMX_HELPER_CFG_MAX_PORT_PER_IFACE] =
    {[0 ... CVMX_HELPER_CFG_MAX_IFACE - 1] =
        {[0 ... CVMX_HELPER_CFG_MAX_PORT_PER_IFACE - 1] =
            {CVMX_HELPER_CFG_INVALID_VALUE,
    	     CVMX_HELPER_CFG_INVALID_VALUE,
    	     CVMX_HELPER_CFG_INVALID_VALUE,
    	     CVMX_HELPER_CFG_INVALID_VALUE,
    	     CVMX_HELPER_CFG_INVALID_VALUE}}};

/*
 * Indexed by the pko_port number
 */
static CVMX_SHARED struct cvmx_cfg_pko_port_param cvmx_cfg_pko_port
    [CVMX_HELPER_CFG_MAX_PKO_PORT] =
    {[0 ... CVMX_HELPER_CFG_MAX_PKO_PORT - 1] =
        {CVMX_HELPER_CFG_INVALID_VALUE,
	 CVMX_HELPER_CFG_INVALID_VALUE}};

static CVMX_SHARED struct cvmx_cfg_pko_port_map cvmx_cfg_pko_port_map
    [CVMX_HELPER_CFG_MAX_PKO_PORT] = 
        {[0 ... CVMX_HELPER_CFG_MAX_PKO_PORT - 1] =
            {CVMX_HELPER_CFG_INVALID_VALUE,
	     CVMX_HELPER_CFG_INVALID_VALUE,
             CVMX_HELPER_CFG_INVALID_VALUE}};

#ifdef CVMX_ENABLE_PKO_FUNCTIONS
/*
 * This array assists translation from ipd_port to pko_port.
 * The ``16'' is the rounded value for the 3rd 4-bit value of
 * ipd_port, used to differentiate ``interfaces.''
 */
static CVMX_SHARED struct cvmx_cfg_pko_port_pair ipd2pko_port_cache[16]
    [CVMX_HELPER_CFG_MAX_PORT_PER_IFACE] =
    {[0 ... 15] = 
        {[0 ... CVMX_HELPER_CFG_MAX_PORT_PER_IFACE - 1] =
	    {CVMX_HELPER_CFG_INVALID_VALUE,
	     CVMX_HELPER_CFG_INVALID_VALUE}}};

#ifdef CVMX_USER_DEFINED_HELPER_CONFIG_INIT

static CVMX_SHARED int cvmx_cfg_default_pko_nqueues = 1;

/*
 * A pool for holding the pko_nqueues for the pko_ports assigned to a
 * physical port.
 */
static CVMX_SHARED uint8_t cvmx_cfg_pko_nqueue_pool
    [CVMX_HELPER_CFG_MAX_PKO_QUEUES] = 
    {[0 ... CVMX_HELPER_CFG_MAX_PKO_QUEUES - 1] = 1};

#endif
#endif

/*
 * Options
 *
 * Each array-elem's intial value is also the option's default value.
 */
static CVMX_SHARED uint64_t cvmx_cfg_opts[CVMX_HELPER_CFG_OPT_MAX] =
    {[0 ... CVMX_HELPER_CFG_OPT_MAX - 1] = 1};

/*
 * MISC
 */
static CVMX_SHARED int cvmx_cfg_max_pko_engines; /* # of PKO DMA engines 
						    allocated */

/*
 * Query the QLM to get MIO_QLM(offset)_CFG[QLM_CFG].
 *
 * @param offset the index of QLM
 * @return the content of the [QLM_CFG] field
 */
static int cvmx_helper_cfg_qlm_cfg(int offset)
{
    union cvmx_mio_qlmx_cfg cfg;

    cfg.u64 = cvmx_read_csr(CVMX_MIO_QLMX_CFG(offset));

    return cfg.s.qlm_cfg;
}

/*
 * Return the [QLM_SPD] for the QLM
 *
 * @param offset the index of QLM
 * @return the content of the [QLM_SPD] field
 */
static int cvmx_helper_cfg_qlm_spd(int offset)
{
    union cvmx_mio_qlmx_cfg cfg;

    cfg.u64 = cvmx_read_csr(CVMX_MIO_QLMX_CFG(offset));

    return cfg.s.qlm_spd;
}

/*
 * Return MIO_QLM(0..4)_CFG[QLM_CFG] for an interface.
 *
 * @param interface the interface number
 * @return the [QLM_CFG] for the interface
 */
static int cvmx_helper_dcfg_iface_qlm_cfg(int interface)
{
    int qlm_spd;
    int qlm_cfg;

#define GET_QLM_PARAMS(qlm_offset)					\
	do {								\
		qlm_spd = cvmx_helper_cfg_qlm_spd(qlm_offset);		\
		if (qlm_spd == CVMX_HELPER_CFG_QLM_SPD_DISABLED)	\
			return CVMX_HELPER_CFG_QLM_CFG_INVALID;		\
		qlm_cfg = cvmx_helper_cfg_qlm_cfg(qlm_offset);		\
	} while (0)

    switch(interface)
    {
    case 0:
        GET_QLM_PARAMS(CVMX_HELPER_CFG_QLM0);
	break;
    case 1:
        GET_QLM_PARAMS(CVMX_HELPER_CFG_QLM0);
	if (qlm_cfg != CVMX_HELPER_CFG_QLM_CFG_RXAUI)
	    return CVMX_HELPER_CFG_QLM_CFG_INVALID;
	break;
    case 2:
    case 3:
    case 4:
        GET_QLM_PARAMS(interface);
	if (qlm_cfg != CVMX_HELPER_CFG_QLM_CFG_XAUI &&
	    qlm_cfg != CVMX_HELPER_CFG_QLM_CFG_SGMII)
	    return CVMX_HELPER_CFG_QLM_CFG_INVALID;
        break;
    case 5:
        GET_QLM_PARAMS(CVMX_HELPER_CFG_QLM1);
	if (qlm_cfg != CVMX_HELPER_CFG_QLM_CFG_ILK)
	    return CVMX_HELPER_CFG_QLM_CFG_INVALID;
	break;
    case 6:
        GET_QLM_PARAMS(CVMX_HELPER_CFG_QLM2);
	if (qlm_cfg != CVMX_HELPER_CFG_QLM_CFG_ILK)
	    return CVMX_HELPER_CFG_QLM_CFG_INVALID;
	break;
    case 7:
        GET_QLM_PARAMS(CVMX_HELPER_CFG_QLM3);
	if (qlm_cfg != CVMX_HELPER_CFG_QLM_CFG_PCIE)
	{
            GET_QLM_PARAMS(CVMX_HELPER_CFG_QLM1);
	    if (qlm_cfg != CVMX_HELPER_CFG_QLM_CFG_PCIE)
	        return CVMX_HELPER_CFG_QLM_CFG_INVALID;
	}
	break;
    case 8:
	qlm_cfg = CVMX_HELPER_CFG_QLM_CFG_LOOP;
	break;
    default:
    	qlm_cfg = CVMX_HELPER_CFG_QLM_CFG_INVALID;
        break;
    }

    return qlm_cfg;
}

#ifdef CVMX_ENABLE_HELPER_FUNCTIONS
/*
 * Retrieve the ILK-module-default nports for an ilk interface
 *
 * @param ilk_interface the ILK interface number
 *
 * @return the default number of channels for the interface defined by the
 * driver.
 */
static int cvmx_helper_cfg_ilk_nports(int ilk_interface)
{
    unsigned char *pchans, num_chans;
    int val;

    val = cvmx_ilk_get_chan_info ((ilk_interface - CVMX_ILK_GBL_BASE), &pchans,
        &num_chans);

    cvmx_helper_cfg_assert(val == 0);

    return num_chans;
}

/*
 * Return the QLM constant nports for an interface
 *
 * @param interface is the interface number
 * @return the number of ports as seen by the QLM.
 */
static int cvmx_helper_cfg_qlm_nports(int interface)
{
    int qlm_cfg;

    qlm_cfg = cvmx_helper_dcfg_iface_qlm_cfg(interface);

    cvmx_helper_cfg_assert(qlm_cfg >= 0);
    cvmx_helper_cfg_assert(qlm_cfg < 10);
    return cvmx_cfg_qlm_params[qlm_cfg].ccqp_nports;
}
#endif

int __cvmx_helper_cfg_num_ports(int interface)
{
    return cvmx_cfg_iface_params[interface].ccip_nports;
}

int __cvmx_helper_cfg_mode(int interface)
{
    int qlm_cfg;

    qlm_cfg = cvmx_helper_dcfg_iface_qlm_cfg(interface);
    cvmx_helper_cfg_assert(qlm_cfg >= 0);
    cvmx_helper_cfg_assert(qlm_cfg < 10);

    return cvmx_cfg_qlm_params[qlm_cfg].ccqp_mode;
}

int __cvmx_helper_cfg_pknd(int interface, int index)
{
    return cvmx_cfg_port[interface][index].ccpp_pknd;
}

int __cvmx_helper_cfg_bpid(int interface, int index)
{
    return cvmx_cfg_port[interface][index].ccpp_bpid;
}

int __cvmx_helper_cfg_pko_port_base(int interface, int index)
{
    return cvmx_cfg_port[interface][index].ccpp_pko_port_base;
}

int __cvmx_helper_cfg_pko_port_num(int interface, int index)
{
    return cvmx_cfg_port[interface][index].ccpp_pko_num_ports;
}

int __cvmx_helper_cfg_pko_queue_num(int pko_port)
{
    return cvmx_cfg_pko_port[pko_port].ccppp_num_queues;
}

int __cvmx_helper_cfg_pko_queue_base(int pko_port)
{
    return cvmx_cfg_pko_port[pko_port].ccppp_queue_base;
}

int __cvmx_helper_cfg_pko_max_queue(void)
{
    int i;

    i = CVMX_HELPER_CFG_MAX_PKO_PORT - 1;

    while (i >= 0)
    {
        if (cvmx_cfg_pko_port[i].ccppp_queue_base !=
	    CVMX_HELPER_CFG_INVALID_VALUE)
	{
	    cvmx_helper_cfg_assert(cvmx_cfg_pko_port[i].ccppp_num_queues > 0);
	    return (cvmx_cfg_pko_port[i].ccppp_queue_base +
	        cvmx_cfg_pko_port[i].ccppp_num_queues);
	}
	i --;
    }

    cvmx_helper_cfg_assert(0); /* shouldn't get here */
    
    return 0;
}

int __cvmx_helper_cfg_pko_max_engine(void)
{
    return cvmx_cfg_max_pko_engines;
}

int cvmx_helper_cfg_opt_set(cvmx_helper_cfg_option_t opt, uint64_t val)
{
    if (opt >= CVMX_HELPER_CFG_OPT_MAX)
        return -1;
    
    cvmx_cfg_opts[opt] = val;

    return 0;
}

uint64_t cvmx_helper_cfg_opt_get(cvmx_helper_cfg_option_t opt)
{
    if (opt >= CVMX_HELPER_CFG_OPT_MAX)
        return (uint64_t)CVMX_HELPER_CFG_INVALID_VALUE;

    return cvmx_cfg_opts[opt];
}

#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
EXPORT_SYMBOL(__cvmx_helper_cfg_num_ports);
EXPORT_SYMBOL(__cvmx_helper_cfg_mode);
EXPORT_SYMBOL(__cvmx_helper_cfg_init);
EXPORT_SYMBOL(__cvmx_helper_cfg_pknd);
EXPORT_SYMBOL(__cvmx_helper_cfg_bpid);
EXPORT_SYMBOL(__cvmx_helper_cfg_pko_port_base);
EXPORT_SYMBOL(__cvmx_helper_cfg_pko_port_num);
EXPORT_SYMBOL(__cvmx_helper_cfg_pko_queue_base);
EXPORT_SYMBOL(__cvmx_helper_cfg_pko_queue_num);
EXPORT_SYMBOL(__cvmx_helper_cfg_pko_max_queue);
EXPORT_SYMBOL(__cvmx_helper_cfg_pko_port_interface);
EXPORT_SYMBOL(__cvmx_helper_cfg_pko_port_index);
EXPORT_SYMBOL(__cvmx_helper_cfg_pko_port_eid);
EXPORT_SYMBOL(__cvmx_helper_cfg_pko_max_engine);
EXPORT_SYMBOL(cvmx_helper_cfg_opt_get);
EXPORT_SYMBOL(cvmx_helper_cfg_opt_set);
EXPORT_SYMBOL(cvmx_helper_cfg_ipd2pko_port_base);
EXPORT_SYMBOL(cvmx_helper_cfg_ipd2pko_port_num);
#endif

#ifdef CVMX_HELPER_CFG_DEBUG
void cvmx_helper_cfg_show_cfg(void)
{
    int i, j;

    for (i = 0; i < CVMX_HELPER_CFG_MAX_IFACE; i++)
    {
	cvmx_dprintf(
	    "cvmx_helper_cfg_show_cfg: interface%d mode %10s nports%4d\n", i,
	    cvmx_helper_interface_mode_to_string(__cvmx_helper_cfg_mode(i)),
	    __cvmx_helper_cfg_num_ports(i));

	for (j = 0; j < __cvmx_helper_cfg_num_ports(i); j++)
	{
	    cvmx_dprintf("\tpknd[%i][%d]%d", i, j,
	        __cvmx_helper_cfg_pknd(i, j));
	    cvmx_dprintf(" pko_port_base[%i][%d]%d", i, j,
	        __cvmx_helper_cfg_pko_port_base(i, j));
	    cvmx_dprintf(" pko_port_num[%i][%d]%d\n", i, j,
	        __cvmx_helper_cfg_pko_port_num(i, j));
	}
    }

    for (i = 0; i < CVMX_HELPER_CFG_MAX_PKO_PORT; i++)
    {
	if (__cvmx_helper_cfg_pko_queue_base(i) !=
	    CVMX_HELPER_CFG_INVALID_VALUE)
	{
            cvmx_dprintf("cvmx_helper_cfg_show_cfg: pko_port%d qbase%d nqueues%d "
	        "interface%d index%d\n", i,
		__cvmx_helper_cfg_pko_queue_base(i),
		__cvmx_helper_cfg_pko_queue_num(i),
		__cvmx_helper_cfg_pko_port_interface(i),
		__cvmx_helper_cfg_pko_port_index(i));
        }
    }
}
#endif

#ifdef CVMX_ENABLE_HELPER_FUNCTIONS
/*
 * initialize cvmx_cfg_pko_port_map
 */
static void cvmx_helper_cfg_init_pko_port_map(void)
{
    int i, j, k;
    int pko_eid;
    int pko_port_base, pko_port_max;
    cvmx_helper_interface_mode_t mode;

    /*
     * one pko_eid is allocated to each port except for ILK, NPI, and
     * LOOP. Each of the three has one eid.
     */
    pko_eid = 0;
    for (i = 0; i < CVMX_HELPER_CFG_MAX_IFACE; i++)
    {
	mode = __cvmx_helper_cfg_mode(i);
        for (j = 0; j < __cvmx_helper_cfg_num_ports(i); j++)
	{
	    pko_port_base = cvmx_cfg_port[i][j].ccpp_pko_port_base;
	    pko_port_max = pko_port_base +
	        cvmx_cfg_port[i][j].ccpp_pko_num_ports;
	    cvmx_helper_cfg_assert(pko_port_base !=
	        CVMX_HELPER_CFG_INVALID_VALUE);
	    cvmx_helper_cfg_assert(pko_port_max >= pko_port_base);
	    for (k = pko_port_base; k < pko_port_max; k++)
	    {
	        cvmx_cfg_pko_port_map[k].ccppl_interface = i;
	        cvmx_cfg_pko_port_map[k].ccppl_index = j;
	        cvmx_cfg_pko_port_map[k].ccppl_eid = pko_eid;
	    }

#if 0
	    /*
	     * For a physical port that is not configured a PKO port,
	     * pko_port_base here equals to pko_port_max. In this
	     * case, the physical port does not take a DMA engine.
	     */
	    if (pko_port_base > pko_port_max)
#endif
	        if (!(mode == CVMX_HELPER_INTERFACE_MODE_NPI ||
	            mode == CVMX_HELPER_INTERFACE_MODE_LOOP ||
	            mode == CVMX_HELPER_INTERFACE_MODE_ILK))
	            pko_eid ++;
	}

	if (mode == CVMX_HELPER_INTERFACE_MODE_NPI ||
	    mode == CVMX_HELPER_INTERFACE_MODE_LOOP ||
	    mode == CVMX_HELPER_INTERFACE_MODE_ILK)
	        pko_eid ++;
    }

    /*
     * Legal pko_eids [0, 0x13] should not be exhausted.
     */
    cvmx_helper_cfg_assert(pko_eid <= 0x14);

    cvmx_cfg_max_pko_engines = pko_eid;
}
#endif

int __cvmx_helper_cfg_pko_port_interface(int pko_port)
{
    return cvmx_cfg_pko_port_map[pko_port].ccppl_interface;
}

int __cvmx_helper_cfg_pko_port_index(int pko_port)
{
    return cvmx_cfg_pko_port_map[pko_port].ccppl_index;
}

int __cvmx_helper_cfg_pko_port_eid(int pko_port)
{
    return cvmx_cfg_pko_port_map[pko_port].ccppl_eid;
}

/**
 * Perform common init tasks for all chips.
 * @return 1 for the caller to continue init and 0 otherwise.
 *
 * Note: ``common'' means this function is executed regardless of
 * 	- chip, and
 * 	- CVMX_ENABLE_HELPER_FUNCTIONS.
 *
 * This function decides based on these conditions if the
 * configuration stage of the init process should continue.
 *
 * This is only meant to be called by __cvmx_helper_cfg_init().
 */
static int __cvmx_helper_cfg_init_common(void)
{
    int val;

#ifndef CVMX_ENABLE_HELPER_FUNCTIONS
    val = 0;
#else
    val = (octeon_has_feature(OCTEON_FEATURE_PKND));
#endif

    return val;
}

#define IPD2PKO_CACHE_Y(ipd_port)	(ipd_port) >> 8
#define IPD2PKO_CACHE_X(ipd_port)	(ipd_port) & 0xff

#ifdef CVMX_ENABLE_PKO_FUNCTIONS
/*
 * ipd_port to pko_port translation cache
 */
static int __cvmx_helper_cfg_init_ipd2pko_cache(void)
{
    int i, j, n;
    int ipd_y, ipd_x, ipd_port;

    for (i = 0; i < CVMX_HELPER_CFG_MAX_IFACE; i++)
    {
	n = __cvmx_helper_cfg_num_ports(i);
    
        for (j = 0; j < n; j++)
	{
	    ipd_port = cvmx_helper_get_ipd_port(i, j);
	    ipd_y = IPD2PKO_CACHE_Y(ipd_port);
	    ipd_x = IPD2PKO_CACHE_X(ipd_port);
	    ipd2pko_port_cache[ipd_y]
	        [(ipd_port & 0x800) ? ((ipd_x >> 4) & 3) : ipd_x] = 
		(struct cvmx_cfg_pko_port_pair)
		{__cvmx_helper_cfg_pko_port_base(i, j),
		 __cvmx_helper_cfg_pko_port_num(i, j)};
	}
    }

    return 0;
}

int cvmx_helper_cfg_ipd2pko_port_base(int ipd_port)
{
	int ipd_y, ipd_x;

        ipd_y = IPD2PKO_CACHE_Y(ipd_port);
	ipd_x = IPD2PKO_CACHE_X(ipd_port);

        return ipd2pko_port_cache[ipd_y]
	    [(ipd_port & 0x800) ? ((ipd_x >> 4) & 3) : ipd_x].ccppp_base_port;
}

int cvmx_helper_cfg_ipd2pko_port_num(int ipd_port)
{
	int ipd_y, ipd_x;

        ipd_y = IPD2PKO_CACHE_Y(ipd_port);
	ipd_x = IPD2PKO_CACHE_X(ipd_port);

        return ipd2pko_port_cache[ipd_y]
	    [(ipd_port & 0x800) ? ((ipd_x >> 4) & 3) : ipd_x].ccppp_nports;
}
#endif

#ifdef CVMX_ENABLE_HELPER_FUNCTIONS
#ifdef CVMX_USER_DEFINED_HELPER_CONFIG_INIT
/**
 * Return the number of queues assigned to this pko_port by user
 *
 * @param pko_port
 * @return the number of queues for this pko_port
 *
 * Note: Called after the pko_port map is set up.
 */
static int __cvmx_ucfg_nqueues(int pko_port)
{
    int interface, index;
    int i, k;

    interface = __cvmx_helper_cfg_pko_port_interface(pko_port);
    index = __cvmx_helper_cfg_pko_port_index(pko_port);

    /*
     * pko_port belongs to no physical port,
     * don't assign a queue to it.
     */
    if (interface == CVMX_HELPER_CFG_INVALID_VALUE ||
        index == CVMX_HELPER_CFG_INVALID_VALUE)
	return 0;

    /*
     * Assign the default number of queues to those pko_ports not
     * assigned explicitly.
     */
    i = cvmx_cfg_port[interface][index].ccpp_pko_nqueues;
    if (i == (uint8_t)CVMX_HELPER_CFG_INVALID_VALUE)
        return cvmx_cfg_default_pko_nqueues;

    /*
     * The user has assigned nqueues to this pko_port,
     * recorded in the pool.
     */
    k = pko_port - cvmx_cfg_port[interface][index].ccpp_pko_port_base;
    cvmx_helper_cfg_assert(k < 
        cvmx_cfg_port[interface][index].ccpp_pko_num_ports);
    return cvmx_cfg_pko_nqueue_pool[i + k];
}

#else

/**
 * Return the number of queues to be assigned to this pko_port
 *
 * @param pko_port
 * @return the number of queues for this pko_port
 *
 * Note: This function exists for backward compatibility.
 * CVMX_PKO_QUEUES_PER_PORT_XXXX defines #queeus per HW port.
 * pko_port is equivalent in pre-o68 SDK.
 */
static int cvmx_helper_cfg_dft_nqueues(int pko_port)
{
    cvmx_helper_interface_mode_t mode;
    int interface;
    int n;

#ifndef CVMX_HELPER_PKO_QUEUES_PER_PORT_INTERFACE0
#define CVMX_HELPER_PKO_QUEUES_PER_PORT_INTERFACE0 1
#endif

#ifndef CVMX_HELPER_PKO_QUEUES_PER_PORT_INTERFACE1
#define CVMX_HELPER_PKO_QUEUES_PER_PORT_INTERFACE1 1
#endif

#ifndef CVMX_HELPER_PKO_QUEUES_PER_PORT_INTERFACE2
#define CVMX_HELPER_PKO_QUEUES_PER_PORT_INTERFACE2 1
#endif

#ifndef CVMX_HELPER_PKO_QUEUES_PER_PORT_INTERFACE3
#define CVMX_HELPER_PKO_QUEUES_PER_PORT_INTERFACE3 1
#endif

#ifndef CVMX_HELPER_PKO_QUEUES_PER_PORT_INTERFACE4
#define CVMX_HELPER_PKO_QUEUES_PER_PORT_INTERFACE4 1
#endif

    n = 1;
    interface = __cvmx_helper_cfg_pko_port_interface(pko_port);
    if (interface == 0)
    {
#ifdef CVMX_PKO_QUEUES_PER_PORT_INTERFACE0
	n = CVMX_PKO_QUEUES_PER_PORT_INTERFACE0;
#endif
    }
    if (interface == 1)
    {
#ifdef CVMX_PKO_QUEUES_PER_PORT_INTERFACE1
	n = CVMX_PKO_QUEUES_PER_PORT_INTERFACE1;
#endif
    }

    if (interface == 2)
    {
#ifdef CVMX_PKO_QUEUES_PER_PORT_INTERFACE2
	n = CVMX_PKO_QUEUES_PER_PORT_INTERFACE2;
#endif
    }
    if (interface == 3)
    {
#ifdef CVMX_PKO_QUEUES_PER_PORT_INTERFACE3
	n = CVMX_PKO_QUEUES_PER_PORT_INTERFACE3;
#endif
    }
    if (interface == 4)
    {
#ifdef CVMX_PKO_QUEUES_PER_PORT_INTERFACE4
	n = CVMX_PKO_QUEUES_PER_PORT_INTERFACE4;
#endif
    }

    mode = __cvmx_helper_cfg_mode(interface);
    if (mode == CVMX_HELPER_INTERFACE_MODE_LOOP)
    {
#ifdef CVMX_PKO_QUEUES_PER_PORT_LOOP
	n = CVMX_PKO_QUEUES_PER_PORT_LOOP;
#endif
    }
    if (mode == CVMX_HELPER_INTERFACE_MODE_NPI)
    {
#ifdef CVMX_PKO_QUEUES_PER_PORT_PCI
	n = CVMX_PKO_QUEUES_PER_PORT_PCI;
#endif
    }

    return n;
}
#endif /* CVMX_USER_DEFINED_HELPER_CONFIG_INIT */
#endif /* CVMX_ENABLE_HELPER_FUNCTIONS */

int __cvmx_helper_cfg_init(void)
{
#ifdef CVMX_ENABLE_HELPER_FUNCTIONS
    struct cvmx_cfg_port_param *pport;
    int cvmx_cfg_default_pko_nports;
    int pknd, bpid, pko_port_base;
    int qbase, mode;
    int i, j, n;

    cvmx_cfg_default_pko_nports = 1;
#endif
    
    if (!__cvmx_helper_cfg_init_common())
        return 0;

#ifdef CVMX_ENABLE_HELPER_FUNCTIONS
    /*
     * First thing: number of ports for each interface.
     */
    for (i = 0; i < CVMX_HELPER_CFG_MAX_IFACE; i++)
    {
	mode = __cvmx_helper_cfg_mode(i);
	if (mode != CVMX_HELPER_INTERFACE_MODE_DISABLED)
	{
            cvmx_cfg_iface_params[i].ccip_nports =
	        cvmx_helper_cfg_qlm_nports(i);

	    /*
	     * Overwrite for ILK 
	     */
	    if (mode == CVMX_HELPER_INTERFACE_MODE_ILK)
                cvmx_cfg_iface_params[i].ccip_nports =
	            cvmx_helper_cfg_ilk_nports (i);
        }
    }

#ifdef CVMX_USER_DEFINED_HELPER_CONFIG_INIT
{
	int cvmx_ucfg_nq;
	cvmx_ucfg_nq = 0;
#include "cvmx-helper-cfg-init.c"
}
#endif

    /*
     * per-port parameters
     */
    pknd = 0;
    bpid = 0;
    pko_port_base = 0;
    
    for (i = 0; i < CVMX_HELPER_CFG_MAX_IFACE; i++)
    {
	n = __cvmx_helper_cfg_num_ports(i);
    
        pport = cvmx_cfg_port[i];
        for (j = 0; j < n; j++, pport++)
	{
	    int t;

	    t = cvmx_cfg_default_pko_nports;
	    if (pport->ccpp_pko_num_ports != CVMX_HELPER_CFG_INVALID_VALUE)
	        t = pport->ccpp_pko_num_ports;

            *pport = (struct cvmx_cfg_port_param) {
		pknd++,
		bpid++,
		pko_port_base,
		t,
	        pport->ccpp_pko_nqueues};
	    pko_port_base += t;
	}
    }
    
    cvmx_helper_cfg_assert(pknd <= CVMX_HELPER_CFG_MAX_PIP_PKND);
    cvmx_helper_cfg_assert(bpid <= CVMX_HELPER_CFG_MAX_PIP_BPID);
    cvmx_helper_cfg_assert(pko_port_base <= CVMX_HELPER_CFG_MAX_PKO_PORT);
    
    /*
     * pko_port map
     */
    cvmx_helper_cfg_init_pko_port_map();

    /*
     * per-pko_port parameters
     */
    qbase = 0;
    for (i = 0; i < pko_port_base; i++)
    {
#ifdef CVMX_USER_DEFINED_HELPER_CONFIG_INIT
	n = __cvmx_ucfg_nqueues(i);
#else
	n = cvmx_helper_cfg_dft_nqueues(i);
#endif
	cvmx_cfg_pko_port[i] = (struct cvmx_cfg_pko_port_param) {qbase, n};
	qbase += n;
	cvmx_helper_cfg_assert(qbase <= CVMX_HELPER_CFG_MAX_PKO_QUEUES);
    }

#ifdef CVMX_ENABLE_PKO_FUNCTIONS
    __cvmx_helper_cfg_init_ipd2pko_cache();
#endif

#ifdef CVMX_HELPER_CFG_DEBUG
    cvmx_helper_cfg_show_cfg();
#endif /* CVMX_HELPER_CFG_DEBUG */
#endif
    return 0;
}
