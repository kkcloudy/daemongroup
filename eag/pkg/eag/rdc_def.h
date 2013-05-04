/*******************************************************************************
Copyright (C) Autelan Technology


This software file is owned and distributed by Autelan Technology 
********************************************************************************


THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR 
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************************
* rdc_def.h
*
*
* CREATOR:
* autelan.software.Network Dep. team
*
* DESCRIPTION:
* rdc typedef
*
*
*******************************************************************************/

#ifndef _RDC_DEF_H
#define _RDC_DEF_H

typedef struct rdc_ins rdc_ins_t;

typedef struct rdc_server rdc_server_t;

typedef struct rdc_client rdc_client_t;
typedef struct rdc_sockclient rdc_sockclient_t;

typedef struct rdc_coa rdc_coa_t;

typedef struct rdc_pktconn_db rdc_pktconn_db_t;
typedef struct rdc_pktconn rdc_pktconn_t;

typedef struct rdc_coaconn_db rdc_coaconn_db_t;
typedef struct rdc_coaconn	rdc_coaconn_t;
typedef struct  rdc_coa_radius_conf rdc_coa_radius_conf_t;

typedef struct rdc_userconn_db rdc_userconn_db_t;
typedef struct rdc_userconn rdc_userconn_t;

#endif	/* _RDC_DEF_H */
