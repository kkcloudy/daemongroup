/*
 * Note: this file originally auto-generated by mib2c using
 *  : generic-table-oids.m2c,v 1.10 2004/10/08 23:39:17 rstory Exp $
 *
 * $Id:$
 */
#ifndef DOT11WTPSTATABLE_OIDS_H
#define DOT11WTPSTATABLE_OIDS_H

#ifdef __cplusplus
extern "C" {
#endif


/* column number definitions for table dot11WtpStaTable */
//#define DOT11WTPSTATABLE_OID              1,3,6,1,4,1,31656,6,1,1,8,1
#define COLUMN_WTPSTAMACADDR		1
#define COLUMN_WTPSTAIP		2
#define COLUMN_WTPWIRELESSCLIENTSNR		3
#define COLUMN_WTPTXTERMINALPACKMOUNT		4
#define COLUMN_WTPTXTERMINALBYTEMOUNT		5
#define COLUMN_WTPTERMINALRXPACKMOUNT		6
#define COLUMN_WTPTERMINALRXBYTEMOUNT		7
#define COLUMN_WTPTERMINALRESPACK		8
#define COLUMN_WTPTERMINALRESBYTE		9
#define COLUMN_WTPTERMINALRXERRPACK		10
#define COLUMN_WTPMACTERMADDRUSRONLINETIME		11
#define COLUMN_WTPMACTERMADDRUSRTXSPD		12
#define COLUMN_WTPMACTERMADDRUSRRXSPD		13
#define COLUMN_WTPMACTERMADDRUSRALLTHROUGHPUT		14
#define COLUMN_WTPMACTERMAPRECEIVEDSTASIGNALSTRENGTH		15
#define COLUMN_WTPMACTERMSTATXFRAGMENTEDPKTS		16
#define COLUMN_WTPMACTERMAPTXFRAGMENTEDPKTS		17
#define COLUMN_WTPBELONGAPID		18
#define COLUMN_WTPTERMINALACCESSTIME		19
#define COLUMN_WTPBELONGAPNAME		21
#define COLUMN_WTPTERMINALRECVDATAPACKMOUNT	22
#define COLUMN_WTPSENDTERMINALDATAPACKMOUNT	23
#define COLUMN_WTPTERMINALTXDATARATEPKTS		24
#define COLUMN_WTPTERMINALRXDATARATEPKTS		25
#define COLUMN_WTPTERMINALTXSIGNALSTRENGTHPKTS		26
#define COLUMN_APRXRATES					27
#define COLUMN_APTXRATES					28
#define COLUMN_MAXOFRATESET					29
#define COLUMN_WTPSTAIPV6ADDR         30
    

#define DOT11WTPSTATABLE_MIN_COL   COLUMN_WTPSTAMACADDR
#define DOT11WTPSTATABLE_MAX_COL   COLUMN_WTPSTAIPV6ADDR
    


#ifdef __cplusplus
}
#endif

#endif /* DOT11WTPSTATABLE_OIDS_H */
