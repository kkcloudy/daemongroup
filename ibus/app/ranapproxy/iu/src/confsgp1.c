/****************************************************************************
** Description:
** Code to describe usage of M3UA API's.
*****************************************************************************
** Copyright(C) 2005 Shabd Communications Pvt. Ltd. http://www.shabdcom.org
*****************************************************************************
** Contact:
** vkgupta@shabdcom.org
*****************************************************************************
** License :
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License as published by the Free Software Foundation; either
** version 2.1 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public
** License along with this library; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*****************************************************************************/

#include <confsgp1.h>
#include "sccp.h"
int cc_success;

int main(int argc, char *argv[])
{
    if (3 > argc) {
        printf("Usage: %s sgp=<IP Address>:<Port> "
               "asp=<IP Address>:<Port>\n", argv[0]);
        exit(1);
    }
	
	ranap_sccp_system();

    /**** Initialise the Transport Layer ****/
    um3_transport_init(argv);

    /**** setup the trace map for M3UA ****/
    um3_m3ua_trace_map();

    /**** Initialise m3ua stack ****/
    um3_m3ua_init();

    /******** Configure M3UA managed objects ********/

    /* Add nwapp */
    um3_m3ua_nwapp(M3_ADD);

    /* Add remote asp */
    um3_m3ua_r_asp(M3_ADD);

    /* Add remote as */
    um3_m3ua_r_as(M3_ADD);

    /* Add sgp */
    um3_m3ua_sgp(M3_ADD);

    /* Add user for sgp */
    um3_m3ua_user(M3_ADD);

    /* Add connection's */
    um3_m3ua_conn(M3_ADD, 0x00010000, 0x00000000, 0);

    /* change connection state */
    um3_m3ua_conn_state(M3_MODIFY, M3_CONN_ESTB, 0x00010000, 0x00000000);
	
 //   waitfor_message(NULL);
    return 0;
}

int um3_get_sgp1_addr(char *argv[], udp_addr_t *paddr)
{
    char        *p;
    char        *ip;
    char        *port;

    if (0 != strncmp("sgp", argv[1], 3))
        return -1;
    p = strchr(argv[1], '=');
    if (NULL == p)
        return -1;
    p++;
    if (0 == strlen(p))
        return -1;
    ip = strtok(p, ":");
    port = strtok(NULL, ":");
    paddr->ipaddr = inet_addr(ip);
    paddr->port = atoi(port);
printf("sgp.c sgp paddr ip %x, port %x\n", paddr->ipaddr, paddr->port);    
    return 0;
}

int um3_get_asp1_addr(char *argv[], udp_addr_t *paddr)
{
    char        *p;
    char        *ip;
    char        *port;

    if (0 != strncmp("asp", argv[2], 3))
        return -1;
    p = strchr(argv[2], '=');
    if (NULL == p)
        return -1;
    p++;
    if (0 == strlen(p))
        return -1;
    ip = strtok(p, ":");
    port = strtok(NULL, ":");
    paddr->ipaddr = inet_addr(ip);
    paddr->port = atoi(port);
printf("sgp.c asp paddr ip %x, port %x\n", paddr->ipaddr, paddr->port);    
    return 0;
}

int um3_transport_init(char *argv[])
{
    udp_addr_t        addr;
    int               asp1_ep, sgp1_ep;

    /* create transport layer endpoints */
    um3_get_sgp1_addr(argv, &addr);
    sgp1_ep = make_lep(addr);
    um3_get_asp1_addr(argv, &addr);
    asp1_ep = make_rep(addr);

    /* create associations between the transport layer endpoints */
    make_assoc(sgp1_ep, asp1_ep, 1);
    return 0;
}

m3_s32 um3_m3ua_init(void)
{
    m3_s32        ret;

    ret = m3ua_init();
    if (-1 == ret) {
        printf("Error recorded at Line:%d, Error code:%d\n", __LINE__, m3errno);
    }
    return ret;
}

