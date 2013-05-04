/*
 *
 * OCTEON SDK
 *
 * Copyright (c) 2011 Cavium Networks. All rights reserved.
 *
 * This file, which is part of the OCTEON SDK which also includes the
 * OCTEON SDK Package from Cavium Networks, contains proprietary and
 * confidential information of Cavium Networks and in some cases its
 * suppliers. 
 *
 * Any licensed reproduction, distribution, modification, or other use of
 * this file or the confidential information or patented inventions
 * embodied in this file is subject to your license agreement with Cavium
 * Networks. Unless you and Cavium Networks have agreed otherwise in
 * writing, the applicable license terms "OCTEON SDK License Type 5" can be found 
 * under the directory: $OCTEON_ROOT/components/driver/licenses/
 *
 * All other use and disclosure is prohibited.
 *
 * Contact Cavium Networks at info@caviumnetworks.com for more information.
 *
 */



#include "cvmcs-common.h"
#include  <cvmx-atomic.h>
#include "cvmcs-nic.h"
#include "cvmcs-nic-api.h"



CVMX_SHARED uint32_t wqe_count[MAX_CORES] = {
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

CVMX_SHARED octnic_dev_t  *octnic;


extern CVMX_SHARED  uint64_t          cpu_freq;
extern CVMX_SHARED  cvm_oct_dev_t    *oct;
extern uint32_t  core_id;

extern int __get_active_pci_oq_count(void);

static inline void cvmcs_nic_print_stats(void)
{
	int i;
	oct_link_stats_t  *st;

	printf("-- RGMII driver stats --\n");
	printf(" (Rx pkts from wire; Tx pkts from host)\n");
	for(i = 0; i < octnic->nports; i++) {
		printf("Port%d: ", i); 
		st = &octnic->port[i].stats;

		printf(" Rx : %llu ", cast64(st->fromwire.total_rcvd));
		if(st->fromwire.err_pko)
			printf("(%llu PKO Err) ", cast64(st->fromwire.err_pko));
		if(st->fromwire.err_link)
			printf("(%llu Link Err) ", cast64(st->fromwire.err_link));
		if(st->fromwire.err_drop)
			printf("(%llu Drops) ", cast64(st->fromwire.err_drop));

		printf(" Tx : %llu ", cast64(st->fromhost.total_rcvd));
		if(st->fromhost.err_pko)
			printf("(%llu PKO Err) ", cast64(st->fromhost.err_pko));
		if(st->fromhost.err_link)
			printf("(%llu Link Err) ", cast64(st->fromhost.err_link));
		if(st->fromhost.err_drop)
			printf("(%llu Drops) ", cast64(st->fromhost.err_drop));
		printf("\n");
	}
}


static inline void cvmcs_nic_dump_ptrs(cvmx_buf_ptr_t  *ptr, int numbufs)
{
	int              i, total=0;
	cvmx_buf_ptr_t  *next;

	for(i = 0; i < numbufs; i++) {
		next = (cvmx_buf_ptr_t *)CVM_DRV_GET_PTR(ptr->s.addr - 8);
		printf("ptr[%d]: 0x%016llx  size: %d pool %d\n", i, CAST64(ptr->s.addr),
				ptr->s.size, ptr->s.pool);
		total += ptr->s.size;
		ptr = next;
	}

	printf("Total Bytes: %d\n", total);
}


/* Direction: 0 - to wire, 1 - to host */
static inline int cvmcs_nic_send_to_pko(cvmx_wqe_t  *wqe,  int dir, int port, int queue, int offload)
{
	cvmx_pko_command_word0_t    pko_command;

	CVMX_SYNCWS;

	/* Prepare to send a packet to PKO. */
	if (dir && octeon_has_feature(OCTEON_FEATURE_PKND))
		cvmx_pko_send_packet_prepare_pkoid(port, queue, 1);
	else
		cvmx_pko_send_packet_prepare(port, queue, 1);

	/* Build a PKO pointer to this packet */
	pko_command.u64           = 0;

	/* Setting II = 0 and DF = 0 will free all buffers whose I bit is not set.
	   Since I bit is not set by default in pkt buffer ptrs, this setting allows
	   packets to be forwarded to host/wire without having to touch each pkt
	   ptr to set the I bit. */
	pko_command.s.ignore_i    = 0;
	pko_command.s.dontfree    = 0;
	pko_command.s.segs        = wqe->word2.s.bufs;
	pko_command.s.total_bytes = cvmx_wqe_get_len(wqe);
	if(offload)
		pko_command.s.ipoffp1 = offload;

	DBG_PRINT(DBG_FLOW,"pko cmd: %016llx lptr: %016llx PORT: %d Q: %d\n",
			cast64(pko_command.u64), cast64(wqe->packet_ptr.u64), port,
			queue);

	/* Send a packet to PKO. */
	if (dir && octeon_has_feature(OCTEON_FEATURE_PKND))
		cvmx_pko_send_packet_finish_pkoid(port, queue, pko_command, wqe->packet_ptr, 1);
	else
		cvmx_pko_send_packet_finish(port, queue, pko_command, wqe->packet_ptr, 1);

	return 0;
}


static inline void cvmcs_nic_to_host(cvmx_wqe_t  *wqe, octnic_port_info_t  *nicport)
{
	cvmx_resp_hdr_t     *resp_ptr;
	cvmx_buf_ptr_t      *lptr;
	int                  port, queue;

	cvmx_atomic_add64(&nicport->stats.fromwire.total_rcvd, 1);

	if(!nicport->state.rx_on) {
		cvmx_atomic_add64(&nicport->stats.fromwire.err_drop, 1);
		cvm_free_wqe_and_pkt_bufs(wqe);
		return;
	}

	wqe->word2.s.bufs += 1;
	cvmx_wqe_set_len(wqe, (cvmx_wqe_get_len(wqe) + CVMX_RESP_HDR_SIZE));

	/* We use the space in WQE starting at byte offset 40 for the response
	   header information. This allows the WQE to be used for data and let
	   PKO free the WQE buffer. */
	lptr      = (cvmx_buf_ptr_t *)&wqe->packet_data[40];
	lptr->u64 = wqe->packet_ptr.u64;

	resp_ptr = (cvmx_resp_hdr_t  *)&wqe->packet_data[48];
	resp_ptr->u64         = 0;
	resp_ptr->s.opcode    = CORE_NW_DATA_OP;
	resp_ptr->s.destqport = nicport->linfo.ifidx;

	/* lptr is reused */
	lptr         = &wqe->packet_ptr;
	lptr->u64    = 0;
	lptr->s.addr = CVM_DRV_GET_PHYS(resp_ptr);
	lptr->s.size = 8;
	lptr->s.pool = CVMX_FPA_WQE_POOL;
	
	//DBG("wqe @ %p wqe bufs: %d len: %d pkt_ptr @ %lx\n", wqe,
		//	wqe->word2.s.bufs, cvmx_wqe_get_len(wqe), (uint64_t)wqe->packet_ptr.s.addr);

	port  = cvm_pci_get_oq_pkoport(nicport->linfo.rxpciq);
	queue = cvm_pci_get_oq_pkoqueue(nicport->linfo.rxpciq);

	if(cvmx_unlikely(port == -1 || queue == -1)) {
		cvmx_atomic_add64(&nicport->stats.fromwire.err_drop, 1);
		cvm_free_wqe_and_pkt_bufs(wqe);
		return;
	}

	//DBG("to host: rxq: %d port: %d queue: %d\n", nicport->linfo.rxpciq, port,
		//	queue);

	if(!cvmcs_nic_send_to_pko(wqe, 1, port, queue, 0)) {
		cvmx_atomic_add64(&nicport->stats.fromwire.total_fwd, 1);
		return;
	}

	cvmx_atomic_add64(&nicport->stats.fromwire.err_pko, 1);
	cvm_free_wqe_and_pkt_bufs(wqe);
}

/**
 * this routine forward the packet to host through the appropriate PCIe channel.
 * packet recv from pip port, each port respond to a PCIe channel.
 * there are 4 channel in a PCIe interface.
 *
 * @param work  wqe pointer
 */
inline void cvmcs_nic_forward_packet_to_host(cvmx_wqe_t  *work)
{
    int  ifidx = cvmcs_nic_find_idx(cvmx_wqe_get_port(work));
    if(ifidx == -1) 
    {
        cvmx_helper_free_packet_data(work);
        cvmx_fpa_free(work, CVMX_FPA_WQE_POOL, 0);
        return;
    }

    if(octnic->port[ifidx].state.active) 
    {
        if(cvmx_likely(!work->word2.snoip.rcv_error)) 
        {
            cvmcs_nic_to_host(work, &octnic->port[ifidx]);
        }
        else 
        {
            octnic->port[ifidx].stats.fromwire.err_drop++;
            cvmx_helper_free_packet_data(work);
            cvmx_fpa_free(work, CVMX_FPA_WQE_POOL, 0);
        }
        return;
    }
}

/**
* forward packet out, send out from gmx port.
* @param wqe      wqe pointer
* @param ifidx      port registed in octnic, the real phy port is octnic->port[ifidx].linfo.gmxport. 
* @param offload  the number of bytes to IP header, use for hardware calculate and insert tcp/udp check sum.
*/
static inline void cvmcs_nic_forward_packet_to_wire(cvmx_wqe_t   *wqe, int ifidx, int offload)
{
	uint64_t             nextptr;
	octnic_port_info_t  *nicport = &octnic->port[ifidx];
	int                  port, queue;


	cvmx_atomic_add64(&nicport->stats.fromhost.total_rcvd, 1);
#if 0
	if(cvmx_unlikely(nicport->linfo.link.s.status == 0) ) {
		cvmx_atomic_add64(&nicport->stats.fromhost.err_link, 1);
		cvm_free_wqe_and_pkt_bufs(wqe);
		return;
	}
#endif
	nextptr = *((uint64_t *)CVM_DRV_GET_PTR(wqe->packet_ptr.s.addr - 8));
	wqe->packet_ptr.s.addr += CVM_RAW_FRONT_SIZE;
	wqe->packet_ptr.s.size -= CVM_RAW_FRONT_SIZE;
	wqe->packet_ptr.s.back   = CVM_COMMON_CALC_BACK_POWER_2(wqe->packet_ptr);
	cvmx_wqe_set_len(wqe, (cvmx_wqe_get_len(wqe) - CVM_RAW_FRONT_SIZE));
	*((uint64_t *)CVM_DRV_GET_PTR(wqe->packet_ptr.s.addr - 8)) = nextptr;

	port  = nicport->linfo.gmxport;
	queue = cvmx_pko_get_base_queue(nicport->linfo.gmxport);

	if(cvmx_unlikely(port == -1 || queue == -1)) {
		cvmx_atomic_add64(&nicport->stats.fromhost.err_drop, 1);
		cvm_free_wqe_and_pkt_bufs(wqe);
		return;
	}

	//DBG("to wire: port: %d queue: %d\n", port, queue);
	if(!cvmcs_nic_send_to_pko(wqe, 0, port, queue, offload)) {
		cvmx_atomic_add64(&nicport->stats.fromhost.total_fwd, 1);
		cvm_drv_fpa_free(wqe, CVMX_FPA_WQE_POOL, 0);
		return;
	}

	cvmx_atomic_add64(&nicport->stats.fromwire.err_pko, 1);
	cvm_free_wqe_and_pkt_bufs(wqe);
}




/**
* process packet which recv form PCIe port. the packet is FCCP or normal pakcet.
* if FCCP packet, return to main_loop to parse.
* if normal packet, send out from appropriate gmx port.
* @param work  wqe pointer
* @return NIC_FCCP_PKT                   FCCP packet, need main_loop to parse.
* @return NIC_PROCESS_PKT_OK        send out success
* @return NIC_PROCESS_PKT_ERROR   send out fail
*/
static inline int cvmcs_nic_process_packet(cvmx_wqe_t  *work)
{
	cvmx_raw_inst_front_t *front = NULL;
	uint16_t proto = 0;
	int8_t* pkt_ptr = NULL;

	pkt_ptr = (int8_t*)cvmx_phys_to_ptr(work->packet_ptr.s.addr) + CVM_RAW_FRONT_SIZE;
	proto = *(uint16_t *)(pkt_ptr + 12);

	if(proto == 0x8050)
	{
	    /* back to main_loop to parse FCCP packet. don't change pkt */
		return NIC_FCCP_PKT;
	}
	else
	{
		/* normal packet, send out by pko  */
		front = (cvmx_raw_inst_front_t *)work->packet_data;
		cvmcs_nic_forward_packet_to_wire(work, front->irh.s.param, front->irh.s.rlenssz);
	}

	return NIC_PROCESS_PKT_OK;
}


static inline int
__get_localbufsize(uint32_t         resplen,
                   uint32_t         curlen,
                   cvmx_buf_ptr_t  *ptr,
                   int              idx)
{
/* If currsize + byes in this buffer exceeds resplen, then bytes in this buffer
   satisfies resplen; in which case return resplen - curr size;

	bytes in this buffer is pkt pool size - offset.
	Subtract 24 for first buffer.
*/
	int offset = (ptr->s.addr - (((ptr->s.addr >> 7) - ptr->s.back) << 7));
	int thisbufsize;

	thisbufsize = (CVMX_FPA_PACKET_POOL_SIZE - offset - ((idx == 0)?24:0));

	return ((curlen + (uint32_t)thisbufsize) > resplen)?(resplen - curlen):(uint32_t)thisbufsize;
}


int
cvmcs_process_direct_instruction(cvmx_wqe_t             *wqe,
                                 cvmx_raw_inst_front_t  *front)
{
	cvmx_oct_pci_dma_inst_hdr_t     dmacmd;
	cvm_dma_remote_ptr_t            rptr;
	cvmx_oct_pci_dma_local_ptr_t    lptr[15];
	cvmx_buf_ptr_t                 *dptr;
	uint64_t                       *respptr;
	uint32_t                        respsize, currsize=0, lastrptrsize=0;
	int                             i = 0, bufidx = 0;


	/* The response contains the entire data payload in the received request
	   PLUS 16 bytes for the ORH and status words. Since the wqe payload
	   contains the front data as well, we take that out of the total. */
	respsize = cvmx_wqe_get_len(wqe) - sizeof(cvmx_raw_inst_front_t) + 16;

	//printf("%s: wqe @ %p, len: %d pkt_ptr: 0x%016lx respsize: %d\n", __FUNCTION__, wqe, wqe->len, wqe->packet_ptr.u64, respsize);


	/* These fields are fixed. The rest we generate on the way. */
	dmacmd.u64      = 0;
	dmacmd.s.nr     = 1;
	dmacmd.s.lport  = front->irh.s.pcie_port;

#ifdef CVMCS_DMA_IC
	dmacmd.s.c      = 0;
#else
	dmacmd.s.c      = core_id & 1;
#endif


	/* We use the space in the wqe->packet data for ORH and status words. We
	   set them to 0. If the host app checks the ORH, set the first 8 bytes
	   accordingly. */
	respptr = (uint64_t *) (wqe->packet_data + sizeof(cvmx_raw_inst_front_t));
	respptr[0] = respptr[1] = 0;


	/* The first 8 bytes to be transmitted are the ORH. */
	lptr[i].u64    = 0;
	lptr[i].s.addr = cvmx_ptr_to_phys(respptr);
	lptr[i].s.size = 8;
	currsize += 8;
	i++;

	/* Then starts the data payload. The first local pointer for data needs to
	   be handled differently since the WQE packet ptr data include the front
	   data. */
	lptr[i].u64    = wqe->packet_ptr.u64;
	lptr[i].s.addr += sizeof(cvmx_raw_inst_front_t);


	/* response length includes 16 bytes which are not present in the input
	   bytes, so don't include them. Also current size includes the 8 bytes of
	   ORH which is not present in the input bytes, so don't include them.
	*/
	lptr[i].s.size = __get_localbufsize(respsize - 16,
	                                    currsize-8, &wqe->packet_ptr, bufidx);

	lptr[i].s.pool = CVMX_FPA_PACKET_POOL;
	lptr[i].s.i    = 1;


	currsize += lptr[i].s.size;
	i++; bufidx++; 

	/* Get the next buffer pointer. */
	dptr = (cvmx_buf_ptr_t *)cvmx_phys_to_ptr(wqe->packet_ptr.s.addr - 8);


	/* If there are more buffers, start adding them to this DMA command. */
	while(bufidx < wqe->word2.s.bufs) {

		lptr[i].u64    = dptr->u64;
		lptr[i].s.size = __get_localbufsize(respsize - 16,
		                                    currsize - 8, dptr, bufidx);
		lptr[i].s.pool = CVMX_FPA_PACKET_POOL;
		lptr[i].s.i    = 1;

		currsize += lptr[i].s.size;
		i++; bufidx++;

		/* If we have collected max local pointers, then issue a DMA command.
		   The rest will be sent in additional DMA commands later on. */
		if(i == octdev_max_dma_localptrs()) {
			dmacmd.s.nl = i;
			/* The location to copy in host memory needs to be adjusted to
			   account for the bytes we already sent. */
			rptr.s.addr = front->rptr + lastrptrsize;
			rptr.s.size = currsize - lastrptrsize;
			CVMX_SYNCWS;
			if(cvm_pci_dma_raw(&dmacmd, lptr, &rptr)) {
				printf("%s: Failed to send DMA command: 0x%016lx\n",
				       __FUNCTION__, dmacmd.u64);
				return 1;
			}
			i = 0; lastrptrsize = currsize;
		}

		/* Get the next buffer pointer */
		dptr = (cvmx_buf_ptr_t *)cvmx_phys_to_ptr(dptr->s.addr - 8);
	}


	/* If there isn't enough space to accomodate the last local pointer (for
	   status word, send the DMA command now for accumulated local buffers. */
	if((i+1) > octdev_max_dma_localptrs()) {
		dmacmd.s.nl = i;
		rptr.s.addr = front->rptr + lastrptrsize;
		rptr.s.size = currsize - lastrptrsize;
		CVMX_SYNCWS;
		if(cvm_pci_dma_raw(&dmacmd, lptr, &rptr)) {
			printf("%s: Failed to send DMA command: 0x%016lx\n",
			       __FUNCTION__, dmacmd.u64);
			return 1;
		}
		i = 0; lastrptrsize = currsize;
	}


	/* Finally, the status bytes should be included in the response. */
	lptr[i].u64    = 0;
	lptr[i].s.addr = cvmx_ptr_to_phys(respptr + 1);
	lptr[i].s.size = 8;
	lptr[i].s.pool = CVMX_FPA_WQE_POOL;
	lptr[i].s.i    = 1;

	currsize += 8;

	i++;

	dmacmd.s.nl = i;
#ifdef CVMCS_DMA_IC
	dmacmd.s.ca = 1;
#endif
	rptr.s.addr = front->rptr + lastrptrsize;
	rptr.s.size = currsize - lastrptrsize;
	CVMX_SYNCWS;

	return cvm_pci_dma_raw(&dmacmd, lptr, &rptr);
}


int
cvmcs_process_instruction(cvmx_wqe_t   *wqe)
{
	cvmx_raw_inst_front_t  *front = (cvmx_raw_inst_front_t *)wqe->packet_data;

	/* Noresponse packets have rptr = 0. These packets are valid, but require
	   no more processing. Free the WQE and return success. */
	if(front->rptr == 0) 
	{
	    printf("cvmcs_process_instruction: rptr == NULL!\n");
		cvm_free_host_instr(wqe);
		return 0;
	}

	if(cvmx_likely(front->irh.s.scatter == 0)) 
	{
		/* For a request with direct response, the return address is known. The
		   response can be sent right away. */
		//printf("cvmcs_process_instruction: ------ cvmcs_process_direct_instruction\n");
		if(cvmcs_process_direct_instruction(wqe, front) != 0)
		{
            printf("cvmcs_process_direct_instruction error!\n");
            cvm_free_host_instr(wqe);
		    return 1;
		}	
		//printf("cvmcs_process_direct_instruction success!\n");
		return 0;
	} 
	else 
	{
	    printf("cvmcs_process_instruction: scatter = 1\n");
		cvm_free_host_instr(wqe);
		return 0;
	}

	return 0;
}


/** 
 * All work received by the PCIe interface is forwarded to this routine.
 * All RAW packets with opcode=0x1234 and param=0x10 are test instructions
 * and handle by the application. All other RAW packets with opcode in
 * the range 0x1000-0x1FFF is given to the core driver. All other packets
 * are dropped.
 *
 * @param wqe  wqe pointer
 */
inline int cvmcs_nic_process_wqe(cvmx_wqe_t  *wqe)
{
	cvmx_raw_inst_front_t     *front;
	uint32_t                   opcode;
    int32_t                   ret = 0;
	
	front = (cvmx_raw_inst_front_t *)wqe->packet_data;
	opcode = front->irh.s.opcode;
	
	switch(opcode) 
	{
#if 0
        /* test begin */
	    case CVMCS_REQRESP_OP:
	        cvmcs_process_instruction(wqe);
	        break;
	    /* test end */
#endif	    
		case OCT_NW_PKT_OP:
		case CVMCS_REQRESP_OP:
		    ret = cvmcs_nic_process_packet(wqe);
			break;

		case OCT_NW_CMD_OP:
			ret = cvmcs_nic_process_cmd(wqe);
			break;

		case HOST_NW_INFO_OP:
			ret = cvmcs_nic_send_link_info(wqe);
			break;

		default: 
			if(opcode >= DRIVER_OP_START && opcode <= DRIVER_OP_END) 
			{
				ret = cvm_drv_process_instr(wqe);
			} 
			else 
			{
				cvm_free_host_instr(wqe);
				ret = 0;
			}
	}

	return ret;
}

void
cvmcs_nic_set_link_status(void)
{
	int                 i;

	i = 0;
	while(octnic->port[i].state.present) {
		oct_link_status_t   link;
		cvmx_helper_link_info_t link_info;
		int port = octnic->port[i].linfo.gmxport;

        link_info.u64 = 0;
        link_info.s.speed = cvmcs_nic_get_port_default_speed(port);
        link_info.s.full_duplex = 1;
        link_info.s.link_up = 1;
        if(cvmx_helper_link_set(port, link_info) < 0)
            printf("cvmx_helper_link_set ipd_port %d FAILED\n", port);

        link.u64 = 0;
        link.s.duplex = link_info.s.full_duplex;
        link.s.status = link_info.s.link_up;
        link.s.speed  = link_info.s.speed;

		if(link.u64 != octnic->port[i].linfo.link.u64) {
			/* If link status has changed, update status info first. */
			octnic->port[i].linfo.link.u64 = link.u64;
			CVMX_SYNCW;
			/* Print the updated status. */
			cvmcs_nic_print_link_status(&link, i, port);
		}
		i++;
	}
}

/*
static void modify_ipd_settings(void)
{
	cvmx_ipd_ctl_status_t ipd_reg;
	ipd_reg.u64 = cvmx_read_csr(CVMX_IPD_CTL_STATUS);
	ipd_reg.s.len_m8 = 1;
	cvmx_write_csr(CVMX_IPD_CTL_STATUS, ipd_reg.u64);
}
*/

/**
 *only called by the first core 
 */
int cvmcs_nic_global_init()
{
	/* Initialize PCI driver */
	if(cvm_drv_init())
		return 1;

	cvmx_helper_ipd_and_packet_input_enable();

	/* overwriting the SLI_TX_PIPE Register with active queue count of 4 */ 
	if(OCTEON_IS_MODEL(OCTEON_CN68XX)) {
		uint32_t 	    activeqcnt;
		cvmx_sli_tx_pipe_t  slitxpipe;

		slitxpipe.u64 = cvmx_read_csr(CVMX_PEXP_SLI_TX_PIPE);
		activeqcnt = __get_active_pci_oq_count();
		slitxpipe.s.nump = activeqcnt;
		cvmx_write_csr(CVMX_PEXP_SLI_TX_PIPE, slitxpipe.u64);
		printf("[ NIC APP ] Active PCI Queues: %d (derived from checking queue registers)\n", activeqcnt);
	}

	/* Enable IPD only after sending the START indication packet to host. */
	cvmx_ipd_disable();

	/* add gmx port */
	if(cvmcs_nic_setup_interfaces())
		return 1;
    
    /* work at nic mode */
    cvm_drv_setup_app_mode(CVM_DRV_NIC_APP);
	return 0;
}


/**
 * local init, called by all cores 
 */
inline void cvmcs_nic_local_init()
{
    cvm_drv_local_init();
}


/** 
 * send a instr to host to notify that OCTEON can work
 */
inline int cvmcs_nic_start()
{
    return cvm_drv_start();
}


