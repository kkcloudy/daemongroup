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

#include <confasp1.h>
#include "sccp.h"
//#include "common/list.h"

int num_packets_sent;
int cc_success;
#define ASP_STATE(state, p)	if (0 == state) p = "DOWN"; \
				else if (1 == state) p = "INACTIVE"; \
				else if (2 == state) p = "ACTIVE";
int main(int argc, char *argv[])
{
    if (3 > argc) {
        printf("Usage: %s asp=<IP Address>:<Port> "
               "sgp=<IP Address>:<Port>\n", argv[0]);
        exit(1);
    }
	ranap_sccp_system();
    /**** Initialise the Transport Layer ****/
    um3_transport_init(argv);

    /**** Initialise m3ua stack ****/
    um3_m3ua_init();

    /**** setup m3ua trace map ****/
    um3_m3ua_trace_map();

    /******** Configure M3UA managed objects ********/

    /* Add nwapp */
    um3_m3ua_nwapp(M3_ADD);

    /* Add remote sgp */
    um3_m3ua_r_sgp(M3_ADD);

    /* Add sg */
    um3_m3ua_sg(M3_ADD);

    /* Add as */
    um3_m3ua_as(M3_ADD);

    /* Add asp */
    um3_m3ua_asp(M3_ADD);

    /* Add user(s) for asp */
    um3_m3ua_user(M3_ADD);

    /* Add routes */
    um3_m3ua_route(M3_ADD);

    /* Add connection's */
    um3_m3ua_conn(M3_ADD, 0x00000000, 0x00010000, 0);

    /* change connection state */
    um3_m3ua_conn_state(M3_MODIFY, M3_CONN_ESTB, 0x00000000, 0x00010000);

    /* send aspup from asp to remote sgp */
    um3_m3ua_asp_state(M3_MODIFY, M3_ASP_INACTIVE);
    
    waitfor_message(NULL);
    return 0;
}

int um3_get_asp1_addr(char *argv[], udp_addr_t *paddr)
{
    char        *p;
    char        *ip;
    char        *port;

    if (0 != strncmp("asp", argv[1], 3))
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
printf("at asp.c asp paddr->ipaddr %x, port %x \n", paddr->ipaddr, paddr->port);
    return 0;
}

int um3_get_sgp1_addr(char *argv[], udp_addr_t *paddr)
{
    char        *p;
    char        *ip;
    char        *port;

    if (0 != strncmp("sgp", argv[2], 3))
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
printf("at asp.c sgp paddr->ipaddr %x, port %x \n", paddr->ipaddr, paddr->port);
    return 0;
}

int um3_transport_init(char *argv[])
{
    udp_addr_t        addr;
    int               asp1_ep, sgp1_ep;

    /* create transport layer endpoints */
    um3_get_asp1_addr(argv, &addr);
    asp1_ep = make_lep(addr);
    um3_get_sgp1_addr(argv, &addr);
    sgp1_ep = make_rep(addr);
    um3_get_cn_addr(argv, &addr);
    sgp1_ep = make_rep(addr);

    /* create associations between the transport layer endpoints */
    make_assoc(asp1_ep, sgp1_ep, 0);
    return 0;
}

m3_s32 um3_m3ua_init(void)
{
    m3_s32        ret;

    ret = m3ua_init();
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
    printf("api.add.info.nw_app = %d\n",api.add.info.nw_app);
    ret = m3ua_nwapp(oprn, &api);
    if (-1 == ret) {
        printf("Error recorded at Line:%d, Error code:%d\n", __LINE__, m3errno);
    }
    return ret;
}