m3_s32 um3_m3ua_nwapp(m3_u8   oprn)
{
    m3_s32         ret;
    m3ua_nwapp_t   api;

    switch(oprn) {
        case M3_ADD: {
            api.add.info.nw_app = 0x01ABCDEF;
            api.add.info.standard = M3_STD_ANSI;
            break;
        }
    }
    ret = m3ua_nwapp(oprn, &api);
    if (-1 == ret) {
        printf("Error recorded at Line:%d, Error code:%d\n", __LINE__, m3errno);
    }
    return ret;
}

m3_s32 um3_m3ua_sgp(m3_u8    oprn)
{
    m3_s32            ret;
    m3ua_sgp_t        api;
    m3_u32            sgp = 0x00010000;
    m3_u16            idx, idx2;
    m3_sgp_confname_t confname = M3_SGP_ADD_R_ASP;
    memset(&api, 0, sizeof(m3ua_sgp_t));
    switch(oprn) {
        case M3_ADD: {
            api.add.info.sctp_ep_id = 0;
            api.add.info.def_nwapp = 0x01ABCDEF;
            for (idx = 0; idx < M3_MAX_AS; idx++)
                for (idx2 = 0; idx2 < M3_MAX_AS; idx2++)
                    api.add.info.r_as_inf[idx].asp_list[idx2] = M3_FALSE;
            api.add.info.r_as_inf[0].asp_list[0] = M3_TRUE;
            break;
        }
        case M3_DELETE:
        case M3_GET: {
            break;
        }
        case M3_MODIFY: {
            switch(confname) {
                case M3_SGP_NWAPP: {
                /* modify default Network Appearance to 0x12345678 for */
                /* this ASP                                            */
                    api.modify.info.def_nwapp = 0x12345678;
                    break;
                }
                case M3_SGP_ADD_R_ASP: {
                /* Add remote asp 2 serving remote as 4 to ASP */
                    api.modify.ras_id = 4;
                    api.modify.rasp_id = 2;
                    break;
                }
                case M3_SGP_DEL_R_ASP: {
                /* Delete remote asp 3 serving remote as 3 from ASP */
                    api.modify.ras_id = 3;
                    api.modify.rasp_id = 3;
                    break;
                }
            }
            api.modify.confname = confname;
            break;
        }
    }
    ret = m3ua_sgp(sgp, oprn, &api);
    if (-1 == ret) {
        printf("Error recorded at Line:%d, Error code:%d\n", __LINE__, m3errno);
    }
    return ret;
}

m3_s32 um3_m3ua_r_asp(m3_u8    oprn)
{
    m3_s32            ret;
    m3ua_r_asp_t      api;
    m3_u32            rasp = 0x00000000;

    switch(oprn) {
        case M3_ADD: {
            api.add.info.sctp_ep_id = 0;
            break;
        }
        case M3_DELETE:
        case M3_GET: {
            break;
        }
    }
    ret = m3ua_r_asp(rasp, oprn, &api);
    if (-1 == ret) {
        printf("Error recorded at Line:%d, Error code:%d\n", __LINE__, m3errno);
    }
    return ret;
}

