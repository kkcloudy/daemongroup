/* file se_agent_octnic.c */
/* support pcie interface for se_agent */
/* se_agnet use this interface to send fccp cmd to slave CPU */
/* author: zhaohan@autelan.com */
/* date: 2012-5-28 */

#include "cavium_sysdep.h"
#include "cavium_defs.h"
#include "octeon-opcodes.h"
#include "octeon-common.h"
#include "octeon_user.h"
#include "cavium_release.h"
#include <signal.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include "se_agent/se_agent_def.h"
#include "se_agent_octnic.h"


#define OCTEON_ID    0
#define REQ_IQ_NO    0
#define TEST_THREAD_COUNT  1
#define MAX_INBUFS 1
#define MAX_OUTBUFS 1
#define INBUF_SIZE    (1 * 1024)
#define OUTBUF_SIZE    (1 * 1024)
#define REQUEST_TIMEOUT   5000
#define MAX_NB_REQUESTS   256

#define   REQ_NONE  0
#define   REQ_PEND  1
#define   REQ_DONE  2
#define __print printf

OCTEON_DMA_MODE        TEST_DMA_MODE  =   OCTEON_DMA_DIRECT;
OCTEON_RESPONSE_ORDER   TEST_RESP_ORDER  =  OCTEON_RESP_NORESPONSE;
OCTEON_RESPONSE_ORDER   TEST_RESP_MODE  =  OCTEON_RESP_NON_BLOCKING;

int oct_id=OCTEON_ID;



int set_buffers(octeon_soft_request_t   *soft_req,
                int                      incnt,
                int                      outcnt)
{
	int i,j;

	__print("incnt: %d outcnt: %d\n", incnt, outcnt);

	if((incnt > MAX_INBUFS) || (outcnt > MAX_OUTBUFS))
		return 1;

	memset(&soft_req->inbuf, 0, sizeof(octeon_buffer_t));
	memset(&soft_req->outbuf, 0, sizeof(octeon_buffer_t));

	soft_req->inbuf.cnt     = 0;
	for(i = 0; i < incnt; i++)  {
		soft_req->inbuf.cnt++;
		SOFT_REQ_INBUF(soft_req, i)  = malloc(INBUF_SIZE);
		if(SOFT_REQ_INBUF(soft_req, i) == NULL)
			goto free_inbufs;
		__print("inbuf.ptr[%d] = %p\n", i, SOFT_REQ_INBUF(soft_req, i)); 
		soft_req->inbuf.size[i] = INBUF_SIZE;
#ifdef VERIFY_DATA
		for(j = 0; j < INBUF_SIZE; j++)
			SOFT_REQ_INBUF(soft_req, i)[j] = ((j&0xff)==0)?1:j;
#endif
	}

	soft_req->outbuf.cnt     = 0;
	for(i = 0; i < outcnt; i++)  {
		soft_req->outbuf.cnt++;
		SOFT_REQ_OUTBUF(soft_req, i)  = malloc(OUTBUF_SIZE);
		if(SOFT_REQ_OUTBUF(soft_req, i) == NULL)
			goto free_outbufs;
		__print("outbuf.ptr[%d] = %p\n", i, SOFT_REQ_OUTBUF(soft_req, i)); 
		soft_req->outbuf.size[i] = OUTBUF_SIZE;
#ifdef VERIFY_DATA
		memset(SOFT_REQ_OUTBUF(soft_req, i), i+1, OUTBUF_SIZE);
#endif
	}
	return 0;
	
free_outbufs:
	for(i = 0; i < outcnt; i++)  {
		if(SOFT_REQ_OUTBUF(soft_req, i))
			free(SOFT_REQ_OUTBUF(soft_req, i));
	}
free_inbufs:
	for(i = 0; i < incnt; i++)  {
		if(SOFT_REQ_INBUF(soft_req, i))
			free(SOFT_REQ_INBUF(soft_req, i));
	}
	return 1;
}