m3_s32 um3_m3ua_asp(m3_u8    oprn)
{
    m3_s32            ret;
    m3ua_asp_t        api;
    m3_u32            asp = 0x00000000;
    m3_u16            idx, idx2;
    m3_asp_confname_t confname = M3_ASP_ADD_R_ASP;

    switch(oprn) {
        case M3_ADD: {
            api.add.info.m3asp_id = 0x0000ABCD;
            api.add.info.sctp_ep_id = 0;
            api.add.info.def_nwapp = 0x01ABCDEF;
            api.add.info.as_list[0] = M3_TRUE;
            for (idx = 1; idx < M3_MAX_AS; idx++)
                api.add.info.as_list[idx] = M3_FALSE;
            for (idx = 0; idx < M3_MAX_R_AS; idx++) //OCT2009
                for (idx2 = 0; idx2 < M3_MAX_R_ASP; idx2++) //OCT2009
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
                case M3_ASP_M3ASP_ID: {
                /* modify ASP ID sent in ASPUP/NTFY message for this ASP */
                    api.modify.info.m3asp_id = 0x0000EF12;
                    break;
                }
                case M3_ASP_NWAPP: {
                /* modify default Network Appearance to 0x12345678 for */
                /* this ASP                                            */
                    api.modify.info.def_nwapp = 0x12345678;
                    break;
                }
                case M3_ASP_ADD_AS: {
                /* Add an AS with id 1 to the ASP */
                    api.modify.as_id = 1;
                    break;
                }
                case M3_ASP_DEL_AS: {
                /* Delete AS with id 3 to the ASP */
                    api.modify.as_id = 3;
                    break;
                }
                case M3_ASP_ADD_R_ASP: {
                /* Add remote asp 2 serving remote as 4 to ASP */
                    api.modify.ras_id = 4;
                    api.modify.rasp_id = 2;
                    break;
                }
                case M3_ASP_DEL_R_ASP: {
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
    ret = m3ua_asp(asp, oprn, &api);
    if (-1 == ret) {
        printf("Error recorded at Line:%d, Error code:%d\n", __LINE__, m3errno);
    }
    return ret;
}

m3_s32 um3_m3ua_as(m3_u8    oprn)
{
    m3_s32            ret;
    m3ua_as_t         api;
    m3_u32            as = 0x00000000;
    m3_as_confname_t  confname = M3_AS_RKEY;

    switch(oprn) {
        case M3_ADD: {
            api.add.info.rtctx = 7;
            api.add.info.rkey.trfmode = M3_TRFMODE_OVERRIDE;
            api.add.info.rkey.nw_app = 0x01ABCDEF;
            api.add.info.rkey.num_rtparam = 1;
            api.add.info.rkey.rtparam[0].dpc = 0x00010101; /* 24 bit pointcode -- ANSI */
            api.add.info.rkey.rtparam[0].num_si = 2;
            api.add.info.rkey.rtparam[0].si_list[0] = 3; /* process traffic for SCCP */
            api.add.info.rkey.rtparam[0].si_list[1] = 5; /* process traffic for ISUP */
            api.add.info.rkey.rtparam[0].num_opc = 4;
            api.add.info.rkey.rtparam[0].opc_list[0] = 0x00010202;
            api.add.info.rkey.rtparam[0].opc_list[1] = 0x00010203;
            api.add.info.rkey.rtparam[0].opc_list[2] = 0x00010304;
            api.add.info.rkey.rtparam[0].opc_list[3] = 0x00010305;
            api.add.info.rkey.rtparam[0].num_ckt_range = 0;
            break;
        }
        case M3_DELETE: 
        case M3_GET: {
            break;
        }
        case M3_MODIFY: {
            switch(confname) {
                case M3_AS_RTCTX: {
                /* modify routing context to 0x55667788 */
                    api.modify.info.rtctx = 0x55667788;
                    break;
                }
                case M3_AS_RKEY: {
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
                case M3_AS_INFO: {
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
    ret = m3ua_as(as, oprn, &api);
    if (-1 == ret) {
        printf("Error recorded at Line:%d, Error code:%d\n", __LINE__, m3errno);
    }
    return ret;
}

m3_s32 um3_m3ua_r_sgp(m3_u8    oprn)
{
    m3_s32            ret;
    m3ua_r_sgp_t      api;
    m3_u32            rsgp = 0x00010000;
    //m3_u32            rsgp = 0x00000000;

    switch(oprn) {
        case M3_ADD: {
            api.add.info.sctp_ep_id = 1;
            break;
        }
        case M3_DELETE:
        case M3_GET: {
            break;
        }
    }
    ret = m3ua_r_sgp(rsgp, oprn, &api);
    if (-1 == ret) {
        printf("Error recorded at Line:%d, Error code:%d\n", __LINE__, m3errno);
    }
    return ret;
}

m3_s32 um3_m3ua_sg(m3_u8    oprn)
{
    m3_s32            ret;
    m3ua_sg_t         api;
    m3_u32            sg = 0x00010000;
    m3_sg_confname_t  confname = M3_SG_MODE;

    switch(oprn) {
        case M3_ADD: {
            api.add.info.sgmode = M3_SGMODE_LOADSHARE;
            api.add.info.num_sgp = 1;
            api.add.info.sgp_list[0] = 0x00010000;
            break;
        }
        case M3_DELETE:
        case M3_GET: {
            break;
        }
        case M3_MODIFY: {
            switch(confname) {
                case M3_SG_MODE: {
                    api.modify.info.sgmode = M3_SGMODE_BROADCAST;
                    break;
                }
                case M3_SG_SGP_LIST: {
                    api.modify.info.num_sgp = 2;
                    api.modify.info.sgp_list[0] = 0x00010000;
                    api.modify.info.sgp_list[1] = 0x00010001;
                    break;
                }
                case M3_SG_INFO: {
                    api.modify.info.sgmode = M3_SGMODE_BROADCAST;
                    api.modify.info.num_sgp = 2;
                    api.modify.info.sgp_list[0] = 0x00010000;
                    api.modify.info.sgp_list[1] = 0x00010001;
                    break;
                }
            }
            api.modify.confname = confname;
            break;
        }
    }
    ret = m3ua_sg(sg, oprn, &api);
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

m3_s32 um3_m3ua_asp_state(m3_u8 oprn, m3_asp_state_t state)
{
    m3_s32            ret;
    m3ua_asp_state_t  api;
    m3_u32            asp = 0x00000000;
    m3_s8             *asp_state;

    ASP_STATE(state, asp_state);
    switch(oprn) {
        case M3_GET: {
            api.get.info.as_id = 0x00000000;
            break;
        }
        case M3_MODIFY: {
        /* modify asp state on both the connections */
            api.modify.info.num_as = 1;
            api.modify.info.as_list[0] = 0x00000000;
            api.modify.state = state;
            break;
        }
    }
    printf("<---- Initiating ASPM procedure to make ASP %s ---->\n", asp_state);
    ret = m3ua_asp_state(asp, 0x00010000, oprn, &api);
    if (-1 == ret) {
        printf("Error recorded at Line:%d, Error code:%d\n", __LINE__, m3errno);
    }
    return ret;
}

m3_s32 um3_m3ua_route(m3_u8    oprn)
{
    m3_s32            ret;
    m3ua_route_t      api;

    switch(oprn) {
        case M3_ADD: {
        /* add routes through configured SG(s) */
            api.add.info.le_id = 0x00010000;
            api.add.info.priority = 1;
            api.add.info.pc_inf.nw_app = 0x01ABCDEF;
            api.add.info.pc_inf.ptcode = 0x00010203;
            ret = m3ua_route(oprn, &api);
            api.add.info.pc_inf.ptcode = 0x00010304;
            ret = m3ua_route(oprn, &api);
            api.add.info.pc_inf.ptcode = 0x00010305;
            ret = m3ua_route(oprn, &api);
            break;
        }
        case M3_DELETE: {
            api.del.info.le_id = 0x00010000;
            api.del.info.priority = 1;  
            api.del.info.pc_inf.nw_app = 0x01ABCDEF;
            api.del.info.pc_inf.ptcode = 0x00010203;
            ret = m3ua_route(oprn, &api);
            api.del.info.pc_inf.ptcode = 0x00010304;
            ret = m3ua_route(oprn, &api);
            api.del.info.pc_inf.ptcode = 0x00010305;
            ret = m3ua_route(oprn, &api);
            break;
        }
    }
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
            api.add.info.sp_id = 0x00000000;
            api.add.info.user.mtp_user.sio = 3;
            api.add.info.user.mtp_user.as_id = 0x00000000;
            ret = m3ua_user(0, oprn, &api);
            if (-1 == ret) {
                printf("Error recorded at Line:%d, Error code:%d\n",
                    __LINE__, m3errno);
            }

            api.add.info.sp_id = 0x00000000;
            api.add.info.user.mtp_user.sio = 5;
            api.add.info.user.mtp_user.as_id = 0x00000000;
            ret = m3ua_user(1, oprn, &api);
            if (-1 == ret) {
                printf("Error recorded at Line:%d, Error code:%d\n",
                    __LINE__, m3errno);
            }
            break;
        }
        case M3_DELETE: {
            ret = m3ua_user(0, oprn, &api);
            if (-1 == ret) {
                printf("Error recorded at Line:%d, Error code:%d\n",
                    __LINE__, m3errno);
            }
            ret = m3ua_user(1, oprn, &api);
            if (-1 == ret) {
                printf("Error recorded at Line:%d, Error code:%d\n",
                    __LINE__, m3errno);
            }
            break;
        }
    }
    return ret;
}

m3_s32 um3_m3ua_txr(m3_u16 user, m3_rt_lbl_t *prtlbl)
{
#if 0
    m3_s32      ret;
    m3ua_txr_t  api;
    m3_u8       prot_data[] = {   0x01,0x01,0x61,0xda,0x02,0x02,0x06,0x04,0x43,0x41,
	0x1f,0x8e,0x0f,0x47,0x00,0x13,0x40,0x43,0x00,0x00,0x06,0x00,0x03,0x40,0x01,0x00,
	0x00,0x0f,0x40,0x06,0x00,0x64,0xf0,0x90,0x18,0x07,0x00,0x3a,0x40,0x08,0x00,0x64,
	0xf0,0x90,0x18,0x07,0x00,0x01,0x00,0x10,0x40,0x11,0x10,0x05,0x24,0x71,0x03,0x00,
	0x00,0x00,0x08,0x49,0x06,0x90,0x08,0x40,0x08,0x87,0x72,0x00,0x4f,0x40,0x03,0x01,
	0x5d,0xf4,0x00,0x56,0x40,0x05,0x64,0xf0,0x90,0x07,0x0f,0x04,0x04,0x43,0x47,0x1f,
	0x8e,0x00};
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
	if (5 == cc_success) {
	    api.prot_data_len = sizeof(prot_data1);
	    api.p_prot_data = prot_data1;
	}
	else {
	    api.prot_data_len = sizeof(prot_data);
	    api.p_prot_data = prot_data;
		cc_success++;
	}
	printf("<---- Sending Data Message with DPC:%x, OPC:%x, SI:%d ---->\n",
        prtlbl->dpc, prtlbl->opc, prtlbl->si);
    ret = m3ua_transfer(user, &api);
    if (-1 == ret) {
        printf("Error recorded at Line:%d, Error code:%d\n", __LINE__, m3errno);
    }

    return ret;
#endif
	return 0;
}

m3_s32 um3_m3ua_audit(m3_u8 user)
{
    m3_s32        ret;
    m3ua_audit_t  api;

    api.num_pc = 3;
    api.pc_list[0] = 0x00010203;
    api.pc_list[1] = 0x00010304;
    api.pc_list[2] = 0x00010305;
    api.nw_app = 0x01ABCDEF;
    api.info_len = 0;
    ret = m3ua_audit(user, &api);
    if (-1 == ret) {
        printf("Error recorded at Line:%d, Error code:%d\n", __LINE__, m3errno);
    }
    return ret;
}

void um3_timer_expiry(void *pdata)
{
    m3_rt_lbl_t    rtlbl;

    if (10 > num_packets_sent) {
        rtlbl.opc = 0x00010101;
        rtlbl.dpc = 0x00010304;
        rtlbl.si = 3;
        rtlbl.ni = 2;
        rtlbl.mp = 4;
        rtlbl.sls = 6;
        um3_m3ua_txr(0, &rtlbl);
        num_packets_sent++;
        um3_timer_start(2, (void *)&num_packets_sent);
    }
    else if (20 > num_packets_sent) {
        rtlbl.opc = 0x00010101;
        rtlbl.dpc = 0x00010305;
        rtlbl.si = 3;
        rtlbl.ni = 2;
        rtlbl.mp = 8;
        rtlbl.sls = 14;
        um3_m3ua_txr(1, &rtlbl);
        num_packets_sent++;
        um3_timer_start(2, (void *)&num_packets_sent);
    }
    else {
        um3_m3ua_asp_state(M3_MODIFY, M3_ASP_DOWN);
    }
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
                if (0x00010000 == pntfy->param.asp.rsp_id) {
                    switch(pntfy->param.asp.state) {
                        case M3_ASP_DOWN: {
                            um3_m3ua_asp_state(M3_MODIFY, M3_ASP_INACTIVE);
                            break;
                        }
                        case M3_ASP_INACTIVE: {
                            if (0xFFFFFFFF == pntfy->param.asp.as_id)
                                um3_m3ua_asp_state(M3_MODIFY, M3_ASP_ACTIVE);
                            break;
                        }
                        case M3_ASP_ACTIVE: {
                            um3_m3ua_audit(0);
                            break;
                        }
                    }
                }
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
                printf("\n<---- RAS-STATE Notification ---->\n");
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
    m3_u32        byte;

    switch(pntfy->type) {
        case M3_USER_NTFY_PAUSE: {
                printf("\n<---- PAUSE Notification ---->\n\n");
                printf("USER-ID:%d, NW-APP:%d, PTCODE:%d\n",
                    pntfy->param.pause.user_id,
                    pntfy->param.pause.nw_app,
                    pntfy->param.pause.ptcode);
                break;
        }
        case M3_USER_NTFY_RESUME: {
                printf("\n<---- RESUME Notification ---->\n");
                printf("USER-ID:%d, NW-APP:%d, PTCODE:%d\n",
                    pntfy->param.resume.user_id,
                    pntfy->param.resume.nw_app,
                    pntfy->param.resume.ptcode);
                num_packets_sent = 0;
                um3_timer_start(2, (void *)&num_packets_sent);
                break;
        }
        case M3_USER_NTFY_STATUS: {
                printf("\n<---- STATUS Notification ---->\n");
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
                printf("USER-ID:%d, NW-APP:%x, RTCTX:%x, CRN-ID-%u\n",
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
                    printf("%02x ", byte);
                    if (0 == idx%16)
                        printf("\n");
                }
                printf("\n");
                /* Check if data has arrived from AS served by ASP2 */
                if (0x00010202 == pntfy->param.transfer.inf.rt_lbl.opc) {
                    pntfy->param.transfer.inf.rt_lbl.dpc = 0x00010202;
                    pntfy->param.transfer.inf.rt_lbl.opc = 0x00010101;
                    um3_m3ua_txr(0, &pntfy->param.transfer.inf.rt_lbl);
                }
                break;
        }
    }
    return;
}

void um3_m3ua_trace_map()
{
    m3ua_set_trace_map(m3uaStartupTrace | m3uaErrorTrace | m3uaInMsgTrace | m3uaOutMsgTrace | m3uaTimerTrace | m3uaAspmTrace | m3uaSsnmTrace);
    m3ua_del_trace(m3uaTimerTrace);
    m3ua_add_trace(m3uaTimerTrace);
}