m3_s32 um3_m3ua_r_as(m3_u8    oprn)
{
    m3_s32              ret;
    m3ua_r_as_t         api;
    m3_u32              ras = 0x00000000;
    m3_r_as_confname_t  confname = M3_R_AS_RKEY;

    switch(oprn) {
        case M3_ADD: {
            api.add.info.rtctx = 7;
            api.add.info.min_act_asp = 1; /* reqd only for ldshr/brdcst */
            api.add.info.rkey.trfmode = M3_TRFMODE_OVERRIDE;
            api.add.info.rkey.nw_app = 0x01ABCDEF;
            api.add.info.rkey.num_rtparam = 1;
            api.add.info.rkey.rtparam[0].dpc = 0x00010101; /* 24 bit pointcode -- ANSI */
            api.add.info.rkey.rtparam[0].num_si = 2;
            api.add.info.rkey.rtparam[0].si_list[0] = 3; /* process traffic for SCCP */
            api.add.info.rkey.rtparam[0].si_list[1] = 5; /* process traffic for ISUP */
            api.add.info.rkey.rtparam[0].num_opc = 3;
            api.add.info.rkey.rtparam[0].opc_list[0] = 0x00010203;
            api.add.info.rkey.rtparam[0].opc_list[1] = 0x00010304;
            api.add.info.rkey.rtparam[0].opc_list[2] = 0x00010305;
            api.add.info.rkey.rtparam[0].num_ckt_range = 0;
            break;
        }
        case M3_DELETE:
        case M3_GET: {
            break;
        }
        case M3_MODIFY: {
            switch(confname) {
                case M3_R_AS_RTCTX: {
                /* modify routing context to 0x55667788 */
                    api.modify.info.rtctx = 0x55667788;
                    break;
                }
                case M3_R_AS_RKEY: {
                /* modify the routing key configured for the AS */
                    api.modify.info.rkey.trfmode = M3_TRFMODE_LOAD_SHARE;
                    api.modify.info.rkey.nw_app = 0x01ABCDEF;
                    api.modify.info.rkey.num_rtparam = 1;
                    api.modify.info.rkey.rtparam[0].dpc = 0x00020202;
                    api.modify.info.rkey.rtparam[0].num_si = 0;
                    api.modify.info.rkey.rtparam[0].num_opc = 0;
                    api.modify.info.rkey.rtparam[0].num_ckt_range = 0;
                    break;
                }
                case M3_R_AS_MIN_ACT_ASP: {
                /* modify minimum active asp requirement for this as */
                    api.modify.info.min_act_asp = 2;
                }
                case M3_R_AS_INFO: {
                    /* modify both routing context and routing key */
                    api.modify.info.rtctx = 0x55667788;
                    api.modify.info.rkey.trfmode = M3_TRFMODE_LOAD_SHARE;
                    api.modify.info.rkey.nw_app = 0x01ABCDEF;
                    api.modify.info.rkey.num_rtparam = 1;
                    api.modify.info.rkey.rtparam[0].dpc = 0x00020202;
                    api.modify.info.rkey.rtparam[0].num_si = 0;
                    api.modify.info.rkey.rtparam[0].num_opc = 0;
                    api.modify.info.rkey.rtparam[0].num_ckt_range = 0;
                    break;
                }
            }
            api.modify.confname = confname;
            break;
        }
    }
    ret = m3ua_r_as(ras, oprn, &api);
    if (-1 == ret) {
        printf("Error recorded at Line:%d, Error code:%d\n", __LINE__, m3errno);
    }
    return ret;
}

m3_s32 um3_m3ua_conn(m3_u8 oprn, m3_u32 lsp, m3_u32 rsp, m3_u32 assoc)
{
    m3_s32             ret;
    m3ua_conn_t        api;
    m3_conn_confname_t confname = M3_CONN_I_STR;

    switch(oprn) {
        case M3_ADD: {
            api.add.info.assoc_id = assoc;
            api.add.info.i_str = 8;
            api.add.info.o_str = 8;
            break;
        }
        case M3_DELETE:
        case M3_GET: {
            break;
        }
        case M3_MODIFY: {
            switch(confname) {
                case M3_CONN_ASSOC: {
                    api.add.info.assoc_id = assoc;
                    break;
                }
                case M3_CONN_I_STR: {
                    api.add.info.i_str = 6;
                    break;
                }
                case M3_CONN_O_STR: {
                    api.add.info.o_str = 6;
                    break;
                }
            }
            api.modify.confname = confname;
            break;
        }
    }
    ret = m3ua_conn(lsp, rsp, oprn, &api);
    if (-1 == ret) {
        printf("Error recorded at Line:%d, Error code:%d\n", __LINE__, m3errno);
    }
    return ret;
}

m3_s32 um3_m3ua_conn_state(m3_u8 oprn, m3_conn_state_t state, m3_u32 lsp, m3_u32 rsp)
{
    m3_s32            ret;
    m3ua_conn_state_t api;

    switch(oprn) {
        case M3_GET: {
            break;
        }
        case M3_MODIFY: {
            api.modify.state = state;
            break;
        }
    }
    ret = m3ua_conn_state(lsp, rsp, oprn, &api);
    if (-1 == ret) {
        printf("Error recorded at Line:%d, Error code:%d\n", __LINE__, m3errno);
    }
    return ret;
}