void print_outbuf(octeon_soft_request_t *sr)
{
    uint32_t i = 0;
    uint8_t* buf = sr->outbuf.ptr[0].addr;
    uint32_t size = sr->outbuf.size[0];

    if(buf == NULL)
        return;
    
    printf("response outbuf data size = %d\n", size);
    for(i = 0; i < size; i ++)
    {
        printf("%x ", buf[i]);
        if((i+1)%16 == 0)
            printf("\n");
    }

    printf("response end\n");
}


int
send_request(int oct_id, octeon_soft_request_t *sr)
{
	int                      status=-1, retval, i;
	octeon_request_info_t   *req_info;

	req_info = SOFT_REQ_INFO(sr);

	retval = octeon_send_request(oct_id, sr);

	if(!retval) {
	    if (req_info->req_mask.resp_mode == OCTEON_RESP_NON_BLOCKING)
			status = retval;
		else
			status = req_info->status;
	} else {
		printf("\n----Request (req_id: %d) FAILED; retval: %d----\n",
		       SOFT_REQ_INFO(sr)->request_id, retval);
	}

	return status;
}


octeon_soft_request_t  *
create_soft_request(OCTEON_DMA_MODE        dma_mode,
                    OCTEON_RESPONSE_ORDER  resp_order,
                    OCTEON_RESPONSE_MODE   resp_mode,
                    uint32_t               inbuf_cnt,
                    uint32_t               outbuf_cnt,
                    uint32_t               tag,
                    uint32_t               q_no)
{
	octeon_soft_request_t   *soft_req=NULL;
	octeon_request_info_t   *req_info=NULL;

	soft_req = malloc(sizeof(octeon_soft_request_t));
	if(soft_req == NULL)
		return NULL;

	req_info = malloc(sizeof(octeon_soft_request_t));
	if(req_info == NULL) {
		free(soft_req);
		return NULL;
	}

	memset(soft_req, 0, sizeof(octeon_soft_request_t));
	memset(req_info, 0, sizeof(octeon_request_info_t));

	SOFT_REQ_INFO(soft_req) = req_info;

	soft_req->ih.raw     = 1;
	soft_req->ih.qos     = 0;
	soft_req->ih.grp     = 0;
	soft_req->ih.rs      = 0;
	soft_req->ih.tagtype = 1;
	soft_req->ih.tag = tag;
	if((dma_mode == OCTEON_DMA_GATHER)
	   || (dma_mode == OCTEON_DMA_SCATTER_GATHER))
		soft_req->ih.gather  = 1;	
	soft_req->irh.opcode = CVMCS_REQRESP_OP;
	soft_req->irh.param  = 0x10;
	soft_req->irh.dport  = 32;
	if((dma_mode == OCTEON_DMA_SCATTER)
	   || (dma_mode == OCTEON_DMA_SCATTER_GATHER))
		soft_req->irh.scatter  = 1;	
	req_info->octeon_id  = 0;
	req_info->request_id = 0xff;
	req_info->req_mask.dma_mode   = dma_mode;
	req_info->req_mask.resp_mode  = resp_mode;
	req_info->req_mask.resp_order = resp_order;
	req_info->req_mask.iq_no = q_no;
	req_info->timeout             =  REQUEST_TIMEOUT;

	if(set_buffers(soft_req, inbuf_cnt, outbuf_cnt)) {
		free(req_info);
		free(soft_req);
		return NULL;
	}
	SOFT_REQ_INFO(soft_req)->status = 3;

	return soft_req;
}



void
free_soft_request(octeon_soft_request_t  *soft_req)
{
	uint32_t i;
	for(i = 0; i < soft_req->outbuf.cnt; i++)  {
		if(SOFT_REQ_OUTBUF(soft_req, i))
			free(SOFT_REQ_OUTBUF(soft_req, i));
	}
	for(i = 0; i < soft_req->inbuf.cnt; i++)  {
		if(SOFT_REQ_INBUF(soft_req, i))
			free(SOFT_REQ_INBUF(soft_req, i));
	}
	free(SOFT_REQ_INFO(soft_req));
	free(soft_req);
}


