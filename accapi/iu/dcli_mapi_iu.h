#ifndef _DCLI_MAPI_IU_H_
#define _DCLI_MAPI_IU_H_

/*
** defines and types of dcli_iu
** founded by book, 2012-1-4
*/


#define MAX_M3_ASP_STATE            7
#define MAX_M3_CONN_STATE           7
#define MAX_SCTP_CONN_STATE         9


/* ---------m3ua asp states---------- */
const char* M3_ASP_STATE[MAX_M3_ASP_STATE] = 
{
    "ASP_DOWN",
    "ASP_INACTIVE",
    "ASP_ACTIVE",
    "ASP_DNSENT",
    "ASP_UPSENT",
    "ASP_ACSENT",
    "ASP_IASENT"
};


/* ---------m3ua conn states--------- */
const char* M3_CONN_STATE[MAX_M3_CONN_STATE] = 
{
    "CONN_NOT_ESTB",
    "CONN_SETUP_IN_PROG",
    "CONN_ESTB",
    "CONN_CONG_1",
    "CONN_CONG_2",
    "CONN_CONG_3",
    "CONN_ALIVE"
};


/* ------------sctp states----------- */
const char* SCTP_CONN_STATE[MAX_SCTP_CONN_STATE] = 
{
    "SCTP_CLOSED",
    "SCTP_LISTEN",
    "SCTP_COOKIE_WAIT",
    "SCTP_COOKIE_ECHOED",
    "SCTP_ESTABLISHED",
    "SCTP_SHUTDOWN_PENDING",
    "SCTP_SHUTDOWN_SEND",
    "SCTP_SHUTDOWN_RECEIVED",
    "SCTP_SHUTDOWN_ACK_SEND"
};


#endif