m3_s32 um3_m3ua_user(m3_u8    oprn)
{
    m3_s32            ret;
    m3ua_user_t       api;

    switch(oprn) {
        case M3_ADD: {
            api.add.info.sp_id = 0x00010000;
            api.add.info.user.nif_user.a_data = M3_TRUE;
            break;
        }
        case M3_DELETE: {
            break;
        }
    }
    ret = m3ua_user(0, oprn, &api);
    if (-1 == ret) {
        printf("Error recorded at Line:%d, Error code:%d\n", __LINE__, m3errno);
    }
    return ret;
}

m3_s32 um3_m3ua_txr(m3_u16 user, m3_rt_lbl_t *prtlbl)
{
    m3_s32      ret;
    m3ua_txr_t  api;
//	m3_u8		prot_data[] = {   0x01,0x01,0x61,0xda,0x02,0x02,0x06,0x04,0x43,0x41,
//	0x1f,0x8e,0x0f,0x47,0x00,0x13,0x40,0x43,0x00,0x00,0x06,0x00,0x03,0x40,0x01,0x00,
//	0x00,0x0f,0x40,0x06,0x00,0x64,0xf0,0x90,0x18,0x07,0x00,0x3a,0x40,0x08,0x00,0x64,
//	0xf0,0x90,0x18,0x07,0x00,0x01,0x00,0x10,0x40,0x11,0x10,0x05,0x24,0x71,0x03,0x00,
//	0x00,0x00,0x08,0x49,0x06,0x90,0x08,0x40,0x08,0x87,0x72,0x00,0x4f,0x40,0x03,0x01,
//	0x5d,0xf4,0x00,0x56,0x40,0x05,0x64,0xf0,0x90,0x07,0x0f,0x04,0x04,0x43,0x47,0x1f,
//	0x8e,0x00};
	m3_u8 prot_data1[] = {        0x06,0x00,0x00,0x03,0x00,0x01,0x20,0x00,0x14,0x40,
	0x1c,0x00,0x00,0x01,0x00,0x10,0x40,0x15,0x14,0x03,0x45,0x04,0x01,0xa0,0x5e,0x07,
	0x81,0x31,0x04,0x08,0x70,0x19,0xf0,0x40,0x04,0x04,0x02,0x60,0x00};
		/*
			0x09, 0x01, 0x03, 0x0e, 0x19, 0x0b, 0x92, 0x06,
			0x00, 0x12, 0x04, 0x19, 0x99, 0x96, 0x76, 0x39, 
			0x98, 0x0b, 0x12, 0x08, 0x00, 0x12, 0x04, 0x19, 
			0x89, 0x96, 0x92, 0x99, 0x29, 0x5a, 0x62, 0x58, 
			0x48, 0x04, 0x86, 0x12, 0x05, 0x72, 0x6b, 0x1a, 
			0x28, 0x18, 0x06, 0x07, 0x00, 0x11, 0x86, 0x05,
			0x01, 0x01, 0x01, 0xa0, 0x0d, 0x60, 0x0b, 0xa1, 
			0x09, 0x06, 0x07, 0x04, 0x00, 0x00, 0x01, 0x00,
			0x05, 0x03, 0x6c, 0x34, 0xa1, 0x32, 0x02, 0x01, 
			0x80, 0x02, 0x01, 0x16, 0x30, 0x2a, 0x80, 0x07,
			0x91, 0x19, 0x99, 0x96, 0x76, 0x39, 0x98, 0x83, 
			0x01, 0x00, 0x85, 0x01, 0x01, 0x86, 0x07, 0x91,
			0x19, 0x89, 0x96, 0x92, 0x99, 0x29, 0x87, 0x08, 
			0x9b, 0xac, 0x03, 0xb9, 0x08, 0x10, 0xae, 0x7d,
			0xab, 0x06, 0x03, 0x02, 0x06, 0xc0, 0x05, 0x00 };
*/


    api.nw_app = 0x01ABCDEF;
    api.add_rtctx = M3_TRUE;
    api.crn_id = 0xFCDC1298;    /* Correlation Id management is part of LME */
    api.rt_lbl = *prtlbl;
	/*if (5 == cc_success) {
	    api.prot_data_len = sizeof(prot_data1);
	    api.p_prot_data = prot_data1;
	}
	else {
		cc_success++;*/
	    api.prot_data_len = sizeof(prot_data1);
	    api.p_prot_data = prot_data1;
	//}
	printf("<---- Sending Data Message with DPC:%x, OPC:%x, SI:%d ---->\n",
        prtlbl->dpc, prtlbl->opc, prtlbl->si);
    ret = m3ua_transfer(user, &api);
    if (-1 == ret) {
        printf("Error recorded at Line:%d, Error code:%d\n", __LINE__, m3errno);
    }

    return ret;
}