int
unordered_blocking_request(uint8_t* buf_send, int32_t slen, uint8_t* buf_rcv, int32_t rlen)
{
	octeon_soft_request_t   *soft_req= NULL;
	int                      req_status=0, retval = 0;
	uint32_t                 incnt, outcnt;
	uint32_t                 insize=0, outsize=0;
	uint32_t                tag = 0x101011;   
	int q_no = REQ_IQ_NO;

	if((!buf_send) || (!buf_rcv) || (slen > INBUF_SIZE) || (rlen > OUTBUF_SIZE))
	{
	    printf("unordered_blocking_request: invalid args\n");
	    return 1;
    }
    
	incnt = MAX_INBUFS;
	outcnt = MAX_OUTBUFS;
	insize = (incnt * INBUF_SIZE);
	outsize = (outcnt * OUTBUF_SIZE);


	soft_req = create_soft_request(OCTEON_DMA_DIRECT, OCTEON_RESP_UNORDERED,
	                               OCTEON_RESP_BLOCKING,
	                               incnt, outcnt, tag, q_no);

	if(soft_req == NULL) {
		printf("Soft request alloc failed\n");
		return 1;
	}
	__print("\n Created Request with incnt: %d outcnt: %d\n", incnt,outcnt);

    memset(SOFT_REQ_INBUF(soft_req, 0), 0, INBUF_SIZE);
    memcpy(SOFT_REQ_INBUF(soft_req, 0), buf_send, slen);
			
	req_status = send_request(oct_id, soft_req);
	if(!req_status) {		
		//print_outbuf(soft_req);
        memcpy(buf_rcv, soft_req->outbuf.ptr[0].addr, rlen);
	} else {
		printf("Request Failed with status %d\n", req_status);
		retval = 1;
	}
	free_soft_request(soft_req);

	return retval;
}


/**********************************************************************************
 *  se_agent_pci_channel_init
 *
 *	DESCRIPTION:
 * 		init pcie channel for se_agent.
 *         open octeon driver
 *
 *	INPUT:
 *		NULL
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		SE_AGENT_RETURN_OK        
 *		SE_AGENT_RETURN_FAIL 
 *		
 **********************************************************************************/
int32_t se_agent_pci_channel_init()
{
    if(octeon_initialize())
    {
		printf("oct_req: Could not find octeon device\n");   
		return SE_AGENT_RETURN_FAIL;
    }
    return SE_AGENT_RETURN_OK;
}

/**********************************************************************************
 *  se_agent_send_fccp_from_pci
 *
 *	DESCRIPTION:
 * 		send fccp cmd to slave cpu from pcie interface
 *
 *	INPUT:
 *          fccp_send - fccp cmd to send
 *          slen - fccp_send len
 *	       rlen - fccp_rcv len
 *
 *	OUTPUT:
 *		fccp_rcv - fccp cmd recv from slave cpu
 *
 * 	RETURN:
 *		SE_AGENT_RETURN_OK        
 *		SE_AGENT_RETURN_FAIL 
 *		
 **********************************************************************************/
int32_t se_agent_send_fccp_from_pci(uint8_t* fccp_send, int32_t slen, uint8_t* fccp_rcv, int32_t rlen)
{
    if(unordered_blocking_request(fccp_send, slen, fccp_rcv, rlen))
    {
        return SE_AGENT_RETURN_FAIL;
    }    

    return SE_AGENT_RETURN_OK;
}

#if 0
int se_agent_request_test()
{
    int i = 0;
    char fccp[128];
    for(i = 0; i < 128; i++)
        fccp[i] = i;
        
	unordered_blocking_request(fccp, 128);

    for(i = 0; i < 128; i++)
    {
        printf("%x ", fccp[i]);
        if((i+1)%16 == 0)
            printf("\n");
    }
    
	return 0;
}
#endif

