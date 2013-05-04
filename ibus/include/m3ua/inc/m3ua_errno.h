/****************************************************************************
** Description:
*****************************************************************************
** Copyright(C) 2009 Shabd Communications Pvt. Ltd. http://www.shabdcom.org
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

#ifndef __M3UA_ERRNO_H__
#define __M3UA_ERRNO_H__

#include <m3uaTraceMgr.h> //OCT2009

/****************** Error codes ******************/
#define M3_NOERR                                      10000
#define EM3_NULL_PARAM                                10001
#define EM3_INV_ASID                                  10002
#define EM3_INV_OPER                                  10003
#define EM3_ASID_ALRDY                                10004
#define EM3_RTCTX_ALRDY                               10005
#define EM3_RKEY_ALRDY                                10006
#define EM3_INV_CONFNAME                              10007
#define EM3_INV_RTCTX                                 10008
#define EM3_INV_ASPID                                 10009
#define EM3_ASPID_ALRDY                               10010
#define EM3_INV_M3ASP                                 10011
#define EM3_M3ASPID_ALRDY                             10012
#define EM3_INV_ASLIST                                10013
#define EM3_INV_ASPLIST                               10014
#define EM3_INV_STD                                   10015
#define EM3_INV_R_ASID                                10016
#define EM3_INV_R_ASPID                               10017
#define EM3_R_ASP_NOT_SERV                            10018
#define EM3_INV_R_SGPID                               10019
#define EM3_INV_RSPID                                 10020
#define EM3_AS_NOT_SERV                               10021
#define EM3_INV_ASPSTATE                              10022
#define EM3_R_ASP_ALRDY_SERV                          10023
#define EM3_INV_LSPID                                 10024
#define EM3_SGP_SGP_CONN                              10025
#define EM3_CONN_ALRDY                                10026
#define EM3_CONN_TABLE_FULL                           10027
#define EM3_INV_CONN                                  10028
#define EM3_INV_CONNSTATE                             10029
#define EM3_INV_USRID                                 10030
#define EM3_INV_NWAPP                                 10031
#define EM3_NWAPP_ALRDY                               10032
#define EM3_NWAPP_TABLE_FULL                          10033
#define EM3_INV_MINACTASP                             10034
#define EM3_MALLOC_FAIL                               10035
#define EM3_INV_SGID                                  10036
#define EM3_INV_LEID                                  10037
#define EM3_R_SGPID_ALRDY                             10038
#define EM3_SGID_ALRDY                                10039
#define EM3_INV_SGMODE                                10040
#define EM3_INV_NUMSGP                                10041
#define EM3_SGPID_ALRDY                               10042
#define EM3_MGMT_NTFY_TABLE_FULL                      10043
#define EM3_USER_NTFY_TABLE_FULL                      10044
#define EM3_INV_TAG                                   10045
#define EM3_INV_ASSOCID                               10046
#define EM3_SENDMSG_FAIL                              10047
#define EM3_INV_TIMERID                               10048
#define EM3_INSUFF_TIMERS                             10049
#define EM3_RTCTX_ABSENT                              10050
#define EM3_INV_ERRCODE                               10051
#define EM3_NO_USER                                   10052
#define EM3_INV_EVTSTATE                              10053
#define EM3_NO_USER_NTFY_PD                           10054
#define EM3_USRID_ALRDY                               10055
#define EM3_INV_SGPID                                 10056
#define EM3_USR_ALRDY_REGD                            10057
#define EM3_INV_SPID                                  10058
#define EM3_R_ASPID_ALRDY                             10059
#define EM3_R_ASID_ALRDY                              10060
#define EM3_NO_MGMT_NTFY_PD                           10061
#define EM3_INV_TIMER                                 10062
#define EM3_NO_HBEAT_PROC                             10063
#define EM3_HBEAT_PROC_ALRDY                          10064
#define EM3_INV_NUM_AS                                10065
#define EM3_INV_R_SPID                                10066
#define EM3_CONN_EXIST                                10067
#define EM3_SG_WITH_SGP                               10068
#define EM3_AS_WITH_USR                               10069
#define EM3_INV_NUMSI                                 10070
#define EM3_INV_NUMOPC                                10071
#define EM3_INV_NUMCKT                                10072
#define EM3_INV_TRAFFIC_MODE                          10073
#define EM3_INV_NUMPC                                 10074
#define EM3_INV_INFOLEN                               10075
#define EM3_INV_CAUSE                                 10076
#define EM3_INV_CONGLEVEL                             10077
#define EM3_INV_RESTART_STATUS                        10078
#define EM3_INV_ROUTING                               10079
#define EM3_R_AS_NOT_ACTIVE                           10080
#define EM3_SG_NOT_ACTIVE                             10081
#define EM3_NO_ROUTE                                  10082
#define EM3_NO_LE_ACTIVE                              10083
#define EM3_INV_NUMDPC                                10084
#define EM3_REP_TAG                                   10085
#define EM3_RKSTATE_MISMATCH                          10086
#define EM3_RTCTX_NONUNIQUE                           10087
#define EM3_INV_REG_STATUS                            10088
#define EM3_INV_DEREG_STATUS                          10089
#define EM3_INV_LRKID                                 10090
#define EM3_INV_NUM_RESULT                            10091
#define EM3_INV_MEMSIZE                               10092
#define EM3_MEMINIT_FAIL                              10093
#define EM3_MEMPOOL_UNAVAIL                           10094
#define EM3_TIMERINIT_FAIL                            10095
#define EM3_INV_NUMSLSRANGE                           10096
#define EM3_INV_RTSTATE                               10097
#define EM3_MAX_ROUTES_PER_SG                         10098
#define EM3_INV_ROUTE_THRU_SG                         10099


#define M3_MAX_ERRNO                                  10099
#define M3_MIN_ERRNO                                  10000

/****************** Error reporting Macro **********************/
extern int  m3errno;
#define M3ERRNO(errno)    M3TRACE(m3uaErrorTrace,("ErrorNo:%d",errno)); m3errno = errno;
#define M3_ASSIGN_ERRNO(errno) m3errno = errno;

#endif