m3_s32 um3_m3ua_resume(m3_u8 user, m3ua_user_ntfy_t *pntfy)
{
    m3_s32         ret;
    m3ua_resume_t  api;

    api.num_pc = 1;
    api.pc_list[0] = pntfy->param.audit.ptcode;
    api.nw_app = pntfy->param.audit.nw_app;
    api.info_len = 0;
    ret = m3ua_resume(pntfy->param.audit.user_id, &api);
    if (-1 == ret) {
        printf("Error recorded at Line:%d, Error code:%d\n", __LINE__, m3errno);
    }
    return ret;
}

void um3_timer_expiry(void *pdata)
{
    return;
}

void um3_process_mgmt_ntfy(m3ua_mgmt_ntfy_t *pntfy)
{
    m3_u16         idx; 

    switch(pntfy->type) {
        case M3_MGMT_NTFY_ASP_STATE: {
                printf("\n<---- ASP-STATE Notification ---->\n");
                printf("ASP:%x, RSP:%x, AS:%x, STATE:%d\n",
                    pntfy->param.asp.asp_id, pntfy->param.asp.rsp_id,
                    pntfy->param.asp.as_id, pntfy->param.asp.state);
                break;
        }
        case M3_MGMT_NTFY_AS_STATE: {
                printf("\n<---- AS-STATE Notification ---->\n");
                printf("AS:%x, LSP:%x, RSP:%x, STATE:%d\n",
                    pntfy->param.as.as_id, pntfy->param.as.lsp_id,
                    pntfy->param.as.rsp_id, pntfy->param.as.state);
                break;
        }
        case M3_MGMT_NTFY_R_ASP_STATE: {
                printf("\n<---- RASP-STATE Notification ---->\n");
                printf("RASP:%x, LSP:%x, AS:%x, STATE:%d\n",
                    pntfy->param.r_asp.asp_id, pntfy->param.r_asp.lsp_id,
                    pntfy->param.r_asp.as_id, pntfy->param.r_asp.state);
                break;
        }
        case M3_MGMT_NTFY_R_AS_STATE: {
                printf("<---- RAS-STATE Notification ---->\n");
                printf("RAS:%x,LSP:%x, STATE:%d\n",
                    pntfy->param.r_as.as_id, pntfy->param.r_as.lsp_id,
                    pntfy->param.r_as.state);
                break;
        }
        case M3_MGMT_NTFY_CONN_STATE: {
                printf("\n<---- CONN-STATE Notification ---->\n");
                printf("LSP:%x,RSP:%x, STATE:%d\n",
                    pntfy->param.conn.lsp_id, pntfy->param.conn.rsp_id,
                    pntfy->param.conn.state);
                break;
        }
        case M3_MGMT_NTFY_NOTIFY: {
                printf("\n<---- NOTIFY Notification ---->\n");
                printf("STATUS-TYPE:%d, STATUS-INF:%d, M3ASP-ID:%x\n",
                pntfy->param.notify.status_type,
                pntfy->param.notify.status_inf,
                pntfy->param.notify.m3asp_id);
                printf("NUM-AS:%d\n", pntfy->param.notify.num_as);
                for (idx = 0; idx < pntfy->param.notify.num_as; idx++) {
                    printf("AS[%d] = %x\n", idx,
                        pntfy->param.notify.as_list[idx]);
                }
                break;
        }
        case M3_MGMT_NTFY_ERR: {
                printf("\n<---- ERROR Notification ---->\n");
                printf("ERR-CODE:%u\n",pntfy->param.err.err_code);
                printf("NUM-RC:%d\n",pntfy->param.err.num_rc);
                for (idx = 0; idx < pntfy->param.err.num_rc; idx++) {
                    printf("RC[%d] = %x\n", idx,
                        pntfy->param.err.rc_list[idx]);
                }
                printf("NUM-PC:%d\n",pntfy->param.err.num_pc);
                for (idx = 0; idx < pntfy->param.err.num_pc; idx++) {
                    printf("PC[%d] = %x\n", idx,
                        pntfy->param.err.pc_list[idx]);
                }
                printf("DIAG-LEN:%d\n",pntfy->param.err.diag_len);
                printf("DIAG-INFO\n");
                for (idx = 0; idx < pntfy->param.err.diag_len; idx++) {
                    if (0 != idx && idx%16 == 0)
                        printf("\n");
                    printf("%2x ", pntfy->param.err.p_diag_inf[idx]);
                }
                printf("\n");
                break;
        }
    }
    return;
}

void um3_process_user_ntfy(m3ua_user_ntfy_t *pntfy)
{
    m3_u16        idx;
    m3_u32        pc;
    m3_u32        byte;

    switch(pntfy->type) {
        case M3_USER_NTFY_STATUS: {
                printf("\n <---- STATUS Notification ---->\n");
                printf("USER-ID:%d, NW-APP:%d, PTCODE:%d\n"
                       "CRND-DPC-%d, CAUSE-%d, INFO-%d\n",
                    pntfy->param.status.user_id,
                    pntfy->param.status.nw_app,
                    pntfy->param.status.ptcode,
                    pntfy->param.status.crnd_dpc,
                    pntfy->param.status.cause,
                    pntfy->param.status.inf);
                break;
        }
        case M3_USER_NTFY_TRANSFER: {
                printf("\n<---- TRANSFER Notification ---->\n");
                printf("USER-ID:%d, NW-APP:%x, RTCTX:%x, CRN-ID-%x\n",
                    pntfy->param.transfer.user_id,
                    pntfy->param.transfer.inf.nw_app,
                    pntfy->param.transfer.inf.rtctx,
                    pntfy->param.transfer.inf.crn_id);
                printf("ROUTE-LABEL -- DPC:%x, OPC:%x, SI:%d, NI:%d,"
                       " MP:%d, SLS:%d\n",
                    pntfy->param.transfer.inf.rt_lbl.dpc,
                    pntfy->param.transfer.inf.rt_lbl.opc,
                    pntfy->param.transfer.inf.rt_lbl.si,
                    pntfy->param.transfer.inf.rt_lbl.ni,
                    pntfy->param.transfer.inf.rt_lbl.mp,
                    pntfy->param.transfer.inf.rt_lbl.sls);
                printf("PROTOCOL-DATA-LEN:%d\n",
                    pntfy->param.transfer.inf.prot_data_len);
                printf("PROTOCOL-DATA\n");
                for (idx = 0; idx < pntfy->param.transfer.inf.prot_data_len; ) {
                    byte = pntfy->param.transfer.inf.p_prot_data[idx++];
                    //printf("%02x ", byte);
                    if (0 == idx%16)
                    {
                        //printf("\n");
                    }
                }
                //printf("\n");
                pc = pntfy->param.transfer.inf.rt_lbl.dpc;
                pntfy->param.transfer.inf.rt_lbl.dpc = pntfy->param.transfer.inf.rt_lbl.opc;
                pntfy->param.transfer.inf.rt_lbl.opc = pc;
                um3_m3ua_txr(0, &pntfy->param.transfer.inf.rt_lbl);
                break;
        }
        case M3_USER_NTFY_AUDIT: {
                printf("\n<---- AUDIT Notification ---->\n");
                printf("USER-ID:%d, NW-APP:%d, MASK:%d, PTCODE:%d\n",
                    pntfy->param.audit.user_id,
                    pntfy->param.audit.nw_app,
                    pntfy->param.audit.mask,
                    pntfy->param.audit.ptcode);
                um3_m3ua_resume(0, pntfy);
                break;
        }
    }
    return;
}

void um3_m3ua_trace_map()
{
    m3ua_set_trace_map(m3uaStartupTrace | m3uaAspmTrace | m3uaErrorTrace | m3uaInMsgTrace | m3uaOutMsgTrace | m3uaTimerTrace | m3uaSsnmTrace);
    m3ua_del_trace(m3uaTimerTrace);
    m3ua_add_trace(m3uaTimerTrace);
}

